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
    ERROR_NOT_FOUND, ERROR_NO_DATA, ERROR_NO_TOKEN, ERROR_OPERATION_ABORTED, ERROR_PIPE_BUSY,
    ERROR_PIPE_CONNECTED, ERROR_PIPE_NOT_CONNECTED, HANDLE, INVALID_HANDLE_VALUE, WAIT_OBJECT_0,
};
use windows_sys::Win32::Security::{
    CopySid, EqualSid, GetLengthSid, GetTokenInformation, IsValidSid, RevertToSelf, TokenUser,
    SECURITY_ATTRIBUTES, TOKEN_QUERY, TOKEN_USER,
};
use windows_sys::Win32::Storage::FileSystem::{
    FlushFileBuffers, ReadFile, WriteFile, FILE_FLAG_FIRST_PIPE_INSTANCE, FILE_FLAG_OVERLAPPED,
    PIPE_ACCESS_DUPLEX,
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
use windows_sys::Win32::Foundation::{DuplicateHandle, DUPLICATE_SAME_ACCESS};
use windows_sys::Win32::System::Threading::GetCurrentProcess;

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
mod griff;
mod listener;
mod queues;
mod senke;
mod trennung;
mod verbindung;
mod win_handles;

use auth::{client_sid_authentisieren, VerbindungsSicherheitsCleanup};
use griff::{fertige_ernten, join_mit_frist, worker_reservieren, TestHilfsthread};
use queues::{Ausgang, Eingang};
use listener::{
    CANCEL_VOR_READ_BEREIT, CANCEL_VOR_READ_FEHLER,
    CANCEL_VOR_READ_FORTGESETZT, CANCEL_VOR_READ_GELESEN, CANCEL_VOR_READ_INAKTIV,
    CANCEL_VOR_READ_READER, CANCEL_VOR_READ_WRITER,
};
use verbindung::{verbindung_bedienen, Senkenruf};
use trennung::{
    auf_telemetrie_getrennt_warten, kopplung_loesen, melden_getrennt,
    telemetrie_getrennt_gemeldet, trennmelder_anlegen, trennmelder_telemetrie_abgesagt,
    trennmelder_telemetrie_erwartet, TrennRegister,
};
use win_handles::{
    abbrechen_und_zaehlen_extern, alle_io_abbrechen, io_abbrechen, ov_lesen, ov_schreiben,
    EndeSignal, Ereignis, HandleRegister, IoAusgang, ListenerInstanz, SicherheitsSpur, TokenGriff,
    Verbindungsgriff,
};

pub use senke::{ControlAnmeldung, Senke, ZaehlSenke};
pub use griff::{V3Closer, V3Griff, V3Sender, V3Statistik, WorkerPlatzProbe};
pub use listener::{
    v3_server_starten, v3_server_starten_fuer_security_vectors, v3_server_starten_mit_epoch,
    v3_server_starten_mit_epoch_und_sender, V3AuthTestFehler, V3SecurityTestOptionen,
    V3StartTestFehler, V3TotesHandleNaht, V3UebergabeBarriere,
};

pub const MAX_VERBINDUNGEN: usize = 96;

/// 96 Worker plus zwei jederzeit bewaffnete Besitzlistener. Windows erlaubt
/// hier 1..=255; alle Instanzen desselben Namens muessen denselben Wert nennen.
pub const PIPE_INSTANZEN: usize = MAX_VERBINDUNGEN + 2;

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

/// Aktive Vertragsfassungen JE FAMILIE. Die Version lebt ausschliesslich im
/// Wire-Envelope: Descriptor-Hostfelder und LUFS-I-Framefelder wurden in
/// SONDE-012 B1 mit Minor 1 belegt.
///
/// SONDE-013 (04.09.2026) hebt **P0 und P1** auf 2: die drei
/// Experimentfamilien, die belegten Felder `evidence_snapshot.ereignisse`
/// und `.stereo` und die zwei neuen Invalidierungsgruende reisen dort.
///
/// 🔑 **P2 bleibt bei 1**, und das ist kein Versehen. `integration_samples`
/// (Feld-ID 14) liegt auf der FlatBuffers-Flaeche, deren eigene
/// `schema_major`/`schema_minor` im 16-Byte-Envelope leben
/// (`nakama_telemetry_v1.fbs`). Ein neues OPTIONALES FlatBuffers-Feld ist
/// genau der additive Fall, den das Format traegt: ein alter Leser
/// uebergeht es. Waere P2 hier mitgehoben worden, haette der Broker jede
/// heutige Sonde abgewiesen, ohne dass sich an P2 etwas geaendert haette.
///
/// ⚠️ WARUM DIESE DREI ZAHLEN UEBERHAUPT EINZELN STEHEN — gemessen am
/// 04.09.2026: mit `kJsonSchemaMinor = 2` auf der C++-Seite und einer 1
/// hier wies der Server JEDEN P0-Rahmen der Sonde ab, bevor er die Senke
/// erreichte. Der Kanon blieb an `subscription_server_integration` haengen,
/// vier `eqcop-store-crash-worker` warteten 17 Minuten ohne CPU-Last auf ein
/// `command_ack`, das nie kam. Ein Fassungsschritt ist erst vollstaendig,
/// wenn BEIDE Seiten und der Transport dazwischen ihn kennen.
const P0_SCHEMA_MINOR: u8 = 2;
const P1_SCHEMA_MINOR: u8 = 2;
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

/// Takt des Wachhunds. Bis NAK-121 stand hier ein nacktes
/// `Duration::from_millis(100)` in der Schleife; H-02 misst gegen diese Frist,
/// also bekommt sie einen Namen. Modulintern, keine oeffentliche Signatur.
pub(super) const WACHHUND_TAKT: Duration = Duration::from_millis(100);

/// H-07: harte Frist des beschraenkten Abflusses vor dem Schliessen. Derselbe
/// Wert wie `ANTWORT_FLUSH_TIMEOUT` auf der v2-Seite, weil es dieselbe Frage
/// ist: wie lange darf ein nicht lesender Peer den Abbau aufhalten.
pub(super) const FLUSH_FRIST: Duration = Duration::from_millis(250);

//==============================================================================

#[cfg(test)]
mod tests {
    use super::listener::v3_server_starten_intern;
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

        // Beide Muster stehen seit der Modulaufteilung in verschiedenen Dateien:
        // der Wachhundfilter in listener.rs, die Joinfrist in griff.rs. Der Schnitt am
        // Testmodul-Marker bleibt, damit kein Treffer aus einem Testteil zaehlt.
        let produktteil = |quelle: &'static str| {
            let testmodul_marker = ["\n#[cfg(test)]\n", "mod tests {"].concat();
            match quelle.rfind(&testmodul_marker) {
                Some(tests_beginn) => &quelle[..tests_beginn],
                None => quelle,
            }
        };
        assert!(produktteil(include_str!("listener.rs")).contains(".filter(|(_, f)| *f <= jetzt)"));
        assert!(produktteil(include_str!("griff.rs")).contains("if Instant::now() >= bis"));

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
        // Die drei Familien haben SEIT SONDE-013 nicht mehr dieselbe
        // Obergrenze. Eine gemeinsame Schleife haette das nicht ausdruecken
        // koennen und war genau deshalb still falsch, als P0/P1 stiegen.
        for familie in [Familie::P0, Familie::P1, Familie::P2] {
            assert!(schema_minor_bekannt(familie, 0));
            assert!(schema_minor_bekannt(familie, 1));
        }
        // P0 und P1 tragen die Fassung 2 (Experimentfamilien, belegte
        // Evidenzfelder, zwei neue Invalidierungsgruende).
        assert!(schema_minor_bekannt(Familie::P0, 2));
        assert!(schema_minor_bekannt(Familie::P1, 2));
        // P2 nicht: dort ist seit SONDE-013 nichts hinzugekommen, was eine
        // Fassung braeuchte - `integration_samples` ist ein optionales
        // FlatBuffers-Feld und damit der additive Fall, den das Format traegt.
        assert!(!schema_minor_bekannt(Familie::P2, 2));
        // Und die Gegenprobe nach oben: eine Fassung, die es nicht gibt,
        // wird auch bei P0/P1 abgewiesen.
        for familie in [Familie::P0, Familie::P1, Familie::P2] {
            assert!(!schema_minor_bekannt(familie, 3));
            assert!(!schema_minor_bekannt(familie, 200));
        }
    }

    /// SONDE-013: die Fassung des Transports und die des JSON-Lesers sind
    /// DIESELBE Zahl. Liefen sie auseinander, wiese der Server Rahmen ab, die
    /// der Coordinator lesen koennte - oder liesse Rahmen durch, fuer die er
    /// keinen Leser hat. Gemessen, nicht kommentiert.
    #[test]
    fn transportfassung_und_json_leser_stimmen_ueberein() {
        assert_eq!(
            u8::from(P1_SCHEMA_MINOR),
            crate::coordinator::JSON_SCHEMA_MINOR_AKTIV_FUER_TEST
        );
        assert_eq!(P0_SCHEMA_MINOR, P1_SCHEMA_MINOR);
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
        // SONDE-013 E-02 (03.09.2026): `experiment_begin` und
        // `experiment_abort` sind aus der Reserve nach `definiert` gewandert,
        // `experiment_manual_result` ist neu dazugekommen. 18/9 -> 21/7.
        // Nacharbeit 2 (04.09.2026, Befunde R16/R21): `experiment_candidate`
        // kommt als vierte Familie dieses Tickets dazu — der Schritt zwischen
        // Begin und Ergebnis. 21/7 -> 22/7.
        assert_eq!(definiert, 22);
        assert_eq!(spaeter, 7);
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
        assert_eq!(main_welcome.kopf.schema_minor, P0_SCHEMA_MINOR);
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
        assert_eq!(snapshot.kopf.schema_minor, P1_SCHEMA_MINOR);
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
