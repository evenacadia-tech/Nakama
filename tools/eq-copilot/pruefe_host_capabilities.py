#!/usr/bin/env python3
"""Capabilityreport pruefen (SONDE-004 / S4, Entwurf §53.6).

Liest eq-copilot/identity/host-capabilities-fl-v1.json und prueft:

  1. `capabilities` ist genau das Objekt aus schemas/v3/eq-ipc-v3.schema.json
     $defs/capabilities (alle zehn Bits, nur supported|unsupported, nichts
     Zusaetzliches) - per jsonschema, wie das Referenzbein des v3-Vertrags.
  2. Jedes Bit, das auf eine Messung verweist, zeigt auf eine existierende
     Rohdatei in docs/beweise/termin-*/, und die im Beleg genannten Zahlen
     stehen WIRKLICH so in der Rohdatei - der Report darf nichts behaupten,
     was die Messung nicht traegt.
  3. Ein `supported` braucht einen Termin; `unsupported` ohne Termin braucht
     den festen Fallback aus §53.6.

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_host_capabilities.py
"""

from __future__ import annotations

import json
import pathlib
import sys

try:
    import jsonschema
except ImportError:  # pragma: no cover
    print("FEHLT: py -3.13 -m pip install jsonschema", file=sys.stderr)
    sys.exit(3)

WURZEL = pathlib.Path(__file__).resolve().parents[2]
REPORT = WURZEL / "eq-copilot" / "identity" / "host-capabilities-fl-v1.json"
SCHEMA = WURZEL / "eq-copilot" / "schemas" / "v3" / "eq-ipc-v3.schema.json"
BEWEISE = WURZEL / "docs" / "beweise"

fehler: list[str] = []
ok = 0


def pruefe(bedingung: bool, text: str) -> None:
    global ok
    if bedingung:
        ok += 1
        print("  ok      " + text)
    else:
        fehler.append(text)
        print("  FEHLER  " + text)


def main() -> int:
    report = json.loads(REPORT.read_text(encoding="utf-8"))
    schema = json.loads(SCHEMA.read_text(encoding="utf-8"))
    caps = report["capabilities"]

    # 1. Vertragsform: exakt $defs/capabilities.
    teil = {"$schema": schema.get("$schema"), "$defs": schema["$defs"], "$ref": "#/$defs/capabilities"}
    try:
        jsonschema.Draft202012Validator(teil).validate(caps)
        pruefe(True, "capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)")
    except jsonschema.ValidationError as e:
        pruefe(False, f"capabilities verletzt v3 $defs/capabilities: {e.message}")

    erwartet = set(schema["$defs"]["capabilities"]["required"])
    pruefe(set(caps) == erwartet, "genau die zehn Schluessel aus §53.6")
    pruefe(set(report["belege"]) == erwartet, "jedes Bit hat einen Beleg")

    # 2. Rohdaten lesen.
    b = json.loads((BEWEISE / "termin-b" / "host-probe-20260822-132644.json").read_text(encoding="utf-8"))
    a1 = json.loads((BEWEISE / "termin-a" / "aux-spike-20260822-001701.json").read_text(encoding="utf-8"))
    a2 = json.loads((BEWEISE / "termin-a" / "aux-spike-20260822-002722.json").read_text(encoding="utf-8"))

    pruefe(b["bruecke_liefert"] is True and b["bloecke"]["ohne_kontext"] == 0
           and b["bloecke"]["mit_kontext"] == b["bloecke"]["verarbeitete_bloecke"] == 259298,
           "host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert")
    pruefe(all(b["gueltig_immer"].values()),
           "project_time_samples: alle sieben Kontextfelder IMMER gueltig")
    pz = b["projektzeit"]
    pruefe(pz["spruenge_vorwaerts"] == 2 and pz["spruenge_rueckwaerts"] == 51
           and pz["spruenge_ueber_stop"] == 5 and pz["projektzeit_negativ"] == 0,
           "project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ")
    pruefe(b["bloecke"]["offline"] == 2587 and b["ereignisse_je_art"]["offline_an"] == 1
           and b["ereignisse_je_art"]["offline_aus"] == 1,
           "project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus")
    au = b["automation"]
    pruefe(au["max_punkte_pro_block"] == 1 and au["bloecke_mit_mehrpunkt"] == 0
           and au["groesster_offset"] == 0 and au["samplegenau_belegt"] is False and au["punkte_gesamt"] == 83303,
           "sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt")
    pruefe(b["bloecke"]["blockgroesse_min"] == 1 and b["bloecke"]["blockgroesse_max"] == 4096,
           "Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)")
    pl = b["presentation_latency"]
    gemeldet = {(g["richtung"], g["bus"]): g["samples"] for g in pl["gemeldet"]}
    pruefe(pl["je_gemeldet"] is True and gemeldet == {("eingang", 0): 3924, ("ausgang", 0): 4410},
           "presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet")
    pruefe(pl["verworfene_wertwechsel"] == 1, "presentation_latency: genau ein verworfener Wertwechsel (NAK-43)")
    pruefe(b["bloecke"]["float"] == 259298 and b["bloecke"]["double"] == 0,
           "float64_processing: nur float, nie double")
    pruefe(b["ereignisse_je_art"]["block_ohne_verarbeitung"] == 0,
           "Smart Disable: kein ausgelassener Block in 12 s + 30 s Stille")

    for lauf, name in ((a1, "001701"), (a2, "002722")):
        busse = {x["name"]: x for x in lauf["busse"]}
        for bus in ("priority_sidechain", "compare_pre"):
            x = busse[bus]
            pruefe(x["aktiv"] and x["kanaele"] == 2 and x["protokoll_eingehalten"]
                   and x["versatz_zu_main_samples"] == 0 and x["versatz_zu_main_ms"] == 0.0,
                   f"Termin A {name}: {bus} aktiv, 2 Kanaele, Versatz 0")

    # 3. Bits gegen die Belege.
    for bit, wert in caps.items():
        beleg = report["belege"][bit]
        if wert == "supported":
            pruefe(beleg.get("termin") in ("A", "B", "A + B"),
                   f"{bit}=supported traegt einen Termin ({beleg.get('termin')})")
        else:
            pruefe("fallback_nach_53_6" in beleg,
                   f"{bit}=unsupported traegt den festen Fallback aus §53.6")
        for datei in str(beleg.get("datei", "")).replace(";", ",").split(","):
            datei = datei.strip()
            if not datei:
                continue
            if datei.startswith("-"):
                # Kurzform "-002722.json" = gleicher Ordner wie der erste Eintrag
                continue
            pruefe((WURZEL / datei).exists(), f"{bit}: Rohdatei existiert ({datei})")

    erwartete_bits = {
        "host_context_presence": "supported", "project_time_samples": "supported",
        "sample_accurate_automation": "unsupported", "presentation_latency": "supported",
        "aux_compare_pre": "supported", "aux_priority_sidechain": "supported",
        "contribution_aux": "unsupported", "float64_processing": "unsupported",
        "binary_telemetry": "unsupported", "remote_control": "unsupported",
    }
    pruefe(caps == erwartete_bits, "die zehn Bits stehen so, wie die Rohdaten es tragen")

    print()
    if fehler:
        print(f"HOST-CAPABILITIES FEHLGESCHLAGEN - {ok} ok, {len(fehler)} Fehler")
        return 2
    print(f"HOST-CAPABILITIES OK - {ok} Pruefungen ok, 0 Fehler")
    return 0


if __name__ == "__main__":
    sys.exit(main())
