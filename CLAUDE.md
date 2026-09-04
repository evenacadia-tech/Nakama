# NAKAMA

Plugin-Familie für FL Studio unter Windows 11 (JUCE 8/C++20, CMake) mit
eigenständigem Rust-Broker über Named Pipes. Seit 23.08.2026 besteht Nakama aus
zwei Apps mit einer Design-Identität: **Nakama Gen** hat zwei Oberflächen;
„Overview" und „EQ-Zentrale" sind dafür nur Arbeits- und Platzhalternamen.
**Nakama Probeeq** vereint die frühere Suna- und Probeeq-Rolle: Sie misst auf
dem Bus passiv und beliefert Gen; bei zugeschaltetem EQ führt dieselbe Instanz
Gens Fernsteuerung auf ihrem Bus aus. Suna ist als App-Name durch Probeeq
ersetzt. Der Bundle-Name lautet **Nakama Studio**. Legacy-Bezeichner wie
`EQ-Copilot`, `EqCop*` und `Eqcp` bleiben bis zum eigenen Identitäts-Ticket
NAK-30 bestehen.

Das Repo ist eigenständig und privat. `design/` gehört seit 22.08.2026 zum
selben Repo. Parallele Sessions sind normal: fremde uncommittete Änderungen nie
anfassen, eigene Commits nur mit explizitem Pathspec, nie `git add -A` und nie
`--amend`. Vor neuer Arbeit nur bei sicherem Worktree pullen. Committen und
Pushen sind für dieses Projekt ausdrücklich autorisiert; eigene logische
Commits gehen ohne Rückfrage raus.

<!-- WAHRHEITSKERN:ANFANG — einzige kompakte Kopie; der Primer liest diesen
     Block ausschließlich nach einer Context-Compaction. -->
## Wahrheitskern

- **Produkt:** Gen hat zwei Oberflächen. Die erste zeigt Quellen, Befunde und
  Advisor; die zweite bedient die Probeeq-Instanzen zentral und enthält den
  vollwertigen Master-EQ. „Overview" und „EQ-Zentrale" sind nur Arbeitsnamen,
  keine festgelegten Produktnamen. Die Sonden werden direkt auf der zweiten
  Oberfläche durchgeschaltet; zwei EQ-Spuren liegen farblich unterscheidbar im
  selben Graph. Beide Gen-Oberflächen arbeiten auf genau einer logischen
  Standardgröße von 950×538. Eine zusätzliche Compact-Fassung und ein
  größenabhängiges Layout sind bis nach Fertigstellung dieser Oberfläche
  vertagt. Die davon getrennte UI-Skalierung vergrößert dieselbe Geometrie,
  erzeugt aber keine zweite Layoutfassung. Probeeq misst auf den
  Bussen passiv und beliefert Gen. Wird ihr EQ zugeschaltet, führt dieselbe
  Instanz die von Gen ferngesteuerten Eingriffe auf ihrem Bus aus; Messsonde
  und EQ-Ausführer sind keine alternativen Produktrollen. Die vollständige
  EQ-Bedienung existiert nur in Gen; Probeeq erhält lokal keinen vollständigen
  EQ-Editor, sondern eine kompakte Status- und Rückfallfläche.
- **Grundgesetz: nichts Ungefragtes.** Verarbeitung findet nur statt, wenn der
  User sie einschaltet. Ausgeschaltet ist der Pfad im Nulltest bitidentisch;
  sonst ist der Passthrough sampleidentisch, ohne Latenz oder Tail. Der Advisor
  schlägt nur vor. Audio-Thread: keine Sperren, Allokationen, Datei-, Pipe- oder
  Netz-Zugriffe und kein Logging; Überlast verwirft Analyseframes, nie Audio.
- **Keine KI-Erklärschicht** im Produkt. Der Advisor ist regelbasiert.
- **Funktionsneustart der UI:** Seit dem ausdrücklichen User-Auftrag vom
  31.08.2026 wird die grundlegende UI-Architektur designneutral aus
  `design/docs/funktions-und-bedien-blueprint.md` und den neuen datierten
  Abnahmen abgeleitet. Frühere Figma-Nodes, Exporte und die drei Rework-Bilder
  vom 25.08. sind dafür Verlauf und liefern weder Raster noch Farben, Material
  oder Positionen. Eine neue visuelle Richtung entsteht erst in einem eigenen
  User-Schritt; sie wird dann in Figma entwickelt, sobald das Fundament aus
  Layout und Skizze steht (User-Wort 02.09.2026, Fünferblock 03). **Dieser
  Schritt ist am 02.09.2026 erfolgt:** Die visuelle Phase läuft in der
  Figma-Datei `fable-dummy` (Key `DvMbHg0MWCPwibDj8q6hI8`, Seite „01 — Nakama
  950×538 · Skizze in Materialsprache"), in der Materialsprache der Entwürfe
  vom 20. bis 22.08.2026 (Unibody, Glas, Kinn, Apertur, Geist, Wein, Cyan nur
  als Saum) und mit der Wortmarke im rot-blauen Verlauf auf beiden Apps. Die
  Skizze bleibt Layout-Wahrheit, Figma ist die visuelle Wahrheit; ein
  Figma-Stand ist ein Vorschlag bis zur datierten Abnahme
  (`design/abnahmen/2026-09-02-wechsel-visuelle-phase-figma.md`, Leseblatt
  `design/visuell/LIES-MICH.md`). Produktsprache
  Englisch; Docs, Commits und Gespräch Deutsch.
- **Aktueller UI-Arbeitsmodus:** Pro User-Schritt genau eine echte offene
  Architekturfrage. Technische Grenzen vorher selbst aus Code, Tests und
  Verträgen ableiten. Antworten werden in festen Fünferblöcken gesammelt;
  nach der fünften werden Skizze, Abnahmen und Blueprint gemeinsam
  aktualisiert und sichtbar geprüft. Der Wortlaut und aktuelle Stand stehen in
  `design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md`.
  Die technische Skizze liegt ausschließlich in `design/skizze/` und ist der
  derzeitige Stand, nicht der finale: Design ist ein laufender Prozess wie
  die Implementierung (User-Wort 02.09.2026). Skizze und Blueprint werden
  nie als fertig oder abgenommen bezeichnet; bindend sind nur die datierten
  Abnahmen.
- **Keine toten UI-Elemente.** Jedes sichtbare Element führt einen Handgriff
  aus oder meldet ehrlich einen Zustand.
- **Tasten sind Material.** User-Gesetz 25.08.2026: „ein button ist ein
  material das niemals einfach die breite verändert". Zustände wie hover,
  pressed, selected oder disabled wechseln Fläche, Schatten, Farbe oder
  Transform, nie die Maße; Beweis am gemessenen Layoutrechteck (bei
  skalierter Bühne `offsetWidth`/`offsetHeight`, nicht das transformierte
  `getBoundingClientRect`).
- **Geparkt:** Material-Kit-Front ist ein nie abgenommenes Provisorium. Prisma,
  Hörkompass, Glas/Licht, Tiefenfeld, Bauplan 2.0, Kunstwerk und Feld-Alphabet
  sind Archiv oder Studie, nicht Produktvorgabe; nie ungefragt reaktivieren.
- **Entscheide:** Nur Datum plus User-Zitat in `design/abnahmen/` oder einem
  ausdrücklich bezeichneten Register machen eine Entscheidung verbindlich.
  Alles andere heißt Vorschlag, Annahme oder Studie.
- **Plan:** Es gibt **zwei** lebende Spezifikationen, seit PR2 (02.09.2026)
  mit fester Rangfolge. `docs/FL-Nakama-Sonden-Design-Entwurf.md` (Fassung 0.5)
  bindet **Technik, Verträge, Phasen und Falsifikation**;
  `design/docs/funktions-und-bedien-blueprint.md` bindet die **Funktions- und
  Bedienarchitektur beider Apps**. **Über beiden stehen der aktuelle Vertrag und
  der Code** — widerspricht eine Spezifikation dem gebauten Vertrag, gilt der
  Vertrag. Ein Widerspruch zwischen beiden wird als Zeile in
  `docs/offene-punkte.md` geführt, nie kaschiert. Der aktuelle, aus dem Repo
  gerechnete Stand und die nächste Arbeit liegen in `docs/PLAN-STAND.md`.
  Danach werden nur Gate-Text, Fachquellen und Manifest des konkreten Tickets
  gelesen. Die frühere Briefing-Seite ist abgeschafft.
<!-- WAHRHEITSKERN:ENDE -->

## Arbeitsregeln

- Code und laufende Beweise sind die Wahrheit. Vor einer Änderung die
  betroffene Quelle lesen; Doku, Memory und Audits sind Hinweise.
- Zu Sessionbeginn und nach jedem abgeschlossenen Ticket
  `py -3.13 tools/plan/planstand.py` laufen lassen; kein Hook tut das
  automatisch. Ändert sich `docs/PLAN-STAND.md` aus bereits committeten
  Quellen, wird nur diese Datei mit Pathspec committet und gepusht
  (`docs/plan/LIES-MICH.md`). Plandokumente — Register, Plan, Abnahmen,
  Blueprint, Leseblätter, diese Datei — laufen vor dem Commit durch
  `py -3.13 tools/plan/dokuriegel.py <datei>` (Tabellen- und Verweisriegel,
  seit PR2 02.09.2026); ein Verweis ins Leere ist ein Befund, kein Stil.
- Beziehungen mitprüfen: speichern↔laden, starten↔stoppen,
  öffnen↔schließen, verbinden↔trennen, aktivieren↔abklingen und
  installieren↔Rückweg gehören jeweils in denselben Änderungssatz.
- Keine Neben-Refactors und keine Legacy-Umbenennung nebenbei. Fixbare Fehler
  im beauftragten Bereich werden behoben und geprüft, nicht nur aufgelistet.
- Fortschritt erst nach einem Beleg aus der laufenden Session behaupten.
  Danach Diff adversarial gegen Zahlenränder, NaN/Inf, stale Closures,
  Save/Load-Symmetrie und irreführende UI-Texte lesen.
- Volatile Zahlen wie Test-, Fixture-, IPC- oder Capability-Anzahlen nie hier
  festschreiben. Aus Code, Manifest oder dem jüngsten Beweis lesen.
- Große technische Schritte ziehen `docs/plugin-wissen.md` nach. Offene
  außerhalb des Auftrags liegende Produktpunkte gehen datiert nach
  `docs/offene-punkte.md`.
- Im Dirigentenbetrieb schreibt genau ein Hintergrundworker im sichtbaren
  Checkout. `worktree.bgIsolation = "none"` ist nur zusammen mit dieser
  Ein-Schreiber-Regel zulässig; Fable bleibt währenddessen bei Repo-Dateien
  lesend. Muss eine zweite Session währenddessen Repo-Dateien ändern
  (Design, Doku), arbeitet sie in einer eigenen lokalen Arbeitskopie
  (`git clone --local -c core.longpaths=true` an einen kurzen Pfad, kein
  `git worktree`, das Cockpit meldet Worktrees als Störung), committet dort
  mit Pathspec und pusht nach origin; der sichtbare Checkout holt den Stand
  beim nächsten Pull des Dirigenten. Screenshots und andere temporäre
  Dateien nie in den Repo-Root legen.
- Sicherheitsarbeit (Reviews, Audits, Härtung an Pipes, Impersonation,
  Signaturen) wird nie im Gesprächs- oder Dirigentenkontext selbst
  ausgeführt, sondern an Worker, Codex oder Review-Skills delegiert
  (User-Wort 01.09.2026).
- Der User ist Projektleiter und Musiker, kein Programmierer. Technische
  Wege (Crates, Schemas, Tests, Pfade, Werkzeuge) entscheidet Claude selbst
  und legt sie nie als Menü vor; dem User werden nur Produktwirkung sowie
  Design- und Produktfragen vorgelegt.
- Rechnerwechsel: Der User arbeitet an PC und Laptop. Memory und die
  globale `~/.claude/CLAUDE.md` sind rechnerlokal. Alles, was eine Session
  auf dem anderen Rechner braucht, steht in dieser Datei, im Skill oder in
  einer Repo-Datei und ist gepusht. Ein Artefakt, das nur in einem
  Werkzeugordner liegt (`~/.codex/visualizations/`, `.playwright-mcp/`,
  Scratchpad), gilt nicht als geliefert. Beide Rechner committen parallel
  auf `master` (am 02.09.2026 lief PR2 auf dem PC, während der Laptop die
  Design-Dokumente nachzog): Sessionstart mit `git pull --ff-only`, vor
  jedem Push bei sauberem Worktree `git pull --rebase`; kein Hook pusht
  automatisch, der Push ist ein eigener Schritt.

## Lesen vor der Arbeit

| Bereich | Zuerst lesen |
|---|---|
| Nächster Schritt | `docs/PLAN-STAND.md`, danach die konkrete Ticketquelle |
| Planstand / offene Fragen | `docs/PLAN-STAND.md`, `docs/plan/plan.json`, `docs/plan/fragen.json`, `docs/plan/LIES-MICH.md` |
| Plugin heute | `docs/plugin-wissen.md` |
| Sondenfamilie / Phasen | `docs/FL-Nakama-Sonden-Design-Entwurf.md` (Fassung 0.5 — bindet Technik, Verträge, Phasen, Falsifikation), `docs/bauaufteilung-sonden.md` |
| Verträge v3 | `eq-copilot/schemas/v3/README.md`, `eq-copilot/schemas/v3/flatbuffers/README.md` |
| State / Migration | `eq-copilot/schemas/state/`, `eq-copilot/plugin/state/`, `eq-copilot/fixtures/state/` |
| Beweise | jüngstes passendes Manifest in `docs/beweise/` |
| FL-Capabilities | `eq-copilot/identity/host-capabilities-fl-v1.json`, `docs/beweise/termin-a/`, `docs/beweise/termin-b/` |
| App-Design | `design/LIES-MICH.md`, `design/docs/funktions-und-bedien-blueprint.md` (bindet Funktions- und Bedienarchitektur beider Apps), `design/skizze/LIES-MICH.md` (laufende technische Skizze), `design/visuell/LIES-MICH.md` (visuelle Phase in Figma seit 02.09.2026) und die jüngste passende Datei in `design/abnahmen/` |
| Widerspruch Entwurf ↔ Blueprint | über beiden stehen aktueller Vertrag und Code; der Widerspruch wird als Zeile in `docs/offene-punkte.md` geführt, nie kaschiert (PR2, 02.09.2026) |
| Externes Wissen | `wissen/INDEX.md`, danach der passende Wissensbereich |
| Verlauf, nie Vorgabe | `docs/archiv/`, `eq-copilot/design/archive/`, `eq-copilot/design/prisma-studie/STATUS.md` |

Die vollständige frühere Daueranweisung liegt als historischer Snapshot in
`docs/archiv/CLAUDE-kontext-vor-bereinigung-2026-08-24.md`. Sie dient nur zur
Spurensuche; aktuelle Source, Beweise und die oben verlinkten Fachdateien gehen
vor.

## Repo-Karte

- `eq-copilot/plugin/`: JUCE-Produktcode, Hostbrücke, Tests und Werkzeuge.
- `eq-copilot/schemas/`: heutige v2-Verträge; `schemas/v3/`: Sondenfamilie.
- `eq-copilot/identity/`: eingefrorene Identität und Host-Capabilities.
- `eq-copilot/install/`: manifestgetriebener Installer und Rückweg.
- `broker/`: eigenständiger Rust-Broker `eqcop-broker.exe`.
- `tools/beweise.ps1`: kanonischer lokaler Beweis-Runner.
- `design/`: Übersetzung der aktuellen User-Vorgabe, laufende technische
  Skizze (`skizze/`), visuelle Phase in Figma (`visuell/`), Abnahmen,
  Designregeln und der historische Web-Prototyp
  (`prototyp/`, Verlauf).
- `wissen/`: eingeordnetes externes Wissen und visuelle Belege; keine
  parallelen Produktentscheide und keine Roh-PDFs.
- `docs/beweise/`: rohe, ticketspezifische Belegmanifeste.

## Tragende technische Invarianten

- **Schemas sind Verträge.** Neue persistente Felder zuerst versionieren;
  Altstände laden; unbekannte Felder dürfen alte Consumer nicht zerstören.
  Save und Load gemeinsam testen. Für v3 sind Discriminator, Zieladresse,
  Revision und Capability nicht additiv erweiterbar.
- **Engine kennt keine Optik.** Sie liefert kohärente Mess-Snapshots; der
  Editor hält Anzeigezustand und rendert nur neue Snapshot-Revisionen.
- **Audio bleibt echtzeitfest.** Steuerung über Pipe oder UI wird außerhalb
  des Audio-Threads übernommen; Zustands- und Parameterhoheit bleibt in der
  Audio führenden Instanz.
- **Zeit ist aktive Musikzeit.** Zonen-Ticks sind deterministisch und durch
  Goldens beweisbar.
- **NaN-Ehrlichkeit.** Nicht-endliche Werte werden verriegelt und gezählt;
  Nyquist wird gekappt; ohne genügend endliche Nachbarn gibt es keine
  Basislinie.
- **Begriffe nicht vermischen:** Probe-Pipe ist nie Produktions-Pipe;
  `ltasReferenzDb` ist keine globale Sollkurve; Paint-FPS ist nicht
  Datenkadenz; Demo-Daten sind keine Plugin-Daten; FL zeigt MIDI 60 als C5.
- **State bleibt verlustfrei.** Unbekannte Major-Versionen oder unzulässige
  Matrizen werden read-only mit Originalbytes gehalten. Jede persistente
  Änderung meldet dem Host Dirty-State.
- **Identität bleibt eingefroren**, bis NAK-30 sie bewusst migriert.
  Class-IDs, Hersteller-/Plugin-Codes oder Replace-V2-Verhalten nie beiläufig
  ändern.

## Bauen und beweisen

Der kanonische Komplettlauf vom Workspace-Root ist:

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-0NN.md -Anhaengen -Titel 'SONDE-0NN'
```

Der Runner baut die benötigten Ziele, fährt die deklarierten Beine, speichert
rohe Ausgabe und verweigert eine Beglaubigung, wenn Prüfbinaries älter als ihre
Quellen sind. Die Rohausgaben liegen seit NAK-96 unter `docs/beweise/roh/`; das
Manifest trägt nur Kopf, Urteilszeile und Übersicht mit Verweis dorthin.
Einzelbefehle und aktuelle Zielnamen stehen im Skript; nicht aus
dieser Datei rekonstruieren. Editor-Sichtprüfung ohne FL läuft über
`EqCopShot.exe`. Installation ist ein bewusster Admin-Schritt des Users und
nicht Teil eines normalen Beweislaufs.

Broker-Ende-zu-Ende immer über den Probe-Pipenamen testen. Nie gleichzeitig
einen Testbroker auf der Produktions-Pipe starten. Vor einer Installation den
State-Migrationsstand und den letzten Installationsbeweis prüfen.

## Design-Arbeitsmodell

1. Für die laufende Funktionsarchitektur den Blueprint und
   `design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md`
   lesen. Frühere visuelle Entwürfe sind in dieser Phase keine Vorgabe.
2. Pro Oberfläche genau eine noch offene Frage zu Aufbau, Hierarchie,
   Sichtbarkeit, Objektbesitz oder Disclosure stellen. Funktionsumfang und
   technische Grenzen vorher selbst aus aktuellen Quellen klären.
3. Antworten in einem laufenden Fünferblock sammeln und den User-Wortlaut in
   dessen Entscheidungsprotokoll sichern. Das Protokoll des nächsten Blocks
   (`design/abnahmen/<datum>-technische-ui-architektur-fuenferblock-NN.md`)
   entsteht mit der ersten beantworteten Frage, nie leer vorab. Liegt die
   Frage bereits als Karte in `docs/plan/fragen.json`, wird sie mit
   `/fragen` gestellt; der Wortlaut geht dann zusätzlich in das
   Blockprotokoll. Nach der fünften Antwort die
   technische Skizze (`design/skizze/nakama-ui-technical-sketch.html`, im
   Repo, nie nur in einem Werkzeugordner), die betroffenen Abnahmen und den
   Blueprint gemeinsam aktualisieren, sichtbar prüfen (Screenshot nach
   `design/skizze/belege/`) und committen; bereits Geschlossenes nicht
   erneut öffnen.
4. Farben, Material, Typografie und visuelle Feinheiten entstehen seit dem
   ausdrücklichen Wechsel vom 02.09.2026 in Figma (`fable-dummy`, Leseblatt
   `design/visuell/LIES-MICH.md`); jeder Figma-Stand wird per Export unter
   `design/visuell/belege/` belegt und bleibt Vorschlag. Erst eine
   dokumentierte Abnahme erlaubt die Spiegelung in `eq-copilot/`.
5. `eq-copilot/design/`, frühere Figma-Stände und die drei Rework-Bilder sind
   Verlauf/Studie. Sie dürfen zur Spurensuche dienen, aber die neue
   Funktionsarchitektur nicht still ergänzen.

## Maschinen-Landminen

- PowerShell `Start-Process -ArgumentList` quotiert Argumente mit Leerzeichen
  nicht automatisch.
- Bash-Heredocs können Backslashes in Windows-Pfaden verändern; Pipes können
  Exitcodes verdecken.
- Bytegleich geprüfte Fixtures und Patches brauchen passende
  `.gitattributes`-Regeln.
- Keine GPU-Batch-Render-Loops auf der Arc A770; Renderprüfung einzeln oder per
  CPU/Software.
- Playwright-MCP lädt keine `file://`-Adressen und legt relative
  Screenshot-Pfade im Repo-Root ab: lokalen HTTP-Server im Zielordner
  starten (`py -3.13 -m http.server <port> --bind 127.0.0.1`), Bilder sofort
  in den Zielordner verschieben, danach `git status --short` lesen.
- Codex legt Visualisierungen unter `~/.codex/visualizations/` ab, nicht im
  Repo. Nach Codex-Design-Arbeit prüfen, dass jedes in Abnahmen referenzierte
  Artefakt committet ist (`grep -rn "codex.visualizations\|127.0.0.1" design/`).
- Ein `git clone` dieses Repos braucht `-c core.longpaths=true` und einen
  kurzen Zielpfad, sonst bricht der Checkout unter `tools/codex-plugins/` mit
  „Filename too long" ab.
- Der MSVC-Standardstack ist 1 MiB: Testfunktionen, die `FeatureEngine`-Objekte
  (rund 0,5 MB) oder ganze Prozessoren im Rahmen anlegen, reißen ihn — in
  SONDE-013 dreimal, jedes Mal an einem neuen Feld unter 2 KiB. Engines in
  Tests auf den Heap (`std::unique_ptr`); der Fund zeigt sich erst im vollen
  Kanon, das betroffene Bein allein läuft grün (Register NAK-175).
