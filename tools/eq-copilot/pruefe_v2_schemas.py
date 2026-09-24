#!/usr/bin/env python3
"""Riegel fuer die fuenf v2-Vertraege unter eq-copilot/schemas/*.schema.json.

ANLASS (Kontext-Inventur 21.08.2026)
------------------------------------
`eq-snapshot.schema.json` — der als "live" gefuehrte Snapshot-Vertrag v3 — war
seit dem 15.08. KEIN gueltiges JSON: zwei Beschreibungstexte schlossen ein
deutsches „Zitat mit einem unmaskierten ASCII-Anfuehrungszeichen. Sechs Tage
lang hat es niemand gemerkt, weil kein Test, kein Skript und kein Binary eines
der fuenf v2-Schemas maschinell laedt; sie wurden nur in Kommentaren zitiert.
Ein Vertrag, den keine Maschine liest, ist keiner. Dieses Bein liest sie.

WAS GEPRUEFT WIRD
-----------------
1. Jede Datei ist gueltiges JSON (UTF-8).
2. Jede Datei ist ein gueltiges JSON Schema nach dem Draft, den sie in
   `$schema` nennt (Metaschema-Pruefung mit `jsonschema`, wie beim v3-Referenzbein).
3. `$schema` und `$id` sind vorhanden, und die `$id`-Menge ist genau die
   eingefrorene Familie unten — ein still umbenannter oder neu dazugelegter
   Vertrag faellt auf.
4. Seit NAK-313 Etappe 4 (R-313-6, E-313-11) die v2-Eintraege der
   Produkteingangstabelle (`fixtures/v3/PRODUKTEINGAENGE-FAELLE.json`): jede
   Instanz laeuft durch DENSELBEN strengen Parselauf wie das v3-Referenzbein
   (`json_laden_strikt` aus `pruefe_v3_vertrag.py`, Duplikat-Hook und
   Tiefengrenze), danach gegen die Definition ihres Nachrichtentyps im
   v2-Vertrag. Verglichen wird das VERTRAGSurteil samt Stufe (`vertrag`), je
   Eintrag ein Fall mit seiner Kennung; am Ende die Zaehlpruefung gegen
   `anzahl_je_fassung["v2"]`.
5. Seit NAK-313 Etappe 5 (R-313-5, A-4, E-313-11; M-95) liest dieser Lauf
   Zahlen mit Nachkommateil oder Exponent als `decimal.Decimal`: ein binary64
   machte aus `2.0000000000000001` die 2 und liesse den Vertrag ein
   `protocol_version` annehmen, das er nicht traegt. Der Typpruefer nimmt
   `integer` als `int` oder ganzzahligen `Decimal`, `number` samt `Decimal`
   (draft 2020-12). Bei eigenem Urteil `gueltig` vergleicht das Bein den Wert
   am Zeiger `feld` mit `wert` - als exakte Ganzzahl.

EXITCODES (wie tools/beweise.ps1 sie liest)
-------------------------------------------
0 gruen · 2 Behauptung widerlegt · 3 Voraussetzung fehlt (Datei/Modul)
"""

from __future__ import annotations

import decimal
import json
import sys
from pathlib import Path

WURZEL = Path(__file__).resolve().parents[2]
SCHEMA_VERZEICHNIS = WURZEL / "eq-copilot" / "schemas"
PRODUKTEINGAENGE = WURZEL / "eq-copilot" / "fixtures" / "v3" / "PRODUKTEINGAENGE-FAELLE.json"

# Eingefrorene Familie der v2-Vertraege: Dateiname -> erwartete $id.
# (v3 lebt in schemas/v3/ und hat eigene Beine: pruefe_v3_vertrag.py, SchemaTest, Rust.)
ERWARTET = {
    "eq-ipc.schema.json": "evenacadia.eq-copilot.ipc.v2",
    "eq-measurement.schema.json": "evenacadia.eq-copilot.measurement.v1",
    "eq-report.schema.json": "evenacadia.eq-copilot.report.v1",
    "eq-snapshot.schema.json": "evenacadia.eq-copilot.snapshot.v3",
    "eq-aggregat.schema.json": "evenacadia.eq-copilot.aggregat.v1",
}


def ist_ganzzahl(_pruefer, instanz) -> bool:
    """draft 2020-12: `integer` ist jede Zahl ohne Nachkommateil - hier auch
    ein ganzzahliger `Decimal`. Ein positiver Exponent ist ohne Rechnung
    ganzzahlig (`2e4294967296` wird nie ausgeschrieben)."""
    if isinstance(instanz, bool):
        return False
    if isinstance(instanz, int):
        return True
    if isinstance(instanz, decimal.Decimal):
        return instanz.is_finite() and (instanz.as_tuple().exponent >= 0
                                        or instanz == instanz.to_integral_value())
    return isinstance(instanz, float) and instanz.is_integer()


def ist_zahl(_pruefer, instanz) -> bool:
    """draft 2020-12: `number` schliesst `integer` ein - hier samt `Decimal`."""
    return not isinstance(instanz, bool) and isinstance(instanz, (int, float, decimal.Decimal))


def ganzzahl_gleich(wert, soll: str) -> bool:
    """Ist `wert` genau die Ganzzahl `soll`? Exakt, nie ueber binary64."""
    if isinstance(wert, bool) or not ist_ganzzahl(None, wert):
        return False
    return int(wert) == int(soll)


def pruefe_produkteingaenge(fehler: list[str]) -> None:
    """Die v2-Eintraege der Produkteingangstabelle gegen `vertrag` (Punkte 4, 5)."""
    import jsonschema
    from jsonschema import validators
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    # Der EINE strenge Lauf der Python-Seite - keine zweite Fassung hier.
    from pruefe_v3_vertrag import json_laden_strikt, stufe_des_strengen_laufs, wert_am_zeiger

    if not PRODUKTEINGAENGE.exists():
        fehler.append(f"{PRODUKTEINGAENGE.name}: Datei fehlt")
        return
    tabelle = json_laden_strikt(PRODUKTEINGAENGE.read_text(encoding="utf-8"))
    vertrag = json_laden_strikt((SCHEMA_VERZEICHNIS / "eq-ipc.schema.json").read_text(encoding="utf-8"))
    typen = jsonschema.Draft202012Validator.TYPE_CHECKER.redefine_many(
        {"integer": ist_ganzzahl, "number": ist_zahl})
    Pruefer = validators.extend(jsonschema.Draft202012Validator, type_checker=typen)
    print()
    gefahren = 0
    for fall in tabelle["faelle"]:
        if fall["fassung"] != "v2":
            continue
        gefahren += 1
        name = f"{fall['id']} {fall['eingang']} {fall['nachricht']} {fall['matrix']}"
        definition = vertrag["$defs"].get(fall["nachricht"])
        if definition is None:
            fehler.append(f"{name}: der v2-Vertrag hat keine Definition {fall['nachricht']}")
            continue
        daten = None
        try:
            daten = json_laden_strikt(bytes.fromhex(fall["bytes_hex"]).decode("utf-8"),
                                      parse_float=decimal.Decimal)
            ist = ("gueltig", None) if Pruefer(
                {"$defs": vertrag["$defs"], "$ref": f"#/$defs/{fall['nachricht']}"}
            ).is_valid(daten) else ("ungueltig", "schema")
        except (json.JSONDecodeError, ValueError) as e:
            ist = ("ungueltig", stufe_des_strengen_laufs(e))
        soll = (fall["vertrag"]["urteil"], fall["vertrag"]["stufe"])
        print(f"{'ok  ' if ist == soll else 'ROT '} {name}: ist {ist[0]}/{ist[1]}, "
              f"soll {soll[0]}/{soll[1]}")
        if ist != soll:
            fehler.append(f"{name}: Vertragsurteil {ist}, erwartet {soll}")
        if ist[0] == "gueltig" and fall.get("wert") is not None:
            try:
                gelesen = wert_am_zeiger(daten, fall["feld"])
            except (KeyError, IndexError, TypeError) as e:
                gelesen = f"nicht lesbar: {e}"
            gleich = ganzzahl_gleich(gelesen, fall["wert"])
            print(f"{'ok  ' if gleich else 'ROT '} {name}: Wert {gelesen!r}, soll {fall['wert']}")
            if not gleich:
                fehler.append(f"{name}: Wert {gelesen!r}, erwartet {fall['wert']}")
    soll_anzahl = tabelle["anzahl_je_fassung"].get("v2", 0)
    print(f"Produkteingaenge: {gefahren} v2-Eintraege gefahren, der Kopf nennt {soll_anzahl}")
    if gefahren != soll_anzahl or gefahren == 0:
        fehler.append(f"Zaehlpruefung: {gefahren} v2-Eintraege gefahren, der Kopf nennt {soll_anzahl}")


def main() -> int:
    try:
        import jsonschema
        from jsonschema import validators
    except ImportError:
        print("VORAUSSETZUNG FEHLT: Python-Modul `jsonschema` (py -3.13 -m pip install jsonschema)")
        return 3

    if not SCHEMA_VERZEICHNIS.is_dir():
        print(f"VORAUSSETZUNG FEHLT: {SCHEMA_VERZEICHNIS}")
        return 3

    vorhanden = sorted(p.name for p in SCHEMA_VERZEICHNIS.glob("*.schema.json"))
    fehler: list[str] = []

    unerwartet = sorted(set(vorhanden) - set(ERWARTET))
    fehlend = sorted(set(ERWARTET) - set(vorhanden))
    for name in fehlend:
        fehler.append(f"{name}: Datei fehlt")
    for name in unerwartet:
        fehler.append(f"{name}: nicht in der eingefrorenen Familie (ERWARTET in diesem Skript nachziehen)")

    print(f"{'Datei':<32} {'$id':<42} JSON  Metaschema")
    for name in vorhanden:
        pfad = SCHEMA_VERZEICHNIS / name
        roh = pfad.read_bytes()
        try:
            schema = json.loads(roh.decode("utf-8"))
            json_ok = "ok"
        except (UnicodeDecodeError, json.JSONDecodeError) as e:
            fehler.append(f"{name}: kein gueltiges JSON — {e}")
            print(f"{name:<32} {'-':<42} FEHLT -")
            continue

        ident = schema.get("$id", "")
        draft = schema.get("$schema", "")
        if not draft:
            fehler.append(f"{name}: `$schema` fehlt")
        if not ident:
            fehler.append(f"{name}: `$id` fehlt")
        elif name in ERWARTET and ident != ERWARTET[name]:
            fehler.append(f"{name}: `$id` ist {ident!r}, eingefroren ist {ERWARTET[name]!r}")

        meta_ok = "ok"
        try:
            cls = validators.validator_for(schema, default=jsonschema.Draft202012Validator)
            cls.check_schema(schema)
        except jsonschema.SchemaError as e:
            meta_ok = "FEHLT"
            fehler.append(f"{name}: kein gueltiges JSON Schema ({draft or 'ohne $schema'}) — {e.message}")

        print(f"{name:<32} {ident or '-':<42} {json_ok:<5} {meta_ok}")

    pruefe_produkteingaenge(fehler)

    print()
    if fehler:
        print(f"ROT — {len(fehler)} Befund(e):")
        for f in fehler:
            print(f"  - {f}")
        return 2

    print(f"GRUEN — {len(vorhanden)} v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren; "
          "v2-Eintraege der Produkteingangstabelle wie `vertrag` klassifiziert (Zahlen als "
          "Decimal), Werte exakt verglichen und gezaehlt.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
