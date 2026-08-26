# Vorhaben 25.08.2026 — schlanker Dirigent ohne Nimbalyst

Status: **Planungsfassung 3, noch nicht umgesetzt**. Dieser Plan ersetzt die
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

### 1.1 Führungsentscheidung vom 26.08.2026

Für die sichtbare Bedienung gilt zusätzlich das User-Wort:

> *„ich bin projektleiter hier im grunde. das heißt ich muss sehen dass jeder
> in der spur läuft und nicht abweicht von plänen. wenn abgewichen wird, muss
> nachvollziehbar sein warum. vollständigkeit, begründung warum nicht.“*

> *„nein dauerhaft offen. wichtig ist auch, dass fable automatisch kontaktiert
> wird bei wichtigen events die eine form von eingreifen erfordern. fable hat
> hier volle entscheidungskompetenz, genau wie ich“*

> *„auf technischer ebene ja , auf designebene nicht.“*

> *„umso schwerer er die arbeitsphase einschätzt umso häufiger muss er
> kontrollieren“*

> *„ich will sehen : bei zum beispiel 4 offenen laufenden sessions : Fable
> Dirigent xhigh aktiv Claude Opus xhigh aktiv codex Sol xhigh aktiv dazu bei
> fable den kontextstand 0-1 mio token. mein nutzungskontingent für claude für
> die 5 stunden und wochenkontingent und codex wochenkontingent.“*

> *„der eingeklappte plan: es geht mir mehr darum zu sehen wieviel erledigt
> wurde und wiewviel noch offen ist. das heißt eine übersichtliche liste anfang
> bis ende , in welcher deutlich ist was fertig und was noch zu erledigen ist.“*

Daraus folgt ein **dauerhaft sichtbares Dirigenten-Cockpit im bestehenden
Terminal**. Es zeigt nur Führungsinformationen: beteiligte Sitzungen, Fables
Kontext, verfügbare Kontingente, Aufsichtsstufe, Gesamtfortschritt und eine
offene User-Frage. Technische Rohdaten, Logausgaben und lange Begründungen
bleiben dahinter. Fable wird bei eingriffsbedürftigen Ereignissen sofort
geweckt; ein risikobasierter Kontrolltakt bleibt als Sicherheitsnetz.

Fable darf technische Vorgehensweisen innerhalb der verbindlichen
Produktgrenzen ändern, wenn die Abweichung für das Ticket nötig ist. Es muss
Grund, Auswirkung und Beweis im vorhandenen Ticketmanifest festhalten. Design,
Produktwirkung und kreative Richtung entscheidet weiterhin ausschließlich
der User. Das Cockpit zeigt eine solche offene Entscheidung als kurze Frage;
es überträgt sie nicht still an Fable.

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

Zusätzlich fehlten zwölf Grenzen:

1. **Native Fähigkeiten zuerst.** Claude Code kann mit `claude --bg` einen
   direkten CLI-Prozess im Hintergrund starten. `claude agents` startet ihn
   nicht, sondern zeigt danach nur seinen Zustand. Dafür wird kein eigener
   WorkerHost benötigt. `--bg` und `-p` sind nicht kombinierbar.
2. **Ereignis vor Uhr.** Ein laufender Monitor meldet Fable bedeutsame
   Zustandswechsel des Workers oder Prüfers sofort. Ein risikobasierter
   `/loop` bleibt nur das zeitliche Sicherheitsnetz für Stillstand und nicht
   gemeldete Ereignisse. Nach Ende des Tickets werden Monitor und Loop beendet.
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
10. **Projektleitung braucht eine andere Sicht als ein Entwickler.** Der User
    muss auf einen Blick erkennen, ob alle Beteiligten in der Spur sind, wie
    viel des Plans wirklich abgenommen ist und ob eine Entscheidung wartet.
    Logs, IDs, Dateilisten und Technikbegründungen gehören nicht in diese
    Grundansicht.
11. **Fortschritt ist kein KI-Gefühl.** „Fertig“ wird nur aus Planquelle,
    Ticketmanifest und aktuellem Repo-Beleg gerechnet. Gebaut, aber noch nicht
    geprüft, zählt weiterhin als offen.
12. **Kontingent und Kontext müssen ehrlich sein.** Das Cockpit zeigt nur vom
    Werkzeug gemeldete Werte samt Alter. Fehlt ein Wert, steht dort
    „nicht verfügbar“ statt einer Schätzung oder scheinbaren Vollständigkeit.

## 3. Feste Komplexitätsgrenze

Für diesen Umbau gelten folgende Grenzen:

- kein eigener Daemon, Scheduler, Watchdog oder Autostart für den Dirigenten;
  der native sessiongebundene `/loop` und ein an die laufende Fable-Session
  gebundener Monitor sind ausdrücklich erlaubt,
- kein eigener Prozessmanager, Zustandsautomat oder Lockdienst,
- keine neuen JSON-Schemata für Bauer-, Review- oder Fixergebnisse,
- keine dauerhaften Laufprotokolle oder Runtime-Dateien im Repository,
- kein zweites Dashboard-System und keine externe Statusdatenbank; erlaubt ist
  genau eine dünne, read-only Terminaldarstellung über Claudes native
  mehrzeilige `statusLine`,
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

Standardfall: **genau ein dünner Cockpit-Renderer, sonst keine neue dauerhafte
Harness-Datei**. Der Renderer nimmt Status-JSON auf stdin entgegen, liest nur
kanonische Quellen und gibt Text aus. Derselbe Helfer darf in einem zweiten,
sessiongebundenen Beobachtungsmodus Zustandswechsel eines konkret benannten
Workers ausgeben; er verwaltet oder startet den Worker nicht. Er besitzt keine
Produktlogik, keine Recovery, keine Logs und keine eigene dauerhafte Wahrheit.
Außer seinem letzten Vergleichswert im laufenden Prozess hält er keinen
Zustand. Falls sich beim
realen Probelauf zeigt, dass der direkte Startbefehl wiederholt falsch
eingegeben wird, ist zusätzlich höchstens ein dünnes `Start-Dirigent.ps1`
erlaubt. Es darf nur den unten stehenden Claude-Aufruf weiterreichen — ohne
Zustand, Prozessverwaltung oder eigene Regeln.

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

### 4.1 Das dauerhaft sichtbare Dirigenten-Cockpit

Das Cockpit sitzt als mehrzeilige native Claude-`statusLine` dauerhaft am
unteren Rand des Dirigententerminals. Es ist eine **Projektleitungsansicht**,
keine Technik-Konsole. Sein Normalzustand passt ohne Scrollen in ungefähr zehn
Zeilen. Die folgenden Zahlen sind nur ein Layoutbeispiel, kein behaupteter
aktueller Projektstand:

```text
NAKAMA DIRIGENT                              ✓ ALLES IN SPUR
AUFSICHT ENG · nächste Kontrolle 04:12 · wichtige Events sofort

Fable       Dirigent · xhigh · aktiv        Kontext ✓ Gesamtplan + Ticket
Claude Opus Bauer · max · arbeitet          Kontext ✓ aktueller Auftrag
Codex Sol   Prüfer · xhigh · wartet          Kontext ✓ voller Prüfbereich

FABLE-KONTEXT  214k / 1.000k  [██░░░░░░░░] 21 %
CLAUDE          5 Stunden 27 % verbraucht · Woche 44 %
CODEX           Woche 18 % verbraucht
PLAN            12 / 36 fertig · 24 offen  [███░░░░░░░]
FRAGE           keine offene Frage          Plan: /dirigent stand
```

Die Zahl der Sitzungszeilen ist dynamisch. Angezeigt werden alle zum aktuellen
Dirigentenlauf gehörenden Sitzungen — arbeitend, wartend, eingabebedürftig oder
seit dem letzten Ticketabschluss beendet. Alte, nicht beteiligte Sitzungen und
technische IDs bleiben verborgen. Nach der Ticketbereinigung verschwinden die
beendeten Teilnehmer.

Die oberste Aussage hat genau drei Zustände:

- `✓ ALLES IN SPUR`: alle laufenden Teilnehmer besitzen den richtigen
  Arbeitsanker, kein ungeklärter Drift und keine offene Entscheidung,
- `! EINGRIFF NÖTIG`: Blockade, unbegründete Abweichung, fehlender Beleg,
  Kontingent-/Kontextgrenze oder fehlende Statusquelle,
- `? ENTSCHEIDUNG WARTET`: Fable braucht eine Design-, Produkt-, Figma-, FL-,
  Installations- oder sonstige User-Entscheidung.

„Kontext ✓“ ist dabei **keine Behauptung, den Gedankeninhalt einer KI lesen zu
können**. Es bedeutet nur: Die Sitzung wurde mit dem verbindlichen Paket für
ihre Rolle gestartet und ihre Arbeitsanker stimmen noch. Für Fable sind das
Gesamtplan, aktuelles Ticket und aktueller Repo-Stand; für Opus der eine
Ticketauftrag mit Basisstand; für Codex der vollständige Prüfbereich mit Basis-
und Zielstand. Nach Resume, Compaction, unbekannter Sitzung oder abweichendem
Stand gilt der Kontext als `? unbestätigt`, bis Fable ihn neu abgeglichen hat.
Ein unbekannter Wert darf nie grün dargestellt werden.

Dieser Abgleich ist messbar: Seit dem letzten Start, Resume oder der letzten
Compaction müssen Fables Planstand und aktuelle Ticketquelle gelesen worden
sein und zum aktuellen HEAD passen. Beim Worker muss der Startauftrag Ticket
und Basisstand tragen; beim Codex-Lauf müssen Basis- und Zielstand im aktuellen
JSONL-Lauf stehen. Der Renderer erkennt diese Belege im werkzeugeigenen
Sitzungsverlauf und in den Startdaten. Ein bloßes „ich habe den Kontext“ einer
KI zählt nicht.

Die Grundansicht zeigt nur das Ergebnis. Muss Fable technisch vom geplanten
Weg abweichen, steht oben kurz `! ABWEICHUNG WIRD GEPRÜFT`. Fable prüft und
entscheidet selbst. Nach einer zulässigen technischen Entscheidung kehrt die
Anzeige auf Grün zurück; die nachvollziehbare Begründung steht im
Ticketmanifest. Nur wenn Wirkung oder Richtung des Produkts betroffen ist,
entsteht eine User-Frage.

#### Fortschrittsansicht

Die eingeklappte Planzeile beantwortet nur zwei Fragen: Wie viel ist wirklich
fertig, und wie viel ist noch offen? Der Zähler wird bei jedem Start und nach
jedem Ticket aus `docs/plan/plan.json`, `docs/PLAN-STAND.md` und den zugehörigen
Beweisen neu gerechnet. Eine KI darf ihn nicht schätzen oder von Hand setzen.

Die ausführliche Ansicht wird mit `/dirigent stand` direkt in derselben Sitzung
eingeblendet. Sie enthält alle Arbeitspakete vom Anfang bis zum Ende,
chronologisch unter verständlichen Hauptphasen gruppiert. Jede Zeile besteht
nur aus Statussymbol und Klartext; Ticketnummer, Pfad und Technikdetails werden
in der Projektleitungsansicht nicht gezeigt:

```text
GRUNDLAGE
✓ Gemeinsame Produktregeln festgelegt
✓ Sicheren Projektzustand eingeführt

HEUTIGE ARBEIT
→ Plugin-Oberfläche mit den freigegebenen Funktionen verbinden
◐ Umsetzung gebaut, abschließende Prüfung fehlt

DANACH
○ Verhalten im echten Musikprogramm prüfen
○ Installation und Rückweg abschließend belegen
```

Die Symbole und die Zählregel sind fest:

- `✓` vollständig umgesetzt, aktuell belegt und abgenommen — zählt als fertig,
- `◐` gebaut oder teilweise belegt, aber noch nicht vollständig abgenommen —
  zählt als offen,
- `→` aktuell in Arbeit — zählt als offen,
- `○` noch nicht begonnen — zählt als offen.

Die ausführliche Ansicht übersetzt vorhandene Planpakete in nachvollziehbare
Sprache; sie erfindet, verschmilzt oder überspringt keine Arbeitspakete. Dafür
erhält jeder Schritt in der kanonischen `docs/plan/plan.json` genau einen
kurzen, nicht technischen `leitungsname`. Er ist nur die Projektleitungszeile,
kein zweiter Status und keine zweite Arbeitsspezifikation. Der ausführliche
Tickettext bleibt maßgeblich. Der Generator prüft, dass jeder Schritt einen
Leitungsnamen besitzt. Er beschreibt Wirkung oder überprüfbares Ergebnis in
höchstens 90 Zeichen und enthält weder Ticket-ID, Dateipfad noch interne
Technikabkürzung. Status und Reihenfolge werden weiterhin ausschließlich aus
den bisherigen Plan- und Beweisquellen gerechnet. Eine separate Übersetzungs-
oder Mappingdatei entsteht nicht.

Ein Paket darf nicht durch Umbenennen oder Ausblenden scheinbar verschwinden.
Ist es begonnen, aber nicht fertig, ergänzt die Ansicht nach `◐` höchstens
einen kurzen Klartextgrund wie `abschließende Prüfung fehlt`. Wird vom
technischen Weg abgewichen, zeigt die Planansicht nur die knappe Auswirkung;
die vollständige technische Begründung und der Beweis stehen im
Ticketmanifest. Ein bewusst nicht umgesetztes oder nicht belegtes Paket bleibt
offen und zählt niemals als `✓`.

#### Offene Frage und Antwort

Die letzte Zeile zeigt entweder `keine offene Frage` oder genau **eine** kurze,
handlungsfähige Frage von Fable. Die vollständige Frage steht direkt darüber
in derselben Claude-/Remote-Control-Sitzung; der normale Eingabebereich ist das
Antwortfeld. Es gibt keinen zweiten Posteingang und keine zusätzliche App.
Planbare User-Fragen stammen aus `docs/plan/fragen.json`; eine akute Frage aus
dem laufenden Ticket wird aus dem werkzeugeigenen Sitzungstranskript erkannt.
Mit der nächsten User-Antwort verschwindet der offene Zustand. Reicht eine
Antwort nicht aus, stellt Fable genau eine präzisierte Folgefrage.

#### Quellen und Ehrlichkeit

Der Renderer liest ausschließlich:

- das von Claudes `statusLine` gelieferte JSON für Fable-Modell, Effort,
  tatsächliches Kontextfenster, Kontextverbrauch sowie Claude-5-Stunden- und
  Claude-Wochenkontingent,
- `claude agents --json` und den werkzeugeigenen Sitzungskontext für
  beteiligte Claude-Sitzungen und ihre Zustände,
- den aktuellen Codex-JSONL-Strom beziehungsweise die offizielle
  `account/rateLimits/read`-Antwort für das Codex-Wochenkontingent,
- die kanonischen Plan- und Beweisquellen für Fortschritt und Arbeitsanker.

Das Kontextmaß zeigt `verbraucht / tatsächlich gemeldete Fenstergröße` auf
der verlangten Skala bis eine Million Token. Stufe A muss belegen, dass die
Fable-Sitzung tatsächlich ein Fenster von `1.000.000` meldet. Meldet das Konto
oder Modell weniger, zeigt das Cockpit den echten Wert und die Umstellung hält
mit dieser fehlenden Fähigkeit an; eine 1-Million-Skala wird nicht simuliert.

Kontingente werden als verbrauchter Anteil plus Rücksetzzeit in der
Detailansicht dargestellt. Fehlt die Quelle, lautet der Wert `nicht
verfügbar`. Für „Woche“ zählt ausschließlich ein ausdrücklich gemeldetes
7-Tage-Fenster beziehungsweise `windowDurationMins = 10080`; ein kürzeres
Fenster darf nicht falsch als Wochenkontingent beschriftet werden. Ein
kurzlebiger Cache unter `$env:TEMP` ist nur erlaubt, um teure Abfragen zu
begrenzen: höchstens 60 Sekunden alt, mit sichtbarem Alter, sessiongebunden und
beim Ticketabschluss löschbar. Er ist keine Projektwahrheit und wird nach einem
Neustart nicht wiederhergestellt.

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

- Fable für das Konto tatsächlich startbar ist und das von der Sitzung
  gemeldete Kontextfenster `1.000.000` Token beträgt,
- Terminal und Remote Control dieselbe Sitzung zeigen,
- Claudes mehrzeilige `statusLine` das Fable-Modell, tatsächliche Effort,
  Kontextverbrauch, 5-Stunden- und Wochenkontingent ohne Schätzung liefert und
  auf eine Statusänderung sichtbar aktualisiert,
- das Codex-Wochenkontingent aus dem aktuellen JSONL-Ereignis oder über
  `account/rateLimits/read` lesbar ist; die Anzeige eines absichtlich
  unterbrochenen Datenwegs ehrlich auf `nicht verfügbar` fällt,
- die Cockpit-Sitzungsliste einen gestarteten, wartenden,
  eingabebedürftigen und beendeten Testteilnehmer jeweils richtig darstellt,
- ein sessiongebundener Zustandsbeobachter zusammen mit Claudes nativem
  `Monitor` eine idle Fable-Sitzung bei einem bedeutsamen Testereignis sofort
  aufweckt und nach dem Endereignis vollständig endet,
- ein einmaliger `/loop 1m` die idle Fable-Sitzung weckt und anschließend
  vollständig gelöscht werden kann,
- die drei geplanten Kontrollintervalle `30m`, `15m` und `5m` akzeptiert,
  eindeutig aufgelistet und nach einem Stufenwechsel ohne doppelten Loop
  ersetzt werden können,
- ein nicht schreibender Opus-Hintergrundagent in `claude agents --json`
  sichtbar, stoppbar und wieder entfernbar ist,
- Opus mit `--permission-mode auto` tatsächlich im Auto-Modus startet und
  einen ausdrücklich erlaubten harmlosen Prüfkommando-Pfad ohne Rückfrage
  beendet; ein stiller Rückfall auf Manual zählt als Fehlschlag,
- der Wiedereinstieg real funktioniert: `--resume` mit einem Nicht-ID-Wert
  öffnet nur den interaktiven Picker mit diesem Suchbegriff; deterministisch
  fortgesetzt wird allein über die Session-ID. Der Preflight belegt beide Wege,
- eine wartende Frage der idle Fable-Sitzung auf der Remote-Oberfläche
  sichtbar wird, im Cockpit `? ENTSCHEIDUNG WARTET` auslöst und nach der
  Antwort wieder verschwindet,
- Plan-Kurzzeile und `/dirigent stand` für denselben Repo-Stand dieselbe Zahl
  fertiger und offener Arbeitspakete liefern; ein `◐`-Testpaket bleibt offen,
- der Codex-Review- und Resume-Aufruf die gewählten Optionen akzeptiert.

Dieser Preflight verändert keine Produktdatei. Scheitert eine Fähigkeit,
bleiben die noch nicht umgestellten Hooks und Einstiegsdateien unangetastet.
Nimbalyst und Matrix sind bereits entfernt und kein Rückfall. Der Plan hält
mit genau der fehlenden Fähigkeit an; sie wird weder versteckt noch durch
einen geschätzten Cockpit-Wert ersetzt.

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

#### Technische Entscheidung und Aufsichtsstufe

Fable besitzt innerhalb von Ticket, Produktinvarianten und freigegebener
Designrichtung volle technische Entscheidungskompetenz. Es wählt Werkzeuge,
Reihenfolge und zulässige technische Abweichungen selbst. Eine technische
Abweichung ist abgeschlossen, wenn Grund, Auswirkung auf die Abnahme und
tatsächlicher Beweis im vorhandenen Ticketmanifest stehen. Fable fragt den
User nicht nach Crate-Aufteilung, Testverdrahtung, Dateipfaden,
Bibliothekswahl oder ähnlichen Implementierungsfragen.

Designrichtung, Produktwirkung, Bedienlogik, sichtbare Priorität und kreative
Abnahme bleiben User-Entscheidungen. Berührt eine technische Alternative diese
Grenze, stoppt Fable die Umsetzung und stellt genau die daraus folgende Frage.

Zusätzlich zum Modell-Effort setzt Fable für jede laufende Arbeitsphase eine
Aufsichtsstufe. Sie steuert nur die maximale Zeit bis zur nächsten
Sicherheitskontrolle; wichtige Ereignisse melden sich in jeder Stufe sofort:

| Aufsicht | Kontrollfenster | Einsatz |
|---|---:|---|
| `LOCKER` | 30 Minuten | kleine, lokal begrenzte und leicht rücknehmbare Phase |
| `NORMAL` | 15 Minuten | normales Ticket, mehrere gekoppelte Dateien oder mittlere Unsicherheit |
| `ENG` | 5 Minuten | Audio-Thread, State/Migration, IPC/Vertrag, Nebenläufigkeit, Sicherheit, Phasengate, Nacharbeit nach bestätigtem Fehler oder erkennbarer Drift |

Fable darf jederzeit hoch- oder herabstufen. Hochstufung gilt sofort;
Herabstufung erst nach einer erfolgreichen Kontrolle. Im Cockpit stehen die
aktuelle Stufe und der Countdown. Aufsicht und Sol-Effort werden zu Beginn im
Ticketmanifest vermerkt; eine spätere Änderung nur dann zusätzlich, wenn sie
auf ein Risiko, einen Befund oder eine technische Abweichung reagiert.

### 5.2 Dirigent starten

Der User startet direkt eine echte Fable-Session:

```powershell
claude --model fable --effort xhigh --name nakama-dirigent --remote-control
```

Terminal und Remote Control bedienen dieselbe Session. Es gibt keinen
zusätzlichen Nachrichtenkanal. Der von Claude selbst bei Bedarf gestartete
Hintergrund-Supervisor ist Werkzeugmechanik, kein Nakama-Dienst.

Die in Stufe B gesetzte native `statusLine` startet mit dieser Sitzung das
Cockpit. Bevor Fable einen Worker startet, müssen dort Fable-Modell und
`xhigh`, das echte 1-Million-Token-Fenster, die verfügbaren Kontingente, der
gerechnete Planstand und `keine offene Frage` beziehungsweise die aktuelle
Frage sichtbar sein. Fehlende Werte werden benannt; eine rote oder unbekannte
Spurlage wird zuerst geklärt und nicht mit einem Worker übergangen.

Wird nur die Remote-Verbindung getrennt, läuft die lokale Sitzung weiter. Wird
der Terminalprozess beendet, sind weder Monitor noch `/loop` ein unabhängiger
Dienst; Fable reagiert erst wieder, wenn genau diese Sitzung fortgesetzt wird.
`--resume` mit dem Sitzungsnamen ist dabei **keine**
deterministische Fortsetzung: ein Nicht-ID-Wert öffnet nur den interaktiven
Picker, vorgefiltert auf den Suchbegriff. Der Name `nakama-dirigent` macht die
Sitzung dort sofort auffindbar; ohne Klick geht es nur über die Session-ID:

```powershell
claude --resume nakama-dirigent   # Picker, vorgefiltert — ein Klick des Users
claude --resume <session-id> --model fable --effort xhigh --remote-control
```

Beim bewussten Beenden stoppt Fable zuerst einen laufenden Worker, beendet den
Zustandsbeobachter und löscht seinen Kontrollloop. Nach Absturz oder Neustart
beginnt die fortgesetzte Sitzung mit `claude agents --json`, `CronList`,
`git status`, dem Vergleich von Ausgangs-SHA zu aktuellem HEAD und einer
Neuberechnung des Cockpits. Läuft der bekannte Worker noch, werden genau ein
neuer Zustandsbeobachter und ein zur aktuellen Aufsichtsstufe passender Loop
gesetzt; doppelte Beobachter oder Loops werden entfernt. Fehlt der Worker,
werden verbliebene Beobachter und Loops beendet. Bis dieser Abgleich fertig
ist, zeigt die Spurlage `? unbestätigt`. Die Sitzung rät keinen Zustand und
baut keine eigene Recovery-Datei.

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

Aus demselben gerechneten Stand aktualisiert das Cockpit seine Kurzzeile und
die vollständige `/dirigent stand`-Ansicht. Das aktuell gewählte Arbeitspaket
wird nur für die laufende Sitzung als `→` dargestellt; dafür wird keine
Planquelle umgeschrieben. Weichen Paketmenge, Fertigzähler oder nächster
Schritt zwischen den Ansichten voneinander ab, lautet die Spurlage
`! EINGRIFF NÖTIG` und es startet kein Worker.

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
Repo-Dateien strikt lesend.

Direkt nach dem Start aktiviert Fable zwei voneinander unabhängige, aber
sessiongebundene Sicherungen:

1. Der Cockpit-Helfer beobachtet im Modus `-WatchWorker` nur den konkreten
   Worker und gibt ausschließlich **Zustandsänderungen** aus. Claudes nativer
   `Monitor` leitet jede ausgegebene Zeile sofort an die Fable-Sitzung weiter.
   Arbeit ohne Zustandsänderung erzeugt weder Meldung noch Kontextverbrauch.
2. Fable setzt genau einen `/loop` mit dem Intervall der in 5.1 gewählten
   Aufsichtsstufe. Dieser Loop fängt Stillstand, einen abgestürzten Beobachter
   oder einen ausgebliebenen Zustandswechsel ab.

Sofortige Eingriffsereignisse sind mindestens:

- `needs input`, Blockade, Fehler, Abbruch oder unerwartetes Verschwinden eines
  Beteiligten,
- Fertigstellung eines Workers oder Reviews,
- unbekannter oder widersprüchlicher Arbeitsanker,
- unerwarteter HEAD-/Worktree-Wechsel außerhalb des laufenden Schreibers,
- Überschreiten einer Kontext- oder Kontingentgrenze,
- eine neu offene User-Entscheidung.

Ein Ereignis weckt Fable zur Prüfung; es entscheidet nicht an Fables Stelle.
Der Beobachter hält nur seinen letzten gesehenen Zustand im eigenen Prozess,
schreibt kein Log und keine Recovery-Datei und endet mit dem beobachteten
Worker. Er wird von Claude als Hintergrundkommando der Dirigentensitzung
überwacht, nicht als Daemon, Autostart oder unabhängiger Dienst betrieben.
Für Kontext- und Kontingentwechsel darf er den höchstens 60 Sekunden alten,
vom StatusLine-Modus erzeugten Telemetriecache lesen. Ist dieser Cache älter
oder fehlt er, meldet der Beobachter `Statusquelle unbekannt` statt einen alten
Wert als gesund zu behandeln.

Der Looptext ist für alle drei Intervalle gleich; nur `30m`, `15m` oder `5m`
ändert sich:

```text
/loop <intervall> Prüfe den laufenden Nakama-Worker und seinen Ereignisbeobachter.
Wenn beides gesund arbeitet, bestätige nur die Spurlage und warte weiter. Wenn der
Worker fertig, fehlgeschlagen, blockiert oder der Beobachter ausgefallen ist, beende
diesen Loop und führe den passenden Mess-, Nacharbeits- oder Haltpfad aus.
```

Der Monitor ist der schnelle Weg, der Loop das zeitliche Sicherheitsnetz. Der
User darf jederzeit früher eine Kontrolle anstoßen. Beim Anlegen merkt sich
Fable die von Claude gemeldete Task-ID nur im Sitzungskontext. Ändert Fable die
Aufsichtsstufe, löscht es erst den alten Loop, belegt dies über `CronList` und
setzt dann genau einen neuen. Bei Worker-Ende oder Halt löscht es die Aufgabe
über `CronDelete`, prüft ihr Fehlen mit `CronList` und beendet den Beobachter.
Beides erzeugt keine Projektdatei. Läuft Fable beim Ereignis oder
Fälligkeitszeitpunkt noch in einem Turn, wird die Kontrolle direkt danach
ausgeführt statt parallel einzugreifen. Das Intervall ist ein maximales
Kontrollfenster, keine exakte Deadline und kein Ersatz für Ereignisse.

Meldet Agent View trotz Auto `needs input`, liest Fable zuerst
`claude logs <worker-id>`. Über eine erwartete, nicht destruktive
Ticketaktion entscheidet Fable selbst. Erzwingt das Werkzeug dafür einen
physischen Klick im Peek-Panel, führt der User nur Fables eindeutige Freigabe
aus; er muss keine technische Entscheidung treffen. Ein Design- oder
Produktentscheid führt dagegen zum User-Halt. Eine destruktive, ticketfremde
oder nicht erklärbare Aktion wird nicht freigegeben und der Worker wird
gestoppt.

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

Während dieses Vordergrundlaufs sieht Fable den JSONL-Strom fortlaufend; das
ist eine engere Aufsicht als jedes Kontrollintervall. Das Cockpit führt den
Codex-Thread mit Modell, tatsächlichem Effort, Rolle und Zustand und übernimmt
aus dessen `token_count`-Ereignis das aktuelle Wochenkontingent. Reißt der
Strom ab, obwohl der Prozess weiterläuft, gilt das als sofortiges
Eingriffsereignis. Ein losgelöster Codex-Prozess ohne sichtbaren Strom ist in
diesem Ablauf nicht zulässig.

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
- einem Design-, Produktwirkungs- oder kreativen Richtungsentscheid,
- überlappenden fremden Änderungen,
- einem materiellen, zweimal nicht geschlossenen Befund,
- einer fehlenden nativen Fähigkeit, deren Ersatz neue Infrastruktur
  erfordern würde,
- erschöpftem Kontingent oder wiederholten API-Fehlern bei Fable, Opus oder
  Codex — Worker stoppen, Beobachter beenden, Loop löschen, dann Halt statt
  blindem Wiederholen,
- Kontextdruck der Dirigentensitzung nach 5.7,
- einem Phasengate oder leerem Plan.

Eine rein technische Unsicherheit ist kein User-Halt: Fable entscheidet sie,
begründet eine Planabweichung knapp im Ticketmanifest und lässt sie mitprüfen.
Nur wenn keine Variante innerhalb der bestehenden Design- und Produktgrenzen
bleibt, wird daraus eine User-Frage.

Jeder echte Halt endet als klare, wartende Frage oder Statusmeldung in der
Sitzung selbst. Das Cockpit wechselt auf `? ENTSCHEIDUNG WARTET` oder
`! EINGRIFF NÖTIG` und zeigt die kurze Handlungsfrage. Das ist der Weg, auf
dem Fable den User im Terminal und über dieselbe Remote-Oberfläche erreicht.
Einen zweiten Meldekanal gibt es nicht.

Vor dem nächsten Ticket schreibt Fable Urteil, Modell, Effort, Basis- und
End-SHA sowie die tatsächlich gelaufenen Beweise in das vorhandene Manifest,
rechnet den Planstand neu und committet/pusht nur diese Abschlussdateien. Dann
löscht es alle für dieses Ticket erzeugten temporären Codex-Dateien, entfernt
die beendete Worker-Session mit `claude rm <worker-id>` und belegt mit
`CronList`, dem beendeten Zustandsbeobachter und `claude agents --json`, dass
weder Kontrollloop noch aktiver Worker übrig sind. Erst danach verschwinden
die beendeten Teilnehmer aus dem Cockpit.

### 5.7 Kontexthaushalt des Dirigenten

Der Dirigentenkontext ist das Einzige, was zwischen den Tickets lebt — und er
ist endlich. Das Cockpit macht diese Grenze als `verbraucht / 1.000.000`
dauerhaft sichtbar. Der Zahlenwert sagt, wie voll das Fenster ist; der
zusätzliche Kontextanker sagt, ob Fable den gemeinsamen Plan und das aktuelle
Ticket nachweislich geladen und gegen den Repo-Stand abgeglichen hat. Nur
beides zusammen darf `Kontext ✓` ergeben.

Die Ampel ist fest und knapp:

- unter 70 %: normal,
- 70 bis unter 85 %: Hinweis, Quellen noch enger lesen und Ticketgrenze
  vorbereiten,
- ab 85 %, nach einer Compaction oder bei unbestätigtem Arbeitsanker: kein
  neues Ticket mehr beginnen; laufende Arbeit bis zur nächsten sicheren Grenze
  führen und eine frische Fable-Sitzung starten.

Der Zustandsbeobachter meldet den Wechsel in die letzte Stufe sofort an Fable.
Für Claude- und Codex-Kontingente gilt dieselbe Führungslogik: ab 85 %
Verbrauch erscheint eine Warnung, ab 95 % beginnt keine neue Arbeitsphase. Ein
laufender sicherer Abschluss darf noch beendet werden, wenn Fable ihn ohne
Qualitätsverlust tragen kann; sonst hält es geordnet. Die vom Anbieter
gemeldete Rücksetzzeit steht nur in der Detailansicht.

Drei Regeln halten den Kontext klein und machen ihn ersetzbar:

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

Meldet das Cockpit Kontextdruck oder kann Fable Tragendes nur noch aus
Zusammenfassungen zitieren, fährt Fable das laufende Ticket bis zur nächsten
sauberen Grenze, räumt Worker, Beobachter und Loop ab, schreibt den Abschluss
ins Manifest und beendet den Lauf mit dem Hinweis, eine frische
Dirigenten-Session zu starten. Die neue Sitzung startet wieder mit dem
Preflight der Arbeitsanker und übernimmt ausschließlich aus Repo, Planstand
und Manifest. Ein Übergaberoman und eine künstlich fortgeschriebene
„Kontextvollständigkeit“ entstehen nicht.

## 6. Aktive Altlasten entfernen

Die noch im Repository verbliebene Kontext- und Hook-Bereinigung ist der
**letzte** Umsetzungsschritt. Sie wird erst freigegeben, wenn Remote Control,
Cockpit, Ereignisbeobachter, risikobasierter Loop, direkter Hintergrundworker
und Codex-Prüfpfad an einem echten Ticket belegt sind. Matrix und Nimbalyst
sind bereits deinstalliert und kein Rückweg. Scheitert der Probelauf, wird der
reversible Umstellungscommit zurückgenommen; die fehlende Fähigkeit bleibt
offen, statt einen neuen Ersatzdienst zu bauen.

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

Den vollständigen Preflight aus 5.0 durchführen, ohne Produktdateien zu
ändern. Ein temporärer Renderer und eine temporäre `--settings`-Datei unter
`$env:TEMP` dürfen die StatusLine-Schnittstelle beweisen; sie werden danach
gelöscht.

Der Beweis umfasst mindestens:

- dieselbe Fable-Sitzung in Terminal und Remote Control, tatsächliches
  `xhigh` und ein gemeldetes Kontextfenster von `1.000.000` Token,
- echte Claude-5-Stunden-/Wochenwerte und den echten Codex-Wochenwert samt
  ehrlichem Ausfallzustand,
- eine dynamische Liste der beteiligten Testsitzungen mit richtigem Zustand
  und unbestätigtem Kontext nach einem simulierten Ankerfehler,
- eine sofortige Monitor-Wiederaufnahme der idle Fable-Sitzung nach einem
  bedeutsamen Testereignis sowie das vollständige Ende des Beobachters,
- `/loop 1m` als Weckbeweis und anschließend alle drei realen Intervalle;
  nach `CronDelete` darf in `CronList` kein alter oder doppelter Loop stehen,
- einen nicht schreibenden Opus-Testagenten über `--bg`, sichtbar in
  `claude agents --json`, im belegten Auto-Modus und vollständig stopp-/lösbar,
- identische Fortschrittszahlen in Cockpit-Kurzzeile und vollständiger
  `/dirigent stand`-Ansicht,
- eine Frage, die in Terminal und Remote Control sichtbar ist, den
  Cockpitstatus wechselt und nach der Antwort als beantwortet gilt,
- parsebare Codex-Review-/Resume-Aufrufe mit sichtbarem JSONL-Strom.

Scheitert diese Stufe, endet das Vorhaben ohne Migration. Es wird kein
Produkt-Hilfsskript gebaut, kein Cockpitwert erfunden und keine fehlende
Fähigkeit mit einem neuen Dienst nachgebaut.

### B — Aktiven Kontext reversibel umstellen

In einem eigenen, expliziten Commit:

- `worktree.bgIsolation = "none"` zusammen mit der Ein-Schreiber-Regel setzen,
- genau `tools/dirigent/cockpit.ps1` mit den drei schmalen Modi `-StatusLine`,
  `-WatchWorker` und `-Plan` anlegen; keine weiteren Renderer-, Ereignis- oder
  Zustandsdateien,
- jedem vorhandenen Schritt in `docs/plan/plan.json` einen kurzen
  `leitungsname` geben und `tools/plan/planstand.py` so absichern, dass ein
  fehlender, mehrzeiliger oder über 90 Zeichen langer Leitungsname fehlschlägt;
  Statuslogik und ausführlicher Tickettext bleiben unverändert,
- Claudes native mehrzeilige `statusLine` in `.claude/settings.json` auf den
  Renderer legen; Sitzungs-/Workerzustände höchstens fünfsekündlich und
  Kontingente höchstens minütlich neu lesen,
- Dirigenten- und Fragen-Skill auf Cockpit, technische
  Entscheidungskompetenz, sofortige Ereignisse, Aufsichtsstufen,
  `/dirigent stand` und den Minimalablauf umstellen,
- Routen in `AGENTS.md`/`CLAUDE.md` auf den gerechneten Planstand reduzieren,
- die Hook-Konfiguration auf Compaction-Primer plus Git-Riegel verkleinern,
- aktive Hinweise auf Auto-Push, automatische Planpflege und die entfernten
  Schleusen berichtigen.

Noch vorhandene Nimbalyst-Laufartefakte, `NEXT-SESSION.md` und das historische
Dirigentenprotokoll bleiben in dieser Stufe unangetastet, werden aber vom neuen
Skill nicht mehr benutzt. Prüfmaßstab: Jeder Cockpitwert hat genau eine
nachprüfbare Quelle; der Helper schreibt keine Projektdatei; jeder verbleibende
Absatz in den aktiven Dateien steuert eine heutige Entscheidung oder einen
heutigen Handgriff. Der Commit wird gepusht, damit die Rückkehr ein normaler
`git revert` bleibt. Die bereits deinstallierten Matrix-/Nimbalyst-Prozesse
werden dadurch nicht wiederhergestellt.

### C — Ein echtes Ticket Ende-zu-Ende führen

Fable wählt den nächsten planmäßigen Schritt nur dann als Probeticket, wenn er
kein Gate und kein User-, Figma-, FL- oder Installationsschritt ist. Das Ticket
wird vollständig gefahren: Fable `xhigh` in Terminal plus Remote Control,
Opus `max` als direkter CLI-Hintergrundprozess im sichtbaren Checkout,
Messung am Repo sowie `gpt-5.6-sol` mit `high` oder `xhigh` für frischen Review
und — nur bei materiellem Befund — Nacharbeit im selben Thread plus frischen
Wiederreview.

Der Beweis muss zeigen:

- das Cockpit bleibt während des gesamten Laufs sichtbar und zeigt Fable,
  Worker und Codex mit tatsächlichem Modell, Effort, Rolle, Zustand und
  ehrlichem Kontextanker,
- Fables Kontextmaß läuft gegen das echte 1-Million-Token-Fenster; Claude- und
  Codex-Kontingente sind aktuell oder sichtbar als nicht verfügbar markiert,
- der Plan nennt vor und nach dem Ticket dieselbe vollständige Paketmenge; nur
  ein wirklich belegtes und abgenommenes Paket wechselt auf `✓`,
- das Abschlussereignis des Workers erreicht Fable vor dem nächsten
  Kontrollloop; ein absichtlich ausgelöster Beobachterausfall wird spätestens
  durch den Loop erkannt,
- eine Hochstufung der Aufsicht ersetzt den alten Loop durch genau einen
  kürzeren und aktualisiert den Countdown im Cockpit,
- eine ungefährliche Testfrage kann in derselben Sitzung angezeigt und vom
  User beantwortet werden, ohne zweiten Kanal oder Projekt-Statusdatei,
- kein neues Claude-Worktree und genau ein schreibender Worker,
- sauberer Arbeitsbaum sowie Worker-Commit und Push liegen mit identischem HEAD
  im sichtbaren `master` und auf `origin/master`,
- Basis- und HEAD-SHA des Codex-Reviews stimmen mit dem Ticketbereich überein,
- die Thread-ID stammt aus dem JSONL-Lauf und Resume nutzt genau diesen Thread,
- Zustandsbeobachter und Kontrollloop sind nach Worker-Ende beendet,
- die beendete Worker-Session ist nach Sicherung der Beweise entfernt,
- Manifest und gezielte Tests tragen das Ergebnis; kein Selbstbericht ersetzt
  den Beleg.

Scheitert der Probelauf, hält Fable. Der Umstellungscommit aus B kann normal
zurückgenommen werden; einen funktionierenden Altpfad gibt es seit dem
26.08.2026 nicht mehr (Stand-Notiz oben).

### D — Verbliebene Altlasten erst nach dem Beweis entfernen

Erst nach bestandenem C werden verbliebene Nimbalyst-Laufartefakte und aktive
Verweise entfernt, `NEXT-SESSION.md` aus dem aktiven Workspace genommen und
die nicht mehr referenzierten Hooks, Proben und Marker gelöscht.
`claude mcp list`, die exakte Prüfung des Windows-Autostarts,
`git status --ignored`, die verbleibende Hook-Probe und ein Neustart belegen,
dass weder Matrix/Nimbalyst noch ein alter Informationskanal zurückgeblieben
sind. Ein Bridge-Ordner wird nicht als Rückweg erhalten, weil der User ihn
bereits vollständig deinstalliert hat.

Wenn dieser Ablauf trägt, gibt es außer dem einen Cockpit-Helfer keinen
Dirigenten-Harness. Dieser Plan wird danach aus dem aktiven
Dokumentationsbereich entfernt oder als abgeschlossener Verlauf archiviert.
Der gebaute Endzustand steht ausschließlich im kurzen Dirigenten-Skill, im
Cockpit-Helfer und den ohnehin kanonischen Projektquellen.

## 8. Fertig bedeutet weniger System

Das Vorhaben ist fertig, wenn:

- Nimbalyst im aktiven Workspace nicht mehr existiert,
- Matrix weder läuft noch automatisch startet,
- der Dirigent in derselben Fable-Session lokal und remote bedienbar ist,
- die Sitzung nach Prozessende über den namensgefilterten Picker oder
  deterministisch über ihre Session-ID fortgesetzt werden kann und vor jeder
  Fortsetzung Worker, Beobachter, Kontrollloop, Git-Status, HEAD und
  Arbeitsanker neu abgleicht,
- das dauerhaft sichtbare Cockpit in seiner Grundansicht ohne Technikprotokoll
  eindeutig `ALLES IN SPUR`, `EINGRIFF NÖTIG` oder `ENTSCHEIDUNG WARTET` zeigt,
- alle zum Lauf gehörenden Fable-, Claude- und Codex-Sitzungen dynamisch mit
  tatsächlichem Modell, Effort, Rolle, Zustand und ehrlichem Kontextanker
  erscheinen; ein unbekannter Anker nie als grün gilt,
- Fables tatsächlicher Kontextverbrauch gegen ein belegtes
  1-Million-Token-Fenster sowie Claude-5-Stunden-/Wochen- und
  Codex-Wochenkontingent sichtbar sind; fehlende oder alte Werte werden
  ausdrücklich so markiert,
- die eingeklappte Planzeile ausschließlich wirklich fertige Pakete zählt und
  `/dirigent stand` alle Pakete von Anfang bis Ende in verständlichen Phasen
  mit `✓`, `◐`, `→` oder `○` vollständig aufführt,
- Fable eine offene User-Frage in derselben Sitzung sichtbar machen kann und
  der User sie im normalen Eingabefeld lokal oder remote beantworten kann,
- Bauer und Prüfer einmal über native Werkzeuge erfolgreich geführt wurden,
- der Bauer ohne `bypassPermissions`, ohne eigenes Worktree und als einziger
  Schreiber im sichtbaren Checkout im belegten Auto-Modus gearbeitet hat,
- ein bedeutsames Workerereignis Fable über den nativen Monitor sofort erreicht
  und der zur Risikostufe passende `30m`-, `15m`- oder `5m`-Loop unabhängig
  davon als Sicherheitsnetz arbeitet,
- eine Aufsichtsstufenänderung niemals zwei Kontrollloops hinterlässt und nach
  Worker-Ende weder Beobachter noch Loop weiterlaufen,
- Fable technische Abweichungen selbst entscheidet und nachvollziehbar im
  Ticketmanifest begründet, Design- und Produktentscheidungen jedoch immer
  beim User bleiben,
- Fable nachweislich mit `xhigh`, Opus mit `max` und jeder Sol-Lauf mit dem von
  Fable ausdrücklich gewählten, von der CLI unterstützten `high` oder `xhigh`
  gestartet und im Ticketmanifest genannt wurde,
- jeder Codex-Review mit dem ursprünglichen Basis-SHA, dem gemessenen HEAD und
  temporär gesichertem JSONL-Stream lief; kein Aufruf enthält den nicht
  unterstützten Schalter `--strict-config`,
- außer `tools/dirigent/cockpit.ps1` kein Dirigenten-Harness hinzugekommen ist
  und dieser Helfer weder Prozesse verwaltet noch Projektzustand, Logs,
  Ergebnisschemata oder Recovery dauerhaft speichert,
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
- [Claude Code: Status line](https://code.claude.com/docs/en/statusline) —
  native mehrzeilige Daueranzeige, JSON auf stdin, Aktualisierung sowie Felder
  für Modell, Effort, Kontextfenster, Kontextverbrauch und 5-Stunden-/
  7-Tage-Kontingent.
- [Claude Code: Tools reference](https://code.claude.com/docs/en/tools-reference)
  — der native `Monitor` leitet neue Ausgabe eines Hintergrundkommandos in die
  laufende Claude-Sitzung, ohne dass Fable selbst pollen muss.
- [Claude Code: Scheduled Tasks](https://code.claude.com/docs/en/scheduled-tasks)
  — `/loop`, Task-IDs, Löschen, Wiederaufnahme, dynamische Monitor-Schleifen
  und Grenzen bei geschlossener Sitzung.
- [Claude Code: Costs and usage](https://code.claude.com/docs/en/costs) —
  5-Stunden-/Wochenanzeige und Regeln für klar als Cache markierte Werte.
- [Claude Code: Remote Control](https://code.claude.com/docs/en/remote-control)
  — dieselbe lokale Session in Terminal und Remote-Oberfläche.
- [Claude Code: Permission Modes](https://code.claude.com/docs/en/permission-modes)
  — Unterschiede zwischen Auto, `acceptEdits`, manuellen Freigaben und
  `bypassPermissions`.
- [OpenAI: Codex Configuration Reference](https://learn.chatgpt.com/docs/config-file/config-reference)
  — gültige Werte für `model_reasoning_effort`.
- [OpenAI: Codex slash commands](https://learn.chatgpt.com/docs/reference/slash-commands)
  — `/status` weist Kontextverbrauch und Rate-Limits als native
  Sitzungsinformationen aus.

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
Das am selben Tag lokal aus `codex app-server generate-json-schema` erzeugte
Protokoll enthält außerdem `account/rateLimits/read`,
`account/rateLimits/updated`, `usedPercent`, `resetsAt` und
`windowDurationMins`; Stufe A belegt vor Verwendung erneut, dass das installierte
Protokoll diese Felder wirklich liefert.
