# Tiefenaudit 3 · Phase 15 · Paket 1 — Verspätete Ergebnisse nach Zustandswechseln

> **Rolle dieses Berichts:** Rohbericht des Paketprüfers 1 (unabhängiger lesender Agent)
> im Tiefenaudit 3, Phase 15 vom 17.09.2026, Quellstand `e008811efa7650f3e0cc2332cc21f9e268cdd125`.
> Er ist Rohmaterial: vollständige Herleitungen, Gegenargumente, verworfene Verdachte und
> gelesener Scope. Verbindlich ist die Einordnung in [../BEFUNDE.md](../BEFUNDE.md),
> Abschnitt „Phase 15“, zusammen mit [phase-15-zentral.md](phase-15-zentral.md)
> (zentrale Quellprüfung, ID-Zuordnung, Zähler) und
> [phase-15-gegenpruefung-paket1.md](phase-15-gegenpruefung-paket1.md)
> (Widerlegungsversuch durch einen anderen Prüfer).
>
> Die Kennungen `P15-V-…` sind lokale Arbeitsnummern dieses Laufs, keine Befund-IDs.
> Kategorie- und Schwerevorschläge in diesem Bericht sind der Stand **vor** der
> Gegenprüfung; wo diese herabgestuft hat, gilt ihr Urteil. Keine dynamische
> Reproduktion, kein Bau, kein Test, kein Host- oder Brokerlauf.


- Basis: `e008811efa7650f3e0cc2332cc21f9e268cdd125` (Produktcode unverändert; im Worktree nur fremde Auditdokumente uncommittet)
- Datum: 17.09.2026
- Arbeitsweise: nur lesen. Keine Datei im Repo geändert, kein Bau, kein Test, kein Broker-, Pipe-, FL- oder MCP-Lauf.
- Evidenz: ausschließlich **Quellherleitung (statisch)**. Offene Annahmen sind als solche markiert. **Dynamische Reproduktion: keine.**
- Sicherheit (Pipe-ACLs, Impersonation, Token, Signaturen, Diagnoseberechtigung, Installerprivilegien, T3-12-*) nicht geprüft und nicht verwendet. Keine Gestaltungsvorschläge.

Ergebnis in Kürze: 4 Kandidaten (3 DEFEKT medium, 1 HÄRTUNG low), 6 offene Prüffelder, 19 verworfene Verdachte.

---

## 1. Gelesener Scope

### 1.1 Rumpflektüre (Dateien mit Zeilenbereichen)

Gelesen in dieser Sitzung; „ganz" heißt: die Datei wurde vor dem Kontextwechsel der Sitzung vollständig gelesen, die genannten Bereiche wurden für die Zitate nachgelesen.

**Gen: Transport und Prozessor**
- `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp` ganz; nachgelesen 136-155, 404-500, 700-880
- `eq-copilot/plugin/core/ipc/ControlClient.cpp` ganz; nachgelesen 85-170
- `eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp` ganz; nachgelesen 150-404
- `eq-copilot/plugin/core/ipc/controlclient/Laufzeit.h` ganz; `controlclient/Intern.h` 395-468; `controlclient/Vertrag.cpp` 50-60
- `eq-copilot/plugin/core/ipc/ControlClient.h` 200-210, 225-404
- `eq-copilot/plugin/core/ipc/TelemetryClient.cpp` 338-360, 740-800
- `eq-copilot/plugin/src/prozessor/Ipc.cpp` ganz; nachgelesen 455-486, 540-840, 851-1123, 1195-1664
- `eq-copilot/plugin/src/prozessor/State.cpp` ganz; nachgelesen 40-229, 283-655
- `eq-copilot/plugin/src/prozessor/Hostbruecke.cpp` 136-150
- `eq-copilot/plugin/src/prozessor/Analyse.cpp` 133-412
- `eq-copilot/plugin/src/PluginProcessor.cpp` 125-446
- `eq-copilot/plugin/src/PluginProcessor.h` 1297-1333
- `eq-copilot/plugin/src/PluginEditor.cpp` 172-201, 240-340, 490-533
- `eq-copilot/plugin/src/SourcesModel.cpp` 349-666, 1180-1368, 1366-1524, 1520-1610, 1630-1645

**Probeeq und State**
- `eq-copilot/plugin/sonde/SondeProcessor.cpp` 130-201, 258-270, 525-900, 985-1365
- `eq-copilot/plugin/sonde/SondeProcessor.h` 150-165, 512-524
- `eq-copilot/plugin/state/NakamaTransaktion.cpp` 232-272, 400-432
- `eq-copilot/plugin/state/NakamaState.cpp` 652-681, 970-990, 2041-2052, 2195-2220

**Broker**
- `broker/src/coordinator/hypothese_verdrahtung.rs` 100-480, 520-700, 900-950, 1170-1275
- `broker/src/coordinator/intent.rs` ganz; nachgelesen 300-670, 800-820
- `broker/src/coordinator/link.rs` ganz; nachgelesen 80-341, 499-620
- `broker/src/coordinator/liveness.rs` ganz; nachgelesen 50-80, 284-300, 317-635, 710-760, 800-911
- `broker/src/coordinator/befehl.rs` 157-616, 665-956
- `broker/src/coordinator/invalidierung_verdrahtung.rs` ganz; nachgelesen 496-560, 596-720
- `broker/src/coordinator/invalidierung.rs` 229-290
- `broker/src/coordinator/evidenz.rs` ganz; nachgelesen 115-255
- `broker/src/coordinator/proposal_verdrahtung.rs` ganz; nachgelesen 25-135; `proposal.rs` 942-960
- `broker/src/coordinator/experiment_verdrahtung.rs` 39-375, 570-625, 840-900, 860-1080, 1372-1472
- `broker/src/coordinator/prepost_verdrahtung.rs` 60-171
- `broker/src/coordinator/mitgliedschaft.rs` ganz; nachgelesen 119-200
- `broker/src/coordinator/senke.rs` ganz; nachgelesen 130-180
- `broker/src/coordinator/subscription.rs`, `flush.rs`, `sicht.rs` (`snapshot_locked` 519-661), `zustand.rs`, `mod.rs` ganz
- `broker/src/coordinator/assistent.rs` 130-137; `intervention.rs` 632-647; `broker/src/transport/server_v3/verbindung.rs` 300-406, 806-890

**Tests**
- `broker/tests/sonde013_verdrahtung.rs` 920-960, 1820-1856
- `broker/tests/sonde014_verdrahtung.rs` 270-301, 355-440, 480-620, 2760-2800
- `broker/tests/sonde014_gegenbeispiele.rs` 1200-1250
- `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` 1118-1273, 2745-2790
- `eq-copilot/plugin/tests/IpcTestMain.cpp` 5220-5311
- `eq-copilot/plugin/tests/Sonde014BefundTest.cpp` 690-775
- `eq-copilot/plugin/tests/Sonde012HostChannelContextTest.cpp` 28-41, 234-247

**Zusagedokumente**
- `docs/beweise/NAK-246.md` 66-72 (D1-D7), 619-626 (M-15 bis M-19), 4895-4926 (R-A1, Registerlücke)
- `docs/beweise/NAK-283.md` 189-196 (R-283-1), 616-620 (M-13, M-75, M-14 bis M-16)
- `docs/beweise/SONDE-014.md` 721-726 (M-07 bis M-12), 878-884 (M-85, M-86, M-88, M-89)
- `docs/beweise/SONDE-013.md` 548-552 (M-53 bis M-57)
- `docs/beweise/SONDE-015.md` 1980 (T17)
- `docs/beweise/NAK-214.md` 46 (R3), 206-229, 365-370 (V-23, V-24)
- `docs/beweise/SONDE-010.md` 105, 213-217 (B-CC-10)
- `docs/offene-punkte.md` 263 (NAK-253), 279 (NAK-269), 288 (NAK-278), 297 (NAK-287)
- Auditbestand: `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (Zeilen zu T3-02-03, T3-03-03, T3-04-02, T3-05-01, T3-07-01, T3-07-04, T3-07-06, T3-08-02 bis T3-08-04, T3-14-01 bis T3-14-04); `roh/phase-04-lebenslauf.md` 56; `roh/phase-08-broker.md` 96-110, 168; `roh/phase-14-blindspots.md` 110; `roh/phase-07-modell.md` 278 (W-10)

### 1.2 Gezielte Suchen (getrennt von der Rumpflektüre)

Aufgeführt sind die in dieser Fassung nachgeprüften Suchen.

| Nr | Muster und Bereich | Ergebnis |
|---|---|---|
| S1 | `hookAuftragAbgeschlossen`, `setzeAuftragAbgeschlossen`, `ausstehendeSourcesCommands`, `bestaetigteSourcesCommands` in `eq-copilot/plugin` | Hook-Setter nur in Tests (`IpcTestMain.cpp:6951`, `:7009`, `:7138`); `PluginProcessor.cpp:410-415` nennt ihn „mit E-15 fort". Zuordnung nur in `Ipc.cpp` und `State.cpp:135-136`. |
| S2 | `inFlight.clear`/`inFlight.erase` in `core/ipc` | einzige Löschung `Nachrichten.cpp:363` |
| S3 | `controlV3.reconnect()` in Gen und Probeeq | `Hostbruecke.cpp:146`, `SondeProcessor.cpp:267`, `:1070`, `State.cpp:65`, `:199`, `:221`, `:361`, `Ipc.cpp:1046`, `:1122`, `PluginProcessor.cpp:222` |
| S4 | die sieben Intent-Handgriffe (`setzeQuellenrolle` bis `entferneQuellenbeziehung`) | nur `PluginProcessor.h`, `State.cpp` und Tests; kein Editor-Aufrufer |
| S5 | `measurement_position`, `Messposition` ohne Tests; Zuweisungen an `zustand` in `sonde/` | Probeeq-Position nur aus State; Schreiber `SondeProcessor.cpp:172`, `:1039` |
| S6 | `evidence_snapshot`, `runtime.gemeldet` ohne Tests | nur Probeeq meldet Runtime-Block und Evidenz |
| S7 | `apply_transaction(` und `StateRevision` in `broker/` | Produktcode ohne Aufrufer; nur `broker/tests/sonde013_taint.rs` |
| S8 | `befunde_neu_bilden = true`, `hypothesen_bilden()`, `hypothesen_bei_bedarf_bilden()` in `broker/src` | Auslöser nur `evidenz.rs:232`, `invalidierung_verdrahtung.rs:126`; Merker `intent.rs:626`, `invalidierung_verdrahtung.rs:279`, `hypothese_verdrahtung.rs:554`, `:565`, `:671` |
| S9 | `darfAudition`, `darfDraft`, `setzeAlleBefundeStale` | kein Editor-Aufrufer; nur `SourcesModel.h:187-189`, `SourcesModel.cpp` und Tests |
| S10 | `mainDarfSchreiben =` | `SourcesModel.cpp:1637-1639` |
| S11 | `messpunkt_wechsel` in `broker/tests` | kein Fall mit Hello derselben Nonce zwischen zwei Positionen |
| S12 | `intent_spiegel_leeren_locked` | `link.rs:602`, `liveness.rs:74`, `:293` (nur Sitzung ohne Clients) |
| S13 | `AeltereRevision` in `broker/tests` | `sonde014_verdrahtung.rs:297`, `:500`, `:516` (Transportordnung) |
| S14 | `fuehreTransaktionAus` ohne Tests | einziger Produktaufrufer `SondeProcessor.cpp:1343` (Gestus) |
| S15 | `erfolgreich =` in `core/ipc` | `Vertrag.cpp:57-58`: `angewandt` und `idempotent_wiederholt` sind Erfolg |
| S16 | `v3ReconnectFuerTest`, `reconnectSources` in Tests | nur `Sonde013PassageStateTest.cpp:822`, `:2276` |
| S17 | `fn descriptor_aus_heartbeat` | `measurement_position` aus `runtime.messpunkt` (`liveness.rs:720-744`) |
| S18 | `wire_adresse !=`, `adresse !=` im Coordinator | Adressriegel u. a. `evidenz.rs:148`, `intent.rs:480`, `liveness.rs:611`, `senke.rs:157` |
| S19 | `vorschlaege` außerhalb `proposal_verdrahtung.rs` | nur Aufruf `hypothese_verdrahtung.rs:152` und Feld `zustand.rs:370`, `:604`; nicht in der Sicht |
| S20 | Auditbestand: `inFlightAck`, `sollAbbrechen`, `B-CC-10`, `ausstehendeSourcesCommands`; `messpunkt`, `measurement_position`, `M-55`; `Vollbestand`, `AeltereRevision`, `bestand_revision`, `M-86`, `M-09`; `controlEnde`, `begonnene Übernahme`, `W-10`; `descriptor`, `reload`, `recall` | keine Überschneidung mit P15-V-01 bis P15-V-04 außer den in Abschnitt 5 abgegrenzten Stellen |
| S21 | `kBackoffStartMs`, `kHeartbeatTaktMs` | 500 ms Start, 1000 ms Heartbeat; bewusster Reconnect ohne Wartezeit (`Verbindung.cpp:474-484`) |
| S22 | `neueSensorId`, `konfliktGemeldet` | Handgriff `PluginEditor.cpp:591` |

---

## 2. Kandidaten

### P15-V-01 — Positiver Sources-ACK wird nach `reconnect()` vom Transport verbraucht; die Zuordnung verwaist, die Mitgliedschaft wird nie nachgeführt, Bind/Remove der Quelle bleibt bis zum Reload wirkungslos

**1. Auslöser und Reihenfolge (Fäden, Sperren)**
1. Message-Thread: Klick „Bind source" oder „Remove source" (`PluginEditor.cpp:179-192`) → `sendeSourcesCommand`: Zuordnung unter `sourcesCommandMutex` (`Ipc.cpp:1334-1343`), dann `sendePersistenzP0` legt den In-Flight-Eintrag unter `sendeMutex` an (`Nachrichten.cpp:209`).
2. Broker-Verbindungsfaden: `session_command` wendet unter dem Standlock an (`befehl.rs:567-579`), reiht den Sitzungssnapshot ein (`:614`) und antwortet `angewandt` (`:615`). Laut `:610-613` haben Snapshot (P1) und ACK (P0) keine zugesagte Zustellreihenfolge.
3. Client-Faden C (Link-Generation G) liest Bytes (`Verbindung.cpp:768-776`) und behandelt jeden vollständigen Frame so: zuerst `inFlightAck` (`:834`), das den Eintrag unter `sendeMutex` löscht (`Nachrichten.cpp:315-363`); danach die ungesperrte Generationsprüfung (`Verbindung.cpp:836`, Definition `:147-150`); nur bei unveränderter Generation folgt `v3Antwort`, das die Zuordnung findet und den Befehl als bestätigt vormerkt (`Ipc.cpp:1067-1076`).
4. Faden H ruft `reconnect()`: Reconnect-Knopf (`PluginEditor.cpp:193-197` → `Ipc.cpp:1120-1123`), Host-`prepareToPlay` (`Hostbruecke.cpp:146`), Rollenwahl (`State.cpp:361`) oder Kennungswechsel (`State.cpp:65`). Die Generation steigt unter `zustandMutex` (`ControlClient.cpp:149-159`), danach `ioAbbrechen` (`:160`).
5. Kritische Folge: H erhöht die Generation, nachdem C die ACK-Bytes schon im Leser hat, etwa während C noch einen im selben Lesezug davor gelieferten Frame verarbeitet (der Snapshot-Callback parst ohne Sperre, `SourcesModel.cpp:639-1233`). Für den ACK-Frame löscht `inFlightAck` den Eintrag, `sollAbbrechen` ist danach wahr, `v3Antwort` läuft nicht.
6. `sendeMutex` (Register) und `sourcesCommandMutex` (Zuordnung) werden nie gemeinsam gehalten; zwischen Register-Freigabe und Aufrufer-Übernahme steht nur die Generationsprüfung.

**2. Aktuelle Stellen (selbst gelesen)**
- `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:834-842`: `inFlightAck (antwort);` vor `if (! sollAbbrechen (generation))`
- `eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp:362-363` (Löschung), `:383-384` (Freigabe-Hook; im Produkt nicht gesetzt, S1), `:258-282` (Replay nur für verbliebene Einträge)
- `eq-copilot/plugin/src/prozessor/Ipc.cpp:1063-1078` (einziger ACK-Abbau der Zuordnung), `:1336-1341` (zweiter Befehl derselben Quelle wird abgewiesen), `:1353-1354` (Löschung nur bei `endgueltigAbgewiesen`)
- `eq-copilot/plugin/src/prozessor/State.cpp:134-136` (einzige weitere Leerung, nur in `setStateInformation`)
- `eq-copilot/plugin/src/SourcesModel.cpp:1276-1279` (Mitgliedschaft der Sicht nur aus dem persistenten Stand; der Brokersnapshot heilt nicht)
- `eq-copilot/plugin/src/PluginEditor.cpp:187-190` (Rückgabewert des Handgriffs wird ignoriert)

**3. Produkteinstieg**
Erreichbar ohne Testhaken: Bind/Remove plus zeitgleicher `reconnect()` aus einem der vier Produkteinstiege (S3). Wettlaufabhängig; das Fenster liegt zwischen Lesen des ACK und seiner Generationsprüfung und wächst mit der Verarbeitungszeit eines davor im selben Lesezug stehenden Frames. Wie oft FL `prepareToPlay` ruft, ist hier nicht verifiziert (offene Annahme, für die Herleitung nicht nötig).

**4. Verletzte Zusage**
- NAK-246 M-16 (`docs/beweise/NAK-246.md:623`): „Der nachgespielte Befehl wird vom Server bestätigt, der ACK findet die Zuordnung, der Persistenzabschluss (M-10) führt die Mitgliedschaft nach". Hier findet der ACK die Zuordnung nie.
- Schadensbild D4 (`docs/beweise/NAK-246.md:69`): „brokerseitig angewandte Mitgliedschaft wird lokal nicht nachgeführt"; ebenso der Codekommentar `Ipc.cpp:1346-1350`.
- CLAUDE.md, Wahrheitskern „Keine toten UI-Elemente (User 24.08.2026): jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich einen Zustand." Der Knopf bleibt für diese Quelle sichtbar und wirkungslos.
- CLAUDE.md, Invariante „jede persistente Änderung meldet dem Host Dirty-State": für die brokerseitig angewandte Änderung unterbleiben Nachführung und Dirty.

**5. Gegenpfad und stärkstes Gegenargument**
- Heilender Gegenpfad existiert nur, solange `inFlightAck` den Frame nicht gelesen hat: Geht der ACK auf der sterbenden Pipe verloren, bleibt der Eintrag, wird unter derselben `command_id` nachgespielt (`Nachrichten.cpp:258-282`, `Verbindung.cpp:739`), der Broker antwortet `idempotent_wiederholt` (`befehl.rs:529-537`), das zählt als Erfolg (`Vertrag.cpp:57-58`), und `v3Antwort` findet die Zuordnung. Genau dieser Weg fehlt, sobald der Transport den ACK verbraucht hat.
- Die Unterdrückung selbst ist gewollt (`Verbindung.cpp:835-836`, B-CC-10 für `stop()`); der Fehler liegt in der Reihenfolge: das Register wird geräumt, bevor feststeht, dass der Aufrufer den ACK bekommt.
- Drains und Tick (`Ipc.cpp:1538-1588`, `State.cpp:85-112`) arbeiten nur bestätigte Befehle ab und ändern nichts.
- Stärkstes Gegenargument: sehr enges Zeitfenster. Dagegen: die Wirkung ist dauerhaft und nicht selbstheilend (kein weiterer ACK, kein Replay, Sicht ohne Brokermitgliedschaft, neuer Handgriff abgewiesen) bis zum nächsten `setStateInformation`.

**6. Bestehender Test und warum er nicht greift**
- `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:1123-1273` (M-16): der ACK kommt nach dem Replay auf dem neuen Link; zwischen Lesen und Generationsprüfung liegt nie ein Wechsel.
- `eq-copilot/plugin/tests/IpcTestMain.cpp:5226-5307` (G17, B-CC-10 bis B-CC-12): prüft Callback-Unterdrückung und Stoppfrist mit Heartbeat-Antworten, nicht die Kopplung Register und Aufrufer bei einem `command_ack`.

**7. Minimaler Folgebeweis**
- Ausgangslage: echter Prozessor mit Testkonstruktor und Probe-Pipe wie M-16; Main mit Bindung; Quelle A als Hauptziel; Testserver mit `commandAckArt = angewandt`, der unmittelbar vor dem `command_ack` einen beliebigen gültigen Frame im selben Schreibzug sendet; `setzeV3AntwortHakenFuerTest` blockiert beim ersten der beiden Frames.
- Ereignisfolge: `bindeSourcesHauptziel(A)` → Haken hält → `v3ReconnectFuerTest()` → Haken freigeben → neuen Link abwarten.
- Erwartbares Fehlverhalten: `controlV3Snapshot().inFlight == 0` und `inFlightErfolg == 1`, aber `ausstehenderSourcesCommandFuerTest()` nicht leer und `bestaetigteSourcesCommandsFuerTest() == 0`; ein zweites `bindeSourcesHauptziel(A)` liefert `false`; nach `getStateInformation` kein Mitglied, Dirty 0.
- Erfolgskriterium: der angewandte Befehl wird bestätigt vorgemerkt (Mitglied nach dem Speichern) oder dem Aufrufer nachweislich gemeldet; kein dauerhaft gesperrter Handgriff.
- Gegenfall: derselbe Ablauf ohne Reconnect: bestätigt, Mitglied nach dem Speichern (M-16-Verhalten bleibt grün).

**8. Abgrenzung**
- T3-07-04: dort erreicht ein negativer ACK `v3Antwort` und verschwindet; hier erreicht ein positiver ACK `v3Antwort` nie, die Zuordnung bleibt und sperrt.
- T3-03-03: gleiche Reihenfolge `inFlightAck` vor Produktcallback, dort falsche Freigabe durch Zahlenüberlauf; hier gültiger ACK und Generationswechsel.
- NAK-278: Divergenz nach Reload ohne Zusage; hier ohne Reload, Zusage M-16 besteht, und die Wiederholung durch den User ist zusätzlich gesperrt.
- NAK-246 D4: dort löschte der Aufrufer die Zuordnung bei `false`; hier behält er sie, der Transport verbraucht den ACK.

**Vorschlag:** Kategorie **DEFEKT** · Schwere **medium** · Klasse **K2, K8** · Evidenzstufe **Quellherleitung (statisch)**; Eintrittshäufigkeit ungemessen.

---

### P15-V-02 — Messpunktwechsel einer Probeeq über State-Reload wird nie erkannt; Evidenz der alten Messposition bleibt gültig (M-55)

**1. Auslöser und Reihenfolge**
1. Host ruft `setStateInformation` auf derselben Probeeq-Instanz mit Bytes derselben Identität (`instanceId`, `projectBindingId`), aber anderer `measurement_position`. Unter `zustandSchloss`: `zustand = std::move (geladen)` (`SondeProcessor.cpp:1039`); danach bedingungslos `controlV3.reconnect()` (`:1070`).
2. Neuer Hello mit derselben `runtime_nonce`: sie ist je Instanz konstant (`SondeProcessor.h:519`, `SondeProcessor.cpp:138`, Hello `:1087-1088`).
3. Broker, altes Linkende: Client wird `stale`, Evidenz bleibt (`link.rs:557-564`). Neuer Hello unter dem Standlock: gleiche Nonce → keine Evidenzentfernung (`link.rs:131-143`); Deskriptor wird `None` (`link.rs:253-257`).
4. Erster Heartbeat mit `runtime` (neue Position): `alt` wird aus dem gerade geleerten Deskriptor gelesen (`liveness.rs:411-417`), `messpunktwechsel` entsteht nur bei `(Some, Some)` (`:423-425`); der Deskriptor wird vollständig ersetzt (`:471-472`). `invalidierung_wegen_messpunkt` (`:521-534`) wird nie erreicht.
5. Keine Ersatzwache: State-Laden startet keine neue Analysereihe der Sonde (Worker-Reset nur bei neuem Anlauf, `SondeProcessor.cpp:556-567`; `setStateInformation` `:1017-1072` ruft keinen), also kein Epochen- oder Segmentsprung, den `invalidierung_aus_transportbruch` sähe (`evidenz.rs:197` → `invalidierung_verdrahtung.rs:617-636`). `evidence_snapshot` trägt keinen Messpunkt; Folge-Evidenz der neuen Position landet in derselben Historie (`evidenz.rs:179-189`).
6. Einzige zufällige Ausnahme: ein Heartbeat, der zwischen `:1039` und `:1070` noch auf dem alten Link reist (1-Hz-Takt, `Verbindung.cpp:851-875`), sieht `(Some, Some)` und invalidiert.

Hinweis Gegenrichtung (heute ohne Sender, kein eigener Kandidat): derselbe Basisverlust macht beim Mixerkanal `None -> Some(k)` zum Wechsel (`liveness.rs:447-461`, `invalidierung_verdrahtung.rs:503-519`); ein Sender mit `host_mixer_index` verlöre bei jedem Reconnect alle Evidenz. Probeeq meldet keinen Index (`SondeProcessor.cpp:1162-1163`).

**2. Aktuelle Stellen (selbst gelesen)**
`broker/src/coordinator/link.rs:131-143`, `:253-257`, `:557-564`; `broker/src/coordinator/liveness.rs:320-323`, `:406-425`, `:471-472`, `:521-534`, `:720-744`; `eq-copilot/plugin/sonde/SondeProcessor.cpp:1039`, `:1070`, `:1087-1088`, `:1113-1134`; `eq-copilot/plugin/sonde/SondeProcessor.h:158-159`, `:519`.

**3. Produkteinstieg**
Verkabelung vollständig Produktcode (Host-API `setStateInformation` → Reconnect → Hello → Heartbeat). Ausdrücklich markiert: Die Vorbedingung „gleiche Identität, andere Position" hat im Produkt heute **keinen Autorenweg**. Probeeq hat keinen Editor (`SondeProcessor.h:158-159`), die Position stammt nur aus dem State (S5); erreichbar über State-Bytes derselben Instanz mit anderer Position (älterer gespeicherter Stand, externes Werkzeug). Strukturell folgt daraus: Für den einzigen Produktsender von Messpositionen läuft jede Positionsänderung über `setStateInformation`, das immer neu verbindet — die Heartbeat-Wache kann produktiv nur im Zufallsfenster aus Punkt 1.6 greifen.

**4. Verletzte Zusage**
- SONDE-013 M-55 (`docs/beweise/SONDE-013.md:550`): „Ein geänderter Messpunkt oder eine geänderte Messpunktklasse invalidiert die abhängige Evidenz." samt Nacharbeit 2: „der Messpunktwechsel invalidiert AUS DEM DESKRIPTORVERGLEICH heraus".
- Codezusage `liveness.rs:320-323`: „Der Heartbeat ist der Weg, auf dem eine laufende Sonde ihre Messposition meldet — ein Wechsel dort ist derselbe Gate-7-Fall wie ueber `descriptor_setzen`."

**5. Gegenpfad und stärkstes Gegenargument**
- `descriptor_setzen` vergleicht ebenfalls nur `(Some, Some)` (`liveness.rs:841-851`, `:888`) und hat keinen Produktaufrufer (`:852-858`).
- Nach dem Sonden-Linkende sperrt der Broker neue Evidenz der Sitzung (`link.rs:584-593`, T3-14-03). Das verzögert die nächste Rechnung, nimmt aber nichts zurück; nach Gens neutralem Neuaufbau rechnet die nächste angenommene Evidenz (`evidenz.rs:232`) über beide Positionen.
- Die Hypothesenrechnung liest die Position nicht (`hypothese_verdrahtung.rs:259-353`: Kanal, `pair_id`), die Vermischung bleibt dort unsichtbar; der PRE/POST-Join liest sie (`prepost_verdrahtung.rs:93-103`), ist aber produktiv tot (T3-08-04).
- Stärkstes Gegenargument: heute kein Autorenweg für die Vorbedingung. Dagegen: M-55 ist als im Produktpfad gemessen ausgewiesen, gemessen wurde über einen Setter ohne Produktaufrufer; der erste bedienbare Positionswechsel macht den Fall ohne Brokeränderung scharf.

**6. Bestehender Test und warum er nicht greift**
- `broker/tests/sonde013_verdrahtung.rs:1826-1855`: dreimal `descriptor_setzen` auf demselben Link, kein Reconnect.
- `broker/tests/sonde013_verdrahtung.rs:931-960`: ruft `invalidierung_wegen_messpunkt_fuer_link` direkt.
- `broker/tests/sonde014_gegenbeispiele.rs:1224-1248`: Kanalwechsel per Heartbeat, aber auf demselben Link.
- Keiner fährt Hello derselben Nonce zwischen zwei Positionen (S11).

**7. Minimaler Folgebeweis**
- Ausgangslage: A4-Bühne `coordinator()`; Main und `passive_probe`; Sonde per Heartbeat `runtime.messpunkt = "pre"`; drei angenommene Evidenzsnapshots.
- Ereignisfolge: `control_ende` des Sondenlinks → `control_hello_registrieren` mit gleicher Adresse und Nonce auf neuem Link → Heartbeat `runtime.messpunkt = "post"`.
- Erwartbares Fehlverhalten: `invalidierungen_zaehler() == 0`; alle Belege ohne `ausschlussgrund`.
- Erfolgskriterium: alle drei Belege tragen `messpunkt_wechsel`, Zähler 1.
- Gegenfall: derselbe Ablauf mit `pre` → `pre` invalidiert nichts. Plugin-Gegenstück: Probeeq mit State `pre`, `setStateInformation` mit State `post` derselben Identität gegen Testserver; erster Heartbeat des neuen Links trägt `post`.

**8. Abgrenzung**
- T3-08-04: gleiche Deskriptor-Schreibwege (`link.rs:253-257`, `liveness.rs:471-472`), dort fehlt `pair_id`; hier geht die Vergleichsbasis der Messposition verloren.
- `roh/phase-08-broker.md:96-103`: Inventar der Deskriptorschreiber ohne Folge für M-55.
- T3-14-03: Sperre neuer Evidenz; hier fehlende Rücknahme alter Evidenz.
- OF-6: DSP-Zustand (M-56), nicht Messpunkt.

**Vorschlag:** Kategorie **DEFEKT** · Schwere **medium** · Klasse **K4, K5** · Evidenzstufe **Quellherleitung (statisch)**; Vorbedingung heute ohne Autorenweg im Produkt (ausdrücklich markiert).

---

### P15-V-03 — Intent-Spiegel weist nach Same-Instance-Recall eines älteren Projektstands den Vollbestand als „ältere Revision" ab; der Broker rechnet weiter auf dem Intent vor dem Laden

**1. Auslöser und Reihenfolge**
1. Gen-Instanz (Bindung B, Epoche E) hat einen Vollbestand mit Revision `R_hoch` gemeldet; der Spiegel trägt `revision = R_hoch` (`intent.rs:526-538`).
2. Host ruft `setStateInformation` auf derselben Instanz mit Bytes derselben Bindung B und `intentBestandRevision = R_tief < R_hoch`. Unter `bindungMutex`: `zustand = geladen` (`State.cpp:204-216`); danach `controlV3.reconnect()` (`:221`).
3. `v3SessionEpoch` bleibt (`PluginProcessor.cpp:134`; `docs/beweise/NAK-246.md:4907`: „`v3SessionEpoch` wird nie beim Reload neu gewürfelt"); Wire-Adresse `Ipc.cpp:465-467` → gleicher SessionKey.
4. Broker, altes Linkende: die Sitzung hat weiter Clients (Gen `stale`, Sonden) → Spiegel bleibt (`link.rs:584-593`; geleert nur ohne Clients `:594-603`, `liveness.rs:58-75`, `:284-294`).
5. Neuer Link: `v3ControlLink(true)` → Subscribe → `sendeIntentVollbestand()` (`Ipc.cpp:1059`, `:809-816`) mit `bestand_revision = R_tief` (`Ipc.cpp:561-571`, `:586`).
6. Broker unter Standlock: `zuletzt = bestand.revision` (`intent.rs:504-505`), Vollbericht `revision < zuletzt` → `AeltereRevision` (`:512-518`). Kein Ersetzen, keine neue Generation, kein Veralten, kein Merker; Gen erfährt nichts (P1 ohne ACK).
7. Folge: `aufnahmen_sammeln` gibt den alten Spiegel an die Rechnung (`hypothese_verdrahtung.rs:229-232`, `:467`), die Vorschlagslage ebenso (`proposal_verdrahtung.rs:107`). Schutzangaben, Rollen und Beziehungen des vorher geladenen Stands wirken weiter; ein Veto, das nur der geladene ältere Stand trägt, fehlt beim Rechnen.
8. Dauer: bis Gen eine Revision größer `R_hoch` meldet, die Sitzung ohne Clients ist oder der Broker neu startet.

**2. Aktuelle Stellen (selbst gelesen)**
`broker/src/coordinator/intent.rs:504-518`, `:526-538`, `:588-590`, `:643-644`; `broker/src/coordinator/link.rs:584-603`; `broker/src/coordinator/liveness.rs:58-75`; `eq-copilot/plugin/src/prozessor/Ipc.cpp:548-571`, `:809-816`, `:1049-1059`; `eq-copilot/plugin/src/prozessor/State.cpp:203-221`; `eq-copilot/plugin/src/PluginProcessor.cpp:134`; `eq-copilot/plugin/state/NakamaState.cpp:979-982` (Revision persistiert), `:2214` (steigt nur je Änderung).

**3. Produkteinstieg**
Kette vollständig Produktcode. Zwei Einschränkungen, ausdrücklich markiert:
- (a) Die sieben Intent-Handgriffe haben keinen Editor-Aufrufer (S4). Heute trägt kein vom User erzeugtes Projekt eine Bestandsrevision größer 0; die Vorbedingung entsteht nur über State-Bytes aus Tests oder Werkzeugen.
- (b) Ob FL `setStateInformation` auf einer bestehenden Gen-Instanz mit älteren Bytes derselben Bindung ruft, statt die Instanz neu zu bauen, ist nicht verifiziert (offene Annahme). T3-07-06 setzt denselben Einstieg voraus.

**4. Verletzte Zusage**
- SONDE-014 M-09 (`docs/beweise/SONDE-014.md:723`): „Userwerte überleben Broker-Neustart und Projekt-Recall … Reihenfolge beim Laden: `MainProject` lesen → Intent rekonstruieren → erst danach Ranking oder Schutz anwenden." Rotfall dort: „oder der Spiegel überschreibt den Plugin-State".
- Zweck von M-86 (`docs/beweise/SONDE-014.md:882`): „ein Veto darf durch Transportverlust nie verschwinden".
- Codezusage `intent.rs:643-644`: „Der Spiegel ist nie autoritativ … nach einem Neustart meldet der Main seinen Vollbestand, und der gewinnt."

**5. Gegenpfad und stärkstes Gegenargument**
- M-85 (`docs/beweise/SONDE-014.md:881`) verlangt eine totale Revisionsordnung; die Abweisung schützt gegen einen verspätet zugestellten alten Vollbericht. Der Broker kann „verspätet" und „nach Recall legitim niedriger" nicht unterscheiden: der Vollbericht trägt weder Ladegeneration noch Linkbezug in der Ordnung.
- Nebenwirkung: der alte Assistentenschritt fällt nur bei angenommenem Vollbericht (`intent.rs:588-590`) und bleibt ebenfalls stehen (nicht weiter verfolgt).
- Stärkstes Gegenargument: (a) und (b). Dagegen: Riegel und Kette sind gebaut und als Produktpfad beschrieben; der erste bedienbare Intent-Handgriff macht den Fall ohne weitere Änderung erreichbar.

**6. Bestehender Test und warum er nicht greift**
- `broker/tests/sonde014_verdrahtung.rs:510-518` bestätigt gerade die Abweisung eines niedrigeren Vollberichts (Transportfall); der Kommentar `:368-371` setzt Monotonie voraus: „Die Bestandsrevision steigt im Main monoton; ein vollstaendiger Bericht traegt deshalb nie eine aeltere Zahl als eine schon gesehene Fortschreibung."
- `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:2757-2783`: Recall in einem neuen Prozessor, ohne Broker.

**7. Minimaler Folgebeweis**
- Ausgangslage: A4-Bühne mit Main-Link; Vollbestand `R = 4` mit Schutzangabe S für Quelle Q; Evidenz, ein Befund.
- Ereignisfolge: Main-Linkende → Hello derselben Adresse und Epoche → Vollbestand `R = 2` ohne S (der geladene ältere Stand) → ein weiterer Beleg.
- Erwartbares Fehlverhalten: `intent_update_json_grund_fuer_test` liefert `AeltereRevision`; `intent_sicht` trägt weiter S und `R = 4`; die neue Rechnung berücksichtigt S.
- Erfolgskriterium: nach dem Recall trägt der Spiegel den gemeldeten Stand ohne S (oder die Sitzung ist bis zur Bestätigung gesperrt); Befunde der alten Generation werden stale.
- Gegenfall: ein verspäteter Vollbericht `R = 2` auf demselben Link nach `R = 4` bleibt abgewiesen (`:510-518` bleibt grün). Plugin-Seite: derselbe Prozessor, Intent bis `R = 4`, Stand bei `R = 2` gespeichert, Recall derselben Instanz gegen Testserver; gesendete `bestand_revision` ist 2.

**8. Abgrenzung**
- T3-02-03: Zahlenbereich der Revisionen, nicht Ordnung nach Recall.
- T3-07-01: Auflösung Passagen- gegen Globalintent, nicht Spiegelannahme.
- NAK-253, NAK-283 R-283-1: Rückschreiben der Rechnung, nicht Annahme des Spiegels.
- `link.rs:596-602`: Phantom-Spiegel ohne Clients (Gegenrichtung, gelöst).

**Vorschlag:** Kategorie **DEFEKT** · Schwere **medium** · Klasse **K2, K4** · Evidenzstufe **Quellherleitung (statisch)** für Plugin- und Brokerkette; Hostverhalten in FL **offene Annahme**; Intent heute ohne Autorenweg (ausdrücklich markiert).

---

### P15-V-04 — Begonnene Snapshot-Übernahme läuft nach `controlEnde` weiter: Sicht wieder „abonniert", Befunde wieder handelbar, bis der nächste Link die Subscription neu beginnt

**1. Auslöser und Reihenfolge**
1. Client-Faden C (Link G): Generationsprüfung bestanden (`Verbindung.cpp:836`), `v3Antwort` (`Ipc.cpp:1096`) → `uebernehmeSessionSnapshot` liest erwartete Bindung und Sitzung (`SourcesModel.cpp:632-636`) und parst ohne Sperre (`:639-1233`).
2. Faden H: `reconnect()` (`ControlClient.cpp:147-163`) → `meldeLinkStatus(false)` synchron auf H (`:162`; `Verbindung.cpp:404-438`) → `v3ControlLink(false)` → `sourcesModel.controlEnde()` (`Ipc.cpp:900`) unter dem Modell-`mutex`: `subscriptionAktiv = false`, alle Befunde stale, Diagnose `brokerUnavailable` (`SourcesModel.cpp:555-585`).
3. C nimmt danach den `mutex` (`:1235`) und prüft nur Bindung und Sitzung (`:1236-1242`), die `controlEnde` nicht ändert; ersetzt die Befunde (`:1338`), setzt `subscriptionAktiv = true` (`:1355`) und Diagnose `keine` (`:1356-1358`).
4. Zustand nach dem Linkende: `mainDarfSchreiben` wahr (`SourcesModel.cpp:1637-1639`), `ready_to_send`-Befunde mit `darfAudition`/`darfDraft` wahr (`SourcesModel.h:187-189`).
5. Heilung: der nächste Link ruft `beginneSubscription` und leert (`Ipc.cpp:1035-1037`, `SourcesModel.cpp:539-553`); nach bewusstem Reconnect verbindet der Client ohne Wartezeit neu (`Verbindung.cpp:474-484`). Scheitert der Neuaufbau, korrigiert `setzeControlTransport` nur die Diagnose (`SourcesModel.cpp:593-609`), nicht `subscriptionAktiv` und nicht die Befundzustände.

**2. Aktuelle Stellen (selbst gelesen)**
`eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:406-409`, `:834-842`; `eq-copilot/plugin/core/ipc/ControlClient.cpp:147-163`; `eq-copilot/plugin/src/prozessor/Ipc.cpp:886-902`, `:1035-1037`; `eq-copilot/plugin/src/SourcesModel.cpp:555-585`, `:593-612`, `:631-638`, `:1235-1242`, `:1335-1362`, `:1637-1639`.

**3. Produkteinstieg**
Erreichbar: Reconnect-Knopf, Host-`prepareToPlay` oder Rollenwahl, während ein Sitzungssnapshot verarbeitet wird. Nicht betroffen ist der Projekt-Reload: `projektReload` leert erwartete Bindung und Sitzung (`SourcesModel.cpp:493-494`), dort greifen `:637-638` und `:1236-1242`. Heute ohne Handgriff an `darfAudition`/`darfDraft` (S9); `mainDarfSchreiben` schaltet Bind/Remove (`Ipc.cpp:1248`, `:1293`), deren Befehle als P0 eingereiht und nach dem Neuaufbau nachgespielt würden.

**4. Verletzte Zusage**
- NAK-214 R3 (`docs/beweise/NAK-214.md:46`): „Verbindungsende (`controlEnde`) und Sitzungswechsel setzen in Gen alle Befunde auf `stale` (nicht handelbar)".
- Codezusage `Verbindung.cpp:406-407`: „Ein Reconnect/stop darf die Subscription sofort entwerten, auch wenn der blockierte Read erst danach zurueckkehrt."

**5. Gegenpfad und stärkstes Gegenargument**
- NAK-214 V-26 (Test `Sonde014BefundTest.cpp:727-738`) erklärt einen vollständigen Folge-Snapshot nach `controlEnde` ausdrücklich zum Rückweg; das Modell kann den Snapshot des toten Links nicht von dem des neuen unterscheiden, weil im Produkt jeder neue Link zuerst `beginneSubscription` durchläuft.
- Stärkstes Gegenargument: flüchtig (bei laufendem Broker nur bis zum sofortigen Neuaufbau) und heute ohne Audition- oder Draft-Handgriff. Dauerhaft nur, wenn der Neuaufbau unmittelbar scheitert.

**6. Bestehender Test und warum er nicht greift**
- `eq-copilot/plugin/tests/Sonde014BefundTest.cpp:698-739` (V-23, V-26) und `Sonde012SourcesModelTest.cpp:364`, `:501`: `controlEnde` und Snapshot strikt nacheinander; keine über `controlEnde` hinweg begonnene Übernahme.

**7. Minimaler Folgebeweis**
- Ausgangslage: Modell wie V-23 (Subscription begonnen, Snapshot mit einem `ready_to_send`-Befund übernommen); produktnah: Prozessor gegen einen Testserver, der nach `subscribe_session` einen gültigen `session_snapshot` mit einem `ready_to_send`-Befund sendet.
- Ereignisfolge: zweiter Snapshot in Faden C angehalten (produktnah per `setzeV3AntwortHakenFuerTest`), Testserver stoppen, `v3ReconnectFuerTest()` (löst `controlEnde` aus), Haken freigeben.
- Erwartbares Fehlverhalten: danach `mainDarfSchreiben` wahr, Befund `ready_to_send`, `darfDraft()` wahr, obwohl kein Link besteht und der Neuaufbau scheitert.
- Erfolgskriterium: eine Übernahme, deren Link beendet ist, ändert die Sicht nicht (oder alle Befunde bleiben stale und `mainDarfSchreiben` falsch).
- Gegenfall: Snapshot nach einem neuen `beginneSubscription` wird übernommen (V-26 bleibt grün).

**8. Abgrenzung**
- T3-04-02: Rücknahme-Übernahme über `projektReload`, Rücknahmeleser ohne Recheck. Hier Snapshot-Übernahme über `controlEnde`; der vorhandene Recheck besteht, weil die Sitzung gleich bleibt.
- `roh/phase-04-lebenslauf.md:56` notiert „`controlV3.reconnect()` beendet keine bereits begonnene Übernahme synchron", wertet den Snapshotleser aber wegen seines Bindungs- und Sitzungs-Rechecks als geschützt; dieser Recheck deckt `controlEnde` nicht.
- NAK-269, T3-08-03: Zustellreihenfolge im Broker; hier Nebenläufigkeit im Gen ohne Reorder.
- P15-V-01 ist das Spiegelbild derselben Generationsprüfung (dort nach dem Wechsel unterdrückt, hier davor bestanden).

**Vorschlag:** Kategorie **HÄRTUNG** · Schwere **low** · Klasse **K2, K8** · Evidenzstufe **Quellherleitung (statisch)**.

---

## 3. Offene Prüffelder

### OF-1 — Hypothesenrechnung: nicht evidenzbasierte Eingaben ohne Rückschreibprüfung
- Unter dem Standlock gesammelt: Intent (`hypothese_verdrahtung.rs:229-232`), führendes Main (`:236-239`), bestätigte und nicht stale Clients (`:259-285`), Mixerkanal (`:296-305`), Paarmarke (`:328-337`), Taint- und Passagenwahl (`:434-446`). Zurückgeprüft werden nur Intentgeneration und verwendete IDs, beides nur bei nicht leerem Ergebnis (`:1193-1212`), sowie die Eingangsmenge (`:1213-1215`).
- Neurechnung lösen nur Evidenz, Heilung und die Merker aus S8 aus; `unbind_probe`, Führungs- oder Deskriptorwechsel setzen keinen. Ein über solche Wechsel hinweg gerechnetes Ergebnis wird eingetragen und bleibt bis zur nächsten Evidenz — ohne Wettlauf gilt dasselbe. Deshalb Regelfrage, kein nachgewiesener Rücksprung.
- Nebenpunkt: ein leeres Ergebnis unter alter Generation besteht (a) und (b) und räumt `stale`-Befunde (`:579-581`), statt sie nach §37.3 sichtbar stale zu lassen (Zitat in `intent.rs:595-600`); Dauer bis zur nächsten Evidenz.
- Kleinster Folgeschritt: A4-Fall mit `rechen_test_haken_ausloesen` (`:140`), der zwischen Sammeln und Rückschreiben `unbind_probe` der Kandidatenquelle oder einen Führungswechsel ausführt; messen, ob der veröffentlichte Befund den alten Stand trägt; danach Regel festlegen (Merker oder Generation für Mitgliedschaft und Führung).

### OF-2 — In-Flight-Replay von Sources-Befehlen nach Same-Instance-Reload
- `setStateInformation` leert die Zuordnungen (`State.cpp:134-136`), nicht das Register des ControlClients (einzige Löschung `Nachrichten.cpp:363`, S2). Ein vor dem Reload angenommener, unbestätigter `confirm_join` oder `unbind_probe` wird nach dem Reconnect nachgespielt (`Nachrichten.cpp:258-282`); bei gleicher Bindung passiert er die Brokerprüfung (`befehl.rs:515-527`), wird angewandt, und der ACK findet keine Zuordnung (`Ipc.cpp:1069-1071`).
- Wirkung wie NAK-278 (Divergenz Broker gegen geladenen State); der Fluss „Anwendung erst nach dem Reload durch Replay" steht dort nicht. Offen, weil NAK-278 die Regel ausdrücklich offen lässt.
- Kleinster Folgeschritt: B14-Fall „Bind bei angehaltenem Server-ACK → Reload derselben Bindung → Server wendet beim Replay an" und die Replay-Frage in die NAK-278-Entscheidung aufnehmen.

### OF-3 — Vorschläge ohne Rückschreibprüfung
- `vorschlaege_bilden` sammelt Befunde und Lage unter Lock (`proposal_verdrahtung.rs:38-46`), rechnet ohne Lock und schreibt unter neuem Lock ohne Vergleich mit dem Eingangsstand (`:49-60`); danach Persistenz und Zustellung (`:77-88`). Zwei Verbindungsfäden können so ältere Vorschläge über jüngere legen.
- Heute ohne sichtbare Wirkung: `capability_vorhanden` immer falsch (`:108`, Pflicht in `proposal.rs:954`), `vorschlaege` nicht in der Sicht (S19). Wirkung beschränkt auf die Store-Projektion `proposals`.
- Kleinster Folgeschritt: vor P6 A4-Fall mit Store und Haken zwischen `:46` und `:49`, zwei Rechnungen über verschiedene Befundstände; Projektion prüfen und R-283-1 auf Vorschläge ausdehnen.

### OF-4 — Experiment-Resultat: Messung und Anwendung in zwei Sperrabschnitten
- `resultatmessung` im ersten Lock (`experiment_verdrahtung.rs:223-227`, `:865`), Anwendung und Persistenz im zweiten (`:244-291`, `:309`) ohne Prüfung, ob Belege dazwischen ausgeschlossen wurden oder neue ankamen; Zustellung an Gen (`:356-360`).
- Produkteinstieg ohne Bedienung (`State.cpp:442-443`: „kein sichtbares Element").
- Kleinster Folgeschritt: A4-Fall mit Haken zwischen `:227` und `:245`, der einen Baseline-Beleg per Intervention ausschließt; prüfen, ob das Resultat ihn zählt.

### OF-5 — PRE/POST-Paarurteil ohne Rückschreibprüfung
- Halbzeuge unter Lock (`prepost_verdrahtung.rs:75-119`), `bilde_paare` ohne Lock, Einsetzen unter neuem Lock ohne Vergleich (`:146-156`); ein älteres Urteil kann ein jüngeres überschreiben.
- Produktiv unerreichbar (T3-08-04: keine `pair_id`).
- Kleinster Folgeschritt: nach der T3-08-04-Reparatur derselbe Interleaving-Fall wie M-13 für `paarurteile`.

### OF-6 — DSP-Zustandswechsel ohne Segmentierung der Evidenz
- `apply_transaction` (`invalidierung.rs:272-285`) hat keinen Produktaufrufer (S7); `state_report` speichert Revision und Hash ohne Folge für Evidenz (`liveness.rs:606-629`). Nach einem Same-Instance-Reload der Probeeq mit anderem DSP-Stand (Ladestart `NakamaTransaktion.cpp:234-270`) oder einem Gestus-Commit bleibt Evidenz des alten Klangzustands gültig.
- M-56 verschiebt den Apply-Pfad ausdrücklich nach P6/P7 (`docs/beweise/SONDE-013.md:551`).
- Kleinster Folgeschritt: Regel festhalten, ob ein Ladestart mit anderem `state_hash` als Apply zählt; Fall: Probeeq mit Evidenz, Reload mit anderem Hash, `ausschlussgrund` der alten Belege prüfen.

---

## 4. Verworfene Verdachte

| ID | Verdacht | Gegenbeleg | Bemerkung |
|---|---|---|---|
| V-01 | Ältere Hypothesenrechnung überschreibt ein jüngeres Ergebnis nach neuer oder zurückgenommener Evidenz | `broker/src/coordinator/hypothese_verdrahtung.rs:551-571`, `:1213-1215`, `:662-673`, `:169-179` | Prüfung unter demselben Standlock vor Einsetzen und vor Store-Annahme; geschlossen mit NAK-253/NAK-283 |
| V-02 | Ergebnis unter alter Intentgeneration wird nach Intent-Update veröffentlicht | `hypothese_verdrahtung.rs:1193-1204`; `broker/src/coordinator/intent.rs:622-626` | Veralten und Merker unter demselben Lock wie die Übernahme; leeres Ergebnis siehe OF-1 |
| V-03 | Vor dem Reload angekommener oder abgeholter Sources-ACK mutiert den geladenen State | `eq-copilot/plugin/src/prozessor/State.cpp:134-137`, `:204-210`, `:100-108`; `Ipc.cpp:1069-1076`, `:1576-1581`; `SourcesModel.cpp:373-377` | R-A1; Rest nach Reload NAK-278, OF-2 |
| V-04 | Ältere Mitgliederkopie eines Drains überholt eine jüngere Publikation | `Ipc.cpp:1518-1535`, `:1457-1459`; `SourcesModel.cpp:383-395` | Folge je Kopie, Sättigung |
| V-05 | Sitzungssnapshot der alten Bindung füllt nach `projektReload` die geleerte Sicht | `SourcesModel.cpp:493-499`, `:637-638`, `:1235-1242` | Recheck unter `mutex`; Rücknahmeleser ist T3-04-02 |
| V-06 | P2-Frame alter Nonce oder Sitzung mutiert eine Quellenzeile | `SourcesModel.cpp:1545-1558`; `broker/src/coordinator/senke.rs:157-164` | verspäteter Frame alter Identität wird abgewiesen; sein `p2_reject`-Marker hebt der nächste gültige Frame auf (`SourcesModel.cpp:1565-1570`) |
| V-07 | Verspätetes `control_ende` des alten Links räumt Client oder Alias des neuen | `broker/src/coordinator/link.rs:525-535`, `:557-564` | Aliasteilung und `current_link`-Vergleich |
| V-08 | Heartbeat eines verdrängten oder alten Links überschreibt den Deskriptor oder invalidiert für den neuen Link | `broker/src/coordinator/liveness.rs:337-399`, `:463-466` | Rückkehr vor Deskriptorschreiben und vor `:513-534` |
| V-09 | `evidence_snapshot` des alten Links oder alter Identität wird nach Neu-Hello angenommen (auch als P1-Replay `Verbindung.cpp:735-738`) | `broker/src/coordinator/evidenz.rs:142-156` | Adresse und `current_link` unter demselben Lock wie der Insert; gleiche Identität mit neuer Position ist P15-V-02 |
| V-10 | Verspätete Intent-Teilmeldung oder Meldung eines fremden Links überschreibt Jüngeres | `intent.rs:473-482`, `:504-518` | Ordnung je Objekt; legitime niedrigere Revision nach Recall ist P15-V-03 |
| V-11 | Analyseworker veröffentlicht nach Messreset ein Ergebnis der alten Reihe | `eq-copilot/plugin/src/prozessor/Analyse.cpp:160-202`, `:342-361` | Reset und Auswertung im selben `analyseSteuerMutex`-Abschnitt; Reload ohne Reset ist T3-07-06 |
| V-12 | Probeeq-Gestus über einen Reload hinweg committet alte Werte auf den geladenen Stand | `eq-copilot/plugin/sonde/SondeProcessor.cpp:1059-1060`, `:1330-1340`; `SondeProcessor.h:158-159` | Merker im Ladeblock geleert; ohne Editor erreicht keine Geste das Plugin (`SondeProcessor.cpp:1308-1310`); Hostmailbox ist T3-05-01 |
| V-13 | Transaktion mit Basisrevision vor dem Reload wird auf dem geladenen Stand angewandt | `eq-copilot/plugin/state/NakamaTransaktion.cpp:428-429`, `:256`, `:263`; `SondeProcessor.cpp:1343` | S1-Vergleich; einziger Produktaufrufer ist der Gestus; gleiche Revision nach Ladestart ist entschieden (T17, `docs/beweise/SONDE-015.md:1980`) |
| V-14 | Bind/Remove auf veralteter Anzeige | `Ipc.cpp:1292-1313`; `PluginEditor.cpp:181` | Hauptziel, Nonce und Mitgliedschaft unter `bindungMutex` neu geprüft; Labeltext ist T3-14-01 |
| V-15 | Älterer Sources-Befehl derselben Quelle überholt einen jüngeren beim Replay | `Ipc.cpp:1336-1341`; `Nachrichten.cpp:264-273` | höchstens eine Zuordnung je Quelle, FIFO-Replay; nach Reload siehe OF-2 |
| V-16 | ACK geht auf der sterbenden Pipe verloren, Zuordnung verwaist | `Nachrichten.cpp:258-282`, `:362-363`; `Verbindung.cpp:739`; `broker/src/coordinator/befehl.rs:529-537`; `controlclient/Vertrag.cpp:57-58`; `Ipc.cpp:1069-1076` | Replay und idempotenter ACK heilen; nur der vom Transport verbrauchte ACK verwaist (P15-V-01) |
| V-17 | `session_command` eines inzwischen nicht mehr führenden Mains wird angewandt | `befehl.rs:486-514`, `:567-579` | Führung und Dispatch im selben Standlock wie die Anwendung geprüft |
| V-18 | Veralteter Vorschlag wird nach Führungs- oder Intentwechsel als `draft_offer` zugestellt | `broker/src/coordinator/proposal_verdrahtung.rs:108`; `proposal.rs:954` | ohne Capability keine Zustellung; Store-Rest siehe OF-3 |
| V-19 | Intent-Spiegel einer beendeten Sitzung wird von einer gleichnamigen neuen geerbt | `link.rs:594-603`; `liveness.rs:58-75`, `:284-294`; `PluginProcessor.cpp:134` | Spiegel fällt mit dem letzten Client; neue Gen-Instanz hat neue Epoche |

---

## 5. Abgrenzungstabelle zu bekannten T3- und NAK-Punkten

| Bekannt | Kern des bekannten Punkts | Bezug in diesem Paket | Warum nicht doppelt |
|---|---|---|---|
| T3-03-03 | ACK-Revision ohne Rohzahlgrenze; `inFlightAck` vor Produktcallback | P15-V-01 | gleiche Reihenfolge, anderer Auslöser (Generationswechsel), gültiger ACK, Wirkung verwaiste Zuordnung und gesperrter Handgriff |
| T3-07-04 | negativer ACK verschwindet, Remove einer abwesenden Quelle wirkungslos | P15-V-01 | dort erreicht der ACK `v3Antwort`, hier nicht; dort `unknown_target`, hier `angewandt` |
| NAK-246 D4, M-16 | Aufrufer löschte die Zuordnung bei `false` | P15-V-01 (Zusage) | Zuordnung bleibt; der Transport verbraucht den ACK |
| NAK-278 | Divergenz Broker gegen geladenen State nach Reload, keine Zusage | P15-V-01, OF-2 | P15-V-01 ohne Reload, mit Zusage und gesperrter Wiederholung; OF-2 gleiche Wirkung über Replay |
| T3-08-04 | `pair_id` erreicht den Deskriptor produktiv nie | P15-V-02, OF-5 | gleiche Schreibwege; P15-V-02 betrifft die verlorene Vergleichsbasis der Messposition |
| `roh/phase-08-broker.md:96-110` | Inventar der Deskriptorschreiber | P15-V-02 | Inventar ohne M-55-Folge |
| T3-14-03 | sitzungsweite Evidenzsperre nach Sondenabbruch | P15-V-02 | Sperre neuer Evidenz, nicht Rücknahme alter |
| M-56 (SONDE-013) | Apply-Segmentierung nach P6/P7 verschoben | OF-6 | ausdrücklich verschoben, daher offen statt Kandidat |
| T3-02-03 | Revisionsbereiche State gegen Wire | P15-V-03 | Zahlenbereich, nicht Ordnung nach Recall |
| T3-07-01 | Passagenrolle verdrängt globalen Userintent | P15-V-03 | Auflösungsregel, nicht Spiegelannahme |
| NAK-253, NAK-283 R-283-1 | Rückschreiben der Hypothesenrechnung | V-01, V-02, OF-1, OF-3 | Befunde geschlossen; nicht evidenzbasierte Eingaben und Vorschläge offen |
| NAK-287 (a) bis (e) | Nachbefunde der Rückschreibenaht | OF-1 | dort Heilungstakt, Gleichheit, doppelte IDs, Dichte, Sitzungsreichweite; OF-1 betrifft ungeprüfte Eingaben |
| T3-04-02 | Rücknahme-Übernahme über `projektReload` ohne Recheck | P15-V-04, V-05 | dort Reload und Rücknahme; hier `controlEnde` und Snapshot mit bestandenem Recheck |
| `roh/phase-04-lebenslauf.md:56` | Skeptikernotiz „reconnect beendet keine begonnene Übernahme" | P15-V-04 | Snapshotleser dort als geschützt gewertet; Recheck deckt `controlEnde` nicht |
| NAK-214 R3, V-26 | `controlEnde` macht Befunde stale; Folge-Snapshot hebt auf | P15-V-04 (Zusage) | V-26 modelliert den Snapshot des neuen Links |
| NAK-269, T3-08-03 | Zustellreihenfolge Rücknahme gegen Snapshot im Broker | P15-V-04, V-05 | Brokerordnung, nicht Gen-Nebenläufigkeit |
| T3-08-02 | Evidenz hinter ihrer Rücknahme unmarkiert persistiert | V-09, OF-4 | Persistenzreihenfolge; OF-4 betrifft Messung vor Anwendung |
| T3-07-06 | lokale Messung und Befundkarten nach Reload | V-11 | bekannt, nicht neu gezählt |
| T3-05-01 | Hostmailbox überschreibt nach Reload | V-12 | bekannt, nicht neu gezählt |
| T3-14-01 | Labelentwurf nach Zielwechsel | V-14 | bekannt, nicht neu gezählt |
| T3-14-02 | Slot-Neubelegung übernimmt alte Dynamikhistorie | — | nicht erneut geprüft, kein Bezug zu den Kandidaten |
| T3-14-04 | Exportkollision | — | kein Bezug |
