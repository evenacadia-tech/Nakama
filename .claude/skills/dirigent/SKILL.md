# /dirigent — den Plan durchfahren

## 0. Start und Cockpit

Der Dirigent ist eine interaktive Fable-Session:

```powershell
pwsh -NoProfile -File tools/dirigent/start-dirigent.ps1
```

Der Starter öffnet das Terminalprofil `Nakama · Champagne Night`, ruft Claude
wie unten auf (30.08., 01.09.2026; Modus nach §1; `[1m]` trägt das 1M-Fenster
für §5) und hält das Fenster: liegt `nakama-dirigent-neustart.marker` im
Temp-Ordner (`[IO.Path]::GetTempPath()`), startet sofort eine frische Session
darin, sonst wartet er auf Enter (neu) oder Esc (schließen). Ersatzaufruf ohne
Starter:

```powershell
claude --remote-control nakama-dirigent --model claude-fable-5-1[1m] --effort xhigh --permission-mode auto --name nakama-dirigent /dirigent
```

Die native `statusLine` startet `tools/dirigent/cockpit.ps1 -StatusLine`. Vor
dem ersten Worker müssen dort Fable/xhigh, echtes Kontextfenster, Claude- und
Codex-Kontingente und frischer Planstand lesbar sein; `nicht verfügbar`, ein
unbekannter Anker oder ein zusätzliches Worktree ist nie grün.

**Plan-Tab und STAND (NAK-256, 11.09.2026):** Der Starter öffnet im selben
Fenster den Tab „Nakama · Plan" (`tools/dirigent/plan-tab.ps1`, lesend);
`start-dirigent.ps1 -NurPlanTab` öffnet ihn neu. Statuszeile und Tab zeigen
`STAND` aus „Als Nächstes" oder dem Ticketanker: bei Ticketbeginn
`pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Anker -Ticket <NAK-nnn>`,
beim Abschluss `-Anker` ohne Ticket. Der Manifestkopf trägt „Etappe n von m"
und den laufenden Schritt als letzten Fettsatz (≤ 70 Zeichen).

`/dirigent stand`: `cockpit.ps1 -Plan` (im Ticket `-Plan -CurrentStep <ID>`);
die Zahlen müssen `docs/PLAN-STAND.md` entsprechen. Kein Ticketlauf beginnt
mit ungeprüfter nativer Fähigkeit (Ersatzwege: §4).

## 1. Rolle

- Keine Berechtigungsfragen (11.09., 19.09.2026): Dirigent `auto`, solange der
  Hook `tools/hooks/keine-rueckfrage.sh` verdrahtet ist (der Starter prüft es),
  sonst `dontAsk` wie jeder Worker. Klassifikator blockt → Worker fährt es;
  `dontAsk` blockt (`.claude/`) → Dirigent; gehäufte Ablehnungen → Neustart
  (§5).
- Der Dirigent entscheidet und misst, baut nie; bei laufendem Worker bleibt
  er an Repo-Dateien lesend.
- Technik innerhalb von Ticket, Invarianten und Designrichtung entscheidet
  Fable (Abweichungen im Manifest begründet); Produktwirkung, Bedienlogik,
  Priorität und Richtung entscheidet der User.
- Genau ein schreibender Worker. Kein eigener Prozessmanager, keine Zustands-
  oder Protokolldatei: Repo, Manifest und Planstand sind der Zustand.

Aufsichtsstufe je Phase; wichtige Ereignisse melden sich unabhängig davon:

| Aufsicht | Kontrolle | Einsatz |
|---|---:|---|
| `LOCKER` | 30 min | klein, lokal, leicht rücknehmbar |
| `NORMAL` | 15 min | normales Ticket, gekoppelte Dateien |
| `ENG` | 5 min | Audio, State, Vertrag, Nebenläufigkeit, Sicherheit, Gate, Nacharbeit |

Hochstufen sofort, herabstufen nach einer erfolgreichen Kontrolle. Beim
Wechsel alten Loop löschen, Fehlen prüfen, dann genau einen neuen setzen.

## 2. Quellen

1. `py -3.13 tools/plan/planstand.py`, dann `docs/PLAN-STAND.md` — „Als
   Nächstes" gewinnt. Blatt nur bei sauberem Quellstand mit Pathspec
   committen und pushen; kein Hook tut das.
2. Für das Ticket: Gate-Text, Fachquellen, Manifest `docs/beweise/<TICKET>.md`.
3. Offene User-Fragen: `docs/plan/fragen.json` über `/fragen`.

Widerspruch: `CLAUDE.md`, dann Manifest, dann jüngstes Datum. Kein
Selbstbericht einer Session ist Quelle.

## 3. Zyklus je Ticket

### 3.1 Vorher

```powershell
git status --short
claude agents --json --cwd . --all
git rev-parse HEAD
py -3.13 tools/plan/planstand.py
pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -Plan
```

Nur aktive Agenten (arbeitend, eingabebedürftig) blockieren. Fremde
Änderungen benennen, nie anfassen; fremder Schreiber → warten. Basis-SHA
merken (steht auch im Workernamen). Nur Gate, Fachquellen und Manifest des
Pakets lesen; alte Übergaben und Protokolle sind keine Arbeitsquelle.

**Liegengebliebener Ticketstand:** uncommitteter Stand ohne laufenden
Schreiber, den das Manifest als eigenen Bau ausweist, wird vor dem
Workerstart mit Pathspec committet (Betreff „<TICKET> Zwischenstand:
<Bauer>, nicht kompiliert, Tests NOT RUN"), im Manifest datiert
nachgetragen; der Worker baut darauf weiter. Ohne Manifestzuordnung bleibt
er fremd → Halt (§4).

### 3.2 Bauen

Worker = frischer Opus-Hintergrundprozess im sichtbaren Checkout; Auftrag als
**erstes Positionsargument** (`--allowed-tools` ist variadisch; dahinter wird
er Werkzeugeintrag, der Worker startet „idle"):

```powershell
claude "<selbsttragender Ticketauftrag>" --model opus --effort max --permission-mode dontAsk `
  --name "nakama-<ticket>-<basis-kurz>-bau" --allowed-tools <liste, keine Wildcard> --bg
```

Der Auftrag nennt Ticketgrenze, verbindliche Quellen, Manifestpfad,
Beweislauf, die Git-Regeln aus §6, `tools/dirigent/pruefliste.md` und den
Satz: beim Warten auf den Kanon einen abgelaufenen Wächter neu setzen und bei
jeder Rückkehr zuerst das Log lesen (NAK-354). Direkt nach dem
Start muss `claude agents --json` `working` zeigen. Kein eigenes
Konsolenfenster: `claude agents`, `claude logs`, `claude attach`.

**Spezifikation vor Code:** berührt das Ticket Nebenläufigkeit, Verträge,
Lebenszyklen oder Rückstau, schreibt der Worker zuerst eine Verhaltensmatrix
ins Manifest (Zustände × Ereignisse × Zusage, Callback-Reihenfolge, Fristen,
je Zeile der Test); ein lesender Codex-Thread (§3.4) prüft nur die Matrix
gegen Entwurf und Gate; erst dann wird gebaut. Jeder Matrix-, Bau- und
Nacharbeitsauftrag trägt wörtlich: „Je Matrixzeile fällt der Rotbeweis an
der Zeile, die die Zusage trägt, nicht an einem Nebeneffekt." Haben Befunde
eine gemeinsame Ursache, ist die Ursache der Auftrag, nicht die
Punktkorrektur.

Beobachter als Hintergrundkommando derselben Session, mit `Monitor`
(30 min, dann neu setzen):

```powershell
pwsh -NoProfile -File tools/dirigent/cockpit.ps1 -WatchWorker `
  -WorkerId <id> -BaseSha <sha> -Aufsicht <LOCKER|NORMAL|ENG> `
  -StartModel Opus -StartEffort max -DirigentSessionId <session-id>
```

Er meldet nur Zustandsänderungen, HEAD-/Worktree-Drift und alte oder
kritische Telemetrie und endet mit dem Worker. „Statusquelle unbekannt"
einmalig unter Last ist Fehlalarm; stirbt er durch Speichermangel eines
Builds, neu setzen. Dazu genau ein Cron-Job per `CronCreate` (nie `/loop`:
dessen Skilltext käme je Tick zurück und fragte nach Cloud); stündlich bei
scharfem Monitor, sonst im Aufsichtsintervall; Tick ohne Befund = ein Wort:

```text
Prüfe den laufenden Nakama-Worker <id> und seinen Ereignisbeobachter.
Gesund: Spurlage mit einem Wort bestätigen. Fertig, fehlgeschlagen, blockiert
oder Beobachter tot: Loop beenden, Mess-, Nacharbeits- oder Haltpfad fahren.
```

Bei Worker-Ende oder Halt: Beobachter beenden, `CronDelete`, mit `CronList`
belegen.

`needs input`: `claude logs <id>` (nur die letzten KB, Escape-Sequenzen
bereinigt). Der User wird nicht gefragt. Eine erwartete, nicht destruktive
Aktion gibt der Dirigent frei: Worker stoppen, Session-ID aus `claude agents
--json --all`, fortsetzen mit `claude "<Zusatz>" --resume <session-id> --bg`
ohne Modell-, Effort-, Modus-, Namens- und Werkzeugflags (mit ihnen startet
eine Kopie unter neuer Kennung: Beobachter und Cron umstellen, alte Session
`claude rm`; NAK-354); die `deny`-Regel bleibt, der Zusatz nennt den
zulässigen Ersatzweg. Ein Worker, der „ich warte auf die Benachrichtigung"
schreibt und steht, ist nicht blockiert, sondern ohne Wächter (NAK-354). Produktentscheid → Frage an den User, sofort anderes
Ticket. Destruktiv, ticketfremd, unerklärlich → stoppen, Ersatzworker mit
engerer Grenze.

### 3.3 Messen

Kein Selbstbericht zählt:

- Diff Basis-SHA..HEAD (`--stat`, dann relevante Hunks), Manifest, gezielte
  Tests, unberührte fremde Pfade.
- Messabdeckung: jede Matrixzeile mit Test und Rotbeweis ist gelaufen; fehlt
  sie, erzwingt ein Fortsetzungsauftrag den Messlauf vor der Erstprüfung.
- Rundenbilanz je Runde `py -3.13 tools/dirigent/rundenbilanz.py
  <vorher>..HEAD`, kumuliert `--runden <basis> <r1> <r2> …`; ihre Zeile
  steht in jedem Stand. Zwei Bau- oder Nacharbeitsrunden in Folge ohne
  Produkt- und Testzeilen → Konvergenzentscheid (§3.4); Matrixrunden zählen
  null.
- **Laufzeit-Arm** (User 12.09.2026, Register NAK-286, Plan S25e): nach
  grünem Kanon auf dem End-Stand `pwsh -NoProfile -File
  tools/fl/laufzeit.ps1 -Ticket <TICKET> -Basis <basis-sha> -Beenden`
  (`-Beenden` immer; 22.09.2026, NAK-358). Ob der Diff lohnt, entscheidet der
  Runner; er zieht die Manifest-Hashes nach, installiert, startet FL und fährt
  die Szenarien. Ein vorgefundenes FL ist Claudes: eigenes beendet der Runner,
  einen Rest ohne Besitzeintrag der Dirigent per `Stop-Process`; nie den User
  bitten. Exit 0 = gemessen oder begründet übersprungen (Diff ohne
  Produktpfad, fremdes FL); Exit 3 = Voraussetzung fehlt → Ursache beheben und
  wiederholen, nur die bekannte Karte U43 (Referenzprojekt) ist kein Befund;
  Exit 4 = Szenario verfehlt → Nacharbeit wie ein Codex-Defekt. Rohdaten
  `docs/beweise/roh/<TICKET>-laufzeit-<sha>.md`, Kopfzeile ins Manifest
  (`tools/fl/LIES-MICH.md`).

Beendet heißt: Baum sauber, Basis-SHA Vorfahr von HEAD, HEAD auf
`origin/master`. Fremde Commits → Halt. Eigene Reste oder nur lokaler
Commit → genau ein Fortsetzungs-Worker (`-fort`, gleicher Basis-SHA,
Auftrag: fertig committen und pushen, nie verwerfen); scheitert auch das →
Halt. Ab ~3 h oder ~30 Kompaktierungen wird ein Thread fehleranfällig: Rest in
enge Einzelaufträge. Unerwarteter Drift → Halt.

### 3.4 Prüfen und nacharbeiten (Codex)

Frischer, lesender Codex-Thread über den vollständigen Ticketbereich;
Pipelines in `pwsh` (5.1 schreibt `Tee-Object` als UTF-16), Temp unter
`$env:TEMP`:

```powershell
$baseSha = '<Stand vor dem Ticket>'; $headSha = git rev-parse HEAD
$pruefModell = 'gpt-6-astra'; $pruefEffort = 'max'   # nie 'ultra', nie Sol
$reviewJsonl = Join-Path $env:TEMP "nakama-$headSha-review.jsonl"
$reviewLast  = Join-Path $env:TEMP "nakama-$headSha-review-last.txt"
# $reviewPrompt nur aus tools/dirigent/pruefauftrag-vorlage.md (A: Erst-/
# Abschlussprüfung, B: Wiederprüfung), Platzhalter gefüllt: SHAs, Ticketpfade
# (nie `.`), Gate wörtlich, Matrix, Ausschlüsse. Freie Prompts sind unzulässig.
$reviewPrompt | codex -a never exec --ignore-user-config `
  -m $pruefModell -c "model_reasoning_effort=`"$pruefEffort`"" `
  -c 'windows.sandbox="elevated"' `
  -C . -s read-only review --json -o $reviewLast - | Tee-Object -FilePath $reviewJsonl
```

Eigener Prompt und `--base` schließen sich aus; die Sandbox-Zeile ist nötig,
weil `--ignore-user-config` sonst lesende Git-Prozesse blockiert. HEAD vor und
nach dem Lauf gleich, sonst ungültig. Thread-ID aus dem JSONL, fehlt
sie → `BLOCKED`. Urteil `PASS`, `NEEDS_WORK`, `BLOCKED` mit geprüft / nicht
geprüft; fehlt die Urteilszeile und der Kopf sagt „bleiben offen" →
`NEEDS_WORK`. Läufe über zehn Minuten nie als Session-Hintergrundbefehl,
sondern `tools/dirigent/codex-lauf.ps1 -Kennung <k> -Prompt <datei> -HeadSha
<sha>` mit Monitor auf der `-start.log` bis `EXIT=`.

**Modell:** jede Codex-Aufgabe `gpt-6-astra`, Effort `max` oder `xhigh`
(10.09.2026; ab 90 % Claude-Woche gilt §3.6), `ultra` nie (delegiert an
Unteragenten); `gpt-5.6-sol` max nur für sicherheitsnahe Prüfungen (Pipes,
Impersonation, Token, Signaturen; 15.09.2026, Astra reguliert dort hart).
Codex-CLI ≥ 0.153.4, sonst `codex update`. Kapazitätsabbruch: `resume` desselben Threads, dann neuer
Astra-Thread; wiederholte Kapazitäts-/API-Fehler → frischer Opus-Thread prüft
(nie der Bauer). Nacharbeit behält Modell und Effort; Wiederprüfung senkt nie
ab. Modell und Effort stehen bei Review-Beginn im Manifest.

**Befunde:** reproduzierbar und abnahmerelevant (critical: Daten-/State-
Verlust, Sicherheitsbruch, Audio-/Nulltest; high: Vertrag/Gate/Normalpfad;
medium: konkreter Funktionsfehler). Kosmetik, Stil, optionale Härtung,
theoretische Randfälle ohne Zusage, Ticketfremdes: nein. Jeden Befund
validiert ein lesender Opus-Agent an der Quelle (Zitat, Matrixzeile); die
Einordnung bleibt Dirigentensache:

- **Defekt** — verletzt Matrix, Gate, Entwurf oder `CLAUDE.md`-Invariante →
  Nacharbeit.
- **Lücke** — Matrix und Entwurf schweigen → Dirigent entscheidet die Regel
  (Technik) in derselben Runde, trägt sie in Matrix und Manifest ein; erst die
  Regel darf Nacharbeit auslösen.
- **Härtung** — von keiner Zusage verlangt → datiert ins Register.

Ein Lauf ohne Defekt ist `PASS`. Klassen, Ausschlüsse und der Sabotagesatz
stehen in der Vorlage; geprüft wird gegen den wörtlichen Gate-Satz, nicht
gegen die Paragraphennummer.

**Nacharbeit:** Der Auftrag enthält nur die bestätigten Defekte wörtlich, je
Defekt die schließende Regel und die Prüfliste — keine Dirigentenwünsche,
keine Deckel ohne Quelle. Codex stagt, committet und pusht nie und fährt
weder Bau noch Kanon; Defekte behebt ein frischer Opus-Worker. Nacharbeit
fährt nur betroffene Beine; voller Kanon beim ersten Bau und beim Abschluss.
Fable prüft den engen Fixdiff, committet nur diese Pfade, pusht; dann prüft
ein neuer Thread. Keine Freigabefrage an den User (User 29.08.2026).

**Prüfbereich schrumpft, wächst nie:** Erst- und Abschlussprüfung gehen über
`basis...HEAD`; jede Wiederprüfung sieht nur `stand-vor-der-runde...HEAD`,
die Befundliste und das Gate und urteilt nur: geschlossen, und nichts
gebrochen.

**Manifeste sind kein Prüfgegenstand:** `docs/**` gehört nicht zur
Befundfläche; Textinkonsistenzen zieht der Abschluss nach, nie als
`NEEDS_WORK`. Ein Manifest hat genau einen lebenden Kopf (Urteilsmarken,
Gate, Riegelkarte oder Matrix, Rundentabelle, jüngster Kanon); darunter
append-only.

**Rundenbudget:** drei Nacharbeitsrunden. Endet die dritte Wiederprüfung
ohne `PASS` → Konvergenzentscheid in derselben Sitzung: jeder offene Befund
wird am Gate-Wortlaut eingeordnet; Defekt nur, wenn Test, Matrix oder
Gate-Satz bricht, sonst Lücke/Härtung ins Register. Verbleibende Defekte
bekommen genau eine weitere Runde mit der Ursache als Auftrag (Wegwechsel:
Matrix als Spezifikation mit Tests, alle betroffenen Sprachen, frischer
Prüfer). Bleibt ein Defekt → eigener Registerpunkt mit Matrix, Ticket bleibt
`gebaut`, nächstes Ticket. Nie durch Wegdeklarieren abnehmen. Ein Ticket je
Worker; nie zwei Nacharbeiten gekoppelt.

### 3.5 Abschluss

Kanon auf dem End-Stand, abgekoppelt, nie als Session-Hintergrundbefehl;
währenddessen kommt nichts in den Worktree:

```powershell
$log = Join-Path $env:TEMP 'nakama-<ticket>-kanon.log'
$befehl = "pwsh -NoProfile -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/<TICKET>.md -Anhaengen -Titel <TICKET> *> $log; Add-Content $log ('EXIT=' + `$LASTEXITCODE)"
Start-Process pwsh -WindowStyle Hidden -WorkingDirectory (Get-Location) -ArgumentList '-NoProfile', '-Command', $befehl
```

Fertig, wenn die letzte Logzeile mit `EXIT=` beginnt; bis dahin höchstens
alle 15 Minuten `Get-Content $log -Tail 3`. Ein hängendes Bein beendet der
Runner nach 60 Minuten (Exit 124), einen Bauschritt nach 180; steht das Log
länger ohne Fortschritt bei 0 % CPU, zuerst die `vctip.exe`-Landmine
(CLAUDE.md), sonst Befund gegen den Runner. Rohausgaben unter
`docs/beweise/roh/<TICKET>-<sha7>[-dirty].md`, im Manifest nur die Kopfzeile. Nach `EXIT=0` folgt der Laufzeit-Arm (§3.3); die
dabei nachgezogenen Hashes in `eq-copilot/install/nakama-installer-v1.json`
gehören zu den Abschlussdateien. Beim Abschluss den lebenden Kopf nachziehen;
über ~3 000 Zeilen wandert der Rundenverlauf unverändert nach
`docs/beweise/<TICKET>-verlauf.md`, der jüngste Kanon-Abschnitt bleibt
(`planstand.py` liest ihn). Urteil, Modell, Effort, Basis- und End-SHA,
kumulierte Rundenbilanz und gelaufene Beweise ins Manifest; Planstand
rechnen; Plandokumente durch `py -3.13 tools/plan/dokuriegel.py <dateien>`
(Befund vor dem Commit beheben); nur Abschlussdateien mit Pathspec committen
und pushen.

**Hygiene in jedem Abschlussfenster** (auch nach Etappen und
Nacharbeitsrunden; 08.09., 12.09.2026), gemessen per Kommando, nie aus dem
Gedächtnis: `py -3.13 tools/plan/gesundheit.py` (Kontextfläche gegen
`docs/context-hygiene-playbook.md` und Codebase-Schwellen; Bein A32),
`dokuriegel.py` auf CLAUDE.md und Skill, `py -3.13 tools/plan/tidy.py`
(clang-tidy-Ratsche, Bein A33); Exit 4 = gerissen, beide nicht
blockierend. Ein Riss der
Kontextfläche (Bytes, Redundanz, Prosa, entbehrliche Zitate) wird im selben
Fenster behoben: Logik identisch, Kommandos exakt, Herkunft eines Entscheids
nur als Datum, Wortlaut bleibt in Abnahmen und Register. Ein Riss der
Codebase wird datierter Registerpunkt [Planarbeit · Pflegeschritt] oder
[Werkzeug], nie stilles Nachbessern. Bei Phasengates zusätzlich der volle
`/freshen`-Lauf nach dem Playbook.

Dann: temporäre Codex-Dateien und `$env:TEMP\nakama-dirigent-<session-id>-*.json`
löschen, `claude rm <worker-id>`, mit beendetem Beobachter, `CronList` und
`claude agents --json` belegen, dass nichts übrig ist. Weiter mit 3.1.

### 3.6 Bauer und Prüfer

Opus baut, Codex prüft (01.09.2026): Der Bauer ist ein frischer Opus-Worker
(`opus` = 5.5), Effort `max` (22.09.2026, NAK-359), kompiliert, fährt Tests
und Kanon selbst (§3.5, abgekoppelt) und übergibt nie `NOT RUN`. Der Prüfer
ist ein frischer Codex-Thread (§3.4). Codex an der Grenze → frischer
Opus-Thread prüft;
Opus-Prüfer und -Validierer immer `max`. Ab 90 % Claude-Woche (User
21.09.2026) kein Opus-Lauf mehr: auf den Codex-Reset warten, dann baut Codex
(`workspace-write`, `gpt-5.6-sol` xhigh) und prüft `gpt-6-astra` high, nie
max; Claude nur Dirigent. Den Codex-Stand committet der Dirigent nach
eigenem Kanonlauf als Zwischenstand (§3.1).

### 3.7 Phasengate-Prüfung

An jedem Gate G6–G9 auf sauberem, kanongrünem, gepushtem Stand fährt der
Dirigent als ersten Torschritt die Torläufe aus `docs/gesundheit/KONZEPT.md`
§6.6 als Prüfsessions (§7): Tiefenaudit, fällige Spezialaudits,
Mutanten-Vollmodus, voller `/freshen`-Lauf. Kein Worker auf diesem Stand, bis
ihre Köpfe gelesen sind. Ultra-Review und Astra-Audit sind kein Torschritt,
der User tut am Tor nichts (12.09.2026, U39). Ergebnisse und jeder externe
Audit des Users laufen wie NAK-246 (`docs/audits/`): validieren, einordnen,
Defekte als Ticket oder Nacharbeit.

## 4. Haltgründe

Ein Haltgrund stoppt nur das Ticket: Frage stellen (nur Design-/Produktfrage
oder User-Handgriff) und sofort das nächste Ticket ohne Haltgrund vorziehen;
erst ohne solches Ticket wartet die Sitzung.

- User- oder Figma-Handgriff (FL und Installation sind keiner, §3.3; einziger
  FL-Handgriff: Diagnoseprojekt nach NAK-30 neu anlegen),
- Produktentscheid,
- überlappende fremde Änderungen,
- Befund, der nur durch Produktentscheid oder User-Handgriff schließbar ist
  (technische Befunde sind nie Haltgrund),
- fehlende native Fähigkeit, deren Ersatz neue Infrastruktur bräuchte (ein
  CLI-Parse- oder Versionsfehler ist keine; erst Hilfe, Doku und kleinste
  Alternative mit gleichem Ziel, Sandbox, Modell, JSONL- und Thread-Vertrag),
- erschöpftes Kontingent oder wiederholte API-Fehler bei Fable, Opus oder
  Codex (Worker stoppen, Loop löschen, dann Halt),
- Kontextdruck (§5),
- Phasengate oder leerer Plan.

Vor jedem Halt: Worker gestoppt, Loop gelöscht, Stand im Manifest; der Halt
endet als wartende Frage oder Statusmeldung in der Sitzung. **Zeitfenster:**
Produktfragen und Handgriffe nur 9:00–23:00 PC-Lokalzeit (`Get-Date`; User
11.09.2026); außerhalb Frage fertig formulieren, One-Shot-Cron kurz nach
9:00, nächstes Ticket vorziehen.

## 5. Kontexthaushalt

- Bei **600k Kontext-Tokens** frische Session (30.08.2026); ab 500k kein neues
  Ticket, keine neue Prüfrunde. Im nächsten sauberen Abschlussfenster
  (kein Worker, kein Kanon, Urteil im Manifest, Planstand gepusht, Loop und
  Beobachter weg) Marker anlegen — `New-Item -ItemType File (Join-Path
  ([IO.Path]::GetTempPath()) 'nakama-dirigent-neustart.marker') -Force` — und
  die eigene PID beenden (`claude agents --json`, Zeile `nakama-dirigent`;
  `Stop-Process -Id <pid>`). Ohne Starter stattdessen `start-dirigent.ps1`
  abgekoppelt starten und die PID erst beenden, wenn `claude agents --json`
  die neue Session zeigt.
- Messung nur über die Statuszeile (`cockpit.ps1 -StatusLine`, Feld
  `context_window.used_percentage`), den Cache
  `$env:TEMP\nakama-dirigent-<session-id>-telemetry.json` (`ContextPercent`)
  oder die vom User genannte Zahl. `<total_tokens>` und Transkriptgröße sind
  keine Kontextmaße; ohne Messung heißt es „nicht gemessen". Nach einer
  Compaction bleibt der Anker unbestätigt, bis Planstand, Ticketquelle und
  HEAD neu abgeglichen sind.
- Kontingente: ab 85 % warnen, ab 95 % (Claude 90 %, §3.6) keine neue Arbeitsphase; laufender
  Abschluss nur ohne Qualitätsverlust. Ausnahme bei absehbarem Wochenreset
  (User 11.09.2026): Worker und Prüfrunden laufen weiter, ein Abbruch
  hinterlässt einen Ticketstand nach §3.1.
- Gezielt lesen: Diffs erst `--stat`; vom JSONL nur Thread-ID und Urteil; vom
  Worker-Log nur den Blockadegrund; Rohausgaben bleiben in Temp-Dateien.
- Keine Meldungen an den User (29.08.2026): keine Spurlage-, Runden- oder
  Abschlusstexte; Loop-Ticks und Beobachter-Ereignisse höchstens einzeilig;
  der einzige Text ist eine Design-/Produktfrage oder ein User-Handgriff.
- Nichts im Kopf führen, was im Repo steht; nach jedem Ticketabschluss ist
  der Kontext verzichtbar.
- Kontextdruck (Compaction gelaufen, Tragendes nur aus Zusammenfassungen):
  Ticket bis zur sauberen Grenze fahren, abräumen, Abschluss ins Manifest,
  frische Session; mit angeschlagenem Kontext beginnt kein neues Ticket.

Nach Absturz oder Neustart:

```powershell
claude --resume nakama-dirigent --model claude-fable-5-1[1m] --effort xhigh --permission-mode auto
claude --resume <session-id> --model claude-fable-5-1[1m] --effort xhigh --permission-mode auto
```

Die Sitzung beginnt mit `claude agents --json`, `CronList`, `git status` und
dem Vergleich Basis-SHA zu HEAD. Bekannter Worker ohne Beobachter oder Loop →
je einen neuen setzen; fehlender Worker → Beobachter beenden, Loop löschen.
Nie Zustand raten, nie eine Recovery-Datei bauen.

## 6. Was der Dirigent nie tut

- Selbst bauen oder bei laufendem Worker schreiben.
- Selbstbericht, Exit-Code oder Commit allein als fertig nehmen.
- Prüfen lassen, was derselbe Thread gebaut hat.
- `git add -A`, `--amend`, fremde uncommittete Dateien anfassen.
- `bypassPermissions` oder `ask`-Regeln nutzen, einen Moduswechsel hinnehmen.
- Ein zweites Protokoll, eine Statusdatei oder einen Ersatzkanal bauen.
- Ein Manifest zum Prüfgegenstand machen, eine Wiederprüfung über den ganzen
  Ticketbereich fahren, eine vierte Runde ohne Konvergenzentscheid starten.
- Einen Prüfauftrag frei formulieren, eine Runde ohne Rundenbilanz
  abschließen, einem Worker mehr auftragen als der Prüfer als Defekt erhob.
- Den Laufzeit-Arm auslassen oder den User nach FL, Installation oder
  Messung fragen.

## 7. Prüfsystem und Kanal

Die Prüfspur neben Kanon und Codex-Review steht in
`docs/gesundheit/KONZEPT.md` (Abnahme 12.09.2026, NAK-259) und bindet, sobald
ihr Aufbauschritt (§10 dort) gebaut ist: §3.1 (Prüfgang vor dem Ticket), §3.2
(Wächter im Auftrag), §3.4 (Kennung K1–K8 je Befund), §3.5 (Zweitbefund,
fällige Prüfsession) und den Pflegebetrieb. Der Dirigent auditiert nie selbst:
Audits laufen als Prüfsession zwischen zwei Tickets, er liest nur Kopf und
Befundliste.
Nachrichten anderer Sessions (`SendMessage`, Absender `nakama-*`) tragen
Zeiger und Entscheid, nie Inhalt; der Dirigent antwortet mit Entscheidung und
Ort im Repo, verarbeitet sie im Abschlussfenster, nimmt sie nie als Haltgrund
oder Anforderungsquelle für Worker und bittet keine Session um Geblocktes
(KONZEPT.md §5.1).
