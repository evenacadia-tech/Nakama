# Vorhaben 25.08.2026 — Dirigent ohne Nimbalyst

Status: **entscheidungsreif, noch nicht umgesetzt**. Dieser Text ist der
verbindliche Umsetzungs- und Abnahmeplan für den Umbau. Er ersetzt den ersten,
widersprüchlichen Entwurf vom selben Tag.

## 1. Ziel und bindende Entscheidungen

Der Dirigent bleibt eine **echte, interaktive Claude-Session mit Fable**. Er
urteilt, priorisiert, startet genau einen ausführenden Arbeiter, prüft dessen
Belege und entscheidet über den nächsten Schritt. Ein kleines PowerShell-Harness
übernimmt ausschließlich Prozessstart, Zustandsablage, Sperre und mechanische
Validierung. Es wird nicht zum Dirigenten und trifft keine Produktentscheidung.

Bindendes User-Wort vom 25.08.2026:

- *„beim dirigenten spart man nicht“* — Fable bleibt der Dirigent.
- *„eine session baut solange wie sie braucht. dafür ist fable da das im auge
  zu haben.“* — kein willkürlicher Bau-Timeout und kein Töten wegen bloßer
  Stille.
- Nimbalyst ist vollständig deinstalliert und soll keine aktive Abhängigkeit
  mehr sein.
- Matrix wurde nur wegen Nimbalysts fehlendem Remote-Zugang benutzt und ist bis
  auf Weiteres stillgelegt. Seine Daten werden erhalten, nicht vernichtet.
- Ein kleiner PowerShell-Harness ist gewünscht.
- Ein Codex-Prüfer darf seine bestätigten Befunde **in derselben Codex-Session**
  beheben; anschließend prüft eine **neue, frische Codex-Session** den gesamten
  Ticket-Diff erneut.
- Reviews suchen nur nach abnahmerelevanten Fehlern. Kosmetik, Stilpflege,
  theoretische Randfälle und Nadel-im-Heuhaufen-Suche sind kein Arbeitsauftrag.

Nicht Bestandteil dieses Vorhabens ist eine allgemeine
„Werkzeug-Entrümpelung“. Insbesondere bleibt `codebase-memory-mcp` bestehen; es
ist laut `AGENTS.md` der verbindliche erste Weg zur Code-Navigation.

## 2. Gemessener Ausgangszustand

Die folgenden Aussagen wurden am 25.08.2026 im aktuellen Workspace gemessen:

- Claude Code ist in Version `2.1.240` installiert. Der Start einer echten
  Session unterstützt `--model`, `--session-id`, `--name` und
  `--remote-control`.
- `claude -p` unterstützt unter anderem `--output-format json` und
  `--json-schema`. `claude agents --json --cwd <repo> --all` ist verfügbar,
  zeigt aber nur Claude-Agenten und ist keine atomare Prozesssperre.
- Codex CLI ist in Version `0.144.6` installiert. `codex review --json` ist
  ungültig. Der geeignete Weg ist `codex exec review`; dieser unterstützt
  JSONL, ein Ergebnisschema und eine gespeicherte Thread-ID. Mit
  `codex exec resume <thread-id>` kann genau dieser Kontext fortgesetzt werden.
- Die globale Codex-Konfiguration gewährt derzeit weitreichende Rechte. Der
  Harness darf sich darauf nicht verlassen, sondern setzt die Rechte pro
  Phase explizit.
- Die aktive Dirigenten- und Fragen-Dokumentation enthält noch
  Nimbalyst-/Matrix-Annahmen.
- Entgegen der beabsichtigten Stilllegung lief bei der Prüfung noch der lokale
  Matrix-Bridge-Prozess. Außerdem existiert weiterhin
  `Startup/Nakama Matrix Bridge.vbs`. Das Log war am 25.08.2026 noch verändert
  worden. Die Stilllegung ist deshalb ein eigener, zuerst auszuführender und
  zu beweisender Schritt.

Die alte Fassung verwirft damit folgende Annahmen ausdrücklich:

1. Ein Hintergrundprozess weckt Fable nicht nachweislich von selbst auf.
2. Remote Control ist der Bedienkanal derselben Claude-Session, kein
   Automations-Callback für Kindprozesse.
3. Exit-Code `0`, ein neuer Commit oder nichtleere Ausgabe bedeuten noch keinen
   fachlichen Erfolg.
4. `codex review --commit <head>` deckt weder mehrere Ticket-Commits noch den
   gewünschten strukturierten Ablauf ab.
5. Matrix-Ausgang und `nimbalyst.py` sind nicht unabhängig: Der Bridge-Dienst
   importiert die Datei und liest den zugehörigen Konfigurationsblock.

## 3. Zielarchitektur

### 3.1 Eine echte Dirigenten-Session

Der kanonische Einstieg wird:

```powershell
pwsh -File tools/dirigent/Start-Dirigent.ps1
```

Das Skript erzeugt eine UUID, legt den Laufzustand an und startet im selben
Terminal genau diese interaktive Session:

```powershell
claude --model fable --session-id <uuid> --name nakama-dirigent `
  --remote-control nakama-dirigent
```

Lokales Terminal und Claude Remote Control bedienen dadurch dieselbe Session.
Das Schließen des Terminals beendet diesen lokalen Betriebsmodus; der Plan
verspricht keine davon unabhängige Dauerinstanz.

### 3.2 Das Harness ist Mechanik, nicht Urteil

Vorgesehene Dateien:

```text
tools/dirigent/Start-Dirigent.ps1
tools/dirigent/Invoke-DirigentWorker.ps1
tools/dirigent/DirigentWorkerHost.ps1
tools/dirigent/schemas/bau-ergebnis.schema.json
tools/dirigent/schemas/review-ergebnis.schema.json
tools/dirigent/schemas/fix-ergebnis.schema.json
tools/dirigent/tests/DirigentHarness.Tests.ps1
```

Laufdaten und Logs gehören nicht in das Repository, sondern nach:

```text
%LOCALAPPDATA%\Nakama\dirigent\<repo-hash>\runs\<run-id>\
```

Pro kanonischem Repository besitzt genau **eine Dirigenten-Session** eine
atomare Owner-Lease. Ein zweiter lokaler Start wird bei lebendem Owner
abgewiesen und zeigt dessen Session-ID; Remote Control verbindet sich mit
demselben Owner. Unter der Lease darf höchstens **ein Worker beliebiger Rolle**
laufen — auch ein lesender Review braucht einen stabilen Repository-Stand.
Lease und Worker-Sperre speichern mindestens Rolle, Ticket, Run-ID,
Dirigenten-Session, Basis-HEAD, Supervisor-PID samt Startzeit und Executable
sowie Kind-PID samt Startzeit und Executable. PID allein genügt wegen
Wiederverwendung nicht.

Prozesse werden über .NET `ProcessStartInfo.ArgumentList` mit explizitem
Arbeitsverzeichnis gestartet. Prompts gehen über Datei oder stdin, nie über
zusammengesetzte Shell-Befehle. stdout, stderr, Startzeit, Endzeit und Exit-Code
werden getrennt aufgezeichnet.

Vor jedem Spawn wird zunächst atomar `RESERVED` persistiert. Nach Start des
Supervisors folgt `STARTING`; erst wenn auch die geprüfte Kindidentität sicher
gespeichert ist, wird der Zustand `RUNNING`. Stirbt eine Startphase, darf die
Sperre nicht freigegeben werden, solange ein möglicher Supervisor oder
Kindprozess nicht eindeutig ausgeschlossen ist.

Mechanische Zustände sind ausschließlich:

```text
RESERVED · STARTING · RUNNING · EXITED_0 · EXITED_NONZERO · ORPHANED · LOST · CANCELLED
```

Keiner dieser Zustände bedeutet `PASS`. Der Dirigent entscheidet erst nach
Repository-, Schema- und Beweisprüfung über Erfolg.

### 3.3 Beobachten ohne erfundenen Wakeup

Das Harness bietet kurze synchrone Wartescheiben von höchstens 60 Sekunden, die
sicher unter dem Timeout des aufrufenden Shell-Werkzeugs bleiben. Eine Scheibe
endet sofort, wenn der Worker endet; andernfalls kehrt sie mit `RUNNING` und
`nextCheckAt` zurück. Fable ruft bis dahin nur weitere Wartescheiben auf; die
teurere Prüfung von Prozess, Log und Repository erfolgt höchstens alle 30
Minuten. Dadurch kann eine Remote-Nachricht höchstens eine kurze Scheibe warten.

Bei Stille prüft Fable Prozessidentität, letzte strukturierte Ausgabe,
Logfortschritt, CPU-Aktivität und Repository-HEAD. Es beendet einen Worker nicht
allein wegen verstrichener Zeit. Ein autonomer Callback darf diese Lösung erst
ersetzen, wenn ein Ende-zu-Ende-Test beweist, dass er die laufende Fable-Session
ohne neuen User-Impuls tatsächlich fortsetzt.

`claude agents --json` darf ergänzend diagnostizieren, aber weder die Sperre
noch die Prozesswahrheit ersetzen.

## 4. Ablauf je Ticket

### 4.1 Vorbedingungen

Fable liest die laut `AGENTS.md`, `CLAUDE.md` und Planstand für das Ticket
verbindlichen Quellen. Danach hält der Harness fest:

- Ticket und unveränderten Gate-Text aus der kanonischen Planung,
- absoluten Repository-Pfad,
- vollständigen `base_sha = git rev-parse HEAD`,
- `git status --short`, einschließlich fremder Ausgangsänderungen,
- erwartetes Beweismanifest und passende Prüfkommandos.

Fremde Änderungen bleiben unangetastet. Der Harness bereinigt, verwirft,
stash-t oder committet sie nie.

### 4.2 Bauen — frische Claude-Worker-Session

Der Bauer ist pro Ticket eine frische, nichtinteraktive Claude-Session mit
Opus. Er erhält nur den Ticketauftrag, die verbindlichen Quellen, den
Ausgangsstatus und den erwarteten Beweisweg. Er arbeitet so lange, wie
Fortschritt oder ein ehrlicher Blocker nachweisbar ist, führt passende Tests
aus und committet ausschließlich seine eigenen Pfade mit explizitem Pathspec.

Sein schemageprüftes Ergebnis enthält mindestens:

- Ticket, Run-ID, `base_sha` und resultierendes `head_sha`,
- Status `completed`, `blocked` oder `failed`,
- geänderte Pfade und erzeugte Commits,
- ausgeführte Prüfungen mit Exit-Codes,
- Manifestpfad,
- verbleibende Risiken und nicht gelaufene Prüfungen.

Nach Prozessende validiert Fable unabhängig:

1. Ergebnisdatei und Schema sind gültig.
2. `base_sha` und `head_sha` sind vollständige SHAs und der Ausgangs-Commit
   ist Vorfahr des neuen HEAD. Nur `completed` verlangt einen neuen HEAD;
   `blocked` oder `failed` darf ohne Commit bei `head_sha == base_sha` enden.
3. Falls ein neuer HEAD existiert, gehört der gesamte Bereich
   `base_sha..head_sha`, nicht nur der letzte Commit, zum Ticket. Auch
   Teilcommits eines blockierten oder fehlgeschlagenen Laufs werden geprüft
   und bleiben offen; sie gehen nicht automatisch ins Review.
4. Ausgangsfremde Pfade wurden nicht verändert.
5. Manifest und aktuelle Testbelege existieren und stimmen mit dem Ergebnis
   überein.

Exit `0` ohne diese Nachweise ist nur `EXITED_0`, nicht fertig.

Unmittelbar vor und nach jeder Worker-Phase vergleicht der Harness HEAD und
Worktree mit dem letzten akzeptierten Snapshot. Insbesondere vor jeder
schreibenden Phase müssen `HEAD == erwarteter head_sha` und alle fremden Pfade
unverändert sein. Drift macht einen lesenden Lauf ungültig und führt vor einem
Fix oder Manifesteintrag zu `HALT`; der Harness schreibt nie auf einen
unerwarteten Stand.

### 4.3 Prüfen — frischer Codex-Thread, lesend

Für T2 bekommt eine neue Codex-Session genau den unveränderten Gate-Text, den
gesamten Diff `base_sha..head_sha` und das aktuelle Beweismanifest. Die
Zusammenfassung des Bauers ist kein Prüfmaßstab.

Der Harness startet aus dem Workspace-Root sinngemäß:

```powershell
codex -C <repo> -s read-only -a never --strict-config exec review `
  --json `
  --output-schema <review-ergebnis.schema.json> `
  -o <review-ergebnis.json> -
```

Der Prompt kommt über stdin und nennt die exakten vollständigen SHAs. Er weist
Codex an, `git diff --find-renames <base_sha>..<head_sha>` sowie bei Bedarf
`git show <head_sha>:<pfad>` zu prüfen. Ein eigener Prompt wird nicht mit den
konkurrierenden Zieloptionen `--base`, `--commit` oder `--uncommitted`
kombiniert.

Ein technisch gültiger Lauf verlangt gleichzeitig:

- Exit-Code `0`, parsebares JSONL und genau ein `thread.started.thread_id`,
- `turn.completed`, kein `turn.failed`,
- eine schemagültige Ergebnisdatei,
- dieselben `base_sha` und `head_sha` im Urteil,
- ein semantisch konsistentes Ergebnis.

stderr darf Fortschritt enthalten und ist allein kein Fehler. Der Status ist
`pass`, `findings` oder `blocked`; `pass` verlangt null Befunde, `findings`
mindestens einen Befund und `blocked` mindestens einen konkreten Blocker.

### 4.4 Befunde priorisieren und an der Quelle validieren

Ein Review darf nur folgende Prioritäten ausgeben:

| Priorität | Bedeutung |
|---|---|
| `critical` | Daten-/State-Verlust, Sicherheitsbruch, Audio-Thread-/Nulltest-Verletzung oder schwere reproduzierbare Korruption/Absturz |
| `high` | expliziter Vertrag, Gate oder normaler unterstützter Bedienpfad ist verletzt |
| `medium` | reproduzierbarer Funktionsfehler in einem unterstützten engeren Pfad, der die Ticketabnahme verhindert |

Jeder Befund braucht Pfad, Zeile, Auslöser, Auswirkung, Beleg und geforderte
Korrektur. Nicht zulässig sind reine Kosmetik, Benennungs- oder Formatwünsche,
optionale Härtung, theoretische/unwahrscheinliche Randfälle, vorbestehende oder
ticketfremde Punkte sowie pauschal „fehlende Tests“ ohne eine dadurch
unbewiesene konkrete Ticketbehauptung.

Fable prüft jeden Befund an der Quelle. Ein widerlegter oder nicht materieller
Befund löst keine Änderung aus. Ein bestätigter Befund blockiert `PASS`.

### 4.5 Beheben — derselbe Codex-Thread, gezielt schreibend

Nur bei bestätigten Befunden setzt der Harness exakt den gespeicherten
Review-Thread fort und wechselt für diese Phase ausdrücklich auf
`workspace-write`:

```powershell
codex -C <repo> -s workspace-write -a never --strict-config exec resume `
  --json `
  --output-schema <fix-ergebnis.schema.json> `
  -o <fix-ergebnis.json> <thread_id> -
```

Der Fixauftrag enthält nur die von Fable bestätigten Befund-IDs. Codex muss
jeden eigenen Befund nochmals reproduzieren, darf ihn als widerlegt
dokumentieren, ändert keine Kosmetik und eröffnet keine neue allgemeine Suche.
Es führt gezielte Prüfungen aus und committet nur seine eigenen Pfade mit
explizitem Pathspec. Als nun beteiligter Bauer darf dieser Thread sich selbst
kein abschließendes `PASS` erteilen.

Das Fixergebnis enthält Befunddispositionen samt Beleg, geänderte Pfade, Tests,
neuen HEAD und nicht behobene Blocker. Fable wiederholt danach alle
Repository- und Beweisprüfungen aus Abschnitt 4.2.

### 4.6 Erneut prüfen — zwingend frischer Codex-Thread

Nach einer Nacharbeit startet **ein neuer** `codex exec review`-Lauf. Seine
Thread-ID muss von der Fix-Session verschieden sein. Er prüft wieder den
vollständigen Bereich vom ursprünglichen `base_sha` bis zum aktuellen HEAD,
nicht nur den Fix-Commit.

Falls wieder materielle Befunde bleiben, darf genau dieser neue Prüfer seine
bestätigten Befunde in seinem Thread beheben; darauf folgt noch eine frische
Prüfung. Nach zwei erfolglosen Zyklen aus Nacharbeit und frischer Wiederprüfung
hält Fable an und meldet den Blocker. Eine dritte Reparaturschleife startet
nicht automatisch.

Ergibt bereits die erste unabhängige Prüfung `pass`, entfällt eine redundante
zweite Prüfung. Das exakte Urteil wird erst danach kontrolliert in das
Ticketmanifest übernommen. Soll Codex selbst diesen Eintrag schreiben, wird
der bestandene Review-Thread nur dafür mit `workspace-write` fortgesetzt; als
Postcondition darf ausschließlich das erwartete Manifest geändert und
committet worden sein.

### 4.7 Zustandsautomat

```text
IDLE
  -> BUILD_RUNNING
  -> BUILD_EXITED
  -> REPO_VALIDATED
  -> REVIEW_RUNNING (frischer Thread, read-only)
       -> PASS -> RECORD_VERDICT -> ACCEPTED
       -> FINDINGS -> FABLE_VALIDATES
            -> FIX_RUNNING (derselbe Review-Thread, workspace-write)
            -> REPO_VALIDATED
            -> REREVIEW_RUNNING (neuer frischer Thread, read-only)
            -> PASS oder nächster/letzter Fixzyklus
       -> BLOCKED -> HALT
```

Fable allein löst die fachlichen Übergänge `PASS`, `FINDINGS`, `ACCEPTED` und
`HALT` aus. Das Harness kann nur mechanische Unterzustände feststellen.

## 5. Nimbalyst entfernen und Matrix wirklich stilllegen

Dieser Umbau erhält historische Belege, entfernt aber jede aktive
Laufzeitabhängigkeit.

### 5.1 Matrix reversibel stilllegen

Als erster Umsetzungsschritt:

1. Prozess-Executable und Commandline prüfen, damit ausschließlich der echte
   Matrix-Bridge-Prozess adressiert wird.
2. Genau diesen Prozessbaum geordnet beenden.
3. `Nakama Matrix Bridge.vbs` aus dem Autostart **verschieben/deaktivieren**,
   nicht löschen.
4. Beweisen, dass kein Bridge-Prozess und keine zugehörige TCP-Verbindung mehr
   läuft und das Dienstlog während eines Beobachtungsfensters nicht wächst.
5. Den gesamten Bridge-Ordner mit Konfiguration, Store, Queue, Inbox, Outbox
   und `nimbalyst.py` unverändert erhalten. Matrix ist pausiert, nicht
   deinstalliert oder migriert.

Der Dirigent sendet keine Matrix-Meldungen mehr. Fortschritt, Rückfragen und
Abschluss laufen über dieselbe Remote-Control-/Terminal-Session.

### 5.2 Aktive Nimbalyst-Verweise bereinigen

Im selben Änderungssatz werden nur aktive Wahrheitsquellen korrigiert:

- `CLAUDE.md`: Remote-Bericht auf Terminal/Remote Control umstellen.
- `.claude/skills/dirigent/SKILL.md`: Nimbalyst-Start, Session-Suche, Wakeup
  und Matrix-Meldeweg durch den hier definierten Harness-Ablauf ersetzen; die
  fachliche Substanz, Priorisierung und Stop-Regeln bleiben erhalten.
- `.claude/skills/fragen/SKILL.md`: die drei entfernten Nimbalyst-Werkzeuge
  durch eine native Bild-/Dateiübergabe im echten Claude-Bedienkanal ersetzen.
  Wenn Remote Control eine benötigte Bilddarstellung nicht unterstützt, muss
  der Skill das ehrlich melden und darf keinen erfundenen Pfad behaupten.
- `docs/NEXT-SESSION.md`: aktive Matrix-/Nimbalyst-Aufträge entfernen oder als
  stillgelegt kennzeichnen und auf diesen Plan verweisen.
- `nimbalyst-local/automations/planstand-nakama.md`: entfernen; Nimbalyst ist
  deinstalliert. Danach den exakten ignorierten Laufartefakt-Ordner
  `nimbalyst-local/automations/planstand-nakama/` mit `history.json` und
  `output.md` gezielt prüfen und entfernen. Der weiterhin nützliche,
  Nimbalyst-freie `tools/hooks/planstand.sh` bleibt bestehen.
- `.gitignore`: erst nach der Artefaktbereinigung nur den dadurch verwaisten,
  eigens beschrifteten Nimbalyst-Automationsblock entfernen.

Nicht umgeschrieben werden historische Belege und Protokolle unter
`docs/beweise/`, `docs/dirigent/protokoll.md`, `docs/archiv/`, abgeschlossene
historische Einträge in `docs/offene-punkte.md` oder Designabnahmen. Ebenso
bleiben fachliche „Matrix“-Begriffe aus Mathematik, State oder DSP unangetastet.

## 6. Umsetzungspakete und Gates

### Paket A — Stilllegung und Wahrheitsabgleich

- Matrix-Prozess und Autostart reversibel stilllegen und beweisen.
- Aktive Nimbalyst-/Matrix-Verweise gemäß Abschnitt 5 korrigieren.
- Keine Historie und keine Bridge-Daten löschen.

**Gate A:** Nach Neustartprüfung startet die Bridge nicht; aktive
Dirigenten-/Fragen-Dokumentation verlangt kein Nimbalyst und kein Matrix.
`claude mcp list` enthält kein Nimbalyst und `git status --ignored` zeigt keine
Reste unter `nimbalyst-local/`.

### Paket B — Harness-Kern

- Laufverzeichnis, atomare Sperre, Prozessidentität und Zustandsdateien bauen.
- stdout/stderr und strukturierte Ergebnisse getrennt persistieren.
- Fremde Ausgangsänderungen erkennen und schützen.
- Crashzustände `ORPHANED` und `LOST` ehrlich behandeln; nie still einen
  zweiten schreibenden Worker starten.

**Gate B:** Pester-Tests beweisen Owner-Lease, atomaren
`RESERVED`/`STARTING`-Handshake, PID-Wiederverwendungs-Schutz,
Exit-Code-Zustände, Crash-Recovery und unveränderte Fremdpfade. Ein zweiter
Dirigentenstart sowie jeder parallele Worker — auch `read-only` — werden
abgewiesen.

### Paket C — Dirigentenstart und Beobachtung

- kanonischen Start derselben Fable-Session lokal und über Remote Control
  bauen,
- Wartescheiben und `nextCheckAt` integrieren,
- den bisherigen Dirigenten-Skill ohne fachliche Verarmung umstellen.

**Gate C:** Derselbe Session-ID-Kontext ist im Terminal und remote bedienbar;
ein Dummy-Worker endet ohne neuen User-Prompt innerhalb einer Wartescheibe und
ein weiterlaufender Worker gibt Fable rechtzeitig die Kontrolle zurück.

### Paket D — Bauer, Reviewer, Fixer

- Claude-Bauer mit Schema und vollständiger Repository-Nachprüfung integrieren.
- Codex-Review mit frischer Thread-ID und `read-only` integrieren.
- gezielte Nacharbeit via `exec resume` desselben Threads mit
  `workspace-write` integrieren.
- frische Vollbereichs-Wiederprüfung und Zwei-Zyklen-Stopp integrieren.

**Gate D:** Ein Wegwerf-Repository durchläuft den vollständigen Ablauf mit
mindestens zwei Ticket-Commits. Ein absichtlich materieller Fehler wird
gefunden, im selben Thread behoben und von einem neuen Thread über
`base_sha..fixed_head_sha` bestanden. Eine kosmetische Falle löst keinen
Befund aus. Simulierte HEAD- oder Worktree-Drift zwischen Review und Fix bzw.
Urteilseintrag führt vor dem Schreiben zu `HALT`.

### Paket E — Ende-zu-Ende-Abnahme

- Einen echten, kleinen Nakama-Auftrag ohne Produktinstallation durchführen.
- Aktuelles Manifest und gezielte Tests prüfen.
- Terminal/Remote-Control-Rückfrage sowie Abschlussbericht erproben.

**Gate E:** Der Ablauf benötigt weder Nimbalyst noch Matrix, bewahrt fremde
Änderungen, akzeptiert nur schemagültige aktuelle Belege und erzeugt genau den
beabsichtigten Ticket-Diff.

## 7. Pflichtprüfungen

Vor Freigabe müssen mindestens diese Szenarien nachweislich bestehen:

1. Ein Terminal und Remote Control zeigen dieselbe Fable-Session-ID.
2. Ein Dummy hält die atomare Sperre; ein zweiter Dirigent sowie ein paralleler
   schreibender oder lesender Worker werden abgewiesen, eine fremde
   ungetrackte Datei bleibt bytegleich.
3. Dummy-Prozesse mit Exit `0` und Exit `7` sowie getrenntem stdout/stderr
   ergeben korrekte mechanische Zustände und niemals automatisch `PASS`.
4. Eine verkürzte 30-Sekunden-Wakeup-Probe zeigt: Fable bekommt ohne
   User-Prompt Kontrolle zurück, plant die nächste Prüfung und erkennt ein
   früheres Worker-Ende.
5. Supervisor-Abbruch und verwaister Kindprozess ergeben `ORPHANED`/`LOST`
   und keinen parallelen Schreibstart.
6. Ein Git-Wegwerfrepo beweist den vollständigen Bau-/Review-/Fix-/Frischreview-
   Ablauf über mehrere Commits und exakte SHAs.
7. Zwei erfolglose Nacharbeitszyklen führen zuverlässig zu `HALT`.
8. Nimbalyst ist aus allen aktiven Pfaden entfernt, Matrix bleibt aus und alle
   Bridge-Daten bleiben erhalten.
9. `/fragen` zeigt ein Bild nativ im Remote-Control-Kanal oder weist die
   konkrete Einschränkung ehrlich aus.

Zusätzlich gelten die risikobasierten Nakama-Prüfungen aus `AGENTS.md` und dem
jeweiligen Ticket. Installation und FL-Studio-Handgriffe bleiben bewusste
User-Schritte und werden vom Harness nie automatisch ausgelöst.

## 8. Abbruch- und Fertigkriterien

Fable hält an und fragt den User, wenn:

- die Sperre nicht sicher einem lebenden oder toten Prozess zugeordnet werden
  kann,
- fremde Änderungen den Ticketbereich überlappen,
- Basis-/HEAD-Beziehung, Ergebnis-Schema oder Manifest widersprüchlich ist,
- ein benötigter User-/FL-Schritt erreicht ist,
- ein materieller Befund nach zwei Nacharbeitszyklen fortbesteht,
- der nächste Schritt neue Rechte oder einen größeren Umfang erfordern würde.

Das Vorhaben ist erst fertig, wenn alle Pakete A–E und ihre Belege grün sind,
die aktive Doku den gebauten Ablauf beschreibt und ein realer kleiner
Nakama-Durchlauf ohne Nimbalyst und Matrix akzeptiert wurde. Ein bloß grüner
Build, Exit-Code `0` oder ein Commit reicht ausdrücklich nicht.

Rollback bleibt je Paket möglich: Harness-Commits sind getrennt, der
Autostart-Eintrag wird nur verschoben, und Bridge-Daten werden nicht verändert.
