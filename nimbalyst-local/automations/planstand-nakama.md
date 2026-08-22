---
automationStatus:
  id: planstand-nakama
  title: Planstand Nakama auffrischen
  enabled: true
  schedule:
    type: daily
    time: "09:00"
  output:
    mode: replace
    location: nimbalyst-local/automations/planstand-nakama/
    fileNameTemplate: "{{date}}-output.md"
  runCount: 0
  nextRun: "2026-08-23T07:00:00.000Z"
---
# Planstand Nakama auffrischen

Frische das Planblatt `docs/PLAN-STAND.md` im Nakama-Repo auf.

## Was zu tun ist

1. Hole zuerst die Antworten und neuen Punkte der Briefing-Seite:
   `py -3.13 tools/hub/hub_sync.py holen`
   Schlägt das fehl (kein Netz, Seite nicht erreichbar), arbeite mit dem
   vorhandenen `docs/hub/hub.json` weiter und vermerke das in der Meldung.

2. Erzeuge das Blatt neu:
   `py -3.13 tools/hub/plan_blatt.py`
   Das Skript liest `docs/hub/hub.json` und überschreibt `docs/PLAN-STAND.md`
   vollständig.

3. Ist `docs/PLAN-STAND.md` danach verändert, committe **nur diese eine
   Datei** per explizitem Pathspec:
   `git add docs/PLAN-STAND.md && git commit -m "Planstand aufgefrischt"`
   Hat sich nichts geändert, committe nichts.

## Harte Regeln

- **`docs/PLAN-STAND.md` niemals von Hand bearbeiten.** Es ist eine erzeugte
  Ansicht. Die einzige Wahrheit ist `docs/hub/hub.json`.
- **`docs/hub/hub.json` in diesem Lauf nicht verändern.** Der Planstand dort
  wird von der arbeitenden Session gepflegt, nicht von dieser Automatik. Diese
  Automatik liest nur und stellt dar.
- **Niemals `git add -A` oder `git add .`** — in diesem Repo laufen parallele
  Sessions, und fremde uncommittete Dateien dürfen nicht mitgenommen werden.
  Immer nur der eine Pfad oben.
- Ist `docs/hub/hub.json` uncommittet (`git status` zeigt sie als geändert),
  arbeitet gerade jemand daran: Blatt trotzdem erzeugen, aber **nicht**
  committen — sonst kollidiert der Commit mit fremder Arbeit. Melde das
  stattdessen.

## Meldung

Gib am Ende einen Zweizeiler zurück: wie viele Schritte von wie vielen fertig
sind, und ob sich seit dem letzten Lauf etwas geändert hat. Keine langen
Berichte.
