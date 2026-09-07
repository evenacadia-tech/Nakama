//! Die Brokerzusage der **Nacharbeit 3** (SONDE-014, 07.09.2026).
//!
//! Die Wiederprüfung 2 (Astra max, `d09a6d7`) fand zwei Befunde; der zweite
//! liegt im Broker:
//!
//! > **WP2-2** — „Die neue Rundung garantiert keinen vertragsgültigen
//! > Wiretext für kleine Rangkomponenten: Für `2.7e-11` wird der Faktor
//! > `1e25`; `round(2.7e-11 * 1e25) / 1e25` ergibt `2.6999999999999997e-11`.
//! > … erforderlich ist eine Begrenzung der tatsächlich serialisierten
//! > Dezimaldarstellung."
//!
//! **WN3-02** schließt ihn: die 15-Stellen-Grenze greift an der
//! SERIALISIERTEN Dezimaldarstellung (`eqcop_broker::vertrag::wire_zahl`,
//! Spiegel von `nakama::wire::wireZahl`), nicht an einer numerischen Rundung.
//! Diese Datei misst drei Ebenen derselben Zusage:
//!
//! * **(a)** ein Eigenschaftstest über 30 000 Werte aus drei Bändern —
//!   `[0,1]`, `[-200,200]` und dem Dekadenband `1e-15..1e-9`, in dem die
//!   alte Rundung fiel;
//! * **(b)** die Wertetabelle, die auch die dreisprachige Fixture führt;
//! * **(c)** der Produktpfad: eine Sitzung mit einem Befund kleiner
//!   Rangkomponenten geht durch `v3_nachricht_lesen` und wird über
//!   `resubscribe_snapshot_push` ausgeliefert, ohne in
//!   `routing_fail_closed` zu fallen.
//!
//! Der Harnisch ist der aus `sonde014_nacharbeit2.rs`: ein Coordinator mit
//! ECHTEM SQLite-Store, Master und Sonde angemeldet, Belege über `p1`. Ohne
//! Store misst (c) nichts — der Re-Subscribe sendet dort die gespeicherte
//! Projektion, und genau ihre Bytes sind der Gegenstand.

#![cfg(windows)]

use eqcop_broker::coordinator::{Coordinator, ManualClock};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
use eqcop_broker::transport::server_v3::Senke;
use eqcop_broker::vertrag::{textriegel_bytes, wire_zahl};
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
const TRANSPORT_EPOCHE: u64 = 17;
const BASIS_SAMPLE: i64 = 44_108_200;

/// Ein Evidenzschnappschuss der Bühne, mit stellbarem Anomaliehub.
///
/// `sockel_db_zehntel` hebt alles AUSSER dem Anomalieband, `anomalie_zehntel`
/// das Anomalieband selbst — beides in Zehntel-dB, wie der Wire-Encoding
/// `q_db_0p1_i16` sie führt.
fn evidenz(a: &Adresse, nr: usize, projekt_start: i64, sockel_db_zehntel: i64, anomalie_zehntel: i64) -> Vec<u8> {
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
            let laenge = werte.len();
            if sockel_db_zehntel != 0 {
                for (index, eintrag) in werte.iter_mut().enumerate().take(laenge) {
                    if !(ANOMALIEBAND..ANOMALIEBAND + 4).contains(&index) {
                        let alt = eintrag.as_i64().unwrap_or(0);
                        *eintrag = json!(alt + sockel_db_zehntel);
                    }
                }
            }
            for index in ANOMALIEBAND..(ANOMALIEBAND + 4).min(laenge) {
                let alt = werte[index].as_i64().unwrap_or(0);
                werte[index] = json!(alt + anomalie_zehntel);
            }
        }
    }
    serde_json::to_vec(&wert).unwrap()
}

struct TestOrdner(std::path::PathBuf);

impl TestOrdner {
    fn neu(name: &str) -> Self {
        let pfad = std::env::temp_dir().join(format!(
            "nakama-sonde014-nacharbeit3-{name}-{}-{}",
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

    fn belege(&self, link: &str, a: &Adresse, ab_nr: usize, anzahl: usize, sockel: i64, anomalie: i64) {
        for i in 0..anzahl {
            self.c.p1(
                link,
                &evidenz(a, ab_nr + i, BASIS_SAMPLE + (i as i64) * 512, sockel, anomalie),
            );
        }
    }

    fn befunde(&self) -> Vec<eqcop_broker::coordinator::CauseHypothesis> {
        self.c
            .befunde_sicht(&self.master.project_binding_id, &self.master.session_epoch)
    }

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

    /// Die gewoehnliche Buehne: Passage, Abo, Belege — bis ein Befund steht
    /// und die Sessionprojektion committet ist.
    fn buehne_mit_befund(&self) {
        self.intent_marke(0);
        self.passage_anlegen(0x943, 0xac4, 12);
        self.abonniert();
        self.belege("main", &self.master, 0, 12, 0, 90);
        self.belege("sonde0", &self.sonde, 100, 12, 0, 90);
        assert!(!self.befunde().is_empty(), "die Buehne traegt einen Befund");
        assert!(
            self.sql("SELECT COUNT(*) FROM sessions") > 0,
            "und die Sessionprojektion ist committet"
        );
    }

    /// Setzt die Rangkomponenten der COMMITTETEN Projektion auf die
    /// Kleinwerte des Urteils — jeden durch `deckel` — und misst, ob der
    /// Re-Subscribe sie ausliefert.
    ///
    /// Der Weg ist der aus `subscription.rs`: `session_state_lesen` liefert
    /// genau diese Bytes, `v3_nachricht_lesen` beurteilt sie, und ein
    /// abgelehntes Dokument endet in `routing_fail_closed` — ohne Push.
    fn projektion_mit_rangwerten_pruefen(&self, deckel: &dyn Fn(f64) -> f64) -> usize {
        // Die Werte aus WP2-2, verteilt auf die sechs Komponenten.
        const KLEIN: [f64; 6] = [
            2.7e-11,
            5.55e-12,
            9.038_084_803_672_431e-11,
            1.234_567_890_123_456_7e-10,
            3.3e-12,
            7.7e-13,
        ];
        let db = rusqlite::Connection::open(self.ordner.db()).expect("Store liegt da");
        let roh: Vec<u8> = db
            .query_row("SELECT state_jcs FROM sessions", [], |z| z.get(0))
            .expect("die Projektion steht im Store");
        let mut wert: Value = serde_json::from_slice(&roh).expect("Projektion ist JSON");
        let mut gesetzt = 0usize;
        for befund in wert["findings"].as_array_mut().expect("findings").iter_mut() {
            for (index, feld) in [
                "bandpassung",
                "koinzidenz",
                "uplift",
                "intent_relevanz",
                "wiederholbarkeit",
                "routingqualitaet",
            ]
            .iter()
            .enumerate()
            {
                befund["rang"][*feld] = json!(deckel(KLEIN[index]));
                gesetzt += 1;
            }
        }
        assert_eq!(gesetzt, 6, "genau ein Befund mit sechs Komponenten");
        let neu = serde_json::to_vec(&wert).expect("Projektion ist serialisierbar");
        db.execute("UPDATE sessions SET state_jcs = ?1", [&neu])
            .expect("Projektion wird ersetzt");
        drop(db);

        self.push.leeren();
        self.abonniert();
        self.push.payloads().len()
    }
}

// ── Werkzeug der Zahlenprüfung ───────────────────────────────────────────

/// Signifikante Dezimalziffern GENAU SO gezählt, wie der Riegel es tut
/// (`vertrag.rs`): führende und abschließende Nullen tragen nichts.
fn signifikante_stellen(text: &str) -> usize {
    let ohne_vorzeichen = text.trim_start_matches('-');
    let mantisse = ohne_vorzeichen
        .split(['e', 'E'])
        .next()
        .unwrap_or(ohne_vorzeichen);
    let ziffern: String = mantisse.chars().filter(char::is_ascii_digit).collect();
    ziffern.trim_start_matches('0').trim_end_matches('0').len()
}

/// Die Zusage in EINEM Satz: was `wire_zahl` liefert, hält der Riegel — in
/// genau den Bytes, die `serde_json` schreibt.
fn haelt_den_riegel(x: f64) -> Result<(), String> {
    let gedeckelt = wire_zahl(x);
    let bytes = serde_json::to_vec(&gedeckelt).map_err(|e| e.to_string())?;
    let text = String::from_utf8(bytes.clone()).map_err(|e| e.to_string())?;
    textriegel_bytes(format!("{{\"a\":{text}}}").as_bytes())
        .map_err(|grund| format!("{x:?} -> {text} ({} Stellen): {grund}", signifikante_stellen(&text)))
}

/// Ein deterministischer Zufallsstrom (SplitMix64).
///
/// Deterministisch ist Absicht: eine Zusage, deren Beweis flackert, ist
/// keine — dieselbe Lehre wie beim Testhaken aus NR-03. Der Strom deckt
/// dennoch den ganzen Wertebereich ab, weil er die Mantisse voll ausschöpft.
struct Strom(u64);

impl Strom {
    fn naechste(&mut self) -> u64 {
        self.0 = self.0.wrapping_add(0x9E37_79B9_7F4A_7C15);
        let mut z = self.0;
        z = (z ^ (z >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
        z = (z ^ (z >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
        z ^ (z >> 31)
    }

    /// Gleichverteilt in `[0, 1)`, mit voller 53-Bit-Mantisse.
    fn einheit(&mut self) -> f64 {
        (self.naechste() >> 11) as f64 / (1u64 << 53) as f64
    }
}

// =========================================================================
// WP2-2 . WN3-02 (a) - der Eigenschaftstest ueber drei Baender
// =========================================================================
//
// Das dritte Band ist der Befund: die numerische Rundung der Nacharbeit 2
// bildete den Faktor `10f64.powi(14 - exponent)`, und der ist ab
// `exponent <= -9` selbst ungenau. Von 100 000 Werten aus `1e-15..1e-9`
// wurden dadurch 34 535 vertragswidrig - jeder von ihnen haette den
// `session_snapshot` seiner Sitzung fuer den EIGENEN Leser unlesbar gemacht.
#[test]
fn wire_zahl_haelt_den_textriegel_ueber_drei_baender() {
    let mut strom = Strom(0x5EED_0014_0003);
    let mut geprueft = 0usize;
    let mut kleine = 0usize;

    for _ in 0..10_000 {
        // Band 1: `[0,1]` - die sechs Rangkomponenten und `confidence.score`.
        let a = strom.einheit();
        haelt_den_riegel(a).expect("Band [0,1]");

        // Band 2: `[-200,200]` - `beobachtung.wert_db` und der
        // Maskierungswert.
        let b = strom.einheit() * 400.0 - 200.0;
        haelt_den_riegel(b).expect("Band [-200,200]");

        // Band 3: das Dekadenband `1e-15..1e-9`, in dem die alte Rundung
        // fiel.
        let exponent = -15.0 + strom.einheit() * 6.0;
        let c = 10f64.powf(exponent);
        haelt_den_riegel(c).expect("Dekadenband 1e-15..1e-9");
        kleine += 1;

        geprueft += 3;
    }

    assert_eq!(geprueft, 30_000);
    assert_eq!(kleine, 10_000);
    println!("{geprueft} Zufallswerte aus drei Baendern gegen den Textriegel geprueft");
}

// =========================================================================
// WP2-2 . WN3-02 (b) - die Wertetabelle, dreisprachig gefuehrt
// =========================================================================
//
// Dieselben Werte stehen als Klassen in `evidenz-zahlen-wire-v1.json` und
// werden dort von C++ (`nakama::wire::wireZahl`, B3c) und Python (A5) gegen
// DENSELBEN Text gehalten. Hier steht die Rustseite: derselbe Wert, derselbe
// Riegel, und die Stellenzahl ausdruecklich gezaehlt - `is_ok()` allein
// saehe auch dann gruen aus, wenn der Riegel eines Tages weicher wuerde.
#[test]
fn die_wertetabelle_der_kleinen_zahlen_haelt_den_riegel() {
    // `-3.3f32 as f64` ist der Fund aus N-30: `beobachtung.wert_db` entsteht
    // aus einem `f32`, und als `f64` gedruckt traegt er sechzehn Stellen.
    let f32_wert = -3.3f32 as f64;
    let faelle: [(&str, f64); 5] = [
        ("2.7e-11", 2.7e-11),
        ("5.55e-12", 5.55e-12),
        ("9.038084803672431e-11", 9.038_084_803_672_431e-11),
        ("-3.3f32 als f64", f32_wert),
        ("0.1+0.2", 0.1 + 0.2),
    ];

    for (name, roh) in faelle {
        let gedeckelt = wire_zahl(roh);
        let text = serde_json::to_string(&gedeckelt).expect("f64 ist serialisierbar");
        let stellen = signifikante_stellen(&text);
        assert!(
            stellen <= 15,
            "{name}: `{text}` traegt {stellen} signifikante Stellen"
        );
        haelt_den_riegel(roh).unwrap_or_else(|grund| panic!("{name}: {grund}"));

        // Und der Deckel ist eine RUNDUNG, keine Ersetzung: der Wert bleibt
        // im Rahmen der 15 Stellen derselbe. Ohne diese Zeile waere ein
        // `wire_zahl`, das stumpf 0 liefert, ebenfalls gruen.
        if roh != 0.0 {
            let abweichung = ((gedeckelt - roh) / roh).abs();
            assert!(
                abweichung < 1e-14,
                "{name}: `{gedeckelt:?}` ist nicht mehr der gemessene Wert {roh:?}"
            );
        }
    }

    // Die Gegenprobe der alten Rundung, damit der Fall nicht nur beschreibt,
    // was er misst: `round(2.7e-11 * 1e25) / 1e25` ist `2.6999999999999997e-11`
    // - siebzehn Stellen, und der eigene Riegel verwirft sie.
    let exponent = 2.7e-11f64.abs().log10().floor() as i32;
    let faktor = 10f64.powi(14 - exponent);
    let alt = (2.7e-11 * faktor).round() / faktor;
    let alt_text = serde_json::to_string(&alt).unwrap();
    assert!(
        signifikante_stellen(&alt_text) > 15,
        "die alte Rundung war der Befund: `{alt_text}`"
    );
    assert!(
        textriegel_bytes(format!("{{\"a\":{alt_text}}}").as_bytes()).is_err(),
        "und der eigene Riegel verwirft sie wirklich"
    );

    println!("5 Kleinwerte und die Gegenprobe der alten Rundung geprueft");
}

// =========================================================================
// WP2-2 . WN3-02 (c) - der Produktpfad: Befund, Snapshot, Re-Subscribe
// =========================================================================
//
// Die Wirkkette wortwoertlich aus dem Urteil: `befund_json` -> `sicht.rs`
// `serde_json::to_vec` -> `subscription.rs` `v3_nachricht_lesen` ->
// `vertrag.rs` `signifikante_stellen > 15` -> `routing_fail_closed`.
//
// Drei Abschnitte, weil die Zusage zwei Haelften hat und eine Gegenprobe
// braucht:
//
//   A. der ECHTE Produktpfad einer Buehne mit Befund - ihre `bandpassung`
//      traegt SECHZEHN signifikante Stellen (`0.09980920462281039`), und
//      ohne Deckel faellt der Re-Subscribe an ihr;
//   B. die KLEINWERTE aus dem Urteil in der gespeicherten Projektion - die
//      Groessenordnung `1e-11`, in der die numerische Rundung der
//      Nacharbeit 2 versagte;
//   C. die Gegenprobe: dieselbe Projektion mit den Zahlen der ALTEN Rundung
//      wird NICHT ausgeliefert. Ohne sie waere B auch dann gruen, wenn der
//      Riegel gar nicht mehr griffe.
//
// Warum B ueber die Projektion und nicht ueber die Rechnung: die Bandwerte
// des Wire-Encodings `q_db_0p1_i16` sind geklemmt, und `bandpassung` hat
// deshalb in JEDER Buehne dieses Korpus einen Boden bei `1/217` - gemessen
// ueber Sockel bis 90 dB und Anomaliehuebe bis -150 dB. Die Groessenordnung
// `1e-11` entsteht am echten Rechenweg erst mit einer Dynamik, die dieser
// Korpus nicht traegt; sie ist deshalb hier eingesetzt, statt behauptet zu
// werden - und zwar mit genau der Funktion, die die Zusage traegt.
#[test]
fn ein_befund_kleiner_rangkomponenten_wird_ausgeliefert() {
    // ── A. Der echte Produktpfad ─────────────────────────────────────────
    let h = Harnisch::neu("wn3-02a");
    h.intent_marke(0);
    h.passage_anlegen(0x943, 0xac4, 12);
    h.abonniert();
    h.belege("main", &h.master, 0, 12, 0, 90);
    h.belege("sonde0", &h.sonde, 100, 12, 0, 90);

    let befunde = h.befunde();
    assert!(!befunde.is_empty(), "die Buehne traegt einen Befund");
    // Die ROHE Zahl der Rechnung - vor jedem Deckel. Sie ist der Grund,
    // warum dieser Pfad ueberhaupt einen braucht.
    let roh = befunde[0].rang.bandpassung;
    assert!(
        signifikante_stellen(&format!("{roh:?}")) > 15,
        "die gerechnete `bandpassung` traegt wirklich mehr als 15 Stellen: {roh:?}"
    );

    h.push.leeren();
    h.abonniert();
    let gesendet = h.push.payloads();
    assert!(
        !gesendet.is_empty(),
        "der Re-Subscribe liefert aus, statt in `routing_fail_closed` zu fallen"
    );
    let mut mit_befund = 0usize;
    for (_, wert) in &gesendet {
        let bytes = serde_json::to_vec(wert).unwrap();
        assert!(
            textriegel_bytes(&bytes).is_ok(),
            "der gesendete Snapshot haelt den eigenen Textriegel: {:?}",
            textriegel_bytes(&bytes)
        );
        if wert
            .get("findings")
            .and_then(Value::as_array)
            .is_some_and(|f| !f.is_empty())
        {
            mit_befund += 1;
            for befund in wert["findings"].as_array().unwrap() {
                for (feld, zahl) in befund["rang"].as_object().unwrap() {
                    let text = serde_json::to_string(zahl).unwrap();
                    assert!(
                        signifikante_stellen(&text) <= 15,
                        "rang.{feld} reist mit {} Stellen: {text}",
                        signifikante_stellen(&text)
                    );
                }
            }
        }
    }
    assert!(
        mit_befund > 0,
        "und er traegt den Befund, statt ihn wegzulassen"
    );

    // ── B. Die Kleinwerte des Urteils, in der gespeicherten Projektion ───
    let klein = Harnisch::neu("wn3-02b");
    klein.buehne_mit_befund();
    let ausgeliefert = klein.projektion_mit_rangwerten_pruefen(&|x| wire_zahl(x));
    assert!(
        ausgeliefert > 0,
        "eine Projektion mit Rangkomponenten um 1e-11 wird ausgeliefert"
    );

    // ── C. Die Gegenprobe: dieselben Werte, alte Rundung ─────────────────
    let alt = Harnisch::neu("wn3-02c");
    alt.buehne_mit_befund();
    let ausgeliefert_alt = alt.projektion_mit_rangwerten_pruefen(&alte_rundung);
    assert_eq!(
        ausgeliefert_alt, 0,
        "mit der numerischen Rundung der Nacharbeit 2 faellt derselbe          Re-Subscribe in `routing_fail_closed` - genau das misst B"
    );

    println!(
        "A: {mit_befund} Snapshots mit Befund ausgeliefert (rohe bandpassung {roh:?});          B: {ausgeliefert} mit Kleinwerten; C: {ausgeliefert_alt} mit der alten Rundung"
    );
}

/// `auf_wirestellen` der Nacharbeit 2, woertlich - nur noch als Gegenprobe.
fn alte_rundung(wert: f64) -> f64 {
    if wert == 0.0 || !wert.is_finite() {
        return wert;
    }
    let exponent = wert.abs().log10().floor() as i32;
    if !(-290..=290).contains(&exponent) {
        return wert;
    }
    let faktor = 10f64.powi(14 - exponent);
    let gerundet = (wert * faktor).round() / faktor;
    if gerundet.is_finite() {
        gerundet
    } else {
        wert
    }
}
