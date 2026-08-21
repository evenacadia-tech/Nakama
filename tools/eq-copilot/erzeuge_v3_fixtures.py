#!/usr/bin/env python3
"""Cross-Language-Fixture-Korpus fuer SONDE-005a (Entwurf §66.2).

Schreibt eq-copilot/fixtures/v3/{gueltig,ungueltig}/*.json und daneben
MANIFEST.json mit dem ERWARTETEN Urteil je Fixture — bei Ablehnung samt der
vollstaendigen Verletzungsmenge.

WARUM DIE ERWARTUNG VON HAND STEHT
----------------------------------
Das Manifest ist die dritte Partei zwischen C++ und Rust. Wuerde es aus einer
der beiden Engines (oder aus `jsonschema`) erzeugt, waere der Vergleich
zirkulaer: die Engine bestaetigte ihre eigene Ausgabe, und ein gemeinsamer
Denkfehler bliebe unsichtbar. Deshalb steht unten JEDE Erwartung als
handgeschriebene Zeile neben der Mutation, die sie ausloest. Eine falsch
geschriebene Erwartung faellt sofort auf: dann widersprechen ihr BEIDE
Engines, und die Frage ist am README zu entscheiden, nicht am Code.

AUFBAU
------
Aus wenigen gueltigen Grundnachrichten (eine je Familie) entstehen die
ungueltigen durch benannte Mutationen: `loesche`, `setze`, `ergaenze`. So ist
an jedem Fixture ablesbar, WELCHE Regel es bricht, statt dass 50 Dateien
nebeneinanderliegen und niemand mehr weiss, warum.

Aufruf:
    py -3.13 tools/eq-copilot/erzeuge_v3_fixtures.py
    py -3.13 tools/eq-copilot/erzeuge_v3_fixtures.py --pruefen
"""

from __future__ import annotations

import base64
import copy
import hashlib
import json
import pathlib
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
ZIEL = WURZEL / "eq-copilot" / "fixtures" / "v3"

S = "#/$defs"


# ------------------------------------------------------------------ Bausteine

ADRESSE = {
    "logon_sid": "S-1-5-21-1111111111-2222222222-3333333333-1001",
    "project_binding_id": "11111111111111111111111111111111",
    "session_epoch": "22222222222222222222222222222222",
    "instance_id": "33333333333333333333333333333333",
    "runtime_nonce": "44444444444444444444444444444444",
}

CAPS = {
    "process_context": "supported",
    "sample_accurate_automation": "supported",
    "presentation_latency": "unsupported",
    "discrete_aux_buses": "supported",
    "offline_render_detection": "supported",
    "double_precision": "unsupported",
    "remote_control": "unsupported",
}

ZAEHLER = {"frames_dropped": 0, "parse_errors": 0, "queue_overflows": 0}

FRISCHE = {"stale": False, "letzter_kontakt_ms": 120}

VALIDITY = {"project_time": True, "play_state": True, "record_state": True, "cycle_bounds": False}

TRANSPORT = {
    "transport_epoch": 17,
    "continuity_segment": 3,
    "sequence": 8241,
    "time_basis": "project_samples",
    "project_sample_start": 44108200,
    "sample_count": 512,
    "sample_rate": 48000,
    "playing": True,
    "recording": False,
    "validity": VALIDITY,
}

KONFIDENZ = {"metrics_version": 1, "klasse": "mittel", "timing_alignment": 0.8}


def bitmap(n: int) -> str:
    return base64.b64encode(b"\xff" * ((n + 7) // 8)).decode("ascii")


def baender(n: int, gitter: str, encoding: str = "q_db_0p1_i16") -> dict:
    return {
        "gitter_id": gitter,
        "encoding": encoding,
        "werte": [-123] * n,
        "gueltig_bitmap": bitmap(n),
        "saturated": False,
    }


PROBE = {
    "adresse": ADRESSE,
    "plugin_kind": "passive_probe",
    "measurement_position": "insert",
    "aussageklasse": "beobachtend",
    "label": "Klavier-Bus",
    "pair_id": None,
    "capabilities": CAPS,
    "frische": FRISCHE,
}

STEUERKOPF = {
    "command_id": "55555555555555555555555555555555",
    "ziel": ADRESSE,
    "base_revision": 12,
    "ttl_ms": 2000,
    "schema_major": 3,
    "schema_minor": 0,
}


# ------------------------------------------------------- gueltige Grundformen

GRUND: dict[str, dict] = {
    "hello_control": {
        "type": "hello",
        "connection_kind": "control",
        "protocol": 3,
        "plugin_version": "0.4.0",
        "plugin_kind": "main",
        "adresse": ADRESSE,
        "host": {"pid": 4711, "name": "FL64", "version": "21.2"},
        "audio": {"samplerate": 48000, "block_size": 512, "channels": 2},
    },
    "hello_telemetry": {
        "type": "hello",
        "connection_kind": "telemetry",
        "protocol": 3,
        "plugin_version": "0.4.0",
        "adresse": ADRESSE,
        "link_id": "66666666666666666666666666666666",
        "challenge": "77777777777777777777777777777777",
    },
    "welcome": {
        "type": "welcome",
        "protocol": 3,
        "broker_version": "0.2.0",
        "broker_epoch": "88888888888888888888888888888888",
        "link_id": "66666666666666666666666666666666",
        "challenge": "77777777777777777777777777777777",
    },
    "reject": {"type": "reject", "code": "protocol_mismatch", "reason": "protocol 2 auf v3-Pipe"},
    "heartbeat": {
        "type": "heartbeat",
        "adresse": ADRESSE,
        "sequence": 91,
        "state_revision": 12,
        "capabilities": CAPS,
        "zaehler": ZAEHLER,
        "intervention_state_unknown": False,
    },
    "heartbeat_ack": {"type": "heartbeat_ack", "sequence": 91, "duplicate_instance_id": False},
    "subscribe_session": {
        "type": "subscribe_session",
        "adresse": ADRESSE,
        "session_epoch": "22222222222222222222222222222222",
    },
    "session_snapshot": {
        "type": "session_snapshot",
        "session_epoch": "22222222222222222222222222222222",
        "broker_epoch": "88888888888888888888888888888888",
        "fuehrendes_main": "33333333333333333333333333333333",
        "beitritt_bestaetigung_noetig": False,
        "mitglieder": [PROBE],
    },
    "evidence_snapshot": {
        "type": "evidence_snapshot",
        "evidence_id": "99999999999999999999999999999999",
        "adresse": ADRESSE,
        "transport": TRANSPORT,
        "metrics_version": 1,
        "baender": baender(221, "nakama_1_24_oct_30_18k_v1"),
        "abdeckung": 0.87,
        "konvergenz": 0.5,
        "beeinflusst": False,
        "ausschlussgrund": None,
        "konfidenz": KONFIDENZ,
    },
    "audible_intervention_begin": {
        "type": "audible_intervention_begin",
        "intervention_id": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
        "adresse": ADRESSE,
        "event_sequence": 4,
        "art": "hoermarkierung",
        "project_sample_start": 44108200,
    },
    "audible_intervention_end": {
        "type": "audible_intervention_end",
        "intervention_id": "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
        "adresse": ADRESSE,
        "event_sequence": 5,
        "project_sample_end": 44120000,
        "tail_samples": 96000,
    },
    "evidence_invalidate": {
        "type": "evidence_invalidate",
        "grund": "intervention",
        "umfang": {"art": "evidence_ids", "evidence_ids": ["99999999999999999999999999999999"]},
    },
    "preview_begin": {
        "type": "preview_begin",
        "kopf": STEUERKOPF,
        "lease_duration_ms": 400,
        "renew_id": "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb",
    },
    "preview_renew": {
        "type": "preview_renew",
        "kopf": STEUERKOPF,
        "renew_id": "cccccccccccccccccccccccccccccccc",
    },
    "preview_end": {"type": "preview_end", "kopf": STEUERKOPF, "grund": "timeout"},
    "command_ack": {
        "type": "command_ack",
        "command_id": "55555555555555555555555555555555",
        "ergebnis": "angewandt",
        "state_revision": 13,
        "state_hash": "d" * 64,
    },
    "state_report": {
        "type": "state_report",
        "adresse": ADRESSE,
        "dsp_schema_version": 2,
        "state_revision": 13,
        "state_hash": "d" * 64,
        "record_state": {"valid": True, "recording": False},
        "undo_tiefe": 7,
    },
    "error": {
        "type": "error",
        "code": "revision_conflict",
        "meldung": "base_revision 12 ist aelter als 13",
        "command_id": "55555555555555555555555555555555",
        "betroffene_revision": 13,
        "rueckweg": "erneut_senden",
    },
}


# ------------------------------------------- zusaetzliche gueltige Sonderfaelle

def zusatz_gueltig() -> list[tuple[str, dict, str]]:
    """(name, daten, warum) — Faelle, die ANGENOMMEN werden muessen."""
    faelle = []

    # Additive Objekte duerfen unbekannte Eigenschaften tragen (§33.1).
    hb = copy.deepcopy(GRUND["heartbeat"])
    hb["zaehler"]["crc_fehler"] = 3
    faelle.append(("heartbeat-zaehler-additiv", hb,
                   "zaehler ist additiv: ein unbekannter Zaehler wird ignoriert, nicht abgelehnt"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["konfidenz"]["gegenprobe"] = 0.4
    faelle.append(("evidence-konfidenz-additiv", ev,
                   "konfidenz waechst laut §34.3 ausdruecklich weiter"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["verteilung"] = {
        "p10": baender(221, "nakama_1_24_oct_30_18k_v1"),
        "p50": baender(221, "nakama_1_24_oct_30_18k_v1"),
        "p95": baender(221, "nakama_1_24_oct_30_18k_v1"),
        "p99": baender(221, "nakama_1_24_oct_30_18k_v1"),
    }
    faelle.append(("evidence-verteilung-additiv", ev,
                   "ein weiteres Perzentil ist eine additive Erweiterung"))

    # Additiv genau AN der Grenze: 3 deklarierte + 8 unbekannte = 11 = maxProperties.
    hb = copy.deepcopy(GRUND["heartbeat"])
    for i in range(8):
        hb["zaehler"][f"zusatz_{i}"] = i
    faelle.append(("heartbeat-zaehler-grenze-genau", hb,
                   "genau maxProperties (11) — die Grenze selbst ist noch gueltig"))

    # Optionale Felder duerfen fehlen.
    hb = copy.deepcopy(GRUND["heartbeat"])
    del hb["intervention_state_unknown"]
    faelle.append(("heartbeat-ohne-optionales-feld", hb,
                   "intervention_state_unknown ist optional"))

    hc = copy.deepcopy(GRUND["hello_control"])
    del hc["host"]
    faelle.append(("hello-control-ohne-host", hc,
                   "host ist ein Hinweis, kein Pflichtfeld (§32.2)"))

    # Ohne gueltige Projektzeit: local_monotonic ist erlaubt, die Bits sagen es.
    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["transport"]["time_basis"] = "local_monotonic"
    del ev["transport"]["project_sample_start"]
    ev["transport"]["validity"] = {"project_time": False, "play_state": False,
                                   "record_state": False, "cycle_bounds": False}
    faelle.append(("evidence-ohne-projektzeit", ev,
                   "ohne gueltige Projektzeit traegt local_monotonic nur lokale Analyse (§32.3)"))

    # Loop-Straddle mit bewiesener Abbildung.
    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["transport"]["validity"]["cycle_bounds"] = True
    ev["transport"]["cycle"] = {
        "active": True, "bounds_valid": True,
        "start_ppq": 918.333333, "end_ppq": 928.75,
        "derived_sample_bounds": {"start": 44000000, "end": 44500000,
                                  "derivation": "validated_block_mapping"},
    }
    faelle.append(("evidence-cycle-validiert", ev,
                   "Samplegrenzen sind ein abgeleitetes Feld und tragen ihre Herleitung (§32.3)"))

    # Und ohne bewiesene Abbildung — auch das ist gueltig, nur schwaecher.
    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["transport"]["cycle"] = {"active": True, "bounds_valid": False,
                                "start_ppq": 918.333333, "end_ppq": 928.75}
    faelle.append(("evidence-cycle-nur-ppq", ev,
                   "nur PPQ-Bounds: gueltig, aber der moegliche Straddle bleibt ungeprueft"))

    # `integer` akzeptiert laut draft 2020-12 auch 1.0 — beide Engines muessen das.
    hb = copy.deepcopy(GRUND["heartbeat"])
    hb["sequence"] = 91.0
    faelle.append(("heartbeat-integer-als-1punkt0", hb,
                   "draft 2020-12: jede Zahl ohne Nachkommateil ist ein integer"))

    # Grenzwerte, die noch drin sind.
    pb = copy.deepcopy(GRUND["preview_begin"])
    pb["lease_duration_ms"] = 1
    faelle.append(("preview-lease-untergrenze", pb, "minimum 1 ist einschliesslich"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["abdeckung"] = 0.0
    ev["konvergenz"] = 1.0
    faelle.append(("evidence-abdeckung-raender", ev, "0 und 1 sind einschliesslich"))

    # Der grobe Bandsatz.
    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["baender"] = baender(64, "nakama_log64_v1", "q_db_0p01_i16")
    faelle.append(("evidence-grobes-gitter", ev, "beide Gitter sind zulaessig"))

    # Beitragsklasse mit eigenem Aux-Bus.
    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["mitglieder"][0]["measurement_position"] = "post_fader_contribution"
    ss["mitglieder"][0]["aussageklasse"] = "beitrag"
    faelle.append(("session-beitragsklasse", ss,
                   "post_fader_contribution ist die einzige Position mit Beitragsaussage (§32.2)"))

    # Leere Sitzung.
    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["mitglieder"] = []
    ss["fuehrendes_main"] = None
    faelle.append(("session-leer", ss, "minItems 0: eine Sitzung ohne Mitglieder ist gueltig"))

    # Ganze Sitzung quarantaenisieren (unbekanntes Routing).
    ei = copy.deepcopy(GRUND["evidence_invalidate"])
    ei["grund"] = "routing_unbekannt"
    ei["umfang"] = {"art": "ganze_sitzung"}
    faelle.append(("invalidate-ganze-sitzung", ei,
                   "bei unbekanntem Routing quarantaenisiert der Broker die ganze Sitzung (§34.2)"))

    return faelle


# ------------------------------------------------------ ungueltige Sonderfaelle
#
# Jede Zeile: (name, grundform, mutationen, erwartete Verletzungen, warum)
# Die Verletzungen stehen VON HAND da — siehe Kopf.

def loesche(*pfad):
    return ("loesche", list(pfad))


def setze(*pfad_und_wert):
    return ("setze", list(pfad_und_wert[:-1]), pfad_und_wert[-1])


def v(instanz: str, schema: str, schluessel: str) -> dict:
    return {"instanz": instanz, "schema": schema, "schluessel": schluessel}


UNGUELTIG: list[tuple] = [
    # --- Discriminator ---------------------------------------------------
    ("unbekannter-typ", "heartbeat", [setze("type", "gibt_es_nicht")],
     [v("/type", "#/oneOf", "oneOf")],
     "unbekannter Discriminator wird abgelehnt, nicht gegen alle Zweige gehalten"),

    ("reservierter-typ-apply", "heartbeat", [setze("type", "apply_transaction")],
     [v("/type", "#/oneOf", "oneOf")],
     "reservierter Name aus reservierte-nachrichten-v1.json — Eigentuemer SONDE-016"),

    ("reservierter-typ-telemetry", "heartbeat", [setze("type", "telemetry_frame")],
     [v("/type", "#/oneOf", "oneOf")],
     "telemetry_frame ist FlatBuffers (SONDE-005b), nie eine JSON-Familie"),

    ("typ-fehlt", "heartbeat", [loesche("type")],
     [v("/type", "#/oneOf", "oneOf")],
     "ohne Discriminator gibt es keinen Zweig"),

    ("typ-ist-zahl", "heartbeat", [setze("type", 7)],
     [v("/type", "#/oneOf", "oneOf")],
     "ein nicht-String-Discriminator waehlt keinen Zweig"),

    ("hello-ohne-connection-kind", "hello_control", [loesche("connection_kind")],
     [v("/connection_kind", f"{S}/hello/oneOf", "oneOf")],
     "der zweite Discriminator sitzt eine Ebene tiefer"),

    ("hello-fremdes-connection-kind", "hello_control", [setze("connection_kind", "sidechain")],
     [v("/connection_kind", f"{S}/hello/oneOf", "oneOf")],
     "nur control und telemetry existieren"),

    # --- Pflichtfelder ---------------------------------------------------
    ("heartbeat-ohne-adresse", "heartbeat", [loesche("adresse")],
     [v("", f"{S}/heartbeat/required/adresse", "required")],
     "ohne Adresse ist die Nachricht nicht zuzuordnen"),

    ("heartbeat-ohne-mehrere", "heartbeat", [loesche("adresse"), loesche("zaehler")],
     [v("", f"{S}/heartbeat/required/adresse", "required"),
      v("", f"{S}/heartbeat/required/zaehler", "required")],
     "je fehlendem Pflichtfeld GENAU eine Verletzung"),

    ("adresse-ohne-nonce", "heartbeat", [loesche("adresse", "runtime_nonce")],
     [v("/adresse", f"{S}/adresse/required/runtime_nonce", "required")],
     "runtime_nonce trennt alte und neue Verbindung derselben Instanz (§32.1)"),

    ("state-report-ohne-record-state", "state_report", [loesche("record_state")],
     [v("", f"{S}/state_report/required/record_state", "required")],
     "unbekannter Aufnahmezustand blockiert sichtbar (§33.4) — er darf nicht fehlen"),

    ("record-state-ohne-valid", "state_report", [loesche("record_state", "valid")],
     [v("/record_state", f"{S}/state_report/properties/record_state/required/valid", "required")],
     "valid=true ist Vorbedingung fuer Remote-Apply"),

    ("transport-ohne-validity", "evidence_snapshot", [loesche("transport", "validity")],
     [v("/transport", f"{S}/transportstempel/required/validity", "required")],
     "ein Frame ohne Gueltigkeitsbits behauptet Zeit, die er nicht hat"),

    ("validity-ohne-record-state", "evidence_snapshot",
     [loesche("transport", "validity", "record_state")],
     [v("/transport/validity", f"{S}/validity/required/record_state", "required")],
     "ein fehlendes Bit ist ein Nein, kein Vielleicht — es darf nicht weggelassen werden"),

    ("baender-ohne-encoding", "evidence_snapshot", [loesche("baender", "encoding")],
     [v("/baender", f"{S}/bandwerte/required/encoding", "required")],
     "Empfaenger raten die Skalierung nie aus dem Nachrichtentyp (§33.1)"),

    ("baender-ohne-bitmap", "evidence_snapshot", [loesche("baender", "gueltig_bitmap")],
     [v("/baender", f"{S}/bandwerte/required/gueltig_bitmap", "required")],
     "ohne Bitmap ist nicht unterscheidbar, welcher Wert gemessen wurde"),

    ("steuerkopf-ohne-ttl", "preview_begin", [loesche("kopf", "ttl_ms")],
     [v("/kopf", f"{S}/steuerkopf/required/ttl_ms", "required")],
     "eine steuernde Nachricht ohne Verfall ist ein offener Failsafe"),

    ("steuerkopf-ohne-base-revision", "preview_renew", [loesche("kopf", "base_revision")],
     [v("/kopf", f"{S}/steuerkopf/required/base_revision", "required")],
     "ohne base_revision kann die Probe keinen Konflikt erkennen (§33.3)"),

    ("capabilities-unvollstaendig", "heartbeat", [loesche("capabilities", "remote_control")],
     [v("/capabilities", f"{S}/capabilities/required/remote_control", "required")],
     "eine fehlende Capability ist keine stille Nein-Antwort"),

    # --- Capability-Riegel (§54) -----------------------------------------
    ("capability-unknown", "heartbeat", [setze("capabilities", "process_context", "unknown")],
     [v("/capabilities/process_context", f"{S}/capability_wert/enum", "enum")],
     "Entwurf §54: kein `unknown, spaeter pruefen` darf P1 passieren"),

    ("capability-vielleicht", "heartbeat",
     [setze("capabilities", "presentation_latency", "partial")],
     [v("/capabilities/presentation_latency", f"{S}/capability_wert/enum", "enum")],
     "auch kein dritter Wert unter anderem Namen"),

    ("capability-null", "heartbeat", [setze("capabilities", "double_precision", None)],
     [v("/capabilities/double_precision", f"{S}/capability_wert/enum", "enum")],
     "null ist ebenfalls kein gueltiger Capabilitywert"),

    # --- strikt vs additiv ------------------------------------------------
    ("strikt-unbekannte-eigenschaft", "heartbeat", [setze("extra_feld", 1)],
     [v("/extra_feld", f"{S}/heartbeat/additionalProperties", "additionalProperties")],
     "Nachrichtenrahmen sind strikt"),

    ("adresse-unbekannte-eigenschaft", "heartbeat", [setze("adresse", "host_pid", 4711)],
     [v("/adresse/host_pid", f"{S}/adresse/additionalProperties", "additionalProperties")],
     "eine unbekannte Eigenschaft an der Adresse waere eine unbekannte Zieladresse (§33.1)"),

    ("capabilities-unbekannte-eigenschaft", "heartbeat",
     [setze("capabilities", "midi_clock", "supported")],
     [v("/capabilities/midi_clock", f"{S}/capabilities/additionalProperties", "additionalProperties")],
     "eine unbekannte Capability ist eine unbekannte Sicherheitsaussage"),

    ("steuerkopf-unbekannte-eigenschaft", "preview_begin",
     [setze("kopf", "prioritaet", 9)],
     [v("/kopf/prioritaet", f"{S}/steuerkopf/additionalProperties", "additionalProperties")],
     "der Steuerkopf ist sicherheitsrelevant und nicht additiv"),

    ("validity-unbekanntes-bit", "evidence_snapshot",
     [setze("transport", "validity", "tempo", True)],
     [v("/transport/validity/tempo", f"{S}/validity/additionalProperties", "additionalProperties")],
     "ein neues Gueltigkeitsbit ist eine Major-Frage, keine additive"),

    # --- additiv, aber ueber der Grenze -----------------------------------
    ("zaehler-ueber-maxproperties", "heartbeat",
     [setze("zaehler", f"zusatz_{i}", i) for i in range(9)],
     [v("/zaehler", f"{S}/zaehler/maxProperties", "maxProperties")],
     "additiv heisst begrenzt erweiterbar, nicht beliebig gross (§33.1)"),

    ("konfidenz-ueber-maxproperties", "evidence_snapshot",
     [setze("konfidenz", f"zusatz_{i}", 0.5) for i in range(8)],
     [v("/konfidenz", f"{S}/konfidenz/maxProperties", "maxProperties")],
     "dieselbe Grenze am zweiten additiven Objekt"),

    # --- Typen ------------------------------------------------------------
    ("sequence-als-string", "heartbeat", [setze("sequence", "91")],
     [v("/sequence", f"{S}/heartbeat/properties/sequence/type", "type")],
     "eine Zahl als String ist keine Zahl"),

    ("sequence-mit-nachkomma", "heartbeat", [setze("sequence", 91.5)],
     [v("/sequence", f"{S}/heartbeat/properties/sequence/type", "type")],
     "integer verlangt einen leeren Nachkommateil"),

    ("adresse-als-array", "heartbeat", [setze("adresse", [1, 2])],
     [v("/adresse", f"{S}/adresse/type", "type")],
     "faellt der Typ, wird NICHT abgestiegen — genau eine Verletzung, keine Lawine"),

    ("mitglieder-als-objekt", "session_snapshot", [setze("mitglieder", {})],
     [v("/mitglieder", f"{S}/session_snapshot/properties/mitglieder/type", "type")],
     "auch am Array gilt: Typfehler beendet den Abstieg"),

    ("playing-als-zahl", "evidence_snapshot", [setze("transport", "playing", 1)],
     [v("/transport/playing", f"{S}/transportstempel/properties/playing/type", "type")],
     "1 ist in JSON kein true"),

    ("label-als-null", "session_snapshot", [setze("mitglieder", 0, "label", None)],
     [v("/mitglieder/0/label", f"{S}/probe_descriptor/properties/label/type", "type")],
     "null ist ein eigener Typ, nicht `fehlt`"),

    # --- Enums und const --------------------------------------------------
    ("time-basis-erfunden", "evidence_snapshot", [setze("transport", "time_basis", "wallclock")],
     [v("/transport/time_basis", f"{S}/transportstempel/properties/time_basis/enum", "enum")],
     "Wandzeit misst nur IPC-Latenz und darf musikalische Frames nie ausrichten (§32.3)"),

    ("plugin-kind-erfunden", "session_snapshot", [setze("mitglieder", 0, "plugin_kind", "hub")],
     [v("/mitglieder/0/plugin_kind", f"{S}/plugin_kind/enum", "enum")],
     "`hub` ist die v2-Rolle; v3 kennt main|passive_probe|active_probe|legacy (§32.2)"),

    ("aussageklasse-vermischt", "session_snapshot",
     [setze("mitglieder", 0, "aussageklasse", "beobachtend_mit_beitrag")],
     [v("/mitglieder/0/aussageklasse", f"{S}/aussageklasse/enum", "enum")],
     "die beiden Klassen duerfen nie vermischt werden (§32.2)"),

    ("gitter-erfunden", "evidence_snapshot", [setze("baender", "gitter_id", "nakama_log32_v1")],
     [v("/baender", f"{S}/bandwerte/properties/gitter_id/enum", "enum")],
     "es gibt genau zwei Bandgitter"),

    ("encoding-erfunden", "evidence_snapshot", [setze("baender", "encoding", "q_db_1_i8")],
     [v("/baender/encoding", f"{S}/band_encoding/enum", "enum")],
     "die Kodierungen stehen in quantisierung-v1.json"),

    ("protokoll-2-auf-v3", "hello_control", [setze("protocol", 2)],
     [v("/protocol", f"{S}/hello_control/properties/protocol/const", "const")],
     "protocol=2 bleibt vollstaendig im v2-Parser (§33.1)"),

    ("schema-major-4", "preview_begin", [setze("kopf", "schema_major", 4)],
     [v("/kopf/schema_major", f"{S}/steuerkopf/properties/schema_major/const", "const")],
     "unbekannte Major-Version wird abgelehnt, nie ignoriert (§33.1)"),

    ("fehlercode-erfunden", "error", [setze("code", "irgendwas_ging_schief")],
     [v("/code", f"{S}/fehlercode/enum", "enum")],
     "der Code ist maschinenlesbar; ein unbekannter faellt auf"),

    ("derivation-erfunden", "evidence_snapshot",
     [setze("transport", "cycle", {"active": True, "bounds_valid": True,
                                   "start_ppq": 1.0, "end_ppq": 2.0,
                                   "derived_sample_bounds": {"start": 1, "end": 2,
                                                             "derivation": "geschaetzt"}})],
     [v("/transport/cycle/derived_sample_bounds/derivation",
        f"{S}/derived_sample_bounds/properties/derivation/enum", "enum")],
     "abgeleitete Samplegrenzen sind validiert oder unbewiesen — nichts dazwischen"),

    # --- Zahlengrenzen ----------------------------------------------------
    ("lease-ueber-400", "preview_begin", [setze("lease_duration_ms", 401)],
     [v("/lease_duration_ms", f"{S}/preview_begin/properties/lease_duration_ms/maximum", "maximum")],
     "die Probe klemmt auf hoechstens 400 ms (§33.4) — der Vertrag laesst mehr gar nicht zu"),

    ("lease-null", "preview_begin", [setze("lease_duration_ms", 0)],
     [v("/lease_duration_ms", f"{S}/preview_begin/properties/lease_duration_ms/minimum", "minimum")],
     "eine Lease ueber 0 ms waere ein sofort abgelaufener Failsafe"),

    ("samplerate-null", "hello_control", [setze("audio", "samplerate", 0)],
     [v("/audio/samplerate", f"{S}/audio_lage/properties/samplerate/exclusiveMinimum",
        "exclusiveMinimum")],
     "sample_rate 0 ist genau der Wert, aus dem der genullte VST3-Context ein NaN macht (§32.3)"),

    ("samplerate-negativ", "evidence_snapshot", [setze("transport", "sample_rate", -48000)],
     [v("/transport/sample_rate", f"{S}/transportstempel/properties/sample_rate/exclusiveMinimum",
        "exclusiveMinimum")],
     "negative Samplerate"),

    ("abdeckung-ueber-1", "evidence_snapshot", [setze("abdeckung", 1.5)],
     [v("/abdeckung", f"{S}/evidence_snapshot/properties/abdeckung/maximum", "maximum")],
     "Abdeckung ist ein Anteil"),

    ("abdeckung-negativ", "evidence_snapshot", [setze("konvergenz", -0.1)],
     [v("/konvergenz", f"{S}/evidence_snapshot/properties/konvergenz/minimum", "minimum")],
     "Konvergenz ebenso"),

    ("sequence-negativ", "heartbeat", [setze("sequence", -1)],
     [v("/sequence", f"{S}/heartbeat/properties/sequence/minimum", "minimum")],
     "monotone Sequenzen beginnen bei 0"),

    ("tail-negativ", "audible_intervention_end", [setze("tail_samples", -1)],
     [v("/tail_samples", f"{S}/audible_intervention_end/properties/tail_samples/minimum", "minimum")],
     "ein negativer Nachlauf wuerde die Quarantaene verkuerzen"),

    ("undo-tiefe-ueber-32", "state_report", [setze("undo_tiefe", 33)],
     [v("/undo_tiefe", f"{S}/state_report/properties/undo_tiefe/maximum", "maximum")],
     "der Undo-Ring ist auf 32 atomische Zustaende begrenzt (§33.5)"),

    ("blockgroesse-null", "hello_control", [setze("audio", "block_size", 0)],
     [v("/audio/block_size", f"{S}/audio_lage/properties/block_size/minimum", "minimum")],
     "ein Block ohne Samples"),

    # --- Strings und Muster ------------------------------------------------
    ("instance-id-zu-kurz", "heartbeat", [setze("adresse", "instance_id", "abc")],
     [v("/adresse/instance_id", f"{S}/hex32/minLength", "minLength"),
      v("/adresse/instance_id", f"{S}/hex32/pattern", "pattern")],
     "Laenge und Muster sind zwei Regeln und melden zwei Verletzungen"),

    ("instance-id-grossbuchstaben", "heartbeat",
     [setze("adresse", "instance_id", "ABCDEF0123456789ABCDEF0123456789")],
     [v("/adresse/instance_id", f"{S}/hex32/pattern", "pattern")],
     "Kleinbuchstaben, damit ein Vergleich nie normalisieren muss"),

    ("instance-id-nicht-hex", "heartbeat",
     [setze("adresse", "instance_id", "z1111111111111111111111111111111")],
     [v("/adresse/instance_id", f"{S}/hex32/pattern", "pattern")],
     "z ist keine Hexziffer"),

    ("logon-sid-leer", "heartbeat", [setze("adresse", "logon_sid", "")],
     [v("/adresse/logon_sid", f"{S}/sid/minLength", "minLength")],
     "eine leere SID ist keine SID"),

    ("label-zu-lang", "session_snapshot", [setze("mitglieder", 0, "label", "x" * 121)],
     [v("/mitglieder/0/label", f"{S}/probe_descriptor/properties/label/maxLength", "maxLength")],
     "das Label ist untrusted data und begrenzt"),

    ("state-hash-zu-kurz", "state_report", [setze("state_hash", "d" * 63)],
     [v("/state_hash", f"{S}/state_report/properties/state_hash/minLength", "minLength")],
     "SHA-256 hex hat 64 Zeichen"),

    # --- Arrays -------------------------------------------------------------
    ("baender-leer", "evidence_snapshot", [setze("baender", "werte", [])],
     [v("/baender/werte", f"{S}/bandwerte/properties/werte/minItems", "minItems")],
     "ein Bandsatz ohne Werte"),

    ("baender-zu-viele", "evidence_snapshot", [setze("baender", "werte", [0] * 222)],
     [v("/baender/werte", f"{S}/bandwerte/properties/werte/maxItems", "maxItems")],
     "222 Werte passen in kein Gitter dieses Vertrags"),

    ("bandwert-als-string", "evidence_snapshot",
     [setze("baender", "werte", [-123, "-124", -125])],
     [v("/baender/werte/1", f"{S}/bandwerte/properties/werte/items/type", "type")],
     "der Index steht im Instanzpfad"),

    ("evidence-ids-leer", "evidence_invalidate", [setze("umfang", "evidence_ids", [])],
     [v("/umfang/evidence_ids",
        f"{S}/evidence_invalidate/properties/umfang/properties/evidence_ids/minItems", "minItems")],
     "eine Ruecknahme ohne Ziel"),

    ("mitglied-unvollstaendig", "session_snapshot",
     [loesche("mitglieder", 0, "capabilities")],
     [v("/mitglieder/0", f"{S}/probe_descriptor/required/capabilities", "required")],
     "Pflichtfelder gelten auch im Arrayelement"),

    # --- mehrere Verletzungen in einer Nachricht -----------------------------
    ("mehrfach-gebrochen", "heartbeat",
     [loesche("adresse"), setze("sequence", -1), setze("extra", 1),
      setze("capabilities", "process_context", "unknown")],
     [v("", f"{S}/heartbeat/required/adresse", "required"),
      v("/capabilities/process_context", f"{S}/capability_wert/enum", "enum"),
      v("/extra", f"{S}/heartbeat/additionalProperties", "additionalProperties"),
      v("/sequence", f"{S}/heartbeat/properties/sequence/minimum", "minimum")],
     "beide Engines melden ALLE Verletzungen des gewaehlten Zweiges, kanonisch sortiert"),

    # --- je Definition mindestens ein Negativfixture -------------------------
    # Diese Zeilen stehen hier, weil pruefe_v3_vertrag.py --abdeckung 14
    # Definitionen ohne Negativfixture NAMENTLICH gemeldet hat. Ohne den Riegel
    # waeren sie nie aufgefallen: alle 102 Fixtures waren gruen, und der Korpus
    # sah vollstaendig aus.

    ("welcome-ohne-broker-epoch", "welcome", [loesche("broker_epoch")],
     [v("", f"{S}/welcome/required/broker_epoch", "required")],
     "ohne broker_epoch ist eine Cache-/Replay-Grenze nicht erkennbar (§32.1)"),

    ("welcome-protokoll-2", "welcome", [setze("protocol", 2)],
     [v("/protocol", f"{S}/welcome/properties/protocol/const", "const")],
     "auch die Antwort traegt die Version fest"),

    ("reject-ohne-grund", "reject", [loesche("reason")],
     [v("", f"{S}/reject/required/reason", "required")],
     "klarer Kompatibilitaetsfehler statt stillem Scheitern heisst: mit Text"),

    ("heartbeat-ack-ohne-duplikatflag", "heartbeat_ack", [loesche("duplicate_instance_id")],
     [v("", f"{S}/heartbeat_ack/required/duplicate_instance_id", "required")],
     "Duplikate werden sichtbar aufgeloest — das Flag darf nicht fehlen (§32.1)"),

    ("subscribe-ohne-adresse", "subscribe_session", [loesche("adresse")],
     [v("", f"{S}/subscribe_session/required/adresse", "required")],
     "ein Abonnement ohne Absender"),

    ("hello-telemetry-ohne-link", "hello_telemetry", [loesche("link_id")],
     [v("", f"{S}/hello_telemetry/required/link_id", "required")],
     "ein ungekoppelter Telemetry-Connect wird geschlossen (§33.1)"),

    ("hello-telemetry-ohne-challenge", "hello_telemetry", [loesche("challenge")],
     [v("", f"{S}/hello_telemetry/required/challenge", "required")],
     "ohne Challenge ist die Kopplung nicht bewiesen"),

    ("hello-telemetry-protokoll-2", "hello_telemetry", [setze("protocol", 2)],
     [v("/protocol", f"{S}/hello_telemetry/properties/protocol/const", "const")],
     "beide Verbindungen tragen dieselbe Version"),

    ("host-ohne-pid", "hello_control", [loesche("host", "pid")],
     [v("/host", f"{S}/host_angabe/required/pid", "required")],
     "pid ist ein starkes Signal; wer host sendet, sendet sie"),

    ("messposition-erfunden", "session_snapshot",
     [setze("mitglieder", 0, "measurement_position", "send")],
     [v("/mitglieder/0/measurement_position", f"{S}/measurement_position/enum", "enum")],
     "vier Positionen, keine fuenfte (§32.2)"),

    ("frische-ohne-stale", "session_snapshot", [loesche("mitglieder", 0, "frische", "stale")],
     [v("/mitglieder/0/frische", f"{S}/frische/required/stale", "required")],
     "stale ist ein Zustand, der in JEDER UI-Fassung sichtbar sein muss (§0.4)"),

    ("cycle-ohne-active", "evidence_snapshot",
     [setze("transport", "cycle", {"bounds_valid": False})],
     [v("/transport/cycle", f"{S}/cycle/required/active", "required")],
     "ohne active ist unklar, ob ueberhaupt eine Schleife laeuft"),

    ("preview-renew-ohne-id", "preview_renew", [loesche("renew_id")],
     [v("", f"{S}/preview_renew/required/renew_id", "required")],
     "nur eine NEUE gueltige Renew-ID verlaengert die Lease (§33.4)"),

    ("preview-end-grund-erfunden", "preview_end", [setze("grund", "abgebrochen")],
     [v("/grund", f"{S}/preview_end/properties/grund/enum", "enum")],
     "die fuenf Gruende decken den Zustandsautomaten aus §33.4 ab"),

    ("command-ack-ergebnis-erfunden", "command_ack", [setze("ergebnis", "teilweise")],
     [v("/ergebnis", f"{S}/command_ack/properties/ergebnis/enum", "enum")],
     "es gibt kein halbes Apply — genau das ist der Transaktionsvertrag"),

    ("error-rueckweg-erfunden", "error", [setze("rueckweg", "ignorieren")],
     [v("/rueckweg", f"{S}/error/properties/rueckweg/enum", "enum")],
     "ein Fehler ohne benannten Rueckweg laesst den Client raten (§33.3)"),

    ("intervention-art-erfunden", "audible_intervention_begin", [setze("art", "sonstiges")],
     [v("/art", f"{S}/audible_intervention_begin/properties/art/enum", "enum")],
     "jede hoerbare Intervention gehoert einer benannten Art an (§34.2)"),

    ("invalidate-grund-erfunden", "evidence_invalidate", [setze("grund", "aufraeumen")],
     [v("/grund", f"{S}/evidence_invalidate/properties/grund/enum", "enum")],
     "eine Ruecknahme ohne benannten Grund ist nicht auditierbar"),

    ("invalidate-umfang-erfunden", "evidence_invalidate", [setze("umfang", "art", "alles_ausser")],
     [v("/umfang/art",
        f"{S}/evidence_invalidate/properties/umfang/properties/art/enum", "enum")],
     "drei Umfaenge: IDs, Bereich, ganze Sitzung"),

    ("konfidenz-klasse-erfunden", "evidence_snapshot", [setze("konfidenz", "klasse", "gut")],
     [v("/konfidenz/klasse", f"{S}/konfidenz/properties/klasse/enum", "enum")],
     "die Gesamtklasse ist ein Wort aus vier, kein gerundeter Score (§34.3)"),

    ("zaehler-ohne-pflichtzaehler", "heartbeat", [loesche("zaehler", "frames_dropped")],
     [v("/zaehler", f"{S}/zaehler/required/frames_dropped", "required")],
     "additiv heisst NICHT, dass die Pflichtzaehler entfallen duerfen"),

    ("bandwerte-saturated-als-string", "evidence_snapshot",
     [setze("baender", "saturated", "false")],
     [v("/baender/saturated", f"{S}/bandwerte/properties/saturated/type", "type")],
     "das Saettigungsbit ist ein bool"),

    ("pair-id-als-zahl", "session_snapshot", [setze("mitglieder", 0, "pair_id", 7)],
     [v("/mitglieder/0/pair_id", f"{S}/probe_descriptor/properties/pair_id/type", "type")],
     "pair_id ist String oder null"),

    ("continuity-segment-negativ", "evidence_snapshot",
     [setze("transport", "continuity_segment", -1)],
     [v("/transport/continuity_segment",
        f"{S}/transportstempel/properties/continuity_segment/minimum", "minimum")],
     "das Segment zaehlt aufwaerts, nie zurueck"),

    ("derived-bounds-ohne-ende", "evidence_snapshot",
     [setze("transport", "cycle", {"active": True, "bounds_valid": True,
                                   "derived_sample_bounds": {"start": 1,
                                                             "derivation": "validated_block_mapping"}})],
     [v("/transport/cycle/derived_sample_bounds",
        f"{S}/derived_sample_bounds/required/end", "required")],
     "eine halbe Schleifengrenze ist keine Grenze"),

    ("sid-zu-lang", "heartbeat", [setze("adresse", "logon_sid", "S-" + "1" * 200)],
     [v("/adresse/logon_sid", f"{S}/sid/maxLength", "maxLength")],
     "die SID hat eine Formatobergrenze"),

    ("audio-zu-viele-kanaele", "hello_control", [setze("audio", "channels", 65)],
     [v("/audio/channels", f"{S}/audio_lage/properties/channels/maximum", "maximum")],
     "64 Kanaele sind die Obergrenze dieses Vertrags"),

    ("mitglieder-zu-viele", "session_snapshot", [setze("mitglieder", [PROBE] * 65)],
     [v("/mitglieder", f"{S}/session_snapshot/properties/mitglieder/maxItems", "maxItems")],
     "32 Sonden sind die Auslegung; 64 die harte Grenze"),

    ("aussageklasse-fehlt", "session_snapshot", [loesche("mitglieder", 0, "aussageklasse")],
     [v("/mitglieder/0", f"{S}/probe_descriptor/required/aussageklasse", "required")],
     "ohne Aussageklasse waere eine Insert-Messung als Summenbeitrag lesbar (hartes Gate 7)"),

    # --- Wurzeltyp -----------------------------------------------------------
    ("wurzel-ist-array", "heartbeat", [("ersetze", [], [1, 2, 3])],
     [v("", "#/oneOf", "oneOf")],
     "auch die Wurzel muss ein Objekt mit Discriminator sein"),

    ("wurzel-ist-string", "heartbeat", [("ersetze", [], "heartbeat")],
     [v("", "#/oneOf", "oneOf")],
     "ein blosser Typname ist keine Nachricht"),
]


# ------------------------------------------------------------------ Mutationen

def hole(daten, pfad):
    for teil in pfad:
        daten = daten[teil]
    return daten


def wende_an(daten, mutation):
    art = mutation[0]
    if art == "ersetze":
        return mutation[2]
    pfad = mutation[1]
    if art == "loesche":
        eltern = hole(daten, pfad[:-1])
        del eltern[pfad[-1]]
    elif art == "setze":
        eltern = hole(daten, pfad[:-1])
        eltern[pfad[-1]] = mutation[2]
    else:
        raise SystemExit(f"unbekannte Mutation {art!r}")
    return daten


def kanonisch(verletzungen: list[dict]) -> list[dict]:
    return sorted(verletzungen, key=lambda x: (x["instanz"], x["schema"], x["schluessel"]))


# ------------------------------------------------------------------ Hauptlauf

def baue() -> tuple[dict, dict[str, dict]]:
    dateien: dict[str, dict] = {}
    eintraege: list[dict] = []

    for name, daten in GRUND.items():
        pfad = f"gueltig/{name}.json"
        dateien[pfad] = daten
        eintraege.append({"datei": pfad, "urteil": "gueltig",
                          "warum": "Grundform der Familie", "verletzungen": []})

    for name, daten, warum in zusatz_gueltig():
        pfad = f"gueltig/{name}.json"
        if pfad in dateien:
            raise SystemExit(f"doppelter Fixturename: {pfad}")
        dateien[pfad] = daten
        eintraege.append({"datei": pfad, "urteil": "gueltig",
                          "warum": warum, "verletzungen": []})

    for name, grund, mutationen, verletzungen, warum in UNGUELTIG:
        pfad = f"ungueltig/{name}.json"
        if pfad in dateien:
            raise SystemExit(f"doppelter Fixturename: {pfad}")
        daten = copy.deepcopy(GRUND[grund])
        for m in mutationen:
            daten = wende_an(daten, m)
        dateien[pfad] = daten
        eintraege.append({"datei": pfad, "urteil": "ungueltig", "warum": warum,
                          "verletzungen": kanonisch(verletzungen)})

    eintraege.sort(key=lambda e: e["datei"])

    manifest = {
        "$id": "evenacadia.nakama.fixtures.v3",
        "titel": "Cross-Language-Fixture-Korpus v3",
        "schema": "../../schemas/v3/eq-ipc-v3.schema.json",
        "zweck": ("Die dritte Partei zwischen C++ und Rust. Beide messen gegen DIESES "
                  "Manifest; stimmen beide mit ihm ueberein, stimmen sie transitiv "
                  "miteinander ueberein. Die Erwartungen sind von Hand geschrieben, nicht "
                  "aus einer Engine erzeugt — sonst waere der Vergleich zirkulaer."),
        "sortierung": ("Verletzungen sind kanonisch nach (instanz, schema, schluessel) "
                       "sortiert, damit der Vergleich nicht von der Auswertungsreihenfolge "
                       "abhaengt."),
        "anzahl_gueltig": sum(1 for e in eintraege if e["urteil"] == "gueltig"),
        "anzahl_ungueltig": sum(1 for e in eintraege if e["urteil"] == "ungueltig"),
        "fixtures": eintraege,
    }
    return manifest, dateien


def als_text(inhalt) -> bytes:
    return (json.dumps(inhalt, indent=2, ensure_ascii=False) + "\n").encode("utf-8")


def main(argv: list[str]) -> int:
    nur_pruefen = "--pruefen" in argv
    manifest, dateien = baue()

    print(f"{manifest['anzahl_gueltig']} gueltige, {manifest['anzahl_ungueltig']} ungueltige Fixtures")

    alle = [(ZIEL / "MANIFEST.json", manifest)]
    alle += [(ZIEL / p, d) for p, d in sorted(dateien.items())]

    if nur_pruefen:
        vorhanden = {p.relative_to(ZIEL).as_posix()
                     for p in ZIEL.rglob("*.json")} if ZIEL.exists() else set()
        erwartet = {p.relative_to(ZIEL).as_posix() for p, _ in alle}
        verwaist = sorted(vorhanden - erwartet)
        if verwaist:
            for w in verwaist:
                print(f"  ROT: verwaiste Datei {w}")
            return 2
        for pfad, inhalt in alle:
            if not pfad.exists():
                print(f"  ROT: {pfad.relative_to(WURZEL)} fehlt")
                return 3
            if pfad.read_bytes() != als_text(inhalt):
                print(f"  ROT: {pfad.relative_to(WURZEL)} weicht ab")
                return 2
        h = hashlib.sha256(als_text(manifest)).hexdigest()
        print(f"  bytegleich: {len(alle)} Dateien, MANIFEST sha256={h}")
        return 0

    for pfad, inhalt in alle:
        pfad.parent.mkdir(parents=True, exist_ok=True)
        pfad.write_bytes(als_text(inhalt))
    print(f"  geschrieben: {len(alle)} Dateien nach {ZIEL.relative_to(WURZEL)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
