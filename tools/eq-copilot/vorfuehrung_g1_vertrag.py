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

WURZEL = pathlib.Path(sys.argv[1])
SCHEMA = WURZEL / "eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
FIX = WURZEL / "eq-copilot/fixtures/v3"

schema = json.loads(SCHEMA.read_text(encoding="utf-8"))

# --- Vorzustand vom 23.08.: die beiden Felder standen UNGEKOPPELT nebeneinander.
alt = json.loads(json.dumps(schema))
d = alt["$defs"]
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
]

print(f"{'Befund':12} {'Fixture':38} {'VOR dem Fix':>12}  {'NACH dem Fix':>13}")
print("-" * 80)
schlecht = 0
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

if schlecht:
    print(f"\nROT: {schlecht} Faelle verhalten sich nicht wie behauptet")
    sys.exit(1)
print("\nGRUEN: jeder neue Riegel ist beim Fallen vorgefuehrt.")
