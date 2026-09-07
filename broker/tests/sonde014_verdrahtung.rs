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

use eqcop_broker::coordinator::{
    Befundzustand, Coordinator, IntentAbweisung, ManualClock, SchrittAbweisung,
};
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

// ── Evidenzbuehne fuer die Nacharbeit 1 (NR-01, NR-03) ───────────────────
//
// Die Faelle dieser Runde messen nicht mehr nur den Spiegel, sondern was der
// Riegel VERHINDERT: einen Befund und einen Vorschlag. Dafuer braucht dieses
// Bein echte Evidenz — dieselbe committete Grundform, die `sonde014_befund`
// faehrt, damit nicht zwei Wire-Wahrheiten nebeneinander stehen.

const ANOMALIEBAND: usize = 98;

fn capabilities() -> Value {
    json!({
        "host_context": true, "project_time": true, "fine_automation": false,
        "double_precision": false, "latency_report": false, "aux_send": false,
        "aux_return": false, "compare_routing": false, "sidechain": false,
        "offline_render": false
    })
}

/// Anmelden MIT Deskriptor — ohne ihn kennt der Broker weder Messposition
/// noch Mixerkanal, und `routing_bekannt` waere strukturell falsch.
fn anmelden_mit_deskriptor(
    c: &Coordinator,
    link: &str,
    a: &Adresse,
    art: &str,
    mixer: Option<i64>,
) {
    let mut h = hello(a.clone());
    h.plugin_kind = art.into();
    let ausgang = c.control_hello_registrieren(link, &h);
    assert!(ausgang.angenommen, "{:?}", ausgang.grund);
    let _ = c.resync_bestaetigen(link, 0);
    let position = if art == "main" { "insert" } else { "post" };
    let mut d = json!({
        "adresse": a,
        "plugin_kind": art,
        "measurement_position": position,
        "aussageklasse": "beobachtend",
        "betrieb": "active",
        "label": "Testquelle",
        "capabilities": capabilities(),
        "frische": {"letzter_kontakt_ms": 10, "stale": false}
    });
    if let Some(index) = mixer {
        d["host_mixer_index"] = json!(index);
    }
    assert!(c.descriptor_setzen(link, d));
}

fn evidenz(a: &Adresse, nr: usize, projekt_start: i64) -> Vec<u8> {
    static ROH: std::sync::OnceLock<Value> = std::sync::OnceLock::new();
    let mut wert = ROH
        .get_or_init(|| {
            let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join(
                "../eq-copilot/fixtures/v3/gueltig/evidence-snapshot-mit-ereignissen-und-stereo.json",
            );
            serde_json::from_slice::<Value>(&std::fs::read(&pfad).expect("Evidenzfixture"))
                .expect("Evidenzfixture ist JSON")
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

fn reihe(c: &Coordinator, link: &str, a: &Adresse, ab_nr: usize, anzahl: usize) {
    for i in 0..anzahl {
        c.p1(link, &evidenz(a, ab_nr + i, 44_108_200 + (i as i64) * 512));
    }
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

// =========================================================================
// M-86 . NR-01 (Nacharbeit 1, 07.09.2026) - die Sperre ist FAIL-CLOSED
// =========================================================================
//
// Der Fall darueber misst `darf_rechnen`. Diese Frage hatte bis zur
// Nacharbeit 1 keinen Produktaufrufer - und die zwei Bedingungen, die den
// Riegel im Produkt trugen, lauteten `is_some_and(|i| !i.vollstaendig)`. Bei
// `intent == None` ist das FALSCH: ohne JEDE Vollstaendigkeitsmeldung
// entstanden Findings und Proposals.
//
// Dieser Fall misst deshalb nicht die Frage, sondern ihre WIRKUNG, und er
// beginnt mit dem Zustand, den EP-01 gefunden hat: gar keine
// `intent_update`-Nachricht.
#[test]
fn keine_rechnung_ohne_jede_intentmeldung() {
    let c = coordinator();
    let master = adresse(0x11, 0x22, 1, 0x41);
    let sonde = adresse(0x11, 0x22, 2, 0x42);
    anmelden_mit_deskriptor(&c, "main", &master, "main", Some(0));
    anmelden_mit_deskriptor(&c, "sonde0", &sonde, "passive_probe", Some(3));

    // -- (1) OHNE jede Meldung: Evidenz reist, aber nichts entsteht -------
    reihe(&c, "main", &master, 0, 12);
    reihe(&c, "sonde0", &sonde, 100, 12);
    assert!(
        c.befunde_sicht(&master.project_binding_id, &master.session_epoch)
            .is_empty(),
        "ohne jede Vollstaendigkeitsmeldung entsteht KEIN Befund"
    );
    assert_eq!(
        c.draft_offers_zaehler(),
        0,
        "und kein Proposal - ein fehlendes Veto saehe aus wie `kein Schutz gewuenscht`"
    );

    // -- (2) GEGENPROBE: mit dem Vollbestand samt Marke entsteht einer ----
    //
    // Ohne sie waere (1) auch dann gruen, wenn die Buehne gar keinen Befund
    // tragen KANN - und der Riegel haette keinen Weg zu fallen.
    let marke = json!({
        "type": "intent_update",
        "adresse": master,
        "session_epoch": master.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 0
    });
    ueber_senke(&c, "main", &marke);
    // Der Ausloeser ist die Evidenzaenderung (Paragraph 33.5 kennt keinen
    // zweiten); ein weiterer Beleg laesst den Broker rechnen.
    reihe(&c, "sonde0", &sonde, 200, 1);
    assert!(
        !c.befunde_sicht(&master.project_binding_id, &master.session_epoch)
            .is_empty(),
        "nach dem Vollbestand mit Marke entsteht der Befund"
    );
}

// =========================================================================
// M-85 . NR-02 (Nacharbeit 1, 07.09.2026) - die Ordnung gilt JE OBJEKT
// =========================================================================
//
// Der Sender koalesziert je `(quelle_id, scope)`, und
// `P1Warteschlange::einreihen()` ersetzt einen Eintrag AN SEINER POSITION.
// Die Ankunft A/1, B/2, A/3 ist damit normal. An der GLOBALEN
// Bestandsrevision gemessen scheiterte B/2 an A/3 - und die Rolle einer
// fremden Quelle fehlte dauerhaft im Broker.
#[test]
fn teilupdate_fremden_schluessels_ueberlebt_eine_koaleszierung() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));

    let quelle_a = hex(0xa1);
    let quelle_b = hex(0xb2);

    let einzeln = |quelle_id: &str, rolle: &str, bestand: i64| {
        let mut w = mit_adresse(fixture("intent-update-einzelne-fortschreibung"), &a);
        w["vollstaendig"] = Value::Bool(false);
        w["bestand_revision"] = json!(bestand);
        w["intents"] = json!([{
            "quelle_id": quelle_id,
            "rolle": rolle,
            "revision": bestand,
            "herkunft": "user",
            "konfidenz": 1.0
        }]);
        w
    };

    // Die Reprofolge aus EP-02 des Prueferurteils, in der Reihenfolge, in der
    // sie beim BROKER ankommt.
    //
    // Der Sender reiht A/1, B/2 und A/3 ein.
    // `P1Warteschlange::einreihen()` koalesziert je Schluessel und ersetzt A
    // AN SEINER BISHERIGEN POSITION (`IpcQueues.h`:349-360) - A/3 steht damit
    // vor B/2, und der Broker sieht zuerst die Revision 3 und danach die 2.
    // Genau das ist der Fall, in dem eine globale Ordnung das fremde Objekt
    // vollstaendig verwirft.
    ueber_senke(&c, "link-a", &einzeln(&quelle_a, "traegt", 3));
    ueber_senke(&c, "link-a", &einzeln(&quelle_b, "geschuetzt", 2));

    let bestand = c.intent_sicht(&a.project_binding_id, &a.session_epoch);
    assert_eq!(
        bestand.wirkend(&quelle_a, "").map(|s| s.rolle.as_str()),
        Some("traegt"),
        "A traegt seine juengste Rolle"
    );
    assert_eq!(
        bestand.wirkend(&quelle_b, "").map(|s| s.rolle.as_str()),
        Some("geschuetzt"),
        "und B ueberlebt die Koaleszierung von A - sonst fehlte ein VETO"
    );

    // Und die Ordnung bleibt total: A/2 nach A/3 wird abgewiesen.
    assert_eq!(
        c.intent_update_json_grund_fuer_test("link-a", &bytes(&einzeln(&quelle_a, "begleitet", 2))),
        Err(IntentAbweisung::AeltereRevision),
        "eine aeltere Revision DESSELBEN Objekts faellt weiterhin"
    );
    assert_eq!(
        c.intent_sicht(&a.project_binding_id, &a.session_epoch)
            .wirkend(&quelle_a, "")
            .map(|s| s.rolle.as_str()),
        Some("traegt"),
        "der juengere Wert steht weiter"
    );
    // Ein VOLLBERICHT ordnet sich weiter an der Sitzungsrevision, und die ist
    // das Maximum: ein Vollbericht mit Revision 2 kaeme nach A/3 zu spaet.
    let mut alt_voll = mit_adresse(fixture("intent-update-leerer-bestand-mit-marke"), &a);
    alt_voll["bestand_revision"] = json!(2);
    assert_eq!(
        c.intent_update_json_grund_fuer_test("link-a", &bytes(&alt_voll)),
        Err(IntentAbweisung::AeltereRevision),
        "die Sitzungsrevision wird als MAXIMUM fortgeschrieben"
    );
}

// =========================================================================
// M-10/M-24 . NR-03 (Nacharbeit 1, 07.09.2026) - geprueft VOR dem Einsetzen
// =========================================================================
//
// `hypothesen_bilden` sammelt unter dem Lock, rechnet ausserhalb und trug
// bis zur Nacharbeit 1 bedingungslos ein. Zwei Interleavings brachen damit
// eine Zusage, und beide stehen hier - jedes ueber den Testhaken, der GENAU
// zwischen Sammeln und Eintragen faellt.
//
// Der Coordinator liegt dafuer in einem `Arc`, und der Haken haelt nur einen
// `Weak` darauf: ein starker Verweis waere ein Zyklus (der Coordinator haelt
// den Haken), und ein roher Zeiger waere `unsafe` fuer nichts.
#[test]
fn veraltetes_rechenergebnis_wird_nicht_veroeffentlicht() {
    // -- (a) die Intent-Revision steigt WAEHREND der Rechnung ------------
    let c = Arc::new(coordinator());
    let master = adresse(0x11, 0x22, 1, 0x41);
    let sonde = adresse(0x11, 0x22, 2, 0x42);
    anmelden_mit_deskriptor(&c, "main", &master, "main", Some(0));
    anmelden_mit_deskriptor(&c, "sonde0", &sonde, "passive_probe", Some(3));
    let marke = json!({
        "type": "intent_update",
        "adresse": master,
        "session_epoch": master.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 1
    });
    ueber_senke(&c, "main", &marke);
    reihe(&c, "main", &master, 0, 12);
    reihe(&c, "sonde0", &sonde, 100, 11);
    let vorher = c.befunde_sicht(&master.project_binding_id, &master.session_epoch);
    assert!(
        vorher.iter().any(|b| b.intent_revision == 1),
        "die Buehne traegt einen Befund unter der Revision 1 - sonst maesse der Fall nichts"
    );

    // Der Haken faellt EINMAL, zwischen `aufnahmen_sammeln` und
    // `befunde_eintragen`: der User setzt in diesem Augenblick eine Rolle,
    // und die Bestandsrevision steigt auf 2.
    let hoehere = json!({
        "type": "intent_update",
        "adresse": master,
        "session_epoch": master.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 2,
        "intents": [{
            "quelle_id": sonde.instance_id,
            "rolle": "traegt",
            "revision": 1,
            "herkunft": "user",
            "konfidenz": 1.0
        }]
    });
    let bytes_hoehere = bytes(&hoehere);
    let schwach = Arc::downgrade(&c);
    c.rechen_test_haken_setzen(Box::new(move || {
        if let Some(k) = schwach.upgrade() {
            assert!(k.intent_update_json("main", &bytes_hoehere));
        }
    }));
    // Der letzte Beleg loest die Rechnung aus.
    reihe(&c, "sonde0", &sonde, 111, 1);

    assert_eq!(
        c.intent_sicht(&master.project_binding_id, &master.session_epoch)
            .revision,
        2,
        "der Haken ist wirklich gefallen - sonst maesse der Fall gar nichts"
    );
    let befunde = c.befunde_sicht(&master.project_binding_id, &master.session_epoch);
    assert!(
        befunde
            .iter()
            .all(|b| !(b.intent_revision == 1 && b.zustand == Befundzustand::ReadyToSend)),
        "kein READY-Befund der ALTEN Revision im Snapshot: {:?}",
        befunde
            .iter()
            .map(|b| (b.intent_revision, b.zustand))
            .collect::<Vec<_>>()
    );
    assert!(
        befunde.iter().any(|b| b.zustand == Befundzustand::Stale),
        "der bereits veraltete Bestand steht weiter - er wurde nicht ueberschrieben"
    );

    // -- (b) eine Evidenz wird WAEHREND der Rechnung zurueckgenommen -----
    let d = Arc::new(coordinator());
    anmelden_mit_deskriptor(&d, "main", &master, "main", Some(0));
    anmelden_mit_deskriptor(&d, "sonde0", &sonde, "passive_probe", Some(3));
    let marke_d = json!({
        "type": "intent_update",
        "adresse": master,
        "session_epoch": master.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 0
    });
    ueber_senke(&d, "main", &marke_d);
    reihe(&d, "main", &master, 0, 12);
    reihe(&d, "sonde0", &sonde, 100, 11);

    // Der Beleg Nummer 105 der Sonde liegt im sechsten Projektfenster; er ist
    // der, den der Haken gleich zurueckzieht.
    let zurueckgenommen = hex(0x1000 + 105);
    let fenster_von = 44_108_200 + 5 * 512;
    assert!(
        d.befunde_sicht(&master.project_binding_id, &master.session_epoch)
            .iter()
            .any(|b| b.evidence_ids.contains(&zurueckgenommen)),
        "die Buehne traegt die spaeter zurueckgenommene ID - sonst maesse der Fall nichts"
    );
    let schwach_d = Arc::downgrade(&d);
    d.rechen_test_haken_setzen(Box::new(move || {
        if let Some(k) = schwach_d.upgrade() {
            // Der Produktweg einer Ruecknahme: ein hoerbarer Eingriff nimmt
            // die Evidenz SEINES Bereichs zurueck (M-52).
            let getroffen = k.invalidierung_wegen_intervention_fuer_link(
                "sonde0",
                fenster_von,
                fenster_von + 1,
            );
            assert!(getroffen > 0, "die Ruecknahme trifft wirklich Evidenz");
        }
    }));
    reihe(&d, "sonde0", &sonde, 111, 1);

    let befunde = d.befunde_sicht(&master.project_binding_id, &master.session_epoch);
    assert!(
        befunde
            .iter()
            .all(|b| !b.evidence_ids.contains(&zurueckgenommen)),
        "eine waehrend der Rechnung zurueckgenommene ID darf nie wieder sichtbar werden"
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

// ═════════════════════════════════════════════════════════════════════════
// SONDE-014 ETAPPE I · Ort, Nebenläufigkeit und Invarianten
// ═════════════════════════════════════════════════════════════════════════

/// **M-86 seit E-11** — die Vollständigkeitsmarke deckt **beide** Bestände.
///
/// 🔑 Der Fund dieser Etappe. Bis hierher deckte die Marke nur den Intent,
/// und das riss ein Loch derselben Klasse, die M-86 für den Intent schließt:
/// lädt der Main ein anderes Projekt oder startet er neu, meldet er seinen
/// Vollbestand samt Marke — und hat er **keinen** offenen Assistentenschritt,
/// sendet er auch keinen (`PluginProcessor::sendeIntentVollbestand`: „ein
/// erfundener wäre schlimmer als keiner"). Der Spiegel trug dann weiter den
/// Schritt der VORIGEN Main-Generation, und Suche wie Crashdiagnose fänden
/// einen Schritt, den es nicht mehr gibt — genau die Aussage, die §33.5 mit
/// „der Spiegel ist nie autoritativ" verbietet.
///
/// Beide Richtungen fallen einzeln: die Marke räumt einen alten Schritt fort,
/// und der unmittelbar danach gemeldete Schritt steht.
#[test]
fn die_vollstaendigkeitsmarke_deckt_beide_bestaende() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&c, "link-a", &hello(a.clone()));

    let voll = |revision: i64| {
        let mut w = mit_adresse(fixture("intent-update-leerer-bestand-mit-marke"), &a);
        w["bestand_revision"] = json!(revision);
        w
    };
    let schritt = |name: &str, revision: i64| {
        let mut w = mit_adresse(fixture("assistant_step_update"), &a);
        w["schritt"] = Value::String(name.into());
        w["revision"] = json!(revision);
        w
    };

    // Die erste Main-Generation: Vollbestand mit Marke, danach ihr Schritt.
    ueber_senke(&c, "link-a", &voll(1));
    ueber_senke(&c, "link-a", &schritt("finding", 1));
    assert_eq!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.schritt),
        Some("finding".to_string()),
        "der Schritt der ersten Generation steht"
    );

    // Die zweite Generation meldet ihren Vollbestand — und hat KEINEN
    // Schritt. Der alte darf sie nicht überleben.
    ueber_senke(&c, "link-a", &voll(2));
    assert!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .is_none(),
        "die_vollstaendigkeitsmarke_deckt_beide_bestaende - die Marke raeumt den Schritt der vorigen Generation fort"
    );
    assert!(
        c.darf_rechnen(&a.project_binding_id, &a.session_epoch),
        "und die Rechnung ist frei: ein leerer Schrittbestand ist eine Aussage"
    );

    // Die Gegenprobe: meldet die zweite Generation einen Schritt, steht er.
    ueber_senke(&c, "link-a", &schritt("evidence", 2));
    assert_eq!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.schritt),
        Some("evidence".to_string()),
        "die Marke raeumt nur, sie sperrt nicht"
    );

    // Eine TEILmeldung raeumt dagegen NICHTS - sie ist keine Marke.
    let mut teil = mit_adresse(fixture("intent-update-einzelne-fortschreibung"), &a);
    teil["vollstaendig"] = Value::Bool(false);
    teil["bestand_revision"] = json!(3);
    ueber_senke(&c, "link-a", &teil);
    assert_eq!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.schritt),
        Some("evidence".to_string()),
        "ein Ausschnitt sagt nichts ueber den Schrittbestand"
    );
}

/// **M-76** — ein frischer Broker rechnet nicht auf seinem Cache.
///
/// „Sessiongraph, Frische, Subscriptions und Broker-Cache sind flüchtig und
/// werden aus Plugin-Reports, Main-State und Store rekonstruiert. Der Broker
/// ist **nicht berechtigt**, beim Reconnect blind seinen Cache auf die Probe
/// zu drücken." Für Intent und Schritt heißt das: nach dem Neustart trägt er
/// **nichts**, und er rechnet erst, wenn der Main seinen Bestand gemeldet
/// hat — auch dann, wenn im Main gerade ein Schritt **offen** ist.
#[test]
fn ein_frischer_broker_traegt_weder_intent_noch_schritt() {
    let alt = coordinator();
    let a = adresse(1, 2, 3, 4);
    anmelden(&alt, "link-a", &hello(a.clone()));
    ueber_senke(
        &alt,
        "link-a",
        &mit_adresse(fixture("intent-update-leerer-bestand-mit-marke"), &a),
    );
    let mut offener_schritt = mit_adresse(fixture("assistant_step_update"), &a);
    offener_schritt["schritt"] = Value::String("evidence".into());
    offener_schritt["revision"] = json!(7);
    offener_schritt["offen"] = Value::Bool(true);
    ueber_senke(&alt, "link-a", &offener_schritt);
    assert!(alt.darf_rechnen(&a.project_binding_id, &a.session_epoch));
    assert!(alt
        .assistent_sicht(&a.project_binding_id, &a.session_epoch)
        .is_some());

    // Der Neustart: ein frischer Coordinator, dieselbe Sitzung, derselbe
    // Main. Ein Brokerneustart ist genau das - der Spiegel ist fluechtig.
    let neu = coordinator();
    anmelden(&neu, "link-a", &hello(a.clone()));
    assert!(
        !neu.darf_rechnen(&a.project_binding_id, &a.session_epoch),
        "ein_frischer_broker_traegt_weder_intent_noch_schritt - er rechnet nicht, bevor der Main gemeldet hat"
    );
    assert!(
        neu.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .is_none(),
        "und er traegt keinen Schritt aus einem Cache, den es nicht gibt"
    );

    // Der Main meldet erneut - und seine Zahlen gewinnen. Eine AELTERE
    // Revision als die des Mains koennte den Spiegel nie zuruecksetzen: sie
    // wird abgewiesen.
    ueber_senke(
        &neu,
        "link-a",
        &mit_adresse(fixture("intent-update-leerer-bestand-mit-marke"), &a),
    );
    ueber_senke(&neu, "link-a", &offener_schritt);
    assert_eq!(
        neu.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.revision),
        Some(7),
        "der Main-Stand steht wieder, unveraendert"
    );
    let mut aelter = offener_schritt.clone();
    aelter["revision"] = json!(3);
    aelter["schritt"] = Value::String("coverage".into());
    assert_eq!(
        neu.assistant_step_update_grund_fuer_test("link-a", &bytes(&aelter)),
        Err(SchrittAbweisung::AeltereRevision),
        "ein aelterer Stand setzt den neueren Main-Schritt NIE zurueck"
    );
}

/// **M-75** — die neuen Familien erben die Ordnung von Verbinden und Trennen.
///
/// Prüfliste B: „Reihenfolge beim Verbinden ist festgelegt und gemessen:
/// anmelden → Callback ‚verbunden‘ abgeschlossen → erst dann Freigabe nach
/// außen" und „Nach dem Schließen einer Queue wird nichts mehr geliefert; das
/// Schließflag wird VOR dem Inhalt geprüft."
///
/// Für `intent_update` und `assistant_step_update` heißt das: vor der
/// Anmeldung wirkt keine der beiden, nach dem Ende des Links ebenso wenig —
/// und in beiden Fällen bleibt der Spiegel unverändert statt still zu
/// verschwinden.
#[test]
fn die_neuen_familien_erben_die_ordnung_von_verbinden_und_trennen() {
    let c = coordinator();
    let a = adresse(1, 2, 3, 4);

    // VOR der Anmeldung: der Link ist unbekannt, beide Familien wirken nicht.
    let voll = mit_adresse(fixture("intent-update-leerer-bestand-mit-marke"), &a);
    let schritt = mit_adresse(fixture("assistant_step_update"), &a);
    assert_eq!(
        c.intent_update_json_grund_fuer_test("link-a", &bytes(&voll)),
        Err(IntentAbweisung::KeinLink),
        "ohne Anmeldung wirkt kein Intent"
    );
    assert_eq!(
        c.assistant_step_update_grund_fuer_test("link-a", &bytes(&schritt)),
        Err(SchrittAbweisung::KeinLink),
        "und kein Schritt"
    );
    assert!(!c.darf_rechnen(&a.project_binding_id, &a.session_epoch));

    // NACH der Anmeldung wirken beide - das ist die Gegenprobe, ohne die die
    // Abweisung oben auch von einem kaputten Leser kommen koennte.
    anmelden(&c, "link-a", &hello(a.clone()));
    ueber_senke(&c, "link-a", &voll);
    ueber_senke(&c, "link-a", &schritt);
    assert!(c.darf_rechnen(&a.project_binding_id, &a.session_epoch));
    let stand = c
        .assistent_sicht(&a.project_binding_id, &a.session_epoch)
        .expect("der Schritt steht");

    // NACH dem Ende des Links wirkt wieder keine von beiden, und der Spiegel
    // bleibt, wie er war: ein Reconnect faende sonst einen leeren Bestand vor.
    c.control_ende("link-a");
    let mut spaeter = schritt.clone();
    spaeter["schritt"] = Value::String("verdict".into());
    spaeter["revision"] = json!(99);
    assert_eq!(
        c.assistant_step_update_grund_fuer_test("link-a", &bytes(&spaeter)),
        Err(SchrittAbweisung::KeinLink),
        "die_neuen_familien_erben_die_ordnung_von_verbinden_und_trennen - an einem beendeten Link wird nichts mehr angenommen"
    );
    assert_eq!(
        c.assistent_sicht(&a.project_binding_id, &a.session_epoch)
            .map(|s| s.schritt),
        Some(stand.schritt.clone()),
        "und der Spiegel steht unveraendert"
    );
}
