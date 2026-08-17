# NAKAMA

Transparentes Berater-VST3 für FL Studio (Windows 11, JUCE 8/C++20, CMake)
mit Rust-Broker (Named Pipe) in der Tauri-Hub-App. Produktname **Nakama**;
Code, Bundle, Pipes und Schemas tragen aus Kompatibilität den Legacy-Namen
**EQ-Copilot** (`EqCop*`) — das ist Absicht, kein Umbenennungs-Task.

**Dieses Folder ist der Session-Anker** (Kontext, Wissen, Handoffs).
**Der Code lebt im FL-Studio-Repo** (Zugriff via `additionalDirectories`):

| Was | Pfad |
|---|---|
| Plugin (JUCE 8 + CMake) | `C:\Users\phili\FL-Studio\eq-copilot\plugin\` |
| Schemas — versionierte Verträge | `C:\Users\phili\FL-Studio\eq-copilot\schemas\` |
| Design (Musterblätter, Tokens, Renders, Fonts) | `C:\Users\phili\FL-Studio\eq-copilot\design\` |
| Projekt-Docs (Baupläne, M0–M3a-Befunde, Testanleitung) | `C:\Users\phili\FL-Studio\eq-copilot\docs\` |
| Broker (Rust, Pipe-Server, Paare, Aggregat) | `C:\Users\phili\FL-Studio\plugin-hub-app\src-tauri\src\eq_copilot\` |
| Sensorübersicht (Hub-App-Frontend) | `C:\Users\phili\FL-Studio\plugin-hub-app\src\lib\eq-copilot\` |
| Fixture-Generator | `C:\Users\phili\FL-Studio\tools\eq-copilot\` |
| Kanonischer Produkt-/Umsetzungsplan | `C:\Users\phili\FL-Studio\FL-EQ-Copilot-Recherche.md` |

**Commit-Routing:** Code-Änderungen → FL-Studio-Repo (Konvention dort:
`feat(eq-copilot): …`, Deutsch). Wissen/Handoffs/Hooks → dieses Nakama-Repo.
Das FL-Studio-Repo wird von parallelen Codex-/Harness-Sessions geteilt:
eigene Edits SOFORT committen, nie `--amend`, fremde rote Tests nie anfassen.

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

## Bauen & Beweisen (aus `C:\Users\phili\FL-Studio\` heraus)

```powershell
$cmake = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -S eq-copilot -B eq-copilot/build -G "Visual Studio 17 2022" -A x64
& $cmake --build eq-copilot/build --config Release --target EqCopilot_VST3 EqCopShot EqCopPaintBench EqCopNullTest EqCopGoldenTest EqCopMarkierungTest EqCopPipeProbe
```

Beweis-Kanon (alles headless; Standard der Befund-Docs: **„ausgeführt und
gesehen"**, nie „sollte funktionieren"):

```powershell
eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures
eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
cargo test --manifest-path plugin-hub-app/src-tauri/Cargo.toml eq_copilot
```

- Golden-WAVs einmalig: `py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav`
- Editor-Sichtprüfung ohne FL: `EqCopShot.exe <ziel.png> [breite]` (echte 20-s-Messung, offscreen)
- paint()-Kosten: `EqCopPaintBench` · Host-Härtung: `pluginval --strictness-level 8`
- Pipe Ende-zu-Ende: `eqcop-broker-probe.exe 30` (Terminal 1) + `EqCopPipeProbe.exe "\\.\pipe\evenacadia.eq-copilot.m2probe"` (Terminal 2) — **immer der eigene Probe-Pipename**, nie der Produktions-Broker (zwei Broker auf einem Namen stahlen sich still Clients; Produktion verweigert per FIRST_PIPE_INSTANCE)
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
  Tusche-Einzelmarken-Piktogramme. Tragfähig: der Graph selbst erzählt mit
  Licht/Tiefe und bildet Position/Breite/Stärke ab. → `docs/design-stand.md`
- **Musterblatt-Beispieldaten ≠ Plugin-Daten** — HTML-Demo-Werte wandern nie
  in das Plugin (Bauplan §1.5).
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

Design-Phase lebt im **Browser** (Sekunden-Schleife), nicht im Plugin
(Minuten + UAC + FL-Neustart). **Aktive Richtung seit 17.08.: das PRISMA**
(User-autorisiert; Drehen = Selektion Tiefen/Mitten/Höhen, Klick = Bündeln;
Film-Compositing mit Blender-Cycles auf CPU) — Einstieg
`docs/NEXT-SESSION.md` + `docs/design-stand.md`. Geparkt, nie ungefragt
reaktivieren: Tiefenfeld, Bauplan 2.0, Kunstwerk-Studie. Kein Schritt zeigt
erfundene Diagnosewerte — fehlt ein Produktionsfeld, bleibt die Darstellung
verborgen.

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
4. **Meta:** Claudes freie Bilderfindung = Mode-Collapse („wie
   algorithmisch komponierte Musik — objektiv ok, aber egal"). Vision und
   Referenzen kommen vom User; Claude übersetzt treu, verdatet ehrlich,
   verfeinert unter seinem Auge. Einzige abgenommene Ästhetik:
   P01-Perkolation (scharfe Punktwolke, Bandfarben Amber/Magenta/Eis).

## Maschinen-Landmine

**Keine GPU-Batch-Render-Loops auf der Arc A770** — der Lüfter-Failsafe
bleibt nach solchen Loops auf 100 % hängen (überlebt Warm-Reboots; Fix =
`Win+Ctrl+Shift+B`, sonst echter Kaltstart). Playwright-/Beweis-Renders
laufen in Software (SwiftShader) und bleiben Einzelläufe.

## Read before working on

| Bereich | Zuerst lesen |
|---|---|
| Wie das Plugin heute funktioniert (Architektur, Datenfluss, IPC) | `docs/plugin-wissen.md` (hier) |
| Design-Prototyp, Verwürfe, Freeze-Stand | `docs/design-stand.md` (hier) |
| Neue Hauptansicht (Ziel, Grammatik, DoD) | `eq-copilot/docs/NAKAMA-SPECTRAL-FIELD-BAUPLAN.md` |
| DSP-/Realtime-/Broker-Regeln, Produktplan | `FL-EQ-Copilot-Recherche.md` (FL-Studio-Wurzel) |
| Aktuellster Plugin-Stand + Beweise | `eq-copilot/docs/M3A-BEFUND.md` (+ M0–M2, CS1) |
| Manuelle FL-Prüfungen (liegen beim User) | `eq-copilot/docs/FL-TESTANLEITUNG.md` |
| Benchmark-Mechaniken (Median-Basislinie, Zonen, Konvergenz) | `eq-copilot/docs/BENCHMARK-STUDIE-RESO-SMARTEQ-PROQ.md` |
| Offene Punkte (durabel, nie still löschen) | `docs/offene-punkte.md` (hier) |

## Arbeitsweise

- Code ist die einzige Wahrheit — Datei öffnen vor jeder Doku-/Memory-Behauptung.
- Fortschritts- und Fertig-Meldungen nur mit Beleg aus dieser Session
  (Testlauf, Render, Diff); nicht Verifiziertes explizit als offen nennen.
  Selbstaudit nach jedem Commit; größere Änderungssätze von einem frischen
  Verifikations-Subagenten gegen Bauplan + Grundgesetz prüfen lassen.
- Im Auftrag bleiben: keine Neben-Refactors, kein „Modernisieren" von
  Legacy-Namen oder Front nebenbei. Problembeschreibung ⇒ Befund liefern,
  erst auf Zuruf fixen.
- Breite mechanische Suchen parallel an Explore-Agenten delegieren und
  währenddessen weiterarbeiten; Design-Urteil bleibt im Hauptlauf.
- Out-of-scope-Funde → `docs/offene-punkte.md` (datierte Zeile mit ID).
- Nach großen Schritten: `docs/design-stand.md` bzw. `docs/plugin-wissen.md`
  nachziehen + Session-Memo ins Memory (`project_session_*.md`, Cap 6 —
  eine Lehre pro Datei, mit dem Warum; Falsches löschen statt stapeln).
- Alle Texte dieses Projekts (Docs, Commits, UI) auf Deutsch — bestehende
  Konvention des eq-copilot-Baums.
