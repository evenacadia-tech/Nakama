#!/usr/bin/env python3
"""Quantisierungsvertrag fuer SONDE-005a (Entwurf §33.1).

Erzeugt eq-copilot/schemas/v3/quantisierung-v1.json: die drei Kodierungen der
Bandwerte samt Testvektoren, gegen die C++ und Rust bitgleich rechnen muessen.

    q_db_0p1_i16    dB * 10   -> i16     Live (10 Hz), 0,1 dB Aufloesung
    q_db_0p01_i16   dB * 100  -> i16     fokussierte PRE/POST-Evidenz
    float32         dB        -> f32     komplexe Kreuzstatistik

WARUM DAS EINEN VERTRAG BRAUCHT
-------------------------------
"Runden" ist keine eindeutige Anweisung. Es gibt mindestens drei gaengige
Regeln, und die drei Sprachen dieses Projekts sind sich uneinig:

    C++   std::round / std::lround   halbe Werte VON NULL WEG
    Rust  f64::round                 halbe Werte VON NULL WEG
    Python round()                   halbe Werte zur GERADEN Zahl (Banker)

Ein Wert von genau -0,05 dB wuerde in C++/Rust zu -1 und in Python zu 0. Der
Vertrag legt deshalb ausdruecklich VON NULL WEG fest, und dieses Werkzeug
implementiert die Regel selbst, statt Pythons round() zu benutzen.

Die zweite Falle liegt eine Ebene tiefer: `floor(abs(x) + 0.5)` ist NICHT
dieselbe Regel. Fuer x = 0.49999999999999994 (der groesste double unter 0,5)
rundet die Addition auf exakt 1.0 auf, und floor liefert 1 statt 0. Die
Korrektur unten faengt genau diesen Fall; er steht als Testvektor drin.

NICHTENDLICHE WERTE
-------------------
NaN, +inf und -inf werden NICHT saturiert und nicht sanitisiert. Sie werden zu
Wert 0 mit GUELTIGKEITSBIT 0 — die Bitmap ist die Wahrheit, nicht der Wert
(§33.1: "Gueltigkeitsbitmap, Saettigungsbit und Encoding gehoeren in jeden
Batch"). Ein saturierter Wert dagegen bleibt GUELTIG und setzt nur das
Saettigungsbit: er ist gemessen, nur nicht mehr aufloesbar.

Aufruf:
    py -3.13 tools/eq-copilot/erzeuge_quantisierung.py
    py -3.13 tools/eq-copilot/erzeuge_quantisierung.py --pruefen
"""

from __future__ import annotations

import hashlib
import json
import math
import pathlib
import struct
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
ZIEL = WURZEL / "eq-copilot" / "schemas" / "v3" / "quantisierung-v1.json"

I16_MIN = -32768
I16_MAX = 32767

# Groesster endlicher binary32-Wert.
F32_MAX = 3.4028234663852886e38


# ------------------------------------------------------------------ Zahlformat

def hex64(x: float) -> str:
    return format(struct.unpack("<Q", struct.pack("<d", x))[0], "016X")


def hex32(x: float) -> str:
    """Bitmuster der binary32-Rundung von x (round-to-nearest-even)."""
    return format(struct.unpack("<I", struct.pack("<f", x))[0], "008X")


# ------------------------------------------------------------------ Rundung

def rund_weg_von_null(x: float) -> float:
    """Halbe Werte von null weg — dieselbe Regel wie std::round und f64::round.

    NICHT `floor(a + 0.5)`: fuer a = 0.49999999999999994 (= 0,5 - 2^-54)
    rundet die Addition auf exakt 1.0 auf, und floor liefert 1 statt 0.

    Und auch nicht der naheliegende Nachbesserungsversuch `if y - a > 0.5`:
    die Differenz 1.0 - (0,5 - 2^-54) ist 0,5 + 2^-54 und liegt genau zwischen
    zwei doubles, rundet also auf exakt 0.5 — die Bedingung greift nie. Diese
    Fassung stand hier und wurde vom eigenen Testvektor widerlegt.

    Tragfaehig ist der Nachkommateil: `a - floor(a)` ist fuer jeden endlichen
    a >= 0 EXAKT (unterhalb 1 trivial, darueber nach Sterbenz), also gibt es
    dort keinen Rundungsfehler, den man nachbessern muesste.
    """
    a = abs(x)
    y = math.floor(a)
    if a - y >= 0.5:
        y += 1.0
    return math.copysign(y, x)


def quantisiere_i16(db: float, skalierung: int) -> tuple[int, bool, bool]:
    """-> (wert, gueltig, saturiert)."""
    if not math.isfinite(db):
        return 0, False, False
    roh = rund_weg_von_null(db * skalierung)
    if roh > I16_MAX:
        return I16_MAX, True, True
    if roh < I16_MIN:
        return I16_MIN, True, True
    return int(roh), True, False


def quantisiere_f32(db: float) -> tuple[str, bool, bool]:
    """-> (bitmuster_hex32, gueltig, saturiert).

    Saettigung ist hier ein anderer Fall als bei i16: ein endlicher double
    kann beim Verengen auf binary32 zu +-inf werden. Das ist ein WERTVERLUST,
    kein Messfehler — der Wert bleibt gueltig und traegt das Saettigungsbit
    (auf den groessten endlichen f32 geklemmt), damit ein Konsument nicht mit
    einer Unendlichkeit weiterrechnet.
    """
    if not math.isfinite(db):
        return hex32(0.0), False, False
    try:
        # Python meldet den Ueberlauf als Ausnahme, C++ und Rust liefern dort
        # +-inf. Dieselbe Grenze, zwei Meldewege — beide Seiten muessen den
        # Fall ABFANGEN, statt mit einer Unendlichkeit weiterzurechnen.
        eng = struct.unpack("<f", struct.pack("<f", db))[0]
    except OverflowError:
        eng = math.copysign(math.inf, db)
    if not math.isfinite(eng):
        grenze = math.copysign(F32_MAX, db)
        return hex32(grenze), True, True
    return hex32(eng), True, False


# ------------------------------------------------------------------ Vektoren

def i16_vektoren(skalierung: int) -> list[dict]:
    grenze_db = I16_MAX / skalierung
    unter_db = I16_MIN / skalierung

    roh = [
        0.0,
        -0.0,
        1.0 / skalierung,                  # exakt eine Stufe
        -1.0 / skalierung,
        0.5 / skalierung,                  # halbe Stufe positiv -> von null weg
        -0.5 / skalierung,                 # halbe Stufe negativ -> von null weg
        1.5 / skalierung,                  # zweite halbe Stufe: Banker wuerde auf 2 gehen, wir auch
        2.5 / skalierung,                  # hier trennt sich Banker (2) von uns (3)
        -2.5 / skalierung,
        0.49999999999999994 / skalierung,  # groesster double unter 0,5 nach Skalierung
        -0.49999999999999994 / skalierung,
        -12.3,
        -60.0,
        -120.0,
        grenze_db,                         # exakt der groesste darstellbare Wert
        unter_db,                          # exakt der kleinste
        grenze_db + 1.0,                   # Saettigung oben
        unter_db - 1.0,                    # Saettigung unten
        1e-300,
        -1e-300,
        float("nan"),
        float("inf"),
        float("-inf"),
    ]

    vektoren = []
    for db in roh:
        wert, gueltig, saturiert = quantisiere_i16(db, skalierung)
        vektoren.append({
            "ein_hex64": hex64(db),
            "ein_dezimal": repr(db),
            "wert": wert,
            "gueltig": gueltig,
            "saturiert": saturiert,
        })
    return vektoren


def f32_vektoren() -> list[dict]:
    roh = [
        0.0,
        -0.0,
        -12.3,
        -60.0,
        1.0,
        0.1,                               # in binary32 nicht exakt
        1.0000000596046448,                # genau zwischen zwei f32 -> zur geraden
        3.4028234663852886e38,             # groesster endlicher f32
        3.5e38,                            # verengt zu +inf -> geklemmt, saturiert
        -3.5e38,
        1e-45,                             # subnormal in f32
        1e-320,                            # verengt zu 0
        float("nan"),
        float("inf"),
        float("-inf"),
    ]
    vektoren = []
    for db in roh:
        bits, gueltig, saturiert = quantisiere_f32(db)
        vektoren.append({
            "ein_hex64": hex64(db),
            "ein_dezimal": repr(db),
            "wert_hex32": bits,
            "gueltig": gueltig,
            "saturiert": saturiert,
        })
    return vektoren


# Plausibilitaetsgrenze der Bandwerte in dB (siehe "plausibilitaet" unten).
PLAUSIBEL_MIN_DB = -144.0
PLAUSIBEL_MAX_DB = 24.0


def inhalt() -> dict:
    return {
        "$id": "evenacadia.nakama.quantisierung.v1",
        "titel": "Kodierung der Bandwerte v1",
        "zweck": "Entwurf §33.1: Empfaenger raten die Skalierung nie aus dem Nachrichtentyp. Diese Datei ist die eine Wahrheit fuer Skalierung, Rundung, Saettigung und den Umgang mit nichtendlichen Werten.",
        "regeln": {
            "reihenfolge": "Energie wird LINEAR integriert und erst danach in dB quantisiert (§33.1). Wer in dB mittelt, misst etwas anderes.",
            "rundung": "Halbe Werte VON NULL WEG (std::round / f64::round). NICHT Pythons round(), das zur geraden Zahl rundet.",
            "rundung_falle": "floor(abs(x)+0.5) ist NICHT dieselbe Regel: fuer x = 0.49999999999999994 liefert sie 1 statt 0. Ein Vektor deckt genau das ab.",
            "nichtendlich": "NaN, +inf und -inf ergeben Wert 0 (bzw. +0.0f) mit gueltig=false. Sie werden nie saturiert und nie sanitisiert — die Gueltigkeitsbitmap ist die Wahrheit.",
            "saettigung": "Ein saturierter Wert bleibt GUELTIG und setzt zusaetzlich das Saettigungsbit: er ist gemessen, nur nicht mehr aufloesbar.",
            "bitmap": "Base64 ueber ceil(n/8) Bytes, LSB-first je Byte: Band i sitzt in Byte i/8, Bit i%8.",
            "plausibilitaet": "`bereich_db` ist der TRAEGERumfang — was ein i16 mit dieser Skalierung ueberhaupt darstellen kann —, NICHT der Bereich, in dem ein Bandwert liegen DARF. Letzteres ist `plausibler_bereich_db`. Die beiden Binaerleser aus SONDE-005b setzen ihn durch und vergleichen ihre einkompilierten Konstanten bei JEDEM Testlauf mit dieser Datei. Vorher standen die Zahlen nur im Quelltext, waehrend README und Beweismanifest `bereich_db` als ihre Quelle nannten (T2-Runde 3, Befund 8) — eine Quellenangabe, die auf etwas anderes zeigte als auf das, was durchgesetzt wurde."
        },
        "plausibler_bereich_db": {
            "wert": [PLAUSIBEL_MIN_DB, PLAUSIBEL_MAX_DB],
            "grund": "-144 dB ist die Untergrenze der NaN-Ehrlichkeit: darunter ist nichts mehr Signal, sondern Rechenrauschen. +24 dB liegt weit ueber jedem Bandpegel, den ein Mix erzeugt, und faengt trotzdem einen umgeklappten oder falsch skalierten Wert. Die Grenze gilt fuer BEIDE i16-Kodierungen in dB, nicht in Traegerwerten — die Traegergrenze folgt aus der Skalierung.",
            "traegergrenzen": {
                "q_db_0p1_i16": [int(PLAUSIBEL_MIN_DB * 10), int(PLAUSIBEL_MAX_DB * 10)],
                "q_db_0p01_i16": [int(PLAUSIBEL_MIN_DB * 100), int(PLAUSIBEL_MAX_DB * 100)]
            }
        },
        "kodierungen": {
            "q_db_0p1_i16": {
                "traeger": "i16",
                "skalierung": 10,
                "bereich_db": [I16_MIN / 10, I16_MAX / 10],
                "verwendung": "Live-Telemetrie 10 Hz (§33.2)",
                "vektoren": i16_vektoren(10)
            },
            "q_db_0p01_i16": {
                "traeger": "i16",
                "skalierung": 100,
                "bereich_db": [I16_MIN / 100, I16_MAX / 100],
                "verwendung": "fokussierte PRE/POST-Evidenz (§33.1)",
                "vektoren": i16_vektoren(100)
            },
            "float32": {
                "traeger": "f32",
                "skalierung": 1,
                "bereich_db": None,
                "verwendung": "komplexe Kreuzstatistik (§33.1)",
                "rundung": "IEEE-754 round-to-nearest-even beim Verengen von binary64 auf binary32",
                "vektoren": f32_vektoren()
            }
        }
    }


# ------------------------------------------------------------------ Pruefungen

def pruefe(daten: dict) -> list[str]:
    fehler: list[str] = []

    def wahr(name: str, bedingung: bool) -> None:
        if not bedingung:
            fehler.append(name)

    for name, k in daten["kodierungen"].items():
        v = k["vektoren"]
        wahr(f"{name}: Vektoren vorhanden", len(v) > 0)
        wahr(f"{name}: nichtendliche Eingaben sind ungueltig",
             all(not e["gueltig"] for e in v
                 if not math.isfinite(float(e["ein_dezimal"]))))
        wahr(f"{name}: nichtendliche Eingaben sind nie saturiert",
             all(not e["saturiert"] for e in v
                 if not math.isfinite(float(e["ein_dezimal"]))))
        wahr(f"{name}: dezimal liest exakt auf hex64 zurueck",
             all(hex64(float(e["ein_dezimal"])) == e["ein_hex64"] for e in v))
        if k["traeger"] == "i16":
            wahr(f"{name}: alle Werte im i16-Bereich",
                 all(I16_MIN <= e["wert"] <= I16_MAX for e in v))
            wahr(f"{name}: saturierte Werte liegen auf einer Grenze",
                 all(e["wert"] in (I16_MIN, I16_MAX) for e in v if e["saturiert"]))
            wahr(f"{name}: saturierte Werte bleiben gueltig",
                 all(e["gueltig"] for e in v if e["saturiert"]))

    # Die Regel, wegen der es diese Datei gibt: 2,5 Stufen -> 3, nicht 2.
    v10 = daten["kodierungen"]["q_db_0p1_i16"]["vektoren"]
    treffer = [e for e in v10 if e["ein_dezimal"] == repr(2.5 / 10)]
    wahr("2,5 Stufen runden von null weg auf 3 (Banker haette 2)",
         len(treffer) == 1 and treffer[0]["wert"] == 3)
    treffer = [e for e in v10 if e["ein_dezimal"] == repr(-2.5 / 10)]
    wahr("-2,5 Stufen runden von null weg auf -3",
         len(treffer) == 1 and treffer[0]["wert"] == -3)

    # Die Falle eine Ebene tiefer.
    wahr("0.49999999999999994 rundet auf 0, nicht auf 1",
         rund_weg_von_null(0.49999999999999994) == 0.0)
    wahr("0.5 rundet auf 1", rund_weg_von_null(0.5) == 1.0)
    wahr("-0.5 rundet auf -1", rund_weg_von_null(-0.5) == -1.0)

    return fehler


def main(argv: list[str]) -> int:
    daten = inhalt()
    fehler = pruefe(daten)

    anzahl = sum(len(k["vektoren"]) for k in daten["kodierungen"].values())
    print(f"{len(daten['kodierungen'])} Kodierungen, {anzahl} Testvektoren")
    print(f"Pruefungen: {len(fehler)} gescheitert")
    for f in fehler:
        print(f"  ROT: {f}")
    if fehler:
        return 2

    text = (json.dumps(daten, indent=2, ensure_ascii=False) + "\n").encode("utf-8")
    if "--pruefen" in argv:
        if not ZIEL.exists():
            print(f"  ROT: {ZIEL} fehlt")
            return 3
        if ZIEL.read_bytes() != text:
            print(f"  ROT: {ZIEL} weicht von der Neuerzeugung ab")
            return 2
        print(f"  bytegleich: {ZIEL.relative_to(WURZEL)} sha256={hashlib.sha256(text).hexdigest()}")
        return 0

    ZIEL.parent.mkdir(parents=True, exist_ok=True)
    ZIEL.write_bytes(text)
    print(f"  geschrieben: {ZIEL.relative_to(WURZEL)} sha256={hashlib.sha256(text).hexdigest()}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
