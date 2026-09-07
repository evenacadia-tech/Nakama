//! Die Brokerhälfte der Befundzustände (SONDE-014 Etappe D, M-29 bis M-35).
//!
//! ── DIE ARBEITSTEILUNG ────────────────────────────────────────────────────
//!
//! Abnahme U21 legt die Abbildung Sicherheit → Zustand in den **Datenweg**,
//! und der Datenweg ist hier zweigeteilt: der Broker **rechnet** den Zustand
//! (`hypothese::zustand_aus_sicherheit`), Gen **sperrt** danach die Aktion
//! (`SourcesModel::Befund::darfDraft`, Bein **B28**). Beide Hälften messen
//! dieselbe Zusage von ihrer Seite — das ist M-77 in klein.
//!
//! Dieses Bein misst die Brokerhälfte am PRODUKTPFAD: die Evidenz geht durch
//! `p1`, der Intent geht durch `p1`, und gelesen wird am `session_snapshot`.

use eqcop_broker::coordinator::{
    Befundzustand, Coordinator, ManualClock, Sicherheitsklasse,
};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
use eqcop_broker::transport::server_v3::Senke;
use serde_json::{json, Value};
use std::sync::Arc;

// ── Werkzeug ─────────────────────────────────────────────────────────────
//
// Bewusst dieselbe Bühne wie `sonde014_hypothese.rs`, aber eigenständig: eine
// gemeinsame Hilfsdatei zwischen zwei Integrationszielen wäre ein drittes
// Ziel, das niemand baut, und ein `include!` wäre eine zweite Wahrheit.

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

fn coordinator() -> Coordinator {
    Coordinator::mit_uhr(Arc::new(ManualClock::default()), hex(0xbeef))
}

fn capabilities() -> Value {
    json!({
        "host_context": true, "project_time": true, "fine_automation": false,
        "double_precision": false, "latency_report": false, "aux_send": false,
        "aux_return": false, "compare_routing": false, "sidechain": false,
        "offline_render": false
    })
}

fn anmelden(c: &Coordinator, link: &str, a: &Adresse, art: &str, mixer: Option<i64>) {
    let h = hello(a.clone(), art);
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

fn evidenz(a: &Adresse, nr: usize, projekt_start: i64) -> Vec<u8> {
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

fn findings(c: &Coordinator) -> Vec<Value> {
    let snapshot: Value =
        serde_json::from_slice(&c.session_snapshot_json(&hex(0x11), &hex(0x22))).unwrap();
    snapshot
        .get("findings")
        .and_then(Value::as_array)
        .cloned()
        .unwrap_or_default()
}

/// **M-86, NR-01:** der leere Vollbestand MIT Marke, bevor gerechnet wird.
///
/// Seit der Nacharbeit 1 (07.09.2026) ist die Sperre fail-closed: ohne Marke
/// rechnet der Broker gar nicht — auch nicht bei `intent == None`. Jede
/// Buehne, die einen Befund erwartet, meldet deshalb zuerst den Bestand, so
/// wie der Main es tut (`sendeIntentVollbestand`).
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

/// Ein Master und eine Sonde, beide angemeldet, mit `fenster` Belegen je Seite.
fn buehne(c: &Coordinator, fenster: usize) -> Vec<Adresse> {
    let master = adresse(1);
    let sonde = adresse(2);
    anmelden(c, "main", &master, "main", Some(0));
    anmelden(c, "sonde0", &sonde, "passive_probe", Some(3));
    // Reihenfolge aus M-86: anmelden -> Vollbestand mit Marke -> erst dann
    // Evidenz und Rechnung.
    intent_marke(c, "main", &master);
    reihe(c, "main", &master, 0, fenster);
    reihe(c, "sonde0", &sonde, 100, fenster);
    vec![master, sonde]
}

// ═════════════════════════════════════════════════════════════════════════
// M-29 · sicherheit_wird_auf_drei_zustaende_abgebildet
// ═════════════════════════════════════════════════════════════════════════
//
// Die Abbildung ist eine FUNKTION, keine Anzeigeentscheidung — und sie ist
// total: jede der drei Sicherheitsstufen hat genau einen Zustand, und ein
// veralteter Befund hat ihn unabhaengig von seiner Stufe.
#[test]
fn sicherheit_wird_auf_drei_zustaende_abgebildet() {
    use eqcop_broker::coordinator::hypothese::zustand_aus_sicherheit;
    assert_eq!(
        zustand_aus_sicherheit(Sicherheitsklasse::Hoch, false),
        Befundzustand::ReadyToSend
    );
    assert_eq!(
        zustand_aus_sicherheit(Sicherheitsklasse::Mittel, false),
        Befundzustand::MoreData
    );
    assert_eq!(
        zustand_aus_sicherheit(Sicherheitsklasse::Unklar, false),
        Befundzustand::MoreData
    );
    // Veraltet schlaegt JEDE Stufe — auch `hoch`. Sonst bliebe ein Befund
    // handelbar, dem der User gerade die Grundlage entzogen hat.
    for klasse in [
        Sicherheitsklasse::Hoch,
        Sicherheitsklasse::Mittel,
        Sicherheitsklasse::Unklar,
    ] {
        assert_eq!(
            zustand_aus_sicherheit(klasse, true),
            Befundzustand::Stale,
            "{klasse:?}"
        );
    }
    // Und der Produktpfad gibt nur Woerter aus dieser Menge aus.
    let c = coordinator();
    let _ = buehne(&c, 12);
    let f = findings(&c);
    assert!(!f.is_empty());
    for befund in &f {
        let wort = befund["zustand"].as_str().unwrap();
        assert!(
            Befundzustand::aus_wire(wort).is_some(),
            "{wort} steht nicht in der Menge"
        );
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-30 · nur_ready_to_send_erlaubt_audition_und_draft
// ═════════════════════════════════════════════════════════════════════════
//
// Die Sperre haengt am Zustand, und sie haengt an BEIDEN Aktionen. Ein
// Zustand, der die eine erlaubte und die andere nicht, waere eine dritte
// Regel — U21 kennt nur eine.
#[test]
fn nur_ready_to_send_erlaubt_audition_und_draft() {
    assert!(Befundzustand::ReadyToSend.erlaubt_draft());
    assert!(Befundzustand::ReadyToSend.erlaubt_audition());
    for zustand in [Befundzustand::MoreData, Befundzustand::Stale] {
        assert!(!zustand.erlaubt_draft(), "{zustand:?}");
        assert!(!zustand.erlaubt_audition(), "{zustand:?}");
    }
    // Am Produktpfad: eine Sitzung mit zu wenig Fenstern liefert `more_data`,
    // und damit ist der Draft gesperrt — nicht ausgegraut, sondern gesperrt.
    let c = coordinator();
    let _ = buehne(&c, 3);
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    let befund = befunde.first().expect("ein Befund");
    assert_eq!(befund.zustand, Befundzustand::MoreData);
    assert!(!befund.zustand.erlaubt_draft());
    // Gegenprobe: mit genug Fenstern ist derselbe Weg offen. Ohne sie waere
    // die Sperre eine Regressionswache und kein Beleg.
    let d = coordinator();
    let _ = buehne(&d, 12);
    let offen = d.befunde_sicht(&hex(0x11), &hex(0x22));
    assert_eq!(offen[0].zustand, Befundzustand::ReadyToSend);
    assert!(offen[0].zustand.erlaubt_draft());
}

// ═════════════════════════════════════════════════════════════════════════
// M-29 (STALE-Quelle) · rollenaenderung_macht_den_befund_stale
// ═════════════════════════════════════════════════════════════════════════
//
// §37.3 wörtlich: „Steigt sie, geht der Befund sichtbar in `stale`, OHNE dass
// Zahlen nachgerechnet werden." Beides wird gemessen: der Zustand wechselt,
// und die Zahlen darunter stehen unveraendert.
#[test]
fn hoehere_intent_revision_macht_den_befund_stale() {
    let c = coordinator();
    let adressen = buehne(&c, 12);
    let vorher = c.befunde_sicht(&hex(0x11), &hex(0x22));
    let befund = vorher.first().expect("ein Befund").clone();
    assert_eq!(befund.zustand, Befundzustand::ReadyToSend);

    // Der User setzt eine Rolle: die Bestandsrevision steigt.
    let intent = json!({
        "type": "intent_update",
        "adresse": adressen[0],
        "session_epoch": adressen[0].session_epoch,
        "vollstaendig": true,
        "bestand_revision": befund.intent_revision + 1,
        "intents": [{
            "quelle_id": adressen[1].instance_id,
            "rolle": "geschuetzt",
            "revision": 1,
            "herkunft": "user",
            "konfidenz": 1.0
        }]
    });
    c.p1("main", &serde_json::to_vec(&intent).unwrap());

    let nachher = c.befunde_sicht(&hex(0x11), &hex(0x22));
    let jetzt = nachher.first().expect("der Befund steht noch da");
    assert_eq!(
        jetzt.zustand,
        Befundzustand::Stale,
        "eine gestiegene Intent-Revision entwertet den Befund sichtbar"
    );
    assert!(!jetzt.zustand.erlaubt_draft());
    // OHNE Nachrechnen: Rang, Beobachtung und Belege stehen unveraendert.
    assert_eq!(jetzt.rang, befund.rang, "die Zahlen wurden nicht nachgerechnet");
    assert_eq!(jetzt.beobachtung, befund.beobachtung);
    assert_eq!(jetzt.evidence_ids, befund.evidence_ids);
    assert_eq!(jetzt.finding_id, befund.finding_id);
    // Und Gen sieht es: der Snapshot traegt den neuen Zustand.
    let f = findings(&c);
    assert_eq!(f[0]["zustand"].as_str(), Some("stale"));
}

// ═════════════════════════════════════════════════════════════════════════
// M-32 · alternative_ist_ein_eigener_befund_mit_eigenem_zustand
// ═════════════════════════════════════════════════════════════════════════
//
// Die Alternative steht als EIGENER Befund in der Liste, mit eigenem Zustand,
// und der fuehrende nennt nur ihre ID. Kein Anhang, kein Freitext.
#[test]
fn alternative_ist_ein_eigener_befund_mit_eigenem_zustand() {
    let c = coordinator();
    let master = adresse(1);
    anmelden(&c, "main", &master, "main", Some(0));
    let a = adresse(2);
    let b = adresse(3);
    anmelden(&c, "sonde0", &a, "passive_probe", Some(3));
    // Die zweite Sonde teilt sich den Kanal mit der ersten: sie bleibt im
    // Rennen, traegt aber keine starke Aussage (M-22).
    anmelden(&c, "sonde1", &b, "passive_probe", Some(3));
    intent_marke(&c, "main", &master);
    reihe(&c, "main", &master, 0, 12);
    reihe(&c, "sonde0", &a, 100, 12);
    reihe(&c, "sonde1", &b, 200, 12);

    let f = findings(&c);
    assert_eq!(f.len(), 2, "beide sind eigene Befunde");
    let alternativen = f[0]["alternatives"].as_array().expect("IDs am fuehrenden");
    assert_eq!(alternativen.len(), 1);
    // Die ID zeigt auf einen Befund, der WIRKLICH in der Liste steht.
    let ziel = alternativen[0].as_str().unwrap();
    assert_eq!(f[1]["finding_id"].as_str(), Some(ziel));
    // Und die Alternative traegt ihren eigenen Zustand.
    assert!(f[1]["zustand"].is_string());
    // Kein Freitext: die Liste besteht aus hex32.
    assert_eq!(ziel.len(), 32);
    assert!(ziel.chars().all(|c| c.is_ascii_hexdigit()));
}

// ═════════════════════════════════════════════════════════════════════════
// M-33 · beleg_ist_zone_und_kein_text
// ═════════════════════════════════════════════════════════════════════════
//
// Der Datenweg liefert die markierte Zone und ihre Quelle — und **kein**
// Belegtextfeld. Der Vertrag hat gar keines; gemessen wird, dass der Erzeuger
// auch keines erfindet.
#[test]
fn beleg_ist_zone_und_kein_text() {
    let c = coordinator();
    let _ = buehne(&c, 12);
    let f = findings(&c);
    let befund = &f[0];
    let von = befund["band_hz"]["von"].as_u64().unwrap();
    let bis = befund["band_hz"]["bis"].as_u64().unwrap();
    assert!(von < bis && bis <= 221, "{von}..{bis}");
    assert!(befund["candidate_source"].is_string());
    for verbotenes in ["beleg", "beleg_text", "begruendung", "erklaerung"] {
        assert!(
            befund.get(verbotenes).is_none(),
            "der Befund traegt {verbotenes}"
        );
    }
    // Und keine Zeichenanweisung (M-80): weder Farbe noch Achse noch Zoom.
    for optik in ["farbe", "achse", "zoom", "ausschnitt"] {
        assert!(befund.get(optik).is_none(), "der Befund traegt {optik}");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-34 · drei_zeilen_sind_datenfelder
// ═════════════════════════════════════════════════════════════════════════
//
// Drei Felder, jedes gefuellt, und `smallest_test` ist die Abbildung von
// `next_test` — gemessen daran, dass die Abbildung TOTAL ist: jeder der sechs
// Werte hat seinen eigenen Satz.
#[test]
fn drei_zeilen_sind_datenfelder() {
    use eqcop_broker::coordinator::hypothese::{NaechsterTest, NAECHSTE_TESTS};
    let mut saetze: Vec<&str> = NaechsterTest::ALLE.iter().map(|t| t.satz()).collect();
    assert_eq!(saetze.len(), NAECHSTE_TESTS.len());
    assert!(saetze.iter().all(|s| !s.is_empty()), "kein leerer Satz");
    saetze.sort_unstable();
    // `passage_messen` und `mehr_daten_sammeln` teilen sich bewusst denselben
    // Satz — beides heisst „laenger messen". Mehr als eine Doppelung waere
    // ein Zeichen, dass die Abbildung Werte zusammenwirft.
    let vorher = saetze.len();
    saetze.dedup();
    assert_eq!(vorher - saetze.len(), 1, "genau eine bewusste Doppelung");

    let c = coordinator();
    let _ = buehne(&c, 12);
    let f = findings(&c);
    let befund = &f[0];
    for zeile in ["likely_cause", "smallest_test", "listen_for"] {
        let text = befund[zeile].as_str().unwrap_or_default();
        assert!(!text.is_empty(), "{zeile} ist leer");
        assert!(text.chars().count() <= 200, "{zeile} ueber der Grenze");
    }
    // Die Zeile bildet den Wert ab, den der Befund wirklich traegt.
    let test = NaechsterTest::aus_wire(befund["next_test"].as_str().unwrap()).unwrap();
    assert_eq!(befund["smallest_test"].as_str(), Some(test.satz()));
}

// ═════════════════════════════════════════════════════════════════════════
// M-35 · messqualitaet_und_befundsicherheit_sind_zwei_felder
// ═════════════════════════════════════════════════════════════════════════
//
// `konfidenz.klasse` des Belegs ist die Messqualitaet der Passage; die
// Sicherheit des Befunds ist etwas anderes. Der Beleg dieser Buehne traegt
// `mittel` — und der Befund erreicht trotzdem `hoch`, weil seine Sicherheit
// aus den Gates kommt und nicht aus der Messqualitaet.
#[test]
fn messqualitaet_und_befundsicherheit_sind_zwei_felder() {
    let c = coordinator();
    let _ = buehne(&c, 12);
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    let befund = befunde.first().expect("ein Befund");
    // Die Fixture meldet `konfidenz.klasse = mittel`.
    let beleg_klasse = evidenz_grundform()["konfidenz"]["klasse"]
        .as_str()
        .unwrap()
        .to_string();
    assert_eq!(beleg_klasse, "mittel");
    assert_eq!(
        befund.confidence.klasse,
        Sicherheitsklasse::Hoch,
        "die Befundsicherheit folgt den Gates, nicht der Messqualitaet"
    );
    // Und der Snapshot fuehrt beide getrennt: die Messqualitaet steht am
    // Mitglied, die Sicherheit am Befund.
    let snapshot: Value =
        serde_json::from_slice(&c.session_snapshot_json(&hex(0x11), &hex(0x22))).unwrap();
    let f = snapshot["findings"].as_array().unwrap();
    assert_eq!(f[0]["confidence"]["class"].as_str(), Some("hoch"));
    assert!(
        f[0].get("konfidenz").is_none(),
        "die Messqualitaet reist nicht im Befund mit"
    );
}
