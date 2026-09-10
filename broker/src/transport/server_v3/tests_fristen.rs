//! Fristen und Fassungen: was der Worker prueft, bevor die Senke etwas sieht.
//!
//! Der Worker entscheidet GENAU drei Dinge (Entwurf §53.9): Envelope,
//! Grenzen und Authentisierung. Diese Faelle messen die ersten beiden — die
//! zwei Fristen mit ihrem exakten Rand und die Abweisung eines Rahmens der
//! falschen Familie oder einer unbekannten Fassung. Alle drei
//! Abweisungspfade enden VOR der Senke; ein Frame, den der Coordinator nie
//! sehen darf, darf ihn auch nicht kurz erreichen.
//!
//! ⚠️ `bootstrap_und_senkenfrist_…` liest mit `include_str!` den
//! Produktteil von `listener.rs` und `griff.rs`. Die Pfade sind relativ zu
//! DIESER Datei — sie liegt deshalb direkt im Modulverzeichnis und nicht in
//! einem Unterordner.

use super::tests_hilfe::*;
use super::*;

#[test]
fn bootstrap_und_senkenfrist_minus_eins_und_exakt_sind_inklusive() {
    assert_eq!(BOOTSTRAP_FRIST, Duration::from_millis(5000));
    assert_eq!(SENKE_FRIST, Duration::from_millis(2000));
    let basis = Instant::now();
    for frist in [BOOTSTRAP_FRIST, SENKE_FRIST] {
        let deadline = basis + frist;
        assert!(basis + frist - Duration::from_millis(1) < deadline);
        assert!(basis + frist >= deadline);
    }

    // Beide Muster stehen seit der Modulaufteilung in verschiedenen Dateien:
    // der Wachhundfilter in listener.rs, die Joinfrist in griff.rs. Der Schnitt am
    // Testmodul-Marker bleibt, damit kein Treffer aus einem Testteil zaehlt.
    let produktteil = |quelle: &'static str| {
        let testmodul_marker = ["\n#[cfg(test)]\n", "mod tests {"].concat();
        match quelle.rfind(&testmodul_marker) {
            Some(tests_beginn) => &quelle[..tests_beginn],
            None => quelle,
        }
    };
    assert!(produktteil(include_str!("listener.rs")).contains(".filter(|(_, f)| *f <= jetzt)"));
    assert!(produktteil(include_str!("griff.rs")).contains("if Instant::now() >= bis"));

    let (blockieren, empfang) = std::sync::mpsc::channel::<()>();
    let thread = std::thread::spawn(move || {
        let _ = empfang.recv();
    });
    assert!(!join_mit_frist(thread, Duration::ZERO, || {}));
    drop(blockieren);
}

/// T2-Befund 5: der Vertrag ordnet die Familien den Verbindungsarten zu.
#[test]
fn p0_auf_der_telemetriepipe_wird_abgewiesen() {
    let pipe = probe_pipe("familie-tele");
    let senke = Arc::new(ZaehlSenke::default());
    let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

    let steuer = Testclient::neu(&pipe).unwrap();
    assert!(steuer.schreiben(&control_hello(&"c".repeat(32))));
    let (link, challenge) = welcome_lesen(&steuer).expect("welcome");

    let tele = Testclient::neu(&pipe).unwrap();
    assert!(tele.schreiben(&telemetry_hello(&"c".repeat(32), &link, &challenge)));
    assert!(welcome_lesen(&tele).is_some());

    assert!(tele.schreiben(&p0("{\"type\":\"heartbeat\",\"sequence\":1}")));

    let stat = griff.statistik.clone();
    assert!(
        warte_auf(4000, || stat.geschlossen_familie.load(Ordering::SeqCst)
            == 1),
        "ein P0 auf der Telemetriepipe muss die Verbindung schliessen"
    );
    assert_eq!(
        senke.p0.load(Ordering::SeqCst),
        0,
        "und er darf die Senke nie erreichen"
    );
}

/// T2-Befund 5, Gegenrichtung.
#[test]
fn p2_auf_der_controlpipe_wird_abgewiesen() {
    let pipe = probe_pipe("familie-control");
    let senke = Arc::new(ZaehlSenke::default());
    let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

    let steuer = Testclient::neu(&pipe).unwrap();
    assert!(steuer.schreiben(&control_hello(&"d".repeat(32))));
    assert!(welcome_lesen(&steuer).is_some());
    assert!(steuer.schreiben(&p2(&[1u8; 32])));

    let stat = griff.statistik.clone();
    assert!(
        warte_auf(4000, || stat.geschlossen_familie.load(Ordering::SeqCst)
            == 1),
        "ein P2 auf der Controlpipe muss die Verbindung schliessen"
    );
    assert_eq!(senke.p2.load(Ordering::SeqCst), 0);
}

#[test]
fn unbekannter_schema_minor_wird_vor_der_senke_abgewiesen() {
    // Die drei Familien haben SEIT SONDE-013 nicht mehr dieselbe
    // Obergrenze. Eine gemeinsame Schleife haette das nicht ausdruecken
    // koennen und war genau deshalb still falsch, als P0/P1 stiegen.
    for familie in [Familie::P0, Familie::P1, Familie::P2] {
        assert!(schema_minor_bekannt(familie, 0));
        assert!(schema_minor_bekannt(familie, 1));
    }
    // P0 und P1 tragen die Fassung 2 (Experimentfamilien, belegte
    // Evidenzfelder, zwei neue Invalidierungsgruende) und seit SONDE-014
    // die Fassung 3 (Intent, Assistentenschritt, Draft und Userurteil).
    assert!(schema_minor_bekannt(Familie::P0, 2));
    assert!(schema_minor_bekannt(Familie::P1, 2));
    assert!(schema_minor_bekannt(Familie::P0, 3));
    assert!(schema_minor_bekannt(Familie::P1, 3));
    // NAK-213: die Fassung 4 ist ab hier bekannt, die 3 bleibt lesbar.
    assert!(schema_minor_bekannt(Familie::P0, 4));
    assert!(schema_minor_bekannt(Familie::P1, 4));
    // SONDE-015: die Fassung 5 (state_report.dsp) ebenso; die 4 bleibt lesbar.
    assert!(schema_minor_bekannt(Familie::P0, 5));
    assert!(schema_minor_bekannt(Familie::P1, 5));
    // P2 nicht: dort ist seit SONDE-013 nichts hinzugekommen, was eine
    // Fassung braeuchte - `integration_samples` ist ein optionales
    // FlatBuffers-Feld und damit der additive Fall, den das Format traegt.
    assert!(!schema_minor_bekannt(Familie::P2, 2));
    // Und die Gegenprobe nach oben: eine Fassung, die es nicht gibt,
    // wird auch bei P0/P1 abgewiesen.
    // NAK-213: derselbe Randwert eine Fassung weiter — aus 4 wird 5.
    // SONDE-015: und weiter — aus 5 wird 6.
    for familie in [Familie::P0, Familie::P1, Familie::P2] {
        assert!(!schema_minor_bekannt(familie, 6));
        assert!(!schema_minor_bekannt(familie, 200));
    }
}

/// SONDE-013: die Fassung des Transports und die des JSON-Lesers sind
/// DIESELBE Zahl. Liefen sie auseinander, wiese der Server Rahmen ab, die
/// der Coordinator lesen koennte - oder liesse Rahmen durch, fuer die er
/// keinen Leser hat. Gemessen, nicht kommentiert.
#[test]
fn transportfassung_und_json_leser_stimmen_ueberein() {
    assert_eq!(
        u8::from(P1_SCHEMA_MINOR),
        crate::coordinator::JSON_SCHEMA_MINOR_AKTIV_FUER_TEST
    );
    assert_eq!(P0_SCHEMA_MINOR, P1_SCHEMA_MINOR);
}
