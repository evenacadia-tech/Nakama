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
    Ausschlussgrund, Befundzustand, Coordinator, IntentAbweisung, ManualClock,
    SchrittAbweisung, Sicherheitsklasse,
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
    anmelden_mit_deskriptor_und_host(c, link, a, art, mixer, 4711);
}

/// Wie `anmelden_mit_deskriptor`, aber mit eigener Host-PID.
///
/// ⚠️ `auto_join_locked` bestätigt eine Mitgliedschaft nur, wenn die
/// `main`-Clients DESSELBEN Hosts in GENAU EINER Sitzung liegen. Zwei
/// Sitzungen auf derselben PID lassen deshalb beide unbestätigt — wer eine
/// zweite, unabhängige Sitzung baut, braucht einen zweiten Host.
fn anmelden_mit_deskriptor_und_host(
    c: &Coordinator,
    link: &str,
    a: &Adresse,
    art: &str,
    mixer: Option<i64>,
    host_pid: u32,
) {
    let mut h = hello(a.clone());
    h.plugin_kind = art.into();
    if let Some(host) = h.host.as_mut() {
        host.pid = host_pid;
    }
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
    evidenz_mit_anhebung(a, nr, projekt_start, 90)
}

/// Dieselbe Fixture mit STEUERBARER Anhebung im Anomalieband.
///
/// Eine konstante Anhebung ist kein messbarer Uplift (NAK-212 R1): jede Reihe
/// aus `reihe` bleibt deshalb bei `mittel`. Wo eine Zeile den POSITIVEN
/// Zusammenhang braucht — und damit die Lage, in der `hoch` ueberhaupt
/// erreichbar ist —, wechselt die Anhebung von Fenster zu Fenster.
fn evidenz_mit_anhebung(a: &Adresse, nr: usize, projekt_start: i64, anhebung: i64) -> Vec<u8> {
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
                werte[index] = json!(alt + anhebung);
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

/// Dieselbe Reihe mit WECHSELNDER Anhebung — die Lage mit positivem
/// Zusammenhang. Das letzte Fenster ist laut, damit `masteranomalie` dieselbe
/// Bandgruppe findet wie die Reihe (Muster `wechselnd` aus
/// `sonde014_gegenbeispiele.rs`).
fn reihe_wechselnd(c: &Coordinator, link: &str, a: &Adresse, ab_nr: usize, anzahl: usize) {
    let laut_bei = (anzahl.max(1) - 1) % 2;
    for i in 0..anzahl {
        let anhebung = if i % 2 == laut_bei { 90 } else { 0 };
        c.p1(
            link,
            &evidenz_mit_anhebung(a, ab_nr + i, 44_108_200 + (i as i64) * 512, anhebung),
        );
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

/// Ein Coordinator MIT echtem Store — derselbe Aufbau wie in
/// `spiegel_weicht_dem_neueren_main_schritt`, hier als Helfer.
///
/// 🔑 **Nacharbeit 1 (08.09.2026, Erstprüfungsbefund 3):** `coordinator()`
/// erzeugt `Coordinator::mit_uhr` OHNE Store. Eine Invalidierung erreicht dort
/// ausschließlich den `Ok(None)`-Zweig von `invalidierung_anwenden` — den
/// Zweig für „ein Broker ohne Store hält seinen Bestand rein flüchtig". Weder
/// die Persistierung noch die Zustellung über den Store werden dabei
/// gefahren, obwohl K-49 wörtlich „Der Broker läuft mit Store" verlangt. Der
/// Rückgabewert trägt den Writer mit: fällt er, schließt der Store.
///
/// ⚠️ `TestOrdner` löscht seinen Pfad im `Drop`. Beide Rückgabewerte müssen
/// deshalb bis zum Testende leben.
fn coordinator_mit_store(
    name: &str,
) -> (Coordinator, eqcop_broker::store::StoreWriter, TestOrdner) {
    let ordner = TestOrdner::neu(name);
    let mut k = eqcop_broker::store::StoreKonfiguration::fuer_pfad(ordner.db());
    k.remote_volume_override = Some(false);
    let writer = eqcop_broker::store::StoreWriter::starten(k);
    assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());
    let c = Coordinator::mit_store(Arc::new(ManualClock::default()), hex(0xbeef), &writer);
    (c, writer, ordner)
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


// ═════════════════════════════════════════════════════════════════════════
// NAK-213 R2/E5 · die STUMME Quelle   (K-17, K-18, K-19, K-20)
// ═════════════════════════════════════════════════════════════════════════
//
// Bis NAK-213 lief `aufnahmen_sammeln` ueber `stand.evidenz.keys()`. Eine
// Quelle, die angemeldet und bestaetigt ist, aber noch nie einen Beleg
// gesendet hat, betrat damit weder die Kandidatenliste noch die Kanaltafel:
// der einzige Ueberlebende sah aus, als sei er allein, und wurde `hoch`
// (Gate-Befunde G-D2, G-H1).

/// Eine Buehne mit Main und `sonden` Sonden, jede auf eigenem Mixerkanal.
fn buehne_nak213(c: &Coordinator, sonden: usize) -> Vec<Adresse> {
    let main = adresse(0x11, 0x22, 1, 0x40);
    anmelden_mit_deskriptor(c, "main", &main, "main", Some(1));
    let marke = json!({
        "type": "intent_update",
        "adresse": main,
        "session_epoch": main.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 0
    });
    c.p1("main", &bytes(&marke));
    let mut aus = vec![main];
    for i in 0..sonden {
        let a = adresse(0x11, 0x22, 2 + i, 0x50 + i);
        anmelden_mit_deskriptor(c, &format!("sonde{i}"), &a, "passive_probe", Some(3 + i as i64));
        aus.push(a);
    }
    aus
}

fn befunde_der_sitzung(c: &Coordinator) -> Vec<eqcop_broker::coordinator::CauseHypothesis> {
    c.befunde_sicht(&hex(0x11), &hex(0x22))
}

/// **K-17.** Eine angemeldete, bestaetigte, lebende Quelle OHNE Beleg wird
/// Kandidat mit leerer Fensterfolge, faellt mit `evidenz_zurueckgenommen` und
/// macht die Konkurrenz unvollstaendig.
///
/// ⚠️ Sie liegt auf einem EIGENEN Mixerkanal — sonst maesse die Zeile
/// zugleich die Duplikatmarke aus R3, und ein gruener Lauf bewiese nicht,
/// welche der beiden Regeln gewirkt hat.
#[test]
fn stumme_quelle_ist_sichtbarer_kandidat() {
    let c = coordinator();
    let a = buehne_nak213(&c, 2);
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 12);
    // Sonde 1 ist angemeldet und schweigt.
    let befunde = befunde_der_sitzung(&c);
    assert_eq!(befunde.len(), 1, "nur die messende Quelle traegt einen Befund");
    let fuehrend = &befunde[0];
    assert!(
        fuehrend
            .ausschluesse
            .iter()
            .any(|x| x.candidate_source == a[2].instance_id
                && x.grund == Ausschlussgrund::EvidenzZurueckgenommen),
        "die stumme Quelle steht als Ausschluss im Befund (M-87): {:?}",
        fuehrend.ausschluesse
    );
    assert_eq!(
        fuehrend.rang.routingqualitaet, 1.0,
        "der Term `parent_duplikat` greift hier ausdruecklich NICHT — sonst \
         bewiese der gruene Lauf nur R3"
    );
    assert!(
        fuehrend.confidence.klasse < Sicherheitsklasse::Hoch,
        "hoechstens `mittel` — heutiger Stand: die stumme Quelle existiert \
         fuer die Rechnung nicht und der Ueberlebende traegt `hoch`: {:?}",
        fuehrend.confidence
    );
}

/// **K-18.** Dieselbe Quelle, aber ABGEMELDET: sie ist kein bestaetigtes
/// Mitglied mehr und deckelt nichts. „Liefert nie" ist damit strukturell
/// beantwortet, OHNE Zahl — die Liveness beantwortet dieselbe Frage bereits,
/// und eine Kadenzgrenze waere eine geratene (M-31).
///
/// ⚠️ Gemessen wird der ABMELDEWEG, nicht der Ablauf der Liveness. Ein
/// Liveness-Tick machte JEDEN Client der Sitzung `stale` — auch Main und
/// Ueberlebenden —, und die Zeile maesse dann nichts ueber die stumme Quelle.
/// Beide Wege enden in derselben Bedingung (`bestaetigt && !stale`).
#[test]
fn stale_quelle_deckelt_nicht() {
    let c = coordinator();
    let a = buehne_nak213(&c, 2);
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 12);
    // Vorbedingung: mit der stummen Quelle ist die Aussage gedeckelt.
    assert!(
        befunde_der_sitzung(&c)
            .first()
            .is_some_and(|b| b.confidence.klasse < Sicherheitsklasse::Hoch),
        "Vorbedingung: die lebende stumme Quelle deckelt"
    );
    // Die stumme Quelle meldet sich ab.
    assert!(c.beitritt_aufheben(&hex(0x11), &hex(0x22), &a[2].instance_id));
    // Eine neue Evidenz stoesst die Rechnung an.
    reihe(&c, "sonde0", &a[1], 200, 1);
    let befunde = befunde_der_sitzung(&c);
    let fuehrend = befunde.first().expect("ein Befund entsteht");
    assert!(
        !fuehrend
            .ausschluesse
            .iter()
            .any(|x| x.candidate_source == a[2].instance_id),
        "eine `stale` Quelle ist weder Kandidat noch Ausschluss: {:?}",
        fuehrend.ausschluesse
    );
}

/// **K-20.** Es gibt KEINEN Sonderfall „nach der Vollstaendigkeitsmarke": die
/// Aufnahme entsteht bei jeder Rechnung frisch aus dem Stand.
#[test]
fn spaeter_beitritt_deckelt_ab_der_bestaetigung() {
    let c = coordinator();
    let a = buehne_nak213(&c, 1);
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 12);
    assert_eq!(
        befunde_der_sitzung(&c)
            .first()
            .map(|b| b.ausschluesse.len()),
        Some(0),
        "Vorbedingung: vor dem Beitritt gibt es keinen Ausschluss"
    );

    // Jetzt meldet sich eine weitere Sonde an — nach der Marke.
    let spaet = adresse(0x11, 0x22, 9, 0x59);
    anmelden_mit_deskriptor(&c, "sonde_spaet", &spaet, "passive_probe", Some(9));
    reihe(&c, "sonde0", &a[1], 200, 1);
    let befunde = befunde_der_sitzung(&c);
    let fuehrend = befunde.first().expect("ein Befund entsteht");
    assert!(
        fuehrend
            .ausschluesse
            .iter()
            .any(|x| x.candidate_source == spaet.instance_id),
        "ab ihrer Bestaetigung deckelt sie: {:?}",
        fuehrend.ausschluesse
    );
}

/// **K-19 (erster Fall).** Ohne einen einzigen Beleg entsteht gar keine
/// Aufnahme — kein Ergebnis und keine Enthaltung. Das ist die Abwesenheit
/// einer Messung, kein verschwiegenes Ergebnis.
#[test]
fn ohne_jeden_beleg_entsteht_keine_aufnahme() {
    let c = coordinator();
    let _ = buehne_nak213(&c, 2);
    assert!(
        befunde_der_sitzung(&c).is_empty(),
        "eine Sitzung ohne jeden Beleg meldet nichts"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-213 R3/E6 · Kanaltafel ueber ALLE Quellen   (K-26, K-27, K-29)
// ═════════════════════════════════════════════════════════════════════════

/// **K-26.** Zwei Sonden auf einem Kanal, die Belege der einen vollstaendig
/// zurueckgenommen: BEIDE tragen weiterhin die Duplikatmarke, weil die
/// Kanaltafel aus den CLIENTS entsteht.
#[test]
fn zurueckgenommener_partner_bleibt_duplikat() {
    let c = coordinator();
    let main = adresse(0x11, 0x22, 1, 0x40);
    anmelden_mit_deskriptor(&c, "main", &main, "main", Some(1));
    let marke = json!({
        "type": "intent_update", "adresse": main,
        "session_epoch": main.session_epoch,
        "vollstaendig": true, "bestand_revision": 0
    });
    c.p1("main", &bytes(&marke));
    let eins = adresse(0x11, 0x22, 2, 0x50);
    let zwei = adresse(0x11, 0x22, 3, 0x51);
    // BEIDE auf Kanal 7.
    anmelden_mit_deskriptor(&c, "sonde0", &eins, "passive_probe", Some(7));
    anmelden_mit_deskriptor(&c, "sonde1", &zwei, "passive_probe", Some(7));
    reihe(&c, "main", &main, 0, 12);
    reihe(&c, "sonde0", &eins, 100, 12);
    for i in 0..12 {
        c.p1("sonde1", &evidenz(&zwei, 300 + i, 44_108_200 + 400 * 512 + (i as i64) * 512));
    }
    let genommen = c.invalidierung_wegen_intervention_fuer_link(
        "sonde1",
        44_108_200 + 400 * 512 - 1,
        44_108_200 + 412 * 512 + 1,
    );
    assert_eq!(genommen, 12, "genau die Belege der zweiten Sonde");
    reihe(&c, "sonde0", &eins, 200, 1);
    let befunde = befunde_der_sitzung(&c);
    let fuehrend = befunde.first().expect("ein Befund entsteht");
    assert_eq!(
        fuehrend.rang.routingqualitaet, 0.5,
        "die verbliebene Sonde traegt weiter die Duplikatmarke — heutiger \
         Stand: die Kanaltafel liest die Evidenzschluessel, und eine Quelle \
         ohne gueltigen Beleg steht in keiner Gruppe: {:?}",
        fuehrend.rang
    );
}

/// **K-27.** Eine Quelle OHNE gemeldeten Kanal teilt keinen — sie ist kein
/// Duplikat. Machte ein fehlender Kanal sie zum Duplikat aller anderen, waere
/// „Routing unbekannt" ein Ausschluss statt eines Deckels; M-22 sagt
/// ausdruecklich das Gegenteil.
#[test]
fn partner_ohne_routing_ist_kein_duplikat() {
    let c = coordinator();
    let main = adresse(0x11, 0x22, 1, 0x40);
    anmelden_mit_deskriptor(&c, "main", &main, "main", Some(1));
    let marke = json!({
        "type": "intent_update", "adresse": main,
        "session_epoch": main.session_epoch,
        "vollstaendig": true, "bestand_revision": 0
    });
    c.p1("main", &bytes(&marke));
    let mit = adresse(0x11, 0x22, 2, 0x50);
    let ohne = adresse(0x11, 0x22, 3, 0x51);
    anmelden_mit_deskriptor(&c, "sonde0", &mit, "passive_probe", Some(7));
    anmelden_mit_deskriptor(&c, "sonde1", &ohne, "passive_probe", None);
    reihe(&c, "main", &main, 0, 12);
    reihe(&c, "sonde0", &mit, 100, 12);
    reihe(&c, "sonde1", &ohne, 300, 12);
    let befunde = befunde_der_sitzung(&c);
    assert_eq!(befunde.len(), 2, "beide bleiben sichtbar");
    let mit_kanal = befunde
        .iter()
        .find(|b| b.candidate_source == mit.instance_id)
        .expect("die Quelle mit Kanal");
    let ohne_kanal = befunde
        .iter()
        .find(|b| b.candidate_source == ohne.instance_id)
        .expect("die Quelle ohne Kanal");
    assert_eq!(
        mit_kanal.rang.routingqualitaet, 1.0,
        "ein fehlender Kanal ist keine Kanalgleichheit"
    );
    assert_eq!(
        ohne_kanal.rang.routingqualitaet, 0.0,
        "und die Quelle ohne Kanal traegt `routing_bekannt = false` (M-22)"
    );
}

/// **K-29.** Eine angemeldete, STUMME Sonde auf demselben Kanal setzt die
/// Duplikatmarke der messenden — die Kanaltafel entsteht aus den Clients.
/// Beide Wirkungen (Duplikat aus R3, Deckel aus R2) sind unabhaengig.
#[test]
fn stumme_quelle_setzt_die_duplikatmarke() {
    let c = coordinator();
    let main = adresse(0x11, 0x22, 1, 0x40);
    anmelden_mit_deskriptor(&c, "main", &main, "main", Some(1));
    let marke = json!({
        "type": "intent_update", "adresse": main,
        "session_epoch": main.session_epoch,
        "vollstaendig": true, "bestand_revision": 0
    });
    c.p1("main", &bytes(&marke));
    let messend = adresse(0x11, 0x22, 2, 0x50);
    let stumm = adresse(0x11, 0x22, 3, 0x51);
    anmelden_mit_deskriptor(&c, "sonde0", &messend, "passive_probe", Some(7));
    anmelden_mit_deskriptor(&c, "sonde1", &stumm, "passive_probe", Some(7));
    reihe(&c, "main", &main, 0, 12);
    reihe(&c, "sonde0", &messend, 100, 12);
    let befunde = befunde_der_sitzung(&c);
    let fuehrend = befunde.first().expect("ein Befund entsteht");
    assert_eq!(
        fuehrend.rang.routingqualitaet, 0.5,
        "die messende Sonde traegt die Duplikatmarke — heutiger Stand \
         (G-H1): „Duplikatpartner ohne Evidenz unsichtbar“, volle \
         Routingqualitaet und `hoch`: {:?}",
        fuehrend.rang
    );
    assert!(
        fuehrend
            .ausschluesse
            .iter()
            .any(|x| x.candidate_source == stumm.instance_id
                && x.grund == Ausschlussgrund::EvidenzZurueckgenommen),
        "und zusaetzlich, unabhaengig davon, den Deckel aus R2: {:?}",
        fuehrend.ausschluesse
    );
}


// ═════════════════════════════════════════════════════════════════════════
// NAK-213 R4/E7 · das FUEHRENDE Main   (K-32, K-33, K-34, K-37, K-53)
// ═════════════════════════════════════════════════════════════════════════
//
// Entwurf `:1669`: „genau ein führendes Main pro aktiver Sitzung". Bis
// NAK-213 las `aufnahmen_sammeln` `fuehrendes_main` NIE — es nahm den letzten
// `main` nach `instance_id`, und der erste verschwand spurlos (G-D4).

/// Meldet ein zweites `main` an, ohne die Sitzung sonst zu ändern.
fn zweites_main(c: &Coordinator, link: &str, instanz: usize) -> Adresse {
    let a = adresse(0x11, 0x22, instanz, 0x70 + instanz);
    anmelden_mit_deskriptor(c, link, &a, "main", Some(2));
    a
}

/// **K-37.** Der Master ist das BENANNTE führende Main, nicht der letzte der
/// Iteration. Die Zeile misst ausdrücklich den Fall, in dem sich beide Regeln
/// unterscheiden: `fuehrendes_main` zeigt auf den nach `instance_id`
/// KLEINEREN.
#[test]
fn master_ist_das_gefuehrte_main_nicht_das_letzte() {
    let c = coordinator();
    let a = buehne_nak213(&c, 1);
    // Das zweite `main` tritt bei — beide verlieren die Bestätigung, und
    // `fuehrung_neu_bewerten_locked` setzt `fuehrendes_main` auf `None`.
    let zweites = zweites_main(&c, "main2", 8);
    assert!(
        zweites.instance_id > a[0].instance_id,
        "Vorbedingung: das zweite `main` ist nach `instance_id` GROESSER — \
         die alte Regel („der letzte gewinnt“) waehlte also IHN"
    );
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 12);
    let befunde = befunde_der_sitzung(&c);
    assert_eq!(befunde.len(), 1, "eine Enthaltung ohne Ort");
    assert_eq!(
        befunde[0].ursachenklasse,
        eqcop_broker::coordinator::Ursachenklasse::DatenReichenNicht
    );
    assert_eq!(
        befunde[0].candidate_source, a[0].instance_id,
        "die lexikographisch KLEINSTE `instance_id` der `main`-Clients — \
         nicht die groesste, die die alte Regel genommen haette"
    );

    // Und die Gegenprobe: mit eindeutiger Führung rechnet die Kette wieder.
    assert!(c.beitritt_aufheben(&hex(0x11), &hex(0x22), &zweites.instance_id));
    assert!(c.beitritt_bestaetigen(&hex(0x11), &hex(0x22), &a[0].instance_id));
    reihe(&c, "sonde0", &a[1], 200, 1);
    let danach = befunde_der_sitzung(&c);
    assert_eq!(
        danach.first().map(|b| b.candidate_source.clone()),
        Some(a[1].instance_id.clone()),
        "der Befund nennt wieder die SONDE als Ursache: {danach:?}"
    );
}

/// **K-32.** Ein zweites `main` hebt eine AUSDRÜCKLICH bestätigte Führung
/// nicht auf. „Eindeutig führend" heisst genau, was `mitgliedschaft.rs`
/// darunter versteht; dieses Ticket erfindet keinen zweiten Führungsbegriff.
#[test]
fn zweites_main_hebt_bestaetigte_fuehrung_nicht_auf() {
    let c = coordinator();
    let a = buehne_nak213(&c, 1);
    // Das erste Main wird EXPLIZIT bestätigt.
    //
    // ⚠️ Der Rückgabewert meldet nur, OB sich die Mitgliedschaft geändert hat
    // (`geaendert = !client.bestaetigt`). Als einziges Main ist es über
    // `auto_join_locked` bereits bestätigt; gesetzt wird hier `explizit_bestaetigt`,
    // und genau das hält gegen den Beitritt des zweiten Mains.
    let _ = c.beitritt_bestaetigen(&hex(0x11), &hex(0x22), &a[0].instance_id);
    let _zweites = zweites_main(&c, "main2", 8);
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 12);
    let befunde = befunde_der_sitzung(&c);
    assert_eq!(
        befunde.first().map(|b| b.candidate_source.clone()),
        Some(a[1].instance_id.clone()),
        "die Kette rechnet weiter, mit dem ersten Main als Master: {befunde:?}"
    );
}

/// **K-33.** Der Rückzug des zweiten Mains ALLEIN stellt die Führung nicht
/// her — `beitritt_aufheben` bestätigt das verbleibende Main nicht, und die
/// Führungswahl zählt nur BESTÄTIGTE Mains. Erst die Bestätigung gibt die
/// Rechnung frei. Anmelden ↔ abmelden liegen damit im selben Änderungssatz.
#[test]
fn rueckzug_und_bestaetigung_geben_die_rechnung_frei() {
    let c = coordinator();
    let a = buehne_nak213(&c, 1);
    let zweites = zweites_main(&c, "main2", 8);
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 12);
    assert_eq!(
        befunde_der_sitzung(&c)
            .first()
            .map(|b| b.ursachenklasse),
        Some(eqcop_broker::coordinator::Ursachenklasse::DatenReichenNicht),
        "Ausgangslage: zwei unbestaetigte Mains, Enthaltung ohne Ort"
    );

    // (1) Nur der Rückzug.
    assert!(c.beitritt_aufheben(&hex(0x11), &hex(0x22), &zweites.instance_id));
    reihe(&c, "sonde0", &a[1], 200, 1);
    assert_eq!(
        befunde_der_sitzung(&c)
            .first()
            .map(|b| b.ursachenklasse),
        Some(eqcop_broker::coordinator::Ursachenklasse::DatenReichenNicht),
        "der Zwischenstand: `beitritt_aufheben` setzt NUR die Mitgliedschaft \
         des Zurueckgezogenen zurueck, das verbleibende Main bleibt \
         unbestaetigt, und `fuehrendes_main` bleibt `None`"
    );

    // (2) Erst die Bestätigung.
    assert!(c.beitritt_bestaetigen(&hex(0x11), &hex(0x22), &a[0].instance_id));
    reihe(&c, "sonde0", &a[1], 300, 1);
    assert_eq!(
        befunde_der_sitzung(&c)
            .first()
            .map(|b| b.candidate_source.clone()),
        Some(a[1].instance_id.clone()),
        "danach liefert die naechste Rechnung wieder Befunde — die Enthaltung \
         ist ein Zustand mit Rueckweg, kein Endzustand"
    );
}

/// **K-34.** Der Beitritt NACH einer gelaufenen Rechnung ersetzt die
/// bestehenden Befunde durch die Enthaltung — auf demselben Weg, auf dem jede
/// Neurechnung sie ersetzt. Es entsteht kein Mischstand.
#[test]
fn beitritt_nach_der_rechnung_ersetzt_die_befunde() {
    let c = coordinator();
    let a = buehne_nak213(&c, 1);
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 12);
    assert_eq!(
        befunde_der_sitzung(&c)
            .first()
            .map(|b| b.candidate_source.clone()),
        Some(a[1].instance_id.clone()),
        "Vorbedingung: ein Befund ueber die Sonde steht im Bestand"
    );

    let _zweites = zweites_main(&c, "main2", 8);
    reihe(&c, "sonde0", &a[1], 200, 1);
    let befunde = befunde_der_sitzung(&c);
    assert_eq!(befunde.len(), 1, "kein Mischstand aus altem Befund und Enthaltung");
    assert_eq!(
        befunde[0].ursachenklasse,
        eqcop_broker::coordinator::Ursachenklasse::DatenReichenNicht
    );
    assert_ne!(
        befunde[0].zustand,
        Befundzustand::ReadyToSend,
        "und kein `ready_to_send`-Befund bleibt sichtbar, waehrend die Sitzung \
         schon keine Fuehrung mehr hat"
    );
}

/// **K-53 (R8).** Eine Sitzung, deren Belege VOLLSTÄNDIG zurückgenommen sind,
/// erzeugt keinen Befund — auch keine Enthaltung ohne Ort.
///
/// Die Sitzungsschlüssel bleiben erhalten (die Rücknahme setzt den
/// `ausschlussgrund` am Eintrag, sie löscht ihn nicht), und eine später durch
/// eine ANDERE Sitzung angestossene Rechnung sammelt sie wieder ein. Ihre
/// Pflichtliste `evidence_ids` liesse sich dann nicht gültig füllen: eine
/// leere Liste fällt an `minItems: 1`, eine zurückgenommene ID am
/// Eintragungsriegel. Deshalb entsteht gar kein Befund.
#[test]
fn sitzung_ohne_gueltige_evidenz_erzeugt_keinen_befund() {
    let c = coordinator();
    let a = buehne_nak213(&c, 1);
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 12);
    assert!(
        !befunde_der_sitzung(&c).is_empty(),
        "Vorbedingung: die Sitzung traegt Befunde"
    );

    // ALLE Belege der Sitzung zurücknehmen.
    let genommen = c.invalidierung_wegen_intervention_fuer_link(
        "main",
        44_108_200 - 1,
        44_108_200 + 24 * 512 + 1,
    );
    assert_eq!(genommen, 24, "die zwoelf Master- und zwoelf Sondenbelege");

    // Eine ZWEITE Sitzung stösst die Rechnung an; `aufnahmen_sammeln` sammelt
    // die erste dabei erneut ein.
    let main2 = adresse(0x33, 0x44, 1, 0x80);
    anmelden_mit_deskriptor_und_host(&c, "main_s2", &main2, "main", Some(1), 4712);
    let marke2 = json!({
        "type": "intent_update", "adresse": main2,
        "session_epoch": main2.session_epoch,
        "vollstaendig": true, "bestand_revision": 0
    });
    c.p1("main_s2", &bytes(&marke2));
    let sonde2 = adresse(0x33, 0x44, 2, 0x81);
    anmelden_mit_deskriptor_und_host(&c, "sonde_s2", &sonde2, "passive_probe", Some(3), 4712);
    reihe(&c, "main_s2", &main2, 500, 12);
    reihe(&c, "sonde_s2", &sonde2, 600, 12);

    assert!(
        befunde_der_sitzung(&c).is_empty(),
        "fuer S1 entsteht KEIN Befund — auch keine Enthaltung ohne Ort (R8): {:?}",
        befunde_der_sitzung(&c)
    );
    assert!(
        !c.befunde_sicht(&hex(0x33), &hex(0x44)).is_empty(),
        "und S2 rechnet unberuehrt weiter"
    );
}

/// **K-54 (Nacharbeit 1, Erstprüfungsbefund 1).** Die Eingabefolge des
/// Prüfers, Schritt für Schritt: Vollständigkeitsmarke, Main A liefert
/// gültige Fenster, ein zweites Main derselben Sitzung tritt automatisch bei,
/// A misst erneut — **und die Sonden liefern nichts.**
///
/// 🔑 Genau hier schwieg die Sitzung. `aufnahmen_sammeln` legte ein
/// Main-Profil nur ab, wenn es das FÜHRENDE war; bei `fuehrendes_main = None`
/// verschwanden alle Main-Profile samt ihren Fenstern. `enthaltung_ohne_ort`
/// sammelte danach nur über Kandidaten und Master, fand ohne Sonde keinen
/// einzigen Beleg und gab nach R8 `None` zurück — obwohl A zwölf gültige
/// Fenster gemessen hatte. R8 sagt „ohne gültigen Beleg der Sitzung kein
/// Befund", nicht „ohne Sonde kein Befund": die Belegsammlung aus E7 gilt der
/// SITZUNG, und wer in ihr misst, gehört hinein — unabhängig davon, wer sie
/// führt.
#[test]
fn zweites_main_ohne_sonden_enthaelt_sich_mit_den_belegen_des_ersten() {
    let c = coordinator();
    // KEINE Sonde: `buehne_nak213(&c, 0)` meldet nur das Main an und setzt
    // die Vollstaendigkeitsmarke.
    let a = buehne_nak213(&c, 0);
    reihe(&c, "main", &a[0], 0, 12);
    let ausgang = befunde_der_sitzung(&c);
    assert_eq!(
        ausgang.len(),
        1,
        "Ausgangslage: ein fuehrendes Main allein, keine Kandidaten — die          BESTEHENDE Enthaltung MIT Ort (das Masterband ist gemessen): {ausgang:?}"
    );
    assert!(
        ausgang[0].beobachtung.gueltig && ausgang[0].band_hz.von > 0,
        "sie traegt einen Ort und einen gueltigen Wert — genau das          unterscheidet sie von der Enthaltung OHNE Ort: {:?}",
        ausgang[0]
    );

    // Das zweite Main tritt bei — beide verlieren die Bestaetigung, und
    // `fuehrung_neu_bewerten_locked` setzt `fuehrendes_main` auf `None`.
    let zweites = zweites_main(&c, "main2", 8);
    assert!(
        zweites.instance_id > a[0].instance_id,
        "Vorbedingung: das zweite Main ist nach `instance_id` groesser"
    );
    // A misst erneut und stoesst damit die Rechnung an. Die Sonden schweigen
    // weiter.
    reihe(&c, "main", &a[0], 100, 1);

    let befunde = befunde_der_sitzung(&c);
    assert_eq!(
        befunde.len(),
        1,
        "eine Enthaltung ohne Ort statt SCHWEIGEN: {befunde:?}"
    );
    let enthaltung = &befunde[0];
    assert_eq!(
        enthaltung.ursachenklasse,
        eqcop_broker::coordinator::Ursachenklasse::DatenReichenNicht
    );
    assert_eq!(
        enthaltung.likely_cause,
        "Zwei Instanzen fuehren diese Sitzung — bis eine von ihnen fuehrt, wird nicht gerechnet.",
        "der Grund ist `KeineEindeutigeFuehrung`, nicht `KeinMasterbeleg` — A \
         hat sehr wohl gemessen"
    );
    assert_eq!(
        enthaltung.candidate_source, a[0].instance_id,
        "die deterministische Adresse ist die lexikographisch kleinste \
         `instance_id` der Mains; sie behauptet KEINE Fuehrung"
    );
    assert!(
        !enthaltung.evidence_ids.is_empty(),
        "und die Pflichtliste traegt As Belege — ohne sie fiele der Befund an \
         `minItems: 1` und die Sitzung schwiege wieder"
    );
    assert!(
        enthaltung.evidence_ids.len() <= 32,
        "gekappt am alten Ende, wie im Befund: {}",
        enthaltung.evidence_ids.len()
    );
    // Die IDs sind die von A — die einzige Quelle, die ueberhaupt gemessen
    // hat. Ohne die Profile ALLER Mains gaebe es sie in der Aufnahme nicht
    // mehr.
    let ids_von_a: std::collections::BTreeSet<String> = c
        .evidenz_historie(&a[0].instance_id)
        .iter()
        .filter(|e| e.ausschlussgrund.is_none())
        .map(|e| e.evidence_id.clone())
        .collect();
    assert!(
        enthaltung
            .evidence_ids
            .iter()
            .all(|id| ids_von_a.contains(id)),
        "jede genannte ID ist ein GUELTIGER Beleg von A: {:?} gegen {ids_von_a:?}",
        enthaltung.evidence_ids
    );
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-213 R5/E8 · der Kanalwechsel als Messpunktwechsel
// (K-40, K-41, K-42, K-43, K-49)
// ═════════════════════════════════════════════════════════════════════════
//
// Der Mixerkanal lebt im DESKRIPTOR, nicht am Beleg. `aufnahmen_sammeln` liest
// ihn zum Zeitpunkt der Rechnung; die Fenster von vorher blieben unverändert
// gültig, und `routingqualitaet` stieg rückwirkend — ohne dass ein einziges
// Fenster neu gemessen worden wäre (Gate-Befund G-L2).
//
// 🔑 Der produktive Ingress ist der HEARTBEAT, nicht der Setter. Die
// Matrixprüfung 1 hat das belegt: `descriptor_setzen` hat unter `broker/src`
// keinen Aufrufer. Jede Zeile hier fährt deshalb `Senke::p0` mit einem echten
// `heartbeat` — denselben Weg, den `befehl.rs` ruft.

/// Der VERTRAGSGUELTIGE Faehigkeitssatz — alle zehn Namen aus §53.6.
///
/// ⚠️ Der Helfer `capabilities()` dieses Beins traegt noch die Namen vor
/// SONDE-012 (`host_context` statt `host_context_presence` und so weiter).
/// `descriptor_setzen` laesst sie durch, weil `descriptor_vertrag_erfuellt`
/// die Capabilities nicht gegen das Schema haelt — ein `heartbeat` dagegen
/// laeuft durch `v3_nachricht_lesen` und faellt daran (Nebenbefund NB-7).
/// Die Faelle unten fahren deshalb DIESEN Satz; der alte Helfer bleibt
/// unangetastet, weil seine Berichtigung SONDE-012-Testflaeche ist und
/// ausserhalb dieser Ticketgrenze liegt.
fn capabilities_vertragsgueltig() -> Value {
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
        "remote_control": "supported"
    })
}

fn runtime_block(mixer: Option<i64>) -> Value {
    let mut wert = json!({"messpunkt": "post", "betrieb": "active"});
    if let Some(index) = mixer {
        wert["host_mixer_index"] = json!(index);
    }
    wert
}

/// Ein echter Heartbeat über den Produktweg, mit beliebigem `runtime`-Block.
fn heartbeat_mit_runtime(
    c: &Coordinator,
    link: &str,
    a: &Adresse,
    sequence: u64,
    runtime: Value,
) -> bool {
    let wert = json!({
        "type": "heartbeat",
        "adresse": a,
        "sequence": sequence,
        "state_revision": 0,
        "capabilities": capabilities_vertragsgueltig(),
        "zaehler": {"frames_dropped": 0, "parse_errors": 0, "queue_overflows": 0},
        "runtime": runtime
    });
    Senke::p0(c, link, &bytes(&wert)).is_some()
}

/// Derselbe Heartbeat für eine SONDE — Messpunkt `post`.
fn heartbeat_mit_kanal(
    c: &Coordinator,
    link: &str,
    a: &Adresse,
    sequence: u64,
    mixer: Option<i64>,
) -> bool {
    heartbeat_mit_runtime(c, link, a, sequence, runtime_block(mixer))
}

/// Und für den MAIN — `descriptor_aus_heartbeat` weist ein `main` mit einem
/// anderen Messpunkt als `insert` ab (`liveness.rs`:683–685).
fn heartbeat_main_mit_kanal(
    c: &Coordinator,
    link: &str,
    a: &Adresse,
    sequence: u64,
    mixer: Option<i64>,
) -> bool {
    let mut runtime = runtime_block(mixer);
    runtime["messpunkt"] = json!("insert");
    heartbeat_mit_runtime(c, link, a, sequence, runtime)
}

/// Der Abonnent, den die Zustellung braucht — und der Blick auf die Outbox
/// GENAU im Moment der Zustellung.
///
/// 🔑 **Nacharbeit 2 (08.09.2026, Wiederprüfungsbefund zu K-49).** Bis hierher
/// registrierte K-49 zwar eine Subscription, aber keinen `SessionPush`:
/// `Coordinator::mit_store` setzt ihn mit `None`, und `invalidierung_zustellen`
/// hat dann kein Ziel, dem es schreiben könnte. Gemessen war damit nur die
/// EINREIHUNG in die Outbox, nicht die zugesagte Zustellung.
///
/// ⚠️ Beide Hälften sind nur hier zugleich sichtbar. Der Broker trägt eine
/// angenommene Zustellschuld sofort ab (`snapshot_schuld_kompaktieren`,
/// „was zugestellt wurde, wird kompaktiert"); nach dem Lauf ist die Zeile
/// deshalb fort. Diese Probe liest die Outbox im Callback — zwischen Append
/// und Kompaktierung —, und der Test prüft danach BEIDES: die Schuld mit ihrer
/// `event_ord` und ihre Abtragung.
///
/// Der Aufruf läuft synchron auf dem Testfaden (`Senke::p0` →
/// `heartbeat_kontakt` → `invalidierung_zustellen`), und der Coordinator hält
/// dabei keinen Standlock; das Lesen ist eine gewöhnliche kurze
/// Leseverbindung.
struct PushProbe {
    empfangen: std::sync::Mutex<Vec<(String, Value)>>,
    schuld_bei_zustellung: std::sync::Mutex<Vec<(eqcop_broker::store::SnapshotZiel, i64, i64)>>,
    store: eqcop_broker::store::StoreHandle,
}

impl PushProbe {
    fn neu(store: eqcop_broker::store::StoreHandle) -> Self {
        Self {
            empfangen: std::sync::Mutex::new(Vec::new()),
            schuld_bei_zustellung: std::sync::Mutex::new(Vec::new()),
            store,
        }
    }

    fn empfangen(&self) -> Vec<(String, Value)> {
        self.empfangen
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .clone()
    }

    fn schuld_bei_zustellung(&self) -> Vec<(eqcop_broker::store::SnapshotZiel, i64, i64)> {
        self.schuld_bei_zustellung
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .clone()
    }
}

impl eqcop_broker::coordinator::SessionPush for PushProbe {
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
        let wert: Value = serde_json::from_slice(payload).expect("der Push ist JSON");
        if wert["type"] == json!("evidence_invalidate") {
            if let Ok(outbox) = self.store.outbox_lesen() {
                *self
                    .schuld_bei_zustellung
                    .lock()
                    .unwrap_or_else(|e| e.into_inner()) = outbox;
            }
        }
        self.empfangen
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .push((link_id.into(), wert));
        // `true` heisst hier genau das, was der Vertrag sagt: der volle
        // Payload ist geschrieben. Erst damit traegt der Broker die Schuld ab
        // — und erst damit ist „zugestellt" gemessen statt nur eingereiht.
        true
    }
}

/// **K-49 · der tragende Rotbeweis von R5.** Der Kanalwechsel wird auf dem
/// PRODUKTIVEN Weg erkannt: `p0` → `befehl.rs` „heartbeat" →
/// `heartbeat_kontakt` → Deskriptorersatz.
///
/// Eine Fassung, die den Vergleich nur in `descriptor_setzen` einbaut, ließe
/// diesen Lauf unverändert: null Invalidierungen, die acht alten Fenster
/// gälten weiter unter dem neuen Kanal — und alle Settertests wären dabei
/// grün. Gemessen wird der ZÄHLER, nicht die Zeit.
///
/// 🔑 **Nacharbeit 1 (08.09.2026, Erstprüfungsbefund 3): der Broker läuft
/// MIT Store.** Bis hierher benutzte diese Zeile `coordinator()` —
/// `Coordinator::mit_uhr` ohne Store. Die Invalidierung erreichte damit
/// ausschließlich den `Ok(None)`-Zweig („ein Broker ohne Store hält seinen
/// Bestand rein flüchtig"), nie ihre Persistierung und nie ihre Zustellung.
/// Zähler und Befundentfernung konnten grün sein, während genau der
/// produktive Teil fehlerhaft war, den K-49 wörtlich verlangt: „Der Broker
/// läuft mit Store".
///
/// 🔑 **Nacharbeit 2 (08.09.2026, Wiederprüfungsbefund zu K-49): der Abonnent
/// bekommt die Nachricht wirklich.** Die Nacharbeit 1 mass die EINREIHUNG in
/// die Outbox und schrieb daneben, die `evidence_invalidate`-Nachricht sei
/// „dem Abonnenten wirklich geschrieben worden". Gemessen war das nicht: ohne
/// registrierten `SessionPush` — `Coordinator::mit_store` legt ihn mit `None`
/// an — hatte `invalidierung_zustellen` gar kein Ziel. Gemessen wird jetzt die
/// ganze Kette: die acht Belege tragen `messpunkt_wechsel` IM STORE (1), der
/// Abonnent hält den `evidence_invalidate`-Payload mit genau diesen acht IDs,
/// Grund `messpunkt_wechsel` und Umfang `evidence_ids` (2), die Schuld stand
/// dabei mit ihrer `event_ord` in der Outbox (3), und die angenommene
/// Zustellung hat sie abgetragen (4).
#[test]
#[cfg(windows)]
fn kanalwechsel_ueber_den_heartbeat_befehl() {
    let (c, writer, _ordner) = coordinator_mit_store("k49-heartbeat-befehl");
    // 🔑 Nacharbeit 2: die Senke, an die zugestellt wird. `Coordinator::mit_store`
    // legt sie mit `None` an; ohne diese Zeile schreibt `invalidierung_zustellen`
    // niemandem, und „zugestellt" wäre nur nicht widerlegt.
    let push = Arc::new(PushProbe::neu(writer.handle()));
    c.session_push_setzen(push.clone());
    let a = buehne_nak213(&c, 1);

    // 🔑 Nacharbeit 2: die Bühne wird über den PRODUKTIVEN Ingress
    // vertragsgültig gemacht, BEVOR abonniert wird — und die Sonde dabei auf
    // den Kanal 7 gestellt, den K-49 wörtlich nennt.
    //
    // ⚠️ Der Grund ist NB-7 und trifft härter als dort notiert.
    // `anmelden_mit_deskriptor` setzt den Deskriptor über
    // `descriptor_setzen`, und der hält die Capabilities NICHT gegen das
    // Schema; die Bühne trägt deshalb die Namen vor SONDE-012. Der daraus
    // gebaute `session_snapshot` verletzt den v3-Vertrag, und
    // `subscribe_session` reagiert genau so, wie er soll: `routing_fail_closed`
    // (`subscription.rs`:162–167). Ab da ist `routing_bereit` falsch, jeder
    // Link steht auf `trennen`, und `push_ziel_noch_gueltig` gibt für IMMER
    // `false` — die Zustellung dieser Zeile wäre selbst mit registrierter
    // Senke unerreichbar gewesen. Ein Heartbeat trägt den vertragsgültigen
    // Satz und ersetzt den Deskriptor vollständig (`liveness.rs`:472).
    //
    // Diese beiden Heartbeats sind KEIN gemessener Kanalwechsel: es liegt noch
    // kein einziger Beleg vor, `invalidierung_vorbereiten` nimmt nichts
    // zurück, und der Zähler steht danach unverändert bei 0.
    assert!(heartbeat_main_mit_kanal(&c, "main", &a[0], 1, Some(1)));
    assert!(heartbeat_mit_kanal(&c, "sonde0", &a[1], 1, Some(7)));
    assert_eq!(
        c.invalidierungen_zaehler(),
        0,
        "die Anlage der Buehne ist KEIN gemessener Wechsel: es liegt noch \r
         kein Beleg vor, und `invalidierung_vorbereiten` nimmt nichts zurueck"
    );
    assert!(
        c.routing_bereit(),
        "die Bühne ist vertragsgültig — sonst faellt das Routing beim \
         Abonnieren fail-closed, und keine Zustellung dieser Zeile waere \
         erreichbar"
    );

    // Ohne Abonnent hat die Zustellung kein Ziel — und eine Zusage über sie
    // wäre nicht gemessen, sondern nur nicht widerlegt.
    assert!(c.subscribe_json(
        "main",
        &bytes(&json!({
            "type": "subscribe_session",
            "adresse": a[0],
            "session_epoch": a[0].session_epoch
        }))
    ));
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 8);
    let vorher = c.invalidierungen_zaehler();
    assert!(
        !befunde_der_sitzung(&c).is_empty(),
        "Vorbedingung: die Sonde traegt einen Befund"
    );
    // Die acht Belege der Sonde — dieselben IDs, die `reihe` vergibt.
    let acht: Vec<String> = (0..8).map(|i| hex(0x1000 + 100 + i)).collect();
    assert_eq!(
        writer
            .handle()
            .evidenz_belegt(&acht)
            .expect("der Store ist lesbar")
            .len(),
        8,
        "Vorbedingung: alle acht Belege liegen GUELTIG im Store — ohne Store \
         waere diese Zeile gar nicht messbar"
    );

    // Der Kanalwechsel 7 -> 9 ueber den ECHTEN Befehlsweg. Die Sequenz laeuft
    // auf diesem Link fort (1 war die Anlage der Buehne).
    assert!(heartbeat_mit_kanal(&c, "sonde0", &a[1], 2, Some(9)));

    assert_eq!(
        c.invalidierungen_zaehler(),
        vorher + 1,
        "genau EINE Invalidierung — auf dem produktiven Weg, nicht am Setter"
    );
    assert!(
        !befunde_der_sitzung(&c)
            .iter()
            .any(|b| b.candidate_source == a[1].instance_id),
        "der Befund ueber die Sonde ist ENTFERNT, nicht `stale`: er \
         referenziert ausschliesslich ihre eigenen Fenster, und der \
         Kanalwechsel nimmt ALLE davon zurueck (M-28)"
    );

    // 1. PERSISTIERT: keiner der acht Belege gilt im Store noch als gueltig.
    assert!(
        writer
            .handle()
            .evidenz_belegt(&acht)
            .expect("der Store ist lesbar")
            .is_empty(),
        "die acht Belege sind IM STORE ausgeschlossen — nicht nur im \
         fluechtigen Bestand"
    );
    // Und zwar mit dem zugesagten Grund, nicht irgendeinem.
    let zeilen: Vec<Value> = writer
        .handle()
        .domaene_lesen(eqcop_broker::store::Domaenentabelle::Evidence)
        .expect("die Evidenztabelle ist lesbar")
        .iter()
        .filter_map(|roh| serde_json::from_slice::<Value>(roh).ok())
        .filter(|z| {
            z["evidence_id"]
                .as_str()
                .is_some_and(|id| acht.iter().any(|a| a == id))
        })
        .collect();
    assert_eq!(zeilen.len(), 8, "alle acht Zeilen stehen im Store: {zeilen:?}");
    assert!(
        zeilen
            .iter()
            .all(|z| z["ausschlussgrund"] == json!("messpunkt_wechsel")),
        "jede traegt `messpunkt_wechsel` — der Kanalwechsel IST ein \
         Messpunktwechsel, kein dritter Grund: {zeilen:?}"
    );

    // 2. ZUGESTELLT: der Abonnent hat die Nachricht WIRKLICH bekommen.
    //
    // 🔑 Nacharbeit 2. Die Outbox allein belegte nur die EINREIHUNG. Hier
    // steht, was beim Abonnenten ankam — und `invalidierung_zustellen` wird
    // ausschliesslich im Zweig `Ok(Some(event_ord))` gerufen: eine empfangene
    // Nachricht ist deshalb zugleich der Beleg, dass der Append gelaufen ist
    // und nicht der `Ok(None)`-Zweig eines Brokers ohne Store.
    let empfangen: Vec<(String, Value)> = push
        .empfangen()
        .into_iter()
        .filter(|(_, wert)| wert["type"] == json!("evidence_invalidate"))
        .collect();
    assert_eq!(
        empfangen.len(),
        1,
        "genau EINE `evidence_invalidate`-Nachricht beim Abonnenten: {empfangen:?}"
    );
    let (link_der_zustellung, nachricht) = &empfangen[0];
    assert_eq!(
        link_der_zustellung, "main",
        "ueber den Link des Abonnenten, nicht den der wechselnden Sonde: \
         {empfangen:?}"
    );
    assert_eq!(
        nachricht["grund"],
        json!("messpunkt_wechsel"),
        "mit dem zugesagten Grund: {nachricht}"
    );
    assert_eq!(
        nachricht["umfang"]["art"],
        json!("evidence_ids"),
        "und dem Umfang `Ids` — die Belege DIESER Quelle, nicht die ganze \
         Sitzung: {nachricht}"
    );
    let mut zugestellt: Vec<String> = nachricht["umfang"]["evidence_ids"]
        .as_array()
        .expect("`evidence_ids` ist eine Liste")
        .iter()
        .map(|wert| {
            wert.as_str()
                .expect("jede Evidenz-ID ist eine Zeichenkette")
                .to_owned()
        })
        .collect();
    zugestellt.sort();
    let mut erwartet = acht.clone();
    erwartet.sort();
    assert_eq!(
        zugestellt, erwartet,
        "genau die acht Belege der Sonde — keiner mehr, keiner weniger: \
         {nachricht}"
    );

    // 3. PERSISTIERT: im Moment der Zustellung stand die Schuld mit ihrer
    //    `event_ord` aus dem Log in der Outbox. Das ist der Store-Weg
    //    (Modulkopf `invalidierung_verdrahtung.rs`, Punkt 3); ohne Store endet
    //    die Invalidierung in `Ok(None)`, es gibt weder eine `event_ord` noch
    //    eine Outbox, und dieser Zweig laeuft nie.
    let schuld = push.schuld_bei_zustellung();
    let unsere: Vec<&(eqcop_broker::store::SnapshotZiel, i64, i64)> = schuld
        .iter()
        .filter(|(ziel, _, _)| {
            ziel.object_key == "evidence_invalidate"
                && ziel.session_epoch == a[0].session_epoch
                && ziel.instance_id == a[0].instance_id
        })
        .collect();
    assert_eq!(
        unsere.len(),
        1,
        "genau eine Zustellschuld `evidence_invalidate` fuer den Abonnenten \
         dieser Sitzung: {schuld:?}"
    );
    assert!(
        unsere[0].1 > 0,
        "sie traegt die `event_ord` ihres Logeintrags — der Beweis, dass der \
         Zweig `Ok(Some(event_ord))` gelaufen ist und nicht `Ok(None)`: {:?}",
        unsere[0]
    );

    // 4. Und die angenommene Zustellung TRAEGT sie ab: „was zugestellt wurde,
    //    wird kompaktiert" (`invalidierung_zustellen`). Eine Schuld, die nach
    //    einer geglueckten Zustellung stehen bliebe, waere die Zusage von
    //    Punkt 2 ohne ihre Gegenrichtung — der Empfaenger bekaeme sie beim
    //    naechsten Subscribe ein zweites Mal.
    let outbox = writer.handle().outbox_lesen().expect("die Outbox ist lesbar");
    assert!(
        !outbox
            .iter()
            .any(|(ziel, _, _)| ziel.object_key == "evidence_invalidate"),
        "nach der angenommenen Zustellung steht keine `evidence_invalidate`-Schuld \
         mehr offen: {outbox:?}"
    );
}

/// **K-55 (Nacharbeit 1, Erstprüfungsbefund 2).** Der Kanalwechsel wird erst
/// wirksam, wenn seine Invalidierung ANGENOMMEN ist.
///
/// Die Eingabefolge des Prüfers: Main und Sonde liefern zwölf gedeckte,
/// ausgerichtete Fenster mit positivem Zusammenhang, die Sonde zunächst OHNE
/// Mixerindex; danach wird der Store gestoppt und ein Heartbeat mit Kanal 9
/// gesendet.
///
/// 🔑 Der Deskriptor ist bereits ersetzt, wenn `invalidierung_anwenden`
/// scheitert. Ihr Fehlerzweig nahm bis hierher nur die Evidenzausschlüsse
/// zurück und rechnete SOFORT neu — mit dem schon gewechselten Kanal: die
/// zwölf alten Fenster zählten damit unter Kanal 9, und das anschließende
/// `link.trennen = true` verhinderte diese Neuberechnung nicht mehr, weil sie
/// gelaufen war. Genau das verbietet R5. Der Rückweg nimmt den Kanalteil des
/// Deskriptorersatzes zurück, BEVOR der Fehlerzweig rechnet.
#[test]
#[cfg(windows)]
fn kanalwechsel_ohne_store_append_bleibt_beim_alten_kanal() {
    let (c, writer, _ordner) = coordinator_mit_store("k55-kanal-rueckweg");
    let main = adresse(0x11, 0x22, 1, 0x40);
    anmelden_mit_deskriptor(&c, "main", &main, "main", Some(1));
    let marke = json!({
        "type": "intent_update", "adresse": main,
        "session_epoch": main.session_epoch,
        "vollstaendig": true, "bestand_revision": 0
    });
    c.p1("main", &bytes(&marke));
    // Die Sonde meldet ZUNAECHST keinen Mixerindex.
    let sonde = adresse(0x11, 0x22, 2, 0x50);
    anmelden_mit_deskriptor(&c, "sonde0", &sonde, "passive_probe", None);
    reihe_wechselnd(&c, "main", &main, 0, 12);
    reihe_wechselnd(&c, "sonde0", &sonde, 100, 12);

    let befund_der_sonde = |c: &Coordinator| {
        befunde_der_sitzung(c)
            .into_iter()
            .find(|b| b.candidate_source == sonde.instance_id)
    };
    let vorher = befund_der_sonde(&c).expect("Vorbedingung: die Sonde traegt einen Befund");
    assert_eq!(
        vorher.next_test,
        eqcop_broker::coordinator::NaechsterTest::RoutingBestaetigen,
        "Vorbedingung: das Routing der Sonde ist UNBEKANNT — genau der \
         Zustand, den ein Kanalwechsel beenden wuerde: {vorher:?}"
    );
    let ausgeschlossen_vorher = c.evidenz_ausgeschlossen_zaehler();
    let invalidierungen_vorher = c.invalidierungen_zaehler();
    let verweigerungen_vorher = c.store_verweigerungen();

    // Der Store verweigert ab hier JEDEN Append.
    writer.handle().append_naht_setzen(true);
    assert!(heartbeat_mit_kanal(&c, "sonde0", &sonde, 1, Some(9)));

    // (1) Die Wirkung, um die es R5 geht: der Befund ueber die Sonde zaehlt
    //     weiterhin unter „Routing unbekannt". Kein Fenster von vor dem
    //     Wechsel zaehlt je unter dem neuen Kanal. Diese Pruefung steht
    //     VORNE, weil sie die PRODUKTWIRKUNG misst und nicht ihre Ursache.
    let nachher = befund_der_sonde(&c).expect("die Sonde traegt weiterhin einen Befund");
    assert_eq!(
        nachher.next_test,
        eqcop_broker::coordinator::NaechsterTest::RoutingBestaetigen,
        "der Kanalwechsel ist NICHT wirksam geworden: {nachher:?}"
    );
    assert_eq!(
        nachher.rang.routingqualitaet, vorher.rang.routingqualitaet,
        "und die Routingqualitaet ist unveraendert — sie steigt nie ohne          angenommene Ruecknahme"
    );
    assert!(
        nachher.confidence.klasse < Sicherheitsklasse::Hoch,
        "und die Aussage wird nicht STARK: {:?}",
        nachher.confidence
    );

    // (2) Die Ursache dahinter: der Deskriptor traegt wieder KEINEN Kanal —
    //     der Ersatz ist zurueckgenommen, nicht nur seine Invalidierung.
    let snapshot: Value = serde_json::from_slice(&c.session_snapshot_json(&hex(0x11), &hex(0x22)))
        .expect("der Snapshot ist JSON");
    let sonde_im_snapshot = snapshot["mitglieder"]
        .as_array()
        .expect("die Mitglieder sind eine Liste")
        .iter()
        .find(|m| m["adresse"]["instance_id"] == json!(sonde.instance_id))
        .expect("die Sonde steht im Snapshot")
        .clone();
    assert_eq!(
        sonde_im_snapshot["probe_descriptor"].get("host_mixer_index"),
        None,
        "der ALTE Stand steht wieder im Deskriptor: kein Kanal. Sonst zaehlten          die zwoelf Fenster von vor dem Wechsel unter Kanal 9: {sonde_im_snapshot}"
    );

    // (3) Kein Beleg der Sonde traegt einen Ausschlussgrund — die Ruecknahme
    //     der Ausschluesse ist der bestehende B16-Weg und bleibt unberuehrt.
    assert!(
        c.evidenz_historie(&sonde.instance_id)
            .iter()
            .all(|e| e.ausschlussgrund.is_none()),
        "B16: ein gescheiterter Append laesst keinen lokalen Ausschluss stehen"
    );
    assert_eq!(
        c.evidenz_ausgeschlossen_zaehler(),
        ausgeschlossen_vorher,
        "und der Zaehler der ausgeschlossenen Belege steht still"
    );
    assert_eq!(
        c.invalidierungen_zaehler(),
        invalidierungen_vorher,
        "es kommt KEINE Invalidierung ins Log"
    );

    // (4) Der Link gilt als storeverweigert.
    assert!(
        c.store_verweigerungen() > verweigerungen_vorher,
        "der verweigerte Append ist gezaehlt"
    );
}

/// **K-40.** Derselbe Kanal nimmt nichts zurück. Ohne den Vergleich löste
/// JEDER Heartbeat mit `runtime` eine Rücknahme aus — und der kommt in der
/// Kadenz der Liveness, nicht nur beim Wechsel; keine Sonde käme je auf acht
/// Fenster.
#[test]
fn gleicher_kanal_nimmt_nichts_zurueck() {
    let c = coordinator();
    let a = buehne_nak213(&c, 1);
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 8);
    let vorher = c.invalidierungen_zaehler();
    for sequence in 1..=3 {
        assert!(heartbeat_mit_kanal(&c, "sonde0", &a[1], sequence, Some(3)));
    }
    assert_eq!(
        c.invalidierungen_zaehler(),
        vorher,
        "drei Heartbeats auf DEMSELBEN Kanal bewegen den Zaehler nicht"
    );
}

/// **K-41 und K-42.** Beide Richtungen sind ein Wechsel — eine einseitige
/// Prüfung (`nur wenn beide Some`) ließe genau diesen Weg offen, dieselbe
/// Lücke, die der Positionsvergleich heute hat (NB-4).
#[test]
fn kanalwechsel_invalidiert_nur_diese_quelle() {
    // ── K-41: `None` -> `Some(9)` ────────────────────────────────────────
    {
        let c = coordinator();
        let main = adresse(0x11, 0x22, 1, 0x40);
        anmelden_mit_deskriptor(&c, "main", &main, "main", Some(1));
        let marke = json!({
            "type": "intent_update", "adresse": main,
            "session_epoch": main.session_epoch,
            "vollstaendig": true, "bestand_revision": 0
        });
        c.p1("main", &bytes(&marke));
        let ohne = adresse(0x11, 0x22, 2, 0x50);
        anmelden_mit_deskriptor(&c, "sonde0", &ohne, "passive_probe", None);
        reihe(&c, "main", &main, 0, 12);
        reihe(&c, "sonde0", &ohne, 100, 8);
        let vorher = c.invalidierungen_zaehler();
        assert!(heartbeat_mit_kanal(&c, "sonde0", &ohne, 1, Some(9)));
        assert_eq!(
            c.invalidierungen_zaehler(),
            vorher + 1,
            "aus „Routing unbekannt“ wird ein Messpunkt: die Belege davor sind \
             keinem Kanal zuzuordnen"
        );
    }

    // ── K-42: `Some(7)` -> `None` ────────────────────────────────────────
    {
        let c = coordinator();
        let a = buehne_nak213(&c, 1);
        reihe(&c, "main", &a[0], 0, 12);
        reihe(&c, "sonde0", &a[1], 100, 8);
        let vorher = c.invalidierungen_zaehler();
        // Der Deskriptor wird bei gesetztem `runtime` VOLLSTAENDIG ersetzt —
        // das Feld faellt damit wirklich weg.
        assert!(heartbeat_mit_kanal(&c, "sonde0", &a[1], 1, None));
        assert_eq!(
            c.invalidierungen_zaehler(),
            vorher + 1,
            "der Messpunkt ist unbekannt geworden: die alten Belege gehoeren \
             zu einem Kanal, den die Quelle nicht mehr meldet"
        );
    }

    // ── Und der Umfang: NUR diese Quelle ─────────────────────────────────
    {
        let c = coordinator();
        let a = buehne_nak213(&c, 2);
        reihe(&c, "main", &a[0], 0, 12);
        reihe(&c, "sonde0", &a[1], 100, 8);
        reihe(&c, "sonde1", &a[2], 300, 8);
        assert!(heartbeat_mit_kanal(&c, "sonde1", &a[2], 1, Some(9)));
        // Die andere Sonde misst weiter — ihre Belege sind unberuehrt.
        reihe(&c, "sonde0", &a[1], 200, 1);
        let befunde = befunde_der_sitzung(&c);
        assert!(
            befunde
                .iter()
                .any(|b| b.candidate_source == a[1].instance_id),
            "R5 sagt „Befunde, DIE DIE QUELLE TRAGEN“ — der Umfang ist \
             `Ids`, nicht die Sitzung: {befunde:?}"
        );
    }
}

/// **K-43.** Eine Quelle OHNE einen einzigen Beleg wechselt den Kanal: die
/// ID-Menge ist leer, und eine leere Menge ist KEINE Invalidierung. Eine
/// Rücknahme, die nichts zurücknimmt, ließe den Empfänger glauben, es sei
/// aufgeräumt (M-57).
#[test]
fn kanalwechsel_ohne_belege_sendet_nichts() {
    let c = coordinator();
    let a = buehne_nak213(&c, 2);
    reihe(&c, "main", &a[0], 0, 12);
    reihe(&c, "sonde0", &a[1], 100, 8);
    // Sonde 1 hat nie gesendet.
    let vorher = c.invalidierungen_zaehler();
    assert!(heartbeat_mit_kanal(&c, "sonde1", &a[2], 1, Some(9)));
    assert_eq!(
        c.invalidierungen_zaehler(),
        vorher,
        "kein `evidence_invalidate` ohne Inhalt auf der Leitung"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-214 R1 · der Proposal-Riegel am PRODUKTPFAD (V-01 bis V-04, V-11, V-12 b)
// ═════════════════════════════════════════════════════════════════════════
//
// Die Bühne braucht dreierlei, das die Bühnen darüber nicht führen: einen
// Store (`experiment_begin` ist ein persistenzpflichtiger P0), eine benannte
// Passage (ohne sie entsteht seit WN-04 gar kein Proposal) und ein
// eindeutiges führendes Main (ohne das ist `target` leer — genau der Befund
// E-D5 = A6).

const NAK214_BASIS_SAMPLE: i64 = 44_108_200;
const NAK214_TRANSPORT_EPOCHE: u64 = 17;

/// Anmelden mit Deskriptor UND `record_state` — ohne den Bericht weist der
/// Broker jeden persistenzpflichtigen Befehl mit `record_state_unknown` ab,
/// und ohne `experiment_begin` gibt es keine benannte Passage.
fn anmelden_fuer_passage(c: &Coordinator, link: &str, a: &Adresse, art: &str, mixer: Option<i64>) {
    anmelden_mit_deskriptor(c, link, a, art, mixer);
    let bericht = json!({
        "type": "state_report",
        "adresse": a,
        "dsp_schema_version": 1,
        "state_revision": 0,
        "state_hash": "a".repeat(64),
        "record_state": {"valid": true, "recording": false}
    });
    assert!(c.state_report_json(link, &bytes(&bericht)));
}

/// Die benannte Passage über den PRODUKTPFAD: `experiment_begin` als P0.
fn passage_anlegen(
    c: &Coordinator,
    master: &Adresse,
    quellen: &[&Adresse],
    fenster: i64,
    command: usize,
    experiment: usize,
    passage: usize,
) -> String {
    let mut wert = fixture("experiment_begin");
    wert["kopf"]["ziel"] = serde_json::to_value(master).unwrap();
    wert["kopf"]["command_id"] = json!(hex(command));
    wert["kopf"]["base_revision"] = json!(0);
    wert["experiment_id"] = json!(hex(experiment));
    let passage_id = hex(passage);
    wert["passage"]["passage_id"] = json!(passage_id);
    wert["passage"]["projekt_von"] = json!(NAK214_BASIS_SAMPLE);
    wert["passage"]["projekt_bis"] = json!(NAK214_BASIS_SAMPLE + fenster * 512);
    wert["passage"]["transport_epoch"] = json!(NAK214_TRANSPORT_EPOCHE);
    wert["passage"]["aktive_quellen"] = json!(quellen
        .iter()
        .map(|a| a.instance_id.clone())
        .collect::<Vec<_>>());
    // Der Vertrag verlangt `messpunktklassen` in DERSELBEN Reihenfolge und
    // Laenge wie `aktive_quellen` - die Zuordnung Quelle/Messpunkt ist Teil
    // des Belegs (M-28/M-55). Der Master steht vorn und misst `insert`.
    wert["passage"]["messpunktklassen"] = json!(quellen
        .iter()
        .enumerate()
        .map(|(i, _)| if i == 0 { "insert" } else { "post" })
        .collect::<Vec<_>>());
    let antwort = Senke::p0(c, "main", &bytes(&wert)).expect("experiment_begin wird beantwortet");
    let ack: Value = serde_json::from_slice(&antwort).unwrap();
    assert_eq!(
        ack["ergebnis"], "angewandt",
        "die Passage entsteht wirklich: {ack:?}"
    );
    passage_id
}

/// Main plus `sonden` Sonden, benannte Passage, wechselnde Reihen — die
/// Lage, in der ein VOLLSTÄNDIGER Vorschlag entsteht.
fn buehne_mit_passage(c: &Coordinator, sonden: usize, fenster: usize) -> (Vec<Adresse>, String) {
    let main = adresse(0x11, 0x22, 1, 0x40);
    anmelden_fuer_passage(c, "main", &main, "main", Some(1));
    let marke = json!({
        "type": "intent_update",
        "adresse": main,
        "session_epoch": main.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 0
    });
    c.p1("main", &bytes(&marke));
    let mut aus = vec![main];
    for i in 0..sonden {
        let a = adresse(0x11, 0x22, 2 + i, 0x50 + i);
        anmelden_fuer_passage(
            c,
            &format!("sonde{i}"),
            &a,
            "passive_probe",
            Some(3 + i as i64),
        );
        aus.push(a);
    }
    let quellen: Vec<&Adresse> = aus.iter().collect();
    let passage_id = passage_anlegen(c, &aus[0], &quellen, fenster as i64, 0x930, 0xab7, 0x5001);
    reihe_wechselnd(c, "main", &aus[0].clone(), 0, fenster);
    for i in 0..sonden {
        let a = aus[1 + i].clone();
        reihe_wechselnd(c, &format!("sonde{i}"), &a, 100 + i * 100, fenster);
    }
    (aus, passage_id)
}

/// Wie viele `event_type = "proposal"`-Zeilen liegen im Store?
///
/// Gelesen an der Projektion `proposals` (`writer.rs`:573), also an dem, was
/// `vorschlag_persistieren` wirklich abgelegt hat — nicht an einem Zähler.
fn proposals_im_store(writer: &eqcop_broker::store::StoreWriter) -> Vec<Value> {
    let conn = rusqlite::Connection::open(writer.handle().db_pfad()).expect("Store ist lesbar");
    let mut stmt = conn
        .prepare("SELECT state_jcs FROM proposals ORDER BY last_event_ord")
        .expect("die Projektion `proposals` existiert");
    let rows = stmt
        .query_map([], |row| row.get::<_, Vec<u8>>(0))
        .expect("Zeilen lesbar");
    rows.map(|r| serde_json::from_slice(&r.expect("Zeile")).expect("JCS ist JSON"))
        .collect()
}

/// **V-01 (Kontrollfall) und V-02 (der Befund E-D5 = A6, geschlossen).**
///
/// Mit eindeutigem führenden Main entsteht ein Vorschlag, dessen `target`
/// die hex32-ID dieses Mains ist und der alle sechs Gate-Felder trägt. Tritt
/// ein ZWEITES Main derselben Sitzung bei, fällt die Führung auf `None` —
/// und damit entsteht KEIN Vorschlag mehr: nicht im Stand, nicht im Store,
/// nicht als Angebot. Der Befund bleibt, als Enthaltung ohne Ort.
///
/// Rotbeweis `NAK-214-rot-V-02.txt`: der Schlussriegel aus E1 entfernt — ein
/// Proposal mit `target: ""` erscheint in `vorschlaege_sicht` UND im Store.
#[test]
#[cfg(windows)]
fn proposal_traegt_das_fuehrende_main_als_ziel_und_zwei_mains_erzeugen_keinen() {
    let (c, writer, _ordner) = coordinator_mit_store("nak214-v02");
    let (a, passage_id) = buehne_mit_passage(&c, 1, 12);

    // V-01 — der Kontrollfall.
    let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    assert!(
        !vorschlaege.is_empty(),
        "mit eindeutiger Fuehrung entsteht ein Vorschlag"
    );
    for vorschlag in &vorschlaege {
        assert_eq!(
            vorschlag.target, a[0].instance_id,
            "das Ziel ist die hex32-ID des fuehrenden Mains"
        );
        assert_eq!(vorschlag.target.len(), 32, "hex32, nicht der leere String");
        assert_eq!(vorschlag.passage_id.as_deref(), Some(passage_id.as_str()));
        for (feld, belegt) in vorschlag.gate_felder_vollstaendig() {
            assert!(belegt, "Gate-Feld {feld} fehlt am erzeugten Objekt");
        }
    }
    let im_store = proposals_im_store(&writer);
    assert!(
        !im_store.is_empty(),
        "der Kontrollfall legt sein Objekt wirklich ab"
    );
    for objekt in &im_store {
        assert_eq!(
            objekt["target"], a[0].instance_id,
            "und im Store steht dasselbe Ziel"
        );
    }
    let store_vorher = im_store.len();

    // V-02 — ein ZWEITES Main tritt bei: beide verlieren die Bestätigung,
    // und `fuehrung_neu_bewerten_locked` setzt `fuehrendes_main` auf `None`
    // (`mitgliedschaft.rs`, FUEHRENDE_MAINS_PRO_SESSION = 1). Genau dieser
    // Zustand füllte `target` bisher mit dem leeren String.
    let zweites = zweites_main(&c, "main2", 8);
    // Weitere Evidenz loest `hypothesen_bilden` -> `vorschlaege_bilden` aus.
    reihe_wechselnd(&c, "sonde0", &a[1].clone(), 300, 2);

    assert!(
        c.vorschlaege_sicht(&hex(0x11), &hex(0x22)).is_empty(),
        "ohne fuehrendes Main entsteht KEIN Vorschlag"
    );
    assert_eq!(
        proposals_im_store(&writer).len(),
        store_vorher,
        "und es kommt keine `proposal`-Zeile mehr in den Store"
    );
    assert_eq!(
        c.draft_offers_zaehler(),
        0,
        "auch kein Angebot - in P5 ohnehin nie"
    );

    // Der Befund BLEIBT: seit NAK-213 die Enthaltung ohne Ort, mit
    // `next_test` und `listen_for` unveraendert.
    let befunde = befunde_der_sitzung(&c);
    assert_eq!(befunde.len(), 1, "die Enthaltung ohne Ort: {befunde:?}");
    assert_eq!(
        befunde[0].ursachenklasse,
        eqcop_broker::coordinator::Ursachenklasse::DatenReichenNicht
    );
    assert!(
        !befunde[0].listen_for.is_empty(),
        "das Hoerziel steht am Befund - die Anzeige verliert nichts"
    );

    // V-03 — der Rückweg im selben Änderungssatz: zieht sich das zweite Main
    // zurück und wird das erste wieder bestätigt, entsteht der Vorschlag
    // wieder, mit dem Ziel des verbleibenden Mains.
    assert!(c.beitritt_aufheben(&hex(0x11), &hex(0x22), &zweites.instance_id));
    assert!(c.beitritt_bestaetigen(&hex(0x11), &hex(0x22), &a[0].instance_id));
    reihe_wechselnd(&c, "sonde0", &a[1].clone(), 400, 2);
    let wieder = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    assert!(
        !wieder.is_empty(),
        "mit wieder eindeutiger Fuehrung entsteht der Vorschlag erneut"
    );
    for vorschlag in &wieder {
        assert_eq!(vorschlag.target, a[0].instance_id);
    }
}

/// **V-04.** Zieht sich das führende Main zurück, verschwindet der Eintrag
/// beim nächsten `vorschlaege_bilden` aus dem Stand — ein BEREITS
/// persistiertes Proposal bleibt im Store, weil der Store append-only ist
/// und ein gültig entstandenes Ereignis nicht rückwirkend unwahr wird.
///
/// Rotbeweis `NAK-214-rot-V-04.txt`: ohne den Riegel bleibt der Vorschlag mit
/// leerem Ziel im Stand und wird erneut persistiert.
#[test]
#[cfg(windows)]
fn main_rueckzug_zwischen_befund_und_vorschlag() {
    let (c, writer, _ordner) = coordinator_mit_store("nak214-v04");
    let (a, _passage_id) = buehne_mit_passage(&c, 1, 12);
    assert!(
        !c.vorschlaege_sicht(&hex(0x11), &hex(0x22)).is_empty(),
        "Vorbedingung: ein Vorschlag existiert"
    );
    let vorher = proposals_im_store(&writer);
    assert!(!vorher.is_empty());

    // Das führende Main zieht sich zurück.
    assert!(c.beitritt_aufheben(&hex(0x11), &hex(0x22), &a[0].instance_id));
    reihe_wechselnd(&c, "sonde0", &a[1].clone(), 300, 2);

    assert!(
        c.vorschlaege_sicht(&hex(0x11), &hex(0x22)).is_empty(),
        "der Eintrag verschwindet aus dem Stand"
    );
    let nachher = proposals_im_store(&writer);
    assert_eq!(
        nachher.len(),
        vorher.len(),
        "kein NEUES Objekt - und die alten bleiben, der Store ist append-only"
    );
    for objekt in &nachher {
        assert_ne!(
            objekt["target"], "",
            "keine Zeile mit leerem Ziel - weder alt noch neu"
        );
    }
}

/// **V-11.** Fällt der Riegel, wird nichts persistiert und nichts zugestellt
/// — gemessen an den BESTÄNDEN: Storeinhalt, Outbox-Schuld und
/// `vorschlaege_sicht`. Ein neuer Zähler entsteht dafür nicht (§2.9 Nr. 2).
///
/// Rotbeweis `NAK-214-rot-V-11.txt`.
#[test]
#[cfg(windows)]
fn unvollstaendiger_vorschlag_erreicht_weder_store_noch_outbox() {
    let (c, writer, _ordner) = coordinator_mit_store("nak214-v11");
    let (a, _passage_id) = buehne_mit_passage(&c, 1, 12);
    let vorher = proposals_im_store(&writer).len();
    assert!(vorher > 0, "Vorbedingung: der vollstaendige Weg legt ab");

    // Zwei Mains: ab hier faellt der Riegel an `target`.
    let _zweites = zweites_main(&c, "main2", 8);
    reihe_wechselnd(&c, "sonde0", &a[1].clone(), 300, 2);

    assert!(c.vorschlaege_sicht(&hex(0x11), &hex(0x22)).is_empty());
    assert_eq!(
        proposals_im_store(&writer).len(),
        vorher,
        "`vorschlag_persistieren` wird nicht gerufen"
    );
    assert_eq!(
        c.draft_offer_schuld_zaehler(),
        0,
        "und die Outbox traegt keine Zustellschuld"
    );
    let outbox = writer.handle().outbox_lesen().expect("Outbox ist lesbar");
    for (ziel, _, _) in &outbox {
        assert!(
            !ziel.object_key.starts_with("proposal:"),
            "keine Proposal-Schuld in der Outbox: {:?}",
            ziel.object_key
        );
    }
}

/// **V-12 (b), Kontrollfall am echten Produktpfad.** Mehrere Befunde einer
/// Sitzung entstehen aus Evidenz, keiner mit leerem Gate-Feld: jeder trägt
/// seinen Vorschlag, und kein Vorschlag steht ohne Befund.
///
/// Die selektive Hälfte (a) liegt als Unit-Fall in
/// `broker/src/coordinator/proposal_verdrahtung.rs` — der Zustand „genau
/// einem Befund fehlt sein `listen_for`" ist von außen strukturell nicht
/// herstellbar.
#[test]
#[cfg(windows)]
fn vorschlaege_bleiben_ihren_befunden_zugeordnet() {
    let (c, _writer, _ordner) = coordinator_mit_store("nak214-v12b");
    let (_a, _passage_id) = buehne_mit_passage(&c, 2, 12);
    let befunde = befunde_der_sitzung(&c);
    let vorschlaege = c.vorschlaege_sicht(&hex(0x11), &hex(0x22));
    assert!(
        !vorschlaege.is_empty(),
        "die Buehne traegt Vorschlaege: {befunde:?}"
    );
    assert_eq!(
        vorschlaege.len(),
        befunde.len(),
        "so viele Vorschlaege wie Befunde - keiner faellt heraus"
    );
    for vorschlag in &vorschlaege {
        let id = vorschlag
            .finding_id
            .as_deref()
            .expect("jeder Vorschlag nennt seinen Befund");
        assert!(
            befunde.iter().any(|b| b.finding_id == id),
            "kein Vorschlag ohne Befund: {id}"
        );
    }
    for befund in &befunde {
        assert!(
            vorschlaege
                .iter()
                .any(|v| v.finding_id.as_deref() == Some(befund.finding_id.as_str())),
            "und kein Befund ohne Vorschlag: {}",
            befund.finding_id
        );
    }
    assert_eq!(c.draft_offer_schuld_zaehler(), 0, "keine Zustellschuld in P5");
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-214 R2/R6 · der Wirkungsbeleg (V-14 bis V-22, V-45)
// ═════════════════════════════════════════════════════════════════════════
//
// ZWEI Ebenen, und die Trennung ist gemessen, nicht gewählt:
//
// (1) AM COORDINATOR — V-17, V-18, V-20. Das ist der Weg, den G5 E-D6
//     gebrochen hat, und der einzige, den P5 wirklich fährt: die
//     Paarhälften tragen dort `presentation_validiert = false` (M-21,
//     `prepost_verdrahtung.rs`:305), also erreicht das Urteil nie mehr als
//     `Probable`.
// (2) AN `hypothesen()` — V-14, V-15, V-16, V-21, V-22, V-45. Sie brauchen
//     ein Paar OBERHALB der Schwelle, und das ist am Produktpfad
//     strukturell unerreichbar (siehe (1)). `hypothesen()` ist `pub`, die
//     `Aufnahme` und das `Quellprofil` ebenso — die Wirkung der Marke auf
//     `claim_class`, `ursachenklasse` und `next_test` ist dort direkt
//     messbar, ohne eine Lage zu behaupten, die es in P5 nicht gibt.
//
// Das Prädikat SELBST — die vier Punkte aus E3/R6 — misst
// `broker/tests/sonde013_prepost.rs` an `bilde_paare`.

/// Anmelden mit `pair_id` und ausdrücklichem Messpunkt — ohne beides gibt es
/// keine Rolle und damit keine Paarhälfte.
fn anmelden_mit_paar(
    c: &Coordinator,
    link: &str,
    a: &Adresse,
    position: &str,
    pair_id: &str,
    mixer: Option<i64>,
) {
    let mut h = hello(a.clone());
    h.plugin_kind = "passive_probe".into();
    let ausgang = c.control_hello_registrieren(link, &h);
    assert!(ausgang.angenommen, "{:?}", ausgang.grund);
    let _ = c.resync_bestaetigen(link, 0);
    let mut d = json!({
        "adresse": a,
        "plugin_kind": "passive_probe",
        "measurement_position": position,
        "aussageklasse": "beobachtend",
        "betrieb": "active",
        "label": "Testquelle",
        "pair_id": pair_id,
        "capabilities": capabilities(),
        "frische": {"letzter_kontakt_ms": 10, "stale": false}
    });
    if let Some(index) = mixer {
        d["host_mixer_index"] = json!(index);
    }
    assert!(c.descriptor_setzen(link, d), "der Deskriptor ist vertragsgueltig");
}

/// Main plus PRE/POST-Paar einer `pair_id`, mit Evidenz auf beiden Hälften.
fn buehne_mit_paar(c: &Coordinator, pair_id: &str) -> Vec<Adresse> {
    let main = adresse(0x11, 0x22, 1, 0x40);
    anmelden_mit_deskriptor(c, "main", &main, "main", Some(1));
    let marke = json!({
        "type": "intent_update",
        "adresse": main,
        "session_epoch": main.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 0
    });
    c.p1("main", &bytes(&marke));
    let pre = adresse(0x11, 0x22, 2, 0x50);
    let post = adresse(0x11, 0x22, 3, 0x51);
    anmelden_mit_paar(c, "pre", &pre, "pre", pair_id, Some(3));
    anmelden_mit_paar(c, "post", &post, "post", pair_id, Some(4));
    // ⚠️ Zwei Eigenschaften dieser Reihe sind TRAGEND, und beide sind an der
    // Quelle nachgerechnet:
    //
    // (1) Die PROJEKTSPANNE. `beurteile_paar` bildet den Suchraum aus
    //     `spanne_s`, und `suchraum_frames` liefert unterhalb weniger
    //     Zehntelsekunden NULL Frames — `schaetze_restlag` gaebe dann
    //     grundsaetzlich `None`, und JEDES Paar bliebe `Unclear`, egal wie
    //     sauber das Material ist. Zwoelf Fenster im 512-Sample-Abstand sind
    //     0,128 s; hier liegen sie 24 000 Samples auseinander, also gut 6 s.
    // (2) APERIODIZITAET. Der Restlagschaetzer korreliert die Huellkurven;
    //     eine konstante oder periodische Kurve hat kein EINDEUTIGES Maximum.
    let mut x: u64 = 0x9E3779B97F4A7C15;
    let anhebungen: Vec<i64> = (0..12)
        .map(|_| {
            x = x
                .wrapping_mul(6_364_136_223_846_793_005)
                .wrapping_add(1_442_695_040_888_963_407);
            ((x >> 33) % 120) as i64
        })
        .collect();
    for (i, anhebung) in anhebungen.iter().enumerate() {
        let zeit = 44_108_200 + (i as i64) * 24_000;
        c.p1("main", &evidenz_mit_anhebung(&main, i, zeit, *anhebung));
        c.p1("pre", &evidenz_mit_anhebung(&pre, 100 + i, zeit, *anhebung));
        // DIESELBE Folge fuer die POST-Haelfte: ein Paar mit Lag 0.
        c.p1("post", &evidenz_mit_anhebung(&post, 200 + i, zeit, *anhebung));
    }
    vec![main, pre, post]
}

/// **V-17 — der Befund E-D6, geschlossen; der tragende Fall von R2.**
///
/// Am Produktpfad entsteht IMMER ein `Probable`-Urteil, und `beurteile_paar`
/// gibt dafür `ergebnis: Some(..)` zurück. Bis zu diesem Ticket trug der
/// Befund daraus `claim_class = wirkungsbeleg`, `ursachenklasse =
/// effektkette_pre_post` und ein `next_test`, das kein Paar mehr verlangte —
/// obwohl das Paar weder ein ausgerichtetes Delta noch eine Wirkung trug.
///
/// Rotbeweis `NAK-214-rot-V-17.txt`: `prepost_wirkungsbeleg` wieder aus
/// `ergebnis.is_some()` gesetzt.
#[test]
fn probables_paar_bleibt_klasse_eins() {
    let c = coordinator();
    let paar = hex(0x77);
    let _a = buehne_mit_paar(&c, &paar);

    let urteil = c
        .paarurteil(&paar)
        .expect("der PRE/POST-Join liefert ein Urteil");
    assert!(
        urteil.klasse < eqcop_broker::coordinator::prepost::Alignmentklasse::AudioAligned,
        "Vorbedingung: am Produktpfad bleibt es unter der Schwelle (M-21): {:?}",
        urteil.klasse
    );
    assert!(
        urteil.ergebnis.is_some(),
        "Vorbedingung: und es traegt TROTZDEM ein Ergebnis - genau daraus entstand E-D6: {urteil:?}"
    );

    let befunde = befunde_der_sitzung(&c);
    assert!(!befunde.is_empty(), "die Buehne traegt Befunde");
    let mit_paar: Vec<_> = befunde
        .iter()
        .filter(|b| b.pre_post == Some("post"))
        .collect();
    assert!(
        !mit_paar.is_empty(),
        "mindestens ein Befund gehoert zu einer Quelle des Paares: {befunde:?}"
    );
    for b in &mit_paar {
        assert_eq!(
            b.claim_class,
            eqcop_broker::coordinator::Aussageklasse::Zusammenhang,
            "V-17: ein Paar unterhalb der Schwelle bleibt Klasse 1"
        );
        assert_ne!(
            b.ursachenklasse,
            eqcop_broker::coordinator::Ursachenklasse::EffektkettePrePost,
            "V-17: und die Ursachenklasse behauptet keine Kette"
        );
        assert_eq!(
            b.next_test,
            eqcop_broker::coordinator::NaechsterTest::PrePostPaarMessen,
            "V-17: `next_test` verlangt das Paar weiterhin (R2 woertlich)"
        );
        // E4: die ORTSANGABE bleibt - eine Quelle misst an einem POST-Punkt,
        // auch wenn ihr Paar nur `Probable` erreicht.
        assert_eq!(b.pre_post, Some("post"));
    }
}

/// **V-20 — eine `pair_id` ohne Urteil setzt keine der beiden Marken.**
#[test]
fn paar_id_ohne_urteil_setzt_keine_marke() {
    let c = coordinator();
    let paar = hex(0x78);
    let main = adresse(0x11, 0x22, 1, 0x40);
    anmelden_mit_deskriptor(&c, "main", &main, "main", Some(1));
    let marke = json!({
        "type": "intent_update",
        "adresse": main,
        "session_epoch": main.session_epoch,
        "vollstaendig": true,
        "bestand_revision": 0
    });
    c.p1("main", &bytes(&marke));
    // NUR die POST-Haelfte: ohne Gegenstueck entsteht kein Urteil mit
    // Ergebnis, und beide Marken bleiben falsch.
    let post = adresse(0x11, 0x22, 3, 0x51);
    anmelden_mit_paar(&c, "post", &post, "post", &paar, Some(4));
    reihe_wechselnd(&c, "main", &main, 0, 12);
    reihe_wechselnd(&c, "post", &post, 200, 12);

    let befunde = befunde_der_sitzung(&c);
    assert!(!befunde.is_empty());
    for b in &befunde {
        assert_eq!(
            b.claim_class,
            eqcop_broker::coordinator::Aussageklasse::Zusammenhang
        );
        assert_eq!(b.pre_post, None, "ohne Urteil MIT Ergebnis auch keine Ortsangabe");
        assert_eq!(
            b.next_test,
            eqcop_broker::coordinator::NaechsterTest::PrePostPaarMessen
        );
    }
}

// ── Die Modulebene: die Wirkung der Marke auf Klasse und `next_test` ──────
//
// V-14, V-15, V-16, V-21, V-22 und V-45 brauchen ein Paar OBERHALB der
// Schwelle. Am Produktpfad ist das strukturell unerreichbar (M-21, siehe
// `probables_paar_bleibt_klasse_eins`), und eine Bühne, die es behauptete,
// behauptete eine Lage, die es in P5 nicht gibt. Gemessen wird deshalb an
// `hypothesen()` — derselben Funktion, die der Coordinator ruft —, mit der
// Marke als Eingabe. Das PRÄDIKAT, aus dem die Marke entsteht, misst
// `sonde013_prepost.rs` an `bilde_paare`.

use eqcop_broker::coordinator::hypothese::{
    bandintervall_der_gruppe, gruppe_von_band, hypothesen, Aufnahme, Evidenzfenster, Quellprofil,
    BAENDER_FEIN,
};

/// Ein Evidenzfenster mit angehobener Gruppe — dieselbe Form wie im
/// Modultestmodul von `hypothese.rs`.
fn nak214_fenster(von: i64, spanne: (usize, usize), db: f32) -> Evidenzfenster {
    let mut f = Evidenzfenster {
        evidence_id: format!("{von:032x}"),
        empfangsfolge: von.unsigned_abs(),
        projekt_von: von,
        projekt_bis: von + 500,
        transport_epoch: 1,
        abdeckung: 0.9,
        p50_db: vec![-20.0; BAENDER_FEIN],
        p50_gueltig: vec![true; BAENDER_FEIN],
        onset: 1.0,
        seitenanteil_db: None,
        hat_baender: true,
        hat_verteilung: true,
        hat_stereo: false,
    };
    for index in spanne.0..spanne.1.min(BAENDER_FEIN) {
        f.p50_db[index] = -20.0 + db;
    }
    f
}

/// Ein Quellprofil mit acht Fenstern, bekanntem Routing und den beiden
/// Paarmarken.
fn nak214_quelle(
    id: &str,
    spanne: (usize, usize),
    db: f32,
    kanal: i64,
    prepost_paar: bool,
    prepost_wirkungsbeleg: bool,
) -> Quellprofil {
    Quellprofil {
        quelle_id: id.into(),
        fenster: (0..8)
            .map(|i| nak214_fenster(1000 + (i as i64) * 500, spanne, db))
            .collect(),
        routing_bekannt: true,
        mixerkanal: Some(kanal),
        parent: None,
        prepost_paar,
        prepost_wirkungsbeleg,
        zurueckgenommen: false,
    }
}

fn nak214_aufnahme(master: Quellprofil, kandidaten: Vec<Quellprofil>) -> Aufnahme {
    Aufnahme {
        mains: vec![master.clone()],
        master: Some(master),
        kandidaten,
        passage: None,
        passage_id: None,
        intent: Some(eqcop_broker::coordinator::IntentBestand {
            vollstaendig: true,
            ..Default::default()
        }),
        metrics_version: 1,
        session_epoch: "e".repeat(32),
    }
}

/// Die Grundlage der Modulfälle: ein Master und ein Kandidat im selben Band,
/// die Marken als Parameter.
fn nak214_befunde(
    prepost_paar: bool,
    prepost_wirkungsbeleg: bool,
) -> Vec<eqcop_broker::coordinator::CauseHypothesis> {
    let spanne = {
        let b = bandintervall_der_gruppe(gruppe_von_band(100));
        (b.von as usize, b.bis as usize)
    };
    let master = nak214_quelle("m", spanne, 12.0, 1, false, false);
    let kandidat = nak214_quelle(
        &format!("{:032x}", 0x20),
        spanne,
        9.0,
        2,
        prepost_paar,
        prepost_wirkungsbeleg,
    );
    hypothesen(&nak214_aufnahme(master, vec![kandidat])).befunde
}

/// **V-14 und V-16.** Trägt das Paar einen gemessenen Wirkungsbeleg, entsteht
/// Klasse 2 — mit der Ursachenklasse der Kette und einem `next_test`, der das
/// Paar nicht mehr verlangt.
///
/// Rotbeweis `NAK-214-rot-V-16.txt`: die Schwelle in
/// `Paarurteil::wirkungsbeleg` von `>=` auf `==` gesetzt — dann trägt kein
/// Paar in P5 je einen Wirkungsbeleg, und dieser Fall fällt.
#[test]
fn wirkungsbeleg_aus_ausgerichtetem_paar() {
    let befunde = nak214_befunde(true, true);
    assert!(!befunde.is_empty(), "die Aufnahme traegt einen Befund");
    let b = &befunde[0];
    assert_eq!(
        b.claim_class,
        eqcop_broker::coordinator::Aussageklasse::Wirkungsbeleg,
        "V-14/V-16: ein Paar MIT gemessener Wirkung traegt Klasse 2"
    );
    assert_eq!(
        b.ursachenklasse,
        eqcop_broker::coordinator::Ursachenklasse::EffektkettePrePost,
        "und die Ursache ist die Kette dieser Quelle"
    );
    assert_ne!(
        b.next_test,
        eqcop_broker::coordinator::NaechsterTest::PrePostPaarMessen,
        "der naechste Beweisschritt verlangt das Paar nicht mehr"
    );
    assert_eq!(b.pre_post, Some("post"), "die Ortsangabe steht (E4)");
}

/// **V-45 und V-15.** Ein Paar OHNE gemessene Wirkung — gleich, ob es an
/// Punkt 2 (kein gültiges Band) oder an Punkt 4 (alles exakt null) scheitert
/// — bleibt Klasse 1, und `next_test` verlangt das Paar weiterhin.
///
/// Die ORTSANGABE bleibt trotzdem gesetzt: `prepost_paar` ist unberührt
/// (E4). Rotbeweis `NAK-214-rot-V-45.txt`.
#[test]
fn paar_ohne_gemessene_wirkung_bleibt_klasse_eins() {
    let befunde = nak214_befunde(true, false);
    assert!(!befunde.is_empty());
    let b = &befunde[0];
    assert_eq!(
        b.claim_class,
        eqcop_broker::coordinator::Aussageklasse::Zusammenhang,
        "V-45/V-15: ohne gemessene Wirkung bleibt es Klasse 1"
    );
    assert_ne!(
        b.ursachenklasse,
        eqcop_broker::coordinator::Ursachenklasse::EffektkettePrePost
    );
    assert_eq!(
        b.next_test,
        eqcop_broker::coordinator::NaechsterTest::PrePostPaarMessen,
        "R2 woertlich: `next_test` verlangt das Paar weiterhin"
    );
    assert_eq!(
        b.pre_post,
        Some("post"),
        "E4: die Ortsangabe haengt an `prepost_paar` und bleibt"
    );
}

/// **V-18.** Ohne Paarurteil mit Ergebnis ist auch die Ortsangabe fort.
#[test]
fn ausgeschlossenes_paar_setzt_keine_marke() {
    let befunde = nak214_befunde(false, false);
    assert!(!befunde.is_empty());
    let b = &befunde[0];
    assert_eq!(
        b.claim_class,
        eqcop_broker::coordinator::Aussageklasse::Zusammenhang
    );
    assert_eq!(b.pre_post, None);
    assert_eq!(
        b.next_test,
        eqcop_broker::coordinator::NaechsterTest::PrePostPaarMessen
    );
}

/// **V-21.** Die Marke wirkt JE KANDIDAT, nicht je Aufnahme — und sie
/// berührt die Sicherheitsklasse nicht. Beides sind zwei Achsen
/// (`hypothese.rs`, unverändert seit SONDE-014).
#[test]
fn wirkungsbeleg_wirkt_je_kandidat() {
    let spanne = {
        let b = bandintervall_der_gruppe(gruppe_von_band(100));
        (b.von as usize, b.bis as usize)
    };
    let master = nak214_quelle("m", spanne, 12.0, 1, false, false);
    let mit = nak214_quelle(&format!("{:032x}", 0x20), spanne, 9.0, 2, true, true);
    let ohne = nak214_quelle(&format!("{:032x}", 0x21), spanne, 8.5, 3, true, false);
    let befunde = hypothesen(&nak214_aufnahme(master, vec![mit, ohne])).befunde;
    assert!(befunde.len() >= 2, "zwei Kandidaten, zwei Befunde: {befunde:?}");
    let a = befunde
        .iter()
        .find(|b| b.candidate_source == format!("{:032x}", 0x20))
        .expect("der Kandidat MIT Wirkungsbeleg");
    let b = befunde
        .iter()
        .find(|b| b.candidate_source == format!("{:032x}", 0x21))
        .expect("der Kandidat OHNE");
    assert_eq!(
        a.claim_class,
        eqcop_broker::coordinator::Aussageklasse::Wirkungsbeleg
    );
    assert_eq!(
        b.claim_class,
        eqcop_broker::coordinator::Aussageklasse::Zusammenhang,
        "nur der EINE traegt den Beleg"
    );
    // Die Sicherheitsklasse bleibt unberuehrt - sie haengt ausdruecklich
    // nicht an der Aussageklasse.
    assert_eq!(
        a.confidence.klasse, b.confidence.klasse,
        "die Aussageklasse bewegt keine Sicherheitsklasse: {:?} gegen {:?}",
        a.confidence, b.confidence
    );
}

/// **V-22.** Der Weg aus Klasse 1 heraus existiert: dasselbe Paar, später mit
/// gemessener Wirkung, hebt Klasse und `next_test`.
///
/// Ohne ihn wäre `next_test = PrePostPaarMessen` eine Aufforderung ohne Ziel
/// (aktivieren ↔ abklingen).
#[test]
fn besseres_paar_hebt_die_klasse() {
    let vorher = nak214_befunde(true, false);
    let nachher = nak214_befunde(true, true);
    assert_eq!(
        vorher[0].claim_class,
        eqcop_broker::coordinator::Aussageklasse::Zusammenhang
    );
    assert_eq!(
        vorher[0].next_test,
        eqcop_broker::coordinator::NaechsterTest::PrePostPaarMessen
    );
    assert_eq!(
        nachher[0].claim_class,
        eqcop_broker::coordinator::Aussageklasse::Wirkungsbeleg,
        "V-22: der Weg aus Klasse 1 heraus existiert"
    );
    assert_ne!(
        nachher[0].next_test,
        eqcop_broker::coordinator::NaechsterTest::PrePostPaarMessen
    );
    assert_eq!(
        vorher[0].finding_id, nachher[0].finding_id,
        "es ist derselbe Befund - nur seine Aussagekraft ist gewachsen"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-214 R4 · der `user_verdict`-Riegel (V-30 bis V-36)
// ═════════════════════════════════════════════════════════════════════════
//
// `user_verdict_p0` legte das Urteil bisher ab, ohne dass der Befund
// existieren musste (G5-Befund E-L5). Der Riegel prüft die EXISTENZ und
// nichts sonst — der Zustand des Befunds ist keine Bedingung (E8).

/// Ein `user_verdict` als P0-Nachricht, aus dem committeten Korpus.
fn user_verdict_wert(ziel: &Adresse, command: usize, finding_id: &str) -> Value {
    let mut wert = fixture("user_verdict");
    wert["kopf"]["ziel"] = serde_json::to_value(ziel).unwrap();
    wert["kopf"]["command_id"] = json!(hex(command));
    wert["kopf"]["base_revision"] = json!(0);
    wert["finding_id"] = json!(finding_id);
    wert["user_verdict_id"] = json!(hex(0xc0c0 + command));
    wert
}

/// Die Antwort auf ein `user_verdict`, über den echten P0-Weg.
fn urteil_senden(c: &Coordinator, wert: &Value) -> Value {
    let antwort = Senke::p0(c, "main", &bytes(wert)).expect("user_verdict wird beantwortet");
    serde_json::from_slice(&antwort).expect("das Ack ist JSON")
}

/// **V-30, V-31, V-32 (Kontrollfälle) und V-33, V-34 (der Befund E-L5).**
///
/// Ein Urteil auf einen existenten Befund wird angenommen und persistiert —
/// gleich, in welchem Zustand er ist. Eines auf eine `finding_id` mit 32
/// gültigen Hexzeichen, die kein Befund trägt, wird abgewiesen, und es liegt
/// danach NICHTS im Store.
///
/// Rotbeweis `NAK-214-rot-V-33.txt`.
#[test]
#[cfg(windows)]
fn user_verdict_ohne_befund_wird_abgewiesen() {
    let (c, writer, _ordner) = coordinator_mit_store("nak214-v33");
    let (a, _passage_id) = buehne_mit_passage(&c, 1, 12);
    let befunde = befunde_der_sitzung(&c);
    assert!(!befunde.is_empty(), "die Buehne traegt Befunde");
    let existent = befunde[0].finding_id.clone();

    // V-30 — der Kontrollfall: ein existenter Befund.
    let ack = urteil_senden(&c, &user_verdict_wert(&a[0], 0x9401, &existent));
    assert_eq!(
        ack["ergebnis"], "angewandt",
        "V-30: ein Urteil auf einen existenten Befund wird angewandt: {ack:?}"
    );
    assert!(
        ack["state_hash"].is_string(),
        "und traegt seinen state_hash: {ack:?}"
    );
    assert!(
        writer
            .handle()
            .command_event_lesen(&hex(0x9401))
            .expect("Store ist lesbar")
            .is_some(),
        "der Befehl liegt im Store"
    );

    // V-33 — 32 gueltige Hexzeichen, die kein Befund traegt.
    let unbekannt = hex(0xdead);
    assert_eq!(unbekannt.len(), 32, "der Zahlenrand: gueltige Form, unbekannt");
    assert!(
        !befunde.iter().any(|b| b.finding_id == unbekannt),
        "Vorbedingung: die Sitzung fuehrt diesen Befund nicht"
    );
    let ack = urteil_senden(&c, &user_verdict_wert(&a[0], 0x9402, &unbekannt));
    assert_eq!(
        ack["ergebnis"], "abgelehnt",
        "V-33: ein Urteil ohne Befund wird abgewiesen: {ack:?}"
    );
    assert_eq!(
        ack["code"], "unknown_target",
        "und zwar mit dem Code, den der C++-Leser kennt (E7): {ack:?}"
    );
    assert!(
        writer
            .handle()
            .command_event_lesen(&hex(0x9402))
            .expect("Store ist lesbar")
            .is_none(),
        "V-33: NICHTS wird persistiert - kein command_event"
    );

    // V-36 — dieselbe Abweisung noch einmal: stabil, weil nichts committet
    // wurde. Idempotenz auch im Abweisungszweig.
    let wieder = urteil_senden(&c, &user_verdict_wert(&a[0], 0x9402, &unbekannt));
    assert_eq!(wieder["ergebnis"], "abgelehnt");
    assert_eq!(wieder["code"], "unknown_target");
    assert_eq!(
        wieder["state_revision"], ack["state_revision"],
        "V-36: dieselbe Antwort mit derselben Revision"
    );
}

/// **V-31 und V-32.** `more_data` und `stale` sind existente Befunde — der
/// Riegel greift NICHT zu weit. R4 nennt ausschließlich die Existenz (E8).
///
/// Die Zustände kommen aus dem Broker selbst: `zustand_aus_sicherheit`
/// bildet sie, und die Bühne trägt beide Klassen.
#[test]
#[cfg(windows)]
fn user_verdict_auf_more_data_und_stale_wird_angenommen() {
    let (c, writer, _ordner) = coordinator_mit_store("nak214-v31");
    let (a, _passage_id) = buehne_mit_passage(&c, 2, 12);
    let befunde = befunde_der_sitzung(&c);
    assert!(!befunde.is_empty());

    let nicht_ready: Vec<_> = befunde
        .iter()
        .filter(|b| b.zustand != eqcop_broker::coordinator::Befundzustand::ReadyToSend)
        .collect();
    assert!(
        !nicht_ready.is_empty(),
        "Vorbedingung: die Buehne traegt mindestens einen Befund, der NICHT \
         handelbar ist: {:?}",
        befunde.iter().map(|b| b.zustand).collect::<Vec<_>>()
    );
    for (i, b) in nicht_ready.iter().enumerate() {
        let command = 0x9410 + i;
        let ack = urteil_senden(&c, &user_verdict_wert(&a[0], command, &b.finding_id));
        assert_eq!(
            ack["ergebnis"], "angewandt",
            "V-31/V-32: der Zustand {:?} ist keine Bedingung: {ack:?}",
            b.zustand
        );
        assert!(
            writer
                .handle()
                .command_event_lesen(&hex(command))
                .expect("Store ist lesbar")
                .is_some(),
            "und das Urteil liegt im Store"
        );
    }
}

/// **V-35.** Der Riegel steht HINTER dem Idempotenzblock: ein bereits
/// angewandter Befehl bleibt dieselbe Wirkung, auch wenn sein Befund
/// inzwischen fort ist.
///
/// Ohne diese Reihenfolge bekäme derselbe `command_id` zwei verschiedene
/// Antworten, und das persistierte Urteil bliebe trotzdem liegen.
///
/// Rotbeweis `NAK-214-rot-V-35.txt`.
#[test]
#[cfg(windows)]
fn wiederholtes_urteil_bleibt_idempotent_auch_ohne_befund() {
    let (c, _writer, _ordner) = coordinator_mit_store("nak214-v35");
    let (a, _passage_id) = buehne_mit_passage(&c, 1, 12);
    let befunde = befunde_der_sitzung(&c);
    let id = befunde[0].finding_id.clone();
    let wert = user_verdict_wert(&a[0], 0x9420, &id);

    let erste = urteil_senden(&c, &wert);
    assert_eq!(erste["ergebnis"], "angewandt", "{erste:?}");

    // Der Befund faellt: das fuehrende Main zieht sich zurueck, die Rechnung
    // laeuft neu, und die Sitzung fuehrt ihn nicht mehr unter dieser ID.
    assert!(c.beitritt_aufheben(&hex(0x11), &hex(0x22), &a[0].instance_id));
    reihe_wechselnd(&c, "sonde0", &a[1].clone(), 300, 2);
    assert!(
        !befunde_der_sitzung(&c).iter().any(|b| b.finding_id == id),
        "Vorbedingung: der Befund ist fort: {:?}",
        befunde_der_sitzung(&c)
            .iter()
            .map(|b| b.finding_id.clone())
            .collect::<Vec<_>>()
    );

    let wieder = urteil_senden(&c, &wert);
    assert_eq!(
        wieder["ergebnis"], "idempotent_wiederholt",
        "V-35: ein bereits angewandter Befehl bleibt dieselbe Wirkung: {wieder:?}"
    );
    assert_eq!(
        wieder["state_hash"], erste["state_hash"],
        "mit demselben state_hash"
    );
}
