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

EXITCODES (wie tools/beweise.ps1 sie liest)
-------------------------------------------
0 gruen · 2 Behauptung widerlegt · 3 Voraussetzung fehlt (Datei/Modul)
"""

from __future__ import annotations

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


def pruefe_produkteingaenge(fehler: list[str]) -> None:
    """Die v2-Eintraege der Produkteingangstabelle gegen `vertrag` (Punkt 4)."""
    import jsonschema
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    # Der EINE strenge Lauf der Python-Seite - keine zweite Fassung hier.
    from pruefe_v3_vertrag import json_laden_strikt, stufe_des_strengen_laufs

    if not PRODUKTEINGAENGE.exists():
        fehler.append(f"{PRODUKTEINGAENGE.name}: Datei fehlt")
        return
    tabelle = json_laden_strikt(PRODUKTEINGAENGE.read_text(encoding="utf-8"))
    vertrag = json_laden_strikt((SCHEMA_VERZEICHNIS / "eq-ipc.schema.json").read_text(encoding="utf-8"))
    print()
    gefahren = 0
    for fall in tabelle["faelle"]:
        if fall["fassung"] != "v2":
            continue
        gefahren += 1
        name = f"{fall['id']} {fall['eingang']} {fall['nachricht']} {fall['matrix']}"
        if fall.get("wert") is not None:
            # Fail-closed: Werte liest dieses Bein erst ab Etappe 5 (Decimal).
            fehler.append(f"{name}: traegt einen Wert, dieses Bein vergleicht noch keine")
            continue
        definition = vertrag["$defs"].get(fall["nachricht"])
        if definition is None:
            fehler.append(f"{name}: der v2-Vertrag hat keine Definition {fall['nachricht']}")
            continue
        try:
            daten = json_laden_strikt(bytes.fromhex(fall["bytes_hex"]).decode("utf-8"))
            ist = ("gueltig", None) if jsonschema.Draft202012Validator(
                {"$defs": vertrag["$defs"], "$ref": f"#/$defs/{fall['nachricht']}"}
            ).is_valid(daten) else ("ungueltig", "schema")
        except (json.JSONDecodeError, ValueError) as e:
            ist = ("ungueltig", stufe_des_strengen_laufs(e))
        soll = (fall["vertrag"]["urteil"], fall["vertrag"]["stufe"])
        print(f"{'ok  ' if ist == soll else 'ROT '} {name}: ist {ist[0]}/{ist[1]}, "
              f"soll {soll[0]}/{soll[1]}")
        if ist != soll:
            fehler.append(f"{name}: Vertragsurteil {ist}, erwartet {soll}")
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
          "v2-Eintraege der Produkteingangstabelle wie `vertrag` klassifiziert und gezaehlt.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
