---
name: nakama-plugin-quality
description: "Audit and improve Nakama plugin correctness, realtime safety, performance, scaling, stability, and maintainability with fresh measurable evidence. Use for quality checks, performance regressions, multi-instance stress, release readiness, or turning confirmed failures into durable guards; not for ordinary feature work without a quality concern."
---

# Nakama Plugin Quality

Behandle Qualität als messbaren Produktvertrag, nicht als Stilnote. Ein grüner
Befund braucht aktuelle Belege aus dem echten Produktionspfad; ein KI-Review
liefert zunächst nur Verdachtsstellen.

## Einstieg

1. Lies `AGENTS.md` und `CLAUDE.md` vollständig und prüfe `git status --short`.
2. Führe den read-only Preflight aus:

   ```powershell
   & .\.agents\skills\nakama-plugin-quality\scripts\quality-preflight.ps1 `
     -WorkingDirectory (Get-Location).Path -Format Json
   ```

   Nutze bei einer Prüfung gegen einen früheren Stand zusätzlich `-BaseRef
   <ref>`. Der Preflight fetcht und verändert nichts. Fehlt `pwsh`, ist der
   kanonische Runner `BLOCKED`; starte `tools/beweise.ps1` nicht ersatzweise
   unter Windows PowerShell 5.1.

   Nach Änderungen am Preflight dessen Vertrag mit
   `scripts/quality-preflight-selftest.ps1` prüfen.
3. Lies den tatsächlichen Diff und danach nur die betroffene Implementierung,
   Tests, Aufrufer und Gegenwege. Code und Belege der laufenden Session gehen
   Plan, Doku und älteren Audits vor.
4. Lies [references/quality-model.md](references/quality-model.md), wenn Profil,
   Messgrößen, Mehrinstanzlast oder die Beförderung eines Befunds zu bestimmen
   sind.

## Profil wählen

Wähle das kleinste Profil, das den realen Schaden falsifizieren kann:

- **quick**: lokale, klar begrenzte Änderung ohne Audio-, Thread-, Vertrags-,
  State-, Installations- oder Lebenszykluswirkung.
- **deep**: DSP, Audiothread, Analyse, IPC, Broker, State, Automation,
  Nebenläufigkeit, Ressourcenbesitz oder mehrere gekoppelte Module.
- **soak**: Skalierung, Queue/Backpressure, Reconnect, Reload, Leaks,
  Scheduling oder seltene Laufzeitspitzen.
- **release**: Auslieferungsbehauptung, echte FL-Hostmatrix, Installer,
  Rollback, Signatur oder vollständige Produktlast.

Ein kleiner Diff ist kein Grund für `quick`, wenn sein möglicher Schaden groß
ist. Speicher↔Laden, Start↔Stop, Verbinden↔Trennen, Aktivieren↔Abklingen und
Installieren↔Rückweg immer gemeinsam prüfen.

## Prüfen und verbessern

1. Formuliere pro Risiko eine falsifizierbare Behauptung und den Messweg.
2. Nutze vorhandene Produktionspfade und Prüfziele. Kopiere keine
   Implementierung in einen Test und erfinde keine Zielnamen oder Ticketpfade.
3. Führe zuerst gezielte Prüfungen aus. Bei breitem oder releasekritischem
   Risiko kommt der kanonische Runner aus `tools/beweise.ps1` hinzu. Sein
   Manifestziel muss aus dem echten Auftrag oder Plan stammen.
4. Miss Realtime als Deadline- und Tail-Problem. Durchschnitt, p95 oder eine
   warme Einzelinstanz reichen nicht als Stabilitätsbeweis.
5. Prüfe den Produktfall **ein Gen plus zehn Probeeq auf Bussen** ausdrücklich,
   sobald die betroffene Phase ihn ausführbar macht. Ergänze die im aktuellen
   Sondenplan verlangte Skalierungsmatrix; ersetze sie nicht.
6. Validiere jeden KI-Befund an Source und Verhalten. Bei einem bestätigten,
   fixbaren Fehler im beauftragten Bereich: Ursache beheben, Gegenbeweis bauen
   und erneut messen. Bei einem Fehlalarm: Gegenbeweis festhalten und die Regel
   enger machen.
7. Lies nach dem Fix den eigenen Diff adversarial: NaN/Inf/Denormal,
   Zahlenränder, kleine und übergroße Blöcke, stale Zustände,
   Save/Load-Symmetrie, Nebenläufigkeit, Ressourcenfreigabe und irreführende
   Zustandsmeldungen.

## Befundstatus

Verwende nur diese Bedeutungen:

- `PASS`: in dieser Session auf dem behaupteten Quellstand gemessen.
- `FAIL`: reproduzierbarer Gegenbeweis oder gefallenes Budget.
- `BLOCKED`: erforderlicher Beweis ist wegen einer konkret benannten externen
  Voraussetzung nicht ausführbar.
- `NOT RUN`: nicht ausgeführt; niemals als grün zusammenfassen.

Gebaut, getestet, abgenommen und releasefähig sind verschiedene Zustände. Alte
Logs, statische Inspektion oder ein vorhandenes Prüfziel beweisen keinen
aktuellen Lauf.

## Lernen ohne Selbstdrift

Der Skill schreibt sich nicht aufgrund seiner eigenen Behauptungen automatisch
um. Befördere nur verifizierte Erkenntnisse:

- bestätigter Bug → kleinstes reproduzierendes Fixture, Regressionstest und
  bei Wiederholungsgefahr stehendes Gate;
- Performancefund → reproduzierbarer Benchmark mit Umgebung und Rohdaten;
- Fehlalarm → Gegenbeispiel und engere Regel;
- blinder Fleck → zuerst Instrumentierung, danach Schwelle;
- neue, wiederverwendbare Fehlerklasse → Primärquelle prüfen und in `wissen/`
  einordnen, ohne dort Produktentscheidungen zu erfinden.

Baselines dürfen strenger werden. Eine Lockerung braucht Messung, Grund und
sichtbare Auswirkung. Änderungen am Skill, seinen Skripten oder Baselines sind
normale versionierte Codeänderungen und werden selbst validiert.

## Abschluss

Berichte kompakt:

- Scope, Profil, Revision und relevante Umgebung;
- Behauptung, Messweg, Ergebnis und Beleg je Gate;
- behobene Ursachen und neu gesetzte Schutzwirkung;
- `BLOCKED`/`NOT RUN` mit Konsequenz;
- bestätigte Lernbeförderungen oder bewusst keine.

Keine Gesamtnote und kein grünes Gesamturteil, wenn ein erforderliches Gate
nicht gelaufen ist.
