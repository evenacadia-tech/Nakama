# G3-SOAK — Werkzeugticket: das 60-Minuten-Dauerlaufbein A24

<!-- NAKAMA-URTEIL: PHASE 1b (Verhaltensmatrix nachgearbeitet) 2026-09-02 — kein Bau, keine Messung -->

| Merkmal | Wert |
|---|---|
| Ticket | G3-SOAK (Werkzeugticket zum Phasengate **G3**) |
| Basis-Commit | `03e1e17` (master, sauberer Worktree) |
| Phase | **1b von 2** — Spezifikation vor Code (Dirigent-Skill §3.2). Diese Datei enthält keinen Code und keine Messung. Phase 1 lief am 02.09.2026 durch einen lesenden Codex-Prüfer und kam als **NEEDS_WORK** mit vier Befunden zurück; Phase 1b schliesst genau diese vier (§12). |
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

Ein Clientpaar ist erst vollständig, wenn **beide** Verbindungen stehen (S01).
Control- und Telemetrierunde laufen zwingend nacheinander: der Telemetriethread
verbindet erst, wenn `link_id` und `challenge` aus dem Control-Welcome
vorliegen, und wartet sonst in einer eigenen Schleife
(`core/ipc/TelemetryClient.cpp:383-398`). Die Telemetrierunde hat danach ihren
**eigenen** Backoff, ihren eigenen Öffnungslauf und ihre eigenen zwei
Bootstrap-E/A-Fristen (`core/ipc/TelemetryClient.cpp:518,596,609`). Die erste
Fassung dieses Abschnitts hörte nach der Control-Runde auf und war damit keine
obere Schranke; das ist hier berichtigt.

Obere Schranke je **Clientpaar**, gerechnet ab „Broker horcht wieder"
(`BEREIT`-Zeile auf stdout):

```
Control-Runde
     8.000 ms   Backoff bis zum naechsten Versuch   (Deckel kBackoffMaxMs)
  +  4.000 ms   WaitNamedPipe-Schleife, 20 x 200 ms
  +  5.000 ms   Hello schreiben                     (kIoFristMs)
  +  5.000 ms   Welcome lesen                       (kIoFristMs)
  = 22.000 ms

Kopplung
  +     51 ms   ein Takt kBackoffStartMs/10 + 1, bis link_id/challenge stehen

Telemetrie-Runde  (eigener Backoff, eigener Oeffnungslauf, eigene Bootstrap-E/A)
  +  8.000 ms   Backoff bis zum naechsten Versuch   (Deckel kBackoffMaxMs)
  +  4.000 ms   WaitNamedPipe-Schleife, 20 x 200 ms
  +  5.000 ms   Telemetry-Hello schreiben           (kIoFristMs)
  +  5.000 ms   Telemetry-Welcome lesen             (kIoFristMs)
  = 22.000 ms

Descriptor
  +  1.000 ms   eine Heartbeat-Kadenz, bis der Descriptor beim Coordinator steht

  = 45.051 ms   Summe je Clientpaar
```

**Was diese Summe ist — und was nicht.** Sie ist eine obere Schranke für den
Weg **eines** Clientpaares. Sie ist **keine** Zusage für alle Paare
gleichzeitig: weder `IpcVerbindung::oeffnen` (`core/ipc/IpcVerbindung.cpp:67-89`)
noch die Annahmeschlange des Listeners geben eine Fairnessgarantie, also kann ein
einzelnes Paar Rennen wiederholt verlieren, während andere durchkommen. Sie
deckt ausserdem ausschliesslich die im Code bezifferten Fristen ab und sagt
nichts über Scheduling, Plattenlast oder Virenscanner.

> **Festgelegte Frist Z2: 60 s ab „Broker horcht wieder", je Clientpaar.**

Die 60 s liegen über der gerechneten Schranke von 45,1 s. Der Abstand ist
bewusst und wird hier benannt statt versteckt: er trägt die fehlende
Fairnessgarantie und den Plattform-Jitter, damit die Frist eine Aussage über
das Produkt bleibt und nicht an OS-Rauschen kippt. Die Frist ist damit
**hergeleitet plus benannter Zuschlag** — sie folgt nicht aus der Summe allein.

**Wie ein Fehlschlag gelesen wird.** Der Bericht trägt je Neustart die
Verteilung der Reconnect-Dauer über alle Clientpaare (`min`, `p95`, `max`,
§7). Daraus folgt die Einordnung ohne Ermessen:

| Messung | Lesart |
|---|---|
| `max` ≤ 45.051 ms | Schranke hält; alles innerhalb der gerechneten Konstanten. |
| 45.051 ms < `max` ≤ 60.000 ms | Frist gehalten, Schranke überschritten — der benannte Zuschlag hat getragen. Der Lauf ist grün, die Überschreitung steht als Zeile im Manifest. |
| `max` > 60.000 ms | ROT. Im Manifest steht, **welches** von beidem bricht: existiert ein Weg, den die Summe nicht abdeckt, ist die **Ableitung** unvollständig und wird ergänzt; deckt die Summe ihn ab, ist es ein **Produktbefund**. |

Die Frist wird in keinem Fall stillschweigend erhöht.

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
   seiner `BEREIT`-Zeile läuft die 60-s-Uhr aus §3.1.
5. Reihenfolge wie §4.1/§4.2, unverändert: der Main subscribt neu, die Sonden
   reporten neu. Kein neuer Produktpfad — genau die bestehenden Client-Wege.

## 5. Verhaltensmatrix

Spalte „Zusage" nennt die Nummer aus dem Auftrag (Z1–Z8). Spalte „Test" nennt
das Bein, das **genau diese Zeile** misst; `A24` bezeichnet den neuen
Prüfer/Client, `A24:<name>` einen benannten Prüfpunkt darin.

| ID | Ausgangszustand × Ereignis | Zusage, Callback-Reihenfolge und Frist | Z | Test, der genau diese Zeile misst | Quelle |
|---|---|---|---|---|---|
| S01 | Kalt, nichts verbunden × Lauf startet | Reihenfolge §4.1 und §4.2. Innerhalb der Frist aus §3.1 (**60 s**) stehen 1 Main-Paar und N Sondenpaare, und der Main hat einen absoluten Snapshot erhalten. Wird die Frist verfehlt, endet der Lauf mit Exit 1 und einer Diagnosezeile (Muster `Sonde012SourcesLatencyTest.cpp:225-240`), nie mit „grün ohne Topologie". | Z1 | **NEU** `A24:topologie_steht_in_frist` | §3.1; `Sonde012SourcesLatencyTest.cpp:206-240`; `IpcLastMain.cpp:167-195` |
| S02 | Topologie steht × Warmup abgelaufen (Gate 2 min, Kurzform 20 s) | Der Snapshot beim Main zeigt **genau N** Mitglieder; die Menge der `instanceId` ist gleich der erwarteten Menge; `fuehrendesMain` ist die Main-Adresse; **keine** fremde Adresse taucht auf. Geprüft auf der Sicht des echten `SourcesModel`, nicht auf dem Rohtext. | Z1 | **NEU** `A24:mitgliedschaft_nach_warmup` | `src/SourcesModel.h:75-84`; Entwurf `:4262-4264`; SONDE-012 L03 |
| S03 | Eingeschwungen × Dauerbetrieb bis Laufende: je Sonde 10-Hz-Liveframes (2.048 Samples), 1-Hz-Grossfenster (16.384 Samples) und 1-Hz-Heartbeats | Die Mitgliedschaft driftet über die ganze Laufzeit nicht: Zeilenzahl bleibt N, die `instanceId`-Menge bleibt identisch, `fuehrendesMain` bleibt dieselbe Adresse. Jede Snapshot-Übernahme wird geprüft, nicht nur die letzte. Ein einziger abweichender Snapshot ausserhalb der Neustartfenster ist ROT. | Z1 | **NEU** `A24:mitgliedschaft_driftet_nicht` | Entwurf `:4262-4264`; `:3586` („ohne ungegrenztes Wachstum") |
| S04 | Eingeschwungen × P0-Heartbeat je Sonde, 1.000 ms Kadenz | Reihenfolge: Sendezeit vor `sendeP0` stempeln; wird `sendeP0` abgewiesen, gilt der Heartbeat als **nie eingereiht** und wird nicht erwartet (Muster `IpcLastMain.cpp:229-235`). Ausserhalb der Neustartfenster geht **kein** ACK verloren; p95 der ACK-Latenz **< 1.000 ms**, auch während die langsamen Leser bremsen. | Z4 | **NEU** `A24:p0_kein_verlust_und_p95` | `IpcLastMain.cpp:42,216-240`; `coordinator.rs:28` |
| S05 | Eingeschwungen × ein Viertel der Sonden liest künstlich verzögert | Die künstliche Verzögerung wird **nach** dem Zeitstempel des Empfangs eingelegt, nie davor. Sonst mässe das Bein seine eigene Bremse statt der Drahtlatenz — die Zusage wäre unfalsifizierbar. Die langsamen Sonden bleiben über den ganzen Lauf Mitglied und werden ausserhalb der Neustartfenster nie stale. | Z6, Z5 | **NEU** `A24:langsame_bleiben_mitglied` | Prüfliste Abschnitt A; `coordinator.rs:29-31` |
| S06 | Langsamer Leser × seine Telemetrieschleuse läuft voll | Rückstau heisst „der älteste weicht": `ersetzt > 0`, `beanspruchtVerworfen == 0`. Jede abgelehnte Veröffentlichung hat einen gezählten Grund (`abgelehnt == zuGross + beanspruchtVerworfen`). Kein langsamer Leser verzögert die schnellen: deren p95 aus S04 hält unabhängig. | Z6 | **NEU** `A24:cap_ersetzt_aeltesten_und_blockiert_nie` | `core/ipc/TelemetryClient.cpp:355-364`; `IpcLastMain.cpp:311-330`; Prüfliste A |
| S07 | Eingeschwungen × Minutentick | RSS beider Prozesse wird minütlich gelesen und als vollständige Kurve berichtet. Nach dem Warmup wächst keiner bis zum Ende seiner Generation um mehr als **max(10 %, 16 MiB)**. Brokerkurve je Generation, Clientkurve über den ganzen Lauf (§3.2). | Z3 | **NEU** `A24:speicherkurve_im_budget` | Entwurf `:3586,3588-3590`; §3.2 |
| S08 | Eingeschwungen × N `SondeProcessor` treiben Ganzblöcke in Echtzeitkadenz | 0 Ganzblockdrops durch Überlauf, 0 durch Oversize; 0 Kontinuitätsbrüche ausserhalb bewusst erzeugter Transportkanten. Kontinuität wird über `transport.continuity_segment` des zuletzt veröffentlichten Producer-Frames beobachtet (`letzterProducerFrameFuerTest`), weil `SondeProcessor` selbst keinen Bruchzähler veröffentlicht — der liegt nur auf `PluginProcessor` (`src/PluginProcessor.h:146`). Grenze §2.2 gilt. | Z7 | **NEU** `A24:audio_ganzblock_ohne_drop` | `sonde/SondeProcessor.h:205-217`; `core/analysis/FeatureEngine.h:140-144`; B4 `QueueStressTestMain.cpp:504-556`; B12 |
| S09 | Eingeschwungen × Broker-Prozess wird zum festen Zeitpunkt getötet | Reihenfolge §4.3 Schritte 1–3. Der Main verlässt `subscriptionAktiv`; das Modell zeigt `disconnected`, nicht die letzte grüne Sicht. Die Zeitmessung des Neustartfensters beginnt beim Kill, die Fristmessung erst bei `BEREIT`. | Z2 | **NEU** `A24:kill_beendet_subscription_sichtbar` | §4.3; SONDE-012 L10; `src/SourcesModel.h:99-108` |
| S10 | Broker tot × Clients im Backoff | Kein Absturz, keine Endlosschleife, **kein anderer Pipename**. Die Clients verdoppeln ihren Backoff bis 8.000 ms und versuchen ausschliesslich denselben Probe-Namen. Der Zeitraum wird als Neustartfenster markiert; Zusagen Z4/Z5 sind darin ausgesetzt und werden getrennt gezählt, nicht stillschweigend ausgelassen. | Z2, Z8 | **NEU** `A24:backoff_ohne_namenswechsel` | `core/ipc/IpcVerbindung.cpp:67-99`; `core/ipc/ControlClient.cpp:1014-1025` |
| S11 | Neuer Broker horcht auf demselben Namen × Clients reconnecten | Innerhalb der Frist aus §3.1 (**60 s ab `BEREIT`**) je Clientpaar: der Main subscribt neu (Reihenfolge §4.1), alle N Sonden reporten neu, und der Snapshot ist wieder vollständig (N Mitglieder, erwartete Adressen, `fuehrendesMain` = Main-Adresse). Der Bericht trägt je Neustart die Verteilung der Reconnect-Dauer (`min`, `p95`, `max`); die Lesart eines Fehlschlags steht in §3.1. Ein Reconnect nutzt ausschliesslich die bestehenden Client-Pfade; A24 fügt keinen Produktpfad hinzu. | Z2, Z1 | **NEU** `A24:reconnect_vollstaendig_in_frist` | §3.1; §4.3; Entwurf `:4262-4264` |
| S12 | Nach dem Neustart × jeder weitere Snapshot | Die `broker_epoch` ist eine **neue**; die alte wird nach dem Neustart nie wieder als aktueller Lauf ausgespielt. Der Zähler dafür muss strukturell 0 bleiben und ist eine Wache mit Test (Prüfliste A). Keine fremde Steueradresse erscheint. | Z2 | **NEU** `A24:alte_epoche_kommt_nie_wieder`; Gegenstueck im Unittest bestehend in `broker/tests/sonde012_sources_slice.rs`, Test `restart_has_no_old_live_or_measurement_truth` (dort Zeile 696) | SONDE-012 L12 (`docs/beweise/SONDE-012.md:152`); `broker/src/bin/eqcop-broker-sonde012-probe.rs:34` |
| S13 | Beliebig × Liveness-Tick | Ausserhalb der Neustartfenster wird **kein** Mitglied stale (Grenze > 2.500 ms) und keines evicted (10.000 ms). Innerhalb der Fenster ist beides erlaubt und wird getrennt gezählt und berichtet. Die Heartbeat-Kadenz von 1.000 ms hält den Abstand zur Stale-Grenze; wird sie im Lauf nicht erreicht, ist das der Befund. | Z5 | **NEU** `A24:kein_stale_ausserhalb_neustart`; bestehend in `broker/tests/coordinator_model.rs` die Tests `stale_konstanten_ableitung` (Zeile 527) und `stale_grenze_exklusiv_und_kontakt_setzt_zurueck` (Zeile 535) | `broker/src/coordinator.rs:28-32`; SONDE-012 L07 |
| S14 | Lauf läuft × Laufende erreicht | Gegenpfad zu S01: Nachlauffenster für ausstehende ACKs, dann Telemetrie **vor** Control stoppen (Muster `IpcLastMain.cpp:280`), Audiotreiber-Threads beenden und joinen, `SondeProcessor` zerstören, danach dem Broker `STOP` schicken und auf ihn warten. Kein hängender Thread, kein verwaister Prozess; das Bein endet mit einer Urteilszeile und einem Exitcode, nie mit Stille. | — (Änderungssatz: verbinden↔trennen, starten↔stoppen) | **NEU** `A24:sauberer_abbau` | Prüfliste Abschnitt B und F; `IpcLastMain.cpp:280`; `sonde/SondeProcessor.cpp:107-115` |
| S15 | Beliebig × Programm bekommt einen Nicht-Probe-Pipenamen | Beide Programme — Prüfer-Client und Probe-Broker — verweigern den Dienst mit **Exit 3** für Produktions- und Golden-Pipename. Der Riegel wird **gefahren**, nicht behauptet: der Prüfer startet beide Binaries vor dem eigentlichen Lauf einmal mit jedem verbotenen Namen und verlangt Exit 3 (Muster A23 `pruefe_sonde012_sources_latency.py:40-48`). Es ist eine Erlaubnisliste, keine Sperrliste. | Z8 | **NEU** `A24:verweigert_produktion_und_golden` | `broker/src/transport/pipetoken.rs:28-45`; `pruefe_ipc_last.py:75-89`; `IpcLastMain.cpp:74-82` |
| S16 | Beliebiges N × der Gate-Lauf fährt die Skalierungsreihe | Das Bein ist über `--sonden` parametrisch; **alle** Zusagen Z1–Z8 gelten je Lauf identisch, und der Bericht trägt `sonden`. Die Kanon-Kurzform bleibt 16 Sonden, 2 Minuten, 1 Neustart. Der Gate-Lauf G3 fährt zusätzlich zu 16 × 60 min und 32 × 30 min drei kurze Läufe mit **1, 4 und 8** Sonden (je 2 min, 1 Neustart) und deckt damit die im Entwurf verlangte Reihe 1/4/8/16/32 vollständig ab. `--langsam` erzeugt `max(1, round(N · Anteil))` langsame Leser — bei **N = 1 ist die einzige Sonde langsam**; der Main ist **nie** langsam. Bei N = 1 hat der Teilsatz „blockiert die anderen nicht" aus S06 keine Gegenpartei und wird als `nicht_anwendbar` berichtet, nie als bestanden. | Z1–Z8 | **NEU** `A24:skalierungsreihe_1_4_8_16_32` — je Lauf laufen dieselben Prüfpunkte S01–S15 | Entwurf `:4523` (`session_soak`: „1/4/8/16/32 Clients mit langsamen Lesern und Brokerneustart"); `broker/src/coordinator.rs:36-37` |

### 5.1 Was in dieser Matrix bewusst nicht steht

- **Kein XRun.** Siehe §2.2. Die Zeile S08 sagt „Ganzblockdrops", nicht „XRun".
- **Die Skalierungsreihe ist Gate-Lauf, nicht Kanon-Kurzform.** S16 legt die
  fünf Läufe 1/4/8/16/32 fest (Entwurf `:4523`, `:3586`); die Kanon-Kurzform
  fährt 16 Sonden über 2 Minuten. Die Gate-Läufe werden im G3-Gate-Manifest
  belegt, nicht hier vorweggenommen.
- **Keine Aussage über den produktiven Broker.** Alles läuft im Probe-Namensraum.

## 6. Killmatrix — der Brokerneustart

Der Neustart ist ein **echter Prozesstod**, kein zurückgegebener Fehler
(Präzedenz: `broker/tests/store_crash_matrix.rs`, `docs/beweise/SONDE-011.md:584-590`).
Der Prüfer beendet den Brokerprozess hart und startet ihn auf demselben
Probe-Namen neu.

**Kein Killpunkt heisst „deterministisch", solange keine gemessene Barriere ihn
hält.** Die erste Fassung dieses Abschnitts nannte K-S1 bis K-S5 „deterministisch
by Konstruktion", obwohl ein Kill auf fester Wanduhr weder einen laufenden
Schreibvorgang noch ein ausstehendes ACK, ein enges Subscribe-Fenster oder einen
erreichten Backoff-Deckel garantiert. Das ist hier berichtigt: jede Zeile trägt
genau **eine** von zwei Formen.

- **Barriere (gemessen).** Das C++-Programm meldet den erreichten Zustand mit
  einer eigenen stdout-Zeile; der Prüfer killt **erst danach**. Der Bericht
  trägt zusätzlich, ob der Zustand zum Killzeitpunkt noch stand.
- **Wahrscheinlich (beobachtet).** Der Kill liegt auf einem festen Zeitpunkt.
  Ob er den Punkt getroffen hat, entscheidet **nach** dem Lauf ein benanntes
  Berichtsfeld — nicht die Absicht.

In beiden Formen gilt: **ein nicht getroffener Punkt zählt nie als bestanden.**
Der Bericht führt je Zeile `getroffen`, `nicht_getroffen` oder `nicht_gefahren`
samt dem Beleg, aus dem das Urteil folgt (§7).

| ID | Killpunkt | Erwartung nach dem Neustart | Wie dieser Punkt getroffen wird | Test |
|---|---|---|---|---|
| K-S1 | Broker stirbt, **während eine Sonde P2 auf ihrem offenen Telemetrie-Handle schreibt** | Ein halber Frame resynchronisiert nicht, sondern beendet den Strom (bestehende Envelope-Regel). Nach dem Neustart ist der erste Zustand am Main `disconnected`, danach wieder vollständig innerhalb der Frist aus §3.1. Kein halber Frame wird je als Messwert sichtbar. | **Wahrscheinlich (beobachtet), mit gemessener Vorbedingung.** Das Signal ist telemetriespezifisch: gewertet wird ausschliesslich der `TelemetryClient` einer Sonde, **nie** deren `ControlClient` — ein Control-Fehler belegt Controlverkehr, nicht den P2-Frame. Feld: `k_s1.telemetrie_handle_fehler`, **getroffen** nur wenn alle vier Punkte gelten: (1) die Sonde war unmittelbar vor dem Kill `TelemetryClient::Status::verbunden` (`core/ipc/TelemetryClient.h:65`) — das Programm hält den Status je Sonde vor dem Kill fest; (2) ihre Schleuse war im letzten Beobachtungsfenster **nachweislich im Rückstau** (`ersetzt` ist gestiegen), sodass der verbundene Lauf im Schreibzweig steht und nicht im Leerlauf-Lesezweig (`core/ipc/TelemetryClient.cpp:736-762`); (3) nach dem Kill ist `letzterFehler` gesetzt — dieses Feld wird beim Eintritt in `verbunden` geleert (`core/ipc/TelemetryClient.cpp:700-704`), jeder spätere Eintrag stammt also **per Konstruktion vom offenen Handle**; (4) `verbindungsVersuche` ist dabei unverändert, der Fehler kommt also nicht von einem neuen Verbindungsversuch. Die fünf vertragsinternen Texte (`P2-Frame zu gross`, `Kopplung gewechselt …`, `Envelope abgelehnt …`, `P0/P1 auf der Telemetrieverbindung …`, `Nachrichtenratengrenze …`) sind ausgeschlossen; sie sind keine Drahtfehler. **Grenze, die bleibt:** Schreibzweig (`:750-757`) und Leerlauf-Lesezweig (`:453-461`) schreiben bei gebrochener Pipe **denselben** Win32-Text in dasselbe Feld, und kein Zähler markiert einen gescheiterten Write (`gesendet` zählt nur Erfolge). Punkt (2) macht den Lesezweig unwahrscheinlich, schliesst ihn aber nicht aus. K-S1 behauptet deshalb „Fehler auf dem offenen P2-Handle einer verbundenen, im Rückstau schreibenden Sonde" — **nicht** „Frame nachweislich mitten im Schreiben". Fehlt einer der vier Punkte, ist K-S1 `nicht_getroffen`, nie bestanden. | **NEU** `A24:kill_im_frameverkehr`; bestehende Envelope-Hälfte `broker/tests/transport_fuzz.rs` (ein kaputter Frame beendet den Strom) |
| K-S2 | Broker stirbt **während `subscribe_session`** — nach Verbindung des Main, bevor der absolute Snapshot beim Main ankommt | Der Main hält keinen halb offenen Subscription-Zustand: `controlEnde()` läuft, `subscriptionAktiv` ist false, das Modell zeigt `disconnected`. Der nächste erfolgreiche Reconnect subscribt erneut und bekommt einen **absoluten** Snapshot; es gibt kein `unsubscribe_session` und keinen Event-Replay. | **Barriere (gemessen), ab `--neustarts >= 2`.** Ein Kill nach der `BEREIT`-Zeile beobachtet dieses Fenster nicht — die Subscription entsteht erst im Verbunden-Callback (`eq-copilot/plugin/tests/Sonde012SourcesLatencyTest.cpp:131-140`). Das C++-Programm schreibt deshalb `MAIN_SUBSCRIBE_GESENDET`, sobald `sendeP1("subscribe_session", …)` eingereiht ist **und noch kein Snapshot angekommen ist**; der Prüfer killt unmittelbar nach dieser Zeile. Der Bericht trägt `k_s2.snapshot_vor_kill`: kam der Snapshot dennoch vor dem Kill an, ist K-S2 `nicht_getroffen` — nie bestanden. Bei `--neustarts 1` (Kanon-Kurzform) `nicht_gefahren`. | **NEU** `A24:kill_waehrend_subscribe` |
| K-S3 | Broker stirbt **während eines Heartbeats** — P0 ist gesendet, das `heartbeat_ack` kommt nicht mehr | Der verlorene Heartbeat wird als `p0.verloren_im_neustartfenster` gezählt und **nicht** als Verletzung von Z4 gewertet; Z4 gilt ausserhalb der Fenster. Nach dem Reconnect laufen Heartbeats derselben Sonde weiter; die Sequenznummern springen nicht rückwärts und kein ACK wird doppelt verbucht. | **Wahrscheinlich (beobachtet).** N Sonden senden mit 1.000 ms Kadenz, aber ein Kill auf fester Wanduhr garantiert kein ausstehendes ACK. Berichtsfeld `k_s3.p0_ohne_ack_im_fenster`: **getroffen** genau dann, wenn `p0.verloren_im_neustartfenster > 0`. Ist der Zähler 0, ist K-S3 `nicht_getroffen` — nicht bestanden. | **NEU** `A24:kill_waehrend_heartbeat` |
| K-S4 | Broker stirbt, **während ein langsamer Leser in seiner künstlichen Verzögerung steckt** | Der langsame Leser blockiert weder seinen eigenen Reconnect noch den der anderen. Er ist innerhalb derselben Frist aus §3.1 wieder Mitglied. Seine Schleuse verwirft dabei alte Liveframes (Cap), verliert aber nie den neuesten. | **Barriere (gemessen).** Ein dauerhaft eingeschalteter langsamer Modus beweist nicht, dass der Kill in einen blockierten Aufruf fällt. Jede langsame Sonde führt deshalb ein **atomares Flag** „steckt gerade in der künstlichen Verzögerung"; sobald es gesetzt ist, schreibt das Programm die Zeile `LANGSAM_IN_VERZOEGERUNG`, und der Prüfer killt unmittelbar danach. Der Bericht trägt `k_s4.flag_zum_killzeitpunkt`; war das Flag beim Kill nicht mehr gesetzt, ist K-S4 `nicht_getroffen`. | **NEU** `A24:kill_mit_langsamem_leser` |
| K-S5 | Broker bleibt nach dem Kill **mindestens 20 s tot**, sodass jeder Control-Client den Backoff-Deckel (8.000 ms) erreicht hat | Auch aus dem Deckel heraus verbinden alle Clientpaare innerhalb der Frist aus §3.1 ab der letzten `BEREIT`-Zeile — genau dafür trägt die Schranke in §3.1 den Deckel zweimal. Kein Client bleibt dauerhaft hängen. | **Barriere (gemessen) durch Totzeit, ab `--neustarts >= 2`.** Ein zweiter Kill nach `BEREIT` erzeugt keinen Deckel; der entsteht erst durch wiederholte Fehlversuche (`core/ipc/ControlClient.cpp:1014-1025`). Die Backoff-Folge eines Control-Clients ist 500 + 1.000 + 2.000 + 4.000 + 8.000 = 15.500 ms bis zum Deckel (`core/ipc/IpcVerbindung.h:34-35`); eine Totzeit von ≥ 20 s liegt darüber. Gemessen wird nicht die private Backoff-Variable, sondern ihr Beleg: `ControlClient::Snapshot::verbindungsVersuche` (`core/ipc/ControlClient.h:176`, erhöht bei **jedem** Versuch, `core/ipc/ControlClient.cpp:1048`) muss in der Totzeit je Control-Client um **≥ 5** steigen. `k_s5.backoff_deckel_erreicht` ist die Zahl der Control-Clients, die das erfüllen; sie muss **N + 1** sein (N Sonden + Main), sonst `nicht_getroffen`. Die Telemetriethreads werden **nicht** mitgezählt: ohne Kopplung nehmen sie den Warte-Zweig, der `backoffMs` nicht verdoppelt (`core/ipc/TelemetryClient.cpp:383-398`). | **NEU** `A24:kill_aus_backoff_deckel` |

## 7. Bericht und Zählerkarte

Der Prüfer schreibt **eine** JSON-Zeile mit allen Zählern und der minütlichen
Speicherkurve auf stdout; sie geht roh ins Manifest. Kein Zähler wird
weggelassen, weil er 0 ist — ein Zähler, der strukturell 0 sein muss, ist eine
Wache mit Test (Prüfliste A).

Feste Felder (Namen sind Vertrag zwischen C++-Programm und Prüfer, kein
Wire-Vertrag):

```
sonden, minuten, neustarts, langsam_anzahl, warmup_s, clientpaare
mitgliedschaft: snapshot_pruefungen, vollstaendig, fremde_adresse,
                fuehrendes_main_falsch
neustart[]:     index, epoch_alt, epoch_neu, frist_ms,
                reconnect_ms{min, p95, max},          # je Clientpaar, §3.1
                bereit_bis_vollstaendig_ms,
                schranke_ms = 45051, ueber_schranke,  # Lesart-Tabelle §3.1
                alte_epoche_nach_neustart_gesehen,
                totzeit_ms                            # nur beim K-S5-Neustart
p0:             gesendet, beantwortet, verloren_ausserhalb_neustart,
                verloren_im_neustartfenster, latenz_p95_ms, latenz_max_ms,
                schranke_ms
liveness:       stale_ausserhalb_neustart, evicted_ausserhalb_neustart,
                stale_im_neustartfenster
langsam:        sonden, immer_mitglied, ersetzte_liveframes,
                neueste_verworfen, abgelehnt, zu_gross, schnelle_p95_ms,
                blockiert_andere_nicht: true | false | nicht_anwendbar
                                                      # nicht_anwendbar bei N = 1
audio:          bloecke, ganzblockdrops_ueberlauf, ganzblockdrops_oversize,
                kontinuitaetsbrueche, publikationen
speicher:       takt_s, budget_prozent, budget_bytes,
                client[]{minute, rss_bytes, im_neustartfenster},
                broker[]{generation, minute, rss_bytes, im_neustartfenster,
                         urteil}
kill:           je Killmatrixzeile { urteil: getroffen | nicht_getroffen |
                nicht_gefahren, beleg }, dazu die Belegfelder
                k_s1.telemetrie_handle_fehler, k_s2.snapshot_vor_kill,
                k_s3.p0_ohne_ack_im_fenster, k_s4.flag_zum_killzeitpunkt,
                k_s5.backoff_deckel_erreicht
```

`kill.*.urteil` folgt allein aus dem zugehörigen Belegfeld (§6), nie aus der
Absicht des Laufs. `nicht_getroffen` und `nicht_gefahren` sind kein Erfolg: das
Gate-Manifest führt sie als offene Killpunkte, bis ein Lauf sie trifft.

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

| Parameter | Standard | Kanon-Kurzform (A24) | Gate-Lauf G3 (fünf Läufe, S16) |
|---|---|---|---|
| `--sonden` | 16 | 16 | **1 · 4 · 8 · 16 · 32** |
| `--minuten` | — | 2 | 2 · 2 · 2 · 60 · 30 |
| `--neustarts` | 1 | 1 | 1 · 1 · 1 · 3 · 3 |
| `--langsam` | 0.25 | 0.25 | 0.25 in allen fünf Läufen |
| langsame Leser = `max(1, round(N · Anteil))` | — | 4 | **1 · 1 · 2 · 4 · 8** |
| Warmup | — | 20 s | 20 s · 20 s · 20 s · 2 min · 2 min |

Der Main ist **nie** langsam. Bei N = 1 ist die einzige Sonde langsam
(`max(1, round(0,25)) = 1`); der Teilsatz „blockiert die anderen nicht" aus S06
hat dort keine Gegenpartei und wird als `nicht_anwendbar` berichtet (S16).

Neustart *k* liegt bei Bruchteil *k/(neustarts+1)* des Messfensters. Die
Barrieren-Kills K-S2 und K-S5 hängen an Neustart 2 und existieren erst ab
`--neustarts >= 2`; in der Kanon-Kurzform sind sie `nicht_gefahren`.

**Messfenster und Neustartfenster.** `--minuten` zählt **beobachtete** Messzeit.
Ein Neustartfenster — vom Kill bis zum wieder vollständigen Snapshot — hält die
Messuhr an; der Lauf verlängert sich um dessen Dauer. Ohne diese Regel bliebe
bei der Kurzform (2 min, Neustart bei 50 %, Frist 60 s) nach dem Neustart keine
Beobachtungszeit übrig, und S03 wie S07 hätten danach nichts mehr zu messen.
Speicherproben, die in ein Neustartfenster fallen, tragen
`im_neustartfenster: true` und gehen nicht in das Wachstumsurteil (§3.2).

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
| A — höhere Klasse wird gelesen, solange niedrigere blockiert | S05 + S06: die langsamen Leser bremsen P2, während S04 die P0-p95 der schnellen Sonden misst. Bei N = 1 (S16) gibt es keine schnelle Gegenpartei; die Zeile wird dort als `nicht_anwendbar` berichtet, nie als bestanden |
| B — Reihenfolge beim Verbinden festgelegt und gemessen | §4.1/§4.2, gemessen in S01 und S11 |
| B — beim Trennen zuerst abmelden, dann Fristen und Joins | S14: Telemetrie vor Control stoppen, Audiothreads joinen, dann Broker `STOP` |
| B — jeder Join hat eine Frist | S01 (60 s), S11 (60 s ab `BEREIT`, je Clientpaar), S14 (Nachlauffenster und Join-Fristen); die Frist selbst ist in §3.1 aus Control-Runde, Kopplung, Telemetrie-Runde und Heartbeat-Kadenz gerechnet |
| B — Reihenfolge beim Verbinden gilt auch für die zweite Verbindung | §3.1 und §4.1/§4.2: die Telemetrie-Runde ist kein Anhängsel der Control-Runde, sondern eine eigene Runde mit eigenem Backoff und eigenen Bootstrap-E/A-Fristen; S01 und S11 messen erst das **vollständige** Clientpaar |
| C — Verträge und Längen | **nicht Gegenstand von A24.** Das Bein berührt keinen Vertrag; die Vertragsriegel bleiben bei A5/A9/B3c/A21 |
| D — ein Bein prüft die Frische seiner Eingaben und meldet Exit 3 | Prüfer prüft die Existenz beider Binaries vor dem Lauf (Muster `pruefe_ipc_last.py:63-72`); die Frische erzwingt der Runner über `$gemesseneZiele` (§9) |
| D — was der Kanon nicht baut, darf er nicht als frisch bezeugen | §9: `EqCopSessionSoak` steht in `$gemesseneZiele` |
| E — Behauptung ≤ Messung | §2.2 (XRun-Grenze steht ausdrücklich drin), §5.1 (was die Matrix nicht sagt), §3.1 (Schranke 45,1 s und Frist 60 s sind getrennt benannt, der Zuschlag ist ausgewiesen statt in die Rechnung gemogelt), §6 (jede Killzeile trägt **Barriere** oder **wahrscheinlich**, nie „deterministisch" ohne Beleg), K-S1 bis K-S5 (`nicht_getroffen`/`nicht_gefahren` statt stiller Erfolg) |
| E — Zahlen im Manifest sind gemessen, nicht abgeschrieben | §3 (jede Konstante mit `Datei:Zeile`), §7 (jeder Zähler kommt aus dem Lauf); K-S5 misst den Backoff-Deckel nicht an der privaten Variablen, sondern an `verbindungsVersuche` — ein öffentlicher Zähler mit belegter Erhöhungsstelle |
| E — jede neue Prüfung wurde einmal absichtlich gebrochen | Phase 2: je Zusage Z1–Z8 ein absichtlich gebrochener Lauf mit Rohausgabe des Rots im Rundenabschnitt dieses Manifests |
| E — eine Zeile, die nicht getroffen wurde, gilt nie als bestanden | §6 (Vorwort und alle fünf Killzeilen), §7 (`kill.*.urteil` folgt allein aus dem Belegfeld), S16 (`nicht_anwendbar` bei N = 1) |
| F — verbinden↔trennen, starten↔stoppen im selben Änderungssatz | S01↔S14 und S09↔S11 sind in derselben Datei und demselben Commit |

## 11. Offene und in Phase 1b geschlossene Punkte

1. **[geschlossen in Phase 1b] Skalierungsreihe 1/4/8/16/32** (Entwurf `:4523`; 32er-Laufzeit `:3586`):
   seit Phase 1b mit **S16** vollständig festgelegt — fünf Gate-Läufe, in jedem
   gelten dieselben Zusagen Z1–Z8, der Bericht trägt `sonden`, und `--langsam`
   erzeugt auch beim 1er-Lauf einen langsamen Leser. Die Reihe ist **nicht** Teil
   der Kanon-Kurzform (die bleibt 16 × 2 min × 1 Neustart) und wird im
   G3-Gate-Manifest belegt, nicht hier. Kein Produktentscheid nötig; damit ist
   der Punkt geschlossen und steht hier nur noch als Verlauf.
2. **[offen] XRun** bleibt FL-Termin (§2.2). Kein Produktentscheid nötig — eine
   Konsolenbinärdatei kann es nicht messen.
3. **[offen: keiner] Kein Produktentscheid.** Dieses Ticket ist reines Prüfwerkzeug;
   es ändert kein sichtbares Produktverhalten und legt dem User nichts vor.

## 12. Verlauf

| Datum | Schritt | Stand |
|---|---|---|
| 2026-09-02 | Phase 1: Verhaltensmatrix, Killmatrix, Fristen aus dem Code abgeleitet; kein Code geschrieben | Basis-Commit `03e1e17`, Stand `d60ceaf` |
| 2026-09-02 | Matrixprüfung Runde 1: Codex Thread `01a061ed-25ea-7242-a384-b936d10c62de` (gpt-5.6-sol, Effort high, read-only), **NEEDS_WORK**, vier Befunde, Nacharbeit Phase 1b | geprüft auf `f7a5125` |
| 2026-09-02 | Matrixprüfung Runde 2: frischer Codex Thread `01a06203-877a-7fe1-aa47-41fe219de8a2` (high, read-only, Fixdiff `d60ceaf`…`a570367`): S16, Fristableitung, K-S2 und K-S5 **geschlossen**; ein Defekt bleibt — K-S1 wertete Control-Felder und belegte damit Controlverkehr statt eines P2-Frames | geprüft auf `a570367` |
| 2026-09-02 | Phase 2, Schritt 1: K-S1 auf ein telemetriespezifisches Eintrittssignal umgestellt (`k_s1.telemetrie_handle_fehler`, vier Bedingungen, verbleibende Grenze benannt) | Basis `a570367` |
| 2026-09-02 | Phase 1b: alle vier Befunde geschlossen — §3.1 rechnet jetzt Control- **und** Telemetrie-Runde und weist den Zuschlag zur 60-s-Frist aus (B2); §6 ersetzt „deterministisch" durch **Barriere** oder **wahrscheinlich** mit Belegfeld (B3, B4); S16 legt die Reihe 1/4/8/16/32 und die Regel `max(1, round(N · Anteil))` fest (B1). Kein Code geschrieben | Basis `f7a5125` |
