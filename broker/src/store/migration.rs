//! Schema, Migration und Pruefsumme der lokalen SQLite-Wahrheit.
//!
//! Ein fremdes oder neueres Schema wird erkannt, bevor es angefasst wird; die
//! Pruefsumme des Migrationsskripts ist die Zusage, dass beide Seiten dasselbe
//! Schema meinen.

use super::{
    geoeffnete_db_volume, projektionen_anwenden, store_pfad_ist_remote, utc_ms_i64, ConflictGuard,
    StoreEvent,
    StoreFehler, StoreKonfiguration, StorePragmas, StoreTestHaken, BUSY_TIMEOUT_MS,
    STORE_SCHEMA_MAJOR,
};
use rusqlite::{params, Connection, OpenFlags, OptionalExtension, TransactionBehavior};
use sha2::{Digest, Sha256};
use std::path::Path;
use std::time::Duration;

pub(super) const MIGRATION_1_SQL: &str = r#"
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

pub fn migration_1_checksum() -> String {
    let mut hash = Sha256::new();
    hash.update(MIGRATION_1_SQL.as_bytes());
    hash.finalize().iter().map(|b| format!("{b:02x}")).collect()
}

pub(super) fn store_vorbereiten(
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
    // D10 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): DIE
    // Volumenentscheidung faellt hier, am geoeffneten Datenbankobjekt.
    //
    // Die Vorpruefung oben klassifiziert einen Pfadnamen und sucht dafuer den
    // naechsten vorhandenen Vorfahren. Beim ersten Start mit fehlenden
    // Komponenten - oder bei einem Austausch zwischen Pruefung und
    // `create_dir_all` beziehungsweise diesem Open - wird damit ein anderes
    // Objekt geoeffnet als geprueft. Jetzt wird das GEOEFFNETE Objekt
    // klassifiziert, bevor der Store es benutzt.
    if konfiguration.remote_volume_override.is_none() {
        let (endgueltig, remote_am_objekt) = geoeffnete_db_volume(&konfiguration.db_pfad)?;
        if remote_am_objekt {
            return Err(StoreFehler::Pfad(format!(
                "Remote-Volume am geoeffneten Objekt ({}): DB und WAL werden dort nicht benutzt",
                endgueltig.display()
            )));
        }
    }
    pragmas_setzen(&conn)?;
    migration_1(&mut conn, konfiguration.test_haken.as_ref())?;
    let guards = konflikt_guards_lesen(&conn)?;
    Ok((conn, guards))
}

pub(super) fn pragmas_setzen(conn: &Connection) -> Result<(), StoreFehler> {
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

pub(super) fn pragmas_lesen(conn: &Connection) -> Result<StorePragmas, StoreFehler> {
    Ok(StorePragmas {
        journal_mode: conn.query_row("PRAGMA journal_mode", [], |row| row.get(0))?,
        synchronous: conn.query_row("PRAGMA synchronous", [], |row| row.get(0))?,
        wal_autocheckpoint: conn.query_row("PRAGMA wal_autocheckpoint", [], |row| row.get(0))?,
        busy_timeout_ms: conn.query_row("PRAGMA busy_timeout", [], |row| row.get(0))?,
        foreign_keys: conn.query_row("PRAGMA foreign_keys", [], |row| row.get(0))?,
    })
}

pub(super) fn migration_1(
    conn: &mut Connection,
    test_haken: Option<&StoreTestHaken>,
) -> Result<(), StoreFehler> {
    let checksum = migration_1_checksum();
    let tx = conn.transaction_with_behavior(TransactionBehavior::Immediate)?;

    // G2-TOCTOU-001: BEURTEILEN, DANN ANFASSEN. Bis NAK-121 lief
    // `execute_batch` zuerst und der Pruefsummenvergleich danach - ein fremdes
    // Schema wurde also erst angefasst und erst dann abgelehnt. Der Rollback
    // rettete das Ergebnis, aber die Reihenfolge selbst ist die Zusage.
    let tabelle_vorhanden: bool = tx
        .query_row(
            "SELECT 1 FROM sqlite_master WHERE type='table' AND name='schema_migrations'",
            [],
            |_| Ok(true),
        )
        .optional()?
        .unwrap_or(false);
    let vorhanden: Option<String> = if tabelle_vorhanden {
        tx.query_row(
            "SELECT checksum_sha256 FROM schema_migrations WHERE major=?1",
            [STORE_SCHEMA_MAJOR],
            |row| row.get::<_, String>(0),
        )
        .optional()?
    } else {
        None
    };
    if let Some(gespeichert) = &vorhanden {
        if gespeichert != &checksum {
            return Err(StoreFehler::Degradiert(
                "Migration 1 hat einen fremden Checksum-Stand".into(),
            ));
        }
    }

    tx.execute_batch(MIGRATION_1_SQL)?;
    if vorhanden.is_none() {
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

pub(super) fn bekanntes_schema_readonly(pfad: &Path) -> Result<Option<i64>, StoreFehler> {
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

pub(super) fn konflikt_guards_lesen(conn: &Connection) -> Result<Vec<ConflictGuard>, StoreFehler> {
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
