//! EQ-Copilot-Broker — M2 (Herkunft: Recherche §8.4, §9.2, §11, heute Archiv unter docs/archiv/; Produktwahrheit: CLAUDE.md).
//!
//! Eigener Scope neben den Katalog-/Bausteinmodellen (Plan §10.2): Named-Pipe-
//! Server für die Sammler-VST3-Instanzen, Sensor-Register mit Last-Seen,
//! Konflikt- und Stale-Sichtbarkeit, PRE/POST-Paar-Auswertung, Profilbindung
//! und der zeitausgerichtete Aggregat-Snapshot. Kein Audio, keine PCM-
//! Übertragung — Messwerte kommen als kompakter Messstand im v2-Heartbeat.
//! Die Verträge liegen in eq-copilot/schemas/.

pub mod aggregat;
pub mod bindung;
pub mod coordinator;
pub mod dto;
pub mod framing;
pub mod generiert;
pub mod instance_alias;
pub mod protokoll;
#[cfg(windows)]
mod server;
pub mod store;
pub mod telemetrie;
pub mod transport;
pub mod vertrag;

mod paar;
mod register;
mod sitzung;
mod zeit;
#[cfg(test)]
mod testhilfe;

pub use paar::*;
pub use register::*;
pub use sitzung::*;
pub use zeit::*;

use serde::Serialize;
use std::path::PathBuf;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex, OnceLock};
use std::thread::JoinHandle;
use std::time::{Duration, Instant};

/// Fester Pipename v1; Zugriff regelt die ACL (nur aktueller Windows-User).
/// Muss mit eqcop::kPipeName im Plugin übereinstimmen. Der Name bleibt auch
/// unter Protokoll v2 stabil — die Version handelt der Handshake aus.
pub const PIPE_NAME: &str = r"\\.\pipe\evenacadia.eq-copilot.v1";
pub const BROKER_PRO_USER_MAX: usize = 1;
pub const BROKER_IDLE_ENDE_MS: u64 = 60_000;

/// H-13: Frist, nach der eine getrennte v2-Sensor-ID ihren Registerplatz
/// zurueckgibt. Gleich dem Leerlaufhorizont, den der Broker fuer sich selbst
/// fuehrt - wer laenger fort ist, als der Broker ohne jeden Sensor
/// weiterlaeuft, hat keinen Anspruch mehr auf einen Platz.
pub const SENSOR_ID_RUECKGABE_MS: u64 = BROKER_IDLE_ENDE_MS;

#[derive(Debug, Clone, Serialize)]
pub struct BrokerStatus {
    pub laeuft: bool,
    pub broker_version: String,
    pub protokoll_version: u32,
    pub pipe_name: String,
    /// Nur die ersten 8 Zeichen — genug zum Wiedererkennen in Logs.
    pub session_token_kurz: String,
    pub gestartet_ms: u64,
    pub pakete_verworfen: u64,
    pub fehler: Vec<String>,
    pub sensoren: Vec<SensorEintrag>,
    pub sessions: Vec<SessionInfo>,
    pub paare: Vec<PaarStatus>,
}

struct BrokerSupervisor {
    stop: Arc<AtomicBool>,
    join: Option<JoinHandle<()>>,
}

impl Drop for BrokerSupervisor {
    fn drop(&mut self) {
        self.stop.store(true, Ordering::SeqCst);
        if let Some(join) = self.join.take() {
            let _ = join.join();
        }
    }
}

struct BrokerLauf {
    #[cfg(windows)]
    _supervisor: Mutex<Option<BrokerSupervisor>>,
    #[cfg(windows)]
    _griff_v2: Mutex<Option<server::ServerGriff>>,
    #[cfg(windows)]
    _griff_v3: Mutex<Option<transport::server_v3::V3Griff>>,
    /// H-09: dieselbe `Mutex<Option<…>>`-Huelle wie bei den drei Geschwistern
    /// darueber. Bis NAK-121 lag der Storeschreiber hier blank, und der
    /// geordnete Stopp nahm nur die Serverhandles - der Schreiberthread starb
    /// am Prozessende ohne geordnetes Schliessen. Die Huelle erlaubt das
    /// Entnehmen; der vorhandene Destruktor von `StoreWriter` fuehrt den Stopp
    /// dann selbst aus, weil `stoppen` eine exklusive Referenz verlangt und der
    /// Brokerlauf nur geteilt aus einer statischen Zelle erreichbar ist.
    #[cfg(windows)]
    store: Mutex<Option<store::StoreWriter>>,
    /// Nur Lebensdauerhalter. Seit NAK-123 liest die Idle-Entscheidung die
    /// aktiven Worker am `V3Griff` (auch unvollstaendige Bootstraps zaehlen),
    /// nicht mehr `Coordinator::client_anzahl`. Der Arc muss aber weiter im
    /// Prozess leben, weil der `nakama-coordinator-tick`-Thread und die
    /// v3-Senken auf Klonen davon arbeiten; deshalb dieselbe
    /// Unterstrich-Konvention wie bei `_supervisor`/`_griff_v2`/`_griff_v3`.
    #[cfg(windows)]
    _coordinator: Arc<coordinator::Coordinator>,
    register: Arc<Mutex<Register>>,
    session_token: String,
    gestartet_ms: u64,
    bindungen_pfad: Option<PathBuf>,
    idle_seit: Mutex<Option<Instant>>,
    beendet: AtomicBool,
}

static BROKER: OnceLock<Result<BrokerLauf, String>> = OnceLock::new();

fn broker_version() -> String {
    env!("CARGO_PKG_VERSION").to_string()
}

/// Startet den Broker genau einmal pro Prozess. Ein Fehler bleibt sichtbar im
/// Status stehen — die App läuft weiter, das Plugin misst lokal (Plan §6.5).
/// `bindungen_pfad` ist die Persistenzdatei der Profilbindungen (None = nur
/// im Speicher, z. B. in Tests).
pub fn broker_starten(bindungen_pfad: Option<PathBuf>) -> Result<(), String> {
    let ergebnis = BROKER.get_or_init(|| {
        #[cfg(windows)]
        {
            let mut register = Register::default();
            if let Some(pfad) = &bindungen_pfad {
                match bindung::bindungen_laden(pfad) {
                    Ok(b) => register.bindungen = b,
                    Err(e) => register.fehler_merken(format!("Profilbindungen: {e}")),
                }
            }
            let register = Arc::new(Mutex::new(register));
            let session_token = uuid::Uuid::new_v4().to_string();
            let store = match store::StoreKonfiguration::standard() {
                Ok(konfiguration) => store::StoreWriter::starten(konfiguration),
                Err(fehler) => store::StoreWriter::degradiert_ohne_pfad(fehler.to_string()),
            };
            let broker_epoch = uuid::Uuid::new_v4().simple().to_string();
            let coordinator = Arc::new(coordinator::Coordinator::instant_mit_store(
                broker_epoch.clone(),
                &store,
            ));
            let griff_v2 = server::server_starten_mit_interventionssenke(
                PIPE_NAME,
                register.clone(),
                broker_version(),
                session_token.clone(),
                coordinator.clone(),
            )?;
            let sender = transport::server_v3::V3Sender::neu();
            coordinator.session_push_setzen(Arc::new(sender.clone()));
            let user_sid = server::aktueller_user_sid()?;
            let pipe_v3 = transport::pipetoken::pipe_name_v3(&user_sid);
            let griff_v3 = transport::server_v3::v3_server_starten_mit_epoch_und_sender(
                &pipe_v3,
                coordinator.clone(),
                broker_version(),
                broker_epoch,
                sender,
            )?;
            let supervisor_stop = Arc::new(AtomicBool::new(false));
            let stop_fuer_thread = supervisor_stop.clone();
            let coordinator_fuer_thread = coordinator.clone();
            let closer = griff_v3.closer();
            let supervisor_join = std::thread::Builder::new()
                .name("nakama-coordinator-tick".into())
                .spawn(move || {
                    while !stop_fuer_thread.load(Ordering::SeqCst) {
                        std::thread::sleep(Duration::from_millis(100));
                        for link_id in coordinator_fuer_thread.liveness_tick() {
                            closer.link_schliessen(&link_id);
                        }
                    }
                })
                .map_err(|e| format!("Coordinator-Tick: {e}"))?;
            Ok(BrokerLauf {
                _supervisor: Mutex::new(Some(BrokerSupervisor {
                    stop: supervisor_stop,
                    join: Some(supervisor_join),
                })),
                _griff_v2: Mutex::new(Some(griff_v2)),
                _griff_v3: Mutex::new(Some(griff_v3)),
                store: Mutex::new(Some(store)),
                _coordinator: coordinator,
                register,
                session_token,
                gestartet_ms: jetzt_ms(),
                bindungen_pfad,
                idle_seit: Mutex::new(Some(Instant::now())),
                beendet: AtomicBool::new(false),
            })
        }
        #[cfg(not(windows))]
        {
            let _ = bindungen_pfad;
            Err("EQ-Copilot-Broker ist V1 nur für Windows gebaut".to_string())
        }
    });
    ergebnis
        .as_ref()
        .map_err(|e| e.clone())
        .and_then(|lauf| {
            if lauf.beendet.load(Ordering::SeqCst) {
                Err("Broker wurde in diesem Prozess bereits geordnet beendet".to_string())
            } else {
                Ok(())
            }
        })
}

/// Geordneter Gegenpfad fuer den normalen Prozessausstieg. Statische
/// `OnceLock`-Werte werden beim Rueckweg aus `main` nicht gedroppt; ohne diese
/// Funktion bliebe die Reihenfolge allein der OS-Prozessbereinigung
/// ueberlassen. Der v3-Griff faellt bewusst nach Supervisor und v2 und joint
/// intern erst Acceptor/Wachhund/Worker, bevor er den letzten Besitzlistener
/// schliesst (NAK-123 A-06/A-09).
pub fn broker_geordnet_stoppen() {
    let Some(Ok(lauf)) = BROKER.get() else {
        return;
    };
    if lauf.beendet.swap(true, Ordering::SeqCst) {
        return;
    }

    #[cfg(windows)]
    {
        let supervisor = lauf
            ._supervisor
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .take();
        drop(supervisor);
        let griff_v2 = lauf
            ._griff_v2
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .take();
        drop(griff_v2);
        let griff_v3 = lauf
            ._griff_v3
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .take();
        drop(griff_v3);
        // H-09: erst Supervisor, dann v2-Handle, dann v3-Handle, DANN Store -
        // damit kein Weg mehr Auftraege einreicht, wenn der Store zumacht. Der
        // Destruktor des entnommenen StoreWriter fuehrt seinen Stopp selbst
        // aus: Shutdown senden, Schreiberthread joinen.
        let store = lauf
            .store
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .take();
        drop(store);
    }
}

pub fn broker_idle_ende_erreicht(idle: Duration, aktive_clients: usize) -> bool {
    aktive_clients == 0 && idle >= Duration::from_millis(BROKER_IDLE_ENDE_MS)
}

pub fn broker_idle_aktualisieren(
    idle_seit: &mut Option<Instant>,
    jetzt: Instant,
    aktive_clients: usize,
) -> bool {
    if aktive_clients != 0 {
        *idle_seit = None;
        return false;
    }
    let seit = idle_seit.get_or_insert(jetzt);
    broker_idle_ende_erreicht(jetzt.saturating_duration_since(*seit), aktive_clients)
}

/// Monotone Selbstende-Bedingung des Brokerprozesses. Das Plugin beendet nie
/// einen Prozess und besitzt deshalb keinen Stop-Pfad zu dieser Funktion.
pub fn broker_soll_idle_enden() -> bool {
    let Some(Ok(lauf)) = BROKER.get() else {
        return false;
    };
    if lauf.beendet.load(Ordering::SeqCst) {
        return false;
    }
    #[cfg(windows)]
    let aktive_v3 = lauf
        ._griff_v3
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .as_ref()
        .map(transport::server_v3::V3Griff::aktive_worker)
        .unwrap_or(0) as usize;
    #[cfg(not(windows))]
    let aktive_v3 = 0usize;
    let aktive_v2 = lauf
        .register
        .lock()
        .expect("Register-Mutex")
        .sensoren
        .values()
        .filter(|sensor| sensor.verbunden)
        .count();
    let aktive = aktive_v2.saturating_add(aktive_v3);
    let mut idle = lauf.idle_seit.lock().unwrap_or_else(|e| e.into_inner());
    broker_idle_aktualisieren(&mut idle, Instant::now(), aktive)
}

/// Sichtbarer Gegenpfad fuer einen fatal beendeten v3-Acceptor. Der Name ist
/// zu diesem Zeitpunkt noch im `V3Griff` besessen; erst der anschliessende
/// geordnete Stopp schliesst die Restlistener nach allen Joins.
pub fn broker_hat_fatalen_v3_listenerfehler() -> bool {
    let Some(Ok(lauf)) = BROKER.get() else {
        return false;
    };
    if lauf.beendet.load(Ordering::SeqCst) {
        return false;
    }
    #[cfg(windows)]
    {
        return lauf
            ._griff_v3
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .as_ref()
            .map(transport::server_v3::V3Griff::fataler_listenerfehler)
            .unwrap_or(false);
    }
    #[cfg(not(windows))]
    false
}

pub fn broker_store_sicht() -> Option<store::StoreSicht> {
    let lauf = BROKER.get()?.as_ref().ok()?;
    if lauf.beendet.load(Ordering::SeqCst) {
        return None;
    }
    #[cfg(windows)]
    {
        // H-09: nach dem geordneten Stopp ist der Store entnommen. Die Sicht
        // meldet das ehrlich mit None, statt auf einem leeren Option zu
        // panisieren.
        return lauf
            .store
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .as_ref()
            .map(|writer| writer.handle().sicht());
    }
    #[cfg(not(windows))]
    {
        let _ = lauf;
        None
    }
}

pub fn broker_status() -> BrokerStatus {
    match BROKER.get() {
        Some(Ok(lauf)) if !lauf.beendet.load(Ordering::SeqCst) => {
            let register = lauf.register.lock().expect("Register-Mutex");
            let sensoren = register.sensoren_snapshot(jetzt_ms());
            let sessions = sessions_bilden(&sensoren);
            let paare = paare_auswerten(&sensoren);
            BrokerStatus {
                laeuft: true,
                broker_version: broker_version(),
                protokoll_version: protokoll::PROTOKOLL_VERSION,
                pipe_name: PIPE_NAME.to_string(),
                session_token_kurz: lauf.session_token.chars().take(8).collect(),
                gestartet_ms: lauf.gestartet_ms,
                pakete_verworfen: register.pakete_verworfen,
                fehler: register.fehler.clone(),
                sensoren,
                sessions,
                paare,
            }
        }
        Some(Ok(_)) => status_ohne_lauf(vec!["Broker wurde geordnet beendet".to_string()]),
        Some(Err(e)) => status_ohne_lauf(vec![e.clone()]),
        None => status_ohne_lauf(vec!["Broker wurde noch nicht gestartet".to_string()]),
    }
}

fn status_ohne_lauf(fehler: Vec<String>) -> BrokerStatus {
    BrokerStatus {
        laeuft: false,
        broker_version: broker_version(),
        protokoll_version: protokoll::PROTOKOLL_VERSION,
        pipe_name: PIPE_NAME.to_string(),
        session_token_kurz: String::new(),
        gestartet_ms: 0,
        pakete_verworfen: 0,
        fehler,
        sensoren: Vec::new(),
        sessions: Vec::new(),
        paare: Vec::new(),
    }
}

/// Prozessgrenzen-Probe: fährt einen ECHTEN Server (eigene ACL, eigenes
/// Register) auf einem FREI WÄHLBAREN Pipenamen — ohne den globalen OnceLock.
/// So kollidiert die Ende-zu-Ende-Probe nie mit dem Broker einer laufenden (bis 18.08.: Hub-App-gestarteten)
/// Hub-App (M2-Fund: zwei Broker auf einem Namen stehlen sich still Clients;
/// die erste Instanz trägt deshalb jetzt FILE_FLAG_FIRST_PIPE_INSTANCE).
/// Liefert am Ende den Status als JSON.
#[cfg(windows)]
pub fn probe_lauf(pipe_name: &str, sekunden: u64) -> Result<String, String> {
    let register = Arc::new(Mutex::new(Register::default()));
    let session_token = uuid::Uuid::new_v4().to_string();
    let griff = server::server_starten(
        pipe_name,
        register.clone(),
        broker_version(),
        session_token.clone(),
    )?;
    std::thread::sleep(std::time::Duration::from_secs(sekunden));
    let status = {
        let r = register
            .lock()
            .map_err(|_| "Register-Mutex vergiftet".to_string())?;
        let sensoren = r.sensoren_snapshot(jetzt_ms());
        let sessions = sessions_bilden(&sensoren);
        let paare = paare_auswerten(&sensoren);
        BrokerStatus {
            laeuft: true,
            broker_version: broker_version(),
            protokoll_version: protokoll::PROTOKOLL_VERSION,
            pipe_name: pipe_name.to_string(),
            session_token_kurz: session_token.chars().take(8).collect(),
            gestartet_ms: 0,
            pakete_verworfen: r.pakete_verworfen,
            fehler: r.fehler.clone(),
            sensoren,
            sessions,
            paare,
        }
    };
    drop(griff);
    serde_json::to_string_pretty(&status).map_err(|e| format!("Status serialisieren: {e}"))
}

/// Profilbindung setzen/lösen + persistieren. Fehler beim Schreiben werden
/// zurückgegeben UND im Register sichtbar gemerkt — nie still (Plan §8.3).
pub fn profil_binden(sensor_id: &str, profil_id: Option<String>) -> Result<(), String> {
    let Some(Ok(lauf)) = BROKER.get() else {
        return Err("Broker läuft nicht".to_string());
    };
    if lauf.beendet.load(Ordering::SeqCst) {
        return Err("Broker läuft nicht".to_string());
    }
    let mut register = lauf.register.lock().expect("Register-Mutex");
    register.profil_binden(sensor_id, profil_id);
    if let Some(pfad) = &lauf.bindungen_pfad {
        if let Err(e) = bindung::bindungen_schreiben(pfad, &register.bindungen) {
            register.fehler_merken(format!("Profilbindungen schreiben: {e}"));
            return Err(e);
        }
    }
    Ok(())
}

/// Zeitausgerichteter Aggregat-Snapshot aller (gefilterten) Sensoren →
/// eine Datei unter %LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\.
/// Liefert den Pfad der geschriebenen Datei.
pub fn aggregat_schreiben(
    filter_profil: Option<String>,
    filter_pid: Option<u32>,
) -> Result<String, String> {
    let Some(Ok(lauf)) = BROKER.get() else {
        return Err("Broker läuft nicht".to_string());
    };
    if lauf.beendet.load(Ordering::SeqCst) {
        return Err("Broker läuft nicht".to_string());
    }
    let (sensoren, token_kurz) = {
        let register = lauf.register.lock().expect("Register-Mutex");
        (
            register.sensoren_snapshot(jetzt_ms()),
            lauf.session_token.chars().take(8).collect::<String>(),
        )
    };
    if let Some(grund) = aggregat_sperrgrund(&sensoren) {
        return Err(format!("Aggregat gesperrt: {grund}"));
    }
    let dokument = aggregat::aggregat_bauen(
        &sensoren,
        filter_profil.as_deref(),
        filter_pid,
        &broker_version(),
        &token_kurz,
        jetzt_ms(),
    );
    let pfad = aggregat::aggregat_schreiben(&dokument)?;
    Ok(pfad.display().to_string())
}
