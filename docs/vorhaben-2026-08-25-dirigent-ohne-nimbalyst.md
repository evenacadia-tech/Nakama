# Vorhaben 25.08.2026 — schlanker Dirigent ohne Nimbalyst

Status: **entscheidungsreif, noch nicht umgesetzt**. Dieser Plan ersetzt die
technisch belastbare, aber zu komplex geratene Fassung vom selben Tag.

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

Zusätzlich fehlten fünf Grenzen:

1. **Native Fähigkeiten zuerst.** Claude Code kann mit `claude -p --bg`
   Hintergrund-Agenten starten und mit `claude agents` verwalten. Dafür wird
   kein eigener WorkerHost benötigt.
2. **Manuelle Erholung ist erlaubt.** Ein seltener fehlender Wakeup ist billiger
   als ein dauerhaft zu pflegender Scheduler. Fable oder der User prüft dann
   den vorhandenen Agentenstatus; daraus entsteht kein neues Subsystem.
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

## 3. Feste Komplexitätsgrenze

Für diesen Umbau gelten folgende Grenzen:

- kein Daemon, Scheduler, Watchdog oder Autostart für den Dirigenten,
- kein eigener Prozessmanager, Zustandsautomat oder Lockdienst,
- keine neuen JSON-Schemata für Bauer-, Review- oder Fixergebnisse,
- keine dauerhaften Laufprotokolle oder Runtime-Dateien im Repository,
- kein neues Dashboard und keine externe Statusdatenbank,
- keine Kopie von Ticketstand oder Produktwahrheit in Hilfsdokumenten,
- keine informierenden oder automatisch commit-/pushenden Dauer-Hooks,
- kein allgemeines Aufräumen funktionierender Werkzeuge nebenbei.

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

### 5.1 Dirigent starten

Der User startet direkt eine echte Fable-Session:

```powershell
claude --model fable --name nakama-dirigent --remote-control nakama-dirigent
```

Terminal und Remote Control bedienen dieselbe Session. Es gibt keinen
zusätzlichen Nachrichtenkanal und keinen Hintergrunddienst. Schließt das
Terminal, endet dieser lokale Betriebsmodus; das ist eine bewusste Grenze,
kein Fehler, der durch weitere Infrastruktur verdeckt werden muss.

### 5.2 Ticket festlegen

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

Läuft bereits ein verändernder Agent oder Bau im selben Workspace, wartet
Fable. Fremde Änderungen werden benannt und nicht angefasst. Der vollständige
Ausgangs-SHA bleibt im Dirigentenkontext und im Ticketauftrag; dafür braucht es
keine eigene Zustandsdatei.

### 5.3 Bauen

Der Bauer ist ein frischer Claude-Hintergrund-Agent mit Opus:

```powershell
claude -p --bg --model opus "<selbsttragender Ticketauftrag>"
```

Der Auftrag enthält nur Ticketgrenze, verbindliche Quellen, Manifestpfad,
Beweislauf und die Git-Regeln. Claude Code verwaltet den Agenten; Fable prüft
seinen Zustand mit `claude agents`. Es gibt genau einen verändernden Arbeiter
zur Zeit als einfache Arbeitsregel, nicht als eigenes Locksystem.

Fable übernimmt keinen Selbstbericht. Nach Ende misst es direkt:

- den vollständigen Diff vom Ausgangs-SHA bis zum aktuellen HEAD,
- das aktuelle Ticketmanifest,
- die ausgeführten gezielten Tests,
- unberührte fremde Pfade.

Ein Exit-Code oder Commit allein bedeutet nicht fertig. Ein blockierter Agent
darf ehrlich ohne Änderung enden. Unerwarteter HEAD- oder Worktree-Drift führt
zum Halt; Fable baut dafür keine automatische Reparatur.

### 5.4 Mit Codex prüfen und gezielt nacharbeiten

Ein frischer Codex-Thread prüft lesend den vollständigen Ticketbereich, den
unveränderten Gate-Text und das Manifest. Temporäre CLI-Ausgaben liegen nur
unter `$env:TEMP` und werden nach dem Ticket entfernt.

```powershell
codex -C . -s read-only -a never --strict-config exec review `
  --json -o <temp-review.txt> -
```

Der Prompt nennt die exakten Basis- und HEAD-SHAs. Der JSONL-Stream liefert die
Thread-ID; ein eigenes Ergebnisschema wird nicht gepflegt. Das Urteil ist
`PASS`, `NEEDS_WORK` oder `BLOCKED` und nennt knapp, was tatsächlich geprüft
und nicht geprüft wurde.

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
codex -C . -s workspace-write -a never --strict-config exec resume `
  <thread-id> -
```

Danach prüft ein neuer frischer Codex-Thread erneut den **gesamten** Bereich
vom ursprünglichen Ausgangs-SHA bis zum neuen HEAD. Nach zwei erfolglosen
Nacharbeitsrunden hält Fable an. Ein bestandenes Urteil wird unverändert in das
vorhandene Ticketmanifest übernommen; dafür entsteht kein weiteres Protokoll.

### 5.5 Weiter oder halten

Fable fährt mit dem nächsten Ticket fort. Es hält nur bei:

- einem User-, Figma-, FL- oder Installationsschritt,
- einem Produktentscheid,
- überlappenden fremden Änderungen,
- einem materiellen, zweimal nicht geschlossenen Befund,
- einer fehlenden nativen Fähigkeit, deren Ersatz neue Infrastruktur
  erfordern würde,
- einem Phasengate oder leerem Plan.

## 6. Aktive Altlasten entfernen

### 6.1 Matrix pausieren

Matrix ist nicht Teil des neuen Wegs. Die Umsetzung:

1. prüft die Identität des noch laufenden Bridge-Prozesses,
2. beendet genau diesen Prozess,
3. verschiebt/deaktiviert `Startup/Nakama Matrix Bridge.vbs`,
4. beweist nach Neustart, dass Prozess und Log inaktiv bleiben.

Der Bridge-Ordner außerhalb des Repositories bleibt mit Konfiguration, Store,
Queues und `nimbalyst.py` unangetastet. Matrix ist pausiert, nicht zerstört.

### 6.2 Nimbalyst aus dem aktiven Workspace entfernen

- `.claude/skills/dirigent/SKILL.md` wird auf Rolle, Quellen, Minimalzyklus,
  Prioritäten und Haltgründe gekürzt. Historische Nimbalyst-/Matrix-Erzählung
  und Werkzeuganekdoten gehören nicht in den aktiven Skill.
- `.claude/skills/fragen/SKILL.md` stellt Fragen und Bilder direkt im
  Claude-/Remote-Control-Kanal; keine Nimbalyst-Werkzeuge und kein Ersatzbus.
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

## 7. Umsetzung in drei kleinen Schritten

### A — Altlast aus

Matrix-Prozess und Autostart pausieren. Nimbalyst-Laufartefakte und aktive
Verweise entfernen. `claude mcp list` und `git status --ignored` zeigen keine
Nimbalyst-Reste im aktiven Workspace.

### B — Aktiven Kontext kürzen

Dirigenten- und Fragen-Skill umstellen, Routen in `AGENTS.md`/`CLAUDE.md` auf
den gerechneten Planstand reduzieren, `NEXT-SESSION.md` aus dem aktiven
Workspace entfernen, das alte Dirigentenprotokoll aus dem Pflichtablauf nehmen
und die Hook-Konfiguration auf Compaction-Primer plus Git-Riegel verkleinern.

Prüfmaßstab: Jeder verbleibende Absatz in diesen aktiven Dateien steuert eine
heutige Entscheidung oder einen heutigen Handgriff. Historie und doppelte
Begründungen fallen heraus.

### C — Ein realer Probelauf

Mit einem kleinen echten Nakama-Ticket den direkten Ablauf einmal vollständig
fahren: Fable per Remote Control, ein nativer Claude-Hintergrund-Agent, Messung
am Repo, frischer Codex-Review und — nur bei materiellem Befund — Nacharbeit im
selben Thread plus frischer Wiederreview.

Wenn dieser Ablauf trägt, ist kein Harness zu bauen. Dieser Plan wird danach
aus dem aktiven Dokumentationsbereich entfernt oder als abgeschlossener Verlauf
archiviert. Der gebaute Endzustand steht ausschließlich im kurzen
Dirigenten-Skill und den ohnehin kanonischen Projektquellen.

## 8. Fertig bedeutet weniger System

Das Vorhaben ist fertig, wenn:

- Nimbalyst im aktiven Workspace nicht mehr existiert,
- Matrix weder läuft noch automatisch startet,
- der Dirigent in derselben Fable-Session lokal und remote bedienbar ist,
- Bauer und Prüfer einmal über native Werkzeuge erfolgreich geführt wurden,
- kein eigener Prozessmanager, Scheduler, Zustandsspeicher, Ergebnisschema oder
  dauerhaftes Laufprotokoll hinzugekommen ist,
- normale Sessionstarts keinen auftragsfremden Plan-, Design- oder
  Tiefenkontext mehr injizieren und kein Hook selbst committet oder pusht,
- eine frische Session nach dem kurzen Einstieg sofort am Nakama-Ticket statt
  am Orchestrierungssystem arbeitet,
- dieser Migrationsplan selbst keine zweite aktive Wahrheit mehr ist.

Für künftige Hilfswerkzeuge gilt vor dem Bau eine einzige Entscheidung:

> Verhindert dieses Werkzeug gerade einen konkreten Produktfehler, den weder
> das vorhandene Werkzeug noch ein einfacher Handgriff abdeckt — und kann es
> später vollständig entfernt werden?

Wenn nicht, wird es nicht gebaut.
