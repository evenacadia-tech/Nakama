#!/usr/bin/env python3
"""Capabilityreport pruefen (SONDE-004 / S4, Entwurf §53.6).

Liest eq-copilot/identity/host-capabilities-fl-v1.json und prueft:

  1. `capabilities` ist genau das Objekt aus schemas/v3/eq-ipc-v3.schema.json
     $defs/capabilities (alle zehn Bits, nur supported|unsupported, nichts
     Zusaetzliches) - per jsonschema, wie das Referenzbein des v3-Vertrags.
  2. Jedes Bit, das auf eine Messung verweist, zeigt auf eine existierende
     Rohdatei in docs/beweise/termin-*/, und JEDE "feld = wert"-Angabe in den
     Belegtexten des Reports wird gegen die Rohdatei aufgeloest - der Report
     darf nichts behaupten, was die Messung nicht traegt (T2-Befund 6: die
     erste Fassung mass nur Skriptkonstanten, nicht den Report).
  3. Ein `supported` braucht einen Termin; `unsupported` braucht den festen
     Fallback aus §53.6.
  4. `host_channel_context` bleibt als Nicht-Wire-Beleg klar vom strikten
     Zehnerobjekt getrennt; bis Termin C ist er ehrlich `unsupported` und
     "nicht gemessen" mit dem U20-Fallback.
  5. Die Smart-Disable-Aussage wird an den Ereignissen gemessen (kein
     zeitsprung_vor in den stummen Fenstern, regelmaessige Wrap-Kadenz) -
     `block_ohne_verarbeitung` zaehlt nur Bruecken-Asymmetrie (T2-Befund 5).

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
    pruefe(len(caps) == 10 and "host_channel_context" not in caps,
           "host_channel_context ist kein elftes Wire-Bit im capabilities-Objekt")
    pruefe(set(report["belege"]) == erwartet, "jedes Bit hat einen Beleg")

    # SONDE-012 E-C01: dokumentarische Hostfaehigkeit, nicht Wire-Vertrag.
    kanal = report.get("host_channel_context", {})
    pflicht_kanal = {
        "wert", "beweis_nach_53_6", "termin", "rohfeld", "fallback_nach_53_6",
    }
    erlaubte_kanal = pflicht_kanal | {"datei"}
    pruefe(pflicht_kanal <= set(kanal) and set(kanal) <= erlaubte_kanal,
           "host_channel_context folgt der §53.6-Belegform und bleibt eigener Abschnitt")
    pruefe(kanal.get("wert") in ("supported", "unsupported"),
           "host_channel_context kennt nur supported|unsupported, kein unknown")
    pruefe(kanal.get("fallback_nach_53_6")
           == "U20-Rueckfall: gespeichertes User-Label, keine FL-Reihenfolge-Behauptung",
           "host_channel_context traegt exakt den U20-Fallback ohne FL-Reihenfolge")

    if kanal.get("termin") == "keiner":
        pruefe(kanal.get("wert") == "unsupported"
               and kanal.get("beweis_nach_53_6") == "nicht gemessen"
               and kanal.get("rohfeld") == "nicht gemessen"
               and "datei" not in kanal,
               "channel_context_unmeasured_is_unsupported_with_no_termin_and_fallback")
    else:
        datei = str(kanal.get("datei", ""))
        pruefe(kanal.get("termin") == "C" and datei.startswith("docs/beweise/termin-c/")
               and datei.endswith(".json") and (WURZEL / datei).is_file()
               and kanal.get("beweis_nach_53_6") != "nicht gemessen"
               and kanal.get("rohfeld") != "nicht gemessen",
               "gemessener host_channel_context verlangt Termin-C-Golden und Rohbeleg")

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
    pruefe(b["host"] == "FruityLoops" and b["wrapper"] == "VST3"
           and a1["host"] == "FruityLoops" and a2["host"] == "FruityLoops"
           and b["bloecke"]["samplerate"] == a1["samplerate"] == a2["samplerate"] == report["host"]["samplerate_hz"],
           "Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report")
    pruefe(b["ereignisse_je_art"]["kontext_weg"] == 0 and b["presentation_latency"]["verworfene_busmeldungen"] == 0
           and b["automation"]["ueberlaeufe"] == 0 and b["automation"]["unplausibel"] == 0,
           "keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte")
    pruefe("fl_version_termin_b" in report["host"] and "juce_version" in report["host"]
           and "26.1.4.5589" in report["host"]["fl_version_termin_b"] and "8.0.9" in report["host"]["juce_version"],
           "Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)")
    # Smart Disable: die beiden stummen Fenster sind die Transportabschnitte
    # NACH dem Export-Abschnitt (Ereignis 68..74) - das erste (13:02:45) liegt
    # davor: Ereignisse 61..67; das zweite (13:11:35) ist der letzte Abschnitt.
    ev = b["ereignisse"]
    pruefe(b["ereignisse_gesamt"] == b["ereignisse_gespeichert"] == len(ev),
           f"Ereignisprotokoll vollstaendig ({len(ev)} Ereignisse, kein Ringueberlauf)")
    abschnitte = []
    start = None
    for i, e in enumerate(ev):
        if e["art"] == "transport_an":
            start = i
        elif e["art"] == "transport_aus" and start is not None:
            abschnitte.append((start, i))
            start = None
    pruefe(len(abschnitte) == 8, f"8 Transportabschnitte (an..aus) gefunden: {len(abschnitte)}")
    stumm = [abschnitte[4], abschnitte[7]]   # 13:02:45 (stumm + Ton, Ereignisse 61..66) und 13:11:35 (88..96)
    for (a, z) in stumm:
        arten = [x["art"] for x in ev[a:z + 1]]
        wraps = [x["block"] for x in ev[a:z + 1] if x["art"] == "zeitsprung_zurueck"]
        kadenz = [wraps[i + 1] - wraps[i] for i in range(len(wraps) - 1)]
        pruefe("zeitsprung_vor" not in arten and len(wraps) >= 3 and max(kadenz) - min(kadenz) <= 4,
               f"Smart Disable: stummes Fenster Ereignisse {a}..{z} ohne zeitsprung_vor, Wrap-Kadenz {kadenz} regelmaessig")
    vor = [i for i, x in enumerate(ev) if x["art"] == "zeitsprung_vor"]
    pruefe(vor == [79, 82], f"genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse {vor})")

    for lauf, name in ((a1, "001701"), (a2, "002722")):
        busse = {x["name"]: x for x in lauf["busse"]}
        for bus in ("priority_sidechain", "compare_pre"):
            x = busse[bus]
            pruefe(x["aktiv"] and x["kanaele"] == 2 and x["protokoll_eingehalten"]
                   and x["versatz_zu_main_samples"] == 0 and x["versatz_zu_main_ms"] == 0.0,
                   f"Termin A {name}: {bus} aktiv, 2 Kanaele, Versatz 0")

    # 2b. Jede "feld = wert"-Angabe der Belegtexte gegen die Rohdateien.
    #     Belegtexte duerfen Kurzformen tragen ("projektzeit.spruenge_vorwaerts = 2,
    #     spruenge_rueckwaerts = 51" / "busse[1] (...): aktiv = true"); der
    #     zuletzt genannte Pfadkopf gilt weiter. Segmente mit "A:" messen gegen
    #     BEIDE Termin-A-Laeufe, alles andere gegen Termin B.
    import re

    def loese(obj, pfad):
        for teil in pfad.split("."):
            m = re.fullmatch(r"(\w+)\[(\d+)\]", teil)
            obj = obj[m.group(1)][int(m.group(2))] if m else obj[teil]
        return obj

    def gleich(ist, soll: str) -> bool:
        if isinstance(ist, bool):
            return soll.lower() == str(ist).lower()
        if isinstance(ist, (int, float)):
            try:
                return float(ist) == float(soll)
            except ValueError:
                return False
        return str(ist) == soll

    token = re.compile(r"(?P<kopf>busse\[\d+\])(?=\s*\()|(?P<pfad>[A-Za-z_]+(?:\.[A-Za-z_]+|\[\d+\])*)\s*=\s*(?P<wert>true|false|[-\d.]+)")
    angaben = 0
    for bit, beleg in report["belege"].items():
        text = beleg.get("rohfeld", "")
        for segment in re.split(r"(?=\bA:\s)|(?=\bB:\s)", text):
            segment = segment.strip()
            if not segment or segment.startswith("nicht gemessen") or segment.startswith("Beweis entsteht"):
                continue
            quellen = [("A1", a1), ("A2", a2)] if segment.startswith("A:") or "busse[" in segment else [("B", b)]
            praefix = ""
            for m in token.finditer(segment):
                if m.group("kopf"):
                    praefix = m.group("kopf")
                    continue
                pfad, soll = m.group("pfad"), m.group("wert")
                for name, quelle in quellen:
                    ist = None
                    for kandidat in ([pfad] + ([praefix + "." + pfad] if praefix else [])):
                        try:
                            ist = loese(quelle, kandidat)
                            if "." in kandidat and kandidat == pfad:
                                praefix = kandidat.rsplit(".", 1)[0]
                            break
                        except (KeyError, IndexError, TypeError):
                            continue
                    if ist is None:
                        pruefe(False, f"{bit}: Rohfeld {pfad} (Praefix '{praefix}') existiert nicht in {name}")
                        continue
                    angaben += 1
                    if not gleich(ist, soll):
                        pruefe(False, f"{bit}: Rohfeld {pfad} in {name} = {ist!r}, Report sagt {soll}")
    pruefe(angaben >= 30, f"{angaben} 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen")

    # 3. Bits gegen die Belege.
    for bit, wert in caps.items():
        beleg = report["belege"][bit]
        if wert == "supported":
            pruefe(beleg.get("termin") in ("A", "B", "A + B"),
                   f"{bit}=supported traegt einen Termin ({beleg.get('termin')})")
            pruefe("Golden" not in beleg.get("zusatz", "") or "nicht erbracht" not in beleg.get("zusatz", ""),
                   f"{bit}=supported behauptet kein 'Golden nicht erbracht'")
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

    # Die Bits, die die Rohdaten TRAGEN (§53.6 Golden erbracht) - alles andere
    # ist unsupported. presentation_latency und aux_priority_sidechain wurden
    # in T2-Runde 1 herabgestuft: Meldung ohne Impulsgolden bzw. Sidechain ohne
    # PDC-Last sind kein Golden.
    erwartete_bits = {
        "host_context_presence": "supported", "project_time_samples": "supported",
        "sample_accurate_automation": "unsupported", "presentation_latency": "unsupported",
        "aux_compare_pre": "unsupported", "aux_priority_sidechain": "unsupported",
        "contribution_aux": "unsupported", "float64_processing": "unsupported",
        "binary_telemetry": "unsupported", "remote_control": "unsupported",
    }
    pruefe(caps == erwartete_bits, "die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)")
    pruefe(report["belege"]["presentation_latency"].get("fallback_nach_53_6") == "keine subtraktive Cross-Probe-Ausrichtung"
           and report["belege"]["aux_priority_sidechain"].get("fallback_nach_53_6") == "keine dynamische Aktuation"
           and report["belege"]["aux_compare_pre"].get("fallback_nach_53_6") == "nur Zustands-A/B, kein lokales Audio-Delta",
           "herabgestufte Bits tragen die Fallbacks aus §53.6")

    # 2c. gemessene_hosttatsachen und die Latenzzahlen des Belegtexts gegen die
    #     Rohdaten (T2-Runde 2, Befund 7: Freitextzahlen waren ungeprueft).
    ht = report["gemessene_hosttatsachen"]
    pruefe(ht["blockgroesse_min_samples"] == b["bloecke"]["blockgroesse_min"]
           and ht["blockgroesse_max_samples"] == b["bloecke"]["blockgroesse_max"]
           and float(ht["tempo_bpm"]) == b["projektzeit"]["letztes_tempo"],
           "gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern")
    lat = report["belege"]["presentation_latency"]["rohfeld"]
    pruefe(f"eingang Bus 0 = {gemeldet[('eingang', 0)]}" in lat and f"ausgang Bus 0 = {gemeldet[('ausgang', 0)]}" in lat,
           "Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte")
    zahlen_text = ht["seeks"]
    for idx, feld in ((38, "projektzeit"), (40, "projektzeit"), (41, "projektzeit"), (79, "zusatz"), (82, "zusatz"), (84, "zusatz"), (50, "zusatz")):
        wert = ev[idx][feld]
        pruefe(str(int(abs(wert))) in zahlen_text and ev[idx]["art"].startswith("zeitsprung"),
               f"gemessene_hosttatsachen.seeks nennt Ereignis {idx} ({ev[idx]['art']} {int(wert)})")
    # Wraps landen bei Projektzeit 0/1 - meine Seeks (38/40/41) nicht.
    wraps_song = sorted({int(abs(x["zusatz"])) for x in ev[31:44] if x["art"] == "zeitsprung_zurueck" and x["projektzeit"] <= 1})
    pruefe(wraps_song == [539634, 539635] and "539634-539635" in ht["song_loop_vor_export"],
           f"Song-Loop vor Export = {wraps_song} Samples wie im Report")
    wraps_pattern = {int(abs(x["zusatz"])) for x in ev[51:59] if x["art"] == "zeitsprung_zurueck"}
    pruefe(wraps_pattern == {173250} and "173250" in ht["pattern_loop"], "Pattern-Loop = 173250 Samples wie im Report")
    wraps_nach = {int(abs(x["zusatz"])) for x in ev[88:97] if x["art"] == "zeitsprung_zurueck"}
    pruefe(wraps_nach <= {509118, 509119} and "509119" in ht["song_loop_nach_export"], f"Song-Loop nach Export = {sorted(wraps_nach)} Samples wie im Report (509119)")
    erster = json.loads((BEWEISE / "termin-b" / "host-probe-20260822-130657.json").read_text(encoding="utf-8"))
    pruefe(erster["ereignisse"] == ev[:len(erster["ereignisse"])] and len(erster["ereignisse"]) == 75,
           "der erste Bericht (75 Ereignisse) ist das Praefix des zweiten")
    zaehler = {}
    for x in ev:
        zaehler[x["art"]] = zaehler.get(x["art"], 0) + 1
    pruefe(all(b["ereignisse_je_art"].get(k, 0) == v for k, v in zaehler.items()),
           "ereignisse_je_art stimmt mit der Ereignisliste ueberein")

    print()
    if fehler:
        print(f"HOST-CAPABILITIES FEHLGESCHLAGEN - {ok} ok, {len(fehler)} Fehler")
        return 2
    print(f"HOST-CAPABILITIES OK - {ok} Pruefungen ok, 0 Fehler")
    return 0


if __name__ == "__main__":
    sys.exit(main())
