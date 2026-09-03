#![cfg(windows)]

//! NAK-123 / Entwurf §66.2 — negative Sicherheitsvektoren fuer den echten
//! v3-Named-Pipe-Listener. Jeder Test verwendet ausschliesslich einen
//! `PROBE_PRAEFIX`-Namen; die Produktionspipe ist hier unerreichbar.

use eqcop_broker::transport::pipetoken::PROBE_PRAEFIX;
use eqcop_broker::transport::server_v3::{
    v3_server_starten_fuer_security_vectors, V3AuthTestFehler, V3Griff, V3SecurityTestOptionen,
    V3StartTestFehler, V3TotesHandleNaht, V3UebergabeBarriere, ZaehlSenke, MAX_VERBINDUNGEN,
    PIPE_INSTANZEN,
};
use eqcop_broker::transport::v3::{
    envelope_pruefen, envelope_schreiben, Familie, MAX_BOOTSTRAP_BYTES,
};
use std::fs::{File, OpenOptions};
use std::io::{BufRead, BufReader, Read, Write};
use std::process::{Child, Command, Stdio};
use std::sync::atomic::{AtomicUsize, Ordering};
use std::sync::{Arc, Condvar, Mutex};
use std::time::{Duration, Instant};
use windows_sys::Win32::Foundation::{CloseHandle, HANDLE, INVALID_HANDLE_VALUE};
use windows_sys::Win32::Storage::FileSystem::{FILE_FLAG_FIRST_PIPE_INSTANCE, PIPE_ACCESS_DUPLEX};
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

fn probe_pipe(stichwort: &str) -> String {
    format!(
        "{PROBE_PRAEFIX}nak123.{stichwort}.{}.{}",
        std::process::id(),
        uuid::Uuid::new_v4().simple()
    )
}

fn warten(ms: u64, mut bedingung: impl FnMut() -> bool) -> bool {
    let ende = Instant::now() + Duration::from_millis(ms);
    while Instant::now() < ende {
        if bedingung() {
            return true;
        }
        std::thread::sleep(Duration::from_millis(10));
    }
    bedingung()
}

fn verbinden(name: &str) -> File {
    let ende = Instant::now() + Duration::from_secs(5);
    loop {
        match OpenOptions::new().read(true).write(true).open(name) {
            Ok(datei) => return datei,
            Err(fehler) if Instant::now() < ende => {
                let _ = fehler;
                std::thread::sleep(Duration::from_millis(10));
            }
            Err(fehler) => panic!("Probe-Client konnte {name} nicht verbinden: {fehler}"),
        }
    }
}

fn bootstrap(json: &str) -> Vec<u8> {
    let mut rahmen = (json.len() as u32).to_le_bytes().to_vec();
    rahmen.extend_from_slice(json.as_bytes());
    rahmen
}

fn adresse_json(nonce: char) -> String {
    let id = nonce.to_string().repeat(32);
    format!(
        "{{\"logon_sid\":\"S-1-5-21-1-2-3-1001\",\
         \"project_binding_id\":\"{id}\",\"session_epoch\":\"{id}\",\
         \"instance_id\":\"{id}\",\"runtime_nonce\":\"{id}\"}}"
    )
}

fn control_hello_json(typ: &str, connection_kind: &str, protocol: u32) -> String {
    format!(
        "{{\"type\":\"{typ}\",\"connection_kind\":\"{connection_kind}\",\"protocol\":{protocol},\
         \"plugin_version\":\"0.3.0\",\"plugin_kind\":\"active_probe\",\
         \"adresse\":{},\"audio\":{{\"samplerate\":48000,\"block_size\":512,\
         \"channels\":2}}}}",
        adresse_json('a')
    )
}

fn control_hello() -> Vec<u8> {
    bootstrap(&control_hello_json("hello", "control", 3))
}

fn telemetry_hello(link: &str, challenge: &str) -> Vec<u8> {
    bootstrap(&format!(
        "{{\"type\":\"hello\",\"connection_kind\":\"telemetry\",\"protocol\":3,\
         \"plugin_version\":\"0.3.0\",\"adresse\":{},\"link_id\":\"{link}\",\
         \"challenge\":\"{challenge}\"}}",
        adresse_json('a')
    ))
}

fn frame_lesen(datei: &mut File) -> serde_json::Value {
    let mut kopf = [0u8; 4];
    datei.read_exact(&mut kopf).expect("Framepraefix lesen");
    let n = u32::from_le_bytes(kopf) as usize;
    assert!(n > 0 && n <= 1024 * 1024);
    let mut draht = kopf.to_vec();
    draht.resize(4 + n, 0);
    datei
        .read_exact(&mut draht[4..])
        .expect("vollstaendigen Frame lesen");
    let rahmen = envelope_pruefen(&draht).expect("gueltiger v3-Frame");
    serde_json::from_slice(&rahmen.payload).expect("JSON-Payload")
}

fn bootstrap_reject_lesen(datei: &mut File) -> serde_json::Value {
    let mut kopf = [0u8; 4];
    datei.read_exact(&mut kopf).expect("Rejectpraefix lesen");
    let n = u32::from_le_bytes(kopf) as usize;
    let mut payload = vec![0u8; n];
    datei.read_exact(&mut payload).expect("Reject lesen");
    serde_json::from_slice(&payload).expect("Reject ist JSON")
}

fn start(name: &str, optionen: V3SecurityTestOptionen) -> (V3Griff, Arc<ZaehlSenke>) {
    let senke = Arc::new(ZaehlSenke::default());
    let griff = v3_server_starten_fuer_security_vectors(name, senke.clone(), optionen)
        .unwrap_or_else(|e| panic!("Security-Vector-Listener starten: {e}"));
    (griff, senke)
}

fn spur_position(spur: &[&str], schritt: &str) -> usize {
    spur.iter()
        .position(|s| *s == schritt)
        .unwrap_or_else(|| panic!("Schritt {schritt:?} fehlt in {spur:?}"))
}

fn assert_reihenfolge(spur: &[&str], schritte: &[&str]) {
    let mut vorher = None;
    for schritt in schritte {
        let jetzt = spur_position(spur, schritt);
        if let Some(v) = vorher {
            assert!(v < jetzt, "Reihenfolge {schritte:?} in {spur:?}");
        }
        vorher = Some(jetzt);
    }
}

fn assert_keine_fachlogik(senke: &ZaehlSenke) {
    assert_eq!(senke.control_verbindungen.load(Ordering::SeqCst), 0);
    assert_eq!(senke.telemetrie_verbindungen.load(Ordering::SeqCst), 0);
    assert_eq!(senke.p0.load(Ordering::SeqCst), 0);
    assert_eq!(senke.p1.load(Ordering::SeqCst), 0);
    assert_eq!(senke.p2.load(Ordering::SeqCst), 0);
}

fn child_starten(name: &str, extra: Option<&str>) -> (Child, BufReader<std::process::ChildStdout>) {
    let mut befehl = Command::new(env!("CARGO_BIN_EXE_eqcop-broker-v3probe"));
    befehl
        .arg(name)
        .arg("90")
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::piped());
    if let Some(extra) = extra {
        befehl.arg(extra);
    }
    let mut child = befehl.spawn().expect("v3-Probeprozess starten");
    let stdout = child.stdout.take().expect("Child-stdout");
    let mut leser = BufReader::new(stdout);
    let mut bereit = String::new();
    leser.read_line(&mut bereit).expect("BEREIT lesen");
    assert_eq!(bereit.trim(), format!("BEREIT {name}"));
    (child, leser)
}

#[test]
fn start_freier_name_bewaffnet_zwei_listener_vor_veroeffentlichung() {
    let pipe = probe_pipe("a01-ok");
    let threads = Arc::new(AtomicUsize::new(0));
    let (mut griff, _) = start(
        &pipe,
        V3SecurityTestOptionen {
            hilfsthread_zaehler: Some(threads.clone()),
            ..V3SecurityTestOptionen::default()
        },
    );
    assert!(warten(2000, || threads.load(Ordering::SeqCst) == 2));
    assert_eq!(
        griff.statistik.bewaffnete_listener.load(Ordering::SeqCst),
        2
    );
    assert!(FirstInstance::nehmen(&pipe).is_none());
    griff.stoppen();
    assert_eq!(threads.load(Ordering::SeqCst), 0);
}

#[test]
fn startfehler_nach_erstem_handle_raeumt_handles_threads_und_namen_vor_publish() {
    for (name, fehler) in [
        ("direkt", V3StartTestFehler::DirektNachErstemHandle),
        ("zweite-instanz", V3StartTestFehler::ZweiteInstanz),
        ("zweite-armierung", V3StartTestFehler::ZweiteArmierung),
        ("wachhund", V3StartTestFehler::WachhundSpawn),
        ("acceptor", V3StartTestFehler::AcceptorSpawn),
    ] {
        let pipe = probe_pipe(name);
        let threads = Arc::new(AtomicUsize::new(0));
        let senke = Arc::new(ZaehlSenke::default());
        let ergebnis = v3_server_starten_fuer_security_vectors(
            &pipe,
            senke,
            V3SecurityTestOptionen {
                start_fehler: fehler,
                hilfsthread_zaehler: Some(threads.clone()),
                ..V3SecurityTestOptionen::default()
            },
        );
        assert!(
            ergebnis.is_err(),
            "Fehlerkante {fehler:?} publizierte einen Griff"
        );
        assert_eq!(threads.load(Ordering::SeqCst), 0, "Fehlerkante {fehler:?}");
        let _wieder_frei = FirstInstance::nehmen(&pipe)
            .unwrap_or_else(|| panic!("Fehlerkante {fehler:?} liess den Namen besetzt"));
    }
}

#[test]
fn start_belegter_name_scheitert_ohne_zweiten_broker() {
    let pipe = probe_pipe("a02");
    let _fremd = FirstInstance::nehmen(&pipe).expect("Fremdserver belegt Namen");
    let senke = Arc::new(ZaehlSenke::default());
    assert!(v3_server_starten_fuer_security_vectors(
        &pipe,
        senke,
        V3SecurityTestOptionen::default()
    )
    .is_err());
}

#[test]
fn accept_bewaffnet_nachfolger_vor_worker_uebergabe_bei_zwei_listenern() {
    let pipe = probe_pipe("a03");
    let barriere = Arc::new(V3UebergabeBarriere::default());
    let (mut griff, _) = start(
        &pipe,
        V3SecurityTestOptionen {
            uebergabe_barriere: Some(barriere.clone()),
            ..V3SecurityTestOptionen::default()
        },
    );
    let client = verbinden(&pipe);
    assert!(warten(3000, || barriere.erreicht()));
    assert_eq!(griff.statistik.worker_uebergaben.load(Ordering::SeqCst), 0);
    assert_eq!(
        griff.statistik.bewaffnete_listener.load(Ordering::SeqCst),
        2
    );
    assert!(FirstInstance::nehmen(&pipe).is_none());
    barriere.freigeben();
    assert!(warten(3000, || griff
        .statistik
        .worker_uebergaben
        .load(Ordering::SeqCst)
        == 1));
    drop(client);
    griff.stoppen();
}

#[test]
fn ersatzlistener_fehler_behaelt_besitz_und_meldet_nicht_bereit() {
    let pipe = probe_pipe("a04");
    let (mut griff, _) = start(
        &pipe,
        V3SecurityTestOptionen {
            ersatzlistener_fehler: true,
            ..V3SecurityTestOptionen::default()
        },
    );
    let client = verbinden(&pipe);
    assert!(warten(3000, || griff
        .statistik
        .listener_fehler
        .load(Ordering::SeqCst)
        == 1));
    assert!(griff.fataler_listenerfehler());
    assert_eq!(griff.statistik.worker_uebergaben.load(Ordering::SeqCst), 0);
    assert_eq!(
        griff.statistik.bewaffnete_listener.load(Ordering::SeqCst),
        1
    );
    assert!(FirstInstance::nehmen(&pipe).is_none());
    griff.stoppen();
    drop(client);
    let _frei = FirstInstance::nehmen(&pipe).expect("Name erst nach geordnetem Stopp frei");
}

#[test]
fn zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz() {
    let pipe = probe_pipe("a05");
    let (mut griff, _) = start(&pipe, V3SecurityTestOptionen::default());
    let mut clients = Vec::with_capacity(MAX_VERBINDUNGEN);
    for _ in 0..MAX_VERBINDUNGEN {
        clients.push(verbinden(&pipe));
    }
    assert!(warten(4000, || griff.aktive_worker() == MAX_VERBINDUNGEN as u64));
    let ueber_cap = verbinden(&pipe);
    assert!(warten(3000, || griff
        .statistik
        .am_worker_cap_abgewiesen
        .load(Ordering::SeqCst)
        == 1));
    assert_eq!(griff.aktive_worker(), MAX_VERBINDUNGEN as u64);
    assert_eq!(
        griff.statistik.worker_uebergaben.load(Ordering::SeqCst),
        MAX_VERBINDUNGEN as u64
    );
    assert_eq!(
        griff.statistik.bewaffnete_listener.load(Ordering::SeqCst),
        2
    );
    assert!(FirstInstance::nehmen(&pipe).is_none());
    drop(ueber_cap);
    drop(clients);
    assert!(warten(8000, || griff.aktive_worker() == 0));
    griff.stoppen();
}

#[test]
fn prozesskill_gibt_name_frei() {
    let pipe = probe_pipe("a07");
    let (mut child, _stdout) = child_starten(&pipe, None);
    assert!(FirstInstance::nehmen(&pipe).is_none());
    child.kill().expect("Probeprozess hart beenden");
    child.wait().expect("Probeprozess reap");
    let _frei = FirstInstance::nehmen(&pipe).expect("Prozesskill gab Pipenamen nicht frei");
}

#[test]
fn neustart_besetzt_name_vor_bereit() {
    let pipe = probe_pipe("a08");
    let (mut alt, _) = start(&pipe, V3SecurityTestOptionen::default());
    alt.stoppen();
    let fremd = FirstInstance::nehmen(&pipe).expect("Name nach altem Broker frei");
    let senke = Arc::new(ZaehlSenke::default());
    assert!(v3_server_starten_fuer_security_vectors(
        &pipe,
        senke,
        V3SecurityTestOptionen::default()
    )
    .is_err());
    drop(fremd);
    let (mut neu, _) = start(&pipe, V3SecurityTestOptionen::default());
    assert_eq!(neu.statistik.bewaffnete_listener.load(Ordering::SeqCst), 2);
    neu.stoppen();
}

#[test]
fn stoppen_schliesst_besitzlistener_zuletzt_und_neustart_ist_sofort_moeglich() {
    let pipe = probe_pipe("a09");
    let (mut griff, senke) = start(&pipe, V3SecurityTestOptionen::default());
    let mut client = verbinden(&pipe);
    client.write_all(&control_hello()).unwrap();
    assert_eq!(frame_lesen(&mut client)["type"], "welcome");
    assert!(warten(2000, || senke
        .control_verbindungen
        .load(Ordering::SeqCst)
        == 1));
    assert_eq!(griff.aktive_worker(), 1);
    assert!(FirstInstance::nehmen(&pipe).is_none());
    griff.stoppen();
    assert_eq!(griff.aktive_worker(), 0);
    assert_eq!(griff.gehaltene_handles(), 0);
    griff.stoppen();
    assert_eq!(
        griff.statistik.bewaffnete_listener.load(Ordering::SeqCst),
        0
    );
    assert_reihenfolge(&griff.sicherheits_spur(), &["close", "listeners_close"]);
    drop(client);
    let frei = FirstInstance::nehmen(&pipe).expect("Name nach idempotentem Stopp frei");
    drop(frei);
    let (mut neu, _) = start(&pipe, V3SecurityTestOptionen::default());
    neu.stoppen();
}

#[test]
fn gleiche_sid_wird_erst_nach_revert_angenommen() {
    let pipe = probe_pipe("b01");
    let (mut griff, senke) = start(&pipe, V3SecurityTestOptionen::default());
    let mut client = verbinden(&pipe);
    client.write_all(&control_hello()).unwrap();
    let welcome = frame_lesen(&mut client);
    assert_eq!(welcome["type"], "welcome");
    assert!(warten(2000, || senke
        .control_verbindungen
        .load(Ordering::SeqCst)
        == 1));
    let spur = griff.sicherheits_spur();
    assert_reihenfolge(
        &spur,
        &[
            "connect",
            "read",
            "impersonate",
            "sid",
            "revert",
            "self",
            "hello_accept",
        ],
    );
    drop(client);
    griff.stoppen();
}

#[test]
fn impersonation_fehlschlag_schliesst_ohne_serverkontext() {
    auth_fehler_schliesst(
        "b02",
        V3AuthTestFehler::Impersonate,
        &["read", "impersonate", "revert", "close"],
    );
}

fn auth_fehler_schliesst(name: &str, fehler: V3AuthTestFehler, folge: &[&str]) {
    let pipe = probe_pipe(name);
    let (mut griff, senke) = start(
        &pipe,
        V3SecurityTestOptionen {
            auth_fehler: fehler,
            ..V3SecurityTestOptionen::default()
        },
    );
    let mut client = verbinden(&pipe);
    client.write_all(&control_hello()).unwrap();
    assert!(warten(3000, || griff.sicherheits_spur().contains(&"close")));
    assert_reihenfolge(&griff.sicherheits_spur(), folge);
    assert_keine_fachlogik(&senke);
    griff.stoppen();
}

#[test]
fn tokenabfragefehler_revertiert_und_schliesst_ohne_senke() {
    for (name, fehler, mit_sid) in [
        ("open-token", V3AuthTestFehler::OpenThreadToken, false),
        ("token-groesse", V3AuthTestFehler::TokenGroesse, false),
        ("token-info", V3AuthTestFehler::TokenInformation, false),
        ("sid-ungueltig", V3AuthTestFehler::SidUngueltig, true),
    ] {
        let folge = if mit_sid {
            vec!["read", "impersonate", "sid", "revert", "close"]
        } else {
            vec!["read", "impersonate", "revert", "close"]
        };
        auth_fehler_schliesst(name, fehler, &folge);
    }
}

#[test]
fn fremde_sid_revertiert_und_erreicht_keinen_coordinator() {
    auth_fehler_schliesst(
        "b04",
        V3AuthTestFehler::SidFremd,
        &["read", "impersonate", "sid", "revert", "close"],
    );
}

#[test]
fn vollstaendiges_abzulehnendes_hello_impersoniert_prueft_sid_und_revertiert_vor_reject() {
    let faelle = [
        ("json", "{".to_string()),
        ("typ", control_hello_json("welcome", "control", 3)),
        ("kind", control_hello_json("hello", "fremd", 3)),
        ("protokoll", control_hello_json("hello", "control", 2)),
    ];
    for (name, json) in faelle {
        let pipe = probe_pipe(name);
        let (mut griff, senke) = start(&pipe, V3SecurityTestOptionen::default());
        let mut client = verbinden(&pipe);
        client.write_all(&bootstrap(&json)).unwrap();
        let reject = bootstrap_reject_lesen(&mut client);
        assert_eq!(reject["type"], "reject");
        assert!(warten(2000, || griff.sicherheits_spur().contains(&"close")));
        assert_reihenfolge(
            &griff.sicherheits_spur(),
            &["read", "impersonate", "sid", "revert", "reject", "close"],
        );
        assert_keine_fachlogik(&senke);
        assert!(!griff.sicherheits_spur().contains(&"hello_accept"));
        griff.stoppen();
    }
}

#[test]
fn bootstrap_timeout_und_lesefehler_revertieren_jeweils_ohne_serverweiterlauf() {
    enum Fall {
        Eof,
        Teilpraefix,
        Uebergroesse,
        Timeout,
    }
    for (name, fall) in [
        ("eof", Fall::Eof),
        ("teilpraefix", Fall::Teilpraefix),
        ("uebergroesse", Fall::Uebergroesse),
        ("timeout", Fall::Timeout),
    ] {
        let pipe = probe_pipe(name);
        let (mut griff, senke) = start(&pipe, V3SecurityTestOptionen::default());
        let mut client = verbinden(&pipe);
        match fall {
            Fall::Eof => drop(client),
            Fall::Teilpraefix => {
                client.write_all(&[4, 0]).unwrap();
                drop(client);
            }
            Fall::Uebergroesse => {
                client
                    .write_all(&(MAX_BOOTSTRAP_BYTES + 1).to_le_bytes())
                    .unwrap();
            }
            Fall::Timeout => {}
        }
        assert!(warten(7000, || griff.sicherheits_spur().contains(&"close")));
        let spur = griff.sicherheits_spur();
        assert_reihenfolge(&spur, &["connect", "revert", "close"]);
        assert!(!spur.contains(&"impersonate"));
        assert!(!spur.contains(&"sid"));
        assert!(!spur.contains(&"hello_accept"));
        assert_keine_fachlogik(&senke);
        griff.stoppen();
    }

    let pipe = probe_pipe("cancel");
    let (mut griff, senke) = start(&pipe, V3SecurityTestOptionen::default());
    let client = verbinden(&pipe);
    assert!(warten(2000, || {
        griff.statistik.worker_uebergaben.load(Ordering::SeqCst) == 1
            && griff.gehaltene_handles() == 1
    }));
    griff.stoppen();
    let spur = griff.sicherheits_spur();
    assert_reihenfolge(&spur, &["connect", "revert", "close", "listeners_close"]);
    assert!(!spur.contains(&"impersonate"));
    assert!(!spur.contains(&"sid"));
    assert!(!spur.contains(&"hello_accept"));
    assert_keine_fachlogik(&senke);
    drop(client);
}

#[test]
fn revert_fehlschlag_beendet_testbrokerprozess_fail_fast() {
    let pipe = probe_pipe("b07");
    let (mut child, _stdout) = child_starten(&pipe, Some("--security-revert-fail"));
    let mut client = verbinden(&pipe);
    client.write_all(&control_hello()).unwrap();
    let ende = Instant::now() + Duration::from_secs(10);
    let status = loop {
        if let Some(status) = child.try_wait().expect("Childstatus") {
            break status;
        }
        assert!(
            Instant::now() < ende,
            "Revert-Fehler liess den Broker weiterlaufen"
        );
        std::thread::sleep(Duration::from_millis(20));
    };
    assert!(!status.success());
    let _frei = FirstInstance::nehmen(&pipe).expect("Fail-fast schloss Prozesshandles nicht");
}

#[test]
fn control_und_telemetry_durchlaufen_die_identische_sid_kette() {
    let pipe = probe_pipe("b08");
    let (mut griff, senke) = start(&pipe, V3SecurityTestOptionen::default());
    let mut control = verbinden(&pipe);
    control.write_all(&control_hello()).unwrap();
    let welcome = frame_lesen(&mut control);
    let link = welcome["link_id"].as_str().unwrap();
    let challenge = welcome["challenge"].as_str().unwrap();
    let mut telemetrie = verbinden(&pipe);
    telemetrie
        .write_all(&telemetry_hello(link, challenge))
        .unwrap();
    assert_eq!(frame_lesen(&mut telemetrie)["type"], "welcome");
    assert!(warten(2000, || senke
        .telemetrie_verbindungen
        .load(Ordering::SeqCst)
        == 1));
    let spur = griff.sicherheits_spur();
    for schritt in [
        "read",
        "impersonate",
        "sid",
        "revert",
        "self",
        "hello_accept",
    ] {
        assert_eq!(
            spur.iter().filter(|s| **s == schritt).count(),
            2,
            "{schritt}: {spur:?}"
        );
    }
    for vorkommen in 0..2 {
        let position = |schritt: &str| {
            spur.iter()
                .enumerate()
                .filter(|(_, s)| **s == schritt)
                .nth(vorkommen)
                .map(|(i, _)| i)
                .unwrap_or_else(|| panic!("{vorkommen}. {schritt} fehlt in {spur:?}"))
        };
        let folge = [
            "read",
            "impersonate",
            "sid",
            "revert",
            "self",
            "hello_accept",
        ];
        for paar in folge.windows(2) {
            assert!(
                position(paar[0]) < position(paar[1]),
                "{folge:?} in {spur:?}"
            );
        }
    }
    drop(telemetrie);
    drop(control);
    griff.stoppen();
}

#[test]
fn nach_revert_bleiben_envelope_und_trennpfade_im_self_kontext() {
    let pipe = probe_pipe("b09");
    let (mut griff, senke) = start(&pipe, V3SecurityTestOptionen::default());
    let mut client = verbinden(&pipe);
    client.write_all(&control_hello()).unwrap();
    assert_eq!(frame_lesen(&mut client)["type"], "welcome");
    let frame = envelope_schreiben(
        Familie::P0,
        1,
        br#"{"type":"security_vector_after_revert"}"#,
    )
    .unwrap();
    client.write_all(&frame).unwrap();
    assert!(warten(2000, || senke.p0.load(Ordering::SeqCst) == 1));
    drop(client);
    assert!(warten(3000, || griff.sicherheits_spur().contains(&"close")));
    let spur = griff.sicherheits_spur();
    assert_eq!(spur.iter().filter(|s| **s == "impersonate").count(), 1);
    assert_eq!(spur.iter().filter(|s| **s == "revert").count(), 1);
    assert_reihenfolge(&spur, &["revert", "self", "hello_accept", "close"]);
    griff.stoppen();
}

// ── A-06 Nebenlaeufigkeit: EINE Zaehlerwahrheit ────────────────────────────
//
// NAK-123 R1 / P1. Der Idle-Selbstende-Pfad (`broker_soll_idle_enden`) fragt
// `V3Griff::aktive_worker`. Solange diese Zahl aus einem gelesenen Vorwert
// fortgeschrieben wurde, konnten zwei verschraenkte Freigaben sie dauerhaft
// falsch stehen lassen. Beide Tests hier verschraenken deterministisch ueber
// eine Condvar-Barriere — kein Sleep, kein Raten.

/// Haelt einen Thread an einer benannten Stelle an, bis ein anderer ihn
/// ausdruecklich freigibt. Beide Richtungen haben eine Frist: ein Test darf
/// hier nie unbegrenzt haengen.
#[derive(Default)]
struct Haltepunkt {
    angekommen: (Mutex<bool>, Condvar),
    frei: (Mutex<bool>, Condvar),
}

impl Haltepunkt {
    const FRIST: Duration = Duration::from_secs(10);

    /// Laeuft IM anzuhaltenden Thread.
    fn halten(&self) {
        {
            let (sperre, signal) = &self.angekommen;
            *sperre.lock().unwrap_or_else(|e| e.into_inner()) = true;
            signal.notify_all();
        }
        let (sperre, signal) = &self.frei;
        let mut frei = sperre.lock().unwrap_or_else(|e| e.into_inner());
        while !*frei {
            let (neu, ablauf) = signal
                .wait_timeout(frei, Self::FRIST)
                .unwrap_or_else(|e| e.into_inner());
            frei = neu;
            assert!(!ablauf.timed_out(), "Haltepunkt wurde nie freigegeben");
        }
    }

    fn warte_bis_angekommen(&self) {
        let (sperre, signal) = &self.angekommen;
        let mut da = sperre.lock().unwrap_or_else(|e| e.into_inner());
        while !*da {
            let (neu, ablauf) = signal
                .wait_timeout(da, Self::FRIST)
                .unwrap_or_else(|e| e.into_inner());
            da = neu;
            assert!(!ablauf.timed_out(), "Haltepunkt wurde nie erreicht");
        }
    }

    fn freigeben(&self) {
        let (sperre, signal) = &self.frei;
        *sperre.lock().unwrap_or_else(|e| e.into_inner()) = true;
        signal.notify_all();
    }
}

fn haltend(punkt: &Arc<Haltepunkt>) -> Option<Arc<dyn Fn() + Send + Sync>> {
    let punkt = punkt.clone();
    Some(Arc::new(move || punkt.halten()) as Arc<dyn Fn() + Send + Sync>)
}

#[test]
fn zwei_verschraenkte_freigaben_lassen_aktive_worker_auf_null() {
    let pipe = probe_pipe("a06-zwei-drops");
    let (mut griff, _) = start(&pipe, V3SecurityTestOptionen::default());

    let haltepunkt = Arc::new(Haltepunkt::default());
    let a = griff
        .worker_platz_probe(haltend(&haltepunkt))
        .expect("erster Workerplatz");
    let b = griff.worker_platz_probe(None).expect("zweiter Workerplatz");
    assert_eq!(griff.aktive_worker(), 2, "zwei belegte Plaetze");

    // A gibt frei und haelt NACH der Zaehlerfreigabe an (2 -> 1).
    let langsam = std::thread::spawn(move || drop(a));
    haltepunkt.warte_bis_angekommen();

    // B laeuft in dieser Luecke vollstaendig durch (1 -> 0).
    drop(b);

    // Erst jetzt darf A zu Ende laufen. Ein nachlaufender Schreibschritt
    // wuerde hier den veralteten Stand 1 zurueckbringen.
    haltepunkt.freigeben();
    langsam.join().expect("Freigabethread");

    assert_eq!(
        griff.aktive_worker(),
        0,
        "nach beiden Freigaben lebt kein Worker mehr; ein Reststand haelt den \
         Broker dauerhaft von seinem Idle-Selbstende ab"
    );
    griff.stoppen();
}

#[test]
fn freigabe_gegen_reserve_laesst_den_lebenden_worker_stehen() {
    let pipe = probe_pipe("a06-drop-gegen-reserve");
    let (mut griff, _) = start(&pipe, V3SecurityTestOptionen::default());

    let haltepunkt = Arc::new(Haltepunkt::default());
    let a = griff
        .worker_platz_probe(haltend(&haltepunkt))
        .expect("erster Workerplatz");
    assert_eq!(griff.aktive_worker(), 1);

    // A gibt frei (1 -> 0) und haelt an.
    let langsam = std::thread::spawn(move || drop(a));
    haltepunkt.warte_bis_angekommen();

    // In dieser Luecke belegt eine neue Verbindung den Platz (0 -> 1).
    let b = griff
        .worker_platz_probe(None)
        .expect("Reserve in der Freigabeluecke");

    haltepunkt.freigeben();
    langsam.join().expect("Freigabethread");

    assert_eq!(
        griff.aktive_worker(),
        1,
        "der frisch reservierte Worker lebt; eine 0 hier beendet den Broker \
         nach der Idle-Frist unter einem laufenden Client"
    );
    drop(b);
    assert_eq!(griff.aktive_worker(), 0);
    griff.stoppen();
}

// ── NAK-121 H-01 und H-02 ──────────────────────────────────────────────────
//
// Beide Zeilen sind EIN Arbeitsstueck: der `Verbindungsgriff` besitzt seither
// Registereintrag und Handle zusammen, und genau daraus folgt sowohl die
// Reihenfolge Austrag-vor-Close (H-01) als auch der Abbruchweg fuer einen
// abgeloesten Schreiber (H-02).
//
// Die vier Zusagen liegen in EINEM Test, nicht in vieren. Grund ist der
// Nachbar `zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz`: er
// oeffnet 96 Pipe-Clients seriell und wartet dann 4000 ms darauf, dass alle 96
// Worker aktiv sind. Unter paralleler Last dauert jedes einzelne `open`
// laenger, und 96 kleine Verzoegerungen summieren sich ueber die Frist.
// Gemessen: mit 20 Tests in dieser Datei vier von vier Laeufen gruen, mit 21
// ebenfalls vier von vier, mit 23 und 24 je einer von drei bzw. fuenf ROT. Der
// Nachbar ist nach Paragraph 3.4 woertlich unveraenderlich, also weicht ihm der
// neue Test aus, statt ihn zu entschaerfen - eine Fristverlaengerung dort waere
// eine Aenderung an einem Test, den die Spezifikation ausdruecklich schuetzt.

#[test]
fn verbindungsgriff_besitzt_handle_und_registereintrag() {
    // ── H-01, Teil 1: die Reihenfolge Austrag vor Close ────────────────────
    let pipe = probe_pipe("h01a");
    let fenster = Arc::new(V3UebergabeBarriere::default());
    let (mut griff, _) = start(
        &pipe,
        V3SecurityTestOptionen {
            destruktor_fenster: Some(fenster.clone()),
            ..V3SecurityTestOptionen::default()
        },
    );
    let client = verbinden(&pipe);
    assert!(warten(3000, || griff.aktive_worker() == 1));
    assert_eq!(griff.gehaltene_handles(), 1);
    drop(client);

    // Der Destruktor haelt GENAU zwischen Registeraustrag und CloseHandle. Die
    // Zusage von H-01 ist beobachtbar, weil in diesem Fenster beides zugleich
    // gilt: das Handle ist noch offen, und das Register fuehrt es nicht mehr.
    // Eine reine Spur-Assertion koennte das nicht messen - sie sieht nur, wo
    // zwei push-Aufrufe stehen, nicht wo CloseHandle steht.
    let fenster_erreicht = warten(5000, || fenster.erreicht());
    // Erst MESSEN, dann FREIGEBEN, dann werten: eine Assertion vor der
    // Freigabe liesse den wartenden Destruktorthread fuer immer stehen und
    // haengte den ganzen Testlauf auf, statt ihn rot zu machen.
    let im_fenster_gehalten = griff.gehaltene_handles();
    let spur_im_fenster = griff.sicherheits_spur();
    fenster.freigeben();

    assert!(fenster_erreicht, "Destruktor erreichte sein Fenster nicht");
    assert_eq!(
        im_fenster_gehalten, 0,
        "das Register fuehrte den Eintrag noch, als das Handle gleich geschlossen wurde"
    );
    assert!(!spur_im_fenster.contains(&"close"));
    assert!(spur_im_fenster.contains(&"register_austrag"));

    assert!(warten(3000, || griff.sicherheits_spur().contains(&"close")));
    assert_reihenfolge(&griff.sicherheits_spur(), &["register_austrag", "close"]);
    assert_eq!(griff.gehaltene_handles(), 0);
    // Gegenprobe zum Zaehler unten: im Normalbetrieb ist er strukturell null,
    // weil ein Registereintrag nur lebt, solange sein Besitzer lebt.
    assert_eq!(
        griff.statistik.cancel_auf_totem_handle.load(Ordering::SeqCst),
        0
    );
    griff.stoppen();

    // ── H-01, Teil 2: ein Abbruch auf ein totes Handle wird gezaehlt ───────
    let pipe = probe_pipe("h01b");
    let (mut griff, _) = start(
        &pipe,
        V3SecurityTestOptionen {
            totes_handle_ins_register: V3TotesHandleNaht::OffenUndAbgeloest,
            ..V3SecurityTestOptionen::default()
        },
    );
    // Der Wachhund feuert bei jedem Tick auf die Phantom-ID. Ein unerwarteter
    // Fehlschlag - im Regelfall ERROR_INVALID_HANDLE - wird gezaehlt statt
    // verschluckt; genau dieses Signal verlangt H-01.
    assert!(warten(5000, || griff
        .statistik
        .cancel_auf_totem_handle
        .load(Ordering::SeqCst)
        > 0));
    griff.stoppen();

    // ── H-01, Teil 3 (R2-5): derselbe Zaehler DURCH `V3Griff::stoppen` ─────
    //
    // D11 der Runde 1 belegte den Stopp-Pfad mit einem Inline-Test, der
    // `alle_io_abbrechen` selbst aufrief. Entkoppelte man den Aufruf aus
    // `V3Griff::stoppen`, blieb dieser Test gruen und der zugesagte Pfad war
    // wieder ungeprueft. Jetzt geht der Beweis durch `stoppen()`.
    //
    // Die Phantom-ID liegt NUR in `offen`: der Wachhund erreicht sie damit
    // nicht (er bricht nur faellige Bootstraps und abgeloeste IDs ab), der
    // Stopp-Pfad ueber `alle_io_abbrechen` schon. Nur so ist der Stopp
    // isolierbar.
    let pipe = probe_pipe("h01c");
    let (mut griff, _) = start(
        &pipe,
        V3SecurityTestOptionen {
            totes_handle_ins_register: V3TotesHandleNaht::NurOffen,
            ..V3SecurityTestOptionen::default()
        },
    );
    // Gegenprobe: mehrere Wachhundtakte (WACHHUND_TAKT = 100 ms) lang bleibt
    // der Zaehler 0. Waere die ID auch in `abgeloest`, haette der Wachhund
    // hier schon gezaehlt und der Stopp bewiese nichts mehr.
    std::thread::sleep(Duration::from_millis(500));
    assert_eq!(
        griff.statistik.cancel_auf_totem_handle.load(Ordering::SeqCst),
        0,
        "der Wachhund hat die Phantom-ID erreicht - der Stopp-Pfad ist nicht isoliert"
    );

    griff.stoppen();
    assert!(
        griff.statistik.cancel_auf_totem_handle.load(Ordering::SeqCst) >= 1,
        "der Stopp-Pfad hat den unerwarteten CancelIoEx-Fehler verschluckt"
    );

    // ── H-02: der abgeloeste Schreiber gibt Instanz und Handle zurueck ─────
    let pipe = probe_pipe("h02");
    let haengt = Arc::new(std::sync::atomic::AtomicBool::new(true));
    let (mut griff, _) = start(
        &pipe,
        V3SecurityTestOptionen {
            schreiber_haengt: Some(haengt.clone()),
            ..V3SecurityTestOptionen::default()
        },
    );
    let mut client = verbinden(&pipe);
    assert!(warten(3000, || griff.aktive_worker() == 1));

    // Erst ein GUELTIGER Bootstrap: der Schreiberthread entsteht ueberhaupt
    // erst danach. Er bleibt sofort an der Naht stehen.
    client.write_all(&control_hello()).expect("Hello schreiben");
    client.flush().ok();
    assert!(warten(5000, || griff
        .sicherheits_spur()
        .contains(&"hello_accept")));
    drop(client);

    // Der Join verpasst seine SENKE_FRIST, die Verbindung wird abgeloest.
    assert!(warten(20_000, || griff
        .statistik
        .schreiber_abgeloest
        .load(Ordering::SeqCst)
        > 0));

    // H-02: der Registereintrag ueberlebt den abgeloesten Schreiber, UND der
    // Wachhund erreicht ihn ueber `HandleRegister::abgeloest`. Das Testflag
    // wird BEWUSST nicht zurueckgesetzt - der Schreiber endet ausschliesslich
    // ueber diesen Weg, sonst misst der Test nur sich selbst. Sobald er endet,
    // faellt der letzte geteilte Griff und traegt aus. Vor NAK-121 blieb die
    // Pipeinstanz bis zum Serverstopp verbrannt, weil `BootstrapFrist` die ID
    // laengst ausgetragen hatte und der Wachhund nur faellige Bootstraps sah.
    //
    // R2-4 (Nacharbeit Runde 2, 03.09.2026): der mit D14 eingefuehrte Zaehler
    // `abgeloest_abbrueche` bekam keinen Gegenweg im Destruktor.
    //
    // Diese Frist misst BEIDES. Der abgeloeste Schreiber endet nur, wenn er
    // seinen Wachhundabbruch am Zaehler SIEHT; seine Notbremse liegt bei 30 s
    // (verbindung.rs). Kommt das Handle also innerhalb von 10 s zurueck, hat
    // die D14-Naht ihren Abbruch gelesen - das Abraeumen hungert sie nicht aus,
    // weil der letzte geteilte Griff erst NACH dem Threadende faellt. Der
    // Zaehler selbst wird hier bewusst NICHT gepollt: er ist ein transienter
    // Zustand, den derselbe Destruktor gleich wieder raeumt.
    assert!(
        warten(10_000, || griff.gehaltene_handles() == 0),
        "abgeloester Schreiber gab sein Handle nicht zurueck"
    );
    // Verbinden gegen Trennen, vollstaendig: keine der drei Mengen fuehrt die
    // ID noch. Vorher wuchs `abgeloest_abbrueche` unbegrenzt weiter.
    assert_eq!(
        griff.registermengen(),
        (0, 0, 0),
        "der Destruktor liess einen Registereintrag stehen (offen, abgeloest, abgeloest_abbrueche)"
    );

    // ... und der Listener nimmt weiterhin an.
    let weiterer = verbinden(&pipe);
    assert!(warten(3000, || griff.aktive_worker() == 1));
    drop(weiterer);
    griff.stoppen();
}

// ── NAK-121 H-22 ───────────────────────────────────────────────────────────

/// Jede `unsafe`-Stelle im Broker traegt ihre Begruendung.
///
/// Diese Quellwache ist der eigentliche Nachweis von H-22, nicht die
/// Lint-Tabelle in Cargo.toml: `undocumented_unsafe_blocks` ist ein reiner
/// Clippy-Lint, und tools/beweise.ps1 faehrt in keinem seiner 40 Beine Clippy -
/// er beweist im Kanon also nichts. Der Test dagegen laeuft im vorhandenen
/// Bein A4 mit.
///
/// Fenster: ein mit `// SAFETY:` beginnender Kommentar in den DREI
/// vorangehenden nicht-leeren Zeilen. Am Basisstand von NAK-121 waren es 100
/// Stellen, 59 erfuellt und 41 offen.
#[test]
fn jeder_unsafe_block_traegt_einen_safety_kommentar() {
    let wurzel = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("src");
    let mut offen: Vec<String> = Vec::new();
    let mut gesamt = 0usize;

    fn sammeln(ordner: &std::path::Path, gesamt: &mut usize, offen: &mut Vec<String>) {
        for eintrag in std::fs::read_dir(ordner).expect("Quellordner lesbar") {
            let pfad = eintrag.expect("Verzeichniseintrag").path();
            if pfad.is_dir() {
                // `generiert/` ist Codegen aus dem .fbs; sein Inhalt wird von
                // flatc erzeugt und von pruefe_flatc_drift.py bewacht.
                if pfad.file_name().and_then(|n| n.to_str()) == Some("generiert") {
                    continue;
                }
                sammeln(&pfad, gesamt, offen);
                continue;
            }
            if pfad.extension().and_then(|e| e.to_str()) != Some("rs") {
                continue;
            }
            let quelle = std::fs::read_to_string(&pfad).expect("Quelldatei lesbar");
            let zeilen: Vec<&str> = quelle.lines().collect();
            for (i, zeile) in zeilen.iter().enumerate() {
                let ist_unsafe = zeile.contains("unsafe impl")
                    || zeile
                        .split("unsafe")
                        .skip(1)
                        .any(|rest| rest.trim_start().starts_with('{'));
                if !ist_unsafe {
                    continue;
                }
                *gesamt += 1;
                let mut fenster: Vec<&str> = Vec::new();
                let mut k = i;
                while k > 0 && fenster.len() < 3 {
                    k -= 1;
                    if !zeilen[k].trim().is_empty() {
                        fenster.push(zeilen[k].trim());
                    }
                }
                if !fenster.iter().any(|f| f.starts_with("// SAFETY:")) {
                    offen.push(format!("{}:{}", pfad.display(), i + 1));
                }
            }
        }
    }

    sammeln(&wurzel, &mut gesamt, &mut offen);
    assert!(
        gesamt >= 100,
        "die Wache findet nur {gesamt} unsafe-Stellen - sie misst offenbar nicht mehr, was sie soll"
    );
    assert!(
        offen.is_empty(),
        "{} von {gesamt} unsafe-Stellen ohne SAFETY-Kommentar: {offen:?}",
        offen.len()
    );
}

// ── NAK-121 H-07 und H-08 ──────────────────────────────────────────────────
//
// Ein Peer, dem der Broker etwas geschrieben hat, bekommt es auch: vor dem
// Schliessen laeuft ein beschraenkter Abfluss mit harter Frist, danach wird
// getrennt, danach geschlossen. Verpasst der Abfluss seine Frist, ist der
// Verlust GEZAEHLT statt unsichtbar.
//
// Was ausdruecklich NICHT geaendert wird: die Bedeutung eines erfolgreichen
// Writes und die Reaktion des Coordinators darauf. O-03 erlaubt woertlich, die
// Snapshotschuld nach vollstaendigem Write zu kompaktieren, "ohne
// Empfaengerwirkung zu behaupten"; O-02 sagt, ein Pipe-Write sei keine
// Zustellwahrheit. Eine Wiederzustellpflicht geht also nicht verloren.

/// Beide Zusagen in EINEM Test, aus demselben Grund wie bei H-01/H-02: der
/// Nachbar `zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz` wird
/// ab 23 Tests in dieser Datei flaky, weil er 96 Pipe-Clients seriell oeffnet
/// und danach nur 4000 ms auf 96 aktive Worker wartet.
#[test]
fn abfluss_vor_dem_schliessen_erreicht_den_peer() {
    // D15 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): der Test
    // verband hier nur einen Client und schloss ihn sofort - ohne Hello, ohne
    // Snapshot, ohne irgendeinen Broker-Write. Es gab also gar keinen
    // Ausgabepuffer abzufliessen; gemessen wurden nur der Spurmarker und ein
    // weiterhin nuller Zaehler. Beide Haelften der Zusage stehen jetzt in
    // ihrem eigenen Lauf, jede mit einem ECHTEN ausstehenden Broker-Write.

    // ── H-07 (a): ein UNGELESENER Broker-Write erreicht den Peer ──────────
    //
    // R2-6 der Nacharbeit Runde 2 (Wiederpruefung 1, 03.09.2026): vorher las
    // dieser Lauf sein Welcome VOLLSTAENDIG und schloss erst danach. Beim
    // Flush war damit gar kein Broker-Write mehr ausstehend - die Haelfte
    // blieb auch ohne `abfliessen_mit_frist` gruen. Jetzt liegt das Welcome
    // beim Beginn des Abbaus noch UNGELESEN im Ausgabepuffer, und der Abbau
    // beginnt brokerseitig.
    //
    // Synchronisiert wird ueber die vorhandene Destruktornaht, nicht ueber
    // einen Sleep: `destruktor_fenster` haelt den Destruktor genau zwischen
    // `register_austrag` (win_handles.rs:470) und dem Abfluss an. Erst dort
    // liest der Peer.
    let pipe = probe_pipe("h07");
    let fenster = Arc::new(V3UebergabeBarriere::default());
    let (mut griff, _) = start(
        &pipe,
        V3SecurityTestOptionen {
            destruktor_fenster: Some(fenster.clone()),
            ..V3SecurityTestOptionen::default()
        },
    );
    let mut client = verbinden(&pipe);
    assert!(warten(3000, || griff.aktive_worker() == 1));
    // Ein gueltiges Hello: der Broker ANTWORTET mit einem Welcome - erst damit
    // liegt wirklich etwas im Ausgabepuffer. Der Peer liest es BEWUSST nicht.
    client.write_all(&control_hello()).expect("Hello schreiben");
    client.flush().ok();
    assert!(warten(5000, || griff
        .sicherheits_spur()
        .contains(&"hello_accept")));

    // Der Peer wartet auf den Destruktormarker und liest erst DANACH. Die
    // Reihenfolge im Leser ist der Kern der Messung: erst freigeben, dann
    // lesen. Setzte er sein Read schon vor der Freigabe ab, bediente der
    // Kernel es unmittelbar aus dem Puffer - beim Abfluss waere dann nichts
    // mehr ausstehend und der Lauf messe wieder nur sich selbst (genau der
    // Defekt aus der Wiederpruefung 1).
    let fenster_leser = fenster.clone();
    let leser = std::thread::spawn(move || -> Result<(serde_json::Value, bool), String> {
        if !warten(15_000, || fenster_leser.erreicht()) {
            // Freigeben, sonst haengt der wartende Destruktorthread fuer immer
            // und der ganze Lauf steht, statt rot zu werden.
            fenster_leser.freigeben();
            return Err("der Destruktor erreichte sein Fenster nicht".into());
        }
        // Ab hier laeuft der Destruktor in den Abfluss. Erst jetzt liest der
        // Peer: `FlushFileBuffers` wartet innerhalb seiner Frist auf ihn.
        fenster_leser.freigeben();
        // Die Verzoegerung ist die MESSBEDINGUNG, keine Synchronisation - die
        // laeuft ueber das Destruktorfenster darueber. Der Peer liest bewusst
        // erst, wenn der Abfluss sicher angelaufen ist: 30 ms sind weit ueber
        // dem Aufwecken des wartenden Destruktorthreads und weit unter
        // FLUSH_FRIST (250 ms). Ohne sie kann der Kernel das Read schon aus dem
        // Puffer bedienen, bevor der Abfluss ueberhaupt beginnt - dann misst
        // der Lauf wieder nur sich selbst.
        std::thread::sleep(Duration::from_millis(30));
        let welcome = frame_lesen(&mut client);
        // Danach ist die Verbindung abgebaut: kein weiteres Byte mehr. Ein
        // Fehler ist hier ebenfalls Ende - eine getrennte Named Pipe meldet
        // ERROR_BROKEN_PIPE statt eines sauberen Nullreads. Nur GELESENE Bytes
        // waeren ein Befund.
        let mut rest = [0u8; 64];
        let eof = !matches!(client.read(&mut rest), Ok(n) if n > 0);
        Ok((welcome, eof))
    });

    // Der Abbau kommt brokerseitig.
    griff.stoppen();
    let (welcome, eof) = leser
        .join()
        .expect("Leserthread")
        .expect("der ungelesene Broker-Write erreichte den Peer nicht");
    assert_eq!(
        welcome["type"], "welcome",
        "der beim Abbaubeginn noch ungelesene Broker-Write kam nicht an"
    );
    assert!(eof, "nach dem Welcome kam noch etwas - die Pipe wurde nicht geschlossen");

    // Die Reihenfolge IST die Zusage: abfliessen, trennen, schliessen. Vorher
    // schloss der Destruktor bar, und was noch im Ausgabepuffer lag, verschwand
    // still.
    assert!(griff.sicherheits_spur().contains(&"close"));
    assert_reihenfolge(
        &griff.sicherheits_spur(),
        &["register_austrag", "flush", "close"],
    );

    // Ein Peer, der liest, kostet keine Frist - der Zaehler bleibt null.
    assert_eq!(
        griff.statistik.flush_abgelaufen.load(Ordering::SeqCst),
        0,
        "der Abfluss lief in seine Frist, obwohl der Peer gelesen hat"
    );

    // ── H-07 (b): der NICHT lesende Peer kostet genau die Frist ───────────
    let pipe = probe_pipe("h07b");
    let (mut griff, _) = start(&pipe, V3SecurityTestOptionen::default());
    let mut client = verbinden(&pipe);
    assert!(warten(3000, || griff.aktive_worker() == 1));
    client.write_all(&control_hello()).expect("Hello schreiben");
    client.flush().ok();
    assert!(warten(5000, || griff
        .sicherheits_spur()
        .contains(&"hello_accept")));

    // Der Peer laesst das Welcome im Puffer stehen und liest NIE. Bei Named
    // Pipes wartet `FlushFileBuffers` auf genau diesen Peer - ohne harte Frist
    // haenge der Abbau hier unbegrenzt.
    let vor_dem_stopp = Instant::now();
    griff.stoppen();
    let dauer = vor_dem_stopp.elapsed();
    assert!(
        griff.statistik.flush_abgelaufen.load(Ordering::SeqCst) > 0,
        "der nicht lesende Peer wurde nicht gezaehlt - lief ueberhaupt ein Abfluss?"
    );
    assert!(
        dauer < Duration::from_secs(5),
        "das Schliessen dauerte {dauer:?}; die Frist je Verbindung sind 250 ms"
    );
    drop(client);

    // ── H-08: der Ablehnungsgrund erreicht den Peer ───────────────────────
    let pipe = probe_pipe("h08");
    let (mut griff, senke) = start(&pipe, V3SecurityTestOptionen::default());
    let mut client = verbinden(&pipe);

    // Ein Hello mit falscher Protokollversion: der Broker schreibt einen
    // Ablehnungsrahmen und schliesst danach.
    client
        .write_all(&bootstrap(&control_hello_json("hello", "control", 2)))
        .expect("Bootstrap schreiben");

    // Der Grund erreicht den Peer - vorher lag er im Ausgabepuffer und der
    // Client sah nur eine abgerissene Pipe.
    let reject = bootstrap_reject_lesen(&mut client);
    assert_eq!(reject["type"], "reject");
    assert!(reject["reason"].is_string());

    assert!(warten(3000, || griff.sicherheits_spur().contains(&"close")));
    assert_reihenfolge(&griff.sicherheits_spur(), &["reject", "flush", "close"]);
    assert_keine_fachlogik(&senke);
    griff.stoppen();
}
