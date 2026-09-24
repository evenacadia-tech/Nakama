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
STATE_FIXTURES = WURZEL / "eq-copilot" / "fixtures" / "state"

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

# ── SONDE-014 (P5), Fassung 3: Intent, Befund, Vorschlag, Urteil ───────────
#
# Die IDs sind bewusst NICHT die der Adresse: ein Fixture, in dem Quelle,
# Passage und Befund dieselbe Zeichenkette tragen, liesse einen vertauschten
# Feldzugriff gruen aussehen.

QUELLE_A    = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
QUELLE_B    = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
PASSAGE_S14 = "99999999999999999999999999999999"
FINDING_ID  = "0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f"
FINDING_ALT = "0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a0a"
PROPOSAL_ID = "0e0e0e0e0e0e0e0e0e0e0e0e0e0e0e0e"
STEP_ID     = "0d0d0d0d0d0d0d0d0d0d0d0d0d0d0d0d"
VERDICT_ID  = "0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c"
EVIDENZ_1   = "0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b"
EVIDENZ_2   = "0909090909090909090909090909090909"[:32]

SOURCE_INTENT = {
    "quelle_id": QUELLE_A,
    "passage_id": PASSAGE_S14,
    "rolle": "fuehrt",
    "revision": 3,
    "herkunft": "user",
    "konfidenz": 1.0,
}

INTENT_SCHUTZ = {
    "quelle_id": QUELLE_A,
    "eigenschaft": "band",
    "band": {"von": 40, "bis": 96},
}

INTENT_BEZIEHUNG = {"quelle_a": QUELLE_A, "quelle_b": QUELLE_B, "art": "fuehrt_vor"}

RANG = {
    "bandpassung": 0.9,
    "koinzidenz": 0.8,
    "uplift": 0.55,
    "intent_relevanz": 0.7,
    "wiederholbarkeit": 0.6,
    "routingqualitaet": 1.0,
}

MASKIERUNG = {
    "quelle_a": QUELLE_A,
    "quelle_b": QUELLE_B,
    "band_von": 40,
    "band_bis": 96,
    "wert_db": -3.25,
    "gueltig": True,
    "herabgesetzt": False,
}

SESSION_FINDING = {
    "finding_id": FINDING_ID,
    "claim_class": "zusammenhang",
    "ursachenklasse": "zwei_quellen_konkurrenz",
    "target_metric": "band_pegel_db",
    "candidate_source": QUELLE_A,
    "pre_post": "post",
    "passage_id": PASSAGE_S14,
    "band_hz": {"von": 40, "bis": 96},
    "beobachtung": {"wert_db": -4.5, "gueltig": True},
    "rang": RANG,
    "confidence": {"class": "mittel", "score": 0.68},
    "evidence_ids": [EVIDENZ_1, EVIDENZ_2],
    "alternatives": [FINDING_ALT],
    "ausschluesse": [{"candidate_source": QUELLE_B, "grund": "coverage_fehlt"}],
    "maskierung": MASKIERUNG,
    "next_test": "pre_post_paar_messen",
    "zustand": "ready_to_send",
    "intent_revision": 4,
    "likely_cause": "Klavierbus konkurriert im markierten Band",
    "smallest_test": "PRE/POST-Paar der Kette messen",
    "listen_for": "Refrainworte klarer, Klavier nicht duenner",
}

PROPOSAL = {
    "proposal_id": PROPOSAL_ID,
    "proposal_schema": 1,
    "target": QUELLE_A,
    "base_revision": 14,
    "passage_id": PASSAGE_S14,
    "finding_id": FINDING_ID,
    "action": "dynamic_eq_cut",
    "parameters": {"frequency_hz": 930.0, "q": 1.1, "max_gain_db": -1.5},
    "allowed_bounds": {
        "frequency_hz": {"von": 700.0, "bis": 1200.0},
        "q": {"von": 0.7, "bis": 2.0},
        "gain_db": {"von": -2.0, "bis": 0.0},
    },
    "evidence_ids": [EVIDENZ_1, EVIDENZ_2],
    "expected_effect": "reduce_masking_pressure",
    "protected_traits": [{"quelle_id": QUELLE_A, "eigenschaft": "attack"}],
    "listen_for": "Refrainworte klarer, Klavier nicht duenner",
    "stop_if": ["guardrail_geschuetzt", "keine_wiederholbare_masteraenderung"],
    "execution": "manual",
    "confidence": {"class": "mittel", "score": 0.68},
    "revert": "manual_only",
    "intent_revision": 4,
    "generatorversion": 1,
}

EXPERIMENT_ZIEL = {
    "band_von": 40,
    "band_bis": 96,
    "geschuetzte_baender": [{"von": 4, "bis": 12}],
    "proposal_id": PROPOSAL_ID,
}



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


FEIN = "nakama_1_24_oct_30_18k_v1"


def stereo_band(wert: float) -> dict:
    """Ein 221er-Bandsatz der SONDE-013-Stereoevidenz.

    Ohne `saturated`: die Saettigungsmarke gehoert zur Quantisierung, und
    diese Bandsaetze sind float32. Eine Marke mitzufuehren, die nie true
    werden kann, waere ein totes Feld auf der Leitung.
    """
    return {
        "gitter_id": FEIN,
        "encoding": "float32",
        "werte": [wert] * 221,
        "gueltig_bitmap": bitmap(221),
    }


STEREO = {
    "fenster_dauer_ms": [400.0] * 221,
    "freiheitsgrade": [12] * 221,
    "mid_db": stereo_band(-18.5),
    "side_db": stereo_band(-27.25),
    "seitenanteil_db": stereo_band(-8.75),
    "korrelation_kurz": stereo_band(0.82),
    "korrelation_mittel": stereo_band(0.79),
    "kohaerenz": stereo_band(0.91),
    "phase_rad": stereo_band(0.05),
    "persistenz": stereo_band(0.66),
    "zeitperzentile": {
        "p10": stereo_band(-12.5),
        "p50": stereo_band(-8.75),
        "p95": stereo_band(-4.25),
    },
    "mono_folddown_db": -1.75,
    "lr_balance_db": 0.25,
}

EREIGNIS = {
    "sample_offset": 12288,
    "staerke_mad": 3.5,
    "band_zentrum_hz": 1250.0,
    "dauer_samples": 480,
    "qualitaet_fluss": True,
    "qualitaet_peak": False,
}

# Zweites Ereignis aus dem EIGENSTAENDIGEN Peakpfad (M-86): ein sehr kurzer
# Impuls, den der spektrale Fluss nicht erreicht. Es traegt genau das
# umgekehrte Bitpaar - waeren beide Faelle im Korpus gleich, koennte kein
# Bein die Trennung der zwei Ausloeser messen.
EREIGNIS_PEAK = {
    "sample_offset": 20480,
    "staerke_mad": 1.25,
    "band_zentrum_hz": 6300.0,
    "dauer_samples": 64,
    "qualitaet_fluss": False,
    "qualitaet_peak": True,
}

FINGERPRINT = {
    "version": 1,
    "band_energie": [(7 * i) % 256 for i in range(32)],
    "chroma": [(21 * i) % 256 for i in range(12)],
    "onset": [(13 * i) % 256 for i in range(32)],
}

FINGERPRINT_UPSTREAM = {
    "version": 1,
    "band_energie": [(11 * i) % 256 for i in range(32)],
    "chroma": [(5 * i) % 256 for i in range(12)],
    "onset": [(3 * i) % 256 for i in range(32)],
}

EXPERIMENT_REFERENZ = {
    "passage_fingerprint": FINGERPRINT,
    "upstream_fingerprint": FINGERPRINT_UPSTREAM,
    "aktive_quellen": [
        "33333333333333333333333333333333",
        "44444444444444444444444444444444",
    ],
    "messpunktklassen": ["insert", "post"],
    "match_gain_db": -1.5,
    "alignment": "feature_aligned",
}

# SONDE-013 Nacharbeit 1 (Befund B23): die vollstaendige Passage aus M-25.
# Sie steht als eigene Konstante, weil die Negativfaelle sie einzeln
# verderben - eine im Fall eingebettete Kopie liefe von ihr weg.
PASSAGE = {
    "passage_id": "cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd",
    "projekt_von": 480000,
    "projekt_bis": 960000,
    "transport_epoch": 3,
    "aktive_quellen": [
        "33333333333333333333333333333333",
        "44444444444444444444444444444444",
    ],
    "messpunktklassen": ["insert", "post"],
    "abdeckung": 0.91,
    "label": "Refrain 2",
    # Eine EIGENE Kopie, nicht dieselbe Instanz wie in EXPERIMENT_REFERENZ:
    # `copy.deepcopy` erhaelt die Aliasbeziehungen INNERHALB der kopierten
    # Struktur, und eine Mutation an `referenz.passage_fingerprint` traefe
    # sonst auch `passage.fingerprint`. Jeder Negativfall soll genau EIN Feld
    # verderben. Die WERTE sind dieselben - in einer echten Nachricht
    # beschreiben beide dieselbe Passage.
    "fingerprint": copy.deepcopy(FINGERPRINT),
}


# SONDE-013 Nacharbeit 2 (Befunde R14/R32): der Rueckweg zu Gen. Beide Listen
# reisen im `session_snapshot` — eine eigene Familie fuer das Ergebnis waere
# ein zweiter Weg fuer dieselbe Aussage gewesen (§53.9).
SESSION_VERSUCH = {
    "experiment_id": "abababababababababababababababab",
    "ereignis": "ergebnis",
    "offen": False,
    "hoerurteil": "kandidat",
    "blindreihenfolge": "kandidat_zuerst",
    "vergleichbarkeit": "stark",
    "urteil": "ziel_verbessert_guardrails_stabil",
}

SESSION_PAARE = [
    {"pair_id": "paar-bus-a", "klasse": "probable", "kettenbefund": "stationaer"},
    # Ein ausgeschlossenes Paar traegt seinen Grund. Ohne ihn waere es
    # unsichtbar verschwunden — ein stiller Ausschluss (M-23).
    {
        "pair_id": "paar-bus-b",
        "klasse": "unclear",
        "kettenbefund": "nicht_beurteilbar",
        "ausschluss": "haelfte_fehlt",
    },
]


PROBE = {
    "adresse": ADRESSE,
    "plugin_kind": "passive_probe",
    "measurement_position": "insert",
    "aussageklasse": "beobachtend",
    "betrieb": "active",
    "label": "Klavier-Bus",
    "pair_id": None,
    "capabilities": CAPS,
    "frische": FRISCHE,
}


def minor_1_sessionform(daten: dict) -> dict:
    """Hebt die historische Descriptor-Liste in die Minor-1-Mitgliedshuelle.

    Die vielen handgeschriebenen Mutationen unten bleiben dadurch auf den
    fachlichen Descriptorfeldern lesbar. Auf der Leitung liegt trotzdem nur
    die neue Form; der Helfer ist idempotent fuer explizite B2b-Fixtures.
    """
    if not isinstance(daten, dict) or daten.get("type") != "session_snapshot":
        return daten
    neu = []
    if not isinstance(daten.get("mitglieder"), list):
        return daten
    for eintrag in daten.get("mitglieder", []):
        if "probe_descriptor" in eintrag or (
            set(eintrag) <= {"adresse", "plugin_kind", "frische", "p2_reject"}
            and "measurement_position" not in eintrag
        ):
            neu.append(eintrag)
            continue
        descriptor = copy.deepcopy(eintrag)
        p2_reject = descriptor.pop("p2_reject", None)
        mitglied = {
            "adresse": copy.deepcopy(descriptor["adresse"]),
            "plugin_kind": descriptor["plugin_kind"],
            "frische": copy.deepcopy(descriptor["frische"]),
            "probe_descriptor": descriptor,
        }
        if p2_reject is not None:
            mitglied["p2_reject"] = p2_reject
        neu.append(mitglied)
    daten["mitglieder"] = neu
    return daten

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

    # --- Untergrenze des Betrags (NAK-313 M-121, R-313-7) -------------------
    # Regel 3 misst am Literal: die fuehrende signifikante Ziffer steht bei
    # einem Dezimalexponenten zwischen -308 und 308 (ausschliesslich). Die drei
    # Riegel rechnen die Untergrenze seit jeher; die Tabelle trug bis Etappe 6
    # nur die Obergrenze (1e307, 1e308). Angehaengt, damit die Nummern der
    # bestehenden Textfaelle bleiben.
    ('{"w": 1e-307}', False, "die Untergrenze selbst: Dezimalexponent -307 liegt im Bereich"),
    ('{"w": 1e-308}', True, "Dezimalexponent -308 liegt ausserhalb - die Kante der Untergrenze"),
    ('{"w": 2e-308}', True,
     "dieselbe Kante mit anderer fuehrender Ziffer: es entscheidet der Exponent"),
    ('{"w": 9.9e-308}', True,
     "knapp unter 1e-307 und ausserhalb: gemessen am Literal, nicht am double"),
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
    "session_command": {
        "type": "session_command",
        "command": "confirm_join",
        "command_id": "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee",
        "ziel": ADRESSE,
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
    "intent_update": {
        "type": "intent_update",
        "adresse": ADRESSE,
        "session_epoch": "22222222222222222222222222222222",
        "bestand_revision": 4,
        "vollstaendig": True,
        "intents": [SOURCE_INTENT],
        "schutzangaben": [INTENT_SCHUTZ],
        "beziehungen": [INTENT_BEZIEHUNG],
    },
    "assistant_step_update": {
        "type": "assistant_step_update",
        "adresse": ADRESSE,
        "session_epoch": "22222222222222222222222222222222",
        "step_id": STEP_ID,
        "schritt": "proposal",
        "revision": 3,
        "offen": True,
        "finding_id": FINDING_ID,
        "proposal_id": PROPOSAL_ID,
    },
    "draft_offer": {
        "type": "draft_offer",
        "kopf": STEUERKOPF,
        "proposal": PROPOSAL,
    },
    "user_verdict": {
        "type": "user_verdict",
        "kopf": STEUERKOPF,
        "user_verdict_id": VERDICT_ID,
        "finding_id": FINDING_ID,
        "proposal_id": PROPOSAL_ID,
        "urteil": "angenommen",
        "notiz": "klingt offener, Klavier bleibt",
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

    "experiment_begin": {
        "type": "experiment_begin",
        "kopf": STEUERKOPF,
        "experiment_id": "abababababababababababababababab",
        "execution_mode": "manual_external",
        "reproduzierbarkeit": "manuell_nicht_wiederherstellbar",
        "passage": PASSAGE,
        "referenz": EXPERIMENT_REFERENZ,
    },
    # SONDE-013 Nacharbeit 2 (Befunde R16/R21): der Schritt VOR dem Urteil.
    # Er erfasst den Kandidaten und bindet die Blindreihenfolge, bevor jemand
    # hoert — danach ist sie nicht mehr frei waehlbar (M-41/M-44).
    "experiment_candidate": {
        "type": "experiment_candidate",
        "kopf": {**STEUERKOPF, "command_id": "66666666666666666666666666666666"},
        "experiment_id": "abababababababababababababababab",
        "referenz": EXPERIMENT_REFERENZ,
        "blindreihenfolge": "kandidat_zuerst",
    },
    "experiment_abort": {
        "type": "experiment_abort",
        "kopf": STEUERKOPF,
        "experiment_id": "abababababababababababababababab",
        "grund": "user_abbruch",
    },
    "experiment_manual_result": {
        "type": "experiment_manual_result",
        "kopf": STEUERKOPF,
        "experiment_id": "abababababababababababababababab",
        "hoerurteil": "kandidat",
        "blindreihenfolge": "baseline_zuerst",
        "notiz": "Saettigung im fremden Werkzeug leicht erhoeht",
        "werkzeug": "Fremd-Saturator 2.1",
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

def dsp_nutzlast(datei: str = "gemischt") -> tuple[str, str]:
    """(kanonischer DTO-Text, sein SHA-256) - aus dem STATE-Korpus.

    Der Text wird NICHT hier gebaut: er kommt aus derselben Datei, an der
    C++, Rust und Python ihren `state_hash` messen (SONDE-006). Ein zweiter,
    hier gebauter DTO waere eine zweite Wahrheit ueber denselben Text - und
    genau die Verbindung zwischen `dsp.jcs` und `state_hash` soll das Fixture
    belegen (M-105).
    """
    import rfc8785  # nur der Erzeuger braucht ihn, nicht der Pruefer im Feld

    roh = (STATE_FIXTURES / f"dto/gueltig/{datei}.json").read_text(encoding="utf-8")
    kanon = rfc8785.dumps(json.loads(roh))
    return kanon.decode("utf-8"), hashlib.sha256(kanon).hexdigest()


def dto_grund(text: str) -> str | None:
    """Der Grund des DTO-Wegs fuer eine Zeichenkette, oder None (gueltig).

    Kein zweiter Validator: das ist derselbe Referenzvalidator, den A12 gegen
    C++ und Rust haelt (`erzeuge_state_fixtures.validiere_dto_python`). Er wird
    hier gebraucht, weil ein Fixture, das AUSSCHLIESSLICH die Hashpruefung
    messen soll, den DTO-Weg noch passieren muss - sonst faellt der Rotbeweis
    an einem anderen Grund (SONDE-015 Etappe 2, Nacharbeit 1, Befund B-04).
    """
    import erzeuge_state_fixtures as sf   # zieht rfc8785 nach, wie dsp_nutzlast

    vertrag = json.loads(sf.VERTRAG.read_text(encoding="utf-8"))
    return sf.validiere_dto_python(vertrag, text)


def dsp_bericht(datei: str = "gemischt") -> dict:
    jcs, _ = dsp_nutzlast(datei)
    return {
        "jcs": jcs,
        "auto_gain_db": -1.5,
        "klemmungen": [
            {"id": "v1.band.2.sidechain_source",
             "gemeldet": "priority_sidechain",
             "wirksam": "internal"}
        ],
        "verletzte_baender": [0, 5],
    }


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

    # SONDE-013 Nacharbeit 1: die fuenf Produktcodes der Experimentfamilien.
    # Jeder benennt eine Regel aus der Matrix; ein `abgelehnt` OHNE Grund
    # liesse den Sender raten, welche.
    for code, warum in (
        ("abdeckung_zu_gering", "M-30: eine Passage ohne genug gemessenes Signal traegt keinen Versuch"),
        ("schon_terminal", "M-47: append-only - ein zweites Terminalereignis wuerde das erste umdeuten"),
        ("ohne_lautheitsabgleich", "M-43/§15: eine Klangwertung ohne vorherigen Lautheitsabgleich ist unzulaessig"),
        ("ohne_resultatmessung", "M-45: ein Urteil ohne Gegenprobe ist kein Ergebnis"),
        ("blindreihenfolge_widerspruch", "M-44: die Reihenfolge laesst sich nicht nachtraeglich zum Urteil passend erzaehlen"),
    ):
        ack = copy.deepcopy(GRUND["command_ack"])
        ack["ergebnis"] = "abgelehnt"
        del ack["state_hash"]
        ack["code"] = code
        faelle.append((f"command-ack-{code.replace('_', '-')}", ack, warum))

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
    ev["transport"]["cycle"] = {
        "active": True, "bounds_valid": False,
        "start_ppq": 918.0, "end_ppq": 920.0,
    }
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

    # NAK-29: jede bedingte Transportaussage mit ihrem positiven Gegenstueck.
    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    faelle.append(("transport-project-samples", ev,
                   "project_samples mit project_time=true und project_sample_start"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["transport"]["time_basis"] = "local_monotonic"
    del ev["transport"]["project_sample_start"]
    ev["transport"]["validity"]["project_time"] = False
    faelle.append(("transport-local-monotonic", ev,
                   "local_monotonic ohne Projektposition und mit project_time=false"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["transport"]["validity"]["cycle_bounds"] = True
    ev["transport"]["cycle"] = {
        "active": False, "bounds_valid": False, "start_ppq": 5.0, "end_ppq": 6.0,
    }
    faelle.append(("validity-cycle-bounds-mit-ppq", ev,
                   "cycle_bounds=true traegt beide rohen PPQ-Grenzen"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["transport"]["validity"]["continuous_time"] = True
    ev["transport"]["continuous_time_samples"] = 91238400
    faelle.append(("validity-continuous-time-mit-wert", ev,
                   "continuous_time=true traegt den zugehoerigen Samplewert"))

    # NAK-114: die bislang fehlenden positiven Klassen-/Positionskombinationen.
    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["mitglieder"][0]["plugin_kind"] = "main"
    faelle.append(("session-main-insert", ss, "main darf die Insertposition fuehren"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["mitglieder"][0]["plugin_kind"] = "active_probe"
    ss["mitglieder"][0]["measurement_position"] = "pre"
    faelle.append(("session-active-probe-pre", ss, "active_probe darf PRE fuehren"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["mitglieder"][0]["plugin_kind"] = "legacy"
    ss["mitglieder"][0]["measurement_position"] = "post"
    faelle.append(("session-legacy-post", ss, "legacy darf POST fuehren"))

    # SONDE-012 E-H02/H06: beide Grenzen und ein bewusst nicht normalisierter
    # Name. Das Grundfixture ohne beide Felder bleibt der Positivbeweis fuer
    # ihre Optionalitaet.
    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["mitglieder"][0]["host_bus_name"] = "K"
    ss["mitglieder"][0]["host_mixer_index"] = 1
    faelle.append(("probe-host-context-minimum", ss,
                   "ein Codepoint und VST3-Index 1 sind die einschliesslichen Untergrenzen"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["mitglieder"][0]["host_bus_name"] = "😀" * 120
    ss["mitglieder"][0]["host_mixer_index"] = 9_007_199_254_740_991
    faelle.append(("probe-host-context-maximum", ss,
                   "120 Unicode-Codepoints und 2^53-1 sind die einschliesslichen Obergrenzen"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["mitglieder"][0]["host_bus_name"] = "  MiXeD Bus  "
    faelle.append(("probe-host-bus-name-codepointgetreu", ss,
                   "angenommener Hosttext bleibt mit inneren und aeusseren Leerzeichen sowie Case unveraendert"))

    hb = copy.deepcopy(GRUND["heartbeat"])
    hb["runtime"] = {
        "messpunkt": "pre",
        "betrieb": "suspended",
        "host_bus_name": "Bus A",
        "host_mixer_index": 1,
    }
    faelle.append(("heartbeat-runtime-vollstaendig", hb,
                   "E-M01: ein vollstaendiger strikter Runtime-Block reist im Heartbeat"))

    hb = copy.deepcopy(GRUND["heartbeat"])
    hb["runtime"] = {
        "messpunkt": "insert",
        "betrieb": "active",
        "label": "😀" * 120,
    }
    faelle.append(("heartbeat-runtime-label", hb,
                   "E-D2: das gespeicherte User-Label reist mit exakt der probe_label-Codepointgrenze"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["mitglieder"][0]["p2_reject"] = {
        "grund": "feature_batch_ungueltig", "zaehler": 1,
    }
    faelle.append(("session-p2-reject", ss,
                   "E-L14: geschlossener Ablehnungsgrund und Zaehler ab 1 reisen je Quelle"))

    ss = minor_1_sessionform(copy.deepcopy(GRUND["session_snapshot"]))
    del ss["mitglieder"][0]["probe_descriptor"]
    faelle.append(("session-mitglied-unclassified", ss,
                   "E-M01/L23: Mitglied ohne gemeldeten Messpunkt bleibt ohne Descriptor sichtbar"))

    command = copy.deepcopy(GRUND["session_command"])
    command["command"] = "unbind_probe"
    faelle.append(("session-command-unbind", command,
                   "E-L18: unbind_probe ist der zweite und einzige Gegenpfad im geschlossenen Zweigsatz"))

    command = copy.deepcopy(GRUND["session_command"])
    faelle.append(("session-command-fremdes-main-vertragsform", command,
                   "Die JSON-Form kennt keinen selbst behaupteten Sender; ob der sendende Link das fuehrende Main ist, entscheidet ausschliesslich der Broker"))

    command = copy.deepcopy(GRUND["session_command"])
    command["session_epoch"] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
    faelle.append(("session-command-falsche-epoche-vertragsform", command,
                   "Beide Epochen sind einzeln hex32; ihre fail-closed Gleichheitsrelation zum Ziel und Sender-Link ist eine Brokerregel und kein mit draft 2020-12 ausdrueckbarer JSON-Typ"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["store_degraded"] = True
    faelle.append(("session-snapshot-store-degraded", ss,
                   "E-L15-Store: nur der positive Diagnosebefund true reist"))

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

    # --- SONDE-013, Wire-Envelope-schema_minor 2 -------------------------

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["ereignisse"] = {"liste": [EREIGNIS, EREIGNIS_PEAK], "verloren": 0}
    ev["stereo"] = copy.deepcopy(STEREO)
    ev["konfidenz"]["verteilung_fenster"] = 41
    # Nacharbeit 1 (Befund B07): der Zaehler der nicht-endlichen
    # Eingangssamples reist im additiven `konfidenz`-Objekt mit. 0 heisst
    # nachweislich keines - deshalb steht er auch im GUTFALL.
    ev["konfidenz"]["samples_nicht_endlich"] = 0
    faelle.append(("evidence-snapshot-mit-ereignissen-und-stereo", ev,
                   "M-05/M-11: der Ereignisstrom und die bandweise Stereoevidenz reisen auf dem Evidenzpfad, nicht im 10-Hz-Liveframe"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["konfidenz"]["samples_nicht_endlich"] = 3
    ev["konfidenz"]["klasse"] = "unbrauchbar"
    faelle.append(("evidence-snapshot-samples-nicht-endlich", ev,
                   "M-07: ein Beleg, dessen Fenster nicht-endliche Eingangssamples "
                   "gesehen hat, ZAEHLT sie und traegt die Klasse `unbrauchbar` - "
                   "nicht `schwach`: die Zahl beschreibt Stille, nicht Musik"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["ereignisse"] = {"liste": [], "verloren": 7}
    faelle.append(("evidence-ereignisse-leer-mit-verlust", ev,
                   "M-05: ein leerer Ring mit Verlustzaehler ist gueltig — der Ring wird bei Ueberlast nie stillschweigend geleert"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["ereignisse"] = {"liste": [copy.deepcopy(EREIGNIS) for _ in range(64)],
                        "verloren": 0}
    faelle.append(("evidence-ereignisse-voller-ring", ev,
                   "M-05: 64 Plaetze sind der Deckel und noch gueltig (Grenztest an N)"))

    ev = copy.deepcopy(GRUND["evidence_snapshot"])
    ev["stereo"] = copy.deepcopy(STEREO)
    ev["stereo"]["freiheitsgrade"] = [0] * 221
    ev["stereo"]["kohaerenz"]["gueltig_bitmap"] = base64.b64encode(
        bytes(28)).decode("ascii")
    ev["stereo"]["phase_rad"]["gueltig_bitmap"] = base64.b64encode(
        bytes(28)).decode("ascii")
    faelle.append(("evidence-stereo-ohne-kohaerenz", ev,
                   "M-11 fail-closed: unter acht Welch-Frames traegt weder Kohaerenz noch Phase ein Bit — es entsteht keine geschaetzte Zahl"))

    for grund in ("material_wechsel", "messpunkt_wechsel"):
        ei = copy.deepcopy(GRUND["evidence_invalidate"])
        ei["grund"] = grund
        faelle.append((f"invalidate-{grund.replace('_', '-')}", ei,
                       "M-54/M-55: neuer Grund der Fassung 2; ein Leser der Fassung 1 lehnt ihn ab, statt ihn still abzubilden"))

    ea = copy.deepcopy(GRUND["experiment_abort"])
    ea["grund"] = "verdraengt"
    faelle.append(("experiment-abort-verdraengt", ea,
                   "M-47/E-03: die Verdraengung durch einen Bestandsdeckel ist der zweite und letzte Ausloeser eines terminalen aborted"))

    for urteil in ("baseline", "kein_unterschied", "enthaltung"):
        r = copy.deepcopy(GRUND["experiment_manual_result"])
        r["hoerurteil"] = urteil
        faelle.append((f"experiment-manual-result-{urteil.replace('_', '-')}", r,
                       "M-46: das Hoerurteil ist Userdatum; enthaltung ist ein vollwertiger Wert und kein fehlender"))

    r = copy.deepcopy(GRUND["experiment_manual_result"])
    r["blindreihenfolge"] = "kandidat_zuerst"
    r["notiz"] = None
    r["werkzeug"] = None
    faelle.append(("experiment-manual-result-ohne-notiz", r,
                   "M-42/M-44: beide Reihenfolgen sind gueltig; null heisst ausdruecklich 'nichts angegeben' und ist keine leere Zeichenkette"))

    eb = copy.deepcopy(GRUND["experiment_begin"])
    eb["passage"]["label"] = None
    faelle.append(("experiment-begin-passage-ohne-label", eb,
                   "M-25: das Userwort ist das EINZIGE optionale Feld der Passage; "
                   "null heisst 'die Passage hat keines' und ist keine leere Zeichenkette"))

    # Nacharbeit 2 (Befund R22): `art=experiment` traegt seinen Bezug. Ohne ihn
    # gaebe es eine Intervention, die zu einem Versuch gehoert, ohne zu sagen,
    # zu welchem — die Zuordnung waere geraten.
    ib = copy.deepcopy(GRUND["audible_intervention_begin"])
    ib["intervention_id"] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab"
    ib["event_sequence"] = 5
    ib["art"] = "experiment"
    ib["experiment_id"] = "abababababababababababababababab"
    ib["project_sample_start"] = 44108200
    faelle.append(("audible-intervention-begin-experiment", ib,
                   "M-59, Nacharbeit 2 (R22): art=experiment traegt seine experiment_id"))

    # Nacharbeit 2 (Befunde R14/R32): der Rueckweg zu Gen.
    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["experimente"] = [copy.deepcopy(SESSION_VERSUCH)]
    ss["paare"] = copy.deepcopy(SESSION_PAARE)
    faelle.append(("session-snapshot-mit-experimenten-und-paaren", ss,
                   "M-49/M-13, Nacharbeit 2 (R14/R32): der Rueckweg zu Gen laeuft "
                   "ueber den bestehenden Snapshot-Pfad"))

    # Nacharbeit 3 (Befund C5, M-07): der Nichtendlich-Zaehler REIST.
    eb = copy.deepcopy(GRUND["experiment_begin"])
    eb["referenz"]["nicht_endliche_samples"] = 0
    faelle.append(("experiment-begin-nicht-endliche-samples-null", eb,
                   "M-07, Nacharbeit 3 (C5): 0 heisst NACHWEISLICH kein "
                   "nichtendliches Sample - nicht 'nicht gemessen'"))

    eb = copy.deepcopy(GRUND["experiment_begin"])
    eb["referenz"]["nicht_endliche_samples"] = 7
    faelle.append(("experiment-begin-nicht-endliche-samples-gezaehlt", eb,
                   "M-07, Nacharbeit 3 (C5): der Zaehler des Vergleichspegels "
                   "reist im Wirezustand mit, statt nur lokal zu verriegeln"))

    eb = copy.deepcopy(GRUND["experiment_begin"])
    eb["referenz"]["alignment"] = "unclear"
    eb["referenz"]["aktive_quellen"] = ["33333333333333333333333333333333"]
    eb["referenz"]["messpunktklassen"] = ["post_fader_contribution"]
    faelle.append(("experiment-begin-unclear-alignment", eb,
                   "M-16: `unclear` ist eine vollwertige der vier Alignmentklassen und sperrt keine Nachricht"))


    # ── SONDE-014 (P5), Fassung 3 ─────────────────────────────────────────
    #
    # Die drei optionalen Listen des `intent_update` duerfen FEHLEN: ein
    # leerer Bestand mit Vollstaendigkeitsmarke ist die ehrliche Meldung
    # "diese Sitzung hat keinen Intent" (M-86). Ohne sie saehe ein
    # Transportverlust wie "kein Schutz gewuenscht" aus.
    iu = copy.deepcopy(GRUND["intent_update"])
    for feld in ("intents", "schutzangaben", "beziehungen"):
        iu.pop(feld)
    iu["bestand_revision"] = 0
    faelle.append(("intent-update-leerer-bestand-mit-marke", iu,
                   "M-86: der vollstaendige LEERE Bestand wird gemeldet, nicht verschwiegen"))

    iu = copy.deepcopy(GRUND["intent_update"])
    iu["vollstaendig"] = False
    iu.pop("schutzangaben")
    iu.pop("beziehungen")
    faelle.append(("intent-update-einzelne-fortschreibung", iu,
                   "M-85: die inkrementelle Fortschreibung traegt genau ein Objekt"))

    iu = copy.deepcopy(GRUND["intent_update"])
    iu["intents"] = [dict(SOURCE_INTENT)]
    iu["intents"][0].pop("passage_id")
    faelle.append(("intent-update-globaler-scope", iu,
                   "Abwesenheit der passage_id heisst GLOBALER Scope, nie null"))

    # Alle fuenf Rollen und alle drei Herkuenfte kommen wirklich vor.
    iu = copy.deepcopy(GRUND["intent_update"])
    iu["intents"] = []
    for i, (rolle, herkunft) in enumerate(
            [("fuehrt", "user"), ("traegt", "template"), ("begleitet", "inferred"),
             ("geschuetzt", "user"), ("verschmolzen", "user")]):
        eintrag = dict(SOURCE_INTENT)
        eintrag["quelle_id"] = f"{i + 1}" * 32
        eintrag["rolle"] = rolle
        eintrag["herkunft"] = herkunft
        iu["intents"].append(eintrag)
    faelle.append(("intent-update-alle-fuenf-rollen", iu,
                   "U22: die Rollenmenge hat genau fuenf Werte, und alle fuenf sind gueltig"))

    asu = copy.deepcopy(GRUND["assistant_step_update"])
    asu["schritt"] = "preview"
    asu["offen"] = False
    faelle.append(("assistant-step-preview-bleibt-im-vertrag", asu,
                   "E-07: `preview` bleibt in der Zustandsmenge; dass P5 keine Kante dorthin fuehrt, gehoert dem Consumer"))

    asu = copy.deepcopy(GRUND["assistant_step_update"])
    asu.pop("finding_id")
    asu.pop("proposal_id")
    faelle.append(("assistant-step-ohne-bezug", asu,
                   "Ein Schritt vor dem ersten Befund traegt keine Bezuege - Abwesenheit ist kein null"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    ss["findings"] = [SESSION_FINDING]
    faelle.append(("session-snapshot-mit-findings", ss,
                   "E-04: die Befunde reisen im Sessionsnapshot, je Befund mit eingebettetem maskierung"))

    # ── NAK-213 (Fassung 4): die zwei neuen Ausschlussgruende ────────────
    #
    # Diese drei Faelle sind GUELTIG in der Fassung 4 und werden vom Leser
    # der Fassung 3 ABGEWIESEN. Genau das misst der Abschnitt "Rueckbau auf
    # Fassung 3" in pruefe_v3_vertrag.py (A5) und der Modultest
    # `fassung_3_kennt_die_neuen_gruende_nicht` in broker schema.rs; der
    # Fixture-Korpus selbst kennt nur die AKTUELLE Fassung und klassifiziert
    # jede Datei gegen sie - deshalb steht ein Dokument, das erst eine
    # Fassung tiefer faellt, hier bei den gueltigen (K-46, K-50).
    for grund in ("screening_ueberboten", "master_duplikat"):
        ss = copy.deepcopy(GRUND["session_snapshot"])
        befund = copy.deepcopy(SESSION_FINDING)
        befund["ausschluesse"] = [{"candidate_source": QUELLE_B, "grund": grund}]
        ss["findings"] = [befund]
        faelle.append((f"finding-ausschlussgrund-{grund.replace('_', '-')}-in-fassung-3", ss,
                       f"NAK-213 R1/R3 (K-46): `{grund}` gilt AB Fassung 4. Ein Leser der "
                       "Fassung 3 muss dieses Dokument ABLEHNEN, statt den Grund still auf "
                       "einen bekannten abzubilden - gemessen im Rueckbau (A5) und in "
                       "SourcesModel (B28)"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    befund = copy.deepcopy(SESSION_FINDING)
    befund["ausschluesse"] = [{"candidate_source": f"{0x3000 + i:032x}", "grund": "coverage_fehlt"}
                              for i in range(33)]
    ss["findings"] = [befund]
    faelle.append(("finding-33-ausschluesse", ss,
                   "NAK-213 R6 (K-50): 33 ist der kleinste Ueberlauf ueber die Grenze der "
                   "Fassung 3 und produktiv erreichbar (38 gate-faehige Sonden). In Fassung 4 "
                   "gueltig, in Fassung 3 ungueltig - die Ausschlussliste wird nie gekappt (M-87)"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    befund = copy.deepcopy(SESSION_FINDING)
    befund["ausschluesse"] = [{"candidate_source": f"{0x3000 + i:032x}", "grund": "coverage_fehlt"}
                              for i in range(64)]
    ss["findings"] = [befund]
    faelle.append(("finding-64-ausschluesse", ss,
                   "NAK-213 R6 (K-50): der obere Rand SESSION_CLIENT_CAP = 64. Mehr Ausschluesse "
                   "als Quellen der Sitzung kann es nicht geben; die Grenze ist damit selbst "
                   "gebunden und nicht geraten"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    befund = copy.deepcopy(SESSION_FINDING)
    befund.pop("maskierung")
    befund.pop("alternatives")
    befund.pop("ausschluesse")
    befund.pop("pre_post")
    befund.pop("passage_id")
    ss["findings"] = [befund]
    faelle.append(("session-snapshot-finding-ohne-optionales", ss,
                   "Abwesenheit von maskierung, Alternativen, Ausschluessen, Ort und Passage heisst `nichts`, nie null"))

    ss = copy.deepcopy(GRUND["session_snapshot"])
    befund = copy.deepcopy(SESSION_FINDING)
    befund["zustand"] = "more_data"
    befund["confidence"] = {"class": "unklar", "score": 0.2}
    befund["maskierung"] = dict(MASKIERUNG)
    befund["maskierung"]["gueltig"] = False
    befund["maskierung"]["wert_db"] = 0.0
    befund["maskierung"]["herabgesetzt"] = True
    ss["findings"] = [befund]
    faelle.append(("session-snapshot-finding-more-data-herabgesetzt", ss,
                   "NaN-Ehrlichkeit: ein Maskierungswert ohne Messung traegt 0 mit gueltig=false, und Rueckstau setzt `herabgesetzt`"))

    eb = copy.deepcopy(GRUND["experiment_begin"])
    eb["ziel"] = EXPERIMENT_ZIEL
    faelle.append(("experiment-begin-mit-ziel", eb,
                   "E-05: mit `ziel` liest der Guardrail-Rechner Ziel- und Schutzbereiche, statt sie zu raten"))

    do = copy.deepcopy(GRUND["draft_offer"])
    do["proposal"] = copy.deepcopy(PROPOSAL)
    do["proposal"]["action"] = "no_change"
    do["proposal"]["parameters"] = {}
    do["proposal"]["allowed_bounds"] = {}
    do["proposal"]["expected_effect"] = "none"
    do["proposal"]["revert"] = "none_needed"
    do["proposal"]["stop_if"] = ["keine_wiederholbare_masteraenderung"]
    faelle.append(("draft-offer-keine-aenderung", do,
                   "§59: `keine Aenderung` ist ein GUELTIGER Vorschlag mit vollstaendigem Objekt, kein leerer Rueckgabewert"))

    do = copy.deepcopy(GRUND["draft_offer"])
    do["proposal"] = copy.deepcopy(PROPOSAL)
    do["proposal"]["action"] = "more_data"
    do["proposal"]["parameters"] = {}
    do["proposal"]["allowed_bounds"] = {}
    do["proposal"]["expected_effect"] = "none"
    do["proposal"]["revert"] = "none_needed"
    do["proposal"]["next"] = None
    do["proposal"].pop("next")
    faelle.append(("draft-offer-mehr-daten", do,
                   "§59: `mehr Daten` ebenso - beide tragen Evidenz-IDs, Hoerziel, Stopbedingung und Rueckweg"))

    uv = copy.deepcopy(GRUND["user_verdict"])
    uv.pop("notiz")
    uv.pop("proposal_id")
    uv["urteil"] = "enthaltung"
    faelle.append(("user-verdict-enthaltung-ohne-notiz", uv,
                   "Eine Enthaltung ohne Notiz und ohne Vorschlag ist ein regulaeres Urteil"))

    # ── SONDE-015 (NAK-110): der bestaetigte DSP im state_report ──────────
    jcs, hash_hex = dsp_nutzlast()
    sr = copy.deepcopy(GRUND["state_report"])
    sr["state_hash"] = hash_hex
    sr["dsp"] = dsp_bericht()
    faelle.append(("state-report-mit-dsp", sr,
                   "Fassung 5: dsp traegt GENAU die RFC-8785-Zeichenkette, ueber die state_hash gebildet wurde; "
                   "SHA-256(dsp.jcs) == state_hash ist damit nachrechenbar (M-103, M-105)"))

    # Ein Bericht mit LEEREN abgeleiteten Listen: `nichts geklemmt` und
    # `kein Band verletzt` sind Aussagen, keine Abwesenheit.
    leer = copy.deepcopy(GRUND["state_report"])
    leer["state_hash"] = hash_hex
    leer["dsp"] = {"jcs": jcs, "auto_gain_db": 0.0,
                   "klemmungen": [], "verletzte_baender": []}
    faelle.append(("state-report-dsp-ohne-klemmung", leer,
                   "leere Listen heissen `nichts geklemmt` und `kein Band verletzt` - beide sind Pflichtfelder"))

    # SCHEMAGUELTIG, aber der Hash passt nicht: die Zeichenkette wurde
    # veraendert, `state_hash` blieb stehen. Das Schema kann das nicht sehen -
    # BEIDE Leser muessen es sehen und den Bericht GANZ abweisen (M-105). Das
    # Fixture liegt deshalb bewusst unter `gueltig/`: der Unterschied zwischen
    # Schemaurteil und Leserurteil IST die Zusage.
    #
    # 🔑 Nacharbeit 1 (B-04): die Mutation bleibt INNERHALB der
    # Vertragsgrenzen. Die vorige Fassung schob `width` von 1.25 auf 2.25 und
    # riss damit die Obergrenze 2.0 - der Rotbeweis M-105 (Hashvergleich
    # abgeschaltet) fiel danach am Bereichsgrund statt an der Annahme, und die
    # Zusage "nur der Hash haelt ihn auf" war nicht gemessen. Die Zusicherung
    # unten haelt das fest, statt es zu behaupten.
    falsch = copy.deepcopy(GRUND["state_report"])
    falsch["state_hash"] = hash_hex
    falsch["dsp"] = dsp_bericht()
    mutiert = jcs.replace('"v1.global.width":1.25', '"v1.global.width":1.5', 1)
    assert mutiert != jcs, "die Mutation muss greifen"
    grund = dto_grund(mutiert)
    assert grund is None, (
        f"die mutierte Zeichenkette muss den DTO-Weg PASSIEREN, sonst misst der "
        f"Rotbeweis M-105 einen anderen Grund als die Hashpruefung (gelesen: {grund})")
    falsch["dsp"]["jcs"] = mutiert
    faelle.append(("state-report-dsp-hash-passt-nicht", falsch,
                   "schemagueltig UND DTO-gueltig - nur SHA-256(dsp.jcs) != state_hash. "
                   "Beide Leser weisen den Bericht GANZ ab; faellt der Hashvergleich weg, "
                   "wird er ANGENOMMEN (M-105)"))

    # Der Gegenfall zu B-02: der Hash STIMMT, das DTO nicht. Ein Leser, der
    # nach der Hashpruefung aufhoert, nimmt ihn an und speichert `{}` als
    # bestaetigten DSP. R13 sagt aber "der Broker liest, VALIDIERT und haelt".
    leeres_dto = "{}"
    assert dto_grund(leeres_dto) is not None, "das Gegenfixture muss am DTO-Weg fallen"
    dto_falsch = copy.deepcopy(GRUND["state_report"])
    dto_falsch["state_hash"] = hashlib.sha256(leeres_dto.encode("utf-8")).hexdigest()
    dto_falsch["dsp"] = dsp_bericht()
    dto_falsch["dsp"]["jcs"] = leeres_dto
    faelle.append(("state-report-dsp-dto-ungueltig", dto_falsch,
                   "schemagueltig und der Hash STIMMT - aber die Zeichenkette ist kein DSP-DTO. "
                   "Beide Leser pruefen nach dem Hash den exakten DTO-Weg und weisen ihn GANZ ab "
                   "(M-105, R13 'liest, validiert und haelt')"))
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
    if instanz == "/mitglieder/0":
        instanz = "/mitglieder/0/probe_descriptor"
    elif instanz.startswith("/mitglieder/0/"):
        rest = instanz.removeprefix("/mitglieder/0/")
        if not rest.startswith(("p2_reject", "probe_descriptor")):
            instanz = "/mitglieder/0/probe_descriptor/" + rest
    return {"instanz": instanz, "schema": schema, "schluessel": schluessel}


def vm(instanz: str, schema: str, schluessel: str) -> dict:
    """Verletzung an der Minor-1-Mitgliedshuelle, nicht am Descriptor."""
    return {"instanz": instanz, "schema": schema, "schluessel": schluessel}


UNGUELTIG: list[tuple] = [

    # ── SONDE-014 (P5), Fassung 3 ─────────────────────────────────────────
    #
    # Je Definition mindestens eine Verletzung, je Wertedomaene eine an ihrem
    # eigenen Pfad. A5 laeuft mit --abdeckung fail-closed.

    ("intent-rolle-sechster-wert", "intent_update",
     [setze("intents", 0, "rolle", "impuls")],
     [v("/intents/0/rolle", f"{S}/intent_rolle/enum", "enum")],
     "U22: die Rollenmenge hat GENAU fuenf Werte; Impuls ist keine Rolle, sondern eine Schutzangabe"),

    ("intent-herkunft-unbekannt", "intent_update",
     [setze("intents", 0, "herkunft", "geraten")],
     [v("/intents/0/herkunft", f"{S}/intent_herkunft/enum", "enum")],
     "§37.1 kennt genau user, template und inferred"),

    ("intent-konfidenz-ueber-eins", "intent_update",
     [setze("intents", 0, "konfidenz", 1.5)],
     [v("/intents/0/konfidenz", f"{S}/source_intent/properties/konfidenz/maximum", "maximum")],
     "Die Konfidenz liegt in [0,1]; 1,5 ist keine grosszuegige Angabe, sondern eine falsche"),

    ("intent-revision-null", "intent_update",
     [setze("intents", 0, "revision", 0)],
     [v("/intents/0/revision", f"{S}/source_intent/properties/revision/minimum", "minimum")],
     "Revision 0 gibt es nicht: die erste Fassung eines Intents traegt 1"),

    ("intent-schutz-eigenschaft-unbekannt", "intent_update",
     [setze("schutzangaben", 0, "eigenschaft", "hall")],
     [v("/schutzangaben/0/eigenschaft", f"{S}/intent_schutzeigenschaft/enum", "enum")],
     "§37.1 nennt Attack, Breite, Ausklang und Bandintervall - Hall gehoert zur Verschmelzungs-Erlaubnis"),

    ("intent-band-ueber-dem-gitterrand", "intent_update",
     [setze("schutzangaben", 0, "band", "bis", 222)],
     [v("/schutzangaben/0/band/bis", f"{S}/bandintervall/properties/bis/maximum", "maximum")],
     "Das Evidenzgitter hat 221 Baender; `bis` ist halboffen und endet bei 221"),

    ("intent-beziehungsart-unbekannt", "intent_update",
     [setze("beziehungen", 0, "art", "haengt_ab")],
     [v("/beziehungen/0/art", f"{S}/intent_beziehungsart/enum", "enum")],
     "§37.1 kennt fuehrt_vor und darf_verschmelzen; gleichrangig ist der aufgeloeste Zyklus"),

    ("intent-beziehung-fremdes-feld", "intent_update",
     [setze("beziehungen", 0, "gewicht", 0.5)],
     [v("/beziehungen/0/gewicht", f"{S}/intent_beziehung/additionalProperties",
        "additionalProperties")],
     "Eine Beziehung traegt zwei Quellen und ihre Art - ein Gewicht waere ein Score, und §37.2 ist eine geordnete Liste"),

    ("intent-update-fremdes-feld", "intent_update",
     [setze("prioritaet", 3)],
     [v("/prioritaet", f"{S}/intent_update/additionalProperties", "additionalProperties")],
     "Der Intenttransport ist STRIKT: ein unbekanntes Feld waere eine unbekannte Absicht"),

    ("intent-update-ohne-vollstaendigkeitsmarke", "intent_update",
     [loesche("vollstaendig")],
     [v("", f"{S}/intent_update/required/vollstaendig", "required")],
     "M-86: ohne die Marke kann der Broker `vollstaendiger Bestand` nicht von `Ausschnitt` unterscheiden"),

    ("assistant-schritt-unbekannt", "assistant_step_update",
     [setze("schritt", "nachdenken")],
     [v("/schritt", f"{S}/assistant_schritt/enum", "enum")],
     "§46.1 fuehrt genau acht Zustaende; ein neunter waere eine zweite Zustandsmaschine"),

    ("assistant-step-fremdes-feld", "assistant_step_update",
     [setze("kommentar", "spaeter")],
     [v("/kommentar", f"{S}/assistant_step_update/additionalProperties", "additionalProperties")],
     "Der Spiegel ist strikt: was der Main nicht sagt, erfindet der Broker nicht"),

    ("assistant-step-revision-null", "assistant_step_update",
     [setze("revision", 0)],
     [v("/revision", f"{S}/assistant_step_update/properties/revision/minimum", "minimum")],
     "Auch der Schritt beginnt bei Revision 1 - sonst waere `nie gemeldet` von `erste Meldung` nicht zu trennen"),

    ("finding-ursachenklasse-achte", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "ursachenklasse", "unbekannt")],
     [v("/findings/0/ursachenklasse", f"{S}/ursachenklasse/enum", "enum")],
     "Entwurf §8 zaehlt SIEBEN Ursachenklassen ab; die siebte ist `daten_reichen_nicht`, eine achte gibt es nicht"),

    ("finding-aussageklasse-vierte", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "claim_class", "vermutung")],
     [v("/findings/0/claim_class", f"{S}/aussageklasse/enum", "enum")],
     "§36.1 fuehrt genau drei Aussageklassen"),

    ("finding-zustand-vierter", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "zustand", "pending")],
     [v("/findings/0/zustand", f"{S}/befund_zustand/enum", "enum")],
     "Abnahme U21: genau drei Zustaende, und die UI liest sie, statt sie zu raten"),

    ("finding-sicherheitsklasse-unbekannt", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "confidence", "class", "sehr_hoch")],
     [v("/findings/0/confidence/class", f"{S}/sicherheitsklasse/enum", "enum")],
     "Entwurf §8: hoch, mittel oder noch unklar - mehr Stufen gibt es nicht"),

    ("finding-konfidenz-fremdes-feld", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "confidence", "prozent", 68)],
     [v("/findings/0/confidence/prozent",
        f"{S}/befund_konfidenz/additionalProperties", "additionalProperties")],
     "Die BefundSICHERHEIT ist strikt - anders als die additive Messqualitaet der Passage"),

    ("finding-zielmetrik-unbekannt", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "target_metric", "master_glanz")],
     [v("/findings/0/target_metric", f"{S}/zielmetrik/enum", "enum")],
     "Eine Zielmetrik ohne Vertragsteil im evidence_snapshot waere nicht messbar"),

    ("finding-naechster-test-unbekannt", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "next_test", "einfach_lauter")],
     [v("/findings/0/next_test", f"{S}/naechster_test/enum", "enum")],
     "Der naechste Beweisschritt kommt aus einer geschlossenen Menge geprueft Templates"),

    ("finding-ausschlussgrund-unbekannt", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "ausschluesse", 0, "grund", "passt_nicht")],
     [v("/findings/0/ausschluesse/0/grund", f"{S}/ausschlussgrund/enum", "enum")],
     "R4: acht Gruende, jeder an eine Matrixzeile gebunden; ein neunter waere erfunden"),

    ("finding-ausschluss-fremdes-feld", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "ausschluesse", 0, "text", "zu schwach")],
     [v("/findings/0/ausschluesse/0/text",
        f"{S}/finding_ausschluss/additionalProperties", "additionalProperties")],
     "Ein Ausschluss traegt Kandidat und Grund, keinen Freitext"),

    ("finding-65-ausschluesse", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "ausschluesse",
            [{"candidate_source": f"{0x3000 + i:032x}", "grund": "coverage_fehlt"}
             for i in range(65)])],
     [v("/findings/0/ausschluesse",
        f"{S}/session_finding/properties/ausschluesse/maxItems", "maxItems")],
     "NAK-213 R6 (K-50): 65 faellt in BEIDEN Fassungen. Die Grenze ist SESSION_CLIENT_CAP = 64 "
     "- ein Main und bis zu 63 Sonden -, also kann kein Befund mehr Ausschluesse tragen als die "
     "Sitzung Quellen hat"),

    ("finding-pre-post-unbekannt", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "pre_post", "mitte")],
     [v("/findings/0/pre_post",
        f"{S}/session_finding/properties/pre_post/enum", "enum")],
     "§8 Teil 1 kennt genau die PRE- und die POST-Stelle"),

    ("finding-ohne-evidenz-ids", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "evidence_ids", [])],
     [v("/findings/0/evidence_ids",
        f"{S}/session_finding/properties/evidence_ids/minItems", "minItems")],
     "Exit-Gate §59 woertlich: JEDE sichtbare Behauptung referenziert existente Evidenz-IDs"),

    ("finding-belegtext-statt-zone", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "beleg_text", "180-280 Hz, 78 Prozent der Faelle")],
     [v("/findings/0/beleg_text", f"{S}/session_finding/additionalProperties",
        "additionalProperties")],
     "Abnahme U21: der Beleg ist die markierte Zone mit zwei Kurven, NICHT ein wiederholter Text"),

    ("finding-vierte-anzeigezeile", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "why_it_matters", "Refrain traegt den Text")],
     [v("/findings/0/why_it_matters", f"{S}/session_finding/additionalProperties",
        "additionalProperties")],
     "Abnahme U21: genau drei Zeilen je Befund - eine vierte entsteht auch im Datenweg nicht"),

    ("finding-rang-ohne-komponente", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      loesche("findings", 0, "rang", "uplift")],
     [v("/findings/0/rang", f"{S}/rangkomponenten/required/uplift", "required")],
     "§36.2: der Rang hat SECHS getrennte Komponenten; eine fehlende waere eine stille Gewichtung"),

    ("finding-beobachtung-ohne-bit", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      loesche("findings", 0, "beobachtung", "gueltig")],
     [v("/findings/0/beobachtung", f"{S}/beobachtung/required/gueltig", "required")],
     "Ohne Gueltigkeitsbit ist die Zahl keine Messung"),

    ("finding-maskierung-fremdes-feld", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "maskierung", "farbe", "#c0392b")],
     [v("/findings/0/maskierung/farbe", f"{S}/maskierung/additionalProperties",
        "additionalProperties")],
     "Die Engine kennt keine Optik: der Datenweg traegt Frequenzbereich und Wert, keine Zeichenanweisung"),

    ("finding-maskierung-band-unter-null", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "maskierung", "band_von", -1)],
     [v("/findings/0/maskierung/band_von",
        f"{S}/maskierung/properties/band_von/minimum", "minimum")],
     "Band 0 ist das erste; ein negativer Index benennt kein Band dieses Gitters"),

    ("finding-band-hz-fremdes-feld", "session_snapshot",
     [setze("findings", [SESSION_FINDING]),
      setze("findings", 0, "band_hz", "achse", "log")],
     [v("/findings/0/band_hz/achse", f"{S}/bandintervall/additionalProperties",
        "additionalProperties")],
     "M-36: der Wert benennt ein Bandintervall des bestehenden Gitters und traegt keine eigene Achse"),

    ("proposal-aktion-unbekannt", "draft_offer",
     [setze("proposal", "action", "kompressor")],
     [v("/proposal/action", f"{S}/proposal_aktion/enum", "enum")],
     "§42.2: der Befundtyp waehlt eine GEPRUEFTE Aktionstemplate, keine freie Aktion"),

    ("proposal-execution-unbekannt", "draft_offer",
     [setze("proposal", "execution", "automatisch")],
     [v("/proposal/execution", f"{S}/proposal_execution/enum", "enum")],
     "§42.2 Punkt 5 kennt manual und previewable; in P5 ist previewable zusaetzlich eine Consumerregel"),

    ("proposal-revert-vierter-wert", "draft_offer",
     [setze("proposal", "revert", "dsp_revert")],
     [v("/proposal/revert", f"{S}/proposal_revert/enum", "enum")],
     "E-06: drei Rueckwege; `dsp_revert` gehoert P7 und ist in P5 ein Vertragsbruch"),

    ("proposal-wirkung-unbekannt", "draft_offer",
     [setze("proposal", "expected_effect", "klingt_besser")],
     [v("/proposal/expected_effect", f"{S}/proposal_wirkung/enum", "enum")],
     "Die erwartete Wirkung ist eine geschlossene Menge, kein Werbetext"),

    ("proposal-stopbedingung-unbekannt", "draft_offer",
     [setze("proposal", "stop_if", ["klingt_komisch"])],
     [v("/proposal/stop_if/0", f"{S}/proposal_stopbedingung/enum", "enum")],
     "Eine Stopbedingung ohne messbaren Guardrail waere nicht ueberpruefbar"),

    ("proposal-ohne-stopbedingung", "draft_offer",
     [setze("proposal", "stop_if", [])],
     [v("/proposal/stop_if", f"{S}/proposal/properties/stop_if/minItems", "minItems")],
     "Exit-Gate §59: JEDES Proposal nennt eine Stopbedingung"),

    ("proposal-ohne-rueckweg", "draft_offer",
     [loesche("proposal", "revert")],
     [v("/proposal", f"{S}/proposal/required/revert", "required")],
     "E-06: der Rueckweg ist ein FELD, kein Versprechen im Text"),

    ("proposal-ohne-passage", "draft_offer",
     [loesche("proposal", "passage_id")],
     [v("/proposal", f"{S}/proposal/required/passage_id", "required")],
     "NR-07: das Exit-Gate verlangt die PASSAGE woertlich - auch fuer "
     "`no_change` und `more_data`; ein Vorschlag ohne Ort ist keiner (M-43/M-46)"),

    ("proposal-ohne-grenzen", "draft_offer",
     [loesche("proposal", "allowed_bounds")],
     [v("/proposal", f"{S}/proposal/required/allowed_bounds", "required")],
     "Exit-Gate §59: jedes Proposal nennt seine Grenzen"),

    ("proposal-schema-zwei", "draft_offer",
     [setze("proposal", "proposal_schema", 2)],
     [v("/proposal/proposal_schema",
        f"{S}/proposal/properties/proposal_schema/const", "const")],
     "Das Proposal-Schema ist eingefroren; eine zweite Fassung ist ein eigener Vertragsschritt"),

    ("proposal-parameter-fremdes-feld", "draft_offer",
     [setze("proposal", "parameters", "slope_db_oct", 12)],
     [v("/proposal/parameters/slope_db_oct",
        f"{S}/proposal_parameter/additionalProperties", "additionalProperties")],
     "Die Parametermenge ist strikt: ein unbekannter Parameter waere ein unbekannter Eingriff"),

    ("proposal-grenzen-fremdes-feld", "draft_offer",
     [setze("proposal", "allowed_bounds", "slope", {"von": 6.0, "bis": 12.0})],
     [v("/proposal/allowed_bounds/slope",
        f"{S}/proposal_grenzen/additionalProperties", "additionalProperties")],
     "Dieselbe Strenge auf der Grenzenseite - sonst traege ein Vorschlag Grenzen fuer etwas, das er nicht setzt"),

    ("proposal-wertebereich-fremdes-feld", "draft_offer",
     [setze("proposal", "allowed_bounds", "q", "schritt", 0.1)],
     [v("/proposal/allowed_bounds/q/schritt",
        f"{S}/wertebereich/additionalProperties", "additionalProperties")],
     "Ein Wertebereich hat zwei Enden und keine dritte Angabe"),

    ("proposal-schutz-als-wortliste", "draft_offer",
     [setze("proposal", "protected_traits", ["piano_attack"])],
     [v("/proposal/protected_traits/0", f"{S}/intent_schutz/type", "type")],
     "M-49: die geschuetzte Eigenschaft ist eine HARTE Constraint in derselben Form wie der Intent, keine Wortliste"),

    ("proposal-fremdes-feld", "draft_offer",
     [setze("proposal", "warum", "klingt besser")],
     [v("/proposal/warum", f"{S}/proposal/additionalProperties", "additionalProperties")],
     "Das Proposal ist zuerst ein validiertes Objekt und erst danach Text"),

    ("draft-offer-fremdes-feld", "draft_offer",
     [setze("dringlichkeit", "hoch")],
     [v("/dringlichkeit", f"{S}/draft_offer/additionalProperties", "additionalProperties")],
     "Der Traeger des Vorschlags ist strikt"),

    ("user-urteil-unbekannt", "user_verdict",
     [setze("urteil", "vielleicht")],
     [v("/urteil", f"{S}/user_urteil/enum", "enum")],
     "Das Userurteil ist eine geschlossene Menge; `vielleicht` ist `spaeter`"),

    ("user-verdict-ohne-finding", "user_verdict",
     [loesche("finding_id")],
     [v("", f"{S}/user_verdict/required/finding_id", "required")],
     "Ein Urteil ohne Befund haette keinen Gegenstand"),

    ("user-verdict-fremdes-feld", "user_verdict",
     [setze("sterne", 4)],
     [v("/sterne", f"{S}/user_verdict/additionalProperties", "additionalProperties")],
     "Userdaten sind Notiz und Urteil, keine Bewertungsskala"),

    ("experiment-ziel-fremdes-feld", "experiment_begin",
     [setze("ziel", EXPERIMENT_ZIEL), setze("ziel", "gain_db", -1.5)],
     [v("/ziel/gain_db", f"{S}/experiment_ziel/additionalProperties", "additionalProperties")],
     "E-05: das Ziel benennt Baender, keine Eingriffswerte - die stehen im Proposal"),

    ("experiment-ziel-band-ueber-dem-rand", "experiment_begin",
     [setze("ziel", EXPERIMENT_ZIEL), setze("ziel", "band_bis", 300)],
     [v("/ziel/band_bis", f"{S}/experiment_ziel/properties/band_bis/maximum", "maximum")],
     "Das Ziel liegt im 221-Band-Gitter wie jede andere bandweise Angabe"),
    # --- SONDE-012 Sessionbefehle und Storediagnose ----------------------
    ("session-command-fremdes-main-senderfeld", "session_command",
     [setze("sender", ADRESSE)],
     [v("/sender", f"{S}/session_command/oneOf/0/additionalProperties",
        "additionalProperties")],
     "Der Sender ist der autoritative Control-Link; ein fremdes Main darf sich nicht per Nutzlastfeld selbst autorisieren"),

    ("session-command-unbekannter-zweig", "session_command",
     [setze("command", "rename_probe")],
     [v("/command", f"{S}/session_command/oneOf", "oneOf")],
     "E-L18/L04 schliesst den Zweigsatz auf confirm_join und unbind_probe"),

    ("session-snapshot-store-degraded-false", "session_snapshot",
     [setze("store_degraded", False)],
     [v("/store_degraded",
        f"{S}/session_snapshot/properties/store_degraded/const", "const")],
     "E-L15-Store: false reist nicht; Abwesenheit bleibt die einzige nicht-positive Form"),

    # --- SONDE-012 Runtime und P2-Fehlerkanal ----------------------------
    ("heartbeat-runtime-ohne-messpunkt", "heartbeat",
     [setze("runtime", {"betrieb": "active"})],
     [v("/runtime", f"{S}/heartbeat_runtime/required/messpunkt", "required")],
     "E-M01: ist runtime vorhanden, ist messpunkt Pflicht"),

    ("heartbeat-runtime-betrieb-unbekannt", "heartbeat",
     [setze("runtime", {"messpunkt": "insert", "betrieb": "sleeping"})],
     [v("/runtime/betrieb", f"{S}/betrieb/enum", "enum")],
     "E-L21/22: nur active, suspended und offline sind Vertragswerte"),

    ("heartbeat-runtime-messpunkt-unbekannt", "heartbeat",
     [setze("runtime", {"messpunkt": "send", "betrieb": "active"})],
     [v("/runtime/messpunkt",
        f"{S}/heartbeat_runtime/properties/messpunkt/enum", "enum")],
     "E-M01: Runtime kennt nur insert, pre und post"),

    ("heartbeat-runtime-hostname-zu-lang", "heartbeat",
     [setze("runtime", {"messpunkt": "insert", "betrieb": "active",
                        "host_bus_name": "😀" * 121})],
     [v("/runtime/host_bus_name", f"{S}/host_bus_name/maxLength", "maxLength")],
     "E-M01 verwendet exakt die B1-Codepointgrenze"),

    ("heartbeat-runtime-label-zu-lang", "heartbeat",
     [setze("runtime", {"messpunkt": "insert", "betrieb": "active",
                        "label": "😀" * 121})],
     [v("/runtime/label", f"{S}/probe_label/maxLength", "maxLength")],
     "E-D2 verwendet exakt die probe_label-Codepointgrenze"),

    ("heartbeat-runtime-label-falscher-typ", "heartbeat",
     [setze("runtime", {"messpunkt": "insert", "betrieb": "active",
                        "label": 7})],
     [v("/runtime/label", f"{S}/probe_label/type", "type")],
     "E-D2 transportiert Text und keine still normalisierte Fremdform"),

    ("session-p2-reject-freitext", "session_snapshot",
     [setze("mitglieder", 0, "p2_reject",
            {"grund": "CRC war heute komisch", "zaehler": 1})],
     [v("/mitglieder/0/p2_reject/grund", f"{S}/p2_reject/properties/grund/enum", "enum")],
     "E-L14 verbietet Freitextgruende auf dem Wire"),

    ("session-p2-reject-zaehler-null", "session_snapshot",
     [setze("mitglieder", 0, "p2_reject",
            {"grund": "feature_batch_ungueltig", "zaehler": 0})],
     [v("/mitglieder/0/p2_reject/zaehler", f"{S}/p2_reject/properties/zaehler/minimum", "minimum")],
     "E-L14: ein vorhandener Ablehnungsstand beginnt bei 1"),

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

    ("reservierter-typ-reference-match", "heartbeat", [setze("type", "reference_match")],
     [v("/type", "#/oneOf", "oneOf")],
     "reservierter Familienname fuer Referenz-Nachbilden/EQ-Match"),

    ("reserviertes-feld-session-snapshot-contribution-inputs", "session_snapshot",
     [setze("contribution_inputs", [])],
     [v("/contribution_inputs", f"{S}/session_snapshot/additionalProperties",
        "additionalProperties")],
     "reservierter Name ohne Nutzlast; Empfaenger ist Gen, nicht eine Sondenklasse"),

    # SEITENWECHSEL (SONDE-015 M-101): `state_report.dsp` ist eingeloest. Der
    # Negativplatz gehoert jetzt einem UNVOLLSTAENDIGEN Bericht - das leere
    # Objekt faellt an `required`, nicht mehr an `additionalProperties`.
    ("dsp-bericht-leer", "state_report", [setze("dsp", {})],
     [v("/dsp", f"{S}/dsp_bericht/required/auto_gain_db", "required"),
      v("/dsp", f"{S}/dsp_bericht/required/jcs", "required"),
      v("/dsp", f"{S}/dsp_bericht/required/klemmungen", "required"),
      v("/dsp", f"{S}/dsp_bericht/required/verletzte_baender", "required")],
     "der bestaetigte DSP ist ganz oder gar nicht: alle vier Felder sind Pflicht"),

    ("dsp-bericht-zusatzfeld", "state_report",
     [setze("dsp", {"jcs": "{}", "auto_gain_db": 0.0, "klemmungen": [],
                    "verletzte_baender": [], "extra": 1})],
     [v("/dsp/extra", f"{S}/dsp_bericht/additionalProperties", "additionalProperties")],
     "der Bericht ist strikt: ein unbekanntes Feld ist ein Senderfehler, keine Erweiterung"),

    ("dsp-auto-gain-ausserhalb", "state_report",
     [setze("dsp", {"jcs": "{}", "auto_gain_db": 120.5, "klemmungen": [],
                    "verletzte_baender": []})],
     [v("/dsp/auto_gain_db", f"{S}/dsp_bericht/properties/auto_gain_db/maximum", "maximum")],
     "120 ist die Berichtsgrenze, die Ableitung kann darueber liegen; 120.5 im Bericht ist ein Fehler"),

    ("dsp-verletztes-band-8", "state_report",
     [setze("dsp", {"jcs": "{}", "auto_gain_db": 0.0, "klemmungen": [],
                    "verletzte_baender": [8]})],
     [v("/dsp/verletzte_baender/0",
        f"{S}/dsp_bericht/properties/verletzte_baender/items/maximum", "maximum")],
     "es gibt acht Slots 0..7; eine 8 ist keine Slotnummer"),

    ("dsp-neunte-klemmung", "state_report",
     [setze("dsp", {"jcs": "{}", "auto_gain_db": 0.0,
                    "klemmungen": [{"id": "v1.band.0.sidechain_source",
                                    "gemeldet": "priority_sidechain",
                                    "wirksam": "internal"}] * 9,
                    "verletzte_baender": []})],
     [v("/dsp/klemmungen", f"{S}/dsp_bericht/properties/klemmungen/maxItems", "maxItems")],
     "hoechstens eine Klemmung je Slot"),

    ("dsp-klemmung-ohne-wirksam", "state_report",
     [setze("dsp", {"jcs": "{}", "auto_gain_db": 0.0,
                    "klemmungen": [{"id": "v1.band.0.sidechain_source",
                                    "gemeldet": "priority_sidechain"}],
                    "verletzte_baender": []})],
     [v("/dsp/klemmungen/0", f"{S}/dsp_klemmung/required/wirksam", "required")],
     "eine Klemmung ohne wirksamen Wert sagt nicht, was tatsaechlich gilt"),

    ("reserviertes-feld-command-ack-applied-dsp", "command_ack", [setze("applied_dsp", {})],
     [v("/applied_dsp", f"{S}/command_ack/oneOf/0/additionalProperties",
        "additionalProperties")],
     "reservierter Bestaetigungsname fuer tatsaechlich angewandte Werte"),

    ("reserviertes-feld-state-report-eq-enabled", "state_report", [setze("eq_enabled", True)],
     [v("/eq_enabled", f"{S}/state_report/additionalProperties", "additionalProperties")],
     "reservierter Betriebszustand ohne Anzeigezusage"),

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
     [v("/transport", f"{S}/transportstempel/required/validity", "required"),
      v("/transport/validity/cycle_bounds",
        f"{S}/transportstempel/oneOf/0/oneOf", "oneOf")],
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
     [setze("konfidenz", f"zusatz_{i}", 0.5) for i in range(14)],
     [v("/konfidenz", f"{S}/konfidenz/maxProperties", "maxProperties")],
     "dieselbe Grenze am zweiten additiven Objekt - konfidenz deklariert seit "
     "der Nacharbeit 1 ACHT Eigenschaften (samples_nicht_endlich kam dazu), "
     "also 8+8=16; drei stehen in der Grundform, vierzehn Zusaetze sind 17 "
     "und damit einer zu viel"),

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
     [v("/transport/time_basis", f"{S}/transportstempel/oneOf", "oneOf")],
     "Wandzeit misst nur IPC-Latenz und darf musikalische Frames nie ausrichten (§32.3)"),

    ("plugin-kind-erfunden", "session_snapshot", [setze("mitglieder", 0, "plugin_kind", "hub")],
     [vm("/mitglieder/0/plugin_kind",
         f"{S}/session_mitglied/properties/plugin_kind/enum", "enum"),
      v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_insert/properties/plugin_kind/enum", "enum")],
     "`hub` ist die v2-Rolle; v3 kennt main|passive_probe|active_probe|legacy (§32.2)"),

    ("hello-plugin-kind-erfunden", "hello_control", [setze("plugin_kind", "hub")],
     [v("/plugin_kind", f"{S}/plugin_kind/enum", "enum")],
     "auch der Hello-Pfad misst den eingefrorenen plugin_kind-Wortschatz"),

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

    ("probe-descriptor-main-pre", "session_snapshot",
     [setze("mitglieder", 0, "plugin_kind", "main"),
      setze("mitglieder", 0, "measurement_position", "pre")],
     [v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_pre/properties/plugin_kind/enum", "enum")],
     "main darf laut State-v2-Matrix keine PRE-Position fuehren"),

    ("probe-descriptor-main-post", "session_snapshot",
     [setze("mitglieder", 0, "plugin_kind", "main"),
      setze("mitglieder", 0, "measurement_position", "post")],
     [v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_post/properties/plugin_kind/enum", "enum")],
     "main darf laut State-v2-Matrix keine POST-Position fuehren"),

    ("beitrag-plugin-kind-main", "session_snapshot",
     [setze("mitglieder", 0, "plugin_kind", "main"),
      setze("mitglieder", 0, "measurement_position", "post_fader_contribution"),
      setze("mitglieder", 0, "aussageklasse", "beitrag")],
     [v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_beitrag/properties/plugin_kind/maxLength", "maxLength")],
     "keine heutige Klasse darf den Beitragszweig fuehren"),

    ("beitrag-plugin-kind-legacy", "session_snapshot",
     [setze("mitglieder", 0, "plugin_kind", "legacy"),
      setze("mitglieder", 0, "measurement_position", "post_fader_contribution"),
      setze("mitglieder", 0, "aussageklasse", "beitrag")],
     [v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_beitrag/properties/plugin_kind/maxLength", "maxLength")],
     "keine heutige Klasse darf den Beitragszweig fuehren"),

    ("beitrag-plugin-kind-passive-probe", "session_snapshot",
     [setze("mitglieder", 0, "plugin_kind", "passive_probe"),
      setze("mitglieder", 0, "measurement_position", "post_fader_contribution"),
      setze("mitglieder", 0, "aussageklasse", "beitrag")],
     [v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_beitrag/properties/plugin_kind/maxLength", "maxLength")],
     "das fruehere Positivfixture ist nach der bindenden Klassenmatrix negativ"),

    ("beitrag-plugin-kind-active-probe", "session_snapshot",
     [setze("mitglieder", 0, "plugin_kind", "active_probe"),
      setze("mitglieder", 0, "measurement_position", "post_fader_contribution"),
      setze("mitglieder", 0, "aussageklasse", "beitrag")],
     [v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_beitrag/properties/plugin_kind/maxLength", "maxLength")],
     "keine heutige Klasse darf den Beitragszweig fuehren"),

    ("beitrag-plugin-kind-erfunden", "session_snapshot",
     [setze("mitglieder", 0, "plugin_kind", "hub"),
      setze("mitglieder", 0, "measurement_position", "post_fader_contribution"),
      setze("mitglieder", 0, "aussageklasse", "beitrag")],
     [v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_beitrag/properties/plugin_kind/enum", "enum"),
      v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_beitrag/properties/plugin_kind/maxLength", "maxLength"),
      vm("/mitglieder/0/plugin_kind",
         f"{S}/session_mitglied/properties/plugin_kind/enum", "enum")],
     "der unerfuellbare Zweig bleibt auch gegen erfundene Klassen strikt"),

    ("beitrag-ohne-contribution-aux", "session_snapshot",
     [setze("mitglieder", 0, "measurement_position", "post_fader_contribution"),
      setze("mitglieder", 0, "aussageklasse", "beitrag"),
      setze("mitglieder", 0, "capabilities", "contribution_aux", "unsupported")],
     [v("/mitglieder/0/capabilities/contribution_aux",
        f"{S}/capabilities_beitrag/properties/contribution_aux/const", "const"),
      v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_beitrag/properties/plugin_kind/maxLength", "maxLength")],
     "eine Beitragsaussage setzt den diskreten Contribution-Aux-Bus nachweislich "
     "voraus; bei unsupported gilt nur Assoziation statt exakter Attribution (§53.6)"),

    # Die Gegenrichtung. Sie ist kein Gate-7-Bruch — eine Beitragsposition, die
    # sich schwaecher nennt, behauptet zu wenig statt zu viel —, aber sie macht
    # das Feld mehrdeutig: zwei Sender beschrieben dieselbe Topologie mit
    # verschiedenen Klassen. §32.2 ordnet zu, es raeumt keinen Spielraum ein.
    ("beitragsposition-beobachtend", "session_snapshot",
     [setze("mitglieder", 0, "measurement_position", "post_fader_contribution")],
     [v("/mitglieder/0/aussageklasse",
        f"{S}/probe_descriptor_beitrag/properties/aussageklasse/const", "const"),
      v("/mitglieder/0/plugin_kind",
        f"{S}/probe_descriptor_beitrag/properties/plugin_kind/maxLength", "maxLength")],
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

    # --- NAK-29: sechs bedingte Transportrelationen ----------------------
    ("transport-project-samples-ohne-project-sample-start", "evidence_snapshot",
     [loesche("transport", "project_sample_start")],
     [v("/transport", f"{S}/transportstempel/oneOf/0/required/project_sample_start",
        "required")],
     "project_samples verlangt einen expliziten Projektstart"),

    ("transport-project-samples-ohne-project-time", "evidence_snapshot",
     [setze("transport", "validity", "project_time", False)],
     [v("/transport/validity/project_time",
        f"{S}/transportstempel/oneOf/0/properties/validity/properties/project_time/const",
        "const")],
     "project_samples verlangt project_time=true"),

    ("transport-local-monotonic-mit-project-time", "evidence_snapshot",
     [setze("transport", "time_basis", "local_monotonic"),
      loesche("transport", "project_sample_start")],
     [v("/transport/validity/project_time",
        f"{S}/transportstempel/oneOf/1/properties/validity/properties/project_time/const",
        "const")],
     "local_monotonic verlangt project_time=false"),

    ("transport-local-monotonic-mit-project-sample-start", "evidence_snapshot",
     [setze("transport", "time_basis", "local_monotonic"),
      setze("transport", "validity", "project_time", False)],
     [v("/transport/project_sample_start",
        f"{S}/transportstempel/oneOf/1/properties/project_sample_start/type", "type")],
     "lokale Zeit darf keine Projektposition behaupten"),

    ("cycle-bounds-valid-ohne-start-ppq", "evidence_snapshot",
     [setze("transport", "cycle", {"active": True, "bounds_valid": True,
                                    "end_ppq": 2.0})],
     [v("/transport/cycle", f"{S}/cycle/oneOf/0/required/start_ppq", "required")],
     "bounds_valid=true verlangt die linke PPQ-Grenze"),

    ("cycle-bounds-valid-ohne-end-ppq", "evidence_snapshot",
     [setze("transport", "cycle", {"active": True, "bounds_valid": True,
                                    "start_ppq": 1.0})],
     [v("/transport/cycle", f"{S}/cycle/oneOf/0/required/end_ppq", "required")],
     "bounds_valid=true verlangt die rechte PPQ-Grenze"),

    ("cycle-bounds-invalid-mit-validated-block-mapping", "evidence_snapshot",
     [setze("transport", "cycle", {"active": False, "bounds_valid": False,
                                    "derived_sample_bounds": {"start": 1, "end": 2,
                                      "derivation": "validated_block_mapping"}})],
     [v("/transport/cycle/derived_sample_bounds/derivation",
        f"{S}/cycle/oneOf/1/properties/derived_sample_bounds/properties/derivation/const",
        "const")],
     "bounds_valid=false darf keine validierte Sampleableitung behaupten"),

    ("validity-cycle-bounds-ohne-start-ppq", "evidence_snapshot",
     [setze("transport", "validity", "cycle_bounds", True),
      setze("transport", "cycle", {"active": False, "bounds_valid": False,
                                    "end_ppq": 2.0})],
     [v("/transport/cycle",
        f"{S}/transportstempel/oneOf/0/oneOf/0/properties/cycle/required/start_ppq",
        "required")],
     "cycle_bounds-Bit bescheinigt beide rohen PPQ-Grenzen"),

    ("validity-cycle-bounds-ohne-end-ppq", "evidence_snapshot",
     [setze("transport", "validity", "cycle_bounds", True),
      setze("transport", "cycle", {"active": False, "bounds_valid": False,
                                    "start_ppq": 1.0})],
     [v("/transport/cycle",
        f"{S}/transportstempel/oneOf/0/oneOf/0/properties/cycle/required/end_ppq",
        "required")],
     "cycle_bounds-Bit bescheinigt beide rohen PPQ-Grenzen"),

    ("validity-continuous-time-ohne-wert", "evidence_snapshot",
     [setze("transport", "validity", "continuous_time", True)],
     [v("/transport",
        f"{S}/transportstempel/oneOf/0/oneOf/1/oneOf/0/required/continuous_time_samples",
        "required")],
     "continuous_time-Bit verlangt den optionalen Samplewert"),

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

    ("host-mixer-index-null", "session_snapshot",
     [setze("mitglieder", 0, "host_mixer_index", 0)],
     [v("/mitglieder/0/host_mixer_index", f"{S}/host_mixer_index/minimum", "minimum")],
     "VST3 kChannelIndexKey beginnt bei 1"),

    ("host-mixer-index-negativ", "session_snapshot",
     [setze("mitglieder", 0, "host_mixer_index", -1)],
     [v("/mitglieder/0/host_mixer_index", f"{S}/host_mixer_index/minimum", "minimum")],
     "negative Hostindizes gelten als nicht geliefert"),

    ("host-bus-name-leer", "session_snapshot",
     [setze("mitglieder", 0, "host_bus_name", "")],
     [v("/mitglieder/0/host_bus_name", f"{S}/host_bus_name/minLength", "minLength"),
      v("/mitglieder/0/host_bus_name", f"{S}/host_bus_name/pattern", "pattern")],
     "ein leerer Hostname gilt als nicht geliefert"),

    ("host-bus-name-nur-whitespace", "session_snapshot",
     [setze("mitglieder", 0, "host_bus_name", " \u00a0\u3000")],
     [v("/mitglieder/0/host_bus_name", f"{S}/host_bus_name/pattern", "pattern")],
     "Unicode-Whitespace allein ist kein sichtbarer Busname"),

    ("host-bus-name-121-codepoints", "session_snapshot",
     [setze("mitglieder", 0, "host_bus_name", "😀" * 121)],
     [v("/mitglieder/0/host_bus_name", f"{S}/host_bus_name/maxLength", "maxLength")],
     "121 Unicode-Codepoints liegen einen ueber der Annahmegrenze"),

    ("host-bus-name-c0-steuerzeichen", "session_snapshot",
     [setze("mitglieder", 0, "host_bus_name", "Bus\u001fA")],
     [v("/mitglieder/0/host_bus_name", f"{S}/host_bus_name/pattern", "pattern")],
     "C0-Steuerzeichen werden nicht als Hosttext angenommen"),

    ("host-bus-name-c1-steuerzeichen", "session_snapshot",
     [setze("mitglieder", 0, "host_bus_name", "Bus\u009fA")],
     [v("/mitglieder/0/host_bus_name", f"{S}/host_bus_name/pattern", "pattern")],
     "C1-Steuerzeichen werden nicht als Hosttext angenommen"),

    ("host-mixer-namespace-drittes-feld", "session_snapshot",
     [setze("mitglieder", 0, "host_mixer_namespace", "FL")],
     [v("/mitglieder/0/host_mixer_namespace",
        f"{S}/probe_descriptor_insert/additionalProperties", "additionalProperties")],
     "E-H02 reserviert kein drittes Namespacefeld; bei Mehrdeutigkeit fehlt der Index"),

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
     [vm("/mitglieder/0/frische", f"{S}/frische/required/stale", "required"),
      v("/mitglieder/0/frische", f"{S}/frische/required/stale", "required")],
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
                                   "start_ppq": 1.0, "end_ppq": 2.0,
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
     [v("/transport/validity", f"{S}/validity/required/continuous_time", "required"),
      v("/transport/validity/continuous_time",
        f"{S}/transportstempel/oneOf/0/oneOf/1/oneOf", "oneOf")],
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

    # --- SONDE-013: Ereignisstrom -----------------------------------------
    ("evidence-ereignisse-ohne-verlustzaehler", "evidence_snapshot",
     [setze("ereignisse", {"liste": [copy.deepcopy(EREIGNIS)]})],
     [v("/ereignisse", f"{S}/evidence_ereignisse/required/verloren", "required")],
     "M-05: der Ring zaehlt seine Verluste. Ohne den Zaehler kann ein Empfaenger "
     "'nichts passiert' nicht von 'verdraengt' unterscheiden"),

    ("evidence-ereignisse-ueber-64", "evidence_snapshot",
     [setze("ereignisse", {"liste": [copy.deepcopy(EREIGNIS) for _ in range(65)],
                           "verloren": 0})],
     [v("/ereignisse/liste",
        f"{S}/evidence_ereignisse/properties/liste/maxItems", "maxItems")],
     "M-05/§48.1: der Ereignisring ist fest auf 64 Plaetze gedeckelt; ein 65. "
     "Eintrag ist ein Senderfehler (Grenztest an N+1)"),

    ("evidence-ereignis-ohne-peakbit", "evidence_snapshot",
     [setze("ereignisse", {"liste": [{k: val for k, val in EREIGNIS.items()
                                      if k != "qualitaet_peak"}],
                           "verloren": 0})],
     [v("/ereignisse/liste/0",
        f"{S}/dynamics_ereignis/required/qualitaet_peak", "required")],
     "M-86: die zwei Qualitaetsbits trennen den Flusspfad vom eigenstaendigen "
     "Peakpfad. Ein Ereignis ohne beide Bits sagt nicht, welcher Ausloeser feuerte"),

    ("evidence-ereignis-bandzentrum-null", "evidence_snapshot",
     [setze("ereignisse", {"liste": [dict(EREIGNIS, band_zentrum_hz=0)],
                           "verloren": 0})],
     [v("/ereignisse/liste/0/band_zentrum_hz",
        f"{S}/dynamics_ereignis/properties/band_zentrum_hz/exclusiveMinimum",
        "exclusiveMinimum")],
     "0 Hz ist kein Bandzentrum; die Grenze ist ausschliessend, nicht einschliessend"),

    # --- SONDE-013: bandweise Stereoevidenz -------------------------------
    ("evidence-stereo-ohne-freiheitsgrade", "evidence_snapshot",
     [setze("stereo", copy.deepcopy(STEREO)),
      loesche("stereo", "freiheitsgrade")],
     [v("/stereo", f"{S}/stereo_evidenz/required/freiheitsgrade", "required")],
     "§40.1 woertlich: 'Fensterdauer und Freiheitsgrade werden Teil der Evidenz'. "
     "Ohne sie kann ein Empfaenger eine null-Kohaerenz nicht begruenden"),

    ("evidence-stereo-falsche-gitter", "evidence_snapshot",
     [setze("stereo", copy.deepcopy(STEREO)),
      setze("stereo", "mid_db", "gitter_id", "nakama_log64_v1"),
      setze("stereo", "korrelation_kurz", "gitter_id", "nakama_log64_v1"),
      setze("stereo", "phase_rad", "gitter_id", "nakama_log64_v1")],
     [v("/stereo/mid_db/gitter_id",
        f"{S}/stereo_bandwerte/properties/gitter_id/const", "const"),
      v("/stereo/korrelation_kurz/gitter_id",
        f"{S}/stereo_bandwerte_normiert/properties/gitter_id/const", "const"),
      v("/stereo/phase_rad/gitter_id",
        f"{S}/stereo_bandwerte_phase/properties/gitter_id/const", "const")],
     "Die Stereoevidenz liegt auf dem 221er-Evidenzgitter. Das 64er-Livegitter "
     "hier zuzulassen waere ein zweites Gitter fuer dieselbe Aussage"),

    ("evidence-stereo-werte-ausserhalb", "evidence_snapshot",
     [setze("stereo", copy.deepcopy(STEREO)),
      setze("stereo", "mid_db", "werte", 0, 500.0),
      setze("stereo", "korrelation_kurz", "werte", 0, 1.5),
      setze("stereo", "phase_rad", "werte", 0, 4.0)],
     [v("/stereo/mid_db/werte/0",
        f"{S}/stereo_bandwerte/properties/werte/items/maximum", "maximum"),
      v("/stereo/korrelation_kurz/werte/0",
        f"{S}/stereo_bandwerte_normiert/properties/werte/items/maximum", "maximum"),
      v("/stereo/phase_rad/werte/0",
        f"{S}/stereo_bandwerte_phase/properties/werte/items/maximum", "maximum")],
     "M-11: eine Korrelation ueber 1 und eine Phase ausserhalb +/-pi sind "
     "Erzeugerfehler und faellen auf der Leitung, nicht erst in der Anzeige"),

    ("evidence-stereo-zusatzfeld", "evidence_snapshot",
     [setze("stereo", copy.deepcopy(STEREO)),
      setze("stereo", "breite", 0.5)],
     [v("/stereo/breite", f"{S}/stereo_evidenz/additionalProperties",
        "additionalProperties")],
     "V-03: `breite` und `korrelation` bleiben der 10-Hz-Livepfad (§33.2). Sie "
     "hier zusaetzlich zu fuehren erzeugte eine zweite Wahrheit ueber dieselbe Groesse"),

    # --- SONDE-013: Experimentfamilien ------------------------------------
    ("experiment-begin-fremder-execution-mode", "experiment_begin",
     [setze("execution_mode", "active_probe")],
     [v("/execution_mode",
        f"{S}/experiment_begin/properties/execution_mode/const", "const")],
     "Der Active-Compare-Pfad gehoert SONDE-017. Ihn hier als Zweig vorzusehen "
     "waere ein Vorgriff (Bauaufteilung §6.2); ihn spaeter zu ergaenzen ist ein "
     "ausdruecklicher Fassungsschritt, den ein Leser der alten Fassung ablehnt"),

    ("experiment-begin-behauptet-wiederherstellbarkeit", "experiment_begin",
     [setze("reproduzierbarkeit", "automatisch_wiederherstellbar")],
     [v("/reproduzierbarkeit",
        f"{S}/experiment_begin/properties/reproduzierbarkeit/const", "const")],
     "M-42: ein manueller Versuch erlaubt KEIN Nakama-Revert. Eine hoehere "
     "Reproduzierbarkeitsklasse zu behaupten waere eine Luege ueber den Rueckweg"),

    # ── Nacharbeit 1 (Befund B23): die vollstaendige Passage ─────────────
    #
    # Vorher trug `experiment_begin` nur `passage_id` und die Referenz. Kein
    # gueltiger Wire-Aufruf konnte damit das von M-25 verlangte Storeobjekt
    # erzeugen - `Experimentstore::beginne` braucht Grenzen, Transportepoche,
    # Quellen mit Messpunktklasse, Abdeckung und Fingerprint. Jedes dieser
    # Felder faellt hier EINZELN.
    ("experiment-begin-ohne-passage", "experiment_begin",
     [loesche("passage")],
     [v("", f"{S}/experiment_begin/required/passage", "required")],
     "M-25/B23: ohne Passageobjekt kann der Broker keine Passage anlegen, und "
     "der Versuch haette kein Fenster, auf das er sich bezieht"),

    ("experiment-begin-passage-ohne-grenzen", "experiment_begin",
     [loesche("passage", "projekt_bis")],
     [v("/passage", f"{S}/passage/required/projekt_bis", "required")],
     "M-25: die Grenzen in Projektsamples sind der Kern der Passage. Ohne sie "
     "waere sie ein Zeitfenster ohne Zeit"),

    ("experiment-begin-passage-ohne-transportepoche", "experiment_begin",
     [loesche("passage", "transport_epoch")],
     [v("/passage", f"{S}/passage/required/transport_epoch", "required")],
     "§32.4: eine Passage bindet an GENAU EINE Transportepoche. Ohne sie waere "
     "ein Fenster ueber einen Seek hinweg dieselbe Zahl wie eines darin"),

    ("experiment-begin-passage-ohne-fingerprint", "experiment_begin",
     [loesche("passage", "fingerprint")],
     [v("/passage", f"{S}/passage/required/fingerprint", "required")],
     "M-26: ohne Fingerprint gibt es keinen Materialbeleg, und ein fehlender "
     "Beleg ist keine Aehnlichkeit"),

    ("experiment-begin-passage-abdeckung-ueber-eins", "experiment_begin",
     [setze("passage", "abdeckung", 1.5)],
     [v("/passage/abdeckung", f"{S}/passage/properties/abdeckung/maximum", "maximum")],
     "M-30: die Abdeckung ist ein Anteil in [0, 1]. Ein Wert darueber waere "
     "keine Messung, sondern ein Rechenfehler beim Sender"),

    ("experiment-begin-passage-fremde-messpunktklasse", "experiment_begin",
     [setze("passage", "messpunktklassen", ["seitenkette"])],
     [v("/passage/messpunktklassen/0",
        f"{S}/passage/properties/messpunktklassen/items/enum", "enum")],
     "M-55: die Messpunktklassen sind eine GESCHLOSSENE Menge. Eine fremde "
     "Klasse waere ein Messpunkt, den keine Invalidierungsregel kennt"),

    ("experiment-begin-passage-leeres-label", "experiment_begin",
     [setze("passage", "label", "")],
     [v("/passage/label", f"{S}/passage/properties/label/minLength", "minLength")],
     "M-25: `null` heisst 'kein Userwort'. Eine leere Zeichenkette waere eine "
     "zweite Schreibweise dafuer, und zwei Schreibweisen fuer denselben "
     "Zustand sind der Anfang von Sonderfaellen"),

    ("experiment-begin-ohne-referenz", "experiment_begin",
     [loesche("referenz")],
     [v("", f"{S}/experiment_begin/required/referenz", "required")],
     "M-40: Baseline und Referenzen werden VOR dem Begin verriegelt. Ein Begin "
     "ohne sie waere ein Versuch ohne rekonstruierbaren Ausgangspunkt"),

    ("experiment-referenz-ohne-match-gain", "experiment_begin",
     [loesche("referenz", "match_gain_db")],
     [v("/referenz", f"{S}/experiment_referenz/required/match_gain_db", "required")],
     "M-43: eine Klangwertung ohne vorherigen Lautheitsabgleich ist unzulaessig; "
     "der Match-Gain gehoert zu den unveraenderlichen Referenzen"),

    ("experiment-referenz-fremde-messpunktklasse", "experiment_begin",
     [setze("referenz", "messpunktklassen", 0, "sidechain")],
     [v("/referenz/messpunktklassen/0",
        f"{S}/experiment_referenz/properties/messpunktklassen/items/enum", "enum")],
     "§32.2 kennt genau vier Messpositionen; eine fuenfte waere eine "
     "Aussageklasse ohne Gate-7-Kopplung"),

    # Nacharbeit 3 (Befund C5): die zwei Kanten des neuen Feldes.
    ("experiment-referenz-negative-nicht-endliche-samples", "experiment_begin",
     [setze("referenz", "nicht_endliche_samples", -1)],
     [v("/referenz/nicht_endliche_samples",
        f"{S}/experiment_referenz/properties/nicht_endliche_samples/minimum", "minimum")],
     "M-07, Nacharbeit 3 (C5): eine negative Anzahl gibt es nicht - sie waere "
     "die stille Umdeutung von 'nicht gemessen' in eine Zahl"),
    ("experiment-referenz-bruchzahl-nicht-endliche-samples", "experiment_begin",
     [setze("referenz", "nicht_endliche_samples", 2.5)],
     [v("/referenz/nicht_endliche_samples",
        f"{S}/experiment_referenz/properties/nicht_endliche_samples/type", "type")],
     "M-07, Nacharbeit 3 (C5): 2,5 Samples gibt es nicht - eine stille Rundung "
     "waere eine erfundene Zahl"),
    ("experiment-referenz-fremde-alignmentklasse", "experiment_begin",
     [setze("referenz", "alignment", "sample_aligned")],
     [v("/referenz/alignment", f"{S}/alignment_klasse/enum", "enum")],
     "M-16: genau vier Klassen. `sample_aligned` behauptete eine "
     "Samplegenauigkeit, die §38.2 ausdruecklich NICHT zusagt"),

    ("experiment-referenz-fingerprint-zu-kurz", "experiment_begin",
     [setze("referenz", "passage_fingerprint", "chroma", [0] * 11)],
     [v("/referenz/passage_fingerprint/chroma",
        f"{S}/fingerprint/properties/chroma/minItems", "minItems")],
     "M-26: der Fingerprint hat eine feste Stuetzstellenzahl. Ein kuerzerer "
     "Verlauf waere ein anderer Fingerprint mit demselben Namen"),

    ("experiment-abort-fremder-grund", "experiment_abort",
     [setze("grund", "sitzungsende")],
     [v("/grund", f"{S}/experiment_abort/properties/grund/enum", "enum")],
     "M-47/E-03: genau ZWEI Ausloeser. Sitzungsende, Reconnect und Neustart "
     "brechen ausdruecklich NICHT ab - ein offener Versuch ueberdauert sie"),

    ("experiment-manual-result-fremdes-urteil", "experiment_manual_result",
     [setze("hoerurteil", "objektiv_besser")],
     [v("/hoerurteil",
        f"{S}/experiment_manual_result/properties/hoerurteil/enum", "enum")],
     "M-46: nie 'objektiv besser' allein aus einem Metrikdelta. Das Hoerurteil "
     "ist Userdatum und traegt genau die vier zulaessigen Werte"),

    ("experiment-manual-result-fremde-blindreihenfolge", "experiment_manual_result",
     [setze("blindreihenfolge", "unbekannt")],
     [v("/blindreihenfolge",
        f"{S}/experiment_manual_result/properties/blindreihenfolge/enum", "enum")],
     "M-44: die Reihenfolge wird VOR dem Urteil gebunden und danach aufgedeckt. "
     "'unbekannt' erlaubte, sie nachtraeglich zum Urteil passend zu erzaehlen"),

    ("experiment-manual-result-mit-state-hash", "experiment_manual_result",
     [setze("state_hash", "d" * 64)],
     [v("/state_hash", f"{S}/experiment_manual_result/additionalProperties",
        "additionalProperties")],
     "E-02: die Familie traegt KEINE State-Hashes, keine Candidate-Revision und "
     "keinen Match-Gain. Im manuellen Modus existiert kein lesbarer Fremdzustand"),

    ("experiment-manual-result-notiz-leer", "experiment_manual_result",
     [setze("notiz", "")],
     [v("/notiz", f"{S}/experiment_manual_result/properties/notiz/minLength",
        "minLength")],
     "`null` heisst 'keine Notiz'. Eine leere Zeichenkette waere eine zweite "
     "Schreibweise dafuer - dieselbe Regel wie bei pair_id"),

    # --- SONDE-013 Nacharbeit 2: Bezug, Kandidat und Rueckweg ------------

    ("audible-intervention-begin-experiment-id-kein-hex32", "audible_intervention_begin",
     [setze("intervention_id", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab"),
      setze("event_sequence", 5),
      setze("art", "experiment"),
      # Der GROSSGESCHRIEBENE Zwilling der echten ID - derselbe Wert, nur
      # eine zweite Schreibweise. Genau die soll es nicht geben.
      setze("experiment_id", "AB" * 16),
      setze("project_sample_start", 44108200)],
     [v("/experiment_id", f"{S}/hex32/pattern", "pattern")],
     "Kleinbuchstaben, damit ein Vergleich nie normalisieren muss"),

    ("experiment-candidate-blindreihenfolge-erfunden", "experiment_candidate",
     [setze("blindreihenfolge", "zufaellig")],
     [v("/blindreihenfolge",
        f"{S}/experiment_candidate/properties/blindreihenfolge/enum", "enum")],
     "genau zwei Reihenfolgen; eine dritte waere ein Etikett ohne Bedeutung"),

    ("experiment-candidate-ohne-blindreihenfolge", "experiment_candidate",
     [loesche("blindreihenfolge")],
     [v("", f"{S}/experiment_candidate/required/blindreihenfolge", "required")],
     "ohne gebundene Blindreihenfolge ist der Schritt keiner - die Reihenfolge "
     "waere danach frei waehlbar (M-44)"),

    ("session-experiment-ereignis-erfunden", "session_snapshot",
     [setze("experimente", [dict(SESSION_VERSUCH, ereignis="vielleicht")]),
      setze("paare", copy.deepcopy(SESSION_PAARE))],
     [v("/experimente/0/ereignis",
        f"{S}/session_experiment/properties/ereignis/enum", "enum")],
     "die Transitionen sind eine geschlossene Menge; eine erfundene waere ein "
     "Etikett ohne Wirkung"),

    ("session-experiment-hoerurteil-erfunden", "session_snapshot",
     [setze("experimente", [dict(SESSION_VERSUCH, hoerurteil="ziemlich_gut")]),
      setze("paare", copy.deepcopy(SESSION_PAARE))],
     [v("/experimente/0/hoerurteil",
        f"{S}/session_experiment/properties/hoerurteil/enum", "enum")],
     "M-45: vier Urteile, und `enthaltung` ist ein vollwertiges - ein fuenftes "
     "gibt es nicht"),

    ("session-experiment-blindreihenfolge-erfunden", "session_snapshot",
     [setze("experimente", [dict(SESSION_VERSUCH, blindreihenfolge="zufaellig")]),
      setze("paare", copy.deepcopy(SESSION_PAARE))],
     [v("/experimente/0/blindreihenfolge",
        f"{S}/session_experiment/properties/blindreihenfolge/enum", "enum")],
     "genau zwei Reihenfolgen; eine dritte waere keine Blindprobe mehr"),

    ("session-experiment-vergleichbarkeit-erfunden", "session_snapshot",
     [setze("experimente", [dict(SESSION_VERSUCH, vergleichbarkeit="sehr_stark")]),
      setze("paare", copy.deepcopy(SESSION_PAARE))],
     [v("/experimente/0/vergleichbarkeit",
        f"{S}/session_experiment/properties/vergleichbarkeit/enum", "enum")],
     "M-30: drei Stufen. Eine vierte waere eine Steigerung, die kein Gate kennt"),

    ("session-experiment-urteil-erfunden", "session_snapshot",
     [setze("experimente", [dict(SESSION_VERSUCH, urteil="objektiv_besser")])],
     [v("/experimente/0/urteil",
        f"{S}/session_experiment/properties/urteil/enum", "enum")],
     "M-46: genau fuenf zulaessige Aussagen, und 'objektiv besser' ist keine "
     "davon - auch nicht auf dem Rueckweg"),

    ("session-experiment-ohne-offen", "session_snapshot",
     [setze("experimente", [{k: v_ for k, v_ in SESSION_VERSUCH.items()
                             if k != "offen"}])],
     [v("/experimente/0", f"{S}/session_experiment/required/offen", "required")],
     "ohne das Offenbit weiss Gen nicht, ob es ein Ergebnis oder einen Zwischen"
     "stand sieht - Abwesenheit ist hier keine Aussage"),

    ("session-paar-kettenbefund-erfunden", "session_snapshot",
     [setze("paare", [dict(SESSION_PAARE[0], kettenbefund="klingt_gut")])],
     [v("/paare/0/kettenbefund",
        f"{S}/session_paar/properties/kettenbefund/enum", "enum")],
     "M-22: fuenf Kettenbefunde. Ein sechster waere eine Aussage ohne Regel"),

    ("session-paar-ausschluss-erfunden", "session_snapshot",
     [setze("experimente", [copy.deepcopy(SESSION_VERSUCH)]),
      setze("paare", [copy.deepcopy(SESSION_PAARE[0]),
                      dict(SESSION_PAARE[1], ausschluss="keine_lust")])],
     [v("/paare/1/ausschluss",
        f"{S}/session_paar/properties/ausschluss/enum", "enum")],
     "der Ausschlussgrund ist geschlossen; ein freier Text waere ein stiller "
     "Ausschluss mit Deckmantel"),
]


# ------------------------------------------------------------------ Mutationen

def hole(daten, pfad):
    for teil in pfad:
        daten = daten[teil]
    return daten


def wende_an(daten, mutation):
    """Wendet EINE Mutation auf eine bereits tiefkopierte Grundform an.

    ⚠️ Der eingesetzte Wert wird ebenfalls TIEFKOPIERT. Ohne das teilen sich
    zwei Fixtures, die denselben Baustein einsetzen (`setze("ziel",
    EXPERIMENT_ZIEL)`), dasselbe Objekt — und die naechste Mutation an dem
    einen erscheint im anderen. Gemessen am 06.09.2026 (SONDE-014): zwanzig
    Fassung-3-Fixtures trugen die Felder ihrer Nachbarn, weil die Grundform
    kopiert wurde, der eingesetzte Baustein aber nicht.
    """
    art = mutation[0]
    if art == "ersetze":
        return copy.deepcopy(mutation[2])
    pfad = mutation[1]
    if art == "loesche":
        eltern = hole(daten, pfad[:-1])
        del eltern[pfad[-1]]
    elif art == "setze":
        eltern = hole(daten, pfad[:-1])
        eltern[pfad[-1]] = copy.deepcopy(mutation[2])
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
        text = als_text(minor_1_sessionform(copy.deepcopy(daten))).decode("utf-8")
        if alt not in text:
            raise SystemExit(f"Rohtext-Fixture: {alt!r} steht nicht in den Daten")
        return text.replace(alt, neu, 1).encode("utf-8")

    def aus(grundform: str, alt: str, neu: str) -> bytes:
        return aus_daten(GRUND[grundform], alt, neu)

    float32 = copy.deepcopy(GRUND["evidence_snapshot"])
    float32["baender"] = baender(221, "nakama_1_24_oct_30_18k_v1", "float32")
    float32["baender"]["werte"][0] = 0.5

    # SONDE-014 Etappe I (M-82): ein Sitzungsschnitt MIT Befund - nur dort
    # stehen Maskierungswert und Konfidenzscore.
    befund_zahlen = copy.deepcopy(GRUND["session_snapshot"])
    befund_zahlen["findings"] = [SESSION_FINDING]

    host_index = copy.deepcopy(GRUND["heartbeat"])
    host_index["runtime"] = {
        "messpunkt": "insert", "betrieb": "active", "host_mixer_index": 1,
    }

    return [
        ("zahl-ueber-2hoch53",
         aus("heartbeat", '"sequence": 91', '"sequence": 9007199254740992'),
         "2^53 ist die erste ganze Zahl, die binary64 nicht mehr exakt traegt"),

        ("host-mixer-index-ueber-json-sicher",
         aus_daten(host_index, '"host_mixer_index": 1',
                   '"host_mixer_index": 9007199254740992'),
         "E-H02-Grenzfall: 2^53 faellt bereits am gemeinsamen Textriegel, bevor "
         "C++, Rust oder Python daraus verschiedene Zahlen machen koennen"),

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

        ("maskierungswert-nicht-endlich",
         aus_daten(befund_zahlen, '"wert_db": -3.25', '"wert_db": 1e400'),
         "SONDE-014 M-82: 1e400 ist als binary64 unendlich. Der Maskierungswert "
         "traegt eine MESSUNG - ein Band ohne Messung traegt 0 mit "
         "gueltig=false, nie NaN und nie Unendlichkeit. Der Riegel faellt VOR "
         "jedem Parser, damit keine der drei Engines sie erst deuten muss"),

        ("befund-score-nicht-endlich",
         aus_daten(befund_zahlen, '"score": 0.68', '"score": 1e400'),
         "SONDE-014 M-82: dasselbe am Konfidenzscore. Er ist im Vertrag auf "
         "[0,1] beschraenkt; eine nicht-endliche Sicherheit ist keine Sicherheit, "
         "und ein Leser, der sie klemmt, machte aus Unendlichkeit eine 1"),

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


# ════════════════════════════════════════════════════════════════════════
# NAK-313 Etappe 4 · der strenge Parselauf (R-313-6) und die Tabelle der
# Produkteingaenge (Manifest docs/beweise/NAK-313.md §7)
# ════════════════════════════════════════════════════════════════════════
#
# Der Textriegel ist ein Tokenpruefer, kein Parser: Nachspann, zweites
# Dokument, Schlusskomma, unbekannte Escapes und doppelte Namen passieren ihn.
# Danach entschieden bis NAK-313 die Parser der Beine, und die waren sich nicht
# einig - JUCE nahm alle fuenf Formen an, serde_json und Pythons json lehnten
# die Syntax ab und behielten beim Duplikat still den letzten Wert. Seit
# Etappe 4 liest jeder Eingang genau einmal streng: C++ `nakama::kanon::lies`,
# Rust `json_streng`, Python `json_laden_strikt` mit Duplikat-Hook, alle drei
# mit derselben Tiefengrenze.
#
# Zwei Erzeugnisse gehoeren dazu:
#   · die MANIFEST-Klasse `parser_lehnt_ab` (`ungueltig/parser-*.json`): ROHBYTES
#     von Hand, nie ueber `als_text` - ein Serialisierer schriebe kein Duplikat,
#     keinen Nachspann und kein `\q`, der Defekt waere weg;
#   · `PRODUKTEINGAENGE-FAELLE.json`: dieselben Bytes und die Vektoren der
#     uebrigen Produkteingaenge, hex-kodiert, je Eintrag mit dem Urteil des
#     Vertrags und dem des Produktlesers (Form A, R-313-13). Beide Urteile
#     stehen VON HAND hier; kein Leser erzeugt sie, sonst waere der Vergleich
#     zirkulaer.

MAX_TIEFE = 64   # dieselbe Grenze in kanon::lies, json_streng und json_laden_strikt

FREMDE_SID = "S-1-5-21-9"
FREMDE_EPOCHE = "55555555555555555555555555555555"
V2_WELCOME = '{"type":"welcome","protocol_version":2,"broker_version":"test","session_token":"tok"}'

# NAK-313 Etappe 5 (Manifest §8.5, „Testserver fuer Tabelleneintraege"): die
# Werte, die der v3-Testserver (`eq-copilot/plugin/tests/V3TestServer.h`) selbst
# traegt. Die Welcome-Eintraege nennen seine Kopplungswerte, damit der
# Kopplungsvergleich des Telemetrie-Clients nicht am Eintrag haengt; die
# ACK-Eintraege tragen die `command_id` des P0, den das Bein einreiht.
ACK_KOPPLUNG = {"link_id": "a" * 32, "challenge": "b" * 32, "broker_epoch": "c" * 32}
ACK_COMMAND_ID = "5" * 32

# Die geschlossenen Mengen der Tabelle (§7.1).
STUFEN = ["textriegel", "parser", "duplikat", "schema", "feldregel"]
WIRKUNGEN = ["annahme", "ablehnung", "keine_teilmutation", "kein_ack",
             "kein_freigegebener_auftrag"]

# Welche Matrixzeilen zu welcher Bauetappe gehoeren (Manifest §6.7). Ein
# Eintrag legt seine Etappe ueber `matrix` fest; die Stufe, die er verlangt,
# muss die Kette seines Lesers zum Ende GENAU DIESER Etappe schon haben.
MATRIX_ETAPPEN = [(1, 16, 2), (17, 38, 3), (39, 54, 4), (55, 97, 5),
                  (98, 122, 6), (123, 145, 7)]

# Die Stufenketten je Leser und Etappe (Manifest §7.2, Tabelle „Stufenketten
# je Leser"). Referenzbeine messen `vertrag`, Produktleser `produkt`. Etappe 5
# aendert keine Kette, nur den Inhalt zweier Stufen: `textriegel` des
# v2-Clients traegt ab jetzt den Zahlriegel, `feldregel` der flachen Leser den
# Bereich des Ganzzahllesers (Manifest §7.1, „Die Stufen"). Etappe 6 gibt dem
# Bootstrap die Stufe schema (das Tor, Manifest §8.6: Textriegel, strenger
# Lauf, Hello-Pruefung des Schemas, Uebernahme); die flachen Leser behalten
# ihre Kette, ihr parser traegt ab Etappe 6 UTF-8, Escapes und den
# Grund-Ausgang.
_KETTEN_ETAPPE_4 = {
    "A5":  ["textriegel", "parser", "duplikat", "schema"],
    "A11": ["parser", "duplikat", "schema"],
    "B16": ["textriegel", "parser", "duplikat", "schema"],
    "cpp_control_handshake":    ["parser", "duplikat", "feldregel"],
    "cpp_telemetrie_handshake": ["parser", "duplikat", "feldregel"],
    "cpp_control_ack":          ["parser", "duplikat", "feldregel"],
    "cpp_sources_snapshot":   ["textriegel", "parser", "duplikat", "feldregel"],
    "cpp_sources_ruecknahme": ["textriegel", "parser", "duplikat", "feldregel"],
    "cpp_v2_client":          ["textriegel", "parser", "duplikat", "feldregel"],
    "rust_bootstrap": ["textriegel", "parser", "duplikat", "feldregel"],
    "rust_p0": ["textriegel", "parser", "duplikat", "schema", "feldregel"],
    "rust_p1": ["textriegel", "parser", "duplikat", "schema", "feldregel"],
}
_KETTEN_ETAPPE_6 = {**_KETTEN_ETAPPE_4,
                    "rust_bootstrap": ["textriegel", "parser", "duplikat", "schema", "feldregel"]}
LESERKETTEN = {leser: {"4": list(kette), "5": list(kette), "6": list(_KETTEN_ETAPPE_6[leser])}
               for leser, kette in _KETTEN_ETAPPE_4.items()}


def _einspeisung_register() -> dict:
    """Das Register der Produkteingaenge (Manifest §7.2) samt Einspeisung.

    Die Einspeisewerte stehen maschinenlesbar hier und nicht im Eintrag: ein
    Eintrag mit Nachspann oder Schlusskomma traegt keine lesbare Adresse, und
    ein Bein, das sie trotzdem aus ihm zoege, laese ihn mit einem zweiten,
    nachsichtigen Parser. Alle Eintraege eines Eingangs sind Varianten
    derselben Grundform, deshalb gilt ein Satz Werte je Eingang.
    """
    sitzung = {
        "project_binding_id": ADRESSE["project_binding_id"],
        "session_epoch": GRUND["session_snapshot"]["session_epoch"],
        "eigenes_main": GRUND["session_snapshot"]["fuehrendes_main"],
        "basis_fixture": "gueltig/session_snapshot.json",
    }
    return {
        "cpp_control_handshake": {
            "produktleser": "flacher Leser Control, erste Antwort welcome oder reject",
            "bein": "B10", "einspeisung": {
                "text": ("der Testserver sendet die Bytes als erste P0-Antwort (Schalter fuer "
                         "rohe Antwortbytes auf das Control-Hello)")}},
        "cpp_telemetrie_handshake": {
            "produktleser": "flacher Leser Telemetrie",
            "bein": "B10", "einspeisung": {
                "text": ("der Testserver sendet die Bytes auf das Telemetrie-Hello (Schalter fuer "
                         "rohe Antwortbytes auf das Telemetrie-Hello); link_id und challenge "
                         "der Bytes sind die Kopplungswerte des Testservers"),
                "link_id": ACK_KOPPLUNG["link_id"], "challenge": ACK_KOPPLUNG["challenge"]}},
        "cpp_control_ack": {
            "produktleser": "commandAckArtLesen ueber inFlightAck und commandAckHaeltVertrag",
            "bein": "B10", "einspeisung": {
                "text": ("ein persistenzpflichtiger P0 (user_verdict) mit command_id, der "
                         "Testserver antwortet mit den Bytes (Schalter fuer rohe ACK-Bytes)"),
                "command_id": ACK_COMMAND_ID}},
        "cpp_sources_snapshot": {
            "produktleser": "SourcesModel::uebernehmeSessionSnapshot",
            "bein": "B13", "einspeisung": {
                "text": ("direkter Aufruf mit der aktiven Fassung; vorher beginneSubscription "
                         "mit Bindung, Sitzung und eigenem Main und die Uebernahme von "
                         "basis_fixture"), **sitzung}},
        "cpp_sources_ruecknahme": {
            "produktleser": "SourcesModel::uebernehmeEvidenzruecknahme",
            "bein": "B13", "einspeisung": {
                "text": "direkter Aufruf, dieselbe abonnierte Sitzung wie cpp_sources_snapshot",
                **sitzung}},
        "cpp_v2_client": {
            "produktleser": "PipeClient (welcome, reject, heartbeat_ack)",
            "bein": "A4b", "einspeisung": {
                "text": ("ein v2-Peer auf einem Probenamen sendet die Bytes als Antwort auf "
                         "hello; fuer heartbeat_ack zuerst welcome_vor_ack, dann so viele "
                         "gueltige ACKs (seq = Nummer des Heartbeats), wie "
                         "einspeisung.gueltige_acks_davor des Eintrags nennt (fehlt es: 0), "
                         "dann die Bytes als Antwort auf den naechsten Heartbeat"),
                "welcome_vor_ack": V2_WELCOME}},
        "rust_bootstrap": {
            "produktleser": "bootstrap_lesen",
            "bein": "A4", "einspeisung": {
                "text": "der Test setzt das Laengenpraefix davor"}},
        "rust_p0": {
            "produktleser": "Coordinator::p0_json_mit_minor",
            "bein": "A4", "einspeisung": {
                "text": "ein Link mit dieser Adresse (control_registrieren), dann die Bytes",
                "link_art": "active_probe", "adresse": ADRESSE}},
        "rust_p1": {
            "produktleser": "P1-Weiche p1_mit_minor und die Leser dahinter",
            "bein": "A4", "einspeisung": {
                "text": ("je Nachricht ein Link dieser Art und Adresse, angemeldet und mit "
                         "einem Heartbeatkontakt, dann die Bytes"),
                "links": {
                    "evidence_snapshot": {"link_art": "passive_probe", "adresse": ADRESSE},
                    "intent_update": {"link_art": "main", "adresse": ADRESSE},
                    "state_report": {"link_art": "passive_probe", "adresse": ADRESSE},
                }}},
    }


def _grundtext(daten: dict) -> str:
    """Die Grundform als Text, genau wie der Korpus sie unter `gueltig/` schreibt."""
    return als_text(minor_1_sessionform(copy.deepcopy(daten))).decode("utf-8")


def _ersetze(text: str, alt: str, neu: str) -> str:
    if alt not in text:
        raise SystemExit(f"Produkteingaenge: {alt!r} steht nicht im Grundtext")
    return text.replace(alt, neu, 1)


def _kompakt(daten) -> str:
    return json.dumps(daten, ensure_ascii=False, separators=(",", ":"))


def _listenkette(ebenen: int) -> list:
    """Eine Liste, die `ebenen` Listen tief verschachtelt ist (innen leer)."""
    wert: list = []
    for _ in range(ebenen - 1):
        wert = [wert]
    return wert


def _doppelt(text: str, zeile: str, schluessel: str, fremd: str,
             einzug: str) -> tuple[str, str]:
    """Die zwei Duplikatvarianten der ersten Zeile `zeile` im Text.

    (passender Wert zuletzt, fremder Wert zuletzt) - ein Parser mit „letzter
    gewinnt" liest im ersten Fall den gueltigen Wert, im zweiten den fremden.
    `einzug` ist die Einrueckung der Zeile, damit die Variante lesbar bleibt.
    """
    fremd_zeile = f'"{schluessel}": {fremd},'
    return (_ersetze(text, zeile, fremd_zeile + "\n" + einzug + zeile),
            _ersetze(text, zeile, zeile + "\n" + einzug + fremd_zeile))


def parser_faelle() -> list[tuple[str, bytes, str, str]]:
    """(name, rohbytes, stufe, warum) - die MANIFEST-Klasse `parser_lehnt_ab`.

    Die Varianten des gueltigen `session_snapshot` aus Manifest §8.4: Nachspann,
    zweites Dokument, Schlusskomma, unbekanntes Escape (Stufe `parser`),
    `type`, `session_epoch` und `logon_sid` doppelt in beiden Reihenfolgen und
    der Escape-Alias fuer `type` (Stufe `duplikat`). Dieselben Bytes fahren die
    Produktleser als Eintraege von `cpp_sources_snapshot` (M-39, M-48).
    """
    text = _grundtext(GRUND["session_snapshot"])
    typ = '"type": "session_snapshot",'
    epoche = GRUND["session_snapshot"]["session_epoch"]
    sid = f'"logon_sid": "{ADRESSE["logon_sid"]}",'
    typ_p, typ_f = _doppelt(text, typ, "type", '"x"', "  ")
    # Die ERSTE Fundstelle ist die Wurzel: ihre Schluessel stehen vor `mitglieder`.
    ep_p, ep_f = _doppelt(text, f'"session_epoch": "{epoche}",', "session_epoch",
                          f'"{FREMDE_EPOCHE}"', "  ")
    # Die ERSTE Fundstelle ist die Adresse der Mitgliedshuelle.
    sid_p, sid_f = _doppelt(text, sid, "logon_sid", f'"{FREMDE_SID}"', "        ")
    faelle = [
        ("nachspann", text + "[]\n", "parser",
         "Nachspann: hinter dem Dokument steht ein zweiter Wert; JUCE las ihn nicht, "
         "serde_json und Python lehnen ab"),
        ("zweites-dokument", text + text, "parser",
         "zwei vollstaendige Dokumente hintereinander sind kein Dokument"),
        ("schlusskomma", _ersetze(text, "\n  ]\n}\n", "\n  ],\n}\n"), "parser",
         "Schlusskomma im Wurzelobjekt: RFC 8259 kennt es nicht, JUCE nahm es an"),
        ("unbekanntes-escape",
         _ersetze(text, '"label": "Klavier-Bus"', '"label": "Klavier' + BS + 'q-Bus"'),
         "parser",
         "unbekanntes Escape im Label: RFC 8259 §7 kennt nur acht Zwei-Zeichen-Escapes, "
         "JUCE machte aus dem Backslash-q ein q"),
        ("type-doppelt-passend-zuletzt", typ_p, "duplikat",
         "Discriminator doppelt, der passende Wert zuletzt: letzter gewinnt las ihn als gueltig"),
        ("type-doppelt-fremd-zuletzt", typ_f, "duplikat",
         "Discriminator doppelt, ein fremder Wert zuletzt"),
        ("session-epoch-doppelt-passend-zuletzt", ep_p, "duplikat",
         "Sitzung doppelt, die erwartete zuletzt: der Sitzungsvergleich sah nur den letzten Wert"),
        ("session-epoch-doppelt-fremd-zuletzt", ep_f, "duplikat",
         "Sitzung doppelt, eine fremde zuletzt"),
        ("logon-sid-doppelt-passend-zuletzt", sid_p, "duplikat",
         "Adressfeld doppelt in einer Mitgliedsadresse, der kanonische Wert zuletzt"),
        ("logon-sid-doppelt-fremd-zuletzt", sid_f, "duplikat",
         "Adressfeld doppelt in einer Mitgliedsadresse, ein fremder Wert zuletzt"),
        ("type-escape-alias",
         _ersetze(text, typ, '"typ' + BS + 'u0065": "session_snapshot",\n  ' + typ),
         "duplikat",
         "Escape-Alias: typ mit escaptem e dekodiert zu type - derselbe Name zweimal"),
    ]
    return [(name, t.encode("utf-8"), stufe, warum) for name, t, stufe, warum in faelle]


def _evidenz_ohne_projektzeit() -> dict:
    """DASSELBE gueltige Fixture wie `gueltig/evidence-ohne-projektzeit.json`.

    Aus `zusatz_gueltig()` gelesen statt nachgebaut: Byteinstanz und
    Negativfixture sollen aus genau diesem Dokument entstehen, nicht aus einer
    zweiten Kopie, die von ihm weglaufen koennte.
    """
    for name, daten, _warum in zusatz_gueltig():
        if name == "evidence-ohne-projektzeit":
            return copy.deepcopy(daten)
    raise SystemExit("evidence-ohne-projektzeit fehlt in zusatz_gueltig()")


# Der Transportblock, den `transportJson` (NakamaEvidenz.cpp) fuer einen Frame
# OHNE Projektzeit schreibt: Kontext anwesend, Spielzustand gueltig und
# spielend, keine Aufnahme-, Zyklus-, Continuous- oder Latenzangabe - genau die
# Flags des Speisers in B16 ohne `kFlagZeitGueltig`. Die Reihenfolge der
# Schluessel ist die des Writers; `project_sample_start` fehlt (R-313-7).
LOKAL_VALIDITY = {
    "project_time": False,
    "play_state": True,
    "record_state": False,
    "cycle_bounds": False,
    "continuous_time": False,
    "input_presentation_latency": False,
    "output_presentation_latency": False,
}


def _lokaler_transportblock(eingabe: dict) -> str:
    """Der Transportblock in Schluesselreihenfolge und Zahlform des Writers."""
    def wahr(w: bool) -> str:
        return "true" if w else "false"

    teile = [
        f'"transport_epoch":{eingabe["transport_epoch"]}',
        f'"continuity_segment":{eingabe["continuity_segment"]}',
        f'"sequence":{eingabe["sequence"]}',
        '"process_context_present":true',
        '"time_basis":"local_monotonic"',
        f'"sample_count":{eingabe["sample_count"]}',
        f'"sample_rate":{wire_zahl(float(eingabe["sample_rate"]))}',
        '"playing":true',
        '"recording":false',
        '"validity":{' + ",".join(f'"{k}":{wahr(v)}' for k, v in LOKAL_VALIDITY.items()) + "}",
    ]
    return "{" + ",".join(teile) + "}"


def _lokale_byteinstanz() -> tuple[dict, str, str]:
    """(eingabe, wire_transport, wire_snapshot) der lokalen Evidenz."""
    basis = _evidenz_ohne_projektzeit()
    t = basis["transport"]
    eingabe = {k: t[k] for k in ("transport_epoch", "continuity_segment", "sequence",
                                 "sample_count", "sample_rate")}
    wire_transport = _lokaler_transportblock(eingabe)
    basis["transport"] = json.loads(wire_transport)
    wire_snapshot = _kompakt(basis)
    # Selbstpruefung: der Block steht BYTEGLEICH im Snapshot, sonst verglichen
    # B16 und A4 zwei verschiedene Texte.
    if '"transport":' + wire_transport + "," not in wire_snapshot:
        raise SystemExit("Byteinstanz: der Transportblock steht nicht bytegleich im Snapshot")
    return eingabe, wire_transport, wire_snapshot


def evidenz_lokal_wire() -> bytes:
    """`evidenz-lokal-wire-v1.json` - die Byteinstanz des lokalen Evidenzwriters.

    NAK-313 R-313-7 (T3-03-01): ohne gueltige Projektzeit schrieb
    `transportJson` `"project_sample_start":null`, der Vertrag laesst aber nur
    das Weglassen zu (Zweig `local_monotonic` verlangt `null`, die gemeinsame
    Eigenschaft `integer` - beide gelten). Diese Instanz ist die dritte Partei
    zwischen dem echten C++-Writer (B16 vergleicht seinen Transportblock
    bytegleich mit `wire_transport`) und dem echten Rust-Empfaenger (A4 speist
    `wire_snapshot` in die P1-Weiche). Keiner der beiden erzeugt sie.
    """
    eingabe, wire_transport, wire_snapshot = _lokale_byteinstanz()
    return als_text({
        "_kommentar": [
            "NAK-313 R-313-7 - die BYTEINSTANZ des lokalen Evidenzwriters.",
            "",
            "`eingabe` traegt die Transportzahlen, die B16 in den Stempel eines",
            "echten Frames ohne Projektzeit setzt; `wire_transport` ist der Block,",
            "den transportJson daraus schreiben muss - Schluesselreihenfolge und",
            "Zahlform des Writers, OHNE project_sample_start; `wire_snapshot` ist",
            "gueltig/evidence-ohne-projektzeit.json mit genau diesem Block, den A4",
            "durch die P1-Weiche in den Evidenzempfaenger speist.",
        ],
        "eingabe": eingabe,
        "wire_transport": wire_transport,
        "wire_snapshot": wire_snapshot,
    })


# ── NAK-313 Etappe 5b · die Handinstanz des Intentwriters ────────────────
#
# R-313-4 (M-89, E-313-14): alle persistenten Revisionen und ihre Drahtform
# enden bei 2^53-1. Diese Instanz haelt den Rand fuer den Intent-Bestand: der
# echte C++-Writer (B27, v3IntentUpdateJson ueber v3IntentUpdateFuerTest)
# wird ab dem Feld bestand_revision bytegleich gegen wire_ausschnitt verglichen,
# der echte Rust-Leser (A4) nimmt wire ueber die P1-Weiche an.

REVISION_MAX = 9_007_199_254_740_991   # 2^53-1, v3-Textriegel Regel 2


def _intent_byteinstanz() -> tuple[dict, str, str]:
    """(eingabe, wire_ausschnitt, wire) des leeren Bestands am Revisionsrand."""
    eingabe = {
        "adresse": dict(ADRESSE),
        "bestand": {
            "intent_revision_v1": REVISION_MAX,
            "source_intents_v1": [],
            "intent_protections_v1": [],
            "intent_relations_v1": [],
        },
        "vollstaendig": True,
    }
    # Schluesselreihenfolge und Zahlform des Writers: Kopf, Adresse in der
    # Reihenfolge von adresseAlsJson, Sitzungsepoche, Revision als
    # Zifferntext, die Marke; ein leerer Bestand traegt keine Liste.
    ausschnitt = f',"bestand_revision":{REVISION_MAX},"vollstaendig":true}}'
    wire = ('{"type":"intent_update","adresse":' + _kompakt(eingabe["adresse"])
            + ',"session_epoch":' + json.dumps(eingabe["adresse"]["session_epoch"])
            + ausschnitt)
    # Selbstpruefung: der Ausschnitt schliesst wire ab, und wire traegt die
    # Revision als exakte Ganzzahl in der Feldfolge des Writers.
    geparst = json.loads(wire)
    if (not wire.endswith(ausschnitt)
            or list(geparst) != ["type", "adresse", "session_epoch", "bestand_revision", "vollstaendig"]
            or list(geparst["adresse"]) != list(ADRESSE)
            or geparst["bestand_revision"] != REVISION_MAX
            or type(geparst["bestand_revision"]) is not int):
        raise SystemExit("Handinstanz intent-wire-v1.json: Wiretext und Ausschnitt passen nicht")
    return eingabe, ausschnitt, wire


def intent_wire() -> bytes:
    """intent-wire-v1.json - die Handinstanz des Intentwriters am Rand 2^53-1.

    NAK-313 R-313-4 (M-89, E-313-14): Ausgabe keiner Implementierung. B27 laedt
    einen Main-Stand mit leerem Bestand und der Revision aus eingabe und
    vergleicht den Writertext ab dem Feld bestand_revision bytegleich mit
    wire_ausschnitt; Adresse und session_epoch sind dort Laufzeitwerte und
    stehen nicht im Vergleich. A4 koppelt einen Link mit der Adresse aus
    eingabe, speist wire in die P1-Weiche und erwartet die
    Bestandsrevision 2^53-1.
    """
    eingabe, ausschnitt, wire = _intent_byteinstanz()
    return als_text({
        "_kommentar": [
            "NAK-313 R-313-4 - die HANDINSTANZ des Intentwriters am Revisionsrand 2^53-1.",
            "",
            "`eingabe` traegt den Bestand, den B27 in einen Main-Stand laedt (leer,",
            "intent_revision_v1 = 2^53-1), und die feste Adresse, mit der A4 einen Link",
            "koppelt. `wire` ist der Text, den v3IntentUpdateJson fuer genau diese",
            "Adresse schreiben muss; `wire_ausschnitt` ist sein Ende ab",
            "`,\"bestand_revision\":` - bis dorthin stehen im Writertext Laufzeitwerte",
            "(Logon-SID, Sitzungsepoche, Laufzeitnonce), deshalb vergleicht B27 nur",
            "den Ausschnitt. Keine der beiden Sprachen erzeugt diese Datei.",
        ],
        "eingabe": eingabe,
        "wire_ausschnitt": ausschnitt,
        "wire": wire,
    })


def handinstanzen_register() -> dict:
    """Der MANIFEST-Eintrag der Handinstanzen, die dieses Register fuehrt."""
    return {
        "zweck": ("Byteinstanzen NEBEN gueltig/, von Hand ausgeschriebene Wiretexte: "
                  "ein echter Writer wird gegen sie bytegleich verglichen, ein echter "
                  "Leser nimmt sie an. Der Erzeuger schreibt sie, A8 haelt sie "
                  "bytegleich, ihr SHA-256 steht hier. Registriert seit NAK-313 "
                  "Etappe 5b (M-89, E-313-14); die aelteren Byteinstanzen neben "
                  "gueltig/ fuehrt dieses Register nicht."),
        "dateien": [{
            "datei": "intent-wire-v1.json",
            "sha256": hashlib.sha256(intent_wire()).hexdigest(),
            "warum": ("R-313-4: der Intentwriter schreibt bestand_revision 2^53-1 "
                      "unveraendert (B27), der Rust-Leser uebernimmt sie (A4)."),
        }],
    }


def lokale_evidenz_negativ() -> list[tuple[str, dict, list[dict], str]]:
    """Das Negativfixture mit `project_sample_start: null` (M-53).

    Dasselbe gueltige Fixture wie oben, `null` direkt hinter `time_basis` -
    dort, wo der Writer es vor NAK-313 schrieb. Der Zweig `local_monotonic`
    nimmt `null` an; die GEMEINSAME Eigenschaft (`integer`) nicht. Die
    Verletzung liegt deshalb an ihr, nicht am Zweig - anders als beim
    Negativfixture mit Zahl (`transport-local-monotonic-mit-project-sample-start`).
    """
    daten = _evidenz_ohne_projektzeit()
    neu: dict = {}
    for feld, wert in daten["transport"].items():
        neu[feld] = wert
        if feld == "time_basis":
            neu["project_sample_start"] = None
    daten["transport"] = neu
    return [("evidence-local-monotonic-project-sample-start-null", daten,
             [v("/transport/project_sample_start",
                f"{S}/transportstempel/properties/project_sample_start/type", "type")],
             "NAK-313 R-313-7: ohne Projektzeit wird project_sample_start weggelassen, "
             "nie null - die gemeinsame Eigenschaft verlangt integer")]


def _pe(eingang: str, fassung: str, nachricht: str, roh: str | bytes, zeige: str,
        stufe: str | None, wirkung: list[str], matrix: str, warum: str) -> dict:
    """Ein Tabelleneintrag der Etappe 4: Vertrag und Produkt urteilen gleich.

    Etappe 4 hat keine Abweichung: jeder ihrer Eintraege passiert die
    Textstufen jeder Kette, und jeder ihrer Leser hat den strengen Lauf
    (Manifest §7.3). Ein Eintrag mit Abweichung setzt `abweichung` selbst.
    """
    roh_bytes = roh if isinstance(roh, bytes) else roh.encode("utf-8")
    urteil = {"urteil": "gueltig" if stufe is None else "ungueltig", "stufe": stufe}
    return {
        "eingang": eingang,
        "fassung": fassung,
        "nachricht": nachricht,
        "bytes_hex": roh_bytes.hex(),
        "zeigetext": zeige,
        "vertrag": dict(urteil),
        "produkt": dict(urteil),
        "abweichung": None,
        "feld": None,
        "wert": None,
        "wirkung": wirkung,
        "matrix": matrix,
        "ub_bei_juce": False,
        "einspeisung": None,
        "warum": warum,
    }


ABGELEHNT = ["ablehnung", "keine_teilmutation"]


def _faelle_quellenmodell() -> list[dict]:
    """M-39 und M-41 (`cpp_sources_snapshot`), M-40 (`cpp_sources_ruecknahme`)."""
    faelle = []
    for name, roh, stufe, warum in parser_faelle():
        faelle.append(_pe("cpp_sources_snapshot", "v3", "session_snapshot", roh,
                          f"session_snapshot, Variante {name}", stufe, ABGELEHNT,
                          "M-39", warum))
    zwei = copy.deepcopy(GRUND["session_snapshot"])
    zweites = copy.deepcopy(PROBE)
    zweites["adresse"] = {**ADRESSE, "instance_id": "c" * 32, "runtime_nonce": "d" * 32}
    zweites["label"] = "Bass-Bus"
    zwei["mitglieder"] = [copy.deepcopy(PROBE), zweites]
    faelle.append(_pe("cpp_sources_snapshot", "v3", "session_snapshot",
                      als_text(minor_1_sessionform(zwei)),
                      "session_snapshot mit zwei Mitgliedern, beide Adressen tragen logon_sid",
                      None, ["annahme"], "M-41",
                      "Gegenfall: derselbe Name in zwei verschiedenen Objekten ist gueltig"))

    ruecknahme = {"type": "evidence_invalidate", "grund": "sequenzluecke",
                  "umfang": {"art": "evidence_ids",
                             "evidence_ids": ["99999999999999999999999999999999"]}}
    text = als_text(ruecknahme).decode("utf-8")
    typ = '"type": "evidence_invalidate",'
    typ_p, typ_f = _doppelt(text, typ, "type", '"x"', "  ")
    grund_p, grund_f = _doppelt(text, '"grund": "sequenzluecke",', "grund",
                                '"gibt_es_nicht"', "  ")
    art_p, art_f = _doppelt(text, '"art": "evidence_ids",', "art", '"sample_range"', "    ")
    varianten = [
        ("nachspann", text + "[]\n", "parser", "Nachspann hinter der Ruecknahme"),
        ("zweites-dokument", text + text, "parser", "zwei Dokumente hintereinander"),
        ("schlusskomma", _ersetze(text, "\n  }\n}\n", "\n  },\n}\n"), "parser",
         "Schlusskomma im Wurzelobjekt"),
        ("unbekanntes-escape",
         _ersetze(text, '"sequenzluecke"', '"se' + BS + 'quenzluecke"'), "parser",
         "unbekanntes Escape: JUCE las daraus den bekannten Grund sequenzluecke"),
        ("type-doppelt-passend-zuletzt", typ_p, "duplikat", "Discriminator doppelt, passend zuletzt"),
        ("type-doppelt-fremd-zuletzt", typ_f, "duplikat", "Discriminator doppelt, fremd zuletzt"),
        ("grund-doppelt-passend-zuletzt", grund_p, "duplikat",
         "Grund doppelt, der bekannte zuletzt"),
        ("grund-doppelt-fremd-zuletzt", grund_f, "duplikat",
         "Grund doppelt, ein unbekannter zuletzt"),
        ("art-doppelt-passend-zuletzt", art_p, "duplikat",
         "Umfangsart doppelt, die zu den Feldern passende zuletzt"),
        ("art-doppelt-fremd-zuletzt", art_f, "duplikat",
         "Umfangsart doppelt, eine Art mit fehlenden Bereichsfeldern zuletzt"),
        ("type-escape-alias",
         _ersetze(text, typ, '"typ' + BS + 'u0065": "evidence_invalidate",\n  ' + typ),
         "duplikat", "Escape-Alias fuer type"),
    ]
    for name, roh, stufe, warum in varianten:
        faelle.append(_pe("cpp_sources_ruecknahme", "v3", "evidence_invalidate", roh,
                          f"evidence_invalidate, Variante {name}", stufe, ABGELEHNT,
                          "M-40", warum))
    return faelle


def _faelle_v2_client() -> list[dict]:
    """M-42 (`cpp_v2_client`): Welcome und das ACK auf den ersten Heartbeat."""
    w = V2_WELCOME
    welcome = [
        ("nachspann", w + "[]", "parser", "Nachspann hinter dem Welcome"),
        ("schlusskomma", w[:-1] + ",}", "parser", "Schlusskomma im Welcome"),
        ("unbekanntes-escape", w.replace('"test"', '"te' + BS + 'qst"'), "parser",
         "unbekanntes Escape in broker_version"),
        ("type-doppelt-passend-zuletzt", w.replace('"type":"welcome"', '"type":"x","type":"welcome"'),
         "duplikat", "Discriminator doppelt, welcome zuletzt"),
        ("type-doppelt-fremd-zuletzt", w.replace('"type":"welcome"', '"type":"welcome","type":"x"'),
         "duplikat", "Discriminator doppelt, fremd zuletzt"),
        ("protocol-version-doppelt-passend-zuletzt",
         w.replace('"protocol_version":2', '"protocol_version":1,"protocol_version":2'),
         "duplikat", "Protokollversion doppelt, 2 zuletzt"),
        ("protocol-version-doppelt-fremd-zuletzt",
         w.replace('"protocol_version":2', '"protocol_version":2,"protocol_version":1'),
         "duplikat", "Protokollversion doppelt, 1 zuletzt"),
    ]
    faelle = [_pe("cpp_v2_client", "v2", "welcome", roh, roh, stufe, ABGELEHNT, "M-42", warum)
              for _name, roh, stufe, warum in welcome]
    ack = [
        ('{"type":"heartbeat_ack","seq":7,"seq":0,"konflikt":false}',
         "Sequenz doppelt, die passende 0 zuletzt"),
        ('{"type":"heartbeat_ack","seq":0,"seq":7,"konflikt":false}',
         "Sequenz doppelt, eine fremde zuletzt"),
    ]
    faelle += [_pe("cpp_v2_client", "v2", "heartbeat_ack", roh, roh, "duplikat",
                   ["ablehnung", "kein_ack"], "M-42", warum) for roh, warum in ack]
    return faelle


def _faelle_rust() -> list[dict]:
    """M-43 und M-49 (`rust_p0`), M-44, M-49 und M-51 bis M-53 (`rust_p1`),
    M-45 (`rust_bootstrap`)."""
    faelle = []
    sid = f'"logon_sid": "{ADRESSE["logon_sid"]}",'
    fremd_sid = f'"{FREMDE_SID}"'

    hb = _grundtext(GRUND["heartbeat"])
    typ = '"type": "heartbeat",'
    typ_p, typ_f = _doppelt(hb, typ, "type", '"x"', "  ")
    sid_p, sid_f = _doppelt(hb, sid, "logon_sid", fremd_sid, "    ")
    p0 = [
        ("type doppelt, heartbeat zuletzt", typ_p, "duplikat"),
        ("type doppelt, fremd zuletzt", typ_f, "duplikat"),
        ("logon_sid doppelt in adresse, kanonisch zuletzt", sid_p, "duplikat"),
        ("logon_sid doppelt in adresse, fremd zuletzt", sid_f, "duplikat"),
        ("Escape-Alias fuer type",
         _ersetze(hb, typ, '"typ' + BS + 'u0065": "heartbeat",\n  ' + typ), "duplikat"),
        ("Nachspann", hb + "[]\n", "parser"),
        ("zweites Dokument", hb + hb, "parser"),
        ("Schlusskomma im Wurzelobjekt",
         _ersetze(hb, '"intervention_state_unknown": false\n}\n',
                  '"intervention_state_unknown": false,\n}\n'), "parser"),
        ("unbekanntes Escape im Schluessel sequence",
         _ersetze(hb, '"sequence": 91,', '"se' + BS + 'quence": 91,'), "parser"),
    ]
    for zeige, roh, stufe in p0:
        faelle.append(_pe("rust_p0", "v3", "heartbeat", roh, f"heartbeat, {zeige}", stufe,
                          ["ablehnung", "kein_ack", "keine_teilmutation"], "M-43",
                          f"Heartbeat mit {zeige}: der strenge Lauf lehnt vor dem Schema ab"))
    # M-49: Gesamttiefe 64 und 65 - Wurzel, `zaehler` (additiv) und darin
    # 62 beziehungsweise 63 Listen. Objekte und Listen zaehlen zusammen.
    for tiefe in (MAX_TIEFE, MAX_TIEFE + 1):
        daten = copy.deepcopy(GRUND["heartbeat"])
        daten["zaehler"]["verschachtelt"] = _listenkette(tiefe - 2)
        gueltig = tiefe <= MAX_TIEFE
        faelle.append(_pe("rust_p0", "v3", "heartbeat", _kompakt(daten),
                          f"heartbeat, Gesamttiefe {tiefe} (Listen im additiven zaehler)",
                          None if gueltig else "parser",
                          ["annahme"] if gueltig else ["ablehnung", "kein_ack", "keine_teilmutation"],
                          "M-49",
                          f"Tiefe {tiefe}: " + ("die Grenze selbst ist gueltig" if gueltig
                                                else "eine Ebene ueber der Grenze von 64")))

    ev = _grundtext(GRUND["evidence_snapshot"])
    ev_typ = '"type": "evidence_snapshot",'
    ev_p, ev_f = _doppelt(ev, ev_typ, "type", '"x"', "  ")
    intent = _grundtext(GRUND["intent_update"])
    epoche = GRUND["intent_update"]["session_epoch"]
    # Die ERSTE Fundstelle ist die Adresse: sie steht vor der Wurzelepoche.
    in_p, in_f = _doppelt(intent, f'"session_epoch": "{epoche}",', "session_epoch",
                          f'"{FREMDE_EPOCHE}"', "    ")
    p1 = [
        ("evidence_snapshot", "type doppelt, evidence_snapshot zuletzt", ev_p),
        ("evidence_snapshot", "type doppelt, fremd zuletzt", ev_f),
        ("intent_update", "session_epoch doppelt in adresse, passend zuletzt", in_p),
        ("intent_update", "session_epoch doppelt in adresse, fremd zuletzt", in_f),
    ]
    for nachricht, zeige, roh in p1:
        faelle.append(_pe("rust_p1", "v3", nachricht, roh, f"{nachricht}, {zeige}", "duplikat",
                          ABGELEHNT, "M-44",
                          f"{nachricht} mit {zeige}: abgelehnt vor Weiche und Schema"))
    for tiefe in (MAX_TIEFE, MAX_TIEFE + 1):
        daten = copy.deepcopy(GRUND["evidence_snapshot"])
        daten["konfidenz"]["verschachtelt"] = _listenkette(tiefe - 2)
        gueltig = tiefe <= MAX_TIEFE
        faelle.append(_pe("rust_p1", "v3", "evidence_snapshot", _kompakt(daten),
                          f"evidence_snapshot, Gesamttiefe {tiefe} (Listen in der additiven konfidenz)",
                          None if gueltig else "parser",
                          ["annahme"] if gueltig else ABGELEHNT, "M-49",
                          f"Tiefe {tiefe}: " + ("die Grenze selbst ist gueltig" if gueltig
                                                else "eine Ebene ueber der Grenze von 64")))
    _eingabe, _block, snapshot = _lokale_byteinstanz()
    faelle.append(_pe("rust_p1", "v3", "evidence_snapshot", snapshot,
                      "wire_snapshot der Byteinstanz: local_monotonic ohne project_sample_start",
                      None, ["annahme"], "M-52",
                      "lokale Evidenz ohne Startwert ist vertragsgueltig und wird angenommen"))
    mit_null = _ersetze(snapshot, '"time_basis":"local_monotonic",',
                        '"time_basis":"local_monotonic","project_sample_start":null,')
    faelle.append(_pe("rust_p1", "v3", "evidence_snapshot", mit_null,
                      "local_monotonic mit project_sample_start null", "schema", ABGELEHNT,
                      "M-53", "null verletzt die gemeinsame Eigenschaft integer"))

    hello = _grundtext(GRUND["hello_control"])
    h_typ = '"type": "hello",'
    h_typ_p, h_typ_f = _doppelt(hello, h_typ, "type", '"x"', "  ")
    h_sid_p, h_sid_f = _doppelt(hello, sid, "logon_sid", fremd_sid, "    ")
    bootstrap = [
        ("logon_sid doppelt in adresse, erst fremd, dann kanonisch", h_sid_p),
        ("logon_sid doppelt in adresse, erst kanonisch, dann fremd", h_sid_f),
        ("type doppelt, hello zuletzt", h_typ_p),
        ("type doppelt, fremd zuletzt", h_typ_f),
        ("Escape-Alias fuer protocol",
         _ersetze(hello, '"protocol": 3,', '"protoco' + BS + 'u006c": 3,\n  "protocol": 3,')),
    ]
    for zeige, roh in bootstrap:
        faelle.append(_pe("rust_bootstrap", "v3", "hello_control", roh,
                          f"Control-Hello, {zeige}", "duplikat", ["ablehnung"], "M-45",
                          f"Hello mit {zeige}: KeinJson mit der Marke des doppelten Schluessels"))
    return faelle


# ════════════════════════════════════════════════════════════════════════
# NAK-313 Etappe 5a · die Wertregel am Wire (R-313-5; Manifest §7.3, „Etappe 5")
# ════════════════════════════════════════════════════════════════════════
#
# Ganzzahlfelder in der `.0`- und der `e`-Form mit ihrem erwarteten Wert, die
# Negativen `1.5` und 2^53 fuer die v3-Eingaenge, NaN, +/-Infinity und 1e999 in
# einem Ganzzahlfeld und die v2-Zahleintraege des Zahlriegels. Ab hier urteilen
# Vertrag und Produkt nicht mehr ueberall an derselben Stufe: wo ein Leser weder
# Textriegel noch Schema hat (flacher Leser, Quellenmodell ohne Schema,
# v2-Client mit Zahlriegel statt Vertrag), steht die Abweichung mit Grund im
# Eintrag und im Kopf gezaehlt (R-313-13). Die Bootstrap-Negativen entstehen
# erst mit dem Tor in Etappe 6 (E-313-11); hier stehen nur gueltige Hellos.

ZWEI_HOCH_53 = "9007199254740992"
NICHT_ENDLICH = ("NaN", "Infinity", "-Infinity", "1e999")
V2_GANZZAHL_MAX = "9223372036854775807"

# Der Stand der Spezifikation nach Etappe 6 (Manifest §7.1, §8.6): genau eine
# Abweichung der Art `urteil` (M-93 c); die Art `stufe` tragen die Eintraege
# der Leser ohne Schema oder ohne Textriegel - nach Etappe 5 ACK 6, Handshake
# Control 8 und Telemetrie 8, Quellenmodell 2 und 2, v2-Client 11 (37); dazu in
# Etappe 6 das Tor 5 (protocol 3.5, NaN, Infinity, -Infinity, 1e999), Handshake
# Control 10, Telemetrie 5 und ACK 1 (21). Von Hand gezaehlt; die
# Selbstpruefung haelt die Tabelle dagegen.
ABWEICHUNGEN_SPEZIFIZIERT = {"stufe": 58, "urteil": 1}

# Die Gruende der Abweichungen der Art `stufe` - je Produktregel ein Satz mit
# ihrer Stelle, damit ein Leser des Eintrags weiss, warum Vertrag und Produkt
# an verschiedenen Stufen fallen.
GRUND_FLACH_ACK = ("der flache Leser hat weder Textriegel noch Schema: commandAckArtLesen "
                   "liest state_revision mit ganzzahlAusLiteral im Bereich 0 bis 2^53-1 "
                   "(core/ipc/controlclient/Intern.h), ein Fehlschlag ist keinAck")
GRUND_FLACH_WELCOME = ("der flache Leser hat weder Textriegel noch Schema: welcomeHaeltVertrag "
                       "liest protocol mit ganzzahlAusLiteral im Bereich 3 bis 3 "
                       "(core/ipc/controlclient/Vertrag.cpp)")
GRUND_QUELLENMODELL = ("das Quellenmodell prueft kein Schema: der Feldleser "
                       "nichtnegativeGanzzahl (src/SourcesModel.cpp) nimmt nur ganzzahlige Werte "
                       "ab 0 aus dem strengen Lauf (wertAlsVar)")
GRUND_ZAHLRIEGEL = ("der v2-Client prueft keinen Vertrag: der Zahlriegel (zahlriegelBytes, "
                    "Regeln 1 bis 3 mit der Grenze INT64_MAX) lehnt vor dem strengen Lauf und "
                    "vor JUCE ab (src/PipeClient.cpp, empfange)")
GRUND_V2_FELD = ("der v2-Client prueft keinen Vertrag: die Zahl passiert den Zahlriegel, und "
                 "die Feldregel nach dem Lesen (protocol_version == 2, src/PipeClient.cpp) "
                 "lehnt ab")


def _stufe(grund: str) -> dict:
    return {"art": "stufe", "grund": grund}


def _pe5(eingang: str, fassung: str, nachricht: str, roh: str, zeige: str,
         vertrag: str | None, produkt: str | None, wirkung: list[str], matrix: str,
         warum: str, feld: str | None = None, wert: str | None = None,
         abweichung: dict | None = None, ub: bool = False,
         einspeisung: dict | None = None) -> dict:
    """Ein Tabelleneintrag der Etappe 5: Vertrag und Produkt getrennt (Form A)."""
    eintrag = _pe(eingang, fassung, nachricht, roh, zeige, vertrag, wirkung, matrix, warum)
    eintrag["produkt"] = {"urteil": "gueltig" if produkt is None else "ungueltig",
                          "stufe": produkt}
    eintrag.update(feld=feld, wert=wert, abweichung=abweichung, ub_bei_juce=ub,
                   einspeisung=einspeisung)
    return eintrag


def _ersetze_alle(text: str, alt: str, neu: str, anzahl: int) -> str:
    if text.count(alt) != anzahl:
        raise SystemExit(f"Produkteingaenge: {alt!r} steht nicht {anzahl}-mal im Grundtext")
    return text.replace(alt, neu)


def _ack_v3(literal: str) -> str:
    """Das ACK in der Form des Testservers, `state_revision` als Literal."""
    return ('{"type":"command_ack","command_id":"' + ACK_COMMAND_ID
            + '","ergebnis":"angewandt","state_revision":' + literal
            + ',"state_hash":"' + "d" * 64 + '"}')


def _welcome_v3(literal: str) -> str:
    """Das welcome in der Form des Testservers, `protocol` als Literal."""
    k = ACK_KOPPLUNG
    return ('{"type":"welcome","protocol":' + literal + ',"broker_version":"test",'
            '"broker_epoch":"' + k["broker_epoch"] + '","link_id":"' + k["link_id"]
            + '","challenge":"' + k["challenge"] + '"}')


def _faelle_flacher_leser() -> list[dict]:
    """M-56, M-57, M-60, M-73, M-96: ACK und welcome am flachen C++-Leser."""
    kein_ack = ["kein_ack", "kein_freigegebener_auftrag"]
    faelle = []
    for literal, wert, warum in (
            ("7.0", "7", "7.0 ist die mathematische Ganzzahl 7 (README Regel 2, draft 2020-12)"),
            ("7e0", "7", "7e0 ist die Ganzzahl 7"),
            ("0", "0", "die Untergrenze des Revisionsbereichs"),
            ("-0", "0", "-0 ist 0 und liegt im Bereich ab 0")):
        faelle.append(_pe5("cpp_control_ack", "v3", "command_ack", _ack_v3(literal),
                           f"command_ack angewandt, state_revision {literal}", None, None,
                           ["annahme"], "M-56", warum, feld="/state_revision", wert=wert))
    faelle.append(_pe5("cpp_control_ack", "v3", "command_ack", _ack_v3(ZWEI_HOCH_53),
                       "command_ack angewandt, state_revision 2^53", "textriegel", "feldregel",
                       kein_ack, "M-57",
                       "2^53 liegt ueber dem Revisionsbereich: keinAck, nie eine Revision",
                       feld="/state_revision", abweichung=_stufe(GRUND_FLACH_ACK)))
    faelle.append(_pe5("cpp_control_ack", "v3", "command_ack", _ack_v3("1.5"),
                       "command_ack angewandt, state_revision 1.5", "schema", "feldregel",
                       kein_ack, "M-73", "1.5 ist keine Ganzzahl", feld="/state_revision",
                       abweichung=_stufe(GRUND_FLACH_ACK)))
    for literal in NICHT_ENDLICH:
        faelle.append(_pe5("cpp_control_ack", "v3", "command_ack", _ack_v3(literal),
                           f"command_ack angewandt, state_revision {literal}", "textriegel",
                           "feldregel", kein_ack, "M-96",
                           f"{literal} ist kein Wert einer Ganzzahl: kein Leser liefert einen",
                           feld="/state_revision", abweichung=_stufe(GRUND_FLACH_ACK)))

    for eingang in ("cpp_control_handshake", "cpp_telemetrie_handshake"):
        for literal, warum in (("3.0", "3.0 ist die Ganzzahl 3 und haelt const 3"),
                               ("3e0", "3e0 ist die Ganzzahl 3"),
                               ("3", "die kanonische Form")):
            faelle.append(_pe5(eingang, "v3", "welcome", _welcome_v3(literal),
                               f"welcome, protocol {literal}", None, None, ["annahme"],
                               "M-60", warum, feld="/protocol", wert="3"))
        for literal, vertrag, matrix, warum in (
                ("4", "schema", "M-60", "4 verletzt const 3"),
                ("3.5", "schema", "M-60", "3.5 ist keine Ganzzahl"),
                ("1.5", "schema", "M-73", "1.5 ist keine Ganzzahl"),
                (ZWEI_HOCH_53, "textriegel", "M-73", "2^53 liegt ueber der Textriegel-Grenze")):
            faelle.append(_pe5(eingang, "v3", "welcome", _welcome_v3(literal),
                               f"welcome, protocol {literal}", vertrag, "feldregel",
                               ["ablehnung"], matrix, warum, feld="/protocol",
                               abweichung=_stufe(GRUND_FLACH_WELCOME)))
        for literal in NICHT_ENDLICH:
            faelle.append(_pe5(eingang, "v3", "welcome", _welcome_v3(literal),
                               f"welcome, protocol {literal}", "textriegel", "feldregel",
                               ["ablehnung"], "M-96",
                               f"{literal} ist kein Wert einer Ganzzahl: das welcome verbindet nicht",
                               feld="/protocol", abweichung=_stufe(GRUND_FLACH_WELCOME)))
    return faelle


def _faelle_quellenmodell_werte() -> list[dict]:
    """M-61, M-73, M-96: Werte aus dem strengen Lauf im Quellenmodell."""
    snapshot = _grundtext(GRUND["session_snapshot"])
    kontakt = '"letzter_kontakt_ms": 120'
    faelle = []
    feld = "/mitglieder/0/frische/letzter_kontakt_ms"
    for literal, wert in (("1500.0", "1500"), ("1.5e3", "1500")):
        faelle.append(_pe5("cpp_sources_snapshot", "v3", "session_snapshot",
                           _ersetze_alle(snapshot, kontakt, f'"letzter_kontakt_ms": {literal}', 2),
                           f"session_snapshot, letzter_kontakt_ms {literal} in Mitglied und Deskriptor",
                           None, None, ["annahme"], "M-61",
                           f"{literal} ist die Ganzzahl {wert}", feld=feld, wert=wert))
    faelle.append(_pe5("cpp_sources_snapshot", "v3", "session_snapshot",
                       _ersetze(snapshot, '"label": "Klavier-Bus",',
                                '"label": "Klavier-Bus",\n        "host_mixer_index": 3.0,'),
                       "session_snapshot, probe_descriptor.host_mixer_index 3.0", None, None,
                       ["annahme"], "M-61",
                       "E-313-9: der Mixerindex 3.0 ist die Ganzzahl 3, die Zeile traegt ihn",
                       feld="/mitglieder/0/probe_descriptor/host_mixer_index", wert="3"))
    for literal, vertrag, produkt, matrix, abw, warum in (
            ("1.5", "schema", "feldregel", "M-61", True, "1.5 ist keine Ganzzahl"),
            ("-1", "schema", "feldregel", "M-61", True, "-1 liegt unter der Untergrenze 0"),
            (ZWEI_HOCH_53, "textriegel", "textriegel", "M-73", False,
             "2^53 liegt ueber der Textriegel-Grenze")):
        faelle.append(_pe5("cpp_sources_snapshot", "v3", "session_snapshot",
                           _ersetze_alle(snapshot, kontakt, f'"letzter_kontakt_ms": {literal}', 2),
                           f"session_snapshot, letzter_kontakt_ms {literal}", vertrag, produkt,
                           ABGELEHNT, matrix, warum, feld=feld,
                           abweichung=_stufe(GRUND_QUELLENMODELL) if abw else None))
    for literal in NICHT_ENDLICH:
        faelle.append(_pe5("cpp_sources_snapshot", "v3", "session_snapshot",
                           _ersetze_alle(snapshot, kontakt, f'"letzter_kontakt_ms": {literal}', 2),
                           f"session_snapshot, letzter_kontakt_ms {literal}", "textriegel",
                           "textriegel", ABGELEHNT, "M-96",
                           f"{literal} faellt am Textriegel des Quellenmodells", feld=feld))

    def ruecknahme(start: str, ende: str) -> str:
        return als_text({"type": "evidence_invalidate", "grund": "sequenzluecke",
                         "umfang": {"art": "sample_range", "sample_start": "@START@",
                                    "sample_end": "@ENDE@"}}).decode("utf-8") \
            .replace('"@START@"', start).replace('"@ENDE@"', ende)

    faelle.append(_pe5("cpp_sources_ruecknahme", "v3", "evidence_invalidate",
                       ruecknahme("1024.0", "2048"), "sample_range 1024.0 bis 2048", None, None,
                       ["annahme"], "M-61", "1024.0 ist die Ganzzahl 1024",
                       feld="/umfang/sample_start", wert="1024"))
    faelle.append(_pe5("cpp_sources_ruecknahme", "v3", "evidence_invalidate",
                       ruecknahme("1024", "2048e0"), "sample_range 1024 bis 2048e0", None, None,
                       ["annahme"], "M-61", "2048e0 ist die Ganzzahl 2048",
                       feld="/umfang/sample_end", wert="2048"))
    for literal, vertrag, produkt, matrix, abw, warum in (
            ("1.5", "schema", "feldregel", "M-61", True, "1.5 ist keine Ganzzahl"),
            ("-1", "schema", "feldregel", "M-61", True, "-1 liegt unter der Untergrenze 0"),
            (ZWEI_HOCH_53, "textriegel", "textriegel", "M-73", False,
             "2^53 liegt ueber der Textriegel-Grenze")):
        faelle.append(_pe5("cpp_sources_ruecknahme", "v3", "evidence_invalidate",
                           ruecknahme(literal, "2048"), f"sample_range {literal} bis 2048",
                           vertrag, produkt, ABGELEHNT, matrix, warum,
                           feld="/umfang/sample_start",
                           abweichung=_stufe(GRUND_QUELLENMODELL) if abw else None))
    for literal in NICHT_ENDLICH:
        faelle.append(_pe5("cpp_sources_ruecknahme", "v3", "evidence_invalidate",
                           ruecknahme(literal, "2048"), f"sample_range {literal} bis 2048",
                           "textriegel", "textriegel", ABGELEHNT, "M-96",
                           f"{literal} faellt am Textriegel des Quellenmodells",
                           feld="/umfang/sample_start"))
    return faelle


def _faelle_v2_zahlen() -> list[dict]:
    """M-91, M-93, M-94, M-96 (`cpp_v2_client`, Fassung v2): der Zahlriegel."""
    w = V2_WELCOME
    faelle = []
    for literal, vertrag, produkt, ub, warum in (
            (V2_GANZZAHL_MAX, "schema", "feldregel", False,
             "INT64_MAX passiert den Riegel und ist keine Protokollversion (enum [1, 2])"),
            ("9223372036854775808", "schema", "textriegel", True,
             "INT64_MAX + 1: JUCEs int64-Akkumulator liefe ueber, der Riegel faengt ihn"),
            ("18446744073709551618", "schema", "textriegel", True,
             "2^64 + 2 klappte in JUCE auf 2 und gaelte als Version 2"),
            ("2e4294967296", "schema", "textriegel", True,
             "der Exponent liefe in JUCE ueber und laese 2.0"),
            ("2.0000000000000001", "schema", "textriegel", False,
             "17 signifikante Stellen: JUCE liest 2.0 (Regel 2, UB-frei)"),
            ("2.0", None, None, False, "2.0 ist die Ganzzahl 2 und haelt enum [1, 2]")):
        abw = None
        if produkt == "textriegel":
            abw = _stufe(GRUND_ZAHLRIEGEL)
        elif produkt == "feldregel":
            abw = _stufe(GRUND_V2_FELD)
        faelle.append(_pe5("cpp_v2_client", "v2", "welcome",
                           w.replace('"protocol_version":2', f'"protocol_version":{literal}'),
                           f"welcome, protocol_version {literal}", vertrag, produkt,
                           ["annahme"] if produkt is None else ABGELEHNT, "M-91", warum,
                           feld="/protocol_version", wert="2" if produkt is None else None,
                           abweichung=abw, ub=ub))

    def ack(literal: str) -> str:
        return '{"type":"heartbeat_ack","seq":' + literal + ',"konflikt":false}'
    faelle.append(_pe5("cpp_v2_client", "v2", "heartbeat_ack", ack("18446744073709551616"),
                       "heartbeat_ack seq 2^64 auf den ersten Heartbeat (seq 0)", "schema",
                       "textriegel", ["ablehnung", "kein_ack"], "M-93",
                       "0 + 2^64 klappte in JUCE auf 0 und bestaetigte den ersten Heartbeat",
                       feld="/seq", abweichung=_stufe(GRUND_ZAHLRIEGEL), ub=True))
    faelle.append(_pe5("cpp_v2_client", "v2", "heartbeat_ack", ack("1.0000000000000001"),
                       "heartbeat_ack seq 1.0000000000000001 auf den zweiten Heartbeat",
                       "schema", "textriegel", ["ablehnung", "kein_ack"], "M-93",
                       "17 signifikante Stellen: JUCE liest 1.0 und bestaetigte Heartbeat 1",
                       feld="/seq", abweichung=_stufe(GRUND_ZAHLRIEGEL),
                       einspeisung={"gueltige_acks_davor": 1}))
    faelle.append(_pe5("cpp_v2_client", "v2", "heartbeat_ack", ack(V2_GANZZAHL_MAX),
                       "heartbeat_ack seq INT64_MAX auf den ersten Heartbeat (seq 0)", None,
                       "feldregel", ["ablehnung", "kein_ack"], "M-93",
                       "INT64_MAX ist vertragsgueltig, beantwortet aber nicht Heartbeat 0",
                       feld="/seq", wert=V2_GANZZAHL_MAX,
                       abweichung={"art": "urteil", "grund": (
                           "zustandsabhaengige Feldregel des v2-Clients: die Sequenz des ACK "
                           "muss die des beantworteten Heartbeats sein (ackSeq != aktuelleSeq, "
                           "src/PipeClient.cpp); das Schema kennt den Heartbeat nicht (E-313-10)")}))

    reject = '{"type":"reject","reason":"inkompatibel","min_protocol":1,"max_protocol":2}'
    for feld, literal, ub, warum in (
            ("min_protocol", "9223372036854775808", True, "INT64_MAX + 1 in min_protocol"),
            ("max_protocol", "2e4294967296", True, "Exponent ueber drei Ziffern in max_protocol"),
            ("min_protocol", "2.0000000000000001", False,
             "der UB-freie Geschwistervektor: JUCE liest 2.0")):
        alt = f'"{feld}":{"1" if feld == "min_protocol" else "2"}'
        faelle.append(_pe5("cpp_v2_client", "v2", "reject",
                           _ersetze(reject, alt, f'"{feld}":{literal}'),
                           f"reject, {feld} {literal}", "schema", "textriegel", ABGELEHNT,
                           "M-94", warum, feld=f"/{feld}", abweichung=_stufe(GRUND_ZAHLRIEGEL),
                           ub=ub))

    for literal in NICHT_ENDLICH:
        riegel = literal == "1e999"
        faelle.append(_pe5("cpp_v2_client", "v2", "welcome",
                           w.replace('"protocol_version":2', f'"protocol_version":{literal}'),
                           f"welcome, protocol_version {literal}",
                           "schema" if riegel else "parser", "textriegel" if riegel else "parser",
                           ABGELEHNT, "M-96",
                           ("1e999 faellt am Zahlriegel (Regel 3), der v2-Vertrag erst am Schema"
                            if riegel else
                            f"{literal} ist keine JSON-Zahl: der strenge Lauf lehnt ab"),
                           feld="/protocol_version",
                           abweichung=_stufe(GRUND_ZAHLRIEGEL) if riegel else None))
    return faelle


def _faelle_rust_werte() -> list[dict]:
    """M-64, M-65, M-66, M-72, M-73, M-90, M-96: die Rust-Produktleser."""
    faelle = []
    kein = ["ablehnung", "kein_ack", "keine_teilmutation"]
    hb = _grundtext(GRUND["heartbeat"])
    seq = '"sequence": 91,'
    for literal in ("91.0", "9.1e1"):
        faelle.append(_pe5("rust_p0", "v3", "heartbeat", _ersetze(hb, seq, f'"sequence": {literal},'),
                           f"heartbeat, sequence {literal}", None, None, ["annahme"], "M-64",
                           f"{literal} ist die Ganzzahl 91: das ACK zitiert 91",
                           feld="/sequence", wert="91"))
    mit_runtime = _ersetze(hb, '"intervention_state_unknown": false\n}\n',
                           '"intervention_state_unknown": false,\n  "runtime": {\n'
                           '    "messpunkt": "insert",\n    "betrieb": "active",\n'
                           '    "host_mixer_index": 3.0\n  }\n}\n')
    faelle.append(_pe5("rust_p0", "v3", "heartbeat", mit_runtime,
                       "heartbeat, runtime.host_mixer_index 3.0", None, None, ["annahme"],
                       "M-65", "E-313-9: der Deskriptor traegt den Mixerindex als Ganzzahl 3",
                       feld="/runtime/host_mixer_index", wert="3"))
    for literal, stufe, matrix, warum in (
            ("1.5", "schema", "M-73", "1.5 ist keine Ganzzahl"),
            (ZWEI_HOCH_53, "textriegel", "M-73", "2^53 liegt ueber der Textriegel-Grenze")):
        faelle.append(_pe5("rust_p0", "v3", "heartbeat", _ersetze(hb, seq, f'"sequence": {literal},'),
                           f"heartbeat, sequence {literal}", stufe, stufe, kein, matrix, warum,
                           feld="/sequence"))
    faelle.append(_pe5("rust_p0", "v3", "heartbeat",
                       _ersetze(hb, '"state_revision": 12,', f'"state_revision": {ZWEI_HOCH_53},'),
                       "heartbeat, state_revision 2^53", "textriegel", "textriegel", kein, "M-90",
                       "eine Revision ueber dem Bereich ist ein Vertragsbruch",
                       feld="/state_revision"))
    for literal in NICHT_ENDLICH:
        faelle.append(_pe5("rust_p0", "v3", "heartbeat", _ersetze(hb, seq, f'"sequence": {literal},'),
                           f"heartbeat, sequence {literal}", "textriegel", "textriegel", kein,
                           "M-96", f"{literal} faellt am Textriegel", feld="/sequence"))

    ev = _grundtext(GRUND["evidence_snapshot"])
    ev_seq = '"sequence": 8241,'
    for literal in ("8241.0", "8.241e3"):
        faelle.append(_pe5("rust_p1", "v3", "evidence_snapshot",
                           _ersetze(ev, ev_seq, f'"sequence": {literal},'),
                           f"evidence_snapshot, transport.sequence {literal}", None, None,
                           ["annahme"], "M-66",
                           f"{literal} ist die Ganzzahl 8241: der Evidenzstand traegt 8241",
                           feld="/transport/sequence", wert="8241"))
    for literal, stufe, warum in (("1.5", "schema", "1.5 ist keine Ganzzahl"),
                                  (ZWEI_HOCH_53, "textriegel",
                                   "2^53 liegt ueber der Textriegel-Grenze")):
        faelle.append(_pe5("rust_p1", "v3", "evidence_snapshot",
                           _ersetze(ev, ev_seq, f'"sequence": {literal},'),
                           f"evidence_snapshot, transport.sequence {literal}", stufe, stufe,
                           ABGELEHNT, "M-73", warum, feld="/transport/sequence"))
    bericht = _grundtext(GRUND["state_report"])
    faelle.append(_pe5("rust_p1", "v3", "state_report",
                       _ersetze(bericht, '"state_revision": 13,',
                                f'"state_revision": {ZWEI_HOCH_53},'),
                       "state_report, state_revision 2^53", "textriegel", "textriegel",
                       ABGELEHNT, "M-90",
                       "eine Revision ueber dem Bereich ist ein Vertragsbruch",
                       feld="/state_revision"))
    for literal in NICHT_ENDLICH:
        faelle.append(_pe5("rust_p1", "v3", "evidence_snapshot",
                           _ersetze(ev, ev_seq, f'"sequence": {literal},'),
                           f"evidence_snapshot, transport.sequence {literal}", "textriegel",
                           "textriegel", ABGELEHNT, "M-96", f"{literal} faellt am Textriegel",
                           feld="/transport/sequence"))

    hello = _grundtext(GRUND["hello_control"])
    for alt, neu, feld, wert in (
            ('"protocol": 3,', '"protocol": 3.0,', "/protocol", "3"),
            ('"protocol": 3,', '"protocol": 3e0,', "/protocol", "3"),
            ('"block_size": 512,', '"block_size": 256.0,', "/audio/block_size", "256"),
            ('"channels": 2\n', '"channels": 2.0\n', "/audio/channels", "2"),
            ('"pid": 4711,', '"pid": 1234.0,', "/host/pid", "1234")):
        literal = neu.split(": ", 1)[1].rstrip(",\n")
        faelle.append(_pe5("rust_bootstrap", "v3", "hello_control", _ersetze(hello, alt, neu),
                           f"Control-Hello, {feld[1:]} {literal}", None, None, ["annahme"],
                           "M-72", f"{literal} ist die Ganzzahl {wert}: die typisierte "
                           "Uebernahme liest sie", feld=feld, wert=wert))
    return faelle


# ════════════════════════════════════════════════════════════════════════
# NAK-313 Etappe 6 · Tor, Handshake, Zieladresse (R-313-7; Manifest §7.3,
# „Etappe 6", §8.6)
# ════════════════════════════════════════════════════════════════════════
#
# Das Tor (bootstrap_lesen) prueft ein v3-Hello in der Reihenfolge
# Textriegel -> ein strenger Lauf -> Protokollwahl -> Hello-Schema ->
# Uebernahme; ein v2-Hello verlaesst es nach dem strengen Lauf als V2. Die
# flachen C++-Leser (Welcome, Reject, ACK) pruefen UTF-8 am Anfang, dekodieren
# Escapes, vergleichen dekodierte Namen und zaehlen Codepunkte. Wo ein Leser
# weder Textriegel noch Schema hat, steht die Abweichung der Art stufe mit
# Grund im Eintrag.
#
# Die Bootstrap-Negativen aus Etappe 5 (block_size 1.5 und 2^53, protocol 3.5,
# NaN, Infinity, -Infinity, 1e999) entstehen hier mit dem Tor (E-313-11); sie
# behalten ihre Matrixzeile (M-72, M-73, M-96) und tragen die anlegende Etappe
# im Erzeuger (Schluessel _etappe, nicht in der Datei), damit die
# Selbstpruefung sie gegen die Kette der Etappe 6 haelt.

GRUND_TOR_PROTOKOLL = ("die Protokollwahl des Tors liest protocol ueber den Ganzzahlhelfer und "
                       "lehnt einen Wert, der nicht 3 ist, vor Textriegel und Schema ab "
                       "(broker/src/transport/bootstrap.rs, bootstrap_lesen; R-313-16, E-313-16)")
GRUND_TOR_PARSER = ("am Tor laeuft der strenge Lauf vor dem gehaltenen Textriegelurteil: ohne Wert "
                    "ist das Protokoll nicht bestimmbar, und serde_json lehnt NaN, Infinity und "
                    "1e999 selbst ab (bootstrap_lesen, KeinJson; Manifest §8.6)")
GRUND_FLACH_LAENGE = ("der flache Leser hat kein Schema: welcomeHaeltVertrag und rejectHaeltVertrag "
                      "pruefen die Laenge nach dem Lesen in Codepunkten (utf8CodepointsBis, "
                      "core/ipc/controlclient/Vertrag.cpp)")
GRUND_FLACH_LESEN = ("der flache Leser hat keinen Textriegel: flachesJsonObjekt lehnt kaputtes "
                     "UTF-8, NUL-Escape, einsame Surrogate und kurze u-Escapes beim Lesen ab "
                     "(core/ipc/IpcVerbindung.cpp, Grund-Ausgang)")

V2_HELLO = ('{"type":"hello","protocol_version":2,"plugin_version":"0.4.0","host_pid":4711,'
            '"sensor":{"sensor_id":"sensor-1","role":"sensor","label":"Klavier"},'
            '"audio":{"samplerate":48000,"block_size":512,"channels":2}}')


def _mit_bytes(text: str, platzhalter: str, ersatz: bytes) -> bytes:
    """Der Text als UTF-8, der Platzhalter genau einmal durch rohe Bytes ersetzt
    - so stehen kaputtes UTF-8 und einzelne Bytes wie FF im Eintrag."""
    roh = text.encode("utf-8")
    marke = platzhalter.encode("utf-8")
    if roh.count(marke) != 1:
        raise SystemExit(f"Produkteingaenge: {platzhalter!r} steht nicht genau einmal im Text")
    return roh.replace(marke, ersatz)


def _welcome_text(broker_version: str) -> str:
    """welcome in der Form des Testservers; broker_version als JSON-Text."""
    k = ACK_KOPPLUNG
    return ('{"type":"welcome","protocol":3,"broker_version":' + broker_version
            + ',"broker_epoch":"' + k["broker_epoch"] + '","link_id":"' + k["link_id"]
            + '","challenge":"' + k["challenge"] + '"}')


def _reject_text(reason: str, code: str = '"protocol_mismatch"') -> str:
    """reject mit code und reason als JSON-Text."""
    return '{"type":"reject","code":' + code + ',"reason":' + reason + '}'


def _faelle_tor() -> list[dict]:
    """M-98 bis M-102, M-104 und die verschobenen Negativen (`rust_bootstrap`)."""
    faelle = []
    hello = _grundtext(GRUND["hello_control"])
    host = ('  "host": {\n    "pid": 4711,\n    "name": "FL64",\n'
            '    "version": "21.2"\n  },\n')
    nul = '"plugin_version": "a' + BS + 'u0000b",'
    version = '"plugin_version": "0.4.0",'
    tor = [
        ("M-98", "Control-Hello ohne host", _ersetze(hello, host, ""), None, None,
         "host ist nicht Pflicht: das Tor nimmt das Hello ohne host an"),
        ("M-99", "Control-Hello mit host null", _ersetze(hello, host, '  "host": null,\n'),
         "schema", "schema", "null ist ein eigener Typ, nicht fehlt: /host verletzt type"),
        ("M-99", "Control-Hello mit host.name null",
         _ersetze(hello, host, '  "host": {\n    "pid": 1,\n    "name": null\n  },\n'),
         "schema", "schema", "/host/name verletzt type string"),
        ("M-99", "Control-Hello mit host.version null",
         _ersetze(hello, host, '  "host": {\n    "pid": 1,\n    "version": null\n  },\n'),
         "schema", "schema", "/host/version verletzt type string"),
        ("M-100", "Control-Hello mit NUL-Escape in plugin_version", _ersetze(hello, version, nul),
         "textriegel", "textriegel", "Regel 5: kein NUL-Escape in einer Zeichenkette"),
        ("M-101", "Control-Hello, samplerate 1e-308",
         _ersetze(hello, '"samplerate": 48000,', '"samplerate": 1e-308,'),
         "textriegel", "textriegel", "Regel 3, Untergrenze: Dezimalexponent -308"),
        ("M-101", "Control-Hello, samplerate 1e-307",
         _ersetze(hello, '"samplerate": 48000,', '"samplerate": 1e-307,'),
         None, None, "1e-307 haelt Regel 3 und exclusiveMinimum 0"),
        ("M-101", "Control-Hello, samplerate 1e-300",
         _ersetze(hello, '"samplerate": 48000,', '"samplerate": 1e-300,'),
         None, None, "1e-300 passiert beide Grenzen (NAK-387 Beobachtung 1, nur benannt)"),
        ("M-101", "Control-Hello, samplerate 48000", hello, None, None,
         "die gewoehnliche Samplerate"),
        ("M-101", "Control-Hello, samplerate 768000.5",
         _ersetze(hello, '"samplerate": 48000,', '"samplerate": 768000.5,'),
         "schema", "schema", "768000.5 verletzt maximum 768000"),
        ("M-102", "Control-Hello mit NUL-Escape in plugin_version und host null",
         _ersetze(_ersetze(hello, version, nul), host, '  "host": null,\n'),
         "textriegel", "textriegel", "der Textriegel geht dem Schema vor"),
    ]
    for matrix, zeige, roh, vertrag, produkt, warum in tor:
        faelle.append(_pe5("rust_bootstrap", "v3", "hello_control", roh, zeige, vertrag, produkt,
                           ["annahme"] if produkt is None else ["ablehnung"], matrix, warum))

    block = '"block_size": 512,'
    verschoben = [
        ("M-73", "block_size 1.5", _ersetze(hello, block, '"block_size": 1.5,'), "schema",
         "schema", "/audio/block_size", None, "1.5 ist keine Ganzzahl"),
        ("M-73", f"block_size {ZWEI_HOCH_53}",
         _ersetze(hello, block, f'"block_size": {ZWEI_HOCH_53},'), "textriegel", "textriegel",
         "/audio/block_size", None, "2^53 liegt ueber der Textriegel-Grenze"),
        ("M-72", "protocol 3.5", _ersetze(hello, '"protocol": 3,', '"protocol": 3.5,'), "schema",
         "feldregel", "/protocol", _stufe(GRUND_TOR_PROTOKOLL),
         "3.5 ist keine Ganzzahl: die Protokollwahl liest es nicht als 3"),
    ]
    for literal in NICHT_ENDLICH:
        verschoben.append(("M-96", f"block_size {literal}",
                           _ersetze(hello, block, f'"block_size": {literal},'), "textriegel",
                           "parser", "/audio/block_size", _stufe(GRUND_TOR_PARSER),
                           f"{literal} ist kein JSON-Wert einer Ganzzahl"))
    for matrix, zeige, roh, vertrag, produkt, feld, abweichung, warum in verschoben:
        eintrag = _pe5("rust_bootstrap", "v3", "hello_control", roh, f"Control-Hello, {zeige}",
                       vertrag, produkt, ["ablehnung"], matrix, warum, feld=feld,
                       abweichung=abweichung)
        eintrag["_etappe"] = "6"
        faelle.append(eintrag)

    for zeige, roh, warum in (
            ("v2-Hello, samplerate 48000.0000000000001",
             V2_HELLO.replace('"samplerate":48000', '"samplerate":48000.0000000000001'),
             "v2-gueltig (number bis 768000); v3-Regel 2 (mehr als 15 Stellen) gilt nicht"),
            ("v2-Hello mit NUL-Escape in sensor.label",
             V2_HELLO.replace('"label":"Klavier"', '"label":"Kla' + BS + 'u0000vier"'),
             "v2-gueltig (string bis 120); v3-Regel 5 (NUL-Escape) gilt nicht")):
        faelle.append(_pe5("rust_bootstrap", "v2", "hello", roh, zeige, None, None, ["annahme"],
                           "M-104", warum))
    return faelle


def _faelle_handshake() -> list[dict]:
    """M-106 bis M-113 (`cpp_control_handshake`, `cpp_telemetrie_handshake`) und
    die ACK-Faelle aus M-108, M-111, M-112 (`cpp_control_ack`)."""
    ein = ["annahme"]
    aus = ["ablehnung"]
    paar = BS + "ud83d" + BS + "ude00"
    escapes = '"' + BS + "n" + BS + BS + BS + '"' + BS + "/" + BS + "u00e9" + paar + '"'
    # Schluessel -> (Nachricht, Bytes, vertrag, produkt, Abweichung, Wirkung, Zeile,
    # Zeigetext, warum); die Telemetriefaelle aus M-113 nehmen dieselben Bytes.
    control = {
        "106a": ("welcome", _welcome_text('"' + "é" * 64 + '"'), None, None, None, ein,
                 "M-106", "welcome, broker_version 64 x e-Akut (128 Bytes)",
                 "64 Codepunkte halten maxLength 64"),
        "106b": ("welcome", _welcome_text('"' + "é" * 65 + '"'), "schema", "feldregel",
                 _stufe(GRUND_FLACH_LAENGE), aus, "M-106",
                 "welcome, broker_version 65 x e-Akut", "65 Codepunkte verletzen maxLength 64"),
        "106c": ("welcome", _welcome_text('"' + "v" * 64 + '"'), None, None, None, ein,
                 "M-106", "welcome, broker_version 64 ASCII", "die Grenze selbst"),
        "107a": ("reject", _reject_text('"' + "é" * 500 + '"'), None, None, None, ein,
                 "M-107", "reject, reason 500 x e-Akut (1000 Bytes)",
                 "500 Codepunkte halten maxLength 500"),
        "107b": ("reject", _reject_text('"' + "é" * 501 + '"'), "schema", "feldregel",
                 _stufe(GRUND_FLACH_LAENGE), aus, "M-107", "reject, reason 501 x e-Akut",
                 "501 Codepunkte verletzen maxLength 500"),
        "108": ("reject", _reject_text(escapes), None, None, None, ein, "M-108",
                "reject, reason mit n, Backslash, Anfuehrungszeichen, Schraegstrich, "
                "u00e9 und Surrogatpaar",
                "RFC-8259-Escapes sind vertragsgueltig; das Paar ist ein Codepunkt"),
        "108r500": ("reject", _reject_text('"' + "a" * 499 + paar + '"'), None, None, None, ein,
                    "M-108", "reject, reason 499 x a plus Surrogatpaar",
                    "500 Codepunkte: das Paar zaehlt als einer"),
        "108r501": ("reject", _reject_text('"' + "a" * 500 + paar + '"'), "schema", "feldregel",
                    _stufe(GRUND_FLACH_LAENGE), aus, "M-108",
                    "reject, reason 500 x a plus Surrogatpaar",
                    "501 Codepunkte verletzen maxLength 500"),
        "109a": ("reject", _reject_text('"' + BS + 'u0000"'), "textriegel", "parser",
                 _stufe(GRUND_FLACH_LESEN), aus, "M-109", "reject, reason mit NUL-Escape",
                 "Regel 5: NUL-Escape"),
        "109b": ("reject", _reject_text('"' + BS + 'ud800"'), "textriegel", "parser",
                 _stufe(GRUND_FLACH_LESEN), aus, "M-109", "reject, reason mit einsamem ud800",
                 "Regel 6: einsames Surrogat"),
        "109c": ("reject", _reject_text('"' + BS + 'q"'), "parser", "parser", None, aus,
                 "M-109", "reject, reason mit unbekanntem Escape q",
                 "RFC 8259 kennt kein q-Escape"),
        "109d": ("reject", _reject_text('"' + BS + 'u00"'), "textriegel", "parser",
                 _stufe(GRUND_FLACH_LESEN), aus, "M-109", "reject, reason mit kurzem u00",
                 "Regel 4: genau vier Hexziffern"),
        "110r": ("reject", _reject_text('""'), None, None, None, ein, "M-110",
                 "reject, leerer reason", "reason hat kein minLength"),
        "110w": ("welcome", _welcome_text('""'), "schema", "feldregel",
                 _stufe(GRUND_FLACH_LAENGE), aus, "M-110", "welcome, leere broker_version",
                 "broker_version verlangt minLength 1"),
        "111a": ("welcome", _mit_bytes(_welcome_text('"@@"'), "@@", b"\xc3("), "textriegel",
                 "parser", _stufe(GRUND_FLACH_LESEN), aus, "M-111",
                 "welcome, broker_version mit den Bytes C3 28", "Regel 9: kein gueltiges UTF-8"),
        "111b": ("reject", _mit_bytes(_reject_text('"ab@@cd"'), "@@", b"\xff"), "textriegel",
                 "parser", _stufe(GRUND_FLACH_LESEN), aus, "M-111",
                 "reject, reason mit dem Byte FF", "Regel 9: kein gueltiges UTF-8"),
        "111c": ("reject", _mit_bytes(_reject_text('"inkompatibel"', '"protocol@@mismatch"'),
                                      "@@", b"\xff"), "textriegel", "parser",
                 _stufe(GRUND_FLACH_LESEN), aus, "M-111", "reject, code mit dem Byte FF",
                 "Regel 9: kein gueltiges UTF-8"),
        "112a": ("welcome", '{"typ' + BS + 'u0065":"reject",' + _welcome_text('"test"')[1:],
                 "duplikat", "duplikat", None, aus, "M-112",
                 "welcome mit typ-u0065-Alias neben type",
                 "der dekodierte Name type steht zweimal"),
        "112b": ("welcome", _welcome_text('"test"')[:-1] + ',"link_id":"' + "a" * 32 + '"}',
                 "duplikat", "duplikat", None, aus, "M-112", "welcome mit link_id zweimal",
                 "derselbe rohe Name zweimal"),
    }
    faelle = [_pe5("cpp_control_handshake", "v3", n, roh, zeige, v, p, w, m, warum,
                   abweichung=a)
              for n, roh, v, p, a, w, m, zeige, warum in control.values()]
    for schluessel in ("106a", "106b", "107a", "108", "109a", "111a", "111b", "111c"):
        n, roh, v, p, a, w, _m, zeige, warum = control[schluessel]
        faelle.append(_pe5("cpp_telemetrie_handshake", "v3", n, roh, zeige, v, p, w, "M-113",
                           warum, abweichung=a))

    kein_ack = ["kein_ack", "kein_freigegebener_auftrag"]
    kopf = '{"type":"command_ack","command_id":"' + ACK_COMMAND_ID + '",'
    faelle.append(_pe5("cpp_control_ack", "v3", "command_ack",
                       kopf + '"ergebnis":"konflikt","state_revision":7,"code":"revision'
                       + BS + 'u005fconflict"}',
                       "command_ack konflikt, code mit u005f-Escape", None, None, ["annahme"],
                       "M-108", "das Escape dekodiert zum bekannten Code revision_conflict"))
    faelle.append(_pe5("cpp_control_ack", "v3", "command_ack",
                       _mit_bytes(kopf + '"ergebnis":"abgelehnt","state_revision":7,'
                                  '"code":"schema_violation@@"}', "@@", b"\xff"),
                       "command_ack abgelehnt, code mit dem Byte FF", "textriegel", "parser",
                       kein_ack, "M-111", "Regel 9: kein gueltiges UTF-8",
                       abweichung=_stufe(GRUND_FLACH_LESEN)))
    faelle.append(_pe5("cpp_control_ack", "v3", "command_ack",
                       kopf + '"ergeb' + BS + 'u006eis":"abgelehnt","ergebnis":"angewandt",'
                       '"state_revision":7,"state_hash":"' + "d" * 64 + '"}',
                       "command_ack mit ergebnis als Escape-Alias und roh", "duplikat",
                       "duplikat", kein_ack, "M-112", "der dekodierte Name ergebnis steht zweimal"))
    return faelle


def _etappe_von(matrix: str) -> str:
    nummer = int(matrix.removeprefix("M-").rstrip("b"))
    for von, bis, etappe in MATRIX_ETAPPEN:
        if von <= nummer <= bis:
            return str(etappe)
    raise SystemExit(f"Produkteingaenge: {matrix} gehoert zu keiner Etappe")


def _tabelle_selbstpruefung(kopf: dict, anlegend: dict[str, str]) -> None:
    """Die Selbstpruefung des Erzeugers (Manifest §7.1, R-313-13).

    Sie prueft die HANDSCHRIFT auf Widerspruchsfreiheit, nicht auf Wahrheit;
    die Wahrheit messen die Beine gegen die echten Leser. `anlegend` nennt je
    Kennung die Etappe, die den Eintrag anlegt - die der Matrixzeile, ausser
    bei den mit dem Tor verschobenen Bootstrap-Negativen (Etappe 6).
    """
    faelle = kopf["faelle"]
    fehler: list[str] = []
    for i, f in enumerate(faelle, start=1):
        kennung = f["id"]
        if kennung != f"PE-{i:03d}":
            fehler.append(f"{kennung}: nicht fortlaufend")
        if f["eingang"] not in kopf["eingaenge"]:
            fehler.append(f"{kennung}: Eingang {f['eingang']} fehlt im Register")
        if f["fassung"] not in ("v3", "v2"):
            fehler.append(f"{kennung}: Fassung {f['fassung']}")
        if not set(f["wirkung"]) <= set(WIRKUNGEN) or not f["wirkung"]:
            fehler.append(f"{kennung}: Wirkung ausserhalb der geschlossenen Menge")
        etappe = anlegend[kennung]
        referenz = "A5" if f["fassung"] == "v3" else "A11"
        for objekt, leser in (("vertrag", referenz), ("produkt", f["eingang"])):
            u = f[objekt]
            if u["urteil"] not in ("gueltig", "ungueltig"):
                fehler.append(f"{kennung}: {objekt}.urteil {u['urteil']}")
            if (u["urteil"] == "gueltig") != (u["stufe"] is None):
                fehler.append(f"{kennung}: {objekt} - gueltig ohne Stufe, ungueltig mit Stufe")
            kette = LESERKETTEN.get(leser, {}).get(etappe)
            if kette is None:
                fehler.append(f"{kennung}: {leser} hat in Etappe {etappe} keine Kette")
            elif u["stufe"] is not None and u["stufe"] not in kette:
                fehler.append(f"{kennung}: Stufe {u['stufe']} liegt nicht in der Kette von {leser}")
        if f["nachricht"] == "evidence_snapshot":
            kette = LESERKETTEN["B16"].get(etappe, [])
            if f["vertrag"]["stufe"] is not None and f["vertrag"]["stufe"] not in kette:
                fehler.append(f"{kennung}: Stufe nicht in der Kette von B16")
        a = f["abweichung"]
        gleich_urteil = f["vertrag"]["urteil"] == f["produkt"]["urteil"]
        gleich_stufe = f["vertrag"]["stufe"] == f["produkt"]["stufe"]
        if a is None and not (gleich_urteil and gleich_stufe):
            fehler.append(f"{kennung}: verschiedene Objekte ohne abweichung")
        if a is not None and a.get("art") == "urteil" and gleich_urteil:
            fehler.append(f"{kennung}: Abweichung urteil bei gleichen Urteilen")
        if a is not None and a.get("art") == "stufe" and (not gleich_urteil or gleich_stufe):
            fehler.append(f"{kennung}: Abweichung stufe ohne Stufenunterschied")
        if a is not None and a.get("art") not in ("urteil", "stufe"):
            fehler.append(f"{kennung}: Abweichung ohne Art")
        if a is not None and not a.get("grund"):
            fehler.append(f"{kennung}: Abweichung ohne Grund")
        # Etappe 5 (§7.1, `feld` und `wert`): ein Ganzzahlfall, der in einem der
        # beiden Objekte gueltig ist, traegt seinen Wert als Dezimaltext; einer,
        # den beide ablehnen, traegt keinen.
        irgendwo_gueltig = "gueltig" in (f["vertrag"]["urteil"], f["produkt"]["urteil"])
        if f["wert"] is not None:
            if f["feld"] is None or not f["feld"].startswith("/"):
                fehler.append(f"{kennung}: wert ohne JSON-Pointer in feld")
            if not isinstance(f["wert"], str) or not f["wert"].lstrip("-").isdigit():
                fehler.append(f"{kennung}: wert ist kein Dezimaltext")
            if not irgendwo_gueltig:
                fehler.append(f"{kennung}: wert, obwohl beide Objekte ablehnen")
        elif f["feld"] is not None and irgendwo_gueltig:
            fehler.append(f"{kennung}: gueltiger Ganzzahlfall ohne wert")
        # Kein Lauf fuehrt einen Ueberlaufvektor JUCE zu (§8.1): er muss am
        # Zahlriegel des v2-Clients fallen, also vor jedem JUCE-Leser.
        if not isinstance(f["ub_bei_juce"], bool):
            fehler.append(f"{kennung}: ub_bei_juce ist kein Wahrheitswert")
        elif f["ub_bei_juce"] and (f["fassung"] != "v2"
                                   or f["produkt"]["stufe"] != "textriegel"):
            fehler.append(f"{kennung}: Ueberlaufvektor, den kein Zahlriegel vor JUCE faengt")
        if f["einspeisung"] is not None and not isinstance(f["einspeisung"], dict):
            fehler.append(f"{kennung}: einspeisung ist weder null noch ein Objekt")
    zaehle = lambda feld: {k: sum(1 for f in faelle if f[feld] == k)
                           for k in sorted({f[feld] for f in faelle})}
    je_eingang = {k: sum(1 for f in faelle if f["eingang"] == k) for k in kopf["eingaenge"]}
    if kopf["anzahl"] != len(faelle):
        fehler.append("anzahl passt nicht zu faelle")
    if kopf["anzahl_je_eingang"] != je_eingang:
        fehler.append("anzahl_je_eingang passt nicht zu faelle")
    if kopf["anzahl_je_fassung"] != zaehle("fassung"):
        fehler.append("anzahl_je_fassung passt nicht zu faelle")
    if kopf["anzahl_je_nachricht"] != zaehle("nachricht"):
        fehler.append("anzahl_je_nachricht passt nicht zu faelle")
    abw = {"stufe": sum(1 for f in faelle if f["abweichung"] and f["abweichung"]["art"] == "stufe"),
           "urteil": sum(1 for f in faelle if f["abweichung"] and f["abweichung"]["art"] == "urteil")}
    if kopf["anzahl_abweichungen"] != abw:
        fehler.append("anzahl_abweichungen passt nicht zu faelle")
    if abw != ABWEICHUNGEN_SPEZIFIZIERT:
        fehler.append(f"anzahl_abweichungen {abw} weicht vom Stand der Spezifikation "
                      f"{ABWEICHUNGEN_SPEZIFIZIERT} ab (Manifest §7.1, §7.3)")
    if fehler:
        raise SystemExit("PRODUKTEINGAENGE-FAELLE widerspricht sich:\n  " + "\n  ".join(fehler))


def produkteingaenge_tabelle() -> dict:
    """`PRODUKTEINGAENGE-FAELLE.json` - dieselben Bytes durch die ECHTEN Leser.

    Hex-kodiert nach dem Muster der Textriegel-Falltabelle: Nachspann, doppelte
    Namen und Escape-Aliase stuenden sonst nicht unveraendert in einer
    JSON-Datei. Produktbeine (B13, A4b, A4) vergleichen `produkt`,
    Referenzbeine (A5 fuer v3, A11 fuer v2, B16 fuer evidence_snapshot)
    `vertrag`; jedes Bein zaehlt seine Eintraege gegen den Kopf.
    """
    faelle = (_faelle_quellenmodell() + _faelle_v2_client() + _faelle_rust()
              + _faelle_flacher_leser() + _faelle_quellenmodell_werte()
              + _faelle_v2_zahlen() + _faelle_rust_werte()
              + _faelle_tor() + _faelle_handshake())
    anlegend: dict[str, str] = {}
    for i, f in enumerate(faelle, start=1):
        f["id"] = f"PE-{i:03d}"
        anlegend[f["id"]] = f.pop("_etappe", None) or _etappe_von(f["matrix"])
    faelle = [{"id": f["id"], **{k: w for k, w in f.items() if k != "id"}} for f in faelle]
    register = _einspeisung_register()
    kopf = {
        "$id": "evenacadia.nakama.produkteingaenge.faelle.v1",
        "titel": "Gemeinsame Vektortabelle der Produkteingaenge",
        "zweck": ("Der Vertragsleser jeder Sprache war laengst gemessen; die PRODUKTLESER "
                  "daneben nicht (Tiefenaudit 3, NAK-313 §1). Diese Tabelle bindet sie an "
                  "dieselben Bytes: je Eintrag das Urteil des Vertrags (Textriegel, "
                  "strenger Parselauf, Schema) und das des Produktlesers am Ende der "
                  "Etappe, die den Eintrag anlegt, jeweils mit der Stufe, an der er "
                  "faellt. Beide Urteile stehen von Hand im Erzeuger."),
        "warum_hex": ("Die Eintraege tragen Nachspann, doppelte Namen und Escape-Aliase; "
                      "als Text waeren sie von keinem JSON-Leser unveraendert "
                      "transportierbar. `zeigetext` ist nur fuer Menschen."),
        "wert_regel": ("`feld` ist bei Ganzzahlfaellen der JSON-Pointer des Feldes; `wert` "
                       "der erwartete Wert als Dezimaltext, wenn `vertrag` oder `produkt` "
                       "gueltig ist. Ein Bein vergleicht ihn, wenn SEIN Urteil gueltig ist "
                       "(C++ std::from_chars, Rust str::parse, Python int)."),
        "ub_bei_juce": ("true: der Vektor liesse JUCEs Zahlenleser ueberlaufen (int64-"
                        "Akkumulator oder Exponentzaehler). Kein Lauf fuehrt ihn JUCE zu; "
                        "der Rotlauf am Aufruf des Zahlriegels laesst ihn aus "
                        "(--ohne-ueberlaufvektoren, Manifest §8.1)."),
        "einspeisung_je_eintrag": ("Was ein Eintrag ueber die Einspeisung seines Eingangs "
                                   "hinaus braucht, sonst null: gueltige_acks_davor (v2-"
                                   "heartbeat_ack) nennt die gueltig bestaetigten Heartbeats "
                                   "vor den Bytes."),
        "eingaenge": register,
        "leser": LESERKETTEN,
        "stufen": STUFEN,
        "wirkungen": WIRKUNGEN,
        "anzahl": len(faelle),
        "anzahl_je_eingang": {k: sum(1 for f in faelle if f["eingang"] == k) for k in register},
        "anzahl_je_fassung": {k: sum(1 for f in faelle if f["fassung"] == k)
                              for k in sorted({f["fassung"] for f in faelle})},
        "anzahl_je_nachricht": {k: sum(1 for f in faelle if f["nachricht"] == k)
                                for k in sorted({f["nachricht"] for f in faelle})},
        "anzahl_abweichungen": {art: sum(1 for f in faelle
                                         if f["abweichung"] and f["abweichung"]["art"] == art)
                                for art in ("stufe", "urteil")},
        "faelle": faelle,
    }
    _tabelle_selbstpruefung(kopf, anlegend)
    return kopf

def baue() -> tuple[dict, dict[str, dict], dict[str, bytes]]:
    dateien: dict[str, dict] = {}
    rohdateien: dict[str, bytes] = {}
    eintraege: list[dict] = []

    for name, daten in GRUND.items():
        pfad = f"gueltig/{name}.json"
        dateien[pfad] = minor_1_sessionform(copy.deepcopy(daten))
        eintraege.append({"datei": pfad, "urteil": "gueltig",
                          "warum": "Grundform der Familie", "verletzungen": []})

    for name, daten, warum in zusatz_gueltig():
        pfad = f"gueltig/{name}.json"
        if pfad in dateien:
            raise SystemExit(f"doppelter Fixturename: {pfad}")
        dateien[pfad] = minor_1_sessionform(copy.deepcopy(daten))
        eintraege.append({"datei": pfad, "urteil": "gueltig",
                          "warum": warum, "verletzungen": []})

    for name, grund, mutationen, verletzungen, warum in UNGUELTIG:
        pfad = f"ungueltig/{name}.json"
        if pfad in dateien:
            raise SystemExit(f"doppelter Fixturename: {pfad}")
        daten = copy.deepcopy(GRUND[grund])
        for m in mutationen:
            daten = wende_an(daten, m)
        daten = minor_1_sessionform(daten)
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

    # NAK-313 Etappe 4 (R-313-6): die Klasse „Parser lehnt ab". Rohbytes wie
    # die Textriegel-Faelle; sie passieren den Textriegel und fallen am
    # strengen Parselauf, bevor das Schema sie sieht.
    for name, roh, _stufe, warum in parser_faelle():
        pfad = f"ungueltig/parser-{name}.json"
        if pfad in dateien or pfad in rohdateien:
            raise SystemExit(f"doppelter Fixturename: {pfad}")
        rohdateien[pfad] = roh
        eintraege.append({"datei": pfad, "urteil": "ungueltig", "warum": warum,
                          "verletzungen": [], "parser_lehnt_ab": True})

    # NAK-313 Etappe 4 (R-313-7, M-53): lokale Evidenz mit `null`.
    for name, daten, verletzungen, warum in lokale_evidenz_negativ():
        pfad = f"ungueltig/{name}.json"
        if pfad in dateien or pfad in rohdateien:
            raise SystemExit(f"doppelter Fixturename: {pfad}")
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
        "parser_lehnt_ab": ("Markiert ein Fixture, das den Textriegel passiert und am "
                            "STRENGEN PARSELAUF faellt, bevor das Schema es sieht: "
                            "Nachspann, zweites Dokument, Schlusskomma, unbekanntes "
                            "Escape, Verschachtelung tiefer als 64 Ebenen oder derselbe "
                            "dekodierte Name zweimal im selben Objekt (auch als "
                            "Escape-Alias). Keine Verletzungsmenge, aus demselben Grund "
                            "wie bei textriegel_lehnt_ab (NAK-313 R-313-6; "
                            "schemas/v3/README.md, Abschnitt zum strengen Parselauf)."),
        "handinstanzen": handinstanzen_register(),
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



# ── NAK-181 R4 · die Byteinstanz der Wire-Zahlform ──────────────────────
#
# 🔑 Die DRITTE Instanz zwischen den Sprachen, in der Bauform von
# `handschlag-v1.json` und `heartbeat-wire-v1.json`. Sie traegt je Zahlklasse
# die Eingabe als IEEE-754-Bitmuster (damit kein Literal auf dem Weg gerundet
# wird — MP4-1 hat gezeigt, wohin das fuehrt) und den erwarteten Wiretext.
#
# Drei Leser messen dagegen:
#   · C++  `EqCopSchemaTest` — `nakama::wire::wireZahl` erzeugt GENAU diesen Text;
#   · Rust `contract_cross_language.rs` — `textriegel_bytes` nimmt ihn an
#          beziehungsweise weist ihn ab, und `serde_json` liest den Wert zurueck;
#   · Python A5 `pruefe_v3_vertrag.py` — derselbe Riegel, dieselbe Antwort.
#
# Laufen zwei Seiten auseinander, faellt genau ein Bein — der Bruch, den ein
# Test auf nur einer Seite nie sieht.

def wire_zahl(x: float) -> str | None:
    """Die Wire-Zahlform aus `eq-copilot/plugin/core/ipc/WireZahl.h`.

    ES6 `Number::toString`, gedeckelt bei 15 signifikanten Stellen fuer
    NICHTGANZZAHLIGE Werte; `None`, wenn der v3-Textriegel den Wert nicht
    traegt. Die Regeln stehen dort im Kopf; hier ist die Python-Haelfte, und
    dass beide dasselbe sagen, misst die Fixture.
    """
    import math
    if not math.isfinite(x):
        return None
    if x == 0.0:
        return "0"                      # deckt +0 und -0 ab
    betrag = abs(x)
    ganzzahlig = betrag == math.floor(betrag)
    if ganzzahlig and betrag > 9007199254740991.0:
        return None                     # Regel 3
    # Ziffernfolge und Exponent, gedeckelt nur bei Nachkommaanteil.
    roh = repr(betrag) if ganzzahlig else "%.*e" % (14, betrag)
    if ganzzahlig:
        ziffern = str(int(betrag))
        n = len(ziffern)
        ziffern = ziffern.rstrip("0") or "0"
    else:
        mant, exp = roh.split("e")
        ziffern = mant.replace(".", "").rstrip("0") or "0"
        n = int(exp) + 1
    dez = n - 1
    if dez >= 308 or dez <= -308:        # Regel 2, am GERUNDETEN Text
        return None
    k = len(ziffern)
    if -5 <= n <= 21:
        if n >= k:
            text = ziffern + "0" * (n - k)
        elif n > 0:
            text = ziffern[:n] + "." + ziffern[n:]
        else:
            text = "0." + "0" * (-n) + ziffern
    else:
        vz = "-" if n < 0 else "+"
        e = abs(n - 1)
        text = (ziffern if k == 1 else ziffern[0] + "." + ziffern[1:]) + "e" + vz + str(e)
    if not ganzzahlig and float(text) == math.floor(float(text)) \
            and abs(float(text)) > 9007199254740991.0:
        return None                     # Regel 3 am gerundeten Text
    return ("-" if x < 0 else "") + text


def zahlklassen_wire() -> bytes:
    """`evidenz-zahlen-wire-v1.json` — die Byteinstanz je Zahlklasse."""
    import math, struct

    def bits(x: float) -> str:
        return "0x%016x" % struct.unpack("<Q", struct.pack("<d", x))[0]

    def eintrag(name: str, x: float, warum: str) -> dict:
        text = wire_zahl(x)
        e = {"klasse": name, "eingabe_hex64": bits(x), "warum": warum,
             "angenommen": text is not None}
        if text is not None:
            e["wire"] = text
        return e

    angenommen = [
        eintrag("minus_null", -0.0, "ES6 Schritt 2: +0 und -0 sind beide `0`"),
        eintrag("ein_zehntel", 0.1, "gewoehnlicher Bruch"),
        eintrag("ein_drittel", 1.0 / 3.0,
                "ungedeckelt 16 Stellen und damit abgelehnt; der Deckel macht ihn tragbar"),
        eintrag("kleinster_normaler_float", float(struct.unpack("f", struct.pack("f", 1.1754943508222875e-38))[0]),
                "jeder Bandwert ist ein float; 9 Stellen reichen ihm exakt"),
        eintrag("eins_e_minus_307", 1e-307, "der kleinste angenommene Betrag (dez = -307)"),
        eintrag("knapp_unter_der_grenze", math.nextafter(1e-307, 0.0),
                "der Wert liegt darunter, sein GEDECKELTER Text nicht - die Grenze gilt dem Text"),
        eintrag("sichere_ganzzahl", 9007199254740991.0,
                "2^53-1 reist EXAKT; der Deckel trifft nie eine Ganzzahl"),
        eintrag("nyquist_gekappter_bandwert", 18000.0, "eine Bandmitte an der Kappe"),
        eintrag("samplerate", 48000.0, "die gewoehnliche Samplerate"),
        # ── WN3-02 (Nacharbeit 3, 07.09.2026): die Kleinwerte aus WP2-2.
        #
        # Die numerische Rundung der Nacharbeit 2 bildete den Faktor
        # `10 ** (14 - exponent)`, und der ist ab `exponent <= -9` selbst
        # ungenau: `round(2.7e-11 * 1e25) / 1e25` ist `2.6999999999999997e-11`
        # - SIEBZEHN signifikante Stellen, und der eigene Textriegel verwirft
        # sie. Diese fuenf Klassen halten die Grenze in allen drei Sprachen an
        # derselben Byteinstanz fest.
        eintrag("rangkomponente_2p7e_minus_11", 2.7e-11,
                "WP2-2: der Wert des Urteils - die alte Rundung machte 17 Stellen daraus"),
        eintrag("rangkomponente_5p55e_minus_12", 5.55e-12,
                "eine Dekade tiefer, derselbe Bruch der alten Rundung"),
        eintrag("rangkomponente_lang", 9.038084803672431e-11,
                "eine gerechnete Rangkomponente in voller Laenge"),
        eintrag("float_minus_3p3", float(struct.unpack("f", struct.pack("f", -3.3))[0]),
                "N-30: `beobachtung.wert_db` entsteht aus einem float und traegt als double 16 Stellen"),
        eintrag("null_komma_eins_plus_null_komma_zwei", 0.1 + 0.2,
                "der klassische Bruch: 0.30000000000000004 kuerzt sich auf 0.3"),
    ]
    verweigert = [
        eintrag("zwei_e_minus_308", 2e-308,
                "GROESSER als 1e-308 und trotzdem drausen: dez = -308"),
        eintrag("eins_e_minus_308", 1e-308, "genau an der unteren Kante"),
        eintrag("zwei_hoch_53", 9007199254740992.0, "eine Ganzzahl ueber der sicheren Grenze"),
        eintrag("groesster_normaler_float", float(struct.unpack("f", struct.pack("f", 3.4028234663852886e38))[0]),
                "eine Ganzzahl weit ueber 2^53 - jeder double ab 2^53 ist ganzzahlig"),
        eintrag("eins_e_308", 1e308, "an der oberen Betragsgrenze"),
        eintrag("nicht_endlich", float("inf"), "unendlich reist nie als Zahl"),
    ]
    return als_text({
        "_kommentar": [
            "NAK-181 R4 - die BYTEINSTANZ der Wire-Zahlform.",
            "",
            "Sie ist die dritte Instanz zwischen C++, Rust und Python: keiner der",
            "drei erzeugt sie, alle drei messen dagegen. Die Eingaben stehen als",
            "IEEE-754-Bitmuster, weil ein Dezimalliteral auf dem Weg gerundet",
            "werden kann - `9.9999999999999999e-308` ist als binary64 bitgleich",
            "mit `1e-307` und belegte deshalb nichts (Matrixpruefung 4, MP4-1).",
            "",
            "`angenommen: false` heisst: der v3-Textriegel traegt den Wert nicht,",
            "und `wireZahl` liefert `false`. Was statt dessen reist, entscheidet",
            "der Aufrufer - 0 ohne Praesenzbit, Ersatzzahl, entfallendes Objekt",
            "oder gar keine Nachricht (docs/beweise/NAK-181.md Paragraph 2.0 E4).",
        ],
        "regeln": {
            "endlichkeit": "NaN und +/-Inf reisen nie als Zahl",
            "betrag": "dez > -308 und dez < 308 am GERUNDETEN Wiretext",
            "ganzzahl": "exakt ganzzahliger Betrag hoechstens 2^53-1, in jeder Schreibweise",
            "stellen": "nichtganzzahlig hoechstens 15 signifikante Dezimalziffern",
        },
        "angenommen": angenommen,
        "verweigert": verweigert,
    })


def evidenz_0p01_paar_wire() -> bytes:
    """`evidenz-0p01-paar-wire-v1.json` — die Byteinstanz der fokussierten
    0,01-dB-Evidenz (NAK-182 R4, Form B).

    SIE HAELT ZAHLEN, NICHT DEN ENCODERAUSGANG. Das ist der Entscheid E4b:
    eine Fixture mit Encoder-Ganzzahlen waere an eine Fliesskommakante
    gebunden — der Verteilungsring speichert `float`, `quantisiere16` rundet
    halbe Werte von null weg, und Bitgleichheit ueber Binaerstaende hinweg
    sagt dieses Repo nirgends zu. Der ABSOLUTE Bandpegel ist ohne zweite
    Implementierung ohnehin nicht vorhersagbar; die DIFFERENZ dagegen schon,
    weil ein skalarer Gain jedes Band um exakt 20*log10(g) verschiebt.

    Wer misst wogegen (M-70: ein Fixture ohne Verbraucher in BEIDEN Sprachen
    waere ein Befund):

      * C++ (B16, `Sonde013EventWireTest`): der echte Encoder erzeugt seinen
        eigenen Wire-Text; daraus kommt `gain_db` je Band mit Bit innerhalb
        `toleranz_db` zurueck, und die Leiter unten misst die Aufloesung.
        Seit NAK-182 Nacharbeit 1 (NR-05) faehrt B16 dabei das MATERIAL aus
        dem Block `material` - Frequenz, Samplerate, Amplitude, Blockgroesse
        und Laufbegrenzung - statt eigener Konstanten, und liest jede Zahl
        aus dem serialisierten `verteilung.p50` statt aus Frame-Arrays.
      * Rust (A4, `sonde013_verdrahtung`): `pre` und `post` gehen durch den
        echten Wirepfad; die Produktmessung gibt `gain_db` je Band wieder,
        und das Bootstrap-Intervall enthaelt ihn.
      * A8 haelt die Datei bytegleich. `pruefe_v3_vertrag.py` liest sie
        NICHT und wird dafuer auch nicht erweitert (NAK-182 MP1-5).

    DIE LEITER (MP1-4): ein einzelnes Pegelpaar im Abstand 0,01 dB beweist
    die Aufloesung NICHT — `quantisiere16` rundet die beiden absoluten Pegel
    getrennt, und bei -30,051/-30,041 liefert sogar Skalierung 10 die
    Ganzzahlen -301 und -300. Zwanzig Stufen sind rundungsphasenunabhaengig:
    `round(x*100 + 1)` unterscheidet sich von `round(x*100)` fuer JEDES x um
    genau 1, und bei Skalierung 10 entstehen ueber 0,19 dB Spanne hoechstens
    drei verschiedene Ganzzahlen statt zwanzig.
    """
    teiler = 100
    pre_db = -30.00
    gain_db = 3.00
    post_db = pre_db + gain_db
    leiter_start = -30.004        # keine Stufe faellt auf eine 0,5-Kante
    leiter_schritte = 20

    # NAK-182 Nacharbeit 1 (NR-05): das MATERIAL steht als Zahl in derselben
    # Datei wie Pegel, Gain und Aufloesung. Bis hierher legte B16 Amplitude,
    # 1000 Hz, 48000 Hz und die Laufbegrenzung LOKAL fest - damit war das
    # zugesagte Material nicht ueber die gemeinsame Datei gebunden, und zwei
    # Verbraucher haetten unbemerkt auf verschiedenem Material messen koennen.
    signal_hz = 1000.0
    samplerate_hz = 48000.0
    amplitude_pre = 0.25
    blockgroesse = 512
    bloecke_hoechstens = 900

    def quant(db: float) -> int:
        """Die Vertragsformel: halbe Werte von null weg, wie `quantisiere16`."""
        x = db * teiler
        return int(x + 0.5) if x >= 0.0 else -int(-x + 0.5)

    leiter = [quant(leiter_start + k * 0.01) for k in range(leiter_schritte)]

    def schnappschuss(db: float, evidenz_id: str, folge: int) -> dict:
        satz = {
            "gitter_id": FEIN,
            "encoding": "q_db_0p01_i16",
            "werte": [quant(db)] * 221,
            "gueltig_bitmap": bitmap(221),
            "saturated": False,
        }
        import copy
        s = copy.deepcopy(GRUND["evidence_snapshot"])
        s["evidence_id"] = evidenz_id
        s["verteilung"] = {"p10": copy.deepcopy(satz),
                           "p50": copy.deepcopy(satz),
                           "p95": copy.deepcopy(satz)}
        s["transport"] = copy.deepcopy(s["transport"])
        s["transport"]["sequence"] = folge
        return s

    return als_text({
        "_kommentar": [
            "NAK-182 R4 (Form B) - die BYTEINSTANZ der fokussierten 0,01-dB-Evidenz.",
            "",
            "Sie haelt ZAHLEN, nicht den Ausgang eines Encoders: Material, Bandpegel,",
            "Gain und Aufloesung stehen als Zahlen da, und die zwei Schnappschuesse",
            "sind aus ihnen mit der Vertragsformel round(db*100) gebaut. Keine der",
            "drei Sprachen erzeugt sie; C++ (B16) und Rust (A4) messen gegen sie,",
            "A8 haelt sie bytegleich.",
            "",
            "NAK-182 Nacharbeit 1 (NR-05): der Block `material` traegt Signalfrequenz,",
            "Samplerate, Amplitude von PRE, Blockgroesse und Laufbegrenzung. B16 liest",
            "sie VON HIER statt aus lokalen Konstanten - sonst waere das zugesagte",
            "Material nicht ueber die gemeinsame Datei gebunden.",
            "",
            "Warum nicht Ganzzahl-Gleichheit mit dem echten Encoder: der",
            "Verteilungsring speichert float, `quantisiere16` rundet halbe Werte von",
            "null weg, und Bitgleichheit ueber Binaerstaende hinweg sagt dieses Repo",
            "nirgends zu. Der absolute Bandpegel ist ohne zweite Implementierung",
            "nicht vorhersagbar; die DIFFERENZ ist es, weil ein skalarer Gain jedes",
            "Band um exakt 20*log10(g) verschiebt.",
        ],
        "material": {
            "zweck": ("Das Material, auf dem die C++-Haelfte PRE und POST faehrt "
                      "(NAK-182 Nacharbeit 1, NR-05). Sinus mit `amplitude_pre` bei "
                      "`signal_hz`, Samplerate `samplerate_hz`, Bloecke zu "
                      "`blockgroesse` Frames, hoechstens `bloecke_hoechstens` davon. "
                      "POST ist dasselbe Material mit Amplitude "
                      "amplitude_pre * 10^(gain_db/20)."),
            "signal_hz": signal_hz,
            "samplerate_hz": samplerate_hz,
            "amplitude_pre": amplitude_pre,
            "blockgroesse": blockgroesse,
            "bloecke_hoechstens": bloecke_hoechstens,
            "dauer_s_hoechstens": round(
                bloecke_hoechstens * blockgroesse / samplerate_hz, 4),
        },
        "aufloesung": {
            "encoding": "q_db_0p01_i16",
            "teiler": teiler,
            "schritt_db": 0.01,
            "rundung": "halbe Werte von null weg (BandGrid.h, quantisiere16)",
        },
        "pegel": {
            "pre_band_db": pre_db,
            "gain_db": gain_db,
            "post_band_db": post_db,
            "toleranz_db": 0.1,
            "pre_ganzzahl": quant(pre_db),
            "post_ganzzahl": quant(post_db),
            "baender": 221,
        },
        "leiter": {
            "zweck": ("Die Aufloesung selbst: zwanzig Pegel im Abstand 0,01 dB "
                      "ergeben zwanzig streng monotone Ganzzahlen, benachbarte "
                      "unterscheiden sich um genau 1."),
            "start_db": leiter_start,
            "schritte": leiter_schritte,
            "schritt_db": 0.01,
            "ganzzahlen": leiter,
        },
        "pre": schnappschuss(pre_db, "aa000000000000000000000000000001", 4001),
        "post": schnappschuss(post_db, "aa000000000000000000000000000002", 4002),
    })


def main(argv: list[str]) -> int:
    nur_pruefen = "--pruefen" in argv
    manifest, dateien, rohdateien = baue()

    print(f"{manifest['anzahl_gueltig']} gueltige, {manifest['anzahl_ungueltig']} ungueltige Fixtures")

    # (Pfad, Bytes) — die Rohtext-Fixtures gehen NICHT durch als_text(), sonst
    # wuerde der Serialisierer genau den Defekt wegformatieren, den sie tragen.
    alle: list[tuple[pathlib.Path, bytes]] = [
        (ZIEL / "MANIFEST.json", als_text(manifest)),
        (ZIEL / "TEXTRIEGEL-FAELLE.json", als_text(textriegel_tabelle())),
        # NAK-181 R4: die Byteinstanz der Wire-Zahlform. Sie liegt NEBEN
        # `gueltig/`, weil sie keine v3-Nachricht ist, sondern eine Tabelle
        # ueber Zahlen - wie `handschlag-v1.json` und `heartbeat-wire-v1.json`.
        (ZIEL / "evidenz-zahlen-wire-v1.json", zahlklassen_wire()),
        # NAK-182 R4 (Form B): die Byteinstanz der fokussierten
        # 0,01-dB-Evidenz. Liegt aus demselben Grund NEBEN `gueltig/`
        # wie die Zahlklassentabelle - sie ist keine einzelne
        # v3-Nachricht, sondern eine Tabelle ueber Pegel und Aufloesung.
        (ZIEL / "evidenz-0p01-paar-wire-v1.json", evidenz_0p01_paar_wire()),
        # NAK-313 Etappe 4 (R-313-6, R-313-13): die Tabelle der
        # Produkteingaenge und die Byteinstanz des lokalen Evidenzwriters
        # (R-313-7). Beide liegen NEBEN `gueltig/`: Tabellen, keine einzelne
        # v3-Nachricht.
        (ZIEL / "PRODUKTEINGAENGE-FAELLE.json", als_text(produkteingaenge_tabelle())),
        (ZIEL / "evidenz-lokal-wire-v1.json", evidenz_lokal_wire()),
        # NAK-313 Etappe 5b (R-313-4, M-89): die Handinstanz des Intentwriters,
        # mit SHA-256 im MANIFEST (Schluessel handinstanzen).
        (ZIEL / "intent-wire-v1.json", intent_wire()),
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
