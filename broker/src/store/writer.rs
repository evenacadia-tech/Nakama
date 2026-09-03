//! Der eine besitzende Schreiberthread.
//!
//! Genau dieser Thread haelt die Read/Write-Connection. Alles, was schreibt,
//! laeuft hier durch: Gruppen-Commit, Projektionen, Kompaktierung, Konfliktriegel
//! und Checkpoint.

use super::*;

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
                let idle_checkpoint_naht = konfiguration.idle_checkpoint_naht.clone();
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
                            idle_checkpoint_naht,
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

pub(super) enum WriterBefehl {
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

pub(super) struct AppendStand {
    rest: VecDeque<StoreEvent>,
    ergebnisse: Vec<AppendAusgang>,
    antwort: mpsc::Sender<Result<Vec<AppendAusgang>, StoreFehler>>,
}

pub(super) struct CommitJob {
    events: Vec<StoreEvent>,
}

pub(super) fn writer_lauf(
    mut conn: Connection,
    receiver: Receiver<WriterBefehl>,
    sicht: Arc<Mutex<StoreSicht>>,
    capture_aktiv: Arc<AtomicBool>,
    wal_pfad: PathBuf,
    test_haken: Option<StoreTestHaken>,
    idle_checkpoint_naht: Option<Arc<IdleCheckpointNaht>>,
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
                let naht_loest_aus = idle_checkpoint_naht
                    .as_ref()
                    .is_some_and(|n| n.sofort_ausloesen.load(Ordering::SeqCst));
                if (naht_loest_aus
                    || checkpoint_ausloesen(
                        wal_groesse(&wal_pfad),
                        letztes_event.elapsed(),
                        capture_aktiv.load(Ordering::SeqCst),
                    ))
                    && !idle_checkpoint_gelaufen
                {
                    // H-18: NUR ein erfolgreicher Checkpoint merkt sich den
                    // Erfolg. Bis NAK-121 wurde das Ergebnis verworfen und das
                    // Merkflag trotzdem gesetzt - ein gescheiterter Checkpoint
                    // wurde deshalb bis zum naechsten Event nie wiederholt,
                    // waehrend die Storesicht weiter gesund meldete.
                    let ergebnis = match &idle_checkpoint_naht {
                        Some(naht) => {
                            naht.versuche.fetch_add(1, Ordering::SeqCst);
                            if naht.fehler_erzwingen.load(Ordering::SeqCst) {
                                Err(StoreFehler::Degradiert("H-18-Naht".into()))
                            } else {
                                checkpoint(&conn, false)
                            }
                        }
                        None => checkpoint(&conn, false),
                    };
                    match ergebnis {
                        Ok(_) => idle_checkpoint_gelaufen = true,
                        Err(_) => {
                            if let Ok(mut s) = sicht.lock() {
                                s.checkpoints_gescheitert =
                                    s.checkpoints_gescheitert.saturating_add(1);
                            }
                        }
                    }
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

pub(super) fn append_gruppe(
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

pub(super) fn append_eins(tx: &Transaction<'_>, event: &StoreEvent) -> Result<AppendAusgang, StoreFehler> {
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

pub(super) fn projektionen_anwenden(
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

pub(super) fn kompaktieren(
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

pub(super) fn konflikt_guard_schreiben(
    conn: &mut Connection,
    guard: &ConflictGuard,
) -> Result<(), StoreFehler> {
    let tx = conn.transaction_with_behavior(TransactionBehavior::Immediate)?;
    // H-14: der Deckel greift VOR dem Einfuegen und nur fuer NEUE Riegel - ein
    // bereits vorhandener darf jederzeit erneut geschrieben werden, sonst
    // scheiterte ein Reconnect an der eigenen Grenze.
    let bereits_da: bool = tx
        .query_row(
            "SELECT 1 FROM conflict_guards WHERE effective_address=?1 AND derived_id=?2",
            params![guard.effective_address, guard.derived_id],
            |_| Ok(true),
        )
        .optional()?
        .unwrap_or(false);
    if !bereits_da {
        let anzahl: i64 =
            tx.query_row("SELECT COUNT(*) FROM conflict_guards", [], |row| row.get(0))?;
        if anzahl as usize >= MAX_KONFLIKT_GUARDS {
            return Err(StoreFehler::Degradiert(format!(
                "Konfliktriegel-Grenze von {MAX_KONFLIKT_GUARDS} erreicht"
            )));
        }
    }
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

pub(super) fn konflikt_guard_loeschen(
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

pub(super) fn checkpoint(conn: &Connection, truncate: bool) -> Result<CheckpointErgebnis, StoreFehler> {
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

#[cfg(test)]
mod tests {
    use super::migration::{migration_1, pragmas_setzen};
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
