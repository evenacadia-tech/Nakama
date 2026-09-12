# NAKAMA

Plugin-Familie für FL Studio unter Windows 11 (JUCE 8/C++20, CMake) mit
eigenständigem Rust-Broker über Named Pipes. Seit 23.08.2026 zwei Apps mit
einer Design-Identität: **Nakama Gen** (zwei Oberflächen; „Overview" und
„EQ-Zentrale" sind Arbeitsnamen) und **Nakama Probeeq** (frühere Suna- und
Probeeq-Rolle: misst passiv auf dem Bus und beliefert Gen; mit zugeschaltetem
EQ führt dieselbe Instanz Gens Fernsteuerung auf ihrem Bus aus). Bundle-Name
**Nakama Studio**. Legacy-Bezeichner `EQ-Copilot`, `EqCop*`, `Eqcp` bleiben
bis NAK-30.

Repo eigenständig und privat; `design/` gehört seit 22.08.2026 dazu.
Parallele Sessions sind normal: fremde uncommittete Änderungen nie anfassen,
Commits nur mit explizitem Pathspec, nie `git add -A`, nie `--amend`. Pullen
nur bei sicherem Worktree. Committen und Pushen sind autorisiert; eigene
logische Commits gehen ohne Rückfrage raus.

## Wahrheitskern

- **Produkt:** Gens erste Oberfläche zeigt Quellen, Befunde und Advisor; die
  zweite bedient die Probeeq-Instanzen zentral und enthält den vollwertigen
  Master-EQ; Sonden werden dort direkt durchgeschaltet, zwei EQ-Spuren liegen
  farblich unterscheidbar im selben Graph. Beide Oberflächen haben genau eine
  logische Standardgröße 950×538; Compact-Fassung und größenabhängiges Layout
  sind vertagt; UI-Skalierung vergrößert dieselbe Geometrie. Probeeq misst
  passiv und beliefert Gen; Messsonde und EQ-Ausführer sind keine
  alternativen Rollen. Die vollständige EQ-Bedienung existiert nur in Gen;
  Probeeq hat lokal nur eine kompakte Status- und Rückfallfläche.
- **Grundgesetz: nichts Ungefragtes.** Verarbeitung nur, wenn der User sie
  einschaltet. Ausgeschaltet ist der Pfad im Nulltest bitidentisch; sonst
  Passthrough sampleidentisch, ohne Latenz oder Tail. Der Advisor schlägt nur
  vor. Audio-Thread: keine Sperren, Allokationen, Datei-, Pipe- oder
  Netzzugriffe, kein Logging; Überlast verwirft Analyseframes, nie Audio.
- **Keine KI-Erklärschicht** im Produkt; der Advisor ist regelbasiert.
- **UI-Funktionsneustart (User 31.08.2026):** Die UI-Architektur wird
  designneutral aus `design/docs/funktions-und-bedien-blueprint.md` und den
  datierten Abnahmen abgeleitet; frühere Figma-Nodes, Exporte und die drei
  Rework-Bilder vom 25.08. sind Verlauf. Die visuelle Phase läuft seit
  02.09.2026 in der Figma-Datei `fable-dummy` (Key `DvMbHg0MWCPwibDj8q6hI8`,
  Seite „01 — Nakama 950×538 · Skizze in Materialsprache") in der
  Materialsprache der Entwürfe vom 20.–22.08.2026 (Unibody, Glas, Kinn,
  Apertur, Geist, Wein, Cyan nur als Saum) mit der Wortmarke im rot-blauen
  Verlauf. Skizze ist Layout-Wahrheit, Figma visuelle Wahrheit; ein
  Figma-Stand ist Vorschlag bis zur datierten Abnahme
  (`design/abnahmen/2026-09-02-wechsel-visuelle-phase-figma.md`,
  `design/visuell/LIES-MICH.md`). Produktsprache Englisch; Docs, Commits,
  Gespräch Deutsch.
- **UI-Arbeitsmodus:** pro User-Schritt genau eine offene Architekturfrage;
  technische Grenzen vorher aus Code, Tests und Verträgen ableiten; Antworten
  in Fünferblöcken, nach der fünften Skizze, Abnahmen und Blueprint gemeinsam
  aktualisieren und sichtbar prüfen
  (`design/abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md`).
  Die technische Skizze liegt nur in `design/skizze/` und ist laufender
  Stand; Skizze und Blueprint heißen nie fertig oder abgenommen — bindend
  sind nur datierte Abnahmen.
- **Keine toten UI-Elemente** (User 24.08.2026): jedes sichtbare Element
  führt einen Handgriff aus oder meldet ehrlich einen Zustand.
- **Tasten sind Material** (User 25.08.2026): Zustände wechseln Fläche,
  Schatten, Farbe oder Transform, nie die Maße; Beweis am gemessenen
  Layoutrechteck (bei skalierter Bühne `offsetWidth`/`offsetHeight`, nicht
  `getBoundingClientRect`).
- **Geparkt:** Material-Kit-Front ist ein nie abgenommenes Provisorium.
  Prisma, Hörkompass, Glas/Licht, Tiefenfeld, Bauplan 2.0, Kunstwerk und
  Feld-Alphabet sind Archiv oder Studie; nie ungefragt reaktivieren.
- **Entscheide:** Nur Datum plus User-Zitat in `design/abnahmen/` oder einem
  ausdrücklich bezeichneten Register macht eine Entscheidung verbindlich;
  alles andere ist Vorschlag, Annahme oder Studie.
- **Plan:** Zwei lebende Spezifikationen mit Rangfolge (seit PR2,
  02.09.2026): `docs/FL-Nakama-Sonden-Design-Entwurf.md` (Fassung 0.5) bindet
  Technik, Verträge, Phasen und Falsifikation;
  `design/docs/funktions-und-bedien-blueprint.md` bindet die Funktions- und
  Bedienarchitektur beider Apps. Über beiden stehen aktueller Vertrag und
  Code. Widersprüche werden als Zeile in `docs/offene-punkte.md` geführt, nie
  kaschiert. Der gerechnete Stand und die nächste Arbeit stehen in
  `docs/PLAN-STAND.md`; danach nur Gate-Text, Fachquellen und Manifest des
  Tickets. `docs/NEXT-SESSION.md` ist seit 08.09.2026 nur ein Zeiger.

## Arbeitsregeln

- Code und laufende Beweise sind die Wahrheit; vor einer Änderung die Quelle
  lesen. Doku, Memory und Audits sind Hinweise.
- Zu Sessionbeginn und nach jedem Ticket `py -3.13 tools/plan/planstand.py`;
  ändert sich `docs/PLAN-STAND.md` aus committeten Quellen, nur diese Datei
  mit Pathspec committen und pushen (`docs/plan/LIES-MICH.md`). Plandokumente
  (Register, Plan, Abnahmen, Blueprint, Leseblätter, diese Datei) laufen vor
  dem Commit durch `py -3.13 tools/plan/dokuriegel.py <datei>`; ein Verweis
  ins Leere ist ein Befund.
- Beziehungen mitprüfen: speichern↔laden, starten↔stoppen, öffnen↔schließen,
  verbinden↔trennen, aktivieren↔abklingen, installieren↔Rückweg gehören in
  denselben Änderungssatz.
- Keine Neben-Refactors, keine Legacy-Umbenennung nebenbei. Fixbare Fehler im
  beauftragten Bereich werden behoben und geprüft.
- **Codebase-Gesundheit ist Planbestandteil** (User 08.09.2026, Register
  NAK-223): je Phase ein Pflegeschritt (S25b–S25d, S31c, S35b) und Schwellen,
  gemessen mit `py -3.13 tools/plan/gesundheit.py` (Exit 4 = gerissen, Exit 2
  = Werkzeugfehler; Kanon-Bein A32, nicht blockierend). Codebase-Pflege läuft
  als eigenes verhaltensneutrales Ticket mit vollem Kanon vorher und nachher
  (Muster S19b). Die Always-on-Kontextfläche (diese Datei, Dirigenten-Skill,
  `MEMORY.md`) wird in jedem Abschlussfenster gemessen und ein Riss sofort
  behoben, nicht registriert (User 12.09.2026; Grenzen in
  `docs/context-hygiene-playbook.md`): für Claude optimiert, Logik unverändert,
  keine Zitate außer als Entscheidquelle.
- Kontexthygiene gehört zu jedem Ticketabschluss; bei Phasengates der volle
  `/freshen`-Lauf nach `docs/context-hygiene-playbook.md`. Übergaben stehen
  im Manifest, Planstand und Register, nie in einem Sessionprotokoll.
- Fortschritt erst nach Beleg aus der laufenden Session behaupten; danach den
  Diff adversarial lesen: Zahlenränder, NaN/Inf, stale Closures,
  Save/Load-Symmetrie, irreführende UI-Texte.
- Volatile Zahlen (Test-, Fixture-, IPC-, Capability-Anzahlen) nie hier
  festschreiben; aus Code, Manifest oder jüngstem Beweis lesen.
- Große technische Schritte ziehen `docs/plugin-wissen.md` nach; offene
  Produktpunkte außerhalb des Auftrags gehen datiert nach
  `docs/offene-punkte.md`.
- Im Dirigentenbetrieb schreibt genau ein Hintergrundworker im sichtbaren
  Checkout (`worktree.bgIsolation = "none"` nur mit dieser Regel); Fable
  bleibt währenddessen an Repo-Dateien lesend. Eine zweite Session, die
  parallel Repo-Dateien ändern muss, arbeitet in einer eigenen Arbeitskopie
  (`git clone --local -c core.longpaths=true` an einen kurzen Pfad, kein
  `git worktree`), committet mit Pathspec und pusht. Screenshots und
  Temporäres nie in den Repo-Root.
- **Keine Berechtigungsfragen** (User 11.09.2026): `.claude/settings.json`
  hält `defaultMode: dontAsk`, `allow` für alles Nötige und `deny` statt
  `ask` für Destruktives (Vorlage `tools/dirigent/settings.dontask.json`; die
  Datei ändert nur der User). Dirigent und Worker starten mit
  `--permission-mode dontAsk`; eine `ask`-Regel oder ein Prompt ist ein
  Befund.
- Sicherheitsarbeit (Reviews, Audits, Härtung an Pipes, Impersonation,
  Signaturen) wird an Worker, Codex oder Review-Skills delegiert, nie im
  Gesprächs- oder Dirigentenkontext ausgeführt (User 01.09.2026).
- Der User ist Projektleiter und Musiker, kein Programmierer. Technische Wege
  entscheidet Claude selbst und legt sie nie als Menü vor; vorgelegt werden
  nur Produktwirkung sowie Design- und Produktfragen.
- Rechnerwechsel: PC und Laptop committen parallel auf `master`; Memory und
  `~/.claude/CLAUDE.md` sind rechnerlokal. Alles, was die andere Seite
  braucht, steht in dieser Datei, im Skill oder einer Repo-Datei und ist
  gepusht; ein Artefakt nur in einem Werkzeugordner
  (`~/.codex/visualizations/`, `.playwright-mcp/`, Scratchpad) ist nicht
  geliefert. Sessionstart `git pull --ff-only`, vor jedem Push bei sauberem
  Worktree `git pull --rebase`; der Push ist ein eigener Schritt.

## Lesen vor der Arbeit

| Bereich | Zuerst lesen |
|---|---|
| Nächster Schritt | `docs/PLAN-STAND.md`, danach die konkrete Ticketquelle |
| Planstand / offene Fragen | `docs/PLAN-STAND.md`, `docs/plan/plan.json`, `docs/plan/fragen.json`, `docs/plan/LIES-MICH.md` |
| Plugin heute | `docs/plugin-wissen.md` |
| Sondenfamilie / Phasen | `docs/FL-Nakama-Sonden-Design-Entwurf.md` (Fassung 0.5), `docs/bauaufteilung-sonden.md` |
| Verträge v3 | `eq-copilot/schemas/v3/README.md`, `eq-copilot/schemas/v3/flatbuffers/README.md` |
| State / Migration | `eq-copilot/schemas/state/`, `eq-copilot/plugin/state/`, `eq-copilot/fixtures/state/` |
| Beweise | jüngstes passendes Manifest in `docs/beweise/` |
| FL-Capabilities | `eq-copilot/identity/host-capabilities-fl-v1.json`, `docs/beweise/termin-a/`, `docs/beweise/termin-b/` |
| FL-Laufzeit, MCP-Server, Diagnoseprojekt | `tools/fl/LIES-MICH.md`, Register NAK-284 bis NAK-286 (MCP-Repo `C:\Users\phili\Projekte\fl-studio-mcp`, GitHub `evenacadia-tech/fl-studio-mcp`) |
| App-Design | `design/LIES-MICH.md`, `design/docs/funktions-und-bedien-blueprint.md`, `design/skizze/LIES-MICH.md`, `design/visuell/LIES-MICH.md`, jüngste passende Datei in `design/abnahmen/` |
| Widerspruch Entwurf ↔ Blueprint | Vertrag und Code stehen über beiden; Widerspruch als Zeile in `docs/offene-punkte.md` |
| Externes Wissen | `wissen/INDEX.md`, danach der passende Wissensbereich |
| Verlauf, nie Vorgabe | `docs/archiv/`, `eq-copilot/design/archive/`, `eq-copilot/design/prisma-studie/STATUS.md` |

Die frühere Daueranweisung liegt als Snapshot in
`docs/archiv/CLAUDE-kontext-vor-bereinigung-2026-08-24.md` (nur Spurensuche).

## Repo-Karte

- `eq-copilot/plugin/`: JUCE-Produktcode, Hostbrücke, Tests, Werkzeuge.
- `eq-copilot/schemas/`: v2-Verträge; `schemas/v3/`: Sondenfamilie.
- `eq-copilot/identity/`: eingefrorene Identität und Host-Capabilities.
- `eq-copilot/install/`: manifestgetriebener Installer und Rückweg.
- `broker/`: Rust-Broker `eqcop-broker.exe`.
- `tools/`: `beweise.ps1` (kanonischer Beweis-Runner), `plan/` (Planstand,
  Dokuriegel, Gesundheit, Antwortblatt), `dirigent/` (Starter, Cockpit,
  Prüfliste, Vorlagen), `fl/` (Laufzeit-Arm, Szenarien), `eq-copilot/`
  (Fixture- und Prüfskripte), `hooks/` (Primer, Git-Riegel), `pruefung/`
  (Prüfsessions), `codex-plugins/` (Codex-Werkzeugquelle).
- `design/`: Übersetzung der User-Vorgabe, Skizze (`skizze/`), visuelle Phase
  (`visuell/`), Abnahmen, Designregeln, historischer Web-Prototyp
  (`prototyp/`).
- `wissen/`: eingeordnetes externes Wissen; keine Produktentscheide, keine
  Roh-PDFs.
- `docs/beweise/`: rohe, ticketspezifische Belegmanifeste; Rohausgaben unter
  `roh/`, User-Messtermine unter `termin-a/` und `termin-b/`.

## Tragende technische Invarianten

- **Schemas sind Verträge.** Neue persistente Felder zuerst versionieren;
  Altstände laden; unbekannte Felder dürfen alte Consumer nicht zerstören;
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
  Nyquist gekappt; ohne genügend endliche Nachbarn keine Basislinie.
- **Begriffe nicht vermischen:** Probe-Pipe ist nie Produktions-Pipe;
  `ltasReferenzDb` ist keine globale Sollkurve; Paint-FPS ist nicht
  Datenkadenz; Demo-Daten sind keine Plugin-Daten; FL zeigt MIDI 60 als C5.
- **State bleibt verlustfrei.** Unbekannte Major-Versionen oder unzulässige
  Matrizen werden read-only mit Originalbytes gehalten; jede persistente
  Änderung meldet dem Host Dirty-State.
- **Identität bleibt eingefroren** bis NAK-30: Class-IDs, Hersteller-/
  Plugin-Codes und Replace-V2-Verhalten nie beiläufig ändern.

## Bauen und beweisen

Kanonischer Komplettlauf vom Workspace-Root:

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-0NN.md -Anhaengen -Titel 'SONDE-0NN'
```

Der Runner baut, fährt die deklarierten Beine, legt Rohausgaben unter
`docs/beweise/roh/` ab (Manifest trägt Kopf, Urteilszeile, Übersicht) und
verweigert die Beglaubigung, wenn Prüfbinaries älter sind als ihre Quellen.
Einzelbefehle und Zielnamen stehen im Skript. Nach grünem Kanon fährt der
Laufzeit-Arm `tools/fl/laufzeit.ps1 -Ticket <T> -Basis <sha>` (installiert,
startet FL mit `eq-copilot/fixtures/fl/Nakama-Diagnose.flp`, fährt die
Szenarien aus `docs/gesundheit/szenarien/`; Exit 4 = Szenario verfehlt;
`tools/fl/LIES-MICH.md`). Editor-Sichtprüfung ohne FL
über `EqCopShot.exe`. Installation läuft seit 12.09.2026 ohne User-Handgriff
über die erhöhten Aufgaben `\Nakama\installieren`, `\Nakama\pruefen`,
`\Nakama\rueckweg` (User-Entscheid, Register NAK-285; je Rechner einmal
erhöht `tools/dirigent/install-aufgaben-registrieren.ps1`; Log unter
`%LOCALAPPDATA%\evenacadia\nakama-installer\<modus>.log`, Exitcode =
Installer). Der Installer verweigert bei laufendem FL und bei
Manifest-Hash-Abweichung: vorher FL beenden, nach grünem Kanon `--hashen`.
Broker-Ende-zu-Ende immer über den Probe-Pipenamen; nie ein Testbroker auf
der Produktions-Pipe. Vor einer Installation State-Migrationsstand und
letzten Installationsbeweis prüfen.

## Design-Arbeitsmodell

1. Blueprint und der Arbeitsmodus vom 31.08.2026 sind die Grundlage; frühere
   visuelle Entwürfe sind keine Vorgabe.
2. Pro Oberfläche genau eine offene Frage zu Aufbau, Hierarchie,
   Sichtbarkeit, Objektbesitz oder Disclosure; Funktionsumfang und
   technische Grenzen vorher selbst klären.
3. Antworten im Fünferblock sammeln, User-Wortlaut im Blockprotokoll
   `design/abnahmen/<datum>-technische-ui-architektur-fuenferblock-NN.md`
   (entsteht mit der ersten Antwort, nie leer vorab); liegt die Frage als
   Karte in `docs/plan/fragen.json`, wird sie mit `/fragen` gestellt. Nach
   der fünften Antwort Skizze (`design/skizze/nakama-ui-technical-sketch.html`),
   Abnahmen und Blueprint gemeinsam aktualisieren, Screenshot nach
   `design/skizze/belege/`, committen; Geschlossenes nicht erneut öffnen.
4. Farben, Material, Typografie entstehen in Figma (`fable-dummy`); jeder
   Stand wird unter `design/visuell/belege/` belegt und bleibt Vorschlag;
   erst eine dokumentierte Abnahme erlaubt die Spiegelung in `eq-copilot/`.
5. `eq-copilot/design/`, frühere Figma-Stände und die drei Rework-Bilder sind
   Verlauf; sie ergänzen die Funktionsarchitektur nicht still.

## Maschinen-Landminen

- PowerShell `Start-Process -ArgumentList` quotiert Argumente mit Leerzeichen
  nicht; doppelte Anführungszeichen in einem `-Command`-Text schluckt die
  Argumentzerlegung (`Add-Content $log ("EXIT=" + …)` wird zum Kommando
  `EXIT=`, die Marke fehlt still). Zeichenketten im Befehlstext einfach
  quotieren.
- Bash-Heredocs können Backslashes in Windows-Pfaden verändern; Pipes
  verdecken Exitcodes.
- Bytegleiche Fixtures und Patches brauchen passende `.gitattributes`-Regeln.
  `-text` gilt für `eq-copilot/schemas/{v3,state}/**`,
  `eq-copilot/fixtures/{v3,state,p4-korpus,p5-korpus}/**`, `design/assets/**`
  und `*.patch`; der übrige Baum ist gemischt (LF und CRLF): vor dem Schreiben
  `git ls-files --eol <datei>` lesen und dieselben Zeilenenden zurückschreiben.
- Keine GPU-Batch-Render-Loops auf der Arc A770; Renderprüfung einzeln oder
  per CPU.
- Playwright-MCP lädt keine `file://`-Adressen und legt relative
  Screenshot-Pfade im Repo-Root ab: lokalen HTTP-Server im Zielordner
  (`py -3.13 -m http.server <port> --bind 127.0.0.1`), Bilder sofort
  verschieben, `git status --short` lesen.
- Codex legt Visualisierungen unter `~/.codex/visualizations/` ab; nach
  Codex-Design-Arbeit prüfen, dass jedes referenzierte Artefakt committet ist
  (`grep -rn "codex.visualizations\|127.0.0.1" design/`).
- `git clone` braucht `-c core.longpaths=true` und einen kurzen Zielpfad,
  sonst „Filename too long" unter `tools/codex-plugins/`.
- MSVC-Standardstack 1 MiB: `FeatureEngine`-Objekte (~0,5 MB) und Prozessoren
  in Tests auf den Heap (`std::unique_ptr`); der Riss zeigt sich erst im
  vollen Kanon (NAK-175).
- `cmake` liegt nicht im Bash-`PATH`: C++-Beine aus pwsh bauen und den
  Binary-Zeitstempel gegen die Quelle prüfen; `tools/beweise.ps1` sucht cmake
  selbst.
- Backticks in einem Bash-Befehlstext werden ausgewertet, auch in `git commit
  -m`; mehrzeilige Commitnachrichten mit Bezeichnern über `git commit -F`.
- Typografische Anführungszeichen in Rust-Strings: `„…"` schließt mit dem
  ASCII-`"` den String; im Quelltext `“` (U+201C) schreiben; der Compiler
  meldet „unknown start of token" an anderer Zeile.
- Im PowerShell-Werkzeug der Session geht in einer Zeichenkettenverkettung
  alles hinter einem Backtick-n-Umbruch verloren; mehrzeilige Einfügungen
  zeilenweise oder aus einer Datei. Der Sandbox-Filter blockt jeden Befehl,
  dessen Text den Namen des PowerShell-Lösch-Cmdlets enthält, auch in einem
  Worker-Prompt. `claude rm <id>` nur als eigenen Befehl absetzen; in einer
  Befehlskette blockt der Filter die ganze Kette (12.09.2026).
- Ein frisch gebauter Kanon kann einmalig mit zwei `flatc`-Beinen
  „Voraussetzung fehlt" enden; vor einem Befund gegen den Runner einmal
  wiederholen. Fehlt es zweimal, fehlt `flatc.exe` (Zeiger in
  `eq-copilot/build/nakama-flatc-pfad-Release.txt`, Bauartefakt,
  nicht ins Repo, verschwindet nach manchem Worker-Bau): `cmake --build eq-copilot/build --config Release --target
  flatc` nachbauen (NAK-280).
- Ein Pipe-Zeichen in einer Markdown-Tabellenzelle reißt `dokuriegel.py`;
  als `\|` schreiben, Zeilenbereiche mit ASCII-Bindestrich.
- `Copy-Item` überträgt `LastWriteTime`: eine zurückgespielte Quelle kann
  älter sein als ihr Objektfile, MSBuild baut dann nicht neu und meldet Exit 0
  (NAK-230). Vor dem Neubau `(Get-Item <datei>).LastWriteTime = Get-Date`
  setzen und den Binary-Zeitstempel prüfen.
- Ein laufender Dirigenten-Starterprozess lädt sein Skript beim
  Marker-Neustart nicht neu (NAK-257); Skriptänderungen greifen erst nach
  Fensterneustart.
