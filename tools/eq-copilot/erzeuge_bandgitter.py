#!/usr/bin/env python3
"""Bandgitter-Erzeuger fuer SONDE-005a (Entwurf §33.1, §33.2).

Erzeugt die beiden gemeinsamen Zahlenfixturen, die C++ und Rust ab P1 lesen:

  nakama_1_24_oct_30_18k_v1   221 Baender, 1/24 Oktave, Evidenzgitter (1-4 Hz)
  nakama_log64_v1              64 Gruppen, Livegitter (10 Hz)

WARUM DIE ZAHLEN IM REPOSITORY LIEGEN UND NICHT BERECHNET WERDEN
----------------------------------------------------------------
`pow` ist nicht bit-portabel. MSVCs `std::pow`, Rusts `f64::powf` und CPythons
`math.pow` duerfen fuer dieselbe Eingabe in der letzten Stelle abweichen — der
C++-Standard verlangt nur "implementation-defined accuracy". Wuerde jede Seite
ihr Gitter selbst rechnen, waeren die Bandgrenzen um Ulps verschieden, und ein
Golden-Test faende Jahre spaeter einen Unterschied, den niemand mehr erklaeren
kann. Deshalb: EINMAL rechnen, als Bitmuster einfrieren, alle lesen dasselbe.

Aus demselben Grund traegt jede Zahl ein 16-stelliges Hex-Bitmuster als
WAHRHEIT und die Dezimalschreibweise nur als Dokumentation. Ein Dezimalstring
muesste von drei verschiedenen JSON-Parsern korrekt gerundet werden (JUCEs
`CharacterFunctions::readDoubleValue` ist ein handgeschriebener Parser, kein
strtod) — beim Hexmuster gibt es nichts zu runden.

DAS GITTER
----------
IEC 61260-1:2014 / ANSI S1.11-2014 kennen zwei Faelle fuer den
Bandbreitenkennwert 1/b:

    b ungerade:  fm = fr * G^(x/b)
    b gerade:    fm = fr * G^((2x+1)/(2b))        <- 1/24 Oktave, b = 24

Mit G = 2 (Basis-2-System), fr = 1000 Hz und allen Mittenfrequenzen im Band
30 Hz .. 18 kHz ergibt das x = -121 .. 99, also GENAU 221 Baender — die Zahl,
die Entwurf §33.2 fuer den Evidenzsnapshot nennt. Keine der drei anderen
denkbaren Konventionen (Basis-10, oder ohne Halbschritt) trifft 221; die
Bandzahl aus dem Entwurf ist damit die Pruefsumme ueber diese Entscheidung.

Der Halbschritt bedeutet anschaulich: die KANTEN liegen auf dem an 1 kHz
verankerten 1/24-Oktav-Gitter, die Mitten dazwischen. Deshalb wird hier zuerst
das Kantengitter erzeugt (fr * 2^(k/24)) und jede Mitte als geometrisches
Mittel ihrer beiden Kanten. Damit ist "obere Kante von Band n == untere Kante
von Band n+1" nicht auf 1e-12 genau, sondern DASSELBE BIT — es gibt gar keine
zweite Rechnung, die abweichen koennte.

DAS GROBE GITTER
----------------
`nakama_log64_v1` ist keine eigene Frequenzachse, sondern eine EXAKTE PARTITION
der 221 feinen Baender in 64 zusammenhaengende Gruppen. Damit ist das
Livespektrum die lineare Energiesumme der Evidenzbaender (§33.1: "Energie wird
linear integriert und erst danach in dB quantisiert") — keine zweite
Filterbank, keine Interpolation, kein Doppelzaehlen. Gruppengroessen sind 3
oder 4 feine Baender (221 / 64 = 3,45).

Aufruf:
    py -3.13 tools/eq-copilot/erzeuge_bandgitter.py            # schreiben
    py -3.13 tools/eq-copilot/erzeuge_bandgitter.py --pruefen  # nur pruefen
"""

from __future__ import annotations

import hashlib
import json
import math
import pathlib
import struct
import sys

# ------------------------------------------------------------------ Konstanten

WURZEL = pathlib.Path(__file__).resolve().parents[2]
ZIEL = WURZEL / "eq-copilot" / "schemas" / "v3" / "bandgitter"

FEIN_ID = "nakama_1_24_oct_30_18k_v1"
GROB_ID = "nakama_log64_v1"

G = 2.0            # Oktavverhaeltnis, Basis-2-System (IEC 61260-1 erlaubt 2 oder 10^0.3)
B = 24             # Bandbreitenkennwert 1/b
FR = 1000.0        # Referenzfrequenz
UNTEN_HZ = 30.0    # Mittenfrequenzen liegen innerhalb dieser Grenzen
OBEN_HZ = 18000.0
GROB_ANZAHL = 64


# ------------------------------------------------------------------ Zahlformat

def hex64(x: float) -> str:
    """IEEE-754-binary64-Bitmuster als 16 Grossbuchstaben-Hexziffern."""
    return format(struct.unpack("<Q", struct.pack("<d", x))[0], "016X")


def aus_hex64(h: str) -> float:
    return struct.unpack("<d", struct.pack("<Q", int(h, 16)))[0]


def dezimal(x: float) -> str:
    """Kuerzeste Dezimalform, die exakt zurueckliest (repr in Python 3)."""
    return repr(x)


def zahlenblock(werte: list[float]) -> dict:
    return {
        "anzahl": len(werte),
        "hex64": [hex64(w) for w in werte],
        "dezimal": [dezimal(w) for w in werte],
    }


# ------------------------------------------------------------------ Bandgitter

def x_bereich() -> tuple[int, int]:
    """Kleinstes und groesstes x, dessen Mittenfrequenz in [UNTEN, OBEN] liegt."""
    xs = [x for x in range(-4000, 4000)
          if UNTEN_HZ <= FR * G ** ((2 * x + 1) / (2 * B)) <= OBEN_HZ]
    return xs[0], xs[-1]


def feines_gitter() -> dict:
    x_min, x_max = x_bereich()
    anzahl = x_max - x_min + 1

    # Kantengitter: fr * G^(k/b) fuer k = x_min .. x_max+1  (anzahl+1 Kanten).
    # Der Halbschritt fuer gerades b faellt damit weg: die Mitten sind die
    # geometrischen Mittel, siehe Kopf.
    kanten = [FR * G ** (k / B) for k in range(x_min, x_max + 2)]
    mitten = [math.sqrt(kanten[i] * kanten[i + 1]) for i in range(anzahl)]

    return {
        "$id": f"evenacadia.nakama.bandgitter.{FEIN_ID}",
        "gitter_id": FEIN_ID,
        "art": "fraktionale_oktave",
        "zweck": "Evidenzgitter — voller Bandsatz des Evidenzsnapshots (Entwurf §33.2, 1-4 Hz)",
        "herkunft": {
            "norm": "IEC 61260-1:2014 / ANSI S1.11-2014",
            "oktavverhaeltnis_G": 2,
            "bandbreitenkennwert_b": B,
            "referenzfrequenz_hz": FR,
            "x_min": x_min,
            "x_max": x_max,
            "regel": ("b ist gerade, also fm = fr*G^((2x+1)/(2b)); gleichbedeutend "
                      "liegen die KANTEN auf fr*G^(k/b) und die Mitten sind deren "
                      "geometrische Mittel — so erzeugt hier."),
            "grenzen_hz": [UNTEN_HZ, OBEN_HZ],
            "grenzen_bedeutung": "Mittenfrequenzen liegen innerhalb; Kanten duerfen darueber hinausragen",
        },
        "band_anzahl": anzahl,
        "zahlformat": ("hex64 ist die Wahrheit: IEEE-754-binary64-Bitmuster, 16 "
                       "Grossbuchstaben-Hexziffern. dezimal ist Dokumentation und "
                       "wird von keinem Konsumenten gelesen."),
        "kanten_hz": zahlenblock(kanten),
        "mitten_hz": zahlenblock(mitten),
    }


def grobes_gitter(fein: dict) -> dict:
    n = fein["band_anzahl"]
    kanten_fein = [aus_hex64(h) for h in fein["kanten_hz"]["hex64"]]

    grenzen = [(j * n) // GROB_ANZAHL for j in range(GROB_ANZAHL + 1)]
    gruppen = [
        {
            "index": j,
            "fein_von": grenzen[j],
            "fein_bis_exkl": grenzen[j + 1],
            "anzahl": grenzen[j + 1] - grenzen[j],
        }
        for j in range(GROB_ANZAHL)
    ]

    kanten = [kanten_fein[g] for g in grenzen]
    mitten = [math.sqrt(kanten[j] * kanten[j + 1]) for j in range(GROB_ANZAHL)]

    return {
        "$id": f"evenacadia.nakama.bandgitter.{GROB_ID}",
        "gitter_id": GROB_ID,
        "art": "gruppierung",
        "zweck": "Livegitter — 10-Hz-Telemetrie (Entwurf §33.2)",
        "basis_gitter": FEIN_ID,
        "herkunft": {
            "regel": (f"Gruppe j deckt die feinen Baender [floor(j*{n}/{GROB_ANZAHL}), "
                      f"floor((j+1)*{n}/{GROB_ANZAHL})). Ganzzahlig, damit die "
                      "Partition exakt ist."),
            "warum": ("Das Livespektrum ist die lineare Energiesumme der Evidenzbaender, "
                      "keine zweite Filterbank. Deshalb sind die groben Kanten "
                      "BITGLEICHE Kopien feiner Kanten."),
        },
        "band_anzahl": GROB_ANZAHL,
        "zahlformat": fein["zahlformat"],
        "gruppen": gruppen,
        "kanten_hz": zahlenblock(kanten),
        "mitten_hz": zahlenblock(mitten),
    }


# ------------------------------------------------------------------ Pruefungen

class Pruefer:
    def __init__(self) -> None:
        self.ok = 0
        self.fehler: list[str] = []

    def gleich(self, name: str, ist, soll) -> None:
        if ist == soll:
            self.ok += 1
        else:
            self.fehler.append(f"{name}: ist {ist!r}, soll {soll!r}")

    def wahr(self, name: str, bedingung: bool) -> None:
        if bedingung:
            self.ok += 1
        else:
            self.fehler.append(name)


def pruefe(fein: dict, grob: dict) -> Pruefer:
    p = Pruefer()

    # --- feines Gitter ---
    n = fein["band_anzahl"]
    p.gleich("feine Bandzahl", n, 221)
    kanten = [aus_hex64(h) for h in fein["kanten_hz"]["hex64"]]
    mitten = [aus_hex64(h) for h in fein["mitten_hz"]["hex64"]]
    p.gleich("Kantenzahl", len(kanten), n + 1)
    p.gleich("Mittenzahl", len(mitten), n)

    p.wahr("alle Kanten endlich", all(math.isfinite(k) for k in kanten))
    p.wahr("alle Mitten endlich", all(math.isfinite(m) for m in mitten))
    p.wahr("Kanten streng monoton", all(kanten[i] < kanten[i + 1] for i in range(len(kanten) - 1)))
    p.wahr("Mitten streng monoton", all(mitten[i] < mitten[i + 1] for i in range(len(mitten) - 1)))
    p.wahr("Mitte liegt echt zwischen ihren Kanten",
           all(kanten[i] < mitten[i] < kanten[i + 1] for i in range(n)))

    # Lueckenlosigkeit ist hier KEINE Toleranzfrage: es gibt nur EIN Kantenarray.
    p.wahr("Bandgrenzen teilen sich dieselbe Kante (per Konstruktion)", len(kanten) == n + 1)

    p.wahr("alle Mitten in [30, 18000]", all(UNTEN_HZ <= m <= OBEN_HZ for m in mitten))
    p.wahr("Mitte unter 30 waere Band x_min-1",
           FR * G ** ((2 * (fein["herkunft"]["x_min"] - 1) + 1) / (2 * B)) < UNTEN_HZ)
    p.wahr("Mitte ueber 18000 waere Band x_max+1",
           FR * G ** ((2 * (fein["herkunft"]["x_max"] + 1) + 1) / (2 * B)) > OBEN_HZ)

    # Dezimalschreibweise darf nicht luegen.
    #
    # T2-Runde 1: hier stand ein blosses `zip(dezimal, hex64)`. `zip` kuerzt auf
    # die kuerzere Liste, und fuer `dezimal` gab es keine Laengenpruefung — eine
    # leere oder verkuerzte Dezimalspalte haette die Pruefung LEER-GRUEN gemacht.
    # Die Laengen werden deshalb zuerst verglichen, und `strict=True` faengt den
    # Fall auch dann, wenn jemand die Vergleichszeile spaeter umbaut.
    for feld in ("kanten_hz", "mitten_hz"):
        p.gleich(f"{feld}: dezimal und hex64 gleich lang",
                 len(fein[feld]["dezimal"]), len(fein[feld]["hex64"]))
        p.wahr(f"dezimal liest exakt auf hex64 zurueck ({feld})",
               len(fein[feld]["dezimal"]) > 0
               and all(float(d) == aus_hex64(h)
                       for d, h in zip(fein[feld]["dezimal"], fein[feld]["hex64"], strict=True)))

    # --- grobes Gitter ---
    gruppen = grob["gruppen"]
    p.gleich("grobe Bandzahl", grob["band_anzahl"], GROB_ANZAHL)
    p.gleich("Gruppenzahl", len(gruppen), GROB_ANZAHL)
    p.gleich("Summe der Gruppengroessen", sum(g["anzahl"] for g in gruppen), n)
    p.wahr("keine leere Gruppe", all(g["anzahl"] > 0 for g in gruppen))
    p.wahr("Gruppen sind lueckenlos und ueberlappungsfrei",
           gruppen[0]["fein_von"] == 0
           and gruppen[-1]["fein_bis_exkl"] == n
           and all(gruppen[j]["fein_bis_exkl"] == gruppen[j + 1]["fein_von"]
                   for j in range(len(gruppen) - 1)))
    p.wahr("jede feine Bandnummer in genau einer Gruppe",
           sorted(i for g in gruppen for i in range(g["fein_von"], g["fein_bis_exkl"]))
           == list(range(n)))

    grobkanten = [aus_hex64(h) for h in grob["kanten_hz"]["hex64"]]
    p.gleich("grobe Kantenzahl", len(grobkanten), GROB_ANZAHL + 1)
    p.wahr("grobe Kanten sind BITGLEICHE Kopien feiner Kanten",
           all(hex64(grobkanten[j]) == hex64(kanten[gruppen[j]["fein_von"]])
               for j in range(GROB_ANZAHL))
           and hex64(grobkanten[-1]) == hex64(kanten[n]))
    p.wahr("grobe Kanten streng monoton",
           all(grobkanten[j] < grobkanten[j + 1] for j in range(len(grobkanten) - 1)))
    p.wahr("aeussere Kanten stimmen ueberein",
           hex64(grobkanten[0]) == hex64(kanten[0]) and hex64(grobkanten[-1]) == hex64(kanten[-1]))

    return p


# ------------------------------------------------------------------ Hauptlauf

def schreibe(pfad: pathlib.Path, inhalt: dict) -> str:
    text = json.dumps(inhalt, indent=2, ensure_ascii=False) + "\n"
    roh = text.encode("utf-8")
    pfad.parent.mkdir(parents=True, exist_ok=True)
    pfad.write_bytes(roh)
    return hashlib.sha256(roh).hexdigest()


def main(argv: list[str]) -> int:
    nur_pruefen = "--pruefen" in argv

    fein = feines_gitter()
    grob = grobes_gitter(fein)

    p = pruefe(fein, grob)
    print(f"{fein['band_anzahl']} Baender (fein, {FEIN_ID})")
    print(f"{grob['band_anzahl']} Gruppen (grob, {GROB_ID}), Summe "
          f"{sum(g['anzahl'] for g in grob['gruppen'])}")
    gr = sorted({g["anzahl"] for g in grob["gruppen"]})
    print(f"Gruppengroessen: {gr}")
    print(f"Kanten gesamt: {aus_hex64(fein['kanten_hz']['hex64'][0]):.6f} Hz .. "
          f"{aus_hex64(fein['kanten_hz']['hex64'][-1]):.6f} Hz")
    print(f"Mitten gesamt: {aus_hex64(fein['mitten_hz']['hex64'][0]):.6f} Hz .. "
          f"{aus_hex64(fein['mitten_hz']['hex64'][-1]):.6f} Hz")
    print(f"Pruefungen: {p.ok} bestanden, {len(p.fehler)} gescheitert")
    for f in p.fehler:
        print(f"  ROT: {f}")
    if p.fehler:
        return 2

    if nur_pruefen:
        # Gegen die Dateien auf Platte pruefen, nicht nur gegen die Rechnung.
        for pfad, inhalt in ((ZIEL / f"{FEIN_ID}.json", fein), (ZIEL / f"{GROB_ID}.json", grob)):
            if not pfad.exists():
                print(f"  ROT: {pfad} fehlt")
                return 3
            soll = (json.dumps(inhalt, indent=2, ensure_ascii=False) + "\n").encode("utf-8")
            if pfad.read_bytes() != soll:
                print(f"  ROT: {pfad} weicht von der Neuerzeugung ab")
                return 2
            print(f"  bytegleich: {pfad.relative_to(WURZEL)} "
                  f"sha256={hashlib.sha256(soll).hexdigest()}")
        return 0

    for pfad, inhalt in ((ZIEL / f"{FEIN_ID}.json", fein), (ZIEL / f"{GROB_ID}.json", grob)):
        h = schreibe(pfad, inhalt)
        print(f"  geschrieben: {pfad.relative_to(WURZEL)} sha256={h}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
