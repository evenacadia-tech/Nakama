//! **NAK-212 — die adversarialen Gegenbeispiele am Produktpfad.**
//!
//! Dieses Bein trägt genau die Fälle, die das **Korpusformat nicht
//! ausdrücken kann** (`eq-copilot/fixtures/p5-korpus/sitzungen.json` kennt je
//! Quelle nur Reihe, Fensterzahl, Mixerkanal, Versatz und die Passage):
//!
//! * eine **Intent-Rolle**, die zwei messtechnisch identische Kandidaten
//!   trennen würde (N-15) und der **Dreierfall** dahinter (N-42),
//! * eine **Passage samt Randwerten** und der Fall, in dem **kein**
//!   Masterfenster in ihr liegt (N-23 bis N-25, N-29, N-43),
//! * eine Quelle in einem **anderen Band** als der Befund (N-02),
//! * **umklammernde und disjunkte** Fensterspannen (N-18, N-19),
//! * die **Rangdifferenz am Quantum** (N-16).
//!
//! Der Gate-Text von G5 nennt adversariale Gegenbeispiele **namentlich**
//! (`docs/bauaufteilung-sonden.md`:395-400); ein namentlich geforderter
//! Beweis, der nur in der Sammelzeile A4 steckt, ist von außen nicht als
//! gefahren zu erkennen — deshalb hat dieses Bein eine eigene Runnerzeile
//! (A31, dieselbe Begründung wie bei A21 `transport_fuzz`).
//!
//! Jeder Fall fährt durch DENSELBEN Produktpfad wie der Korpus:
//! `Coordinator` → `anmelden` → Vollständigkeitsmarke → `p1` →
//! `befunde_sicht`. Ein Bein, das `hypothesen()` direkt riefe, könnte grün
//! sein, während die Verdrahtung fehlt.

use eqcop_broker::coordinator::{
    rang_quantisiert, Ausschlussgrund, Befundzustand, CauseHypothesis, Coordinator, ManualClock,
    Sicherheitsklasse, Ursachenklasse,
};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
use eqcop_broker::transport::server_v3::Senke;
use serde_json::{json, Value};
use std::sync::Arc;

const BAND_VON: usize = 98;
const BAND_BIS: usize = 102;
const BASIS: i64 = 44_108_200;
const FENSTER: i64 = 512;

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

fn wurzel() -> std::path::PathBuf {
    std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .expect("das Repo liegt ueber broker/")
        .to_path_buf()
}

fn fixture(name: &str) -> Value {
    let pfad = wurzel().join(format!("eq-copilot/fixtures/v3/gueltig/{name}.json"));
    serde_json::from_slice(&std::fs::read(&pfad).expect("Fixture liegt im Korpus"))
        .expect("Fixture ist JSON")
}

fn grundform() -> Value {
    static ROH: std::sync::OnceLock<Value> = std::sync::OnceLock::new();
    ROH.get_or_init(|| fixture("evidence-snapshot-mit-ereignissen-und-stereo"))
        .clone()
}

/// Ein Beleg mit Bandanhebung und optionaler Onsetstärke.
fn evidenz(
    a: &Adresse,
    nr: usize,
    projekt_start: i64,
    anhebung_db: f64,
    band: (usize, usize),
) -> Vec<u8> {
    evidenz_zwei_baender(a, nr, projekt_start, anhebung_db, band, None)
}

/// Wie `evidenz`, aber mit einer zusätzlichen Anhebung in einem ZWEITEN Band.
///
/// Sie ist der Kern von N-16: der Unterschied zwischen zwei sonst identischen
/// Kandidaten liegt in einem Band weit **außerhalb** des Befundintervalls und
/// hat für die Ursachenbehauptung keine Bedeutung — er bewegt nur die
/// `bandpassung` und mit ihr den Gesamtrang.
fn evidenz_zwei_baender(
    a: &Adresse,
    nr: usize,
    projekt_start: i64,
    anhebung_db: f64,
    band: (usize, usize),
    zusatz: Option<(usize, usize, f64)>,
) -> Vec<u8> {
    let mut wert = grundform();
    wert["adresse"] = serde_json::to_value(a).unwrap();
    wert["evidence_id"] = json!(hex(0x10000 + nr));
    wert["transport"]["sequence"] = json!(nr as u64 + 1);
    wert["transport"]["project_sample_start"] = json!(projekt_start);
    for pfad in [
        "/baender/werte",
        "/verteilung/p10/werte",
        "/verteilung/p50/werte",
        "/verteilung/p95/werte",
    ] {
        if let Some(Value::Array(werte)) = wert.pointer_mut(pfad) {
            for index in band.0..band.1.min(werte.len()) {
                let alt = werte[index].as_i64().unwrap_or(0);
                werte[index] = json!(alt + (anhebung_db * 10.0).round() as i64);
            }
            if let Some((von, bis, db)) = zusatz {
                for index in von..bis.min(werte.len()) {
                    let alt = werte[index].as_i64().unwrap_or(0);
                    werte[index] = json!(alt + (db * 10.0).round() as i64);
                }
            }
        }
    }
    serde_json::to_vec(&wert).unwrap()
}

/// Das gleichläufige Muster: jedes zweite Fenster laut, das LETZTE immer.
///
/// Das letzte Fenster bestimmt `masteranomalie`; wäre es leise, fände sie eine
/// andere Bandgruppe und der Befund zeigte auf ein Band, in dem die Quelle
/// nichts tut.
fn wechselnd(anzahl: usize) -> impl Fn(usize) -> f64 {
    let laut_bei = (anzahl.max(1) - 1) % 2;
    move |i| if i % 2 == laut_bei { 9.0 } else { 0.0 }
}

struct TestOrdner(std::path::PathBuf);

impl TestOrdner {
    fn neu(name: &str) -> Self {
        let pfad = std::env::temp_dir()
            .join(format!("nakama-nak212-{name}-{}", std::process::id()));
        let _ = std::fs::remove_dir_all(&pfad);
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

/// Der gemeinsame Harnisch: ein `Coordinator`, auf Wunsch mit Store.
struct Buehne {
    c: Arc<Coordinator>,
    master: Adresse,
    #[allow(dead_code)]
    writer: Option<eqcop_broker::store::StoreWriter>,
    #[allow(dead_code)]
    ordner: Option<TestOrdner>,
}

impl Buehne {
    /// Ohne Store — genügt für alles außer `experiment_begin`.
    fn schlank() -> Self {
        Self {
            c: Arc::new(Coordinator::mit_uhr(
                Arc::new(ManualClock::default()),
                hex(0xbeef),
            )),
            master: adresse(1),
            writer: None,
            ordner: None,
        }
    }

    /// Mit echtem Store — `experiment_begin` ist ein persistenzpflichtiger P0.
    fn mit_store(name: &str) -> Self {
        let ordner = TestOrdner::neu(name);
        let mut k = eqcop_broker::store::StoreKonfiguration::fuer_pfad(ordner.db());
        k.remote_volume_override = Some(false);
        let writer = eqcop_broker::store::StoreWriter::starten(k);
        assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());
        let c = Coordinator::mit_store(Arc::new(ManualClock::default()), hex(0xbeef), &writer);
        Self {
            c: Arc::new(c),
            master: adresse(1),
            writer: Some(writer),
            ordner: Some(ordner),
        }
    }

    fn anmelden(&self, link: &str, a: &Adresse, art: &str, mixer: Option<i64>) {
        let h = hello(a.clone(), art);
        assert!(self.c.control_hello_registrieren(link, &h).angenommen);
        let _ = self.c.resync_bestaetigen(link, 0);
        let position = if art == "main" { "insert" } else { "post" };
        let mut d = json!({
            "adresse": a,
            "plugin_kind": art,
            "measurement_position": position,
            "aussageklasse": "beobachtend",
            "betrieb": "active",
            "label": "Gegenbeispielquelle",
            "capabilities": capabilities(),
            "frische": {"letzter_kontakt_ms": 10, "stale": false}
        });
        if let Some(index) = mixer {
            d["host_mixer_index"] = json!(index);
        }
        assert!(self.c.descriptor_setzen(link, d));
    }

    /// M-86: der Vollbestand MIT Marke, bevor gerechnet wird.
    fn marke(&self, revision: i64, intents: Value) {
        let wert = json!({
            "type": "intent_update",
            "adresse": self.master,
            "session_epoch": self.master.session_epoch,
            "vollstaendig": true,
            "bestand_revision": revision,
            "intents": intents
        });
        self.c.p1("main", &serde_json::to_vec(&wert).unwrap());
    }

    fn state_report(&self) {
        let bericht = json!({
            "type": "state_report",
            "adresse": self.master,
            "dsp_schema_version": 1,
            "state_revision": 0,
            "state_hash": "a".repeat(64),
            "record_state": {"valid": true, "recording": false}
        });
        assert!(self
            .c
            .state_report_json("main", &serde_json::to_vec(&bericht).unwrap()));
    }

    /// Belege mit einer Anhebung, die vom Fensterindex abhängt.
    fn belege_je_fenster(
        &self,
        link: &str,
        a: &Adresse,
        ab_nr: usize,
        anzahl: usize,
        ab_fenster: i64,
        anhebung: impl Fn(usize) -> f64,
        band: (usize, usize),
    ) {
        for i in 0..anzahl {
            let zeit = BASIS + (ab_fenster + i as i64) * FENSTER;
            self.c
                .p1(link, &evidenz(a, ab_nr + i, zeit, anhebung(i), band));
        }
    }

    /// Eine benannte Passage über den PRODUKTPFAD.
    fn passage(&self, von_offset: i64, bis_offset: i64, quellen: &[&Adresse]) {
        self.state_report();
        let mut wert = fixture("experiment_begin");
        wert["kopf"]["ziel"] = serde_json::to_value(&self.master).unwrap();
        wert["kopf"]["command_id"] = json!(hex(0x930));
        wert["kopf"]["base_revision"] = json!(0);
        wert["experiment_id"] = json!(hex(0x940));
        wert["passage"]["passage_id"] = json!(hex(0x5032));
        wert["passage"]["projekt_von"] = json!(BASIS + von_offset);
        wert["passage"]["projekt_bis"] = json!(BASIS + bis_offset);
        wert["passage"]["transport_epoch"] = grundform()["transport"]["transport_epoch"].clone();
        let mut ids: Vec<String> = vec![self.master.instance_id.clone()];
        ids.extend(quellen.iter().map(|a| a.instance_id.clone()));
        wert["passage"]["aktive_quellen"] = json!(ids);
        let antwort = Senke::p0(&*self.c, "main", &serde_json::to_vec(&wert).unwrap())
            .expect("experiment_begin wird beantwortet");
        let ack: Value = serde_json::from_slice(&antwort).unwrap();
        assert_eq!(ack["ergebnis"], "angewandt", "die Passage entsteht: {ack:?}");
    }

    fn befunde(&self) -> Vec<CauseHypothesis> {
        self.c
            .befunde_sicht(&self.master.project_binding_id, &self.master.session_epoch)
    }
}

fn stark(befunde: &[CauseHypothesis]) -> usize {
    befunde
        .iter()
        .filter(|b| b.confidence.klasse >= Sicherheitsklasse::Hoch)
        .count()
}

// ═════════════════════════════════════════════════════════════════════════
// N-15 · a1_intent_rolle_trennt_identische_kandidaten_nicht
// ═════════════════════════════════════════════════════════════════════════
//
// Codex-Befund A1: `rolle = fuehrt` auf dem Distraktor hebt seine
// Intent-Relevanz und damit seinen Gesamtrang. Die Fassung vor NAK-212
// erklärte die beiden damit für getrennt, und der Distraktor wurde `hoch` —
// eine musikalische Priorität erzeugte die nach M-21 verbotene starke
// Ursachenbehauptung, ohne dass eine einzige zusätzliche Messung sie trüge.
#[test]
fn a1_intent_rolle_trennt_identische_kandidaten_nicht() {
    let b = Buehne::schlank();
    let wahr = adresse(2);
    let distraktor = adresse(3);
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sonde0", &wahr, "passive_probe", Some(3));
    b.anmelden("sonde1", &distraktor, "passive_probe", Some(4));
    // Der User setzt die Rollen: der DISTRAKTOR führt, die wahre Quelle
    // begleitet nur.
    b.marke(
        1,
        json!([
            {"quelle_id": distraktor.instance_id, "rolle": "fuehrt",
             "revision": 1, "herkunft": "user", "konfidenz": 1.0},
            {"quelle_id": wahr.instance_id, "rolle": "begleitet",
             "revision": 1, "herkunft": "user", "konfidenz": 1.0}
        ]),
    );
    let muster = wechselnd(12);
    let band = (BAND_VON, BAND_BIS);
    b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
    b.belege_je_fenster("sonde0", &wahr, 100, 12, 0, &muster, band);
    b.belege_je_fenster("sonde1", &distraktor, 200, 12, 0, &muster, band);

    let befunde = b.befunde();
    assert_eq!(befunde.len(), 2, "beide bleiben sichtbar (M-26)");
    // Die Intent-Relevanz trennt die GESAMTRAENGE wirklich — sonst maesse der
    // Fall nichts.
    assert_ne!(
        rang_quantisiert(&befunde[0].rang),
        rang_quantisiert(&befunde[1].rang),
        "die Rollen trennen die Raenge: {:?} gegen {:?}",
        befunde[0].rang,
        befunde[1].rang
    );
    assert_eq!(
        befunde[0].rang.intent_relevanz, 1.0,
        "der Distraktor fuehrt und steht oben"
    );
    // Aber die Zusammenhangskomponenten sind gleich — also ungetrennt.
    assert_eq!(
        stark(&befunde),
        0,
        "eine musikalische Prioritaet ist keine Ursachenbehauptung: {:?}",
        befunde.iter().map(|b| b.confidence).collect::<Vec<_>>()
    );
    for b in &befunde {
        assert_ne!(b.zustand, Befundzustand::ReadyToSend);
        assert_eq!(b.alternatives.len(), 1, "jeder ist Alternative des anderen");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// N-42 · d2_fuehrender_wird_gegen_jeden_geprueft
// ═════════════════════════════════════════════════════════════════════════
//
// Matrixprüfung 1, Befund D2: bei DREI sichtbaren Kandidaten kann einer durch
// seine Intent-Relevanz zwischen zwei messtechnisch identische rutschen. Der
// Vergleich nur mit dem Zweitplatzierten liefert dann `getrennt = true`, und
// der Führende wird stark, obwohl der Dritte von ihm ununterscheidbar ist.
//
// ⚠️ Rolle `begleitet`, nicht `verschmolzen`: letztere löst ohne eingetragene
// `fuehrt_vor`-Beziehung `IntentVetoVerschmolzen` aus (`intent.rs`:175-187) —
// der dritte Kandidat fiele am Gate, und der Fall träfe den Defekt nicht mehr.
#[test]
fn d2_fuehrender_wird_gegen_jeden_geprueft() {
    let b = Buehne::schlank();
    let a = adresse(2); // Uplift belegt, rolle = fuehrt   -> Rang am hoechsten
    let m = adresse(3); // Uplift kleiner, rolle = traegt  -> Rang in der Mitte
    let c = adresse(4); // Uplift wie A,     rolle = begleitet -> Rang am kleinsten
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sonde0", &a, "passive_probe", Some(3));
    b.anmelden("sonde1", &m, "passive_probe", Some(4));
    b.anmelden("sonde2", &c, "passive_probe", Some(5));
    b.marke(
        1,
        json!([
            {"quelle_id": a.instance_id, "rolle": "fuehrt",
             "revision": 1, "herkunft": "user", "konfidenz": 1.0},
            {"quelle_id": m.instance_id, "rolle": "traegt",
             "revision": 1, "herkunft": "user", "konfidenz": 1.0},
            {"quelle_id": c.instance_id, "rolle": "begleitet",
             "revision": 1, "herkunft": "user", "konfidenz": 1.0}
        ]),
    );
    let band = (BAND_VON, BAND_BIS);
    let muster = wechselnd(12);
    b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
    b.belege_je_fenster("sonde0", &a, 100, 12, 0, &muster, band);
    // ⚠️ Der mittlere ist in ANDEREN Fenstern aktiv — nicht nur leiser.
    // Der bedingte Uplift ist auf die Spanne des MASTERS normiert, und die
    // Kandidatenamplitude geht nur ueber den Median-Split ein: eine halb so
    // laute Reihe mit demselben Muster ergibt exakt denselben Uplift. Nur ein
    // anderes AKTIVITAETSMUSTER teilt die Masterfenster anders auf.
    b.belege_je_fenster("sonde1", &m, 200, 12, 0, |i| if i % 3 == 0 { 9.0 } else { 0.0 }, band);
    b.belege_je_fenster("sonde2", &c, 300, 12, 0, &muster, band);

    let befunde = b.befunde();
    assert_eq!(befunde.len(), 3, "alle drei bleiben sichtbar");
    let von = |id: &Adresse| {
        befunde
            .iter()
            .find(|b| b.candidate_source == id.instance_id)
            .expect("jeder Kandidat hat einen Befund")
    };
    // Der Aufbau trifft den Defekt wirklich: A und C sind im Zusammenhang
    // identisch, der mittlere unterscheidet sich, und A fuehrt im Rang.
    assert_eq!(
        (von(&a).rang.uplift, von(&a).rang.wiederholbarkeit),
        (von(&c).rang.uplift, von(&c).rang.wiederholbarkeit),
        "A und C sind messtechnisch identisch"
    );
    assert_ne!(
        von(&a).rang.uplift,
        von(&m).rang.uplift,
        "der mittlere unterscheidet sich wirklich"
    );
    assert_eq!(
        befunde[0].candidate_source,
        a.instance_id,
        "A fuehrt: {:?}",
        befunde.iter().map(|b| b.rang.rang()).collect::<Vec<_>>()
    );
    assert_ne!(
        rang_quantisiert(&befunde[0].rang),
        rang_quantisiert(&befunde[1].rang),
        "A ist vom ZWEITEN getrennt — ohne D2 wuerde A damit stark"
    );
    // Und trotzdem: A ist von C nicht getrennt, also traegt niemand `hoch`.
    assert_eq!(
        stark(&befunde),
        0,
        "der Fuehrende wird gegen JEDEN geprueft: {:?}",
        befunde.iter().map(|b| b.confidence).collect::<Vec<_>>()
    );
}

// ═════════════════════════════════════════════════════════════════════════
// N-02 · eigen4_quelle_im_falschen_band
// ═════════════════════════════════════════════════════════════════════════
//
// Eine Quelle, die ihre Energie in einem ganz anderen Bandbereich trägt als
// der Masterbefund. `gate()` prüft Rücknahme, Capability, Coverage,
// Alignment, Passage und Intent — es prüft nie, ob der Kandidat im Befundband
// überhaupt Energie hat. Fünf von sechs Rangkomponenten sagten „kein
// Zusammenhang", und die Klasse sah keine davon.
#[test]
fn eigen4_quelle_im_falschen_band() {
    let b = Buehne::schlank();
    let sonde = adresse(2);
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
    b.marke(0, json!([]));
    let muster = wechselnd(12);
    b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, (BAND_VON, BAND_BIS));
    // Dieselbe Anhebung — aber siebzig Bandindizes weiter unten.
    b.belege_je_fenster("sonde0", &sonde, 100, 12, 0, &muster, (30, 34));

    let befunde = b.befunde();
    let befund = befunde.first().expect("ein Befund entsteht");
    assert_eq!(
        befund.candidate_source, sonde.instance_id,
        "der Kandidat ist im Rennen, das hier ist keine Enthaltung"
    );
    assert!(
        befund.rang.bandpassung < 0.05,
        "die Energie liegt woanders: {:?}",
        befund.rang
    );
    assert_eq!(
        stark(&befunde),
        0,
        "ohne Zusammenhang im Befundband keine starke Aussage: {:?}",
        (befund.confidence, befund.rang)
    );
}

// ═════════════════════════════════════════════════════════════════════════
// N-16 · f7_rangdifferenz_am_quantum
// ═════════════════════════════════════════════════════════════════════════
//
// Der schärfste Trennungsfall des G5-Laufs: beide Sonden tragen im Befundband
// exakt dieselbe Anhebung. Der Unterschied ist +0,1 dB in Band 210 — über
// hundert Bandindizes vom Befund entfernt und für die Behauptung ohne jede
// Bedeutung. Er senkte den Rang der zweiten um zwei Quanten, und das genügte.
#[test]
fn f7_rangdifferenz_am_quantum() {
    let b = Buehne::schlank();
    let eins = adresse(2);
    let zwei = adresse(3);
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sonde0", &eins, "passive_probe", Some(3));
    b.anmelden("sonde1", &zwei, "passive_probe", Some(4));
    b.marke(0, json!([]));
    let band = (BAND_VON, BAND_BIS);
    let muster = wechselnd(12);
    b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
    b.belege_je_fenster("sonde0", &eins, 100, 12, 0, &muster, band);
    // ⚠️ Die zweite Sonde traegt dieselben zwoelf Fenster mit derselben
    // Anhebung im Befundband — der Unterschied ist +0,1 dB in Band 210, ueber
    // hundert Bandindizes entfernt. Er steckt IN den bestehenden Belegen und
    // nicht in einem zusaetzlichen Fenster: ein dreizehntes Fenster aenderte
    // die Reihen und damit den Uplift, und der Fall maesse dann etwas anderes.
    for i in 0..12usize {
        b.c.p1(
            "sonde1",
            &evidenz_zwei_baender(
                &zwei,
                200 + i,
                BASIS + i as i64 * FENSTER,
                muster(i),
                band,
                Some((210, 211, 0.1)),
            ),
        );
    }

    let befunde = b.befunde();
    assert_eq!(befunde.len(), 2, "beide bleiben sichtbar");
    assert_ne!(
        rang_quantisiert(&befunde[0].rang),
        rang_quantisiert(&befunde[1].rang),
        "die Raenge unterscheiden sich wirklich — sonst misst der Fall nichts"
    );
    assert_eq!(
        stark(&befunde),
        0,
        "ein fremdes Band trennt keine Ursachen: {:?}",
        befunde.iter().map(|b| b.confidence).collect::<Vec<_>>()
    );
}

// ═════════════════════════════════════════════════════════════════════════
// N-18, N-19 · d4_umklammernde_und_disjunkte_spanne
// ═════════════════════════════════════════════════════════════════════════
//
// Der Befund E-D4 am Produktpfad: `spanne` ist ein lückenblindes min/max.
// Sondenfenster VOR und NACH den Masterfenstern umklammern deren Spanne
// vollständig; `zeitueberdeckung` meldete 1,0, das Gate hielt, und die Klasse
// wurde `hoch` — obwohl kein einziges Fenster ein gemeinsames Sample mit dem
// Master hat.
#[test]
fn d4_umklammernde_und_disjunkte_spanne() {
    for (name, ab_vorne, ab_hinten) in [
        ("umklammernd", Some(-40i64), Some(80i64)),
        ("disjunkt", Some(-80), None),
    ] {
        let b = Buehne::schlank();
        let sonde = adresse(2);
        b.anmelden("main", &b.master, "main", Some(0));
        b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
        b.marke(0, json!([]));
        let band = (BAND_VON, BAND_BIS);
        let muster = wechselnd(12);
        b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
        if let Some(ab) = ab_vorne {
            b.belege_je_fenster("sonde0", &sonde, 100, 6, ab, &muster, band);
        }
        if let Some(ab) = ab_hinten {
            b.belege_je_fenster("sonde0", &sonde, 200, 6, ab, &muster, band);
        }

        let befunde = b.befunde();
        let befund = befunde.first().expect("ein Ergebnis entsteht immer (M-27)");
        assert_eq!(
            befund.ursachenklasse,
            Ursachenklasse::DatenReichenNicht,
            "{name}: ohne gemeinsames Zeitfenster gibt es keine Ursache"
        );
        assert!(
            befund
                .ausschluesse
                .iter()
                .any(|a| a.grund == Ausschlussgrund::AlignmentFalsch),
            "{name}: der Kandidat verlaesst das Ranking MIT Grund (M-87): {:?}",
            befund.ausschluesse
        );
        assert_eq!(stark(&befunde), 0, "{name}");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// N-23 bis N-25, N-43 · f5_passagenrandwerte
// ═════════════════════════════════════════════════════════════════════════
//
// Die Passage über den echten Produktpfad, an ihren Rändern. Der Riegel stand
// im committeten Korpus nie unter Last (G-L1) — `sitzung_fahren` legte keine
// Passage an, und `gate()` Schritt 4 lief nie.
#[test]
fn f5_passagenrandwerte() {
    // (a) Zwölf Fensterlängen: alle zwölf Fenster liegen vollständig darin.
    // (b) Acht: der Randwert `GATE_MINDEST_FENSTER` — acht sind genug.
    // (c) Sieben: eines zu wenig.
    for (fenster, soll_stark) in [(12i64, true), (8, true), (7, false)] {
        let b = Buehne::mit_store(&format!("f5-{fenster}"));
        let sonde = adresse(2);
        b.anmelden("main", &b.master, "main", Some(0));
        b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
        b.marke(0, json!([]));
        b.passage(0, fenster * FENSTER, &[&sonde]);
        let band = (BAND_VON, BAND_BIS);
        let muster = wechselnd(12);
        b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
        b.belege_je_fenster("sonde0", &sonde, 100, 12, 0, &muster, band);

        let befunde = b.befunde();
        let befund = befunde.first().expect("ein Ergebnis entsteht immer");
        if soll_stark {
            assert_eq!(
                stark(&befunde),
                1,
                "Passage ueber {fenster} Fenster traegt die Aussage: {:?}",
                (befund.confidence, befund.rang)
            );
        } else {
            assert_eq!(
                stark(&befunde),
                0,
                "Passage ueber {fenster} Fenster ist zu kurz: {:?}",
                befund.confidence
            );
            assert!(
                befund
                    .ausschluesse
                    .iter()
                    .any(|a| a.grund == Ausschlussgrund::PassageZuKurz),
                "und der Kandidat faellt MIT Grund: {:?}",
                befund.ausschluesse
            );
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════
// N-29, N-43 · d3_passage_ohne_masterfenster_ergibt_enthaltung
// ═════════════════════════════════════════════════════════════════════════
//
// Matrixprüfung 1, Befund D3: liegt KEIN Masterfenster in der benannten
// Passage, gab `masteranomalie` in der Fassung Etappe 1 `None` zurück, und
// `hypothesen()` endete VOR den Gates — die Sitzung schwieg. M-27 verlangt
// aber ein Ergebnis. Mit dem Rückfall (jüngstes Fenster, UNGÜLTIGE
// Beobachtung) laufen die Gates, `fenster_in_passage(master)` ist 0, und jeder
// Kandidat fällt mit Grund.
#[test]
fn d3_passage_ohne_masterfenster_ergibt_enthaltung() {
    let b = Buehne::mit_store("d3-ohne-master");
    let sonde = adresse(2);
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
    b.marke(0, json!([]));
    // Die Passage liegt HINTER allen Belegen — genau die Lage aus §2.6 Nr. 17.
    b.passage(12 * FENSTER, 24 * FENSTER, &[&sonde]);
    let band = (BAND_VON, BAND_BIS);
    let muster = wechselnd(12);
    b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
    b.belege_je_fenster("sonde0", &sonde, 100, 12, 0, &muster, band);

    let befunde = b.befunde();
    let befund = befunde
        .first()
        .expect("die Sitzung SCHWEIGT NICHT — M-27 verlangt ein Ergebnis");
    assert_eq!(
        befund.ursachenklasse,
        Ursachenklasse::DatenReichenNicht,
        "das Ergebnis ist die Enthaltung"
    );
    assert_eq!(befund.confidence.klasse, Sicherheitsklasse::Unklar);
    assert!(
        !befund.beobachtung.gueltig,
        "die Beobachtung stammt von ausserhalb der Passage und ist deshalb \
         keine Messung ueber sie"
    );
    assert!(
        befund.ausschluesse.iter().any(|a| matches!(
            a.grund,
            Ausschlussgrund::PassageUnvergleichbar | Ausschlussgrund::PassageZuKurz
        )),
        "und jeder Kandidat faellt MIT Grund (M-87): {:?}",
        befund.ausschluesse
    );
}
