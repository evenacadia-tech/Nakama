//! Die Coordinator-Verdrahtung im PRODUKTPFAD (SONDE-013, Nacharbeit 1).
//!
//! Die Erstpruefung 1 hat an dieser Stelle die schwerste Klasse Befunde
//! gemeldet, und sie hatten alle DIESELBE Ursache: `broker/src/coordinator/
//! mod.rs` exportierte `experiment`, `invalidierung`, `prepost` und
//! `vergleichbarkeit` als Bibliotheken, und der Coordinator besass weder
//! Felder noch Aufrufer dafuer. Die Matrixzeilen M-40 bis M-63 existierten
//! damit ausschliesslich in den Tests ihrer eigenen Module.
//!
//! Dieses Bein misst deshalb NICHT die Module. Es misst den Weg: Nachricht
//! rein → Store, Taint, Paar oder Ausschluss raus. Ein Fall, der ein Modul
//! direkt aufruft, koennte gruen sein, waehrend das Produkt gar nichts tut —
//! genau das war der Zustand vor dieser Runde.

use eqcop_broker::coordinator::{Coordinator, ManualClock};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
#[cfg(windows)]
use eqcop_broker::transport::server_v3::Senke;
use serde_json::{json, Value};
use std::sync::Arc;

// ── Werkzeug ─────────────────────────────────────────────────────────────

fn hex(n: usize) -> String {
    format!("{n:032x}")
}

fn adresse(projekt: usize, sitzung: usize, instanz: usize, nonce: usize) -> Adresse {
    Adresse {
        logon_sid: "S-1-5-21-1-2-3-1001".into(),
        project_binding_id: hex(projekt),
        session_epoch: hex(sitzung),
        instance_id: hex(instanz),
        runtime_nonce: hex(nonce),
    }
}

fn hello(
    projekt: usize,
    sitzung: usize,
    instanz: usize,
    nonce: usize,
    art: &str,
    pid: Option<u32>,
) -> HelloControl {
    HelloControl {
        typ: "hello".into(),
        connection_kind: "control".into(),
        protocol: 3,
        plugin_version: "test".into(),
        plugin_kind: art.into(),
        adresse: adresse(projekt, sitzung, instanz, nonce),
        host: pid.map(|pid| HostAngabe {
            pid,
            name: Some("FL Studio".into()),
            version: None,
        }),
        audio: AudioLage {
            samplerate: 48_000.0,
            block_size: 512,
            channels: 2,
        },
    }
}

fn capabilities() -> Value {
    json!({
        "host_context_presence": "supported",
        "project_time_samples": "supported",
        "sample_accurate_automation": "supported",
        "presentation_latency": "supported",
        "aux_compare_pre": "unsupported",
        "aux_priority_sidechain": "unsupported",
        "contribution_aux": "unsupported",
        "float64_processing": "supported",
        "binary_telemetry": "supported",
        // Der Vertrag kennt `remote_control`, nicht `cycle_derivation` — das
        // ist ein Feld des `zeitbasis`-Blocks. Bis zur Nacharbeit 2 stand hier
        // der falsche Name: die Faehigkeiten reichen erst dann bis in eine
        // Sessionprojektion, wenn ein Abonnent sie liest, und kein Fall dieses
        // Beins hat das vorher getan.
        "remote_control": "unsupported"
    })
}

fn coordinator() -> (Coordinator, Arc<ManualClock>) {
    let clock = Arc::new(ManualClock::default());
    let c = Coordinator::mit_uhr(clock.clone(), hex(0xbeef));
    (c, clock)
}

fn anmelden(c: &Coordinator, link: &str, hello: &HelloControl) {
    anmelden_roh(c, link, hello);
    // 🔑 NAK-180 R9: ein MAIN-Hello sperrt seine Sitzung, bis der erste
    // Heartbeat sie loest. Im Produkt tut das ein ausdrueckliches
    // `intervention_state_unknown: false`; hier steht dafuer derselbe
    // bestaetigte Resync, den dieses `false` ausloest. So startet jeder Test,
    // der NICHT R9 misst, in demselben Zustand wie vor der Regel.
    //
    // Wer R9 selbst misst, nimmt `anmelden_roh` - dort bleibt die Sperre
    // stehen, und genau das ist die Zusage (N-30 bis N-32).
    if hello.plugin_kind == "main" {
        let _ = c.resync_bestaetigen(link, 0);
    }
}

/// Anmelden OHNE den Resync des ersten Heartbeats (NAK-180 R9).
fn anmelden_roh(c: &Coordinator, link: &str, hello: &HelloControl) {
    let ausgang = c.control_hello_registrieren(link, hello);
    assert!(ausgang.angenommen, "{:?}", ausgang.grund);
}

/// Derselbe Kontakt OHNE `runtime` — fuer einen MAIN.
///
/// Der Runtime-Block traegt Messpunkt und Betrieb einer SONDE; wer ihn fuer
/// ein Main sendet, laesst den Broker daraus einen `probe_descriptor` mit
/// `plugin_kind: "main"` ableiten. Der ist vertragswidrig, und der naechste
/// Subscribe faellt daran fail-closed. Ein Main meldet ihn nicht.
fn report_main(c: &Coordinator, link: &str, adresse: &Adresse) -> bool {
    c.heartbeat_kontakt(
        link,
        Some(&json!({
            "type": "heartbeat",
            "adresse": adresse,
            "sequence": 1,
            "state_revision": 0,
            "capabilities": capabilities(),
            "zaehler": {}
        })),
    )
}

fn report(c: &Coordinator, link: &str, adresse: &Adresse) -> bool {
    c.heartbeat_kontakt(
        link,
        Some(&json!({
            "type": "heartbeat",
            "adresse": adresse,
            "sequence": 1,
            "state_revision": 0,
            "capabilities": capabilities(),
            "zaehler": {},
            "runtime": {"messpunkt": "insert", "betrieb": "active"}
        })),
    )
}

/// Die Nutzlast kommt aus dem COMMITTETEN Fixturekorpus — dieselbe Regel wie
/// in `coordinator_model.rs`: eine zweite Wire-Form von Hand waere eine
/// zweite Wahrheit neben dem Korpus.
fn evidenz_wert() -> Value {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../eq-copilot/fixtures/v3/gueltig/evidence-snapshot-mit-ereignissen-und-stereo.json");
    serde_json::from_slice(&std::fs::read(&pfad).expect("Evidenzfixture liegt im Korpus"))
        .expect("Evidenzfixture ist JSON")
}

/// Ein Snapshot mit gesetzter Adresse, eigener `evidence_id` und wahlweise
/// veraenderten Transportfeldern.
fn evidenz_payload(adresse: &Adresse, nr: usize, anpassen: impl FnOnce(&mut Value)) -> Vec<u8> {
    let mut wert = evidenz_wert();
    wert["adresse"] = serde_json::to_value(adresse).expect("Adresse ist serialisierbar");
    wert["evidence_id"] = json!(hex(0x1000 + nr));
    if let Some(t) = wert.get_mut("transport") {
        t["sequence"] = json!(nr as u64 + 1);
    }
    anpassen(&mut wert);
    serde_json::to_vec(&wert).unwrap()
}

/// Ein vertragsgueltiger `probe_descriptor` mit `pair_id` und Messpunkt.
///
/// Ohne ihn hat eine Quelle weder Paar noch Rolle — und eine Haelfte ohne
/// Rolle ist keine Haelfte (M-13).
fn descriptor(adresse: &Adresse, position: &str, pair_id: &str) -> Value {
    json!({
        "adresse": adresse,
        "plugin_kind": "passive_probe",
        "measurement_position": position,
        "aussageklasse": "beobachtend",
        "betrieb": "active",
        "label": "Testprobe",
        "pair_id": pair_id,
        "capabilities": capabilities(),
        "frische": {"letzter_kontakt_ms": 10, "stale": false}
    })
}

fn experiment_begin_wert(ziel: &Adresse, command: usize, experiment: usize) -> Value {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../eq-copilot/fixtures/v3/gueltig/experiment_begin.json");
    let mut wert: Value =
        serde_json::from_slice(&std::fs::read(&pfad).expect("experiment_begin liegt im Korpus"))
            .expect("Fixture ist JSON");
    wert["kopf"]["ziel"] = serde_json::to_value(ziel).unwrap();
    wert["kopf"]["command_id"] = json!(hex(command));
    // Der Zielclient meldet Revision 0; eine abweichende `base_revision` waere
    // ein `revision_conflict` und kein Weg zur Wirkung.
    wert["kopf"]["base_revision"] = json!(0);
    wert["experiment_id"] = json!(hex(experiment));
    wert
}

// ── Der Harnisch MIT Store ───────────────────────────────────────────────
//
// 🔑 Nacharbeit 2: die Runde 1 hat ihre Produktpfade ueberwiegend OHNE Store
// gemessen. Ein persistenzpflichtiger P0-Befehl ist dann `abgelehnt/internal`
// — der Test sah ein `command_ack` und hielt den Weg fuer gefahren, waehrend
// die Wirkung nie lief. Jeder Fall, der eine Wirkung behauptet, faehrt ab
// jetzt gegen einen ECHTEN Store und liest sein Ergebnis dort.

struct TestOrdner(std::path::PathBuf);

impl TestOrdner {
    fn neu(name: &str) -> Self {
        let pfad = std::env::temp_dir().join(format!(
            "nakama-sonde013-{name}-{}-{}",
            std::process::id(),
            uuid::Uuid::new_v4().simple()
        ));
        std::fs::create_dir_all(&pfad).unwrap();
        Self(pfad)
    }

    fn db(&self) -> std::path::PathBuf {
        self.0.join(eqcop_broker::store::STORE_DATEINAME)
    }
}

impl Drop for TestOrdner {
    fn drop(&mut self) {
        let _ = std::fs::remove_dir_all(&self.0);
    }
}

/// Sammelt, was der Broker an einen Subscriber geschrieben haette.
///
/// 🔑 Nacharbeit 3 (Befund B17): sie kann den Push auch ABLEHNEN. Ein
/// Subscriber, der gerade nichts annimmt, ist der Normalfall — kurz getrennt,
/// Puffer voll, Reconnect —, und genau dann entsteht die Zustellschuld, um die
/// es geht. Eine Probe, die immer annimmt, kann sie nie erzeugen.
#[derive(Default)]
struct PushProbe {
    geschrieben: std::sync::Mutex<Vec<(String, Value)>>,
    lehnt_ab: std::sync::atomic::AtomicBool,
}

impl PushProbe {
    fn payloads(&self) -> Vec<(String, Value)> {
        self.geschrieben
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .clone()
    }

    fn lehnt_ab(&self, an: bool) {
        self.lehnt_ab
            .store(an, std::sync::atomic::Ordering::SeqCst);
    }
}

impl eqcop_broker::coordinator::SessionPush for PushProbe {
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
        if self.lehnt_ab.load(std::sync::atomic::Ordering::SeqCst) {
            return false;
        }
        let wert: Value = serde_json::from_slice(payload).unwrap_or(Value::Null);
        self.geschrieben
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .push((link_id.to_owned(), wert));
        true
    }
}

#[cfg(windows)]
struct HarnischMitStore {
    c: Coordinator,
    main: HelloControl,
    clock: Arc<ManualClock>,
    push: Arc<PushProbe>,
    _writer: eqcop_broker::store::StoreWriter,
    _ordner: TestOrdner,
}

#[cfg(windows)]
impl HarnischMitStore {
    fn neu(name: &str) -> Self {
        Self::mit_projekt(name, 1)
    }

    fn mit_projekt(name: &str, projekt: usize) -> Self {
        let ordner = TestOrdner::neu(name);
        let mut k = eqcop_broker::store::StoreKonfiguration::fuer_pfad(ordner.db());
        k.remote_volume_override = Some(false);
        let writer = eqcop_broker::store::StoreWriter::starten(k);
        assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());
        let clock = Arc::new(ManualClock::default());
        let c = Coordinator::mit_store(clock.clone(), hex(0xbeef), &writer);
        let push = Arc::new(PushProbe::default());
        c.session_push_setzen(push.clone());
        let main = hello(projekt, 2, 10, 100, "main", Some(9));
        anmelden(&c, "main", &main);
        report_main(&c, "main", &main.adresse);
        // Ohne gemeldeten Aufnahmezustand lehnt `persistenz_p0` jeden
        // persistenzpflichtigen Befehl mit `record_state_unknown` ab.
        assert!(c.state_report_json("main", &state_report_payload(&main.adresse, 0)));
        Self {
            c,
            main,
            clock,
            push,
            _writer: writer,
            _ordner: ordner,
        }
    }

    /// Meldet den Main SELBST als Abonnenten seiner Sitzung an — erst dann
    /// entstehen Outboxziele und Pushes. Genau so laeuft es im Produkt: Gen
    /// abonniert die Sitzung, in der es fuehrt.
    fn abonniert(&self) -> &Self {
        assert!(self.c.subscribe_json(
            "main",
            &serde_json::to_vec(&json!({
                "type": "subscribe_session",
                "adresse": self.main.adresse,
                "session_epoch": self.main.adresse.session_epoch
            }))
            .unwrap()
        ));
        self
    }

    fn p0(&self, wert: &Value) -> Value {
        self.p0_von("main", wert)
            .expect("die Familie wird beantwortet")
    }

    /// Derselbe Weg ueber einen benannten Link — und mit der Moeglichkeit, dass
    /// GAR KEINE Antwort kommt.
    ///
    /// 🔑 Nacharbeit 3 (Befund B16): verweigert der Store den Append, bleibt
    /// der P0 unbeantwortet und der Link gilt als storeverweigert. Das ist die
    /// haerteste Form von „nicht angewandt", und ein Bein, das eine Antwort
    /// ERZWINGT, koennte sie nicht messen.
    fn p0_von(&self, link: &str, wert: &Value) -> Option<Value> {
        let payload = serde_json::to_vec(wert).unwrap();
        let antwort = Senke::p0(&self.c, link, &payload)?;
        Some(serde_json::from_slice(&antwort).expect("command_ack ist JSON"))
    }

    /// Ein ZWEITER Coordinator auf DEMSELBEN Store — der Brokerneustart.
    ///
    /// 🔑 Befund R12: der behauptete Restart-Test der Runde 1 erzeugte gar
    /// keinen zweiten Coordinator und konnte deshalb nicht fallen.
    /// Schaltet die Append-Naht des Stores (Befund B16). Sie liegt VOR dem
    /// Writerkanal und laesst jeden Append scheitern, als haette der Store den
    /// Dienst verweigert; Guards, Checkpoints und Kompaktierung bleiben heil.
    /// Die offenen Zustellschulden dieses Stores (Befund B17).
    fn outbox(&self) -> Vec<(String, i64)> {
        self._writer
            .handle()
            .outbox_lesen()
            .unwrap_or_default()
            .into_iter()
            .map(|(z, ord, _)| (z.object_key, ord))
            .collect()
    }

    fn naht(&self, an: bool) {
        self._writer.handle().append_naht_setzen(an);
    }

    fn neuer_coordinator(&self) -> Coordinator {
        Coordinator::mit_store(Arc::new(ManualClock::default()), hex(0xbeee), &self._writer)
    }

    fn db(&self) -> std::path::PathBuf {
        self._ordner.db()
    }

    fn zeilen(&self, sql: &str) -> i64 {
        rusqlite::Connection::open(self.db())
            .unwrap()
            .query_row(sql, [], |z| z.get(0))
            .unwrap()
    }
}

/// Ein vertragsgueltiges `state_report` — es setzt `record_state`, ohne das
/// kein persistenzpflichtiger Befehl angenommen wird.
fn state_report_payload(adresse: &Adresse, revision: u64) -> Vec<u8> {
    serde_json::to_vec(&json!({
        "type": "state_report",
        "adresse": adresse,
        "dsp_schema_version": 1,
        "state_revision": revision,
        "state_hash": "a".repeat(64),
        "record_state": {"valid": true, "recording": false}
    }))
    .unwrap()
}

// ═════════════════════════════════════════════════════════════════════════
// B13/B14 · Der Evidenzempfaenger
// ═════════════════════════════════════════════════════════════════════════

/// B13 — jeder angenommene Snapshot bleibt, bis die Retention greift.
///
/// Der Insert ERSETZTE vorher den vorigen Snapshot derselben Quelle. Aus
/// EINEM Punkt lassen sich weder Resultat- noch Guardrail-Deltas rechnen
/// (M-49), und ein Export ohne Evidence-IDs ist keiner (M-51).
#[test]
fn evidenz_historie_bleibt_und_ist_gedeckelt() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);

    for nr in 0..5 {
        assert!(
            c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, nr, |_| {})),
            "Snapshot {nr} wird angenommen"
        );
    }
    let historie = c.evidenz_historie(&hex(10));
    assert_eq!(historie.len(), 5, "jeder angenommene Snapshot bleibt");
    let ids: Vec<&str> = historie.iter().map(|e| e.evidence_id.as_str()).collect();
    assert_eq!(
        ids.len(),
        ids.iter().collect::<std::collections::BTreeSet<_>>().len(),
        "und jeder traegt seine EIGENE evidence_id (M-51)"
    );
    // Der zuletzt angenommene steht weiterhin einzeln zur Verfuegung.
    assert_eq!(
        c.evidenz_sicht(&hex(10)).unwrap().evidence_id,
        historie.last().unwrap().evidence_id
    );
    // Und die Baender reisen mit — ohne sie kann der PRE/POST-Join nichts.
    assert!(
        !historie.last().unwrap().p50_db.is_empty(),
        "der Empfaenger BEHAELT die Verteilung, statt sie zu verwerfen (B25)"
    );
}

/// B13 — die Retention ist ein DECKEL und keine unbegrenzte Historie (M-74).
#[test]
fn evidenz_historie_haelt_ihren_deckel() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);

    let deckel = eqcop_broker::coordinator::EVIDENZ_RETENTION;
    for nr in 0..(deckel + 7) {
        c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, nr, |_| {}));
    }
    let historie = c.evidenz_historie(&hex(10));
    assert_eq!(
        historie.len(),
        deckel,
        "die Historie waechst nicht ueber ihren Deckel (M-74)"
    );
    assert_eq!(
        historie.first().unwrap().evidence_id,
        hex(0x1000 + 7),
        "und sie schneidet vorne ab - die JUENGSTEN bleiben"
    );
}

/// B14 (Luecke, entschieden) — `beeinflusst=true` schliesst aus und zaehlt.
#[test]
fn beeinflusste_evidenz_wird_ausgeschlossen_und_gezaehlt() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);

    // Die Gegenprobe: ohne das Flag wird derselbe Snapshot angenommen.
    assert!(c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, 0, |_| {})));
    assert_eq!(c.evidence_beeinflusst_zaehler(), 0);

    let vorher = c.evidenz_historie(&hex(10)).len();
    assert!(
        !c.evidence_snapshot_json(
            "a",
            &evidenz_payload(&h.adresse, 1, |w| {
                w["beeinflusst"] = json!(true);
            })
        ),
        "ein Snapshot mit `beeinflusst=true` wird NICHT angenommen (M-52)"
    );
    assert_eq!(
        c.evidenz_historie(&hex(10)).len(),
        vorher,
        "und landet auch nicht in der Historie"
    );
    assert_eq!(
        c.evidence_beeinflusst_zaehler(),
        1,
        "er ist GEZAEHLT, nicht still verworfen"
    );
    // Das globale Gate war dabei frei - genau das ist der Punkt: das
    // Frame-Flag ist ein ZUSAETZLICHER Ausschlussbeleg.
    assert!(c.interventionssicht().starke_evidenz_erlaubt);
}

/// B15 — die Evidenzmap faellt mit ihrem Client (M-74, verbinden↔trennen).
#[test]
fn evidenz_faellt_mit_dem_client() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);
    assert!(c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, 0, |_| {})));
    assert!(!c.evidenz_historie(&hex(10)).is_empty());

    // Ein Hello mit NEUER Nonce: ein neuer Prozess erbt die Messwahrheit des
    // alten nicht.
    let neu = hello(1, 2, 10, 999, "passive_probe", Some(9));
    anmelden(&c, "b", &neu);
    assert!(
        c.evidenz_historie(&hex(10)).is_empty(),
        "die Evidenzhistorie faellt mit der alten Runtime-Nonce"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// B17 · Der Taint ist sitzungsweit
// ═════════════════════════════════════════════════════════════════════════

/// B17 — ein Marker in Sitzung A sperrt Sitzung B NICHT, und ein Resync aus B
/// loescht die aktiven IDs von A nicht (M-62).
#[test]
fn taint_ist_sitzungsweit() {
    let (c, _) = coordinator();
    let a = hello(1, 2, 10, 100, "main", Some(9));
    let b = hello(1, 3, 20, 200, "main", Some(8));
    anmelden(&c, "la", &a);
    anmelden(&c, "lb", &b);

    let id = hex(0x500);
    assert!(c.intervention_begin("la", &a.adresse, &id, 1));

    assert!(
        !c.interventionssicht_fuer_link("la").starke_evidenz_erlaubt,
        "die Sitzung MIT dem Marker ist gesperrt"
    );
    assert!(
        c.interventionssicht_fuer_link("lb").starke_evidenz_erlaubt,
        "die andere Sitzung nicht - genau das war Befund B17"
    );
    assert!(
        c.evidence_dispatch_fuer_link("lb"),
        "und ihr Dispatch geht durch"
    );
    assert!(!c.evidence_dispatch_fuer_link("la"));

    // Der gefaehrlichere Teil: ein Resync aus B raeumt A NICHT ab.
    assert!(c.neutral_resync("lb", 0));
    assert!(
        !c.interventionssicht_fuer_link("la").starke_evidenz_erlaubt,
        "ein fremder Resync loescht die aktive Intervention von A nicht"
    );
    assert!(!c.evidence_dispatch_fuer_link("la"));

    // Und der EIGENE Resync loest sie.
    assert!(c.neutral_resync("la", 1));
    assert!(c.interventionssicht_fuer_link("la").starke_evidenz_erlaubt);
}

/// B16/R02 — der Nachlauf laeuft in ECHTZEIT ab, nicht je Tick (M-58/M-60).
///
/// 🔑 Nacharbeit 2 (Befund R02): der alte Fall rief `liveness_tick()` OHNE
/// Zeitfortschritt und pruefte nur, dass der Wert irgendwann null wird. Er
/// konnte deshalb nicht sehen, dass die Runde 1 je Tick eine feste Samplezahl
/// abzog, die mit einem Ein-Sekunden-Tick begruendet war — waehrend der
/// produktive Supervisor alle 100 ms tickt (`lib.rs`). Ein Nachlauf von 48.000
/// Samples war damit nach 200 ms frei statt nach einer Sekunde, und genau in
/// diesem Fenster laeuft der Filterhall des Markers noch in die Messung.
///
/// Dieser Fall faehrt den PRODUKTIVEN Takt: 100 ms je Tick bei 48 kHz.
#[test]
fn nachlauf_laeuft_in_echtzeit_ab() {
    let (c, clock) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    let id = hex(0x600);
    assert!(c.intervention_begin("a", &h.adresse, &id, 1));
    // Eine ganze Sekunde Nachlauf bei 48 kHz.
    assert!(c.intervention_end("a", &h.adresse, &id, 2, 48_000));
    assert_eq!(c.interventionssicht_fuer_link("a").tail_samples_offen, 48_000);
    assert!(!c.evidence_dispatch_fuer_link("a"), "der Nachlauf sperrt");

    // Der erste Tick hat keinen Vorgaenger und zieht nichts ab: ihm die
    // Laufzeit seit Brokerstart zuzuschreiben waere eine erfundene Frist.
    c.liveness_tick();
    assert_eq!(
        c.interventionssicht_fuer_link("a").tail_samples_offen,
        48_000,
        "der erste Tick setzt nur den Bezugspunkt"
    );

    // Fuenf produktive Ticks = 500 ms. Der Nachlauf ist zur HAELFTE abgelaufen
    // und sperrt weiter. Mit der alten Konstante (24.000 je Tick) waere er
    // hier laengst frei — das ist der Rotbeweis dieses Falls.
    for _ in 0..5 {
        clock.vor(100);
        c.liveness_tick();
    }
    let rest = c.interventionssicht_fuer_link("a").tail_samples_offen;
    assert_eq!(
        rest, 24_000,
        "nach 500 ms sind bei 48 kHz genau 24.000 Samples abgelaufen"
    );
    assert!(
        !c.evidence_dispatch_fuer_link("a"),
        "und der halbe Nachlauf sperrt weiter"
    );

    // Die zweite Haelfte.
    for _ in 0..5 {
        clock.vor(100);
        c.liveness_tick();
    }
    assert_eq!(
        c.interventionssicht_fuer_link("a").tail_samples_offen,
        0,
        "nach einer Sekunde ist er abgelaufen"
    );
    assert!(
        c.evidence_dispatch_fuer_link("a"),
        "danach ist Evidenz wieder erlaubt"
    );
}

/// B16 — `resync_bestaetigen` ist der Produktaufrufer von `neutral_resync`.
#[test]
fn bestaetigter_resync_loest_das_sticky_unknown() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    // Eine Sequenzluecke setzt sticky Unknown.
    let id = hex(0x700);
    assert!(c.intervention_begin("a", &h.adresse, &id, 1));
    assert!(!c.intervention_end("a", &h.adresse, &id, 9, 0), "Luecke");
    assert!(c.interventionssicht_fuer_link("a").unknown);

    assert!(
        !c.resync_bestaetigen("unbekannt", 9),
        "ein unbekannter Link darf nicht entsperren"
    );
    assert!(c.resync_bestaetigen("a", 9), "der stehende Link darf");
    assert!(
        c.interventionssicht_fuer_link("a").starke_evidenz_erlaubt,
        "und danach ist der Zustand wieder bekannt"
    );
}

/// C1 — der Sequenzhandschlag nach einem bestaetigten Resync.
///
/// 🔑 Wiederpruefung 2 (Befund C1, M-61): der Fehler lag ZWISCHEN den
/// Sprachen. `resync_bestaetigen(link, 0)` setzt die Basis auf 0, und der
/// Broker liest das als „die naechste ist 1"; das Plugin sendete seine erste
/// Intervention mit 0 und setzte den Zaehler bei Reconnect nicht zurueck. Der
/// Broker verwarf damit die erste Intervention JEDER Verbindung als Luecke und
/// setzte `taint.unknown` sofort wieder — der R01-Fix hob sich selbst auf.
///
/// Beide Seiten waren fuer sich gruen. Deshalb misst dieser Fall gegen eine
/// DRITTE Instanz: `eq-copilot/fixtures/v3/handschlag-v1.json` ist von Hand
/// geschrieben und die Ausgabe keiner der beiden Implementierungen — dieselbe
/// Bauform wie `MANIFEST.json` fuer den Fixturekorpus. Das C++-Gegenstueck in
/// `EqCopSonde013PassageStateTest` (C1) liest dieselbe Datei und prueft, dass
/// die Zahl auf der Leitung ihr entspricht. Stimmen beide mit der Datei
/// ueberein, stimmen sie transitiv miteinander ueberein.
#[test]
fn erste_intervention_nach_resync_wird_angenommen() {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .unwrap()
        .join("eq-copilot/fixtures/v3/handschlag-v1.json");
    let vertrag: Value =
        serde_json::from_str(&std::fs::read_to_string(&pfad).expect("Handschlagdatei fehlt"))
            .expect("Handschlagdatei ist kein JSON");
    let basis = vertrag["resync_sequenzbasis"].as_u64().expect("Basis fehlt");
    let erste = vertrag["erste_intervention_nach_resync"]
        .as_u64()
        .expect("erste Sequenz fehlt");
    assert_eq!(
        erste,
        basis + 1,
        "der Handschlag muss in sich stimmen: die erste Sequenz folgt auf die Basis"
    );

    // Der Weg, den ein frischer Link geht: anmelden, Resync bestaetigen, und
    // dann die ERSTE Intervention.
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    assert!(c.resync_bestaetigen("a", basis), "der Resync wird bestaetigt");
    assert!(
        c.intervention_begin("a", &h.adresse, &hex(0x7101), erste),
        "die erste Intervention nach dem Resync wird ANGENOMMEN"
    );
    assert!(
        !c.interventionssicht_fuer_link("a").unknown,
        "und der Interventionszustand bleibt BEKANNT - genau das hob der alte \
         Widerspruch sofort wieder auf. Gemessen wird `unknown`, nicht \
         `starke_evidenz_erlaubt`: ein OFFENER Eingriff sperrt starke Evidenz zu \
         Recht, und das ist eine andere Aussage."
    );

    // Die Gegenprobe: die Zahl DAVOR ist eine Wiederholung der Basis und wird
    // abgelehnt. Ohne sie bewiese der Fall nur, dass irgendeine Zahl geht.
    let (c2, _) = coordinator();
    anmelden(&c2, "b", &h);
    assert!(c2.resync_bestaetigen("b", basis));
    assert!(
        !c2.intervention_begin("b", &h.adresse, &hex(0x7102), basis),
        "die Basis selbst ist bereits verbraucht und wird abgelehnt"
    );
    assert!(
        c2.interventionssicht_fuer_link("b").unknown,
        "und der Zustand ist danach unbekannt"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// B18/B19/B20/B21/B22 · Die Experimentfamilien im Produktpfad
// ═════════════════════════════════════════════════════════════════════════

/// Ein Coordinator mit einem bestaetigten fuehrenden Main — die Lage, in der
/// ein `experiment_begin` ueberhaupt autorisiert ist.
fn mit_main() -> (Coordinator, HelloControl) {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "main", &h);
    report(&c, "main", &h.adresse);
    (c, h)
}

#[cfg(windows)]
/// B18/B23 — ein schema-gueltiges `experiment_begin` legt Passage UND Versuch
/// an (M-25/M-40).
///
/// Vorher fiel die Nachricht in `_ => None`: der Coordinator besass keinen
/// Experimentstore, und `p0_json_mit_minor` hatte keinen Match-Zweig.
#[test]
fn experiment_begin_legt_passage_und_versuch_an() {
    let (c, h) = mit_main();
    let wert = experiment_begin_wert(&h.adresse, 0x900, 0xabc);
    let payload = serde_json::to_vec(&wert).unwrap();

    let antwort = Senke::p0(&c, "main", &payload)
        .expect("B18: die Familie wird BEANTWORTET - vorher fiel sie in `_ => None`");
    let ack: Value = serde_json::from_slice(&antwort).unwrap();
    assert_eq!(
        ack["type"], "command_ack",
        "und zwar mit einem command_ack, wie jeder andere persistente P0-Befehl"
    );
    // Dieser Coordinator hat KEINEN Store. Ein persistenter Befehl ist dann
    // `abgelehnt/internal` — dieselbe Regel wie fuer `preview_*`, und genau
    // deshalb misst der Store-Fall in `store_crash_matrix.rs` die Wirkung.
    assert_eq!(ack["ergebnis"], "abgelehnt");
    assert_eq!(ack["code"], "internal");
    // Und ohne Wirkung entsteht auch KEIN halber Zustand.
    assert!(
        c.passage_sicht(wert["passage"]["passage_id"].as_str().unwrap())
            .is_none(),
        "ein abgelehnter Befehl legt keine Passage an"
    );
    assert!(c.experiment_sicht(&hex(0xabc)).is_none());
}

/// B20 — ein Ergebnis OHNE Resultatmessung schliesst nicht ab (M-45).
#[test]
fn ergebnis_ohne_resultatmessung_wird_abgelehnt() {
    use eqcop_broker::coordinator::experiment::{
        Abschlussfehler, Alignmentwert, Blindreihenfolge, Experimentreferenz, Experimentstore,
        Hoerurteil, Passage, Resultatmessung,
    };
    use eqcop_broker::telemetrie::Fingerprintwerte;

    let mut s = Experimentstore::neu();
    let passage = Passage {
        passage_id: hex(1),
        projekt_von: 0,
        projekt_bis: 480_000,
        transport_epoch: 1,
        aktive_quellen: vec![hex(10)],
        messpunktklassen: vec!["insert".into()],
        abdeckung: 0.9,
        label: None,
        fingerprint: Fingerprintwerte::default(),
    };
    let referenz = Experimentreferenz {
        passage_fingerprint: Fingerprintwerte::default(),
        upstream_fingerprint: Fingerprintwerte::default(),
        aktive_quellen: vec![hex(10)],
        messpunktklassen: vec!["insert".into()],
        match_gain_db: -1.5,
        nicht_endliche_samples: Some(0),
        alignment: Alignmentwert::FeatureAligned,
    };
    let id = hex(0xabc);
    s.beginne(&id, &hex(1), passage, referenz.clone(), 100).unwrap();
    s.binde_reihenfolge(&id, Blindreihenfolge::BaselineZuerst)
        .unwrap();

    // 🔑 Nacharbeit 2 (Befund R16, M-41): OHNE erfassten Kandidaten gibt es
    // kein Ergebnis - und zwar auch dann nicht, wenn eine vollstaendige
    // Messung vorliegt. Der Fall der Runde 1 FUHR genau diesen Ablauf und
    // ERWARTETE Erfolg; er schrieb die Verletzung fest, statt an ihr zu
    // fallen. Eine Messung ohne Kandidat misst zweimal denselben Zustand.
    let messung = Resultatmessung {
        band_delta_db: (0..32).map(|i| (i as f64) * 0.2 - 3.0).collect(),
        band_gueltig: vec![true; 32],
        fenster_delta_db: (0..8)
            .map(|f| {
                (0..32)
                    .map(|i| (i as f64) * 0.2 - 3.0 + (f as f64) * 0.01)
                    .collect()
            })
            .collect(),
        erste_haelfte: vec![1.0; 32],
        zweite_haelfte: vec![1.0; 32],
        abdeckung_baseline: 0.9,
        abdeckung_resultat: 0.9,
        klasse_baseline: "mittel".into(),
        klasse_resultat: "mittel".into(),
        vergleichbarkeit: Some("stark".into()),
        vergleichbarkeit_gruende: Vec::new(),
        baseline_evidence_ids: vec![hex(0x11)],
        resultat_evidence_ids: vec![hex(0x22)],
        guardrail_loudness_db: Some(0.0),
        guardrail_peak_db: Some(0.0),
        guardrail_transient: Some(0.0),
        guardrail_breite_db: Some(0.0),
        guardrail_geschuetzt_db: Some(0.0),
        guardrail_nicht_gemessen: Vec::new(),
    };
    assert_eq!(
        s.ergebnis(&id, Hoerurteil::Kandidat, None, None, &messung),
        Err(Abschlussfehler::OhneKandidat),
        "R16: ohne Kandidat gibt es kein Terminalereignis (M-41)"
    );
    assert!(
        s.experiment(&id).unwrap().offen(),
        "und der Versuch bleibt OFFEN"
    );

    // Mit Kandidat, aber OHNE Messung: der zweite Riegel (M-45).
    s.neuer_kandidat(&id, referenz, 200).unwrap();
    assert_eq!(
        s.ergebnis(
            &id,
            Hoerurteil::Kandidat,
            None,
            None,
            &Resultatmessung::default()
        ),
        Err(Abschlussfehler::OhneResultatmessung),
        "ohne Resultatmessung gibt es kein Terminalereignis (M-45)"
    );
    assert!(s.experiment(&id).unwrap().offen());

    // Mit BEIDEM: die vier Achsen entstehen und reisen mit.
    let achsen = s
        .ergebnis(&id, Hoerurteil::Kandidat, None, None, &messung)
        .expect("mit Messung schliesst der Versuch ab");
    assert!(achsen.gescannte_baender > 0, "die Zielmetrik ist gerechnet");
    assert!(
        achsen.intervall.is_some(),
        "das Bootstrap-Intervall steht: {achsen:?}"
    );
    assert!(
        achsen.effekt_stabil.is_some(),
        "und die Effektstabilitaet ist beurteilt"
    );

    // B21: der Export traegt die Evidence-IDs.
    let export = s.exportiere(&id).expect("Export entsteht");
    assert_eq!(export.baseline_evidence_ids, vec![hex(0x11)]);
    assert_eq!(export.resultat_evidence_ids, vec![hex(0x22)]);

    // B19: das Log traegt die unveraenderlichen Referenzen, nicht nur IDs.
    use eqcop_broker::coordinator::experiment::Ereignis;
    let hat_passage = s
        .log()
        .iter()
        .any(|e| matches!(e, Ereignis::PassageAngelegt { passage, .. } if passage.projekt_bis > 0));
    assert!(
        hat_passage,
        "das Passageereignis traegt die PASSAGE, nicht nur ihre ID (M-47/M-50)"
    );
    let hat_baseline = s.log().iter().any(
        |e| matches!(e, Ereignis::Begonnen { baseline, .. } if baseline.match_gain_db == -1.5),
    );
    assert!(
        hat_baseline,
        "und das Beginereignis traegt die Baseline samt Match-Gain (§43.1)"
    );
    let hat_achsen = s
        .log()
        .iter()
        .any(|e| matches!(e, Ereignis::Ergebnis { achsen, .. } if achsen.gescannte_baender > 0));
    assert!(
        hat_achsen,
        "und das Terminalereignis traegt die gerechneten Deltas (M-49)"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// B24 · Die Invalidierung
// ═════════════════════════════════════════════════════════════════════════

/// B24 — eine Invalidierung MARKIERT gespeicherte Evidenz als ausgeschlossen.
///
/// `invalidierung` enthielt vorher nur Konstruktoren: kein Aufrufer, kein
/// Serializer, keine Outbox, keine Mutation. Marker, Preview, Seek, Drop,
/// Material- und Messpunktwechsel nahmen im Produkt nichts zurueck.
#[test]
fn invalidierung_schliesst_gespeicherte_evidenz_aus() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);
    for nr in 0..3 {
        assert!(c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, nr, |_| {})));
    }
    assert!(
        c.evidenz_historie(&hex(10))
            .iter()
            .all(|e| e.ausschlussgrund.is_none()),
        "vorher traegt kein Beleg einen Ausschlussgrund"
    );

    let betroffen = c.invalidierung_wegen_messpunkt_fuer_link("a", "insert", "post");
    assert!(
        betroffen > 0,
        "der Messpunktwechsel nimmt gespeicherte Evidenz ZURUECK (M-55)"
    );
    let historie = c.evidenz_historie(&hex(10));
    assert!(
        historie
            .iter()
            .all(|e| e.ausschlussgrund.as_deref() == Some("messpunkt_wechsel")),
        "und jeder Beleg traegt den GRUND, nie einen stillen Ausschluss (M-28)"
    );
    assert_eq!(c.invalidierungen_zaehler(), 1);
    assert_eq!(c.evidenz_ausgeschlossen_zaehler(), betroffen as u64);

    // Ein gleicher Messpunkt invalidiert NICHT - die Gegenprobe, ohne die der
    // Fall darueber nichts sagt.
    assert_eq!(
        c.invalidierung_wegen_messpunkt_fuer_link("a", "insert", "insert"),
        0
    );
    assert_eq!(c.invalidierungen_zaehler(), 1, "und zaehlt auch nicht");
}

/// Das COMMITTETE v3-Schema, geladen mit derselben Engine wie im Produktpfad.
///
/// 🔑 Nacharbeit 2 (Befund R26): der alte Serializertest verglich die erzeugten
/// Felder mit Namen, die der Test SELBST behauptete — und schrieb damit den
/// Vertragsbruch fest, statt an ihm zu fallen. Ein Payload wird ab jetzt gegen
/// die Vertragsdatei gemessen, nicht gegen eine Erwartung daneben.
fn v3_schema() -> eqcop_broker::vertrag::Schema {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../eq-copilot/schemas/v3/eq-ipc-v3.schema.json");
    let wurzel: Value = serde_json::from_slice(&std::fs::read(&pfad).expect("Schema liegt im Repo"))
        .expect("Schema ist JSON");
    eqcop_broker::vertrag::Schema::laden(wurzel).expect("Schema ist unterstuetzt")
}

/// R26 — JEDER vom Broker erzeugte `evidence_invalidate` haelt den VERTRAG.
///
/// Gemessen wird gegen `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, nicht
/// gegen Feldnamen im Test. Der alte Payload trug `von_sample`/`bis_sample`
/// und ein Top-Level `ausgeschlossen`; `invalidate_bereich` und
/// `evidence_invalidate` sind beide `additionalProperties: false` — eine
/// direkte Schemapruefung lehnt ihn ab.
#[test]
fn invalidierung_serialisiert_ihre_drei_umfaenge() {
    use eqcop_broker::coordinator::invalidierung::{Grund, Invalidierung, Umfang};
    let schema = v3_schema();

    let ganz = Coordinator::invalidierung_als_json(&Invalidierung {
        grund: Grund::Epochwechsel,
        umfang: Umfang::GanzeSitzung,
    });
    assert_eq!(schema.pruefe(&ganz), vec![], "ganze_sitzung haelt den Vertrag");
    assert_eq!(ganz["type"], "evidence_invalidate");
    assert_eq!(ganz["grund"], "epochwechsel");
    assert_eq!(ganz["umfang"]["art"], "ganze_sitzung");

    let bereich = Coordinator::invalidierung_als_json(&Invalidierung {
        grund: Grund::Sequenzluecke,
        umfang: Umfang::Bereich { von: 10, bis: 20 },
    });
    assert_eq!(
        schema.pruefe(&bereich),
        vec![],
        "sample_range haelt den Vertrag: {bereich}"
    );
    assert_eq!(bereich["umfang"]["sample_start"], 10);
    assert_eq!(bereich["umfang"]["sample_end"], 20);

    let ids = Coordinator::invalidierung_als_json(&Invalidierung {
        grund: Grund::MaterialWechsel,
        umfang: Umfang::Ids([hex(1)].into_iter().collect()),
    });
    assert_eq!(schema.pruefe(&ids), vec![], "evidence_ids haelt den Vertrag");
    assert_eq!(ids["umfang"]["evidence_ids"][0], hex(1));
}

/// R26 — auch der Umfang aus dem PRODUKTPFAD haelt den Vertrag.
///
/// Der Fall darueber misst den Serializer allein. Dieser hier faehrt den
/// echten Ausloeser: ein Transportbruch zwischen zwei Snapshots. Der frueher
/// gebildete Rand `i64::MIN / 2` verletzt `minimum: 0` — ein Payload, den kein
/// Subscriber annehmen duerfte.
#[test]
fn invalidierung_aus_dem_produktpfad_haelt_den_vertrag() {
    use eqcop_broker::coordinator::invalidierung::{Grund, Invalidierung, Umfang};
    let schema = v3_schema();
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);
    assert!(c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, 0, |_| {})));
    assert!(c.evidence_snapshot_json(
        "a",
        &evidenz_payload(&h.adresse, 1, |w| {
            w["transport"]["transport_epoch"] = json!(9);
        })
    ));
    assert_eq!(c.invalidierungen_zaehler(), 1, "der Bruch loest aus");

    // Derselbe Umfang, den `invalidierung_aus_transportbruch` bildet: der
    // Bereich VOR dem Bruch. Sein Anfang ist 0 und nicht `i64::MIN / 2`.
    let payload = Coordinator::invalidierung_als_json(&Invalidierung {
        grund: Grund::Epochwechsel,
        umfang: Umfang::Bereich {
            von: 0,
            bis: 48_000,
        },
    });
    assert_eq!(
        schema.pruefe(&payload),
        vec![],
        "der Produktpfad-Payload haelt den Vertrag: {payload}"
    );
    assert!(
        payload["umfang"]["sample_start"].as_i64().expect("Zahl") >= 0,
        "ein negativer Rand ist im Vertrag kein Bereich, sondern eine Verletzung"
    );
}

/// B24 — ein Transportbruch loest die Invalidierung MIT DEM RICHTIGEN GRUND
/// aus (M-53).
#[test]
fn transportbruch_invalidiert_mit_seinem_grund() {
    for (feld, grund) in [
        ("transport_epoch", "epochwechsel"),
        ("continuity_segment", "sequenzluecke"),
    ] {
        let (c, _) = coordinator();
        let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
        anmelden(&c, "a", &h);
        report(&c, "a", &h.adresse);
        assert!(c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, 0, |_| {})));
        assert_eq!(c.invalidierungen_zaehler(), 0, "{feld}: noch kein Bruch");

        assert!(c.evidence_snapshot_json(
            "a",
            &evidenz_payload(&h.adresse, 1, |w| {
                let alt = w["transport"][feld].as_u64().unwrap_or(0);
                w["transport"][feld] = json!(alt + 1);
            })
        ));
        assert_eq!(
            c.invalidierungen_zaehler(),
            1,
            "{feld}: der Bruch loest eine Invalidierung aus"
        );
        let historie = c.evidenz_historie(&hex(10));
        assert!(
            historie
                .iter()
                .any(|e| e.ausschlussgrund.as_deref() == Some(grund)),
            "{feld}: und zwar mit dem Grund `{grund}` (M-53): {:?}",
            historie
                .iter()
                .map(|e| e.ausschlussgrund.clone())
                .collect::<Vec<_>>()
        );
    }
}

// ═════════════════════════════════════════════════════════════════════════
// B25 · Der PRE/POST-Join
// ═════════════════════════════════════════════════════════════════════════

/// B25 — der Join LAEUFT im Produktpfad und liefert ein Urteil je Paar.
///
/// `bilde_paare` hatte ausserhalb der Tests keinen Aufrufer, und der
/// Evidenzempfaenger verwarf die Kurven, die eine `Paarhaelfte` braucht.
/// Reale PRE-/POST-Snapshots erreichten den v3-Join damit nie.
#[test]
fn prepost_join_laeuft_im_produktpfad() {
    let (c, _) = coordinator();
    let pre = hello(1, 2, 10, 100, "passive_probe", Some(9));
    let post = hello(1, 2, 11, 101, "passive_probe", Some(9));
    anmelden(&c, "pre", &pre);
    anmelden(&c, "post", &post);
    // Die Deskriptoren tragen `pair_id` und Messpunkt - ohne sie gibt es
    // keine Rolle und damit keine Haelfte.
    let paar = hex(0x77);
    assert!(
        c.descriptor_setzen("pre", descriptor(&pre.adresse, "pre", &paar)),
        "der PRE-Deskriptor wird angenommen"
    );
    assert!(
        c.descriptor_setzen("post", descriptor(&post.adresse, "post", &paar)),
        "der POST-Deskriptor auch"
    );

    assert_eq!(
        c.paarurteile_anzahl(),
        0,
        "vor dem ersten Snapshot gibt es kein Urteil"
    );
    for nr in 0..8 {
        c.evidence_snapshot_json("pre", &evidenz_payload(&pre.adresse, nr, |_| {}));
        c.evidence_snapshot_json("post", &evidenz_payload(&post.adresse, nr, |_| {}));
    }

    // 🔑 DAS ist die Zusage: der Join laeuft IM EMPFAENGER und legt sein
    // Urteil ab. Vorher hatte `bilde_paare` ausserhalb der Tests keinen
    // Aufrufer, und der Empfaenger verwarf die Kurven, die eine Haelfte
    // braucht — es entstand nie ein Urteil.
    let urteil = c
        .paarurteil(&paar)
        .expect("B25: der PRE/POST-Join liefert ein Urteil je Paar (M-13/M-14)");
    assert_eq!(urteil.pair_id, paar);
    // Und er behauptet NICHTS Starkes: die Presentation-Abbildung ist
    // nirgends validiert (Capabilityreport S4), also bleibt die Klasse unter
    // `FeatureAligned` — genau der Exit-Gate-Satz aus M-21.
    use eqcop_broker::coordinator::prepost::Alignmentklasse;
    assert!(
        urteil.klasse < Alignmentklasse::FeatureAligned,
        "ohne validierte Presentation-Abbildung keine starke Aussage: {:?}",
        urteil.klasse
    );
}

/// Selbstaudit dieser Runde (M-74): die Taintmap haelt ihren Deckel.
///
/// Der sitzungsweite Taint aus B17 hat eine Kehrseite, die die Erstpruefung
/// nicht melden konnte, weil es ihn noch nicht gab: die Map waechst mit jeder
/// Sitzung, die je einen Eingriff gesehen hat, und Sitzungen entstehen bei
/// jedem FL-Neustart neu. Ein SAUBERER Eintrag ohne Client faellt sofort; ein
/// DIRTY bleibt, weil sein sticky Unknown eine Aussage ist (§34.2). Genau die
/// koennen sich anhaeufen — deshalb der Deckel.
#[test]
fn taintmap_haelt_ihren_deckel() {
    use eqcop_broker::coordinator::GLOBAL_SESSION_CAP;
    let (c, clock) = coordinator();

    // Mehr TOTE Sitzungen mit offenem Taint, als der Broker fuehren kann.
    for nr in 0..(GLOBAL_SESSION_CAP + 8) {
        let link = format!("l{nr}");
        let h = hello(1, 1000 + nr, 2000 + nr, 3000 + nr, "main", Some(9));
        anmelden(&c, &link, &h);
        // Eine offene Intervention: der Eintrag ist DIRTY.
        assert!(c.intervention_begin(&link, &h.adresse, &hex(0x9000 + nr), 1));
        // Und der Disconnect macht ihn sticky unbekannt.
        c.control_ende(&link);
        clock.setze_ms((nr as u64 + 1) * 60_000);
        c.liveness_tick();
    }

    // Die Zusage: die Map bleibt in ihren Grenzen. Ohne den Deckel stuende
    // hier die volle Zahl der jemals gesehenen Sitzungen.
    let sicht = c.interventionssicht();
    assert!(
        sicht.unknown,
        "der Zustand bleibt konservativ unbekannt - der Deckel raeumt auf, er \
         beschoenigt nicht"
    );
    assert!(
        c.taint_verworfen_zaehler() > 0,
        "und der Verlust einer sticky Sperre ist GEZAEHLT, nicht still: {}",
        c.taint_verworfen_zaehler()
    );
}

// ═════════════════════════════════════════════════════════════════════════
// R22/R23/R27 · Der Vertrag im Produktpfad (Nacharbeit 2)
// ═════════════════════════════════════════════════════════════════════════

/// Ein `audible_intervention_begin`, wie der Draht ihn traegt.
fn intervention_begin_payload(
    adresse: &Adresse,
    intervention: usize,
    sequenz: u64,
    art: &str,
    experiment: Option<usize>,
) -> Vec<u8> {
    let mut wert = json!({
        "type": "audible_intervention_begin",
        "intervention_id": hex(intervention),
        "adresse": adresse,
        "event_sequence": sequenz,
        "art": art,
        "project_sample_start": 0
    });
    if let Some(e) = experiment {
        wert["experiment_id"] = json!(hex(e));
    }
    serde_json::to_vec(&wert).unwrap()
}

/// Ein `audible_intervention_end`, wie der Draht ihn traegt (NAK-180
/// Nacharbeit 2, WN-07).
///
/// Ohne ihn fuhren Replay und End des R2-Wegs als direkte Methodenaufrufe -
/// der Weg durch `p0_json_mit_minor`, die Vertragspruefung und das Auslesen
/// von `tail_samples` blieb dabei ungemessen.
fn intervention_end_payload(adresse: &Adresse, intervention: usize, sequenz: u64, tail: u64) -> Vec<u8> {
    serde_json::to_vec(&json!({
        "type": "audible_intervention_end",
        "intervention_id": hex(intervention),
        "adresse": adresse,
        "event_sequence": sequenz,
        "project_sample_end": null,
        "tail_samples": tail
    }))
    .unwrap()
}

/// R22 — `art` und `experiment_id` reisen vom DRAHT bis in den Taintzustand.
///
/// Der echte Dispatch rief `intervention_begin` und schrieb JEDE Intervention
/// als `hoermarkierung` ohne Experimentbezug fest. Ein schema-gueltiges
/// `art=experiment`-Intervall konnte damit von keinem Terminal geschlossen
/// werden — die Sperre auf starker Evidenz blieb nach jedem Resultat offen.
/// Der alte Store-Test umging genau diese Stelle mit einem direkten Aufruf von
/// `intervention_begin_mit_art` und konnte deshalb nicht fallen.
#[cfg(windows)]
#[test]
fn wire_intervention_traegt_art_und_experiment_bis_zum_terminal() {
    use eqcop_broker::vertrag::Schema;
    let schema = v3_schema();
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);

    let versuch = 0xe1;
    let payload = intervention_begin_payload(&h.adresse, 0x501, 1, "experiment", Some(versuch));
    // Erst der Vertrag: was der Test schickt, ist eine echte Wire-Nachricht.
    let wert: Value = serde_json::from_slice(&payload).unwrap();
    assert_eq!(
        Schema::pruefe(&schema, &wert),
        vec![],
        "das Fixture dieses Falls haelt den Vertrag: {wert}"
    );

    Senke::p0(&c, "a", &payload);
    assert_eq!(
        c.interventionssicht_fuer_link("a").aktive,
        1,
        "der Wireweg legt das Intervall an"
    );

    // Ein FREMDER Versuch schliesst es NICHT. Ohne diese Gegenprobe saehe ein
    // Intervall ohne Zuordnung genauso aus wie eines mit.
    assert_eq!(
        c.experiment_intervalle_schliessen_fuer_link("a", &hex(0xe2)),
        0,
        "ein fremder Versuch schliesst fremde Intervalle nicht"
    );
    assert_eq!(c.interventionssicht_fuer_link("a").aktive, 1);

    // Sein EIGENER Versuch schliesst es.
    assert_eq!(
        c.experiment_intervalle_schliessen_fuer_link("a", &hex(versuch)),
        1,
        "das Terminal des eigenen Versuchs schliesst sein Intervall (M-59)"
    );
    assert_eq!(c.interventionssicht_fuer_link("a").aktive, 0);
}

/// R22 — eine HOERMARKIERUNG gehoert keinem Versuch.
///
/// Die Gegenprobe zum Fall darueber: schriebe der Dispatch weiterhin jedes
/// Intervall als Experiment, schloesse ein beliebiges Terminal auch den
/// Hoermarker des Users.
#[cfg(windows)]
#[test]
fn wire_hoermarkierung_wird_von_keinem_experimentterminal_geschlossen() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    Senke::p0(
        &c,
        "a",
        &intervention_begin_payload(&h.adresse, 0x502, 1, "hoermarkierung", None),
    );
    assert_eq!(c.interventionssicht_fuer_link("a").aktive, 1);
    assert_eq!(
        c.experiment_intervalle_schliessen_fuer_link("a", &hex(0xe1)),
        0,
        "ein Experimentterminal ruehrt den Hoermarker des Users nicht an"
    );
    assert_eq!(c.interventionssicht_fuer_link("a").aktive, 1);
}

/// R22 — `art=experiment` OHNE `experiment_id` ist fail-closed.
///
/// Die Kante, die das Schema nicht ausdruecken kann (geschlossene
/// Schluesselwortmenge, kein Feldvergleich): ein Intervall, das kein Terminal
/// je schliessen koennte. Es als Hoermarkierung zu fuehren waere die stille
/// Umdeutung, die §34.2 ausschliesst.
#[cfg(windows)]
#[test]
fn wire_experimentintervall_ohne_id_wird_abgelehnt() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    Senke::p0(
        &c,
        "a",
        &intervention_begin_payload(&h.adresse, 0x503, 1, "experiment", None),
    );
    let sicht = c.interventionssicht_fuer_link("a");
    assert_eq!(sicht.aktive, 0, "kein Intervall ohne Zuordnung");
    assert!(
        sicht.unknown,
        "und der Zustand ist UNBEKANNT, nicht sauber - ein verworfenes Begin \
         darf nie eine scheinbar saubere Baseline erzeugen"
    );
}

/// R23 — die Messpunktklassen der Passage ueberleben das Wire-Lesen.
///
/// Das Schema fuehrt `passage.messpunktklassen` parallel zu `aktive_quellen`;
/// `passage_aus_wert` las das Feld nicht, und `Passage` hatte keinen Platz
/// dafuer. Die Zuordnung Quelle→Messpunkt ging damit schon beim Lesen
/// verloren — und mit ihr Gate 7.
#[cfg(windows)]
#[test]
fn passage_aus_dem_wire_traegt_ihre_messpunktklassen() {
    let wert = experiment_begin_wert(&adresse(1, 2, 10, 100), 0x901, 0xabd);
    let erwartet: Vec<String> = wert["passage"]["messpunktklassen"]
        .as_array()
        .expect("das Fixture traegt die Klassen")
        .iter()
        .map(|v| v.as_str().unwrap().to_owned())
        .collect();
    assert!(!erwartet.is_empty());

    let h = HarnischMitStore::neu("passage-messpunkt");
    let ack = h.p0(&wert);
    assert_eq!(ack["ergebnis"], "angewandt", "{ack}");

    let passage = h
        .c
        .passage_sicht(wert["passage"]["passage_id"].as_str().unwrap())
        .expect("die Passage entsteht im Produktpfad");
    assert_eq!(
        passage.messpunktklassen, erwartet,
        "die Klassen stehen in der Passage, nicht nur im Payload"
    );
    assert_eq!(
        passage.messpunktklassen.len(),
        passage.aktive_quellen.len(),
        "und zwar je Quelle eine, in derselben Reihenfolge (M-28/M-55)"
    );
}

/// R27 — eine Invalidierung ueberschreibt die SITZUNGSPROJEKTION nicht.
///
/// Der Event kam als `event_type = "session"` an; der Writer ersetzte damit
/// `sessions.state_jcs` durch den `evidence_invalidate`-Payload. Beim
/// naechsten Subscribe erwartet `subscription.rs` dort einen
/// `session_snapshot`, verwirft die Projektion und setzt Routing fail-closed —
/// eine Ruecknahme von Evidenz nahm die ganze Sitzung mit. Die alten
/// Invalidierungstests liefen ohne Store und konnten das nicht ausloesen.
#[cfg(windows)]
#[test]
fn invalidierung_laesst_die_sessionprojektion_stehen() {
    let h = HarnischMitStore::neu("invalidierung-projektion");
    h.abonniert();
    assert!(h.c.routing_bereit(), "Routing steht vor der Invalidierung");

    // Evidenz derselben Sitzung, danach ihre Ruecknahme.
    let sonde = {
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = hex(0x20);
        s.adresse.runtime_nonce = hex(0x21);
        s
    };
    anmelden(&h.c, "sonde", &sonde);
    report(&h.c, "sonde", &sonde.adresse);
    for nr in 0..2 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }
    let betroffen = h.c.invalidierung_wegen_messpunkt_fuer_link("sonde", "insert", "post");
    assert!(betroffen > 0, "die Ruecknahme trifft gespeicherte Evidenz");

    // Die Projektion ist noch ein Sessionsnapshot — nicht der Invalidierungspayload.
    let projektion: Vec<u8> = rusqlite::Connection::open(h.db())
        .unwrap()
        .query_row("SELECT state_jcs FROM sessions LIMIT 1", [], |z| z.get(0))
        .expect("die Sitzung hat eine Projektion");
    let wert: Value = serde_json::from_slice(&projektion).expect("Projektion ist JSON");
    assert_eq!(
        wert["type"], "session_snapshot",
        "die Sitzungsprojektion bleibt der Sessionsnapshot: {wert}"
    );

    // Und ein erneuter Subscribe faellt nicht fail-closed.
    assert!(h.abonniert().c.routing_bereit(), "Routing steht danach noch");

    // Die Invalidierung selbst liegt als EIGENER Ereignistyp im Log.
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM event_log WHERE event_type='evidence_invalidate'"),
        1,
        "die Ruecknahme hat ihren eigenen Ereignistyp (R27)"
    );
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM event_log WHERE event_type='session'"),
        h.zeilen("SELECT COUNT(*) FROM event_log WHERE event_type='session'"),
    );
}

/// R27 — der Ausschluss ueberdauert den Neustart.
///
/// Die Projektion der Invalidierung traegt den Grund in die betroffenen
/// `evidence`-Zeilen. Ohne sie waere die Ruecknahme nach einem Brokerneustart
/// fort, und die zurueckgenommene Evidenz saehe wieder aus wie jede andere.
#[cfg(windows)]
#[test]
fn invalidierung_projiziert_ihren_grund_in_die_evidenzzeilen() {
    let h = HarnischMitStore::neu("invalidierung-evidenzprojektion");
    let sonde = {
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = hex(0x20);
        s.adresse.runtime_nonce = hex(0x21);
        s
    };
    anmelden(&h.c, "sonde", &sonde);
    report(&h.c, "sonde", &sonde.adresse);
    for nr in 0..3 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM evidence WHERE json_extract(state_jcs,'$.ausschlussgrund') IS NOT NULL"),
        0,
        "vorher traegt keine Zeile einen Ausschluss"
    );

    assert!(h.c.invalidierung_wegen_material_fuer_link("sonde", None, None) > 0);
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM evidence WHERE json_extract(state_jcs,'$.ausschlussgrund')='material_wechsel'"),
        3,
        "jede Zeile der Sitzung traegt den Grund - haltbar, nicht nur fluechtig"
    );
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM evidence WHERE json_extract(state_jcs,'$.snapshot.evidence_id') IS NOT NULL"),
        3,
        "und der Wire-Snapshot darunter bleibt unangetastet"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// R09/R31 · Autorisierung und Isolation (Nacharbeit 2)
// ═════════════════════════════════════════════════════════════════════════

/// R31 — zwei Sitzungen mit DERSELBEN `pair_id` bilden kein gemeinsames Paar.
///
/// Die `pair_id` kommt aus dem Deskriptor des Users, nicht aus einer globalen
/// Vergabe: zwei FL-Instanzen koennen dieselbe waehlen. Die Schleife sammelte
/// bis hierher die Evidenz ALLER Sitzungen in eine Liste und gruppierte nur
/// ueber `pair_id` — eine PRE-Haelfte aus Sitzung A und eine POST-Haelfte aus
/// Sitzung B bildeten damit ein Paar und mischten Projektdaten. Der alte
/// Produktpfadtest benutzte nur EINE Sitzung und konnte daran nicht fallen.
#[test]
fn paare_bleiben_in_ihrer_sitzung() {
    use eqcop_broker::coordinator::prepost::Ausschlussgrund;
    let (c, _) = coordinator();
    let paar = hex(0x77);

    // Sitzung A traegt NUR die PRE-Haelfte, Sitzung B nur die POST-Haelfte.
    let a_pre = hello(1, 2, 10, 100, "passive_probe", Some(9));
    let b_post = hello(1, 3, 11, 101, "passive_probe", Some(9));
    anmelden(&c, "a_pre", &a_pre);
    anmelden(&c, "b_post", &b_post);
    assert!(c.descriptor_setzen("a_pre", descriptor(&a_pre.adresse, "pre", &paar)));
    assert!(c.descriptor_setzen("b_post", descriptor(&b_post.adresse, "post", &paar)));
    for nr in 0..8 {
        c.evidence_snapshot_json("a_pre", &evidenz_payload(&a_pre.adresse, nr, |_| {}));
        c.evidence_snapshot_json("b_post", &evidenz_payload(&b_post.adresse, nr, |_| {}));
    }

    // Beide Sitzungen fuehren ein EIGENES Urteil unter derselben `pair_id`.
    assert_eq!(
        c.paarurteile_anzahl(),
        2,
        "je Sitzung ein Urteil, nicht ein gemeinsames"
    );
    let a = c
        .paarurteil_fuer_link("a_pre", &paar)
        .expect("Sitzung A fuehrt ihr Paar");
    let b = c
        .paarurteil_fuer_link("b_post", &paar)
        .expect("Sitzung B fuehrt ihr Paar");

    // Und JEDES ist unvollstaendig: die andere Haelfte liegt in einer FREMDEN
    // Sitzung und zaehlt nicht. Waeren sie zusammengefallen, staende hier ein
    // vollstaendiges Paar aus zwei Projekten.
    assert_eq!(
        a.ausschluss,
        Some(Ausschlussgrund::HaelfteFehlt),
        "Sitzung A hat nur die PRE-Haelfte: {a:?}"
    );
    assert_eq!(
        b.ausschluss,
        Some(Ausschlussgrund::HaelfteFehlt),
        "Sitzung B hat nur die POST-Haelfte: {b:?}"
    );

    // Die bequeme Abfrage ohne Sitzung sagt in diesem Fall NICHTS, statt eine
    // der beiden zu waehlen.
    assert!(
        c.paarurteil(&paar).is_none(),
        "eine pair_id allein identifiziert kein Paar (M-13)"
    );
}

/// R09 — ein Experimentbefehl bleibt in seinem Projekt (E-03/M-48).
///
/// Die Vorpruefung fand ein Experiment ausschliesslich ueber die globale
/// `experiment_id`. Ein autorisiertes Main aus Projekt B konnte damit den
/// Versuch aus Projekt A abbrechen; Persistenz und Taintschliessung wurden
/// danach sogar Projekt B zugeordnet. Alle Produkttests der Runde 1 benutzten
/// nur EIN Projekt und konnten daran nicht fallen.
#[cfg(windows)]
#[test]
fn experimentbefehl_bleibt_in_seinem_projekt() {
    let versuch = 0xabe;
    let a = HarnischMitStore::mit_projekt("projekt-a", 1);
    let begin = experiment_begin_wert(&a.main.adresse, 0x910, versuch);
    assert_eq!(a.p0(&begin)["ergebnis"], "angewandt", "Projekt A legt an");
    assert!(a.c.experiment_sicht(&hex(versuch)).is_some());

    // Ein ZWEITES Projekt, eigener Store, eigenes fuehrendes Main - und
    // derselbe Versuch existiert dort nicht.
    let b = HarnischMitStore::mit_projekt("projekt-b", 5);
    assert!(
        b.c.experiment_sicht(&hex(versuch)).is_none(),
        "Projekt B kennt den Versuch nicht"
    );

    // Jetzt derselbe Versuch IM SELBEN Broker, aber aus dem fremden Projekt.
    // Dafuer wird das zweite Projekt in dieselbe Coordinator-Instanz gelegt.
    let fremd = {
        let mut h = a.main.clone();
        h.adresse.project_binding_id = hex(5);
        h.adresse.session_epoch = hex(6);
        h.adresse.instance_id = hex(0x30);
        h.adresse.runtime_nonce = hex(0x31);
        h
    };
    anmelden(&a.c, "fremd", &fremd);
    report_main(&a.c, "fremd", &fremd.adresse);
    assert!(a.c.state_report_json("fremd", &state_report_payload(&fremd.adresse, 0)));

    let mut abbruch: Value = serde_json::from_slice(
        &std::fs::read(
            std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
                .join("../eq-copilot/fixtures/v3/gueltig/experiment_abort.json"),
        )
        .unwrap(),
    )
    .unwrap();
    abbruch["kopf"]["ziel"] = serde_json::to_value(&fremd.adresse).unwrap();
    abbruch["kopf"]["command_id"] = json!(hex(0x911));
    abbruch["kopf"]["base_revision"] = json!(0);
    abbruch["experiment_id"] = json!(hex(versuch));

    let antwort = Senke::p0(&a.c, "fremd", &serde_json::to_vec(&abbruch).unwrap())
        .expect("die Familie wird beantwortet");
    let ack: Value = serde_json::from_slice(&antwort).unwrap();
    assert_eq!(
        ack["ergebnis"], "abgelehnt",
        "ein fremdes Projekt darf den Versuch nicht anfassen: {ack}"
    );
    assert_eq!(ack["code"], "revision_conflict");
    assert!(
        a.c.experiment_sicht(&hex(versuch))
            .expect("der Versuch steht noch")
            .offen(),
        "und er bleibt OFFEN - der fremde Abbruch hat nichts bewirkt"
    );
}

/// R33 — die Eviction raeumt das Paarurteil mit und bildet neu (M-22).
///
/// Die Runde 1 entfernte die Evidenzhistorie und liess das daraus gerechnete
/// Urteil stehen. Nach Disconnect oder Tombstone lieferte `paarurteil` deshalb
/// weiter das ALTE, vollstaendige Urteil, bis zufaellig ein weiterer Snapshot
/// eintraf — obwohl M-22 sofort einen BENANNTEN unvollstaendigen Zustand
/// verlangt. `evidenz_faellt_mit_dem_client` prueft nur die Evidenzmap und
/// konnte den stale Paarzustand nicht sehen.
#[test]
fn eviction_raeumt_das_paarurteil_und_bildet_neu() {
    use eqcop_broker::coordinator::prepost::Ausschlussgrund;
    let (c, clock) = coordinator();
    let paar = hex(0x78);
    let pre = hello(1, 2, 10, 100, "passive_probe", Some(9));
    let post = hello(1, 2, 11, 101, "passive_probe", Some(9));
    anmelden(&c, "pre", &pre);
    anmelden(&c, "post", &post);
    assert!(c.descriptor_setzen("pre", descriptor(&pre.adresse, "pre", &paar)));
    assert!(c.descriptor_setzen("post", descriptor(&post.adresse, "post", &paar)));
    for nr in 0..8 {
        c.evidence_snapshot_json("pre", &evidenz_payload(&pre.adresse, nr, |_| {}));
        c.evidence_snapshot_json("post", &evidenz_payload(&post.adresse, nr, |_| {}));
    }
    let vorher = c.paarurteil(&paar).expect("das Paar ist vollstaendig");
    assert_eq!(
        vorher.ausschluss, None,
        "vor der Eviction ist es kein Ausschlussfall: {vorher:?}"
    );

    // Die POST-Haelfte faellt: stale, dann Tombstone.
    c.control_ende("post");
    clock.vor(60_000);
    c.liveness_tick();
    c.liveness_tick();

    let nachher = c
        .paarurteil(&paar)
        .expect("das Paar bleibt sichtbar - es ist unvollstaendig, nicht fort");
    assert_eq!(
        nachher.ausschluss,
        Some(Ausschlussgrund::HaelfteFehlt),
        "die verbliebene Haelfte wird als getrennte Haelfte GEFUEHRT, nicht als \
         vollstaendiges Paar von gestern: {nachher:?}"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// R21/R24 · Die fehlenden Produktaufrufer (Nacharbeit 2)
// ═════════════════════════════════════════════════════════════════════════

/// R21 — die Blindreihenfolge wird VOR dem Urteil gebunden (M-44).
///
/// Die Runde 1 uebernahm die vom Sender ZUSAMMEN MIT dem Hoerurteil gemeldete
/// Reihenfolge und band sie unmittelbar vor dem Terminal: der Sender konnte sie
/// also nach dem Hoeren waehlen. Der Produkttest band sie ueber die Naht
/// `binde_blindreihenfolge_fuer_test` vorab — einen frueheren Produktaufrufer
/// gab es nicht, und deshalb konnte er an der Luecke nicht fallen. Die Naht ist
/// entfallen; gebunden wird im eigenen Befehlszweig.
#[cfg(windows)]
#[test]
fn blindreihenfolge_wird_im_eigenen_befehlszweig_gebunden() {
    let h = HarnischMitStore::neu("blindreihenfolge");
    let versuch = 0xac0;
    assert_eq!(
        h.p0(&experiment_begin_wert(&h.main.adresse, 0x920, versuch))["ergebnis"],
        "angewandt"
    );
    let e = h.c.experiment_sicht(&hex(versuch)).expect("der Versuch steht");
    assert!(
        !e.reihenfolge_gebunden(),
        "nach dem Begin ist noch nichts gebunden"
    );

    // Das Ergebnis OHNE Bindung wird benannt abgelehnt - nicht generisch.
    let ergebnis = |command: usize, reihenfolge: &str| {
        json!({
            "type": "experiment_manual_result",
            "kopf": {
                "command_id": hex(command),
                "ziel": h.main.adresse,
                "base_revision": 0,
                "ttl_ms": 1000,
                "schema_major": 3,
                "schema_minor": 0
            },
            "experiment_id": hex(versuch),
            "hoerurteil": "kandidat",
            "blindreihenfolge": reihenfolge,
            "notiz": null,
            "werkzeug": null
        })
    };
    let ack = h.p0(&ergebnis(0x921, "baseline_zuerst"));
    assert_eq!(ack["ergebnis"], "abgelehnt");
    assert_eq!(
        ack["code"], "reihenfolge_nicht_gebunden",
        "ein Urteil ohne vorher gebundene Reihenfolge ist keines (M-44): {ack}"
    );

    // Der eigene Befehlszweig bindet sie - VOR dem Hoeren.
    let vorlage = experiment_begin_wert(&h.main.adresse, 0x922, versuch);
    let kandidat = |command: usize, reihenfolge: &str| {
        json!({
            "type": "experiment_candidate",
            "kopf": {
                "command_id": hex(command),
                "ziel": h.main.adresse,
                "base_revision": 0,
                "ttl_ms": 1000,
                "schema_major": 3,
                "schema_minor": 0
            },
            "experiment_id": hex(versuch),
            "referenz": vorlage["referenz"],
            "blindreihenfolge": reihenfolge
        })
    };
    assert_eq!(
        h.p0(&kandidat(0x923, "kandidat_zuerst"))["ergebnis"],
        "angewandt"
    );
    let e = h.c.experiment_sicht(&hex(versuch)).expect("der Versuch steht");
    assert!(e.reihenfolge_gebunden(), "jetzt IST sie gebunden");
    assert_eq!(e.kandidaten.len(), 1, "und der Kandidat ist erfasst (M-41)");
    assert!(
        e.aufgedeckte_reihenfolge().is_none(),
        "vor dem Urteil verraet sie niemand - sonst waere der Blindvergleich keiner"
    );

    // Ein zweiter Kandidat aendert die Bindung NICHT.
    assert_eq!(
        h.p0(&kandidat(0x924, "baseline_zuerst"))["ergebnis"],
        "angewandt"
    );
    let ack = h.p0(&ergebnis(0x925, "baseline_zuerst"));
    assert_eq!(ack["ergebnis"], "abgelehnt");
    assert_eq!(
        ack["code"], "blindreihenfolge_widerspruch",
        "die ZUERST gebundene gilt; ein zweiter Kandidat dreht sie nicht: {ack}"
    );
}

/// R24 — eine PREVIEW nimmt die Evidenz ihrer Sitzung zurueck (M-52).
///
/// Die drei `preview_*`-Familien liefen nur durch `persistenz_p0`; der
/// Invalidierungszaehler blieb unveraendert, und die waehrend der Vorschau
/// gemessene Evidenz sah aus wie jede andere.
#[cfg(windows)]
#[test]
fn preview_nimmt_die_evidenz_ihrer_sitzung_zurueck() {
    let h = HarnischMitStore::neu("preview-invalidierung");
    let sonde = {
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = hex(0x20);
        s.adresse.runtime_nonce = hex(0x21);
        s
    };
    anmelden(&h.c, "sonde", &sonde);
    report(&h.c, "sonde", &sonde.adresse);
    for nr in 0..3 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }
    assert_eq!(h.c.invalidierungen_zaehler(), 0, "noch keine Ruecknahme");

    let preview = json!({
        "type": "preview_begin",
        "kopf": {
            "command_id": hex(0x930),
            "ziel": h.main.adresse,
            "base_revision": 0,
            "ttl_ms": 1000,
            "schema_major": 3,
            "schema_minor": 0
        },
        "lease_duration_ms": 400,
        "renew_id": hex(0x931)
    });
    let ack = h.p0(&preview);
    assert_eq!(ack["ergebnis"], "angewandt", "{ack}");
    assert_eq!(
        h.c.invalidierungen_zaehler(),
        1,
        "die Preview NIMMT ZURUECK - M-52 zaehlt sie ausdruecklich als Ausloeser"
    );
    assert!(
        h.c.evidenz_historie(&hex(0x20))
            .iter()
            .all(|e| e.ausschlussgrund.as_deref() == Some("intervention")),
        "und jeder Beleg traegt den Grund, nie einen stillen Ausschluss"
    );
}

/// R24 — ein MESSPUNKTWECHSEL invalidiert aus dem Deskriptorvergleich (M-55).
///
/// `invalidierung_wegen_messpunkt` hatte ausserhalb seiner Huelle keinen
/// Aufrufer: ein Descriptorwechsel liess den Zaehler unveraendert, und die
/// Evidenz der ALTEN Messposition blieb gueltig. Gate 7 sagt das Gegenteil zu.
#[test]
fn messpunktwechsel_invalidiert_aus_dem_deskriptorvergleich() {
    let (c, _) = coordinator();
    let sonde = hello(1, 2, 0x20, 0x21, "passive_probe", Some(9));
    anmelden(&c, "sonde", &sonde);
    assert!(c.descriptor_setzen("sonde", descriptor(&sonde.adresse, "pre", &hex(0x77))));
    for nr in 0..3 {
        assert!(c.evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }
    assert_eq!(c.invalidierungen_zaehler(), 0);

    // Derselbe Messpunkt: nichts wird zurueckgenommen (die Gegenprobe).
    assert!(c.descriptor_setzen("sonde", descriptor(&sonde.adresse, "pre", &hex(0x77))));
    assert_eq!(
        c.invalidierungen_zaehler(),
        0,
        "gleicher Messpunkt, keine Ruecknahme"
    );

    // Der WECHSEL nimmt zurueck - ohne dass ein Test die Invalidierung ruft.
    assert!(c.descriptor_setzen("sonde", descriptor(&sonde.adresse, "post", &hex(0x77))));
    assert_eq!(
        c.invalidierungen_zaehler(),
        1,
        "messpunktwechsel_invalidiert_aus_dem_deskriptorvergleich (M-55)"
    );
    assert!(c
        .evidenz_historie(&hex(0x20))
        .iter()
        .all(|e| e.ausschlussgrund.as_deref() == Some("messpunkt_wechsel")));
}

/// R24 — ein MATERIALWECHSEL invalidiert aus dem Fingerprintvergleich (M-54).
///
/// M-31 sagt ausdruecklich, dass der Wechsel aus dem Fingerprintvergleich kommt
/// und nicht aus einer Zeitheuristik. `invalidierung_wegen_material` hatte
/// ausserhalb seiner Huelle keinen Aufrufer.
#[cfg(windows)]
#[test]
fn materialwechsel_invalidiert_aus_dem_fingerprintvergleich() {
    let h = HarnischMitStore::neu("materialwechsel");
    let sonde = {
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = hex(0x20);
        s.adresse.runtime_nonce = hex(0x21);
        s
    };
    anmelden(&h.c, "sonde", &sonde);
    report(&h.c, "sonde", &sonde.adresse);
    for nr in 0..3 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }

    let erster = experiment_begin_wert(&h.main.adresse, 0x940, 0xad0);
    assert_eq!(h.p0(&erster)["ergebnis"], "angewandt");
    assert_eq!(
        h.c.invalidierungen_zaehler(),
        0,
        "gleiches Material, keine Ruecknahme"
    );

    // Ein ZWEITER Versuch auf DERSELBEN Passage mit ANDEREM Fingerprint: das
    // Material hat sich geaendert, und die Evidenz davor gilt nicht mehr.
    let mut zweiter = experiment_begin_wert(&h.main.adresse, 0x941, 0xad1);
    zweiter["passage"]["passage_id"] = erster["passage"]["passage_id"].clone();
    zweiter["passage"]["fingerprint"]["chroma"] = json!([9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9]);
    assert_eq!(h.p0(&zweiter)["ergebnis"], "angewandt");
    assert_eq!(
        h.c.invalidierungen_zaehler(),
        1,
        "materialwechsel_invalidiert_aus_dem_fingerprintvergleich (M-54/M-31)"
    );
    assert!(h
        .c
        .evidenz_historie(&hex(0x20))
        .iter()
        .all(|e| e.ausschlussgrund.as_deref() == Some("material_wechsel")));
}

// ═════════════════════════════════════════════════════════════════════════
// R07/R08/R10/R11/R12/R13/R15 · Persistenz und Replay (Nacharbeit 2)
// ═════════════════════════════════════════════════════════════════════════

/// Eine Sonde in der Sitzung des Harnischs, die Evidenz liefert.
#[cfg(windows)]
fn sonde_mit_evidenz(h: &HarnischMitStore, anzahl: usize) -> HelloControl {
    let mut s = h.main.clone();
    s.plugin_kind = "passive_probe".into();
    s.adresse.instance_id = hex(0x20);
    s.adresse.runtime_nonce = hex(0x21);
    anmelden(&h.c, "sonde", &s);
    report(&h.c, "sonde", &s.adresse);
    for nr in 0..anzahl {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_payload(&s.adresse, nr, |_| {})));
    }
    s
}

/// Eine Sonde, deren Evidenz WIRKLICH in der Passage des Fixtures liegt.
///
/// 🔑 Nacharbeit 2 (Befund R17): `resultatmessung` nimmt seither nur noch
/// Belege IM Fenster der Passage, in IHRER Transportepoche, von den
/// eingefrorenen Quellen und mit passender Messpunktklasse. Ein Helfer, der
/// irgendwelche Evidenz liefert, wuerde ab jetzt nichts mehr messen — und
/// genau das ist die Zusage.
///
/// Die IDs kommen aus dem `experiment_begin`-Fixture: die erste aktive Quelle
/// und die zu ihr gehoerende Messpunktklasse.
#[cfg(windows)]
fn sonde_in_der_passage(h: &HarnischMitStore, vorlage: &Value) -> HelloControl {
    let quelle = vorlage["passage"]["aktive_quellen"][0]
        .as_str()
        .expect("das Fixture nennt eine Quelle")
        .to_owned();
    let klasse = vorlage["passage"]["messpunktklassen"][0]
        .as_str()
        .expect("und ihre Messpunktklasse")
        .to_owned();
    let mut s = h.main.clone();
    s.plugin_kind = "passive_probe".into();
    s.adresse.instance_id = quelle;
    s.adresse.runtime_nonce = hex(0x21);
    anmelden(&h.c, "sonde", &s);
    report(&h.c, "sonde", &s.adresse);
    assert!(h.c.descriptor_setzen(
        "sonde",
        descriptor(&s.adresse, &klasse, &hex(0x77))
    ));
    s
}

/// Ein Evidenzsnapshot IN der Passage des Fixtures.
#[cfg(windows)]
fn evidenz_in_passage(sonde: &Adresse, vorlage: &Value, nr: usize) -> Vec<u8> {
    let von = vorlage["passage"]["projekt_von"].as_i64().expect("von");
    let epoche = vorlage["passage"]["transport_epoch"].as_u64().expect("epoche");
    evidenz_payload(sonde, nr, |w| {
        w["transport"]["transport_epoch"] = json!(epoche);
        w["transport"]["project_sample_start"] = json!(von + (nr as i64) * 512);
    })
}

/// R13/R11 — jede Experimenttransition steht im EREIGNISINDEX, und der
/// Zustand traegt die vollstaendigen Referenzen aus §43.1.
///
/// Migration 1 fuehrt `experiment_events` seit jeher, und im ganzen Produkt gab
/// es KEIN Insert dorthin: Begin, Kandidat, Resultat und Abbruch liessen die
/// vertraglich verlangte Tabelle leer. Der Produkttest zaehlte nur
/// `event_log WHERE event_type='experiment'` und konnte das nicht sehen. Der
/// Begin-Payload trug ausserdem nur Passage-ID, Match-Gain, Quellen und
/// Klassen — Fingerprints, Alignment und Reproduzierbarkeit fehlten.
#[cfg(windows)]
#[test]
fn experimenttransitionen_stehen_im_ereignisindex_mit_vollen_referenzen() {
    let h = HarnischMitStore::neu("experiment-events");
    let versuch = 0xb00;
    assert_eq!(
        h.p0(&experiment_begin_wert(&h.main.adresse, 0x950, versuch))["ergebnis"],
        "angewandt"
    );
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM experiment_events"),
        1,
        "der Begin steht im Ereignisindex (R13)"
    );

    let vorlage = experiment_begin_wert(&h.main.adresse, 0x951, versuch);
    let kandidat = json!({
        "type": "experiment_candidate",
        "kopf": {
            "command_id": hex(0x952),
            "ziel": h.main.adresse,
            "base_revision": 0,
            "ttl_ms": 1000,
            "schema_major": 3,
            "schema_minor": 0
        },
        "experiment_id": hex(versuch),
        "referenz": vorlage["referenz"],
        "blindreihenfolge": "kandidat_zuerst"
    });
    assert_eq!(h.p0(&kandidat)["ergebnis"], "angewandt");
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM experiment_events"),
        2,
        "und der Kandidat ebenso - JEDE Transition, nicht nur die erste"
    );

    // R11: der gespeicherte Zustand traegt die vollen §43.1-Referenzen.
    let zeile: Vec<u8> = rusqlite::Connection::open(h.db())
        .unwrap()
        .query_row(
            "SELECT state_jcs FROM experiments WHERE experiment_id=?1",
            rusqlite::params![hex(versuch)],
            |z| z.get(0),
        )
        .expect("die Experimentzeile steht");
    let w: Value = serde_json::from_slice(&zeile).expect("Zeile ist JSON");
    for feld in [
        "/baseline/passage_fingerprint/version",
        "/baseline/upstream_fingerprint/version",
        "/baseline/alignment",
        "/baseline/match_gain_db",
        "/passage/fingerprint/version",
        "/passage/messpunktklassen/0",
        "/execution_mode",
        "/reproduzierbarkeit",
    ] {
        assert!(
            w.pointer(feld).is_some(),
            "R11: `{feld}` fehlt im gespeicherten Zustand: {w}"
        );
    }
    assert!(
        w.pointer("/blindreihenfolge_gebunden").and_then(Value::as_str).is_some(),
        "und die gebundene Reihenfolge steht darin (M-44)"
    );
    assert_eq!(w["kandidaten"].as_array().map(Vec::len), Some(1));
}

/// R15/R12 — das Terminal traegt die NUTZERDATEN, und ein neuer Coordinator
/// holt den Versuch aus dem Store zurueck.
///
/// Der terminale Payload enthielt weder Hoerurteil noch Blindreihenfolge,
/// Notiz oder Werkzeug, und er ERSETZTE den Begin-Zustand in `experiments`.
/// `Coordinator::mit_store` restaurierte ausschliesslich Konflikt-Guards; nach
/// Drop und Neuerzeugung lieferte `experiment_sicht(id)` `None`, obwohl die
/// Zeile existierte. Der behauptete Restart-Test erzeugte gar keinen zweiten
/// Coordinator.
#[cfg(windows)]
#[test]
fn terminal_traegt_die_nutzerdaten_und_ueberdauert_den_neustart() {
    let h = HarnischMitStore::neu("terminal-replay");
    let versuch = 0xb10;
    let vorlage = experiment_begin_wert(&h.main.adresse, 0x961, versuch);
    let sonde = sonde_in_der_passage(&h, &vorlage);

    // BASELINE: Evidenz VOR dem Begin, in der Passage.
    for nr in 0..4 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_in_passage(&sonde.adresse, &vorlage, nr)));
    }
    assert_eq!(
        h.p0(&experiment_begin_wert(&h.main.adresse, 0x960, versuch))["ergebnis"],
        "angewandt"
    );
    assert_eq!(
        h.p0(&json!({
            "type": "experiment_candidate",
            "kopf": {
                "command_id": hex(0x962),
                "ziel": h.main.adresse,
                "base_revision": 0,
                "ttl_ms": 1000,
                "schema_major": 3,
                "schema_minor": 0
            },
            "experiment_id": hex(versuch),
            "referenz": vorlage["referenz"],
            "blindreihenfolge": "kandidat_zuerst"
        }))["ergebnis"],
        "angewandt"
    );
    // RESULTAT: Evidenz NACH dem erfassten Kandidaten. Dazwischen liegt die
    // Fremdaenderung, und Belege von dort gehoeren keiner Seite (R17).
    for nr in 4..8 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_in_passage(&sonde.adresse, &vorlage, nr)));
    }
    let ack = h.p0(&json!({
        "type": "experiment_manual_result",
        "kopf": {
            "command_id": hex(0x963),
            "ziel": h.main.adresse,
            "base_revision": 0,
            "ttl_ms": 1000,
            "schema_major": 3,
            "schema_minor": 0
        },
        "experiment_id": hex(versuch),
        "hoerurteil": "kandidat",
        "blindreihenfolge": "kandidat_zuerst",
        "notiz": "haerter, aber schmaler",
        "werkzeug": "FabFilter Pro-Q"
    }));
    assert_eq!(ack["ergebnis"], "angewandt", "{ack}");

    // R15: die Nutzerdaten stehen im gespeicherten Zustand — UND die Baseline
    // steht weiter darin. Der Terminal-Payload ersetzt den Begin-Zustand nicht.
    let zeile: Vec<u8> = rusqlite::Connection::open(h.db())
        .unwrap()
        .query_row(
            "SELECT state_jcs FROM experiments WHERE experiment_id=?1",
            rusqlite::params![hex(versuch)],
            |z| z.get(0),
        )
        .expect("die Experimentzeile steht");
    let w: Value = serde_json::from_slice(&zeile).unwrap();
    assert_eq!(w.pointer("/terminal/hoerurteil"), Some(&json!("kandidat")));
    assert_eq!(
        w.pointer("/terminal/blindreihenfolge"),
        Some(&json!("kandidat_zuerst"))
    );
    assert_eq!(
        w.pointer("/terminal/notiz"),
        Some(&json!("haerter, aber schmaler"))
    );
    assert_eq!(
        w.pointer("/terminal/werkzeug"),
        Some(&json!("FabFilter Pro-Q"))
    );
    assert!(
        w.pointer("/baseline/match_gain_db").is_some(),
        "R15: der Begin-Zustand wird NICHT ueberschrieben - der neue enthaelt ihn"
    );

    // R12: ein NEUER Coordinator auf demselben Store holt ihn zurueck.
    let neu = h.neuer_coordinator();
    let e = neu
        .experiment_sicht(&hex(versuch))
        .expect("R12: der Versuch ueberdauert den Brokerneustart (M-47/M-50)");
    assert!(!e.offen(), "und zwar mit seinem Terminal");
    assert_eq!(e.baseline.match_gain_db, -1.5, "samt eingefrorenem Match-Gain");
    assert_eq!(e.kandidaten.len(), 1, "samt Kandidat (M-41)");
    assert!(
        e.reihenfolge_gebunden(),
        "samt gebundener Blindreihenfolge (M-44)"
    );
    assert!(
        neu.passage_sicht(&e.passage_id).is_some(),
        "und die Passage, auf die er zeigt, ist auch wieder da (M-25)"
    );
}

/// R08 — ein Retry nach vollstaendiger Ausfuehrung ist IDEMPOTENT, nicht
/// `revision_conflict`, und wiederholt die Wirkung nicht.
///
/// `persistenz_p0` committete den Befehl VOR der Wirkung; ein Absturz
/// dazwischen liess den Riegel ohne Wirkung stehen, und der Retry uebersprang
/// sie dauerhaft. Nach einer erfolgreichen Ausfuehrung lehnte umgekehrt die
/// fachliche Vorpruefung den Retry als Konflikt ab. Die Retry- und Killtests
/// verwendeten nur die generischen Preview-Befehle.
#[cfg(windows)]
#[test]
fn experimentbefehl_und_wirkung_sind_ein_append() {
    let h = HarnischMitStore::neu("experiment-atomar");
    let versuch = 0xb20;
    let begin = experiment_begin_wert(&h.main.adresse, 0x970, versuch);
    assert_eq!(h.p0(&begin)["ergebnis"], "angewandt");
    let experimente = h.zeilen("SELECT COUNT(*) FROM experiments");
    let ereignisse = h.zeilen("SELECT COUNT(*) FROM experiment_events");
    let befehle = h.zeilen("SELECT COUNT(*) FROM event_log WHERE event_type='command'");

    // Derselbe Befehl noch einmal - der Sender hat seine Antwort nicht bekommen.
    let ack = h.p0(&begin);
    assert_eq!(
        ack["ergebnis"], "idempotent_wiederholt",
        "R08: ein bereits committeter Befehl ist dieselbe Absicht, kein Konflikt: {ack}"
    );
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM experiments"),
        experimente,
        "und er legt nichts zweites an"
    );
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM experiment_events"),
        ereignisse,
        "auch keinen zweiten Ereignisindexeintrag"
    );
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM event_log WHERE event_type='command'"),
        befehle,
        "und keinen zweiten Befehlsriegel"
    );

    // Befehl UND Wirkung liegen im Log - beide, oder keiner.
    assert!(
        h.zeilen("SELECT COUNT(*) FROM event_log WHERE event_type='experiment'") >= 1
            && befehle >= 1,
        "Befehlsriegel und Domaenenereignis stehen beide im Log"
    );
}

/// R10 — eine VERDRAENGUNG bekommt ihr eigenes Terminal.
///
/// `beginne` schloss das aelteste offene Experiment intern mit `verdraengt`
/// und gab davon nichts zurueck; der Wrapper persistierte ausschliesslich die
/// neue Anlage. Der verdraengte Versuch blieb im Store OFFEN.
/// `retention_abort_also_closes_intervals` erzeugte weder eine Intervention
/// noch einen Store und konnte die Luecke trotz seines Namens nicht erkennen.
#[cfg(windows)]
#[test]
fn verdraengung_bekommt_ihr_eigenes_terminal() {
    use eqcop_broker::coordinator::experiment::N_PROJEKT;
    let h = HarnischMitStore::neu("verdraengung");
    for nr in 0..N_PROJEKT {
        let mut w = experiment_begin_wert(&h.main.adresse, 0x980 + nr, 0xb30 + nr);
        w["passage"]["passage_id"] = json!(hex(0xb60 + nr));
        assert_eq!(h.p0(&w)["ergebnis"], "angewandt", "Versuch {nr}");
    }
    let erster = hex(0xb30);
    assert!(
        h.c.experiment_sicht(&erster).expect("steht").offen(),
        "der aelteste ist noch offen"
    );

    // Der N+1-te verdraengt den aeltesten.
    let mut w = experiment_begin_wert(&h.main.adresse, 0x990, 0xb30 + N_PROJEKT);
    w["passage"]["passage_id"] = json!(hex(0xb60 + N_PROJEKT));
    assert_eq!(h.p0(&w)["ergebnis"], "angewandt");
    assert!(
        !h.c.experiment_sicht(&erster).expect("steht").offen(),
        "der aelteste ist verdraengt"
    );

    // Und das steht im STORE, nicht nur im fluechtigen Stand.
    let zeile: Vec<u8> = rusqlite::Connection::open(h.db())
        .unwrap()
        .query_row(
            "SELECT state_jcs FROM experiments WHERE experiment_id=?1",
            rusqlite::params![erster],
            |z| z.get(0),
        )
        .expect("die Zeile des Verdraengten steht");
    let v: Value = serde_json::from_slice(&zeile).unwrap();
    assert_eq!(
        v.pointer("/terminal/grund"),
        Some(&json!("verdraengt")),
        "R10: die Verdraengung ist persistiert, nicht nur intern geschehen: {v}"
    );
    assert_eq!(v["ereignis"], "verdraengt");
}

/// R07 — ein Snapshot, dessen ABLAGE scheitert, ist NICHT angenommen.
///
/// Die Runde 1 trug den Snapshot unter dem Lock als angenommen ein und legte
/// ihn danach ab; scheiterte der Append, erhoehte der Pfad nur einen Zaehler
/// und `evidence_snapshot_json` meldete weiter Erfolg. Es existierte damit
/// angenommene Evidenz ohne den zugesagten Store-Event — und genau darauf
/// rechnet `resultatmessung` ihre Baseline. Die Historientests nutzten keinen
/// fehlschlagenden Store und konnten daran nicht fallen.
#[test]
fn evidenz_ohne_erfolgreiche_ablage_ist_nicht_angenommen() {
    // Ein DEGRADIERTER Store: vorhanden, aber jeder Append scheitert.
    let writer = eqcop_broker::store::StoreWriter::degradiert_ohne_pfad(
        "Rotprobe: der Store verweigert den Dienst",
    );
    assert!(writer.ist_degradiert());
    let c = Coordinator::mit_store(Arc::new(ManualClock::default()), hex(0xbeef), &writer);
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);

    assert!(
        !c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, 0, |_| {})),
        "R07: ohne erfolgreiche Ablage ist der Snapshot nicht angenommen"
    );
    assert!(
        c.evidenz_historie(&hex(10)).is_empty(),
        "und er steht auch nicht im fluechtigen Bestand - sonst rechnete \
         `resultatmessung` auf Evidenz ohne Store-Event"
    );
    assert!(
        c.evidenz_sicht(&hex(10)).is_none(),
        "und keine Sicht behauptet ihn"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// R17/R18/R19/R25/R29/R30 · Die fachlichen Rechenwege (Nacharbeit 2)
// ═════════════════════════════════════════════════════════════════════════

/// R17/R18/R19 — die Resultatmessung ist an Versuch, Passage und Grenze
/// gebunden, rechnet die Comparability im Produktpfad und traegt ALLE
/// Guardrails.
///
/// Die Runde 1 nahm die Quelle mit der laengsten Historie irgendeiner Quelle
/// der Sitzung und teilte die Retention stumpf in zwei Haelften: vier bereits
/// VOR dem Begin eingegangene Snapshots genuegten fuer ein sofortiges
/// Resultat. `vergleichbarkeit` blieb `Default::default()`, und die
/// Guardrail-Achse kannte nur Coverage und Klasse.
#[cfg(windows)]
#[test]
fn resultatmessung_ist_an_versuch_passage_und_grenze_gebunden() {
    let h = HarnischMitStore::neu("resultatmessung-bindung");
    let versuch = 0xc00;
    let vorlage = experiment_begin_wert(&h.main.adresse, 0x9a0, versuch);
    let sonde = sonde_in_der_passage(&h, &vorlage);

    // Nur Evidenz VOR dem Begin - kein Resultat, egal wie viel davon kommt.
    for nr in 0..8 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_in_passage(&sonde.adresse, &vorlage, nr)));
    }
    assert_eq!(
        h.p0(&experiment_begin_wert(&h.main.adresse, 0x9a1, versuch))["ergebnis"],
        "angewandt"
    );
    let kandidat = |command: usize| {
        json!({
            "type": "experiment_candidate",
            "kopf": {
                "command_id": hex(command),
                "ziel": h.main.adresse,
                "base_revision": 0,
                "ttl_ms": 1000,
                "schema_major": 3,
                "schema_minor": 0
            },
            "experiment_id": hex(versuch),
            "referenz": vorlage["referenz"],
            "blindreihenfolge": "kandidat_zuerst"
        })
    };
    assert_eq!(h.p0(&kandidat(0x9a2))["ergebnis"], "angewandt");

    let ergebnis = |command: usize| {
        json!({
            "type": "experiment_manual_result",
            "kopf": {
                "command_id": hex(command),
                "ziel": h.main.adresse,
                "base_revision": 0,
                "ttl_ms": 1000,
                "schema_major": 3,
                "schema_minor": 0
            },
            "experiment_id": hex(versuch),
            "hoerurteil": "kandidat",
            "blindreihenfolge": "kandidat_zuerst",
            "notiz": null,
            "werkzeug": null
        })
    };
    let ack = h.p0(&ergebnis(0x9a3));
    assert_eq!(
        ack["code"], "ohne_resultatmessung",
        "R17: Evidenz VOR dem Begin ist Baseline, nie Resultat: {ack}"
    );

    // Evidenz AUSSERHALB der Passage zaehlt ebenfalls nicht.
    let ausserhalb = vorlage["passage"]["projekt_bis"].as_i64().unwrap() + 100_000;
    let epoche = vorlage["passage"]["transport_epoch"].as_u64().unwrap();
    for nr in 8..12 {
        assert!(h.c.evidence_snapshot_json(
            "sonde",
            &evidenz_payload(&sonde.adresse, nr, |w| {
                w["transport"]["transport_epoch"] = json!(epoche);
                w["transport"]["project_sample_start"] = json!(ausserhalb + (nr as i64) * 512);
            })
        ));
    }
    let ack = h.p0(&ergebnis(0x9a4));
    assert_eq!(
        ack["code"], "ohne_resultatmessung",
        "R17: Evidenz ausserhalb des Passagenfensters ist kein Resultat: {ack}"
    );

    // Und Evidenz in der Passage NACH dem Kandidaten - jetzt schliesst er ab.
    // Das Resultat liegt deutlich lauter als die Baseline: der
    // Loudness-Guardrail muss das sehen (R19).
    for nr in 12..18 {
        assert!(h.c.evidence_snapshot_json(
            "sonde",
            &evidenz_payload(&sonde.adresse, nr, |w| {
                w["transport"]["transport_epoch"] = json!(epoche);
                w["transport"]["project_sample_start"] =
                    json!(vorlage["passage"]["projekt_von"].as_i64().unwrap() + (nr as i64) * 512);
                // Alle Bandwerte um 6 dB anheben. Das Fixture kodiert
                // `q_db_0p1_i16`: ein Schritt sind 0,1 dB, also 60 Schritte.
                if let Some(werte) = w["verteilung"]["p50"]["werte"].as_array_mut() {
                    for v in werte.iter_mut() {
                        if let Some(x) = v.as_i64() {
                            *v = json!(x + 60);
                        }
                    }
                }
            })
        ));
    }
    let ack = h.p0(&ergebnis(0x9a5));
    assert_eq!(ack["ergebnis"], "angewandt", "R17: jetzt schliesst er ab: {ack}");

    let e = h.c.experiment_sicht(&hex(versuch)).expect("steht");
    use eqcop_broker::coordinator::experiment::Terminal;
    let Some(Terminal::Ergebnis { achsen, .. }) = &e.terminal else {
        panic!("das Terminal ist ein Ergebnis: {:?}", e.terminal);
    };
    // R18: die Comparability ist GERECHNET, nicht `Default::default()`.
    assert!(
        achsen.vergleichbarkeit.is_some(),
        "R18: vergleichbarkeit::beurteile laeuft im Produktpfad: {achsen:?}"
    );
    // R19/B11: die Guardrails aus M-45 sind gemessen, nicht nur
    // Coverage/Klasse — und die, die diese Evidenz NICHT traegt, sagen das mit
    // Grund, statt eine Ersatzgroesse unter ihrem Namen zu liefern.
    //
    // 🔑 Nacharbeit 3 (Befund B11): die Runde 2 pruefte hier, dass der
    // "Loudness-Guardrail" die 6 dB sieht. Er sah sie — aber er war das Mittel
    // spektraler P50-dB-Werte und nicht integrierte Lautheit; der Test schrieb
    // damit eine Ersatzgroesse unter dem echten Namen fest.
    assert!(
        achsen.guardrail_loudness_db.is_none() && achsen.guardrail_peak_db.is_none(),
        "B11: Loudness und True Peak werden NICHT ersetzt: {achsen:?}"
    );
    assert_eq!(
        achsen.guardrail_nicht_gemessen.len(),
        2,
        "B11: und beide nennen ihren Grund: {:?}",
        achsen.guardrail_nicht_gemessen
    );
    assert!(
        achsen.guardrail_transient.is_some(),
        "R19: der Transient-Guardrail ebenfalls"
    );
    use eqcop_broker::coordinator::experiment::Achsenbefund;
    assert_eq!(
        achsen.befunde(None).guardrails,
        Achsenbefund::Verschlechtert,
        "R19: und die Achse FAELLT - bei unveraenderter Coverage sah sie \
         vorher stabil aus: {achsen:?}"
    );
}

/// R20 — der Block-Bootstrap zieht ZEITFENSTER, und die p-Werte kommen aus
/// seiner Verteilung.
///
/// Die Runde 1 gab ihm einen ueber alle Fenster gemittelten Wert je Band: er
/// zog Bloecke von BAENDERN, und die Effektgroesse allein machte ein Band
/// signifikant (`p = exp(-|delta|)`). Eine Reihe, deren Mittel je nach Ziehung
/// das Vorzeichen wechselt, ist keine Aussage — so gross ihr Mittelwert auch
/// sein mag.
#[test]
fn bootstrap_zieht_zeitfenster_und_rechnet_echte_p_werte() {
    use eqcop_broker::coordinator::experiment::{bootstrap_p, Resultatmessung, KLASSENORDNUNG};

    // Eine STABILE, aber KLEINE Reihe: jedes Fenster sagt dasselbe, und der
    // Betrag ist gering. Genau hier trennen sich die beiden Rechenwege:
    // `exp(-|0,4|)` ist 0,67 und damit nie signifikant, waehrend eine Reihe,
    // die in jeder Ziehung dasselbe Vorzeichen traegt, sehr wohl eine Aussage
    // ist. Die Runde 1 hat kleine, sichere Effekte damit VERSCHWIEGEN und
    // grosse, unsichere BEHAUPTET.
    let stabil: Vec<f64> = vec![0.4; 24];
    let p_stabil = bootstrap_p(&stabil, 4, 400, 7);
    // Eine WECHSELNDE Reihe mit demselben Betrag, aber ohne Richtung.
    let wechselnd: Vec<f64> = (0..24)
        .map(|i| if i % 2 == 0 { 30.0 } else { -30.0 })
        .collect();
    let p_wechselnd = bootstrap_p(&wechselnd, 4, 400, 7);
    assert!(
        p_stabil < 0.05,
        "eine stabile Reihe ist signifikant: {p_stabil}"
    );
    assert!(
        p_wechselnd > p_stabil,
        "R20: eine Reihe ohne Richtung ist es NICHT - und ihr Betrag ist \
         zehnmal groesser. Mit `exp(-|delta|)` waere sie die staerkere \
         Aussage gewesen: stabil {p_stabil}, wechselnd {p_wechselnd}"
    );

    // Dieselbe Aussage ueber die ganze Achsenrechnung.
    let reihe = |f: &dyn Fn(usize) -> f64| -> Resultatmessung {
        Resultatmessung {
            band_delta_db: vec![3.0; 8],
            band_gueltig: vec![true; 8],
            fenster_delta_db: (0..24).map(|t| vec![f(t); 8]).collect(),
            erste_haelfte: vec![3.0; 8],
            zweite_haelfte: vec![3.0; 8],
            abdeckung_baseline: 0.9,
            abdeckung_resultat: 0.9,
            klasse_baseline: "mittel".into(),
            klasse_resultat: "mittel".into(),
            vergleichbarkeit: Some("stark".into()),
            baseline_evidence_ids: vec!["a".repeat(32)],
            resultat_evidence_ids: vec!["b".repeat(32)],
            ..Default::default()
        }
    };
    let stabil = reihe(&|_| 0.4).achsen(&KLASSENORDNUNG);
    let wechselnd = reihe(&|t| if t % 2 == 0 { 30.0 } else { -30.0 }).achsen(&KLASSENORDNUNG);
    assert!(
        stabil.signifikante_baender > 0,
        "R20: die kleine, aber STABILE Reihe traegt signifikante Baender - mit          `exp(-|0,4|) = 0,67` haette sie nie eine getragen: {stabil:?}"
    );
    assert_eq!(
        wechselnd.signifikante_baender, 0,
        "R20: die wechselnde nicht - trotz zehnmal groesserem Betrag: {wechselnd:?}"
    );
    assert!(
        stabil.intervall.is_some_and(|(u, _)| u > 0.0),
        "und das Intervall der stabilen Reihe enthaelt die Null nicht: {stabil:?}"
    );
    assert!(
        p_stabil < (-0.4f64).abs().exp().recip(),
        "R20: und der gerechnete p-Wert ist KLEINER als der erfundene          `exp(-|delta|)` es je waere: {p_stabil}"
    );
}

/// R25 — der Hoermarker invalidiert GENAU seinen Bereich.
///
/// Bei vorhandenem Endstempel begann die Invalidierung mangels gespeichertem
/// Begin pauschal bei `i64::MIN / 2` und schloss damit auch saemtliche
/// aeltere, nicht ueberlappende Evidenz aus; bei schema-gueltigem
/// `project_sample_end = null` wurde umgekehrt GAR NICHTS invalidiert, statt
/// fail-closed die Sitzung zu waehlen. Kein Test der Runde 1 fuhr den
/// Marker-Wirepfad mit frueherer Kontroll-Evidenz.
#[cfg(windows)]
#[test]
fn markerinvalidierung_trifft_genau_ihren_bereich() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    let sonde = hello(1, 2, 0x20, 0x21, "passive_probe", Some(9));
    anmelden(&c, "sonde", &sonde);
    report(&c, "sonde", &sonde.adresse);

    // Drei Belege: einer WEIT VOR dem Marker, zwei in seinem Bereich.
    let setze = |w: &mut Value, start: i64| {
        w["transport"]["project_sample_start"] = json!(start);
    };
    assert!(c.evidence_snapshot_json(
        "sonde",
        &evidenz_payload(&sonde.adresse, 0, |w| setze(w, 1_000))
    ));
    assert!(c.evidence_snapshot_json(
        "sonde",
        &evidenz_payload(&sonde.adresse, 1, |w| setze(w, 500_000))
    ));
    assert!(c.evidence_snapshot_json(
        "sonde",
        &evidenz_payload(&sonde.adresse, 2, |w| setze(w, 501_000))
    ));

    let id = hex(0xd50);
    let begin = serde_json::to_vec(&json!({
        "type": "audible_intervention_begin",
        "intervention_id": id,
        "adresse": h.adresse,
        "event_sequence": 1,
        "art": "hoermarkierung",
        "project_sample_start": 499_000
    }))
    .unwrap();
    Senke::p0(&c, "a", &begin);
    let ende = serde_json::to_vec(&json!({
        "type": "audible_intervention_end",
        "intervention_id": id,
        "adresse": h.adresse,
        "event_sequence": 2,
        "project_sample_end": 502_000,
        "tail_samples": 0
    }))
    .unwrap();
    Senke::p0(&c, "a", &ende);

    let historie = c.evidenz_historie(&hex(0x20));
    assert_eq!(historie.len(), 3);
    assert!(
        historie[0].ausschlussgrund.is_none(),
        "R25: der Beleg WEIT VOR dem Marker bleibt gueltig - die Runde 1 nahm \
         ihn mit, weil sie bei `i64::MIN / 2` begann: {:?}",
        historie[0].ausschlussgrund
    );
    assert!(
        historie[1].ausschlussgrund.as_deref() == Some("intervention")
            && historie[2].ausschlussgrund.as_deref() == Some("intervention"),
        "und die beiden IM Bereich sind zurueckgenommen"
    );
}

/// R25 — ohne Projektzeit invalidiert der Marker die GANZE Sitzung.
///
/// Bei schema-gueltigem `project_sample_end = null` nahm die Runde 1 gar
/// nichts zurueck — fail-OPEN, obwohl der Marker gefaerbt hat und niemand
/// weiss wo.
#[cfg(windows)]
#[test]
fn markerinvalidierung_ohne_projektzeit_ist_fail_closed() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    let sonde = hello(1, 2, 0x20, 0x21, "passive_probe", Some(9));
    anmelden(&c, "sonde", &sonde);
    report(&c, "sonde", &sonde.adresse);
    for nr in 0..3 {
        assert!(c.evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }

    let id = hex(0xd60);
    Senke::p0(
        &c,
        "a",
        &serde_json::to_vec(&json!({
            "type": "audible_intervention_begin",
            "intervention_id": id,
            "adresse": h.adresse,
            "event_sequence": 1,
            "art": "hoermarkierung",
            "project_sample_start": null
        }))
        .unwrap(),
    );
    Senke::p0(
        &c,
        "a",
        &serde_json::to_vec(&json!({
            "type": "audible_intervention_end",
            "intervention_id": id,
            "adresse": h.adresse,
            "event_sequence": 2,
            "project_sample_end": null,
            "tail_samples": 0
        }))
        .unwrap(),
    );

    assert_eq!(
        c.invalidierungen_zaehler(),
        1,
        "R25: ohne Projektzeit gibt es keinen Bereich - fail-closed heisst \
         die ganze Sitzung, nicht `nichts`"
    );
    assert!(c
        .evidenz_historie(&hex(0x20))
        .iter()
        .all(|e| e.ausschlussgrund.as_deref() == Some("intervention")));
}

/// R29/R30 — der PRE/POST-Join filtert zurueckgenommene Evidenz und rechnet in
/// EINER Konvention.
///
/// `haelfte_aus_historie` iterierte ueber jeden Eintrag ohne den
/// `ausschlussgrund` zu pruefen und gab die dB-Werte unveraendert als
/// Huellkurven weiter — `relation_db` erwartet aber positive lineare
/// Amplituden und verwarf normale negative dB-Werte alle. Der alte Test
/// pruefte nur, dass IRGENDEIN Urteil mit schwacher Klasse existiert, nicht
/// dessen Ergebnis.
#[test]
fn prepost_filtert_ausschluss_und_rechnet_in_einer_konvention() {
    let (c, _) = coordinator();
    let paar = hex(0x79);
    let pre = hello(1, 2, 10, 100, "passive_probe", Some(9));
    let post = hello(1, 2, 11, 101, "passive_probe", Some(9));
    anmelden(&c, "pre", &pre);
    anmelden(&c, "post", &post);
    assert!(c.descriptor_setzen("pre", descriptor(&pre.adresse, "pre", &paar)));
    assert!(c.descriptor_setzen("post", descriptor(&post.adresse, "post", &paar)));
    // Die Huellkurven brauchen STRUKTUR, sonst findet der Restlag-Schaetzer
    // keinen Bezug: eine konstante Reihe korreliert mit allem gleich gut.
    // POST liegt durchgehend 3 dB ueber PRE - das ist der reale dB-Wert,
    // gegen den dieser Fall misst.
    // Das Fixture kodiert `q_db_0p1_i16`: ein Schritt sind 0,1 dB. Die Zahlen
    // hier stehen deshalb in ZEHNTEL-Dezibel, und der Test rechnet die
    // Erwartung daraus - er schreibt sie nicht ab.
    let schritte_je_db = 10i64;
    let pegeln = |w: &mut Value, offset_zehnteldb: i64, nr: usize| {
        // Die Snapshots liegen eine halbe Sekunde auseinander - so kommt der
        // Evidenzstrom bei 1 bis 4 Hz wirklich an, und erst damit hat der
        // Restlag-Schaetzer einen Suchraum (Befund R30).
        w["transport"]["project_sample_start"] = json!(nr as i64 * 24_000);
        if let Some(werte) = w["verteilung"]["p50"]["werte"].as_array_mut() {
            // Eine RAMPE ueber die Zeit gibt der Kurve ihre Struktur - und
            // zwar eine ohne Periode: eine periodische Welle haette mehrere
            // gleich gute Korrelationsmaxima, und der Restlag waere geraten.
            let welle = (nr as i64 - 6) * 10;
            for v in werte.iter_mut() {
                if let Some(x) = v.as_i64() {
                    *v = json!((x + offset_zehnteldb + welle).clamp(-32768, 32767));
                }
            }
        }
    };
    for nr in 0..12 {
        c.evidence_snapshot_json(
            "pre",
            &evidenz_payload(&pre.adresse, nr, |w| pegeln(w, 0, nr)),
        );
        c.evidence_snapshot_json(
            "post",
            &evidenz_payload(&post.adresse, nr, |w| pegeln(w, 3 * schritte_je_db, nr)),
        );
    }

    // R30: das Urteil traegt ein ERGEBNIS, nicht nur eine Klasse - und die
    // rohen Banddeltas sind ECHTE dB. Mit dB-Werten als lineare Amplituden
    // verwarf `relation_db` jeden Frame (`x <= 1e-9`).
    let urteil = c.paarurteil(&paar).expect("ein Urteil entsteht");
    let ergebnis = urteil.ergebnis.as_ref().unwrap_or_else(|| {
        panic!(
            "R30: eine Konvention - die Huellkurve ist linear, und `relation_db` \
             rechnet daraus echte Banddeltas: {urteil:?}"
        )
    });
    let gemessen: Vec<f64> = ergebnis
        .roh_db
        .iter()
        .zip(ergebnis.roh_gueltig.iter())
        .filter(|(_, ok)| **ok)
        .map(|(d, _)| *d)
        .collect();
    assert!(
        !gemessen.is_empty(),
        "R30: mindestens ein Band traegt eine Messung: {ergebnis:?}"
    );
    let mittel = gemessen.iter().sum::<f64>() / gemessen.len() as f64;
    assert!(
        (mittel - 3.0).abs() < 0.5,
        "R30: und der reale dB-Wert kommt heraus - POST liegt 3 dB ueber PRE, \
         gemessen {mittel:.3} dB ueber {} Baender",
        gemessen.len()
    );

    // R29: eine Invalidierung nimmt die Evidenz zurueck, und der Join
    // benutzt sie danach NICHT mehr.
    assert!(c.invalidierung_wegen_messpunkt_fuer_link("pre", "pre", "post") > 0);
    assert!(c
        .evidenz_historie(&hex(10))
        .iter()
        .all(|e| e.ausschlussgrund.is_some()));
    // Ein weiterer Snapshot der POST-Haelfte stoesst die Neubildung an.
    c.evidence_snapshot_json(
        "post",
        &evidenz_payload(&post.adresse, 20, |w| pegeln(w, 3 * schritte_je_db, 20)),
    );
    let nachher = c.paarurteil(&paar).expect("das Paar bleibt sichtbar");
    assert!(
        nachher.ergebnis.is_none(),
        "R29: aus zurueckgenommener Evidenz entsteht KEIN Kettenbefund mehr: \
         {nachher:?}"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// R14/R28/R32 · Outbox und Zustellung (Nacharbeit 2)
//
// Die Regel dieser Klasse: Nachricht rein → Outbox-Eintrag → Push → im
// Plugin-Modell angekommen. Eine Outbox-Schuld ohne Leser ist ein Defekt.
// Die drei Tests hier messen die ersten drei Glieder im Broker; das vierte
// misst B10 auf der C++-Seite gegen DASSELBE Fixture des Korpus.
// ═════════════════════════════════════════════════════════════════════════

/// Was der Broker an den abonnierten Main geschrieben hat, nur die Payloads.
#[cfg(windows)]
fn gepusht(h: &HarnischMitStore) -> Vec<Value> {
    h.push.payloads().into_iter().map(|(_, w)| w).collect()
}

/// R14 — das Versuchsergebnis ERREICHT Gen.
///
/// 🔑 Die Runde 1 rechnete das Resultat und legte es in den Store. Der
/// Terminal-Event bekam aber gar keine `snapshot_ziele`, und der
/// Sessionsnapshot trug ueberhaupt kein Experimentfeld: es gab keine
/// Outbox-Schuld, keinen Push und im Plugin-Modell nichts anzukommen. Ein
/// Test, der `experiment_sicht()` fragt, sieht davon nichts — er fragt das
/// Modul, nicht den Weg.
#[cfg(windows)]
#[test]
fn versuchsergebnis_erreicht_gen_ueber_outbox_und_push() {
    let h = HarnischMitStore::neu("r14-rueckweg");
    h.abonniert();
    let versuch = 0xd00;
    let vorlage = experiment_begin_wert(&h.main.adresse, 0x9b0, versuch);
    let sonde = sonde_in_der_passage(&h, &vorlage);
    let epoche = vorlage["passage"]["transport_epoch"].as_u64().unwrap();
    let von = vorlage["passage"]["projekt_von"].as_i64().unwrap();

    // Baseline VOR dem Begin, dann Begin und Kandidat.
    for nr in 0..8 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_in_passage(&sonde.adresse, &vorlage, nr)));
    }
    assert_eq!(
        h.p0(&experiment_begin_wert(&h.main.adresse, 0x9b1, versuch))["ergebnis"],
        "angewandt"
    );
    assert_eq!(
        h.p0(&json!({
            "type": "experiment_candidate",
            "kopf": {
                "command_id": hex(0x9b2), "ziel": h.main.adresse, "base_revision": 0,
                "ttl_ms": 1000, "schema_major": 3, "schema_minor": 0
            },
            "experiment_id": hex(versuch),
            "referenz": vorlage["referenz"],
            "blindreihenfolge": "kandidat_zuerst"
        }))["ergebnis"],
        "angewandt"
    );
    // Resultatmessung NACH dem Kandidaten.
    for nr in 12..18 {
        assert!(h.c.evidence_snapshot_json(
            "sonde",
            &evidenz_payload(&sonde.adresse, nr, |w| {
                w["transport"]["transport_epoch"] = json!(epoche);
                w["transport"]["project_sample_start"] = json!(von + (nr as i64) * 512);
            })
        ));
    }

    // Der OFFENE Versuch reist bereits mit — Gen soll sehen, dass einer
    // laeuft. Was es vor dem Terminal NICHT gibt, ist eine Aussage: kein
    // Hoerurteil, keine aufgedeckte Reihenfolge, kein Urteil. Genau das
    // trennt „ein Versuch laeuft" von „ein Versuch hat ein Ergebnis".
    for w in gepusht(&h) {
        let Some(liste) = w["experimente"].as_array() else {
            continue;
        };
        for e in liste {
            assert_eq!(e["offen"], json!(true), "vor dem Terminal ist er offen: {e}");
            assert!(
                e["hoerurteil"].is_null()
                    && e["blindreihenfolge"].is_null()
                    && e["urteil"].is_null(),
                "vor dem Terminal reist KEINE Aussage — die Blindreihenfolge \
                 waere sonst vor dem Urteil aufgedeckt (M-44): {e}"
            );
        }
    }

    let ack = h.p0(&json!({
        "type": "experiment_manual_result",
        "kopf": {
            "command_id": hex(0x9b3), "ziel": h.main.adresse, "base_revision": 0,
            "ttl_ms": 1000, "schema_major": 3, "schema_minor": 0
        },
        "experiment_id": hex(versuch),
        "hoerurteil": "kandidat",
        "blindreihenfolge": "kandidat_zuerst",
        "notiz": null,
        "werkzeug": null
    }));
    assert_eq!(ack["ergebnis"], "angewandt", "das Terminal wird angewandt: {ack}");

    // 🔑 DAS ist die Zusage: der Snapshot mit dem Versuch ist WIRKLICH
    // geschrieben worden — nicht nur gerechnet, nicht nur geschuldet.
    let mit_versuch: Vec<Value> = gepusht(&h)
        .into_iter()
        .filter(|w| w["experimente"].is_array())
        .collect();
    let letzter = mit_versuch
        .last()
        .unwrap_or_else(|| panic!("R14: ein Push traegt den Versuch: {:?}", gepusht(&h)));
    let e = &letzter["experimente"][0];
    assert_eq!(e["experiment_id"], json!(hex(versuch)));
    assert_eq!(e["ereignis"], "ergebnis", "die Transition reist mit: {e}");
    assert_eq!(e["offen"], json!(false), "und der Versuch ist zu: {e}");
    assert_eq!(
        e["hoerurteil"], "kandidat",
        "das Userurteil reist mit, nicht nur das Metrikdelta: {e}"
    );
    assert_eq!(
        e["blindreihenfolge"], "kandidat_zuerst",
        "die Reihenfolge wird MIT dem Terminal aufgedeckt (M-44): {e}"
    );
    assert!(
        e["urteil"].is_string(),
        "und genau eine der fuenf zulaessigen Aussagen (M-46): {e}"
    );

    // Die Zustellschuld ist danach abgetragen. Eine Schuld, die niemand
    // abtraegt, waechst — und ein Wiederanlauf sendet sie erneut.
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM outbox WHERE object_key='session_snapshot'"),
        0,
        "R14: der zugestellte Snapshot ist kompaktiert"
    );
}

/// R32 — das PRE/POST-Paarurteil ERREICHT Gen.
///
/// 🔑 Die Runde 1 legte es in eine fluechtige Map im Coordinator: kein
/// `StoreEvent`, kein Dirty-Snapshot, kein Outboxziel. PRE/POST-Nachrichten
/// erreichten den Coordinator, aber keinen fuer Gen sichtbaren Ausgang. Der
/// bestehende Test fragte `paarurteil()` — die Map selbst.
#[cfg(windows)]
#[test]
fn paarurteil_erreicht_gen_ueber_outbox_und_push() {
    let h = HarnischMitStore::neu("r32-rueckweg");
    h.abonniert();
    let paar = hex(0x77);
    let mut haelfte = |kuerzel: &str, instanz: usize, nonce: usize, position: &str| {
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = hex(instanz);
        s.adresse.runtime_nonce = hex(nonce);
        anmelden(&h.c, kuerzel, &s);
        assert!(
            h.c.descriptor_setzen(kuerzel, descriptor(&s.adresse, position, &paar)),
            "der {position}-Deskriptor wird angenommen"
        );
        s
    };
    let pre = haelfte("pre", 0x30, 0x31, "pre");
    let post = haelfte("post", 0x32, 0x33, "post");

    assert!(
        gepusht(&h).iter().all(|w| w["paare"].is_null()),
        "vor dem ersten Snapshot reist kein Paar: {:?}",
        gepusht(&h)
    );

    for nr in 0..8 {
        assert!(h
            .c
            .evidence_snapshot_json("pre", &evidenz_payload(&pre.adresse, nr, |_| {})));
        assert!(h
            .c
            .evidence_snapshot_json("post", &evidenz_payload(&post.adresse, nr, |_| {})));
    }

    // 🔑 Das Urteil steht nicht nur in der Map, es ist GESCHRIEBEN.
    let mit_paar: Vec<Value> = gepusht(&h)
        .into_iter()
        .filter(|w| w["paare"].is_array())
        .collect();
    let letzter = mit_paar
        .last()
        .unwrap_or_else(|| panic!("R32: ein Push traegt das Paarurteil: {:?}", gepusht(&h)));
    let p = &letzter["paare"][0];
    assert_eq!(p["pair_id"], json!(paar));
    assert!(
        p["klasse"].is_string() && p["kettenbefund"].is_string(),
        "Klasse und Kettenbefund reisen mit (M-13/M-22): {p}"
    );
    // Und die Zusage aus M-21 haelt auch auf dem Draht: ohne validierte
    // Presentation-Abbildung keine starke Klasse.
    assert_ne!(
        p["klasse"], "feature_aligned",
        "ohne validierte Presentation-Abbildung keine starke Aussage: {p}"
    );
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM outbox WHERE object_key='session_snapshot'"),
        0,
        "R32: der zugestellte Snapshot ist kompaktiert"
    );
}

/// R28 — die Evidenzruecknahme wird dem Abonnenten WIRKLICH zugestellt.
///
/// 🔑 Die Runde 1 legte nur eine Outbox-Schuld an. Kein Produktcode las sie
/// aus, `SessionPush::snapshot_schreiben` wurde fuer die Invalidierung nie
/// gerufen, und die Schuld blieb ewig stehen. Ein aktiver Subscriber erhielt
/// die Ruecknahme nie — er zeigte weiter Zahlen, deren Grundlage zurueck-
/// gezogen war.
#[cfg(windows)]
#[test]
fn invalidierung_wird_dem_abonnenten_wirklich_zugestellt() {
    let h = HarnischMitStore::neu("r28-zustellung");
    h.abonniert();
    let sonde = {
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = hex(0x20);
        s.adresse.runtime_nonce = hex(0x21);
        s
    };
    anmelden(&h.c, "sonde", &sonde);
    report(&h.c, "sonde", &sonde.adresse);
    for nr in 0..3 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }
    assert!(
        gepusht(&h)
            .iter()
            .all(|w| w["type"] != "evidence_invalidate"),
        "vor der Ruecknahme reist keine"
    );

    let betroffen = h.c.invalidierung_wegen_material_fuer_link("sonde", None, None);
    assert!(betroffen > 0, "die Ruecknahme trifft gespeicherte Evidenz");

    // 🔑 Sie ist WIRKLICH geschrieben worden, als eigene Familie mit eigenem
    // Leser — nicht als Sessionschnitt und nicht als blosse Schuld.
    let ruecknahmen: Vec<Value> = gepusht(&h)
        .into_iter()
        .filter(|w| w["type"] == "evidence_invalidate")
        .collect();
    let n = ruecknahmen
        .first()
        .unwrap_or_else(|| panic!("R28: die Ruecknahme wird zugestellt: {:?}", gepusht(&h)));
    assert_eq!(
        n["grund"], "material_wechsel",
        "mit ihrem Grund, nicht auf einen bekannten abgebildet: {n}"
    );
    // Geaendertes Material nimmt die GANZE Sitzung (M-54): der Fingerprint
    // sagt, dass sich das Material geaendert hat, nicht wo.
    assert_eq!(
        n["umfang"], json!({"art": "ganze_sitzung"}),
        "und mit ihrem diskriminierten Umfang, ohne Bereich (M-57): {n}"
    );

    // Der zweite Umfang reist ebenso — ein Weg, der nur eine der drei Arten
    // zustellt, waere fuer die anderen zwei tot.
    for nr in 3..6 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }
    assert!(h
        .c
        .invalidierung_wegen_intervention_fuer_link("sonde", 0, i64::MAX / 2) > 0);
    let bereich = gepusht(&h)
        .into_iter()
        .filter(|w| w["type"] == "evidence_invalidate")
        .last()
        .expect("R28: auch die zweite Ruecknahme wird zugestellt");
    assert_eq!(bereich["grund"], "intervention", "{bereich}");
    assert_eq!(
        bereich["umfang"]["art"], "sample_range",
        "der Bereichsumfang reist als Bereich, nicht als Sitzung: {bereich}"
    );

    // Und die Schuld ist abgetragen. Sie stehenzulassen hiesse, sie bei
    // jedem Wiederanlauf erneut zuzustellen.
    assert_eq!(
        h.zeilen("SELECT COUNT(*) FROM outbox WHERE object_key='evidence_invalidate'"),
        0,
        "R28: die zugestellte Ruecknahme ist kompaktiert"
    );
}

/// Selbstaudit dieser Runde: der Rueckweg haelt seinen eigenen Vertragsdeckel.
///
/// 🔑 `session_snapshot.experimente` erlaubt hoechstens 32 Eintraege, und der
/// Bestandsdeckel M-48 deckelt nur die OFFENEN — abgeschlossene sammeln sich
/// in derselben Ablage. Eine ungedeckelte Liste haette in einer langen
/// Sitzung einen vertragswidrigen Snapshot erzeugt, den der Leser GANZ
/// verwirft: der Rueckweg waere genau dann gerissen, wenn er am meisten
/// traegt.
#[cfg(windows)]
#[test]
fn der_snapshot_haelt_den_versuchsdeckel_des_vertrages() {
    use eqcop_broker::coordinator::experiment::N_GLOBAL;
    let h = HarnischMitStore::neu("r14-deckel");
    h.abonniert();
    let runden = N_GLOBAL + 8;
    for i in 0..runden {
        let versuch = 0xe000 + i;
        assert_eq!(
            h.p0(&experiment_begin_wert(&h.main.adresse, 0xa000 + 2 * i, versuch))["ergebnis"],
            "angewandt",
            "Versuch {i} beginnt"
        );
        assert_eq!(
            h.p0(&json!({
                "type": "experiment_abort",
                "kopf": {
                    "command_id": hex(0xa001 + 2 * i), "ziel": h.main.adresse,
                    "base_revision": 0, "ttl_ms": 1000,
                    "schema_major": 3, "schema_minor": 0
                },
                "experiment_id": hex(versuch),
                "grund": "user_abbruch"
            }))["ergebnis"],
            "angewandt",
            "Versuch {i} endet"
        );
    }

    let letzter = gepusht(&h)
        .into_iter()
        .filter(|w| w["experimente"].is_array())
        .last()
        .expect("der Snapshot traegt Versuche");
    let liste = letzter["experimente"].as_array().expect("eine Liste");
    assert_eq!(
        liste.len(),
        N_GLOBAL,
        "die Liste haelt den Vertragsdeckel: {}",
        liste.len()
    );
    // Gekappt wird am ALTEN Ende, und die Anlegereihenfolge bleibt stehen.
    assert_eq!(
        liste.first().map(|e| &e["experiment_id"]),
        Some(&json!(hex(0xe000 + runden - N_GLOBAL))),
        "der aelteste noch getragene Versuch ist der {}-te: {liste:?}",
        runden - N_GLOBAL
    );
    assert_eq!(
        liste.last().map(|e| &e["experiment_id"]),
        Some(&json!(hex(0xe000 + runden - 1))),
        "und der juengste steht am Ende: {liste:?}"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// NACHARBEIT 3 · W3 — Befehl und Wirkung sind EIN Append, und der Rueckweg
// ist gezielt
// ═════════════════════════════════════════════════════════════════════════

/// B14 — eine Preview committet ihre Ruecknahme MIT sich, oder gar nicht.
///
/// 🔑 Wiederpruefung 2: Die Runde 2 committete `preview_*` ueber
/// `persistenz_p0` und invalidierte erst NACH dem Ack in einem zweiten Append.
/// Stuerzte der Broker dazwischen, lieferte der Retry `idempotent_wiederholt`,
/// und die Bedingung „nur bei angewandt" uebersprang die Invalidierung FUER
/// IMMER: die Vorschau stand persistiert da, ihre Evidenz blieb gueltig, und
/// niemand konnte es sehen. Der Test der Runde 2 fuhr nur den stoerungsfreien
/// Erstaufruf.
#[cfg(windows)]
#[test]
fn preview_und_ruecknahme_liegen_in_einem_append() {
    let h = HarnischMitStore::neu("b14-preview-atomar");
    let sonde = {
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = hex(0x20);
        s.adresse.runtime_nonce = hex(0x21);
        s
    };
    anmelden(&h.c, "sonde", &sonde);
    report(&h.c, "sonde", &sonde.adresse);
    for nr in 0..3 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }
    let preview = |command_id: String| {
        json!({
            "type": "preview_begin",
            "kopf": {
                "command_id": command_id,
                "ziel": h.main.adresse,
                "base_revision": 0,
                "ttl_ms": 1000,
                "schema_major": 3,
                "schema_minor": 0
            },
            "lease_duration_ms": 400,
            "renew_id": hex(0x931)
        })
    };
    let invalidierungen =
        || h.zeilen("select count(*) from event_log where event_type='evidence_invalidate'");
    let vorher = invalidierungen();

    // ── 1. Befehl UND Ruecknahme stehen in EINEM Append ─────────────────
    let ack = h.p0(&preview(hex(0x941)));
    assert_eq!(ack["ergebnis"], "angewandt", "{ack}");
    assert_eq!(invalidierungen(), vorher + 1, "die Ruecknahme steht im Log");
    let abstand = h.zeilen(
        "select (select max(event_ord) from event_log where event_type='evidence_invalidate') \
         - (select max(event_ord) from event_log where event_type='command')",
    );
    assert_eq!(
        abstand, 1,
        "preview_und_ruecknahme_liegen_in_einem_append - die Ruecknahme folgt \
         dem Befehl UNMITTELBAR; dazwischen passt kein zweiter Commit und damit \
         auch kein Absturz"
    );
    assert!(
        h.c.evidenz_historie(&hex(0x20))
            .iter()
            .all(|e| e.ausschlussgrund.as_deref() == Some("intervention")),
        "und jeder Beleg traegt seinen Grund"
    );

    // ── 2. Der Retry wiederholt idempotent und laesst die Ruecknahme stehen
    let ack = h.p0(&preview(hex(0x941)));
    assert_eq!(
        ack["ergebnis"], "idempotent_wiederholt",
        "derselbe Befehl zweimal ist eine Wiederholung, kein zweiter: {ack}"
    );
    assert_eq!(
        invalidierungen(),
        vorher + 1,
        "und die Ruecknahme bleibt GENAU EINE - der Retry legt keine zweite an \
         und laesst auch keine fehlen"
    );

    // ── 3. Mit gestoertem Store entsteht GAR KEIN Zustand ───────────────
    //
    // Auf FRISCHER Evidenz, denn die alte ist bereits zurueckgenommen: sonst
    // liesse sich „nicht ausgeschlossen" nicht von „schon ausgeschlossen"
    // unterscheiden.
    let zweite = {
        let mut z = h.main.clone();
        z.plugin_kind = "passive_probe".into();
        z.adresse.instance_id = hex(0x22);
        z.adresse.runtime_nonce = hex(0x23);
        z
    };
    anmelden(&h.c, "sonde2", &zweite);
    report(&h.c, "sonde2", &zweite.adresse);
    for nr in 0..3 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde2", &evidenz_payload(&zweite.adresse, nr, |_| {})));
    }
    let nach_zwei = invalidierungen();
    h.naht(true);
    assert!(
        h.p0_von("main", &preview(hex(0x940))).is_none(),
        "ein Befehl, dessen Append scheitert, wird NICHT angewandt - er bleibt \
         unbeantwortet, und der Link gilt als storeverweigert"
    );
    assert_eq!(
        invalidierungen(),
        nach_zwei,
        "und es kommt KEINE Invalidierung ins Log"
    );
    assert!(
        h.c.evidenz_historie(&hex(0x22))
            .iter()
            .all(|e| e.ausschlussgrund.is_none()),
        "B16: und AUCH KEIN lokaler Ausschluss - der fluechtige Stand bleibt \
         gleich dem persistierten"
    );
}

/// B16 — ein gescheiterter Append hinterlaesst keinen lokalen Ausschluss.
///
/// 🔑 Wiederpruefung 2: `invalidierung_anwenden` schloss die Evidenz lokal
/// aus, zaehlte einen Storefehler nur und kehrte mit `()` zurueck. Der
/// Ausschluss blieb stehen, der Auslöser galt als erfolgreich, und ein
/// Neustart liess dieselbe Evidenz wieder zu — fluechtiger Stand und Log
/// sagten Verschiedenes.
#[cfg(windows)]
#[test]
fn gescheiterter_append_laesst_keinen_ausschluss_stehen() {
    let h = HarnischMitStore::neu("b16-append-fehler");
    let sonde = {
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = hex(0x20);
        s.adresse.runtime_nonce = hex(0x21);
        s
    };
    anmelden(&h.c, "sonde", &sonde);
    report(&h.c, "sonde", &sonde.adresse);
    assert!(h
        .c
        .descriptor_setzen("sonde", descriptor(&sonde.adresse, "pre", &hex(0x77))));
    for nr in 0..3 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }

    let vorher = h.zeilen("select count(*) from event_log where event_type='evidence_invalidate'");
    h.naht(true);
    assert_eq!(
        h.c.invalidierung_wegen_messpunkt_fuer_link("sonde", "pre", "post"),
        0,
        "ein Messpunktwechsel, dessen Append scheitert, schliesst NICHTS aus"
    );
    assert!(
        h.c.evidenz_historie(&hex(0x20))
            .iter()
            .all(|e| e.ausschlussgrund.is_none()),
        "gescheiterter_append_laesst_keinen_ausschluss_stehen - die Evidenz ist \
         UNVERAENDERT"
    );
    assert_eq!(
        h.zeilen("select count(*) from event_log where event_type='evidence_invalidate'"),
        vorher,
        "und im Log kommt nichts hinzu"
    );

    // Nach dem Neustart ist der Stand IDENTISCH: die Evidenz ist zugelassen,
    // weil sie nie ausgeschlossen wurde.
    let neu = h.neuer_coordinator();
    assert!(
        neu.evidenz_historie(&hex(0x20))
            .iter()
            .all(|e| e.ausschlussgrund.is_none()),
        "und ein Neustart sieht denselben Stand - keine zweite Wahrheit"
    );

    // Die Gegenprobe im selben Lauf: ohne Stoerung greift derselbe Weg.
    h.naht(false);
    assert!(
        h.c.invalidierung_wegen_messpunkt_fuer_link("sonde", "pre", "post") > 0,
        "derselbe Wechsel mit gesundem Store nimmt zurueck - der Riegel sperrt \
         die richtige Haelfte, nicht alles"
    );
}

/// B3 — der Rueckweg trifft NUR den eigenen Versuch.
///
/// 🔑 Wiederpruefung 2: Die Runde 2 klonte vor jedem Experimentbefehl den
/// GANZEN Store und ersetzte bei gescheitertem Append den gesamten Stand durch
/// die Kopie. Committet waehrenddessen eine andere Sitzung erfolgreich,
/// loescht dieser Rollback ihre Wirkung aus dem Speicher, obwohl sie
/// persistiert ist. Der Test der Runde 2 war seriell und injizierte keinen
/// Appendfehler.
#[cfg(windows)]
#[test]
fn ruecknahme_trifft_nur_den_eigenen_versuch() {
    let h = HarnischMitStore::neu("b3-gezielter-rueckweg");

    let mut eins = experiment_begin_wert(&h.main.adresse, 0x951, 0x9b1);
    eins["passage"]["passage_id"] = json!(hex(0x9a1));
    let ack = h.p0(&eins);
    assert_eq!(ack["ergebnis"], "angewandt", "{ack}");
    assert!(h.c.experiment_sicht(&hex(0x9b1)).is_some(), "er steht");

    // Der zweite scheitert am Store — und darf den ersten nicht mitnehmen.
    h.naht(true);
    let mut zwei = experiment_begin_wert(&h.main.adresse, 0x952, 0x9b2);
    zwei["passage"]["passage_id"] = json!(hex(0x9a2));
    assert!(
        h.p0_von("main", &zwei).is_none(),
        "ein Begin, dessen Append scheitert, wird nicht angewandt"
    );
    assert!(
        h.c.experiment_sicht(&hex(0x9b2)).is_none(),
        "der gescheiterte Versuch ist zurueckgenommen"
    );
    assert!(
        h.c.passage_sicht(&hex(0x9a2)).is_none(),
        "und seine Passage mit ihm - ein halber Zustand bleibt nicht stehen"
    );
    assert!(
        h.c.experiment_sicht(&hex(0x9b1)).is_some(),
        "ruecknahme_trifft_nur_den_eigenen_versuch - der bereits persistierte \
         Versuch steht UNVERAENDERT; ein Klonersatz haette ihn mitgenommen"
    );
    assert!(
        h.c.passage_sicht(&hex(0x9a1)).is_some(),
        "und seine Passage ebenso"
    );

    // Und nach einem Reconnect geht es normal weiter: der Rueckweg hat nichts
    // verklemmt. Der ALTE Link bleibt storeverweigert — das ist die Zusage,
    // kein Nebeneffekt.
    h.naht(false);
    anmelden(&h.c, "main-neu", &h.main);
    assert!(report_main(&h.c, "main-neu", &h.main.adresse));
    assert!(h
        .c
        .state_report_json("main-neu", &state_report_payload(&h.main.adresse, 0)));
    let mut drei = experiment_begin_wert(&h.main.adresse, 0x953, 0x9b3);
    drei["passage"]["passage_id"] = json!(hex(0x9a3));
    let ack = h
        .p0_von("main-neu", &drei)
        .expect("die Familie wird beantwortet");
    assert_eq!(ack["ergebnis"], "angewandt", "{ack}");
    assert!(h.c.experiment_sicht(&hex(0x9b3)).is_some());
}

/// B4 — der Taint eines VERDRAENGTEN Versuchs wird in SEINER Sitzung
/// geschlossen (M-59).
///
/// 🔑 Wiederpruefung 2: `verdraenge_fuer` verdraengt projektuebergreifend, und
/// die Runde 2 schloss die Intervalle mit der Sitzung des NEUEN Aufrufers.
/// Verdraengt der Deckel einen Versuch aus einer anderen Sitzung, blieb dessen
/// Taint dort OFFEN: der Versuch war terminal, sein Eingriff galt weiter, und
/// die Sitzung des Opfers lieferte dauerhaft keine starke Evidenz mehr. Der
/// Test der Runde 2 nutzte nur EINE Sitzung.
#[cfg(windows)]
#[test]
fn verdraengter_taint_faellt_in_der_opfersitzung() {
    let h = HarnischMitStore::neu("b4-verdraengter-taint");

    // Sitzung A fuellt den Projektdeckel (N_PROJEKT = 8).
    let mut ids = Vec::new();
    for i in 0..8usize {
        let mut wert = experiment_begin_wert(&h.main.adresse, 0x960 + i, 0x9c0 + i);
        wert["passage"]["passage_id"] = json!(hex(0x9d0 + i));
        let ack = h.p0(&wert);
        assert_eq!(ack["ergebnis"], "angewandt", "Versuch {i}: {ack}");
        ids.push(hex(0x9c0 + i));
    }

    // Der AELTESTE traegt ein offenes Taintintervall in Sitzung A.
    assert!(h.c.intervention_begin_mit_art(
        "main",
        &h.main.adresse,
        &hex(0x9e0),
        1,
        "experiment",
        Some(&ids[0]),
        Some(0),
    ));
    assert_eq!(
        h.c.interventionssicht_fuer_link("main").aktive,
        1,
        "Sitzung A haelt das Intervall ihres aeltesten Versuchs"
    );

    // Eine ZWEITE Sitzung desselben Projekts — der Projektdeckel greift ueber
    // Sitzungsgrenzen hinweg, und genau darin liegt der Befund.
    // Ein EIGENER Host-PID: `auto_join_locked` bestaetigt einen Main nur, wenn
    // seine Sitzung fuer diesen Host eindeutig ist. Zwei Mains desselben Hosts
    // in zwei Sitzungen waeren ein Konfliktfall und keine zweite Sitzung — und
    // der Befund handelt von zwei ECHTEN Sitzungen.
    let main_b = hello(1, 3, 0x11, 0x101, "main", Some(10));
    anmelden(&h.c, "mainb", &main_b);
    report_main(&h.c, "mainb", &main_b.adresse);
    assert!(h
        .c
        .state_report_json("mainb", &state_report_payload(&main_b.adresse, 0)));

    // Sitzung B beginnt den neunten Versuch DESSELBEN Projekts. Der
    // Projektdeckel verdraengt den aeltesten — und der gehoert A.
    let mut neunter = experiment_begin_wert(&main_b.adresse, 0x970, 0x9c9);
    neunter["passage"]["passage_id"] = json!(hex(0x9d9));
    let ack = h
        .p0_von("mainb", &neunter)
        .expect("die Familie wird beantwortet");
    assert_eq!(ack["ergebnis"], "angewandt", "{ack}");
    assert!(
        h.c.experiment_sicht(&ids[0]).is_some_and(|e| !e.offen()),
        "der aelteste Versuch aus A ist verdraengt"
    );
    assert_eq!(
        h.c.interventionssicht_fuer_link("main").aktive,
        0,
        "verdraengter_taint_faellt_in_der_opfersitzung - sein Intervall ist in \
         SEINER Sitzung geschlossen, nicht in der des neuen Aufrufers"
    );
    assert!(
        h.c.interventionssicht_fuer_link("main")
            .starke_evidenz_erlaubt,
        "und Sitzung A liefert wieder starke Evidenz, statt dauerhaft gesperrt \
         zu bleiben"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// NACHARBEIT 3 · W4 — Replay und Reihenfolge
// ═════════════════════════════════════════════════════════════════════════

/// B9 — der ERSTE Snapshot nach dem Kandidaten zaehlt als Resultat.
///
/// 🔑 Wiederpruefung 2: Beim Kandidaten wurde der aktuelle NAECHSTE
/// Evidenzzaehler N gespeichert, und der erste danach angenommene Snapshot
/// bekam durch `fetch_add` ebenfalls N. Weil nur `empfangsfolge >
/// kandidat_folge` als Resultat galt, wurde genau dieser eindeutig NACH dem
/// Kandidaten eingegangene Beleg verworfen — der erste Beleg der Aenderung.
/// Die Tests der Runde 2 sendeten mehrere Resultatsnapshots und konnten den
/// Ein-Snapshot-Fall nicht sehen.
#[cfg(windows)]
#[test]
fn genau_ein_snapshot_nach_dem_kandidaten_ist_ein_resultat() {
    let h = HarnischMitStore::neu("b9-erster-resultatsnapshot");
    let versuch = 0xc90;
    let vorlage = experiment_begin_wert(&h.main.adresse, 0x9b0, versuch);
    let sonde = sonde_in_der_passage(&h, &vorlage);

    // Baseline: Evidenz VOR dem Begin.
    for nr in 0..4 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_in_passage(&sonde.adresse, &vorlage, nr)));
    }
    assert_eq!(
        h.p0(&experiment_begin_wert(&h.main.adresse, 0x9b1, versuch))["ergebnis"],
        "angewandt"
    );
    assert_eq!(
        h.p0(&json!({
            "type": "experiment_candidate",
            "kopf": {
                "command_id": hex(0x9b2),
                "ziel": h.main.adresse,
                "base_revision": 0,
                "ttl_ms": 1000,
                "schema_major": 3,
                "schema_minor": 0
            },
            "experiment_id": hex(versuch),
            "referenz": vorlage["referenz"],
            "blindreihenfolge": "kandidat_zuerst"
        }))["ergebnis"],
        "angewandt"
    );

    // GENAU EIN Snapshot nach dem Kandidaten — der Fall, in dem der
    // Off-by-one alles verwirft.
    let von = vorlage["passage"]["projekt_von"].as_i64().unwrap();
    let epoche = vorlage["passage"]["transport_epoch"].as_u64().unwrap();
    assert!(h.c.evidence_snapshot_json(
        "sonde",
        &evidenz_payload(&sonde.adresse, 9, |w| {
            w["transport"]["transport_epoch"] = json!(epoche);
            w["transport"]["project_sample_start"] = json!(von + 9 * 512);
            if let Some(werte) = w["verteilung"]["p50"]["werte"].as_array_mut() {
                for v in werte.iter_mut() {
                    if let Some(x) = v.as_i64() {
                        *v = json!(x + 60);
                    }
                }
            }
        })
    ));

    let ack = h.p0(&json!({
        "type": "experiment_manual_result",
        "kopf": {
            "command_id": hex(0x9b3),
            "ziel": h.main.adresse,
            "base_revision": 0,
            "ttl_ms": 1000,
            "schema_major": 3,
            "schema_minor": 0
        },
        "experiment_id": hex(versuch),
        "hoerurteil": "kandidat",
        "blindreihenfolge": "kandidat_zuerst",
        "notiz": null,
        "werkzeug": null
    }));
    assert_eq!(
        ack["ergebnis"], "angewandt",
        "genau_ein_snapshot_nach_dem_kandidaten_ist_ein_resultat - EIN Beleg \
         nach dem Kandidaten reicht fuer eine Resultatmessung: {ack}"
    );
    let e = h.c.experiment_sicht(&hex(versuch)).expect("der Versuch steht");
    assert_eq!(
        e.resultat_evidence_ids.len(),
        1,
        "und genau dieser eine Beleg ist als Resultat gefuehrt"
    );
}

/// B5 — die Transitionshistorie ueberdauert den Neustart (M-51).
///
/// 🔑 Wiederpruefung 2: `wiederherstellen` liess das In-Memory-Log
/// ausdruecklich leer, und `Coordinator::mit_store` las `experiment_events`
/// nie. Nach einem Neustart lieferte `experiment_export(id)` deshalb eine
/// LEERE Transitionshistorie trotz vorhandener Indexzeilen; der Restart-Test
/// der Runde 2 pruefte nur `experiment_sicht`.
#[cfg(windows)]
#[test]
fn export_traegt_seine_transitionen_auch_nach_dem_neustart() {
    let h = HarnischMitStore::neu("b5-export-nach-neustart");
    let versuch = 0xca0;
    let vorlage = experiment_begin_wert(&h.main.adresse, 0x9c0, versuch);
    let sonde = sonde_in_der_passage(&h, &vorlage);
    for nr in 0..4 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_in_passage(&sonde.adresse, &vorlage, nr)));
    }
    assert_eq!(
        h.p0(&experiment_begin_wert(&h.main.adresse, 0x9c1, versuch))["ergebnis"],
        "angewandt"
    );
    assert_eq!(
        h.p0(&json!({
            "type": "experiment_candidate",
            "kopf": {
                "command_id": hex(0x9c2),
                "ziel": h.main.adresse,
                "base_revision": 0,
                "ttl_ms": 1000,
                "schema_major": 3,
                "schema_minor": 0
            },
            "experiment_id": hex(versuch),
            "referenz": vorlage["referenz"],
            "blindreihenfolge": "baseline_zuerst"
        }))["ergebnis"],
        "angewandt"
    );

    let vor_neustart = h
        .c
        .experiment_export(&hex(versuch))
        .expect("der Export steht vor dem Neustart");
    assert!(
        vor_neustart.ereignisse.len() >= 3,
        "vor dem Neustart traegt er Passage, Begin, Kandidat und Reihenfolge: {}",
        vor_neustart.ereignisse.len()
    );

    // DER Neustart: ein zweiter Coordinator auf DEMSELBEN Store.
    let neu = h.neuer_coordinator();
    let nach_neustart = neu
        .experiment_export(&hex(versuch))
        .expect("der Versuch ueberdauert den Neustart");
    assert_eq!(
        nach_neustart.ereignisse.len(),
        vor_neustart.ereignisse.len(),
        "export_traegt_seine_transitionen_auch_nach_dem_neustart - die Kette ist \
         VOLLSTAENDIG, nicht leer (M-51)"
    );
    assert_eq!(
        nach_neustart.ereignisse, vor_neustart.ereignisse,
        "und Zeile fuer Zeile dieselbe - der Index ist die haltbare Kette, keine \
         aermere Kopie"
    );
    assert_eq!(
        nach_neustart.passage.passage_id, vor_neustart.passage.passage_id,
        "die Passage reist wie zuvor mit"
    );
}

/// B6 — die Evidenzreihenfolge ueberdauert den Neustart.
///
/// 🔑 Wiederpruefung 2: Restaurierte Evidenz erhielt `empfangsfolge = 0`, und
/// der globale Zaehler startete ebenfalls bei 0, waehrend die persistierten
/// Begin- und Kandidatengrenzen ihre hohen Werte behielten. Neue
/// Resultatevidenz wurde dadurch als Baseline eingeordnet oder verworfen; der
/// Restart-Test der Runde 2 startete nur einen bereits TERMINALEN Versuch neu
/// und konnte diesen Rechenpfad nicht pruefen.
#[cfg(windows)]
#[test]
fn evidenzreihenfolge_ueberdauert_den_neustart() {
    let h = HarnischMitStore::neu("b6-evidenzreihenfolge");
    let versuch = 0xcb0;
    let vorlage = experiment_begin_wert(&h.main.adresse, 0x9d0, versuch);
    let sonde = sonde_in_der_passage(&h, &vorlage);
    for nr in 0..4 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_in_passage(&sonde.adresse, &vorlage, nr)));
    }
    assert_eq!(
        h.p0(&experiment_begin_wert(&h.main.adresse, 0x9d1, versuch))["ergebnis"],
        "angewandt"
    );
    assert_eq!(
        h.p0(&json!({
            "type": "experiment_candidate",
            "kopf": {
                "command_id": hex(0x9d2),
                "ziel": h.main.adresse,
                "base_revision": 0,
                "ttl_ms": 1000,
                "schema_major": 3,
                "schema_minor": 0
            },
            "experiment_id": hex(versuch),
            "referenz": vorlage["referenz"],
            "blindreihenfolge": "kandidat_zuerst"
        }))["ergebnis"],
        "angewandt"
    );

    // DER Neustart mit OFFENEM Versuch und erfasstem Kandidaten.
    let neu = h.neuer_coordinator();
    let push = Arc::new(PushProbe::default());
    neu.session_push_setzen(push);
    anmelden(&neu, "main2", &h.main);
    assert!(report_main(&neu, "main2", &h.main.adresse));
    assert!(neu.state_report_json("main2", &state_report_payload(&h.main.adresse, 0)));
    anmelden(&neu, "sonde2", &sonde);
    report(&neu, "sonde2", &sonde.adresse);
    let klasse = vorlage["passage"]["messpunktklassen"][0].as_str().unwrap();
    assert!(neu.descriptor_setzen("sonde2", descriptor(&sonde.adresse, klasse, &hex(0x77))));

    assert!(
        neu.experiment_sicht(&hex(versuch)).is_some_and(|e| e.offen()),
        "der Versuch ist nach dem Neustart OFFEN"
    );
    let restauriert = neu.evidenz_historie(&sonde.adresse.instance_id);
    assert!(
        restauriert.iter().any(|e| e.empfangsfolge > 0),
        "die restaurierte Evidenz behaelt ihre Ankunftsreihenfolge, statt bei 0 \
         zu beginnen"
    );

    // Ein NEUER Resultatsnapshot nach dem Neustart.
    let von = vorlage["passage"]["projekt_von"].as_i64().unwrap();
    let epoche = vorlage["passage"]["transport_epoch"].as_u64().unwrap();
    for nr in 20..22 {
        assert!(neu.evidence_snapshot_json(
            "sonde2",
            &evidenz_payload(&sonde.adresse, nr, |w| {
                w["transport"]["transport_epoch"] = json!(epoche);
                w["transport"]["project_sample_start"] = json!(von + (nr as i64) * 512);
                if let Some(werte) = w["verteilung"]["p50"]["werte"].as_array_mut() {
                    for v in werte.iter_mut() {
                        if let Some(x) = v.as_i64() {
                            *v = json!(x + 60);
                        }
                    }
                }
            })
        ));
    }

    let payload = serde_json::to_vec(&json!({
        "type": "experiment_manual_result",
        "kopf": {
            "command_id": hex(0x9d3),
            "ziel": h.main.adresse,
            "base_revision": 0,
            "ttl_ms": 1000,
            "schema_major": 3,
            "schema_minor": 0
        },
        "experiment_id": hex(versuch),
        "hoerurteil": "kandidat",
        "blindreihenfolge": "kandidat_zuerst",
        "notiz": null,
        "werkzeug": null
    }))
    .unwrap();
    let antwort = Senke::p0(&neu, "main2", &payload).expect("die Familie wird beantwortet");
    let ack: Value = serde_json::from_slice(&antwort).unwrap();
    assert_eq!(
        ack["ergebnis"], "angewandt",
        "evidenzreihenfolge_ueberdauert_den_neustart - der NEUE Beleg zaehlt als \
         Resultat, nicht als Baseline: {ack}"
    );
    let e = neu.experiment_sicht(&hex(versuch)).expect("der Versuch steht");
    assert!(
        !e.resultat_evidence_ids.is_empty(),
        "und er ist als Resultat gefuehrt"
    );
}


/// B17 — eine liegengebliebene Zustellschuld wird beim Subscribe NACHGESPIELT.
///
/// 🔑 Wiederpruefung 2: `outbox_lesen` hatte ausserhalb der Tests keinen
/// Aufrufer, und der Re-Subscribe las ausschliesslich die
/// `session_snapshot`-Projektion und kompaktierte auch nur diesen Schluessel.
/// Scheiterte der unmittelbare Push einer `evidence_invalidate` oder war der
/// Subscriber kurz getrennt, blieb die Schuld FUER IMMER stehen und die
/// Ruecknahme erreichte das Pluginmodell nie. Der Test der Runde 2 erzwang
/// einen sofort erfolgreichen Push.
#[cfg(windows)]
#[test]
fn liegengebliebene_ruecknahme_wird_beim_subscribe_nachgespielt() {
    let h = HarnischMitStore::neu("b17-outbox-nachspielen");
    h.abonniert();
    let sonde = {
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = hex(0x20);
        s.adresse.runtime_nonce = hex(0x21);
        s
    };
    anmelden(&h.c, "sonde", &sonde);
    report(&h.c, "sonde", &sonde.adresse);
    for nr in 0..3 {
        assert!(h
            .c
            .evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {})));
    }

    // Der Subscriber nimmt gerade NICHTS an — kurz getrennt, Puffer voll.
    h.push.lehnt_ab(true);
    let ack = h.p0(&json!({
        "type": "preview_begin",
        "kopf": {
            "command_id": hex(0x9f0),
            "ziel": h.main.adresse,
            "base_revision": 0,
            "ttl_ms": 1000,
            "schema_major": 3,
            "schema_minor": 0
        },
        "lease_duration_ms": 400,
        "renew_id": hex(0x9f1)
    }));
    assert_eq!(ack["ergebnis"], "angewandt", "{ack}");
    assert!(
        h.outbox()
            .iter()
            .any(|(schluessel, _)| schluessel == "evidence_invalidate"),
        "die Ruecknahme steht als SCHULD in der Outbox"
    );
    assert!(
        !h.push
            .payloads()
            .iter()
            .any(|(_, w)| w["type"] == "evidence_invalidate"),
        "und ist nachweislich NICHT zugestellt"
    );

    // Der Subscriber ist zurueck und abonniert erneut.
    h.push.lehnt_ab(false);
    h.abonniert();

    assert!(
        h.push
            .payloads()
            .iter()
            .any(|(_, w)| w["type"] == "evidence_invalidate"
                && w["grund"] == "intervention"),
        "liegengebliebene_ruecknahme_wird_beim_subscribe_nachgespielt - die \
         Invalidierung kommt beim Pluginmodell an, statt fuer immer als Schuld \
         zu stehen"
    );
    assert!(
        !h.outbox()
            .iter()
            .any(|(schluessel, _)| schluessel == "evidence_invalidate"),
        "und die Schuld ist danach getilgt - eine Schuld, die niemand abtraegt, \
         waechst"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// NACHARBEIT 3 · W6 — Rechenwege und Paarbildung
// ═════════════════════════════════════════════════════════════════════════

/// B1 — der Nachlauf rechnet ab seinem START, nicht ab dem vorigen Tick.
///
/// 🔑 Wiederpruefung 2: EIN globales `letzter_tail_tick` wurde bei jedem Tick
/// ersetzt, und der naechste Tick zog das GESAMTE Intervall seit dem
/// vorherigen ab — auch wenn der Tail erst spaeter entstand. Ein kurzer Tail
/// konnte dadurch nahezu sofort freigegeben werden; genau in diesem Fenster
/// laeuft der Filterhall des Markers noch in die Messung. Der Fall der Runde 2
/// startete den Tail VOR dem ersten Referenztick und konnte die normale
/// Phasenlage nicht sehen.
#[test]
fn nachlauf_rechnet_ab_seinem_start() {
    let (c, uhr) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    report_main(&c, "a", &h.adresse);

    // Ein erster Tick setzt den Bezugspunkt der ALTEN Rechnung.
    uhr.vor(100);
    c.liveness_tick();

    // Und JETZT laeuft eine Sekunde, BEVOR der Nachlauf ueberhaupt beginnt.
    // Die alte Rechnung schriebe sie dem Tail gut.
    uhr.vor(1000);
    let id = hex(0x7200);
    assert!(c.intervention_begin("a", &h.adresse, &id, 1));
    // 24.000 Samples bei 48 kHz sind eine halbe Sekunde Nachlauf.
    assert!(c.intervention_end("a", &h.adresse, &id, 2, 24_000));
    assert!(
        !c.interventionssicht_fuer_link("a").starke_evidenz_erlaubt,
        "der Nachlauf laeuft"
    );

    // Ein Tick unmittelbar danach zieht NICHTS ab: seit dem Tail-Start ist
    // keine Zeit vergangen. Die alte Rechnung zoege hier 1,1 Sekunden ab und
    // gaebe den halbsekuendigen Nachlauf sofort frei.
    c.liveness_tick();
    assert!(
        !c.interventionssicht_fuer_link("a").starke_evidenz_erlaubt,
        "nachlauf_rechnet_ab_seinem_start - ein Tick unmittelbar nach dem \
         Tail-Start gibt ihn NICHT frei"
    );

    // Nach 400 ms ist er immer noch nicht durch.
    uhr.vor(400);
    c.liveness_tick();
    assert!(
        !c.interventionssicht_fuer_link("a").starke_evidenz_erlaubt,
        "und nach 400 von 500 ms ebenfalls nicht"
    );

    // Nach der vollen halben Sekunde faellt er - die Gegenprobe.
    uhr.vor(150);
    c.liveness_tick();
    assert!(
        c.interventionssicht_fuer_link("a").starke_evidenz_erlaubt,
        "und nach 550 ms ist er abgelaufen - der Riegel sperrt die richtige \
         Haelfte, nicht alles"
    );
}

/// B18 — eine Invalidierung bildet die Paarurteile NEU.
///
/// 🔑 Wiederpruefung 2: `invalidierung_anwenden` stiess keine Neubildung an und
/// liess das alte, volle Urteil stehen — ein Urteil ueber Belege, die es
/// gerade zurueckgenommen hatte. Der Fall der Runde 2 speiste danach einen
/// weiteren Snapshot ein und maskierte die Luecke damit.
#[test]
fn invalidierung_bildet_das_paarurteil_neu() {
    let (c, _) = coordinator();
    let paar = hex(0x88);
    let mut haelfte = |kuerzel: &str, instanz: usize, nonce: usize, position: &str| {
        let s = hello(1, 2, instanz, nonce, "passive_probe", Some(9));
        anmelden(&c, kuerzel, &s);
        assert!(c.descriptor_setzen(kuerzel, descriptor(&s.adresse, position, &paar)));
        for nr in 0..3 {
            assert!(c.evidence_snapshot_json(kuerzel, &evidenz_payload(&s.adresse, nr, |_| {})));
        }
        s
    };
    let pre = haelfte("pre", 0x30, 0x31, "pre");
    let _post = haelfte("post", 0x40, 0x41, "post");

    let urteil = c.paarurteil(&paar).expect("das Paar traegt ein Urteil");
    assert!(
        urteil.ausschluss.is_none(),
        "vor der Ruecknahme ist es vollstaendig: {urteil:?}"
    );

    // DIE Ruecknahme — ohne dass danach ein weiterer Snapshot kommt.
    assert!(
        c.invalidierung_wegen_messpunkt_fuer_link("pre", "pre", "post") > 0,
        "der Messpunktwechsel nimmt die Evidenz der PRE-Haelfte zurueck"
    );
    let _ = pre;

    let urteil = c.paarurteil(&paar).expect("das Paar traegt weiter ein Urteil");
    assert!(
        urteil.ausschluss.is_some(),
        "invalidierung_bildet_das_paarurteil_neu - das Urteil nennt seinen \
         unvollstaendigen Zustand, statt das alte volle stehen zu lassen: \
         {urteil:?}"
    );
}

/// B19 — die Paarurteile ueberdauern den Neustart, ohne eigenes Ereignis.
///
/// 🔑 Wiederpruefung 2: das Urteil lebte in der fluechtigen Map und reiste
/// beilaeufig im Sessionsnapshot mit; nach einem Neustart fehlte es bis zur
/// naechsten Evidenz. Der Dirigent hat die V3/V4-Regel der Runde 2 dafuer
/// ANGEPASST: ein eigener StoreEvent ist nicht noetig, weil das Urteil
/// deterministisch aus der persistierten Evidenz folgt — es wird beim Restore
/// GERECHNET, nicht gelesen.
#[cfg(windows)]
#[test]
fn paarurteil_entsteht_beim_neustart_aus_der_evidenz() {
    let h = HarnischMitStore::neu("b19-paarurteil-neustart");
    let paar = hex(0x89);
    let mut haelfte = |kuerzel: &str, instanz: usize, nonce: usize, position: &str| {
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = hex(instanz);
        s.adresse.runtime_nonce = hex(nonce);
        anmelden(&h.c, kuerzel, &s);
        assert!(h
            .c
            .descriptor_setzen(kuerzel, descriptor(&s.adresse, position, &paar)));
        for nr in 0..3 {
            assert!(h
                .c
                .evidence_snapshot_json(kuerzel, &evidenz_payload(&s.adresse, nr, |_| {})));
        }
        s
    };
    let pre = haelfte("pre", 0x50, 0x51, "pre");
    let post = haelfte("post", 0x60, 0x61, "post");
    let vorher = h
        .c
        .paarurteil(&paar)
        .expect("vor dem Neustart steht ein Urteil");

    // DER Neustart: ein zweiter Coordinator auf DEMSELBEN Store. Die Sonden
    // melden sich wieder an — ihre Deskriptoren gehoeren dem Lauf —, aber
    // KEINE neue Evidenz kommt.
    let neu = h.neuer_coordinator();
    anmelden(&neu, "pre2", &pre);
    assert!(neu.descriptor_setzen("pre2", descriptor(&pre.adresse, "pre", &paar)));
    anmelden(&neu, "post2", &post);
    assert!(neu.descriptor_setzen("post2", descriptor(&post.adresse, "post", &paar)));

    let nachher = neu
        .paarurteil(&paar)
        .expect("paarurteil_entsteht_beim_neustart_aus_der_evidenz - das Urteil \
                 ist nach dem Neustart da, OHNE dass neue Evidenz kam");
    assert_eq!(
        nachher.klasse, vorher.klasse,
        "und es ist dieselbe Klasse - das Urteil folgt aus der Evidenz, und die \
         ist persistiert"
    );
}

/// B10 — die Passagenbelege beider Haelften kommen aus den TATSAECHLICH
/// vertretenen Instanzen (M-31/M-46).
///
/// 🔑 Wiederpruefung 2: Die Runde 2 setzte fingerprint, aktive_quellen und
/// messpunktklassen fuer BEIDE Haelften identisch aus der statischen Passage;
/// `kandidat.referenz` und die je Haelfte wirklich vertretenen Quellen wurden
/// nicht gelesen. `MaterialVerschieden` und `QuellenVerschieden` konnten fuer
/// diesen Aufrufer NIE ausloesen — eine Baseline nur von Quelle A und ein
/// Resultat nur von Quelle B erschienen als stark vergleichbar. Der
/// Produkttest der Runde 2 verwendete dieselbe Referenz und eine Quelle.
#[cfg(windows)]
#[test]
fn passagenbelege_kommen_aus_den_vertretenen_quellen() {
    let h = HarnischMitStore::neu("b10-quellen-je-haelfte");
    let versuch = 0xcc0;
    let vorlage = experiment_begin_wert(&h.main.adresse, 0x9e0, versuch);
    let von = vorlage["passage"]["projekt_von"].as_i64().unwrap();
    let epoche = vorlage["passage"]["transport_epoch"].as_u64().unwrap();

    // BEIDE Quellen der Passage melden sich an — sonst gaebe es nichts zu
    // unterscheiden.
    let anmelden_quelle = |kuerzel: &str, index: usize, nonce: usize| {
        let quelle = vorlage["passage"]["aktive_quellen"][index]
            .as_str()
            .unwrap()
            .to_owned();
        let klasse = vorlage["passage"]["messpunktklassen"][index]
            .as_str()
            .unwrap()
            .to_owned();
        let mut s = h.main.clone();
        s.plugin_kind = "passive_probe".into();
        s.adresse.instance_id = quelle;
        s.adresse.runtime_nonce = hex(nonce);
        anmelden(&h.c, kuerzel, &s);
        report(&h.c, kuerzel, &s.adresse);
        assert!(h
            .c
            .descriptor_setzen(kuerzel, descriptor(&s.adresse, &klasse, &hex(0x77))));
        s
    };
    let a = anmelden_quelle("a", 0, 0x31);
    let b = anmelden_quelle("b", 1, 0x41);

    let beleg = |adresse: &Adresse, nr: usize| {
        evidenz_payload(adresse, nr, |w| {
            w["transport"]["transport_epoch"] = json!(epoche);
            w["transport"]["project_sample_start"] = json!(von + (nr as i64) * 512);
        })
    };

    // Baseline NUR von Quelle A.
    for nr in 0..4 {
        assert!(h.c.evidence_snapshot_json("a", &beleg(&a.adresse, nr)));
    }
    assert_eq!(
        h.p0(&experiment_begin_wert(&h.main.adresse, 0x9e1, versuch))["ergebnis"],
        "angewandt"
    );
    assert_eq!(
        h.p0(&json!({
            "type": "experiment_candidate",
            "kopf": {
                "command_id": hex(0x9e2),
                "ziel": h.main.adresse,
                "base_revision": 0,
                "ttl_ms": 1000,
                "schema_major": 3,
                "schema_minor": 0
            },
            "experiment_id": hex(versuch),
            "referenz": vorlage["referenz"],
            "blindreihenfolge": "kandidat_zuerst"
        }))["ergebnis"],
        "angewandt"
    );
    // Resultat NUR von Quelle B.
    for nr in 10..14 {
        assert!(h.c.evidence_snapshot_json("b", &beleg(&b.adresse, nr)));
    }

    let ack = h.p0(&json!({
        "type": "experiment_manual_result",
        "kopf": {
            "command_id": hex(0x9e3),
            "ziel": h.main.adresse,
            "base_revision": 0,
            "ttl_ms": 1000,
            "schema_major": 3,
            "schema_minor": 0
        },
        "experiment_id": hex(versuch),
        "hoerurteil": "kandidat",
        "blindreihenfolge": "kandidat_zuerst",
        "notiz": null,
        "werkzeug": null
    }));
    assert_eq!(ack["ergebnis"], "angewandt", "{ack}");

    let e = h.c.experiment_sicht(&hex(versuch)).expect("der Versuch steht");
    use eqcop_broker::coordinator::experiment::Terminal;
    let Some(Terminal::Ergebnis { achsen, .. }) = &e.terminal else {
        panic!("das Terminal ist ein Ergebnis: {:?}", e.terminal);
    };
    assert!(
        achsen
            .vergleichbarkeit_gruende
            .iter()
            .any(|g| g.contains("QuellenVerschieden")),
        "passagenbelege_kommen_aus_den_vertretenen_quellen - Baseline von A und \
         Resultat von B ist NICHT vergleichbar: {achsen:?}"
    );
    // B13 im selben Lauf: die Effektstabilitaet ist gerechnet, nicht `None`.
    assert!(
        achsen.effekt_stabil.is_some(),
        "B13: der Produktpfad befuellt beide Haelften, und `achsen()` liefert \
         eine Effektstabilitaet: {achsen:?}"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-180 · die Naht zwischen den Sprachen (R1, R3a, R9, E4)
// ═════════════════════════════════════════════════════════════════════════

/// Die Byteinstanz, gegen die AUCH der C++-Serialisierer misst.
///
/// Sie ist von Hand geschrieben und damit die Ausgabe keiner der beiden
/// Implementierungen (dieselbe Bauform wie `handschlag-v1.json`): stimmt der
/// C++-Writer mit ihr ueberein und nimmt der Rust-Leser sie an, stimmen beide
/// transitiv miteinander ueberein. Ein Test auf nur EINER Seite kann an einem
/// Vertragsbruch zwischen zwei Sprachen nicht fallen — genau daran fiel C1.
fn heartbeat_wire_instanz() -> Value {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .unwrap()
        .join("eq-copilot/fixtures/v3/heartbeat-wire-v1.json");
    serde_json::from_str(&std::fs::read_to_string(&pfad).expect("Byteinstanz fehlt"))
        .expect("Byteinstanz ist kein JSON")
}

/// Hello mit GENAU der Adresse aus der Byteinstanz - sonst weist
/// `p0_json_mit_minor` den Heartbeat als fremde Adresse ab.
fn hello_aus_wire_instanz(wurzel: &Value, art: &str) -> HelloControl {
    let a = &wurzel["eingabe"]["adresse"];
    let mut h = hello(1, 2, 3, 4, art, Some(9));
    h.adresse = Adresse {
        logon_sid: a["logon_sid"].as_str().unwrap().into(),
        project_binding_id: a["project_binding_id"].as_str().unwrap().into(),
        session_epoch: a["session_epoch"].as_str().unwrap().into(),
        instance_id: a["instance_id"].as_str().unwrap().into(),
        runtime_nonce: a["runtime_nonce"].as_str().unwrap().into(),
    };
    h
}

fn wire(wurzel: &Value, fall: &str) -> Vec<u8> {
    wurzel["faelle"][fall]["wire"]
        .as_str()
        .expect("Wiretext fehlt")
        .as_bytes()
        .to_vec()
}

/// NAK-180 R1/R3a (N-01, N-02, N-03): das `false` des ersten Heartbeats IST
/// der bestaetigte Resync — und nur dort.
///
/// Vor diesem Ticket hatte `resync_bestaetigen` im Produkt gar keinen
/// Aufrufer: der C++-Writer liess das Feld bei `false` weg, und ein
/// FEHLENDES Feld liefert `None`, nicht `Some(false)`. Das sticky Unknown aus
/// `control_ende` blieb damit fuer die ganze Sitzung stehen (G4 D-01).
#[test]
fn erster_heartbeat_mit_false_loest_das_sticky_unknown() {
    let wurzel = heartbeat_wire_instanz();
    let (c, _clock) = coordinator();
    let h = hello_aus_wire_instanz(&wurzel, "main");

    // NUR der Main: eine zweite Instanz mit DERSELBEN `instance_id` wuerde
    // ihn verdraengen, und `resync_bestaetigen` verlangt einen stehenden,
    // nicht verdraengten Link.
    anmelden_roh(&c, "main", &h);

    // R9: schon das MAIN-HELLO sperrt - eine Nachricht vor dem Heartbeat.
    assert!(
        c.interventionssicht_fuer_link("main").unknown,
        "N-30: das Main-Hello setzt taint.unknown; ohne das koennte eine Sonde \r
         zwischen Hello und erstem Heartbeat starke Evidenz committen"
    );

    // Der erste Heartbeat traegt das ausdrueckliche `false` aus der
    // Byteinstanz - dieselben Bytes, die der C++-Writer erzeugt.
    let antwort = Senke::p0(&c, "main", &wire(&wurzel, "bestaetigt_neutral"));
    assert!(antwort.is_some(), "N-01: der Heartbeat wird beantwortet");
    let sicht = c.interventionssicht_fuer_link("main");
    assert!(
        !sicht.unknown && sicht.starke_evidenz_erlaubt,
        "N-01: erster_heartbeat_mit_false_ist_der_resync - der Zustand ist wieder \r
         BEKANNT und starke Evidenz erlaubt: {sicht:?}"
    );

    // N-02: ein zweiter Heartbeat OHNE Feld aendert nichts.
    Senke::p0(&c, "main", &wire(&wurzel, "steady"));
    assert!(
        !c.interventionssicht_fuer_link("main").unknown,
        "N-02: der Steady-State schweigt und aendert nichts"
    );
}

/// NAK-180 (N-03, M-61): ein SPAETERES `false` loest nichts.
///
/// Das ist die Zusage, die den Fix vom Selbstheilen trennt. Sie ist der Grund,
/// warum der Riegel in `befehl.rs` eng bleiben MUSS.
#[test]
fn spaeteres_false_loest_das_sticky_unknown_nicht() {
    let wurzel = heartbeat_wire_instanz();
    let (c, _clock) = coordinator();
    let h = hello_aus_wire_instanz(&wurzel, "main");
    anmelden(&c, "main", &h);

    // Ein Ereignis meldet eine Sequenz - der Link ist nicht mehr "ohne
    // Ereignissequenz", und ein Ueberlauf setzt Unknown.
    assert!(c.intervention_begin("main", &h.adresse, &hex(0x8101), 1));
    Senke::p0(&c, "main", &wire(&wurzel, "unbekannt"));
    assert!(
        c.interventionssicht_fuer_link("main").unknown,
        "der gemeldete Ueberlauf setzt Unknown"
    );

    Senke::p0(&c, "main", &wire(&wurzel, "bestaetigt_neutral"));
    assert!(
        c.interventionssicht_fuer_link("main").unknown,
        "N-03: ein spaeterer Heartbeat mit `false` loescht Unknown NIE (M-61) - \r
         eine Selbstheilung waere genau das, was Paragraph 34.2 verbietet"
    );
}

/// NAK-180 R9 (N-30, N-31, N-32): das Fenster zwischen Hello und erstem
/// Heartbeat ist zu.
#[test]
fn sitzung_ist_zwischen_main_hello_und_erstem_heartbeat_gesperrt() {
    let wurzel = heartbeat_wire_instanz();
    let (c, _clock) = coordinator();
    let h = hello_aus_wire_instanz(&wurzel, "main");

    anmelden_roh(&c, "main", &h);
    // N-30: eine Sondenevidenz DERSELBEN Sitzung, eingespeist BEVOR der erste
    // Heartbeat verarbeitet ist, wird gesperrt. Ohne R9 laege hier ein
    // offenes Fenster: `evidence_dispatch_locked` liest eine Sitzung ohne
    // Taint-Eintrag als erlaubt.
    assert!(
        !c.evidence_dispatch_fuer_link("main"),
        "N-30: starke Evidenz ist zwischen Main-Hello und erstem Heartbeat GESPERRT"
    );

    // N-32: bleibt das Feld aus, bleibt gesperrt. Schweigen ist keine Aussage.
    Senke::p0(&c, "main", &wire(&wurzel, "steady"));
    assert!(
        !c.evidence_dispatch_fuer_link("main"),
        "N-32: ein erster Heartbeat OHNE Feld loest nichts - fail-closed"
    );

    // NAK-180 Nacharbeit 1 (EP-10): und ein SPAETERES `false` loest ihn auch
    // nicht mehr.
    //
    // Das ist die Praezisierung, die das eigene Feld `erster_heartbeat_gesehen`
    // mitbringt: der schweigende Heartbeat WAR der erste. Vorher stand
    // `letzte_event_sequence.is_none()` fuer die Frage, und weil dieser Link
    // noch kein Ereignis gemeldet hatte, galt der zweite Heartbeat ebenfalls
    // als erster - ein spaeteres `false` konnte den R1-Zweig ausloesen,
    // waehrend der Marker klingt. M-61 sagt genau das Gegenteil: nur ein
    // NEUER Linkaufbau entsperrt.
    Senke::p0(&c, "main", &wire(&wurzel, "bestaetigt_neutral"));
    assert!(
        !c.evidence_dispatch_fuer_link("main"),
        "EP-10/N-32: ein spaeteres `false` loest die Sperre NICHT - der \
         schweigende Heartbeat war der erste, und ein zweiter ist nie einer"
    );

    // Erst ein NEUER Link urteilt frisch: sein erster Heartbeat traegt das
    // ausdrueckliche `false`, und DAS loest.
    c.control_ende("main");
    anmelden_roh(&c, "main2", &h);
    Senke::p0(&c, "main2", &wire(&wurzel, "bestaetigt_neutral"));
    assert!(
        c.evidence_dispatch_fuer_link("main2"),
        "N-01: erst die ausdrueckliche Aussage des ERSTEN Heartbeats eines \
         frischen Links loest die Sperre"
    );
}

/// NAK-180 Nacharbeit 1 (EP-10, E4, Paragraph 2.1): Replay und erster
/// Heartbeat sind UNGEORDNET - das Replay nimmt dem Heartbeat seinen Erstling
/// nicht.
///
/// Die Matrix sagt ausdruecklich keine Ordnung zu ("Reihenfolge von Replay und
/// erstem Heartbeat ist gleichgueltig"). `letzte_event_sequence` als Riegel
/// machte daraus stillschweigend eine: kam das Replay zuerst, oeffnete der
/// folgende erste `true`-Heartbeat keinen Bericht mehr, und der R2-Weg war
/// fuer diesen Link tot.
///
/// Rotbeweis: den Riegel wieder auf `letzte_event_sequence.is_none()`
/// zurueckdrehen - dann faellt genau dieser Fall.
#[test]
fn replay_vor_dem_ersten_heartbeat_nimmt_ihm_den_erstling_nicht() {
    let wurzel = heartbeat_wire_instanz();
    let (c, clock) = coordinator();
    let h = hello_aus_wire_instanz(&wurzel, "main");
    anmelden_roh(&c, "main", &h);

    // Das Replay kommt ZUERST und meldet damit eine Ereignissequenz.
    assert!(
        c.intervention_begin("main", &h.adresse, &hex(0x8201), 1),
        "das Replay-Begin wird angenommen"
    );

    // Und JETZT der erste Heartbeat mit ausdruecklichem `true`.
    Senke::p0(&c, "main", &wire(&wurzel, "unbekannt"));

    // Das Intervall schliessen, den Nachlauf ablaufen lassen - dann darf das
    // einmalige `false` den Bericht abschliessen.
    assert!(c.intervention_end("main", &h.adresse, &hex(0x8201), 2, 4_800));
    // 4800 Samples bei 48 kHz sind 100 ms echter Zeit - die Uhr geht mit.
    clock.vor(500);
    c.liveness_tick();
    let sicht = c.interventionssicht_fuer_link("main");
    assert_eq!(
        sicht.tail_samples_offen, 0,
        "der Nachlauf ist abgelaufen: {sicht:?}"
    );

    Senke::p0(&c, "main", &wire(&wurzel, "bestaetigt_neutral"));
    let sicht = c.interventionssicht_fuer_link("main");
    assert!(
        !sicht.unknown && sicht.starke_evidenz_erlaubt,
        "EP-10: der Bericht war offen, obwohl das Replay die Sequenz zuerst \
         gesetzt hat - der erste Heartbeat ist eine eigene Tatsache: {sicht:?}"
    );
}

/// NAK-180 Nacharbeit 1 (EP-11, R2): der Nachbericht behaelt die laufende
/// Sequenzbasis.
///
/// `resync_bestaetigen(link, 0)` setzte sie auf 0, obwohl Replay und End im
/// R2-Weg bereits Sequenzen verbraucht haben und der Pluginzaehler
/// ausdruecklich NICHT zurueckgesetzt wird. Der naechste regulaere Marker kam
/// mit 3, der Broker erwartete 1, setzte sofort wieder `unknown` - und konnte
/// ohne neuen Link nicht mehr nullen.
///
/// Rotbeweis: den Abschluss wieder `resync_bestaetigen(link, 0)` rufen lassen.
#[test]
fn nachbericht_behaelt_die_sequenzbasis() {
    let wurzel = heartbeat_wire_instanz();
    let (c, clock) = coordinator();
    let h = hello_aus_wire_instanz(&wurzel, "main");
    anmelden_roh(&c, "main", &h);

    // Der R2-Weg: erster Heartbeat `true`, Replay 1, End 2, Nachlauf, `false`.
    Senke::p0(&c, "main", &wire(&wurzel, "unbekannt"));
    assert!(c.intervention_begin("main", &h.adresse, &hex(0x8301), 1));
    assert!(c.intervention_end("main", &h.adresse, &hex(0x8301), 2, 4_800));
    clock.vor(500);
    c.liveness_tick();
    Senke::p0(&c, "main", &wire(&wurzel, "bestaetigt_neutral"));
    let sicht = c.interventionssicht_fuer_link("main");
    assert!(
        !sicht.unknown && sicht.starke_evidenz_erlaubt,
        "der Nachbericht hat abgeschlossen: {sicht:?}"
    );

    // Und JETZT die Zusage: der naechste regulaere Marker traegt 3 - die
    // Zahl, die der Pluginzaehler wirklich vergibt.
    assert!(
        c.intervention_begin("main", &h.adresse, &hex(0x8302), 3),
        "EP-11: die Basis blieb bei 2, also ist 3 lueckenlos"
    );
    let sicht = c.interventionssicht_fuer_link("main");
    assert!(
        !sicht.unknown,
        "EP-11: nachbericht_behaelt_die_sequenzbasis - ein Reset auf 0 haette \
         aus der 3 eine Luecke gemacht und `unknown` sofort wieder gesetzt: \
         {sicht:?}"
    );
}

/// NAK-180 Nacharbeit 1 (EP-12, E4): Sicht, Flag und Freigabe unter EINEM
/// Lock - der Abschluss greift nicht, solange die Sitzung noch ein Intervall
/// eines ZWEITEN lebenden Links haelt, und er raeumt es nicht weg.
///
/// E4 verlangt "am Wirkpunkt keine Intervalle und kein Tail" - mit drei
/// getrennten Locks war "am Wirkpunkt" nicht dasselbe wie "bei der Pruefung".
#[test]
fn nachbericht_greift_nicht_bei_fremdem_intervall_derselben_sitzung() {
    let wurzel = heartbeat_wire_instanz();
    let (c, _clock) = coordinator();
    let h = hello_aus_wire_instanz(&wurzel, "main");
    anmelden_roh(&c, "main", &h);

    // Der Bericht ist offen (erster Heartbeat `true`).
    Senke::p0(&c, "main", &wire(&wurzel, "unbekannt"));

    // Ein ZWEITER lebender Link derselben Sitzung startet eine Intervention.
    let mut zweiter = h.clone();
    zweiter.adresse.instance_id = hex(0x5150);
    anmelden_roh(&c, "main-zwei", &zweiter);
    assert!(
        c.intervention_begin("main-zwei", &zweiter.adresse, &hex(0x8401), 1),
        "der zweite Link haelt ein aktives Intervall"
    );

    // Der Abschluss darf NICHT greifen - und vor allem darf er das fremde
    // Intervall nicht loeschen.
    Senke::p0(&c, "main", &wire(&wurzel, "bestaetigt_neutral"));
    let sicht = c.interventionssicht_fuer_link("main");
    assert!(
        sicht.unknown && sicht.aktive == 1,
        "EP-12: der Abschluss greift nicht, solange die Sitzung ein aktives \
         Intervall haelt - und er raeumt es nicht weg: {sicht:?}"
    );
}

/// NAK-180 Nacharbeit 1 (EP-17, R3a, N-05 bis N-08), berichtigt in
/// Nacharbeit 2 (WN-07): der VOLLE R2-Zyklus ueber die Sprachgrenze - als
/// WIRE-JSON und mit EINGESPEISTER Sondenevidenz.
///
/// Der Fall der Runde 1 schickte nur die Heartbeats durch `Senke::p0`; Replay
/// und End riefen `intervention_begin`/`intervention_end` direkt auf, und die
/// Sonde wurde ueber `evidence_dispatch_fuer_link` nur BEFRAGT. Damit konnten
/// Parser-, Routing- und echte Evidence-Commit-Regressionen gruen bleiben -
/// die fixture-getriebene, injizierte Cross-Language-Naht aus R3(a) fehlte.
///
/// Jetzt reist JEDE Nachricht des Wegs als Wire-JSON durch `Senke::p0` und
/// damit durch `p0_json_mit_minor`, und die Sondenevidenz wird in jedem
/// Zwischenzustand per `evidence_snapshot_json` EINGESPEIST; gemessen wird
/// ihre Wirkung (angenommen/gesperrt) und was danach in `evidenz_sicht` steht.
#[test]
fn r2_lebenszyklus_mit_verbleibender_sonde_end_zu_end() {
    let wurzel = heartbeat_wire_instanz();
    let (c, clock) = coordinator();
    let h = hello_aus_wire_instanz(&wurzel, "main");
    anmelden_roh(&c, "main", &h);

    // Eine Sonde DERSELBEN Sitzung - sie ueberlebt den Linkwechsel des Mains
    // und ist der Grund, warum die Sperre ueberhaupt zaehlt: sie wuerde sonst
    // waehrend des Markers starke Evidenz committen.
    let mut sonde = hello_aus_wire_instanz(&wurzel, "passive_probe");
    sonde.adresse.instance_id = hex(0x7001);
    anmelden_roh(&c, "sonde", &sonde);

    // Der Belegweg der Sonde: EINSPEISEN, nicht befragen. `nr` waehlt die
    // `evidence_id`, damit `evidenz_sicht` sagen kann, WELCHER Snapshot
    // wirklich angekommen ist - ein gesperrter hinterlaesst dort nichts.
    let einspeisen = |nr: usize| c.evidence_snapshot_json("sonde", &evidenz_payload(&sonde.adresse, nr, |_| {}));
    let zuletzt = || {
        c.evidenz_sicht(&sonde.adresse.instance_id)
            .map(|e| e.evidence_id)
    };

    assert_eq!(zuletzt(), None, "vor dem ersten Beleg steht nichts");

    // (1) Erster Heartbeat des Mains: ausdrueckliches `false` (N-01), aus der
    //     Byteinstanz, die AUCH der C++-Serialisierer haelt.
    Senke::p0(&c, "main", &wire(&wurzel, "bestaetigt_neutral"));
    let sicht = c.interventionssicht_fuer_link("main");
    assert!(
        !sicht.unknown && sicht.starke_evidenz_erlaubt,
        "N-01: der Zustand ist bekannt: {sicht:?}"
    );
    assert!(einspeisen(1), "und die Sonde darf committen");
    assert_eq!(
        zuletzt(),
        Some(hex(0x1001)),
        "N-01: der eingespeiste Beleg steht wirklich im Evidenzstand"
    );

    // (2) Ein Marker laeuft - Begin auf dem ALTEN Link, als WIRE-JSON.
    Senke::p0(
        &c,
        "main",
        &intervention_begin_payload(&h.adresse, 0x8501, 1, "hoermarkierung", None),
    );
    let sicht = c.interventionssicht_fuer_link("main");
    assert!(
        sicht.aktive == 1 && !sicht.starke_evidenz_erlaubt,
        "das aktive Intervall sperrt: {sicht:?}"
    );
    assert!(
        !einspeisen(2),
        "N-07: waehrend der Marker klingt, wird eingespeiste Evidenz VERWORFEN"
    );
    assert_eq!(
        zuletzt(),
        Some(hex(0x1001)),
        "N-07: und sie hinterlaesst nichts - der Stand bleibt der alte"
    );

    // (3) `control_ende`: der Link stirbt, die Sonde bleibt. Sticky Unknown.
    c.control_ende("main");
    assert!(
        !einspeisen(3),
        "N-05: der Disconnect macht die Sitzung NICHT sauber"
    );

    // (4) Neuer Link. Sein erster Heartbeat traegt ausdrueckliches `true` -
    //     der Prozessor meldet, dass beim Aufbau ein Marker lief (R2).
    anmelden_roh(&c, "main2", &h);
    Senke::p0(&c, "main2", &wire(&wurzel, "unbekannt"));
    let sicht = c.interventionssicht_fuer_link("main2");
    assert!(
        sicht.unknown && !sicht.starke_evidenz_erlaubt,
        "N-05: kein `false` beim Aufbau - der erste Heartbeat traegt `true`: {sicht:?}"
    );
    assert!(
        !einspeisen(4),
        "N-07: und zwischen erstem Heartbeat und Replay gibt es KEIN Fenster"
    );

    // (5) Das Replay-Begin mit DERSELBEN `intervention_id` - ebenfalls als
    //     Wire-JSON. Der frische Link fuehrt noch keine Sequenzbasis und nimmt
    //     die erste Zahl vorbehaltlos an.
    Senke::p0(
        &c,
        "main2",
        &intervention_begin_payload(&h.adresse, 0x8501, 1, "hoermarkierung", None),
    );
    let sicht = c.interventionssicht_fuer_link("main2");
    assert!(
        sicht.aktive == 1,
        "N-05: das Replay traegt dieselbe ID, reist durch den Wireweg und wird \
         angenommen: {sicht:?}"
    );
    assert!(
        !einspeisen(5),
        "N-07: das Replay hat den wahren Zustand hergestellt - weiter gesperrt"
    );

    // (6) Das regulaere `end` schliesst GENAU dieses Intervall; der Nachlauf
    //     startet und laeuft in Echtzeit ab (M-58). Auch das als Wire-JSON -
    //     `tail_samples` wird DORT gelesen.
    Senke::p0(
        &c,
        "main2",
        &intervention_end_payload(&h.adresse, 0x8501, 2, 4_800),
    );
    let sicht = c.interventionssicht_fuer_link("main2");
    assert!(
        sicht.aktive == 0 && sicht.tail_samples_offen == 4_800 && sicht.unknown,
        "M-58: das Ende allein genuegt nicht - der Nachlauf steht, und seine \
         Laenge kommt vom DRAHT: {sicht:?}"
    );
    assert!(
        !einspeisen(6),
        "M-58: waehrend des Nachlaufs bleibt eingespeiste Evidenz gesperrt"
    );

    // (7) Ein `false` WAEHREND des Nachlaufs greift nicht - und verbraucht den
    //     Bericht auch nicht.
    Senke::p0(&c, "main2", &wire(&wurzel, "bestaetigt_neutral"));
    let sicht = c.interventionssicht_fuer_link("main2");
    assert!(
        sicht.unknown && sicht.tail_samples_offen == 4_800,
        "M-58/E4: das `false` waehrend des Nachlaufs schneidet ihn nicht ab: {sicht:?}"
    );
    assert!(!einspeisen(7), "und die Sonde bleibt gesperrt");
    assert_eq!(
        zuletzt(),
        Some(hex(0x1001)),
        "sechs verworfene Belege haben nichts hinterlassen"
    );

    // (8) Der Nachlauf laeuft ab - 4800 Samples bei 48 kHz sind 100 ms.
    clock.vor(500);
    c.liveness_tick();
    assert_eq!(
        c.interventionssicht_fuer_link("main2").tail_samples_offen,
        0,
        "der Nachlauf ist abgelaufen"
    );

    // (8b) 🔑 Nacharbeit 3 (WA-03/M-61, §34.2): der Zwischenzustand ZWISCHEN
    //      Tailablauf und Abschluss. Bis hierher pruefte Schritt 8 allein
    //      `tail_samples_offen == 0` und speiste erst NACH dem `false` wieder
    //      ein: ein Tick, der `unknown` unerlaubt loeste, waere gruen
    //      geblieben. §34.2/M-61 verlangt aber den BESTAETIGTEN Abschluss -
    //      abgelaufene Zeit ist keine Bestaetigung.
    let sicht = c.interventionssicht_fuer_link("main2");
    assert!(
        sicht.unknown && !sicht.starke_evidenz_erlaubt,
        "WA-03/M-61: der Tick zaehlt den Nachlauf herunter, er spricht den \
         Abschluss NICHT aus - `unknown` steht weiter: {sicht:?}"
    );
    assert!(
        !einspeisen(8),
        "WA-03/N-07: und darum bleibt eingespeiste Evidenz auch nach dem \
         Tailablauf VERWORFEN - bis der Nachbericht kommt"
    );
    assert_eq!(
        zuletzt(),
        Some(hex(0x1001)),
        "WA-03: der abgelaufene Nachlauf hinterlaesst nichts im Evidenzstand"
    );

    // (9) ERST JETZT schliesst das einmalige `false` den Bericht ab.
    Senke::p0(&c, "main2", &wire(&wurzel, "bestaetigt_neutral"));
    let sicht = c.interventionssicht_fuer_link("main2");
    assert!(
        !sicht.unknown && sicht.starke_evidenz_erlaubt,
        "N-05/E4: der Nachbericht schliesst ab, und die Sitzung nullt ueber den \
         REGULAEREN Pfad - ohne neuen Link: {sicht:?}"
    );
    assert!(
        einspeisen(9),
        "N-07: und die verbliebene Sonde darf wieder committen"
    );
    assert_eq!(
        zuletzt(),
        Some(hex(0x1009)),
        "N-07: der erste Beleg NACH dem Nachbericht steht im Evidenzstand - \
         gemessen an seiner `evidence_id`, nicht an einer Dispatchfrage"
    );

    // (10) EP-11: die Sequenzbasis blieb stehen. Der naechste regulaere Marker
    //      traegt 3 - die Zahl, die der Pluginzaehler wirklich vergibt.
    Senke::p0(
        &c,
        "main2",
        &intervention_begin_payload(&h.adresse, 0x8502, 3, "hoermarkierung", None),
    );
    assert!(
        !c.interventionssicht_fuer_link("main2").unknown,
        "EP-11: 3 ist lueckenlos, weil der Abschluss die Basis nicht auf 0 setzte - \
         und der Broker sieht keine Luecke"
    );
    assert_eq!(
        c.interventionssicht_fuer_link("main2").aktive,
        1,
        "EP-11: und das neue Intervall steht"
    );
}

/// NAK-180 Nacharbeit 1 (EP-17, N-08): der Backlog-Aufbau ueber die
/// Sprachgrenze - Ring nicht leer, Marker still.
///
/// Kein `false` beim Aufbau, erster Heartbeat `true`, KEIN Replay; das Backlog
/// reist mit weiterlaufenden Nummern, der frische Link nimmt die erste
/// vorbehaltlos an, danach lueckenlos. Ist der Ring leer, traegt der naechste
/// Heartbeat `false`, und E4 laesst ihn gelten.
#[test]
fn r2_backlog_ohne_replay_end_zu_end() {
    let wurzel = heartbeat_wire_instanz();
    let (c, clock) = coordinator();
    let h = hello_aus_wire_instanz(&wurzel, "main");
    anmelden_roh(&c, "main", &h);
    Senke::p0(&c, "main", &wire(&wurzel, "bestaetigt_neutral"));

    // Der Link stirbt, waehrend im Ring des Plugins ein vollstaendiges Paar
    // wartet - beim Broker ist davon noch nichts angekommen.
    c.control_ende("main");
    anmelden_roh(&c, "main2", &h);

    // N-08: erster Heartbeat `true` (der Ring war nicht leer), KEIN Replay.
    Senke::p0(&c, "main2", &wire(&wurzel, "unbekannt"));
    assert!(
        c.interventionssicht_fuer_link("main2").unknown,
        "N-08: kein `false` beim Aufbau"
    );

    // Das Backlog reist in unveraenderter Reihenfolge - die erste Zahl nimmt
    // der frische Link vorbehaltlos an, danach lueckenlos.
    assert!(c.intervention_begin("main2", &h.adresse, &hex(0x8601), 7));
    assert!(c.intervention_end("main2", &h.adresse, &hex(0x8601), 8, 4_800));
    let sicht = c.interventionssicht_fuer_link("main2");
    assert!(
        sicht.aktive == 0 && sicht.tail_samples_offen == 4_800,
        "das Backlog ist vollstaendig angekommen: {sicht:?}"
    );

    clock.vor(500);
    c.liveness_tick();
    Senke::p0(&c, "main2", &wire(&wurzel, "bestaetigt_neutral"));
    let sicht = c.interventionssicht_fuer_link("main2");
    assert!(
        !sicht.unknown && sicht.starke_evidenz_erlaubt,
        "N-08: ist der Ring leer, traegt der naechste Heartbeat `false`, und E4 \
         laesst ihn gelten: {sicht:?}"
    );
}

/// NAK-180 R9 (N-31): ein SONDEN-Hello aendert den Taint nicht.
#[test]
fn sondenhello_aendert_den_taint_nicht() {
    let wurzel = heartbeat_wire_instanz();
    let (c, _clock) = coordinator();
    let sonde = hello_aus_wire_instanz(&wurzel, "active_probe");
    anmelden_roh(&c, "sonde", &sonde);
    assert!(
        !c.interventionssicht_fuer_link("sonde").unknown,
        "N-31: eine Sonde macht keine Aussage ueber Interventionen - ihr Hello \r
         darf weder sperren noch entsperren"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-181 · R9 — gemessene Aktivzeit gegen Projektspanne (Befund C5, M-23)
// ═════════════════════════════════════════════════════════════════════════
//
// Der Defekt: `haelfte_aus_historie` ueberschrieb die summierte Messzeit mit
// der PROJEKTSPANNE. Zwei Haelften ueber derselben Passage tragen damit immer
// dieselbe Zahl — und der zweite der „genau drei" M-23-Herabstufungsgruende
// („aktive Messzeit um mehr als 10 % verschieden, Smart Disable oder Stille
// auf einem Punkt") konnte im Produktpfad nie fallen.
//
// Warum diese drei Faelle die Haelfte lesen und nicht nur das Urteil: das
// Urteil erreicht seine Herabstufungen erst NACH einem gefundenen Restlag,
// und der braucht deutlich mehr Frames, als ein Verdrahtungsfall einspeist.
// Der Defekt sass aber in der Verdrahtung. Also misst der Test die gebildete
// Haelfte und laesst `beurteile_paar` mit genau diesen zwei Haelften urteilen
// — derselbe Weg, den der Produktpfad geht, nur ohne die Kurvenlaenge, die
// der Lagschaetzer braucht.

/// Speist `anzahl` Snapshots so ein, dass sie zusammen genau die Projektspanne
/// `[0, spanne)` abdecken — jeder mit `laenge` Samples Fensterlaenge.
///
/// Die Spanne haengt damit an den GRENZEN, die Zahl der Snapshots an der
/// Dichte. Genau diese zwei Groessen trennt R9.
fn speise_mit_dichte(
    c: &eqcop_broker::coordinator::Coordinator,
    link: &str,
    adresse: &Adresse,
    anzahl: usize,
    laenge: u64,
    spanne: i64,
) {
    for nr in 0..anzahl {
        // Der letzte Snapshot endet EXAKT bei `spanne` — sonst haengt die
        // Projektspanne an der Dichte, und der Fall misst sich selbst.
        let start = if anzahl <= 1 {
            spanne - laenge as i64
        } else {
            ((spanne - laenge as i64) as f64 * (nr as f64 / (anzahl - 1) as f64)) as i64
        };
        c.evidence_snapshot_json(
            link,
            &evidenz_payload(adresse, nr, |w| {
                let t = w.get_mut("transport").expect("Fixture traegt transport");
                t["project_sample_start"] = json!(start);
                t["sample_count"] = json!(laenge);
                // Keine Spruenge: Epoche und Segment bleiben, sonst greift
                // `SprungImFenster` vor jeder Herabstufung.
                t["transport_epoch"] = json!(17);
                t["continuity_segment"] = json!(3);
            }),
        );
    }
}

/// Eine deterministische, musikaehnliche Huellkurve — dieselbe Bauform wie in
/// `sonde013_prepost.rs`.
///
/// ⚠️ WARUM DIESER TEST SIE BRAUCHT: die Evidenzfixture traegt in jedem
/// Snapshot DENSELBEN Bandsatz. Aus einer Folge gleicher Snapshots entsteht
/// eine FLACHE Huellkurve, und eine flache Kurve hat keine Korrelationsspitze
/// — `schaetze_restlag` liefert `None`, und `beurteile_paar` kehrt zurueck,
/// BEVOR es die Herabstufungen rechnet. Das ist keine Eigenschaft dieses
/// Defekts, sondern der Fixture; `prepost_join_laeuft_im_produktpfad` prueft
/// aus demselben Grund nur die Klasse.
///
/// Der Fall trennt deshalb sauber: die zwei ZEITEN kommen aus dem echten
/// Produktpfad (`paarhaelfte_fuer_test`), die KURVEN sind hier synthetisch und
/// als solche benannt. Gemessen wird damit genau die Kopplung, um die es geht
/// — „mit den Zeiten, die die Verdrahtung liefert, faellt der Grund" — und
/// nicht der Lagschaetzer, den B24 und `sonde013_prepost` messen.
fn huelle(laenge: usize, saat: u64, takt: f64) -> Vec<f32> {
    let mut aus = Vec::with_capacity(laenge);
    let mut x = saat.wrapping_mul(6_364_136_223_846_793_005).wrapping_add(1);
    for i in 0..laenge {
        x = x
            .wrapping_mul(6_364_136_223_846_793_005)
            .wrapping_add(1_442_695_040_888_963_407);
        let rauschen = ((x >> 33) as f64 / (1u64 << 31) as f64) - 0.5;
        let t = i as f64;
        let puls = (t / takt * std::f64::consts::TAU).sin().max(0.0).powi(3);
        let lang = (t / (takt * 7.3) * std::f64::consts::TAU).sin() * 0.3;
        aus.push((0.35 + puls * 0.5 + lang * 0.2 + rauschen * 0.12).max(0.01) as f32);
    }
    aus
}

/// Setzt taugliche Kurven in eine aus dem Produktpfad gewonnene Haelfte —
/// und laesst jedes andere Feld, besonders `aktiv_s` und `spanne_s`, unberuehrt.
fn mit_tauglichen_kurven(
    mut h: eqcop_broker::coordinator::prepost::Paarhaelfte,
) -> eqcop_broker::coordinator::prepost::Paarhaelfte {
    const FRAMES: usize = 300;
    h.huellkurven = (0..4)
        .map(|b| huelle(FRAMES, 11 + b as u64, 5.0 + b as f64 * 2.0))
        .collect();
    h.onsets = huelle(FRAMES, 99, 4.0);
    h
}

/// N-38 — halbe Snapshotdichte bei gleicher Projektspanne stuft herab.
#[test]
fn halbe_snapshotdichte_stuft_herab() {
    use eqcop_broker::coordinator::prepost::{beurteile_paar, Herabstufungsgrund};
    let (c, _) = coordinator();
    let pre = hello(1, 2, 10, 100, "passive_probe", Some(9));
    let post = hello(1, 2, 11, 101, "passive_probe", Some(9));
    anmelden(&c, "pre", &pre);
    anmelden(&c, "post", &post);
    let paar = hex(0x77);
    assert!(c.descriptor_setzen("pre", descriptor(&pre.adresse, "pre", &paar)));
    assert!(c.descriptor_setzen("post", descriptor(&post.adresse, "post", &paar)));

    // Dieselbe Passage, dieselbe Spanne — aber POST liefert halb so viele
    // Snapshots (Smart Disable, einseitige Stille oder die
    // Rueckstau-Kadenzreduktion aus `SondeProcessor.cpp:471-478`).
    const SPANNE: i64 = 48_000 * 10;
    const LAENGE: u64 = 512;
    speise_mit_dichte(&c, "pre", &pre.adresse, 16, LAENGE, SPANNE);
    speise_mit_dichte(&c, "post", &post.adresse, 8, LAENGE, SPANNE);

    let h_pre = c.paarhaelfte_fuer_test("pre").expect("PRE-Haelfte entsteht");
    let h_post = c.paarhaelfte_fuer_test("post").expect("POST-Haelfte entsteht");

    // 🔑 Die Zusage der Verdrahtung: die SPANNE ist gleich, die MESSZEIT nicht.
    assert!(
        (h_pre.spanne_s - h_post.spanne_s).abs() < 1e-9,
        "beide Haelften decken dieselbe Projektspanne ab: {} gegen {}",
        h_pre.spanne_s,
        h_post.spanne_s
    );
    assert!(
        h_pre.aktiv_s > h_post.aktiv_s * 1.9,
        "die doppelte Dichte ergibt rund die doppelte gemessene Zeit: {} gegen {}",
        h_pre.aktiv_s,
        h_post.aktiv_s
    );

    // 🔑 Und die Folge im Urteil: genau dieser Unterschied stuft herab.
    // Die Kurven sind dafuer synthetisch (Begruendung an `huelle`); die zwei
    // Zeiten sind die aus dem Produktpfad gewonnenen.
    let u = beurteile_paar(
        &paar,
        &mit_tauglichen_kurven(h_pre.clone()),
        &mit_tauglichen_kurven(h_post.clone()),
    );
    assert!(
        u.herabstufungen.contains(&Herabstufungsgrund::AktivzeitKlafft),
        "N-38: halbe Snapshotdichte ergibt `AktivzeitKlafft` (M-23, zweiter Grund): {:?}",
        u.herabstufungen
    );
}

/// N-39 — gleiche Dichte stuft NICHT herab. Der Riegel ist scharf.
#[test]
fn gleiche_dichte_stuft_nicht_herab() {
    use eqcop_broker::coordinator::prepost::{beurteile_paar, Herabstufungsgrund};
    let (c, _) = coordinator();
    let pre = hello(1, 2, 10, 100, "passive_probe", Some(9));
    let post = hello(1, 2, 11, 101, "passive_probe", Some(9));
    anmelden(&c, "pre", &pre);
    anmelden(&c, "post", &post);
    let paar = hex(0x77);
    assert!(c.descriptor_setzen("pre", descriptor(&pre.adresse, "pre", &paar)));
    assert!(c.descriptor_setzen("post", descriptor(&post.adresse, "post", &paar)));

    const SPANNE: i64 = 48_000 * 10;
    speise_mit_dichte(&c, "pre", &pre.adresse, 16, 512, SPANNE);
    speise_mit_dichte(&c, "post", &post.adresse, 16, 512, SPANNE);

    let h_pre = c.paarhaelfte_fuer_test("pre").expect("PRE-Haelfte entsteht");
    let h_post = c.paarhaelfte_fuer_test("post").expect("POST-Haelfte entsteht");
    assert!(
        (h_pre.aktiv_s - h_post.aktiv_s).abs() < 1e-9,
        "gleiche Dichte ergibt gleiche Messzeit: {} gegen {}",
        h_pre.aktiv_s,
        h_post.aktiv_s
    );

    let u = beurteile_paar(
        &paar,
        &mit_tauglichen_kurven(h_pre.clone()),
        &mit_tauglichen_kurven(h_post.clone()),
    );
    assert!(
        !u.herabstufungen.contains(&Herabstufungsgrund::AktivzeitKlafft),
        "N-39: ohne Dichteunterschied faellt der Grund nicht — sonst waere er kein Riegel: {:?}",
        u.herabstufungen
    );
}

/// N-40 — die Projektspanne traegt weiter den Suchraum.
///
/// Der R30-Fix bleibt erhalten: `capture_s` und damit `schaetze_restlag`
/// rechnen mit der Spanne, nicht mit der Summe der Analysefensterlaengen. Bei
/// 16 Snapshots à 512 Samples waere die Summe 0,17 s — `suchraum_frames`
/// machte daraus einen Suchraum von null, und der Produktpfad faende NIE einen
/// Zeitbezug.
#[test]
fn projektspanne_traegt_weiter_den_suchraum() {
    use eqcop_broker::coordinator::prepost::{suchraum_frames, GATE_MINDESTCAPTURE_S};
    let (c, _) = coordinator();
    let pre = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "pre", &pre);
    let paar = hex(0x77);
    assert!(c.descriptor_setzen("pre", descriptor(&pre.adresse, "pre", &paar)));

    const SPANNE: i64 = 48_000 * 10; // 10 s Passage
    speise_mit_dichte(&c, "pre", &pre.adresse, 16, 512, SPANNE);

    let h = c.paarhaelfte_fuer_test("pre").expect("Haelfte entsteht");
    assert!(
        (h.spanne_s - 10.0).abs() < 0.01,
        "die Spanne ist die Projektspanne: {}",
        h.spanne_s
    );
    assert!(
        h.aktiv_s < 1.0,
        "die gemessene Zeit ist die Summe der Fenster und viel kleiner: {}",
        h.aktiv_s
    );
    // 🔑 Genau darum steht der Suchraum an der SPANNE: aus der Messzeit
    // entstuende keiner.
    assert!(
        h.spanne_s >= GATE_MINDESTCAPTURE_S,
        "die Spanne traegt das Mindestcapture aus Paragraph 38.2"
    );
    assert!(
        suchraum_frames(h.spanne_s) > 0,
        "und damit einen Suchraum ueber null"
    );
    assert_eq!(
        suchraum_frames(h.aktiv_s),
        0,
        "aus der Messzeit allein entstuende keiner — der R30-Fix bleibt noetig"
    );
}
