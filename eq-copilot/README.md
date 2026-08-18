# Nakama — transparentes Sammler-VST3 + Tauri-Broker

Kanonischer Plan: `../FL-EQ-Copilot-Recherche.md` (Dateiname historisch — es ist der
Produkt- und Umsetzungsplan). Verbindlicher Plan der freigegebenen neuen
VST3-Hauptansicht: `docs/NAKAMA-SPECTRAL-FIELD-BAUPLAN.md`. Visuelle Referenz:
`design/nakama-spectral-field-vorentwurf.html` plus die geprüften Bilder unter
`design/vorentwurf-renders/`.

**Ist/Ziel ehrlich getrennt:** Der aktuelle Quellcode und das gestagte Bundle
tragen intern und teilweise sichtbar noch den Legacy-Namen `EQ-Copilot` und
verwenden die helle Material-Gerätefront. Das freigegebene, noch zu bauende Ziel
heißt **Nakama** und ist ein frei skalierbares, bildschirmfüllendes Spectral
Field mit textfreien Werkzeugkreisen, überlagerbaren Problemsymbolen,
manuellem Befundarchiv und fünf umschaltbaren Farbpaketen. Interne `EqCop*`-,
Schema- und Pipe-Namen dürfen für Kompatibilität bestehen bleiben.

**Grundgesetz:** Nakama berät nur. Es setzt keine Parameter, schreibt keine
Automation, verändert kein Audio (Passthrough sampleidentisch, 0 Latenz) und ruft
Claude nur auf bewussten Klick. Der Audiothread enthält keine Sperren, Allokationen,
Datei-/Netz-Zugriffe (Plan §12). Einzige dokumentierte Ausnahme seit 0.3.0: die
**Hör-Markierung** färbt auf bewussten Klick das Monitorsignal (verriegelt auf
bewiesene Echtzeit ∧ offenen Editor ∧ Transport ∧ `!isNonRealtime()`);
Render/Export bleibt bitidentisch (`EqCopMarkierungTest`).

## Layout (Plan §10.4)

```
eq-copilot/
  plugin/        JUCE 8 + CMake · transparentes VST3 (+ Pipe-Probe, Nulltest)
  schemas/       eq-ipc (v2) / eq-measurement / eq-report / eq-snapshot /
                 eq-aggregat — versionierte Verträge
  design/        tokens.json (maschinenlesbare Leitstand-Tokens) + Generator
  fixtures/      Golden-Audio und Host-Testfälle (ab M1)
  docs/          Lizenznotiz, M0/M1/M2-Befund, FL-Testanleitung
plugin-hub-app/src-tauri/src/eq_copilot/   Broker (Pipe-Server, Register,
                 Paare, Profilbindung, Aggregat)
plugin-hub-app/src/lib/eq-copilot/         Sensorübersicht (App-UI, M2)
```

## Bauen (Windows, VS 2022 Build Tools)

```powershell
$cmake = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -S eq-copilot -B eq-copilot/build -G "Visual Studio 17 2022" -A x64
& $cmake --build eq-copilot/build --config Release --target EqCopilot_VST3 EqCopPipeProbe EqCopNullTest EqCopGoldenTest EqCopMarkierungTest EqCopShot EqCopPaintBench
```

Artefakt: `eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3`
→ nach `C:\Program Files\Common Files\VST3\` kopieren (FL-Standard-Scanpfad).

## Prüfen (headless)

```powershell
py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav   # einmalig: Golden-WAVs
eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures
eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
cargo test --manifest-path plugin-hub-app/src-tauri/Cargo.toml eq_copilot
```

Ende-zu-Ende (v2 + Konflikt-Roundtrip) — die Probe läuft auf einem EIGENEN
Pipenamen, damit sie nie mit dem Broker einer laufenden Hub-App kollidiert
(zwei Broker auf einem Namen stahlen sich still Clients; der
Produktions-Broker verweigert das inzwischen per FIRST_PIPE_INSTANCE):

```powershell
# Terminal 1 (Rust-Broker, 30 s):
plugin-hub-app\src-tauri\target\debug\eqcop-broker-probe.exe 30
# Terminal 2 (C++-Client-Probe gegen den Probe-Namen):
eq-copilot\build\plugin\EqCopPipeProbe_artefacts\Release\EqCopPipeProbe.exe "\\.\pipe\evenacadia.eq-copilot.m2probe"
# Erwartet: PROBE OK v2 · KONFLIKT OK · KONFLIKT-ENDE OK (Exit 0)
```

## Meilensteine

M0: Beweis-Spike — Passthrough-VST3, Named-Pipe-Handshake, Schemas, Tokens,
Nulltest + Probe (`docs/M0-BEFUND.md`).
M1: Einzelinstanz-Messung — AnalyseEngine (Mehrfachauflösung §5.10.1,
BS.1770-LUFS, True Peak, Abdeckung, Resonanzkandidaten), Editor-Graph,
lokaler Snapshot, Kreuzvalidierung **GOLDEN OK 88/88** (`docs/M1-BEFUND.md`).
**M2 (dieser Stand): Multi-Instanz + Projektbindung** — Protokoll v2
(Messstand im Heartbeat, heartbeat_ack mit Konflikt-Flag), Projektzeit-Fenster
mit Sprungzähler, PRE/POST-Paar-Auswertung mit ehrlicher Herabstufung (§5.7),
persistente Profilbindung, Aggregat-Snapshot (`schemas/eq-aggregat.schema.json`),
Sensorübersicht in der Hub-App, Konflikt-UI „neue Kennung" (§8.4),
Pipe-Squatting-Härtung (`docs/M2-BEFUND.md`). FL-Prüfliste:
`docs/FL-TESTANLEITUNG.md` §11–15. Danach M3 Ziele + deterministische
Diagnose (Plan §11).
**Änderungssatz 1 (2026-08-14, metrics `m3-2026-08-14`):** P0-Rust-FFI-Fix
(Alignment/RAII/SAFETY) · Spektrum+Aktivität+Crest auf Kanalenergie
(Antiphase-Fix) · NaN-Riegel mit Zähler · Nyquist-Kappe der LTAS · Fixtures
v2 (L==R) + 6 adversariale GoldenTest-Fälle — **GOLDEN OK 178/178**
(`docs/CS1-BEFUND.md`; Auftrag: `docs/EQ-COPILOT-TECHNIK-UX-UEBERGABE.md`).
**EQ-Copilot Material Kit (2026-08-15):** vollständige eigenständige
Vektorfront aus `plugin/src/EqCopilotAssetKit.h` (Tokens-Gruppe `copilot`, Vertrag:
`design/ASSET-KIT.md`). Warme Metallfläche + schwarzes Analyseglas + sparsamer
Ember-Akzent; keine übernommenen Fremd-Assets und keine Fake-Potis. LUFS Short
und True Peak sind eigene Anzeigeschienen, Werkzeuge echte physische Tasten,
Kennzahlen getrennte Statuszellen. Jedes Bauteil ist unabhängig verschiebbar
und skaliert ohne Bitmap-Unschärfe; Fensterformat 750:520, Standard 1200:832.
Sichtprüfung ohne FL: `EqCopShot.exe <ziel.png> [breite]` rendert den Editor
mit echter 20-s-Messung offscreen als PNG.
**M3a + FPS-Fix (Nacht 15.08., metrics/diagnose m4-2026-08-15):** Sichtbare
Datenrate 4 Hz → ~20 Hz (`auswertenLeicht()` im Worker-Takt + Editor malt nur
bei neuer Snapshot-Revision, ruht im Leerlauf; `EqCopPaintBench` bewies
vorher, dass paint() mit ~2,4 ms unschuldig war). Kern-Mechaniken aus der
Benchmark-Studie (`docs/BENCHMARK-STUDIE-RESO-SMARTEQ-PROQ.md`): Median-Basislinie
±1/2 Okt statt Mittel, Zonen-Zeitverlauf (1-s-Aktiv-Ticks, geteiltes
`ZonenRegeln.h`) → Zonen-Karten sagen „in NN % der Musikzeit" und erreichen
ehrlich „hoch", Band-Perzentile P10/50/95 → Charakter-Zeile (ruhig ↔ Wellen),
Konvergenz-Anzeige **„KURVE STEHT"** (rein informativ), Notennamen in
FL-Zählung (116 Hz = A#3). Snapshot-Datei v3. Beweise: **GOLDEN OK 239/239**
(u. a. Halbzeit-Mulm ≈ 0,5 · wandernder Ton = zeitweise) · NULLTEST ·
pluginval 8 · SHOT (`docs/M3A-BEFUND.md`).
**Hör-Markierung (2026-08-16, Plugin 0.3.0, metrics/diagnose m4.1-2026-08-15):**
[Solo]/[Puls] je Befundkarte färben das Monitorsignal hörbar (Resonanz =
Bandpass-Kaskade, Zone = Butterworth-Paar; Puls im 250/375-ms-Raster,
Headroom-bewusst aus dem eigenen True Peak). Message-Thread baut den
kompletten Auftrag, Audiothread liest lock-frei aus dem 4-Slot-Ring;
Totmann-Timer 10 min, `Markierung aus` immer sichtbar; Analyse-Abgriff
sitzt VOR der Färbung. m4.1-Kalibrierfix: Zonen-Ticks brauchen ≥ −80 dB
je Region (vorher zählte Quasi-Stille als „Härte"-Zeit). Beweise:
**MARKIERUNGSTEST 30/30** (inkl. Freewheel/Render bitidentisch) · GOLDEN
weiter 239/239 (`docs/HOER-MARKIERUNG-KONZEPT.md`,
`kalibration/KALIBRIER-PROTOKOLL.md`).
