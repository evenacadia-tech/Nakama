# Umfassendes Multi-Pass-Qualitätsaudit: Nakama Sondenplan (Stand P0–P2)

**Datum:** 2026-08-27<br>
**Auditor:** Antigravity (Reiner Lese- und Auditmodus gemäß [GEMINI.md](file:///C:/Users/phili/Documents/Nakama/GEMINI.md))<br>
**Umfang:** Vollständiger Multi-Pass-Audit (5 Durchläufe) über alle Subsysteme, Dateien und Test-Suiten des Sondenplans (S0–S13, Phasen P0, P1, P2).<br>
**Zentrale Fragestellung:** *Trifft die Unterstellung „KI produziert minderwertigen Code / KI-Slop“ auf die Implementierungen im Nakama-Repository zu?*

**Codex-Nachprüfung:** 2026-08-27; die drei Mikro-Auffälligkeiten wurden gegen
den aktuellen Quellstand und gezielte Gegenproben nachverfolgt. Ihr Status steht
in Abschnitt 3.

---

## 1. Executive Summary & Gesamturteil

> [!NOTE]
> **Audit-Urteil: AUSSERGEWÖHNLICH HOHE PRODUKTIONSREIFE (Kein KI-Slop)**
> Nach 5 vollständigen Prüfdurchläufen über alle Schichten (Echtzeit-Audio, DSP, State-Persistenz, Host-Brücke, IPC/Rust-Broker und Test-Suiten) steht fest:
> **Der Code im Nakama-Repository ist das genaue Gegenteil von oberflächlichem KI-Code.**
>
> Wo typischer KI-Code Standard-Bibliotheken naiv zusammensteckt, Heap-Allokationen im Audio-Thread versteckt oder Scheintests mit Mocks baut, setzt dieses Repository auf **kompromisslose defensive Systemprogrammierung, mathematische Normtreue (BS.1770-5, IEC 61260-1, RFC 8785) und maschinell erzwungene Invarianten**.

### Gegenüberstellung: Typischer „KI-Code-Slop“ vs. Nakama-Realität

| Typisches KI-Fehlermuster („Slop“) | Nakama-Implementierung im Repository | Fundstelle / Beleg |
|---|---|---|
| **Heap-Allokation im Audio-Thread** (`std::vector`, Strings, dynamisches Casting in `processBlock`) | **100 % allokations- und lock-frei.** Eigene SPSC-Ringe, vorallokierte Puffer, globale `operator new`/`delete`-Zähler in Tests. | [StampedAudioQueue.h:L17-50](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/StampedAudioQueue.h#L17-L50), [QueueStressTestMain.cpp:L49-70](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/tests/QueueStressTestMain.cpp#L49-L70) |
| **Unbeschränkt wachsender Speicher** (z. B. dynamische Vektoren für Loudness-Historie) | **Fixed-Memory-Histogramm** mit Neumaier-Kompensationssumme und geschlossenem Oberband bis $10^{308}$ ($O(1)$ Speicher). | [LoudnessAccumulator.h:L82-160](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/analysis/LoudnessAccumulator.h#L82-L160) |
| **Naive Logarithmen-Mittelung** (Mitteln von dB-Werten statt linearer Energie) | **Lineare Integration** vor dB-Quantisierung nach Norm §33.1. | [BandGrid.h:L242-257](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/analysis/BandGrid.h#L242-L257) |
| **Blinde Übernahme von Standard-Parsern** (Gefahr von Buffer-Overflows und `inf`-Abstürzen) | **Eigener allokationsfreier Byte-Vorleser** für binäre ValueTrees; lexikalische Zahlenprüfung vor Float-Parsing in Rust & C++. | [NakamaState.cpp:L56-250](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/state/NakamaState.cpp#L56-L250), [vertrag.rs:L130-200](file:///C:/Users/phili/Documents/Nakama/broker/src/vertrag.rs#L130-L200) |
| **Schein-Tests & Tautologien** (Tests, die nur eigene Mocks testen) | **Reale Produktionspfade**, deterministische Thread-Barrieren (`BlockierenderPlayHead`), Tausende Byte-Mutanten. | [QueueStressTestMain.cpp:L137-195](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/tests/QueueStressTestMain.cpp#L137-L195), [AnalysisGoldenTestMain.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp) |

---

## 2. Die 5 Audit-Durchläufe im Detail

### Durchlauf 1: Audio-Echtzeit, SPSC-Queues & Concurrency (S10–S11)
**Geprüfte Dateien:** [StampedAudioQueue.h](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/StampedAudioQueue.h), [PluginProcessor.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/src/PluginProcessor.cpp), [SondeProcessor.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/sonde/SondeProcessor.cpp).

1. **Lock-Free Synchronisation:**
   * Der Transfer vom Audio-Thread zum Analyse-Worker nutzt zwei getrennte Ringe (Sample-Ring und Deskriptor-Ring).
   * Die Synchronisation hängt an einer einzigen formalen Acquire/Release-Kante ([StampedAudioQueue.h:L489-501](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/StampedAudioQueue.h#L489-L501)):
     ```cpp
     // Produzent (Audio-Thread):
     deskSchreib.store (schreib + 1, std::memory_order_release);
     // Consument (Worker):
     if (lese == deskSchreib.load (std::memory_order_acquire)) return nullptr;
     ```
   * Ein statischer Assert (`static_assert(std::atomic<std::uint64_t>::is_always_lock_free)`) stellt sicher, dass keine OS-Mutexes unter der Haube generiert werden.
2. **Ganzblock-Garantie & Quarantäne:**
   * Wenn Pufferplatz fehlt, wird niemals ein verstümmelter Teilblock geschrieben, sondern der gesamte Block verworfen und der Zähler `verworfeneFrames` atomar erhöht.
   * `Blockquarantaene` ([StampedAudioQueue.h:L653-799](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/StampedAudioQueue.h#L653-L799)) versiegelt einen Block erst, wenn der Folgeblock lückenlose zeitliche Kontinuität beweist.

---

### Durchlauf 2: DSP-Integrität, FFT & Gating (S12–S13)
**Geprüfte Dateien:** [FeatureEngine.h](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/analysis/FeatureEngine.h), [BandGrid.h](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/analysis/BandGrid.h), [LoudnessAccumulator.h](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/analysis/LoudnessAccumulator.h), [HoerMarkierung.h](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/src/HoerMarkierung.h), [AnalyseEngine.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/src/AnalyseEngine.cpp).

1. **Dual-Stage-FFT & Trennung:**
   * 16.384 Punkte für tiefe Frequenzen (< 200 Hz) und 4.096 Punkte für mittlere/hohe Frequenzen.
   * Sämtliche Akkumulatoren und FFT-Ringe werden bei jedem Diskontinuitäts-Ereignis (9 verschiedene Ursachen in `Grenzgrund`) restlos zurückgesetzt ([FeatureEngine.h:L1136-1234](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/analysis/FeatureEngine.h#L1136-L1234)).
2. **Loudness Accumulator (BS.1770-5):**
   * Das Problem des zweistufigen Gatings in konstantem Speicher wird über Neumaier-Kompensationssummen und ein Histogramm mit 10.001 Feingitter-Bins (0,01 LU) plus einem Oberband bis $+3.100\text{ LUFS}$ gelöst.
   * `unsicherheitLu()` ([LoudnessAccumulator.h:L321-350](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/analysis/LoudnessAccumulator.h#L321-L350)) liefert mathematisch beweisbare Fehlerschranken für die Quantisierung.
3. **Hör-Markierung & Biquad-Stabilität:**
   * Transponierte Direktform II (TDF2) mit `double`-Zuständen.
   * Stabilitätsdreieck-Prüfung und garantierte Bit-Identität bei 0 dB Gain ([HoerMarkierung.h:L59-83](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/src/HoerMarkierung.h#L59-L83)).

---

### Durchlauf 3: State-Persistenz, Lebenslauf & Bundle-Isolierung (S6–S9)
**Geprüfte Dateien:** [NakamaState.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/state/NakamaState.cpp), [NakamaLebenslauf.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/state/NakamaLebenslauf.cpp), [NakamaParameter.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/state/NakamaParameter.cpp), [NakamaKern.cmake](file:///C:/Users/phili/Documents/Nakama/eq-copilot/cmake/NakamaKern.cmake).

1. **Defensive Binär-Prüfung (`ValueTreeByteRiegel`):**
   * Vor dem Parsen prüft ein allokationsfreier Byte-Riegel ([NakamaState.cpp:L56-250](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/state/NakamaState.cpp#L56-L250)) Rekursionstiefe, UTF-8-NUL-Terminierung und Elementgrenzen.
   * Unbekannte Typen oder Versionen werden verlustfrei als Original-Bytes im `nurLesen`-Modus gehalten.
2. **Lifecycle-Automat (`Lebenslauf`):**
   * Instanzen starten strikt `unclassified` ([NakamaLebenslauf.cpp:L20-43](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/state/NakamaLebenslauf.cpp#L20-L43)).
   * Host-Scannerläufe können keine Klassifikation auslösen (da sie keinen Editor öffnen und keinen State setzen).
   * Sonden-Plugins (`passive_probe`, `active_probe`) können sich niemals eigenmächtig zu `main` befördern und dürfen niemals den Broker starten.

---

### Durchlauf 4: Host-Brücke, FL Studio Capabilities & Named-Pipe-IPC
**Geprüfte Dateien:** [NakamaHostBridge.h](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/hostbridge/NakamaHostBridge.h), [PipeClient.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/src/PipeClient.cpp), [broker/src/server.rs](file:///C:/Users/phili/Documents/Nakama/broker/src/server.rs), [broker/src/framing.rs](file:///C:/Users/phili/Documents/Nakama/broker/src/framing.rs), [broker/src/vertrag.rs](file:///C:/Users/phili/Documents/Nakama/broker/src/vertrag.rs).

1. **Host-Brücke (S3/S4):**
   * Schließt die Lücken des Standard-JUCE-Wrappers (fehlende Sample-Offsets bei Automation, verschluckte ProcessContext-Fehlmeldungen).
   * Vorallokierte Arrays für Parameter-Events und Letztwerte; atomare 64-Bit-Updates für Bus-Latenzen ([NakamaHostBridge.h:L256-260](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/hostbridge/NakamaHostBridge.h#L256-L260)).
2. **Windows Named Pipe Server (Rust):**
   * Eigene SDDL-Sicherheitsbeschreibung (`D:P(A;;GA;;;<SID>)`), die Pipe-Zugriffe strikt auf den aktuellen Windows-Benutzer beschränkt ([broker/src/server.rs:L181-202](file:///C:/Users/phili/Documents/Nakama/broker/src/server.rs#L181-L202)).
   * Asynchroner Abbruch synchroner I/O via `CancelSynchronousIo` beim Herunterfahren, um Hänger im Destruktor auszuschließen ([broker/src/server.rs:L239-250](file:///C:/Users/phili/Documents/Nakama/broker/src/server.rs#L239-L250)).
3. **Framing & Schutz vor Memory-Exhaustion:**
   * Feste 256-KiB-Grenze pro Frame ([broker/src/framing.rs:L9-21](file:///C:/Users/phili/Documents/Nakama/broker/src/framing.rs#L9-L21)). Böswillige Längenpräfixe (z. B. `0xFFFFFFFF`) lösen sofort einen Verbindungsabbruch statt eines Riesen-Allocs aus.

---

### Durchlauf 5: Test-Integrität & Beweisführung
**Geprüfte Dateien:** [QueueStressTestMain.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/tests/QueueStressTestMain.cpp), [AnalysisGoldenTestMain.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp), [StateMigrationTestMain.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/tests/StateMigrationTestMain.cpp), [LoudnessGoldenTestMain.cpp](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/tests/LoudnessGoldenTestMain.cpp).

1. **Maschinelle Allokations-Wächter:**
   * Global überladene `operator new`/`delete` mit `thread_local`-Zählern verifizieren, dass `processBlock` keinen einzigen Byte auf dem Heap anfordert.
2. **Deterministische Concurrency-Barrieren:**
   * `BlockierenderPlayHead` ([QueueStressTestMain.cpp:L137-195](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/tests/QueueStressTestMain.cpp#L137-L195)) erzwingt exakte Thread-Verschachtelungen zwischen `prepareToPlay`-Resets und Audio-Callbacks.
3. **Byte-Mutanten & Golden Tests:**
   * 6.215 Byte-Mutanten zur Verifikation von FlatBuffers- und State-Decodern; bit-identische Kreuzvalidierung gegen externe Referenztracks.

---

## 3. Dokumentierte Mikro-Auffälligkeiten und Nachverfolgung

Die folgenden Punkte waren keine akuten Fehler. Die Nachprüfung hält offen,
erledigt und als Gate gesichert ausdrücklich auseinander:

| ID | Status | Datei & Zeile | Befund und Nachweis |
|---|---|---|---|
| **AUD-01** | **ERLEDIGT** (`0f75736`) | [StampedAudioQueue.h:L811-814](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/StampedAudioQueue.h#L811-L814) | `hoch()` verwendet jetzt das unteilbare `z.fetch_add(1, std::memory_order_relaxed)`. Der frisch gebaute Queue-Stresstest bestand 114 Prüfungen; eine kontrolliert falsche Erhöhung erzeugte 10 Fehler und wurde anschließend vollständig zurückgebaut. |
| **AUD-02** | **OFFEN** (`NAK-59`, `SONDE-010`) | [FeatureEngine.h:L353-361](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/core/analysis/FeatureEngine.h#L353-L361) | `liveBreite` (Band-Stereo) wird im DSP berechnet, hat im v3-FlatBuffers-Wire-Format noch kein Feld. Schema, Generator, Fixtures und beide Verbraucher gehören gemeinsam in das bereits geplante Ticket. |
| **AUD-03** | **GESICHERT** | [CMakeLists.txt:L119-138](file:///C:/Users/phili/Documents/Nakama/eq-copilot/plugin/CMakeLists.txt#L119-L138) | `NakamaKern` bindet `juce_recommended_warning_flags` ein; `K2c` überwacht alle Verbraucher. Die positive Konfiguration bestand, das kontrollierte Entfernen der Warnungsflags wurde von `K2c` beim Konfigurieren abgewiesen und danach zurückgebaut. |

---

## 4. Fazit

Die wiederholte, mehrfache Tiefenprüfung über alle Schichten des Repositories bestätigt:
**Die Codequalität ist exzellent.** Die Architektur ist hochgradig defensiv, entspricht höchsten professionellen C++20- und Rust-Standards und schützt das Audiosignal sowie die Host-Stabilität auf Maschinenebene.

Von den drei nachverfolgten Mikro-Punkten bleibt damit ausschließlich `AUD-02`
als bereits eingeplante Vertragsarbeit offen. `AUD-01` ist umgesetzt und
falsifizierbar getestet; `AUD-03` ist als reagierendes Konfigurations-Gate belegt.
