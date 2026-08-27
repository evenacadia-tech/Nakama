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

Die VERLETZUNGSMENGE prueft dieses Bein bewusst nicht: eine echte
JSON-Schema-Implementierung meldet bei `oneOf` die Fehler aller Zweige,
waehrend die eigenen Engines ueber den Discriminator in genau einen absteigen.
Beides ist korrekt, nur nicht vergleichbar (README).

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py
    py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung
"""

from __future__ import annotations

import json
import math
import pathlib
import sys

try:
    import jsonschema
except ImportError:
    print("VORAUSSETZUNG FEHLT: py -3.13 -m pip install jsonschema")
    sys.exit(3)

WURZEL = pathlib.Path(__file__).resolve().parents[2]
SCHEMA = WURZEL / "eq-copilot" / "schemas" / "v3" / "eq-ipc-v3.schema.json"
RESERVIERT = WURZEL / "eq-copilot" / "schemas" / "v3" / "reservierte-nachrichten-v1.json"
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
}


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


def ist_ascii_ziffer(c: str) -> bool:
    """NICHT `str.isdigit()`.

    T2-Runde 2, Befund BF-4: `str.isdigit()` ist auch fuer arabisch-indische
    Ziffern und Hochzahlen wahr, waehrend Rusts `is_ascii_digit` und C++
    `c >= '0' && c <= '9'` es nicht sind. Bei `{"w": 0\u0662}` meldete dieses
    Bein deshalb eine fuehrende Null und die anderen beiden nicht - drei
    Ziffernbegriffe waeren drei Grammatiken.
    """
    return "0" <= c <= "9"


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

    Acht Regeln, jede gegen eine GEMESSENE Abweichung zwischen den Beinen.
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
    tabelle = json.loads(FALLTABELLE.read_text(encoding="utf-8"))
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


def pruefe_probe_descriptor(lauf: Lauf, schema: dict) -> None:
    """§32.2-Kopplung: Messposition bestimmt die Aussageklasse (G1-Befund §4.1).

    Bis zum 24.08.2026 standen `measurement_position` und `aussageklasse`
    unabhaengig nebeneinander — `insert` + `beitrag` validierte, also eine
    gewoehnliche Insertmessung, die sich exakter Mastersummenbeitrag nennt
    (Gate 7 aus §49.2). Die Kopplung steht jetzt als diskriminierte Union.

    Der Preis dieser Form sind vier fast gleiche Zweige. Dieser Riegel macht
    daraus eine GEMESSENE Invariante: er verlangt, dass sich die vier Zweige in
    NICHTS unterscheiden ausser den beiden `const`. Ohne ihn waere jeder Zweig
    eine eigene Stelle, an der eine spaetere Feldaenderung haengenbleiben kann.
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
                  {"measurement_position", "aussageklasse"} <= set(zweig.get("required", [])))
        lauf.wahr(f"{name} ist strikt", zweig.get("additionalProperties") is False)

    # Der eigentliche Riegel: die vier Zweige duerfen sich NUR in den zwei
    # const unterscheiden. `description` ist Anmerkung und darf abweichen.
    def rumpf(name: str) -> dict:
        z = {k: v for k, v in defs[name].items() if k != "description"}
        z["properties"] = {k: v for k, v in z["properties"].items()
                           if k not in ("measurement_position", "aussageklasse")}
        return z

    erster = rumpf(next(iter(KOPPLUNG)))
    abweichend = [n for n in KOPPLUNG if rumpf(n) != erster]
    lauf.wahr("die vier Zweige unterscheiden sich NUR in den zwei const",
              not abweichend, ", ".join(abweichend))

    # Und die Gegenprobe zum Riegel selbst: er muss ueberhaupt etwas finden
    # koennen. Ein Riegel, der nichts FINDET, sagt nichts, bis gezeigt ist,
    # dass er etwas finden koennte (Lehre A14/SONDE-007a).
    verdorben = {n: rumpf(n) for n in KOPPLUNG}
    verdorben["probe_descriptor_pre"]["required"] = ["adresse"]
    lauf.wahr("Gegenprobe: ein verdorbener Zweig faellt am selben Vergleich",
              any(v != erster for v in verdorben.values()))


def pruefe_namen(lauf: Lauf, schema: dict, reserviert: dict) -> None:
    zweige = [r["$ref"].removeprefix("#/$defs/") for r in schema["oneOf"]]
    definiert = reserviert["definiert"]
    reserv = [r["name"] for r in reserviert["reserviert"]]

    lauf.wahr("oneOf-Zweige == definierte Liste", zweige == definiert,
              f"{set(zweige) ^ set(definiert)}")
    lauf.wahr("definiert und reserviert sind disjunkt",
              not (set(definiert) & set(reserv)), f"{set(definiert) & set(reserv)}")
    lauf.wahr(f"Summe ist {reserviert['gesamt_erwartet']} Familien (Entwurf §33.3)",
              len(definiert) + len(reserv) == reserviert["gesamt_erwartet"],
              f"{len(definiert)} + {len(reserv)}")
    lauf.wahr("jede definierte Familie hat ein $defs",
              all(n in schema["$defs"] for n in definiert))
    lauf.wahr("keine reservierte Familie hat ein $defs",
              not any(n in schema["$defs"] for n in reserv))
    lauf.wahr("jedes Eigentuemerticket ist genannt",
              all(r.get("eigentuemer") and r.get("grund") for r in reserviert["reserviert"]))


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
            daten = json.loads(roh_bytes.decode("utf-8"))
        except json.JSONDecodeError as e:
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
            sammle_benutzt(json.loads(pfad.read_text(encoding="utf-8")))
        except json.JSONDecodeError:
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

    diskriminatoren = {teil["x-nakama-discriminator"]
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

def main(argv: list[str]) -> int:
    schema = json.loads(SCHEMA.read_text(encoding="utf-8"))
    reserviert = json.loads(RESERVIERT.read_text(encoding="utf-8"))
    manifest = json.loads((FIXTURES / "MANIFEST.json").read_text(encoding="utf-8"))

    lauf = Lauf()
    pruefe_textriegel(lauf)
    pruefe_schema(lauf, schema)
    pruefe_namen(lauf, schema, reserviert)
    pruefe_probe_descriptor(lauf, schema)
    pruefe_fixtures(lauf, schema, manifest)

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
