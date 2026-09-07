//! Die Storefälle der **Nacharbeit 1** (SONDE-014, Erstprüfung 1 vom
//! 07.09.2026): NR-04, NR-05, NR-09, NR-10 und NR-11.
//!
//! ── WARUM DIESE DATEI EXISTIERT ───────────────────────────────────────────
//!
//! Die Erstprüfung 1 hat fünf Zusagen gefunden, die nur **gegen einen echten
//! SQLite-Store** messbar sind — und deren bestehende Beine ihn genau deshalb
//! nicht hatten:
//!
//! * **NR-04** (M-28): „existent" ist eine Prüfung gegen die Tabelle
//!   `evidence`, nicht gegen den Speicher. Ein Test ohne Store misst diese
//!   Zusage nicht, egal wie gründlich er den Cache prüft.
//! * **NR-05** (M-23): die Mindestfensterzahl gilt **je beteiligter Quelle**
//!   in der Passage. Eine benannte Passage entsteht nur aus einem
//!   `experiment_begin`, und das ist ein persistenzpflichtiger P0 — ohne
//!   Store ist es `abgelehnt/internal`.
//! * **NR-09** (M-48, Entscheid E-13): das Experimentziel überlebt den
//!   Brokerneustart. Ein Neustart ohne Store ist keiner.
//! * **NR-10** (M-73): `user_verdict` wird persistiert und erst danach
//!   bestätigt.
//! * **NR-11** (M-71): der versionierte SQLite-Spiegel des Intents.
//!
//! Der Harnisch ist bewusst eine eigene Kopie und keine geteilte Hilfsdatei —
//! dieselbe Regel wie in `sonde014_befund.rs`: ein gemeinsames Modul zwischen
//! zwei Integrationszielen wäre ein drittes Ziel, das niemand baut.

#![cfg(windows)]

use eqcop_broker::coordinator::{Ausschlussgrund, Befundzustand, Coordinator, ManualClock};
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

fn capabilities() -> Value {
    json!({
        "host_context": true, "project_time": true, "fine_automation": false,
        "double_precision": false, "latency_report": false, "aux_send": false,
        "aux_return": false, "compare_routing": false, "sidechain": false,
        "offline_render": false
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
        .get_or_init(|| {
            fixture("evidence-snapshot-mit-ereignissen-und-stereo")
        })
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
            "nakama-sonde014-nacharbeit1-{name}-{}-{}",
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

/// Ein Coordinator MIT echtem Store, dazu Master und Sonde angemeldet.
struct Harnisch {
    c: Coordinator,
    master: Adresse,
    sonde: Adresse,
    writer: eqcop_broker::store::StoreWriter,
    ordner: TestOrdner,
}

impl Harnisch {
    fn neu(name: &str) -> Self {
        let ordner = TestOrdner::neu(name);
        let (c, writer) = Self::coordinator_auf(&ordner);
        let harnisch = Self {
            c,
            master: adresse(1),
            sonde: adresse(2),
            writer,
            ordner,
        };
        harnisch.anmelden();
        harnisch
    }

    fn coordinator_auf(
        ordner: &TestOrdner,
    ) -> (Coordinator, eqcop_broker::store::StoreWriter) {
        let mut k = eqcop_broker::store::StoreKonfiguration::fuer_pfad(ordner.db());
        k.remote_volume_override = Some(false);
        let writer = eqcop_broker::store::StoreWriter::starten(k);
        assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());
        let c = Coordinator::mit_store(
            Arc::new(ManualClock::default()),
            hex(0xbeef),
            &writer,
        );
        (c, writer)
    }

    /// **Der Brokerneustart.** Derselbe Store, ein neuer Coordinator — genau
    /// das, was NR-09 und NR-11 messen.
    fn neustarten(&mut self) {
        let (c, writer) = Self::coordinator_auf(&self.ordner);
        self.c = c;
        self.writer = writer;
        self.anmelden();
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
            let position = if art == "main" { "insert" } else { "post" };
            let d = json!({
                "adresse": a,
                "plugin_kind": art,
                "measurement_position": position,
                "aussageklasse": "beobachtend",
                "betrieb": "active",
                "label": "Testquelle",
                "capabilities": capabilities(),
                "frische": {"letzter_kontakt_ms": 10, "stale": false},
                "host_mixer_index": mixer
            });
            assert!(self.c.descriptor_setzen(link, d));
            // Ohne `record_state` nimmt der Broker keinen persistenzpflichtigen
            // Befehl an - `experiment_begin` waere `record_state_unknown`.
            let bericht = json!({
                "type": "state_report",
                "adresse": a,
                "dsp_schema_version": 1,
                "state_revision": 0,
                "state_hash": "a".repeat(64),
                "record_state": {"valid": true, "recording": false}
            });
            assert!(self
                .c
                .state_report_json(link, &serde_json::to_vec(&bericht).unwrap()));
        }
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
        self.c.p1("main", &serde_json::to_vec(&wert).unwrap());
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

    fn snapshot(&self) -> Value {
        serde_json::from_slice(
            &self
                .c
                .session_snapshot_json(&self.master.project_binding_id, &self.master.session_epoch),
        )
        .expect("Snapshot ist JSON")
    }

    /// Eine benannte Passage über den PRODUKTPFAD: `experiment_begin` als P0.
    ///
    /// Die Passage deckt `fenster` Projektfenster ab Basis ab und trägt die
    /// Transportepoche der Evidenzgrundform — eine andere Epoche wäre eine
    /// andere Passage (M-23).
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
        let antwort = Senke::p0(&self.c, "main", &serde_json::to_vec(&wert).unwrap())
            .expect("experiment_begin wird beantwortet");
        let ack: Value = serde_json::from_slice(&antwort).unwrap();
        assert_eq!(
            ack["ergebnis"], "angewandt",
            "die Passage entsteht wirklich: {ack:?}"
        );
        passage_id
    }
}

// =========================================================================
// M-28 . NR-04 - „existent" ist eine Pruefung gegen den STORE
// =========================================================================
//
// Der sichtbare Snapshot serialisierte bis zur Nacharbeit 1 ausschliesslich
// den Cache; auch die Erzeugung filterte nur dessen `ausschlussgrund`. Eine
// `evidence`-Zeile, die im Store fehlt, blieb damit unsichtbar unsichtbar —
// der Befund stand weiter da, mitsamt seinen IDs.
#[test]
fn befund_mit_geloeschter_evidenzzeile_wird_unsichtbar() {
    let h = Harnisch::neu("nr04");
    h.intent_marke(0);
    h.belege("main", &h.master, 0, 12);
    h.belege("sonde0", &h.sonde, 100, 12);

    let vorher = h.befunde();
    assert!(!vorher.is_empty(), "die Buehne traegt einen Befund");
    let ids: Vec<String> = vorher
        .iter()
        .flat_map(|b| b.evidence_ids.iter().cloned())
        .collect();
    assert!(!ids.is_empty(), "und er traegt Belege");
    assert!(
        !h.snapshot()["findings"].as_array().unwrap().is_empty(),
        "und der Snapshot zeigt ihn"
    );

    // Die Zeilen verschwinden AUS DEM STORE — nicht aus dem Cache. Genau der
    // Zustand, den EP-04 beschreibt: der Speicher sagt „da", SQLite sagt
    // „fort".
    {
        let conn = rusqlite::Connection::open(h.ordner.db()).expect("Store oeffnen");
        let treffer = conn
            .execute("DELETE FROM evidence", [])
            .expect("Evidenzzeilen loeschen");
        assert!(treffer > 0, "es gab wirklich Zeilen zu entfernen");
    }

    let findings = h.snapshot();
    let leer = findings["findings"]
        .as_array()
        .map(Vec::is_empty)
        .unwrap_or(true);
    assert!(
        leer,
        "ohne Belegzeile im Store wird nichts mehr gezeigt: {:?}",
        findings["findings"]
    );
}

// =========================================================================
// M-23 . NR-05 - die Mindestfensterzahl gilt JE BETEILIGTER Quelle
// =========================================================================
//
// `fenster` zaehlte bis zur Nacharbeit 1 nur die Kandidatenfenster. Weil
// `ueberdeckung` relativ rechnet und auf das kuerzere Intervall normiert,
// bestanden acht Kandidatenfenster gegen ein EINZIGES ueberlappendes
// Masterfenster alle vier relativen Gates — und ein einzelner Kandidat
// erreichte `hoch`/READY.
#[test]
fn passagengate_misst_auch_die_fenster_des_masters() {
    // (1) Acht Kandidatenfenster, EIN Masterfenster.
    let h = Harnisch::neu("nr05-duenn");
    h.intent_marke(0);
    h.passage_anlegen(0x900, 0xab1, 8);
    h.belege("main", &h.master, 0, 1);
    h.belege("sonde0", &h.sonde, 100, 8);

    let befunde = h.befunde();
    let befund = befunde.first().expect("auch hier gibt es ein Ergebnis");
    assert_ne!(
        befund.zustand,
        Befundzustand::ReadyToSend,
        "ein einziges Masterfenster traegt keine starke Aussage: {:?}",
        (befund.confidence, befund.zustand)
    );
    let gruende: Vec<Ausschlussgrund> = befunde
        .iter()
        .flat_map(|b| b.ausschluesse.iter().map(|a| a.grund))
        .collect();
    assert!(
        gruende.contains(&Ausschlussgrund::PassageZuKurz),
        "und der Grund steht dran: {gruende:?}"
    );

    // (2) GEGENPROBE: derselbe Aufbau mit acht Masterfenstern wird handelbar.
    // Ohne sie waere (1) eine Regressionswache und kein Beleg.
    let g = Harnisch::neu("nr05-dicht");
    g.intent_marke(0);
    g.passage_anlegen(0x901, 0xab2, 8);
    g.belege("main", &g.master, 0, 8);
    g.belege("sonde0", &g.sonde, 100, 8);
    let stark = g.befunde();
    let befund = stark.first().expect("ein Befund");
    assert_eq!(
        befund.zustand,
        Befundzustand::ReadyToSend,
        "mit genug Material auf BEIDEN Seiten wird derselbe Fall handelbar: {:?}",
        (befund.confidence, befund.rang)
    );
}
