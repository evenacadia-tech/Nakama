Der Zielstand c8fc3c2 enthält mehrere produktive Gate-Verletzungen, insbesondere einen vollständig fehlenden persistenten Command-/ACK-Weg, inkorrekte Reconnect-Projektionen und unvollständige beziehungsweise nicht idempotente Produktverdrahtung des Autostarts. Geprüft wurden Diff und Zielquellen statisch; Laufzeittests wurden gemäß rein lesendem Auftrag nicht erneut ausgeführt, und das inzwischen extern auf 494c0f4 bewegte HEAD ist in allen beauftragten Prüfpfaden bytegleich zu c8fc3c2.

Full review comments:

- [P1] Verarbeite persistente P0-Befehle im Produkt-Coordinator — C:/Users/phili/Projekte/Nakama/broker/src/coordinator.rs:1736-1736
  **DEFEKT:** Bei einem gültigen `preview_begin` oder anderen persistenzpflichtigen P0-Befehl fällt der produktive Coordinator in diesen Defaultarm, schreibt nichts in den Store und erzeugt kein `command_ack`. `git grep -n -E 'preview_begin|command_ack' c8fc3c2 -- broker/src` zeigt, dass der einzige Handler im ausdrücklich nie vom Broker gestarteten Crash-Testworker liegt. Damit bleiben C++-In-Flight-Aufträge offen und S-02, O-01, C-09 sowie K-01–K-03 sind verletzt.

- [P1] Trenne Domain-Events von der Sessionprojektion — C:/Users/phili/Projekte/Nakama/broker/src/store.rs:960-965
  **DEFEKT:** Sobald nach einem Session-Snapshot ein `evidence`-, `finding`- oder anderes Domain-Event committed wird, überschreibt dieser bedingungslose UPSERT `sessions.state_jcs` mit dessen Domain-Payload. Beim nächsten Reconnect ist das kein `session_snapshot`, die Schema-Prüfung sperrt das Routing und die committete Sessionwirkung ist nicht rekonstruierbar; der vorhandene O-04-Test prüft nach dem Evidence-Append keinen Reconnect. Das verletzt O-02, O-03, O-04 und K-07.

- [P1] Bereinige Projektionen an der Brokerlaufgrenze — C:/Users/phili/Projekte/Nakama/broker/src/coordinator.rs:631-633
  **DEFEKT:** Wenn sich nach einem Brokerneustart ein Client vor seinem ersten `state_report` abonniert, wird der letzte Snapshot des alten Laufs unverändert gesendet. Dieser enthält den alten `broker_epoch`, die alte Führung und alte Mitglieder, obwohl C-04/E-06 nach Neustart freie Führung und C-05 einen neuen `broker_epoch` verlangen. Dafür ist kein NAK-120-State-Ingress nötig; die laufgebundenen Felder müssen beim Resubscribe aus dem aktuellen Lauf stammen.

- [P1] Behandle Projektionslesefehler fail-closed — C:/Users/phili/Projekte/Nakama/broker/src/coordinator.rs:628-629
  **DEFEKT:** Bei einem SQLite-Open-/Queryfehler verwirft `.ok().flatten()` den Fehler wie einen nicht vorhandenen Datensatz und sendet den flüchtigen Fallback. Besonders direkt nach einem Neustart kann dadurch ein leerer Snapshot eine bereits committete Wirkung ersetzen, statt die Subscription sichtbar fail-closed zu halten. Das bricht S-03 sowie die verlustfreie Rekonstruktion aus O-02/O-03.

- [P1] Serialisiere Snapshot-Commits pro Session — C:/Users/phili/Projekte/Nakama/broker/src/coordinator.rs:1421-1425
  **DEFEKT:** Zwei Linkthreads können dieselbe Session gleichzeitig flushen: Thread A erfasst einen alten Payload und pausiert nach Freigabe des Locks, Thread B committed den neueren Stand, danach erhält A die spätere Sequence und überschreibt Projektion und Outbox wieder mit dem alten Stand. Damit ist bei konkurrierenden Heartbeats/Reports nicht mehr der aktuelle Snapshot der letzte, entgegen O-02/O-03.

- [P1] Prüfe Pushziele unmittelbar vor dem Schreiben erneut — C:/Users/phili/Projekte/Nakama/broker/src/coordinator.rs:641-644
  **DEFEKT:** Zwischen der Zielermittlung unter dem Coordinator-Lock und diesem späteren Write liegen Storezugriff und weitere Threadwechsel, ohne erneute Prüfung von Subscription, Linkbesitz oder Konfliktriegel. Eine parallele Eviction, Trennung oder Aliasquarantäne kann daher semantisch bereits aufgeräumt haben, während der alte Push trotzdem noch gestartet wird; derselbe Ablauf existiert im normalen `flush_session`. Das verletzt die 28-B-Reihenfolge C-06 sowie C-07/E-05.

- [P2] Validiere Coordinator-JSON vollständig gegen das v3-Schema — C:/Users/phili/Projekte/Nakama/broker/src/coordinator.rs:1681-1681
  **DEFEKT:** `p0_json` verwendet nur `serde_json::from_slice`, und `subscribe_json` macht dieselbe manuelle Teilprüfung. Dadurch wird etwa ein Heartbeat ohne schemafeste Pflichtfelder bestätigt und ein `subscribe_session` mit zusätzlichem Cursor-/UUID-Feld registriert, obwohl der Vertrag `additionalProperties: false` vorgibt. Das umgeht die C-09-Domänenvalidierung und die cursorfreie O-02-Subscription.

- [P1] Akzeptiere alle schemafesten Interventionsarten — C:/Users/phili/Projekte/Nakama/broker/src/coordinator.rs:1713-1714
  **DEFEKT:** Ein gültiges `audible_intervention_begin` mit `art` gleich `preview`, `focus_burst` oder `experiment` wird hier verworfen; nur `hoermarkierung` erreicht den gemeinsamen Interventionszustand. Während einer solchen Intervention bleibt starke Evidenz daher fälschlich erlaubt, obwohl C-08 alle v3-Begin/End-Arten in denselben Zustand bindet.

- [P1] Markiere ein unbekanntes Intervention-Ende als unknown — C:/Users/phili/Projekte/Nakama/broker/src/coordinator.rs:1559-1560
  **DEFEKT:** Trifft nach Reconnect oder verlorenem Begin zuerst ein schemafestes End mit unbekannter ID ein, akzeptiert die Sequenzprüfung den ersten Zähler und dieser Zweig kehrt zurück, ohne `intervention_state_unknown` zu setzen. Der Broker erscheint danach sauber und erlaubt Evidenz, obwohl C-08 ausdrücklich verlangt, dass ein verlorenes Begin oder End niemals eine scheinbar saubere Baseline erzeugt.

- [P2] Verifiziere P2 vor dem Evidenzdispatch — C:/Users/phili/Projekte/Nakama/broker/src/coordinator.rs:1794-1800
  **DEFEKT:** Der P2-Handler ignoriert den Payload vollständig und zählt jeden korrekt gerahmten Byteblock eines gekoppelten Links als Evidenzdispatch. Der vorhandene FlatBuffers-Verifier `telemetrie::pruefe` wird im Produktpfad nicht aufgerufen, sodass auch ungültige FeatureBatch-Bytes fachlich wirksam werden. Das verletzt die C-09-Grenze zwischen validiertem Frame und Domänenmutation.

- [P1] Verbinde den v2-Hörmarker mit dem Coordinator — C:/Users/phili/Projekte/Nakama/broker/src/lib.rs:891-894
  **DEFEKT:** Der Produktstart gibt dem v2-Server weiterhin ein separates `Register` und dem v3-Server einen unabhängigen Coordinator; `git grep -n 'hoermarkierung_v2' c8fc3c2 -- broker/src` findet außerhalb der Definition nur Unit-Tests. Daher sperrt eine aktive v2-Hörmarkierung den v3-Evidenzdispatch nicht und umgekehrt, obwohl C-08 genau einen gemeinsamen Zustand zusagt.

- [P1] Sende Produkt-Heartbeats auf der v3-Verbindung — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/src/PluginProcessor.cpp:135-135
  **DEFEKT:** Nach `controlV3.start()` sendet der Produktcode weder `heartbeat` noch `state_report`; `git grep -n -E 'heartbeat|state_report' c8fc3c2 -- eq-copilot/plugin/core/ipc/ControlClient.cpp eq-copilot/plugin/src/PluginProcessor.cpp` liefert keinen Sender. Eine gültig gebundene Main-Instanz wird deshalb nach 2500 ms stale und nach weiteren 10 s evicted und reconnectet dauerhaft, entgegen E-01/E-02.

- [P1] Stelle vor dem Connect eine gültige Projektbindung bereit — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/src/PluginProcessor.cpp:746-746
  **DEFEKT:** Frische und aus Schema 1 migrierte Zustände lassen `projectBindingId` absichtlich leer, und die einzige Rolleninitialisierung `setzeBindung` befüllt es nicht. Der ControlClient lehnt deshalb die Adresse vor jedem Pipeversuch ab und setzt `brokerPipeFehlt` nicht; selbst ein explizit gewähltes Main mit offenem Editor erreicht den Autostart nie. Das verletzt A-01/A-02; die Behebung braucht eine autoritative Bindungsquelle, nicht eine erfundene Identität.

- [P1] Verdrahte Probeeq als reinen v3-Connector — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/src/PluginProcessor.cpp:135-135
  **DEFEKT:** Nur `EqCopilotProcessor` startet einen ControlClient; `git grep -n -E 'ControlClient|TelemetryClient' c8fc3c2 -- eq-copilot/plugin/sonde` findet in `SondeProcessor` keinen Connector. Probeeq kann sich damit überhaupt nicht mit einem vorhandenen Broker verbinden. Der Lebenslauftest beweist lediglich, dass Probes nicht starten dürfen, aber nicht die in A-04 zugesagte reine Connectorrolle.

- [P1] Prüfe nach Mutexgewinn die Pipe statt nur den Cache — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:520-522
  **DEFEKT:** Ein Mutex-Verlierer kann den Mutex übernehmen, nachdem der Sieger den Broker gestartet und seinen eigenen Connect abgeschlossen hat, während der ControlClient des Verlierers noch den alten getrennten Zustand cached. Diese Prüfung bleibt dann falsch und der Verlierer startet einen zweiten Prozess; der vorhandene Test wartet vor der Freigabe bereits auf beide verbundenen Clients und deckt das Rennen daher nicht. Das bricht A-04 und `BROKER_PRO_USER_MAX = 1`.

- [P1] Baue das releasefähige Broker-Artefakt im Kanon — C:/Users/phili/Projekte/Nakama/eq-copilot/install/nakama-installer-v1.json:52-52
  **DEFEKT:** In einem sauberen Checkout fehlt das ignorierte `broker/target/release/eqcop-broker.exe`, während `tools/beweise.ps1 -Bauen` ausschließlich `eqcop-broker-v3probe` baut. `git check-ignore -v broker/target/release/eqcop-broker.exe` und die einzige `--bin`-Zeile im Runner reproduzieren die Lücke; A17 bricht sauber wegen des fehlenden Artefakts ab und der hier festgeschriebene Hash stammt nur aus einem lokalen Dirty-Bau. Damit ist das für A-03 benötigte Autostart-Artefakt nicht reproduzierbar.

- [P2] Warte trotz anderer Befehle bis zum Commit-Auslöser — C:/Users/phili/Projekte/Nakama/broker/src/store.rs:781-783
  **DEFEKT:** Trifft während eines offenen Append-Batches vor Ablauf der 50 ms etwa ein Checkpoint- oder Guard-Befehl ein, wird er vorgemerkt und die Schleife sofort verlassen; `append_gruppe` committed dadurch unterhalb von Fenster und Cap. Eine Append-Anforderung plus paralleler Checkpoint reproduziert den vorzeitigen Commit und verletzt den einzigen in S-05 erlaubten Auslöser „Fenster oder Batchcap“.

- [P2] Begrenze jeden Group-Commit auf 64 Events — C:/Users/phili/Projekte/Nakama/broker/src/store.rs:778-778
  **DEFEKT:** Die Schleife prüft nur den bisherigen Zähler und fügt anschließend den gesamten nächsten Job hinzu. Ein 63-Event-Job gefolgt von einem 2-Event-Job committed daher 65 Events; auch ein einzelner größerer Job wird ungeteilt angenommen. Das überschreitet das in S-05 zugesagte `COMMIT_BATCH_MAX = 64`.

- [P1] Prüfe das Lifecycle-Gate direkt vor dem Spawn erneut — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:551-552
  **DEFEKT:** Schließt der User den Editor oder ändert die Rolle, während die Hash-/Signaturprüfung läuft, bleibt das vor der Prüfung gelesene positive Gate wirksam und der Prozess wird anschließend trotzdem gestartet. Da der ControlClient unabhängig vom Editor weiterläuft, kann dieser Broker danach aktiv bleiben. A-02 verlangt jedoch, dass nur ein zum Spawnzeitpunkt klassifiziertes Main mit offenem Editor startet.