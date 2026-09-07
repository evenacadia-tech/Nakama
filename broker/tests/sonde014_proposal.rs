//! Die Proposal-Policy am Produktpfad (SONDE-014 Etappe F, M-42 bis M-54,
//! M-63 und M-10).
//!
//! ── ZWEI EBENEN, EINE ZUSAGE ──────────────────────────────────────────────
//!
//! Die Fälle hier fahren die Bühne aus Evidenz durch `p1` und lesen das
//! Ergebnis an `vorschlaege_sicht` — also an dem, was der Coordinator
//! wirklich gebildet hat. Wo eine Zusage eine **Eigenschaft der Rechnung**
//! ist (Determinismus, Budgets, Rückweg je Typ), steht sie zusätzlich als
//! Eigenschaftsprüfung über zufällige Eingänge; das ist die Ebene, die ein
//! einzelner Bühnenfall nicht erreicht.
//!
//! M-48 — der gelesene gegen den geratenen Zielbereich — liegt in
//! `sonde013_verdrahtung.rs`, wo der Experimentpfad samt Store schon steht.

use eqcop_broker::coordinator::{
    Aktion, Ausfuehrung, Coordinator, ManualClock, NaechsterTest, Rueckweg, Stopbedingung,
};
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

/// **WN-04 (Nacharbeit 2, 07.09.2026): diese Buehne fuehrt jetzt eine
/// benannte Passage - und dafuer braucht sie einen Store.**
///
/// Ohne Passage entsteht seit WN-04 gar kein Proposal (WP1-4), und eine
/// benannte Passage entsteht nur aus einem persistenzpflichtigen
/// `experiment_begin`. Die Faelle dieser Datei messen die Policy an einem
/// VOLLSTAENDIGEN Objekt; der Fall "ohne Passage kein Objekt" liegt in
/// `sonde014_nacharbeit2.rs`, wo er die Store-Zeile mitmisst.
struct TestOrdner(std::path::PathBuf);

impl TestOrdner {
    fn neu() -> Self {
        let pfad = std::env::temp_dir().join(format!(
            "nakama-sonde014-proposal-{}-{}",
            std::process::id(),
            uuid::Uuid::new_v4().simple()
        ));
        std::fs::create_dir_all(&pfad).unwrap();
        Self(pfad)
    }
}

impl Drop for TestOrdner {
    fn drop(&mut self) {
        let _ = std::fs::remove_dir_all(&self.0);
    }
}

/// Der Coordinator samt Store, der ihn ueberlebt.
///
/// `Deref` haelt jeden bestehenden Aufruf `c.p1(..)` und `buehne(&c, ..)`
/// unveraendert - der Umbau ist die Buehne, nicht die Messung.
struct Buehne {
    c: Coordinator,
    _writer: eqcop_broker::store::StoreWriter,
    _ordner: TestOrdner,
}

impl std::ops::Deref for Buehne {
    type Target = Coordinator;
    fn deref(&self) -> &Coordinator {
        &self.c
    }
}

fn coordinator() -> Buehne {
    let ordner = TestOrdner::neu();
    let mut k =
        eqcop_broker::store::StoreKonfiguration::fuer_pfad(ordner.0.join(eqcop_broker::store::STORE_DATEINAME));
    k.remote_volume_override = Some(false);
    let writer = eqcop_broker::store::StoreWriter::starten(k);
    assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());
    let c = Coordinator::mit_store(Arc::new(ManualClock::default()), hex(0xbeef), &writer);
    Buehne {
        c,
        _writer: writer,
        _ordner: ordner,
    }
}

fn fixture(name: &str) -> Value {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../eq-copilot/fixtures/v3/gueltig")
        .join(format!("{name}.json"));
    serde_json::from_slice(&std::fs::read(&pfad).expect("Fixture liegt im Korpus"))
        .expect("Fixture ist JSON")
}

fn capabilities() -> Value {
    json!({
        "host_context": true, "project_time": true, "fine_automation": false,
        "double_precision": false, "latency_report": false, "aux_send": false,
        "aux_return": false, "compare_routing": false, "sidechain": false,
        "offline_render": false
    })
}

fn anmelden(c: &Coordinator, link: &str, a: &Adresse, art: &str, mixer: i64) {
    let h = hello(a.clone(), art);
    assert!(c.control_hello_registrieren(link, &h).angenommen);
    let _ = c.resync_bestaetigen(link, 0);
    let position = if art == "main" { "insert" } else { "post" };
    assert!(c.descriptor_setzen(
        link,
        json!({
            "adresse": a,
            "plugin_kind": art,
            "measurement_position": position,
            "aussageklasse": "beobachtend",
            "betrieb": "active",
            "label": "Testquelle",
            "host_mixer_index": mixer,
            "capabilities": capabilities(),
            "frische": {"letzter_kontakt_ms": 10, "stale": false}
        })
    ));
    // WN-04: ohne `record_state` weist der Broker jeden persistenzpflichtigen
    // Befehl mit `record_state_unknown` ab - und ohne `experiment_begin` gibt
    // es keine benannte Passage.
    let bericht = json!({
        "type": "state_report",
        "adresse": a,
        "dsp_schema_version": 1,
        "state_revision": 0,
        "state_hash": "a".repeat(64),
        "record_state": {"valid": true, "recording": false}
    });
    assert!(c.state_report_json(link, &serde_json::to_vec(&bericht).unwrap()));
}

fn evidenz_grundform() -> Value {
    static ROH: std::sync::OnceLock<Value> = std::sync::OnceLock::new();
    ROH.get_or_init(|| {
        let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join(
            "../eq-copilot/fixtures/v3/gueltig/evidence-snapshot-mit-ereignissen-und-stereo.json",
        );
        serde_json::from_slice(&std::fs::read(&pfad).expect("Evidenzfixture liegt im Korpus"))
            .expect("Evidenzfixture ist JSON")
    })
    .clone()
}

const ANOMALIEBAND: usize = 98;
/// Die Transportepoche der committeten Evidenzgrundform. Passage und Belege
/// muessen dieselbe fuehren, sonst ist die Passage eine ANDERE (M-23).
const TRANSPORT_EPOCHE: u64 = 17;
const BASIS_SAMPLE: i64 = 44_108_200;

fn evidenz(a: &Adresse, nr: usize, projekt_start: i64, anhebung_db: f64) -> Vec<u8> {
    let mut wert = evidenz_grundform();
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
                werte[index] = json!(alt + (anhebung_db * 10.0).round() as i64);
            }
        }
    }
    serde_json::to_vec(&wert).unwrap()
}

/// **M-86, NR-01:** der leere Vollbestand MIT Marke, bevor gerechnet wird.
///
/// Seit der Nacharbeit 1 (07.09.2026) sperrt `intent == None` genauso wie ein
/// unvollstaendiger Bestand. Jede Buehne, die einen Befund erwartet, meldet
/// deshalb zuerst den Bestand — wie der Main es tut.
fn intent_marke(c: &Coordinator, link: &str, a: &Adresse) {
    let wert = json!({
        "type": "intent_update",
        "adresse": a,
        "session_epoch": a.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 0
    });
    c.p1(link, &serde_json::to_vec(&wert).unwrap());
}

/// Master und Sonde, `fenster` Belege je Seite. Rückgabe: die Adressen.
fn buehne(c: &Coordinator, fenster: usize) -> Vec<Adresse> {
    let master = adresse(1);
    let sonde = adresse(2);
    anmelden(c, "main", &master, "main", 0);
    anmelden(c, "sonde0", &sonde, "passive_probe", 3);
    intent_marke(c, "main", &master);
    passage_anlegen(c, &master, &[&master, &sonde], fenster as i64);
    // 🔑 **NAK-212 R1 (07.09.2026).** Jedes zweite Fenster ist laut, das
    // LETZTE immer. Mit konstanter Anhebung liegt jeder Wert auf oder ueber
    // dem eigenen Median, die Vergleichsmenge „ohne die Quelle" bleibt leer,
    // und der bedingte Uplift ist nach M-19 nicht messbar — der Befund
    // erreichte dann zu Recht nur `mittel`, und die Proposal-Faelle, die
    // einen handelbaren Befund brauchen, maessen nichts mehr. Das letzte
    // Fenster ist laut, damit `masteranomalie` dasselbe Band findet.
    let laut = |i: usize| if i % 2 == (fenster.max(1) - 1) % 2 { 9.0 } else { 0.0 };
    for i in 0..fenster {
        let zeit = BASIS_SAMPLE + (i as i64) * 512;
        c.p1("main", &evidenz(&master, i, zeit, laut(i)));
        c.p1("sonde0", &evidenz(&sonde, 100 + i, zeit, laut(i)));
    }
    vec![master, sonde]
}

/// Die benannte Passage ueber den PRODUKTPFAD: `experiment_begin` als P0.
///
/// **WN-04 (Nacharbeit 2):** ohne sie entsteht kein Proposal mehr. Die
/// Passage deckt `fenster` Projektfenster ab Basis ab und traegt die
/// Transportepoche der Evidenzgrundform - eine andere Epoche waere eine
/// ANDERE Passage (M-23).
fn passage_anlegen(
    c: &Coordinator,
    master: &Adresse,
    quellen: &[&Adresse],
    fenster: i64,
) -> String {
    let mut wert = fixture("experiment_begin");
    wert["kopf"]["ziel"] = serde_json::to_value(master).unwrap();
    wert["kopf"]["command_id"] = json!(hex(0x930));
    wert["kopf"]["base_revision"] = json!(0);
    wert["experiment_id"] = json!(hex(0xab7));
    let passage_id = hex(0x5001);
    wert["passage"]["passage_id"] = json!(passage_id);
    wert["passage"]["projekt_von"] = json!(BASIS_SAMPLE);
    wert["passage"]["projekt_bis"] = json!(BASIS_SAMPLE + fenster * 512);
    wert["passage"]["transport_epoch"] = json!(TRANSPORT_EPOCHE);
    wert["passage"]["aktive_quellen"] =
        json!(quellen.iter().map(|a| a.instance_id.clone()).collect::<Vec<_>>());
    let antwort = Senke::p0(c, "main", &serde_json::to_vec(&wert).unwrap())
        .expect("experiment_begin wird beantwortet");
    let ack: Value = serde_json::from_slice(&antwort).unwrap();
    assert_eq!(
        ack["ergebnis"], "angewandt",
        "die Passage entsteht wirklich: {ack:?}"
    );
    passage_id
}

fn vertrag_schema() -> Value {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../eq-copilot/schemas/v3/eq-ipc-v3.schema.json");
    serde_json::from_slice(&std::fs::read(&pfad).expect("Vertrag liegt im Repo"))
        .expect("Vertrag ist JSON")
}

// ═════════════════════════════════════════════════════════════════════════
// M-42 · proposal_traegt_die_fuenfzehn_felder
// ═════════════════════════════════════════════════════════════════════════
//
// Die Vertragshälfte liegt in Etappe B (A5 mit Negativfixture je Feld). HIER
// fällt die Erzeugerhälfte: das Objekt, das der Produktpfad WIRKLICH baut,
// trägt jedes Pflichtfeld — und keines mehr.
#[test]
fn proposal_traegt_die_fuenfzehn_felder() {
    let c = coordinator();
    let _ = buehne(&c, 12);
    let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    assert!(!vorschlaege.is_empty(), "ein Vorschlag entsteht");

    let vertrag = vertrag_schema();
    let pflicht: Vec<&str> = vertrag["$defs"]["proposal"]["required"]
        .as_array()
        .expect("Pflichtfelder stehen im Vertrag")
        .iter()
        .map(|v| v.as_str().unwrap())
        .collect();
    // 🔑 NR-07 (Nacharbeit 1, 07.09.2026): SECHZEHN. `passage_id` ist seit
    // der Nacharbeit Pflicht - das Exit-Gate verlangt die Passage woertlich
    // fuer jedes Proposal, auch fuer `no_change` und `more_data` (M-43/M-46).
    assert_eq!(pflicht.len(), 16, "fuenfzehn Vertragsfelder plus die Passage");
    let erlaubt: Vec<&str> = vertrag["$defs"]["proposal"]["properties"]
        .as_object()
        .unwrap()
        .keys()
        .map(String::as_str)
        .collect();

    let wire = Coordinator::proposal_json_fuer_test(&vorschlaege[0]);
    let objekt = wire.as_object().unwrap();
    for feld in &pflicht {
        // 🔑 WN-04 (Nacharbeit 2, 07.09.2026): der Sprung ist FORT.
        //
        // Bis zu dieser Runde stand hier eine Ausnahme fuer `passage_id` —
        // der Test schrieb damit die Vertragsabweichung fest, die er messen
        // sollte. Seit WN-04 entsteht ohne benannte Passage gar kein Objekt,
        // und diese Buehne fuehrt eine (`passage_anlegen`). Jedes Pflichtfeld
        // steht deshalb am Draht, ohne Ausnahme.
        assert!(objekt.contains_key(*feld), "Pflichtfeld {feld} fehlt");
    }
    for feld in objekt.keys() {
        assert!(
            erlaubt.contains(&feld.as_str()),
            "{feld} steht nicht im Vertrag"
        );
    }
    assert_eq!(objekt["proposal_schema"], json!(1));
}

// ═════════════════════════════════════════════════════════════════════════
// M-43 · sechs_gate_felder_sind_pflicht / revert_hat_drei_werte
// ═════════════════════════════════════════════════════════════════════════
//
// Das Exit-Gate verlangt sechs Angaben wörtlich. Der Rückweg ist ein **Feld**
// — und seine Menge hat drei Werte, nicht vier: `dsp_revert` gehört P7.
#[test]
fn sechs_gate_felder_sind_pflicht_und_revert_hat_drei_werte() {
    let c = coordinator();
    let _ = buehne(&c, 12);
    let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    // 🔑 NR-07 (Nacharbeit 1, 07.09.2026): der Sprung ueber `passage_id` ist
    // fort. Er stand hier, weil diese Buehne keine benannte Passage fuehrt —
    // und er machte aus einem Pflichtfeld eine Ausnahme.
    //
    // Die Zusage hat seither zwei Haelften, und sie liegen an zwei Orten:
    // HIER faellt sie an der Folge (ohne Passage entsteht kein Angebot), und
    // mit Passage prueft `sechs_gate_felder_sind_pflicht_mit_passage` in
    // `sonde014_nacharbeit1.rs` alle sechs Felder am vollstaendigen Objekt.
    // Eine Passage braucht ein persistenzpflichtiges `experiment_begin` und
    // damit einen echten Store; der Harnisch dafuer liegt dort.
    for vorschlag in &vorschlaege {
        let felder = vorschlag.gate_felder_vollstaendig();
        assert_eq!(felder.len(), 6, "das Exit-Gate nennt SECHS Angaben");
        // 🔑 WN-04: alle sechs, ohne Ausnahme. Die Buehne fuehrt seit
        // dieser Runde eine benannte Passage, und ein Vorschlag, dem eine der
        // sechs Angaben fehlte, entstuende gar nicht erst.
        for (feld, belegt) in felder {
            assert!(belegt, "Gate-Feld {feld} fehlt");
        }
    }
    // In P5 geht trotzdem kein Angebot hinaus: die Capability fehlt (M-45,
    // M-52). Das ist die zweite Sperre, nicht dieselbe.
    assert_eq!(
        c.draft_offers_zaehler(),
        0,
        "in P5 gibt es keine Capability, also kein Angebot"
    );
    assert!(!befunde.is_empty());

    // Die geschlossene Menge, gegen den Vertrag gehalten.
    let vertrag = vertrag_schema();
    let menge: Vec<&str> = vertrag["$defs"]["proposal_revert"]["enum"]
        .as_array()
        .unwrap()
        .iter()
        .map(|v| v.as_str().unwrap())
        .collect();
    assert_eq!(menge.len(), 3, "drei Rueckwege");
    for wort in &menge {
        assert!(Rueckweg::aus_wire(wort).is_some(), "{wort}");
    }
    // Ein VIERTER faellt — und `dsp_revert` ist genau dieser vierte.
    assert!(Rueckweg::aus_wire("dsp_revert").is_none());
    assert!(Rueckweg::aus_wire("").is_none());
    assert!(!menge.contains(&"dsp_revert"));

    // Je Vorschlagstyp sein Rueckweg — die Abbildung ist total.
    for aktion in Aktion::ALLE {
        let r = rueckweg_fuer(aktion);
        assert!(
            menge.contains(&r.wire()),
            "{} traegt einen fremden Rueckweg",
            aktion.wire()
        );
    }
}

/// Der Rückweg je Aktion, wie der Erzeuger ihn setzt. Er steht hier als
/// Erwartung des Beins, nicht als zweite Rechnung: der Fall darunter hält ihn
/// gegen den tatsächlich erzeugten Vorschlag.
fn rueckweg_fuer(aktion: Aktion) -> Rueckweg {
    match aktion {
        Aktion::StaticEqCut | Aktion::DynamicEqCut | Aktion::Trim => Rueckweg::ExperimentAbort,
        Aktion::ManualGuidance => Rueckweg::ManualOnly,
        Aktion::NoChange | Aktion::MoreData => Rueckweg::NoneNeeded,
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-44 · derselbe_eingang_erzeugt_denselben_entwurf
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn derselbe_eingang_erzeugt_denselben_entwurf() {
    let lauf = || {
        let c = coordinator();
        let _ = buehne(&c, 12);
        let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
        serde_json::to_vec(
            &vorschlaege
                .iter()
                .map(Coordinator::proposal_json_fuer_test)
                .collect::<Vec<_>>(),
        )
        .unwrap()
    };
    let referenz = lauf();
    assert!(!referenz.is_empty());
    for runde in 0..100 {
        assert_eq!(lauf(), referenz, "Lauf {runde} weicht ab");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-45 · in_p5_ist_jede_aktion_manual
// ═════════════════════════════════════════════════════════════════════════
//
// `previewable` setzt eine eigene DSP-Fähigkeit voraus, und die entsteht erst
// in P6. Ein Proposal mit `execution = previewable` ist in P5 ein
// Vertragsbruch, kein optimistischer Vorgriff.
#[test]
fn in_p5_ist_jede_aktion_manual() {
    let c = coordinator();
    let _ = buehne(&c, 12);
    let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    assert!(!vorschlaege.is_empty());
    for vorschlag in &vorschlaege {
        assert_eq!(
            vorschlag.execution,
            Ausfuehrung::Manual,
            "{} traegt previewable",
            vorschlag.action.wire()
        );
    }
    // Und der Wert steht auf der Leitung genauso.
    for vorschlag in &vorschlaege {
        let wire = Coordinator::proposal_json_fuer_test(vorschlag);
        assert_eq!(wire["execution"].as_str(), Some("manual"));
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-46 · keine_aenderung_und_mehr_daten_sind_vorschlaege
// ═════════════════════════════════════════════════════════════════════════
//
// Beide sind reguläre Ergebnisse mit **vollständigem** Objekt, nicht ein
// leerer Rückgabewert. Gemessen an einer Bühne ohne Kandidat: dort entsteht
// die Enthaltung, und aus ihr ein Vorschlag.
#[test]
fn keine_aenderung_und_mehr_daten_sind_vorschlaege() {
    let c = coordinator();
    let master = adresse(1);
    let sonde = adresse(2);
    anmelden(&c, "main", &master, "main", 0);
    // Die Sonde ist ANGEMELDET, liefert aber keinen Beleg: der Vertrag
    // verlangt zwei aktive Quellen an der Passage, die Rechnung braucht
    // Evidenz. Kandidat wird sie damit keiner - genau die Lage, die dieser
    // Fall misst.
    anmelden(&c, "sonde0", &sonde, "passive_probe", 3);
    intent_marke(&c, "main", &master);
    // WN-04: auch die Enthaltung braucht ihren Ort. `no_change` und
    // `more_data` sind VOLLSTAENDIGE Objekte (M-46) — und
    // `vollstaendig` schliesst die Passage ein.
    passage_anlegen(&c, &master, &[&master, &sonde], 12);
    for i in 0..12 {
        c.p1(
            "main",
            &evidenz(&master, i, 44_108_200 + (i as i64) * 512, 9.0),
        );
    }
    let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    assert_eq!(vorschlaege.len(), 1, "auch ohne Kandidat gibt es ein Objekt");
    let v = &vorschlaege[0];
    assert!(
        matches!(v.action, Aktion::MoreData | Aktion::NoChange),
        "{:?}",
        v.action
    );
    // VOLLSTAENDIG: Ziel, Passage, Hoerziel, Stopbedingung, Rueckweg.
    assert!(!v.listen_for.is_empty());
    assert!(!v.stop_if.is_empty());
    assert_eq!(v.revert, Rueckweg::NoneNeeded);
    // M-54: auch dieses Objekt traegt Evidenz-IDs.
    assert!(!v.evidence_ids.is_empty());
    // Und es traegt KEINE Parameter — ein Nichteingriff hat keine.
    assert_eq!(v.parameters.frequency_hz, None);
    assert_eq!(v.parameters.max_gain_db, None);
}

// ═════════════════════════════════════════════════════════════════════════
// M-47 · zielbereich_kommt_aus_dem_befund_nicht_aus_dem_delta
// ═════════════════════════════════════════════════════════════════════════
//
// Die Frequenz kommt aus `band_hz` des Befunds. Das ist NAK-168 wörtlich —
// und der Grund, warum `band_hz` überhaupt im Befund steht.
#[test]
fn zielbereich_kommt_aus_dem_befund_nicht_aus_dem_delta() {
    use eqcop_broker::coordinator::proposal::{bandmitte_hz, proposal, Proposallage};
    let c = coordinator();
    let _ = buehne(&c, 12);
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    assert_eq!(befunde.len(), vorschlaege.len());
    // Am Produktpfad: der Vorschlag zeigt auf DIESEN Befund.
    assert_eq!(
        vorschlaege[0].finding_id.as_deref(),
        Some(befunde[0].finding_id.as_str())
    );

    // ⚠️ In P5 entsteht am Produktpfad NIE ein Eingriff — es gibt keine
    // Capability (M-45, M-52). Die Frage „woher kommt die Frequenz" laesst
    // sich dort also gar nicht stellen. Gemessen wird sie deshalb an einer
    // Lage, in der ein Eingriff moeglich WAERE; der Befund selbst kommt
    // unveraendert aus dem Produktpfad.
    let befund = befunde[0].clone();
    let lage = Proposallage {
        ziel_instanz: hex(1),
        base_revision: 0,
        intent: None,
        capability_vorhanden: true,
        messbare_guardrails: Stopbedingung::ALLE.to_vec(),
        usergrenze_db: None,
        session_epoch: hex(0x22),
        metrics_version: 1,
    };
    let v = proposal(&befund, &lage)
        .expect("WN-04: die Buehne fuehrt eine benannte Passage, also entsteht ein Objekt");
    assert!(
        v.action.ist_eingriff(),
        "die Gegenprobe: mit Capability entsteht wirklich ein Eingriff ({:?})",
        v.action
    );
    let mitte = bandmitte_hz(befund.band_hz.von + (befund.band_hz.bis - befund.band_hz.von) / 2);
    let gesetzt = v.parameters.frequency_hz.expect("eine Frequenz");
    assert!(
        (gesetzt - mitte).abs() < 0.01,
        "die Frequenz kommt aus dem Band {}..{}: {mitte} gegen {gesetzt}",
        befund.band_hz.von,
        befund.band_hz.bis
    );
    let grenzen = v.allowed_bounds.frequency_hz.expect("Grenzen");
    assert!(grenzen.von <= gesetzt && gesetzt <= grenzen.bis);
    // Und die Grenzen liegen im BAND des Befunds, nicht irgendwo.
    assert!((grenzen.von - bandmitte_hz(befund.band_hz.von)).abs() < 0.01);
}

// ═════════════════════════════════════════════════════════════════════════
// Die Bandmitte folgt der eingefrorenen Gitterregel
// ═════════════════════════════════════════════════════════════════════════
//
// ⚠️ Der Broker rechnet die Frequenz aus der Regel des Gitters, statt eine
// 221 Werte lange Tabelle zu kopieren. Dieser Fall hält die Regel gegen die
// **committete** Datei — damit ist die Kopie vermieden UND die Übereinstimmung
// gemessen.
#[test]
fn bandmitte_folgt_dem_eingefrorenen_gitter() {
    use eqcop_broker::coordinator::proposal::bandmitte_hz;
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../eq-copilot/schemas/v3/bandgitter/nakama_1_24_oct_30_18k_v1.json");
    let gitter: Value =
        serde_json::from_slice(&std::fs::read(&pfad).expect("das Gitter liegt im Repo")).unwrap();
    let mitten = gitter["mitten_hz"]["dezimal"]
        .as_array()
        .expect("die Mitten stehen dezimal in der Datei");
    assert_eq!(mitten.len(), 221);
    for (index, wert) in mitten.iter().enumerate() {
        let soll: f64 = wert.as_str().unwrap().parse().unwrap();
        let ist = bandmitte_hz(index as u32);
        assert!(
            (ist - soll).abs() < 1e-9 * soll.max(1.0),
            "Band {index}: Regel {ist} gegen Gitter {soll}"
        );
    }
    // Die Raender von beiden Seiten.
    assert!((bandmitte_hz(0) - 30.80197495750586).abs() < 1e-9);
    assert!(bandmitte_hz(220) > 17_000.0 && bandmitte_hz(220) < 18_000.0);
}

// ═════════════════════════════════════════════════════════════════════════
// M-49 · geschuetzte_eigenschaft_ist_harte_constraint
// ═════════════════════════════════════════════════════════════════════════
//
// Ein Schutz im Zielband entfernt den Eingriff **vor** der Gewichtung — er
// ist kein Gewicht, das ein guter Rang überstimmen könnte. Und die
// geschützten Eigenschaften stehen als Constraint im Vorschlag, in derselben
// Form wie der Intent.
#[test]
fn geschuetzte_eigenschaft_ist_harte_constraint() {
    use eqcop_broker::coordinator::proposal::{proposal, Proposallage};
    let c = coordinator();
    let adressen = buehne(&c, 12);
    let befund = c.befunde_sicht(&hex(0x11), &hex(0x22))[0].clone();

    let lage = |intent: Option<Value>| {
        let mut l = Proposallage {
            ziel_instanz: hex(1),
            base_revision: 0,
            intent: None,
            // Mit Capability — sonst entstuende ohnehin kein Eingriff, und der
            // Schutz haette nichts zu verhindern.
            capability_vorhanden: true,
            messbare_guardrails: Stopbedingung::ALLE.to_vec(),
            usergrenze_db: None,
            session_epoch: hex(0x22),
            metrics_version: 1,
        };
        if let Some(_) = intent {
            let mut bestand = eqcop_broker::coordinator::IntentBestand {
                vollstaendig: true,
                revision: 1,
                ..Default::default()
            };
            bestand
                .schutzangaben
                .insert(eqcop_broker::coordinator::SchutzangabeSpiegel {
                    quelle_id: adressen[1].instance_id.clone(),
                    eigenschaft: "band".into(),
                    band: Some((befund.band_hz.von, befund.band_hz.bis)),
                });
            l.intent = Some(bestand);
        }
        l
    };

    // Die GEGENPROBE zuerst: ohne Schutz entsteht ein Eingriff.
    let ohne = proposal(&befund, &lage(None))
        .expect("WN-04: die Buehne fuehrt eine benannte Passage, also entsteht ein Objekt");
    assert!(
        ohne.action.ist_eingriff(),
        "ohne Schutz entsteht ein Eingriff ({:?})",
        ohne.action
    );
    // Und mit Schutz nicht mehr — das ist eine HARTE Constraint, kein Gewicht.
    let mit = proposal(&befund, &lage(Some(json!({}))))
        .expect("WN-04: die Buehne fuehrt eine benannte Passage, also entsteht ein Objekt");
    assert!(
        !mit.action.ist_eingriff(),
        "ein geschuetztes Band traegt keinen Eingriff: {:?}",
        mit.action
    );
    // Und der Schutz steht als Constraint IM Vorschlag.
    assert!(
        mit.protected_traits
            .iter()
            .any(|s| s.quelle_id == adressen[1].instance_id && s.eigenschaft == "band"),
        "die geschuetzte Eigenschaft fehlt im Vorschlag: {:?}",
        mit.protected_traits
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-50 · stop_if_auf_nicht_messbarem_guardrail_ist_nicht_handelbar
// ═════════════════════════════════════════════════════════════════════════
//
// `guardrail_loudness_db` und `guardrail_peak_db` sind heute hart `None`
// (NAK-204), `seitenanteil_db` erreicht den Broker strukturell nie (NAK-193).
// Ein Vorschlag, dessen Stopbedingung darauf zeigt, ist **nicht handelbar** —
// und meldet das, statt so zu tun, als wäre der Guardrail grün.
#[test]
fn stop_if_auf_nicht_messbarem_guardrail_ist_nicht_handelbar() {
    let c = coordinator();
    let _ = buehne(&c, 12);
    let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    let v = vorschlaege.first().expect("ein Vorschlag");

    // Die Buehne erzeugt einen Bandpegel-Befund; seine Stopbedingung ist
    // `guardrail_loudness` — und die ist heute NICHT messbar.
    assert!(
        v.stop_if.contains(&Stopbedingung::GuardrailLoudness),
        "{:?}",
        v.stop_if
    );
    assert_eq!(
        v.action,
        Aktion::MoreData,
        "M-50 woertlich: nicht handelbar, und die Meldung ist MORE DATA -          nicht eine Ersatzaktion, die den Abbruch als ueberwacht ausgibt"
    );
    assert!(!v.action.ist_eingriff());
    // ⚠️ Der Guardrail bleibt trotzdem in `stop_if`. Ihn wegzulassen hiesse,
    // den Abbruch stillschweigend zu streichen.
    assert!(!v.stop_if.is_empty());
    // Und es geht KEIN Angebot hinaus.
    assert_eq!(c.draft_offers_zaehler(), 0);
}

// ═════════════════════════════════════════════════════════════════════════
// M-51 · jede_zahl_hat_feld_evidenz_und_generatorversion
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn jede_zahl_hat_feld_evidenz_und_generatorversion() {
    use eqcop_broker::coordinator::GENERATORVERSION;
    let c = coordinator();
    let _ = buehne(&c, 12);
    let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    let v = vorschlaege.first().expect("ein Vorschlag");
    assert_eq!(v.generatorversion, GENERATORVERSION);
    assert!(v.generatorversion >= 1);
    assert!(!v.evidence_ids.is_empty());
    // Jede Zahl im Wire-Objekt steht in einem benannten Feld — es gibt keine
    // Zahl ohne Namen.
    let wire = Coordinator::proposal_json_fuer_test(v);
    fn zahlen_haben_namen(wert: &Value, pfad: &str) {
        match wert {
            Value::Number(_) => assert!(!pfad.is_empty(), "eine Zahl ohne Feld"),
            Value::Object(o) => {
                for (k, v) in o {
                    zahlen_haben_namen(v, k);
                }
            }
            Value::Array(a) => {
                for v in a {
                    zahlen_haben_namen(v, pfad);
                }
            }
            _ => {}
        }
    }
    zahlen_haben_namen(&wire, "wurzel");
    assert!(wire["generatorversion"].is_number());
}

// ═════════════════════════════════════════════════════════════════════════
// M-52 · veraltet_ungueltig_capability_erreichen_keine_probe
// ═════════════════════════════════════════════════════════════════════════
//
// Drei Fälle, ein Ergebnis: kein `draft_offer`. Die Sperre liegt im Datenweg —
// das Angebot wird gar nicht erst gebildet.
#[test]
fn veraltet_ungueltig_capability_erreichen_keine_probe() {
    use eqcop_broker::coordinator::proposal::{darf_draft_offer, proposal, Proposallage};
    let c = coordinator();
    let adressen = buehne(&c, 12);
    let befund = c.befunde_sicht(&hex(0x11), &hex(0x22))[0].clone();
    let lage = |capability: bool, revision: u64| Proposallage {
        ziel_instanz: hex(1),
        base_revision: revision,
        intent: None,
        capability_vorhanden: capability,
        messbare_guardrails: Stopbedingung::ALLE.to_vec(),
        usergrenze_db: None,
        session_epoch: hex(0x22),
        metrics_version: 1,
    };

    // Die GEGENPROBE: ein handelbarer Befund mit Capability und Passage WUERDE
    // ein Angebot ergeben. Ohne sie waeren die drei Sperren unten nicht von
    // „hier passiert ohnehin nichts" zu unterscheiden.
    let mut mit_passage = befund.clone();
    mit_passage.passage_id = Some(hex(0x501));
    let offen = proposal(&mit_passage, &lage(true, 0))
        .expect("WN-04: die Buehne fuehrt eine benannte Passage, also entsteht ein Objekt");
    assert!(offen.action.ist_eingriff());
    assert!(
        darf_draft_offer(&mit_passage, &offen, &lage(true, 0)),
        "die Gegenprobe: so entstuende ein Angebot"
    );

    // (a) CAPABILITY fehlt — in P5 immer.
    let ohne_cap = proposal(&mit_passage, &lage(false, 0))
        .expect("WN-04: die Buehne fuehrt eine benannte Passage, also entsteht ein Objekt");
    assert!(!darf_draft_offer(&mit_passage, &ohne_cap, &lage(false, 0)));

    // (b) VERALTET: der Befund ist `stale`.
    let mut veraltet = mit_passage.clone();
    veraltet.zustand = eqcop_broker::coordinator::Befundzustand::Stale;
    assert!(
        !darf_draft_offer(&veraltet, &offen, &lage(true, 0)),
        "ein veralteter Befund bietet nichts an"
    );

    // (c) UNGUELTIG: die `base_revision` des Vorschlags passt nicht mehr.
    assert!(
        !darf_draft_offer(&mit_passage, &offen, &lage(true, 7)),
        "eine veraltete base_revision bietet nichts an"
    );

    // Und am Produktpfad geht in P5 nie ein Angebot hinaus.
    assert_eq!(c.draft_offers_zaehler(), 0);
    let _ = adressen;
}

// ═════════════════════════════════════════════════════════════════════════
// M-53 · hard_caps_und_engeres_userbudget_werden_nie_ueberschritten
// ═════════════════════════════════════════════════════════════════════════
//
// Eine EIGENSCHAFTSPRÜFUNG über zufällige Eingänge: kein erzeugter Vorschlag
// überschreitet je ein Hard Cap, und eine **engere Usergrenze gewinnt
// immer** — nie umgekehrt.
#[test]
fn hard_caps_und_engeres_userbudget_werden_nie_ueberschritten() {
    use eqcop_broker::coordinator::proposal::{
        proposal, Proposallage, BUDGET_DYNAMISCH_HARD_CAP_DB, BUDGET_EQ_BAENDER_MAX,
        BUDGET_STATISCH_DB,
    };
    let c = coordinator();
    let _ = buehne(&c, 12);
    let befund = c.befunde_sicht(&hex(0x11), &hex(0x22))[0].clone();

    // Ein deterministischer Wuerfel — dieselbe Bauform wie im Bootstrap:
    // reproduzierbar, damit ein Fehlschlag wiederholbar ist.
    let mut zustand: u64 = 0x2026_0906;
    let mut naechste = || {
        zustand ^= zustand << 13;
        zustand ^= zustand >> 7;
        zustand ^= zustand << 17;
        zustand
    };
    let mut eingriffe = 0usize;
    for runde in 0..500 {
        let usergrenze = match naechste() % 4 {
            0 => None,
            1 => Some((naechste() % 6000) as f64 / 1000.0),
            2 => Some(0.0),
            _ => Some(100.0),
        };
        let lage = Proposallage {
            ziel_instanz: hex(1),
            base_revision: naechste() % 8,
            intent: None,
            capability_vorhanden: naechste() % 2 == 0,
            messbare_guardrails: Stopbedingung::ALLE.to_vec(),
            usergrenze_db: usergrenze,
            session_epoch: hex(0x22),
            metrics_version: 1,
        };
        let v = proposal(&befund, &lage)
            .expect("WN-04: die Buehne fuehrt eine benannte Passage, also entsteht ein Objekt");
        assert!(
            v.budgets_gehalten(usergrenze),
            "Runde {runde}: Budget verletzt bei {usergrenze:?}: {:?}",
            v.allowed_bounds
        );
        assert!(v.baender() <= BUDGET_EQ_BAENDER_MAX, "Runde {runde}");
        if let Some(g) = v.parameters.max_gain_db {
            let deckel = match v.action {
                Aktion::DynamicEqCut => BUDGET_DYNAMISCH_HARD_CAP_DB,
                _ => BUDGET_STATISCH_DB,
            };
            assert!(g.abs() <= deckel + 1e-9, "Runde {runde}: Hard Cap");
            // Und die ENGERE Usergrenze gewinnt.
            if let Some(u) = usergrenze {
                assert!(
                    g.abs() <= u.max(0.0).min(deckel) + 1e-9,
                    "Runde {runde}: die Usergrenze {u} wurde ueberschritten mit {g}"
                );
            }
        }
        // Und in P5 bleibt jede Aktion `manual`, egal was der Wuerfel sagt.
        assert_eq!(v.execution, Ausfuehrung::Manual);
        if v.action.ist_eingriff() {
            eingriffe += 1;
        }
    }
    // ⚠️ Die Gegenprobe zur ganzen Pruefung: ein Property-Test, der nie einen
    // Eingriff erzeugt, prueft kein Budget. Er waere gruen, auch wenn jede
    // Grenze gerissen waere.
    assert!(
        eingriffe > 50,
        "die Pruefung hat nur {eingriffe} Eingriffe gesehen - sie misst nichts"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-54 · evidenz_ids_und_intent_revision_sind_pflicht
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn evidenz_ids_und_intent_revision_sind_pflicht() {
    let c = coordinator();
    let adressen = buehne(&c, 12);
    let intent = json!({
        "type": "intent_update",
        "adresse": adressen[0],
        "session_epoch": adressen[0].session_epoch,
        "vollstaendig": true,
        "bestand_revision": 7,
        "intents": []
    });
    c.p1("main", &serde_json::to_vec(&intent).unwrap());
    // Ein neuer Beleg loest die Neurechnung aus — jetzt laeuft der Solver
    // gegen Revision 7.
    c.p1("main", &evidenz(&adressen[0], 12, 44_108_200 + 12 * 512, 9.0));
    c.p1("sonde0", &evidenz(&adressen[1], 200, 44_108_200 + 12 * 512, 9.0));

    let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    let v = vorschlaege.first().expect("ein Vorschlag");
    assert!(!v.evidence_ids.is_empty(), "ohne Evidenz kein Objekt");
    assert_eq!(
        v.intent_revision, 7,
        "die Revision, GEGEN DIE der Solver gelaufen ist"
    );
    // Die Evidenz-IDs kommen aus dem Befund und werden nicht neu gesammelt.
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert_eq!(v.evidence_ids, befunde[0].evidence_ids);
}

// ═════════════════════════════════════════════════════════════════════════
// M-63 · manueller_busvorschlag_ist_ein_proposal
// ═════════════════════════════════════════════════════════════════════════
//
// Der Busvorschlag ist **dieselbe Struktur** wie jedes andere Proposal — kein
// zweites Objekt und kein Fremdwerkzeugkommando.
#[test]
fn manueller_busvorschlag_ist_ein_proposal() {
    use eqcop_broker::coordinator::proposal::{proposal, Proposallage};
    let c = coordinator();
    let _ = buehne(&c, 12);
    let befund = c.befunde_sicht(&hex(0x11), &hex(0x22))[0].clone();

    // ⚠️ In P5 fehlt die CAPABILITY — und genau das laesst die manuelle
    // Anleitung uebrig. Die Guardrails sind hier alle messbar, damit M-50
    // nicht vorher greift; gemessen wird der Busvorschlag, nicht die
    // Guardrailkette.
    let lage = Proposallage {
        ziel_instanz: hex(1),
        base_revision: 0,
        intent: None,
        capability_vorhanden: false,
        messbare_guardrails: Stopbedingung::ALLE.to_vec(),
        usergrenze_db: None,
        session_epoch: hex(0x22),
        metrics_version: 1,
    };
    let v = proposal(&befund, &lage)
        .expect("WN-04: die Buehne fuehrt eine benannte Passage, also entsteht ein Objekt");
    assert_eq!(
        v.action,
        Aktion::ManualGuidance,
        "ohne Capability bleibt die manuelle Anleitung"
    );
    // Er ist DIESELBE Struktur: Ziel, Grenzen, Hoerziel, Stopbedingung,
    // Rueckweg — nur die Passage fehlt, weil diese Buehne keine fuehrt.
    for (feld, belegt) in v.gate_felder_vollstaendig() {
        if feld == "passage_id" {
            continue;
        }
        assert!(belegt, "auch der manuelle Vorschlag traegt {feld}");
    }
    assert_eq!(v.execution, Ausfuehrung::Manual);
    assert_eq!(v.revert, Rueckweg::ManualOnly, "er nimmt sich selbst zurueck");
    // Und er ist KEIN Fremdwerkzeugkommando: das Objekt hat kein Feld dafuer.
    let wire = Coordinator::proposal_json_fuer_test(&v);
    for fremd in ["werkzeug", "tool", "kommando", "command", "plugin"] {
        assert!(wire.get(fremd).is_none(), "{fremd} steht im Vorschlag");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-10 · rollenaenderung_macht_den_vorschlag_stale
// ═════════════════════════════════════════════════════════════════════════
//
// §37.3: eine gestiegene Intent-Revision entwertet den Vorschlag mit seinem
// Befund. §7.1 hatte die Zeile ausdrücklich hierher vertagt.
#[test]
fn rollenaenderung_macht_den_vorschlag_stale() {
    let c = coordinator();
    let adressen = buehne(&c, 12);
    let vorher = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    assert!(!vorher.is_empty());
    assert_eq!(
        c.befunde_sicht(&hex(0x11), &hex(0x22))[0].zustand,
        eqcop_broker::coordinator::Befundzustand::ReadyToSend
    );

    let intent = json!({
        "type": "intent_update",
        "adresse": adressen[0],
        "session_epoch": adressen[0].session_epoch,
        "vollstaendig": true,
        "bestand_revision": 3,
        "intents": [{
            "quelle_id": adressen[1].instance_id,
            "rolle": "traegt",
            "revision": 1,
            "herkunft": "user",
            "konfidenz": 1.0
        }]
    });
    c.p1("main", &serde_json::to_vec(&intent).unwrap());

    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert_eq!(
        befunde[0].zustand,
        eqcop_broker::coordinator::Befundzustand::Stale,
        "der Befund unter dem Vorschlag ist entwertet"
    );
    assert!(
        !befunde[0].zustand.erlaubt_draft(),
        "und damit ist der Vorschlag nicht mehr handelbar"
    );
    assert_eq!(c.draft_offers_zaehler(), 0);
}

// ═════════════════════════════════════════════════════════════════════════
// SONDE-014 ETAPPE I · die Produktinvarianten am Vorschlagspfad
// ═════════════════════════════════════════════════════════════════════════

/// Sammelt, was der Broker an wen geschrieben hätte.
///
/// Sie steht hier, weil M-78 eine **Abwesenheit** misst: an die Sonde geht
/// nichts. Eine Abwesenheit ohne Gegenprobe ist wertlos — deshalb hält
/// dieselbe Probe fest, dass der Main sehr wohl beliefert wird.
#[derive(Default)]
struct Pushprobe {
    geschrieben: std::sync::Mutex<Vec<(String, Value)>>,
}

impl eqcop_broker::coordinator::SessionPush for Pushprobe {
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
        let wert: Value = serde_json::from_slice(payload).unwrap_or(Value::Null);
        self.geschrieben
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .push((link_id.to_owned(), wert));
        true
    }
}

/// **M-78** — nichts Ungefragtes: ein Proposal bleibt ohne Useraktion
/// folgenlos.
///
/// Gemessen an dem, was die **Sonde** erlebt. Der Advisor bildet auf dieser
/// Bühne Befunde und Vorschläge; an der Probe darf davon nichts ankommen und
/// nichts sich ändern. Die Gegenprobe steht daneben: der Main, der die
/// Sitzung abonniert, bekommt sehr wohl seine Schnitte — der Pushweg ist also
/// offen, und das Schweigen zur Sonde hin ist echt.
#[test]
fn ein_proposal_ohne_useraktion_bleibt_folgenlos() {
    let c = coordinator();
    let push = Arc::new(Pushprobe::default());
    c.session_push_setzen(push.clone());
    let adressen = buehne(&c, 12);
    let sonde = &adressen[1];

    assert!(
        !c.befunde_sicht(&hex(0x11), &hex(0x22)).is_empty(),
        "die Bühne trägt Befunde - sonst misst der Fall nichts"
    );
    assert!(
        !c.vorschlaege_sicht(&hex(0x11), &hex(0x22)).is_empty(),
        "und Vorschläge"
    );

    let evidenz_vorher = c.evidenz_sicht(&sonde.instance_id);
    assert!(evidenz_vorher.is_some(), "die Sonde hat gemessen");

    // 1. Kein Angebot ist hinausgegangen, und keines ist auch nur geschuldet.
    assert_eq!(c.draft_offers_zaehler(), 0, "kein `draft_offer` in P5");
    assert_eq!(c.draft_offer_schuld_zaehler(), 0);
    // 2. Keine Evidenz wurde zurückgenommen oder gesperrt.
    assert_eq!(c.invalidierungen_zaehler(), 0);
    assert_eq!(c.evidenz_ausgeschlossen_zaehler(), 0);
    // 3. Kein Experiment ist begonnen worden.
    assert!(
        c.experiment_sicht(&hex(0xdead)).is_none(),
        "der Advisor beginnt keinen Versuch"
    );
    // 4. An die SONDE ging kein einziger Frame.
    let geschrieben = push
        .geschrieben
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .clone();
    assert!(
        !geschrieben.iter().any(|(link, _)| link == "sonde0"),
        "ein_proposal_ohne_useraktion_bleibt_folgenlos - an die Probe geht nichts"
    );
    // 5. Und der Messstand der Sonde ist unverändert - kein Eingriff, keine
    //    Markierung, keine Zustandsänderung.
    let evidenz_nachher = c.evidenz_sicht(&sonde.instance_id);
    assert_eq!(
        evidenz_vorher.map(|e| e.evidence_id.clone()),
        evidenz_nachher.map(|e| e.evidence_id.clone()),
        "der Advisor fasst die Evidenz der Probe nicht an"
    );
}

/// **M-79** — keine KI-Erklärschicht: jeder Text entsteht aus festen
/// Bausteinen, und **keine Zahl darin ist erfunden**.
///
/// Der Rotbeweis der Zeile lautet wörtlich „ein Textbaustein trägt eine Zahl,
/// die in keinem Feld steht". Genau das misst der Fall: jede Zahl, die in
/// `likely_cause`, `smallest_test` oder `listen_for` vorkommt, muss in einem
/// **Feld desselben Befunds** stehen. Der Extraktor läuft daneben gegen einen
/// mutierten Text — sonst wäre er auch dann grün, wenn er gar nichts fände.
#[test]
fn text_entsteht_aus_bausteinen_und_erfindet_keine_zahl() {
    /// Jede zusammenhängende Ziffernfolge eines Textes.
    fn zahlen(text: &str) -> Vec<String> {
        let mut aus = Vec::new();
        let mut laufend = String::new();
        for z in text.chars() {
            if z.is_ascii_digit() {
                laufend.push(z);
            } else if !laufend.is_empty() {
                aus.push(std::mem::take(&mut laufend));
            }
        }
        if !laufend.is_empty() {
            aus.push(laufend);
        }
        aus
    }

    let c = coordinator();
    let _ = buehne(&c, 12);
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert!(!befunde.is_empty());

    // Die geschlossene Menge der `smallest_test`-Sätze: sie kommt aus der
    // Abbildung am Enum, nicht aus einer zweiten Liste hier.
    let saetze: Vec<&'static str> = NaechsterTest::ALLE.iter().map(|n| n.satz()).collect();

    let mut geprueft = 0usize;
    for b in &befunde {
        assert!(
            saetze.contains(&b.smallest_test.as_str()),
            "`smallest_test` steht nicht in der geschlossenen Menge: {}",
            b.smallest_test
        );
        // Die Felder, in denen eine Zahl dieses Befunds stehen darf. Die
        // Kennung der Quelle steht abgekuerzt im Satz — deshalb zaehlt fuer
        // sie der TEILSTRING von `candidate_source`, fuer die Bandraender die
        // exakte Zahl.
        let felder: Vec<String> = vec![
            b.band_hz.von.to_string(),
            b.band_hz.bis.to_string(),
            b.intent_revision.to_string(),
        ];
        for text in [&b.likely_cause, &b.smallest_test, &b.listen_for] {
            for zahl in zahlen(text) {
                geprueft += 1;
                let steht_im_feld = felder.contains(&zahl)
                    || b.candidate_source.contains(&zahl)
                    || b.passage_id.as_deref().is_some_and(|p| p.contains(&zahl));
                assert!(
                    steht_im_feld,
                    "text_entsteht_aus_bausteinen_und_erfindet_keine_zahl - die Zahl {zahl} aus {text:?} steht in keinem Feld"
                );
            }
        }
    }
    assert!(
        geprueft > 0,
        "die Texte tragen wirklich Zahlen - sonst waere der Riegel trivial"
    );

    // Die Gegenprobe: derselbe Extraktor an einem erfundenen Wert.
    let erfunden = "Der Master verliert 7,5 dB im Bandbereich 40..96.";
    let quelle = befunde[0].candidate_source.clone();
    let felder = ["40".to_string(), "96".to_string()];
    let erfundene: Vec<String> = zahlen(erfunden)
        .into_iter()
        .filter(|z| !felder.contains(z) && !quelle.contains(z))
        .collect();
    assert_eq!(
        erfundene,
        vec!["7".to_string(), "5".to_string()],
        "der Extraktor findet GENAU die erfundene Zahl - sonst misst er nichts"
    );
}

/// **M-82** — NaN-Ehrlichkeit über den ganzen Weg: kein nicht-endlicher Wert
/// erreicht die Leitung.
///
/// Die Zusage steht am ERZEUGER („nicht-endliche Werte werden beim Erzeugen
/// verriegelt und gezählt, nie sanitisiert auf die Leitung gelassen"), und
/// deshalb wird am Ausgang gemessen: der serialisierte Sitzungsschnitt trägt
/// weder `NaN` noch `Infinity` noch ein `e`-Exponentenungetüm, und jede
/// gelesene Zahl ist endlich. Die Gegenprobe hält fest, dass wirklich Zahlen
/// darin stehen.
#[test]
fn kein_nicht_endlicher_wert_erreicht_die_leitung() {
    fn zahlen_sammeln(wert: &Value, aus: &mut Vec<f64>) {
        match wert {
            Value::Number(n) => {
                aus.push(n.as_f64().unwrap_or(f64::NAN));
            }
            Value::Array(a) => a.iter().for_each(|w| zahlen_sammeln(w, aus)),
            Value::Object(o) => o.values().for_each(|w| zahlen_sammeln(w, aus)),
            _ => {}
        }
    }

    let c = coordinator();
    let _ = buehne(&c, 12);
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert!(!befunde.is_empty());

    let schnitt = c.session_snapshot_json(&hex(0x11), &hex(0x22));
    let text = String::from_utf8(schnitt.clone()).expect("der Schnitt ist UTF-8");
    // Genau die drei Token, die ein Serialisierer fuer nicht-endliche Zahlen
    // schreibt. Eine Suche nach `nan` oder `inf` als Teilwort faende jeden
    // Bezeichner, der sie zufaellig enthaelt, und waere damit keine Messung,
    // sondern ein Zufallstreffer.
    for wort in ["NaN", "Infinity", "-Infinity"] {
        assert!(
            !text.contains(wort),
            "kein_nicht_endlicher_wert_erreicht_die_leitung - {wort} steht im Sitzungsschnitt"
        );
    }
    let wert: Value = serde_json::from_slice(&schnitt).expect("der Schnitt ist JSON");
    // Der Schnitt traegt die Befunde wirklich - sonst pruefte der Riegel
    // einen Text, in dem gar keine Befundzahl vorkommen kann.
    assert!(
        wert["findings"].as_array().is_some_and(|f| !f.is_empty()),
        "der Sitzungsschnitt traegt die Befunde"
    );
    let mut alle = Vec::new();
    zahlen_sammeln(&wert, &mut alle);
    assert!(
        alle.len() >= 20,
        "der Schnitt traegt wirklich Zahlen ({}) - sonst waere der Riegel trivial",
        alle.len()
    );
    for z in &alle {
        assert!(z.is_finite(), "eine nicht-endliche Zahl auf der Leitung: {z}");
    }

    // Und die zweite Haelfte der Zeile: ein Band ohne Messung traegt 0 mit
    // `gueltig = false`, nie NaN.
    for b in &befunde {
        if let Some(m) = &b.maskierung {
            assert!(m.wert_db.is_finite(), "der Maskierungswert ist endlich");
            if !m.gueltig {
                assert_eq!(m.wert_db, 0.0, "ohne Messung steht 0, nicht NaN");
            }
        }
        assert!(b.confidence.score.is_finite());
        assert!((0.0..=1.0).contains(&b.confidence.score));
    }
}
