//! Der Zugang fuer alle fremden Threads.
//!
//! Coordinator und I/O-Threads sehen den Store ausschliesslich durch dieses
//! Handle: begrenzte Auftraege hinein, Ergebnis heraus, kein Zugriff auf die
//! besitzende Connection.

use super::*;

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

    pub(super) fn warten(&self) {
        let (schloss, signal) = &*self.stand;
        let mut offen = schloss.lock().unwrap_or_else(|e| e.into_inner());
        while !*offen {
            offen = signal.wait(offen).unwrap_or_else(|e| e.into_inner());
        }
    }
}

/// H-18-Naht: macht den Leerlauf-Checkpoint in Millisekunden statt in
/// `STORE_IDLE_MS` messbar und laesst ihn auf Wunsch scheitern.
///
/// Ohne sie waere die eigentliche Zusage - ein gescheiterter Checkpoint merkt
/// sich keinen Erfolg und wird beim naechsten Leerlauf ERNEUT versucht - nur
/// ueber einen echten SQLite-Fehler beobachtbar, den ein Test nicht
/// verlaesslich herstellt.
#[doc(hidden)]
#[derive(Default)]
pub struct IdleCheckpointNaht {
    /// Loest den Leerlauf-Checkpoint ohne Ruecksicht auf WAL-Groesse und
    /// Leerlaufdauer aus.
    pub sofort_ausloesen: AtomicBool,
    /// Laesst ihn scheitern, als haette SQLite den Dienst verweigert.
    pub fehler_erzwingen: AtomicBool,
    /// Wie oft er versucht wurde. Der zweite Anstieg IST der Beweis der
    /// Wiederholung.
    pub versuche: AtomicU64,
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
    #[doc(hidden)]
    pub idle_checkpoint_naht: Option<Arc<IdleCheckpointNaht>>,
}

impl StoreKonfiguration {
    pub fn fuer_pfad(db_pfad: impl Into<PathBuf>) -> Self {
        Self {
            db_pfad: db_pfad.into(),
            remote_volume_override: None,
            start_barriere: None,
            test_haken: None,
            idle_checkpoint_naht: None,
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
    pub(super) fn erreichen(&self, punkt: &str) {
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
    pub(super) sender: Option<SyncSender<WriterBefehl>>,
    pub(super) sicht: Arc<Mutex<StoreSicht>>,
    pub(super) db_pfad: PathBuf,
    pub(super) capture_aktiv: Arc<AtomicBool>,
    /// 🔑 Naht fuer Befund B16: laesst JEDEN Append scheitern, als haette der
    /// Store den Dienst verweigert.
    ///
    /// Die Zusage „kein lokaler Ausschluss ohne persistierte Invalidierung"
    /// ist nur pruefbar, wenn ein Append verlaesslich scheitert. Ein echter
    /// SQLite-Fehler laesst sich nicht deterministisch herstellen — dieselbe
    /// Begruendung wie bei `IdleCheckpointNaht` daneben. Sie liegt VOR dem
    /// Kanal und beruehrt Guards, Checkpoints und Kompaktierung nicht.
    pub(super) append_naht: Arc<AtomicBool>,
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

    /// Schaltet die Append-Naht (Befund B16). Nur Tests rufen sie.
    #[doc(hidden)]
    pub fn append_naht_setzen(&self, an: bool) {
        self.append_naht.store(an, Ordering::SeqCst);
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
        if self.append_naht.load(Ordering::SeqCst) {
            return Err(StoreFehler::Degradiert(
                "Testnaht: der Append wird verweigert".into(),
            ));
        }
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

    /// Die gespeicherten Passagen, Experimente und Evidenzbelege einer
    /// Domaenentabelle (SONDE-013 M-50, Befund R12).
    ///
    /// 🔑 Nacharbeit 2: der Store schrieb diese drei Tabellen, und NIEMAND las
    /// sie. `Coordinator::mit_store` restaurierte ausschliesslich die
    /// Konfliktriegel; Experimentstore und Evidenzhistorie starteten immer
    /// leer. Nach einem Brokerneustart lieferte `experiment_sicht(id)` deshalb
    /// `None`, obwohl die SQLite-Zeile existierte — M-47 und M-50 sagen aber
    /// ausdruecklich zu, dass ein offener Versuch den Neustart ueberdauert.
    ///
    /// Der Tabellenname kommt aus einer GESCHLOSSENEN Menge, nie aus Store-
    /// oder Wiredaten: derselbe Riegel wie in `projektionen_anwenden`.
    pub fn domaene_lesen(&self, tabelle: Domaenentabelle) -> Result<Vec<Vec<u8>>, StoreFehler> {
        let sql = match tabelle {
            Domaenentabelle::Passages => {
                "SELECT state_jcs FROM passages ORDER BY last_event_ord"
            }
            Domaenentabelle::Experiments => {
                "SELECT state_jcs FROM experiments ORDER BY last_event_ord"
            }
            Domaenentabelle::Evidence => {
                "SELECT state_jcs FROM evidence ORDER BY last_event_ord"
            }
        };
        kurze_leseconnection(&self.db_pfad, |conn| {
            let mut stmt = conn.prepare(sql)?;
            let rows = stmt.query_map([], |row| row.get::<_, Vec<u8>>(0))?;
            rows.collect::<Result<Vec<_>, _>>()
                .map_err(StoreFehler::from)
        })
    }

    /// Die Payloads ALLER Experimenttransitionen in Ereignisreihenfolge
    /// (Befund B5, M-51).
    ///
    /// `experiment_events` indiziert jede Transition ueber ihre `event_uuid`;
    /// der Payload steht im `event_log`. Der Index allein sagt nur, DASS es
    /// eine Transition gab — erst der Payload macht sie exportierbar.
    pub fn experiment_ereignisse_lesen(&self) -> Result<Vec<Vec<u8>>, StoreFehler> {
        kurze_leseconnection(&self.db_pfad, |conn| {
            let mut stmt = conn.prepare(
                "SELECT l.payload_jcs FROM experiment_events e \
                 JOIN event_log l ON l.event_uuid = e.event_uuid \
                 ORDER BY l.event_ord",
            )?;
            let rows = stmt.query_map([], |row| row.get::<_, Vec<u8>>(0))?;
            rows.collect::<Result<Vec<_>, _>>()
                .map_err(StoreFehler::from)
        })
    }

    /// Der Payload EINES Ereignisses (Befund B17).
    ///
    /// Eine Outbox-Zeile nennt nur die `event_ord` ihrer Schuld. Ohne den
    /// Payload liesse sich die Schuld zaehlen, aber nicht abtragen — und genau
    /// das war der Zustand: `outbox_lesen` hatte ausserhalb der Tests keinen
    /// Aufrufer.
    pub fn event_payload_lesen(&self, event_ord: i64) -> Result<Option<Vec<u8>>, StoreFehler> {
        kurze_leseconnection(&self.db_pfad, |conn| {
            let mut stmt =
                conn.prepare("SELECT payload_jcs FROM event_log WHERE event_ord=?1")?;
            let mut rows = stmt.query([event_ord])?;
            match rows.next()? {
                Some(zeile) => Ok(Some(zeile.get::<_, Vec<u8>>(0)?)),
                None => Ok(None),
            }
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

/// Die Domaenentabellen, aus denen der Coordinator seinen Stand rekonstruiert.
///
/// Eine geschlossene Aufzaehlung statt eines Strings: ein Tabellenname aus
/// Store- oder Wiredaten waere derselbe Fehler wie ein SQL-Fragment daraus.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Domaenentabelle {
    Passages,
    Experiments,
    Evidence,
}

pub(super) fn kurze_leseconnection<T>(
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

pub(super) fn degradiere(sicht: &Arc<Mutex<StoreSicht>>, grund: String) {
    if let Ok(mut s) = sicht.lock() {
        s.degradiert = true;
        if s.grund.is_none() {
            s.grund = Some(grund);
        }
    }
}

pub(super) fn utc_ms_i64() -> i64 {
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_millis().min(i64::MAX as u128) as i64)
        .unwrap_or(0)
}
