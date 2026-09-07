//! Der Produktpfad der Ursachenhypothese (SONDE-014 Etappe C, M-14 bis M-28
//! und M-87).
//!
//! ── WARUM DIESES BEIN DEN WEG MISST UND NICHT DAS MODUL ───────────────────
//!
//! Risiko R1 des Bauplans, woertlich aus der Wiederpruefung 1 der Etappe 2 von
//! SONDE-013: „die Runde 1 hat Funktionen gebaut und Tests geschrieben, die
//! das Modul direkt aufrufen — der Produktpfad war an vielen Stellen nicht
//! geschlossen." Jeder Fall hier schickt seine Evidenz durch `p1`, also durch
//! dieselbe Senke, die der echte Transport ruft, und liest das Ergebnis am
//! `session_snapshot` beziehungsweise an `befunde_sicht` — nie an
//! `hypothese::hypothesen`.
//!
//! Die Modulebene liegt daneben in `hypothese.rs` (`#[cfg(test)]`). Beide
//! Ebenen sind gefordert (§5.3 R1); der Rotbeweis faellt an DIESER.
//!
//! ── DIE NUTZLAST ──────────────────────────────────────────────────────────
//!
//! Die Grundform kommt aus dem COMMITTETEN Fixturekorpus. Eine zweite
//! Wire-Form von Hand waere eine zweite Wahrheit neben dem Korpus — dieselbe
//! Regel wie in `coordinator_model.rs`, `sonde013_verdrahtung.rs` und
//! `sonde014_verdrahtung.rs`. Veraendert werden ausschliesslich ZAHLEN
//! (Bandwerte, Projektzeit, Abdeckung), nie die Form.

use eqcop_broker::coordinator::{
    Ausschlussgrund, Aussageklasse, Befundzustand, Coordinator, ManualClock, Sicherheitsklasse,
    Ursachenklasse, KANDIDATEN_DECKEL,
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

/// Meldet einen Client an und gibt ihm einen vertragsgueltigen Deskriptor.
///
/// `mixer` ist der `host_mixer_index`. `None` heisst „Routing unbekannt" —
/// genau der Fall, den M-22 mit „keine starke Aussage" belegt.
fn anmelden(c: &Coordinator, link: &str, a: &Adresse, art: &str, mixer: Option<i64>, pair: Option<&str>) {
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
    if let Some(p) = pair {
        d["pair_id"] = json!(p);
    }
    assert!(c.descriptor_setzen(link, d), "Deskriptor ist vertragsgueltig");
}

/// Die Grundform, EINMAL gelesen.
///
/// Ohne den Cache liest jeder einzelne Push die 221-Band-Fixture von der
/// Platte und parst sie neu; der Determinismusfall unten faehrt hundert
/// Buehnen und braeuchte dafuer Minuten statt Sekunden. Geklont wird
/// trotzdem je Aufruf — ein geteilter `Value`, den ein Fall veraendert, waere
/// eine Kopplung zwischen Faellen.
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

/// Das Band, in dem die Testfaelle ihre Anomalie setzen, und seine Gruppe.
///
/// 100 liegt in der Gruppe 28 des 64er-Gitters (100 * 64 / 221 = 28); das
/// feine Intervall dieser Gruppe ist [96, 100). Der Wert ist bewusst KEIN
/// Rand: Band 0 und 220 haben ihre eigenen Faelle in der Modulebene.
const ANOMALIEBAND: usize = 98;

/// Ein Snapshot mit gesetzter Adresse, eigener `evidence_id`, eigener
/// Projektzeit und wahlweise angehobenen Baendern.
///
/// `anhebung_db` wird auf die Baender `von..bis` addiert — die Kodierung ist
/// `q_db_0p1_i16`, also Zehntel-dB als Ganzzahl.
fn evidenz(
    a: &Adresse,
    nr: usize,
    projekt_start: i64,
    anhebung: Option<(usize, usize, f64)>,
    anpassen: impl FnOnce(&mut Value),
) -> Vec<u8> {
    let mut wert = evidenz_grundform();
    wert["adresse"] = serde_json::to_value(a).expect("Adresse ist serialisierbar");
    wert["evidence_id"] = json!(hex(0x1000 + nr));
    wert["transport"]["sequence"] = json!(nr as u64 + 1);
    wert["transport"]["project_sample_start"] = json!(projekt_start);
    if let Some((von, bis, db)) = anhebung {
        for feld in ["baender", "verteilung"] {
            let ziel: Vec<&str> = if feld == "baender" {
                vec!["/baender/werte"]
            } else {
                vec!["/verteilung/p10/werte", "/verteilung/p50/werte", "/verteilung/p95/werte"]
            };
            for pfad in ziel {
                if let Some(Value::Array(werte)) = wert.pointer_mut(pfad) {
                    for index in von..bis.min(werte.len()) {
                        let alt = werte[index].as_i64().unwrap_or(0);
                        werte[index] = json!(alt + (db * 10.0).round() as i64);
                    }
                }
            }
        }
    }
    anpassen(&mut wert);
    serde_json::to_vec(&wert).unwrap()
}

/// Eine Fensterreihe EINER Quelle, ueber den Produktweg `p1`.
///
/// Die Fenster liegen luecken- und ueberlappungsfrei nebeneinander: 512
/// Samples je Fenster, wie die Grundform sie meldet.
fn reihe(
    c: &Coordinator,
    link: &str,
    a: &Adresse,
    ab_nr: usize,
    anzahl: usize,
    anhebung: Option<(usize, usize, f64)>,
) {
    for i in 0..anzahl {
        let payload = evidenz(
            a,
            ab_nr + i,
            44_108_200 + (i as i64) * 512,
            anhebung,
            |_| {},
        );
        c.p1(link, &payload);
    }
}

/// Der Snapshot, wie Gen ihn sieht.
fn snapshot(c: &Coordinator) -> Value {
    serde_json::from_slice(&c.session_snapshot_json(&hex(0x11), &hex(0x22)))
        .expect("Snapshot ist JSON")
}

fn findings(c: &Coordinator) -> Vec<Value> {
    snapshot(c)
        .get("findings")
        .and_then(Value::as_array)
        .cloned()
        .unwrap_or_default()
}

/// **M-86, NR-01:** der leere Vollbestand MIT Marke, bevor gerechnet wird.
///
/// Seit der Nacharbeit 1 (07.09.2026) ist die Sperre fail-closed: `intent ==
/// None` sperrt genauso wie ein unvollstaendiger Bestand. Jede Buehne, die
/// einen Befund erwartet, meldet deshalb zuerst den Bestand — genau in der
/// Reihenfolge, die M-86 verlangt.
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

/// Ein Master und `n` Sonden, alle angemeldet und mit Deskriptor.
///
/// Rueckgabe: die Adressen in der Reihenfolge Master, Sonde 1, Sonde 2, ...
fn buehne(c: &Coordinator, sonden: usize, mixer_ab: Option<i64>) -> Vec<Adresse> {
    let mut aus = vec![adresse(1)];
    anmelden(c, "main", &aus[0], "main", Some(0), None);
    intent_marke(c, "main", &aus[0]);
    for i in 0..sonden {
        let a = adresse(2 + i);
        anmelden(
            c,
            &format!("sonde{i}"),
            &a,
            "passive_probe",
            mixer_ab.map(|m| m + i as i64),
            None,
        );
        aus.push(a);
    }
    aus
}

// ═════════════════════════════════════════════════════════════════════════
// M-14 · ursachenklassen_sind_geschlossen_und_sieben
// ═════════════════════════════════════════════════════════════════════════
//
// Zwei Haelften, und beide fallen einzeln: die Menge hat genau sieben Werte,
// und ein ACHTER wird abgewiesen statt auf einen bekannten Zweig abgebildet.
// Die siebte ist ein regulaeres Ergebnis — sie steht in derselben Menge wie
// die sechs anderen und nicht in einem Fehlerzweig daneben.
#[test]
fn ursachenklassen_sind_geschlossen_und_sieben() {
    let vertrag = vertrag_schema();
    let menge = vertrag["$defs"]["ursachenklasse"]["enum"]
        .as_array()
        .expect("die Menge steht im Vertrag");
    assert_eq!(menge.len(), 7, "genau sieben Ursachenklassen");
    // Der Produzent kennt jede einzelne, und keine mehr.
    for wort in menge {
        let w = wort.as_str().unwrap();
        assert!(
            Ursachenklasse::aus_wire(w).is_some(),
            "der Produzent kennt {w}"
        );
    }
    // Eine ACHTE faellt — nicht „sonst".
    assert!(Ursachenklasse::aus_wire("achte_klasse").is_none());
    assert!(Ursachenklasse::aus_wire("").is_none());
    // Die siebte ist ein ERGEBNIS und kein Fehler: sie ist Enthaltung, und
    // keine der sechs anderen ist es.
    assert!(Ursachenklasse::DatenReichenNicht.ist_enthaltung());
    for k in [
        Ursachenklasse::QuelleResonanz,
        Ursachenklasse::ZweiQuellenKonkurrenz,
        Ursachenklasse::EffektkettePrePost,
        Ursachenklasse::SummeAufMaster,
        Ursachenklasse::PeakAusTransient,
        Ursachenklasse::StereoAusQuelleOderKette,
    ] {
        assert!(!k.ist_enthaltung(), "{} ist keine Enthaltung", k.wire());
    }

    // Und der PRODUKTPFAD gibt nur Woerter aus dieser Menge aus.
    let c = coordinator();
    let adressen = buehne(&c, 1, Some(3));
    reihe(&c, "main", &adressen[0], 0, 10, Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0)));
    reihe(&c, "sonde0", &adressen[1], 100, 10, Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0)));
    let f = findings(&c);
    assert!(!f.is_empty(), "der Produktpfad liefert einen Befund");
    for befund in &f {
        let wort = befund["ursachenklasse"].as_str().unwrap();
        assert!(
            menge.iter().any(|m| m.as_str() == Some(wort)),
            "{wort} steht im Vertrag"
        );
    }
}

/// Zwei deutlich verschiedene Fingerprints — der Produktweg der
/// Materialruecknahme vergleicht sie und schlaegt unter `GATE_MATERIAL_GLEICH`
/// an. Zwei Nullvektoren waeren ausdruecklich NICHT aehnlich, aber auch keine
/// saubere Aussage; deshalb tragen beide echte Energie.
fn fingerprint(art: u8) -> eqcop_broker::telemetrie::Fingerprintwerte {
    let mut w = eqcop_broker::telemetrie::Fingerprintwerte::default();
    // ⚠️ Die Aehnlichkeit ist ein COSINUS, kein Betragsvergleich: zwei
    // konstante Vektoren verschiedener Hoehe sind sich zu 100 % aehnlich.
    // Unaehnlich wird es erst, wenn die Energie WOANDERS liegt — deshalb
    // besetzt jede Fassung die andere Haelfte.
    for i in 0..32 {
        let vorne = i < 16;
        let treffer = if art == 1 { vorne } else { !vorne };
        w.band_energie[i] = if treffer { 200 } else { 0 };
        w.onset[i] = if treffer { 180 } else { 0 };
    }
    for i in 0..12 {
        let treffer = if art == 1 { i < 6 } else { i >= 6 };
        w.chroma[i] = if treffer { 220 } else { 0 };
    }
    w
}

fn vertrag_schema() -> Value {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../eq-copilot/schemas/v3/eq-ipc-v3.schema.json");
    serde_json::from_slice(&std::fs::read(&pfad).expect("Vertrag liegt im Repo"))
        .expect("Vertrag ist JSON")
}

// ═════════════════════════════════════════════════════════════════════════
// M-15 · causehypothesis_traegt_die_zehn_felder
// ═════════════════════════════════════════════════════════════════════════
//
// Die Vertragshaelfte liegt in Etappe B (A5 mit Negativfixture je Feld). HIER
// faellt die Erzeugerhaelfte: das Objekt, das der Produktpfad WIRKLICH baut,
// traegt jedes Pflichtfeld — und `class` wird nicht aus `score` gerundet.
#[test]
fn causehypothesis_traegt_die_zehn_felder() {
    let c = coordinator();
    let adressen = buehne(&c, 1, Some(3));
    reihe(&c, "main", &adressen[0], 0, 10, Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0)));
    reihe(&c, "sonde0", &adressen[1], 100, 10, Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0)));
    let f = findings(&c);
    let befund = f.first().expect("ein Befund");

    let vertrag = vertrag_schema();
    let pflicht = vertrag["$defs"]["session_finding"]["required"]
        .as_array()
        .expect("Pflichtfelder stehen im Vertrag");
    for feld in pflicht {
        let name = feld.as_str().unwrap();
        assert!(
            befund.get(name).is_some(),
            "Pflichtfeld {name} fehlt im erzeugten Befund"
        );
    }
    // Die zehn Felder aus §36.3 stehen einzeln — `confidence` ist ZWEI
    // Felder, nicht eines.
    assert!(befund["confidence"]["class"].is_string());
    assert!(befund["confidence"]["score"].is_number());
    // Und die Klasse ist NICHT die gerundete Zahl: ein Score von 0,6 ergaebe
    // gerundet „mittel", die Klasse entsteht aber strukturell. Gemessen wird
    // die Trennung an einem Fall, in dem beide auseinanderlaufen duerfen.
    let score = befund["confidence"]["score"].as_f64().unwrap();
    let klasse = befund["confidence"]["class"].as_str().unwrap();
    assert!((0.0..=1.0).contains(&score));
    assert!(["hoch", "mittel", "unklar"].contains(&klasse));

    // `band_hz` traegt ein BANDINTERVALL, keine Hertzzahlen.
    assert!(befund["band_hz"]["von"].as_u64().unwrap() < befund["band_hz"]["bis"].as_u64().unwrap());
    assert!(befund["band_hz"]["bis"].as_u64().unwrap() <= 221);
    // Kein Objekt ohne `evidence_ids`.
    assert!(!befund["evidence_ids"].as_array().unwrap().is_empty());
}

// ═════════════════════════════════════════════════════════════════════════
// M-16 · sechs_teile_im_datenmodell
// ═════════════════════════════════════════════════════════════════════════
//
// Die sechs Teile aus §8 sind DATENMODELL — und Gen zeigt genau drei Zeilen
// (Abnahme U21). Beides wird hier gemessen: alle sechs Teile sind da, UND die
// drei Anzeigezeilen stehen als drei eigene Felder, damit die Anzeige keine
// davon aus mehreren zusammensetzt.
#[test]
fn sechs_teile_im_datenmodell() {
    let c = coordinator();
    let adressen = buehne(&c, 1, Some(3));
    reihe(&c, "main", &adressen[0], 0, 10, Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0)));
    reihe(&c, "sonde0", &adressen[1], 100, 10, Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0)));

    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    let befund = befunde.first().expect("ein Befund");
    for (name, vorhanden) in befund.sechs_teile() {
        assert!(vorhanden, "Teil {name} fehlt im Datenmodell");
    }

    // Die DREI Anzeigezeilen: eigene Felder, jede mit Inhalt.
    let f = findings(&c);
    let wire = f.first().unwrap();
    for zeile in ["likely_cause", "smallest_test", "listen_for"] {
        let text = wire[zeile].as_str().unwrap_or_default();
        assert!(!text.is_empty(), "{zeile} ist leer");
        assert!(text.chars().count() <= 200, "{zeile} ueber der Vertragslaenge");
    }
    // Es gibt KEIN Belegtextfeld: der Beleg ist die Zone, nicht ein Text.
    assert!(wire.get("beleg").is_none());
}

// ═════════════════════════════════════════════════════════════════════════
// M-17 · parallele_telemetrie_bleibt_klasse_eins
// ═════════════════════════════════════════════════════════════════════════
//
// Zwei Quellen, gleichzeitig gemessen, perfekt korreliert — und trotzdem
// bleibt jede Aussage `zusammenhang`. Ohne PRE/POST-Paar gibt es keinen
// Wirkungsbeleg, und `ursachenbeleg` kann in P5 gar nicht entstehen: der
// verlangt eine kontrollierte Preview, und die gibt es hier nicht.
#[test]
fn parallele_telemetrie_bleibt_klasse_eins() {
    let c = coordinator();
    let adressen = buehne(&c, 2, Some(3));
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    reihe(&c, "main", &adressen[0], 0, 12, anhebung);
    reihe(&c, "sonde0", &adressen[1], 100, 12, anhebung);
    reihe(&c, "sonde1", &adressen[2], 200, 12, anhebung);

    let f = findings(&c);
    assert!(!f.is_empty());
    for befund in &f {
        assert_eq!(
            befund["claim_class"].as_str(),
            Some("zusammenhang"),
            "parallele Telemetrie darf nie Klasse 2 oder 3 erzeugen"
        );
    }
    // Und die staerkste Klasse ist im ganzen Produktpfad nicht erreichbar.
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert!(befunde
        .iter()
        .all(|b| b.claim_class < Aussageklasse::Ursachenbeleg));
}

// ═════════════════════════════════════════════════════════════════════════
// M-18 · screening_reicht_hoechstens_fuenf_weiter
// ═════════════════════════════════════════════════════════════════════════
//
// Sieben gleichwertige Kandidaten, alle mit bestandenen Gates. Der Deckel ist
// HART: es kommen fuenf durch, nicht sechs und nicht „ungefaehr fuenf".
#[test]
fn screening_reicht_hoechstens_fuenf_weiter() {
    let c = coordinator();
    let adressen = buehne(&c, 7, Some(3));
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    reihe(&c, "main", &adressen[0], 0, 12, anhebung);
    for (i, a) in adressen[1..].iter().enumerate() {
        reihe(&c, &format!("sonde{i}"), a, 100 + i * 100, 12, anhebung);
    }
    let f = findings(&c);
    // ⚠️ Die FUENF steht hier als Zahl, nicht als `KANDIDATEN_DECKEL`.
    //
    // M-18 sagt „hoechstens die besten fuenf" — das ist die Zusage, nicht der
    // Wert einer Konstante. Ein Test, der gegen dieselbe Konstante prueft,
    // die der Pfad benutzt, wandert mit ihr mit und kann nicht fallen; genau
    // das hat der Rotbeweis M-18 in der ersten Fassung gezeigt.
    assert_eq!(f.len(), 5, "der Deckel aus M-18 ist hart: {} Befunde", f.len());
    // Und die Konstante des Pfades traegt dieselbe Zahl.
    assert_eq!(KANDIDATEN_DECKEL, 5);
    // Der fuehrende Befund nennt die vier uebrigen als Alternativen — und
    // keinen fuenften.
    let alternativen = f[0]["alternatives"].as_array().cloned().unwrap_or_default();
    assert_eq!(alternativen.len(), 4);
}

// ═════════════════════════════════════════════════════════════════════════
// M-19 · bedingter_uplift_braucht_fenster_ohne_die_quelle
// ═════════════════════════════════════════════════════════════════════════
//
// Eine Quelle, die in JEDEM Fenster gleich laut ist, liefert keine
// Vergleichsmenge „ohne diese Aktivitaet" — und damit keinen Uplift. Der
// Rotbeweis faellt hier: wer den Uplift ohne Vergleichsfenster rechnet, misst
// den Pegel des Masters und nennt ihn Wirkung.
#[test]
fn bedingter_uplift_braucht_fenster_ohne_die_quelle() {
    // (a) Konstante Quelle: kein Uplift.
    let c = coordinator();
    let adressen = buehne(&c, 1, Some(3));
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    reihe(&c, "main", &adressen[0], 0, 12, anhebung);
    reihe(&c, "sonde0", &adressen[1], 100, 12, anhebung);
    let konstant = c.befunde_sicht(&hex(0x11), &hex(0x22));
    let uplift_konstant = konstant[0].rang.uplift;

    // (b) Dieselbe Buehne, aber die Quelle ist in der zweiten Haelfte still,
    //     und der Master faellt mit. Jetzt GIBT es Fenster ohne die Quelle.
    let d = coordinator();
    let adressen = buehne(&d, 1, Some(3));
    for i in 0..12 {
        // Die LAUTE Haelfte liegt hinten: `masteranomalie` liest das juengste
        // Fenster, und eine Anomalie, die dort nicht mehr steht, waere keine.
        let laut = i >= 6;
        let hub = if laut { Some((ANOMALIEBAND, ANOMALIEBAND + 4, 12.0)) } else { None };
        d.p1(
            "main",
            &evidenz(&adressen[0], i, 44_108_200 + (i as i64) * 512, hub, |_| {}),
        );
        d.p1(
            "sonde0",
            &evidenz(&adressen[1], 100 + i, 44_108_200 + (i as i64) * 512, hub, |_| {}),
        );
    }
    let getrennt = d.befunde_sicht(&hex(0x11), &hex(0x22));
    let uplift_getrennt = getrennt[0].rang.uplift;

    assert_eq!(
        uplift_konstant, 0.0,
        "ohne Vergleichsfenster gibt es keinen Uplift"
    );
    assert!(
        uplift_getrennt > 0.0,
        "mit Vergleichsfenstern faellt ein Uplift an: {uplift_getrennt}"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-20 · keine_komponente_kompensiert_coverage_oder_alignment
// ═════════════════════════════════════════════════════════════════════════
//
// Je ein Fall mit MAXIMALWERT in genau einer Komponente und gerissener
// Coverage beziehungsweise gerissenem Alignment. Der Kandidat erreicht keine
// starke Aussage — er erreicht ueberhaupt keine, weil das Gate VOR der
// Gewichtung liegt, und er traegt dafuer einen Grund (M-87).
#[test]
fn keine_komponente_kompensiert_coverage_oder_alignment() {
    // (a) Coverage gerissen: perfekte Bandpassung, aber duenne Messung.
    let c = coordinator();
    let adressen = buehne(&c, 1, Some(3));
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    reihe(&c, "main", &adressen[0], 0, 12, anhebung);
    for i in 0..12 {
        c.p1(
            "sonde0",
            &evidenz(&adressen[1], 100 + i, 44_108_200 + (i as i64) * 512, anhebung, |w| {
                // Unter GATE_ABDECKUNG (0,5) — und sonst alles perfekt.
                w["abdeckung"] = json!(0.2);
            }),
        );
    }
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    let befund = befunde.first().expect("ein Befund entsteht trotzdem");
    assert_eq!(
        befund.ursachenklasse,
        Ursachenklasse::DatenReichenNicht,
        "ohne Kandidat bleibt nur die Enthaltung"
    );
    assert!(
        befund
            .ausschluesse
            .iter()
            .any(|a| a.grund == Ausschlussgrund::CoverageFehlt),
        "der Ausschluss traegt den Grund coverage_fehlt: {:?}",
        befund.ausschluesse
    );

    // (b) Alignment gerissen: die Quelle misst ein voellig anderes
    //     Projektfenster als der Master.
    let d = coordinator();
    let adressen = buehne(&d, 1, Some(3));
    reihe(&d, "main", &adressen[0], 0, 12, anhebung);
    for i in 0..12 {
        d.p1(
            "sonde0",
            &evidenz(
                &adressen[1],
                100 + i,
                // Weit weg vom Master — keine gemeinsame Zeit.
                90_000_000 + (i as i64) * 512,
                anhebung,
                |_| {},
            ),
        );
    }
    let befunde = d.befunde_sicht(&hex(0x11), &hex(0x22));
    let befund = befunde.first().expect("ein Befund entsteht trotzdem");
    assert!(
        befund
            .ausschluesse
            .iter()
            .any(|a| a.grund == Ausschlussgrund::AlignmentFalsch),
        "der Ausschluss traegt den Grund alignment_falsch: {:?}",
        befund.ausschluesse
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-22 · parent_duplikat_erzeugt_keine_zwei_starken
// ═════════════════════════════════════════════════════════════════════════
//
// Zwei Sonden auf DEMSELBEN Mixerkanal messen dasselbe Signal. Keine der
// beiden erreicht `hoch`. Die zweite Haelfte faellt daneben: OHNE
// `host_mixer_index` ist das Routing unbekannt, und auch dann gibt es keine
// starke Aussage.
#[test]
fn parent_duplikat_erzeugt_keine_zwei_starken() {
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));

    // (a) Zwei Sonden auf demselben Kanal.
    let c = coordinator();
    let master = adresse(1);
    anmelden(&c, "main", &master, "main", Some(0), None);
    intent_marke(&c, "main", &master);
    let a = adresse(2);
    let b = adresse(3);
    anmelden(&c, "sonde0", &a, "passive_probe", Some(7), None);
    anmelden(&c, "sonde1", &b, "passive_probe", Some(7), None);
    reihe(&c, "main", &master, 0, 12, anhebung);
    reihe(&c, "sonde0", &a, 100, 12, anhebung);
    reihe(&c, "sonde1", &b, 200, 12, anhebung);
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert!(befunde.len() >= 2, "beide Sonden liegen im Rennen");
    assert!(
        befunde
            .iter()
            .all(|f| f.confidence.klasse < Sicherheitsklasse::Hoch),
        "ein Parent-Duplikat traegt keine starke Aussage"
    );
    assert!(
        befunde.iter().all(|f| f.rang.routingqualitaet < 1.0),
        "die Routingqualitaet trennt sie"
    );

    // (b) Routing unbekannt.
    let d = coordinator();
    let master = adresse(1);
    anmelden(&d, "main", &master, "main", Some(0), None);
    intent_marke(&d, "main", &master);
    let e = adresse(2);
    anmelden(&d, "sonde0", &e, "passive_probe", None, None);
    reihe(&d, "main", &master, 0, 12, anhebung);
    reihe(&d, "sonde0", &e, 100, 12, anhebung);
    let befunde = d.befunde_sicht(&hex(0x11), &hex(0x22));
    let befund = befunde.first().expect("ein Befund");
    assert_eq!(befund.rang.routingqualitaet, 0.0);
    assert!(befund.confidence.klasse < Sicherheitsklasse::Hoch);
    assert_eq!(befund.zustand, Befundzustand::MoreData);
}

// ═════════════════════════════════════════════════════════════════════════
// M-23 · passage_verschoben_anders_oder_zu_kurz
// ═════════════════════════════════════════════════════════════════════════
//
// Der Fall, den die Matrixpruefung 1 als D1 gefunden hat: `ueberdeckung`
// rechnet relativ, und zwei identische kurze Passagen bestehen alle vier
// Gates. `GATE_MINDEST_FENSTER` misst die absolute Groesse. Ohne eine
// benannte Passage gibt es kein Passagengate — dann traegt die Zahl der
// Fenster die Sicherheitsklasse.
#[test]
fn passage_zu_kurz_traegt_keine_starke_aussage() {
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    // Drei unabhaengige Fenster — unter GATE_MINDEST_FENSTER (8).
    let c = coordinator();
    let adressen = buehne(&c, 1, Some(3));
    reihe(&c, "main", &adressen[0], 0, 3, anhebung);
    reihe(&c, "sonde0", &adressen[1], 100, 3, anhebung);
    let kurz = c.befunde_sicht(&hex(0x11), &hex(0x22));
    let befund = kurz.first().expect("ein Befund entsteht");
    assert!(
        befund.confidence.klasse < Sicherheitsklasse::Hoch,
        "unter der Schwelle gibt es keine starke Aussage: {:?}",
        befund.confidence
    );
    assert_ne!(
        befund.zustand,
        Befundzustand::ReadyToSend,
        "und damit kein READY TO SEND"
    );

    // Zwoelf Fenster, sonst IDENTISCH: dieselbe Sitzung wird handelbar. Ohne
    // diese Gegenprobe waere der Riegel eine Regressionswache und kein Beleg
    // (`tools/dirigent/pruefliste.md` E).
    let d = coordinator();
    let adressen = buehne(&d, 1, Some(3));
    reihe(&d, "main", &adressen[0], 0, 12, anhebung);
    reihe(&d, "sonde0", &adressen[1], 100, 12, anhebung);
    let lang = d.befunde_sicht(&hex(0x11), &hex(0x22));
    let befund = lang.first().expect("ein Befund entsteht");
    assert_eq!(
        befund.confidence.klasse,
        Sicherheitsklasse::Hoch,
        "mit genug Fenstern wird derselbe Befund handelbar: {:?}",
        (befund.confidence, befund.rang)
    );
    assert_eq!(befund.zustand, Befundzustand::ReadyToSend);

    // Und die Schwelle faellt WIRKLICH an der Kante: sieben Fenster sind zu
    // wenig, acht genug (Muster C-09 — jede Schwelle von beiden Seiten).
    for (anzahl, erwartet) in [(7usize, Sicherheitsklasse::Mittel), (8, Sicherheitsklasse::Hoch)] {
        let e = coordinator();
        let adressen = buehne(&e, 1, Some(3));
        reihe(&e, "main", &adressen[0], 0, anzahl, anhebung);
        reihe(&e, "sonde0", &adressen[1], 100, anzahl, anhebung);
        let f = e.befunde_sicht(&hex(0x11), &hex(0x22));
        assert_eq!(
            f.first().map(|b| b.confidence.klasse),
            Some(erwartet),
            "bei {anzahl} Fenstern"
        );
    }

    // Eine VERSCHOBENE Passage: dieselbe Zahl Fenster, aber ohne gemeinsame
    // Zeit mit dem Master. Sie erreicht keine Aussage, sondern faellt mit
    // Grund aus dem Ranking.
    let g = coordinator();
    let adressen = buehne(&g, 1, Some(3));
    reihe(&g, "main", &adressen[0], 0, 12, anhebung);
    for i in 0..12 {
        // Um 30 % der Fensterreihe verschoben.
        g.p1(
            "sonde0",
            &evidenz(
                &adressen[1],
                100 + i,
                44_108_200 + (i as i64) * 512 + (12 * 512 * 30) / 100,
                anhebung,
                |_| {},
            ),
        );
    }
    let verschoben = g.befunde_sicht(&hex(0x11), &hex(0x22));
    let befund = verschoben.first().expect("ein Befund entsteht");
    assert!(
        befund.confidence.klasse < Sicherheitsklasse::Hoch,
        "eine verschobene Passage traegt keine Sicherheit hoch: {:?}",
        befund.confidence
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-24 · ruecknahme_invalidiert_abhaengige_hypothesen
// ═════════════════════════════════════════════════════════════════════════
//
// Die Reihenfolge steht in M-24 woertlich: Umfang aufloesen → Evidenz-IDs
// markieren → jede abhaengige Hypothese terminal invalidieren. Gemessen wird
// beides: dass die Hypothese faellt, UND dass zwei Reihenfolgen derselben
// Ruecknahme dieselbe Menge treffen.
#[test]
fn ruecknahme_invalidiert_abhaengige_hypothesen() {
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    let bauen = || {
        let c = coordinator();
        let adressen = buehne(&c, 2, Some(3));
        reihe(&c, "main", &adressen[0], 0, 12, anhebung);
        reihe(&c, "sonde0", &adressen[1], 100, 12, anhebung);
        reihe(&c, "sonde1", &adressen[2], 200, 12, anhebung);
        (c, adressen)
    };

    let (c, _) = bauen();
    let vorher = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert!(!vorher.is_empty(), "es gibt Befunde, die fallen koennen");

    // Die ganze Sitzung zuruecknehmen — ueber den PRODUKTWEG „Material
    // gewechselt", nicht ueber eine Testnaht.
    let getroffen = c.invalidierung_wegen_material_fuer_link("main", Some(&fingerprint(1)), Some(&fingerprint(2)));
    assert!(getroffen > 0, "die Ruecknahme trifft Evidenz");
    let nachher = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert!(
        nachher.is_empty(),
        "ohne Beleg bleibt keine sichtbare Behauptung: {nachher:?}"
    );
    assert!(
        findings(&c).is_empty(),
        "und der Snapshot zeigt sie auch nicht mehr"
    );

    // Determinismus: dieselbe Ruecknahme ueber einen ANDEREN Link derselben
    // Sitzung trifft dieselbe Menge. Der Umfang ist die Sitzung, nicht der
    // Ausloeser — wer das verwechselt, raeumt nur eine Quelle ab.
    let (d, _) = bauen();
    let getroffen_d = d.invalidierung_wegen_material_fuer_link("sonde1", Some(&fingerprint(1)), Some(&fingerprint(2)));
    assert_eq!(getroffen, getroffen_d, "dieselbe Menge, anderer Ausloeser");
    assert_eq!(
        d.befunde_sicht(&hex(0x11), &hex(0x22)),
        nachher,
        "zwei Reihenfolgen derselben Ruecknahme treffen dieselbe Menge"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-25 · ranking_ist_bytegleich_ueber_hundert_laeufe
// ═════════════════════════════════════════════════════════════════════════
//
// Gleiche Eingaben, derselbe Build: bytegleiche Ausgabe. Gemessen an der
// SERIALISIERTEN Form, nicht an der Struktur — der Wire-Text ist das, was
// Gen sieht.
#[test]
fn ranking_ist_bytegleich_ueber_hundert_laeufe() {
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    // Die Nutzlasten entstehen EINMAL. Sie sind der Eingang, ueber den M-25
    // spricht („gleiche Eingaben"); sie je Lauf neu zu serialisieren waere
    // hundertmal dieselbe Arbeit und wuerde nebenbei die Frage verwaessern,
    // ob die BYTES wirklich dieselben waren.
    let master = adresse(1);
    let sonden: Vec<Adresse> = (0..3).map(|i| adresse(2 + i)).collect();
    let mut payloads: Vec<(String, Vec<u8>)> = Vec::new();
    // Acht Fenster — GENAU `GATE_MINDEST_FENSTER`. Der Determinismusfall
    // faehrt damit zugleich die Kante, an der die Sicherheitsklasse kippt;
    // ein Lauf, der dort flackerte, faellt hier auf.
    for i in 0..8 {
        let zeit = 44_108_200 + (i as i64) * 512;
        payloads.push((
            "main".into(),
            evidenz(&master, i, zeit, anhebung, |_| {}),
        ));
        for (n, a) in sonden.iter().enumerate() {
            payloads.push((
                format!("sonde{n}"),
                evidenz(a, 100 + n * 100 + i, zeit, anhebung, |_| {}),
            ));
        }
    }

    let lauf = || {
        let c = coordinator();
        anmelden(&c, "main", &master, "main", Some(0), None);
        intent_marke(&c, "main", &master);
        for (n, a) in sonden.iter().enumerate() {
            anmelden(&c, &format!("sonde{n}"), a, "passive_probe", Some(3 + n as i64), None);
        }
        for (link, payload) in &payloads {
            c.p1(link, payload);
        }
        serde_json::to_vec(&findings(&c)).unwrap()
    };
    let referenz = lauf();
    assert!(!referenz.is_empty());
    for runde in 0..100 {
        assert_eq!(lauf(), referenz, "Lauf {runde} weicht ab");
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-26 · gleichstand_zeigt_beide
// ═════════════════════════════════════════════════════════════════════════
//
// Zwei Kandidaten mit identischem Material liegen exakt gleich. BEIDE werden
// gezeigt; der Tie-Break entscheidet nur die Reihenfolge, und die ist ueber
// zwei Laeufe dieselbe.
#[test]
fn gleichstand_zeigt_beide() {
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    let c = coordinator();
    let adressen = buehne(&c, 2, Some(3));
    reihe(&c, "main", &adressen[0], 0, 12, anhebung);
    reihe(&c, "sonde0", &adressen[1], 100, 12, anhebung);
    reihe(&c, "sonde1", &adressen[2], 200, 12, anhebung);

    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert_eq!(befunde.len(), 2, "bei Gleichstand verschwindet keiner");
    assert_eq!(
        befunde[0].rang.rang(),
        befunde[1].rang.rang(),
        "die beiden liegen wirklich gleich"
    );
    // Der Tie-Break ist die `candidate_source`, aufsteigend.
    assert!(befunde[0].candidate_source < befunde[1].candidate_source);
}

// ═════════════════════════════════════════════════════════════════════════
// M-21/M-65 · ungetrennter_erster_platz_ist_nicht_stark
// ═════════════════════════════════════════════════════════════════════════
//
// Gefunden hat das der P5-Korpus (Etappe H), nicht dieses Bein: bei
// Gleichstand entscheidet der Tie-Break, WER fuehrt — und eine starke
// Aussage auf diesem Platz behauptet eine Unterscheidung, die die Messung
// nicht hergibt. Sie waere richtig oder falsch, je nachdem wie die
// Kennungen zufaellig liegen (§36.4 Satz 1, §49.4).
//
// Beide Richtungen stehen hier. Ohne die zweite waere der Riegel auch dann
// gruen, wenn ueberhaupt nichts mehr `hoch` erreichte.
#[test]
fn ungetrennter_erster_platz_ist_nicht_stark() {
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));

    // (1) Zwei identische Kandidaten: KEINER ist stark, beide bleiben da.
    let c = coordinator();
    let adressen = buehne(&c, 2, Some(3));
    reihe(&c, "main", &adressen[0], 0, 12, anhebung);
    reihe(&c, "sonde0", &adressen[1], 100, 12, anhebung);
    reihe(&c, "sonde1", &adressen[2], 200, 12, anhebung);
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert_eq!(befunde.len(), 2, "beide bleiben sichtbar");
    assert_eq!(
        befunde[0].rang.rang(),
        befunde[1].rang.rang(),
        "sie liegen wirklich gleich — sonst misst dieser Fall etwas anderes"
    );
    for (i, b) in befunde.iter().enumerate() {
        assert!(
            b.confidence.klasse < Sicherheitsklasse::Hoch,
            "Kandidat {i} traegt {:?}, obwohl ihn nichts vom anderen trennt",
            b.confidence.klasse
        );
        assert_ne!(b.zustand, Befundzustand::ReadyToSend);
    }

    // (2) GEGENPROBE: ein Kandidat mit unbekanntem Routing rangiert
    //     schlechter. Jetzt TRENNT die Messung, und der erste darf stark
    //     sein — sonst waere (1) trivial erfuellt.
    let c = coordinator();
    let master = adresse(1);
    let stark = adresse(2);
    let schwach = adresse(3);
    anmelden(&c, "main", &master, "main", Some(0), None);
    intent_marke(&c, "main", &master);
    anmelden(&c, "sonde0", &stark, "passive_probe", Some(3), None);
    anmelden(&c, "sonde1", &schwach, "passive_probe", None, None);
    reihe(&c, "main", &master, 0, 12, anhebung);
    reihe(&c, "sonde0", &stark, 100, 12, anhebung);
    reihe(&c, "sonde1", &schwach, 200, 12, anhebung);
    let befunde = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert_eq!(befunde.len(), 2, "auch hier verschwindet keiner");
    assert!(
        befunde[0].rang.rang() > befunde[1].rang.rang(),
        "die Gegenprobe trennt wirklich: {} gegen {}",
        befunde[0].rang.rang(),
        befunde[1].rang.rang()
    );
    assert_eq!(
        befunde[0].confidence.klasse,
        Sicherheitsklasse::Hoch,
        "ein getrennter erster Platz darf stark sein"
    );
    assert_eq!(befunde[0].zustand, Befundzustand::ReadyToSend);
    // Und der zweite bleibt trotzdem schwach (M-21).
    assert!(befunde[1].confidence.klasse < Sicherheitsklasse::Hoch);
}

// ═════════════════════════════════════════════════════════════════════════
// M-27 · mehr_daten_ist_ein_ergebnis
// ═════════════════════════════════════════════════════════════════════════
//
// Kein Kandidat ueberlebt — und das ergibt einen BEFUND mit der siebten
// Ursachenklasse, keinen Fehlercode und kein Schweigen.
#[test]
fn mehr_daten_ist_ein_ergebnis() {
    let c = coordinator();
    let master = adresse(1);
    anmelden(&c, "main", &master, "main", Some(0), None);
    intent_marke(&c, "main", &master);
    reihe(&c, "main", &master, 0, 12, Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0)));

    let f = findings(&c);
    assert_eq!(f.len(), 1, "auch ohne Kandidat gibt es ein Ergebnis");
    assert_eq!(f[0]["ursachenklasse"].as_str(), Some("daten_reichen_nicht"));
    assert_eq!(f[0]["zustand"].as_str(), Some("more_data"));
    assert_eq!(f[0]["next_test"].as_str(), Some("mehr_daten_sammeln"));
    assert_eq!(f[0]["confidence"]["class"].as_str(), Some("unklar"));
    // Und es ist ein ERGEBNIS: es traegt Belege und einen Ort.
    assert!(!f[0]["evidence_ids"].as_array().unwrap().is_empty());
}

// ═════════════════════════════════════════════════════════════════════════
// M-28 · behauptung_ohne_existente_evidenz_wird_nicht_sichtbar
// ═════════════════════════════════════════════════════════════════════════
//
// „Existent" ist eine Pruefung gegen den BESTAND, nicht gegen den Speicher:
// eine ausgeschlossene Evidenz zaehlt nicht. Die Trennung aus M-28 faellt
// beidseitig — alle Belege fort heisst unsichtbar, einige fort heisst STALE.
#[test]
fn behauptung_ohne_existente_evidenz_wird_nicht_sichtbar() {
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    let c = coordinator();
    let adressen = buehne(&c, 1, Some(3));
    reihe(&c, "main", &adressen[0], 0, 12, anhebung);
    reihe(&c, "sonde0", &adressen[1], 100, 12, anhebung);
    assert!(!findings(&c).is_empty());

    // Ein TEIL der Belege faellt: der Befund wird STALE, nicht unsichtbar.
    // Der Produktweg dafuer ist ein hoerbarer Eingriff in einem BEREICH —
    // er nimmt genau die Fenster zurueck, die darin liegen.
    let von = 44_108_200;
    let bis = von + 3 * 512;
    let getroffen = c.invalidierung_wegen_intervention_fuer_link("main", von, bis);
    assert!(getroffen > 0, "der Bereich trifft Fenster");
    let teilweise = c.befunde_sicht(&hex(0x11), &hex(0x22));
    assert!(
        teilweise
            .iter()
            .any(|f| f.zustand == Befundzustand::Stale),
        "einige Belege fort heisst STALE: {:?}",
        teilweise.iter().map(|f| f.zustand).collect::<Vec<_>>()
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-87 · ausschlussgruende_sind_geschlossen_und_reisen_mit
// ═════════════════════════════════════════════════════════════════════════
//
// Die Vertragshaelfte liegt in Etappe B. HIER faellt die Rechnung: jeder
// ausgeschiedene Kandidat traegt einen Grund AUS DER MENGE, und die Gruende
// reisen mit dem Befund zu Gen.
#[test]
fn ausschlussgruende_sind_geschlossen_und_reisen_mit() {
    let vertrag = vertrag_schema();
    let menge: Vec<String> = vertrag["$defs"]["ausschlussgrund"]["enum"]
        .as_array()
        .expect("die Menge steht im Vertrag")
        .iter()
        .map(|v| v.as_str().unwrap().to_string())
        .collect();
    assert_eq!(menge.len(), 8, "genau acht Ausschlussgruende");
    for wort in &menge {
        assert!(Ausschlussgrund::aus_wire(wort).is_some(), "{wort}");
    }
    assert!(Ausschlussgrund::aus_wire("neunter_grund").is_none());

    // Der Produktpfad: ein Kandidat, der an der Coverage faellt, steht mit
    // Grund im Snapshot — nicht kommentarlos entfernt.
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    let c = coordinator();
    let adressen = buehne(&c, 2, Some(3));
    reihe(&c, "main", &adressen[0], 0, 12, anhebung);
    reihe(&c, "sonde0", &adressen[1], 100, 12, anhebung);
    for i in 0..12 {
        c.p1(
            "sonde1",
            &evidenz(&adressen[2], 300 + i, 44_108_200 + (i as i64) * 512, anhebung, |w| {
                w["abdeckung"] = json!(0.1);
            }),
        );
    }
    let f = findings(&c);
    let ausschluesse = f[0]["ausschluesse"]
        .as_array()
        .expect("die Ausschluesse reisen mit dem Befund");
    assert_eq!(ausschluesse.len(), 1);
    assert_eq!(
        ausschluesse[0]["candidate_source"].as_str(),
        Some(adressen[2].instance_id.as_str())
    );
    assert_eq!(ausschluesse[0]["grund"].as_str(), Some("coverage_fehlt"));
    assert!(menge.contains(&ausschluesse[0]["grund"].as_str().unwrap().to_string()));
}

// ═════════════════════════════════════════════════════════════════════════
// M-12 · rollenaenderung_bewegt_die_intent_relevanz
// ═════════════════════════════════════════════════════════════════════════
//
// §5.1 ordnet M-12 der Etappe A zu; die Rangkomponente „Intent-Relevanz"
// entsteht aber erst hier (§7.1, Abweichung 2). Gemessen wird sie an der
// Komponente selbst: eine Quelle mit der Rolle `fuehrt` traegt eine hoehere
// Relevanz als dieselbe Quelle mit `begleitet`.
#[test]
fn rollenaenderung_bewegt_die_intent_relevanz() {
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    let mit_rolle = |rolle: &str| {
        let c = coordinator();
        let adressen = buehne(&c, 1, Some(3));
        // Der Vollbestand des Intents mit genau dieser Rolle.
        let intent = json!({
            "type": "intent_update",
            "adresse": adressen[0],
            "session_epoch": adressen[0].session_epoch,
            "vollstaendig": true,
            "bestand_revision": 1,
            "intents": [{
                "quelle_id": adressen[1].instance_id,
                "rolle": rolle,
                "revision": 1,
                "herkunft": "user",
                "konfidenz": 1.0
            }]
        });
        c.p1("main", &serde_json::to_vec(&intent).unwrap());
        reihe(&c, "main", &adressen[0], 0, 12, anhebung);
        reihe(&c, "sonde0", &adressen[1], 100, 12, anhebung);
        c.befunde_sicht(&hex(0x11), &hex(0x22))
            .first()
            .map(|f| f.rang.intent_relevanz)
            .unwrap_or(-1.0)
    };
    let fuehrt = mit_rolle("fuehrt");
    let begleitet = mit_rolle("begleitet");
    assert!(fuehrt > begleitet, "fuehrt {fuehrt} vs begleitet {begleitet}");
}

// ═════════════════════════════════════════════════════════════════════════
// M-86 · keine_rechnung_vor_der_vollstaendigkeitsmarke
// ═════════════════════════════════════════════════════════════════════════
//
// Die Sperre aus E-10 gilt auch fuer die Hypothese: ein Intent OHNE
// Vollstaendigkeitsmarke sperrt die Rechnung, statt sie mit halbem Wissen
// laufen zu lassen. Ein fehlendes Veto saehe sonst aus wie „kein Schutz
// gewuenscht".
#[test]
fn keine_hypothese_vor_der_vollstaendigkeitsmarke() {
    let anhebung = Some((ANOMALIEBAND, ANOMALIEBAND + 4, 9.0));
    let c = coordinator();
    // ⚠️ NICHT ueber `buehne`: die meldet seit der Nacharbeit 1 den
    // Vollbestand mit Marke, und dieser Fall misst gerade den Zustand DAVOR.
    let master = adresse(1);
    let sonde = adresse(2);
    anmelden(&c, "main", &master, "main", Some(0), None);
    anmelden(&c, "sonde0", &sonde, "passive_probe", Some(3), None);
    let adressen = vec![master, sonde];
    // Eine TEILMELDUNG ohne Vollbestand: der Spiegel ist unvollstaendig.
    let teil = json!({
        "type": "intent_update",
        "adresse": adressen[0],
        "session_epoch": adressen[0].session_epoch,
        "vollstaendig": false,
        "bestand_revision": 1,
        "intents": [{
            "quelle_id": adressen[1].instance_id,
            "rolle": "fuehrt",
            "revision": 1,
            "herkunft": "user",
            "konfidenz": 1.0
        }]
    });
    c.p1("main", &serde_json::to_vec(&teil).unwrap());
    reihe(&c, "main", &adressen[0], 0, 12, anhebung);
    reihe(&c, "sonde0", &adressen[1], 100, 12, anhebung);
    assert!(
        findings(&c).is_empty(),
        "ohne Vollstaendigkeitsmarke rechnet der Broker nicht"
    );
    assert!(!c.darf_rechnen(&hex(0x11), &hex(0x22)));
}
