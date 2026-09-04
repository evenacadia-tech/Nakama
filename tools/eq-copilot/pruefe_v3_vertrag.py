#!/usr/bin/env python3
"""Referenzbein des v3-Vertrags (SONDE-005a).

Drei Aufgaben, die keine der beiden eigenen Engines uebernehmen kann:

1. **Das Schema selbst pruefen** — gegen das Metaschema von draft 2020-12.
2. **Die Engine-Teilmenge durchsetzen** — kein Schluesselwort und kein
   `pattern` im Schema, das die beiden handgeschriebenen Engines nicht
   implementieren. JSON Schema ignoriert Unbekanntes absichtlich; ohne diese
   Pruefung verschwaende eine spaeter ergaenzte Einschraenkung still auf zwei
   von drei Seiten.
3. **Das Urteil unabhaengig nachrechnen** — mit `jsonschema`, einer fremden
   Implementierung, gegen dasselbe MANIFEST wie C++ und Rust.
4. **Den Fassungsschritt beweisen** (seit SONDE-013/P4) — der Vertrag ist an
   Discriminator, Zieladresse, Revision und Capability NICHT additiv
   erweiterbar. `fassung_1_schema()` baut aus der committeten Fassung 2 die
   Fassung 1 zurueck (die Liste dafuer ist der `fassungen`-Eintrag im
   Register, keine zweite Kopie) und `pruefe_sonde013_fassung_2` misst, dass
   der alte Leser jede Neuerung ABLEHNT statt sie still auf einen bekannten
   Zweig abzubilden. Ohne diesen Nachweis waere „nicht additiv erweiterbar"
   eine Behauptung ohne Messung.

Die VERLETZUNGSMENGE prueft dieses Bein bewusst nicht: eine echte
JSON-Schema-Implementierung meldet bei `oneOf` die Fehler aller Zweige,
waehrend die eigenen Engines ueber den Discriminator in genau einen absteigen.
Beides ist korrekt, nur nicht vergleichbar (README).

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py
    py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung
"""

from __future__ import annotations

import copy
import json
import math
import pathlib
import re
import sys

try:
    import jsonschema
except ImportError:
    print("VORAUSSETZUNG FEHLT: py -3.13 -m pip install jsonschema")
    sys.exit(3)

WURZEL = pathlib.Path(__file__).resolve().parents[2]
SCHEMA = WURZEL / "eq-copilot" / "schemas" / "v3" / "eq-ipc-v3.schema.json"
RESERVIERT = WURZEL / "eq-copilot" / "schemas" / "v3" / "reservierte-nachrichten-v1.json"
QUANTISIERUNG = WURZEL / "eq-copilot" / "schemas" / "v3" / "quantisierung-v1.json"
METRIKEN = WURZEL / "eq-copilot" / "schemas" / "v3" / "metriken-v1.json"
FIXTURES = WURZEL / "eq-copilot" / "fixtures" / "v3"
MAX_DOKUMENT_BYTES = 16 * 1024 * 1024

# Genau die Liste aus schemas/v3/README.md. Wer sie hier erweitert, muss sie
# in BEIDEN Engines und im README erweitern - sonst faellt diese Pruefung.
SCHLUESSELWOERTER = {
    "$ref", "type", "const", "enum",
    "required", "properties", "additionalProperties", "maxProperties",
    "minimum", "maximum", "exclusiveMinimum", "exclusiveMaximum",
    "minLength", "maxLength", "pattern",
    "items", "minItems", "maxItems",
    "oneOf", "x-nakama-discriminator",
}
ANMERKUNGEN = {"$schema", "$id", "title", "description", "$comment", "$defs"}

# Geschlossene Mustertabelle — dieselbe Menge wie `musterPasst` (NakamaVertrag.cpp)
# und `muster_passt` (broker/src/vertrag.rs). Ein Muster, das hier fehlt, bricht
# den Ladevorgang, statt still zu gelten.
#
# 🔑 Jedes Muster steht im Schema NEBEN einem festen minLength == maxLength.
# Das ist keine Doppelung, sondern der Grund, warum die drei Beine hier zum
# selben Urteil kommen: Pythons `re` laesst `$` auch VOR einem abschliessenden
# Zeilenumbruch passen, die beiden Handschleifen in C++ und Rust nicht. Die
# Laengenschranke faengt den Umbruch, bevor die Ankersemantik zaehlt
# (Fixture `state-hash-mit-umbruch`, G1-Nacharbeit 24.08.).
MUSTER = {
    "^[0-9a-f]{32}$",
    "^[0-9a-f]{64}$",
    "^[A-Za-z0-9+/]{37}[AQgw]==$",
    "^[A-Za-z0-9+/]{10}[AEIMQUYcgkosw048]=$",
    r"^(?![\s\S]*[\u0000-\u001F\u007F-\u009F])(?=[\s\S]*\S)[\s\S]+$",
}

HOST_BUS_NAME_PATTERN = (
    r"^(?![\s\S]*[\u0000-\u001F\u007F-\u009F])(?=[\s\S]*\S)[\s\S]+$")
HOST_DESCRIPTOR_FELDER = {"host_bus_name", "host_mixer_index"}


class Lauf:
    def __init__(self) -> None:
        self.ok = 0
        self.fehler: list[str] = []

    def wahr(self, name: str, bedingung: bool, zusatz: str = "") -> None:
        if bedingung:
            self.ok += 1
        else:
            self.fehler.append(name + (f" — {zusatz}" if zusatz else ""))


# ------------------------------------------------------------------ Schemalauf

def teilschemata(knoten, pfad: str):
    """Liefert (pfad, teilschema) fuer jedes Teilschema, Wurzel eingeschlossen."""
    if not isinstance(knoten, dict):
        return
    yield pfad, knoten
    for name, wert in knoten.items():
        if name in ("properties", "$defs"):
            for k, v in wert.items():
                yield from teilschemata(v, f"{pfad}/{name}/{k}")
        elif name == "items":
            yield from teilschemata(wert, f"{pfad}/items")
        elif name == "oneOf":
            for i, v in enumerate(wert):
                yield from teilschemata(v, f"{pfad}/oneOf/{i}")


SICHERE_GANZZAHL = 9007199254740991      # 2**53 - 1
DEZ_GRENZE = 308                         # |x| < 1e308
BS = chr(92)                             # Backslash
HEX = "0123456789abcdefABCDEF"
FALLTABELLE = FIXTURES / "TEXTRIEGEL-FAELLE.json"


def json_konstante_ablehnen(name: str):
    raise ValueError(f"nicht-endliches JSON-Literal: {name}")


def json_laden_strikt(quelle):
    """JSON ohne Pythons nicht-standardisierte NaN-/Infinity-Erweiterungen.

    Der Textriegel bleibt fuer dieselbe Klassifikation samt Position in allen
    drei Sprachen noetig. `parse_constant` ist der unabhaengige zweite Riegel:
    selbst bei einer kuenftigen Scannerluecke erzeugt `json.loads` aus den
    Python-Erweiterungen niemals einen nicht-endlichen Wert.
    """
    return json.loads(quelle, parse_constant=json_konstante_ablehnen)


def ist_ascii_ziffer(c: str) -> bool:
    """NICHT `str.isdigit()`.

    T2-Runde 2, Befund BF-4: `str.isdigit()` ist auch fuer arabisch-indische
    Ziffern und Hochzahlen wahr, waehrend Rusts `is_ascii_digit` und C++
    `c >= '0' && c <= '9'` es nicht sind. Bei `{"w": 0\u0662}` meldete dieses
    Bein deshalb eine fuehrende Null und die anderen beiden nicht - drei
    Ziffernbegriffe waeren drei Grammatiken.
    """
    return "0" <= c <= "9"


def ist_ascii_buchstabe(c: str) -> bool:
    return "a" <= c <= "z" or "A" <= c <= "Z"


def zahl_pruefen(ganz: str, bruch: str, exp_ziffern: str, exp_negativ: bool,
                 lit: str, schema_ganzzahl_sichern: bool) -> str | None:
    """Entscheidet AUS DEM LITERAL, ob eine Zahl im Vertragsbereich liegt.

    T2-Runde 2, Blocker BL-1/BL-2: die erste Fassung fragte hier `float(lit)`
    bzw. auf der C++-Seite `getDoubleValue()`. Genau das war der Fehler -
    `getDoubleValue()` ist derselbe Leser, gegen dessen Ueberlauf der Riegel
    schuetzen soll. `juce_CharacterFunctions.h` akkumuliert den Exponenten in
    einem `int` OHNE Schranke, und der `max_exponent10`-Riegel laeuft DANACH;
    `1e4294967296` kam dort als **1.0** an, waehrend Rust und Python `inf`
    lasen. Der zweite Zweig (`extraExponent`) hat gar keinen Riegel und
    schreibt bei 1018 Vorkommastellen `':00'` statt eines Exponenten.

    Die Lehre, die ueber diesen Fall hinausgeht: **ein Riegel darf nie die
    Bibliothek befragen, gegen deren Verhalten er schuetzt.** Die erste
    Ganzzahlregel erfasste nur die Form ohne Punkt/Exponent; heute werden Wert,
    Ganzzahligkeit und Praezision fuer alle Schreibweisen lexikalisch bestimmt.
    Die Endlichkeitsregel war delegiert und hat ebenfalls nicht gehalten.

    Hier wird deshalb nur mit kleinen ganzen Zahlen gerechnet, ohne jede
    Gleitkommaoperation.
    """
    if not bruch and not exp_ziffern:
        if len(ganz) > 16 or (len(ganz) == 16 and int(ganz) > SICHERE_GANZZAHL):
            return f"Ganzzahl ausserhalb 2^53-1: {lit}"
        return None

    # Der Exponent selbst: mehr als drei Ziffern liegen schon ausserhalb, und
    # so wird er auch nie gross genug, um irgendwo ueberzulaufen.
    ohne_null = exp_ziffern.lstrip("0")
    if len(ohne_null) > 3:
        return f"Exponent ausserhalb +/-{DEZ_GRENZE}: {lit[:40]}"
    exp = int(ohne_null) if ohne_null else 0
    if exp_negativ:
        exp = -exp

    alle = ganz + bruch
    signifikant = alle.lstrip("0")
    if not signifikant:
        return None                       # der Wert ist exakt 0

    # Die Endlichkeitsgrenze hat Vorrang vor der engeren Ganzzahlregel, damit
    # ein 1e308-Ueberlauf sprachuebergreifend ein Zahlenbereichsfehler bleibt.
    fuehrende = len(alle) - len(signifikant)
    dez = (len(ganz) - fuehrende - 1) + exp
    if dez >= DEZ_GRENZE or dez <= -DEZ_GRENZE:
        return f"Zahl ausserhalb +/-1e{DEZ_GRENZE}: {lit[:40]}"

    # JSON Schema beurteilt den mathematischen Wert: auch 5.0 und 5e0 sind
    # Integer. Daher gilt die 2^53-Grenze fuer jede exakt ganzzahlige
    # Dezimal-/Exponentialschreibweise, noch bevor binary64 runden kann.
    skala = exp - len(bruch)               # alle * 10**skala
    ist_ganzzahl = False
    ganzzahl_zu_gross = False
    if skala >= 0:
        ist_ganzzahl = True
        stellen = len(signifikant) + skala
        if stellen > 16:
            ganzzahl_zu_gross = True
        elif stellen == 16:
            ganzzahl_zu_gross = signifikant + "0" * skala > "9007199254740991"
    else:
        abzuschneiden = -skala
        if (abzuschneiden <= len(alle)
                and alle[len(alle) - abzuschneiden:] == "0" * abzuschneiden):
            ist_ganzzahl = True
            normalisiert = alle[:len(alle) - abzuschneiden].lstrip("0")
            ganzzahl_zu_gross = (len(normalisiert) > 16
                                  or (len(normalisiert) == 16
                                      and normalisiert > "9007199254740991"))
    if ganzzahl_zu_gross:
        return f"Ganzzahl ausserhalb 2^53-1: {lit}"

    # Nichtganzzahlige Eingaben mit mehr als 15 signifikanten Dezimalziffern
    # koennen beim binary64-Lesen auf eine Ganzzahl kippen und damit einen
    # `type: integer`-Riegel umgehen. Exakte Integer haben oben die 2^53-Kante.
    signifikante_stellen = len(signifikant.rstrip("0"))
    if schema_ganzzahl_sichern and not ist_ganzzahl and signifikante_stellen > 15:
        return f"Zahl mit mehr als 15 signifikanten Dezimalziffern: {lit[:40]}"

    return None


def textriegel_bytes(roh: bytes, *, schema_ganzzahl_sichern: bool = True) -> str | None:
    """Der Riegel auf BYTE-Ebene — so, wie ein Dokument wirklich ankommt.

    Vier Regeln lassen sich nur hier ausdruecken (T2-Runde 2, BF-6/BF-7 und
    der Roh-NUL-Gegenpfad):

    * **BOM.** RFC 8259 §8.1: `serde_json` und Pythons `json` lehnen ein BOM
      ab, JUCEs `loadFileAsString` streift es und parst weiter.
    * **Kaputtes UTF-8.** Gemessen liefen die drei Beine hier voellig
      auseinander: dieses hier warf eine ungefangene `UnicodeDecodeError`, das
      Rust-Bein panickte beim Lesen, und JUCE ersetzte das Byte still.
    * **Rohes NUL.** Terminatorbasierte C++-Leser duerfen keinen gueltigen
      Praefix annehmen und die restliche Bytefolge ignorieren.
    * **Groesse.** Direkte DTO-/Datei-Caller sind wie C++ und Rust auf
      inklusive 16 MiB begrenzt; der Pipe-Framer ist mit 256 KiB enger.
    """
    if len(roh) > MAX_DOKUMENT_BYTES:
        return "Dokument zu gross"
    if roh.startswith(b"\xef\xbb\xbf"):
        return "BOM am Dokumentanfang"
    if b"\0" in roh:
        return "rohes NUL im Dokument"
    try:
        text = roh.decode("utf-8")
    except UnicodeDecodeError as e:
        return f"kein gueltiges UTF-8 an Byte {e.start}"
    return textriegel(text, schema_ganzzahl_sichern=schema_ganzzahl_sichern)


def textriegel(text: str, *, schema_ganzzahl_sichern: bool = True) -> str | None:
    """Prueft die Zeichen eines v3-Dokuments, BEVOR ein Parser sie sieht.

    Neun Regeln, jede gegen eine GEMESSENE Abweichung zwischen den Beinen.
    Auslegung und Begruendung: `eq-copilot/schemas/v3/README.md`.

    @returns None wenn sauber, sonst den Grund.
    """
    i, n = 0, len(text)
    while i < n:
        c = text[i]

        if c == '"':
            j, hoch = i + 1, None
            while True:
                if j >= n:
                    return "unbeendete Zeichenkette"
                d = text[j]
                if d == BS:
                    if j + 5 < n and text[j + 1] == "u":
                        roh = text[j + 2:j + 6]
                        # GENAU vier ASCII-Hexziffern (BF-2/BF-3): `int(roh, 16)`
                        # naehme "+123", " 12 ", "0x1f", "1_23" und
                        # arabisch-indische Ziffern; Rusts `from_str_radix` naehme
                        # das Vorzeichen; die C++-Handschleife nichts davon.
                        if len(roh) != 4 or any(z not in HEX for z in roh):
                            return f"kein 4-stelliges Hex-Escape an Position {j}"
                        cp = int(roh, 16)
                        if cp == 0:
                            return f"NUL-Escape in Zeichenkette an Position {j}"
                        tief = 0xDC00 <= cp <= 0xDFFF
                        if tief and hoch is None:
                            return f"einsames tiefes Surrogat U+{cp:04X} an Position {j}"
                        if hoch is not None and not tief:
                            return f"hohes Surrogat ohne Paar an Position {j}"
                        hoch = cp if 0xD800 <= cp <= 0xDBFF else None
                        j += 6
                        continue
                    if hoch is not None:
                        return f"hohes Surrogat ohne Paar an Position {j}"
                    j += 2
                    continue
                if hoch is not None:
                    return f"hohes Surrogat ohne Paar an Position {j}"
                if d == '"':
                    break
                if ord(d) < 0x20:
                    return f"rohes Steuerzeichen U+{ord(d):04X} an Position {j}"
                j += 1
            if hoch is not None:
                return "hohes Surrogat ohne Paar am Zeichenkettenende"
            leer = j == i + 1
            k = j + 1
            while k < n and text[k] in " \t\r\n":
                k += 1
            if leer and k < n and text[k] == ":":
                return f"leerer Objektschluessel an Position {i}"
            i = j + 1
            continue

        # JSON kennt ausserhalb von Zeichenketten genau drei alphabetische
        # Literale. Python akzeptiert zusaetzlich NaN/Infinity, JUCE und
        # serde_json nicht. Das optionale Minus gehoert zur Position und zum
        # gemeldeten Literal, damit -Infinity in allen drei Beinen gleich
        # klassifiziert wird.
        vorzeichen_vor_literal = (c == "-" and i + 1 < n
                                   and ist_ascii_buchstabe(text[i + 1]))
        if ist_ascii_buchstabe(c) or vorzeichen_vor_literal:
            j = i + (1 if c == "-" else 0)
            while j < n and ist_ascii_buchstabe(text[j]):
                j += 1
            literal = text[i:j]
            if literal not in ("true", "false", "null"):
                return f"unbekanntes Literal {literal} an Position {i}"
            i = j
            continue

        if c == "-" or ist_ascii_ziffer(c):
            j = i
            if text[j] == "-":
                j += 1
            anfang = j
            while j < n and ist_ascii_ziffer(text[j]):
                j += 1
            ganz = text[anfang:j]
            if not ganz:
                return f"Zahl ohne Ziffern an Position {i}"
            if len(ganz) > 1 and ganz[0] == "0":
                return f"fuehrende Null in {text[i:j][:20]!r} an Position {i}"

            bruch = ""
            if j < n and text[j] == ".":
                j += 1
                a = j
                while j < n and ist_ascii_ziffer(text[j]):
                    j += 1
                bruch = text[a:j]
                if not bruch:
                    return f"Dezimalpunkt ohne Nachkommaziffern an Position {i}"

            exp_ziffern, exp_negativ = "", False
            if j < n and text[j] in "eE":
                j += 1
                if j < n and text[j] in "+-":
                    exp_negativ = text[j] == "-"
                    j += 1
                a = j
                while j < n and ist_ascii_ziffer(text[j]):
                    j += 1
                exp_ziffern = text[a:j]
                if not exp_ziffern:
                    # BF-1: getDoubleValue("1e") liefert 1.0.
                    return f"Exponent ohne Ziffern an Position {i}"

            grund = zahl_pruefen(ganz, bruch, exp_ziffern, exp_negativ, text[i:j],
                                 schema_ganzzahl_sichern)
            if grund:
                return grund
            i = j
            continue

        i += 1
    return None


def pruefe_textriegel(lauf: Lauf) -> None:
    """Faehrt die GEMEINSAME Falltabelle.

    T2-Runde 2, BF-5: vorher trug jedes Bein eine eigene Kopie - gezaehlt 31,
    32 und 33 Faelle -, waehrend das Beweismanifest 'dieselbe 31-Faelle-Tabelle'
    behauptete. Drei handgepflegte Kopien driften; eine gelesene Datei kann es
    nicht.
    """
    if not FALLTABELLE.exists():
        lauf.wahr("Textriegel-Falltabelle vorhanden", False, str(FALLTABELLE))
        return
    tabelle = json_laden_strikt(FALLTABELLE.read_text(encoding="utf-8"))
    rot: list[str] = []
    for fall in tabelle["faelle"]:
        roh = bytes.fromhex(fall["text_hex"])
        abgelehnt = textriegel_bytes(roh) is not None
        if abgelehnt != fall["wird_abgelehnt"]:
            rot.append(f"#{fall['nr']} {fall['zeigetext'][:40]}")
    lauf.wahr(f"Textriegel deckt jede gemessene Kante ({tabelle['anzahl']} Faelle)",
              not rot, "; ".join(rot))
    # T2-Runde 3, Befund 3: hier stand `tabelle["anzahl"]` - das METADATENFELD.
    # Wer die Fallliste kuerzt und die Zahl stehen laesst, bliebe in genau
    # diesem Bein gruen, waehrend C++ und Rust die echte Liste zaehlen.
    lauf.wahr("Falltabelle hat Substanz (>= 50 Faelle)",
              len(tabelle["faelle"]) >= 50, str(len(tabelle["faelle"])))
    lauf.wahr("Falltabelle: `anzahl` stimmt mit der Liste ueberein",
              tabelle["anzahl"] == len(tabelle["faelle"]),
              f'{tabelle["anzahl"]} vs. {len(tabelle["faelle"])}')
    parser_durchlass: list[str] = []
    for literal in ("NaN", "Infinity", "-Infinity"):
        try:
            json_laden_strikt(f'{{"w": {literal}}}')
            parser_durchlass.append(literal)
        except ValueError:
            pass
    lauf.wahr("json.loads lehnt nicht-endliche Python-Erweiterungen unabhaengig ab",
              not parser_durchlass, ", ".join(parser_durchlass))


def werttyp_passt(name: str, wert) -> bool:
    """Welchen Werttyp verlangt ein Schluesselwort?

    Bis T2-Runde 1 sah die Ladepruefung nur NAMEN. Gemessen: `"maxLength": 5.0`
    wurde vom Rust-Bein still verworfen und vom C++-Bein durchgesetzt —
    dieselbe Fehlerklasse wie ein unbekanntes Schluesselwort, nur eine Ebene
    tiefer. Diese Tabelle muss mit `NakamaVertrag.cpp` und `vertrag.rs`
    zeichengenau uebereinstimmen.
    """
    def ganzzahl(x) -> bool:
        return isinstance(x, int) and not isinstance(x, bool) and x >= 0

    if name == "type":
        return isinstance(wert, str) or (
            isinstance(wert, list) and bool(wert) and all(isinstance(e, str) for e in wert))
    if name == "const":
        return True
    if name in ("enum", "oneOf"):
        return isinstance(wert, list) and bool(wert)
    if name == "required":
        return isinstance(wert, list) and all(isinstance(e, str) for e in wert)
    if name in ("properties", "$defs", "items"):
        return isinstance(wert, dict)
    if name == "additionalProperties":
        return isinstance(wert, bool)
    if name in ("maxProperties", "minLength", "maxLength", "minItems", "maxItems"):
        return ganzzahl(wert)
    if name in ("minimum", "maximum", "exclusiveMinimum", "exclusiveMaximum"):
        return isinstance(wert, (int, float)) and not isinstance(wert, bool)
    if name in ("pattern", "$ref", "x-nakama-discriminator"):
        return isinstance(wert, str)
    return True


def pruefe_schema(lauf: Lauf, schema: dict) -> None:
    pruefer = jsonschema.Draft202012Validator
    try:
        pruefer.check_schema(schema)
        lauf.wahr("Schema ist gueltiges draft 2020-12", True)
    except jsonschema.SchemaError as e:
        lauf.wahr("Schema ist gueltiges draft 2020-12", False, str(e).splitlines()[0])

    fremde: list[str] = []
    muster: list[str] = []
    falsche_typen: list[str] = []
    haengend: list[str] = []
    defs = schema.get("$defs", {})
    for pfad, teil in teilschemata(schema, "#"):
        for name, wert in teil.items():
            if name in ANMERKUNGEN:
                continue
            if name not in SCHLUESSELWOERTER:
                fremde.append(f"{pfad}/{name}")
                continue
            if not werttyp_passt(name, wert):
                falsche_typen.append(f"{pfad}/{name}")
        if "pattern" in teil and teil["pattern"] not in MUSTER:
            muster.append(f"{pfad}: {teil['pattern']!r}")
        if "oneOf" in teil and "x-nakama-discriminator" not in teil:
            lauf.wahr(f"oneOf ohne Discriminator bei {pfad}", False)
        if teil.get("additionalProperties") is True:
            if "maxProperties" not in teil:
                lauf.wahr(f"additives Objekt {pfad} ohne maxProperties", False)
            else:
                # T2-Runde 1: der README nannte die Regel "deklarierte
                # Eigenschaften plus acht", das Schema hielt sie an einer von
                # drei Stellen nicht ein (konfidenz: 6 deklariert, aber 10).
                # Eine Regel, die nur im Fliesstext steht, wird irgendwann
                # falsch - also steht sie jetzt hier und faellt, wenn jemand
                # ein additives Objekt mit anderer Luft anlegt.
                soll = len(teil.get("properties") or {}) + 8
                lauf.wahr(f"maxProperties bei {pfad} ist deklarierte + 8",
                          teil["maxProperties"] == soll,
                          f"{teil['maxProperties']} statt {soll}")
        if "$ref" in teil and isinstance(teil["$ref"], str):
            if not teil["$ref"].startswith("#/$defs/"):
                lauf.wahr(f"nicht-lokale Referenz bei {pfad}", False, teil["$ref"])
            elif teil["$ref"].removeprefix("#/$defs/") not in defs:
                haengend.append(f"{pfad}: {teil['$ref']}")

    lauf.wahr("nur implementierte Schluesselwoerter", not fremde, ", ".join(fremde))
    lauf.wahr("nur Muster aus der Tabelle", not muster, ", ".join(muster))
    lauf.wahr("jedes Schluesselwort traegt den richtigen Werttyp",
              not falsche_typen, ", ".join(falsche_typen))
    lauf.wahr("keine haengende Referenz", not haengend, ", ".join(haengend))


# Die Kopplung aus Entwurf §32.2: jede Messposition traegt GENAU EINE
# Aussageklasse. Handgeschrieben, nicht aus dem Schema abgeleitet — eine aus dem
# Schema gerechnete Erwartung koennte nur bestaetigen, was dort steht.
KOPPLUNG = {
    "probe_descriptor_insert": ("insert", "beobachtend"),
    "probe_descriptor_pre": ("pre", "beobachtend"),
    "probe_descriptor_post": ("post", "beobachtend"),
    "probe_descriptor_beitrag": ("post_fader_contribution", "beitrag"),
}

PLUGIN_KIND_MATRIX = {
    "probe_descriptor_insert": {
        "type": "string",
        "enum": ["main", "passive_probe", "active_probe", "legacy"],
    },
    "probe_descriptor_pre": {
        "type": "string",
        "enum": ["passive_probe", "active_probe", "legacy"],
    },
    "probe_descriptor_post": {
        "type": "string",
        "enum": ["passive_probe", "active_probe", "legacy"],
    },
    "probe_descriptor_beitrag": {
        "type": "string",
        "enum": ["main", "passive_probe", "active_probe", "legacy"],
        "maxLength": 0,
    },
}


def pruefe_discriminator_enginekante(lauf: Lauf) -> None:
    """Direkte Referenzproben fuer Boolean und RFC-6901-Discriminator."""
    boolean_schema = {
        "type": "object",
        "required": ["flag"],
        "additionalProperties": False,
        "properties": {
            "flag": {"type": "boolean"},
            "wahr": {"type": "integer"},
            "falsch": {"type": "integer"},
        },
        "x-nakama-discriminator": "flag",
        "oneOf": [
            {"required": ["wahr"], "properties": {"flag": {"const": True}}},
            {"required": ["falsch"], "properties": {"flag": {"const": False}}},
        ],
    }
    b = jsonschema.Draft202012Validator(boolean_schema)
    lauf.wahr("discriminator_boolean_true_false",
              b.is_valid({"flag": True, "wahr": 1})
              and b.is_valid({"flag": False, "falsch": 1})
              and not b.is_valid({"flag": True, "falsch": 1}))
    lauf.wahr("discriminator_boolean_falscher_typ",
              not b.is_valid({"flag": "true"}))
    lauf.wahr("discriminator_boolean_fehlt", not b.is_valid({}))

    pointer_schema = {
        "type": "object",
        "required": ["validity"],
        "additionalProperties": False,
        "properties": {
            "validity": {
                "type": "object",
                "required": ["active"],
                "additionalProperties": False,
                "properties": {"active": {"type": "boolean"}},
            },
        },
        "x-nakama-discriminator": "/validity/active",
        "oneOf": [
            {"properties": {"validity": {"properties": {
                "active": {"const": True}}}}},
            {"properties": {"validity": {"properties": {
                "active": {"const": False}}}}},
        ],
    }
    p = jsonschema.Draft202012Validator(pointer_schema)
    lauf.wahr("discriminator_json_pointer_boolean",
              p.is_valid({"validity": {"active": True}})
              and p.is_valid({"validity": {"active": False}}))
    lauf.wahr("discriminator_json_pointer_segment_fehlt",
              not p.is_valid({"validity": {}}))


def pruefe_probe_descriptor(lauf: Lauf, schema: dict, reserviert: dict) -> None:
    """§32.2-Kopplung: Messposition bestimmt die Aussageklasse (G1-Befund §4.1).

    Bis zum 24.08.2026 standen `measurement_position` und `aussageklasse`
    unabhaengig nebeneinander — `insert` + `beitrag` validierte, also eine
    gewoehnliche Insertmessung, die sich exakter Mastersummenbeitrag nennt
    (Gate 7 aus §49.2). Die Kopplung steht jetzt als diskriminierte Union.

    Der Preis dieser Form sind vier fast gleiche Zweige. Dieser Riegel macht
    daraus eine GEMESSENE Invariante: er verlangt, dass sich die vier Zweige in
    NICHTS unterscheiden ausser den beiden `const` und `capabilities`. Ohne ihn
    waere jeder Zweig eine eigene Stelle, an der eine spaetere Feldaenderung
    haengenbleiben kann.
    """
    defs = schema["$defs"]

    fehlend = sorted(n for n in KOPPLUNG if n not in defs)
    lauf.wahr("alle vier probe_descriptor-Zweige sind definiert", not fehlend, ", ".join(fehlend))
    if fehlend:
        return

    wurzel = defs["probe_descriptor"]
    lauf.wahr("probe_descriptor diskriminiert ueber measurement_position",
              wurzel.get("x-nakama-discriminator") == "measurement_position",
              repr(wurzel.get("x-nakama-discriminator")))
    lauf.wahr("probe_descriptor traegt NUR das oneOf",
              set(wurzel) == {"description", "x-nakama-discriminator", "oneOf"},
              # Ein Geschwister neben oneOf waere still wirkungslos: beide
              # eigenen Engines steigen in den gewaehlten Zweig ab und kehren
              # zurueck (NakamaVertrag.cpp `pruefeWert`), waehrend jsonschema
              # es anwenden wuerde. Das Urteil liefe auseinander.
              f"{sorted(set(wurzel))}")
    lauf.wahr("die oneOf-Zweige sind genau die vier gekoppelten",
              [r.get("$ref") for r in wurzel.get("oneOf", [])]
              == [f"#/$defs/{n}" for n in KOPPLUNG],
              f"{[r.get('$ref') for r in wurzel.get('oneOf', [])]}")

    for name, (position, klasse) in KOPPLUNG.items():
        zweig = defs[name]
        props = zweig.get("properties", {})
        lauf.wahr(f"{name} pinnt measurement_position auf {position}",
                  props.get("measurement_position") == {"const": position},
                  f"{props.get('measurement_position')}")
        lauf.wahr(f"{name} pinnt aussageklasse auf {klasse}",
                  props.get("aussageklasse") == {"const": klasse},
                  f"{props.get('aussageklasse')}")
        lauf.wahr(f"{name} verlangt beide Felder",
                  {"measurement_position", "aussageklasse", "betrieb"}
                  <= set(zweig.get("required", [])))
        lauf.wahr(f"{name} ist strikt", zweig.get("additionalProperties") is False)
        lauf.wahr(f"{name} fuehrt genau die zwei optionalen Hostfelder",
                  {k for k in props if k.startswith("host_")} == HOST_DESCRIPTOR_FELDER
                  and not (HOST_DESCRIPTOR_FELDER & set(zweig.get("required", []))))
        lauf.wahr(f"{name} referenziert die gemeinsamen Hostfelddefinitionen",
                  props.get("host_bus_name") == {"$ref": "#/$defs/host_bus_name"}
                  and props.get("host_mixer_index") == {"$ref": "#/$defs/host_mixer_index"})
        lauf.wahr(f"{name} referenziert den gemeinsamen Betriebszustand",
                  props.get("betrieb") == {"$ref": "#/$defs/betrieb"})

    busname = defs.get("host_bus_name", {})
    mixerindex = defs.get("host_mixer_index", {})
    lauf.wahr("host_bus_name hat exakt Typ, Codepointgrenzen und H06-Muster",
              busname.get("type") == "string"
              and busname.get("minLength") == 1
              and busname.get("maxLength") == 120
              and busname.get("pattern") == HOST_BUS_NAME_PATTERN)
    lauf.wahr("host_mixer_index hat exakt den JSON-sicheren VST3-Bereich",
              mixerindex.get("type") == "integer"
              and mixerindex.get("minimum") == 1
              and mixerindex.get("maximum") == 9_007_199_254_740_991)
    index_pruefer = jsonschema.Draft202012Validator(mixerindex)
    lauf.wahr("H02-Indexgrenzen 1/Maximum gueltig, 0/negativ/Maximum+1 ungueltig",
              index_pruefer.is_valid(1)
              and index_pruefer.is_valid(9_007_199_254_740_991)
              and not index_pruefer.is_valid(0)
              and not index_pruefer.is_valid(-1)
              and not index_pruefer.is_valid(9_007_199_254_740_992))

    # E-H02: Die Version reist im Envelope, nicht als drittes Descriptorfeld.
    version = reserviert.get("wire_envelope_schema_minor", {})
    fassungen = version.get("fassungen", {})
    lauf.wahr("host_channel_context_fields_are_optional_strict_and_versioned",
              version.get("familie") == "P1"
              and version.get("vorher") == 1
              and version.get("aktuell") == 2
              and fassungen.get("0", {}).get("probe_descriptor_hostfelder") == []
              and set(fassungen.get("1", {}).get("probe_descriptor_hostfelder", []))
                  == HOST_DESCRIPTOR_FELDER
              and fassungen.get("1", {}).get("heartbeat_runtime") is True
              and fassungen.get("1", {}).get("heartbeat_runtime_label") is True
              and fassungen.get("1", {}).get("session_mitglied_probe_descriptor_optional") is True
              and fassungen.get("1", {}).get("session_mitglied_p2_reject") is True
              and fassungen.get("1", {}).get("session_command") is True
              and fassungen.get("1", {}).get("session_snapshot_store_degraded") is True
              and "schema_minor" in version.get("auswahlregel", "")
              and "Wire-Envelope" in version.get("auswahlregel", ""))
    lauf.wahr("probe_descriptor erfindet kein Versions- oder Namespacefeld",
              all("schema_minor" not in defs[n].get("properties", {})
                  and "host_mixer_namespace" not in defs[n].get("properties", {})
                  for n in KOPPLUNG))

    # Referenzbeweis der Auswahlregel: Minor 0 liest die historische direkte
    # Descriptorliste, Minor 1 die strikte Mitgliedshuelle. Nur die jeweils
    # zum Envelope-Minor gehoerende Schemafassung darf den Payload annehmen.
    basis_minor_1 = json_laden_strikt((FIXTURES / "gueltig/session_snapshot.json")
                                      .read_text(encoding="utf-8"))
    mit_host = copy.deepcopy(basis_minor_1)
    mit_host["mitglieder"][0]["probe_descriptor"]["host_bus_name"] = "Bus A"
    mit_host["mitglieder"][0]["probe_descriptor"]["host_mixer_index"] = 1
    basis_minor_0 = copy.deepcopy(basis_minor_1)
    basis_minor_0["mitglieder"] = [m["probe_descriptor"] for m in basis_minor_0["mitglieder"]]
    for descriptor in basis_minor_0["mitglieder"]:
        descriptor.pop("betrieb", None)
    schema_minor_0 = copy.deepcopy(schema)
    for name in KOPPLUNG:
        schema_minor_0["$defs"][name]["required"].remove("betrieb")
        schema_minor_0["$defs"][name]["properties"].pop("betrieb", None)
        for feld in HOST_DESCRIPTOR_FELDER:
            schema_minor_0["$defs"][name]["properties"].pop(feld, None)
    schema_minor_0["$defs"]["heartbeat"]["properties"].pop("runtime", None)
    schema_minor_0["oneOf"] = [r for r in schema_minor_0["oneOf"]
                                if r.get("$ref") != "#/$defs/session_command"]
    schema_minor_0["$defs"].pop("session_command", None)
    schema_minor_0["$defs"]["session_snapshot"]["properties"].pop(
        "store_degraded", None)
    schema_minor_0["$defs"]["session_snapshot"]["properties"]["mitglieder"]["items"] = {
        "$ref": "#/$defs/probe_descriptor"
    }
    pruefer_0 = jsonschema.Draft202012Validator(schema_minor_0)
    pruefer_1 = jsonschema.Draft202012Validator(schema)
    lauf.wahr("Empfaenger waehlt die Session-Schemafassung nach Envelope-Minor",
              pruefer_0.is_valid(basis_minor_0)
              and not pruefer_1.is_valid(basis_minor_0)
              and not pruefer_0.is_valid(basis_minor_1)
              and pruefer_1.is_valid(basis_minor_1)
              and pruefer_1.is_valid(mit_host))

    h06 = jsonschema.Draft202012Validator(busname)
    lauf.wahr("H06 akzeptiert 1/120 Codepoints ohne Normalisierung",
              h06.is_valid("K") and h06.is_valid("😀" * 120)
              and h06.is_valid("  MiXeD Bus  "))
    lauf.wahr("H06 lehnt leer/Whitespace/121/C0/C1 ab",
              all(not h06.is_valid(wert) for wert in
                  ("", " \t", "x" * 121, "A\u001fB", "A\u0085B")))

    matrix_ok = all(defs[n].get("properties", {}).get("plugin_kind") == erwartet
                    for n, erwartet in PLUGIN_KIND_MATRIX.items())
    lauf.wahr("probe_descriptor_plugin_kind_matrix_ist_exakt", matrix_ok)
    lauf.wahr("probe_descriptor_beitragszweig_bleibt_vorhanden",
              "probe_descriptor_beitrag" in defs
              and "#/$defs/probe_descriptor_beitrag"
                  in {r.get("$ref") for r in wurzel.get("oneOf", [])})
    beitrags_kind = defs["probe_descriptor_beitrag"]["properties"]["plugin_kind"]
    lauf.wahr("beitragszweig_hat_keine_heutige_traegerklasse",
              beitrags_kind == PLUGIN_KIND_MATRIX["probe_descriptor_beitrag"]
              and all(len(wert) > beitrags_kind["maxLength"]
                      for wert in beitrags_kind["enum"]))

    beitrag_capabilities_ref = (defs["probe_descriptor_beitrag"]
                                .get("properties", {}).get("capabilities"))
    lauf.wahr("probe_descriptor_beitrag nutzt den eigenen Faehigkeitssatz",
              beitrag_capabilities_ref == {"$ref": "#/$defs/capabilities_beitrag"},
              repr(beitrag_capabilities_ref))
    beitrag_contribution_aux = (defs.get("capabilities_beitrag", {})
                                .get("properties", {}).get("contribution_aux"))
    lauf.wahr("beitragszweig_verlangt_contribution_aux_supported",
              beitrag_contribution_aux == {"const": "supported"},
              repr(beitrag_contribution_aux))

    # Der eigentliche Riegel: die vier Zweige duerfen sich NUR in den zwei
    # const und capabilities unterscheiden. `description` ist Anmerkung und
    # darf abweichen.
    def rumpf(name: str) -> dict:
        z = {k: v for k, v in defs[name].items() if k != "description"}
        z["properties"] = {k: v for k, v in z["properties"].items()
                           if k not in ("measurement_position", "aussageklasse",
                                        "capabilities", "plugin_kind")}
        return z

    erster = rumpf(next(iter(KOPPLUNG)))
    abweichend = [n for n in KOPPLUNG if rumpf(n) != erster]
    lauf.wahr("die vier Zweige unterscheiden sich NUR in const, capabilities und plugin_kind",
              not abweichend, ", ".join(abweichend))

    # Und die Gegenprobe zum Riegel selbst: er muss ueberhaupt etwas finden
    # koennen. Ein Riegel, der nichts FINDET, sagt nichts, bis gezeigt ist,
    # dass er etwas finden koennte (Lehre A14/SONDE-007a).
    verdorben = {n: rumpf(n) for n in KOPPLUNG}
    verdorben["probe_descriptor_pre"]["required"] = ["adresse"]
    lauf.wahr("Gegenprobe: ein verdorbener Zweig faellt am selben Vergleich",
              any(v != erster for v in verdorben.values()))

    matrix_drift = copy.deepcopy(PLUGIN_KIND_MATRIX)
    matrix_drift["probe_descriptor_pre"]["enum"].append("main")
    lauf.wahr("plugin_kind_matrix_drift_faellt", matrix_drift != PLUGIN_KIND_MATRIX)
    beitragsriegel_gelockert = copy.deepcopy(beitrags_kind)
    beitragsriegel_gelockert.pop("maxLength", None)
    lauf.wahr("beitragsriegel_gelockert_faellt",
              beitragsriegel_gelockert != PLUGIN_KIND_MATRIX["probe_descriptor_beitrag"])


def fassung_1_schema(schema: dict) -> dict:
    """Baut die Fassung 1 des P1-Vertrags aus der committeten Fassung 2 zurueck.

    Warum zurueckbauen statt eine zweite Datei zu pflegen: zwei Kopien
    driften. Der Rueckbau ist genau die Liste aus
    `wire_envelope_schema_minor.fassungen."2"` - stimmt sie nicht mit dem
    Schema ueberein, faellt schon der Rueckbau und nicht erst ein Fixture.
    """
    alt = copy.deepcopy(schema)
    neue_familien = {"experiment_begin", "experiment_abort", "experiment_manual_result"}
    alt["oneOf"] = [r for r in alt["oneOf"]
                    if r.get("$ref", "").removeprefix("#/$defs/") not in neue_familien]
    for name in (neue_familien | {"experiment_referenz", "alignment_klasse", "fingerprint",
                                  "evidence_ereignisse", "dynamics_ereignis",
                                  "stereo_evidenz", "stereo_bandwerte",
                                  "stereo_bandwerte_normiert", "stereo_bandwerte_phase"}):
        alt["$defs"].pop(name, None)
    for feld in ("ereignisse", "stereo"):
        alt["$defs"]["evidence_snapshot"]["properties"].pop(feld, None)
    grund = alt["$defs"]["evidence_invalidate"]["properties"]["grund"]
    grund["enum"] = [g for g in grund["enum"]
                     if g not in ("material_wechsel", "messpunkt_wechsel")]
    return alt


def pruefe_sonde013_fassung_2(lauf: Lauf, schema: dict, reserviert: dict) -> None:
    """SONDE-013 M-66/M-67 und §7.1 E-02/E-04.

    Der Riegel dieses Tickets: EIN Fassungsschritt traegt alle
    P1-Vertragsaenderungen, und ein Leser der Fassung 1 LEHNT jede von ihnen
    AB, statt sie still auf einen bekannten Zweig abzubilden. Ohne diesen
    Nachweis waere `nicht additiv erweiterbar` eine Behauptung ohne Messung.
    """
    fassung = reserviert.get("wire_envelope_schema_minor", {}).get("fassungen", {}).get("2", {})
    lauf.wahr("fassung_2_nennt_jede_neuerung_dieses_tickets",
              fassung.get("experiment_begin") is True
              and fassung.get("experiment_abort") is True
              and fassung.get("experiment_manual_result") is True
              and fassung.get("evidence_snapshot_ereignisse") is True
              and fassung.get("evidence_snapshot_stereo") is True
              and fassung.get("evidence_invalidate_grund_erweitert")
                  == ["material_wechsel", "messpunkt_wechsel"])

    gruende = schema["$defs"]["evidence_invalidate"]["properties"]["grund"]["enum"]
    lauf.wahr("grund_material_wechsel", "material_wechsel" in gruende)
    lauf.wahr("grund_messpunkt_wechsel", "messpunkt_wechsel" in gruende)
    lauf.wahr("evidence_invalidate_hat_keinen_steuerkopf",
              set(schema["$defs"]["evidence_invalidate"]["required"])
                  == {"type", "grund", "umfang"}
              and set(schema["$defs"]["evidence_invalidate"]["properties"])
                  == {"type", "grund", "umfang"})

    pruefer_2 = jsonschema.Draft202012Validator(schema)
    pruefer_1 = jsonschema.Draft202012Validator(fassung_1_schema(schema))

    def lade(name: str) -> dict:
        return json_laden_strikt((FIXTURES / f"gueltig/{name}.json").read_text(encoding="utf-8"))

    invalid_material = lade("evidence_invalidate")
    invalid_material["grund"] = "material_wechsel"
    invalid_messpunkt = lade("evidence_invalidate")
    invalid_messpunkt["grund"] = "messpunkt_wechsel"
    invalid_unbekannt = lade("evidence_invalidate")
    invalid_unbekannt["grund"] = "gibt_es_nicht"

    lauf.wahr("unbekannter_grund_wird_abgelehnt",
              not pruefer_2.is_valid(invalid_unbekannt)
              and not pruefer_1.is_valid(invalid_unbekannt))
    lauf.wahr("fassung_1_leser_lehnt_neue_gruende_ab",
              pruefer_2.is_valid(invalid_material)
              and pruefer_2.is_valid(invalid_messpunkt)
              and not pruefer_1.is_valid(invalid_material)
              and not pruefer_1.is_valid(invalid_messpunkt))

    neu = {name: lade(name) for name in
           ("experiment_begin", "experiment_abort", "experiment_manual_result")}
    lauf.wahr("fassung_1_leser_lehnt_die_drei_experimentfamilien_ab",
              all(pruefer_2.is_valid(d) for d in neu.values())
              and not any(pruefer_1.is_valid(d) for d in neu.values()))

    mit_ereignissen = lade("evidence-snapshot-mit-ereignissen-und-stereo")
    lauf.wahr("fassung_1_leser_lehnt_ereignisse_und_stereo_ab",
              pruefer_2.is_valid(mit_ereignissen)
              and not pruefer_1.is_valid(mit_ereignissen))

    # Gegenprobe zum Rueckbau selbst: er muss ueberhaupt etwas entfernen. Ein
    # Rueckbau, der nichts aendert, macht jede Zeile darueber wertlos.
    lauf.wahr("Gegenprobe: der Rueckbau auf Fassung 1 aendert das Schema wirklich",
              fassung_1_schema(schema) != schema
              and pruefer_1.is_valid(lade("evidence_snapshot")))


def pruefe_runtime_und_p2_reject(lauf: Lauf, schema: dict) -> None:
    defs = schema["$defs"]
    runtime = defs.get("heartbeat_runtime", {})
    runtime_props = runtime.get("properties", {})
    lauf.wahr("heartbeat.runtime ist optional und referenziert den strikten Block",
              defs["heartbeat"].get("properties", {}).get("runtime")
              == {"$ref": "#/$defs/heartbeat_runtime"}
              and "runtime" not in defs["heartbeat"].get("required", [])
              and runtime.get("additionalProperties") is False)
    lauf.wahr("runtime verlangt exakt Messpunkt und Betrieb",
              set(runtime.get("required", [])) == {"messpunkt", "betrieb"}
              and runtime_props.get("messpunkt", {}).get("enum")
                  == ["insert", "pre", "post"]
              and runtime_props.get("betrieb") == {"$ref": "#/$defs/betrieb"})
    lauf.wahr("runtime nutzt exakt die zwei B1-Hostfelder und kein drittes",
              {k for k in runtime_props if k.startswith("host_")}
                  == HOST_DESCRIPTOR_FELDER
              and runtime_props.get("host_bus_name")
                  == {"$ref": "#/$defs/host_bus_name"}
              and runtime_props.get("host_mixer_index")
                  == {"$ref": "#/$defs/host_mixer_index"})
    lauf.wahr("runtime.label ist optional und exakt probe_label",
              runtime_props.get("label") == {"$ref": "#/$defs/probe_label"}
              and "label" not in runtime.get("required", []))
    lauf.wahr("Betriebszustand ist exakt active/suspended/offline",
              defs.get("betrieb", {}).get("enum")
              == ["active", "suspended", "offline"])

    mitglied = defs.get("session_mitglied", {})
    mitglied_props = mitglied.get("properties", {})
    lauf.wahr("session_snapshot fuehrt strikte Mitgliedshuelle",
              defs["session_snapshot"]["properties"]["mitglieder"]["items"]
                  == {"$ref": "#/$defs/session_mitglied"}
              and mitglied.get("additionalProperties") is False
              and set(mitglied.get("required", []))
                  == {"adresse", "plugin_kind", "frische"})
    lauf.wahr("probe_descriptor ist je Mitglied optional und unclassified bleibt darstellbar",
              mitglied_props.get("probe_descriptor")
                  == {"$ref": "#/$defs/probe_descriptor"}
              and "probe_descriptor" not in mitglied.get("required", []))

    reject = defs.get("p2_reject", {})
    katalog = {
        "feature_batch_ungueltig",
        "quellframe_anzahl_ungueltig",
        "routing_nicht_freigegeben",
        "quelladresse_abweichend",
        "lautheit_ungueltig",
    }
    lauf.wahr("p2_reject ist optional, strikt und je Mitglied",
              mitglied_props.get("p2_reject") == {"$ref": "#/$defs/p2_reject"}
              and "p2_reject" not in mitglied.get("required", [])
              and reject.get("additionalProperties") is False
              and set(reject.get("required", [])) == {"grund", "zaehler"})
    lauf.wahr("p2_reject nutzt geschlossenen Katalog und Zaehler ab 1",
              set(reject.get("properties", {}).get("grund", {}).get("enum", []))
                  == katalog
              and reject.get("properties", {}).get("zaehler")
                  == {"type": "integer", "minimum": 1})
    lauf.wahr("striktes error bleibt ohne quellbezogenen B2b-Kanal",
              "p2_reject" not in defs["error"].get("properties", {})
              and "adresse" not in defs["error"].get("properties", {}))

    fixture_namen = {
        p.name for p in (FIXTURES / "gueltig").glob("*.json")
    } | {p.name for p in (FIXTURES / "ungueltig").glob("*.json")}
    verlangt = {
        "heartbeat-runtime-vollstaendig.json",
        "heartbeat-runtime-ohne-messpunkt.json",
        "heartbeat-runtime-messpunkt-unbekannt.json",
        "heartbeat-runtime-betrieb-unbekannt.json",
        "heartbeat-runtime-hostname-zu-lang.json",
        "host-mixer-index-ueber-json-sicher.json",
        "session-p2-reject.json",
        "session-mitglied-unclassified.json",
        "session-p2-reject-freitext.json",
        "session-p2-reject-zaehler-null.json",
    }
    lauf.wahr("B2b-Cross-Language-Fixtures decken Runtime und p2_reject",
              verlangt <= fixture_namen, ", ".join(sorted(verlangt - fixture_namen)))


def pruefe_session_command_und_store(lauf: Lauf, schema: dict) -> None:
    """SONDE-012 E-L18/L04 und E-L15-Store, ohne neue Wire-Version."""
    defs = schema["$defs"]
    command = defs.get("session_command", {})
    lauf.wahr("session_command diskriminiert geschlossen ueber command",
              command.get("x-nakama-discriminator") == "command"
              and set(command) == {"description", "x-nakama-discriminator", "oneOf"})
    zweige = zweige_nach_const(command, "command")
    lauf.wahr("session_command kennt genau confirm_join und unbind_probe",
              set(zweige) == {"confirm_join", "unbind_probe"}, repr(sorted(zweige)))
    pflicht = {"type", "command", "command_id", "ziel", "session_epoch"}
    for art, zweig in zweige.items():
        props = zweig.get("properties", {})
        lauf.wahr(f"session_command/{art} ist strikt und vollstaendig",
                  zweig.get("additionalProperties") is False
                  and set(zweig.get("required", [])) == pflicht
                  and set(props) == pflicht)
        lauf.wahr(f"session_command/{art} nutzt Zieladresse und Hexbezug",
                  props.get("type") == {"const": "session_command"}
                  and props.get("command_id") == {"$ref": "#/$defs/hex32"}
                  and props.get("ziel") == {"$ref": "#/$defs/adresse"}
                  and props.get("session_epoch") == {"$ref": "#/$defs/hex32"}
                  and "sender" not in props and "schema_minor" not in props)

    store = defs.get("session_snapshot", {}).get("properties", {}).get(
        "store_degraded", {})
    lauf.wahr("store_degraded ist optional und nur true darf reisen",
              store == {"$comment": store.get("$comment"),
                        "type": "boolean", "const": True}
              and "store_degraded" not in defs["session_snapshot"].get("required", []))
    store_pruefer = jsonschema.Draft202012Validator(schema)
    basis = json_laden_strikt((FIXTURES / "gueltig/session_snapshot.json")
                              .read_text(encoding="utf-8"))
    mit_true = copy.deepcopy(basis)
    mit_true["store_degraded"] = True
    mit_false = copy.deepcopy(basis)
    mit_false["store_degraded"] = False
    lauf.wahr("store_degraded true/abwesend gueltig, false ungueltig",
              store_pruefer.is_valid(basis) and store_pruefer.is_valid(mit_true)
              and not store_pruefer.is_valid(mit_false))

    fixture_namen = {
        p.name for p in (FIXTURES / "gueltig").glob("*.json")
    } | {p.name for p in (FIXTURES / "ungueltig").glob("*.json")}
    verlangt = {
        "session_command.json",
        "session-command-unbind.json",
        "session-command-fremdes-main-vertragsform.json",
        "session-command-fremdes-main-senderfeld.json",
        "session-command-falsche-epoche-vertragsform.json",
        "session-command-unbekannter-zweig.json",
        "session-snapshot-store-degraded.json",
        "session-snapshot-store-degraded-false.json",
    }
    lauf.wahr("B3c-Cross-Language-Fixtures decken Commands und Storezustand",
              verlangt <= fixture_namen, ", ".join(sorted(verlangt - fixture_namen)))


def zweige_nach_const(knoten: dict, discriminator: str) -> dict[str, dict]:
    """Ordnet vollstaendige oneOf-Zweige ihrem Discriminator-const zu."""
    ergebnis: dict[str, dict] = {}
    for zweig in knoten.get("oneOf", []):
        wert = zweig.get("properties", {}).get(discriminator, {}).get("const")
        if isinstance(wert, str) and wert not in ergebnis:
            ergebnis[wert] = zweig
    return ergebnis


def pruefe_bandkodierung(lauf: Lauf, schema: dict, quantisierung: dict) -> None:
    """28.08.2026: encoding bestimmt Typ und plausible Traegergrenzen."""
    defs = schema["$defs"]
    grenzen = quantisierung["plausibler_bereich_db"]["traegergrenzen"]
    erwartet = {
        "q_db_0p1_i16": grenzen["q_db_0p1_i16"],
        "q_db_0p01_i16": grenzen["q_db_0p01_i16"],
        "float32": None,
    }

    for name, (gitter, anzahl) in {
        "bandwerte_fein": ("nakama_1_24_oct_30_18k_v1", 221),
        "bandwerte_grob": ("nakama_log64_v1", 64),
    }.items():
        wurzel = defs[name]
        lauf.wahr(f"{name} diskriminiert ueber encoding",
                  wurzel.get("x-nakama-discriminator") == "encoding")
        lauf.wahr(f"{name} traegt NUR das oneOf",
                  set(wurzel) == {"description", "x-nakama-discriminator", "oneOf"},
                  f"{sorted(set(wurzel))}")
        zweige = zweige_nach_const(wurzel, "encoding")
        lauf.wahr(f"{name} kennt genau die drei Kodierungen",
                  set(zweige) == set(erwartet), f"{sorted(zweige)}")

        # Die vollstaendigen Zweige sind wegen der bewusst kleinen Engine-
        # Teilmenge noetig. Alles ausser encoding und dessen Wertevertrag muss
        # trotzdem identisch bleiben; sonst reparierte ein spaeterer Autor nur
        # eine von sechs Kopien.
        def bandrumpf(zweig: dict) -> dict:
            rumpf = json_laden_strikt(json.dumps(zweig))
            rumpf.pop("description", None)
            rumpf["properties"].pop("encoding", None)
            rumpf["properties"]["werte"].pop("items", None)
            return rumpf

        if zweige:
            erster = bandrumpf(next(iter(zweige.values())))
            abweichend = [e for e, z in zweige.items() if bandrumpf(z) != erster]
            lauf.wahr(f"{name}-Zweige unterscheiden sich nur im Wertevertrag",
                      not abweichend, ", ".join(abweichend))

        for encoding, soll_grenzen in erwartet.items():
            zweig = zweige.get(encoding)
            if zweig is None:
                continue
            props = zweig.get("properties", {})
            werte = props.get("werte", {})
            items = werte.get("items", {})
            lauf.wahr(f"{name}/{encoding} pinnt das Gitter",
                      props.get("gitter_id") == {"const": gitter})
            lauf.wahr(f"{name}/{encoding} verlangt {anzahl} Werte",
                      werte.get("minItems") == anzahl and werte.get("maxItems") == anzahl)
            lauf.wahr(f"{name}/{encoding} ist strikt",
                      zweig.get("additionalProperties") is False)
            if soll_grenzen is None:
                lauf.wahr(f"{name}/{encoding} traegt Zahlen",
                          items == {"type": "number"}, repr(items))
            else:
                lauf.wahr(f"{name}/{encoding} folgt quantisierung-v1.json",
                          items == {"type": "integer", "minimum": soll_grenzen[0],
                                    "maximum": soll_grenzen[1]}, repr(items))


def pruefe_command_ack(lauf: Lauf, schema: dict) -> None:
    """28.08.2026: Erfolg bestaetigt immer einen konkreten Stand."""
    defs = schema["$defs"]
    wurzel = defs["command_ack"]
    lauf.wahr("command_ack diskriminiert ueber ergebnis",
              wurzel.get("x-nakama-discriminator") == "ergebnis")
    lauf.wahr("command_ack traegt NUR das oneOf",
              set(wurzel) == {"description", "x-nakama-discriminator", "oneOf"},
              f"{sorted(set(wurzel))}")

    zweige = zweige_nach_const(wurzel, "ergebnis")
    erfolg = {"angewandt", "idempotent_wiederholt"}
    alle = erfolg | {"abgelehnt", "konflikt", "abgelaufen"}
    lauf.wahr("command_ack kennt genau die fuenf Ergebnisse",
              set(zweige) == alle, f"{sorted(zweige)}")
    for ergebnis, zweig in zweige.items():
        pflicht = set(zweig.get("required", []))
        hash_ref = zweig.get("properties", {}).get("state_hash", {}).get("$ref")
        if ergebnis in erfolg:
            lauf.wahr(f"command_ack/{ergebnis} verlangt state_hash",
                      "state_hash" in pflicht)
            lauf.wahr(f"command_ack/{ergebnis} verlangt nicht-null state_hash",
                      hash_ref == "#/$defs/state_hash_erfolg", repr(hash_ref))
        else:
            lauf.wahr(f"command_ack/{ergebnis} laesst state_hash optional",
                      "state_hash" not in pflicht)
            lauf.wahr(f"command_ack/{ergebnis} darf nullable state_hash tragen",
                      hash_ref == "#/$defs/state_hash", repr(hash_ref))

    def ackrumpf(zweig: dict) -> dict:
        rumpf = json_laden_strikt(json.dumps(zweig))
        rumpf["required"] = sorted(f for f in rumpf.get("required", [])
                                   if f != "state_hash")
        rumpf["properties"].pop("ergebnis", None)
        rumpf["properties"].pop("state_hash", None)
        return rumpf

    if zweige:
        erster = ackrumpf(next(iter(zweige.values())))
        abweichend = [e for e, z in zweige.items() if ackrumpf(z) != erster]
        lauf.wahr("command_ack-Zweige unterscheiden sich nur in Ergebnis und Hashpflicht",
                  not abweichend, ", ".join(abweichend))

    normal = defs["state_hash"]
    erfolgs_hash = defs["state_hash_erfolg"]
    lauf.wahr("Erfolgs- und nullable state_hash teilen Laenge und Alphabet",
              all(normal.get(k) == erfolgs_hash.get(k)
                  for k in ("minLength", "maxLength", "pattern")))
    lauf.wahr("state_hash_erfolg schliesst null aus",
              erfolgs_hash.get("type") == "string")


def pruefe_namen(lauf: Lauf, schema: dict, reserviert: dict) -> None:
    zweige = [r["$ref"].removeprefix("#/$defs/") for r in schema["oneOf"]]
    definiert = reserviert["definiert"]
    reserv = [r["name"] for r in reserviert["reserviert"]]
    belegt_nachrichten = reserviert.get("belegte_nachrichten", [])

    lauf.wahr("oneOf-Zweige == definierte Liste", zweige == definiert,
              f"{set(zweige) ^ set(definiert)}")
    lauf.wahr("definiert und reserviert sind disjunkt",
              not (set(definiert) & set(reserv)), f"{set(definiert) & set(reserv)}")
    lauf.wahr(f"Summe ist {reserviert['gesamt_erwartet']} registrierte Familien",
              len(definiert) + len(reserv) == reserviert["gesamt_erwartet"],
              f"{len(definiert)} + {len(reserv)}")
    lauf.wahr("jede definierte Familie hat ein $defs",
              all(n in schema["$defs"] for n in definiert))
    lauf.wahr("keine reservierte Familie hat ein $defs",
              not any(n in schema["$defs"] for n in reserv))
    lauf.wahr("jedes Eigentuemerticket ist genannt",
              all(r.get("eigentuemer") and r.get("grund") for r in reserviert["reserviert"]))
    lauf.wahr("belegte Nachricht folgt der Regelform und ist aktiv definiert",
              all(set(n) == {"name", "eigentuemer", "grund"}
                  for n in belegt_nachrichten)
              and {n.get("name") for n in belegt_nachrichten}
                  == {"session_command", "experiment_begin", "experiment_abort",
                      "experiment_manual_result"}
              and all(n.get("name") in definiert and n.get("name") not in reserv
                      for n in belegt_nachrichten))

    # SONDE-013 E-02: die drei Experimentfamilien. `experiment_begin` und
    # `experiment_abort` wandern aus `reserviert` nach `definiert`,
    # `experiment_manual_result` entsteht neu und direkt in `definiert`.
    lauf.wahr("zwei_familien_wandern_von_reserviert_nach_definiert",
              "experiment_begin" in definiert and "experiment_abort" in definiert
              and "experiment_begin" not in reserv and "experiment_abort" not in reserv
              and "experiment_begin" in schema["$defs"]
              and "experiment_abort" in schema["$defs"])
    lauf.wahr("experiment_manual_result_definiert",
              "experiment_manual_result" in definiert
              and "experiment_manual_result" in schema["$defs"]
              and "#/$defs/experiment_manual_result" in {r["$ref"] for r in schema["oneOf"]}
              and next(n["eigentuemer"] for n in belegt_nachrichten
                       if n["name"] == "experiment_manual_result").startswith("SONDE-013"))
    lauf.wahr("summe_ist_28",
              reserviert["gesamt_erwartet"] == 28
              and len(definiert) == 21 and len(reserv) == 7)

    # M-73: kein Ticket belegt einen Namen, dessen Eigentuemer ein anderes
    # Ticket ist - und der Vertrag kennt nur GANZE Familien, keine
    # teilreservierten Discriminator-Zweige.
    eigentuemer = {r["name"]: r.get("eigentuemer", "") for r in reserviert["reserviert"]}
    lauf.wahr("experiment_result_bleibt_reserviert_fuer_sonde017",
              "experiment_result" in reserv
              and eigentuemer.get("experiment_result", "").startswith("SONDE-017")
              and "experiment_result" not in definiert
              and "experiment_result" not in schema["$defs"])
    lauf.wahr("fremde_eigentuemer_bleiben_unberuehrt",
              eigentuemer.get("user_verdict", "").startswith("SONDE-014")
              and "user_verdict" not in definiert
              and "user_verdict" not in schema["$defs"]
              and not any(n.startswith("experiment_result")
                          or n.startswith("user_verdict")
                          for n in schema["$defs"]))
    lauf.wahr("reservierter_name_reference_match_wird_nicht_umgewidmet",
              "reference_match" in reserv and "reference_match" not in zweige
              and "reference_match" not in schema["$defs"])
    lauf.wahr("kein_unsubscribe_session_name",
              "unsubscribe_session" not in reserv and "unsubscribe_session" not in zweige
              and "unsubscribe_session" not in schema["$defs"])

    felder = reserviert.get("reservierte_felder", [])
    belegt = reserviert.get("belegte_felder", [])
    lauf.wahr("reserviertes_feld_hat_keine_nutzlast",
              all(set(f) == {"name", "eigentuemer", "grund"} for f in felder))
    lauf.wahr("belegtes_feld_folgt_der_gleichen_Regelform",
              all(set(f) == {"name", "eigentuemer", "grund"} for f in belegt))
    erwartete_felder = {
        "Frame.band_dynamic_gain_db",
        "session_snapshot.contribution_inputs",
        "state_report.dsp",
        "command_ack.applied_dsp",
        "state_report.eq_enabled",
    }
    lauf.wahr("reservierte Feldnamen sind exakt und kollisionsfrei",
              {f.get("name") for f in felder} == erwartete_felder)
    erwartete_belegte = {
        "probe_descriptor.host_bus_name",
        "probe_descriptor.host_mixer_index",
        "heartbeat.runtime",
        "session_snapshot.mitglieder[].probe_descriptor",
        "session_snapshot.mitglieder[].p2_reject",
        "session_snapshot.store_degraded",
        "evidence_snapshot.ereignisse",
        "evidence_snapshot.stereo",
        "evidence_snapshot.stereo.phase_rad",
        "evidence_snapshot.stereo.fenster_dauer_ms",
        "evidence_snapshot.stereo.freiheitsgrade",
    }
    lauf.wahr("SONDE-012-Minor-1-Felder sind als belegt fortgeschrieben",
              {f.get("name") for f in belegt} == erwartete_belegte
              and not ({f.get("name") for f in felder} & erwartete_belegte))

    # SONDE-013 M-65: das Belegen von `evidence_snapshot.ereignisse` ist KEIN
    # additiver Schritt. Das Objekt ist additionalProperties:false, also muss
    # das Feld im aktiven Vertrag WIRKLICH stehen - und der Feldname darf
    # nicht mehr in der Reserve liegen.
    ereignisse = schema["$defs"]["evidence_snapshot"].get("properties", {}).get("ereignisse")
    stereo = schema["$defs"]["evidence_snapshot"].get("properties", {}).get("stereo")
    lauf.wahr("evidence_snapshot_ereignisse_belegt",
              ereignisse == {"$ref": "#/$defs/evidence_ereignisse"}
              and stereo == {"$ref": "#/$defs/stereo_evidenz"}
              and schema["$defs"]["evidence_snapshot"]["additionalProperties"] is False
              and "ereignisse" not in schema["$defs"]["evidence_snapshot"]["required"]
              and "stereo" not in schema["$defs"]["evidence_snapshot"]["required"])
    ring = schema["$defs"]["evidence_ereignisse"]["properties"]
    lauf.wahr("ereignisring_bleibt_bei_64_und_zaehlt_verluste",
              ring["liste"]["maxItems"] == 64
              and ring["liste"]["minItems"] == 0
              and ring["verloren"] == {
                  "$comment": ring["verloren"]["$comment"],
                  "type": "integer", "minimum": 0})
    ereignis = schema["$defs"]["dynamics_ereignis"]
    lauf.wahr("dynamics_ereignis_traegt_beide_qualitaetsbits",
              set(ereignis["required"]) == {
                  "sample_offset", "staerke_mad", "band_zentrum_hz",
                  "dauer_samples", "qualitaet_fluss", "qualitaet_peak"}
              and ereignis["properties"]["qualitaet_fluss"] == {"type": "boolean"}
              and ereignis["properties"]["qualitaet_peak"] == {"type": "boolean"})

    # M-11: Kohaerenz traegt Fensterdauer und Freiheitsgrade je Band mit,
    # und die Phase liegt in einer EIGENEN Definition mit eigenem Bereich -
    # sie ist keine Umdeutung eines normierten Bandsatzes.
    stereo_defs = schema["$defs"]["stereo_evidenz"]["properties"]
    lauf.wahr("kohaerenz_traegt_fenster_und_freiheitsgrade",
              stereo_defs["fenster_dauer_ms"]["minItems"] == 221
              and stereo_defs["freiheitsgrade"]["minItems"] == 221
              and stereo_defs["freiheitsgrade"]["items"]["type"] == "integer"
              and stereo_defs["kohaerenz"]["$ref"] == "#/$defs/stereo_bandwerte_normiert"
              and stereo_defs["phase_rad"] == {"$ref": "#/$defs/stereo_bandwerte_phase"})

    # M-06: Gewichte und Schwellen leben in einer versionierten
    # metrics_version, nicht als Literal im Produktpfad. Der Vertrag traegt
    # das Feld an JEDEM Traeger einer Metrik dieses Tickets.
    lauf.wahr("metrics_version_bindet_schwellen",
              schema["$defs"]["evidence_snapshot"]["properties"]["metrics_version"]
                  == {"type": "integer", "minimum": 1}
              and "metrics_version" in schema["$defs"]["evidence_snapshot"]["required"])

    def feld_fehlt(definition: str, feld: str) -> bool:
        return feld not in schema["$defs"][definition].get("properties", {})

    aktive_felder_fehlen = (
        feld_fehlt("session_snapshot", "contribution_inputs")
        and feld_fehlt("state_report", "dsp")
        and feld_fehlt("state_report", "eq_enabled")
        and all("applied_dsp" not in z.get("properties", {})
                for z in schema["$defs"]["command_ack"].get("oneOf", []))
    )
    lauf.wahr("reservierte Felder sind im aktiven Vertrag weiter abgelehnt",
              aktive_felder_fehlen)
    lauf.wahr("belegte Hostfelder stehen in jedem probe_descriptor-Zweig",
              all(not feld_fehlt(n, f) for n in KOPPLUNG
                  for f in HOST_DESCRIPTOR_FELDER))


# ------------------------------------------------------------------ Fixturelauf

MINDESTKORPUS = 100


def pruefe_fixtures(lauf: Lauf, schema: dict, manifest: dict) -> None:
    pruefer = jsonschema.Draft202012Validator(schema)

    lauf.wahr("Textriegel lehnt ein Dokument oberhalb 16 MiB vor dem Parser ab",
              textriegel_bytes(b" " * (MAX_DOKUMENT_BYTES + 1)) == "Dokument zu gross")

    # T2-Runde 1: C++ und Rust haben je einen `>= 100`-Riegel, dieses Bein
    # hatte keinen. Mit geleerter Fixtureliste waere ein Lauf OHNE --abdeckung
    # gruen durchgegangen — eine Pruefung, die nicht fehlschlagen kann.
    lauf.wahr(f"Korpus hat Substanz (>= {MINDESTKORPUS} Fixtures)",
              len(manifest["fixtures"]) >= MINDESTKORPUS,
              f"{len(manifest['fixtures'])}")

    for eintrag in manifest["fixtures"]:
        pfad = FIXTURES / eintrag["datei"]
        if not pfad.exists():
            lauf.wahr(f"{eintrag['datei']} vorhanden", False)
            continue
        roh_bytes = pfad.read_bytes()

        # Der Textriegel laeuft VOR dem Parser, und zwar ueber JEDES Fixture.
        # Die markierten muessen an ihm fallen, alle uebrigen ihn passieren.
        # Ohne die zweite Haelfte waere der Riegel eine Behauptung, die nur an
        # wenigen Dateien geprueft wird.
        grund = textriegel_bytes(roh_bytes)
        if eintrag.get("textriegel_lehnt_ab"):
            lauf.wahr(f"Textriegel lehnt ab: {eintrag['datei']}", grund is not None)
            continue
        if grund is not None:
            lauf.fehler.append(
                f"{eintrag['datei']}: Textriegel lehnt ab, soll passieren lassen: {grund}")
            continue

        try:
            daten = json_laden_strikt(roh_bytes.decode("utf-8"))
        except (json.JSONDecodeError, ValueError) as e:
            # Ein nicht lesbares Fixture ist eine benannte Abweichung, kein
            # Abbruch des Laufs - dasselbe Prinzip wie der wurzel_skalar-Zweig
            # der C++-Seite.
            lauf.fehler.append(f"{eintrag['datei']}: nicht lesbar: {e}")
            continue
        gueltig = pruefer.is_valid(daten)
        soll = eintrag["urteil"] == "gueltig"
        if gueltig == soll:
            lauf.ok += 1
        else:
            grund = ""
            if not gueltig:
                erster = sorted(pruefer.iter_errors(daten), key=lambda e: list(e.absolute_path))[0]
                grund = f"jsonschema sagt ungueltig: {erster.message[:120]}"
            else:
                grund = "jsonschema sagt gueltig, Manifest erwartet ungueltig"
            lauf.fehler.append(f"{eintrag['datei']}: {grund}")

    lauf.wahr("Manifestzahlen stimmen",
              manifest["anzahl_gueltig"] == sum(1 for e in manifest["fixtures"] if e["urteil"] == "gueltig")
              and manifest["anzahl_ungueltig"] == sum(1 for e in manifest["fixtures"] if e["urteil"] == "ungueltig"))
    lauf.wahr("jedes Fixture traegt eine Begruendung",
              all(e.get("warum") for e in manifest["fixtures"]))
    lauf.wahr("gueltige Fixtures tragen keine Verletzungen",
              all(not e["verletzungen"] for e in manifest["fixtures"] if e["urteil"] == "gueltig"))
    # Ausgenommen sind die Textriegel-Fixtures: sie erreichen das Schema nie,
    # also gibt es dort nichts zu verletzen. Eine erfundene Verletzungsmenge
    # waere eine Luege ueber den ORT der Ablehnung.
    lauf.wahr("ungueltige Fixtures tragen mindestens eine Verletzung",
              all(e["verletzungen"] for e in manifest["fixtures"]
                  if e["urteil"] == "ungueltig" and not e.get("textriegel_lehnt_ab")))
    lauf.wahr("Textriegel-Fixtures tragen KEINE Verletzungsmenge",
              all(not e["verletzungen"] for e in manifest["fixtures"]
                  if e.get("textriegel_lehnt_ab")))

    # T2-Runde 1: acht deklarierte Eigenschaften kamen in KEINEM Fixture vor.
    # Der Abdeckungsriegel unten sah das nicht - er zaehlt Definitionen mit
    # Negativfixture, nicht Felder, die je in einer Nachricht standen. Ein Feld,
    # das nie gesendet wird, ist an dieser Stelle ungeprueft.
    deklariert: set[str] = set()

    def sammle_deklariert(knoten) -> None:
        if isinstance(knoten, dict):
            deklariert.update((knoten.get("properties") or {}).keys())
            for wert in knoten.values():
                sammle_deklariert(wert)
        elif isinstance(knoten, list):
            for wert in knoten:
                sammle_deklariert(wert)

    sammle_deklariert(schema)

    benutzt: set[str] = set()

    def sammle_benutzt(knoten) -> None:
        if isinstance(knoten, dict):
            for name, wert in knoten.items():
                benutzt.add(name)
                sammle_benutzt(wert)
        elif isinstance(knoten, list):
            for wert in knoten:
                sammle_benutzt(wert)

    for eintrag in manifest["fixtures"]:
        pfad = FIXTURES / eintrag["datei"]
        if not pfad.exists() or eintrag.get("textriegel_lehnt_ab"):
            continue
        try:
            sammle_benutzt(json_laden_strikt(pfad.read_text(encoding="utf-8")))
        except (json.JSONDecodeError, ValueError):
            continue

    unberuehrt = sorted(deklariert - benutzt)
    lauf.wahr(f"jede der {len(deklariert)} deklarierten Eigenschaften steht in "
              "mindestens einem Fixture", not unberuehrt, ", ".join(unberuehrt))


# ------------------------------------------------------------------ Abdeckung

def abdeckung(schema: dict, manifest: dict) -> tuple[list[str], dict[str, tuple[int, int]], list[str]]:
    """-> (unabgedeckte $defs, Schluesselwort -> (geprueft, vorhanden), harte Luecken).

    Zwei Feinheiten, die eine naive Zaehlung falsch macht:

    * `items` ist ein APPLIKATOR, keine Behauptung — es kann gar nicht selbst
      verletzt werden. Abgedeckt heisst hier: eine Verletzung liegt UNTERHALB
      davon.
    * Der `const` einer Discriminator-Eigenschaft ist per Konstruktion
      unerreichbar: passt er nicht, waehlt der Discriminator den Zweig erst gar
      nicht, und die Verletzung heisst `oneOf`. Ein Negativfixture dafuer zu
      verlangen waere eine Forderung nach einer unmoeglichen Datei.
    """
    getroffen_defs: set[str] = set()
    getroffen_pfade: set[str] = set()

    for e in manifest["fixtures"]:
        for verl in e["verletzungen"]:
            getroffen_pfade.add(verl["schema"])
            if verl["schema"].startswith("#/$defs/"):
                getroffen_defs.add(verl["schema"].removeprefix("#/$defs/").split("/")[0])

    offene_defs = sorted(set(schema["$defs"]) - getroffen_defs)

    diskriminatoren = {teil["x-nakama-discriminator"].rsplit("/", 1)[-1]
                       for _, teil in teilschemata(schema, "#")
                       if "x-nakama-discriminator" in teil}

    applikatoren = {"items"}
    zaehlung: dict[str, list[int]] = {}
    harte_luecken: list[str] = []

    def zaehle(name: str, voll: str, getroffen: bool, hart: bool) -> None:
        z = zaehlung.setdefault(name, [0, 0])
        z[1] += 1
        if getroffen:
            z[0] += 1
        elif hart:
            harte_luecken.append(voll)

    for pfad, teil in teilschemata(schema, "#"):
        for name in teil:
            if name not in SCHLUESSELWOERTER or name in ("properties", "$ref", "x-nakama-discriminator"):
                continue
            if name == "required":
                for feld in teil["required"]:
                    zaehle("required", f"{pfad}/required/{feld}",
                           f"{pfad}/required/{feld}" in getroffen_pfade, hart=False)
                continue
            voll = f"{pfad}/{name}"
            if name in applikatoren:
                zaehle(name, voll, any(p.startswith(voll + "/") for p in getroffen_pfade), hart=False)
                continue
            if name == "const" and pfad.rsplit("/", 1)[-1] in diskriminatoren:
                # unerreichbar per Konstruktion — nicht mitzaehlen
                continue
            # Harte Luecke nur dort, wo ein falscher Wert ein echtes Produktrisiko
            # ist: jede Wertedomaene (enum) und jede erreichbare Festlegung (const).
            zaehle(name, voll, voll in getroffen_pfade, hart=name in ("enum", "const"))

    return offene_defs, {k: (v[0], v[1]) for k, v in sorted(zaehlung.items())}, harte_luecken


# ------------------------------------------------------------------ Hauptlauf

def _konstanten_aus_kern(dateien: list[pathlib.Path]) -> dict[str, tuple[str, str]]:
    """Liest jede `inline constexpr`-Zeile der genannten Kernheader.

    Rueckgabe: Name -> (Rohwert als Text, Dateiname). Der Wert bleibt TEXT und
    wird erst beim Vergleich gedeutet: `12.0` und `12` sind in C++ dasselbe,
    in JSON aber nicht, und ein Vergleich ueber `float()` waere gegenueber
    einem vertippten `1.20` blind.
    """
    muster = re.compile(
        r"^inline\s+constexpr\s+(?:std::)?\w+\s+(k\w+)\s*=\s*([^;]+);", re.MULTILINE)
    aus: dict[str, tuple[str, str]] = {}
    for datei in dateien:
        if not datei.exists():
            continue
        for name, wert in muster.findall(datei.read_text(encoding="utf-8")):
            aus[name] = (wert.strip().rstrip("uf"), datei.name)
    return aus


def pruefe_metrikregister(lauf: Lauf) -> None:
    """SONDE-013 M-06: die Schwellen haengen an einer `metrics_version`.

    Entwurf §34.3 woertlich: "Zahlengewichte und Schwellen gehoeren in eine
    versionierte `metrics_version`". Das ist keine Doku-Zusage, sondern eine
    ueber die Kalibrierbarkeit: wer eine Schwelle aendert, ohne die Version
    zu heben, macht aus einer spaeteren Kalibrierung einen stillen Bruch
    statt eines Versionsschritts (SONDE-013 §5.3, Risiko 5).

    Der Riegel greift in beide Richtungen:

    - jede im Register gefuehrte Schwelle steht mit GENAU DIESEM Wert im Code;
    - die im Code benutzte `kFeatureMetricsVersion` ist die `aktuell` des
      Registers.

    Was er NICHT prueft: dass jede Codekonstante im Register steht. Der Kern
    fuehrt Ressourcengrenzen (`kEreignisPlaetze`, `kVerteilungPlaetze`) und
    normfeste Zahlen (die drei LRA-Gates aus EBU Tech 3342), und beide sind
    ausdruecklich KEINE kalibrierbaren Schwellen — sie stehen im Register
    unter `nicht_gefuehrt` mit Begruendung. Eine Vollstaendigkeitspruefung
    zwaenge jede neue Konstante ins Register und machte die Unterscheidung
    wertlos.
    """
    if not METRIKEN.exists():
        lauf.wahr("metrics_version_bindet_schwellen: Register vorhanden", False,
                  f"{METRIKEN} fehlt")
        return
    register = json_laden_strikt(METRIKEN.read_text(encoding="utf-8"))

    kern = WURZEL / "eq-copilot/plugin/core/analysis"
    konstanten = _konstanten_aus_kern([
        kern / "FeatureEngine.h",
        kern / "Konfidenz.h",
    ])
    lauf.wahr("metrics_version_bindet_schwellen: Kernkonstanten lesbar",
              len(konstanten) > 5, f"{len(konstanten)} gefunden")

    aktuell = str(register.get("aktuell", ""))
    fassungen = register.get("fassungen", {})
    lauf.wahr("metrics_version_bindet_schwellen: `aktuell` hat einen Eintrag",
              aktuell in fassungen, aktuell)

    # Die Version im Code IST die des Registers.
    code_version = konstanten.get("kFeatureMetricsVersion", ("", ""))[0]
    lauf.wahr("metrics_version_bindet_schwellen: Code und Register nennen dieselbe Version",
              code_version == aktuell,
              f"Code {code_version!r}, Register {aktuell!r}")

    eintrag = fassungen.get(aktuell, {})
    fehlend, abweichend = [], []
    gefuehrt = 0
    for block, ganzzahlig in (("schwellen", False), ("ganzzahlige_schwellen", True)):
        for name, feld in eintrag.get(block, {}).items():
            gefuehrt += 1
            if name not in konstanten:
                fehlend.append(name)
                continue
            roh = konstanten[name][0]
            soll = feld.get("wert")
            try:
                passt = (int(roh) == int(soll)) if ganzzahlig else (float(roh) == float(soll))
            except ValueError:
                passt = False
            if not passt:
                abweichend.append(f"{name}: Code {roh}, Register {soll}")
            # Die Datei muss ebenfalls stimmen - sonst zeigte das Register auf
            # eine Stelle, an der die Zahl gar nicht steht.
            elif not feld.get("datei", "").endswith(konstanten[name][1]):
                abweichend.append(f"{name}: Register nennt {feld.get('datei')!r}, "
                                  f"gefunden in {konstanten[name][1]}")

    lauf.wahr("metrics_version_bindet_schwellen: die Fassung fuehrt Schwellen",
              gefuehrt >= 8, f"{gefuehrt} Eintraege")
    lauf.wahr("metrics_version_bindet_schwellen: jede gefuehrte Schwelle steht im Code",
              not fehlend, ", ".join(fehlend))
    lauf.wahr("metrics_version_bindet_schwellen: und mit demselben Wert an derselben Stelle",
              not abweichend, "; ".join(abweichend))


def main(argv: list[str]) -> int:
    schema = json_laden_strikt(SCHEMA.read_text(encoding="utf-8"))
    reserviert = json_laden_strikt(RESERVIERT.read_text(encoding="utf-8"))
    quantisierung = json_laden_strikt(QUANTISIERUNG.read_text(encoding="utf-8"))
    manifest = json_laden_strikt((FIXTURES / "MANIFEST.json").read_text(encoding="utf-8"))

    lauf = Lauf()
    pruefe_textriegel(lauf)
    pruefe_schema(lauf, schema)
    pruefe_discriminator_enginekante(lauf)
    pruefe_namen(lauf, schema, reserviert)
    pruefe_sonde013_fassung_2(lauf, schema, reserviert)
    pruefe_probe_descriptor(lauf, schema, reserviert)
    pruefe_runtime_und_p2_reject(lauf, schema)
    pruefe_session_command_und_store(lauf, schema)
    pruefe_bandkodierung(lauf, schema, quantisierung)
    pruefe_command_ack(lauf, schema)
    pruefe_fixtures(lauf, schema, manifest)
    pruefe_metrikregister(lauf)

    print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
    print(f"{len(schema['$defs'])} Definitionen, {len(schema['oneOf'])} Nachrichtenfamilien, "
          f"{len(manifest['fixtures'])} Fixtures")
    print(f"Pruefungen: {lauf.ok} bestanden, {len(lauf.fehler)} gescheitert")
    for f in lauf.fehler:
        print(f"  ROT: {f}")

    offene, zaehlung, harte_luecken = abdeckung(schema, manifest)

    if "--abdeckung" in argv:
        print()
        print("Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).")
        print("HART sind nur enum und const: dort ist ein falscher Wert ein echtes")
        print("Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit")
        print("Unterabdeckung sichtbar bleibt statt still zu sein.")
        for name, (a, b) in zaehlung.items():
            hart = " HART" if name in ("enum", "const") else ""
            print(f"  {name:<22} {a:>4} / {b:<4}{hart}")
        print()
        if offene:
            print(f"$defs OHNE Negativfixture ({len(offene)}):")
            for d in offene:
                print(f"  {d}")
        else:
            print("$defs ohne Negativfixture: keine")
        if harte_luecken:
            print()
            print(f"Harte Luecken ({len(harte_luecken)}):")
            for h in harte_luecken:
                print(f"  {h}")

    if lauf.fehler:
        return 2
    if "--abdeckung" in argv:
        leer = [k for k, (a, _) in zaehlung.items() if a == 0]
        if leer:
            print(f"ROT: Schluesselwoerter ohne jedes Negativfixture: {leer}")
            return 2
        if offene:
            print(f"ROT: {len(offene)} Definitionen ohne Negativfixture")
            return 2
        if harte_luecken:
            print(f"ROT: {len(harte_luecken)} harte Luecken (enum/const ohne Negativfixture)")
            return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
