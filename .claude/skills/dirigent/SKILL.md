---
name: dirigent
description: Führt den Nakama-Bauplan Ticket für Ticket als echte Fable-Session aus — startet je Ticket einen frischen Opus-Worker als nativen Hintergrundagenten im sichtbaren Checkout, misst das Ergebnis am Repo, lässt Codex frisch prüfen und gezielt nacharbeiten, und fährt fort, bis der Plan leer ist, ein Gate kommt oder nur der User entscheiden kann. Der Dirigent baut selbst nie. Aufruf ohne Argument (nächstes offenes Ticket) oder mit Ticketnummer.
---

# /dirigent — den Plan durchfahren

## 0. Start und Cockpit

Der Dirigent ist eine echte interaktive Fable-Session:

```powershell
pwsh -NoProfile -File tools/dirigent/start-dirigent.ps1
```

Der Starter öffnet das lokale Windows-Terminal-Profil
`Nakama · Champagne Night`, zeigt `tools/dirigent/logo.ps1` und ruft Claude mit
Fable/xhigh, Auto-Modus und `/dirigent` auf. Fehlt das Profil oder scheitert die
Terminal-Aktivierung, öffnet er ein normales lokales PowerShell-Fenster. Remote
Control bleibt aus; das lokale Terminal ist der einzige Bedien- und Meldekanal.
Der direkte Ersatzaufruf lautet:

```powershell
claude --model fable --effort xhigh --permission-mode auto --name nakama-dirigent /dirigent
```

Die projektweite native `statusLine` startet
`tools/dirigent/cockpit.ps1 -StatusLine`. Vor dem ersten Worker müssen dort
Fable/xhigh, das echte Kontextfenster, Claude- und Codex-Kontingente sowie der
frische Planstand ehrlich lesbar sein. `nicht verfügbar`, ein unbekannter
Arbeitsanker oder ein zusätzliches Worktree wird nie grün dargestellt.

`/dirigent stand` bedeutet:

```powershell
pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Plan
pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Plan -CurrentStep <Schritt-ID> # während eines Tickets
```

Die Ansicht muss dieselben Fertig-/Offen-Zahlen wie `docs/PLAN-STAND.md`
zeigen. Ein voller Ticketlauf beginnt nicht, wenn eine dafür nötige native
Fähigkeit ungeprüft ist. Ein einzelner ungültiger CLI-Aufruf ist aber noch kein
Halt: lokale Hilfe, aktuelle offizielle Doku und die kleinste semantisch
gleiche Konstruktion werden zuerst geprüft.

## 1. Rolle

- Der ausdrückliche Auto-Modus verhindert, dass ein globaler
  `bypassPermissions`-Default geerbt wird.
- Der Dirigent entscheidet und misst. Er baut **nie**; solange ein Worker
  läuft, bleibt er auch bei Repo-Dateien strikt lesend.
- Technische Wege innerhalb von Ticket, Produktinvarianten und freigegebener
  Designrichtung entscheidet Fable selbst und begründet Abweichungen im
  vorhandenen Manifest. Produktwirkung, Bedienlogik, sichtbare Priorität und
  kreative Richtung entscheidet der User.
- Genau **ein** schreibender Worker zur Zeit. Kein eigener Prozessmanager,
  keine Zustands- oder Protokolldatei: Repo, Ticketmanifest und gerechneter
  Planstand **sind** der Zustand.

Für jede Arbeitsphase setzt Fable eine Aufsichtsstufe. Wichtige Ereignisse
melden sich unabhängig davon sofort:

| Aufsicht | Kontrolle | Einsatz |
|---|---:|---|
| `LOCKER` | 30 Minuten | klein, lokal, leicht rücknehmbar |
| `NORMAL` | 15 Minuten | normales Ticket oder mehrere gekoppelte Dateien |
| `ENG` | 5 Minuten | Audio, State, Vertrag, Nebenläufigkeit, Sicherheit, Gate oder Nacharbeit |

Hochstufen gilt sofort; herabstufen erst nach einer erfolgreichen Kontrolle.
Beim Wechsel wird der alte Loop zuerst gelöscht und sein Fehlen geprüft, erst
dann entsteht genau ein neuer.

## 2. Quellen

1. `py -3.13 tools/plan/planstand.py` rechnen, dann `docs/PLAN-STAND.md`
   lesen — die Zeile „Als Nächstes" gewinnt. Das Blatt nur bei sauberem
   Quellstand und mit explizitem Pathspec committen und direkt pushen; sonst
   anhalten. Es gibt keinen Hook, der das nachholt.
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
py -3.13 tools/plan/planstand.py
pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Plan
```

Nur aktive Agentenzustände (arbeitend, eingabebedürftig) blockieren; alte
abgeschlossene Zeilen nicht. Fremde Änderungen benennen und **nie** anfassen;
läuft ein fremder Schreiber, warten. Den vollständigen Basis-SHA merken — er
steckt zusätzlich im Workernamen. Gate-Text, Fachquellen und Manifest nur für
das konkrete Paket lesen; historische Übergaben und Protokolle sind keine
Arbeitsquellen mehr.

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

Direkt nach dem Start den Zustandsbeobachter als Hintergrundkommando derselben
Fable-Sitzung starten und mit Claudes nativem `Monitor` beobachten:

```powershell
pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -WatchWorker `
  -WorkerId <worker-id> -BaseSha <basis-sha> -Aufsicht <LOCKER|NORMAL|ENG> `
  -StartModel Opus -StartEffort max -DirigentSessionId <session-id>
```

Der Helfer meldet nur Zustandsänderungen, HEAD-/Worktree-Drift und eine alte
oder kritische Telemetriequelle; er schreibt kein Log und keine Projektdatei
und endet mit dem Worker. Zusätzlich genau einen nativen Kontrollloop im
Intervall der Aufsicht setzen und die Task-ID im Sitzungskontext merken:

```text
/loop <30m|15m|5m> Prüfe den laufenden Nakama-Worker und seinen Ereignisbeobachter.
Wenn beides gesund arbeitet, bestätige nur die Spurlage und warte weiter. Wenn der
Worker fertig, fehlgeschlagen, blockiert oder der Beobachter ausgefallen ist, beende
diesen Loop und führe den passenden Mess-, Nacharbeits- oder Haltpfad aus.
```

Der Monitor ist der schnelle Weg, der Loop das zeitliche Sicherheitsnetz. Bei
Worker-Ende oder Halt: Beobachter beenden, `CronDelete` auf genau diese Task
und mit `CronList` belegen, dass beides weg ist.

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

# $reviewPrompt ist das einzige Review-Ziel. Er nennt Basis- und Ziel-SHA,
# unveränderten Gate-Text, Manifest und den vollständigen Ticketbereich.
$reviewPrompt | codex -a never exec --ignore-user-config `
  -m gpt-5.6-sol -c "model_reasoning_effort=`"$solEffort`"" `
  -c 'windows.sandbox="elevated"' `
  -C . -s read-only review --json -o $reviewLast - |
  Tee-Object -FilePath $reviewJsonl
```

Ein eigener Review-Prompt und `--base` sind in Codex gegenseitig exklusiv.
Deshalb muss der Prompt Codex ausdrücklich auf
`git diff $baseSha...$headSha`, Gate und Manifest begrenzen; `--base` darf in
dieser Form nicht ergänzt werden. Die explizite Windows-Sandbox-Auswahl ist
nötig, weil `--ignore-user-config` sonst auf diesem Rechner bereits lesende
Git-Prozesse blockiert.

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
  -c 'windows.sandbox="elevated"' `
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
Codex-Dateien und alle exakt zur Dirigenten-Session gehörenden
`$env:TEMP\nakama-dirigent-<session-id>-*.json`-Caches löschen,
`claude rm <worker-id>`, und mit beendetem Beobachter, `CronList` plus
`claude agents --json` belegen, dass weder Loop noch Worker übrig sind. Weiter
mit 3.1.

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

Ein einzelner CLI-Parse- oder Versionsfehler ist noch keine fehlende native
Fähigkeit. Vor dem Halt prüft Fable lokale Unterbefehlshilfe, aktuelle
offizielle Werkzeugdokumentation und die kleinste Alternative, die Ziel,
Sandbox, Modell/Effort, JSONL- und Thread-Vertrag unverändert erhält. Nur wenn
keine solche Variante funktioniert, greift der Halt; neue Infrastruktur bleibt
verboten.

Vor jedem Halt: Worker gestoppt, Loop gelöscht, Stand ins Manifest. Jeder Halt
endet als klare, wartende Frage oder Statusmeldung in der Sitzung selbst — so
erreicht er den User im lokalen Terminal. Einen zweiten Kanal gibt es nicht.

## 5. Kontexthaushalt

Der Dirigentenkontext ist das Einzige, was zwischen den Tickets lebt — und er
ist endlich. Der Lauf endet planmäßig an dieser Grenze, nicht an einem Fehler:

- Unter 70 % Kontextverbrauch normal arbeiten; 70 bis unter 85 % enger lesen
  und die Ticketgrenze vorbereiten; ab 85 % kein neues Ticket beginnen.
  Nach einer Compaction bleibt der Arbeitsanker unbestätigt, bis Planstand,
  Ticketquelle und HEAD erneut gelesen und abgeglichen sind.
- Für Claude- und Codex-Kontingente gilt: ab 85 % warnen, ab 95 % keine neue
  Arbeitsphase beginnen. Ein bereits laufender sicherer Abschluss darf nur
  ohne Qualitätsverlust bis zur sauberen Grenze geführt werden.

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
  fahren, Worker, Beobachter und Loop abräumen, Abschluss ins Manifest, Lauf beenden mit
  dem Hinweis, eine frische Dirigenten-Session zu starten. Mit angeschlagenem
  Kontext beginnt kein neues Ticket.

Nach Absturz oder Neustart: Fortsetzung über den Picker oder deterministisch
über die Session-ID; beide Wege setzen den Rollenvertrag erneut ausdrücklich:

```powershell
claude --resume nakama-dirigent --model fable --effort xhigh --permission-mode auto
claude --resume <session-id> --model fable --effort xhigh --permission-mode auto
```

Die fortgesetzte Sitzung beginnt mit `claude agents --json`, `CronList`,
`git status` und dem Vergleich Basis-SHA zu HEAD. Läuft der bekannte Worker
ohne Beobachter oder Loop → genau je einen neuen setzen. Fehlt der Worker →
verbliebenen Beobachter beenden und Loop löschen. Nie Zustand raten, nie eine
Recovery-Datei bauen.

## 6. Was der Dirigent nie tut

- Selbst bauen — auch nicht „nur schnell" — oder bei laufendem Worker
  schreiben.
- Einen Selbstbericht, Exit-Code oder Commit allein als fertig nehmen.
- Prüfen lassen, was derselbe Thread gebaut hat: die Wiederprüfung ist immer
  ein frischer Thread.
- `git add -A`, `--amend`, fremde uncommittete Dateien anfassen.
- `bypassPermissions` nutzen oder einen stillen Moduswechsel hinnehmen.
- Ein zweites Protokoll, eine Statusdatei oder einen Ersatzkanal bauen.
