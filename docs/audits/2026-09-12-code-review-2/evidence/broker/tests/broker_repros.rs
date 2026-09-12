#![cfg(windows)]

use eqcop_broker::coordinator::{Befundzustand, Coordinator, ManualClock};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
use eqcop_broker::transport::server_v3::Senke;
use serde_json::{json, Value};
use std::sync::Arc;

// ── Werkzeug ─────────────────────────────────────────────────────────────

fn hex(n: usize) -> String {
    format!("{n:032x}")
}

fn adresse(instanz: usize) -> Adresse {
    Adresse {
        logon_sid: "S-1-5-21-1-2-3-1001".into(),
        project_binding_id: hex(0x11),
        session_epoch: hex(0x22),
        instance_id: hex(instanz),
        runtime_nonce: hex(0x40 + instanz),
    }
}

fn hello(adresse: Adresse, art: &str) -> HelloControl {
    HelloControl {
        typ: "hello".into(),
        connection_kind: "control".into(),
        protocol: 3,
        plugin_version: "test".into(),
        plugin_kind: art.into(),
        adresse,
        host: Some(HostAngabe {
            pid: 4711,
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

/// Die Faehigkeitsmenge des VERTRAGS.
///
/// Ein Deskriptor mit anderen Namen kommt durch `descriptor_setzen`, faellt
/// aber spaeter gegen `$defs/probe_descriptor_post` - und mit ihm die ganze
/// Sessionprojektion, sobald ein Abonnent sie liest (N-31 dieser Runde).
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
        "remote_control": "unsupported"
    })
}

fn fixture(name: &str) -> Value {
    let pfad = std::path::Path::new("C:/Users/phili/Projekte/Nakama/broker")
        .join("../eq-copilot/fixtures/v3/gueltig")
        .join(format!("{name}.json"));
    serde_json::from_slice(&std::fs::read(&pfad).expect("Fixture liegt im Korpus"))
        .expect("Fixture ist JSON")
}

const ANOMALIEBAND: usize = 98;
/// Die Transportepoche der committeten Evidenzgrundform. Passage und Belege
/// müssen dieselbe führen, sonst ist die Passage eine ANDERE (M-23).
const TRANSPORT_EPOCHE: u64 = 17;
const BASIS_SAMPLE: i64 = 44_108_200;

/// Ein Beleg mit waehlbarer Anhebung im Anomalieband (Zehntel-dB).
///
/// 🔑 **NAK-212 R1 (07.09.2026).** Mit KONSTANTER Anhebung liegt jeder Wert
/// auf oder ueber dem eigenen Median, die Vergleichsmenge „ohne die Quelle"
/// bleibt leer, und der bedingte Uplift ist nach M-19 nicht messbar — der
/// Befund erreicht dann zu Recht nur `mittel`, und jeder Fall, der einen
/// handelbaren Befund braucht, misst nichts mehr.
fn evidenz(a: &Adresse, nr: usize, projekt_start: i64, anhebung: i64) -> Vec<u8> {
    static ROH: std::sync::OnceLock<Value> = std::sync::OnceLock::new();
    let mut wert = ROH
        .get_or_init(|| fixture("evidence-snapshot-mit-ereignissen-und-stereo"))
        .clone();
    wert["adresse"] = serde_json::to_value(a).unwrap();
    wert["evidence_id"] = json!(hex(0x1000 + nr));
    wert["transport"]["sequence"] = json!(nr as u64 + 1);
    wert["transport"]["project_sample_start"] = json!(projekt_start);
    for pfad in [
        "/baender/werte",
        "/verteilung/p10/werte",
        "/verteilung/p50/werte",
        "/verteilung/p95/werte",
    ] {
        if let Some(Value::Array(werte)) = wert.pointer_mut(pfad) {
            for index in ANOMALIEBAND..(ANOMALIEBAND + 4).min(werte.len()) {
                let alt = werte[index].as_i64().unwrap_or(0);
                werte[index] = json!(alt + anhebung);
            }
        }
    }
    serde_json::to_vec(&wert).unwrap()
}

struct TestOrdner(std::path::PathBuf);

impl TestOrdner {
    fn neu(name: &str) -> Self {
        let pfad = std::path::PathBuf::from("C:/na-audit2-broker-20260912").join(format!(
            "nakama-sonde014-nacharbeit2-{name}-{}-{}",
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

/// Was der Broker wirklich hinausgeschrieben hat.
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

    fn leeren(&self) {
        self.geschrieben
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .clear();
    }
}

impl eqcop_broker::coordinator::SessionPush for PushProbe {
    fn snapshot_schreiben(
        &self,
        link_id: &str,
        _object_key: &str,
        _ordnung: i64,
        payload: &[u8],
    ) -> bool {
        let wert: Value = serde_json::from_slice(payload).unwrap_or(Value::Null);
        self.geschrieben
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .push((link_id.to_owned(), wert));
        true
    }
}

/// Ein Coordinator MIT echtem Store, dazu Master und Sonde angemeldet.
struct Harnisch {
    c: Arc<Coordinator>,
    master: Adresse,
    sonde: Adresse,
    push: Arc<PushProbe>,
    _writer: eqcop_broker::store::StoreWriter,
    ordner: TestOrdner,
}

impl Harnisch {
    fn neu(name: &str) -> Self {
        let ordner = TestOrdner::neu(name);
        let mut k = eqcop_broker::store::StoreKonfiguration::fuer_pfad(ordner.db());
        k.remote_volume_override = Some(false);
        let writer = eqcop_broker::store::StoreWriter::starten(k);
        assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());
        let c = Arc::new(Coordinator::mit_store(
            Arc::new(ManualClock::default()),
            hex(0xbeef),
            &writer,
        ));
        let push = Arc::new(PushProbe::default());
        c.session_push_setzen(push.clone());
        let harnisch = Self {
            c,
            master: adresse(1),
            sonde: adresse(2),
            push,
            _writer: writer,
            ordner,
        };
        harnisch.anmelden();
        harnisch
    }

    fn anmelden(&self) {
        for (link, a, art, mixer) in [
            ("main", &self.master, "main", 0i64),
            ("sonde0", &self.sonde, "passive_probe", 3),
        ] {
            let h = hello(a.clone(), art);
            let ausgang = self.c.control_hello_registrieren(link, &h);
            assert!(ausgang.angenommen, "{:?}", ausgang.grund);
            let _ = self.c.resync_bestaetigen(link, 0);
            // Ein MAIN bekommt KEINEN Sondendeskriptor: der Runtime-Block
            // traegt Messpunkt und Betrieb einer Sonde, und ein daraus
            // abgeleiteter  mit  ist
            // vertragswidrig - die Sessionprojektion faellt daran fail-closed,
            // und mit ihr jeder Push (WN-03 misst genau den).
            if art != "main" {
                let d = json!({
                    "adresse": a,
                    "plugin_kind": art,
                    "measurement_position": "post",
                    "aussageklasse": "beobachtend",
                    "betrieb": "active",
                    "label": "Testquelle",
                    "capabilities": capabilities(),
                    "frische": {"letzter_kontakt_ms": 10, "stale": false},
                    "host_mixer_index": mixer
                });
                assert!(self.c.descriptor_setzen(link, d));
            }
            // Ohne Heartbeat-Kontakt gibt der Alias-/Routingriegel den
            // Snapshot-Push nicht frei - der Fall WN-03 misst genau ihn.
            assert!(self.c.heartbeat_kontakt(
                link,
                Some(&json!({
                    "type": "heartbeat",
                    "adresse": a,
                    "sequence": 1,
                    "state_revision": 0,
                    "capabilities": capabilities(),
                    "zaehler": {}
                })),
            ));
            self.state_report(link, a, 0);
        }
    }

    /// Der `state_report` ist die EINZIGE Quelle, aus der der Broker die
    /// State-Revision eines Clients kennt (WN-01).
    fn state_report(&self, link: &str, a: &Adresse, revision: u64) {
        let bericht = json!({
            "type": "state_report",
            "adresse": a,
            "dsp_schema_version": 1,
            "state_revision": revision,
            "state_hash": "a".repeat(64),
            "record_state": {"valid": true, "recording": false}
        });
        assert!(self
            .c
            .state_report_json(link, &serde_json::to_vec(&bericht).unwrap()));
    }

    /// Der Main abonniert seine eigene Sitzung — erst dann entstehen Pushes.
    fn abonniert(&self) {
        assert!(self.c.subscribe_json(
            "main",
            &serde_json::to_vec(&json!({
                "type": "subscribe_session",
                "adresse": self.master,
                "session_epoch": self.master.session_epoch
            }))
            .unwrap()
        ));
    }

    /// M-86/NR-01: der Vollbestand mit Marke, bevor gerechnet wird.
    fn intent_marke(&self, revision: i64) {
        let wert = json!({
            "type": "intent_update",
            "adresse": self.master,
            "session_epoch": self.master.session_epoch,
            "vollstaendig": true,
            "bestand_revision": revision
        });
        assert!(self
            .c
            .intent_update_json("main", &serde_json::to_vec(&wert).unwrap()));
    }

    /// Ein TEILupdate genau eines Objekts: die Rolle EINER Quelle.
    fn teilupdate(&self, quelle: &Adresse, rolle: &str, revision: i64) -> bool {
        let wert = json!({
            "type": "intent_update",
            "adresse": self.master,
            "session_epoch": self.master.session_epoch,
            "vollstaendig": false,
            "bestand_revision": revision,
            "intents": [{
                "quelle_id": quelle.instance_id,
                "rolle": rolle,
                "revision": revision,
                "herkunft": "user",
                "konfidenz": 1.0
            }]
        });
        self.c
            .intent_update_json("main", &serde_json::to_vec(&wert).unwrap())
    }

    /// Belege mit wechselndem Pegel — die Form, in der ein bedingter Uplift
    /// (M-19) ueberhaupt messbar ist.
    ///
    /// ⚠️ BEIDE Stufen heben das Anomalieband an (9,0 und 4,0 dB), nicht
    /// „laut gegen null". So findet `masteranomalie` dasselbe Band in JEDEM
    /// Fenster — unabhaengig davon, ob das juengste gerade laut oder leise
    /// ist —, und Master und Kandidat laufen gleich, auch wenn sie
    /// verschieden viele Fenster tragen.
    fn belege(&self, link: &str, a: &Adresse, ab_nr: usize, anzahl: usize) {
        for i in 0..anzahl {
            let anhebung = if i % 2 == 0 { 90 } else { 40 };
            self.c.p1(
                link,
                &evidenz(a, ab_nr + i, BASIS_SAMPLE + (i as i64) * 512, anhebung),
            );
        }
    }

    fn befunde(&self) -> Vec<eqcop_broker::coordinator::CauseHypothesis> {
        self.c
            .befunde_sicht(&self.master.project_binding_id, &self.master.session_epoch)
    }

    fn vorschlaege(&self) -> Vec<eqcop_broker::coordinator::Proposal> {
        self.c
            .vorschlaege_sicht(&self.master.project_binding_id, &self.master.session_epoch)
    }

    /// Eine benannte Passage über den PRODUKTPFAD: `experiment_begin` als P0.
    fn passage_anlegen(&self, command: usize, experiment: usize, fenster: i64) -> String {
        let mut wert = fixture("experiment_begin");
        wert["kopf"]["ziel"] = serde_json::to_value(&self.master).unwrap();
        wert["kopf"]["command_id"] = json!(hex(command));
        wert["kopf"]["base_revision"] = json!(0);
        wert["experiment_id"] = json!(hex(experiment));
        let passage_id = hex(0x5000 + experiment);
        wert["passage"]["passage_id"] = json!(passage_id);
        wert["passage"]["projekt_von"] = json!(BASIS_SAMPLE);
        wert["passage"]["projekt_bis"] = json!(BASIS_SAMPLE + fenster * 512);
        wert["passage"]["transport_epoch"] = json!(TRANSPORT_EPOCHE);
        wert["passage"]["aktive_quellen"] =
            json!([self.master.instance_id, self.sonde.instance_id]);
        let antwort = Senke::p0(&*self.c, "main", &serde_json::to_vec(&wert).unwrap())
            .expect("experiment_begin wird beantwortet");
        let ack: Value = serde_json::from_slice(&antwort).unwrap();
        assert_eq!(
            ack["ergebnis"], "angewandt",
            "die Passage entsteht wirklich: {ack:?}"
        );
        passage_id
    }

    fn sql(&self, sql: &str) -> i64 {
        let db = rusqlite::Connection::open(self.ordner.db()).expect("Store liegt da");
        db.query_row(sql, [], |z| z.get(0)).expect("Zaehlung")
    }

    fn sql_ausfuehren(&self, sql: &str) {
        let db = rusqlite::Connection::open(self.ordner.db()).expect("Store liegt da");
        db.execute(sql, []).expect("Anweisung");
    }
}

#[test]
fn v2_juengeres_hypothesenergebnis_darf_nicht_zurueckrollen() {
    let h = Harnisch::neu("audit-v2");
    h.intent_marke(0);
    h.passage_anlegen(0xa201, 0xa202, 16);
    h.belege("main", &h.master, 0, 12);
    h.belege("sonde0", &h.sonde, 100, 11);
    h.abonniert();
    h.push.leeren();

    let neuer = Arc::new(std::sync::Mutex::new(Vec::new()));
    let neuer_im_haken = Arc::clone(&neuer);
    let schwach = Arc::downgrade(&h.c);
    let master = h.master.clone();
    let sonde = h.sonde.clone();
    h.c.rechen_test_haken_setzen(Box::new(move || {
        let c = schwach.upgrade().unwrap();
        assert!(c.evidence_snapshot_json(
            "main", &evidenz(&master, 212, BASIS_SAMPLE + 12 * 512, 90)));
        assert!(c.evidence_snapshot_json(
            "sonde0", &evidenz(&sonde, 312, BASIS_SAMPLE + 12 * 512, 90)));
        let publiziert = c.befunde_sicht(&master.project_binding_id, &master.session_epoch);
        assert!(!publiziert.is_empty(), "B hat wirklich ein Ergebnis publiziert");
        assert!(publiziert.iter().any(|b| b.evidence_ids.contains(&hex(0x1000 + 312))),
            "B traegt den neuen Quellbeleg");
        *neuer_im_haken.lock().unwrap() = publiziert;
    }));

    assert!(h.c.evidence_snapshot_json(
        "sonde0", &evidenz(&h.sonde, 111, BASIS_SAMPLE + 11 * 512, 40)));
    let b = neuer.lock().unwrap().clone();
    let ende = h.befunde();
    println!("V2 newer evidence counts: {:?}", b.iter().map(|x| x.evidence_ids.len()).collect::<Vec<_>>());
    println!("V2 final evidence counts: {:?}", ende.iter().map(|x| x.evidence_ids.len()).collect::<Vec<_>>());
    println!("V2 latest source evidence still retained: {}", h.c.evidenz_historie(&h.sonde.instance_id).iter().any(|e| e.evidence_id == hex(0x1000 + 312)));
    let snapshots = h.push.payloads();
    let wire = snapshots.iter().rev().find(|(_, v)| v["type"] == "session_snapshot").unwrap();
    println!("V2 final session-snapshot findings: {}", wire.1["findings"]);
    let db = rusqlite::Connection::open(h.ordner.db()).unwrap();
    let persistent: Vec<u8> = db.query_row("SELECT state_jcs FROM findings ORDER BY last_event_ord DESC LIMIT 1", [], |r| r.get(0)).unwrap();
    println!("V2 final SQLite finding: {}", String::from_utf8_lossy(&persistent));
    assert_eq!(ende, b, "Ein aelteres A darf das bereits publizierte B nicht ersetzen");
}

fn warten_auf(mut bedingung: impl FnMut() -> bool, beschreibung: &str) {
    let frist = std::time::Instant::now() + std::time::Duration::from_millis(1500);
    while !bedingung() {
        assert!(std::time::Instant::now() < frist, "nicht beobachtet: {beschreibung}");
        std::thread::sleep(std::time::Duration::from_millis(1));
    }
}

#[test]
fn ruecknahme_nach_storefehler_darf_keinen_fremden_ausschluss_loeschen() {
    let h = Harnisch::neu("audit-rollback-index");
    h.belege("sonde0", &h.sonde, 100, 32);
    let e0 = hex(0x1000 + 100);
    let e1 = hex(0x1000 + 101);
    assert_eq!(h.c.invalidierung_wegen_intervention_fuer_link(
        "sonde0", BASIS_SAMPLE + 512, BASIS_SAMPLE + 1024), 1);
    assert!(h.c.evidenz_historie(&h.sonde.instance_id).iter()
        .find(|e| e.evidence_id == e1).unwrap().ausschlussgrund.is_some());

    let db = rusqlite::Connection::open(h.ordner.db()).unwrap();
    db.execute_batch("BEGIN IMMEDIATE").unwrap();
    let c_invalidierung = Arc::clone(&h.c);
    let invalidierung = std::thread::spawn(move || {
        c_invalidierung.invalidierung_wegen_intervention_fuer_link(
            "sonde0", BASIS_SAMPLE, BASIS_SAMPLE + 512)
    });
    warten_auf(|| h.c.evidenz_historie(&h.sonde.instance_id).iter()
        .find(|e| e.evidence_id == e0).is_some_and(|e| e.ausschlussgrund.is_some()),
        "lokaler E0-Ausschluss vor Store-Fehler");

    let c_evidenz = Arc::clone(&h.c);
    let sonde = h.sonde.clone();
    let eingang = std::thread::spawn(move || {
        c_evidenz.evidence_snapshot_json(
            "sonde0", &evidenz(&sonde, 132, BASIS_SAMPLE + 32 * 512, 90))
    });
    warten_auf(|| {
        let historie = h.c.evidenz_historie(&h.sonde.instance_id);
        !historie.iter().any(|e| e.evidence_id == e0)
            && historie.iter().any(|e| e.evidence_id == hex(0x1000 + 132))
    }, "Retention verschiebt die Deque waehrend des Store-Wartens");
    assert_eq!(invalidierung.join().unwrap(), 0, "SQLite-Fehler erreicht Rueckweg");
    assert!(!eingang.join().unwrap(), "neuer Beleg wurde wegen Storefehler verworfen");
    db.execute_batch("ROLLBACK").unwrap();

    let historie = h.c.evidenz_historie(&h.sonde.instance_id);
    let cache_e1 = historie.iter().find(|e| e.evidence_id == e1).unwrap();
    let gespeichert: Vec<u8> = db.query_row(
        "SELECT state_jcs FROM evidence WHERE evidence_id=?1", [&e1], |r| r.get(0)).unwrap();
    let persistent: Value = serde_json::from_slice(&gespeichert).unwrap();
    println!("Rollback E1 cache exclusion: {:?}", cache_e1.ausschlussgrund);
    println!("Rollback E1 SQLite exclusion: {}", persistent["ausschlussgrund"]);
    assert!(persistent["ausschlussgrund"].is_string(), "E1 bleibt dauerhaft ausgeschlossen");
    assert!(cache_e1.ausschlussgrund.is_some(),
        "Der fehlgeschlagene E0-Rueckweg darf den frueher committeten E1-Ausschluss nicht loeschen");
}
