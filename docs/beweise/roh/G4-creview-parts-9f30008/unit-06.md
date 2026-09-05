# unit-06 — core/ipc/PipeToken.{h,cpp}, core/ipc/TelemetryClient.{h,cpp}, core/ipc/WireEnvelope.{h,cpp}, hostbridge/NakamaHostBridge.h, hostprobe/HostProbeEditor.{h,cpp}, hostprobe/HostProbeFactory.cpp
Gelesen: 70 von 70 Einheiten vollständig · Zeilen: 3763 (Summe der Einheitenspannen; 2608 verschiedene Quellzeilen, weil die `file-scope`-Einheiten die ganze Datei überdecken)

Bedrohungsmodell LOCAL_UNPRIVILEGED, HEAD `9f3000881df632f4969d78551fb0a516b61288cb`. Kein CRITICAL, kein HIGH, kein MEDIUM.

## Befunde

- **[LOW]** `core/ipc/PipeToken.h:58-64` · logik/protokoll (fail-closed-Riegel umgehbar) · **Der C++-Zwilling des Probe-Pipe-Riegels `istProbePipename` prüft den Namen roh: weder Windows-Namensnormalisierung noch ein geschlossenes Alphabet hinter dem Präfix.** · Auslöser: ein Pipename der Form `\\.\pipe\evenacadia.nakama.v3.probe.\..\evenacadia.nakama.v3.<26-Zeichen-Token>` — `name.compare (0, praefix.size(), praefix) == 0` sagt „ja", Win32 löst `..`-Segmente in `\\.\`-Gerätepfaden auf, und das Öffnen landet auf dem PRODUKTIONSNAMEN. Zweiter Auslöser: `\\.\PIPE\evenacadia.nakama.v3.PROBE.x` und `\\?\pipe\evenacadia.nakama.v3.probe.x` — für den Kernel dieselbe Pipe, für diesen Riegel kein Probename (hier fail-closed, aber der Riegel entscheidet nachweislich über eine andere Zeichenkette als das Betriebssystem). · Wirkung: die maschinelle Durchsetzung von „Probe-Pipe ist nie Produktions-Pipe" (CLAUDE.md, Entwurf §48.3) hält auf der C++-Seite nicht; ein Probe- oder Lastbein kann auf der Produktionspipe landen. · Erreichbar: **unter dem Bedrohungsmodell nein** — alle heutigen C++-Aufrufer liegen in `tests/` (`tests/IpcLastMain.cpp:79`, `tests/SessionSoakMain.cpp:484` und `:1650`, `tests/Sonde012SourcesLatencyTest.cpp:305`) und beziehen den Namen aus `argv[1]`, also same-user, was Entwurf §48.4 ausschließt. Härtung, kein Angriffsweg. · Beleg: `PipeToken.h:58-64` gegen den bereits reparierten Zwilling `broker/src/transport/pipetoken.rs:38-61` (`normalisierter_pipename`, Kommentar „G3-STRCMP-001"); die `..`-Hälfte ist für den Rust-Zwilling experimentell bestätigt. · Register: **NAK-118** (offen) benennt ausschließlich `broker/src/transport/pipetoken.rs:38-43`; die Groß-/Kleinschreibungs- und `\\?\pipe\`-Hälfte ist über `G3-STRCMP-001` (NAK-142/NAK-121, geschlossen `28fb0f1a`) **nur in Rust** gebaut worden. Die C++-Fassung ist in keiner der beiden Zeilen genannt und hat beide Hälften nicht.

- **[LOW]** `core/ipc/TelemetryClient.cpp:280` (zusammen mit `:285-300`) · objektlebenszyklus/caller-contract · **Nach einem `stop()` AUS DEM CLIENTTHREAD bleibt `thread` joinable; das nächste `start()` weist einem joinable `std::thread` zu und ruft damit `std::terminate()`.** · Auslöser: der Klassenvertrag erlaubt genau diesen Aufruf — `TelemetryClient.h:116-121` sagt zu, `stop()` kehre „in JEDEM Zustand" zurück, „aus dem `helloProvider` heraus ohne Self-Join". `stop()` nimmt dann den Frühausstieg `if (ausDemClientthread) return;` (`:299-300`) und joint oder detacht nie; `start()` (`:270-283`) prüft nur `k->laeuft` und führt danach `thread = std::thread (...)` (`:280`) aus. `std::thread::operator=` ruft `std::terminate()`, wenn das Ziel joinable ist — und das bleibt es, bis jemand joint oder detacht, auch wenn der Betriebssystemthread längst beendet ist. · Wirkung: harter Prozessabbruch des Hosts (FL Studio); kein Speicherfehler. · Erreichbar: **heute nein.** Produktiv wird `start()` genau einmal im Konstruktor und `stop()` genau einmal im Destruktor gerufen (`src/PluginProcessor.cpp:159-160` und `:168-169`, `sonde/SondeProcessor.cpp:107-108` und `:118-119`), und kein Callback ruft `stop()`. Der Befund ist die Lücke zwischen zugesagtem und tragfähigem Vertrag. · Beleg: `TelemetryClient.cpp:270-283`, `:285-300`; Zusage `TelemetryClient.h:116-121`. · Register: neu (der wortgleiche Zwilling `core/ipc/ControlClient.cpp:678-693` und `:695-713` liegt außerhalb meiner Einheiten, siehe unten).

- **[LOW]** `hostprobe/HostProbeEditor.cpp:30-31` · konvertierung (uint32 → int) · **Die Latenzzeile castet die vom Host gemeldete Samplezahl nach `int` und zeigt ab 2^31 eine negative Samplezahl an.** · Auslöser: `IAudioPresentationLatency::setAudioPresentationLatencySamples` liefert `Steinberg::uint32`; der Wert reist unverändert über `Bruecke::setzeBuslatenz` (`hostbridge/NakamaHostBridge.h:256-261`) und `Messstand::LatenzEintrag::samples` (`hostprobe/HostProbeProcessor.h:131`) bis in `juce::String ((int) eintraege[bus].samples)`. Ein Host, der 2147483648 oder mehr meldet, erzeugt „Bus 0 = -2147483648". · Wirkung: die Anzeige des Messgeräts lügt über einen gemessenen Wert — gegen die eigene Regel 1 der Datei („Sie zeigt AUSSCHLIESSLICH Gemessenes", `HostProbeEditor.h:5-8`). Keine Speicherwirkung; `(int)` aus `uint32` ist auf MSVC implementierungsdefiniert, kein UB. · Erreichbar: nur über einen Hostwert; unter dem Bedrohungsmodell keine Angreiferwirkung, und der Befund liegt in einem reinen Diagnosewerkzeug (Termin B). · Beleg: `HostProbeEditor.cpp:30-31`, `NakamaHostBridge.h:256-261`, `HostProbeProcessor.h:131`. · Register: neu.

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren, Allokationen, Datei-, Pipe- und Netzzugriff und ohne Logging — hält.**
   `hostbridge/NakamaHostBridge.h` ist der einzige Audiothread-Schreibpfad in meinen Einheiten. `beginneBlock` (`:267-277`), `kontextAus` (`:291-326`), `punkt` (`:331-389`) und `uebergib` (`:401-431`) allozieren nicht (`puffer[512]`, `letztwerte[128]` und `latenzEingang`/`latenzAusgang[16]` sind Member, `:464-477`), sperren nicht (nur `std::atomic<std::uint64_t>` mit `memory_order_relaxed`, abgesichert durch `static_assert (std::atomic<std::uint64_t>::is_always_lock_free)`, `:483-484`), loggen nicht und fassen keine Datei und keine Pipe an. Die stabile Einfügung in `punkt` (`:378-383`) ist im schlechtesten Fall quadratisch über höchstens 512 Einträge und bricht davor über `if (anzahl >= kMaxParameterEreignisse) { ++ueberlaufImBlock; return; }` (`:374`) ab — kein unbegrenzter Aufwand, kein Überlauf des Puffers.
   `TelemetryClient::veroeffentlichen (const std::uint8_t*, std::size_t) noexcept` (`TelemetryClient.cpp:363-366`) ist der allokationsfreie Erzeugerpfad in die vorallokierte `P2Schleuse<8192>`. Die **allozierende** Überladung (`:368-374`, `std::vector<std::uint8_t> puffer;`) hat genau einen Produktionsaufrufer, und der ist der Analyseworker, nicht der Audiothread (`sonde/SondeProcessor.cpp:396`, innerhalb der `analyseQueue`-Schleife des Workers).
2. **Passthrough bitidentisch, 0 Samples Latenz, kein Tail — nicht berührt.** Keine Einheit meiner Zuweisung liest oder schreibt einen Hostaudiopuffer.
3. **Gate 5 (kein Telemetrie- oder Steuerframe steuert samplegenauen Gain oder erzeugt ein hörbares PRE/POST-Delta) — hält, an beiden Grenzen nachgemessen.**
   (a) Hostbrücke: die Parameterereignisse sind reine Beobachtung. `Blockbefund::ereignisse`, `::letztwerte`, `ParameterEvent::normalisedValue` und `Blockbefund::hatLetztenBlockwert` (`NakamaHostBridge.h:171-211` und `:385-388`) haben **keinen** Produktionsverbraucher — beide `Senke`-Implementierungen lesen ausschließlich `befund.kontext` (`sonde/SondeProcessor.cpp:226-258`, `src/PluginProcessor.cpp:629-667`); die einzigen Leser von `hatLetztenBlockwert` und `normalisedValue` liegen in `tests/`. Ein NaN oder ein negativer Offset aus dem Host wird deshalb gezählt (`:346-366`) und erreicht keinen Gain.
   (b) Telemetrie-Empfangspfad: `leerlaufLesen` reicht den Payload nur als `beiFrame (e.payload, e.payloadLaenge, e.kopf.schemaMinor)` weiter (`TelemetryClient.cpp:530-531`); produktiv landet er über `EqCopilotProcessor::v3Frame` in `SourcesModel::uebernehmeP2` (`src/PluginProcessor.cpp:1273-1279`, `src/SourcesModel.cpp:947`), also im UI-Modell. Kein Pfad in den Audiothread.
4. **NaN-Ehrlichkeit — hält.**
   `featureFrameAlsFlatbuffer` (`TelemetryClient.cpp:43-180`) verweigert den Frame (`return false`, keine halbe Serialisierung) bei nicht-endlicher `liveBreite` oder Werten außerhalb [0,1] (`:61-63`), bei halbem LUFS-I-Paar, bei nicht-endlichem `lufsI` oder `lufsIUnsicherheit` und bei `lufsIStatus` außerhalb 1..2 (`:66-73`). `Bruecke::punkt` verriegelt nicht, aber **zählt** (`++wertImBlock`, `:365-366`) und zieht dem ganzen Block `sampleAccurateAutomation` ab (`:425-428`) — genau die zugesagte Ehrlichkeit.
   Nachgeprüft und **kein** Befund: die zehn SONDE-013-Felder (`lufsM`, `truePeakDb`, `truePeakPassageDb`, `plrDb`, `lraLu`, `crestKurzDb`, `headroomP10/P50/P95Db`, `headroomFenster`) werden im Serialisierer nicht erneut auf Endlichkeit geprüft (`:142-173`). Ein nicht-endlicher Wert würde von beiden Lesern als `nicht_endlich` abgewiesen (`vertrag/NakamaTelemetrie.cpp:469-470` und `:538-539`; `broker/src/telemetrie.rs:607` und `:708`) und den Frame verlieren. Er ist aber nicht erreichbar: jede dB-Zahl entsteht hinter einer `> 0.0`-Wache vor dem `log10` (`core/analysis/FeatureEngine.h:4019-4088`, `:3547-3550`), und `plrDb` wird nur bei endlichem `lufsI` gebildet (`sonde/SondeProcessor.cpp:384-388`). Ohne Eingabepfad kein Befund; als Härtung erwähnenswert.
   Zusätzlich geprüft: `flatbuffers::Offset<T>` hat einen nullenden Default-Konstruktor (`flatbuffers/buffer.h:35`), die drei uninitialisiert wirkenden Offsets `schleife`, `stereo` und `headroom` (`TelemetryClient.cpp:83`, `:122`, `:148`) sind also auf jedem Pfad 0 und nicht Müll. Die Argumentfolge von `CreateBandwerte` (i16-Vektor vor f32-Vektor) und `CreateFrame` (22 Felder) stimmt mit `vertrag/generiert/nakama_telemetry_v1_generated.h:807-814` und `:1219-1242` überein.
5. **State bleibt verlustfrei — nicht berührt.** Keine Einheit meiner Zuweisung liest oder schreibt persistenten Zustand.

Zusätzlich am Wire-Parser nachgemessen — dem einzigen Pfad, den ein feindlicher lokaler Pipe-Gegenüber direkt füttert —, **ohne Befund**:
`envelopePruefen` (`WireEnvelope.cpp:94-189`) klemmt `frame_len` auf `[16, 262144]` (`:101-106`), verlangt `laenge >= 4 + frameLen` vor jedem Kopfzugriff (`:108-110`), rechnet `16 + payload_len` in **64 Bit** (`:118-120`) und erzwingt Gleichheit — damit ist `payload_len <= 262128`, und `crc32c (payload, payloadLen)` (`:166`) liegt beweisbar im Eingabepuffer. `StromLeser` hält `gelesen <= puffer.size()` (`:249` addiert nur die geprüfte `drahtlaenge`) und verdichtet bei `gelesen > 65536` (`:228-233`); die unverarbeitete Menge ist durch einen Frame plus einen Lesepuffer beschränkt, kein unbegrenztes Wachstum. `Ratengrenze` (`:263-291`) kann nicht durch 0 teilen (`maxImFensterIn == 0 ? 1u`), unterläuft nicht (`jetztMs >= aeltester` steht vor der Subtraktion) und indiziert `zeitpunkte` nur modulo seiner Größe. `envelopeSchreiben` (`:191-219`) weist `> kMaxPayloadBytes` ab, bevor es nach `std::uint32_t` verengt, und lässt `ziel` in diesem Fall unberührt. Dass die CRC vor dem Verstoßurteil gerechnet wird, ist mit `broker/src/transport/v3.rs:296` symmetrisch und von den Fixtures gebunden — Vertrag, kein Befund.
`aktuelleLogonSid` (`PipeToken.cpp:147-182`) schließt `token` auf **jedem** Rückweg (`:158`, `:164`, `:167`), gibt `breit` per `LocalFree` frei (`:177`) und leckt auch im `ConvertSidToStringSidW`-Fehlerpfad kein Handle (`:171-172`, dort ist noch nichts alloziert). Die ignorierte Rückgabe des ersten `GetTokenInformation` (`:155`) ist über `noetig == 0` (`:156`) abgefangen, der zweite Aufruf ist geprüft. Die ASCII-Großschreibung in `pipeToken` (`:134-135`) weicht von Rusts Unicode-`to_uppercase` (`broker/src/transport/pipetoken.rs:90`) ab, ist für SID-Zeichenketten aber wertgleich; beide Seiten speisen dieselbe Quelle ein (`TokenUser`, `broker/src/server.rs:136` und `:154`).
`HostProbeEditor::zeilen` erzeugt im Leerzustand die längste Liste (die Hinweiszeile `:142-143` fällt später weg), deshalb deckt die im Konstruktor aus `zeilen (Messstand {}, false)` gerechnete Fensterhöhe (`:191-192`) jeden späteren Zustand — der T2-Fall „49 px abgeschnitten" kann nicht wiederkehren.

## Ledger

- `core\ipc\PipeToken.cpp:128-140` · clean
- `core\ipc\PipeToken.cpp:142-145` · clean
- `core\ipc\PipeToken.cpp:147-182` · clean
- `core\ipc\PipeToken.h:file-scope-1` · clean
- `core\ipc\PipeToken.h:33-82` · Befund 1
- `core\ipc\TelemetryClient.cpp:file-scope-1` · clean
- `core\ipc\TelemetryClient.cpp:32-40` · clean
- `core\ipc\TelemetryClient.cpp:43-180` · clean
- `core\ipc\TelemetryClient.cpp:185-190` · clean
- `core\ipc\TelemetryClient.cpp:201-202` · clean
- `core\ipc\TelemetryClient.cpp:220-224` · clean
- `core\ipc\TelemetryClient.cpp:226-232` · clean
- `core\ipc\TelemetryClient.cpp:253-263` · clean
- `core\ipc\TelemetryClient.cpp:265-268` · clean
- `core\ipc\TelemetryClient.cpp:270-283` · Befund 2
- `core\ipc\TelemetryClient.cpp:285-340` · Befund 2
- `core\ipc\TelemetryClient.cpp:342-356` · clean
- `core\ipc\TelemetryClient.cpp:358-361` · clean
- `core\ipc\TelemetryClient.cpp:363-366` · clean
- `core\ipc\TelemetryClient.cpp:368-374` · clean
- `core\ipc\TelemetryClient.cpp:376-376` · clean
- `core\ipc\TelemetryClient.cpp:378-387` · clean
- `core\ipc\TelemetryClient.cpp:389-464` · clean
- `core\ipc\TelemetryClient.cpp:466-539` · clean
- `core\ipc\TelemetryClient.cpp:541-642` · clean
- `core\ipc\TelemetryClient.cpp:643-743` · clean
- `core\ipc\TelemetryClient.cpp:744-806` · clean
- `core\ipc\TelemetryClient.h:file-scope-1` · clean
- `core\ipc\TelemetryClient.h:34-34` · clean
- `core\ipc\TelemetryClient.h:36-146` · clean
- `core\ipc\WireEnvelope.cpp:file-scope-1` · clean
- `core\ipc\WireEnvelope.cpp:33-39` · clean
- `core\ipc\WireEnvelope.cpp:41-47` · clean
- `core\ipc\WireEnvelope.cpp:50-56` · clean
- `core\ipc\WireEnvelope.cpp:59-63` · clean
- `core\ipc\WireEnvelope.cpp:65-74` · clean
- `core\ipc\WireEnvelope.cpp:76-92` · clean
- `core\ipc\WireEnvelope.cpp:94-189` · clean
- `core\ipc\WireEnvelope.cpp:191-219` · clean
- `core\ipc\WireEnvelope.cpp:221-236` · clean
- `core\ipc\WireEnvelope.cpp:238-261` · clean
- `core\ipc\WireEnvelope.cpp:263-268` · clean
- `core\ipc\WireEnvelope.cpp:270-291` · clean
- `core\ipc\WireEnvelope.h:file-scope-1` · clean
- `core\ipc\WireEnvelope.h:86-229` · clean
- `hostbridge\NakamaHostBridge.h:file-scope-1` · clean
- `hostbridge\NakamaHostBridge.h:69-69` · clean
- `hostbridge\NakamaHostBridge.h:70-75` · clean
- `hostbridge\NakamaHostBridge.h:83-146` · clean
- `hostbridge\NakamaHostBridge.h:155-156` · clean
- `hostbridge\NakamaHostBridge.h:164-165` · clean
- `hostbridge\NakamaHostBridge.h:176-176` · clean
- `hostbridge\NakamaHostBridge.h:177-326` · clean
- `hostbridge\NakamaHostBridge.h:327-455` · clean
- `hostbridge\NakamaHostBridge.h:456-486` · clean
- `hostprobe\HostProbeEditor.cpp:file-scope-1` · clean
- `hostprobe\HostProbeEditor.cpp:16-19` · clean
- `hostprobe\HostProbeEditor.cpp:23-38` · Befund 3
- `hostprobe\HostProbeEditor.cpp:42-146` · clean
- `hostprobe\HostProbeEditor.cpp:148-157` · clean
- `hostprobe\HostProbeEditor.cpp:159-162` · clean
- `hostprobe\HostProbeEditor.cpp:165-194` · clean
- `hostprobe\HostProbeEditor.cpp:196-202` · clean
- `hostprobe\HostProbeEditor.cpp:204-214` · clean
- `hostprobe\HostProbeEditor.cpp:216-224` · clean
- `hostprobe\HostProbeEditor.cpp:226-276` · clean
- `hostprobe\HostProbeEditor.h:file-scope-1` · clean
- `hostprobe\HostProbeEditor.h:36-37` · clean
- `hostprobe\HostProbeEditor.h:41-50` · clean
- `hostprobe\HostProbeFactory.cpp:file-scope-1` · clean

## Nicht gelesen

keine.

## Außerhalb meiner Einheiten

- `eq-copilot/plugin/core/ipc/ControlClient.cpp:678-693` und `:695-713` — wortgleiche `start()`/`stop()`-Form wie Befund 2 (`if (ausDemClientthread) return;` ohne Join, danach `thread = std::thread (...)` ohne Joinable-Prüfung); der Kommentar `:697-699` nennt den Self-Join ausdrücklich als vermiedene Falle.
- `eq-copilot/plugin/core/ipc/ControlClient.cpp:1180-1186` und `:1262-1271` sowie `eq-copilot/plugin/core/ipc/TelemetryClient.cpp:630-631` und `:723-727` — an diesen vier Stellen schreiben beide Clients `zustand.status`, `serverPruefstatus` und `serverPid` ohne die Generationswache, die dieselben Funktionen sonst überall setzen (`TelemetryClient.cpp:548`, `:582`, `:785`, `:797`).
- `eq-copilot/plugin/hostprobe/HostProbeProcessor.cpp:451-467` — Seqlock, der `Messstand` per `std::memcpy` über nicht-atomaren, nebenläufig beschriebenen Speicher kopiert.
- `eq-copilot/plugin/core/ipc/IpcQueues.h:617-619` — `abholen` kopiert nur bei `laenge <= zielGroesse`; die Bindung des Ziels an `schleuse.slotGroesse()` in `TelemetryClient.cpp:743` hängt an dieser Zusage.
- `broker/src/transport/pipetoken.rs:38-43` — die Alphabetprüfung aus NAK-118 fehlt auch nach `G3-STRCMP-001` noch.
