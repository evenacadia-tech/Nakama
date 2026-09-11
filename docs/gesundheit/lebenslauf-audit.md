# Lebenslauf-Audit (K1 Rückstau und Ordnung, K2 Lebenslauf und Besitz)

**Kanonische Datei, überschrieben; Git-Historie ist das Archiv.** Erster
Lauf 12.09.2026 als Prüfsession in `nakama-d8` nach
`tools/pruefung/auftrag-lebenslauf-audit.md`, gelesen am Stand `493e96d0`
in der Arbeitskopie. Population aus der Abdeckungskarte (K1: 30 Objekte, K2:
43 Objekte); zwei Leser (Broker, Plugin) haben die 28 Zellen geklärt, die
dort „Join ohne Frist" oder „unklar" trugen; zwei Skeptiker haben jede
offene Zelle zu widerlegen versucht und zusätzlich gesucht, ob eine
Verhaltensmatrix eines abgenommenen Tickets die Zelle als Zusage trägt.

## Kopf: Urteil, Befunde, Zähler

**Urteil: kein Defekt.** Keine Zelle bricht eine Matrixzeile, einen
Gate-Text oder eine Invariante. Die Broker-Seite hält Prüfliste A vollständig
(jede Queue mit deklarierter Politik und Abfluss ohne Reconnect) und
Prüfliste B bis auf einen bekannten, offen zugesagten Punkt (H-06). Die
Plugin-Seite hat keinen fristlosen Join ohne Begründung, aber drei
Self-Join-Pfade und zwei Stoppfenster, die weder bewacht noch zugesagt noch
gemessen sind. Das ist die Klasse, die Prüfliste B seit dem 30.08.2026
nennt und die kein Detektor trägt.

**Befunde** (alle Härtungen; K-Kennung nach `KONZEPT.md` §3; seit
12.09.2026 als NAK-268 im Register, Commit `40e09edc`, B-6 mit Querverweis
NAK-192):

| Nr | K | Befund | Beleg | Klasse |
|---|---|---|---|---|
| B-1 | K2 | Der Join des Tickthreads in `BrokerSupervisor::drop` ist fristlos; der Tickkörper wartet in `flush_session` auf den Store ohne Frist. In der Praxis begrenzt (Writer leert stets, SQLite `busy_timeout`, Sender-Drop ⇒ `Beendet`, Stoppreihenfolge Supervisor vor Store). | `broker/src/lebenslauf.rs`, `broker/src/coordinator/liveness.rs`, `broker/src/store/handle.rs`; Zusage **H-06** in `docs/beweise/NAK-121.md` („jeder Join hat eine Frist", dort ausdrücklich halb umgesetzt und offen) | [Härtung · Lebenslauf-Audit] unter NAK-268 mit Querverweis NAK-121/NAK-149; der Join-Frist-Teil von H-06 hatte bisher keine eigene Zeile (Dirigent, 12.09.2026) |
| B-2 | K2 | Kein Test misst, dass `StoreWriter::stoppen`/Drop und der Tickthread-Stopp terminieren, auch nicht indirekt: der Probeprozess in `broker_idle.rs` (70-s-Frist) startet nur den v3-Listener, `store_crash_matrix.rs` wendet seine 30-s-Frist nur auf den C++-Client an. | `broker/tests/broker_idle.rs`, `broker/src/bin/eqcop-broker-v3probe.rs`, `broker/tests/store_crash_matrix.rs` | [Härtung · Test] |
| B-3 | K1 | `Ausgang::entnehmen` liest den Inhalt vor dem Schließflag. Strukturell dicht, weil `schliessen` unter demselben Mutex leert und `einreihen_eintrag` das Flag zuerst prüft; die Matrixzeile A-P0-09 („Schließflag VOR dem Inhalt") und ihr Test gelten aber nur dem Eingang. | `broker/src/transport/server_v3/queues.rs`; Matrix `docs/beweise/SONDE-010.md` A-P0-09; Test `tests_rueckstau.rs` (`geschlossener_eingang_liefert_nichts_mehr`) | [Härtung · Test]: Spiegeltest für den Ausgang |
| B-4 | K2 | `control_schliesst` läuft beim V3-Stopp fristlos auf dem gejointen Thread. Begrenzt (nur Standlock, Mapoperationen, kein Flush), aber ohne blockierende Probe unter Stopp, wie sie `tests_kopplung.rs` für `telemetrie_getrennt`, `telemetrie_gekoppelt` und `control_verbunden` hat. | `broker/src/coordinator/senke.rs`, `broker/src/coordinator/link.rs`, `broker/src/transport/server_v3/griff.rs`, `broker/src/transport/server_v3/tests_kopplung.rs` | [Härtung · Test] |
| B-5 | K2 | ControlClient und TelemetryClient: Hook- und Probe-Setter ohne Stoppfenster-Wache (`laeuft` fällt vor dem Join, eine Registrierung im Fenster geht durch), `start()` überschreibt den Thread ohne `joinable`-Prüfung. Konvention nur als Kommentar und durch die drei Aufrufe im Konstruktor vor `start()`. Prüfliste B: „Registrierung nach begonnenem Stop ist ein Test, kein Zufall." | `eq-copilot/plugin/core/ipc/ControlClient.cpp`, `eq-copilot/plugin/core/ipc/TelemetryClient.cpp`, `eq-copilot/plugin/src/PluginProcessor.cpp` (Aufrufe im Ctor) | [Härtung/Struktur] |
| B-6 | K2 | BrokerLifecycle: Hooks laufen auf dem Lebenslaufthread, `stop()` joint ohne `threadId`-Vergleich (Self-Join aus einem Hook möglich), keine Sperre zwischen `start()` und `stop()`; Header ohne Aufrufkonvention; ausgeschlossen allein durch die Aufrufstellen in Konstruktor und Destruktor. NAK-192 nennt nur den fristlosen Join. | `eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp`, `BrokerLifecycle.h`; Register NAK-192 | [Härtung/Struktur]; Querverweis NAK-192 |
| B-7 | K2 | PipeClient: Provider-Callbacks auf dem Pipethread, `stop()` joint unbedingt unter `lebenslaufMutex`; ein Provider, der `stop()` ruft, wäre Self-Join, ohne Zusage und Test (das Stoppfenster von `start()` ist gehärtet). TelemetryClient: Self-Join bewacht und für `helloProvider` zugesagt (B-TC-07), für `beiFrame` weder zugesagt noch gemessen. | `eq-copilot/plugin/src/PipeClient.cpp`, `PipeClient.h`; `eq-copilot/plugin/core/ipc/TelemetryClient.cpp`; Matrix `docs/beweise/SONDE-010.md` B-TC-07 | [Härtung · Test] |
| B-8 | K2 | `InterventionsRing::zuruecksetzen` hat keinen Produktaufrufer; `resync` und `StampedAudioQueue::vorbereiten` halten ihre Thread-Konvention nur durch Aufrufort und den Callbackvertrag in `ControlClient.h`, ohne Test und ohne Dokument, das sie als bewusst wachlos erklärt. | `eq-copilot/plugin/core/ipc/InterventionsRing.h`, `eq-copilot/plugin/core/StampedAudioQueue.h`, `eq-copilot/plugin/src/prozessor/Ipc.cpp` | [Härtung · Test]; `zuruecksetzen` als Kandidat für die Zwecktreue |

**Zähler.** K1: 30 Objekte, 28 mit Test, 1 Befund (A-1 in der Karte, NAK-267).
K2: 43 Objekte; 28 Zellen geprüft (Rust 19 Objekte, C++ 9); fristlose Joins
ohne Begründung nach Skeptikerpass 0 (R1 ist als H-06 offen zugesagt);
Self-Join möglich ohne Test 3 (B-6, B-7 zweimal); Stoppfenster ohne Wache 2
(B-5); Zellen ohne Test 11 (Plugin) plus 4 (Broker). Widerlegt: 2 von 5
Rust-Zellen, 1 von 6 C++-Zellen ganz, 4 teilweise.

---

## Widerlegte Zellen (nicht erneut finden)

| Zelle | Beleg |
|---|---|
| v2-Verbindungsthread: Senkenaufruf im Drop fristlos | Matrix H-05 in `docs/beweise/NAK-121.md`: „Keine Frist; die Grenze ist, dass der Destruktor unter keinem Zustand des Coordinators panisch endet"; `getrennt` ist reiner Standlock (`broker/src/coordinator/intervention.rs`); Test `broker/tests/coordinator_model.rs` |
| Schleuse: Callback ruft `schliessen` aus seinem Zug | Konvention im Kopf von `eq-copilot/plugin/core/ipc/controlclient/Schleuse.h` („Der Besitzer SCHLIESST sie in seinem Destruktor, NACH `stop()` seiner Clients"), einziger Aufrufer im Destruktor von `PluginProcessor.cpp`; gemessen als M-08/M-09 in `docs/beweise/NAK-246.md` |
| ControlClient Self-Join | Wache in `ControlClient.cpp`, Vertrag in `ControlClient.h`, Zusage B-CC-11 in `docs/beweise/SONDE-010.md`, Test `IpcTestMain.cpp` |
| PipeClient Stoppfenster von `start()` | `PipeClient.cpp` joint einen alten Thread vor der Neuzuweisung |
| `writer_lauf vorgemerkt`, Store-Antwortkanäle, v2-Flushthread, StoreWriter-Close-Flag, `session_push_setzen`, `thread_abbrechen_und_join` | siehe Abdeckungskarte K1/K2 (Skeptikerpass vom 12.09.2026) |

## Matrix Broker (K2, geprüfte Zellen)

| Objekt | Join | Close-Flag vor Inhalt | Callback-Besitz | Test |
|---|---|---|---|---|
| Tickthread (`lebenslauf.rs`) | fristlos; Stop-Flag vor Join; Körper über Store ohne Frist (B-1, H-06) | n. a. | Closure besitzt `Arc<Coordinator>`; Stop nur vom Hauptthread | keiner (B-2) |
| `broker_geordnet_stoppen` | erbt Joins; Reihenfolge Supervisor, v2, v3, Store | n. a. | kein Fremdcode | `broker_idle.rs` (Quelltext-Reihenfolge; Prozessende ohne Supervisor) |
| StoreWriter `stoppen`/Drop | fristlos, begründet (Shutdown im FIFO vor Join; Schleife bricht bei Shutdown und Disconnected) | absichtlich kein Flag (Kommentar) | keine Callbacks | keiner (B-2) |
| `ServerGriff::stoppen` (v2) | fristlos, begründet (Stop-Flag, wiederholtes `CancelSynchronousIo` bis `is_finished`) | n. a. | — | `server.rs` (≤ 2 s bei stiller Pipe) |
| v2-Verbindungsthread | Senkenaufruf fristlos, per H-05 so zugesagt | n. a. | Coordinator-Senke, `catch_unwind` | `server.rs`, `coordinator_model.rs` |
| `V3Griff::stoppen`/Drop | fristlos, begründet (Stop-Flag, Acceptor-Event, `alle_io_abbrechen`); `control_schliesst` begrenzt, ungetestet unter Stopp (B-4) | n. a. | Senke gehört dem Coordinator | `tests_lebenszyklus.rs`, `security_vectors.rs` |
| `fertige_ernten`, `WorkerPlatz` Drop, `BootstrapFrist` Drop, `ListenerInstanz` Drop | Join nur auf `is_finished` oder kein Join; begründet | n. a. | kein Fremdcode | `tests_lebenszyklus.rs`, `transport_fuzz.rs`, `security_vectors.rs`, `tests_fristen.rs` |
| Wachhund, Acceptor | fristlos, begründet (Flag am Kopf, Takt 100 ms; `WaitForMultipleObjects` auf Stop-Event) | n. a. | kein Fremdcode | `security_vectors.rs` (H-02) |
| Verbindungsthread | gejointet in `griff.rs` | n. a. | Senke hineingeklont | `tests_lebenszyklus.rs` |
| Schreiberthread | Frist ja (`join_mit_frist`, wiederholtes `io_abbrechen`) | n. a. | kein Fremdcode | `security_vectors.rs` |
| `Eingang::schliessen` | n. a. | ja (Flag vor Inhalt, Kommentar) | Verbraucher-Threads | `tests_rueckstau.rs` |
| `Ausgang::schliessen`/`entnehmen` | n. a. | Inhalt vor Flag, strukturell dicht (B-3) | Writer-Thread | keiner (B-3) |

## Matrix Plugin (K2, geprüfte Zellen)

| Objekt | Join | Close-Flag vor Inhalt | Self-Join | Stoppfenster | Test |
|---|---|---|---|---|---|
| ControlClient Hooks, Probe-Setter | Frist ja (`stop()` mit Detach) | n. a. | bewacht, zugesagt (B-CC-11), gemessen | Setter unbewacht (B-5) | `IpcTestMain.cpp` (Self-Join); Setter keiner |
| Schleuse | kein Join; `schliessen` wartet auf `aktiv == 0`, begründet | ja (zweimal gelesen) | ausgeschlossen durch Besitzer, dokumentiert und gemessen (M-08/M-09) | Eintritt nach Schließen abgewiesen und gezählt | `IpcTestMain.cpp`, `Sonde012ProjectReloadTest.cpp` |
| BrokerLifecycle | fristlos, begründet (`laeuft` und `notify` vor Join, Wartezeit gedeckelt); NAK-192 offen | n. a. | möglich, unbewacht, undokumentiert (B-6) | keine Sperre start/stop (B-6) | `LebenslaufTestMain.cpp` (nur Stoppfrist) |
| PipeClient | fristlos, begründet (`laeuft`, Generation, `notify`, `aktiveIoAbbrechen` vor Join; I/O mit Deadline) | Abbruch vor Verbrauch | möglich aus Provider, ohne Zusage und Test (B-7) | `start()` joint Vorgänger (gehärtet) | `PipeClientLifecycleTestMain.cpp` (Fristen) |
| IpcVerbindung | kein Join | ja (`abbruch` zuerst) | keine Callbacks | Generation vor `oeffnen` | `IpcTestMain.cpp` |
| Processor Worker, Destruktorfolge | fristlos, begründet (`workerLaeuft`, `notify` vor Join; Client-Stops mit Fristen) | ja (Schleuse nach den Stops) | Destruktor nie aus Callback | Callback nach Schließen abgewiesen | `Sonde012ProjectReloadTest.cpp` |
| InterventionsRing `resync`/`zuruecksetzen`, StampedAudioQueue `vorbereiten` | kein Join | Leere vor Inhalt; Wunschbit vor Inhalt, begründet | keine Callbacks | Konvention ohne Wache (B-8) | `Sonde013InterventionRingTest.cpp`, `QueueStressTestMain.cpp` (Wirkung, nicht Thread) |
| TelemetryClient | Frist ja (`kStopFristMs`, Detach) | ja (vor `beiFrame`, zweimal) | bewacht; `helloProvider` zugesagt (B-TC-07), `beiFrame` nicht (B-7) | `start()` ohne `joinable`-Prüfung (B-5) | `IpcTestMain.cpp` |

## Scope-Beweis

Leser Broker: 20 Dateien (`lebenslauf.rs`, `store/writer.rs`, `store/handle.rs`,
`server.rs`, `transport/server_v3/` mit `griff`, `listener`, `verbindung`,
`queues`, `win_handles`, `senke`, `mod`, `tests_fristen`, `tests_lebenszyklus`,
`tests_rueckstau`; `coordinator/intervention.rs`, `coordinator/liveness.rs`,
`main.rs`; Tests `broker_idle.rs`, `security_vectors.rs`, `store_crash_matrix.rs`,
`transport_fuzz.rs`). Leser Plugin: 21 Dateien (Client-, Lebenslauf-, Pipe-,
Verbindungs-, Ring- und Queue-Quellen samt Headern; Tests `IpcTestMain`,
`LebenslaufTestMain`, `PipeClientLifecycleTestMain`, `QueueStressTestMain`,
`Sonde013InterventionRingTest`, `Sonde012ProjectReloadTest`). Skeptiker: 32
und 25 Dateien, darunter `docs/beweise/NAK-121.md`, `SONDE-010.md`,
`NAK-246.md`, `NAK-180.md`, `NAK-123.md`, `NAK-225.md`, `G2.md`, `G3.md` und
`docs/offene-punkte.md` als Zusagenquellen. Fehlende Pfade: keine. Alle
Läufe auf `493e96d0` in der Arbeitskopie.

## Änderungsprotokoll

| Datum | Änderung | Anlass |
|---|---|---|
| 12.09.2026 | Erster Lauf; Befunde B-1 bis B-8, 3 Zellen ganz und 4 teilweise widerlegt | `KONZEPT.md` §10 Schritt 3 |
