//! SONDE-010 — v3-Listener und I/O-Worker.
//!
//! Der Worker entscheidet GENAU drei Dinge (Entwurf §53.9): Envelope,
//! Grenzen und Authentisierung. Alles Weitere geht als typisiertes Ereignis
//! an eine schmale `Senke`. Session, Eviction, Store und Outbox — also der
//! `Coordinator` — sind SONDE-011 und stehen hier bewusst nicht.
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

use std::fs::File;
use std::io::{Read, Write};
use std::os::windows::io::{AsRawHandle, FromRawHandle};
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::{Arc, Mutex};
use std::thread::JoinHandle;
use std::time::{Duration, Instant};

use windows_sys::Win32::Foundation::{
    CloseHandle, GetLastError, ERROR_PIPE_CONNECTED, HANDLE, INVALID_HANDLE_VALUE,
};
use windows_sys::Win32::Security::SECURITY_ATTRIBUTES;
use windows_sys::Win32::Storage::FileSystem::{FILE_FLAG_FIRST_PIPE_INSTANCE, PIPE_ACCESS_DUPLEX};
use windows_sys::Win32::System::Pipes::{
    ConnectNamedPipe, CreateNamedPipeW, DisconnectNamedPipe, PIPE_READMODE_BYTE,
    PIPE_REJECT_REMOTE_CLIENTS, PIPE_TYPE_BYTE, PIPE_WAIT,
};

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

/// Was der I/O-Worker nach oben gibt. Bewusst byteorientiert: die Bedeutung
/// des Payloads kennt erst der Coordinator.
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
    fn control_getrennt(&self, _link_id: &str) {}
    fn telemetrie_gekoppelt(&self, _link_id: &str) {
        self.telemetrie_verbindungen.fetch_add(1, Ordering::SeqCst);
    }
    fn telemetrie_getrennt(&self, _link_id: &str) {}

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

/// Handles der lebenden Verbindungen — nur, damit `stoppen()` blockierende
/// Reads wirklich loesen kann. Ein Stop, der auf einen stillen Peer wartet,
/// ist kein Stop (wissen/engineering 2026-08-27: "Cancel als Abschluss
/// behandelt").
#[derive(Default)]
struct HandleRegister {
    offen: Vec<(u64, isize)>,
}

// SAFETY: Win32-HANDLEs sind prozessweite Kernel-Referenzen ohne Thread-
// Affinitaet. Das Register haelt sie nur, solange der besitzende Thread sein
// `File` noch nicht fallen gelassen hat; Eintragen und Austragen laufen unter
// demselben Mutex wie das Abbrechen.
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
    pub ingress_p2_verworfen: AtomicU64,
    pub ingress_p1_verworfen: AtomicU64,
}

impl V3Griff {
    pub fn pipe_name(&self) -> &str {
        &self.pipe_name
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
                    alle_io_abbrechen(&self.handles);
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
            // File haelt; Austragen und Abbrechen laufen unter diesem Mutex.
            unsafe {
                windows_sys::Win32::System::IO::CancelIoEx(*h as HANDLE, std::ptr::null_mut());
            }
        }
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

/// Startet den v3-Listener auf `pipe_name`.
///
/// Der Aufrufer waehlt den Namen — im Test und in der Probe ist es ein
/// Probe-Name, nie die Produktions-Pipe.
pub fn v3_server_starten(
    pipe_name: &str,
    senke: Arc<dyn Senke>,
    broker_version: String,
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
            PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
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
    let wachhund = std::thread::Builder::new()
        .name("eqcop-v3-wachhund".into())
        .spawn(move || {
            while !stop_w.load(Ordering::SeqCst) {
                std::thread::sleep(Duration::from_millis(100));
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
                                windows_sys::Win32::System::IO::CancelIoEx(
                                    *h as HANDLE,
                                    std::ptr::null_mut(),
                                );
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
    let name2 = pipe_name.to_string();
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
            let mut naechstes: HANDLE = erstes_isize as HANDLE;
            let mut folge: u64 = 0;
            loop {
                if stop2.load(Ordering::SeqCst) {
                    // SAFETY: `naechstes` ist ein gueltiges, noch nicht
                    // uebergebenes Pipe-Handle.
                    unsafe { CloseHandle(naechstes) };
                    break;
                }
                // SAFETY: `naechstes` ist die eben angelegte Pipe-Instanz.
                let verbunden = unsafe { ConnectNamedPipe(naechstes, std::ptr::null_mut()) };
                // SAFETY: GetLastError direkt nach ConnectNamedPipe.
                let fehler = if verbunden == 0 { unsafe { GetLastError() } } else { 0 };
                if verbunden == 0 && fehler != ERROR_PIPE_CONNECTED {
                    // SAFETY: exklusives Handle, genau einmal geschlossen.
                    unsafe { CloseHandle(naechstes) };
                    if stop2.load(Ordering::SeqCst) {
                        break;
                    }
                    // SAFETY: wie oben — dieselbe Vorschrift, neue Instanz.
                    naechstes = unsafe {
                        CreateNamedPipeW(
                            name_w.as_ptr(),
                            PIPE_ACCESS_DUPLEX,
                            PIPE_TYPE_BYTE
                                | PIPE_READMODE_BYTE
                                | PIPE_WAIT
                                | PIPE_REJECT_REMOTE_CLIENTS,
                            MAX_VERBINDUNGEN as u32,
                            65536,
                            65536,
                            0,
                            &attrs,
                        )
                    };
                    if naechstes == INVALID_HANDLE_VALUE {
                        break;
                    }
                    continue;
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
                // SAFETY: das Handle geht exklusiv an das File ueber; ab hier
                // schliesst dessen Drop genau einmal.
                let datei = unsafe { File::from_raw_handle(naechstes as _) };
                statistik2.angenommen.fetch_add(1, Ordering::SeqCst);

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
                        verbindung_bedienen(
                            id, datei, senke, kopplungen, handles, bootstraps, statistik, bv, be,
                            conn_stop,
                        );
                    }) {
                    Ok(j) => verbindungen2.lock().unwrap_or_else(|e| e.into_inner()).push(j),
                    Err(_) => break,
                }

                // Naechste Instanz vorbereiten.
                // SAFETY: dieselbe Vorschrift wie oben.
                naechstes = unsafe {
                    CreateNamedPipeW(
                        name_w.as_ptr(),
                        PIPE_ACCESS_DUPLEX,
                        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                        MAX_VERBINDUNGEN as u32,
                        65536,
                        65536,
                        0,
                        &attrs,
                    )
                };
                if naechstes == INVALID_HANDLE_VALUE {
                    break;
                }
            }
            let _ = &name2;
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
    mut datei: File,
    senke: Arc<dyn Senke>,
    kopplungen: Arc<Mutex<Kopplungen>>,
    handles: Arc<Mutex<HandleRegister>>,
    bootstraps: Arc<Mutex<Vec<(u64, Instant)>>>,
    statistik: Arc<V3Statistik>,
    broker_version: String,
    broker_epoch: String,
    stop: Arc<AtomicBool>,
) {
    let roh_handle = datei.as_raw_handle() as isize;
    if let Ok(mut r) = handles.lock() {
        r.offen.push((id, roh_handle));
    }
    let _handle_eintrag = HandleEintrag { id, register: handles.clone() };

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
                senke.abgewiesen(&format!("bootstrap: {e:?}"));
                return;
            }
        }
        match datei.read(&mut puffer) {
            Ok(0) => {
                senke.abgewiesen("bootstrap: Verbindung vor dem Hello beendet");
                return;
            }
            Ok(n) => {
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
                    senke.abgewiesen("bootstrap: mehr Bytes als ein Hello tragen darf");
                    return;
                }
            }
            Err(_) => {
                statistik.geschlossen_bootstrap.fetch_add(1, Ordering::SeqCst);
                senke.abgewiesen("bootstrap: Lesefehler oder Frist abgelaufen");
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
            let _ = crate::framing::frame_schreiben(&mut datei, json);
            statistik.geschlossen_bootstrap.fetch_add(1, Ordering::SeqCst);
            senke.abgewiesen("bootstrap: v2-Hello am v3-Endpunkt");
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
                    senke.abgewiesen("bootstrap: zu viele offene Kopplungen");
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
                    if datei.write_all(&frame).is_err() {
                        let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                        k.control_abmelden(&link);
                        return;
                    }
                }
                Err(_) => return,
            }
            senke.control_verbunden(&link, &h);
            (link, true)
        }
        Bootstrap::V3Telemetry(h) => {
            let ok = {
                let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                k.telemetrie_koppeln(&h)
            };
            if let Err(e) = ok {
                statistik.geschlossen_bootstrap.fetch_add(1, Ordering::SeqCst);
                senke.abgewiesen(&format!("telemetry ungekoppelt: {e:?}"));
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
                if datei.write_all(&frame).is_err() {
                    let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                    k.telemetrie_entkoppeln(&h.link_id);
                    return;
                }
            }
            senke.telemetrie_gekoppelt(&h.link_id);
            (h.link_id.clone(), false)
        }
    };

    // ── Ab hier ausschliesslich v3-Frames ─────────────────────────────────
    let mut leser = StromLeser::neu();
    leser.fuettern(&roh);
    let mut ingress: IngressWarteschlange<(Familie, Vec<u8>)> = IngressWarteschlange::neu();
    let mut rate = Ratengrenze::neu(RATE_PRO_SEKUNDE, 1000);
    let beginn = Instant::now();

    loop {
        loop {
            match leser.naechster() {
                LeseErgebnis::Unvollstaendig => break,
                LeseErgebnis::Verstoesse(v) => {
                    statistik.geschlossen_envelope.fetch_add(1, Ordering::SeqCst);
                    let namen: Vec<&str> = v.iter().map(|x| x.name()).collect();
                    senke.abgewiesen(&format!("envelope: {}", namen.join(",")));
                    trennen(&kopplungen, &link_id, ist_control, &senke);
                    return;
                }
                LeseErgebnis::Frame(r) => {
                    let jetzt_ms = beginn.elapsed().as_millis() as u64;
                    if !rate.erlaubt(jetzt_ms) {
                        statistik.geschlossen_rate.fetch_add(1, Ordering::SeqCst);
                        senke.abgewiesen("rate: Nachrichtenratengrenze ueberschritten");
                        trennen(&kopplungen, &link_id, ist_control, &senke);
                        return;
                    }
                    let familie = r.kopf.familie;
                    match ingress.einreihen(familie, (familie, r.payload.clone())) {
                        IngressErgebnis::Eingereiht => {}
                        IngressErgebnis::P2Verworfen => {
                            statistik.ingress_p2_verworfen.fetch_add(1, Ordering::SeqCst);
                        }
                        IngressErgebnis::ClientTrennen => {
                            statistik.geschlossen_p0_ueberlauf.fetch_add(1, Ordering::SeqCst);
                            senke.abgewiesen("ingress: P0-Ueberlauf");
                            trennen(&kopplungen, &link_id, ist_control, &senke);
                            return;
                        }
                        IngressErgebnis::Verworfen => {
                            if familie == Familie::P1 {
                                statistik.ingress_p1_verworfen.fetch_add(1, Ordering::SeqCst);
                            } else {
                                statistik.ingress_p2_verworfen.fetch_add(1, Ordering::SeqCst);
                            }
                        }
                    }
                    // Ingress sofort leeren: der Coordinator kommt erst in
                    // SONDE-011, hier ist die Senke der Abnehmer.
                    while let Some((_, (f, payload))) = ingress.entnehmen() {
                        match f {
                            Familie::P0 => {
                                if let Some(antwort) = senke.p0(&link_id, &payload) {
                                    if let Ok(frame) = envelope_schreiben(Familie::P0, 0, &antwort) {
                                        if datei.write_all(&frame).is_err() {
                                            trennen(&kopplungen, &link_id, ist_control, &senke);
                                            return;
                                        }
                                    }
                                }
                            }
                            Familie::P1 => senke.p1(&link_id, &payload),
                            Familie::P2 => senke.p2(&link_id, &payload),
                        }
                    }
                }
            }
        }

        if stop.load(Ordering::SeqCst) {
            break;
        }
        // Harte Obergrenze fuer den Lesepuffer: ein Peer darf nicht beliebig
        // viele Teilbytes anhaeufen, ohne je einen Frame zu vollenden.
        if leser.offen() > MAX_FRAME_BYTES as usize + 4 {
            statistik.geschlossen_envelope.fetch_add(1, Ordering::SeqCst);
            senke.abgewiesen("envelope: Teilframe ueber der Paketgrenze");
            break;
        }
        match datei.read(&mut puffer) {
            Ok(0) => break,
            Ok(n) => leser.fuettern(&puffer[..n]),
            Err(_) => break,
        }
    }

    trennen(&kopplungen, &link_id, ist_control, &senke);
}

fn trennen(
    kopplungen: &Arc<Mutex<Kopplungen>>,
    link_id: &str,
    ist_control: bool,
    senke: &Arc<dyn Senke>,
) {
    let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
    if ist_control {
        // Die Control-Verbindung besitzt die Kopplung: geht sie, geht auch
        // der Telemetrieplatz. Sonst bliebe eine halb offene Kopplung stehen
        // (Fehlerlexikon: "zwei Pipes halb verbunden").
        if k.control_abmelden(link_id) {
            senke.control_getrennt(link_id);
        }
    } else if k.telemetrie_entkoppeln(link_id) {
        senke.telemetrie_getrennt(link_id);
    }
}
