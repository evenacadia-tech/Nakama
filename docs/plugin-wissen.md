# Plugin-Wissen — wie Plugin, Broker und Verträge heute funktionieren

> **Stand: 22.08.2026** · Version **0.3.0** (`project(… VERSION 0.3.0)` ==
> `kPluginVersion`, Configure-Riegel `eq-copilot/CMakeLists.txt:3-22`) ·
> metrics/diagnose `m4.1-2026-08-15` · Snapshot-Datei v3 · IPC-Protokoll v2 ·
> **Host-State Schema 2** (`NakamaState`, seit SONDE-006 am 22.08.; liest Schema 1).
> **Installiert in FL ist das Bundle vom 16.08.** (moduleinfo „0.1.0", Hash
> `74D86BD5…`). Das Bundle im Build-Ordner (21.08. 21:11, mit Hostbrücke) ist
> nicht installiert und meldet **0.3.0** (Kanon-Lauf 21:10 nach dem Versionsriegel:
> 15/15 grün, `docs/beweise/KONTEXT-INVENTUR-2026-08-21.md`).
>
> Hier steht die **Architektur des heutigen Codes**: Plugin, Broker, Verträge,
> Beweise. Produkt und Entscheide → `CLAUDE.md`; Sondenfamilie im Entwurf →
> `docs/FL-Nakama-Sonden-Design-Entwurf.md`, `docs/bauaufteilung-sonden.md`.
>
> Namen: **Nakama Gen** (Main), **Probeeq** (aktive Sonde, vollwertiger EQ),
> **Suna** (passive Sonde), Bundle künftig „Nakama Studio". Im Code heißt
> heute alles `EqCop*` / „EQ-Copilot" / `Eqcp` — Legacy, Umbenennung ist
> NAK-30, kein Nebenbei-Refactor. Das gebaute Plugin ist der Vorläufer von
> Gen; seine Material-Kit-Front ist ein Provisorium.

Vier Threads im Plugin: **Audiothread** (`processBlock`) · **Worker** (besitzt
die AnalyseEngine) · **Pipe-Thread** (besitzt den PipeClient) · Message-Thread
(Editor).

## 1 · Produkt-Plugin `plugin/src/`

### 1.1 Audiopfad

`processBlock` (`PluginProcessor.cpp:91-209`), Reihenfolge ist Vertrag:
`ScopedNoDenormals` → RMS-/NaN-Scan → Hostzeit und Projektzeit-Fenster (nur
bei Play; Sprünge > 64 Samples gezählt) → **Analyse-Abgriff in den FIFO** →
`lebenszeichen()` → Hör-Markierung als einziger Schreiber des Puffers.

- FIFO `juce::AbstractFifo`, 65 536 Frames interleaved L/R, einmal alloziert;
  Überlast ⇒ `framesDropped`, nie warten. 0 Samples Latenz, kein Tail, Busse
  nur mono/stereo mit Eingang == Ausgang. Nach außen nur Atomics; keine
  Sperre, Allokation, Datei, Pipe, Logging.
- Lebenszeichen (`:216-277`): Audiozeit/Wandzeit je Fenster; Verhältnis
  0,5…1,2 zweimal ⇒ `echtzeitOk`; > 1,5 (Freilauf/Offline) löscht den Beweis
  und meldet `freilaufKill`; Transportkante oder Lücke > 250 ms setzt zurück.
  `hatTransport` ist im VST3-Pfad immer wahr (NAK-24).

### 1.2 Hör-Markierung — die eine Audio-Ausnahme

`HoerMarkierung.h` (header-only; Konzept `eq-copilot/docs/HOER-MARKIERUNG-
KONZEPT.md`). Modi `aus / solo / puls`: **Solo** = nur das Problemband,
**Puls** = 250-ms-Kosinusflanken mit 375 ms Ruhe bei exakt 0 dB; alles
vorberechnet, der Audiothread kopiert höchstens einen POD-Auftrag aus 4
Ring-Slots.

**Verriegelung, gemessen (`PluginProcessor.cpp:202-207`):**
`erlaubt = (echtzeitOk ∨ testEchtzeit) ∧ (spielt ∨ ¬hatTransport) ∧
¬isNonRealtime() ∧ (editorOffen ∨ testEchtzeit)` — `testEchtzeit` setzt nur
`testForciereEchtzeit()`, ohne Aufrufer im Produkt. Der Editor beendet bei
Fensterschluss, Samplerate-Wechsel, Freilauf und Totmann 10 min
(`PluginEditor.cpp:186-219`). Analyse-Abgriff VOR der Färbung; Render bleibt
bitidentisch (MarkierungTest T3/T4/T10).

### 1.3 AnalyseEngine — die Uhren

Worker (`PluginProcessor.cpp:279-331`): alle 50 ms FIFO leeren →
`verarbeite()`; jeder 5. Tick (~250 ms) `auswerten()` schwer, nur wenn neue
Samples kamen; sonst `auswertenLeicht()` (~20 Hz). Beide publizieren über
`fuelleBasis()` (EINE Quelle) mit monotoner `revision`; ohne neue Samples
publiziert niemand. `zonenTick()` je 1 s AKTIVER Zeit in `verarbeite()`.

- Welch: Bass 16384 (< 200 Hz) · Referenz 8192 (Kreuzvalidierungs-Achse) ·
  Mitten 4096 · Höhen 2048; Hann, Hop n/2; PSD = Kanalenergie-Mittel
  (L²+R²)/2. LTAS 1/24 Oktave, 221 Bänder 30 Hz…< 18 kHz,
  Nyquist-Kappe min(18 kHz, 0,95·Nyquist), darüber NaN.
- Schwellen versioniert in `AnalyseEngine.cpp:18-27`. NaN-Riegel:
  nicht-endliche Samples werden VOR jeder Rechnung ersetzt und als
  `nanErsetzt` gezählt. `snapshot()` ist der einzige threadsichere Einstieg.

### 1.4 Diagnose und Snapshot-Datei

`Diagnose.cpp` — pur, zustandslos, auf der Snapshot-KOPIE; dieselbe Funktion
speist Hinweis-Knopf (1×/s), Snapshot-Datei und GoldenTest. Fünf Befundklassen
(`Diagnose.h:39-46`; Snapshot-Namen `PluginProcessor.cpp:674-678`):
`resonanz` (die zwei stärksten Kandidaten) · `mitten_loch` (500–2000 Hz
≥ 3 dB UNTER der Schulterlinie) · `mulm` (120–300 Hz ≥ 4 dB darüber) ·
`haerte` (2,5–5 kHz ≥ 4 dB darüber) · `hoehen_hype` (8–14 kHz > 1 dB über
2–6 kHz). Geometrie EINMAL in `ZonenRegeln.h:30-35`, geteilt mit der Engine;
eigenkurven-relativ, kein Zielkorridor.

`schreibeSnapshotDatei()` (`PluginProcessor.cpp:516-724`): `snapshot_version
3`, alle Messfelder, Befunde, `raw_audio: null`; NaN/±inf ⇒ `null`; Ablage
`%LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\`. Kein Befundarchiv im
Plugin.

### 1.4b Host-State — Schema 2 (SONDE-006, 22.08.)

Vertrag `eq-copilot/schemas/state/nakama-state-v2.md`; Code `plugin/state/`
(JUCE-core, keine `JucePlugin_*`-Konstante, nicht für den Audiothread).
`PluginProcessor` hält `nakama::state::Zustand` unter `bindungMutex`
(`PluginProcessor.h`, Abschnitt „Bindung"):

- **Form:** ValueTree `NakamaState{schema=2}` mit `Common{schema=1}`
  (`instance_id`, `plugin_kind`, `measurement_position`, `label`, optional
  `pair_id`, `project_binding_id`), `MainProject{schema=1}` nur für `main`,
  `Parameters{schema=1}` nur für `active_probe` (109 Werte); `Dsp`/`Pairing`
  sind reservierte Namen (SONDE-015/016). Kind-Matrix und Bundle-Klassenmenge
  (`Bundle::eqcp()` = {main, legacy}) werden beim Laden erzwungen.
- **Laden** (`setStateInformation`): `EqCopilotState{schema=1}` wird **rein
  migriert** (`hub→main+insert`, `sensor|pre|post→legacy+…`, `sensor_id`
  bytegleich, nichts erfunden; Goldens `fixtures/state/schema2/`); Schema 2
  wird gelesen, unbekannte Eigenschaften bleiben im Baum und reisen beim
  Speichern zurück. Unbekanntes Major / verletzte Matrix ⇒ **read-only**:
  Originalbytes werden bytegleich zurückgegeben, `setzeBindung`/`neueSensorId`
  verweigern, die Pipe wird gestoppt, der Editor zeigt es (Kopf „READ-ONLY",
  Zelle „STATE READ-ONLY", Dauermeldung). Fremder Baumtyp/Müll ⇒ ignoriert.
- **Host-Dirty:** jede persistente Änderung (`setzeBindung` mit echter
  Änderung, `neueSensorId`) ruft `updateHostDisplay (ChangeDetails()
  .withNonParameterStateChanged (true))` — der VST3-Wrapper macht daraus
  `setDirty (true)`. Laden/Migration melden nicht. (Vor dem 22.08. gab es im
  Plugin keinen einzigen `updateHostDisplay`-Aufruf.)
- **v2-Brücke:** das `hello` trägt weiter `role` = `v2Rolle (common)`; die
  v3-Adresse (`hex32`) kommt mit SONDE-010 (NAK-40).
- **Parameterbestand** (`schemas/state/nakama-parameter-v1.json`, C++-Tabelle
  `NakamaParameter.cpp`, deckungsgleich gemessen): 5 global + 8×13 = 109 IDs
  `v1.global.*` / `v1.band.<slot>.*`; heute trägt **kein** Bundle Hostparameter
  (§53.8: der `Eqcp`-Eintrag ändert seine Parameterliste nicht).
- **`state_hash`:** SHA-256-Hex des RFC-8785-Kanons des DTO
  `{"dsp_schema_version":1,"parameters":{…}}`. `NakamaKanon` hat dafür einen
  **eigenen JSON-Leser** (`std::from_chars`) — JUCEs Zahlenleser flusht
  Subnormale und verweigert `""` als Schlüssel (gemessen am Korpus). Drei Beine
  messen gegen `fixtures/state/MANIFEST.json`: C++ (B2), Python `rfc8785`
  (A12), Rust `serde_json_canonicalizer` (`contract_cross_language.rs`).

### 1.5 PipeClient

`PipeClient.cpp`, eigener Thread: `hello` (protocol_version 2, Version,
host_pid, sensor{id, nonce, role, label, pair_id}, audio) → `reject` ⇒ Fehler;
angenommen wird **nur** ein `welcome` mit `protocol_version == 2` (`:219`; ein
v1-Broker gilt als „unerwartete Antwort", der v1-Leseschutz in `:252` ist
unerreichbar) → Heartbeat 1 Hz mit Stats + `messKompakt()` (Messstand inkl.
`hoermarkierung`) → `heartbeat_ack` (`konflikt` ⇒ „Kennung doppelt!",
Auflösung: neue UUID + Reconnect) → `bye`. Backoff 500 ms, verdoppelt bis
8000 ms (`EqCopilotIds.h:18-25`). Framing u32-LE + UTF-8-JSON, max. 262 144
Byte.

### 1.6 Editor — Material-Kit-Front (Provisorium)

`PluginEditor.cpp` + `EqCopilotAssetKit.h` (`skin::`, Tokens aus
`design/tokens.json`). Größe (`:176-183`): `setResizable(true, true)`,
Limits 600×416…1950×1352, festes Verhältnis 750:520, Start 1200×832, Timer
30 Hz. Der Timer ist Poll, kein Maltakt (`:195-267`): EIN Snapshot-Zug pro
Tick; `repaint()` nur bei neuer `revision`, UI-Änderung, Pipe-Statuswechsel
oder Meldungsfenster — im Leerlauf malt er nichts. paint() kostet 2,47 ms @
1200×832 — nie der Engpass.

## 2 · Hostbrücke und Wegwerf-Messgeräte

### 2.1 Hostbrücke (SONDE-003)

Der gevendorte JUCE-8.0.9-Wrapper (`build/_deps/juce-src/…/
juce_audio_plugin_client_VST3.cpp`) wird beim Configure gepatcht
(`third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch`, 163 Zeilen, 149
mit CR, `.gitattributes: *.patch -text`). Gate
`eq-copilot/cmake/NakamaBruecke.cmake:19-22,53-72` hasht den
CRLF→LF-normalisierten Inhalt: gepatcht `6e5d4660…` ⇒ No-Op, unberührt
`1374eb40…` ⇒ `git apply` + nachmessen, fremd ⇒ `FATAL_ERROR`.

Gegenseite `plugin/hostbridge/NakamaHostBridge.h`: `Bruecke` sammelt je Block
einen `Blockbefund` für eine `Senke`. `NAKAMA_HOST_BRIDGE=1` steht PUBLIC an
`EqCopilot` und `EqCopHostProbe` (`plugin/CMakeLists.txt:45,237`) — **im
Produkt kompiliert, aber unbenutzt**: `src/` hat keinen Treffer auf
`hostbruecke`; einzige `Senke` ist `HostProbeProcessor`
(`hostprobe/HostProbeProcessor.h:166`); Verbraucher SONDE-008/009.
`EqCopHostContextTest` (91) misst Gate und Abbildung gegen den echten
`Steinberg::Vst::ProcessContext`.

### 2.2 `hostprobe/` — Termin B, `NkHp`

`EqCopHostProbe` (`EqCop-Host-Probe.vst3`) misst je Block Context-Anwesenheit,
Gültigkeitsbits, Zeitsprünge, Offline-Render, Buslatenz und samplegenaue
Automation. Bericht `host-probe-<zeit>.json` nach
`%APPDATA%\evenacadia\nakama\spike\` (`HostProbeProcessor.cpp:502-507`).
Selbsttest `EqCopHostProbeTest`: **85** Prüfungen ohne Argument, **89** nur mit
PNG-Pfad, dessen Name „leerzustand" enthält — der zweite Bildbeweis leitet
seinen Pfad per `replace("leerzustand","messzustand")` ab
(`tests/HostProbeTestMain.cpp:726-768`); anderer Name ⇒ 1 rot (NAK-34).

### 2.3 `spike/` — Termin A, `NkSp`

`EqCopAuxSpike` (`EqCop-Aux-Spike.vst3`): Aux-Busse `priority_sidechain` und
`compare_pre`, misst Ankunft, Reihenfolge, PDC-Versatz, Recall
(Impuls-WAVs `fixtures/aux-spike/`). Bericht `aux-spike-<zeit>.json` in
denselben Ordner (`AuxSpikeProcessor.cpp:290-295`). Selbsttest
`EqCopAuxSpikeTest` (41), nicht im Kanon (NAK-37).

**Status beider Termine: nicht gelaufen** — der Spike-Ordner existiert und ist
leer, die Wegwerf-Bundles liegen nur im Build-Ordner. Klicklisten
`eq-copilot/docs/FL-TERMIN-{A-AUX-PDC,B-HOSTZEIT}.md`.

## 3 · Verträge

### 3.1 v2 — Vertrag des heutigen Plugins

Fünf Schemas in `eq-copilot/schemas/`, `$id`-Familie eingefroren:
`evenacadia.eq-copilot.ipc.v2` (Pipe), `.snapshot.v3` (Datei), `.aggregat.v1`
(Broker-Produkt), `.measurement.v1` und `.report.v1` (geplante M0-Verträge
ohne Code). Kein Binary lädt sie — deshalb Kanon-Bein A11
`pruefe_v2_schemas.py` (JSON, Metaschema 2020-12, `$id`-Menge);
`eq-snapshot.schema.json` war 15.–21.08. kein gültiges JSON (`ad6c233`).

### 3.2 v3-Baum — Sondenfamilie (SONDE-005a)

`eq-copilot/schemas/v3/`: `eq-ipc-v3.schema.json` (`$id
evenacadia.nakama.ipc.v3`, 17 Nachrichtenfamilien als `oneOf` mit
Discriminator, 47 `$defs`) · `reservierte-nachrichten-v1.json` (8 reservierte
Namen) · `quantisierung-v1.json` (3 Kodierungen, 61 Testvektoren) ·
`bandgitter/nakama_1_24_oct_30_18k_v1.json` (221 Bänder, hex64-Bitmuster) ·
`nakama_log64_v1.json` (64 Gruppen, exakte Partition der 221). **Textriegel**
= acht Regeln auf dem Rohtext VOR dem Parser (Liste in `schemas/v3/README.md`),
Fälle in EINER Datei `fixtures/v3/TEXTRIEGEL-FAELLE.json` (59, hex-kodiert).

### 3.3 FlatBuffers (SONDE-005b)

`schemas/v3/flatbuffers/nakama_telemetry_v1.fbs` (Namespace
`evenacadia.nakama.v3`, `file_identifier "NKT3"`, 5 Enums, 8 Tabellen, 47
Felder alle mit `id`, `root_type FeatureBatch`), `FELD-IDS.json`
(handgeschrieben), `WERKZEUG.json` (`flatc` auf Commit `7e163021…`, 25.12.19,
Rust-Crate gleich). `pruefe_flatc_drift.py` verlangt bytegleiche
Neugenerierung des Codegens und dieselbe Version an Compiler, Header, Crate
und ruft `pruefe_fbs_feldids.py` — das seit T2-Runde 4 als Prüfung 7 auch
hält, dass **jedes** Offsetfeld im `strukturriegel` des Rust-Beins steht
(`broker/src/telemetrie.rs`; Rusts Verifier kennt C++' „May not point to
itself" nicht).

### 3.4 Drei Leser, handgeschriebene Manifeste

Referenz `tools/eq-copilot/pruefe_v3_vertrag.py` (`jsonschema` 4.26; nur das
Urteil) · C++ `plugin/vertrag/NakamaVertrag.*` + `NakamaTelemetrie.*` via
`EqCopSchemaTest` · Rust `broker/src/vertrag.rs` + `telemetrie.rs` via
`broker/tests/contract_cross_language.rs` (beide: Urteil UND
Verletzungsmenge). Alle gegen dieselben **handgeschriebenen** Manifeste:
`fixtures/v3/MANIFEST.json` (153 = 36 gültig + 117 ungültig) und
`fixtures/v3/flatbuffers/MANIFEST.json` (T2-Runde 4: **51 = 9 + 42**; Runde 3:
47 = 9 + 38; davor 40 = 8 + 32 — die geltende Zahl steht im MANIFEST, nicht hier).

### 3.5 Identität (SONDE-001)

`identity/plugin-identities-v1.json`: Hersteller `evenacadia` / `Evna`; `main`
= `Eqcp`, Bundle `EQ-Copilot.vst3`, Component-CID
`ABCDEF019182FAEB45766E6145716370`, Controller-CID `ABCDEF011234ABCD…`,
`state_schema 1`; reserviert `NkPr` (Suna) und `NkAc` (Probeeq) mit CIDs.
`JUCE_VST3_CAN_REPLACE_VST2=0` ist Teil der Identität
(`plugin/CMakeLists.txt:43`). `NkSp` ausdrücklich nicht hier vergeben, `NkHp`
kommt nicht vor. `EqCopIdentityTest` (63) misst das gebaute `moduleinfo.json`,
prüft den CMake-Quelltext, rechnet die reservierten CIDs nach, hält die
State-Goldens.

## 4 · Broker `broker/`

Crate `eqcop-broker` 0.1.0 (lib `eqcop_broker`). Module: `aggregat` ·
`bindung` · `framing` (`MAX_FRAME_BYTES 262144`) · `generiert` (flatc-Code) ·
`protokoll` (`PROTOKOLL_VERSION 2`, `MIN_PROTOKOLL 1`) · `telemetrie`
(FlatBuffers-Leser) · `vertrag` (JSON-Schema-Engine) · `server` (privat).
Binaries `eqcop-broker.exe [--bindungen <pfad>]` (Standard
`%APPDATA%\evenacadia\nakama\eq-copilot-bindungen.json`) und
`eqcop-broker-probe.exe [sekunden] [pipe-name]` (Default `…m2probe`).

- Pipes: Produktion `\\.\pipe\evenacadia.eq-copilot.v1` (`lib.rs:29` ==
  `EqCopilotIds.h:18`), Probe `…m2probe`. Erste Instanz mit
  `FILE_FLAG_FIRST_PIPE_INSTANCE` (`server.rs:242`): fremder Besitzer ⇒ Start
  verweigert. SDDL nur aktueller User, `PIPE_REJECT_REMOTE_CLIENTS`.
- Handshake (`protokoll.rs:163-182`): Versionen 1..=2 angenommen, `welcome`
  spiegelt die angenommene Version, sonst `reject`. Register: Stats/Messstand
  schreibt nur die Besitzer-Nonce; `stale` nach 5 s; Getrennte bleiben
  sichtbar; zweite lebende Verbindung derselben `sensor_id` ⇒
  `heartbeat_ack{konflikt: true}`.
- PRE/POST-Paare (`lib.rs:312-445`): `unklar` / `wahrscheinlich` (Sprünge,
  Überlappung < 80 %, fremde `host_pid`, Aktivzeit-Differenz > 10 %) /
  `ausgerichtet`, `grund` nie leer. Aggregat atomisch nach
  `%LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\aggregat-<ms>.json`.
- Tests: **57** (`cargo test`, Stand `4f7182b`) — 51 Unit (davon `vertrag.rs` 15)
  + 6 Integration in `tests/contract_cross_language.rs`; `telemetrie.rs` hat keine
  eigenen `#[test]`, sein Beweis ist der Integrationstest.
- Betriebsstand 21.08.: kein Broker läuft, keine `evenacadia`-Pipe offen,
  Bindungsdatei nie angelegt, letzte Snapshots vom 16.08.;
  `target/release/*.exe` (18.08. 11:43) älter als `lib.rs`/`vertrag.rs`/
  `telemetrie.rs` (21.08.) — NAK-36; kein Autostart (NAK-13); das
  `hoermarkierung`-Flag wird nicht gelesen (NAK-10).

## 5 · Bauen und Beweisen

**15 CMake-Ziele** (`plugin/CMakeLists.txt`): `EqCopilot` (VST3-Produkt) ·
`EqCopAuxSpike`, `EqCopHostProbe` (VST3, Wegwerf) · Konsolen `EqCopPipeProbe`,
`EqCopNullTest`, `EqCopGoldenTest`, `EqCopMarkierungTest`, `EqCopShot`
(seit 22.08. mit `--state <datei.bin>`), `EqCopPaintBench`, `EqCopAuxSpikeTest`,
`EqCopIdentityTest`, `EqCopHostProbeTest`, `EqCopHostContextTest`,
`EqCopSchemaTest`, **`EqCopStateMigrationTest`** (SONDE-006). Die
State-Bibliothek hängt über `nakama_state_anbinden(<ziel>)` an jedem Ziel, das
den Prozessor kompiliert. Binaries unter
`eq-copilot/build/plugin/<Ziel>_artefacts/Release/`.

**Kanon, 17 Beine (`tools/beweise.ps1`, Tabelle `$kanon`):** A1 NullTest · A2
GoldenTest · A3 MarkierungTest · A4 `cargo test` (inkl. JCS-Bein) · A5
`pruefe_v3_vertrag.py --abdeckung` · A6 `erzeuge_bandgitter.py --pruefen` · A7
`erzeuge_quantisierung.py --pruefen` · A8 `erzeuge_v3_fixtures.py --pruefen` ·
A9 `pruefe_flatc_drift.py` · A10 `erzeuge_fb_fixtures.py --pruefen` · A11
`pruefe_v2_schemas.py` · **A12 `erzeuge_state_fixtures.py --pruefen`** · B1
IdentityTest · **B2 StateMigrationTest** · B3 HostContextTest · B3b
HostProbeTest (ohne Argument) · B3c SchemaTest. Die Prüfzahlen stehen im
jüngsten Manifest (`docs/beweise/SONDE-006.md`: 17/17). Geplant, nicht gebaut:
B4 `EqCopQueueStressTest`, B5 `EqCopAnalysisGoldenTest`, B6
`EqCopDspGoldenTest`, B7 `EqCopTransactionTest`.

Runner `pwsh -File tools/beweise.ps1 [-Bauen] -Ziel docs/beweise/<Ticket>.md
[-Anhaengen] -Titel '…'`. Exitcodes (`:43-48`): 0 grün · 2 ein Bein rot · 3
Voraussetzung fehlt · 4 Läufe grün, aber Binaries älter als Quellen.
Baustand-Scan (`:421-469`): EIN globaler „neueste
Quelle"-Zeitstempel über alle Quellorte gegen jedes Prüfbinary — ohne
`-Bauen` zu grob (NAK-25), mit `-Bauen` zählt das Urteil des Buildsystems.

**Nicht im Kanon:** `EqCopAuxSpikeTest` (41, NAK-37) · `EqCopShot <ziel.png>
[breite]` (echte Messung offscreen) · `EqCopPaintBench [breite] [frames]`
· `EqCopPipeProbe [pipe] [s]` (braucht einen laufenden Broker, immer
`…m2probe`) · `pluginval --strictness-level 8` (nur in `%TEMP%`, NAK-26).

**Python-Werkzeuge (14, `tools/eq-copilot/`):** `erzeuge_state_fixtures.py`
State-Korpus + MANIFEST (RFC-8785-Vektoren mit `rfc8785` als Referenz, DTOs,
Parametervertrag; `--pruefen` = bytegleich) · `erzeuge_fixtures.py`
Golden-WAVs + `golden-referenz.json` · `erzeuge_aux_spike_fixtures.py`
Impuls-WAV je Projektrate · `erzeuge_bandgitter.py` beide Gitter ·
`erzeuge_quantisierung.py` Quantisierungsvertrag · `erzeuge_v3_fixtures.py`
JSON-Korpus + MANIFEST · `erzeuge_fb_fixtures.py` Binärkorpus + MANIFEST (je
`--pruefen` = bytegleich) · `pruefe_v3_vertrag.py` Referenzbein ·
`pruefe_v2_schemas.py` v2-Riegel · `pruefe_flatc_drift.py` Codegen-Drift ·
`pruefe_fbs_feldids.py` Feld-ID-Disziplin ·
`erzeuge_testsong.py` acht Kalibrier-MIDIs · `smf.py` SMF-Schreiber ·
`verify_testsong.py` Verifikator mit fremdem Parser (mido).

## 6 · Bekannte Lücken und Landminen

- Installiert (16.08.) ≠ gebaut (21.08.); Install-Skript auf den 16.08.-Hash
  festgenagelt, `eq-copilot/install/` gitignoriert und nur auf dem Desktop —
  NAK-32.
- `tools/analyze-track.py` (Erzeuger der Golden-Referenz, `GoldenTestMain.cpp:3`)
  liegt nur in `C:\Users\phili\FL-Studio\tools\` — NAK-31.
- HostProbeTest 85 vs. 89 — NAK-34 · `EqCopAuxSpikeTest` ohne Kanon-Bein —
  NAK-37 · Broker-Binaries älter als Quellen — NAK-36 · kein Autostart —
  NAK-13 · `hoermarkierung` ungelesen — NAK-10 · Sensorübersicht (`.svelte` ohne Zuhause; NAK-12 am 21.08. geschlossen: Hub-App kein Produktteil) —
  NAK-12 · Baustand-Riegel zu grob — NAK-25 · `pluginval` nur in `%TEMP%` —
  NAK-26 · `hatTransport` Tautologie — NAK-24.
- Veraltete Kommentare: `plugin/CMakeLists.txt:3` („Vier Targets"),
  `tests/ShotTestMain.cpp:7` und `probe/PipeProbeMain.cpp:1` (Hub-App bzw.
  „Tauri-Broker" — gibt es nicht mehr). `PipeClient.cpp:252` (v1-Zweig) ist
  unerreichbar, weil `:219` nur ein v2-`welcome` annimmt.

## Gemessen am 21.08.2026

HEAD `c5f6833`, Arbeitskopie mit uncommitteten SONDE-005b-Änderungen.

- **0.3.0, Riegel, Bundles 0.1.0:** `grep kPluginVersion plugin/src/EqCopilotIds.h`
  · `eq-copilot/CMakeLists.txt:3-22` · `build/CMakeCache.txt` · `grep Version
  …/Resources/moduleinfo.json`.
- **NullTest 10 · Markierung 30 · Identity 63 · Hostkontext 91 · HostProbe 85
  · AuxSpike 41 · Golden 239:** die sieben `.exe` unter
  `build/plugin/*_artefacts/Release/`, in dieser Sitzung gefahren, alle Exit 0.
  HostProbe 89: `tests/HostProbeTestMain.cpp:726-768`.
- **Schema · Broker 57:** SchemaTest-Lauf am Stand `ca008f5` (53), seit `4f7182b` mehr Fixtures ·
  `grep -c '#[test]' broker/src/*.rs` (51) + HEAD
  `tests/contract_cross_language.rs` (5; Arbeitskopie 6).
- **14 Ziele · 15 Beine + 5 geplant · Exitcodes · v2 grün:** `grep juce_add_
  plugin/CMakeLists.txt` · `tools/beweise.ps1:206-282`, `:43-48`, `:421-469` ·
  `py -3.13 tools/eq-copilot/pruefe_v2_schemas.py`.
- **153 · 59 · 47 · 17 Familien · 47 `$defs` · 8 reserviert · 3
  Kodierungen · 221/64 · 8 Tabellen/47 Felder:** `json.load` über die
  Manifeste und `schemas/v3/**.json` · `grep "^table\|^enum"
  nakama_telemetry_v1.fbs` · `WERKZEUG.json`.
- **Brücke ohne Senke:** `grep -rn "hostbruecke\|Senke" eq-copilot/plugin/src/` → 0.
- **Kein Broker · leerer Spike-Ordner · keine Bindungsdatei · Binaries 18.08.:**
  `tasklist` (nur drei `FL64.exe`) · `%APPDATA%\evenacadia\nakama\` ·
  `ls broker/target/release/*.exe`. **paint() 2,47 ms:** `EqCopPaintBench`.
