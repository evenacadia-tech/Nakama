# NAKAMA

Transparentes Berater-VST3 für FL Studio (Windows 11, JUCE 8/C++20, CMake)
mit **eigenständigem Rust-Broker** (Named Pipe, `broker/`). Produktname
**Nakama**; Code, Bundle, Pipes und Schemas tragen aus Kompatibilität den
Legacy-Namen **EQ-Copilot** (`EqCop*`) — das ist Absicht, kein
Umbenennungs-Task.

**Dieser Workspace ist seit 18.08.2026 EIGENSTÄNDIG** — Code, Broker,
Design, Docs und Wissen leben in EINEM Repo. Git-Historie davor:
FL-Studio-Repo (`C:\Users\phili\FL-Studio`, bis Commit `7964777`); der
FL-CLAUDE.md-Auszug liegt wortgleich in
`docs/fl-claudemd-auszug-2026-08-18.md`.

| Was | Pfad (relativ zum Workspace-Root) |
|---|---|
| Plugin (JUCE 8 + CMake) | `eq-copilot/plugin/` |
| Schemas — versionierte Verträge | `eq-copilot/schemas/` |
| Design (Musterblätter, Prisma, Renders, Fonts) | `eq-copilot/design/` |
| Projekt-Docs (Baupläne, M0–M3a-Befunde, Testanleitung) | `eq-copilot/docs/` |
| **Broker (eigene Crate, eqcop-broker.exe)** | `broker/` |
| Sensorübersicht (Svelte-Referenz, ohne Zuhause — NAK-12) | `broker/sensoruebersicht-referenz/` |
| Fixture-Generator + Testsong-Werkzeuge | `tools/eq-copilot/` |
| Kanonischer Produkt-/Umsetzungsplan | `FL-EQ-Copilot-Recherche.md` |
| Wissens-Docs (design-stand, geschmacksprofil, plugin-wissen …) | `docs/` |

**Commit-Routing:** EIN Repo — alles hierher, Deutsch, kleine benannte
Commits. Parallele Codex-Sessions sind möglich (Vorentwurf-Blatt =
Codex-Besitz): eigene Edits SOFORT committen, nie `--amend`, fremde
Änderungen nie anfassen. **Remote seit 19.08.:**
`https://github.com/evenacadia-tech/Nakama` (privat) — nach Commits
pushen (User arbeitet wechselnd an Desktop und Laptop und zieht per
Pull); vor Arbeitsbeginn `git pull`, damit der andere Rechner nicht
überholt wird.

## Grundgesetz (nicht verhandelbar)

Nakama **berät nur**. Es setzt keine EQ-Parameter, schreibt keine Automation,
verändert kein Audio (Passthrough sampleidentisch, 0 Samples gemeldete
Latenz, kein Tail) und ruft Claude nur auf bewussten Klick. Der Audiothread
enthält keine Sperren, keine Allokationen, keine Datei-/Pipe-/Netz-Zugriffe,
kein Logging. Überlast verwirft Analyseframes, nie Audio.

**Die eine dokumentierte Ausnahme (seit 0.3.0):** die **Hör-Markierung**
färbt auf bewussten Klick das Monitorsignal (Solo/Puls je Befund), streng
verriegelt: nur bei bewiesener Echtzeit ∧ Editor offen ∧ Transport ∧
`!isNonRealtime()`. **Render/Export bleibt bitidentisch** (MarkierungTest
beweist es); der Analyse-Abgriff sitzt VOR der Färbung. Jede weitere
Audio-Ausnahme braucht denselben Verriegelungs- und Beweisstandard.

## Bauen & Beweisen (vom Workspace-Root)

```powershell
$cmake = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -S eq-copilot -B eq-copilot/build -G "Visual Studio 17 2022" -A x64
& $cmake --build eq-copilot/build --config Release --target EqCopilot_VST3 EqCopShot EqCopPaintBench EqCopNullTest EqCopGoldenTest EqCopMarkierungTest EqCopPipeProbe EqCopIdentityTest EqCopHostContextTest
```

**Ein Befehl für den ganzen Kanon (seit 20.08., ersetzt die bewusst nicht
gebaute CI):**

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-0NN.md -Anhaengen -Titel 'SONDE-0NN'
```

Er fährt alles unten, schreibt die **rohe** Ausgabe ins Manifest und misst den
**Baustand**: sind die Prüfbinaries älter als die Quellen, verweigert er mit
Exitcode 4 die Beglaubigung, statt eine veraltete Messung als Beweis auszugeben
(0 grün · 2 rot · 3 Voraussetzung fehlt · 4 nicht beglaubigt). Ticket-Vorlage
`docs/beweise/VORLAGE.md`, Regressions-Basislinie `docs/beweise/S0-basislinie.md`.

Beweis-Kanon einzeln (alles headless; Standard der Befund-Docs: **„ausgeführt und
gesehen"**, nie „sollte funktionieren"):

```powershell
eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures
eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
cargo test --manifest-path broker/Cargo.toml
```

**Kanon-Stand 21.08.2026: 7/7 grün** (`docs/beweise/SONDE-003b.md`) — Nulltest ·
Golden · Markierung · Broker · Identitaet · Hostkontext · Host-Probe. Fuenf
weitere Pruefbinaries stehen als „geplant" in der Runner-Tabelle und werden
Pflicht, sobald ihr Ticket sie baut.

- Golden-WAVs einmalig: `py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav`
- Editor-Sichtprüfung ohne FL: `EqCopShot.exe <ziel.png> [breite]` (echte 20-s-Messung, offscreen)
- paint()-Kosten: `EqCopPaintBench` · Host-Härtung: `pluginval --strictness-level 8`
- **Broker-Betrieb:** `broker\target\release\eqcop-broker.exe [--bindungen <pfad>]`
  (eigenständiger Prozess; Standard-Bindungen
  `%APPDATA%\evenacadia\nakama\eq-copilot-bindungen.json`; die Hub-App
  startet ihn NICHT mehr — Umzug 18.08.)
- Pipe Ende-zu-Ende: `broker\target\release\eqcop-broker-probe.exe 30`
  (Terminal 1) + `EqCopPipeProbe.exe "\\.\pipe\evenacadia.eq-copilot.m2probe"`
  (Terminal 2) — **immer der eigene Probe-Pipename**, nie der
  Produktions-Broker (zwei Broker auf einem Namen stahlen sich still
  Clients; Produktion verweigert per FIRST_PIPE_INSTANCE)
- **Identitaet ist ab P0 ein Dateiformat:** `eq-copilot/identity/plugin-identities-v1.json`
  friert Bundle, Plugin-Code, beide Class-IDs und `JUCE_VST3_CAN_REPLACE_VST2=0` ein;
  `NkPr`/`NkAc` sind fuer die Sonden reserviert. `EqCopIdentityTest` misst das
  GEBAUTE `moduleinfo.json` dagegen, prueft zusaetzlich den **CMake-Quelltext**
  (greift auch ohne Bau), faellt bei veraltetem Artefakt, rechnet die
  reservierten CIDs nach und haelt die Schema-1-State-Goldens der vier Rollen
  (`eq-copilot/fixtures/identity/`). Goldens neu schreiben: `--schreibe-goldens`.
- **Hostbruecke (SONDE-003, seit 21.08.):** der gevendorte JUCE-8.0.9-VST3-Wrapper
  wird beim **Configure** um drei Beobachtungen erweitert, die die oeffentliche
  `AudioProcessor`-API nicht traegt — Anwesenheit des `ProcessContext`, ALLE
  Parameterpunkte mit Sample-Offset, `IAudioPresentationLatency` je Bus.
  Patch `third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch`, Gate
  `eq-copilot/cmake/NakamaBruecke.cmake` (unberuehrt ⇒ patchen und **nachmessen** ·
  gepatcht ⇒ No-Op · fremd ⇒ **Bauabbruch** mit gemessenem Hash; Hash ueber den
  zeilenende-normalisierten Inhalt). Gegenseite: `plugin/hostbridge/NakamaHostBridge.h`
  (JUCE-/SDK-frei, vorallokiert, 0 Allokationen im Blockpfad). Pruefer
  `EqCopHostContextTest` (91 Pruefungen).
  🔑 **Zwei teuer bezahlte Regeln aus dem T2-Verfahren dieses Tickets:** (1) ein
  Puffer, der beim Ueberlauf hinten abschneidet, darf NIE die Quelle eines
  Wertes sein, den ein Vertrag ueberleben laesst — der Rueckfallwert braucht
  seine eigene Struktur (hier `Letztwert`-Tabelle). (2) Ein Zaehler muss
  beschreiben, was der HOST geliefert hat, nicht was in unsere Struktur passte;
  stand der Kapazitaets-Ausstieg vor den Pruefungen, meldete `unplausibleWerte`
  0, waehrend ein NaN stiller Rueckfallwert wurde. ⚠️ **`_deps` nie von Hand editieren** —
  wer den Wrapper anfasst, laesst das Gate beim naechsten Configure fallen.
  🚨 **Bei jedem JUCE-Update ist der Patch NEU zu beweisen**: beide Hashes im
  Gate nachziehen, `EqCopHostContextTest` fahren.
  ⚠️ **Landmine: JUCE speichert den Wrapper im Objektspeicher mit CRLF**
  (4165/4165 gemessen) — der Patch traegt deshalb CRLF in 149 seiner 163 Zeilen.
  `.gitattributes` haelt `*.patch` per `-text` bytegleich; ohne das haette git
  die CR beim Commit entfernt und einen Patch eingecheckt, der auf dem
  Zweitrechner nicht mehr anwendbar ist (hier waere es nie aufgefallen).
- **Termin-B-Messgeraet (S3b, Wegwerfware):** Ziele `EqCopHostProbe_VST3`
  (Bundle, Plugin-Code `NkHp`) + `EqCopHostProbeTest` (Selbsttest, 88 Pruefungen, inkl. Layout-Riegel, Nebenlaeufigkeits-Rauchtest und zwei gerenderten Bildbeweisen).
  **Erstes Ziel, das die Hostbruecke BENUTZT** — sein Processor ist eine
  `eqcop::hostbruecke::Senke`. Misst je Block: Context-Anwesenheit,
  Gueltigkeitsbits (immer/manchmal/nie), Zeitspruenge (Seek · Loop · Smart
  Disable, mit Fehlalarm-Riegel gegen fortlaufendes Spiel und Stop/Play),
  Offline-Render, float/double, Presentation-Latency und **samplegenaue
  Automation** (Punkte je Block + Offsets — die Frage, fuer die der Bridge-Patch
  gebaut wurde). Bericht als JSON nach `%APPDATA%\evenacadia\nakama\spike\`.
  Klickliste `eq-copilot/docs/FL-TERMIN-B-HOSTZEIT.md`. Messstand geht per
  **Seqlock** zum Nachrichtenthread — der Audiothread wartet nie.
- **Aux-/PDC-Messgeraet (SONDE-004a, Wegwerfware):** Ziele `EqCopAuxSpike_VST3`
  (Bundle) + `EqCopAuxSpikeTest` (Selbsttest, 41 Pruefungen). Plugin-Code `NkSp`,
  bewusst **ausserhalb** der eingefrorenen Identitaet (`Eqcp`/`NkPr`/`NkAc`).
  Klickliste fuer den FL-Termin: `eq-copilot/docs/FL-TERMIN-A-AUX-PDC.md`;
  Impulse per `tools/eq-copilot/erzeuge_aux_spike_fixtures.py` (Hashes im
  Fixture-MANIFEST). Wird nach dem Capabilityreport (S4) entsorgt.
- **Installation = User-Klick:** `eq-copilot\install\Install-EQ-Copilot.ps1` als Admin (UAC), Rollback-Datei liegt daneben. Nie automatisch installieren. Vorher FL beenden.

## Invarianten — tragend, jede Runde präsent

- **Schemas sind Verträge** (`eq-ipc` v2 · `eq-measurement` · `eq-report` ·
  `eq-snapshot` v3 · `eq-aggregat`): neue Felder ⇒ ERST Versionierung; alte
  Snapshots laden ohne die Felder; unbekannte Felder zerstören alte Consumer
  nicht; Save + Load im selben Änderungssatz testen.
- **Engine kennt keine Optik:** AnalyseEngine liefert kohärente MessSnapshots
  (~20 Hz Leichtpfad `auswertenLeicht()` + 250-ms-Schwerauswertung, EINE
  Quelle `fuelleBasis()`); der Editor hält NUR Anzeigezustand und malt nur
  bei neuer Snapshot-Revision — im Leerlauf exakt nichts.
- **Gegenpfade** (Bauplan §11.5): starten↔stoppen · öffnen↔schließen ·
  speichern↔laden · aktivieren↔abklingen/archivieren · installieren↔Rollback
  — immer beide Hälften im selben Änderungssatz.
- **Zeit ist Aktivzeit:** Zonen-Ticks laufen je 1 s AKTIVER Musikzeit in
  `verarbeite()` (deterministisch, GoldenTest-beweisbar) — in Pausen vergeht
  keine Tick-Zeit.
- **NaN-Ehrlichkeit:** NaN-Riegel mit Zähler, Nyquist-Kappe der LTAS; neue
  Rechenwege müssen NaN-ehrlich sein (< 5 endliche Nachbarn ⇒ keine Basislinie).

## Anti-Conflation (jede Zeile war ein echter Irrweg)

- **Pixel-Beweis ≠ Schönheits-Beweis** — E0–E5 war 18/18 bewiesen und
  hässlich; Optik nimmt der User am lebenden Blatt ab.
- **VERWORFEN, nie neu vorschlagen:** 3D-Papier-Shader (E0–E5) ·
  Tusche-Einzelmarken-Piktogramme · **rohe Energie-Punktwolke als
  Dauerinhalt** (18.08.: „Wasserfall aus Pixeln" — ein Profi liest daran
  nichts ab; Inhalt = wenige präzise BEFUND-Objekte). → `docs/design-stand.md`
- **Musterblatt-Beispieldaten ≠ Plugin-Daten** — HTML-Demo-Werte wandern nie
  in das Plugin (Bauplan §1.5). Die 30-s-Songschleife ist die einzige
  sanktionierte Design-Datenquelle der Blätter.
- **`ltasReferenzDb` ≠ Sollkurve** — reine 8192er-Messachse; es gibt KEINEN
  globalen Zielkorridor; Befunde messen gegen ihre eigene Schulterlinie.
- **paint()-FPS ≠ Datenkadenz** — M3a: 4-Hz-Snapshots sahen aus wie
  Render-Lag; paint() war mit 2,4 ms unschuldig. Erst messen (PaintBench),
  dann optimieren.
- **`claude.html` ≠ `vorentwurf.html`** — Claudes Design-Blatt vs.
  Codex-Besitz (Guard-Hook blockt Edits am Codex-Blatt).
- **Probe-Pipe ≠ Produktions-Pipe** — Produktion ist
  `\\.\pipe\evenacadia.eq-copilot.v1` (Name bleibt „v1", die
  Protokollversion wird im Handshake verhandelt); Tests nutzen `…m2probe`.
- **FL-Notennamen:** FL zeigt MIDI 60 als **C5** (Oktave = MIDI div 12) —
  116 Hz = A#3. Nie die Standard-Oktavzählung annehmen.
- **Spot-Watt in Blender sind KUGELnormiert** — enger Kegel maskiert statt
  bündelt; Zwei-Flächen-Kaustik ist in Cycles unidirektional unsampelbar
  (Details: `docs/design-stand.md`, Cycles-Grenzen).

## Design-Arbeitsmodell

**KREATIV-SCHLEUSE (User-Regel 17.08., nicht verhandelbar):** Der kreative
Prozess wird NIE übersprungen. Eine Idee wird im Gespräch ausgearbeitet und
konkretisiert, solange bis der User eine Vorstellung hat und sie freigibt —
erst dann wird gebaut. Kunst ist wertlos ohne ihren Entstehungsprozess.
Mechanisch erzwungen: `tools/hooks/kreativ-schleuse.sh` blockt Write/Edit
unter `eq-copilot/design/`, solange `.claude/kreativ-freigabe.md` fehlt oder
älter als 24 h ist; der Marker hält die freigegebene Vorstellung in den
Worten des Users fest und wird NUR nach dessen ausdrücklicher Freigabe
geschrieben.

Design-Phase lebt im **Browser** (Sekunden-Schleife), nicht im Plugin.
**Aktive Richtung: das PRISMA** (User-autorisiert 17.08.; Drehen = Selektion
Tiefen/Mitten/Höhen, Klick = Bündeln; Film-Compositing mit Blender-Cycles
auf CPU). **Visueller Nordstern (User-Freigabe 19.08.): der HÖRKOMPASS** —
verbindlicher Zielvertrag `docs/visuelles-zielbild-hoerkompass.md`: der Mix
bleibt unsichtbar, gesund = leeres Glas; nur ein belastbarer Befund bricht
lokal das Licht; keine neue Grundmetapher, kein Dauer-Visualizer.
**Produktarchitektur (User-Entscheid 20.08., überall klarzustellen): die
Prisma-App ist eine EIGENSTÄNDIGE Begleit-App NEBEN Master-Plugin +
Sonden** — das Master-Plugin (Nakama Main) bekommt eine konventionellere
Arbeits-UI mit den Sonden; die Prisma-App ist Addition, nie Master-Hub;
der Hörkompass-Zielvertrag gilt der Prisma-App, nicht der Plugin-UI.
**Stand 19.08.:** Prisma-OBJEKT abgenommen („sieht top aus";
klar > rauchig, Frost raus), drehbarer Prototyp steht
(`eq-copilot/design/prisma/prisma-prototyp.html`); NAK-16
ST-Map-Live-Refraktion: Probe BESTANDEN, 72er-Drehsequenz im Blatt
(`stmap-probe.html`) — offen: Optik-Abnahme durch den User; rohe Punktwolke
als Dauerinhalt VERWORFEN; **offen: Befund-Verkörperung** (1 Befund = 1
präzises Objekt im Glas — Resonanz zuerst; WAS es ist, kommt vom User).
Einstieg `docs/NEXT-SESSION.md` + `docs/design-stand.md`. Geparkt, nie
ungefragt reaktivieren: Tiefenfeld, Bauplan 2.0, Kunstwerk-Studie. Kein
Schritt zeigt erfundene Diagnosewerte — fehlt ein Produktionsfeld, bleibt
die Darstellung verborgen.

**DESIGN-VERFASSUNG (4 Proben, teuer bezahlt — Volltext mit wörtlichen
User-Urteilen: `docs/geschmacksprofil.md`, vor jeder Design-Entscheidung
lesen):**
1. **Schärfe:** Detail aus Auflösung in echte Punkte/Struktur; Glow/Bloom/
   Weichzeichner = „billig", Unity-Baukasten-Effekte verboten.
2. **Handschrift:** lieber einfach mit persönlichem Stil und Liebe zum
   Detail als komplex-generisch.
3. **Sichtbarer Denkvorgang mit Distanz:** laut→hell ist Tautologie;
   NOTWENDIG statt konstruiert („kein Grund, dass es da ist" = Todesurteil);
   ENTDECKT statt entworfen (Phänomen/Emergenz schlägt Designer-Objekt).
   **Verschärft 18.08.: LESBARKEIT ist der Produktmaßstab** — ein Profi
   (20 Spuren) muss in 2 Sekunden ABLESEN können; Berater zeigen BEFUNDE,
   keine Atmosphäre; leeres Glas = gesundes Band.
4. **Meta:** Claudes freie Bilderfindung = Mode-Collapse („wie
   algorithmisch komponierte Musik — objektiv ok, aber egal"). Vision und
   Referenzen kommen vom User; Claude übersetzt treu, verdatet ehrlich,
   verfeinert unter seinem Auge. Abgenommen: P01-Perkolations-ÄSTHETIK
   (scharfe Punktwolke, Bandfarben Amber/Magenta/Eis) + das Prisma-Objekt.

## Maschinen-Landmine

⚠️ **PowerShell: `Start-Process -ArgumentList` quotiert NICHTS** — es klebt die
Array-Elemente mit Leerzeichen zu einer Kommandozeile zusammen (anders als
.NETs `ProcessStartInfo.ArgumentList`). Gemessen beim Bau des Beweis-Runners an
`git log --format=%h %s`: Exit 128, weil `%s` ein eigenes Argument wurde. Jedes
Argument mit Leerzeichen selbst escapen (`Argument-Quoten` in
`tools/beweise.ps1`), sonst zerfaellt ein Pfad still.

**Keine GPU-Batch-Render-Loops auf der Arc A770** — der Lüfter-Failsafe
bleibt nach solchen Loops auf 100 % hängen (überlebt Warm-Reboots; Fix =
`Win+Ctrl+Shift+B`, sonst echter Kaltstart). Playwright-/Beweis-Renders
laufen in Software (SwiftShader) und bleiben Einzelläufe; Blender-Cycles
rendert auf CPU.

## Read before working on

| Bereich | Zuerst lesen |
|---|---|
| Wie das Plugin heute funktioniert (Architektur, Datenfluss, IPC) | `docs/plugin-wissen.md` |
| Design-Prototyp, Verwürfe, Freeze-Stand | `docs/design-stand.md` |
| Visueller Nordstern — Hörkompass-Zielvertrag (19.08.) | `docs/visuelles-zielbild-hoerkompass.md` |
| Sonden-Produkt, Technik + Phasenplan (Fassung 0.4) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` (+ `docs/pruefbericht-sondenentwurf-2026-08-20.md`) |
| Geschmacksprofil (wörtliche User-Urteile) | `docs/geschmacksprofil.md` |
| Neue Hauptansicht (Ziel, Grammatik, DoD) | `eq-copilot/docs/NAKAMA-SPECTRAL-FIELD-BAUPLAN.md` |
| DSP-/Realtime-/Broker-Regeln, Produktplan | `FL-EQ-Copilot-Recherche.md` |
| Aktuellster Plugin-Stand + Beweise | `eq-copilot/docs/M3A-BEFUND.md` (+ M0–M2, CS1) |
| Manuelle FL-Prüfungen (liegen beim User) | `eq-copilot/docs/FL-TESTANLEITUNG.md` |
| FL-Termin A — Aux/PDC/Recall messen (Sondenbau P0) | `eq-copilot/docs/FL-TERMIN-A-AUX-PDC.md` |
| FL-Termin B — Hostzeit, Transport, Automation messen (P0) | `eq-copilot/docs/FL-TERMIN-B-HOSTZEIT.md` |
| Benchmark-Mechaniken (Median-Basislinie, Zonen, Konvergenz) | `eq-copilot/docs/BENCHMARK-STUDIE-RESO-SMARTEQ-PROQ.md` |
| Offene Punkte (durabel, nie still löschen) | `docs/offene-punkte.md` |

## Arbeitsweise

- Code ist die einzige Wahrheit — Datei öffnen vor jeder Doku-/Memory-Behauptung.
- Fortschritts- und Fertig-Meldungen nur mit Beleg aus dieser Session
  (Testlauf, Render, Diff); nicht Verifiziertes explizit als offen nennen.
  Selbstaudit nach jedem Commit; größere Änderungssätze von einem frischen
  Verifikations-Subagenten gegen Bauplan + Grundgesetz prüfen lassen.
- Im Auftrag bleiben: keine Neben-Refactors, kein „Modernisieren" von
  Legacy-Namen nebenbei. Problembeschreibung ⇒ Befund liefern, erst auf
  Zuruf fixen.
- Breite mechanische Suchen parallel an Explore-Agenten delegieren und
  währenddessen weiterarbeiten; Design-Urteil bleibt im Hauptlauf.
- Out-of-scope-Funde → `docs/offene-punkte.md` (datierte Zeile mit ID).
- Nach großen Schritten: `docs/design-stand.md` bzw. `docs/plugin-wissen.md`
  nachziehen + Session-Memo ins Memory (`project_session_*.md`, Cap 6 —
  eine Lehre pro Datei, mit dem Warum; Falsches löschen statt stapeln).
- Alle Texte dieses Projekts (Docs, Commits, UI) auf Deutsch.
