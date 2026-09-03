//! Start, Acceptor und Instanzverwaltung.
//!
//! Der Acceptor haelt die bewaffneten Pipe-Instanzen und uebergibt jede
//! angenommene Verbindung an ihren eigenen Thread; er bedient selbst keine.

use super::*;

/// Deterministische Kanten fuer `broker/tests/security_vectors.rs`. Sie sind
/// kein alternativer Produktpfad: dieselbe Startfunktion faehrt bis unmittelbar
/// an die benannte Win32-/Threadkante und muss dann ihren normalen RAII-
/// Gegenpfad beweisen.
#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub enum V3StartTestFehler {
    #[default]
    Keiner,
    DirektNachErstemHandle,
    ZweiteInstanz,
    ZweiteArmierung,
    WachhundSpawn,
    AcceptorSpawn,
}

#[derive(Clone, Copy, Debug, Default, Eq, PartialEq)]
pub enum V3AuthTestFehler {
    #[default]
    Keiner,
    Impersonate,
    OpenThreadToken,
    TokenGroesse,
    TokenInformation,
    SidUngueltig,
    SidFremd,
    Revert,
}

#[derive(Default)]
pub struct V3UebergabeBarriere {
    erreicht: AtomicBool,
    freigegeben: Mutex<bool>,
    signal: Condvar,
}

impl V3UebergabeBarriere {
    pub fn erreicht(&self) -> bool {
        self.erreicht.load(Ordering::SeqCst)
    }

    pub fn freigeben(&self) {
        if let Ok(mut frei) = self.freigegeben.lock() {
            *frei = true;
            self.signal.notify_all();
        }
    }

    /// H-01-Naht: haelt den Destruktor des `Verbindungsgriff` GENAU zwischen
    /// Registeraustrag und `CloseHandle` an. In diesem Fenster darf das
    /// Register den Eintrag nicht mehr fuehren — anders als bei einer
    /// Spur-Assertion misst das die Reihenfolge selbst und nicht nur, wo zwei
    /// `push`-Aufrufe stehen.
    pub(super) fn im_destruktorfenster_warten(&self) {
        self.vor_worker_uebergabe_warten()
    }

    pub(super) fn vor_worker_uebergabe_warten(&self) {
        self.erreicht.store(true, Ordering::SeqCst);
        let mut frei = self.freigegeben.lock().unwrap_or_else(|e| e.into_inner());
        while !*frei {
            frei = self.signal.wait(frei).unwrap_or_else(|e| e.into_inner());
        }
    }
}

#[derive(Clone, Default)]
pub struct V3SecurityTestOptionen {
    pub start_fehler: V3StartTestFehler,
    pub auth_fehler: V3AuthTestFehler,
    pub ersatzlistener_fehler: bool,
    pub uebergabe_barriere: Option<Arc<V3UebergabeBarriere>>,
    /// Pro Startversuch isolierter Zaehler fuer A-01. Er misst, dass ein
    /// Rueckgabefehler keinen bereits gestarteten Wachhund-/Acceptor-Thread
    /// hinterlaesst; Produktaufrufe setzen ihn nicht.
    pub hilfsthread_zaehler: Option<Arc<AtomicUsize>>,
    /// H-01-Naht: traegt beim Start ein bereits geschlossenes Handle unter
    /// einer Phantom-ID ins Abbruchregister ein. Nur so laesst sich messen,
    /// dass ein `CancelIoEx` auf ein totes Handle GEZAEHLT und nicht
    /// verschluckt wird — im Produktbetrieb ist dieser Zustand strukturell
    /// unerreichbar, weil der Eintrag nur lebt, solange sein Besitzer lebt.
    pub totes_handle_ins_register: bool,
    /// H-02-Naht: laesst den Schreiberthread nach dem ersten Frame so lange
    /// stehen, dass `join_mit_frist` seine `SENKE_FRIST` verpasst. Damit ist
    /// die Ablosung erzwingbar, statt auf einen langsamen Peer zu hoffen.
    pub schreiber_haengt: Option<Arc<AtomicBool>>,
    /// H-01-Naht: haelt den Destruktor des Verbindungsgriffs zwischen
    /// Registeraustrag und `CloseHandle` an, damit der Test genau dieses
    /// Fenster messen kann.
    pub destruktor_fenster: Option<Arc<V3UebergabeBarriere>>,
    /// H-19-Naht: laesst den Verbindungsthread unmittelbar nach der Uebergabe
    /// panisieren. Der Zaehler ist sonst unbeweisbar, weil eine Panik dort im
    /// Normalbetrieb strukturell nicht vorkommt.
    pub verbindungsthread_panik: bool,
}

/// Legt die naechste Pipe-Instanz an — und gibt NICHT auf, wenn gerade alle
/// belegt sind.
///
/// Die alte Fassung brach die Acceptorschleife bei jedem Fehlschlag ab. An der
/// Verbindungsgrenze (`nMaxInstances`) heisst das: ein Peer oeffnet
/// `MAX_VERBINDUNGEN` Verbindungen, laesst sie wieder los — und danach horcht
/// niemand mehr, obwohl alle Plaetze frei sind (T2-Befund 6 vom 2026-08-29).
/// `ERROR_PIPE_BUSY` ist deshalb kein Ende, sondern ein Warten.
pub(super) fn naechste_instanz(
    name_w: &[u16],
    attrs: &SECURITY_ATTRIBUTES,
    stop: &AtomicBool,
    verbindungen: &Arc<Mutex<Vec<JoinHandle<()>>>>,
    statistik: &Arc<V3Statistik>,
    fehler_injiziert: bool,
) -> Result<ListenerInstanz, String> {
    if fehler_injiziert {
        return Err("Ersatzlistener-Fehler injiziert".into());
    }
    let mut fremde_fehler = 0u32;
    loop {
        if stop.load(Ordering::SeqCst) {
            return Err("Listenerstart wegen Serverstopp abgebrochen".into());
        }
        match ListenerInstanz::neu(name_w, attrs, false) {
            Ok(mut listener) => match listener.armieren() {
                Ok(()) => return Ok(listener),
                Err(f) => {
                    fremde_fehler += 1;
                    if fremde_fehler > 200 {
                        return Err(format!("ConnectNamedPipe Ersatzlistener: Win32 {f}"));
                    }
                }
            },
            Err(f) if f == ERROR_PIPE_BUSY => {
                statistik
                    .acceptor_wartet_auf_instanz
                    .fetch_add(1, Ordering::SeqCst);
            }
            Err(f) => {
                // Nicht die Verbindungsgrenze, sondern etwas anderes. Ein paar
                // Versuche sind billig; endlos zu drehen waere ein stiller Hang.
                fremde_fehler += 1;
                if fremde_fehler > 200 {
                    return Err(format!("CreateNamedPipe Ersatzlistener: Win32 {f}"));
                }
            }
        }
        // Ein beendeter Nachbar gibt seinen Platz erst frei, wenn sein Handle
        // wirklich zu ist — also hier ernten, nicht nur schlafen.
        fertige_ernten(verbindungen, statistik);
        std::thread::sleep(Duration::from_millis(25));
    }
}

// Deterministische Testphase fuer das Cancel-vor-Read-Interleaving. Im
// Produktpfad bleibt sie INAKTIV; ein Test armiert genau einen folgenden Read.
pub(super) const CANCEL_VOR_READ_BEREIT: u64 = 0;

pub(super) const CANCEL_VOR_READ_GELESEN: u64 = 1;

pub(super) const CANCEL_VOR_READ_READER: u64 = 2;

pub(super) const CANCEL_VOR_READ_WRITER: u64 = 3;

pub(super) const CANCEL_VOR_READ_FORTGESETZT: u64 = 4;

pub(super) const CANCEL_VOR_READ_INAKTIV: u64 = 5;

pub(super) const CANCEL_VOR_READ_FEHLER: u64 = 6;

/// Startet den v3-Listener auf `pipe_name`.
///
/// Der Aufrufer waehlt den Namen — im Test und in der Probe ist es ein
/// Probe-Name, nie die Produktions-Pipe.
pub fn v3_server_starten(
    pipe_name: &str,
    senke: Arc<dyn Senke>,
    broker_version: String,
) -> Result<V3Griff, String> {
    v3_server_starten_mit_epoch(pipe_name, senke, broker_version, neue_kennung())
}

pub fn v3_server_starten_mit_epoch(
    pipe_name: &str,
    senke: Arc<dyn Senke>,
    broker_version: String,
    broker_epoch: String,
) -> Result<V3Griff, String> {
    v3_server_starten_mit_epoch_und_sender(
        pipe_name,
        senke,
        broker_version,
        broker_epoch,
        V3Sender::neu(),
    )
}

pub fn v3_server_starten_mit_epoch_und_sender(
    pipe_name: &str,
    senke: Arc<dyn Senke>,
    broker_version: String,
    broker_epoch: String,
    sender: V3Sender,
) -> Result<V3Griff, String> {
    v3_server_starten_intern(
        pipe_name,
        senke,
        broker_version,
        broker_epoch,
        sender,
        Arc::new(AtomicU64::new(0)),
        Arc::new(AtomicBool::new(false)),
        Arc::new(AtomicU64::new(CANCEL_VOR_READ_INAKTIV)),
        false,
        V3SecurityTestOptionen::default(),
    )
}

#[doc(hidden)]
pub fn v3_server_starten_fuer_security_vectors(
    pipe_name: &str,
    senke: Arc<dyn Senke>,
    optionen: V3SecurityTestOptionen,
) -> Result<V3Griff, String> {
    v3_server_starten_intern(
        pipe_name,
        senke,
        "security-vectors".into(),
        neue_kennung(),
        V3Sender::neu(),
        Arc::new(AtomicU64::new(0)),
        Arc::new(AtomicBool::new(false)),
        Arc::new(AtomicU64::new(CANCEL_VOR_READ_INAKTIV)),
        true,
        optionen,
    )
}

/// Wie `v3_server_starten`, aber mit einer Testnaht: `probe_verzoegerung_ms`
/// laesst jeden frisch angenommenen Verbindungsthread vor seiner ersten Arbeit
/// warten. Damit trifft ein Test das Fenster zwischen Annahme und Bedienung
/// deterministisch, statt es zu erwuerfeln. In Produktion ist der Wert 0.
pub(super) fn v3_server_starten_intern(
    pipe_name: &str,
    senke: Arc<dyn Senke>,
    broker_version: String,
    broker_epoch: String,
    sender: V3Sender,
    probe_verzoegerung_ms: Arc<AtomicU64>,
    writer_fehler_erzwungen: Arc<AtomicBool>,
    cancel_vor_read_phase: Arc<AtomicU64>,
    sicherheits_spur_aktiv: bool,
    security_optionen: V3SecurityTestOptionen,
) -> Result<V3Griff, String> {
    let sicherheit = Arc::new(crate::server::sicherheit_nur_user()?);
    let stop = Arc::new(AtomicBool::new(false));
    let acceptor_stop = Arc::new(
        EndeSignal::neu()
            .ok_or_else(|| "CreateEvent v3-Acceptorstop fehlgeschlagen".to_string())?,
    );
    let rest_listener: Arc<Mutex<Vec<ListenerInstanz>>> = Arc::new(Mutex::new(Vec::new()));
    let aktive_worker = Arc::new(AtomicUsize::new(0));
    let sicherheits_spur = Arc::new(SicherheitsSpur::neu(sicherheits_spur_aktiv));
    let verbindungen: Arc<Mutex<Vec<JoinHandle<()>>>> = Arc::new(Mutex::new(Vec::new()));
    let handles: Arc<Mutex<HandleRegister>> = Arc::new(Mutex::new(HandleRegister::default()));
    let bootstraps: Arc<Mutex<Vec<(u64, Instant)>>> = Arc::new(Mutex::new(Vec::new()));
    let statistik = Arc::new(V3Statistik::default());
    let kopplungen = Arc::new(Mutex::new(Kopplungen::neu()));
    let closer = V3Closer {
        kopplungen: kopplungen.clone(),
        handles: handles.clone(),
    };
    // Wartepunkte der Trennreihenfolge, einer je lebender Kopplung (`C-LS-06`).
    let trennmelder: TrennRegister = Arc::new(Mutex::new(HashMap::new()));
    let control_ausgaenge = sender.control_ausgaenge.clone();
    let telemetrie_ausgaenge = sender.telemetrie_ausgaenge.clone();

    let mut name_w: Vec<u16> = pipe_name.encode_utf16().collect();
    name_w.push(0);

    let attrs = SECURITY_ATTRIBUTES {
        nLength: std::mem::size_of::<SECURITY_ATTRIBUTES>() as u32,
        lpSecurityDescriptor: sicherheit.deskriptor,
        bInheritHandle: 0,
    };

    // A-01: beide Besitzhandles werden synchron per RAII erzeugt UND mit
    // ConnectNamedPipe bewaffnet, bevor irgendein Servergriff sichtbar wird.
    let mut erster = ListenerInstanz::neu(&name_w, &attrs, true)
        .map_err(|f| format!("CreateNamedPipe v3 (erste Instanz): Win32 {f}"))?;
    if security_optionen.start_fehler == V3StartTestFehler::DirektNachErstemHandle {
        return Err("Startfehler direkt nach erstem Handle injiziert".into());
    }
    erster
        .armieren()
        .map_err(|f| format!("ConnectNamedPipe v3 (erste Instanz): Win32 {f}"))?;
    if security_optionen.start_fehler == V3StartTestFehler::ZweiteInstanz {
        return Err("Startfehler zweite Instanz injiziert".into());
    }
    let mut zweiter = ListenerInstanz::neu(&name_w, &attrs, false)
        .map_err(|f| format!("CreateNamedPipe v3 (zweite Instanz): Win32 {f}"))?;
    zweiter
        .armieren()
        .map_err(|f| format!("ConnectNamedPipe v3 (zweite Instanz): Win32 {f}"))?;
    if security_optionen.start_fehler == V3StartTestFehler::ZweiteArmierung {
        // Beide OVERLAPPED-Connects stehen bereits. Dieser Gegenpfad misst
        // deshalb nicht nur zwei rohe Handles, sondern auch Cancel +
        // Completion vor dem Freigeben ihrer stabilen OVERLAPPED-Speicher.
        return Err("Startfehler nach zweiter Armierung injiziert".into());
    }
    statistik.bewaffnete_listener.store(2, Ordering::SeqCst);

    if security_optionen.totes_handle_ins_register {
        // H-01-Naht: ein Event-Handle, sofort geschlossen, danach unter einer
        // Phantom-ID ins Abbruchregister. Der naechste Wachhundtick feuert
        // `CancelIoEx` darauf und muss den Fehlschlag ZAEHLEN. Ohne diese Naht
        // waere der Zaehler unbeweisbar: im Produktbetrieb lebt ein
        // Registereintrag nur, solange sein Besitzer lebt (H-01).
        if let Some(e) = Ereignis::neu() {
            let roh = e.roh();
            drop(e);
            let mut r = handles.lock().unwrap_or_else(|x| x.into_inner());
            r.offen.push((u64::MAX, roh as isize));
            // Ohne den Eintrag in `abgeloest` liefe der Wachhund an der
            // Phantom-ID vorbei: er bricht nur faellige Bootstraps und
            // abgeloeste Verbindungen ab.
            r.abgeloest.insert(u64::MAX);
        }
    }

    if security_optionen.start_fehler == V3StartTestFehler::WachhundSpawn {
        return Err("Wachhund-Spawnfehler injiziert".into());
    }

    let stop_w = stop.clone();
    let handles_w = handles.clone();
    let bootstraps_w = bootstraps.clone();
    let verbindungen_w = verbindungen.clone();
    let wachhund_testzaehler = security_optionen.hilfsthread_zaehler.clone();
    let statistik_w = statistik.clone();
    let wachhund = std::thread::Builder::new()
        .name("eqcop-v3-wachhund".into())
        .spawn(move || {
            let _lebend = TestHilfsthread::neu(wachhund_testzaehler);
            while !stop_w.load(Ordering::SeqCst) {
                std::thread::sleep(WACHHUND_TAKT);
                // Auch ohne neue Verbindung muessen fertige Threads fallen.
                fertige_ernten(&verbindungen_w, &statistik_w);
                let jetzt = Instant::now();
                let faellig: Vec<u64> = {
                    let b = bootstraps_w.lock().unwrap_or_else(|e| e.into_inner());
                    b.iter()
                        .filter(|(_, f)| *f <= jetzt)
                        .map(|(id, _)| *id)
                        .collect()
                };
                // H-02: neben den faelligen Bootstraps bricht derselbe Tick
                // jede ABGELOESTE Verbindung ab, und zwar bei JEDEM Tick
                // erneut, bis ihr Thread endet und der letzte geteilte Griff
                // faellt. Ohne diesen Weg blieb ein abgeloester etablierter
                // Schreiber bis zum Serverstopp haengen: `BootstrapFrist` hat
                // seine ID laengst ausgetragen, und der Wachhund sah nur
                // faellige Bootstrap-IDs.
                if let Ok(reg) = handles_w.lock() {
                    if faellig.is_empty() && reg.abgeloest.is_empty() {
                        continue;
                    }
                    for (id, h) in reg.offen.iter() {
                        if faellig.contains(id) || reg.abgeloest.contains(id) {
                            abbrechen_und_zaehlen_extern(*h as HANDLE, &statistik_w);
                        }
                    }
                }
            }
        })
        .map_err(|e| format!("Wachhundthread: {e}"))?;

    if security_optionen.start_fehler == V3StartTestFehler::AcceptorSpawn {
        stop.store(true, Ordering::SeqCst);
        acceptor_stop.setzen();
        let _ = wachhund.join();
        return Err("Acceptor-Spawnfehler injiziert".into());
    }

    let stop2 = stop.clone();
    let acceptor_stop2 = acceptor_stop.clone();
    let rest_listener2 = rest_listener.clone();
    let aktive_worker2 = aktive_worker.clone();
    let verbindungen2 = verbindungen.clone();
    let handles2 = handles.clone();
    let bootstraps2 = bootstraps.clone();
    let statistik2 = statistik.clone();
    let trennmelder2 = trennmelder.clone();
    let control_ausgaenge2 = control_ausgaenge.clone();
    let telemetrie_ausgaenge2 = telemetrie_ausgaenge.clone();
    let sicherheit2 = sicherheit.clone();
    let security_optionen2 = security_optionen.clone();
    let sicherheits_spur2 = sicherheits_spur.clone();
    let acceptor_testzaehler = security_optionen.hilfsthread_zaehler.clone();
    let acceptor_ergebnis = std::thread::Builder::new()
        .name("eqcop-v3-acceptor".into())
        .spawn(move || {
            let _lebend = TestHilfsthread::neu(acceptor_testzaehler);
            let attrs = SECURITY_ATTRIBUTES {
                nLength: std::mem::size_of::<SECURITY_ATTRIBUTES>() as u32,
                lpSecurityDescriptor: sicherheit2.deskriptor,
                bInheritHandle: 0,
            };
            let mut listener = [Some(erster), Some(zweiter)];
            let mut zusaetzlich = Vec::new();
            let mut folge: u64 = 0;
            loop {
                if stop2.load(Ordering::SeqCst) {
                    break;
                }
                let warte_handles = [
                    listener[0].as_ref().expect("Listener 0").ereignis(),
                    listener[1].as_ref().expect("Listener 1").ereignis(),
                    acceptor_stop2.roh(),
                ];
                // SAFETY: alle drei Eventhandles leben ueber den Wait.
                let ausgang = unsafe {
                    WaitForMultipleObjects(
                        warte_handles.len() as u32,
                        warte_handles.as_ptr(),
                        0,
                        INFINITE,
                    )
                };
                if stop2.load(Ordering::SeqCst) || ausgang == WAIT_OBJECT_0 + 2 {
                    break;
                }
                if ausgang != WAIT_OBJECT_0 && ausgang != WAIT_OBJECT_0 + 1 {
                    statistik2.listener_fehler.fetch_add(1, Ordering::SeqCst);
                    stop2.store(true, Ordering::SeqCst);
                    acceptor_stop2.setzen();
                    break;
                }
                let index = (ausgang - WAIT_OBJECT_0) as usize;
                let mut angenommen = listener[index].take().expect("signalisierter Listener");
                statistik2
                    .bewaffnete_listener
                    .fetch_sub(1, Ordering::SeqCst);
                if !angenommen.verbindung_fertig() {
                    zusaetzlich.push(angenommen);
                    statistik2.listener_fehler.fetch_add(1, Ordering::SeqCst);
                    stop2.store(true, Ordering::SeqCst);
                    acceptor_stop2.setzen();
                    alle_io_abbrechen(&handles2);
                    break;
                }
                sicherheits_spur2.push("connect");
                statistik2.angenommen.fetch_add(1, Ordering::SeqCst);

                let Some(worker_platz) = worker_reservieren(&aktive_worker2) else {
                    // A-05: der 97. Client wird geschlossen, solange der
                    // andere Listener den Namen besitzt. Erst danach entsteht
                    // seine neue Reserve; kein Worker sieht diesen Handle.
                    drop(angenommen);
                    statistik2
                        .am_worker_cap_abgewiesen
                        .fetch_add(1, Ordering::SeqCst);
                    match naechste_instanz(
                        &name_w,
                        &attrs,
                        &stop2,
                        &verbindungen2,
                        &statistik2,
                        security_optionen2.ersatzlistener_fehler,
                    ) {
                        Ok(ersatz) => {
                            listener[index] = Some(ersatz);
                            statistik2
                                .bewaffnete_listener
                                .fetch_add(1, Ordering::SeqCst);
                        }
                        Err(_) => {
                            statistik2.listener_fehler.fetch_add(1, Ordering::SeqCst);
                            stop2.store(true, Ordering::SeqCst);
                            acceptor_stop2.setzen();
                            alle_io_abbrechen(&handles2);
                            break;
                        }
                    }
                    continue;
                };

                // A-03/A-04: der angenommene Handle und der zweite Listener
                // bleiben beim Acceptor, bis der Ersatz ERZEUGT UND BEWAFFNET
                // ist. Erst danach beginnt irgendeine Worker-Uebergabe.
                match naechste_instanz(
                    &name_w,
                    &attrs,
                    &stop2,
                    &verbindungen2,
                    &statistik2,
                    security_optionen2.ersatzlistener_fehler,
                ) {
                    Ok(ersatz) => {
                        listener[index] = Some(ersatz);
                        statistik2
                            .bewaffnete_listener
                            .fetch_add(1, Ordering::SeqCst);
                    }
                    Err(_) => {
                        drop(worker_platz);
                        zusaetzlich.push(angenommen);
                        statistik2.listener_fehler.fetch_add(1, Ordering::SeqCst);
                        stop2.store(true, Ordering::SeqCst);
                        acceptor_stop2.setzen();
                        alle_io_abbrechen(&handles2);
                        break;
                    }
                }
                if let Some(barriere) = &security_optionen2.uebergabe_barriere {
                    barriere.vor_worker_uebergabe_warten();
                }
                if stop2.load(Ordering::SeqCst) {
                    drop(worker_platz);
                    zusaetzlich.push(angenommen);
                    break;
                }

                folge += 1;
                let id = folge;
                let verbundenes_handle = angenommen.handle_uebernehmen();

                // Das Handle geht ins Abbruchregister, BEVOR der Thread
                // existiert. Lief `stoppen()` frueher zwischen Spawn und
                // Registrierung, sahen beide `alle_io_abbrechen`-Aufrufe ein
                // leeres Register; der Thread startete danach, trug sein
                // Handle ein und blockierte ohne verbliebenen Wachhund im
                // Bootstrap-Read (T2-Befund 6 vom 2026-08-29). Jetzt gilt:
                // entweder der Stop erfasst das Handle, oder der Thread sieht
                // `stop` selbst und endet.
                {
                    let mut r = handles2.lock().unwrap_or_else(|e| e.into_inner());
                    r.offen.push((id, verbundenes_handle as isize));
                }
                // H-01: der Griff wird ERST NACH dem Registereintrag gebaut und
                // besitzt ihn ab hier mit. Scheitert `spawn`, faellt die
                // Closure, faellt der letzte Griff, und sein Destruktor traegt
                // unter der Registersperre aus, bevor er schliesst. Bis NAK-121
                // trug ein getrennter `HandleEintrag` den Austrag; seine
                // Fallreihenfolge gegen den Griff war unbestimmt.
                let griff = Arc::new(Verbindungsgriff {
                    h: verbundenes_handle,
                    sicherheits_spur: sicherheits_spur2.clone(),
                    id,
                    register: handles2.clone(),
                    destruktor_fenster: security_optionen2.destruktor_fenster.clone(),
                });
                let verzoegerung = probe_verzoegerung_ms.clone();
                let writer_fehler = writer_fehler_erzwungen.clone();
                let cancel_vor_read = cancel_vor_read_phase.clone();

                let senke = senke.clone();
                let kopplungen = kopplungen.clone();
                let trennmelder = trennmelder2.clone();
                let handles = handles2.clone();
                let bootstraps = bootstraps2.clone();
                let statistik = statistik2.clone();
                let bv = broker_version.clone();
                let be = broker_epoch.clone();
                let conn_stop = stop2.clone();
                let control_ausgaenge = control_ausgaenge2.clone();
                let telemetrie_ausgaenge = telemetrie_ausgaenge2.clone();
                let erwartete_sicherheit = sicherheit2.clone();
                let auth_fehler = security_optionen2.auth_fehler;
                let sicherheits_spur = sicherheits_spur2.clone();
                let schreiber_haengt = security_optionen2.schreiber_haengt.clone();
                let panik_naht = security_optionen2.verbindungsthread_panik;
                match std::thread::Builder::new()
                    .name("eqcop-v3-conn".into())
                    .spawn(move || {
                        let _worker_platz = worker_platz;
                        // Testnaht: reisst das Fenster zwischen Annahme und
                        // erster Arbeit deterministisch auf. In Produktion
                        // steht hier 0 und der Aufruf kostet einen Ladevorgang.
                        let ms = verzoegerung.load(Ordering::SeqCst);
                        if ms > 0 {
                            std::thread::sleep(Duration::from_millis(ms));
                        }
                        if panik_naht {
                            panic!("H-19-Naht: absichtliche Panik im Verbindungsthread");
                        }
                        verbindung_bedienen(
                            id,
                            griff,
                            senke,
                            kopplungen,
                            trennmelder,
                            handles,
                            bootstraps,
                            statistik,
                            bv,
                            be,
                            conn_stop,
                            writer_fehler,
                            cancel_vor_read,
                            control_ausgaenge,
                            telemetrie_ausgaenge,
                            erwartete_sicherheit,
                            auth_fehler,
                            sicherheits_spur,
                            schreiber_haengt,
                        );
                    }) {
                    Ok(j) => {
                        statistik2.worker_uebergaben.fetch_add(1, Ordering::SeqCst);
                        verbindungen2
                            .lock()
                            .unwrap_or_else(|e| e.into_inner())
                            .push(j)
                    }
                    Err(_) => {
                        // Closure-Drop gibt Workerplatz und Griff frei; dessen
                        // Destruktor traegt den Registereintrag aus und
                        // schliesst danach (H-01). Beide Listener sind bereits
                        // wieder bewaffnet.
                    }
                }

                fertige_ernten(&verbindungen2, &statistik2);
            }

            // A-09: Nicht hier schliessen. Der Griff joint erst Acceptor,
            // Wachhund und alle Worker; DANACH leert er dieses Register als
            // letzten Pipebesitz. Auch ein fataler Ersatzfehler behaelt damit
            // den Namen bis zum geordneten Gegenpfad.
            let mut rest = rest_listener2.lock().unwrap_or_else(|e| e.into_inner());
            for slot in listener.into_iter().flatten() {
                rest.push(slot);
            }
            rest.append(&mut zusaetzlich);
        });
    let acceptor = match acceptor_ergebnis {
        Ok(j) => j,
        Err(e) => {
            stop.store(true, Ordering::SeqCst);
            acceptor_stop.setzen();
            let _ = wachhund.join();
            return Err(format!("v3-Acceptorthread: {e}"));
        }
    };

    Ok(V3Griff {
        stop,
        pipe_name: pipe_name.to_string(),
        acceptor_stop,
        acceptor: Some(acceptor),
        rest_listener,
        verbindungen,
        handles,
        bootstraps,
        closer,
        wachhund: Some(wachhund),
        sender,
        sicherheits_spur,
        uebergabe_barriere: security_optionen.uebergabe_barriere,
        worker_zaehler: aktive_worker,
        statistik,
    })
}
