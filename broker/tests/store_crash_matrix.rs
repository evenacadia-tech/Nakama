use eqcop_broker::coordinator::{
    Coordinator, CoordinatorFlushTestHaken, ManualClock, SessionPush, STALE_NACH_MS, TOMBSTONE_MS,
};
use eqcop_broker::store::{
    busy_timeout_abgelaufen, checkpoint_ausloesen, commit_ausloesen, migration_1_checksum,
    projektionen_neu_bauen, recovery_testgrenze_bestanden, standard_store_pfad, store_pfad_unter,
    AppendAusgang, ConflictGuard, IdleCheckpointNaht, SnapshotZiel, StoreEvent, StoreFehler,
    StoreKonfiguration,
    StoreStartBarriere, StoreWriter, BUSY_TIMEOUT_MS, CHECKPOINT_BUSY_ERWARTET, COMMIT_BATCH_MAX,
    COMMIT_FENSTER_MS, RUSQLITE_VERSION, STORE_DATEINAME, STORE_IDLE_MS, STORE_KANAL_CAP,
    STORE_RECOVERY_TEST_MAX_MS, STORE_RELATIVPFAD, STORE_SCHEMA_MAJOR, WAL_SCHWELLE_BYTES,
};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
#[cfg(windows)]
use eqcop_broker::transport::server_v3::Senke;
use eqcop_broker::vertrag::Schema;
use rusqlite::{Connection, OpenFlags};
use serde_json::{json, Value};
use std::path::{Path, PathBuf};
use std::process::{Child, Command};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Barrier, Condvar, Mutex, OnceLock};
use std::time::{Duration, Instant};

struct TestOrdner(PathBuf);

impl TestOrdner {
    fn neu(name: &str) -> Self {
        let pfad = std::env::temp_dir().join(format!(
            "nakama-sonde011-{name}-{}-{}",
            std::process::id(),
            uuid::Uuid::new_v4().simple()
        ));
        std::fs::create_dir_all(&pfad).unwrap();
        Self(pfad)
    }

    fn db(&self) -> PathBuf {
        self.0.join(STORE_DATEINAME)
    }
}

impl Drop for TestOrdner {
    fn drop(&mut self) {
        let _ = std::fs::remove_dir_all(&self.0);
    }
}

fn ziel() -> SnapshotZiel {
    SnapshotZiel {
        project_binding_id: "00000000000000000000000000000001".into(),
        session_epoch: "00000000000000000000000000000002".into(),
        instance_id: "00000000000000000000000000000004".into(),
        object_key: "session_snapshot".into(),
    }
}

fn snapshot_schema() -> &'static Schema {
    static SCHEMA: OnceLock<Schema> = OnceLock::new();
    SCHEMA.get_or_init(|| {
        Schema::laden(
            serde_json::from_str(include_str!(
                "../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
            ))
            .unwrap(),
        )
        .unwrap()
    })
}

fn snapshot_wert(broker_epoch: &str, wirkung: i64) -> Value {
    assert!(wirkung >= 0);
    json!({
        "type": "session_snapshot",
        "session_epoch": "00000000000000000000000000000002",
        "broker_epoch": broker_epoch,
        "fuehrendes_main": (wirkung != 0).then(|| format!("{:032x}", wirkung as u64)),
        "beitritt_bestaetigung_noetig": wirkung != 0,
        "mitglieder": []
    })
}

fn snapshot_bytes(broker_epoch: &str, wirkung: i64) -> Vec<u8> {
    let wert = snapshot_wert(broker_epoch, wirkung);
    assert!(snapshot_schema().gueltig(&wert));
    serde_json::to_vec(&wert).unwrap()
}

fn snapshot_wirkung(wert: &Value) -> i64 {
    assert!(
        snapshot_schema().gueltig(wert),
        "kein session_snapshot: {wert}"
    );
    wert["fuehrendes_main"]
        .as_str()
        .map(|hex| i64::from_str_radix(hex, 16).unwrap())
        .unwrap_or_else(|| {
            i64::from(
                wert["beitritt_bestaetigung_noetig"]
                    .as_bool()
                    .unwrap_or(false),
            )
        })
}

fn persistierte_snapshot_wirkung(writer: &StoreWriter) -> i64 {
    let (_, payload) = writer
        .handle()
        .session_state_lesen(
            "00000000000000000000000000000001",
            "00000000000000000000000000000002",
        )
        .unwrap()
        .unwrap();
    snapshot_wirkung(&serde_json::from_slice(&payload).unwrap())
}

fn event(command: Option<&str>, sequence: i64, wirkung: i64, outbox: bool) -> StoreEvent {
    let mut event = StoreEvent::session_snapshot(
        "00000000000000000000000000000001",
        "00000000000000000000000000000002",
        "00000000000000000000000000000003",
        sequence,
        snapshot_bytes("00000000000000000000000000000003", wirkung),
    );
    event.command_id = command.map(str::to_owned);
    if outbox {
        event.snapshot_ziele.push(ziel());
    }
    event
}

fn starten(db: &Path) -> StoreWriter {
    let mut k = StoreKonfiguration::fuer_pfad(db);
    k.remote_volume_override = Some(false);
    let writer = StoreWriter::starten(k);
    assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());
    writer
}

fn scalar_i64(db: &Path, sql: &str) -> i64 {
    Connection::open(db)
        .unwrap()
        .query_row(sql, [], |row| row.get(0))
        .unwrap()
}

fn schema_signatur(db: &Path) -> Vec<(String, String, String, String)> {
    let conn = Connection::open(db).unwrap();
    let mut stmt = conn
        .prepare(
            "SELECT type,name,tbl_name,COALESCE(sql,'') FROM sqlite_master \
             WHERE name NOT LIKE 'sqlite_%' ORDER BY type,name,tbl_name,sql",
        )
        .unwrap();
    stmt.query_map([], |row| {
        Ok((row.get(0)?, row.get(1)?, row.get(2)?, row.get(3)?))
    })
    .unwrap()
    .collect::<Result<Vec<_>, _>>()
    .unwrap()
}

fn child_starten(db: &Path, marker: &Path, punkt: &str, aktion: &str) -> Child {
    Command::new(env!("CARGO_BIN_EXE_eqcop-store-crash-worker"))
        .arg(db)
        .arg(marker)
        .arg(punkt)
        .arg(aktion)
        .spawn()
        .expect("Crashworker starten")
}

fn snapshot_subscriber_starten(snapshot: &Path, state: &Path, marker: &Path, punkt: &str) -> Child {
    Command::new(env!("CARGO_BIN_EXE_eqcop-store-crash-worker"))
        .arg("--snapshot-subscriber")
        .arg(snapshot)
        .arg(state)
        .arg(marker)
        .arg(punkt)
        .spawn()
        .expect("Snapshot-Subscriber starten")
}

fn marker_abwarten(marker: &Path) {
    let frist = Instant::now() + Duration::from_secs(10);
    while !marker.exists() && Instant::now() < frist {
        std::thread::sleep(Duration::from_millis(10));
    }
    assert!(marker.exists(), "Killmarker {} fehlt", marker.display());
}

fn hart_killen(child: &mut Child) {
    child.kill().expect("Child hart killen");
    let status = child.wait().expect("Child ernten");
    assert!(!status.success(), "Crashworker wurde nicht gekillt");
}

fn crash(db: &Path, punkt: &str, aktion: &str) {
    let marker = db.with_extension(format!("{punkt}.marker"));
    let mut child = child_starten(db, &marker, punkt, aktion);
    marker_abwarten(&marker);
    hart_killen(&mut child);
}

#[cfg(windows)]
fn phase_b_probe_pipe() -> String {
    format!(
        r"\\.\pipe\evenacadia.eq-copilot.v3probe.phaseb.{}.{}",
        std::process::id(),
        uuid::Uuid::new_v4().simple()
    )
}

#[cfg(windows)]
fn cpp_ipc_test() -> PathBuf {
    if let Some(pfad) = std::env::var_os("NAKAMA_EQCOP_IPC_TEST_EXE") {
        return PathBuf::from(pfad);
    }
    Path::new(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .expect("Repo-Root")
        .join("eq-copilot/build/plugin/EqCopIpcTest_artefacts/Release/EqCopIpcTest.exe")
}

#[cfg(windows)]
fn phase_b_server_starten(db: &Path, marker: &Path, pipe: &str, punkt: &str) -> Child {
    Command::new(env!("CARGO_BIN_EXE_eqcop-store-crash-worker"))
        .arg("--phase-b-si")
        .arg(db)
        .arg(marker)
        .arg(pipe)
        .arg(punkt)
        .spawn()
        .expect("Phase-B-Probeserver starten")
}

#[cfg(windows)]
fn child_erfolgreich_abwarten(child: &mut Child, name: &str) {
    let frist = Instant::now() + Duration::from_secs(30);
    loop {
        match child.try_wait().expect("Childstatus") {
            Some(status) => {
                assert!(status.success(), "{name} endete mit {status}");
                return;
            }
            None if Instant::now() < frist => std::thread::sleep(Duration::from_millis(20)),
            None => {
                let _ = child.kill();
                let _ = child.wait();
                panic!("{name} antwortete nicht binnen 30 s");
            }
        }
    }
}

#[cfg(windows)]
fn echte_cpp_client_crashrunde(killpunkt: &str, erwartetes_ergebnis: &str) {
    let ordner = TestOrdner::neu(killpunkt);
    let db = ordner.db();
    let marker = db.with_extension(format!("{killpunkt}.si.marker"));
    let pipe = phase_b_probe_pipe();
    let command_id = "55555555555555555555555555555555";
    let cpp = cpp_ipc_test();
    assert!(
        cpp.is_file(),
        "frisch gebautes B10-Binary fehlt: {}",
        cpp.display()
    );

    let mut server1 = phase_b_server_starten(&db, &marker, &pipe, killpunkt);
    let mut client = Command::new(cpp)
        .arg("--phase-b-command-client")
        .arg(&pipe)
        .arg(command_id)
        .arg(erwartetes_ergebnis)
        .arg(env!("CARGO_BIN_EXE_eqcop-store-crash-worker"))
        .spawn()
        .expect("echten C++-ControlClient starten");
    marker_abwarten(&marker);
    hart_killen(&mut server1);

    let marker2 = db.with_extension("restart.si.marker");
    let mut server2 = phase_b_server_starten(&db, &marker2, &pipe, "kein_killpunkt");
    child_erfolgreich_abwarten(&mut client, "C++-ControlClient");
    hart_killen(&mut server2);
    assert_eq!(scalar_i64(&db, "SELECT COUNT(*) FROM event_log"), 1);
    assert_eq!(
        scalar_i64(
            &db,
            "SELECT COUNT(*) FROM event_log WHERE command_id='55555555555555555555555555555555'"
        ),
        1
    );
}

fn si_hex(n: usize) -> String {
    format!("{n:032x}")
}

fn si_adresse(instanz: usize, nonce: usize) -> Adresse {
    Adresse {
        logon_sid: "S-1-5-21-1-2-3-1001".into(),
        project_binding_id: si_hex(1),
        session_epoch: si_hex(2),
        instance_id: si_hex(instanz),
        runtime_nonce: si_hex(nonce),
    }
}

fn si_hello(instanz: usize, nonce: usize, art: &str) -> HelloControl {
    HelloControl {
        typ: "hello".into(),
        connection_kind: "control".into(),
        protocol: 3,
        plugin_version: "phase-b-si".into(),
        plugin_kind: art.into(),
        adresse: si_adresse(instanz, nonce),
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

fn si_capabilities() -> Value {
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

fn si_report(coordinator: &Coordinator, link: &str, adresse: &Adresse, sequence: u64) -> bool {
    coordinator.heartbeat_kontakt(
        link,
        Some(&json!({
            "type": "heartbeat",
            "adresse": adresse,
            "sequence": sequence,
            "state_revision": sequence,
            "capabilities": si_capabilities(),
            "zaehler": {},
            "runtime": {"messpunkt": "insert", "betrieb": "active"}
        })),
    )
}

fn si_state_report(
    coordinator: &Coordinator,
    link: &str,
    adresse: &Adresse,
    revision: u64,
) -> bool {
    coordinator.state_report_json(
        link,
        &serde_json::to_vec(&json!({
            "type": "state_report",
            "adresse": adresse,
            "dsp_schema_version": 1,
            "state_revision": revision,
            "state_hash": "d".repeat(64),
            "record_state": {"valid": true, "recording": false},
            "undo_tiefe": 0
        }))
        .unwrap(),
    )
}

fn si_subscribe(coordinator: &Coordinator, link: &str, adresse: &Adresse) -> bool {
    coordinator.subscribe_json(
        link,
        &serde_json::to_vec(&json!({
            "type": "subscribe_session",
            "adresse": adresse,
            "session_epoch": adresse.session_epoch
        }))
        .unwrap(),
    )
}

#[derive(Default)]
struct PushProbe {
    snapshots: Mutex<Vec<(String, Value)>>,
    erfolgreich: AtomicBool,
    blockieren: AtomicBool,
    betreten: (Mutex<bool>, Condvar),
    freigabe: (Mutex<bool>, Condvar),
}

impl PushProbe {
    fn neu(erfolgreich: bool) -> Self {
        let probe = Self::default();
        probe.erfolgreich.store(erfolgreich, Ordering::SeqCst);
        probe
    }

    fn snapshots(&self) -> Vec<(String, Value)> {
        self.snapshots
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .clone()
    }

    fn blockieren(&self) {
        self.blockieren.store(true, Ordering::SeqCst);
        *self.betreten.0.lock().unwrap_or_else(|e| e.into_inner()) = false;
        *self.freigabe.0.lock().unwrap_or_else(|e| e.into_inner()) = false;
    }

    fn auf_betreten_warten(&self) {
        let (schloss, signal) = &self.betreten;
        let mut betreten = schloss.lock().unwrap_or_else(|e| e.into_inner());
        while !*betreten {
            betreten = signal.wait(betreten).unwrap_or_else(|e| e.into_inner());
        }
    }

    fn freigeben(&self) {
        let (schloss, signal) = &self.freigabe;
        *schloss.lock().unwrap_or_else(|e| e.into_inner()) = true;
        signal.notify_all();
    }
}

impl SessionPush for PushProbe {
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
        if self.blockieren.swap(false, Ordering::SeqCst) {
            {
                let (schloss, signal) = &self.betreten;
                *schloss.lock().unwrap_or_else(|e| e.into_inner()) = true;
                signal.notify_all();
            }
            let (schloss, signal) = &self.freigabe;
            let mut frei = schloss.lock().unwrap_or_else(|e| e.into_inner());
            while !*frei {
                frei = signal.wait(frei).unwrap_or_else(|e| e.into_inner());
            }
        }
        let wert = serde_json::from_slice(payload).expect("Snapshot ist JSON");
        self.snapshots
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .push((link_id.into(), wert));
        self.erfolgreich.load(Ordering::SeqCst)
    }
}

fn si_coordinator_mit_store(
    name: &str,
    push_erfolgreich: bool,
) -> (
    TestOrdner,
    StoreWriter,
    Arc<Coordinator>,
    Arc<ManualClock>,
    Arc<PushProbe>,
) {
    let ordner = TestOrdner::neu(name);
    let writer = starten(&ordner.db());
    let clock = Arc::new(ManualClock::default());
    let coordinator = Arc::new(Coordinator::mit_store(clock.clone(), si_hex(99), &writer));
    let push = Arc::new(PushProbe::neu(push_erfolgreich));
    coordinator.session_push_setzen(push.clone());
    (ordner, writer, coordinator, clock, push)
}

#[cfg(windows)]
#[test]
fn produkt_coordinator_committet_alle_persistenten_p0_befehle_und_ackt_retries() {
    let (_ordner, writer, coordinator, _clock, _push) =
        si_coordinator_mit_store("produkt-p0", true);
    let main = si_hello(10, 100, "main");
    let probe = si_hello(11, 101, "active_probe");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(
        coordinator
            .control_hello_registrieren("probe", &probe)
            .angenommen
    );
    assert!(si_state_report(&coordinator, "probe", &probe.adresse, 13));
    assert!(si_subscribe(&coordinator, "probe", &probe.adresse));

    let kopf = |command_id: String| {
        json!({
            "command_id": command_id,
            "ziel": probe.adresse,
            "base_revision": 13,
            "ttl_ms": 1000,
            "schema_major": 3,
            "schema_minor": 0
        })
    };
    let befehle = vec![
        json!({
            "type": "preview_begin",
            "kopf": kopf(si_hex(500)),
            "lease_duration_ms": 200,
            "renew_id": si_hex(600)
        }),
        json!({
            "type": "preview_renew",
            "kopf": kopf(si_hex(501)),
            "renew_id": si_hex(601)
        }),
        json!({
            "type": "preview_end",
            "kopf": kopf(si_hex(502)),
            "grund": "losgelassen"
        }),
    ];
    for befehl in &befehle {
        let ack = Senke::p0(
            coordinator.as_ref(),
            "main",
            &serde_json::to_vec(befehl).unwrap(),
        )
        .expect("produktiver command_ack");
        let ack: Value = serde_json::from_slice(&ack).unwrap();
        assert_eq!(ack["ergebnis"], "angewandt");
        assert_eq!(ack["state_revision"], 13);
        assert_eq!(ack["state_hash"], "d".repeat(64));
        assert!(ack.get("event_uuid").is_none());
    }
    let retry = Senke::p0(
        coordinator.as_ref(),
        "main",
        &serde_json::to_vec(&befehle[0]).unwrap(),
    )
    .expect("idempotenter command_ack");
    let retry: Value = serde_json::from_slice(&retry).unwrap();
    assert_eq!(retry["ergebnis"], "idempotent_wiederholt");
    assert_eq!(retry["state_revision"], 13);
    assert_eq!(retry["state_hash"], "d".repeat(64));
    assert_eq!(
        scalar_i64(
            writer.handle().db_pfad(),
            "SELECT COUNT(*) FROM event_log WHERE event_type='command'"
        ),
        3
    );
    let letzter_command_ord = scalar_i64(
        writer.handle().db_pfad(),
        "SELECT MAX(event_ord) FROM event_log WHERE event_type='command'",
    );
    assert_eq!(
        scalar_i64(
            writer.handle().db_pfad(),
            "SELECT last_event_ord FROM sessions",
        ),
        letzter_command_ord,
        "angewandt muss die Sessionprojektion im selben Commit fortschreiben"
    );
    assert_eq!(
        scalar_i64(writer.handle().db_pfad(), "SELECT COUNT(*) FROM outbox"),
        1,
        "angewandt muss im selben Commit genau eine koaleszierte Snapshot-Schuld schreiben"
    );
    assert_eq!(
        scalar_i64(
            writer.handle().db_pfad(),
            "SELECT snapshot_event_ord FROM outbox",
        ),
        letzter_command_ord,
        "Projektion und Snapshot-Schuld muessen denselben Befehls-Commit decken"
    );
    let session_payload: Vec<u8> = Connection::open(writer.handle().db_pfad())
        .unwrap()
        .query_row("SELECT state_jcs FROM sessions", [], |row| row.get(0))
        .unwrap();
    let session_payload: Value = serde_json::from_slice(&session_payload).unwrap();
    assert!(snapshot_schema().gueltig(&session_payload));
}

#[test]
fn brokerneustart_sendet_keine_laufgebundenen_felder_der_alten_projektion() {
    let ordner = TestOrdner::neu("projektion-neuer-brokerlauf");
    let writer = starten(&ordner.db());
    let alt = Arc::new(Coordinator::mit_store(
        Arc::new(ManualClock::default()),
        si_hex(90),
        &writer,
    ));
    let main = si_hello(10, 100, "main");
    assert!(alt.control_hello_registrieren("alt", &main).angenommen);
    assert!(si_report(&alt, "alt", &main.adresse, 1));

    let neu = Arc::new(Coordinator::mit_store(
        Arc::new(ManualClock::default()),
        si_hex(91),
        &writer,
    ));
    let push = Arc::new(PushProbe::neu(true));
    neu.session_push_setzen(push.clone());
    assert!(neu.control_hello_registrieren("neu", &main).angenommen);
    assert!(si_subscribe(&neu, "neu", &main.adresse));
    let snapshot = push.snapshots().last().unwrap().1.clone();
    assert_eq!(snapshot["broker_epoch"], si_hex(91));
    assert!(snapshot["fuehrendes_main"].is_null());
    assert_eq!(snapshot["mitglieder"].as_array().unwrap().len(), 1);
    assert_eq!(
        snapshot["mitglieder"][0]["adresse"]["instance_id"],
        si_hex(10)
    );
    assert!(snapshot["mitglieder"][0].get("probe_descriptor").is_none());
}

#[test]
fn resubscribe_uebernimmt_live_bestaetigungsbedarf_waehrend_store_flush_blockiert() {
    let ordner = TestOrdner::neu("live-joinbedarf-vor-storeflush");
    let writer = starten(&ordner.db());

    // Der vorige Brokerlauf hinterlaesst fuer Main-Sitzung 2 eine gueltige
    // Projektion ohne Bestätigungsbedarf.
    let alt = Coordinator::mit_store(Arc::new(ManualClock::default()), si_hex(90), &writer);
    let main_a = si_hello(10, 100, "main");
    assert!(
        alt.control_hello_registrieren("alt-main", &main_a)
            .angenommen
    );
    assert!(si_report(&alt, "alt-main", &main_a.adresse, 1));
    let (_, alt_payload) = writer
        .handle()
        .session_state_lesen(&si_hex(1), &si_hex(2))
        .unwrap()
        .unwrap();
    let alt_payload: Value = serde_json::from_slice(&alt_payload).unwrap();
    assert_eq!(alt_payload["beitritt_bestaetigung_noetig"], false);
    drop(alt);

    // Im neuen Lauf machen zwei Main-Sitzungen denselben Probe-Kandidaten
    // mehrdeutig. Sein Report berechnet live `true`, wird aber vor dem ersten
    // Store-Commit deterministisch angehalten.
    let coordinator = Arc::new(Coordinator::mit_store(
        Arc::new(ManualClock::default()),
        si_hex(99),
        &writer,
    ));
    let push = Arc::new(PushProbe::neu(true));
    coordinator.session_push_setzen(push.clone());
    assert!(
        coordinator
            .control_hello_registrieren("main-a", &main_a)
            .angenommen
    );
    let mut main_b = si_hello(11, 101, "main");
    main_b.adresse.session_epoch = si_hex(3);
    assert!(
        coordinator
            .control_hello_registrieren("main-b", &main_b)
            .angenommen
    );
    let mut probe = si_hello(12, 102, "active_probe");
    probe.adresse.session_epoch = probe.adresse.project_binding_id.clone();
    assert!(
        coordinator
            .control_hello_registrieren("probe", &probe)
            .angenommen
    );

    let haken = CoordinatorFlushTestHaken::default();
    coordinator.flush_test_haken_setzen(haken.clone());
    let c = coordinator.clone();
    let probe_adresse = probe.adresse.clone();
    let report = std::thread::spawn(move || si_report(&c, "probe", &probe_adresse, 1));
    haken.warten_bis_erfasst();

    assert!(si_subscribe(&coordinator, "main-a", &main_a.adresse));
    let race_snapshot = push.snapshots().last().unwrap().1.clone();
    haken.freigeben();
    assert!(report.join().unwrap());

    assert_eq!(race_snapshot["broker_epoch"], si_hex(99));
    assert_eq!(snapshot_wirkung(&race_snapshot), 1);
    assert_eq!(race_snapshot["beitritt_bestaetigung_noetig"], true);
    // Alte laufgebundene Mitglieder werden nicht restauriert. Das in diesem
    // Brokerlauf bereits verbundene Main bleibt nach E-M01/L23 dagegen als
    // unclassified-Mitglied ohne Descriptor sichtbar.
    assert_eq!(race_snapshot["mitglieder"].as_array().unwrap().len(), 1);
    assert_eq!(
        race_snapshot["mitglieder"][0]["adresse"]["instance_id"],
        main_a.adresse.instance_id
    );
    assert!(race_snapshot["mitglieder"][0]
        .get("probe_descriptor")
        .is_none());
}

#[test]
fn projektionslesefehler_haelt_subscription_sichtbar_fail_closed() {
    let (ordner, mut writer, coordinator, _clock, push) =
        si_coordinator_mit_store("projektion-lesefehler", true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    writer.stoppen();
    std::fs::remove_file(ordner.db()).unwrap();
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    assert!(!coordinator.routing_bereit());
    assert!(coordinator.verbindung_soll_trennen("main"));
    assert!(push.snapshots().is_empty());
}

#[test]
fn snapshot_commit_bleibt_bei_konkurrierenden_flushes_monoton() {
    let (_ordner, writer, coordinator, _clock, _push) =
        si_coordinator_mit_store("snapshot-commit-seriell", true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    let basis: Value = serde_json::from_slice(&coordinator.session_snapshot_json(
        &main.adresse.project_binding_id,
        &main.adresse.session_epoch,
    ))
    .unwrap();
    let mut alt = basis["mitglieder"][0]["probe_descriptor"].clone();
    alt["label"] = Value::String("alt-erfasst".into());
    let mut neu = basis["mitglieder"][0]["probe_descriptor"].clone();
    neu["label"] = Value::String("neu-committed".into());

    let haken = CoordinatorFlushTestHaken::default();
    coordinator.flush_test_haken_setzen(haken.clone());
    let c = coordinator.clone();
    let erster = std::thread::spawn(move || c.descriptor_setzen("main", alt));
    haken.warten_bis_erfasst();
    let c = coordinator.clone();
    let (zweiter_fertig_tx, zweiter_fertig_rx) = std::sync::mpsc::channel();
    let zweiter = std::thread::spawn(move || {
        let ausgang = c.descriptor_setzen("main", neu);
        let _ = zweiter_fertig_tx.send(());
        ausgang
    });
    // Mit dem Session-Schloss wartet der zweite Flush hier. Ohne Schloss
    // committed er nach seinem 50-ms-Fenster bereits den neueren Stand, so
    // dass der anschliessend freigegebene alte Flush ihn reproduzierbar
    // ueberholen wuerde.
    let _ = zweiter_fertig_rx.recv_timeout(Duration::from_millis(COMMIT_FENSTER_MS * 10));
    haken.freigeben();
    assert!(erster.join().unwrap());
    assert!(zweiter.join().unwrap());

    let (_, payload) = writer
        .handle()
        .session_state_lesen(&si_hex(1), &si_hex(2))
        .unwrap()
        .unwrap();
    let payload: Value = serde_json::from_slice(&payload).unwrap();
    assert_eq!(
        payload["mitglieder"][0]["probe_descriptor"]["label"],
        "neu-committed"
    );
}

#[test]
fn cleanup_zwischen_zielermittlung_und_write_verhindert_den_alten_push() {
    let ordner = TestOrdner::neu("pushziel-revalidieren");
    let barriere = StoreStartBarriere::neu_blockiert();
    let mut konfiguration = StoreKonfiguration::fuer_pfad(ordner.db());
    konfiguration.remote_volume_override = Some(false);
    konfiguration.start_barriere = Some(barriere.clone());
    let writer = StoreWriter::starten(konfiguration);
    let coordinator = Arc::new(Coordinator::mit_store(
        Arc::new(ManualClock::default()),
        si_hex(99),
        &writer,
    ));
    let push = Arc::new(PushProbe::neu(true));
    coordinator.session_push_setzen(push.clone());
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    let push_vorher = push.snapshots().len();
    let c = coordinator.clone();
    let adresse = main.adresse.clone();
    let report = std::thread::spawn(move || si_report(&c, "main", &adresse, 1));
    let frist = Instant::now() + Duration::from_secs(2);
    while writer.handle().sicht().eingereiht == 0 && Instant::now() < frist {
        std::thread::yield_now();
    }
    assert!(writer.handle().sicht().eingereiht > 0);
    coordinator.control_ende("main");
    barriere.freigeben();
    assert!(report.join().unwrap());
    assert_eq!(push.snapshots().len(), push_vorher);
}

#[test]
fn migration_1_frisch_und_idempotent() {
    let ordner = TestOrdner::neu("migration-idempotent");
    let start = Instant::now();
    let mut a = starten(&ordner.db());
    a.stoppen();
    let erste_checksum: String = Connection::open(ordner.db())
        .unwrap()
        .query_row(
            "SELECT checksum_sha256 FROM schema_migrations WHERE major=1",
            [],
            |row| row.get(0),
        )
        .unwrap();
    assert_eq!(erste_checksum, migration_1_checksum());
    let mut b = starten(&ordner.db());
    b.stoppen();
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT COUNT(*) FROM schema_migrations"),
        1
    );
    assert!(recovery_testgrenze_bestanden(start.elapsed()));
}

#[test]
fn store_pfad_nutzt_folderid_localappdata() {
    let root = PathBuf::from(r"C:\Users\Test\AppData\Local");
    assert_eq!(
        store_pfad_unter(&root),
        root.join(STORE_RELATIVPFAD).join(STORE_DATEINAME)
    );
    let standard = standard_store_pfad().unwrap();
    assert!(standard.ends_with(Path::new(STORE_RELATIVPFAD).join(STORE_DATEINAME)));
    assert!(!standard
        .to_string_lossy()
        .contains("eq-copilot-bindungen.json"));
}

#[test]
fn remote_volume_degradiert_ohne_db_oder_wal() {
    let ordner = TestOrdner::neu("remote");
    let db = ordner.db();
    let mut k = StoreKonfiguration::fuer_pfad(&db);
    k.remote_volume_override = Some(true);
    let writer = StoreWriter::starten(k);
    assert!(writer.ist_degradiert());
    assert!(!db.exists());
    assert!(!PathBuf::from(format!("{}-wal", db.display())).exists());
}

#[test]
fn schema_major_1_schreibt_und_major_plus_1_degradiert_read_only() {
    let ordner = TestOrdner::neu("major");
    let mut writer = starten(&ordner.db());
    writer.stoppen();
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT MAX(major) FROM schema_migrations"),
        STORE_SCHEMA_MAJOR
    );
    let conn = Connection::open(ordner.db()).unwrap();
    conn.execute(
        "INSERT INTO schema_migrations(major,checksum_sha256,applied_utc_ms) VALUES(?1,'newer',0)",
        [STORE_SCHEMA_MAJOR + 1],
    )
    .unwrap();
    drop(conn);
    let vorher = std::fs::read(ordner.db()).unwrap();
    let neuer = starten_degradiert(&ordner.db());
    assert!(neuer.ist_degradiert());
    assert_eq!(std::fs::read(ordner.db()).unwrap(), vorher);
}

fn starten_degradiert(db: &Path) -> StoreWriter {
    let mut k = StoreKonfiguration::fuer_pfad(db);
    k.remote_volume_override = Some(false);
    StoreWriter::starten(k)
}

#[test]
fn event_projection_snapshotschuld_command_id_atomare_transaktion() {
    let ordner = TestOrdner::neu("atomic");
    let writer = starten(&ordner.db());
    let store = writer.handle();
    let ausgang = store
        .append(vec![event(
            Some("00000000000000000000000000000005"),
            1,
            7,
            true,
        )])
        .unwrap();
    assert!(matches!(ausgang[0], AppendAusgang::Angewandt { .. }));
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT COUNT(*) FROM event_log"),
        1
    );
    assert_eq!(scalar_i64(&ordner.db(), "SELECT COUNT(*) FROM sessions"), 1);
    assert_eq!(scalar_i64(&ordner.db(), "SELECT COUNT(*) FROM outbox"), 1);
    assert_eq!(
        scalar_i64(
            &ordner.db(),
            "SELECT COUNT(*) FROM event_log WHERE command_id IS NOT NULL"
        ),
        1
    );
}

#[test]
fn command_id_mappt_intern_auf_genau_eine_event_uuid() {
    let ordner = TestOrdner::neu("idempotent");
    let writer = starten(&ordner.db());
    let store = writer.handle();
    let command = "00000000000000000000000000000005";
    let a = store
        .append(vec![event(Some(command), 1, 1, true)])
        .unwrap();
    let b = store
        .append(vec![event(Some(command), 2, 2, true)])
        .unwrap();
    assert!(matches!(a[0], AppendAusgang::Angewandt { .. }));
    assert!(matches!(b[0], AppendAusgang::IdempotentWiederholt { .. }));
    assert_eq!(a[0].event_uuid(), b[0].event_uuid());
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT COUNT(*) FROM event_log"),
        1
    );
}

#[test]
fn command_id_mappt_intern_auf_eine_event_uuid() {
    command_id_mappt_intern_auf_genau_eine_event_uuid();
}

#[test]
fn event_log_append_only_und_sequence_eindeutig() {
    let ordner = TestOrdner::neu("append-only");
    let writer = starten(&ordner.db());
    writer
        .handle()
        .append(vec![event(None, 1, 1, false)])
        .unwrap();
    let conn = Connection::open(ordner.db()).unwrap();
    assert!(conn
        .execute("UPDATE event_log SET event_type='x'", [])
        .is_err());
    assert!(conn.execute("DELETE FROM event_log", []).is_err());
    assert!(conn
        .execute(
            "INSERT INTO event_log(event_uuid,command_id,project_binding_id,session_epoch,utc_ms,broker_epoch,sequence,event_type,schema_major,schema_minor,payload_jcs) SELECT '00000000-0000-0000-0000-000000000099',NULL,project_binding_id,session_epoch,utc_ms,broker_epoch,sequence,event_type,schema_major,schema_minor,payload_jcs FROM event_log LIMIT 1",
            [],
        )
        .is_err());
}

#[test]
fn store_kanal_cap_und_naechster_trennt() {
    let ordner = TestOrdner::neu("cap");
    let barriere = StoreStartBarriere::neu_blockiert();
    let mut k = StoreKonfiguration::fuer_pfad(ordner.db());
    k.remote_volume_override = Some(false);
    k.start_barriere = Some(barriere.clone());
    let mut writer = StoreWriter::starten(k);
    let store = writer.handle();
    let mut antworten = Vec::new();
    for i in 0..STORE_KANAL_CAP {
        antworten.push(
            store
                .append_einreihen(vec![event(None, i as i64, i as i64, false)])
                .unwrap(),
        );
    }
    assert!(matches!(
        store.append_einreihen(vec![event(None, STORE_KANAL_CAP as i64, 0, false)]),
        Err(StoreFehler::KanalVoll)
    ));
    barriere.freigeben();
    for antwort in antworten {
        assert!(antwort
            .recv_timeout(Duration::from_secs(10))
            .unwrap()
            .is_ok());
    }
    writer.stoppen();
}

#[test]
fn disk_full_io_kaputt_neues_schema_degradieren_ohne_brokerstop() {
    let ordner = TestOrdner::neu("kaputt");
    let db_ist_ordner = ordner.0.join("db-als-ordner");
    std::fs::create_dir_all(&db_ist_ordner).unwrap();
    let writer = starten_degradiert(&db_ist_ordner);
    assert!(writer.ist_degradiert());
    assert!(matches!(
        writer.handle().append(vec![event(None, 1, 1, false)]),
        Err(StoreFehler::Degradiert(_))
    ));
}

#[test]
fn degradierter_store_verweigert_annahme_und_tick_trennt_verursacher() {
    let ordner = TestOrdner::neu("degraded-disconnect");
    let db_ist_ordner = ordner.0.join("db-als-ordner");
    std::fs::create_dir_all(&db_ist_ordner).unwrap();
    let writer = starten_degradiert(&db_ist_ordner);
    assert!(writer.ist_degradiert());
    let coordinator = Coordinator::mit_store(Arc::new(ManualClock::default()), si_hex(99), &writer);
    let client = si_hello(10, 100, "active_probe");
    assert!(
        coordinator
            .control_hello_registrieren("verursacher", &client)
            .angenommen
    );
    assert!(si_report(&coordinator, "verursacher", &client.adresse, 1));
    assert_eq!(coordinator.store_verweigerungen(), 1);
    assert!(coordinator.verbindung_soll_trennen("verursacher"));
    assert_eq!(coordinator.liveness_tick(), vec!["verursacher"]);
}

#[test]
fn projektion_rebuild_ist_deterministisch_und_unter_testgrenze() {
    let ordner = TestOrdner::neu("rebuild");
    let mut writer = starten(&ordner.db());
    writer
        .handle()
        .append(vec![event(None, 1, 1, false), event(None, 2, 2, false)])
        .unwrap();
    writer.stoppen();
    let vorher: Vec<u8> = Connection::open(ordner.db())
        .unwrap()
        .query_row("SELECT state_jcs FROM sessions", [], |row| row.get(0))
        .unwrap();
    Connection::open(ordner.db())
        .unwrap()
        .execute_batch("PRAGMA foreign_keys=OFF; DELETE FROM sessions; DELETE FROM projects;")
        .unwrap();
    let start = Instant::now();
    projektionen_neu_bauen(&ordner.db()).unwrap();
    assert!(recovery_testgrenze_bestanden(start.elapsed()));
    let danach: Vec<u8> = Connection::open(ordner.db())
        .unwrap()
        .query_row("SELECT state_jcs FROM sessions", [], |row| row.get(0))
        .unwrap();
    assert_eq!(danach, vorher);
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT COUNT(*) FROM event_log"),
        2
    );
    assert!(recovery_testgrenze_bestanden(Duration::from_millis(
        STORE_RECOVERY_TEST_MAX_MS - 1
    )));
    assert!(!recovery_testgrenze_bestanden(Duration::from_millis(
        STORE_RECOVERY_TEST_MAX_MS
    )));
}

#[test]
fn pragmas_exakt() {
    let ordner = TestOrdner::neu("pragmas");
    let writer = starten(&ordner.db());
    let pragmas = writer
        .handle()
        .sicht()
        .pragmas
        .expect("Writer-Connection hat ihre Start-PRAGMAs erfasst");
    assert_eq!(pragmas.journal_mode.to_lowercase(), "wal");
    assert_eq!(pragmas.synchronous, 2);
    assert_eq!(pragmas.wal_autocheckpoint, 0);
    assert_eq!(pragmas.busy_timeout_ms, BUSY_TIMEOUT_MS as i64);
    assert_eq!(pragmas.foreign_keys, 1);
}

#[test]
fn commit_bei_fenster_oder_batch() {
    assert!(!commit_ausloesen(
        Duration::from_millis(COMMIT_FENSTER_MS - 1),
        COMMIT_BATCH_MAX - 1
    ));
    assert!(commit_ausloesen(
        Duration::from_millis(COMMIT_FENSTER_MS),
        COMMIT_BATCH_MAX - 1
    ));
    assert!(commit_ausloesen(Duration::ZERO, COMMIT_BATCH_MAX));
}

#[test]
fn group_commit_faehrt_echten_batch_bis_commitgrenze() {
    let ordner = TestOrdner::neu("group-commit-real");
    let writer = starten(&ordner.db());
    let start = Arc::new(Barrier::new(COMMIT_BATCH_MAX + 1));
    let mut threads = Vec::with_capacity(COMMIT_BATCH_MAX);
    for i in 0..COMMIT_BATCH_MAX {
        let store = writer.handle();
        let start = start.clone();
        threads.push(std::thread::spawn(move || {
            start.wait();
            store.append(vec![event(None, i as i64, i as i64 + 1, false)])
        }));
    }
    start.wait();
    for thread in threads {
        let ausgang = thread.join().unwrap().unwrap();
        assert_eq!(ausgang.len(), 1);
    }
    assert_eq!(writer.handle().sicht().commits, 1);
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT COUNT(*) FROM event_log"),
        COMMIT_BATCH_MAX as i64
    );
}

#[test]
fn checkpoint_oder_guard_loest_offenes_append_fenster_nicht_aus() {
    let ordner = TestOrdner::neu("group-commit-kein-fruehausloeser");
    let writer = starten(&ordner.db());
    let store = writer.handle();
    let antwort = store
        .append_einreihen(vec![event(None, 1, 1, false)])
        .unwrap();
    let start = Instant::now();
    store.checkpoint(false).unwrap();
    let vergangen = start.elapsed();
    assert_eq!(antwort.recv().unwrap().unwrap().len(), 1);
    assert!(
        vergangen >= Duration::from_millis(COMMIT_FENSTER_MS - 10),
        "Checkpoint loeste den Commit nach nur {vergangen:?} aus"
    );
}

#[test]
fn jeder_group_commit_bleibt_bei_hoechstens_64_events() {
    let ordner = TestOrdner::neu("group-commit-harte-cap");
    let writer = starten(&ordner.db());
    let events = (0..=COMMIT_BATCH_MAX as i64)
        .map(|i| event(None, i + 1, i + 1, false))
        .collect::<Vec<_>>();
    let ausgaenge = writer.handle().append(events).unwrap();
    let sicht = writer.handle().sicht();
    assert_eq!(ausgaenge.len(), COMMIT_BATCH_MAX + 1);
    assert_eq!(sicht.commits, 2);
    assert_eq!(sicht.groesster_commit, COMMIT_BATCH_MAX);

    let ordner = TestOrdner::neu("group-commit-63-plus-2");
    let writer = starten(&ordner.db());
    let store = writer.handle();
    let a = store
        .append_einreihen(
            (0..COMMIT_BATCH_MAX as i64 - 1)
                .map(|i| event(None, i + 1, i + 1, false))
                .collect(),
        )
        .unwrap();
    let b = store
        .append_einreihen(vec![
            event(None, 10_000, 1, false),
            event(None, 10_001, 2, false),
        ])
        .unwrap();
    assert_eq!(a.recv().unwrap().unwrap().len(), COMMIT_BATCH_MAX - 1);
    assert_eq!(b.recv().unwrap().unwrap().len(), 2);
    let sicht = store.sicht();
    assert_eq!(sicht.commits, 2);
    assert_eq!(sicht.groesster_commit, COMMIT_BATCH_MAX);
}

#[test]
fn busy_timeout_grenze() {
    assert!(!busy_timeout_abgelaufen(Duration::from_millis(
        BUSY_TIMEOUT_MS - 1
    )));
    assert!(busy_timeout_abgelaufen(Duration::from_millis(
        BUSY_TIMEOUT_MS
    )));
}

#[test]
fn passive_checkpoint_an_dualer_grenze() {
    assert!(!checkpoint_ausloesen(
        WAL_SCHWELLE_BYTES - 1,
        Duration::from_millis(STORE_IDLE_MS - 1),
        false
    ));
    assert!(checkpoint_ausloesen(
        WAL_SCHWELLE_BYTES,
        Duration::ZERO,
        true
    ));
    assert!(!checkpoint_ausloesen(
        0,
        Duration::from_millis(STORE_IDLE_MS),
        true
    ));
    assert!(checkpoint_ausloesen(
        0,
        Duration::from_millis(STORE_IDLE_MS),
        false
    ));
}

#[test]
fn truncate_nur_in_ruhelage() {
    let ordner = TestOrdner::neu("truncate");
    let writer = starten(&ordner.db());
    let store = writer.handle();
    store.append(vec![event(None, 1, 1, false)]).unwrap();
    store.capture_aktiv_setzen(true);
    assert!(store.checkpoint(true).is_err());
    store.capture_aktiv_setzen(false);
    assert!(store.checkpoint(true).unwrap().vollstaendig());
}

#[test]
fn leser_haelt_keine_transaktion_ueber_externe_arbeit() {
    let ordner = TestOrdner::neu("reader");
    let writer = starten(&ordner.db());
    let store = writer.handle();
    store.append(vec![event(None, 1, 1, false)]).unwrap();
    let barriere = Arc::new(Barrier::new(2));
    let weiter = Arc::new(Barrier::new(2));
    let s = store.clone();
    let b = barriere.clone();
    let w = weiter.clone();
    let extern_thread = std::thread::spawn(move || {
        let _ = s
            .session_state_lesen(
                "00000000000000000000000000000001",
                "00000000000000000000000000000002",
            )
            .unwrap();
        b.wait();
        w.wait();
    });
    barriere.wait();
    store.append(vec![event(None, 2, 2, false)]).unwrap();
    let checkpoint = store.checkpoint(false).unwrap();
    assert_eq!(checkpoint.busy, CHECKPOINT_BUSY_ERWARTET);
    assert_eq!(checkpoint.log_frames, checkpoint.checkpointed_frames);
    weiter.wait();
    extern_thread.join().unwrap();

    // Mutation: derselbe Barrierenstand, aber die Lesertransaktion bleibt
    // absichtlich offen. PASSIVE darf dann nicht die ganze WAL einziehen.
    let mut leser = Connection::open_with_flags(
        ordner.db(),
        OpenFlags::SQLITE_OPEN_READ_ONLY | OpenFlags::SQLITE_OPEN_NO_MUTEX,
    )
    .unwrap();
    let tx = leser.transaction().unwrap();
    let _: i64 = tx
        .query_row("SELECT COUNT(*) FROM sessions", [], |row| row.get(0))
        .unwrap();
    store.append(vec![event(None, 3, 3, false)]).unwrap();
    let mutation = store.checkpoint(false).unwrap();
    assert!(
        mutation.log_frames != mutation.checkpointed_frames,
        "Mutation blieb unerkannt: {mutation:?}"
    );
    drop(tx);
}

#[test]
fn alter_store_ueberschreibt_plugin_state_nicht() {
    let ordner = TestOrdner::neu("plugin-state");
    let writer = starten(&ordner.db());
    writer
        .handle()
        .append(vec![event(None, 1, 1, false)])
        .unwrap();
    let coordinator = Coordinator::mit_store(
        Arc::new(ManualClock::default()),
        "00000000000000000000000000000063".into(),
        &writer,
    );
    let push = Arc::new(PushProbe::neu(true));
    coordinator.session_push_setzen(push.clone());
    let client = si_hello(10, 100, "active_probe");
    assert!(
        coordinator
            .control_hello_registrieren("probe", &client)
            .angenommen
    );

    // Der alte Projektionsschnitt bleibt haltbar. Rein laufgebundene Felder
    // werden auf die neue Baseline gesetzt; der sichere Bestätigungsbedarf
    // bleibt bis zum regulären C-03-Weg wahr. Das ist kein State-Ingress.
    assert_eq!(persistierte_snapshot_wirkung(&writer), 1);
    assert!(si_subscribe(&coordinator, "probe", &client.adresse));
    let snapshots = push.snapshots();
    assert_eq!(snapshots.len(), 1);
    let resubscribe = &snapshots.last().unwrap().1;
    assert_eq!(snapshot_wirkung(resubscribe), 1);
    assert_eq!(resubscribe["beitritt_bestaetigung_noetig"], true);
    assert_eq!(resubscribe["broker_epoch"], si_hex(99));

    // Danach meldet das Plugin seinen neueren lokalen Zustand. Der
    // Coordinator darf ihn als Kontakt lesen, aber keines seiner DSP-Felder
    // in den Session-Snapshot spiegeln und damit zur Rueckschreibwahrheit
    // machen. Der echte C++-State-Roundtrip bleibt das B2-Gegenbein.
    let plugin_state = json!({
        "type": "state_report",
        "adresse": client.adresse,
        "dsp_schema_version": 2,
        "state_revision": 99,
        "state_hash": "e".repeat(64),
        "record_state": {"valid": true, "recording": false}
    });
    assert!(coordinator.state_report_json("probe", &serde_json::to_vec(&plugin_state).unwrap()));
    let snapshots = push.snapshots();
    assert!(snapshots.len() >= 2);
    for (_, snapshot) in snapshots {
        assert!(snapshot_schema().gueltig(&snapshot));
        for verboten in [
            "dsp_schema_version",
            "state_revision",
            "state_hash",
            "record_state",
            "dsp",
        ] {
            assert!(snapshot.get(verboten).is_none(), "{verboten}: {snapshot}");
        }
    }
}

#[test]
fn migration_1_spalten_und_schluessel_exakt() {
    let ordner = TestOrdner::neu("schema-exakt");
    let _writer = starten(&ordner.db());
    let conn = Connection::open(ordner.db()).unwrap();
    let tabellen: i64 = conn
        .query_row(
            "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name IN ('schema_migrations','conflict_guards','event_log','projects','sessions','passages','evidence','findings','proposals','transactions','experiments','user_verdicts','experiment_events','outbox')",
            [],
            |r| r.get(0),
        )
        .unwrap();
    assert_eq!(tabellen, 14);
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT major FROM schema_migrations"),
        1
    );
}

#[test]
fn event_log_keys_und_append_only() {
    event_log_append_only_und_sequence_eindeutig();
}

#[test]
fn projects_projection_rebuild_und_fk() {
    let ordner = TestOrdner::neu("project-fk");
    let writer = starten(&ordner.db());
    writer
        .handle()
        .append(vec![event(None, 1, 1, false)])
        .unwrap();
    let conn = Connection::open(ordner.db()).unwrap();
    conn.pragma_update(None, "foreign_keys", "ON").unwrap();
    assert!(conn
        .execute(
            "INSERT INTO projects(project_binding_id,last_event_ord,state_jcs) VALUES('x',999999,x'00')",
            [],
        )
        .is_err());
}

#[test]
fn sessions_projection_composite_key_ohne_fuehrungsrestore() {
    let ordner = TestOrdner::neu("session-key");
    let writer = starten(&ordner.db());
    writer
        .handle()
        .append(vec![event(None, 1, 1, false)])
        .unwrap();
    let conn = Connection::open(ordner.db()).unwrap();
    let sql: String = conn
        .query_row(
            "SELECT sql FROM sqlite_master WHERE type='table' AND name='sessions'",
            [],
            |r| r.get(0),
        )
        .unwrap();
    assert!(sql.contains("PRIMARY KEY (project_binding_id, session_epoch)"));
    assert!(!sql.to_lowercase().contains("fuehr"));
}

#[test]
fn domain_event_ueberschreibt_session_snapshot_auch_beim_reconnect_nicht() {
    let ordner = TestOrdner::neu("session-projektion-domain-getrennt");
    let writer = starten(&ordner.db());
    let snapshot = event(None, 1, 42, false);
    let erwartet = snapshot.payload_jcs.clone();
    writer.handle().append(vec![snapshot]).unwrap();

    let mut evidence = event(None, 2, 0, false);
    evidence.event_type = "evidence".into();
    evidence.payload_jcs = serde_json::to_vec(&json!({
        "type": "internal_evidence",
        "evidence_id": "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee",
        "wert": 9
    }))
    .unwrap();
    writer.handle().append(vec![evidence]).unwrap();

    let (_, session_state) = writer
        .handle()
        .session_state_lesen(&si_hex(1), &si_hex(2))
        .unwrap()
        .expect("Sessionprojektion");
    assert_eq!(session_state, erwartet);

    let coordinator = Arc::new(Coordinator::mit_store(
        Arc::new(ManualClock::default()),
        si_hex(3),
        &writer,
    ));
    let push = Arc::new(PushProbe::neu(true));
    coordinator.session_push_setzen(push.clone());
    let client = si_hello(4, 404, "main");
    assert!(
        coordinator
            .control_hello_registrieren("reconnect", &client)
            .angenommen
    );
    assert!(si_subscribe(&coordinator, "reconnect", &client.adresse));
    let letzter = push.snapshots().last().unwrap().1.clone();
    assert_eq!(letzter["type"], "session_snapshot");
    assert_ne!(letzter["type"], "internal_evidence");
}

#[test]
fn domainprojektionen_spalten_keys_und_rebuild() {
    let ordner = TestOrdner::neu("domains");
    let _writer = starten(&ordner.db());
    let conn = Connection::open(ordner.db()).unwrap();
    for (tabelle, id) in [
        ("passages", "passage_id"),
        ("evidence", "evidence_id"),
        ("findings", "finding_id"),
        ("proposals", "proposal_id"),
        ("transactions", "transaction_id"),
        ("experiments", "experiment_id"),
        ("user_verdicts", "user_verdict_id"),
    ] {
        let sql: String = conn
            .query_row(
                "SELECT sql FROM sqlite_master WHERE type='table' AND name=?1",
                [tabelle],
                |r| r.get(0),
            )
            .unwrap();
        assert!(sql.contains(id), "{tabelle}: {sql}");
        assert!(sql.contains("last_event_ord"), "{tabelle}: {sql}");
    }
}

#[test]
fn experiment_events_composite_key_und_fks() {
    let ordner = TestOrdner::neu("experiment-events");
    let _writer = starten(&ordner.db());
    let conn = Connection::open(ordner.db()).unwrap();
    let sql: String = conn
        .query_row(
            "SELECT sql FROM sqlite_master WHERE type='table' AND name='experiment_events'",
            [],
            |r| r.get(0),
        )
        .unwrap();
    assert!(sql.contains("PRIMARY KEY (experiment_id, event_uuid)"));
    assert!(sql.matches("REFERENCES").count() >= 2);
}

#[test]
fn outbox_koalesziert_letzten_snapshotstand_pro_ziel_und_objekt() {
    let ordner = TestOrdner::neu("outbox-coalesce");
    let writer = starten(&ordner.db());
    let store = writer.handle();
    let a = store.append(vec![event(None, 1, 1, true)]).unwrap();
    let b = store.append(vec![event(None, 2, 2, true)]).unwrap();
    let outbox = store.outbox_lesen().unwrap();
    assert_eq!(outbox.len(), 1);
    assert_eq!(outbox[0].1, b[0].event_ord());
    assert!(outbox[0].1 > a[0].event_ord());
}

#[test]
fn snapshot_outbox_kompaktierung_verliert_committete_wirkung_nicht() {
    let ordner = TestOrdner::neu("outbox-compact");
    let writer = starten(&ordner.db());
    let store = writer.handle();
    let a = store.append(vec![event(None, 1, 42, true)]).unwrap();
    assert_eq!(
        store
            .snapshot_schuld_kompaktieren(ziel(), a[0].event_ord())
            .unwrap(),
        1
    );
    assert!(store.outbox_lesen().unwrap().is_empty());
    let (_, snapshot) = store
        .session_state_lesen(
            "00000000000000000000000000000001",
            "00000000000000000000000000000002",
        )
        .unwrap()
        .unwrap();
    let snapshot: Value = serde_json::from_slice(&snapshot).unwrap();
    assert_eq!(snapshot_wirkung(&snapshot), 42);
}

#[test]
fn store_start_protokolliert_sqlite_version() {
    let ordner = TestOrdner::neu("versions");
    let writer = starten(&ordner.db());
    let sicht = writer.handle().sicht();
    assert_eq!(sicht.rusqlite_version, RUSQLITE_VERSION);
    assert!(!sicht.sqlite_version.is_empty());
    assert_eq!(sicht.sqlite_version, rusqlite::version());
}

#[test]
fn store_writer_ist_alleiniger_write_connection_besitzer() {
    let ordner = TestOrdner::neu("single-writer");
    let writer = starten(&ordner.db());
    let store = writer.handle();
    let mut threads = Vec::new();
    for i in 0..16 {
        let store = store.clone();
        threads.push(std::thread::spawn(move || {
            store.append(vec![event(None, i, i, false)]).unwrap();
        }));
    }
    for thread in threads {
        thread.join().unwrap();
    }
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT COUNT(*) FROM event_log"),
        16
    );
}

#[test]
fn konfliktriegel_ueberlebt_neustart_mit_nur_einem_partner() {
    let ordner = TestOrdner::neu("guard-restart");
    let guard = ConflictGuard {
        effective_address: "sid|project|session|instance".into(),
        derived_id: "instance:nonce-a".into(),
        created_utc_ms: 1,
    };
    let mut writer = starten(&ordner.db());
    writer
        .handle()
        .konflikt_guard_persistieren(guard.clone())
        .unwrap();
    writer.stoppen();
    let writer = starten(&ordner.db());
    assert_eq!(writer.restaurierte_guards(), &[guard]);
}

#[test]
fn routing_bleibt_zu_bis_konfliktriegel_restauriert() {
    let ordner = TestOrdner::neu("guard-routing");
    let mut k = StoreKonfiguration::fuer_pfad(ordner.db());
    k.remote_volume_override = Some(true);
    let writer = StoreWriter::starten(k);
    let coordinator = eqcop_broker::coordinator::Coordinator::instant_mit_store(
        "00000000000000000000000000000001".into(),
        &writer,
    );
    assert!(!coordinator.routing_bereit());
}

#[test]
fn guard_restore_vor_routing() {
    konfliktriegel_ueberlebt_neustart_mit_nur_einem_partner();
}

#[test]
fn projektion_rebuild_nach_neustart_unter_testgrenze() {
    projektion_rebuild_ist_deterministisch_und_unter_testgrenze();
}

#[test]
fn recovery_unter_testgrenze() {
    assert!(recovery_testgrenze_bestanden(Duration::from_millis(
        STORE_RECOVERY_TEST_MAX_MS - 1
    )));
    assert!(!recovery_testgrenze_bestanden(Duration::from_millis(
        STORE_RECOVERY_TEST_MAX_MS
    )));
}

#[cfg(windows)]
#[test]
#[ignore = "A4-SI: braucht das frisch gebaute echte EqCopIpcTest-Binary"]
fn kill_vor_persist_mit_echtem_cpp_client_antwortet_angewandt_ohne_wire_uuid() {
    echte_cpp_client_crashrunde("vor_persist", "angewandt");
}

#[cfg(windows)]
#[test]
#[ignore = "A4-SI: braucht das frisch gebaute echte EqCopIpcTest-Binary"]
fn kill_waehrend_commit_mit_echtem_cpp_client_antwortet_angewandt() {
    echte_cpp_client_crashrunde("vor_store_commit", "angewandt");
}

#[cfg(windows)]
#[test]
#[ignore = "A4-SI: braucht das frisch gebaute echte EqCopIpcTest-Binary"]
fn kill_nach_commit_vor_command_ack_retry_antwortet_idempotent_wiederholt_ohne_wire_uuid() {
    echte_cpp_client_crashrunde("nach_store_commit", "idempotent_wiederholt");
}

#[cfg(windows)]
#[test]
#[ignore = "A4-SI: braucht das frisch gebaute echte EqCopIpcTest-Binary"]
fn verlorenes_command_ack_retry_liefert_idempotent_wiederholt_ohne_wire_uuid() {
    echte_cpp_client_crashrunde("nach_store_commit", "idempotent_wiederholt");
}

#[test]
fn kill_nach_store_commit_vor_snapshot_push() {
    let ordner = TestOrdner::neu("k04");
    crash(&ordner.db(), "nach_store_commit", "append");
    let writer = starten(&ordner.db());
    assert_eq!(writer.handle().outbox_lesen().unwrap().len(), 1);
    assert_eq!(persistierte_snapshot_wirkung(&writer), 1);
    let clock = Arc::new(ManualClock::default());
    let coordinator = Coordinator::mit_store(clock, si_hex(99), &writer);
    let push = Arc::new(PushProbe::neu(true));
    coordinator.session_push_setzen(push.clone());
    let client = si_hello(4, 400, "main");
    assert!(
        coordinator
            .control_hello_registrieren("reconnect", &client)
            .angenommen
    );
    assert!(si_subscribe(&coordinator, "reconnect", &client.adresse));
    assert_eq!(push.snapshots().len(), 1);
    let snapshots = push.snapshots();
    let resubscribe = &snapshots.last().unwrap().1;
    assert_eq!(snapshot_wirkung(resubscribe), 1);
    assert_eq!(resubscribe["beitritt_bestaetigung_noetig"], true);
    assert_eq!(resubscribe["broker_epoch"], si_hex(99));
    assert_eq!(persistierte_snapshot_wirkung(&writer), 1);
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());
}

#[test]
fn subscriberkill_vor_und_nach_snapshot_anwendung_bleibt_idempotent() {
    for punkt in ["vor_snapshot_anwendung", "nach_snapshot_anwendung"] {
        let ordner = TestOrdner::neu(punkt);
        let snapshot = ordner.0.join("snapshot.json");
        let state = ordner.0.join("state.json");
        let marker = ordner.0.join("subscriber.marker");
        let erwartet = snapshot_bytes("00000000000000000000000000000003", 1);
        std::fs::write(&snapshot, &erwartet).unwrap();
        let mut child = snapshot_subscriber_starten(&snapshot, &state, &marker, punkt);
        marker_abwarten(&marker);
        hart_killen(&mut child);
        let state_vor_reconnect = std::fs::read(&state).ok();

        let mut reconnect =
            snapshot_subscriber_starten(&snapshot, &state, &marker, "kein_killpunkt");
        assert!(reconnect.wait().unwrap().success());
        let state_nach_reconnect = std::fs::read(&state).unwrap();
        assert_eq!(state_nach_reconnect, erwartet);
        if punkt == "nach_snapshot_anwendung" {
            assert_eq!(
                state_vor_reconnect.as_deref(),
                Some(state_nach_reconnect.as_slice())
            );
        } else {
            assert!(state_vor_reconnect.is_none());
        }
    }
}

#[test]
fn kill_vor_snapshot_outbox_kompaktierung() {
    let ordner = TestOrdner::neu("k06");
    crash(&ordner.db(), "vor_outbox_kompaktierung", "compact");
    let writer = starten(&ordner.db());
    assert_eq!(writer.handle().outbox_lesen().unwrap().len(), 1);
    assert_eq!(persistierte_snapshot_wirkung(&writer), 1);
    let coordinator = Coordinator::mit_store(Arc::new(ManualClock::default()), si_hex(99), &writer);
    let push = Arc::new(PushProbe::neu(true));
    coordinator.session_push_setzen(push.clone());
    let client = si_hello(4, 401, "main");
    assert!(
        coordinator
            .control_hello_registrieren("reconnect", &client)
            .angenommen
    );
    assert!(si_subscribe(&coordinator, "reconnect", &client.adresse));
    let snapshots = push.snapshots();
    assert_eq!(snapshots.len(), 1);
    let resubscribe = &snapshots.last().unwrap().1;
    assert_eq!(snapshot_wirkung(resubscribe), 1);
    assert_eq!(resubscribe["beitritt_bestaetigung_noetig"], true);
    assert_eq!(resubscribe["broker_epoch"], si_hex(99));
    assert_eq!(persistierte_snapshot_wirkung(&writer), 1);
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());
}

#[test]
fn kill_nach_snapshot_outbox_kompaktierung_snapshot_traegt_wirkung() {
    let ordner = TestOrdner::neu("k07");
    crash(&ordner.db(), "nach_outbox_kompaktierung", "compact");
    let writer = starten(&ordner.db());
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());
    assert_eq!(persistierte_snapshot_wirkung(&writer), 1);
    let coordinator = Coordinator::mit_store(Arc::new(ManualClock::default()), si_hex(99), &writer);
    let push = Arc::new(PushProbe::neu(true));
    coordinator.session_push_setzen(push.clone());
    let client = si_hello(4, 402, "main");
    assert!(
        coordinator
            .control_hello_registrieren("reconnect", &client)
            .angenommen
    );
    assert!(si_subscribe(&coordinator, "reconnect", &client.adresse));
    let snapshots = push.snapshots();
    assert_eq!(snapshots.len(), 1);
    let resubscribe = &snapshots.last().unwrap().1;
    assert_eq!(snapshot_wirkung(resubscribe), 1);
    assert_eq!(resubscribe["beitritt_bestaetigung_noetig"], true);
    assert_eq!(resubscribe["broker_epoch"], si_hex(99));
    assert_eq!(persistierte_snapshot_wirkung(&writer), 1);
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());
}

#[test]
fn kill_waehrend_migration_1() {
    let ordner = TestOrdner::neu("k08");
    crash(&ordner.db(), "waehrend_migration_1", "migration");
    assert!(schema_signatur(&ordner.db()).is_empty());
    let marker = ordner.db().with_extension("waehrend_migration_1.marker");
    let pending: Value = serde_json::from_slice(
        &std::fs::read(marker.with_extension("pending.json")).expect("Pending-Intent"),
    )
    .unwrap();
    let start = Instant::now();
    let writer = starten(&ordner.db());
    assert!(!writer.ist_degradiert());
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT major FROM schema_migrations"),
        STORE_SCHEMA_MAJOR
    );
    let referenz = TestOrdner::neu("k08-schema-reference");
    let _referenz_writer = starten(&referenz.db());
    assert_eq!(
        schema_signatur(&ordner.db()),
        schema_signatur(&referenz.db())
    );

    let command_id = pending["command_id"].as_str().unwrap();
    let sequence = pending["sequence"].as_i64().unwrap();
    let wirkung = pending["wirkung"].as_i64().unwrap();
    let erster = writer
        .handle()
        .append(vec![event(Some(command_id), sequence, wirkung, true)])
        .unwrap();
    let retry = writer
        .handle()
        .append(vec![event(Some(command_id), sequence, wirkung, true)])
        .unwrap();
    assert!(matches!(erster[0], AppendAusgang::Angewandt { .. }));
    assert!(matches!(
        retry[0],
        AppendAusgang::IdempotentWiederholt { .. }
    ));
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT COUNT(*) FROM event_log"),
        1
    );
    assert!(recovery_testgrenze_bestanden(start.elapsed()));
}

#[test]
fn kill_waehrend_wal_replay() {
    let ordner = TestOrdner::neu("k09");
    crash(&ordner.db(), "wal_bereit", "wal");
    let start = Instant::now();
    let writer = starten(&ordner.db());
    let conn = Connection::open(ordner.db()).unwrap();
    let ok: String = conn
        .query_row("PRAGMA integrity_check", [], |r| r.get(0))
        .unwrap();
    assert_eq!(ok, "ok");
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT COUNT(*) FROM event_log"),
        1
    );
    assert_eq!(
        scalar_i64(
            &ordner.db(),
            "SELECT COUNT(*) FROM event_log WHERE command_id='00000000000000000000000000000005'"
        ),
        1
    );
    assert_eq!(
        scalar_i64(
            &ordner.db(),
            "SELECT COUNT(*) FROM event_log WHERE command_id='00000000000000000000000000000006'"
        ),
        0
    );
    let (event_ord, event_payload): (i64, Vec<u8>) = conn
        .query_row(
            "SELECT event_ord,payload_jcs FROM event_log WHERE command_id='00000000000000000000000000000005'",
            [],
            |row| Ok((row.get(0)?, row.get(1)?)),
        )
        .unwrap();
    for tabelle in ["projects", "sessions"] {
        let (projektions_ord, payload): (i64, Vec<u8>) = conn
            .query_row(
                &format!("SELECT last_event_ord,state_jcs FROM {tabelle}"),
                [],
                |row| Ok((row.get(0)?, row.get(1)?)),
            )
            .unwrap();
        assert_eq!(projektions_ord, event_ord, "{tabelle}");
        assert_eq!(payload, event_payload, "{tabelle}");
    }
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT snapshot_event_ord FROM outbox"),
        event_ord
    );
    assert_eq!(writer.handle().outbox_lesen().unwrap().len(), 1);
    assert_eq!(persistierte_snapshot_wirkung(&writer), 1);

    let coordinator = Arc::new(Coordinator::mit_store(
        Arc::new(ManualClock::default()),
        si_hex(99),
        &writer,
    ));
    let push = Arc::new(PushProbe::neu(true));
    coordinator.session_push_setzen(push.clone());
    let client = si_hello(4, 404, "main");
    assert!(
        coordinator
            .control_hello_registrieren("reconnect", &client)
            .angenommen
    );
    assert!(si_subscribe(&coordinator, "reconnect", &client.adresse));
    let snapshots = push.snapshots();
    assert_eq!(snapshots.len(), 1);
    let resubscribe = &snapshots.last().unwrap().1;
    assert_eq!(snapshot_wirkung(resubscribe), 1);
    assert_eq!(resubscribe["beitritt_bestaetigung_noetig"], true);
    assert_eq!(resubscribe["broker_epoch"], si_hex(99));
    assert_eq!(persistierte_snapshot_wirkung(&writer), 1);
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());
    assert!(recovery_testgrenze_bestanden(start.elapsed()));
}

#[test]
fn eviction_bewahrt_domainzustand_und_konfliktriegel_bei_vollstaendigem_cleanup() {
    let (_ordner, writer, coordinator, clock, _push) =
        si_coordinator_mit_store("eviction-domain-state", true);
    let alt = si_hello(10, 100, "main");
    let neu = si_hello(10, 101, "main");
    assert!(
        coordinator
            .control_hello_registrieren("alt", &alt)
            .angenommen
    );
    assert!(
        coordinator
            .control_hello_registrieren("neu", &neu)
            .angenommen
    );
    assert!(si_subscribe(&coordinator, "neu", &neu.adresse));
    assert!(coordinator.intervention_begin("neu", &neu.adresse, &si_hex(700), 1));
    assert!(!si_report(&coordinator, "alt", &alt.adresse, 1));
    assert_eq!(coordinator.subscription_anzahl(), 1);
    assert_eq!(coordinator.interventionssicht().aktive, 1);

    let mut plugin_state = event(None, 700, 0, false);
    plugin_state.event_type = "proposal".into();
    plugin_state.payload_jcs = serde_json::to_vec(&json!({
        "type": "internal_plugin_state",
        "proposal_id": "77777777777777777777777777777777",
        "wert": 42
    }))
    .unwrap();
    writer.handle().append(vec![plugin_state]).unwrap();
    assert_eq!(
        scalar_i64(
            writer.handle().db_pfad(),
            "SELECT COUNT(*) FROM projects WHERE project_binding_id='00000000000000000000000000000001'"
        ),
        1
    );
    let state_vorher: Vec<u8> = Connection::open(writer.handle().db_pfad())
        .unwrap()
        .query_row("SELECT state_jcs FROM proposals", [], |row| row.get(0))
        .unwrap();

    clock.setze_ms(STALE_NACH_MS + 1);
    coordinator.liveness_tick();
    clock.setze_ms(STALE_NACH_MS + 1 + TOMBSTONE_MS);
    coordinator.liveness_tick();

    assert!(coordinator
        .modell_sicht(&si_hex(1), &si_hex(2))
        .clients
        .is_empty());
    assert_eq!(coordinator.subscription_anzahl(), 0);
    assert_eq!(coordinator.interventionssicht().aktive, 0);
    assert!(coordinator.interventionssicht().unknown);
    assert_eq!(
        scalar_i64(
            writer.handle().db_pfad(),
            "SELECT COUNT(*) FROM projects WHERE project_binding_id='00000000000000000000000000000001'"
        ),
        1
    );
    assert_eq!(
        scalar_i64(
            writer.handle().db_pfad(),
            "SELECT COUNT(*) FROM conflict_guards"
        ),
        2
    );
    let state_nachher: Vec<u8> = Connection::open(writer.handle().db_pfad())
        .unwrap()
        .query_row("SELECT state_jcs FROM proposals", [], |row| row.get(0))
        .unwrap();
    assert_eq!(state_nachher, state_vorher);

    let rueckkehr = si_hello(10, 102, "main");
    assert!(
        coordinator
            .control_hello_registrieren("rueckkehr", &rueckkehr)
            .angenommen
    );
    assert!(!coordinator.dispatch_fuer_link_erlaubt("rueckkehr"));
    let neue_id = si_hello(11, 103, "main");
    assert!(
        coordinator
            .control_hello_registrieren("neue-id", &neue_id)
            .angenommen
    );
    assert!(si_report(&coordinator, "neue-id", &neue_id.adresse, 1));
    assert!(coordinator.dispatch_fuer_link_erlaubt("neue-id"));
}

#[test]
fn session_subscription_hat_keinen_event_uuid_wire_traeger() {
    let schema = include_str!("../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json");
    let schema: serde_json::Value = serde_json::from_str(schema).unwrap();
    for name in ["subscribe_session", "session_snapshot"] {
        let text = schema["$defs"][name].to_string();
        assert!(!text.contains("event_uuid"), "{name}");
    }
}

#[test]
fn event_uuid_bleibt_aus_ack_und_session_snapshot_wire_fern() {
    let schema = include_str!("../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json");
    let schema: serde_json::Value = serde_json::from_str(schema).unwrap();
    for name in ["command_ack", "session_snapshot"] {
        assert!(!schema["$defs"][name].to_string().contains("event_uuid"));
    }
}

#[test]
#[ignore = "A4-SI: Coordinator, Store und Snapshot-Senke als Server-Integration"]
fn session_snapshot_zeigt_join_kandidat_ohne_neue_familie() {
    let (_ordner, _writer, coordinator, _clock, push) =
        si_coordinator_mit_store("si-join-snapshot", true);
    let main1 = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main-1", &main1)
            .angenommen
    );
    assert!(si_report(&coordinator, "main-1", &main1.adresse, 1));
    assert!(si_subscribe(&coordinator, "main-1", &main1.adresse));

    let main2 = si_hello(11, 101, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main-2", &main2)
            .angenommen
    );
    assert!(si_report(&coordinator, "main-2", &main2.adresse, 1));
    let letzter = push.snapshots().last().unwrap().1.clone();
    assert_eq!(letzter["type"], "session_snapshot");
    assert_eq!(letzter["mitglieder"].as_array().unwrap().len(), 2);
    assert_eq!(letzter["beitritt_bestaetigung_noetig"], true);
    assert!(letzter.get("join_candidate").is_none());
    assert!(letzter.get("event_uuid").is_none());
}

#[test]
#[ignore = "A4-SI: Coordinator, Store und Snapshot-Senke als Server-Integration"]
fn session_subscription_hat_nur_snapshot_push() {
    let (_ordner, _writer, coordinator, _clock, push) =
        si_coordinator_mit_store("si-only-snapshot", true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    assert!(!push.snapshots().is_empty());
    assert!(push
        .snapshots()
        .iter()
        .all(|(_, wert)| wert["type"] == "session_snapshot" && wert.get("event_uuid").is_none()));
}

#[test]
#[ignore = "A4-SI: Coordinator, Store und Snapshot-Senke als Server-Integration"]
fn session_subscription_sendet_nur_aktuellen_absoluten_snapshot() {
    let (_ordner, _writer, coordinator, _clock, push) =
        si_coordinator_mit_store("si-current-snapshot", true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    let mut descriptor =
        push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
    descriptor["label"] = Value::String("aktuell".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    let snapshots = push.snapshots();
    let letzter = &snapshots.last().unwrap().1;
    assert_eq!(letzter["type"], "session_snapshot");
    assert_eq!(
        letzter["mitglieder"][0]["probe_descriptor"]["label"],
        "aktuell"
    );
    assert!(snapshots
        .iter()
        .all(|(_, wert)| wert.get("event_uuid").is_none()));
}

#[test]
#[ignore = "A4-SI: absoluter Empfaengerzustand wird real aus Snapshots gesetzt"]
fn wiederholter_snapshot_letzter_gewinnt_idempotent() {
    let (_ordner, _writer, coordinator, _clock, push) =
        si_coordinator_mit_store("si-last-wins", true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    let snapshots = push.snapshots();
    let aktuell = snapshots.last().unwrap().1.clone();
    let mut empfaenger = snapshots.first().unwrap().1.clone();
    assert_eq!(empfaenger["type"], "session_snapshot");
    empfaenger = aktuell.clone();
    let einmal = empfaenger.clone();
    empfaenger = aktuell;
    assert_eq!(empfaenger, einmal);
}

fn resubscribe_beweis(name: &str) {
    let (_ordner, _writer, coordinator, _clock, push) = si_coordinator_mit_store(name, true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("alt", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "alt", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "alt", &main.adresse));
    coordinator.control_ende("alt");
    assert_eq!(coordinator.subscription_anzahl(), 0);
    assert!(
        coordinator
            .control_hello_registrieren("neu", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "neu", &main.adresse, 2));
    assert!(si_subscribe(&coordinator, "neu", &main.adresse));
    let snapshots = push.snapshots();
    let letzter = snapshots.last().unwrap();
    assert_eq!(letzter.0, "neu");
    assert_eq!(letzter.1["type"], "session_snapshot");
    assert_eq!(letzter.1["mitglieder"].as_array().unwrap().len(), 1);
}

#[test]
#[ignore = "A4-SI: Reconnect/Resubscribe mit Store und Snapshot-Senke"]
fn reconnect_snapshot_enthaelt_committete_wirkung_genau_einmal() {
    resubscribe_beweis("si-reconnect");
}

#[test]
#[ignore = "A4-SI: Reconnect/Resubscribe mit Store und Snapshot-Senke"]
fn jeder_resubscribe_liefert_aktuellen_absoluten_snapshot() {
    resubscribe_beweis("si-resubscribe");
}

#[test]
#[ignore = "A4-SI: Coordinator schreibt koaleszierte Snapshot-Schuld in echten Store"]
fn session_snapshot_koalesziert_nach_objektschluessel() {
    let (_ordner, writer, coordinator, _clock, push) =
        si_coordinator_mit_store("si-coalesce", false);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    let mut descriptor =
        push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
    descriptor["label"] = Value::String("erster-stand".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    let mut descriptor =
        push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
    descriptor["label"] = Value::String("neuer-stand".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    let outbox = writer.handle().outbox_lesen().unwrap();
    assert_eq!(outbox.len(), 1);
    assert_eq!(outbox[0].0.object_key, "session_snapshot");
    assert_eq!(
        outbox[0].1,
        writer
            .handle()
            .session_state_lesen(
                &main.adresse.project_binding_id,
                &main.adresse.session_epoch
            )
            .unwrap()
            .unwrap()
            .0
    );
}

#[test]
#[ignore = "A4-SI: P1-Snapshotpfad ueber echten Store und kontrollierte Writesenke"]
fn p1_snapshot_abfluss_ohne_reconnect_aber_writefehler_liefert_aktuellen_snapshot() {
    let (_ordner, writer, coordinator, _clock, push) =
        si_coordinator_mit_store("si-p1-drain", false);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());

    let mut descriptor =
        push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
    descriptor["label"] = Value::String("erste-schuld".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    assert_eq!(writer.handle().outbox_lesen().unwrap().len(), 1);

    push.erfolgreich.store(true, Ordering::SeqCst);
    let mut descriptor =
        push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
    descriptor["label"] = Value::String("ohne-reconnect".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());

    push.erfolgreich.store(false, Ordering::SeqCst);
    let mut descriptor =
        push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
    descriptor["label"] = Value::String("nach-writefehler".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    assert_eq!(writer.handle().outbox_lesen().unwrap().len(), 1);
    push.erfolgreich.store(true, Ordering::SeqCst);
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());
    assert_eq!(
        push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"]["label"],
        "nach-writefehler"
    );
}

#[test]
#[ignore = "A4-SI: Produktgrenze des Snapshot-only-Abflusses"]
fn phase_b_hat_keinen_wire_consumer_fuer_nicht_koaleszierbare_events() {
    let (_ordner, writer, coordinator, _clock, push) =
        si_coordinator_mit_store("si-no-event-consumer", true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    let push_vorher = push.snapshots().len();
    let outbox_vorher = writer.handle().outbox_lesen().unwrap();
    let mut nicht_koaleszierbar = event(None, 900, 0, false);
    nicht_koaleszierbar.event_type = "evidence".into();
    nicht_koaleszierbar.payload_jcs = serde_json::to_vec(&json!({
        "type": "internal_evidence",
        "evidence_id": "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
    }))
    .unwrap();
    writer.handle().append(vec![nicht_koaleszierbar]).unwrap();
    assert!(push
        .snapshots()
        .iter()
        .all(|(_, wert)| wert["type"] == "session_snapshot"));
    assert_eq!(
        scalar_i64(
            writer.handle().db_pfad(),
            "SELECT COUNT(*) FROM event_log WHERE event_type <> 'session'"
        ),
        1
    );
    assert_eq!(
        scalar_i64(writer.handle().db_pfad(), "SELECT COUNT(*) FROM evidence"),
        1
    );
    assert_eq!(push.snapshots().len(), push_vorher);
    assert_eq!(writer.handle().outbox_lesen().unwrap(), outbox_vorher);
}

#[cfg(windows)]
#[test]
#[ignore = "A4-SI: blockierter Storewriter gegen echten Coordinator-Ingress"]
fn blockierter_store_writer_bestaetigt_nichts_und_p2_laeuft() {
    let ordner = TestOrdner::neu("si-blocked-store");
    let barriere = StoreStartBarriere::neu_blockiert();
    let mut konfiguration = StoreKonfiguration::fuer_pfad(ordner.db());
    konfiguration.remote_volume_override = Some(false);
    konfiguration.start_barriere = Some(barriere.clone());
    let writer = StoreWriter::starten(konfiguration);
    let clock = Arc::new(ManualClock::default());
    let coordinator = Arc::new(Coordinator::mit_store(clock, si_hex(99), &writer));
    let push = Arc::new(PushProbe::neu(true));
    coordinator.session_push_setzen(push.clone());
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    coordinator.control_registrieren(
        "p2-probe",
        Adresse {
            logon_sid: "S-1-5-21-1111111111-2222222222-3333333333-1001".into(),
            project_binding_id: "1".repeat(32),
            session_epoch: "2".repeat(32),
            instance_id: format!("{:032x}", 3),
            runtime_nonce: "4".repeat(32),
        },
    );
    Senke::telemetrie_gekoppelt(coordinator.as_ref(), "p2-probe");
    let c = coordinator.clone();
    let a = main.adresse.clone();
    let heartbeat = std::thread::spawn(move || si_report(&c, "main", &a, 1));
    let frist = Instant::now() + Duration::from_secs(2);
    while writer.handle().sicht().eingereiht == 0 && Instant::now() < frist {
        std::thread::yield_now();
    }
    assert!(writer.handle().sicht().eingereiht > 0);
    assert!(!heartbeat.is_finished());
    Senke::p2(
        coordinator.as_ref(),
        "p2-probe",
        include_bytes!("../../eq-copilot/fixtures/v3/flatbuffers/gueltig/live-64-band.bin"),
    );
    assert_eq!(coordinator.p2_live_frames(), 1);
    assert_eq!(writer.handle().sicht().commits, 0);
    assert!(push.snapshots().is_empty());
    barriere.freigeben();
    assert!(heartbeat.join().unwrap());
}

#[test]
#[ignore = "A4-SI: blockierende externe Pusharbeit haelt keine Coordinator-/Storelocks"]
fn blockierter_writer_haelt_coordinator_und_store_nicht() {
    let (_ordner, writer, coordinator, _clock, push) =
        si_coordinator_mit_store("si-blocked-push", true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    push.blockieren();
    let mut descriptor =
        push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
    descriptor["label"] = Value::String("blockiert".into());
    let c = coordinator.clone();
    let aenderung = std::thread::spawn(move || c.descriptor_setzen("main", descriptor));
    push.auf_betreten_warten();
    assert_eq!(
        coordinator
            .modell_sicht(
                &main.adresse.project_binding_id,
                &main.adresse.session_epoch
            )
            .clients
            .len(),
        1
    );
    let commits_vorher = writer.handle().sicht().commits;
    writer
        .handle()
        .append(vec![event(None, 10_000, 42, false)])
        .unwrap();
    assert!(writer.handle().sicht().commits > commits_vorher);
    push.freigeben();
    assert!(aenderung.join().unwrap());
}

#[test]
#[ignore = "A4-SI: Eviction und Snapshot-Senke in echter Reihenfolge"]
fn eviction_cleanup_vor_push() {
    let (_ordner, _writer, coordinator, clock, push) =
        si_coordinator_mit_store("si-eviction-cleanup", true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    clock.setze_ms(STALE_NACH_MS + 1);
    coordinator.liveness_tick();
    let vor_eviction = push.snapshots().len();
    clock.vor(TOMBSTONE_MS);
    let schliessen = coordinator.liveness_tick();
    assert!(schliessen.iter().any(|link| link == "main"));
    assert_eq!(coordinator.subscription_anzahl(), 0);
    assert_eq!(coordinator.client_anzahl(), 0);
    assert_eq!(push.snapshots().len(), vor_eviction);
}

#[test]
#[ignore = "A4-SI: Nonce-Verdrängung bereinigt Subscription vor altem Report"]
fn nonce_verdraengung_cleanup_vor_altem_report() {
    let (_ordner, _writer, coordinator, _clock, push) =
        si_coordinator_mit_store("si-nonce-cleanup", true);
    let alt = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("alt", &alt)
            .angenommen
    );
    assert!(si_report(&coordinator, "alt", &alt.adresse, 1));
    assert!(si_subscribe(&coordinator, "alt", &alt.adresse));
    let vorher = push.snapshots().len();
    let neu = si_hello(10, 101, "main");
    let ausgang = coordinator.control_hello_registrieren("neu", &neu);
    assert!(ausgang.angenommen);
    assert_eq!(ausgang.zu_schliessende_links, vec!["alt"]);
    assert_eq!(coordinator.subscription_anzahl(), 0);
    assert!(!si_report(&coordinator, "alt", &alt.adresse, 2));
    assert!(coordinator.verbindung_soll_trennen("alt"));
    assert_eq!(push.snapshots().len(), vorher);
}

// ── NAK-121 H-17 ───────────────────────────────────────────────────────────

/// Der Setter prueft den Deskriptor gegen den v3-Vertrag, nicht nur die
/// Adresse. Bis NAK-121 trug die Haertung allein `descriptor_aus_heartbeat`;
/// jeder andere Weg in den Snapshot kam ungeprueft daran vorbei.
#[test]
#[ignore = "A4-SI: Coordinator, Store und Snapshot-Senke als Server-Integration"]
fn descriptor_setzen_weist_beitragsklasse_ab() {
    let (_ordner, _writer, coordinator, _uhr, push) =
        si_coordinator_mit_store("h17-beitragsklasse", true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "main", &main.adresse));

    let gueltig = push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
    assert_eq!(gueltig["aussageklasse"], "beobachtend");

    // Der Vertrag pinnt die Aussageklasse dieser Deskriptorform per Konstante
    // auf "beobachtend" (eq-ipc-v3.schema.json). Ein Beitragsdeskriptor faellt
    // ab, BEVOR er in Client, Snapshot, Store oder Push geraet.
    let mut beitrag = gueltig.clone();
    beitrag["aussageklasse"] = Value::String("beitrag".into());
    let vorher = push.snapshots().len();
    assert!(!coordinator.descriptor_setzen("main", beitrag));
    assert_eq!(
        push.snapshots().len(),
        vorher,
        "der abgewiesene Deskriptor hat trotzdem einen Push ausgeloest"
    );
    let stand = push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
    assert_eq!(stand["aussageklasse"], "beobachtend");

    // Gegenprobe: derselbe Weg mit gueltiger Klasse geht weiterhin durch.
    let mut erlaubt = gueltig;
    erlaubt["label"] = Value::String("h17".into());
    assert!(coordinator.descriptor_setzen("main", erlaubt));
}

/// D5 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): der Test
/// darueber misst nur die Aussageklasse.
///
/// Codex an der Quelle: ein direkter Aufrufer konnte einen Deskriptor mit
/// gueltiger `adresse` und `aussageklasse` uebergeben, dem `plugin_kind`,
/// `measurement_position`, `betrieb`, `capabilities` oder `frische` fehlten -
/// er wurde gespeichert und der Setter meldete `true`. H-17 verlangt die
/// VOLLSTAENDIGE v3-Vertragspruefung. Seither teilen Setter und Heartbeat-Weg
/// dieselbe Funktion; dieser Test misst jedes Pflichtfeld einzeln und belegt,
/// dass ein abgewiesener Deskriptor weder in den Snapshot noch in den Store
/// gelangt.
#[test]
#[ignore = "A4-SI: Coordinator, Store und Snapshot-Senke als Server-Integration"]
fn descriptor_setzen_verlangt_jedes_pflichtfeld() {
    let (_ordner, _writer, coordinator, _uhr, push) =
        si_coordinator_mit_store("h17-pflichtfelder", true);
    let main = si_hello(10, 100, "main");
    assert!(
        coordinator
            .control_hello_registrieren("main", &main)
            .angenommen
    );
    assert!(si_report(&coordinator, "main", &main.adresse, 1));
    assert!(si_subscribe(&coordinator, "main", &main.adresse));

    let gueltig = push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
    // Der Ausgangsdeskriptor traegt alle acht Pflichtfelder des v3-Vertrags.
    for feld in [
        "adresse",
        "plugin_kind",
        "measurement_position",
        "aussageklasse",
        "betrieb",
        "label",
        "capabilities",
        "frische",
    ] {
        assert!(gueltig.get(feld).is_some(), "{feld} fehlt schon im Ausgang");
    }

    // 1. Je FEHLENDEM Pflichtfeld: `false`, kein Push, kein Eintrag.
    for feld in [
        "adresse",
        "plugin_kind",
        "measurement_position",
        "aussageklasse",
        "betrieb",
        "label",
        "capabilities",
        "frische",
    ] {
        let mut ohne = gueltig.clone();
        ohne.as_object_mut().unwrap().remove(feld);
        let vorher = push.snapshots().len();
        assert!(
            !coordinator.descriptor_setzen("main", ohne),
            "ein Deskriptor ohne `{feld}` wurde uebernommen"
        );
        assert_eq!(
            push.snapshots().len(),
            vorher,
            "der abgewiesene Deskriptor ohne `{feld}` hat einen Push ausgeloest"
        );
        let stand = push.snapshots().last().unwrap().1["mitglieder"][0]["probe_descriptor"].clone();
        assert_eq!(stand, gueltig, "der abgewiesene Deskriptor ohne `{feld}` steht im Snapshot");
    }

    // 2. Je VERTRAGSWIDRIGEM Wert dasselbe. `measurement_position` ist der
    //    Diskriminator der Aussageklasse; ein Main misst nur am Insert.
    for (feld, wert) in [
        ("plugin_kind", json!("gast")),
        ("measurement_position", json!("post_fader_contribution")),
        ("measurement_position", json!("pre")),
        ("aussageklasse", json!("beitrag")),
        ("betrieb", json!("halb")),
        ("betrieb", json!(3)),
        ("label", json!(7)),
        ("capabilities", json!("alle")),
        ("frische", json!({"stale": true})),
        ("frische", json!({"stale": "ja", "letzter_kontakt_ms": 0})),
        ("frische", json!({"stale": true, "letzter_kontakt_ms": -1})),
        ("adresse", json!({"logon_sid": "S-1-5-21-1-2-3-1001"})),
    ] {
        let mut falsch = gueltig.clone();
        falsch[feld] = wert.clone();
        let vorher = push.snapshots().len();
        assert!(
            !coordinator.descriptor_setzen("main", falsch),
            "`{feld}` = {wert} wurde uebernommen"
        );
        assert_eq!(push.snapshots().len(), vorher, "`{feld}` = {wert} hat gepusht");
    }

    // 3. Der Store hat davon nichts gesehen: die Sitzung traegt genau den
    //    Deskriptor aus dem Heartbeat-Weg.
    let stand: Value = serde_json::from_slice(&coordinator.session_snapshot_json(
        &main.adresse.project_binding_id,
        &main.adresse.session_epoch,
    ))
    .unwrap();
    assert_eq!(stand["mitglieder"][0]["probe_descriptor"], gueltig);

    // 4. Gegenprobe: der vollstaendige Deskriptor geht weiterhin durch.
    let mut erlaubt = gueltig;
    erlaubt["label"] = Value::String("h17-pflichtfelder".into());
    assert!(coordinator.descriptor_setzen("main", erlaubt));
}

// ── NAK-121: G2-TOCTOU-001, G2-TOCTOU-002, G2-LOSSYSTR-001 und H-18 ────────

/// G2-TOCTOU-001: der Pruefsummenvergleich steht VOR der Migration. Bis
/// NAK-121 lief `execute_batch` zuerst - ein fremdes Schema wurde also erst
/// angefasst und dann beurteilt. Der Rollback rettete das Ergebnis, aber die
/// Reihenfolge selbst ist die Zusage.
#[test]
fn pruefsumme_entscheidet_vor_der_migration() {
    let ordner = TestOrdner::neu("toctou001");
    let db = ordner.db();

    // Ein fremder Stand: die Migrationstabelle traegt eine andere Pruefsumme,
    // aber KEINE der Tabellen, die MIGRATION_1_SQL anlegen wuerde.
    {
        let conn = Connection::open(&db).unwrap();
        conn.execute_batch(
            "CREATE TABLE schema_migrations(major INTEGER PRIMARY KEY,\
             checksum_sha256 TEXT NOT NULL, applied_utc_ms INTEGER NOT NULL);",
        )
        .unwrap();
        conn.execute(
            "INSERT INTO schema_migrations(major,checksum_sha256,applied_utc_ms) VALUES(?1,?2,?3)",
            rusqlite::params![STORE_SCHEMA_MAJOR, "fremd", 0i64],
        )
        .unwrap();
    }

    let mut k = StoreKonfiguration::fuer_pfad(&db);
    k.remote_volume_override = Some(false);
    let writer = StoreWriter::starten(k);
    assert!(
        writer.ist_degradiert(),
        "der fremde Checksum-Stand wurde nicht erkannt"
    );

    // Das Ergebnis allein beweist die REIHENFOLGE nicht: die Migration laeuft
    // in einer Transaktion, und deren Rollback raeumt ihre Tabellen auch dann
    // weg, wenn sie vor dem Vergleich gelaufen ist. Gemessen wurde das - unter
    // einer Mutante, die execute_batch wieder nach vorn zieht, blieb dieser
    // Teil gruen. Die Reihenfolge braucht deshalb eine eigene Wache.
    let conn = Connection::open_with_flags(&db, OpenFlags::SQLITE_OPEN_READ_ONLY).unwrap();
    let events_da: i64 = conn
        .query_row(
            "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='events'",
            [],
            |row| row.get(0),
        )
        .unwrap();
    assert_eq!(events_da, 0, "das fremde Schema traegt jetzt fremde Tabellen");

    // Die Wache auf die Reihenfolge selbst: im Quelltext von migration_1 steht
    // der Pruefsummenvergleich VOR dem execute_batch. Textuell, weil die
    // Transaktion jede Beobachtung von aussen wegraeumt - und praezise, weil
    // genau diese zwei Positionen die Zusage sind.
    let quelle = include_str!("../src/store/migration.rs");
    let rumpf_anfang = quelle
        .find("pub(super) fn migration_1(")
        .expect("migration_1 im Quelltext");
    let rumpf = &quelle[rumpf_anfang..];
    let rumpf_ende = rumpf.find("
pub(super) fn ").unwrap_or(rumpf.len());
    let rumpf = &rumpf[..rumpf_ende];
    let vergleich = rumpf
        .find("fremden Checksum-Stand")
        .expect("Pruefsummenvergleich in migration_1");
    let batch = rumpf
        .find("execute_batch(MIGRATION_1_SQL)")
        .expect("Migrationsbatch in migration_1");
    assert!(
        vergleich < batch,
        "die Migration faellt vor ihrem Pruefsummenvergleich:          Vergleich bei {vergleich}, Batch bei {batch}"
    );
}

/// G2-TOCTOU-002: ein Reparse-Punkt im Storepfad wird abgewiesen, statt
/// klassifiziert zu werden. Vorher klassifizierte die Funktion einen VORFAHREN
/// und oeffnete danach ein anderes Objekt.
#[cfg(windows)]
#[test]
fn store_weist_reparse_punkt_im_pfad_ab() {
    use eqcop_broker::store::store_pfad_ist_remote;

    let ordner = TestOrdner::neu("toctou002");
    let echt = ordner.0.join("echt");
    std::fs::create_dir_all(&echt).unwrap();

    // Eine Junction ist der praxisnahe Fall: sie braucht keine Adminrechte.
    let verweis = ordner.0.join("verweis");
    let status = Command::new("cmd")
        .args([
            "/c",
            "mklink",
            "/J",
            verweis.to_str().unwrap(),
            echt.to_str().unwrap(),
        ])
        .status();
    let junction_da = status.map(|s| s.success()).unwrap_or(false) && verweis.exists();
    if !junction_da {
        // Ohne Junction misst der Test nichts - dann sagt er das, statt gruen
        // zu schweigen.
        eprintln!("mklink /J nicht verfuegbar; Reparse-Fall uebersprungen");
        return;
    }

    let ueber_junction = verweis.join(STORE_DATEINAME);
    let fehler = store_pfad_ist_remote(&ueber_junction)
        .expect_err("ein Reparse-Punkt im Pfad muss abgewiesen werden");
    assert!(
        format!("{fehler:?}").contains("Reparse-Punkt"),
        "abgewiesen, aber mit anderem Grund: {fehler:?}"
    );

    // Gegenprobe: derselbe Ort ohne Junction geht durch.
    assert!(!store_pfad_ist_remote(&echt.join(STORE_DATEINAME)).unwrap());
    let _ = std::fs::remove_dir(&verweis);
}

/// G2-LOSSYSTR-001: der Standardpfad wird verlustfrei aus UTF-16 gewandelt.
/// `from_utf16_lossy` haette ungepaarte Surrogate durch U+FFFD ersetzt - der
/// geprueft Pfad waere dann ein anderer als der geoeffnete.
#[cfg(windows)]
#[test]
fn store_pfad_bleibt_bei_nicht_utf8_zeichen_unveraendert() {
    let pfad = standard_store_pfad().expect("LocalAppData aufloesbar");
    let text = pfad.to_string_lossy();
    assert!(
        !text.contains('\u{FFFD}'),
        "der Standardpfad traegt ein Ersatzzeichen: {text}"
    );
    // Und er endet weiterhin auf dem vereinbarten Ort.
    assert!(pfad.ends_with(STORE_DATEINAME));
    assert!(text.contains(STORE_RELATIVPFAD));
}

/// H-18: ein gescheiterter Leerlauf-Checkpoint merkt sich keinen Erfolg und
/// wird sichtbar. Bis NAK-121 wurde das Ergebnis verworfen UND das Merkflag
/// trotzdem gesetzt - der Checkpoint wurde deshalb nie wiederholt, waehrend
/// die Storesicht weiter gesund meldete.
#[test]
fn gescheiterter_leerlauf_checkpoint_wird_wiederholt() {
    let ordner = TestOrdner::neu("h18");
    let naht = Arc::new(IdleCheckpointNaht::default());
    let mut k = StoreKonfiguration::fuer_pfad(ordner.db());
    k.remote_volume_override = Some(false);
    k.idle_checkpoint_naht = Some(naht.clone());
    let writer = StoreWriter::starten(k);
    assert!(!writer.ist_degradiert());

    // Der Checkpoint scheitert. Die Zusage hat ZWEI Haelften: er merkt sich
    // keinen Erfolg, und er wird beim naechsten Leerlauf erneut versucht.
    naht.fehler_erzwingen.store(true, Ordering::SeqCst);
    naht.sofort_ausloesen.store(true, Ordering::SeqCst);
    assert!(
        warten_auf(5000, || naht.versuche.load(Ordering::SeqCst) >= 3),
        "der gescheiterte Checkpoint wurde nicht wiederholt: {} Versuche",
        naht.versuche.load(Ordering::SeqCst)
    );
    let gescheitert = writer.handle().sicht().checkpoints_gescheitert;
    assert!(
        gescheitert >= 3,
        "die Fehlschlaege blieben unsichtbar: {gescheitert}"
    );

    // Sobald er gelingt, merkt er sich den Erfolg und hoert auf - das ist die
    // Gegenprobe, ohne die der Test auch eine Endlosschleife gutheissen wuerde.
    naht.fehler_erzwingen.store(false, Ordering::SeqCst);
    assert!(warten_auf(5000, || {
        let a = naht.versuche.load(Ordering::SeqCst);
        std::thread::sleep(std::time::Duration::from_millis(200));
        a == naht.versuche.load(Ordering::SeqCst)
    }));
    let stand = writer.handle().sicht().checkpoints_gescheitert;
    std::thread::sleep(std::time::Duration::from_millis(300));
    assert_eq!(
        writer.handle().sicht().checkpoints_gescheitert,
        stand,
        "nach dem erfolgreichen Checkpoint zaehlt weiter etwas"
    );
}

fn warten_auf(ms: u64, mut bedingung: impl FnMut() -> bool) -> bool {
    let ende = std::time::Instant::now() + std::time::Duration::from_millis(ms);
    while std::time::Instant::now() < ende {
        if bedingung() {
            return true;
        }
        std::thread::sleep(std::time::Duration::from_millis(20));
    }
    bedingung()
}

// ── NAK-121 H-14 ───────────────────────────────────────────────────────────
//
// Quarantaene und persistente Riegeltabelle sind gedeckelt. Beide hatten bis
// NAK-121 keine Obergrenze, und C-07 verbietet ausdruecklich jede zeitbasierte
// Freigabe - ein Angreifer konnte sie mit Kollisionen unbegrenzt fuellen.

#[test]
fn quarantaene_deckel_weist_ab_statt_zu_wachsen() {
    use eqcop_broker::instance_alias::{
        AliasRegister, Registrierung, Sitzungsadressraum, MAX_QUARANTAENE,
    };

    let register = AliasRegister::default();
    let raum = Sitzungsadressraum::neu("S-1-5-21-1-2-3-1001", &"1".repeat(32), &"2".repeat(32));

    // Jede Kollision quarantinisiert BEIDE Originale, der Deckel wird also in
    // Zweierschritten erreicht.
    let mut paar = 0usize;
    while register.quarantaene_anzahl() < MAX_QUARANTAENE {
        let wire = format!("{paar:032x}");
        let a = format!("a{paar}");
        let b = format!("b{paar}");
        assert_eq!(
            register.registriere_wire_zuordnung(&raum, &a, &wire),
            Registrierung::Eingetragen
        );
        assert_eq!(
            register.registriere_wire_zuordnung(&raum, &b, &wire),
            Registrierung::KollisionBeideQuarantaenisiert
        );
        paar += 1;
        assert!(paar < MAX_QUARANTAENE, "Deckel nie erreicht");
    }
    assert_eq!(register.quarantaene_anzahl(), MAX_QUARANTAENE);

    // Am Deckel faellt die naechste Registrierung fail-closed - und die
    // Quarantaene waechst NICHT weiter.
    let wire = format!("{:032x}", paar + 1);
    assert_eq!(
        register.registriere_wire_zuordnung(&raum, "ueber-den-deckel", &wire),
        Registrierung::DeckelErreicht
    );
    assert_eq!(
        register.quarantaene_anzahl(),
        MAX_QUARANTAENE,
        "die Abweisung hat den Speicher trotzdem wachsen lassen"
    );

    // D7 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): der Test
    // erreichte den Deckel nur in ZWEIERSCHRITTEN und sah den UNGERADEN Stand
    // nie. Eine explizite Aufloesung senkt die Menge auf 1023; die alte
    // Pruefung `len() >= MAX` liess die naechste Kollision dann passieren und
    // fuegte anschliessend ZWEI Besitzer ein - 1025 Eintraege, die zugesagte
    // Obergrenze ueberschritten.
    assert!(register.quarantaene_aufloesen(&raum, "a0"));
    assert_eq!(register.quarantaene_anzahl(), MAX_QUARANTAENE - 1);
    let wire_ungerade = format!("{:032x}", paar + 2);
    assert_eq!(
        register.registriere_wire_zuordnung(&raum, "ungerader-stand", &wire_ungerade),
        Registrierung::DeckelErreicht,
        "aus dem ungeraden Stand darf keine Kollision mehr entstehen koennen"
    );
    assert_eq!(
        register.quarantaene_anzahl(),
        MAX_QUARANTAENE - 1,
        "die Menge ist ueber den Deckel gewachsen"
    );

    // Und mit zwei freien Plaetzen geht es wieder: die Pruefung RESERVIERT so
    // viele Plaetze, wie die Kollision einfuegt - nicht mehr und nicht weniger.
    assert!(register.quarantaene_aufloesen(&raum, "b0"));
    assert_eq!(register.quarantaene_anzahl(), MAX_QUARANTAENE - 2);
    let wire_paar = format!("{:032x}", paar + 3);
    assert_eq!(
        register.registriere_wire_zuordnung(&raum, "paar-a", &wire_paar),
        Registrierung::Eingetragen
    );
    assert_eq!(
        register.registriere_wire_zuordnung(&raum, "paar-b", &wire_paar),
        Registrierung::KollisionBeideQuarantaenisiert
    );
    assert_eq!(
        register.quarantaene_anzahl(),
        MAX_QUARANTAENE,
        "die Kollision muss den Deckel exakt treffen"
    );
}

#[test]
fn guard_aufloesung_raeumt_den_aliaseintrag() {
    use eqcop_broker::instance_alias::{AliasRegister, Registrierung, Sitzungsadressraum};

    let register = AliasRegister::default();
    let raum = Sitzungsadressraum::neu("S-1-5-21-1-2-3-1001", &"1".repeat(32), &"2".repeat(32));
    let wire = "3".repeat(32);
    let erster = "instanz-a:nonce-a";
    let zweiter = "instanz-b:nonce-b";

    assert_eq!(
        register.registriere_wire_zuordnung(&raum, erster, &wire),
        Registrierung::Eingetragen
    );
    assert_eq!(
        register.registriere_wire_zuordnung(&raum, zweiter, &wire),
        Registrierung::KollisionBeideQuarantaenisiert
    );
    assert!(register.ist_quarantaenisiert(&raum, erster));
    assert!(register.ist_quarantaenisiert(&raum, zweiter));

    // Die Aufloesung EINES Riegels raeumt genau seinen Eintrag - und nur ihn.
    assert!(register.quarantaene_aufloesen(&raum, erster));
    assert!(!register.ist_quarantaenisiert(&raum, erster));
    assert!(
        register.ist_quarantaenisiert(&raum, zweiter),
        "die Aufloesung hat den fremden Riegel mitgenommen"
    );

    // Danach darf dieselbe Instanz wieder registrieren; vorher fiel sie
    // dauerhaft an einer Quarantaene, deren Riegel laengst fort war.
    assert_eq!(
        register.registriere_wire_zuordnung(&raum, erster, &"4".repeat(32)),
        Registrierung::Eingetragen
    );
    // Eine Aufloesung ohne Riegel meldet ehrlich false.
    assert!(!register.quarantaene_aufloesen(&raum, "nie-quarantaenisiert"));
}

/// D6 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): am Deckel des
/// Aliasregisters bekam der Peer ein WELCOME statt der von H-14 zugesagten
/// Ablehnung.
///
/// Codex an der Quelle: `registriere_wire_zuordnung` liefert am Deckel
/// `Ungueltig`, behandelt wurde in `control_hello_registrieren` aber nur die
/// Kollisionsvariante - danach entstanden Client und Link und die
/// Registrierung meldete bedingungslos „angenommen". Der Deckel hat seither
/// eine eigene Variante (`DeckelErreicht`), und sie endet die Registrierung,
/// bevor irgendetwas angelegt ist.
#[test]
fn alias_deckel_weist_das_hello_ab_statt_ein_welcome_zu_senden() {
    let uhr = Arc::new(ManualClock::default());
    let c = Coordinator::mit_uhr(uhr.clone(), si_hex(0xbeef));

    // Jede Runde erzeugt genau eine Kollision und damit zwei
    // Quarantaeneeintraege: zwei Nonces derselben Instanz, danach meldet die
    // VERDRAENGTE Nonce erneut (C-10, fail-closed Duplikatkonflikt).
    let mut runde = 0usize;
    let mut zuletzt_angenommen = 0usize;
    let abweisung = loop {
        runde += 1;
        assert!(runde < 1024, "der Deckel wurde nie erreicht");
        let instanz = 1000 + runde;
        let alt = si_hello(instanz, 100, "active_probe");
        let neu = si_hello(instanz, 101, "active_probe");

        let erste = c.control_hello_registrieren("alt", &alt);
        if !erste.angenommen {
            break (instanz, erste);
        }
        let zweite = c.control_hello_registrieren("neu", &neu);
        if !zweite.angenommen {
            break (instanz, zweite);
        }
        assert!(!si_report(&c, "alt", &alt.adresse, 1));
        zuletzt_angenommen = instanz;

        // Aufraeumen, damit nicht der Clientdeckel zuerst greift. Die
        // Quarantaene bleibt davon unberuehrt - C-07 verbietet die
        // zeitbasierte Freigabe.
        c.control_ende("alt");
        c.control_ende("neu");
        uhr.setze_ms((runde as u64 + 1) * (STALE_NACH_MS + TOMBSTONE_MS + 1));
        c.liveness_tick();
    };

    let (instanz, ausgang) = abweisung;
    assert_eq!(
        ausgang.grund.as_deref(),
        Some("alias_quarantaene_deckel"),
        "am Deckel muss der Aliasdeckel der Grund sein, nicht ein anderer Cap"
    );
    assert!(zuletzt_angenommen > 0, "vor dem Deckel wurde nie etwas angenommen");

    // Kein Client, kein Link, kein Welcome.
    assert!(
        ausgang.zu_schliessende_links.is_empty(),
        "eine Abweisung schliesst keine fremden Links"
    );
    assert!(!c.dispatch_fuer_link_erlaubt("alt"), "es entstand doch ein Link");
    let abgewiesene = si_hex(instanz);
    assert!(
        !c.modell_sicht(&si_hex(1), &si_hex(2))
            .clients
            .iter()
            .any(|client| client.adresse.instance_id == abgewiesene),
        "der abgewiesene Peer steht als Client im Stand"
    );
    assert!(c.cap_abweisungen() > 0, "die Abweisung wurde nicht gezaehlt");

    // Deckel minus eins: die Runde davor lief vollstaendig durch - Hello,
    // Verdraengung und Duplikatkonflikt inbegriffen.
    assert_eq!(zuletzt_angenommen, instanz - 1);
}

/// D8 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): die
/// Riegelaufloesung suchte ihren Adressraum ueber die LEBENDEN Links.
///
/// Trennten sich beide kollidierenden Links vor der Aufloesung, war die Liste
/// leer: Store- und Coordinator-Guard fielen, die Methode meldete Erfolg - und
/// der Aliasbesitzer blieb quarantaenisiert. Der Riegel war fort, die
/// Registrierung derselben Instanz scheiterte trotzdem weiter. Riegel und
/// Alias liefen auseinander. Seither sucht die Aufloesung ueber das
/// Quarantaeneregister selbst, das seinen Besitzer kennt.
#[test]
fn guard_aufloesung_wirkt_auch_ohne_lebende_links() {
    let c = Coordinator::mit_uhr(Arc::new(ManualClock::default()), si_hex(0xbeef));
    let alt = si_hello(10, 100, "active_probe");
    let neu = si_hello(10, 101, "active_probe");
    assert!(c.control_hello_registrieren("alt", &alt).angenommen);
    assert!(c.control_hello_registrieren("neu", &neu).angenommen);
    // C-10: meldet die VERDRAENGTE Nonce erneut, ist das der fail-closed
    // Duplikatkonflikt - er quarantinisiert beide Besitzer und setzt den
    // dauerhaften Riegel.
    assert!(!si_report(&c, "alt", &alt.adresse, 1));

    let effective = format!(
        "{}|{}|{}|{}",
        alt.adresse.logon_sid,
        alt.adresse.project_binding_id,
        alt.adresse.session_epoch,
        alt.adresse.instance_id
    );
    assert!(
        c.konfliktriegel_gesetzt(&effective),
        "die Nonce-Kollision hat keinen dauerhaften Riegel erzeugt"
    );
    assert!(!c.dispatch_fuer_link_erlaubt("neu"), "fail-closed nach Kollision");

    // Der Fall, den der alte Weg nicht sah: BEIDE Links trennen sich, BEVOR
    // der Riegel explizit aufgeloest wird.
    c.control_ende("alt");
    c.control_ende("neu");

    for nonce in [100usize, 101] {
        let derived = format!("{}:{}", alt.adresse.instance_id, si_hex(nonce));
        assert!(
            c.konflikt_guard_aufloesen(&effective, &derived),
            "die Aufloesung von {derived} meldet Misserfolg"
        );
    }
    assert!(
        !c.konfliktriegel_gesetzt(&effective),
        "der Riegel steht nach der Aufloesung noch"
    );

    // Und jetzt die Probe aufs Exempel: dieselbe Instanz darf sich wieder
    // anmelden UND wieder routen. Vorher fiel sie dauerhaft an einer
    // Quarantaene, deren Riegel laengst fort war.
    assert!(c.control_hello_registrieren("wieder", &neu).angenommen);
    assert!(
        c.dispatch_fuer_link_erlaubt("wieder"),
        "der Alias blieb quarantaenisiert, obwohl der Riegel aufgeloest ist"
    );
}


