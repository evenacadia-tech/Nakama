#!/usr/bin/env python3
"""SONDE-009 — erzeugt die einkompilierten Bandgitter aus den eingefrorenen
Fixtures und prueft, dass der committete Header noch bytegleich dazu ist.

WARUM EIN GENERATOR UND KEINE RECHENVORSCHRIFT IM C++.
CLAUDE.md, Technik-Zement zu SONDE-005a: "Bandgitter sind eingefrorene
Hex-Zahlen, keine Rechenvorschrift."  Der Grund ist nicht Bequemlichkeit,
sondern Reproduzierbarkeit: `std::pow(2.0, k/24.0)` liefert je nach libm-Fassung
verschiedene letzte Bits.  Ein Gitter, dessen Zahlen ein Empfaenger
NACHRECHNET, ist deshalb keine gemeinsame Wahrheit mehr, sobald zwei Maschinen
im Spiel sind.  Die Fixture traegt hex64 - IEEE-754-Bitmuster - und genau die
wandern hier in den Header.

WARUM TROTZDEM EIN ZWEITER PRUEFER (Kanon-Bein B5).
Dieses Skript ist der ERZEUGER.  Ein Erzeuger, der sein eigenes Erzeugnis
prueft, ist eine Tautologie: er vergleicht seine Ausgabe mit seiner Ausgabe.
Deshalb liest das C++-Bein `EqCopAnalysisGoldenTest` die Fixture ZUR TESTZEIT
noch einmal und misst sie gegen die einkompilierten Konstanten - dasselbe
Muster wie `quantisierung-v1.json` gegen die beiden Binaerleser aus SONDE-005b.
Zwei Wege zur selben Zahl, sonst waere die Pruefung Zeremonie.

Aufruf:
    py -3.13 tools/eq-copilot/erzeuge_bandgitter_header.py            # schreiben
    py -3.13 tools/eq-copilot/erzeuge_bandgitter_header.py --pruefen  # nur messen
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

WURZEL = Path(__file__).resolve().parents[2]
GITTER = WURZEL / "eq-copilot" / "schemas" / "v3" / "bandgitter"
ZIEL = WURZEL / "eq-copilot" / "plugin" / "core" / "analysis" / "BandGridZahlen.h"

EVIDENZ = "nakama_1_24_oct_30_18k_v1"
LIVE = "nakama_log64_v1"


def lade(name: str) -> dict:
    with (GITTER / f"{name}.json").open(encoding="utf-8") as f:
        return json.load(f)


def hexblock(werte: list[str], einrueckung: str = "    ") -> str:
    """Vier Hexliteral je Zeile — der Header bleibt lesbar und diffbar."""
    zeilen = []
    for i in range(0, len(werte), 4):
        stueck = ", ".join(f"0x{h}ull" for h in werte[i : i + 4])
        zeilen.append(einrueckung + stueck + ",")
    return "\n".join(zeilen)


def zahlblock(werte: list[int], einrueckung: str = "    ") -> str:
    zeilen = []
    for i in range(0, len(werte), 16):
        zeilen.append(einrueckung + ", ".join(str(w) for w in werte[i : i + 16]) + ",")
    return "\n".join(zeilen)


def baue() -> str:
    ev = lade(EVIDENZ)
    li = lade(LIVE)

    ev_kanten = ev["kanten_hz"]["hex64"]
    ev_mitten = ev["mitten_hz"]["hex64"]
    li_kanten = li["kanten_hz"]["hex64"]
    li_mitten = li["mitten_hz"]["hex64"]
    gruppen = li["gruppen"]

    # Sanity: die Fixture beschreibt sich selbst - wenn ihre eigenen
    # Anzahl-Felder nicht zu ihren Listen passen, ist etwas kaputt, und der
    # Header waere ein stiller Traeger dieses Fehlers.
    pruefe_selbst(ev, li, ev_kanten, ev_mitten, li_kanten, li_mitten, gruppen)

    n_ev = ev["band_anzahl"]
    n_li = li["band_anzahl"]

    return f"""// ERZEUGT von tools/eq-copilot/erzeuge_bandgitter_header.py — NICHT VON HAND
// AENDERN.  Quelle sind die eingefrorenen Fixtures aus SONDE-005a:
//   eq-copilot/schemas/v3/bandgitter/{EVIDENZ}.json
//   eq-copilot/schemas/v3/bandgitter/{LIVE}.json
//
// Die Zahlen stehen als IEEE-754-binary64-BITMUSTER da, nicht als Dezimaltext.
// Ein Dezimalliteral laeuft durch den Compiler-Parser; ein Bitmuster nicht.
// Bei einer Zahl wie 30.360373161050177 ist das der Unterschied zwischen
// "derselbe Wert" und "derselbe Wert bis auf das letzte Bit" — und ein Gitter,
// das zwei Prozesse verschieden runden, ist kein gemeinsamer Vertrag mehr.
//
// Kanon-Bein B5 (`EqCopAnalysisGoldenTest`) liest die Fixture zur Testzeit
// erneut und misst sie bytegleich gegen diese Konstanten.  Dieses Skript ist
// nur der Erzeuger — es beglaubigt sich nicht selbst.
#pragma once

#include <cstdint>

namespace nakama::analyse::gitter
{{

// ── Evidenzgitter `{EVIDENZ}` (§33.2, 1–4 Hz) ──
inline constexpr int kEvidenzBaender = {n_ev};

inline constexpr std::uint64_t kEvidenzKantenBits[{len(ev_kanten)}] = {{
{hexblock(ev_kanten)}
}};

inline constexpr std::uint64_t kEvidenzMittenBits[{len(ev_mitten)}] = {{
{hexblock(ev_mitten)}
}};

// ── Livegitter `{LIVE}` (§33.2, 10 Hz) ──
// Reine GRUPPIERUNG des Evidenzgitters, keine zweite Filterbank: "Das
// Livespektrum ist die lineare Energiesumme der Evidenzbaender" (Fixture,
// Feld `herkunft.warum`).  Deshalb sind die groben Kanten bitgleiche Kopien
// feiner Kanten — und deshalb darf die Live-Summe NUR linear gebildet werden.
inline constexpr int kLiveBaender = {n_li};

inline constexpr std::uint16_t kLiveGruppeVon[{len(gruppen)}] = {{
{zahlblock([g["fein_von"] for g in gruppen])}
}};

inline constexpr std::uint16_t kLiveGruppeBisExkl[{len(gruppen)}] = {{
{zahlblock([g["fein_bis_exkl"] for g in gruppen])}
}};

inline constexpr std::uint64_t kLiveKantenBits[{len(li_kanten)}] = {{
{hexblock(li_kanten)}
}};

inline constexpr std::uint64_t kLiveMittenBits[{len(li_mitten)}] = {{
{hexblock(li_mitten)}
}};

}} // namespace nakama::analyse::gitter
"""


def pruefe_selbst(ev, li, ev_kanten, ev_mitten, li_kanten, li_mitten, gruppen) -> None:
    fehler: list[str] = []
    if len(ev_mitten) != ev["band_anzahl"]:
        fehler.append(f"Evidenz: {len(ev_mitten)} Mitten, band_anzahl {ev['band_anzahl']}")
    if len(ev_kanten) != ev["band_anzahl"] + 1:
        fehler.append(f"Evidenz: {len(ev_kanten)} Kanten, erwartet {ev['band_anzahl'] + 1}")
    if len(li_mitten) != li["band_anzahl"]:
        fehler.append(f"Live: {len(li_mitten)} Mitten, band_anzahl {li['band_anzahl']}")
    if len(li_kanten) != li["band_anzahl"] + 1:
        fehler.append(f"Live: {len(li_kanten)} Kanten, erwartet {li['band_anzahl'] + 1}")
    if len(gruppen) != li["band_anzahl"]:
        fehler.append(f"Live: {len(gruppen)} Gruppen, band_anzahl {li['band_anzahl']}")

    # Die Partition muss LUECKENLOS und UEBERSCHNEIDUNGSFREI ueber alle 221
    # feinen Baender laufen.  Eine Luecke waere Energie, die im Livespektrum
    # verschwindet; eine Ueberschneidung waere Energie, die doppelt zaehlt.
    erwartet = 0
    for g in gruppen:
        if g["fein_von"] != erwartet:
            fehler.append(f"Gruppe {g['index']}: fein_von {g['fein_von']}, erwartet {erwartet}")
        if g["fein_bis_exkl"] - g["fein_von"] != g["anzahl"]:
            fehler.append(f"Gruppe {g['index']}: anzahl {g['anzahl']} passt nicht zu den Grenzen")
        erwartet = g["fein_bis_exkl"]
    if erwartet != ev["band_anzahl"]:
        fehler.append(f"Partition endet bei {erwartet}, erwartet {ev['band_anzahl']}")

    # Und die groben Kanten sind bitgleiche KOPIEN feiner Kanten (Fixture,
    # `herkunft.warum`).  Steht das nicht, ist das Livegitter eine zweite
    # Filterbank geworden, ohne dass es jemand gesagt hat.
    for g in gruppen:
        if li_kanten[g["index"]] != ev_kanten[g["fein_von"]]:
            fehler.append(f"Live-Kante {g['index']} ist keine Kopie der feinen Kante {g['fein_von']}")
    if li_kanten[-1] != ev_kanten[gruppen[-1]["fein_bis_exkl"]]:
        fehler.append("Letzte Live-Kante ist keine Kopie der letzten feinen Kante")

    if fehler:
        for f in fehler:
            print(f"FEHLER: {f}", file=sys.stderr)
        raise SystemExit(2)


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--pruefen", action="store_true",
                   help="nur messen, ob der committete Header bytegleich zur Fixture ist")
    args = p.parse_args()

    text = baue()
    if args.pruefen:
        if not ZIEL.exists():
            print(f"FEHLER: {ZIEL} fehlt", file=sys.stderr)
            return 2
        ist = ZIEL.read_text(encoding="utf-8")
        if ist != text:
            print("FEHLER: BandGridZahlen.h weicht von den Fixtures ab.", file=sys.stderr)
            print("        Neu erzeugen: py -3.13 tools/eq-copilot/erzeuge_bandgitter_header.py",
                  file=sys.stderr)
            # Erste abweichende Zeile nennen — sonst sucht der Naechste blind.
            a, b = ist.splitlines(), text.splitlines()
            for i in range(max(len(a), len(b))):
                za = a[i] if i < len(a) else "<Datei zu Ende>"
                zb = b[i] if i < len(b) else "<Datei zu Ende>"
                if za != zb:
                    print(f"        erste Abweichung Zeile {i + 1}:", file=sys.stderr)
                    print(f"          Header:  {za}", file=sys.stderr)
                    print(f"          Fixture: {zb}", file=sys.stderr)
                    break
            return 1
        zeilen = text.count("\n") + 1
        print(f"OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen ({zeilen} Zeilen).")
        print(f"    Evidenz {EVIDENZ}: 221 Baender, 222 Kanten")
        print(f"    Live    {LIVE}: 64 Gruppen, lueckenlose Partition der 221")
        return 0

    ZIEL.parent.mkdir(parents=True, exist_ok=True)
    ZIEL.write_text(text, encoding="utf-8", newline="\n")
    print(f"geschrieben: {ZIEL.relative_to(WURZEL)} ({text.count(chr(10)) + 1} Zeilen)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
