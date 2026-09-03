use eqcop_broker::transport::pipetoken::PROBE_PRAEFIX;
use eqcop_broker::transport::server_v3::PIPE_INSTANZEN;
use eqcop_broker::{broker_idle_aktualisieren, BROKER_IDLE_ENDE_MS, BROKER_PRO_USER_MAX};
use std::io::{BufRead, BufReader, Read, Write};
use std::process::{Command, Stdio};
use std::time::{Duration, Instant};
use windows_sys::Win32::Foundation::{CloseHandle, HANDLE, INVALID_HANDLE_VALUE};
use windows_sys::Win32::Storage::FileSystem::{
    FILE_FLAG_FIRST_PIPE_INSTANCE, PIPE_ACCESS_DUPLEX,
};
use windows_sys::Win32::System::Pipes::{
    CreateNamedPipeW, PIPE_READMODE_BYTE, PIPE_REJECT_REMOTE_CLIENTS, PIPE_TYPE_BYTE, PIPE_WAIT,
};

struct FirstInstance(HANDLE);

impl FirstInstance {
    fn nehmen(name: &str) -> Option<Self> {
        let mut breit: Vec<u16> = name.encode_utf16().collect();
        breit.push(0);
        let h = unsafe {
            CreateNamedPipeW(
                breit.as_ptr(),
                PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                PIPE_INSTANZEN as u32,
                4096,
                4096,
                0,
                std::ptr::null(),
            )
        };
        if h == INVALID_HANDLE_VALUE {
            None
        } else {
            Some(Self(h))
        }
    }
}

impl Drop for FirstInstance {
    fn drop(&mut self) {
        unsafe { CloseHandle(self.0) };
    }
}

#[test]
fn letzter_client_idle_stop_an_grenze() {
    let basis = Instant::now();
    let mut idle_seit = Some(basis);
    assert!(!broker_idle_aktualisieren(
        &mut idle_seit,
        basis + Duration::from_millis(BROKER_IDLE_ENDE_MS - 1),
        0
    ));
    assert!(broker_idle_aktualisieren(
        &mut idle_seit,
        basis + Duration::from_millis(BROKER_IDLE_ENDE_MS),
        0
    ));
}

#[test]
fn aktiver_fremdclient_verhindert_stop() {
    let basis = Instant::now();
    let mut idle_seit = Some(basis);
    assert!(!broker_idle_aktualisieren(
        &mut idle_seit,
        basis + Duration::from_millis(BROKER_IDLE_ENDE_MS),
        BROKER_PRO_USER_MAX
    ));
    assert!(idle_seit.is_none());
}

#[test]
fn client_resetet_idlefrist() {
    let basis = Instant::now();
    let mut idle_seit = Some(basis);
    assert!(!broker_idle_aktualisieren(
        &mut idle_seit,
        basis + Duration::from_millis(BROKER_IDLE_ENDE_MS),
        1
    ));
    let neu = basis + Duration::from_millis(BROKER_IDLE_ENDE_MS + 1);
    assert!(!broker_idle_aktualisieren(&mut idle_seit, neu, 0));
    assert!(!broker_idle_aktualisieren(
        &mut idle_seit,
        neu + Duration::from_millis(BROKER_IDLE_ENDE_MS - 1),
        0
    ));
    assert!(broker_idle_aktualisieren(
        &mut idle_seit,
        neu + Duration::from_millis(BROKER_IDLE_ENDE_MS),
        0
    ));
}

#[test]
fn echter_brokerprozess_beendet_sich_nach_letztem_client_selbst() {
    let pipe = format!(
        "{PROBE_PRAEFIX}idle-process.{}.{}",
        std::process::id(),
        uuid::Uuid::new_v4().simple()
    );
    let mut child = Command::new(env!("CARGO_BIN_EXE_eqcop-broker-v3probe"))
        .arg(&pipe)
        .arg("90")
        .arg("--idle-self-exit")
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .expect("Probe-Brokerprozess starten");
    let _stdin_offen_halten = child.stdin.take().expect("Child-stdin");
    let stdout = child.stdout.take().expect("Child-stdout");
    let mut leser = BufReader::new(stdout);
    let mut bereit = String::new();
    leser.read_line(&mut bereit).expect("BEREIT lesen");
    assert_eq!(bereit.trim(), format!("BEREIT {pipe}"));

    let adresse = "0".repeat(32);
    let nonce = "1".repeat(32);
    let hello = format!(
        "{{\"type\":\"hello\",\"connection_kind\":\"control\",\"protocol\":3,\
         \"plugin_version\":\"0.3.0\",\"plugin_kind\":\"active_probe\",\
         \"adresse\":{{\"logon_sid\":\"S-1-5-21-1-2-3-1001\",\
         \"project_binding_id\":\"{adresse}\",\"session_epoch\":\"{adresse}\",\
         \"instance_id\":\"{adresse}\",\"runtime_nonce\":\"{nonce}\"}},\
         \"audio\":{{\"samplerate\":48000,\"block_size\":512,\"channels\":2}}}}"
    );
    let mut bootstrap = (hello.len() as u32).to_le_bytes().to_vec();
    bootstrap.extend_from_slice(hello.as_bytes());
    let mut client = std::fs::OpenOptions::new()
        .read(true)
        .write(true)
        .open(&pipe)
        .expect("echten Control-Client verbinden");
    client
        .write_all(&bootstrap)
        .expect("Control-Hello schreiben");
    client.flush().expect("Control-Hello flushen");
    std::thread::sleep(Duration::from_millis(250));
    drop(client);

    let idle_beginn = Instant::now();
    let frist = idle_beginn + Duration::from_secs(70);
    let status = loop {
        if let Some(status) = child.try_wait().expect("Childstatus") {
            break Some(status);
        }
        if Instant::now() >= frist {
            let _ = child.kill();
            let _ = child.wait();
            break None;
        }
        std::thread::sleep(Duration::from_millis(25));
    };
    assert!(
        status.is_some(),
        "Brokerprozess lief ueber seine 60-s-Idlefrist"
    );
    assert!(status.unwrap().success());
    let dauer = idle_beginn.elapsed();
    assert!(dauer >= Duration::from_millis(BROKER_IDLE_ENDE_MS));
    let mut bericht = String::new();
    leser.read_to_string(&mut bericht).unwrap();
    let bericht: serde_json::Value = serde_json::from_str(bericht.trim()).unwrap();
    assert_eq!(bericht["ende_grund"], "idle_self_exit");
    assert_eq!(bericht["besitzlistener_vor_stopp"], 2);
    assert_eq!(bericht["besitzlistener_nach_stopp"], 0);
    assert_eq!(bericht["control_verbindungen"], 1);
    assert_eq!(bericht["control_getrennt"], 1);
    let _name_frei = FirstInstance::nehmen(&pipe)
        .expect("idle_exit_schliesst_besitzlistener_zuletzt: Name nach Prozessende frei");
}

// ── NAK-121 H-09 ───────────────────────────────────────────────────────────

/// Der geordnete Stopp beendet den Storeschreiber, BEVOR der Prozess endet -
/// und zwar als letztes Glied, damit kein Weg mehr Auftraege einreicht, wenn
/// der Store zumacht.
///
/// Gemessen wird am Quelltext von `broker_stoppen`, weil der Brokerlauf in
/// einer statischen Zelle je Prozess genau einmal existiert: ein
/// Integrationstest kann ihn nicht starten, stoppen und danach erneut starten,
/// ohne den Prozess zu wechseln. Die Zusage ist eine REIHENFOLGE von vier
/// Entnahmen, und genau die ist im Quelltext eindeutig ablesbar.
#[test]
fn geordneter_stopp_schliesst_den_store() {
    let quelle = include_str!("../src/lib.rs");
    let anfang = quelle
        .find("pub fn broker_geordnet_stoppen")
        .expect("broker_geordnet_stoppen im Quelltext");
    let rumpf = &quelle[anfang..];
    let ende = rumpf
        .find("\npub fn broker_idle_ende_erreicht")
        .expect("Ende von broker_geordnet_stoppen");
    let rumpf = &rumpf[..ende];

    let pos = |nadel: &str| {
        rumpf
            .find(nadel)
            .unwrap_or_else(|| panic!("{nadel:?} fehlt im geordneten Stopp"))
    };
    let supervisor = pos("._supervisor");
    let v2 = pos("._griff_v2");
    let v3 = pos("._griff_v3");
    let store = pos(".store");

    assert!(
        supervisor < v2 && v2 < v3 && v3 < store,
        "Stoppreihenfolge verletzt: Supervisor {supervisor}, v2 {v2}, v3 {v3}, Store {store}"
    );
    // Und der Store wird ENTNOMMEN, nicht nur gelesen - nur dann laeuft sein
    // Destruktor und mit ihm StoreWriter::stoppen (Shutdown senden, joinen).
    let store_block = &rumpf[store..];
    assert!(
        store_block[..store_block.len().min(200)].contains(".take()"),
        "der Store wird beim geordneten Stopp nicht entnommen"
    );

    // Die Storesicht meldet den entnommenen Zustand ehrlich, statt auf einem
    // leeren Option zu panisieren.
    let sicht_anfang = quelle
        .find("pub fn broker_store_sicht")
        .expect("broker_store_sicht im Quelltext");
    let sicht = &quelle[sicht_anfang..sicht_anfang + 800];
    assert!(
        sicht.contains(".as_ref()") && sicht.contains(".map("),
        "broker_store_sicht liest den Store nicht optional"
    );
    assert!(
        !sicht.contains(".unwrap()") && !sicht.contains(".expect("),
        "broker_store_sicht panisiert auf dem entnommenen Store"
    );
}
