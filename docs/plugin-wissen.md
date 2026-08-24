# Plugin-Wissen — wie Plugin, Broker und Verträge heute funktionieren

> **Stand: 23.08.2026** (S12–13/SONDE-009: FeatureEngine v2, Zeit- und Bandverträge) · Version **0.3.0** (`project(… VERSION 0.3.0)` ==
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

`processBlock`, Reihenfolge ist Vertrag: `ScopedNoDenormals` → RMS-/NaN-Scan →
**Zeitstempel** (Hostbrücke, sonst Playhead) → Projektzeit-Fenster (nur bei
Play; Sprünge > 64 Samples gezählt) → **Analyse-Abgriff in die
StampedAudioQueue** → `lebenszeichen()` → Hör-Markierung als einziger Schreiber
des Puffers.

- **Analyseweg seit SONDE-008 (23.08.):** `nakama::echtzeit::StampedAudioQueue`
  statt `juce::AbstractFifo` (`plugin/core/StampedAudioQueue.h`, JUCE-frei wie
  die Hostbrücke). Zwei feste SPSC-Ringe — Samples (131 072 Frames je
  Stereo-Tap) und 2 048 Deskriptoren; ein Block geht **ganz oder gar nicht**
  hinein. Der alte FIFO schrieb bei Platzmangel einen Teilblock; der Worker sah
  danach einen lückenlosen Strom mit fehlender Zeit darin. Reicht ein Ring
  nicht, fällt der komplette Analyseblock, ein Zähler steigt, der nächste
  Deskriptor trägt `kFlagLueckeDavor` und ein neues `continuity_segment`.
  Blöcke über der festen Slotkapazität (16 384 Frames) sind `oversize_drop` —
  Audio läuft unberührt weiter. Der Backing-Store entsteht im **Konstruktor**;
  `prepareToPlay` meldet nur noch einen Neuanlauf (kein `fifo.reset()` vom
  Nachrichtenthread mehr in einen laufenden Leser hinein). 0 Samples Latenz,
  kein Tail, Busse nur mono/stereo mit Eingang == Ausgang. Nach außen nur
  Atomics; keine Sperre, Allokation, Datei, Pipe, Logging — gemessen in
  `EqCopQueueStressTest` §L (4 000 Blöcke wechselnder Größe, 0 Allokationen).
- **Ein-Block-Quarantäne im Worker (§53.7):** ein Block erreicht die Engine
  erst, wenn sein Nachfolger die Fortsetzung beweist. Ein Bruch (Queue-Lücke,
  Seek, Transportkante, Kanalwechsel) kostet genau EINEN Block, der Strom läuft
  weiter. ⚠️ **Stehende Projektzeit ist KEIN Bruch** — FL zerteilt Puffer bis
  1 Sample, die Teilstücke tragen dieselbe Zeit (NAK-56). 🔑 Die **Kehrseite**
  seit der T2-Nacharbeit (23.08.): geprüft wird bei **bekanntem** Transport —
  laufend *oder* stehend — dieselbe eine Frage: **bewegt sich die Zeit, muss sie
  es lückenlos tun.** Eine stehende Zeit sagt nichts, eine lückenlos
  fortschreitende ist eine Fortsetzung, ein *Sprung* ist eine Grenze (§32.3).
  Ist `spielt` **unbekannt**, bleibt die Zeit unbewertet — dort wäre dieselbe
  Regel ein Bruch je Block.
- **Anlaufwechsel (`startFolge`):** `prepareToPlay` erhöht den Anlauf
  **sofort** (`neustartAnfordern()`), nicht erst mit dem nächsten Audioblock —
  sonst sähen die Blöcke, die noch im Ring liegen, wie aktuelle aus und würden
  mit der **neuen** Samplerate analysiert (T2-3, 23.08.). Der Worker liest
  `aktuellerAnlauf()` je Block.
- **Hostbrücke verdrahtet (SONDE-008):** `EqCopilotProcessor` ist eine `Senke`;
  der Zeitstempel kommt aus dem echten `processContextPresent`, nicht aus dem
  tautologischen Playhead. Ohne Brücke (Konsolenziele) gilt der Playhead als
  Rückfallweg; ohne beides ist Transport **unbekannt**. `hatTransport` heißt
  seither „Transport ist bekannt", die Projektzeit trägt ihr eigenes
  Gültigkeitsbit — **NAK-24 geschlossen**.
- Lebenszeichen: Audiozeit/Wandzeit je Fenster; Verhältnis 0,5…1,2 zweimal ⇒
  `echtzeitOk`; > 1,5 (Freilauf/Offline) löscht den Beweis und meldet
  `freilaufKill`; Transportkante oder Lücke > 250 ms setzt zurück.

### 1.2 Hör-Markierung — die eine Audio-Ausnahme

`HoerMarkierung.h` (header-only; Konzept `eq-copilot/docs/HOER-MARKIERUNG-
KONZEPT.md`). Modi `aus / solo / puls`: **Solo** = nur das Problemband,
**Puls** = 250-ms-Kosinusflanken mit 375 ms Ruhe bei exakt 0 dB; alles
vorberechnet, der Audiothread kopiert höchstens einen POD-Auftrag aus 4
Ring-Slots.

**Verriegelung, gemessen:**
`erlaubt = istMainKlassifiziert ∧ (echtzeitOk ∨ testEchtzeit) ∧
(spieltGültig ∧ spielt) ∧ ¬isNonRealtime() ∧ (editorOffen ∨ testEchtzeit)` —
`testEchtzeit` setzt nur `testForciereEchtzeit()`, ohne Aufrufer im Produkt.
Der Editor beendet bei Fensterschluss, Samplerate-Wechsel, Freilauf und
Totmann 10 min (`PluginEditor.cpp:186-219`). Analyse-Abgriff VOR der Färbung;
Render bleibt bitidentisch (MarkierungTest T3/T4/T10).

⚠️ **`istMainKlassifiziert` ist seit S9 (SONDE-007b, 23.08.) der erste Term**
— §53.5 Satz 1: bis zur positiven Klassifikation ist der Entry audio-neutral,
und `legacy` ist „immer passiv". **Eine `legacy`-Instanz färbt damit nicht
mehr**; wer die Markierung will, wählt im Editor die Rolle `hub`. Der Term ist
die Atomic-Spiegelung von `Lebenslauf::audioAusnahmeErlaubt()` (§1.4c) und
wird **nicht** von `testForciereEchtzeit` umgangen: der Schalter umgeht nur,
was an der Wanduhr hängt. `EqCopLebenslaufTest` misst beide Seiten an Audio.

⚠️ **Der Transport-Term ist seit SONDE-008 (23.08.) `spieltGültig ∧ spielt`** —
das fail-open `(spielt ∨ ¬hatTransport)` ist gefallen (User 22.08., Hub `U10`:
„Nein, nur mit Signal"; NAK-35/NAK-24 geschlossen). Schließbar war er erst mit
der verdrahteten Hostbrücke: vorher konnte `hatTransport` „Transport unbekannt"
gar nicht ausdrücken (Entwurf §0.1). **In FL ändert das nichts** — dort war der
Zweig tot. Headless färbt ohne Playhead nichts mehr; Markierungs- und
Lebenslauftest tragen deshalb einen laufenden Transport.

### 1.3 AnalyseEngine — die Uhren

Worker (`PluginProcessor.cpp:367-441`): alle 50 ms die Analysequeue **Block für
Block** durch die Quarantäne ziehen (§1.1) und jeden versiegelten Block an
`verarbeite()` geben — bis SONDE-008 war es EIN Bulk-Zug aus dem FIFO ohne
Blockgrenzen. Jeder 5. Tick (~250 ms) `auswerten()` schwer, nur wenn neue
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
- **Seit SONDE-009 (23.08.) teilt sie ihre K-Gewichtung**: die zwei
  RBJ-Filterentwürfe standen als Lambdas in `AnalyseEngine.cpp` und liegen
  jetzt in `plugin/core/analysis/KGewichtung.h`, weil die FeatureEngine v2
  dieselbe Kette braucht. Dass es dieselbe Rechnung geblieben ist, misst
  `EqCopGoldenTest` — die Lautheit ist Teil seiner Kreuzvalidierung.

### 1.3b FeatureEngine v2 — die zweite Analyseschicht (SONDE-009, 23.08.)

`plugin/core/analysis/FeatureEngine.h`, JUCE-frei wie der Rest von `core/`.
Der Worker reicht **jeden versiegelten Block an beide** Engines: M1 bekommt die
Samples, die FeatureEngine zusätzlich den Deskriptor (sie braucht Zeit, Flags
und Kontinuität).

⚠️ **Warum zwei Engines und nicht eine.** Beide messen 221 Bänder à 1/24
Oktave — auf **verschiedenen Achsen**: M1 verankert bei 30 Hz (die Achse von
`tools/analyze-track.py`, Maßstab der Golden-Kreuzvalidierung), der v3-Vertrag
nach IEC 61260-1 bei 1000 Hz. Gemessen **1,2 % Versatz**, rund ein Fünftel
Bandbreite. M1 auf das v3-Gitter zu ziehen hieße, `EqCopGoldenTest` aufzugeben.
Der Vermerk steht im Kopf von `BandGrid.h`.

- **Zwei Auflösungsstufen** (16384 unter 200 Hz, 4096 darüber) — nicht nur
  Genauigkeit: zwei gleichzeitig offene Fenster verschiedener Länge (341 ms und
  85 ms) sind die Beweisfläche für die Fenstertrennung.
- **Neun Grenzarten** mit je eigener Ursache (`Grenzgrund`). Drop zählt als
  `continuity_segment`, alles andere als `transport_epoch` (§32.3 — „die
  Host-Zeitachse wird dadurch nicht fälschlich als Seek bezeichnet").
- **An jeder Grenze fällt alles Offene:** FFT-Fenster beider Stufen,
  Loudness-Zelle, 3-s-Historie, Korrelationsfenster, Fluss-Vorgänger **und der
  K-Filterzustand**. 🔑 Der Filterzustand ist die subtilste Form der
  Überbrückung — er trägt Audio über die Grenze, ohne dass ein Puffer wächst.
- **Möglicher Straddle** (§32.3): liegt die Schleifengrenze rechnerisch im
  Block und ist die PPQ→Sample-Abbildung unbewiesen (sie ist es immer —
  Capabilityreport S4 hat kein Golden), fällt der Block ganz. Fehlen Tempo oder
  PPQ, wird **nicht** vorsorglich getrennt: das träfe jeden Block einer
  laufenden Schleife.
- **NAK-29 im Erzeuger:** `nak29Verstoss()` verhindert, dass ein
  widersprüchlicher Transportstempel überhaupt entsteht.
- Fester Speicher, alles in `vorbereiten()`; Ereignisring auf 64 gedeckelt.
  Telemetrie über `EqCopilotProcessor::merkmale*()` — **ohne Anzeige**
  (NAK-57).

🔑 **Wie „an jeder Grenze fällt alles Offene" GEDECKT wird — zwei Beine mit
verschiedenen Fragen** (Stand 24.08., nach zwei T2-Runden; Manifest
`docs/beweise/SONDE-009.md` §10.1, §11.4, §12.1):

1. **`keineAkkusUeberleben()` fragt Auskünfte** — je Träger eine
   (`liveAkkuBelegteBaender()` und Nachbarn). Scharf, benennbar, und
   **strukturell blind für alles ohne Auskunft**. Genau daran ist es zweimal
   gescheitert: erst an den drei Bandakkus (T2-1), dann an den zehn
   Rahmen-Skalaren (T2R2-1) — beide Male war der Code richtig und das Bein sah
   nichts.
2. **G13, die Zwillingsprobe, fragt den FRAME.** Zwei Engines, dieselbe
   Blockfolge Zug um Zug, gegensätzlicher Inhalt davor (laut / digitale
   Stille), dieselbe Grenze, danach **bitgleiches** Audio in beide — ab da muss
   jeder Frame **feldgleich** sein. Verglichen wird mit
   `FeatureFrame::operator== = default`: memberweise, vom **Compiler** gepflegt,
   also ist ein künftig hinzugefügtes Feld automatisch gedeckt. Möglich ist das
   nur, weil `liveSamples` am Ende von `verarbeiteSamples()` **unbedingt**
   wächst, inhaltsunabhängig — die Zwillinge
   bleiben deshalb in exakt gleicher Kadenz.

⚠️ **Keins ersetzt das andere.** Bandgetorte Träger (`liveBreiteAkku` erreicht
den Frame nur über Bänder mit `liveAkku[b].n > 0`) sieht G13 nicht: der erste
Frame nach der Grenze fällt ~1–2 Blöcke später, das 4096-Fenster braucht 8.
Per-Sample-Träger (die `rahmen…`-Skalare) sieht dafür nur G13. **Nie eine
zweite Feldliste neben `grenzeZiehen()` anlegen** — zwei Listen laufen
auseinander, das ist die Ursache, nicht die Kur.

⚠️ **Ein `FeatureFrame` ist nicht byteweise vergleichbar** (NAK-69): seine
Füllbytes tragen unter `/O2` unbestimmte Werte, auch bei `FeatureFrame f {}` —
gemessen 24.08. an 18–21 abweichenden Bytes ohne einen einzigen
Feldunterschied. Wer ihn serialisiert, hasht oder als Golden einfriert, muss
das **feldweise** tun.

### 1.4 Diagnose und Snapshot-Datei

`Diagnose.cpp` — pur, zustandslos, auf der Snapshot-KOPIE; dieselbe Funktion
speist Hinweis-Knopf (1×/s), Snapshot-Datei und GoldenTest. Fünf Befundklassen
(`Diagnose.h:39-46`; Snapshot-Namen `PluginProcessor.cpp:855-859`):
`resonanz` (die zwei stärksten Kandidaten) · `mitten_loch` (500–2000 Hz
≥ 3 dB UNTER der Schulterlinie) · `mulm` (120–300 Hz ≥ 4 dB darüber) ·
`haerte` (2,5–5 kHz ≥ 4 dB darüber) · `hoehen_hype` (8–14 kHz > 1 dB über
2–6 kHz). Geometrie EINMAL in `ZonenRegeln.h:30-35`, geteilt mit der Engine;
eigenkurven-relativ, kein Zielkorridor.

`schreibeSnapshotDatei()` (`PluginProcessor.cpp:697-857`): `snapshot_version
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
- **Bundlevertrag aus der Target-Schicht (S9):** `setStateInformation` ruft
  `bundleVertrag()` (`PluginProcessor.h`), nicht mehr `Bundle::eqcp()` direkt.
  Welcher der drei Verträge gilt, sagt `NAKAMA_BUNDLE_MAIN` aus
  `plugin/CMakeLists.txt`; ein `#error` fängt ein Ziel, das `src/`
  mitübersetzt, ohne sich zu erklären.

### 1.4c Lifecycle-Klassifikation (`state::Lebenslauf`, §53.5, S9, 23.08.)

Im gemeinsamen Kern, JUCE-core-Code ohne Identitätskonstante. **Produktklasse
≠ Klassifikation:** die Produktklasse sitzt fest am Bundle (Suna ist immer
`passive_probe`), die Klassifikation entsteht erst aus dem restaurierten Stand.

| Ereignis | Ergebnis |
|---|---|
| Konstruktor / frische Instanz | `unclassified` — audio-neutral, kein Brokerstart |
| `lade()` ⇒ `ignoriert` (fremder Baum/Müll) | unverändert — der Host hat nichts restauriert |
| `lade()` ⇒ `nurLesen` | zurück auf `unclassified`, **auch aus `main`** |
| Schema-1 `sensor\|pre\|post` (migriert ⇒ `legacy`) | `legacy`, immer passiv |
| Schema-1 `hub` / Schema-2 mit `plugin_kind=main` | `main` |
| `setzeEditorOffen(true)` allein | **nichts** — klassifiziert nie für sich |
| `setzeBindung(…)` bei offenem Editor | die explizite Initialisierung: klassifiziert nach dem neuen Stand, in **beide** Richtungen |
| Sondenbundle, gültiger eigener Stand | seine Produktklasse; `main` ist unerreichbar |

🔑 **„Ein Scannerlauf klassifiziert nicht" ist keine Sonderbehandlung.** Es
gibt keine Zeile, die einen Scanner erkennt — es wäre auch keine ehrliche zu
schreiben. Die Regel folgt aus der Startbedingung: ein Scanner ruft nie
`setStateInformation` und öffnet nie einen Editor.

🔑 **„Bestätigter Schema-2-Main-State" ist der Leser selbst.** `lade()` hat die
Kind-Matrix §2.1 geprüft und damit ein `MainProject`-Kind gesehen, sonst wäre
der Stand read-only. Eine zweite Prüfung im Automaten wäre eine Kopie.

Zwei Verbraucher, mehr gibt es heute nicht: die Hör-Markierung (§1.2) und
`darfBrokerStarten()` = `main` **und** offener Editor. Letzteres ist ein
Vertrag ohne Pfad — in `plugin/src` gibt es keinen Spawn; `SONDE-010` hängt
ihn dort an, statt eine zweite Bedingung zu erfinden. Der Automat ist **nicht**
Teil des States: eine mitgespeicherte Klassifikation wäre eine zweite Wahrheit
neben `plugin_kind`.
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
`EqCopilot` und `EqCopHostProbe` — **seit SONDE-008 (23.08.) im Produkt
BENUTZT**: `EqCopilotProcessor` ist eine `Senke` und bezieht daraus den
Zeitstempel der `StampedAudioQueue` sowie den ehrlichen Transportstand
(§1.1/§1.2). Bis dahin war sie kompiliert, aber ohne Verbraucher; zweite
`Senke` bleibt `HostProbeProcessor`. Die Auswertung der Zeit (Epochen, Fenster)
kommt mit SONDE-009.
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

**Status beider Termine: gemessen am 22.08.** — Termin A 00:17/00:27 (zwei
Nebenwege samplegenau, recall-stabil), Termin B 12:45–13:27 (User legte
Aufbau, Claude fuhr Transport über den FL-MCP). Rohdaten `docs/beweise/termin-a/`,
`termin-b/`; Auswertung S4 `docs/beweise/SONDE-004.md`; maschinenlesbar
`eq-copilot/identity/host-capabilities-fl-v1.json` (Kanon A13). Kernbefunde:
Kontext in 100 % der Blöcke, alle Zeitfelder immer gültig, Seeks/Loops/Render
gemeldet, **Automation nie >1 Punkt je Block bei Offset 0 — FL zerteilt Puffer
bis 1 Sample**, nur float, Latenz 3 924/4 410 gemeldet (ohne Impulsgolden ⇒ Bit
unsupported), Smart disable bei offenem Fenster ohne Lücke; PDC in Termin A nie
ausgeübt und Kanalreihenfolge nicht unterscheidbar ⇒ beide Aux-Bits unsupported bis
Termin A2 (NAK-44). Stand: **2 supported, 8 unsupported**; Host FL Studio 2026 26.1.4.5589. Klicklisten `eq-copilot/docs/FL-TERMIN-{A-AUX-PDC,B-HOSTZEIT}.md`
tragen einen Stand-Hinweis mit Berichtigungen.

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

**22 Programm-Ziele + 1 Bibliothek** (`plugin/CMakeLists.txt`; gezählt am
23.08.: 18 benannte `juce_add_*`-Aufrufe plus 4 aus den beiden Zielfunktionen.
Bis dahin stand hier 19 — die Zahl war schon vor SONDE-008 um eins zu klein).
Seit S9
(SONDE-007b, 23.08.) sind es **drei Produkt-Bundles**: `EqCopilot` (`Eqcp`),
**`NakamaSuna`** (`NkPr`, Bundle „Nakama Suna.vst3") und **`NakamaProbeeq`**
(`NkAc`, „Nakama Probeeq.vst3"). Die beiden neuen entstehen aus einer
geteilten Quelle `plugin/sonde/` über die CMake-Funktion `nakama_sonde_ziel()`;
ihre Identität kommt wie die des Mains aus `identity/plugin-identities-v1.json`
(`cmake/NakamaIdentitaet.cmake`, kein Literal im Bauskript). Ihre
Produktklasse ist ein **Define** aus der dünnen Schicht — der geteilte Code
darf sie nicht wissen, er behauptete sonst für beide dasselbe. Beweise:
`EqCopSunaNullTest` / `EqCopProbeeqNullTest` (Kanon A15/A16, Passthrough +
Gegenpfad). Weiter: `EqCopilot` (VST3-Produkt) ·
`EqCopAuxSpike`, `EqCopHostProbe` (VST3, Wegwerf) · Konsolen `EqCopPipeProbe`,
`EqCopNullTest`, `EqCopGoldenTest`, `EqCopMarkierungTest`, `EqCopShot`
(seit 22.08. mit `--state <datei.bin>`), `EqCopPaintBench`, `EqCopAuxSpikeTest`,
`EqCopIdentityTest`, `EqCopHostProbeTest`, `EqCopHostContextTest`,
`EqCopSchemaTest`, **`EqCopStateMigrationTest`** (SONDE-006) und seit S8 die
Static-Lib **`NakamaKern`** (SONDE-007a). Sie trägt die vier geteilten Quellen
(`state/NakamaKanon`, `state/NakamaParameter`, `state/NakamaState`,
`vertrag/NakamaVertrag`), wird **einmal** übersetzt und über
`nakama_kern_anbinden(<ziel>)` an **14** Verbraucher gehängt (Zahl aus der
Configure-Ausgabe, nicht abgeschrieben — sie wächst mit jedem Ziel). Vorher übersetzten
sieben Ziele alle vier Quellen selbst (`nakama_state_anbinden()`) und
`EqCopSchemaTest` eine davon — 29 Übersetzungen derselben vier Dateien, jetzt 4.
Ihre Übersetzungsschalter hängen seit dem T2-Lauf (23.08.) ausdrücklich am Kern
(`juce_recommended_config_flags` + `_warning_flags`, **kein** `_lto_flags`):
als eigene Lib erbt er die PUBLIC-Schalter seiner Verbraucher nicht mehr und
übersetzte bis dahin als einziger Code im Baum unter `/W1`. Riegel **K2c**
(`cmake/NakamaKern.cmake`) hält es fest.
Binaries unter `eq-copilot/build/plugin/<Ziel>_artefacts/Release/`, die Lib
unter `eq-copilot/build/plugin/Release/NakamaKern.lib`.

**Kanon, 26 Beine (`tools/beweise.ps1`, Tabelle `$kanon`):** A1 NullTest · A2
GoldenTest · A3 MarkierungTest · A4 `cargo test` (inkl. JCS-Bein) · A5
`pruefe_v3_vertrag.py --abdeckung` · A6 `erzeuge_bandgitter.py --pruefen` · A7
`erzeuge_quantisierung.py --pruefen` · A8 `erzeuge_v3_fixtures.py --pruefen` ·
A9 `pruefe_flatc_drift.py` · A10 `erzeuge_fb_fixtures.py --pruefen` · A11
`pruefe_v2_schemas.py` · **A12 `erzeuge_state_fixtures.py --pruefen`** · **A13
`pruefe_host_capabilities.py`** · **A14 `pruefe_kern_identitaetsfrei.py`** ·
**A15 `EqCopSunaNullTest`** · **A16 `EqCopProbeeqNullTest`** · **A17
`pruefe_installer_manifest.py`** · **A18 `pruefe_installer_gegenpfad.py`** · B1
IdentityTest · **B2 StateMigrationTest** · B3 HostContextTest · B3b
HostProbeTest (ohne Argument) · B3c SchemaTest · **B4 `EqCopQueueStressTest`**
(SONDE-008) · **B8 `EqCopLebenslaufTest`** · **B9 `EqCopLoudnessGoldenTest`**
(SONDE-008) · **A19 `erzeuge_bandgitter_header.py --pruefen`** und **B5
`EqCopAnalysisGoldenTest`** (SONDE-009). Die Prüfzahlen stehen im jüngsten
Manifest (`docs/beweise/SONDE-009.md`: 28/28). Geplant, nicht gebaut:
B6 `EqCopDspGoldenTest`, B7 `EqCopTransactionTest`.

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

**Python-Werkzeuge (15, `tools/eq-copilot/`):** `pruefe_host_capabilities.py`
Capabilityreport gegen Rohdaten + v3-Vertragsform · `erzeuge_state_fixtures.py`
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
  NAK-26. **`hatTransport` Tautologie (NAK-24) und das Markierungs-fail-open
  (NAK-35) sind seit SONDE-008 geschlossen.**
- **Neu offen aus SONDE-008:** ob FL die Projektzeit über die Teilstücke eines
  zerteilten Puffers fortschreibt, ist ungemessen — beide Fälle sind abgedeckt,
  der reale zeigt sich am Zähler `analyseKontinuitaetsbrueche()` (NAK-56). Die
  neue Analyse-Telemetrie (Drops, Oversize, Brüche, `unsicherheitLu`) ist
  auslesbar, aber nirgends angezeigt (NAK-57) — die Oberfläche kommt aus Figma.
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
- ~~**Brücke ohne Senke:** `grep -rn "hostbruecke\|Senke" eq-copilot/plugin/src/` → 0.~~
  **Überholt seit SONDE-008 (23.08.):** derselbe grep trifft jetzt — der
  Prozessor ist die zweite `Senke`.
- **Kein Broker · leerer Spike-Ordner · keine Bindungsdatei · Binaries 18.08.:**
  `tasklist` (nur drei `FL64.exe`) · `%APPDATA%\evenacadia\nakama\` ·
  `ls broker/target/release/*.exe`. **paint() 2,47 ms:** `EqCopPaintBench`.
