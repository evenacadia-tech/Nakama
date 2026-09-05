# unit-07 — hostprobe/HostProbeFactory.cpp, hostprobe/HostProbeProcessor.{cpp,h}, probe/PipeProbeMain.cpp, sonde/SondeFactory.cpp, sonde/SondeProcessor.{cpp,h}, spike/AuxSpikeEditor.{cpp,h}, spike/AuxSpikeFactory.cpp, spike/AuxSpikeProcessor.cpp (file-scope + 2 Funktionen)
Gelesen: 92 von 92 Einheiten vollständig · Zeilen: 4416

## Befunde

- **[LOW]** `hostprobe/HostProbeProcessor.cpp:126-129` · integer → bounds · **Der Ereignisring indiziert mit einem vorzeichenbehafteten Zähler, der nie zurückgesetzt wird; nach dem Überlauf schreibt `merke` VOR den Puffer.** · Auslöser: 2^31 aufgezeichnete Ereignisse in einer Instanzlebenszeit. `schreibIndex` ist `int` (`HostProbeProcessor.h:239`) und wächst nur (`:129` `schreibIndex.store (i + 1)`); bei `i == INT_MAX` ist schon das `i + 1` vorzeichenbehafteter Überlauf (UB), danach ist `i` negativ und `i % kMaxEreignisse` (`:127`) in C++ **negativ** — `2147483647 % 512` ist 511, `-2147483647 % 512` ist −511. · Wirkung: Schreiben eines 64-Byte-`Ereignis` bis zu 511 Plätze (≈32 KiB) vor `protokoll[]`, also über die übrigen Member von `HostProbeProcessor` bzw. den davorliegenden Heap. Kein Wächter, kein Zähler. · Erreichbar: ja, aber nur über Laufzeit, nicht über eine Eingabe. Aufruferkette Audiothread → `HostProbeProcessor::nakamaBlockEmpfangen` (`:188`) → `merke` (`:117`); je Parameter-Flush ohne folgendes `processBlock` genau ein Ereignis (`:215-216`; der Flush ist im Brückenvertrag zugesagt: `third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch`, `beginneBlock(0)` … `uebergib()`), dazu je erkanntem Zeitsprung eines (`:318`, `:325`). Bei einigen hundert Ereignissen je Sekunde sind das Wochen Dauerbetrieb — für ein ausdrücklich als WEGWERF-MESSGERÄT gebautes, nie ausgeliefertes Bundle (`HostProbeProcessor.h:1`, `:18-21`, eigener Plugin-Code `NkHp`) unrealistisch. Deshalb LOW; **dieselbe Stelle in Produktcode wäre HIGH**. Der Gegenleser `ereignisseLesen` (`:480-486`) fängt den negativen Zähler zufällig ab (`jmin` macht `anzahl` negativ, die Kopierschleife läuft nicht) — der Schreiber tut es nicht. Fix ist eine Zeile: unsigned zählen oder `schreibIndex` schon beim Speichern modulo halten. · Beleg: `:126-129`, `HostProbeProcessor.h:239`, `:35` · Register: neu

- **[LOW]** `hostprobe/HostProbeProcessor.cpp:101-105` · nebenläufigkeit · **Der Seqlock des Messstands hat zwei Schreiber; „Generation gerade" heißt dann nicht mehr „stabil".** · Auslöser: ein `prepareToPlay` (Nachrichten-/Hostthread), das sich mit einem laufenden `zaehleBlock`/`nakamaBlockEmpfangen` (Audiothread) überlappt — Blockgrößen- oder Sampleratewechsel im Betrieb, Wechsel Echtzeit↔Offline-Render. · Wirkung: `prepareToPlay` fährt dieselbe `generation`-Sequenz wie der Audiothread (`:101`/`:105` gegen `:134`/`:171` und `:192`/`:447`) und schreibt `stand.samplerate`, `stand.kleinsterBlock`, `stand.groessterBlock` nicht-atomar. Überlappen zwei Schreiber, ist die Generation während einer laufenden Schreibphase gerade; `messstand()` (`:456-462`) liest dann `g1 == g2` und liefert eine **zerrissene** `Messstand`-Kopie als „konsistenten Schnappschuss" aus, ohne Wiederholung. Zusätzlich ist der `memcpy` (`:459`) gegen die nicht-atomaren Audiothread-Schreiber formal ein Datenrennen. Keine Speicherunsicherheit: die Folge sind falsche Zahlen in Anzeige und Termin-B-Bericht — ein Messgerät, das leise lügt. · Erreichbar: ja, sobald der Host `prepareToPlay` nicht strikt gegen `processBlock` serialisiert; JUCE sagt das zu, FLs tatsächliches Verhalten ist genau das, was dieses Gerät messen soll. · Beleg: `:97-106`, `:132-134`, `:171`, `:192`, `:447`, `:451-467`, `HostProbeProcessor.h:232-233` · Register: neu

- **[LOW]** `hostprobe/HostProbeProcessor.cpp:496-499` · logik/protokoll (Gegenpfad aktivieren↔abklingen) · **`messungZuruecksetzen()` wird nur im Brückenpfad eingelöst; ohne Bridge-Patch setzt der Knopf nichts zurück, die Oberfläche behauptet es trotzdem.** · Auslöser: ein Bau oder Host **ohne** den gepatchten VST3-Wrapper — der von `HostProbeProcessor.h:220-222` ausdrücklich vorgesehene Fall (`brueckeLiefert() == false`). · Wirkung: `messungZuruecksetzen` setzt nur die Flagge `zuruecksetzen` (`:498`); eingelöst wird sie ausschließlich in `nakamaBlockEmpfangen` (`:194-211`). `zaehleBlock` (`:132-172`), ohne Brücke der einzige laufende Zähler, prüft sie nie. Ohne Brücke laufen `bloecke`, `verarbeiteteBloecke`, `bloeckeFloat/Double`, `kleinsterBlock`/`groessterBlock` und der Ereignisring nach dem Reset unverändert weiter, während die Oberfläche „Messung zurueckgesetzt - der naechste Block faengt neu an" meldet (`hostprobe/HostProbeEditor.cpp:181`). Ein Termin-B-Bericht mischt danach zwei Messreihen. Verstößt gegen „keine toten Elemente / jedes Element meldet ehrlich einen Zustand". · Erreichbar: ja, Nachrichtenthread → Editor-Knopf → `messungZuruecksetzen`; die Flagge bleibt dauerhaft `true` stehen und schlägt verspätet zu, falls doch einmal ein Brückenblock eintrifft. · Beleg: `:496-499`, `:194-211`, `:132-172`, `HostProbeProcessor.h:220-222`, `:242` · Register: neu

- **[LOW]** `sonde/SondeProcessor.cpp:735` · nebenläufigkeit · **Der Control-Thread greift für eine einzige bool-Lesung nach genau der Sperre, die JUCE um jeden `processBlock` hält.** · Auslöser: jede `control_status`-Abfrage über den v3-Link, also eine Nachricht eines lokalen Prozesses (des Brokers) auf der Pipe. · Wirkung: `v3Status()` nimmt `getCallbackLock()`, um `isSuspended()` zu lesen. Der gepinnte JUCE-8.0.9-VST3-Wrapper hält dieselbe Sperre um `processBlock` (nachgemessen an `eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3890-3910`). Wird der nicht-echtzeitfähige Control-Thread innerhalb dieses Abschnitts verdrängt, wartet der Audiothread — Prioritätsinversion ohne Vererbung (Windows-`CRITICAL_SECTION`); je nach Blockgröße ein hörbarer Aussetzer. Der Abschnitt ist eine einzige bool-Lesung, das Fenster also klein, und der Weg ist der von JUCE dokumentierte; der Kommentar `:732-736` wählt ihn bewusst. Ausweg ohne fremde Sperre: `suspendProcessing` überschreiben und den Zustand in ein eigenes `std::atomic<bool>` spiegeln, wie es die Zeile darunter für `nonRealtime` schon tut. · Erreichbar: ja, ControlClient-Thread → `v3Status` (`:684`) → `:735`; Häufigkeit = Statuskadenz des Brokers. · Beleg: `:728-741`, `SondeProcessor.h:250` · Register: neu

## Gate-5- und Invariantenbeleg

1. **Audiothread ohne Sperre/Allokation/IO/Logging; Überlast verwirft Analyseframes, nie Audio — hält.**
   `SondeProcessor::processBlock` (`sonde/SondeProcessor.cpp:155-224`) nimmt keine Sperre, allokiert nicht und ruft nur `analyseQueue.veroeffentliche` (`:217`); die Queue ist vorallokiert und verwirft Übergröße und Rückstau als gezählten **Analyse**verlust (`core/StampedAudioQueue.h:419-431`), nie Audio. `nakamaBlockEmpfangen` (`:226-258`) beschreibt nur den vorallokierten `brueckeStand`. `HostProbeProcessor::processBlock` (`hostprobe/HostProbeProcessor.cpp:174-185`) → `zaehleBlock` (`:132-172`) → `merke` (`:117-130`): nur Zähler und ein fester Ring. Zwei Vorbehalte, beide oben als Befund geführt: `prepareToPlay` hält `analyseSchloss` (`sonde/SondeProcessor.cpp:130`), während der Worker es über `sendeP1`/`veroeffentlichen` hält — beide reihen nur ein (`core/ipc/ControlClient.cpp:784-816`), es blockiert also der Nachrichten-, nie der Audiothread; und die Callback-Lock-Konkurrenz aus Befund 4.
2. **Passthrough bitidentisch, 0 Samples Latenz, kein Tail — hält.**
   `HostProbeProcessor::processBlock` fasst den Puffer nicht an (`:174-179`, `:181-185`), meldet `setLatencySamples(0)` in Konstruktor und `prepareToPlay` (`:94`, `:99`) und `getTailLengthSeconds() == 0.0` (`HostProbeProcessor.h:183`). `SondeProcessor::processBlock` liest nur (`getReadPointer`, `:214-215`); die JUCE-Sicherheitsschleife `:222-223` läuft beweisbar null Mal, weil `isBusesLayoutSupported` (`:141-153`) `ein == aus` erzwingt und deaktivierte Hauptbusse ablehnt. Tail `0.0` (`SondeProcessor.h:149`). Latenz: die Sonde ruft kein `setLatencySamples`, verlässt sich auf die JUCE-Vorgabe 0 und sichert sie nur mit `jassert` (`:126`) — im Release unbewacht, aber durch Konstruktion 0. Kein Hörmarker in diesen Einheiten.
3. **Gate 5 (kein Telemetrie- oder Steuerframe steuert samplegenauen Gain oder erzeugt ein hörbares PRE/POST-Delta) — hält.**
   In beiden `processBlock` gibt es keinen Schreibpfad in den Hostpuffer. Der Audiothread liest aus dem v3-Bereich ausschließlich `v3Samplerate` (`sonde/SondeProcessor.cpp:205`), geschrieben nur von `prepareToPlay` (`:131`) — kein Pipe-Frame. `ControlClient`/`TelemetryClient` werden im Audiothread nirgends berührt; ein Weg Pipe → Audio existiert in diesen Einheiten nicht.
4. **NaN-Ehrlichkeit — hält.**
   `SondeProcessor::prepareToPlay` verriegelt nicht-endliche und unplausible Sampleraten fail-closed auf 0.0 (`:127-128`), und 0.0 schaltet die Auswertung ab (`:304-305`, `:345`). Die PLR-Rechnung `frame.truePeakPassageDb - lautheit.lufsI` (`:387`) hat auf beiden Seiten eine Endlichkeitszusage des Erzeugers (`src/AnalyseEngine.cpp:717-723`, `core/analysis/FeatureEngine.h:4044-4048`). `evidenzIntervallSetzen` weist Nicht-Endliches ab und klemmt (`core/analysis/FeatureEngine.h:1447-1453`); die Verlustdifferenz sättigt (`:494-495` mit `:57-61`). Im HostProbe-Bericht werden nicht-endliche Hostwerte (ppq, Tempo aus dem Kontext) von JUCE als `null` geschrieben, nicht als „nan" — nachgemessen an `juce_core/json/juce_JSON.cpp:500-512`; das ist genau die geforderte „null statt erfundener Null".
5. **State bleibt verlustfrei — hält.**
   `SondeProcessor::setStateInformation` (`:630-656`) übernimmt nie halb: `ignoriert` lässt den gehaltenen Stand unberührt (`:641-642`), sonst wandern Automat und Zustand unter **einer** Sperre (`:644-652`), und der Automat sieht den geladenen Stand vor dem `std::move`. `getStateInformation` (`:624-628`) schreibt unter derselben Sperre; `nakama::state::speichere` gibt bei `nurLesen` die Originalbytes zurück (`state/NakamaState.cpp:982-993`) — unbekannte Major-Versionen überleben einen Speicher-/Ladezyklus bytegleich. Der Gegenpfad speichern↔laden ist symmetrisch verriegelt. `updateTrackProperties` (`:260-284`) fasst `zustand.common.label` bewusst nicht an und prüft den untrusted Hostnamen fail-closed (`:41-55`).

## Ledger
hostprobe\HostProbeFactory.cpp:5-8 · clean
hostprobe\HostProbeProcessor.cpp:file-scope-1 · clean
hostprobe\HostProbeProcessor.cpp:14-24 · clean
hostprobe\HostProbeProcessor.cpp:26-35 · clean
hostprobe\HostProbeProcessor.cpp:37-48 · clean
hostprobe\HostProbeProcessor.cpp:52-80 · clean
hostprobe\HostProbeProcessor.cpp:83-95 · clean
hostprobe\HostProbeProcessor.cpp:97-106 · Befund 2
hostprobe\HostProbeProcessor.cpp:108-114 · clean
hostprobe\HostProbeProcessor.cpp:117-130 · Befund 1
hostprobe\HostProbeProcessor.cpp:132-172 · clean
hostprobe\HostProbeProcessor.cpp:174-179 · clean
hostprobe\HostProbeProcessor.cpp:181-185 · clean
hostprobe\HostProbeProcessor.cpp:188-228 · Befund 3
hostprobe\HostProbeProcessor.cpp:229-348 · clean
hostprobe\HostProbeProcessor.cpp:349-448 · clean
hostprobe\HostProbeProcessor.cpp:451-467 · Befund 2
hostprobe\HostProbeProcessor.cpp:469-494 · clean
hostprobe\HostProbeProcessor.cpp:496-499 · Befund 3
hostprobe\HostProbeProcessor.cpp:502-508 · clean
hostprobe\HostProbeProcessor.cpp:510-624 · clean
hostprobe\HostProbeProcessor.cpp:626-638 · clean
hostprobe\HostProbeProcessor.cpp:640-643 · clean
hostprobe\HostProbeProcessor.h:file-scope-1 · clean
hostprobe\HostProbeProcessor.h:75-76 · clean
hostprobe\HostProbeProcessor.h:88-89 · clean
hostprobe\HostProbeProcessor.h:104-104 · clean
hostprobe\HostProbeProcessor.h:105-105 · clean
hostprobe\HostProbeProcessor.h:115-115 · clean
hostprobe\HostProbeProcessor.h:158-162 · clean
hostprobe\HostProbeProcessor.h:file-scope-2 · clean
hostprobe\HostProbeProcessor.h:176-176 · clean
hostprobe\HostProbeProcessor.h:191-191 · clean
hostprobe\HostProbeProcessor.h:192-192 · clean
hostprobe\HostProbeProcessor.h:193-193 · clean
hostprobe\HostProbeProcessor.h:194-194 · clean
hostprobe\HostProbeProcessor.h:195-195 · clean
hostprobe\HostProbeProcessor.h:197-197 · clean
hostprobe\HostProbeProcessor.h:198-198 · clean
hostprobe\HostProbeProcessor.h:206-222 · clean
hostprobe\HostProbeProcessor.h:232-232 · clean
hostprobe\HostProbeProcessor.h:233-233 · clean
hostprobe\HostProbeProcessor.h:238-238 · clean
hostprobe\HostProbeProcessor.h:239-239 · clean
hostprobe\HostProbeProcessor.h:260-268 · clean
probe\PipeProbeMain.cpp:file-scope-1 · clean
probe\PipeProbeMain.cpp:27-42 · clean
probe\PipeProbeMain.cpp:49-61 · clean
probe\PipeProbeMain.cpp:63-85 · clean
probe\PipeProbeMain.cpp:88-97 · clean
probe\PipeProbeMain.cpp:99-187 · clean
sonde\SondeFactory.cpp:file-scope-1 · clean
sonde\SondeFactory.cpp:8-11 · clean
sonde\SondeProcessor.cpp:file-scope-1 · clean
sonde\SondeProcessor.cpp:20-30 · clean
sonde\SondeProcessor.cpp:32-39 · clean
sonde\SondeProcessor.cpp:41-55 · clean
sonde\SondeProcessor.cpp:57-61 · clean
sonde\SondeProcessor.cpp:63-68 · clean
sonde\SondeProcessor.cpp:71-110 · clean
sonde\SondeProcessor.cpp:112-120 · clean
sonde\SondeProcessor.cpp:122-139 · clean
sonde\SondeProcessor.cpp:141-153 · clean
sonde\SondeProcessor.cpp:155-224 · clean
sonde\SondeProcessor.cpp:226-258 · clean
sonde\SondeProcessor.cpp:260-284 · clean
sonde\SondeProcessor.cpp:286-290 · clean
sonde\SondeProcessor.cpp:292-422 · clean
sonde\SondeProcessor.cpp:448-597 · clean
sonde\SondeProcessor.cpp:598-610 · clean
sonde\SondeProcessor.cpp:613-621 · clean
sonde\SondeProcessor.cpp:624-628 · clean
sonde\SondeProcessor.cpp:630-656 · clean
sonde\SondeProcessor.cpp:658-682 · clean
sonde\SondeProcessor.cpp:684-761 · Befund 4
sonde\SondeProcessor.cpp:763-770 · clean
sonde\SondeProcessor.h:file-scope-1 · clean
sonde\SondeProcessor.h:79-161 · clean
sonde\SondeProcessor.h:184-191 · clean
sonde\SondeProcessor.h:file-scope-2 · clean
spike\AuxSpikeEditor.cpp:file-scope-1 · clean
spike\AuxSpikeEditor.cpp:16-40 · clean
spike\AuxSpikeEditor.cpp:42-48 · clean
spike\AuxSpikeEditor.cpp:50-58 · clean
spike\AuxSpikeEditor.cpp:60-68 · clean
spike\AuxSpikeEditor.cpp:70-177 · clean
spike\AuxSpikeEditor.h:file-scope-1 · clean
spike\AuxSpikeFactory.cpp:file-scope-1 · clean
spike\AuxSpikeFactory.cpp:5-8 · clean
spike\AuxSpikeProcessor.cpp:file-scope-1 · clean
spike\AuxSpikeProcessor.cpp:11-14 · clean
spike\AuxSpikeProcessor.cpp:16-33 · clean

## Nicht gelesen
keine

## Außerhalb meiner Einheiten
- `eq-copilot/plugin/hostbridge/NakamaHostBridge.h:311-315` — `ppqPosition` und `barPositionPpq` werden ohne Endlichkeitsprüfung aus dem Hostkontext übernommen, anders als `timeInSeconds` weiter oben, das ausdrücklich verworfen wird.
- `eq-copilot/plugin/hostprobe/HostProbeEditor.cpp:181` — der Text „Messung zurueckgesetzt - der naechste Block faengt neu an" ist die Oberfläche zu Befund 3.
- `eq-copilot/plugin/core/ipc/TelemetryClient.cpp:134-135` — `optional()` legt jedes gesetzte Float-Feld ohne `isfinite`-Riegel auf den Draht; heute garantieren die Erzeuger die Endlichkeit.
- `eq-copilot/plugin/vertrag/NakamaEvidenz.h:85-89` — `Stereosicht` trägt einen rohen Bandzeiger ohne Anzahl; der Verbraucher liest fest `Gitter::evidenzBaender` Einträge (`vertrag/NakamaEvidenz.cpp:281-286`).
- `eq-copilot/plugin/core/StampedAudioQueue.h:240` — `GenStrom` ist `StreamLayout<1, 131072, 2048, 16384>`; diese vier Zahlen sind der stille Vertrag hinter `veroeffentliche (&abgriff, 1, …)` in `sonde/SondeProcessor.cpp:217`.
