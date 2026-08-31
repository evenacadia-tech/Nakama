#!/usr/bin/env python3
"""Binaerer Cross-Language-Fixture-Korpus fuer SONDE-005b (Entwurf §66.2).

Schreibt `eq-copilot/fixtures/v3/flatbuffers/{gueltig,ungueltig}/*.bin` und
daneben `MANIFEST.json` mit dem ERWARTETEN Urteil je Fixture — bei Ablehnung
samt der vollstaendigen Verstossmenge.

WARUM DIE ERWARTUNG VON HAND STEHT
----------------------------------
Dasselbe Argument wie beim JSON-Korpus: das Manifest ist die dritte Partei
zwischen C++ und Rust. Waere es aus einer der beiden Engines erzeugt, waere
der Vergleich zirkulaer — die Engine bestaetigte ihre eigene Ausgabe, und ein
gemeinsamer Denkfehler bliebe unsichtbar. Unten steht deshalb JEDE Erwartung
als handgeschriebene Zeile neben der Eingabe, die sie ausloest.

WIE DIE BINAERDATEIEN ENTSTEHEN
-------------------------------
Nicht durch einen selbstgebauten Encoder, sondern durch den GEPINNTEN `flatc`
aus einer JSON-Beschreibung (`flatc -b schema.fbs daten.json`). Das hat drei
Gruende: der Erzeuger ist damit dasselbe Werkzeug, das auch den Lesecode
erzeugt hat; die Fixtures sind im Diff als JSON lesbar; und ein eigener
Encoder waere eine dritte Implementierung des Formats, die niemand prueft.

Strukturell kaputte Puffer (falsche Dateikennung, abgeschnitten, zerstoerter
Offset) kann flatc nicht erzeugen — die entstehen als BYTE-MUTATION eines
gueltigen Puffers und stehen unten in ROHE_MUTATIONEN.

Aufruf:
    py -3.13 tools/eq-copilot/erzeuge_fb_fixtures.py
    py -3.13 tools/eq-copilot/erzeuge_fb_fixtures.py --pruefen

Exitcodes: 0 gruen · 2 Abweichung · 3 Voraussetzung fehlt (flatc).
"""

from __future__ import annotations

import copy
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile

WURZEL = pathlib.Path(__file__).resolve().parents[2]
WERKZEUG = WURZEL / "eq-copilot/schemas/v3/flatbuffers/WERKZEUG.json"
SCHEMA = WURZEL / "eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs"
ZIEL = WURZEL / "eq-copilot/fixtures/v3/flatbuffers"
ZEIGER = WURZEL / "eq-copilot/build/nakama-flatc-pfad-Release.txt"


# ------------------------------------------------------------------ Bausteine

def adresse(nr: int = 3) -> dict:
    """Vier Hex32-Token; `nr` variiert nur die instance_id."""
    return {
        "logon_sid": "S-1-5-21-1111111111-2222222222-3333333333-1001",
        "project_binding_id": "11111111111111111111111111111111",
        "session_epoch": "22222222222222222222222222222222",
        # 32 Kleinbuchstaben-Hexziffern, auch fuer nr >= 10: `str(nr) * 32`
        # waere dort 64 Zeichen lang gewesen und damit selbst ungueltig.
        "instance_id": f"{nr:032x}",
        "runtime_nonce": "44444444444444444444444444444444",
    }


def bitmap(n: int) -> list[int]:
    """Alle n Baender gueltig, Fuellbits des letzten Bytes auf 0.

    Dieselbe Regel wie im JSON-Korpus — dort Erzeugerdisziplin, hier vom
    Leser durchgesetzt, weil ein Bytevektor darauf pruefbar ist.
    """
    bytes_ = [0xFF] * ((n + 7) // 8)
    rest = n % 8
    if rest:
        bytes_[-1] = (1 << rest) - 1
    return bytes_


def baender(n: int, gitter: str, encoding: str = "q_db_0p1_i16") -> dict:
    b = {"gitter": gitter, "encoding": encoding, "gueltig_bitmap": bitmap(n),
         "saturated": False}
    if encoding == "float32":
        b["werte_f32"] = [-12.25] * n
    else:
        b["werte_i16"] = [-123] * n
    return b


def band_stereo(wert: float = 0.5, *, gueltig: bool = True) -> dict:
    return {
        "gitter": "nakama_log64_v1",
        "encoding": "float32",
        "werte_f32": [wert] * 64,
        "gueltig_bitmap": bitmap(64) if gueltig else [0] * 8,
        "saturated": False,
    }


TRANSPORT = {
    "process_context_present": True,
    "transport_epoch": 17,
    "continuity_segment": 3,
    "sequence": 8241,
    "zeitbasis": "project_samples",
    "project_sample_start": 44108200,
    "sample_count": 512,
    "sample_rate": 48000.0,
    "playing": True,
    "recording": False,
    "gueltigkeit": "project_time play_state record_state",
}


def frame(n: int = 64, gitter: str = "nakama_log64_v1",
          encoding: str = "q_db_0p1_i16") -> dict:
    return {
        "transport": copy.deepcopy(TRANSPORT),
        "baender": baender(n, gitter, encoding),
        "metrics_version": 1,
        "aktivitaet": 0.62,
        "lufs_s": -14.3,
        "peak_db": -1.1,
        "crest_db": 12.4,
        "psr_db": 9.8,
        "breite": 0.71,
        "korrelation": 0.42,
    }


def batch(*eintraege: dict) -> dict:
    return {"eintraege": list(eintraege)}


def eintrag(nr: int = 3, **kwargs) -> dict:
    return {"quelle": adresse(nr), "frame": frame(**kwargs)}


def v(pfad: str, regel: str) -> dict:
    return {"pfad": pfad, "regel": regel}


# ------------------------------------------- Minimaler FlatBuffers-Wegweiser

# Nur so viel Formatwissen, wie noetig ist, um EINE Offsetzelle zu FINDEN.
# T2-Runde 4 hat den Fall an einem festen Offset (348) reproduziert; ein fester
# Offset waere beim naechsten Feld still danebengelaufen und das Fixture haette
# etwas anderes geprueft, als sein Name sagt.

def _u32(d: bytes, p: int) -> int:
    return int.from_bytes(d[p:p + 4], "little")


def _i32(d: bytes, p: int) -> int:
    return int.from_bytes(d[p:p + 4], "little", signed=True)


def _u16(d: bytes, p: int) -> int:
    return int.from_bytes(d[p:p + 2], "little")


def _tabelle(d: bytes, zeiger: int) -> int:
    """Folgt einem uoffset an `zeiger` auf die Tabelle."""
    return zeiger + _u32(d, zeiger)


def _feldzelle(d: bytes, tabelle: int, slot: int) -> int | None:
    """Position der Zelle fuer `slot` (VT_-Konstante), oder None wenn nicht gesetzt."""
    vtable = tabelle - _i32(d, tabelle)
    if slot >= _u16(d, vtable):
        return None
    voffset = _u16(d, vtable + slot)
    return None if voffset == 0 else tabelle + voffset


def zelle_schleife(d: bytes) -> int:
    """Position der `schleife`-Offsetzelle im ersten Eintrag.

    Wegstrecke: Wurzel -> FeatureBatch -> eintraege[0] -> frame -> transport,
    dann Slot VT_SCHLEIFE. Die Slotnummern folgen aus den eingefrorenen
    Feld-IDs (FELD-IDS.json): voffset = 4 + 2*id.
    """
    batch = _tabelle(d, 0)
    z = _feldzelle(d, batch, 4)                 # FeatureBatch.eintraege, id 0
    assert z is not None, "eintraege fehlt"
    vektor = _tabelle(d, z)
    eintrag = _tabelle(d, vektor + 4)           # erstes Element
    z = _feldzelle(d, eintrag, 6)               # QuellenEintrag.frame, id 1
    assert z is not None, "frame fehlt"
    frame = _tabelle(d, z)
    z = _feldzelle(d, frame, 4)                 # Frame.transport, id 0
    assert z is not None, "transport fehlt"
    transport = _tabelle(d, z)
    z = _feldzelle(d, transport, 24)            # Transportstempel.schleife, id 10
    assert z is not None, "schleife fehlt - Mutationsquelle muss eine haben"
    return z


# ------------------------------------------------------------------- gueltig

def gueltige() -> list[tuple[str, dict, str]]:
    faelle: list[tuple[str, dict, str]] = []

    faelle.append(("live-64-band", batch(eintrag()),
                   "der Regelfall: Sonde->Broker, genau EIN Eintrag (§33.1: Queuecap 2, "
                   "replace-oldest - dort kann nie gebuendelt werden)"))

    faelle.append(("band-stereo-fehlt-altframe", batch(eintrag()),
                   "altes Frame ohne optionales Feld bleibt gueltig"))

    for name, wert, gueltig, warum in [
        ("band-stereo-bitmap-alle-werte-ungueltig", float("nan"), False,
         "nicht gesetzte Bitmapwerte tragen keine Messbehauptung"),
        ("band-stereo-wert-0", 0.0, True, "0 ist die einschliessliche Untergrenze"),
        ("band-stereo-wert-1", 1.0, True, "1 ist die einschliessliche Obergrenze"),
    ]:
        e = eintrag()
        e["frame"]["band_stereo"] = band_stereo(wert, gueltig=gueltig)
        faelle.append((name, batch(e), warum))

    lokal = eintrag()
    lokal["frame"]["transport"]["zeitbasis"] = "local_monotonic"
    del lokal["frame"]["transport"]["project_sample_start"]
    lokal["frame"]["transport"]["gueltigkeit"] = "play_state record_state"
    faelle.append(("transport-local-monotonic", batch(lokal),
                   "lokale Zeit traegt weder Projektbit noch Projektposition"))

    faelle.append(("evidenz-221-band",
                   batch(eintrag(n=221, gitter="nakama_1_24_oct_30_18k_v1",
                                 encoding="q_db_0p01_i16")),
                   "fokussierte PRE/POST-Evidenz auf dem feinen Gitter (§33.1)"))

    faelle.append(("float32-kreuzstatistik",
                   batch(eintrag(n=221, gitter="nakama_1_24_oct_30_18k_v1",
                                 encoding="float32")),
                   "komplexe Kreuzstatistik darf float32 tragen (§33.1)"))

    faelle.append(("broker-buendel-5",
                   batch(*[eintrag(nr=n) for n in range(1, 6)]),
                   "nur Broker->Main buendelt - typisch vier bis fuenf Sonden (§33.1); "
                   "fuenf VERSCHIEDENE Quellen sind erlaubt, fuenf gleiche nicht"))

    ohne = eintrag()
    for k in ("aktivitaet", "lufs_s", "peak_db", "crest_db", "psr_db", "breite",
              "korrelation"):
        del ohne["frame"][k]
    faelle.append(("ohne-optionale-kennzahlen", batch(ohne),
                   "ein nicht messbarer Wert wird WEGGELASSEN, nicht als 0 oder NaN "
                   "gesendet - das ist die NaN-Ehrlichkeit im Binaerformat"))

    mit_schleife = eintrag()
    mit_schleife["frame"]["transport"]["schleife"] = {
        "active": True, "bounds_valid": True,
        "start_ppq": 918.333333, "end_ppq": 928.75,
        "abgeleitete_grenzen": {"start": 44000000, "ende": 44500000,
                                "herleitung": "validated_block_mapping"},
    }
    mit_schleife["frame"]["transport"]["gueltigkeit"] = (
        "project_time play_state record_state cycle_bounds")
    faelle.append(("mit-schleife", batch(mit_schleife),
                   "Samplegrenzen sind ein ABGELEITETES Feld und tragen ihre Herleitung "
                   "mit (§32.3)"))

    volle_zeit = eintrag()
    volle_zeit["frame"]["transport"]["continuous_time_samples"] = 91238400
    volle_zeit["frame"]["transport"]["input_presentation_latency"] = 0
    volle_zeit["frame"]["transport"]["output_presentation_latency"] = 512
    volle_zeit["frame"]["transport"]["gueltigkeit"] = (
        "project_time play_state record_state cycle_bounds continuous_time "
        "input_presentation_latency output_presentation_latency")
    volle_zeit["frame"]["transport"]["schleife"] = {
        "active": False, "bounds_valid": False,
        "start_ppq": 918.0, "end_ppq": 920.0,
    }
    faelle.append(("alle-validity-bits", batch(volle_zeit),
                   "alle sieben Bits gesetzt (§32.3). Die Latenz 0 ist hier GEMESSEN "
                   "und nicht 'unbekannt' - genau die Unterscheidung, fuer die das "
                   "eigene Bit existiert"))

    nul_sid = eintrag()
    nul_sid["quelle"]["logon_sid"] = "\u0000"
    faelle.append(("sid-ist-ein-nul", batch(nul_sid),
                   "GUELTIG, und das ist kein Versehen: §32.1 sagt, die SID werde "
                   "'nie geparst, nur verglichen' - der Vertrag bindet ausschliesslich "
                   "ihre LAENGE. Ein NUL ist ein voellig normaler Codepunkt, und beide "
                   "Beine zaehlen ihn gleich. Das Fixture steht hier, damit die "
                   "Uebereinstimmung bewiesen ist statt angenommen"))

    faelle.append(("grenze-32-eintraege",
                   batch(*[eintrag(nr=n) for n in range(1, 33)]),
                   "genau 32 Sonden (§53.9) - die Grenze selbst ist noch gueltig"))

    return faelle


# ----------------------------------------------------------------- ungueltig

def ungueltige() -> list[tuple[str, dict, list[dict], str]]:
    faelle: list[tuple[str, dict, list[dict], str]] = []
    P = "/eintraege/0/frame"

    # --- §33.1: hoechstens ein Frame je Quelle -----------------------------
    faelle.append((
        "quelle-doppelt", batch(eintrag(nr=1), eintrag(nr=1)),
        [v("/eintraege/1/quelle/instance_id", "quelle_doppelt")],
        "DIE Regel, die kein Schema ausdruecken kann und die dem Format die zweite "
        "Wrapper-Ebene erspart (§33.1, Pruefbericht Befund D)"))

    faelle.append((
        "eintraege-leer", batch(),
        [v("/eintraege", "eintraege_leer")],
        "ein Batch ohne Eintraege traegt nichts und ist kein Batch"))

    faelle.append((
        "eintraege-33", batch(*[eintrag(nr=n) for n in range(1, 34)]),
        [v("/eintraege", "eintraege_zu_viele")],
        "eine ueber der Systemgrenze aus §53.9 - 32 ist die Grenze, nicht die Erwartung"))

    # --- Bandgitter, Bitmap, Encoding --------------------------------------
    b = batch(eintrag())
    b["eintraege"][0]["frame"]["baender"]["gueltig_bitmap"] = bitmap(64)[:-1]
    faelle.append((
        "bitmap-zu-kurz", b,
        [v(f"{P}/baender/gueltig_bitmap", "bitmap_laenge")],
        "ceil(64/8) = 8 Byte; sieben beschreiben nur 56 Baender"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["baender"]["gueltig_bitmap"] = bitmap(64) + [0]
    faelle.append((
        "bitmap-zu-lang", b,
        [v(f"{P}/baender/gueltig_bitmap", "bitmap_laenge")],
        "und ein Byte zu viel ist genauso falsch wie eines zu wenig"))

    b = batch(eintrag(n=221, gitter="nakama_1_24_oct_30_18k_v1"))
    voll = bitmap(221)
    voll[-1] = 0xFF          # die drei Fuellbits mitgesetzt
    b["eintraege"][0]["frame"]["baender"]["gueltig_bitmap"] = voll
    faelle.append((
        "bitmap-fuellbits", b,
        [v(f"{P}/baender/gueltig_bitmap", "bitmap_fuellbits")],
        "221 Baender belegen im 28. Byte nur fuenf Bits. Werden die drei uebrigen "
        "mitgesetzt, erzeugen zwei Sender fuer DIESELBE Messung zwei verschiedene "
        "Puffer - und ein Bytevergleich waere keine Aussage mehr"))

    b = batch(eintrag(n=221, gitter="nakama_1_24_oct_30_18k_v1"))
    b["eintraege"][0]["frame"]["baender"]["gitter"] = "nakama_log64_v1"
    faelle.append((
        "bandzahl-passt-nicht-zum-gitter", b,
        [v(f"{P}/baender", "bandzahl")],
        "221 Werte unter dem 64er-Gitter. Die Gitter sind als Zahlen EINGEFROREN; "
        "ein Bandsatz, der sich nicht an sie haelt, misst etwas anderes als er "
        "behauptet. NUR ein Verstoss: die Bitmap wird gegen die TATSAECHLICHE "
        "Werteanzahl geprueft (221 -> 28 Byte, und 28 sind es), nicht gegen die vom "
        "Gitter geforderte - sonst waere sie eine zweite Klage ueber dieselbe Ursache"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["baender"]["encoding"] = "float32"
    faelle.append((
        "encoding-passt-nicht-zu-i16", b,
        [v(f"{P}/baender", "encoding_passt_nicht")],
        "§33.1 woertlich: 'Empfaenger raten die Skalierung nie aus dem Nachrichtentyp'. "
        "Sie steht im Batch - und wenn sie nicht zur Nutzlast passt, ist der Batch "
        "falsch, nicht auslegbar"))

    b = batch(eintrag(encoding="float32"))
    b["eintraege"][0]["frame"]["baender"]["encoding"] = "q_db_0p1_i16"
    faelle.append((
        "encoding-passt-nicht-zu-f32", b,
        [v(f"{P}/baender", "encoding_passt_nicht")],
        "dieselbe Regel in der anderen Richtung"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["baender"]["werte_f32"] = [-12.25] * 64
    faelle.append((
        "zwei-traeger", b,
        [v(f"{P}/baender", "zwei_traeger")],
        "beide Traeger gesetzt hiesse zwei Wahrheiten ueber dieselbe Messung"))

    b = batch(eintrag())
    del b["eintraege"][0]["frame"]["baender"]["werte_i16"]
    faelle.append((
        "kein-traeger", b,
        [v(f"{P}/baender", "kein_traeger")],
        "und keiner gesetzt hiesse ein Bandsatz ohne Baender"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["baender"]["werte_i16"] = [-123] * 63 + [30000]
    faelle.append((
        "bandwert-ausser-bereich", b,
        [v(f"{P}/baender/werte_i16/63", "bandwert_bereich")],
        "q_db_0p1_i16 traegt -144.0 .. +24.0 dB, also -1440 .. 240 (quantisierung-v1.json)"))

    b = batch(eintrag(n=221, gitter="nakama_1_24_oct_30_18k_v1", encoding="float32"))
    werte = [-12.25] * 221
    werte[7] = float("inf")
    b["eintraege"][0]["frame"]["baender"]["werte_f32"] = werte
    faelle.append((
        "bandwert-nicht-endlich", b,
        [v(f"{P}/baender/werte_f32/7", "nicht_endlich")],
        "quantisierung-v1.json: Nichtendliches wird beim ERZEUGEN zu Wert 0 mit "
        "gueltig=false. Auf der Leitung ist es ein Senderfehler"))

    # --- Enums: FlatBuffers prueft ihren Bereich NICHT ---------------------
    b = batch(eintrag())
    b["eintraege"][0]["frame"]["baender"]["gitter"] = "unbekannt"
    faelle.append((
        "enum-unbekannt-gitter", b,
        [v(f"{P}/baender/gitter", "enum_unbekannt")],
        "`unbekannt = 0` ist der Default eines fehlenden Skalarfelds. Waere 0 eine "
        "gueltige Bedeutung, liesse sich 'weggelassen' nicht von 'genau das gemeint' "
        "unterscheiden. Ohne Gitter faellt auch die Bandzahlpruefung weg - deshalb "
        "steht sie hier NICHT in der Erwartung"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["baender"]["encoding"] = "unbekannt"
    faelle.append((
        "enum-unbekannt-encoding", b,
        [v(f"{P}/baender/encoding", "enum_unbekannt")],
        "dieselbe Regel an der Kodierung"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["zeitbasis"] = "unbekannt"
    faelle.append((
        "enum-unbekannt-zeitbasis", b,
        [v(f"{P}/transport/zeitbasis", "enum_unbekannt")],
        "ohne Zeitbasis ist kein Zeitstempel auslegbar (§32.3)"))

    b = batch(eintrag())
    del b["eintraege"][0]["frame"]["transport"]["process_context_present"]
    faelle.append((
        "transport-ohne-context-bit", b,
        [v(f"{P}/transport/process_context_present", "context_bit_fehlt")],
        "§32.3: ohne dieses Bit ist 'der Host hat GAR KEINEN ProcessContext angelegt' "
        "nicht von 'Projektzeit ist ungueltig' zu unterscheiden. Es ist ein Optional "
        "und kein bool mit Default - sonst waere sein Fehlen unsichtbar"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["gueltigkeit"] = 129
    faelle.append((
        "validity-unbekanntes-bit", b,
        [v(f"{P}/transport/gueltigkeit", "validity_unbekanntes_bit")],
        "Bit 7 ist keins der sieben aus §32.3. FlatBuffers prueft Bitflags beim "
        "Verifizieren NICHT - das muss der Leser tun"))

    # --- NAK-29: dieselben sechs Relationen wie im JSON-Vertrag -----------
    b = batch(eintrag())
    del b["eintraege"][0]["frame"]["transport"]["project_sample_start"]
    faelle.append((
        "transport-project-samples-ohne-project-sample-start", b,
        [v(f"{P}/transport/project_sample_start", "project_sample_start_fehlt")],
        "project_samples verlangt einen expliziten Projektstart"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["gueltigkeit"] = "play_state record_state"
    faelle.append((
        "transport-project-samples-ohne-project-time-bit", b,
        [v(f"{P}/transport/gueltigkeit", "project_time_bit_fehlt")],
        "project_samples verlangt das project_time-Bit"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["zeitbasis"] = "local_monotonic"
    del b["eintraege"][0]["frame"]["transport"]["project_sample_start"]
    faelle.append((
        "transport-local-monotonic-mit-project-time-bit", b,
        [v(f"{P}/transport/gueltigkeit", "local_project_time_bit")],
        "local_monotonic verlangt ein geloeschtes project_time-Bit"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["zeitbasis"] = "local_monotonic"
    b["eintraege"][0]["frame"]["transport"]["gueltigkeit"] = "play_state record_state"
    faelle.append((
        "transport-local-monotonic-mit-project-sample-start", b,
        [v(f"{P}/transport/project_sample_start", "local_project_sample_start")],
        "lokale Zeit darf keine Projektposition behaupten"))

    for name, fehlt, regel in [
        ("schleife-bounds-valid-ohne-start-ppq", "start_ppq", "cycle_start_ppq_fehlt"),
        ("schleife-bounds-valid-ohne-end-ppq", "end_ppq", "cycle_end_ppq_fehlt"),
    ]:
        b = batch(eintrag())
        s = {"active": True, "bounds_valid": True, "start_ppq": 1.0, "end_ppq": 2.0}
        del s[fehlt]
        b["eintraege"][0]["frame"]["transport"]["schleife"] = s
        faelle.append((name, b, [v(f"{P}/transport/schleife/{fehlt}", regel)],
                       "bounds_valid=true verlangt beide PPQ-Grenzen"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["schleife"] = {
        "active": False, "bounds_valid": False,
        "abgeleitete_grenzen": {"start": 1, "ende": 2,
                                "herleitung": "validated_block_mapping"},
    }
    faelle.append((
        "schleife-bounds-invalid-mit-validated-block-mapping", b,
        [v(f"{P}/transport/schleife/abgeleitete_grenzen/herleitung",
           "validated_mapping_ohne_bounds")],
        "bounds_valid=false darf keine validierte Sampleableitung behaupten"))

    for name, fehlt, regel in [
        ("validity-cycle-bounds-ohne-start-ppq", "start_ppq",
         "cycle_bounds_start_ppq_fehlt"),
        ("validity-cycle-bounds-ohne-end-ppq", "end_ppq",
         "cycle_bounds_end_ppq_fehlt"),
    ]:
        b = batch(eintrag())
        b["eintraege"][0]["frame"]["transport"]["gueltigkeit"] = (
            "project_time play_state record_state cycle_bounds")
        s = {"active": False, "bounds_valid": False, "start_ppq": 1.0, "end_ppq": 2.0}
        del s[fehlt]
        b["eintraege"][0]["frame"]["transport"]["schleife"] = s
        faelle.append((name, b, [v(f"{P}/transport/schleife/{fehlt}", regel)],
                       "cycle_bounds-Bit bescheinigt beide rohen PPQ-Grenzen"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["gueltigkeit"] = (
        "project_time play_state record_state continuous_time")
    faelle.append((
        "validity-continuous-time-ohne-wert", b,
        [v(f"{P}/transport/continuous_time_samples", "continuous_time_samples_fehlt")],
        "continuous_time-Bit verlangt den optionalen Samplewert"))

    # --- NAK-59: optionaler 64er-float32-Stereobandsatz ------------------
    def stereo_fall() -> dict:
        e = eintrag()
        e["frame"]["band_stereo"] = band_stereo()
        return batch(e)

    b = stereo_fall()
    b["eintraege"][0]["frame"]["band_stereo"]["werte_f32"][0] = float("nan")
    faelle.append(("band-stereo-nan", b,
                   [v(f"{P}/band_stereo/werte_f32/0", "nicht_endlich")],
                   "gesetzter Stereowert muss endlich sein"))

    for name, wert in [("band-stereo-unter-0", -0.01), ("band-stereo-ueber-1", 1.01)]:
        b = stereo_fall()
        b["eintraege"][0]["frame"]["band_stereo"]["werte_f32"][0] = wert
        faelle.append((name, b,
                       [v(f"{P}/band_stereo/werte_f32/0", "band_stereo_bereich")],
                       "gesetzter Stereowert liegt ausserhalb [0,1]"))

    b = stereo_fall()
    b["eintraege"][0]["frame"]["band_stereo"]["saturated"] = True
    faelle.append(("band-stereo-saturated-true", b,
                   [v(f"{P}/band_stereo/saturated", "band_stereo_saturated")],
                   "Stereobreite kennt keine gesaettigte Kodierung"))

    b = stereo_fall()
    b["eintraege"][0]["frame"]["band_stereo"]["gitter"] = "nakama_1_24_oct_30_18k_v1"
    faelle.append(("band-stereo-falsches-gitter", b,
                   [v(f"{P}/band_stereo/gitter", "band_stereo_gitter")],
                   "Band-Stereo ist fest an log64 gebunden"))

    b = stereo_fall()
    b["eintraege"][0]["frame"]["band_stereo"]["encoding"] = "q_db_0p1_i16"
    faelle.append(("band-stereo-falsches-encoding", b,
                   [v(f"{P}/band_stereo/encoding", "band_stereo_encoding")],
                   "Band-Stereo wird nie quantisiert als dB gelesen"))

    b = stereo_fall()
    b["eintraege"][0]["frame"]["band_stereo"]["werte_i16"] = [0] * 64
    faelle.append(("band-stereo-werte-i16-gesetzt", b,
                   [v(f"{P}/band_stereo/werte_i16", "band_stereo_werte_i16")],
                   "der i16-Traeger ist fuer Band-Stereo verboten"))

    b = stereo_fall()
    b["eintraege"][0]["frame"]["band_stereo"]["werte_f32"] = [0.5] * 63
    faelle.append(("band-stereo-laenge-falsch", b,
                   [v(f"{P}/band_stereo/werte_f32", "band_stereo_bandzahl")],
                   "Band-Stereo traegt genau 64 Werte"))

    b = stereo_fall()
    b["eintraege"][0]["frame"]["band_stereo"]["gueltig_bitmap"] = [0xff] * 7
    faelle.append(("band-stereo-bitmap-laenge-falsch", b,
                   [v(f"{P}/band_stereo/gueltig_bitmap", "band_stereo_bitmap_laenge")],
                   "64 Werte verlangen genau acht Bitmapbytes"))

    # --- Zahlen- und Textgrenzen -------------------------------------------
    for name, feld, wert, regel, warum in [
        ("sample-rate-null", "sample_rate", 0.0, "sample_rate_bereich",
         "eine Samplerate 0 ist keine Messlage"),
        ("sample-rate-zu-hoch", "sample_rate", 768001.0, "sample_rate_bereich",
         "dieselbe Obergrenze wie im JSON-Vertrag (audio_lage)"),
        ("sample-count-zu-gross", "sample_count", 1048577, "sample_count_bereich",
         "dieselbe Obergrenze wie im JSON-Vertrag (transportstempel)"),
    ]:
        b = batch(eintrag())
        b["eintraege"][0]["frame"]["transport"][feld] = wert
        faelle.append((name, b, [v(f"{P}/transport/{feld}", regel)], warum))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["sample_rate"] = float("-inf")
    faelle.append((
        "sample-rate-nicht-endlich", b,
        [v(f"{P}/transport/sample_rate", "nicht_endlich")],
        "Minus unendlich ist keine Abtastrate. Nichtendlichkeit ist eine eigene "
        "Vertragsverletzung; die endlichen Grenzen pruefen sample-rate-null und "
        "sample-rate-zu-hoch"))

    b = batch(eintrag())
    b["eintraege"][0]["quelle"]["instance_id"] = "3" * 31 + "A"
    faelle.append((
        "hex32-grossbuchstabe", b,
        [v("/eintraege/0/quelle/instance_id", "hex32")],
        "hex32 ist KLEINbuchstaben-hex - sonst waeren zwei Schreibweisen desselben "
        "Tokens zwei verschiedene Adressen"))

    b = batch(eintrag())
    b["eintraege"][0]["quelle"]["logon_sid"] = ""
    faelle.append((
        "sid-leer", b,
        [v("/eintraege/0/quelle/logon_sid", "sid_laenge")],
        "die SID wird nie geparst, aber ohne sie gibt es keine Nutzergrenze"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["metrics_version"] = 0
    faelle.append((
        "metrics-version-null", b,
        [v(f"{P}/metrics_version", "metrics_version")],
        "eine Kennzahl ohne Version laesst sich spaeter nicht vergleichen"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["korrelation"] = 1.5
    faelle.append((
        "korrelation-ueber-1", b,
        [v(f"{P}/korrelation", "korrelation_bereich")],
        "Korrelation liegt in [-1, 1]"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["breite"] = -0.5
    faelle.append((
        "breite-negativ", b,
        [v(f"{P}/breite", "breite_negativ")],
        "eine negative Breite gibt es nicht"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["lufs_s"] = float("nan")
    faelle.append((
        "kennzahl-nan", b,
        [v(f"{P}/lufs_s", "nicht_endlich")],
        "NaN wird abgelehnt, nicht sanitisiert: ein nicht messbarer Wert wird "
        "WEGGELASSEN (siehe gueltig/ohne-optionale-kennzahlen)"))

    # Jede optionale Frame-Kennzahl hat ein EIGENES Nichtendlich-Fixture. Ein
    # Sammelfall wuerde nur beweisen, dass irgendein Feld aus der Leserschleife
    # faellt; diese Einzelpuffer machen jede ausgelassene Zeile lokalisierbar.
    # `lufs_s` ist oben bereits durch `kennzahl-nan` gedeckt und wird deshalb
    # hier nicht ein zweites Mal erzeugt.
    for name, feld, wert, warum in [
        ("aktivitaet-nicht-endlich", "aktivitaet", float("inf"),
         "Plus unendlich ist keine Aktivitaetskennzahl"),
        ("peak-db-nicht-endlich", "peak_db", float("-inf"),
         "Minus unendlich wird weggelassen statt uebertragen"),
        ("crest-db-nicht-endlich", "crest_db", float("nan"),
         "NaN ist kein Crest-Faktor"),
        ("psr-db-nicht-endlich", "psr_db", float("inf"),
         "Plus unendlich ist kein PSR-Messwert"),
        ("breite-nicht-endlich", "breite", float("-inf"),
         "Minus unendlich ist keine Stereobreite"),
        ("korrelation-nicht-endlich", "korrelation", float("nan"),
         "NaN ist keine Korrelation"),
    ]:
        b = batch(eintrag())
        b["eintraege"][0]["frame"][feld] = wert
        faelle.append((name, b, [v(f"{P}/{feld}", "nicht_endlich")], warum))

    # --- T2-Runde 3: eingebettete NUL in laengenbehafteten Strings ----------
    #
    # FlatBuffers-Strings tragen eine EXPLIZITE Laenge und duerfen ein NUL
    # enthalten; flatc erzeugt solche Puffer aus einem u0000-Escape anstandslos.
    # Die C++-Seite las sie vorher ueber `c_str()` und brach am NUL ab - zwei
    # Wahrheiten ueber dieselbe Zeichenkette, und die Urteile liefen in BEIDE
    # Richtungen auseinander.

    b = batch(eintrag())
    b["eintraege"][0]["quelle"]["project_binding_id"] = "1" * 32 + "\u0000zz"
    faelle.append((
        "hex32-nul-dahinter", b,
        [v("/eintraege/0/quelle/project_binding_id", "hex32")],
        "32 Hexziffern, dann ein NUL und Muell. Ueber `c_str()` gelesen sah das "
        "wie ein gueltiges hex32 aus; ueber die Laenge gelesen sind es 35 Zeichen"))

    b = batch(eintrag())
    b["eintraege"][0]["quelle"]["logon_sid"] = "a" * 184 + "\u0000" + "b" * 400
    faelle.append((
        "sid-nul-dahinter", b,
        [v("/eintraege/0/quelle/logon_sid", "sid_laenge")],
        "184 Zeichen, dann ein NUL und 400 weitere. Ueber `c_str()` gelesen war "
        "die SID 184 lang und damit gerade noch zulaessig; ueber die Laenge "
        "gelesen sind es 585"))

    # --- Schleife -----------------------------------------------------------
    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["schleife"] = {
        "active": True, "bounds_valid": True,
        "start_ppq": 1.0, "end_ppq": 2.0,
        "abgeleitete_grenzen": {"start": 44500000, "ende": 44000000,
                                "herleitung": "validated_block_mapping"}}
    faelle.append((
        "grenzen-verdreht", b,
        [v(f"{P}/transport/schleife/abgeleitete_grenzen", "grenzen_verdreht")],
        "ein Bereich, dessen Ende vor seinem Anfang liegt"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["schleife"] = {
        "active": True, "bounds_valid": True,
        "start_ppq": 1.0, "end_ppq": 2.0,
        "abgeleitete_grenzen": {"start": 1, "ende": 2, "herleitung": "unbekannt"}}
    faelle.append((
        "herleitung-unbekannt", b,
        [v(f"{P}/transport/schleife/abgeleitete_grenzen/herleitung", "enum_unbekannt")],
        "abgeleitete Grenzen OHNE Herleitung sind eine Behauptung ohne Beleg (§32.3)"))

    # --- G1-Befund §4.3: die zwei ungeprueften Fliesskomma-Traeger ----------
    #
    # `start_ppq` und `end_ppq` waren bis zum 24.08.2026 die einzigen der vier
    # Float-Gruppen des Vertrags, die in BEIDEN handgeschriebenen Lesern
    # ungeprueft blieben. Kein Bein sah es, weil der Kreuzsprachtest die beiden
    # Leser GEGENEINANDER haelt - und sie waren sich einig. Ein Vergleich
    # zweier Spiegel findet keine gemeinsame Auslassung; erst ein Fixture, das
    # von aussen kommt, tut es. `grep -c ppq MANIFEST.json` war 0.
    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["schleife"] = {
        "active": True, "bounds_valid": True,
        "start_ppq": float("nan"), "end_ppq": 928.75}
    faelle.append((
        "ppq-start-nan", b,
        [v(f"{P}/transport/schleife/start_ppq", "nicht_endlich")],
        "NaN ist keine Position in der Zeitbasis des Hosts - dieselbe Regel, "
        "die sample_rate, werte_f32 und die sieben Frame-Kennzahlen laengst "
        "hatten"))

    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["schleife"] = {
        "active": True, "bounds_valid": True,
        "start_ppq": 918.333333, "end_ppq": float("inf")}
    faelle.append((
        "ppq-ende-unendlich", b,
        [v(f"{P}/transport/schleife/end_ppq", "nicht_endlich")],
        "Unendlich ist kein Schleifenende"))

    # Beide nicht endlich: die Verstossmenge traegt BEIDE Zeilen. Ein Leser,
    # der beim ersten Fund abbricht, faellt hier auf - er meldete nur eine.
    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["schleife"] = {
        "active": True, "bounds_valid": True,
        "start_ppq": float("nan"), "end_ppq": float("-inf")}
    faelle.append((
        "ppq-beide-nicht-endlich", b,
        [v(f"{P}/transport/schleife/end_ppq", "nicht_endlich"),
         v(f"{P}/transport/schleife/start_ppq", "nicht_endlich")],
        "beide Traeger fallen einzeln und mit eigenem Pfad"))

    # Die Ordnungsregel, die `grenzen_verdreht` eine Ebene tiefer schon hatte -
    # in der Zeitbasis des Hosts fehlte sie. Nur bei `bounds_valid`: ohne das
    # Bit behauptet §32.3 ueber die Grenzen nichts.
    b = batch(eintrag())
    b["eintraege"][0]["frame"]["transport"]["schleife"] = {
        "active": True, "bounds_valid": True,
        "start_ppq": 928.75, "end_ppq": 918.333333}
    faelle.append((
        "ppq-verdreht", b,
        [v(f"{P}/transport/schleife", "ppq_verdreht")],
        "eine Schleife, deren Ende vor ihrem Anfang liegt"))

    return faelle


# ------------------------------------------------- strukturelle Byte-Mutationen

def rohe_mutationen() -> list[tuple[str, str, object, list[dict], str]]:
    """(name, quelle, mutation, verstoesse, warum).

    Diese Puffer kann `flatc` nicht erzeugen - ein Serialisierer schreibt keine
    falsche Dateikennung und keinen zerstoerten Offset. Sie entstehen als
    Byte-Mutation eines GUELTIGEN Puffers, damit drumherum eine echte Nachricht
    stehen bleibt und der Defekt genau einer ist.
    """
    def kennung_falsch(roh: bytes) -> bytes:
        # Die Dateikennung steht an Offset 4..8.
        return roh[:4] + b"XXXX" + roh[8:]

    def abgeschnitten(roh: bytes) -> bytes:
        return roh[: len(roh) // 2]

    def sid_ungueltiges_utf8(roh: bytes) -> bytes:
        """Ein nacktes Fortsetzungsbyte 0x80 in die SID.

        Gesucht wird ueber den INHALT, nicht ueber einen festen Offset - ein
        fester Offset waere beim naechsten Feld still danebengelaufen und das
        Fixture haette etwas anderes geprueft, als es behauptet.
        """
        marke = b"S-1-5-21-"
        i = roh.find(marke)
        if i < 0:
            raise SystemExit("SID-Marke nicht im Puffer gefunden")
        d = bytearray(roh)
        d[i + len(marke)] = 0x80
        return bytes(d)

    def auf_laenge(n: int):
        return lambda roh: roh[:n]

    def schleife_selbstbezug(roh: bytes) -> bytes:
        """Setzt die `schleife`-Offsetzelle auf 0 — ein Selbstbezug.

        T2-Runde 4, BL-A: der C++-Verifier lehnt das ab ("May not point to
        itself"), der Rust-Verifier folgte dem Offset und las eine vtable der
        Laenge 0 — also eine Tabelle OHNE JEDES FELD, deren lauter Defaults
        semantisch unauffaellig sind. Ein Byte, und die beiden Beine sagten
        Gegenteiliges. Die Zelle wird STRUKTURELL gesucht, nicht gezaehlt.
        """
        zelle = zelle_schleife(roh)
        d = bytearray(roh)
        d[zelle:zelle + 4] = b"" + bytes(4)
        return bytes(d)

    def sid_bytes(*ersatz: int):
        """Schreibt eine Bytefolge in die SID, ohne ihre Laenge zu aendern.

        Damit lassen sich die klassischen UTF-8-Kanten pruefen, die `flatc` aus
        JSON nicht erzeugen KANN: eine Ueberlangkodierung, ein Surrogat und ein
        Codepunkt jenseits von U+10FFFF. Der handgeschriebene C++-Pruefer muss
        dort dasselbe sagen wie Rusts `str::from_utf8`.
        """
        def mutiere(roh: bytes) -> bytes:
            marke = b"S-1-5-21-"
            i = roh.find(marke)
            if i < 0:
                raise SystemExit("SID-Marke nicht im Puffer gefunden")
            d = bytearray(roh)
            for k, byte in enumerate(ersatz):
                d[i + len(marke) + k] = byte
            return bytes(d)
        return mutiere

    def wurzeloffset_kaputt(roh: bytes) -> bytes:
        # Die ersten vier Bytes sind der Offset auf die Wurzeltabelle. Ein Wert
        # weit hinter dem Puffer muss den Verifier ausloesen, nicht einen
        # Absturz.
        return (0x7FFFFFF0).to_bytes(4, "little") + roh[4:]

    return [
        ("dateikennung-falsch", "live-64-band", kennung_falsch,
         [v("", "dateikennung")],
         "vier Bytes an Offset 4. Das Erste, was ein Leser sehen kann - ein fremder "
         "Puffer faellt hier, VOR jedem Feldzugriff"),

        ("abgeschnitten", "live-64-band", abgeschnitten,
         [v("", "verifier")],
         "die halbe Nachricht. Der Verifier ist der eine Punkt, an dem FlatBuffers "
         "wirklich hart ist - ohne ihn waere jeder Feldzugriff undefiniert"),

        ("wurzeloffset-kaputt", "live-64-band", wurzeloffset_kaputt,
         [v("", "verifier")],
         "ein Wurzeloffset weit hinter dem Puffer muss den Verifier ausloesen, nicht "
         "einen Absturz"),

        # --- T2-Runde 3, der schwerste Fund der ganzen Sitzung --------------
        ("sid-ungueltiges-utf8", "mit-schleife", sid_ungueltiges_utf8,
         [v("", "verifier")],
         "EIN Byte (0x80 in der SID). Vorher: die Rust-Seite lehnte ab, die C++-Seite "
         "sagte 'gueltig' - und ab der zweiten Verarbeitung starb der Prozess mit "
         "STATUS_HEAP_CORRUPTION. Ursache: Rusts FlatBuffers-Verifier prueft "
         "Stringinhalte auf gueltiges UTF-8, der C++-Verifier NICHT, und "
         "juce::String::fromUTF8 laeuft auf einem nackten Fortsetzungsbyte im "
         "Laengen- und im Kopierdurchlauf verschieden weit. Der Vertrag war darauf "
         "gebaut, dass sich DIESELBE Bibliothek in zwei Sprachen gleich verhaelt"),

        ("puffer-leer",   "live-64-band", auf_laenge(0), [v("", "dateikennung")],
         "0 Byte. Die Rust-Seite PANICKTE hier vorher im flatbuffers-Crate "
         "(data.len() >= SIZE_UOFFSET + FILE_IDENTIFIER_LENGTH), waehrend C++ sauber "
         "urteilte. Ein Panic im Broker beendet den Thread, der die Pipe bedient"),
        ("puffer-vier",   "live-64-band", auf_laenge(4), [v("", "dateikennung")],
         "vier Byte - gerade der Wurzeloffset, noch keine Dateikennung"),
        ("puffer-sieben", "live-64-band", auf_laenge(7), [v("", "dateikennung")],
         "sieben Byte - ein Byte zu wenig fuer die Kennung. Ab acht einigen sich "
         "beide Beine auf `verifier`"),

        # --- die klassischen UTF-8-Kanten ----------------------------------
        # Der C++-Pruefer ist handgeschrieben und muss `str::from_utf8` in
        # JEDER dieser drei Ecken treffen, nicht nur beim nackten
        # Fortsetzungsbyte. flatc kann sie aus JSON nicht erzeugen - eine
        # Ueberlangkodierung ist per Definition das, was ein Serialisierer
        # NICHT schreibt.
        ("sid-ueberlang", "mit-schleife", sid_bytes(0xC0, 0x80),
         [v("", "verifier")],
         "C0 80 kodiert U+0000 in zwei Byte - eine Ueberlangkodierung. Sie ist der "
         "klassische Weg, einen Filter zu umgehen, der nur die kurze Form kennt"),
        ("sid-surrogat", "mit-schleife", sid_bytes(0xED, 0xA0, 0x80),
         [v("", "verifier")],
         "ED A0 80 ist U+D800, die Haelfte eines Surrogatpaars. In UTF-8 gibt es "
         "keine Surrogate - sie sind eine UTF-16-Eigenheit"),
        ("sid-ueber-10ffff", "mit-schleife", sid_bytes(0xF5, 0x80, 0x80, 0x80),
         [v("", "verifier")],
         "F5 80 80 80 waere U+140000 - jenseits des Unicode-Bereichs, den es gibt"),

        # --- T2-Runde 4: die Gegenrichtung derselben Klasse ----------------
        ("offset-selbstbezug", "mit-schleife", schleife_selbstbezug,
         [v("", "verifier")],
         "EIN Byte: die `schleife`-Offsetzelle auf 0. Der C++-Verifier lehnt das ab "
         "('May not point to itself'), der Rust-Verifier hatte diese Pruefung NICHT - "
         "er folgte dem Selbstbezug, las eine vtable der Laenge 0 und damit eine "
         "Tabelle ohne jedes Feld, deren lauter Defaults semantisch unauffaellig sind. "
         "Die Richtung ist die gefaehrliche - der Broker sitzt ZWISCHEN Sonde und "
         "Main und haette so einen Batch als gueltig durchgereicht. Gemessen wurde "
         "die KLASSE, nicht dieser eine Fall: jede 4-byte-ausgerichtete Zelle ab "
         "Offset 4 aller neun gueltigen Fixtures einzeln auf 0 (6215 Puffer, beide "
         "Beine, voll verglichene Verstossmenge) - 143 liefen auseinander, mit dem "
         "Strukturriegel in broker/src/telemetrie.rs sind es 0"),
    ]


# ------------------------------------------------------------------- Erzeugung

def finde_flatc() -> pathlib.Path | None:
    if not ZEIGER.exists():
        return None
    kandidat = pathlib.Path(ZEIGER.read_text(encoding="utf-8").strip())
    return kandidat if kandidat.exists() else None


def nach_binaer(flatc: pathlib.Path, tmp: pathlib.Path, name: str, daten: dict) -> bytes:
    quelle = tmp / f"{name}.json"
    quelle.write_text(json.dumps(daten, indent=2, ensure_ascii=False, allow_nan=True),
                      encoding="utf-8", newline="")
    lauf = subprocess.run(
        [str(flatc), "-b", "--strict-json", "-o", str(tmp), str(SCHEMA), str(quelle)],
        capture_output=True, text=True)
    if lauf.returncode != 0:
        # `--strict-json` verbietet `nan`; fuer genau diese Faelle noch einmal ohne.
        lauf = subprocess.run(
            [str(flatc), "-b", "-o", str(tmp), str(SCHEMA), str(quelle)],
            capture_output=True, text=True)
    if lauf.returncode != 0:
        raise SystemExit(f"flatc scheiterte an {name}: {(lauf.stdout + lauf.stderr).strip()}")
    erzeugt = tmp / f"{name}.nktb"
    if not erzeugt.exists():
        raise SystemExit(f"flatc erzeugte {erzeugt} nicht")
    return erzeugt.read_bytes()


def baue(flatc: pathlib.Path) -> tuple[dict, dict[str, bytes]]:
    dateien: dict[str, bytes] = {}
    eintraege: list[dict] = []
    gueltige_puffer: dict[str, bytes] = {}

    with tempfile.TemporaryDirectory() as t:
        tmp = pathlib.Path(t)

        for name, daten, warum in gueltige():
            roh = nach_binaer(flatc, tmp, name, daten)
            gueltige_puffer[name] = roh
            pfad = f"gueltig/{name}.bin"
            dateien[pfad] = roh
            eintraege.append({"datei": pfad, "urteil": "gueltig", "warum": warum,
                              "verstoesse": []})

        for name, daten, verstoesse, warum in ungueltige():
            roh = nach_binaer(flatc, tmp, name, daten)
            pfad = f"ungueltig/{name}.bin"
            if pfad in dateien:
                raise SystemExit(f"doppelter Fixturename: {pfad}")
            dateien[pfad] = roh
            eintraege.append({"datei": pfad, "urteil": "ungueltig", "warum": warum,
                              "verstoesse": sorted(verstoesse,
                                                   key=lambda x: (x["pfad"], x["regel"]))})

        for name, quelle, mutiere, verstoesse, warum in rohe_mutationen():
            if quelle not in gueltige_puffer:
                raise SystemExit(f"Mutationsquelle {quelle} gibt es nicht")
            roh = mutiere(gueltige_puffer[quelle])
            if roh == gueltige_puffer[quelle]:
                raise SystemExit(f"Mutation {name} hat nichts geaendert")
            pfad = f"ungueltig/{name}.bin"
            dateien[pfad] = roh
            eintraege.append({"datei": pfad, "urteil": "ungueltig", "warum": warum,
                              "verstoesse": sorted(verstoesse,
                                                   key=lambda x: (x["pfad"], x["regel"])),
                              "byte_mutation_von": f"gueltig/{quelle}.bin"})

    eintraege.sort(key=lambda e: e["datei"])

    manifest = {
        "$id": "evenacadia.nakama.fixtures.v3.flatbuffers",
        "titel": "Binaerer Cross-Language-Fixture-Korpus (SONDE-005b)",
        "schema": "../../../schemas/v3/flatbuffers/nakama_telemetry_v1.fbs",
        "zweck": ("Die dritte Partei zwischen C++ und Rust fuer den BINAEREN Teil des "
                  "v3-Vertrags. Beide Leser messen gegen DIESES Manifest; stimmen beide "
                  "mit ihm ueberein, stimmen sie transitiv miteinander ueberein. Die "
                  "Erwartungen sind von Hand geschrieben, nicht aus einem Leser erzeugt "
                  "— sonst waere der Vergleich zirkulaer."),
        "erzeugung": ("Die .bin-Dateien entstehen mit dem GEPINNTEN flatc aus einer "
                      "JSON-Beschreibung (flatc -b). Ein eigener Encoder waere eine "
                      "dritte Implementierung des Formats, die niemand prueft."),
        "byte_mutation_von": ("Markiert einen Puffer, den flatc nicht erzeugen KANN "
                              "(falsche Dateikennung, abgeschnitten, zerstoerter "
                              "Wurzeloffset). Er entsteht als Byte-Mutation des "
                              "genannten gueltigen Puffers."),
        "regeln": ("Die geschlossene Liste der Regelnamen steht in "
                   "eq-copilot/schemas/v3/flatbuffers/README.md. Ein Verstoss ist "
                   "(pfad, regel); die Menge wird kanonisch nach (pfad, regel) "
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
    if not WERKZEUG.exists() or not SCHEMA.exists():
        print("VORAUSSETZUNG FEHLT: Werkzeugsteckbrief oder Schema nicht gefunden")
        return 3
    flatc = finde_flatc()
    if flatc is None:
        print("VORAUSSETZUNG FEHLT: flatc nicht gefunden.")
        print("  cmake --build eq-copilot/build --config Release --target flatc")
        return 3

    nur_pruefen = "--pruefen" in argv
    manifest, dateien = baue(flatc)
    print(f"{manifest['anzahl_gueltig']} gueltige, {manifest['anzahl_ungueltig']} "
          "ungueltige Fixtures")

    alle: list[tuple[pathlib.Path, bytes]] = [(ZIEL / "MANIFEST.json", als_text(manifest))]
    alle += [(ZIEL / p, b) for p, b in sorted(dateien.items())]

    if nur_pruefen:
        vorhanden = {p.relative_to(ZIEL).as_posix()
                     for p in ZIEL.rglob("*") if p.is_file()} if ZIEL.exists() else set()
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
