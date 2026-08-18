#!/usr/bin/env python3
"""Golden-Audio-Fixtures des EQ-Copilot (Plan §10.3, M1-Abnahme §11).

Erzeugt DETERMINISTISCH (fester Seed, feste Phasen) 20-s-Signale
48 kHz/Stereo/24-bit nach eq-copilot/fixtures/ und friert die Referenzwerte des
bestehenden Analyzers (tools/analyze-track.py) als golden-referenz.json ein.
Die WAVs werden NICHT eingecheckt — jeder Lauf erzeugt bitidentische Dateien,
das JSON ist der eingefrorene Vertrag.

ALLE Fixtures sind seit metrics m3-2026-08-14 kanalidentisch (L==R): analyze-
track misst spektral den Mid-Mix 0.5*(L+R), die Engine seit m3 die mittlere
KANALenergie (PSD_L+PSD_R)/2 — nur bei L==R sind beide mathematisch identisch,
und die Kreuzvalidierung bleibt ein exakter Maßstab. Die Stereo-Korrektheit
(Antiphase, Only-L, dekorreliert) beweisen analytische Zusatzpruefungen direkt
im GoldenTest, abgeleitet aus denselben WAVs.

Aufruf:  py -3.13 tools/eq-copilot/erzeuge_fixtures.py [--nur-wav]
"""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import sys
import wave
from pathlib import Path

import numpy as np

RATE = 48_000
DAUER_S = 20.0
N = int(RATE * DAUER_S)
WURZEL = Path(__file__).resolve().parents[2]
FIXDIR = WURZEL / "eq-copilot" / "fixtures"


def _schreibe_wav(pfad: Path, links: np.ndarray, rechts: np.ndarray) -> str:
    """24-bit-PCM schreiben und SHA-256 zurückgeben (Determinismus-Beleg)."""
    stereo = np.stack([links, rechts], axis=1)
    stereo = np.clip(stereo, -1.0, 1.0)
    ganz = (stereo * 8_388_607.0).astype(np.int32)
    roh = bytearray()
    flach = ganz.reshape(-1)
    bytes_alle = flach.astype("<i4").tobytes()
    # 24-bit: von jedem little-endian int32 die unteren 3 Bytes.
    for i in range(0, len(bytes_alle), 4):
        roh += bytes_alle[i : i + 3]
    with wave.open(str(pfad), "wb") as w:
        w.setnchannels(2)
        w.setsampwidth(3)
        w.setframerate(RATE)
        w.writeframes(bytes(roh))
    return hashlib.sha256(pfad.read_bytes()).hexdigest()


def _pegel(db: float) -> float:
    return 10.0 ** (db / 20.0)


def baue_signale() -> dict[str, tuple[np.ndarray, np.ndarray]]:
    t = np.arange(N, dtype=np.float64) / RATE
    signale: dict[str, tuple[np.ndarray, np.ndarray]] = {}

    # 1 · Sinus 1 kHz, −12 dBFS, beide Kanäle identisch — Frequenz-/Pegelanker.
    sinus = _pegel(-12.0) * np.sin(2 * np.pi * 1000.0 * t)
    signale["sinus-1k-minus12"] = (sinus, sinus.copy())

    # 2 · Pink Noise −20 dBFS RMS (Voss-frei: 1/f über FFT-Formung, fester Seed)
    # — Breitband-Anker für LTAS-Neigung und Bandenergie. L==R (s. Docstring).
    rng = np.random.default_rng(20260813)
    weiss = rng.standard_normal(N)
    spektrum = np.fft.rfft(weiss)
    f = np.fft.rfftfreq(N, 1.0 / RATE)
    formung = np.ones_like(f)
    formung[1:] = 1.0 / np.sqrt(f[1:])
    formung[0] = 0.0
    pink = np.fft.irfft(spektrum * formung, n=N)
    pink *= _pegel(-20.0) / np.sqrt(np.mean(pink**2))
    signale["pink-minus20"] = (pink, pink.copy())

    # 3 · Resonanz-Fall der Beispielsitzung §7.9: Pink-Bett −26 dBFS plus
    # dauerhafter 116-Hz-Ton +12 dB über dem Bett in seinem Band — der
    # R-RES-01-Kandidat, den M1 finden MUSS (Bassfenster-Pflicht §5.10.1).
    bett = pink * _pegel(-6.0)
    ton116 = _pegel(-20.0) * np.sin(2 * np.pi * 116.0 * t)
    reso = bett + ton116
    signale["resonanz-116hz"] = (reso, reso.copy())

    # 4 · Stille mit einem kurzen −60-dBFS-Tick — das Gating-Signal: hier darf
    # NIE ein Befund entstehen („noch nicht messbar", §5.3).
    still = np.zeros(N)
    still[RATE : RATE + 48] = _pegel(-60.0)
    signale["stille-mit-tick"] = (still, still.copy())

    # ── M3-Diagnose-Fixtures: Pink mit KONSTRUIERTEN Fehlern ────────────────
    # (Gauß-Glocken in log2-f auf das SELBE Pink — die Seeds oben bleiben
    # unangetastet, die vier Kern-SHAs ändern sich nicht.) Beweisidee: der
    # eingebaute Fehler MUSS als genau seine Befundklasse gefunden werden,
    # das unveränderte Pink (pink-minus20) bleibt karten-still.
    def forme(sig: np.ndarray, fc: float, breite_okt: float, gain_db: float) -> np.ndarray:
        sp = np.fft.rfft(sig)
        fr = np.fft.rfftfreq(N, 1.0 / RATE)
        x = np.log2(np.maximum(fr, 1e-6) / fc) / breite_okt
        g = gain_db * np.exp(-0.5 * x * x)
        aus = np.fft.irfft(sp * 10.0 ** (g / 20.0), n=N)
        return aus

    # 5 · „Smile-EQ": Mittenloch −6 dB um 1 kHz (≈ −3…−6 dB über 500–2000).
    scoop = forme(pink, 1000.0, 0.9, -6.0)
    signale["diag-scoop-mitte"] = (scoop, scoop.copy())
    # 6 · Mulm: +7 dB um 180 Hz (deckt 120–300 deutlich über der Schwelle).
    mulm = forme(pink, 180.0, 0.55, 7.0)
    signale["diag-mulm"] = (mulm, mulm.copy())
    # 7 · Härte: +6 dB um 3,3 kHz (deckt 2,5–5 kHz).
    haerte = forme(pink, 3300.0, 0.45, 6.0)
    signale["diag-haerte"] = (haerte, haerte.copy())

    # ── M3a-Zeitverlauf-Fixtures (Benchmark-Studie → Zonen-Persistenz) ──────
    # 8 · Mulm NUR in der zweiten Hälfte: erste 10 s reines Pink, danach Pink
    # mit +10-dB-Glocke um 180 Hz. +10 statt +7, weil die Halbzeit-Mittelung
    # in LEISTUNG komprimiert: 10·log10((1+10^(g/10))/2) macht aus +7 dB nur
    # ~+3,2 dB Zonen-Delta (unter der 4-dB-Schwelle — Karte erschiene nie);
    # +10 dB landet bei ~+4,8 dB. Der Zonen-Zeitverlauf MUSS ≈ halbe
    # Musikzeit melden — Beweis, dass Persistenz „durchgehend" von
    # „zeitweise" trennt.
    halbzeit = np.concatenate([pink[: N // 2], forme(pink, 180.0, 0.55, 10.0)[N // 2 :]])
    signale["diag-mulm-halb"] = (halbzeit, halbzeit.copy())
    # 9 · Wandernder Ton auf Pink-Bett (−26 dBFS): 4 × 5 s bei 116/164/232/
    # 328 Hz — jedes Band trägt den Ton nur ~25 % der Zeit. Der Detektor MUSS
    # zeitweise Kandidaten liefern und darf KEINEN als dauerhaft führen
    # (RESO-Prinzip: Persistenz-Statistik, nicht Momentaufnahme).
    bett = pink * _pegel(-6.0)
    wander = np.zeros(N)
    viertel = N // 4
    for i, fq in enumerate([116.0, 164.0, 232.0, 328.0]):
        von = i * viertel
        bis = (i + 1) * viertel if i < 3 else N
        wander[von:bis] = _pegel(-20.0) * np.sin(2 * np.pi * fq * t[von:bis])
    wanderton = bett + wander
    signale["diag-wander-ton"] = (wanderton, wanderton.copy())

    return signale


def lade_analyze():
    # analyze-track.py ist ein lebendes FL-Studio-Werkzeug und blieb beim
    # Workspace-Umzug (18.08.2026) dort — dokumentierter Fallback, nur für
    # die Golden-Referenz-Regeneration gebraucht (--nur-wav braucht es nicht).
    kandidaten = [
        WURZEL / "tools" / "analyze-track.py",
        Path.home() / "FL-Studio" / "tools" / "analyze-track.py",
    ]
    pfad = next((k for k in kandidaten if k.exists()), None)
    if pfad is None:
        raise SystemExit(
            "analyze-track.py nicht gefunden (gesucht: "
            + ", ".join(str(k) for k in kandidaten)
            + ") — fuer reine WAV-Erzeugung --nur-wav nutzen."
        )
    spec = importlib.util.spec_from_file_location("analyze_track", pfad)
    modul = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(modul)  # type: ignore[union-attr]
    return modul


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--nur-wav", action="store_true", help="nur WAVs erzeugen, keine Analyse")
    args = ap.parse_args()

    FIXDIR.mkdir(parents=True, exist_ok=True)
    hashes: dict[str, str] = {}
    for name, (l, r) in baue_signale().items():
        pfad = FIXDIR / f"{name}.wav"
        hashes[name] = _schreibe_wav(pfad, l, r)
        print(f"  {name}.wav  sha256 {hashes[name][:16]}…")

    if args.nur_wav:
        return 0

    modul = lade_analyze()
    referenz: dict[str, object] = {
        "kommentar": (
            "Eingefrorene Referenzwerte des Offline-Analyzers für die "
            "M1-Kreuzvalidierung (Plan §11 M1-Abnahme). Neu einfrieren NUR mit "
            "neuer metrics_version und begründetem Commit. Die diag-*-Signale "
            "tragen nur SHA-Riegel: sie prüfen die M3-Diagnose STRUKTURELL "
            "(konstruierter Fehler wird als seine Klasse gefunden) — eine "
            "analyze-Referenz bewiese dort nichts Zusätzliches."
        ),
        "erzeugt_mit": "analyze-track.py (FL-Studio-Werkzeug, Fallback-Suche)",
        "rate_hz": RATE,
        "dauer_s": DAUER_S,
        "wav_sha256": hashes,
        "signale": {},
    }
    # Echte Schlüssel des analyze()-Dicts (nachgeschlagen, nicht geraten):
    # Skalare + LTAS-Logspektrum + Resonanzkandidaten — genau die Achsen, die
    # M1 live nachrechnen und hier innerhalb Toleranz treffen muss.
    zentrale_felder = [
        "lufs", "tp_dbtp", "crest", "centroid_mag", "rolloff",
        "flat", "width", "corr", "low_frac",
    ]
    kern_namen = [n for n in hashes if not n.startswith("diag-")]
    for name in kern_namen:
        pfad = FIXDIR / f"{name}.wav"
        print(f"  analysiere {name} …")
        voll = modul.analyze(str(pfad))
        fehlend = [k for k in zentrale_felder if k not in voll]
        if fehlend:
            raise SystemExit(f"analyze() liefert {fehlend} nicht mehr — Vertrag prüfen")
        auszug = {k: voll[k] for k in zentrale_felder}
        auszug["band_pct"] = voll.get("band_pct")
        auszug["ltas"] = {
            "centers_hz": voll.get("logspec_centers"),
            "db": voll.get("logspec_db"),
        }
        auszug["resonances"] = voll.get("resonances")
        referenz["signale"][name] = auszug  # type: ignore[index]

    def strikt(obj):
        """±inf/NaN → null: striktes JSON, sonst lehnt der Rust-Parser die
        Datei ab (Stille liefert lufs = −inf — das ist Inhalt, kein Fehler)."""
        if isinstance(obj, dict):
            return {k: strikt(v) for k, v in obj.items()}
        if isinstance(obj, (list, tuple)):
            return [strikt(v) for v in obj]
        if isinstance(obj, float) and (obj != obj or obj in (float("inf"), float("-inf"))):
            return None
        return obj

    ziel = FIXDIR / "golden-referenz.json"
    ziel.write_text(
        json.dumps(strikt(referenz), indent=2, ensure_ascii=False, allow_nan=False),
        encoding="utf-8",
    )
    print(f"Referenz eingefroren: {ziel}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
