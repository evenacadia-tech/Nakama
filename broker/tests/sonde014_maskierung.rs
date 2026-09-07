//! Der Maskierungs-Datenweg am Produktpfad (SONDE-014 Etappe E, M-36 bis M-41).
//!
//! ── WAS HIER GEMESSEN WIRD UND WAS NICHT ──────────────────────────────────
//!
//! Dieses Ticket liefert den **Datenweg**, nicht die Zone. Die farbige Zone
//! auf der EQ-Fläche ist Anzeige und gehört S31b (Abnahme U16, PR2). Gemessen
//! wird deshalb genau dreierlei: dass der Wert **aus dem Befund herausfällt**,
//! dass er einen **Frequenzbereich des bestehenden Gitters** benennt, und dass
//! er **keine Zeichenanweisung** trägt.
//!
//! Die Gen-Hälfte — der Leser und die Trennung „nicht vorhanden" gegen „nicht
//! gemessen" — liegt im Bein **B28**.

use eqcop_broker::coordinator::{Coordinator, ManualClock};
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

/// Ein Snapshot mit gesetzter Adresse, Projektzeit und Bandanhebung.
///
/// `anhebung_db` wird auf `ANOMALIEBAND..+4` addiert; die Kodierung ist
/// `q_db_0p1_i16`, also Zehntel-dB als Ganzzahl.
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

/// Master und Sonde, `fenster` zusammenhängende Belege je Seite. Der Master
/// hebt seine Anomaliebänder um `master_db`, die Sonde um `sonde_db`.
fn buehne(c: &Coordinator, fenster: usize, master_db: f64, sonde_db: f64) -> Vec<Adresse> {
    let master = adresse(1);
    let sonde = adresse(2);
    anmelden(c, "main", &master, "main", 0);
    anmelden(c, "sonde0", &sonde, "passive_probe", 3);
    intent_marke(c, "main", &master);
    for i in 0..fenster {
        let zeit = 44_108_200 + (i as i64) * 512;
        c.p1("main", &evidenz(&master, i, zeit, master_db));
        c.p1("sonde0", &evidenz(&sonde, 100 + i, zeit, sonde_db));
    }
    vec![master, sonde]
}

// ═════════════════════════════════════════════════════════════════════════
// M-36 · maskierung_haengt_am_finding_und_benennt_einen_frequenzbereich
// ═════════════════════════════════════════════════════════════════════════
//
// Zwei Hälften, und beide fallen einzeln: der Wert steht **im** Befund, und er
// trägt ein Bandintervall des eingefrorenen Gitters — keine zweite Liste, kein
// zweites Raster.
#[test]
fn maskierung_haengt_am_finding_und_benennt_einen_frequenzbereich() {
    let c = coordinator();
    let adressen = buehne(&c, 12, 9.0, 9.0);
    let snapshot: Value =
        serde_json::from_slice(&c.session_snapshot_json(&hex(0x11), &hex(0x22))).unwrap();

    // Es gibt KEINE zweite Liste neben `findings`.
    for verdaechtig in ["maskierungen", "maskierung", "zonen", "ueberdeckungen"] {
        assert!(
            snapshot.get(verdaechtig).is_none(),
            "der Snapshot traegt eine zweite Liste {verdaechtig}"
        );
    }

    let f = snapshot["findings"].as_array().expect("Befunde");
    let m = f[0]["maskierung"]
        .as_object()
        .expect("der Wert haengt AM Befund");
    // Der Bereich ist derselbe wie der des Befunds — eine zweite Achse gäbe es
    // sonst schon hier.
    assert_eq!(m["band_von"], f[0]["band_hz"]["von"]);
    assert_eq!(m["band_bis"], f[0]["band_hz"]["bis"]);
    let von = m["band_von"].as_u64().unwrap();
    let bis = m["band_bis"].as_u64().unwrap();
    assert!(von < bis && bis <= 221, "{von}..{bis}");
    // Und er nennt beide Quellen: die maskierte und die maskierende.
    assert_eq!(m["quelle_a"].as_str(), Some(adressen[0].instance_id.as_str()));
    assert_eq!(m["quelle_b"].as_str(), Some(adressen[1].instance_id.as_str()));
    assert_eq!(m["gueltig"].as_bool(), Some(true));
}

// ═════════════════════════════════════════════════════════════════════════
// M-37 · maskierungswert_ist_hoechstens_ein_evidenzintervall_alt
// ═════════════════════════════════════════════════════════════════════════
//
// Gemessen wird das **Alter relativ zur jüngsten Evidenz**, nicht eine
// Wallclock-Rate: der Wert folgt dem letzten Fenster beider Quellen. Wird die
// Sonde lauter, ändert sich der Wert mit dem nächsten Beleg — nicht später.
#[test]
fn maskierungswert_ist_hoechstens_ein_evidenzintervall_alt() {
    // Der Master traegt seine Anomalie (sonst gaebe es keinen Befund in diesem
    // Bandbereich), die Sonde liegt darunter.
    let c = coordinator();
    let adressen = buehne(&c, 12, 9.0, 6.0);
    let vorher = findings(&c)[0]["maskierung"]["wert_db"]
        .as_f64()
        .expect("ein Wert");
    // Zwoelf gleiche Fenster: die Sonde liegt unter dem Master, der Wert ist
    // negativ.
    assert!(vorher < 0.0, "die Sonde liegt unter dem Master: {vorher}");

    // EIN weiterer Beleg der Sonde, deutlich lauter. Der Wert muss ihm folgen.
    let zeit = 44_108_200 + 12 * 512;
    c.p1("main", &evidenz(&adressen[0], 12, zeit, 9.0));
    c.p1("sonde0", &evidenz(&adressen[1], 200, zeit, 18.0));
    let nachher = findings(&c)[0]["maskierung"]["wert_db"]
        .as_f64()
        .expect("ein Wert");

    assert!(
        nachher > vorher + 5.0,
        "der Wert folgt der juengsten Evidenz: {vorher} -> {nachher}"
    );
    // Und er ist KEIN Mittel ueber die Historie. Zwoelf Fenster bei 6 dB und
    // eines bei 18 dB ergaeben gemittelt kaum mehr als der Ausgangswert; der
    // Wert springt stattdessen auf die Differenz der beiden JUENGSTEN Fenster.
    //
    // Nachgerechnet: die Basis ist −12,3 dB, angehoben werden die Baender 98
    // und 99 innerhalb des Bereichs [96, 100). Master: 2·10^−1,23 + 2·10^−0,33
    // = 1,053 → +0,23 dB. Sonde bei +18: 2·10^−1,23 + 2·10^0,57 = 7,549
    // → +8,78 dB. Die Differenz ist rund 8,55 dB.
    assert!(
        (nachher - 8.55).abs() < 0.2,
        "die Differenz der beiden juengsten Bandpegel ist rund 8,55 dB, gemessen {nachher}"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-37 · rueckstau_setzt_das_kennzeichen_herabgesetzt
// ═════════════════════════════════════════════════════════════════════════
//
// Bei reduzierter Kadenz trägt der Wert ein **Kennzeichen** — nie einen
// interpolierten Zwischenwert. Der Broker misst die Reduktion an dem, was er
// selbst sieht: die Fenster liegen nicht mehr aneinander.
#[test]
fn rueckstau_setzt_das_kennzeichen_herabgesetzt() {
    // Dicht: kein Kennzeichen.
    let c = coordinator();
    let _ = buehne(&c, 12, 0.0, 6.0);
    assert_eq!(
        findings(&c)[0]["maskierung"]["herabgesetzt"].as_bool(),
        Some(false),
        "eine dichte Reihe ist nicht herabgesetzt"
    );

    // Dieselbe Buehne, aber die Sonde laesst zwischen den beiden juengsten
    // Fenstern eine Luecke von mehr als einer Fensterlaenge.
    let d = coordinator();
    let adressen = buehne(&d, 12, 0.0, 6.0);
    let zeit = 44_108_200 + 12 * 512;
    d.p1("main", &evidenz(&adressen[0], 12, zeit, 0.0));
    d.p1("sonde0", &evidenz(&adressen[1], 200, zeit + 8 * 512, 6.0));
    let m = &findings(&d)[0]["maskierung"];
    assert_eq!(
        m["herabgesetzt"].as_bool(),
        Some(true),
        "eine Luecke ist eine Kadenzreduktion"
    );
    // Der Wert selbst bleibt eine MESSUNG: er ist gueltig und nicht
    // interpoliert.
    assert_eq!(m["gueltig"].as_bool(), Some(true));
    assert!(m["wert_db"].as_f64().unwrap().is_finite());
}

// ═════════════════════════════════════════════════════════════════════════
// M-38 / M-39 / M-80 · datenweg_traegt_keine_zeichenanweisung
// ═════════════════════════════════════════════════════════════════════════
//
// Der Datenweg liefert den Bereich — und **nur** den. Keine Achse, kein
// Ausschnitt, kein Zoom, keine Farbe. Ein Zoom ist eine bewusste Useraktion
// und kann aus dem Datenweg gar nicht kommen: es gibt kein Feld dafür.
#[test]
fn datenweg_traegt_keine_zeichenanweisung() {
    let c = coordinator();
    let _ = buehne(&c, 12, 0.0, 6.0);
    let snapshot = String::from_utf8(c.session_snapshot_json(&hex(0x11), &hex(0x22))).unwrap();
    for optik in [
        "farbe", "color", "achse", "axis", "zoom", "ausschnitt", "viewport", "alpha", "pixel",
    ] {
        assert!(
            !snapshot.contains(optik),
            "der Snapshot traegt eine Zeichenanweisung: {optik}"
        );
    }
    // Der Maskierungswert hat GENAU die sieben Vertragsfelder.
    let m = findings(&c)[0]["maskierung"].as_object().unwrap().clone();
    let mut felder: Vec<&String> = m.keys().collect();
    felder.sort();
    assert_eq!(
        felder,
        vec![
            "band_bis",
            "band_von",
            "gueltig",
            "herabgesetzt",
            "quelle_a",
            "quelle_b",
            "wert_db"
        ]
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-41 · zone_hat_keine_eigene_schwelle
// ═════════════════════════════════════════════════════════════════════════
//
// Der Datenweg kennt keinen Grenzwert: ein winziger Wert reist genauso wie ein
// großer. Eine Schwelle hier wäre eine zweite Detektion neben dem Befund — und
// eine Maskierung ohne Befund gibt es nicht.
#[test]
fn zone_hat_keine_eigene_schwelle() {
    // Sonde und Master gleich laut: der Wert ist rund 0 dB — und reist.
    let c = coordinator();
    let _ = buehne(&c, 12, 6.0, 6.0);
    let f = findings(&c);
    let m = f[0]["maskierung"].as_object().expect("auch 0 dB reist");
    let wert = m["wert_db"].as_f64().unwrap();
    assert!(wert.abs() < 0.5, "gleiche Pegel ergeben rund 0 dB: {wert}");
    assert_eq!(m["gueltig"].as_bool(), Some(true));
    // Kein Feld, das eine Schwelle oder Sichtbarkeit trüge.
    for schwelle in ["schwelle", "sichtbar", "mindestwert", "threshold"] {
        assert!(m.get(schwelle).is_none(), "der Wert traegt {schwelle}");
    }

    // Und keine Maskierung ohne Befund: ohne Kandidat gibt es einen Befund
    // (die Enthaltung), und der trägt keinen Wert.
    let d = coordinator();
    let master = adresse(1);
    anmelden(&d, "main", &master, "main", 0);
    intent_marke(&d, "main", &master);
    for i in 0..12 {
        d.p1(
            "main",
            &evidenz(&master, i, 44_108_200 + (i as i64) * 512, 9.0),
        );
    }
    let allein = findings(&d);
    assert_eq!(allein.len(), 1);
    assert!(
        allein[0].get("maskierung").is_none(),
        "eine Enthaltung traegt keinen Maskierungswert"
    );
}
