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

MUSTER = {"^[0-9a-f]{32}$"}


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
BS = chr(92)                             # Backslash


def textriegel(text: str) -> str | None:
    """Prueft den ROHTEXT eines v3-Dokuments, BEVOR ihn ein Parser sieht.

    Warum vor dem Parser und nicht als Schemaregel: T2-Runde 1 hat gemessen,
    dass JUCEs `parseNumber` `intValue * 10 + digit` in einem `int64` ohne
    Bereichspruefung akkumuliert. `18446744073709552016` kommt auf der
    C++-Seite als 400 an — ein `maximum: 400` wuerde dort anstandslos
    passieren und auf der Rust-Seite fallen. Der Wert ist beim Ankommen
    bereits verfaelscht; der einzige Ort, an dem alle drei Beine dasselbe
    sehen koennen, ist der Text.

    Sechs Regeln, jede gegen eine GEMESSENE Abweichung. Wortgleiche
    Gegenstuecke: `NakamaVertrag.cpp` und `broker/src/vertrag.rs`.

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
                        try:
                            cp = int(roh, 16)
                        except ValueError:
                            return f"unlesbares u-Escape {roh!r} an Position {j}"
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

        if c == "-" or c.isdigit():
            j = i
            if text[j] == "-":
                j += 1
            anfang = j
            while j < n and text[j].isdigit():
                j += 1
            ganz = text[anfang:j]
            if not ganz:
                return f"Zahl ohne Ziffern an Position {i}"
            if len(ganz) > 1 and ganz[0] == "0":
                return f"fuehrende Null in {text[i:j]!r} an Position {i}"
            bruch = exp = False
            if j < n and text[j] == ".":
                bruch = True
                j += 1
                while j < n and text[j].isdigit():
                    j += 1
            if j < n and text[j] in "eE":
                exp = True
                j += 1
                if j < n and text[j] in "+-":
                    j += 1
                while j < n and text[j].isdigit():
                    j += 1
            lit = text[i:j]
            if not bruch and not exp:
                if len(ganz) > 16 or (len(ganz) == 16 and int(ganz) > SICHERE_GANZZAHL):
                    return f"Ganzzahl ausserhalb 2^53-1: {lit}"
            else:
                try:
                    d = float(lit)
                except ValueError:
                    return f"unlesbare Zahl {lit!r}"
                if not math.isfinite(d):
                    return f"nicht endliche Zahl: {lit}"
            i = j
            continue

        i += 1
    return None


# Dieselbe Tabelle steht in SchemaTestMain.cpp und broker/src/vertrag.rs.
# Laufen die drei auseinander, faellt genau hier eine von ihnen.
TEXTRIEGEL_FAELLE: list[tuple[str, bool]] = [
    ('{"w": 9007199254740991}', False),
    ('{"w": 9007199254740992}', True),
    ('{"w": -9007199254740991}', False),
    ('{"w": -9007199254740992}', True),
    ('{"w": 18446744073709552016}', True),
    ('{"w": 10000000000000000000}', True),
    ('{"w": 091}', True),
    ('{"w": -091}', True),
    ('{"w": 0}', False),
    ('{"w": -0}', False),
    ('{"w": 0.5}', False),
    ('{"w": 1e400}', True),
    ('{"w": -1e400}', True),
    ('{"w": 1e-400}', False),
    ('{"w": 1e300}', False),
    ('{"w": 1.5e3}', False),
    ('{"w": "091 nur Text"}', False),
    ('{"w": "1e400"}', False),
    ('{"w": "a' + BS + 'u0000b"}', True),
    ('{"w": "\U0001F600"}', False),
    ('{"w": "' + BS + 'ud83d"}', True),
    ('{"w": "' + BS + 'ude00"}', True),
    ('{"w": "' + BS + 'ud83dx"}', True),
    ('{"": 1}', True),
    ('{"a": {"": 2}}', True),
    ('{"w": ""}', False),
    ('{"w" : 1}', False),
    ('{"w": "er sagte ' + BS + '"hallo' + BS + '""}', False),
    ('{"w": "backslash am Ende ' + BS + BS + '"}', False),
    ('{"w": 512, "x": [1,2,3]}', False),
    ('{"w": "Doppelpunkt : im Text"}', False),
    ('{"w": "roher Tab: \t"}', True),
]


def pruefe_textriegel(lauf: Lauf) -> None:
    rot = [t for t, ab in TEXTRIEGEL_FAELLE if (textriegel(t) is not None) != ab]
    lauf.wahr(f"Textriegel deckt jede gemessene Kante ({len(TEXTRIEGEL_FAELLE)} Faelle)",
              not rot, "; ".join(rot))


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
        roh = pfad.read_text(encoding="utf-8")

        # Der Textriegel laeuft VOR dem Parser, und zwar ueber JEDES Fixture.
        # Die markierten muessen an ihm fallen, alle uebrigen ihn passieren.
        # Ohne die zweite Haelfte waere der Riegel eine Behauptung, die nur an
        # wenigen Dateien geprueft wird.
        grund = textriegel(roh)
        if eintrag.get("textriegel_lehnt_ab"):
            lauf.wahr(f"Textriegel lehnt ab: {eintrag['datei']}", grund is not None)
            continue
        if grund is not None:
            lauf.fehler.append(
                f"{eintrag['datei']}: Textriegel lehnt ab, soll passieren lassen: {grund}")
            continue

        try:
            daten = json.loads(roh)
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
