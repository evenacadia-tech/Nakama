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
use std::collections::VecDeque;
use std::fmt;
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::mpsc::{self, Receiver, RecvTimeoutError, SyncSender, TrySendError};
use std::sync::{Arc, Condvar, Mutex};
use std::thread::JoinHandle;
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

mod handle;
mod migration;
mod pfad;
mod writer;

pub use handle::{
    IdleCheckpointNaht, StoreHandle, StoreKonfiguration, StoreStartBarriere, StoreTestHaken,
};
pub use migration::{migration_1_checksum, projektionen_neu_bauen};
use handle::{degradiere, utc_ms_i64};
pub use pfad::{geoeffnete_db_volume, standard_store_pfad, store_pfad_ist_remote, store_pfad_unter};
pub use writer::StoreWriter;
use writer::{projektionen_anwenden, WriterBefehl};
use pfad::{wal_groesse, wal_pfad};
use migration::{pragmas_lesen, store_vorbereiten};

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
/// H-14: Obergrenze der persistenten Konfliktriegel. Die Tabelle hatte bis
/// NAK-121 keinen Deckel, und C-07 verbietet jede zeitbasierte Freigabe - ein
/// Angreifer konnte sie mit Kollisionen unbegrenzt fuellen. Ist die Grenze
/// erreicht, faellt das Schreiben fail-closed, statt die Datenbank wachsen zu
/// lassen. Derselbe Wert wie MAX_QUARANTAENE: beide Speicher fuehren dieselben
/// Kollisionen, der eine fluechtig, der andere dauerhaft.
pub const MAX_KONFLIKT_GUARDS: usize = crate::instance_alias::MAX_QUARANTAENE;

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
    /// H-18: gescheiterte Leerlauf-Checkpoints. Bis NAK-121 wurde das Ergebnis
    /// verworfen UND der Erfolg trotzdem gemerkt - ein gescheiterter Checkpoint
    /// wurde deshalb nie wiederholt, waehrend die Storesicht weiter gesund
    /// meldete. Der Zaehler macht den Fehlschlag sichtbar; im Normalbetrieb
    /// bleibt er null.
    pub checkpoints_gescheitert: u64,
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
            checkpoints_gescheitert: 0,
            sqlite_version: String::new(),
            rusqlite_version: RUSQLITE_VERSION.into(),
            pragmas: None,
        }
    }
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

#[cfg(test)]
mod tests {
    use super::*;

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
        let cargo = include_str!("../../Cargo.toml");
        let zeile = cargo
            .lines()
            .find(|zeile| zeile.trim_start().starts_with("rusqlite ="))
            .expect("rusqlite-Abhaengigkeit fehlt");
        assert!(zeile.contains("version = \"=0.40.2\""));
        assert!(zeile.contains("features = [\"bundled\"]"));
    }

    /// G2-MSRV-001, der Zwilling des Tests darueber: der Compilerboden ist eine
    /// pruefbare Zusage, keine Absichtserklaerung. `MSRV_BODEN` haelt den
    /// deklarierten Wert fest, und der zweite Zwilling
    /// `rust_version_deckt_alle_gesperrten_abhaengigkeiten` misst, dass er
    /// nicht unter dem Boden des gesperrten Baumes liegt. Zusaetzlich muss die
    /// Begruendung im Manifest die juengste benutzte Standardmethode nennen -
    /// `Option::is_none_or`, stabil seit 1.82.0 -, damit die Zahl beim
    /// naechsten Aufraeumen nicht wieder zur runden Behauptung wird. Wer die
    /// Zeile entfernt oder senkt, faellt hier auf.
    #[test]
    fn rust_version_boden_ist_exakt_gepinnt() {
        const MSRV_BODEN: &str = "1.85";
        let cargo = include_str!("../../Cargo.toml");
        let zeile = cargo
            .lines()
            .find(|zeile| zeile.trim_start().starts_with("rust-version"))
            .expect("rust-version fehlt in Cargo.toml");
        assert!(
            zeile.contains(&format!("\"{MSRV_BODEN}\"")),
            "rust-version steht nicht auf {MSRV_BODEN}: {zeile}"
        );
        // Die Begruendung steht im Kommentar darueber und muss die Methode
        // nennen, aus der der Boden folgt - sonst ist die Zahl beim naechsten
        // Aufraeumen wieder eine runde Behauptung.
        assert!(
            cargo.contains("is_none_or"),
            "die Begruendung des Compilerbodens fehlt in Cargo.toml"
        );
    }

    /// G2-MSRV-001, Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026):
    /// Der Test darueber vergleicht nur den Manifestwert mit dem eigenen Code
    /// und konnte deshalb nicht sehen, dass die im Lockfile GESPERRTEN
    /// Abhaengigkeiten einen hoeheren Boden verlangen - `cargo +1.82 check
    /// --locked` brach vor dem Kompilieren ab. Dieser Zwilling misst den
    /// tatsaechlichen Boden: er liest die `rust-version` jedes gesperrten
    /// Pakets aus `cargo metadata --locked` und verlangt, dass keines mehr
    /// fordert als das eigene Manifest deklariert.
    #[test]
    fn rust_version_deckt_alle_gesperrten_abhaengigkeiten() {
        let manifest = concat!(env!("CARGO_MANIFEST_DIR"), "/Cargo.toml");
        // Cargo setzt `CARGO` fuer jeden Testlauf; ohne die Variable liefe der
        // Test blind und duerfte nicht still durchgehen.
        let cargo = std::env::var("CARGO").expect("CARGO ist im Testlauf gesetzt");
        let ausgabe = std::process::Command::new(&cargo)
            .args([
                "metadata",
                "--format-version",
                "1",
                "--locked",
                "--manifest-path",
                manifest,
            ])
            .output()
            .expect("cargo metadata laesst sich starten");
        assert!(
            ausgabe.status.success(),
            "cargo metadata --locked scheitert: {}",
            String::from_utf8_lossy(&ausgabe.stderr)
        );
        let baum: serde_json::Value =
            serde_json::from_slice(&ausgabe.stdout).expect("cargo metadata liefert JSON");
        let pakete = baum["packages"]
            .as_array()
            .expect("cargo metadata nennt packages");

        let eigen = pakete
            .iter()
            .find(|p| p["name"].as_str() == Some("eqcop-broker"))
            .and_then(|p| p["rust_version"].as_str())
            .expect("eqcop-broker deklariert rust-version");
        let eigen_stufen = version_stufen(eigen);

        let mut hoechste = eigen_stufen;
        let mut fordernde = String::from("eqcop-broker");
        for paket in pakete {
            let Some(boden) = paket["rust_version"].as_str() else {
                continue;
            };
            let stufen = version_stufen(boden);
            if stufen > hoechste {
                hoechste = stufen;
                fordernde = format!(
                    "{} {} ({boden})",
                    paket["name"].as_str().unwrap_or("?"),
                    paket["version"].as_str().unwrap_or("?")
                );
            }
        }

        assert!(
            hoechste <= eigen_stufen,
            "rust-version = \"{eigen}\" ist zu niedrig: {fordernde} verlangt mehr.              Der Compilerboden ist das Maximum ueber den gesperrten Baum"
        );
    }

    /// `1.85.0`, `1.85` und `1.85.0-beta` sollen vergleichbar sein, ohne eine
    /// Semver-Kiste in die Abhaengigkeiten zu ziehen. Fehlende Stellen zaehlen
    /// als 0, ein Vorabteil wird abgeschnitten.
    fn version_stufen(text: &str) -> (u64, u64, u64) {
        let kern = text.split(['-', '+']).next().unwrap_or(text);
        let mut teile = kern.split('.').map(|t| t.trim().parse::<u64>().unwrap_or(0));
        (
            teile.next().unwrap_or(0),
            teile.next().unwrap_or(0),
            teile.next().unwrap_or(0),
        )
    }
}
