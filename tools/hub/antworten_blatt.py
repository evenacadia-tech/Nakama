"""Erzeugt docs/ANTWORTEN-OFFEN.md aus docs/hub/hub.json.

EINE Wahrheit, ein Ort: hub.json ist die Quelle, dieses Blatt ist NUR eine
Ansicht davon. Es wird bei jedem Lauf vollstaendig ueberschrieben.

Der Status kommt aus den Daten, nicht aus dem Text: solange eine Antwort
`neu` ist, sagt das Blatt das; sobald sie `eingearbeitet` ist, zeigt es das
Ergebnis. Frueher standen Titel und Statussatz als feste Zeichenketten hier
drin — das Blatt behauptete dann weiter "noch nicht eingearbeitet", auch
wenn laengst alles uebernommen war.
"""
import json
import pathlib

QUELLE = pathlib.Path("docs/hub/hub.json")
ZIEL = pathlib.Path("docs/ANTWORTEN-OFFEN.md")

NEU = "neu"
FERTIG = "eingearbeitet"


def num(k):
    """U2.10 sortiert hinter U2.9, nicht dazwischen."""
    t = k.replace("U", "").split(".")
    try:
        return tuple(int(x) for x in t)
    except ValueError:
        return (999,)


def zelle(wert):
    """Pipes und Umbrueche zerlegen sonst die Tabelle."""
    return str(wert or "").replace("|", "\\|").replace("\n", " ").strip()


def main() -> int:
    d = json.loads(QUELLE.read_text(encoding="utf-8"))
    a = d.get("antworten") or {}
    schluessel = sorted(a, key=num)
    eintrag = {k: (a[k] if isinstance(a[k], dict) else {}) for k in schluessel}

    offen = [k for k in schluessel if (eintrag[k].get("status") or NEU) == NEU]
    fertig = [k for k in schluessel if (eintrag[k].get("status") or NEU) == FERTIG]
    andere = [k for k in schluessel if k not in offen and k not in fertig]

    daten = sorted(str(eintrag[k].get("datum") or "")[:10] for k in schluessel if eintrag[k].get("datum"))
    spanne = ""
    if daten:
        spanne = daten[0] if daten[0] == daten[-1] else f"{daten[0]} bis {daten[-1]}"

    if not a:
        titel = "# Deine Antworten — noch keine da"
    elif offen:
        titel = f"# Deine Antworten — {len(offen)} noch nicht eingearbeitet"
    else:
        titel = "# Deine Antworten — alle eingearbeitet"

    aus = [titel, "",
           "> **Erzeugt, nicht gepflegt.** Quelle ist `docs/hub/hub.json`;",
           "> hier nur zum Lesen. Handarbeit geht beim naechsten Lauf verloren.",
           ""]

    teile = [f"**{len(a)} Antworten**"]
    if spanne:
        teile.append(f"gegeben am {spanne}")
    aus.append(", ".join(teile) + ".")
    aus.append("")

    if offen:
        aus += [f"Davon **{len(offen)} mit Status `neu`** — also noch **nicht** ins Register "
                "(`CLAUDE.md`) oder nach `design/abnahmen/` uebernommen.", "",
                "Pflicht laut CLAUDE.md: jede Antwort ist User-Wort und gehoert mit Datum "
                "und Wortlaut eingetragen; danach Status `eingearbeitet` und `ergebnis` "
                "in `hub.json`.", ""]
    else:
        aus += ["Alle tragen Status `eingearbeitet` — jede ist mit Datum und Wortlaut "
                "uebernommen. Was daraus gilt, steht in der Spalte *Ergebnis*.", ""]
    if andere:
        aus += [f"{len(andere)} Antwort(en) tragen einen anderen Status: "
                + ", ".join(f"`{k}` ({eintrag[k].get('status')})" for k in andere) + ".", ""]

    aus += ["| Frage | Deine Wahl | Deine Anmerkung | wann | Ergebnis |",
            "|---|---|---|---|---|"]
    for k in schluessel:
        v = eintrag[k]
        wahl = zelle(v.get("wahl") or v.get("choice"))
        text = zelle(v.get("text") or v.get("note"))
        datum = zelle(v.get("datum"))[:16]
        erg = zelle(v.get("ergebnis")) or ("— noch offen —" if (v.get("status") or NEU) == NEU else "—")
        aus.append(f"| `{k}` | {wahl or '—'} | {text or '—'} | {datum} | {erg} |")

    eigen = [k for k in schluessel if zelle(eintrag[k].get("text") or eintrag[k].get("note"))]
    if eigen:
        aus += ["", f"## Die {len(eigen)} Antworten mit eigenem Text", "",
                "Diese tragen mehr als eine Auswahl — hier steckt die eigentliche Aussage:", ""]
        for k in eigen:
            v = eintrag[k]
            aus.append(f"- **`{k}`** — Wahl *{zelle(v.get('wahl') or v.get('choice'))}*: "
                       f"„{zelle(v.get('text') or v.get('note'))}\"")

    ZIEL.write_text("\n".join(aus) + "\n", encoding="utf-8", newline="\n")
    print(f"geschrieben: {ZIEL} ({len(a)} Antworten, {len(offen)} offen, "
          f"{len(fertig)} eingearbeitet, {len(eigen)} mit eigenem Text)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
