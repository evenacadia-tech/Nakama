# Qualitätsmodell für Nakama

Lies diese Referenz, wenn ein Prüfprofil, eine Performance-Messung, eine
Mehrinstanzlast oder die dauerhafte Beförderung eines Befunds festzulegen ist.
Die aktuellen Zahlen, Zielnamen und Phasen immer aus Source, Planstand und
Runner lesen; diese Referenz beschreibt die stabilen Entscheidungsregeln.

## Was Qualität hier bedeutet

| Achse | Zu falsifizierende Behauptung | Mindestform des Belegs |
|---|---|---|
| Korrektheit | Der Produktionspfad liefert für gültige und adversariale Eingaben den vertraglichen Zustand beziehungsweise Audiooutput. | Goldens, Nulltests, Roundtrips, Grenz- und Mutationstests |
| Realtime | Kein nicht begrenzter oder sperrender Pfad gefährdet eine Audioblock-Deadline. | strukturelle Guards plus Callback-Messung unter Übergangs- und Sättigungslast |
| Performance | Die relevante Arbeit bleibt innerhalb eines versionierten Budgets. | Rohverteilung, hohe Quantile/Tail-Diagnose, Deadline-Verhältnis und benannte Umgebung |
| Skalierung | Gemeinsame Ressourcen wachsen und konkurrieren unter realer Instanzlast kontrolliert. | aktuelle Planmatrix plus 10 als Nutzerfall, nicht aus einer Instanz hochgerechnet |
| Stabilität | Lange Laufzeit und wiederholte Lebenszyklen erzeugen keine XRuns, Leaks, Hänger oder stale Wahrheit. | Soak plus Reload/Reconnect/Teardown und Ressourcenverlauf |
| Effizienz | CPU, Speicher, Threads, Handles, IPC und UI-Arbeit wachsen erklärbar und begrenzt. | Start/Ende/Peak/Steigung und Queue-/Drop-Zähler je Lastprofil |
| Wartbarkeit | Eine heutige Reparatur verhindert dieselbe Fehlerklasse morgen. | Regressionstest, ausreichend empfindlicher Gegenbeweis und bei Bedarf stehendes Gate |

## Profile

### quick

Für begrenzte Änderungen ohne Hochrisikonaht:

- Preflight und realen Diff prüfen;
- betroffene Source, Tests und Gegenweg lesen;
- gezielten Build/Test/Lint aus dem heutigen Repo fahren;
- Behauptung und aktuelle Roh-Ausgabe festhalten;
- prüfen, ob die Änderung unbeabsichtigt eine Hochrisikonaht berührt; dann auf
  `deep` eskalieren.

Dokumentations- oder Skilländerungen prüfen zusätzlich Links, Befehle,
Discovery und das eigene Kontext-/Prüfbudget. Ein Textvergleich allein beweist
nicht, dass ein Skript ausführbar ist.

### deep

Für Audio, Verträge, State, IPC, Broker, Nebenläufigkeit oder gekoppelte
Lebenszyklen zusätzlich:

- aktuellen kanonischen Beweislauf verwenden, wenn Ticket und Manifestziel
  feststehen;
- jede betroffene Richtung und Fehlerkante einzeln provozieren;
- echte Produktionsquelle linken, keine Testkopie der Logik;
- Audiopfad mit kleinen, nominalen, wechselnden und übergroßen Blöcken sowie
  relevanten Sampleraten prüfen;
- Cold Start, Stille/Denormal, NaN/Inf, Reset, Bank-/Statewechsel, Automation,
  Stop/Seek/Loop, Drop und Reconnect nach Risiko einbeziehen;
- Testempfindlichkeit durch einen kontrollierten Bruch zeigen, wenn ein neues
  sicherheitskritisches Gate entsteht.

### soak

Für Skalierung und seltene Fehler zusätzlich:

- Referenzsession: ein Gen plus zehn Probeeq auf Bussen;
- Skalierungsstufen aus dem aktuellen Sondenplan einschließlich 10 als
  realem Nutzerfall;
- UI geschlossen und relevante Gen-Oberfläche geöffnet getrennt messen;
- Signal, Stille/Abklingen und Worst-Case-Übergänge als getrennte Lastklassen;
- Callback-Deadline-Verhältnis, Verteilung, höchstes beobachtetes Tail,
  XRuns/Dropzähler, CPU, Working Set, Allokationen, Threads, Handles,
  Queuefüllung, IPC-Latenz und Teardown-Deltas erfassen;
- keine Catch-up-Stürme nach Pause, Debuggerhalt, Suspend oder Worker-Stau;
- Soak-Dauer und Instanzstufen aus dem aktuellen Plan ableiten und in der
  Evidenz wiederholen.

Ein Soak beweist nur die erhobene Umgebung. Er ersetzt weder den
Audiothread-Codevertrag noch eine reale Hostabnahme.

### release

Für Auslieferungsbehauptungen zusätzlich:

- alle planmäßig fälligen harten Gates und den vollständigen Beweisrunner;
- echte FL-Studio-Fixtures mit Routing, Reload und Offline-Render;
- unterstützte Host-, Samplerate-, Blockgrößen- und Bridge-Matrix;
- pluginval in der geforderten Stufe;
- Installer, Manifest, Signatur, Repair, Migration und Rückweg;
- extern nicht ausführbare Admin-, Hardware- oder Hostschritte als `BLOCKED`,
  nie als indirekt bestanden.

## Performance richtig messen

Für einen Block mit `n` Samples und Samplerate `f_s` ist die nominelle
Blockdeadline `n / f_s`. Speichere neben der absoluten Callbackzeit auch ihr
Verhältnis zu dieser Deadline. Kleine Blöcke verkürzen die reale Frist und
decken fixen Pro-Callback-Overhead auf.

Eine Messung nennt mindestens:

- Commit und unveränderten/dirty Quellstand;
- Buildtyp, Compiler und relevante Schalter;
- CPU, Betriebssystem, Energieprofil und Host;
- Samplerate, Blockgröße, Kanal-/Sidechainform und Instanzzahl;
- Signalzustand und ausgelöste Zustandskante;
- Warm-up, Laufdauer, Stichprobenzahl und Instrumentierung;
- Rohartefakt sowie Auswertungsversion.

Mittelwert und p95 sind Betriebsmetriken, kein Realtime-Beweis. Betrachte die
Verteilung und seltene Spitzen; unterscheide stationären DSP von Cold Start,
Banktausch, Bypass, Reset, Automation, Reconnect und Teardown. Ein Profiler kann
Scheduling und Cache verändern, deshalb leichte In-Process-Telemetrie gegen
eine zweite Messmethode plausibilisieren.

## Baselines und Ratchet

- Starte mit gemessener Realität, nicht mit einem Wunschwert.
- Binde jede Baseline an Umgebung, Lastprofil, Metrikversion und Rohdaten.
- Vergleiche Gleiches mit Gleichem; ein Hardware- oder Buildprofilwechsel
  erzeugt eine neue Vergleichsreihe.
- Verriegle bestätigte Verbesserungen nach oben beziehungsweise Budgets nach
  unten.
- Eine Lockerung ist eine sichtbare Qualitätsänderung und braucht Messung,
  Grund und Userwirkung.
- Neue Phasen aktivieren neue Gates. Noch nicht gebaute Prüfpfade bleiben
  `NOT RUN`, nicht grün.

## Lernbeförderung

Ein Befund wird erst dauerhaftes Skillwissen, wenn Source und reproduzierbares
Verhalten ihn bestätigen.

| Ereignis | Dauerhafte Reaktion |
|---|---|
| bestätigter Funktionsbug | minimales Fixture, Regressionstest, bei wiederkehrender Klasse Gate |
| Performance-Regression | reproduzierbarer Benchmark, Rohdaten, Baseline und Ursache |
| Fehlalarm | Gegenbeispiel bewahren, Detektor enger machen |
| fehlende Sichtbarkeit | Instrumentierung bauen, erst danach Schwelle setzen |
| neue allgemeine Fehlerklasse | beste Primärquelle prüfen und eingeordnet in `wissen/` dokumentieren |
| verbesserter Messwert | identischen Lauf wiederholen, dann Ratchet anheben |

Der Skill darf aus einem einzelnen Beispiel keine universelle Regel machen und
ändert sich nie automatisch aus seinem eigenen Bericht. Jede Änderung an Skill,
Skript, Test oder Baseline braucht einen eigenen überprüften Diff.

## Ergebnisformat

Der Qualitätsbericht enthält:

1. Scope, Profil, Revision und Umgebung.
2. Pro Gate: Behauptung, Messweg, Status, Rohbeleg und Konsequenz.
3. Bestätigte Ursachen und ausgeführte Fixes.
4. Nicht gelaufene oder blockierte Beweise ohne Grünfärbung.
5. Baseline-Deltas und ihre Vergleichbarkeit.
6. Lernbeförderungen: Test, Gate, Benchmark, Wissen oder bewusst keine.

Keine Gesamtnote. Eine Zahl wie „92/100“ verschleiert, welche harte
Produkteigenschaft ungeprüft oder gebrochen ist.
