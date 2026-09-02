# G3-SOAK — Werkzeugticket: das 60-Minuten-Dauerlaufbein A24

<!-- NAKAMA-URTEIL: PHASE 1 (Verhaltensmatrix) 2026-09-02 — kein Bau, keine Messung -->

| Merkmal | Wert |
|---|---|
| Ticket | G3-SOAK (Werkzeugticket zum Phasengate **G3**) |
| Basis-Commit | `03e1e17` (master, sauberer Worktree) |
| Phase | **1 von 2** — Spezifikation vor Code (Dirigent-Skill §3.2). Diese Datei enthält keinen Code und keine Messung. |
| Gate-Text (wörtlich, `docs/bauaufteilung-sonden.md:385`) | „**Gate:** `/rust-review` + Codex + 60-min-Soak. Falsifikation: Gate 7" · Prüfstufe T3 |
| Auftrag | genau ein neues Kanon-Bein **A24** bauen. In Phase 2 **anzulegen** (heute Zielpfad, noch nicht angelegt): der Prüfer `tools/eq-copilot/pruefe_session_soak.py` und das Konsolenprogramm `eq-copilot/plugin/tests/SessionSoakMain.cpp` (CMake-Ziel `EqCopSessionSoak`). Dazu Erweiterung des bestehenden Probe-Brokers `broker/src/bin/eqcop-broker-sonde012-probe.rs` und die Registrierung in `tools/beweise.ps1`. Sonst nichts. |
| Bindende Quellen | Entwurf §49.3 Budget `docs/FL-Nakama-Sonden-Design-Entwurf.md:3586`; P3-Exit-Gate `:4262-4264`; Testfamilie `session_soak` `:4523`; Prüflistenabschnitt A `tools/dirigent/pruefliste.md` |
| Vorbilder (Bau) | A22 `tools/eq-copilot/pruefe_ipc_last.py` + `eq-copilot/plugin/tests/IpcLastMain.cpp`; A23 `tools/eq-copilot/pruefe_sonde012_sources_latency.py` + `eq-copilot/plugin/tests/Sonde012SourcesLatencyTest.cpp`; B4 `tests/QueueStressTestMain.cpp`; B12 `tests/Sonde012LoudnessSourceTest.cpp` |
| Ticketgrenze | kein Wire-, State- oder Testvertrag; keine Schemas, keine Fixtures, keine Broker-Produktmodule (alles unter `broker/src/` ausser dem Probe-Binary), kein Produktcode im Plugin. |

## 1. Wofür diese Datei da ist

Das Phasengate G3 verlangt einen 60-Minuten-Dauerlauf. Den gibt es als Werkzeug
nicht. Er ist auch nicht dasselbe wie das vorhandene Lastbein A22: A22 misst
**acht Sekunden Flut** und beweist, dass P0 unter Rückstau nicht verhungert.
Ein Soak beweist etwas anderes — dass über eine lange Zeit **nichts wegdriftet**:
keine Mitgliedschaft, keine Adresse, kein Speicher, keine Antwortzeit; und dass
ein Brokerneustart mittendrin den Zustand vollständig und richtig
wiederherstellt, statt eine alte Wahrheit weiterzuspielen.

Diese Datei friert **vor dem Bau** ein, was das Bein zusagt und woran jede
Zusage fällt. Sie ist weder Bau- noch Prüferurteil. Ein Testname in der Spalte
„Test" trägt **NEU**, solange er nicht existiert; er ist damit Teil der
Bauaufgabe von Phase 2 und keine Behauptung, der Test liefe bereits.

**Leseregel bei Konflikt:** über Entwurf und Manifest stehen der aktuelle
Vertrag und der Code (`CLAUDE.md`, Abschnitt „Plan"). Jede Frist und jede
Konstante in dieser Datei ist deshalb aus der Quelle abgelesen und mit
`Datei:Zeile` belegt, nicht aus dem Entwurf abgeschrieben.

## 2. Was A24 misst — und was es ausdrücklich nicht misst

### 2.1 Der gemessene Aufbau

Ein Prozess `EqCopSessionSoak` fährt gegen **einen echten Rust-Coordinator** auf
einem ausschliesslichen Probe-Pipenamen:

- **1 Main**: `ControlClient` + `TelemetryClient`, `subscribe_session` wie in
  `eq-copilot/plugin/tests/Sonde012SourcesLatencyTest.cpp:131-140`; Endpunkt ist
  das echte `eqcop::SourcesModel`.
- **N Sonden** (Standard 16): je ein `ControlClient` + `TelemetryClient`-Paar wie
  in `eq-copilot/plugin/tests/IpcLastMain.cpp:96-146`, gekoppelt über `link_id`
  und `challenge`.
- **N Audiotreiber**: je eine echte `nakama::sonde::SondeProcessor`-Instanz in
  einem eigenen Thread, die in Echtzeitkadenz Ganzblöcke durch die vorallokierte
  `StampedAudioQueue` zum Analyseworker schiebt (Muster B12
  `eq-copilot/plugin/tests/Sonde012LoudnessSourceTest.cpp:71-127`).
- **Ein Viertel der Sonden liest langsam** (`--langsam`, Standard 0.25): ihre
  Lese-Callbacks halten künstlich an, **nachdem** sie den Empfangszeitpunkt
  genommen haben (siehe §5, Zeile S05).
- **Der Broker wird zu festen Zeitpunkten getötet und neu gestartet**
  (`--neustarts`, Kurzform 1, Gate-Lauf 3): Prozess beenden, auf demselben
  Probe-Namen neu starten.

**Zur Kadenz:** Die Lastform ist 10 Hz kleine Fenster plus 1 Hz Grossfenster je
Sonde. Das 1-Hz-Grossfenster ist eine **Lastform** (16.384 Samples, der teuerste
Fall aus A23), keine Aussage darüber, auf welchem Pfad Bassbänder reisen — laut
Entwurf `:3578` reisen sie auf dem 10-Hz-Livepfad. A24 verschiebt daran nichts.

### 2.2 Die Lücke, die bleibt (Z7, XRun)

„Ohne XRun" ist im Entwurf (`:3586`) eine Aussage über den **Audiotreiber im
echten Host**. Ein Konsolenprogramm hat keinen Audiotreiber. A24 ersetzt das
nicht, sondern misst einen benannten Ersatzpfad und nennt dessen Grenze:

- **Was gemessen wird:** N echte `SondeProcessor` treiben in Echtzeitkadenz den
  echten Sondenpfad. Gezählt werden Ganzblockdrops
  (`analyseDropsUeberlaufFuerTest()`, `analyseDropsOversizeFuerTest()`,
  `eq-copilot/plugin/sonde/SondeProcessor.h:210-217`) und Kontinuitätsbrüche.
  Diese Threads konkurrieren im **selben Prozess** mit N echten IPC-Clientpaaren
  unter Dauerlast um CPU und um das Named-Pipe-Subsystem des Betriebssystems.
  Steigen die Ganzblockdrops unter dieser Last, ist das sichtbar.
- **Was nicht gemessen wird:** Unter `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1`
  (`eq-copilot/plugin/CMakeLists.txt:391`) startet `SondeProcessor` seine eigenen
  `controlV3`/`telemetryV3` **nicht** (`sonde/SondeProcessor.cpp:100-105`). Der
  Worker ruft `veroeffentlichen` weiterhin (`sonde/SondeProcessor.cpp:377`), und
  der Ruf läuft real in die Schleuse (`core/ipc/TelemetryClient.cpp:340-351`) —
  aber niemand entleert sie auf den Draht. Die Kopplung „**Rückstau auf dem
  Draht bremst den Analyseworker derselben Instanz**" wird damit **nicht**
  gefahren.
- **Warum nicht anders:** `SondeProcessor::v3PipeName` ist `const` und wird im
  Konstruktor aus `nakama::ipc::pipeNameV3(v3LogonSid)` gesetzt
  (`sonde/SondeProcessor.h:271`, `sonde/SondeProcessor.cpp:73`). Es gibt keinen
  Injektionspunkt für einen Probe-Namen. Einen zu schaffen wäre Produktcode im
  Plugin (Ticketgrenze) und führte ausserdem eine Testbinärdatei an die
  **Produktions**-Pipe — genau das, was Z8 verbietet.
- **Folge:** Die XRun-Hälfte des Entwurfsbudgets bleibt ausdrücklich ein
  **FL-Termin**. Sie wird nicht durch einen Ersatzzähler als erledigt
  ausgegeben. A24 sagt zu: „0 Ganzblockdrops und 0 unerklärte
  Kontinuitätsbrüche, während N IPC-Paare unter Dauerlast im selben Prozess
  laufen" — nicht „kein XRun in FL".

## 3. Fristen und Konstanten — abgelesen, nicht angenommen

| Grösse | Wert | Quelle |
|---|---|---|
| P0-Heartbeat-Kadenz (Vertrag) | 1.000 ms | `broker/src/coordinator.rs:28` `HEARTBEAT_INTERVAL_MS`; C++-Gegenstück `core/ipc/ControlClient.cpp:27` `kHeartbeatTaktMs` |
| Control wird stale | > 2.500 ms ohne Kontakt (exklusiv) | `broker/src/coordinator.rs:29-31`: `STALE_VERPASSTE_INTERVALLE 2 · 1000 + STALE_JITTER_MS 500` |
| Eviction (Tombstone) | ab 10.000 ms stale | `broker/src/coordinator.rs:32` `TOMBSTONE_MS` |
| Reconnect-Backoff Client | Start 500 ms, Verdopplung, Deckel 8.000 ms; nach *erfolgreicher* Verbindung zurück auf 500 ms | `core/ipc/IpcVerbindung.h:34-35`; Rücksetzung `core/ipc/ControlClient.cpp:984-992`, `core/ipc/TelemetryClient.cpp:370-405` |
| Verbindungsversuch bei belegter Pipe | bis 20 × `WaitNamedPipeW(200 ms)` = 4.000 ms | `core/ipc/IpcVerbindung.cpp:67-89` |
| Hello/Welcome-E/A-Frist | 5.000 ms je Richtung | `core/ipc/IpcVerbindung.h:38` `kIoFristMs`; Verwendung `core/ipc/ControlClient.cpp:1148,1161` |
| Telemetriekopplung wartet auf `link_id`/`challenge` | Takt 51 ms (`kBackoffStartMs/10 + 1`) | `core/ipc/TelemetryClient.cpp:393` |
| Sichtbare Sonden normal / Lastvertrag | 16 / 32 | `broker/src/coordinator.rs:36-37` |
| Client-Deckel Session / global | 64 / 128 | `broker/src/coordinator.rs:33-34` |
| P0-Latenzschranke (von A22 übernommen) | 1.000 ms | `eq-copilot/plugin/tests/IpcLastMain.cpp:42` `kMaxP0LatenzMs` |
| Probe-Namensraum | Präfix `PROBE_PRAEFIX`, Rest darf nicht leer sein | `broker/src/transport/pipetoken.rs:28-40`; C++-Gegenstück `istProbePipename` in `core/ipc/PipeToken.h` |

### 3.1 Ableitung der Neustartfrist (Z2)

Die Frist ist **nicht frei gewählt**, sondern aus den Zeilen oben addiert. Ab dem
Zeitpunkt, an dem der neue Broker wieder horcht (seine `BEREIT`-Zeile auf
stdout), gilt im schlechtesten Fall je Client:

```
   8.000 ms   laufender Backoff bis zum nächsten Versuch (Deckel kBackoffMaxMs)
+  4.000 ms   WaitNamedPipe-Schleife, wenn N+1 Clients gleichzeitig verbinden
+  5.000 ms   Hello schreiben       (kIoFristMs)
+  5.000 ms   Welcome lesen         (kIoFristMs)
+  1.000 ms   eine Heartbeat-Kadenz, bis der Descriptor beim Coordinator steht
= 23.000 ms   Control-Runde
```

Die Telemetrierunde folgt danach mit eigenem Backoff und eigener Kopplung.

> **Festgelegte Frist Z2: 30 s ab „Broker horcht wieder".**

Der Wert liegt über der gerechneten Control-Runde und in derselben
Grössenordnung wie die 25 s, die `Sonde012SourcesLatencyTest.cpp:206` heute
schon für den kalten Aufbau der vollständigen Topologie zulässt. Wird er im
Gate-Lauf verfehlt, ist das ein Befund am Produkt — kein Anlass, die Frist zu
erhöhen.

### 3.2 Startbudget Wachstum (Z3)

Entwurf §49.3 stellt seine Budgets ausdrücklich als **änderbare Startwerte mit
Abnahmetest** ein (`docs/FL-Nakama-Sonden-Design-Entwurf.md:3570-3573,3588-3590`).
Das Bein trägt deshalb eine feste Zahl:

> Nach dem Warmup wächst der Working Set weder des Brokerprozesses noch des
> Clientprozesses bis zum Laufende um mehr als **max(10 %, 16 MiB)**.

Weicht eine Messung ab, bleibt die Zahl im Bein stehen und die Abweichung samt
Messkurve kommt in dieses Manifest. Sie wird nicht stillschweigend nachgezogen.

**Messverfahren:** `GetProcessMemoryInfo` (psapi) über `ctypes` — reine
Standardbibliothek, kein `psutil`. Gelesen wird
`PROCESS_MEMORY_COUNTERS.WorkingSetSize`, minütlich, für beide PIDs.

**Der Broker wird während des Laufs getötet.** Seine Kurve bricht an jedem
Neustart ab. Das Budget gilt deshalb **je Brokergeneration**: Basiswert am Ende
des Warmups dieser Generation, Vergleichswert am Ende derselben Generation. Der
Clientprozess läuft durch; für ihn gilt eine einzige Kurve über den ganzen Lauf.
Eine Generation, die kürzer als zwei Messminuten ist, liefert keinen
Wachstumswert und wird im Bericht als `unzureichend` ausgewiesen — nicht als
bestanden.

## 4. Callback-Reihenfolge

Die Matrix in §5 verweist auf diese drei Folgen. Sie sind aus dem heutigen Code
abgelesen, nicht erfunden.

### 4.1 Main — Verbinden und Subscribe

1. `ControlClient` verbindet, authentisiert den Server
   (`namedPipeServerAuthentisieren`, `core/ipc/IpcVerbindung.cpp:106`), schreibt
   Hello, liest Welcome.
2. Der Verbunden-Callback läuft. In ihm zuerst
   `model.beginneSubscription(projectBindingId, sessionEpoch, eigeneInstanceId)`,
   **danach** `sendeP1("subscribe_session", …)`
   (`Sonde012SourcesLatencyTest.cpp:131-140`). Die Reihenfolge ist bindend: das
   Modell muss das Subscription-Fenster offen haben, bevor der erste Snapshot
   eintreffen kann.
3. Wird der P1 abgewiesen oder ist er zu gross, folgt sofort
   `model.controlEnde()`. Kein halboffener Zustand.
4. Der Broker validiert und pusht einen **absoluten** `session_snapshot`. Der
   Control-Lese-Callback des Main nimmt ihn entgegen; A24 liest daraus **zuerst
   selbst** `broker_epoch` (flaches JSON, Helfer `flachesJsonObjekt`/`jsonText`
   aus `core/ipc/IpcVerbindung.h`, Muster `IpcLastMain.cpp:122-140`) und reicht
   ihn **danach** an `model.uebernehmeSessionSnapshot(...)` weiter.
   Grund: `SourcesModel::Sicht` (`src/SourcesModel.h:75-84`) veröffentlicht
   `fuehrendesMain`, aber **nicht** `brokerEpoch` — das Feld ist privat
   (`src/SourcesModel.h:157`). Ohne den eigenen Lesepfad wäre Z2 am Main nicht
   beobachtbar, und ein Produktcode-Zugriff wäre Ticketgrenzverletzung.
5. `TelemetryClient` des Main verbindet erst, wenn `control->kopplung(linkId,
   challenge)` gültige Hex32-Werte liefert (`core/ipc/TelemetryClient.cpp:383-393`).

### 4.2 Sonde — Verbinden und Reporten

1. `ControlClient` verbindet, Hello mit `pluginKind = "active_probe"`, Welcome.
2. Coordinator löst Projekt/Session gegen das eindeutig führende Main auf, bildet
   den Descriptor, setzt Control-Liveness `connected`, markiert die Session dirty
   und pusht einen neuen absoluten Snapshot (SONDE-012 Matrixzeile L03).
3. `TelemetryClient` der Sonde verbindet nach gültiger Kopplung; erst danach
   sind P2-Frames überhaupt zustellbar.
4. Heartbeats laufen mit 1.000 ms Kadenz weiter; ein P2-Frame setzt den
   Control-Liveness-Timer **nicht** zurück (SONDE-012 L08).

### 4.3 Brokerneustart — Trennen und Wiederkommen

1. Broker-Prozess stirbt. Jeder Client sieht seinen Lese- oder Schreibfehler und
   verlässt `eineVerbindung`.
2. **Main:** Verbunden-Callback mit `false` ⇒ `model.controlEnde()`. Die
   linkeigene Subscription ist beim Coordinator ohnehin fort — der Coordinator
   existiert nicht mehr. Ab hier zeigt das Modell `disconnected`, nicht die alte
   grüne Wahrheit.
3. **Alle Clients:** Backoff (§3, Zeile „Reconnect-Backoff"). Solange die Pipe
   nicht existiert, kommt `ERROR_FILE_NOT_FOUND` sofort zurück
   (`core/ipc/IpcVerbindung.cpp:84-85,90-99`) und der Backoff verdoppelt sich bis
   zum Deckel. Es wird **kein anderer Pipename** versucht.
4. Neuer Broker horcht auf demselben Probe-Namen mit **neuer** `broker_epoch`
   (`neue_kennung()`, `broker/src/bin/eqcop-broker-sonde012-probe.rs:34`). Ab
   seiner `BEREIT`-Zeile läuft die 30-s-Uhr aus §3.1.
5. Reihenfolge wie §4.1/§4.2, unverändert: der Main subscribt neu, die Sonden
   reporten neu. Kein neuer Produktpfad — genau die bestehenden Client-Wege.

## 5. Verhaltensmatrix

Spalte „Zusage" nennt die Nummer aus dem Auftrag (Z1–Z8). Spalte „Test" nennt
das Bein, das **genau diese Zeile** misst; `A24` bezeichnet den neuen
Prüfer/Client, `A24:<name>` einen benannten Prüfpunkt darin.

| ID | Ausgangszustand × Ereignis | Zusage, Callback-Reihenfolge und Frist | Z | Test, der genau diese Zeile misst | Quelle |
|---|---|---|---|---|---|
| S01 | Kalt, nichts verbunden × Lauf startet | Reihenfolge §4.1 und §4.2. Innerhalb **30 s** stehen 1 Main-Paar und N Sondenpaare, und der Main hat einen absoluten Snapshot erhalten. Wird die Frist verfehlt, endet der Lauf mit Exit 1 und einer Diagnosezeile (Muster `Sonde012SourcesLatencyTest.cpp:225-240`), nie mit „grün ohne Topologie". | Z1 | **NEU** `A24:topologie_steht_in_frist` | §3.1; `Sonde012SourcesLatencyTest.cpp:206-240`; `IpcLastMain.cpp:167-195` |
| S02 | Topologie steht × Warmup abgelaufen (Gate 2 min, Kurzform 20 s) | Der Snapshot beim Main zeigt **genau N** Mitglieder; die Menge der `instanceId` ist gleich der erwarteten Menge; `fuehrendesMain` ist die Main-Adresse; **keine** fremde Adresse taucht auf. Geprüft auf der Sicht des echten `SourcesModel`, nicht auf dem Rohtext. | Z1 | **NEU** `A24:mitgliedschaft_nach_warmup` | `src/SourcesModel.h:75-84`; Entwurf `:4262-4264`; SONDE-012 L03 |
| S03 | Eingeschwungen × Dauerbetrieb bis Laufende: je Sonde 10-Hz-Liveframes (2.048 Samples), 1-Hz-Grossfenster (16.384 Samples) und 1-Hz-Heartbeats | Die Mitgliedschaft driftet über die ganze Laufzeit nicht: Zeilenzahl bleibt N, die `instanceId`-Menge bleibt identisch, `fuehrendesMain` bleibt dieselbe Adresse. Jede Snapshot-Übernahme wird geprüft, nicht nur die letzte. Ein einziger abweichender Snapshot ausserhalb der Neustartfenster ist ROT. | Z1 | **NEU** `A24:mitgliedschaft_driftet_nicht` | Entwurf `:4262-4264`; `:3586` („ohne ungegrenztes Wachstum") |
| S04 | Eingeschwungen × P0-Heartbeat je Sonde, 1.000 ms Kadenz | Reihenfolge: Sendezeit vor `sendeP0` stempeln; wird `sendeP0` abgewiesen, gilt der Heartbeat als **nie eingereiht** und wird nicht erwartet (Muster `IpcLastMain.cpp:229-235`). Ausserhalb der Neustartfenster geht **kein** ACK verloren; p95 der ACK-Latenz **< 1.000 ms**, auch während die langsamen Leser bremsen. | Z4 | **NEU** `A24:p0_kein_verlust_und_p95` | `IpcLastMain.cpp:42,216-240`; `coordinator.rs:28` |
| S05 | Eingeschwungen × ein Viertel der Sonden liest künstlich verzögert | Die künstliche Verzögerung wird **nach** dem Zeitstempel des Empfangs eingelegt, nie davor. Sonst mässe das Bein seine eigene Bremse statt der Drahtlatenz — die Zusage wäre unfalsifizierbar. Die langsamen Sonden bleiben über den ganzen Lauf Mitglied und werden ausserhalb der Neustartfenster nie stale. | Z6, Z5 | **NEU** `A24:langsame_bleiben_mitglied` | Prüfliste Abschnitt A; `coordinator.rs:29-31` |
| S06 | Langsamer Leser × seine Telemetrieschleuse läuft voll | Rückstau heisst „der älteste weicht": `ersetzt > 0`, `beanspruchtVerworfen == 0`. Jede abgelehnte Veröffentlichung hat einen gezählten Grund (`abgelehnt == zuGross + beanspruchtVerworfen`). Kein langsamer Leser verzögert die schnellen: deren p95 aus S04 hält unabhängig. | Z6 | **NEU** `A24:cap_ersetzt_aeltesten_und_blockiert_nie` | `core/ipc/TelemetryClient.cpp:355-364`; `IpcLastMain.cpp:311-330`; Prüfliste A |
| S07 | Eingeschwungen × Minutentick | RSS beider Prozesse wird minütlich gelesen und als vollständige Kurve berichtet. Nach dem Warmup wächst keiner bis zum Ende seiner Generation um mehr als **max(10 %, 16 MiB)**. Brokerkurve je Generation, Clientkurve über den ganzen Lauf (§3.2). | Z3 | **NEU** `A24:speicherkurve_im_budget` | Entwurf `:3586,3588-3590`; §3.2 |
| S08 | Eingeschwungen × N `SondeProcessor` treiben Ganzblöcke in Echtzeitkadenz | 0 Ganzblockdrops durch Überlauf, 0 durch Oversize; 0 Kontinuitätsbrüche ausserhalb bewusst erzeugter Transportkanten. Kontinuität wird über `transport.continuity_segment` des zuletzt veröffentlichten Producer-Frames beobachtet (`letzterProducerFrameFuerTest`), weil `SondeProcessor` selbst keinen Bruchzähler veröffentlicht — der liegt nur auf `PluginProcessor` (`src/PluginProcessor.h:146`). Grenze §2.2 gilt. | Z7 | **NEU** `A24:audio_ganzblock_ohne_drop` | `sonde/SondeProcessor.h:205-217`; `core/analysis/FeatureEngine.h:140-144`; B4 `QueueStressTestMain.cpp:504-556`; B12 |
| S09 | Eingeschwungen × Broker-Prozess wird zum festen Zeitpunkt getötet | Reihenfolge §4.3 Schritte 1–3. Der Main verlässt `subscriptionAktiv`; das Modell zeigt `disconnected`, nicht die letzte grüne Sicht. Die Zeitmessung des Neustartfensters beginnt beim Kill, die Fristmessung erst bei `BEREIT`. | Z2 | **NEU** `A24:kill_beendet_subscription_sichtbar` | §4.3; SONDE-012 L10; `src/SourcesModel.h:99-108` |
| S10 | Broker tot × Clients im Backoff | Kein Absturz, keine Endlosschleife, **kein anderer Pipename**. Die Clients verdoppeln ihren Backoff bis 8.000 ms und versuchen ausschliesslich denselben Probe-Namen. Der Zeitraum wird als Neustartfenster markiert; Zusagen Z4/Z5 sind darin ausgesetzt und werden getrennt gezählt, nicht stillschweigend ausgelassen. | Z2, Z8 | **NEU** `A24:backoff_ohne_namenswechsel` | `core/ipc/IpcVerbindung.cpp:67-99`; `core/ipc/ControlClient.cpp:1014-1025` |
| S11 | Neuer Broker horcht auf demselben Namen × Clients reconnecten | Innerhalb **30 s ab `BEREIT`**: der Main subscribt neu (Reihenfolge §4.1), alle N Sonden reporten neu, und der Snapshot ist wieder vollständig (N Mitglieder, erwartete Adressen, `fuehrendesMain` = Main-Adresse). Ein Reconnect nutzt ausschliesslich die bestehenden Client-Pfade; A24 fügt keinen Produktpfad hinzu. | Z2, Z1 | **NEU** `A24:reconnect_vollstaendig_in_frist` | §3.1; §4.3; Entwurf `:4262-4264` |
| S12 | Nach dem Neustart × jeder weitere Snapshot | Die `broker_epoch` ist eine **neue**; die alte wird nach dem Neustart nie wieder als aktueller Lauf ausgespielt. Der Zähler dafür muss strukturell 0 bleiben und ist eine Wache mit Test (Prüfliste A). Keine fremde Steueradresse erscheint. | Z2 | **NEU** `A24:alte_epoche_kommt_nie_wieder`; Gegenstueck im Unittest bestehend in `broker/tests/sonde012_sources_slice.rs`, Test `restart_has_no_old_live_or_measurement_truth` (dort Zeile 696) | SONDE-012 L12 (`docs/beweise/SONDE-012.md:152`); `broker/src/bin/eqcop-broker-sonde012-probe.rs:34` |
| S13 | Beliebig × Liveness-Tick | Ausserhalb der Neustartfenster wird **kein** Mitglied stale (Grenze > 2.500 ms) und keines evicted (10.000 ms). Innerhalb der Fenster ist beides erlaubt und wird getrennt gezählt und berichtet. Die Heartbeat-Kadenz von 1.000 ms hält den Abstand zur Stale-Grenze; wird sie im Lauf nicht erreicht, ist das der Befund. | Z5 | **NEU** `A24:kein_stale_ausserhalb_neustart`; bestehend in `broker/tests/coordinator_model.rs` die Tests `stale_konstanten_ableitung` (Zeile 527) und `stale_grenze_exklusiv_und_kontakt_setzt_zurueck` (Zeile 535) | `broker/src/coordinator.rs:28-32`; SONDE-012 L07 |
| S14 | Lauf läuft × Laufende erreicht | Gegenpfad zu S01: Nachlauffenster für ausstehende ACKs, dann Telemetrie **vor** Control stoppen (Muster `IpcLastMain.cpp:280`), Audiotreiber-Threads beenden und joinen, `SondeProcessor` zerstören, danach dem Broker `STOP` schicken und auf ihn warten. Kein hängender Thread, kein verwaister Prozess; das Bein endet mit einer Urteilszeile und einem Exitcode, nie mit Stille. | — (Änderungssatz: verbinden↔trennen, starten↔stoppen) | **NEU** `A24:sauberer_abbau` | Prüfliste Abschnitt B und F; `IpcLastMain.cpp:280`; `sonde/SondeProcessor.cpp:107-115` |
| S15 | Beliebig × Programm bekommt einen Nicht-Probe-Pipenamen | Beide Programme — Prüfer-Client und Probe-Broker — verweigern den Dienst mit **Exit 3** für Produktions- und Golden-Pipename. Der Riegel wird **gefahren**, nicht behauptet: der Prüfer startet beide Binaries vor dem eigentlichen Lauf einmal mit jedem verbotenen Namen und verlangt Exit 3 (Muster A23 `pruefe_sonde012_sources_latency.py:40-48`). Es ist eine Erlaubnisliste, keine Sperrliste. | Z8 | **NEU** `A24:verweigert_produktion_und_golden` | `broker/src/transport/pipetoken.rs:28-45`; `pruefe_ipc_last.py:75-89`; `IpcLastMain.cpp:74-82` |

### 5.1 Was in dieser Matrix bewusst nicht steht

- **Kein XRun.** Siehe §2.2. Die Zeile S08 sagt „Ganzblockdrops", nicht „XRun".
- **Keine 32-Sonden-Zusage.** Der Entwurf verlangt zusätzlich „mindestens 30 min
  mit 32 Sonden" (`:3586`). Das Bein **kann** das über `--sonden 32 --minuten 30`,
  aber A24 sagt es nicht als Kanon zu; die Kanon-Kurzform fährt 16 Sonden über
  2 Minuten. Der 32er-Lauf ist ein eigener Gate-Aufruf und wird als solcher im
  Gate-Manifest belegt, nicht hier vorweggenommen.
- **Keine Aussage über den produktiven Broker.** Alles läuft im Probe-Namensraum.

## 6. Killmatrix — der Brokerneustart

Der Neustart ist ein **echter Prozesstod**, kein zurückgegebener Fehler
(Präzedenz: `broker/tests/store_crash_matrix.rs`, `docs/beweise/SONDE-011.md:584-590`).
Der Prüfer beendet den Brokerprozess hart und startet ihn auf demselben
Probe-Namen neu.

Die Kills liegen zu **festen Zeitpunkten** im Lauf. Was beim Kill gerade in
Flug ist, ist damit teils Konstruktion, teils Wahrscheinlichkeit. Die Spalte
„Wie dieser Punkt getroffen wird" sagt für jede Zeile, welches von beidem gilt —
eine Zeile, die nur wahrscheinlich getroffen wird, wird nicht als deterministisch
ausgegeben.

| ID | Killpunkt | Erwartung nach dem Neustart | Wie dieser Punkt getroffen wird | Test |
|---|---|---|---|---|
| K-S1 | Broker stirbt **mitten im Frameverkehr** — mindestens ein P2-Liveframe ist unterwegs oder halb geschrieben | Ein halber Frame resynchronisiert nicht, sondern beendet den Strom (bestehende Envelope-Regel). Nach dem Neustart ist der erste Zustand am Main `disconnected`, danach wieder vollständig innerhalb 30 s. Kein halber Frame wird je als Messwert sichtbar. | **Deterministisch by Konstruktion.** Bei N=16 Sonden × 10 Hz Liveframes + 1 Hz Bassframes ist zu jedem Zeitpunkt Frameverkehr in Flug; ein Kill zu beliebiger Wanduhrzeit trifft ihn. | **NEU** `A24:kill_im_frameverkehr`; bestehende Envelope-Hälfte `broker/tests/transport_fuzz.rs` (ein kaputter Frame beendet den Strom) |
| K-S2 | Broker stirbt **während `subscribe_session`** — nach Verbindung des Main, bevor der absolute Snapshot beim Main ankommt | Der Main hält keinen halb offenen Subscription-Zustand: `controlEnde()` läuft, `subscriptionAktiv` ist false, das Modell zeigt `disconnected`. Der nächste erfolgreiche Reconnect subscribt erneut und bekommt einen **absoluten** Snapshot; es gibt kein `unsubscribe_session` und keinen Event-Replay. | **Deterministisch by Konstruktion, ab `--neustarts >= 2`:** ein „Doppelschlag" — der Broker wird ein zweites Mal getötet, sobald seine `BEREIT`-Zeile erschienen ist und der Main im Reconnect steht. Die 30-s-Uhr für Z2 läuft dann ab der **letzten** `BEREIT`-Zeile. Bei `--neustarts 1` (Kanon-Kurzform) ist diese Zeile nicht abgedeckt und wird im Bericht als `nicht_gefahren` ausgewiesen. | **NEU** `A24:kill_waehrend_subscribe` |
| K-S3 | Broker stirbt **während eines Heartbeats** — P0 ist gesendet, das `heartbeat_ack` kommt nicht mehr | Der verlorene Heartbeat wird als `p0_verloren_im_neustartfenster` gezählt und **nicht** als Verletzung von Z4 gewertet; Z4 gilt ausserhalb der Fenster. Nach dem Reconnect laufen Heartbeats derselben Sonde weiter; die Sequenznummern springen nicht rückwärts und kein ACK wird doppelt verbucht. | **Deterministisch by Konstruktion.** N Sonden senden mit 1.000 ms Kadenz; ein Kill trifft im Erwartungswert ausstehende ACKs. Der Zähler ist im Bericht sichtbar; er darf 0 sein, dann ist die Zeile im Lauf nicht getroffen und wird als `nicht_getroffen` gemeldet, nicht als bestanden. | **NEU** `A24:kill_waehrend_heartbeat` |
| K-S4 | Broker stirbt, **während ein langsamer Leser noch am Rückstau hängt** | Der langsame Leser blockiert weder seinen eigenen Reconnect noch den der anderen. Er ist innerhalb derselben 30-s-Frist wieder Mitglied. Seine Schleuse verwirft dabei alte Liveframes (Cap), verliert aber nie den neuesten. | **Deterministisch by Konstruktion** — die langsamen Leser bremsen den ganzen Lauf über, also auch zum Killzeitpunkt. | **NEU** `A24:kill_mit_langsamem_leser` |
| K-S5 | Zwei Kills **kurz hintereinander**, sodass Clients im Backoff-Deckel (8.000 ms) stehen | Auch aus dem Deckel heraus verbinden alle Clients innerhalb der 30-s-Frist ab der letzten `BEREIT`-Zeile — genau dafür ist die Frist in §3.1 aus dem Deckel plus E/A-Fristen gerechnet. Kein Client bleibt dauerhaft hängen. | **Deterministisch by Konstruktion, ab `--neustarts >= 2`** (derselbe Doppelschlag wie K-S2, aus dem Backoff-Deckel gemessen). | **NEU** `A24:kill_aus_backoff_deckel` |

## 7. Bericht und Zählerkarte

Der Prüfer schreibt **eine** JSON-Zeile mit allen Zählern und der minütlichen
Speicherkurve auf stdout; sie geht roh ins Manifest. Kein Zähler wird
weggelassen, weil er 0 ist — ein Zähler, der strukturell 0 sein muss, ist eine
Wache mit Test (Prüfliste A).

Feste Felder (Namen sind Vertrag zwischen C++-Programm und Prüfer, kein
Wire-Vertrag):

```
sonden, minuten, neustarts, langsam_anzahl, warmup_s
mitgliedschaft: snapshot_pruefungen, vollstaendig, fremde_adresse,
                fuehrendes_main_falsch
neustart[]:     index, epoch_alt, epoch_neu, bereit_bis_vollstaendig_ms,
                frist_ms, alte_epoche_nach_neustart_gesehen
p0:             gesendet, beantwortet, verloren_ausserhalb_neustart,
                verloren_im_neustartfenster, latenz_p95_ms, latenz_max_ms,
                schranke_ms
liveness:       stale_ausserhalb_neustart, evicted_ausserhalb_neustart,
                stale_im_neustartfenster
langsam:        sonden, immer_mitglied, ersetzte_liveframes,
                neueste_verworfen, abgelehnt, zu_gross, schnelle_p95_ms
audio:          bloecke, ganzblockdrops_ueberlauf, ganzblockdrops_oversize,
                kontinuitaetsbrueche, publikationen
speicher:       takt_s, budget_prozent, budget_bytes,
                client[]{minute, rss_bytes},
                broker[]{generation, minute, rss_bytes, urteil}
kill:           je Killmatrixzeile getroffen | nicht_getroffen | nicht_gefahren
```

**Exitcodes** wie bei A22/A23: `0` grün · `2` Zusage verfehlt · `3`
Voraussetzung fehlt (Binary nicht gebaut, Broker nicht `BEREIT`).

## 8. Sicherheitsriegel (Z8)

- Der Pipename wird vom Prüfer erzeugt: `PROBE_PRAEFIX` + `soak.` + PID +
  Zeitstempel — derselbe Bau wie `pruefe_ipc_last.py:55-56`.
- Vor dem Lauf prüft der Prüfer **fahrend**, dass Probe-Broker und
  `EqCopSessionSoak` sowohl den Produktions- als auch den Golden-Pipenamen mit
  Exit 3 verweigern (Matrixzeile S15).
- Beide Programme entscheiden über eine **Erlaubnisliste**
  (`ist_probe_pipename` / `istProbePipename`), nie über eine Sperrliste. Der
  Grund steht in `broker/src/transport/pipetoken.rs:32-37`: eine Sperrliste
  liess am 2026-08-29 ausgerechnet den produktiven v3-Namensraum durch.
- Es wird **nie** ein Testbroker auf der Produktions-Pipe gestartet. Die
  `SondeProcessor`-Instanzen laufen unter
  `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1` und starten ihre eigenen v3-Clients
  nicht (§2.2) — auch deshalb berührt kein Teil dieses Beins die Produktionspipe.

## 9. Parameter und Kanonform

| Parameter | Standard | Kanon-Kurzform (A24) | Gate-Lauf |
|---|---|---|---|
| `--sonden` | 16 | 16 | 16 (zusätzlicher Lauf 32) |
| `--minuten` | — | 2 | 60 (32-Sonden-Lauf: 30) |
| `--neustarts` | 1 | 1 | 3 |
| `--langsam` | 0.25 | 0.25 | 0.25 |
| Warmup | — | 20 s | 2 min |

Neustart *k* liegt bei Bruchteil *k/(neustarts+1)* des Messfensters. Der
Doppelschlag aus K-S2/K-S5 hängt an Neustart 2 und existiert erst ab
`--neustarts >= 2`.

**Registrierung in `tools/beweise.ps1`:**

- Kanon-Eintrag `A24`, `Art='python'`, `Name='pruefe_session_soak.py'`,
  `AbPhase='P3'` — neben A23 (`tools/beweise.ps1:575`).
- Bauziel `EqCopSessionSoak` in `$gemesseneZiele` mit
  `Marker='juce_add_console_app(EqCopSessionSoak'` und einer `Wegen`-Zeile
  (Muster `tools/beweise.ps1:675-681`). Ohne diesen Eintrag mässe A24 ein altes
  Artefakt (Prüfliste D, NAK-93).
- Der Probe-Broker `eqcop-broker-sonde012-probe` wird bereits im
  `cargo build --release`-Schritt mitgebaut (`tools/beweise.ps1:768-772`); ein
  zweiter Eintrag ist nicht nötig und wäre eine tote Zeile.
- Die `Behauptung` bleibt kurz und nennt das Gate-Mass (Prüfliste E: die
  A14-Behauptung war am 30.08. selbst Befundquelle). Wachen und Ausnahmen gehören
  in den Skriptkopf, nicht in die Behauptung.

## 10. Prüfliste — wo jede Zeile gemessen wird

`tools/dirigent/pruefliste.md`, abgehakt vor jedem Commit. Diese Spalte ist der
Plan für Phase 2; nach dem Bau steht hier der gemessene Ort.

| Prüflistenzeile | Wo A24 sie misst |
|---|---|
| A — Politik bei *voll* steht ausdrücklich je Klasse | S06: `ersetzt`/`beanspruchtVerworfen`/`zuGross` werden einzeln berichtet und geprüft, nicht summiert |
| A — Rückgabewerte und Zähler werden ausgewertet, nicht ignoriert | S04 (`sendeP0`-Rückgabe entscheidet über „eingereiht"), S06 (`veroeffentlichen`-Rückgabe zählt), S12 (`alte_epoche_nach_neustart_gesehen` ist eine Wache, die strukturell 0 sein muss) |
| A — höhere Klasse wird gelesen, solange niedrigere blockiert | S05 + S06: die langsamen Leser bremsen P2, während S04 die P0-p95 der schnellen Sonden misst |
| B — Reihenfolge beim Verbinden festgelegt und gemessen | §4.1/§4.2, gemessen in S01 und S11 |
| B — beim Trennen zuerst abmelden, dann Fristen und Joins | S14: Telemetrie vor Control stoppen, Audiothreads joinen, dann Broker `STOP` |
| B — jeder Join hat eine Frist | S01 (30 s), S11 (30 s ab `BEREIT`), S14 (Nachlauffenster und Join-Fristen) |
| C — Verträge und Längen | **nicht Gegenstand von A24.** Das Bein berührt keinen Vertrag; die Vertragsriegel bleiben bei A5/A9/B3c/A21 |
| D — ein Bein prüft die Frische seiner Eingaben und meldet Exit 3 | Prüfer prüft die Existenz beider Binaries vor dem Lauf (Muster `pruefe_ipc_last.py:63-72`); die Frische erzwingt der Runner über `$gemesseneZiele` (§9) |
| D — was der Kanon nicht baut, darf er nicht als frisch bezeugen | §9: `EqCopSessionSoak` steht in `$gemesseneZiele` |
| E — Behauptung ≤ Messung | §2.2 (XRun-Grenze steht ausdrücklich drin), §5.1 (was die Matrix nicht sagt), K-S2/K-S3 (`nicht_getroffen`/`nicht_gefahren` statt stiller Erfolg) |
| E — Zahlen im Manifest sind gemessen, nicht abgeschrieben | §3 (jede Konstante mit `Datei:Zeile`), §7 (jeder Zähler kommt aus dem Lauf) |
| E — jede neue Prüfung wurde einmal absichtlich gebrochen | Phase 2: je Zusage Z1–Z8 ein absichtlich gebrochener Lauf mit Rohausgabe des Rots im Rundenabschnitt dieses Manifests |
| F — verbinden↔trennen, starten↔stoppen im selben Änderungssatz | S01↔S14 und S09↔S11 sind in derselben Datei und demselben Commit |

## 11. Offene Punkte dieser Phase

1. **32-Sonden-Hälfte des Entwurfsbudgets** (`:3586`, „mindestens 30 min mit
   32 Sonden"): technisch derselbe Aufruf mit anderen Parametern, aber ein
   eigener Gate-Lauf mit eigener Laufzeit. Er ist **nicht** Teil der Kanon-Kurzform
   und wird im G3-Gate-Manifest belegt, nicht hier. Kein Produktentscheid nötig.
2. **XRun** bleibt FL-Termin (§2.2). Kein Produktentscheid nötig — eine
   Konsolenbinärdatei kann es nicht messen.
3. **Kein offener Produktentscheid.** Dieses Ticket ist reines Prüfwerkzeug;
   es ändert kein sichtbares Produktverhalten und legt dem User nichts vor.

## 12. Verlauf

| Datum | Schritt | Stand |
|---|---|---|
| 2026-09-02 | Phase 1: Verhaltensmatrix, Killmatrix, Fristen aus dem Code abgeleitet; kein Code geschrieben | Basis-Commit `03e1e17` |
