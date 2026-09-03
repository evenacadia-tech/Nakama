//! Named-Pipe-Server des EQ-Copilot-Brokers (Plan §9.2).
//!
//! Warum roher Win32-Server statt einer Crate: die Pflicht „Pipe-ACL nur für
//! den aktuellen Windows-User" braucht einen eigenen SECURITY_DESCRIPTOR am
//! CreateNamedPipeW — das ist mit windows-sys direkt und prüfbar, mit
//! Wrapper-Crates versteckt. Der Server nimmt Verbindungen blockierend an und
//! bedient jede in einem eigenen Thread; das Pipe-HANDLE wandert in ein
//! std::fs::File (FromRawHandle), damit Lesen/Schreiben und das Framing
//! gewöhnlicher std::io-Code bleiben.

// Nur-Windows-Modul — das `#[cfg(windows)]` sitzt am `mod server;` in mod.rs.

use super::framing::{frame_lesen, frame_schreiben, FrameFehler};
use super::protokoll::{
    hello_pruefen, BrokerNachricht, ClientNachricht, MIN_PROTOKOLL, PROTOKOLL_VERSION,
};
use super::Register;
use std::fs::File;
use std::os::windows::io::{AsRawHandle, FromRawHandle};
use std::sync::atomic::{AtomicBool, AtomicU64, AtomicUsize, Ordering};
use std::sync::{Arc, Mutex};
use std::thread::JoinHandle;

use windows_sys::Win32::Foundation::{
    CloseHandle, GetLastError, LocalFree, ERROR_INSUFFICIENT_BUFFER, ERROR_NOT_FOUND,
    ERROR_PIPE_CONNECTED, HANDLE, INVALID_HANDLE_VALUE,
};
use windows_sys::Win32::Security::Authorization::{
    ConvertSidToStringSidW, ConvertStringSecurityDescriptorToSecurityDescriptorW,
};
use windows_sys::Win32::Security::{
    CopySid, GetLengthSid, GetTokenInformation, IsValidSid, TokenUser, SECURITY_ATTRIBUTES,
    TOKEN_QUERY, TOKEN_USER,
};
use windows_sys::Win32::Storage::FileSystem::{
    FlushFileBuffers, FILE_FLAG_FIRST_PIPE_INSTANCE, PIPE_ACCESS_DUPLEX,
};
use windows_sys::Win32::System::Pipes::{
    ConnectNamedPipe, CreateNamedPipeW, DisconnectNamedPipe, PIPE_READMODE_BYTE,
    PIPE_REJECT_REMOTE_CLIENTS, PIPE_TYPE_BYTE, PIPE_WAIT,
};
use windows_sys::Win32::System::Threading::{GetCurrentProcess, OpenProcessToken};
use windows_sys::Win32::System::IO::CancelSynchronousIo;

/// Der Produktvertrag testet bis 32 Quellen. Der Faktor zwei lässt einen
/// kurzen Reconnect-/Duplikat-Überlapp zu, hält aber Threads, Kernelpuffer und
/// Handles hart begrenzt. Windows erlaubt höchstens 255 Pipe-Instanzen.
const MAX_VERBINDUNGEN: usize = 64;

/// Getrennte Sensoren bleiben im heutigen v2-Register sichtbar. Ohne diese
/// zweite Grenze könnte ein Client trotz Connection-Cap seriell unendlich
/// viele Tombstones erzeugen. Reconnects bekannter IDs bleiben erlaubt.
const MAX_SENSOR_IDS: usize = 128;

/// Reject-Antworten sollen den Client erreichen, dürfen den Server aber nicht
/// auf dessen Read-Verhalten festnageln. Danach wird die Flush-I/O abgebrochen.
const ANTWORT_FLUSH_TIMEOUT: std::time::Duration = std::time::Duration::from_millis(100);

/// Sicherheitsbeschreibung „nur der aktuelle Windows-User" (SDDL `D:P(A;;GA;;;<SID>)`).
/// Hält den LocalAlloc-Deskriptor am Leben, solange der Server läuft.
pub(crate) struct Sicherheit {
    pub(crate) deskriptor: *mut core::ffi::c_void,
    /// Exakte binäre SID, aus der auch der SDDL-Text gebaut wurde. Der v3-
    /// Listener vergleicht jeden impersonierten Client gegen genau diese Bytes.
    sid: Vec<u64>,
}
// SAFETY: `deskriptor` ist ein exklusiv besessener LocalAlloc-Block ohne
// Thread-Affinität; nach dem Bau wird er nur noch gelesen (CreateNamedPipeW)
// und genau einmal in Drop freigegeben — kein geteilter veränderlicher Zustand.
unsafe impl Send for Sicherheit {}
// SAFETY: Nach dem Konstruktor werden Deskriptor und SID nur gelesen. Beide
// Allokationen leben bis zum letzten Besitzer und werden erst dann freigegeben.
unsafe impl Sync for Sicherheit {}

impl Drop for Sicherheit {
    fn drop(&mut self) {
        if !self.deskriptor.is_null() {
            // SAFETY: der Pointer stammt aus einem erfolgreichen
            // ConvertStringSecurityDescriptorToSecurityDescriptorW (LocalAlloc)
            // und wird nur hier, genau einmal, freigegeben.
            unsafe { LocalFree(self.deskriptor) };
        }
    }
}

impl Sicherheit {
    pub(crate) fn user_sid(&self) -> *mut core::ffi::c_void {
        self.sid.as_ptr().cast_mut().cast()
    }
}

/// RAII für ein rohes Win32-HANDLE — schließt bei jedem Ausstieg, auch im
/// Fehlerpfad (P0-Fund: der Thread-Spawn-Fehlerpfad leakte das Pipe-Handle).
struct HandleGuard(HANDLE);

impl HandleGuard {
    /// Übergibt das Handle an einen neuen Besitzer; dieser Guard schließt nicht mehr.
    fn uebernehmen(self) -> HANDLE {
        let h = self.0;
        std::mem::forget(self);
        h
    }
}

impl Drop for HandleGuard {
    fn drop(&mut self) {
        // SAFETY: das Handle stammt aus einem erfolgreichen Win32-Aufruf, der
        // Guard besitzt es exklusiv und schließt es genau einmal.
        unsafe { CloseHandle(self.0) };
    }
}

// SAFETY: Win32-HANDLEs sind prozessweite Kernel-Referenzen ohne Thread-
// Affinität; der Guard besitzt seines exklusiv und gibt es genau einmal frei.
unsafe impl Send for HandleGuard {}

// Kompilezeit-Riegel für den u64-Puffer in aktueller_user_sid(): TOKEN_USER
// darf kein strengeres Alignment verlangen, als u64-Elemente garantieren.
const _: () = assert!(std::mem::align_of::<TOKEN_USER>() <= std::mem::align_of::<u64>());

fn aktueller_user_sid_mit_binaer() -> Result<(String, Vec<u64>), String> {
    let mut token: HANDLE = std::ptr::null_mut();
    // SAFETY: GetCurrentProcess liefert ein Pseudo-Handle (nicht zu schließen);
    // OpenProcessToken schreibt nur bei Erfolg ein gültiges Token nach `token`.
    if unsafe { OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &mut token) } == 0 {
        // SAFETY: GetLastError liest nur den threadlokalen Fehlercode.
        return Err(format!("OpenProcessToken: Win32 {}", unsafe {
            GetLastError()
        }));
    }
    let token = HandleGuard(token);

    let mut noetig: u32 = 0;
    // SAFETY: dokumentierte Größenabfrage — Nullpuffer + Länge 0 führen zu
    // ERROR_INSUFFICIENT_BUFFER, die benötigte Bytezahl landet in `noetig`.
    unsafe { GetTokenInformation(token.0, TokenUser, std::ptr::null_mut(), 0, &mut noetig) };
    // Fehlercode SOFORT sichern — jeder weitere Win32-Aufruf (auch ein
    // CloseHandle im Drop) überschriebe ihn.
    // SAFETY: GetLastError liest nur den threadlokalen Fehlercode.
    let fehler = unsafe { GetLastError() };
    if fehler != ERROR_INSUFFICIENT_BUFFER || noetig == 0 {
        return Err(format!("GetTokenInformation (Größe): Win32 {fehler}"));
    }

    // TOKEN_USER verlangt Pointer-Alignment — ein Vec<u8> (Alignment 1) wäre
    // beim Dereferenzieren Undefined Behavior. u64-Elemente garantieren
    // Alignment 8 (Kompilezeit-Riegel oben); div_ceil rundet auf ganze Elemente.
    let mut puffer = vec![0u64; (noetig as usize).div_ceil(std::mem::size_of::<u64>())];
    // SAFETY: `puffer` ist exklusiv, mindestens `noetig` Bytes groß und wird von
    // der API mit einer TOKEN_USER-Struktur samt dahinter liegender SID gefüllt.
    let ok = unsafe {
        GetTokenInformation(
            token.0,
            TokenUser,
            puffer.as_mut_ptr().cast(),
            noetig,
            &mut noetig,
        )
    };
    // SAFETY: GetLastError liest nur den threadlokalen Fehlercode.
    let fehler = unsafe { GetLastError() };
    drop(token);
    if ok == 0 {
        return Err(format!("GetTokenInformation: Win32 {fehler}"));
    }

    // SAFETY: der Puffer beginnt laut API-Vertrag mit einer gültigen
    // TOKEN_USER-Struktur, das Alignment stimmt (u64-Puffer + Riegel oben),
    // und `User.Sid` zeigt in denselben, weiterhin lebenden Puffer.
    let sid_ptr = unsafe { (*puffer.as_ptr().cast::<TOKEN_USER>()).User.Sid };
    // Die TokenInformation-Pufferadresse darf nicht zur zweiten Wahrheit
    // werden. SID-Bytes einmal kopieren; genau diese Kopie bleibt zusammen mit
    // dem daraus gebauten DACL-Deskriptor im `Sicherheit`-Objekt am Leben.
    // SAFETY: `sid_ptr` zeigt in den noch lebenden TokenInformation-Puffer;
    // IsValidSid liest ihn nur und ist genau die Pruefung, die alle folgenden
    // SID-Aufrufe voraussetzen.
    if unsafe { IsValidSid(sid_ptr) } == 0 {
        return Err("TokenUser enthielt keine gueltige SID".into());
    }
    // SAFETY: die SID ist eine Zeile hoeher als gueltig bestaetigt; GetLengthSid
    // liest nur ihre Laengenfelder.
    let sid_laenge = unsafe { GetLengthSid(sid_ptr) };
    if sid_laenge == 0 {
        return Err("GetLengthSid lieferte 0".into());
    }
    let mut sid_binaer = vec![0u64; (sid_laenge as usize).div_ceil(std::mem::size_of::<u64>())];
    // SAFETY: das Ziel ist auf u64 ausgerichtet und mindestens `sid_laenge`
    // Bytes gross - die Vec-Laenge ist genau darauf aufgerundet; die Quelle ist
    // die geprueft gueltige SID.
    if unsafe { CopySid(sid_laenge, sid_binaer.as_mut_ptr().cast(), sid_ptr) } == 0 {
        // SAFETY: reine Abfrage des threadlokalen Fehlercodes.
        return Err(format!("CopySid: Win32 {}", unsafe { GetLastError() }));
    }
    let sid_ptr = sid_binaer.as_mut_ptr().cast();
    let mut sid_w: *mut u16 = std::ptr::null_mut();
    // SAFETY: `sid_ptr` zeigt in die lebende, unabhaengige SID-Kopie; bei Erfolg alloziert
    // die API einen nullterminierten UTF-16-String nach `sid_w`.
    if unsafe { ConvertSidToStringSidW(sid_ptr, &mut sid_w) } == 0 {
        // SAFETY: GetLastError liest nur den threadlokalen Fehlercode.
        return Err(format!("ConvertSidToStringSid: Win32 {}", unsafe {
            GetLastError()
        }));
    }
    // SAFETY: `sid_w` ist bei Erfolg ein gültiger nullterminierter UTF-16-
    // String; nach der Kopie wird der LocalAlloc-Block genau einmal freigegeben.
    let sid = unsafe {
        let mut len = 0usize;
        while *sid_w.add(len) != 0 {
            len += 1;
        }
        let s = String::from_utf16_lossy(std::slice::from_raw_parts(sid_w, len));
        LocalFree(sid_w.cast());
        s
    };
    Ok((sid, sid_binaer))
}

pub(crate) fn aktueller_user_sid() -> Result<String, String> {
    aktueller_user_sid_mit_binaer().map(|(text, _)| text)
}

pub(crate) fn sicherheit_nur_user() -> Result<Sicherheit, String> {
    let (sid, sid_binaer) = aktueller_user_sid_mit_binaer()?;
    let sddl = format!("D:P(A;;GA;;;{sid})");
    let mut sddl_w: Vec<u16> = sddl.encode_utf16().collect();
    sddl_w.push(0);
    let mut deskriptor: *mut core::ffi::c_void = std::ptr::null_mut();
    // SAFETY: `sddl_w` ist nullterminiert; bei Erfolg alloziert die API den
    // Deskriptor (LocalAlloc), den `Sicherheit` besitzt und in Drop freigibt.
    // 1 = SDDL_REVISION_1. GetLastError liest nur den threadlokalen Fehlercode.
    unsafe {
        if ConvertStringSecurityDescriptorToSecurityDescriptorW(
            sddl_w.as_ptr(),
            1,
            &mut deskriptor,
            std::ptr::null_mut(),
        ) == 0
        {
            return Err(format!("SDDL→Deskriptor: Win32 {}", GetLastError()));
        }
    }
    Ok(Sicherheit {
        deskriptor,
        sid: sid_binaer,
    })
}

fn fehler_merken(register: &Arc<Mutex<Register>>, text: String) {
    if let Ok(mut r) = register.lock() {
        r.fehler_merken(text);
    }
}

fn protokollfehler_merken(register: &Arc<Mutex<Register>>, text: String) {
    if let Ok(mut r) = register.lock() {
        r.paket_verworfen();
        r.fehler_merken(text);
    }
}

fn messstand_protokollfehler_merken(
    register: &Arc<Mutex<Register>>,
    sensor_id: &str,
    nonce: &str,
    text: String,
) {
    if let Ok(mut r) = register.lock() {
        r.messung_verwerfen_von(sensor_id, nonce);
        r.paket_verworfen();
        r.fehler_merken(text);
    }
}

fn join_auswerten(join: JoinHandle<()>, register: &Arc<Mutex<Register>>, art: &str) {
    if join.join().is_err() {
        fehler_merken(register, format!("{art} ist mit Panic beendet"));
    }
}

/// Bricht die synchrone I/O eines besessenen Rust-Threads so lange ab, bis der
/// Thread wirklich beendet ist. Der JoinHandle hält seinen Windows-Threadhandle
/// während des gesamten Vorgangs offen; damit gibt es keinen Check/Close-Race.
fn thread_abbrechen_und_join(join: JoinHandle<()>, register: &Arc<Mutex<Register>>, art: &str) {
    let mut unerwarteter_cancel_fehler = None;
    while !join.is_finished() {
        // SAFETY: der RawHandle gehört weiterhin `join` und bleibt bis zum
        // abschließenden join gültig. CancelSynchronousIo betrifft nur die von
        // genau diesem Thread ausgegebenen synchronen I/O-Operationen.
        let ok = unsafe { CancelSynchronousIo(join.as_raw_handle() as HANDLE) };
        if ok == 0 {
            // ERROR_NOT_FOUND ist der erwartete Übergang zwischen zwei I/Os:
            // Stop ist bereits gesetzt, daher geht der Thread entweder heraus
            // oder betritt eine neue I/O, die der nächste Durchlauf abbricht.
            // SAFETY: reine Abfrage des threadlokalen Fehlercodes, unmittelbar
            // nach dem fehlgeschlagenen Aufruf.
            let fehler = unsafe { GetLastError() };
            if fehler != ERROR_NOT_FOUND && unerwarteter_cancel_fehler.is_none() {
                unerwarteter_cancel_fehler = Some(fehler);
            }
        }
        if !join.is_finished() {
            std::thread::sleep(std::time::Duration::from_millis(1));
        }
    }
    if let Some(fehler) = unerwarteter_cancel_fehler {
        fehler_merken(
            register,
            format!("{art}: CancelSynchronousIo meldete Win32 {fehler}"),
        );
    }
    join_auswerten(join, register, art);
}

fn fertige_verbindungen_einsammeln(
    verbindungen: &Arc<Mutex<Vec<JoinHandle<()>>>>,
    register: &Arc<Mutex<Register>>,
) {
    let fertig = {
        let mut joins = verbindungen.lock().unwrap_or_else(|e| e.into_inner());
        let mut fertig = Vec::new();
        let mut i = 0;
        while i < joins.len() {
            if joins[i].is_finished() {
                fertig.push(joins.swap_remove(i));
            } else {
                i += 1;
            }
        }
        fertig
    };
    for join in fertig {
        join_auswerten(join, register, "Pipe-Verbindungsthread");
    }
}

struct AktiveVerbindung(Arc<AtomicUsize>);

impl Drop for AktiveVerbindung {
    fn drop(&mut self) {
        self.0.fetch_sub(1, Ordering::SeqCst);
    }
}

struct RegistrierteVerbindung {
    register: Arc<Mutex<Register>>,
    sensor_id: String,
    nonce: String,
    interventionssenke: Option<Arc<dyn V2Interventionssenke>>,
    interventions_link_id: String,
}

impl Drop for RegistrierteVerbindung {
    fn drop(&mut self) {
        if let Ok(mut r) = self.register.lock() {
            r.sensor_trennen(&self.sensor_id, &self.nonce);
        }
        if let Some(senke) = &self.interventionssenke {
            // H-05: der Senkenaufruf ist fremder Code und laeuft hier im
            // Abwickeln. Panisiert er, waere das eine zweite Panik im Drop und
            // damit ein sofortiger Prozessabbruch, der den geordneten Stopp
            // uebergeht. Zusammen mit H-04 - vergiftungstolerante Sperrgriffe
            // im Coordinator - kann eine einzelne Panik den Broker damit nicht
            // mehr dauerhaft ausfallen lassen.
            let ergebnis = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
                senke.getrennt(&self.interventions_link_id);
            }));
            if ergebnis.is_err() {
                SENKE_PANIK_IM_DESTRUKTOR.fetch_add(1, Ordering::SeqCst);
            }
        }
    }
}

/// Wie oft eine panische Interventionssenke im Destruktor gefasst wurde. Im
/// Normalbetrieb strukturell null; ein Wert groesser null ist der sichtbare
/// Beleg, dass H-05 gegriffen hat, statt den Prozess mitzureissen.
pub static SENKE_PANIK_IM_DESTRUKTOR: AtomicU64 = AtomicU64::new(0);

/// Schmale Bruecke vom bestehenden v2-Server in den produktiven
/// Coordinator. Das Register bleibt v2-Sichtmodell; allein diese Senke macht
/// den Interventionsriegel fuer v2 und v3 zu EINER Wahrheit.
pub trait V2Interventionssenke: Send + Sync {
    fn hoermarkierung(&self, link_id: &str, aktiv: bool);
    fn getrennt(&self, link_id: &str);
}

pub struct ServerGriff {
    stop: Arc<AtomicBool>,
    join: Option<JoinHandle<()>>,
    verbindungen: Arc<Mutex<Vec<JoinHandle<()>>>>,
    register: Arc<Mutex<Register>>,
}

impl ServerGriff {
    pub fn stoppen(&mut self) {
        self.stop.store(true, Ordering::SeqCst);
        if let Some(join) = self.join.take() {
            thread_abbrechen_und_join(join, &self.register, "Pipe-Acceptorthread");
        }

        // Erst nach dem Acceptor-Join kann kein neuer Connection-Thread mehr
        // in die Registry gelangen. Danach jede blockierende Read/Write-I/O
        // abbrechen und jeden Thread wirklich joinen.
        let joins = {
            let mut verbindungen = self.verbindungen.lock().unwrap_or_else(|e| e.into_inner());
            std::mem::take(&mut *verbindungen)
        };
        for join in joins {
            thread_abbrechen_und_join(join, &self.register, "Pipe-Verbindungsthread");
        }
    }
}

impl Drop for ServerGriff {
    fn drop(&mut self) {
        self.stoppen();
    }
}

/// Startet den Server auf `pipe_name` (z. B. `\\.\pipe\evenacadia.eq-copilot.v1`).
/// Jeder Fehler beim Anlegen einer Pipe-Instanz landet sichtbar im Register —
/// nichts scheitert still (Plan §9.2).
///
/// Die ERSTE Instanz wird mit FILE_FLAG_FIRST_PIPE_INSTANCE angelegt: hält ein
/// anderer Prozess den Namen bereits (zweite App, Probe neben laufender App,
/// Zombie), scheitert der Start SOFORT SICHTBAR — sonst stähle jeder Broker
/// dem anderen still die Hälfte der Sensoren (M2-Fund: genau das passierte
/// der Ende-zu-Ende-Probe neben der laufenden Hub-App).
pub fn server_starten(
    pipe_name: &str,
    register: Arc<Mutex<Register>>,
    broker_version: String,
    session_token: String,
) -> Result<ServerGriff, String> {
    server_starten_mit_grenzen(
        pipe_name,
        register,
        broker_version,
        session_token,
        MAX_VERBINDUNGEN,
        MAX_SENSOR_IDS,
        None,
    )
}

pub fn server_starten_mit_interventionssenke(
    pipe_name: &str,
    register: Arc<Mutex<Register>>,
    broker_version: String,
    session_token: String,
    interventionssenke: Arc<dyn V2Interventionssenke>,
) -> Result<ServerGriff, String> {
    server_starten_mit_grenzen(
        pipe_name,
        register,
        broker_version,
        session_token,
        MAX_VERBINDUNGEN,
        MAX_SENSOR_IDS,
        Some(interventionssenke),
    )
}

fn server_starten_mit_grenzen(
    pipe_name: &str,
    register: Arc<Mutex<Register>>,
    broker_version: String,
    session_token: String,
    max_verbindungen: usize,
    max_sensor_ids: usize,
    interventionssenke: Option<Arc<dyn V2Interventionssenke>>,
) -> Result<ServerGriff, String> {
    if !(1..=255).contains(&max_verbindungen) {
        return Err(format!(
            "ungültige Pipe-Instanzgrenze {max_verbindungen} (erlaubt: 1..=255)"
        ));
    }
    if max_sensor_ids == 0 {
        return Err("ungültige Sensor-ID-Grenze 0".into());
    }

    let sicherheit = sicherheit_nur_user()?;
    let stop = Arc::new(AtomicBool::new(false));
    let stop2 = stop.clone();
    let verbindungen = Arc::new(Mutex::new(Vec::new()));
    let verbindungen2 = verbindungen.clone();
    let aktive_verbindungen2 = Arc::new(AtomicUsize::new(0));
    let register2 = register.clone();
    let interventionssenke2 = interventionssenke.clone();
    let name = pipe_name.to_string();
    let mut name_w: Vec<u16> = name.encode_utf16().collect();
    name_w.push(0);

    // Erste Instanz synchron — der Belegt-Fehler gehört dem Aufrufer.
    let attrs = SECURITY_ATTRIBUTES {
        nLength: std::mem::size_of::<SECURITY_ATTRIBUTES>() as u32,
        lpSecurityDescriptor: sicherheit.deskriptor,
        bInheritHandle: 0,
    };
    // SAFETY: `name_w` ist nullterminiert, `attrs` lebt über den Aufruf, der
    // Deskriptor darin gehört `sicherheit` und lebt so lange wie der Server.
    let erstes_handle = unsafe {
        CreateNamedPipeW(
            name_w.as_ptr(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
            max_verbindungen as u32,
            65536,
            65536,
            0,
            &attrs,
        )
    };
    if erstes_handle == INVALID_HANDLE_VALUE {
        // SAFETY: GetLastError liest nur den threadlokalen Fehlercode.
        let fehler = unsafe { GetLastError() };
        return Err(if fehler == 5 {
            format!(
                "Pipe {name} ist schon belegt — läuft bereits ein EQ-Copilot-Broker \
                 (zweite App-Instanz oder Probe neben laufender App)? Win32 5"
            )
        } else {
            format!("CreateNamedPipe (erste Instanz): Win32 {fehler}")
        });
    }
    // RAII ab der ersten Sekunde: scheitert unten der Thread-Spawn, schließt
    // der Guard das Handle — vorher leakte genau dieser Fehlerpfad die Pipe.
    let erstes_handle = HandleGuard(erstes_handle);

    let join = std::thread::Builder::new()
        .name("eqcop-pipe-server".into())
        .spawn(move || {
            // Edition-2021-Detail: ohne diese Zeile capturete die Closure nur
            // das rohe Pointer-FELD (nicht Send) statt der ganzen Sicherheit-
            // Struct — und ihr Drop liefe nie.
            let sicherheit = sicherheit;
            let attrs = SECURITY_ATTRIBUTES {
                nLength: std::mem::size_of::<SECURITY_ATTRIBUTES>() as u32,
                lpSecurityDescriptor: sicherheit.deskriptor,
                bInheritHandle: 0,
            };
            let mut naechstes_handle = Some(erstes_handle);

            loop {
                fertige_verbindungen_einsammeln(&verbindungen2, &register2);
                if stop2.load(Ordering::SeqCst) {
                    break;
                }
                if aktive_verbindungen2.load(Ordering::SeqCst) >= max_verbindungen {
                    // Bei voller Kapazität wird bewusst keine weitere lauschende
                    // Instanz erzeugt. Kurzes Polling hält Stop deterministisch,
                    // ohne einen weiteren untrusted Client zur Weckhilfe zu brauchen.
                    std::thread::sleep(std::time::Duration::from_millis(5));
                    continue;
                }

                let handle = match naechstes_handle.take() {
                    Some(h) => h,
                    // SAFETY: wie beim ersten CreateNamedPipeW — `name_w` ist
                    // nullterminiert, `attrs`/Deskriptor leben im Thread weiter.
                    None => {
                        let h = unsafe {
                            CreateNamedPipeW(
                                name_w.as_ptr(),
                                PIPE_ACCESS_DUPLEX,
                                PIPE_TYPE_BYTE
                                    | PIPE_READMODE_BYTE
                                    | PIPE_WAIT
                                    | PIPE_REJECT_REMOTE_CLIENTS,
                                max_verbindungen as u32,
                                65536,
                                65536,
                                0,
                                &attrs,
                            )
                        };
                        if h == INVALID_HANDLE_VALUE {
                            // SAFETY: GetLastError liest nur den threadlokalen
                            // Fehlercode, unmittelbar nach CreateNamedPipeW.
                            let fehler = unsafe { GetLastError() };
                            fehler_merken(&register2, format!("CreateNamedPipe: Win32 {fehler}"));
                            std::thread::sleep(std::time::Duration::from_millis(50));
                            continue;
                        }
                        HandleGuard(h)
                    }
                };

                // SAFETY: `handle.0` ist die eben angelegte, exklusive Pipe-Instanz.
                let verbunden = unsafe { ConnectNamedPipe(handle.0, std::ptr::null_mut()) };
                // SAFETY: GetLastError liest nur den threadlokalen Fehlercode
                // — direkt nach ConnectNamedPipe, vor jedem weiteren Aufruf.
                let verbinde_fehler = if verbunden == 0 {
                    unsafe { GetLastError() }
                } else {
                    0
                };
                if verbunden == 0 && verbinde_fehler != ERROR_PIPE_CONNECTED {
                    if !stop2.load(Ordering::SeqCst) {
                        fehler_merken(
                            &register2,
                            format!("ConnectNamedPipe: Win32 {verbinde_fehler}"),
                        );
                    }
                    continue; // Guard schließt das Handle
                }
                if stop2.load(Ordering::SeqCst) {
                    break; // Guard schließt das Handle
                }

                // SAFETY: der Guard übergibt sein exklusives, gültiges Handle
                // an das File — ab hier schließt dessen Drop genau einmal.
                let datei = unsafe { File::from_raw_handle(handle.uebernehmen() as _) };
                let reg = register2.clone();
                let bv = broker_version.clone();
                let tok = session_token.clone();
                let conn_stop = stop2.clone();
                let aktiv = aktive_verbindungen2.clone();
                let interventionssenke = interventionssenke2.clone();
                aktive_verbindungen2.fetch_add(1, Ordering::SeqCst);
                match std::thread::Builder::new()
                    .name("eqcop-pipe-conn".into())
                    .spawn(move || {
                        let _aktiv = AktiveVerbindung(aktiv);
                        verbindung_bedienen(
                            datei,
                            reg,
                            bv,
                            tok,
                            conn_stop,
                            max_sensor_ids,
                            interventionssenke,
                        );
                    }) {
                    Ok(join) => verbindungen2
                        .lock()
                        .unwrap_or_else(|e| e.into_inner())
                        .push(join),
                    Err(e) => {
                        // Der Closure-Drop schließt `datei`; nur der vor dem
                        // Spawn reservierte Zähler muss hier zurückgenommen werden.
                        aktive_verbindungen2.fetch_sub(1, Ordering::SeqCst);
                        fehler_merken(
                            &register2,
                            format!("Pipe-Verbindungsthread konnte nicht starten: {e}"),
                        );
                    }
                }
            }

            fertige_verbindungen_einsammeln(&verbindungen2, &register2);
        })
        .map_err(|e| format!("Serverthread: {e}"))?;

    Ok(ServerGriff {
        stop,
        join: Some(join),
        verbindungen,
        register,
    })
}

fn verbindung_bedienen(
    mut datei: File,
    register: Arc<Mutex<Register>>,
    broker_version: String,
    session_token: String,
    stop: Arc<AtomicBool>,
    max_sensor_ids: usize,
    interventionssenke: Option<Arc<dyn V2Interventionssenke>>,
) {
    // Erstes Paket muss ein gültiges hello sein — sonst Verbindung beenden.
    let erster = match frame_lesen(&mut datei) {
        Ok(json) => json,
        Err(FrameFehler::Ende) => return,
        Err(e) => {
            if !stop.load(Ordering::SeqCst) {
                fehler_merken(&register, format!("Verbindung vor hello beendet: {e:?}"));
            }
            return;
        }
    };
    let hello = match serde_json::from_str::<ClientNachricht>(&erster) {
        Ok(ClientNachricht::Hello(h)) => h,
        _ => {
            protokollfehler_merken(&register, "erstes Paket war kein gültiges hello".into());
            return;
        }
    };

    let mut antwort = hello_pruefen(&hello, &broker_version, &session_token);
    if matches!(antwort, BrokerNachricht::Reject { .. }) {
        if let Ok(json) = serde_json::to_string(&antwort) {
            if frame_schreiben(&mut datei, &json).is_ok() {
                pipe_nach_antwort_schliessen(&datei, &register);
                return;
            }
        }
        pipe_schliessen(&datei);
        return;
    }

    // Effektive Verbindungs-Nonce: die des v2-Clients oder eine server-
    // vergebene für v1 — damit die Konfliktzählung (Plan §8.4) einheitlich
    // über lebende Verbindungen läuft, nicht über ein einzelnes Flag.
    let nonce = hello
        .sensor
        .instance_nonce
        .clone()
        .unwrap_or_else(|| uuid::Uuid::new_v4().to_string());
    // v1-Clients erwarten keine ACKs — ihnen darf der Broker nichts senden.
    let version = hello.protocol_version;
    let sensor_id = hello.sensor.sensor_id.clone();

    // Prüfung und Eintrag sind unter demselben Lock atomar. Bekannte IDs dürfen
    // reconnecten; neue IDs werden ab der festen Registergrenze abgelehnt.
    let ablehnung = match register.lock() {
        Ok(mut r) => {
            // H-13: der Sweep laeuft IM Gate, unter demselben Lock wie die
            // Grenzpruefung und VOR ihr. Einen periodischen v2-Tick gibt es
            // nicht; ohne diesen Weg hatte der Pool ueberhaupt keine Rueckgabe.
            r.getrennte_ids_freigeben_zu(crate::jetzt_ms());
            if r.verbindung_ist_lebend(&sensor_id, &nonce) {
                Some(format!(
                    "Instanz {nonce} für Sensor {sensor_id} ist bereits verbunden"
                ))
            } else if !r.sensoren.contains_key(&sensor_id) && r.sensoren.len() >= max_sensor_ids {
                // Der harte Reject bleibt, wenn die Frist keinen Platz frei
                // gemacht hat. Paragraph 4 sieht hier zusaetzlich eine
                // FRISTLOSE Verdraengung des am laengsten Getrennten vor; sie
                // ist nicht umgesetzt, weil sie die Aussage des bestehenden
                // Tests sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten
                // umkehren wuerde - dort sind beide Sensoren gerade erst
                // getrennt, und der Test sagt „die Grenze ist hart". Der
                // Gate-Text steht ueber der Matrixzeile: bestehende Tests
                // behalten ihre Aussage. Begruendung im Bau-Verlauf des
                // Manifests.
                Some(format!("Sensor-ID-Grenze von {max_sensor_ids} erreicht"))
            } else {
                r.sensor_verbinden(&hello, &nonce);
                None
            }
        }
        Err(_) => return,
    };
    if let Some(reason) = ablehnung {
        antwort = BrokerNachricht::Reject {
            reason,
            min_protocol: MIN_PROTOKOLL,
            max_protocol: PROTOKOLL_VERSION,
        };
        if let Ok(json) = serde_json::to_string(&antwort) {
            if frame_schreiben(&mut datei, &json).is_ok() {
                pipe_nach_antwort_schliessen(&datei, &register);
                return;
            }
        }
        pipe_schliessen(&datei);
        return;
    }
    let registrierung = RegistrierteVerbindung {
        register: register.clone(),
        sensor_id: sensor_id.clone(),
        nonce: nonce.clone(),
        interventionssenke: interventionssenke.clone(),
        interventions_link_id: format!("v2:{}:{sensor_id}:{nonce}", sensor_id.len()),
    };

    let json = match serde_json::to_string(&antwort) {
        Ok(json) => json,
        Err(_) => return,
    };
    if frame_schreiben(&mut datei, &json).is_err() {
        pipe_schliessen(&datei);
        return;
    }

    // Established ist strikt an genau dieses hello, seine Sensor-ID und das
    // ausgegebene Session-Token gebunden. Ein Protokollbruch beendet die
    // Verbindung, damit ein synchron wartender Client nicht ohne Antwort hängt.
    let mut letzte_seq = None;
    while !stop.load(Ordering::SeqCst) {
        match frame_lesen(&mut datei) {
            Ok(json) => match serde_json::from_str::<ClientNachricht>(&json) {
                Ok(ClientNachricht::Heartbeat(hb)) => {
                    if hb.session_token != session_token {
                        messstand_protokollfehler_merken(
                            &register,
                            &sensor_id,
                            &nonce,
                            format!("Heartbeat mit fremdem Session-Token von {sensor_id}"),
                        );
                        break;
                    }
                    if !hb.ist_gueltig() {
                        messstand_protokollfehler_merken(
                            &register,
                            &sensor_id,
                            &nonce,
                            format!(
                                "Heartbeat-Felder von {sensor_id} liegen außerhalb des Vertrags"
                            ),
                        );
                        break;
                    }
                    if letzte_seq.is_some_and(|vorher| hb.seq <= vorher) {
                        messstand_protokollfehler_merken(
                            &register,
                            &sensor_id,
                            &nonce,
                            format!(
                                "Heartbeat-Sequenz von {sensor_id} lief zurück oder wurde wiederholt"
                            ),
                        );
                        break;
                    }
                    letzte_seq = Some(hb.seq);
                    // Schema-Guards, die serde nicht abdeckt (LTAS-Länge,
                    // Wertebereiche, Zustands-Enum): Ein ungültiger Messstand
                    // beendet die Session. ACK/Last-Seen mit stehen gelassener
                    // alter Evidenz wäre eine gefährliche Teilerfolg-Semantik.
                    if version < 2 && hb.measurement.is_some() {
                        messstand_protokollfehler_merken(
                            &register,
                            &sensor_id,
                            &nonce,
                            format!("v1-Heartbeat von {sensor_id} enthielt v2-Messdaten"),
                        );
                        break;
                    }
                    if hb.measurement.as_ref().is_some_and(|m| !m.ist_gueltig()) {
                        messstand_protokollfehler_merken(
                            &register,
                            &sensor_id,
                            &nonce,
                            format!("ungültiger Messstand von {sensor_id}"),
                        );
                        break;
                    }
                    let hoermarkierung = hb.measurement.as_ref().map(|m| m.hoermarkierung);
                    let konflikt = match register.lock() {
                        Ok(mut r) => {
                            r.heartbeat(&sensor_id, &nonce, hb.stats, hb.measurement);
                            r.konflikt_von(&sensor_id)
                        }
                        Err(_) => false,
                    };
                    if let (Some(senke), Some(aktiv)) =
                        (&registrierung.interventionssenke, hoermarkierung)
                    {
                        senke.hoermarkierung(&registrierung.interventions_link_id, aktiv);
                    }
                    if version >= 2 {
                        let ack = BrokerNachricht::HeartbeatAck {
                            seq: hb.seq,
                            konflikt,
                        };
                        let json = serde_json::to_string(&ack).unwrap_or_default();
                        if frame_schreiben(&mut datei, &json).is_err() {
                            break;
                        }
                    }
                }
                Ok(ClientNachricht::Bye(bye)) => {
                    if bye.session_token.as_deref() != Some(session_token.as_str()) {
                        messstand_protokollfehler_merken(
                            &register,
                            &sensor_id,
                            &nonce,
                            format!("Bye mit fehlendem oder fremdem Session-Token von {sensor_id}"),
                        );
                    }
                    break;
                }
                Ok(ClientNachricht::Hello(_)) => {
                    messstand_protokollfehler_merken(
                        &register,
                        &sensor_id,
                        &nonce,
                        format!("zweites hello in Established-Verbindung von {sensor_id}"),
                    );
                    break;
                }
                Err(_) => {
                    messstand_protokollfehler_merken(
                        &register,
                        &sensor_id,
                        &nonce,
                        format!("ungültige Established-Nachricht von {sensor_id}"),
                    );
                    break;
                }
            },
            Err(FrameFehler::Ende) => break,
            Err(e) => {
                if !stop.load(Ordering::SeqCst) {
                    fehler_merken(&register, format!("Verbindung {sensor_id}: {e:?}"));
                }
                break;
            }
        }
    }

    drop(registrierung);
    // H-08: der v2-Abbau geht denselben Weg wie die Reject-Pfade dieser Datei -
    // beschraenkter Flush mit harter Frist, dann trennen, dann schliessen.
    // Vorher schloss er bar, und eine letzte Antwort, die noch im
    // Ausgabepuffer lag, verschwand still. `pipe_nach_antwort_schliessen`
    // enthaelt genau dieses Muster und schliesst danach selbst.
    pipe_nach_antwort_schliessen(&datei, &register);
}

fn pipe_schliessen(datei: &File) {
    // Disconnect beendet die Instanz; CloseHandle übernimmt der File-Drop.
    // SAFETY: `datei` lebt ueber den ganzen Aufruf, ihr Rohhandle ist damit
    // gueltig, und DisconnectNamedPipe schliesst es nicht.
    unsafe {
        let h = datei.as_raw_handle() as HANDLE;
        DisconnectNamedPipe(h);
    }
}

fn pipe_nach_antwort_schliessen(datei: &File, register: &Arc<Mutex<Register>>) {
    // FlushFileBuffers ist bei Named Pipes synchron und wartet auf den Client.
    // Deshalb läuft nur der seltene Reject-Flush in einem eigenen Thread und
    // wird nach einer harten Frist über dessen stabiles Threadhandle abgebrochen.
    let flush_datei = match datei.try_clone() {
        Ok(datei) => datei,
        Err(e) => {
            fehler_merken(register, format!("Pipe-Handle für Antwort-Flush: {e}"));
            pipe_schliessen(datei);
            return;
        }
    };
    match std::thread::Builder::new()
        .name("eqcop-pipe-flush".into())
        // Das Cancel-und-Join-Protokoll unten haelt die Soundness: der Thread
        // wird abgebrochen, nie sein Handle unter ihm entzogen.
        // SAFETY: `flush_datei` ist ein eigener geklonter Handle, den allein
        // dieser Thread besitzt; er lebt bis zu seinem Ende.
        .spawn(move || unsafe {
            FlushFileBuffers(flush_datei.as_raw_handle() as HANDLE);
        }) {
        Ok(join) => {
            let deadline = std::time::Instant::now() + ANTWORT_FLUSH_TIMEOUT;
            while !join.is_finished() && std::time::Instant::now() < deadline {
                std::thread::sleep(std::time::Duration::from_millis(1));
            }
            if join.is_finished() {
                join_auswerten(join, register, "Pipe-Antwort-Flushthread");
            } else {
                thread_abbrechen_und_join(join, register, "Pipe-Antwort-Flushthread");
            }
        }
        Err(e) => fehler_merken(
            register,
            format!("Pipe-Antwort-Flushthread konnte nicht starten: {e}"),
        ),
    }
    pipe_schliessen(datei);
}

#[cfg(test)]
mod tests {
    use super::super::protokoll::PROTOKOLL_VERSION;
    use super::*;
    use std::io::Write;

    /// NAK-121 H-05: eine panische Interventionssenke verlaesst den Destruktor
    /// von `RegistrierteVerbindung` nicht. Ohne die Fassung waere das eine
    /// zweite Panik im Abwickeln und damit ein sofortiger Prozessabbruch, der
    /// den geordneten Stopp uebergeht - und zwar an einer Stelle, die im
    /// Normalbetrieb bei jedem v2-Verbindungsende laeuft.
    struct PanischeSenke;

    impl V2Interventionssenke for PanischeSenke {
        fn getrennt(&self, _link_id: &str) {
            panic!("absichtliche Panik in der Interventionssenke");
        }
        fn hoermarkierung(&self, _link_id: &str, _aktiv: bool) {}
    }

    #[test]
    fn panische_senke_verlaesst_den_v2_destruktor_nicht() {
        let vorher = SENKE_PANIK_IM_DESTRUKTOR.load(Ordering::SeqCst);
        let register = Arc::new(Mutex::new(Register::default()));
        // Die Panik faellt IM Destruktor. Sie darf den Test nicht abbrechen -
        // genau das ist die Zusage.
        let voriger_haken = std::panic::take_hook();
        std::panic::set_hook(Box::new(|_| {}));
        {
            let _verbindung = RegistrierteVerbindung {
                register: register.clone(),
                sensor_id: "s-h05".into(),
                nonce: "n-h05".into(),
                interventionssenke: Some(Arc::new(PanischeSenke)),
                interventions_link_id: "v2:h05".into(),
            };
        }
        std::panic::set_hook(voriger_haken);
        assert_eq!(
            SENKE_PANIK_IM_DESTRUKTOR.load(Ordering::SeqCst),
            vorher + 1,
            "die Panik der Senke wurde nicht gefasst und gezaehlt"
        );
        // Und der Registerteil des Destruktors lief davor wie immer.
        assert!(register.lock().unwrap().sensoren.is_empty());
    }

    /// NAK-121 H-08, v2-Haelfte: der Abbau einer etablierten Verbindung geht
    /// ueber `pipe_nach_antwort_schliessen` - beschraenkter Flush mit harter
    /// Frist, dann trennen, dann schliessen -, nicht ueber das bare
    /// `pipe_schliessen`. Vorher verschwand eine letzte Antwort, die noch im
    /// Ausgabepuffer lag, still.
    ///
    /// Gemessen am Quelltext, weil ein Verhaltenstest dafuer einen Peer
    /// braeuchte, der nachweislich nicht liest, und die Frist von 250 ms in
    /// jedem Lauf abwarten muesste. Die Zusage IST hier die Wahl der
    /// Schliessfunktion am Ende von `verbindung_bedienen`.
    #[test]
    fn v2_abbau_flusht_die_letzte_antwort() {
        let quelle = include_str!("server.rs");
        // Die Marke wird zusammengesetzt, sonst faende rfind das Literal in
        // DIESEM Test statt der echten Codestelle - genau daran fiel die erste
        // Fassung im Rotbeweis auf.
        let marke = ["drop(registri", "erung);"].concat();
        let stelle = quelle
            .rfind(&marke)
            .expect("Abbau am Ende der Verbindung");
        // Nur bis zum Ende DIESER Funktion schauen, und nach dem AUFRUF suchen:
        // der blosse Name traefe auch die Definition, die gleich darunter
        // steht, und die Wache waere wertlos. Genau daran fiel ihre erste
        // Fassung im Rotbeweis auf.
        let rest = &quelle[stelle..];
        let ende = rest.find("
fn ").unwrap_or(rest.len());
        let rumpf = &rest[..ende];
        let flush = rumpf
            .find(&["pipe_nach_antwort_schl", "iessen(&datei"].concat())
            .expect("der v2-Abbau schliesst ohne beschraenkten Flush");
        if let Some(bar) = rumpf.find(&["pipe_schli", "essen(&datei)"].concat()) {
            assert!(
                flush < bar,
                "der v2-Abbau schliesst bar, bevor er abfliessen laesst"
            );
        }
    }

    fn test_pipe_name(zusatz: &str) -> String {
        format!(
            r"\\.\pipe\evenacadia.eq-copilot.test.{}.{}",
            std::process::id(),
            zusatz
        )
    }

    fn client_oeffnen(name: &str) -> File {
        // Der Broker braucht einen Moment bis zur ersten Pipe-Instanz.
        for _ in 0..50 {
            if let Ok(f) = std::fs::OpenOptions::new()
                .read(true)
                .write(true)
                .open(name)
            {
                return f;
            }
            std::thread::sleep(std::time::Duration::from_millis(20));
        }
        panic!("Pipe {name} nicht erreichbar");
    }

    fn hello_json(version: u32, sensor_id: &str) -> String {
        format!(
            r#"{{"type":"hello","protocol_version":{version},"plugin_version":"0.1.0","host_pid":7,
            "sensor":{{"sensor_id":"{sensor_id}","role":"post","label":"PIANO POST","pair_id":"a"}},
            "audio":{{"samplerate":48000.0,"block_size":512,"channels":2}}}}"#
        )
    }

    fn warte_auf<F: Fn(&Register) -> bool>(reg: &Arc<Mutex<Register>>, pruefung: F) -> bool {
        for _ in 0..100 {
            if pruefung(&reg.lock().unwrap()) {
                return true;
            }
            std::thread::sleep(std::time::Duration::from_millis(20));
        }
        false
    }

    #[test]
    fn user_sid_ist_wohlgeformt() {
        // P0-Abnahme: der alignment-sichere Puffer liefert weiterhin eine
        // echte SID (Format S-1-…) — der Pfad läuft sonst nur beim Serverstart.
        let sid = aktueller_user_sid().expect("SID muss lesbar sein");
        assert!(sid.starts_with("S-1-"), "SID war: {sid}");
    }

    #[test]
    fn handshake_heartbeat_und_geordneter_abschied() {
        let name = test_pipe_name("a");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten(&name, reg.clone(), "test".into(), "token-1".into()).unwrap();

        let mut client = client_oeffnen(&name);
        frame_schreiben(&mut client, &hello_json(PROTOKOLL_VERSION, "s-eins")).unwrap();
        let antwort = frame_lesen(&mut client).unwrap();
        assert!(
            antwort.contains(r#""type":"welcome""#),
            "Antwort war: {antwort}"
        );
        assert!(antwort.contains("token-1"));

        frame_schreiben(
            &mut client,
            r#"{"type":"heartbeat","session_token":"token-1","seq":1,
                "stats":{"rms_l":0.25,"rms_r":0.2,"frames_dropped":3,"nan_seen":false}}"#,
        )
        .unwrap();
        // hello_json spricht v2 (aktuelle Version) — jeder Heartbeat wird quittiert.
        let ack = frame_lesen(&mut client).unwrap();
        assert!(
            ack.contains(r#""type":"heartbeat_ack""#),
            "Antwort war: {ack}"
        );
        assert!(warte_auf(&reg, |r| {
            r.sensoren
                .get("s-eins")
                .is_some_and(|s| s.verbunden && s.heartbeats == 1)
        }));
        {
            let r = reg.lock().unwrap();
            let s = &r.sensoren["s-eins"];
            assert_eq!(s.role, "post");
            assert_eq!(s.stats.frames_dropped, 3);
        }

        frame_schreiben(&mut client, r#"{"type":"bye","session_token":"token-1"}"#).unwrap();
        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-eins").is_some_and(|s| !s.verbunden)
        }));
        // Getrennt heißt sichtbar getrennt — nie still entfernt (Plan §7.9-Sensorik).
        assert!(reg.lock().unwrap().sensoren["s-eins"]
            .getrennt_seit_ms
            .is_some());

        griff.stoppen();
    }

    #[test]
    fn falsche_protokollversion_bekommt_reject() {
        let name = test_pipe_name("b");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff = server_starten(&name, reg, "test".into(), "tok".into()).unwrap();

        let mut client = client_oeffnen(&name);
        frame_schreiben(&mut client, &hello_json(99, "s-zwei")).unwrap();
        let antwort = frame_lesen(&mut client).unwrap();
        assert!(
            antwort.contains(r#""type":"reject""#),
            "Antwort war: {antwort}"
        );
        assert!(antwort.contains(r#""min_protocol":1"#));
        assert!(antwort.contains(r#""max_protocol":2"#));

        griff.stoppen();
    }

    #[test]
    fn feindliches_laengenpraefix_beendet_nur_diese_verbindung() {
        let name = test_pipe_name("c");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff = server_starten(&name, reg.clone(), "test".into(), "tok".into()).unwrap();

        // Böser Client: 4-GiB-Ankündigung. Der Server darf weder allozieren noch fallen.
        let mut boese = client_oeffnen(&name);
        boese.write_all(&[0xff, 0xff, 0xff, 0xff]).unwrap();
        boese.flush().unwrap();
        drop(boese);

        // Ein zweiter, ehrlicher Client funktioniert danach normal.
        let mut client = client_oeffnen(&name);
        frame_schreiben(&mut client, &hello_json(PROTOKOLL_VERSION, "s-drei")).unwrap();
        let antwort = frame_lesen(&mut client).unwrap();
        assert!(antwort.contains(r#""type":"welcome""#));
        assert!(warte_auf(&reg, |r| r.sensoren.contains_key("s-drei")));

        griff.stoppen();
    }

    #[test]
    fn doppelte_sensor_id_wird_als_konflikt_sichtbar() {
        let name = test_pipe_name("d");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff = server_starten(&name, reg.clone(), "test".into(), "tok".into()).unwrap();

        // v1-Clients ohne eigene Nonce — der Server vergibt pro Verbindung eine.
        let mut c1 = client_oeffnen(&name);
        frame_schreiben(&mut c1, &hello_json(1, "s-dup")).unwrap();
        let _ = frame_lesen(&mut c1).unwrap();
        assert!(warte_auf(&reg, |r| r.sensoren.contains_key("s-dup")));

        let mut c2 = client_oeffnen(&name);
        frame_schreiben(&mut c2, &hello_json(1, "s-dup")).unwrap();
        let _ = frame_lesen(&mut c2).unwrap();
        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-dup").is_some_and(|s| s.konflikt)
        }));

        // Eine Verbindung endet → Konflikt vorbei, Sensor bleibt verbunden.
        drop(c2);
        assert!(warte_auf(&reg, |r| {
            r.sensoren
                .get("s-dup")
                .is_some_and(|s| !s.konflikt && s.verbunden)
        }));

        griff.stoppen();
    }

    #[test]
    fn zweiter_broker_auf_demselben_namen_scheitert_sichtbar() {
        let name = test_pipe_name("g");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff = server_starten(&name, reg, "test".into(), "tok".into()).unwrap();

        // M2-Fund: ohne FIRST_PIPE_INSTANCE stähle ein zweiter Broker still
        // die Hälfte der Clients — jetzt ist es ein klarer Startfehler.
        let reg2 = Arc::new(Mutex::new(Register::default()));
        let fehler = server_starten(&name, reg2, "test".into(), "tok2".into())
            .err()
            .expect("zweiter Broker muss scheitern");
        assert!(fehler.contains("belegt"), "Fehler war: {fehler}");

        griff.stoppen();
    }

    fn hello_v2_json(sensor_id: &str, nonce: &str, role: &str, pair: &str) -> String {
        format!(
            r#"{{"type":"hello","protocol_version":2,"plugin_version":"0.2.0","host_pid":7,
            "sensor":{{"sensor_id":"{sensor_id}","instance_nonce":"{nonce}","role":"{role}","label":"L {sensor_id}","pair_id":"{pair}"}},
            "audio":{{"samplerate":48000.0,"block_size":512,"channels":2}}}}"#
        )
    }

    fn verbinde_v2(name: &str, token: &str, sensor_id: &str, nonce: &str) -> File {
        let mut client = client_oeffnen(name);
        frame_schreiben(
            &mut client,
            &hello_v2_json(sensor_id, nonce, "sensor", "paar-test"),
        )
        .unwrap();
        let welcome = frame_lesen(&mut client).unwrap();
        assert!(
            welcome.contains(r#""type":"welcome""#),
            "Antwort war: {welcome}"
        );
        assert!(welcome.contains(token), "Antwort war: {welcome}");
        client
    }

    fn sende_heartbeat(client: &mut File, token: &str, seq: u64) {
        frame_schreiben(
            client,
            &format!(r#"{{"type":"heartbeat","session_token":"{token}","seq":{seq}}}"#),
        )
        .unwrap();
    }

    #[test]
    fn v2_heartbeat_bekommt_ack_und_messstand_landet_im_register() {
        let name = test_pipe_name("e");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff = server_starten(&name, reg.clone(), "test".into(), "tok-2".into()).unwrap();

        let mut client = client_oeffnen(&name);
        frame_schreiben(
            &mut client,
            &hello_v2_json("s-v2", "nonce-a", "post", "paar-x"),
        )
        .unwrap();
        let welcome = frame_lesen(&mut client).unwrap();
        assert!(
            welcome.contains(r#""protocol_version":2"#),
            "welcome war: {welcome}"
        );

        frame_schreiben(
            &mut client,
            r#"{"type":"heartbeat","session_token":"tok-2","seq":5,
                "measurement":{"zustand":"messbereit","metrics_version":"m1-2026-08-13",
                    "aktiv_s":4.2,"gesamt_s":5.0,"lufs_i":-17.5,
                    "ltas_komposit_db":[-40.0,null,-41.5],
                    "projekt_fenster":{"von_samples":100,"bis_samples":48100,"spruenge":1}}}"#,
        )
        .unwrap();
        let ack = frame_lesen(&mut client).unwrap();
        assert!(ack.contains(r#""type":"heartbeat_ack""#), "ack war: {ack}");
        assert!(ack.contains(r#""seq":5"#));
        assert!(ack.contains(r#""konflikt":false"#));
        {
            let r = reg.lock().unwrap();
            let s = &r.sensoren["s-v2"];
            assert_eq!(s.protokoll_version, 2);
            let m = s.messung.as_ref().expect("Messstand muss ankommen");
            assert_eq!(m.lufs_i, Some(-17.5));
            assert_eq!(m.projekt_fenster.unwrap().spruenge, 1);
        }

        // Zweite Verbindung mit derselben Sensor-ID → das nächste ACK trägt konflikt=true.
        let mut dup = client_oeffnen(&name);
        frame_schreiben(
            &mut dup,
            &hello_v2_json("s-v2", "nonce-b", "post", "paar-x"),
        )
        .unwrap();
        let _ = frame_lesen(&mut dup).unwrap();
        assert!(warte_auf(&reg, |r| r.sensoren["s-v2"].konflikt));
        frame_schreiben(
            &mut client,
            r#"{"type":"heartbeat","session_token":"tok-2","seq":6}"#,
        )
        .unwrap();
        let ack = frame_lesen(&mut client).unwrap();
        assert!(ack.contains(r#""konflikt":true"#), "ack war: {ack}");

        griff.stoppen();
    }

    #[test]
    fn produktiver_v2_server_speist_den_gemeinsamen_coordinator_interventionsriegel() {
        let produktquelle = include_str!("lib.rs")
            .split_whitespace()
            .collect::<Vec<_>>()
            .join(" ");
        assert!(produktquelle.contains(
            "let griff_v2 = server::server_starten_mit_interventionssenke("
        ));
        assert!(produktquelle.contains("session_token.clone(), coordinator.clone(), )?;"));

        let name = test_pipe_name("v2-coordinator-intervention");
        let reg = Arc::new(Mutex::new(Register::default()));
        let coordinator = Arc::new(crate::coordinator::Coordinator::default());
        let mut griff = server_starten_mit_interventionssenke(
            &name,
            reg,
            "test".into(),
            "tok-v2-shared".into(),
            coordinator.clone(),
        )
        .unwrap();
        let mut client = verbinde_v2(
            &name,
            "tok-v2-shared",
            "s-v2-shared",
            "nonce-v2-shared",
        );
        frame_schreiben(
            &mut client,
            r#"{"type":"heartbeat","session_token":"tok-v2-shared","seq":1,
            "measurement":{"zustand":"sammelt","metrics_version":"m1","hoermarkierung":true,
            "aktiv_s":0.0,"gesamt_s":1.0}}"#,
        )
        .unwrap();
        assert!(frame_lesen(&mut client)
            .unwrap()
            .contains(r#""type":"heartbeat_ack""#));
        for _ in 0..100 {
            if coordinator.interventionssicht().aktive == 1 {
                break;
            }
            std::thread::sleep(std::time::Duration::from_millis(10));
        }
        assert_eq!(coordinator.interventionssicht().aktive, 1);
        assert!(!coordinator.evidence_dispatch());

        frame_schreiben(
            &mut client,
            r#"{"type":"heartbeat","session_token":"tok-v2-shared","seq":2,
            "measurement":{"zustand":"sammelt","metrics_version":"m1","hoermarkierung":false,
            "aktiv_s":0.0,"gesamt_s":1.0}}"#,
        )
        .unwrap();
        let _ = frame_lesen(&mut client).unwrap();
        for _ in 0..100 {
            if coordinator.interventionssicht().aktive == 0 {
                break;
            }
            std::thread::sleep(std::time::Duration::from_millis(10));
        }
        assert!(coordinator.interventionssicht().starke_evidenz_erlaubt);

        drop(client);
        for _ in 0..100 {
            if coordinator.interventionssicht().unknown {
                break;
            }
            std::thread::sleep(std::time::Duration::from_millis(10));
        }
        assert!(coordinator.interventionssicht().unknown);
        assert!(!coordinator.evidence_dispatch());
        griff.stoppen();
    }

    #[test]
    fn ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz() {
        let name = test_pipe_name("f");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff = server_starten(&name, reg.clone(), "test".into(), "tok-3".into()).unwrap();

        let riesig: Vec<String> = (0..600).map(|_| "0.0".to_string()).collect();
        let faelle = [
            (
                "s-negativ",
                "nonce-negativ",
                r#"{"zustand":"messbereit","metrics_version":"x","aktiv_s":-0.1,"gesamt_s":1.0}"#
                    .to_string(),
            ),
            (
                "s-oversize",
                "nonce-oversize",
                format!(
                    r#"{{"zustand":"messbereit","metrics_version":"x","aktiv_s":1.0,"gesamt_s":1.0,"ltas_komposit_db":[{}]}}"#,
                    riesig.join(",")
                ),
            ),
            (
                "s-extra",
                "nonce-extra",
                r#"{"zustand":"messbereit","metrics_version":"x","aktiv_s":1.0,"gesamt_s":1.0,"extra":true}"#
                    .to_string(),
            ),
        ];

        for (sensor_id, nonce, ungueltig) in faelle {
            let mut client = verbinde_v2(&name, "tok-3", sensor_id, nonce);
            frame_schreiben(
                &mut client,
                r#"{"type":"heartbeat","session_token":"tok-3","seq":1,
                    "measurement":{"zustand":"messbereit","metrics_version":"x",
                        "aktiv_s":1.0,"gesamt_s":1.0}}"#,
            )
            .unwrap();
            let ack = frame_lesen(&mut client).unwrap();
            assert!(ack.contains(r#""seq":1"#));
            assert!(reg.lock().unwrap().sensoren[sensor_id].messung.is_some());

            frame_schreiben(
                &mut client,
                &format!(
                    r#"{{"type":"heartbeat","session_token":"tok-3","seq":2,"measurement":{ungueltig}}}"#
                ),
            )
            .unwrap();
            assert!(warte_auf(&reg, |r| {
                r.sensoren.get(sensor_id).is_some_and(|s| !s.verbunden)
            }));
            assert!(
                frame_lesen(&mut client).is_err(),
                "ungültiger Messstand bekam fälschlich ein ACK"
            );
            let r = reg.lock().unwrap();
            assert_eq!(r.sensoren[sensor_id].heartbeats, 1);
            assert!(
                r.sensoren[sensor_id].messung.is_none(),
                "alter Stand blieb nach Protokollbruch als Evidenz stehen"
            );
        }

        let r = reg.lock().unwrap();
        assert_eq!(r.pakete_verworfen, 3);
        drop(r);

        griff.stoppen();
    }

    #[test]
    fn v1_heartbeat_mit_measurement_ist_protokollbruch() {
        let name = test_pipe_name("v1-no-measurement");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff = server_starten(&name, reg.clone(), "test".into(), "tok-v1".into()).unwrap();

        let mut client = client_oeffnen(&name);
        frame_schreiben(&mut client, &hello_json(1, "s-v1-messung")).unwrap();
        let welcome = frame_lesen(&mut client).unwrap();
        assert!(welcome.contains(r#""protocol_version":1"#));
        frame_schreiben(
            &mut client,
            r#"{"type":"heartbeat","session_token":"tok-v1","seq":1,
                "measurement":{"zustand":"sammelt","metrics_version":"x","gesamt_s":1.0}}"#,
        )
        .unwrap();

        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-v1-messung").is_some_and(|s| !s.verbunden)
        }));
        let r = reg.lock().unwrap();
        assert_eq!(r.sensoren["s-v1-messung"].heartbeats, 0);
        assert!(r.sensoren["s-v1-messung"].messung.is_none());
        assert_eq!(r.pakete_verworfen, 1);
        drop(r);

        griff.stoppen();
    }

    #[test]
    fn zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen() {
        let name = test_pipe_name("same-live-nonce");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten(&name, reg.clone(), "test".into(), "tok-nonce".into()).unwrap();

        let mut erste = verbinde_v2(&name, "tok-nonce", "s-nonce", "nonce-identisch");
        let mut zweite = client_oeffnen(&name);
        frame_schreiben(
            &mut zweite,
            &hello_v2_json("s-nonce", "nonce-identisch", "sensor", "paar-test"),
        )
        .unwrap();
        let reject = frame_lesen(&mut zweite).unwrap();
        assert!(
            reject.contains(r#""type":"reject""#),
            "Antwort war: {reject}"
        );
        assert!(
            reject.contains("bereits verbunden"),
            "Antwort war: {reject}"
        );

        {
            let r = reg.lock().unwrap();
            let sensor = &r.sensoren["s-nonce"];
            assert!(sensor.verbunden);
            assert_eq!(sensor.lebende, 1);
            assert!(!sensor.konflikt);
        }
        sende_heartbeat(&mut erste, "tok-nonce", 1);
        let ack = frame_lesen(&mut erste).unwrap();
        assert!(ack.contains(r#""seq":1"#));
        assert_eq!(reg.lock().unwrap().sensoren["s-nonce"].heartbeats, 1);

        frame_schreiben(&mut erste, r#"{"type":"bye","session_token":"tok-nonce"}"#).unwrap();
        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-nonce").is_some_and(|s| !s.verbunden)
        }));
        griff.stoppen();
    }

    #[test]
    fn vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet() {
        let name = test_pipe_name("empty-nonce");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten(&name, reg.clone(), "test".into(), "tok-empty".into()).unwrap();

        let mut client = client_oeffnen(&name);
        frame_schreiben(
            &mut client,
            &hello_v2_json("s-empty", "", "sensor", "paar-test"),
        )
        .unwrap();
        let reject = frame_lesen(&mut client).unwrap();
        assert!(
            reject.contains(r#""type":"reject""#),
            "Antwort war: {reject}"
        );
        assert!(reject.contains("instance_nonce"), "Antwort war: {reject}");
        assert!(!reg.lock().unwrap().sensoren.contains_key("s-empty"));

        griff.stoppen();
    }

    #[test]
    fn fremdes_session_token_beendet_established_statt_acklos_zu_haengen() {
        let name = test_pipe_name("strict-token");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten(&name, reg.clone(), "test".into(), "tok-richtig".into()).unwrap();

        let mut client = verbinde_v2(&name, "tok-richtig", "s-token", "nonce-token");
        sende_heartbeat(&mut client, "tok-falsch", 1);

        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-token").is_some_and(|s| !s.verbunden)
        }));
        let r = reg.lock().unwrap();
        assert_eq!(r.sensoren["s-token"].heartbeats, 0);
        assert_eq!(r.pakete_verworfen, 1);
        assert!(r.fehler.iter().any(|f| f.contains("fremdem Session-Token")));
        drop(r);

        griff.stoppen();
    }

    #[test]
    fn bye_braucht_exakt_das_ausgegebene_session_token() {
        let name = test_pipe_name("strict-bye");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten(&name, reg.clone(), "test".into(), "tok-bye".into()).unwrap();

        let mut ohne = verbinde_v2(&name, "tok-bye", "s-bye-ohne", "nonce-bye-ohne");
        frame_schreiben(&mut ohne, r#"{"type":"bye"}"#).unwrap();
        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-bye-ohne").is_some_and(|s| !s.verbunden)
        }));

        let mut falsch = verbinde_v2(&name, "tok-bye", "s-bye-falsch", "nonce-bye-falsch");
        frame_schreiben(
            &mut falsch,
            r#"{"type":"bye","session_token":"tok-anders"}"#,
        )
        .unwrap();
        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-bye-falsch").is_some_and(|s| !s.verbunden)
        }));

        let mut null = verbinde_v2(&name, "tok-bye", "s-bye-null", "nonce-bye-null");
        frame_schreiben(&mut null, r#"{"type":"bye","session_token":null}"#).unwrap();
        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-bye-null").is_some_and(|s| !s.verbunden)
        }));

        let r = reg.lock().unwrap();
        assert_eq!(r.pakete_verworfen, 3);
        assert_eq!(
            r.fehler
                .iter()
                .filter(|f| f.contains("Bye mit fehlendem oder fremdem"))
                .count(),
            3
        );
        drop(r);
        griff.stoppen();
    }

    #[test]
    fn sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung() {
        let name = test_pipe_name("strict-seq");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten(&name, reg.clone(), "test".into(), "tok-seq".into()).unwrap();

        for (sensor_id, erste, zweite) in [("s-seq-rueck", 5, 4), ("s-seq-dup", 7, 7)] {
            let mut client =
                verbinde_v2(&name, "tok-seq", sensor_id, &format!("nonce-{sensor_id}"));
            sende_heartbeat(&mut client, "tok-seq", erste);
            let ack = frame_lesen(&mut client).unwrap();
            assert!(ack.contains(&format!(r#""seq":{erste}"#)));

            sende_heartbeat(&mut client, "tok-seq", zweite);
            assert!(warte_auf(&reg, |r| {
                r.sensoren.get(sensor_id).is_some_and(|s| !s.verbunden)
            }));
            assert_eq!(reg.lock().unwrap().sensoren[sensor_id].heartbeats, 1);
        }

        assert_eq!(reg.lock().unwrap().pakete_verworfen, 2);
        griff.stoppen();
    }

    #[test]
    fn zweites_hello_kann_die_established_bindung_nicht_umschreiben() {
        let name = test_pipe_name("strict-second-hello");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten(&name, reg.clone(), "test".into(), "tok-hello".into()).unwrap();

        let mut client = verbinde_v2(&name, "tok-hello", "s-hello-alt", "nonce-hello");
        frame_schreiben(
            &mut client,
            &hello_v2_json("s-hello-neu", "nonce-neu", "post", "paar-neu"),
        )
        .unwrap();

        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-hello-alt").is_some_and(|s| !s.verbunden)
        }));
        let r = reg.lock().unwrap();
        assert!(!r.sensoren.contains_key("s-hello-neu"));
        assert_eq!(r.pakete_verworfen, 1);
        drop(r);
        griff.stoppen();
    }

    #[test]
    fn sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten() {
        let name = test_pipe_name("sensor-cap");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten_mit_grenzen(
                &name, reg.clone(), "test".into(), "tok-cap".into(), 4, 2, None,
            )
                .unwrap();

        for (sensor_id, nonce) in [("s-cap-1", "nonce-cap-1"), ("s-cap-2", "nonce-cap-2")] {
            let mut client = verbinde_v2(&name, "tok-cap", sensor_id, nonce);
            frame_schreiben(&mut client, r#"{"type":"bye","session_token":"tok-cap"}"#).unwrap();
            assert!(warte_auf(&reg, |r| {
                r.sensoren.get(sensor_id).is_some_and(|s| !s.verbunden)
            }));
        }

        let mut neu = client_oeffnen(&name);
        frame_schreiben(
            &mut neu,
            &hello_v2_json("s-cap-3", "nonce-cap-3", "sensor", "paar-test"),
        )
        .unwrap();
        let reject = frame_lesen(&mut neu).unwrap();
        assert!(
            reject.contains(r#""type":"reject""#),
            "Antwort war: {reject}"
        );
        assert!(reject.contains("Sensor-ID-Grenze"));
        assert_eq!(reg.lock().unwrap().sensoren.len(), 2);

        let reconnect = verbinde_v2(&name, "tok-cap", "s-cap-1", "nonce-cap-reconnect");
        assert!(warte_auf(&reg, |r| r.sensoren["s-cap-1"].verbunden));
        drop(reconnect);
        griff.stoppen();
    }

    #[test]
    fn connection_und_pipe_instanzzahl_bleiben_hart_begrenzt() {
        let name = test_pipe_name("connection-cap");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten_mit_grenzen(
                &name, reg, "test".into(), "tok-conn-cap".into(), 2, 8, None,
            )
                .unwrap();

        // Zwei Clients bleiben absichtlich schon vor hello stehen und binden
        // damit genau die beiden zulässigen Connection-Threads.
        let c1 = client_oeffnen(&name);
        let c2 = client_oeffnen(&name);
        std::thread::sleep(std::time::Duration::from_millis(30));
        for _ in 0..20 {
            assert!(
                std::fs::OpenOptions::new()
                    .read(true)
                    .write(true)
                    .open(&name)
                    .is_err(),
                "dritte Pipe-Verbindung durfte die feste Grenze nicht überschreiten"
            );
            std::thread::sleep(std::time::Duration::from_millis(2));
        }

        drop(c1);
        let c3 = client_oeffnen(&name);
        drop((c2, c3));
        griff.stoppen();
    }

    #[test]
    fn stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab() {
        let name = test_pipe_name("stop-silent-connection");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten(&name, reg.clone(), "test".into(), "tok-stop".into()).unwrap();
        let client = verbinde_v2(&name, "tok-stop", "s-stop", "nonce-stop");
        assert!(warte_auf(&reg, |r| r.sensoren["s-stop"].verbunden));

        let (fertig_tx, fertig_rx) = std::sync::mpsc::channel();
        let stop_thread = std::thread::spawn(move || {
            griff.stoppen();
            fertig_tx.send(()).unwrap();
        });
        fertig_rx
            .recv_timeout(std::time::Duration::from_secs(2))
            .expect("Server-Stop hing an stiller Pipe-I/O");
        stop_thread.join().unwrap();
        drop(client);
    }

    #[test]
    fn acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup() {
        for i in 0..100 {
            let name = test_pipe_name(&format!("stop-race-{i}"));
            let reg = Arc::new(Mutex::new(Register::default()));
            let mut erster =
                server_starten(&name, reg.clone(), "test".into(), "tok-a".into()).unwrap();
            erster.stoppen();

            let mut zweiter = server_starten(&name, reg, "test".into(), "tok-b".into()).unwrap();
            zweiter.stoppen();
        }
    }

    #[test]
    fn ungelesenes_welcome_blockiert_connection_ende_nicht() {
        let name = test_pipe_name("no-unbounded-flush");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten_mit_grenzen(
                &name, reg.clone(), "test".into(), "tok-flush".into(), 1, 8, None,
            )
                .unwrap();

        let mut erster = client_oeffnen(&name);
        frame_schreiben(
            &mut erster,
            &hello_v2_json("s-flush", "nonce-flush", "sensor", "paar-test"),
        )
        .unwrap();
        // Welcome bewusst NICHT lesen; Bye läuft über die unabhängige
        // Client→Server-Richtung der Duplex-Pipe.
        frame_schreiben(&mut erster, r#"{"type":"bye","session_token":"tok-flush"}"#).unwrap();
        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-flush").is_some_and(|s| !s.verbunden)
        }));

        // Bei altem unbounded Flush bliebe die einzige Kapazität belegt und
        // dieser zweite Open könnte nie erfolgreich werden.
        let zweiter = client_oeffnen(&name);
        drop((erster, zweiter));
        griff.stoppen();
    }

    #[test]
    fn ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist() {
        let name = test_pipe_name("bounded-reject-flush");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff =
            server_starten_mit_grenzen(
                &name, reg, "test".into(), "tok-reject".into(), 1, 8, None,
            )
                .unwrap();

        let mut erster = client_oeffnen(&name);
        frame_schreiben(&mut erster, &hello_json(99, "s-reject")).unwrap();
        // Reject bewusst nicht lesen. Nach 100 ms muss der Flush abgebrochen
        // und die einzige Verbindungskapazität wieder frei sein.
        let zweiter = client_oeffnen(&name);

        drop((erster, zweiter));
        griff.stoppen();
    }
}
