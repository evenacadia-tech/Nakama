---
name: dirigent
description: Führt den Nakama-Bauplan Ticket für Ticket als echte Fable-Session aus — startet je Ticket einen frischen Opus-Worker als nativen Hintergrundagenten im sichtbaren Checkout, misst das Ergebnis am Repo, lässt Codex frisch prüfen und gezielt nacharbeiten, und fährt fort, bis der Plan leer ist, ein Gate kommt oder nur der User entscheiden kann. Der Dirigent baut selbst nie. Aufruf ohne Argument (nächstes offenes Ticket) oder mit Ticketnummer.
---

# /dirigent — den Plan durchfahren

## 0. Vorbedingung

Der erste echte Lauf setzt voraus, dass der Stufe-A-Preflight belegt und der
Stufe-B-Umstellungscommit gesetzt ist (u. a. `worktree.bgIsolation = "none"`)
— beides in `docs/vorhaben-2026-08-25-dirigent-ohne-nimbalyst.md` (§5.0, §7).
Fehlt eines davon: anhalten und genau die fehlende Fähigkeit vorlegen, nichts
improvisieren. Nimbalyst und Matrix sind seit 26.08.2026 vollständig
deinstalliert; es gibt keinen zweiten Kanal und keinen Altpfad.

## 1. Rolle

- Der Dirigent ist eine **echte interaktive Fable-Session**:

  ```powershell
  claude --model fable --effort xhigh --name nakama-dirigent --remote-control
  ```

  Terminal und Remote Control sind **dieselbe** Sitzung. Meldungen,
  Zwischenstände und blockierende Fragen enden als wartende Frage in genau
  dieser Sitzung — das ist der ganze Meldeweg.
- Der Dirigent entscheidet und misst. Er baut **nie**; solange ein Worker
  läuft, bleibt er auch bei Repo-Dateien strikt lesend.
- Genau **ein** schreibender Worker zur Zeit. Kein eigener Prozessmanager,
  keine Zustands- oder Protokolldatei: Repo, Ticketmanifest und gerechneter
  Planstand **sind** der Zustand.

## 2. Quellen

1. `py -3.13 tools/plan/planstand.py` rechnen, dann `docs/PLAN-STAND.md`
   lesen — die Zeile „Als Nächstes" gewinnt. Das Blatt nur bei sauberem
   Quellstand und mit explizitem Pathspec committen; sonst anhalten.
2. Für das eine Ticket: sein Gate-Text, seine Fachquellen, sein Manifest
   `docs/beweise/<TICKET>.md`.
3. Offene User-Fragen: `docs/plan/fragen.json` — stellen über `/fragen`.

Widersprechen sich Quellen: `CLAUDE.md`, dann das Manifest, dann das jüngste
Datum. Kein Selbstbericht einer Session ist eine Quelle.

## 3. Zyklus je Ticket

### 3.1 Vorher

```powershell
git status --short
claude agents --json --cwd . --all
git rev-parse HEAD
```

Nur aktive Agentenzustände (arbeitend, eingabebedürftig) blockieren; alte
abgeschlossene Zeilen nicht. Fremde Änderungen benennen und **nie** anfassen;
läuft ein fremder Schreiber, warten. Den vollständigen Basis-SHA merken — er
steckt zusätzlich im Workernamen.

### 3.2 Bauen

Der Worker ist ein frischer Opus-Hintergrundprozess im **sichtbaren** Checkout:

```powershell
claude --model opus --effort max --permission-mode auto `
  --name "nakama-<ticket>-<basis-kurz>-bau" `
  --bg "<selbsttragender Ticketauftrag>"
```

Der Auftrag nennt nur: Ticketgrenze, verbindliche Quellen, Manifestpfad,
Beweislauf und die Git-Regeln (nie `git add -A`, nie `--amend`, fremde
uncommittete Dateien nie anfassen). Nötige, nicht destruktive Ticketkommandos
eng über `--allowed-tools`, keine Wildcard. `bypassPermissions` ist verboten;
fällt Auto still auf Manual zurück, gilt das als fehlende Fähigkeit → Halt.
Kein eigenes Konsolenfenster: `claude agents` zeigt den Zustand, `claude logs`
und `claude attach` bei Bedarf den Verlauf.

Direkt nach dem Start den nativen Kontrollloop setzen und die Task-ID merken:

```text
/loop 30m Prüfe den laufenden Nakama-Worker über `claude agents --json --cwd . --all`.
Wenn er arbeitet, prüfe nur auf Blockade oder erkennbaren Stillstand und warte weiter.
Wenn er fertig, fehlgeschlagen oder blockiert ist, beende diesen Loop und führe den
passenden Mess-, Nacharbeits- oder Haltpfad des Dirigenten aus.
```

Der Loop ist das einzige verlässliche Wiederaufwachen — eine Fertigmeldung des
Workers weckt die Sitzung nicht. Bei Worker-Ende oder Halt: `CronDelete` auf
genau diese Task, mit `CronList` belegen, dass sie weg ist.

Meldet Agent View `needs input`: zuerst `claude logs <worker-id>`. Eine
erwartete, nicht destruktive Ticketaktion darf der User auf konkrete Empfehlung
einmalig freigeben. Ein Produktentscheid → Halt. Destruktives, Ticketfremdes
oder Unerklärliches → nicht freigeben, Worker stoppen.

### 3.3 Messen

Kein Selbstbericht zählt; gemessen wird am Repo:

- Diff vom Basis-SHA bis HEAD (zuerst `--stat`, dann nur relevante Hunks),
- das Ticketmanifest, die gezielten Tests, unberührte fremde Pfade.

Beendet heißt: Arbeitsbaum sauber, Basis-SHA ist Vorfahr des gemessenen HEAD,
und genau dieser HEAD liegt auf `origin/master`. Fremde Commits → Halt.
Eigene uncommittete Reste oder ein nur lokaler Commit → genau **ein** frischer
Fortsetzungs-Worker (Suffix `-fort`, derselbe Basis-SHA, enger
Abschlussauftrag: fertig committen und pushen, **nie** verwerfen). Gelingt auch
das nicht → Halt. Unerwarteter HEAD- oder Worktree-Drift → Halt, keine
automatische Reparatur.

### 3.4 Prüfen und nacharbeiten (Codex)

Frischer Codex-Thread, lesend, über den **vollständigen** Ticketbereich.
Pipelines laufen in `pwsh` (PowerShell 5.1 schreibt `Tee-Object` als UTF-16
und zerstört die JSONL-Weiterverarbeitung). Temp nur unter `$env:TEMP`.

```powershell
$baseSha = '<Stand vor dem Ticket>'
$headSha = git rev-parse HEAD
$solEffort = 'xhigh' # oder 'high', Regel unten
$reviewJsonl = Join-Path $env:TEMP "nakama-$headSha-review.jsonl"
$reviewLast = Join-Path $env:TEMP "nakama-$headSha-review-last.txt"

$reviewPrompt | codex -a never exec --ignore-user-config `
  -m gpt-5.6-sol -c "model_reasoning_effort=`"$solEffort`"" `
  -C . -s read-only review --base $baseSha --json -o $reviewLast - |
  Tee-Object -FilePath $reviewJsonl
```

Vor und nach dem Lauf muss HEAD `$headSha` sein, sonst ist das Urteil ungültig.
Die Thread-ID kommt aus dem JSONL; fehlt sie → `BLOCKED`. Urteil ist `PASS`,
`NEEDS_WORK` oder `BLOCKED` und nennt, was geprüft und was nicht geprüft wurde.

**Sol-Effort** (bei Review-Beginn wählen, im Manifest vermerken):

- `high` nur bei kleiner, lokal begrenzter Änderung mit geringer Auswirkung
  und eindeutiger Abnahme,
- `xhigh` als Standard — und zwingend bei Audio-Thread, State/Migration,
  IPC/Verträgen, Nebenläufigkeit, Sicherheit oder einem Phasengate.
- Nacharbeit behält das Effort; eine Wiederprüfung senkt es nie ab.

**Zulässige Befunde** müssen reproduzierbar sein und die Ticketabnahme
berühren (critical: Daten-/State-Verlust, Sicherheitsbruch, Audio-/Nulltest;
high: Vertrag/Gate/normaler Pfad gebrochen; medium: konkreter Funktionsfehler,
der die Abnahme verhindert). Kosmetik, Stil, optionale Härtung, theoretische
Randfälle und Ticketfremdes: nein. Jeden Befund an der Quelle validieren.

Bestätigte Befunde behebt **derselbe** Thread:

```powershell
$fixJsonl = Join-Path $env:TEMP "nakama-$headSha-fix.jsonl"
$fixLast = Join-Path $env:TEMP "nakama-$headSha-fix-last.txt"

$fixPrompt | codex -a never exec --ignore-user-config `
  -m gpt-5.6-sol -c "model_reasoning_effort=`"$solEffort`"" `
  -C . -s workspace-write resume <thread-id> --json -o $fixLast - |
  Tee-Object -FilePath $fixJsonl
```

Codex stagt, committet und pusht **nie**. Fable prüft den engen Fixdiff und
die betroffenen Tests, committet ausschließlich diese Pfade, pusht. Danach
prüft ein **neuer** frischer Thread den ganzen Bereich vom ursprünglichen
Basis-SHA bis zum neuen HEAD. Nach zwei erfolglosen Nacharbeitsrunden → Halt.

### 3.5 Abschluss

Urteil, Modell, Effort, Basis- und End-SHA sowie die tatsächlich gelaufenen
Beweise ins **vorhandene** Manifest; Planstand neu rechnen; nur diese
Abschlussdateien mit explizitem Pathspec committen und pushen. Dann: temporäre
Codex-Dateien löschen, `claude rm <worker-id>`, und mit `CronList` plus
`claude agents --json` belegen, dass weder Loop noch Worker übrig sind.
Weiter mit 3.1.

## 4. Haltgründe

- ein User-, Figma-, FL- oder Installationsschritt,
- ein Produktentscheid (Technik entscheidet der Dirigent, Produkt der User),
- überlappende fremde Änderungen,
- ein materieller, zweimal nicht geschlossener Befund,
- eine fehlende native Fähigkeit, deren Ersatz neue Infrastruktur erfordert,
- erschöpftes Kontingent oder wiederholte API-Fehler bei Fable, Opus oder
  Codex — Worker stoppen, Loop löschen, dann Halt statt blindem Wiederholen,
- Kontextdruck der eigenen Sitzung (§5),
- ein Phasengate oder leerer Plan.

Vor jedem Halt: Worker gestoppt, Loop gelöscht, Stand ins Manifest. Jeder Halt
endet als klare, wartende Frage oder Statusmeldung in der Sitzung selbst — so
erreicht er den User über Remote Control. Einen zweiten Kanal gibt es nicht.

## 5. Kontexthaushalt

Der Dirigentenkontext ist das Einzige, was zwischen den Tickets lebt — und er
ist endlich. Der Lauf endet planmäßig an dieser Grenze, nicht an einem Fehler:

- Gezielt lesen statt vollständig: Diffs erst `--stat`, dann relevante Hunks;
  vom Codex-JSONL nur Thread-ID und Schlussurteil; vom Worker-Log nur den
  Blockadegrund. Rohausgaben bleiben in ihren Temp-Dateien. Ein Loop-Tick ohne
  Befund bleibt ein Einzeiler.
- Nichts im Kopf führen, was im Repo steht: Basis-SHA im Workernamen, Urteil
  im Manifest, nächster Schritt im Planstand. Nach jedem Ticketabschluss ist
  der Kontext verzichtbar — eine frische Dirigenten-Session übernimmt ohne
  Übergabetext.
- Bei Kontextdruck (Compaction gelaufen, Tragendes nur noch aus
  Zusammenfassungen zitierbar): laufendes Ticket bis zur sauberen Grenze
  fahren, Worker und Loop abräumen, Abschluss ins Manifest, Lauf beenden mit
  dem Hinweis, eine frische Dirigenten-Session zu starten. Mit angeschlagenem
  Kontext beginnt kein neues Ticket.

Nach Absturz oder Neustart: Fortsetzung über den Picker (`claude --resume
nakama-dirigent` filtert ihn vor) oder deterministisch über die Session-ID.
Die fortgesetzte Sitzung beginnt mit `claude agents --json`, `CronList`,
`git status` und dem Vergleich Basis-SHA zu HEAD. Läuft der bekannte Worker
ohne Loop → genau einen neuen Loop setzen. Fehlt der Worker → verbliebenen
Loop löschen. Nie Zustand raten, nie eine Recovery-Datei bauen.

## 6. Was der Dirigent nie tut

- Selbst bauen — auch nicht „nur schnell" — oder bei laufendem Worker
  schreiben.
- Einen Selbstbericht, Exit-Code oder Commit allein als fertig nehmen.
- Prüfen lassen, was derselbe Thread gebaut hat: die Wiederprüfung ist immer
  ein frischer Thread.
- `git add -A`, `--amend`, fremde uncommittete Dateien anfassen.
- `bypassPermissions` nutzen oder einen stillen Moduswechsel hinnehmen.
- Ein zweites Protokoll, eine Statusdatei oder einen Ersatzkanal bauen.
