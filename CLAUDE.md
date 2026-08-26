# NAKAMA

Plugin-Familie für FL Studio unter Windows 11 (JUCE 8/C++20, CMake) mit
eigenständigem Rust-Broker über Named Pipes. Seit 23.08.2026 besteht Nakama aus
zwei Apps mit einer Design-Identität: **Nakama Gen** ist Hub/Studio und
EQ-Zentrale; **Nakama Probeeq** ist die Sonde auf den Bussen und führt den
zuschaltbaren EQ aus. Suna ist in Probeeq aufgegangen. Der Bundle-Name lautet
**Nakama Studio**. Legacy-Bezeichner wie `EQ-Copilot`, `EqCop*` und `Eqcp`
bleiben bis zum eigenen Identitäts-Ticket NAK-30 bestehen.

Das Repo ist eigenständig und privat. `design/` gehört seit 22.08.2026 zum
selben Repo. Parallele Sessions sind normal: fremde uncommittete Änderungen nie
anfassen, eigene Commits nur mit explizitem Pathspec, nie `git add -A` und nie
`--amend`. Vor neuer Arbeit nur bei sicherem Worktree pullen. Committen und
Pushen sind für dieses Projekt ausdrücklich autorisiert; eigene logische
Commits gehen ohne Rückfrage raus.

<!-- WAHRHEITSKERN:ANFANG — einzige kompakte Kopie; der Primer liest diesen
     Block ausschließlich nach einer Context-Compaction. -->
## Wahrheitskern

- **Produkt:** Gen zeigt Quellen, Befunde und Advisor. Seite 2 ist die
  EQ-Zentrale für alle Sonden plus vollwertigem Master-EQ. Die Sonden werden
  direkt dort durchgeschaltet; zwei EQ-Spuren liegen farblich unterscheidbar im
  selben Graph. Gen Seite 1 und 2 sind beide 760×430. Probeeq misst auf den
  Bussen und führt dort seinen EQ aus; Gen fernbedient ihn.
- **Grundgesetz: nichts Ungefragtes.** Verarbeitung findet nur statt, wenn der
  User sie einschaltet. Ausgeschaltet ist der Pfad im Nulltest bitidentisch;
  sonst ist der Passthrough sampleidentisch, ohne Latenz oder Tail. Der Advisor
  schlägt nur vor. Audio-Thread: keine Sperren, Allokationen, Datei-, Pipe- oder
  Netz-Zugriffe und kein Logging; Überlast verwirft Analyseframes, nie Audio.
- **Keine KI-Erklärschicht** im Produkt. Der Advisor ist regelbasiert.
- **Aktuelle visuelle Wahrheit in progress:** Seit dem User-Entscheid vom
  25.08.2026 sind ausschließlich die drei Bilder unter
  `design/assets/rework-basis-2026-08-25/` Grundlage des großen Reworks.
  Wortlaut und Leseregel stehen in
  `design/abnahmen/2026-08-25-rework-referenz-drei-designs.md`. Frühere
  Figma-Nodes und Exporte sind Verlauf. Keine eigene Stilsuche.
  Produktsprache Englisch; Docs, Commits und Gespräch Deutsch.
- **Keine toten UI-Elemente.** Jedes sichtbare Element führt einen Handgriff
  aus oder meldet ehrlich einen Zustand.
- **Geparkt:** Material-Kit-Front ist ein nie abgenommenes Provisorium. Prisma,
  Hörkompass, Glas/Licht, Tiefenfeld, Bauplan 2.0, Kunstwerk und Feld-Alphabet
  sind Archiv oder Studie, nicht Produktvorgabe; nie ungefragt reaktivieren.
- **Entscheide:** Nur Datum plus User-Zitat in `design/abnahmen/` oder einem
  ausdrücklich bezeichneten Register machen eine Entscheidung verbindlich.
  Alles andere heißt Vorschlag, Annahme oder Studie.
- **Plan:** Der technische Entwurf ist
  `docs/FL-Nakama-Sonden-Design-Entwurf.md`; der aktuelle, aus dem Repo
  gerechnete Stand und die nächste Arbeit liegen in `docs/PLAN-STAND.md`.
  Danach werden nur Gate-Text, Fachquellen und Manifest des konkreten Tickets
  gelesen. Die frühere Briefing-Seite ist abgeschafft.
<!-- WAHRHEITSKERN:ENDE -->

## Arbeitsregeln

- Code und laufende Beweise sind die Wahrheit. Vor einer Änderung die
  betroffene Quelle lesen; Doku, Memory und Audits sind Hinweise.
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
  lesend.

## Lesen vor der Arbeit

| Bereich | Zuerst lesen |
|---|---|
| Nächster Schritt | `docs/PLAN-STAND.md`, danach die konkrete Ticketquelle |
| Planstand / offene Fragen | `docs/PLAN-STAND.md`, `docs/plan/plan.json`, `docs/plan/fragen.json`, `docs/plan/LIES-MICH.md` |
| Plugin heute | `docs/plugin-wissen.md` |
| Sondenfamilie / Phasen | `docs/FL-Nakama-Sonden-Design-Entwurf.md`, `docs/bauaufteilung-sonden.md` |
| Verträge v3 | `eq-copilot/schemas/v3/README.md`, `eq-copilot/schemas/v3/flatbuffers/README.md` |
| State / Migration | `eq-copilot/schemas/state/`, `eq-copilot/plugin/state/`, `eq-copilot/fixtures/state/` |
| Beweise | jüngstes passendes Manifest in `docs/beweise/` |
| FL-Capabilities | `eq-copilot/identity/host-capabilities-fl-v1.json`, `docs/beweise/termin-a/`, `docs/beweise/termin-b/` |
| App-Design | `design/LIES-MICH.md`, relevante Datei in `design/abnahmen/` und die drei aktuellen Rework-Bilder |
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
- `design/`: Figma-Übersetzung, Prototyp, Abnahmen und Designregeln.
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
Quellen sind. Einzelbefehle und aktuelle Zielnamen stehen im Skript; nicht aus
dieser Datei rekonstruieren. Editor-Sichtprüfung ohne FL läuft über
`EqCopShot.exe`. Installation ist ein bewusster Admin-Schritt des Users und
nicht Teil eines normalen Beweislaufs.

Broker-Ende-zu-Ende immer über den Probe-Pipenamen testen. Nie gleichzeitig
einen Testbroker auf der Produktions-Pipe starten. Vor einer Installation den
State-Migrationsstand und den letzten Installationsbeweis prüfen.

## Design-Arbeitsmodell

1. Die drei aktuellen Rework-Bilder und ihren bindenden Abnahmetext lesen;
   sie bestimmen Look und Identität in progress.
2. Strukturfragen zuerst als Skizze, bei Bedarf als 760×430-Graybox klären;
   erst danach in Figma ausarbeiten
   (`design/abnahmen/2026-08-25-skizzen-vor-figma.md`).
3. `design/` übersetzt die Vorgabe in funktionierende Blätter und
   dokumentiert Zustände, Größen und Grenzfälle. User-Wortlaut kommt nach
   `design/abnahmen/`.
4. Erst eine Abnahme erlaubt die Spiegelung in `eq-copilot/`.
5. `eq-copilot/design/` ist Altbestand/Studie. Aktive Arbeitsanweisung und Git
   schützen diese alte Fläche. Der normale Prototyp unter `design/prototyp/`
   ist frei; die frühere Prototyp-Schleuse ist seit 24.08.2026 stillgelegt.

## Maschinen-Landminen

- PowerShell `Start-Process -ArgumentList` quotiert Argumente mit Leerzeichen
  nicht automatisch.
- Bash-Heredocs können Backslashes in Windows-Pfaden verändern; Pipes können
  Exitcodes verdecken.
- Bytegleich geprüfte Fixtures und Patches brauchen passende
  `.gitattributes`-Regeln.
- Keine GPU-Batch-Render-Loops auf der Arc A770; Renderprüfung einzeln oder per
  CPU/Software.
