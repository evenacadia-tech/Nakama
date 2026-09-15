"""Nulltest im echten Host (Plan S25e (4), Register NAK-286, Manifest F-17 bis F-19).

Vergleicht den Render des Diagnoseprojekts im Auslieferungszustand
(`FL64.exe /R`, geschrieben von tools/fl/laufzeit.ps1) Sample fuer Sample mit
der Quelle des Audioclips, `eq-copilot/kalibration/Testtrack.wav` (Weg R1).

    py -3.13 tools/fl/nulltest.py --renderstatus <render.json> --quelle <Testtrack.wav> \
        --songlaenge-ms <ms> --ergebnis <ergebnis.json>
    py -3.13 tools/fl/nulltest.py --selbsttest [--nur <fall>]

Ablauf: Renderstatus lesen (kein Render -> KEIN_RENDER), `fmt `-Chunks pruefen
(Render 44 100 Hz, 2 Kanaele, PCM 24 Bit oder IEEE-Float 32 Bit, sonst
FORMATFEHLER), N = floor(Songlaenge_ms * 44,1) Frames, Versatzsuche ueber die
volle Songlaenge (Kreuzkorrelation beider Kanaele, |v| < N), Vergleich nach
Ausrichtung ueber die ueberlappenden Renderframes max(0, v) bis
min(N, Quellframes + v) - 1 auf der exakten Float-32-Darstellung beider Dateien.

v ist der Frameindex im Render, an dem Quellframe 0 liegt (Manifest §13.2 P-7):
v > 0 heisst, die Quelle beginnt im Render spaeter; v < 0 heisst, Renderframe 0
entspricht Quellframe -v.

Urteil und Szenario-Exit (F-19, F-24): BITIDENTISCH 0 (v = 0, 0 Abweichungen) ·
VERSATZ 5 (v != 0, 0 Abweichungen) · KETTE 5 (alle Abweichungen erklaert ein
einziger Faktor g bis auf Float-32-Rundung, v roh) · ABWEICHUNG 4 (sonst) ·
KEIN_RENDER 5 · FORMATFEHLER 5. Exit 2 = Werkzeugfehler. Das Ergebnis steht
in `ergebnis.json` (Render-Ordner) fuer die Folgeszenarien (F-26).

Float-32-Rundung fuer KETTE: |r_i - g*q_i| <= 2 * ulp32(g*q_i) + h, h = 2^-24 bei
einem PCM-24-Render (halbe Quantisierungsstufe), sonst 0 - zwei Rundungsstufen
im Float-32-Pfad. Ein Faktor g, der die Abweichungen nicht erklaert (etwa g = 1
bei einem gekippten Bit), ergibt ABWEICHUNG.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import struct
import sys
import tempfile
import traceback
from datetime import datetime, timezone
from pathlib import Path

EXIT_OK, EXIT_WERKZEUG, EXIT_VERFEHLT, EXIT_SZENARIO_VORAUSSETZUNG = 0, 2, 4, 5
RATE = 44100
QUELLE_FRAMES = 5_470_096  # Testtrack.wav, Manifest §0.4
ERGEBNIS_FORMAT = "nakama.laufzeit.nulltest.v1"
PCM, IEEE_FLOAT, EXTENSIBLE = 1, 3, 0xFFFE
HINWEIS_LATENZ = "Offline-Latenz des Plugins 0 (Kanon A1/A16); Clip-Lage oder Latenz trennt nur Weg R2 (Karte U43)"


class NulltestFehler(Exception):
    """Voraussetzung eines Vergleichs fehlt (Szenario-Exit 5) - mit Urteil und Grund."""

    def __init__(self, urteil: str, grund: str):
        super().__init__(grund)
        self.urteil = urteil
        self.grund = grund


# ---------------------------------------------------------------- WAV

class Wav:
    def __init__(self, pfad: Path, tag: int, kanaele: int, rate: int, bits: int, block: int,
                 daten_offset: int, daten_bytes: int):
        self.pfad = pfad
        self.tag = tag
        self.kanaele = kanaele
        self.rate = rate
        self.bits = bits
        self.block = block
        self.daten_offset = daten_offset
        self.daten_bytes = daten_bytes
        self.frames = daten_bytes // block if block else 0

    def beschreibung(self) -> dict:
        art = {PCM: "PCM", IEEE_FLOAT: "IEEE-Float"}.get(self.tag, f"Formattag {self.tag}")
        return {"art": art, "bits": self.bits, "kanaele": self.kanaele, "rate": self.rate,
                "frames": self.frames}


def lies_kopf(pfad: Path) -> Wav:
    """RIFF/WAVE-Kopf: `fmt ` (auch WAVE_FORMAT_EXTENSIBLE) und `data`."""
    with open(pfad, "rb") as f:
        kopf = f.read(12)
        if len(kopf) < 12 or kopf[:4] != b"RIFF" or kopf[8:12] != b"WAVE":
            raise NulltestFehler("FORMATFEHLER", f"{pfad.name}: kein RIFF/WAVE")
        fmt = None
        while True:
            chunk = f.read(8)
            if len(chunk) < 8:
                break
            name, groesse = chunk[:4], struct.unpack("<I", chunk[4:])[0]
            if name == b"fmt ":
                roh = f.read(groesse)
                tag, kanaele, rate, _bps, block, bits = struct.unpack("<HHIIHH", roh[:16])
                if tag == EXTENSIBLE and len(roh) >= 40:
                    tag = struct.unpack("<H", roh[24:26])[0]  # Untertyp-GUID beginnt mit dem Formattag
                fmt = (tag, kanaele, rate, bits, block)
                if groesse % 2:
                    f.read(1)
            elif name == b"data":
                if fmt is None:
                    raise NulltestFehler("FORMATFEHLER", f"{pfad.name}: data vor fmt")
                return Wav(pfad, fmt[0], fmt[1], fmt[2], fmt[3], fmt[4], f.tell(), groesse)
            else:
                f.seek(groesse + (groesse % 2), 1)
    raise NulltestFehler("FORMATFEHLER", f"{pfad.name}: kein fmt- oder data-Chunk")


def render_format_haelt_quelle(w: Wav) -> bool:
    """Jeder 24-Bit-Wert k/2^23 ist in Float 32 exakt darstellbar (F-19)."""
    if w.rate != RATE or w.kanaele != 2:
        return False
    return (w.tag == PCM and w.bits == 24) or (w.tag == IEEE_FLOAT and w.bits == 32)


def lade_float32(w: Wav, frames: int | None = None):
    """Samples als exakte Float-32-Darstellung, Form (frames, 2)."""
    import numpy as np

    n = w.frames if frames is None else min(frames, w.frames)
    with open(w.pfad, "rb") as f:
        f.seek(w.daten_offset)
        roh = f.read(n * w.block)
    if w.tag == IEEE_FLOAT and w.bits == 32:
        return np.frombuffer(roh, dtype="<f4").reshape(-1, w.kanaele).astype(np.float32)
    if w.tag == PCM and w.bits == 24:
        b = np.frombuffer(roh, dtype=np.uint8).reshape(-1, 3).astype(np.int32)
        k = b[:, 0] | (b[:, 1] << 8) | (b[:, 2] << 16)
        k = np.where(k >= 1 << 23, k - (1 << 24), k)
        return (k.astype(np.float32) / np.float32(1 << 23)).reshape(-1, w.kanaele)
    if w.tag == PCM and w.bits == 16:
        k = np.frombuffer(roh, dtype="<i2").astype(np.float32)
        return (k / np.float32(1 << 15)).reshape(-1, w.kanaele)
    raise NulltestFehler("FORMATFEHLER", f"{w.pfad.name}: Format {w.beschreibung()} nicht lesbar")


def sha256_datei(pfad: Path) -> str:
    h = hashlib.sha256()
    with open(pfad, "rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest().upper()


# ---------------------------------------------------------------- Vergleich

def versatz_kandidat(render, quelle, n: int, suche: int | None = None) -> int:
    """Kreuzkorrelation beider Kanaele; v mit |v| < n (volle Songlaenge, P-4)."""
    import numpy as np
    from scipy import fft

    grenze = n if suche is None else min(n, suche)
    m = quelle.shape[0]
    laenge = fft.next_fast_len(n + m - 1)
    summe = np.zeros(laenge)
    for kanal in range(2):
        r = fft.rfft(render[:n, kanal].astype(np.float64), laenge, workers=-1)
        q = fft.rfft(quelle[:, kanal].astype(np.float64), laenge, workers=-1)
        summe += fft.irfft(r * np.conj(q), laenge, workers=-1)
    # summe[k] = sum_i r[i + k] * q[i]; v = k fuer k < n, v = k - laenge fuer negative Versaetze.
    positiv = np.abs(summe[:grenze])
    negativ = np.abs(summe[laenge - (grenze - 1):]) if grenze > 1 else np.zeros(0)
    kp = int(np.argmax(positiv))
    if negativ.size and float(negativ.max()) > float(positiv[kp]):
        return int(np.argmax(negativ)) - (grenze - 1)
    return kp


def vergleiche(render, quelle, n: int, render_pcm24: bool, suche: int | None = None) -> dict:
    """Urteil nach Ausrichtung um den gefundenen Versatz v (F-19)."""
    import numpy as np

    m = quelle.shape[0]
    v = versatz_kandidat(render, quelle, n, suche)
    i0, i1 = max(0, v), min(n, m + v)
    if i1 <= i0:
        return {"urteil": "ABWEICHUNG", "v": v, "grund": "keine ueberlappenden Frames",
                "ueberlappend": {"von": i0, "bis": i1, "frames": 0}, "abweichungen": None}
    r = np.ascontiguousarray(render[i0:i1])
    q = np.ascontiguousarray(quelle[i0 - v:i1 - v])
    ungleich = r != q
    abweichungen = int(np.count_nonzero(ungleich))
    ergebnis = {
        "v": v,
        "ueberlappend": {"von": i0, "bis": i1, "frames": i1 - i0},
        "abweichungen": abweichungen,
        "sha256_render_bereich": hashlib.sha256(r.astype("<f4").tobytes()).hexdigest().upper(),
        "sha256_quelle_bereich": hashlib.sha256(q.astype("<f4").tobytes()).hexdigest().upper(),
        "spitzen_bereich": {"render": [float(np.max(np.abs(r[:, c]))) for c in range(2)],
                            "quelle": [float(np.max(np.abs(q[:, c]))) for c in range(2)]},
        "erste": None,
        "letzte": None,
        "g": None,
        "g_db": None,
    }
    if abweichungen:
        idx = np.argwhere(ungleich)
        ergebnis["erste"] = {"frame": int(idx[0][0]) + i0, "kanal": int(idx[0][1])}
        ergebnis["letzte"] = {"frame": int(idx[-1][0]) + i0, "kanal": int(idx[-1][1])}
    if abweichungen == 0:
        ergebnis["urteil"] = "BITIDENTISCH" if v == 0 else "VERSATZ"
        if v == 0:
            ergebnis["g"], ergebnis["g_db"] = 1.0, 0.0
        return ergebnis

    rd, qd = r.astype(np.float64), q.astype(np.float64)
    nenner = float(np.sum(qd * qd))
    g = float(np.sum(rd * qd) / nenner) if nenner > 0 else 0.0
    produkt = (g * qd).astype(np.float32)
    toleranz = 2.0 * np.spacing(np.abs(produkt)).astype(np.float64) + (2.0 ** -24 if render_pcm24 else 0.0)
    rest = np.abs(rd - g * qd)
    erklaert = bool(np.all(rest <= toleranz))
    if erklaert:
        ergebnis["urteil"] = "KETTE"
        ergebnis["g"] = g
        ergebnis["g_db"] = 20.0 * math.log10(abs(g)) if g != 0 else None
        if g == 0:
            ergebnis["grund"] = "Render ist Stille (g = 0)"
    else:
        ergebnis["urteil"] = "ABWEICHUNG"
        ergebnis["g"] = g
        ergebnis["rest_max"] = float(rest.max())
        ergebnis["rest_ueber_toleranz"] = int(np.count_nonzero(rest > toleranz))
    return ergebnis


EXIT_JE_URTEIL = {"BITIDENTISCH": EXIT_OK, "VERSATZ": EXIT_SZENARIO_VORAUSSETZUNG,
                  "KETTE": EXIT_SZENARIO_VORAUSSETZUNG, "ABWEICHUNG": EXIT_VERFEHLT,
                  "KEIN_RENDER": EXIT_SZENARIO_VORAUSSETZUNG, "FORMATFEHLER": EXIT_SZENARIO_VORAUSSETZUNG}


def nulltest(renderstatus: Path, quelle_pfad: Path, songlaenge_ms: int) -> dict:
    """Die ganze Rechnung ohne Dateiausgabe; das Urteil steht in `urteil`."""
    status = None
    if renderstatus.exists():
        try:
            status = json.loads(renderstatus.read_text(encoding="utf-8"))
        except (OSError, ValueError) as e:
            status = {"grund": f"render.json unlesbar: {e}"}
    aus: dict = {"format": ERGEBNIS_FORMAT, "songlaenge_ms": songlaenge_ms, "renderstatus": status,
                 "hinweis": HINWEIS_LATENZ}
    if not isinstance(status, dict) or not status.get("datei") or status.get("grund"):
        grund = (status or {}).get("grund") if isinstance(status, dict) else None
        raise NulltestFehler("KEIN_RENDER", f"kein Render: {grund or 'render.json fehlt oder nennt keine Datei'}")
    render_pfad = Path(status["datei"])
    if not render_pfad.exists():
        raise NulltestFehler("KEIN_RENDER", f"kein Render: {render_pfad} fehlt")

    q = lies_kopf(quelle_pfad)
    if not (q.tag == PCM and q.bits == 24 and q.kanaele == 2 and q.rate == RATE):
        raise NulltestFehler("FORMATFEHLER", f"Quelle hat nicht das Format der Kalibrierquelle: {q.beschreibung()}")
    w = lies_kopf(render_pfad)
    aus["render"] = {"pfad": str(render_pfad), "sha256_datei": sha256_datei(render_pfad), "format": w.beschreibung()}
    aus["quelle"] = {"pfad": str(quelle_pfad), "sha256_datei": sha256_datei(quelle_pfad), "format": q.beschreibung()}
    if not render_format_haelt_quelle(w):
        raise NulltestFehler("FORMATFEHLER", f"Renderformat haelt die Quelle nicht verlustfrei: {w.beschreibung()}")
    n = songlaenge_ms * RATE // 1000  # floor(ms * 44,1) ganzzahlig; 45 696 ms -> 2 015 193 Frames
    aus["N"] = n
    if n <= 0:
        raise NulltestFehler("FORMATFEHLER", f"Songlaenge {songlaenge_ms} ms ergibt kein N")
    if w.frames < n:
        raise NulltestFehler("FORMATFEHLER", f"Render kuerzer als die Songlaenge: {w.frames} < N = {n} Frames")

    import numpy as np

    render = lade_float32(w, n)
    quelle = lade_float32(q)
    aus["spitzen_datei"] = {"render": [float(np.max(np.abs(lade_float32(w)[:, c]))) for c in range(2)],
                            "quelle": [float(np.max(np.abs(quelle[:, c]))) for c in range(2)]}
    aus.update(vergleiche(render, quelle, n, render_pcm24=(w.tag == PCM)))
    return aus


def schreibe_ergebnis(pfad: Path, ergebnis: dict) -> None:
    ergebnis["erzeugt_utc"] = datetime.now(timezone.utc).isoformat(timespec="seconds")
    pfad.parent.mkdir(parents=True, exist_ok=True)
    tmp = pfad.with_name(pfad.name + ".tmp")
    tmp.write_text(json.dumps(ergebnis, ensure_ascii=False, indent=1), encoding="utf-8")
    tmp.replace(pfad)


def fahre(renderstatus: Path, quelle: Path, songlaenge_ms: int, ergebnis_pfad: Path) -> tuple[int, dict]:
    try:
        ergebnis = nulltest(renderstatus, quelle, songlaenge_ms)
    except NulltestFehler as e:
        ergebnis = {"format": ERGEBNIS_FORMAT, "songlaenge_ms": songlaenge_ms, "urteil": e.urteil,
                    "grund": e.grund, "v": None, "g": None, "g_db": None, "hinweis": HINWEIS_LATENZ}
    ergebnis["exit"] = EXIT_JE_URTEIL[ergebnis["urteil"]]
    schreibe_ergebnis(ergebnis_pfad, ergebnis)
    return ergebnis["exit"], ergebnis


def rohzeile(e: dict) -> str:
    teile = [f"Urteil {e['urteil']}", f"Exit {e.get('exit')}"]
    for schluessel in ("grund", "N", "v", "abweichungen", "g", "g_db"):
        if e.get(schluessel) is not None:
            teile.append(f"{schluessel} {e[schluessel]}")
    if e.get("ueberlappend"):
        u = e["ueberlappend"]
        teile.append(f"ueberlappend {u['frames']} Frames [{u['von']}, {u['bis']})")
    for schluessel in ("erste", "letzte"):
        if e.get(schluessel):
            teile.append(f"{schluessel} Abweichung Frame {e[schluessel]['frame']} Kanal {e[schluessel]['kanal']}")
    for schluessel in ("sha256_render_bereich", "sha256_quelle_bereich"):
        if e.get(schluessel):
            teile.append(f"{schluessel} {e[schluessel]}")
    if e.get("spitzen_datei"):
        teile.append(f"Stichprobenspitzen Datei {e['spitzen_datei']}")
    if e.get("spitzen_bereich"):
        teile.append(f"Stichprobenspitzen Bereich {e['spitzen_bereich']}")
    if e.get("render"):
        teile.append(f"Render {e['render']['format']} SHA-256 {e['render']['sha256_datei']}")
    if e["urteil"] in ("VERSATZ", "KETTE"):
        teile.append(e.get("hinweis", HINWEIS_LATENZ))
    return " · ".join(str(t) for t in teile)


# ---------------------------------------------------------------- Selbsttest

def _schreibe_wav(pfad: Path, daten, art: str, rate: int = RATE) -> None:
    """Attrappen-WAV: daten (frames, kanaele) als float-Werte k/2^23."""
    import numpy as np

    kanaele = daten.shape[1]
    if art == "pcm24":
        k = np.round(daten.astype(np.float64) * (1 << 23)).astype(np.int32)
        k = np.where(k < 0, k + (1 << 24), k).astype(np.uint32)
        b = np.stack([k & 0xFF, (k >> 8) & 0xFF, (k >> 16) & 0xFF], axis=-1).astype(np.uint8)
        roh, tag, bits = b.tobytes(), PCM, 24
    elif art == "float32":
        roh, tag, bits = daten.astype("<f4").tobytes(), IEEE_FLOAT, 32
    elif art == "pcm16":
        k = np.round(daten.astype(np.float64) * (1 << 15)).astype("<i2")
        roh, tag, bits = k.tobytes(), PCM, 16
    else:
        raise ValueError(art)
    block = kanaele * bits // 8
    fmt = struct.pack("<HHIIHH", tag, kanaele, rate, rate * block, block, bits)
    inhalt = b"WAVE" + b"fmt " + struct.pack("<I", len(fmt)) + fmt + b"data" + struct.pack("<I", len(roh)) + roh
    pfad.write_bytes(b"RIFF" + struct.pack("<I", len(inhalt)) + inhalt)


def _quelle(frames: int, seed: int = 286):
    import numpy as np

    rng = np.random.default_rng(seed)
    k = rng.integers(-(1 << 22), 1 << 22, size=(frames, 2), dtype=np.int32)
    return (k.astype(np.float32) / np.float32(1 << 23)).astype(np.float32)


class Selbsttest:
    def __init__(self, ordner: Path):
        self.ordner = ordner
        self.fehler: list[str] = []

    def pruefe(self, bedingung: bool, text: str) -> None:
        if not bedingung:
            self.fehler.append(text)

    def fall(self, name: str, render, art: str, songlaenge_ms: int, quelle=None, status: dict | None = None,
             rate: int = RATE):
        import numpy as np

        q = _quelle(400_000) if quelle is None else quelle
        qpfad = self.ordner / "quelle.wav"
        if not qpfad.exists() or quelle is not None:
            _schreibe_wav(qpfad, q, "pcm24")
        rpfad = self.ordner / f"{name}.wav"
        if render is not None:
            _schreibe_wav(rpfad, np.asarray(render), art, rate)
        spfad = self.ordner / f"{name}-render.json"
        s = {"datei": str(rpfad) if render is not None else None} if status is None else status
        spfad.write_text(json.dumps(s), encoding="utf-8")
        return fahre(spfad, qpfad, songlaenge_ms, self.ordner / f"{name}-ergebnis.json")


MS_TEST = 4535
N_TEST = MS_TEST * RATE // 1000  # floor(ms * 44,1) = 199 993 Frames


def fall_identisch_null_abweichungen(t: Selbsttest) -> None:
    """M-62: identisch -> BITIDENTISCH; ein gekipptes LSB -> genau 1 Abweichung an k."""
    import numpy as np

    q = _quelle(400_000)
    code, e = t.fall("identisch", q[:N_TEST], "pcm24", MS_TEST, quelle=q)
    t.pruefe(e.get("N") == N_TEST, f"N {e.get('N')} statt {N_TEST}")
    t.pruefe(code == 0 and e["urteil"] == "BITIDENTISCH" and e["v"] == 0 and e["abweichungen"] == 0,
             f"identischer Render: Urteil {e['urteil']}, v {e.get('v')}, Abweichungen {e.get('abweichungen')}, Exit {code}")
    k = 123_457
    r = q[:N_TEST].copy()
    r[k, 0] = q[k, 0] + np.float32(2.0 ** -23)
    code, e = t.fall("lsb", r, "float32", MS_TEST, quelle=q)
    t.pruefe(e["urteil"] == "ABWEICHUNG" and code == 4,
             f"gekipptes LSB: Urteil {e['urteil']} Exit {code} statt ABWEICHUNG 4")
    t.pruefe(e.get("abweichungen") == 1 and (e.get("erste") or {}).get("frame") == k
             and (e.get("letzte") or {}).get("frame") == k,
             f"gekipptes LSB an {k}: Abweichungen {e.get('abweichungen')}, erste {e.get('erste')}, letzte {e.get('letzte')}")
    verschoben = np.concatenate([np.zeros((1, 2), np.float32), q[:N_TEST - 1]])
    code, e = t.fall("plus1", verschoben, "pcm24", MS_TEST, quelle=q)
    t.pruefe(e["urteil"] == "VERSATZ" and e["v"] == 1 and e["abweichungen"] == 0 and code == 5,
             f"ein Nullframe vor der Quelle: Urteil {e['urteil']} v {e.get('v')} Abweichungen {e.get('abweichungen')} Exit {code}")


def fall_versatz_ist_voraussetzung(t: Selbsttest) -> None:
    """M-75: v = +1, -1, +75 600, -75 600 mit bekannter Richtung aus der Konstruktion."""
    import numpy as np

    q = _quelle(400_000)
    m = q.shape[0]
    for v in (1, -1, 75_600, -75_600):
        if v > 0:
            r = np.concatenate([np.zeros((v, 2), np.float32), q[:N_TEST - v]])
            erwartet_frames = N_TEST - v
        else:
            r = q[-v:-v + N_TEST]
            erwartet_frames = min(N_TEST, m + v)
        code, e = t.fall(f"versatz{v}", r, "pcm24", MS_TEST, quelle=q)
        t.pruefe(e["urteil"] == "VERSATZ" and code == 5,
                 f"v = {v}: Urteil {e['urteil']} Exit {code} statt VERSATZ 5")
        t.pruefe(e.get("v") == v, f"v = {v}: gemeldet v = {e.get('v')} (Vorzeichen nach P-7)")
        t.pruefe(e.get("abweichungen") == 0, f"v = {v}: {e.get('abweichungen')} Abweichungen nach Ausrichtung")
        u = e.get("ueberlappend") or {}
        t.pruefe(u.get("frames") == erwartet_frames and u.get("von") == max(0, v),
                 f"v = {v}: ueberlappend {u} statt {erwartet_frames} Frames ab {max(0, v)}")


def fall_konstanter_faktor_ist_voraussetzung(t: Selbsttest) -> None:
    """M-63: Render x 0,5 -> KETTE (Exit 5); x 0,5 mit einem gestoerten Sample -> ABWEICHUNG (Exit 4)."""
    import numpy as np

    q = _quelle(400_000)
    halb = (q[:N_TEST].astype(np.float64) * 0.5).astype(np.float32)
    code, e = t.fall("kette", halb, "float32", MS_TEST, quelle=q)
    t.pruefe(e["urteil"] == "KETTE" and code == 5,
             f"Faktor 0,5: Urteil {e['urteil']} Exit {code} statt KETTE 5")
    t.pruefe(e.get("g") is not None and e.get("g_db") is not None
             and abs(e["g"] - 0.5) < 1e-9 and abs(e["g_db"] - 20 * math.log10(0.5)) < 1e-6,
             f"Faktor 0,5: g {e.get('g')} g_db {e.get('g_db')}")
    gestoert = halb.copy()
    gestoert[54_321, 1] += np.float32(1e-3)
    code, e = t.fall("kette-gestoert", gestoert, "float32", MS_TEST, quelle=q)
    t.pruefe(e["urteil"] == "ABWEICHUNG" and code == 4,
             f"Faktor 0,5 mit Stoerung: Urteil {e['urteil']} Exit {code} statt ABWEICHUNG 4")


def fall_format_ist_voraussetzung(t: Selbsttest) -> None:
    """M-61: 16 Bit, Mono, andere Rate -> FORMATFEHLER mit Szenario-Exit 5, nie Exit 4."""
    import numpy as np

    q = _quelle(400_000)
    for name, render, art, rate in (("pcm16", q[:N_TEST], "pcm16", RATE),
                                    ("mono", q[:N_TEST, :1], "pcm24", RATE),
                                    ("rate48k", q[:N_TEST], "pcm24", 48_000)):
        code, e = t.fall(name, render, art, MS_TEST, quelle=q, rate=rate)
        t.pruefe(e["urteil"] == "FORMATFEHLER" and code == 5,
                 f"{name}: Urteil {e['urteil']} Exit {code} statt FORMATFEHLER 5")
        t.pruefe("Renderformat haelt die Quelle nicht verlustfrei" in (e.get("grund") or ""),
                 f"{name}: Grund {e.get('grund')!r}")
    _ = np  # numpy wird fuer die Attrappen gebraucht


def fall_render_status(t: Selbsttest) -> None:
    """M-60 (Szenario-Haelfte): ohne Renderdatei endet nulltest-host mit Szenario-Exit 5."""
    q = _quelle(400_000)
    for name, status in (("ohne-datei", {"datei": None, "grund": "keine Datei: Nakama-Diagnose.wav fehlt nach Exit 0"}),
                         ("haengt", {"datei": None, "grund": "Prozess haengt (Frist 600 s), Fenster 'Cloud path warning'"})):
        code, e = t.fall(name, None, "pcm24", MS_TEST, quelle=q, status=status)
        t.pruefe(e["urteil"] == "KEIN_RENDER" and code == 5,
                 f"{name}: Urteil {e['urteil']} Exit {code} statt KEIN_RENDER 5")
        t.pruefe(status["grund"] in (e.get("grund") or ""), f"{name}: Grund {e.get('grund')!r} nennt den gemessenen Grund nicht")
    spfad = t.ordner / "fehlt-render.json"
    code, e = fahre(spfad, t.ordner / "quelle.wav", MS_TEST, t.ordner / "fehlt-ergebnis.json")
    t.pruefe(e["urteil"] == "KEIN_RENDER" and code == 5, f"render.json fehlt: Urteil {e['urteil']} Exit {code}")


FAELLE = [
    ("M-62", "identisch_null_abweichungen", fall_identisch_null_abweichungen),
    ("M-75", "versatz_ist_voraussetzung", fall_versatz_ist_voraussetzung),
    ("M-63", "konstanter_faktor_ist_voraussetzung", fall_konstanter_faktor_ist_voraussetzung),
    ("M-61", "format_ist_voraussetzung", fall_format_ist_voraussetzung),
    ("M-60", "render_status", fall_render_status),
]


def selbsttest(nur: list[str]) -> int:
    rot = 0
    gelaufen = 0
    with tempfile.TemporaryDirectory(prefix="nak286-nulltest-") as tmp:
        for zeile, name, funktion in FAELLE:
            if nur and name not in nur and zeile not in nur:
                continue
            gelaufen += 1
            t = Selbsttest(Path(tmp))
            try:
                funktion(t)
            except Exception as e:  # noqa: BLE001 - eine Ausnahme ist ein roter Fall mit Ort, kein Werkzeugfehler
                ort = traceback.extract_tb(e.__traceback__)[-1]
                t.fehler.append(f"Ausnahme {type(e).__name__}: {e} (nulltest.py:{ort.lineno})")
            if t.fehler:
                rot += 1
                print(f"[ROT] {zeile} {name}: " + " | ".join(t.fehler))
            else:
                print(f"[ok] {zeile} {name}")
    if gelaufen == 0:
        print(f"SELBSTTEST nulltest.py: kein Fall passt zu {nur}")
        return EXIT_WERKZEUG
    print(f"SELBSTTEST nulltest.py: {gelaufen - rot} ok, {rot} rot")
    return EXIT_VERFEHLT if rot else EXIT_OK


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--renderstatus", type=Path)
    ap.add_argument("--quelle", type=Path)
    ap.add_argument("--songlaenge-ms", type=int)
    ap.add_argument("--ergebnis", type=Path)
    ap.add_argument("--selbsttest", action="store_true")
    ap.add_argument("--nur", action="append", default=[])
    args = ap.parse_args(argv)
    try:
        if args.selbsttest:
            return selbsttest(args.nur)
        if not (args.renderstatus and args.quelle and args.songlaenge_ms is not None and args.ergebnis):
            ap.error("--renderstatus, --quelle, --songlaenge-ms und --ergebnis oder --selbsttest")
        code, ergebnis = fahre(args.renderstatus, args.quelle, args.songlaenge_ms, args.ergebnis)
        print("NULLTEST " + rohzeile(ergebnis))
        return code
    except Exception:  # noqa: BLE001 - Werkzeugfehler ist ein eigener Ausgang
        traceback.print_exc()
        print("NULLTEST WERKZEUGFEHLER")
        return EXIT_WERKZEUG


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
