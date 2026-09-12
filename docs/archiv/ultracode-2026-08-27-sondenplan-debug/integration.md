# Integration

Stand: 27.08.2026, integrierter Arbeitsbaum nach den Paketen 01 bis 09 und
dem kanonischen Abschlusslauf. Geprüft ist der heute gebaute Vor-Umschnitt bis
S13; P6/P7-Funktionalität wurde nicht vorgezogen.

## Integrierter Endstand

| Fläche | Geschlossener Endzustand |
|---|---|
| v3-Vertrag und State | C++, Rust und Python teilen Byte-/UTF-8-/NUL-/Größenregeln. JSON-Zahlen werden vor binary64 sprachneutral begrenzt; vier echte Heartbeat-Gegenfälle sichern den Integer-Rundungsbypass. State verlangt vollständigen Verbrauch, begrenzt Tiefe/Einträge/Bytes, erhält unbekannte lesbare Daten und fällt bei nicht verlustfrei lesbaren Ständen mit Originalbytes auf read-only zurück. `state_hash` ist im Report Pflicht. |
| v2-IPC und Broker | Unbekannte Felder, Breiten-, Null-, Text- und Messwertgrenzen sind Schema-/Serde-paritätisch. Handshake, Sessiontoken, Sequenz, Zweit-Hello, v1/v2-Messung und Same-Nonce-Reconnect sind strikte Zustandsübergänge. Ungültige Messung erhält kein ACK und entwertet alte Evidenz. Stale wird monoton bestimmt. |
| C++-PipeClient | Named Pipe nutzt Identification-SQOS, validiert UTF-8/NUL sowie Welcome/ACK und besitzt abbrechbare Handle-/Generationslebenszyklen. Eine absolute `steady_clock`-Frist gilt für den ganzen Frame einschließlich aller Teil-I/Os; `u64`-Diagnosezähler werden am signed Wirevertrag gesättigt. |
| Audio, Queue und Worker | Audio bleibt bitgleich, ohne neue Locks oder Allokationen. Queue-Generation und tatsächlich angenommene Blocks definieren Mess-/Projektfensterkanten. 50-/250-ms-Kadenzen sind monotone Deadlines ohne Catch-up; wartende Steueroperationen werden vor einem neuen Drain-Zug bedient. Ungültige Samplerate deaktiviert Analyse fail-closed. |
| FeatureEngine | Drops, Seek, Loop, Transport, Rate, Generation und Beweislage trennen offene Zustände. FFT-Support, Projekt-/Continuous-Zeit und Events werden samplegenau über den tatsächlichen Overlap getragen; NaN/Inf, nicht darstellbare Extents und Regel-7-Verstöße werden nicht publiziert. Ablehnung verbraucht fällige Evidenz, statt zu livelocken. |
| Hörmarkierung | Der eigene Abgriff bleibt sauber; nach beobachteter fremder Markierung sperrt der Broker fremde Evidenz, Paare und Aggregate bis zu einem zeitlich beweisbaren Reset. Stale/Disconnect öffnet nicht fail-open; Nonce-Bestand ist begrenzt. |
| Persistenz und Installer | Brokerdateien werden über exklusive Tempdatei, vollständigen Write, `sync_all` und atomaren Replace/No-Replace veröffentlicht. Installer und Rückweg verwenden geschützten Anker, geschützte Backups, haltbare Zustandsjournale, kanonische Ziel-/Quellpolicy, exklusiven Lock und kompensierbare Teilstände. Ein vorhandenes strukturfremdes VST3-Bundle fällt vor Transaktion und Mutation. |
| Beweisweg | Der Runner baut das neue PipeClient-Testziel mit, beschreibt die verschärften Queue-/Featurebehauptungen und lässt weiterhin B6/B7 ehrlich als erst ab P6 geplant stehen. |

## Gekoppelte Lebenszyklen und Cross-Cuts

- **Reset/Generation:** Der Produzent bestätigt die tatsächlich übernommene
  Queue-Generation; erst dann beginnen Projektfenster, Worker-Kadenz und
  Featurezustand neu. Ein verworfener Block wird nicht als gemessene Zeit
  ausgegeben.
- **Frame/Wire:** Feature-Extents bleiben in `int64`/`u32` darstellbar;
  PipeClient und Rust-Broker teilen signed Wirebreiten. Ungültige Frames
  erreichen weder ACK noch Registerzustand.
- **Text/Parser:** Der v3-Schemaweg aktiviert die 15-Signifikanzstellen-Regel
  standardmäßig in allen drei Sprachen. Nur der eigene exakt gerundete
  DTO/JCS-Leser deaktiviert diese Schema-Sicherung explizit; der eingefrorene
  Wert `q = 0.7071067811865476` bleibt dadurch byte- und hashgleich.
- **Start/Stop:** Client und Broker besitzen ihre blockierenden Operationen
  und Threads bis zum Join. Reject-Flush ist auf 100 ms begrenzt, normale
  Verbindungsenden flushen nicht blockierend.
- **Speichern/Rückweg:** Writer, Reader, Journal, Backup und Restore prüfen
  denselben Vor-/Nachzustand. Öffentliche Diagnosejournale sind keine
  privilegierte Restoreautorität.

## Adversariales Schlussurteil

Die beiden letzten High-Gegenpfade aus Paket 08 sind im integrierten Stand
geschlossen: das nicht rückwegfähige Fremdbundle und der sprachübergreifende
binary64-Integerbypass. Paket 09 fand im finalen Feature-/Zeitpfad keinen
kritischen oder hohen Restfehler. Der erste kanonische Abschlusslauf deckte
zusätzlich eine bytegenaue FlatBuffers-Codegen-Abweichung auf: Ein Formatter
hatte nur die Zeilenenden der generierten Rust-Datei verändert. Neugenerierung
mit dem gepinnten `flatc 25.12.19` stellte Drift 0 her; erst der danach erneut
gestartete vollständige Lauf gilt als Abschlussbeweis.

## Verbleibende Risiken

1. **Mittel, Vertragsentscheidung (NAK-83):** Der FFT-Frameanker kann vor dem
   Block liegen, aus dem `recording`, Cycle und Presentation-Latenzen stammen.
   Ob diese Felder Frameanfang oder Publikationssnapshot bedeuten, muss vor dem
   nächsten Consumer entschieden werden.
2. **Begrenzte Verfügbarkeit:** Der Broker deckelt aktive Verbindungen auf 64
   und bekannte Sensor-IDs auf 128, besitzt aber noch keine Bootstrap-/Idle-
   Deadline. Ein Same-User-Slowloris kann die begrenzten Slots belegen.
3. **Niedrige Ränder:** Der lokale `uint64`-Audiostrom hat keinen expliziten
   Wrap-Golden; exakt gültige `int64`-/NAK-29-Obergrenzen sind nicht je einzeln
   regressionsfest. Der heutige Produktionspfad erreicht den Wrap praktisch
   nicht.
4. **Zeit-/Schedulergrenzen:** Worker-Fairness und Pipefristen sind
   deterministisch getestet, aber keine harte Betriebssystem-SLA. Ein Reset
   ohne nachfolgenden Audioblock kann das alte Projektfenster sichtbar lassen;
   der jüngste angenommene Block kann bis zur Quarantäneversiegelung noch im
   Produzentenfenster stehen.
5. **Externe Beweise:** Kein echter FL-Studio-Scan/Load, keine erhöhte echte
   Installation, kein Codesigning, kein Stromausfall während Flush, kein
   TSAN/UBSan/Application-Verifier-/Win32-Fault-Injection-Lauf und kein
   ARM-Beleg. B6/B7 bleiben planmäßig ungebaut, weil der aktive DSP und
   Apply/Revert erst P6 sind.
