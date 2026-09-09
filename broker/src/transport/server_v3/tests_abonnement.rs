//! Abonnement und Uebergabe an den Coordinator.
//!
//! Ein Abonnement haengt am EIGENEN Control-Link: ein zweiter Client kann es
//! nicht mitbenutzen, und ein P2-Push geht ausschliesslich auf die
//! Telemetriepipe des Abonnenten, mit den aktiven Minorfassungen im
//! Envelope. Nach dem Aufraeumen eines Abonnements erreicht ein weiterer Push
//! niemanden mehr.
//!
//! ⚠️ **Der Worker uebergibt ausschliesslich validierte Bytes.** Er kennt die
//! Bedeutung des Payloads nicht — die kennt erst der Coordinator (§53.9). Der
//! Fall `join_kandidat_…` misst genau diese Grenze: ein Join-Kandidat reist
//! ueber die Probe-Pipe OHNE eigene Nachrichtenfamilie.

use super::listener::v3_server_starten_intern;
use super::tests_hilfe::*;
use super::*;
use crate::transport::bootstrap::Adresse;

/// SONDE-011 A4-SI / 28-B: Die semantische Bindung wird durch den echten
/// Listener, zwei echte Control-Pipes und dessen fruehen Cleanup-Hook
/// gefahren. A5/A8/B3c koennen dieses Linkeigentum nicht messen.
#[test]
fn subscription_ist_an_eigenen_control_link_gebunden() {
    let pipe = probe_pipe("subscriptionbesitz");
    let coordinator = Arc::new(crate::coordinator::Coordinator::default());
    let griff = v3_server_starten(&pipe, coordinator.clone(), "test".into()).unwrap();
    let adresse_a = test_adresse('a');
    let adresse_b = test_adresse('b');

    let control_a = Testclient::neu(&pipe).unwrap();
    assert!(control_a.schreiben(&control_hello_adresse(&adresse_a)));
    let (link_a, _) = welcome_lesen(&control_a).expect("welcome A");
    let control_b = Testclient::neu(&pipe).unwrap();
    assert!(control_b.schreiben(&control_hello_adresse(&adresse_b)));
    let (_link_b, _) = welcome_lesen(&control_b).expect("welcome B");

    // A versucht zuerst, die effektive Adresse/Session von B zu besitzen.
    assert!(control_a.schreiben(&subscribe(&adresse_b)));
    assert!(warte_auf(3000, || coordinator.subscription_abweisungen() == 1));
    assert!(coordinator
        .letzter_subscription_grund()
        .contains("fremde effektive Adresse"));
    assert_eq!(coordinator.subscription_anzahl(), 0);

    assert!(control_a.schreiben(&subscribe(&adresse_a)));
    assert!(control_b.schreiben(&subscribe(&adresse_b)));
    assert!(warte_auf(3000, || coordinator.subscription_anzahl() == 2));

    // Protokollfehler beendet B. A muss dabei exakt erhalten bleiben.
    assert!(control_b.schreiben(&p2(b"falsche-familie")));
    assert!(warte_auf(5000, || {
        coordinator.subscription_anzahl() == 1 && coordinator.subscription_cleanups() == 1
    }));
    assert_eq!(
        coordinator.session_push_ziele(&adresse_a.session_epoch, &adresse_a),
        vec![link_a]
    );
    assert!(coordinator
        .session_push_ziele(&adresse_b.session_epoch, &adresse_b)
        .is_empty());

    // EOF von A entfernt ausschliesslich A und genau einmal.
    drop(control_a);
    assert!(warte_auf(5000, || {
        coordinator.subscription_anzahl() == 0 && coordinator.subscription_cleanups() == 2
    }));
    assert!(coordinator
        .session_push_ziele(&adresse_a.session_epoch, &adresse_a)
        .is_empty());
    drop(control_b);
    drop(griff);
}

#[test]
fn join_kandidat_laeuft_ueber_probe_pipe_ohne_join_candidate_nachrichtenfamilie() {
    let pipe = probe_pipe("joinwire");
    let coordinator = Arc::new(crate::coordinator::Coordinator::default());
    let sender = V3Sender::neu();
    coordinator.session_push_setzen(Arc::new(sender.clone()));
    let griff = v3_server_starten_mit_epoch_und_sender(
        &pipe,
        coordinator.clone(),
        "test".into(),
        neue_kennung(),
        sender,
    )
    .unwrap();

    let mut main_adresse = test_adresse('a');
    main_adresse.instance_id = "1".repeat(32);
    main_adresse.runtime_nonce = "2".repeat(32);
    let main = Testclient::neu(&pipe).unwrap();
    assert!(main.schreiben(&control_hello_fach(&main_adresse, "main", Some(7711))));
    assert!(welcome_lesen(&main).is_some());
    assert!(main.schreiben(&vollstaendiger_heartbeat(&main_adresse, 1)));
    assert!(frame_json_lesen(&main).is_some_and(|wert| wert["type"] == "heartbeat_ack"));
    assert!(main.schreiben(&subscribe(&main_adresse)));
    let erster = frame_json_lesen(&main).expect("erster session_snapshot");
    assert_eq!(erster["type"], "session_snapshot");
    assert_eq!(erster["mitglieder"].as_array().unwrap().len(), 1);

    let mut bridge_adresse = main_adresse.clone();
    bridge_adresse.instance_id = "3".repeat(32);
    bridge_adresse.runtime_nonce = "4".repeat(32);
    let bridge = Testclient::neu(&pipe).unwrap();
    assert!(bridge.schreiben(&control_hello_fach(&bridge_adresse, "active_probe", None,)));
    assert!(welcome_lesen(&bridge).is_some());
    assert!(bridge.schreiben(&vollstaendiger_heartbeat(&bridge_adresse, 1)));
    assert!(frame_json_lesen(&bridge).is_some_and(|wert| wert["type"] == "heartbeat_ack"));
    let join_snapshot = frame_json_lesen(&main).expect("Join-Snapshot ueber Probe-Pipe");
    assert_eq!(join_snapshot["type"], "session_snapshot");
    assert_eq!(join_snapshot["mitglieder"].as_array().unwrap().len(), 2);
    assert_eq!(join_snapshot["beitritt_bestaetigung_noetig"], true);
    assert!(join_snapshot.get("join_candidate").is_none());

    let schema: serde_json::Value = serde_json::from_str(include_str!(
        "../../../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
    ))
    .unwrap();
    let reserviert: serde_json::Value = serde_json::from_str(include_str!(
        "../../../../eq-copilot/schemas/v3/reservierte-nachrichten-v1.json"
    ))
    .unwrap();
    let definiert = schema["oneOf"].as_array().unwrap().len();
    let spaeter = reserviert["reserviert"].as_array().unwrap().len();
    // SONDE-013 E-02 (03.09.2026): `experiment_begin` und
    // `experiment_abort` sind aus der Reserve nach `definiert` gewandert,
    // `experiment_manual_result` ist neu dazugekommen. 18/9 -> 21/7.
    // Nacharbeit 2 (04.09.2026, Befunde R16/R21): `experiment_candidate`
    // kommt als vierte Familie dieses Tickets dazu — der Schritt zwischen
    // Begin und Ergebnis. 21/7 -> 22/7.
    // SONDE-014 E-09/E-10/E-11 (06.09.2026): `draft_offer` und
    // `user_verdict` wandern aus der Reserve nach `definiert`,
    // `intent_update` und `assistant_step_update` kommen neu dazu.
    // 22/7 -> 26/5.
    assert_eq!(definiert, 26);
    assert_eq!(spaeter, 5);
    assert_eq!(
        definiert + spaeter,
        reserviert["gesamt_erwartet"].as_u64().unwrap() as usize
    );

    drop(bridge);
    drop(main);
    drop(griff);
}

#[test]
fn io_worker_uebergibt_validierte_bytes_ausschliesslich_an_den_coordinator() {
    let pipe = probe_pipe("ioworkergraph");
    let coordinator = Arc::new(crate::coordinator::Coordinator::default());
    let griff = v3_server_starten(&pipe, coordinator.clone(), "test".into()).unwrap();
    let adresse = test_adresse('6');
    assert_eq!(coordinator.client_anzahl(), 0);

    let kaputt = Testclient::neu(&pipe).unwrap();
    assert!(kaputt.schreiben(&praefix("{")));
    assert!(warte_auf(3000, || griff
        .statistik
        .geschlossen_bootstrap
        .load(Ordering::SeqCst)
        >= 1));
    assert_eq!(coordinator.client_anzahl(), 0);

    let gueltig = Testclient::neu(&pipe).unwrap();
    assert!(gueltig.schreiben(&control_hello_adresse(&adresse)));
    assert!(welcome_lesen(&gueltig).is_some());
    assert!(warte_auf(3000, || coordinator.client_anzahl() == 1));
    assert_eq!(
        coordinator
            .modell_sicht(&adresse.project_binding_id, &adresse.session_epoch)
            .clients[0]
            .adresse,
        adresse
    );

    drop(kaputt);
    drop(gueltig);
    drop(griff);
}

#[test]
fn broker_p2_push_nutzt_subscriber_telemetriepipe_und_aktive_minors() {
    let pipe = probe_pipe("sonde012-p2push");
    let coordinator = Arc::new(crate::coordinator::Coordinator::default());
    let sender = V3Sender::neu();
    coordinator.session_push_setzen(Arc::new(sender.clone()));
    let griff = v3_server_starten_mit_epoch_und_sender(
        &pipe,
        coordinator.clone(),
        "test".into(),
        neue_kennung(),
        sender,
    )
    .unwrap();

    let main_adresse = Adresse {
        logon_sid: "S-1-5-21-1111111111-2222222222-3333333333-1001".into(),
        project_binding_id: "1".repeat(32),
        session_epoch: "2".repeat(32),
        instance_id: "a".repeat(32),
        runtime_nonce: "b".repeat(32),
    };
    let main_control = Testclient::neu(&pipe).unwrap();
    assert!(main_control.schreiben(&control_hello_fach(&main_adresse, "main", Some(7711),)));
    let main_welcome = frame_roh_lesen(&main_control).expect("Main-Welcome");
    assert_eq!(main_welcome.kopf.familie, Familie::P0);
    assert_eq!(main_welcome.kopf.schema_minor, P0_SCHEMA_MINOR);
    let main_welcome_json: serde_json::Value =
        serde_json::from_slice(&main_welcome.payload).expect("Welcome ist JSON");
    let main_link = main_welcome_json["link_id"]
        .as_str()
        .expect("Welcome-Link")
        .to_owned();
    let main_challenge = main_welcome_json["challenge"]
        .as_str()
        .expect("Welcome-Challenge")
        .to_owned();
    let main_telemetrie = Testclient::neu(&pipe).unwrap();
    assert!(main_telemetrie.schreiben(&telemetry_hello_adresse(
        &main_adresse,
        &main_link,
        &main_challenge,
    )));
    assert!(welcome_lesen(&main_telemetrie).is_some());
    assert!(main_control.schreiben(&vollstaendiger_heartbeat(&main_adresse, 1)));
    assert!(frame_json_lesen(&main_control).is_some_and(|wert| wert["type"] == "heartbeat_ack"));
    assert!(main_control.schreiben(&subscribe(&main_adresse)));
    let snapshot = frame_roh_lesen(&main_control).expect("absoluter Snapshot");
    assert_eq!(snapshot.kopf.familie, Familie::P1);
    assert_eq!(snapshot.kopf.schema_minor, P1_SCHEMA_MINOR);
    assert_eq!(
        serde_json::from_slice::<serde_json::Value>(&snapshot.payload).unwrap()["type"],
        "session_snapshot"
    );

    let source_adresse = Adresse {
        logon_sid: main_adresse.logon_sid.clone(),
        project_binding_id: main_adresse.project_binding_id.clone(),
        session_epoch: main_adresse.session_epoch.clone(),
        instance_id: format!("{:032x}", 3),
        runtime_nonce: "4".repeat(32),
    };
    let source_control = Testclient::neu(&pipe).unwrap();
    assert!(source_control.schreiben(&control_hello_fach(
        &source_adresse,
        "active_probe",
        Some(7711),
    )));
    let (source_link, source_challenge) =
        welcome_lesen(&source_control).expect("Source-Welcome");
    let source_telemetrie = Testclient::neu(&pipe).unwrap();
    assert!(source_telemetrie.schreiben(&telemetry_hello_adresse(
        &source_adresse,
        &source_link,
        &source_challenge,
    )));
    assert!(welcome_lesen(&source_telemetrie).is_some());
    assert!(source_control.schreiben(&vollstaendiger_heartbeat(&source_adresse, 1)));
    assert!(
        frame_json_lesen(&source_control).is_some_and(|wert| wert["type"] == "heartbeat_ack")
    );

    let payload =
        include_bytes!("../../../../eq-copilot/fixtures/v3/flatbuffers/gueltig/live-64-band.bin");
    assert!(source_telemetrie
        .schreiben(&envelope_schreiben(Familie::P2, P2_SCHEMA_MINOR, payload).unwrap()));
    let weiter = frame_roh_lesen(&main_telemetrie).expect("P2-Push an Main");
    assert_eq!(weiter.kopf.familie, Familie::P2);
    assert_eq!(weiter.kopf.schema_minor, P2_SCHEMA_MINOR);
    assert_eq!(weiter.payload, payload);
    assert_eq!(coordinator.p2_live_frames(), 1);

    drop(source_telemetrie);
    drop(source_control);
    drop(main_telemetrie);
    drop(main_control);
    drop(griff);
}

/// SONDE-011 A4-SI / 28-B: Der Serverpfad faehrt EOF,
/// Protokoll-/Writefehler und Serverstopp wirklich. Ein Post-Welcome-
/// Idle-Timeout existiert heute nicht; dessen identischer atomarer Hook
/// wird deterministisch ausgeloest, statt im Test eine nicht existente
/// Transportfrist zu erfinden.
#[test]
fn subscription_cleanup_vor_weiterem_push() {
    for grund in [
        "EOF",
        "Protokollfehler",
        "Timeout",
        "Writefehler",
        "Serverstopp",
    ] {
        let pipe = probe_pipe(&format!("subscriptioncleanup-{grund}"));
        let coordinator = Arc::new(crate::coordinator::Coordinator::default());
        let writer_fehler = Arc::new(AtomicBool::new(false));
        let cancel_vor_read = Arc::new(AtomicU64::new(CANCEL_VOR_READ_INAKTIV));
        let mut griff = if grund == "Writefehler" {
            v3_server_starten_intern(
                &pipe,
                coordinator.clone(),
                "test".into(),
                neue_kennung(),
                V3Sender::neu(),
                Arc::new(AtomicU64::new(0)),
                writer_fehler.clone(),
                cancel_vor_read.clone(),
                false,
                V3SecurityTestOptionen::default(),
            )
            .unwrap()
        } else {
            v3_server_starten(&pipe, coordinator.clone(), "test".into()).unwrap()
        };
        let statistik = griff.statistik.clone();
        let adresse = test_adresse('c');
        let client = Testclient::neu(&pipe).unwrap();
        assert!(
            client.schreiben(&control_hello_adresse(&adresse)),
            "{grund}"
        );
        let (link, _) = welcome_lesen(&client).expect("welcome");
        assert!(client.schreiben(&subscribe(&adresse)), "{grund}");
        assert!(
            warte_auf(3000, || coordinator.subscription_anzahl() == 1),
            "{grund}"
        );

        let mut client = Some(client);
        match grund {
            "EOF" => drop(client.take()),
            "Protokollfehler" => {
                assert!(client.as_ref().unwrap().schreiben(&p2(b"falsche-familie")));
            }
            "Timeout" => coordinator.control_ende(&link),
            "Writefehler" => {
                // Der naechste Read liefert den Heartbeat. Danach haelt die
                // Testnaht den Reader NACH seinem Ende-Check, aber VOR dem
                // folgenden ReadFile. Der Writer wartet genau darauf,
                // scheitert und setzt sein einmaliges Cancel ab. Erst dann
                // darf der Reader weiter: Das Ende-Event, nicht EOF oder
                // ein zufaellig bereits pending Read, muss ihn aufloesen.
                cancel_vor_read.store(CANCEL_VOR_READ_BEREIT, Ordering::SeqCst);
                writer_fehler.store(true, Ordering::SeqCst);
                assert!(client
                    .as_ref()
                    .unwrap()
                    .schreiben(&heartbeat_fuer_adresse(&adresse, 1)));
                assert!(
                    warte_auf(5000, || statistik.geschlossen_writer.load(Ordering::SeqCst)
                        == 1),
                    "Writerfehler wurde nicht erreicht"
                );
                assert!(
                    warte_auf(5000, || cancel_vor_read.load(Ordering::SeqCst)
                        == CANCEL_VOR_READ_FORTGESETZT),
                    "Cancel muss VOR dem erst danach abgesetzten Read liegen"
                );
                assert!(
                    client.is_some(),
                    "Client muss fuer den isolierten Writerfehler offen bleiben"
                );
            }
            "Serverstopp" => griff.stoppen(),
            _ => unreachable!(),
        }

        assert!(
            warte_auf(5000, || coordinator.subscription_anzahl() == 0),
            "{grund}: Subscription blieb stehen"
        );
        assert!(
            coordinator
                .session_push_ziele(&adresse.session_epoch, &adresse)
                .is_empty(),
            "{grund}: Push sah den geschlossenen Link"
        );
        assert_eq!(coordinator.subscription_cleanups(), 1, "{grund}");
        drop(client);
        griff.stoppen();
    }
}
