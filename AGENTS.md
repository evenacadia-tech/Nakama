# AGENTS.md — Nakama

Diese Datei gilt für das gesamte Repository. Eine tiefer liegende `AGENTS.md`
ergänzt oder überschreibt sie nur für ihren Teilbaum.

## Autorität und Einstieg

`CLAUDE.md` an der Repo-Wurzel ist der gemeinsame, kanonische Produkt- und
Arbeitskontext für Claude und Codex. Vor jeder Nakama-Arbeit die Datei vollständig
lesen. Diese Datei bleibt ein Arbeitsrouter und führt keine zweite Produktwahrheit.

Vor Änderungen:

1. `git status --short` prüfen und fremde oder unklare Änderungen markieren.
2. Nur die für den Auftrag passenden Quellen und Fachdateien lesen.
3. Bei fortgesetzter Planarbeit `py -3.13 tools/plan/planstand.py` ausführen und
   mit `docs/PLAN-STAND.md` beginnen; danach nur die dort genannte Ticketquelle
   lesen. Plandokumente laufen vor dem Commit durch
   `py -3.13 tools/plan/dokuriegel.py <datei>`; ein Verweis ins Leere ist ein Befund.
4. Umfang und passenden Beweisweg festlegen, bevor Code geändert wird.

Aktueller Code und Beweise aus der laufenden Session gehen beschreibender Doku vor.
Produktentscheidungen sind nur bindend, wenn sie entsprechend `CLAUDE.md` mit Datum
und User-Wortlaut dokumentiert sind. Archive und Studien sind Verlauf, keine Vorgabe.

## Aufgabenbezogen lesen

| Aufgabe | Verbindlicher Einstieg |
|---|---|
| Nächster geplanter Schritt | `docs/PLAN-STAND.md`, danach Gate-Text, Fachquelle und Manifest des konkreten Tickets |
| Planstand oder offene Fragen | `docs/PLAN-STAND.md`, `docs/plan/LIES-MICH.md` und die zugehörigen JSON-Dateien |
| Heutige Plugin-Architektur | `docs/plugin-wissen.md` |
| Sondenfamilie und Bauphasen | `docs/FL-Nakama-Sonden-Design-Entwurf.md`, `docs/bauaufteilung-sonden.md` |
| Verträge v3 | `eq-copilot/schemas/v3/README.md`, `eq-copilot/schemas/v3/flatbuffers/README.md` |
| State oder Migration | `eq-copilot/schemas/state/`, `eq-copilot/plugin/state/`, `eq-copilot/fixtures/state/` |
| Installation | `eq-copilot/install/` und der jüngste passende Installationsbeweis |
| UI, UX oder Plugin-Design | `design/LIES-MICH.md`, `design/docs/funktions-und-bedien-blueprint.md`, `design/skizze/LIES-MICH.md`, `design/visuell/LIES-MICH.md`, danach die jüngste passende Datei in `design/abnahmen/` |
| Ticketbeweis | das jüngste passende Manifest in `docs/beweise/` und `tools/beweise.ps1` |

Nicht vorsorglich den gesamten Dokumentationsbaum laden. Historische Dateien unter
`docs/archiv/`, `eq-copilot/design/archive/` und der Prisma-Studie nur zur
Spurensuche verwenden.

## Änderungsdisziplin

- Den kleinsten kohärenten Änderungssatz bauen; keine Neben-Refactors und keine
  Legacy-Umbenennungen ohne eigenen Auftrag.
- Vor dem Editieren die betroffene Implementierung und ihre Tests lesen. Bei
  Verhalten über Modulgrenzen auch die Aufrufbeziehungen prüfen.
- Gekoppelte Lebenszyklen gemeinsam behandeln: speichern/laden, starten/stoppen,
  öffnen/schließen, verbinden/trennen, aktivieren/abklingen und
  installieren/Rückweg.
- Persistente Formate und IPC sind Verträge. Writer, Reader, Migration, Fixtures und
  Cross-Language-Verbraucher im selben Änderungssatz berücksichtigen.
- Audio-Thread-Regeln, Nulltest, State-Verlustfreiheit und eingefrorene Identität aus
  `CLAUDE.md` sind harte Invarianten.
- Fixbare Fehler innerhalb des beauftragten Bereichs beheben und prüfen. Punkte
  außerhalb des Auftrags nicht nebenbei ändern; bei Relevanz datiert in
  `docs/offene-punkte.md` festhalten.
- Große technische Änderungen in `docs/plugin-wissen.md` nachziehen. Volatile
  Zahlen und Zielnamen aus Code, Manifest oder Runner lesen, nicht in Kontextdateien
  kopieren.

## Parallelbetrieb und Git

Parallele Sessions sind normal.

- Fremde uncommittete Änderungen weder verändern noch entfernen oder formatieren.
- Keine bereinigenden oder destruktiven Git-Befehle gegen den gemeinsamen Worktree.
- Vor Abschluss erneut `git status --short` und den eigenen Diff prüfen.
- Eigene Commits ausschließlich mit explizitem Pathspec erstellen; nie
  `git add -A`, nie `git commit --amend`.
- Nur bei sicher sauberem Worktree pullen. Eigene logische Commits und Pushes sind
  laut `CLAUDE.md` autorisiert; fremde Pfade bleiben aus dem Commit.

## Bauen, testen und beweisen

Die Prüfung muss zum Risiko passen. Zuerst gezielte Tests für den geänderten Bereich,
bei breiten oder releasekritischen Änderungen zusätzlich den kanonischen Beweislauf
vom Workspace-Root ausführen:

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-0NN.md -Anhaengen -Titel 'SONDE-0NN'
```

Ticket und Zielpfad nicht erfinden; aus Auftrag, Plan oder Runner ableiten. Alte Logs
sind kein Beleg für den aktuellen Stand. Nach den Tests den Diff adversarial auf
Grenzwerte, NaN/Inf, Nebenläufigkeit, stale Zustände, Save/Load-Symmetrie und
irreführende UI-Zustände lesen.

- Broker-E2E nur über den Probe-Pipenamen; nie einen Testbroker auf der
  Produktions-Pipe starten.
- Installation läuft seit 12.09.2026 ohne User-Handgriff über die erhöhten
  Aufgaben `\Nakama\installieren`, `\Nakama\pruefen`, `\Nakama\rueckweg`
  (`CLAUDE.md`, „Bauen und beweisen"); der Installer verweigert bei laufendem FL
  und bei Manifest-Hash-Abweichung.
- UI-Sichtprüfung ohne FL Studio über `EqCopShot.exe`; relevante Zustände und feste
  Fenstermaße visuell prüfen.
- Einen nicht ausführbaren Test mit Grund melden und einen kleineren, ehrlichen Beleg
  liefern; niemals Erfolg aus bloßer Codeinspektion behaupten.

## UI-, UX- und Figma-Arbeit — geparkt seit 12.09.2026

Der gesamte Designteil ruht (User-Wort 12.09.2026,
`design/abnahmen/2026-09-12-designteil-geparkt.md`), bis alles Technische
erledigt ist und beide Plugins reibungslos funktionieren: keine Designfragen,
keine Figma-, Skizzen- oder Blueprint-Arbeit, keine Spiegelung von
Designständen in den Produktcode. `design/LIES-MICH.md` beschreibt den Stand
beim Parken; `design/docs/funktions-und-bedien-blueprint.md` bleibt die
Beschreibung der Funktions- und Bedienarchitektur, die technische Schritte
zitieren. Weiter gültig für die vorhandene Oberfläche: sichtbare Elemente
führen einen Handgriff aus oder zeigen einen ehrlichen Zustand; keine geparkte
Designrichtung und keine eigene Stilsuche ungefragt reaktivieren.

## Sprache und Abschluss

Produktsprache ist Englisch; Doku, Commits und Gespräch sind Deutsch. Bestehende
Bezeichner im Code nicht beiläufig übersetzen.

Ein Abschlussbericht nennt knapp:

- geänderte Pfade und das erreichte Verhalten,
- ausgeführte Prüfungen mit Ergebnis,
- verbleibende Risiken, nicht gelaufene Prüfungen oder echte Blocker.

Fortschritt und Fertigstellung nur mit einem Beleg aus der laufenden Session melden.
Kanonische lokale Codex-Hilfswerkzeuge unter `tools/codex-plugins/` erhalten.
