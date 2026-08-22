import json, pathlib
d = json.loads(pathlib.Path("docs/hub/hub.json").read_text(encoding="utf-8"))
a = d.get("antworten") or {}
def num(k):
    t = k.replace("U", "").split(".")
    try: return tuple(int(x) for x in t)
    except ValueError: return (999,)
zeilen = ["# Deine Antworten — noch nicht eingearbeitet", "",
          "> **Erzeugt** aus `docs/hub/hub.json` mit `tools/hub/antworten_blatt.py`.",
          "> Quelle ist die Briefing-Seite; hier nur zum Lesen.", "",
          f"**{len(a)} Antworten**, gegeben am 22.08.2026. Status aller: `neu` — "
          "das heisst: noch **nicht** ins Register (CLAUDE.md) oder in "
          "`design/abnahmen/` uebernommen.", "",
          "Pflicht laut CLAUDE.md: jede Antwort ist User-Wort und gehoert mit "
          "Datum + Wortlaut eingetragen; danach Status `eingearbeitet` + "
          "`ergebnis` in `hub.json`.", "",
          "| Frage | Deine Wahl | Deine Anmerkung | wann |", "|---|---|---|---|"]
offen = []
for k in sorted(a, key=num):
    v = a[k] if isinstance(a[k], dict) else {}
    wahl = str(v.get("wahl") or v.get("choice") or "").replace("|", chr(92)+"|")
    text = str(v.get("text") or v.get("note") or "").replace("|", chr(92)+"|").replace("\n", " ")
    datum = str(v.get("datum") or "")[:16]
    zeilen.append(f"| `{k}` | {wahl or '—'} | {text or '—'} | {datum} |")
    if text: offen.append((k, wahl, text))
zeilen += ["", f"## Die {len(offen)} Antworten mit eigenem Text", "",
           "Diese tragen mehr als eine Auswahl — hier steckt die eigentliche Aussage:", ""]
for k, wahl, text in offen:
    zeilen.append(f"- **`{k}`** — Wahl *{wahl}*: „{text}\"")
zeilen += ["", "---", "",
           "Nicht von Hand pflegen: beim naechsten Lauf des Erzeugers wird das Blatt "
           "vollstaendig ueberschrieben."]
pathlib.Path("docs/ANTWORTEN-OFFEN.md").write_text("\n".join(zeilen) + "\n", encoding="utf-8")
print(f"geschrieben: docs/ANTWORTEN-OFFEN.md ({len(a)} Antworten, {len(offen)} mit eigenem Text)")
