# Vorhaben 25.08.2026 — schlanker Dirigent ohne Nimbalyst

Status: **Planungsfassung 2, noch nicht umgesetzt**. Dieser Plan ersetzt die
technisch belastbare, aber zu komplex geratene Fassung vom selben Tag.

Arbeitsgrenze vom 26.08.2026: Dieses Dokument wird zuerst verbessert und
abgenommen. Bis der User die Umsetzung ausdrücklich startet, werden daraus
keine Prozesse gestartet oder beendet, keine Hooks umgestellt und keine
Altlasten entfernt.

Stand 26.08.2026, später am Tag: Der User hat Nimbalyst samt Matrix-Bridge
selbst vollständig deinstalliert — User-Wort: *„nimbalyst ist vollständig
deinstalliert, die alte fassung ist daher unnötig"*. Nachgemessen: der
Bridge-Ordner unter dem Benutzerprofil existiert nicht mehr, ein
Nimbalyst-MCP ist nicht registriert. Folgen: 6.1 ist faktisch erledigt; der
Skill-Umbau aus 6.2 (Dirigent und Fragen) wurde vorgezogen, weil beide Skills
sonst tote Werkzeuge aufriefen, ebenso der Matrix-Block in `CLAUDE.md`. Der
alte Weg steht **nicht mehr als Rückfall** zur Verfügung: scheitert Stufe C,
bleibt nur der `git revert` des Umstellungscommits, kein funktionierender
Altpfad. Alles Übrige (Hooks, `NEXT-SESSION.md`, Nimbalyst-Laufartefakte im
Repo) wartet weiter auf den ausdrücklichen Start der Umsetzung.

## 1. Leitentscheidung

User-Wort vom 25.08.2026:

> *„Ein komplexes System erfordert genauso viel Komplexität, es zu pflegen,
> und lenkt die Aufmerksamkeit weg vom eigentlichen Projekt. Der Drift ist
> vorprogrammiert. Ein schlankes System hält den Fokus auf dem Projekt.“*

Das Ziel ist deshalb nicht, Nimbalyst funktionsgleich nachzubauen. Das Ziel ist
ein begrenzter Nakama-Workspace, in dem die Aufmerksamkeit auf Ticket, Code und
Beweis liegt. Hilfswerkzeuge dürfen nur Mechanik abnehmen, die bereits vom
verwendeten Werkzeug getragen wird. Sie bekommen keine eigene Produktlogik,
keinen dauerhaften Zustand und keine zweite Wahrheit über das Projekt.

Der Dirigent bleibt eine **echte interaktive Claude-Session mit Fable**. Er
entscheidet. Ein frischer Claude-Agent baut. Codex prüft. Das Repository mit
seinen Manifesteinträgen und Tests ist der einzige dauerhafte Beleg.

## 2. Was die vorige Fassung übersehen hat

Die vorige Fassung entfernte Nimbalyst, plante aber an seiner Stelle bereits
wieder ein Orchestrierungssystem:

- drei PowerShell-Skripte,
- drei Ergebnisschemata,
- eigene Laufverzeichnisse und Logs,
- Owner-Lease, Worker-Sperre und acht Prozesszustände,
- Crash-Recovery, PID-Wiederverwendungslogik und eine eigene Testmatrix.

Das wäre technisch kontrollierbar, aber es hätte wieder einen zweiten
Pflegegegenstand neben Nakama geschaffen. Seine Zuverlässigkeit müsste
überwacht, seine Verträge müssten migriert und seine Dokumentation müsste mit
Claude, Codex und dem Repository synchron gehalten werden. Genau das ist der
Drift, den dieser Umbau beenden soll.

Zusätzlich fehlten neun Grenzen:

1. **Native Fähigkeiten zuerst.** Claude Code kann mit `claude --bg` einen
   direkten CLI-Prozess im Hintergrund starten. `claude agents` startet ihn
   nicht, sondern zeigt danach nur seinen Zustand. Dafür wird kein eigener
   WorkerHost benötigt. `--bg` und `-p` sind nicht kombinierbar.
2. **Ein nativer Wecker gehört hinein.** Fable setzt pro laufendem Worker mit
   `/loop 30m` eine sessiongebundene Kontrolle. Das nimmt Fable das eigene
   Erinnern ab, ohne einen selbstgebauten Scheduler, Dienst oder dauerhaften
   Zustand einzuführen. Nach Ende des Workers wird der Loop sofort beendet.
3. **Nur eine aktive Wahrheit.** `CLAUDE.md`, `NEXT-SESSION.md`, der
   Dirigenten-Skill und ein eigenes Dirigentenprotokoll dürfen den Stand nicht
   viermal erzählen. `NEXT-SESSION.md` ist bereits zur Chronik angewachsen und
   dupliziert den gerechneten Planstand; es wird deshalb nicht erneut „kurz
   gepflegt“, sondern aus dem aktiven Einstieg entfernt.
4. **Hilfsartefakte brauchen ein Ende.** Dieser Migrationsplan darf nach der
   Umsetzung nicht als weitere aktive Anleitung liegen bleiben.
5. **Aufmerksamkeitskosten sind ein Abnahmekriterium.** Nicht nur „funktioniert
   es?“, sondern auch „was muss ab morgen zusätzlich verstanden und gepflegt
   werden?“ entscheidet über die Annahme einer Lösung.
6. **Automatische Hooks sind ebenfalls ein System.** Heute laufen bei jedem
   Sessionstart Depth-, Plan- und Design-Primer; nach Werkzeugaufrufen folgen
   Erinnerer, Planstand-Commit und Auto-Push, beim Stop ein weiterer Wächter.
   Auch wenn jeder einzeln gut begründet ist, formen sie zusammen wieder eine
   unsichtbare Orchestrierung und laden Kontext unabhängig vom Auftrag.
7. **Hintergrundagenten schreiben standardmäßig nicht in diesen Checkout.**
   Claude Code verschiebt jeden mit `--bg` gestarteten Schreiber vor dem ersten
   Edit in ein eigenes Worktree. Nakama braucht für den geteilten Build und den
   unmittelbar sichtbaren HEAD ausdrücklich die native Einstellung
   `worktree.bgIsolation = "none"`; sonst misst der Dirigent den falschen Baum.
8. **Der Werkzeugzustand darf nicht mit eigenem Systemzustand verwechselt
   werden.** Claudes eigener, bei Bedarf gestarteter Supervisor und seine
   Sessiondaten unter dem Benutzerprofil sind zulässig. Nakama baut darauf
   weder ein zweites Protokoll noch eigene Recovery-Dateien.
9. **Die konkreten CLI-Aufrufe müssen vor dem Umbau parsebar sein.** Die lokal
   installierte Codex-CLI akzeptiert `model_reasoning_effort` nur bis `xhigh`
   und kennt `--strict-config` nicht. Ein Plan mit `max` oder diesem Schalter
   würde erst nach dem Rückbau des alten Wegs scheitern.

## 3. Feste Komplexitätsgrenze

Für diesen Umbau gelten folgende Grenzen:

- kein eigener Daemon, Scheduler, Watchdog oder Autostart für den Dirigenten;
  der native sessiongebundene `/loop` ist ausdrücklich erlaubt,
- kein eigener Prozessmanager, Zustandsautomat oder Lockdienst,
- keine neuen JSON-Schemata für Bauer-, Review- oder Fixergebnisse,
- keine dauerhaften Laufprotokolle oder Runtime-Dateien im Repository,
- kein neues Dashboard und keine externe Statusdatenbank,
- keine Kopie von Ticketstand oder Produktwahrheit in Hilfsdokumenten,
- keine informierenden oder automatisch commit-/pushenden Dauer-Hooks,
- kein Abschalten des alten Wegs, bevor der neue Weg Ende-zu-Ende belegt ist
  — seit 26.08.2026 gegenstandslos: der User hat den alten Weg selbst
  deinstalliert (Stand-Notiz oben),
- während eines Dirigentenlaufs genau ein schreibender Nakama-Worker; andere
  Sessions bleiben lesend oder der Dirigent hält,
- kein allgemeines Aufräumen funktionierender Werkzeuge nebenbei.

Vom Werkzeug selbst verwaltete Session- und Supervisor-Daten außerhalb des
Repositories zählen nicht als neues Nakama-System. Sie werden nur über die
offiziellen Claude-Befehle gelesen, fortgesetzt, gestoppt oder entfernt.

Standardfall: **keine neue dauerhafte Harness-Datei**. Falls sich beim realen
Probelauf zeigt, dass der direkte Startbefehl wiederholt falsch eingegeben
wird, ist höchstens ein dünnes `Start-Dirigent.ps1` erlaubt. Es darf nur den
unten stehenden Claude-Aufruf weiterreichen — ohne Zustand, Monitoring,
Prozessverwaltung oder eigene Regeln.

Reichen die nativen Fähigkeiten nachweislich nicht aus, hält die Umsetzung an
und legt dem User genau die fehlende Fähigkeit vor. Sie erweitert den Umfang
nicht still um ein Ersatz-Nimbalyst.

## 4. Begrenzter aktiver Workspace

Eine frische Nakama-Arbeit braucht nur diesen Einstieg:

1. `AGENTS.md` führt zu `CLAUDE.md`; dort stehen Produkt und harte Invarianten.
2. `tools/plan/planstand.py` rechnet bei Arbeitsbeginn `docs/PLAN-STAND.md`
   neu; das Blatt nennt dann den nächsten Schritt. Offene User-Fragen stehen
   in `docs/plan/fragen.json`.
3. Für das konkrete Ticket werden nur dessen Gate-Text, betroffene Fachquellen
   und Beweismanifest gelesen.

Git und das Ticketmanifest belegen, was tatsächlich geschehen ist. Historische
Erklärungen werden nicht in aktive Einstiegsdateien kopiert; die Git-Historie
und bestehende Archive reichen zur Spurensuche.

Der Dirigent führt kein zweites fortlaufendes Projektprotokoll. Das bestehende
`docs/dirigent/protokoll.md` bleibt historisch lesbar, wird aber nicht mehr als
Pflichtartefakt fortgeschrieben. Seine verwertbare Wahrheit steht bereits in
Commits, Manifesten und Planstand.

## 5. Minimaler Ablauf

### 5.0 Erst beweisen, dann umstellen

Die Umstellung beginnt mit einem nicht schreibenden Werkzeug-Preflight. Er
prüft im sauberen Nakama-Checkout:

```powershell
claude --version
codex --version
claude agents --json --cwd . --all
git status --short
git rev-parse HEAD
```

Danach werden die vorgesehenen Startbefehle jeweils mit `--help` bis zum
Unterbefehl geparst. Der Preflight muss außerdem belegen, dass

- Fable für das Konto tatsächlich startbar ist,
- Terminal und Remote Control dieselbe Sitzung zeigen,
- ein einmaliger `/loop 1m` die idle Fable-Sitzung weckt und anschließend
  vollständig gelöscht werden kann,
- ein nicht schreibender Opus-Hintergrundagent in `claude agents --json`
  sichtbar, stoppbar und wieder entfernbar ist,
- Opus mit `--permission-mode auto` tatsächlich im Auto-Modus startet und
  einen ausdrücklich erlaubten harmlosen Prüfkommando-Pfad ohne Rückfrage
  beendet; ein stiller Rückfall auf Manual zählt als Fehlschlag,
- der Wiedereinstieg real funktioniert: `--resume` mit einem Nicht-ID-Wert
  öffnet nur den interaktiven Picker mit diesem Suchbegriff; deterministisch
  fortgesetzt wird allein über die Session-ID. Der Preflight belegt beide Wege,
- eine wartende Frage der idle Fable-Sitzung auf der Remote-Oberfläche
  sichtbar wird. Ohne diesen Rückweg ersetzt der neue Weg die abgeschaffte
  Matrix-Meldung bei blockenden Entscheidungen nicht,
- der Codex-Review- und Resume-Aufruf die gewählten Optionen akzeptiert.

Dieser Preflight verändert keine Produktdatei. Scheitert eine Fähigkeit,
bleiben Matrix, Nimbalyst, Hooks und Einstiegsdateien unangetastet. Der Plan
hält mit genau dieser fehlenden Fähigkeit an.

### 5.1 Rollen, Modelle und Effort

Modell und Effort werden pro Rolle ausdrücklich gesetzt und nie aus einer
globalen Default-Konfiguration abgeleitet:

| Rolle | Modell | Effort | Grund |
|---|---|---|---|
| Dirigent | Claude Fable | `xhigh` | hält den Gesamtfaden, priorisiert und entscheidet; `xhigh` genügt für diese begrenzte Orchestrierung |
| Bauer | Claude Opus | `max` | baut genau ein Ticket samt Beweis; Implementierungsqualität geht vor Laufzeit |
| alle Codex-Instanzen | `gpt-5.6-sol` | durch Fable: `high` oder `xhigh` | Fable setzt die höchste von der aktuellen CLI belegte Denktiefe passend zum materiellen Risiko des konkreten Tickets |

Fable trifft die Sol-Auswahl ohne weitere Datei oder Punktesystem:

- `high` nur bei einer kleinen, lokal begrenzten Änderung mit geringer
  Auswirkung und eindeutiger Abnahme,
- `xhigh` als Standard für normale Implementierungstickets, mehrere gekoppelte
  Dateien oder einen nicht trivialen Befund,
- ebenfalls `xhigh` für qualitätskritische Arbeit an Audio-Thread,
  State/Migration, IPC/Verträgen, Nebenläufigkeit, Sicherheit oder einem
  Phasengate, weil die aktuelle Codex-Konfiguration oberhalb davon keinen
  gültigen Wert anbietet.

Die Auswahl wird bei Beginn eines frischen Review-Threads einmal getroffen und
im vorhandenen Ticketmanifest zusammen mit Modell und Urteil vermerkt. Eine
Nacharbeit im selben Thread behält dieses Effort. Die anschließende frische
Wiederprüfung verwendet mindestens dasselbe Effort; Fable darf es erhöhen,
aber nach einem bestätigten Fehler nicht absenken. Globale Codex-Defaults sind
damit unerheblich.

Reasoning-Tiefe und Prüfbreite bleiben zwei verschiedene Dinge. Mehr Effort
darf einen materiellen Befund gründlicher validieren, aber niemals den engen
Ticket- und Prioritätsprompt ausweiten. Kosmetik, Randfallgrabung und
Nadel-im-Heuhaufen-Suche bleiben bei jeder Stufe ausgeschlossen.

### 5.2 Dirigent starten

Der User startet direkt eine echte Fable-Session:

```powershell
claude --model fable --effort xhigh --name nakama-dirigent --remote-control
```

Terminal und Remote Control bedienen dieselbe Session. Es gibt keinen
zusätzlichen Nachrichtenkanal. Der von Claude selbst bei Bedarf gestartete
Hintergrund-Supervisor ist Werkzeugmechanik, kein Nakama-Dienst.

Wird nur die Remote-Verbindung getrennt, läuft die lokale Sitzung weiter. Wird
der Terminalprozess beendet, feuern keine `/loop`-Aufgaben, bis genau diese
Sitzung fortgesetzt wird. `--resume` mit dem Sitzungsnamen ist dabei **keine**
deterministische Fortsetzung: ein Nicht-ID-Wert öffnet nur den interaktiven
Picker, vorgefiltert auf den Suchbegriff. Der Name `nakama-dirigent` macht die
Sitzung dort sofort auffindbar; ohne Klick geht es nur über die Session-ID:

```powershell
claude --resume nakama-dirigent   # Picker, vorgefiltert — ein Klick des Users
claude --resume <session-id> --model fable --effort xhigh --remote-control
```

Beim bewussten Beenden stoppt Fable zuerst einen laufenden Worker und löscht
seinen Kontrollloop. Nach Absturz oder Neustart beginnt die fortgesetzte
Sitzung mit `claude agents --json`, `CronList`, `git status` und dem Vergleich
von Ausgangs-SHA zu aktuellem HEAD. Läuft der bekannte Worker noch und fehlt
sein Loop, wird genau ein neuer Kontrollloop gesetzt. Fehlt der Worker, wird
ein verbliebener Loop gelöscht. Die Sitzung rät keinen Zustand und baut keine
eigene Recovery-Datei.

### 5.3 Ticket festlegen

Fable rechnet den Planstand einmal zu Arbeitsbeginn und liest ihn:

```powershell
py -3.13 tools/plan/planstand.py
```

Ändert der Generator das Blatt auf Basis bereits committeter Quellen, wird nur
`docs/PLAN-STAND.md` vor dem Ticket mit explizitem Pathspec committet und
gepusht. Bei uncommittierten Planquellen hält Fable stattdessen an. Danach liest
es die Quellen des genau einen Tickets. Vor jedem Arbeiter prüft Fable:

```powershell
git status --short
claude agents --json --cwd . --all
git rev-parse HEAD
```

Bei `claude agents --json` zählen nur aktive Zustände wie arbeitend oder
eingabebedürftig; alte abgeschlossene Zeilen sind kein Blocker. Läuft bereits
ein verändernder Agent oder Bau im selben Workspace, wartet Fable. Da Claude
nicht jede fremde interaktive Anwendung als Agent registriert, gilt für den
Dirigentenlauf zusätzlich der sichtbare Betriebsmodus: kein anderer
schreibender Nakama-Task parallel. Fremde Änderungen werden benannt und nicht
angefasst. Der vollständige Ausgangs-SHA bleibt im Dirigentenkontext und im
Ticketauftrag; dafür braucht es keine eigene Zustandsdatei.

### 5.4 Bauen

Claude-Hintergrundagenten wechseln vor dem ersten Edit standardmäßig in ein
eigenes Worktree. Für Nakama wird deshalb in der Projektkonfiguration die
native Ausnahme gesetzt:

```json
{
  "worktree": {
    "bgIsolation": "none"
  }
}
```

Das ist nur zusammen mit der Ein-Schreiber-Regel zulässig. Der Preflight und
der reale Probelauf müssen belegen, dass kein neues `.claude/worktrees/`
entsteht und der Worker tatsächlich den sichtbaren HEAD dieses Checkouts
ändert. Die vorhandenen engen `allow`- und `ask`-Regeln in
`.claude/settings.json` bleiben dabei erhalten.

Der Bauer ist ein direkt gestarteter, frischer Opus-CLI-Prozess im
Hintergrund:

```powershell
claude --model opus --effort max --permission-mode auto `
  --name "nakama-<ticket>-<basis-kurz>-bau" `
  --bg "<selbsttragender Ticketauftrag>"
```

Der Auftrag enthält nur Ticketgrenze, verbindliche Quellen, Manifestpfad,
Beweislauf und die Git-Regeln. Zusätzlich nötige, nicht destruktive
Ticketkommandos werden beim Start eng über `--allowed-tools` freigegeben, nicht
als globale Wildcard. Ein eigenes Konsolenfenster bekommt der Worker nicht:
`claude agents` zeigt seinen Zustand, `claude logs` und `claude attach` bei
Bedarf den Verlauf — dieselbe Sichtbarkeit ohne zweiten Fensterhaushalt. `auto` ist erforderlich, weil `acceptEdits` zwar Dateien,
aber nicht jeden Test- oder Buildbefehl ohne Rückfrage erlaubt;
`bypassPermissions` bleibt verboten. Ist Auto für Konto und Opus nicht
verfügbar, greift die Haltregel aus 5.0 statt eines stillen Moduswechsels.

`claude agents` wird nicht zum Starten oder Delegieren benutzt; es liest
ausschließlich den von der CLI registrierten Prozesszustand. Es gibt genau
einen verändernden Arbeiter zur Zeit als einfache Arbeitsregel, nicht als
eigenes Locksystem. Solange der Worker läuft, bleibt Fable selbst bei
Repo-Dateien strikt lesend. Direkt nach dem Start aktiviert Fable den nativen
Kontrollloop:

```text
/loop 30m Prüfe den laufenden Nakama-Worker über `claude agents --json --cwd . --all`.
Wenn er arbeitet, prüfe nur auf Blockade oder erkennbaren Stillstand und warte weiter.
Wenn er fertig, fehlgeschlagen oder blockiert ist, beende diesen Loop und führe den
passenden Mess-, Nacharbeits- oder Haltpfad des Dirigenten aus.
```

Der Loop ist das regelmäßige Sicherheitsnetz. Eine Fertigmeldung des Workers
weckt die unabhängige Fable-Sitzung nicht verlässlich; autonomes
Wiederaufwachen kommt deshalb ausschließlich über den Loop. Der User darf
jederzeit früher eine Kontrolle anstoßen. Beim Anlegen merkt sich Fable die von
Claude gemeldete Task-ID nur im Sitzungskontext. Bei Worker-Ende oder Halt
löscht es genau diese Aufgabe über `CronDelete` und prüft über `CronList`, dass
sie nicht mehr existiert. Der Loop erzeugt keine Projektdatei. Läuft Fable beim
Fälligkeitszeitpunkt noch in einem Turn, wird die Kontrolle direkt danach
ausgeführt statt parallel in den laufenden Turn einzugreifen. Das Intervall ist
ein Kontrollfenster, keine exakte Deadline.

Meldet Agent View trotz Auto `needs input`, liest Fable zuerst
`claude logs <worker-id>`. Eine erwartete, nicht destruktive Ticketaktion darf
der User auf Fables konkrete Empfehlung einmalig im Peek-Panel freigeben. Ein
Produktentscheid führt zum User-Halt; eine destruktive, ticketfremde oder nicht
erklärbare Aktion wird nicht freigegeben und der Worker wird gestoppt.

Fable übernimmt keinen Selbstbericht. Nach Ende misst es direkt:

- den vollständigen Diff vom Ausgangs-SHA bis zum aktuellen HEAD,
- das aktuelle Ticketmanifest,
- die ausgeführten gezielten Tests,
- unberührte fremde Pfade.

Ein Exit-Code oder Commit allein bedeutet nicht fertig. Ein blockierter Agent
darf ehrlich ohne Änderung enden. Unerwarteter HEAD- oder Worktree-Drift führt
zum Halt; Fable baut dafür keine automatische Reparatur.

Ein verändernder Worker gilt außerdem erst als beendet, wenn der Arbeitsbaum
sauber ist, der Ausgangs-SHA Vorfahr des gemessenen HEAD ist und genau dieser
HEAD nachweislich auf `origin/master` liegt. Fremde Commits führen vor dem
Review zum Halt. Hinterlässt der eigene Worker uncommittierte Reste oder einen
nur lokalen Commit, darf genau **ein** frischer Fortsetzungs-Worker (Name mit
Suffix `-fort`, derselbe Basis-SHA, enger Abschlussauftrag) den Stand fertig
committen und pushen; verwerfen darf er nichts, und eine zweite Fortsetzung
gibt es nicht. Gelingt auch das nicht, gilt der Halt.

### 5.5 Mit Codex prüfen und gezielt nacharbeiten

Ein frischer Codex-Thread prüft lesend den vollständigen Ticketbereich, den
unveränderten Gate-Text und das Manifest. Temporäre CLI-Ausgaben liegen nur
unter `$env:TEMP` und werden nach dem Ticket entfernt. Die Pipelines laufen in
`pwsh` (PowerShell 7): Windows PowerShell 5.1 schreibt `Tee-Object` ohne
Encoding-Wahl als UTF-16 und macht die JSONL-Datei für die Weiterverarbeitung
unbrauchbar.

```powershell
$baseSha = '<Stand vor dem Ticket>'
$headSha = git rev-parse HEAD
$solEffort = 'xhigh' # Fable setzt nach 5.1 high oder xhigh
$reviewJsonl = Join-Path $env:TEMP "nakama-$headSha-review.jsonl"
$reviewLast = Join-Path $env:TEMP "nakama-$headSha-review-last.txt"

$reviewPrompt | codex -a never exec --ignore-user-config `
  -m gpt-5.6-sol -c "model_reasoning_effort=`"$solEffort`"" `
  -C . -s read-only review --base $baseSha --json -o $reviewLast - |
  Tee-Object -FilePath $reviewJsonl
```

`--ignore-user-config` verhindert, dass ein später geänderter globaler Default
den Rollenvertrag bricht; Authentifizierung und Repo-Anweisungen bleiben
erhalten. `--strict-config` wird nicht verwendet, weil die installierte
Codex-CLI diesen Schalter nicht besitzt. Der Prompt nennt die exakten Basis-
und HEAD-SHAs, `--base` bindet den Review zusätzlich an den Ausgangsstand. Vor
und nach dem Lauf muss HEAD weiter `$headSha` sein; sonst ist das Urteil
ungültig. Der JSONL-Stream wird nur temporär gehalten und liefert die Thread-ID.
Fehlt sie, gilt der Lauf als `BLOCKED`; ein eigenes Ergebnisschema wird nicht
gepflegt. Das Urteil ist `PASS`, `NEEDS_WORK` oder `BLOCKED` und nennt knapp,
was tatsächlich geprüft und nicht geprüft wurde.

Ein zulässiger Befund muss reproduzierbar sein und die Ticketabnahme berühren:

- `critical`: Daten-/State-Verlust, Sicherheitsbruch,
  Audio-Thread-/Nulltest-Verletzung oder schwere Korruption,
- `high`: expliziter Vertrag, Gate oder normaler unterstützter Pfad gebrochen,
- `medium`: konkreter Funktionsfehler in einem unterstützten engeren Pfad, der
  die Abnahme verhindert.

Kosmetik, Stil, Benennung, optionale Härtung, theoretische Randfälle,
vorgefundene ticketfremde Probleme und Nadel-im-Heuhaufen-Suche werden nicht
weiterverfolgt. Fable validiert jeden Befund an der Quelle.

Bestätigte Befunde behebt derselbe Codex-Thread gezielt:

```powershell
$fixJsonl = Join-Path $env:TEMP "nakama-$headSha-fix.jsonl"
$fixLast = Join-Path $env:TEMP "nakama-$headSha-fix-last.txt"

$fixPrompt | codex -a never exec --ignore-user-config `
  -m gpt-5.6-sol -c "model_reasoning_effort=`"$solEffort`"" `
  -C . -s workspace-write resume <thread-id> --json -o $fixLast - |
  Tee-Object -FilePath $fixJsonl
```

Der Fixerauftrag verbietet Stage, Commit und Push. Nach seinem Ende prüft Fable
den engen Fixdiff und die betroffenen Tests, committet ausschließlich diese
Pfade und pusht den neuen HEAD. Währenddessen läuft kein weiterer Schreiber.

Danach prüft ein neuer frischer Codex-Thread erneut den **gesamten** Bereich
vom ursprünglichen Ausgangs-SHA bis zum neuen HEAD. Nach zwei erfolglosen
Nacharbeitsrunden hält Fable an. Ein bestandenes Urteil wird unverändert in das
vorhandene Ticketmanifest übernommen; dafür entsteht kein weiteres Protokoll.
Die frische Wiederprüfung verwendet eine neue Thread-ID und einen erneut
expliziten Modell-/Effort-Aufruf. Sie behält `$solEffort` oder erhöht es nach
den Regeln aus 5.1.

### 5.6 Weiter oder halten

Fable fährt mit dem nächsten Ticket fort. Es hält nur bei:

- einem User-, Figma-, FL- oder Installationsschritt,
- einem Produktentscheid,
- überlappenden fremden Änderungen,
- einem materiellen, zweimal nicht geschlossenen Befund,
- einer fehlenden nativen Fähigkeit, deren Ersatz neue Infrastruktur
  erfordern würde,
- erschöpftem Kontingent oder wiederholten API-Fehlern bei Fable, Opus oder
  Codex — Worker stoppen, Loop löschen, dann Halt statt blindem Wiederholen,
- Kontextdruck der Dirigentensitzung nach 5.7,
- einem Phasengate oder leerem Plan.

Jeder Halt endet als klare, wartende Frage oder Statusmeldung in der Sitzung
selbst — das ist der Weg, auf dem er den User über die Remote-Oberfläche
erreicht. Einen zweiten Meldekanal gibt es nicht.

Vor dem nächsten Ticket schreibt Fable Urteil, Modell, Effort, Basis- und
End-SHA sowie die tatsächlich gelaufenen Beweise in das vorhandene Manifest,
rechnet den Planstand neu und committet/pusht nur diese Abschlussdateien. Dann
löscht es alle für dieses Ticket erzeugten temporären Codex-Dateien, entfernt
die beendete Worker-Session mit `claude rm <worker-id>` und belegt mit
`CronList` und `claude agents --json`, dass weder Kontrollloop noch aktiver
Worker übrig sind.

### 5.7 Kontexthaushalt des Dirigenten

Der Dirigentenkontext ist das Einzige, was zwischen den Tickets lebt — und er
ist endlich. Der Lauf endet planmäßig nicht an einem Fehler, sondern an dieser
Grenze. Zwei Regeln halten den Kontext klein, eine macht ihn ersetzbar:

- Fable liest groß Gewachsenes gezielt statt vollständig: Diffs zuerst als
  `--stat`, dann nur die relevanten Hunks; vom Codex-JSONL nur Thread-ID und
  Schlussurteil; vom Worker-Log nur den Blockadegrund. Rohausgaben bleiben in
  ihren Temp-Dateien. Ein Loop-Tick ohne Befund bleibt ein Einzeiler.
- Reihenfolge und Zwischenstände werden nicht im Kopf mitgeführt: Basis-SHA
  steckt im Workernamen, Urteil und Beweise im Manifest, der nächste Schritt
  im gerechneten Planstand.
- Nach jedem Ticketabschluss (5.6) steht deshalb alles Tragende im Repo.
  Ab dieser Grenze ist der Dirigentenkontext verzichtbar; eine frische
  Dirigenten-Session kann ohne Übergabetext übernehmen.

Meldet die Sitzung Kontextdruck — eine Compaction ist gelaufen oder Fable kann
Tragendes nur noch aus Zusammenfassungen zitieren —, fährt Fable das laufende
Ticket bis zur nächsten sauberen Grenze, räumt Worker und Loop ab, schreibt
den Abschluss ins Manifest und beendet den Lauf mit dem Hinweis, eine frische
Dirigenten-Session zu starten. Mit angeschlagenem Kontext beginnt kein neues
Ticket.

## 6. Aktive Altlasten entfernen

Dieser Abschnitt ist der **letzte** Umsetzungsschritt, nicht der erste. Er wird
erst freigegeben, wenn Remote Control, Loop, direkter Hintergrundworker und
Codex-Prüfpfad an einem echten Ticket belegt sind. Bis dahin bleibt der alte
Weg als Rückfallmöglichkeit unverändert. Scheitert der Probelauf, wird nur der
reversible Umstellungscommit zurückgenommen; es wird nichts weiter abgebaut.

### 6.1 Matrix pausieren

**Erledigt durch den User am 26.08.2026.** Der Bridge-Ordner unter dem
Benutzerprofil existiert nicht mehr; es gibt nichts zu pausieren und keinen
Matrix-Rückweg. Bei einer späteren Stufe D bleibt nur zu prüfen, dass kein
Autostart-Eintrag (`Startup/Nakama Matrix Bridge.vbs`) zurückgeblieben ist.

### 6.2 Nimbalyst aus dem aktiven Workspace entfernen

- `.claude/skills/dirigent/SKILL.md` wird auf Rolle, Quellen, Minimalzyklus,
  Prioritäten, die Sol-Effort-Regeln und Haltgründe gekürzt. Historische
  Nimbalyst-/Matrix-Erzählung und Werkzeuganekdoten gehören nicht in den
  aktiven Skill. **Erledigt 26.08.2026** (vorgezogen, da die alte Fassung
  tote Werkzeuge aufrief; die alte Fassung liegt in der Git-Historie).
- `.claude/skills/fragen/SKILL.md` stellt Fragen und Bilder direkt im
  Claude-/Remote-Control-Kanal; keine Nimbalyst-Werkzeuge und kein Ersatzbus.
  **Erledigt 26.08.2026** (vorgezogen, gleicher Grund).
- `CLAUDE.md` verliert den gesamten Matrix-Remote-Berichtsblock und verweist
  für den nächsten Schritt auf `docs/PLAN-STAND.md` statt auf
  `docs/NEXT-SESSION.md`. Der direkte Remote-Control-Start wird nur im
  Dirigenten-Skill beschrieben, nicht erneut als zweite Anleitung.
- `AGENTS.md` verweist für geplante Arbeit ebenfalls direkt auf
  `docs/PLAN-STAND.md` und die konkrete Ticketquelle.
- `docs/NEXT-SESSION.md` wird aus dem aktiven Workspace entfernt; seine
  Chronik bleibt über Git auffindbar. Echte offene User-Fragen bleiben in
  `docs/plan/fragen.json`, technische offene Punkte in
  `docs/offene-punkte.md`.
- `nimbalyst-local/automations/planstand-nakama.md` und der ignorierte
  Laufartefakt-Ordner darunter werden nach exakter Pfadprüfung entfernt.
- Danach fällt nur der zugehörige Nimbalyst-Block aus `.gitignore`.

### 6.3 Automatische Kontext- und Pflegeebene verkleinern

In `.claude/settings.json` bleiben nur Hooks, die einen konkreten schweren
Fehler verhindern und deren Wirkung nicht schon durch die normale
Arbeitsanweisung getragen wird:

- `nakama-primer.sh` nur nach Context-Compaction, weil er den Wahrheitskern aus
  `CLAUDE.md` liest statt ihn zu kopieren,
- `git-riegel.sh` als enger Blocker gegen destruktive oder fremde Arbeit
  erfassende Git-Befehle.

Aus der aktiven Hook-Konfiguration fallen:

- `depth-primer.sh`, `plan-primer.sh` und `design-primer.sh` — Kontext wird nur
  geladen, wenn der Auftrag ihn braucht,
- `agent-reminder-nakama.sh` — seine Invarianten stehen bereits in
  `CLAUDE.md`,
- `planstand.sh` als PostToolUse-Automatik — der vorhandene Generator wird
  bewusst zu Arbeitsbeginn und nach einem Ticket ausgeführt,
- `auto-push.sh` — die arbeitende Session committet und pusht ihren eigenen
  Änderungssatz unmittelbar selbst,
- `session-start-marker.sh` und `commit-stop.sh` — Abschlussprüfung und
  explizite Pathspec-Commits bleiben Aufgabe der arbeitenden Session,
- `fremdmodell-riegel.sh` — fremde Modelle erhalten bei ihrem konkreten Aufruf
  explizit nur Leserechte statt eines globalen Dauerwächters,
- `kreativ-schleuse.sh` — alte Designflächen sind durch aktive
  Arbeitsanweisung und Git geschützt; Designarbeit liest ohnehin zuerst
  `design/LIES-MICH.md` und die aktuelle Abnahme.

Danach werden die nicht mehr referenzierten Hook-Skripte, Proben und Marker
entfernt statt als totes Wartungsinventar liegen zu bleiben. Der kleine Test
des verbleibenden Git-Riegels wird auf dessen reale Verbote begrenzt. Die
aktuelle ungetrackte `.claude/settings.local.json` besteht aus alten
Einmal-Berechtigungen für Temp-Pfade und erledigte Befehle. Nach Sicherung
außerhalb des Workspaces wird sie entfernt; neue Einmal-Freigaben werden nicht
als dauerhafte Projektkonfiguration behandelt.

Historische Beweise und Archive werden nicht umgeschrieben. Es gibt keine
allgemeine MCP-/Skill-Aufräumaktion; nur die bei jeder Session aktive
Kontext-/Pflegeebene wird auf ihren belegten Kern reduziert.

## 7. Umsetzung in vier reversiblen Stufen

### A — Native Fähigkeiten beweisen

Den Preflight aus 5.0 durchführen, ohne Produktdateien zu ändern. Der
`/loop 1m`-Test muss seine Task-ID zeigen, einmal feuern und nach `CronDelete`
in `CronList` fehlen. Ein nicht schreibender Opus-Testagent muss über `--bg`
starten, in `claude agents --json` erscheinen und sich über `claude stop` und
`claude rm` vollständig beenden lassen. Remote Control muss dieselbe
Fable-Sitzung gleichzeitig im Terminal und auf der zweiten Oberfläche zeigen.
Der Opus-Test muss außerdem den tatsächlich aktiven Auto-Modus und einen
harmlosen, eng freigegebenen Kommandoaufruf ohne Eingriff des Users belegen.

Scheitert diese Stufe, endet das Vorhaben ohne Migration. Es wird kein
Hilfsskript gebaut und kein alter Weg verändert.

### B — Aktiven Kontext reversibel umstellen

In einem eigenen, expliziten Commit:

- `worktree.bgIsolation = "none"` zusammen mit der Ein-Schreiber-Regel setzen,
- Dirigenten- und Fragen-Skill auf den Minimalablauf umstellen,
- Routen in `AGENTS.md`/`CLAUDE.md` auf den gerechneten Planstand reduzieren,
- die Hook-Konfiguration auf Compaction-Primer plus Git-Riegel verkleinern,
- aktive Hinweise auf Auto-Push, automatische Planpflege und die entfernten
  Schleusen berichtigen.

Matrix, Nimbalyst, `NEXT-SESSION.md` und das historische
Dirigentenprotokoll bleiben in dieser Stufe noch erhalten, werden aber vom
neuen Skill nicht mehr benutzt. Prüfmaßstab: Jeder verbleibende Absatz in den
aktiven Dateien steuert eine heutige Entscheidung oder einen heutigen
Handgriff. Historie und doppelte Begründungen fallen heraus. Der Commit wird
gepusht, damit die Rückkehr ein normaler `git revert` bleibt.

### C — Ein echtes Ticket Ende-zu-Ende führen

Fable wählt den nächsten planmäßigen Schritt nur dann als Probeticket, wenn er
kein Gate und kein User-, Figma-, FL- oder Installationsschritt ist. Das Ticket
wird vollständig gefahren: Fable `xhigh` in Terminal plus Remote Control,
Opus `max` als direkter CLI-Hintergrundprozess im sichtbaren Checkout,
Messung am Repo sowie `gpt-5.6-sol` mit `high` oder `xhigh` für frischen Review
und — nur bei materiellem Befund — Nacharbeit im selben Thread plus frischen
Wiederreview.

Der Beweis muss zeigen:

- kein neues Claude-Worktree und genau ein schreibender Worker,
- sauberer Arbeitsbaum sowie Worker-Commit und Push liegen mit identischem HEAD
  im sichtbaren `master` und auf `origin/master`,
- Basis- und HEAD-SHA des Codex-Reviews stimmen mit dem Ticketbereich überein,
- die Thread-ID stammt aus dem JSONL-Lauf und Resume nutzt genau diesen Thread,
- der Kontrollloop ist nach Worker-Ende gelöscht,
- die beendete Worker-Session ist nach Sicherung der Beweise entfernt,
- Manifest und gezielte Tests tragen das Ergebnis; kein Selbstbericht ersetzt
  den Beleg.

Scheitert der Probelauf, hält Fable. Der Umstellungscommit aus B kann normal
zurückgenommen werden; einen funktionierenden Altpfad gibt es seit dem
26.08.2026 nicht mehr (Stand-Notiz oben).

### D — Alten Weg erst nach dem Beweis stilllegen

Erst nach bestandenem C werden Matrix-Prozess und Autostart pausiert,
Nimbalyst-Laufartefakte und aktive Verweise entfernt, `NEXT-SESSION.md` aus dem
aktiven Workspace genommen und die nicht mehr referenzierten Hooks, Proben und
Marker gelöscht. `claude mcp list`, `git status --ignored`, die verbleibende
Hook-Probe und ein Neustart belegen den Endzustand. Der Bridge-Ordner bleibt als
Rückweg unangetastet.

Wenn dieser Ablauf trägt, ist kein Harness zu bauen. Dieser Plan wird danach
aus dem aktiven Dokumentationsbereich entfernt oder als abgeschlossener Verlauf
archiviert. Der gebaute Endzustand steht ausschließlich im kurzen
Dirigenten-Skill und den ohnehin kanonischen Projektquellen.

## 8. Fertig bedeutet weniger System

Das Vorhaben ist fertig, wenn:

- Nimbalyst im aktiven Workspace nicht mehr existiert,
- Matrix weder läuft noch automatisch startet,
- der Dirigent in derselben Fable-Session lokal und remote bedienbar ist,
- die Sitzung nach Prozessende über ihren Namen fortgesetzt werden kann und
  vor jeder Fortsetzung Worker, Kontrollloop, Git-Status und HEAD neu abgleicht,
- Bauer und Prüfer einmal über native Werkzeuge erfolgreich geführt wurden,
- der Bauer ohne `bypassPermissions`, ohne eigenes Worktree und als einziger
  Schreiber im sichtbaren Checkout im belegten Auto-Modus gearbeitet hat,
- der sessiongebundene Weckloop Fable regelmäßig ohne User-Prompt zurückholt
  und nach Worker-Ende laut `CronList` nicht weiterläuft,
- Fable nachweislich mit `xhigh`, Opus mit `max` und jeder Sol-Lauf mit dem von
  Fable ausdrücklich gewählten, von der CLI unterstützten `high` oder `xhigh`
  gestartet und im Ticketmanifest genannt wurde,
- jeder Codex-Review mit dem ursprünglichen Basis-SHA, dem gemessenen HEAD und
  temporär gesichertem JSONL-Stream lief; kein Aufruf enthält den nicht
  unterstützten Schalter `--strict-config`,
- kein eigener Prozessmanager, Scheduler, Zustandsspeicher, Ergebnisschema oder
  dauerhaftes Laufprotokoll hinzugekommen ist,
- normale Sessionstarts keinen auftragsfremden Plan-, Design- oder
  Tiefenkontext mehr injizieren und kein Hook selbst committet oder pusht,
- eine frische Session nach dem kurzen Einstieg sofort am Nakama-Ticket statt
  am Orchestrierungssystem arbeitet,
- eine frische Dirigenten-Session an jeder Ticketgrenze allein aus Repo-Stand
  übernehmen kann — Manifest, Planstand und Git genügen, ohne Übergabetext,
- dieser Migrationsplan selbst keine zweite aktive Wahrheit mehr ist.

Für künftige Hilfswerkzeuge gilt vor dem Bau eine einzige Entscheidung:

> Verhindert dieses Werkzeug gerade einen konkreten Produktfehler, den weder
> das vorhandene Werkzeug noch ein einfacher Handgriff abdeckt — und kann es
> später vollständig entfernt werden?

Wenn nicht, wird es nicht gebaut.

## 9. Verifizierte Werkzeugannahmen

Diese Links begründen die Mechanik, nicht den Nakama-Produktstand:

- [Claude Code: Agent View](https://code.claude.com/docs/en/agent-view) —
  `--bg`, Supervisor, `claude agents` und das standardmäßige Worktree-Verhalten
  samt `worktree.bgIsolation = "none"`.
- [Claude Code: Scheduled Tasks](https://code.claude.com/docs/en/scheduled-tasks)
  — `/loop`, Task-IDs, Löschen, Wiederaufnahme und Grenzen bei geschlossener
  Sitzung.
- [Claude Code: Remote Control](https://code.claude.com/docs/en/remote-control)
  — dieselbe lokale Session in Terminal und Remote-Oberfläche.
- [Claude Code: Permission Modes](https://code.claude.com/docs/en/permission-modes)
  — Unterschiede zwischen Auto, `acceptEdits`, manuellen Freigaben und
  `bypassPermissions`.
- [OpenAI: Codex Configuration Reference](https://learn.chatgpt.com/docs/config-file/config-reference)
  — gültige Werte für `model_reasoning_effort`.

Am 26.08.2026 lokal geprüft (zweitgeprüft am selben Tag): Claude Code `2.1.218`
unterstützt `fable`, `--effort`, `--bg`, `--name`, `--remote-control`,
`agents --json --cwd --all`, `attach`, `logs`, `stop`, `rm` und
`worktree.bgIsolation`; `--permission-mode` listet `auto` als Wahlwert, seine
Konto-/Modellverfügbarkeit bleibt bewusst Beweis von Stufe A. `--resume` mit
einem Nicht-ID-Wert öffnet laut CLI-Hilfe nur den interaktiven Picker mit
Suchbegriff — deterministische Fortsetzung braucht die Session-ID (in 5.2
eingearbeitet). Codex CLI `0.130.0-alpha.5` akzeptiert die geplanten
`exec review`-/`exec resume`-Formen (`exec resume` nimmt UUID oder
Thread-Namen), `--ignore-user-config`, `--base`, `-a never` und Sandboxes;
`--strict-config` und Effort `max` werden nicht akzeptiert. Diese Versionszeile
ist nur Preflight-Snapshot und wird vor einer späteren Umsetzung neu gemessen.
