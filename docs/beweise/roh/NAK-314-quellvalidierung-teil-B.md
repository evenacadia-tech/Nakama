# NAK-314 — Quellvalidierung Teil B (lesender Opus-Agent, HEAD 4ee4c407)

**Ticket:** NAK-314 · Planschritt S25m „Tiefenaudit 3 abarbeiten, Teil 6: Broker" · Etappe 0
„Quellvalidierung", **Teil B** (Storedegradation, Routing-Diagnose, Telemetriekopplung)
**HEAD:** `4ee4c40752845d98fb451f550ddf8743079153be` (Zweig `master`)
**Datum:** 26.09.2026 · **Modell:** Opus 5.5 (lesend)
**IDs (3):** T3-15-12 und T3-15-13 (W15); T3-16-02 (W16). Dazu die drei Regelfragen dieses Teils:
(1) Livestand ohne Commit, (2) ehrliche Diagnose bei fail-closed Routing mit gesundem Store,
(3) Telemetriegeneration.
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (Phasenzeilen `:643`,
`:644`, `:721`; Herabstufung P15-T-02 `:658`; Präzisierung T3-15-12 `:732`; Abschlusszeile F13
`:760`; Arbeitsübersicht W15 `:818`, W16 `:819`). Im Abschnitt „Geordnete Abarbeitung ab
18.09.2026" (`:108-161`) steht **keine** Zeile zu diesen drei IDs (gesucht nach `T3-15-12`,
`T3-15-13`, `T3-16-02`, `W15`, `W16`; die Bündelliste B01–B36 stammt aus Phase 11, die IDs aus
Phase 15/16). Rohberichte unter `docs/audits/2026-09-15-tiefenaudit/roh/`:
`phase-15-paket3-teilfehler.md:108-192`, `phase-15-gegenpruefung-paket3.md:19-179`,
`phase-15-zentral.md:36-37`, `:57-58`, `phase-16-b-verbindungen.md:14-41`, `:61-63`, `:70`,
`phase-16-f-gegenpruefung.md:10`, `:22`, `phase-16-abdeckung.md:21`, `:34`,
`phase-16-nachweise/b/wire-run.txt:67-68`, dazu `components-run.txt:5-8`, `wire.rs:108-140`,
`components.rs:76-110` (Harnisch, nur gelesen).
**Gate:** Schritt S25m `docs/plan/plan.json:382-388` (Feld `text` in `:385`); Registerzeilen
`docs/offene-punkte.md:317` (NAK-314) und `:593` (NAK-378).
**Muster:** `docs/beweise/roh/NAK-313-quellvalidierung-teil-A.md` (Form, Tiefe, Kopf) und
`docs/beweise/roh/NAK-313-quellvalidierung-teil-C.md` (Trennung Technik und Produkt).

**Gate-Text wörtlich** (`docs/plan/plan.json:385`, Feld `text`):

> „ANGELEGT 18.09.2026 (Register NAK-314; Arbeitsübersicht Phase 16 W13, W14, W15, W16, W17,
> W25, W28). INHALT: 10 Befunde im Broker — T3-08-01 (Domain-Atomarität am 64er-Batchrand;
> NAK-157), T3-08-02 und T3-08-03 (Rücknahme vor Erstpersistenz nach Restore wieder gültig,
> Commit- und Publikationsordnung; NAK-269), T3-15-12 und T3-15-13 (degradierter Store trennt
> Clients im Sekundentakt, transienter Lesefehler lässt Routing fail-closed ohne Anzeige),
> T3-16-02 (verspätetes Ende einer alten Telemetrieverbindung entkoppelt die angenommene
> Nachfolgerin), T3-15-15 und T3-14-03 (Klonkonflikt ohne Löser mit dauerhaften Sperren, Unknown
> nach reinem Sonde-Abbruch ohne Rückweg), T3-16-03 (erfolgreich geschriebene P1-Evidenz geht
> beim gewöhnlichen Linkende verloren), T3-08-05 (vollständiger Loop-Wrap invalidiert gültige
> Evidenz entgegen M-53). Produktentscheide als Karten: Klon-Identität und Sperrenende,
> Empfangs-ACK oder gezählter Verlust. GRENZE: ACK erst nach vollständigem Commit, Replay
> idempotent, keine Callbacks auf abgemeldeter Sitzung, Konfliktrestore fail-closed, keine
> Sicherheitsneubewertung (S25i). BEWEIS: docs/beweise/NAK-314.md mit Matrix, Rotbeweisen in A4,
> A4-SI (store_crash_matrix), A22, B8, vollem Kanon GRÜN und Laufzeit-Arm."

**Ruhegrund NAK-310** (`docs/offene-punkte.md:593`, NAK-378): „Vier offene Matrixdefekte der
Etappe 1 von NAK-310 … die Codeetappen beginnen erst nach ihrer Schließung in einer
Matrixnacharbeit 4 und einer PASS-Matrixprüfung 5. … Bis dahin ruht NAK-310". Berührungen mit
NAK-310 werden unten nur genannt, nicht bewertet.

**Prüfform:** ausschließlich Quellenlektüre am HEAD. Kein Bau, kein cargo, kein Testlauf, kein
Kanon, kein Skript, kein FL Studio, kein Codex, keine Fehlerinjektion. Git nur lesend (`status`,
`rev-parse`, `log`, `diff`, `blame`). Jede Zeilennummer unten ist am HEAD `4ee4c407` selbst
nachgelesen; wo eine Auditzeile gewandert ist, steht die neue Zahl. Sicherheitsbefunde
(T3-12-\*) sind nicht Gegenstand; Legacy-Bezeichner sind kein Befund; der Designteil ist geparkt
— unten steht kein Oberflächenvorschlag, nur ein Diagnosezustand und sein Text.

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn* (26.09.2026, 15:22:06 +0200):
die fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` sowie die drei
Auftragsdateien `docs/beweise/roh/NAK-314-quellvalidierung-teil-A-auftrag.txt`,
`…-teil-B-auftrag.txt`, `…-teil-C-auftrag.txt`. *Unmittelbar vor dem Schreiben* (15:40:09
+0200): dasselbe und zusätzlich die untracked Datei `docs/beweise/NAK-314.md` (nicht von mir;
das Manifest des Dirigenten, nicht angefasst). HEAD beide Male `4ee4c407`; er ist **nicht**
gewandert, ein Zeilenabgleich war nicht nötig. Diese Datei ist meine einzige Schreibaktion;
`…-teil-A.md` und `…-teil-C.md` gehören den Parallelvalidierern.

**Stand des Audits gegen den HEAD.** Phase 15 prüfte `e008811e` (17.09.), Phase 16 `aff2d818`
(18.09.). `git log aff2d818..HEAD -- broker/` zeigt 21 Commits (NAK-309 Etappe 4, NAK-313
Etappen 4–7, NAK-380 Etappen 2–6). Für die drei IDs entscheidend:

- `broker/src/coordinator/flush.rs`, `coordinator/mod.rs`, `store/handle.rs`,
  `store/writer.rs`, `store/mod.rs`, `lebenslauf.rs`, `transport/server_v3/verbindung.rs`,
  `transport/server_v3/senke.rs`, `transport/server_v3/trennung.rs`, `coordinator/link.rs`,
  `coordinator/zustand.rs`: in `git diff --stat aff2d818 HEAD` **nicht** enthalten — am HEAD
  wörtlich wie im Audit.
- `coordinator/liveness.rs` (NAK-313 `28839892`, `8fb8d46f`): nur `state_report`-Wertvariante,
  Ganzzahlhelfer und `host_mixer_index` als Ganzzahl; Heartbeat-Dirty (`:505-506`) und
  Verursacherflush (`:535-537`) unverändert.
- `coordinator/senke.rs` (NAK-313): Hunks nur bei der P1-Weiche (`:92-132`) und im neuen
  Testmodul (`:275` ff.); `telemetrie_gekoppelt`/`telemetrie_getrennt` (`:58-78`) stammen laut
  `git blame` unverändert von `6005c1b0` (03.09.). Die Auditzeilen `:143`/`:149` sind um 9 Zeilen
  nach `:152`/`:158` gewandert.
- `coordinator/subscription.rs` (NAK-313 `28839892`): neue Funktion
  `subscribe_json_mit_minor_wert` (`:26-82`), dadurch +10 Zeilen; der Lesefehlerzweig (Audit
  `:143`) steht jetzt bei `:152-155`, inhaltsgleich.
- `transport/bootstrap.rs` (NAK-313 Etappen 4–6, Bootstrap-Tor): stark geändert; das
  Kopplungsregister `Kopplungen` (Audit `:382`) steht jetzt bei `:417-532`, die Belegtprüfung bei
  `:485-487`, inhaltsgleich.
- C++: `TelemetryClient.cpp` (NAK-313 `349b1b90`) nur im Welcome-Leser ab `:667`; die
  Reconnectschleife (Audit `:432`) steht unverändert bei `:389-459`. `SourcesModel.cpp`
  (NAK-312/313) ist gewachsen; die Auditzeilen `:549`/`:593` („Authenticating") stehen jetzt bei
  `:635-636` und `:682-685`, inhaltsgleich. `controlclient/Verbindung.cpp` (`349b1b90`) nur im
  Welcome-Leser; P0-vor-P1 (`:893-904`) unverändert.

**Kein Commit seit `aff2d818` hat einen der drei Befunde behoben oder verändert.**

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorievorschlag | Schwere | Rotbeweis fällt an | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-15-12 (W15) | BESTÄTIGT; dazu: P2 fällt nach dem ersten Heartbeat still statt mit `routing_nicht_freigegeben`; ein voller Storekanal trennt auch bei gesundem Store jeden Heartbeat-Absender; das A4-Bein `degradierter_store_verweigert_annahme_und_tick_trennt_verursacher` schreibt den Defekt als gewollt fest; der Heartbeat-ACK meldet im Startfall für jeden Link `duplicate_instance_id: true` | DEFEKT | high (Audit: high) | A4 `store_crash_matrix.rs` neben `:1558-1576` (fünf Runden, Start- und Laufzeitvariante); Zusagezeile der neue abgeleitete Zweig in `flush_session` `flush.rs:147-171` und das P2-Tor `senke.rs:158` | nein — Technikregel (Regelfrage 1) |
| T3-15-13 (W15) | **PRÄZISIERT**: die falsche Anzeige bricht `CLAUDE.md:44-45` und SONDE-012 L15 wörtlich (Kategorie DEFEKT statt HÄRTUNG); nach dem Latch liest der Resubscribe **nie wieder**; die Aussage der Gegenprüfung „der Dauerfall landet ehrlich als `storeDegraded`" trägt heute wegen T3-15-12 nicht; der Grund der Sperre wird verworfen (`_grund`) | DEFEKT | low (Audit: HÄRTUNG/low) | A4 neben `store_crash_matrix.rs:1156-1172` (Broker meldet den Zustand) und B13 `Sonde012SourcesModelTest.cpp` neben `:935-950` (Gen zeigt ihn); Zusagezeile das neue Diagnosesignal | nein — Technikregel (Regelfrage 2) |
| T3-16-02 (W16) | BESTÄTIGT; dazu: die Zusagenbasis ist ein Matrixwiderspruch (C-LS-04 „höchstens einmal je `link_id`" gegen B-TC-10 und den Transport, der Neukopplung ausdrücklich vorsieht); `telemetrie_einmalig_gekoppelt` schreibt die `link_id`-Semantik fest; dieselbe Wurzel lässt nach **jeder** Neukopplung die C-LS-06-Reihenfolge am späteren Control-Ende ungeprüft (hergeleitet) | DEFEKT | medium (Audit: medium) | A4 `server_v3`-Inlinetest neben `tests_kopplung.rs:189-221` mit echtem Coordinator; Zusagezeile der Besitzervergleich in `telemetrie_getrennt` `coordinator/senke.rs:72-78` | nein — Technikregel (Regelfrage 3) |

---

## 1. T3-15-12 — bei degradiertem Store trennt der Heartbeat-Flush seinen eigenen Absender (W15)

*Befundzeile: `BEFUNDE.md:643` · Präzisierung Phase 16: `:732` · Arbeitsübersicht W15: `:818` ·
Rohbelege: `roh/phase-15-paket3-teilfehler.md:108-154` (P15-T-01),
`roh/phase-15-gegenpruefung-paket3.md:19-118` (K1), `roh/phase-15-zentral.md:36`, `:57`,
`roh/phase-16-b-verbindungen.md:61-63`, `:70`, `roh/phase-16-f-gegenpruefung.md:22`,
`roh/phase-16-nachweise/b/components-run.txt:5-8`*

### a) Behauptung des Audits

Bei degradiertem Store wird der Append des abgeleiteten Session-Snapshots, den jeder Heartbeat
auslöst, verweigert und sein Absender getrennt; alle v3-Clients kreisen im Sekundentakt, in der
Startvariante bleibt `routing_bereit` dauerhaft falsch und P2 läuft nicht, und die ehrliche
Anzeige erreicht Gen nicht (`BEFUNDE.md:643`); in Phase 16 an echter SQLite reproduziert: je
fünf Runden zehn Trennungen, keine neuen P2/Pushs, gesund null Trennungen (`:732`).

### b) Quellkette am HEAD

**Station 1 — der Client sendet P0 vor P1, sofort und danach im 1-Hz-Takt.**
`eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:738` ruft `meldeLinkStatus (true);` vor
der Sendeschleife; darin reiht Gen den Subscribe als P1 ein,
`eq-copilot/plugin/src/prozessor/Ipc.cpp:1035-1041`:

```
    sourcesModel.beginneSubscription (h.adresse.projectBindingId,
                                      h.adresse.sessionEpoch,
                                      h.adresse.instanceId);
    const auto subscribe = v3SubscribeJson();
    const auto ergebnis = subscribe.empty()
        ? nakama::ipc::P1Ergebnis::abgewiesen
        : controlV3.sendeP1 ("subscribe_session", subscribe);
```

Danach `Verbindung.cpp:766` `auto naechsterHeartbeat = std::chrono::steady_clock::now();` — der
erste Heartbeat entsteht in der ersten Runde (`:860-883`, Takt `kHeartbeatTaktMs = 1000`,
`ControlClient.h:42`) — und `:893-904`:

```
        // 1) Steuerung zuerst, immer. P1 kommt erst dran, wenn P0 leer ist —
        //    das ist die Client-Haelfte von "kein P0 wartet hinter Daten".
        …
            istP0 = p0.entnehmen (p0Eintrag);
            if (istP0)
                nachricht = p0Eintrag.json;
            etwasGesendet = istP0 || p1.entnehmen (schluessel, nachricht);
```

Der Heartbeat verlässt den Draht also **vor** dem Subscribe. Im Broker laufen P0 und P1 auf
getrennten Verbrauchern (`transport/server_v3/verbindung.rs:583` `let verbraucher_p0 = {`,
`:616-634` `eqcop-v3-ingress-rest` mit `Familie::P1 => senke.p1_mit_minor(…)`).

**Station 2 — jeder Heartbeat markiert die Sitzung dirty und flusht mit Verursacher.**
`broker/src/coordinator/befehl.rs:771-778`:

```
                let _ = self.heartbeat_kontakt(link_id, Some(&wert));
                let duplicate_instance_id = self.alias_quarantaenisiert(link_id);
                Some(
                    format!(
                        "{{\"type\":\"heartbeat_ack\",\"sequence\":{sequence},\"duplicate_instance_id\":{duplicate_instance_id}}}"
```

`coordinator/liveness.rs:505-506` (bedingungslos, ohne Änderungsvergleich):

```
                stand.dirty_sessions.extend(dirty_sessions.iter().cloned());
                stand.dirty_sessions.insert(session.clone());
```

und `:535-537`:

```
        for session in dirty_sessions {
            self.flush_session(&session, Some(link_id));
        }
```

**Station 3 — der Flush behandelt den abgeleiteten Snapshot als verweigerbare Annahme.**
`coordinator/flush.rs:147-170`:

```
        let mut event_ord = None;
        if let Some(store) = &self.store {
            let mut event = StoreEvent::session_snapshot(
            …
            match store.append(vec![event]) {
                Ok(ausgaenge) => event_ord = ausgaenge.first().map(|a| a.event_ord()),
                Err(_) => {
                    let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
                    stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
                    if let Some(link_id) = verursacher_link {
                        if let Some(link) = stand.links.get_mut(link_id) {
                            link.trennen = true;
                        }
                    }
                    return;
                }
```

Geprüft wird nur, ob ein Store existiert (`:148`), nicht `store_degradiert()` (`:18-22`). Der
Resubscribe dagegen kennt den Degradationszweig: `subscription.rs:143-145`
(`let degradiert = self.store_degradiert(); let projektion = if degradiert { None } …`) und
`:198` (`None if degradiert => super::flush::MARKE_OHNE_ORDINAL,`).

**Station 4 — der degradierte Handle verweigert sofort.** `store/handle.rs:148-151` →
`append_einreihen` (`:156-171`) → `senden`, `:219-223`:

```
    fn senden(&self, befehl: WriterBefehl) -> Result<(), StoreFehler> {
        if let Some(grund) = self.sicht().grund.filter(|_| self.sicht().degradiert) {
            self.verweigerung_zaehlen();
            return Err(StoreFehler::Degradiert(grund));
        }
```

Ohne Writer-Thread ebenso (`:224-231`), bei vollem Kanal `:239-242`
`Err(TrySendError::Full(_)) => { self.verweigerung_zaehlen(); Err(StoreFehler::KanalVoll) }`.
`flush.rs:160` fängt alle drei Fälle mit `Err(_)` gleich ab.

**Station 5 — der 100-ms-Tick schließt.** `lebenslauf.rs:181-185`:

```
                    while !stop_fuer_thread.load(Ordering::SeqCst) {
                        std::thread::sleep(Duration::from_millis(100));
                        for link_id in coordinator_fuer_thread.liveness_tick() {
                            closer.link_schliessen(&link_id);
                        }
```

mit `liveness.rs:264-269` (`.filter(|(_, link)| link.trennen)`). Die Control-Trennung reißt die
gekoppelte Telemetrie mit (`transport/server_v3/trennung.rs:156-169`, `control_abmelden` und
`io_abbrechen`).

**Station 6 — der Client verbindet nach 500 ms neu, ohne Dämpfung.**
`controlclient/Verbindung.cpp:448-452` (`const bool stand = eineVerbindung (…); … if (stand)
backoffMs = kBackoffStartMs;`) und `:474-479` (Warten `backoffMs`), `kBackoffStartMs = 500`
(`eq-copilot/plugin/core/ipc/IpcVerbindung.h:34`). Jeder Neuaufbau erreicht das Welcome und setzt
den Backoff zurück; der nächste Heartbeat trennt wieder. **Zyklus:** Aufbau → Heartbeat → ≤ 100 ms
Tick → Schluss → 500 ms → Aufbau.

**Station 7 — die Degradation hat keinen Rücksetzer, der Broker kein Ende.**
`store/handle.rs:495-502` (`degradiere` setzt nur `s.degradiert = true;`); der einzige Wert
`false` ist der Default `store/mod.rs:220`. Degradationsstellen: Start `writer.rs:16-31`
(`degradiert_ohne_pfad`), `:93-94` (Öffnungs-/Migrationsfehler), Laufzeit `:345-353`
(Commitfehler), `:367-369` (Kompaktierung), `:374-376` (Guard schreiben), `:385-387` (Guard
löschen). Das Leerlaufende braucht null aktive Clients über `BROKER_IDLE_ENDE_MS = 60_000`
(`lebenslauf.rs:45`, `:280-281`); die kreisenden Clients verhindern es.

**Station 8 — Startvariante: `routing_bereit` bleibt falsch, der Riegelrestore entfällt.**
`lebenslauf.rs:143-151`:

```
            let store = match store::StoreKonfiguration::standard() {
                Ok(konfiguration) => store::StoreWriter::starten(konfiguration),
                Err(fehler) => store::StoreWriter::degradiert_ohne_pfad(fehler.to_string()),
            };
            …
            let coordinator = Arc::new(coordinator::Coordinator::instant_mit_store(
```

`coordinator/mod.rs:316-317`:

```
        stand.routing_bereit = !store_writer.ist_degradiert();
        if stand.routing_bereit {
```

Guard- und Domänenrestore liegen in diesem Block (`:318-332`). **Vollständige Setzerliste von
`routing_bereit`** (Suche `routing_bereit` über `broker/src`): wahr nur durch den Default
`zustand.rs:624` (`routing_bereit: true,`, gilt für `mit_uhr` ohne Store) und `mod.rs:316` bei
gesundem Start; falsch durch `mod.rs:316` bei degradiertem Start und `flush.rs:33`
(`routing_fail_closed`). Kein Setzer macht es nach der Konstruktion wieder wahr.

**Station 9 — P2 ist an `routing_bereit` und an `trennen` gebunden.**
`coordinator/senke.rs:151-165`:

```
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(key) = Self::aktueller_telemetrie_client_locked(&stand, link_id) else {
                return;
            };
            let Some(link) = stand.links.get(link_id) else {
                return;
            };
            if !self.dispatch_fuer_link_erlaubt_locked(&stand, link) {
                let _ = Self::messframe_abweisen_locked(
                    &mut stand,
                    link_id,
                    P2RejectGrund::RoutingNichtFreigegeben,
                );
                return;
            }
```

`flush.rs:289-291` (`stand.routing_bereit && !link.trennen && …`). **Aber:**
`aktueller_telemetrie_client_locked` prüft vorher `link.trennen` (`link.rs:640`
`if link.trennen {` → `return None;`). Nach dem ersten Heartbeat einer Verbindung fällt P2
deshalb an `senke.rs:152-154` **still** — kein `messfehler`, kein `p2_reject`, kein
`p2_live_frames`. `routing_nicht_freigegeben` entsteht nur für P2, das vor dem ersten Heartbeat
der Verbindung eintrifft. Der Liveframe-Push an das Main verlangt zusätzlich `routing_bereit`
(`senke.rs:243-244`, `subscription.rs:399-400`).

**Station 10 — die ehrliche Diagnose erreicht Gen nicht.** `store_degraded` entsteht in der
Snapshotbildung `sicht.rs:657-658` (`if self.store_degradiert() {
objekt.insert("store_degraded".into(), Value::Bool(true)); }`). Zugestellt wird sie nur:
(i) im Flush — der kehrt bei degradiertem Store vor dem Push zurück (`flush.rs:168`), und seine
Zielliste verlangt `routing_bereit` (`:113-114`); (ii) im Resubscribe — der kehrt bei
`link.trennen` vorher zurück (`subscription.rs:103`). Der Heartbeat hat den Link zu diesem
Zeitpunkt regelmäßig schon markiert (Station 1). Der Heartbeat-ACK trägt kein Diagnosefeld
(`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:942-955`, `additionalProperties: false`), und das
Plugin liest `duplicate_instance_id` nirgends (Suche über `eq-copilot/plugin` ohne `tests/`:
kein Treffer). In Gen: jede neue Verbindung setzt `authenticating` ohne Handgriff
(`SourcesModel.cpp:635-636`), jeder Linkverlust `brokerUnavailable` **mit** Handgriff
(`:666-672`), jeder Verbindungsversuch wieder `authenticating` (`:682-685`). Sichtbar wird im
Wechsel „Authenticating" und „Broker unavailable" samt Knopf „Reconnect"
(`PluginEditor.cpp:227`, Sichtbarkeit `:1229-1233`), der die Degradation nicht heilen kann.
Selbst wenn der Subscribe das Rennen gewinnt und `storeDegraded` kurz erscheint
(`SourcesModel.cpp:1468`), überschreibt der nächste Verbindungsversuch es mit `authenticating`
(`:682-685` prüft `storeDegraded` nicht; nur `controlEnde` `:666-667` schont es).

**Ergänzung 1 — auch der gesunde, aber volle Store trennt.** `flush.rs:160` behandelt
`KanalVoll` wie `Degradiert`. Stehen `STORE_KANAL_CAP = 256` Aufträge (`store/mod.rs:48`), wird
jeder Heartbeat-Absender getrennt, obwohl sein Heartbeat kein persistenzpflichtiges Ereignis
ist. Der Gegenfall des Audits „gesunder Store trennt nie" (`BEFUNDE.md:643`) gilt heute also nur
unterhalb der Kanalgrenze. Hergeleitet, nicht gemessen.

**Ergänzung 2 — der Heartbeat-ACK meldet einen falschen Duplikatbefund.** `flush.rs:300-308`:

```
    pub(super) fn alias_quarantaenisiert(&self, link_id: &str) -> bool {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand.links.get(link_id).is_some_and(|link| {
            !stand.routing_bereit
                || stand.guard_gesetzt(&effektive_adresse(&link.adresse))
```

Im Startfall trägt jeder Heartbeat-ACK `"duplicate_instance_id":true` — im Auditlauf gemessen
(`components-run.txt:6`, `main_ack` und `probe_ack` jeder Runde). Das Schema sagt dazu
„Duplikate entstehen durch Kopieren einer Plugininstanz im Host"
(`eq-ipc-v3.schema.json:950`). Heute ohne Anzeigewirkung, weil kein Produktleser existiert.

**Ergänzung 3 — dieselbe Verursachertrennung hängt an drei weiteren Snapshotflushs, und
einer davon ist persistenzpflichtig.** `flush_session(…, Some(link_id))` rufen außer dem
Heartbeat auch `befehl.rs:616` (nach `session_command`), `experiment_verdrahtung.rs:365` (nach
einem Experimentterminal) und `liveness.rs:936` (`descriptor_setzen`). Zwei davon sind
abgeleitet: das Experimentterminal persistiert seine Domäne vorher selbst (Kommentar
`experiment_verdrahtung.rs:350-360`), `descriptor_setzen` seine Invalidierung ebenso
(`liveness.rs:914`, `:927`). **`session_command` hat dagegen keinen eigenen Append** (in
`befehl.rs:476-616` kein `store.`-Aufruf); der Snapshot-Append im Flush `:616` ist die einzige
Persistenz seiner Wirkung — und `:617` quittiert danach bedingungslos `angewandt`. Das ist der
offene Registerpunkt NAK-125 (`docs/offene-punkte.md:161`: „`session_command` quittiert
„angewandt" ohne Persistenz bei degradiertem Store", Planarbeit S29–31). Dieser eine Flush ist
persistenzpflichtig; die Verursachertrennung dort ist von S-03 gedeckt.

**Erreichbarkeit: Stufe 1** in beiden Varianten. Start: `StoreKonfiguration::standard()`
scheitert oder `StoreWriter::starten` degradiert (Remote-Volume, neueres Schema,
Öffnungs-/Migrationsfehler). Laufzeit: ein einzelner Commit- oder Kompaktierungsfehler, etwa
`SQLITE_BUSY` über `BUSY_TIMEOUT_MS = 2000` (`store/mod.rs:49`).

**Zahlenränder und Fristen.** 100-ms-Tick (`lebenslauf.rs:182`); 500-ms-Backoff, Maximum 8000
(`IpcVerbindung.h:34-35`), nach jedem Welcome zurückgesetzt; Heartbeat 1000 ms;
`store_verweigerungen` sättigt bei `u64::MAX` (`flush.rs:162`) und wächst um genau 1 je
Heartbeat, weil der Flush die Dirty-Marke vor dem Append verbraucht (`:92-94`) und der Tick
danach nichts mehr flusht — im Auditlauf 2, 4, 6, 8, 10 für zwei Clients über fünf Runden
(`components-run.txt:6`, `:8`). `p2_live_frames` bleibt im Startfall 0, im Laufzeitfall bei dem
einen vor der Degradation angenommenen Frame (1).

### c) Zusage wörtlich

- `docs/beweise/SONDE-011.md:535` (S-03): „Bei vollem Kanal oder degradiertem Store wird die
  Annahme persistenzpflichtiger Ereignisse verweigert und der Verursacher getrennt; nichts
  Unhaltbares wird bestätigt. Disk-full, I/O-Fehler, beschädigte/neue DB stoppen nicht den
  Broker: sichtbarer gezählter Betrieb ohne Historie, P2 läuft weiter."
- `SONDE-011.md:629` (L-08): „Bei vollem Kanal oder degradiertem Store wird die Annahme
  persistenzpflichtiger Ereignisse verweigert und die Verursacherverbindung getrennt;
  Wiederholung kommt vom Sender-Reconnect. Disk-full, I/O-Fehler oder beschädigte DB stoppen den
  Broker nicht: sichtbar gezählter Betrieb ohne Historie, keine falsche Annahmebestätigung, P2
  läuft weiter."
- `SONDE-011.md:533` (S-01): „Remote-Volume, neueres Schema oder Storefehler öffnen/schreiben
  nichts und führen sichtbar in L-08-Degradation; Routing bleibt bis zum Konfliktriegel-Restore
  fail-closed, P2 darf weiterlaufen."
- `SONDE-011.md:560` (O-01): „Voller/degradierter Store verweigert Annahme und trennt den
  Verursacher; P2 bleibt aktiv."
- `SONDE-011.md:513` (C-07): „Misslingt Persistenz oder Restore, bleibt Routing fail-closed.
  … kein Routing vor erfolgreichem Guard-Restore." — trägt die Startsperre, nicht die
  Trennschleife.
- `docs/beweise/SONDE-012.md:485-488` (E-L15-Store): „`session_snapshot` erhält ein optionales
  boolesches Feld `store_degraded`; nur `true` reist, Abwesenheit behauptet nichts … Main zeigt
  den Diagnosezustand nur bei `true`."
- `SONDE-012.md:155` (L15): „Der reale Zustand ist Diagnose, nicht Connected-Ersatz. Ein
  sichtbarer Handgriff erscheint nur, wenn er die konkrete Recovery ausführt".
- `docs/FL-Nakama-Sonden-Design-Entwurf.md:2176-2177`: „Fehlt oder wird die SQLite-Datenbank
  gelöscht, bleiben Projekt-Recall und DSP vollständig; lediglich historische Experimente und
  gelerntes Ranking fehlen."
- `CLAUDE.md:44-45`: „jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich
  einen Zustand."

Der Heartbeat trägt kein persistenzpflichtiges Ereignis; der Session-Snapshot ist laut Vertrag
„Fluechtiger Brokerzustand — aus Plugin-Reports, Main-State und Store rekonstruierbar"
(`eq-ipc-v3.schema.json:1071`; gleich Entwurf `:2172`). S-03/L-08 begrenzen die
Verursachertrennung auf persistenzpflichtige Ereignisse und verlangen zugleich weiterlaufenden
Betrieb mit P2 — beides kann nicht gelten, wenn der 1-Hz-Heartbeat der Verursacher ist.

**Irreführende Texte:** `subscription.rs:194-197` („Nach der Degradation committet kein Flush
mehr (`flush.rs`, der Append scheitert)") verschweigt, dass derselbe Append den Absender trennt
und den Livestand-Weg dieser Zeilen damit unerreichbar macht. `docs/plugin-wissen.md:2273-2275`
(„Storefehler stoppen den Broker nicht: persistenzpflichtige Annahmen werden verweigert und der
Verursacher getrennt, P2 kann sichtbar degradiert weiterlaufen.") beschreibt einen Zustand, den
der Code nicht erreicht. `liveness.rs:240-241` nennt für den 100-ms-Supervisor `lib.rs`; er
steht in `lebenslauf.rs:178-186`. `writer.rs:33-35` ist korrekt.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Was sie nicht misst |
|---|---|---|
| A4 `store_crash_matrix.rs:1558-1576` `degradierter_store_verweigert_annahme_und_tick_trennt_verursacher` | ein **Heartbeat** (`si_report` ist `heartbeat_kontakt`, `:347-360`) bei degradiertem Store: `store_verweigerungen() == 1`, `verbindung_soll_trennen`, `liveness_tick() == ["verursacher"]` | **schreibt den Defekt als gewollt fest**: der Heartbeat ist hier der „Verursacher". Keine zweite Runde, kein Reconnect, kein P2, keine Gen-Sicht. SONDE-011:682 führt ihn als Beleg für S-03 |
| A4 `:1545-1556` `disk_full_io_kaputt_neues_schema_degradieren_ohne_brokerstop` | Writer degradiert, `append` liefert `Degradiert` | kein Coordinator, kein Client |
| A4 `:1514-1543` `store_kanal_cap_und_naechster_trennt` | 256 Aufträge angenommen, der 257. `KanalVoll` | **keine Trennung** — der Name behauptet mehr, als der Test misst |
| A4 `:2178-2189` `routing_bleibt_zu_bis_konfliktriegel_restauriert` | degradierter Start → `!routing_bereit()` | nur die Sperre, kein P2 |
| A4-SI `:2850-2900` `blockierter_store_writer_bestaetigt_nichts_und_p2_laeuft` | **blockierter**, nicht degradierter Writer; P2 über `control_registrieren` ohne Heartbeat → `p2_live_frames == 1` | `routing_bereit` bleibt wahr; kein Heartbeat, kein Tick. SONDE-011:682 führt ihn als Beleg für „P2 läuft weiter" |
| A4 `tests_abonnement.rs:1187-1256` | Laufzeitdegradation, Resubscribe liefert Livestand mit `store_degraded: true` und Hochwassermarke | kein Heartbeat danach — mit ihm wäre der Link getrennt |
| A4 `sonde012_sources_slice.rs:1395-1418` `store_degraded_travels_only_as_true` | Main meldet sich an und abonniert **ohne** Heartbeat; Push trägt `store_degraded` | genau die Reihenfolge, die der Produktclient nie fährt (Station 1) |
| B13 `Sonde012SourcesModelTest.cpp:885-902` | Snapshotfolge mit `store_degraded` → `storeDegraded` ohne Handgriff | Fixture, kein Transport; das Flackern über `setzeControlTransport` misst er nicht |
| A22 (`tools/beweise.ps1:637`) | 32 C++-Sondenpaare gegen den echten Listener | fährt `eqcop-broker-v3probe` mit `ZaehlSenke` (`broker/src/bin/eqcop-broker-v3probe.rs:25`, `:49`) — **kein Coordinator, kein Store** |
| B8 (`tools/beweise.ps1:779`) | Lebenslauf-Klassifikation, kein Brokeraufruf aus dem Audio-Thread | berührt den Weg nicht |

Behauptungszeilen wörtlich: A4-SI `tools/beweise.ps1:527` „SONDE-011 Phase-B-Systemintegration
auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber
Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute
session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus
und bereinigen Eviction/Nonce vor spaeterem Push. …"; A4 `:518` beginnt „Broker-Vertragstests:
eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner
Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften
Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix."
— keine der beiden Zeilen nennt Degradationsbetrieb, Heartbeat-Trennung oder P2 bei
degradiertem Store. A22 `:637` „Ende-zu-Ende ueber die PROBE-Pipe … Keine Verbindung wird wegen
Envelope, Rate oder P0-Ueberlauf geschlossen." B8 `:779` „Lifecycle-Klassifikation §53.5 bleibt
erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() … null
Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. …"

### e) Urteil: BESTÄTIGT

Alle zehn Stationen am HEAD gelesen; die Kette schließt, und kein Commit seit `aff2d818` hat eine
tragende Datei geändert. Ergänzt: (1) P2 fällt nach dem ersten Heartbeat still an
`link.rs:640`, nicht als `routing_nicht_freigegeben` — Gen bekommt nicht einmal einen
`p2_reject`; (2) auch ein voller Kanal bei gesundem Store trennt jeden Heartbeat-Absender; (3)
das A4-Bein `:1558-1576` schreibt den Defekt als gewollt fest und dient SONDE-011:682 als Beleg;
(4) der Heartbeat-ACK meldet im Startfall `duplicate_instance_id: true` für jeden Link; (5) Gen
zeigt einen „Reconnect"-Knopf, der nichts heilen kann; (6) von den vier Snapshotflushs mit
Verursacher ist genau einer persistenzpflichtig — der nach `session_command` (`befehl.rs:616`,
NAK-125) —, der Fix muss die Flusharten deshalb unterscheiden. Die Phase-16-Zahlen (zehn Trennungen je
fünf Runden, `p2` 0 bzw. 1, `refusals` 2…10; gesund 0 Trennungen, P2 1→5, Snapshots 1→13)
stammen aus `components-run.txt:5-8`; ich habe sie nicht nachgefahren.

### f) Kategorie und Schwere

**DEFEKT** gegen S-03, L-08, O-01 (zweite Satzhälfte: Betrieb ohne Historie, P2 läuft/bleibt
aktiv) und in der Startvariante zusätzlich S-01 („P2 darf weiterlaufen"). **Schwere high:** im
zugesagten Weiterbetriebsfall ist jede Instanz dauerhaft in einer Trennschleife, P2 erreicht
Gen nicht, und Gen zeigt eine falsche Ursache mit einem wirkungslosen Handgriff; Auslöser ist
selbst ein Fehlerfall, Audio und gespeicherte Identität bleiben unberührt.

### g) Kleinster Fix

**Ort:** `flush_session` (`flush.rs:58-221`), `p2` (`coordinator/senke.rs:134-270`),
`messframes_an_subscriber_push` (`subscription.rs:385-423`), `alias_quarantaenisiert`
(`flush.rs:300-309`).

0. **Zwei Flusharten:** `flush_session` erfährt vom Aufrufer, ob er eine Befehlswirkung
   persistiert (heute nur `befehl.rs:616`, `session_command`) oder einen abgeleiteten Stand
   liefert (alle übrigen Aufrufer, Liste unter „Regelfragen", Frage 1). Der
   persistenzpflichtige Flush verhält sich wie heute (Verweigerung, Zählung,
   Verursachertrennung); die Frage nach dem ACK `angewandt` bleibt NAK-125 und wird hier nicht
   verschärft — insbesondere geht für ihn **kein** Livestand ohne Commit hinaus, der die
   Befehlswirkung als bestehend zeigt.
1. **Abgeleiteter Flush bei degradiertem Store:** kein Append-Versuch; der Snapshot geht als
   Livestand mit `MARKE_OHNE_ORDINAL` (`flush.rs:15`) an die Abonnenten, wie im Resubscribe
   (`subscription.rs:143-145`, `:198`). Die Zielliste nimmt dafür
   `(stand.routing_bereit || self.store_degradiert())` statt `stand.routing_bereit`
   (`flush.rs:114`) — dieselbe Ausnahme, die `push_ziel_noch_gueltig` schon trägt
   (`subscription.rs:437`). Keine Verursachertrennung, keine Schuldkompaktierung, kein
   `event_ord`.
2. **Abgeleiteter Flush bei gesundem Store mit Fehler** (`KanalVoll`, `Beendet`): keine
   Verursachertrennung; die Sitzung wird wieder dirty markiert, der nächste Tick (≤ 100 ms)
   flusht neu. Gezählt wird weiter.
3. **P2 und Liveframe-Push:** das Tor in `senke.rs:158` und die Zielfilter `senke.rs:243-244`
   und `subscription.rs:399-400` erlauben P2 bei `routing_bereit || store_degradiert()`, mit
   unveränderter Prüfung von `trennen`, Guard und Alias je Adresse. Evidenz, Befehle und
   Auto-Join bleiben am unveränderten `routing_bereit` (`senke.rs:266-269`,
   `mitgliedschaft.rs:17`, `befehl.rs:285`, `:318`, `:506`) — C-07 bleibt gewahrt.
4. **Duplikatflag:** `alias_quarantaenisiert` rechnet `!stand.routing_bereit` nicht mehr ein
   (`flush.rs:303`); der Test `heartbeat_ack_meldet_alias_quarantaene_fuer_beide_links`
   (`coordinator/mod.rs:574`) bleibt grün.
5. Persistenzpflichtige Wege bleiben unverändert: `persistenz_p0` (`befehl.rs:212-215`,
   `:439-471`), der Flush nach `session_command` (`:616`), `liveness.rs:513-534`, `:914`,
   `:927`, `befehl.rs:875-899` trennen weiter.
6. Texte: `subscription.rs:194-197`, `liveness.rs:240-241`, `docs/plugin-wissen.md:2273-2275`.

**Schema:** keine Änderung — `store_degraded` existiert (`eq-ipc-v3.schema.json:1098-1102`), der
Livestand ist ein gültiger `session_snapshot`. **Mitzuprüfende Beziehungen:** verbinden↔trennen
(gesund: 0 Trennungen; degradiert: 0 Trennungen durch Heartbeats; persistenzpflichtiger Befehl
bei vollem Kanal oder degradiertem Store: weiter getrennt), degradieren↔erholen (keine Erholung
zugesagt; die Degradation bleibt einseitig, das ist kein Teil dieses Fixes), sperren↔freigeben
(`routing_bereit` bleibt im Startfall falsch, Riegelrestore bleibt aus). **Audio-Thread:** nicht
berührt. **Pipe-Latenz:** sinkt (kein Append-Versuch, keine Reconnectschleife). **NAK-310:**
keine Datei von Bootstrap oder Pipe-Autorisierung; `alias_quarantaenisiert` ist die
C-07-Identitätssicht — dort ändert sich nur die Einrechnung des Routingstands, keine
Kollisionsregel. **Berührung mit Teil A:** `flush_session` ist auch der Ort von
W14/T3-08-02/T3-08-03 (Commit- und Publikationsordnung) — Änderungssätze nacheinander, nicht
parallel; der Livestand ohne Commit darf nichts als committet ausgeben (W15-Abhängigkeit
`BEFUNDE.md:818`: „W13/W14 nicht durch unsichere Fakecommits umgehen").

### h) Ort des Rotbeweises

**Bein A4**, `broker/tests/store_crash_matrix.rs` neben `:1558-1576`, deterministisch mit
`ManualClock` und echter SQLite. Der Folgebeweis des Audits läuft vollständig in A4: Startvariante
wie `components.rs:76-110` (DB-Pfad ist ein Verzeichnis, echter degradierter Writer),
Laufzeitvariante mit gehaltenem `BEGIN IMMEDIATE` über `BUSY_TIMEOUT_MS` wie
`tests_abonnement.rs:1206-1217` (echte 2 s, deterministisch). Main und Probe über
`Senke::control_verbunden`, Probe `telemetrie_gekoppelt`, Main abonniert; **fünf Runden** aus
`clock.vor(1000)`, Heartbeat über `Senke::p0` für beide (P0 vor dem Subscribe, wie der
Produktclient), Subscribe, ein P2-Frame, `liveness_tick()`. Assertionen:

- `liveness_tick()` liefert in keiner Runde einen Link, `store_verweigerungen()` wächst nicht
  durch Heartbeats, `p2_live_frames()` steigt in beiden Varianten je Runde um 1, jeder
  Subscribe/Flush liefert einen Snapshot mit `store_degraded: true`, jeder Heartbeat-ACK
  `duplicate_instance_id: false` — **heute rot** (zehn Trennungen, `p2` 0 bzw. 1, keine
  Snapshots).
- Startvariante zusätzlich: `routing_bereit()` bleibt falsch, ein `session_command` wird
  `abgelehnt`, kein Auto-Join.
- Gegenfall gesund: 0 Trennungen, P2 1→5.
- Gegenfall persistenzpflichtig: in der Laufzeitvariante (Routing frei, Store degradiert) und
  bei vollem Kanal (Muster `store_kanal_cap_und_naechster_trennt`, `:1514-1543`, mit
  Startbarriere) trennt ein persistenzpflichtiger P0 über `persistenz_p0`
  (`befehl.rs:439-471`) und ein `session_command` (Wirkung über `:616`) den Verursacher weiter.
  In der Startvariante lehnt das Routingtor den `session_command` schon vorher ab
  (`befehl.rs:285`, `:506`).

Der bestehende Test `:1558-1576` wird umgeschrieben: sein Verursacher wird ein
persistenzpflichtiges Ereignis; der Heartbeatfall erwartet „keine Trennung". Mutationen an den
Zeilen, die die Zusage tragen: den abgeleiteten Zweig in `flush_session` wieder
`link.trennen = true` setzen lassen → Runde 1 reißt; das P2-Tor `senke.rs:158` wieder an
`dispatch_fuer_link_erlaubt_locked` binden → Startvariante `p2_live_frames` 0 → rot.

**Bein A4-SI** für die reale Clientschleife: ein neuer Modus des B10-Binaries nach Muster
`--phase-b-command-client` (`IpcTestMain.cpp:4427`, Aufrufer
`store_crash_matrix.rs:258-275`) gegen einen Phase-B-Server mit degradiertem Store: echter
`ControlClient` mit P0 vor P1 und 500-ms-Backoff, gezählte Verbindungsversuche über 5 s
(erwartet: einer), Gen-Diagnose `storeDegraded` stabil. **A22** kann den Beweis nicht tragen
(kein Coordinator, kein Store). **B13** für die Gen-Seite: `setzeControlTransport` darf ein
geliefertes `storeDegraded` nicht auf `authenticating` zurücksetzen, solange die Verbindung
steht (Nachbar `Sonde012SourcesModelTest.cpp:935-950`).

---

## 2. T3-15-13 — transienter Lesefehler bei gesundem Writer: Routing fail-closed, Gen zeigt „Authenticating" (W15)

*Befundzeile: `BEFUNDE.md:644` · Herabstufung: `:658` · Arbeitsübersicht W15: `:818` ·
Rohbelege: `roh/phase-15-paket3-teilfehler.md:158-192` (P15-T-02),
`roh/phase-15-gegenpruefung-paket3.md:122-179` (K2), `roh/phase-15-zentral.md:37`, `:58`*

### a) Behauptung des Audits

Nach einem transienten Store-Lesefehler bei gesundem Writer bleibt das Routing brokerweit
dauerhaft fail-closed; die Sperre ist zugesagt, aber Gen zeigt dazu weiter „Authenticating"
ohne Handgriff — der Zustand wird nicht ehrlich gemeldet (`BEFUNDE.md:644`, verengt nach
Gegenprüfung `:658`).

### b) Quellkette am HEAD

**Station 1 — Lesefehler oder Vertragsbruch im Resubscribe.** `subscription.rs:147-156`:

```
            match &self.store {
                Some(store) => match store
                    .session_state_lesen(&session.project_binding_id, &session.session_epoch)
                {
                    Ok(projektion) => projektion,
                    Err(_) => {
                        self.routing_fail_closed("Sessionprojektion konnte nicht gelesen werden");
                        return;
                    }
                },
```

Weitere Aufrufer desselben Latches: `:163`, `:176`, `:215`, `:228` (Projektionsvertrag),
`flush.rs:242` und `link.rs:329` (Konfliktriegel nicht persistiert). Der Lesepfad ist eine
kurze Read-only-Verbindung mit `BUSY_TIMEOUT_MS = 2000` (`store/handle.rs:256-271`, `:483-493`).

**Station 2 — der Latch ist brokerweit und verwirft seinen Grund.** `flush.rs:31-37`:

```
    pub(super) fn routing_fail_closed(&self, _grund: &str) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand.routing_bereit = false;
        for link in stand.links.values_mut() {
            link.trennen = true;
        }
    }
```

Der Grund (`_grund`) wird nirgends gehalten. Setzerliste wie in 1b Station 8: kein Setzer macht
`routing_bereit` wieder wahr.

**Station 3 — Neuaufbau gelingt, liefert aber nichts, und liest nie wieder.** `link.rs`
enthält kein `routing_bereit` (Suche) — Hellos werden angenommen. Der Subscribe trägt die
Subscription ein (`subscription.rs:72-79`) und ruft den Resubscribe (`:80`), der vor jedem
Lesen zurückkehrt, `:103-113`:

```
            if link.trennen
                || (!stand.routing_bereit && !self.store_degradiert())
                || stand.guard_gesetzt(&effektive_adresse(&link.adresse))
                …
            {
                return;
            }
```

Das Lesen (`:147-156`) liegt dahinter. **Nach dem Latch wird die Projektion also nie wieder
gelesen** — auch wenn die Datei längst wieder lesbar ist. Heartbeat-Flushs laufen mit gesundem
Writer durch (`flush.rs:158-159`), haben aber keine Ziele (`:114`); P2 fällt mit
`routing_nicht_freigegeben` (`senke.rs:158-165`), Auto-Join ist aus (`mitgliedschaft.rs:17`),
`confirm_join` wird abgelehnt (`befehl.rs:285`, `:506`). Der Zustand ist stabil: verbunden,
Heartbeats quittiert, keine Sitzungsdaten.

**Station 4 — Gen zeigt „Authenticating" ohne Handgriff.** `SourcesModel.cpp:624-637`
(`beginneSubscription`, bei jedem Linkaufbau aus `Ipc.cpp:1035`):

```
    subscriptionAktiv = false;
    …
    diagnose = Diagnose::authenticating;
    diagnoseHatHandgriff = false;
```

und `:681-685` (`setzeControlTransport`, aus `Ipc.cpp:1116` je Tick):

```
    bool handgriff = false;
    if (transport.status == nakama::ipc::ControlClient::Status::verbindet
        || (transport.status == nakama::ipc::ControlClient::Status::verbunden
            && ! subscriptionAktiv))
        neu = Diagnose::authenticating;
```

`subscriptionAktiv` wird nur in der Snapshotübernahme wahr (`:1467`), und kein Snapshot kommt.
Text `SourcesModel.cpp:2092` `case Diagnose::authenticating: return "Authenticating";`; der
Knopf „Reconnect" bleibt verborgen (`PluginEditor.cpp:1229-1233`, nur bei
`brokerUnavailable && diagnoseHatHandgriff`). Der Diagnosekatalog (`SourcesModel.h:46-55`)
kennt keinen Zustand „Routing gesperrt".

**Station 5 — der Draht behauptet dabei ein Duplikat.** Wie 1b Ergänzung 2: bei
`!routing_bereit` meldet jeder Heartbeat-ACK `duplicate_instance_id: true`
(`flush.rs:303`, `befehl.rs:772-775`). Ohne Produktleser folgenlos, aber der einzige Ort, an dem
der Broker den Zustand heute auf den Draht bringt — unter falschem Namen.

**Präzisierung gegen die Gegenprüfung.** `phase-15-gegenpruefung-paket3.md:139-151` stuft
herab, weil der Dauerfall „innerhalb eines Heartbeats auch den Writer" degradiere und dann über
die Ausnahmen `subscription.rs:104`/`:437` „ehrlich" als `storeDegraded` lande. Am HEAD trägt
das nicht: bei degradiertem Writer trennt der nächste Heartbeat seinen Absender, bevor der
Subscribe durchkommt (Abschnitt 1, Stationen 1 und 10). Die ehrliche Landung des Dauerfalls
entsteht erst mit dem Fix zu T3-15-12.

**Erreichbarkeit: Stufe 1**, aber eng: ein Lesefehler, der nicht zum Schreibfehler wird —
`SQLITE_BUSY` über 2000 ms auf der Leseverbindung oder eine kurze Fremdsperre der Datei; dazu
ein Projektionsvertragsbruch. Häufigkeit nicht bestimmbar (siehe „Nicht geprüft").

### c) Zusage wörtlich

- Die Sperre: `SONDE-011.md:1061` (Erstprüfungsdefekt 4, Tabelle ab `:1048`):
  „`resubscribe_snapshot_push`/`routing_fail_closed`: Store-Lesefehler sperren Routing und
  markieren den Link zum Trennen; kein leerer Fallback." C-07 (`:513`): „Misslingt Persistenz
  oder Restore, bleibt Routing fail-closed. … keine zeitbasierte Freigabe; kein Routing vor
  erfolgreichem Guard-Restore."
- Die Anzeige: `CLAUDE.md:44-45` „**Keine toten UI-Elemente** (User 24.08.2026): jedes sichtbare
  Element führt einen Handgriff aus oder meldet ehrlich einen Zustand." SONDE-012 L15
  (`SONDE-012.md:155`): „Broker unavailable, authenticating, Join-Bestätigung nötig,
  incompatible oder Store degraded | Der reale Zustand ist Diagnose, nicht Connected-Ersatz. Ein
  sichtbarer Handgriff erscheint nur, wenn er die konkrete Recovery ausführt; bei automatischem
  Reconnect genügt Status."
- **Schweigen:** zu Wiederanlauf, Reichweite (brokerweit gegen sitzungsweit) und Hinweis sagt
  keine Quelle etwas (gesucht in SONDE-011, SONDE-012, Entwurf §33/§53.9, `plugin-wissen.md`,
  Schema-README). Die Arbeitsübersicht W15 (`BEFUNDE.md:818`, letzte Spalte): „Routing-
  Wiederfreigabe nach Lesefehler nur mit belegtem Restoreentscheid; aktuelle Sperre ist
  vorgesehen."

„Authenticating" ist nicht der reale Zustand: die Authentisierung ist mit dem Welcome
abgeschlossen, jeder Heartbeat wird quittiert.

**Irreführende Texte:** der Testname `projektionslesefehler_haelt_subscription_sichtbar_fail_closed`
(`store_crash_matrix.rs:1157`) — „sichtbar" ist nicht gemessen; `flush.rs:31` nimmt einen Grund
entgegen und verwirft ihn; das Schema-`$comment` zu `duplicate_instance_id`
(`eq-ipc-v3.schema.json:950`) gegen `flush.rs:303`.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Was sie nicht misst |
|---|---|---|
| A4 `store_crash_matrix.rs:1156-1172` | Writer gestoppt, DB gelöscht, Subscribe → `!routing_bereit()`, `verbindung_soll_trennen("main")`, keine Snapshots | Reconnect, fremde Sitzung, Gen-Diagnose, erneutes Lesen; schreibt die Sperre fest (richtig nach SONDE-011:1061), nicht die Anzeige |
| A4 `coordinator/mod.rs:574` `heartbeat_ack_meldet_alias_quarantaene_fuer_beide_links` | Duplikatflag bei echter Aliaskollision | den Routingzustand im Flag |
| B13 `Sonde012SourcesModelTest.cpp:935-950` `diagnostics_are_distinct_and_actions_are_live` | `verbindet` → `authenticating`, Fehlertext → `incompatible`, gesetztes `storeDegraded` bleibt | **setzt die Diagnose über `setzeDiagnoseFuerSichtbeweis`**, nicht über einen Brokerweg; der Fall „verbunden, kein Snapshot, Routing gesperrt" fehlt |
| Behauptungszeile B13 (`tools/beweise.ps1:750`) | Gen ersetzt sein Quellenmodell durch absolute Snapshot-Sichten | nennt keinen Routingzustand |

Die Gegenprüfung nennt C-07, S-01 und das Bein `:1156-1172` als Beleg, dass Sperre, Reichweite
und fehlender Zeitablauf gewollt sind — zutreffend. Kein Test schreibt „Authenticating" als
gewollt fest.

### e) Urteil: PRÄZISIERT

Die Kette schließt am HEAD (Code seit dem Audit nur verschoben). Präzisiert: (1) die falsche
Anzeige bricht `CLAUDE.md:44-45` und SONDE-012 L15 wörtlich — das ist ein Defekt, keine bloße
Härtung; (2) nach dem Latch gibt es **keinen** weiteren Leseversuch (`subscription.rs:103-113`
liegt vor dem Lesen), die Sperre überdauert also auch einen längst behobenen Lesefehler bis zum
Brokerende; (3) die Begründung der Herabstufung („Dauerfall landet ehrlich als `storeDegraded`")
trägt heute nicht, weil T3-15-12 diesen Weg abschneidet; (4) der Broker hält den Grund der
Sperre nicht (`_grund`), und der einzige Drahtausdruck ist ein falsches Duplikatflag.

### f) Kategorie und Schwere

**DEFEKT** gegen `CLAUDE.md:44-45` und SONDE-012 L15 (die Anzeige); die Sperre selbst bleibt
zugesagt (SONDE-011:1061, C-07) und ist kein Defekt. **Schwere low:** enger Auslöser, keine
falsche Messung, kein Datenverlust, Audio unberührt; aber ohne Hinweis und ohne Rückweg bis zum
Brokerende.

### g) Kleinster Fix

**Ort:** `routing_fail_closed` (`flush.rs:31-37`), Heartbeat-ACK (`befehl.rs:771-778`),
Gen `SourcesModel` (`Diagnose` `SourcesModel.h:46-55`, `setzeControlTransport`
`SourcesModel.cpp:676-708`, `wort` `:2087-2096`) und der Antwortweg in `Ipc.cpp`.

1. **Grund halten:** `routing_fail_closed` merkt sich die Art der Sperre (Projektionslesefehler
   bzw. Projektionsvertrag gegen Guard-Persistenz) im `Stand`.
2. **Zustand auf den Draht:** ein optionales Feld im `heartbeat_ack`, nach dem Muster von
   `store_degraded` „nur `true` reist" — der Heartbeat-ACK ist der einzige Weg, der in diesem
   Zustand verlässlich zugestellt wird; ein `session_snapshot` darf ohne lesbare Projektion nicht
   entstehen („kein leerer Fallback", SONDE-011:1061). `duplicate_instance_id` meldet danach nur
   noch Guard und Aliasquarantäne (wie 1g Punkt 4).
3. **Gen:** ein eigener Diagnosezustand für „Broker hält Sitzungsdaten zurück" mit ehrlichem
   Text nach dem Muster „Store degraded - no client recovery command"
   (`PluginEditor.cpp:1529-1530`); `setzeControlTransport` überschreibt ihn nicht mit
   `authenticating`. Kein Handgriff, solange keiner heilt (L15).
4. **Optional nach Regelfrage 2:** Wiederanlauf nur für die Sperrart „Projektionslesefehler",
   ausgelöst von einem neuen Subscribe (nicht von der Uhr), und nur nach einem erfolgreichen,
   vertragsgültigen Lesen derselben Projektion; Guard-Persistenzfehler bleiben dauerhaft
   fail-closed (C-07). Dann wird der vorhandene „Reconnect"-Knopf eine echte Recovery und darf
   nach L15 erscheinen.

**Schema:** **ja.** Das neue Feld in `heartbeat_ack` (`eq-ipc-v3.schema.json:942-955`,
`additionalProperties: false`) braucht einen Minorschritt der P0-Familie in der Fassungsleiter
(`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json`, `wire_envelope_schema_minor`),
Vertragstext, Rust- und C++-Leser und -Schreiber und Fixtures; der Broker sendet das Feld nur an
Clients der neuen Fassung, ältere Plugins bleiben gültig. Es ist kein Discriminator, keine
Zieladresse, keine Revision und keine Capability (`CLAUDE.md:178-181`), also additiv
zulässig. **Mitzuprüfende Beziehungen:** sperren↔freigeben (bei Punkt 4: Sperre bleibt, solange
das Lesen scheitert; Freigabe nur nach erfolgreichem Lesen; Guardfehler nie), verbinden↔trennen
(der Latch trennt weiter alle Links einmal). **Audio-Thread:** nicht berührt (Diagnose auf dem
Nachrichtenthread). **Pipe-Latenz:** ein Boolean im ACK. **NAK-310:** keine Bootstrap- oder
Autorisierungsdatei; `duplicate_instance_id` ist die C-07-Identitätsanzeige (Berührung genannt,
nicht bewertet).

### h) Ort des Rotbeweises

**Bein A4**, `store_crash_matrix.rs` neben `:1156-1172`: gesunder Writer, zwei Sitzungen mit je
Main und Probe; genau ein gescheitertes `session_state_lesen` über eine Testnaht nach Muster
`append_naht` (`store/handle.rs:126`, `:142-146`), danach Neu-Hello, Subscribe, Heartbeat, P2 in
beiden Sitzungen. Assertionen: `routing_bereit()` falsch, kein Snapshot, jeder Heartbeat-ACK trägt
das neue Feld und `duplicate_instance_id: false` — **heute rot** (kein Feld, Duplikatflag wahr).
Mit Punkt 4: nach Wegnahme der Naht liefert der nächste Subscribe den Projektionsstand und
`routing_bereit()` wird wahr; Gegenfall dauerhaft beschädigte Projektion (Datei gelöscht wie
`:1166-1167`) bleibt gesperrt, kein Snapshot verlässt den Broker; Gegenfall Guard-Persistenzfehler
bleibt gesperrt. **Bein B13**, `Sonde012SourcesModelTest.cpp` neben `:935-950`: Transport
`verbunden`, kein Snapshot, Heartbeat-ACK mit dem Feld → neue Diagnose statt `authenticating`,
kein Handgriff — **heute rot**. Mutation an der Zeile, die die Zusage trägt: die Abbildung des
Feldes in Gen entfernen → B13 reißt; das Setzen des Feldes im Broker entfernen → A4 reißt.

---

## 3. T3-16-02 — das späte Ende der alten Telemetrie entkoppelt die angenommene Nachfolgerin (W16)

*Befundzeile: `BEFUNDE.md:721` · Abschlusszeile F13: `:760` · Arbeitsübersicht W16: `:819` ·
Rohbelege: `roh/phase-16-b-verbindungen.md:14-41`, `roh/phase-16-f-gegenpruefung.md:10`,
`roh/phase-16-abdeckung.md:21`, `roh/phase-16-nachweise/b/wire-run.txt:67-68`,
Harnisch `wire.rs:108-140`*

### a) Behauptung des Audits

Das verspätete Ende einer alten Telemetrieverbindung entfernt die semantische Kopplung ihrer
bereits angenommenen Nachfolgerin; deren gültige Frames werden still ignoriert, Control und die
neue Pipe bleiben offen, ein weiterer Reconnect heilt; am echten Listener gemessen: Kopplungen
1 → 0, `p2_live_frames` bleibt 1 (`BEFUNDE.md:721`, `wire-run.txt:67`).

### b) Quellkette am HEAD

**Station 1 — das Leserende von T1 gibt den Telemetrieplatz vor den Joins frei.**
`transport/server_v3/verbindung.rs:810-838`:

```
    ende.setzen();
    …
    eingang.schliessen();
    if ist_control {
        …
    } else {
        telemetrie_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .remove(&link_id);
    }
    …
    kopplung_loesen(&kopplungen, &handles, &link_id, ist_control, &statistik);
    ausgang.schliessen();
    io_abbrechen(&handles, id, &statistik);
```

Für die Telemetrieseite ist das `trennung.rs:170-173`
(`k.telemetrie_entkoppeln(link_id);`), `bootstrap.rs:516-527`:

```
    /// Gegenpfad zu `telemetrie_koppeln`: die Kopplung bleibt bestehen (die
    /// Control-Verbindung lebt weiter), nur der Telemetrieplatz wird frei.
    pub fn telemetrie_entkoppeln(&mut self, link_id: &str) -> bool {
        match self.nach_link.get_mut(link_id) {
            Some(k) if k.telemetrie_verbunden => {
                k.telemetrie_verbunden = false;
                k.telemetrie_verbindung = None;
```

**Station 2 — die Joins dauern bis `SENKE_FRIST`, erst danach meldet T1 sein Ende.**
`verbindung.rs:843-864` (je Verbraucher und Schreiber `join_mit_frist(j, SENKE_FRIST, …)`,
`SENKE_FRIST = 2000 ms` `server_v3/mod.rs:221`), dann `:866-883`:

```
    // Erst JETZT die Senke benachrichtigen: waehrend der Joins konnte noch ein
    // `p0`/`p1`/`p2` derselben Verbindung laufen, …
    …
    melden_getrennt(&mut senkenruf, &link_id, ist_control);
    if !ist_control {
        telemetrie_getrennt_gemeldet(&trennmelder, &link_id);
```

**Station 3 — T2 koppelt im Fenster.** Der Client wartet nach seinem eigenen Schluss 500 ms
(`TelemetryClient.cpp:424-428` Rücksetzen auf `kBackoffStartMs` nach einer stehenden
Verbindung, `:447-452` Warten `backoffMs`) und koppelt mit denselben Werten; der Transport
lässt das zu, weil der Platz frei ist, `bootstrap.rs:478-490`:

```
        if k.challenge != h.challenge || k.runtime_nonce != h.adresse.runtime_nonce {
            return Err(BootstrapFehler::KopplungPasstNicht);
        }
        if k.telemetrie_verbunden {
            return Err(BootstrapFehler::KopplungSchonBelegt);
        }
        k.telemetrie_verbunden = true;
        k.telemetrie_verbindung = Some(verbindung);
```

Danach Welcome und `telemetrie_gekoppelt` (`verbindung.rs:420-468`, Callback `:459`).

**Station 4 — der Coordinator kennt die Kopplung nur als Control-`link_id`.**
`coordinator/senke.rs:58-78`:

```
    fn telemetrie_gekoppelt(&self, link_id: &str) {
        let neu = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let neu =
                stand.links.contains_key(link_id) && stand.telemetry_links.insert(link_id.into());
            if neu {
                stand.telemetry_kopplungen = stand.telemetry_kopplungen.saturating_add(1);
            }
            neu
        };
        …
    }
    fn telemetrie_getrennt(&self, link_id: &str) {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .telemetry_links
            .remove(link_id);
    }
```

`telemetry_links: HashSet<String>` (`zustand.rs:404`). Weil T1s Eintrag noch steht, liefert
`insert` für T2 `false`: T2 wird **nicht** als Kopplung gezählt (daher im Auditlauf
`coordinator_coupled_before: [1,1]`, `wire-run.txt:67`). T1s spätes `telemetrie_getrennt`
entfernt denselben Schlüssel ohne Besitzervergleich → `[0,1]`.

**Station 5 — T2s Frames fallen still.** `senke.rs:152-154` über `link.rs:635-639`:

```
    pub(super) fn aktueller_telemetrie_client_locked(stand: &Stand, link_id: &str) -> Option<ClientKey> {
        if !stand.telemetry_links.contains(link_id) {
            return None;
        }
```

Kein `messfehler`, kein `p2_reject`, keine Trennung. Der Transport selbst lässt T2s Frames
passieren, weil er den Besitzer kennt (`verbindung.rs:697-708`, `k.telemetrie_lebt(&link_id,
id)`, `bootstrap.rs:497-502`). Control bleibt unberührt, Heartbeats werden quittiert. Gen sieht
die Messung dieser Sonde als `missing`/`stale` (`phase-16-b-verbindungen.md:64`, F14), nicht
als frisch.

**Station 6 — Heilung nur durch eine weitere Neukopplung.** Erst ein T3 findet den Schlüssel
frei (`insert` → `true`) — gemessen `wire-run.txt:68` (`p2` 2, `couplings [1,2]`). Nichts löst T3
aus: T2 bekommt keinen Fehler.

**Ergänzung 1 — der Trennmelder hat dieselbe Lücke, am Control-Ende (hergeleitet).**
`trennung.rs:13-34`: `TrennStand { erwartet, gemeldet }` je Control-`link_id`, angelegt mit dem
Control-Welcome (`verbindung.rs:367`), entfernt am Control-Ende (`:885-888`). `gemeldet` wird nur
auf `true` gesetzt (`trennung.rs:96`, einzige Stelle), nie zurück. Nach **jeder** Neukopplung
unter lebendem Control — auch der geordneten, bei der T1 vollständig endet, bevor T2 koppelt —
steht `gemeldet` schon auf `true`. Endet später die Control-Verbindung, kehrt der Wartepunkt
sofort zurück (`:124` `while g.erwartet && !g.gemeldet`), und `control_getrennt` kann vor T2s
`telemetrie_getrennt` laufen, ohne dass `lebenszyklus_reihenfolge_verletzt` zählt. Heute ohne
Coordinatorwirkung, weil `control_ende` den Telemetrieschlüssel schon entfernt (`link.rs:570`);
die Zusage C-LS-06 ist trotzdem gebrochen.

**Ergänzung 2 — Reichweite.** Nötig sind (i) ein Telemetrieende bei lebendem Control und (ii) ein
T1-Verbraucher oder -Schreiber, der beim Schluss noch mindestens die 500 ms des Client-Backoffs
belegt ist. Gens eigene Telemetrie-Reconnects laufen stets mit einem **neuen** Control-Link und
damit neuem Schlüssel (`Ipc.cpp:886-901`, `:993`, `:1060`) und sind nicht betroffen. Der
P2-Senkenaufruf macht keine Store-I/O (`intervention.rs:669-673`); das Fenster entsteht nur aus
Sperrkonkurrenz oder einem hängenden Push. Häufigkeit im Host nicht gemessen (Audit
`phase-16-b-verbindungen.md:16` sagt dasselbe).

**Fristen.** 500 ms Backoff (Audit wartet 550 ms, `wire.rs:120`), bis 2000 ms je Join, P2-Frist
des Clients `kIoFristMs` 5000 ms (B-TC-10), Wachhund 100 ms (`server_v3/mod.rs:226`).
Kopplungszähler 1 → 0 (Menge), `telemetry_kopplungen` bleibt 1 (T2 nie gezählt),
`p2_live_frames` bleibt 1 bei vier P2-Aufrufen am Transport.

### c) Zusage wörtlich

- `docs/beweise/SONDE-010.md:237` (B-TC-10): „der Frame in Arbeit gilt als verloren — P2 ist per
  Vertrag verlusttolerant (§33.1); die Schleuse behält ihren Inhalt (`A-P2-07`); die Verbindung
  endet über `ioAbbrechen`, OHNE auf `kIoFristMs` (5000 ms) zu warten; der Reconnect läuft mit
  den AKTUELLEN Kopplungswerten".
- `SONDE-010.md:256` (C-LS-06): „**Solange jede Senke ihre Frist hält**, läuft je Kopplung
  jeder Callback GENAU EINMAL, in der Reihenfolge Kopplung lösen (VOR den fristbegrenzten Joins,
  mitsamt Abbruch der I/O der mitfallenden Telemetrieverbindung) → `telemetrie_getrennt` →
  `control_getrennt`. Die Control-Seite wartet dafür höchstens `SENKE_FRIST` (2000 ms) auf das
  `telemetrie_getrennt` der Telemetrieseite … kein Trenn-Callback entfällt und keiner läuft
  doppelt." (Anwendungsfall: „Verbindungsende der Control-Seite".)
- `SONDE-010.md:254` (C-LS-04): „Je `link_id` läuft `telemetrie_gekoppelt` HÖCHSTENS EINMAL:
  ein zweites Telemetry-Hello auf dieselbe `link_id` wird abgewiesen, ohne den Callback erneut
  auszulösen".
- `SONDE-010.md:231` (B-TC-04) und B10 `IpcTestMain.cpp:6166`, Fall a) `:6198-6210`: der Client
  koppelt nach einem Telemetrieschluss durch den Broker „binnen Frist wieder" — mit denselben
  Werten, gegen einen C++-Testserver.
- Entwurf `:4156`: „Damit koppelt `link_id + runtime_nonce` genau eine Control- und eine
  Telemetry-Verbindung."; `:2067`: „sein Ausfall degradiert Analyse, nicht Control."

**Widerspruch in der Matrix.** C-LS-04 verbietet wörtlich ein zweites `telemetrie_gekoppelt` je
`link_id`; B-TC-10, B-TC-04, der Transport (`bootstrap.rs:516-517`: „die Kopplung bleibt
bestehen …, nur der Telemetrieplatz wird frei") und `verbindung.rs:459` sehen die Neukopplung
unter derselben `link_id` ausdrücklich vor. Der Test zu C-LS-04 (`tests_kopplung.rs:149-187`)
misst nur ein **gleichzeitiges** zweites Hello. Der Coordinator folgt der wörtlichen C-LS-04-
Lesart (idempotentes `insert`), der Transport der B-TC-10-Lesart — T3-16-02 entsteht genau an
dieser Naht. C-LS-06 schweigt zum Telemetrieende bei lebendem Control; seine Regel „je Kopplung
jeder Callback genau einmal" ist nur sinnvoll, wenn „Kopplung" eine Telemetrie-Inkarnation ist.

**Irreführende Kommentare:** `tests_kopplung.rs:149-152` („je `link_id` … hoechstens ein
`telemetrie_gekoppelt`"); `trennung.rs:45-47` („Ein Melder je lebender Kopplung. Er wird mit
`control_verbunden` angelegt und mit `control_getrennt` entfernt") — er gilt je Control-Link,
über mehrere Kopplungen; `trennung.rs:31-33` (`gemeldet`: „Ihr `telemetrie_getrennt` ist
gemeldet") — nach einer Neukopplung beschreibt er die Vorgängerin.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Was sie nicht misst |
|---|---|---|
| A4 `coordinator_model.rs:262-273` `telemetrie_einmalig_gekoppelt` | zweimal `telemetrie_gekoppelt("link")`, einmal `telemetrie_getrennt("link")` → `(0, 1)` | **schreibt die `link_id`-Semantik als gewollt fest**: nach zwei Kopplungen und einem Ende ist nichts mehr gekoppelt — die Wirkung von T3-16-02 im Kleinen |
| A4 `tests_kopplung.rs:149-187` `connect_callbacks_je_kopplung_genau_einmal` | gleichzeitiges zweites Hello → abgewiesen, ein Callback | Neukopplung nach Entkopplung |
| A4 `tests_kopplung.rs:189-221` `trennreihenfolge_je_callback_genau_einmal` | eine Kopplung, Control-Ende, Reihenfolge | Control-Ende **nach** einer Neukopplung (Ergänzung 1) |
| A4 `tests_lebenszyklus.rs:369` `kopplung_faellt_mit_dem_leserende_nicht_erst_nach_den_joins` | Kopplung fällt vor den Joins | dass genau das den Platz für T2 öffnet |
| B10 `IpcTestMain.cpp:6198-6210` | C++-Client koppelt nach Telemetrieschluss wieder | gegen `TestServer`, nicht gegen den Rust-Coordinator |
| A22 `tools/beweise.ps1:637` | 32 Paare, Kopplung über `link_id + challenge + gleiche runtime_nonce` | `ZaehlSenke`, kein Coordinator, keine Neukopplung |

Behauptungszeile A4 (`tools/beweise.ps1:518`): keine Aussage zur Telemetrie-Neukopplung.

### e) Urteil: BESTÄTIGT

Alle sechs Stationen am HEAD gelesen; `verbindung.rs`, `trennung.rs`, `server_v3/senke.rs` und
die Kopplungsmethoden des Coordinators sind seit `aff2d818` unverändert, `bootstrap.rs` nur
verschoben. Ergänzt: (1) die Zusagenbasis ist ein Widerspruch zwischen C-LS-04 und B-TC-10, den
die Reparatur mit auflösen muss; (2) `telemetrie_einmalig_gekoppelt` schreibt das Fehlverhalten
fest; (3) dieselbe fehlende Generation im `TrennStand` bricht C-LS-06 am späteren Control-Ende
nach jeder Neukopplung (hergeleitet, nicht gemessen); (4) Reichweite eng (Ergänzung 2).

### f) Kategorie und Schwere

**DEFEKT** gegen C-LS-06 („je Kopplung jeder Callback GENAU EINMAL … keiner läuft doppelt" —
die Nachfolgerin erleidet zwei Trennwirkungen und keine Kopplungswirkung) zusammen mit B-TC-10
und Entwurf `:4156` (die bestätigte Nachfolgerin ist die eine gekoppelte Telemetrie). Der
erlaubte P2-Verlust (§33.1) deckt einzelne Frames, keinen dauerhaft stummen, per Welcome
bestätigten Nachfolger. **Schwere medium:** die Messwerte einer Sonde fehlen bis zum nächsten
Control-Neuaufbau, sichtbar nur als „stale"; enger Auslöser, keine falsche Messung.

### g) Kleinster Fix

**Ort:** Senke-Trait (`transport/server_v3/senke.rs:53-55`), Coordinator
`telemetrie_gekoppelt`/`telemetrie_getrennt` (`coordinator/senke.rs:58-78`), `Stand`
(`zustand.rs:404-405`), Trennmelder (`trennung.rs:13-99`), Aufrufe in `verbindung.rs:457-465`
und `:878-889`.

1. **Generation = Verbindungskennung.** Der Transport kennt sie schon: `id: u64` in
   `verbindung_bedienen` (`verbindung.rs:149`), gespeichert als `Kopplung.telemetrie_verbindung`
   (`bootstrap.rs:406`, `:489`), geprüft je Frame (`:497-502`). Sie reist in
   `telemetrie_gekoppelt(link_id, verbindung)` und `telemetrie_getrennt(link_id, verbindung)` mit.
2. **Coordinator:** `telemetry_links` wird eine Zuordnung `link_id → verbindung`. Koppeln setzt
   den neuen Besitzer (und zählt, und pusht die Liveframes); Trennen entfernt nur, wenn der
   gemeldete Besitzer der eingetragene ist. `control_ende` entfernt weiter nach `link_id`
   (`link.rs:570`), Eviction ebenso (`liveness.rs:154`).
3. **Trennmelder:** `gemeldet` gilt der aktuellen Telemetrie-Inkarnation — beim Koppeln
   (`trennmelder_telemetrie_erwartet`, `trennung.rs:60-70`) zurückgesetzt bzw. nur gesetzt, wenn
   die meldende Verbindung die aktuelle ist.
4. Matrix- und Testtexte wie unter „Reihenfolge".

**Schema:** keine Änderung — die Kennung verlässt den Broker nie; C++ bleibt unberührt.
**Mitzuprüfende Beziehungen:** koppeln↔entkoppeln je Inkarnation (T1 gehalten, T2 gekoppelt,
T1-Ende lässt T2 stehen; geordneter Reconnect wie heute; beide Trennreihenfolgen),
verbinden↔trennen (Control-Ende räumt alle eigenen Kinder ab, C-LS-06-Reihenfolge auch nach einer
Neukopplung). **Fristen, die bleiben:** Kopplung lösen vor den Joins (`verbindung.rs:828-836`),
`SENKE_FRIST` 2000 ms je Join, begrenzter Stop, 500-ms-Backoff, Warten der Control-Seite
höchstens `SENKE_FRIST`. **Audio-Thread:** nicht berührt. **Pipe-Latenz:** ein
Zuordnungsvergleich je Lebenszyklusereignis, nicht je Frame. **NAK-310:** `bootstrap.rs` wird
nicht geändert (die Kennung liegt dort schon); `verbindung.rs` wird im Lebenszyklusteil
(`:453-468`, `:866-889`) berührt, NAK-310 zitiert dieselbe Datei im Bootstraptor
(`docs/beweise/NAK-310.md:1182`, `:1190`: `verbindung.rs:284-299`, `:343-359`) — Nachbarschaft
genannt, nicht bewertet. Alle Testsenken implementieren den Trait mit
(`tests_hilfe.rs:430-447`, `tests_kopplung.rs:76-106`, `tests_lebenszyklus.rs:289-304`,
`tests_rueckstau.rs:157-178`, `bin/eqcop-store-crash-worker.rs`, `ZaehlSenke`).

### h) Ort des Rotbeweises

**Bein A4**, `broker/src/transport/server_v3/` neben `tests_kopplung.rs:189-221`, mit echtem
Listener auf einem Probe-Pipenamen und dem echten Coordinator hinter einer Senke, die die
Rückkehr des ersten P2 hält (Muster `ReihenfolgeSenke` `blockiert_in`,
`tests_kopplung.rs:76-106`; Audit-Harnisch `wire.rs:108-140`). **Deterministisch statt mit
550 ms:** T1 koppeln, P2 senden und am Haltepunkt festhalten, T1 schließen, auf die
Entkopplung warten (Registerzustand), T2 koppeln und Welcome lesen, erst dann T1 freigeben.
Assertionen: `telemetrie_kopplungen().0 == 1` nach T1s Ende, drei T2-Frames erhöhen
`p2_live_frames` von 1 auf 4, T2 bleibt offen, Control quittiert — **heute rot** (0, bleibt 1).
Gegenfälle: geordneter Reconnect (T1 endet ganz, dann T2) wie heute grün; Control-Ende nach einer
Neukopplung meldet `telemetrie_getrennt` vor `control_getrennt` (Ergänzung 1, heute
unbestimmt). `telemetrie_einmalig_gekoppelt` (`coordinator_model.rs:262-273`) wird zu „zwei
Inkarnationen, Ende der ersten lässt die zweite stehen" umgeschrieben. Mutation an der Zeile, die
die Zusage trägt: den Besitzervergleich in `telemetrie_getrennt` entfernen → der erste Fall
reißt; das Zurücksetzen von `gemeldet` entfernen → der Control-Ende-Fall reißt.

**„T2 nach echtem Backoff"** mit einem echten C++-`TelemetryClient` (500 ms aus
`IpcVerbindung.h:34`) läuft nur in **A4-SI** über einen neuen Modus des B10-Binaries gegen den
Phase-B-Server; für den Beweis der Zusage ist er nicht nötig, weil die Reihenfolge im A4-Fall
erzwungen ist. **A22** trägt ihn nicht (kein Coordinator).

---

## Gemeinsame Ursachen

**Wurzel A — abgeleiteter Zustand wird wie persistenzpflichtige Annahme behandelt.** T3-15-12
allein: `flush_session` unterscheidet nicht, ob ein Snapshot eine Befehlswirkung persistiert
(nur nach `session_command`, `befehl.rs:616`) oder einen abgeleiteten Stand liefert, und P2 hängt
am Routingschalter statt an der Storegesundheit. Eigener Änderungssatz in `flush.rs`,
`coordinator/senke.rs`, `subscription.rs` mit A4/A4-SI. **Berührung NAK-125**
(`docs/offene-punkte.md:161`, S29–31): derselbe Flush nach `session_command` quittiert heute
bedingungslos `angewandt`; der T3-15-12-Satz lässt diesen Flush persistenzpflichtig und die
ACK-Frage bei NAK-125, darf sie aber nicht verschärfen.

**Wurzel B — ein Schalter ohne Grund und ohne Rückweg.** T3-15-12 (Startvariante) und T3-15-13
teilen `routing_bereit`: drei Setzer (`zustand.rs:624`, `mod.rs:316`, `flush.rs:33`), fünf
Gründe (degradierter Start, Guard-Persistenz `flush.rs:242`/`link.rs:329`, Projektionslesefehler
`subscription.rs:153`, Projektionsvertrag `:163`/`:176`/`:215`/`:228`), kein gehaltener Grund,
kein Setzer zurück, und der einzige Drahtausdruck ist das Duplikatflag. **Reihenfolge der
Sätze:** T3-15-12 zuerst — erst danach landet der Dauerfall überhaupt als `storeDegraded`, auf den
T3-15-13 sich stützt; beide ändern `alias_quarantaenisiert` (`flush.rs:303`) — diese Zeile
gehört in den ersten Satz, der zweite baut darauf auf.

**Wurzel C — Besitz nur nach Control-`link_id`.** T3-16-02 allein, auf drei Ebenen: Coordinator
(`telemetry_links`), Trennmelder (`TrennStand`) und Senke-Trait. Unabhängig vom Store; eigener
Satz (W16: „Unabhängig von Store W15", `BEFUNDE.md:819`).

**Nur die Datei geteilt:** T3-15-12 und T3-16-02 berühren beide `coordinator/senke.rs`, aber
verschiedene Funktionen (`p2` gegen `telemetrie_gekoppelt`/`telemetrie_getrennt`).

**Berührung mit Teil A (T3-08-01, W13/W14).** Derselbe Writer (`store/writer.rs:325-353`,
`append_gruppe`) und derselbe `flush_session`. T3-15-12 ändert den Fehlerzweig und die Zielliste
des Flushs, nicht den Writer; T3-08-01 teilt Gruppen am 64er-Rand, T3-08-02/03 ordnen Commit und
Publikation im Flush. Getrennte Sätze, aber nacheinander in `flush.rs`. Gemeinsame Grenze aus dem
Gate: „ACK erst nach vollständigem Commit" — der Livestand bei degradiertem Store ist kein ACK und
darf weder Ordinal noch Schuldkompaktierung tragen.

**Berührung mit Teil C (T3-14-03).** T3-14-03 setzt beim Control-Ende `taint.unknown = true` für
die Sitzung (`link.rs:589-593`), getragen von C-08 (`SONDE-011.md:514`: „Overflow/Lücke/
Disconnect setzen sticky unknown"); das Ende räumt dort nur die eigenen Interventionen ab
(`:590-592`, `intervention.link_id != link_id`). T3-16-02 entkoppelt beim Telemetrieende fremden
Besitz. Die gemeinsame Lebenslaufregel ist **„ein Ende wirkt nur auf das, was seine Inkarnation
besitzt"** — für Control-Links gilt sie schon (eigene `link_id` je Verbindung, `verbindung.rs:302`;
C-10/L-03 Verdrängung nach Nonce), für Telemetrie fehlt die Inkarnation. T3-14-03 ist dagegen
eine Frage nach dem **Umfang** der C-08-Wirkung, nicht nach Besitz — verschiedene Wurzeln,
getrennte Sätze.

**Berührung mit NAK-121.** H-01 (`NAK-121.md:215`: „Registereintrag und Handle haben **einen**
Besitzer") ist das Vorbild für Wurzel C; keine NAK-121-Zeile wird von den Fixes verletzt oder
muss sich ändern.

**Irreführende Texte, die in die Sätze gehören:** Satz A — `subscription.rs:194-197`,
`liveness.rs:240-241`, `docs/plugin-wissen.md:2273-2275`, Testname
`store_kanal_cap_und_naechster_trennt` (`store_crash_matrix.rs:1515`); Satz B — `flush.rs:31`
(`_grund`), Testname `…_sichtbar_fail_closed` (`:1157`), Schema-`$comment`
`eq-ipc-v3.schema.json:950` gegen `flush.rs:303`; Satz C — `tests_kopplung.rs:149-152`,
`trennung.rs:31-33`, `:45-47`.

---

## Regelfragen

### 1. Livestand ohne Commit (T3-15-12)

**Abgeleitet** sind die `session_snapshot`-Flushs, deren Wirkung schon anderswo persistiert ist
oder laufgebunden bleibt: Heartbeat (`liveness.rs:535-537`), `descriptor_setzen` (`:936`), Tick
(`:311-313`), Verdrängungspush (`coordinator/senke.rs:54-56`), nach Experimentterminal
(`experiment_verdrahtung.rs:365`), Hypothese, Intent und Invalidierung
(`hypothese_verdrahtung.rs:742`, `intent.rs:684`, `invalidierung_verdrahtung.rs:452`) und der
Resubscribe (`subscription.rs:84-252`). Der Vertrag nennt den Snapshot „Fluechtiger
Brokerzustand … rekonstruierbar" (`eq-ipc-v3.schema.json:1071`, Entwurf `:2172`); O-04
(`SONDE-011.md:563`): „Eventlog und Projektion bleiben haltbar; Snapshotbytes werden daraus
rekonstruiert."; L-13 (`:634`): „`broker_epoch`, `fuehrendes_main` und `mitglieder` bleiben
rein laufgebunden". **Persistenzpflichtig** sind die P0-Befehle mit `command_id` (O-01) —
`persistenz_p0` (`befehl.rs:212-215`, `:439-471`) und `session_command`, dessen einzige
Persistenz der Flush `befehl.rs:616` ist (1b Ergänzung 3, Register NAK-125; die
Beitrittswirkung setzt `befehl.rs:571`). Dieselbe Wirkung tragen die öffentlichen
Mitgliedschaftsfunktionen `mitgliedschaft.rs:95-117`, `:139-161`, `:180-211` (Flush `:115`,
`:159`, `:209`); sie haben in `broker/src` außerhalb der Tests keinen Aufrufer, wären bei
Gebrauch aber ebenso persistenzpflichtig —, Evidenz-,
Intent-, Assistent- und Invalidierungsereignisse (`liveness.rs:513-534`, `befehl.rs:875-899`,
`evidenz.rs:481`, `intent.rs:758`) und die Konfliktriegel (C-07, `flush.rs:235-245`).

**Zusagen** (wörtlich in 1c): S-03 und L-08 begrenzen Verweigerung und Verursachertrennung auf
„persistenzpflichtige Ereignisse" und sagen „sichtbarer gezählter Betrieb ohne Historie, P2 läuft
weiter"; S-01 „Routing bleibt bis zum Konfliktriegel-Restore fail-closed, P2 darf weiterlaufen";
O-01 „P2 bleibt aktiv".

**Setzer von `routing_bereit`:** 1b Station 8. **Diagnose bis Gen heute:** `store_degraded` nur im
Snapshot (`sicht.rs:657-658`), zugestellt nur über den Resubscribe eines nicht getrennten Links
— den der erste Heartbeat regelmäßig schon getrennt hat; der Heartbeat-ACK trägt nichts (Schema
`:942-955`), das Plugin liest ihn nicht. **Darf ein gesunder Store trennen?** Ja, aber nur für
persistenzpflichtige Ereignisse: S-03/L-08 („Bei vollem Kanal … wird die Annahme
persistenzpflichtiger Ereignisse verweigert und der Verursacher getrennt"). Heute trennt er auch
bei abgeleiteten Flushs (1b Ergänzung 1) — das verlangt keine Zusage.

**Technik oder Produkt: Technik.** Die Quellen legen das Ergebnis fest: der Heartbeat ist kein
persistenzpflichtiges Ereignis, der Snapshot ist abgeleitet, und bei Storefehler ist Betrieb ohne
Historie mit P2 ausdrücklich zugesagt. Der Livestand ohne Commit ist schon heute der Weg des
Resubscribe (NAK-246 D5, `subscription.rs:184-209`). Eine Wahl zwischen „Clients kreisen" und
„Betrieb ohne Historie" hat der User nicht — die zweite ist beschlossen. **Kein Kartenentwurf.**
Regel für den Dirigenten: *Ein abgeleiteter Snapshotflush trennt nie; bei degradiertem Store geht
er als Livestand ohne Commit mit `store_degraded` an die Abonnenten; P2 und Liveframe-Push laufen
bei degradiertem Store trotz gesperrtem Routing; nur persistenzpflichtige Ereignisse — darunter
der Flush, der die Wirkung eines `session_command` persistiert — werden bei vollem Kanal oder
degradiertem Store verweigert und trennen ihren Verursacher.* Die ACK-Frage dieses einen Flushs
bleibt NAK-125.

### 2. Ehrliche Diagnose bei fail-closed Routing mit gesundem Store (T3-15-13)

**Heute in Gen:** „Authenticating" ohne Handgriff, weil `setzeControlTransport` „verbunden, aber
kein Snapshot" als `authenticating` deutet (`SourcesModel.cpp:682-685`) und der Broker in diesem
Zustand weder Snapshot noch Diagnose liefert (2b Stationen 3–5). **Darf ein erneutes Lesen den
Wiederanlauf tragen?** Die Quellen verbieten nur zweierlei: den leeren oder erfundenen Snapshot
(SONDE-011:1061 „kein leerer Fallback") und die zeitbasierte Freigabe sowie Routing vor
erfolgreichem Guard-Restore (C-07). Ein erneutes Lesen **derselben** Projektion, ausgelöst von
einem neuen Subscribe und nur bei Erfolg und Vertragsgültigkeit freigebend, liefert keinen
erfundenen Snapshot, ist nicht zeitbasiert und berührt die Konfliktriegel nicht — die sind bei
gesundem Store beim Start restauriert (`mod.rs:317-319`). Voraussetzung ist die Trennung der
Sperrgründe (2g Punkt 1): ein Guard-Persistenzfehler (`flush.rs:242`, `link.rs:329`) darf nie über
einen Lesevorgang freigegeben werden. W15 verlangt dafür einen „belegten Restoreentscheid"
(`BEFUNDE.md:818`) — das ist eine Regel des Dirigenten mit Rotbeweis, kein Userentscheid.

**Technik oder Produkt: Technik.** Die ehrliche Zustandsmeldung verlangen `CLAUDE.md:44-45` und
SONDE-012 L15 schon; sie ist ein Diagnosezustand im bestehenden Katalog, keine Oberfläche. Beim
Wiederanlauf hätte der User nur die Wahl zwischen „Nakama nimmt die Arbeit wieder auf, sobald die
Datenbank wieder lesbar ist" und „Nakama bleibt gesperrt, bis alle FL-Fenster zu sind" — ohne
Sicherheitsgewinn für die zweite, weil die Sperre in beiden Fällen hält, solange das Lesen
scheitert. Das ist keine echte Wahl. **Kein Kartenentwurf.** Regel für den Dirigenten: *Der
Broker hält den Grund einer Routingsperre und meldet ihn im Heartbeat-ACK; Gen zeigt ihn als
eigenen Diagnosezustand statt „Authenticating"; eine Sperre wegen Projektionslesefehler wird nur
durch ein erfolgreiches, vertragsgültiges Lesen bei einem neuen Subscribe aufgehoben, eine
Guard-Persistenzsperre nie.* Der Wiederanlauf ist optional für S25m; ohne ihn bleibt die
Diagnose ohne Handgriff.

### 3. Telemetriegeneration (T3-16-02)

**Eigentümer heute:** Transport — `Kopplungen.nach_link` nach Control-`link_id`, mit Challenge,
`runtime_nonce` und der Telemetrie-Verbindungskennung (`bootstrap.rs:397-407`, `:420-423`), die
je Frame geprüft wird (`verbindung.rs:697-708`); Trennmelder — nach Control-`link_id`, ohne
Generation (`trennung.rs:13-34`, `:48`); Coordinator — `HashSet` der Control-`link_id`s
(`zustand.rs:404`), Senke-Callbacks nur mit `link_id` (`server_v3/senke.rs:53-55`). Die
`runtime_nonce` ist für T1 und T2 dieselbe (sie gehört der Plugininstanz, `bootstrap.rs:482`) und
taugt nicht als Generation; die Challenge ebenso.

**Zusagen wörtlich:** B-TC-10 und C-LS-06 in 3c; dazu C-LS-04 und der Widerspruch.

**Generation ohne v3-Vertragsänderung:** ja. Die Verbindungskennung `id: u64` ist rein
brokerintern, entsteht beim Annehmen, liegt schon im Kopplungsregister und muss nur durch den
internen Senke-Trait, den Coordinator und den Trennmelder gereicht werden. Kein Wirefeld, keine
Fixture, keine C++-Änderung.

**Fristen, die bleiben müssen:** Kopplung lösen vor den Joins (C-LS-06, `verbindung.rs:828-836`,
`plugin-wissen.md:2198-2205`), `SENKE_FRIST = 2000 ms` je Join und als Höchstwartezeit der
Control-Seite (`server_v3/mod.rs:221`, `trennung.rs:101-138`), begrenzter Stop (C-LS-03,
B-TC-07), Client-Backoff 500 ms bis 8000 ms (`IpcVerbindung.h:34-35`), Abbruch ohne
`kIoFristMs` (B-TC-10), Wachhund 100 ms. Die Alternative „alten Platz bis nach T1s Meldung
halten" würde T2 bis zu 2 s und mehr mit `KopplungSchonBelegt` abweisen, den Client-Backoff
verdoppeln und die Messlücke verlängern; sie widerspricht dem Entscheid „Die Kopplung fällt mit
dem Leserende" (`plugin-wissen.md:2198`).

**Technik oder Produkt: Technik.** Für den User gibt es nur ein sinnvolles Verhalten: eine neu
verbundene Messleitung liefert. **Kein Kartenentwurf.** Regel für den Dirigenten: *Je Control-
`link_id` höchstens eine Telemetrie gleichzeitig; jede Kopplung ist eine eigene Inkarnation mit
der Verbindungskennung als Besitzer; ihre Callbacks laufen genau einmal und wirken nur auf sie;
ein Ende wirkt nie auf eine Nachfolgerin.* Daraus folgt die Umschrift von C-LS-04.

---

## Reihenfolge

Maßstab ist die Arbeitsübersicht (`BEFUNDE.md:818-819`): W16 ist unabhängig von W15; in W15
kommt der Weiterbetrieb vor der Anzeige.

**Etappe 1 — T3-16-02 (Wurzel C).** Klein, unabhängig, keine Schemaänderung. Senke-Trait mit
Verbindungskennung, Coordinator-Besitz, Trennmelder, Testsenken, Umschrift
`telemetrie_einmalig_gekoppelt`; Bein A4 (optional A4-SI mit echtem C++-Client). Kann parallel zu
Teil A laufen, weil es `flush.rs` nicht berührt.

**Etappe 2 — T3-15-12 (Wurzel A, erster Teil von B).** Nach den `flush_session`-Änderungen aus
Teil A (W13/W14), weil dieselbe Funktion; Regel aus Regelfrage 1 vorher festschreiben.
Abgeleiteter Flush, P2-Tor, Liveframe-Push, Duplikatflag, Umschrift des A4-Beins `:1558-1576`;
Beine A4, A4-SI, B13.

**Etappe 3 — T3-15-13 (Rest von Wurzel B).** Nach Etappe 2, weil erst dann der Dauerfall ehrlich
als `storeDegraded` landet und beide die Gen-Diagnose und den Heartbeat-ACK berühren. Grund halten,
Heartbeat-ACK-Feld mit Minorschritt, Gen-Diagnose; optional Wiederanlauf nach Regelfrage 2;
Beine A4, B13, A5/A8 (Vertragsvektoren des neuen Feldes), B10 (C++-Leser).

**Matrixzeilen, die nachgezogen werden müssen:**

- SONDE-011 **S-03** (`:535`) und **L-08** (`:629`): „persistenzpflichtige Ereignisse" gegen
  abgeleitete Snapshots abgrenzen; der Belegsatz S-03 (`:682`) nennt heute einen Heartbeattest
  als Verursacherbeleg und einen *blockierten* statt degradierten Writer als P2-Beleg.
- SONDE-011 **S-01** (`:533`) und **O-01** (`:560`): „P2 darf weiterlaufen"/„P2 bleibt aktiv"
  mit dem neuen P2-Tor belegen; **C-07** (`:513`): klarstellen, dass „Routing" Dispatch,
  Befehle und Beitritt meint und P2 bei Storefehler nicht einschließt.
- SONDE-011 **Erstprüfungsdefekt 4** (`:1061`): Grund der Sperre, Diagnose und gegebenenfalls
  Wiederanlauf; der Testname „sichtbar" wird gemessen oder geändert.
- SONDE-012 **L15** (`:155`) und **E-L15-Store** (`:485-488`): neuer Diagnosezustand, neues
  optionales Feld im Heartbeat-ACK.
- SONDE-010 **C-LS-04** (`:254`): „höchstens eine gleichzeitig je `link_id`; jede Kopplung eine
  Inkarnation"; **C-LS-06** (`:256`): „je Kopplung" als Telemetrie-Inkarnation, auch nach einer
  Neukopplung, dazu der Fall „Telemetrieende bei lebendem Control"; **B-TC-10** (`:237`): der
  Reconnect unter lebendem Control liefert wieder an den Coordinator.
- NAK-121: keine Zeile muss sich ändern (H-01 ist Vorbild).
- `docs/plugin-wissen.md:2273-2275` und der Abschnitt zur Kopplung (`:2198-2205`) ziehen mit.

---

## Nicht geprüft

- **Kein Lauf.** Bau, cargo, Test, Kanon, Skript, FL und Fehlerinjektion waren verboten. Alle
  Aussagen sind Quellenlektüre; die Zahlen aus Phase 16 (`components-run.txt`, `wire-run.txt`)
  habe ich gelesen, nicht nachgefahren.
- **Ergänzung 1 zu T3-16-02** (C-LS-06 am Control-Ende nach einer Neukopplung) und **Ergänzung 1
  zu T3-15-12** (voller Kanal bei gesundem Store trennt Heartbeat-Absender) sind hergeleitet, nicht
  gemessen.
- **Häufigkeit der Auslöser:** wie oft ein Leser `SQLITE_BUSY` über 2000 ms sieht, ohne dass der
  Writer degradiert, und wie oft eine Telemetrie bei lebendem Control endet, während ein
  Verbraucher über 500 ms belegt ist — aus dem Repo nicht bestimmbar.
- **Der Rennausgang P0 gegen P1 im Broker** (Heartbeat vor Subscribe auf getrennten Verbrauchern)
  ist aus der Sendereihenfolge des Clients hergeleitet; der seltene umgekehrte Ausgang ändert das
  Urteil nicht (2b Station 4, 1b Station 10).
- **Probeeq-Anzeige:** die kompakte Status- und Rückfallfläche der Probeeq im Degradationsfall
  habe ich nicht gelesen; die Befunde betreffen Gens Diagnose.
- **Erholung nach Laufzeitdegradation** (die Degradation ist einseitig bis zum Brokerende): von
  keiner Quelle verlangt, nicht bewertet und nicht Teil der vorgeschlagenen Fixes.
- **Auto-Join und Beitritt im degradierten Start:** gelesen nur so weit, dass eine ungebundene
  Probe ihre Main-Sitzung beim Hello ohne `routing_bereit` erhält (`link.rs:91-97`) und ihre P2
  damit die Main-Abonnements erreicht; den vollständigen Beitrittsweg ohne Routing habe ich nicht
  verfolgt.
- **Sicherheitsseite:** Pipe-Autorisierung, Identitätsbindung und das Bootstrap-Tor (NAK-310)
  sind nicht bewertet; Berührungen stehen je Abschnitt unter g).
- **`docs/beweise/NAK-314.md`** entstand während meines Laufs als untracked Datei; ich habe sie
  weder gelesen noch angefasst.
- **Dokuriegel nicht gelaufen**; diese Datei ist ein Rohbeleg unter `docs/beweise/roh/`, kein
  Plandokument.

---

FERTIG Teil B, 2 bestätigt, 1 präzisiert, 0 widerlegt.
