//! SONDE-010 — v3-Listener und I/O-Worker.
//!
//! Der Worker entscheidet GENAU drei Dinge (Entwurf §53.9): Envelope,
//! Grenzen und Authentisierung. Alles Weitere geht als typisiertes Ereignis
//! an eine schmale `Senke`. Session, Eviction, Store und Outbox — also der
//! `Coordinator` — sind SONDE-011 und stehen hier bewusst nicht.
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
//! ── Warum dieser Listener heute nur ueber einen PROBE-Namen laeuft ─────────
//!
//! Der Broker oeffnet in Produktion weiterhin ausschliesslich die v2-Pipe.
//! Einen SID-gebundenen v3-Endpunkt zusaetzlich zu oeffnen, waere heute ein
//! Endpunkt, der Verbindungen annimmt und danach nichts damit anfangen kann:
//! ohne Coordinator gibt es keine Session, kein Register und keinen Store.
//! Das waere ein totes Element im Sinne des Grundgesetzes. Die Funktion
//! `pipetoken::pipe_name_v3` liegt fertig und mit Golden bereit; der
//! Produktivbetrieb schaltet sie in SONDE-011 zusammen mit dem Coordinator
//! ein. Bis dahin faehrt der Listener ueber den Probe-Namen — nie ueber die
//! Produktions-Pipe (CLAUDE.md, "Bauen und beweisen").
//!
//! Sicherheit ist trotzdem nicht Probe-Qualitaet: derselbe SDDL-Deskriptor
//! ("nur der aktuelle User"), `PIPE_REJECT_REMOTE_CLIENTS` und
//! `FILE_FLAG_FIRST_PIPE_INSTANCE` wie beim v2-Server — die Helfer kommen aus
//! `server.rs`, damit es nur EINE Wahrheit ueber die Pipe-Sicherheit gibt.

use std::collections::VecDeque;
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::{Arc, Condvar, Mutex};
use std::thread::JoinHandle;
use std::time::{Duration, Instant};

use windows_sys::Win32::Foundation::{
    CloseHandle, GetLastError, ERROR_BROKEN_PIPE, ERROR_IO_PENDING, ERROR_NO_DATA,
    ERROR_OPERATION_ABORTED, ERROR_PIPE_BUSY, ERROR_PIPE_CONNECTED, ERROR_PIPE_NOT_CONNECTED,
    HANDLE, INVALID_HANDLE_VALUE, WAIT_OBJECT_0,
};
use windows_sys::Win32::Security::SECURITY_ATTRIBUTES;
use windows_sys::Win32::Storage::FileSystem::{
    ReadFile, WriteFile, FILE_FLAG_FIRST_PIPE_INSTANCE, FILE_FLAG_OVERLAPPED, PIPE_ACCESS_DUPLEX,
};
use windows_sys::Win32::System::Pipes::{
    ConnectNamedPipe, CreateNamedPipeW, DisconnectNamedPipe, PIPE_READMODE_BYTE,
    PIPE_REJECT_REMOTE_CLIENTS, PIPE_TYPE_BYTE, PIPE_WAIT,
};
use windows_sys::Win32::System::Threading::{CreateEventW, ResetEvent, WaitForSingleObject, INFINITE};
use windows_sys::Win32::System::IO::{CancelIoEx, GetOverlappedResult, OVERLAPPED};

use crate::transport::bootstrap::{
    bootstrap_lesen, neue_kennung, Bootstrap, BootstrapFehler, HelloControl, Kopplungen, Welcome,
};
use crate::transport::v3::{
    envelope_schreiben, Familie, LeseErgebnis, Ratengrenze, StromLeser, MAX_FRAME_BYTES,
};
use crate::transport::warteschlange::{IngressErgebnis, IngressWarteschlange};

/// Hoechstens so viele gleichzeitige Verbindungen. Zwei je Instanz (Control +
/// Telemetry) mal 32 Sonden plus Reserve.
pub const MAX_VERBINDUNGEN: usize = 96;

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

/// Was der I/O-Worker nach oben gibt. Bewusst byteorientiert: die Bedeutung
/// des Payloads kennt erst der Coordinator.
/// Frist, die der Verbindungsschluss einem LAUFENDEN Senkenaufruf noch
/// laesst. Danach wird der Verbraucherthread abgeloest statt gejoint.
pub const SENKE_FRIST: Duration = Duration::from_millis(2000);

/// Wohin der Listener seine Nachrichten uebergibt.
///
/// **Senkenvertrag.** `p0`, `p1` und `p2` laufen auf dem Ingressthread der
/// Verbindung und muessen ZUEGIG zurueckkehren. Blockieren duerfen sie, aber
/// nicht unbegrenzt: beim Verbindungsschluss wartet der Broker hoechstens
/// `SENKE_FRIST` auf einen laufenden Aufruf und LOEST den Verbraucherthread
/// danach AB, statt ihn zu joinen (`V3Statistik::senke_abgeloest`). Vorher
/// wartete `stoppen()` unbegrenzt im Fremdaufruf: weder das Schliessen der
/// Queue noch `CancelIoEx` loesen einen Thread, der in fremdem Code steht
/// (T2-Befund 7 vom 2026-08-29). Ein abgeloester Thread haelt nur noch seine
/// `Arc`s und endet von selbst, sobald der Fremdaufruf zurueckkommt.
pub trait Senke: Send + Sync {
    fn control_verbunden(&self, link_id: &str, hello: &HelloControl);
    fn control_getrennt(&self, link_id: &str);
    fn telemetrie_gekoppelt(&self, link_id: &str);
    fn telemetrie_getrennt(&self, link_id: &str);
    /// P0. Ein `Some(payload)` wird als v3-P0-Frame zurueckgeschrieben.
    fn p0(&self, link_id: &str, payload: &[u8]) -> Option<Vec<u8>>;
    fn p1(&self, link_id: &str, payload: &[u8]);
    fn p2(&self, link_id: &str, payload: &[u8]);
    /// Eine Verbindung wurde abgewiesen oder geschlossen; `grund` ist ein
    /// kurzer, maschinennaher Text fuer Diagnose und Manifest.
    fn abgewiesen(&self, grund: &str);
}

/// Zaehlende Standardsenke: beantwortet `heartbeat` mit `heartbeat_ack` und
/// haelt genau die Zahlen, die das Lastbein braucht.
#[derive(Debug, Default)]
pub struct ZaehlSenke {
    pub control_verbindungen: AtomicU64,
    pub telemetrie_verbindungen: AtomicU64,
    pub control_getrennt: AtomicU64,
    pub telemetrie_getrennt: AtomicU64,
    pub p0: AtomicU64,
    pub p0_beantwortet: AtomicU64,
    pub p1: AtomicU64,
    pub p2: AtomicU64,
    pub abweisungen: AtomicU64,
    pub letzter_grund: Mutex<String>,
}

impl Senke for ZaehlSenke {
    fn control_verbunden(&self, _link_id: &str, _hello: &HelloControl) {
        self.control_verbindungen.fetch_add(1, Ordering::SeqCst);
    }
    fn control_getrennt(&self, _link_id: &str) {
        self.control_getrennt.fetch_add(1, Ordering::SeqCst);
    }
    fn telemetrie_gekoppelt(&self, _link_id: &str) {
        self.telemetrie_verbindungen.fetch_add(1, Ordering::SeqCst);
    }
    fn telemetrie_getrennt(&self, _link_id: &str) {
        self.telemetrie_getrennt.fetch_add(1, Ordering::SeqCst);
    }

    fn p0(&self, _link_id: &str, payload: &[u8]) -> Option<Vec<u8>> {
        self.p0.fetch_add(1, Ordering::SeqCst);
        // Nur `heartbeat` wird beantwortet — und zwar mit genau dem
        // `heartbeat_ack` aus dem v3-Schema (type, sequence,
        // duplicate_instance_id). Eine erfundene Antwortform waere eine
        // Luege ueber den Vertrag.
        let wert: serde_json::Value = serde_json::from_slice(payload).ok()?;
        if wert.get("type")?.as_str()? != "heartbeat" {
            return None;
        }
        let seq = wert.get("sequence")?.as_u64()?;
        self.p0_beantwortet.fetch_add(1, Ordering::SeqCst);
        Some(
            format!("{{\"type\":\"heartbeat_ack\",\"sequence\":{seq},\"duplicate_instance_id\":false}}")
                .into_bytes(),
        )
    }

    fn p1(&self, _link_id: &str, _payload: &[u8]) {
        self.p1.fetch_add(1, Ordering::SeqCst);
    }
    fn p2(&self, _link_id: &str, _payload: &[u8]) {
        self.p2.fetch_add(1, Ordering::SeqCst);
    }
    fn abgewiesen(&self, grund: &str) {
        self.abweisungen.fetch_add(1, Ordering::SeqCst);
        if let Ok(mut g) = self.letzter_grund.lock() {
            *g = grund.to_string();
        }
    }
}

//==============================================================================
// Overlapped-I/O: die kleinste Menge Win32, die drei Threads auf einem Handle
// erlaubt.

/// Ein Ereignisobjekt fuer overlapped I/O. JEDER Thread haelt sein eigenes —
/// zwei Threads an einem Event waere genau das Rennen, das die Trennung
/// vermeiden soll.
struct Ereignis(HANDLE);

// SAFETY: ein Event-HANDLE ist eine prozessweite Kernel-Referenz ohne
// Thread-Affinitaet; dieser Typ gibt es nie zwei Threads gleichzeitig weiter.
unsafe impl Send for Ereignis {}

impl Ereignis {
    fn neu() -> Option<Self> {
        // SAFETY: alle Zeiger sind null bzw. gueltig; das Handle wird im Drop
        // genau einmal geschlossen.
        let h = unsafe { CreateEventW(std::ptr::null(), 1, 0, std::ptr::null()) };
        if h.is_null() {
            None
        } else {
            Some(Ereignis(h))
        }
    }
    fn roh(&self) -> HANDLE {
        self.0
    }
}

impl Drop for Ereignis {
    fn drop(&mut self) {
        // SAFETY: exklusiver Besitz, genau einmal geschlossen.
        unsafe { CloseHandle(self.0) };
    }
}

fn leeres_overlapped(e: HANDLE) -> OVERLAPPED {
    // SAFETY: OVERLAPPED ist ein reines POD-Feld ohne Invarianten; genullt ist
    // der von Win32 verlangte Startzustand.
    let mut ov: OVERLAPPED = unsafe { std::mem::zeroed() };
    ov.hEvent = e;
    ov
}

/// Ausgang eines overlapped Lesevorgangs.
enum IoAusgang {
    Bytes(usize),
    /// Der Peer hat geschlossen.
    Ende,
    /// `CancelIoEx` — Stop, Bootstrapfrist oder Verbindungsende.
    Abgebrochen,
    Fehler(u32),
}

fn io_fehler_deuten(f: u32) -> IoAusgang {
    match f {
        ERROR_BROKEN_PIPE | ERROR_PIPE_NOT_CONNECTED | ERROR_NO_DATA => IoAusgang::Ende,
        ERROR_OPERATION_ABORTED => IoAusgang::Abgebrochen,
        andere => IoAusgang::Fehler(andere),
    }
}

/// Ein Lesevorgang. Er wartet unbegrenzt; beendet wird er durch `CancelIoEx`
/// (Stop, Bootstrapfrist, Verbindungsende) — nicht durch einen Timeout, denn
/// eine stille Sekunde ist kein Fehler.
fn ov_lesen(h: HANDLE, e: HANDLE, ziel: &mut [u8]) -> IoAusgang {
    // SAFETY: `h` ist ein gueltiges, overlapped geoeffnetes Pipe-Handle, `e`
    // gehoert allein diesem Thread, `ov` lebt bis GetOverlappedResult zurueck
    // ist, und `ziel` bleibt fuer die Dauer des Aufrufs gueltig.
    unsafe {
        ResetEvent(e);
        let mut ov = leeres_overlapped(e);
        let mut n: u32 = 0;
        let ok = ReadFile(
            h,
            ziel.as_mut_ptr(),
            ziel.len() as u32,
            std::ptr::null_mut(),
            &mut ov,
        );
        if ok == 0 {
            let f = GetLastError();
            if f != ERROR_IO_PENDING {
                return io_fehler_deuten(f);
            }
            if WaitForSingleObject(e, INFINITE) != WAIT_OBJECT_0 {
                CancelIoEx(h, &ov);
                let _ = GetOverlappedResult(h, &ov, &mut n, 1);
                return IoAusgang::Abgebrochen;
            }
        }
        if GetOverlappedResult(h, &ov, &mut n, 1) == 0 {
            return io_fehler_deuten(GetLastError());
        }
        if n == 0 {
            IoAusgang::Ende
        } else {
            IoAusgang::Bytes(n as usize)
        }
    }
}

/// Schreibt ALLE Bytes oder scheitert. Blockiert, solange der Peer nicht
/// abholt — genau deshalb sitzt der Aufruf auf einem eigenen Thread.
fn ov_schreiben(h: HANDLE, e: HANDLE, daten: &[u8]) -> bool {
    let mut ab = 0usize;
    while ab < daten.len() {
        // SAFETY: wie in `ov_lesen`.
        unsafe {
            ResetEvent(e);
            let mut ov = leeres_overlapped(e);
            let mut n: u32 = 0;
            let rest = &daten[ab..];
            let ok = WriteFile(
                h,
                rest.as_ptr(),
                rest.len() as u32,
                std::ptr::null_mut(),
                &mut ov,
            );
            if ok == 0 {
                let f = GetLastError();
                if f != ERROR_IO_PENDING {
                    return false;
                }
                if WaitForSingleObject(e, INFINITE) != WAIT_OBJECT_0 {
                    CancelIoEx(h, &ov);
                    let _ = GetOverlappedResult(h, &ov, &mut n, 1);
                    return false;
                }
            }
            if GetOverlappedResult(h, &ov, &mut n, 1) == 0 || n == 0 {
                return false;
            }
            ab += n as usize;
        }
    }
    true
}

/// Besitzt genau ein Verbindungshandle. Alle drei Threads halten einen `Arc`
/// darauf; geschlossen wird es, wenn der letzte geht.
struct Verbindungsgriff {
    h: HANDLE,
}

// SAFETY: Win32-HANDLEs sind prozessweite Kernel-Referenzen ohne
// Thread-Affinitaet; overlapped I/O erlaubt gleichzeitige Operationen.
unsafe impl Send for Verbindungsgriff {}
unsafe impl Sync for Verbindungsgriff {}

impl Drop for Verbindungsgriff {
    fn drop(&mut self) {
        // SAFETY: exklusiver Besitz ueber den Arc, genau einmal geschlossen.
        unsafe { CloseHandle(self.h) };
    }
}

//==============================================================================
// Die zwei bounded Queues zwischen den drei Threads.

/// Ingress je Verbindung (Cap 256, §53.9). Der Leser reiht ein, der
/// Verbraucher entnimmt — die Politik selbst liegt in `warteschlange.rs`.
struct Eingang {
    inhalt: Mutex<(IngressWarteschlange<(Familie, Vec<u8>)>, bool)>,
    signal: Condvar,
}

impl Eingang {
    fn neu() -> Self {
        Self {
            inhalt: Mutex::new((IngressWarteschlange::neu(), false)),
            signal: Condvar::new(),
        }
    }

    fn einreihen(&self, familie: Familie, payload: Vec<u8>) -> IngressErgebnis {
        let e = {
            let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
            g.0.einreihen(familie, (familie, payload))
        };
        self.signal.notify_one();
        e
    }

    /// Blockiert, bis ein Eintrag da ist oder die Queue geschlossen wurde.
    ///
    /// Das SCHLIESSFLAG steht VOR dem Inhaltstest. Vorher lief der Verbraucher
    /// nach dem Schliessen noch durch den Restbestand — und rief P0/P1 fuer
    /// eine Verbindung, deren Kopplung schon abgemeldet war. Die Frames
    /// gehoeren zu einer Sitzung, die es nicht mehr gibt; sie fallen mit ihr
    /// (T2-Befund 4 Runde 3 vom 2026-08-29).
    fn entnehmen(&self) -> Option<(Familie, Vec<u8>)> {
        let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
        loop {
            if g.1 {
                return None;
            }
            if let Some((_, wert)) = g.0.entnehmen() {
                return Some(wert);
            }
            let (neu, _) = self
                .signal
                .wait_timeout(g, Duration::from_millis(50))
                .unwrap_or_else(|x| x.into_inner());
            g = neu;
        }
    }

    fn laenge(&self) -> usize {
        self.inhalt.lock().unwrap_or_else(|x| x.into_inner()).0.len()
    }

    fn schliessen(&self) {
        {
            let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
            g.1 = true;
        }
        self.signal.notify_all();
    }
}

/// Writerqueue je Verbindung (Cap `CAP_WRITER`). Nur der Verbraucher reiht
/// ein, nur der Schreiber entnimmt.
struct Ausgang {
    inhalt: Mutex<(VecDeque<Vec<u8>>, bool)>,
    signal: Condvar,
}

impl Ausgang {
    fn neu() -> Self {
        Self {
            inhalt: Mutex::new((VecDeque::with_capacity(16), false)),
            signal: Condvar::new(),
        }
    }

    /// `false` = die Queue ist voll oder geschlossen. Voll heisst: der Peer
    /// holt seine Antworten nicht ab.
    fn einreihen(&self, frame: Vec<u8>) -> bool {
        let ok = {
            let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
            if g.1 || g.0.len() >= CAP_WRITER {
                false
            } else {
                g.0.push_back(frame);
                true
            }
        };
        if ok {
            self.signal.notify_one();
        }
        ok
    }

    fn entnehmen(&self) -> Option<Vec<u8>> {
        let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
        loop {
            if let Some(f) = g.0.pop_front() {
                return Some(f);
            }
            if g.1 {
                return None;
            }
            let (neu, _) = self
                .signal
                .wait_timeout(g, Duration::from_millis(50))
                .unwrap_or_else(|x| x.into_inner());
            g = neu;
        }
    }

    fn schliessen(&self) {
        {
            let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
            g.1 = true;
        }
        self.signal.notify_all();
    }
}

//==============================================================================

/// Handles der lebenden Verbindungen — nur, damit `stoppen()` blockierende
/// Reads wirklich loesen kann. Ein Stop, der auf einen stillen Peer wartet,
/// ist kein Stop (wissen/engineering 2026-08-27: "Cancel als Abschluss
/// behandelt").
#[derive(Default)]
struct HandleRegister {
    offen: Vec<(u64, isize)>,
}

// SAFETY: Win32-HANDLEs sind prozessweite Kernel-Referenzen ohne Thread-
// Affinitaet. Das Register haelt sie nur, solange der besitzende Thread seinen
// `Verbindungsgriff` noch nicht fallen gelassen hat; Eintragen und Austragen
// laufen unter demselben Mutex wie das Abbrechen.
unsafe impl Send for HandleRegister {}

pub struct V3Griff {
    stop: Arc<AtomicBool>,
    pipe_name: String,
    acceptor: Option<JoinHandle<()>>,
    verbindungen: Arc<Mutex<Vec<JoinHandle<()>>>>,
    handles: Arc<Mutex<HandleRegister>>,
    /// Deadlines der noch nicht abgeschlossenen Bootstraps.
    bootstraps: Arc<Mutex<Vec<(u64, Instant)>>>,
    wachhund: Option<JoinHandle<()>>,
    pub statistik: Arc<V3Statistik>,
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
    pub ingress_p2_verworfen: AtomicU64,
    pub ingress_p1_verworfen: AtomicU64,
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
        self.verbindungen.lock().unwrap_or_else(|e| e.into_inner()).len()
    }

    /// Wie viele Verbindungshandles das Abbruchregister gerade fuehrt. Ein
    /// Handle steht dort, seit der Acceptor die Verbindung angenommen hat —
    /// NICHT erst, seit sein Thread laeuft (T2-Befund 6 vom 2026-08-29).
    pub fn gehaltene_handles(&self) -> usize {
        self.handles.lock().unwrap_or_else(|e| e.into_inner()).offen.len()
    }

    /// Gegenpfad zu `v3_server_starten`. Setzt Stop, weckt den parkenden
    /// Acceptor mit einer eigenen Verbindung und bricht die I/O aller
    /// lebenden Verbindungen ab; danach werden alle Threads gejoint.
    pub fn stoppen(&mut self) {
        self.stop.store(true, Ordering::SeqCst);
        wecken(&self.pipe_name);
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
    }
}

impl Drop for V3Griff {
    fn drop(&mut self) {
        self.stoppen();
    }
}

fn alle_io_abbrechen(handles: &Arc<Mutex<HandleRegister>>) {
    if let Ok(reg) = handles.lock() {
        for (_, h) in reg.offen.iter() {
            // SAFETY: der Eintrag lebt nur, solange der besitzende Thread sein
            // Handle haelt; Austragen und Abbrechen laufen unter diesem Mutex.
            unsafe {
                CancelIoEx(*h as HANDLE, std::ptr::null_mut());
            }
        }
    }
}

fn io_abbrechen(handles: &Arc<Mutex<HandleRegister>>, id: u64) {
    if let Ok(reg) = handles.lock() {
        for (i, h) in reg.offen.iter() {
            if *i == id {
                // SAFETY: siehe alle_io_abbrechen.
                unsafe {
                    CancelIoEx(*h as HANDLE, std::ptr::null_mut());
                }
            }
        }
    }
}

/// Joint mit Frist. Laeuft sie ab, wird der Thread ABGELOEST statt gejoint:
/// das `JoinHandle` faellt, der Thread haelt nur noch seine `Arc`s und endet
/// von selbst, sobald sein Fremdaufruf zurueckkommt. `false` = abgeloest.
///
/// `zwischendurch` laeuft in jeder Warterunde — der Schreiber braucht dort
/// sein wiederholtes `CancelIoEx`, der Verbraucher nichts.
fn join_mit_frist(
    j: JoinHandle<()>,
    frist: Duration,
    mut zwischendurch: impl FnMut(),
) -> bool {
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
        Self { senke, statistik, stumm: false }
    }

    /// `false` = der Aufruf kam nicht binnen Frist zurueck, oder er unterblieb,
    /// weil ein frueherer noch haengt.
    fn rufen(&mut self, f: impl FnOnce(&dyn Senke) + Send + 'static) -> bool {
        if self.stumm {
            self.statistik
                .lebenszyklus_uebersprungen
                .fetch_add(1, Ordering::SeqCst);
            return false;
        }
        let s = self.senke.clone();
        let j = match std::thread::Builder::new()
            .name("eqcop-v3-senkenruf".into())
            .spawn(move || f(s.as_ref()))
        {
            Ok(j) => j,
            Err(_) => {
                self.stumm = true;
                return false;
            }
        };
        if join_mit_frist(j, SENKE_FRIST, || {}) {
            return true;
        }
        self.stumm = true;
        self.statistik
            .lebenszyklus_abgeloest
            .fetch_add(1, Ordering::SeqCst);
        false
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

/// Weckt einen in `ConnectNamedPipe` parkenden Acceptor.
fn wecken(pipe_name: &str) {
    let mut w: Vec<u16> = pipe_name.encode_utf16().collect();
    w.push(0);
    // SAFETY: `w` ist nullterminiert; ein Fehlschlag ist hier belanglos —
    // der Acceptor beendet sich dann beim naechsten Durchlauf ueber `stop`.
    unsafe {
        let h = windows_sys::Win32::Storage::FileSystem::CreateFileW(
            w.as_ptr(),
            windows_sys::Win32::Foundation::GENERIC_READ,
            0,
            std::ptr::null(),
            windows_sys::Win32::Storage::FileSystem::OPEN_EXISTING,
            windows_sys::Win32::Storage::FileSystem::SECURITY_SQOS_PRESENT
                | windows_sys::Win32::Storage::FileSystem::SECURITY_IDENTIFICATION,
            std::ptr::null_mut(),
        );
        if h != INVALID_HANDLE_VALUE {
            CloseHandle(h);
        }
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
) -> Option<HANDLE> {
    let mut fremde_fehler = 0u32;
    loop {
        if stop.load(Ordering::SeqCst) {
            return None;
        }
        // SAFETY: `name_w` ist nullterminiert, `attrs` lebt ueber den Aufruf.
        let h = unsafe {
            CreateNamedPipeW(
                name_w.as_ptr(),
                PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                MAX_VERBINDUNGEN as u32,
                65536,
                65536,
                0,
                attrs,
            )
        };
        if h != INVALID_HANDLE_VALUE {
            return Some(h);
        }
        // SAFETY: GetLastError liest nur den threadlokalen Fehlercode.
        let f = unsafe { GetLastError() };
        if f == ERROR_PIPE_BUSY {
            statistik.acceptor_wartet_auf_instanz.fetch_add(1, Ordering::SeqCst);
        } else {
            // Nicht die Verbindungsgrenze, sondern etwas anderes. Ein paar
            // Versuche sind billig; endlos zu drehen waere ein stiller Hang.
            fremde_fehler += 1;
            if fremde_fehler > 200 {
                return None;
            }
        }
        // Ein beendeter Nachbar gibt seinen Platz erst frei, wenn sein Handle
        // wirklich zu ist — also hier ernten, nicht nur schlafen.
        fertige_ernten(verbindungen);
        std::thread::sleep(Duration::from_millis(25));
    }
}

/// Startet den v3-Listener auf `pipe_name`.
///
/// Der Aufrufer waehlt den Namen — im Test und in der Probe ist es ein
/// Probe-Name, nie die Produktions-Pipe.
pub fn v3_server_starten(
    pipe_name: &str,
    senke: Arc<dyn Senke>,
    broker_version: String,
) -> Result<V3Griff, String> {
    v3_server_starten_intern(pipe_name, senke, broker_version, Arc::new(AtomicU64::new(0)))
}

/// Wie `v3_server_starten`, aber mit einer Testnaht: `probe_verzoegerung_ms`
/// laesst jeden frisch angenommenen Verbindungsthread vor seiner ersten Arbeit
/// warten. Damit trifft ein Test das Fenster zwischen Annahme und Bedienung
/// deterministisch, statt es zu erwuerfeln. In Produktion ist der Wert 0.
fn v3_server_starten_intern(
    pipe_name: &str,
    senke: Arc<dyn Senke>,
    broker_version: String,
    probe_verzoegerung_ms: Arc<AtomicU64>,
) -> Result<V3Griff, String> {
    let sicherheit = crate::server::sicherheit_nur_user()?;
    let stop = Arc::new(AtomicBool::new(false));
    let verbindungen: Arc<Mutex<Vec<JoinHandle<()>>>> = Arc::new(Mutex::new(Vec::new()));
    let handles: Arc<Mutex<HandleRegister>> = Arc::new(Mutex::new(HandleRegister::default()));
    let bootstraps: Arc<Mutex<Vec<(u64, Instant)>>> = Arc::new(Mutex::new(Vec::new()));
    let statistik = Arc::new(V3Statistik::default());
    let kopplungen = Arc::new(Mutex::new(Kopplungen::neu()));
    let broker_epoch = neue_kennung();

    let mut name_w: Vec<u16> = pipe_name.encode_utf16().collect();
    name_w.push(0);

    let attrs = SECURITY_ATTRIBUTES {
        nLength: std::mem::size_of::<SECURITY_ATTRIBUTES>() as u32,
        lpSecurityDescriptor: sicherheit.deskriptor,
        bInheritHandle: 0,
    };

    // Erste Instanz synchron: ein fremder Besitzer des Namens muss SOFORT
    // sichtbar scheitern, nicht die Haelfte der Sonden stehlen (M2-Fund).
    // SAFETY: `name_w` ist nullterminiert, `attrs` lebt ueber den Aufruf.
    let erstes = unsafe {
        CreateNamedPipeW(
            name_w.as_ptr(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
            MAX_VERBINDUNGEN as u32,
            65536,
            65536,
            0,
            &attrs,
        )
    };
    if erstes == INVALID_HANDLE_VALUE {
        // SAFETY: GetLastError liest nur den threadlokalen Fehlercode.
        let f = unsafe { GetLastError() };
        return Err(format!("CreateNamedPipe v3 (erste Instanz): Win32 {f}"));
    }

    let stop_w = stop.clone();
    let handles_w = handles.clone();
    let bootstraps_w = bootstraps.clone();
    let verbindungen_w = verbindungen.clone();
    let wachhund = std::thread::Builder::new()
        .name("eqcop-v3-wachhund".into())
        .spawn(move || {
            while !stop_w.load(Ordering::SeqCst) {
                std::thread::sleep(Duration::from_millis(100));
                // Auch ohne neue Verbindung muessen fertige Threads fallen.
                fertige_ernten(&verbindungen_w);
                let jetzt = Instant::now();
                let faellig: Vec<u64> = {
                    let b = bootstraps_w.lock().unwrap_or_else(|e| e.into_inner());
                    b.iter().filter(|(_, f)| *f <= jetzt).map(|(id, _)| *id).collect()
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

    // HANDLE ist ein roher Zeiger und damit nicht `Send`. Der Acceptor
    // bekommt ihn deshalb als `isize` und setzt ihn drinnen wieder zusammen —
    // dieselbe Form, die `HandleRegister` benutzt.
    let erstes_isize = erstes as isize;
    let stop2 = stop.clone();
    let verbindungen2 = verbindungen.clone();
    let handles2 = handles.clone();
    let bootstraps2 = bootstraps.clone();
    let statistik2 = statistik.clone();
    let acceptor = std::thread::Builder::new()
        .name("eqcop-v3-acceptor".into())
        .spawn(move || {
            // Der Deskriptor gehoert ab hier dem Acceptor.
            let sicherheit = sicherheit;
            let attrs = SECURITY_ATTRIBUTES {
                nLength: std::mem::size_of::<SECURITY_ATTRIBUTES>() as u32,
                lpSecurityDescriptor: sicherheit.deskriptor,
                bInheritHandle: 0,
            };
            let ereignis = match Ereignis::neu() {
                Some(e) => e,
                None => return,
            };
            let mut naechstes: HANDLE = erstes_isize as HANDLE;
            let mut folge: u64 = 0;
            loop {
                if stop2.load(Ordering::SeqCst) {
                    // SAFETY: `naechstes` ist ein gueltiges, noch nicht
                    // uebergebenes Pipe-Handle.
                    unsafe { CloseHandle(naechstes) };
                    break;
                }
                let verbunden = warten_auf_verbindung(naechstes, ereignis.roh());
                if !verbunden {
                    // SAFETY: exklusives Handle, genau einmal geschlossen.
                    unsafe { CloseHandle(naechstes) };
                    if stop2.load(Ordering::SeqCst) {
                        break;
                    }
                    match naechste_instanz(&name_w, &attrs, &stop2, &verbindungen2, &statistik2) {
                        Some(h) => {
                            naechstes = h;
                            continue;
                        }
                        None => break,
                    }
                }
                if stop2.load(Ordering::SeqCst) {
                    // SAFETY: exklusives Handle.
                    unsafe {
                        DisconnectNamedPipe(naechstes);
                        CloseHandle(naechstes);
                    }
                    break;
                }

                folge += 1;
                let id = folge;
                let griff = Arc::new(Verbindungsgriff { h: naechstes });
                statistik2.angenommen.fetch_add(1, Ordering::SeqCst);

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
                    r.offen.push((id, naechstes as isize));
                }
                // Der Eintrag gehoert ab hier dem Thread; scheitert `spawn`,
                // faellt die Closure samt Eintrag und traegt ihn wieder aus.
                let handle_eintrag = HandleEintrag { id, register: handles2.clone() };
                let verzoegerung = probe_verzoegerung_ms.clone();

                let senke = senke.clone();
                let kopplungen = kopplungen.clone();
                let handles = handles2.clone();
                let bootstraps = bootstraps2.clone();
                let statistik = statistik2.clone();
                let bv = broker_version.clone();
                let be = broker_epoch.clone();
                let conn_stop = stop2.clone();
                match std::thread::Builder::new()
                    .name("eqcop-v3-conn".into())
                    .spawn(move || {
                        // Testnaht: reisst das Fenster zwischen Annahme und
                        // erster Arbeit deterministisch auf. In Produktion
                        // steht hier 0 und der Aufruf kostet einen Ladevorgang.
                        let ms = verzoegerung.load(Ordering::SeqCst);
                        if ms > 0 {
                            std::thread::sleep(Duration::from_millis(ms));
                        }
                        verbindung_bedienen(
                            id, griff, senke, kopplungen, handles, bootstraps, statistik, bv, be,
                            conn_stop, handle_eintrag,
                        );
                    }) {
                    Ok(j) => verbindungen2.lock().unwrap_or_else(|e| e.into_inner()).push(j),
                    Err(_) => break,
                }

                // Beendete Nachbarn ernten, DANN die naechste Instanz holen —
                // sonst zaehlt ein laengst toter Thread noch gegen die Grenze.
                fertige_ernten(&verbindungen2);
                match naechste_instanz(&name_w, &attrs, &stop2, &verbindungen2, &statistik2) {
                    Some(h) => naechstes = h,
                    None => break,
                }
            }
        })
        .map_err(|e| format!("v3-Acceptorthread: {e}"))?;

    Ok(V3Griff {
        stop,
        pipe_name: pipe_name.to_string(),
        acceptor: Some(acceptor),
        verbindungen,
        handles,
        bootstraps,
        wachhund: Some(wachhund),
        statistik,
    })
}

/// `ConnectNamedPipe` auf einem overlapped Handle. `lpOverlapped` DARF hier
/// nicht null sein — sonst meldet Win32 den Verbindungsaufbau falsch fertig.
fn warten_auf_verbindung(h: HANDLE, e: HANDLE) -> bool {
    // SAFETY: `h` ist die eben angelegte Pipe-Instanz, `e` gehoert dem
    // Acceptor allein, `ov` lebt bis GetOverlappedResult zurueck ist.
    unsafe {
        ResetEvent(e);
        let mut ov = leeres_overlapped(e);
        let ok = ConnectNamedPipe(h, &mut ov);
        if ok != 0 {
            return true;
        }
        let f = GetLastError();
        if f == ERROR_PIPE_CONNECTED {
            return true;
        }
        if f != ERROR_IO_PENDING {
            return false;
        }
        if WaitForSingleObject(e, INFINITE) != WAIT_OBJECT_0 {
            CancelIoEx(h, &ov);
            return false;
        }
        let mut n: u32 = 0;
        GetOverlappedResult(h, &ov, &mut n, 1) != 0
    }
}

struct HandleEintrag {
    id: u64,
    register: Arc<Mutex<HandleRegister>>,
}

impl Drop for HandleEintrag {
    fn drop(&mut self) {
        if let Ok(mut r) = self.register.lock() {
            r.offen.retain(|(i, _)| *i != self.id);
        }
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
    handles: Arc<Mutex<HandleRegister>>,
    bootstraps: Arc<Mutex<Vec<(u64, Instant)>>>,
    statistik: Arc<V3Statistik>,
    broker_version: String,
    broker_epoch: String,
    stop: Arc<AtomicBool>,
    handle_eintrag: HandleEintrag,
) {
    // Der Acceptor hat das Handle schon registriert; der Eintrag traegt es
    // beim Verlassen dieser Funktion wieder aus.
    let _handle_eintrag = handle_eintrag;

    // Und JETZT `stop` pruefen. Kam der Stop im Fenster zwischen Annahme und
    // diesem Punkt, endet der Thread hier — statt sich in einen Bootstrap-Read
    // zu legen, den kein Wachhund mehr aufloest (T2-Befund 6 vom 2026-08-29).
    if stop.load(Ordering::SeqCst) {
        return;
    }

    // Jeder Lebenszyklus- und Diagnoseaufruf dieser Verbindung geht ueber den
    // fristbegrenzten, abloesbaren Pfad — direkt auf diesem Thread waeren sie
    // ein unbegrenztes `stoppen()` (T2-Befund 5 Runde 3).
    let mut senkenruf = Senkenruf::neu(senke.clone(), statistik.clone());

    let leseereignis = match Ereignis::neu() {
        Some(e) => e,
        None => return,
    };

    // Bootstrap-Frist: der Wachhund bricht die I/O ab, wenn das Hello nicht
    // rechtzeitig ganz da ist.
    if let Ok(mut b) = bootstraps.lock() {
        b.push((id, Instant::now() + BOOTSTRAP_FRIST));
    }
    let frist = BootstrapFrist { id, liste: bootstraps.clone() };

    let mut roh: Vec<u8> = Vec::with_capacity(4096);
    let mut puffer = [0u8; 4096];

    // ── Bootstrap lesen ───────────────────────────────────────────────────
    let (bs, verbraucht) = loop {
        match bootstrap_lesen(&roh) {
            Ok(x) => break x,
            Err(BootstrapFehler::PraefixUnvollstaendig) | Err(BootstrapFehler::Unvollstaendig) => {}
            Err(e) => {
                statistik.geschlossen_bootstrap.fetch_add(1, Ordering::SeqCst);
                senkenruf.abweisen(format!("bootstrap: {e:?}"));
                return;
            }
        }
        match ov_lesen(griff.h, leseereignis.roh(), &mut puffer) {
            IoAusgang::Ende => {
                senkenruf.abweisen("bootstrap: Verbindung vor dem Hello beendet");
                return;
            }
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
                    statistik.geschlossen_bootstrap.fetch_add(1, Ordering::SeqCst);
                    senkenruf.abweisen("bootstrap: mehr Bytes als ein Hello tragen darf");
                    return;
                }
            }
            IoAusgang::Abgebrochen | IoAusgang::Fehler(_) => {
                statistik.geschlossen_bootstrap.fetch_add(1, Ordering::SeqCst);
                senkenruf.abweisen("bootstrap: Lesefehler oder Frist abgelaufen");
                return;
            }
        }
    };
    frist.erfuellt();
    roh.drain(0..verbraucht);

    let (link_id, ist_control) = match bs {
        Bootstrap::V2 { .. } => {
            // Auf dem v3-Endpunkt gibt es kein v2-Register. Statt still zu
            // scheitern bekommt der Peer einen v2-gerahmten `reject` — er
            // spricht ja noch v2-Framing (§33.3 "klarer Kompatibilitaetsfehler").
            let json = "{\"type\":\"reject\",\"code\":\"protocol_mismatch\",\
                        \"reason\":\"dieser Endpunkt spricht nur v3\"}";
            let mut rahmen = (json.len() as u32).to_le_bytes().to_vec();
            rahmen.extend_from_slice(json.as_bytes());
            let _ = ov_schreiben(griff.h, leseereignis.roh(), &rahmen);
            statistik.geschlossen_bootstrap.fetch_add(1, Ordering::SeqCst);
            senkenruf.abweisen("bootstrap: v2-Hello am v3-Endpunkt");
            return;
        }
        Bootstrap::V3Control(h) => {
            let link = neue_kennung();
            let challenge = neue_kennung();
            {
                let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                if k.control_anmelden(&h.adresse.runtime_nonce, link.clone(), challenge.clone())
                    .is_err()
                {
                    statistik.geschlossen_bootstrap.fetch_add(1, Ordering::SeqCst);
                    senkenruf.abweisen("bootstrap: zu viele offene Kopplungen");
                    return;
                }
            }
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
            match envelope_schreiben(Familie::P0, 0, &payload) {
                Ok(frame) => {
                    if !ov_schreiben(griff.h, leseereignis.roh(), &frame) {
                        let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                        k.control_abmelden(&link);
                        return;
                    }
                }
                Err(_) => return,
            }
            // Kommt die Senke hier nicht binnen Frist zurueck, wird der
            // Aufruf abgeloest — und diese Verbindung endet, statt bedient zu
            // werden: eine Senke, die den Verbindungsbeginn nicht annehmen
            // kann, wird auch ihre Frames nicht annehmen. Der Platz wird
            // sofort frei, statt bis zum Stop belegt zu bleiben.
            //
            // Der Abbau laeuft ueber `kopplung_loesen`, nicht ueber ein blosses
            // `control_abmelden`: der Peer hat sein `welcome` schon, und in den
            // bis zu `SENKE_FRIST` kann er die Telemetrieverbindung bereits
            // gekoppelt haben. Sie faellt mit — sonst bliebe genau die halb
            // offene Kopplung stehen, die Runde 1 geschlossen hat.
            //
            // Ein `control_getrennt` folgt hier bewusst NICHT: der abgeloeste
            // `control_verbunden` steht noch; ein Gegenstueck davor waere eine
            // Luege ueber die Reihenfolge. Die Senke sieht ihren Verbindungs-
            // beginn also ohne Ende — sichtbar an `lebenszyklus_abgeloest`.
            let link_fuer_senke = link.clone();
            let hello_fuer_senke = (*h).clone();
            if !senkenruf.rufen(move |s| {
                s.control_verbunden(&link_fuer_senke, &hello_fuer_senke)
            }) {
                kopplung_loesen(&kopplungen, &handles, &link, true);
                return;
            }
            (link, true)
        }
        Bootstrap::V3Telemetry(h) => {
            let ok = {
                let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                k.telemetrie_koppeln(&h, id)
            };
            if let Err(e) = ok {
                statistik.geschlossen_bootstrap.fetch_add(1, Ordering::SeqCst);
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
            if let Ok(frame) = envelope_schreiben(Familie::P0, 0, &payload) {
                if !ov_schreiben(griff.h, leseereignis.roh(), &frame) {
                    let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                    k.telemetrie_entkoppeln(&h.link_id);
                    return;
                }
            }
            let link_fuer_senke = h.link_id.clone();
            if !senkenruf.rufen(move |s| s.telemetrie_gekoppelt(&link_fuer_senke)) {
                kopplung_loesen(&kopplungen, &handles, &h.link_id, false);
                return;
            }
            (h.link_id.clone(), false)
        }
    };

    // ── Ab hier ausschliesslich v3-Frames, auf drei Threads ───────────────
    let ende = Arc::new(AtomicBool::new(false));
    let eingang = Arc::new(Eingang::neu());
    let ausgang = Arc::new(Ausgang::neu());

    let schreiber = {
        let griff = griff.clone();
        let ausgang = ausgang.clone();
        std::thread::Builder::new()
            .name("eqcop-v3-writer".into())
            .spawn(move || {
                let ereignis = match Ereignis::neu() {
                    Some(e) => e,
                    None => return,
                };
                while let Some(frame) = ausgang.entnehmen() {
                    if !ov_schreiben(griff.h, ereignis.roh(), &frame) {
                        break;
                    }
                }
            })
            .ok()
    };

    let verbraucher = {
        let senke = senke.clone();
        let eingang = eingang.clone();
        let ausgang = ausgang.clone();
        let statistik = statistik.clone();
        let handles = handles.clone();
        let ende = ende.clone();
        let link = link_id.clone();
        std::thread::Builder::new()
            .name("eqcop-v3-ingress".into())
            .spawn(move || {
                while let Some((familie, payload)) = eingang.entnehmen() {
                    match familie {
                        Familie::P0 => {
                            if let Some(antwort) = senke.p0(&link, &payload) {
                                if let Ok(frame) = envelope_schreiben(Familie::P0, 0, &antwort) {
                                    if !ausgang.einreihen(frame) {
                                        // Der Peer holt seine Antworten nicht
                                        // ab. Still weiterzaehlen waere eine
                                        // Luege ueber "nichts verwerfen".
                                        statistik
                                            .geschlossen_writer
                                            .fetch_add(1, Ordering::SeqCst);
                                        senke.abgewiesen("writer: Antwortqueue laeuft ueber");
                                        ende.store(true, Ordering::SeqCst);
                                        io_abbrechen(&handles, id);
                                        break;
                                    }
                                }
                            }
                        }
                        Familie::P1 => senke.p1(&link, &payload),
                        Familie::P2 => senke.p2(&link, &payload),
                    }
                }
            })
            .ok()
    };

    let mut leser = StromLeser::neu();
    leser.fuettern(&roh);
    let mut rate = Ratengrenze::neu(RATE_PRO_SEKUNDE, 1000);
    let beginn = Instant::now();

    'lesen: loop {
        loop {
            match leser.naechster() {
                LeseErgebnis::Unvollstaendig => break,
                LeseErgebnis::Verstoesse(v) => {
                    statistik.geschlossen_envelope.fetch_add(1, Ordering::SeqCst);
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
                            statistik.geschlossen_kopplung.fetch_add(1, Ordering::SeqCst);
                            senkenruf.abweisen("kopplung: Control-Verbindung ist fort");
                            break 'lesen;
                        }
                    }

                    match eingang.einreihen(familie, r.payload) {
                        IngressErgebnis::Eingereiht => {}
                        IngressErgebnis::P2Verworfen => {
                            statistik.ingress_p2_verworfen.fetch_add(1, Ordering::SeqCst);
                        }
                        IngressErgebnis::ClientTrennen => {
                            statistik.geschlossen_p0_ueberlauf.fetch_add(1, Ordering::SeqCst);
                            senkenruf.abweisen("ingress: P0-Ueberlauf");
                            break 'lesen;
                        }
                        IngressErgebnis::Verworfen => {
                            if familie == Familie::P1 {
                                statistik.ingress_p1_verworfen.fetch_add(1, Ordering::SeqCst);
                            } else {
                                statistik.ingress_p2_verworfen.fetch_add(1, Ordering::SeqCst);
                            }
                        }
                    }
                    statistik.hoechststand_melden(eingang.laenge());
                }
            }
        }

        if stop.load(Ordering::SeqCst) || ende.load(Ordering::SeqCst) {
            break;
        }
        // Harte Obergrenze fuer den Lesepuffer: ein Peer darf nicht beliebig
        // viele Teilbytes anhaeufen, ohne je einen Frame zu vollenden.
        if leser.offen() > MAX_FRAME_BYTES as usize + 4 {
            statistik.geschlossen_envelope.fetch_add(1, Ordering::SeqCst);
            senkenruf.abweisen("envelope: Teilframe ueber der Paketgrenze");
            break;
        }
        match ov_lesen(griff.h, leseereignis.roh(), &mut puffer) {
            IoAusgang::Bytes(n) => leser.fuettern(&puffer[..n]),
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
    ende.store(true, Ordering::SeqCst);
    // Die KOPPLUNG faellt zuerst — vor den fristbegrenzten Joins, nicht nach
    // ihnen. Sonst blieb sie bei langsamer Senke bis zu zweimal `SENKE_FRIST`
    // im Register, und ein Telemetrieframe passierte in dieser Zeit weiter
    // `telemetrie_lebt`, obwohl die Control-Verbindung schon fort war
    // (T2-Befund 3 Runde 3 vom 2026-08-29). Der Registereintrag und der
    // Abbruch der Telemetrie-I/O gehoeren dabei zusammen: ohne den Abbruch
    // stuende der Telemetriearbeiter noch in seinem Read.
    kopplung_loesen(&kopplungen, &handles, &link_id, ist_control);
    eingang.schliessen();
    ausgang.schliessen();
    io_abbrechen(&handles, id);
    // Beide Joins haben eine FRIST. Steht der Verbraucher in einem
    // Senkenaufruf oder der Schreiber in einem Write, den `CancelIoEx` nicht
    // loest, wird der Thread abgeloest statt gejoint — sonst waere `stoppen()`
    // ein unbegrenztes Warten auf fremden Code (T2-Befund 7 vom 2026-08-29).
    if let Some(j) = verbraucher {
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
    melden_getrennt(&mut senkenruf, &link_id, ist_control);
}

/// Nimmt die Kopplung aus dem Register und bricht die I/O der mitfallenden
/// Telemetrieverbindung ab. Beruehrt die Senke NICHT — die Meldung folgt
/// getrennt, nach den Joins (`melden_getrennt`).
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
    use crate::transport::pipetoken::PROBE_PRAEFIX;
    use std::sync::atomic::AtomicUsize;

    static FOLGE: AtomicUsize = AtomicUsize::new(0);

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

    fn telemetry_hello(nonce: &str, link: &str, challenge: &str) -> Vec<u8> {
        praefix(&format!(
            "{{\"type\":\"hello\",\"connection_kind\":\"telemetry\",\"protocol\":3,\
             \"plugin_version\":\"0.3.0\",\"adresse\":{a},\"link_id\":\"{link}\",\
             \"challenge\":\"{challenge}\"}}",
            a = adresse_json(nonce)
        ))
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
        envelope_schreiben(Familie::P0, 0, json.as_bytes()).unwrap()
    }
    fn p1(json: &str) -> Vec<u8> {
        envelope_schreiben(Familie::P1, 0, json.as_bytes()).unwrap()
    }
    fn p2(bytes: &[u8]) -> Vec<u8> {
        envelope_schreiben(Familie::P2, 0, bytes).unwrap()
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
        fn control_verbunden(&self, l: &str, h: &HelloControl) {
            self.zaehl.control_verbunden(l, h);
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
            warte_auf(5000, || stat.ingress_p2_verworfen.load(Ordering::SeqCst) > 0),
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

        // Erst den Ingress mit P1 fuellen (Cap 256), dann P0 nachlegen: es
        // gibt kein P2 zum Verwerfen, also trennt der Broker.
        let eins = p1("{\"type\":\"state_report\"}");
        for _ in 0..300 {
            if !steuer.schreiben(&eins) {
                break;
            }
        }
        let herz = p0("{\"type\":\"heartbeat\",\"sequence\":1}");
        for _ in 0..40 {
            if !steuer.schreiben(&herz) {
                break;
            }
        }

        let stat = griff.statistik.clone();
        assert!(
            warte_auf(6000, || stat.geschlossen_p0_ueberlauf.load(Ordering::SeqCst) > 0),
            "P0-Ueberlauf muss die Verbindung trennen (p1_verworfen={}, hoechststand={})",
            stat.ingress_p1_verworfen.load(Ordering::SeqCst),
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
            warte_auf(4000, || stat.geschlossen_familie.load(Ordering::SeqCst) == 1),
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
            warte_auf(4000, || stat.geschlossen_familie.load(Ordering::SeqCst) == 1),
            "ein P2 auf der Controlpipe muss die Verbindung schliessen"
        );
        assert_eq!(senke.p2.load(Ordering::SeqCst), 0);
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

        // Erst wenn der Acceptor an der Grenze WIRKLICH auf einen freien Platz
        // wartet, ist die Lage hergestellt, ueber die dieser Test spricht. Ohne
        // diesen Halt liesse der Test die Verbindungen womoeglich schon wieder
        // los, bevor der Acceptor die Grenze ueberhaupt gesehen hat — und
        // bewiese dann nichts.
        let stat = griff.statistik.clone();
        assert!(
            warte_auf(8000, || stat.acceptor_wartet_auf_instanz.load(Ordering::SeqCst) > 0),
            "der Acceptor hat die Verbindungsgrenze nie erreicht ({} Verbindungen offen) —              entweder ist er schon beendet, oder der Test misst die falsche Lage",
            offen.len()
        );

        // Alles wieder loslassen — danach MUSS wieder jemand horchen.
        offen.clear();

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
        let mut griff =
            v3_server_starten_intern(&pipe, senke, "test".into(), verzoegerung).unwrap();

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
        let rechtzeitig = rx.recv_timeout(SENKE_FRIST + Duration::from_secs(8)).is_ok();
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
            e.einreihen(Familie::P0, b"a".to_vec()),
            IngressErgebnis::Eingereiht
        ));
        assert!(matches!(
            e.einreihen(Familie::P1, b"b".to_vec()),
            IngressErgebnis::Eingereiht
        ));
        assert_eq!(e.laenge(), 2, "die Vorbedingung muss wirklich gefuellt sein");

        e.schliessen();
        assert!(
            e.entnehmen().is_none(),
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
        fn control_verbunden(&self, l: &str, h: &HelloControl) {
            self.warten();
            self.zaehl.control_verbunden(l, h);
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
        let rechtzeitig = rx.recv_timeout(SENKE_FRIST + Duration::from_secs(8)).is_ok();
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
}
