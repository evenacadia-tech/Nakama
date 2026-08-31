//! SONDE-011 Phase B: lokale, append-only SQLite-Wahrheit des Brokers.
//!
//! Die Besitzgrenze ist absichtlich hart: genau der `StoreWriter`-Thread
//! besitzt die einzige Read/Write-Connection. Coordinator und I/O-Threads
//! reichen begrenzte Auftraege ein und warten nur ausserhalb ihrer eigenen
//! Sperren auf das Commit-Ergebnis. Leser oeffnen kurze Read-only-Snapshots;
//! keine API gibt eine SQLite-Transaktion an fremde Arbeit weiter.

use rusqlite::{
    params, Connection, OpenFlags, OptionalExtension, Transaction, TransactionBehavior,
};
use sha2::{Digest, Sha256};
use std::collections::VecDeque;
use std::fmt;
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::mpsc::{self, Receiver, RecvTimeoutError, SyncSender, TrySendError};
use std::sync::{Arc, Condvar, Mutex};
use std::thread::JoinHandle;
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

pub const RUSQLITE_VERSION: &str = "0.40.2";
pub const STORE_SCHEMA_MAJOR: i64 = 1;
pub const STORE_RELATIVPFAD: &str = r"evenacadia\nakama-broker";
pub const STORE_DATEINAME: &str = "nakama-broker.sqlite3";
pub const STORE_RECOVERY_TEST_MAX_MS: u64 = 5000;
pub const STORE_KANAL_CAP: usize = 256;
pub const BUSY_TIMEOUT_MS: u64 = 2000;
pub const COMMIT_FENSTER_MS: u64 = 50;
pub const COMMIT_BATCH_MAX: usize = 64;
pub const WAL_SCHWELLE_BYTES: u64 = 4 * 1024 * 1024;
pub const STORE_IDLE_MS: u64 = 5000;
pub const CHECKPOINT_BUSY_ERWARTET: i64 = 0;
pub const AUDIO_THREAD_STORE_WAIT_MS_MAX: u64 = 0;

const MIGRATION_1_SQL: &str = r#"
CREATE TABLE IF NOT EXISTS schema_migrations (
    major INTEGER PRIMARY KEY NOT NULL,
    checksum_sha256 TEXT NOT NULL,
    applied_utc_ms INTEGER NOT NULL
);
CREATE TABLE IF NOT EXISTS conflict_guards (
    effective_address TEXT NOT NULL,
    derived_id TEXT NOT NULL,
    created_utc_ms INTEGER NOT NULL,
    PRIMARY KEY (effective_address, derived_id)
);
CREATE TABLE IF NOT EXISTS event_log (
    event_ord INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    event_uuid TEXT NOT NULL UNIQUE,
    command_id TEXT UNIQUE,
    project_binding_id TEXT NOT NULL,
    session_epoch TEXT NOT NULL,
    utc_ms INTEGER NOT NULL,
    broker_epoch TEXT NOT NULL,
    sequence INTEGER NOT NULL,
    event_type TEXT NOT NULL,
    schema_major INTEGER NOT NULL,
    schema_minor INTEGER NOT NULL,
    payload_jcs BLOB NOT NULL,
    UNIQUE (broker_epoch, sequence)
);
CREATE INDEX IF NOT EXISTS event_log_session_ord
    ON event_log(project_binding_id, session_epoch, event_ord);
CREATE TRIGGER IF NOT EXISTS event_log_append_only_update
BEFORE UPDATE ON event_log BEGIN
    SELECT RAISE(ABORT, 'event_log ist append-only');
END;
CREATE TRIGGER IF NOT EXISTS event_log_append_only_delete
BEFORE DELETE ON event_log BEGIN
    SELECT RAISE(ABORT, 'event_log ist append-only');
END;
CREATE TABLE IF NOT EXISTS projects (
    project_binding_id TEXT PRIMARY KEY NOT NULL,
    last_event_ord INTEGER NOT NULL REFERENCES event_log(event_ord),
    state_jcs BLOB NOT NULL
);
CREATE TABLE IF NOT EXISTS sessions (
    project_binding_id TEXT NOT NULL,
    session_epoch TEXT NOT NULL,
    last_event_ord INTEGER NOT NULL REFERENCES event_log(event_ord),
    state_jcs BLOB NOT NULL,
    PRIMARY KEY (project_binding_id, session_epoch)
);
CREATE TABLE IF NOT EXISTS passages (
    passage_id TEXT PRIMARY KEY NOT NULL,
    project_binding_id TEXT NOT NULL,
    session_epoch TEXT NOT NULL,
    last_event_ord INTEGER NOT NULL REFERENCES event_log(event_ord),
    state_jcs BLOB NOT NULL
);
CREATE INDEX IF NOT EXISTS passages_session_ord
    ON passages(project_binding_id, session_epoch, last_event_ord);
CREATE TABLE IF NOT EXISTS evidence (
    evidence_id TEXT PRIMARY KEY NOT NULL,
    project_binding_id TEXT NOT NULL,
    session_epoch TEXT NOT NULL,
    last_event_ord INTEGER NOT NULL REFERENCES event_log(event_ord),
    state_jcs BLOB NOT NULL
);
CREATE INDEX IF NOT EXISTS evidence_session_ord
    ON evidence(project_binding_id, session_epoch, last_event_ord);
CREATE TABLE IF NOT EXISTS findings (
    finding_id TEXT PRIMARY KEY NOT NULL,
    project_binding_id TEXT NOT NULL,
    session_epoch TEXT NOT NULL,
    last_event_ord INTEGER NOT NULL REFERENCES event_log(event_ord),
    state_jcs BLOB NOT NULL
);
CREATE INDEX IF NOT EXISTS findings_session_ord
    ON findings(project_binding_id, session_epoch, last_event_ord);
CREATE TABLE IF NOT EXISTS proposals (
    proposal_id TEXT PRIMARY KEY NOT NULL,
    project_binding_id TEXT NOT NULL,
    session_epoch TEXT NOT NULL,
    last_event_ord INTEGER NOT NULL REFERENCES event_log(event_ord),
    state_jcs BLOB NOT NULL
);
CREATE INDEX IF NOT EXISTS proposals_session_ord
    ON proposals(project_binding_id, session_epoch, last_event_ord);
CREATE TABLE IF NOT EXISTS transactions (
    transaction_id TEXT PRIMARY KEY NOT NULL,
    project_binding_id TEXT NOT NULL,
    session_epoch TEXT NOT NULL,
    last_event_ord INTEGER NOT NULL REFERENCES event_log(event_ord),
    state_jcs BLOB NOT NULL
);
CREATE INDEX IF NOT EXISTS transactions_session_ord
    ON transactions(project_binding_id, session_epoch, last_event_ord);
CREATE TABLE IF NOT EXISTS experiments (
    experiment_id TEXT PRIMARY KEY NOT NULL,
    project_binding_id TEXT NOT NULL,
    session_epoch TEXT NOT NULL,
    last_event_ord INTEGER NOT NULL REFERENCES event_log(event_ord),
    state_jcs BLOB NOT NULL
);
CREATE INDEX IF NOT EXISTS experiments_session_ord
    ON experiments(project_binding_id, session_epoch, last_event_ord);
CREATE TABLE IF NOT EXISTS user_verdicts (
    user_verdict_id TEXT PRIMARY KEY NOT NULL,
    project_binding_id TEXT NOT NULL,
    session_epoch TEXT NOT NULL,
    last_event_ord INTEGER NOT NULL REFERENCES event_log(event_ord),
    state_jcs BLOB NOT NULL
);
CREATE INDEX IF NOT EXISTS user_verdicts_session_ord
    ON user_verdicts(project_binding_id, session_epoch, last_event_ord);
CREATE TABLE IF NOT EXISTS experiment_events (
    experiment_id TEXT NOT NULL REFERENCES experiments(experiment_id),
    event_uuid TEXT NOT NULL REFERENCES event_log(event_uuid),
    PRIMARY KEY (experiment_id, event_uuid)
);
CREATE TABLE IF NOT EXISTS outbox (
    target_project_binding_id TEXT NOT NULL,
    target_session_epoch TEXT NOT NULL,
    target_instance_id TEXT NOT NULL,
    object_key TEXT NOT NULL,
    snapshot_event_ord INTEGER NOT NULL REFERENCES event_log(event_ord),
    write_attempts INTEGER NOT NULL,
    PRIMARY KEY (
        target_project_binding_id,
        target_session_epoch,
        target_instance_id,
        object_key
    )
);
"#;

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum StoreFehler {
    KanalVoll,
    Beendet,
    Degradiert(String),
    Sqlite(String),
    Pfad(String),
}

impl fmt::Display for StoreFehler {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::KanalVoll => write!(f, "Storekanal voll"),
            Self::Beendet => write!(f, "Store-Writer beendet"),
            Self::Degradiert(g) => write!(f, "Store degradiert: {g}"),
            Self::Sqlite(g) => write!(f, "SQLite: {g}"),
            Self::Pfad(g) => write!(f, "Storepfad: {g}"),
        }
    }
}

impl std::error::Error for StoreFehler {}

impl From<rusqlite::Error> for StoreFehler {
    fn from(value: rusqlite::Error) -> Self {
        Self::Sqlite(value.to_string())
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ConflictGuard {
    pub effective_address: String,
    pub derived_id: String,
    pub created_utc_ms: i64,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SnapshotZiel {
    pub project_binding_id: String,
    pub session_epoch: String,
    pub instance_id: String,
    pub object_key: String,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StoreEvent {
    pub event_uuid: Option<String>,
    pub command_id: Option<String>,
    pub project_binding_id: String,
    pub session_epoch: String,
    pub utc_ms: i64,
    pub broker_epoch: String,
    pub sequence: i64,
    pub event_type: String,
    pub schema_major: i64,
    pub schema_minor: i64,
    pub payload_jcs: Vec<u8>,
    pub snapshot_ziele: Vec<SnapshotZiel>,
}

impl StoreEvent {
    pub fn session_snapshot(
        project_binding_id: impl Into<String>,
        session_epoch: impl Into<String>,
        broker_epoch: impl Into<String>,
        sequence: i64,
        payload_jcs: Vec<u8>,
    ) -> Self {
        Self {
            event_uuid: None,
            command_id: None,
            project_binding_id: project_binding_id.into(),
            session_epoch: session_epoch.into(),
            utc_ms: utc_ms_i64(),
            broker_epoch: broker_epoch.into(),
            sequence,
            event_type: "session".into(),
            schema_major: STORE_SCHEMA_MAJOR,
            schema_minor: 0,
            payload_jcs,
            snapshot_ziele: Vec::new(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum AppendAusgang {
    Angewandt { event_ord: i64, event_uuid: String },
    IdempotentWiederholt { event_ord: i64, event_uuid: String },
}

impl AppendAusgang {
    pub fn event_ord(&self) -> i64 {
        match self {
            Self::Angewandt { event_ord, .. } | Self::IdempotentWiederholt { event_ord, .. } => {
                *event_ord
            }
        }
    }

    pub fn event_uuid(&self) -> &str {
        match self {
            Self::Angewandt { event_uuid, .. } | Self::IdempotentWiederholt { event_uuid, .. } => {
                event_uuid
            }
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct CheckpointErgebnis {
    pub busy: i64,
    pub log_frames: i64,
    pub checkpointed_frames: i64,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct StorePragmas {
    pub journal_mode: String,
    pub synchronous: i64,
    pub wal_autocheckpoint: i64,
    pub busy_timeout_ms: i64,
    pub foreign_keys: i64,
}

impl CheckpointErgebnis {
    pub fn vollstaendig(self) -> bool {
        self.busy == CHECKPOINT_BUSY_ERWARTET && self.log_frames == self.checkpointed_frames
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StoreSicht {
    pub degradiert: bool,
    pub grund: Option<String>,
    pub verweigert: u64,
    pub eingereiht: u64,
    pub commits: u64,
    /// Groesste Zahl von Events, die seit dem Start in EINER SQLite-
    /// Transaktion committed wurde. Der Wert macht die harte S-05-Grenze
    /// auch im produktiven Lauf messbar.
    pub groesster_commit: usize,
    pub sqlite_version: String,
    pub rusqlite_version: String,
    pub pragmas: Option<StorePragmas>,
}

impl Default for StoreSicht {
    fn default() -> Self {
        Self {
            degradiert: false,
            grund: None,
            verweigert: 0,
            eingereiht: 0,
            commits: 0,
            groesster_commit: 0,
            sqlite_version: String::new(),
            rusqlite_version: RUSQLITE_VERSION.into(),
            pragmas: None,
        }
    }
}

/// Testnaht fuer die echte Kanalgrenze: der Writer wartet vor seinem ersten
/// `recv`, waehrend der Test exakt `STORE_KANAL_CAP` Auftraege einreiht.
#[doc(hidden)]
#[derive(Clone)]
pub struct StoreStartBarriere {
    stand: Arc<(Mutex<bool>, Condvar)>,
}

impl StoreStartBarriere {
    pub fn neu_blockiert() -> Self {
        Self {
            stand: Arc::new((Mutex::new(false), Condvar::new())),
        }
    }

    pub fn freigeben(&self) {
        let (schloss, signal) = &*self.stand;
        if let Ok(mut offen) = schloss.lock() {
            *offen = true;
            signal.notify_all();
        }
    }

    fn warten(&self) {
        let (schloss, signal) = &*self.stand;
        let mut offen = schloss.lock().unwrap_or_else(|e| e.into_inner());
        while !*offen {
            offen = signal.wait(offen).unwrap_or_else(|e| e.into_inner());
        }
    }
}

#[derive(Clone)]
pub struct StoreKonfiguration {
    pub db_pfad: PathBuf,
    /// Nur fuer deterministische Pfadtests. `None` fragt das Betriebssystem.
    #[doc(hidden)]
    pub remote_volume_override: Option<bool>,
    #[doc(hidden)]
    pub start_barriere: Option<StoreStartBarriere>,
    #[doc(hidden)]
    pub test_haken: Option<StoreTestHaken>,
}

impl StoreKonfiguration {
    pub fn fuer_pfad(db_pfad: impl Into<PathBuf>) -> Self {
        Self {
            db_pfad: db_pfad.into(),
            remote_volume_override: None,
            start_barriere: None,
            test_haken: None,
        }
    }

    pub fn standard() -> Result<Self, StoreFehler> {
        Ok(Self::fuer_pfad(standard_store_pfad()?))
    }
}

/// Ausschliesslich vom echten Child-Prozess der Crashmatrix gesetzt. Der
/// Produktionspfad konstruiert stets `None` und enthaelt keine Wartebarriere.
#[doc(hidden)]
#[derive(Clone)]
pub struct StoreTestHaken {
    pub punkt: String,
    pub marker_pfad: PathBuf,
}

impl StoreTestHaken {
    fn erreichen(&self, punkt: &str) {
        if self.punkt != punkt {
            return;
        }
        let _ = std::fs::write(&self.marker_pfad, punkt.as_bytes());
        loop {
            std::thread::park_timeout(Duration::from_millis(50));
        }
    }
}

#[derive(Clone)]
pub struct StoreHandle {
    sender: Option<SyncSender<WriterBefehl>>,
    sicht: Arc<Mutex<StoreSicht>>,
    db_pfad: PathBuf,
    capture_aktiv: Arc<AtomicBool>,
}

impl StoreHandle {
    pub fn sicht(&self) -> StoreSicht {
        self.sicht.lock().unwrap_or_else(|e| e.into_inner()).clone()
    }

    pub fn db_pfad(&self) -> &Path {
        &self.db_pfad
    }

    pub fn capture_aktiv_setzen(&self, aktiv: bool) {
        self.capture_aktiv.store(aktiv, Ordering::SeqCst);
    }

    pub fn append(&self, events: Vec<StoreEvent>) -> Result<Vec<AppendAusgang>, StoreFehler> {
        let antwort_rx = self.append_einreihen(events)?;
        antwort_rx.recv().map_err(|_| StoreFehler::Beendet)?
    }

    /// Trennt die begrenzte Annahmekante vom spaeteren Commit-Warten. Der
    /// Aufrufer darf zwischen diesen Schritten insbesondere keinen
    /// Coordinator-Lock halten.
    pub fn append_einreihen(
        &self,
        events: Vec<StoreEvent>,
    ) -> Result<mpsc::Receiver<Result<Vec<AppendAusgang>, StoreFehler>>, StoreFehler> {
        let (antwort_tx, antwort_rx) = mpsc::channel();
        self.senden(WriterBefehl::AppendBatch {
            events,
            antwort: antwort_tx,
        })?;
        Ok(antwort_rx)
    }

    pub fn konflikt_guard_persistieren(&self, guard: ConflictGuard) -> Result<(), StoreFehler> {
        let (antwort_tx, antwort_rx) = mpsc::channel();
        self.senden(WriterBefehl::PersistConflictGuard {
            guard,
            antwort: antwort_tx,
        })?;
        antwort_rx.recv().map_err(|_| StoreFehler::Beendet)?
    }

    pub fn konflikt_guard_aufloesen(
        &self,
        effective_address: String,
        derived_id: String,
    ) -> Result<(), StoreFehler> {
        let (antwort_tx, antwort_rx) = mpsc::channel();
        self.senden(WriterBefehl::ResolveConflictGuard {
            effective_address,
            derived_id,
            antwort: antwort_tx,
        })?;
        antwort_rx.recv().map_err(|_| StoreFehler::Beendet)?
    }

    pub fn snapshot_schuld_kompaktieren(
        &self,
        ziel: SnapshotZiel,
        gedeckt_bis_event_ord: i64,
    ) -> Result<usize, StoreFehler> {
        let (antwort_tx, antwort_rx) = mpsc::channel();
        self.senden(WriterBefehl::CompactSnapshotDebt {
            ziel,
            gedeckt_bis_event_ord,
            antwort: antwort_tx,
        })?;
        antwort_rx.recv().map_err(|_| StoreFehler::Beendet)?
    }

    pub fn checkpoint(&self, truncate: bool) -> Result<CheckpointErgebnis, StoreFehler> {
        let (antwort_tx, antwort_rx) = mpsc::channel();
        self.senden(WriterBefehl::Checkpoint {
            truncate,
            antwort: antwort_tx,
        })?;
        antwort_rx.recv().map_err(|_| StoreFehler::Beendet)?
    }

    fn senden(&self, befehl: WriterBefehl) -> Result<(), StoreFehler> {
        if let Some(grund) = self.sicht().grund.filter(|_| self.sicht().degradiert) {
            self.verweigerung_zaehlen();
            return Err(StoreFehler::Degradiert(grund));
        }
        let Some(sender) = &self.sender else {
            self.verweigerung_zaehlen();
            return Err(StoreFehler::Degradiert(
                self.sicht()
                    .grund
                    .unwrap_or_else(|| "nicht gestartet".into()),
            ));
        };
        match sender.try_send(befehl) {
            Ok(()) => {
                if let Ok(mut sicht) = self.sicht.lock() {
                    sicht.eingereiht = sicht.eingereiht.saturating_add(1);
                }
                Ok(())
            }
            Err(TrySendError::Full(_)) => {
                self.verweigerung_zaehlen();
                Err(StoreFehler::KanalVoll)
            }
            Err(TrySendError::Disconnected(_)) => {
                self.verweigerung_zaehlen();
                Err(StoreFehler::Beendet)
            }
        }
    }

    fn verweigerung_zaehlen(&self) {
        if let Ok(mut sicht) = self.sicht.lock() {
            sicht.verweigert = sicht.verweigert.saturating_add(1);
        }
    }

    pub fn session_state_lesen(
        &self,
        project_binding_id: &str,
        session_epoch: &str,
    ) -> Result<Option<(i64, Vec<u8>)>, StoreFehler> {
        kurze_leseconnection(&self.db_pfad, |conn| {
            conn.query_row(
                "SELECT last_event_ord, state_jcs FROM sessions \
                 WHERE project_binding_id=?1 AND session_epoch=?2",
                params![project_binding_id, session_epoch],
                |row| Ok((row.get(0)?, row.get(1)?)),
            )
            .optional()
            .map_err(StoreFehler::from)
        })
    }

    /// Interne, haltbare Wirkung eines bereits bekannten P0-Befehls. Der
    /// Coordinator vergleicht damit beim Retry nicht nur die `command_id`,
    /// sondern auch den kanonischen Befehl und gibt exakt denselben
    /// Revisions-/Hashstand zurueck.
    pub fn command_event_lesen(&self, command_id: &str) -> Result<Option<Vec<u8>>, StoreFehler> {
        kurze_leseconnection(&self.db_pfad, |conn| {
            conn.query_row(
                "SELECT payload_jcs FROM event_log WHERE command_id=?1",
                params![command_id],
                |row| row.get(0),
            )
            .optional()
            .map_err(StoreFehler::from)
        })
    }

    pub fn outbox_lesen(&self) -> Result<Vec<(SnapshotZiel, i64, i64)>, StoreFehler> {
        kurze_leseconnection(&self.db_pfad, |conn| {
            let mut stmt = conn.prepare(
                "SELECT target_project_binding_id, target_session_epoch, \
                        target_instance_id, object_key, snapshot_event_ord, write_attempts \
                 FROM outbox ORDER BY target_project_binding_id, target_session_epoch, \
                                      target_instance_id, object_key",
            )?;
            let rows = stmt.query_map([], |row| {
                Ok((
                    SnapshotZiel {
                        project_binding_id: row.get(0)?,
                        session_epoch: row.get(1)?,
                        instance_id: row.get(2)?,
                        object_key: row.get(3)?,
                    },
                    row.get(4)?,
                    row.get(5)?,
                ))
            })?;
            rows.collect::<Result<Vec<_>, _>>()
                .map_err(StoreFehler::from)
        })
    }
}

pub struct StoreWriter {
    handle: StoreHandle,
    join: Option<JoinHandle<()>>,
    restaurierte_guards: Vec<ConflictGuard>,
}

impl StoreWriter {
    pub fn degradiert_ohne_pfad(grund: impl Into<String>) -> Self {
        let grund = grund.into();
        let sicht = Arc::new(Mutex::new(StoreSicht::default()));
        degradiere(&sicht, grund);
        Self {
            handle: StoreHandle {
                sender: None,
                sicht,
                db_pfad: PathBuf::new(),
                capture_aktiv: Arc::new(AtomicBool::new(false)),
            },
            join: None,
            restaurierte_guards: Vec::new(),
        }
    }

    /// Startfehler werden absichtlich als degradierter Handle zurueckgegeben:
    /// der Broker darf weiterleben, aber kein persistenzpflichtiger Auftrag
    /// erhaelt eine falsche Annahmebestaetigung.
    pub fn starten(konfiguration: StoreKonfiguration) -> Self {
        let sicht = Arc::new(Mutex::new(StoreSicht::default()));
        let capture_aktiv = Arc::new(AtomicBool::new(false));
        let db_pfad = konfiguration.db_pfad.clone();
        match store_vorbereiten(&konfiguration) {
            Ok((conn, guards)) => {
                let sqlite_version = rusqlite::version().to_string();
                let pragmas = pragmas_lesen(&conn).ok();
                {
                    let mut s = sicht.lock().unwrap_or_else(|e| e.into_inner());
                    s.sqlite_version = sqlite_version.clone();
                    s.pragmas = pragmas;
                }
                eprintln!(
                    "Nakama Store: rusqlite {RUSQLITE_VERSION}, SQLite {sqlite_version}, {}",
                    db_pfad.display()
                );
                let (sender, receiver) = mpsc::sync_channel(STORE_KANAL_CAP);
                let sicht_thread = sicht.clone();
                let capture_thread = capture_aktiv.clone();
                let wal_pfad = wal_pfad(&db_pfad);
                let start_barriere = konfiguration.start_barriere.clone();
                let test_haken = konfiguration.test_haken.clone();
                let join = std::thread::Builder::new()
                    .name("nakama-store-writer".into())
                    .spawn(move || {
                        if let Some(barriere) = start_barriere {
                            barriere.warten();
                        }
                        writer_lauf(
                            conn,
                            receiver,
                            sicht_thread,
                            capture_thread,
                            wal_pfad,
                            test_haken,
                        );
                    })
                    .ok();
                if join.is_none() {
                    degradiere(&sicht, "Store-Writer-Thread konnte nicht starten".into());
                }
                let handle = StoreHandle {
                    sender: join.as_ref().map(|_| sender),
                    sicht,
                    db_pfad,
                    capture_aktiv,
                };
                Self {
                    handle,
                    join,
                    restaurierte_guards: guards,
                }
            }
            Err(fehler) => {
                degradiere(&sicht, fehler.to_string());
                let handle = StoreHandle {
                    sender: None,
                    sicht,
                    db_pfad,
                    capture_aktiv,
                };
                Self {
                    handle,
                    join: None,
                    restaurierte_guards: Vec::new(),
                }
            }
        }
    }

    pub fn handle(&self) -> StoreHandle {
        self.handle.clone()
    }

    pub fn restaurierte_guards(&self) -> &[ConflictGuard] {
        &self.restaurierte_guards
    }

    pub fn ist_degradiert(&self) -> bool {
        self.handle.sicht().degradiert
    }

    pub fn stoppen(&mut self) {
        if let Some(sender) = &self.handle.sender {
            let _ = sender.send(WriterBefehl::Shutdown);
        }
        if let Some(join) = self.join.take() {
            let _ = join.join();
        }
        self.handle.sender = None;
    }
}

impl Drop for StoreWriter {
    fn drop(&mut self) {
        self.stoppen();
    }
}

enum WriterBefehl {
    AppendBatch {
        events: Vec<StoreEvent>,
        antwort: mpsc::Sender<Result<Vec<AppendAusgang>, StoreFehler>>,
    },
    CompactSnapshotDebt {
        ziel: SnapshotZiel,
        gedeckt_bis_event_ord: i64,
        antwort: mpsc::Sender<Result<usize, StoreFehler>>,
    },
    PersistConflictGuard {
        guard: ConflictGuard,
        antwort: mpsc::Sender<Result<(), StoreFehler>>,
    },
    ResolveConflictGuard {
        effective_address: String,
        derived_id: String,
        antwort: mpsc::Sender<Result<(), StoreFehler>>,
    },
    Checkpoint {
        truncate: bool,
        antwort: mpsc::Sender<Result<CheckpointErgebnis, StoreFehler>>,
    },
    Shutdown,
}

struct AppendStand {
    rest: VecDeque<StoreEvent>,
    ergebnisse: Vec<AppendAusgang>,
    antwort: mpsc::Sender<Result<Vec<AppendAusgang>, StoreFehler>>,
}

struct CommitJob {
    events: Vec<StoreEvent>,
}

fn writer_lauf(
    mut conn: Connection,
    receiver: Receiver<WriterBefehl>,
    sicht: Arc<Mutex<StoreSicht>>,
    capture_aktiv: Arc<AtomicBool>,
    wal_pfad: PathBuf,
    test_haken: Option<StoreTestHaken>,
) {
    let mut vorgemerkt = VecDeque::new();
    let mut letztes_event = Instant::now();
    let mut idle_checkpoint_gelaufen = false;
    loop {
        let befehl = if let Some(v) = vorgemerkt.pop_front() {
            Ok(v)
        } else {
            receiver.recv_timeout(Duration::from_millis(COMMIT_FENSTER_MS))
        };
        let befehl = match befehl {
            Ok(v) => v,
            Err(RecvTimeoutError::Disconnected) => break,
            Err(RecvTimeoutError::Timeout) => {
                if checkpoint_ausloesen(
                    wal_groesse(&wal_pfad),
                    letztes_event.elapsed(),
                    capture_aktiv.load(Ordering::SeqCst),
                ) && !idle_checkpoint_gelaufen
                {
                    let _ = checkpoint(&conn, false);
                    idle_checkpoint_gelaufen = true;
                }
                continue;
            }
        };

        match befehl {
            WriterBefehl::AppendBatch { events, antwort } => {
                let mut aktiv = VecDeque::from([AppendStand {
                    rest: events.into(),
                    ergebnisse: Vec::new(),
                    antwort,
                }]);
                let mut kanal_getrennt = false;

                // Ein Aufrufer darf mehr als 64 Events als EINEN logischen
                // Auftrag einreichen. Der Writer zerlegt ihn geordnet in
                // mehrere harte Transaktionsabschnitte und antwortet erst,
                // wenn alle Abschnitte committed sind.
                while !aktiv.is_empty() {
                    while aktiv.front().is_some_and(|job| job.rest.is_empty()) {
                        if let Some(job) = aktiv.pop_front() {
                            let _ = job.antwort.send(Ok(job.ergebnisse));
                        }
                    }
                    if aktiv.is_empty() {
                        break;
                    }

                    let start = Instant::now();
                    let mut anzahl = 0usize;
                    let mut commit_jobs = Vec::new();
                    let mut zuordnung = Vec::new();
                    let mut barriere_gesehen = false;

                    loop {
                        // Alles bereits Angenommene wird in Kanalreihenfolge
                        // bis zur 64er-Grenze in diesen Commit gezogen.
                        let mut index = 0usize;
                        while anzahl < COMMIT_BATCH_MAX && index < aktiv.len() {
                            let job = aktiv.get_mut(index).expect("Append-Index");
                            if job.rest.is_empty() {
                                index += 1;
                                continue;
                            }
                            let nehmen = (COMMIT_BATCH_MAX - anzahl).min(job.rest.len());
                            let events = job.rest.drain(..nehmen).collect::<Vec<_>>();
                            anzahl += events.len();
                            commit_jobs.push(CommitJob { events });
                            zuordnung.push(index);
                            index += 1;
                        }
                        if anzahl >= COMMIT_BATCH_MAX {
                            break;
                        }

                        let rest = Duration::from_millis(COMMIT_FENSTER_MS)
                            .saturating_sub(start.elapsed());
                        if rest.is_zero() {
                            break;
                        }
                        match receiver.recv_timeout(rest) {
                            Ok(WriterBefehl::AppendBatch { events, antwort })
                                if !barriere_gesehen =>
                            {
                                aktiv.push_back(AppendStand {
                                    rest: events.into(),
                                    ergebnisse: Vec::new(),
                                    antwort,
                                });
                            }
                            Ok(anderer) => {
                                // Checkpoint, Guard oder Shutdown bleibt in
                                // seiner Reihenfolge, beendet aber NICHT das
                                // offene Append-Fenster. Nach der ersten
                                // Barriere werden auch spaetere Appends nur
                                // vorgemerkt, damit sie sie nicht ueberholen.
                                barriere_gesehen = true;
                                vorgemerkt.push_back(anderer);
                            }
                            Err(RecvTimeoutError::Timeout) => break,
                            Err(RecvTimeoutError::Disconnected) => {
                                kanal_getrennt = true;
                                std::thread::sleep(rest);
                                break;
                            }
                        }
                    }

                    if anzahl == 0 {
                        continue;
                    }
                    let ergebnisse = append_gruppe(&mut conn, &commit_jobs, test_haken.as_ref());
                    match ergebnisse {
                        Ok(pro_job) => {
                            for (index, ergebnis) in zuordnung.into_iter().zip(pro_job) {
                                aktiv
                                    .get_mut(index)
                                    .expect("Append-Antwortzuordnung")
                                    .ergebnisse
                                    .extend(ergebnis);
                            }
                            if let Ok(mut s) = sicht.lock() {
                                s.commits = s.commits.saturating_add(1);
                                s.groesster_commit = s.groesster_commit.max(anzahl);
                            }
                            letztes_event = Instant::now();
                            idle_checkpoint_gelaufen = false;
                            if wal_groesse(&wal_pfad) >= WAL_SCHWELLE_BYTES {
                                let _ = checkpoint(&conn, false);
                            }
                        }
                        Err(fehler) => {
                            degradiere(&sicht, fehler.to_string());
                            for job in aktiv.drain(..) {
                                let _ = job.antwort.send(Err(StoreFehler::Degradiert(
                                    fehler.to_string(),
                                )));
                            }
                            break;
                        }
                    }
                }
                if kanal_getrennt {
                    break;
                }
            }
            WriterBefehl::CompactSnapshotDebt {
                ziel,
                gedeckt_bis_event_ord,
                antwort,
            } => {
                let ergebnis =
                    kompaktieren(&mut conn, &ziel, gedeckt_bis_event_ord, test_haken.as_ref());
                if ergebnis.is_err() {
                    degradiere(&sicht, ergebnis.as_ref().unwrap_err().to_string());
                }
                let _ = antwort.send(ergebnis);
            }
            WriterBefehl::PersistConflictGuard { guard, antwort } => {
                let ergebnis = konflikt_guard_schreiben(&mut conn, &guard);
                if ergebnis.is_err() {
                    degradiere(&sicht, ergebnis.as_ref().unwrap_err().to_string());
                }
                let _ = antwort.send(ergebnis);
            }
            WriterBefehl::ResolveConflictGuard {
                effective_address,
                derived_id,
                antwort,
            } => {
                let ergebnis = konflikt_guard_loeschen(&mut conn, &effective_address, &derived_id);
                if ergebnis.is_err() {
                    degradiere(&sicht, ergebnis.as_ref().unwrap_err().to_string());
                }
                let _ = antwort.send(ergebnis);
            }
            WriterBefehl::Checkpoint { truncate, antwort } => {
                let erlaubt = !truncate || !capture_aktiv.load(Ordering::SeqCst);
                let ergebnis = if erlaubt {
                    checkpoint(&conn, truncate)
                } else {
                    Err(StoreFehler::Sqlite(
                        "TRUNCATE bei aktiver Capture-Session verweigert".into(),
                    ))
                };
                let _ = antwort.send(ergebnis);
            }
            WriterBefehl::Shutdown => break,
        }
    }
}

fn append_gruppe(
    conn: &mut Connection,
    jobs: &[CommitJob],
    test_haken: Option<&StoreTestHaken>,
) -> Result<Vec<Vec<AppendAusgang>>, StoreFehler> {
    let event_anzahl = jobs.iter().map(|job| job.events.len()).sum::<usize>();
    if event_anzahl > COMMIT_BATCH_MAX {
        return Err(StoreFehler::Sqlite(format!(
            "Group-Commit mit {event_anzahl} Events ueberschreitet Cap {COMMIT_BATCH_MAX}"
        )));
    }
    let tx = conn.transaction_with_behavior(TransactionBehavior::Immediate)?;
    let mut alle = Vec::with_capacity(jobs.len());
    for job in jobs {
        let mut ausgaenge = Vec::with_capacity(job.events.len());
        for event in &job.events {
            ausgaenge.push(append_eins(&tx, event)?);
        }
        alle.push(ausgaenge);
    }
    if let Some(haken) = test_haken {
        haken.erreichen("vor_store_commit");
    }
    tx.commit()?;
    if let Some(haken) = test_haken {
        haken.erreichen("nach_store_commit");
    }
    Ok(alle)
}

fn append_eins(tx: &Transaction<'_>, event: &StoreEvent) -> Result<AppendAusgang, StoreFehler> {
    if let Some(command_id) = event.command_id.as_deref() {
        if let Some((event_ord, event_uuid)) = tx
            .query_row(
                "SELECT event_ord, event_uuid FROM event_log WHERE command_id=?1",
                [command_id],
                |row| Ok((row.get(0)?, row.get(1)?)),
            )
            .optional()?
        {
            return Ok(AppendAusgang::IdempotentWiederholt {
                event_ord,
                event_uuid,
            });
        }
    }

    let event_uuid = event
        .event_uuid
        .clone()
        .unwrap_or_else(|| uuid::Uuid::new_v4().to_string());
    tx.execute(
        "INSERT INTO event_log(\
             event_uuid, command_id, project_binding_id, session_epoch, utc_ms,\
             broker_epoch, sequence, event_type, schema_major, schema_minor, payload_jcs\
         ) VALUES (?1,?2,?3,?4,?5,?6,?7,?8,?9,?10,?11)",
        params![
            event_uuid,
            event.command_id,
            event.project_binding_id,
            event.session_epoch,
            event.utc_ms,
            event.broker_epoch,
            event.sequence,
            event.event_type,
            event.schema_major,
            event.schema_minor,
            event.payload_jcs,
        ],
    )?;
    let event_ord = tx.last_insert_rowid();
    projektionen_anwenden(tx, event, event_ord)?;
    for ziel in &event.snapshot_ziele {
        tx.execute(
            "INSERT INTO outbox(target_project_binding_id,target_session_epoch,target_instance_id,object_key,snapshot_event_ord,write_attempts) \
             VALUES (?1,?2,?3,?4,?5,0) \
             ON CONFLICT(target_project_binding_id,target_session_epoch,target_instance_id,object_key) \
             DO UPDATE SET snapshot_event_ord=excluded.snapshot_event_ord,write_attempts=0 \
             WHERE excluded.snapshot_event_ord > outbox.snapshot_event_ord",
            params![
                ziel.project_binding_id,
                ziel.session_epoch,
                ziel.instance_id,
                ziel.object_key,
                event_ord,
            ],
        )?;
    }
    Ok(AppendAusgang::Angewandt {
        event_ord,
        event_uuid,
    })
}

fn projektionen_anwenden(
    tx: &Transaction<'_>,
    event: &StoreEvent,
    event_ord: i64,
) -> Result<(), StoreFehler> {
    tx.execute(
        "INSERT INTO projects(project_binding_id,last_event_ord,state_jcs) VALUES(?1,?2,?3) \
         ON CONFLICT(project_binding_id) DO UPDATE SET \
         last_event_ord=excluded.last_event_ord,state_jcs=excluded.state_jcs \
         WHERE excluded.last_event_ord > projects.last_event_ord",
        params![event.project_binding_id, event_ord, event.payload_jcs],
    )?;
    // `sessions.state_jcs` ist ausschliesslich die absolute
    // `session_snapshot`-Projektion. Domain-Events derselben Session haben
    // eigene Tabellen und duerfen diese Rekonstruktionsquelle nie ersetzen.
    // Ein persistenter P0-Befehl traegt seinen Snapshot deshalb ausdruecklich
    // im internen Eventpayload: Eventwahrheit, Projektion und Outbox-Schuld
    // bleiben ein einzelner Commit, ohne ein Domainpayload als Snapshot
    // auszugeben. Der verschachtelte Schnitt bleibt im Eventlog erhalten und
    // steht damit auch dem Projektions-Rebuild zur Verfuegung.
    let session_payload = if event.event_type == "session" {
        Some(event.payload_jcs.clone())
    } else if event.event_type == "command" {
        let intern: serde_json::Value = serde_json::from_slice(&event.payload_jcs)
            .map_err(|e| StoreFehler::Sqlite(format!("Command-Projektionspayload: {e}")))?;
        if intern.get("type").and_then(serde_json::Value::as_str)
            == Some("internal_p0_command")
        {
            intern
                .get("session_snapshot")
                .map(serde_json::to_vec)
                .transpose()
                .map_err(|e| StoreFehler::Sqlite(format!("Command-Sessionprojektion: {e}")))?
        } else {
            None
        }
    } else {
        None
    };
    if let Some(session_payload) = session_payload {
        tx.execute(
            "INSERT INTO sessions(project_binding_id,session_epoch,last_event_ord,state_jcs) \
             VALUES(?1,?2,?3,?4) \
             ON CONFLICT(project_binding_id,session_epoch) DO UPDATE SET \
             last_event_ord=excluded.last_event_ord,state_jcs=excluded.state_jcs \
             WHERE excluded.last_event_ord > sessions.last_event_ord",
            params![
                event.project_binding_id,
                event.session_epoch,
                event_ord,
                session_payload,
            ],
        )?;
    }

    let domain = match event.event_type.as_str() {
        "passage" => Some(("passages", "passage_id")),
        "evidence" => Some(("evidence", "evidence_id")),
        "finding" => Some(("findings", "finding_id")),
        "proposal" => Some(("proposals", "proposal_id")),
        "transaction" => Some(("transactions", "transaction_id")),
        "experiment" => Some(("experiments", "experiment_id")),
        "user_verdict" => Some(("user_verdicts", "user_verdict_id")),
        _ => None,
    };
    if let Some((tabelle, id_feld)) = domain {
        let payload: serde_json::Value = serde_json::from_slice(&event.payload_jcs)
            .map_err(|e| StoreFehler::Sqlite(format!("Projektionspayload: {e}")))?;
        let objekt_id = payload
            .get(id_feld)
            .and_then(serde_json::Value::as_str)
            .ok_or_else(|| StoreFehler::Sqlite(format!("Projektionspayload ohne {id_feld}")))?;
        // Tabellen- und Spaltenname kommen ausschliesslich aus der obigen
        // geschlossenen Zuordnung, nie aus Store- oder Wiredaten.
        let sql = format!(
            "INSERT INTO {tabelle}({id_feld},project_binding_id,session_epoch,last_event_ord,state_jcs) \
             VALUES(?1,?2,?3,?4,?5) ON CONFLICT({id_feld}) DO UPDATE SET \
             project_binding_id=excluded.project_binding_id,session_epoch=excluded.session_epoch,\
             last_event_ord=excluded.last_event_ord,state_jcs=excluded.state_jcs \
             WHERE excluded.last_event_ord > {tabelle}.last_event_ord"
        );
        tx.execute(
            &sql,
            params![
                objekt_id,
                event.project_binding_id,
                event.session_epoch,
                event_ord,
                event.payload_jcs,
            ],
        )?;
    }
    Ok(())
}

fn kompaktieren(
    conn: &mut Connection,
    ziel: &SnapshotZiel,
    gedeckt_bis_event_ord: i64,
    test_haken: Option<&StoreTestHaken>,
) -> Result<usize, StoreFehler> {
    let tx = conn.transaction_with_behavior(TransactionBehavior::Immediate)?;
    let geaendert = tx.execute(
        "DELETE FROM outbox WHERE target_project_binding_id=?1 AND target_session_epoch=?2 \
         AND target_instance_id=?3 AND object_key=?4 AND snapshot_event_ord<=?5",
        params![
            ziel.project_binding_id,
            ziel.session_epoch,
            ziel.instance_id,
            ziel.object_key,
            gedeckt_bis_event_ord,
        ],
    )?;
    if let Some(haken) = test_haken {
        haken.erreichen("vor_outbox_kompaktierung");
    }
    tx.commit()?;
    if let Some(haken) = test_haken {
        haken.erreichen("nach_outbox_kompaktierung");
    }
    Ok(geaendert)
}

fn konflikt_guard_schreiben(
    conn: &mut Connection,
    guard: &ConflictGuard,
) -> Result<(), StoreFehler> {
    let tx = conn.transaction_with_behavior(TransactionBehavior::Immediate)?;
    tx.execute(
        "INSERT OR IGNORE INTO conflict_guards(effective_address,derived_id,created_utc_ms)\
         VALUES(?1,?2,?3)",
        params![
            guard.effective_address,
            guard.derived_id,
            guard.created_utc_ms
        ],
    )?;
    tx.commit()?;
    Ok(())
}

fn konflikt_guard_loeschen(
    conn: &mut Connection,
    effective_address: &str,
    derived_id: &str,
) -> Result<(), StoreFehler> {
    let tx = conn.transaction_with_behavior(TransactionBehavior::Immediate)?;
    tx.execute(
        "DELETE FROM conflict_guards WHERE effective_address=?1 AND derived_id=?2",
        params![effective_address, derived_id],
    )?;
    tx.commit()?;
    Ok(())
}

fn checkpoint(conn: &Connection, truncate: bool) -> Result<CheckpointErgebnis, StoreFehler> {
    let pragma = if truncate {
        "PRAGMA wal_checkpoint(TRUNCATE)"
    } else {
        "PRAGMA wal_checkpoint(PASSIVE)"
    };
    conn.query_row(pragma, [], |row| {
        Ok(CheckpointErgebnis {
            busy: row.get(0)?,
            log_frames: row.get(1)?,
            checkpointed_frames: row.get(2)?,
        })
    })
    .map_err(StoreFehler::from)
}

pub fn commit_ausloesen(alter: Duration, batch_len: usize) -> bool {
    alter >= Duration::from_millis(COMMIT_FENSTER_MS) || batch_len >= COMMIT_BATCH_MAX
}

pub fn checkpoint_ausloesen(wal_bytes: u64, idle: Duration, capture_aktiv: bool) -> bool {
    wal_bytes >= WAL_SCHWELLE_BYTES
        || (!capture_aktiv && idle >= Duration::from_millis(STORE_IDLE_MS))
}

pub fn recovery_testgrenze_bestanden(dauer: Duration) -> bool {
    dauer < Duration::from_millis(STORE_RECOVERY_TEST_MAX_MS)
}

pub fn busy_timeout_abgelaufen(dauer: Duration) -> bool {
    dauer >= Duration::from_millis(BUSY_TIMEOUT_MS)
}

pub fn migration_1_checksum() -> String {
    let mut hash = Sha256::new();
    hash.update(MIGRATION_1_SQL.as_bytes());
    hash.finalize().iter().map(|b| format!("{b:02x}")).collect()
}

fn store_vorbereiten(
    konfiguration: &StoreKonfiguration,
) -> Result<(Connection, Vec<ConflictGuard>), StoreFehler> {
    let remote = match konfiguration.remote_volume_override {
        Some(v) => v,
        None => store_pfad_ist_remote(&konfiguration.db_pfad)?,
    };
    if remote {
        return Err(StoreFehler::Pfad(
            "Remote-Volume: DB und WAL werden dort nicht geoeffnet".into(),
        ));
    }

    if konfiguration.db_pfad.exists() {
        match bekanntes_schema_readonly(&konfiguration.db_pfad) {
            Ok(Some(major)) if major > STORE_SCHEMA_MAJOR => {
                return Err(StoreFehler::Degradiert(format!(
                    "Schema-Major {major} ist neuer als {STORE_SCHEMA_MAJOR}; kein Downgrade"
                )));
            }
            Ok(_) => {}
            Err(e) => {
                return Err(StoreFehler::Degradiert(format!(
                    "DB kann nicht sicher gelesen werden: {e}"
                )));
            }
        }
    }

    if let Some(eltern) = konfiguration.db_pfad.parent() {
        std::fs::create_dir_all(eltern)
            .map_err(|e| StoreFehler::Pfad(format!("{}: {e}", eltern.display())))?;
    }
    let mut conn = Connection::open_with_flags(
        &konfiguration.db_pfad,
        OpenFlags::SQLITE_OPEN_READ_WRITE
            | OpenFlags::SQLITE_OPEN_CREATE
            | OpenFlags::SQLITE_OPEN_NO_MUTEX,
    )?;
    pragmas_setzen(&conn)?;
    migration_1(&mut conn, konfiguration.test_haken.as_ref())?;
    let guards = konflikt_guards_lesen(&conn)?;
    Ok((conn, guards))
}

fn pragmas_setzen(conn: &Connection) -> Result<(), StoreFehler> {
    conn.pragma_update(None, "foreign_keys", "ON")?;
    let modus: String = conn.query_row("PRAGMA journal_mode=WAL", [], |row| row.get(0))?;
    if !modus.eq_ignore_ascii_case("wal") {
        return Err(StoreFehler::Sqlite(format!(
            "journal_mode ist {modus}, nicht WAL"
        )));
    }
    conn.pragma_update(None, "synchronous", "FULL")?;
    conn.busy_timeout(Duration::from_millis(BUSY_TIMEOUT_MS))?;
    conn.pragma_update(None, "wal_autocheckpoint", 0)?;
    Ok(())
}

fn pragmas_lesen(conn: &Connection) -> Result<StorePragmas, StoreFehler> {
    Ok(StorePragmas {
        journal_mode: conn.query_row("PRAGMA journal_mode", [], |row| row.get(0))?,
        synchronous: conn.query_row("PRAGMA synchronous", [], |row| row.get(0))?,
        wal_autocheckpoint: conn.query_row("PRAGMA wal_autocheckpoint", [], |row| row.get(0))?,
        busy_timeout_ms: conn.query_row("PRAGMA busy_timeout", [], |row| row.get(0))?,
        foreign_keys: conn.query_row("PRAGMA foreign_keys", [], |row| row.get(0))?,
    })
}

fn migration_1(
    conn: &mut Connection,
    test_haken: Option<&StoreTestHaken>,
) -> Result<(), StoreFehler> {
    let checksum = migration_1_checksum();
    let tx = conn.transaction_with_behavior(TransactionBehavior::Immediate)?;
    tx.execute_batch(MIGRATION_1_SQL)?;
    if let Some(vorhanden) = tx
        .query_row(
            "SELECT checksum_sha256 FROM schema_migrations WHERE major=?1",
            [STORE_SCHEMA_MAJOR],
            |row| row.get::<_, String>(0),
        )
        .optional()?
    {
        if vorhanden != checksum {
            return Err(StoreFehler::Degradiert(
                "Migration 1 hat einen fremden Checksum-Stand".into(),
            ));
        }
    } else {
        tx.execute(
            "INSERT INTO schema_migrations(major,checksum_sha256,applied_utc_ms)\
             VALUES(?1,?2,?3)",
            params![STORE_SCHEMA_MAJOR, checksum, utc_ms_i64()],
        )?;
    }
    if let Some(haken) = test_haken {
        haken.erreichen("waehrend_migration_1");
    }
    tx.commit()?;
    Ok(())
}

fn bekanntes_schema_readonly(pfad: &Path) -> Result<Option<i64>, StoreFehler> {
    let conn = Connection::open_with_flags(
        pfad,
        OpenFlags::SQLITE_OPEN_READ_ONLY | OpenFlags::SQLITE_OPEN_NO_MUTEX,
    )?;
    let gibt_es: bool = conn.query_row(
        "SELECT EXISTS(SELECT 1 FROM sqlite_master WHERE type='table' AND name='schema_migrations')",
        [],
        |row| row.get(0),
    )?;
    if !gibt_es {
        return Ok(None);
    }
    conn.query_row("SELECT MAX(major) FROM schema_migrations", [], |row| {
        row.get(0)
    })
    .map_err(StoreFehler::from)
}

fn konflikt_guards_lesen(conn: &Connection) -> Result<Vec<ConflictGuard>, StoreFehler> {
    let mut stmt = conn.prepare(
        "SELECT effective_address,derived_id,created_utc_ms FROM conflict_guards \
         ORDER BY effective_address,derived_id",
    )?;
    let rows = stmt.query_map([], |row| {
        Ok(ConflictGuard {
            effective_address: row.get(0)?,
            derived_id: row.get(1)?,
            created_utc_ms: row.get(2)?,
        })
    })?;
    rows.collect::<Result<Vec<_>, _>>()
        .map_err(StoreFehler::from)
}

pub fn projektionen_neu_bauen(pfad: &Path) -> Result<(), StoreFehler> {
    let mut conn = Connection::open_with_flags(
        pfad,
        OpenFlags::SQLITE_OPEN_READ_WRITE | OpenFlags::SQLITE_OPEN_NO_MUTEX,
    )?;
    pragmas_setzen(&conn)?;
    let events = {
        let mut stmt = conn.prepare(
            "SELECT event_uuid,command_id,project_binding_id,session_epoch,utc_ms,\
                    broker_epoch,sequence,event_type,schema_major,schema_minor,payload_jcs,event_ord \
             FROM event_log ORDER BY event_ord",
        )?;
        let rows = stmt.query_map([], |row| {
            Ok((
                StoreEvent {
                    event_uuid: Some(row.get(0)?),
                    command_id: row.get(1)?,
                    project_binding_id: row.get(2)?,
                    session_epoch: row.get(3)?,
                    utc_ms: row.get(4)?,
                    broker_epoch: row.get(5)?,
                    sequence: row.get(6)?,
                    event_type: row.get(7)?,
                    schema_major: row.get(8)?,
                    schema_minor: row.get(9)?,
                    payload_jcs: row.get(10)?,
                    snapshot_ziele: Vec::new(),
                },
                row.get::<_, i64>(11)?,
            ))
        })?;
        rows.collect::<Result<Vec<_>, _>>()?
    };
    let tx = conn.transaction_with_behavior(TransactionBehavior::Immediate)?;
    tx.execute_batch(
        "DELETE FROM experiment_events; DELETE FROM passages; DELETE FROM evidence;\
         DELETE FROM findings; DELETE FROM proposals; DELETE FROM transactions;\
         DELETE FROM experiments; DELETE FROM user_verdicts; DELETE FROM sessions;\
         DELETE FROM projects;",
    )?;
    for (event, event_ord) in &events {
        projektionen_anwenden(&tx, event, *event_ord)?;
    }
    tx.commit()?;
    Ok(())
}

fn kurze_leseconnection<T>(
    pfad: &Path,
    arbeit: impl FnOnce(&Connection) -> Result<T, StoreFehler>,
) -> Result<T, StoreFehler> {
    let conn = Connection::open_with_flags(
        pfad,
        OpenFlags::SQLITE_OPEN_READ_ONLY | OpenFlags::SQLITE_OPEN_NO_MUTEX,
    )?;
    conn.busy_timeout(Duration::from_millis(BUSY_TIMEOUT_MS))?;
    arbeit(&conn)
}

fn degradiere(sicht: &Arc<Mutex<StoreSicht>>, grund: String) {
    if let Ok(mut s) = sicht.lock() {
        s.degradiert = true;
        if s.grund.is_none() {
            s.grund = Some(grund);
        }
    }
}

fn utc_ms_i64() -> i64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_millis().min(i64::MAX as u128) as i64)
        .unwrap_or(0)
}

fn wal_pfad(db_pfad: &Path) -> PathBuf {
    let mut roh = db_pfad.as_os_str().to_os_string();
    roh.push("-wal");
    PathBuf::from(roh)
}

fn wal_groesse(wal_pfad: &Path) -> u64 {
    std::fs::metadata(wal_pfad).map(|m| m.len()).unwrap_or(0)
}

pub fn store_pfad_unter(local_app_data: &Path) -> PathBuf {
    local_app_data.join(STORE_RELATIVPFAD).join(STORE_DATEINAME)
}

#[cfg(windows)]
pub fn standard_store_pfad() -> Result<PathBuf, StoreFehler> {
    use windows_sys::Win32::Foundation::HANDLE;
    use windows_sys::Win32::System::Com::CoTaskMemFree;
    use windows_sys::Win32::UI::Shell::{
        FOLDERID_LocalAppData, SHGetKnownFolderPath, KF_FLAG_DEFAULT,
    };

    let mut roh: *mut u16 = std::ptr::null_mut();
    // SAFETY: die API schreibt bei Erfolg einen CoTaskMem-String nach `roh`;
    // er wird nach der Kopie genau einmal freigegeben.
    let hr = unsafe {
        SHGetKnownFolderPath(
            &FOLDERID_LocalAppData,
            KF_FLAG_DEFAULT as u32,
            std::ptr::null_mut::<core::ffi::c_void>() as HANDLE,
            &mut roh,
        )
    };
    if hr < 0 || roh.is_null() {
        return Err(StoreFehler::Pfad(format!(
            "FOLDERID_LocalAppData konnte nicht aufgeloest werden (HRESULT {hr:#x})"
        )));
    }
    // SAFETY: `roh` ist ein nullterminierter UTF-16-String aus dem gerade
    // erfolgreichen Aufruf und bleibt bis `CoTaskMemFree` gueltig.
    let root = unsafe {
        let mut len = 0usize;
        while *roh.add(len) != 0 {
            len += 1;
        }
        let pfad = PathBuf::from(String::from_utf16_lossy(std::slice::from_raw_parts(
            roh, len,
        )));
        CoTaskMemFree(roh.cast());
        pfad
    };
    Ok(store_pfad_unter(&root))
}

#[cfg(not(windows))]
pub fn standard_store_pfad() -> Result<PathBuf, StoreFehler> {
    let root = std::env::var_os("LOCALAPPDATA")
        .map(PathBuf::from)
        .ok_or_else(|| {
            StoreFehler::Pfad("FOLDERID_LocalAppData ist nur auf Windows verfuegbar".into())
        })?;
    Ok(store_pfad_unter(&root))
}

#[cfg(windows)]
pub fn store_pfad_ist_remote(pfad: &Path) -> Result<bool, StoreFehler> {
    use std::os::windows::ffi::OsStrExt;
    use windows_sys::Win32::Storage::FileSystem::{GetDriveTypeW, GetVolumePathNameW};
    use windows_sys::Win32::System::WindowsProgramming::DRIVE_REMOTE;

    let mut kandidat = pfad;
    while !kandidat.exists() {
        kandidat = kandidat.parent().ok_or_else(|| {
            StoreFehler::Pfad(format!(
                "kein existierender Vorfahr fuer {}",
                pfad.display()
            ))
        })?;
    }
    let mut wide: Vec<u16> = kandidat.as_os_str().encode_wide().collect();
    wide.push(0);
    let mut volume = vec![0u16; 32768];
    // SAFETY: beide Puffer sind nullterminiert bzw. ausreichend gross; die API
    // schreibt hoechstens die angegebene Zahl UTF-16-Codeunits.
    let ok = unsafe { GetVolumePathNameW(wide.as_ptr(), volume.as_mut_ptr(), volume.len() as u32) };
    if ok == 0 {
        return Err(StoreFehler::Pfad(format!(
            "Volume fuer {} konnte nicht bestimmt werden",
            kandidat.display()
        )));
    }
    // SAFETY: `volume` enthaelt nach Erfolg einen nullterminierten Rootpfad.
    Ok(unsafe { GetDriveTypeW(volume.as_ptr()) } == DRIVE_REMOTE)
}

#[cfg(not(windows))]
pub fn store_pfad_ist_remote(_pfad: &Path) -> Result<bool, StoreFehler> {
    Ok(false)
}

#[cfg(test)]
mod tests {
    use super::*;
    use rusqlite::ffi;
    use std::ffi::c_int;
    use std::ptr;
    use std::sync::atomic::{AtomicPtr, AtomicUsize};
    use std::sync::Once;

    static VFS_REGISTRIEREN: Once = Once::new();
    static BASIS_VFS: AtomicPtr<ffi::sqlite3_vfs> = AtomicPtr::new(ptr::null_mut());
    static BASIS_DATEIGROESSE: AtomicUsize = AtomicUsize::new(0);
    static SYNC_AUFRUFE: AtomicUsize = AtomicUsize::new(0);
    const ZAEHL_VFS: &str = "nakama-test-sync-counter";
    const ZAEHL_VFS_C: &[u8] = b"nakama-test-sync-counter\0";

    type SyncFn = unsafe extern "C" fn(*mut ffi::sqlite3_file, c_int) -> c_int;

    #[repr(C)]
    struct DateiAnhang {
        methoden: ffi::sqlite3_io_methods,
        original_sync: Option<SyncFn>,
    }

    fn anhang_offset() -> usize {
        let basis = BASIS_DATEIGROESSE.load(Ordering::SeqCst);
        let ausrichtung = std::mem::align_of::<DateiAnhang>();
        (basis + ausrichtung - 1) & !(ausrichtung - 1)
    }

    unsafe fn anhang(datei: *mut ffi::sqlite3_file) -> *mut DateiAnhang {
        // SAFETY: Der registrierte Wrapper vergroessert `szOsFile` exakt um
        // diesen ausgerichteten Anhang; SQLite reicht denselben Puffer an alle
        // I/O-Methoden weiter.
        unsafe { (datei as *mut u8).add(anhang_offset()).cast() }
    }

    unsafe extern "C" fn sync_zaehlen(datei: *mut ffi::sqlite3_file, flags: c_int) -> c_int {
        SYNC_AUFRUFE.fetch_add(1, Ordering::SeqCst);
        // SAFETY: `zaehl_open` hat den Originalzeiger vor dem Ersetzen von
        // `xSync` im dateieigenen Anhang abgelegt.
        let original = unsafe { (*anhang(datei)).original_sync };
        match original {
            Some(sync) => unsafe { sync(datei, flags) },
            None => ffi::SQLITE_IOERR,
        }
    }

    unsafe extern "C" fn zaehl_open(
        _wrapper: *mut ffi::sqlite3_vfs,
        name: ffi::sqlite3_filename,
        datei: *mut ffi::sqlite3_file,
        flags: c_int,
        aus_flags: *mut c_int,
    ) -> c_int {
        let basis = BASIS_VFS.load(Ordering::SeqCst);
        if basis.is_null() {
            return ffi::SQLITE_CANTOPEN;
        }
        // SAFETY: `BASIS_VFS` stammt aus `sqlite3_vfs_find(NULL)` und bleibt
        // fuer die Prozesslebenszeit registriert.
        let Some(open) = (unsafe { (*basis).xOpen }) else {
            return ffi::SQLITE_CANTOPEN;
        };
        let rc = unsafe { open(basis, name, datei, flags, aus_flags) };
        if rc != ffi::SQLITE_OK || unsafe { (*datei).pMethods.is_null() } {
            return rc;
        }
        // SAFETY: Die Default-VFS hat den vorderen Dateipuffer initialisiert;
        // ihr `sqlite3_io_methods` ist Copy und fuer die Dateilebenszeit
        // gueltig. Nur xSync wird im dateieigenen Abbild ersetzt.
        let original_methoden = unsafe { *(*datei).pMethods };
        let ziel = unsafe { anhang(datei) };
        unsafe {
            ptr::write(
                ziel,
                DateiAnhang {
                    original_sync: original_methoden.xSync,
                    methoden: original_methoden,
                },
            );
            (*ziel).methoden.xSync = Some(sync_zaehlen);
            (*datei).pMethods = &(*ziel).methoden;
        }
        rc
    }

    fn zaehl_vfs_registrieren() {
        VFS_REGISTRIEREN.call_once(|| unsafe {
            let basis = ffi::sqlite3_vfs_find(ptr::null());
            assert!(!basis.is_null(), "SQLite-Default-VFS fehlt");
            BASIS_VFS.store(basis, Ordering::SeqCst);
            BASIS_DATEIGROESSE.store((*basis).szOsFile as usize, Ordering::SeqCst);
            let mut wrapper = *basis;
            wrapper.pNext = ptr::null_mut();
            wrapper.zName = ZAEHL_VFS_C.as_ptr().cast();
            wrapper.szOsFile = (anhang_offset() + std::mem::size_of::<DateiAnhang>()) as c_int;
            wrapper.xOpen = Some(zaehl_open);
            let wrapper = Box::into_raw(Box::new(wrapper));
            let rc = ffi::sqlite3_vfs_register(wrapper, 0);
            assert_eq!(rc, ffi::SQLITE_OK, "Test-VFS registrieren");
        });
    }

    fn test_event(sequence: i64) -> StoreEvent {
        StoreEvent::session_snapshot(
            "00000000000000000000000000000001",
            "00000000000000000000000000000002",
            "00000000000000000000000000000003",
            sequence,
            br#"{"type":"session_snapshot"}"#.to_vec(),
        )
    }

    #[test]
    fn konstante_trigger_haben_exakte_grenzen() {
        assert!(!commit_ausloesen(
            Duration::from_millis(COMMIT_FENSTER_MS - 1),
            COMMIT_BATCH_MAX - 1
        ));
        assert!(commit_ausloesen(
            Duration::from_millis(COMMIT_FENSTER_MS),
            COMMIT_BATCH_MAX - 1
        ));
        assert!(commit_ausloesen(Duration::ZERO, COMMIT_BATCH_MAX));
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
        assert!(recovery_testgrenze_bestanden(Duration::from_millis(
            STORE_RECOVERY_TEST_MAX_MS - 1
        )));
        assert!(!recovery_testgrenze_bestanden(Duration::from_millis(
            STORE_RECOVERY_TEST_MAX_MS
        )));
    }

    #[test]
    fn rusqlite_version_0_40_2_ist_exakt_gepinnt() {
        assert_eq!(RUSQLITE_VERSION, "0.40.2");
        let cargo = include_str!("../Cargo.toml");
        let zeile = cargo
            .lines()
            .find(|zeile| zeile.trim_start().starts_with("rusqlite ="))
            .expect("rusqlite-Abhaengigkeit fehlt");
        assert!(zeile.contains("version = \"=0.40.2\""));
        assert!(zeile.contains("features = [\"bundled\"]"));
    }

    #[test]
    fn group_commit_batchcap_hat_keinen_einzel_fsync_je_event() {
        zaehl_vfs_registrieren();
        let pfad = std::env::temp_dir().join(format!(
            "nakama-sync-counter-{}-{}.sqlite3",
            std::process::id(),
            uuid::Uuid::new_v4().simple()
        ));
        let flags = OpenFlags::SQLITE_OPEN_READ_WRITE
            | OpenFlags::SQLITE_OPEN_CREATE
            | OpenFlags::SQLITE_OPEN_NO_MUTEX;
        let mut conn = Connection::open_with_flags_and_vfs(&pfad, flags, ZAEHL_VFS).unwrap();
        pragmas_setzen(&conn).unwrap();
        migration_1(&mut conn, None).unwrap();

        SYNC_AUFRUFE.store(0, Ordering::SeqCst);
        append_gruppe(
            &mut conn,
            &[CommitJob {
                events: vec![test_event(0)],
            }],
            None,
        )
        .unwrap();
        let sync_ein_event = SYNC_AUFRUFE.swap(0, Ordering::SeqCst);

        append_gruppe(
            &mut conn,
            &[CommitJob {
                events: (1..=COMMIT_BATCH_MAX as i64).map(test_event).collect(),
            }],
            None,
        )
        .unwrap();
        let sync_voller_batch = SYNC_AUFRUFE.load(Ordering::SeqCst);
        assert!(
            sync_ein_event > 0,
            "die VFS-Zaehlnadel sah keinen xSync-Aufruf"
        );
        assert_eq!(
            sync_voller_batch, sync_ein_event,
            "ein 64er Group-Commit muss dieselbe xSync-Zahl wie ein Einzelevent haben"
        );
        assert!(sync_voller_batch < COMMIT_BATCH_MAX);

        drop(conn);
        let _ = std::fs::remove_file(&pfad);
        let _ = std::fs::remove_file(pfad.with_extension("sqlite3-wal"));
        let _ = std::fs::remove_file(pfad.with_extension("sqlite3-shm"));
    }
}
