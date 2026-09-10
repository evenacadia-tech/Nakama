#!/usr/bin/env python3
"""State-Fixtures fuer SONDE-006 (State-Schema 2, Parameterbestand, state_hash).

Erzeugt und prueft eq-copilot/fixtures/state/:

    jcs/zahlen.json          RFC-8785-Zahlenvektoren (IEEE-754-Hex -> Erwartung)
    jcs/dokumente/*.json     JCS-Dokumente (Eingabe) + Erwartung im MANIFEST
    dto/gueltig/*.json       DSP-DTOs, die gehasht werden
    dto/ungueltig/*.json     DSP-DTOs, die VOR dem Hash fallen (mit Grund)
    preset/gueltig/*.json    Presets in der Form des Writers (RFC-8785-kanonisch)
    preset/ungueltig/*.json  Presets, die der Leser abweist (mit Grund)
    MANIFEST.json            die dritte Partei zwischen C++, Rust und Python

und prueft den Vertrag eq-copilot/schemas/state/nakama-parameter-v2.json
(120 Kennungen, davon 112 Host-Parameter, eindeutige IDs, Defaults im
Bereich) samt der ABLEITUNG aus der eingefrorenen v1-Datei sowie das
Presetschema eq-copilot/schemas/state/nakama-preset-v1.json.

WARUM DIE ERWARTUNGEN VON HAND GESCHRIEBEN SIND
-----------------------------------------------
Die Zahlenvektoren aus RFC 8785 Anhang B stehen hier mit dem Text, den der
RFC DRUCKT. Die Python-Referenz (`rfc8785`, Trail of Bits) wird damit selbst
gegen den RFC gemessen - nicht nur C++ und Rust gegen Python. Eigene Vektoren
und Dokumente tragen eine von Hand nach ECMA-262 `Number::toString` bzw.
RFC 8785 §3.2 hergeleitete Erwartung; `rfc8785` muss sie bestaetigen. Stimmt
die Referenz nicht mit der Hand ueberein, bricht der Erzeuger ab - dann ist
entweder die Hand oder die Referenz falsch, und beides ist ein Befund.

Nur fuer die DSP-DTOs (109 Schluessel) liefert `rfc8785` die Erwartung: ein
von Hand sortiertes 109-Schluessel-Dokument waere eine zweite Implementierung,
keine unabhaengige Erwartung. Die JCS-Korrektheit ist an dieser Stelle schon
durch die Handvektoren belegt; das DTO-Fixture prueft, dass C++ und Rust an
einem ECHTEN DTO dieselben Bytes liefern.

Aufruf:
    py -3.13 tools/eq-copilot/erzeuge_state_fixtures.py            # schreiben
    py -3.13 tools/eq-copilot/erzeuge_state_fixtures.py --pruefen  # bytegleich?
"""

from __future__ import annotations

import hashlib
import json
import math
import pathlib
import struct
import sys

try:
    import rfc8785
except ImportError:  # pragma: no cover
    print("FEHLT: py -3.13 -m pip install rfc8785", file=sys.stderr)
    sys.exit(3)

# Der v3-Textriegel (acht Regeln, schemas/v3/README.md) ist die erste Stufe
# des DTO-Lesers in ALLEN Beinen - auch hier, sonst liest Python `1e-400` als
# 0.0 und sagt "gueltig", wo C++ und Rust "nichtendlich" sagen.
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from pruefe_v3_vertrag import textriegel_bytes  # noqa: E402

WURZEL = pathlib.Path(__file__).resolve().parents[2]
VERTRAG = WURZEL / "eq-copilot" / "schemas" / "state" / "nakama-parameter-v2.json"
VERTRAG_V1 = WURZEL / "eq-copilot" / "schemas" / "state" / "nakama-parameter-v1.json"
PRESET = WURZEL / "eq-copilot" / "schemas" / "state" / "nakama-preset-v1.json"
FIXTURES = WURZEL / "eq-copilot" / "fixtures" / "state"
MANIFEST = FIXTURES / "MANIFEST.json"

BS = chr(92)   # Backslash - als chr(), damit keine Werkzeugschicht ihn deutet
DQ = chr(34)   # Anfuehrungszeichen


def hex64(x: float) -> str:
    return format(struct.unpack("<Q", struct.pack("<d", x))[0], "016x")


def aus_hex64(h: str) -> float:
    return struct.unpack("<d", struct.pack("<Q", int(h, 16)))[0]


def sha256_hex(b: bytes) -> str:
    return hashlib.sha256(b).hexdigest()


def json_text(obj) -> str:
    return json.dumps(obj, ensure_ascii=False, indent=2, sort_keys=False) + "\n"


def kanon_text(obj) -> str:
    """RFC-8785-kanonisch - genau das, was `nakama::preset::schreibe` erzeugt."""
    return rfc8785.dumps(obj).decode("utf-8")


# ------------------------------------------------------------------ Vertrag

# Die sechs Namen, die ein Preset NIE traegt (R12, nakama-preset-v1.json).
# Nicht "Identitaetsfelder" allgemein, sondern genau diese sechs: die Regel
# unterscheidet zwischen einem unbekannten Feld (ignorieren) und einem
# verbotenen (ablehnen), und dieser Unterschied braucht eine geschlossene Menge.
PRESET_VERBOTEN = ["adresse", "instance_id", "host_bus_name", "label",
                   "eq_enabled", "undo_ring"]

# Der Klanginhalt: alle Kennungen AUSSER dem Rollenschalter. Abgeleitet, nie
# als zweite Liste gepflegt - zwei Listen driften.
def PRESET_INHALT(v: dict) -> list[str]:
    return [i for i in v["ids"] if i != "v2.global.eq_enabled"]


def pruefe_vertrag() -> dict:
    """Layout v2 (SONDE-015 R1) - und die Ableitung aus dem eingefrorenen v1.

    Der teure Teil ist nicht die Zahl 120, sondern die Zusage, dass die 109
    v1-Kennungen UNVERAENDERT sind. Sie wird hier ALGORITHMISCH gehalten:
    jeder Eintrag mit layout=v1 muss - ohne die zwei neuen Schluessel `layout`
    und `host_parameter` - dem Eintrag gleicher ID in der eingefrorenen
    v1-Datei entsprechen. Aendert jemand einen v1-Default, faellt diese
    Funktion, nicht ein Kommentar (M-86).
    """
    v = json.loads(VERTRAG.read_text(encoding="utf-8"))
    v1 = json.loads(VERTRAG_V1.read_text(encoding="utf-8"))
    fehler: list[str] = []

    if v.get("dsp_schema_version") != 2:
        fehler.append("dsp_schema_version muss 2 sein")
    if v1.get("dsp_schema_version") != 1 or v1.get("anzahl_parameter") != 109:
        fehler.append("die v1-Datei ist nicht mehr eingefroren (dsp_schema_version 1, 109 Kennungen)")
    slots = v.get("slot_anzahl")
    if slots != 8:
        fehler.append("slot_anzahl muss 8 sein (§53.8: Slots 0..7)")

    # Vertragsreihenfolge: erst alle v1-Eintraege (global, dann je Slot), dann
    # die v2-Globalen, dann die v2-Slotfelder. Genau diese Reihenfolge macht
    # die 112 Host-Parameter zum PRAEFIX (M-88).
    g1 = [g for g in v["global"] if g.get("layout") == "v1"]
    g2 = [g for g in v["global"] if g.get("layout") == "v2"]
    b1 = [p for p in v["band_vorlage"] if p.get("layout") == "v1"]
    b2 = [p for p in v["band_vorlage"] if p.get("layout") == "v2"]
    erwartet_ids = [g["id"] for g in g1]
    for slot in range(slots):
        for p in b1:
            erwartet_ids.append(f"v1.band.{slot}.{p['name']}")
    erwartet_ids += [g["id"] for g in g2]
    for slot in range(slots):
        for p in b2:
            erwartet_ids.append(f"v2.band.{slot}.{p['name']}")
    ids = v["ids"]
    if ids != erwartet_ids:
        fehler.append("ids-Liste weicht von der Vertragsreihenfolge ab (109 v1, dann v2 global, dann v2 je Slot)")
    if len(ids) != len(set(ids)):
        fehler.append("IDs nicht eindeutig")
    if len(ids) != v.get("anzahl_parameter"):
        fehler.append(f"anzahl_parameter {v.get('anzahl_parameter')} != {len(ids)}")
    if len(g1) != 5 or len(b1) != 13:
        fehler.append("§53.8 verlangt 5 globale und 13 Slot-Parameter im v1-Bestand")
    if len(g2) != 3 or len(b2) != 1:
        fehler.append("R1 verlangt drei v2-Globale (eq_enabled, mix, auto_gain) und ein v2-Slotfeld (occupied)")
    if len(ids) != 120:
        fehler.append("Summe muss 120 sein")
    if v.get("anzahl_parameter_v1") != 109:
        fehler.append("anzahl_parameter_v1 muss 109 sein")

    # Die 109 v1-IDs stehen unveraendert VORNE.
    if ids[:109] != v1["ids"]:
        fehler.append("die 109 v1-IDs stehen nicht unveraendert am Anfang der v2-Reihenfolge")

    # Host-Parameter: genau die ersten 112, occupied ist keiner.
    nach_id_roh = {g["id"]: g for g in v["global"]}
    for slot in range(slots):
        for p in v["band_vorlage"]:
            nach_id_roh[f"v{p['layout'][1:]}.band.{slot}.{p['name']}"] = p
    host = [i for i in ids if nach_id_roh[i].get("host_parameter") is True]
    if host != ids[:112]:
        fehler.append("die Host-Parameter sind nicht das Praefix der ersten 112 Kennungen")
    if v.get("anzahl_host_parameter") != 112:
        fehler.append("anzahl_host_parameter muss 112 sein")
    if any(nach_id_roh[i].get("host_parameter") is not False for i in ids[112:]):
        fehler.append("occupied darf kein Host-Parameter sein (R5: kein Weg an der Transaktion vorbei)")

    # Die ABLEITUNG: v1-Eintrag woertlich plus genau zwei Schluessel.
    v1_nach_id = {g["id"]: g for g in v1["global"]}
    v1_nach_name = {p["name"]: p for p in v1["band_vorlage"]}
    for eintrag in v["global"] + v["band_vorlage"]:
        if eintrag.get("layout") != "v1":
            continue
        alt = v1_nach_id.get(eintrag.get("id")) or v1_nach_name.get(eintrag.get("name"))
        if alt is None:
            fehler.append(f"v1-Eintrag ohne Gegenstueck in der eingefrorenen Datei: {eintrag}")
            continue
        ohne = {k: w for k, w in eintrag.items() if k not in ("layout", "host_parameter")}
        if ohne != alt:
            fehler.append(f"v1-Eintrag nicht woertlich uebernommen: {ohne.get('id') or ohne.get('name')}")

    namen = {"enabled", "type", "freq_hz", "q", "gain_db", "channel_mode", "dynamic_enabled",
             "dynamic_range_db", "threshold_db", "attack_ms", "hold_ms", "release_ms", "sidechain_source"}
    if {p["name"] for p in b1} != namen:
        fehler.append("Slot-Parameternamen weichen von §53.8 ab")
    if {p["name"] for p in b2} != {"occupied"}:
        fehler.append("das einzige v2-Slotfeld heisst occupied (R5)")
    gl = {"v1.global.bypass", "v1.global.input_trim_db", "v1.global.output_trim_db",
          "v1.global.width", "v1.global.mono_bass_hz"}
    if {g["id"] for g in g1} != gl:
        fehler.append("globale IDs weichen von §53.8 ab")
    if {g["id"] for g in g2} != {"v2.global.eq_enabled", "v2.global.mix", "v2.global.auto_gain"}:
        fehler.append("die drei v2-Globalen sind eq_enabled, mix und auto_gain (R2/R3/R4)")

    def pruefe_eintrag(e: dict, wo: str) -> None:
        typ = e["typ"]
        if e["wechsel"] not in ("rampe", "blockrand"):
            fehler.append(f"{wo}: wechsel unbekannt")
        if typ == "bool":
            if not isinstance(e["default"], bool):
                fehler.append(f"{wo}: bool-Default ist kein bool")
            if e["wechsel"] != "blockrand":
                fehler.append(f"{wo}: bool muss blockrand sein")
        elif typ == "float":
            mn, mx, d = e["min"], e["max"], e["default"]
            for w in (mn, mx, d):
                if not isinstance(w, (int, float)) or isinstance(w, bool) or not math.isfinite(w):
                    fehler.append(f"{wo}: Grenze/Default nicht endlich")
            if not (mn < mx):
                fehler.append(f"{wo}: min < max verletzt")
            if not (mn <= d <= mx):
                fehler.append(f"{wo}: Default ausserhalb des Bereichs")
            if e["wechsel"] != "rampe":
                fehler.append(f"{wo}: float muss rampe sein")
        elif typ == "enum":
            werte = e["werte"]
            if len(werte) != len(set(werte)) or not werte:
                fehler.append(f"{wo}: Enumwerte nicht eindeutig/leer")
            if e["default"] not in werte:
                fehler.append(f"{wo}: Enum-Default nicht in werte")
            if e["wechsel"] != "blockrand":
                fehler.append(f"{wo}: enum muss blockrand sein")
        else:
            fehler.append(f"{wo}: typ unbekannt {typ}")

    for g in v["global"]:
        pruefe_eintrag(g, g["id"])
    for p in v["band_vorlage"]:
        pruefe_eintrag(p, "band." + p["name"])

    # §53.8 nennt vier topologische Slotparameter; R8 nimmt `occupied` dazu -
    # eine Belegung aendert die Bank und wechselt deshalb am Blockrand.
    topo = {p["name"] for p in v["band_vorlage"] if p.get("topologisch")}
    if topo != {"type", "channel_mode", "dynamic_enabled", "sidechain_source", "occupied"}:
        fehler.append("topologisch muss genau type/channel_mode/dynamic_enabled/sidechain_source/occupied markieren (§53.8, R8)")
    if any(g.get("topologisch") for g in v["global"]):
        fehler.append("kein globaler Parameter ist topologisch (R2/R3/R4: nicht topologisch)")
    sc = next(p for p in v["band_vorlage"] if p["name"] == "sidechain_source")
    if sc["werte"] != ["none", "internal", "priority_sidechain"]:
        fehler.append("sidechain_source kennt nur none|internal|priority_sidechain (§53.8)")

    # ── Schutz-Zonen (R6) ──────────────────────────────────────────────────
    z = v.get("schutz_zonen", {})
    if z.get("hoechstens") != 8:
        fehler.append("hoechstens acht Schutz-Zonen (R6)")
    zf = {f["name"]: f for f in z.get("felder", [])}
    if set(zf) != {"id", "low_hz", "high_hz", "enabled"}:
        fehler.append("eine Zone traegt genau id, low_hz, high_hz, enabled")
    else:
        if (zf["id"].get("min"), zf["id"].get("max")) != (0, 7):
            fehler.append("Zonen-id laeuft von 0 bis 7")
        for name in ("low_hz", "high_hz"):
            if (zf[name].get("min"), zf[name].get("max")) != (20.0, 20000.0):
                fehler.append(f"Zonengrenze {name} laeuft von 20 bis 20000 Hz (Nyquistkappung ist Laufzeit, R6 Abweichung 2)")

    # ── DTO-Form ───────────────────────────────────────────────────────────
    dto = v.get("dsp_dto", {})
    for wort in ("mehr als acht Schutz-Zonen", "doppelte Zonen-id",
                 "Zonenliste nicht streng aufsteigend nach id", "dsp_schema_version != 2"):
        if wort not in dto.get("ablehnung_vor_dem_hash", []):
            fehler.append(f"dsp_dto.ablehnung_vor_dem_hash nennt nicht: {wort}")

    # ── Presetschema (R12) ─────────────────────────────────────────────────
    pv = json.loads(PRESET.read_text(encoding="utf-8"))
    if pv.get("preset_schema_version") != 1:
        fehler.append("preset_schema_version muss 1 sein")
    if pv.get("geschrieben_fuer_dsp_schema_version") != v.get("dsp_schema_version"):
        fehler.append("das Preset nennt nicht das Layout, fuer das es geschrieben ist")
    verboten = [f["name"] for f in pv.get("verbotene_felder", [])]
    if sorted(verboten) != sorted(PRESET_VERBOTEN):
        fehler.append(f"verbotene_felder weichen ab: {verboten}")
    if any(not f.get("grund") for f in pv.get("verbotene_felder", [])):
        fehler.append("jedes verbotene Feld nennt seinen Grund")
    for wort in ("verbotenes_feld", "preset_schema_version", "zone_anzahl",
                 "zone_doppelt", "zone_sortierung"):
        if wort not in pv.get("ablehnungsgruende", []):
            fehler.append(f"preset ablehnungsgruende nennt nicht: {wort}")

    if fehler:
        for f in fehler:
            print("VERTRAG FEHLER:", f)
        sys.exit(2)
    print(f"Parameterbestand v2: {len(ids)} Kennungen ({len(host)} Host-Parameter), "
          f"{len(set(ids))} eindeutige IDs, 109 v1-Eintraege woertlich abgeleitet; "
          f"Preset {len(PRESET_INHALT(v))} Kennungen, {len(verboten)} verbotene Felder")
    return v


# ------------------------------------------------------------------ Parameter-Hilfen

def parameter_liste(v: dict) -> list[dict]:
    """Flache Liste aller 120 Parameter mit id + Beschreibung, in
    VERTRAGSREIHENFOLGE (v["ids"]) - nicht in der Reihenfolge der Vorlagen:
    die drei v2-Globalen stehen HINTER allen v1-Slotfeldern."""
    nach_id: dict[str, dict] = {}
    for g in v["global"]:
        nach_id[g["id"]] = dict(g)
    for slot in range(v["slot_anzahl"]):
        for p in v["band_vorlage"]:
            e = dict(p)
            e["id"] = f"{p['layout']}.band.{slot}.{p['name']}"
            nach_id[e["id"]] = e
    return [nach_id[i] for i in v["ids"]]


# Acht Zonen an ihren Vertragsraendern, streng aufsteigend nach id.
def zonen_maxima() -> list[dict]:
    return [{"enabled": i % 2 == 0, "high_hz": 20000.0, "id": i, "low_hz": 20.0}
            for i in range(8)]


def dto_default(v: dict) -> dict:
    return {"dsp_schema_version": 2,
            "parameters": {p["id"]: p["default"] for p in parameter_liste(v)},
            "schutz_zonen": []}


def dto_grenze(v: dict, welche: str) -> dict:
    d = {"dsp_schema_version": 2, "parameters": {},
         "schutz_zonen": zonen_maxima() if welche == "max" else []}
    for p in parameter_liste(v):
        if p["typ"] == "float":
            d["parameters"][p["id"]] = p[welche]
        elif p["typ"] == "bool":
            d["parameters"][p["id"]] = (welche == "max")
        else:
            d["parameters"][p["id"]] = p["werte"][-1] if welche == "max" else p["werte"][0]
    return d


def dto_gemischt(v: dict) -> dict:
    """Nicht-triviale Doubles, damit die ES6-Zahlenform an echten Werten haengt."""
    d = dto_default(v)
    p = d["parameters"]
    p["v1.global.input_trim_db"] = -3.5
    p["v1.global.output_trim_db"] = 0.1
    p["v1.global.width"] = 1.25
    p["v1.global.mono_bass_hz"] = 120.5
    p["v1.band.0.enabled"] = True
    p["v1.band.0.type"] = "low_cut"
    p["v1.band.0.freq_hz"] = 32.7
    p["v1.band.0.q"] = 0.7071067811865476
    p["v1.band.1.enabled"] = True
    p["v1.band.1.freq_hz"] = 1234.5678
    p["v1.band.1.gain_db"] = -2.25
    p["v1.band.1.q"] = 0.15
    p["v1.band.1.channel_mode"] = "side"
    p["v1.band.2.enabled"] = True
    p["v1.band.2.dynamic_enabled"] = True
    p["v1.band.2.dynamic_range_db"] = -6.0
    p["v1.band.2.threshold_db"] = -18.3
    p["v1.band.2.attack_ms"] = 0.1
    p["v1.band.2.hold_ms"] = 12.5
    p["v1.band.2.release_ms"] = 4999.9
    p["v1.band.2.sidechain_source"] = "priority_sidechain"
    p["v1.band.7.type"] = "high_cut"
    p["v1.band.7.freq_hz"] = 20000.0
    p["v1.band.7.q"] = 24.0
    # SONDE-015: die v2-Werte gehoeren in denselben gemischten Fall - sonst
    # haengt der teuerste Hash des Korpus nur am v1-Bestand.
    p["v2.global.eq_enabled"] = True
    p["v2.global.mix"] = 0.30000000000000004
    p["v2.global.auto_gain"] = True
    for slot in (0, 1, 2, 7):
        p[f"v2.band.{slot}.occupied"] = True
    d["schutz_zonen"] = [
        {"enabled": True, "high_hz": 120.25, "id": 0, "low_hz": 40.5},
        {"enabled": False, "high_hz": 1100.0, "id": 5, "low_hz": 900.0},
    ]
    return d


def validiere_dto_python(v: dict, text: str) -> str | None:
    """Kleiner Referenz-Validator: liefert den Grundcode oder None (gueltig).

    Reihenfolge wie im Vertrag (nakama-state-v2.md §4): doppelter Schluessel
    (Textstufe) -> Struktur -> dsp_schema_version -> unbekannt -> fehlend ->
    Typ -> nichtendlich -> Bereich/Enum.
    """
    # Stufe 0: v3-Byte-/Grammatikriegel in der Variante fuer den danach
    # folgenden exakt gerundeten DTO-Leser - wie in NakamaParameter.cpp.
    # Der eigene JCS-Leser rundet korrekt und validiert Typ/Bereich feldgenau;
    # nur der binary64-Schemaweg braucht die globale 15-Ziffern-Sicherung.
    riegel = textriegel_bytes(text.encode("utf-8"), schema_ganzzahl_sichern=False)
    if riegel is not None:
        if riegel.startswith("Exponent ausserhalb") or riegel.startswith("Zahl ausserhalb"):
            return "nichtendlich"
        if riegel.startswith("Ganzzahl ausserhalb"):
            return "bereich"
        return "kein_json"

    # Textstufe: doppelte Schluessel im selben Objekt.
    doppelt = False

    def hook(paare):
        nonlocal doppelt
        schl = [k for k, _ in paare]
        if len(schl) != len(set(schl)):
            doppelt = True
        return dict(paare)

    try:
        obj = json.loads(text, object_pairs_hook=hook, parse_constant=lambda s: float("nan"))
    except ValueError:
        return "kein_json"
    if doppelt:
        return "doppelter_schluessel"
    if not isinstance(obj, dict):
        return "struktur"
    if set(obj.keys()) != {"dsp_schema_version", "parameters", "schutz_zonen"}:
        return "struktur"
    if obj["dsp_schema_version"] != 2 or isinstance(obj["dsp_schema_version"], bool):
        return "dsp_schema_version"
    par = obj["parameters"]
    if not isinstance(par, dict):
        return "struktur"
    if not isinstance(obj["schutz_zonen"], list):
        return "struktur"
    tabelle = {p["id"]: p for p in parameter_liste(v)}
    for k in par:
        if k not in tabelle:
            return "unbekannter_schluessel"
    for k in tabelle:
        if k not in par:
            return "fehlender_schluessel"
    # Stufenweise wie das C++-Bein: erst ALLE Typen, dann je Parameter
    # nichtendlich -> Bereich/Enum. So meldet ein Dokument mit zwei Fehlern in
    # beiden Beinen denselben ersten Grund.
    for k, p in tabelle.items():
        w = par[k]
        if p["typ"] == "bool":
            if not isinstance(w, bool):
                return "typ"
        elif p["typ"] == "float":
            if isinstance(w, bool) or not isinstance(w, (int, float)):
                return "typ"
        else:
            if not isinstance(w, str):
                return "typ"
    for k, p in tabelle.items():
        w = par[k]
        if p["typ"] == "float":
            if not math.isfinite(w):
                return "nichtendlich"
            if not (p["min"] <= w <= p["max"]):
                return "bereich"
        elif p["typ"] == "enum":
            if w not in p["werte"]:
                return "enum"
    return validiere_zonen_python(obj["schutz_zonen"])


def validiere_zonen_python(zonen) -> str | None:
    """Die Zonenregeln aus R6 - dieselbe Reihenfolge wie NakamaParameter.cpp:
    Struktur/Typ je Zone, dann Anzahl, Bereich, doppelte id, Sortierung."""
    for z in zonen:
        if not isinstance(z, dict) or set(z) != {"enabled", "high_hz", "id", "low_hz"}:
            return "struktur"
        if not isinstance(z["enabled"], bool):
            return "typ"
        for name in ("id", "low_hz", "high_hz"):
            if isinstance(z[name], bool) or not isinstance(z[name], (int, float)):
                return "typ"
        if not float(z["id"]).is_integer():
            return "bereich"
    if len(zonen) > 8:
        return "zone_anzahl"
    for z in zonen:
        if not (0 <= z["id"] <= 7):
            return "bereich"
        if not math.isfinite(z["low_hz"]) or not math.isfinite(z["high_hz"]):
            return "nichtendlich"
        if z["low_hz"] < 20.0 or z["low_hz"] >= z["high_hz"] or z["high_hz"] > 20000.0:
            return "bereich"
    ids = [z["id"] for z in zonen]
    if len(ids) != len(set(ids)):
        return "zone_doppelt"
    if any(ids[i - 1] >= ids[i] for i in range(1, len(ids))):
        return "zone_sortierung"
    return None


def validiere_preset_python(v: dict, text: str) -> str | None:
    """Kleiner Referenz-Validator fuer Presets - dieselbe Stufenfolge wie
    `nakama::preset::lies` (nakama-preset-v1.json, reihenfolge_der_pruefung)."""
    riegel = textriegel_bytes(text.encode("utf-8"), schema_ganzzahl_sichern=False)
    if riegel is not None:
        if riegel.startswith("Exponent ausserhalb") or riegel.startswith("Zahl ausserhalb"):
            return "nichtendlich"
        if riegel.startswith("Ganzzahl ausserhalb"):
            return "bereich"
        return "kein_json"

    doppelt = False

    def hook(paare):
        nonlocal doppelt
        schl = [k for k, _ in paare]
        if len(schl) != len(set(schl)):
            doppelt = True
        return dict(paare)

    try:
        obj = json.loads(text, object_pairs_hook=hook, parse_constant=lambda s: float("nan"))
    except ValueError:
        return "kein_json"
    if doppelt:
        return "doppelter_schluessel"
    if not isinstance(obj, dict):
        return "struktur"

    # Verbotene Felder FRUEH - bevor irgendein Inhalt gedeutet wird.
    for name in PRESET_VERBOTEN:
        if name in obj:
            return "verbotenes_feld"

    if obj.get("preset_schema_version") != 1 or isinstance(obj.get("preset_schema_version"), bool):
        return "preset_schema_version"
    if obj.get("dsp_schema_version") != v["dsp_schema_version"] or isinstance(obj.get("dsp_schema_version"), bool):
        return "dsp_schema_version"
    par = obj.get("parameters")
    if not isinstance(par, dict):
        return "struktur"
    if not isinstance(obj.get("schutz_zonen"), list):
        return "struktur"
    # Jedes WEITERE Top-Level-Feld ist additiv und wird ignoriert.

    inhalt = set(PRESET_INHALT(v))
    tabelle = {p["id"]: p for p in parameter_liste(v) if p["id"] in inhalt}
    for k in par:
        if k not in tabelle:
            return "unbekannter_schluessel"
    for k in tabelle:
        if k not in par:
            return "fehlender_schluessel"
    for k, p in tabelle.items():
        w = par[k]
        if p["typ"] == "bool":
            if not isinstance(w, bool):
                return "typ"
        elif p["typ"] == "float":
            if isinstance(w, bool) or not isinstance(w, (int, float)):
                return "typ"
        else:
            if not isinstance(w, str):
                return "typ"
    for k, p in tabelle.items():
        w = par[k]
        if p["typ"] == "float":
            if not math.isfinite(w):
                return "nichtendlich"
            if not (p["min"] <= w <= p["max"]):
                return "bereich"
        elif p["typ"] == "enum":
            if w not in p["werte"]:
                return "enum"
    return validiere_zonen_python(obj["schutz_zonen"])


# ------------------------------------------------------------------ JCS-Zahlen

# RFC 8785 Anhang B, Tabelle 1 - WOERTLICH aus dem RFC-Text (rfc-editor.org,
# gelesen 22.08.2026). Leere Erwartung = NaN/Infinity = abgelehnt.
RFC_TABELLE = [
    ("0000000000000000", "0"),
    ("8000000000000000", "0"),
    ("0000000000000001", "5e-324"),
    ("8000000000000001", "-5e-324"),
    ("7fefffffffffffff", "1.7976931348623157e+308"),
    ("ffefffffffffffff", "-1.7976931348623157e+308"),
    ("4340000000000000", "9007199254740992"),
    ("c340000000000000", "-9007199254740992"),
    ("4430000000000000", "295147905179352830000"),
    ("7fffffffffffffff", None),
    ("7ff0000000000000", None),
    ("44b52d02c7e14af5", "9.999999999999997e+22"),
    ("44b52d02c7e14af6", "1e+23"),
    ("44b52d02c7e14af7", "1.0000000000000001e+23"),
    ("444b1ae4d6e2ef4e", "999999999999999700000"),
    ("444b1ae4d6e2ef4f", "999999999999999900000"),
    ("444b1ae4d6e2ef50", "1e+21"),
    ("3eb0c6f7a0b5ed8c", "9.999999999999997e-7"),
    ("3eb0c6f7a0b5ed8d", "0.000001"),
    ("41b3de4355555553", "333333333.3333332"),
    ("41b3de4355555554", "333333333.33333325"),
    ("41b3de4355555555", "333333333.3333333"),
    ("41b3de4355555556", "333333333.3333334"),
    ("41b3de4355555557", "333333333.33333343"),
    ("becbf647612f3696", "-0.0000033333333333333333"),
    ("43143ff3c1cb0959", "1424953923781206.2"),
]

# Eigene Kanten, Erwartung von Hand nach ECMA-262 6.1.6.1.20 (n in [-5,21]
# => Positionsschreibweise, sonst d.ddde+-X; k=1 => ohne Punkt).
HAND_VEKTOREN = [
    (1.0, "1"),
    (-1.0, "-1"),
    (100.0, "100"),
    (1e20, "100000000000000000000"),           # n = 21 = k..21 -> Ziffern + Nullen
    (1e21, "1e+21"),                            # n = 22 > 21 -> Exponent
    (1.5e21, "1.5e+21"),
    (1e-6, "0.000001"),                         # n = -5 -> "0." + 5 Nullen
    (1e-7, "1e-7"),                             # n = -6 < -5 -> Exponent, k = 1
    (1.5e-7, "1.5e-7"),
    (0.1, "0.1"),
    (0.30000000000000004, "0.30000000000000004"),
    (2.5e-3, "0.0025"),
    (-1.5, "-1.5"),
    (123456.789, "123456.789"),
    (1e100, "1e+100"),
    (-1e-100, "-1e-100"),
    (4.5, "4.5"),
    (1e15, "1000000000000000"),
    (1234567890123456.0, "1234567890123456"),
    (0.5, "0.5"),
    (3.0e-5, "0.00003"),                        # n = -4
    (0.000012345, "0.000012345"),               # n = -4, k = 5
    (1.0e-5, "0.00001"),                        # n = -4
    (12.0e-7, "0.0000012"),                     # n = -5, k = 2
    (9.5e-7, "9.5e-7"),                         # n = -6
    (20000.0, "20000"),
    (0.7071067811865476, "0.7071067811865476"),
    (4999.9, "4999.9"),
]


def erzeuge_zahlen() -> dict:
    vektoren = []
    bestaetigt_rfc = 0
    for h, erw in RFC_TABELLE:
        x = aus_hex64(h)
        if erw is None:
            try:
                rfc8785.dumps(x)
                raise SystemExit(f"rfc8785 haette {h} ablehnen muessen")
            except ValueError:
                pass
            vektoren.append({"hex64": h, "abgelehnt": True, "quelle": "rfc8785-anhang-b"})
            continue
        ist = rfc8785.dumps(x).decode("utf-8")
        if ist != erw:
            raise SystemExit(f"Referenz weicht vom RFC ab: {h}: rfc8785={ist!r} RFC={erw!r}")
        bestaetigt_rfc += 1
        vektoren.append({"hex64": h, "erwartet": erw, "quelle": "rfc8785-anhang-b"})
    for x, erw in HAND_VEKTOREN:
        ist = rfc8785.dumps(x).decode("utf-8")
        if ist != erw:
            raise SystemExit(f"Hand-Erwartung und Referenz uneins: {x!r}: rfc8785={ist!r} Hand={erw!r}")
        vektoren.append({"hex64": hex64(x), "erwartet": erw, "quelle": "hand-ecma262"})
    # Sicherheitsnetz: jeder erwartete Text muss als double EXAKT auf dieselben
    # Bits zurueckfuehren - einzige Ausnahme ist Minus-Null, die RFC 8785
    # absichtlich auf "0" abbildet (ToNumber(ToString(-0)) ist +0).
    for e in vektoren:
        if "erwartet" in e and hex64(float(e["erwartet"])) != e["hex64"]:
            if e["hex64"] == "8000000000000000" and e["erwartet"] == "0":
                continue
            raise SystemExit(f"Erwartung {e['erwartet']} ist nicht round-trip-exakt fuer {e['hex64']}")
    print(f"RFC-8785-Tabelle: {bestaetigt_rfc} Zahlen gegen den RFC-Text bestaetigt, "
          f"{len(HAND_VEKTOREN)} Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)")
    return {
        "$id": "evenacadia.nakama.fixtures.state.jcs-zahlen.v1",
        "zweck": "IEEE-754-Bitmuster -> RFC-8785-Zahlentext. Die RFC-Zeilen tragen den Text, den RFC 8785 Anhang B druckt; die Handzeilen sind nach ECMA-262 Number::toString hergeleitet. rfc8785 (Python) hat jede bestaetigt.",
        "regel": "Ein Leser baut den double aus hex64, kanonisiert ihn und vergleicht den Text byteweise. `abgelehnt` = der Kanonisierer muss mit Fehler enden (NaN/Infinity sind in JSON nicht darstellbar).",
        "anzahl": len(vektoren),
        "vektoren": vektoren,
    }


# ------------------------------------------------------------------ JCS-Dokumente

def dokumente() -> list[dict]:
    """(name, eingabetext, erwartete kanonische Form) - Erwartung VON HAND."""
    u = chr
    d = []

    # RFC 8785 §3.2.2 - das Beispiel des RFC, Eingabe und Ausgabe woertlich.
    rfc_ein = ("{" + chr(10) +
               "  " + DQ + "numbers" + DQ + ": [333333333.33333329, 1E30, 4.50, 2e-3, 0.000000000000000000000000001]," + chr(10) +
               "  " + DQ + "string" + DQ + ": " + DQ + BS + "u20ac$" + BS + "u000F" + BS + "u000aA'" + BS + "u0042" + BS + "u0022" + BS + "u005c" + BS + BS + BS + DQ + BS + "/" + DQ + "," + chr(10) +
               "  " + DQ + "literals" + DQ + ": [null, true, false]" + chr(10) +
               "}" + chr(10))
    rfc_aus = ("{" + DQ + "literals" + DQ + ":[null,true,false]," + DQ + "numbers" + DQ +
               ":[333333333.3333333,1e+30,4.5,0.002,1e-27]," + DQ + "string" + DQ + ":" + DQ +
               u(0x20AC) + "$" + BS + "u000f" + BS + "nA'B" + BS + DQ + BS + BS + BS + BS + BS + DQ + "/" + DQ + "}")
    d.append(("rfc-beispiel", rfc_ein, rfc_aus))

    d.append(("leeres-objekt", "{}" + chr(10), "{}"))
    d.append(("leeres-array", "[ ]", "[]"))
    d.append(("verschachtelt",
              "{" + DQ + "b" + DQ + ":{" + DQ + "z" + DQ + ":1," + DQ + "a" + DQ + ":[{" + DQ + "y" + DQ + ":2," + DQ + "x" + DQ + ":1}]}," + DQ + "a" + DQ + ":[]}",
              "{" + DQ + "a" + DQ + ":[]," + DQ + "b" + DQ + ":{" + DQ + "a" + DQ + ":[{" + DQ + "x" + DQ + ":1," + DQ + "y" + DQ + ":2}]," + DQ + "z" + DQ + ":1}}"))

    # Steuerzeichen: 0x01 -> , 0x08 \b, 0x09 \t, 0x0A \n, 0x0C \f, 0x0D \r,
    # 0x1F -> , 0x7F (DEL) bleibt roh, " und \ werden escapet, / nicht.
    steuer_ein = ("{" + DQ + "s" + DQ + ":" + DQ + BS + "u0001" + BS + "b" + BS + "t" + BS + "n" + BS + "f" + BS + "r" +
                  BS + "u001F" + u(0x7F) + BS + DQ + BS + BS + "/" + BS + "/" + "e" + u(0x301) + DQ + "}")
    steuer_aus = ("{" + DQ + "s" + DQ + ":" + DQ + BS + "u0001" + BS + "b" + BS + "t" + BS + "n" + BS + "f" + BS + "r" +
                  BS + "u001f" + u(0x7F) + BS + DQ + BS + BS + "//e" + u(0x301) + DQ + "}")
    d.append(("steuerzeichen", steuer_ein, steuer_aus))

    # Sortierung nach UTF-16-Code-Units: "" < A < B < a < aa < ab < e-acute(00E9)
    # < U+1D11E (D834 DD1E) < U+FF21. Eine Codepunkt-Sortierung setzte U+FF21
    # VOR U+1D11E - genau das unterscheidet RFC 8785 von "nach Unicode sortieren".
    schl = [u(0x1D11E), u(0xFF21), "ab", "a", "B", "aa", "A", u(0xE9), ""]
    sort_ein = "{" + ",".join(DQ + k + DQ + ":" + str(i) for i, k in enumerate(schl)) + "}"
    reihenfolge = ["", "A", "B", "a", "aa", "ab", u(0xE9), u(0x1D11E), u(0xFF21)]
    sort_aus = "{" + ",".join(DQ + k + DQ + ":" + str(schl.index(k)) for k in reihenfolge) + "}"
    d.append(("sortierung-utf16", sort_ein, sort_aus))

    # Zahlen im Kontext - grosse Ganzzahlen als Exponentform, damit kein Bein
    # sie als int64 liest (JUCE akkumuliert int64 ohne Schranke: v3-Textriegel Regel 2).
    zahlen_ein = ("{" + DQ + "n" + DQ + ":[1.0, -0.0, 1e21, 1e-7, 0.000001, 1.2345678901234568e20, 0.1, 1E+2, 100e-2, "
                  "5e-324, 1.7976931348623157e308, -1.5, 2.5e-3, 0, -0, 10, 1e20]}")
    zahlen_aus = ("{" + DQ + "n" + DQ + ":[1,0,1e+21,1e-7,0.000001,123456789012345680000,0.1,100,1,"
                  "5e-324,1.7976931348623157e+308,-1.5,0.0025,0,0,10,100000000000000000000]}")
    d.append(("zahlen-im-kontext", zahlen_ein, zahlen_aus))

    d.append(("unicode-roh",
              "{" + DQ + u(0xE4) + DQ + ":" + DQ + u(0xF6) + u(0x20AC) + u(0x1D11E) + DQ + "}",
              "{" + DQ + u(0xE4) + DQ + ":" + DQ + u(0xF6) + u(0x20AC) + u(0x1D11E) + DQ + "}"))
    d.append(("literale",
              "[ true , false , null , " + DQ + "true" + DQ + " , 1 , " + DQ + "1" + DQ + " ]",
              "[true,false,null," + DQ + "true" + DQ + ",1," + DQ + "1" + DQ + "]"))
    d.append(("whitespace",
              chr(10) + chr(9) + "{ " + chr(10) + DQ + "k" + DQ + " : " + chr(9) + "[ 1 , { " + DQ + "b" + DQ + ":2, " + DQ + "a" + DQ + " :1 } ] " + chr(13) + chr(10) + "}" + chr(10),
              "{" + DQ + "k" + DQ + ":[1,{" + DQ + "a" + DQ + ":1," + DQ + "b" + DQ + ":2}]}"))
    d.append(("tiefe-struktur",
              "[[{" + DQ + "b" + DQ + ":[{" + DQ + "d" + DQ + ":null," + DQ + "c" + DQ + ":true}]," + DQ + "a" + DQ + ":1}]]",
              "[[{" + DQ + "a" + DQ + ":1," + DQ + "b" + DQ + ":[{" + DQ + "c" + DQ + ":true," + DQ + "d" + DQ + ":null}]}]]"))
    # Escape-Eingabeformen: A -> A, \/ -> /, é -> e-acute, Surrogatpaar -> U+1D11E.
    d.append(("escape-eingabeformen",
              "{" + DQ + "k" + DQ + ":" + DQ + BS + "u0041" + BS + "/" + BS + "u00e9" + BS + "ud834" + BS + "udd1e" + DQ + "}",
              "{" + DQ + "k" + DQ + ":" + DQ + "A/" + u(0xE9) + u(0x1D11E) + DQ + "}"))
    # Schluessel, die nur als UTF-16 vergleichbar sind, in einem Array von Objekten.
    d.append(("array-von-objekten",
              "[{" + DQ + "z" + DQ + ":0," + DQ + "y" + DQ + ":0},{" + DQ + "b" + DQ + ":{" + DQ + "q" + DQ + ":[3,2,1]}," + DQ + "a" + DQ + ":[]}]",
              "[{" + DQ + "y" + DQ + ":0," + DQ + "z" + DQ + ":0},{" + DQ + "a" + DQ + ":[]," + DQ + "b" + DQ + ":{" + DQ + "q" + DQ + ":[3,2,1]}}]"))
    return d


# ------------------------------------------------------------------ DTO-Fixtures

def dto_ungueltige(v: dict) -> list[tuple[str, str, str]]:
    """(name, text, erwarteter Grund) - Text von Hand gebaut."""
    basis = dto_default(v)

    def text_mit(aenderung) -> str:
        d = json.loads(json.dumps(basis))
        aenderung(d)
        return json.dumps(d, ensure_ascii=False, indent=1) + chr(10)

    faelle = []

    def f1(d): d["parameters"]["v1.band.0.freq_hz"] = 1e999
    # 1e999 -> Python liest inf (parse_constant greift nicht, es ist ein Zahlenliteral)
    faelle.append(("nichtendlich-exponent", text_mit(f1).replace("Infinity", "1e999"), "nichtendlich"))

    # Unterlauf: Python/serde_json laesen 1e-400 als 0.0 - der Textriegel
    # (Regel 3, |Exponent| < 308) faengt es in allen drei Beinen VOR dem Parser.
    faelle.append(("nichtendlich-unterlauf", text_mit(f1).replace("Infinity", "1e-400"), "nichtendlich"))
    # Ganzzahl jenseits 2^53-1 (Textriegel Regel 2): liegt ausserhalb jedes Bereichs.
    faelle.append(("bereich-ganzzahl-2e53", text_mit(f1).replace("Infinity", "9007199254740993"), "bereich"))

    def f2(d): d["parameters"]["v1.band.3.sidechain_source"] = "external"
    faelle.append(("enum-unbekannt", text_mit(f2), "enum"))

    def f3(d): d["parameters"]["v1.band.5.gain_db"] = 12.000001
    faelle.append(("bereich-gain", text_mit(f3), "bereich"))

    def f4(d): d["parameters"]["v1.band.2.q"] = 0.1
    faelle.append(("bereich-q-unten", text_mit(f4), "bereich"))

    def f5(d): d["parameters"]["v1.global.extra"] = 1
    faelle.append(("unbekannter-schluessel", text_mit(f5), "unbekannter_schluessel"))

    def f6(d): del d["parameters"]["v1.band.7.release_ms"]
    faelle.append(("fehlender-schluessel", text_mit(f6), "fehlender_schluessel"))

    # SEITENWECHSEL (M-94): bis SONDE-015 war `dsp_schema_version: 2` der
    # ungueltige Fall. Layout v2 ist jetzt der aktive Vertrag - die alte Fixture
    # `dto/ungueltig/dsp-schema-version-2.json` faellt weg (der Erzeuger meldet
    # sie sonst als VERWAIST), und die 1 nimmt ihren Platz ein.
    def f7(d): d["dsp_schema_version"] = 1
    faelle.append(("dsp-schema-version-1", text_mit(f7), "dsp_schema_version"))

    # ── Schutz-Zonen (R6, M-66/M-72/M-73) ─────────────────────────────────
    def z_neun(d): d["schutz_zonen"] = [
        {"enabled": True, "high_hz": 200.0, "id": i, "low_hz": 100.0} for i in range(9)]
    faelle.append(("zone-neunte", text_mit(z_neun), "zone_anzahl"))

    def z_doppelt(d): d["schutz_zonen"] = [
        {"enabled": True, "high_hz": 200.0, "id": 3, "low_hz": 100.0},
        {"enabled": False, "high_hz": 400.0, "id": 3, "low_hz": 300.0}]
    faelle.append(("zone-doppelte-id", text_mit(z_doppelt), "zone_doppelt"))

    def z_sort(d): d["schutz_zonen"] = [
        {"enabled": True, "high_hz": 200.0, "id": 5, "low_hz": 100.0},
        {"enabled": True, "high_hz": 400.0, "id": 1, "low_hz": 300.0}]
    faelle.append(("zone-unsortiert", text_mit(z_sort), "zone_sortierung"))

    def z_gleich(d): d["schutz_zonen"] = [
        {"enabled": True, "high_hz": 200.0, "id": 0, "low_hz": 200.0}]
    faelle.append(("zone-low-gleich-high", text_mit(z_gleich), "bereich"))

    def z_unter20(d): d["schutz_zonen"] = [
        {"enabled": True, "high_hz": 200.0, "id": 0, "low_hz": 19.9}]
    faelle.append(("zone-unter-20-hz", text_mit(z_unter20), "bereich"))

    def z_ueber20k(d): d["schutz_zonen"] = [
        {"enabled": True, "high_hz": 20000.1, "id": 0, "low_hz": 100.0}]
    faelle.append(("zone-ueber-20-khz", text_mit(z_ueber20k), "bereich"))

    def z_id8(d): d["schutz_zonen"] = [
        {"enabled": True, "high_hz": 200.0, "id": 8, "low_hz": 100.0}]
    faelle.append(("zone-id-8", text_mit(z_id8), "bereich"))

    def z_feld(d): d["schutz_zonen"] = [
        {"enabled": True, "high_hz": 200.0, "id": 0, "low_hz": 100.0, "extra": 1}]
    faelle.append(("zone-zusatzfeld", text_mit(z_feld), "struktur"))

    def z_typ(d): d["schutz_zonen"] = [
        {"enabled": "ja", "high_hz": 200.0, "id": 0, "low_hz": 100.0}]
    faelle.append(("zone-enabled-als-string", text_mit(z_typ), "typ"))

    def z_fehlt(d): del d["schutz_zonen"]
    faelle.append(("schutz-zonen-fehlt", text_mit(z_fehlt), "struktur"))

    # Eine v2-Kennung fehlt: ein Stand des Layouts v1 auf dem DTO-Weg ist
    # KEIN Teilstand, sondern ein fehlender Schluessel (M-90 gilt fuer den
    # STATE-Weg, nicht fuer den Draht).
    def v2_fehlt(d): del d["parameters"]["v2.band.4.occupied"]
    faelle.append(("fehlende-v2-kennung", text_mit(v2_fehlt), "fehlender_schluessel"))

    def f8(d): d["parameters"]["v1.band.1.enabled"] = 1
    faelle.append(("typ-bool-als-zahl", text_mit(f8), "typ"))

    def f9(d): d["parameters"]["v1.band.1.freq_hz"] = "1000"
    faelle.append(("typ-zahl-als-string", text_mit(f9), "typ"))

    def f10(d): d["parameters"]["v1.band.4.type"] = True
    faelle.append(("typ-enum-als-bool", text_mit(f10), "typ"))

    # doppelter Schluessel: Textstufe - json.dumps kann das nicht, also von Hand.
    doppelt = json.dumps(basis, ensure_ascii=False)
    doppelt = doppelt.replace(DQ + "v1.global.width" + DQ + ": 1.0",
                              DQ + "v1.global.width" + DQ + ": 1.0, " + DQ + "v1.global.width" + DQ + ": 1.5")
    assert doppelt.count(DQ + "v1.global.width" + DQ) == 2
    faelle.append(("doppelter-schluessel", doppelt + chr(10), "doppelter_schluessel"))

    def f11(d): d["extra"] = 1
    faelle.append(("struktur-extra-wurzel", text_mit(f11), "struktur"))

    faelle.append(("struktur-array", "[1,2,3]" + chr(10), "struktur"))

    def f12(d): d["parameters"]["v1.global.mono_bass_hz"] = -0.0
    # -0.0 liegt im Bereich [0, 500]: GUELTIG ist hier die richtige Antwort - steht
    # absichtlich NICHT in der ungueltigen Liste; siehe dto/gueltig/minus-null.json.

    return faelle


def dto_gueltige(v: dict) -> list[tuple[str, str]]:
    aus = [
        ("default", json.dumps(dto_default(v), ensure_ascii=False, indent=1) + chr(10)),
        ("minima", json.dumps(dto_grenze(v, "min"), ensure_ascii=False, indent=1) + chr(10)),
        ("maxima", json.dumps(dto_grenze(v, "max"), ensure_ascii=False, indent=1) + chr(10)),
        ("gemischt", json.dumps(dto_gemischt(v), ensure_ascii=False, indent=1) + chr(10)),
    ]
    # -0.0 im Bereich: gueltig, kanonisch "0" (RFC 8785: Minus-Null -> 0) - der
    # Hash von minus-null ist damit DERSELBE wie der von default.
    mn = dto_default(v)
    mn["parameters"]["v1.global.mono_bass_hz"] = -0.0
    aus.append(("minus-null", json.dumps(mn, ensure_ascii=False, indent=1).replace(": -0.0", ": -0.0") + chr(10)))
    # Ganzzahlschreibweise fuer einen float-Parameter ist typgerecht (JSON kennt
    # keinen int/float-Unterschied): "1000" == "1000.0".
    ganz = json.dumps(dto_default(v), ensure_ascii=False, indent=1).replace(": 1000.0", ": 1000") + chr(10)
    aus.append(("ganzzahl-schreibweise", ganz))
    # SONDE-015: eine EINZELNE Zone am unteren Rand des Bestands - der Fall,
    # den die Oberflaeche zuerst erzeugt, und der einzige mit genau einer.
    eine = dto_default(v)
    eine["schutz_zonen"] = [{"enabled": True, "high_hz": 120.25, "id": 0, "low_hz": 40.5}]
    aus.append(("zone-einzeln", json.dumps(eine, ensure_ascii=False, indent=1) + chr(10)))
    return aus


# ------------------------------------------------------------------ Preset

def preset_default(v: dict) -> dict:
    """Ein Preset in der Form des WRITERS: kanonisch, ohne eq_enabled."""
    inhalt = set(PRESET_INHALT(v))
    return {
        "dsp_schema_version": v["dsp_schema_version"],
        "parameters": {p["id"]: p["default"] for p in parameter_liste(v) if p["id"] in inhalt},
        "preset_schema_version": 1,
        "schutz_zonen": [],
    }


def preset_gemischt(v: dict) -> dict:
    d = preset_default(v)
    p = d["parameters"]
    p["v1.band.0.enabled"] = True
    p["v1.band.0.type"] = "low_shelf"
    p["v1.band.0.freq_hz"] = 120.0
    p["v1.band.0.q"] = 0.7071067811865476
    p["v1.band.0.gain_db"] = -2.5
    p["v2.band.0.occupied"] = True
    p["v2.global.mix"] = 0.75
    p["v2.global.auto_gain"] = True
    d["schutz_zonen"] = [{"enabled": True, "high_hz": 120.25, "id": 0, "low_hz": 40.5}]
    return d


def preset_gueltige(v: dict) -> list[tuple[str, str, str]]:
    """(name, text, warum) - Faelle, die ein Leser ANNEHMEN muss."""
    aus = [
        ("default", kanon_text(preset_default(v)), "der neutrale Klanginhalt"),
        ("gemischt", kanon_text(preset_gemischt(v)), "ein belegtes Band, Mix, Auto-Gain und eine Zone"),
    ]
    # Ein unbekanntes Top-Level-Feld wird IGNORIERT (M-98) - der Unterschied zu
    # den sechs verbotenen Namen ist Absicht.
    zusatz = preset_default(v)
    zusatz["autor"] = "ein spaeterer Schreiber"
    aus.append(("unbekanntes-top-level-feld",
                json.dumps(zusatz, ensure_ascii=False, indent=1, sort_keys=True) + chr(10),
                "additive Erweiterung: der Leser ignoriert sie und laedt"))
    return aus


def preset_ungueltige(v: dict) -> list[tuple[str, str, str, str]]:
    """(name, text, grund, warum) - Faelle, die ein Leser ABWEISEN muss."""
    faelle = []

    def mit(aenderung):
        d = json.loads(json.dumps(preset_default(v)))
        aenderung(d)
        return json.dumps(d, ensure_ascii=False, indent=1, sort_keys=True) + chr(10)

    # M-97: je verbotenem Namen ein eigenes Negativfixture. Auch mit leerem
    # Wert - ein optionales, leeres Identitaetsfeld waere eine offene Tuer.
    beispiel = {"adresse": {}, "instance_id": "", "host_bus_name": "Bus A",
                "label": "", "eq_enabled": True, "undo_ring": []}
    for name in PRESET_VERBOTEN:
        def setze_verboten(d, n=name): d[n] = beispiel[n]
        faelle.append((f"verboten-{name.replace('_', '-')}", mit(setze_verboten),
                       "verbotenes_feld", f"Identitaet oder Betrieb: {name} hat kein Feld"))

    def eq_in_parametern(d): d["parameters"]["v2.global.eq_enabled"] = True
    faelle.append(("eq-enabled-in-parametern", mit(eq_in_parametern), "unbekannter_schluessel",
                   "der Rollenschalter hat auch INNERHALB von parameters kein Feld"))

    def falsche_preset_version(d): d["preset_schema_version"] = 2
    faelle.append(("preset-major-2", mit(falsche_preset_version), "preset_schema_version",
                   "unbekannter Preset-Major wird abgelehnt, nicht read-only gehalten"))

    def falsche_dsp_version(d): d["dsp_schema_version"] = 1
    faelle.append(("dsp-schema-version-1", mit(falsche_dsp_version), "dsp_schema_version",
                   "der Inhalt spricht ein Layout, das dieser Build nicht deutet"))

    def fehlt(d): del d["parameters"]["v1.band.3.q"]
    faelle.append(("fehlender-schluessel", mit(fehlt), "fehlender_schluessel",
                   "parameters ist exakt, nicht additiv"))

    def unbekannt(d): d["parameters"]["v1.global.extra"] = 1
    faelle.append(("unbekannter-schluessel", mit(unbekannt), "unbekannter_schluessel",
                   "parameters ist exakt, nicht additiv"))

    def bereich(d): d["parameters"]["v1.band.5.gain_db"] = 12.000001
    faelle.append(("bereich-gain", mit(bereich), "bereich", "derselbe Bereichsriegel wie im DTO"))

    def zone(d): d["schutz_zonen"] = [
        {"enabled": True, "high_hz": 200.0, "id": 5, "low_hz": 100.0},
        {"enabled": True, "high_hz": 400.0, "id": 1, "low_hz": 300.0}]
    faelle.append(("zone-unsortiert", mit(zone), "zone_sortierung",
                   "dieselben Zonenregeln wie im DTO"))

    return faelle


# ------------------------------------------------------------------ Schreiben / Pruefen

def baue(v: dict) -> tuple[dict[str, bytes], dict]:
    """Alle Dateien (relativ zu FIXTURES) + Manifest."""
    dateien: dict[str, bytes] = {}
    zahlen = erzeuge_zahlen()
    dateien["jcs/zahlen.json"] = json_text(zahlen).encode("utf-8")

    dok_eintraege = []
    for name, ein, aus_hand in dokumente():
        ist = rfc8785.dumps(json.loads(ein)).decode("utf-8")
        if ist != aus_hand:
            raise SystemExit(f"Dokument {name}: Hand-Erwartung und rfc8785 uneins:" + chr(10) +
                             f"  hand   = {aus_hand!r}" + chr(10) + f"  rfc8785= {ist!r}")
        pfad = f"jcs/dokumente/{name}.json"
        dateien[pfad] = ein.encode("utf-8")
        kanon = aus_hand.encode("utf-8")
        dok_eintraege.append({"datei": pfad, "kanon": aus_hand, "kanon_sha256": sha256_hex(kanon),
                              "kanon_bytes": len(kanon)})
    print(f"JCS-Dokumente: {len(dok_eintraege)} Hand-Erwartungen von rfc8785 bestaetigt")

    dto_ok = []
    for name, text in dto_gueltige(v):
        grund = validiere_dto_python(v, text)
        if grund is not None:
            raise SystemExit(f"DTO {name} sollte gueltig sein, Python sagt {grund}")
        kanon = rfc8785.dumps(json.loads(text))
        pfad = f"dto/gueltig/{name}.json"
        dateien[pfad] = text.encode("utf-8")
        dto_ok.append({"datei": pfad, "kanon_sha256": sha256_hex(kanon), "kanon_bytes": len(kanon),
                       "state_hash": sha256_hex(kanon)})
    dto_nein = []
    for name, text, grund_hand in dto_ungueltige(v):
        grund = validiere_dto_python(v, text)
        if grund != grund_hand:
            raise SystemExit(f"DTO {name}: Hand-Grund {grund_hand} != Python-Grund {grund}")
        pfad = f"dto/ungueltig/{name}.json"
        dateien[pfad] = text.encode("utf-8")
        dto_nein.append({"datei": pfad, "grund": grund_hand})
    print(f"DTO: {len(dto_ok)} gueltige gehasht, {len(dto_nein)} ungueltige mit Grund")

    # ── Preset (SONDE-015 R12) ────────────────────────────────────────────
    preset_ok = []
    for name, text, warum in preset_gueltige(v):
        grund = validiere_preset_python(v, text)
        if grund is not None:
            raise SystemExit(f"Preset {name} sollte gueltig sein, Python sagt {grund}")
        pfad = f"preset/gueltig/{name}.json"
        dateien[pfad] = text.encode("utf-8")
        preset_ok.append({"datei": pfad, "warum": warum,
                          "sha256": sha256_hex(text.encode("utf-8")),
                          "bytes": len(text.encode("utf-8"))})
    preset_nein = []
    for name, text, grund_hand, warum in preset_ungueltige(v):
        grund = validiere_preset_python(v, text)
        if grund != grund_hand:
            raise SystemExit(f"Preset {name}: Hand-Grund {grund_hand} != Python-Grund {grund}")
        pfad = f"preset/ungueltig/{name}.json"
        dateien[pfad] = text.encode("utf-8")
        preset_nein.append({"datei": pfad, "grund": grund_hand, "warum": warum})
    print(f"Preset: {len(preset_ok)} gueltige, {len(preset_nein)} ungueltige mit Grund")

    # Schema-2-Goldens schreibt EqCopStateMigrationTest --schreibe-goldens (JUCE-
    # Binaerformat). Hier werden sie nur registriert, damit eine stille Aenderung
    # am --pruefen faellt.
    goldens = []
    # SONDE-014 Etappe A: `main-intent-v1.bin` kommt aus demselben Writer
    # (§5.5, "Writer-Fixtures statt Handschrift") und traegt die vier neuen
    # MainProject-Eigenschaften an ihren Raendern.
    # SONDE-015: zwei weitere Writer-Goldens - ein Stand mit vollem Kind `Dsp`
    # und ein Stand im LAYOUT V1 (kein `dsp_schema_version`, 109 Werte), an dem
    # die Migration nach R5 gemessen wird.
    for datei in ("aus-schema1-sensor", "aus-schema1-hub", "aus-schema1-pre", "aus-schema1-post",
                  "fremdes-major-3", "main-intent-v1", "dsp-v2-voll", "layout-v1"):
        pfad = FIXTURES / "schema2" / f"{datei}.bin"
        if pfad.exists():
            b = pfad.read_bytes()
            goldens.append({"datei": f"schema2/{datei}.bin", "sha256": sha256_hex(b), "bytes": len(b)})
        else:
            goldens.append({"datei": f"schema2/{datei}.bin", "sha256": None, "bytes": None,
                            "hinweis": "noch nicht geschrieben - EqCopStateMigrationTest --schreibe-goldens"})

    manifest = {
        "$id": "evenacadia.nakama.fixtures.state.v1",
        "titel": "State-Fixture-Korpus (SONDE-006): JCS, DSP-DTO, Schema-2-Goldens",
        "zweck": "Die dritte Partei zwischen C++ (NakamaKanon/NakamaParameter/NakamaState), Rust (serde_json_canonicalizer in contract_cross_language.rs) und Python (rfc8785). Die JCS-Erwartungen sind von Hand geschrieben bzw. aus dem RFC-Text uebernommen; rfc8785 hat sie bestaetigt. Die DTO-Kanonform liefert rfc8785 - die JCS-Korrektheit ist an dieser Stelle bereits durch die Handvektoren belegt.",
        "vertrag": "../../schemas/state/nakama-parameter-v2.json",
        "vertrag_v1_eingefroren": "../../schemas/state/nakama-parameter-v1.json",
        "vertrag_preset": "../../schemas/state/nakama-preset-v1.json",
        "seitenwechsel": {
            "ticket": "SONDE-015 (M-94)",
            "datei": "dto/ungueltig/dsp-schema-version-2.json",
            "war": "ungueltig (Layout v1 war der aktive Vertrag, 2 wurde abgelehnt)",
            "ist": "entfallen - Layout v2 IST der aktive Vertrag; den Negativplatz nimmt dto/ungueltig/dsp-schema-version-1.json ein",
        },
        "state_hash": "SHA-256-Hex des RFC-8785-kanonischen UTF-8 des validierten DTO (nakama-state-v2.md §4).",
        "gruende": ["kein_json", "doppelter_schluessel", "struktur", "dsp_schema_version", "unbekannter_schluessel",
                    "fehlender_schluessel", "typ", "nichtendlich", "bereich", "enum",
                    # SONDE-015 R6: die drei Zonenregeln, die kein bestehendes
                    # Wort traegt. Anzahl, Identitaet und Reihenfolge sind
                    # KEINE Bereichsfehler - sie betreffen die Liste, nicht
                    # einen Wert.
                    "zone_anzahl", "zone_doppelt", "zone_sortierung",
                    # SONDE-015 R12: nur das Preset kennt ihn.
                    "verbotenes_feld", "preset_schema_version"],
        "jcs_zahlen": {"datei": "jcs/zahlen.json", "anzahl": zahlen["anzahl"],
                       "abgelehnt": sum(1 for e in zahlen["vektoren"] if e.get("abgelehnt"))},
        "jcs_dokumente": dok_eintraege,
        "dto_gueltig": dto_ok,
        "dto_ungueltig": dto_nein,
        "preset_gueltig": preset_ok,
        "preset_ungueltig": preset_nein,
        "schema2_goldens": goldens,
        "referenz": {"python": "rfc8785 " + __import__("importlib.metadata").metadata.version("rfc8785")},
    }
    dateien["MANIFEST.json"] = json_text(manifest).encode("utf-8")
    return dateien, manifest


def main() -> int:
    pruefen = "--pruefen" in sys.argv[1:]
    v = pruefe_vertrag()
    dateien, _ = baue(v)

    verwaltet = {"MANIFEST.json"} | set(dateien)
    if pruefen:
        fehler = 0
        for rel, inhalt in dateien.items():
            pfad = FIXTURES / rel
            if not pfad.exists():
                print("FEHLT:", rel); fehler += 1
            elif pfad.read_bytes() != inhalt:
                print("ABWEICHUNG:", rel); fehler += 1
        # Verwaiste Dateien in den von diesem Erzeuger verwalteten Ordnern.
        for ordner in ("jcs", "dto", "preset"):
            if not (FIXTURES / ordner).exists():
                continue
            for p in sorted((FIXTURES / ordner).rglob("*")):
                if p.is_file():
                    rel = p.relative_to(FIXTURES).as_posix()
                    if rel not in verwaltet:
                        print("VERWAIST:", rel); fehler += 1
        if fehler:
            print(f"STATE-FIXTURES ABWEICHEND ({fehler})")
            return 2
        print(f"STATE-FIXTURES OK ({len(dateien)} Dateien bytegleich, keine verwaiste Datei)")
        return 0

    for rel, inhalt in dateien.items():
        pfad = FIXTURES / rel
        pfad.parent.mkdir(parents=True, exist_ok=True)
        pfad.write_bytes(inhalt)
    print(f"geschrieben: {len(dateien)} Dateien nach {FIXTURES}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
