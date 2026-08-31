//! Hilfsprozess fuer `store_crash_matrix`. Er wird nie vom Broker gestartet.

use eqcop_broker::store::{
    AppendAusgang, SnapshotZiel, StoreEvent, StoreHandle, StoreKonfiguration, StoreTestHaken,
    StoreWriter,
};
use std::path::PathBuf;
#[cfg(windows)]
use std::sync::Arc;

fn warten() -> ! {
    loop {
        std::thread::park_timeout(std::time::Duration::from_millis(50));
    }
}

fn event(command_id: &str, sequence: i64, mit_outbox: bool) -> StoreEvent {
    let mut event = StoreEvent::session_snapshot(
        "00000000000000000000000000000001",
        "00000000000000000000000000000002",
        "00000000000000000000000000000003",
        sequence,
        serde_json::to_vec(&serde_json::json!({
            "type": "session_snapshot",
            "session_epoch": "00000000000000000000000000000002",
            "broker_epoch": "00000000000000000000000000000003",
            "fuehrendes_main": "00000000000000000000000000000001",
            "beitritt_bestaetigung_noetig": true,
            "mitglieder": []
        }))
        .expect("statischer session_snapshot"),
    );
    event.command_id = Some(command_id.into());
    if mit_outbox {
        event.snapshot_ziele.push(SnapshotZiel {
            project_binding_id: event.project_binding_id.clone(),
            session_epoch: event.session_epoch.clone(),
            instance_id: "00000000000000000000000000000004".into(),
            object_key: "session_snapshot".into(),
        });
    }
    event
}

fn snapshot_subscriber(args: &[std::ffi::OsString]) {
    assert_eq!(
        args.len(),
        4,
        "--snapshot-subscriber Snapshot State Marker Killpunkt"
    );
    let snapshot = std::fs::read(&args[0]).expect("Snapshot lesen");
    eqcop_broker::vertrag::textriegel_bytes(&snapshot).expect("Snapshot-Textriegel");
    let wert: serde_json::Value = serde_json::from_slice(&snapshot).expect("Snapshot-JSON");
    let schema = eqcop_broker::vertrag::Schema::laden(
        serde_json::from_str(include_str!(
            "../../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
        ))
        .expect("v3-Schema JSON"),
    )
    .expect("v3-Schema laden");
    assert!(
        schema.gueltig(&wert),
        "Subscriber erhielt keinen gueltigen v3-Snapshot"
    );
    assert_eq!(wert["type"], "session_snapshot");
    let state = PathBuf::from(&args[1]);
    let marker = PathBuf::from(&args[2]);
    let killpunkt = args[3].to_string_lossy();
    if killpunkt == "vor_snapshot_anwendung" {
        std::fs::write(&marker, b"vor_snapshot_anwendung").expect("Marker");
        warten();
    }
    std::fs::write(&state, &snapshot).expect("absoluten Snapshot setzen");
    if killpunkt == "nach_snapshot_anwendung" {
        std::fs::write(&marker, b"nach_snapshot_anwendung").expect("Marker");
        warten();
    }
}

#[cfg(windows)]
struct PhaseBSenke {
    store: StoreHandle,
    marker: PathBuf,
    killpunkt: String,
}

#[cfg(windows)]
impl eqcop_broker::transport::server_v3::Senke for PhaseBSenke {
    fn control_verbunden(
        &self,
        _link_id: &str,
        _hello: &eqcop_broker::transport::bootstrap::HelloControl,
    ) -> eqcop_broker::transport::server_v3::ControlAnmeldung {
        eqcop_broker::transport::server_v3::ControlAnmeldung::angenommen()
    }

    fn control_schliesst(&self, _link_id: &str) {}
    fn control_getrennt(&self, _link_id: &str) {}
    fn telemetrie_gekoppelt(&self, _link_id: &str) {}
    fn telemetrie_getrennt(&self, _link_id: &str) {}

    fn p0(&self, _link_id: &str, payload: &[u8]) -> Option<Vec<u8>> {
        let wert: serde_json::Value = serde_json::from_slice(payload).ok()?;
        if wert.get("type").and_then(serde_json::Value::as_str) != Some("preview_begin") {
            return None;
        }
        let command_id = wert
            .pointer("/kopf/command_id")
            .and_then(serde_json::Value::as_str)?;
        if self.killpunkt == "vor_persist" {
            std::fs::write(&self.marker, b"vor_persist").ok()?;
            warten();
        }
        let ausgang = self.store.append(vec![event(command_id, 1, true)]).ok()?;
        let ergebnis = match ausgang.first()? {
            AppendAusgang::Angewandt { .. } => "angewandt",
            AppendAusgang::IdempotentWiederholt { .. } => "idempotent_wiederholt",
        };
        serde_json::to_vec(&serde_json::json!({
            "type": "command_ack",
            "command_id": command_id,
            "ergebnis": ergebnis,
            "state_revision": 1,
            "state_hash": "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
        }))
        .ok()
    }

    fn p1(&self, _link_id: &str, _payload: &[u8]) {}
    fn p2(&self, _link_id: &str, _payload: &[u8]) {}
    fn abgewiesen(&self, _grund: &str) {}
}

#[cfg(windows)]
fn phase_b_si(args: &[std::ffi::OsString]) -> ! {
    assert_eq!(args.len(), 4, "--phase-b-si DB Marker Pipe Killpunkt");
    let db = PathBuf::from(&args[0]);
    let marker = PathBuf::from(&args[1]);
    let pipe = args[2].to_string_lossy().into_owned();
    let killpunkt = args[3].to_string_lossy().into_owned();
    let mut konfiguration = StoreKonfiguration::fuer_pfad(db);
    konfiguration.remote_volume_override = Some(false);
    if matches!(killpunkt.as_str(), "vor_store_commit" | "nach_store_commit") {
        konfiguration.test_haken = Some(StoreTestHaken {
            punkt: killpunkt.clone(),
            marker_pfad: marker.clone(),
        });
    }
    let writer = StoreWriter::starten(konfiguration);
    assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());
    let senke = Arc::new(PhaseBSenke {
        store: writer.handle(),
        marker,
        killpunkt,
    });
    let _server =
        eqcop_broker::transport::server_v3::v3_server_starten(&pipe, senke, "phase-b-si".into())
            .expect("Phase-B-Probeserver");
    let _writer = writer;
    warten()
}

fn main() {
    let alle_args = std::env::args_os().skip(1).collect::<Vec<_>>();
    #[cfg(windows)]
    if alle_args.first().is_some_and(|a| a == "--phase-b-si") {
        phase_b_si(&alle_args[1..]);
    }
    if alle_args
        .first()
        .is_some_and(|a| a == "--snapshot-subscriber")
    {
        snapshot_subscriber(&alle_args[1..]);
        return;
    }
    let mut args = alle_args.into_iter();
    let db = PathBuf::from(args.next().expect("DB-Pfad"));
    let marker = PathBuf::from(args.next().expect("Markerpfad"));
    let punkt = args
        .next()
        .expect("Killpunkt")
        .to_string_lossy()
        .into_owned();
    let aktion = args.next().expect("Aktion").to_string_lossy().into_owned();

    let mut konfiguration = StoreKonfiguration::fuer_pfad(db);
    konfiguration.remote_volume_override = Some(false);
    if punkt != "wal_bereit" {
        konfiguration.test_haken = Some(StoreTestHaken {
            punkt: punkt.clone(),
            marker_pfad: marker.clone(),
        });
    }
    let writer = StoreWriter::starten(konfiguration);
    assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());
    let store = writer.handle();

    match aktion.as_str() {
        "migration" => unreachable!("Migration-Haken kehrt vor dem Kill nicht zurueck"),
        "vor_persist" => {
            std::fs::write(&marker, b"vor_persist").expect("Marker");
            warten();
        }
        "append" => {
            let _ = store.append(vec![event("00000000000000000000000000000005", 1, true)]);
        }
        "compact" => {
            let ausgang = store
                .append(vec![event("00000000000000000000000000000005", 1, true)])
                .expect("Vorbereitungsevent");
            let ziel = SnapshotZiel {
                project_binding_id: "00000000000000000000000000000001".into(),
                session_epoch: "00000000000000000000000000000002".into(),
                instance_id: "00000000000000000000000000000004".into(),
                object_key: "session_snapshot".into(),
            };
            let _ = store.snapshot_schuld_kompaktieren(ziel, ausgang[0].event_ord());
        }
        "wal" => {
            store
                .append(vec![event("00000000000000000000000000000005", 1, true)])
                .expect("WAL-Commit");
            std::fs::write(&marker, b"wal_bereit").expect("Marker");
            warten();
        }
        _ => panic!("unbekannte Aktion {aktion}"),
    }
    warten();
}
