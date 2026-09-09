//! Die geteilten Bauteile der v3-Listenertests.
//!
//! Ein roher Testclient auf der Probe-Pipe, die Rahmenbauer der drei
//! Prioritaetsklassen und die Bootstrap-Nachrichten. Sie stehen hier, weil
//! jeder Fachbereich der Tests dieselbe Verbindung aufbaut; eine zweite
//! Fassung je Datei waeren mehrere Gelegenheiten, den Handshake verschieden
//! zu schreiben.
//!
//! ⚠️ Die Tests rufen den Listener AUSSCHLIESSLICH mit einem Probe-Namen
//! (`PROBE_PRAEFIX`) — nie mit der Produktions-Pipe (CLAUDE.md, „Bauen und
//! beweisen"). Sicherheit ist trotzdem nicht Probe-Qualitaet: derselbe
//! SDDL-Deskriptor und dieselben Flags wie im Produktpfad.
//!
//! Nur unter `cfg(test)` gebaut.

use super::*;
use crate::transport::bootstrap::Adresse;
use crate::transport::pipetoken::PROBE_PRAEFIX;
use std::sync::atomic::AtomicUsize;

static FOLGE: AtomicUsize = AtomicUsize::new(0);

pub(super) fn probe_pipe(fall: &str) -> String {
    format!(
        "{PROBE_PRAEFIX}srv.{}.{}.{fall}",
        std::process::id(),
        FOLGE.fetch_add(1, Ordering::SeqCst)
    )
}

/// Ein roher Testclient. Synchron, weil der Test seine Schritte ohnehin
/// nacheinander geht.
pub(super) struct Testclient {
    h: HANDLE,
}

// SAFETY: wie beim Verbindungsgriff — ein Pipe-HANDLE hat keine
// Threadaffinitaet.
unsafe impl Send for Testclient {}

impl Testclient {
    pub(super) fn neu(pipe: &str) -> Option<Self> {
        Self::mit_geduld(pipe, 10_000)
    }

    pub(super) fn mit_geduld(pipe: &str, millis: u64) -> Option<Self> {
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

    pub(super) fn schreiben(&self, daten: &[u8]) -> bool {
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

    pub(super) fn lesen(&self, ziel: &mut [u8]) -> usize {
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

pub(super) fn praefix(json: &str) -> Vec<u8> {
    let mut aus = (json.len() as u32).to_le_bytes().to_vec();
    aus.extend_from_slice(json.as_bytes());
    aus
}

pub(super) fn adresse_json(nonce: &str) -> String {
    format!(
        "{{\"logon_sid\":\"S-1-5-21-1-2-3-1001\",\"project_binding_id\":\"{p}\",\
         \"session_epoch\":\"{p}\",\"instance_id\":\"{p}\",\"runtime_nonce\":\"{nonce}\"}}",
        p = "0".repeat(32)
    )
}

pub(super) fn control_hello(nonce: &str) -> Vec<u8> {
    praefix(&format!(
        "{{\"type\":\"hello\",\"connection_kind\":\"control\",\"protocol\":3,\
         \"plugin_version\":\"0.3.0\",\"plugin_kind\":\"active_probe\",\"adresse\":{a},\
         \"audio\":{{\"samplerate\":48000,\"block_size\":512,\"channels\":2}}}}",
        a = adresse_json(nonce)
    ))
}

pub(super) fn test_adresse(zeichen: char) -> Adresse {
    Adresse {
        logon_sid: "S-1-5-21-1-2-3-1001".into(),
        project_binding_id: zeichen.to_string().repeat(32),
        session_epoch: zeichen.to_string().repeat(32),
        instance_id: zeichen.to_string().repeat(32),
        runtime_nonce: zeichen.to_string().repeat(32),
    }
}

pub(super) fn control_hello_adresse(adresse: &Adresse) -> Vec<u8> {
    praefix(&format!(
        "{{\"type\":\"hello\",\"connection_kind\":\"control\",\"protocol\":3,\
         \"plugin_version\":\"0.3.0\",\"plugin_kind\":\"active_probe\",\"adresse\":{},\
         \"audio\":{{\"samplerate\":48000,\"block_size\":512,\"channels\":2}}}}",
        serde_json::to_string(adresse).unwrap()
    ))
}

pub(super) fn control_hello_fach(adresse: &Adresse, plugin_kind: &str, host_pid: Option<u32>) -> Vec<u8> {
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

pub(super) fn subscribe(adresse: &Adresse) -> Vec<u8> {
    p1(&serde_json::json!({
        "type": "subscribe_session",
        "adresse": adresse,
        "session_epoch": adresse.session_epoch.clone()
    })
    .to_string())
}

pub(super) fn heartbeat_fuer_adresse(adresse: &Adresse, sequence: u64) -> Vec<u8> {
    vollstaendiger_heartbeat(adresse, sequence)
}

pub(super) fn vollstaendiger_heartbeat(adresse: &Adresse, sequence: u64) -> Vec<u8> {
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

pub(super) fn telemetry_hello(nonce: &str, link: &str, challenge: &str) -> Vec<u8> {
    praefix(&format!(
        "{{\"type\":\"hello\",\"connection_kind\":\"telemetry\",\"protocol\":3,\
         \"plugin_version\":\"0.3.0\",\"adresse\":{a},\"link_id\":\"{link}\",\
         \"challenge\":\"{challenge}\"}}",
        a = adresse_json(nonce)
    ))
}

pub(super) fn telemetry_hello_adresse(adresse: &Adresse, link: &str, challenge: &str) -> Vec<u8> {
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
pub(super) fn welcome_lesen(c: &Testclient) -> Option<(String, String)> {
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

pub(super) fn p0(json: &str) -> Vec<u8> {
    envelope_schreiben(Familie::P0, P0_SCHEMA_MINOR, json.as_bytes()).unwrap()
}

pub(super) fn p1(json: &str) -> Vec<u8> {
    envelope_schreiben(Familie::P1, P1_SCHEMA_MINOR, json.as_bytes()).unwrap()
}

pub(super) fn p2(bytes: &[u8]) -> Vec<u8> {
    envelope_schreiben(Familie::P2, P2_SCHEMA_MINOR, bytes).unwrap()
}

pub(super) fn warte_auf(millis: u64, mut bedingung: impl FnMut() -> bool) -> bool {
    let frist = Instant::now() + Duration::from_millis(millis);
    while Instant::now() < frist {
        if bedingung() {
            return true;
        }
        std::thread::sleep(Duration::from_millis(5));
    }
    bedingung()
}

/// Liest EINEN v3-gerahmten Frame und gibt seinen Payload als JSON.
pub(super) fn frame_json_lesen(c: &Testclient) -> Option<serde_json::Value> {
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

pub(super) fn frame_roh_lesen(c: &Testclient) -> Option<crate::transport::v3::Rahmen> {
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

/// Eine Senke, die in `p1`/`p2` blockiert — der Gegenspieler, ohne den
/// "der Leser haengt nicht an der Senke" keine pruefbare Aussage ist.
#[derive(Default)]
pub(super) struct BlockSenke {
    pub(super) zaehl: ZaehlSenke,
    pub(super) blockiert: AtomicBool,
    /// Steht gerade ein Aufruf IN der Senke? Ohne diese Zahl misst ein
    /// Test ueber den Senkenhang nur seine eigene Hoffnung.
    pub(super) in_senke: AtomicBool,
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
