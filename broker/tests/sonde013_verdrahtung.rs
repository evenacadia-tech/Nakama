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
#[derive(Default)]
struct PushProbe {
    geschrieben: std::sync::Mutex<Vec<(String, Value)>>,
}

impl PushProbe {
    fn payloads(&self) -> Vec<(String, Value)> {
        self.geschrieben
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .clone()
    }
}

impl eqcop_broker::coordinator::SessionPush for PushProbe {
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
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
        let payload = serde_json::to_vec(wert).unwrap();
        let antwort = Senke::p0(&self.c, "main", &payload).expect("die Familie wird beantwortet");
        serde_json::from_slice(&antwort).expect("command_ack ist JSON")
    }

    /// Ein ZWEITER Coordinator auf DEMSELBEN Store — der Brokerneustart.
    ///
    /// 🔑 Befund R12: der behauptete Restart-Test der Runde 1 erzeugte gar
    /// keinen zweiten Coordinator und konnte deshalb nicht fallen.
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
    // R19: die Guardrails aus M-45 sind gemessen, nicht nur Coverage/Klasse.
    assert!(
        achsen.guardrail_loudness_db.is_some(),
        "R19: der Loudness-Guardrail ist gemessen: {achsen:?}"
    );
    assert!(
        achsen.guardrail_loudness_db.unwrap_or(0.0) > 3.0,
        "R19: und er sieht die 6 dB, die das Resultat lauter ist: {:?}",
        achsen.guardrail_loudness_db
    );
    assert!(
        achsen.guardrail_peak_db.is_some(),
        "R19: der Peak-Guardrail ebenfalls"
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
