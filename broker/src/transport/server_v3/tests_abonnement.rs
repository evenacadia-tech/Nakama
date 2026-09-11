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
use super::queues::{AusgangEintrag, Ausgangsart};
use super::tests_hilfe::*;
use super::*;
use crate::coordinator::{Coordinator, CoordinatorFlushTestHaken, ManualClock, SessionPush};
use crate::store::{StoreKonfiguration, StoreWriter, BUSY_TIMEOUT_MS, STORE_DATEINAME};
use crate::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
use serde_json::{json, Value};

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
    // 🔑 SONDE-015 B-01: gesendet wird mit der Zahl aus dem REGISTER - also
    // mit derselben, die der C++-Sender auf den Draht schreibt. Mit
    // `P2_SCHEMA_MINOR` hier folgte der Test einer zurueckgedrehten Konstante
    // stumm nach unten und bliebe gruen, waehrend das Produkt jede echte Sonde
    // beim ersten Frame abweist. Faellt die Empfaengergrenze zurueck, schliesst
    // `verbindung.rs` die Verbindung und `expect` unten schlaegt zu.
    let sendeminor = p2_minor_aus_register();
    let envelope_vorher = griff
        .statistik
        .geschlossen_envelope
        .load(Ordering::SeqCst);
    assert!(source_telemetrie
        .schreiben(&envelope_schreiben(Familie::P2, sendeminor, payload).unwrap()));

    /*  🔑 SONDE-015 B-01-Rest (Nacharbeit 2): der VERBINDUNGSABBRUCH wird
        BEGRENZT beobachtet, BEVOR irgendwo blockierend gelesen wird.

        Der erste Anlauf dieses Rotbeweises hing: faellt `P2_SCHEMA_MINOR`
        zurueck, schliesst `verbindung.rs` die QUELLverbindung und der Test
        wartete danach mit fristlosem `ReadFile` auf der weiterhin offenen
        Main-Pipe. Ein Haenger ist kein Rot. `warte_auf_p2_entscheidung` kehrt
        zurueck, sobald eines von beidem gilt - Frame zugestellt oder
        Quellverbindung zu -, und die Zusicherung darunter faellt genau an der
        Zusage. */
    assert!(
        warte_auf_p2_entscheidung(
            &source_telemetrie,
            &main_telemetrie,
            FRIST_P2_ENTSCHIEDEN_MS
        ),
        "innerhalb der Frist ist weder ein Frame angekommen noch die Quellverbindung gefallen"
    );
    assert!(
        source_telemetrie.verbindung_steht(),
        "die Quellverbindung bleibt offen: der Broker kennt die Fassung {sendeminor} des Senders.          Ist sie zu, hat `verbindung.rs` den Rahmen als `schema_minor {sendeminor} fuer P2 unbekannt`          abgewiesen - ein Fassungsschritt, den nur eine Seite kennt"
    );
    assert_eq!(
        griff
            .statistik
            .geschlossen_envelope
            .load(Ordering::SeqCst),
        envelope_vorher,
        "kein Envelope-Abbruch: der Zaehler, den `verbindung.rs` beim Schliessen hochzaehlt, steht still"
    );

    let weiter = frame_roh_lesen(&main_telemetrie)
        .expect("P2-Push an Main: der Broker nimmt die Fassung des Senders an");
    assert_eq!(weiter.kopf.familie, Familie::P2);
    assert_eq!(weiter.kopf.schema_minor, sendeminor);
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

// ═══════════════════════════════════════════════════════════════════════════
// NAK-246 Etappe 5 - Zustellordnung (D5) und Objektschluessel (D9) HINTER dem
// echten `V3Sender` und dem echten `Ausgang` (Manifest §3.5, §3.6; L-M1-1).
//
// Der Consumer des Link-Ausgangs ist der Test selbst: er entnimmt erst, wenn
// der Fall es sagt. Die Writerqueue ist damit deterministisch angehalten - kein
// langsamer Leser, kein Schlafintervall (R-M1-7). Der Coordinator ist mit dem
// echten Sender verdrahtet (`session_push_setzen`), und jede Zustellung laeuft
// durch `V3Sender::snapshot_schreiben` in genau diesen Ausgang. Eine Probe, die
// `SessionPush` selbst implementiert und zustellt, waere kein Beleg (R-M1-5).
// ═══════════════════════════════════════════════════════════════════════════

/// Frist jedes Warteschritts. Sie trennt Rot von Haenger, nicht schnell von
/// langsam: jeder Schritt entscheidet sich im Normalfall in Millisekunden.
const NAK246_FRIST_MS: u64 = 5_000;

/// So lange muss die Queue leer bleiben, bis eine Zustellfolge als
/// vollstaendig gilt.
const NAK246_RUHE_MS: u64 = 500;

fn nak246_hex(n: usize) -> String {
    format!("{n:032x}")
}

/// Ein eigener Ordner fuer den echten Store eines Falls; faellt mit dem Fall.
struct Nak246Ordner(std::path::PathBuf);

impl Nak246Ordner {
    fn neu(fall: &str) -> Self {
        let pfad = std::env::temp_dir().join(format!(
            "nakama-nak246-{fall}-{}-{}",
            std::process::id(),
            uuid::Uuid::new_v4().simple()
        ));
        std::fs::create_dir_all(&pfad).unwrap();
        Self(pfad)
    }

    fn db(&self) -> std::path::PathBuf {
        self.0.join(STORE_DATEINAME)
    }
}

impl Drop for Nak246Ordner {
    fn drop(&mut self) {
        let _ = std::fs::remove_dir_all(&self.0);
    }
}

/// Die Buehne eines Falls: ein Coordinator (mit oder ohne echten Store), der
/// ECHTE `V3Sender` als seine Senke und der ECHTE `Ausgang` des Links `main`.
/// Die Felder fallen in Deklarationsreihenfolge: Coordinator, Store, Ordner.
struct Zustellbuehne {
    coordinator: Arc<Coordinator>,
    sender: V3Sender,
    ausgang: Arc<Ausgang>,
    main: HelloControl,
    writer: Option<StoreWriter>,
    ordner: Option<Nak246Ordner>,
}

impl Zustellbuehne {
    /// Main angemeldet, Heartbeat mit Deskriptor, abonniert. Der absolute
    /// Snapshot des Subscribe laeuft durch Sender und Ausgang; der Test
    /// entnimmt und bestaetigt ihn, damit jeder Fall mit leerer Queue beginnt.
    fn aufbauen(fall: &str, mit_store: bool) -> Self {
        let (ordner, writer, coordinator) = if mit_store {
            let ordner = Nak246Ordner::neu(fall);
            let mut konfiguration = StoreKonfiguration::fuer_pfad(ordner.db());
            konfiguration.remote_volume_override = Some(false);
            let writer = StoreWriter::starten(konfiguration);
            assert!(!writer.ist_degradiert(), "{fall}: {:?}", writer.handle().sicht());
            let coordinator = Arc::new(Coordinator::mit_store(
                Arc::new(ManualClock::default()),
                nak246_hex(99),
                &writer,
            ));
            (Some(ordner), Some(writer), coordinator)
        } else {
            let coordinator = Arc::new(Coordinator::mit_uhr(
                Arc::new(ManualClock::default()),
                nak246_hex(99),
            ));
            (None, None, coordinator)
        };
        let sender = V3Sender::neu();
        coordinator.session_push_setzen(Arc::new(sender.clone()));
        let ausgang = Arc::new(Ausgang::neu());
        sender
            .control_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .insert("main".into(), ausgang.clone());
        let main = nak246_main_hello();
        assert!(
            coordinator.control_hello_registrieren("main", &main).angenommen,
            "{fall}: Main angemeldet"
        );
        assert!(nak246_heartbeat(&coordinator, &main.adresse), "{fall}: Heartbeat");
        let buehne = Self {
            coordinator,
            sender,
            ausgang,
            main,
            writer,
            ordner,
        };
        let abo = buehne.resubscribe_starten();
        let erster = eintrag_abholen(&buehne.ausgang, NAK246_FRIST_MS).unwrap_or_else(|| {
            panic!("{fall}: der absolute Snapshot des Subscribe kommt am Ausgang an")
        });
        bestaetigen(erster, true);
        assert!(abo.join().unwrap(), "{fall}: abonniert");
        buehne
    }

    fn resubscribe_starten(&self) -> std::thread::JoinHandle<bool> {
        let coordinator = self.coordinator.clone();
        let payload = serde_json::to_vec(&json!({
            "type": "subscribe_session",
            "adresse": self.main.adresse,
            "session_epoch": self.main.adresse.session_epoch,
        }))
        .unwrap();
        std::thread::spawn(move || coordinator.subscribe_json("main", &payload))
    }

    fn descriptor_mit_label(&self, label: &str) -> Value {
        let basis: Value = serde_json::from_slice(&self.coordinator.session_snapshot_json(
            &self.main.adresse.project_binding_id,
            &self.main.adresse.session_epoch,
        ))
        .unwrap();
        let mut descriptor = basis["mitglieder"][0]["probe_descriptor"].clone();
        assert!(descriptor.is_object(), "der Heartbeat hat einen Deskriptor gebildet: {basis}");
        descriptor["label"] = Value::String(label.into());
        descriptor
    }

    fn descriptor_setzen_starten(&self, descriptor: Value) -> std::thread::JoinHandle<bool> {
        let coordinator = self.coordinator.clone();
        std::thread::spawn(move || coordinator.descriptor_setzen("main", descriptor))
    }

    fn live_label(&self) -> Option<String> {
        let wert: Value = serde_json::from_slice(&self.coordinator.session_snapshot_json(
            &self.main.adresse.project_binding_id,
            &self.main.adresse.session_epoch,
        ))
        .ok()?;
        label_von(&wert)
    }

    /// Ordinal und Label der Sessionprojektion im Store.
    fn projektion(&self) -> (i64, Option<String>) {
        let (ord, payload) = self
            .writer
            .as_ref()
            .expect("Fall mit Store")
            .handle()
            .session_state_lesen(
                &self.main.adresse.project_binding_id,
                &self.main.adresse.session_epoch,
            )
            .unwrap()
            .expect("Projektion vorhanden");
        (ord, label_von(&serde_json::from_slice(&payload).unwrap()))
    }

    /// Die offene Zustellschuld des Main fuer einen Objektschluessel (ihre Ordinale).
    fn schuld(&self, objekt_schluessel: &str) -> Vec<i64> {
        self.writer
            .as_ref()
            .expect("Fall mit Store")
            .handle()
            .outbox_lesen()
            .unwrap()
            .into_iter()
            .filter(|(ziel, _, _)| {
                ziel.instance_id == self.main.adresse.instance_id
                    && ziel.object_key == objekt_schluessel
            })
            .map(|(_, ord, _)| ord)
            .collect()
    }
}

fn nak246_main_hello() -> HelloControl {
    HelloControl {
        typ: "hello".into(),
        connection_kind: "control".into(),
        protocol: 3,
        plugin_version: "nak246-etappe5".into(),
        plugin_kind: "main".into(),
        adresse: Adresse {
            logon_sid: "S-1-5-21-1-2-3-1001".into(),
            project_binding_id: nak246_hex(1),
            session_epoch: nak246_hex(2),
            instance_id: nak246_hex(10),
            runtime_nonce: nak246_hex(100),
        },
        host: Some(HostAngabe {
            pid: 77,
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

fn nak246_heartbeat(coordinator: &Coordinator, adresse: &Adresse) -> bool {
    coordinator.heartbeat_kontakt(
        "main",
        Some(&json!({
            "type": "heartbeat",
            "adresse": adresse,
            "sequence": 1,
            "state_revision": 1,
            "capabilities": {
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
            },
            "zaehler": {},
            "runtime": {"messpunkt": "insert", "betrieb": "active"}
        })),
    )
}

fn ausgang_laenge(ausgang: &Ausgang) -> usize {
    ausgang.inhalt.lock().unwrap_or_else(|e| e.into_inner()).0.len()
}

fn schluessel_von(eintrag: &AusgangEintrag) -> String {
    match &eintrag.art {
        Ausgangsart::Snapshot(schluessel) => schluessel.clone(),
        andere => format!("{andere:?}"),
    }
}

/// Schluessel und Marke der wartenden Eintraege, in Queue-Reihenfolge.
fn belegung_lesen(ausgang: &Ausgang) -> Vec<(String, i64)> {
    ausgang
        .inhalt
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .0
        .iter()
        .map(|eintrag| (schluessel_von(eintrag), eintrag.marke))
        .collect()
}

/// Entnimmt EINEN Eintrag - mit Frist. `Ausgang::entnehmen` wartet ohne
/// Frist; ein Haenger waere kein Rot.
fn eintrag_abholen(ausgang: &Ausgang, frist_ms: u64) -> Option<AusgangEintrag> {
    if warte_auf(frist_ms, || ausgang_laenge(ausgang) > 0) {
        ausgang.entnehmen()
    } else {
        None
    }
}

/// Der Writer meldet den Eintrag als geschrieben (oder nicht).
fn bestaetigen(mut eintrag: AusgangEintrag, geschrieben: bool) {
    if let Some(antwort) = eintrag.geschrieben.take() {
        let _ = antwort.send(geschrieben);
    }
}

/// Schluessel, Marke und Nutzlast (als JSON) eines Snapshot-Eintrags.
fn eintrag_lesen(eintrag: &AusgangEintrag) -> (String, i64, Value) {
    let rahmen = crate::transport::v3::envelope_pruefen(&eintrag.frame).expect("P1-Envelope");
    let wert = serde_json::from_slice(&rahmen.payload).expect("Nutzlast ist JSON");
    (schluessel_von(eintrag), eintrag.marke, wert)
}

fn label_von(snapshot: &Value) -> Option<String> {
    snapshot["mitglieder"][0]["probe_descriptor"]["label"]
        .as_str()
        .map(str::to_owned)
}

/// Entnimmt, bis die Queue `NAK246_RUHE_MS` lang leer bleibt, und meldet jeden
/// Eintrag als geschrieben - der Consumer nach der Freigabe.
fn zustellfolge_abholen(ausgang: &Ausgang) -> Vec<(String, i64, Value)> {
    let mut folge = Vec::new();
    while let Some(eintrag) = eintrag_abholen(ausgang, NAK246_RUHE_MS) {
        folge.push(eintrag_lesen(&eintrag));
        bestaetigen(eintrag, true);
    }
    folge
}

/// Schluessel, Marke und Label je Zustellung - die lesbare Form einer Folge.
fn folge_mit_labels(folge: &[(String, i64, Value)]) -> Vec<(String, i64, Option<String>)> {
    folge
        .iter()
        .map(|(schluessel, marke, wert)| (schluessel.clone(), *marke, label_von(wert)))
        .collect()
}

/// Eine Barriere VOR dem echten Sender (M-24): der naechste Snapshot-Push - auf
/// Wunsch nur einer, dessen Nutzlast eine bestimmte Zeichenkette enthaelt -
/// haelt, bis der Test ihn freigibt, und geht dann UNVERAENDERT an den
/// `V3Sender`. Sie stellt nichts selbst zu und misst nichts - ueber Annahme und
/// Ordnung entscheidet der echte Ausgang dahinter.
struct SperreVorDemSender {
    sender: V3Sender,
    scharf: AtomicBool,
    nur_mit: Mutex<Option<String>>,
    betreten: (Mutex<bool>, Condvar),
    frei: (Mutex<bool>, Condvar),
}

impl SperreVorDemSender {
    fn neu(sender: V3Sender) -> Self {
        Self {
            sender,
            scharf: AtomicBool::new(false),
            nur_mit: Mutex::new(None),
            betreten: (Mutex::new(false), Condvar::new()),
            frei: (Mutex::new(false), Condvar::new()),
        }
    }

    fn scharf_stellen(&self) {
        self.scharf.store(true, Ordering::SeqCst);
    }

    /// Haelt nur den naechsten Push, dessen Nutzlast `inhalt` enthaelt; alle
    /// anderen gehen ungehindert durch.
    fn scharf_stellen_fuer(&self, inhalt: &str) {
        *self.nur_mit.lock().unwrap_or_else(|e| e.into_inner()) = Some(inhalt.to_owned());
        self.scharf_stellen();
    }

    /// Ob dieser Push gehalten wird - und wenn ja, ist die Barriere danach entschaerft.
    fn trifft(&self, payload: &[u8]) -> bool {
        let nur_mit = self.nur_mit.lock().unwrap_or_else(|e| e.into_inner());
        let passt = nur_mit.as_deref().map_or(true, |inhalt| {
            payload
                .windows(inhalt.len().max(1))
                .any(|fenster| fenster == inhalt.as_bytes())
        });
        passt
            && self
                .scharf
                .compare_exchange(true, false, Ordering::SeqCst, Ordering::SeqCst)
                .is_ok()
    }

    fn betreten_abwarten(&self, frist_ms: u64) -> bool {
        let (schloss, signal) = &self.betreten;
        let bis = Instant::now() + Duration::from_millis(frist_ms);
        let mut betreten = schloss.lock().unwrap_or_else(|e| e.into_inner());
        while !*betreten {
            let jetzt = Instant::now();
            if jetzt >= bis {
                return false;
            }
            betreten = signal
                .wait_timeout(betreten, bis - jetzt)
                .unwrap_or_else(|e| e.into_inner())
                .0;
        }
        true
    }

    fn freigeben(&self) {
        let (schloss, signal) = &self.frei;
        *schloss.lock().unwrap_or_else(|e| e.into_inner()) = true;
        signal.notify_all();
    }
}

impl SessionPush for SperreVorDemSender {
    fn snapshot_schreiben(
        &self,
        link_id: &str,
        object_key: &str,
        ordnung: i64,
        payload: &[u8],
    ) -> bool {
        if self.trifft(payload) {
            {
                let (schloss, signal) = &self.betreten;
                *schloss.lock().unwrap_or_else(|e| e.into_inner()) = true;
                signal.notify_all();
            }
            // Notbremse weit hinter jeder Frist des Falls: ein roter Lauf
            // endet, statt zu haengen.
            let (schloss, signal) = &self.frei;
            let bis = Instant::now() + Duration::from_secs(30);
            let mut frei = schloss.lock().unwrap_or_else(|e| e.into_inner());
            while !*frei && Instant::now() < bis {
                frei = signal
                    .wait_timeout(frei, Duration::from_millis(50))
                    .unwrap_or_else(|e| e.into_inner())
                    .0;
            }
        }
        self.sender
            .snapshot_schreiben(link_id, object_key, ordnung, payload)
    }

    fn messframe_schreiben(&self, link_id: &str, instance_id: &str, payload: &[u8]) -> bool {
        self.sender.messframe_schreiben(link_id, instance_id, payload)
    }
}

/// NAK-246 M-26 (R-D9): der Sender vergibt den Queue-Schluessel je Objekt.
///
/// Eine Ruecknahme, ein Vollsnapshot und ein Angebot laufen fuer DENSELBEN Link
/// durch `V3Sender::snapshot_schreiben`; der Consumer entnimmt nichts. Danach
/// liegen drei Eintraege mit drei Schluesseln in der Queue, keiner hat einen
/// anderen ersetzt, und der Writer gibt sie in Einreihreihenfolge ab.
#[test]
fn sender_vergibt_je_objekt_einen_queue_schluessel() {
    let sender = V3Sender::neu();
    let ausgang = Arc::new(Ausgang::neu());
    sender
        .control_ausgaenge
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .insert("link-m26".into(), ausgang.clone());
    let nachrichten = [
        (
            "evidence_invalidate",
            21_i64,
            json!({"type": "evidence_invalidate", "grund": "messpunkt_wechsel"}),
        ),
        ("session_snapshot", 22, json!({"type": "session_snapshot"})),
        ("proposal:m26", 23, json!({"type": "draft_offer"})),
    ];
    let mut schreiber: Vec<std::thread::JoinHandle<bool>> = Vec::new();
    for (index, (schluessel, marke, wert)) in nachrichten.iter().enumerate() {
        let vorher = ausgang_laenge(&ausgang);
        let sender = sender.clone();
        let objekt_schluessel = (*schluessel).to_owned();
        let marke = *marke;
        let payload = serde_json::to_vec(wert).unwrap();
        schreiber.push(std::thread::spawn(move || {
            sender.snapshot_schreiben("link-m26", &objekt_schluessel, marke, &payload)
        }));
        // Eingereiht ist, sobald die Queue waechst - oder, falls der Aufruf
        // den vorigen Eintrag verdraengt hat, sobald dessen Aufrufer mit
        // `false` zurueck ist.
        assert!(
            warte_auf(NAK246_FRIST_MS, || {
                ausgang_laenge(&ausgang) > vorher
                    || (index > 0 && schreiber[index - 1].is_finished())
            }),
            "M-26: der Aufruf fuer {schluessel} hat eingereiht"
        );
    }
    let belegung = belegung_lesen(&ausgang);
    println!("NAK-246 M-26: Belegung nach dem dritten snapshot_schreiben {belegung:?}");
    assert_eq!(
        belegung,
        vec![
            ("evidence_invalidate".to_owned(), 21),
            ("session_snapshot".to_owned(), 22),
            ("proposal:m26".to_owned(), 23),
        ],
        "M-26: drei Eintraege mit drei Schluesseln - der Queue-Schluessel ist der Objektschluessel, keiner ersetzt einen anderen"
    );
    for (schluessel, marke, wert) in &nachrichten {
        let eintrag = eintrag_abholen(&ausgang, NAK246_FRIST_MS).expect("M-26: der Writer entnimmt");
        let (gelesen_schluessel, gelesen_marke, gelesen_wert) = eintrag_lesen(&eintrag);
        assert_eq!(
            (gelesen_schluessel.as_str(), gelesen_marke, &gelesen_wert),
            (*schluessel, *marke, wert),
            "M-26: die Writerreihenfolge ist die Einreihreihenfolge"
        );
        bestaetigen(eintrag, true);
    }
    for aufruf in schreiber {
        assert!(aufruf.join().unwrap(), "M-26: jeder Aufrufer bekommt true");
    }
}

/// NAK-246 M-23 (R-D5; §1.8 schliessender Beweis): zwei Flushes derselben
/// Sitzung, der aeltere hinter dem neueren.
///
/// A erfasst und committet den aelteren Stand und haelt NACH der Freigabe des
/// Flush-Schlosses vor dem Einreihen (zweiter Haken, §5.5 Feinheit 6); B
/// committet und reiht ein, waehrend A haelt; dann wird A freigegeben. Der
/// Consumer entnimmt erst danach: der zuletzt angenommene Zustand am
/// Empfaenger ist der neue.
#[test]
fn aelterer_flush_ueberholt_den_neueren_hinter_dem_echten_sender_nicht() {
    let buehne = Zustellbuehne::aufbauen("m23", true);
    let coordinator = &buehne.coordinator;
    let alt = buehne.descriptor_mit_label("alt-erfasst");
    let neu = buehne.descriptor_mit_label("neu-committed");

    let erfassung = CoordinatorFlushTestHaken::default();
    coordinator.flush_test_haken_setzen(erfassung.clone());
    let flush_a = buehne.descriptor_setzen_starten(alt);
    erfassung.warten_bis_erfasst();
    // A hat den alten Stand erfasst und steht am ersten Haken, unter dem
    // Schloss und vor dem Append. Der zweite Haken wird JETZT gesetzt: A findet
    // ihn nach seinem Commit, hinter `drop(_flush_guard)`.
    let zustellung = CoordinatorFlushTestHaken::default();
    coordinator.flush_test_haken_setzen(zustellung.clone());
    erfassung.freigeben();
    zustellung.warten_bis_erfasst();
    let (ord_a, label_a) = buehne.projektion();
    assert_eq!(
        label_a.as_deref(),
        Some("alt-erfasst"),
        "A hat committet und steht vor dem Einreihen"
    );

    let flush_b = buehne.descriptor_setzen_starten(neu);
    assert!(
        warte_auf(NAK246_FRIST_MS, || ausgang_laenge(&buehne.ausgang) == 1),
        "M-23: der Flush-Lock ist vor der Zustellung frei - B committet und reiht ein, waehrend A vor dem Einreihen steht"
    );

    zustellung.freigeben();
    // A reiht seinen aelteren Stand ein: verworfen (A ist zurueck) oder er
    // verdraengt B (B ist mit false zurueck). Erst danach entnimmt der
    // Consumer - ohne weiteren Zwischenschritt, denn B wartet auf den Writer.
    assert!(
        warte_auf(NAK246_FRIST_MS, || flush_a.is_finished() || flush_b.is_finished()),
        "A hat eingereiht"
    );
    let zustellfolge = zustellfolge_abholen(&buehne.ausgang);
    assert!(flush_a.join().unwrap());
    assert!(flush_b.join().unwrap());
    let (ord_b, label_b) = buehne.projektion();
    assert_eq!(label_b.as_deref(), Some("neu-committed"));
    assert!(ord_b > ord_a, "die Commit-Reihenfolge ist A vor B [{ord_a} < {ord_b}]");
    let folge = folge_mit_labels(&zustellfolge);
    let verworfen = buehne.ausgang.nachzuegler_verworfen.load(Ordering::SeqCst);
    let hochwasser = buehne.ausgang.hochwasser("session_snapshot");
    let schuld = buehne.schuld("session_snapshot");
    println!(
        "NAK-246 M-23: Commit-Ordinal A={ord_a} B={ord_b}; Zustellfolge {folge:?}; nachzuegler_verworfen={verworfen}; Hochwasser session_snapshot={hochwasser:?}; offene Schuld session_snapshot={schuld:?}"
    );

    let letzter = folge.iter().rev().find(|(schluessel, _, _)| schluessel == "session_snapshot");
    assert_eq!(
        letzter.and_then(|(_, _, label)| label.as_deref()),
        Some("neu-committed"),
        "M-23: der zuletzt angenommene Zustand am Empfaenger ist der neue [Zustellfolge {folge:?}]"
    );
    assert!(
        folge.windows(2).all(|paar| paar[0].1 <= paar[1].1),
        "M-23: die Marken der Zustellfolge sind monoton [{folge:?}]"
    );
    assert_eq!(folge.len(), 1, "M-23: der aeltere fehlt [{folge:?}]");
    assert_eq!(
        letzter.map(|(_, marke, _)| *marke),
        Some(ord_b),
        "M-23: die Marke ist das Store-Ordinal des Commits von B - keine neue Zahl"
    );
    assert_eq!(verworfen, 1, "M-23: A ist als Nachzuegler verworfen und gezaehlt");
    assert_eq!(hochwasser, Some(ord_b), "M-23: das Hochwasser ist die Marke von B");
    assert!(
        schuld.is_empty(),
        "M-23: B ist geschrieben und bis zu seinem Ordinal kompaktiert; A hinterlaesst keine Schuld [{schuld:?}]"
    );
}

/// NAK-246 M-27 (R-D9; §1.11 schliessender Beweis): die echte Writerqueue ist
/// angehalten, bevor der Writer entnimmt.
///
/// Der Coordinator reiht ueber den Sender eine Ruecknahme und danach einen
/// Vollsnapshot ein. Vor der Freigabe liegen zwei Eintraege mit zwei
/// Schluesseln in der Queue; nach der Freigabe kommen beide in
/// Einreihreihenfolge an, und die Schuld der Ruecknahme ist kompaktiert. Die
/// Wirkung im Modell (evidenzRuecknahmen, Messachse, Befunde) misst B13.
#[test]
fn ruecknahme_und_vollsnapshot_liegen_zugleich_in_der_angehaltenen_queue() {
    let buehne = Zustellbuehne::aufbauen("m27", true);
    let coordinator = buehne.coordinator.clone();
    // Ein Messpunktwechsel nimmt die ganze Sitzung zurueck (M-55): committet und
    // ueber den Sender eingereiht. Der Aufruf wartet auf den Writer.
    let ruecknahme = std::thread::spawn(move || {
        coordinator.invalidierung_wegen_messpunkt_fuer_link("main", "insert", "post")
    });
    assert!(
        warte_auf(NAK246_FRIST_MS, || ausgang_laenge(&buehne.ausgang) == 1),
        "M-27: die Ruecknahme liegt in der angehaltenen Queue"
    );
    let vollsnapshot =
        buehne.descriptor_setzen_starten(buehne.descriptor_mit_label("voll-nach-ruecknahme"));
    // Eingereiht ist, sobald die Queue waechst - oder, falls der Vollsnapshot
    // die Ruecknahme verdraengt hat, sobald diese mit `false` zurueck ist.
    assert!(
        warte_auf(NAK246_FRIST_MS, || {
            ausgang_laenge(&buehne.ausgang) == 2 || ruecknahme.is_finished()
        }),
        "M-27: der Vollsnapshot hat eingereiht"
    );
    let belegung = belegung_lesen(&buehne.ausgang);
    println!("NAK-246 M-27: Belegung vor der Freigabe {belegung:?}");
    let schluessel: Vec<&str> = belegung.iter().map(|(k, _)| k.as_str()).collect();
    assert_eq!(
        schluessel,
        vec!["evidence_invalidate", "session_snapshot"],
        "M-27: vor der Freigabe liegen zwei Eintraege mit zwei Schluesseln - der Vollsnapshot hat die Ruecknahme nicht verdraengt [{belegung:?}]"
    );
    assert!(
        belegung[0].1 < belegung[1].1,
        "die Store-Ordinale folgen der Einreihreihenfolge [{belegung:?}]"
    );

    // Freigabe: der Writer entnimmt.
    let zustellfolge = zustellfolge_abholen(&buehne.ausgang);
    ruecknahme.join().unwrap();
    assert!(vollsnapshot.join().unwrap());
    let typen: Vec<String> = zustellfolge
        .iter()
        .map(|(k, _, wert)| format!("{k}:{}", wert["type"].as_str().unwrap_or("?")))
        .collect();
    let schuld = buehne.schuld("evidence_invalidate");
    println!("NAK-246 M-27: Zustellfolge {typen:?}; offene Schuld evidence_invalidate={schuld:?}");
    assert_eq!(
        typen,
        vec![
            "evidence_invalidate:evidence_invalidate".to_owned(),
            "session_snapshot:session_snapshot".to_owned(),
        ],
        "M-27: der Empfaenger erhaelt beide in Einreihreihenfolge"
    );
    assert!(
        schuld.is_empty(),
        "M-27: die Schuld der Ruecknahme ist kompaktiert (snapshot_schuld_kompaktieren gerufen), nicht stehengeblieben [{schuld:?}]"
    );
}

/// NAK-246 M-24 (R-D5; §5.5 Feinheit 1, zweiter Fall): ein Subscribe-Snapshot,
/// der wirklich aelter ist als ein bereits eingereihter Flush-Snapshot, wird
/// verworfen; seine Schuld ist durch den neueren gedeckt.
///
/// Der Resubscribe bildet seinen absoluten Snapshot aus der Projektion (Marke =
/// ihr Ordinal) und haelt vor dem echten Sender - an einer Barriere, die
/// unveraendert weiterreicht. Waehrend er haelt, committet ein Flush den
/// neueren Stand und reiht ihn ein. Dann reiht der Resubscribe ein.
#[test]
fn subscribe_snapshot_ueberholt_keinen_neueren_flush() {
    let buehne = Zustellbuehne::aufbauen("m24-subscribe", true);
    let vorher = buehne.descriptor_setzen_starten(buehne.descriptor_mit_label("vorher"));
    let eintrag = eintrag_abholen(&buehne.ausgang, NAK246_FRIST_MS)
        .expect("der Flush des Stands 'vorher' kommt an");
    bestaetigen(eintrag, true);
    assert!(vorher.join().unwrap());
    let (ord_vorher, label_vorher) = buehne.projektion();
    assert_eq!(label_vorher.as_deref(), Some("vorher"));

    let sperre = Arc::new(SperreVorDemSender::neu(buehne.sender.clone()));
    buehne.coordinator.session_push_setzen(sperre.clone());
    sperre.scharf_stellen();
    let resubscribe = buehne.resubscribe_starten();
    assert!(
        sperre.betreten_abwarten(NAK246_FRIST_MS),
        "M-24: der Resubscribe hat seinen Snapshot aus der Projektion gebildet und steht vor dem Sender"
    );

    let flush = buehne.descriptor_setzen_starten(buehne.descriptor_mit_label("neu"));
    assert!(
        warte_auf(NAK246_FRIST_MS, || ausgang_laenge(&buehne.ausgang) == 1),
        "M-24: der neuere Flush-Snapshot liegt in der Queue"
    );

    sperre.freigeben();
    // Erst reiht der Resubscribe ein, dann entnimmt der Consumer - ohne
    // weiteren Zwischenschritt, denn der Flush wartet auf den Writer.
    assert!(
        warte_auf(NAK246_FRIST_MS, || resubscribe.is_finished() || flush.is_finished()),
        "der Resubscribe hat eingereiht"
    );
    let zustellfolge = zustellfolge_abholen(&buehne.ausgang);
    assert!(resubscribe.join().unwrap());
    assert!(flush.join().unwrap());
    let (ord_neu, label_neu) = buehne.projektion();
    assert_eq!(label_neu.as_deref(), Some("neu"));
    assert!(ord_neu > ord_vorher, "{ord_neu} > {ord_vorher}");
    let folge = folge_mit_labels(&zustellfolge);
    let verworfen = buehne.ausgang.nachzuegler_verworfen.load(Ordering::SeqCst);
    let schuld = buehne.schuld("session_snapshot");
    println!(
        "NAK-246 M-24 Subscribe: Projektion vorher={ord_vorher}, Flush neu={ord_neu}; Zustellfolge {folge:?}; nachzuegler_verworfen={verworfen}; offene Schuld session_snapshot={schuld:?}"
    );
    let letzter = folge.iter().rev().find(|(schluessel, _, _)| schluessel == "session_snapshot");
    assert_eq!(
        letzter.and_then(|(_, _, label)| label.as_deref()),
        Some("neu"),
        "M-24: der aeltere Subscribe-Snapshot (Marke {ord_vorher}) ueberholt den bereits eingereihten neueren Flush (Marke {ord_neu}) nicht [{folge:?}]"
    );
    assert_eq!(folge.len(), 1, "M-24: der aeltere Subscribe-Snapshot ist nicht geschrieben [{folge:?}]");
    assert_eq!(verworfen, 1, "M-24: er ist als Nachzuegler verworfen und gezaehlt");
    assert!(
        schuld.is_empty(),
        "M-24: seine Schuld ist durch den geschriebenen neueren Snapshot gedeckt [{schuld:?}]"
    );
}

/// NAK-246 M-24 (R-D5, R-M2-1; §5.5 Feinheit 1, dritter Fall): ein absoluter
/// Resubscribe-Snapshot bei degradiertem Store ist der aktuelle Livestand und
/// wird nach einem zugestellten persistenten Snapshot nicht verworfen.
///
/// Ein Flush liefert seinen persistenten Snapshot, der Ausgang schreibt ihn -
/// das Hochwasser ist sein Ordinal. Waehrend dessen Schuld kompaktiert wird,
/// haelt ein zweiter Schreiber die Datenbank (`BEGIN IMMEDIATE`); der
/// Store-Writer scheitert nach `BUSY_TIMEOUT_MS` an der Kompaktierung und
/// degradiert (`store/writer.rs`). Kein weiterer Commit. Der Resubscribe findet
/// keine Projektion und liefert den Livestand: er traegt das Hochwasser, kommt
/// an, und `nachzuegler_verworfen` bleibt 0.
#[test]
fn live_snapshot_bei_projektion_none_wird_nach_persistentem_snapshot_nicht_verworfen() {
    let buehne = Zustellbuehne::aufbauen("m24-live", true);
    let writer = buehne.writer.as_ref().expect("Fall mit Store");
    let db = buehne.ordner.as_ref().expect("Fall mit Store").db();
    let flush = buehne.descriptor_setzen_starten(buehne.descriptor_mit_label("persistent"));
    let persistent = eintrag_abholen(&buehne.ausgang, NAK246_FRIST_MS)
        .expect("M-24: der persistente Snapshot kommt an");
    let (schluessel, marke_persistent, wert) = eintrag_lesen(&persistent);
    let (ord, _) = buehne.projektion();
    assert_eq!(
        (schluessel.as_str(), label_von(&wert).as_deref()),
        ("session_snapshot", Some("persistent"))
    );
    assert_eq!(
        marke_persistent, ord,
        "die Marke des persistenten Snapshots ist das Ordinal seines Commits"
    );

    let sperre = rusqlite::Connection::open(&db).unwrap();
    sperre
        .busy_timeout(Duration::from_millis(BUSY_TIMEOUT_MS))
        .unwrap();
    sperre.execute_batch("BEGIN IMMEDIATE").unwrap();
    bestaetigen(persistent, true);
    assert!(flush.join().unwrap());
    assert!(
        writer.ist_degradiert(),
        "M-24: der Store ist degradiert [{:?}]",
        writer.handle().sicht()
    );
    sperre.execute_batch("ROLLBACK").unwrap();
    drop(sperre);
    let letzter_commit: i64 = rusqlite::Connection::open(&db)
        .unwrap()
        .query_row("SELECT MAX(event_ord) FROM event_log", [], |zeile| zeile.get(0))
        .unwrap();
    assert_eq!(letzter_commit, marke_persistent, "kein weiterer Commit");
    assert_eq!(
        buehne.ausgang.hochwasser("session_snapshot"),
        Some(marke_persistent),
        "das Hochwasser ist die Marke des geschriebenen persistenten Snapshots"
    );

    let resubscribe = buehne.resubscribe_starten();
    let live = eintrag_abholen(&buehne.ausgang, NAK246_FRIST_MS);
    let verworfen = buehne.ausgang.nachzuegler_verworfen.load(Ordering::SeqCst);
    let live = live.unwrap_or_else(|| {
        panic!(
            "M-24: der Livestand bei Projektion None kommt an - er ist nie ein Nachzuegler [nachzuegler_verworfen={verworfen}]"
        )
    });
    let (schluessel, marke_live, wert) = eintrag_lesen(&live);
    bestaetigen(live, true);
    assert!(resubscribe.join().unwrap());
    println!(
        "NAK-246 M-24 Livestand: persistente Marke {marke_persistent}, Marke des Livestands {marke_live}, store_degraded {}, nachzuegler_verworfen {verworfen}",
        wert["store_degraded"]
    );
    assert_eq!(schluessel, "session_snapshot");
    assert_eq!(
        wert["store_degraded"], true,
        "es ist der Livestand des degradierten Stores"
    );
    assert_eq!(
        marke_live, marke_persistent,
        "M-24: ohne weiteren Commit traegt der Livestand die Marke des zugestellten persistenten Snapshots"
    );
    assert_eq!(verworfen, 0, "M-24: nachzuegler_verworfen bleibt 0");
}

/// NAK-246 M-24 (R-D5; §5.5 Feinheit 1, erster Fall): ohne Store folgt die
/// Marke der Erfassung.
///
/// Flush A erfasst den alten Stand, zieht dabei seine Marke und haelt am ersten
/// Haken; N setzt den neuen Stand und wartet am Flush-Schloss, das A haelt; ein
/// Resubscribe erfasst den neuen Livestand, zieht die groessere Marke und wird
/// geschrieben. Danach stellt A seinen frueher erfassten Stand zu - und erst
/// wenn dessen Einreihen entschieden ist, reiht N ein: eine Barriere vor dem
/// echten Sender haelt nur die Nutzlast mit dem neuen Stand. Ohne sie verdeckte
/// N einen durchgelassenen alten Stand in der Queue, bevor der Consumer ihn
/// sieht. Zusage: nach dem neuen Livestand wird kein alter Stand zugestellt.
#[test]
fn ohne_store_folgt_die_marke_der_erfassung() {
    let buehne = Zustellbuehne::aufbauen("m24-ohne-store", false);
    let alt = buehne.descriptor_mit_label("alt-erfasst");
    let neu = buehne.descriptor_mit_label("neu-live");
    let sperre = Arc::new(SperreVorDemSender::neu(buehne.sender.clone()));
    buehne.coordinator.session_push_setzen(sperre.clone());

    let erfassung = CoordinatorFlushTestHaken::default();
    buehne.coordinator.flush_test_haken_setzen(erfassung.clone());
    let flush_a = buehne.descriptor_setzen_starten(alt);
    erfassung.warten_bis_erfasst();
    // N setzt den neuen Stand unter dem Standlock und wartet dann am
    // Flush-Schloss, das A haelt.
    let flush_n = buehne.descriptor_setzen_starten(neu);
    assert!(
        warte_auf(NAK246_FRIST_MS, || buehne.live_label().as_deref() == Some("neu-live")),
        "der neue Stand ist gesetzt"
    );
    // Der Resubscribe erfasst den neuen Livestand - ohne das Flush-Schloss.
    let resubscribe = buehne.resubscribe_starten();
    let live = eintrag_abholen(&buehne.ausgang, NAK246_FRIST_MS)
        .expect("der Livestand des Resubscribe kommt an, waehrend A haelt");
    let erster = eintrag_lesen(&live);
    bestaetigen(live, true);
    assert!(resubscribe.join().unwrap());
    assert_eq!(label_von(&erster.2).as_deref(), Some("neu-live"));

    // Ab hier haelt die Barriere den Push mit dem neuen Stand (N); A geht durch.
    sperre.scharf_stellen_fuer("neu-live");
    erfassung.freigeben();
    // A hat eingereiht: verworfen (A ist zurueck) oder aufgenommen (sein
    // Eintrag liegt in der Queue).
    assert!(
        warte_auf(NAK246_FRIST_MS, || {
            flush_a.is_finished() || ausgang_laenge(&buehne.ausgang) > 0
        }),
        "A hat eingereiht"
    );
    assert!(
        sperre.betreten_abwarten(NAK246_FRIST_MS),
        "N steht mit dem neuen Stand vor dem Sender"
    );
    let mut zustellfolge = vec![erster];
    zustellfolge.extend(zustellfolge_abholen(&buehne.ausgang));
    sperre.freigeben();
    zustellfolge.extend(zustellfolge_abholen(&buehne.ausgang));
    assert!(flush_a.join().unwrap());
    assert!(flush_n.join().unwrap());
    let folge = folge_mit_labels(&zustellfolge);
    let verworfen = buehne.ausgang.nachzuegler_verworfen.load(Ordering::SeqCst);
    println!("NAK-246 M-24 ohne Store: Zustellfolge {folge:?}; nachzuegler_verworfen={verworfen}");
    assert!(
        !folge
            .iter()
            .skip(1)
            .any(|(_, _, label)| label.as_deref() == Some("alt-erfasst")),
        "M-24 (kein Store): nach dem geschriebenen neuen Livestand wird kein frueher erfasster alter Stand zugestellt [{folge:?}]"
    );
    assert!(
        folge.windows(2).all(|paar| paar[0].1 <= paar[1].1),
        "M-24: die Marken der Zustellfolge sind monoton [{folge:?}]"
    );
    assert_eq!(verworfen, 1, "M-24: A ist als Nachzuegler verworfen");
}
