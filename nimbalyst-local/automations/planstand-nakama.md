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
  runCount: 4
  nextRun: "2026-08-26T07:00:00.000Z"
  lastRun: "2026-08-25T07:02:12.146Z"
  lastRunStatus: success
---
# Planstand Nakama auffrischen

Frische das Planblatt `docs/PLAN-STAND.md` im Nakama-Repo auf.

**Was sich am 23.08.2026 geändert hat:** Die Briefing-Seite ist abgeschafft
(User-Wort im Register von `CLAUDE.md`). Der Planstand wird nicht mehr aus
einem gepflegten Statusfeld übernommen, sondern **aus dem Repo gerechnet**:
Beweismanifest da = gebaut, Urteilsmarke mit PASS = abgenommen. Während einer
Session hält `tools/hooks/planstand.sh` das Blatt selbst frisch — diese
Automatik ist das Netz für die Zeit, in der keine Session läuft.

## Was zu tun ist

1. `git pull --ff-only` (der User arbeitet an zwei Rechnern). Scheitert das,
   melde es und brich ab — nichts erzwingen.

2. Erzeuge das Blatt neu:
   `py -3.13 tools/plan/planstand.py`
   Es liest `docs/plan/plan.json` (Text) und die Urteilsmarken in
   `docs/beweise/` (Status) und überschreibt `docs/PLAN-STAND.md` vollständig.
   **Exitcode 4 heißt: eine Urteilsmarke ist unlesbar** — das gehört in die
   Meldung, denn betroffene Schritte fallen dann auf „nicht abgenommen".

3. Ist `docs/PLAN-STAND.md` danach verändert, committe **nur diese eine
   Datei** per explizitem Pathspec:
   `git add docs/PLAN-STAND.md && git commit -m "Planstand aufgefrischt"`
   Hat sich nichts geändert, committe nichts.

## Harte Regeln

- **`docs/PLAN-STAND.md` niemals von Hand bearbeiten.** Es ist eine erzeugte
  Ansicht; jeder Lauf überschreibt sie.
- **Nichts unter `docs/plan/` oder `docs/beweise/` in diesem Lauf verändern.**
  Diese Automatik misst und stellt dar, sie urteilt nicht. Eine Urteilsmarke
  setzt nur ein Prüfer.
- **Niemals `git add -A` oder `git add .`** — in diesem Repo laufen parallele
  Sessions, und fremde uncommittete Dateien dürfen nicht mitgenommen werden.
  Immer nur der eine Pfad oben.
- Liegt unter `docs/plan/` oder `docs/beweise/` etwas Uncommittetes, arbeitet
  gerade jemand daran: Blatt trotzdem erzeugen, aber **nicht** committen —
  sonst kollidiert der Commit mit fremder Arbeit. Melde das stattdessen.

## Meldung

Zwei Zeilen: der gemessene Stand (wie viele abgenommen, wie viele gebaut ohne
Urteil, wie viele offen) und was als Nächstes dransteht.

⚠️ **„Das Blatt hat sich nicht geändert" ist NICHT dasselbe wie „im Projekt ist
nichts passiert".** Genau diese Verwechslung hat am 23.08. eine falsche
Meldung erzeugt. Wenn du „unverändert" meldest, sag dazu, worauf sich das
bezieht — und nenne den Quellstand (die Zeile `<!-- quellstand: … -->` im
Blatt) sowie die Zahl der Commits seither
(`git rev-list --count <quellstand>..HEAD`).
