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

Sieben Pruefungen:

  1. Jedes Tabellenfeld traegt ein explizites `id:`.
  2. Je Tabelle: IDs lueckenlos ab 0, keine doppelt.
  3. Schema und FELD-IDS.json stimmen in JEDEM Name→ID-Paar ueberein.
  4. Keine unter `verbrannt` gefuehrte ID ist wieder in Gebrauch.
  5. Der Vertrag enthaelt keine `struct`-Typen — die koennen keine ids tragen
     und sind fuer immer unveraenderlich.
  6. Kein `include`: eine eingebundene Datei braechte Tabellen mit, die dieser
     Riegel nie sieht (T2-Runde 3, Befund 6).
  7. JEDES Offsetfeld (string, Vektor, Tabelle) ist im Strukturriegel des
     Rust-Beins genannt — siehe unten, `pruefe_strukturriegel`.

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
# Pruefung 7 liest die Rust-Quelle als TEXT. Absicht: sie soll rot werden,
# wenn jemand ein Offsetfeld ergaenzt und den Riegel vergisst — dafuer muss
# sie die Riegelzeilen sehen, nicht das uebersetzte Verhalten.
RIEGEL = WURZEL / "broker/src/telemetrie.rs"


def blockkommentare_entfernen(text: str) -> str:
    """Entfernt `/* ... */`, ohne in Zeichenketten zu schneiden.

    T2-Runde 3, Befund 5: der Riegel kannte sie nicht. Gemessen wurde
    `/* table Geist { a:int; b:int; } */` - `flatc` akzeptiert das (Exit 0),
    der Riegel meldete 9 Tabellen und 4x ROT. Der Irrtum ging in die SICHERE
    Richtung (falsches Rot), widerlegte aber den Anspruch dieses Parsers,
    Kommentare zu verstehen. Ein Riegel, der aus dem falschen Grund rot wird,
    ist beim naechsten Mal aus dem falschen Grund gruen.
    """
    heraus: list[str] = []
    i, n, in_string = 0, len(text), False
    while i < n:
        c = text[i]
        if in_string:
            if c == "\\":
                heraus.append(text[i:i + 2])
                i += 2
                continue
            if c == '"':
                in_string = False
            heraus.append(c)
            i += 1
            continue
        if c == '"':
            in_string = True
            heraus.append(c)
            i += 1
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            ende = text.find("*/", i + 2)
            j = n if ende < 0 else ende + 2
            # Zeilenumbrueche erhalten, damit Zeilennummern nicht verrutschen.
            heraus.append("\n" * text.count("\n", i, j))
            i = j
            continue
        heraus.append(c)
        i += 1
    return "".join(heraus)


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
# name : TYP [= default] [(attribute)]  — der Typ ist alles bis `=` oder `(`.
TYP_RE = re.compile(r"^\s*[A-Za-z_][A-Za-z0-9_]*\s*:\s*([^=(]+?)\s*(?:=|\(|$)")


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


def tabellen_lesen(text: str) -> dict[str, list[tuple[str, int | None, str, bool]]]:
    """Je Tabelle die Felder als (name, id, typ, required).

    `typ` und `required` braucht Pruefung 7; die anderen vier lesen weiter nur
    name und id. EIN Parser fuer alle — ein zweiter waere die zweite Wahrheit
    darueber, was in dieser Datei steht.
    """
    tabellen: dict[str, list[tuple[str, int | None, str, bool]]] = {}
    for treffer in TABELLE_RE.finditer(text):
        name = treffer.group(1)
        inhalt = block_lesen(text, treffer.end() - 1)
        felder: list[tuple[str, int | None, str, bool]] = []
        for roh in inhalt.split(";"):
            zeile = roh.strip()
            if not zeile:
                continue
            m = FELD_RE.match(zeile)
            if not m:
                continue
            mid = ID_RE.search(zeile)
            mtyp = TYP_RE.match(zeile)
            typ = mtyp.group(1).strip() if mtyp else ""
            felder.append((m.group(1), int(mid.group(1)) if mid else None, typ,
                           bool(re.search(r"\brequired\b", zeile))))
        tabellen[name] = felder
    return tabellen


def ist_offsetfeld(typ: str, tabellennamen: set[str]) -> bool:
    """Traegt dieses Feld einen `uoffset` — also eine Zelle, die 0 sein kann?

    Genau drei Sorten: Zeichenketten, Vektoren und Tabellen. Skalare und Enums
    liegen INLINE in der Tabelle; ihre Zelle ist keine Adresse und ein 0-Byte
    darin ist ein Wert, kein Selbstbezug. `struct` gibt es in diesem Vertrag
    nicht (Pruefung 5).
    """
    return typ == "string" or typ.startswith("[") or typ in tabellennamen


def pruefe_strukturriegel(tabellen, riegelquelle: str) -> list[str]:
    """T2-Runde 4, BL-A: jedes Offsetfeld MUSS im Rust-Strukturriegel stehen.

    Der Riegel spiegelt C++' `VerifyOffset`-Regel „May not point to itself",
    die dem Rust-Verifier fehlt. Er ist handgeschrieben — ein spaeter
    ergaenztes Offsetfeld liefe still an ihm vorbei, und die beiden Beine
    klassifizierten wieder verschieden. Gemessen war das an 6215 mutierten
    Puffern (143 auseinander vor dem Riegel, 0 danach); diese Pruefung haelt
    das Ergebnis, ohne die Messung jedes Mal zu wiederholen.

    Zusaetzlich der Sonderfall, der die Messung UEBERLEBT hat: bei einem
    Vektor von Tabellen ist jedes ELEMENT ein eigener uoffset. Er braucht
    keine eigene Riegelzeile, solange die Elementtabelle mindestens ein
    `required`-Feld traegt — dann faellt eine feldlose Tabelle schon in BEIDEN
    Verifiern. Faellt das `required` weg, faellt diese Begruendung mit, und
    zwar still. Deshalb steht sie hier als Pruefung.
    """
    namen = set(tabellen)
    fehler: list[str] = []
    for tab, felder in sorted(tabellen.items()):
        for feld, _id, typ, _req in felder:
            if not ist_offsetfeld(typ, namen):
                continue
            marke = f"{tab}::VT_{feld.upper()}"
            if marke not in riegelquelle:
                fehler.append(
                    f"Strukturriegel kennt {marke} nicht ({tab}.{feld}: {typ}) — "
                    "ein Offsetfeld ohne Riegelzeile ist die Luecke aus T2-Runde 4.")
            if typ.startswith("[") and typ[1:-1] in namen:
                element = typ[1:-1]
                if not any(r for _f, _i, _t, r in tabellen[element]):
                    fehler.append(
                        f"{tab}.{feld} ist ein Vektor von {element}, und {element} "
                        "traegt kein `required`-Feld mehr — damit faellt die "
                        "Begruendung, warum die Elementoffsets keinen eigenen "
                        "Riegel brauchen.")
    return fehler


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
    text = kommentare_entfernen(blockkommentare_entfernen(roh))
    tabellen = tabellen_lesen(text)
    frozen = json.loads(IDS.read_text(encoding="utf-8"))
    erwartet = frozen["tabellen"]

    fehler: list[str] = []
    ohne_id = 0
    felder_gesamt = 0

    # 6. `include` — T2-Runde 3, Befund 6: eine eingebundene Datei bringt
    #    Tabellen mit, die dieser Riegel nie sieht. Heute folgenlos, weil
    #    `flatc --cpp` fuer die eingebundene Datei keinen Code erzeugt - mit
    #    `--gen-all` oder einem zweiten Codegen-Aufruf waere es ein echtes
    #    Loch. Der Vertrag ist EINE Datei, und das steht jetzt hier.
    if re.search(r"^\s*include\s", text, re.MULTILINE):
        fehler.append(
            "`include` im Vertrag: eine eingebundene Datei bringt Tabellen mit, die "
            "dieser Riegel nicht sieht. Der Telemetrievertrag ist EINE Datei.")

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
        fehlend = [f for f, i, _t, _r in felder if i is None]
        ohne_id += len(fehlend)
        if fehlend:
            fehler.append(f"{name}: Felder ohne id: {', '.join(fehlend)}")

        ids = [i for _f, i, _t, _r in felder if i is not None]

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
        ist = {f: i for f, i, _t, _r in felder}
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

    # 7. Offsetfeld-Abdeckung des Rust-Strukturriegels (T2-Runde 4, BL-A).
    offsetfelder = sum(
        1 for _t, fs in tabellen.items() for _f, _i, typ, _r in fs
        if ist_offsetfeld(typ, set(tabellen)))
    if RIEGEL.exists():
        fehler.extend(pruefe_strukturriegel(tabellen, RIEGEL.read_text(encoding="utf-8")))
    else:
        fehler.append(f"Strukturriegel-Quelle fehlt: {RIEGEL}")

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
    print(f"Offsetfelder (string/Vektor/Tabelle) im Strukturriegel: {offsetfelder}")
    if not fehler:
        print("keine Luecke, keine Doppelung, keine Abweichung zur eingefrorenen Liste")
    for f in fehler:
        print(f"  ROT: {f}")
    print(f"Pruefungen: {'0 rot' if not fehler else f'{len(fehler)} rot'}")
    return 0 if not fehler else 2


if __name__ == "__main__":
    sys.exit(main())
