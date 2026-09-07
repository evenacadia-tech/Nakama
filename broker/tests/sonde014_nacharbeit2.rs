//! Die vier Broker-Zusagen der **Nacharbeit 2** (SONDE-014, 07.09.2026).
//!
//! Die Wiederprüfung 1 (Astra max, `f30ac47`) fand fünf Befunde. Vier davon
//! liegen im Broker beziehungsweise an seiner Kante zum Plugin, und jeder hat
//! hier seinen Fall:
//!
//! * **WP1-1 / WN-01** — der Kopf eines `user_verdict` trägt die zuletzt
//!   GEMELDETE State-Revision. Die Empfängerhälfte steht hier: mit der
//!   gemeldeten Revision wird `angewandt`, mit `n+1` `konflikt`. Die
//!   Senderhälfte misst B29.
//! * **WP1-2 / WN-02 (E-14)** — ein verspätetes Teilupdate mit hartem Veto
//!   entwertet abhängige Befunde, obwohl die Sitzungsrevision nicht steigt.
//! * **WP1-3 / WN-03** — der Re-Subscribe hält seine `findings` gegen den
//!   Store, auch wenn er die gespeicherte Projektion sendet.
//! * **WP1-4 / WN-04** — ohne benannte Passage entsteht kein Proposal, weder
//!   im Stand noch im Store.
//!
//! Der Harnisch ist der aus `sonde014_nacharbeit1.rs`: ein Coordinator mit
//! ECHTEM SQLite-Store, Master und Sonde angemeldet, Belege über `p1`. Ohne
//! Store misst keiner dieser Fälle etwas — drei von vier fragen die Tabelle
//! `evidence` beziehungsweise `event_log` direkt.

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
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
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

fn evidenz(a: &Adresse, nr: usize, projekt_start: i64) -> Vec<u8> {
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
                werte[index] = json!(alt + 90);
            }
        }
    }
    serde_json::to_vec(&wert).unwrap()
}

struct TestOrdner(std::path::PathBuf);

impl TestOrdner {
    fn neu(name: &str) -> Self {
        let pfad = std::env::temp_dir().join(format!(
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
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
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

    fn belege(&self, link: &str, a: &Adresse, ab_nr: usize, anzahl: usize) {
        for i in 0..anzahl {
            self.c
                .p1(link, &evidenz(a, ab_nr + i, BASIS_SAMPLE + (i as i64) * 512));
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

// =========================================================================
// WP1-1 . WN-01 - der Kopf traegt die GEMELDETE State-Revision
// =========================================================================
//
// `assistentAenderungMelden()` erhoeht `v3StateRevision` und der Kopf des
// unmittelbar danach gesendeten `user_verdict` uebernahm die NEUE Zahl. Der
// Broker kennt bis zum naechsten `state_report` aber noch die alte: das
// Urteil (P0) ueberholt den Bericht (P1) strukturell, `befehl.rs` antwortet
// `revision_conflict`, und `inFlightAck()` entfernte das unpersistierte
// Urteil endgueltig.
//
// Dieser Fall misst die EMPFAENGERHAELFTE an genau der Zeile, die die Zusage
// traegt: `base_revision == gemeldete Revision` wird angewandt, `n+1` nicht.
#[test]
fn user_verdict_haelt_sich_an_die_gemeldete_revision() {
    let h = Harnisch::neu("wn01");

    // Der Broker kennt die Revision 0 - so hat sie der `state_report` der
    // Anmeldung gemeldet. Das Urteil kommt aus der committeten Fixture; nur
    // der Kopf wechselt, denn genau er traegt die Zusage.
    let urteil = |command: usize, base: u64| {
        let mut wert = fixture("user_verdict");
        wert["kopf"]["ziel"] = serde_json::to_value(&h.master).unwrap();
        wert["kopf"]["command_id"] = json!(hex(command));
        wert["kopf"]["base_revision"] = json!(base);
        wert["user_verdict_id"] = json!(hex(0x7000 + command));
        wert
    };

    // (a) MIT der gemeldeten Revision: angewandt.
    let antwort = Senke::p0(&*h.c, "main", &serde_json::to_vec(&urteil(0x71, 0)).unwrap())
        .expect("das Urteil wird beantwortet");
    let ack: Value = serde_json::from_slice(&antwort).unwrap();
    assert_eq!(
        ack["ergebnis"], "angewandt",
        "die gemeldete Revision traegt: {ack:?}"
    );
    assert_eq!(
        h.sql("SELECT COUNT(*) FROM user_verdicts"),
        1,
        "und das Urteil steht im Store"
    );

    // (b) Mit dem LOKAL soeben erhoehten Zaehler `n+1`: konflikt. Genau das
    //     tat das Plugin, und genau hier ging das Urteil verloren.
    let antwort = Senke::p0(&*h.c, "main", &serde_json::to_vec(&urteil(0x72, 1)).unwrap())
        .expect("das Urteil wird beantwortet");
    let ack: Value = serde_json::from_slice(&antwort).unwrap();
    assert_eq!(ack["ergebnis"], "konflikt", "n+1 faellt: {ack:?}");
    assert_eq!(ack["code"], "revision_conflict", "und der Grund steht dabei: {ack:?}");
    assert_eq!(
        h.sql("SELECT COUNT(*) FROM user_verdicts"),
        1,
        "nichts wurde persistiert"
    );

    // (c) Und der Weg zurueck: meldet der Main die Revision 1, traegt genau
    //     sie das wiederholte Urteil - unter DERSELBEN `command_id`.
    h.state_report("main", &h.master, 1);
    let antwort = Senke::p0(&*h.c, "main", &serde_json::to_vec(&urteil(0x72, 1)).unwrap())
        .expect("das Urteil wird beantwortet");
    let ack: Value = serde_json::from_slice(&antwort).unwrap();
    assert_eq!(
        ack["ergebnis"], "angewandt",
        "die Wiederholung mit frischem Kopf traegt: {ack:?}"
    );
    assert_eq!(
        h.sql("SELECT COUNT(*) FROM user_verdicts"),
        2,
        "zwei Urteile, keines verloren"
    );
}

// =========================================================================
// WP1-2 . WN-02 (E-14) - ein verspaetetes Teilupdate entwertet
// =========================================================================
//
// Vollbestand/0 -> Teilupdate A/3 -> Befundbildung -> verspaetetes B/2 mit
// `rolle=geschuetzt`. Das Schutzveto wird uebernommen, die Sitzungsrevision
// bleibt 3 - und bis zu dieser Runde blieb der READY-Befund handelbar, weil
// §37.3 `stale` woertlich an eine GESTIEGENE Revision band.
//
// E-14 entscheidet: maszgeblich ist jede wirksam uebernommene
// Bestandsaenderung. Die Bestandsgeneration steigt, der Befund geht sichtbar
// in STALE, und ein Angebot entsteht nicht.
#[test]
fn verspaeteter_teilbericht_entwertet_abhaengige_befunde() {
    let h = Harnisch::neu("wn02");
    h.intent_marke(0);
    h.passage_anlegen(0x941, 0xac2, 12);
    // A/3 kommt VOR B/2 an - genau die Folge, die die Koaleszierung des
    // Senders erzeugt (NR-02).
    assert!(h.teilupdate(&h.master, "fuehrt", 3), "A/3 wird uebernommen");
    h.belege("main", &h.master, 0, 12);
    h.belege("sonde0", &h.sonde, 100, 12);

    let vorher = h.befunde();
    assert!(
        vorher
            .iter()
            .any(|b| b.zustand == Befundzustand::ReadyToSend),
        "die Buehne traegt einen handelbaren Befund - sonst misst der Fall nichts: {:?}",
        vorher.iter().map(|b| b.zustand).collect::<Vec<_>>()
    );
    let revision_vorher = h
        .c
        .intent_sicht(&h.master.project_binding_id, &h.master.session_epoch)
        .revision;
    assert_eq!(revision_vorher, 3);

    // Das VERSPAETETE Teilupdate: eine andere Quelle, kleinere Revision,
    // hartes Veto.
    assert!(
        h.teilupdate(&h.sonde, "geschuetzt", 2),
        "B/2 ist ein anderes Koaleszierungsobjekt und wird uebernommen (NR-02)"
    );
    assert_eq!(
        h.c.intent_sicht(&h.master.project_binding_id, &h.master.session_epoch)
            .revision,
        3,
        "die Sitzungsrevision steigt NICHT - genau darum geht es"
    );

    let nachher = h.befunde();
    assert!(
        !nachher.is_empty(),
        "die Befunde bleiben stehen, sie werden nur entwertet"
    );
    assert!(
        nachher.iter().all(|b| b.zustand == Befundzustand::Stale),
        "jeder abhaengige Befund ist STALE: {:?}",
        nachher
            .iter()
            .map(|b| (b.intent_revision, b.zustand))
            .collect::<Vec<_>>()
    );
    assert_eq!(
        h.c.draft_offers_zaehler(),
        0,
        "und kein Draft geht hinaus (M-30)"
    );

    // ── Zweiter Abschnitt: B/2 trifft WAEHREND der Rechnung ein ─────────
    //
    // Der Testhaken aus NR-03 faellt genau einmal zwischen
    // `aufnahmen_sammeln` und `befunde_eintragen`. Die numerische
    // Intent-Revision des Ergebnisses stimmt danach weiterhin - die
    // Generation nicht mehr, und genau daran faellt die Eintragung.
    let g = Harnisch::neu("wn02b");
    g.intent_marke(0);
    g.passage_anlegen(0x942, 0xac3, 12);
    assert!(g.teilupdate(&g.master, "fuehrt", 3));
    g.belege("main", &g.master, 0, 11);
    g.belege("sonde0", &g.sonde, 100, 11);

    let sonde = g.sonde.clone();
    let master = g.master.clone();
    // Derselbe Weg wie in `sonde014_verdrahtung.rs`: eine schwache
    // Referenz, damit der Haken den Coordinator nicht am Leben haelt.
    let schwach = Arc::downgrade(&g.c);
    g.c.rechen_test_haken_setzen(Box::new(move || {
        let Some(c) = schwach.upgrade() else {
            return;
        };
        let wert = json!({
            "type": "intent_update",
            "adresse": master,
            "session_epoch": master.session_epoch,
            "vollstaendig": false,
            "bestand_revision": 2,
            "intents": [{
                "quelle_id": sonde.instance_id,
                "rolle": "geschuetzt",
                "revision": 2,
                "herkunft": "user",
                "konfidenz": 1.0
            }]
        });
        assert!(c.intent_update_json("main", &serde_json::to_vec(&wert).unwrap()));
    }));
    // Der letzte Beleg loest die Rechnung aus.
    g.belege("sonde0", &g.sonde, 111, 1);

    assert!(
        g.c.intent_sicht(&g.master.project_binding_id, &g.master.session_epoch)
            .schutzangaben
            .iter()
            .any(|s| s.quelle_id == g.sonde.instance_id)
            || g.c
                .intent_sicht(&g.master.project_binding_id, &g.master.session_epoch)
                .intents
                .values()
                .any(|i| i.rolle == "geschuetzt"),
        "der Haken ist wirklich gefallen - sonst misst der Fall nichts"
    );
    assert!(
        g.befunde()
            .iter()
            .all(|b| b.zustand != Befundzustand::ReadyToSend),
        "kein Ergebnis der alten Generation wird eingetragen: {:?}",
        g.befunde()
            .iter()
            .map(|b| (b.intent_revision, b.zustand))
            .collect::<Vec<_>>()
    );
}

// =========================================================================
// WP1-3 . WN-03 - der Re-Subscribe haelt die Belege gegen den Store
// =========================================================================
//
// Nach Befundbildung und `DELETE FROM evidence` genuegte ein weiteres
// `subscribe_session` ohne zwischenzeitlichen Flush: `resubscribe_snapshot_push`
// las die gespeicherte Sessionprojektion und versandte deren `findings`
// unveraendert. Weder die Haertung des Caches (NR-04) noch eine zuvor
// gehaertete Speicheransicht korrigierte diese Bytes.
#[test]
fn re_subscribe_haelt_die_belege_gegen_den_store() {
    let h = Harnisch::neu("wn03");
    h.intent_marke(0);
    h.passage_anlegen(0x943, 0xac4, 12);
    // Das Abo kommt NACH dem persistenzpflichtigen Befehl - ein Subscribe
    // waehrend des  liefe in  - und VOR den
    // Belegen, damit der Flush der Befundbildung wirklich pusht.
    h.abonniert();
    h.belege("main", &h.master, 0, 12);
    h.belege("sonde0", &h.sonde, 100, 12);

    let befunde = h.befunde();
    assert!(!befunde.is_empty(), "die Buehne traegt einen Befund");
    let ids: Vec<String> = befunde
        .iter()
        .flat_map(|b| b.evidence_ids.iter().cloned())
        .collect();
    assert!(!ids.is_empty(), "und er nennt seine Belege");

    // Der Flush hat die Projektion committet - genau die Bytes, die der
    // Re-Subscribe spaeter sendet.
    assert!(
        h.push
            .payloads()
            .iter()
            .any(|(_, w)| w.get("findings").is_some()),
        "der Push traegt Befunde, bevor die Zeilen fehlen: {:?}",
        h.push
            .payloads()
            .iter()
            .map(|(l, w)| (l.clone(), w.get("findings").is_some(), w.as_object().map(|o| o.keys().cloned().collect::<Vec<_>>())))
            .collect::<Vec<_>>()
    );
    assert!(
        h.sql("SELECT COUNT(*) FROM sessions") > 0,
        "und die Sessionprojektion ist committet"
    );

    // Die Belegzeilen verschwinden - ohne dass irgendetwas den Cache anfasst.
    h.sql_ausfuehren("DELETE FROM evidence");
    h.push.leeren();

    // Ein weiteres `subscribe_session`, KEIN Flush dazwischen.
    h.abonniert();

    let gesendet = h.push.payloads();
    assert!(
        !gesendet.is_empty(),
        "der Re-Subscribe pusht wirklich einen Snapshot"
    );
    for (_, wert) in &gesendet {
        let findings = wert
            .get("findings")
            .and_then(Value::as_array)
            .cloned()
            .unwrap_or_default();
        assert!(
            findings.is_empty(),
            "kein Befund ohne existente Evidenz verlaesst das Haus: {findings:?}"
        );
    }
}

// =========================================================================
// WP1-4 . WN-04 - ohne Passage entsteht kein Proposal
// =========================================================================
//
// Der Erzeuger baute bis zu dieser Runde auch ohne `passage_id` ein Objekt;
// es landete in `stand.vorschlaege` und ueber `vorschlag_persistieren` als
// `event_type = "proposal"` im Store - obwohl `passage_id` seit NR-07
// Pflichtfeld des Vertrags ist. Der ausgefuehrte Test schrieb die Abweichung
// sogar fest.
#[test]
fn ohne_passage_entsteht_kein_proposal() {
    let h = Harnisch::neu("wn04");
    h.intent_marke(0);
    // KEINE Passage: kein `experiment_begin`.
    h.belege("main", &h.master, 0, 12);
    h.belege("sonde0", &h.sonde, 100, 12);

    assert!(
        !h.befunde().is_empty(),
        "der BEFUND entsteht auch ohne Passage - er behauptet etwas ueber die \
         Ursache, und nur der Vorschlag braucht den Ort"
    );
    assert!(
        h.befunde().iter().all(|b| b.passage_id.is_none()),
        "und er traegt wirklich keine Passage"
    );
    assert!(
        h.vorschlaege().is_empty(),
        "ohne Passage entsteht KEIN Vorschlag: {:?}",
        h.vorschlaege()
            .iter()
            .map(|v| (v.action, v.passage_id.clone()))
            .collect::<Vec<_>>()
    );
    assert_eq!(
        h.sql("SELECT COUNT(*) FROM event_log WHERE event_type = 'proposal'"),
        0,
        "und nichts davon steht im Store"
    );
    assert_eq!(h.c.draft_offers_zaehler(), 0);

    // ── Die GEGENPROBE: mit benannter Passage entsteht ein vollstaendiges
    //    Sechs-Felder-Objekt. Ohne sie waere „kein Objekt" nicht von „diese
    //    Buehne traegt ohnehin nichts" zu unterscheiden.
    let g = Harnisch::neu("wn04b");
    g.intent_marke(0);
    let passage = g.passage_anlegen(0x944, 0xac5, 12);
    g.belege("main", &g.master, 0, 12);
    g.belege("sonde0", &g.sonde, 100, 12);

    let vorschlaege = g.vorschlaege();
    assert!(!vorschlaege.is_empty(), "mit Passage entsteht ein Vorschlag");
    for vorschlag in &vorschlaege {
        for (feld, belegt) in vorschlag.gate_felder_vollstaendig() {
            assert!(belegt, "Gate-Feld {feld} fehlt");
        }
        assert_eq!(vorschlag.passage_id.as_deref(), Some(passage.as_str()));
    }
    assert!(
        g.sql("SELECT COUNT(*) FROM event_log WHERE event_type = 'proposal'") > 0,
        "und der Store traegt ihn"
    );
}
