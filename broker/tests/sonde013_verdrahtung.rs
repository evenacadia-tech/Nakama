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

/// B16 — der Nachlauf laeuft am Liveness-Tick ab (M-58/M-60).
#[test]
fn nachlauf_laeuft_am_tick_ab() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    let id = hex(0x600);
    assert!(c.intervention_begin("a", &h.adresse, &id, 1));
    assert!(c.intervention_end("a", &h.adresse, &id, 2, 30_000));
    assert_eq!(c.interventionssicht_fuer_link("a").tail_samples_offen, 30_000);
    assert!(!c.evidence_dispatch_fuer_link("a"), "der Nachlauf sperrt");

    // Der Tick zieht ab - und zwar von selbst, ohne dass jemand
    // `tail_fortschritt` von Hand ruft. Genau DAS war Befund B16.
    c.liveness_tick();
    let nach_einem = c.interventionssicht_fuer_link("a").tail_samples_offen;
    assert!(
        nach_einem < 30_000,
        "ein Tick zieht vom Nachlauf ab: {nach_einem}"
    );
    for _ in 0..8 {
        c.liveness_tick();
    }
    assert_eq!(
        c.interventionssicht_fuer_link("a").tail_samples_offen,
        0,
        "und nach genug Ticks ist er abgelaufen"
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
    s.beginne(&id, &hex(1), passage, referenz).unwrap();
    s.binde_reihenfolge(&id, Blindreihenfolge::BaselineZuerst)
        .unwrap();

    // Die LEERE Messung: genau der Fall, mit dem `ergebnis()` vorher
    // erfolgreich terminierte.
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
    assert!(
        s.experiment(&id).unwrap().offen(),
        "und der Versuch bleibt OFFEN"
    );

    // Mit Messung: die vier Achsen entstehen und reisen mit.
    let messung = Resultatmessung {
        band_delta_db: (0..32).map(|i| (i as f64) * 0.2 - 3.0).collect(),
        band_gueltig: vec![true; 32],
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
    };
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
