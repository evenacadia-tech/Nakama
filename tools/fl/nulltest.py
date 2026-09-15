"""Nulltest im echten Host (Plan S25e (4), Register NAK-286, Manifest F-17 bis F-19, M-64, P-21).

Vergleicht den Render des Diagnoseprojekts im Auslieferungszustand
(`FL64.exe /R`, geschrieben von tools/fl/laufzeit.ps1) Sample fuer Sample mit
der Quelle des Audioclips, `eq-copilot/kalibration/Testtrack.wav` (Weg R1).
Weg R2 (M-64, P-21) vergleicht die Renders der Referenzprojekte aus Karte U43,
die der Runner in ihren eigenen Ordner `render\\referenz\\<Projekt>\\` schreibt.

    py -3.13 tools/fl/nulltest.py --renderstatus <render.json> --quelle <Testtrack.wav> \
        --songlaenge-ms <ms> --ergebnis <ergebnis.json>
    py -3.13 tools/fl/nulltest.py --vergleich verarbeitung_ein --renderstatus <referenz\\...\\render.json> \
        --quelle <Testtrack.wav> --songlaenge-ms <ms> --ergebnis <referenz\\...\\ergebnis.json>
    py -3.13 tools/fl/nulltest.py --vergleich ohne_slots --renderstatus <referenz\\...\\render.json> \
        --auslieferung <render\\ergebnis.json> --songlaenge-ms <ms> --ergebnis <referenz\\...\\ergebnis.json>
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

Weg R2, Urteil und Szenario-Exit des Schritts: GEMESSEN 0 · VERFEHLT 4 ·
VORAUSSETZUNG 5 (auch KEIN_RENDER und FORMATFEHLER). Das Ergebnis steht in der
`ergebnis.json` im Ordner des Referenzprojekts, nie in der des
Auslieferungsrenders - ein Referenzschritt loest keinen Rueckweg aus (M-65).
`verarbeitung_ein`: derselbe Vergleich gegen die Quelle wie R1, umgekehrt
bewertet - Abweichungen nach Ausrichtung GEMESSEN (auch mit v != 0 oder Faktor
g), 0 Abweichungen (BITIDENTISCH, VERSATZ) VERFEHLT, der Vergleich waere blind.
`ohne_slots`: SHA-256 des Datenbereichs des Referenzrenders ueber denselben
Framebereich und in derselben Float-32-Darstellung wie `sha256_render_bereich`
der ergebnis.json des Auslieferungsrenders - gleich GEMESSEN, ungleich
VERFEHLT (mit Abweichungszahl und erster Abweichung gegen den
Auslieferungsrender); ohne diese ergebnis.json oder ohne BITIDENTISCH fehlt die
Voraussetzung.
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


# ---------------------------------------------------------------- Weg R2: Referenzschritte (M-64, P-21)

REFERENZ_FORMAT = "nakama.laufzeit.nulltest.referenz.v1"
REFERENZ_VERGLEICHE = ("verarbeitung_ein", "ohne_slots")
EXIT_JE_SCHRITT = {"GEMESSEN": EXIT_OK, "VERFEHLT": EXIT_VERFEHLT, "VORAUSSETZUNG": EXIT_SZENARIO_VORAUSSETZUNG}


def lies_renderstatus(pfad: Path) -> dict | None:
    """Renderstatus des Runners (nakama.laufzeit.render.v1) als dict; None ohne lesbare Datei."""
    try:
        status = json.loads(pfad.read_text(encoding="utf-8"))
    except (OSError, ValueError):
        return None
    return status if isinstance(status, dict) else None


def renderangaben(status: dict | None) -> dict:
    """Projektname, SHA-256 des Projekts und Renderdauer aus dem Renderstatus (P-21 (f))."""
    s = status or {}
    projekt = s.get("projekt")
    return {"projekt": Path(projekt).name if isinstance(projekt, str) and projekt else None,
            "sha256_projekt": s.get("sha256_projekt"), "renderdauer_s": s.get("dauer_s")}


def referenzrender(status: dict | None) -> Path:
    """Renderdatei eines Referenzschritts; ohne Render KEIN_RENDER mit dem Grund aus dem Renderstatus."""
    if not status or not status.get("datei") or status.get("grund"):
        grund = (status or {}).get("grund")
        raise NulltestFehler("KEIN_RENDER", f"kein Render: {grund or 'render.json fehlt oder nennt keine Datei'}")
    pfad = Path(status["datei"])
    if not pfad.exists():
        raise NulltestFehler("KEIN_RENDER", f"kein Render: {pfad} fehlt")
    return pfad


def bewerte_verarbeitung(e: dict) -> dict:
    """M-64 (c), P-21: der Render mit eingeschalteter Verarbeitung muss nach Ausrichtung von der Quelle abweichen.
    Abweichungszahl groesser 0 -> GEMESSEN, auch mit v != 0 oder einem Faktor g (beide in der Rohzeile); 0 Abweichungen
    (BITIDENTISCH, VERSATZ) -> VERFEHLT, der Vergleich waere blind; ohne ueberlappende Frames kein Vergleich (Exit 5)."""
    e["befund"] = e["urteil"]
    g = e.get("g")
    if e.get("g_db") is None and isinstance(g, float) and math.isfinite(g) and g != 0:
        e["g_db"] = 20.0 * math.log10(abs(g))
    abweichungen = e.get("abweichungen")
    if abweichungen is None:
        e["urteil"] = "VORAUSSETZUNG"
        e["grund"] = f"kein Vergleich: {e.get('grund') or 'keine ueberlappenden Frames'}"
    elif abweichungen > 0:
        e["urteil"] = "GEMESSEN"
    else:
        e["urteil"] = "VERFEHLT"
        e["grund"] = (f"Render mit eingeschalteter Verarbeitung nach Ausrichtung wertgleich zur Quelle ({e['befund']}, "
                      f"v {e.get('v')}): der Vergleich waere blind")
    return e


def lies_auslieferung(pfad: Path, n: int) -> dict:
    """ergebnis.json des Auslieferungsrenders als Bezug fuer ohne_slots (P-21 (d)): vorhanden, Format von R1, BITIDENTISCH,
    fuer dasselbe N, mit Framebereich und sha256_render_bereich; sonst fehlt die Voraussetzung (Exit 5)."""
    try:
        a = json.loads(pfad.read_text(encoding="utf-8"))
    except OSError:
        raise NulltestFehler("OHNE_AUSLIEFERUNG", f"ergebnis.json des Auslieferungsrenders fehlt: {pfad}") from None
    except ValueError as e:
        raise NulltestFehler("OHNE_AUSLIEFERUNG", f"ergebnis.json des Auslieferungsrenders unlesbar: {e}") from None
    if not isinstance(a, dict) or a.get("format") != ERGEBNIS_FORMAT:
        raise NulltestFehler("OHNE_AUSLIEFERUNG", f"ergebnis.json des Auslieferungsrenders ist fremd: {pfad}")
    if a.get("urteil") != "BITIDENTISCH":
        raise NulltestFehler("OHNE_AUSLIEFERUNG",
                             f"Auslieferungsschritt endete {a.get('urteil')}, nicht BITIDENTISCH ({a.get('grund') or 'ohne Grund'}): "
                             "die Zusage vergleicht gegen einen bitidentischen Auslieferungsrender")
    u = a.get("ueberlappend") if isinstance(a.get("ueberlappend"), dict) else {}
    von, bis, sha = u.get("von"), u.get("bis"), a.get("sha256_render_bereich")
    if not (isinstance(von, int) and isinstance(bis, int) and 0 <= von < bis) or not (
            isinstance(sha, str) and len(sha) == 64 and all(z in "0123456789ABCDEF" for z in sha)):
        raise NulltestFehler("OHNE_AUSLIEFERUNG", f"ergebnis.json des Auslieferungsrenders ohne Framebereich oder SHA-256: {pfad}")
    if a.get("N") != n:
        raise NulltestFehler("OHNE_AUSLIEFERUNG",
                             f"ergebnis.json des Auslieferungsrenders gilt fuer N = {a.get('N')}, die Songlaenge ergibt N = {n}")
    return a


def abweichungen_gegen_auslieferung(r, a: dict, von: int, bis: int) -> dict:
    """Abweichungszahl, erste und letzte Abweichung und g des Referenzrenders gegen den Auslieferungsrender ueber denselben
    Bereich, bitgenau auf der Float-32-Darstellung wie der SHA-256 - nur gegen die Renderdatei, die der Auslieferungsschritt
    verglichen hat (SHA-256 der Datei aus seiner ergebnis.json); sonst steht der Grund in der Rohzeile."""
    import numpy as np

    angaben = a.get("render") if isinstance(a.get("render"), dict) else {}
    pfad = Path(angaben["pfad"]) if isinstance(angaben.get("pfad"), str) else None
    if pfad is None or not pfad.exists():
        return {"abweichungen_grund": f"Renderdatei des Auslieferungsrenders fehlt: {angaben.get('pfad')}"}
    if sha256_datei(pfad) != angaben.get("sha256_datei"):
        return {"abweichungen_grund": "Renderdatei des Auslieferungsrenders seit ihrem Vergleich veraendert (SHA-256 der Datei)"}
    try:
        w = lies_kopf(pfad)
    except NulltestFehler as e:
        return {"abweichungen_grund": f"Renderdatei des Auslieferungsrenders unlesbar: {e.grund}"}
    if not render_format_haelt_quelle(w) or w.frames < bis:
        return {"abweichungen_grund": f"Renderdatei des Auslieferungsrenders nicht vergleichbar: {w.beschreibung()}"}
    d = np.ascontiguousarray(lade_float32(w, bis)[von:bis])
    verschieden = r.view(np.uint32) != d.view(np.uint32)
    anzahl = int(np.count_nonzero(verschieden))
    aus: dict = {"abweichungen": anzahl, "erste": None, "letzte": None, "g": None, "g_db": None}
    if anzahl:
        idx = np.argwhere(verschieden)
        aus["erste"] = {"frame": int(idx[0][0]) + von, "kanal": int(idx[0][1])}
        aus["letzte"] = {"frame": int(idx[-1][0]) + von, "kanal": int(idx[-1][1])}
    rd, dd = r.astype(np.float64), d.astype(np.float64)
    nenner = float(np.sum(dd * dd))
    if nenner > 0:
        aus["g"] = float(np.sum(rd * dd) / nenner)
        aus["g_db"] = 20.0 * math.log10(abs(aus["g"])) if aus["g"] != 0 else None
    return aus


def ohne_slots(renderstatus: Path, auslieferung: Path, songlaenge_ms: int) -> dict:
    """M-64 (d), P-21: der Render des Referenzprojekts ohne Nakama-Slots traegt dieselben SHA-256 der Datenbereiche wie der
    Render im Auslieferungszustand. Gehasht wird derselbe Framebereich in derselben Float-32-Darstellung wie
    sha256_render_bereich der ergebnis.json des Auslieferungsrenders (bei v = 0 die Frames [0, N)); gleich -> GEMESSEN,
    ungleich -> VERFEHLT, mit Abweichungszahl und erster Abweichung gegen den Auslieferungsrender in der Rohzeile. Keine
    Versatzsuche: der Bezug ist bitidentisch."""
    import numpy as np

    status = lies_renderstatus(renderstatus)
    render_pfad = referenzrender(status)
    n = songlaenge_ms * RATE // 1000
    a = lies_auslieferung(auslieferung, n)
    von, bis = a["ueberlappend"]["von"], a["ueberlappend"]["bis"]
    w = lies_kopf(render_pfad)
    if not render_format_haelt_quelle(w):
        raise NulltestFehler("FORMATFEHLER", f"Renderformat haelt die Quelle nicht verlustfrei: {w.beschreibung()}")
    if w.frames < bis:
        raise NulltestFehler("FORMATFEHLER", f"Render kuerzer als der verglichene Bereich: {w.frames} < {bis} Frames")
    r = np.ascontiguousarray(lade_float32(w, bis)[von:bis])
    sha = hashlib.sha256(r.astype("<f4").tobytes()).hexdigest().upper()
    gleich = sha == a["sha256_render_bereich"]
    aus: dict = {"renderstatus": status,
                 "render": {"pfad": str(render_pfad), "sha256_datei": sha256_datei(render_pfad), "format": w.beschreibung()},
                 "auslieferung": {"ergebnis": str(auslieferung), "render": a.get("render")},
                 "N": n, "v": a.get("v"), "ueberlappend": {"von": von, "bis": bis, "frames": bis - von},
                 "sha256_render_bereich": sha, "sha256_auslieferung_bereich": a["sha256_render_bereich"],
                 "spitzen_bereich": {"render": [float(np.max(np.abs(r[:, c]))) for c in range(2)],
                                     "auslieferung": (a.get("spitzen_bereich") or {}).get("render")}}
    aus.update(abweichungen_gegen_auslieferung(r, a, von, bis))
    if gleich:
        aus["urteil"], aus["befund"] = "GEMESSEN", "GLEICH"
    else:
        aus["urteil"], aus["befund"] = "VERFEHLT", "UNGLEICH"
        aus["grund"] = (f"SHA-256 des Datenbereichs [{von}, {bis}) {sha} ungleich {a['sha256_render_bereich']} des "
                        "Auslieferungsrenders: der Render ohne Nakama-Slots traegt nicht dieselben Daten")
    return aus


def fahre_referenz(vergleich: str, renderstatus: Path, songlaenge_ms: int, ergebnis_pfad: Path,
                   quelle: Path | None = None, auslieferung: Path | None = None) -> tuple[int, dict]:
    """Ein Referenzschritt (M-64, P-21): Urteil GEMESSEN, VERFEHLT oder VORAUSSETZUNG mit Exit 0, 4 oder 5 in der
    ergebnis.json im Ordner des Referenzprojekts - nie in der des Auslieferungsrenders (F-26; kein Rueckweg, M-65)."""
    if vergleich not in REFERENZ_VERGLEICHE:
        raise ValueError(f"unbekannter Vergleich {vergleich!r}")
    status = lies_renderstatus(renderstatus)
    try:
        if vergleich == "verarbeitung_ein":
            if quelle is None:
                raise ValueError("verarbeitung_ein braucht die Quelle")
            ergebnis = bewerte_verarbeitung(nulltest(renderstatus, quelle, songlaenge_ms))
            ergebnis.pop("hinweis", None)
        else:
            if auslieferung is None:
                raise ValueError("ohne_slots braucht die ergebnis.json des Auslieferungsrenders")
            ergebnis = ohne_slots(renderstatus, auslieferung, songlaenge_ms)
    except NulltestFehler as e:
        ergebnis = {"urteil": "VORAUSSETZUNG", "befund": e.urteil, "grund": e.grund, "N": None, "v": None, "g": None,
                    "g_db": None, "renderstatus": status}
    ergebnis.update(format=REFERENZ_FORMAT, vergleich=vergleich, songlaenge_ms=songlaenge_ms, **renderangaben(status))
    ergebnis["exit"] = EXIT_JE_SCHRITT[ergebnis["urteil"]]
    schreibe_ergebnis(ergebnis_pfad, ergebnis)
    return ergebnis["exit"], ergebnis


def rohzeile(e: dict) -> str:
    teile = [f"Urteil {e['urteil']}", f"Exit {e.get('exit')}"]
    if e.get("vergleich"):
        teile = [f"Vergleich {e['vergleich']}", *teile, f"Befund {e.get('befund')}"]
    for schluessel in ("grund", "N", "v", "abweichungen", "g", "g_db"):
        if e.get(schluessel) is not None:
            teile.append(f"{schluessel} {e[schluessel]}")
    if e.get("abweichungen_grund"):
        teile.append(f"Abweichungszahl nicht bestimmt: {e['abweichungen_grund']}")
    if e.get("ueberlappend"):
        u = e["ueberlappend"]
        teile.append(f"ueberlappend {u['frames']} Frames [{u['von']}, {u['bis']})")
    for schluessel in ("erste", "letzte"):
        if e.get(schluessel):
            teile.append(f"{schluessel} Abweichung Frame {e[schluessel]['frame']} Kanal {e[schluessel]['kanal']}")
    for schluessel in ("sha256_render_bereich", "sha256_quelle_bereich", "sha256_auslieferung_bereich"):
        if e.get(schluessel):
            teile.append(f"{schluessel} {e[schluessel]}")
    if e.get("spitzen_datei"):
        teile.append(f"Stichprobenspitzen Datei {e['spitzen_datei']}")
    if e.get("spitzen_bereich"):
        teile.append(f"Stichprobenspitzen Bereich {e['spitzen_bereich']}")
    if e.get("render"):
        teile.append(f"Render {e['render']['format']} SHA-256 {e['render']['sha256_datei']}")
    if e.get("vergleich"):
        teile += [f"Projekt {e.get('projekt')}", f"SHA-256 Projekt {e.get('sha256_projekt')}", f"Renderdauer {e.get('renderdauer_s')} s"]
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

    def referenz(self, name: str, vergleich: str, render, art: str, songlaenge_ms: int, quelle=None,
                 auslieferung: Path | None = None, status: dict | None = None, rate: int = RATE):
        """Referenzschritt (M-64, P-21) wie im Runner: Renderdatei und render.json im eigenen Ordner
        referenz\\<Projekt>\\, das Ergebnis ebenda."""
        import numpy as np

        projekt = f"{name}.flp"
        ordner = self.ordner / "referenz" / projekt
        ordner.mkdir(parents=True, exist_ok=True)
        rpfad = ordner / f"{name}.wav"
        if render is not None:
            _schreibe_wav(rpfad, np.asarray(render), art, rate)
        qpfad = self.ordner / "quelle.wav"
        if quelle is not None:
            _schreibe_wav(qpfad, quelle, "pcm24")
        s = ({"format": "nakama.laufzeit.render.v1", "projekt": str(self.ordner / "projekt" / projekt),
              "sha256_projekt": "AB" * 32, "datei": str(rpfad) if render is not None else None, "grund": None,
              "dauer_s": 4.2} if status is None else status)
        spfad = ordner / "render.json"
        spfad.write_text(json.dumps(s), encoding="utf-8")
        return fahre_referenz(vergleich, spfad, songlaenge_ms, ordner / "ergebnis.json", quelle=qpfad,
                              auslieferung=auslieferung)


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


def fall_verarbeitung_ein_umgekehrt(t: Selbsttest) -> None:
    """M-64 (c), P-21: verarbeitung_ein vergleicht wie R1 gegen die Quelle und bewertet umgekehrt - Abweichungen nach
    Ausrichtung sind GEMESSEN (auch mit v != 0 oder Faktor g), 0 Abweichungen VERFEHLT (der Vergleich waere blind),
    Formatfehler und kein Render Szenario-Exit 5. Die Rohzeile traegt die Werte aus M-62 und dazu Projektname, SHA-256
    des Projekts und Renderdauer aus dem Renderstatus (P-21 (f))."""
    import numpy as np

    q = _quelle(400_000)
    band = q[:N_TEST].copy()
    band[50_000:60_000] = (band[50_000:60_000].astype(np.float64) * 2.0).astype(np.float32)  # Bandaenderung als Attrappe
    code, e = t.referenz("band", "verarbeitung_ein", band, "pcm24", MS_TEST, quelle=q)
    t.pruefe(code == 0 and e.get("urteil") == "GEMESSEN" and e.get("befund") == "ABWEICHUNG"
             and (e.get("abweichungen") or 0) > 0 and e.get("v") == 0,
             f"Bandaenderung: Urteil {e.get('urteil')} Befund {e.get('befund')} Abweichungen {e.get('abweichungen')} "
             f"v {e.get('v')} Exit {code} statt GEMESSEN 0")
    zeile = rohzeile(e)
    for teil in ("Vergleich verarbeitung_ein", "Urteil GEMESSEN", "Befund ABWEICHUNG", f"N {N_TEST}", "v 0",
                 "abweichungen ", "erste Abweichung Frame 50000", "letzte Abweichung Frame 59999", "ueberlappend ",
                 "sha256_render_bereich ", "sha256_quelle_bereich ", "Stichprobenspitzen Bereich", "g_db ",
                 "Projekt band.flp", "SHA-256 Projekt " + "AB" * 32, "Renderdauer 4.2 s"):
        t.pruefe(teil in zeile, f"Rohzeile verarbeitung_ein ohne '{teil}': {zeile[:400]}")
    t.pruefe((t.ordner / "referenz" / "band.flp" / "ergebnis.json").exists(),
             "Ergebnis von verarbeitung_ein nicht im Ordner des Referenzprojekts")
    verschoben = np.concatenate([np.zeros((1, 2), np.float32), band[:N_TEST - 1]])
    code, e = t.referenz("band-plus1", "verarbeitung_ein", verschoben, "pcm24", MS_TEST, quelle=q)
    t.pruefe(code == 0 and e.get("urteil") == "GEMESSEN" and e.get("v") == 1 and "v 1" in rohzeile(e),
             f"Bandaenderung mit v = +1: Urteil {e.get('urteil')} v {e.get('v')} Exit {code} statt GEMESSEN 0 mit v 1")
    halb = (q[:N_TEST].astype(np.float64) * 0.5).astype(np.float32)
    code, e = t.referenz("faktor", "verarbeitung_ein", halb, "float32", MS_TEST, quelle=q)
    t.pruefe(code == 0 and e.get("urteil") == "GEMESSEN" and e.get("befund") == "KETTE" and e.get("g_db") is not None
             and abs(e["g_db"] - 20 * math.log10(0.5)) < 1e-6 and "g_db -6.02" in rohzeile(e),
             f"Faktor 0,5: Urteil {e.get('urteil')} Befund {e.get('befund')} g_db {e.get('g_db')} Exit {code} statt GEMESSEN 0 "
             "mit g in der Rohzeile")
    for name, render, befund, v in (("identisch", q[:N_TEST], "BITIDENTISCH", 0),
                                    ("versatz", np.concatenate([np.zeros((1, 2), np.float32), q[:N_TEST - 1]]), "VERSATZ", 1)):
        code, e = t.referenz(name, "verarbeitung_ein", render, "pcm24", MS_TEST, quelle=q)
        t.pruefe(code == 4 and e.get("urteil") == "VERFEHLT" and e.get("befund") == befund and e.get("abweichungen") == 0
                 and e.get("v") == v,
                 f"0 Abweichungen ({befund}, v {v}): Urteil {e.get('urteil')} Befund {e.get('befund')} Exit {code} statt "
                 "VERFEHLT 4 - der Vergleich waere blind")
        t.pruefe("blind" in (e.get("grund") or ""), f"0 Abweichungen ({befund}): Grund {e.get('grund')!r} nennt den blinden Vergleich nicht")
    code, e = t.referenz("pcm16", "verarbeitung_ein", q[:N_TEST], "pcm16", MS_TEST, quelle=q)
    t.pruefe(code == 5 and e.get("urteil") == "VORAUSSETZUNG" and e.get("befund") == "FORMATFEHLER",
             f"16 Bit: Urteil {e.get('urteil')} Befund {e.get('befund')} Exit {code} statt VORAUSSETZUNG 5 FORMATFEHLER")
    code, e = t.referenz("haengt", "verarbeitung_ein", None, "pcm24", MS_TEST, quelle=q,
                         status={"format": "nakama.laufzeit.render.v1", "datei": None, "dauer_s": 600.0,
                                 "grund": "Prozess haengt (Frist 600 s), Fenster 'Missing samples'"})
    t.pruefe(code == 5 and e.get("urteil") == "VORAUSSETZUNG" and e.get("befund") == "KEIN_RENDER"
             and "Prozess haengt" in (e.get("grund") or ""),
             f"ohne Render: Urteil {e.get('urteil')} Befund {e.get('befund')} Grund {e.get('grund')!r} Exit {code} statt 5")


def fall_ohne_slots_render_gegen_render(t: Selbsttest) -> None:
    """M-64 (d), P-21: ohne_slots hasht den Datenbereich des Referenzrenders - derselbe Framebereich und dieselbe
    Float-32-Darstellung wie sha256_render_bereich der ergebnis.json des Auslieferungsrenders - und vergleicht mit diesem
    Wert: gleich GEMESSEN, ungleich VERFEHLT mit beiden SHA-256, der Abweichungszahl und der ersten Abweichung. Ohne
    ergebnis.json oder ohne bitidentischen Auslieferungsschritt, ohne Render und bei Formatfehler Szenario-Exit 5."""
    import numpy as np

    q = _quelle(400_000)
    code, a = t.fall("auslieferung", q[:N_TEST + 400], "pcm24", MS_TEST, quelle=q)
    t.pruefe(code == 0 and a["urteil"] == "BITIDENTISCH", f"Attrappe des Auslieferungsrenders: Urteil {a['urteil']} Exit {code}")
    auslieferung = t.ordner / "auslieferung-ergebnis.json"
    # andere Laenge und anderes Format, in [0, N) dieselben Daten
    gleiche_daten = np.concatenate([q[:N_TEST], np.full((250, 2), np.float32(0.25))])
    code, e = t.referenz("ohne-slots", "ohne_slots", gleiche_daten, "float32", MS_TEST, auslieferung=auslieferung)
    t.pruefe(code == 0 and e.get("urteil") == "GEMESSEN" and e.get("sha256_render_bereich") == a.get("sha256_render_bereich")
             and e.get("sha256_auslieferung_bereich") == a.get("sha256_render_bereich") and e.get("abweichungen") == 0,
             f"dieselben Daten in [0, N): Urteil {e.get('urteil')} Exit {code}, SHA-256 {e.get('sha256_render_bereich')} gegen "
             f"{a.get('sha256_render_bereich')}, Abweichungen {e.get('abweichungen')} statt GEMESSEN 0")
    k = 98_765
    gekippt = q[:N_TEST].copy()
    gekippt[k, 1] = q[k, 1] + np.float32(2.0 ** -23)
    code, e = t.referenz("ohne-slots-lsb", "ohne_slots", gekippt, "pcm24", MS_TEST, auslieferung=auslieferung)
    t.pruefe(code == 4 and e.get("urteil") == "VERFEHLT",
             f"ein gekipptes LSB an Frame {k}: Urteil {e.get('urteil')} Exit {code} statt VERFEHLT 4")
    t.pruefe(e.get("abweichungen") == 1 and (e.get("erste") or {}).get("frame") == k and (e.get("erste") or {}).get("kanal") == 1
             and e.get("sha256_render_bereich") not in (None, a.get("sha256_render_bereich")),
             f"ein gekipptes LSB an Frame {k}: Abweichungen {e.get('abweichungen')}, erste {e.get('erste')}, "
             f"SHA-256 {e.get('sha256_render_bereich')}")
    zeile = rohzeile(e)
    for teil in ("Vergleich ohne_slots", "Urteil VERFEHLT", f"sha256_render_bereich {e.get('sha256_render_bereich')}",
                 f"sha256_auslieferung_bereich {a.get('sha256_render_bereich')}", "abweichungen 1",
                 f"erste Abweichung Frame {k} Kanal 1", f"N {N_TEST}", "v 0", "Stichprobenspitzen Bereich", "g_db ",
                 "Projekt ohne-slots-lsb.flp", "SHA-256 Projekt " + "AB" * 32, "Renderdauer 4.2 s"):
        t.pruefe(teil in zeile, f"Rohzeile ohne_slots ohne '{teil}': {zeile[:400]}")
    verschoben = np.concatenate([np.zeros((1, 2), np.float32), q[:N_TEST - 1]])
    code, e = t.referenz("ohne-slots-versatz", "ohne_slots", verschoben, "pcm24", MS_TEST, auslieferung=auslieferung)
    t.pruefe(code == 4 and e.get("urteil") == "VERFEHLT" and (e.get("abweichungen") or 0) > 0,
             f"um 1 Frame verschobener Render ohne Slots: Urteil {e.get('urteil')} Exit {code} statt VERFEHLT 4 "
             "(keine Versatzsuche gegen den Auslieferungsrender)")
    code, e = t.referenz("ohne-slots-ohne-ergebnis", "ohne_slots", gleiche_daten, "float32", MS_TEST,
                         auslieferung=t.ordner / "nie-geschrieben-ergebnis.json")
    t.pruefe(code == 5 and e.get("urteil") == "VORAUSSETZUNG" and "fehlt" in (e.get("grund") or ""),
             f"ohne ergebnis.json des Auslieferungsrenders: Urteil {e.get('urteil')} Grund {e.get('grund')!r} Exit {code} statt 5")
    code, kette = t.fall("auslieferung-kette", (q[:N_TEST].astype(np.float64) * 0.5).astype(np.float32), "float32", MS_TEST,
                         quelle=q)
    code, e = t.referenz("ohne-slots-kette", "ohne_slots", gleiche_daten, "float32", MS_TEST,
                         auslieferung=t.ordner / "auslieferung-kette-ergebnis.json")
    t.pruefe(kette["urteil"] == "KETTE" and code == 5 and e.get("urteil") == "VORAUSSETZUNG"
             and "nicht BITIDENTISCH" in (e.get("grund") or ""),
             f"Auslieferungsschritt KETTE: Urteil {e.get('urteil')} Grund {e.get('grund')!r} Exit {code} statt VORAUSSETZUNG 5")
    for name, render, art, grund in (("ohne-slots-pcm16", q[:N_TEST], "pcm16", "Renderformat"),
                                     ("ohne-slots-kurz", q[:N_TEST - 10], "pcm24", "kuerzer")):
        code, e = t.referenz(name, "ohne_slots", render, art, MS_TEST, auslieferung=auslieferung)
        t.pruefe(code == 5 and e.get("befund") == "FORMATFEHLER" and grund in (e.get("grund") or ""),
                 f"{name}: Befund {e.get('befund')} Grund {e.get('grund')!r} Exit {code} statt FORMATFEHLER 5")
    code, e = t.referenz("ohne-slots-fehlt", "ohne_slots", None, "pcm24", MS_TEST, auslieferung=auslieferung,
                         status={"format": "nakama.laufzeit.render.v1", "datei": None,
                                 "grund": "Referenzprojekt fehlt (Karte U43, K-286-1)"})
    t.pruefe(code == 5 and e.get("befund") == "KEIN_RENDER" and "Referenzprojekt fehlt" in (e.get("grund") or ""),
             f"ohne Render: Befund {e.get('befund')} Grund {e.get('grund')!r} Exit {code} statt KEIN_RENDER 5")


FAELLE = [
    ("M-62", "identisch_null_abweichungen", fall_identisch_null_abweichungen),
    ("M-75", "versatz_ist_voraussetzung", fall_versatz_ist_voraussetzung),
    ("M-63", "konstanter_faktor_ist_voraussetzung", fall_konstanter_faktor_ist_voraussetzung),
    ("M-61", "format_ist_voraussetzung", fall_format_ist_voraussetzung),
    ("M-60", "render_status", fall_render_status),
    ("M-64", "verarbeitung_ein_umgekehrt", fall_verarbeitung_ein_umgekehrt),
    ("M-64", "ohne_slots_render_gegen_render", fall_ohne_slots_render_gegen_render),
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
    ap.add_argument("--vergleich", choices=("auslieferung", *REFERENZ_VERGLEICHE), default="auslieferung")
    ap.add_argument("--auslieferung", type=Path, help="ergebnis.json des Auslieferungsrenders (ohne_slots)")
    ap.add_argument("--selbsttest", action="store_true")
    ap.add_argument("--nur", action="append", default=[])
    args = ap.parse_args(argv)
    try:
        if args.selbsttest:
            return selbsttest(args.nur)
        if args.vergleich in REFERENZ_VERGLEICHE:
            bezug = args.quelle if args.vergleich == "verarbeitung_ein" else args.auslieferung
            if not (args.renderstatus and args.songlaenge_ms is not None and args.ergebnis and bezug):
                ap.error("--vergleich verarbeitung_ein braucht --renderstatus, --quelle, --songlaenge-ms und --ergebnis; "
                         "ohne_slots statt --quelle --auslieferung")
            code, ergebnis = fahre_referenz(args.vergleich, args.renderstatus, args.songlaenge_ms, args.ergebnis,
                                            quelle=args.quelle, auslieferung=args.auslieferung)
            print("NULLTEST " + rohzeile(ergebnis))
            return code
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
