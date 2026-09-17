# Tiefenaudit 3 · Phase 15 · Paket 3 — Erholung nach Teilfehlern im normalen Betrieb

> **Rolle dieses Berichts:** Rohbericht des Paketprüfers 3 (unabhängiger lesender Agent)
> im Tiefenaudit 3, Phase 15 vom 17.09.2026, Quellstand `e008811efa7650f3e0cc2332cc21f9e268cdd125`.
> Er ist Rohmaterial: Ketten von der Anforderung bis zur Wiederholung, Herleitungen,
> Gegenargumente, verworfene Verdachte und gelesener Scope. Verbindlich ist die Einordnung
> in [../BEFUNDE.md](../BEFUNDE.md), Abschnitt „Phase 15“, zusammen mit
> [phase-15-zentral.md](phase-15-zentral.md) (zentrale Quellprüfung, ID-Zuordnung, Zähler)
> und [phase-15-gegenpruefung-paket3.md](phase-15-gegenpruefung-paket3.md)
> (Widerlegungsversuch durch einen anderen Prüfer).
>
> Die Kennungen `P15-T-…` sind lokale Arbeitsnummern dieses Laufs, keine Befund-IDs.
> Kategorie- und Schwerevorschläge sind der Stand **vor** der Gegenprüfung: `P15-T-02` ist
> dort auf HÄRTUNG/low herabgestuft und auf die unehrliche Anzeige verengt, `P15-T-03` als
> **Duplikat zu NAK-188** eingeordnet und **nicht** als neuer Befund aufgenommen. Keine
> dynamische Reproduktion, kein Bau, kein Test, kein Host- oder Brokerlauf.


Stand: `HEAD e008811efa7650f3e0cc2332cc21f9e268cdd125` (per `git rev-parse HEAD` gelesen). Produktcode unverändert; der Arbeitsbaum entspricht dem Startzustand (fremde Einträge `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md`, `roh/phase-14-blindspots.md`, `briefing-hub/`, `nimbalyst-local/` unberührt). Nur gelesen: keine Builds, Tests, Starts, Installationen, FL-/MCP-Aufrufe; nur lesende git-Befehle.

Evidenzstufe aller Kandidaten: **Quellenherleitung** (statisch, Datei:Zeile selbst gelesen). Offene Annahmen stehen je Kandidat getrennt. **Keine dynamische Reproduktion** in diesem Lauf.

Ergebnis: **5 Kandidaten** (3 DEFEKT, 2 LÜCKE), **8 offene Prüffelder**, **15 verworfene Verdachte** (davon einer als Sammelzeile erkannter gewollter Sperren).

---

## 1. Gelesener Scope

### 1.1 Körperlektüre (Datei:Zeilen)

**Broker — Coordinator**
- `broker/src/coordinator/flush.rs:20-229` (`routing_bereit`, `routing_fail_closed`, `flush_session`), `:232-319` (`guards_persistieren`, `konflikt_guard_aufloesen`, `dispatch_fuer_link_erlaubt_locked`, `alias_quarantaenisiert`, `store_verweigert_fuer_link`)
- `broker/src/coordinator/liveness.rs:180-209` (Eviction), `:225-315` (`liveness_tick`), `:317-539` (`heartbeat_kontakt`), `:549-631` (`state_report_json_mit_minor`)
- `broker/src/coordinator/befehl.rs:157-472` (`persistenz_p0_intern`), `:474-616` (`session_command`), `:673-768` (Heartbeat-Zweig), `:806-862` (Interventionsende)
- `broker/src/coordinator/link.rs:80-338` (`control_hello_registrieren`), `:431-494` (Nachbericht, `resync_bestaetigen`), `:499-617` (`control_ende`)
- `broker/src/coordinator/intervention.rs:54-102`, `:260-439` (`hoermarkierung_v2`, `hoermarkierung_v2_getrennt`, Tail, `neutral_resync`), `:625-654` (`evidence_dispatch_locked`)
- `broker/src/coordinator/subscription.rs:14-242` (Subscribe, `resubscribe_snapshot_push`)
- `broker/src/coordinator/senke.rs:60-249` (Kopplung, P1-Dispatch, P2)
- `broker/src/coordinator/mod.rs:280-339` (`mit_uhr`, `mit_store`), `:588-697` (Interventionstests)
- `broker/src/coordinator/mitgliedschaft.rs:9-55` (Vorlektüre), `:57-71`, `:95-269` (Führung, Beitritt, `beitritt_noetig_locked`)
- `broker/src/coordinator/zustand.rs:440-589` (`Deckelausgang`, `kollisionsriegel_setzen_locked`, Guard-Index), `:582-624` (Default)
- `broker/src/coordinator/sicht.rs:236-252`, `:519-658` (`snapshot_locked`)
- `broker/src/coordinator/evidenz.rs:119-249`; `intent.rs:136-138`, `:470-529`, `:588-590`; `schema.rs:413-452`

**Broker — Store, Lebenslauf, Transport, Alias**
- `broker/src/store/writer.rs:36-109`, `:330-399` (Commit-/Kompaktierungs-/Guardfehler → `degradiere`), `:745-814` (`konflikt_guard_schreiben`, `konflikt_guard_loeschen`)
- `broker/src/store/handle.rs:145-271` (`append`, `senden`, Guardaufträge, `session_state_lesen`), `:480-509` (`kurze_leseconnection`, `degradiere`)
- `broker/src/store/mod.rs:200-230` (`StoreSicht::default`)
- `broker/src/lebenslauf.rs:138-189` (Produktstart, Supervisor-Tick), `:280-327` (Leerlaufende, Vorlektüre)
- `broker/src/server.rs:340-375` (`RegistrierteVerbindung::drop`), `:752-861` (v2-Link-ID, Heartbeat-Schleife)
- `broker/src/transport/server_v3/verbindung.rs:300-469`, `:583-635`, `:711-734`, `:810-889`; `trennung.rs:149-174`; `transport/bootstrap.rs:317-429`
- `broker/src/instance_alias.rs:60-379` (Registrierung, Quarantäne, Deckel, Auflösung)

**Plugin**
- `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:215-335` (`heartbeatSchritt`, `aufbauZug`), `:404-493` (Linkstatus, Backoff), `:495-1074` (`eineVerbindung`; gezielt nachgelesen `:715-769`, `:840-909`)
- `eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp:116-421` (`sendeP0`, `sendePersistenzP0`, `inFlightNachReconnect`, `inFlightAck`, `sendeP1`)
- `eq-copilot/plugin/core/ipc/controlclient/Intern.h:206-297`, `:437-447`
- `eq-copilot/plugin/core/ipc/IpcQueues.h:122-528` (P0-FIFO mit Reservierung, P1 mit Wiederholpuffer)
- `eq-copilot/plugin/core/ipc/TelemetryClient.cpp:389-464`, `:541-806`; `BrokerLifecycle.cpp:722-939`; `BrokerLifecycle.h:22-28`
- `eq-copilot/plugin/src/prozessor/Ipc.cpp:118-458`, `:484-544`, `:809-825`, `:851-1104`, `:1120-1123`, `:1196-1356`, `:1401-1588`
- `eq-copilot/plugin/src/prozessor/Analyse.cpp:777-1046`, `:1201-1309`
- `eq-copilot/plugin/src/prozessor/State.cpp:40-79`, `:140-229`, `:287-365`
- `eq-copilot/plugin/src/PluginProcessor.cpp:134-234`, `:437`, `:475`
- `eq-copilot/plugin/src/PluginEditor.cpp:179-197`, `:560-619` (Konfliktpanel), `:833-872`, `:900-947`, `:1196-1219`, `:1318-1320`
- `eq-copilot/plugin/src/SourcesModel.cpp:456-513`, `:536-621`, `:1318-1364`, `:1930-1945`
- `eq-copilot/plugin/sonde/SondeProcessor.cpp:138-142`, `:195-196`, `:212-230`, `:529-708`, `:734-896`, `:1030-1104`, `:1190-1364`, `:1413-1444`; `SondeProcessor.h:150-164`
- `eq-copilot/plugin/state/NakamaTransaktion.cpp:591-593`, `:639-770`

**Tests**
- `broker/tests/store_crash_matrix.rs:255-299` (`echte_cpp_client_crashrunde`), `:600-699`, `:1157-1172`, `:1399-1408`, `:1540-1584`, `:2179-2189`, `:2846-2904`, `:2960-3029`
- `broker/src/bin/eqcop-store-crash-worker.rs:81-133` (`PhaseBSenke`)
- `broker/tests/coordinator_model.rs:880-949`, `:1380-1415`
- `broker/src/transport/server_v3/tests_abonnement.rs:494-547`, `:1180-1259`
- `broker/tests/sonde012_sources_slice.rs:170-199`, `:344-376`, `:1170-1197`; `broker/tests/sonde013_verdrahtung.rs:2273-2298`
- `eq-copilot/plugin/tests/IpcTestMain.cpp:5662-5803`

**Zusagen, Register, bekannte Befunde**
- `docs/beweise/SONDE-011.md` Zeilen 165, 513, 514, 516, 533, 535, 548, 560, 596-602, 624-630, 670, 678, 1061
- `docs/beweise/SONDE-013.md` Zeilen 524, 537, 561, 562; `docs/beweise/SONDE-014.md:823` (M-73)
- `docs/FL-Nakama-Sonden-Design-Entwurf.md:1836-1865` (§32.1), `:2093-2098` (§33.3), `:2150-2166` (§33.5)
- `eq-copilot/schemas/state/nakama-state-v2.md:264-274` (§7 Duplicate)
- `docs/offene-punkte.md`: NAK-118 (119), NAK-120 (113), NAK-124 (124), NAK-125 (162), NAK-126 (163), NAK-138 (170), NAK-164 (191), NAK-165 (192), NAK-181 (485), NAK-188 (212), NAK-274 (284), Zeile 431 (Probeeq-Rückfallfläche/S31b); weitere NAK-155 bis NAK-196 und NAK-268 bis NAK-287 aus der Vorlektüre
- `docs/beweise/NAK-121.md` Zeilen 136, 228, 379, 1499-1500
- `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (abschnittsweise und per Suche): T3-01-01 bis T3-14-04, insbesondere Zeilen 380, 401, 413, 436-441, 451-453, 466-470, 481-485, 507-553, 588-591
- `docs/audits/2026-09-15-tiefenaudit/roh/phase-14-blindspots.md` Zeilen 43, 57, 65 und Umfeld; `roh/phase-04-lebenslauf.md` Zeilen 125, 131, 167, 188; `roh/phase-08-broker.md` Zeilen 90, 173, 190
- `CLAUDE.md:51-52`

### 1.2 Gezielte Suchen (nicht als Volllektüre gezählt)
- `konflikt_guard_aufloesen|quarantaene_aufloesen` über `broker/`: Produktcode nur `flush.rs:247/250/277`, `handle.rs:182`, `instance_alias.rs:302/336/348`; Aufrufer sonst ausschließlich `broker/tests/store_crash_matrix.rs:2144,2171,3637-3707,3962`.
- `heartbeat_ack` / `duplicate_instance_id` über `eq-copilot/plugin`: v3-Flag nur in `tests/V3TestServer.h:484, 506, 636`; kein Produktleser. v2-`konflikt` in `PipeClient.*`.
- `neueSensorId|Duplikat|dupliz` über `eq-copilot/plugin`: Neu-ID nur `src/prozessor/State.cpp:54`, Aufruf `src/PluginEditor.cpp:591`.
- `beginneVersuch \(|urteileVersuch \(|assistentVersuchStarten \(|assistentAntwort \(` über Plugin: nur `Analyse.cpp`, `PluginProcessor.h`, `tests/Sonde013PassageStateTest.cpp`, `tests/Sonde014AssistentTest.cpp`; `processor\.(assistent|beginne|urteile|brich|…)` in `src/`: kein Treffer.
- `sendePersistenzP0`: Produktaufrufer `Analyse.cpp:783`, `:1273`, `Ipc.cpp:1351`.
- `hoermarkierung_v2|v2_getrennt`: `intervention.rs:29-33`, Tests `mod.rs:592-687`, `coordinator_model.rs:1405-1407` (nur v3-Link-IDs).
- `fuehrendes_main|fuehrung_neu_bewerten`, `verdraengt`, `tombstone`, `fn session_des_links` (Kontext `intervention.rs:40-56`), `fn aus_adresse` (Kontext `zustand.rs:10-32`), `fn effektive_adresse` (Kontext `mod.rs:457-470`), `guard_gesetzt(`, `routing_bereit\s*[:=]`, `degradiert = false`, `MAX_KONFLIKT_GUARDS` (Kontext `store/mod.rs:58-66`, `zustand.rs:501-508`), `alias_quarantaenisiert|session_push_erlaubt`, `RoutingNichtFreigegeben` (Kontext `zustand.rs:253-282`), `interventionssenke` (`server.rs:348-471`, `lebenslauf.rs:152`), `idle|Leerlauf` (`lebenslauf.rs:8-51`, `:280-326`).
- Plugin: `v3SessionEpoch|v3RuntimeNonce`, `mainDarfSchreiben` (Kontext `SourcesModel.cpp:1633-1639`), `doppelteId` (Kontext `:1173-1179`, `:1228-1234`), `confirmationRequired|diagnoseHatHandgriff`, `p2_reject` (u. a. `PluginEditor.cpp:1284-1288`), `verdr|supersed|abgeloest` in `core/ipc`, `enum class Diagnose` (Kontext `SourcesModel.h:39-61`), `std::string EqCopilotProcessor::v3SubscribeJson`.
- Doku/Register: `Konfliktriegel|Neu-ID|Quarantäne|Duplikat|Kennung|Rückfallfläche|hasEditor` in `docs/offene-punkte.md`, `docs/plugin-wissen.md`, Entwurf, Blueprint, `eq-copilot/schemas/state/`; `degrad|routing_fail_closed|routing_bereit|Platzhalter|Ping|Sturm|Schleife|verdr|Konflikt` im Auditordner.

### 1.3 Arbeitsweise und Abweichung
- `BEFUNDE.md` war für eine Lektüre zu groß: abschnittsweise plus Suche. Lange Registerzeilen per `awk` ausgegeben.
- **Abweichung von „Einzige Schreibstelle“:** In der Vorlektüre entstanden per Shell-Umleitung zwei Hilfsauszüge im selben Scratchpad-Ordner: `op-1-179.txt` und `op-209-259.txt` (Zeilenauszüge aus `docs/offene-punkte.md`). Im Repo wurde nichts geändert oder angelegt.
- Außerhalb: Sicherheit (Pipe-ACLs, Impersonation, Authentisierung, Token, Signaturen, Diagnose-Berechtigung, Installer, T3-12-*) nicht geprüft; keine Gestaltungsvorschläge.

---

## 2. Kandidaten

### P15-T-01 — Degradierter Store: jeder Heartbeat trennt seinen Absender, statt „sichtbarer gezählter Betrieb, P2 läuft weiter“

**DEFEKT · high · K2 (Lebenslauf) + K8 · Quellenherleitung** (Auslösehäufigkeit und P0/P1-Reihenfolge beim Subscribe: offene Annahmen)

| Schritt | Ort | Befund |
|---|---|---|
| Anforderung | `Verbindung.cpp:757`, `:851-875`; `SondeProcessor.cpp:139-142`, `:195-196` | Jeder v3-Client (Gen und jede Probeeq) reiht beim Verbinden sofort und danach im 1-Hz-Takt einen Heartbeat (P0 `bericht`) ein; bei Änderung zusätzlich `state_report` (P1, `:866`). |
| Lokale Zustandsänderung | — | Keine; reine Lebens- und Statusmeldung. |
| Übergabe | `befehl.rs:673-760` → `liveness.rs:317-537`; `liveness.rs:549-631` | Der Coordinator markiert die Sitzung dirty (`liveness.rs:505-506`) und flusht mit `Some(link_id)` als Verursacher (`:535-537`). |
| Annahme/Ablehnung | `flush.rs:148-169`; `handle.rs:219-223` | Der Append des abgeleiteten Session-Snapshots wird vom degradierten Store sofort verweigert; `store_verweigerungen += 1`, Verursacherlink `trennen = true`, Rückkehr ohne Push. |
| Abschluss | `liveness.rs:264-269`; `lebenslauf.rs:178-187` | Der Supervisor-Tick (100 ms) schließt den Link. Startvariante zusätzlich: `routing_bereit = false` (`mod.rs:316`) sperrt P2 (`senke.rs:149-155` über `flush.rs:289-292`) und alle Flush-Pushziele (`flush.rs:114`); Riegelrestore entfällt (`mod.rs:317-320`). |
| Wiederholung/Rücknahme | `Verbindung.cpp:440-493` (Backoff-Reset `:448-452`); `handle.rs:495-502`; `store/mod.rs:217-221`; `zustand.rs:624` | Der Client verbindet nach 500 ms neu und läuft beim ersten Heartbeat in denselben Abbruch. Die Degradation ist einseitig (einziger Setzer auf `false` ist der Default); `routing_bereit` wird nach der Konstruktion nie wieder wahr. Der Broker endet nur nach 60 s ohne Clients (`lebenslauf.rs:45`, `:280-281`), also nie bei offenem FL. |

**(1) Auslöser und Ablauf.**
- *Startvariante:* `StoreKonfiguration::standard()` scheitert → `StoreWriter::degradiert_ohne_pfad` (`lebenslauf.rs:143-146`), oder `StoreWriter::starten` degradiert (Remote-Volume, neueres Schema, Öffnungs-/Migrationsfehler; `writer.rs:36-109`).
- *Laufzeitvariante:* ein einzelner Commitfehler (`writer.rs:345-353`), Kompaktierungsfehler (`:365-370`), Guard-Schreib- oder Löschfehler (`:372-389`). `tests_abonnement.rs:1206-1217` zeigt im Repo genau diesen Weg: eine zweite Verbindung hält `BEGIN IMMEDIATE` länger als `BUSY_TIMEOUT_MS`, der Writer degradiert dauerhaft.
- *Threads und Sperren:* P0-Consumer je Link (`verbindung.rs:583-614`) → `heartbeat_kontakt` nimmt und löst den Standlock (`liveness.rs:331-509`) → `flush_session` nimmt Shard-Schloss (`flush.rs:60-63`) und Standlock (`:81-136`), ruft außerhalb des Standlocks `store.append` (`:158`), das in `senden` sofort `Degradiert` liefert (`handle.rs:219-223`), und setzt unter erneutem Standlock `trennen` (`flush.rs:161-167`). Der Tick-Thread `nakama-coordinator-tick` schließt (`lebenslauf.rs:181-185`). Clientseitig beendet `eineVerbindung` den Lauf, `threadLauf` verbindet neu.
- *Folgen:* Alle v3-Clients kreisen im Abstand von rund einer halben bis einer Sekunde. Jede Trennung löst die Telemetriekopplung (`verbindung.rs:810-889`, `trennung.rs:149-174`), P2 kommt höchstens in kurzen Fenstern an (Laufzeitvariante) oder nie (Startvariante, Routinggate). Gen sieht `store_degraded` nur über den Resubscribe-Livestand (`subscription.rs:93-104`, `:133-135`, `:175-199`; `sicht.rs:657-658`) — und nur, wenn der P1-Subscribe verarbeitet wird, bevor der P0-Heartbeat den Link auf `trennen` setzt (`subscription.rs:93`); beide laufen auf getrennten Consumern (`verbindung.rs:583-635`), die Reihenfolge ist **offene Annahme**. Die Gen-Diagnose wechselt dadurch zwischen `authenticating` (`SourcesModel.cpp:550`, `:593-596`), `brokerUnavailable` mit Reconnect-Handgriff (`:577-583`, `:597-608`; Knopf `PluginEditor.cpp:938-943`, der nichts heilen kann) und allenfalls kurz `storeDegraded` (`SourcesModel.cpp:1356`; Text „Store degraded - no client recovery command“, `PluginEditor.cpp:1212-1213`).

**(2) Quellenstellen:** wie Kettentabelle; zusätzlich `flush.rs:24-37`, `handle.rs:148-171`, `mod.rs:309-333`, `senke.rs:125-156`, `subscription.rs:74-242`.

**(3) Produkteinstieg:** erreichbar. Jeder Gen- und Probeeq-Prozessor mit v3-ControlClient (`PluginProcessor.cpp:134-234`; `SondeProcessor.cpp:139-142`, `:195-196`); Store-Start im Produktbroker `lebenslauf.rs:143-151`.

**(4) Verletzte Zusage (wörtlich):**
- `docs/beweise/SONDE-011.md:535` (S-03): „Bei vollem Kanal oder degradiertem Store wird die Annahme persistenzpflichtiger Ereignisse verweigert und der Verursacher getrennt; nichts Unhaltbares wird bestätigt. Disk-full, I/O-Fehler, beschädigte/neue DB stoppen nicht den Broker: sichtbarer gezählter Betrieb ohne Historie, P2 läuft weiter.“
- `docs/beweise/SONDE-011.md:629` (L-08): „Disk-full, I/O-Fehler oder beschädigte DB stoppen den Broker nicht: sichtbar gezählter Betrieb ohne Historie, keine falsche Annahmebestätigung, P2 läuft weiter.“
- `docs/beweise/SONDE-011.md:533` (S-01): „Remote-Volume, neueres Schema oder Storefehler öffnen/schreiben nichts und führen sichtbar in L-08-Degradation; Routing bleibt bis zum Konfliktriegel-Restore fail-closed, P2 darf weiterlaufen.“

**(5) Gegenpfad und stärkstes Gegenargument.** Stärkstes Gegenargument: Die Verursachertrennung ist gewollt und getestet (`store_crash_matrix.rs:1558-1576` erwartet genau, dass ein Report bei degradiertem Store trennt; L-08: „Wiederholung kommt vom Sender-Reconnect“). Gegenprüfung: (i) Der Heartbeat trägt kein persistenzpflichtiges Userereignis; erst die Implementierung macht den Flush des abgeleiteten Snapshots zur verweigerbaren Annahme (`flush.rs:148` prüft nur, ob ein Store existiert, nicht `store_degradiert()`; der Resubscribe kennt dagegen einen Degradationszweig, `subscription.rs:133-135`, `:188`). (ii) Weil jeder Client im 1-Hz-Takt meldet, wird die einmalige erlaubte Trennung zur periodischen Trennung aller Clients; dadurch sind „sichtbarer gezählter Betrieb“ und „P2 läuft weiter“ derselben Zusage unerreichbar. (iii) In der Startvariante verweigert das Routinggate P2 unabhängig von der Schleife. Gegenpfad Erholung: keiner — kein Rücksetzer der Degradation, kein Setzer für `routing_bereit`, kein Leerlaufende bei aktiven Clients. Befund ist die Kombination, nicht die einzelne Trennung.

**(6) Vorhandene Tests und warum unzureichend.**
- `store_crash_matrix.rs:1546-1556`: nur der Writer verweigert; kein Coordinator.
- `store_crash_matrix.rs:1558-1576`: ein Report → Trennung; kein Reconnect, kein zweiter Heartbeat, kein P2, keine Gen-Sicht.
- `store_crash_matrix.rs:2850-2900` (`blockierter_store_writer_bestaetigt_nichts_und_p2_laeuft`): *blockierter*, nicht degradierter Writer (`routing_bereit` bleibt wahr); Probe per `control_registrieren` und `telemetrie_gekoppelt` ohne Heartbeat und ohne Tick.
- `sonde012_sources_slice.rs:1173-1197` (`store_degraded_travels_only_as_true`): Main meldet sich an und abonniert **ohne** Heartbeat; mit dem im Produkt sofort folgenden Heartbeat wäre der Link getrennt.
- `tests_abonnement.rs:1187-1256`: Laufzeitdegradation und Livestand beim Resubscribe; kein Heartbeat danach.

**(7) Kleinster Folgebeweis.**
- *Ausgangslage:* (a) `starten_degradiert` + `Coordinator::mit_store` wie `store_crash_matrix.rs:1558-1565`; (b) gesunder Store, Laufzeitdegradation per `BEGIN IMMEDIATE` wie `tests_abonnement.rs:1206-1217`. Jeweils `ManualClock`, `PushProbe`; Main und Probe registriert, Probe `telemetrie_gekoppelt`, Main abonniert.
- *Ereignisfolge:* fünf Runden aus Uhr +1000 ms, Heartbeat-Report von Main und Probe, `liveness_tick`, für jeden zurückgegebenen Link Neu-Hello derselben Adresse (Main zusätzlich Subscribe, Probe erneut gekoppelt), ein P2-Frame der Probe.
- *Erwartbares Fehlverhalten heute:* `liveness_tick` liefert in jeder Runde beide Links; `store_verweigerungen` wächst je Runde um mindestens 2; in (a) bleibt `p2_live_frames` 0; Pushes höchstens als Resubscribe-Livestand.
- *Erfolgskriterium:* Heartbeats allein trennen nicht; `p2_live_frames` wächst und P2 erreicht den Abonnenten; ein Snapshot mit `store_degraded: true` bleibt zustellbar; ein persistenzpflichtiger P0 bekommt keine falsche Annahme.
- *Gegenfall:* gesunder Store trennt nie; voller Storekanal (`KanalVoll`, `handle.rs:239-241`) bei einem persistenzpflichtigen Befehl darf den Verursacher weiter trennen.

**(8) Abgrenzung.** NAK-125 (`offene-punkte.md:162`): derselbe Append-Fehlerpfad, dort beim `session_command` mit falscher Quittung „angewandt“ — hier Dauertrennung aller Clients durch den Heartbeat-Flush und P2-Sperre. NAK-164 (`:191`): persistenter P0 ohne Antwort bei Append-Fehler, nicht neu gezählt. NAK-274 E5-2 (`:284`): Ordnungsrand am Übergang gesund→degradiert. T3-08-01: Group-Commit-Teilung. Keine T3-Zeile zur Degradation (Suche im Auditordner ohne Befundtreffer).

**Vorschlag:** Heartbeat-/Report-Flush bei degradiertem Store nicht als verweigerte Annahme mit Verursachertrennung behandeln (Livestand ohne Commit, wie der Resubscribe), das P2-Routing nicht an die Storegesundheit binden, wie S-01 zusagt, und eine stabile `store_degraded`-Diagnose sicherstellen; A4-SI-Fall aus (7).

---

### P15-T-02 — Ein einzelner Store-Lesefehler beim Resubscribe schaltet Routing brokerweit dauerhaft ab; Gen bleibt ohne Hinweis auf „Authenticating“

**LÜCKE · medium · K2 + K8 · Quellenherleitung** (Auslösehäufigkeit: offene Annahme)

| Schritt | Ort | Befund |
|---|---|---|
| Anforderung | `Ipc.cpp:1031-1041` | Gen abonniert nach jedem (Re-)Link `subscribe_session` (P1). |
| Lokale Zustandsänderung | `SourcesModel.cpp:539-553` | `subscriptionAktiv = false`, Diagnose `authenticating`, kein Handgriff. |
| Übergabe | `senke.rs:97-99` → `subscription.rs:18-72` | Subscription eingetragen (`:62-68`), `resubscribe_snapshot_push` (`:70`). |
| Annahme/Ablehnung | `subscription.rs:137-146`; ebenso `:151-155`, `:163-168`, `:201-206`, `:217-219` | Ein Fehler von `session_state_lesen` (kurze Read-only-Verbindung mit 2000 ms Busy-Frist, `handle.rs:483-493`) oder ein Projektionsvertragsbruch ruft `routing_fail_closed`. |
| Abschluss | `flush.rs:31-37` | `routing_bereit = false` und `trennen` für **alle** Links aller Sitzungen und Projekte; der Tick schließt sie. |
| Wiederholung/Rücknahme | `link.rs:80-338`; `subscription.rs:93-94`; `mod.rs:316`; `zustand.rs:624` | Neu-Hellos werden angenommen, Subscribes eingetragen, doch der Resubscribe kehrt vor jedem Push zurück. Kein Setzer macht `routing_bereit` wieder wahr. |

**(1) Auslöser und Ablauf.** Der P1-Consumer des Gen-Links (`verbindung.rs:616-635`) ruft Subscribe (Standlock `subscription.rs:34-69`, dann frei), `resubscribe_snapshot_push` nimmt den Standlock (`:86-126`), liest außerhalb (`:137-146`) und nimmt für `routing_fail_closed` erneut den Standlock (`flush.rs:32`). Danach: gesunde Heartbeat-Flushes trennen nicht mehr (Unterschied zu T-01) — der Zustand ist stabil tot. P2 → `routing_nicht_freigegeben` (`senke.rs:149-155`), Auto-Join aus (`mitgliedschaft.rs:17-19`), Bind/Remove → `abgelehnt/unauthorized` (`befehl.rs:493-513`), negativer ACK still gelöscht (`Ipc.cpp:1066-1077`). Gen verharrt auf „Authenticating“ (`SourcesModel.cpp:593-596`, `:1940`) ohne Handgriff (`:551`, `:616`). Nebenbefund am Draht: `heartbeat_ack.duplicate_instance_id` meldet für **jeden** Link `true`, weil `alias_quarantaenisiert` `!routing_bereit` einrechnet (`flush.rs:300-304`; `befehl.rs:761-764`); ein v3-Leser existiert nicht, daher heute ohne Anzeigewirkung.

**(2) Quellenstellen:** wie Tabelle; `handle.rs:256-271`, `lebenslauf.rs:45`, `:280-281`.

**(3) Produkteinstieg:** erreichbar über jeden Gen-Linkaufbau (`Ipc.cpp:1035-1041`) bei transientem Lesefehler (z. B. `SQLITE_BUSY` über 2000 ms, Öffnungsfehler durch fremde Dateisperre) oder Projektionsvertragsbruch; zusätzlich Guard-Persistfehler (`link.rs:323-331`, `flush.rs:235-244`), der den Store zugleich degradiert (`writer.rs:372-377`, → T-01). Häufigkeit: **offene Annahme**.

**(4) Zusage.** Die Sperre selbst ist dokumentiert gewollt: `docs/beweise/SONDE-011.md:1061` „`resubscribe_snapshot_push`/`routing_fail_closed`: Store-Lesefehler sperren Routing und markieren den Link zum Trennen; kein leerer Fallback.“ Für Wiederanlauf, Umfang und Hinweis: **keine Zusage** → LÜCKE. Anzeigebezug: `CLAUDE.md:51-52` „jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich einen Zustand“ — „Authenticating“ beschreibt den Zustand falsch.

**(5) Gegenpfad und stärkstes Gegenargument.** Stärkstes Gegenargument: fail-closed ist richtig; ein automatischer Wiederanlauf könnte eine beschädigte Projektion als gültig ausliefern. Gegenpfad geprüft: einziger Rückweg ist ein Brokerneustart, der bei offenem FL nicht eintritt (`lebenslauf.rs:280-281`). Befund ist der fehlende Erholungs- und Hinweisweg und die brokerweite Reichweite eines sitzungsbezogenen Lesefehlers, nicht die Sperre.

**(6) Vorhandene Tests.** `store_crash_matrix.rs:1157-1172` (`projektionslesefehler_haelt_subscription_sichtbar_fail_closed`) misst `routing_bereit() == false` nach dem Lesefehler; weder Reconnect, fremde Sitzung, Gen-Diagnose noch Wiederanlauf. Matrixzeile `SONDE-011.md:1061` belegt nur die Sperre.

**(7) Kleinster Folgebeweis.**
- *Ausgangslage:* Coordinator mit gesundem Store; zwei Sitzungen (zwei Projekte) mit je Main und Probe; beide Mains abonniert; eine Naht oder Dateisperre, die genau einen `session_state_lesen` scheitern lässt.
- *Ereignisfolge:* Resubscribe von Main 1 mit Lesefehler → Tick → Neu-Hello aller Links → Subscribes → Heartbeats → P2 → `confirm_join` von Main 2; danach Lesepfad gesund, 60 s ManualClock.
- *Erwartbares Fehlverhalten:* `routing_bereit()` bleibt `false`, keine Pushes, P2 `routing_nicht_freigegeben`, `confirm_join` `unauthorized` auch in Sitzung 2; SourcesModel (B-Test mit Fixture-Snapshotfolge) bleibt `authenticating`.
- *Erfolgskriterium:* definierter Wiederanlauf nach erfolgreichem erneuten Lesen bzw. Restore oder eine eigene ehrliche Diagnose; Sitzung 2 unbeeinträchtigt oder begründet mitgesperrt.
- *Gegenfall:* dauerhaft beschädigte Projektion bleibt fail-closed; kein leerer oder erfundener Snapshot verlässt den Broker.

**(8) Abgrenzung.** T-01 (degradierter Writer; hier gesunder Writer, Leseweg). NAK-165 (`offene-punkte.md:192`): Outbox-Nachspielen nach gescheitertem Push. NAK-188 (`:212`): Platzhalter-Evidenzsperre, anderer Mechanismus. Keine T3-Zeile.

**Vorschlag:** Für `routing_fail_closed` nach Lesefehler einen definierten Wiederanlauf festlegen (erneuter Lese-/Restoreversuch) oder den Zustand ehrlich melden (eigene Diagnose statt „Authenticating“); die Reichweite auf die betroffene Sitzung prüfen; das Duplikatflag nicht aus `!routing_bereit` ableiten.

---

### P15-T-03 — Jedes v2-Verbindungsende setzt den unzuordenbaren Platzhalter auf unknown und sperrt Evidenz brokerweit; bei stehendem Main-v3-Link gibt es weder Löser noch Hinweis

**LÜCKE · medium · K2 (+K8) · Quellenherleitung** (Reihenfolge v2-Drop gegen v3-Neuaufbau beim Reload: offene Annahme)

| Schritt | Ort | Befund |
|---|---|---|
| Anforderung | `server.rs:761-767` | Jede v2-Verbindung erhält die Interventionssenke (Produkt: der Coordinator, `lebenslauf.rs:152-158`) und die Link-ID `v2:<len>:<sensor>:<nonce>`. |
| Lokale Zustandsänderung | `PluginProcessor.cpp:475`; `State.cpp:175`, `:218-219`, `:360`, `:64` | Plugin-Destruktor, read-only-Reload, Reload, Bindungsänderung, neue Kennung beenden bzw. erneuern die v2-Verbindung. |
| Übergabe | `server.rs:352-371` → `intervention.rs:32-33` | `Drop` ruft `senke.getrennt(interventions_link_id)`. |
| Annahme/Ablehnung | `intervention.rs:299-307`, `:40-45` | Die v2-ID steht nie in `stand.links` → `SessionKey::unbekannt()`; `unknown = true` **bedingungslos**, auch ohne je aktiven v2-Marker. |
| Abschluss | `intervention.rs:632-647` | Der Platzhalter sperrt Evidenz in **jeder** Sitzung (`evidence_gesperrt`). |
| Wiederholung/Rücknahme | `intervention.rs:401-434` (`:432`); `befehl.rs:732-734`, `:756`; `link.rs:480-494` | Einziger Löser ist `neutral_resync` über den ersten neutralen Heartbeat oder den Nachbericht eines **neuen** Main-v3-Links. Bei stehendem Main-Link kein Auslöser; ACK und Snapshot tragen keinen Hinweis (`befehl.rs:759-767`; `sicht.rs:576-660`). |

**(1) Auslöser und Ablauf.** Der v2-Worker-Thread liest EOF oder bricht bei Protokollfehler ab (`server.rs:786-839`, `break`), `RegistrierteVerbindung` wird abgebaut, `hoermarkierung_v2_getrennt` nimmt den Standlock (`intervention.rs:300`). Ohne neuen Main-Link bleibt die Sperre: Entfernen einer zweiten v2-Instanz (Gen-/Legacy-Klassifikation), v2-Protokollabbruch oder v2-only-Reconnect einer Instanz. Bei Reload/Bindung/Neu-ID folgen v2- und v3-Reconnect (`State.cpp:218-221`, `:360-361`, `:64-65`); ob der Broker das alte v2-Ende vor dem ersten neutralen Heartbeat des neuen v3-Links verarbeitet, ist **offene Annahme** (verarbeitet er es danach, bleibt die Sperre ebenfalls stehen). Kontext: auch ein aktiver v2-Marker landet mit Produkt-ID auf dem Platzhalter (`intervention.rs:275`), wirkt also brokerweit.

**(2) Quellenstellen:** wie Tabelle; `intervention.rs:269-297` (`false` löscht nur den Eintrag, nie unknown).

**(3) Produkteinstieg:** erreichbar über jede Instanz mit v2-PipeClient (`PluginProcessor.cpp:135-152`, `:437`, `:475`) und die genannten Reconnectpfade.

**(4) Zusage.** Gewollte Sperre: `docs/beweise/SONDE-011.md:514` (C-08) „Overflow/Lücke/Disconnect setzen sticky unknown; End, v2-`false`, Sessionende, stale oder Eviction löschen es nicht. Nur bestätigter Neutral-/Sequenz-Resync entsperrt; Überlappung und Tail laufen vollständig aus.“ Registerannahme zur Lösung: `docs/offene-punkte.md:212` (NAK-188) „Ein Link ohne auffindbare Sitzung sperrt alle Sitzungen — heute ohne Produktlöser.“ … „Löst sich mit NAK-180, weil `neutral_resync` den Platzhalter mitnimmt (`intervention.rs:398`).“ Für automatische Freigabe oder Hinweis bei stehendem Main-Link: **keine Zusage** → LÜCKE.

**(5) Gegenpfad und stärkstes Gegenargument.** Stärkstes Gegenargument: Der v2-Legacyweg kennt keine Projektzeit und gilt fail-closed für „die ganze Sitzung“ (`intervention.rs:288-289`); ein Verbindungsende kann einen laufenden Marker abgeschnitten haben, sticky unknown ist dann richtig. Gegenprüfung: Die Sperre entsteht auch für Verbindungen ohne je gemeldeten Marker, trifft alle Sitzungen aller Projekte und hat ohne Main-Neuaufbau keinen Löser; genau die Lösungsannahme von NAK-188 greift für diesen Auslöser nicht. Befund ist der fehlende Wiederherstellungs- und Hinweisweg samt Reichweite, nicht die Sperre.

**(6) Vorhandene Tests.** `coordinator/mod.rs:592-633` und `coordinator_model.rs:1405-1408` rufen die v2-Marker mit **v3-Link-IDs** (`"link-a"`, `"a"`), die in `stand.links` stehen — sie modellieren eine Sitzungszuordnung, die der Produktweg (`server.rs:766`) nie liefert. Kein Test fährt v2-Ende mit Produkt-ID → Platzhalter → Evidenz einer fremden Sitzung → Freigabe.

**(7) Kleinster Folgebeweis.**
- *Ausgangslage:* Coordinator mit zwei Sitzungen S1 (Main M1, Probe P1) und S2 (Main M2, Probe P2); beide Mains mit neutralem ersten Heartbeat; Evidenz beider Probes wird angenommen.
- *Ereignisfolge:* `hoermarkierung_v2_getrennt("v2:32:<sensor>:<nonce>")` in Produktform, ohne vorheriges `hoermarkierung_v2(true)`; dann Evidenz von P1 und P2; weitere, nicht erste Heartbeats von M1 und M2. Integrationsvariante: `server_starten_mit_interventionssenke` mit echtem v2-Client, zweiter v2-Client trennt bei stehendem v3-Main.
- *Erwartbares Fehlverhalten:* `evidence_gesperrt` für P1 und P2; bleibt nach den Heartbeats bestehen.
- *Erfolgskriterium:* v2-Ende ohne aktiven Marker sperrt nicht, oder die Sperre bleibt auf die Sitzung der Instanz begrenzt und ein Produktweg bzw. Hinweis existiert.
- *Gegenfall:* v2-Ende während aktivem Marker bleibt fail-closed bis zum bestätigten Neutral-Resync.

**(8) Abgrenzung.** T3-14-03 (`BEFUNDE.md:590`): Sondenende setzt die Sitzung unknown, fehlender Löser bei stehendem Gen — gleiche Lückenklasse. Neu hier: Auslöser v2-Verbindungsende jeder v2-Instanz (auch ohne Marker) und brokerweite Wirkung über den Platzhalter (neuer Ablauf und neue Wirkung). NAK-188: gleiche Platzhalterwirkung aus anderem Auslöser; dessen Lösungsannahme trägt hier nicht. NAK-180: Neutralitätserklärung nur beim eigenen Main-Linkaufbau.

**Vorschlag:** v2-Verbindungen ohne aktiven Marker nicht als unzuordenbares unknown werten oder v2-Links ihrer Main-Sitzung zuordnen; gemeinsam mit T3-14-03/NAK-188 einen Wiederfreigabe- oder Hinweisweg bei stehendem Main-Link festlegen und mit Produkt-Link-IDs testen.

---

### P15-T-04 — Replay vor dem ersten Heartbeat: nach Brokerneustart oder Eviction wird ein offener Sources-Befehl endgültig `unauthorized` und verschwindet still

**DEFEKT · medium · K1 (Ordnung) + K2 · Quellenherleitung** (heutige Produktreichweite schmal; Eviction-Variante siehe OF-06)

| Schritt | Ort | Befund |
|---|---|---|
| Anforderung | `PluginEditor.cpp:179-192`; `Ipc.cpp:1200-1203`, `:1245-1287` | User klickt „Bind source“ oder „Remove source“ (verbundene Quelle). |
| Lokale Zustandsänderung | `Ipc.cpp:1289-1356` (`:1334-1343`) | Ausstehender Befehl vermerkt; die Mitgliedschaft wird erst nach positivem ACK übernommen (`:1073-1075`). |
| Übergabe | `Nachrichten.cpp:157-256`; `Verbindung.cpp:884-909` | In-Flight-Register plus P0-Queue, Wire-Write. |
| Annahme/Ablehnung | Broker stirbt vor dem ACK; der neue Broker restauriert Riegel, Passagen, Experimente und Evidenz, aber keinen Clientstand (`mod.rs:309-333`). Client: `Verbindung.cpp:722`, `:739` Replay **vor** dem ersten Heartbeat (`:851-875`); P0 ist FIFO (`IpcQueues.h:127-136`, `:160-168`). Broker: `befehl.rs:493-513` | Neuer Main-`ClientStand` mit `bestaetigt = false` (`link.rs:250`, nichts geerbt), keine Führung → `abgelehnt/unauthorized`. Persistente P0 identisch (`befehl.rs:264-286`), außer der Befehl war bereits committet (Idempotenz vor Autorisierung, `:185-215`). |
| Abschluss | `Nachrichten.cpp:297-400`; `Ipc.cpp:1066-1077` | In-Flight endgültig freigegeben; Gen löscht den ausstehenden Befehl ohne Anzeige. Die Main-Bestätigung entsteht erst danach über den Heartbeat (`liveness.rs:481`). |
| Wiederholung/Rücknahme | — | Keine automatische Wiederholung; kein lokaler Rückbau nötig (nichts vorweggenommen), aber der Handgriff ist still verloren. |

**(1) Auslöser und Ablauf.** Clientthread: `eineVerbindung` → `aufbauZug` (`Verbindung.cpp:722`) → Linkcallback (`:729`) → `inFlightNachReconnect` reiht die offenen persistenten P0 unter `sendeMutex` ein (`Nachrichten.cpp:258-282`) → erste Schleifenrunde reiht den Heartbeat ein (`Verbindung.cpp:851-875` über `heartbeatSchritt` → `sendeP0`, `:272`) → Entnahme P0 vor P1 in FIFO-Ordnung (`:884-902`). Broker: ein P0-Consumer je Link verarbeitet in Ankunftsreihenfolge (`verbindung.rs:583-614`). Eviction-Variante: Gen länger als `TOMBSTONE_MS` getrennt → `client_eviktieren_locked` entfernt Client und Führung (`liveness.rs:249-273`, `:180-209`) → gleiche Folge. Zusätzlich kann auch bei richtiger Reihenfolge die Zielprobe noch fehlen → `unknown_target` (`befehl.rs:549-566`; Behandlung wie T3-07-04).

**(2) Quellenstellen:** wie Tabelle.

**(3) Produkteinstieg:** Sources-Befehle erreichbar, Fenster eng: Brokerabsturz oder -kill im In-Flight-Fenster bzw. Gen-Ausfall über 10 s. Während einer Trennung ist der Knopf gesperrt (`SourcesModel.cpp:1637-1639`; `PluginEditor.cpp:916-918`), neue Befehle entstehen dann nicht. `user_verdict` und Versuchs-P0 (`Analyse.cpp:783`, `:1273`) laufen über denselben Mechanismus, sind aber **nicht verdrahtet** (keine Editoraufrufer).

**(4) Verletzte Zusage (wörtlich).**
- `docs/beweise/SONDE-011.md:560` (O-01): „Verbindungsverlust vor einem ACK reiht dieselbe `command_id` erneut ein.“
- `docs/beweise/SONDE-011.md:596` (K-01): „Broker-Kill trennt den echten C++-Client; er reiht dieselbe `command_id` wieder ein. Der erste erfolgreiche Commit erzeugt genau eine interne Event-UUID und antwortet `ergebnis = angewandt`.“ Ebenso K-02 (`:597`).
- `docs/beweise/SONDE-014.md:823` (M-73, Hervorhebung entfernt): „`user_verdict` als persistenzpflichtiger P0 über `sendePersistenzP0`, mit Wiederholung unter derselben `command_id` über einen Brokerkill hinweg (A4-SI-Muster)“.
Die Wiederholung findet statt, wird aber durch die eigene Sendereihenfolge endgültig abgewiesen; die zugesagte Wirkung bleibt aus.

**(5) Gegenpfad und stärkstes Gegenargument.** Stärkstes Gegenargument: O-01 lässt `abgelehnt` ausdrücklich als endgültige Antwort zu, und nach Brokerneustart startet Führung frei (NAK-120, `offene-punkte.md:113`: „Bis dieser Vertrags- und Bauweg existiert, startet Führung nach Brokerneustart frei“). Gegenprüfung: Die Ablehnung stammt nicht aus dem Befehl, sondern aus der clientseitigen Reihenfolge — derselbe Befehl wäre Sekundenbruchteile später nach dem autorisierenden Heartbeat zulässig. K-03 (bereits committet) bleibt korrekt, weil die Idempotenz vor der Autorisierung steht (`befehl.rs:185-215`). Reconnect ohne Neustart unter 10 s erbt `bestaetigt` (`link.rs:250`) und ist nicht betroffen.

**(6) Vorhandene Tests.**
- `store_crash_matrix.rs:257-295` (`echte_cpp_client_crashrunde`): echter C++-Client gegen `phase_b_server_starten`; der Worker nutzt `PhaseBSenke` (`eqcop-store-crash-worker.rs:81-133`) ohne Coordinator-Autorisierung.
- `store_crash_matrix.rs:603-619`: Produkt-Coordinator, aber Heartbeat (`si_report`) vor den Befehlen und ohne Neustart.
- `IpcTestMain.cpp:5662-5803`: Fake-TestServer ohne Autorisierung.
Keiner kombiniert echten Coordinator, Neustart und reale Clientreihenfolge.

**(7) Kleinster Folgebeweis.**
- *Ausgangslage:* echter v3-Server mit `Coordinator::mit_store` auf Probe-Pipe; echter C++-ControlClient als Main mit Statusprovider; Probe verbunden und bestätigt.
- *Ereignisfolge:* Main sendet `session_command confirm_join` (bzw. einen persistenten P0); Server vor dem ACK hart beenden; Server mit demselben Store neu starten; Client verbindet neu (Replay und erster Heartbeat); Probe verbindet danach.
- *Erwartbares Fehlverhalten:* ACK `abgelehnt/unauthorized` (bzw. `unknown_target`), In-Flight leer, keine Wirkung.
- *Erfolgskriterium:* nach Wiederaufbau `angewandt` bzw. `idempotent_wiederholt`, oder der Befehl bleibt wiederholbar, bis Autorisierung und Ziel bestehen; negativer Ausgang in Gen sichtbar.
- *Gegenfall:* Reconnect ohne Neustart unter 10 s → `angewandt`; ein fremder, nicht führender Main bleibt `unauthorized`.

**(8) Abgrenzung.** T3-07-04 (`BEFUNDE.md:439`, `:536`): `unknown_target` bei abwesender Quelle und still verworfener negativer ACK — die Behandlungsseite ist gemeinsam und wird nicht neu gezählt; neu ist `unauthorized` aus der Replay-vor-Heartbeat-Reihenfolge nach Neustart/Eviction, das auch persistente P0 (K-01/K-02/M-73) trifft. NAK-120 (Führungsrekonstruktion), NAK-125 (flüchtige `session_commands`), NAK-164 (Append-Fehler ohne Antwort), T3-03-03 (ACK-Zahlengrenze): kein Duplikat.

**Vorschlag:** Den Aufbau-Heartbeat vor Replay und wartende persistente P0 stellen (oder brokerseitig `unauthorized` vor dem ersten Heartbeat eines Links als wiederholbar statt endgültig beantworten); negativen Sources-ACK sichtbar machen (mit T3-07-04); A4-SI mit echtem Coordinator statt `PhaseBSenke`.

---

### P15-T-05 — Duplizierte Probeeq: gegenseitige Verdrängung, danach dauerhafte Sperre beider Instanzen ohne produktiven Auflösungsweg; Gen bietet einen Handgriff an, der positiv quittiert und nichts löst

**DEFEKT · medium · K2 + K8 · Quellenherleitung** (Zeitpunkt des Konflikts im Verdrängungswechsel, FL-Host-Preset als Auslöser und Langzeitdeckel: offene Annahmen)

| Schritt | Ort | Befund |
|---|---|---|
| Anforderung | `nakama-state-v2.md:269`; `SondeProcessor.cpp:138`, `:1039`, `:1087-1088` | FL-Klon bzw. Kopie einer Probeeq samt State: beide Instanzen tragen dieselbe `instance_id` und je eine eigene `runtime_nonce`. |
| Lokale Zustandsänderung | `SondeProcessor.h:158-159` (`createEditor` nullptr); keine Neu-ID-Funktion in `SondeProcessor` | Keine; die Probeeq hat keinen Weg zu einer neuen Kennung. |
| Übergabe | `link.rs:89-99`, `:131-192`; `zustand.rs:10-25` | Gleicher `ClientKey` (ohne Nonce). Jedes Hello verdrängt den anderen Link und löscht Messframes, Messfehler, Lautheit und Evidenz des Schlüssels (`link.rs:137-142`). Der verdrängte Client verbindet neu (`Verbindung.cpp:440-493`) und verdrängt zurück. |
| Annahme/Ablehnung | `liveness.rs:337-399`; `instance_alias.rs:181-196`; `zustand.rs:481-520`; `flush.rs:235-244` | Erster Report eines verdrängten Links → Kollision, beide Besitzer quarantänisiert, Riegel für die effektive Adresse (`mod.rs:457-462`, inkl. `session_epoch` und `instance_id`) persistiert, `bestaetigt = false`. Quarantänisierte Besitzer werden weiter angenommen, aber nicht geroutet (`instance_alias.rs:156-158`; `link.rs:223-236`) und verdrängen sich weiter. |
| Abschluss | `flush.rs:289-298`; `subscription.rs:93-103`; `mitgliedschaft.rs:17-19`, `:238-242`; `senke.rs:149-155`; `sicht.rs:535-586` | Dispatch, Push, Auto-Join und P2 für beide gesperrt. Der Snapshot trägt kein Konfliktfeld; der Riegel setzt nur `beitritt_bestaetigung_noetig`. Gen zeigt beim führenden Main „Join confirmation required - choose a source and bind it“ (`SourcesModel.cpp:1356-1360`; `PluginEditor.cpp:1208-1211`). |
| Wiederholung/Rücknahme | `flush.rs:247-279` (Aufrufer nur Tests); `befehl.rs:567-574`, `:614-615`; `mitgliedschaft.rs:119-133`; `befehl.rs:761-764`; `State.cpp:54-67` | Keine produktive Auflösung. „Bind“ wird `angewandt` (sofern die Zieladresse aktuell ist), setzt `bestaetigt`, ändert den Riegel nicht; die Diagnose bleibt. Das v3-Duplikatflag hat keinen Plugin-Leser. `neueSensorId` existiert nur für Gen. Entfernen des Klons löst die Originalinstanz für diese Gen-Sitzung nicht (Riegel je Epoche; Alias-Quarantäne bleibt bis Brokerneustart, `instance_alias.rs:220-243`). Beim nächsten Projektstart mit beiden Instanzen: neue Epoche, erneute Kollision, neue Riegelzeilen. |

**(1) Auslöser und Ablauf.** Pro Link ein Consumer (`verbindung.rs:583-635`); Hello unter Standlock (`link.rs:90`) und Alias-Registerlock (`instance_alias.rs:153`); Riegel-Persistenz außerhalb des Standlocks (`link.rs:321-331`; `liveness.rs:510`); Tick schließt die verdrängten Links (`liveness.rs:264-269`). Der **Verdrängungswechsel** ist Quellenherleitung (keine Noncesperre, keine Wartefrist; Suche `tombstone|verdraengt` ohne Gegenmechanismus). Der **Konflikt** braucht einen Report des verdrängten Links zwischen Hello-Verarbeitung und Linkschluss; pro Wechsel ein kleines Fenster, über den fortlaufenden Wechsel wiederholt — Eintrittszeitpunkt **offene Annahme**. Bereits ohne Konflikt verlieren beide Messpunkte bei jedem Wechsel ihre Messwahrheit. `doppelteId` in Gen (`SourcesModel.cpp:1176`, `:1230-1231`) greift nie, weil Mitglieder je `ClientKey` ohne Nonce gebildet werden (`sicht.rs:521-525`).

**(2) Quellenstellen:** wie Tabelle; zusätzlich `instance_alias.rs:96-103`, `:144-243`; `zustand.rs:529-579`; `SourcesModel.cpp:1935-1945`.

**(3) Produkteinstieg:** erreichbar durch Klonen oder Kopieren einer Probeeq in FL (Duplikat laut `nakama-state-v2.md:269` gewollt möglich). FL-eigene Host-Presets als weiterer Auslöser: offene Annahme (OF-08).

**(4) Verletzte Zusage (wörtlich).**
- `docs/FL-Nakama-Sonden-Design-Entwurf.md:1841`: „`instance_id` | im Plugin-State persistent | stabiler Messpunkt; Duplikate werden sichtbar aufgelöst“; `:1857-1859`: „Bei Bridge, zwei offenen Projekten oder duplizierten IDs ist eine kurze sichtbare Bestätigung Pflicht.“
- `eq-copilot/schemas/state/nakama-state-v2.md:269`: „Der Broker meldet die zweite lebende Verbindung (`heartbeat_ack{konflikt}`), der User löst sichtbar auf (`neueSensorId` ⇒ neue ID, Label/Paar bleiben, Host-Dirty).“
- `docs/beweise/SONDE-011.md:516` (C-10): „Meldet die verdrängte Nonce danach erneut, entsteht der fail-closed-Duplikatkonflikt: beide Seiten sind bis zur expliziten Auflösung nach C-03/C-07 gesperrt.“; `:513` (C-07): „Auflösung ausschließlich über explizite Neu-ID im Phase-A-Weg.“
- Für Gens Anzeige: `CLAUDE.md:51-52` „jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich einen Zustand.“

**(5) Gegenpfad und stärkstes Gegenargument.** Stärkstes Gegenargument: Probeeqs lokale Status- und Rückfallfläche ist geparkt (S31b; `offene-punkte.md:431`) — die sichtbare Auflösung könnte dort entstehen; Sperre und sofortige Verdrängung sind gewollt (C-07, C-10). Gegenprüfung: Es fehlt nicht nur die Fläche, sondern jede technische Auflösungsoperation für Probeeq (kein Neu-ID-Weg im Prozessor, kein Befehl über Gen oder Broker, `konflikt_guard_aufloesen` ohne Produktaufrufer, obwohl der Kommentar `flush.rs:262-263` genau diese Methode als „explizite Neu-ID“ führt; `NAK-121.md:228` und `:1499` bauen sie so). Gens bestehende Fläche bildet den Konflikt auf eine Beitrittsbestätigung ab und quittiert „Bind“ positiv ohne Wirkung. Der fortlaufende Wechsel zweier lebender Nonces ist von C-10 (einmalige Verdrängung, danach Report) nicht beschrieben. Gegenpfad Gen geprüft: Gen-Duplikate kollidieren in v3 nicht (eigene Epoche je Prozessor, `PluginProcessor.cpp:134`) und haben den v2-Konfliktweg (`PluginEditor.cpp:580-595`; `State.cpp:54-67`).

**(6) Vorhandene Tests.** `coordinator_model.rs:901-925` (eine Verdrängung; verdrängter Report sperrt beide; kein Reconnect des Verdrängten, keine Auflösung); `store_crash_matrix.rs:2966-2988` (Cleanup vor altem Report); `StateMigrationTestMain.cpp:2243-2248` prüft nur Gens `neueSensorId`; NAK-138 (`offene-punkte.md:170`) benennt den fehlenden Ende-zu-Ende-Test. Kein Test für Gens Diagnose oder „Bind“ bei gesetztem Riegel, keiner für eine Probeeq-Auflösung.

**(7) Kleinster Folgebeweis.**
- *Ausgangslage:* Coordinator mit Store und PushProbe; Main M abonniert; Probe X mit Nonce a verbunden und bestätigt.
- *Ereignisfolge:* Hello X/b → Report von Link a (verdrängt) → Hello X/a (Reconnect) → Hello X/b; Snapshot lesen; M sendet `confirm_join` für X/b; Link b endgültig schließen; Link a meldet Heartbeats; Tick über `TOMBSTONE_MS`.
- *Erwartbares Fehlverhalten:* `zu_schliessende_links` wechselt bei jedem Hello; `konfliktriegel_gesetzt(effektiv)` true; Snapshot `beitritt_bestaetigung_noetig: true` ohne Konfliktangabe; `confirm_join` → `angewandt`, danach `dispatch_fuer_link_erlaubt(a)` false und Snapshot unverändert; nach Wegfall von b bleibt a gesperrt.
- *Erfolgskriterium:* ein Produktweg beendet den Konflikt (neue Kennung einer Instanz oder ein Auflösungsbefehl, der Riegel und Quarantäne gemeinsam räumt); Snapshot und Gen führen den Konflikt als solchen; kein erfolgreicher ACK ohne Wirkung; zwei lebende Nonces verdrängen sich nicht endlos.
- *Gegenfall:* Reload derselben Probeeq (alter Link tot, neue Nonce) erzeugt keinen Konflikt; Gen-Duplikat über `neueSensorId` funktioniert weiter.

**(8) Abgrenzung.** NAK-138: nur Testlücke der Duplikatkette. NAK-126 (`offene-punkte.md:163`): verdrängte Control-Pipe bleibt eingabefähig — anderer Effekt. NAK-121/H-14 und G3 `RESEXHAUST-004`: Deckel und Auflösungs-API gebaut, Produktaufrufer fehlt weiterhin; nicht registriert. T3-12-02: Sicherheitsfrage der Draht-Identität, nicht geprüft, keine Überschneidung behauptet. Geparkte Probeeq-Fläche (S31b): betrifft nur die sichtbare Form.

**Vorschlag:** Einen technischen Auflösungsweg für Probeeq-Duplikate bauen (Neu-ID-Operation bzw. produktiver Aufruf von `konflikt_guard_aufloesen`), das v3-Duplikatflag auswerten, den Konflikt im Snapshot und in Gens Diagnose als solchen führen statt „choose a source and bind it“, und den Verdrängungswechsel zweier lebender Nonces begrenzen. Die sichtbare Form auf der Probeeq ist geparkte Produkt-/Designfrage.

---

## 3. Offene Prüffelder

| ID | Feld | Gelesene Stellen | Kleinster Folgebeweis |
|---|---|---|---|
| OF-01 | Beendet das Ende einer alten Telemetrieverbindung die neue Kopplung desselben Control-Links, sodass P2 still verworfen wird? | `senke.rs:60-78` (Eintrag je Control-Link-ID), `:143-145` (stilles Return); `TelemetryClient.cpp:755-765`; `trennung.rs:149-174`; `bootstrap.rs:317-429` (nicht abschließend gegen diese Folge gelesen) | server_v3-Test: Control verbunden, Telemetrie T1 koppelt, T2 koppelt denselben Control-Link, T1 endet danach; `telemetrie_kopplungen()` (`sicht.rs:237-240`) und `p2_live_frames` für Frames über T2 prüfen. |
| OF-02 | Dauerhaft scheiternde Telemetriekopplung bei verbundenem Control ohne Gen-Diagnose | `TelemetryClient.cpp:389-464`; `SourcesModel.h:39-48` (kein Telemetriezustand) | Control verbunden und abonniert, Telemetrie-Hello wiederholt abgewiesen; Gen-Diagnose und Quellenzeilen über 10 s beobachten. |
| OF-03 | Geschriebene, aber beim Broker-Linkschluss unverarbeitet verworfene P1-Ereignisse (`evidence_snapshot`, `intent_update`) ohne Wiederholung | `roh/phase-04-lebenslauf.md:167` (Close-Flag vor Entnahme); `verbindung.rs:810-889`; `IpcQueues.h:342-528` (Wiederholpuffer nur für nicht Geschriebenes) | Link mit gefüllter Eingangsqueue auf `trennen` setzen; Clientzähler gegen `evidence_angenommen` bzw. Intentspiegel vergleichen. |
| OF-04 | Intent-Vollbericht mit kleinerer Revision nach State-Reload derselben Sitzung wird als `AeltereRevision` verworfen; der Broker verwirft P1 still, Gen erfährt nichts | `intent.rs:486-518`; `senke.rs:114-116`; `Ipc.cpp:809-825`, `:1059`; `State.cpp:203-221` | Main mit gespiegeltem Intent (Revision 5), Reload mit Revision 3 bei gleicher Bindung, Reconnect unter 10 s; Spiegel gegen lokalen Bestand. Erreichbarkeit hängt an Intents im State (kein UI-Editor). |
| OF-05 | Zwei Gen-Main-Instanzen in einem Projekt: Probes ohne eindeutige Main-Sitzung, welche Diagnose, und löst das Entfernen einer Main den Zustand (inkl. T-03)? | `link.rs:89-98`; `mitgliedschaft.rs:231-268` | Modelltest: zwei Mains gleicher PID, eine Probe; danach eine Main trennen; Snapshot, Beitrittsbedarf und Evidenzsperre prüfen. |
| OF-06 | T-04 über Eviction statt Brokerneustart | `liveness.rs:180-209`, `:249-273`; `Verbindung.cpp:440-493` | ManualClock: Main-Link getrennt, Uhr +10 s, Tick, Neu-Hello mit in-flight `confirm_join` vor dem ersten Heartbeat. |
| OF-07 | Langzeitdeckel aus T-05: Riegelzeilen werden produktiv nie gelöscht, Quarantäne ist global und wird bei jedem Hello geprüft | `store/mod.rs:62`; `writer.rs:752-788`; `zustand.rs:505-507`; `instance_alias.rs:103`, `:177-179`; `link.rs:126-129` | Modelltest: 511 Kollisionen mit verschiedenen Nonces, danach ein gewöhnliches Hello eines unbeteiligten Clients (Abweisung `alias_quarantaene_deckel`?); Zeilenzuwachs je Projektstart mit dupliziertem Probeeq zählen. |
| OF-08 | Transportiert FLs eigener Presetweg den vollständigen Probeeq-State inklusive `instance_id` in eine zweite Instanz? | `SondeProcessor.cpp:1039`, `:1087`; `nakama-preset-v1.json:29` (schließt `instance_id` nur für das Nakama-Preset aus) | Laufzeitarm-Szenario (außerhalb dieses Audits): Host-Preset speichern, auf zweitem Track laden, v3-Hello-Adressen vergleichen. |

---

## 4. Verworfene Verdachte

| ID | Verdacht | Gegenbeleg |
|---|---|---|
| V-01 | Versuch Begin/Urteil/Abbruch mit ausfallender Sonde lässt Teilzustand | Nicht produktiv erreichbar: `Analyse.cpp:790`, `:965`, `:1023`, `:1291-1302` ohne Editoraufrufer (Suche: nur Tests `Sonde013PassageStateTest.cpp`, `Sonde014AssistentTest.cpp`). Lokales Urteil vor Sendung (`Analyse.cpp:986`) ist als phase-14-Notiz „Urteil nach endgültig abgelehnter Sendung“ bekannt. |
| V-02 | Assistentenantwort/-abbruch mit halbem Zustand | Nicht verdrahtet: `Analyse.cpp:1201-1277`; keine `processor.assistent…`-Aufrufe in `src/`. |
| V-03 | Hörbare v3-Intervention, Linkwechsel vor End/ACK | Gen-UI erreicht den v3-Interventionsweg nicht (T3-07-02, `BEFUNDE.md:437`); Aufbauzug stellt Replay-Begin voran und verwirft alte Berichte (`IpcQueues.h:138-155`, `:181-210`); Sequenzlücke → sticky unknown gewollt (C-08, `SONDE-011.md:514`). |
| V-04 | P0 endgültig abgelehnt, nachdem Gen lokal vorweggenommen hat | Keine Vorwegnahme: Join/Unbind erst nach positivem ACK (`Ipc.cpp:1066-1077`); Label rein lokal ohne Brokerbefehl (`Ipc.cpp:1205-1243`); lokaler Unbind nie verbundener Quellen ohne Broker (`:1257-1285`). |
| V-05 | Storefehler → Trennung → Wiederholung derselben `command_id` erzeugt Doppelwirkung | Idempotenz vor Vorbedingung und Autorisierung (`befehl.rs:185-215`); Append-Fehler ohne Antwort ist NAK-164. |
| V-06 | ACK `konflikt`/`abgelaufen` ohne lokalen Rückbau | Konflikt-Haken mit höchstens drei Neuausgaben (`Nachrichten.cpp:332-361`); Gen-Sources übernehmen nur Erfolg (`Ipc.cpp:1073`); TTL-Ablauf an der Sonde nicht verdrahtet (Entwurf §33.3, `:2093-2098`). |
| V-07 | Lokaler Subscribe-Fehlerweg (`Ipc.cpp:1038-1048`) läuft in eine Reconnectschleife | `subscribe_session` ist schlüsselkoaleszierender P1 und für Gen nicht abweisbar (`IpcQueues.h:342-528`); Broker-Abweisungen verlangen fremde Adresse oder Quarantäne, Hello und Subscribe entstehen aus derselben Funktion (`Ipc.cpp:530-539` gegen `:1031-1037`); ein State-Wechsel dazwischen löst selbst `controlV3.reconnect()` aus (`State.cpp:221`, `:361`). |
| V-08 | Intent-Vollbestand geht bei P1-Fehler verloren | `sendeIntentVollbestand` liefert nur bei Koaleszierung `false` (`Ipc.cpp:809-825`); P1-Abweisung für Gen unerreichbar (wie V-07). |
| V-09 | BrokerLifecycle-Spawn endgültig verweigert ohne Erholung | Abklingzeit und erneuter Versuch (`BrokerLifecycle.cpp:883-898`, `:919-929`; `BrokerLifecycle.h:22-28`); ehrliche Diagnose `brokerUnavailable` mit Reconnect (`SourcesModel.cpp:597-608`; `PluginEditor.cpp:938-943`); veralteter Pin ist T3-05-03. |
| V-10 | Sondenende während Gen-Aufbau oder Nachbericht | Nachbericht gehört dem Main-Link (`link.rs:431-472`, `:480-494`); die Sitzungssperre durch Sondenende ist T3-14-03. |
| V-11 | DSP-Commit halb publiziert | Publikation nach Commit ist `noexcept` (`NakamaTransaktion.cpp:639-708`, `:697`); Busy-Wiederholung vor dem Commit (`:591-593`). |
| V-12 | Gestenabschluss mit halber Anwendung | Probeeq ohne Editor (`SondeProcessor.h:158-159`), Wrapper reicht keine Hostgeste (`SondeProcessor.cpp:1308-1310`); Abweisung setzt Hostparameter zurück (`:1347-1357`). |
| V-13 | `session_command` quittiert trotz Append-Fehler | Bekannt NAK-125 (`befehl.rs:614-615`). |
| V-14 | Duplikat eines Gen blockiert v3 dauerhaft | Eigene Epoche je Prozessor (`PluginProcessor.cpp:134`), Epoche Teil des `ClientKey` (`zustand.rs:10-25`) → keine v3-Kollision; v2-Konfliktpanel mit `neueSensorId` (`PluginEditor.cpp:580-595`; `State.cpp:54-67`). |
| V-15 | Gewollte Sperren (nicht als Fehler gewertet) | Verursachertrennung bei Storeverweigerung (S-03; `store_crash_matrix.rs:1558-1576`); fail-closed Routing bei Lesefehler (`SONDE-011.md:1061`); sticky unknown (C-08); Kollisionssperre beider Seiten (C-07/C-10); Main-Hello sperrt Sitzung bis Neutralität (`link.rs:269-287`); Guard-Persistfehler fail-closed (`link.rs:323-331`). Kandidaten T-01 bis T-03 und T-05 betreffen jeweils Erholung, Reichweite oder Anzeige, nicht diese Sperren. |

---

## 5. Abgrenzungstabelle

| Kandidat/Feld | Bekannter Punkt | Verhältnis |
|---|---|---|
| T-01 | NAK-125 (`offene-punkte.md:162`) | Gleicher Append-Fehlerpfad beim `session_command`, dort falsche Quittung; hier Dauertrennung aller Clients und P2-Sperre. |
| T-01 | NAK-164 (`:191`) | Persistenter P0 ohne Antwort bei Append-Fehler; nicht neu gezählt. |
| T-01 | NAK-274 E5-2 (`:284`) | Ordnungsrand am Degradationsübergang; andere Wirkung. |
| T-01 | T3-08-01 (`BEFUNDE.md:451`) | Group-Commit-Teilung; kein Bezug zur Degradationsschleife. |
| T-02 | `SONDE-011.md:1061` | Sperre gewollt; Kandidat betrifft Wiederanlauf, Reichweite, Anzeige. |
| T-02 | NAK-165 (`:192`) | Outbox-Nachspielen nach gescheitertem Push; anderer Pfad. |
| T-02 | T-01 | Gesunder Writer, Leseweg; stabil tot statt Schleife. |
| T-03 | T3-14-03 (`BEFUNDE.md:590`) | Gleiche Lückenklasse (Löser bei stehendem Gen); neu: v2-Auslöser ohne Marker, brokerweite Wirkung. |
| T-03 | NAK-188 (`:212`) | Gleiche Platzhalterwirkung aus anderem Auslöser; deren Lösungsannahme über NAK-180 greift hier nicht. |
| T-03 | NAK-180 | Neutralität nur beim eigenen Main-Linkaufbau (`Ipc.cpp:956-975`). |
| T-04 | T3-07-04 (`BEFUNDE.md:439`, `:536`) | Stiller negativer ACK und `unknown_target` gemeinsam, nicht neu gezählt; neu: `unauthorized` aus Replay vor Heartbeat nach Neustart/Eviction. |
| T-04 | NAK-120 (`:113`), NAK-125 | Führungsrekonstruktion bzw. flüchtige `session_commands`; Kandidat trifft auch persistente P0 (K-01/K-02/M-73). |
| T-04 | T3-03-03 | ACK-Zahlengrenze; kein Bezug. |
| T-05 | NAK-138 (`:170`) | Nur Testlücke der Duplikatkette; kein Auflösungsweg benannt. |
| T-05 | NAK-126 (`:163`) | Verdrängte Pipe bleibt eingabefähig; anderer Effekt. |
| T-05 | NAK-121 H-14, G3 `RESEXHAUST-004` | Deckel und Auflösungs-API gebaut; Produktaufrufer fehlt, nicht registriert. |
| T-05 | T3-12-02 | Sicherheitsfrage der Draht-Identität; nicht geprüft, keine Überschneidung behauptet. |
| T-05 | `offene-punkte.md:431` (S31b, geparkt) | Sichtbare Probeeq-Fläche geparkt; Kandidat betrifft fehlende technische Auflösung und Gens bestehende Anzeige. |
| OF-01, OF-02 | T3-12-* (Sicherheit) | Nur Kopplungslebenslauf im Normalbetrieb, keine Authentisierungsfrage. |
| OF-03 | `roh/phase-04-lebenslauf.md:167` | Phase 04 beschreibt die Queuepolitik; offene Frage ist der Verlust bereits geschriebener P1 ohne Wiederholung. |
| OF-04 | T3-02-03 | Dort Revisionen über 2^53; hier kleinere Revision nach Reload. |
| OF-07 | NAK-121 H-14 | Deckel existiert; offen ist die Häufigkeit im Normalbetrieb ohne Löschweg. |
| V-01, V-02 | phase-14-Notiz „Urteil nach endgültig abgelehnter Sendung“ | Bekannt und nicht verdrahtet. |
| V-03 | T3-07-02 | Bekannt. |
| V-09 | T3-05-03 | Bekannt. |
| V-10 | T3-14-03 | Bekannt. |
| V-13 | NAK-125 | Bekannt. |
