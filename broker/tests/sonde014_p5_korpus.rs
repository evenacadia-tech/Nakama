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
           band: (usize, usize)) -> Vec<u8> {
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
    serde_json::to_vec(&wert).unwrap()
}

/// Fährt EINE Sitzung durch den Produktpfad und gibt die ausgegebenen
/// Befunde zurück.
fn sitzung_fahren(s: &Value, band: (usize, usize)) -> Vec<Value> {
    let c = Coordinator::mit_uhr(Arc::new(ManualClock::default()), hex(0xbeef));
    let master = adresse(1);
    anmelden(&c, "main", &master, "main", Some(0));
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

    let master_db = s["master"]["anhebung_db"].as_f64().unwrap_or(0.0);
    let master_fenster = s["master"]["fenster"].as_u64().unwrap_or(0) as usize;

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
    for i in 0..master_fenster {
        c.p1("main", &evidenz(&master, i, basis + (i as i64) * 512, master_db, band));
    }
    for (n, q) in quellen.iter().enumerate() {
        let instanz = q["instanz"].as_u64().unwrap() as usize;
        let a = adresse(instanz);
        let db = q["anhebung_db"].as_f64().unwrap_or(0.0);
        let fenster = q["fenster"].as_u64().unwrap_or(0) as usize;
        let versatz = q["versatz_fenster"].as_i64().unwrap_or(0);
        for i in 0..fenster {
            let zeit = basis + (i as i64 + versatz) * 512;
            c.p1(
                &format!("sonde{n}"),
                &evidenz(&a, 1000 + n * 100 + i, zeit, db, band),
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
        // Die AUSGEGEBENE Hypothese, unverändert. Dieses Bein wertet nicht —
        // es schreibt auf, was der Produktpfad wirklich gesagt hat.
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
