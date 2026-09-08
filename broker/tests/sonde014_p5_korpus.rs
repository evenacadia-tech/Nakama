//! Der P5-Korpus als **Kette** (SONDE-014 Etappe H, R2, M-64 bis M-70).
//!
//! ── WARUM DIESES BEIN ZWISCHEN KORPUS UND PRUEFER STEHT ───────────────────
//!
//! Die Matrixprüfung 1 hat als Defekt **D2** gefunden: „der Korpus maß
//! Hygiene, nicht die Unterscheidung". Der P4-Korpus ist ein Register — er
//! hält fest, welches Bein einen Fall misst und welche Aussage Nakama darüber
//! trägt. Eine **falsche starke Produktbehauptung ändert diese Angaben
//! nicht**, und deshalb konnte sie der Korpus nicht finden.
//!
//! Regel **R2** zieht daraus den Schluss: der P5-Korpus ist eine Kette. Dieses
//! Bein ist ihr mittleres Glied. Es liest die Wahrheit aus dem committeten
//! Korpus, fährt jede Sitzung als Evidenzbestand durch `p1` — also durch
//! **denselben** Produktpfad wie im Betrieb —, liest die **tatsächlich
//! ausgegebene** Hypothese und schreibt sie als Ergebnis. Erst
//! `pruefe_p5_korpus.py` hält beides gegeneinander.
//!
//! ⚠️ Dieses Bein **wertet nicht**. Es rechnet keine Precision, keinen Brier
//! und keine Schwelle — täte es das, gäbe es die Kennzahlen zweimal, und die
//! zweite wäre die, an der niemand hinsieht.

use eqcop_broker::coordinator::{Coordinator, ManualClock};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
use eqcop_broker::transport::server_v3::Senke;
use serde_json::{json, Map, Value};
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

fn anmelden(c: &Coordinator, link: &str, a: &Adresse, art: &str, mixer: Option<i64>) {
    let h = hello(a.clone(), art);
    assert!(c.control_hello_registrieren(link, &h).angenommen);
    let _ = c.resync_bestaetigen(link, 0);
    let position = if art == "main" { "insert" } else { "post" };
    let mut d = json!({
        "adresse": a,
        "plugin_kind": art,
        "measurement_position": position,
        "aussageklasse": "beobachtend",
        "betrieb": "active",
        "label": "Korpusquelle",
        "capabilities": capabilities(),
        "frische": {"letzter_kontakt_ms": 10, "stale": false}
    });
    if let Some(index) = mixer {
        d["host_mixer_index"] = json!(index);
    }
    assert!(c.descriptor_setzen(link, d));
}

fn wurzel() -> std::path::PathBuf {
    std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .expect("das Repo liegt ueber broker/")
        .to_path_buf()
}

/// Eine v3-Fixture aus dem eingefrorenen Korpus.
fn fixture(name: &str) -> Value {
    let pfad = wurzel().join(format!("eq-copilot/fixtures/v3/gueltig/{name}.json"));
    serde_json::from_slice(&std::fs::read(&pfad).expect("Fixture liegt im Korpus"))
        .expect("Fixture ist JSON")
}

fn evidenz_grundform() -> Value {
    static ROH: std::sync::OnceLock<Value> = std::sync::OnceLock::new();
    ROH.get_or_init(|| {
        let pfad = wurzel()
            .join("eq-copilot/fixtures/v3/gueltig/evidence-snapshot-mit-ereignissen-und-stereo.json");
        serde_json::from_slice(&std::fs::read(&pfad).expect("Evidenzfixture liegt im Korpus"))
            .expect("Evidenzfixture ist JSON")
    })
    .clone()
}

/// Ein Beleg mit gesetzter Adresse, Projektzeit und Bandanhebung.
///
/// Die Bandgrenzen kommen aus dem **Manifest** des Korpus, nicht aus einer
/// Zahl hier — sonst gäbe es das Fenster zweimal.
fn evidenz(a: &Adresse, nr: usize, projekt_start: i64, anhebung_db: f64,
           band: (usize, usize), onset: Option<f64>) -> Vec<u8> {
    let mut wert = evidenz_grundform();
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
        }
    }
    // 🔑 NAK-212 E7: die Onsetstaerke je Fenster. `Evidenzstand::onset` ist
    // die SUMME der `staerke_mad` in `/ereignisse/liste` (`evidenz.rs`:305);
    // ein einziger Eintrag genuegt, damit die Reihe streut und die
    // Onset-Koinzidenz ueberhaupt messbar wird.
    if let Some(s) = onset {
        if let Some(Value::Array(liste)) = wert.pointer_mut("/ereignisse/liste") {
            liste.truncate(1);
            if let Some(e) = liste.first_mut() {
                e["staerke_mad"] = json!(s);
            }
        }
    }
    serde_json::to_vec(&wert).unwrap()
}

/// Der Testordner fuer Sitzungen mit Passage — `experiment_begin` ist ein
/// persistenzpflichtiger P0 und braucht einen echten Store.
struct TestOrdner(std::path::PathBuf);

impl TestOrdner {
    fn neu(name: &str) -> Self {
        let pfad = std::env::temp_dir().join(format!(
            "nakama-p5korpus-{name}-{}",
            std::process::id()
        ));
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

/// Die Reihe einer Quelle als Vektor: `reihe_db` steht ausgeschrieben in der
/// Fixture (NAK-212 E7), `anhebung_db` ist nur die Kurzform des Erzeugers.
fn reihe_db(wert: &Value, fenster: usize) -> Vec<f64> {
    match wert.get("reihe_db").and_then(Value::as_array) {
        Some(r) if r.len() == fenster => r.iter().map(|x| x.as_f64().unwrap_or(0.0)).collect(),
        _ => vec![wert["anhebung_db"].as_f64().unwrap_or(0.0); fenster],
    }
}

/// Die Onsetreihe, falls die Sitzung eine fuehrt.
fn onsets(wert: &Value, fenster: usize) -> Vec<Option<f64>> {
    match wert.get("onsets").and_then(Value::as_array) {
        Some(r) if r.len() == fenster => r.iter().map(Value::as_f64).collect(),
        _ => vec![None; fenster],
    }
}

/// Fährt EINE Sitzung durch den Produktpfad und gibt die ausgegebenen
/// Befunde zurück.
fn sitzung_fahren(s: &Value, band: (usize, usize)) -> Vec<Value> {
    // 🔑 **NAK-212 N-32/G-L1 (07.09.2026).** Sitzungen MIT Passage brauchen
    // einen echten Store: `experiment_begin` ist ein persistenzpflichtiger P0.
    // Ohne ihn legte das Bein nie eine Passage an, `aufnahmen_sammeln` las
    // `juengste_passage_im_projekt` als `None`, und `gate()` Schritt 4 lief im
    // GESAMTEN Korpuslauf nicht — auch nicht in `verschobene_passage`, die
    // nachweislich am Master-Alignment faellt. Sitzungen ohne Passage laufen
    // unveraendert ohne Store.
    let kennung = s["kennung"].as_str().unwrap_or("?");
    let ordner = s.get("passage").filter(|p| !p.is_null()).map(|_| TestOrdner::neu(kennung));
    let writer = ordner.as_ref().map(|o| {
        let mut k = eqcop_broker::store::StoreKonfiguration::fuer_pfad(o.db());
        k.remote_volume_override = Some(false);
        let w = eqcop_broker::store::StoreWriter::starten(k);
        assert!(!w.ist_degradiert(), "{kennung}: {:?}", w.handle().sicht());
        w
    });
    let c = match writer.as_ref() {
        Some(w) => Coordinator::mit_store(Arc::new(ManualClock::default()), hex(0xbeef), w),
        None => Coordinator::mit_uhr(Arc::new(ManualClock::default()), hex(0xbeef)),
    };
    let master = adresse(1);
    // 🔑 **NAK-213 E6/K-24:** Kanal **1** statt 0. Der Vertrag laesst
    // `host_mixer_index` erst ab 1 zu (`minimum: 1`); nur der permissive
    // Testsetter liess die 0 durch, und ein Produktzustand mit 0 entsteht auf
    // keinem Weg. Die Kanaele 1 und 2 sind im uebrigen Korpus frei, die
    // Erwartung jeder anderen Sitzung bleibt damit unberuehrt.
    anmelden(&c, "main", &master, "main", Some(1));
    // 🔑 M-86/NR-01 (Nacharbeit 1, 07.09.2026): der Vollbestand MIT Marke,
    // bevor gerechnet wird. Die Sperre ist seither fail-closed — `intent ==
    // None` sperrt genauso wie ein unvollstaendiger Bestand -, und ein
    // Korpuslauf ohne Marke maesse nur noch das Schweigen des Riegels.
    // Die Reihenfolge ist die des Mains: anmelden -> Marke -> Evidenz.
    {
        let marke = json!({
            "type": "intent_update",
            "adresse": master,
            "session_epoch": master.session_epoch,
            "vollstaendig": true,
            "bestand_revision": 0
        });
        c.p1("main", &serde_json::to_vec(&marke).unwrap());
    }

    let master_fenster = s["master"]["fenster"].as_u64().unwrap_or(0) as usize;
    let master_reihe = reihe_db(&s["master"], master_fenster);
    let master_onsets = onsets(&s["master"], master_fenster);

    let quellen = s["quellen"].as_array().cloned().unwrap_or_default();
    for (i, q) in quellen.iter().enumerate() {
        let instanz = q["instanz"].as_u64().unwrap() as usize;
        anmelden(
            &c,
            &format!("sonde{i}"),
            &adresse(instanz),
            "passive_probe",
            q["mixer"].as_i64(),
        );
    }

    // ⚠️ Die Belege gehen durch `p1` — durch dieselbe Senke, die der echte
    // Transport ruft. Ein Bein, das `hypothesen()` direkt riefe, koennte
    // gruen sein, waehrend die Verdrahtung fehlt (Risiko R1).
    let basis: i64 = 44_108_200;

    // Die Passage VOR der Evidenz: `aufnahmen_sammeln` liest sie zum
    // Rechenzeitpunkt, und die Rechnung laeuft mit jedem Beleg.
    if let Some(pass) = s.get("passage").filter(|p| !p.is_null()) {
        // WN-04: ohne `record_state` weist der Broker jeden
        // persistenzpflichtigen Befehl mit `record_state_unknown` ab — und
        // ohne `experiment_begin` gibt es keine benannte Passage.
        let bericht = json!({
            "type": "state_report",
            "adresse": master,
            "dsp_schema_version": 1,
            "state_revision": 0,
            "state_hash": "a".repeat(64),
            "record_state": {"valid": true, "recording": false}
        });
        assert!(c.state_report_json("main", &serde_json::to_vec(&bericht).unwrap()));

        let mut wert = fixture("experiment_begin");
        wert["kopf"]["ziel"] = serde_json::to_value(&master).unwrap();
        wert["kopf"]["command_id"] = json!(hex(0x930));
        wert["kopf"]["base_revision"] = json!(0);
        wert["experiment_id"] = json!(hex(0x940));
        wert["passage"]["passage_id"] = json!(hex(0x5032));
        wert["passage"]["projekt_von"] = json!(basis + pass["von_offset"].as_i64().unwrap_or(0));
        wert["passage"]["projekt_bis"] = json!(basis + pass["bis_offset"].as_i64().unwrap_or(0));
        wert["passage"]["transport_epoch"] =
            evidenz_grundform()["transport"]["transport_epoch"].clone();
        let mut ids: Vec<String> = vec![master.instance_id.clone()];
        for q in &quellen {
            ids.push(adresse(q["instanz"].as_u64().unwrap() as usize).instance_id);
        }
        wert["passage"]["aktive_quellen"] = json!(ids);
        let antwort = Senke::p0(&c, "main", &serde_json::to_vec(&wert).unwrap())
            .expect("experiment_begin wird beantwortet");
        let ack: Value = serde_json::from_slice(&antwort).unwrap();
        assert_eq!(
            ack["ergebnis"], "angewandt",
            "{kennung}: die Passage entsteht wirklich: {ack:?}"
        );
    }

    for i in 0..master_fenster {
        c.p1(
            "main",
            &evidenz(&master, i, basis + (i as i64) * 512, master_reihe[i], band,
                     master_onsets[i]),
        );
    }
    for (n, q) in quellen.iter().enumerate() {
        let instanz = q["instanz"].as_u64().unwrap() as usize;
        let a = adresse(instanz);
        let fenster = q["fenster"].as_u64().unwrap_or(0) as usize;
        let reihe = reihe_db(q, fenster);
        let onsetreihe = onsets(q, fenster);
        let versatz = q["versatz_fenster"].as_i64().unwrap_or(0);
        for i in 0..fenster {
            let zeit = basis + (i as i64 + versatz) * 512;
            c.p1(
                &format!("sonde{n}"),
                &evidenz(&a, 1000 + n * 100 + i, zeit, reihe[i], band, onsetreihe[i]),
            );
        }
    }

    let snapshot: Value =
        serde_json::from_slice(&c.session_snapshot_json(&hex(0x11), &hex(0x22))).unwrap();
    snapshot
        .get("findings")
        .and_then(Value::as_array)
        .cloned()
        .unwrap_or_default()
}

// ═════════════════════════════════════════════════════════════════════════
// Die Kette
// ═════════════════════════════════════════════════════════════════════════
//
// Der Name steht wörtlich im Manifest des Korpus (`kettenbein.bezeichner`);
// der Erzeuger prüft das. Ein umbenannter Fall fällt dort und nicht erst dem
// Leser auf.
#[test]
fn korpus_kette_laeuft_durch_den_produktpfad() {
    let korpus = wurzel().join("eq-copilot/fixtures/p5-korpus");
    let manifest: Value =
        serde_json::from_slice(&std::fs::read(korpus.join("MANIFEST.json")).expect("Manifest"))
            .expect("Manifest ist JSON");
    let sitzungen: Value =
        serde_json::from_slice(&std::fs::read(korpus.join("sitzungen.json")).expect("Sitzungen"))
            .expect("Sitzungen sind JSON");
    let band = (
        manifest["bandfenster"]["von"].as_u64().unwrap() as usize,
        manifest["bandfenster"]["bis"].as_u64().unwrap() as usize,
    );

    let liste = sitzungen["sitzungen"].as_array().expect("eine Liste");
    assert!(!liste.is_empty(), "der Korpus fuehrt Sitzungen");
    assert_eq!(
        liste.len() as u64,
        manifest["sitzungen_gesamt"].as_u64().unwrap(),
        "das Manifest zaehlt so viele Sitzungen, wie die Datei traegt"
    );

    let mut ergebnisse: Vec<Value> = Vec::new();
    for s in liste {
        let kennung = s["kennung"].as_str().expect("jede Sitzung hat eine Kennung");
        let befunde = sitzung_fahren(s, band);

        // 🔑 **NAK-212 Nacharbeit 1, NR-04.** Eine Sitzung, die „NUR X
        // trennt" zusagt, nennt ihre Vorbedingungen — und sie werden hier
        // gemessen, nicht angenommen. Das ist die DRITTE Zusage, die dieses
        // Bein selbst misst (die beiden anderen stehen unten): eine
        // Konstruktion, die an einer anderen Regel faellt als der zugesagten,
        // ist keine Gegenprobe. `g5_fenster_sieben` lief bis zur
        // Erstpruefung GEGEN den Master — sein Uplift war negativ, der Fall
        // blieb schon an R1/R2 `mittel`, und die Fensterzahl mass gar
        // nichts. Der Riegel faellt sofort, wenn eine Reihe wieder in die
        // falsche Phase geraet. Bei leerer Befundliste bleibt `beste` auf
        // `-inf` und der Riegel faellt ebenfalls — eine Sitzung ohne Befund
        // erfuellt keine Vorbedingung.
        for k in s["vorbedingung"].as_array().map(Vec::as_slice).unwrap_or(&[]) {
            let name = k.as_str().expect("Vorbedingung ist ein Name");
            let beste = befunde
                .iter()
                .filter_map(|b| b["rang"][name].as_f64())
                .fold(f64::NEG_INFINITY, f64::max);
            assert!(
                beste > 0.0,
                "{kennung}: Vorbedingung `{name}` ist nicht positiv belegt ({beste}) \
                 — der Fall faellt an einer anderen Regel und misst nicht, was er zusagt"
            );
        }

        // Die AUSGEGEBENE Hypothese, unverändert. Dieses Bein wertet sie
        // nicht — es schreibt auf, was der Produktpfad wirklich gesagt hat.
        let mut eintrag = Map::new();
        eintrag.insert("kennung".into(), json!(kennung));
        eintrag.insert(
            "befunde".into(),
            Value::Array(
                befunde
                    .iter()
                    .map(|b| {
                        json!({
                            "finding_id": b["finding_id"],
                            "candidate_source": b["candidate_source"],
                            "ursachenklasse": b["ursachenklasse"],
                            "claim_class": b["claim_class"],
                            "confidence_class": b["confidence"]["class"],
                            "confidence_score": b["confidence"]["score"],
                            "zustand": b["zustand"],
                            "alternatives": b.get("alternatives").cloned()
                                .unwrap_or_else(|| json!([])),
                            "ausschluesse": b.get("ausschluesse").cloned()
                                .unwrap_or_else(|| json!([]))
                        })
                    })
                    .collect(),
            ),
        );
        ergebnisse.push(Value::Object(eintrag));
    }

    // Das Ergebnis ist ein BAUARTEFAKT und wird nicht committet: es haengt am
    // Code, nicht am Korpus, und ein committetes Ergebnis waere eine zweite
    // Wahrheit neben dem Lauf, der es erzeugt hat.
    let ziel = wurzel().join("eq-copilot/build/p5-korpus-ergebnis.json");
    std::fs::create_dir_all(ziel.parent().unwrap()).expect("Bauordner");
    let text = serde_json::to_string_pretty(&json!({
        "erzeugt_von": "broker/tests/sonde014_p5_korpus.rs",
        "korpus_manifest_sha256": manifest["dateien"]["sitzungen.json"]["sha256"],
        "ergebnisse": ergebnisse
    }))
    .unwrap();
    std::fs::write(&ziel, text + "\n").expect("Ergebnisdatei schreiben");

    // Zwei Zusagen misst dieses Bein selbst, weil sie ohne den Prüfer schon
    // hier fallen können:
    //
    // 1. JEDE Sitzung liefert ein Ergebnis — auch die ohne Kandidat. Eine
    //    Sitzung ohne Befund wäre ein Schweigen, und M-27 verlangt ein
    //    Ergebnis.
    for (s, e) in liste.iter().zip(ergebnisse.iter()) {
        let kennung = s["kennung"].as_str().unwrap();
        assert!(
            !e["befunde"].as_array().unwrap().is_empty(),
            "{kennung}: der Produktpfad schweigt statt zu antworten"
        );
    }
    // 2. Kein Befund trägt eine Ursachenklasse ausserhalb der geschlossenen
    //    Menge — der Prüfer liest sie aus demselben Manifest.
    let klassen: Vec<&str> = manifest["mengen"]["ursachenklassen"]
        .as_array()
        .unwrap()
        .iter()
        .map(|v| v.as_str().unwrap())
        .collect();
    for e in &ergebnisse {
        for b in e["befunde"].as_array().unwrap() {
            let k = b["ursachenklasse"].as_str().unwrap_or_default();
            assert!(
                klassen.contains(&k),
                "{}: Ursachenklasse {k} steht nicht im Korpusmanifest",
                e["kennung"]
            );
        }
    }
}
