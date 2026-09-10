# Nakama — unabhängiger Codeaudit vom 10.09.2026

**Ergebnis:** Zehn konkrete Befunde an Audio-, Lebenszyklus-, Persistenz-, Zustell- und Nachweisgrenzen; drei mit Priorität P1. Ein Funktionsfehler wurde in dieser Session gegen den originalen Produktionsheader reproduziert. Weitere Befunde sind durch nachvollzogene Quellketten belegt, keine behaupteten Laufzeitreproduktionen. Hinzu kommen ein strukturelles Ressourcenrisiko, Optimierungspotenzial und ausdrücklich offene Gegenbeweise.

**Auftrag:** Allgemeiner, rein lesender Audit parallel zu Fables Implementierung; Audio-Performance, Robustheit, Wartbarkeit und vermeidbare KI-typische Fehlermuster. Produktcode, vorhandene Tests, Pläne und Register wurden nicht geändert. Dieser Bericht ist keine Releasefreigabe und kein Urteil über Fables inzwischen weitergeschriebenen Stand.

## 1. Prüfstand und Grenzen

| Feld | Stand |
|---|---|
| Geprüfter Commit | `844b9c15935377e89d124ac33848e9cba1ecf4c2` |
| Commitzeit | 10.09.2026, 18:19:48 MESZ |
| Prüfort | Eigene lokale Kopie `C:/na-audit-20260910-844b9c1` |
| Aktiver Checkout | `C:/Users/phili/Projekte/Nakama`; von diesem Audit nur gelesen |
| Profil | deep: Quellprüfung, Aufrufketten, Testprüfung, ein isolierter Gegenbeweis |
| Team | Drei unabhängige Agenten für Audio/DSP, C++ State/IPC, Rust-Broker; Root für Testanbindung, Wartbarkeit, Installer und Gegenprüfung |
| Planrechnung | Frisch aus dem Snapshot: 33 von 45 Schritten abgenommen; SONDE-015 läuft noch |
| Inventar | 286 ausgewählte Quell-/Test-/Prüfdateien, 193.455 physische Zeilen; generierter Code ausgeschlossen |

**Inventar ist keine Behauptung, jede Zeile tief geprüft zu haben.** Die detaillierte Prüfabdeckung und die Grenzen stehen in den [Audio-/DSP-](packets/audio-dsp.md), [State-/IPC-](packets/state-ipc.md) und [Broker-Paketen](packets/broker.md). Root prüfte die übernommenen Befunde zusätzlich an ihren Quell- und Gegenwegen.

Der Startcheckout enthielt laufende Änderungen an `DspKern.*`, `CMakeLists.txt`, dem Kernprüfer und neue `NakamaTransaktion.*`. Diese uncommittierten Änderungen sind ausgeschlossen. Die noch ausstehende Verdrahtung des aktiven EQ in SONDE-015 wurde nicht als Defekt gewertet.

Source-Links sind auf den geprüften Commit fixiert. Fremdbibliotheken, Designprototypen, alle Testzeilen und alle Analyse-Fachmodelle sind nicht vollständig auditiert. Ein neuer Vollkanon, FL-Studio-Läufe, Sanitizer, Installation und lange Lastläufe wurden nicht ausgeführt. Der einzige neu gebaute Code ist ein kleiner Audit-Harness gegen einen unveränderten Produktionsheader.

## 2. Priorisierte Befunde

P1: vorrangig bearbeiten, weil Speicher-/Audio-Sicherheit oder bestätigte Persistenz betroffen ist. P2: relevanter Funktions-, Robustheits- oder Nachweisfehler. „Quellbelegt“ heißt nachvollzogene Auslösekette; es wird damit kein beobachteter Absturz behauptet.

| ID | Priorität | Befund | Evidenz |
|---|---|---|---|
| D1 | P1 | Audio-Markierungsring kann einen noch gelesenen Slot überschreiben | Quellbelegt |
| D2 | P1 | Abgelöste IPC-Callbacks können einen bereits zerstörten Prozessor verwenden | Quellbelegt; Callback muss Stoppfrist überschreiten |
| D3 | P1 | Bestätigte Quellenänderung hängt beim Speichern am offenen Editor | Quellbelegt |
| D4 | P2 | P0 meldet Ablehnung, behält den Befehl aber für spätere Ausführung | Quellbelegt |
| D5 | P2 | Neuer Broker-Snapshot kann durch einen älteren ersetzt werden | Quellbelegt; Sender/Consumer unabhängig geprüft |
| D6 | P2 | Projektwechsel lässt alte Experimente, Paare und Befunde im Modell | Quellbelegt |
| D7 | P2 | Stereo-Vergleichspegel ist nach 200 statt 400 ms bereit | Reproduziert: zwei verletzte Zeitgrenzen |
| D8 | P2 | Zonen-ID wird vor Bereichsprüfung von double nach int konvertiert | Quellbelegt; kein UBSan-Lauf |
| D9 | P2 | Rücknahmeereignis kann von einem gewöhnlichen Snapshot verdrängt werden | Quellbelegt |
| D10 | P2 | Vier-Bank-CPU-Nachweis misst tatsächlich zwei rechnende Banken | Quellbelegte Fehlbelastung des Prüfers |

### D1 — Slotbesitz im Audio-Markierungsring fehlt

**Stellen:** [Publisher](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/HoerMarkierung.h#L314), [Audioleser](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/HoerMarkierung.h#L377), [vier Slots](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/HoerMarkierung.h#L660), [UI-Aufrufer](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginEditor.cpp#L754).

**Auslöser:** Audio liest den Publikationszähler und wird vor oder während der Slotkopie unterbrochen. Der Message-Thread veröffentlicht vier weitere Aufträge und verwendet denselben nichtatomaren Slot erneut. Audio kann Modus und Filterdaten aus verschiedenen Schreibständen lesen. Release/Acquire veröffentlicht frühere Writes, verhindert aber die Slotwiederverwendung nicht. Eine Scheduler-Unterbrechung benötigt keine ungewöhnlich schnelle menschliche Bedienung.

**Wirkung:** Datenrennen und undefiniertes Verhalten im hörbaren Pfad. Ein konkreter Klick oder Absturz wurde nicht provoziert. Die Begründung „menschlich unerreichbar“ im Kopfkommentar ersetzt keinen Besitzvertrag.

**Gegenprüfung:** Auftragsbildung außerhalb von Audio und begrenzter Kopieraufwand sind richtig. Sequenzielle Markierungs-/Allokationsprüfungen belegen keine gleichzeitig stattfindende Slotwiederverwendung.

**Schließender Beweis:** Leser zwischen Zählerlesen und Kopie gezielt anhalten, Ring mindestens einmal umlaufen lassen, Konsistenz/Racefreiheit prüfen. Korrektur mit explizitem Slotbesitz oder geeigneter begrenzter Übergabe; keine wartende Schleife und kein Mutex im Audiothread.

### D2 — IPC-Laufzeit lebt länger als der Besitzer ihrer Callbacks

**Stellen:** [ControlClient::stop](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/ipc/ControlClient.cpp#L116), [TelemetryClient::stop](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/ipc/TelemetryClient.cpp#L313), [rohe Prozessor-Captures](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginProcessor.cpp#L111), [Destruktor](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginProcessor.cpp#L324).

**Auslöser:** Ein bereits gestarteter Provider-/Antwort-/Frame-Callback bleibt länger als die 2.000-ms-Stoppfrist aktiv oder wird entsprechend lange unterbrochen. stop() löst seinen Thread per detach() ab. Der Thread hält die interne Laufzeit per shared_ptr am Leben, die produktiven Lambdas halten den Prozessor dagegen nur als rohes this. Dieser kann inzwischen fertig zerstört sein.

**Wirkung:** Fortsetzung eines Callbacks auf freigegebenem Prozessor oder zerstörten Unterobjekten. Eine Verbindungsgeneration kann bereits begonnenen fremden Callbackcode nicht zurückholen.

**Gegenprüfung:** Selbst-Stop, begrenzter Stop und Client-Pimpl-Lebensdauer sind ausdrücklich behandelt. Das deckt die Lebensdauer des Callback-Empfängers nicht. Kein im normalen Betrieb beobachteter UAF wird behauptet.

**Schließender Beweis:** Produkt-Callback unmittelbar vor Besitzerzugriff festhalten, Prozessorabbau über die Frist führen, Callback freigeben; ASan-/Lebensdauerkontrolle. Begrenzten Host-Abbau und sicheren Callbackbesitz gemeinsam lösen. Ein Nullcheck des rohen Zeigers genügt nicht.

### D3 — ACK-Übernahme in Projektstate hängt am Editor-Timer

**Stellen:** [ACK wird nur vorgemerkt](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Ipc.cpp#L1063), [sourcesTick](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Ipc.cpp#L1108), [einziger produktiver Tick-Aufrufer](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginEditor.cpp#L247), [Statewriter](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/State.cpp#L63), [State-/Dirty-Übernahme](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Ipc.cpp#L1327).

**Auslöser:** User bestätigt Join/Unbind und schließt den Editor vor ACK plus nächstem Timer. Der ACK landet in bestaetigteSourcesCommands. Ohne Editor fehlt der einzige produktive Drain. getStateInformation() schreibt den bisherigen zustand; Reload leert die Warteliste.

**Wirkung:** Brokerseitig bestätigte Mitgliedschaft ist im gespeicherten Projekt nicht enthalten; die Dirty-Meldung fehlt. Das kurze Fenster zwischen ACK und nächstem Timer besteht auch bei geöffnetem Editor.

**Gegenprüfung:** Der [ProjectReload-Test](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp#L294) ruft nach ACK ausdrücklich sourcesTick() auf und prüft einen günstigeren Ablauf.

**Schließender Beweis:** Join und Unbind jeweils senden, Editor vor ACK schließen, ACK empfangen, ohne manuellen UI-Tick speichern und in neuer Instanz laden. Mitgliedschaft/Dirty müssen zur bestätigten Aktion passen. Persistenzabschluss braucht einen vom Editor unabhängigen Besitzer.

### D4 — false und spätere Ausführung widersprechen sich über Modulgrenzen

**Stellen:** [Retention vor Queueversuch](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp#L176), [false / Replay](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp#L218), [Aufrufer löscht Zuordnung](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Ipc.cpp#L1320).

**Auslöser:** P0-Queue voll. sendePersistenzP0() hält den neuen Befehl in inFlight, liefert aber false. Der Sources-Aufrufer entfernt daraufhin seine command_id-Zuordnung. Nach Reconnect wird der Befehl trotzdem gesendet und kann erfolgreich angewandt werden. Der ACK findet keine Zuordnung mehr.

**Wirkung:** Eine als nicht eingereiht gemeldete Aktion kann später wirken; ihre lokale Persistenznachführung fehlt. Unabhängig von D3.

**Gegenprüfung:** [IpcTestMain.cpp](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/IpcTestMain.cpp#L5937) verlangt gerade false, Retention und spätere erfolgreiche Anwendung. Der [Assistenttest](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/Sonde014AssistentTest.cpp#L921) prüft separat die false-Rückmeldung. Die Integrationssemantik ist damit nicht bewiesen.

**Schließender Beweis:** Volle Queue → Quellen-Join/Unbind → Reconnect → ACK → Save/Load als eine Kette. API-Zustände endgültig abgewiesen, zur Wiederholung angenommen und abgeschlossen eindeutig unterscheiden. Zusätzlich begrenzte Zulassung neuer Aufträge: Die 64er-Drahtqueue deckelt das davor gehaltene inFlight-Register nicht. Bereits angenommene Befehle nicht willkürlich verwerfen.

### D5 — Snapshot-Zustellung ist nach dem Commit nicht mehr geordnet

**Stellen:** [Flush entsperrt vor Zustellung](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/broker/src/coordinator/flush.rs#L137), [Sender](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/broker/src/transport/server_v3/griff.rs#L72), [Koaleszierung](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/broker/src/transport/server_v3/queues.rs#L164), [Consumer](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/SourcesModel.cpp#L1152).

**Auslöser:** Flush A committed einen älteren Snapshot und pausiert nach drop(_flush_guard). Flush B committed und sendet den neueren. A reiht danach den alten ein. Der Queue-Schlüssel ist jeweils session_snapshot; eine monotone Snapshotfolge reist nicht mit. C++ prüft Sitzung/Bindung, aber kein Alter innerhalb derselben Sitzung.

**Wirkung:** Ein alter Zustand kann den neuen ersetzen, auch schon in der Queue. Quellen-/Experiment-/Befundanzeige kann rückwärts springen. Korrekte DB-Commitreihenfolge verhindert diese Zustellreihenfolge nicht.

**Gegenprüfung:** Der bestehende Store-Racetest prüft die Commitprojektion. Sender und Consumer wurden unabhängig gegengelesen. Reentrant sichere Zustellung ohne Coordinatorlock ist ein berechtigtes Ziel; die Korrektur darf nicht einfach fremde Callbacks unter denselben Lock ziehen.

**Schließender Beweis:** Zwei Flushes, Barriere nach älterem Commit vor Enqueue, neueren zuerst zustellen; letzter angenommener Zustand muss der neue bleiben. Ordnungsbeleg bis Queue und Consumer führen.

### D6 — Projekt-Reload leert erweiterten Sitzungszustand nicht vollständig

**Stellen:** [projektReload](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/SourcesModel.cpp#L401), [Bereinigung bei Subscription](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/SourcesModel.cpp#L450), [Sicht gibt Listen aus](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/SourcesModel.cpp#L1560).

**Auslöser:** Das Modell enthält Experimente, Paare, Befunde und Evidenzrücknahmen. Ein akzeptierter neuer Projektstate ruft projektReload() auf. Diese Methode leert Quellen-/Verbindungsdaten, lässt diese Listen/Zähler aber stehen. Ein neuer ungebundener oder read-only State muss keine Subscription aufbauen; der vollständige Clear in beginneSubscription() kann ausbleiben.

**Wirkung:** Alte Sitzungsinformationen bleiben im neuen Projektmodell. controlEnde() macht Befunde zwar stale, beseitigt fremde Experimente/Paare aber nicht.

**Gegenprüfung:** Der Reload-Test kontrolliert Quellen, Nonce und Messwerte. Später ergänzte Listen sind nicht Teil dieses Gegenbeweises.

**Schließender Beweis:** Alle Listen/Rücknahmefelder vorbelegen, in ungebundenen/read-only State wechseln, Broker offline lassen; sofortige Sicht ohne alte Sitzungsobjekte. Vollständige gemeinsame Reset-Semantik herstellen.

### D7 — Stereokanäle werden als zusätzliche Messdauer gezählt

**Stellen:** [400-ms-Vertrag](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/analysis/Vergleichspegel.h#L35), [Zähler](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/analysis/Vergleichspegel.h#L119), [Readiness](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/analysis/Vergleichspegel.h#L314), [ein Aufruf je Kanal](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginProcessor.cpp#L777).

Der Kern verlangt 0,4 × fs Samples, erhält sie jedoch pro Kanal. 9.600 Stereo-Frames bei 48 kHz ergeben 19.200 gezählte Samples und Bereitschaft nach 200 ms. Die Zeitbasis muss Frames/Musikdauer zählen; beide Kanalenergien dürfen trotzdem in den Pegel eingehen.

**Frischer Beleg:** Kleiner MSVC-C++20-Harness bindet den unveränderten Produktionsheader ein und verwendet die originale Aufruftopologie je Kanal. Mono/Stereo wurden bei 199, 200, 399 und 400 ms geprüft. Mono ist erst bei 400 ms bereit; Stereo bereits bei 200 und 399 ms. Auch friereEin() akzeptiert diese zu kurzen Stereoabschnitte. Build erfolgreich, Prüflauf **Exit 1, zwei Verletzungen**.

[Rohausgabe](evidence/repro-vergleichspegel.txt) · [Harness](evidence/repro-vergleichspegel.cpp) · [Bau-/Laufkommando](evidence/repro-vergleichspegel.cmd). Das prüft den Originalkern, nicht den ganzen VST/Host.

**Schließender Beweis:** Die acht Harnessfälle nach Korrektur plus echter Prozessorpfad; Blockteilung und Mono/Stereo dürfen den Bereitschaftszeitpunkt nicht verschieben.

### D8 — Bereichsprüfung kommt nach der Konvertierung

**Stellen:** [DSP-DTO](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/state/NakamaParameter.cpp#L390), [Presetreader](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/state/NakamaPreset.cpp#L240).

**Auslöser:** Zonen-ID 2147483648 ist endlich/ganzzahlig, passt aber nicht in den hier verwendeten 32-Bit-int. Beide Leser prüfen zunächst nur Endlichkeit/Integralität und casten anschließend. Erst danach kontrolliert validiereZonen() den zulässigen Slotbereich. Der Rust-Gegenweg verwendet i64 und weist diesen Wert sicher ab.

**Wirkung:** Float-zu-Integer-Konvertierung außerhalb des darstellbaren Bereichs vor der vorgesehenen Ablehnung. Ein erwartetes MSVC-Ergebnis wie INT_MIN wäre keine portable Absicherung gegen undefiniertes Verhalten. Kein Speicherexploit oder beobachteter Absturz wird behauptet.

**Schließender Beweis:** DTO und Preset mit gültigen Rand-IDs, -1, 8, 2147483648 und großen sicheren JSON-Ganzzahlen durch Originalreader schicken: unveränderter Zielzustand, kontrollierte Ablehnung, passende Sanitizerprüfung. Bereich vor dem Cast prüfen.

### D9 — Rücknahmeereignisse teilen sich den Schlüssel mit Vollsnapshots

**Stellen:** [Rücknahme über Snapshot-Push](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/broker/src/coordinator/invalidierung_verdrahtung.rs#L384), [fester session_snapshot-Schlüssel](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/broker/src/transport/server_v3/griff.rs#L86), [Queueersetzung](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/broker/src/transport/server_v3/queues.rs#L164), [Wirkung der Rücknahme](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/SourcesModel.cpp#L1421).

**Auslöser:** Writer verzögert. evidence_invalidate wartet in seiner Queue. Ein anderer Flush reiht einen gewöhnlichen session_snapshot ein. Beide erhalten denselben Objektschlüssel, deshalb ersetzt der Vollsnapshot die Rücknahme. Der alte Aufrufer bekommt false; die persistente Zustellschuld bleibt bestehen.

**Wirkung:** Die Messachse des Consumers wird nicht wie vorgesehen invalidiert. Ein Vollsnapshot ersetzt diese Wirkung nicht allgemein: bei gleicher Epoch/Nonce werden frühere Messwerte übernommen. Ohne weiteren Evidence-Takt oder Resubscribe kann die Rücknahme ausstehen, obwohl gewöhnliche Snapshots ankommen.

**Gegenprüfung:** Die gespeicherte Schuld verhindert vielfach endgültigen Verlust; deshalb wird hier kein pauschaler Datenverlust behauptet. Vorhandene Tests prüfen direkte Zustellung beziehungsweise Schuld und Replay, nicht die echte Koaleszierung dieser beiden Nachrichtenarten. Zwei disjunkte Rücknahmen werden nicht zusätzlich als zweiter aktueller Gen-Schaden gezählt: der heutige Consumer invalidiert ohnehin global.

**Schließender Beweis:** Echte Writerqueue anhalten, Rücknahme und Vollsnapshot einreihen, freigeben; eigenständige Rücknahmewirkung muss ankommen oder vollständig in einer ausdrücklich absoluten Ersatznachricht enthalten sein.

### D10 — Vier belegte Slots sind kein Vier-Bank-Rechenbeweis

**Stellen:** [B6-Messabschnitt](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/DspGoldenTestMain.cpp#L4063), [Rampenkompatibilität](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/dsp/DspProgramm.cpp#L139), [Übergangswahl](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/dsp/DspKern.cpp#L322), [zweiter Bankdurchlauf nur bei Crossfade](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/dsp/DspKern.cpp#L667).

**Auslöser:** Das zweite Programm im Messabschnitt ändert nur Output-Trim. Das ist rampenkompatibel, erzeugt also keinen Crossfade mit zweitem Quellbankdurchlauf. Die Committed-Rampe endet schon im vorbereitenden 256-Sample-Stilleblock, die Candidate-Rampe im ersten Messblock. Die übrigen 3.999 Blöcke publizieren keine neuen Programme. freieSlots()==0 zählt auch bereits ausgediente und bereite Banken.

**Wirkung:** Der als Vier-Bank-Fall bezeichnete Wert misst durchgehend zwei aktive Bankdurchläufe; die behauptete zusätzliche Worst-Case-Rechenlast fehlt. Dies ist ein Fehler des Nachweises, keine aus diesem Befund abgeleitete Bankpool-Reparatur. Die Codekette wurde von Root und Audio-Agent geprüft; B6 wurde hier nicht ausgeführt.

**Schließender Beweis:** Tatsächlich inkompatible Programme auf beiden Pfaden mit zeitlich überlappenden Crossfades. Während jeder gewerteten Region aktive/quellende Bankidentitäten und echte Bankdurchläufe beweisen. Übergangsregionen separat und ohne Testsignalaufbau messen, ausreichend oft für Callback-Tails.
## 3. Ressourcenrisiko und gezieltes Optimierungspotenzial

### R1 — Historischer Brokerzustand wird vollständig in RAM aufgebaut

**Einordnung:** Strukturelles Wachstumsrisiko, keine gemessene Speicherbudgetverletzung und kein Beleg für einen klassischen verlorenen Speicherblock.

**Stellen:** [vollständiges Laden](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/broker/src/store/handle.rs#L301), [Restore der Evidenz](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/broker/src/coordinator/experiment_verdrahtung.rs#L1372), [Experiment-Restore](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/broker/src/coordinator/experiment.rs#L882).

Store-Leser materialisieren alle historischen Passagen, Experimente und Evidenzblobs. Die Evidenzgrenze von 32 gilt pro ClientKey; die Anzahl historischer Schlüssel ist damit nicht begrenzt. Nach Neustart müssen restaurierte historische Schlüssel keinen lebenden Client haben. Der Liveness-Abbau entfernt nicht automatisch solche verwaisten Historieneinträge. Auch terminale Experimente und ihr Übergangslog wachsen weiter: Die Grenzen von acht beziehungsweise 32 betreffen offene Experimente. Die auf 32 begrenzte Snapshotausgabe kommt erst nach Sammlung und Sortierung.

Ein dauerhaftes Archiv kann richtig sein. Daraus folgt keine Notwendigkeit, bei jedem Start seine gesamte Historie gleichzeitig im aktiven RAM zu halten. Zu prüfen: legitime große Datenbank mit vielen abgeschlossenen Experimenten und wechselnden Client-Nonces, Startspitze und stationärer Speicherverbrauch bei unverändert kleiner Zahl aktiver Clients. Paging, bedarfsgeladenes Archiv und begrenzter Arbeitscache wären mögliche Lösungen; keine ungeprüfte Löschung von Nutzerhistorie.

### O1 — Sonde berechnet zwei Analysepfade, obwohl sie aus einem nur Lautheit abholt

**Stellen:** [beide Blockverarbeiter](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/sonde/SondeProcessor.cpp#L367), [alter True-Peak-/Welch-Pfad](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/AnalyseEngine.cpp#L403).

Der Sondenworker bedient FeatureEngine und AnalyseEngine pro freigegebenem Analyseblock. Im untersuchten Sondenpfad wird aus AnalyseEngine anschließend nur lautheitFuerTelemetrie() verwendet. Trotzdem berechnet dieser Pfad zusätzlich True Peak und vier Welch-Spektren, während FeatureEngine eigene entsprechende Merkmale berechnet.

Allein die alte True-Peak-Schleife verarbeitet über ihre acht Phasen insgesamt 161 Taps pro Kanal und Frame. Für Stereo bei 48 kHz sind das aus dem Code abgeleitet 15,456 Millionen Tap-Multiply-Accumulates pro Sekunde und Sonde; bei zehn Sonden 154,56 Millionen. Das ist eine Operationszählung, kein gemessener CPU-Prozentsatz oder zugesicherter Einsparwert. Compileroptimierung und tatsächliche Konfiguration bleiben relevant.

**Empfehlung:** Eigenes Optimierungsticket für einen passend zugeschnittenen Lautheitspfad oder nachgewiesene Wiederverwendung. Reset, Gap-Behandlung, integrierte Lautheit, Unsicherheit und PLR müssen im Vergleich der Ausgaben erhalten bleiben. Zuerst den realen Workeranteil messen und bestehende fachliche Goldens als Gegenbeweis nutzen.

### O2 — Komplexität sitzt an denselben Grenzen wie die Funktionsfehler

Der frische Gesundheitslauf zählt im eigenen, eingeschränkten Quellumfang 28 Funktionen mit mehr als 200 Zeilen. Beispiele: SourcesModel::uebernehmeSessionSnapshot und Rust verbindung_bedienen mit jeweils 744, ControlClient::eineVerbindung mit 580 und processBlock mit 394 Zeilen. Das beweist allein keinen Fehler.

D3/D6 zeigen aber eine konkrete Wartbarkeitsfolge: Zustandsvalidierung, Sitzungswechsel, Anzeigeprojektion und Persistenzabschluss sind über große Methoden beziehungsweise verschiedene Lebenszyklen verteilt. Bei Erweiterungen wird ein weiterer Reset oder ein unabhängiger Drain leicht vergessen. D4 zeigt einen Bool-Rückgabewert mit widersprüchlichen Bedeutungen.

Eleganter Code entsteht hier durch eindeutigen Besitz, explizite Ergebniszustände, zusammengehörige Reset-/Commit-Pfade und kleine fachliche Operationen. Eine bloße Aufteilung nach Zeilenzahl, zusätzliche generische Abstraktionen oder flächige Umbenennungen würde diese Probleme nicht zuverlässig lösen.

### O3 — Der Gesundheitsprüfer deckt wesentliche neue Produktionsverzeichnisse nicht ab

[QUELLORTE](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/tools/plan/gesundheit.py#L176) enthält broker/src, plugin/src und plugin/core. Die neuen Bereiche plugin/state, plugin/dsp, plugin/sonde, plugin/vertrag und plugin/hostbridge liegen außerhalb dieses Zählumfangs: im Snapshot zusammen 31 handgeschriebene Dateien mit 13.560 physischen Zeilen.

Deshalb ist „0 Quelldateien über 2.000 Zeilen“ aus diesem Lauf nur innerhalb des Filters wahr. Beispielsweise hat state/NakamaState.cpp außerhalb des Filters 2.863 physische Zeilen. Das ist kein automatischer Größenverstoß gegen eine bislang nicht auf diese Datei angewandte Regel; es ist eine Lücke, wenn das Ergebnis als allgemeine Projektgesundheit gelesen wird.

**Empfehlung:** Den Quellumfang aus den tatsächlichen Buildzielen beziehungsweise einem überprüfbaren Inventar ableiten, generierte und fremde Quellen ausdrücklich ausschließen. Die Kommentarheuristik meldet 42 gegenüber einer Grenze von 30; enthaltene externe/API-Bezeichner und fehlende lokale Claude-Dateien machen daraus weder zwölf neue Slop-Stellen noch eine bewiesene Regression. Clippy wurde in diesem Audit nicht ausgeführt.

## 4. Noch offene Nachweise und Verdachtsfälle

Die folgenden Punkte sind **keine weiteren bestätigten Funktionsfehler**. Sie benennen sinnvolle nächste Gegenbeweise.

### M1 — Belastbare Host- und Callback-Performance ist noch offen

B6 summiert den gesamten Messloop einschließlich Testsignalaufbau und prüft nur eine positive, endliche Dauer. Das ist ein Smoke-/Vergleichswert; es beweist keine Deadline-Verteilung. Zusätzlich muss zuerst D10 korrigiert werden. Aus einem fehlenden festen CPU-Prozentsatz wird kein erfundener Produktvertragsverstoß abgeleitet.

Der [kanonische Kurzlauf A24](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/tools/beweise.ps1#L614) mit 16 Sonden, zwei Minuten und einem Neustart ersetzt für sich genommen nicht die längeren [Startbudgets in Abschnitt 49.3](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/docs/FL-Nakama-Sonden-Design-Entwurf.md#L3579): dort stehen unter anderem 16 Sonden über 60 Minuten und 32 über 30 Minuten. Diese Werte sind dokumentierte, änderbare Startbudgets; kein hier frisch gemessener Releasebeleg.

Ein späterer kontrollierter Messlauf sollte beispielsweise 1 Gen plus 10 Probeeq ebenso abdecken wie die skalierten Planfälle. Relevante Achsen sind 44,1/48/96/192 kHz, kleine/wechselnde/große Blöcke, Signal/Stille/Denormals, Automation, Bypass, DSP-Übergänge, Editor offen/geschlossen, Reconnect und Projekt-Reload.

Zu erfassen sind Callbackdauer im Verhältnis zu Blocklänge/Samplerate, Verteilung einschließlich Tails und Maximum, XRuns, CPU, Arbeitsspeicher, Threads/Handles, Queuefüllung/Drops und Rückkehr zum Ausgangsniveau nach Instanzabbau. Ein gleichzeitig stark bauender Rechner liefert dafür keine saubere Performancebasis. Dieser Audit hat solche Läufe bewusst nicht ausgeführt.

### M2 — Kleine Hostblöcke treffen auf zeitgesteuertes Queue-Polling

Die [StampedAudioQueue](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/StampedAudioQueue.h#L240) hat 2.048 Blockdeskriptoren. Der Gen-Worker wartet bei leerer Queue bis zu [50 ms](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Analyse.cpp#L337), die Sonde [20 ms](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/sonde/SondeProcessor.cpp#L435). Der Audiopfad weckt sie nicht für jeden Block.

Bei dauerhaften Ein-Sample-Blöcken entsprechen 2.048 Deskriptoren bei 48 kHz nur 42,67 ms und bei 192 kHz 10,67 ms. Damit kann bereits das Pollingfenster Analyse-Drops begünstigen, ohne dass der Rechner mit der Analyse selbst überfordert ist. Einzelne Grenzblocktests und absichtlich überlaufende Bursttests belegen diesen zeitlich realistisch getakteten Fall nicht. Erst ein entsprechender Dauertest klärt Häufigkeit, Gap-Verarbeitung und praktische Hostrelevanz. Daraus wird hier kein beobachteter Audioaussetzer behauptet.
### V1 — Statewriter-Headroom prüft Bytes, möglicherweise nicht alle Readergrenzen

[hatWriterHeadroom](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/state/NakamaState.cpp#L1094) prüft die resultierende Bytegröße. Der Reader besitzt zusätzlich Collection- und Gesamtknotengrenzen. Beim Synchronisieren kann eine bisher fehlende optionale Label-Property zu einem ansonsten erhaltenen unbekannten Statebaum hinzukommen.

**Zu widerlegender Verdacht:** Ein akzeptierter Baum direkt an einer Collectiongrenze könnte nach dieser Ergänzung noch weit unter 16 MiB liegen, aber vom eigenen Reader abgewiesen werden. Die Kette braucht einen vollständigen gültigen Grenzfixture und einen echten Read/Write/Read-Lauf. Der Audit hat diesen Fixture nicht gebaut; deshalb kein bestätigter Stateverlustbefund. Zu prüfen sind insbesondere 65.536 Kinder/Properties und die Gesamtknotengrenze 262.144.

### V2 — Ältere Hypothesenrechnung könnte ein neueres Ergebnis überschreiben

Die [Hypothesenrechnung](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/broker/src/coordinator/hypothese_verdrahtung.rs#L72) erfasst Input unter Lock und berechnet außerhalb. Vor Einsetzen werden Intentgeneration und Gültigkeit der verwendeten Evidenz geprüft. Ein gesonderter vollständiger Input-/Ergebnis-Generationsvergleich ist im geprüften Pfad nicht ersichtlich.

**Zu widerlegender Verdacht:** Rechnung A nutzt ältere, weiterhin gültige Evidenz und pausiert. Neue Evidenz stößt B an, B setzt ein neueres Ergebnis ein, anschließend setzt A sein älteres ein. Vorhandene Hooktests konzentrieren sich auf Intentwechsel und Invalidierung. Ein reproduzierter Konkurrenztest mit weiterhin gültigen alten Evidenz-IDs fehlt hier. D5 betrifft unabhängig davon die Zustellung bereits gebauter Snapshots; V2 wird nicht als zweites bestätigtes Orderingproblem gezählt.

## 5. Was die bestehende Architektur bereits gut absichert

Die Prüfung fand mehr als oberflächliche Erfolgsprüfungen: feste Audioqueue-Kapazitäten, Vorbereitung des DSP außerhalb von Audio, explizite Bank-/ACK-Verträge sowie Null-, Golden-, Mutations-, Zustands- und Fehlerpfadtests. Vorwärtskompatibler Stateerhalt über Originalbytes und read-only Behandlung unbekannter Zustände sind sinnvolle Grundlagen. Brokertransport, Coordinator und Store sind grundsätzlich getrennt; Eingangsgrößen und Protokollzustände werden an vielen Stellen begrenzt und validiert.

Root prüfte außerdem den Installationspfad lesend: Pfad-/Hash-Preflight, gesperrtes Journal, verifizierte Sicherungen, Hashkontrolle nach Kopie und geschützter Rückweg sind im Code vorhanden. Daraus folgt keine in dieser Session bestätigte Installation, Signatur oder Admin-Freigabe.

Die wesentlichen Schwächen liegen an Übergängen zwischen diesen Absicherungen: Slotveröffentlichung ohne Besitzschutz, Clientlebensdauer ohne Callbackbesitzer, bestätigter Befehl ohne editorunabhängigen Stateabschluss, geordneter Store ohne geordnete Zustellung und ein Lasttest, dessen Bezeichnung mehr verspricht als sein tatsächlicher Rechenpfad.

Das sind konkrete Qualitätsmuster, an denen sich vermeidbarer KI-Slop prüfen lässt. Kommentarstil, Sprache oder vermutete Autorenschaft allein sind kein Befund. Die wirksamste Verbesserung ist ein nachprüfbarer Vertrag über die gesamte Fehlerkette, nicht mehr Abstraktion oder mehr grüne Einzeltests um jeden Teil davon.

## 6. Frische Prüfungen und verfügbare Belege

| Prüfung in dieser Session | Ergebnis | Aussagegrenze |
|---|---|---|
| Isolierter Checkout und HEAD-Prüfung | Commit 844b9c1, unveränderter Produktstand | Schließt laufende uncommittierte Fable-Arbeit aus |
| Planrechnung mit in Auditordner umgeleitetem Ausgabeziel | 33/45 abgenommen | Dokumentarischer Planstand; keine neue Ticketabnahme |
| gesundheit.py --mit-selbsttest --json | 121/121 Selbsttests bestanden; Gesamtexit 4 | Heuristischer Gesundheitsstatus im eingeschränkten Quellumfang |
| Gesundheitsumfang gegen zusätzliches Quellinventar | 31 Dateien / 13.560 Zeilen außerhalb der genannten Filter | Keine Behauptung vollständiger manueller Zeilenprüfung |
| Isolierter C++-Harness mit MSVC /O2, originaler Vergleichspegel-Header | Build erfolgreich; Laufexit 1, zwei verletzte Erwartungen | Mono/Stereo-Zeitgrenzen am Header und seiner Produkt-Aufruftopologie |
| Drei unabhängige Quellreviews plus Root-Gegenprüfung | D1–D10 integriert, weitere Punkte klassifiziert | Überwiegend statische Beweise |
| Kanon, Cargo-Suite, Clippy, ASan/TSan/UBSan, FL Studio, Installation, Langzeitperformance | Nicht ausgeführt | Keine entsprechende Freigabe |

**Reproduktion D7:** Bei 48 kHz wird Mono bei 199, 200 und 399 ms korrekt als nicht bereit erkannt, bei 400 ms als bereit. Stereo ist bei 199 ms nicht bereit, bei 200 und 399 ms bereits bereit und eingefroren, obwohl beide vor der 400-ms-Grenze liegen. Bei 400 ms ist es erwartungsgemäß bereit. Der Harness ruft den unveränderten Header wie der Prozessor einmal pro Kanal auf. Er ist kein vollständiger Plugin-/Hosttest.

Die Rohbelege und die genaue Reproquelle sind mitgeliefert:

- [Preflight](evidence/preflight-snapshot.json), [frisch berechneter Planstand](evidence/planstand.md), [Quellinventar](evidence/source-inventory.json).
- [Gesundheitsdaten](evidence/gesundheit.json) und [ungekürzte Ausgabe](evidence/gesundheit-raw.txt).
- [Reproquelle](evidence/repro-vergleichspegel.cpp), [damals ausgeführte Build-/Laufdatei](evidence/repro-vergleichspegel.cmd), [Compiler-/Laufausgabe](evidence/repro-vergleichspegel.txt). Die Laufdatei enthält bewusst die tatsächlich verwendeten absoluten Snapshotpfade.
- Unabhängige Arbeitsberichte: [Audio/DSP](packets/audio-dsp.md), [State/IPC](packets/state-ipc.md), [Broker](packets/broker.md). Ihre Paketnummern sind keine zusätzlichen D-Befunde; maßgeblich für Priorität und Evidenzstufe ist dieser integrierte Bericht.
- [Methodenplan](evidence/method-plan.md), [Integration und verworfene Zuspitzungen](evidence/integration.md), [Abschlussprüfung](evidence/verification.md).

## 7. Empfohlene Bearbeitungsreihenfolge für Fable

1. **D1/D2/D3 zuerst als getrennte Aufgaben:** Besitzervertrag des Audiorings, Besitzervertrag laufender IPC-Callbacks und editorunabhängiger Persistenzabschluss. Jeweils zuerst den im Befund beschriebenen Gegenbeweis herstellen, dann klein kohärent korrigieren. D3 und D4 anschließend in derselben Save/Load-Kette gemeinsam gegenprüfen.
2. **D4/D5/D9 als Vertragsarbeit über Modulgrenzen:** Annahme-/Replaystatus, monotone Zustellung und eigenständige Rücknahmewirkung. Bestehende Queues, ACKs und Reentranz bleiben Teil der Prüfung; ein lokaler Fix darf keinen Nachbarvertrag verschlechtern.
3. **D6/D7/D8 gezielt schließen:** vollständiger Projektreset, kanalunabhängige Zeitmessung und Bereichsvalidierung vor Konvertierung. Die Grenzfälle direkt an den produktiven öffentlichen Eingängen testen.
4. **D10 vor Performanceaussagen reparieren; dann O1 und R1 vermessen.** Erst die wirklich aktive Last belegen, danach CPU-/Speicheroptimierung anhand gemessener Anteile priorisieren. Kleine Hostblöcke aus M2 und die dokumentierten Dauertests einbeziehen.
5. **V1/V2 mit begrenzten Gegenbeweisen entscheiden.** Bestätigte Fehler in Tickets überführen; widerlegte Hypothesen samt Beweis schließen. O2/O3 als konkrete Wartbarkeits-/Nachweisaufgaben bearbeiten, ohne laufende Fachimplementierung großflächig umzubauen.

Jeder Abschluss braucht den reproduzierenden Ausgangsfall, den korrigierten Fall und den passenden bestehenden Regressionsbeleg. Ein altes grünes Manifest, ein Build ohne Fehler oder ein niedriges mittleres CPU-Ergebnis allein schließt diese Befunde nicht. Für den inzwischen fortgeschriebenen Branch zuerst prüfen, ob die betreffende Quellkette noch existiert; dieser Audit bleibt absichtlich an seinem unveränderlichen Snapshot festgemacht.
