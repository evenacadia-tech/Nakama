//! Die Coordinator-Verdrahtung von Intent und Assistentenschritt im
//! PRODUKTPFAD (SONDE-014 Etappe B, Matrixzeilen M-85 bis M-89).
//!
//! ── WARUM DIESES BEIN DEN WEG MISST UND NICHT DAS MODUL ───────────────────
//!
//! Risiko R1 des Bauplans, woertlich aus der Wiederpruefung 1 der Etappe 2 von
//! SONDE-013: „die Runde 1 hat Funktionen gebaut und Tests geschrieben, die
//! das Modul direkt aufrufen — der Produktpfad war an vielen Stellen nicht
//! geschlossen." Jeder Fall hier geht deshalb durch `p1_mit_minor`, also
//! durch dieselbe Senke, die der echte Transport ruft. Ein Fall, der
//! `intent_update_json` direkt riefe, koennte gruen sein, waehrend die Senke
//! die Familie gar nicht kennt.
//!
//! Die Nutzlasten kommen aus dem COMMITTETEN Fixturekorpus. Eine zweite
//! Wire-Form von Hand waere eine zweite Wahrheit neben dem Korpus — dieselbe
//! Regel wie in `coordinator_model.rs` und `sonde013_verdrahtung.rs`.

use eqcop_broker::coordinator::{Coordinator, IntentAbweisung, ManualClock, SchrittAbweisung};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
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

fn hello(adresse: Adresse) -> HelloControl {
    HelloControl {
        typ: "hello".into(),
        connection_kind: "control".into(),
        protocol: 3,
        plugin_version: "test".into(),
        plugin_kind: "main".into(),
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

fn coordinator() -> Coordinator {
    Coordinator::mit_uhr(Arc::new(ManualClock::default()), hex(0xbeef))
}

fn anmelden(c: &Coordinator, link: &str, h: &HelloControl) {
    let ausgang = c.control_hello_registrieren(link, h);
    assert!(ausgang.angenommen, "{:?}", ausgang.grund);
    let _ = c.resync_bestaetigen(link, 0);
}

/// Die Grundform aus dem committeten Korpus, mit der Adresse des Links.
fn fixture(name: &str) -> Value {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../eq-copilot/fixtures/v3/gueltig")
        .join(format!("{name}.json"));
    serde_json::from_slice(&std::fs::read(&pfad).expect("Fixture liegt im Korpus"))
        .expect("Fixture ist JSON")
}

fn mit_adresse(mut wert: Value, a: &Adresse) -> Value {
    wert["adresse"] = serde_json::to_value(a).unwrap();
    wert["session_epoch"] = Value::String(a.session_epoch.clone());
    wert
}

fn bytes(wert: &Value) -> Vec<u8> {
    serde_json::to_vec(wert).unwrap()
}

/// Der PRODUKTWEG: durch die Senke, nicht am Modul vorbei.
fn ueber_senke(c: &Coordinator, link: &str, wert: &Value) {
    c.p1(link, &bytes(wert));
}

// ═════════════════════════════════════════════════════════════════════════
// M-85 · intent_update_koalesziert_je_quelle_und_scope
// ═════════════════════════════════════════════════════════════════════════
//
// Die Zusage hat zwei Haelften, und beide fallen einzeln: zwei Passagen
// DERSELBEN Quelle sind zwei Objekte und verdraengen sich nicht (M-08), und
// eine AELTERE Revision ueberschreibt nach der Koaleszierung NIE eine
// juengere.
#[test]
fn intent_update_koalesziert_je_quelle_und_scope() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));

    let quelle = hex(0xa1);
    let passage_eins = hex(0x501);
    let passage_zwei = hex(0x502);

    let voll = mit_adresse(fixture("intent-update-leerer-bestand-mit-marke"), &a);
    ueber_senke(&c, "link-a", &voll);
    assert!(
        c.darf_rechnen(&a.project_binding_id, &a.session_epoch),
        "die Vollstaendigkeitsmarke kommt an"
    );

    let einzeln = |quelle_id: &str, passage: &str, rolle: &str, revision: i64, bestand: i64| {
        let mut w = mit_adresse(fixture("intent-update-einzelne-fortschreibung"), &a);
        w["vollstaendig"] = Value::Bool(false);
        w["bestand_revision"] = json!(bestand);
        w["intents"] = json!([{
            "quelle_id": quelle_id,
            "passage_id": passage,
            "rolle": rolle,
            "revision": revision,
            "herkunft": "user",
            "konfidenz": 1.0
        }]);
        w
    };

    ueber_senke(&c, "link-a", &einzeln(&quelle, &passage_eins, "fuehrt", 1, 1));
    ueber_senke(
        &c,
        "link-a",
        &einzeln(&quelle, &passage_zwei, "begleitet", 1, 2),
    );

    let bestand = c.intent_sicht(&a.project_binding_id, &a.session_epoch);
    assert_eq!(
        bestand.intents.len(),
        2,
        "zwei Passagenrollen derselben Quelle stehen NEBENEINANDER"
    );
    assert_eq!(
        bestand.wirkend(&quelle, &passage_eins).map(|s| s.rolle.as_str()),
        Some("fuehrt")
    );
    assert_eq!(
        bestand.wirkend(&quelle, &passage_zwei).map(|s| s.rolle.as_str()),
        Some("begleitet")
    );

    // Die Revisionsordnung ist TOTAL: eine aeltere Bestandsrevision wird
    // abgewiesen, nicht eingemischt.
    let alt = einzeln(&quelle, &passage_eins, "traegt", 9, 1);
    assert_eq!(
        c.intent_update_json_grund_fuer_test("link-a", &bytes(&alt)),
        Err(IntentAbweisung::AeltereRevision)
    );
    assert_eq!(
        c.intent_sicht(&a.project_binding_id, &a.session_epoch)
            .wirkend(&quelle, &passage_eins)
            .map(|s| s.rolle.as_str()),
        Some("fuehrt"),
        "der juengere Wert steht weiter"
    );

    // Gegenprobe: eine JUENGERE Revision wirkt.
    ueber_senke(&c, "link-a", &einzeln(&quelle, &passage_eins, "traegt", 2, 3));
    assert_eq!(
        c.intent_sicht(&a.project_binding_id, &a.session_epoch)
            .wirkend(&quelle, &passage_eins)
            .map(|s| s.rolle.as_str()),
        Some("traegt")
    );
}

/// M-85, Consumerregel 5: eine Teilmeldung traegt genau EINEN Eintrag. Der
/// P1-Schluessel adressiert genau ein Objekt; eine Sammelnachricht unter
/// demselben Schluessel koennte fremde Objekte verdraengen.
#[test]
fn teilmeldung_mit_mehreren_objekten_wird_abgewiesen() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));

    let mut w = mit_adresse(fixture("intent_update"), &a);
    w["vollstaendig"] = Value::Bool(false);
    assert_eq!(
        c.intent_update_json_grund_fuer_test("link-a", &bytes(&w)),
        Err(IntentAbweisung::TeilmeldungNichtEinzeln),
        "drei Listen mit je einem Eintrag sind drei Objekte, nicht eines"
    );

    // Gegenprobe: mit der Vollstaendigkeitsmarke ist dieselbe Nachricht
    // zulaessig - sie ist dann kein Ausschnitt, sondern der Bestand.
    let voll = mit_adresse(fixture("intent_update"), &a);
    assert!(c.intent_update_json_grund_fuer_test("link-a", &bytes(&voll)).is_ok());
}

// ═════════════════════════════════════════════════════════════════════════
// M-86 · keine_rechnung_vor_der_vollstaendigkeitsmarke
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn keine_rechnung_vor_der_vollstaendigkeitsmarke() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));

    assert!(
        !c.darf_rechnen(&a.project_binding_id, &a.session_epoch),
        "ohne jede Meldung rechnet der Broker nicht"
    );

    // Eine Teilmeldung hebt die Sperre NICHT. Genau daran haengt die Zusage:
    // ein Veto darf durch Transportverlust nie verschwinden, und ein
    // Ausschnitt beweist nicht, dass keins existiert.
    let mut teil = mit_adresse(fixture("intent-update-einzelne-fortschreibung"), &a);
    teil["vollstaendig"] = Value::Bool(false);
    teil["bestand_revision"] = json!(1);
    ueber_senke(&c, "link-a", &teil);
    assert!(
        !c.darf_rechnen(&a.project_binding_id, &a.session_epoch),
        "eine Teilmeldung ist keine Vollstaendigkeitsmarke"
    );

    // Der LEERE Bestand mit Marke hebt sie. „Diese Sitzung hat keinen Intent"
    // ist eine Aussage, „ich habe nichts gehoert" ist keine.
    // Die Bestandsrevision steigt im Main monoton; ein vollstaendiger Bericht
    // traegt deshalb nie eine aeltere Zahl als eine schon gesehene
    // Fortschreibung. Genau das misst die Revisionsordnung ausserdem: eine
    // aeltere Marke wuerde ABGEWIESEN, nicht eingemischt.
    let mut leer = mit_adresse(fixture("intent-update-leerer-bestand-mit-marke"), &a);
    leer["bestand_revision"] = json!(2);
    ueber_senke(&c, "link-a", &leer);
    assert!(c.darf_rechnen(&a.project_binding_id, &a.session_epoch));
    assert!(
        c.intent_sicht(&a.project_binding_id, &a.session_epoch)
            .intents
            .is_empty(),
        "und der Bestand ist wirklich leer, nicht der Rest der Teilmeldung"
    );
}

/// M-76/M-86: das SITZUNGSENDE loescht den Spiegel. Eine gleichnamige neue
/// Sitzung darf keine fremde Vollstaendigkeitsmarke erben.
///
/// ⚠️ Das Ende eines LINKS ist noch kein Sitzungsende: ein Main darf sich neu
/// verbinden, und sein Client ueberlebt bis zur Eviction. Wuerde der Spiegel
/// schon beim Disconnect fallen, wuerde jeder Reconnect kurzzeitig auf einem
/// leeren Bestand rechnen - schlimmer als der Zustand, den die Zeile
/// verhindern soll. Gemessen wird deshalb der Weg, den das Produkt wirklich
/// geht: stale, Tombstone, Eviction.
#[test]
fn sitzungsende_loescht_den_spiegel() {
    let clock = Arc::new(ManualClock::default());
    let c = Coordinator::mit_uhr(clock.clone(), hex(0xbeef));
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));
    ueber_senke(
        &c,
        "link-a",
        &mit_adresse(fixture("intent-update-leerer-bestand-mit-marke"), &a),
    );
    ueber_senke(
        &c,
        "link-a",
        &mit_adresse(fixture("assistant_step_update"), &a),
    );
    assert!(c.darf_rechnen(&a.project_binding_id, &a.session_epoch));
    assert!(c.assistent_sicht(&a.project_binding_id, &a.session_epoch).is_some());

    c.control_ende("link-a");
    assert!(
        c.darf_rechnen(&a.project_binding_id, &a.session_epoch),
        "das Ende eines Links allein ist kein Sitzungsende - ein Reconnect          faende sonst einen leeren Bestand vor"
    );

    clock.vor(60_000);
    c.liveness_tick();
    c.liveness_tick();
    assert!(
        !c.darf_rechnen(&a.project_binding_id, &a.session_epoch),
        "nach dem letzten Client rechnet der Broker nicht auf einem Rest aus seinem Cache"
    );
    assert!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch).is_none(),
        "und der Schrittspiegel faellt GEMEINSAM mit dem Intentspiegel"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-88 · assistant_step_update_koalesziert_je_session
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn assistant_step_update_koalesziert_je_session() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));

    let schritt = |name: &str, revision: i64, offen: bool| {
        let mut w = mit_adresse(fixture("assistant_step_update"), &a);
        w["schritt"] = Value::String(name.into());
        w["revision"] = json!(revision);
        w["offen"] = Value::Bool(offen);
        w
    };

    // Ein Schrittwechsel OHNE Experimentabschluss und OHNE Userurteil - genau
    // der Fall, fuer den es vor E-11 keinen Weg gab.
    ueber_senke(&c, "link-a", &schritt("finding", 1, true));
    assert_eq!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.schritt),
        Some("finding".to_string()),
        "der Schritt erreicht den Spiegel ohne jedes Experimentereignis"
    );

    ueber_senke(&c, "link-a", &schritt("evidence", 2, true));
    assert_eq!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.schritt),
        Some("evidence".to_string())
    );

    // Die Revisionsordnung ist TOTAL.
    let alt = schritt("coverage", 1, true);
    assert_eq!(
        c.assistant_step_update_grund_fuer_test("link-a", &bytes(&alt)),
        Err(SchrittAbweisung::AeltereRevision)
    );
    assert_eq!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.schritt),
        Some("evidence".to_string()),
        "der juengere Schritt steht weiter"
    );

    // E-07/M-55: `preview` BLEIBT in der Zustandsmenge. Der Vertrag traegt
    // ihn; dass P5 keine Kante dorthin fuehrt, gehoert dem Main.
    ueber_senke(&c, "link-a", &schritt("preview", 3, false));
    assert_eq!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.schritt),
        Some("preview".to_string())
    );
}

// ═════════════════════════════════════════════════════════════════════════
// Die Consumerregeln, die das Schema nicht ausdruecken kann
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn consumerregeln_des_intents_fallen_einzeln() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));

    let grund = |w: &Value| c.intent_update_json_grund_fuer_test("link-a", &bytes(w));

    // Regel 1: das Bandintervall ist halboffen und nicht leer.
    let mut w = mit_adresse(fixture("intent_update"), &a);
    w["schutzangaben"] = json!([{"quelle_id": hex(0xa1), "eigenschaft": "band",
                                 "band": {"von": 40, "bis": 40}}]);
    assert_eq!(grund(&w), Err(IntentAbweisung::BandintervallLeer));

    // Regel 2, beide Richtungen: `band` genau dann, wenn die Eigenschaft es
    // ist. Beide Haelften fallen einzeln - eine Bandangabe ohne Intervall
    // benennt kein Band, und Attack mit Intervall behauptet eins.
    let mut w = mit_adresse(fixture("intent_update"), &a);
    w["schutzangaben"] = json!([{"quelle_id": hex(0xa1), "eigenschaft": "band"}]);
    assert_eq!(grund(&w), Err(IntentAbweisung::SchutzOhneBand));

    let mut w = mit_adresse(fixture("intent_update"), &a);
    w["schutzangaben"] = json!([{"quelle_id": hex(0xa1), "eigenschaft": "attack",
                                 "band": {"von": 0, "bis": 4}}]);
    assert_eq!(grund(&w), Err(IntentAbweisung::SchutzMitFremdemBand));

    // Regel 3: zwei verschiedene Quellen, je geordnetem Paar eine Beziehung.
    let mut w = mit_adresse(fixture("intent_update"), &a);
    w["beziehungen"] = json!([{"quelle_a": hex(0xa1), "quelle_b": hex(0xa1),
                               "art": "fuehrt_vor"}]);
    assert_eq!(grund(&w), Err(IntentAbweisung::Selbstbeziehung));

    let mut w = mit_adresse(fixture("intent_update"), &a);
    w["beziehungen"] = json!([
        {"quelle_a": hex(0xa1), "quelle_b": hex(0xb2), "art": "fuehrt_vor"},
        {"quelle_a": hex(0xa1), "quelle_b": hex(0xb2), "art": "gleichrangig"}
    ]);
    assert_eq!(grund(&w), Err(IntentAbweisung::DoppeltesPaar));

    // Regel 4: der `fuehrt_vor`-Teilgraph ist zyklenfrei (§37.4).
    let mut w = mit_adresse(fixture("intent_update"), &a);
    w["beziehungen"] = json!([
        {"quelle_a": hex(0xa1), "quelle_b": hex(0xb2), "art": "fuehrt_vor"},
        {"quelle_a": hex(0xb2), "quelle_b": hex(0xc3), "art": "fuehrt_vor"},
        {"quelle_a": hex(0xc3), "quelle_b": hex(0xa1), "art": "fuehrt_vor"}
    ]);
    assert_eq!(grund(&w), Err(IntentAbweisung::Zyklus));

    // Dieselbe Runde mit einer als `gleichrangig` MARKIERTEN Kante ist kein
    // Zyklus - das ist der von §37.2 verlangte Ausweg, kein Schlupfloch.
    let mut w = mit_adresse(fixture("intent_update"), &a);
    w["beziehungen"] = json!([
        {"quelle_a": hex(0xa1), "quelle_b": hex(0xb2), "art": "fuehrt_vor"},
        {"quelle_a": hex(0xb2), "quelle_b": hex(0xc3), "art": "fuehrt_vor"},
        {"quelle_a": hex(0xc3), "quelle_b": hex(0xa1), "art": "gleichrangig"}
    ]);
    assert!(grund(&w).is_ok());

    // Regel 6: Inhalt verlangt eine Revision.
    let mut w = mit_adresse(fixture("intent_update"), &a);
    w["bestand_revision"] = json!(0);
    assert_eq!(grund(&w), Err(IntentAbweisung::InhaltOhneRevision));
}

/// Dieselbe Kante wie beim `state_report` und beim `evidence_snapshot`: die
/// Adresse IM Payload muss die des sendenden Links sein. Ein Peer schreibt
/// sich seine Quelle nicht selbst zu.
#[test]
fn fremde_adresse_wird_abgewiesen() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    let fremd = adresse(1, 2, 99, 98);
    anmelden(&c, "link-a", &hello(a.clone()));

    let w = mit_adresse(fixture("intent_update"), &fremd);
    assert_eq!(
        c.intent_update_json_grund_fuer_test("link-a", &bytes(&w)),
        Err(IntentAbweisung::FremdeAdresse)
    );
    let s = mit_adresse(fixture("assistant_step_update"), &fremd);
    assert_eq!(
        c.assistant_step_update_grund_fuer_test("link-a", &bytes(&s)),
        Err(SchrittAbweisung::FremdeAdresse)
    );
}

/// E-02/M-04 auf der Brokerseite: dieselbe Regel wie im Main. Der Broker
/// rechnet mit dem Veto, das der User gesetzt hat - nicht mit einem eigenen.
#[test]
fn entmaskierungsveto_wirkt_im_spiegel_wie_im_main() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));

    let qa = hex(0xa1);
    let qb = hex(0xb2);
    let qc = hex(0xc3);

    let mut w = mit_adresse(fixture("intent_update"), &a);
    w["intents"] = json!([{"quelle_id": qa, "rolle": "verschmolzen", "revision": 1,
                           "herkunft": "user", "konfidenz": 1.0}]);
    w["schutzangaben"] = json!([]);
    w["beziehungen"] = json!([]);
    ueber_senke(&c, "link-a", &w);

    let bestand = c.intent_sicht(&a.project_binding_id, &a.session_epoch);
    assert!(!bestand.entmaskierung_erlaubt(&qa, &qb, ""), "globales Veto");
    assert!(!bestand.entmaskierung_erlaubt(&qc, &qa, ""), "auch gegen eine dritte Quelle");
    assert!(bestand.entmaskierung_erlaubt(&qb, &qc, ""), "ein Paar ohne A bleibt frei");

    // Die ausdrueckliche Beziehung hebt das Veto NUR fuer ihr Paar auf.
    let mut w2 = mit_adresse(fixture("intent_update"), &a);
    w2["bestand_revision"] = json!(5);
    w2["intents"] = json!([{"quelle_id": qa, "rolle": "verschmolzen", "revision": 1,
                            "herkunft": "user", "konfidenz": 1.0}]);
    w2["schutzangaben"] = json!([]);
    w2["beziehungen"] = json!([{"quelle_a": qa, "quelle_b": qb, "art": "fuehrt_vor"}]);
    ueber_senke(&c, "link-a", &w2);

    let bestand = c.intent_sicht(&a.project_binding_id, &a.session_epoch);
    assert!(bestand.entmaskierung_erlaubt(&qa, &qb, ""), "spezifisch vor global");
    assert!(
        !bestand.entmaskierung_erlaubt(&qa, &qc, ""),
        "und fuer ein anderes Paar bleibt das Veto stehen"
    );
}

/// Die Fassungswahl ist eine EIGENSCHAFT des Empfaengers, keine Kulanz: ein
/// `intent_update` unter der Fassung 2 wird abgewiesen, weil die Familie
/// dort nicht existiert.
#[test]
fn intent_update_unter_der_fassung_zwei_wird_abgewiesen() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));
    let w = mit_adresse(fixture("intent_update"), &a);

    assert!(!c.intent_update_json_mit_minor_fuer_test("link-a", &bytes(&w), 2));
    assert!(!c.intent_update_json_mit_minor_fuer_test("link-a", &bytes(&w), 1));
    assert!(c.intent_update_json_mit_minor_fuer_test("link-a", &bytes(&w), 3));

    let s = mit_adresse(fixture("assistant_step_update"), &a);
    assert!(!c.assistant_step_update_json_mit_minor_fuer_test("link-a", &bytes(&s), 2));
    assert!(c.assistant_step_update_json_mit_minor_fuer_test("link-a", &bytes(&s), 3));
}

// ═════════════════════════════════════════════════════════════════════════
// M-89 · spiegel_weicht_dem_neueren_main_schritt
// ═════════════════════════════════════════════════════════════════════════

struct TestOrdner(std::path::PathBuf);

impl TestOrdner {
    fn neu(name: &str) -> Self {
        let pfad = std::env::temp_dir().join(format!(
            "nakama-sonde014-{name}-{}-{}",
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

/// Der Schritt reist in den append-only `event_log` — und der Spiegel ist
/// NIE autoritativ.
///
/// §33.5 (Zeile 2147) sagt den versionierten SQLite-Spiegel des aktuellen
/// `AssistantStep` zu. Dieses Bein misst beide Haelften der Zusage:
///
///   1. der Schritt landet wirklich im Store (sonst waere „versionierter
///      Spiegel fuer Suche und Crashdiagnose" eine Behauptung);
///   2. nach einem Brokerneustart ueberschreibt der ALTE Eintrag NIE den
///      neueren Main-Schritt — der Broker startet mit LEEREM Spiegel und
///      uebernimmt, was der Main meldet.
#[test]
#[cfg(windows)]
fn spiegel_weicht_dem_neueren_main_schritt() {
    let ordner = TestOrdner::neu("schrittspiegel");
    let mut k = eqcop_broker::store::StoreKonfiguration::fuer_pfad(ordner.db());
    k.remote_volume_override = Some(false);
    let writer = eqcop_broker::store::StoreWriter::starten(k);
    assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());

    let a = adresse(1, 2, 3, 4);
    let c = Coordinator::mit_store(Arc::new(ManualClock::default()), hex(0xbeef), &writer);
    anmelden(&c, "link-a", &hello(a.clone()));

    let schritt = |name: &str, revision: i64| {
        let mut w = mit_adresse(fixture("assistant_step_update"), &a);
        w["schritt"] = Value::String(name.into());
        w["revision"] = json!(revision);
        w
    };
    ueber_senke(&c, "link-a", &schritt("finding", 1));
    ueber_senke(&c, "link-a", &schritt("proposal", 2));
    assert_eq!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.revision),
        Some(2)
    );

    // 1. Der Spiegel liegt wirklich im append-only Log — und zwar BEIDE
    //    Fassungen, nicht nur die letzte. Genau das ist „versioniert": eine
    //    Crashdiagnose braucht den Weg, nicht den Endpunkt.
    let schritte = writer
        .handle()
        .assistent_schritte_lesen(&a.project_binding_id, &a.session_epoch)
        .expect("das Ereignislog ist lesbar");
    assert_eq!(
        schritte.len(),
        2,
        "beide Schrittfassungen stehen im append-only Log"
    );
    let erster: Value = serde_json::from_slice(&schritte[0]).unwrap();
    assert_eq!(erster["schritt"], "finding");
    assert_eq!(erster["revision"], 1);
    let letzter: Value = serde_json::from_slice(&schritte[1]).unwrap();
    assert_eq!(letzter["schritt"], "proposal");
    assert_eq!(letzter["revision"], 2);

    // 2. Der Neustart: ein ZWEITER Coordinator auf DEMSELBEN Store.
    let neu = Coordinator::mit_store(Arc::new(ManualClock::default()), hex(0xbeee), &writer);
    assert!(
        neu.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .is_none(),
        "der frische Broker traegt KEINEN Schritt aus der Datenbank - der \
         Spiegel ist nie autoritativ (§33.5, M-59, M-76)"
    );
    assert!(
        !neu.darf_rechnen(&a.project_binding_id, &a.session_epoch),
        "und er rechnet nicht, bevor der Main seinen Bestand gemeldet hat"
    );

    anmelden(&neu, "link-b", &hello(a.clone()));
    ueber_senke(&neu, "link-b", &schritt("remeasure", 3));
    assert_eq!(
        neu.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.schritt),
        Some("remeasure".to_string()),
        "der neuere Main-Schritt gewinnt"
    );
}

/// Ein abgelehnter P1 schliesst die Verbindung nicht - er wird GEZAEHLT.
/// Ohne den Zaehler waere „angenommen" von „still verworfen" nicht zu
/// unterscheiden (Prueflistenzeile A: Rueckgabewerte einer Politik werden
/// ausgewertet).
#[test]
fn angenommene_und_abgewiesene_meldungen_sind_zaehlbar() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));

    assert_eq!(c.intent_updates(), 0);
    assert_eq!(c.assistent_updates(), 0);

    ueber_senke(&c, "link-a", &mit_adresse(fixture("intent_update"), &a));
    ueber_senke(&c, "link-a", &mit_adresse(fixture("assistant_step_update"), &a));
    assert_eq!(c.intent_updates(), 1);
    assert_eq!(c.assistent_updates(), 1);

    // Eine abgewiesene Meldung erhoeht den Zaehler NICHT und beendet die
    // Verbindung auch nicht.
    let mut kaputt = mit_adresse(fixture("intent_update"), &a);
    kaputt["bestand_revision"] = json!(0);
    ueber_senke(&c, "link-a", &kaputt);
    assert_eq!(c.intent_updates(), 1);
    assert!(c.dispatch_fuer_link_erlaubt("link-a"), "der Link lebt weiter");
}
