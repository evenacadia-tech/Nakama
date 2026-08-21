#!/usr/bin/env python3
"""Riegel fuer die Feld-ID-Disziplin des FlatBuffers-Vertrags (SONDE-005b).

Entwurf §53 verlangt zwei Dinge:

    "FlatBuffers-Felder erhalten explizite numerische id-Attribute.
     IDs werden nie wiederverwendet, auch nicht nach Entfernen eines Feldes."

`flatc` erzwingt davon nur einen Teil, und zwar genau den unwichtigeren:
BENUTZT eine Tabelle ids, muessen alle ihre Felder welche haben und sie muessen
bei 0 lueckenlos sein. Eine Tabelle GANZ OHNE ids uebersetzt anstandslos — sie
faellt dann still auf "Reihenfolge ist Identitaet" zurueck, und der naechste,
der ein Feld einsortiert statt anhaengt, aendert das Wireformat, ohne dass
irgendetwas rot wird.

Den zweiten Satz kann `flatc` grundsaetzlich nicht pruefen: eine Datei kennt
ihre eigene Vergangenheit nicht. Deshalb liegt die Vergangenheit als eigene,
handgeschriebene Datei daneben (FELD-IDS.json).

Fuenf Pruefungen:

  1. Jedes Tabellenfeld traegt ein explizites `id:`.
  2. Je Tabelle: IDs lueckenlos ab 0, keine doppelt.
  3. Schema und FELD-IDS.json stimmen in JEDEM Name→ID-Paar ueberein.
  4. Keine unter `verbrannt` gefuehrte ID ist wieder in Gebrauch.
  5. Der Vertrag enthaelt keine `struct`-Typen — die koennen keine ids tragen
     und sind fuer immer unveraenderlich.

Exitcodes: 0 alles gruen · 2 mindestens eine Pruefung rot · 3 Datei fehlt.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

WURZEL = Path(__file__).resolve().parents[2]
FBS = WURZEL / "eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs"
IDS = WURZEL / "eq-copilot/schemas/v3/flatbuffers/FELD-IDS.json"


def kommentare_entfernen(text: str) -> str:
    """Entfernt `//`-Kommentare, ohne in Zeichenketten zu schneiden.

    Ohne die Zeichenkettenbehandlung wuerde ein `file_identifier "a//b"` die
    Zeile abschneiden. Heute kommt das im Vertrag nicht vor — aber ein Parser,
    der nur fuer den heutigen Inhalt richtig ist, ist ein Parser, der beim
    naechsten Feld falsch wird.
    """
    heraus: list[str] = []
    for zeile in text.splitlines():
        in_string = False
        i = 0
        while i < len(zeile):
            z = zeile[i]
            if z == '"' and (i == 0 or zeile[i - 1] != "\\"):
                in_string = not in_string
            elif z == "/" and not in_string and i + 1 < len(zeile) and zeile[i + 1] == "/":
                zeile = zeile[:i]
                break
            i += 1
        heraus.append(zeile)
    return "\n".join(heraus)


TABELLE_RE = re.compile(r"\btable\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{", re.MULTILINE)
STRUCT_RE = re.compile(r"\bstruct\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{", re.MULTILINE)
FELD_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_]*)\s*:")
ID_RE = re.compile(r"\bid\s*:\s*(\d+)\b")


def block_lesen(text: str, offen: int) -> str:
    """Gibt den Inhalt zwischen `offen` (Index der `{`) und der passenden `}`."""
    tiefe = 0
    for i in range(offen, len(text)):
        if text[i] == "{":
            tiefe += 1
        elif text[i] == "}":
            tiefe -= 1
            if tiefe == 0:
                return text[offen + 1 : i]
    raise ValueError("unbalancierte Klammer im Schema")


def tabellen_lesen(text: str) -> dict[str, list[tuple[str, int | None]]]:
    tabellen: dict[str, list[tuple[str, int | None]]] = {}
    for treffer in TABELLE_RE.finditer(text):
        name = treffer.group(1)
        inhalt = block_lesen(text, treffer.end() - 1)
        felder: list[tuple[str, int | None]] = []
        for roh in inhalt.split(";"):
            zeile = roh.strip()
            if not zeile:
                continue
            m = FELD_RE.match(zeile)
            if not m:
                continue
            mid = ID_RE.search(zeile)
            felder.append((m.group(1), int(mid.group(1)) if mid else None))
        tabellen[name] = felder
    return tabellen


def main() -> int:
    # Zwei optionale Pfade, damit der Riegel an einer MUTIERTEN Kopie
    # vorgefuehrt werden kann, ohne den Vertrag anzufassen. Ein Riegel, den
    # niemand fallen gesehen hat, ist eine Behauptung.
    global FBS, IDS
    if len(sys.argv) >= 2:
        FBS = Path(sys.argv[1]).resolve()
    if len(sys.argv) >= 3:
        IDS = Path(sys.argv[2]).resolve()

    for datei in (FBS, IDS):
        if not datei.exists():
            print(f"FEHLT: {datei}", file=sys.stderr)
            return 3

    roh = FBS.read_text(encoding="utf-8")
    text = kommentare_entfernen(roh)
    tabellen = tabellen_lesen(text)
    frozen = json.loads(IDS.read_text(encoding="utf-8"))
    erwartet = frozen["tabellen"]

    fehler: list[str] = []
    ohne_id = 0
    felder_gesamt = 0

    # 5. struct-Typen
    strukturen = [m.group(1) for m in STRUCT_RE.finditer(text)]
    if strukturen:
        fehler.append(
            f"struct-Typen im Vertrag: {', '.join(strukturen)} — structs koennen "
            "keine Feld-IDs tragen und sind fuer immer unveraenderlich."
        )

    for name, felder in sorted(tabellen.items()):
        felder_gesamt += len(felder)

        # 1. jedes Feld hat eine id
        fehlend = [f for f, i in felder if i is None]
        ohne_id += len(fehlend)
        if fehlend:
            fehler.append(f"{name}: Felder ohne id: {', '.join(fehlend)}")

        ids = [i for _, i in felder if i is not None]

        # 2. lueckenlos ab 0, keine Doppelung
        if len(set(ids)) != len(ids):
            doppelt = sorted({i for i in ids if ids.count(i) > 1})
            fehler.append(f"{name}: doppelte id(s) {doppelt}")
        if ids and sorted(ids) != list(range(len(ids))):
            fehler.append(f"{name}: ids nicht lueckenlos ab 0 — {sorted(ids)}")

        # 3. Abgleich gegen die eingefrorene Liste
        if name not in erwartet:
            fehler.append(f"{name}: Tabelle fehlt in FELD-IDS.json (neue Tabelle bitte eintragen)")
            continue
        soll = erwartet[name]["felder"]
        ist = {f: i for f, i in felder}
        if ist != soll:
            nur_schema = {k: v for k, v in ist.items() if soll.get(k) != v}
            nur_frozen = {k: v for k, v in soll.items() if ist.get(k) != v}
            fehler.append(
                f"{name}: Schema und FELD-IDS.json weichen ab — "
                f"im Schema {nur_schema}, eingefroren {nur_frozen}"
            )

        # 4. verbrannte ids
        verbrannt = set(erwartet[name].get("verbrannt", []))
        wieder = sorted(verbrannt & set(ids))
        if wieder:
            fehler.append(
                f"{name}: verbrannte id(s) {wieder} sind wieder in Gebrauch — "
                "eine entfernte id darf nie neu vergeben werden."
            )

    for name in erwartet:
        if name not in tabellen:
            fehler.append(f"{name}: in FELD-IDS.json gefuehrt, aber nicht mehr im Schema")

    # Die Gesamtzahlen sind eine billige Pruefsumme gegen einen Parser, der
    # still die Haelfte uebersieht.
    gesamt = frozen.get("erwartet_gesamt", {})
    if gesamt.get("tabellen") != len(tabellen):
        fehler.append(f"Tabellenzahl: erwartet {gesamt.get('tabellen')}, gemessen {len(tabellen)}")
    if gesamt.get("felder") != felder_gesamt:
        fehler.append(f"Feldzahl: erwartet {gesamt.get('felder')}, gemessen {felder_gesamt}")

    try:
        gezeigt = FBS.relative_to(WURZEL)
    except ValueError:      # mutierte Kopie ausserhalb des Baums
        gezeigt = FBS
    print(f"Schema:   {gezeigt}")
    print(f"Tabellen: {len(tabellen)}")
    print(f"Felder:   {felder_gesamt}")
    print(f"Felder ohne id: {ohne_id}")
    if not fehler:
        print("keine Luecke, keine Doppelung, keine Abweichung zur eingefrorenen Liste")
    for f in fehler:
        print(f"  ROT: {f}")
    print(f"Pruefungen: {'0 rot' if not fehler else f'{len(fehler)} rot'}")
    return 0 if not fehler else 2


if __name__ == "__main__":
    sys.exit(main())
