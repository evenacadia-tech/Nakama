use eqcop_broker::coordinator::{
    Coordinator, ManualClock, SessionPush, STALE_NACH_MS, TOMBSTONE_MS,
};
use eqcop_broker::store::{
    busy_timeout_abgelaufen, checkpoint_ausloesen, commit_ausloesen, migration_1_checksum,
    projektionen_neu_bauen, recovery_testgrenze_bestanden, standard_store_pfad, store_pfad_unter,
    AppendAusgang, ConflictGuard, SnapshotZiel, StoreEvent, StoreFehler, StoreKonfiguration,
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
        .unwrap_or(0)
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
            "zaehler": {}
        })),
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

    // Zuerst fliesst wirklich der AELTERE Projektionsschnitt aus dem Store.
    assert!(si_subscribe(&coordinator, "probe", &client.adresse));
    assert_eq!(snapshot_wirkung(&push.snapshots().last().unwrap().1), 1);

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
    assert_eq!(snapshot_wirkung(&push.snapshots().last().unwrap().1), 1);
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
    assert_eq!(snapshot_wirkung(&push.snapshots().last().unwrap().1), 1);
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());
}

#[test]
fn kill_nach_snapshot_outbox_kompaktierung_snapshot_traegt_wirkung() {
    let ordner = TestOrdner::neu("k07");
    crash(&ordner.db(), "nach_outbox_kompaktierung", "compact");
    let writer = starten(&ordner.db());
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());
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
    assert_eq!(snapshot_wirkung(&push.snapshots().last().unwrap().1), 1);
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());
}

#[test]
fn kill_waehrend_migration_1() {
    let ordner = TestOrdner::neu("k08");
    crash(&ordner.db(), "waehrend_migration_1", "migration");
    let start = Instant::now();
    let writer = starten(&ordner.db());
    assert!(!writer.ist_degradiert());
    assert_eq!(
        scalar_i64(&ordner.db(), "SELECT major FROM schema_migrations"),
        STORE_SCHEMA_MAJOR
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
    assert_eq!(writer.handle().outbox_lesen().unwrap().len(), 1);
    assert!(recovery_testgrenze_bestanden(start.elapsed()));
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
    let mut descriptor = push.snapshots().last().unwrap().1["mitglieder"][0].clone();
    descriptor["label"] = Value::String("aktuell".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    let snapshots = push.snapshots();
    let letzter = &snapshots.last().unwrap().1;
    assert_eq!(letzter["type"], "session_snapshot");
    assert_eq!(letzter["mitglieder"][0]["label"], "aktuell");
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
    let mut descriptor = push.snapshots().last().unwrap().1["mitglieder"][0].clone();
    descriptor["label"] = Value::String("erster-stand".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    let mut descriptor = push.snapshots().last().unwrap().1["mitglieder"][0].clone();
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

    let mut descriptor = push.snapshots().last().unwrap().1["mitglieder"][0].clone();
    descriptor["label"] = Value::String("erste-schuld".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    assert_eq!(writer.handle().outbox_lesen().unwrap().len(), 1);

    push.erfolgreich.store(true, Ordering::SeqCst);
    let mut descriptor = push.snapshots().last().unwrap().1["mitglieder"][0].clone();
    descriptor["label"] = Value::String("ohne-reconnect".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());

    push.erfolgreich.store(false, Ordering::SeqCst);
    let mut descriptor = push.snapshots().last().unwrap().1["mitglieder"][0].clone();
    descriptor["label"] = Value::String("nach-writefehler".into());
    assert!(coordinator.descriptor_setzen("main", descriptor));
    assert_eq!(writer.handle().outbox_lesen().unwrap().len(), 1);
    push.erfolgreich.store(true, Ordering::SeqCst);
    assert!(si_subscribe(&coordinator, "main", &main.adresse));
    assert!(writer.handle().outbox_lesen().unwrap().is_empty());
    assert_eq!(
        push.snapshots().last().unwrap().1["mitglieder"][0]["label"],
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
    assert!(push
        .snapshots()
        .iter()
        .all(|(_, wert)| wert["type"] == "session_snapshot"));
    assert_eq!(
        scalar_i64(
            writer.handle().db_pfad(),
            "SELECT COUNT(*) FROM event_log WHERE event_type <> 'session'"
        ),
        0
    );
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
    let c = coordinator.clone();
    let a = main.adresse.clone();
    let heartbeat = std::thread::spawn(move || si_report(&c, "main", &a, 1));
    let frist = Instant::now() + Duration::from_secs(2);
    while writer.handle().sicht().eingereiht == 0 && Instant::now() < frist {
        std::thread::yield_now();
    }
    assert!(writer.handle().sicht().eingereiht > 0);
    assert!(!heartbeat.is_finished());
    Senke::p2(coordinator.as_ref(), "main", b"live");
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
    let mut descriptor = push.snapshots().last().unwrap().1["mitglieder"][0].clone();
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
