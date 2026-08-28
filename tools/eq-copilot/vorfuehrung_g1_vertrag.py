"""Vorfuehrung G1-Nacharbeit: die neuen Riegel beim FALLEN zeigen.

Kein `git checkout`, keine Dateimutation (NAK-73: core.autocrlf macht eine
byteweise Rueckstellung unmessbar). Stattdessen wird der VORZUSTAND des
Vertrags im Speicher rekonstruiert und dagegen validiert. Die Dateien auf
Platte werden nur gelesen.
"""
import json
import pathlib
import sys

import jsonschema

from pruefe_v3_vertrag import textriegel_bytes


def ermittle_wurzel(argv: list[str]) -> pathlib.Path:
    if len(argv) > 1:
        print("FEHLER: hoechstens ein optionaler Repository-Pfad ist erlaubt", file=sys.stderr)
        raise SystemExit(2)
    wurzel = (pathlib.Path(argv[0]) if argv else pathlib.Path(__file__).resolve().parents[2]).resolve()
    erwartet = wurzel / "eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
    if not erwartet.is_file():
        print(f"FEHLER: kein Nakama-Repository-Root: {wurzel} "
              f"(fehlt: {erwartet.relative_to(wurzel)})", file=sys.stderr)
        raise SystemExit(2)
    return wurzel


WURZEL = ermittle_wurzel(sys.argv[1:])
SCHEMA = WURZEL / "eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
FIX = WURZEL / "eq-copilot/fixtures/v3"

schema = json.loads(SCHEMA.read_text(encoding="utf-8"))

# --- Der jeweilige Vorzustand wird ausschliesslich im Speicher rekonstruiert.
alt = json.loads(json.dumps(schema))
d = alt["$defs"]

# --- Vorzustand vom 28.08.: ACK-Ergebnis und state_hash waren ungekoppelt.
d["command_ack"] = {
    "type": "object",
    "required": ["type", "command_id", "ergebnis", "state_revision"],
    "additionalProperties": False,
    "properties": {
        "type": {"const": "command_ack"},
        "command_id": {"$ref": "#/$defs/hex32"},
        "ergebnis": {"enum": ["angewandt", "abgelehnt", "konflikt", "abgelaufen",
                                "idempotent_wiederholt"]},
        "state_revision": {"type": "integer", "minimum": 0},
        "state_hash": {"$ref": "#/$defs/state_hash"},
        "code": {"$ref": "#/$defs/fehlercode"},
    },
}
d.pop("state_hash_erfolg", None)

# --- Vorzustand vom 28.08.: encoding war nur ein Enum neben number-Werten.
d["band_encoding"] = {
    "enum": ["q_db_0p1_i16", "q_db_0p01_i16", "float32"]
}
for name in ("bandwerte_fein", "bandwerte_grob"):
    zweig = json.loads(json.dumps(d[name]["oneOf"][0]))
    zweig["properties"]["encoding"] = {"$ref": "#/$defs/band_encoding"}
    zweig["properties"]["werte"]["items"] = {"type": "number"}
    d[name] = zweig

# --- Vorzustand vom 28.08.: pair_id war String oder null ohne Laenge.
d["pair_id"].pop("minLength")
d["pair_id"].pop("maxLength")

# --- Vorzustand vom 23.08.: Messposition und Aussageklasse waren ungekoppelt.
d["measurement_position"] = {"enum": ["insert", "pre", "post", "post_fader_contribution"]}
d["aussageklasse"] = {"enum": ["beobachtend", "beitrag"]}
zweig = json.loads(json.dumps(d["probe_descriptor_insert"]))
zweig["properties"]["measurement_position"] = {"$ref": "#/$defs/measurement_position"}
zweig["properties"]["aussageklasse"] = {"$ref": "#/$defs/aussageklasse"}
zweig.pop("description", None)
d["probe_descriptor"] = zweig
for n in ("probe_descriptor_insert", "probe_descriptor_pre",
          "probe_descriptor_post", "probe_descriptor_beitrag"):
    d.pop(n)

# --- Vorzustand vom 23.08.: state_hash und Bitmap OHNE Muster.
d["state_hash"].pop("pattern")
for g, n in (("bandwerte_fein", 40), ("bandwerte_grob", 12)):
    d[g]["properties"]["gueltig_bitmap"] = {"type": "string", "minLength": n, "maxLength": n}

neu = jsonschema.Draft202012Validator(schema)
vorher = jsonschema.Draft202012Validator(alt)

FAELLE = [
    ("§4.1 Gate 7", "ungueltig/insert-mit-beitragsklasse.json"),
    ("§4.1 Gate 7", "ungueltig/pre-mit-beitragsklasse.json"),
    ("§4.1 Gate 7", "ungueltig/post-mit-beitragsklasse.json"),
    ("§4.1 Gate 7", "ungueltig/beitragsposition-beobachtend.json"),
    ("§4.5 hash  ", "ungueltig/state-hash-alphabet.json"),
    ("§4.5 hash  ", "ungueltig/state-hash-grossbuchstaben.json"),
    ("§4.5 bitmap", "ungueltig/bitmap-alphabet-fremd.json"),
    ("§4.5 bitmap", "ungueltig/bitmap-fuellzeichen-vorn.json"),
    ("§4.5 bitmap", "ungueltig/bitmap-fuellbits-gesetzt.json"),
    ("§4.5 bitmap", "ungueltig/bitmap-grob-fuellbits-gesetzt.json"),
    ("ACK hash", "ungueltig/command-ack-angewandt-ohne-state-hash.json"),
    ("ACK hash", "ungueltig/command-ack-angewandt-state-hash-null.json"),
    ("ACK hash", "ungueltig/command-ack-idempotent-ohne-state-hash.json"),
    ("Band q0p1", "ungueltig/bandwert-q0p1-nicht-ganzzahlig.json"),
    ("Band q0p1", "ungueltig/bandwert-q0p1-unter-minimum.json"),
    ("Band q0p1", "ungueltig/bandwert-q0p1-ueber-maximum.json"),
    ("Band q0p01", "ungueltig/bandwert-q0p01-nicht-ganzzahlig.json"),
    ("Band q0p01", "ungueltig/bandwert-q0p01-unter-minimum.json"),
    ("Band q0p01", "ungueltig/bandwert-q0p01-ueber-maximum.json"),
    ("pair_id", "ungueltig/pair-id-leer.json"),
    ("pair_id", "ungueltig/pair-id-65-codepoints.json"),
]

print(f"{'Befund':12} {'Fixture':38} {'VOR dem Fix':>12}  {'NACH dem Fix':>13}")
print("-" * 80)
schlecht = 0

# Der alte Scanner uebersprang alphabetische Tokens vollstaendig. Fuer genau
# den neuen Fall rekonstruiert `null` diesen Vorzustand im Speicher: alter und
# neuer Scanner lassen das erlaubte Literal passieren, waehrend nur der neue
# Scanner das an derselben Stelle stehende NaN ablehnt. Alle anderen Bytes und
# damit alle bisherigen Riegel bleiben unveraendert in der Vorfuehrung.
nan_rel = "ungueltig/zahl-nan-token.json"
nan_roh = (FIX / nan_rel).read_bytes()
nan_marke = b'"werte": [NaN,'
if nan_roh.count(nan_marke) != 1:
    print(f"REGRESSION: {nan_rel} traegt nicht genau ein erwartetes NaN-Token")
    schlecht += 1
    nan_vor_sauber = False
else:
    nan_vor_roh = nan_roh.replace(nan_marke, b'"werte": [null,', 1)
    nan_vor_sauber = textriegel_bytes(nan_vor_roh) is None
nan_nach_sauber = textriegel_bytes(nan_roh) is None
if not (nan_vor_sauber and not nan_nach_sauber):
    schlecht += 1
print(f"{'NaN-Riegel':12} {pathlib.Path(nan_rel).name:38} "
      f"{'gueltig' if nan_vor_sauber else 'abgelehnt':>12}  "
      f"{'gueltig' if nan_nach_sauber else 'abgelehnt':>13}")

for befund, rel in FAELLE:
    daten = json.loads((FIX / rel).read_text(encoding="utf-8"))
    v = vorher.is_valid(daten)
    n = neu.is_valid(daten)
    if not (v and not n):
        schlecht += 1
    print(f"{befund:12} {pathlib.Path(rel).name:38} "
          f"{'GUELTIG (!)' if v else 'abgelehnt':>12}  {'abgelehnt' if not n else 'GUELTIG (!)':>13}")

print("-" * 80)
print("Lesart: 'GUELTIG (!)' vor dem Fix = der Vertrag liess den Fall durch.")
print("        'abgelehnt' nach dem Fix  = der neue Riegel faengt ihn.")

# Gegenprobe: der gueltige Korpus bleibt gueltig. Ein Riegel, der ALLES
# ablehnt, faengt die Faelle oben auch - und waere trotzdem falsch.
gueltig_ok = 0
for p in sorted((FIX / "gueltig").glob("*.json")):
    daten = json.loads(p.read_text(encoding="utf-8"))
    if not neu.is_valid(daten):
        print(f"REGRESSION: {p.name} war gueltig und ist es nicht mehr")
        schlecht += 1
    else:
        gueltig_ok += 1
print(f"Gegenprobe: {gueltig_ok} gueltige Fixtures sind weiterhin gueltig "
      f"(darunter session-beitragsklasse.json = die ERLAUBTE Paarung).")
print(f"NaN-Textriegel: VOR={'gueltig' if nan_vor_sauber else 'abgelehnt'}/"
      f"NACH={'gueltig' if nan_nach_sauber else 'abgelehnt'}")

if schlecht:
    print(f"\nROT: {schlecht} Faelle verhalten sich nicht wie behauptet")
    sys.exit(1)
print("\nGRUEN: jeder neue Riegel ist beim Fallen vorgefuehrt.")
