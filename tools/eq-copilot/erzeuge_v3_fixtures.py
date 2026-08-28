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

# Die zehn Capabilities aus Entwurf §53.6, woertlich und vollstaendig.
# Die Werte sind Platzhalter — welche `supported` heissen, entscheidet erst der
# Capabilityreport aus S4. Die NAMEN sind Vertrag.
CAPS = {
    "host_context_presence": "supported",
    "project_time_samples": "supported",
    "sample_accurate_automation": "supported",
    "presentation_latency": "unsupported",
    "aux_compare_pre": "supported",
    "aux_priority_sidechain": "unsupported",
    "contribution_aux": "supported",
    "float64_processing": "unsupported",
    "binary_telemetry": "supported",
    "remote_control": "unsupported",
}

ZAEHLER = {"frames_dropped": 0, "parse_errors": 0, "queue_overflows": 0}

FRISCHE = {"stale": False, "letzter_kontakt_ms": 120}

VALIDITY = {
    "project_time": True,
    "play_state": True,
    "record_state": True,
    "cycle_bounds": False,
    "continuous_time": False,
    "input_presentation_latency": False,
    "output_presentation_latency": False,
}

TRANSPORT = {
    "process_context_present": True,
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
    """Alle n Baender gueltig, FUELLBITS DES LETZTEN BYTES AUF 0.

    Bei 221 Baendern traegt das 28. Byte nur 5 echte Bits. Setzte man die
    drei uebrigen mit, erzeugten zwei Sender fuer dieselbe Messung zwei
    verschiedene Bitmaps — und ein Bytevergleich waere keine Aussage mehr.
    Dieselbe Regel steht im FlatBuffers-Vertrag (SONDE-005b), wo der Leser
    sie durchsetzen kann; hier ist sie Erzeugerdisziplin, weil ein
    Base64-String im JSON-Schema nicht auf Fuellbits pruefbar ist.
    """
    bytes_ = bytearray(b"\xff" * ((n + 7) // 8))
    rest = n % 8
    if rest:
        bytes_[-1] = (1 << rest) - 1
    return base64.b64encode(bytes(bytes_)).decode("ascii")


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



# ------------------------------------------------- Textriegel-Falltabelle

# T2-Runde 2, Befund BF-5: die drei Beine trugen je eine EIGENE Kopie dieser
# Tabelle - gezaehlt 31, 32 und 33 Faelle -, waehrend das Manifest "dieselbe
# 31-Faelle-Tabelle" behauptete. Drei handgepflegte Kopien driften; genau das
# ist passiert. Die Tabelle steht deshalb jetzt EINMAL hier und wird als
# TEXTRIEGEL-FAELLE.json von allen drei Beinen GELESEN.
#
# Der Text steht HEX-KODIERT in der Datei. Das ist kein Selbstzweck: die
# Tabelle enthaelt NUL-Escapes, rohe Steuerzeichen und ein BOM - Inhalte, an
# denen ein JSON-Leser oder eine Zwischenschicht sich verschluckt. Hex ist
# exakt und in allen drei Sprachen gleich zu dekodieren.

BS = chr(92)
E = BS + "u"

TEXTRIEGEL_FAELLE: list[tuple[str, bool, str]] = [
    # --- Ganzzahlbereich ---------------------------------------------------
    ('{"w": 9007199254740991}', False, "2^53-1 ist exakt darstellbar"),
    ('{"w": 9007199254740992}', True, "eine mehr ist es nicht"),
    ('{"w": -9007199254740991}', False, "auch negativ exakt"),
    ('{"w": -9007199254740992}', True, "und eine mehr auch negativ nicht"),
    ('{"w": 18446744073709552016}', True,
     "GEMESSEN: JUCEs parseNumber liest hier 400 (int64-Ueberlauf ohne Riegel)"),
    ('{"w": 10000000000000000000}', True, "10^19"),
    ('{"w": 091}', True, "GEMESSEN: JUCE liest 91; RFC 8259 verbietet die fuehrende Null"),
    ('{"w": -091}', True, "dasselbe mit Vorzeichen"),
    ('{"w": 0}', False, "die einzelne Null ist keine fuehrende Null"),
    ('{"w": -0}', False, "negative Null ebenso"),
    ('{"w": 0.5}', False, "und als Vorkommastelle eines Bruchs"),

    # JSON Schema wertet den mathematischen Wert: auch 5.0 und 5e0 sind
    # Integer. Die sichere Grenze darf deshalb nicht an der Schreibweise
    # ohne Dezimalpunkt/Exponent haengen.
    ('{"w": 9007199254740991.0}', False, "2^53-1 auch als Dezimalform"),
    ('{"w": -9007199254740991.0}', False, "negative Dezimalgrenze"),
    ('{"w": 9007199254740991e0}', False, "2^53-1 in Exponentialform"),
    ('{"w": 90071992547409910e-1}', False, "sichere Grenze mit entfernbarer Endnull"),
    ('{"w": 0.9007199254740991e16}', False, "sichere Grenze aus einem echten Bruch"),
    ('{"w": 90071992547409.1}', False, "nichtganzzahlig mit 15 signifikanten Ziffern"),
    ('{"w": 90071992547409.1000}', False, "aequivalente Endnullen aendern die Praezision nicht"),
    ('{"w": 1.2300000000000000}', False, "Endnullen sind keine zusaetzliche Wertpraezision"),
    ('{"w": 1.23456789012345e-100}', False, "15 signifikante Ziffern mit Exponent"),
    ('{"w": 1.00000000000001}', False, "15 signifikante Ziffern an der Praezisionskante"),
    ('{"w": -0.0}', False, "negative Null als Dezimalform"),
    ('{"w": -0e0}', False, "negative Null in Exponentialform"),
    ('{"w": 9007199254740992.0}', True,
     "eine zu grosse Ganzzahl darf sich nicht hinter .0 verstecken"),
    ('{"w": -9007199254740992.0}', True, "derselbe Bypass mit negativem Vorzeichen"),
    ('{"w": 9007199254740992e0}', True, "derselbe Bypass in Exponentialform"),
    ('{"w": 90071992547409920e-1}', True, "Endnull und negativer Exponent"),
    ('{"w": -90071992547409920e-1}', True, "Endnull, Exponent und negatives Vorzeichen"),
    ('{"w": 0.9007199254740992e16}', True, "Ganzzahl aus Bruch und positivem Exponenten"),
    ('{"w": 9007199254740992e-1}', True,
     "nichtganzzahlig, aber mehr als 15 signifikante Ziffern"),
    ('{"w": 9007199254740991.1}', True,
     "GEMESSEN: Python und C++ runden auf eine Ganzzahl; Rust kann abweichend runden"),
    ('{"w": 9007199254740992.1}', True, "binary64 verliert den Nachkommateil"),
    ('{"w": 4503599627370495.9}', True, "binary64 rundet auch unterhalb 2^52 auf ganzzahlig"),
    ('{"w": 1.00000000000000001}', True, "kleiner Wert, aber Nachkommateil geht in binary64 verloren"),
    ('{"w": 1.000000000000001}', True, "16 signifikante Ziffern fallen konservativ"),

    # --- Gleitkommabereich (T2-Runde 2, BL-1/BL-2/BF-1) --------------------
    ('{"w": 1e400}', True, "als binary64 unendlich"),
    ('{"w": -1e400}', True, "auch negativ"),
    ('{"w": 1e-400}', True,
     "unterlaeuft zu 0. Der Riegel lehnt ab, damit alle drei Beine DASSELBE "
     "sagen statt drei stille Nullen zu erzeugen"),
    ('{"w": 1e307}', True,
     "zwar unter 1e308, aber mathematisch eine Ganzzahl weit oberhalb 2^53-1"),
    ('{"w": 1e308}', True, "die Grenze selbst liegt ausserhalb"),
    ('{"w": 1.5e3}', False, "gewoehnliche Exponentialform"),
    ('{"w": 4.8e4}', False, "eine legitime Samplerate in Exponentialform"),
    ('{"w": 0.0000000000000000001}', False, "klein, aber darstellbar"),
    ('{"w": 1e4294967296}', True,
     "BL-1 GEMESSEN: juce_CharacterFunctions akkumuliert den Exponenten in einem "
     "int OHNE Schranke; 4294967296 laeuft auf 0 ueber, der max_exponent10-Riegel "
     "sieht nur noch die 0, und JUCE liest 1.0 - waehrend Rust und Python inf lesen"),
    ('{"w": 1e-4294967296}', True, "dieselbe Klasse mit negativem Exponenten"),
    ('{"w": ' + "1" + "0" * 1017 + '.0}', True,
     "BL-2 GEMESSEN: 1018 Vorkommastellen. writeExponentDigits schreibt IMMER genau "
     "drei Ziffern; bei extraExponent 1000 entsteht ':00', strtod bricht dort ab und "
     "JUCE liest 1e17. Der Zweig hat ausserdem keinen max_exponent10-Riegel"),
    ('{"w": ' + "1" + "0" * 399 + '.0}', True, "400 Stellen - dort waren sich alle drei schon einig"),
    ('{"w": 1e}', True, "BF-1 GEMESSEN: getDoubleValue(\"1e\") liefert 1.0"),
    ('{"w": 1e+}', True, "dasselbe mit Vorzeichen"),
    ('{"w": 1E-}', True, "und mit grossem E"),
    ('{"w": 1.}', True, "Dezimalpunkt ohne Nachkommaziffern"),

    # --- Hex-Grammatik der \u-Escapes (BF-2/BF-3) --------------------------
    ('{"w": "' + E + '+123"}', True, "BF-2 GEMESSEN: Pythons int(roh,16) nahm das Vorzeichen"),
    ('{"w": "' + E + ' 12 "}', True, "BF-3 GEMESSEN: und Leerzeichen"),
    ('{"w": "' + E + '0x1f"}', True, "BF-3 GEMESSEN: und das 0x-Praefix"),
    ('{"w": "' + E + '1_23"}', True, "BF-3 GEMESSEN: und den Ziffern-Trenner"),
    ('{"w": "' + E + chr(0x0660) * 4 + '"}', True,
     "BF-3 GEMESSEN: und arabisch-indische Ziffern"),
    ('{"w": "' + E + '00e4"}', False, "ein echtes Escape bleibt gueltig"),
    ('{"w": "' + E + '00E4"}', False, "Grossbuchstaben-Hex ebenso"),
    ('{"w": "' + E + 'ud83d"}', True, "vier Hexziffern, nicht fuenf"),

    # --- Ziffernbegriff (BF-4) ---------------------------------------------
    ('{"w": 0' + chr(0x0662) + '}', False,
     "BF-4 GEMESSEN: Pythons str.isdigit() sah hier ZWEI Ziffern und meldete eine "
     "fuehrende Null, Rust und C++ nicht. Mit ASCII-Ziffernbegriff lesen alle drei "
     "nur die 0 - und der Parser lehnt das Dokument danach ohnehin ueberall ab"),
    ('{"w": 0' + chr(0x00B2) + '}', False, "dasselbe mit einer Hochzahl"),

    # --- Zeichenketten ------------------------------------------------------
    ('{"w": "091 nur Text"}', False, "in einer Zeichenkette gilt keine Zahlenregel"),
    ('{"w": "1e400"}', False, "auch nicht fuer Exponentialform"),
    ('{"w": "a' + E + '0000b"}', True,
     "GEMESSEN: juce::String ist nullterminiert und bricht hier im Parser ab, "
     "waehrend serde_json und Python das Dokument annehmen"),
    ('{"w": "\U0001F600"}', False, "ein rohes Astralzeichen ist unbedenklich"),
    ('{"w": "' + E + 'd83d"}', True, "einsames hohes Surrogat"),
    ('{"w": "' + E + 'de00"}', True, "einsames tiefes Surrogat"),
    ('{"w": "' + E + 'd83dx"}', True, "hohes Surrogat ohne Partner"),
    ('{"w": "' + E + 'd83d' + E + 'de00"}', False, "ein gueltiges Paar bleibt gueltig"),
    ('{"w": "er sagte ' + BS + '"hallo' + BS + '""}', False, "escapete Anfuehrungszeichen"),
    ('{"w": "backslash am Ende ' + BS + BS + '"}', False, "escapeter Backslash"),
    ('{"w": "\u00e4"}', False, "Umlaut als rohes Zeichen"),
    ('{"w": "Doppelpunkt : im Text"}', False, "loest die Schluesselregel nicht aus"),
    ('{"w": "roher Tab: \t"}', True, "rohes Steuerzeichen in einer Zeichenkette"),

    # --- Objektschluessel ---------------------------------------------------
    ('{"": 1}', True,
     "GEMESSEN: JUCE lehnt einen leeren Property-Namen im Parser ab; im ADDITIVEN "
     "zaehler haette serde_json ihn angenommen"),
    ('{"a": {"": 2}}', True, "auch verschachtelt"),
    ('{"w": ""}', False, "eine leere Zeichenkette als WERT ist in Ordnung"),
    ('{"a": "", "b": 1}', False, "auch unmittelbar vor einem Komma"),
    ('{"w" : 1}', False, "Leerzeichen vor dem Doppelpunkt sind unbedenklich"),

    # --- Dokumentrahmen (T2-Runde 2, BF-6/BF-7) -----------------------------
    ('{"w": 512, "x": [1,2,3]}', False, "eine gewoehnliche Nachricht"),

    # --- Alphabetische JSON-Literale --------------------------------------
    ('{"w": NaN}', True,
     "GEMESSEN: Pythons json.loads akzeptiert NaN, JUCE und serde_json lehnen ab"),
    ('{"w": Infinity}', True,
     "GEMESSEN: Pythons json.loads akzeptiert Infinity als nicht-endliche Zahl"),
    ('{"w": -Infinity}', True,
     "das optionale Minus gehoert zum unbekannten Literal und dessen Position"),
    ('[NaN]', True, "dieselbe Python-Erweiterung als Arrayelement"),
    ('{"w": nan}', True, "Kleinschreibung macht aus NaN kein JSON-Literal"),
    ('{"w": inf}', True, "auch die Kurzform inf ist kein JSON-Literal"),
    ('{"w": undefined}', True, "undefined ist weder JSON noch ein Vertragswert"),
    ('{"w": True}', True, "JSON-Literale sind kleingeschrieben"),
    ('{"w": "NaN"}', False, "innerhalb einer Zeichenkette ist NaN nur Text"),
    ('{"w": true}', False, "true ist ein erlaubtes JSON-Literal"),
    ('{"w": false}', False, "false ist ein erlaubtes JSON-Literal"),
    ('{"w": null}', False, "null ist ein erlaubtes JSON-Literal"),
]

# Faelle, die sich nur auf BYTE-Ebene ausdruecken lassen - sie stehen als
# rohe Bytes und nicht als Text, weil es dafuer keinen gueltigen Text gibt.
TEXTRIEGEL_BYTEFAELLE: list[tuple[bytes, bool, str]] = [
    (b"\xef\xbb\xbf" + b'{"w": 1}', True,
     "BF-7 GEMESSEN: RFC 8259 §8.1 - serde_json und Pythons json lehnen ein BOM ab, "
     "JUCEs loadFileAsString streift es und parst weiter. Der Riegel lehnt ab, damit "
     "alle drei dasselbe sehen"),
    (b'{"w": "a\xffb"}', True,
     "BF-6 GEMESSEN: kaputtes UTF-8. Das Python-Bein warf eine ungefangene "
     "UnicodeDecodeError, das Rust-Bein panickte beim Lesen, und JUCE ersetzte das "
     "Byte still - drei verschiedene Ausgaenge fuer dieselbe Datei"),
    (b'{"w":"\xc3("}', True,
     "ungueltiges Fortsetzungsbyte: JUCEs isValidString prueft dessen 10xxxxxx-Form nicht"),
    (b'{"w":"\x80"}', True, "freistehendes Fortsetzungsbyte"),
    (b'{"w":"\xc0\xaf"}', True, "overlong kodierter ASCII-Codepunkt"),
    (b'{"w":"\xed\xa0\x80"}', True, "UTF-8-kodierter Surrogat-Codepunkt"),
    (b'{"w":"\xf4\x90\x80\x80"}', True, "Codepunkt oberhalb U+10FFFF"),
    (b'{"w":"\xe2\x82"}', True, "abgeschnittene Mehrbytefolge"),
    (b'{"w": 1}\x00{"hinter_dem_nul": true}', True,
     "GEMESSEN: JUCEs UTF-8-Pruefer und Stringaufbau endeten am rohen NUL und "
     "nahmen nur den gueltigen Praefix an; Rust und Python beurteilen die ganze Bytefolge"),
    (b'{"w": 1}', False, "dieselbe Nachricht ohne BOM ist gueltig"),
]

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
        "verteilung": {
            "p10": baender(221, "nakama_1_24_oct_30_18k_v1"),
            "p50": baender(221, "nakama_1_24_oct_30_18k_v1"),
            "p95": baender(221, "nakama_1_24_oct_30_18k_v1"),
        },
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
    ev["transport"]["validity"] = {k: False for k in VALIDITY}
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

    # 28.08.2026: Die Kodierung bestimmt Typ und plausible Traegergrenzen.
    # Diese Gegenproben halten die einschliesslichen Raender und float32 als
    # endlichen Zahlenzweig offen, waehrend die Negativfixtures unten direkt
    # daneben fallen.
    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["baender"]["werte"][0] = -1440
    ev["baender"]["werte"][-1] = 240
    faelle.append(("evidence-bandwerte-q0p1-raender", ev,
                   "q_db_0p1_i16: -1440 und 240 sind einschliessliche "
                   "Traegergrenzen aus quantisierung-v1.json"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["baender"] = baender(221, "nakama_1_24_oct_30_18k_v1", "q_db_0p01_i16")
    ev["baender"]["werte"][0] = -14400
    ev["baender"]["werte"][-1] = 2400
    faelle.append(("evidence-bandwerte-q0p01-raender", ev,
                   "q_db_0p01_i16: -14400 und 2400 sind einschliessliche "
                   "Traegergrenzen aus quantisierung-v1.json"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["baender"] = baender(221, "nakama_1_24_oct_30_18k_v1", "float32")
    ev["baender"]["werte"][0] = 0.5
    faelle.append(("evidence-bandwerte-float32", ev,
                   "float32 traegt endliche JSON-Zahlen und darf Bruchteile enthalten"))

    ack = copy.deepcopy(GRUND["command_ack"])
    ack["ergebnis"] = "abgelehnt"
    del ack["state_hash"]
    ack["code"] = "revision_conflict"
    faelle.append(("command-ack-abgelehnt-ohne-state-hash", ack,
                   "eine Ablehnung bestaetigt keinen angewandten Stand und darf ohne Hash antworten"))

    ack = copy.deepcopy(GRUND["command_ack"])
    ack["ergebnis"] = "idempotent_wiederholt"
    faelle.append(("command-ack-idempotent-mit-state-hash", ack,
                   "die Wiederholung bestaetigt denselben bereits angewandten Stand samt Hash"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["mitglieder"][0]["pair_id"] = "😀" * 64
    faelle.append(("pair-id-64-codepoints", ss,
                   "pair_id zaehlt wie label Unicode-Codepoints; genau 64 sind gueltig"))

    # Der grobe Bandsatz.
    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["baender"] = baender(64, "nakama_log64_v1", "q_db_0p01_i16")
    ev["verteilung"] = {p: baender(64, "nakama_log64_v1", "q_db_0p01_i16")
                        for p in ("p10", "p50", "p95")}
    faelle.append(("evidence-grobes-gitter", ev, "beide Gitter sind zulaessig"))

    # T2-Runde 1: fuenf deklarierte Eigenschaften wurden von KEINEM Fixture
    # beruehrt - ein Vertrag, dessen Felder nie in einer Nachricht stehen, ist
    # an dieser Stelle ungeprueft, auch wenn jede Definition ein Negativfixture
    # hat. Diese beiden schliessen die Luecke.

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["transport"]["continuous_time_samples"] = 91238400
    ev["transport"]["input_presentation_latency"] = 0
    ev["transport"]["output_presentation_latency"] = 512
    ev["transport"]["validity"] = dict(VALIDITY)
    ev["transport"]["validity"]["continuous_time"] = True
    ev["transport"]["validity"]["input_presentation_latency"] = True
    ev["transport"]["validity"]["output_presentation_latency"] = True
    ev["konfidenz"] = {
        "metrics_version": 1,
        "klasse": "stark",
        "timing_alignment": 0.94,
        "messpunkt_routing": 1,
        "alternativerklaerungen": 0.2,
        "bootstrap_stabilitaet": 0.81,
    }
    faelle.append(("evidence-volle-hostzeit", ev,
                   "alle hostabhaengigen Zeitfelder MIT ihren eigenen Gueltigkeitsbits "
                   "(§32.3) und die vollstaendige Konfidenz (§34.3). Die Latenz 0 ist "
                   "hier ausdruecklich GEMESSEN und nicht 'unbekannt' - genau die "
                   "Unterscheidung, fuer die das eigene Bit existiert"))

    inv = copy.deepcopy(GRUND["evidence_invalidate"])
    inv["grund"] = "sequenzluecke"
    inv["umfang"] = {"art": "sample_range", "sample_start": 44100, "sample_end": 88200}
    faelle.append(("invalidate-bereich", inv,
                   "§34.2 verlangt Ruecknahme per ID ODER Bereich - der Bereichszweig "
                   "kam im ganzen Korpus bisher in keinem gueltigen Fixture vor"))

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

    ("state-report-ohne-state-hash", "state_report", [loesche("state_hash")],
     [v("", f"{S}/state_report/required/state_hash", "required")],
     "kein Stand wird explizit als null gemeldet; ein fehlendes Feld ist keine dritte Wahrheit"),

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
     [v("/baender/encoding", f"{S}/bandwerte_fein/oneOf", "oneOf")],
     "Empfaenger raten die Skalierung nie aus dem Nachrichtentyp (§33.1)"),

    ("baender-ohne-bitmap", "evidence_snapshot", [loesche("baender", "gueltig_bitmap")],
     [v("/baender", f"{S}/bandwerte_fein/oneOf/0/required/gueltig_bitmap", "required")],
     "ohne Bitmap ist nicht unterscheidbar, welcher Wert gemessen wurde"),

    # 28.08.2026: Apply bestaetigt einen KONKRETEN Stand. Die idempotente
    # Wiederholung bestaetigt denselben bereits angewandten Befehl und traegt
    # deshalb dieselbe Pflicht. Ablehnung ohne Hash bleibt oben positiv.
    ("command-ack-angewandt-ohne-state-hash", "command_ack", [loesche("state_hash")],
     [v("", f"{S}/command_ack/oneOf/0/required/state_hash", "required")],
     "angewandt ohne state_hash kann die bestaetigte Revision keinem konkreten "
     "Zustand zuordnen (Entwurf §33.4)"),

    ("command-ack-angewandt-state-hash-null", "command_ack", [setze("state_hash", None)],
     [v("/state_hash", f"{S}/state_hash_erfolg/type", "type")],
     "null bedeutet keinen gemeldeten Stand und ist deshalb keine Erfolgsbestaetigung"),

    ("command-ack-idempotent-ohne-state-hash", "command_ack",
     [setze("ergebnis", "idempotent_wiederholt"), loesche("state_hash")],
     [v("", f"{S}/command_ack/oneOf/4/required/state_hash", "required")],
     "idempotent_wiederholt bestaetigt denselben angewandten Stand und muss "
     "dessen Hash wiederholen"),

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
    ("capability-unknown", "heartbeat", [setze("capabilities", "host_context_presence", "unknown")],
     [v("/capabilities/host_context_presence", f"{S}/capability_wert/enum", "enum")],
     "Entwurf §54: kein `unknown, spaeter pruefen` darf P1 passieren"),

    ("capability-vielleicht", "heartbeat",
     [setze("capabilities", "presentation_latency", "partial")],
     [v("/capabilities/presentation_latency", f"{S}/capability_wert/enum", "enum")],
     "auch kein dritter Wert unter anderem Namen"),

    ("capability-null", "heartbeat", [setze("capabilities", "float64_processing", None)],
     [v("/capabilities/float64_processing", f"{S}/capability_wert/enum", "enum")],
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
     [setze("konfidenz", f"zusatz_{i}", 0.5) for i in range(12)],
     [v("/konfidenz", f"{S}/konfidenz/maxProperties", "maxProperties")],
     "dieselbe Grenze am zweiten additiven Objekt - konfidenz deklariert sechs "
     "Eigenschaften, also 6+8=14; drei stehen in der Grundform, zwoelf Zusaetze "
     "sind 15 und damit einer zu viel"),

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
     [v("/mitglieder/0/label", f"{S}/probe_label/type", "type")],
     "null ist ein eigener Typ, nicht `fehlt`"),

    # --- Enums und const --------------------------------------------------
    ("time-basis-erfunden", "evidence_snapshot", [setze("transport", "time_basis", "wallclock")],
     [v("/transport/time_basis", f"{S}/transportstempel/properties/time_basis/enum", "enum")],
     "Wandzeit misst nur IPC-Latenz und darf musikalische Frames nie ausrichten (§32.3)"),

    ("plugin-kind-erfunden", "session_snapshot", [setze("mitglieder", 0, "plugin_kind", "hub")],
     [v("/mitglieder/0/plugin_kind", f"{S}/plugin_kind/enum", "enum")],
     "`hub` ist die v2-Rolle; v3 kennt main|passive_probe|active_probe|legacy (§32.2)"),

    # Bis zum 24.08.2026 hiess dieses Fixture `aussageklasse-vermischt` und
    # versprach damit die Gate-7-Absicherung — sein Inhalt war aber ein
    # ERFUNDENES Wort, das schon jede Enumpruefung faengt. Der Gate-Lauf G1
    # (§4.1) hat den Namen als Zusage gelesen, die der Inhalt nicht haelt:
    # "Ein Fixture, dessen Name mehr zusagt als sein Inhalt haelt, ist
    # schlimmer als ein fehlendes: es macht die Luecke unsichtbar."
    # Der Name sagt jetzt, was drinsteht — die echte Vermischung ZWEIER
    # gueltiger Woerter steht in den vier `*-mit-beitragsklasse`-Fixturen
    # darunter.
    ("aussageklasse-erfunden", "session_snapshot",
     [setze("mitglieder", 0, "aussageklasse", "beobachtend_mit_beitrag")],
     [v("/mitglieder/0/aussageklasse",
        f"{S}/probe_descriptor_insert/properties/aussageklasse/const", "const")],
     "es gibt genau zwei Aussageklassen, keine dritte (§32.2)"),

    # --- Gate 7: die verbotene Paarung ZWEIER gueltiger Woerter -------------
    # Entwurf §32.2 ordnet jeder Messposition genau eine Aussageklasse zu und
    # schliesst mit "Beide Klassen duerfen in Text und Konfidenz nie vermischt
    # werden." Ohne die Kopplung im Schema validierten alle 4x2 Kombinationen —
    # auch `insert` + `beitrag`, also eine gewoehnliche Insertmessung, die sich
    # exakter Mastersummenbeitrag nennt. Das ist Gate 7 aus §49.2 im Wortlaut.
    # Diese vier Fixtures waeren VOR dem Fix vom 24.08. gruen gewesen.
    ("insert-mit-beitragsklasse", "session_snapshot",
     [setze("mitglieder", 0, "aussageklasse", "beitrag")],
     [v("/mitglieder/0/aussageklasse",
        f"{S}/probe_descriptor_insert/properties/aussageklasse/const", "const")],
     "eine Standard-Insertprobe kennt ihren exakten Beitrag zur Mastersumme "
     "NICHT und darf sich nie so nennen (Gate 7, §49.2 Nr. 7)"),

    ("pre-mit-beitragsklasse", "session_snapshot",
     [setze("mitglieder", 0, "measurement_position", "pre"),
      setze("mitglieder", 0, "aussageklasse", "beitrag")],
     [v("/mitglieder/0/aussageklasse",
        f"{S}/probe_descriptor_pre/properties/aussageklasse/const", "const")],
     "die PRE-Haelfte eines Paares ist beobachtend (§32.2)"),

    ("post-mit-beitragsklasse", "session_snapshot",
     [setze("mitglieder", 0, "measurement_position", "post"),
      setze("mitglieder", 0, "aussageklasse", "beitrag")],
     [v("/mitglieder/0/aussageklasse",
        f"{S}/probe_descriptor_post/properties/aussageklasse/const", "const")],
     "die POST-Haelfte eines Paares ist beobachtend (§32.2)"),

    # Die Gegenrichtung. Sie ist kein Gate-7-Bruch — eine Beitragsposition, die
    # sich schwaecher nennt, behauptet zu wenig statt zu viel —, aber sie macht
    # das Feld mehrdeutig: zwei Sender beschrieben dieselbe Topologie mit
    # verschiedenen Klassen. §32.2 ordnet zu, es raeumt keinen Spielraum ein.
    ("beitragsposition-beobachtend", "session_snapshot",
     [setze("mitglieder", 0, "measurement_position", "post_fader_contribution")],
     [v("/mitglieder/0/aussageklasse",
        f"{S}/probe_descriptor_beitrag/properties/aussageklasse/const", "const")],
     "post_fader_contribution ist die einzige Beitragsposition und traegt "
     "immer `beitrag` (§32.2)"),

    ("gitter-erfunden", "evidence_snapshot", [setze("baender", "gitter_id", "nakama_log32_v1")],
     [v("/baender/gitter_id", f"{S}/bandwerte/oneOf", "oneOf")],
     "es gibt genau zwei Bandgitter - und weil `bandwerte` ueber `gitter_id` "
     "diskriminiert, ist ein drittes kein enum-Fehler, sondern ein Zweig, den es nicht gibt"),

    ("encoding-erfunden", "evidence_snapshot", [setze("baender", "encoding", "q_db_1_i8")],
     [v("/baender/encoding", f"{S}/bandwerte_fein/oneOf", "oneOf")],
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
     [v("/mitglieder/0/label", f"{S}/probe_label/maxLength", "maxLength")],
     "das Label ist untrusted data und begrenzt"),

    ("pair-id-leer", "session_snapshot", [setze("mitglieder", 0, "pair_id", "")],
     [v("/mitglieder/0/pair_id", f"{S}/pair_id/minLength", "minLength")],
     "null bezeichnet bereits 'kein Paar'; die leere Zeichenkette ist keine zweite Form"),

    ("pair-id-65-codepoints", "session_snapshot",
     [setze("mitglieder", 0, "pair_id", "😀" * 65)],
     [v("/mitglieder/0/pair_id", f"{S}/pair_id/maxLength", "maxLength")],
     "v2 und v3 begrenzen pair_id auf 64 Unicode-Codepoints"),

    # Seit dem Muster (24.08.) faellt eine zu kurze Kette an ZWEI Behauptungen:
    # die Laenge stimmt nicht, und `^…{64}$` passt auf 63 Zeichen ebenfalls
    # nicht. Beide eigenen Engines melden ALLE Verletzungen des gewaehlten
    # Zweiges, also stehen beide hier.
    ("state-hash-zu-kurz", "state_report", [setze("state_hash", "d" * 63)],
     [v("/state_hash", f"{S}/state_hash/minLength", "minLength"),
      v("/state_hash", f"{S}/state_hash/pattern", "pattern")],
     "SHA-256 hex hat 64 Zeichen"),

    # G1-Befund §4.5: bis zum 24.08. stand am state_hash nur die LAENGE. 64
    # beliebige Zeichen galten als SHA-256 — ein Empfaenger, der zwei Hashes
    # vergleicht, um eine Transaktion anzunehmen (§33.5), haette einen Wert
    # verglichen, der keiner ist. Dieses Fixture waere vor dem Fix gruen
    # gewesen: die Laenge stimmt, nur das Alphabet nicht.
    ("state-hash-alphabet", "state_report", [setze("state_hash", "z" * 64)],
     [v("/state_hash", f"{S}/state_hash/pattern", "pattern")],
     "64 Zeichen sind noch kein SHA-256; `z` ist keine Hexziffer"),

    ("state-hash-grossbuchstaben", "state_report", [setze("state_hash", "D" * 64)],
     [v("/state_hash", f"{S}/state_hash/pattern", "pattern")],
     "Hex ist hier kleingeschrieben — zwei Schreibweisen desselben Hashes "
     "waeren zwei Zeichenketten und ein Vergleich waere keine Aussage mehr"),

    # 🔑 Der Grund, warum minLength/maxLength NEBEN dem Muster kein Ballast
    # sind: Pythons `re` laesst `$` auch VOR einem abschliessenden
    # Zeilenumbruch passen, die Handschleifen in C++ und Rust nicht. Beim
    # Referenzbein passt das Muster hier also, bei den beiden eigenen Engines
    # nicht — die Laengenschranke faengt den Umbruch trotzdem in ALLEN dreien,
    # deshalb ist das URTEIL identisch. Genau das misst dieses Fixture; ohne es
    # waere die Aussage eine Ueberlegung statt einer Messung.
    ("state-hash-mit-umbruch", "state_report", [setze("state_hash", "d" * 64 + "\n")],
     [v("/state_hash", f"{S}/state_hash/maxLength", "maxLength"),
      v("/state_hash", f"{S}/state_hash/pattern", "pattern")],
     "65 Zeichen: die Laengenschranke faengt den Umbruch in allen drei Beinen, "
     "bevor die Ankersemantik von `$` ueberhaupt zaehlt"),

    # --- Arrays -------------------------------------------------------------
    ("baender-leer", "evidence_snapshot", [setze("baender", "werte", [])],
     [v("/baender/werte", f"{S}/bandwerte_fein/oneOf/0/properties/werte/minItems", "minItems")],
     "ein Bandsatz ohne Werte"),

    ("baender-zu-viele", "evidence_snapshot", [setze("baender", "werte", [0] * 222)],
     [v("/baender/werte", f"{S}/bandwerte_fein/oneOf/0/properties/werte/maxItems", "maxItems")],
     "222 Werte passen in kein Gitter dieses Vertrags"),

    ("bandwert-als-string", "evidence_snapshot",
     [setze("baender", "werte", [-123] + ["-124"] + [-123] * 219)],
     [v("/baender/werte/1", f"{S}/bandwerte_fein/oneOf/0/properties/werte/items/type", "type")],
     "der Index steht im Instanzpfad"),

    # Typ und plausible Traegergrenzen folgen aus encoding, nicht bloss aus
    # der Existenz eines numerischen JSON-Werts. Je i16-Kodierung fallen
    # Bruch, Unter- und Obergrenze getrennt; float32 bleibt ein Zahlenzweig.
    ("bandwert-q0p1-nicht-ganzzahlig", "evidence_snapshot",
     [setze("baender", "werte", [0.5] + [-123] * 220)],
     [v("/baender/werte/0", f"{S}/bandwerte_fein/oneOf/0/properties/werte/items/type", "type")],
     "q_db_0p1_i16 traegt Ganzzahlen, keine zu rundenden Bruchteile"),

    ("bandwert-q0p1-unter-minimum", "evidence_snapshot",
     [setze("baender", "werte", [-1441] + [-123] * 220)],
     [v("/baender/werte/0", f"{S}/bandwerte_fein/oneOf/0/properties/werte/items/minimum", "minimum")],
     "q_db_0p1_i16 endet laut quantisierung-v1.json bei -1440"),

    ("bandwert-q0p1-ueber-maximum", "evidence_snapshot",
     [setze("baender", "werte", [241] + [-123] * 220)],
     [v("/baender/werte/0", f"{S}/bandwerte_fein/oneOf/0/properties/werte/items/maximum", "maximum")],
     "q_db_0p1_i16 endet laut quantisierung-v1.json bei 240"),

    ("bandwert-q0p01-nicht-ganzzahlig", "evidence_snapshot",
     [setze("baender", "encoding", "q_db_0p01_i16"),
      setze("baender", "werte", [0.5] + [-123] * 220)],
     [v("/baender/werte/0", f"{S}/bandwerte_fein/oneOf/1/properties/werte/items/type", "type")],
     "q_db_0p01_i16 traegt Ganzzahlen, keine zu rundenden Bruchteile"),

    ("bandwert-q0p01-unter-minimum", "evidence_snapshot",
     [setze("baender", "encoding", "q_db_0p01_i16"),
      setze("baender", "werte", [-14401] + [-123] * 220)],
     [v("/baender/werte/0", f"{S}/bandwerte_fein/oneOf/1/properties/werte/items/minimum", "minimum")],
     "q_db_0p01_i16 endet laut quantisierung-v1.json bei -14400"),

    ("bandwert-q0p01-ueber-maximum", "evidence_snapshot",
     [setze("baender", "encoding", "q_db_0p01_i16"),
      setze("baender", "werte", [2401] + [-123] * 220)],
     [v("/baender/werte/0", f"{S}/bandwerte_fein/oneOf/1/properties/werte/items/maximum", "maximum")],
     "q_db_0p01_i16 endet laut quantisierung-v1.json bei 2400"),

    ("bandwert-float32-als-string", "evidence_snapshot",
     [setze("baender", "encoding", "float32"),
      setze("baender", "werte", ["0.5"] + [-123] * 220)],
     [v("/baender/werte/0", f"{S}/bandwerte_fein/oneOf/2/properties/werte/items/type", "type")],
     "float32 traegt endliche JSON-Zahlen, keine Zahltexte"),

    ("evidence-ids-leer", "evidence_invalidate", [setze("umfang", "evidence_ids", [])],
     [v("/umfang/evidence_ids", f"{S}/invalidate_ids/properties/evidence_ids/minItems", "minItems")],
     "eine Ruecknahme ohne Ziel"),

    ("mitglied-unvollstaendig", "session_snapshot",
     [loesche("mitglieder", 0, "capabilities")],
     [v("/mitglieder/0", f"{S}/probe_descriptor_insert/required/capabilities", "required")],
     "Pflichtfelder gelten auch im Arrayelement"),

    # --- mehrere Verletzungen in einer Nachricht -----------------------------
    ("mehrfach-gebrochen", "heartbeat",
     [loesche("adresse"), setze("sequence", -1), setze("extra", 1),
      setze("capabilities", "host_context_presence", "unknown")],
     [v("", f"{S}/heartbeat/required/adresse", "required"),
      v("/capabilities/host_context_presence", f"{S}/capability_wert/enum", "enum"),
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

    # Seit dem 24.08. ist `measurement_position` der Discriminator des
    # probe_descriptor. Eine fuenfte Position waehlt damit KEINEN Zweig — die
    # Verletzung heisst `oneOf` statt `enum`, und der Vertrag lehnt sie ab,
    # statt sie gegen alle Zweige zu halten (v3-README, §33.1: unbekannter
    # Discriminator erzwingt Ablehnung).
    ("messposition-erfunden", "session_snapshot",
     [setze("mitglieder", 0, "measurement_position", "send")],
     [v("/mitglieder/0/measurement_position", f"{S}/probe_descriptor/oneOf", "oneOf")],
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
     [v("/ergebnis", f"{S}/command_ack/oneOf", "oneOf")],
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
     [v("/umfang/art", f"{S}/invalidate_umfang/oneOf", "oneOf")],
     "drei Umfaenge: IDs, Bereich, ganze Sitzung — seit der Umfang ueber `art` "
     "diskriminiert, ist ein vierter kein enum-Fehler, sondern ein fehlender Zweig"),

    ("konfidenz-klasse-erfunden", "evidence_snapshot", [setze("konfidenz", "klasse", "gut")],
     [v("/konfidenz/klasse", f"{S}/konfidenz/properties/klasse/enum", "enum")],
     "die Gesamtklasse ist ein Wort aus vier, kein gerundeter Score (§34.3)"),

    ("zaehler-ohne-pflichtzaehler", "heartbeat", [loesche("zaehler", "frames_dropped")],
     [v("/zaehler", f"{S}/zaehler/required/frames_dropped", "required")],
     "additiv heisst NICHT, dass die Pflichtzaehler entfallen duerfen"),

    ("bandwerte-saturated-als-string", "evidence_snapshot",
     [setze("baender", "saturated", "false")],
     [v("/baender/saturated", f"{S}/bandwerte_fein/oneOf/0/properties/saturated/type", "type")],
     "das Saettigungsbit ist ein bool"),

    ("pair-id-als-zahl", "session_snapshot", [setze("mitglieder", 0, "pair_id", 7)],
     [v("/mitglieder/0/pair_id", f"{S}/pair_id/type", "type")],
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
     [v("/mitglieder/0", f"{S}/probe_descriptor_insert/required/aussageklasse", "required")],
     "ohne Aussageklasse waere eine Insert-Messung als Summenbeitrag lesbar (hartes Gate 7)"),

    # --- Wurzeltyp -----------------------------------------------------------
    ("wurzel-ist-array", "heartbeat", [("ersetze", [], [1, 2, 3])],
     [v("", "#/oneOf", "oneOf")],
     "auch die Wurzel muss ein Objekt mit Discriminator sein; ist sie kein "
     "Objekt, zeigt die Verletzung auf die Instanz selbst statt auf ein /type, "
     "das es dort nicht gibt"),

    ("wurzel-ist-string", "heartbeat", [("ersetze", [], "heartbeat")],
     [v("", "#/oneOf", "oneOf")],
     "ein blosser Typname ist keine Nachricht. ACHTUNG Skalar-Wurzel: JUCEs "
     "JSON-Leser folgt RFC 4627 und verlangt { oder [ am Anfang, waehrend "
     "serde_json und Python RFC 8259 folgen und jeden Wert als Dokument "
     "annehmen. Beide Seiten LEHNEN AB, aber an verschiedenen Stellen - C++ "
     "schon im Parser, Rust erst am Schema. Die Verletzungsmenge unten gilt "
     "deshalb nur fuer die Beine mit RFC-8259-Parser."),

    # --- T2-Runde 1: die Loecher, die der Frischkontext-Pruefer gefunden hat --
    #
    # Jede dieser Zeilen belegt eine Regel, die der Vertrag VORHER nicht
    # durchgesetzt hat. Ohne sie waere jede Schemaverschaerfung oben eine
    # Behauptung: der Riegel ist erst bewiesen, wenn er an einer Eingabe faellt.

    # T-2: die Bandzahl folgt aus dem Gitter. Vorher waren 221 Werte unter dem
    # 64er-Gitter GUELTIG - die Gitter sind bitgenau eingefroren, die Nachricht
    # die ihre Werte traegt war es nicht.
    ("bandzahl-passt-nicht-zum-gitter", "evidence_snapshot",
     [setze("baender", "gitter_id", "nakama_log64_v1"),
      setze("baender", "gueltig_bitmap", bitmap(64))],
     [v("/baender/werte", f"{S}/bandwerte_grob/oneOf/0/properties/werte/maxItems", "maxItems")],
     "221 Werte unter nakama_log64_v1: das grobe Gitter hat genau 64 Gruppen"),

    ("bitmap-laenge-passt-nicht", "evidence_snapshot",
     [setze("baender", "gueltig_bitmap", bitmap(64))],
     [v("/baender/gueltig_bitmap", f"{S}/bandwerte_fein/oneOf/0/properties/gueltig_bitmap/minLength",
        "minLength"),
      v("/baender/gueltig_bitmap", f"{S}/bandwerte_fein/oneOf/0/properties/gueltig_bitmap/pattern",
        "pattern")],
     "ceil(221/8) = 28 Byte = 40 Base64-Zeichen; eine 12-Zeichen-Bitmap "
     "beschreibt 64 Baender und kann fuer 221 nicht stimmen"),

    # G1-Befund §4.5: bis zum 24.08. stand an der Bitmap nur die LAENGE. `!` mal
    # 40 galt damit als gueltige Base64-Bitmap. Ein Empfaenger, der sie
    # dekodiert, um zu wissen, WELCHE Baender gueltig sind, haette Muell
    # dekodiert — oder je nach Dekodierer etwas anderes. Diese drei Fixtures
    # waeren vor dem Fix gruen gewesen; die Laenge stimmt in allen dreien.
    # Dasselbe fuer das GROBE Gitter — 8 Byte, 12 Zeichen, EIN Fuellzeichen,
    # und ein letztes Alphabetzeichen mit zwei Fuellbits. Ohne diese Zeile
    # traegt genau eines der vier Muster kein Negativfixture; die Quote
    # `pattern 3 / 4` des Abdeckungslaufs hat es sichtbar gemacht.
    ("bitmap-grob-fuellbits-gesetzt", "evidence_snapshot",
     [setze("baender", "gitter_id", "nakama_log64_v1"),
      setze("baender", "werte", [-123] * 64),
      setze("baender", "gueltig_bitmap", "/" * 10 + "9=")],
     [v("/baender/gueltig_bitmap", f"{S}/bandwerte_grob/oneOf/0/properties/gueltig_bitmap/pattern",
        "pattern")],
     "`9` ist Index 61 (111101b): die zwei untersten Bits gehoeren zu keinem "
     "der acht Byte und muessen null sein"),

    ("bitmap-alphabet-fremd", "evidence_snapshot",
     [setze("baender", "gueltig_bitmap", "!" * 38 + "==")],
     [v("/baender/gueltig_bitmap", f"{S}/bandwerte_fein/oneOf/0/properties/gueltig_bitmap/pattern",
        "pattern")],
     "`!` steht in keinem Base64-Alphabet — 40 Zeichen sind noch keine Bitmap"),

    ("bitmap-fuellzeichen-vorn", "evidence_snapshot",
     [setze("baender", "gueltig_bitmap", "==" + "/" * 37 + "w")],
     [v("/baender/gueltig_bitmap", f"{S}/bandwerte_fein/oneOf/0/properties/gueltig_bitmap/pattern",
        "pattern")],
     "Fuellzeichen stehen am ENDE. Base64 mit `=` vorn ist keine Umkodierung "
     "derselben Bytes, sondern eine andere Zeichenkette"),

    # Die schaerfste der drei: alle 40 Zeichen sind aus dem Alphabet, beide
    # Fuellzeichen sitzen richtig — nur das letzte Alphabetzeichen traegt
    # BASE64-FUELLBITS, die nicht null sind. `x` ist Index 49 (110001b): das
    # unterste Bit gehoert zu keinem Byte. Ein Dekodierer wirft es weg, ein
    # anderer meldet einen Fehler, und zwei Sender erzeugen fuer dieselben 28
    # Byte zwei verschiedene Ketten — womit ein Bytevergleich der Bitmaps keine
    # Aussage mehr waere. Genau diese Klasse faengt die Zeichenklasse [AQgw].
    ("bitmap-fuellbits-gesetzt", "evidence_snapshot",
     [setze("baender", "gueltig_bitmap", "/" * 37 + "x==")],
     [v("/baender/gueltig_bitmap", f"{S}/bandwerte_fein/oneOf/0/properties/gueltig_bitmap/pattern",
        "pattern")],
     "das letzte Alphabetzeichen einer 40er-Kette traegt nur zwei echte Bits; "
     "die vier Fuellbits muessen null sein, sonst ist die Kodierung nicht "
     "eindeutig"),

    # T-6: §33.2 zaehlt den Inhalt des Evidenzsnapshots abschliessend auf.
    ("evidence-ohne-verteilung", "evidence_snapshot", [loesche("verteilung")],
     [v("", f"{S}/evidence_snapshot/required/verteilung", "required")],
     "P10/P50/P95 sind Inhalt des Snapshots (§33.2), keine Zugabe"),

    # T-4: §33.3 verlangt Code, betroffene Revision UND Rueckweg.
    ("error-ohne-betroffene-revision", "error", [loesche("betroffene_revision")],
     [v("", f"{S}/error/required/betroffene_revision", "required")],
     "eine fehlende Revision ist nicht dasselbe wie `null` - null heisst "
     "'keine betroffen', fehlend heisst 'vergessen'"),

    ("error-ohne-rueckweg", "error", [loesche("rueckweg")],
     [v("", f"{S}/error/required/rueckweg", "required")],
     "ein Fehler ohne Rueckweg laesst den Empfaenger raten (§33.3)"),

    # T-5: die drei neuen Gueltigkeitsbits aus §32.3.
    ("transport-ohne-context-bit", "evidence_snapshot",
     [loesche("transport", "process_context_present")],
     [v("/transport", f"{S}/transportstempel/required/process_context_present", "required")],
     "§32.3: ohne dieses Bit ist 'der Host hat GAR KEINEN ProcessContext angelegt' "
     "nicht von 'Projektzeit ist ungueltig' zu unterscheiden - zwei verschiedene "
     "Konfidenzaussagen"),

    ("validity-ohne-continuous-time", "evidence_snapshot",
     [loesche("transport", "validity", "continuous_time")],
     [v("/transport/validity", f"{S}/validity/required/continuous_time", "required")],
     "§32.3 gibt continuous_time_samples ein EIGENES Gueltigkeitsbit"),

    ("validity-ohne-latenzbit", "evidence_snapshot",
     [loesche("transport", "validity", "output_presentation_latency")],
     [v("/transport/validity", f"{S}/validity/required/output_presentation_latency", "required")],
     "§32.3: 'Ein Latenzwert 0 kann keine oder nicht bekannt bedeuten' - ohne "
     "eigenes Bit ist genau das nicht unterscheidbar"),

    # T-3: der Umfang der Invalidierung traegt jetzt, was seine Art braucht.
    ("invalidate-bereich-ohne-ende", "evidence_invalidate",
     [setze("umfang", {"art": "sample_range", "sample_start": 44100})],
     [v("/umfang", f"{S}/invalidate_bereich/required/sample_end", "required")],
     "art=sample_range ohne Bereich war vorher gueltig - ein Etikett ohne Wirkung"),

    ("invalidate-sitzung-mit-ids", "evidence_invalidate",
     [setze("umfang", {"art": "ganze_sitzung",
                       "evidence_ids": ["99999999999999999999999999999999"]})],
     [v("/umfang/evidence_ids", f"{S}/invalidate_sitzung/additionalProperties",
        "additionalProperties")],
     "die ganze Sitzung UND eine ID-Liste ist ein Widerspruch, kein Zusatz"),

    ("invalidate-ids-ohne-ids", "evidence_invalidate",
     [setze("umfang", {"art": "evidence_ids"})],
     [v("/umfang", f"{S}/invalidate_ids/required/evidence_ids", "required")],
     "art=evidence_ids ohne IDs invalidiert nichts"),

    # T-1: der Capabilitysatz ist §53.6, woertlich. Der alte Name ist jetzt
    # eine unbekannte Capability - genau die Ablehnung, die §33.1 verlangt.
    ("capability-alter-name", "heartbeat",
     [loesche("capabilities", "host_context_presence"),
      setze("capabilities", "process_context", "supported")],
     [v("/capabilities", f"{S}/capabilities/required/host_context_presence", "required"),
      v("/capabilities/process_context", f"{S}/capabilities/additionalProperties",
        "additionalProperties")],
     "`process_context` war ein selbst erfundener Name; §53.6 heisst die "
     "Capability `host_context_presence`. Ein Absender mit dem alten Namen "
     "faellt jetzt zweifach auf - fehlendes Pflichtfeld und unbekannte Eigenschaft"),

    ("capability-aux-zusammengelegt", "heartbeat",
     [loesche("capabilities", "aux_compare_pre"),
      loesche("capabilities", "aux_priority_sidechain"),
      loesche("capabilities", "contribution_aux"),
      setze("capabilities", "discrete_aux_buses", "supported")],
     [v("/capabilities", f"{S}/capabilities/required/aux_compare_pre", "required"),
      v("/capabilities", f"{S}/capabilities/required/aux_priority_sidechain", "required"),
      v("/capabilities", f"{S}/capabilities/required/contribution_aux", "required"),
      v("/capabilities/discrete_aux_buses", f"{S}/capabilities/additionalProperties",
        "additionalProperties")],
     "§53.6 gibt den drei Aux-Capabilities DREI verschiedene Fallbacks ('kein "
     "lokales Audio-Delta' / 'keine dynamische Aktuation' / 'nur Assoziation "
     "statt exakter Attribution'). Ein zusammengelegtes Bit loescht genau die "
     "Unterscheidung, fuer die sie getrennt sind"),
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

BS = chr(92)   # Backslash — als Literal frisst ihn jede Zwischenschicht


def rohtext_faelle() -> list[tuple[str, bytes, str]]:
    """Fixtures, die der TEXTRIEGEL abweisen muss — vor jedem Parser.

    Diese zwoelf lassen sich nicht ueber `json.dumps` erzeugen: eine fuehrende
    Null oder ein einsames Surrogat ist keine Ausgabe, die ein Serialisierer
    je schreiben wuerde. Sie entstehen deshalb aus einer gueltigen Grundform
    durch eine TEXTUELLE Ersetzung — so bleibt drumherum eine echte Nachricht
    stehen und der Defekt ist genau einer.

    Jeder Fall steht fuer eine in T2-Runde 1 GEMESSENE Abweichung zwischen den
    Beinen, nicht fuer eine ausgedachte.
    """
    def aus_daten(daten: dict, alt: str, neu: str) -> bytes:
        text = als_text(daten).decode("utf-8")
        if alt not in text:
            raise SystemExit(f"Rohtext-Fixture: {alt!r} steht nicht in den Daten")
        return text.replace(alt, neu, 1).encode("utf-8")

    def aus(grundform: str, alt: str, neu: str) -> bytes:
        return aus_daten(GRUND[grundform], alt, neu)

    float32 = copy.deepcopy(GRUND["evidence_snapshot"])
    float32["baender"] = baender(221, "nakama_1_24_oct_30_18k_v1", "float32")
    float32["baender"]["werte"][0] = 0.5

    return [
        ("zahl-ueber-2hoch53",
         aus("heartbeat", '"sequence": 91', '"sequence": 9007199254740992'),
         "2^53 ist die erste ganze Zahl, die binary64 nicht mehr exakt traegt"),

        ("zahl-bruch-rundet-nahe-2hoch53-ab",
         aus("heartbeat", '"sequence": 91', '"sequence": 9007199254740991.1'),
         "GEMESSEN: Python und Rust koennen aus denselben Bytes verschiedene "
         "Ganzzahlen bilden; alle drei Schema-Engines sahen vor dem Textriegel "
         "statt des mathematischen Bruchs eine zulaessige Ganzzahl"),

        ("zahl-bruch-rundet-auf-2hoch53",
         aus("heartbeat", '"sequence": 91', '"sequence": 9007199254740992.1'),
         "Der mathematische Bruch rundet in binary64 auf 2^53 und konnte so die "
         "Schema-Typpruefung fuer integer umgehen"),

        ("zahl-bruch-rundet-unter-2hoch53-auf",
         aus("heartbeat", '"sequence": 91', '"sequence": 4503599627370495.9'),
         "Der mathematische Bruch rundet in binary64 auf eine scheinbar sichere "
         "Ganzzahl unterhalb von 2^53"),

        ("zahl-kleiner-bruch-rundet-auf-eins",
         aus("heartbeat", '"sequence": 91', '"sequence": 1.00000000000000001'),
         "Ein hochpraeziser mathematischer Bruch rundet in binary64 auf 1 und "
         "konnte so die Schema-Typpruefung fuer integer umgehen"),

        ("zahl-jenseits-u64",
         aus("heartbeat", '"sequence": 91', '"sequence": 18446744073709552016'),
         "GEMESSEN: JUCEs parseNumber akkumuliert in int64 ohne Bereichspruefung "
         "und liest hier 400. Rust und Python lesen 1.8446744e19 bzw. den exakten "
         "Wert - dieselbe Datei, drei verschiedene Zahlen"),

        ("zahl-fuehrende-null",
         aus("heartbeat", '"sequence": 91', '"sequence": 091'),
         "GEMESSEN: JUCE liest 91, serde_json und Python lehnen im Parser ab. "
         "RFC 8259 verbietet die fuehrende Null"),

        ("zahl-nicht-endlich",
         aus("evidence_snapshot", '"sample_rate": 48000', '"sample_rate": 1e400'),
         "1e400 ist als binary64 unendlich; ein Vertrag traegt keine Unendlichkeit"),

        ("zahl-nan-token",
         aus_daten(float32, '"werte": [0.5,', '"werte": [NaN,'),
         "GEMESSEN: Pythons json.loads akzeptiert rohes NaN als nicht-endliche "
         "float32-Bandzahl, waehrend JUCE und serde_json schon im Parser ablehnen"),

        ("nul-escape-im-label",
         aus("session_snapshot", '"label": "Klavier-Bus"', '"label": "a' + BS + 'u0000b"'),
         "GEMESSEN: juce::String ist nullterminiert, JUCE bricht hier im Parser ab, "
         "waehrend serde_json und Python das Dokument annehmen"),

        ("einsames-surrogat-im-label",
         aus("session_snapshot", '"label": "Klavier-Bus"', '"label": "' + BS + 'ud83d"'),
         "GEMESSEN: beide eigenen Engines lehnen ab, das Referenzbein nimmt an - "
         "die umgekehrte Richtung derselben Klasse"),

        ("leerer-objektschluessel",
         aus("heartbeat", '"zaehler": {', '"zaehler": {' + chr(10) + '      "": 7,'),
         "GEMESSEN: JUCE lehnt einen leeren Property-Namen im Parser ab. Im "
         "ADDITIVEN zaehler haette serde_json ihn dagegen angenommen - genau dort, "
         "wo additionalProperties:true ihn nicht auffaengt"),
    ]



def textriegel_tabelle() -> dict:
    """Die gemeinsame Falltabelle, wie sie alle drei Beine LESEN.

    Der Text steht hex-kodiert: die Tabelle enthaelt NUL-Escapes, rohe
    Steuerzeichen, kaputtes UTF-8 und ein BOM. Ein JSON-Leser oder eine
    Zwischenschicht verschluckt sich daran; Hex ist exakt und in allen drei
    Sprachen gleich zu dekodieren. `zeigetext` ist NUR fuer Menschen da und
    wird von keinem Bein gelesen.
    """
    faelle = []
    for nr, (text, ab, warum) in enumerate(TEXTRIEGEL_FAELLE, start=1):
        roh = text.encode("utf-8")
        faelle.append({
            "nr": nr,
            "text_hex": roh.hex(),
            "zeigetext": repr(text)[1:-1][:120],
            "wird_abgelehnt": ab,
            "warum": warum,
        })
    for nr, (roh, ab, warum) in enumerate(TEXTRIEGEL_BYTEFAELLE,
                                          start=len(TEXTRIEGEL_FAELLE) + 1):
        faelle.append({
            "nr": nr,
            "text_hex": roh.hex(),
            "zeigetext": repr(roh)[2:-1][:120],
            "wird_abgelehnt": ab,
            "warum": warum,
        })
    return {
        "$id": "evenacadia.nakama.textriegel.faelle.v1",
        "titel": "Gemeinsame Falltabelle des Textriegels",
        "zweck": ("Der Textriegel ist in DREI Sprachen von Hand implementiert. Diese "
                  "Datei ist die eine Tabelle, gegen die alle drei gefahren werden. "
                  "Vorher trug jedes Bein eine eigene Kopie - gezaehlt 31, 32 und 33 "
                  "Faelle -, waehrend das Beweismanifest 'dieselbe 31-Faelle-Tabelle' "
                  "behauptete (T2-Runde 2, Befund BF-5). Drei handgepflegte Kopien "
                  "driften; eine gelesene Datei kann es nicht."),
        "warum_hex": ("Die Tabelle enthaelt NUL-Escapes, rohe Steuerzeichen, kaputtes "
                      "UTF-8 und ein BOM. Als Text waere sie von keinem JSON-Leser "
                      "unveraendert transportierbar. `zeigetext` ist nur fuer Menschen."),
        "anzahl": len(faelle),
        "faelle": faelle,
    }

def baue() -> tuple[dict, dict[str, dict], dict[str, bytes]]:
    dateien: dict[str, dict] = {}
    rohdateien: dict[str, bytes] = {}
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
        eintrag = {"datei": pfad, "urteil": "ungueltig", "warum": warum,
                   "verletzungen": kanonisch(verletzungen)}
        # Ein Dokument, dessen Wurzel KEIN Objekt und kein Array ist. Der
        # C++-Leser (JUCE, RFC 4627) lehnt es schon im Parser ab, statt eine
        # Verletzungsmenge zu bilden - die Zeile sagt das, statt es zu
        # verschweigen oder das Fixture zu entfernen.
        if not isinstance(daten, (dict, list)):
            eintrag["wurzel_skalar"] = True
        eintraege.append(eintrag)

    for name, rohtext, warum in rohtext_faelle():
        pfad = f"ungueltig/{name}.json"
        if pfad in dateien or pfad in rohdateien:
            raise SystemExit(f"doppelter Fixturename: {pfad}")
        rohdateien[pfad] = rohtext
        eintraege.append({
            "datei": pfad,
            "urteil": "ungueltig",
            "warum": warum,
            "verletzungen": [],
            # Kein Schemapfad, weil das Dokument den Parser nie erreicht. Eine
            # erfundene Verletzungsmenge waere hier eine Luege ueber den Ort,
            # an dem die Ablehnung stattfindet.
            "textriegel_lehnt_ab": True,
        })

    eintraege.sort(key=lambda e: e["datei"])

    manifest = {
        "$id": "evenacadia.nakama.fixtures.v3",
        "titel": "Cross-Language-Fixture-Korpus v3",
        "schema": "../../schemas/v3/eq-ipc-v3.schema.json",
        "zweck": ("Die dritte Partei zwischen C++ und Rust. Beide messen gegen DIESES "
                  "Manifest; stimmen beide mit ihm ueberein, stimmen sie transitiv "
                  "miteinander ueberein. Die Erwartungen sind von Hand geschrieben, nicht "
                  "aus einer Engine erzeugt — sonst waere der Vergleich zirkulaer."),
        "wurzel_skalar": ("Markiert ein Fixture, dessen Wurzel weder Objekt noch Array "
                          "ist. JUCEs JSON-Leser folgt RFC 4627 und lehnt es schon im "
                          "PARSER ab; serde_json und Python folgen RFC 8259 und lehnen es "
                          "erst am Schema ab. Beide Wege sind eine Ablehnung, nur an "
                          "verschiedenen Stellen - die Verletzungsmenge gilt daher nur fuer "
                          "die Beine mit RFC-8259-Parser."),
        "sortierung": ("Verletzungen sind kanonisch nach (instanz, schema, schluessel) "
                       "sortiert, damit der Vergleich nicht von der Auswertungsreihenfolge "
                       "abhaengt."),
        "textriegel_lehnt_ab": ("Markiert ein Fixture, das der TEXTRIEGEL abweist, "
                                "BEVOR ein Parser es sieht. Diese Fixtures tragen keine "
                                "Verletzungsmenge, weil sie das Schema nie erreichen — "
                                "eine erfundene waere eine Luege ueber den Ort der "
                                "Ablehnung. Regeln und Begruendung: schemas/v3/README.md."),
        "anzahl_gueltig": sum(1 for e in eintraege if e["urteil"] == "gueltig"),
        "anzahl_ungueltig": sum(1 for e in eintraege if e["urteil"] == "ungueltig"),
        "fixtures": eintraege,
    }
    return manifest, dateien, rohdateien


def als_text(inhalt) -> bytes:
    """Pretty-JSON, aber reine ZAHLENARRAYS auf einer Zeile.

    Warum: seit `verteilung` Pflicht ist (§33.2), traegt jeder
    Evidenzsnapshot vier 221er-Bandarrays. Mit `indent=2` sind das rund
    900 Zeilen `-123,` je Fixture und ueber den Korpus etwa 35 000 — ein
    Diff, in dem eine echte Aenderung nicht mehr zu sehen ist. Die
    Verdichtung aendert kein Byte an der geparsten Bedeutung; sie ist
    Formatierung, und `--pruefen` misst sie deterministisch mit.

    Mechanik: jedes reine Zahlenarray wird vor dem Dump durch einen
    Platzhalterstring ersetzt, der ein NUL enthaelt, und danach wieder
    eingesetzt. Ein NUL kann in keinem echten Fixturewert stehen, ohne
    dass er als komplettes Literal ersetzt wuerde — der Austausch trifft
    nur den exakten Platzhalter, nie ein Teilstueck.
    """
    platzhalter: dict[str, str] = {}

    def ersetze(o):
        if isinstance(o, list):
            if o and all(isinstance(x, (int, float)) and not isinstance(x, bool) for x in o):
                schluessel = chr(0) + f"ZAHLENARRAY{len(platzhalter)}" + chr(0)
                platzhalter[schluessel] = json.dumps(o, ensure_ascii=False)
                return schluessel
            return [ersetze(x) for x in o]
        if isinstance(o, dict):
            return {k: ersetze(v) for k, v in o.items()}
        return o

    text = json.dumps(ersetze(inhalt), indent=2, ensure_ascii=False)
    for schluessel, kompakt in platzhalter.items():
        text = text.replace(json.dumps(schluessel, ensure_ascii=False), kompakt)
    return (text + "\n").encode("utf-8")


def main(argv: list[str]) -> int:
    nur_pruefen = "--pruefen" in argv
    manifest, dateien, rohdateien = baue()

    print(f"{manifest['anzahl_gueltig']} gueltige, {manifest['anzahl_ungueltig']} ungueltige Fixtures")

    # (Pfad, Bytes) — die Rohtext-Fixtures gehen NICHT durch als_text(), sonst
    # wuerde der Serialisierer genau den Defekt wegformatieren, den sie tragen.
    alle: list[tuple[pathlib.Path, bytes]] = [
        (ZIEL / "MANIFEST.json", als_text(manifest)),
        (ZIEL / "TEXTRIEGEL-FAELLE.json", als_text(textriegel_tabelle())),
    ]
    alle += [(ZIEL / p, als_text(d)) for p, d in sorted(dateien.items())]
    alle += [(ZIEL / p, b) for p, b in sorted(rohdateien.items())]
    alle.sort(key=lambda e: e[0].as_posix())

    if nur_pruefen:
        # Nur das EIGENE Revier: gueltig/, ungueltig/ und das eigene MANIFEST.
        # Ein rglob ueber ZIEL wuerde auch fixtures/v3/flatbuffers/ einsammeln -
        # das gehoert erzeuge_fb_fixtures.py, und ein Generator, der die
        # Ausgabe eines anderen als "verwaist" meldet, ist ein Fehlalarm mit
        # rotem Kanon.
        vorhanden = set()
        if ZIEL.exists():
            for unter in ("gueltig", "ungueltig"):
                ordner = ZIEL / unter
                if ordner.is_dir():
                    vorhanden |= {q.relative_to(ZIEL).as_posix()
                                  for q in ordner.rglob("*") if q.is_file()}
            if (ZIEL / "MANIFEST.json").exists():
                vorhanden.add("MANIFEST.json")
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
            if pfad.read_bytes() != inhalt:
                print(f"  ROT: {pfad.relative_to(WURZEL)} weicht ab")
                return 2
        h = hashlib.sha256(als_text(manifest)).hexdigest()
        print(f"  bytegleich: {len(alle)} Dateien, MANIFEST sha256={h}")
        return 0

    for pfad, inhalt in alle:
        pfad.parent.mkdir(parents=True, exist_ok=True)
        pfad.write_bytes(inhalt)
    print(f"  geschrieben: {len(alle)} Dateien nach {ZIEL.relative_to(WURZEL)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
