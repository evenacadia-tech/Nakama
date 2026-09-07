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
                werte[index] = json!(alt + anhebung);
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
        self.experiment_mit_ziel(command, experiment, fenster, None)
    }

    /// Wie `passage_anlegen`, aber mit einem ausdruecklichen `ziel` (E-05).
    fn experiment_mit_ziel(
        &self,
        command: usize,
        experiment: usize,
        fenster: i64,
        ziel: Option<Value>,
    ) -> String {
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
        if let Some(z) = ziel {
            wert["ziel"] = z;
        }
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

/// **NR-04, der FLUSH-Pfad.** `session_snapshot_json` ist die Lesesicht der
/// Beine; der Push des Produkts geht durch `flush_session`.
///
/// Beide bauen denselben Snapshot (`snapshot_locked`), aber nur einer von
/// beiden ist der Weg, auf dem Gen die Behauptung wirklich sieht. Ein Riegel,
/// der nur an der Lesesicht haengt, waere genau die Sorte Beleg, die
/// `tools/dirigent/pruefliste.md` E ausschliesst.
///
/// Gemessen wird ohne Evidenzaenderung: eine gestiegene Intent-Revision
/// veraltet die Befunde und loest den Flush aus. Damit laeuft
/// `befunde_eintragen` NICHT mit — die Haertung, die hier greift, ist die im
/// Flush-Pfad und keine andere.
#[test]
fn der_flush_pfad_haelt_die_belege_ebenfalls_gegen_den_store() {
    let h = Harnisch::neu("nr04-flush");
    h.intent_marke(0);
    h.belege("main", &h.master, 0, 12);
    h.belege("sonde0", &h.sonde, 100, 12);
    assert!(!h.befunde().is_empty(), "die Buehne traegt einen Befund");

    {
        let conn = rusqlite::Connection::open(h.ordner.db()).expect("Store oeffnen");
        assert!(
            conn.execute("DELETE FROM evidence", []).expect("loeschen") > 0,
            "es gab Zeilen zu entfernen"
        );
    }
    // Der Cache traegt sie noch: ohne einen Weg, der die Belege prueft, bliebe
    // die Behauptung stehen.
    // `befunde_sicht` liest nur den fluechtigen Stand und haertet nichts: der
    // Befund steht noch da. Genau darum geht es - ohne einen Weg, der die
    // Belege prueft, bliebe die Behauptung auf dem Draht.
    assert!(
        !h.befunde().is_empty(),
        "der fluechtige Bestand haelt den Befund weiter"
    );

    // Eine gestiegene Bestandsrevision veraltet die Befunde und flusht die
    // Sitzung. KEINE Evidenzaenderung, also keine Neurechnung.
    let hoehere = json!({
        "type": "intent_update",
        "adresse": h.master,
        "session_epoch": h.master.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 1
    });
    h.c.p1("main", &serde_json::to_vec(&hoehere).unwrap());

    assert!(
        h.befunde().is_empty(),
        "der Flush-Pfad haelt die Belege gegen den Store: {:?}",
        h.befunde().iter().map(|b| b.finding_id.clone()).collect::<Vec<_>>()
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

// =========================================================================
// M-48 . NR-09 (Entscheid E-13) - das Experimentziel ueberlebt den Neustart
// =========================================================================
//
// `experiment_json()` schrieb das Ziel nicht, und die Wiederherstellung
// setzte es ausdruecklich auf `None`. Ein mit Ziel- und Schutzbaendern
// begonnener Versuch rechnete nach einem Brokerneustart wieder mit der
// Heuristik - und mit ihr aenderte sich die Guardrail-Auswertung.
#[test]
fn experimentziel_ueberlebt_den_brokerneustart() {
    let mut h = Harnisch::neu("nr09");
    let experiment_id = hex(0xab9);
    let ziel = json!({
        "band_von": 40,
        "band_bis": 96,
        "geschuetzte_baender": [{"von": 10, "bis": 20}],
        "proposal_id": hex(0x7777)
    });
    h.experiment_mit_ziel(0x910, 0xab9, 8, Some(ziel.clone()));
    let vorher = h
        .c
        .experiment_sicht(&experiment_id)
        .expect("der Versuch liegt an");
    assert_eq!(
        vorher.ziel.as_ref().map(|z| (z.band_von, z.band_bis)),
        Some((40, 96)),
        "das Ziel ist angekommen"
    );

    // DER NEUSTART: derselbe Store, ein neuer Coordinator.
    h.neustarten();
    let nachher = h
        .c
        .experiment_sicht(&experiment_id)
        .expect("der Versuch ueberlebt den Neustart (M-47)");
    let z = nachher
        .ziel
        .as_ref()
        .expect("und sein ZIEL mit ihm (E-13, NR-09)");
    assert_eq!((z.band_von, z.band_bis), (40, 96), "die Bereiche sind dieselben");
    assert_eq!(
        z.geschuetzte_baender,
        vec![(10u32, 20u32)],
        "und die Schutzbaender ebenso"
    );
    assert_eq!(z.proposal_id.as_deref(), Some(hex(0x7777).as_str()));

    // GEGENPROBE: ein Versuch OHNE Ziel traegt nach dem Neustart weiter
    // keines - das ist der SONDE-013-Pfad, und er bleibt ehrlich.
    let mut g = Harnisch::neu("nr09-ohne");
    g.experiment_mit_ziel(0x911, 0xaba, 8, None);
    g.neustarten();
    assert!(
        g.c.experiment_sicht(&hex(0xaba))
            .expect("auch dieser Versuch ueberlebt")
            .ziel
            .is_none(),
        "ohne Ziel bleibt es bei der Heuristik (`ziel_geraten`)"
    );
}

// =========================================================================
// M-73/E-09 . NR-10 - `user_verdict` wird persistiert und bestaetigt
// =========================================================================
//
// Die Familie fiel bis zur Nacharbeit 1 in `_ => None`: der Schema-Leser
// nahm sie an, der Coordinator kannte sie nicht, und weder Persistenz noch
// ACK entstanden. Die Projektion `user_verdicts` hatte keinen Produzenten.
#[test]
fn user_verdict_wird_persistiert_und_bestaetigt() {
    let h = Harnisch::neu("nr10");
    let mut wert = fixture("user_verdict");
    wert["kopf"]["ziel"] = serde_json::to_value(&h.master).unwrap();
    wert["kopf"]["command_id"] = json!(hex(0x920));
    wert["kopf"]["base_revision"] = json!(0);

    let antwort = Senke::p0(&h.c, "main", &serde_json::to_vec(&wert).unwrap())
        .expect("NR-10: die Familie wird BEANTWORTET - vorher fiel sie in `_ => None`");
    let ack: Value = serde_json::from_slice(&antwort).unwrap();
    assert_eq!(ack["type"], "command_ack");
    assert_eq!(
        ack["ergebnis"], "angewandt",
        "der ACK kommt vom Coordinator, nicht von einem Testserver: {ack:?}"
    );

    // Und die Zeile steht wirklich in SQLite - der ACK kommt NACH dem Append.
    let zeilen = zaehle(&h, "SELECT COUNT(*) FROM user_verdicts");
    assert_eq!(zeilen, 1, "genau eine Zeile in der Projektion `user_verdicts`");
    let id: String = {
        let conn = rusqlite::Connection::open(h.ordner.db()).expect("Store");
        conn.query_row("SELECT user_verdict_id FROM user_verdicts", [], |r| r.get(0))
            .expect("die Zeile traegt ihre Kennung")
    };
    assert_eq!(id, wert["user_verdict_id"].as_str().unwrap());

    // IDEMPOTENZ unter derselben `command_id`: eine Wiederholung erzeugt
    // KEINE zweite Zeile. Ein Userurteil koalesziert nicht - aber es
    // verdoppelt sich auch nicht (E-09).
    let zweite = Senke::p0(&h.c, "main", &serde_json::to_vec(&wert).unwrap())
        .expect("die Wiederholung wird beantwortet");
    let ack2: Value = serde_json::from_slice(&zweite).unwrap();
    assert_eq!(
        ack2["ergebnis"], "idempotent_wiederholt",
        "dieselbe command_id ist dieselbe Absicht: {ack2:?}"
    );
    assert_eq!(
        zaehle(&h, "SELECT COUNT(*) FROM user_verdicts"),
        1,
        "und sie bleibt EINE Zeile"
    );
}

/// **NR-10, A4-SI:** dieselbe `command_id` ueber einen BROKERNEUSTART hinweg.
///
/// Der Kill ist hier der Neustart auf demselben Store - der Zustand, den ein
/// Brokerkill hinterlaesst. Der Sender wiederholt sein Urteil, weil er die
/// Antwort nicht bekommen hat; genau eine Zeile darf entstehen.
#[test]
fn user_verdict_wiederholt_sich_idempotent_ueber_den_brokerkill() {
    let mut h = Harnisch::neu("nr10-si");
    let mut wert = fixture("user_verdict");
    wert["kopf"]["ziel"] = serde_json::to_value(&h.master).unwrap();
    wert["kopf"]["command_id"] = json!(hex(0x921));
    wert["kopf"]["base_revision"] = json!(0);
    let payload = serde_json::to_vec(&wert).unwrap();

    let ack: Value =
        serde_json::from_slice(&Senke::p0(&h.c, "main", &payload).expect("ACK")).unwrap();
    assert_eq!(ack["ergebnis"], "angewandt");
    assert_eq!(zaehle(&h, "SELECT COUNT(*) FROM user_verdicts"), 1);

    h.neustarten();

    let nach: Value =
        serde_json::from_slice(&Senke::p0(&h.c, "main", &payload).expect("ACK nach dem Kill"))
            .unwrap();
    assert_eq!(
        nach["ergebnis"], "idempotent_wiederholt",
        "der Befehl ist committet - der Retry bekommt dieselbe Antwort: {nach:?}"
    );
    assert_eq!(
        zaehle(&h, "SELECT COUNT(*) FROM user_verdicts"),
        1,
        "genau EINE Zeile ueber den Kill hinweg"
    );
}

// =========================================================================
// M-71 . NR-11 - der versionierte SQLite-Spiegel des Intents
// =========================================================================
//
// Angenommene `intent_update`-Nachrichten aenderten bis zur Nacharbeit 1
// ausschliesslich `stand.intent`. Anders als beim AssistantStep gab es keinen
// Store-Append; nach einem Vollbericht enthielt SQLite keinen versionierten
// SourceIntent-Spiegel fuer Suche und Crashdiagnose.
#[test]
fn intent_bestand_wird_versioniert_gespiegelt() {
    let mut h = Harnisch::neu("nr11");
    h.intent_marke(1);
    let mit_rolle = json!({
        "type": "intent_update",
        "adresse": h.master,
        "session_epoch": h.master.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 2,
        "intents": [{
            "quelle_id": h.sonde.instance_id,
            "rolle": "geschuetzt",
            "revision": 1,
            "herkunft": "user",
            "konfidenz": 1.0
        }]
    });
    h.c.p1("main", &serde_json::to_vec(&mit_rolle).unwrap());

    let zeilen = spiegelzeilen(&h);
    assert_eq!(zeilen.len(), 2, "zwei Ereigniszeilen, eine je Update");
    let revisionen: Vec<i64> = zeilen
        .iter()
        .map(|z| z["bestand_revision"].as_i64().unwrap_or(-1))
        .collect();
    assert_eq!(revisionen, vec![1, 2], "mit STEIGENDER Revision");
    assert_eq!(
        zeilen[1]["intents"][0]["rolle"], "geschuetzt",
        "und der Spiegel traegt die Rolle, um die es geht"
    );

    // Nach dem Neustart ueberschreibt der Spiegel den MAIN-Bestand nicht:
    // er ist nie autoritativ (M-89-Analogie). Der Main meldet seinen
    // Vollbestand, und der gewinnt.
    h.neustarten();
    assert!(
        !h.c.darf_rechnen(&h.master.project_binding_id, &h.master.session_epoch),
        "der Spiegel hebt die Vollstaendigkeitsmarke NICHT - der Broker rechnet nicht"
    );
    let neuer_bestand = json!({
        "type": "intent_update",
        "adresse": h.master,
        "session_epoch": h.master.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 3
    });
    h.c.p1("main", &serde_json::to_vec(&neuer_bestand).unwrap());
    let sicht = h
        .c
        .intent_sicht(&h.master.project_binding_id, &h.master.session_epoch);
    assert_eq!(sicht.revision, 3, "der MAIN-Stand gilt");
    assert!(
        sicht.intents.is_empty(),
        "der aeltere Spiegel setzt die Rolle NICHT zurueck in den Bestand"
    );
}

/// Eine Zahl aus dem Store.
fn zaehle(h: &Harnisch, sql: &str) -> i64 {
    let conn = rusqlite::Connection::open(h.ordner.db()).expect("Store oeffnen");
    conn.query_row(sql, [], |r| r.get(0)).expect("Zaehlung")
}

/// Die gespiegelten Intent-Ereignisse dieser Sitzung, in Ereignisreihenfolge.
fn spiegelzeilen(h: &Harnisch) -> Vec<Value> {
    let conn = rusqlite::Connection::open(h.ordner.db()).expect("Store oeffnen");
    let mut stmt = conn
        .prepare(
            "SELECT payload_jcs FROM event_log WHERE event_type='intent_update' \
             AND project_binding_id=?1 AND session_epoch=?2 ORDER BY event_ord",
        )
        .expect("Abfrage");
    let rows = stmt
        .query_map(
            [&h.master.project_binding_id, &h.master.session_epoch],
            |r| r.get::<_, Vec<u8>>(0),
        )
        .expect("Zeilen");
    rows.map(|z| serde_json::from_slice(&z.expect("Zeile")).expect("JSON"))
        .collect()
}

// =========================================================================
// M-43/M-46 . NR-07 - das Exit-Gate am VOLLSTAENDIGEN Proposal
// =========================================================================
//
// `passage_id` fehlte in `$defs/proposal.required`: die lesende Gegenprobe
// der Erstpruefung hat das Feld aus `draft_offer`, `draft-offer-mehr-daten`
// und `draft-offer-keine-aenderung` entfernt, und alle drei blieben gueltig.
// Seit der Nacharbeit 1 steht es in `required` - und dieser Fall misst, dass
// der Erzeuger es auf einer Buehne MIT benannter Passage auch wirklich
// liefert, fuer alle sechs Gate-Felder.
#[test]
fn sechs_gate_felder_sind_pflicht_mit_passage() {
    let h = Harnisch::neu("nr07");
    h.intent_marke(0);
    let passage_id = h.passage_anlegen(0x930, 0xab7, 12);
    h.belege("main", &h.master, 0, 12);
    h.belege("sonde0", &h.sonde, 100, 12);

    let vorschlaege = h
        .c
        .vorschlaege_sicht(&h.master.project_binding_id, &h.master.session_epoch);
    assert!(!vorschlaege.is_empty(), "die Buehne traegt einen Vorschlag");
    for vorschlag in &vorschlaege {
        for (feld, belegt) in vorschlag.gate_felder_vollstaendig() {
            assert!(belegt, "Gate-Feld {feld} fehlt - auch `passage_id` ist Pflicht");
        }
        assert_eq!(
            vorschlag.passage_id.as_deref(),
            Some(passage_id.as_str()),
            "und es ist DIE Passage, in der gemessen wurde"
        );
    }

    // Das Wire-Objekt traegt jede Pflichtzeile des Vertrags - gelesen aus dem
    // Schema, nicht aus einer zweiten Liste.
    let vertrag: Value = serde_json::from_slice(
        &std::fs::read(
            std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
                .join("../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"),
        )
        .expect("Vertrag liegt im Repo"),
    )
    .expect("Vertrag ist JSON");
    let pflicht: Vec<&str> = vertrag["$defs"]["proposal"]["required"]
        .as_array()
        .expect("die Pflichtliste steht im Vertrag")
        .iter()
        .map(|v| v.as_str().unwrap())
        .collect();
    assert!(
        pflicht.contains(&"passage_id"),
        "NR-07: `passage_id` steht in `$defs/proposal.required`"
    );
    let wire = Coordinator::proposal_json_fuer_test(&vorschlaege[0]);
    let objekt = wire.as_object().expect("ein Objekt");
    for feld in &pflicht {
        assert!(objekt.contains_key(*feld), "Pflichtfeld {feld} fehlt am Draht");
    }
}
