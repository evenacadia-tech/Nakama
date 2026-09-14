//! NAK-286 Etappe 3, M-46: der Diagnose-Briefkasten Ende zu Ende ueber einen Probe-Pipenamen
//! unter PROBE_PRAEFIX mit echtem v2-Hello (docs/beweise/NAK-286.md §4.4, §5.6 Punkt 4). Die
//! Produktions-Pipe oeffnet hier niemand, und `probe_lauf` startet keinen Briefkasten.
#![cfg(windows)]

use eqcop_broker::framing::{frame_lesen, frame_schreiben};
use eqcop_broker::transport::pipetoken::{ist_probe_pipename, PROBE_PRAEFIX};
use eqcop_broker::{briefkasten_probe_lauf_starten, briefkasten_starts, probe_lauf, PIPE_NAME};
use std::fs::File;
use std::path::{Path, PathBuf};
use std::sync::Mutex;
use std::time::{Duration, Instant};

/// Beide Tests lesen den prozessweiten Startzaehler; sie laufen nacheinander.
static NACHEINANDER: Mutex<()> = Mutex::new(());

fn probe_pipename(zweck: &str) -> String {
    let name = format!("{PROBE_PRAEFIX}nak286-{zweck}.{}.{}", std::process::id(), uuid::Uuid::new_v4().simple());
    assert!(
        ist_probe_pipename(&name) && !name.eq_ignore_ascii_case(PIPE_NAME),
        "M-46: {name} ist kein Probe-Pipename"
    );
    name
}

/// Fragt den Zustand bis zur Frist ab; die Frist ist nur ein Deckel gegen Haengen.
fn warte_bis(frist: Duration, mut bedingung: impl FnMut() -> bool) -> bool {
    let beginn = Instant::now();
    while !bedingung() {
        if beginn.elapsed() >= frist {
            return false;
        }
        std::thread::sleep(Duration::from_millis(20));
    }
    true
}

struct TempWurzel(PathBuf);

impl Drop for TempWurzel {
    fn drop(&mut self) {
        let _ = std::fs::remove_dir_all(&self.0);
    }
}

fn antwort_im_ordner(ordner: &Path, praefix: &str) -> Option<PathBuf> {
    let eintraege = std::fs::read_dir(ordner).into_iter().flatten().filter_map(Result::ok);
    eintraege.map(|eintrag| eintrag.path()).find(|pfad| {
        let name = pfad.file_name().and_then(|teil| teil.to_str()).unwrap_or_default();
        name.starts_with(praefix) && name.ends_with(".json")
    })
}

#[test]
fn probe_lauf_startet_keinen_briefkasten() {
    let _nacheinander = NACHEINANDER.lock().unwrap_or_else(|e| e.into_inner());
    let pipe = probe_pipename("probe-lauf");
    let vorher = briefkasten_starts();
    let status = probe_lauf(&pipe, 1).expect("probe_lauf auf dem Probe-Pipenamen");
    let gestartet = briefkasten_starts() - vorher;
    assert!(status.contains("\"laeuft\": true"), "M-46 probe_lauf_startet_keinen_briefkasten vorbedingung: {status}");
    assert_eq!(gestartet, 0, "M-46 probe_lauf_startet_keinen_briefkasten: Startzaehler {gestartet} statt 0");
}

#[test]
fn briefkasten_ende_zu_ende_probe_pipe() {
    let _nacheinander = NACHEINANDER.lock().unwrap_or_else(|e| e.into_inner());
    // Die Buehne ist der Anfragende: sie legt diagnose\ und antwort\ an (T-4).
    let name = format!("nak286-e2e-{}-{}", std::process::id(), uuid::Uuid::new_v4().simple());
    let wurzel = TempWurzel(std::env::temp_dir().join(name));
    let diagnose = wurzel.0.join("evenacadia").join("nakama").join("diagnose");
    let antwort = diagnose.join("antwort");
    std::fs::create_dir_all(&antwort).expect("Briefkasten der Buehne anlegen");
    let pipe = probe_pipename("briefkasten");
    let vorher = briefkasten_starts();
    let lauf = briefkasten_probe_lauf_starten(&pipe, &wurzel.0).expect("Probelauf starten");
    let laeuft = lauf.briefkasten_gestartet() && briefkasten_starts() == vorher + 1;
    assert!(laeuft, "M-46 briefkasten_ende_zu_ende_probe_pipe vorbedingung: Briefkasten laeuft");

    // Ein echtes v2-Hello ueber die Probe-Pipe.
    let mut client: Option<File> = None;
    let verbunden = warte_bis(Duration::from_secs(5), || {
        client = std::fs::OpenOptions::new().read(true).write(true).open(&pipe).ok();
        client.is_some()
    });
    assert!(verbunden, "M-46 briefkasten_ende_zu_ende_probe_pipe: Probe-Pipe nicht erreichbar");
    let mut client = client.expect("Client verbunden");
    let sensor_id = format!("s-nak286-e2e-{}", std::process::id());
    let hello = format!(
        r#"{{"type":"hello","protocol_version":2,"plugin_version":"0.2.0","host_pid":{},"sensor":{{"sensor_id":"{sensor_id}","role":"sensor","label":"NAK-286 E2E","pair_id":null}},"audio":{{"samplerate":48000.0,"block_size":512,"channels":2}}}}"#,
        std::process::id()
    );
    frame_schreiben(&mut client, &hello).expect("Hello schreiben");
    let welcome = frame_lesen(&mut client).expect("Welcome lesen");
    assert!(welcome.contains(r#""type":"welcome""#), "M-46 briefkasten_ende_zu_ende_probe_pipe: kein Welcome: {welcome}");

    // Die Anfrage kommt erst, wenn das Register den Sensor fuehrt.
    let im_register = warte_bis(Duration::from_secs(5), || lauf.sensor_ids().contains(&sensor_id));
    assert!(im_register, "M-46 briefkasten_ende_zu_ende_probe_pipe: Sensor nicht im Register");
    let kennung = uuid::Uuid::new_v4().simple().to_string();
    let anfrage = format!(r#"{{"format":"nakama.diagnose.anfrage.v1","anfrage_id":"{kennung}"}}"#);
    std::fs::write(diagnose.join("anfrage.json"), anfrage).expect("Anfrage schreiben");
    let praefix = format!("{kennung}.broker.{}.", std::process::id());
    let mut datei = None;
    let beantwortet = warte_bis(Duration::from_secs(10), || {
        datei = antwort_im_ordner(&antwort, &praefix);
        datei.is_some()
    });
    assert!(beantwortet, "M-46 briefkasten_ende_zu_ende_probe_pipe: keine Antwort der Rolle broker");

    let text = std::fs::read(datei.expect("Antwortdatei")).expect("Antwort lesen");
    let umschlag: serde_json::Value = serde_json::from_slice(&text).expect("Antwort ist JSON");
    let kopf = (umschlag["rolle"].as_str(), umschlag["anfrage_id"].as_str());
    assert_eq!(kopf, (Some("broker"), Some(kennung.as_str())), "M-46 briefkasten_ende_zu_ende_probe_pipe: Kopf");
    let laufzeit = umschlag["laufzeit_id"].as_str().map(str::to_string);
    assert_eq!(laufzeit, lauf.laufzeit_id(), "M-46 briefkasten_ende_zu_ende_probe_pipe: laufzeit_id");
    let sensoren = umschlag["aggregat"]["sensoren"].as_array().into_iter().flatten();
    let ids: Vec<&str> = sensoren.filter_map(|sensor| sensor["sensor_id"].as_str()).collect();
    assert!(
        ids.contains(&sensor_id.as_str()),
        "M-46 briefkasten_ende_zu_ende_probe_pipe: die Antwort nennt den per Probe-Pipe angemeldeten Sensor nicht: {ids:?}"
    );
    drop(client);
    drop(lauf);
}
