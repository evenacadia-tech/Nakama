//! SONDE-010 — v3-Listener und I/O-Worker.
//!
//! Der Worker entscheidet GENAU drei Dinge (Entwurf §53.9): Envelope,
//! Grenzen und Authentisierung. Alles Weitere geht als typisiertes Ereignis
//! an eine schmale `Senke`. Session, Eviction, Store und Outbox liegen im
//! `Coordinator`; der Transport bleibt auch im produktiven SONDE-011-Pfad
//! auf diese schmale Schnittstelle begrenzt.
//!
//! ── Drei Threads je Verbindung, und warum ──────────────────────────────────
//!
//! §53.9 verlangt woertlich: "Antworten gehen ueber getrennte bounded
//! Writerqueues zurueck; ein blockierender Pipe-Write haelt weder Coordinator
//! noch Storelock." Die erste Fassung dieses Listeners leerte den Ingress nach
//! JEDEM Frame vollstaendig und schrieb die Antwort im Leserthread. Damit
//! konnte die Ingressqueue nie ueber Groesse 1 wachsen, und eine langsame
//! Senke oder ein Peer, der Antworten nicht abholt, hielt den Leser am ersten
//! Frame fest — die Cap-256-, P2-Drop- und P0-Ueberlaufpfade waren im echten
//! Listener unerreichbar (T2-Befund 3 vom 2026-08-29).
//!
//! Deshalb hat jede Verbindung jetzt drei Threads mit genau einer Aufgabe:
//!
//! ```text
//!   Leser      Bytes -> Envelope -> Ingress (bounded 256)   nie Senke, nie write
//!   Verbraucher Ingress -> Senke -> Antwort in die Writerqueue (bounded 256)
//!   Schreiber  Writerqueue -> ein einziger write_all auf der Pipe
//! ```
//!
//! Der Leser blockiert dadurch nie hinter der Senke, und ein blockierender
//! Write blockiert nur den Schreiber. Damit Lesen und Schreiben auf DEMSELBEN
//! Pipe-Handle wirklich nebenlaeufig sind, laufen die Instanzen als
//! `FILE_FLAG_OVERLAPPED`: bei einem synchronen Handle serialisiert der
//! I/O-Manager alle Operationen, ein haengender Read wuerde also einen Write
//! blockieren — genau das, was die Trennung verhindern soll.
//!
//! ── Produktions- und Probe-Namen ───────────────────────────────────────────
//!
//! Der Broker oeffnet in Produktion den SID-gebundenen v3-Endpunkt neben der
//! v2-Legacy-Pipe und verdrahtet ihn mit Coordinator, Store und Outbox. Tests
//! rufen denselben Listener ausschliesslich mit einem Probe-Namen auf — nie
//! mit der Produktions-Pipe (CLAUDE.md, "Bauen und beweisen").
//!
//! Sicherheit ist trotzdem nicht Probe-Qualitaet: derselbe SDDL-Deskriptor
//! ("nur der aktuelle User"), `PIPE_REJECT_REMOTE_CLIENTS` und
//! `FILE_FLAG_FIRST_PIPE_INSTANCE` wie beim v2-Server — die Helfer kommen aus
//! `server.rs`, damit es nur EINE Wahrheit ueber die Pipe-Sicherheit gibt.

use std::collections::{HashMap, VecDeque};
use std::sync::atomic::{AtomicBool, AtomicU64, AtomicUsize, Ordering};
use std::sync::{Arc, Condvar, Mutex};
use std::thread::JoinHandle;
use std::time::{Duration, Instant};

use windows_sys::Win32::Foundation::{
    CloseHandle, GetLastError, ERROR_BROKEN_PIPE, ERROR_INSUFFICIENT_BUFFER, ERROR_IO_PENDING,
    ERROR_NO_DATA, ERROR_NO_TOKEN, ERROR_OPERATION_ABORTED, ERROR_PIPE_BUSY, ERROR_PIPE_CONNECTED,
    ERROR_PIPE_NOT_CONNECTED, HANDLE, INVALID_HANDLE_VALUE, WAIT_OBJECT_0,
};
use windows_sys::Win32::Security::{
    CopySid, EqualSid, GetLengthSid, GetTokenInformation, IsValidSid, RevertToSelf, TokenUser,
    SECURITY_ATTRIBUTES, TOKEN_QUERY, TOKEN_USER,
};
use windows_sys::Win32::Storage::FileSystem::{
    ReadFile, WriteFile, FILE_FLAG_FIRST_PIPE_INSTANCE, FILE_FLAG_OVERLAPPED, PIPE_ACCESS_DUPLEX,
};
use windows_sys::Win32::System::Pipes::{
    ConnectNamedPipe, CreateNamedPipeW, DisconnectNamedPipe, ImpersonateNamedPipeClient,
    PIPE_READMODE_BYTE, PIPE_REJECT_REMOTE_CLIENTS, PIPE_TYPE_BYTE, PIPE_WAIT,
};
use windows_sys::Win32::System::Threading::{
    CreateEventW, GetCurrentThread, OpenThreadToken, ResetEvent, SetEvent, WaitForMultipleObjects,
    WaitForSingleObject, INFINITE,
};
use windows_sys::Win32::System::IO::{CancelIoEx, GetOverlappedResult, OVERLAPPED};

use crate::transport::bootstrap::{
    bootstrap_lesen, neue_kennung, Bootstrap, HelloControl, Kopplungen, Welcome,
};
use crate::transport::v3::{
    envelope_schreiben, Familie, LeseErgebnis, Ratengrenze, StromLeser, MAX_FRAME_BYTES,
};
#[cfg(test)]
use crate::transport::warteschlange::CAP_INGRESS;
use crate::transport::warteschlange::{IngressErgebnis, IngressWarteschlange};

/// Hoechstens so viele gleichzeitige Verbindungen. Zwei je Instanz (Control +
/// Telemetry) mal 32 Sonden plus Reserve.
mod auth;
mod queues;
mod senke;
mod win_handles;

use auth::{client_sid_authentisieren, VerbindungsSicherheitsCleanup};
use queues::{Ausgang, Eingang};
use win_handles::{
    alle_io_abbrechen, io_abbrechen, ov_lesen, ov_schreiben, EndeSignal, Ereignis, HandleEintrag,
    HandleRegister, IoAusgang, ListenerInstanz, SicherheitsSpur, TokenGriff, Verbindungsgriff,
};

pub use senke::{ControlAnmeldung, Senke, ZaehlSenke};

pub const MAX_VERBINDUNGEN: usize = 96;

/// 96 Worker plus zwei jederzeit bewaffnete Besitzlistener. Windows erlaubt
/// hier 1..=255; alle Instanzen desselben Namens muessen denselben Wert nennen.
pub const PIPE_INSTANZEN: usize = MAX_VERBINDUNGEN + 2;

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

    fn vor_worker_uebergabe_warten(&self) {
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
}

/// Frist fuer das Bootstrap-Hello. Ohne sie haelt ein lokaler Slowloris einen
/// Verbindungsslot beliebig lange (Fehlerlexikon, wissen/engineering
/// 2026-08-27).
pub const BOOTSTRAP_FRIST: Duration = Duration::from_millis(5000);

/// Nachrichtenratengrenze je Verbindung: 4000 Frames pro Sekunde. 32 Sonden
/// bei 10 Hz sind 320 — die Grenze faengt eine Flut, nicht den Betrieb.
pub const RATE_PRO_SEKUNDE: u32 = 4000;

/// Getrennte, begrenzte Writerqueue je Verbindung (§53.9). Laeuft sie ueber,
/// holt der Peer seine Antworten nicht ab — dann faellt die Verbindung, nicht
/// die Antwort still unter den Tisch.
pub const CAP_WRITER: usize = 256;

/// Aktive additive Vertragsfassungen. Die Version lebt ausschliesslich im
/// Wire-Envelope: Descriptor-Hostfelder und LUFS-I-Framefelder wurden in
/// SONDE-012 B1 mit Minor 1 belegt.
const P0_SCHEMA_MINOR: u8 = 1;
const P1_SCHEMA_MINOR: u8 = 1;
const P2_SCHEMA_MINOR: u8 = 1;

fn schema_minor_bekannt(familie: Familie, schema_minor: u8) -> bool {
    let hoechster = match familie {
        Familie::P0 => P0_SCHEMA_MINOR,
        Familie::P1 => P1_SCHEMA_MINOR,
        Familie::P2 => P2_SCHEMA_MINOR,
    };
    schema_minor <= hoechster
}

/// Was der I/O-Worker nach oben gibt. Bewusst byteorientiert: die Bedeutung
/// des Payloads kennt erst der Coordinator.
/// Frist, die der Verbindungsschluss einem LAUFENDEN Senkenaufruf noch
/// laesst. Danach wird der Verbraucherthread abgeloest statt gejoint.
pub const SENKE_FRIST: Duration = Duration::from_millis(2000);

//==============================================================================
// Overlapped-I/O: die kleinste Menge Win32, die drei Threads auf einem Handle
// erlaubt.

//==============================================================================
// Die zwei bounded Queues zwischen den drei Threads.

//==============================================================================

pub struct V3Griff {
    stop: Arc<AtomicBool>,
    pipe_name: String,
    acceptor_stop: Arc<EndeSignal>,
    acceptor: Option<JoinHandle<()>>,
    rest_listener: Arc<Mutex<Vec<ListenerInstanz>>>,
    verbindungen: Arc<Mutex<Vec<JoinHandle<()>>>>,
    handles: Arc<Mutex<HandleRegister>>,
    /// Deadlines der noch nicht abgeschlossenen Bootstraps.
    bootstraps: Arc<Mutex<Vec<(u64, Instant)>>>,
    closer: V3Closer,
    wachhund: Option<JoinHandle<()>>,
    sender: V3Sender,
    sicherheits_spur: Arc<SicherheitsSpur>,
    uebergabe_barriere: Option<Arc<V3UebergabeBarriere>>,
    /// Die EINZIGE Wahrheit ueber die Zahl lebender Worker: derselbe Zaehler,
    /// den `worker_reservieren` per `compare_exchange` besetzt und
    /// `WorkerPlatz::drop` per `fetch_sub` freigibt. Eine zweite, aus
    /// gelesenen Vorwerten fortgeschriebene Kopie gab es bis NAK-123 R1 in
    /// `V3Statistik`; sie konnte bei verschraenkten Freigaben dauerhaft
    /// auseinanderlaufen und `broker_soll_idle_enden` belogen.
    worker_zaehler: Arc<AtomicUsize>,
    pub statistik: Arc<V3Statistik>,
}

#[derive(Clone)]
pub struct V3Closer {
    kopplungen: Arc<Mutex<Kopplungen>>,
    handles: Arc<Mutex<HandleRegister>>,
}

impl V3Closer {
    pub fn link_schliessen(&self, link_id: &str) {
        kopplung_loesen(&self.kopplungen, &self.handles, link_id, true);
    }
}

#[derive(Clone)]
pub struct V3Sender {
    control_ausgaenge: Arc<Mutex<HashMap<String, Arc<Ausgang>>>>,
    telemetrie_ausgaenge: Arc<Mutex<HashMap<String, Arc<Ausgang>>>>,
}

impl V3Sender {
    pub fn neu() -> Self {
        Self {
            control_ausgaenge: Arc::new(Mutex::new(HashMap::new())),
            telemetrie_ausgaenge: Arc::new(Mutex::new(HashMap::new())),
        }
    }

    pub fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
        let frame = match envelope_schreiben(Familie::P1, P1_SCHEMA_MINOR, payload) {
            Ok(frame) => frame,
            Err(_) => return false,
        };
        let ausgang = self
            .control_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .get(link_id)
            .cloned();
        let Some(ausgang) = ausgang else {
            return false;
        };
        let Some(antwort) = ausgang.snapshot_einreihen_mit_antwort("session_snapshot", frame)
        else {
            return false;
        };
        antwort.recv_timeout(SENKE_FRIST).unwrap_or(false)
    }

    pub fn messframe_schreiben(&self, link_id: &str, instance_id: &str, payload: &[u8]) -> bool {
        let frame = match envelope_schreiben(Familie::P2, P2_SCHEMA_MINOR, payload) {
            Ok(frame) => frame,
            Err(_) => return false,
        };
        let ausgang = self
            .telemetrie_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .get(link_id)
            .cloned();
        ausgang.is_some_and(|ausgang| ausgang.messframe_einreihen(instance_id, frame))
    }
}

impl crate::coordinator::SessionPush for V3Sender {
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
        V3Sender::snapshot_schreiben(self, link_id, payload)
    }

    fn messframe_schreiben(&self, link_id: &str, instance_id: &str, payload: &[u8]) -> bool {
        V3Sender::messframe_schreiben(self, link_id, instance_id, payload)
    }
}

/// Zahlen des Listeners selbst (nicht der Senke).
#[derive(Debug, Default)]
pub struct V3Statistik {
    pub angenommen: AtomicU64,
    pub geschlossen_bootstrap: AtomicU64,
    pub geschlossen_envelope: AtomicU64,
    pub geschlossen_rate: AtomicU64,
    pub geschlossen_p0_ueberlauf: AtomicU64,
    /// Familie auf der falschen Verbindungsart (Control traegt P0/P1,
    /// Telemetry traegt P2 — `eq-ipc-v3.schema.json`).
    pub geschlossen_familie: AtomicU64,
    /// Telemetrieframe, dessen Control-Verbindung nicht mehr lebt.
    pub geschlossen_kopplung: AtomicU64,
    /// Der Peer holt seine Antworten nicht ab; die Writerqueue lief ueber.
    pub geschlossen_writer: AtomicU64,
    /// Wie oft der Acceptor an der Verbindungsgrenze auf einen freien Platz
    /// warten musste. Ist die Zahl 0, WAR die Grenze nie erreicht — ein Test
    /// darueber spraeche dann ueber nichts.
    pub acceptor_wartet_auf_instanz: AtomicU64,
    /// Zwei nach erfolgreichem Start, null erst beim finalen Listener-Drop.
    pub bewaffnete_listener: AtomicU64,
    pub am_worker_cap_abgewiesen: AtomicU64,
    pub listener_fehler: AtomicU64,
    pub worker_uebergaben: AtomicU64,
    pub ingress_p2_verworfen: AtomicU64,
    /// Wie oft ein P1-Ueberlauf ohne P2 zum Verwerfen die Verbindung getrennt
    /// hat (Matrix `A-IN-04`). P1 faellt nie still: §53.9 gibt ihm den
    /// Wiederholweg ueber Reconnect/Outbox, und den nimmt ihm ein stiller
    /// Verlust bei offener Verbindung.
    pub ingress_p1_ueberlauf_trennt: AtomicU64,
    /// Hoechststand der Ingressqueue ueber alle Verbindungen. Er ist der
    /// Beleg, dass der Leser nicht am ersten Frame haengt.
    pub ingress_hoechststand: AtomicU64,
    /// Wie oft ein Verbraucherthread beim Verbindungsschluss ABGELOEST wurde,
    /// weil sein Senkenaufruf laenger als `SENKE_FRIST` stand. Die Zahl ist
    /// der sichtbare Preis dafuer, dass `stoppen()` trotzdem endet.
    pub senke_abgeloest: AtomicU64,
    /// Dasselbe fuer den Schreiberthread, dessen `ov_schreiben` trotz
    /// wiederholtem `CancelIoEx` nicht zurueckkam.
    pub schreiber_abgeloest: AtomicU64,
    /// Wie oft ein LEBENSZYKLUS-Aufruf der Senke (`control_verbunden`,
    /// `telemetrie_gekoppelt`, `*_getrennt`, `abgewiesen`) laenger als
    /// `SENKE_FRIST` stand und deshalb abgeloest wurde. Diese Aufrufe liefen
    /// frueher unbegrenzt auf dem Verbindungsthread — und genau auf den
    /// wartet `stoppen()` (T2-Befund 5 Runde 3 vom 2026-08-29).
    pub lebenszyklus_abgeloest: AtomicU64,
    /// Wie viele weitere Lebenszyklusaufrufe DESHALB unterblieben. Nach einem
    /// abgeloesten Aufruf schweigt die Verbindung gegenueber ihrer Senke,
    /// statt neben dem haengenden Aufruf in falscher Reihenfolge weiterzureden.
    pub lebenszyklus_uebersprungen: AtomicU64,
    /// Wie oft die Control-Seite ihr `control_getrennt` melden musste, OHNE
    /// dass das `telemetrie_getrennt` der Gegenseite binnen `SENKE_FRIST` kam
    /// (Matrix `C-LS-06`). Die Meldung faellt nie aus — sie ist dann nur
    /// nicht mehr geordnet, und genau das steht hier.
    pub lebenszyklus_reihenfolge_verletzt: AtomicU64,
    pub aktive_controls: AtomicU64,
}

impl V3Statistik {
    fn hoechststand_melden(&self, stand: usize) {
        let stand = stand as u64;
        let mut alt = self.ingress_hoechststand.load(Ordering::Relaxed);
        while stand > alt {
            match self.ingress_hoechststand.compare_exchange_weak(
                alt,
                stand,
                Ordering::Relaxed,
                Ordering::Relaxed,
            ) {
                Ok(_) => break,
                Err(a) => alt = a,
            }
        }
    }
}

impl V3Griff {
    pub fn pipe_name(&self) -> &str {
        &self.pipe_name
    }

    /// Wie viele Verbindungs-Threadhandles der Listener gerade haelt. Sie
    /// werden laufend geerntet; die Zahl darf ueber viele Verbindungszyklen
    /// nicht wachsen (T2-Befund 8 vom 2026-08-29).
    pub fn gehaltene_verbindungen(&self) -> usize {
        self.verbindungen
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .len()
    }

    /// Wie viele Verbindungshandles das Abbruchregister gerade fuehrt. Ein
    /// Handle steht dort, seit der Acceptor die Verbindung angenommen hat —
    /// NICHT erst, seit sein Thread laeuft (T2-Befund 6 vom 2026-08-29).
    pub fn gehaltene_handles(&self) -> usize {
        self.handles
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .offen
            .len()
    }

    pub fn sender(&self) -> V3Sender {
        self.sender.clone()
    }

    pub fn closer(&self) -> V3Closer {
        self.closer.clone()
    }

    pub fn aktive_controls(&self) -> u64 {
        self.statistik.aktive_controls.load(Ordering::SeqCst)
    }

    /// Alle bereits angenommenen v3-Verbindungen, einschliesslich eines noch
    /// laufenden Bootstrap-Hellos. Auch dieser Zustand ist nicht idle: Der
    /// Prozess darf einem gerade authentisierenden Client nicht unter dem
    /// Handle wegsterben, nur weil der Coordinator ihn noch nicht kennt.
    pub fn aktive_worker(&self) -> u64 {
        self.worker_zaehler.load(Ordering::SeqCst) as u64
    }

    /// Nur fuer Tests: belegt einen Workerplatz auf DEMSELBEN Zaehler, den der
    /// Acceptor benutzt, ueber genau dieselbe `worker_reservieren`. Der
    /// optionale Haltepunkt laeuft im `drop` nach der Zaehlerfreigabe; damit
    /// verschraenkt ein Test zwei Freigaben ohne Sleep-Raten (NAK-123 R1, P1).
    #[doc(hidden)]
    pub fn worker_platz_probe(
        &self,
        freigabe_haltepunkt: Option<Arc<dyn Fn() + Send + Sync>>,
    ) -> Option<WorkerPlatzProbe> {
        let mut platz = worker_reservieren(&self.worker_zaehler)?;
        platz.freigabe_haltepunkt = freigabe_haltepunkt;
        Some(WorkerPlatzProbe(platz))
    }

    /// Ein Listener-/Ersatzfehler beendet den Acceptor fail-closed. Der
    /// Prozess-Lifecycle liest dieses Signal und faehrt den Griff sofort
    /// geordnet herunter; bis dahin bleiben die Restlistener in Besitz.
    pub fn fataler_listenerfehler(&self) -> bool {
        self.statistik.listener_fehler.load(Ordering::SeqCst) != 0
    }

    #[doc(hidden)]
    pub fn sicherheits_spur(&self) -> Vec<&'static str> {
        self.sicherheits_spur.snapshot()
    }

    /// Gegenpfad zu `v3_server_starten`. Setzt Stop, weckt den parkenden
    /// Acceptor ueber sein Event und bricht die I/O aller lebenden
    /// Verbindungen ab. Die beiden Besitzlistener bleiben waehrend aller
    /// Joins offen und fallen als letzte Pipehandles.
    pub fn stoppen(&mut self) {
        self.stop.store(true, Ordering::SeqCst);
        self.acceptor_stop.setzen();
        if let Some(barriere) = &self.uebergabe_barriere {
            barriere.freigeben();
        }
        alle_io_abbrechen(&self.handles);
        if let Some(j) = self.acceptor.take() {
            let _ = j.join();
        }
        if let Some(j) = self.wachhund.take() {
            let _ = j.join();
        }
        loop {
            let naechster = {
                let mut v = self.verbindungen.lock().unwrap_or_else(|e| e.into_inner());
                v.pop()
            };
            match naechster {
                Some(j) => {
                    // WIEDERHOLT abbrechen, nicht einmal. Ein einzelnes
                    // `CancelIoEx` verpufft, wenn der Thread seinen Read erst
                    // danach absetzt — und `join` waere dann ein Hang.
                    while !j.is_finished() {
                        alle_io_abbrechen(&self.handles);
                        std::thread::sleep(Duration::from_millis(5));
                    }
                    let _ = j.join();
                }
                None => break,
            }
        }
        let _ = self.bootstraps.lock().map(|mut b| b.clear());
        let listener_geschlossen = {
            let mut listener = self.rest_listener.lock().unwrap_or_else(|e| e.into_inner());
            let vorhanden = !listener.is_empty();
            listener.clear();
            vorhanden
        };
        self.statistik
            .bewaffnete_listener
            .store(0, Ordering::SeqCst);
        if listener_geschlossen {
            // A-09: erst nachdem Acceptor, Wachhund und alle normalen
            // Verbindungsworker beendet und deren Handles geschlossen sind,
            // fallen die beiden letzten Besitzlistener.
            self.sicherheits_spur.push("listeners_close");
        }
    }
}

impl Drop for V3Griff {
    fn drop(&mut self) {
        self.stoppen();
    }
}

/// Joint mit Frist. Laeuft sie ab, wird der Thread ABGELOEST statt gejoint:
/// das `JoinHandle` faellt, der Thread haelt nur noch seine `Arc`s und endet
/// von selbst, sobald sein Fremdaufruf zurueckkommt. `false` = abgeloest.
///
/// `zwischendurch` laeuft in jeder Warterunde — der Schreiber braucht dort
/// sein wiederholtes `CancelIoEx`, der Verbraucher nichts.
fn join_mit_frist(j: JoinHandle<()>, frist: Duration, mut zwischendurch: impl FnMut()) -> bool {
    let bis = Instant::now() + frist;
    while !j.is_finished() {
        if Instant::now() >= bis {
            return false;
        }
        zwischendurch();
        std::thread::sleep(Duration::from_millis(2));
    }
    let _ = j.join();
    true
}

/// Fuehrt die Lebenszyklus- und Diagnoseaufrufe EINER Verbindung mit Frist
/// aus.
///
/// `p0`/`p1`/`p2` laufen auf dem Ingressthread und sind ueber dessen
/// fristbegrenzten Join abgesichert. `control_verbunden`,
/// `telemetrie_gekoppelt`, `control_getrennt`, `telemetrie_getrennt` und
/// `abgewiesen` liefen dagegen unbegrenzt direkt auf dem VERBINDUNGSthread —
/// und `stoppen()` wartet auf genau diesen Thread, bis er fertig ist. Eine
/// Senke, die dort haengt, hielt den Stop unbegrenzt auf; der Haengertest
/// deckte nur `p0/p1/p2` (T2-Befund 5 Runde 3 vom 2026-08-29).
///
/// Jeder Aufruf laeuft deshalb auf einem eigenen, kurzlebigen Thread und wird
/// mit `SENKE_FRIST` gejoint. Laeuft sie ab, wird er ABGELOEST — und die
/// Verbindung sagt ihrer Senke danach nichts mehr: ein zweiter Aufruf liefe
/// neben dem haengenden ersten und erreichte die Senke in falscher
/// Reihenfolge. Ein Thread je Lebenszyklusereignis ist bezahlbar: es sind
/// wenige je Verbindung, und die Verbindung selbst kostet ohnehin drei.
struct Senkenruf {
    senke: Arc<dyn Senke>,
    statistik: Arc<V3Statistik>,
    stumm: bool,
}

impl Senkenruf {
    fn neu(senke: Arc<dyn Senke>, statistik: Arc<V3Statistik>) -> Self {
        Self {
            senke,
            statistik,
            stumm: false,
        }
    }

    /// `false` = der Aufruf kam nicht binnen Frist zurueck, oder er unterblieb,
    /// weil ein frueherer noch haengt.
    fn rufen(&mut self, f: impl FnOnce(&dyn Senke) + Send + 'static) -> bool {
        self.rufen_mit_ergebnis(move |senke| {
            f(senke);
        })
        .is_some()
    }

    fn rufen_mit_ergebnis<T: Send + 'static>(
        &mut self,
        f: impl FnOnce(&dyn Senke) -> T + Send + 'static,
    ) -> Option<T> {
        if self.stumm {
            self.statistik
                .lebenszyklus_uebersprungen
                .fetch_add(1, Ordering::SeqCst);
            return None;
        }
        let s = self.senke.clone();
        let (antwort_tx, antwort_rx) = std::sync::mpsc::sync_channel(1);
        let j = match std::thread::Builder::new()
            .name("eqcop-v3-senkenruf".into())
            .spawn(move || {
                let _ = antwort_tx.send(f(s.as_ref()));
            }) {
            Ok(j) => j,
            Err(_) => {
                self.stumm = true;
                return None;
            }
        };
        if join_mit_frist(j, SENKE_FRIST, || {}) {
            return antwort_rx.try_recv().ok();
        }
        self.stumm = true;
        self.statistik
            .lebenszyklus_abgeloest
            .fetch_add(1, Ordering::SeqCst);
        None
    }

    /// Kurzform fuer den haeufigsten Fall.
    fn abweisen(&mut self, grund: impl Into<String>) {
        let g = grund.into();
        self.rufen(move |s| s.abgewiesen(&g));
    }
}

/// Beendete Verbindungsthreads joinen und aus der Liste nehmen. Ohne das
/// waechst der Vektor — und mit ihm die nativen Threadhandles — bei jedem
/// Verbinden/Trennen unbegrenzt (T2-Befund 8 vom 2026-08-29).
fn fertige_ernten(verbindungen: &Arc<Mutex<Vec<JoinHandle<()>>>>) {
    let fertig: Vec<JoinHandle<()>> = {
        let mut v = verbindungen.lock().unwrap_or_else(|e| e.into_inner());
        let mut raus = Vec::new();
        let mut i = 0;
        while i < v.len() {
            if v[i].is_finished() {
                raus.push(v.remove(i));
            } else {
                i += 1;
            }
        }
        raus
    };
    for j in fertig {
        let _ = j.join();
    }
}

/// Legt die naechste Pipe-Instanz an — und gibt NICHT auf, wenn gerade alle
/// belegt sind.
///
/// Die alte Fassung brach die Acceptorschleife bei jedem Fehlschlag ab. An der
/// Verbindungsgrenze (`nMaxInstances`) heisst das: ein Peer oeffnet
/// `MAX_VERBINDUNGEN` Verbindungen, laesst sie wieder los — und danach horcht
/// niemand mehr, obwohl alle Plaetze frei sind (T2-Befund 6 vom 2026-08-29).
/// `ERROR_PIPE_BUSY` ist deshalb kein Ende, sondern ein Warten.
fn naechste_instanz(
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
        fertige_ernten(verbindungen);
        std::thread::sleep(Duration::from_millis(25));
    }
}

struct WorkerPlatz {
    zaehler: Arc<AtomicUsize>,
    /// Ausschliesslich `worker_platz_probe` setzt das: laeuft im `drop`
    /// unmittelbar NACH der Zaehlerfreigabe und macht das Interleaving zweier
    /// Freigaben bzw. Freigabe gegen Reserve deterministisch messbar. Im
    /// Acceptor-Pfad ist der Wert immer `None`.
    freigabe_haltepunkt: Option<Arc<dyn Fn() + Send + Sync>>,
}

/// Ein von `V3Griff::worker_platz_probe` belegter Workerplatz. Er haelt den
/// ECHTEN `WorkerPlatz` und gibt ihn beim Fallenlassen ueber genau denselben
/// `Drop` frei, den auch ein Verbindungsthread benutzt.
#[doc(hidden)]
pub struct WorkerPlatzProbe(#[allow(dead_code)] WorkerPlatz);

struct TestHilfsthread {
    zaehler: Option<Arc<AtomicUsize>>,
}

impl TestHilfsthread {
    fn neu(zaehler: Option<Arc<AtomicUsize>>) -> Self {
        if let Some(z) = &zaehler {
            z.fetch_add(1, Ordering::SeqCst);
        }
        Self { zaehler }
    }
}

impl Drop for TestHilfsthread {
    fn drop(&mut self) {
        if let Some(z) = &self.zaehler {
            z.fetch_sub(1, Ordering::SeqCst);
        }
    }
}

impl Drop for WorkerPlatz {
    fn drop(&mut self) {
        // Die Freigabe IST dieses `fetch_sub` — danach folgt kein zweiter
        // Schreibschritt mehr. Ein `store(vorher-1)` aus dem gelesenen Vorwert
        // war bis NAK-123 R1 genau die Luecke, in der zwei verschraenkte
        // Freigaben den sichtbaren Stand dauerhaft falsch stehen liessen (P1).
        let vorher = self.zaehler.fetch_sub(1, Ordering::SeqCst);
        debug_assert!(vorher > 0);
        if let Some(haltepunkt) = &self.freigabe_haltepunkt {
            haltepunkt();
        }
    }
}

fn worker_reservieren(zaehler: &Arc<AtomicUsize>) -> Option<WorkerPlatz> {
    let mut stand = zaehler.load(Ordering::SeqCst);
    loop {
        if stand >= MAX_VERBINDUNGEN {
            return None;
        }
        match zaehler.compare_exchange_weak(stand, stand + 1, Ordering::SeqCst, Ordering::SeqCst) {
            // Der geglueckte Tausch ist die Reservierung. Keine abgeleitete
            // Kopie: `V3Griff::aktive_worker` liest genau diesen Zaehler.
            Ok(_) => {
                return Some(WorkerPlatz {
                    zaehler: zaehler.clone(),
                    freigabe_haltepunkt: None,
                })
            }
            Err(neu) => stand = neu,
        }
    }
}

// Deterministische Testphase fuer das Cancel-vor-Read-Interleaving. Im
// Produktpfad bleibt sie INAKTIV; ein Test armiert genau einen folgenden Read.
const CANCEL_VOR_READ_BEREIT: u64 = 0;
const CANCEL_VOR_READ_GELESEN: u64 = 1;
const CANCEL_VOR_READ_READER: u64 = 2;
const CANCEL_VOR_READ_WRITER: u64 = 3;
const CANCEL_VOR_READ_FORTGESETZT: u64 = 4;
const CANCEL_VOR_READ_INAKTIV: u64 = 5;
const CANCEL_VOR_READ_FEHLER: u64 = 6;

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
fn v3_server_starten_intern(
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

    if security_optionen.start_fehler == V3StartTestFehler::WachhundSpawn {
        return Err("Wachhund-Spawnfehler injiziert".into());
    }

    let stop_w = stop.clone();
    let handles_w = handles.clone();
    let bootstraps_w = bootstraps.clone();
    let verbindungen_w = verbindungen.clone();
    let wachhund_testzaehler = security_optionen.hilfsthread_zaehler.clone();
    let wachhund = std::thread::Builder::new()
        .name("eqcop-v3-wachhund".into())
        .spawn(move || {
            let _lebend = TestHilfsthread::neu(wachhund_testzaehler);
            while !stop_w.load(Ordering::SeqCst) {
                std::thread::sleep(Duration::from_millis(100));
                // Auch ohne neue Verbindung muessen fertige Threads fallen.
                fertige_ernten(&verbindungen_w);
                let jetzt = Instant::now();
                let faellig: Vec<u64> = {
                    let b = bootstraps_w.lock().unwrap_or_else(|e| e.into_inner());
                    b.iter()
                        .filter(|(_, f)| *f <= jetzt)
                        .map(|(id, _)| *id)
                        .collect()
                };
                if faellig.is_empty() {
                    continue;
                }
                if let Ok(reg) = handles_w.lock() {
                    for (id, h) in reg.offen.iter() {
                        if faellig.contains(id) {
                            // SAFETY: siehe alle_io_abbrechen.
                            unsafe {
                                CancelIoEx(*h as HANDLE, std::ptr::null_mut());
                            }
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
                let griff = Arc::new(Verbindungsgriff {
                    h: verbundenes_handle,
                    sicherheits_spur: sicherheits_spur2.clone(),
                });

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
                // Der Eintrag gehoert ab hier dem Thread; scheitert `spawn`,
                // faellt die Closure samt Eintrag und traegt ihn wieder aus.
                let handle_eintrag = HandleEintrag {
                    id,
                    register: handles2.clone(),
                };
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
                            handle_eintrag,
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
                        // Closure-Drop gibt Workerplatz, Handle und Register
                        // frei. Beide Listener sind bereits wieder bewaffnet.
                    }
                }

                fertige_ernten(&verbindungen2);
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

enum BootstrapRahmenStand {
    Unvollstaendig,
    ZuGross,
    Vollstaendig(usize),
}

fn bootstrap_rahmenstand(roh: &[u8]) -> BootstrapRahmenStand {
    if roh.len() < 4 {
        return BootstrapRahmenStand::Unvollstaendig;
    }
    let n = u32::from_le_bytes([roh[0], roh[1], roh[2], roh[3]]) as usize;
    if n > crate::transport::v3::MAX_BOOTSTRAP_BYTES as usize {
        return BootstrapRahmenStand::ZuGross;
    }
    let gesamt = 4usize.saturating_add(n);
    if roh.len() < gesamt {
        BootstrapRahmenStand::Unvollstaendig
    } else {
        BootstrapRahmenStand::Vollstaendig(gesamt)
    }
}

fn bootstrap_reject_schreiben(
    h: HANDLE,
    ereignis: HANDLE,
    grund: &str,
    cleanup: &VerbindungsSicherheitsCleanup,
) {
    let payload = serde_json::json!({
        "type": "reject",
        "code": "protocol_mismatch",
        "reason": grund.chars().take(500).collect::<String>()
    });
    if let Ok(payload) = serde_json::to_vec(&payload) {
        let mut rahmen = (payload.len() as u32).to_le_bytes().to_vec();
        rahmen.extend_from_slice(&payload);
        cleanup.spur("reject");
        let _ = ov_schreiben(h, ereignis, &rahmen);
    }
}

struct BootstrapFrist {
    id: u64,
    liste: Arc<Mutex<Vec<(u64, Instant)>>>,
}

impl BootstrapFrist {
    fn erfuellt(self) {}
}

impl Drop for BootstrapFrist {
    fn drop(&mut self) {
        if let Ok(mut l) = self.liste.lock() {
            l.retain(|(i, _)| *i != self.id);
        }
    }
}

#[allow(clippy::too_many_arguments)]
fn verbindung_bedienen(
    id: u64,
    griff: Arc<Verbindungsgriff>,
    senke: Arc<dyn Senke>,
    kopplungen: Arc<Mutex<Kopplungen>>,
    trennmelder: TrennRegister,
    handles: Arc<Mutex<HandleRegister>>,
    bootstraps: Arc<Mutex<Vec<(u64, Instant)>>>,
    statistik: Arc<V3Statistik>,
    broker_version: String,
    broker_epoch: String,
    stop: Arc<AtomicBool>,
    writer_fehler_erzwungen: Arc<AtomicBool>,
    cancel_vor_read_phase: Arc<AtomicU64>,
    control_ausgaenge: Arc<Mutex<HashMap<String, Arc<Ausgang>>>>,
    telemetrie_ausgaenge: Arc<Mutex<HashMap<String, Arc<Ausgang>>>>,
    erwartete_sicherheit: Arc<crate::server::Sicherheit>,
    auth_fehler: V3AuthTestFehler,
    sicherheits_spur: Arc<SicherheitsSpur>,
    handle_eintrag: HandleEintrag,
) {
    // Der Acceptor hat das Handle schon registriert; der Eintrag traegt es
    // beim Verlassen dieser Funktion wieder aus.
    let _handle_eintrag = handle_eintrag;
    let mut sicherheits_cleanup = VerbindungsSicherheitsCleanup {
        revertiert: false,
        auth_fehler,
        spur: sicherheits_spur,
    };

    // Und JETZT `stop` pruefen. Kam der Stop im Fenster zwischen Annahme und
    // diesem Punkt, endet der Thread hier — statt sich in einen Bootstrap-Read
    // zu legen, den kein Wachhund mehr aufloest (T2-Befund 6 vom 2026-08-29).
    if stop.load(Ordering::SeqCst) {
        return;
    }

    let leseereignis = match Ereignis::neu() {
        Some(e) => e,
        None => return,
    };
    let ende = match EndeSignal::neu() {
        Some(e) => Arc::new(e),
        None => return,
    };

    // Bootstrap-Frist: der Wachhund bricht die I/O ab, wenn das Hello nicht
    // rechtzeitig ganz da ist.
    if let Ok(mut b) = bootstraps.lock() {
        b.push((id, Instant::now() + BOOTSTRAP_FRIST));
    }
    let frist = BootstrapFrist {
        id,
        liste: bootstraps.clone(),
    };

    let mut roh: Vec<u8> = Vec::with_capacity(4096);
    let mut puffer = [0u8; 4096];

    // ── Bootstrap lesen ───────────────────────────────────────────────────
    let rahmen_laenge = loop {
        match bootstrap_rahmenstand(&roh) {
            BootstrapRahmenStand::Vollstaendig(n) => break n,
            BootstrapRahmenStand::ZuGross => {
                statistik
                    .geschlossen_bootstrap
                    .fetch_add(1, Ordering::SeqCst);
                return;
            }
            BootstrapRahmenStand::Unvollstaendig => {}
        }
        match ov_lesen(griff.h, leseereignis.roh(), None, &mut puffer) {
            IoAusgang::Ende => return,
            IoAusgang::Bytes(n) => {
                roh.extend_from_slice(&puffer[..n]);
                // Die Laengengrenze des Hellos selbst prueft `bootstrap_lesen`
                // (ZuGross) — schon ab dem vierten Byte, also vor jeder
                // Allokation. Hier steht nur der Schutz gegen einen Peer, der
                // pipelined: er darf hinter dem Hello schon v3-Frames
                // mitschicken, aber nicht beliebig viel, bevor das Hello
                // ueberhaupt vollstaendig ist. Ein Puffer von Hellogrenze plus
                // einem Lesehaeppchen ist die ehrliche Schranke; die alte
                // Fassung prueft VOR dem Parsen und haette ein legitimes
                // Hello mit angehaengten Frames in einem einzigen Read
                // abgewiesen.
                if roh.len() > (crate::transport::v3::MAX_BOOTSTRAP_BYTES as usize) + 4 + 4096 {
                    statistik
                        .geschlossen_bootstrap
                        .fetch_add(1, Ordering::SeqCst);
                    return;
                }
            }
            IoAusgang::Abgebrochen | IoAusgang::Fehler(_) => {
                statistik
                    .geschlossen_bootstrap
                    .fetch_add(1, Ordering::SeqCst);
                return;
            }
        }
    };
    sicherheits_cleanup.spur("read");

    // Die fachliche Auswertung ist absichtlich NACH der verbindlichen
    // Impersonation/SID/Revert-Kette. Auch ein vollstaendig gerahmtes, aber
    // syntaktisch oder semantisch abzulehnendes Hello darf vorher weder Reject
    // noch Senke/Coordinator erreichen (B-05).
    if !client_sid_authentisieren(griff.h, &erwartete_sicherheit, &mut sicherheits_cleanup) {
        statistik
            .geschlossen_bootstrap
            .fetch_add(1, Ordering::SeqCst);
        return;
    }
    let (bs, verbraucht) = match bootstrap_lesen(&roh[..rahmen_laenge]) {
        Ok(x) => x,
        Err(e) => {
            statistik
                .geschlossen_bootstrap
                .fetch_add(1, Ordering::SeqCst);
            bootstrap_reject_schreiben(
                griff.h,
                leseereignis.roh(),
                &format!("bootstrap: {e:?}"),
                &sicherheits_cleanup,
            );
            return;
        }
    };
    frist.erfuellt();
    roh.drain(0..verbraucht);

    // Erst nach erfolgreichem Revert existiert ein Fachcallback-Helfer.
    let mut senkenruf = Senkenruf::neu(senke.clone(), statistik.clone());

    // Die Writerqueue existiert bereits vor dem Telemetrie-Kopplungs-Callback:
    // ein Coordinator darf dabei den gehaltenen absoluten Messstand an einen
    // reconnectenden Subscriber einreihen, ohne auf Pipe-I/O zu warten.
    let ausgang = Arc::new(Ausgang::neu());
    let (link_id, ist_control) = match bs {
        Bootstrap::V2 { .. } => {
            // Auf dem v3-Endpunkt gibt es kein v2-Register. Statt still zu
            // scheitern bekommt der Peer einen v2-gerahmten `reject` — er
            // spricht ja noch v2-Framing (§33.3 "klarer Kompatibilitaetsfehler").
            let json = "{\"type\":\"reject\",\"code\":\"protocol_mismatch\",\
                        \"reason\":\"dieser Endpunkt spricht nur v3\"}";
            let mut rahmen = (json.len() as u32).to_le_bytes().to_vec();
            rahmen.extend_from_slice(json.as_bytes());
            sicherheits_cleanup.spur("reject");
            let _ = ov_schreiben(griff.h, leseereignis.roh(), &rahmen);
            statistik
                .geschlossen_bootstrap
                .fetch_add(1, Ordering::SeqCst);
            return;
        }
        Bootstrap::V3Control(h) => {
            sicherheits_cleanup.spur("hello_accept");
            let link = neue_kennung();
            let challenge = neue_kennung();
            {
                let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                if k.control_anmelden(&h.adresse.runtime_nonce, link.clone(), challenge.clone())
                    .is_err()
                {
                    statistik
                        .geschlossen_bootstrap
                        .fetch_add(1, Ordering::SeqCst);
                    senkenruf.abweisen("bootstrap: zu viele offene Kopplungen");
                    return;
                }
            }
            // `C-LS-02` (Regel 5): `control_verbunden` ist ABGESCHLOSSEN,
            // bevor das Welcome den Draht verlaesst. Vorher lief es umgekehrt —
            // der Peer bekam sein Welcome, koppelte seine Telemetrieverbindung,
            // und `telemetrie_gekoppelt` erreichte die Senke auf dem anderen
            // Verbindungsthread VOR `control_verbunden`. Die Senke sah den
            // zweiten Teilnehmer einer Kopplung, die sie noch nicht kannte, und
            // `telemetrie_gekoppelt` traegt nur die `link_id` — den fehlenden
            // Kontext konnte sie nicht rekonstruieren (NAK-95, Befund 5).
            //
            // Kommt die Senke nicht binnen `SENKE_FRIST` zurueck, wird der
            // Aufruf abgeloest; dann verlaesst KEIN Welcome den Draht und diese
            // Verbindung endet (`C-LS-03`). Der Abbau laeuft ueber
            // `kopplung_loesen`, nicht ueber ein blosses `control_abmelden`:
            // die Kopplung steht schon im Register und muss ganz fallen.
            //
            // Ein `control_getrennt` folgt hier bewusst NICHT: der abgeloeste
            // `control_verbunden` steht noch; ein Gegenstueck davor waere eine
            // Luege ueber die Reihenfolge. Die Senke sieht ihren Verbindungs-
            // beginn also ohne Ende — sichtbar an `lebenszyklus_abgeloest`.
            let link_fuer_senke = link.clone();
            let hello_fuer_senke = (*h).clone();
            let Some(anmeldung) = senkenruf.rufen_mit_ergebnis(move |s| {
                s.control_verbunden(&link_fuer_senke, &hello_fuer_senke)
            }) else {
                kopplung_loesen(&kopplungen, &handles, &link, true);
                return;
            };
            if !anmeldung.angenommen {
                let grund = anmeldung
                    .grund
                    .unwrap_or_else(|| "Coordinator hat die Verbindung abgewiesen".into());
                let payload = serde_json::json!({
                    "type": "reject",
                    "code": "rate_limited",
                    "reason": grund.chars().take(500).collect::<String>()
                });
                if let Ok(payload) = serde_json::to_vec(&payload) {
                    if let Ok(frame) = envelope_schreiben(Familie::P0, P0_SCHEMA_MINOR, &payload) {
                        let _ = ov_schreiben(griff.h, leseereignis.roh(), &frame);
                    }
                }
                kopplung_loesen(&kopplungen, &handles, &link, true);
                return;
            }
            for alter_link in anmeldung.zu_schliessende_links {
                if alter_link != link {
                    kopplung_loesen(&kopplungen, &handles, &alter_link, true);
                }
            }
            // Ab jetzt kann eine Telemetrieverbindung koppeln; ihr
            // `telemetrie_getrennt` erwartet diese Seite beim Abbau.
            trennmelder_anlegen(&trennmelder, &link);

            let welcome = Welcome {
                typ: "welcome".into(),
                protocol: 3,
                broker_version: broker_version.clone(),
                broker_epoch: broker_epoch.clone(),
                link_id: link.clone(),
                challenge,
            };
            let payload = match serde_json::to_vec(&welcome) {
                Ok(p) => p,
                Err(_) => return,
            };
            match envelope_schreiben(Familie::P0, P0_SCHEMA_MINOR, &payload) {
                Ok(frame) => {
                    if !ov_schreiben(griff.h, leseereignis.roh(), &frame) {
                        senke.control_schliesst(&link);
                        kopplung_loesen(&kopplungen, &handles, &link, true);
                        melden_getrennt(&mut senkenruf, &link, true);
                        trennmelder
                            .lock()
                            .unwrap_or_else(|e| e.into_inner())
                            .remove(&link);
                        return;
                    }
                }
                Err(_) => return,
            }
            (link, true)
        }
        Bootstrap::V3Telemetry(h) => {
            sicherheits_cleanup.spur("hello_accept");
            let ok = {
                let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                k.telemetrie_koppeln(&h, id)
            };
            if let Err(e) = ok {
                statistik
                    .geschlossen_bootstrap
                    .fetch_add(1, Ordering::SeqCst);
                senkenruf.abweisen(format!("telemetry ungekoppelt: {e:?}"));
                return;
            }
            let welcome = Welcome {
                typ: "welcome".into(),
                protocol: 3,
                broker_version: broker_version.clone(),
                broker_epoch: broker_epoch.clone(),
                link_id: h.link_id.clone(),
                challenge: h.challenge.clone(),
            };
            let payload = match serde_json::to_vec(&welcome) {
                Ok(p) => p,
                Err(_) => return,
            };
            if let Ok(frame) = envelope_schreiben(Familie::P0, P0_SCHEMA_MINOR, &payload) {
                if !ov_schreiben(griff.h, leseereignis.roh(), &frame) {
                    let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                    k.telemetrie_entkoppeln(&h.link_id);
                    return;
                }
            }
            // Die Trennpflicht steht VOR dem Kopplungs-Callback, nicht nach
            // seiner Rueckkehr. Blockiert die Senke hier fristgerecht (etwa
            // 800 ms) und enden beide Clients waehrend des Blocks, so fand die
            // Control-Seite an ihrem Wartepunkt `erwartet == false`,
            // uebersprang ihn und meldete `control_getrennt` VOR dem
            // `telemetrie_getrennt`, das gleich danach folgte — die Kopplung
            // war laengst gekoppelt, nur die Zusage fehlte noch (`C-LS-06`,
            // NAK-104 Runde 2, Pruefbefund vom 2026-08-30).
            //
            // Ab hier ist ein `telemetrie_getrennt` zugesagt. Nur ein
            // ABGELOESTES `telemetrie_gekoppelt` nimmt die Zusage zurueck: dann
            // endet diese Verbindung ohne Gegenstueck (`C-LS-04`), und ohne die
            // Ruecknahme wartete die Control-Seite die volle `SENKE_FRIST` auf
            // ein Ereignis, das nie kommt.
            telemetrie_ausgaenge
                .lock()
                .unwrap_or_else(|e| e.into_inner())
                .insert(h.link_id.clone(), ausgang.clone());
            trennmelder_telemetrie_erwartet(&trennmelder, &h.link_id);
            let link_fuer_senke = h.link_id.clone();
            if !senkenruf.rufen(move |s| s.telemetrie_gekoppelt(&link_fuer_senke)) {
                telemetrie_ausgaenge
                    .lock()
                    .unwrap_or_else(|e| e.into_inner())
                    .remove(&h.link_id);
                trennmelder_telemetrie_abgesagt(&trennmelder, &h.link_id);
                kopplung_loesen(&kopplungen, &handles, &h.link_id, false);
                return;
            }
            (h.link_id.clone(), false)
        }
    };

    // ── Ab hier ausschliesslich v3-Frames, auf drei Threads ───────────────
    let eingang = Arc::new(Eingang::neu());
    if ist_control {
        control_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .insert(link_id.clone(), ausgang.clone());
        statistik.aktive_controls.fetch_add(1, Ordering::SeqCst);
    } else {
        telemetrie_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .insert(link_id.clone(), ausgang.clone());
    }

    let schreiber = {
        let griff = griff.clone();
        let ausgang = ausgang.clone();
        let ende = ende.clone();
        let handles = handles.clone();
        let statistik = statistik.clone();
        let cancel_vor_read_phase = cancel_vor_read_phase.clone();
        std::thread::Builder::new()
            .name("eqcop-v3-writer".into())
            .spawn(move || {
                let ereignis = match Ereignis::neu() {
                    Some(e) => e,
                    None => {
                        statistik.geschlossen_writer.fetch_add(1, Ordering::SeqCst);
                        ende.setzen();
                        io_abbrechen(&handles, id);
                        return;
                    }
                };
                while let Some(eintrag) = ausgang.entnehmen() {
                    let erzwungen = writer_fehler_erzwungen.swap(false, Ordering::SeqCst);
                    if erzwungen
                        && cancel_vor_read_phase.load(Ordering::SeqCst) != CANCEL_VOR_READ_INAKTIV
                    {
                        let frist = Instant::now() + Duration::from_secs(5);
                        while cancel_vor_read_phase.load(Ordering::SeqCst) != CANCEL_VOR_READ_READER
                            && Instant::now() < frist
                        {
                            std::thread::yield_now();
                        }
                        if cancel_vor_read_phase.load(Ordering::SeqCst) != CANCEL_VOR_READ_READER {
                            cancel_vor_read_phase.store(CANCEL_VOR_READ_FEHLER, Ordering::SeqCst);
                        }
                    }
                    let geschrieben =
                        !erzwungen && ov_schreiben(griff.h, ereignis.roh(), &eintrag.frame);
                    if let Some(antwort) = eintrag.geschrieben {
                        let _ = antwort.send(geschrieben);
                    }
                    if !geschrieben {
                        statistik.geschlossen_writer.fetch_add(1, Ordering::SeqCst);
                        ende.setzen();
                        io_abbrechen(&handles, id);
                        if cancel_vor_read_phase.load(Ordering::SeqCst) == CANCEL_VOR_READ_READER {
                            cancel_vor_read_phase.store(CANCEL_VOR_READ_WRITER, Ordering::SeqCst);
                        }
                        break;
                    }
                }
            })
            .ok()
    };

    // ZWEI Verbraucher, nicht einer (Matrix `C-LS-07`, Regel 4 auf der
    // Rust-Seite). Mit einem einzigen Thread stand jeder eingereihte P0-Frame
    // hinter einer Senke, die gerade in `p1` blockiert — die Antwort kam erst,
    // wenn fremder Code zurueckkehrte. Das ist P0-Starvation, gleich wie
    // schnell der Leser ist. Die Ingressqueue selbst bleibt EINE Queue mit
    // Cap 256 und der Politik aus §53.9; nur die Sicht darauf ist getrennt.
    let verbraucher_p0 = {
        let senke = senke.clone();
        let eingang = eingang.clone();
        let ausgang = ausgang.clone();
        let statistik = statistik.clone();
        let handles = handles.clone();
        let ende = ende.clone();
        let link = link_id.clone();
        std::thread::Builder::new()
            .name("eqcop-v3-ingress-p0".into())
            .spawn(move || {
                while let Some((_, schema_minor, payload)) = eingang.entnehmen_p0() {
                    if let Some(antwort) = senke.p0_mit_minor(&link, schema_minor, &payload) {
                        if let Ok(frame) =
                            envelope_schreiben(Familie::P0, P0_SCHEMA_MINOR, &antwort)
                        {
                            if !ausgang.einreihen(frame) {
                                // Der Peer holt seine Antworten nicht ab.
                                // Still weiterzaehlen waere eine Luege ueber
                                // "nichts verwerfen".
                                statistik.geschlossen_writer.fetch_add(1, Ordering::SeqCst);
                                senke.abgewiesen("writer: Antwortqueue laeuft ueber");
                                ende.setzen();
                                io_abbrechen(&handles, id);
                                break;
                            }
                        }
                    }
                }
            })
            .ok()
    };

    let verbraucher_rest = {
        let senke = senke.clone();
        let eingang = eingang.clone();
        let link = link_id.clone();
        std::thread::Builder::new()
            .name("eqcop-v3-ingress-rest".into())
            .spawn(move || {
                while let Some((familie, schema_minor, payload)) = eingang.entnehmen_ohne_p0() {
                    match familie {
                        Familie::P1 => senke.p1_mit_minor(&link, schema_minor, &payload),
                        Familie::P2 => senke.p2_mit_minor(&link, schema_minor, &payload),
                        // `entnehmen_ohne_p0` liefert per Konstruktion kein
                        // P0; der Arm ist der Riegel gegen eine spaetere
                        // Aenderung, nicht toter Code.
                        Familie::P0 => debug_assert!(false, "P0 gehoert dem eigenen Thread"),
                    }
                }
            })
            .ok()
    };

    let mut leser = StromLeser::neu();
    leser.fuettern(&roh);
    let mut rate = Ratengrenze::neu(RATE_PRO_SEKUNDE, 1000);
    let beginn = Instant::now();
    let mut cancel_vor_read_barriere = false;

    'lesen: loop {
        loop {
            match leser.naechster() {
                LeseErgebnis::Unvollstaendig => break,
                LeseErgebnis::Verstoesse(v) => {
                    statistik
                        .geschlossen_envelope
                        .fetch_add(1, Ordering::SeqCst);
                    let namen: Vec<&str> = v.iter().map(|x| x.name()).collect();
                    senkenruf.abweisen(format!("envelope: {}", namen.join(",")));
                    break 'lesen;
                }
                LeseErgebnis::Frame(r) => {
                    let jetzt_ms = beginn.elapsed().as_millis() as u64;
                    if !rate.erlaubt(jetzt_ms) {
                        statistik.geschlossen_rate.fetch_add(1, Ordering::SeqCst);
                        senkenruf.abweisen("rate: Nachrichtenratengrenze ueberschritten");
                        break 'lesen;
                    }
                    let familie = r.kopf.familie;
                    let schema_minor = r.kopf.schema_minor;

                    if !schema_minor_bekannt(familie, schema_minor) {
                        statistik
                            .geschlossen_envelope
                            .fetch_add(1, Ordering::SeqCst);
                        senkenruf.abweisen(format!(
                            "envelope: schema_minor {schema_minor} fuer {familie:?} unbekannt"
                        ));
                        break 'lesen;
                    }

                    // Familienzuordnung des Vertrags: die Control-Verbindung
                    // traegt P0/P1, die Telemetrieverbindung traegt P2
                    // (`eq-ipc-v3.schema.json`, hello_control/hello_telemetry).
                    // Ohne diese Sperre koennte eine Telemetriepipe einen
                    // gueltigen P0-Heartbeat setzen und umgekehrt ein
                    // Featureframe ueber die Steuerleitung laufen.
                    let erlaubt = if ist_control {
                        familie != Familie::P2
                    } else {
                        familie == Familie::P2
                    };
                    if !erlaubt {
                        statistik.geschlossen_familie.fetch_add(1, Ordering::SeqCst);
                        senkenruf.abweisen(format!(
                            "familie: {familie:?} auf einer {}-Verbindung",
                            if ist_control { "control" } else { "telemetry" }
                        ));
                        break 'lesen;
                    }

                    // Die Control-Verbindung BESITZT die Kopplung. Endet sie,
                    // gehoert dieser Telemetrieframe zu keiner Sitzung mehr.
                    if !ist_control {
                        let lebt = {
                            let k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                            k.telemetrie_lebt(&link_id, id)
                        };
                        if !lebt {
                            statistik
                                .geschlossen_kopplung
                                .fetch_add(1, Ordering::SeqCst);
                            senkenruf.abweisen("kopplung: Control-Verbindung ist fort");
                            break 'lesen;
                        }
                    }

                    match eingang.einreihen(familie, schema_minor, r.payload) {
                        IngressErgebnis::Eingereiht => {}
                        IngressErgebnis::P2Verworfen => {
                            statistik
                                .ingress_p2_verworfen
                                .fetch_add(1, Ordering::SeqCst);
                        }
                        IngressErgebnis::ClientTrennen => {
                            // `A-IN-03`/`A-IN-04`: P0 UND P1 trennen. Ein
                            // stiller P1-Verlust bei gesund wirkender
                            // Verbindung nimmt dem Sender den Wiederholweg.
                            if familie == Familie::P1 {
                                statistik
                                    .ingress_p1_ueberlauf_trennt
                                    .fetch_add(1, Ordering::SeqCst);
                                senkenruf.abweisen("ingress: P1-Ueberlauf");
                            } else {
                                statistik
                                    .geschlossen_p0_ueberlauf
                                    .fetch_add(1, Ordering::SeqCst);
                                senkenruf.abweisen("ingress: P0-Ueberlauf");
                            }
                            break 'lesen;
                        }
                        IngressErgebnis::Verworfen => {
                            statistik
                                .ingress_p2_verworfen
                                .fetch_add(1, Ordering::SeqCst);
                        }
                    }
                    statistik.hoechststand_melden(eingang.laenge());
                }
            }
        }

        if stop.load(Ordering::SeqCst) || ende.gesetzt() {
            break;
        }
        // Harte Obergrenze fuer den Lesepuffer: ein Peer darf nicht beliebig
        // viele Teilbytes anhaeufen, ohne je einen Frame zu vollenden.
        if leser.offen() > MAX_FRAME_BYTES as usize + 4 {
            statistik
                .geschlossen_envelope
                .fetch_add(1, Ordering::SeqCst);
            senkenruf.abweisen("envelope: Teilframe ueber der Paketgrenze");
            break;
        }
        if std::mem::take(&mut cancel_vor_read_barriere) {
            // Deterministische Testbarriere fuer genau das verlorene
            // Cancel-vor-Read-Fenster: Der letzte Ende-Check liegt hinter uns,
            // der naechste Read wurde aber noch nicht abgesetzt.
            if cancel_vor_read_phase
                .compare_exchange(
                    CANCEL_VOR_READ_GELESEN,
                    CANCEL_VOR_READ_READER,
                    Ordering::SeqCst,
                    Ordering::SeqCst,
                )
                .is_err()
            {
                cancel_vor_read_phase.store(CANCEL_VOR_READ_FEHLER, Ordering::SeqCst);
            }
            let frist = Instant::now() + Duration::from_secs(5);
            while cancel_vor_read_phase.load(Ordering::SeqCst) != CANCEL_VOR_READ_WRITER
                && Instant::now() < frist
            {
                std::thread::yield_now();
            }
            let phase = if cancel_vor_read_phase.load(Ordering::SeqCst) == CANCEL_VOR_READ_WRITER {
                CANCEL_VOR_READ_FORTGESETZT
            } else {
                CANCEL_VOR_READ_FEHLER
            };
            cancel_vor_read_phase.store(phase, Ordering::SeqCst);
        }
        match ov_lesen(griff.h, leseereignis.roh(), Some(&ende), &mut puffer) {
            IoAusgang::Bytes(n) => {
                leser.fuettern(&puffer[..n]);
                cancel_vor_read_barriere = cancel_vor_read_phase
                    .compare_exchange(
                        CANCEL_VOR_READ_BEREIT,
                        CANCEL_VOR_READ_GELESEN,
                        Ordering::SeqCst,
                        Ordering::SeqCst,
                    )
                    .is_ok();
            }
            IoAusgang::Ende | IoAusgang::Abgebrochen => break,
            IoAusgang::Fehler(f) => {
                senkenruf.abweisen(format!("lesen: Win32 {f}"));
                break;
            }
        }
    }

    // Gegenpfad zu den drei Threads: erst die Queues schliessen, dann die
    // haengende I/O aufloesen, dann joinen. Ein Schreiber, der auf einen
    // stillen Peer wartet, muss abgebrochen werden — sonst waere `join` ein
    // Hang.
    ende.setzen();
    // 28-B: Kein neuer Ingress darf den atomaren Cleanup ueberholen. Ein
    // schon laufender Coordinator-Aufruf teilt dessen Lock; entweder commitet
    // er vorher und wird hier entfernt, oder er sieht danach `schliessend`.
    eingang.schliessen();
    if ist_control {
        control_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .remove(&link_id);
        statistik.aktive_controls.fetch_sub(1, Ordering::SeqCst);
        senke.control_schliesst(&link_id);
    } else {
        telemetrie_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .remove(&link_id);
    }
    // Nach dem atomaren semantischen Gegenpfad faellt die KOPPLUNG ebenfalls
    // vor den fristbegrenzten Joins, nicht nach ihnen. Sonst blieb sie bei
    // langsamer Senke bis zu zweimal `SENKE_FRIST`
    // im Register, und ein Telemetrieframe passierte in dieser Zeit weiter
    // `telemetrie_lebt`, obwohl die Control-Verbindung schon fort war
    // (T2-Befund 3 Runde 3 vom 2026-08-29). Der Registereintrag und der
    // Abbruch der Telemetrie-I/O gehoeren dabei zusammen: ohne den Abbruch
    // stuende der Telemetriearbeiter noch in seinem Read.
    kopplung_loesen(&kopplungen, &handles, &link_id, ist_control);
    ausgang.schliessen();
    io_abbrechen(&handles, id);
    // Alle Joins haben eine FRIST. Steht ein Verbraucher in einem
    // Senkenaufruf oder der Schreiber in einem Write, den `CancelIoEx` nicht
    // loest, wird der Thread abgeloest statt gejoint — sonst waere `stoppen()`
    // ein unbegrenztes Warten auf fremden Code (T2-Befund 7 vom 2026-08-29).
    for j in [verbraucher_p0, verbraucher_rest].into_iter().flatten() {
        if !join_mit_frist(j, SENKE_FRIST, || {}) {
            statistik.senke_abgeloest.fetch_add(1, Ordering::SeqCst);
        }
    }
    if let Some(j) = schreiber {
        if !join_mit_frist(j, SENKE_FRIST, || io_abbrechen(&handles, id)) {
            statistik.schreiber_abgeloest.fetch_add(1, Ordering::SeqCst);
        }
    }

    // Erst JETZT die Senke benachrichtigen: waehrend der Joins konnte noch ein
    // `p0`/`p1`/`p2` derselben Verbindung laufen, und ein `*_getrennt` davor
    // waere eine Luege ueber die Reihenfolge.
    //
    // `C-LS-06`: die Control-Seite meldet ihr `control_getrennt` NACH dem
    // `telemetrie_getrennt` der mitfallenden Telemetrieverbindung. Zwischen
    // zwei Verbindungsthreads ist die Reihenfolge sonst unbestimmt; die Senke
    // saehe das Ende der Kopplung vor dem Ende ihres Teilnehmers. Gewartet
    // wird hoechstens `SENKE_FRIST` — laeuft sie ab (weil das
    // `telemetrie_getrennt` der Gegenseite selbst abgeloest wurde), meldet
    // diese Seite TROTZDEM: ein abgeloester Trenn-Callback zaehlt als
    // gelaufen, es entfaellt keiner und keiner laeuft doppelt.
    if ist_control {
        auf_telemetrie_getrennt_warten(&trennmelder, &link_id, SENKE_FRIST, &statistik);
    }
    melden_getrennt(&mut senkenruf, &link_id, ist_control);
    if !ist_control {
        telemetrie_getrennt_gemeldet(&trennmelder, &link_id);
    } else {
        trennmelder
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .remove(&link_id);
    }
}

/// Stand einer Kopplung fuer die Trennreihenfolge (Matrix `C-LS-06`).
///
/// Beide Tatsachen liegen unter EINEM Schloss, weil die Control-Seite sie
/// zusammen liest: "kommt ueberhaupt ein `telemetrie_getrennt`" und "ist es
/// schon da".
#[derive(Default)]
struct TrennStand {
    /// Eine Telemetrieverbindung hat sich auf diese `link_id` gekoppelt und
    /// wird deshalb ein `telemetrie_getrennt` melden.
    ///
    /// Diese Tatsache MUSS getrennt vom Kopplungsregister stehen. Endet die
    /// Telemetrieverbindung zuerst, nimmt sie ihren Registereintrag schon vor
    /// den Joins heraus; `kopplung_loesen` der Control-Seite faende danach
    /// keine Telemetrie mehr und uebersprang den Wartepunkt, obwohl deren
    /// Trenn-Callback noch lief — `control_getrennt` konnte vor
    /// `telemetrie_getrennt` laufen (NAK-104, Pruefbefund vom 2026-08-30).
    ///
    /// Sie gilt ab der KOPPLUNG, nicht erst ab der Rueckkehr von
    /// `telemetrie_gekoppelt`: waehrend dieses Callbacks ist die Kopplung
    /// bereits vollzogen, und beide Clients koennen in diesem Fenster enden
    /// (NAK-104 Runde 2). Zurueckgenommen wird sie nur, wenn der Callback
    /// ABGELOEST wurde — dann folgt kein Gegenstueck (`C-LS-04`).
    erwartet: bool,
    /// Ihr `telemetrie_getrennt` ist gemeldet — oder abgeloest, was nach
    /// `C-LS-06` als gelaufen zaehlt.
    gemeldet: bool,
}

/// Wartepunkt fuer die Trennreihenfolge einer Kopplung (Matrix `C-LS-06`).
/// Die Telemetrieseite setzt ihn, nachdem sie `telemetrie_getrennt` gemeldet
/// hat; die Control-Seite wartet darauf, bevor sie `control_getrennt` meldet.
#[derive(Default)]
struct TrennMelder {
    stand: Mutex<TrennStand>,
    signal: Condvar,
}

/// Ein Melder je lebender Kopplung. Er wird mit `control_verbunden` angelegt
/// und mit `control_getrennt` entfernt — derselbe Aenderungssatz wie das
/// Register selbst.
type TrennRegister = Arc<Mutex<HashMap<String, Arc<TrennMelder>>>>;

fn trennmelder_anlegen(reg: &TrennRegister, link_id: &str) {
    reg.lock()
        .unwrap_or_else(|e| e.into_inner())
        .insert(link_id.to_string(), Arc::new(TrennMelder::default()));
}

/// Ab jetzt wartet die Control-Seite beim Abbau auf ein `telemetrie_getrennt`.
/// Gesetzt wird das, sobald die Telemetrie gekoppelt ist und ihr
/// `telemetrie_gekoppelt` laufen KANN — also VOR dem Callback, nicht nach
/// seiner Rueckkehr (`C-LS-06`, NAK-104 Runde 2).
fn trennmelder_telemetrie_erwartet(reg: &TrennRegister, link_id: &str) {
    let m = reg
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .get(link_id)
        .cloned();
    if let Some(m) = m {
        m.stand.lock().unwrap_or_else(|e| e.into_inner()).erwartet = true;
        m.signal.notify_all();
    }
}

/// Das Gegenstueck zu `trennmelder_telemetrie_erwartet`: das
/// `telemetrie_gekoppelt` wurde ABGELOEST, diese Verbindung meldet deshalb kein
/// `telemetrie_getrennt` (`C-LS-04`). Ohne die Ruecknahme haengt die
/// Control-Seite die volle `SENKE_FRIST` und zaehlt eine Reihenfolge-
/// verletzung, die keine ist.
fn trennmelder_telemetrie_abgesagt(reg: &TrennRegister, link_id: &str) {
    let m = reg
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .get(link_id)
        .cloned();
    if let Some(m) = m {
        m.stand.lock().unwrap_or_else(|e| e.into_inner()).erwartet = false;
        m.signal.notify_all();
    }
}

fn telemetrie_getrennt_gemeldet(reg: &TrennRegister, link_id: &str) {
    let m = reg
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .get(link_id)
        .cloned();
    if let Some(m) = m {
        m.stand.lock().unwrap_or_else(|e| e.into_inner()).gemeldet = true;
        m.signal.notify_all();
    }
}

/// `C-LS-06`: hoechstens `frist` warten, dann in jedem Fall weitermelden.
fn auf_telemetrie_getrennt_warten(
    reg: &TrennRegister,
    link_id: &str,
    frist: Duration,
    statistik: &Arc<V3Statistik>,
) {
    let m = match reg
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .get(link_id)
        .cloned()
    {
        Some(m) => m,
        None => return,
    };
    let bis = Instant::now() + frist;
    let mut g = m.stand.lock().unwrap_or_else(|e| e.into_inner());
    // Nie eine Telemetrieverbindung gehabt — oder die Zusage faellt waehrend
    // des Wartens zurueck, weil ihr `telemetrie_gekoppelt` abgeloest wurde:
    // beides heisst "es kommt kein Gegenstueck", und beides ist KEINE
    // Reihenfolgeverletzung. Die Bedingung gehoert deshalb in die Schleife und
    // nicht nur davor.
    while g.erwartet && !g.gemeldet {
        let rest = bis.saturating_duration_since(Instant::now());
        if rest.is_zero() {
            statistik
                .lebenszyklus_reihenfolge_verletzt
                .fetch_add(1, Ordering::SeqCst);
            return;
        }
        let (neu, _) = m
            .signal
            .wait_timeout(g, rest)
            .unwrap_or_else(|e| e.into_inner());
        g = neu;
    }
}

/// Nimmt die Kopplung aus dem Register und bricht die I/O der mitfallenden
/// Telemetrieverbindung ab. Beruehrt die Senke NICHT — die Meldung folgt
/// getrennt, nach den Joins (`melden_getrennt`).
///
/// Ob die Control-Seite auf ein `telemetrie_getrennt` warten muss, entscheidet
/// diese Funktion NICHT: das Register sagt nur, ob die Telemetrie JETZT NOCH
/// haengt. Endete sie zuerst, ist ihr Eintrag laengst fort, waehrend ihr
/// Trenn-Callback noch laeuft. Diese Frage beantwortet allein `TrennStand`
/// (`C-LS-06`, NAK-104).
fn kopplung_loesen(
    kopplungen: &Arc<Mutex<Kopplungen>>,
    handles: &Arc<Mutex<HandleRegister>>,
    link_id: &str,
    ist_control: bool,
) {
    if ist_control {
        // Die Control-Verbindung besitzt die Kopplung: geht sie, geht auch
        // der Telemetrieplatz. Sonst bliebe eine halb offene Kopplung stehen
        // (Fehlerlexikon: "zwei Pipes halb verbunden"). Den Registereintrag zu
        // entfernen genuegt nicht — der Telemetriearbeiter laeuft weiter und
        // liefert P2 zu einer Sitzung, die es nicht mehr gibt. Er wird deshalb
        // hier ABGEBROCHEN (T2-Befund 2 vom 2026-08-29).
        let ab = {
            let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
            k.control_abmelden(link_id)
        };
        if let Some(v) = ab.telemetrie_verbindung {
            io_abbrechen(handles, v);
        }
    } else {
        let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
        k.telemetrie_entkoppeln(link_id);
    }
}

/// Das Gegenstueck der Verbindungsmeldung, ueber den fristbegrenzten Pfad.
///
/// Unbedingt melden: die Senke hat ein `control_verbunden` bzw.
/// `telemetrie_gekoppelt` bekommen und braucht sein Gegenstueck auch dann,
/// wenn die Control-Verbindung die Kopplung schon aus dem Register genommen
/// hat. Nur ein frueher ABGELOESTER Aufruf laesst diesen hier ausfallen — dann
/// steht die Senke ohnehin noch im vorigen und wuerde ihn in falscher
/// Reihenfolge sehen; der Ausfall ist als `lebenszyklus_uebersprungen`
/// sichtbar.
fn melden_getrennt(senkenruf: &mut Senkenruf, link_id: &str, ist_control: bool) {
    let l = link_id.to_string();
    if ist_control {
        senkenruf.rufen(move |s| s.control_getrennt(&l));
    } else {
        senkenruf.rufen(move |s| s.telemetrie_getrennt(&l));
    }
}

//==============================================================================

#[cfg(test)]
mod tests {
    use super::*;
    use crate::transport::bootstrap::Adresse;
    use crate::transport::pipetoken::PROBE_PRAEFIX;
    use std::sync::atomic::AtomicUsize;

    static FOLGE: AtomicUsize = AtomicUsize::new(0);

    #[test]
    fn bootstrap_und_senkenfrist_minus_eins_und_exakt_sind_inklusive() {
        assert_eq!(BOOTSTRAP_FRIST, Duration::from_millis(5000));
        assert_eq!(SENKE_FRIST, Duration::from_millis(2000));
        let basis = Instant::now();
        for frist in [BOOTSTRAP_FRIST, SENKE_FRIST] {
            let deadline = basis + frist;
            assert!(basis + frist - Duration::from_millis(1) < deadline);
            assert!(basis + frist >= deadline);
        }

        let source = include_str!("mod.rs");
        let testmodul_marker = ["\n#[cfg(test)]\n", "mod tests {"].concat();
        let tests_beginn = source
            .rfind(&testmodul_marker)
            .expect("Testmodul-Marker im eigenen Quelltext");
        let produkt = &source[..tests_beginn];
        assert!(produkt.contains(".filter(|(_, f)| *f <= jetzt)"));
        assert!(produkt.contains("if Instant::now() >= bis"));

        let (blockieren, empfang) = std::sync::mpsc::channel::<()>();
        let thread = std::thread::spawn(move || {
            let _ = empfang.recv();
        });
        assert!(!join_mit_frist(thread, Duration::ZERO, || {}));
        drop(blockieren);
    }

    fn probe_pipe(fall: &str) -> String {
        format!(
            "{PROBE_PRAEFIX}srv.{}.{}.{fall}",
            std::process::id(),
            FOLGE.fetch_add(1, Ordering::SeqCst)
        )
    }

    /// Ein roher Testclient. Synchron, weil der Test seine Schritte ohnehin
    /// nacheinander geht.
    struct Testclient {
        h: HANDLE,
    }

    // SAFETY: wie beim Verbindungsgriff — ein Pipe-HANDLE hat keine
    // Threadaffinitaet.
    unsafe impl Send for Testclient {}

    impl Testclient {
        fn neu(pipe: &str) -> Option<Self> {
            Self::mit_geduld(pipe, 10_000)
        }

        fn mit_geduld(pipe: &str, millis: u64) -> Option<Self> {
            let mut w: Vec<u16> = pipe.encode_utf16().collect();
            w.push(0);
            let frist = Instant::now() + Duration::from_millis(millis);
            loop {
                // SAFETY: `w` ist nullterminiert; das Handle geht in den Typ
                // ueber und wird im Drop genau einmal geschlossen.
                let h = unsafe {
                    windows_sys::Win32::Storage::FileSystem::CreateFileW(
                        w.as_ptr(),
                        windows_sys::Win32::Foundation::GENERIC_READ
                            | windows_sys::Win32::Foundation::GENERIC_WRITE,
                        0,
                        std::ptr::null(),
                        windows_sys::Win32::Storage::FileSystem::OPEN_EXISTING,
                        windows_sys::Win32::Storage::FileSystem::SECURITY_SQOS_PRESENT
                            | windows_sys::Win32::Storage::FileSystem::SECURITY_IDENTIFICATION,
                        std::ptr::null_mut(),
                    )
                };
                if h != INVALID_HANDLE_VALUE {
                    return Some(Testclient { h });
                }
                if Instant::now() >= frist {
                    return None;
                }
                std::thread::sleep(Duration::from_millis(10));
            }
        }

        fn schreiben(&self, daten: &[u8]) -> bool {
            let mut ab = 0usize;
            while ab < daten.len() {
                let mut n: u32 = 0;
                // SAFETY: synchrones Handle, gueltiger Puffer.
                let ok = unsafe {
                    WriteFile(
                        self.h,
                        daten[ab..].as_ptr(),
                        (daten.len() - ab) as u32,
                        &mut n,
                        std::ptr::null_mut(),
                    )
                };
                if ok == 0 || n == 0 {
                    return false;
                }
                ab += n as usize;
            }
            true
        }

        fn lesen(&self, ziel: &mut [u8]) -> usize {
            let mut n: u32 = 0;
            // SAFETY: synchrones Handle, gueltiger Puffer.
            let ok = unsafe {
                ReadFile(
                    self.h,
                    ziel.as_mut_ptr(),
                    ziel.len() as u32,
                    &mut n,
                    std::ptr::null_mut(),
                )
            };
            if ok == 0 {
                0
            } else {
                n as usize
            }
        }
    }

    impl Drop for Testclient {
        fn drop(&mut self) {
            // SAFETY: exklusiver Besitz, genau einmal geschlossen.
            unsafe { CloseHandle(self.h) };
        }
    }

    fn praefix(json: &str) -> Vec<u8> {
        let mut aus = (json.len() as u32).to_le_bytes().to_vec();
        aus.extend_from_slice(json.as_bytes());
        aus
    }

    fn adresse_json(nonce: &str) -> String {
        format!(
            "{{\"logon_sid\":\"S-1-5-21-1-2-3-1001\",\"project_binding_id\":\"{p}\",\
             \"session_epoch\":\"{p}\",\"instance_id\":\"{p}\",\"runtime_nonce\":\"{nonce}\"}}",
            p = "0".repeat(32)
        )
    }

    fn control_hello(nonce: &str) -> Vec<u8> {
        praefix(&format!(
            "{{\"type\":\"hello\",\"connection_kind\":\"control\",\"protocol\":3,\
             \"plugin_version\":\"0.3.0\",\"plugin_kind\":\"active_probe\",\"adresse\":{a},\
             \"audio\":{{\"samplerate\":48000,\"block_size\":512,\"channels\":2}}}}",
            a = adresse_json(nonce)
        ))
    }

    fn test_adresse(zeichen: char) -> Adresse {
        Adresse {
            logon_sid: "S-1-5-21-1-2-3-1001".into(),
            project_binding_id: zeichen.to_string().repeat(32),
            session_epoch: zeichen.to_string().repeat(32),
            instance_id: zeichen.to_string().repeat(32),
            runtime_nonce: zeichen.to_string().repeat(32),
        }
    }

    fn control_hello_adresse(adresse: &Adresse) -> Vec<u8> {
        praefix(&format!(
            "{{\"type\":\"hello\",\"connection_kind\":\"control\",\"protocol\":3,\
             \"plugin_version\":\"0.3.0\",\"plugin_kind\":\"active_probe\",\"adresse\":{},\
             \"audio\":{{\"samplerate\":48000,\"block_size\":512,\"channels\":2}}}}",
            serde_json::to_string(adresse).unwrap()
        ))
    }

    fn control_hello_fach(adresse: &Adresse, plugin_kind: &str, host_pid: Option<u32>) -> Vec<u8> {
        let host = host_pid
            .map(|pid| format!(",\"host\":{{\"pid\":{pid},\"name\":\"FL Studio\"}}"))
            .unwrap_or_default();
        praefix(&format!(
            "{{\"type\":\"hello\",\"connection_kind\":\"control\",\"protocol\":3,\
             \"plugin_version\":\"0.3.0\",\"plugin_kind\":\"{plugin_kind}\",\"adresse\":{}{host},\
             \"audio\":{{\"samplerate\":48000,\"block_size\":512,\"channels\":2}}}}",
            serde_json::to_string(adresse).unwrap()
        ))
    }

    fn subscribe(adresse: &Adresse) -> Vec<u8> {
        p1(&serde_json::json!({
            "type": "subscribe_session",
            "adresse": adresse,
            "session_epoch": adresse.session_epoch.clone()
        })
        .to_string())
    }

    fn heartbeat_fuer_adresse(adresse: &Adresse, sequence: u64) -> Vec<u8> {
        vollstaendiger_heartbeat(adresse, sequence)
    }

    fn vollstaendiger_heartbeat(adresse: &Adresse, sequence: u64) -> Vec<u8> {
        p0(&serde_json::json!({
            "type": "heartbeat",
            "adresse": adresse,
            "sequence": sequence,
            "state_revision": 0,
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
            "zaehler": {
                "frames_dropped": 0,
                "parse_errors": 0,
                "queue_overflows": 0
            },
            "runtime": {"messpunkt": "insert", "betrieb": "active"}
        })
        .to_string())
    }

    fn telemetry_hello(nonce: &str, link: &str, challenge: &str) -> Vec<u8> {
        praefix(&format!(
            "{{\"type\":\"hello\",\"connection_kind\":\"telemetry\",\"protocol\":3,\
             \"plugin_version\":\"0.3.0\",\"adresse\":{a},\"link_id\":\"{link}\",\
             \"challenge\":\"{challenge}\"}}",
            a = adresse_json(nonce)
        ))
    }

    fn telemetry_hello_adresse(adresse: &Adresse, link: &str, challenge: &str) -> Vec<u8> {
        praefix(
            &serde_json::json!({
                "type": "hello",
                "connection_kind": "telemetry",
                "protocol": 3,
                "plugin_version": "0.3.0",
                "adresse": adresse,
                "link_id": link,
                "challenge": challenge
            })
            .to_string(),
        )
    }

    /// Liest das v3-gerahmte `welcome` und gibt (link_id, challenge).
    fn welcome_lesen(c: &Testclient) -> Option<(String, String)> {
        let mut puffer = [0u8; 4096];
        let mut roh: Vec<u8> = Vec::new();
        for _ in 0..50 {
            let n = c.lesen(&mut puffer);
            if n == 0 {
                return None;
            }
            roh.extend_from_slice(&puffer[..n]);
            if let Ok(r) = crate::transport::v3::envelope_pruefen(&roh) {
                let wert: serde_json::Value = serde_json::from_slice(&r.payload).ok()?;
                let link = wert.get("link_id")?.as_str()?.to_string();
                let ch = wert.get("challenge")?.as_str()?.to_string();
                return Some((link, ch));
            }
        }
        None
    }

    fn p0(json: &str) -> Vec<u8> {
        envelope_schreiben(Familie::P0, P0_SCHEMA_MINOR, json.as_bytes()).unwrap()
    }
    fn p1(json: &str) -> Vec<u8> {
        envelope_schreiben(Familie::P1, P1_SCHEMA_MINOR, json.as_bytes()).unwrap()
    }
    fn p2(bytes: &[u8]) -> Vec<u8> {
        envelope_schreiben(Familie::P2, P2_SCHEMA_MINOR, bytes).unwrap()
    }

    fn warte_auf(millis: u64, mut bedingung: impl FnMut() -> bool) -> bool {
        let frist = Instant::now() + Duration::from_millis(millis);
        while Instant::now() < frist {
            if bedingung() {
                return true;
            }
            std::thread::sleep(Duration::from_millis(5));
        }
        bedingung()
    }

    /// Eine Senke, die in `p1`/`p2` blockiert — der Gegenspieler, ohne den
    /// "der Leser haengt nicht an der Senke" keine pruefbare Aussage ist.
    #[derive(Default)]
    struct BlockSenke {
        zaehl: ZaehlSenke,
        blockiert: AtomicBool,
        /// Steht gerade ein Aufruf IN der Senke? Ohne diese Zahl misst ein
        /// Test ueber den Senkenhang nur seine eigene Hoffnung.
        in_senke: AtomicBool,
    }

    impl BlockSenke {
        /// Blockiert hoechstens `kBlockFrist`. Eine Senke, die WIRKLICH ewig
        /// haengt, wuerde einen roten Test in einen Hang verwandeln — und ein
        /// Hang sagt nichts. Die Frist ist um Groessenordnungen laenger als
        /// jede Wartezeit im Test.
        fn warten(&self) {
            self.in_senke.store(true, Ordering::SeqCst);
            let bis = Instant::now() + Duration::from_secs(20);
            while self.blockiert.load(Ordering::SeqCst) && Instant::now() < bis {
                std::thread::sleep(Duration::from_millis(5));
            }
            self.in_senke.store(false, Ordering::SeqCst);
        }
    }

    impl Senke for BlockSenke {
        fn control_verbunden(&self, l: &str, h: &HelloControl) -> ControlAnmeldung {
            self.zaehl.control_verbunden(l, h)
        }
        fn control_getrennt(&self, l: &str) {
            self.zaehl.control_getrennt(l);
        }
        fn telemetrie_gekoppelt(&self, l: &str) {
            self.zaehl.telemetrie_gekoppelt(l);
        }
        fn telemetrie_getrennt(&self, l: &str) {
            self.zaehl.telemetrie_getrennt(l);
        }
        fn p0(&self, l: &str, p: &[u8]) -> Option<Vec<u8>> {
            self.warten();
            self.zaehl.p0(l, p)
        }
        fn p1(&self, l: &str, p: &[u8]) {
            self.warten();
            self.zaehl.p1(l, p);
        }
        fn p2(&self, l: &str, p: &[u8]) {
            self.warten();
            self.zaehl.p2(l, p);
        }
        fn abgewiesen(&self, g: &str) {
            self.zaehl.abgewiesen(g);
        }
    }

    /// T2-Befund 3: mit blockierender Senke muss der LESER weiterlaufen. Der
    /// Beweis ist die Ingressqueue: sie wird voll (Hoechststand 256), P2
    /// faellt zuerst — beides war in der alten Fassung unerreichbar, weil der
    /// Ingress nach jedem einzelnen Frame geleert wurde.
    #[test]
    fn blockierende_senke_haelt_den_leser_nicht_auf() {
        let pipe = probe_pipe("blocksenke");
        let senke = Arc::new(BlockSenke::default());
        senke.blockiert.store(true, Ordering::SeqCst);
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"a".repeat(32))));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome");

        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&"a".repeat(32), &link, &challenge)));
        assert!(welcome_lesen(&tele).is_some());

        // 600 P2-Frames gegen eine Senke, die nichts abholt.
        let frame = p2(&[0x5Au8; 64]);
        for _ in 0..600 {
            if !tele.schreiben(&frame) {
                break;
            }
        }

        let stat = griff.statistik.clone();
        assert!(
            warte_auf(5000, || stat.ingress_p2_verworfen.load(Ordering::SeqCst)
                > 0),
            "der Ingress muss ueberlaufen, sonst hat der Leser am ersten Frame gewartet"
        );
        assert!(
            stat.ingress_hoechststand.load(Ordering::SeqCst) > 1,
            "Hoechststand {} — eine Queue, die nie ueber 1 waechst, ist keine Queue",
            stat.ingress_hoechststand.load(Ordering::SeqCst)
        );
        senke.blockiert.store(false, Ordering::SeqCst);
    }

    /// T2-Befund 3, zweite Haelfte: laeuft der Ingress mit P0 ueber und liegt
    /// kein P2 zum Verwerfen darin, wird die Verbindung getrennt.
    #[test]
    fn p0_ueberlauf_trennt_die_verbindung() {
        let pipe = probe_pipe("p0ueberlauf");
        let senke = Arc::new(BlockSenke::default());
        senke.blockiert.store(true, Ordering::SeqCst);
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"b".repeat(32))));
        assert!(welcome_lesen(&steuer).is_some());

        // Erst den Ingress mit P1 fuellen (Cap 256) — aber KEINEN Frame
        // weiter: seit `A-IN-04` traegt der 257. P1 selbst die Trennung, und
        // dann spraeche dieser Test ueber den falschen Pfad. Deshalb wird bis
        // zum Hoechststand getaktet und danach P0 nachgelegt.
        let stat = griff.statistik.clone();
        let eins = p1("{\"type\":\"state_report\"}");
        for _ in 0..400 {
            if stat.ingress_hoechststand.load(Ordering::SeqCst) >= CAP_INGRESS as u64 {
                break;
            }
            if !steuer.schreiben(&eins) {
                break;
            }
            std::thread::sleep(Duration::from_millis(1));
        }
        assert_eq!(
            stat.ingress_p1_ueberlauf_trennt.load(Ordering::SeqCst),
            0,
            "die Vorbereitung darf nicht schon ueber den P1-Pfad getrennt haben"
        );
        let herz = p0("{\"type\":\"heartbeat\",\"sequence\":1}");
        for _ in 0..40 {
            if !steuer.schreiben(&herz) {
                break;
            }
        }

        assert!(
            warte_auf(6000, || stat
                .geschlossen_p0_ueberlauf
                .load(Ordering::SeqCst)
                > 0),
            "P0-Ueberlauf muss die Verbindung trennen (p1_ueberlauf_trennt={}, hoechststand={})",
            stat.ingress_p1_ueberlauf_trennt.load(Ordering::SeqCst),
            stat.ingress_hoechststand.load(Ordering::SeqCst)
        );
        senke.blockiert.store(false, Ordering::SeqCst);
    }

    /// T2-Befund 5: der Vertrag ordnet die Familien den Verbindungsarten zu.
    #[test]
    fn p0_auf_der_telemetriepipe_wird_abgewiesen() {
        let pipe = probe_pipe("familie-tele");
        let senke = Arc::new(ZaehlSenke::default());
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"c".repeat(32))));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome");

        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&"c".repeat(32), &link, &challenge)));
        assert!(welcome_lesen(&tele).is_some());

        assert!(tele.schreiben(&p0("{\"type\":\"heartbeat\",\"sequence\":1}")));

        let stat = griff.statistik.clone();
        assert!(
            warte_auf(4000, || stat.geschlossen_familie.load(Ordering::SeqCst)
                == 1),
            "ein P0 auf der Telemetriepipe muss die Verbindung schliessen"
        );
        assert_eq!(
            senke.p0.load(Ordering::SeqCst),
            0,
            "und er darf die Senke nie erreichen"
        );
    }

    /// T2-Befund 5, Gegenrichtung.
    #[test]
    fn p2_auf_der_controlpipe_wird_abgewiesen() {
        let pipe = probe_pipe("familie-control");
        let senke = Arc::new(ZaehlSenke::default());
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"d".repeat(32))));
        assert!(welcome_lesen(&steuer).is_some());
        assert!(steuer.schreiben(&p2(&[1u8; 32])));

        let stat = griff.statistik.clone();
        assert!(
            warte_auf(4000, || stat.geschlossen_familie.load(Ordering::SeqCst)
                == 1),
            "ein P2 auf der Controlpipe muss die Verbindung schliessen"
        );
        assert_eq!(senke.p2.load(Ordering::SeqCst), 0);
    }

    #[test]
    fn unbekannter_schema_minor_wird_vor_der_senke_abgewiesen() {
        for familie in [Familie::P0, Familie::P1, Familie::P2] {
            assert!(schema_minor_bekannt(familie, 0));
            assert!(schema_minor_bekannt(familie, 1));
            assert!(!schema_minor_bekannt(familie, 2));
        }
    }

    /// T2-Befund 2: endet die Control-Verbindung, endet die Telemetrie mit —
    /// nicht nur ihr Registereintrag.
    #[test]
    fn control_ende_beendet_die_telemetrie() {
        let pipe = probe_pipe("kopplungsende");
        let senke = Arc::new(ZaehlSenke::default());
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"e".repeat(32))));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome");

        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&"e".repeat(32), &link, &challenge)));
        assert!(welcome_lesen(&tele).is_some());

        let frame = p2(&[7u8; 32]);
        assert!(tele.schreiben(&frame));
        assert!(warte_auf(3000, || senke.p2.load(Ordering::SeqCst) >= 1));

        // Nur die Control-Pipe schliessen.
        drop(steuer);
        assert!(
            warte_auf(4000, || senke.telemetrie_getrennt.load(Ordering::SeqCst) > 0),
            "das Abmelden muss den Telemetriearbeiter WIRKLICH beenden, nicht nur              seinen Registereintrag entfernen (control_getrennt={}, p2={})",
            senke.control_getrennt.load(Ordering::SeqCst),
            senke.p2.load(Ordering::SeqCst)
        );
        let _ = &griff;

        let vorher = senke.p2.load(Ordering::SeqCst);
        for _ in 0..20 {
            if !tele.schreiben(&frame) {
                break;
            }
            std::thread::sleep(Duration::from_millis(10));
        }
        std::thread::sleep(Duration::from_millis(300));
        assert_eq!(
            senke.p2.load(Ordering::SeqCst),
            vorher,
            "nach dem Ende der Control-Verbindung darf kein P2 mehr durchkommen"
        );
    }

    /// T2-Befund 6: die Verbindungsgrenze darf den Acceptor nicht toeten.
    #[test]
    fn acceptor_ueberlebt_die_verbindungsgrenze() {
        let pipe = probe_pipe("grenze");
        let senke = Arc::new(ZaehlSenke::default());
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        // Bis an die Grenze verbinden, ohne je ein Hello zu senden.
        let mut offen: Vec<Testclient> = Vec::new();
        for _ in 0..MAX_VERBINDUNGEN {
            // Kurze Geduld: an der Grenze horcht per Vertrag NIEMAND mehr, und
            // genau dieses Nein soll der Test schnell bekommen.
            match Testclient::mit_geduld(&pipe, 400) {
                Some(c) => offen.push(c),
                None => break,
            }
        }
        assert!(
            offen.len() >= MAX_VERBINDUNGEN - 1,
            "nur {} Verbindungen erreicht",
            offen.len()
        );

        // Die beiden Reservelistener liegen AUSSERHALB des Worker-Caps. Erst
        // der naechste Client beweist deshalb, dass der Acceptor die 96er-
        // Grenze selbst durchsetzt, ohne seinen Besitzlistener zu verlieren.
        let stat = griff.statistik.clone();
        assert!(
            warte_auf(8000, || griff.aktive_worker() == MAX_VERBINDUNGEN as u64),
            "der Worker-Cap wurde nicht vollstaendig hergestellt ({} Verbindungen offen)",
            offen.len()
        );
        let _ueber_cap = Testclient::mit_geduld(&pipe, 2000).expect("97. Client verbindet Reserve");
        assert!(
            warte_auf(4000, || stat
                .am_worker_cap_abgewiesen
                .load(Ordering::SeqCst)
                > 0),
            "der 97. Client wurde nicht sichtbar am Worker-Cap abgewiesen"
        );
        assert_eq!(griff.aktive_worker(), MAX_VERBINDUNGEN as u64);
        assert_eq!(stat.bewaffnete_listener.load(Ordering::SeqCst), 2);

        // Alles wieder loslassen — danach MUSS wieder jemand horchen.
        offen.clear();
        assert!(warte_auf(8000, || griff.aktive_worker() == 0));

        let neu = Testclient::neu(&pipe);
        assert!(
            neu.is_some(),
            "nach der Grenze horcht niemand mehr: der Acceptor hat sich beendet"
        );
        let neu = neu.unwrap();
        assert!(neu.schreiben(&control_hello(&"f".repeat(32))));
        assert!(
            welcome_lesen(&neu).is_some(),
            "die neue Verbindung muss ein welcome bekommen"
        );
        drop(griff);
    }

    /// T2-Befund 8: die Threadhandles duerfen nicht unbegrenzt wachsen.
    #[test]
    fn beendete_verbindungen_werden_geerntet() {
        let pipe = probe_pipe("ernte");
        let senke = Arc::new(ZaehlSenke::default());
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        for _ in 0..40 {
            let c = Testclient::neu(&pipe).expect("verbinden");
            assert!(c.schreiben(&control_hello(&"9".repeat(32))));
            assert!(welcome_lesen(&c).is_some());
            drop(c);
        }
        assert!(
            warte_auf(8000, || griff.gehaltene_verbindungen() <= 4),
            "nach 40 Zyklen haelt der Listener noch {} Threadhandles",
            griff.gehaltene_verbindungen()
        );
    }

    /// T2-Befund 6 vom 2026-08-29: `stoppen()` genau im Fenster zwischen
    /// Annahme der Verbindung und der ersten Arbeit ihres Threads.
    ///
    /// Die Testnaht `probe_verzoegerung_ms` haelt den Thread dort fest, statt
    /// das Fenster zu erwuerfeln. Gemessen wird dreierlei, und jede Zusicherung
    /// gehoert zu einem anderen Teil des Fixes:
    ///
    ///   1. das Handle steht im Register, BEVOR der Thread gearbeitet hat
    ///      (Registrierung im Acceptor);
    ///   2. der Thread endet AM STOP und nicht erst an einem abgebrochenen
    ///      Read (`geschlossen_bootstrap` bleibt 0);
    ///   3. `stoppen()` kehrt ueberhaupt zurueck (wiederholter Abbruch).
    #[test]
    fn stop_im_fenster_vor_der_bedienung_haengt_nicht() {
        let pipe = probe_pipe("stopfenster");
        let verzoegerung = Arc::new(AtomicU64::new(600));
        let senke = Arc::new(ZaehlSenke::default());
        let mut griff = v3_server_starten_intern(
            &pipe,
            senke,
            "test".into(),
            neue_kennung(),
            V3Sender::neu(),
            verzoegerung,
            Arc::new(AtomicBool::new(false)),
            Arc::new(AtomicU64::new(CANCEL_VOR_READ_INAKTIV)),
            false,
            V3SecurityTestOptionen::default(),
        )
        .unwrap();

        // Verbinden, aber KEIN Hello senden: ohne den Fix legte sich der Thread
        // gleich danach in einen Read, den nach dem Stop niemand mehr aufloest.
        let c = Testclient::neu(&pipe).unwrap();
        let stat = griff.statistik.clone();
        assert!(
            warte_auf(3000, || stat.angenommen.load(Ordering::SeqCst) >= 1),
            "der Acceptor muss die Verbindung angenommen haben"
        );

        // Erst MESSEN, dann urteilen: ohne den Fix haengt der Stop, und ein
        // Test, der vorher panickt, verwandelt sein Rot in einen Hang.
        let registriert = warte_auf(200, || griff.gehaltene_handles() >= 1);

        let (tx, rx) = std::sync::mpsc::channel();
        std::thread::spawn(move || {
            griff.stoppen();
            let _ = tx.send(());
        });
        let beendet = rx.recv_timeout(Duration::from_secs(10)).is_ok();
        let am_stop_geendet = stat.geschlossen_bootstrap.load(Ordering::SeqCst);
        drop(c);

        assert!(
            registriert,
            "das Handle muss VOR der ersten Arbeit des Threads im Register stehen"
        );
        assert!(
            beendet,
            "stoppen() darf im Fenster vor der Registrierung nicht haengen"
        );
        assert_eq!(
            am_stop_geendet, 0,
            "der Thread muss am Stop enden, nicht erst an einem abgebrochenen Read"
        );
    }

    /// T2-Befund 7 vom 2026-08-29: `stoppen()` endet binnen Frist, auch wenn
    /// die Senke WEITER blockiert. Der bestehende Blocktest gab vor dem Ende
    /// frei und prueft diesen Gegenpfad deshalb nicht.
    #[test]
    fn stoppen_endet_auch_bei_haengender_senke() {
        let pipe = probe_pipe("senkenhang");
        let senke = Arc::new(BlockSenke::default());
        senke.blockiert.store(true, Ordering::SeqCst);
        let mut griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"b".repeat(32))));
        assert!(welcome_lesen(&steuer).is_some());
        assert!(steuer.schreiben(&p1("{\"type\":\"state_report\"}")));
        assert!(
            warte_auf(5000, || senke.in_senke.load(Ordering::SeqCst)),
            "der Verbraucher muss WIRKLICH in der Senke stehen, sonst misst der Test nichts"
        );

        let stat = griff.statistik.clone();
        let (tx, rx) = std::sync::mpsc::channel();
        let t = std::thread::spawn(move || {
            griff.stoppen();
            let _ = tx.send(());
        });
        // Grosszuegig, aber ENDLICH: gemessen wird "endet", nicht "ist schnell".
        let rechtzeitig = rx
            .recv_timeout(SENKE_FRIST + Duration::from_secs(8))
            .is_ok();
        let abgeloest = stat.senke_abgeloest.load(Ordering::SeqCst);
        // Der SCHREIBER haengt nicht: ihn loest `CancelIoEx` sehr wohl. Die
        // Zahl trennt die beiden Faelle — sonst waere "abgeloest" ein
        // Sammelbegriff, der nichts mehr unterscheidet.
        let schreiber = stat.schreiber_abgeloest.load(Ordering::SeqCst);
        // Erst JETZT freigeben — vorher waere der Gegenpfad nicht gemessen.
        senke.blockiert.store(false, Ordering::SeqCst);
        t.join().unwrap();

        assert!(rechtzeitig, "stoppen() haengt im Senkenaufruf");
        assert!(
            abgeloest >= 1,
            "der haengende Verbraucher muss abgeloest und gezaehlt sein (war {abgeloest})"
        );
        assert_eq!(
            schreiber, 0,
            "nur die Senke haengt — der Schreiber wird abgebrochen, nicht abgeloest"
        );
    }

    /// T2-Befund 4 Runde 3: eine GESCHLOSSENE Ingressqueue liefert nichts
    /// mehr. Vorher pruefte `entnehmen` das Schliessflag erst NACH dem
    /// Inhalt — der Verbraucher lief nach dem Schliessen noch durch den
    /// Restbestand und rief P0/P1 fuer eine Verbindung, deren Kopplung schon
    /// abgemeldet war.
    #[test]
    fn geschlossener_eingang_liefert_nichts_mehr() {
        let e = Eingang::neu();
        assert!(matches!(
            e.einreihen(Familie::P0, 0, b"a".to_vec()),
            IngressErgebnis::Eingereiht
        ));
        assert!(matches!(
            e.einreihen(Familie::P1, 0, b"b".to_vec()),
            IngressErgebnis::Eingereiht
        ));
        assert_eq!(
            e.laenge(),
            2,
            "die Vorbedingung muss wirklich gefuellt sein"
        );

        e.schliessen();
        // BEIDE Sichten (`C-LS-07`) pruefen das Schliessflag vor dem Inhalt.
        assert!(
            e.entnehmen_p0().is_none() && e.entnehmen_ohne_p0().is_none(),
            "nach dem Schliessen darf kein Eintrag mehr kommen — er gehoert zu \
             einer Sitzung, die es nicht mehr gibt"
        );
    }

    /// Eine Senke, die im LEBENSZYKLUS blockiert statt in `p0`/`p1`/`p2` —
    /// genau die Luecke, die `stoppen_endet_auch_bei_haengender_senke`
    /// offenliess (T2-Befund 5 Runde 3).
    #[derive(Debug, Default)]
    struct LebenszyklusBlockSenke {
        zaehl: ZaehlSenke,
        blockiert: AtomicBool,
        in_senke: AtomicBool,
    }

    impl LebenszyklusBlockSenke {
        /// Wie `BlockSenke::warten`: endlich, damit ein rotes Ergebnis rot
        /// bleibt statt zum Hang zu werden.
        fn warten(&self) {
            self.in_senke.store(true, Ordering::SeqCst);
            let bis = Instant::now() + Duration::from_secs(20);
            while self.blockiert.load(Ordering::SeqCst) && Instant::now() < bis {
                std::thread::sleep(Duration::from_millis(5));
            }
            self.in_senke.store(false, Ordering::SeqCst);
        }
    }

    impl Senke for LebenszyklusBlockSenke {
        fn control_verbunden(&self, l: &str, h: &HelloControl) -> ControlAnmeldung {
            self.warten();
            self.zaehl.control_verbunden(l, h)
        }
        fn control_getrennt(&self, l: &str) {
            self.zaehl.control_getrennt(l);
        }
        fn telemetrie_gekoppelt(&self, l: &str) {
            self.zaehl.telemetrie_gekoppelt(l);
        }
        fn telemetrie_getrennt(&self, l: &str) {
            self.zaehl.telemetrie_getrennt(l);
        }
        fn p0(&self, l: &str, p: &[u8]) -> Option<Vec<u8>> {
            self.zaehl.p0(l, p)
        }
        fn p1(&self, l: &str, p: &[u8]) {
            self.zaehl.p1(l, p);
        }
        fn p2(&self, l: &str, p: &[u8]) {
            self.zaehl.p2(l, p);
        }
        fn abgewiesen(&self, g: &str) {
            self.zaehl.abgewiesen(g);
        }
    }

    /// T2-Befund 5 Runde 3: `stoppen()` endet auch dann, wenn die Senke im
    /// LEBENSZYKLUSaufruf haengt. Der bestehende Haengertest deckt nur
    /// `p0`/`p1`/`p2` — die laufen auf dem Ingressthread, der schon eine Frist
    /// hatte. `control_verbunden` lief unbegrenzt auf dem Verbindungsthread,
    /// und genau auf den wartet `stoppen()`.
    #[test]
    fn stoppen_endet_auch_bei_haengendem_lebenszyklusaufruf() {
        let pipe = probe_pipe("lebenszyklushang");
        let senke = Arc::new(LebenszyklusBlockSenke::default());
        senke.blockiert.store(true, Ordering::SeqCst);
        let mut griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"c".repeat(32))));
        assert!(
            warte_auf(5000, || senke.in_senke.load(Ordering::SeqCst)),
            "die Senke muss WIRKLICH im control_verbunden stehen, sonst misst \
             der Test nichts"
        );

        let stat = griff.statistik.clone();
        let (tx, rx) = std::sync::mpsc::channel();
        let t = std::thread::spawn(move || {
            griff.stoppen();
            let _ = tx.send(());
        });
        // Grosszuegig, aber ENDLICH: gemessen wird "endet", nicht "ist schnell".
        let rechtzeitig = rx
            .recv_timeout(SENKE_FRIST + Duration::from_secs(8))
            .is_ok();
        let abgeloest = stat.lebenszyklus_abgeloest.load(Ordering::SeqCst);
        let verbraucher = stat.senke_abgeloest.load(Ordering::SeqCst);
        // Erst JETZT freigeben — vorher waere der Gegenpfad nicht gemessen.
        senke.blockiert.store(false, Ordering::SeqCst);
        t.join().unwrap();
        drop(steuer);

        assert!(
            rechtzeitig,
            "stoppen() haengt im Lebenszyklusaufruf der Senke"
        );
        assert!(
            abgeloest >= 1,
            "der haengende Lebenszyklusaufruf muss abgeloest und gezaehlt sein \
             (war {abgeloest})"
        );
        assert_eq!(
            verbraucher, 0,
            "nur der Lebenszyklusaufruf haengt — der Verbraucherthread nicht"
        );
    }

    /// T2-Befund 3 Runde 3: die Kopplung faellt mit dem LESERENDE, nicht erst
    /// nach den fristbegrenzten Joins. Gemessen an der Telemetriepipe: sie
    /// muss lange vor `SENKE_FRIST` zu sein, obwohl der Verbraucher der
    /// Control-Verbindung noch in der Senke steht.
    #[test]
    fn kopplung_faellt_mit_dem_leserende_nicht_erst_nach_den_joins() {
        let pipe = probe_pipe("kopplungsofort");
        let senke = Arc::new(BlockSenke::default());
        let mut griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let nonce = "d".repeat(32);
        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&nonce)));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome der Control-Verbindung");
        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&nonce, &link, &challenge)));
        assert!(
            welcome_lesen(&tele).is_some(),
            "die Telemetrieverbindung muss gekoppelt sein, sonst misst der Test nichts"
        );

        // Der Verbraucher der CONTROL-Verbindung soll in der Senke stehen,
        // wenn ihr Leser endet — sonst laufen die Joins durch und der Test
        // spraeche ueber nichts.
        senke.blockiert.store(true, Ordering::SeqCst);
        assert!(steuer.schreiben(&p1("{\"type\":\"state_report\"}")));
        assert!(
            warte_auf(5000, || senke.in_senke.load(Ordering::SeqCst)),
            "der Verbraucher muss WIRKLICH in der Senke stehen"
        );

        // Ein Leser auf der Telemetriepipe: er kehrt zurueck, sobald der
        // Broker sie schliesst.
        let (tx, rx) = std::sync::mpsc::channel();
        let leser = std::thread::spawn(move || {
            let mut z = [0u8; 64];
            let _ = tele.lesen(&mut z);
            let _ = tx.send(());
            tele
        });

        let beginn = Instant::now();
        drop(steuer); // Control-Pipe zu ⇒ Leserende ⇒ Abbau
        let zu = rx.recv_timeout(SENKE_FRIST * 4).is_ok();
        let dauer = beginn.elapsed();
        senke.blockiert.store(false, Ordering::SeqCst);
        let _ = leser.join();
        griff.stoppen();

        assert!(
            zu,
            "die Telemetrieverbindung muss mit ihrer Control-Verbindung fallen"
        );
        assert!(
            dauer < SENKE_FRIST / 2,
            "sie faellt erst nach {dauer:?} — die Kopplung haengt an den Joins \
             statt am Leserende (SENKE_FRIST {SENKE_FRIST:?})"
        );
    }

    //== Ursachenrunde 2026-08-30: die Zeilen der Verhaltensmatrix ============

    /// Protokolliert die Lebenszyklus-Callbacks in ihrer Reihenfolge und kann
    /// in genau einem von ihnen blockieren. Ohne Protokoll misst ein Test
    /// ueber Reihenfolge nur seine eigene Hoffnung.
    #[derive(Default)]
    struct ReihenfolgeSenke {
        zaehl: ZaehlSenke,
        log: Mutex<Vec<String>>,
        /// Was BETRETEN wurde — vor dem Blockieren. `log` traegt erst den
        /// Austritt; ohne diese zweite Spur koennte ein Test nicht abwarten,
        /// dass die Gegenseite wirklich IN ihrem Callback steht, und muesste
        /// die Taktung raten.
        betreten: Mutex<Vec<String>>,
        blockiert_in: Mutex<String>,
        blockdauer_ms: AtomicU64,
    }

    impl ReihenfolgeSenke {
        fn notieren(&self, was: &str) {
            self.betreten
                .lock()
                .unwrap_or_else(|e| e.into_inner())
                .push(was.to_string());
            let blockieren = {
                let b = self.blockiert_in.lock().unwrap_or_else(|e| e.into_inner());
                *b == was
            };
            if blockieren {
                let ms = self.blockdauer_ms.load(Ordering::SeqCst);
                std::thread::sleep(Duration::from_millis(ms));
            }
            self.log
                .lock()
                .unwrap_or_else(|e| e.into_inner())
                .push(was.to_string());
        }
        fn eintraege(&self) -> Vec<String> {
            self.log.lock().unwrap_or_else(|e| e.into_inner()).clone()
        }
        fn anzahl(&self, was: &str) -> usize {
            self.eintraege()
                .iter()
                .filter(|x| x.as_str() == was)
                .count()
        }
        fn stelle(&self, was: &str) -> Option<usize> {
            self.eintraege().iter().position(|x| x.as_str() == was)
        }
        fn betreten_anzahl(&self, was: &str) -> usize {
            self.betreten
                .lock()
                .unwrap_or_else(|e| e.into_inner())
                .iter()
                .filter(|x| x.as_str() == was)
                .count()
        }
    }

    impl Senke for ReihenfolgeSenke {
        fn control_verbunden(&self, l: &str, h: &HelloControl) -> ControlAnmeldung {
            let ausgang = self.zaehl.control_verbunden(l, h);
            self.notieren("control_verbunden");
            ausgang
        }
        fn control_getrennt(&self, l: &str) {
            self.zaehl.control_getrennt(l);
            self.notieren("control_getrennt");
        }
        fn telemetrie_gekoppelt(&self, l: &str) {
            self.zaehl.telemetrie_gekoppelt(l);
            self.notieren("telemetrie_gekoppelt");
        }
        fn telemetrie_getrennt(&self, l: &str) {
            self.zaehl.telemetrie_getrennt(l);
            self.notieren("telemetrie_getrennt");
        }
        fn p0(&self, l: &str, p: &[u8]) -> Option<Vec<u8>> {
            self.zaehl.p0(l, p)
        }
        fn p1(&self, l: &str, p: &[u8]) {
            self.zaehl.p1(l, p);
        }
        fn p2(&self, l: &str, p: &[u8]) {
            self.zaehl.p2(l, p);
        }
        fn abgewiesen(&self, g: &str) {
            self.zaehl.abgewiesen(g);
        }
    }

    /// Blockiert AUSSCHLIESSLICH in `p1`. Der Gegenspieler fuer `C-LS-07`:
    /// eine Senke, die ueberall blockiert, kann nicht zeigen, dass P0
    /// waehrenddessen beantwortet wird.
    #[derive(Default)]
    struct P1BlockSenke {
        zaehl: ZaehlSenke,
        blockiert: AtomicBool,
        in_p1: AtomicBool,
    }

    impl Senke for P1BlockSenke {
        fn control_verbunden(&self, l: &str, h: &HelloControl) -> ControlAnmeldung {
            self.zaehl.control_verbunden(l, h)
        }
        fn control_getrennt(&self, l: &str) {
            self.zaehl.control_getrennt(l);
        }
        fn telemetrie_gekoppelt(&self, l: &str) {
            self.zaehl.telemetrie_gekoppelt(l);
        }
        fn telemetrie_getrennt(&self, l: &str) {
            self.zaehl.telemetrie_getrennt(l);
        }
        fn p0(&self, l: &str, p: &[u8]) -> Option<Vec<u8>> {
            self.zaehl.p0(l, p)
        }
        fn p1(&self, l: &str, p: &[u8]) {
            self.in_p1.store(true, Ordering::SeqCst);
            let bis = Instant::now() + Duration::from_secs(20);
            while self.blockiert.load(Ordering::SeqCst) && Instant::now() < bis {
                std::thread::sleep(Duration::from_millis(5));
            }
            self.in_p1.store(false, Ordering::SeqCst);
            self.zaehl.p1(l, p);
        }
        fn p2(&self, l: &str, p: &[u8]) {
            self.zaehl.p2(l, p);
        }
        fn abgewiesen(&self, g: &str) {
            self.zaehl.abgewiesen(g);
        }
    }

    /// Liest EINEN v3-gerahmten Frame und gibt seinen Payload als JSON.
    fn frame_json_lesen(c: &Testclient) -> Option<serde_json::Value> {
        let mut puffer = [0u8; 4096];
        let mut roh: Vec<u8> = Vec::new();
        for _ in 0..50 {
            let n = c.lesen(&mut puffer);
            if n == 0 {
                return None;
            }
            roh.extend_from_slice(&puffer[..n]);
            if let Ok(r) = crate::transport::v3::envelope_pruefen(&roh) {
                return serde_json::from_slice(&r.payload).ok();
            }
        }
        None
    }

    fn frame_roh_lesen(c: &Testclient) -> Option<crate::transport::v3::Rahmen> {
        let mut puffer = [0u8; 65_536];
        let mut roh: Vec<u8> = Vec::new();
        for _ in 0..50 {
            let n = c.lesen(&mut puffer);
            if n == 0 {
                return None;
            }
            roh.extend_from_slice(&puffer[..n]);
            if let Ok(rahmen) = crate::transport::v3::envelope_pruefen(&roh) {
                return Some(rahmen);
            }
        }
        None
    }

    /// Matrix `C-LS-02`/`C-LS-04` (Regel 5): `control_verbunden` ist
    /// ABGESCHLOSSEN, bevor das Welcome den Draht verlaesst — sonst kann
    /// `telemetrie_gekoppelt` auf dem anderen Verbindungsthread vorlaufen.
    #[test]
    fn welcome_folgt_dem_abgeschlossenen_control_verbunden() {
        let pipe = probe_pipe("cvvorwelcome");
        let senke = Arc::new(ReihenfolgeSenke::default());
        *senke.blockiert_in.lock().unwrap() = "control_verbunden".into();
        senke.blockdauer_ms.store(400, Ordering::SeqCst);
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        let t0 = Instant::now();
        assert!(steuer.schreiben(&control_hello(&"c".repeat(32))));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
        let dauer = t0.elapsed();
        assert!(
            dauer >= Duration::from_millis(350),
            "das Welcome kam schon nach {dauer:?} — also VOR dem abgeschlossenen \
             control_verbunden (die Senke haelt es 400 ms)"
        );
        assert_eq!(
            senke.eintraege().first().map(String::as_str),
            Some("control_verbunden"),
            "erster Lebenszyklusaufruf muss control_verbunden sein"
        );

        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&"c".repeat(32), &link, &challenge)));
        assert!(welcome_lesen(&tele).is_some());
        assert!(warte_auf(4000, || senke.anzahl("telemetrie_gekoppelt") == 1));
        assert!(
            senke.stelle("control_verbunden") < senke.stelle("telemetrie_gekoppelt"),
            "telemetrie_gekoppelt lief vor control_verbunden: {:?}",
            senke.eintraege()
        );
        drop(steuer);
        drop(tele);
        drop(griff);
    }

    /// Matrix `C-LS-02`/`C-LS-04`: je `link_id` genau ein
    /// `control_verbunden` und hoechstens ein `telemetrie_gekoppelt`. Ein
    /// zweites Telemetry-Hello auf dieselbe Kopplung wird abgewiesen, OHNE den
    /// Callback erneut auszuloesen.
    #[test]
    fn connect_callbacks_je_kopplung_genau_einmal() {
        let pipe = probe_pipe("connecteinmal");
        let senke = Arc::new(ReihenfolgeSenke::default());
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"d".repeat(32))));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&"d".repeat(32), &link, &challenge)));
        assert!(welcome_lesen(&tele).is_some());
        assert!(warte_auf(4000, || senke.anzahl("telemetrie_gekoppelt") == 1));

        // Zweite Telemetrieverbindung auf dieselbe link_id: abgewiesen.
        let zweite = Testclient::neu(&pipe).unwrap();
        assert!(zweite.schreiben(&telemetry_hello(&"d".repeat(32), &link, &challenge)));
        assert!(warte_auf(4000, || griff
            .statistik
            .geschlossen_bootstrap
            .load(Ordering::SeqCst)
            > 0));
        std::thread::sleep(Duration::from_millis(200));
        assert_eq!(senke.anzahl("control_verbunden"), 1);
        assert_eq!(
            senke.anzahl("telemetrie_gekoppelt"),
            1,
            "die abgewiesene zweite Verbindung darf den Callback nicht erneut ausloesen: {:?}",
            senke.eintraege()
        );
        drop(steuer);
        drop(tele);
        drop(zweite);
        drop(griff);
    }

    /// Matrix `C-LS-06` (Regel 5), Normalfall: Kopplung loesen →
    /// `telemetrie_getrennt` → `control_getrennt`, je genau einmal.
    #[test]
    fn trennreihenfolge_je_callback_genau_einmal() {
        let pipe = probe_pipe("trennreihe");
        let senke = Arc::new(ReihenfolgeSenke::default());
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"e".repeat(32))));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&"e".repeat(32), &link, &challenge)));
        assert!(welcome_lesen(&tele).is_some());
        assert!(warte_auf(4000, || senke.anzahl("telemetrie_gekoppelt") == 1));

        drop(steuer); // Control-Ende reisst die Telemetrie mit
        assert!(
            warte_auf(8000, || senke.anzahl("control_getrennt") == 1),
            "control_getrennt fehlt: {:?}",
            senke.eintraege()
        );
        std::thread::sleep(Duration::from_millis(300));
        let log = senke.eintraege();
        assert_eq!(senke.anzahl("telemetrie_getrennt"), 1, "{log:?}");
        assert_eq!(senke.anzahl("control_getrennt"), 1, "{log:?}");
        assert!(
            senke.stelle("telemetrie_getrennt") < senke.stelle("control_getrennt"),
            "control_getrennt kam vor telemetrie_getrennt: {log:?}"
        );
        drop(tele);
        drop(griff);
    }

    /// Matrix `C-LS-06`, Fristfall (Nachtrag der Wiederpruefung): ein
    /// abgeloestes `telemetrie_getrennt` haelt `control_getrennt` NICHT auf.
    /// Ein abgeloester Trenn-Callback zaehlt als gelaufen; es entfaellt keiner
    /// und keiner laeuft doppelt.
    #[test]
    fn abgeloestes_telemetrie_getrennt_haelt_control_getrennt_nicht_auf() {
        let pipe = probe_pipe("trennfrist");
        let senke = Arc::new(ReihenfolgeSenke::default());
        *senke.blockiert_in.lock().unwrap() = "telemetrie_getrennt".into();
        senke
            .blockdauer_ms
            .store(SENKE_FRIST.as_millis() as u64 * 2, Ordering::SeqCst);
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"f".repeat(32))));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&"f".repeat(32), &link, &challenge)));
        assert!(welcome_lesen(&tele).is_some());
        assert!(warte_auf(4000, || senke.anzahl("telemetrie_gekoppelt") == 1));

        let beginn = Instant::now();
        drop(steuer);
        assert!(
            warte_auf(SENKE_FRIST.as_millis() as u64 * 4, || senke
                .anzahl("control_getrennt")
                == 1),
            "control_getrennt blieb aus, obwohl nur telemetrie_getrennt haengt: {:?}",
            senke.eintraege()
        );
        let dauer = beginn.elapsed();
        assert!(
            dauer < SENKE_FRIST * 3,
            "control_getrennt kam erst nach {dauer:?} — es haengt an der vollen \
             Blockdauer statt an SENKE_FRIST ({SENKE_FRIST:?})"
        );
        assert!(
            warte_auf(SENKE_FRIST.as_millis() as u64 * 3, || griff
                .statistik
                .lebenszyklus_abgeloest
                .load(Ordering::SeqCst)
                >= 1),
            "der Fristfall muss als lebenszyklus_abgeloest sichtbar sein"
        );
        assert_eq!(senke.anzahl("control_getrennt"), 1, "nie doppelt");
        drop(tele);
        drop(griff);
    }

    /// Matrix `C-LS-06` (NAK-104): die TELEMETRIE endet ZUERST, ihr
    /// Trenn-Callback laeuft noch fristgerecht — und trotzdem meldet die
    /// Control-Seite ihr `control_getrennt` erst danach.
    ///
    /// Alle vorhandenen Proben beenden Control zuerst. Genau deshalb fiel nicht
    /// auf, dass der Wartepunkt am Kopplungsregister haengt: die
    /// Telemetrieverbindung nimmt ihren Eintrag schon VOR den Joins heraus,
    /// also bevor sie ihr `telemetrie_getrennt` ueberhaupt meldet. Die
    /// Control-Seite sah danach "keine Telemetrie" und uebersprang den
    /// Wartepunkt.
    #[test]
    fn telemetrie_endet_zuerst_control_getrennt_folgt_trotzdem() {
        let pipe = probe_pipe("trenntelezuerst");
        let senke = Arc::new(ReihenfolgeSenke::default());
        *senke.blockiert_in.lock().unwrap() = "telemetrie_getrennt".into();
        // KUERZER als SENKE_FRIST: das hier ist der fristgerechte Fall, nicht
        // der Abloesefall (`abgeloestes_telemetrie_getrennt_...`).
        senke.blockdauer_ms.store(800, Ordering::SeqCst);
        assert!(
            Duration::from_millis(800) < SENKE_FRIST,
            "die Blockdauer muss unter SENKE_FRIST liegen, sonst misst der Test den Fristfall"
        );
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"1".repeat(32))));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&"1".repeat(32), &link, &challenge)));
        assert!(welcome_lesen(&tele).is_some());
        assert!(warte_auf(4000, || senke.anzahl("telemetrie_gekoppelt") == 1));

        // 1) Die Telemetrie geht zuerst und steht danach in ihrem
        //    Trenn-Callback. Die Vorbedingung wird gemessen, nicht gehofft.
        drop(tele);
        assert!(
            warte_auf(4000, || senke.betreten_anzahl("telemetrie_getrennt") == 1),
            "die Telemetrieseite muss wirklich in ihrem Trenn-Callback stehen: {:?}",
            senke.eintraege()
        );

        // 2) Erst JETZT endet Control — waehrend drueben der Callback laeuft.
        drop(steuer);
        assert!(
            warte_auf(8000, || senke.anzahl("control_getrennt") == 1),
            "control_getrennt fehlt: {:?}",
            senke.eintraege()
        );
        std::thread::sleep(Duration::from_millis(300));
        let log = senke.eintraege();
        // Erst die REIHENFOLGE, dann die Zahlen — und ausgepackt verglichen:
        // `Option::cmp` haelt `None` fuer kleiner als jedes `Some`, ein ganz
        // fehlendes `telemetrie_getrennt` saehe im blossen Vergleich also wie
        // die richtige Reihenfolge aus.
        let stelle_tele = senke.stelle("telemetrie_getrennt");
        let stelle_ctrl = senke.stelle("control_getrennt");
        assert!(
            matches!((stelle_tele, stelle_ctrl), (Some(t), Some(c)) if t < c),
            "control_getrennt lief vor telemetrie_getrennt (oder telemetrie_getrennt \
             fehlt ganz), obwohl die Telemetrie ihre Frist hielt: {log:?}"
        );
        assert_eq!(senke.anzahl("telemetrie_getrennt"), 1, "{log:?}");
        assert_eq!(senke.anzahl("control_getrennt"), 1, "{log:?}");
        assert_eq!(
            griff
                .statistik
                .lebenszyklus_reihenfolge_verletzt
                .load(Ordering::SeqCst),
            0,
            "der fristgerechte Fall darf keine Reihenfolgeverletzung zaehlen"
        );
        drop(griff);
    }

    /// Matrix `C-LS-06` (NAK-104 Runde 2): das FENSTER des Kopplungs-Callbacks.
    ///
    /// Die Telemetrie ist gekoppelt und ihr Welcome ist auf dem Draht, aber
    /// `telemetrie_gekoppelt` steht noch bei der Senke. Enden beide Clients
    /// genau jetzt, muss die Reihenfolge trotzdem halten. Vorher wurde die
    /// Trennpflicht erst NACH Rueckkehr des Callbacks gesetzt: die
    /// Control-Seite fand `erwartet == false`, uebersprang den Wartepunkt und
    /// meldete `control_getrennt` VOR dem `telemetrie_getrennt`, das kurz
    /// danach folgte.
    ///
    /// Der Vorgaengertest `telemetrie_endet_zuerst_...` blockiert erst im
    /// TRENN-Callback und wartet bis `telemetrie_gekoppelt` durch ist — er
    /// betritt dieses Fenster nie.
    #[test]
    fn trennreihenfolge_haelt_im_kopplungs_callback_fenster() {
        let pipe = probe_pipe("trennkoppelfenster");
        let senke = Arc::new(ReihenfolgeSenke::default());
        *senke.blockiert_in.lock().unwrap() = "telemetrie_gekoppelt".into();
        // FRISTGERECHT: unter `SENKE_FRIST`, sonst misst der Test den
        // Abloesefall statt des Fensters.
        senke.blockdauer_ms.store(800, Ordering::SeqCst);
        assert!(
            Duration::from_millis(800) < SENKE_FRIST,
            "die Blockdauer muss unter SENKE_FRIST liegen, sonst misst der Test den Fristfall"
        );
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"2".repeat(32))));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&"2".repeat(32), &link, &challenge)));
        // Das Welcome der Telemetrie geht VOR `telemetrie_gekoppelt` raus —
        // der Client ist also schon bedient, waehrend die Senke noch haelt.
        assert!(welcome_lesen(&tele).is_some());

        // Vorbedingung messen, nicht hoffen: die Senke steht IM
        // Kopplungs-Callback (`betreten` traegt den Eintritt, `log` erst den
        // Austritt).
        assert!(
            warte_auf(4000, || senke.betreten_anzahl("telemetrie_gekoppelt") == 1),
            "die Senke muss wirklich in telemetrie_gekoppelt stehen: {:?}",
            senke.eintraege()
        );
        assert_eq!(
            senke.anzahl("telemetrie_gekoppelt"),
            0,
            "der Callback darf hier noch NICHT zurueck sein — sonst ist das Fenster \
             schon vorbei und der Test misst nichts"
        );

        // Beide Clients enden IM Fenster: erst die Telemetrie, dann Control.
        drop(tele);
        drop(steuer);
        let beginn = Instant::now();
        assert!(
            warte_auf(8000, || senke.anzahl("control_getrennt") == 1),
            "control_getrennt fehlt: {:?}",
            senke.eintraege()
        );
        let dauer = beginn.elapsed();
        std::thread::sleep(Duration::from_millis(300));
        let log = senke.eintraege();
        // Ausgepackt vergleichen: `Option::cmp` haelt `None` fuer kleiner als
        // jedes `Some`, ein ganz fehlendes `telemetrie_getrennt` saehe sonst
        // wie die richtige Reihenfolge aus.
        let stelle_tele = senke.stelle("telemetrie_getrennt");
        let stelle_ctrl = senke.stelle("control_getrennt");
        assert!(
            matches!((stelle_tele, stelle_ctrl), (Some(t), Some(c)) if t < c),
            "control_getrennt lief vor telemetrie_getrennt (oder telemetrie_getrennt \
             fehlt ganz), obwohl beide Clients im Kopplungs-Callback endeten: {log:?}"
        );
        assert_eq!(senke.anzahl("telemetrie_getrennt"), 1, "{log:?}");
        assert_eq!(senke.anzahl("control_getrennt"), 1, "{log:?}");
        // Das `telemetrie_getrennt` kann fruehestens nach dem 800-ms-Block
        // laufen. Kommt `control_getrennt` schon vorher, hat die Control-Seite
        // den Wartepunkt uebersprungen — genau der Defekt.
        assert!(
            dauer >= Duration::from_millis(500),
            "control_getrennt kam schon nach {dauer:?} — die Control-Seite hat den \
             Wartepunkt uebersprungen, statt das telemetrie_getrennt nach dem \
             800-ms-Block abzuwarten: {log:?}"
        );
        assert_eq!(
            griff
                .statistik
                .lebenszyklus_reihenfolge_verletzt
                .load(Ordering::SeqCst),
            0,
            "der fristgerechte Fall darf keine Reihenfolgeverletzung zaehlen"
        );
        drop(griff);
    }

    /// Gegenstueck zum Fenster (`C-LS-04`/`C-LS-06`, NAK-104 Runde 2): wird das
    /// `telemetrie_gekoppelt` ABGELOEST, kommt kein `telemetrie_getrennt` — die
    /// vorgezogene Trennpflicht muss dann zurueckfallen. Sonst haengt die
    /// Control-Seite die volle `SENKE_FRIST` und zaehlt eine
    /// Reihenfolgeverletzung, die keine ist.
    #[test]
    fn abgeloestes_telemetrie_gekoppelt_laesst_control_getrennt_nicht_warten() {
        let pipe = probe_pipe("koppelabloese");
        let senke = Arc::new(ReihenfolgeSenke::default());
        *senke.blockiert_in.lock().unwrap() = "telemetrie_gekoppelt".into();
        senke
            .blockdauer_ms
            .store(SENKE_FRIST.as_millis() as u64 * 2, Ordering::SeqCst);
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"3".repeat(32))));
        let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
        let tele = Testclient::neu(&pipe).unwrap();
        assert!(tele.schreiben(&telemetry_hello(&"3".repeat(32), &link, &challenge)));
        assert!(welcome_lesen(&tele).is_some());
        assert!(
            warte_auf(SENKE_FRIST.as_millis() as u64 * 3, || griff
                .statistik
                .lebenszyklus_abgeloest
                .load(Ordering::SeqCst)
                >= 1),
            "das telemetrie_gekoppelt muss wirklich abgeloest worden sein"
        );

        let beginn = Instant::now();
        drop(steuer);
        assert!(
            warte_auf(SENKE_FRIST.as_millis() as u64 * 4, || senke
                .anzahl("control_getrennt")
                == 1),
            "control_getrennt fehlt: {:?}",
            senke.eintraege()
        );
        let dauer = beginn.elapsed();
        assert!(
            dauer < SENKE_FRIST / 2,
            "control_getrennt kam erst nach {dauer:?} — es wartet auf ein \
             telemetrie_getrennt, das nach der Abloesung nie kommt (SENKE_FRIST \
             {SENKE_FRIST:?})"
        );
        assert_eq!(
            griff
                .statistik
                .lebenszyklus_reihenfolge_verletzt
                .load(Ordering::SeqCst),
            0,
            "ein abgesagtes Gegenstueck ist keine Reihenfolgeverletzung"
        );
        drop(tele);
        drop(griff);
    }

    /// Matrix `C-LS-07` (Regel 4 auf der Rust-Seite): blockiert die Senke in
    /// `p1`, wird ein P0-Frame trotzdem gelesen UND BEANTWORTET. Zweiter Fall:
    /// ist der Ingress voll und liegt kein P2 darin, greift `A-IN-03`/
    /// `A-IN-04` — dann endet die Verbindung sichtbar, statt still zu hungern.
    #[test]
    fn p0_wird_beantwortet_waehrend_p1_die_senke_blockiert() {
        let pipe = probe_pipe("p0trotzp1");
        let senke = Arc::new(P1BlockSenke::default());
        senke.blockiert.store(true, Ordering::SeqCst);
        let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

        let steuer = Testclient::neu(&pipe).unwrap();
        assert!(steuer.schreiben(&control_hello(&"9".repeat(32))));
        assert!(welcome_lesen(&steuer).is_some());

        // Die Senke steht in `p1` — die Vorbedingung wird gemessen, nicht
        // gehofft.
        assert!(steuer.schreiben(&p1("{\"type\":\"state_report\"}")));
        assert!(
            warte_auf(4000, || senke.in_p1.load(Ordering::SeqCst)),
            "die Senke muss wirklich in p1 stehen"
        );

        // Fall 1: P0 kommt durch und wird beantwortet.
        assert!(steuer.schreiben(&p0("{\"type\":\"heartbeat\",\"sequence\":7}")));
        let antwort = frame_json_lesen(&steuer).expect("kein heartbeat_ack — P0 hungert");
        assert_eq!(
            antwort.get("type").and_then(|v| v.as_str()),
            Some("heartbeat_ack")
        );
        assert_eq!(antwort.get("sequence").and_then(|v| v.as_u64()), Some(7));
        assert!(
            senke.in_p1.load(Ordering::SeqCst),
            "die Senke muss beim Eintreffen der Antwort NOCH in p1 stehen"
        );

        // Fall 2: der Ingress laeuft ohne P2 ueber — Trennen, nicht hungern.
        let eins = p1("{\"type\":\"state_report\"}");
        for _ in 0..(CAP_INGRESS + 60) {
            if !steuer.schreiben(&eins) {
                break;
            }
        }
        let stat = griff.statistik.clone();
        assert!(
            warte_auf(8000, || stat
                .ingress_p1_ueberlauf_trennt
                .load(Ordering::SeqCst)
                > 0),
            "der volle Ingress ohne P2 muss nach A-IN-04 trennen"
        );
        senke.blockiert.store(false, Ordering::SeqCst);
        drop(steuer);
        drop(griff);
    }

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
        assert_eq!(definiert, 18);
        assert_eq!(spaeter, 9);
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
        assert_eq!(main_welcome.kopf.schema_minor, 1);
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
        assert_eq!(snapshot.kopf.schema_minor, 1);
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
        assert!(source_telemetrie
            .schreiben(&envelope_schreiben(Familie::P2, P2_SCHEMA_MINOR, payload).unwrap()));
        let weiter = frame_roh_lesen(&main_telemetrie).expect("P2-Push an Main");
        assert_eq!(weiter.kopf.familie, Familie::P2);
        assert_eq!(weiter.kopf.schema_minor, P2_SCHEMA_MINOR);
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
}
