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
//! Seit der **Nacharbeit 1** (07.09.2026) faehrt es ausserdem **alle 15
//! Ketteneingaben** aus §2.6 Zeile 13 bis 27 des Manifests durch den
//! Produktpfad und **druckt** je Eingabe ihr Ergebnis. Sechs davon standen
//! schon hier; neun kamen dazu (`[0,11)`, `f5b`, `f8`, `f8b`, `f4` mit
//! beiden Staenden, `f4b`, `eigen3`, `eigen5`) und mit ihnen der
//! Produktpfadfall `a4`. Faelle, die **NAK-213** oder **NAK-214** gehoeren,
//! werden gefahren und mit **benannter, gedruckter Luecke** ausgegeben
//! (Muster NAK-190) — nicht geloest, nicht als gruen gewertet und nicht
//! ausgelassen (R6).
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
        self.passage_versuch(0, 0x5032, von_offset, bis_offset, quellen);
    }

    /// Ein `experiment_begin` mit EIGENER Kennung — für `f5b`.
    ///
    /// `begin_anwenden_locked` legt eine bekannte Passage nicht neu an: „Sie
    /// wird nur beim ERSTEN Versuch angelegt, der sie nennt." Trägt der
    /// zweite Versuch dieselbe `passage_id` mit anderem Zeitfenster, misst
    /// der Broker weiter gegen das alte — genau das prüft `f5b`. Der Zähler
    /// `nr` trennt Befehl und Experiment, `passage_id` bleibt frei wählbar.
    fn passage_versuch(
        &self,
        nr: usize,
        passage_id: usize,
        von_offset: i64,
        bis_offset: i64,
        quellen: &[&Adresse],
    ) -> String {
        let mut wert = fixture("experiment_begin");
        wert["kopf"]["ziel"] = serde_json::to_value(&self.master).unwrap();
        wert["kopf"]["command_id"] = json!(hex(0x930 + nr));
        // ⚠ `experiment_begin` schreibt keinen DSP-State und hebt die
        // `state_revision` deshalb NICHT. Beide Versuche nennen dieselbe
        // Basisrevision; eine hochgezaehlte faellt mit `revision_conflict`.
        wert["kopf"]["base_revision"] = json!(0);
        wert["experiment_id"] = json!(hex(0x940 + nr));
        wert["passage"]["passage_id"] = json!(hex(passage_id));
        wert["passage"]["projekt_von"] = json!(BASIS + von_offset);
        wert["passage"]["projekt_bis"] = json!(BASIS + bis_offset);
        wert["passage"]["transport_epoch"] = grundform()["transport"]["transport_epoch"].clone();
        let mut ids: Vec<String> = vec![self.master.instance_id.clone()];
        ids.extend(quellen.iter().map(|a| a.instance_id.clone()));
        // 🔑 Die Messpunktklassen reisen PARALLEL zu `aktive_quellen` und in
        // DERSELBEN Reihenfolge; `passage_aus_wert` weist eine andere Laenge
        // fail-closed ab (Befund R23). Bis NAK-213 stand die Liste fest bei
        // zwei Eintraegen aus der Fixture — jede Passage mit mehr als einer
        // Sonde fiel deshalb mit `schema_violation`, ohne dass der Aufbau
        // etwas Falsches getan haette.
        let mut klassen: Vec<String> = vec!["insert".into()];
        klassen.extend(quellen.iter().map(|_| "post".to_string()));
        wert["passage"]["aktive_quellen"] = json!(ids);
        wert["passage"]["messpunktklassen"] = json!(klassen);
        let antwort = Senke::p0(&*self.c, "main", &serde_json::to_vec(&wert).unwrap())
            .expect("experiment_begin wird beantwortet");
        let ack: Value = serde_json::from_slice(&antwort).unwrap();
        assert_eq!(ack["ergebnis"], "angewandt", "die Passage entsteht: {ack:?}");
        hex(passage_id)
    }

    fn befunde(&self) -> Vec<CauseHypothesis> {
        self.c
            .befunde_sicht(&self.master.project_binding_id, &self.master.session_epoch)
    }

    /// Der Sitzungs-Snapshot — die Sicht, die Gen wirklich bekommt.
    fn snapshot(&self) -> Value {
        serde_json::from_slice(
            &self
                .c
                .session_snapshot_json(&self.master.project_binding_id, &self.master.session_epoch),
        )
        .expect("der Snapshot ist JSON")
    }

    /// Denselben Deskriptor noch einmal setzen — mit anderem Mixerkanal.
    ///
    /// Das ist der Kern von `f4`: der Kanal lebt im Deskriptor, nicht am
    /// Beleg. Ein Wechsel ändert die Duplikaterkennung rückwirkend für ALLE
    /// Fenster, ohne dass ein einziger Messwert neu entstanden wäre.
    fn deskriptor(&self, link: &str, a: &Adresse, mixer: Option<i64>) {
        let mut d = json!({
            "adresse": a,
            "plugin_kind": "passive_probe",
            "measurement_position": "post",
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
}

/// Was der Produktpfad auf eine Ketteneingabe wirklich geantwortet hat.
///
/// R6 verlangt, dass **jede** der 15 Ketteneingaben ihr Ergebnis ausgibt —
/// ein Riegel, der nur `assert` sagt, zeigt dem Leser nicht, was gemessen
/// wurde. Die Form folgt dem G5-Gegenbeispiellauf
/// (`docs/beweise/roh/G5-gegenbeispiele-bf92891.md`).
fn protokoll(titel: &str, befunde: &[CauseHypothesis]) {
    println!("── {titel} ──");
    if befunde.is_empty() {
        println!("   (KEIN BEFUND — der Produktpfad schweigt)");
    }
    for b in befunde {
        let r = b.rang;
        println!(
            "   quelle=..{} klasse={:?} zustand={:?} ursache={:?} score={:.6}",
            &b.candidate_source[30..],
            b.confidence.klasse,
            b.zustand,
            b.ursachenklasse,
            b.confidence.score
        );
        println!(
            "     rang: bandpassung={:.6} koinzidenz={:.6} uplift={:.6} \
             intent={:.6} wiederholbarkeit={:.6} routing={:.6}",
            r.bandpassung,
            r.koinzidenz,
            r.uplift,
            r.intent_relevanz,
            r.wiederholbarkeit,
            r.routingqualitaet
        );
        println!(
            "     band={:?} passage={:?} alternatives={} ausschluesse={:?}",
            b.band_hz,
            b.passage_id.as_deref().map(|p| p[24..].to_string()),
            b.alternatives.len(),
            b.ausschluesse
                .iter()
                .map(|a| (a.candidate_source[30..].to_string(), a.grund.wire()))
                .collect::<Vec<_>>()
        );
    }
    println!("   => starke Befunde: {}", stark(befunde));
}

/// Eine **benannte, gedruckte Lücke** (Muster NAK-190, R6).
///
/// Der Fall wird gefahren und sein Ergebnis ausgegeben, aber ein anderes
/// Ticket schließt ihn. Er wird hier weder gelöst noch als grün gewertet —
/// er ist benannt. Dasselbe Muster führt `pruefe_p5_korpus.py::offene_luecken`
/// für die Korpussitzungen.
fn luecke(kennung: &str, ticket: &str, zusage: &str, heute: &str) {
    println!("   ⚠ OFFENE LUECKE {kennung} [{ticket}]");
    println!("     zusage:          {zusage}");
    println!("     was heute passiert: {heute}");
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
    protokoll("A1 rolle=fuehrt auf dem Distraktor, sonst identisch", &befunde);
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
    //
    // 🔑 **NAK-212 Nacharbeit 1, NR-02.** Das Muster lautete `i % 3 == 0` —
    // VIER laute gegen ACHT leise Fenster. Der Median von zwoelf Werten ist
    // das Mittel der beiden mittleren, hier also der LEISE Wert; mit
    // `k >= schwelle` landen dann ALLE Fenster in `mit`, `ohne` bleibt leer,
    // und M-19 liefert weder Uplift noch Wiederholbarkeit. `m` rangierte
    // damit HINTER C — der Aufbau baute A > C > M statt der zugesagten
    // Reihenfolge A > M > C, und schon der Vergleich mit dem Zweiten C
    // erzwang die Enthaltung. Jetzt ist `m` in SIEBEN Fenstern laut (allen
    // sechs lauten des Masters plus dem ersten leisen): der Median liegt auf
    // dem lauten Wert, der Split ist 7 zu 5, und sein Uplift ist positiv,
    // aber kleiner als der von A und C — ein EIGENER Zusammenhang, der ihn
    // zwischen die beiden setzt.
    b.belege_je_fenster(
        "sonde1",
        &m,
        200,
        12,
        0,
        |i| if i % 2 == 1 || i == 0 { 9.0 } else { 0.0 },
        band,
    );
    b.belege_je_fenster("sonde2", &c, 300, 12, 0, &muster, band);

    let befunde = b.befunde();
    protokoll("D2 drei Kandidaten: A und C identisch, M dazwischen", &befunde);
    assert_eq!(befunde.len(), 3, "alle drei bleiben sichtbar");
    let von = |id: &Adresse| {
        befunde
            .iter()
            .find(|b| b.candidate_source == id.instance_id)
            .expect("jeder Kandidat hat einen Befund")
    };
    // ── Vorbedingungen, alle gemessen (NAK-212 Nacharbeit 1, NR-02) ───
    // (a) A und C sind im Zusammenhang identisch.
    assert_eq!(
        (von(&a).rang.uplift, von(&a).rang.wiederholbarkeit),
        (von(&c).rang.uplift, von(&c).rang.wiederholbarkeit),
        "A und C sind messtechnisch identisch"
    );
    // (b) Der mittlere traegt einen EIGENEN, belegten Zusammenhang: sein
    //     Uplift ist positiv und von dem der beiden anderen verschieden.
    //     Ohne diesen Riegel maesse der Fall nur eine Quelle, die an M-19
    //     scheitert — und der Vergleich mit dem Zweitplatzierten erzwaenge
    //     die Enthaltung schon allein.
    assert!(
        von(&m).rang.uplift > 0.0,
        "der mittlere hat einen belegten Uplift: {}",
        von(&m).rang.uplift
    );
    assert_ne!(
        von(&a).rang.uplift,
        von(&m).rang.uplift,
        "und er unterscheidet sich wirklich von A"
    );
    // (c) Die zugesagte Reihenfolge ist A > M > C, nicht A > C > M. Nur so
    //     steht der ununterscheidbare C wirklich HINTER dem Zweiten, und die
    //     Enthaltung folgt nicht schon aus dem Vergleich mit ihm.
    let reihenfolge: Vec<&str> = befunde
        .iter()
        .map(|b| b.candidate_source.as_str())
        .collect();
    assert_eq!(
        reihenfolge,
        vec![
            a.instance_id.as_str(),
            m.instance_id.as_str(),
            c.instance_id.as_str()
        ],
        "A > M > C: {:?}",
        befunde.iter().map(|b| b.rang.rang()).collect::<Vec<_>>()
    );
    // (d) A ist vom Zweiten nach BEIDEN Bedingungen aus E6 getrennt: der
    //     Rang unterscheidet sich UND der Zusammenhang. Ohne D2 waere A
    //     damit stark.
    assert_ne!(
        rang_quantisiert(&befunde[0].rang),
        rang_quantisiert(&befunde[1].rang),
        "A ist vom ZWEITEN (M) im Rang getrennt"
    );
    // Und trotzdem: A ist von C nicht getrennt, also traegt niemand `hoch`.
    assert_eq!(
        stark(&befunde),
        0,
        "der Fuehrende wird gegen JEDEN geprueft: {:?}",
        befunde.iter().map(|b| b.confidence).collect::<Vec<_>>()
    );

    // ── Gegenprobe: DIESELBE Buehne ohne C ────────────────────────
    // Ohne den dritten Kandidaten ist A von jedem Ueberlebenden getrennt und
    // wird stark. Ohne diese Haelfte waere die Zusage nicht von „niemand ist
    // hier je stark" zu unterscheiden.
    let o = Buehne::schlank();
    o.anmelden("main", &o.master, "main", Some(0));
    o.anmelden("sonde0", &a, "passive_probe", Some(3));
    o.anmelden("sonde1", &m, "passive_probe", Some(4));
    o.marke(
        1,
        json!([
            {"quelle_id": a.instance_id, "rolle": "fuehrt",
             "revision": 1, "herkunft": "user", "konfidenz": 1.0},
            {"quelle_id": m.instance_id, "rolle": "traegt",
             "revision": 1, "herkunft": "user", "konfidenz": 1.0}
        ]),
    );
    o.belege_je_fenster("main", &o.master, 0, 12, 0, &muster, band);
    o.belege_je_fenster("sonde0", &a, 100, 12, 0, &muster, band);
    o.belege_je_fenster(
        "sonde1",
        &m,
        200,
        12,
        0,
        |i| if i % 2 == 1 || i == 0 { 9.0 } else { 0.0 },
        band,
    );
    let ohne_c = o.befunde();
    assert_eq!(ohne_c.len(), 2, "ohne C bleiben zwei sichtbar");
    assert_eq!(
        ohne_c[0].candidate_source, a.instance_id,
        "A fuehrt auch hier"
    );
    assert_eq!(
        stark(&ohne_c),
        1,
        "ohne den ununterscheidbaren Dritten ist A stark: {:?}",
        ohne_c.iter().map(|b| b.confidence).collect::<Vec<_>>()
    );
    assert_eq!(ohne_c[0].confidence.klasse, Sicherheitsklasse::Hoch);
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
    protokoll("EIGEN-4 Sonde hebt Band 30..34, Masterbefund liegt bei 98..102", &befunde);
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
    protokoll("F7 kleinste darstellbare Rangstoerung (0,1 dB in Band 210)", &befunde);
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
        protokoll(&format!("D4 {name}"), &befunde);
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
    // (b) Elf: GENAU EIN Fenster liegt außerhalb — elf sind immer noch genug.
    // (c) Acht: der Randwert `GATE_MINDEST_FENSTER` — acht sind genug.
    // (d) Sieben: eines zu wenig.
    //
    // 🔑 **NAK-212 Nacharbeit 1, NR-03:** die Eingabe `[0,11)` gehört zu den
    // 15 vereinbarten Ketteneingaben (§2.6 Zeile 14) und fehlte.
    for (fenster, soll_stark) in [(12i64, true), (11, true), (8, true), (7, false)] {
        let b = Buehne::mit_store(&format!("f5-{fenster}"));
        let sonde = adresse(2);
        b.anmelden("main", &b.master, "main", Some(0));
        b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
        b.marke(0, json!([]));
        b.passage(0, fenster * FENSTER, &[&sonde]);
        let band = (BAND_VON, BAND_BIS);
        // ⚠ Das Muster richtet sich nach der PASSAGE, nicht nach der
        // Fensterzahl: seit E5 liest `masteranomalie` das letzte Fenster
        // INNERHALB der Passage. Bei `[0,11)` ist das Fenster 10 — mit
        // `wechselnd(12)` waere es leise, die Anomalie laege in einer
        // fremden Bandgruppe (gemessen: `Bandintervall { von: 0, bis: 4 }`),
        // und der Fall maesse nicht mehr die Passagenlaenge. `wechselnd(n)`
        // macht genau das Fenster `n - 1` laut.
        let muster = wechselnd(fenster as usize);
        b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
        b.belege_je_fenster("sonde0", &sonde, 100, 12, 0, &muster, band);

        let befunde = b.befunde();
        protokoll(&format!("F5 Passage [0,{fenster}) Fensterlaengen"), &befunde);
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
    protokoll("F5 Passage [12,24) — alle Belege liegen DAVOR", &befunde);
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

// ═════════════════════════════════════════════════════════════════════════
// §2.6 Nr. 18 · f5b_gleiche_passage_id_verschobenes_fenster
// ═════════════════════════════════════════════════════════════════════════
//
// `begin_anwenden_locked` legt eine bekannte Passage NICHT neu an: „Sie wird
// nur beim ERSTEN Versuch angelegt, der sie nennt; ein zweiter Versuch
// derselben Passage schreibt sie nicht um." Trägt der zweite Versuch
// dieselbe `passage_id` mit einem ANDEREN Zeitfenster, misst der Broker
// weiter gegen das alte — und die Belege, die im zweiten Fenster liegen,
// verfehlen die gemerkte Passage. Das Ergebnis ist die Enthaltung mit Grund,
// nicht eine Aussage über das falsche Fenster: **fail-closed**.
#[test]
fn f5b_gleiche_passage_id_verschobenes_fenster() {
    let b = Buehne::mit_store("f5b");
    let sonde = adresse(2);
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
    b.marke(0, json!([]));
    b.state_report();
    // Erst die Passage [0,12) Fensterlängen …
    let erste = b.passage_versuch(0, 0x5032, 0, 12 * FENSTER, &[&sonde]);
    // … dann DIESELBE Kennung mit dem Fenster [12,24).
    let zweite = b.passage_versuch(1, 0x5032, 12 * FENSTER, 24 * FENSTER, &[&sonde]);
    assert_eq!(erste, zweite, "beide Versuche nennen dieselbe passage_id");

    // Die Belege liegen im ZWEITEN, verschobenen Fenster.
    let band = (BAND_VON, BAND_BIS);
    let muster = wechselnd(12);
    b.belege_je_fenster("main", &b.master, 0, 12, 12, &muster, band);
    b.belege_je_fenster("sonde0", &sonde, 100, 12, 12, &muster, band);

    let befunde = b.befunde();
    protokoll("F5b gleiche passage_id, Fenster verschoben [0,12) -> [12,24)", &befunde);
    let befund = befunde.first().expect("ein Ergebnis entsteht immer (M-27)");
    assert_eq!(
        befund.ursachenklasse,
        Ursachenklasse::DatenReichenNicht,
        "fail-closed: gegen die GEMERKTE Passage misst kein Beleg"
    );
    assert!(
        befund
            .ausschluesse
            .iter()
            .any(|a| a.grund == Ausschlussgrund::PassageUnvergleichbar),
        "der Kandidat faellt MIT Grund (M-87): {:?}",
        befund.ausschluesse
    );
    assert_eq!(stark(&befunde), 0);
}

// ═════════════════════════════════════════════════════════════════════════
// §2.6 Nr. 19 · f8_zwei_master_mit_widersprechendem_befundband
// ═════════════════════════════════════════════════════════════════════════
//
// `aufnahmen_sammeln` schreibt `master = Some(profil)` ohne Prüfung, ob
// schon einer da ist. Die Schlüssel sind nach `instance_id` sortiert — der
// LETZTE `main` gewinnt, der erste verschwindet ganz.
//
// **Was NAK-212 hier schließt:** die Sonde trägt im Band des gewinnenden
// Masters kaum Energie und bei konstantem Pegel keinen messbaren Uplift; R1
// senkt sie auf `mittel`. Die falsche STARKE Behauptung ist damit weg.
// **Was offen bleibt:** der erste Master verschwindet weiterhin lautlos.
#[test]
fn f8_zwei_master_mit_widersprechendem_befundband() {
    let b = Buehne::schlank();
    let master_a = adresse(1);
    let master_b = adresse(9);
    let sonde = adresse(2);
    b.anmelden("main", &master_a, "main", Some(0));
    b.anmelden("main2", &master_b, "main", Some(1));
    b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
    b.marke(0, json!([]));
    // Master A hat seine Anomalie im Band der Sonde …
    b.belege_je_fenster("main", &master_a, 0, 12, 0, |_| 9.0, (BAND_VON, BAND_BIS));
    // … Master B WOANDERS.
    b.belege_je_fenster("main2", &master_b, 200, 12, 0, |_| 9.0, (150, 154));
    // Die Sonde drueckt ausschliesslich in 98..102.
    b.belege_je_fenster("sonde0", &sonde, 100, 12, 0, |_| 9.0, (BAND_VON, BAND_BIS));

    let befunde = b.befunde();
    protokoll("F8 zwei Master (A: Band 98..102, B: Band 150..154)", &befunde);

    // 🔑 **NAK-213 R4 (K-31).** Bis hierher nahm `aufnahmen_sammeln` den
    // letzten `main` nach `instance_id`: Master A verschwand lautlos, und die
    // Sonde trug einen Befund im Band des ANDEREN Masters. Jetzt entsteht
    // eine ENTHALTUNG OHNE ORT — kein Befund ueber eine Quelle, kein stiller
    // Gewinner.
    assert_eq!(befunde.len(), 1, "genau ein Ergebnis: die Enthaltung");
    let enthaltung = &befunde[0];
    assert_eq!(
        enthaltung.ursachenklasse,
        Ursachenklasse::DatenReichenNicht,
        "die siebte Ursachenklasse ist ein regulaeres Ergebnis (M-27)"
    );
    assert_eq!(enthaltung.confidence.klasse, Sicherheitsklasse::Unklar);
    assert_eq!(
        (enthaltung.band_hz.von, enthaltung.band_hz.bis),
        (0, 221),
        "das VOLLE Gitter heisst „nicht eingegrenzt“, nicht „ueberall“"
    );
    assert!(
        !enthaltung.beobachtung.gueltig && enthaltung.beobachtung.wert_db == 0.0,
        "ohne das Bit ist die Zahl keine Messung: {:?}",
        enthaltung.beobachtung
    );
    assert_eq!(
        enthaltung.candidate_source,
        std::cmp::min(master_a.instance_id.clone(), master_b.instance_id.clone()),
        "die lexikographisch kleinste `instance_id` der `main`-Clients — sie \
         behauptet KEINE Fuehrung, sie ist die deterministische Adresse"
    );
    assert!(
        !befunde
            .iter()
            .any(|f| f.candidate_source == sonde.instance_id),
        "kein Befund ueber die Sonde: es hat keine Rechnung stattgefunden"
    );
    assert_eq!(
        stark(&befunde),
        0,
        "und erst recht keine starke Aussage"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// §2.6 Nr. 20 · f8b_master_ohne_fenster
// ═════════════════════════════════════════════════════════════════════════
//
// Der Master ist angemeldet, hat aber keinen einzigen Beleg gesendet.
// `masteranomalie` gibt `None`, und `hypothesen()` endet vor den Gates: die
// Sitzung SCHWEIGT. Das ist G-L4 und gehört **NAK-213 R4** — dieses Ticket
// ändert daran nichts, und der Fall steht hier, damit das Schweigen benannt
// ist statt unbemerkt.
#[test]
fn f8b_master_ohne_fenster() {
    let b = Buehne::schlank();
    let sonde = adresse(2);
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
    b.marke(0, json!([]));
    b.belege_je_fenster("sonde0", &sonde, 100, 12, 0, wechselnd(12), (BAND_VON, BAND_BIS));

    let befunde = b.befunde();
    protokoll("F8b Master angemeldet, aber ohne Beleg", &befunde);

    // 🔑 **NAK-213 R4 (K-35).** Bis hierher gab `masteranomalie` `None`,
    // `hypothesen()` endete vor den Gates, und im Snapshot standen NULL
    // findings — die Sitzung schwieg. M-27 verlangt ein Ergebnis.
    assert_eq!(befunde.len(), 1, "eine Enthaltung statt Schweigen");
    let enthaltung = &befunde[0];
    assert_eq!(enthaltung.ursachenklasse, Ursachenklasse::DatenReichenNicht);
    assert_eq!(enthaltung.confidence.klasse, Sicherheitsklasse::Unklar);
    assert_eq!(enthaltung.candidate_source, b.master.instance_id);
    assert!(
        !enthaltung.evidence_ids.is_empty(),
        "R8: die Pflichtliste traegt die Belege der SONDE — das Exit-Gate \
         verlangt EXISTENTE IDs, nicht IDs des Masters"
    );
    assert!(
        enthaltung.evidence_ids.len() <= 32,
        "und sie bleibt unter dem Vertragsdeckel: {}",
        enthaltung.evidence_ids.len()
    );
}

// ═════════════════════════════════════════════════════════════════════════
// §2.6 Nr. 21 und 22 · f4_kanalwechsel_zwischen_den_fenstern
// ═════════════════════════════════════════════════════════════════════════
//
// Der Mixerkanal lebt im DESKRIPTOR, nicht am Beleg. `aufnahmen_sammeln`
// liest ihn zum Zeitpunkt der Rechnung — die Duplikaterkennung kennt deshalb
// nur den LETZTEN Stand, nie die Kanalgeschichte der Fenster.
//
// **Was NAK-212 hier schließt:** bei konstantem Pegel ist kein Uplift
// messbar, also bleibt es nach R1 in BEIDEN Ständen bei `mittel` — die
// starke Aussage nach dem Wechsel ist weg. **Was offen bleibt:** die
// Deskriptoränderung hebt die `routingqualitaet` rückwirkend, ohne dass eine
// einzige neue MESSUNG dazugekommen wäre.
#[test]
fn f4_kanalwechsel_zwischen_den_fenstern() {
    let b = Buehne::schlank();
    let a = adresse(2);
    let c = adresse(3);
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sondeA", &a, "passive_probe", Some(7));
    b.anmelden("sondeC", &c, "passive_probe", Some(7));
    b.marke(0, json!([]));
    let band = (BAND_VON, BAND_BIS);
    b.belege_je_fenster("main", &b.master, 0, 12, 0, |_| 9.0, band);
    // A fuehrt um 0,1 dB — sonst traennte sie nichts, und der Fall maesse nur
    // den Gleichstandsriegel.
    b.belege_je_fenster("sondeA", &a, 100, 12, 0, |_| 9.1, band);
    b.belege_je_fenster("sondeC", &c, 200, 12, 0, |_| 9.0, band);

    let vorher = b.befunde();
    protokoll("F4 beide Sonden auf Mixerkanal 7", &vorher);
    let routing_vorher: Vec<f64> = vorher.iter().map(|f| f.rang.routingqualitaet).collect();
    assert_eq!(vorher.len(), 2, "beide bleiben sichtbar");
    assert_eq!(stark(&vorher), 0, "geteilter Kanal: keine starke Aussage");

    // Jetzt wechselt C den Kanal — ohne einen einzigen neuen MESSWERT.
    b.deskriptor("sondeC", &c, Some(9));
    // Ein weiterer Beleg loest die Rechnung aus. ⚠ Der Master bekommt sein
    // dreizehntes Fenster ZEITGLEICH: seit das Alignment paarweise misst,
    // haette ein Kandidatenfenster ohne Masterpartner den Anteil auf 12/13
    // gedrueckt, und beide fielen mit `alignment_falsch` — der Fall maesse
    // dann den Kanalwechsel gar nicht mehr.
    b.belege_je_fenster("main", &b.master, 300, 1, 12, |_| 9.0, band);
    b.belege_je_fenster("sondeA", &a, 130, 1, 12, |_| 9.1, band);
    b.belege_je_fenster("sondeC", &c, 230, 1, 12, |_| 9.0, band);

    let nachher = b.befunde();
    protokoll("F4 nach dem Kanalwechsel von C (7 -> 9), Messwerte unveraendert", &nachher);
    assert_eq!(nachher.len(), 2, "beide bleiben sichtbar");
    assert_eq!(
        stark(&nachher),
        0,
        "auch nach dem Wechsel traegt niemand `hoch` — bei konstantem Pegel \
         ist kein Uplift messbar (R1): {:?}",
        nachher.iter().map(|f| f.confidence).collect::<Vec<_>>()
    );
    // Die Luecke wird GEMESSEN, nicht behauptet: die Routingqualitaet STEIGT
    // wirklich, und zwar bei BEIDEN — der geteilte Kanal war die
    // Duplikatmarke, und ohne ihn faellt sie fuer beide weg.
    let routing_nachher: Vec<f64> = nachher.iter().map(|f| f.rang.routingqualitaet).collect();
    assert!(
        routing_nachher
            .iter()
            .zip(routing_vorher.iter())
            .all(|(n, v)| n > v),
        "der Kanalwechsel hebt die Routingqualitaet ohne neue Messung: \
         {routing_vorher:?} -> {routing_nachher:?}"
    );
    luecke(
        "f4_kanalwechsel",
        "NAK-213 R5",
        "Eine Deskriptoraenderung ohne neue Messung darf keine Rangkomponente heben.",
        &format!(
            "`routingqualitaet` steigt allein durch den Kanalwechsel: {routing_vorher:?} \
             -> {routing_nachher:?}, ohne dass ein Fenster neu gemessen worden waere."
        ),
    );
}

// ═════════════════════════════════════════════════════════════════════════
// §2.6 Nr. 23 · f4b_drei_sonden_auf_einem_kanal
// ═════════════════════════════════════════════════════════════════════════
//
// Die „Elternbeziehung" des Produkts ist eine flache Menge geteilter
// Mixerkanäle: jede Quelle zeigt auf EINE andere desselben Kanals. Eine
// zweite Ebene hat im Datenmodell keine Darstellung — ein Deskriptor trägt
// genau einen Kanal. Gemessen wird, ob die flache Menge wenigstens transitiv
// hält: alle drei sind einander Duplikat, keiner trägt eine starke Aussage.
#[test]
fn f4b_drei_sonden_auf_einem_kanal() {
    let b = Buehne::schlank();
    let (a, c, d) = (adresse(2), adresse(3), adresse(4));
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sondeA", &a, "passive_probe", Some(7));
    b.anmelden("sondeC", &c, "passive_probe", Some(7));
    b.anmelden("sondeD", &d, "passive_probe", Some(7));
    b.marke(0, json!([]));
    let band = (BAND_VON, BAND_BIS);
    b.belege_je_fenster("main", &b.master, 0, 12, 0, |_| 9.0, band);
    b.belege_je_fenster("sondeA", &a, 100, 12, 0, |_| 9.2, band);
    b.belege_je_fenster("sondeC", &c, 200, 12, 0, |_| 9.1, band);
    b.belege_je_fenster("sondeD", &d, 300, 12, 0, |_| 9.0, band);

    let befunde = b.befunde();
    protokoll("F4b drei Sonden auf Mixerkanal 7", &befunde);
    assert_eq!(befunde.len(), 3, "alle drei bleiben sichtbar");
    assert_eq!(
        stark(&befunde),
        0,
        "drei Quellen auf EINEM Kanal behaupten nicht dreimal dieselbe Ursache"
    );
    let routing: Vec<f64> = befunde.iter().map(|f| f.rang.routingqualitaet).collect();
    for f in &befunde {
        assert!(
            f.rang.routingqualitaet <= 0.5,
            "jede traegt die Duplikatmarke: {}",
            f.rang.routingqualitaet
        );
    }
    // 🔑 Nacharbeit 2 (08.09.2026, WN-02): die AUSGABE ist nach §2.6 Nr. 23
    // korrekt und bleibt unverändert — die Erwartung `mittel` wird hier NICHT
    // angefasst. Was fehlte, ist die benannte Lücke: R6/NR-03 nennt `f4b`
    // ausdrücklich unter den Fällen, die mit gedruckter Lücke ausgegeben
    // werden, weil **NAK-213 R3** die Parent-Erkennung neu regelt. Ohne diese
    // Zeile las der Beleg wie ein rundum geschlossener Fall.
    luecke(
        "f4b_drei_sonden_auf_einem_kanal",
        "NAK-213 R3",
        "Parent-Duplikate werden ueber ALLE Quellen der Sitzung erkannt - \
         einschliesslich Master, zurueckgenommener und stummer Quellen (M-22).",
        &format!(
            "Die Ausgabe ist korrekt: {} Befunde, davon {} starke, jeder mit der \
             Duplikatmarke {routing:?}. Enger als die Zusage ist der ERKENNUNGSSATZ: \
             die Kanalkarte entsteht ueber `stand.evidenz`, eine angemeldete Quelle \
             ohne einen einzigen Beleg betritt sie also nie; und `parent` wird nur an \
             Kandidaten geschrieben, waehrend auch die Suche nach dem Kind nur ueber \
             die Kandidaten laeuft - der Master ist eines der {} Mitglieder dieser \
             Sitzung und traegt trotzdem nie eine Duplikatmarke.",
            befunde.len(),
            stark(&befunde),
            b.snapshot()["mitglieder"].as_array().map(Vec::len).unwrap_or(0),
        ),
    );
}

// ═════════════════════════════════════════════════════════════════════════
// N-08, N-10, §2.6 Nr. 24 · eigen3_antikorrelierte_quelle
// ═════════════════════════════════════════════════════════════════════════
//
// Der schärfste Fall, den die Klassenwahl vor NAK-212 zuließ: die Quelle ist
// im Befundband GENAU DANN laut, wenn der Master dort leise ist. Der
// bedingte Uplift ist negativ, die Onset-Korrelation ebenfalls — und beide
// wurden auf 0 geklemmt. Schlimmer noch: `bootstrap_p` war zweiseitig und
// gab der stabil NEGATIVEN Reihe `wiederholbarkeit` = 0,995, den Rang
// 0,4358 und die Klasse `hoch`.
#[test]
fn eigen3_antikorrelierte_quelle() {
    let b = Buehne::schlank();
    let sonde = adresse(2);
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
    b.marke(0, json!([]));
    let band = (BAND_VON, BAND_BIS);
    // Der Master braucht eine Reihe, die in BEIDEN Mengen misst — sonst
    // waere seine Spanne null und der Uplift nicht normierbar.
    b.belege_je_fenster("main", &b.master, 0, 12, 0,
                        |i| if i % 2 == 1 { 12.0 } else { 6.0 }, band);
    // Die Sonde: genau umgekehrt.
    b.belege_je_fenster("sonde0", &sonde, 100, 12, 0,
                        |i| if i % 2 == 1 { 6.0 } else { 12.0 }, band);

    let befunde = b.befunde();
    protokoll("EIGEN-3 antikorrelierte Quelle (laut, wenn der Master leise ist)", &befunde);
    let befund = befunde.first().expect("ein Befund entsteht");
    assert_eq!(
        befund.candidate_source, sonde.instance_id,
        "der Kandidat ist im Rennen — das hier ist keine Enthaltung"
    );
    assert_eq!(stark(&befunde), 0, "ein Gegenbeleg schliesst `hoch` aus (R2)");
    assert_eq!(
        befund.rang.wiederholbarkeit, 0.0,
        "die EINSEITIGE Stabilitaet belohnt den Gegenbeleg nicht mehr (war 0,995)"
    );
    assert_eq!(befund.rang.uplift, 0.0, "und der negative Uplift traegt keinen Rang");
}

// ═════════════════════════════════════════════════════════════════════════
// §2.6 Nr. 26 · eigen5_antikorreliert_neben_korreliert
// ═════════════════════════════════════════════════════════════════════════
//
// Beide Quellen nebeneinander: eine läuft MIT dem Master, eine GEGEN ihn.
// Das ist der Kontrollfall zu R2 und R3 zugleich — die beiden sind über den
// Uplift getrennt (`Some(positiv)` gegen `Some(negativ)`), und genau EINER
// darf stark sein. Vor NAK-212 waren es zwei Befunde, von denen der
// antikorrelierte über seine geschenkte `wiederholbarkeit` mithielt.
#[test]
fn eigen5_antikorreliert_neben_korreliert() {
    let b = Buehne::schlank();
    let mit = adresse(2);
    let gegen = adresse(3);
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sondeM", &mit, "passive_probe", Some(3));
    b.anmelden("sondeG", &gegen, "passive_probe", Some(4));
    b.marke(0, json!([]));
    let band = (BAND_VON, BAND_BIS);
    let laut_ungerade = |i: usize| if i % 2 == 1 { 12.0 } else { 6.0 };
    let laut_gerade = |i: usize| if i % 2 == 1 { 6.0 } else { 12.0 };
    b.belege_je_fenster("main", &b.master, 0, 12, 0, laut_ungerade, band);
    b.belege_je_fenster("sondeM", &mit, 100, 12, 0, laut_ungerade, band);
    b.belege_je_fenster("sondeG", &gegen, 200, 12, 0, laut_gerade, band);

    let befunde = b.befunde();
    protokoll("EIGEN-5 korrelierte und antikorrelierte Quelle nebeneinander", &befunde);
    assert_eq!(befunde.len(), 2, "beide bleiben sichtbar (M-26)");
    // 🔑 D7: der quantisierte Rangabstand ist VORBEDINGUNG jeder Zeile, die
    // einen starken Befund neben einem zweiten Kandidaten zusagt — und er
    // wird gemessen, nicht aus der Konstruktion erschlossen.
    assert_ne!(
        rang_quantisiert(&befunde[0].rang),
        rang_quantisiert(&befunde[1].rang),
        "die Gesamtraenge trennen wirklich: {:?} gegen {:?}",
        befunde[0].rang,
        befunde[1].rang
    );
    assert_eq!(
        befunde[0].candidate_source, mit.instance_id,
        "die MITlaufende Quelle fuehrt"
    );
    assert_eq!(
        stark(&befunde),
        1,
        "genau EIN starker Befund (M-21): {:?}",
        befunde.iter().map(|f| f.confidence).collect::<Vec<_>>()
    );
    assert_eq!(befunde[0].confidence.klasse, Sicherheitsklasse::Hoch);
    assert!(
        befunde[1].confidence.klasse < Sicherheitsklasse::Hoch,
        "und die antikorrelierte bleibt schwach"
    );
    assert_eq!(
        befunde[1].rang.wiederholbarkeit, 0.0,
        "ihre Stabilitaet ist kein Beleg mehr"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// N-28 · a4_masteranomalie_ausserhalb_der_passage
// ═════════════════════════════════════════════════════════════════════════
//
// Codex-Befund A4: `masteranomalie` las `master.fenster.last()` ohne
// Passagenbezug. Liefert der Master ein Fenster HINTER der benannten Passage
// — mit einer Anomalie in einem ganz anderen Band —, nannte der Befund die
// alte Passage und behauptete deren FREMDE Anomalie, mit `hoch` und READY.
//
// Seit E5 wird die Anomalie aus dem letzten Fenster INNERHALB der Passage
// bestimmt; das Außenfenster geht in nichts ein.
#[test]
fn a4_masteranomalie_ausserhalb_der_passage() {
    let b = Buehne::mit_store("a4");
    let sonde = adresse(2);
    b.anmelden("main", &b.master, "main", Some(0));
    b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
    b.marke(0, json!([]));
    // Acht Fensterlängen Passage — der Randwert `GATE_MINDEST_FENSTER`.
    b.passage(0, 8 * FENSTER, &[&sonde]);
    let band = (BAND_VON, BAND_BIS);
    let muster = wechselnd(8);
    b.belege_je_fenster("main", &b.master, 0, 8, 0, &muster, band);
    b.belege_je_fenster("sonde0", &sonde, 100, 8, 0, &muster, band);
    // Das NEUNTE Masterfenster liegt hinter der Passage — und trägt seine
    // Anomalie siebzig Bandindizes weiter oben.
    b.belege_je_fenster("main", &b.master, 300, 1, 8, |_| 9.0, (150, 154));

    let befunde = b.befunde();
    protokoll("A4 Masteranomalie in einem Fenster HINTER der Passage", &befunde);
    let befund = befunde.first().expect("ein Ergebnis entsteht immer");
    // Die Vorbedingung: der Fall trifft den Defekt nur, wenn wirklich eine
    // Passage wirkt und der Kandidat sie besteht.
    assert_eq!(
        befund.candidate_source, sonde.instance_id,
        "der Kandidat ist im Rennen, das hier ist keine Enthaltung"
    );
    assert!(befund.passage_id.is_some(), "die Passage wirkt wirklich");
    // Die Zusage: das Befundband stammt aus der PASSAGE, nicht aus dem
    // Fenster dahinter.
    assert!(
        befund.band_hz.von < 150 && befund.band_hz.bis <= 150,
        "die Anomalie kommt aus der Passage (98..102), nicht aus dem \
         Aussenfenster (150..154): {:?}",
        befund.band_hz
    );
    assert!(
        befund.beobachtung.gueltig,
        "und sie ist eine echte Messung: {:?}",
        befund.beobachtung
    );
    assert_eq!(
        stark(&befunde),
        1,
        "acht Fenster in der Passage tragen die Aussage: {:?}",
        befund.confidence
    );
}


// ═════════════════════════════════════════════════════════════════════════
// NAK-213 R2/E4 · einzelueberlebender_neben_passagenausschluss  (K-12, K-13)
// ═════════════════════════════════════════════════════════════════════════
//
// R2 nennt fuenf Messgruende. Drei von ihnen misst
// `sonde014_hypothese.rs::einzelueberlebender_neben_messausschluss` am
// Produktpfad; die zwei PASSAGENgruende brauchen eine benannte Passage und
// liegen deshalb hier, wo der Harnisch sie ueber `experiment_begin` samt
// Store anlegt. Eine zweite Passagenflaeche im Nachbarbein waere eine zweite
// Wahrheit.
//
// Die Zusage ist in beiden Faellen dieselbe: der EINZIGE Ueberlebende ist
// nicht „getrennt", solange ein Konkurrent aus einem MESSGRUND ausgeschieden
// ist. Er hat sich nicht behauptet, er hat nur ueberlebt.
#[test]
fn einzelueberlebender_neben_passagenausschluss() {
    let band = (BAND_VON, BAND_BIS);

    // ── Vorbedingung: DERSELBE Aufbau ohne Konkurrenten traegt `hoch`.
    {
        let b = Buehne::mit_store("k12-vorbedingung");
        let sonde = adresse(2);
        b.anmelden("main", &b.master, "main", Some(0));
        b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
        b.marke(0, json!([]));
        b.passage(0, 12 * FENSTER, &[&sonde]);
        let muster = wechselnd(12);
        b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
        b.belege_je_fenster("sonde0", &sonde, 100, 12, 0, &muster, band);
        let befunde = b.befunde();
        protokoll("K-12/K-13 Vorbedingung: allein in der Passage", &befunde);
        assert_eq!(
            stark(&befunde),
            1,
            "Vorbedingung: ohne Konkurrenten ist der Ueberlebende stark"
        );
    }

    // ── Fall c: `passage_unvergleichbar` ─────────────────────────────────
    //
    // 🔑 Der Aufbau ist enger, als er aussieht. Gate 3 (Alignment) steht VOR
    // dem Passagengate; eine verschobene oder epochenfremde Fensterreihe
    // faellt deshalb mit `alignment_falsch`, und die Zeile maesse einen
    // anderen Grund als den benannten (im ersten Aufbau genau so gemessen).
    // Der Kandidat traegt hier alle zwoelf Fenster AUF den Masterfenstern —
    // `paarueberdeckung` nimmt das Maximum beider Richtungen, also bleibt das
    // Alignment 1,0 — plus EIN Fenster weit hinter der Passage. Damit waechst
    // seine SPANNE ueber die Passage hinaus, und genau die liest Gate 4a.
    {
        let b = Buehne::mit_store("k12-unvergleichbar");
        let sonde = adresse(2);
        let konkurrent = adresse(3);
        b.anmelden("main", &b.master, "main", Some(0));
        b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
        b.anmelden("sonde1", &konkurrent, "passive_probe", Some(4));
        b.marke(0, json!([]));
        // Die Passage ist DOPPELT so lang wie die Masterfenster: nur dann
        // kann ein Kandidat, der jedes Masterfenster deckt, die PASSAGE
        // trotzdem verfehlen — `zeitueberdeckung` normiert auf das KUERZERE
        // Intervall, und liegt eines ganz im anderen, ist das Verhaeltnis
        // immer 1,0.
        b.passage(0, 20 * FENSTER, &[&sonde, &konkurrent]);
        let muster = wechselnd(20);
        b.belege_je_fenster("main", &b.master, 0, 10, 10, &muster, band);
        b.belege_je_fenster("sonde0", &sonde, 100, 10, 10, &muster, band);
        b.belege_je_fenster("sonde1", &konkurrent, 300, 20, 10, &muster, band);
        let befunde = b.befunde();
        protokoll("K-12 Konkurrent faellt an `passage_unvergleichbar`", &befunde);
        let fuehrend = befunde.first().expect("ein Ergebnis entsteht immer");
        assert!(
            fuehrend
                .ausschluesse
                .iter()
                .any(|a| a.grund == Ausschlussgrund::PassageUnvergleichbar),
            "Vorbedingung: der Konkurrent faellt MIT diesem Grund: {:?}",
            fuehrend.ausschluesse
        );
        assert_eq!(
            stark(&befunde),
            0,
            "K-12: hoechstens `mittel` neben einem Messausschluss — heutiger \
             Stand: `getrennt` ist trivial wahr und der Ueberlebende traegt \
             `hoch`: {:?}",
            fuehrend.confidence
        );
    }

    // ── Fall d: `passage_zu_kurz` ────────────────────────────────────────
    //
    // Der Konkurrent liegt IN der Passage, traegt darin aber nur vier
    // unabhaengige Fenster — unter `GATE_MINDEST_FENSTER`.
    {
        let b = Buehne::mit_store("k13-zu-kurz");
        let sonde = adresse(2);
        let konkurrent = adresse(3);
        b.anmelden("main", &b.master, "main", Some(1));
        b.anmelden("sonde0", &sonde, "passive_probe", Some(3));
        b.anmelden("sonde1", &konkurrent, "passive_probe", Some(4));
        b.marke(0, json!([]));
        b.passage(0, 12 * FENSTER, &[&sonde, &konkurrent]);
        let muster = wechselnd(12);
        b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
        b.belege_je_fenster("sonde0", &sonde, 100, 12, 0, &muster, band);
        b.belege_je_fenster("sonde1", &konkurrent, 300, 4, 0, &muster, band);
        let befunde = b.befunde();
        protokoll("K-13 Konkurrent faellt an `passage_zu_kurz`", &befunde);
        let fuehrend = befunde.first().expect("ein Ergebnis entsteht immer");
        assert!(
            fuehrend
                .ausschluesse
                .iter()
                .any(|a| a.grund == Ausschlussgrund::PassageZuKurz),
            "Vorbedingung: der Konkurrent faellt MIT diesem Grund: {:?}",
            fuehrend.ausschluesse
        );
        assert_eq!(
            stark(&befunde),
            0,
            "K-13: hoechstens `mittel` neben einem Messausschluss: {:?}",
            fuehrend.confidence
        );
    }
}


// ═════════════════════════════════════════════════════════════════════════
// NAK-213 R3/E6 · a3_sonde_auf_masterkanal   (K-22, K-23)
// ═════════════════════════════════════════════════════════════════════════
//
// Gate-Befund E-D1 = A3: eine Sonde auf dem Mixerkanal des Masters misst
// dessen SIGNAL. Bis NAK-213 lasen `ids` und `ist_parent` nur die Kandidaten,
// die Kanaltafel entstand aus den Evidenzschlüsseln, und `parent` wurde nie
// an den Master geschrieben — die Sonde bekam `duplikat = false`, volle
// Routingqualität und `hoch`/`ready_to_send`.
//
// G5 §8 wörtlich: „eine Quelle, die das Mastersignal misst, ist nie dessen
// Ursache". Deshalb ein AUSSCHLUSS und kein Deckel: der Befundsatz „X drängt
// im Bandbereich a..b gegen den Master" ist über sie nicht schwächer wahr,
// sondern falsch (M-69).
#[test]
fn a3_sonde_auf_masterkanal() {
    let band = (BAND_VON, BAND_BIS);
    let muster = wechselnd(12);

    // ── K-22: die Sonde auf dem Masterkanal fällt mit Grund ──────────────
    {
        let b = Buehne::schlank();
        let sonde = adresse(2);
        // Kanal 1 ist der kleinste vertragsgültige Index (`minimum: 1`).
        b.anmelden("main", &b.master, "main", Some(1));
        b.anmelden("sonde0", &sonde, "passive_probe", Some(1));
        b.marke(0, json!([]));
        b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
        b.belege_je_fenster("sonde0", &sonde, 100, 12, 0, &muster, band);

        let befunde = b.befunde();
        protokoll("A3 Sonde auf dem Masterkanal (beide Kanal 1)", &befunde);
        assert!(
            !befunde
                .iter()
                .any(|f| f.candidate_source == sonde.instance_id),
            "kein Befund ueber sie — weder `hoch` noch `mittel`: {befunde:?}"
        );
        let enthaltung = befunde.first().expect("die Enthaltung entsteht");
        assert_eq!(
            enthaltung.ursachenklasse,
            Ursachenklasse::DatenReichenNicht,
            "die Sitzung enthaelt sich: {:?}",
            enthaltung.ursachenklasse
        );
        assert!(
            enthaltung
                .ausschluesse
                .iter()
                .any(|x| x.candidate_source == sonde.instance_id
                    && x.grund == Ausschlussgrund::MasterDuplikat),
            "und sie steht MIT Grund in der Enthaltung: {:?}",
            enthaltung.ausschluesse
        );
    }

    // ── K-23: `master_duplikat` ist KEIN Messgrund ───────────────────────
    //
    // Gegenprobe: zählte er als Messausschluss, senkte eine Doppelmessung am
    // Master jede andere Aussage der Sitzung.
    {
        let b = Buehne::schlank();
        let auf_master = adresse(2);
        let eigene = adresse(3);
        b.anmelden("main", &b.master, "main", Some(1));
        b.anmelden("sonde0", &auf_master, "passive_probe", Some(1));
        b.anmelden("sonde1", &eigene, "passive_probe", Some(4));
        b.marke(0, json!([]));
        b.belege_je_fenster("main", &b.master, 0, 12, 0, &muster, band);
        b.belege_je_fenster("sonde0", &auf_master, 100, 12, 0, &muster, band);
        b.belege_je_fenster("sonde1", &eigene, 300, 12, 0, &muster, band);

        let befunde = b.befunde();
        protokoll("A3 zweite Sonde auf eigenem Kanal", &befunde);
        let fuehrend = befunde.first().expect("ein Befund entsteht");
        assert_eq!(
            fuehrend.candidate_source, eigene.instance_id,
            "die zweite Sonde bleibt und fuehrt"
        );
        // Vorbedingung: sie erfuellt die `hoch`-Bedingungen aus NAK-212.
        assert_eq!(fuehrend.rang.routingqualitaet, 1.0, "eigener Kanal, kein Duplikat");
        assert!(fuehrend.rang.bandpassung > 0.0, "Energie im Befundband");
        assert!(fuehrend.rang.uplift > 0.0, "belegter Zusammenhang");
        assert_eq!(
            stark(&befunde),
            1,
            "`master_duplikat` ist KEIN Messgrund — der Ausschluss belegt keine \
             fehlende Messung, sondern eine Quelle, die per Konstruktion keine \
             Ursache sein kann: {:?}",
            fuehrend.confidence
        );
    }
}
