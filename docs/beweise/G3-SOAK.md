# G3-SOAK — Werkzeugticket: das 60-Minuten-Dauerlaufbein A24

<!-- NAKAMA-URTEIL: T2 PASS 2026-09-02 -->

| Merkmal | Wert |
|---|---|
| Ticket | G3-SOAK (Werkzeugticket zum Phasengate **G3**) |
| Basis-Commit | Phase 1 `03e1e17` · Phase 1b `f7a5125` · Phase 2 `a570367` · Nacharbeit Runde 1 `4ae3ee6` · **Nacharbeit Runde 2 `882b2aa`**. Die Phase-2-Commits wurden vor dem Push auf `8cd5903` rebased; §12 nennt zu jedem Schritt beide SHAs. |
| Phase | **Abgeschlossen — T2 PASS 2026-09-02 (Dirigent), Stand `d52fd15`.** Phase 1/1b spezifizierten vor dem Code (zwei Codex-Matrixrunden); Phase 2 baute A24 und fuhr es; die Abschlussprüfung (Codex, max) fand 17 Defekte mit einer Ursache; Nacharbeit Runde 1 schloss 16, Runde 2 den Rest, Wiederprüfung 2 PASS. Abschnitt 17 trägt Urteil, Runden, Bilanz und Kanon. Die Stufe T3 gehört dem Gate G3, dessen Beweisbein dieses Werkzeug ist. |
| Gate-Text (wörtlich, `docs/bauaufteilung-sonden.md:385`) | „**Gate:** `/rust-review` + Codex + 60-min-Soak. Falsifikation: Gate 7" · Prüfstufe T3 |
| Auftrag | genau ein neues Kanon-Bein **A24**. In Phase 2 **angelegt**: der Prüfer `tools/eq-copilot/pruefe_session_soak.py` und das Konsolenprogramm `eq-copilot/plugin/tests/SessionSoakMain.cpp` (CMake-Ziel `EqCopSessionSoak`), dazu die Registrierung A24 in `tools/beweise.ps1` samt Bauziel in `$gemesseneZiele`. Der Probe-Broker `broker/src/bin/eqcop-broker-sonde012-probe.rs` blieb **unverändert** — er kann bereits beides, was A24 braucht: `BEREIT` melden und auf `STOP` enden. Sonst nichts. |
| Bindende Quellen | Entwurf §49.3 Budget `docs/FL-Nakama-Sonden-Design-Entwurf.md:3586`; P3-Exit-Gate `:4262-4264`; Testfamilie `session_soak` `:4523`; Prüflistenabschnitt A `tools/dirigent/pruefliste.md` |
| Vorbilder (Bau) | A22 `tools/eq-copilot/pruefe_ipc_last.py` + `eq-copilot/plugin/tests/IpcLastMain.cpp`; A23 `tools/eq-copilot/pruefe_sonde012_sources_latency.py` + `eq-copilot/plugin/tests/Sonde012SourcesLatencyTest.cpp`; B4 `tests/QueueStressTestMain.cpp`; B12 `tests/Sonde012LoudnessSourceTest.cpp` |
| Ticketgrenze | kein Wire-, State- oder Testvertrag; keine Schemas, keine Fixtures, keine Broker-Produktmodule (alles unter `broker/src/` ausser dem Probe-Binary), kein Produktcode im Plugin. |
| Geänderte Dateien Runde 1 | `eq-copilot/plugin/tests/SessionSoakMain.cpp`, `tools/eq-copilot/pruefe_session_soak.py`, dieses Manifest und der gespeicherte Bericht `roh/G3-SOAK-nacharbeit-1-kanon-bericht.json`. `eq-copilot/plugin/CMakeLists.txt` blieb **unverändert** — `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` stand dort bereits (`:423`); der Probe-Broker ebenfalls, weil kein neuer Handshake auf seiner Seite nötig war. |
| Geänderte Dateien Runde 2 | nur `tools/eq-copilot/pruefe_session_soak.py`, dieses Manifest und die Rohbelege unter `docs/beweise/roh/`. **Kein C++**, kein Broker, kein Vertrag: der Rest lag allein in der Urteilslogik des Prüfers. `SessionSoakMain.cpp` liefert `k_s5.erwartet` bereits als `anzahl + 1` (`:1407`) — geändert hat sich, dass der Prüfer diesen Wert nicht mehr glaubt, sondern nachrechnet. |

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
| S03 | Eingeschwungen × Dauerbetrieb bis Laufende: je Sonde 10-Hz-Liveframes (2.048 Samples), 1-Hz-Grossfenster (16.384 Samples) und 1-Hz-Heartbeats | Die Mitgliedschaft driftet über die ganze Laufzeit nicht: Zeilenzahl bleibt N, die `instanceId`-Menge bleibt identisch, `fuehrendesMain` bleibt dieselbe Adresse. Jede Snapshot-Übernahme wird **im Callback, unmittelbar nach der Übernahme** geprüft — Rückgabewert von `uebernehmeSessionSnapshot` und die daraus entstandene Sicht (N Mitglieder, Menge der `instanceId`, `fuehrendesMain`). Die 250-ms-Abtastung bleibt als **zusätzlicher** Prüfpunkt (`abtast_*`), weil sie auch Drift ohne neuen Snapshot sieht. Ein einziger abweichender Snapshot ausserhalb der Neustartfenster ist ROT; ein `ungueltig`-Ergebnis ebenso. | Z1 | **NEU** `A24:mitgliedschaft_driftet_nicht` | Entwurf `:4262-4264`; `:3586` („ohne ungegrenztes Wachstum") |
| S04 | Eingeschwungen × P0-Heartbeat je Sonde, 1.000 ms Kadenz | Reihenfolge: Sendezeit vor `sendeP0` stempeln; wird `sendeP0` abgewiesen, gilt der Heartbeat als **nie eingereiht** und wird nicht erwartet (Muster `IpcLastMain.cpp:229-235`). Ausserhalb der Neustartfenster geht **kein** ACK verloren; p95 der ACK-Latenz **< 1.000 ms**, auch während die langsamen Leser bremsen. | Z4 | **NEU** `A24:p0_kein_verlust_und_p95` | `IpcLastMain.cpp:42,216-240`; `coordinator.rs:28` |
| S05 | Eingeschwungen × ein Viertel der Sonden liest künstlich verzögert | Die künstliche Verzögerung wird **nach** dem Zeitstempel des Empfangs eingelegt, nie davor. Sonst mässe das Bein seine eigene Bremse statt der Drahtlatenz — die Zusage wäre unfalsifizierbar. Die langsamen Sonden bleiben über den ganzen Lauf Mitglied und werden ausserhalb der Neustartfenster nie stale. | Z6, Z5 | **NEU** `A24:langsame_bleiben_mitglied` | Prüfliste Abschnitt A; `coordinator.rs:29-31` |
| S06 | Langsamer Leser × seine Telemetrieschleuse läuft voll | Rückstau heisst „der älteste weicht": `ersetzt > 0`, `beanspruchtVerworfen == 0`. Jede abgelehnte Veröffentlichung hat einen gezählten Grund (`abgelehnt == zuGross + beanspruchtVerworfen`), wobei `abgelehnt` aus den **Rückgabewerten** von `veroeffentlichen()` kommt — sonst wäre die Gleichung eine Tautologie (§15, Defekt 4). Alle Cap-Zähler unter `langsam` summieren **nur über die langsamen Sonden**; der K-S1-Fluter steht getrennt unter `fluter.*` und zählt nie für S06 (§15, Defekt 5). Damit die Schleuse eines langsamen Lesers wirklich vollläuft, überproduziert er `kLangsamBurst = 3` Frames je Livetakt — ein langsamer Lese-Callback bremst den Control-Pfad, nicht die Telemetrieschleuse. Kein langsamer Leser verzögert die schnellen: deren p95 aus S04 hält unabhängig. | Z6 | **NEU** `A24:cap_ersetzt_aeltesten_und_blockiert_nie` | `core/ipc/TelemetryClient.cpp:355-364`; `IpcLastMain.cpp:311-330`; Prüfliste A |
| S07 | Eingeschwungen × Minutentick | RSS beider Prozesse wird minütlich gelesen und als vollständige Kurve berichtet. Nach dem Warmup wächst keiner bis zum Ende seiner Generation um mehr als **max(10 %, 16 MiB)**. Brokerkurve je Generation, Clientkurve über den ganzen Lauf (§3.2). | Z3 | **NEU** `A24:speicherkurve_im_budget` | Entwurf `:3586,3588-3590`; §3.2 |
| S08 | Eingeschwungen × N `SondeProcessor` treiben Ganzblöcke in Echtzeitkadenz | 0 Ganzblockdrops durch Überlauf, 0 durch Oversize; 0 Kontinuitätsbrüche ausserhalb bewusst erzeugter Transportkanten. **Je Sonde** muss ausserdem mindestens ein Producer-Frame je Analysefenster veröffentlicht sein: der Bericht trägt `publikationen_je_sonde_min`, und der Prüfer rechnet die Erwartung aus `ceil(kLiveIntervallS · fs / block)` (`core/analysis/FeatureEngine.h:445`) mit Toleranz 0,8. Eine blosse Summe verbärge einen einzelnen stehengebliebenen Analyseworker (§15, Defekt 7). Kontinuität wird über `transport.continuity_segment` des zuletzt veröffentlichten Producer-Frames beobachtet (`letzterProducerFrameFuerTest`), weil `SondeProcessor` selbst keinen Bruchzähler veröffentlicht — der liegt nur auf `PluginProcessor` (`src/PluginProcessor.h:146`). Grenze §2.2 gilt. | Z7 | **NEU** `A24:audio_ganzblock_ohne_drop` | `sonde/SondeProcessor.h:205-217`; `core/analysis/FeatureEngine.h:140-144`; B4 `QueueStressTestMain.cpp:504-556`; B12 |
| S09 | Eingeschwungen × Broker-Prozess wird zum festen Zeitpunkt getötet | Reihenfolge §4.3 Schritte 1–3. Der Main verlässt `subscriptionAktiv`; das Modell zeigt `disconnected`, nicht die letzte grüne Sicht. **Je Kill** verlangt der Bericht beides: `s09_subscription_weg > 0` **und** `s09_quellen_getrennt > 0` (mindestens eine Quelle auf `Control::getrennt`, `SourcesModel.cpp:381-387`). Ein Gesamtzähler liesse einen Kill ohne sichtbares `disconnected` durchgehen, solange ein anderer zählte (§15, Defekt 8). Die Zeitmessung des Neustartfensters beginnt beim Kill, die Fristmessung erst bei `BEREIT`. | Z2 | **NEU** `A24:kill_beendet_subscription_sichtbar` | §4.3; SONDE-012 L10; `src/SourcesModel.h:99-108` |
| S10 | Broker tot × Clients im Backoff | Kein Absturz, keine Endlosschleife, **kein anderer Pipename**. Drei getrennte Belege statt eines nie setzbaren Zählers (§15, Defekt 9): (1) jede der 2·(N+1) Pipenamen-Übergaben an einen Client läuft durch die Erlaubnisliste und wird gezählt; (2) das Programm verweigert mit **Exit 3**, wenn es ohne `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` gebaut ist — sonst öffneten die `SondeProcessor` die Produktions-Pipe; (3) der Prüfer listet den **Pipe-Namensraum** vor und nach dem Start (Verfahren aus Register NAK-118) und verlangt, dass die eigene Probe-Pipe dort sichtbar ist und kein Produktions- oder Goldenname neu erschien. Die Clients verdoppeln ihren Backoff bis 8.000 ms und versuchen ausschliesslich denselben Probe-Namen. Der Zeitraum wird als Neustartfenster markiert; Zusagen Z4/Z5 sind darin ausgesetzt und werden getrennt gezählt, nicht stillschweigend ausgelassen. | Z2, Z8 | **NEU** `A24:backoff_ohne_namenswechsel` | `core/ipc/IpcVerbindung.cpp:67-99`; `core/ipc/ControlClient.cpp:1014-1025` |
| S11 | Neuer Broker horcht auf demselben Namen × Clients reconnecten | Innerhalb der Frist aus §3.1 (**60 s ab `BEREIT`**) je Clientpaar: der Main subscribt neu (Reihenfolge §4.1), alle N Sonden reporten neu, und der Snapshot ist wieder vollständig (N Mitglieder, erwartete Adressen, `fuehrendesMain` = Main-Adresse). Der Bericht trägt je Neustart die Verteilung der Reconnect-Dauer (`min`, `p95`, `max`) über **N + 1 einzeln gemessene Clientpaare** — Main-Paar plus jede Sonde, jeweils vom `BEREIT` bis zum **eigenen** Wieder-Mitglied-Zeitpunkt (beide Verbindungen stehen und das Paar ist wieder in der Sicht des Main); `reconnect_paare` belegt die Zahl. Die Vorfassung trug N Kopien desselben Topologieendes (§15, Defekt 10). Die Lesart eines Fehlschlags steht in §3.1. Ein Reconnect nutzt ausschliesslich die bestehenden Client-Pfade; A24 fügt keinen Produktpfad hinzu. | Z2, Z1 | **NEU** `A24:reconnect_vollstaendig_in_frist` | §3.1; §4.3; Entwurf `:4262-4264` |
| S12 | Nach dem Neustart × jeder weitere Snapshot | Die `broker_epoch` ist eine **neue**; die alte wird nach dem Neustart nie wieder als aktueller Lauf ausgespielt. Der Zähler dafür muss strukturell 0 bleiben und ist eine Wache mit Test (Prüfliste A). Keine fremde Steueradresse erscheint. | Z2 | **NEU** `A24:alte_epoche_kommt_nie_wieder`; Gegenstueck im Unittest bestehend in `broker/tests/sonde012_sources_slice.rs`, Test `restart_has_no_old_live_or_measurement_truth` (dort Zeile 696) | SONDE-012 L12 (`docs/beweise/SONDE-012.md:152`); `broker/src/bin/eqcop-broker-sonde012-probe.rs:34` |
| S13 | Beliebig × Liveness-Tick | Ausserhalb der Neustartfenster wird **kein** Mitglied stale (Grenze > 2.500 ms) und keines evicted (10.000 ms). Innerhalb der Fenster ist beides erlaubt und wird getrennt gezählt und berichtet. Die Heartbeat-Kadenz von 1.000 ms hält den Abstand zur Stale-Grenze; wird sie im Lauf nicht erreicht, ist das der Befund. | Z5 | **NEU** `A24:kein_stale_ausserhalb_neustart`; bestehend in `broker/tests/coordinator_model.rs` die Tests `stale_konstanten_ableitung` (Zeile 527) und `stale_grenze_exklusiv_und_kontakt_setzt_zurueck` (Zeile 535) | `broker/src/coordinator.rs:28-32`; SONDE-012 L07 |
| S14 | Lauf läuft × Laufende erreicht | Gegenpfad zu S01: Nachlauffenster von `kNachlaufMs` = Heartbeat-Kadenz + P0-Schranke = **2.000 ms** für ausstehende ACKs — in ihm wird kein Heartbeat mehr eingereiht, aber alle Clients lesen weiter; erst danach heisst ein offener ACK Verlust (§15, Defekt 16). Danach Telemetrie **vor** Control stoppen (Muster `IpcLastMain.cpp:280`), Audiotreiber-Threads beenden und joinen, `SondeProcessor` zerstören, danach dem Broker `STOP` schicken und auf ihn warten. Kein hängender Thread, kein verwaister Prozess; das Bein endet mit einer Urteilszeile und einem Exitcode, nie mit Stille. | — (Änderungssatz: verbinden↔trennen, starten↔stoppen) | **NEU** `A24:sauberer_abbau` | Prüfliste Abschnitt B und F; `IpcLastMain.cpp:280`; `sonde/SondeProcessor.cpp:107-115` |
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

**Wann der Beleg genommen wird — nachgearbeitet in Runde 1 (§15).** Ein Beleg,
der zu einem anderen Zeitpunkt entsteht als das Ereignis, über das er urteilt,
belegt nichts. Die vier Zeitpunkte sind seither Handshakes im Protokoll:

| Zeitpunkt | Handshake | Was dort belegt wird |
|---|---|---|
| **vor** dem Kill | `KILL_VORBEREITEN` → `KILL_BEREIT` | nur die **Vorbedingung** von K-S1: war die Sonde verbunden und im Rückstau? |
| **am** Kill | `KILL_ERFOLGT <i> <killdauer_ms>` | K-S2 (`snapshot_vor_kill`) und K-S4 (`flag_zum_killzeitpunkt`) — beide gegen den Killzeitpunkt, den das Programm beim Empfang dieser Zeile stempelt |
| **in** der Totzeit | `TOTZEIT_ENDE <i>` → `TOTZEIT_ERFASST <i>` | K-S1 (`telemetrie_handle_fehler`) und K-S5 (`backoff_deckel_erreicht`) — **bevor** der Prüfer den neuen Broker startet |
| **nach** dem Nachlauffenster | Schlussbericht | K-S3 (`p0_ohne_ack_im_fenster`) = `p0.verloren_im_neustartfenster`; erst ein hier noch offener ACK ist endgültig verloren |

Der Killzeitpunkt-Stempel entsteht beim Empfang von `KILL_ERFOLGT`, also einen
Pipe-Write und ein `getline` nach dem Rücksprung aus `Popen.kill()`. Der Rest
wirkt konservativ: ein zu später Stempel meldet höchstens `nicht_getroffen`,
nie einen falschen Treffer.

| ID | Killpunkt | Erwartung nach dem Neustart | Wie dieser Punkt getroffen wird | Test |
|---|---|---|---|---|
| K-S1 | Broker stirbt, **während eine Sonde P2 auf ihrem offenen Telemetrie-Handle schreibt** | Ein halber Frame resynchronisiert nicht, sondern beendet den Strom (bestehende Envelope-Regel). Nach dem Neustart ist der erste Zustand am Main `disconnected`, danach wieder vollständig innerhalb der Frist aus §3.1. Kein halber Frame wird je als Messwert sichtbar. | **Wahrscheinlich (beobachtet), mit gemessener Vorbedingung.** Das Signal ist telemetriespezifisch: gewertet wird ausschliesslich der `TelemetryClient` einer Sonde, **nie** deren `ControlClient` — ein Control-Fehler belegt Controlverkehr, nicht den P2-Frame. Feld: `k_s1.telemetrie_handle_fehler`, **getroffen** nur wenn alle vier Punkte gelten: (1) die Sonde war unmittelbar vor dem Kill `TelemetryClient::Status::verbunden` (`core/ipc/TelemetryClient.h:65`) — das Programm hält den Status je Sonde vor dem Kill fest; (2) ihre Schleuse war im letzten Beobachtungsfenster **nachweislich im Rückstau** (`ersetzt` ist gestiegen), sodass der verbundene Lauf im Schreibzweig steht und nicht im Leerlauf-Lesezweig (`core/ipc/TelemetryClient.cpp:736-762`); (3) nach dem Kill ist `letzterFehler` gesetzt — dieses Feld wird beim Eintritt in `verbunden` geleert (`core/ipc/TelemetryClient.cpp:700-704`), jeder spätere Eintrag stammt also **per Konstruktion vom offenen Handle**; (4) `verbindungsVersuche` ist dabei unverändert, der Fehler kommt also nicht von einem neuen Verbindungsversuch. Die fünf vertragsinternen Texte (`P2-Frame zu gross`, `Kopplung gewechselt …`, `Envelope abgelehnt …`, `P0/P1 auf der Telemetrieverbindung …`, `Nachrichtenratengrenze …`) sind ausgeschlossen; sie sind keine Drahtfehler. **Grenze, die bleibt:** Schreibzweig (`:750-757`) und Leerlauf-Lesezweig (`:453-461`) schreiben bei gebrochener Pipe **denselben** Win32-Text in dasselbe Feld, und kein Zähler markiert einen gescheiterten Write (`gesendet` zählt nur Erfolge). Punkt (2) macht den Lesezweig unwahrscheinlich, schliesst ihn aber nicht aus. K-S1 behauptet deshalb „Fehler auf dem offenen P2-Handle einer verbundenen, im Rückstau schreibenden Sonde" — **nicht** „Frame nachweislich mitten im Schreiben". Fehlt einer der vier Punkte, ist K-S1 `nicht_getroffen`, nie bestanden. | **NEU** `A24:kill_im_frameverkehr`; bestehende Envelope-Hälfte `broker/tests/transport_fuzz.rs` (ein kaputter Frame beendet den Strom) |
| K-S2 | Broker stirbt **während `subscribe_session`** — nach Verbindung des Main, bevor der absolute Snapshot beim Main ankommt | Der Main hält keinen halb offenen Subscription-Zustand: `controlEnde()` läuft, `subscriptionAktiv` ist false, das Modell zeigt `disconnected`. Der nächste erfolgreiche Reconnect subscribt erneut und bekommt einen **absoluten** Snapshot; es gibt kein `unsubscribe_session` und keinen Event-Replay. | **Barriere (gemessen), ab `--neustarts >= 2`.** Ein Kill nach der `BEREIT`-Zeile beobachtet dieses Fenster nicht — die Subscription entsteht erst im Verbunden-Callback (`eq-copilot/plugin/tests/Sonde012SourcesLatencyTest.cpp:131-140`). Das C++-Programm schreibt deshalb `MAIN_SUBSCRIBE_GESENDET`, sobald `sendeP1("subscribe_session", …)` eingereiht ist **und noch kein Snapshot angekommen ist**; der Prüfer killt unmittelbar nach dieser Zeile. Der Bericht trägt `k_s2.snapshot_vor_kill`: kam der Snapshot dennoch vor dem Kill an, ist K-S2 `nicht_getroffen` — nie bestanden. **Gemessen wird das am Killzeitpunkt** (§15, Defekt 11): das Programm vergleicht die Ankunftszeit der letzten *übernommenen* Snapshot-Nachricht gegen den Stempel aus `KILL_ERFOLGT`, statt wie vorher ein Flag vor `KILL_BEREIT` abzulesen. Bei `--neustarts 1` (Kanon-Kurzform) `nicht_gefahren`. | **NEU** `A24:kill_waehrend_subscribe` |
| K-S3 | Broker stirbt **während eines Heartbeats** — P0 ist gesendet, das `heartbeat_ack` kommt nicht mehr | Der verlorene Heartbeat wird als `p0.verloren_im_neustartfenster` gezählt und **nicht** als Verletzung von Z4 gewertet; Z4 gilt ausserhalb der Fenster. Nach dem Reconnect laufen Heartbeats derselben Sonde weiter; die Sequenznummern springen nicht rückwärts und kein ACK wird doppelt verbucht. | **Wahrscheinlich (beobachtet).** N Sonden senden mit 1.000 ms Kadenz, aber ein Kill auf fester Wanduhr garantiert kein ausstehendes ACK. Berichtsfeld `k_s3.p0_ohne_ack_im_fenster`: **getroffen** genau dann, wenn `p0.verloren_im_neustartfenster > 0`. Ist der Zähler 0, ist K-S3 `nicht_getroffen` — nicht bestanden. **Der Beleg wird erst nach dem Nachlauffenster genommen** (§15, Defekt 1): ein zum Kill unbeantworteter Heartbeat ist noch kein verlorener ACK, solange er später beantwortet werden kann. Der Prüfer hält beide Zahlen gegeneinander; weichen sie ab, misst der Killbeleg etwas anderes als der Schlussbericht. | **NEU** `A24:kill_waehrend_heartbeat` |
| K-S4 | Broker stirbt, **während ein langsamer Leser in seiner künstlichen Verzögerung steckt** | Der langsame Leser blockiert weder seinen eigenen Reconnect noch den der anderen. Er ist innerhalb derselben Frist aus §3.1 wieder Mitglied. Seine Schleuse verwirft dabei alte Liveframes (Cap), verliert aber nie den neuesten. | **Barriere (gemessen).** Ein dauerhaft eingeschalteter langsamer Modus beweist nicht, dass der Kill in einen blockierten Aufruf fällt. Jede langsame Sonde führt deshalb ein **atomares Flag** „steckt gerade in der künstlichen Verzögerung"; sobald es gesetzt ist, schreibt das Programm die Zeile `LANGSAM_IN_VERZOEGERUNG`, und der Prüfer killt unmittelbar danach. Der Bericht trägt `k_s4.flag_zum_killzeitpunkt`; war das Flag beim Kill nicht mehr gesetzt, ist K-S4 `nicht_getroffen`. Seit Runde 1 führt jede langsame Sonde **zwei Ereignisstempel** statt eines Flags (`verzoegerungBeginn`, `verzoegerungEnde`); bei `KILL_ERFOLGT` gilt sie genau dann als steckend, wenn `beginn <= killzeit && ende < beginn` — ein Flag hätte nur den Abfragezeitpunkt beantwortet (§15, Defekt 11). | **NEU** `A24:kill_mit_langsamem_leser` |
| K-S5 | Broker bleibt nach dem Kill **mindestens 20 s tot**, sodass jeder Control-Client den Backoff-Deckel (8.000 ms) erreicht hat | Auch aus dem Deckel heraus verbinden alle Clientpaare innerhalb der Frist aus §3.1 ab der letzten `BEREIT`-Zeile — genau dafür trägt die Schranke in §3.1 den Deckel zweimal. Kein Client bleibt dauerhaft hängen. | **Barriere (gemessen) durch Totzeit, ab `--neustarts >= 2`.** Ein zweiter Kill nach `BEREIT` erzeugt keinen Deckel; der entsteht erst durch wiederholte Fehlversuche (`core/ipc/ControlClient.cpp:1014-1025`). Die Backoff-Folge eines Control-Clients ist 500 + 1.000 + 2.000 + 4.000 + 8.000 = 15.500 ms bis zum Deckel (`core/ipc/IpcVerbindung.h:34-35`); eine Totzeit von ≥ 20 s liegt darüber. Gemessen wird nicht die private Backoff-Variable, sondern ihr Beleg: `ControlClient::Snapshot::verbindungsVersuche` (`core/ipc/ControlClient.h:176`, erhöht bei **jedem** Versuch, `core/ipc/ControlClient.cpp:1048`) muss in der Totzeit je Control-Client um **≥ 5** steigen. `k_s5.backoff_deckel_erreicht` ist die Zahl der Control-Clients, die das erfüllen; sie muss **N + 1** sein (N Sonden + Main), sonst `nicht_getroffen`. **Diesen Sollwert rechnet der Prüfer selbst** aus der geplanten Sondenzahl `--sonden`, seit Runde 2 (§16); das Berichtsfeld `k_s5.erwartet` bleibt Pflicht, wird aber gegen die eigene Rechnung gehalten statt sie zu ersetzen — sonst genügte ein Bericht mit `backoff_deckel_erreicht = erwartet = 0`, um K-S5 grün zu melden, ohne dass ein Client den Deckel je erreicht hat. **Der Zähler wird während der Totzeit gelesen** (§15, Defekt 12), bevor der Prüfer den neuen Broker startet — nach `BEREIT` gemessen, konnte schon ein erfolgreicher Versuch gegen den NEUEN Broker den Deckel scheinbar erreichen. Die Telemetriethreads werden **nicht** mitgezählt: ohne Kopplung nehmen sie den Warte-Zweig, der `backoffMs` nicht verdoppelt (`core/ipc/TelemetryClient.cpp:383-398`). | **NEU** `A24:kill_aus_backoff_deckel` |

## 7. Bericht und Zählerkarte

Der Prüfer schreibt **eine** JSON-Zeile mit allen Zählern und der minütlichen
Speicherkurve auf stdout; sie geht roh ins Manifest. Kein Zähler wird
weggelassen, weil er 0 ist — ein Zähler, der strukturell 0 sein muss, ist eine
Wache mit Test (Prüfliste A).

Feste Felder (Namen sind Vertrag zwischen C++-Programm und Prüfer, kein
Wire-Vertrag):

```
sonden, minuten, neustarts, langsam_anzahl, warmup_s, clientpaare
mitgliedschaft: snapshot_pruefungen, vollstaendig,    # SOFORT je Uebernahme
                uebernahmen, ungueltig,               # Rueckgabewert S03
                abtast_pruefungen, abtast_vollstaendig,   # 250-ms-Abtastung
                fremde_adresse, fuehrendes_main_falsch
neustart[]:     index, epoch_alt, epoch_neu, frist_ms,
                reconnect_ms{min, p95, max},          # je Clientpaar, §3.1
                reconnect_paare,                      # muss N + 1 sein
                bereit_bis_vollstaendig_ms,
                schranke_ms = 45051, ueber_schranke,  # Lesart-Tabelle §3.1
                alte_epoche_nach_neustart_gesehen,
                s09_sichten, s09_subscription_weg,    # Zeile S09, je Kill
                s09_quellen_getrennt,                 #   beide muessen > 0 sein
                kill_erfolgt_gesehen, kill_dauer_ms,  # Handshake KILL_ERFOLGT
                totzeit_erfasst,                      # Handshake TOTZEIT_ENDE
                backoff_deckel_in_totzeit,            # K-S5, in der Totzeit
                totzeit_ms
p0:             gesendet, beantwortet, verloren_ausserhalb_neustart,
                verloren_im_neustartfenster, latenz_p95_ms, latenz_max_ms,
                gemessen_ausserhalb, gemessen_im_stoerfenster,
                latenz_max_im_stoerfenster_ms,
                schranke_ms, nachlauf_ms              # Nachlauffenster S14
liveness:       stale_ausserhalb_neustart, evicted_ausserhalb_neustart,
                stale_im_neustartfenster
langsam:        sonden, immer_mitglied, ersetzte_liveframes,
                neueste_verworfen, abgelehnt,         # aus dem Rueckgabewert
                veroeffentlichungen, zu_gross, kollisionsloecher,
                schnelle_p95_ms, fluter_ueberlappt,   # true nur bei N = 1
                blockiert_andere_nicht: true | false | nicht_anwendbar
                                                      # nicht_anwendbar bei N = 1
                # ALLE Cap-Zaehler nur ueber die LANGSAMEN Sonden (§15, Defekt 5)
fluter:         ersetzte_liveframes, neueste_verworfen, abgelehnt,
                veroeffentlichungen, zu_gross, kollisionsloecher
                # der K-S1-Fluter (Sonde 0) getrennt; zaehlt NIE fuer S06
audio:          bloecke, ganzblockdrops_ueberlauf, ganzblockdrops_oversize,
                kontinuitaetsbrueche, publikationen,
                publikationen_je_sonde_min,           # Zeile S08, muss > 0 sein
                bloecke_je_sonde_min, samplerate, blockgroesse
subscription:   weg_im_neustartfenster                # Zeile S09, muss > 0 sein
pipe:           fremder_name_versucht,                # Zeile S10, muss 0 sein
                clientnamen_geprueft,                 # 2 x (N + 1) Uebergaben
                ohne_produkt_v3,                      # muss true sein
                namensraum_lesbar, eigene_probe_sichtbar,
                produktionsname_neu, goldenname_neu,  # beide muessen false sein
                namen_vor, namen_nach                 # Namensraum, NAK-118
riegel:         golden_<programm>, produktion_<programm>   # Zeile S15, je 3
speicher:       takt_s, budget_prozent, budget_bytes,
                client[]{minute, sekunden, rss_bytes, im_neustartfenster},
                broker[]{generation, minute, sekunden, rss_bytes,
                         im_neustartfenster}
                # `sekunden` traegt die BEOBACHTETE Messzeit: erster und
                # letzter gueltiger Punkt einer Kurve muessen >= 30 s
                # auseinanderliegen (§15, Defekt 2)
client_exit, abbau_haenger                            # Zeile S14
kill:           je Killmatrixzeile { urteil: getroffen | nicht_getroffen |
                nicht_gefahren, gefahren, beleg }, dazu die Belegfelder
                k_s1.telemetrie_handle_fehler, k_s2.snapshot_vor_kill,
                k_s3.p0_ohne_ack_im_fenster, k_s4.flag_zum_killzeitpunkt,
                k_s5.backoff_deckel_erreicht, k_s5.erwartet
```

Seit der Nacharbeit Runde 1 (§15) **rechnet der Prüfer jedes `kill.*.urteil` aus
seinem Belegfeld nach** und hält es gegen das gelieferte Urteil; `gefahren` steht
dafür als eigenes Feld daneben. Alle fünf Einträge sind Pflicht — ein fehlender
Eintrag oder ein fehlendes Belegfeld ist ROT, nie ein übersprungener Punkt.

Seit der Nacharbeit Runde 2 (§16) gilt dasselbe für den **Sollwert**: `k_s5.erwartet`
ist Pflichtfeld, aber es liefert den Sollwert nicht. Der Prüfer rechnet ihn selbst
als `--sonden + 1` (alle Control-Clients: Main plus N Sonden) und hält das
Berichtsfeld dagegen; die Ableitung von `k_s5.urteil` benutzt ausschliesslich die
eigene Rechnung. Ein Bericht, der `erwartet` an seinen eigenen Istwert anpasst,
fällt damit zweimal — am Sollwertvergleich und am Urteilswiderspruch. Ein
Sollwertfeld an einer Killzeile, für die der Prüfer keine eigene Rechnung kennt,
ist ebenfalls ROT; K-S5 ist zurzeit die einzige Zeile mit einem Sollwert.

Das Laufprotokoll nennt zu jeder Urteilszeile die Matrix-ID **und** den
Prüfpunktnamen aus §5 beziehungsweise §6 — `[S01 · A24:topologie_steht_in_frist]`,
`[k_s1 · A24:kill_im_frameverkehr]`. Ein Verweis aus diesem Manifest ist damit im
Protokoll auffindbar; die Tabellen dafür stehen als `PRUEFPUNKTE` und
`KILLPUNKTE` im Prüfer.

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

`tools/dirigent/pruefliste.md`, abgehakt vor jedem Commit. Nach dem Bau steht
in der rechten Spalte der **gemessene** Ort: Datei, Zeilenname oder Berichtsfeld
aus den Läufen in §13.

| Prüflistenzeile | Wo A24 sie misst |
|---|---|
| A — Politik bei *voll* steht ausdrücklich je Klasse | **gemessen** in §15: `langsam.ersetzte_liveframes`, `neueste_verworfen`, `zu_gross`, `abgelehnt` und `veroeffentlichungen` — fünf getrennte Felder, seit Runde 1 **nur über die langsamen Sonden** summiert; der K-S1-Fluter steht getrennt unter `fluter.*` (Defekt 5) |
| A — Rückgabewerte und Zähler werden ausgewertet, nicht ignoriert | `SessionSoakMain.cpp`: `Sonde::publiziere()`/`publiziereRoh()` zählen **jeden** Rückgabewert von `veroeffentlichen()` — `langsam.abgelehnt` kommt seit Runde 1 aus der Rückgabe, nicht aus `zuGross + beanspruchtVerworfen` (§15, Defekt 4). `heartbeatLauf()`: eine abgewiesene `sendeP0` nimmt den Eintrag wieder zurück; `mainText()` wertet `SnapshotErgebnis` aus und zählt `ungueltig`. Die Wachen `alte_epoche_nach_neustart_gesehen` und `pipe.fremder_name_versucht` sind strukturell 0 und mit `--mutant s12` bzw. `s10_define` gebrochen (§14, §15) |
| A — höhere Klasse wird gelesen, solange niedrigere blockiert | **gemessen** in §15.4: 4 langsame Sonden bremsen ihren Lesethread und halten ihre eigene Schleuse im Rückstau (`langsam.ersetzte_liveframes` 6.139 bei 17.664 Veröffentlichungen); `langsam.schnelle_p95_ms` bleibt bei 21,6 ms und `langsam.blockiert_andere_nicht` ist true. Bei N = 1 (S16) meldet das Feld `nicht_anwendbar`, nie bestanden — und der Prüfer verlangt das dort ausdrücklich |
| B — Reihenfolge beim Verbinden festgelegt und gemessen | `SessionSoakMain.cpp`, `mainLinkStatus()`: `beginneSubscription` **vor** `sendeP1`; ein abgewiesener P1 endet sofort in `controlEnde()`. Gemessen als `topologie_ms` 82 bzw. 81 ms (§13), in Runde 1 83 ms (§15.4) |
| B — beim Trennen zuerst abmelden, dann Fristen und Joins | `SessionSoakMain.cpp`: vor `abbauen()` läuft seit Runde 1 ein **Nachlauffenster** von `kNachlaufMs` = Heartbeat-Kadenz + P0-Schranke = 2.000 ms, in dem keine neuen Heartbeats eingereiht werden, alle Clients aber weiterlesen; erst danach joinen die Threads und stoppt Telemetrie vor Control. Der Prüfer schickt danach `STOP` an den Broker. Gemessen als `client_exit` 0 und `p0.nachlauf_ms` 2000 (§15); mit `--mutant s14_nachlauf` fällt die Zeile |
| B — jeder Join hat eine Frist | **gemessen** in §15.4: Topologie 83 ms von 60.000 ms; der Reconnect wird seit Runde 1 **je Clientpaar** gemessen — min 538,8 / p95 2.517 / max 2.517 ms über 17 Paare, alle unter der gerechneten Schranke von 45.051 ms, `ueber_schranke` false. Mit `--mutant s11` (Frist 0 ms) fällt die Zeile, mit `--mutant s11_paare` die Verteilung selbst (§14, §15.3) |
| B — Reihenfolge beim Verbinden gilt auch für die zweite Verbindung | §3.1 und §4.1/§4.2: die Telemetrie-Runde ist kein Anhängsel der Control-Runde, sondern eine eigene Runde mit eigenem Backoff und eigenen Bootstrap-E/A-Fristen; S01 und S11 messen erst das **vollständige** Clientpaar |
| C — Verträge und Längen | **nicht Gegenstand von A24.** Das Bein berührt keinen Vertrag; die Vertragsriegel bleiben bei A5/A9/B3c/A21 |
| D — ein Bein prüft die Frische seiner Eingaben und meldet Exit 3 | `pruefe_session_soak.py`, `main()`: fehlt eines der beiden Binaries, endet der Lauf mit **Exit 3** und dem Baubefehl. Die Frische erzwingt der Runner über `$gemesseneZiele` (§9); `--mutant s15` zeigt Exit 3 auch für den Pipenamen-Riegel (§14). Seit Runde 1 prüft `SessionSoakMain.cpp` in `main()` zusätzlich, dass das Ziel **mit** `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` gebaut ist, und meldet sonst Exit 3 — ohne dieses Define öffneten die `SondeProcessor` die Produktions-Pipe (§15, Defekt 9) |
| D — was der Kanon nicht baut, darf er nicht als frisch bezeugen | §9: `EqCopSessionSoak` steht in `$gemesseneZiele` |
| E — Behauptung ≤ Messung | §2.2 (XRun-Grenze steht ausdrücklich drin), §5.1 (was die Matrix nicht sagt), §3.1 (Schranke 45,1 s und Frist 60 s sind getrennt benannt, der Zuschlag ist ausgewiesen statt in die Rechnung gemogelt), §6 (jede Killzeile trägt **Barriere** oder **wahrscheinlich**, nie „deterministisch" ohne Beleg), K-S1 bis K-S5 (`nicht_getroffen`/`nicht_gefahren` statt stiller Erfolg). **§15.6** nennt, was auch Runde 1 nicht hergibt: die Exit-3-Wache für das fehlende Define ist nicht gefahren, und die Vorbedingung von K-S1 unterscheidet seit der S06-Korrektur nicht mehr zwischen Fluter und langsamem Leser. **§15.5** korrigiert eine Behauptung aus Phase 2 nach unten: K-S3 wurde nie getroffen |
| E — Zahlen im Manifest sind gemessen, nicht abgeschrieben | §3 (jede Konstante mit `Datei:Zeile`), §7 (jeder Zähler kommt aus dem Lauf); K-S5 misst den Backoff-Deckel nicht an der privaten Variablen, sondern an `verbindungsVersuche` — ein öffentlicher Zähler mit belegter Erhöhungsstelle — und seit Runde 1 **während der Totzeit**, nicht nach dem Neustart. Auch die S08-Erwartung wird gerechnet statt abgeschrieben: `bloecke_je_fenster = ceil(kLiveIntervallS · fs / block)` aus `core/analysis/FeatureEngine.h:445` und der Samplerate/Blockgrösse des Laufs |
| E — jede neue Prüfung wurde einmal absichtlich gebrochen | **§14** (Phase 2, sechs Mutanten), **§15.3** (Runde 1, siebzehn weitere — einer je geschlossenem Defekt) und **§16.2** (Runde 2, der 23. Mutant `k_s5_sollwert`). Jeder verfälscht genau eine Grösse; alle enden mit Exit 2, `s15` mit Exit 3. Der Rotbeweis läuft seit Runde 1 über `--bericht <datei> --mutant <name>` an einem gespeicherten Lauf, statt je Mutant einen weiteren Soak zu fahren |
| E — eine Zeile, die nicht getroffen wurde, gilt nie als bestanden | §6 (Vorwort und alle fünf Killzeilen), §7 (`kill.*.urteil` folgt allein aus dem Belegfeld), S16 (`nicht_anwendbar` bei N = 1). Seit Runde 1 **rechnet der Prüfer jedes Killurteil aus dem Beleg nach** und verlangt alle fünf Pflichteinträge; ein fehlender Eintrag oder ein Widerspruch ist rot (§15, Defekt 14). Seit Runde 2 gilt dasselbe für den **Sollwert**: K-S5 fällt gegen `--sonden + 1` aus dem Aufruf, nicht gegen das gelieferte `erwartet` (§16). K-S3 meldet ehrlich `nicht_getroffen`, statt früh abgetastete, später beantwortete ACKs als Treffer auszuweisen |
| F — verbinden↔trennen, starten↔stoppen im selben Änderungssatz | `aufbauen()`↔`abbauen()` und `killErfolgt()`↔`totzeitEnde()`↔`bereitWieder()` stehen in derselben Datei und kamen im selben Commit; der Prüfer startet und beendet den Broker im selben `try/finally`. Seit Runde 1 hat jede Kill-Halbzeile ihren eigenen Handshake (`KILL_ERFOLGT`, `TOTZEIT_ENDE`/`TOTZEIT_ERFASST`), damit Beleg und Ereignis nicht auseinanderfallen |

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
3. **[offen] Der Gate-Lauf selbst.** Dieses Ticket liefert das Werkzeug und hat
   es über 2 und 5 Minuten gefahren (§13). Der 60-Minuten-Lauf, der 32er-Lauf
   über 30 Minuten und die drei kurzen Läufe mit 1/4/8 Sonden (S16) sind der
   Gate-Schritt G3 und werden im G3-Gate-Manifest belegt, nicht hier.
4. **[offen] K-S2 und K-S5 fehlen in der Kanon-Kurzform.** Sie brauchen
   `--neustarts >= 2` und werden dort als `nicht_gefahren` berichtet. Der
   Killpunkt-Lauf in §13 trifft beide; der Gate-Lauf mit `--neustarts 3`
   ebenfalls.
5. **[offen: keiner] Kein Produktentscheid.** Dieses Ticket ist reines Prüfwerkzeug;
   es ändert kein sichtbares Produktverhalten und legt dem User nichts vor.
6. **Weggelassen, weil ticketfremd:** in dieser Session wurde kein Befehl aus
   den gesperrten Mustern (`Remove-Item`, `rm -rf`, `git clean`,
   `git reset --hard`) ausgeführt. Der Prüfer beendet den selbst gestarteten
   Probe-Broker ausschliesslich über das eigene `subprocess.Popen`-Objekt
   (`broker_toeten`), nie über einen Prozessnamen. Temporäre Laufprotokolle
   liegen unter `$env:TEMP` und wurden nicht gelöscht.

## 12. Verlauf

| Datum | Schritt | Stand |
|---|---|---|
| 2026-09-02 | Phase 1: Verhaltensmatrix, Killmatrix, Fristen aus dem Code abgeleitet; kein Code geschrieben | Basis-Commit `03e1e17`, Stand `d60ceaf` |
| 2026-09-02 | Matrixprüfung Runde 1: Codex Thread `01a061ed-25ea-7242-a384-b936d10c62de` (gpt-5.6-sol, Effort high, read-only), **NEEDS_WORK**, vier Befunde, Nacharbeit Phase 1b | geprüft auf `f7a5125` |
| 2026-09-02 | Matrixprüfung Runde 2: frischer Codex Thread `01a06203-877a-7fe1-aa47-41fe219de8a2` (high, read-only, Fixdiff `d60ceaf`…`a570367`): S16, Fristableitung, K-S2 und K-S5 **geschlossen**; ein Defekt bleibt — K-S1 wertete Control-Felder und belegte damit Controlverkehr statt eines P2-Frames | geprüft auf `a570367` |
| 2026-09-02 | Phase 2, Schritt 1: K-S1 auf ein telemetriespezifisches Eintrittssignal umgestellt (`k_s1.telemetrie_handle_fehler`, vier Bedingungen, verbleibende Grenze benannt) | `edc01f0`, nach dem Rebase `850433a` |
| 2026-09-02 | Phase 2, Bau: `SessionSoakMain.cpp` (Ziel `EqCopSessionSoak`), `pruefe_session_soak.py`, Registrierung A24 samt Bauziel. Vier Fehler im eigenen Werkzeug gefunden und behoben — Zugriffsverletzung im Bericht, `flachesJsonObjekt` am verschachtelten Snapshot, Heartbeats vor dem Kill falsch dem Aussenraum zugerechnet, und messende Heartbeats ohne `adresse`, die der echte Coordinator nie beantwortet hat | `edc01f0` → `5a7670b`, nach dem Rebase `850433a` → `0ca3080` |
| 2026-09-02 | Phase 2, Beweis: beide Läufe grün (§13), alle fünf Killpunkte im Killpunkt-Lauf getroffen, sechs Rotmutationen liefern das erwartete Rot (§14) | `5a7670b`, nach dem Rebase `0ca3080` |
| 2026-09-02 | Phase 2, Kanon: **GRUEN 40/40** auf `7386741`, abgekoppelt gefahren. Die Zahl der Beine wächst um genau eines — A24 (146,42 s, Exit 0); SONDE-012 schloss mit 39/39. Kopf und Übersicht hängt der Runner selbst an, Rohausgabe `docs/beweise/roh/G3-SOAK-7386741.md` | `7386741`, nach dem Rebase `ef25b1c` |
| 2026-09-02 | Phase 2, Selbstprüfung am eigenen Diff: die Prüfpunktnamen `A24:<name>` standen nur im Manifest, nicht im Laufprotokoll; S10, S15 und S16 hatten keine eigene Urteilszeile; **S09 war gar nicht gemessen**. Alles ergänzt — S09 zählt jetzt, wie oft der Main im Neustartfenster ohne aktive Subscription gesehen wird | `9909722`, nach dem Rebase `7f92a82` |
| 2026-09-02 | Phase 2, Kanon nach dieser Nacharbeit: **GRUEN 40/40** auf `9909722`, Rohausgabe `docs/beweise/roh/G3-SOAK-9909722.md` | `9909722`, nach dem Rebase `f7b5056` |
| 2026-09-02 | **Abschlussprüfung Runde 1:** frischer Codex-Thread `01a0626a-234c-7831-9e4b-d2a4ae6426ca` (gpt-5.6-sol, Effort max, read-only) auf `4ae3ee6`, **NEEDS_WORK**, 18 Befunde. Der Dirigent bestätigte 17 als DEFEKT an der Quelle; der 18. (Frischeprüfung des Soak-Binaries im Runner, `tools/beweise.ps1:683-688`) ging als **Härtung** ins Register und blieb ausserhalb dieser Runde. **Ursache über allen 17:** Belegfelder wurden abgetastet oder synthetisiert statt am Ereignis erfasst, und der Prüfer übernahm Urteile beziehungsweise übersprang fehlende Einträge, statt jedes Urteil aus seinem Beleg abzuleiten | geprüft auf `4ae3ee6` |
| 2026-09-02 | **Nacharbeit Runde 1:** alle 17 Defekte geschlossen (§15). Jede Zusage wird an ihrem Ereignis erfasst — neue Handshakes `KILL_ERFOLGT` und `TOTZEIT_ENDE`/`TOTZEIT_ERFASST`, Snapshotprüfung bei jeder Übernahme, Reconnect je Clientpaar, Nachlauffenster vor dem Abbau; der Prüfer leitet jedes Urteil aus Belegen ab, verlangt Pflichtfelder und wird bei fehlenden Einträgen rot. Siebzehn neue Rotmutationen, eine je Defekt | Basis `4ae3ee6` |
| 2026-09-02 | Phase 1b: alle vier Befunde geschlossen — §3.1 rechnet jetzt Control- **und** Telemetrie-Runde und weist den Zuschlag zur 60-s-Frist aus (B2); §6 ersetzt „deterministisch" durch **Barriere** oder **wahrscheinlich** mit Belegfeld (B3, B4); S16 legt die Reihe 1/4/8/16/32 und die Regel `max(1, round(N · Anteil))` fest (B1). Kein Code geschrieben | Basis `f7a5125` |
| 2026-09-02 | **Wiederprüfung Runde 1:** frischer Codex-Thread `01a062b9-c5ec-7cb3-a2d5-4ac1012b8e22` (gpt-5.6-sol, Effort max, read-only) auf `882b2aa`. **16 der 17 Defekte als geschlossen bestätigt**, genau **ein Rest**: der K-S5-Sollwert wurde aus dem Bericht übernommen (`k_s5.erwartet`), statt unabhängig aus der geplanten Sondenzahl zu folgen — damit blieben sowohl `backoff_deckel_erreicht = erwartet = 0` mit `urteil = getroffen` als auch ein ganz fehlendes `erwartet` bei Exit 0 | geprüft auf `882b2aa` |
| 2026-09-02 | **Nacharbeit Runde 2:** der eine Rest geschlossen (§16). Der Prüfer rechnet den K-S5-Sollwert selbst als `--sonden + 1`, verlangt `k_s5.erwartet` als Pflichtfeld und hält es gegen die eigene Rechnung; die vier übrigen Killzeilen tragen keinen Sollwert und bekommen einen Riegel gegen einen künftigen. Drei In-Memory-Fälle vor und nach dem Fix, die 23. Rotmutation `k_s5_sollwert` und ein Lauf mit **N = 4** (Sollwert 5 statt 17) | Basis `882b2aa` |

## 13. Selbst gefahrene Laeufe (Phase 2, 02.09.2026)

Beide Laeufe auf dem Baustand `5a7670b` (nach dem Rebase `0ca3080`), Windows 11, Release. Der Prueferbericht
ist die einzige Quelle der Zahlen unten; er steht darunter roh.

| Lauf | Aufruf | Exit | Kernzahlen |
|---|---|---|---|
| Kanon-Kurzform | `--sonden 16 --minuten 2 --neustarts 1` | **0 (GRUEN)** | Topologie 82 ms · 485/485 Snapshots vollstaendig · 2.336/2.336 P0 beantwortet, p95 23,1 ms · 217.680 Ganzbloecke, 0 Drops · Neustart in 2.527 ms wieder vollstaendig · Client-RSS +0,8 MB |
| Killpunkt-Lauf | `--sonden 16 --minuten 5 --neustarts 2` | **0 (GRUEN)** | 1.210/1.210 Snapshots · 5.616/5.616 P0, p95 21,8 ms ausserhalb des Stoerfensters · 525.472 Ganzbloecke, 0 Drops · zwei Neustarts in 2.526 und 2.574 ms · **alle fuenf Killpunkte getroffen**, K-S5 mit 17/17 Control-Clients am Backoff-Deckel |

### 13.1 Was die Laeufe zu den Killpunkten sagen

| Killpunkt | Kurzform (1 Neustart) | Killpunkt-Lauf (2 Neustarts) |
|---|---|---|
| K-S1 Fehler auf offenem P2-Handle | getroffen (1) | getroffen (2) |
| K-S2 Kill waehrend `subscribe_session` | `nicht_gefahren` (braucht `--neustarts >= 2`) | **getroffen**, `snapshot_vor_kill` false |
| K-S3 Kill waehrend Heartbeat | getroffen (20 P0 ohne ACK im Fenster) | getroffen (356) |
| K-S4 Kill in der Leserverzoegerung | getroffen (4 Sonden mit gesetztem Flag) | getroffen (6) |
| K-S5 Kill aus dem Backoff-Deckel | `nicht_gefahren` | **getroffen**, 17/17 Control-Clients mit mindestens 5 Verbindungsversuchen in 21 s Totzeit |

Die Kanon-Kurzform faehrt K-S2 und K-S5 nicht; das steht so in der Killmatrix
(§6) und im Bericht als `nicht_gefahren` — nie als Erfolg. Der Gate-Lauf mit
`--neustarts 3` deckt beide.

> **Nachtrag 02.09.2026 (Nacharbeit Runde 1, §15).** Die beiden K-S3-Zahlen in
> dieser Tabelle — 20 und 356 — sind **widerlegt**. Sie zählten Heartbeats, die
> zum Zeitpunkt von `bereitWieder()` noch unbeantwortet waren und ihr ACK später
> sehr wohl bekamen; der Schlussbericht derselben Läufe trug
> `verloren_im_neustartfenster = 0`. Der Verlauf oben bleibt stehen, wie er
> gemessen wurde; die richtige Lesart steht in §15.5. Auch die Zahlen unter
> `langsam.*` in §13.3 stammen aus der Summe über **alle** Sonden und enthalten
> damit den K-S1-Fluter; seit Runde 1 sind beide Grössen getrennt (§15, Defekt 5).

### 13.2 Was die Laeufe NICHT hergeben

- **`p0.latenz_max_ms` liegt ueber der Schranke** (Kurzform 2.748,7 ms,
  Killpunkt-Lauf 2.223,3 ms), waehrend p95 bei rund 22 ms liegt. Die Zusage Z4
  ist auf p95 formuliert; der Maximalwert entsteht bei den **langsamen** Sonden,
  deren Lesethread hinter der eigenen kuenstlichen Bremse steht. Das ist kein
  Produktbefund, sondern die gewollte Eigenschaft eines langsamen Lesers — die
  Zahl steht trotzdem im Bericht, statt weggelassen zu werden.
- **Die Laufzeiten sind 2 und 5 Minuten, nicht 60.** Der Gate-Lauf ueber
  60 Minuten und die Skalierungsreihe aus S16 sind ein eigener Schritt im
  G3-Gate, nicht Teil dieses Werkzeugtickets (§11).
- **Kein XRun.** Unveraendert §2.2.

### 13.3 Rohe Berichte

Kanon-Kurzform, `--sonden 16 --minuten 2 --neustarts 1`, Exit 0:

```json
{"audio": {"bloecke": 217680, "ganzblockdrops_oversize": 0, "ganzblockdrops_ueberlauf": 0, "kontinuitaetsbrueche": 0, "publikationen": 21760}, "client_exit": 0, "clientpaare": 17, "kill": {"k_s1": {"telemetrie_handle_fehler": 1, "urteil": "getroffen"}, "k_s2": {"snapshot_vor_kill": false, "urteil": "nicht_gefahren"}, "k_s3": {"p0_ohne_ack_im_fenster": 20, "urteil": "getroffen"}, "k_s4": {"flag_zum_killzeitpunkt": 4, "urteil": "getroffen"}, "k_s5": {"backoff_deckel_erreicht": 0, "erwartet": 17, "urteil": "nicht_gefahren"}}, "langsam": {"abgelehnt": 0, "blockiert_andere_nicht": true, "ersetzte_liveframes": 438, "immer_mitglied": true, "kollisionsloecher": 0, "neueste_verworfen": 0, "schnelle_p95_ms": 21.4, "sonden": 4, "zu_gross": 0}, "langsam_anzahl": 4, "liveness": {"evicted_ausserhalb_neustart": 0, "stale_ausserhalb_neustart": 0, "stale_im_neustartfenster": 0}, "minuten": 2, "mitgliedschaft": {"fremde_adresse": 0, "fuehrendes_main_falsch": 0, "snapshot_pruefungen": 485, "vollstaendig": 485}, "neustart": [{"alte_epoche_nach_neustart_gesehen": 0, "bereit_bis_vollstaendig_ms": 2527, "epoch_alt": "1634fc8ea9d74dc8a0c4b77a37e70a9a", "epoch_neu": "25ff7ee38211404182e29b74e18c882b", "frist_ms": 60000, "index": 1, "reconnect_ms": {"max": 2527.0, "min": 2527.0, "p95": 2527.0}, "schranke_ms": 45051, "totzeit_ms": 1008, "ueber_schranke": false}], "p0": {"beantwortet": 2336, "gesendet": 2336, "latenz_max_ms": 2748.7, "latenz_p95_ms": 23.1, "schranke_ms": 1000, "verloren_ausserhalb_neustart": 0, "verloren_im_neustartfenster": 0}, "pipe": {"fremder_name_versucht": 0}, "sonden": 16, "speicher": {"broker": [{"generation": 0, "im_neustartfenster": false, "minute": 0, "rss_bytes": 12984320}, {"generation": 0, "im_neustartfenster": false, "minute": 1, "rss_bytes": 13070336}, {"generation": 1, "im_neustartfenster": true, "minute": 1, "rss_bytes": 12738560}, {"generation": 1, "im_neustartfenster": false, "minute": 2, "rss_bytes": 13312000}, {"generation": 1, "im_neustartfenster": false, "minute": 3, "rss_bytes": 13312000}], "budget_bytes": 16777216, "budget_prozent": 10, "client": [{"im_neustartfenster": false, "minute": 0, "rss_bytes": 110403584}, {"im_neustartfenster": false, "minute": 1, "rss_bytes": 110436352}, {"im_neustartfenster": true, "minute": 1, "rss_bytes": 111198208}, {"im_neustartfenster": false, "minute": 2, "rss_bytes": 111198208}, {"im_neustartfenster": false, "minute": 3, "rss_bytes": 111198208}], "takt_s": 60}, "topologie_ms": 82, "warmup_s": 20}
```

Killpunkt-Lauf, `--sonden 16 --minuten 5 --neustarts 2`, Exit 0:

```json
{"audio": {"bloecke": 525472, "ganzblockdrops_oversize": 0, "ganzblockdrops_ueberlauf": 0, "kontinuitaetsbrueche": 0, "publikationen": 52544}, "client_exit": 0, "clientpaare": 17, "kill": {"k_s1": {"telemetrie_handle_fehler": 2, "urteil": "getroffen"}, "k_s2": {"snapshot_vor_kill": false, "urteil": "getroffen"}, "k_s3": {"p0_ohne_ack_im_fenster": 356, "urteil": "getroffen"}, "k_s4": {"flag_zum_killzeitpunkt": 6, "urteil": "getroffen"}, "k_s5": {"backoff_deckel_erreicht": 17, "erwartet": 17, "urteil": "getroffen"}}, "langsam": {"abgelehnt": 0, "blockiert_andere_nicht": true, "ersetzte_liveframes": 4362, "immer_mitglied": true, "kollisionsloecher": 0, "neueste_verworfen": 0, "schnelle_p95_ms": 21.2, "sonden": 4, "zu_gross": 0}, "langsam_anzahl": 4, "liveness": {"evicted_ausserhalb_neustart": 0, "stale_ausserhalb_neustart": 0, "stale_im_neustartfenster": 0}, "minuten": 5, "mitgliedschaft": {"fremde_adresse": 0, "fuehrendes_main_falsch": 0, "snapshot_pruefungen": 1210, "vollstaendig": 1210}, "neustart": [{"alte_epoche_nach_neustart_gesehen": 0, "bereit_bis_vollstaendig_ms": 2526, "epoch_alt": "d702aeabed80496790a9c928432ad933", "epoch_neu": "afc28ce6138d4af3b47933875ee03092", "frist_ms": 60000, "index": 1, "reconnect_ms": {"max": 2526.0, "min": 2526.0, "p95": 2526.0}, "schranke_ms": 45051, "totzeit_ms": 1006, "ueber_schranke": false}, {"alte_epoche_nach_neustart_gesehen": 0, "bereit_bis_vollstaendig_ms": 2574, "epoch_alt": "afc28ce6138d4af3b47933875ee03092", "epoch_neu": "f425c72cf1184e4cb5a52e38370f7a59", "frist_ms": 60000, "index": 2, "reconnect_ms": {"max": 2574.0, "min": 2574.0, "p95": 2574.0}, "schranke_ms": 45051, "totzeit_ms": 21007, "ueber_schranke": false}], "p0": {"beantwortet": 5616, "gemessen_ausserhalb": 5136, "gemessen_im_stoerfenster": 480, "gesendet": 5616, "latenz_max_im_stoerfenster_ms": 23208.8, "latenz_max_ms": 2223.3, "latenz_p95_ms": 21.8, "schranke_ms": 1000, "verloren_ausserhalb_neustart": 0, "verloren_im_neustartfenster": 0}, "pipe": {"fremder_name_versucht": 0}, "sonden": 16, "speicher": {"broker": [{"generation": 0, "im_neustartfenster": false, "minute": 0, "rss_bytes": 12902400}, {"generation": 0, "im_neustartfenster": false, "minute": 1, "rss_bytes": 13004800}, {"generation": 1, "im_neustartfenster": true, "minute": 1, "rss_bytes": 12513280}, {"generation": 1, "im_neustartfenster": false, "minute": 2, "rss_bytes": 13127680}, {"generation": 1, "im_neustartfenster": false, "minute": 3, "rss_bytes": 13283328}, {"generation": 2, "im_neustartfenster": true, "minute": 3, "rss_bytes": 12365824}, {"generation": 2, "im_neustartfenster": false, "minute": 4, "rss_bytes": 13189120}, {"generation": 2, "im_neustartfenster": false, "minute": 5, "rss_bytes": 13402112}, {"generation": 2, "im_neustartfenster": false, "minute": 6, "rss_bytes": 13402112}], "budget_bytes": 16777216, "budget_prozent": 10, "client": [{"im_neustartfenster": false, "minute": 0, "rss_bytes": 111067136}, {"im_neustartfenster": false, "minute": 1, "rss_bytes": 111169536}, {"im_neustartfenster": true, "minute": 1, "rss_bytes": 111476736}, {"im_neustartfenster": false, "minute": 2, "rss_bytes": 111476736}, {"im_neustartfenster": false, "minute": 3, "rss_bytes": 111476736}, {"im_neustartfenster": true, "minute": 3, "rss_bytes": 111960064}, {"im_neustartfenster": false, "minute": 4, "rss_bytes": 111964160}, {"im_neustartfenster": false, "minute": 5, "rss_bytes": 112099328}, {"im_neustartfenster": false, "minute": 6, "rss_bytes": 112099328}], "takt_s": 60}, "topologie_ms": 81, "warmup_s": 20}
```

## 14. Rotmutationen — jede Wache einmal absichtlich gebrochen

Ein Bein, das nie rot werden kann, beweist nichts (Prueflistenzeile E). Der
Pruefer traegt dafuer `--mutant <name>`; jeder Mutant verfaelscht **genau eine**
Groesse und laesst alles andere unberuehrt. `--mutant-liste` druckt sie.

| Mutant | Was verfaelscht wird | Gemessenes Ergebnis | Exit |
|---|---|---|---|
| `s02` | erwartet eine Sonde mehr, als gefahren wird | `ROT [S02] der Lauf traegt genau 5 Sonden [4]` | **2** |
| `s04` | P0-Latenzschranke auf 0 ms | `ROT [S04] P0-ACK p95 unter 0 ms [p95 20,0 ms, max 120,1 ms]` | **2** |
| `s07` | Wachstumsbudget auf 0 | `ROT [S07] Client-Working-Set bleibt im Budget [37,8 -> 38,2 MB, +0,4 MB, Grenze 0,0 MB]` | **2** |
| `s11` | Neustartfrist auf 0 ms | `ROT [S01] die Topologie stand in der Frist (0 ms) [66 ms]` | **2** |
| `s12` | gemessener Zaehler `alte_epoche_nach_neustart_gesehen` auf 1 | `ROT [S12] Neustart 1: die alte broker_epoch kam nie wieder [1]` | **2** |
| `s15` | Produktions-Pipename statt Probe-Pipe | beide Programme melden `Exit 3`; der Pruefer endet mit Voraussetzung-fehlt | **3** |

Alle sechs am 02.09.2026 auf `5a7670b` (nach dem Rebase `0ca3080`) gefahren (`--sonden 4 --minuten 1
--neustarts 1`, `s15` ohne Lauf). Ein Mutant, der gruen bliebe, waere ein
Befund am Bein.


## 15. Nacharbeit Runde 1 — die 17 Defekte der Abschlussprüfung

Die Abschlussprüfung durch einen lesenden Codex-Thread
(`01a0626a-234c-7831-9e4b-d2a4ae6426ca`, gpt-5.6-sol, Effort max, read-only,
Stand `4ae3ee6`) endete mit **NEEDS_WORK** und 18 Befunden. Der Dirigent hat 17
davon an der Quelle als **DEFEKT** bestätigt; der 18. — die Frischeprüfung des
Soak-Binaries im Runner (`tools/beweise.ps1:683-688`) — ging als **Härtung** ins
Register und war nicht Teil dieser Runde.

### 15.1 Die eine Ursache

Die 17 Defekte sind nicht 17 Einzelfehler, sondern zwei Muster:

1. **Belege entstanden neben ihrem Ereignis, nicht an ihm.** K-S2 und K-S4
   wurden **vor** `KILL_BEREIT` erfasst — also bevor der Broker starb. K-S5
   wurde **nach** `BEREIT` gelesen — also gegen den *neuen* Broker. K-S3 zählte
   in `bereitWieder()`, was zu diesem frühen Zeitpunkt unbeantwortet war; später
   eintreffende ACKs widerlegten den Treffer. S03 tastete das Modell alle 250 ms
   ab, statt jede Übernahme zu prüfen. S11 trug N Kopien desselben
   Topologieendes statt einer Verteilung. S06 synthetisierte `abgelehnt` aus
   `zuGross + beanspruchtVerworfen`, statt den Rückgabewert zu lesen — und
   summierte die Cap-Zähler des K-S1-Fluters unter `langsam`. S10 hielt einen
   Zähler, den kein Aufruf setzen konnte.
2. **Der Prüfer übernahm Urteile oder übersprang fehlende Einträge.** Die
   Killurteile wurden gedruckt, nicht nachgerechnet. Die S11/S12-Prüfungen waren
   eine Schleife über die *gelieferten* Neustarts — bei leerer Liste lief sie
   null Mal und blieb grün. Eine unterabgetastete Brokergeneration endete in
   einem `continue`. `audio.publikationen` und die S16-Skalierungsformel wurden
   gar nicht gewertet. Ein Orchestrierungs-Timeout verliess das Werkzeug mit
   Exit 1 ohne Urteilsbericht.

Der Auftrag dieser Runde war deshalb keine Liste von Punktkorrekturen, sondern
**jede Zusage an ihrem Ereignis erfassen und jedes Urteil aus seinem Beleg
ableiten**. Konkret:

- vier neue Handshakes im Protokoll (`KILL_ERFOLGT`, `TOTZEIT_ENDE` →
  `TOTZEIT_ERFASST`) und ein Nachlauffenster vor dem Abbau, damit jeder Beleg zu
  seinem Ereignis gehört (§6, Zeitpunkttabelle);
- ein Prüfer, der Pflichtfelder verlangt, bei fehlenden rot wird und jedes
  `kill.*.urteil` aus seinem Belegfeld neu ableitet (§7).

### 15.2 Die 17 Defekte und ihre Regel

| # | Defekt | Regel, die ihn schliesst | Wo |
|---|---|---|---|
| 1 | K-S3 wies 89 Treffer aus, während der Schlussbericht `verloren_im_neustartfenster = 0` trug | K-S3 folgt **nach dem Nachlauffenster** allein aus `p0.verloren_im_neustartfenster`; der Prüfer hält beide Zahlen gegeneinander | `SessionSoakMain.cpp` `killBericht (verlorenFenster)`; `pruefe_session_soak.py` `killurteile()` |
| 2 | am Laufende entstand eine zweite Speicherprobe mit identischem RSS | eine Endprobe nur nach ≥ 30 s; jede Probe trägt `sekunden`, und erster zu letztem gültigen Punkt müssen ≥ 30 s auseinanderliegen | `fahre()`, `urteile()` |
| 3 | eine Brokergeneration mit einem Messpunkt endete in `continue` und blieb grün | weniger als zwei Messpunkte ist **unzureichend und nicht bestanden** — eigener roter Prüfpunkt, Exit 2 | `urteile()` |
| 4 | beide Rückgabewerte von `veroeffentlichen()` wurden verworfen, `abgelehnt` war synthetisch | `Sonde::publiziere()`/`publiziereRoh()` zählen jeden Rückgabewert; `abgelehnt` kommt aus der Rückgabe | `SessionSoakMain.cpp` |
| 5 | die Cap-Zähler unter `langsam` summierten über **alle** Sonden, also auch den Fluter | `langsam.*` nur über die langsamen Sonden, `fluter.*` getrennt; die langsamen Sonden überproduzieren `kLangsamBurst = 3` je Takt, damit ihre eigene Schleuse wirklich vollläuft | `bericht()`, `liveLauf()` |
| 6 | Snapshot-Rückgabewert und die entstandene Sicht wurden nicht geprüft, nur alle 250 ms abgetastet | jede Übernahme wird sofort geprüft (`snapshot_pruefungen`); die Abtastung bleibt zusätzlich (`abtast_*`); `ungueltig` wird gezählt | `mainText()`, `pruefeSicht()` |
| 7 | S08 wertete `audio.publikationen` nicht — ein stehengebliebener Analyseworker blieb grün | `publikationen > 0` ist Pflicht, und `publikationen_je_sonde_min` muss die aus `kLiveIntervallS` gerechnete Erwartung erreichen | `bericht()`, `urteile()` |
| 8 | S09 zählte nur über alle Fenster und nur `subscriptionAktiv == false` | je Kill werden **beide** verlangt: `s09_subscription_weg > 0` und `s09_quellen_getrennt > 0` | `wacheLauf()`, `urteile()` |
| 9 | `fremderPipename` wurde nie gesetzt — ein Zähler, den kein Aufruf erreichen kann | drei Belege: Erlaubnisliste an jeder der 2·(N+1) Übergaben, Exit 3 ohne `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3`, und die Namensraumauflistung des Prüfers (NAK-118) | `clientPipe()`, `main()`, `pipe_namensraum()` |
| 10 | `reconnect_ms` trug N Kopien desselben Topologieendes | Reconnect je Clientpaar, Main plus jede Sonde, vom `BEREIT` bis zum eigenen Wieder-Mitglied-Zeitpunkt; `reconnect_paare` belegt die Zahl | `bereitWieder()` |
| 11 | K-S2 und K-S4 wurden **vor** dem Kill erfasst | neuer Handshake `KILL_ERFOLGT`; das Programm stempelt den Killzeitpunkt und vergleicht Snapshot-Ankunft und Verzögerungsstempel dagegen | `killErfolgt()` |
| 12 | K-S5 wurde **nach** dem Neustart gelesen | neuer Handshake `TOTZEIT_ENDE` → `TOTZEIT_ERFASST`; K-S1 und K-S5 werden erfasst, bevor der Prüfer den neuen Broker startet | `totzeitEnde()`, `fahre()` |
| 13 | S16 verlangte nur `sonden >= 1` | `langsam_anzahl == max(1, round(N · Anteil))` wird gerechnet (`floor(x+0.5)`, wie `std::lround`), und bei N = 1 werden die `nicht_anwendbar`-Felder verlangt | `urteile()` |
| 14 | der Prüfer druckte das gelieferte `kill.*.urteil` | er leitet jedes Urteil aus seinem Belegfeld neu ab, verlangt alle fünf Pflichteinträge und wird bei Widerspruch rot | `killurteile()` |
| 15 | S11/S12 waren eine Schleife über die gelieferten Neustarts | `len(neustart) == args.neustarts` ist ein eigener Prüfpunkt | `urteile()` |
| 16 | `abbauen()` lief unmittelbar nach `ENDE` | Nachlauffenster `kNachlaufMs` = Heartbeat-Kadenz + P0-Schranke = 2.000 ms; erst danach heisst ein offener ACK Verlust | `main()`, `heartbeatLauf()` |
| 17 | `TimeoutExpired` verliess das Werkzeug mit Exit 1 ohne Bericht | jeder Abbau-Hänger endet mit Urteilsbericht und Exit 2, jedes fehlende `BEREIT` mit Exit 3 | `fahre()`, `main()` |

### 15.3 Rotmutationen — eine je Defekt

Der Prüfer trägt seit dieser Runde `--bericht <datei>`: er fährt dann **nur**
`urteile()` über einen gespeicherten Lauf, ohne Soak. Damit lässt sich jeder
Mutant an genau demselben echten Lauf zeigen, statt je Mutant weitere Minuten zu
fahren. Der gespeicherte Lauf ist
[`roh/G3-SOAK-nacharbeit-1-kanon-bericht.json`](roh/G3-SOAK-nacharbeit-1-kanon-bericht.json)
(die Kanon-Kurzform aus §15.4). Der Aufruf lautet:

```powershell
py -3.13 tools/eq-copilot/pruefe_session_soak.py `
    --bericht docs/beweise/roh/G3-SOAK-nacharbeit-1-kanon-bericht.json `
    --sonden 16 --neustarts 1 --mutant <name>
```

| # | Mutant | Gemessene rote Zeile | Exit |
|---|---|---|---|
| 1 | `k_s3_beleg` | `ROT [S04] K-S3 zaehlt genau die endgueltig im Fenster verlorenen ACKs [7 vs. 0]` | **2** |
| 2 | `speicherluecke` | `ROT [S07] Client: die Kurve traegt echte Minutenabstaende [0 s ... (mindestens 30 s)]` | **2** |
| 3 | `s03` | `ROT [S07] Broker Generation 1: mindestens zwei Messpunkte ausserhalb der Neustartfenster [1 von 2 Punkten — unzureichend ist nicht bestanden]` | **2** |
| 4 | `s06_grund` | `ROT [S06] jede abgelehnte Veroeffentlichung hat einen gezaehlten Grund [1 abgelehnt (aus dem Rueckgabewert) = 0 + 0]` | **2** |
| 5 | `s06_fluter` | `ROT [S06] die Schleuse der langsamen Leser lag wirklich im Rueckstau [0 ersetzt bei 17664 Veroeffentlichungen (Fluter getrennt: 178)]` | **2** |
| 6 | `s03_sofort` | `ROT [S03] jede Snapshot-Uebernahme war sofort vollstaendig und richtig [3918/3919 bei 4645 Uebernahmen]` | **2** |
| 7 | `s08` | `ROT [S08] jede Sonde hat je Analysefenster einen Producer-Frame veroeffentlicht [0 gesamt, kleinste Sonde 0 >= 1103 (13792 Bloecke / 10 je Fenster, Toleranz 0.8)]` | **2** |
| 8 | `s09_getrennt` | `ROT [S09] Neustart 1: Subscription weg UND Quellen getrennt [15 ohne Subscription, 0 mit getrennter Quelle]` | **2** |
| 9 | `s10_define` | `ROT [S10] kein Client bekam je einen anderen Pipenamen [... ohne_produkt_v3 False]` und `ROT [S10] im Pipe-Namensraum entstand kein Produktions- oder Goldenname [... produktionsname_neu True, 170 -> 172 Namen]` | **2** |
| 10 | `s11_paare` | `ROT [S11] Neustart 1: Reconnect je Clientpaar gemessen [1 Paare (erwartet 17), min 538.8 / p95 2517.0 / max 2517.0 ms]` | **2** |
| 11 | `k_s4_widerspruch` | `ROT [k_s4] getroffen (abgeleitet: nicht_getroffen) {'flag_zum_killzeitpunkt': 0, 'gefahren': True}` | **2** |
| 12 | `k_s5_totzeit` | `ROT [S09] Neustart 1: Kill und Totzeit wurden belegt [kill_erfolgt True, totzeit_erfasst False]` | **2** |
| 13 | `s16` | `ROT [S16] langsam_anzahl folgt max(1, round(N x Anteil)) [N = 16, langsam_anzahl 0, erwartet 4]` | **2** |
| 14 | `kill` | `ROT [k_s1] Pflichteintrag fehlt im Bericht` (und dieselbe Zeile für k_s2…k_s5, dazu `ROT [S04] K-S3 traegt seinen Beleg [fehlt]`) — 6 rote Zeilen | **2** |
| 15 | `neustart` | `ROT [S11] der Bericht traegt genau 1 Neustart(e) [0 geliefert]` | **2** |
| 16 | `s14_nachlauf` | `ROT [S14] vor dem Abbau lief ein Nachlauffenster fuer ausstehende ACKs [0 ms]` | **2** |
| 17 | `s14` | `ROT [S14] das C++-Programm endet sauber [Exit None, kuenstlicher Abbau-Haenger]` | **2** |

Die Gegenprobe ohne Mutant über denselben Bericht endet mit **GRUEN, Exit 0**.
Die fünf berichtsbasierten Mutanten aus Phase 2 (`s02`, `s04`, `s07`, `s11`,
`s12`) laufen ebenfalls über `--bericht` und bleiben rot — zusammen sind es
**22 Rotmutationen über denselben Lauf**. Der sechste Phase-2-Mutant `s15`
braucht weiterhin einen echten Lauf, weil er die beiden Binaries fährt.

**Ein Rotbeweis hat dabei einen Fehler im Prüfer selbst gefunden.** `--mutant
k_s3_beleg` druckte die rote Zeile, endete aber mit Exit 0:
`fehler += killurteile(bericht, args, pruefe)` lädt in CPython die linke Seite
**vor** dem Aufruf, sodass der Zuwachs, den `killurteile` über die
`nonlocal`-Closure von `pruefe` erzeugt, beim Zurückschreiben verlorenging.
Sichtbar wurde das nur, weil die Kreuzprobe der einzige Fehler war. Behoben
durch zwei getrennte Anweisungen; der Kommentar an der Stelle nennt den Grund.

### 15.4 Die beiden Beweisläufe

Beide auf dem Baustand dieser Runde, Windows 11, Release. Der Prüferbericht ist
die einzige Quelle der Zahlen; er steht in §15.7 roh.

| Lauf | Aufruf | Exit | Kernzahlen |
|---|---|---|---|
| Kanon-Kurzform | `--sonden 16 --minuten 2 --neustarts 1` | **0 (GRUEN)** | Topologie 83 ms · **3.919/3.919 Snapshot-Übernahmen sofort vollständig** bei 4.645 Übernahmen, 0 ungültig · 1.228/1.228 Abtastungen · 2.336/2.336 P0 beantwortet, p95 22,2 ms · 220.672 Ganzblöcke, 0 Drops, kleinste Sonde 1.379 Publikationen · Reconnect je Paar min 538,8 / p95 2.517 / max 2.517 ms über **17 Paare** · Client-RSS +0,6 MB |
| Killpunkt-Lauf | `--sonden 16 --minuten 5 --neustarts 2` | **0 (GRUEN)** | Topologie 81 ms · **9.723/9.723 Snapshot-Übernahmen sofort vollständig** bei 10.508 Übernahmen, 0 ungültig · 3.033/3.033 Abtastungen · 5.616/5.616 P0 beantwortet, p95 21,6 ms · 528.432 Ganzblöcke, 0 Drops, kleinste Sonde 3.302 Publikationen · langsame Sonden 16.171 ersetzte Liveframes bei 42.276 Veröffentlichungen (Fluter getrennt 577) · zwei Neustarts in 2.513 und 2.543 ms · **alle fünf Killpunkte gefahren**, K-S5 mit 17/17 Control-Clients am Backoff-Deckel, in 21,0 s Totzeit erfasst |

### 15.5 Was diese Läufe zu den Killpunkten sagen

| Killpunkt | Kurzform (1 Neustart) | Killpunkt-Lauf (2 Neustarts) |
|---|---|---|
| K-S1 Fehler auf offenem P2-Handle | getroffen (1) | getroffen (6) |
| K-S2 Kill während `subscribe_session` | `nicht_gefahren` (braucht `--neustarts >= 2`) | **getroffen**, `snapshot_vor_kill` false — gemessen am Killzeitpunkt |
| K-S3 Kill während Heartbeat | **`nicht_getroffen` (0)** | **`nicht_getroffen` (0)** |
| K-S4 Kill in der Leserverzögerung | getroffen (4) | getroffen (4) |
| K-S5 Kill aus dem Backoff-Deckel | `nicht_gefahren` | **getroffen**, 17/17 Control-Clients, in 21,0 s Totzeit erfasst |

**K-S3 ist der ehrliche Verlierer dieser Runde und der wichtigste Befund am
Bein.** Phase 2 meldete 20 beziehungsweise 356 Treffer. Diese Zahlen waren
falsch: sie zählten Heartbeats, die zum Zeitpunkt von `bereitWieder()` noch
unbeantwortet waren — und später sehr wohl ihr ACK bekamen. Der Schlussbericht
derselben Läufe trug `verloren_im_neustartfenster = 0`; genau diesen Widerspruch
hat die Abschlussprüfung gefunden. Seit dieser Runde folgt K-S3 aus dem
endgültigen Verlust, und der Prüfer hält beide Zahlen gegeneinander.

Das Ergebnis ist: **K-S3 wurde bisher nie getroffen.** Der Grund ist an der
Quelle messbar und kein Werkzeugfehler. `ControlClient::sendeP0` reiht **an der
Tür** ein, nicht am Draht: die Nachricht geht in die P0-Queue, solange sie nicht
über der Paketgrenze liegt und die Queue nicht überläuft
(`core/ipc/ControlClient.cpp:785-818`) — der Verbindungszustand spielt dort keine
Rolle. Ein während der Totzeit eingereihter Heartbeat **überlebt den Neustart in
der Queue** und wird nach dem Reconnect gesendet und beantwortet. Der
Killpunkt-Lauf belegt das direkt: `p0.latenz_max_im_stoerfenster_ms` beträgt
**23.176,3 ms** bei 21,0 s Totzeit, und `gesendet` gleich `beantwortet` (5.616)
— kein einziger ACK ging verloren. In der Kurzform sind es 2.760,5 ms.

Ein endgültig verlorener ACK setzt deshalb voraus, dass der Broker genau in dem
schmalen Fenster stirbt, in dem ein Heartbeat schon auf dem Draht ist: rund 20 ms
ACK-Umlaufzeit gegen eine Kadenz von 1.000 ms. Ein Kill auf fester Wanduhr trifft
das selten — genau deshalb klassifiziert §6 K-S3 als **wahrscheinlich
(beobachtet)**, nicht als Barriere. K-S3 bleibt damit ein offener Killpunkt und
wird im G3-Gate-Manifest als solcher geführt, nie als bestanden.

### 15.6 Was auch diese Runde nicht hergibt

- **Die Exit-3-Wache für das fehlende Define (Defekt 9) ist nicht gefahren.**
  Sie zu fahren hiesse, ein zweites CMake-Ziel ohne
  `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` zu bauen — das läge ausserhalb der
  Ticketgrenze und führte eine Testbinärdatei an die Produktions-Pipe. Belegt
  sind die beiden anderen Hälften: der Berichtszähler `pipe.ohne_produkt_v3` und
  die Namensraummessung, beide mit `--mutant s10_define` gebrochen.
- **Die Vorbedingung von K-S1 ist seit dieser Runde weniger trennscharf.** Weil
  die langsamen Sonden jetzt selbst überproduzieren (Defekt 5), erfüllen auch
  sie `vorKillRueckstau`. Das erzeugt keinen falschen Treffer — entscheidend
  bleiben der Fehler auf dem offenen Handle und der unveränderte
  `verbindungsVersuche` —, aber die Vorbedingung unterscheidet nicht mehr
  zwischen Fluter und langsamem Leser.
- **Die Reconnect-Verteilung trennt in der Praxis nur zwei Gruppen.** Eine Sonde
  wird wieder Mitglied, sobald der Coordinator EINEN absoluten Snapshot mit allen
  Deskriptoren pusht — alle Sondenpaare fallen deshalb auf denselben 25-ms-
  Abtastschritt. Neustart 1 zeigt das als min 538,8 ms (Main-Paar) gegen max
  2.517,0 ms (Sondengruppe) in der Kurzform und min 537,8 gegen max 2.513,3 ms im
  Killpunkt-Lauf; bei dessen Neustart 2 fiel auch das Main-Paar in denselben
  Schritt, `min` und `max` sind dort beide 2.544,0 ms. Das ist die Messung, kein wiederholter Wert: die
  Zahl der gemessenen Paare steht als `reconnect_paare` daneben und fällt mit
  `--mutant s11_paare`.
- **Die Laufzeiten sind 2 und 5 Minuten, nicht 60.** Unverändert §13.2.
- **Kein XRun.** Unverändert §2.2.

### 15.7 Rohe Berichte

Kanon-Kurzform, `--sonden 16 --minuten 2 --neustarts 1`, Exit 0 (auch als
[`roh/G3-SOAK-nacharbeit-1-kanon-bericht.json`](roh/G3-SOAK-nacharbeit-1-kanon-bericht.json),
weil die Rotmutationen in §15.3 ihn über `--bericht` wiederverwenden):

```json
{"abbau_haenger": "", "audio": {"blockgroesse": 512, "bloecke": 220672, "bloecke_je_sonde_min": 13792, "ganzblockdrops_oversize": 0, "ganzblockdrops_ueberlauf": 0, "kontinuitaetsbrueche": 0, "publikationen": 22064, "publikationen_je_sonde_min": 1379, "samplerate": 48000.0}, "client_exit": 0, "clientpaare": 17, "fluter": {"abgelehnt": 0, "ersetzte_liveframes": 178, "kollisionsloecher": 0, "neueste_verworfen": 0, "veroeffentlichungen": 1652, "zu_gross": 0}, "kill": {"k_s1": {"gefahren": true, "telemetrie_handle_fehler": 1, "urteil": "getroffen"}, "k_s2": {"gefahren": false, "snapshot_vor_kill": false, "urteil": "nicht_gefahren"}, "k_s3": {"gefahren": true, "p0_ohne_ack_im_fenster": 0, "urteil": "nicht_getroffen"}, "k_s4": {"flag_zum_killzeitpunkt": 4, "gefahren": true, "urteil": "getroffen"}, "k_s5": {"backoff_deckel_erreicht": 0, "erwartet": 17, "gefahren": false, "urteil": "nicht_gefahren"}}, "langsam": {"abgelehnt": 0, "blockiert_andere_nicht": true, "ersetzte_liveframes": 6139, "fluter_ueberlappt": false, "immer_mitglied": true, "kollisionsloecher": 0, "neueste_verworfen": 0, "schnelle_p95_ms": 21.6, "sonden": 4, "veroeffentlichungen": 17664, "zu_gross": 0}, "langsam_anzahl": 4, "liveness": {"evicted_ausserhalb_neustart": 0, "stale_ausserhalb_neustart": 0, "stale_im_neustartfenster": 0}, "minuten": 2, "mitgliedschaft": {"abtast_pruefungen": 1228, "abtast_vollstaendig": 1228, "fremde_adresse": 0, "fuehrendes_main_falsch": 0, "snapshot_pruefungen": 3919, "uebernahmen": 4645, "ungueltig": 0, "vollstaendig": 3919}, "neustart": [{"alte_epoche_nach_neustart_gesehen": 0, "backoff_deckel_in_totzeit": 0, "bereit_bis_vollstaendig_ms": 2517, "epoch_alt": "7772ffa81977416d890ba88edc5f630f", "epoch_neu": "7204a0ddba86403584e66226cd53a4c9", "frist_ms": 60000, "index": 1, "kill_dauer_ms": 3, "kill_erfolgt_gesehen": true, "reconnect_ms": {"max": 2517.0, "min": 538.8, "p95": 2517.0}, "reconnect_paare": 17, "s09_quellen_getrennt": 15, "s09_sichten": 35, "s09_subscription_weg": 15, "schranke_ms": 45051, "totzeit_erfasst": true, "totzeit_ms": 1008, "ueber_schranke": false}], "p0": {"beantwortet": 2336, "gemessen_ausserhalb": 2256, "gemessen_im_stoerfenster": 80, "gesendet": 2336, "latenz_max_im_stoerfenster_ms": 2760.5, "latenz_max_ms": 119.2, "latenz_p95_ms": 22.2, "nachlauf_ms": 2000, "schranke_ms": 1000, "verloren_ausserhalb_neustart": 0, "verloren_im_neustartfenster": 0}, "pipe": {"clientnamen_geprueft": 34, "eigene_probe_sichtbar": true, "fremder_name_versucht": 0, "goldenname_neu": false, "namen_nach": 172, "namen_vor": 170, "namensraum_lesbar": true, "ohne_produkt_v3": true, "produktionsname_neu": false}, "riegel": {"golden_EqCopSessionSoak": 3, "golden_eqcop-broker-sonde012-probe": 3, "produktion_EqCopSessionSoak": 3, "produktion_eqcop-broker-sonde012-probe": 3}, "sonden": 16, "speicher": {"broker": [{"generation": 0, "im_neustartfenster": false, "minute": 0, "rss_bytes": 13008896, "sekunden": 0.0}, {"generation": 0, "im_neustartfenster": false, "minute": 1, "rss_bytes": 13148160, "sekunden": 60.0}, {"generation": 1, "im_neustartfenster": true, "minute": 1, "rss_bytes": 5459968, "sekunden": 60.0}, {"generation": 1, "im_neustartfenster": false, "minute": 1, "rss_bytes": 12763136, "sekunden": 60.0}, {"generation": 1, "im_neustartfenster": false, "minute": 2, "rss_bytes": 13299712, "sekunden": 120.1}], "budget_bytes": 16777216, "budget_prozent": 10, "client": [{"im_neustartfenster": false, "minute": 0, "rss_bytes": 110141440, "sekunden": 0.0}, {"im_neustartfenster": false, "minute": 1, "rss_bytes": 110333952, "sekunden": 60.0}, {"im_neustartfenster": true, "minute": 1, "rss_bytes": 110362624, "sekunden": 60.0}, {"im_neustartfenster": false, "minute": 1, "rss_bytes": 111116288, "sekunden": 60.0}, {"im_neustartfenster": false, "minute": 2, "rss_bytes": 111116288, "sekunden": 120.1}], "takt_s": 60}, "subscription": {"weg_im_neustartfenster": 15}, "topologie_ms": 83, "warmup_s": 20}
```

Killpunkt-Lauf, `--sonden 16 --minuten 5 --neustarts 2`, Exit 0 (auch als
[`roh/G3-SOAK-nacharbeit-1-killpunkt-bericht.json`](roh/G3-SOAK-nacharbeit-1-killpunkt-bericht.json)):

```json
{"abbau_haenger": "", "audio": {"blockgroesse": 512, "bloecke": 528432, "bloecke_je_sonde_min": 33027, "ganzblockdrops_oversize": 0, "ganzblockdrops_ueberlauf": 0, "kontinuitaetsbrueche": 0, "publikationen": 52832, "publikationen_je_sonde_min": 3302, "samplerate": 48000.0}, "client_exit": 0, "clientpaare": 17, "fluter": {"abgelehnt": 0, "ersetzte_liveframes": 577, "kollisionsloecher": 0, "neueste_verworfen": 0, "veroeffentlichungen": 3883, "zu_gross": 0}, "kill": {"k_s1": {"gefahren": true, "telemetrie_handle_fehler": 6, "urteil": "getroffen"}, "k_s2": {"gefahren": true, "snapshot_vor_kill": false, "urteil": "getroffen"}, "k_s3": {"gefahren": true, "p0_ohne_ack_im_fenster": 0, "urteil": "nicht_getroffen"}, "k_s4": {"flag_zum_killzeitpunkt": 4, "gefahren": true, "urteil": "getroffen"}, "k_s5": {"backoff_deckel_erreicht": 17, "erwartet": 17, "gefahren": true, "urteil": "getroffen"}}, "langsam": {"abgelehnt": 0, "blockiert_andere_nicht": true, "ersetzte_liveframes": 16171, "fluter_ueberlappt": false, "immer_mitglied": true, "kollisionsloecher": 0, "neueste_verworfen": 0, "schnelle_p95_ms": 21.1, "sonden": 4, "veroeffentlichungen": 42276, "zu_gross": 0}, "langsam_anzahl": 4, "liveness": {"evicted_ausserhalb_neustart": 0, "stale_ausserhalb_neustart": 0, "stale_im_neustartfenster": 0}, "minuten": 5, "mitgliedschaft": {"abtast_pruefungen": 3033, "abtast_vollstaendig": 3033, "fremde_adresse": 0, "fuehrendes_main_falsch": 0, "snapshot_pruefungen": 9723, "uebernahmen": 10508, "ungueltig": 0, "vollstaendig": 9723}, "neustart": [{"alte_epoche_nach_neustart_gesehen": 0, "backoff_deckel_in_totzeit": 0, "bereit_bis_vollstaendig_ms": 2513, "epoch_alt": "1696b935123f4c468f1a377828a933c5", "epoch_neu": "ab78f8952fdb483fad9b5528d64e5677", "frist_ms": 60000, "index": 1, "kill_dauer_ms": 3, "kill_erfolgt_gesehen": true, "reconnect_ms": {"max": 2513.3, "min": 537.8, "p95": 2513.3}, "reconnect_paare": 17, "s09_quellen_getrennt": 15, "s09_sichten": 35, "s09_subscription_weg": 15, "schranke_ms": 45051, "totzeit_erfasst": true, "totzeit_ms": 1007, "ueber_schranke": false}, {"alte_epoche_nach_neustart_gesehen": 0, "backoff_deckel_in_totzeit": 17, "bereit_bis_vollstaendig_ms": 2543, "epoch_alt": "ab78f8952fdb483fad9b5528d64e5677", "epoch_neu": "b3ffec84bf394588a1b36c958c595e95", "frist_ms": 60000, "index": 2, "kill_dauer_ms": 3, "kill_erfolgt_gesehen": true, "reconnect_ms": {"max": 2544.0, "min": 2544.0, "p95": 2544.0}, "reconnect_paare": 17, "s09_quellen_getrennt": 234, "s09_sichten": 234, "s09_subscription_weg": 234, "schranke_ms": 45051, "totzeit_erfasst": true, "totzeit_ms": 21007, "ueber_schranke": false}], "p0": {"beantwortet": 5616, "gemessen_ausserhalb": 5136, "gemessen_im_stoerfenster": 480, "gesendet": 5616, "latenz_max_im_stoerfenster_ms": 23176.3, "latenz_max_ms": 2196.7, "latenz_p95_ms": 21.6, "nachlauf_ms": 2000, "schranke_ms": 1000, "verloren_ausserhalb_neustart": 0, "verloren_im_neustartfenster": 0}, "pipe": {"clientnamen_geprueft": 34, "eigene_probe_sichtbar": true, "fremder_name_versucht": 0, "goldenname_neu": false, "namen_nach": 168, "namen_vor": 167, "namensraum_lesbar": true, "ohne_produkt_v3": true, "produktionsname_neu": false}, "riegel": {"golden_EqCopSessionSoak": 3, "golden_eqcop-broker-sonde012-probe": 3, "produktion_EqCopSessionSoak": 3, "produktion_eqcop-broker-sonde012-probe": 3}, "sonden": 16, "speicher": {"broker": [{"generation": 0, "im_neustartfenster": false, "minute": 0, "rss_bytes": 13037568, "sekunden": 0.0}, {"generation": 0, "im_neustartfenster": false, "minute": 1, "rss_bytes": 13234176, "sekunden": 60.0}, {"generation": 1, "im_neustartfenster": true, "minute": 1, "rss_bytes": 5451776, "sekunden": 100.1}, {"generation": 1, "im_neustartfenster": false, "minute": 1, "rss_bytes": 12726272, "sekunden": 100.1}, {"generation": 1, "im_neustartfenster": false, "minute": 2, "rss_bytes": 13058048, "sekunden": 120.1}, {"generation": 1, "im_neustartfenster": false, "minute": 3, "rss_bytes": 13479936, "sekunden": 180.1}, {"generation": 2, "im_neustartfenster": true, "minute": 3, "rss_bytes": 5455872, "sekunden": 200.1}, {"generation": 2, "im_neustartfenster": false, "minute": 3, "rss_bytes": 13049856, "sekunden": 200.1}, {"generation": 2, "im_neustartfenster": false, "minute": 4, "rss_bytes": 13242368, "sekunden": 240.2}, {"generation": 2, "im_neustartfenster": false, "minute": 5, "rss_bytes": 13348864, "sekunden": 300.2}], "budget_bytes": 16777216, "budget_prozent": 10, "client": [{"im_neustartfenster": false, "minute": 0, "rss_bytes": 110686208, "sekunden": 0.0}, {"im_neustartfenster": false, "minute": 1, "rss_bytes": 110944256, "sekunden": 60.0}, {"im_neustartfenster": true, "minute": 1, "rss_bytes": 110972928, "sekunden": 100.1}, {"im_neustartfenster": false, "minute": 1, "rss_bytes": 111595520, "sekunden": 100.1}, {"im_neustartfenster": false, "minute": 2, "rss_bytes": 111595520, "sekunden": 120.1}, {"im_neustartfenster": false, "minute": 3, "rss_bytes": 111595520, "sekunden": 180.1}, {"im_neustartfenster": true, "minute": 3, "rss_bytes": 111595520, "sekunden": 200.1}, {"im_neustartfenster": false, "minute": 3, "rss_bytes": 111771648, "sekunden": 200.1}, {"im_neustartfenster": false, "minute": 4, "rss_bytes": 111771648, "sekunden": 240.2}, {"im_neustartfenster": false, "minute": 5, "rss_bytes": 111906816, "sekunden": 300.2}], "takt_s": 60}, "subscription": {"weg_im_neustartfenster": 249}, "topologie_ms": 81, "warmup_s": 20}
```

Die rohe Ausgabe aller 17 Rotmutationen liegt als
[`roh/G3-SOAK-nacharbeit-1-rotmutationen.txt`](roh/G3-SOAK-nacharbeit-1-rotmutationen.txt).

## 16. Nacharbeit Runde 2 — der eine Rest der Wiederprüfung

Die **Wiederprüfung** von Runde 1 (frischer Codex-Thread
`01a062b9-c5ec-7cb3-a2d5-4ac1012b8e22`, gpt-5.6-sol, Effort max, read-only, auf
`882b2aa`) bestätigte **16 der 17 Defekte als geschlossen** und erhob **genau
einen Rest**, wörtlich:

> Leite den K-S5-Sollwert aus N ab — tools/eq-copilot/pruefe_session_soak.py:1020-1023.
> Wenn der C++-Bericht k_s5.erwartet falsch setzt oder weglaesst, uebernimmt diese
> Ableitung den gelieferten Wert statt unabhaengig args.sonden + 1 zu verlangen.
> Auf dem gespeicherten gruenen Bericht liefern sowohl backoff_deckel_erreicht=0,
> erwartet=0, gefahren=true, urteil=getroffen als auch ein fehlendes erwartet bei
> konsistentem nicht_getroffen weiterhin Exit 0; damit kann K-S5 ohne einen
> einzigen Client am Backoff-Deckel gruen erscheinen. Das widerspricht dem
> Pflichtfeld- und N+1-Vertrag in docs/beweise/G3-SOAK.md:346,420-423; berechne
> den Sollwert aus der geplanten Sondenzahl und pruefe das Berichtsfeld dagegen.

Die beiden Zeilenangaben gelten zum Stand `882b2aa`: `:1020-1023` war die
Ableitungszeile in `killurteile()`, `:346` die K-S5-Zeile der Killmatrix und
`:420-423` der Absatz über die nachgerechneten Killurteile in §7.

### 16.1 Der Rest und die Regel, die ihn schliesst

Runde 1 hatte die richtige Regel — *jedes Urteil folgt aus seinem Beleg* — nur
eine Ebene zu flach angewandt. Der **Beleg** (`backoff_deckel_erreicht`) wurde
nicht mehr übernommen, der **Sollwert**, gegen den er fällt, sehr wohl:
`erwartet_wert = eintrag.get("erwartet")` holte die Vergleichszahl aus derselben
Datei, über die geurteilt wurde. Der Vergleich `v == e` ist dann kein Test mehr,
sondern ein Zirkelschluss: `0 == 0` besteht ihn genauso wie `17 == 17`.

| # | Rest | Regel, die ihn schliesst | Wo |
|---|---|---|---|
| 1 | der K-S5-Sollwert kam aus dem Bericht (`k_s5.erwartet`), nicht aus N | der Prüfer rechnet ihn selbst als `--sonden + 1` (alle Control-Clients: Main plus N Sonden); `k_s5.erwartet` bleibt **Pflichtfeld** und wird gegen die eigene Rechnung gehalten, statt sie zu ersetzen. Die Ableitung von `k_s5.urteil` benutzt ausschliesslich die gerechnete Zahl | `pruefe_session_soak.py`: neue Tabelle `SOLLWERTE`, ausgewertet in `killurteile()` |

Der Riegel hat deshalb **zwei** Hälften, und beide werden gebraucht: die
Nachrechnung allein liesse ein fehlendes `erwartet` durch, der Pflichtfeldtest
allein einen falschen Wert. Ein Bericht, der `erwartet` an seinen eigenen Istwert
anpasst, fällt jetzt zweimal — am Sollwertvergleich und am Urteilswiderspruch.

**Die vier übrigen Killzeilen tragen keinen Sollwert.** K-S1, K-S3 und K-S4
fallen gegen die feste Schwelle „> 0", K-S2 gegen `snapshot_vor_kill == false`;
alle vier Schwellen stehen im Prüfer, keine im Bericht. Damit dieses Muster nicht
still zurückkehrt, ist der Riegel generisch: ein Sollwertfeld an einer Killzeile,
für die `SOLLWERTE` keine eigene Rechnung kennt, ist ROT — ein künftiges
`k_s3.erwartet` müsste seine Rechnung mitbringen, statt sich selbst zu prüfen.

### 16.2 Rotbeweis — dreimal, je einmal vor und nach dem Fix

Der Beweis läuft in-memory über `urteile()`, gegen **denselben** gespeicherten
grünen Bericht ([`roh/G3-SOAK-nacharbeit-1-killpunkt-bericht.json`](roh/G3-SOAK-nacharbeit-1-killpunkt-bericht.json),
N = 16, Sollwert 17), einmal mit dem Prüfer von `882b2aa` und einmal mit dem
Stand dieser Runde. Das Skript liegt als
[`roh/G3-SOAK-nacharbeit-2-sollwert-inmemory.py`](roh/G3-SOAK-nacharbeit-2-sollwert-inmemory.py)
im Repo, die rohe Ausgabe als
[`roh/G3-SOAK-nacharbeit-2-sollwert-inmemory.txt`](roh/G3-SOAK-nacharbeit-2-sollwert-inmemory.txt).

| Fall | Was am Bericht verändert wird | Prüfer `882b2aa` | Prüfer Runde 2 |
|---|---|---|---|
| a | `erwartet = 0`, `backoff_deckel_erreicht = 0`, `gefahren = true`, `urteil = getroffen` | **Exit 0** — grün ohne einen Client am Deckel | **Exit 2**, zwei rote Zeilen: `` `erwartet` im Bericht ist 0, gerechnet 17 (N = 16, alle Control-Clients: Main plus N Sonden) `` und `getroffen (abgeleitet: nicht_getroffen)` |
| b | `erwartet` fehlt ganz, `backoff_deckel_erreicht = 0`, `urteil = nicht_getroffen` (in sich konsistent) | **Exit 0** | **Exit 2**: `Pflichtfeld(er) fehlen: ['erwartet']` |
| c | nichts — der unveränderte grüne Bericht | Exit 0 | **Exit 0** |

Fall a und b sind damit **Belege, keine Regressionswachen** (Prüfliste E): sie
waren vor dem Fix grün. Fall c zeigt, dass der neue Riegel den echten Lauf nicht
verschiebt.

Dazu kommt der Mutant **`k_s5_sollwert`** als 23. Rotmutation über den
gespeicherten Kanon-Bericht — er greift genau die *Herkunft* des Sollwerts an:

```powershell
py -3.13 tools/eq-copilot/pruefe_session_soak.py `
    --bericht docs/beweise/roh/G3-SOAK-nacharbeit-1-kanon-bericht.json `
    --sonden 16 --neustarts 1 --mutant k_s5_sollwert
```

```
  ROT     [k_s5 · A24:kill_aus_backoff_deckel] `erwartet` im Bericht ist 0, gerechnet 17 (N = 16, alle Control-Clients: Main plus N Sonden)
  ROT     [k_s5 · A24:kill_aus_backoff_deckel] getroffen (abgeleitet: nicht_getroffen)  {'backoff_deckel_erreicht': 0, 'erwartet': 0, 'gefahren': True}
```

Alle **23** berichtsbasierten Mutanten (die 17 aus Runde 1, die fünf aus Phase 2
ausser `s15`, und der neue) enden weiterhin mit **Exit 2**, die Gegenprobe ohne
Mutant mit **Exit 0**; rohe Ausgabe aller 24 Läufe in
[`roh/G3-SOAK-nacharbeit-2-rotmutationen.txt`](roh/G3-SOAK-nacharbeit-2-rotmutationen.txt).

### 16.3 Die drei Läufe dieser Runde — die Rechnung an einer anderen Sondenzahl

Ein grüner Lauf mit N = 16 allein hätte den Fix nicht belegt: dort ist der
gerechnete Sollwert 17 und der gelieferte auch. **Bei N = 4 ist er 5** — eine
Zahl, die nirgends im Prüfer steht. Alle drei Läufe auf dem Baustand dieser
Runde, Windows 11, Release:

| Lauf | Aufruf | Exit | K-S5 | Kernzahlen |
|---|---|---|---|---|
| Kanon-Kurzform | `--sonden 16 --minuten 2 --neustarts 1` | **0 (GRUEN)** | `nicht_gefahren` (braucht `--neustarts >= 2`), Sollwert 17 gegen `erwartet` 17 geprüft | Topologie 79 ms · **3.914/3.914 Snapshot-Übernahmen sofort vollständig** bei 4.606 Übernahmen, 0 ungültig · 1.228/1.228 Abtastungen · 2.336/2.336 P0 beantwortet, p95 21,8 ms · 220.656 Ganzblöcke, 0 Drops, kleinste Sonde 1.379 Publikationen · Reconnect min 538,1 / max 2.512,3 ms über **17 Paare** |
| Auftragsform | `--sonden 4 --minuten 1 --neustarts 2` | **2 (ROT)** — an S07, siehe §16.4 | **getroffen, 5/5** bei gerechnetem Sollwert 5 | Topologie 66 ms · 587/587 Snapshot-Übernahmen sofort vollständig · 444/444 P0 beantwortet, p95 20,4 ms · Reconnect über 5 Paare |
| Beleglauf | `--sonden 4 --minuten 5 --neustarts 2` | **0 (GRUEN)** | **getroffen, 5/5**, in 21,0 s Totzeit erfasst | Topologie 67 ms · **2.735/2.735 Snapshot-Übernahmen sofort vollständig** bei 3.016 Übernahmen, 0 ungültig · 3.034/3.034 Abtastungen · 1.404/1.404 P0 beantwortet, p95 21,1 ms · 132.104 Ganzblöcke, 0 Drops, kleinste Sonde 3.302 Publikationen · zwei Neustarts in 2.509 und 2.551 ms über je **5 Paare** · 1 langsame Sonde (`max(1, round(4 · 0,25))`), 4.032 ersetzte Liveframes bei 10.569 Veröffentlichungen |

Die Kanon-Kurzform ist mitgefahren, weil der Riegel dort die *andere* Hälfte
zeigt: K-S5 ist `nicht_gefahren`, und trotzdem wird `erwartet` als Pflichtfeld
gegen die Rechnung geprüft — ein Bericht könnte den Sollwert sonst genau dort
verfälschen, wo niemand hinsieht.

Rohe Protokolle: [`roh/G3-SOAK-nacharbeit-2-kanon.txt`](roh/G3-SOAK-nacharbeit-2-kanon.txt),
[`roh/G3-SOAK-nacharbeit-2-n4.txt`](roh/G3-SOAK-nacharbeit-2-n4.txt)
(grün, mit Bericht als [`roh/G3-SOAK-nacharbeit-2-n4-bericht.json`](roh/G3-SOAK-nacharbeit-2-n4-bericht.json))
und [`roh/G3-SOAK-nacharbeit-2-n4-minuten1.txt`](roh/G3-SOAK-nacharbeit-2-n4-minuten1.txt) (rot).

### 16.4 Warum `--minuten 1 --neustarts 2` nicht grün werden kann

Die Auftragsform ist rot, aber **nicht an K-S5** — die drei roten Zeilen stehen
alle an S07, der Speicherkurve:

```
  ROT     [S07 · A24:speicherkurve_im_budget] Broker Generation 0: mindestens zwei Messpunkte ausserhalb der Neustartfenster  [1 von 1 Punkten — unzureichend ist nicht bestanden]
  ROT     [S07 · A24:speicherkurve_im_budget] Broker Generation 1: mindestens zwei Messpunkte ausserhalb der Neustartfenster  [1 von 2 Punkten — unzureichend ist nicht bestanden]
  ROT     [S07 · A24:speicherkurve_im_budget] Broker Generation 2: die Kurve traegt echte Minutenabstaende  [20 s (mindestens 30 s)]
```

Der Grund ist Arithmetik, kein Defekt: Neustart *k* liegt bei *k/(neustarts+1)*
des Messfensters (§9), also bei 20 s und 40 s einer Minute. Jede Brokergeneration
lebt damit 20 s — weniger als der Speichertakt von 60 s und weniger als die
30 s Mindestspanne, die Runde 1 gegen die Scheinkurve aus Defekt 2 und 3 gesetzt
hat. **Diese Kombination ist strukturell nicht grün**, und zwar unabhängig von
Runde 2: derselbe Bericht, durch den Prüfer von `882b2aa` gefahren, liefert
dieselben drei S07-Zeilen und Exit 2
([`roh/G3-SOAK-nacharbeit-2-n4-minuten1-alter-pruefer.txt`](roh/G3-SOAK-nacharbeit-2-n4-minuten1-alter-pruefer.txt)).

S07 wurde **nicht** angefasst: die Zeile misst richtig, die Parameterwahl passt
nicht zu ihr, und sie liegt ausserhalb des einen beauftragten Restes. Wer K-S2
und K-S5 in einer kurzen Form sehen will, braucht je Generation mindestens einen
Speichertakt — `--minuten 5 --neustarts 2` ist die kleinste belegte Form
(Kills bei 100 s und 200 s), und genau die ist als Beleglauf gefahren.

### 16.5 Was auch diese Runde nicht hergibt

- **K-S3 bleibt `nicht_getroffen`.** Unverändert §15.5: ein während der Totzeit
  eingereihter Heartbeat überlebt den Neustart in der P0-Queue und bekommt sein
  ACK. Der Lauf mit N = 4 zeigt dasselbe Bild (`p0_ohne_ack_im_fenster` 0 bei
  `verloren_im_neustartfenster` 0).
- **Kein C++ geprüft oder geändert.** Der Rest lag allein in der Urteilslogik.
  Dass `SessionSoakMain.cpp` den Sollwert korrekt als `anzahl + 1` schreibt
  (`:1407`), ist jetzt eine *gemessene* Übereinstimmung mit der Rechnung des
  Prüfers — vorher war es eine unbelegte Annahme.
- **Die Laufzeiten sind 1, 2 und 5 Minuten, nicht 60.** Unverändert §13.2.
- **Kein XRun.** Unverändert §2.2.

---

## Kanon-Lauf - G3-SOAK

**Lauf:** 2026-09-02 15:17 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 40/40 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/G3-SOAK-7386741.md](roh/G3-SOAK-7386741.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-02 15:17:56 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 7386741 G3-SOAK Phase 2: Manifest um gefahrene Laeufe, Rotmutationen und Messorte |
| Commit (voll) | 7386741de779c9c8bbe124dfad46714eb8b5ce35 |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,09 s | [A1](roh/G3-SOAK-7386741.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,18 s | [A2](roh/G3-SOAK-7386741.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,16 s | [A3](roh/G3-SOAK-7386741.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 73,04 s | [A4](roh/G3-SOAK-7386741.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 6,87 s | [A4-SI](roh/G3-SOAK-7386741.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,09 s | [A4b](roh/G3-SOAK-7386741.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,75 s | [A5](roh/G3-SOAK-7386741.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [A6](roh/G3-SOAK-7386741.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [A7](roh/G3-SOAK-7386741.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,19 s | [A8](roh/G3-SOAK-7386741.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,24 s | [A9](roh/G3-SOAK-7386741.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen und band_stereo ID 10 samt saturated-/Grenzfaellen sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,91 s | [A10](roh/G3-SOAK-7386741.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,22 s | [A11](roh/G3-SOAK-7386741.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,22 s | [A12](roh/G3-SOAK-7386741.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,19 s | [A13](roh/G3-SOAK-7386741.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 4,65 s | [A14](roh/G3-SOAK-7386741.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,97 s | [A17](roh/G3-SOAK-7386741.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 31,29 s | [A18](roh/G3-SOAK-7386741.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,13 s | [A19](roh/G3-SOAK-7386741.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,30 s | [A20](roh/G3-SOAK-7386741.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,27 s | [A21](roh/G3-SOAK-7386741.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,03 s | [A22](roh/G3-SOAK-7386741.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 146,42 s | [A24](roh/G3-SOAK-7386741.md#a24) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,61 s | [A23](roh/G3-SOAK-7386741.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,10 s | [A16](roh/G3-SOAK-7386741.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,11 s | [B1](roh/G3-SOAK-7386741.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,07 s | [B2](roh/G3-SOAK-7386741.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/G3-SOAK-7386741.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/G3-SOAK-7386741.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,23 s | [B3c](roh/G3-SOAK-7386741.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,30 s | [B4](roh/G3-SOAK-7386741.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [B9](roh/G3-SOAK-7386741.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,03 s | [B5](roh/G3-SOAK-7386741.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 59,59 s | [B10](roh/G3-SOAK-7386741.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 0,12 s | [B11](roh/G3-SOAK-7386741.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,52 s | [B12](roh/G3-SOAK-7386741.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/G3-SOAK-7386741.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,19 s | [B14](roh/G3-SOAK-7386741.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,06 s | [B15](roh/G3-SOAK-7386741.md#b15) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,13 s | [B8](roh/G3-SOAK-7386741.md#b8) |


---

## Kanon-Lauf - G3-SOAK

**Lauf:** 2026-09-02 15:47 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 40/40 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/G3-SOAK-9909722.md](roh/G3-SOAK-9909722.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-02 15:47:45 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 9909722 G3-SOAK Phase 2: Pruefpunktnamen fuehren, S09 messen |
| Commit (voll) | 99097229564cc082f8ec4606bce005f0e80d66f9 |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,09 s | [A1](roh/G3-SOAK-9909722.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,11 s | [A2](roh/G3-SOAK-9909722.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,17 s | [A3](roh/G3-SOAK-9909722.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 70,66 s | [A4](roh/G3-SOAK-9909722.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 7,37 s | [A4-SI](roh/G3-SOAK-9909722.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 0,60 s | [A4b](roh/G3-SOAK-9909722.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,72 s | [A5](roh/G3-SOAK-9909722.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,12 s | [A6](roh/G3-SOAK-9909722.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,11 s | [A7](roh/G3-SOAK-9909722.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,17 s | [A8](roh/G3-SOAK-9909722.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,24 s | [A9](roh/G3-SOAK-9909722.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen und band_stereo ID 10 samt saturated-/Grenzfaellen sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,43 s | [A10](roh/G3-SOAK-9909722.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,23 s | [A11](roh/G3-SOAK-9909722.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,22 s | [A12](roh/G3-SOAK-9909722.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,18 s | [A13](roh/G3-SOAK-9909722.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 4,54 s | [A14](roh/G3-SOAK-9909722.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,96 s | [A17](roh/G3-SOAK-9909722.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 31,14 s | [A18](roh/G3-SOAK-9909722.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,12 s | [A19](roh/G3-SOAK-9909722.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,29 s | [A20](roh/G3-SOAK-9909722.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,26 s | [A21](roh/G3-SOAK-9909722.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,03 s | [A22](roh/G3-SOAK-9909722.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 146,43 s | [A24](roh/G3-SOAK-9909722.md#a24) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,61 s | [A23](roh/G3-SOAK-9909722.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,10 s | [A16](roh/G3-SOAK-9909722.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,13 s | [B1](roh/G3-SOAK-9909722.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,09 s | [B2](roh/G3-SOAK-9909722.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [B3](roh/G3-SOAK-9909722.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/G3-SOAK-9909722.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,23 s | [B3c](roh/G3-SOAK-9909722.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,25 s | [B4](roh/G3-SOAK-9909722.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [B9](roh/G3-SOAK-9909722.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,01 s | [B5](roh/G3-SOAK-9909722.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 59,59 s | [B10](roh/G3-SOAK-9909722.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 0,12 s | [B11](roh/G3-SOAK-9909722.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,51 s | [B12](roh/G3-SOAK-9909722.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/G3-SOAK-9909722.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,18 s | [B14](roh/G3-SOAK-9909722.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,11 s | [B15](roh/G3-SOAK-9909722.md#b15) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,12 s | [B8](roh/G3-SOAK-9909722.md#b8) |


---

## Kanon-Lauf - G3-SOAK

**Lauf:** 2026-09-02 18:15 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 40/40 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/G3-SOAK-d52fd15.md](roh/G3-SOAK-d52fd15.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-02 18:15:50 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | d52fd15 G3-SOAK Nacharbeit Runde 2: der K-S5-Sollwert kommt aus N, nicht aus dem Bericht |
| Commit (voll) | d52fd151d60c8f718c553016928b004e9464c5b9 |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,09 s | [A1](roh/G3-SOAK-d52fd15.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,07 s | [A2](roh/G3-SOAK-d52fd15.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 20,88 s | [A3](roh/G3-SOAK-d52fd15.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 75,59 s | [A4](roh/G3-SOAK-d52fd15.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 7,33 s | [A4-SI](roh/G3-SOAK-d52fd15.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 0,59 s | [A4b](roh/G3-SOAK-d52fd15.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 2,65 s | [A5](roh/G3-SOAK-d52fd15.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,14 s | [A6](roh/G3-SOAK-d52fd15.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [A7](roh/G3-SOAK-d52fd15.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,20 s | [A8](roh/G3-SOAK-d52fd15.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,34 s | [A9](roh/G3-SOAK-d52fd15.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen und band_stereo ID 10 samt saturated-/Grenzfaellen sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,89 s | [A10](roh/G3-SOAK-d52fd15.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,24 s | [A11](roh/G3-SOAK-d52fd15.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,30 s | [A12](roh/G3-SOAK-d52fd15.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,23 s | [A13](roh/G3-SOAK-d52fd15.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 4,68 s | [A14](roh/G3-SOAK-d52fd15.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,01 s | [A17](roh/G3-SOAK-d52fd15.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 32,03 s | [A18](roh/G3-SOAK-d52fd15.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,13 s | [A19](roh/G3-SOAK-d52fd15.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [A20](roh/G3-SOAK-d52fd15.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,26 s | [A21](roh/G3-SOAK-d52fd15.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,85 s | [A22](roh/G3-SOAK-d52fd15.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,95 s | [A24](roh/G3-SOAK-d52fd15.md#a24) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 5,53 s | [A23](roh/G3-SOAK-d52fd15.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,10 s | [A16](roh/G3-SOAK-d52fd15.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,15 s | [B1](roh/G3-SOAK-d52fd15.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,12 s | [B2](roh/G3-SOAK-d52fd15.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,06 s | [B3](roh/G3-SOAK-d52fd15.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/G3-SOAK-d52fd15.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,24 s | [B3c](roh/G3-SOAK-d52fd15.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,60 s | [B4](roh/G3-SOAK-d52fd15.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [B9](roh/G3-SOAK-d52fd15.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,04 s | [B5](roh/G3-SOAK-d52fd15.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 114,09 s | [B10](roh/G3-SOAK-d52fd15.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 0,12 s | [B11](roh/G3-SOAK-d52fd15.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,53 s | [B12](roh/G3-SOAK-d52fd15.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/G3-SOAK-d52fd15.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,22 s | [B14](roh/G3-SOAK-d52fd15.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,09 s | [B15](roh/G3-SOAK-d52fd15.md#b15) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,13 s | [B8](roh/G3-SOAK-d52fd15.md#b8) |

## 17. Abschluss T2 — Dirigent 2026-09-02

**Marke:** `T2 PASS 2026-09-02`. Geprüfter Stand `d52fd15`; Basis-SHA des Tickets `03e1e17`
(Phase 1), `f7a5125` (Phase 1b), `a570367` (Phase 2), `4ae3ee6` (Nacharbeit Runde 1). Prüfer: Codex
`gpt-5.6-sol`, lesend, jede Runde ein frischer Thread; Bauer: Opus (max) im sichtbaren Checkout.

| Runde | Prüfbereich | Thread | Effort | Urteil |
|---|---|---|---|---|
| Matrix 1 | Verhaltensmatrix (Spezifikation vor Code) auf `f7a5125` | `01a061ed-25ea-7242-a384-b936d10c62de` | high | NEEDS_WORK — 4 Befunde (3 Defekte, 1 Lücke mit Dirigentenregel) |
| Matrix 2 | Fixdiff `d60ceaf`…`a570367` | `01a06203-877a-7fe1-aa47-41fe219de8a2` | high | NEEDS_WORK — 1 Defekt (K-S1-Belegfeld); Regel des Dirigenten, als erster Schritt von Phase 2 umgesetzt |
| Abschluss | Ticketpfade `a570367`…`4ae3ee6` | `01a0626a-234c-7831-9e4b-d2a4ae6426ca` | max | NEEDS_WORK — 18 Befunde: 17 Defekte mit einer Ursache, 1 Härtung (Runner-Frische, NAK-133) |
| Wiederprüfung 1 | Fixdiff `4ae3ee6`…`882b2aa` | `01a062b9-c5ec-7cb3-a2d5-4ac1012b8e22` | max | NEEDS_WORK — 16 von 17 geschlossen, 1 Rest (K-S5-Sollwert aus dem Bericht statt aus N) |
| Nacharbeit 2 | `pruefe_session_soak.py`: Sollwert `args.sonden + 1` vom Prüfer berechnet, Pflichtfeld geprüft, Killurteile aus Belegen | Opus-Worker `51f69bf2` | — | Commit `d52fd15` |
| Wiederprüfung 2 | Fixdiff `882b2aa`…`d52fd15` | `01a062e2-121d-7b51-8e1a-0864a836f554` | max | **PASS** |

**Ursache der 17 Defekte** (Dirigent, aus dem Urteil): Belege wurden abgetastet oder synthetisiert statt
am Ereignis erfasst, und der Prüfer übernahm Urteile beziehungsweise übersprang fehlende Einträge,
statt jedes Urteil aus seinem Beleg abzuleiten. Der Nacharbeitsauftrag nannte die Ursache und die
17 Defekte wörtlich (Abschnitt 15).

**Rundenbilanz** (`rundenbilanz.py --runden 03e1e17 d60ceaf a570367 4ae3ee6 d52fd15`):
Phase 1 und 1b Doku +392/−0 und +272/−90 (Spezifikation vor Code, bewusst ohne Code); Phase 2
Produkt +27/−1 (CMake-Zielregistrierung), Tests +1185/−0, Prüfwerkzeug +616/−0; Nacharbeit 1
Tests +478/−117, Prüfwerkzeug +610/−94; Nacharbeit 2 Prüfwerkzeug +70/−6 (`882b2aa`…`d52fd15`). Das Konvergenzsignal nach den zwei Spezifikationsrunden ist
ein Artefakt der Phase „Spezifikation vor Code“, kein Stillstand: die Bauphase folgte unmittelbar.

**Kanon:** GRUEN 40/40 auf `d52fd15` (Lauf 2026-09-02 18:15, Exitcode 0, Rohausgabe `docs/beweise/roh/G3-SOAK-d52fd15.md`, Runner-Kopf oben in diesem Manifest). Der abgekoppelte Start erhielt im Temp-Log keine EXIT=-Zeile, weil der Add-Content-Nachsatz nicht lief; der Beweis ist der Runner-Kopf, nicht das Log.

**Nicht Teil dieses Tickets, im Gate-Manifest G3 zu belegen:** der 60-Minuten-Lauf mit 16 Sonden, der
30-Minuten-Lauf mit 32 Sonden und die Skalierungsreihe 1/4/8 (S16); die XRun-Hälfte bleibt FL-Termin
(Abschnitt 2.2). Register: NAK-132 (A22-Heartbeat gegen Zählsenke), NAK-133 (Runner-Frische).

## 16. Nachtrag 2026-09-03 (NAK-134) — zwei Zähler, damit A24 den Defekt sieht

**Append-only.** Nichts oben wird umgeschrieben; die Matrixzeilen S04 und S11
bekommen je einen zusätzlichen Beleg, der Rest bleibt Verlauf.

**Der Anlass.** Der Gate-Lauf G3 (32 Sonden, 30 min, 3 Neustarts) war rot, und
A24 hat den eigentlichen Ausfall **nicht gesehen**. Nach jedem Brokerneustart
blieb ein Teil der Sonden dauerhaft getrennt (NAK-134). Zwei Zähler haben das
verdeckt:

* `SessionSoakMain.cpp` nahm eine abgewiesene P0-Einreihung **still** zurück
  (`gesendet.pop_back()`). Ein Client, der gar nichts mehr einreiht, erzeugt
  damit keinen einzigen sichtbaren Verlust — „nie eingereiht" heißt „nie
  erwartet", und `p0.verloren_ausserhalb_neustart` blieb bei 0. Der Ausfall war
  nur an einer Kopfrechnung über `p0.gesendet` erkennbar.
* `reconnect_paare` war `n.reconnectMs.size()`, also strukturell `anzahl + 1`,
  und der Prüfer verglich diesen Wert gegen `args.sonden + 1`. Ist und Soll
  kamen aus derselben Konstante — dieselbe Klasse von Tautologie wie der
  `k_s5.erwartet`-Defekt aus §15.

**Was sich ändert.**

| Zeile | Neuer Beleg | Bedeutung |
|---|---|---|
| S04 | `p0.einreihung_abgelehnt` (Summe, Pflichtfeld) und `p0.einreihung_abgelehnt_ausserhalb_neustart` | Die Rücknahme bleibt richtig — nie eingereiht heißt nie erwartet —, sie ist aber nicht mehr still. Ausserhalb der Neustartfenster muss der zweite Wert **0** sein. Die Fensterzuordnung folgt dem Stand, der beim Eintrag gesetzt wurde, nicht einer erneuten Abfrage. |
| S11 | `reconnect_paare` (Feldname gleich, **Bedeutung geändert**) | Ab hier die **gemessene** Zahl der Paare, deren Control **und** Telemetrie am Ende des Fristfensters wieder `verbunden` waren — bewusst **ohne** die Mitgliedschaftsbedingung, die `proPaar` zusätzlich verlangt. Der Bericht trennt damit „Transport steht wieder" von „Mitglied ist wieder in der Sicht". Die Prüfzeile bleibt wortgleich `== args.sonden + 1` und hört auf, tautologisch zu sein. |

Die Zählerkarte in §7 gilt mit diesen zwei Ergänzungen; `reconnect_paare`
trägt dort ab sofort die neue Bedeutung.

**Beide Zähler haben je zwei Proben** (NAK-134 §9): einen deterministischen
C++-Erzeugertest im neuen `EqCopSessionSoak`-Modus `selbsttest-zaehler` und
eine Berichtsmutante. Eine Mutante am fertigen Bericht beweist nur den
Python-Prüfer, nie die C++-Erzeugung.

**Neue Rotmutante:** `--mutant p0_einreihung` setzt
`einreihung_abgelehnt_ausserhalb_neustart` auf 1 — S04 muss fallen. Die
bestehende `--mutant s11_paare` trifft weiterhin genau das umgestellte Feld und
wurde nach der Umstellung erneut gefahren.

Belege, Exitcodes und die gemessenen Werte stehen in `docs/beweise/NAK-134.md`
(§13, §14 Phase 2).
