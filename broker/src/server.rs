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
use super::protokoll::{hello_pruefen, BrokerNachricht, ClientNachricht};
use super::Register;
use std::fs::File;
use std::os::windows::io::{AsRawHandle, FromRawHandle};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use std::thread::JoinHandle;

use windows_sys::Win32::Foundation::{
    CloseHandle, GetLastError, LocalFree, ERROR_INSUFFICIENT_BUFFER, ERROR_PIPE_CONNECTED,
    HANDLE, INVALID_HANDLE_VALUE,
};
use windows_sys::Win32::Security::Authorization::{
    ConvertSidToStringSidW, ConvertStringSecurityDescriptorToSecurityDescriptorW,
};
use windows_sys::Win32::Security::{
    GetTokenInformation, TokenUser, SECURITY_ATTRIBUTES, TOKEN_QUERY, TOKEN_USER,
};
use windows_sys::Win32::Storage::FileSystem::{
    FlushFileBuffers, FILE_FLAG_FIRST_PIPE_INSTANCE, PIPE_ACCESS_DUPLEX,
};
use windows_sys::Win32::System::Pipes::{
    ConnectNamedPipe, CreateNamedPipeW, DisconnectNamedPipe, PIPE_READMODE_BYTE,
    PIPE_REJECT_REMOTE_CLIENTS, PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES, PIPE_WAIT,
};
use windows_sys::Win32::System::Threading::{GetCurrentProcess, OpenProcessToken};

/// Sicherheitsbeschreibung „nur der aktuelle Windows-User" (SDDL `D:P(A;;GA;;;<SID>)`).
/// Hält den LocalAlloc-Deskriptor am Leben, solange der Server läuft.
struct Sicherheit {
    deskriptor: *mut core::ffi::c_void,
}
// SAFETY: `deskriptor` ist ein exklusiv besessener LocalAlloc-Block ohne
// Thread-Affinität; nach dem Bau wird er nur noch gelesen (CreateNamedPipeW)
// und genau einmal in Drop freigegeben — kein geteilter veränderlicher Zustand.
unsafe impl Send for Sicherheit {}

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

fn aktueller_user_sid() -> Result<String, String> {
    let mut token: HANDLE = std::ptr::null_mut();
    // SAFETY: GetCurrentProcess liefert ein Pseudo-Handle (nicht zu schließen);
    // OpenProcessToken schreibt nur bei Erfolg ein gültiges Token nach `token`.
    if unsafe { OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &mut token) } == 0 {
        // SAFETY: GetLastError liest nur den threadlokalen Fehlercode.
        return Err(format!("OpenProcessToken: Win32 {}", unsafe { GetLastError() }));
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
    let mut sid_w: *mut u16 = std::ptr::null_mut();
    // SAFETY: `sid_ptr` zeigt in den lebenden `puffer`; bei Erfolg alloziert
    // die API einen nullterminierten UTF-16-String nach `sid_w`.
    if unsafe { ConvertSidToStringSidW(sid_ptr, &mut sid_w) } == 0 {
        // SAFETY: GetLastError liest nur den threadlokalen Fehlercode.
        return Err(format!("ConvertSidToStringSid: Win32 {}", unsafe { GetLastError() }));
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
    Ok(sid)
}

fn sicherheit_nur_user() -> Result<Sicherheit, String> {
    let sid = aktueller_user_sid()?;
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
    Ok(Sicherheit { deskriptor })
}

pub struct ServerGriff {
    stop: Arc<AtomicBool>,
    pipe_name: String,
    join: Option<JoinHandle<()>>,
}

impl ServerGriff {
    pub fn stoppen(&mut self) {
        self.stop.store(true, Ordering::SeqCst);
        // ConnectNamedPipe blockiert — ein Wegwerf-Client weckt den Server.
        let _ = std::fs::OpenOptions::new()
            .read(true)
            .write(true)
            .open(&self.pipe_name);
        if let Some(j) = self.join.take() {
            let _ = j.join();
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
    let sicherheit = sicherheit_nur_user()?;
    let stop = Arc::new(AtomicBool::new(false));
    let stop2 = stop.clone();
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
            PIPE_UNLIMITED_INSTANCES,
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
            let mut naechstes_handle: Option<HANDLE> = Some(erstes_handle.uebernehmen());

            while !stop2.load(Ordering::SeqCst) {
                let handle = match naechstes_handle.take() {
                    Some(h) => h,
                    // SAFETY: wie beim ersten CreateNamedPipeW — `name_w` ist
                    // nullterminiert, `attrs`/Deskriptor leben im Thread weiter.
                    None => unsafe {
                        CreateNamedPipeW(
                            name_w.as_ptr(),
                            PIPE_ACCESS_DUPLEX,
                            PIPE_TYPE_BYTE
                                | PIPE_READMODE_BYTE
                                | PIPE_WAIT
                                | PIPE_REJECT_REMOTE_CLIENTS,
                            PIPE_UNLIMITED_INSTANCES,
                            65536,
                            65536,
                            0,
                            &attrs,
                        )
                    },
                };
                if handle == INVALID_HANDLE_VALUE {
                    // SAFETY: GetLastError liest nur den threadlokalen Fehlercode.
                    let fehler = unsafe { GetLastError() };
                    if let Ok(mut r) = register.lock() {
                        r.fehler_merken(format!("CreateNamedPipe: Win32 {fehler}"));
                    }
                    std::thread::sleep(std::time::Duration::from_millis(500));
                    continue;
                }
                let handle = HandleGuard(handle);

                // SAFETY: `handle.0` ist die eben angelegte, exklusive Pipe-Instanz.
                let verbunden = unsafe { ConnectNamedPipe(handle.0, std::ptr::null_mut()) };
                // SAFETY: GetLastError liest nur den threadlokalen Fehlercode
                // — direkt nach ConnectNamedPipe, vor jedem weiteren Aufruf.
                let verbinde_fehler = if verbunden == 0 { unsafe { GetLastError() } } else { 0 };
                if verbunden == 0 && verbinde_fehler != ERROR_PIPE_CONNECTED {
                    continue; // Guard schließt das Handle
                }
                if stop2.load(Ordering::SeqCst) {
                    break; // Guard schließt das Handle
                }

                // SAFETY: der Guard übergibt sein exklusives, gültiges Handle
                // an das File — ab hier schließt dessen Drop genau einmal.
                let datei = unsafe { File::from_raw_handle(handle.uebernehmen() as _) };
                let reg = register.clone();
                let bv = broker_version.clone();
                let tok = session_token.clone();
                let _ = std::thread::Builder::new()
                    .name("eqcop-pipe-conn".into())
                    .spawn(move || verbindung_bedienen(datei, reg, bv, tok));
            }
        })
        .map_err(|e| format!("Serverthread: {e}"))?;

    Ok(ServerGriff { stop, pipe_name: name, join: Some(join) })
}

fn verbindung_bedienen(
    mut datei: File,
    register: Arc<Mutex<Register>>,
    broker_version: String,
    session_token: String,
) {
    // Erstes Paket muss ein gültiges hello sein — sonst Verbindung beenden.
    let erster = match frame_lesen(&mut datei) {
        Ok(json) => json,
        Err(FrameFehler::Ende) => return,
        Err(e) => {
            if let Ok(mut r) = register.lock() {
                r.fehler_merken(format!("Verbindung vor hello beendet: {e:?}"));
            }
            return;
        }
    };
    let hello = match serde_json::from_str::<ClientNachricht>(&erster) {
        Ok(ClientNachricht::Hello(h)) => h,
        _ => {
            if let Ok(mut r) = register.lock() {
                r.fehler_merken("erstes Paket war kein gültiges hello".into());
            }
            return;
        }
    };

    let antwort = hello_pruefen(&hello, &broker_version, &session_token);
    let json = serde_json::to_string(&antwort).unwrap_or_default();
    if frame_schreiben(&mut datei, &json).is_err() {
        return;
    }
    if matches!(antwort, BrokerNachricht::Reject { .. }) {
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
        .filter(|n| !n.is_empty())
        .unwrap_or_else(|| uuid::Uuid::new_v4().to_string());
    // v1-Clients erwarten keine ACKs — ihnen darf der Broker nichts senden.
    let mut version = hello.protocol_version;
    let mut sensor_id = hello.sensor.sensor_id.clone();
    if let Ok(mut r) = register.lock() {
        r.sensor_verbinden(&hello, &nonce);
    }

    // Heartbeat-Schleife — ungültige Pakete werden verworfen, nie zum Absturz.
    loop {
        match frame_lesen(&mut datei) {
            Ok(json) => match serde_json::from_str::<ClientNachricht>(&json) {
                Ok(ClientNachricht::Heartbeat(hb)) => {
                    if hb.session_token != session_token {
                        if let Ok(mut r) = register.lock() {
                            r.fehler_merken(format!(
                                "Heartbeat mit fremdem Session-Token von {sensor_id}"
                            ));
                        }
                        continue;
                    }
                    // Schema-Guards, die serde nicht abdeckt (LTAS-Länge,
                    // Zustands-Enum): ungültig ⇒ gezählt verwerfen, nie kürzen.
                    let messung = match hb.measurement {
                        Some(m) if !m.ist_gueltig() => {
                            if let Ok(mut r) = register.lock() {
                                r.paket_verworfen();
                            }
                            None
                        }
                        m => m,
                    };
                    let konflikt = match register.lock() {
                        Ok(mut r) => {
                            r.heartbeat(&sensor_id, &nonce, hb.stats, messung);
                            r.konflikt_von(&sensor_id)
                        }
                        Err(_) => false,
                    };
                    if version >= 2 {
                        let ack = BrokerNachricht::HeartbeatAck { seq: hb.seq, konflikt };
                        let json = serde_json::to_string(&ack).unwrap_or_default();
                        if frame_schreiben(&mut datei, &json).is_err() {
                            break;
                        }
                    }
                }
                Ok(ClientNachricht::Bye(_)) => break,
                Ok(ClientNachricht::Hello(h2)) => {
                    // Erneutes hello innerhalb der Verbindung: Bindung übernehmen.
                    // Wechselt die Sensor-ID (»neue Kennung«), wird die alte
                    // sauber getrennt — sonst bliebe sie ewig »verbunden«.
                    if let Ok(mut r) = register.lock() {
                        if h2.sensor.sensor_id != sensor_id {
                            r.sensor_trennen(&sensor_id, &nonce);
                            sensor_id = h2.sensor.sensor_id.clone();
                        }
                        version = h2.protocol_version;
                        r.sensor_verbinden(&h2, &nonce);
                    }
                }
                Err(_) => {
                    if let Ok(mut r) = register.lock() {
                        r.paket_verworfen();
                    }
                }
            },
            Err(FrameFehler::Ende) => break,
            Err(e) => {
                if let Ok(mut r) = register.lock() {
                    r.fehler_merken(format!("Verbindung {sensor_id}: {e:?}"));
                }
                break;
            }
        }
    }

    if let Ok(mut r) = register.lock() {
        r.sensor_trennen(&sensor_id, &nonce);
    }
    pipe_schliessen(&datei);
}

fn pipe_schliessen(datei: &File) {
    // SAFETY: das Handle gehört dem noch lebenden File; Flush/Disconnect sind
    // auf einer Server-Pipe-Instanz gültig. CloseHandle übernimmt der File-Drop.
    unsafe {
        let h = datei.as_raw_handle() as HANDLE;
        FlushFileBuffers(h);
        DisconnectNamedPipe(h);
    }
}

#[cfg(test)]
mod tests {
    use super::super::protokoll::PROTOKOLL_VERSION;
    use super::*;
    use std::io::Write;

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
            if let Ok(f) = std::fs::OpenOptions::new().read(true).write(true).open(name) {
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
        assert!(antwort.contains(r#""type":"welcome""#), "Antwort war: {antwort}");
        assert!(antwort.contains("token-1"));

        frame_schreiben(
            &mut client,
            r#"{"type":"heartbeat","session_token":"token-1","seq":1,
                "stats":{"rms_l":0.25,"rms_r":0.2,"frames_dropped":3,"nan_seen":false}}"#,
        )
        .unwrap();
        // hello_json spricht v2 (aktuelle Version) — jeder Heartbeat wird quittiert.
        let ack = frame_lesen(&mut client).unwrap();
        assert!(ack.contains(r#""type":"heartbeat_ack""#), "Antwort war: {ack}");
        assert!(warte_auf(&reg, |r| {
            r.sensoren.get("s-eins").is_some_and(|s| s.verbunden && s.heartbeats == 1)
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
        assert!(reg.lock().unwrap().sensoren["s-eins"].getrennt_seit_ms.is_some());

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
        assert!(antwort.contains(r#""type":"reject""#), "Antwort war: {antwort}");
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
            r.sensoren.get("s-dup").is_some_and(|s| !s.konflikt && s.verbunden)
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

    #[test]
    fn v2_heartbeat_bekommt_ack_und_messstand_landet_im_register() {
        let name = test_pipe_name("e");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff = server_starten(&name, reg.clone(), "test".into(), "tok-2".into()).unwrap();

        let mut client = client_oeffnen(&name);
        frame_schreiben(&mut client, &hello_v2_json("s-v2", "nonce-a", "post", "paar-x")).unwrap();
        let welcome = frame_lesen(&mut client).unwrap();
        assert!(welcome.contains(r#""protocol_version":2"#), "welcome war: {welcome}");

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
        frame_schreiben(&mut dup, &hello_v2_json("s-v2", "nonce-b", "post", "paar-x")).unwrap();
        let _ = frame_lesen(&mut dup).unwrap();
        assert!(warte_auf(&reg, |r| r.sensoren["s-v2"].konflikt));
        frame_schreiben(&mut client, r#"{"type":"heartbeat","session_token":"tok-2","seq":6}"#).unwrap();
        let ack = frame_lesen(&mut client).unwrap();
        assert!(ack.contains(r#""konflikt":true"#), "ack war: {ack}");

        griff.stoppen();
    }

    #[test]
    fn feindliches_ltas_array_wird_gezaehlt_verworfen() {
        let name = test_pipe_name("f");
        let reg = Arc::new(Mutex::new(Register::default()));
        let mut griff = server_starten(&name, reg.clone(), "test".into(), "tok-3".into()).unwrap();

        let mut client = client_oeffnen(&name);
        frame_schreiben(&mut client, &hello_v2_json("s-boese", "nonce-x", "sensor", "")).unwrap();
        let _ = frame_lesen(&mut client).unwrap();

        let riesig: Vec<String> = (0..600).map(|_| "0.0".to_string()).collect();
        frame_schreiben(
            &mut client,
            &format!(
                r#"{{"type":"heartbeat","session_token":"tok-3","seq":1,
                    "measurement":{{"zustand":"messbereit","metrics_version":"x",
                        "ltas_komposit_db":[{}]}}}}"#,
                riesig.join(",")
            ),
        )
        .unwrap();
        // ACK kommt trotzdem (die Verbindung lebt weiter) …
        let ack = frame_lesen(&mut client).unwrap();
        assert!(ack.contains("heartbeat_ack"));
        // … aber der Messstand wurde verworfen und gezählt.
        let r = reg.lock().unwrap();
        assert!(r.sensoren["s-boese"].messung.is_none());
        assert_eq!(r.pakete_verworfen, 1);
        drop(r);

        griff.stoppen();
    }
}
