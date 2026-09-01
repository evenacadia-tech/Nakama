#![cfg(windows)]

use eqcop_broker::coordinator::{
    Coordinator, Lautheitszustand, ManualClock, SessionPush, STALE_NACH_MS, TOMBSTONE_MS,
};
use eqcop_broker::store::StoreWriter;
use eqcop_broker::generiert::nakama_telemetry_v1_generated::nakama::v_3 as fb;
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
use eqcop_broker::transport::server_v3::Senke;
use flatbuffers::FlatBufferBuilder;
use serde_json::{json, Value};
use std::sync::{Arc, Mutex};

fn hex(n: usize) -> String {
    format!("{n:032x}")
}

fn adresse(projekt: usize, sitzung: usize, instanz: usize, nonce: usize) -> Adresse {
    Adresse {
        logon_sid: "S-1-5-21-1-2-3-1001".into(),
        project_binding_id: hex(projekt),
        session_epoch: hex(sitzung),
        instance_id: hex(instanz),
        runtime_nonce: hex(nonce),
    }
}

fn hello(adresse: &Adresse, plugin_kind: &str, host_pid: Option<u32>) -> HelloControl {
    HelloControl {
        typ: "hello".into(),
        connection_kind: "control".into(),
        protocol: 3,
        plugin_version: "sonde012-test".into(),
        plugin_kind: plugin_kind.into(),
        adresse: adresse.clone(),
        host: host_pid.map(|pid| HostAngabe {
            pid,
            name: Some("FL Studio".into()),
            version: None,
        }),
        audio: AudioLage {
            samplerate: 48_000.0,
            block_size: 512,
            channels: 2,
        },
    }
}

fn capabilities() -> Value {
    json!({
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
    })
}

fn runtime(
    messpunkt: &str,
    betrieb: &str,
    host_bus_name: Option<&str>,
    host_mixer_index: Option<u64>,
) -> Value {
    let mut wert = json!({"messpunkt": messpunkt, "betrieb": betrieb});
    if let Some(name) = host_bus_name {
        wert["host_bus_name"] = Value::String(name.into());
    }
    if let Some(index) = host_mixer_index {
        wert["host_mixer_index"] = Value::from(index);
    }
    wert
}

fn heartbeat_mit_runtime(
    c: &Coordinator,
    link: &str,
    adresse: &Adresse,
    sequence: u64,
    runtime: Option<Value>,
) -> bool {
    let mut wert = json!({
        "type": "heartbeat",
        "adresse": adresse,
        "sequence": sequence,
        "state_revision": 0,
        "capabilities": capabilities(),
        "zaehler": {
            "frames_dropped": 0,
            "parse_errors": 0,
            "queue_overflows": 0
        }
    });
    if let Some(runtime) = runtime {
        wert["runtime"] = runtime;
    }
    Senke::p0(c, link, &serde_json::to_vec(&wert).unwrap()).is_some()
}

fn heartbeat(c: &Coordinator, link: &str, adresse: &Adresse, sequence: u64) -> bool {
    heartbeat_mit_runtime(
        c,
        link,
        adresse,
        sequence,
        Some(runtime("insert", "active", None, None)),
    )
}

fn mitglied<'a>(snapshot: &'a Value, instance_id: &str) -> &'a Value {
    snapshot["mitglieder"]
        .as_array()
        .unwrap()
        .iter()
        .find(|mitglied| mitglied["adresse"]["instance_id"] == instance_id)
        .expect("Mitglied steht im Snapshot")
}

fn anmelden(c: &Coordinator, link: &str, adresse: &Adresse, art: &str, pid: Option<u32>) {
    let ausgang = c.control_hello_registrieren(link, &hello(adresse, art, pid));
    assert!(ausgang.angenommen, "{:?}", ausgang.grund);
}

fn quelle_anmelden(c: &Coordinator, link: &str, adresse: &Adresse) {
    anmelden(c, link, adresse, "active_probe", Some(9001));
    assert!(heartbeat(c, link, adresse, 1));
    Senke::telemetrie_gekoppelt(c, link);
}

fn abonnieren(c: &Coordinator, link: &str, adresse: &Adresse) -> bool {
    c.subscribe_json(
        link,
        &serde_json::to_vec(&json!({
            "type": "subscribe_session",
            "adresse": adresse,
            "session_epoch": adresse.session_epoch
        }))
        .unwrap(),
    )
}

fn session_command(
    c: &Coordinator,
    link: &str,
    command: &str,
    command_id: usize,
    ziel: &Adresse,
    session_epoch: &str,
) -> Value {
    let payload = serde_json::to_vec(&json!({
        "type": "session_command",
        "command": command,
        "command_id": hex(command_id),
        "ziel": ziel,
        "session_epoch": session_epoch
    }))
    .unwrap();
    let ack = Senke::p0(c, link, &payload).expect("session_command antwortet mit command_ack");
    serde_json::from_slice(&ack).expect("command_ack ist JSON")
}

fn feature_batch(
    adresse: &Adresse,
    sequence: u64,
    sample_count: u32,
    lufs_i: Option<f32>,
    unsicherheit: Option<f32>,
    status: Option<u8>,
) -> Vec<u8> {
    let mut fbb = FlatBufferBuilder::new();
    let sid = fbb.create_string(&adresse.logon_sid);
    let projekt = fbb.create_string(&adresse.project_binding_id);
    let sitzung = fbb.create_string(&adresse.session_epoch);
    let instanz = fbb.create_string(&adresse.instance_id);
    let nonce = fbb.create_string(&adresse.runtime_nonce);
    let quelle = fb::Adresse::create(
        &mut fbb,
        &fb::AdresseArgs {
            logon_sid: Some(sid),
            project_binding_id: Some(projekt),
            session_epoch: Some(sitzung),
            instance_id: Some(instanz),
            runtime_nonce: Some(nonce),
        },
    );
    let werte = fbb.create_vector(&[0i16; 64]);
    let bitmap = fbb.create_vector(&[0xffu8; 8]);
    let baender = fb::Bandwerte::create(
        &mut fbb,
        &fb::BandwerteArgs {
            gitter: fb::Bandgitter::nakama_log64_v1,
            encoding: fb::BandEncoding::q_db_0p1_i16,
            werte_i16: Some(werte),
            gueltig_bitmap: Some(bitmap),
            ..Default::default()
        },
    );
    let transport = fb::Transportstempel::create(
        &mut fbb,
        &fb::TransportstempelArgs {
            transport_epoch: 1,
            continuity_segment: 1,
            sequence,
            zeitbasis: fb::Zeitbasis::local_monotonic,
            sample_count,
            sample_rate: 48_000.0,
            process_context_present: Some(false),
            ..Default::default()
        },
    );
    let frame = fb::Frame::create(
        &mut fbb,
        &fb::FrameArgs {
            transport: Some(transport),
            baender: Some(baender),
            metrics_version: 1,
            aktivitaet: Some(sequence as f32),
            lufs_s: Some(-18.0),
            lufs_i,
            lufs_i_unsicherheit_lu: unsicherheit,
            lufs_i_status: status,
            ..Default::default()
        },
    );
    let eintrag = fb::QuellenEintrag::create(
        &mut fbb,
        &fb::QuellenEintragArgs {
            quelle: Some(quelle),
            frame: Some(frame),
        },
    );
    let eintraege = fbb.create_vector(&[eintrag]);
    let batch = fb::FeatureBatch::create(
        &mut fbb,
        &fb::FeatureBatchArgs {
            eintraege: Some(eintraege),
        },
    );
    fb::finish_feature_batch_buffer(&mut fbb, batch);
    fbb.finished_data().to_vec()
}

#[derive(Default)]
struct PushProbe {
    snapshots: Mutex<Vec<(String, Value)>>,
    frames: Mutex<Vec<(String, String, Vec<u8>)>>,
}

impl PushProbe {
    fn snapshots(&self) -> Vec<(String, Value)> {
        self.snapshots.lock().unwrap().clone()
    }

    fn frames(&self) -> Vec<(String, String, Vec<u8>)> {
        self.frames.lock().unwrap().clone()
    }
}

impl SessionPush for PushProbe {
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
        let wert = serde_json::from_slice(payload).expect("Snapshot ist JSON");
        self.snapshots.lock().unwrap().push((link_id.into(), wert));
        true
    }

    fn messframe_schreiben(&self, link_id: &str, instance_id: &str, payload: &[u8]) -> bool {
        self.frames
            .lock()
            .unwrap()
            .push((link_id.into(), instance_id.into(), payload.to_vec()));
        true
    }
}

fn coordinator() -> (Coordinator, Arc<ManualClock>, Arc<PushProbe>) {
    let clock = Arc::new(ManualClock::default());
    let c = Coordinator::mit_uhr(clock.clone(), hex(0xbeef));
    let push = Arc::new(PushProbe::default());
    c.session_push_setzen(push.clone());
    (c, clock, push)
}

#[test]
fn main_subscribe_pushes_absolute_snapshot_and_reconnect_resubscribes() {
    let (c, _clock, push) = coordinator();
    let main_a = adresse(1, 2, 10, 100);
    anmelden(&c, "main-a", &main_a, "main", Some(77));
    assert!(heartbeat(&c, "main-a", &main_a, 1));
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 1, 2048, None, None, Some(1)),
    );
    assert!(abonnieren(&c, "main-a", &main_a));
    let snapshots = push.snapshots();
    assert_eq!(snapshots.len(), 1);
    assert_eq!(snapshots[0].0, "main-a");
    assert_eq!(snapshots[0].1["type"], "session_snapshot");
    assert!(snapshots[0].1.get("event_uuid").is_none());
    assert_eq!(push.frames().len(), 1);
    assert_eq!(push.frames()[0].0, "main-a");

    c.control_ende("main-a");
    assert_eq!(c.subscription_anzahl(), 0);
    let mut main_b = main_a.clone();
    main_b.runtime_nonce = hex(101);
    anmelden(&c, "main-b", &main_b, "main", Some(77));
    assert_eq!(
        push.snapshots().len(),
        1,
        "Reconnect allein abonniert nicht"
    );
    assert!(heartbeat(&c, "main-b", &main_b, 1));
    assert!(abonnieren(&c, "main-b", &main_b));
    let snapshots = push.snapshots();
    assert_eq!(snapshots.len(), 2);
    assert_eq!(snapshots[1].0, "main-b");
    assert_eq!(push.frames().len(), 2);
    assert_eq!(push.frames()[1].0, "main-b");
    assert_eq!(c.subscription_anzahl(), 1);
}

#[test]
fn first_valid_p2_changes_only_measurement_axis() {
    let (c, _clock, _push) = coordinator();
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);
    let control_vorher = c.modell_sicht(&hex(1), &hex(2));
    assert_eq!(
        c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap().sequence,
        None
    );

    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 7, 2048, None, None, Some(1)),
    );

    assert_eq!(c.modell_sicht(&hex(1), &hex(2)), control_vorher);
    let messung = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert_eq!(messung.sequence, Some(7));
    assert_eq!(messung.sample_count, Some(2048));
    assert_eq!(messung.lautheitszustand, Lautheitszustand::Collecting);
    assert!(!messung.messung_ungueltig);
}

#[test]
fn join_is_connected_but_measurement_missing() {
    let (c, _clock, push) = coordinator();
    let main = adresse(1, 2, 10, 100);
    anmelden(&c, "main", &main, "main", Some(77));
    assert!(heartbeat(&c, "main", &main, 1));
    assert!(abonnieren(&c, "main", &main));
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);

    let control = c
        .modell_sicht(&hex(1), &hex(2))
        .clients
        .into_iter()
        .find(|client| client.adresse.instance_id == source.instance_id)
        .unwrap();
    assert!(control.verbunden);
    assert!(!control.stale);
    let messung = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert_eq!(messung.sequence, None);
    assert_eq!(messung.lautheitszustand, Lautheitszustand::Missing);
    assert_eq!(
        push.snapshots().last().unwrap().1["type"],
        "session_snapshot"
    );
}

#[test]
fn runtime_block_sets_and_updates_descriptor() {
    let (c, _clock, push) = coordinator();
    let main = adresse(1, 2, 10, 100);
    anmelden(&c, "main", &main, "main", Some(77));
    assert!(heartbeat(&c, "main", &main, 1));
    assert!(abonnieren(&c, "main", &main));
    let source = adresse(1, 2, 20, 200);
    anmelden(&c, "probe", &source, "active_probe", Some(9001));

    assert!(heartbeat_mit_runtime(
        &c,
        "probe",
        &source,
        1,
        Some(runtime("pre", "active", Some("Keys"), Some(7))),
    ));
    let snapshot: Value =
        serde_json::from_slice(&c.session_snapshot_json(&hex(1), &hex(2))).unwrap();
    let descriptor = &mitglied(&snapshot, &source.instance_id)["probe_descriptor"];
    assert_eq!(descriptor["measurement_position"], "pre");
    assert_eq!(descriptor["betrieb"], "active");
    assert_eq!(descriptor["host_bus_name"], "Keys");
    assert_eq!(descriptor["host_mixer_index"], 7);

    let vor_update = push.snapshots().len();
    assert!(heartbeat_mit_runtime(
        &c,
        "probe",
        &source,
        2,
        Some(runtime("post", "offline", Some("Keys Print"), None)),
    ));
    assert_eq!(push.snapshots().len(), vor_update + 1);
    let snapshot = push.snapshots().last().unwrap().1.clone();
    let descriptor = &mitglied(&snapshot, &source.instance_id)["probe_descriptor"];
    assert_eq!(descriptor["measurement_position"], "post");
    assert_eq!(descriptor["betrieb"], "offline");
    assert_eq!(descriptor["host_bus_name"], "Keys Print");
    assert!(descriptor.get("host_mixer_index").is_none());
}

#[test]
fn missing_messpunkt_yields_member_without_descriptor() {
    let (c, _clock, _push) = coordinator();
    let source = adresse(1, 2, 20, 200);
    anmelden(&c, "probe", &source, "active_probe", Some(9001));
    assert!(heartbeat_mit_runtime(&c, "probe", &source, 1, None));

    let snapshot: Value =
        serde_json::from_slice(&c.session_snapshot_json(&hex(1), &hex(2))).unwrap();
    let source = mitglied(&snapshot, &source.instance_id);
    assert_eq!(source["plugin_kind"], "active_probe");
    assert_eq!(source["frische"]["stale"], false);
    assert!(source.get("probe_descriptor").is_none());
}

#[test]
fn betrieb_suspended_and_offline_reach_snapshot() {
    let (c, _clock, _push) = coordinator();
    let source = adresse(1, 2, 20, 200);
    anmelden(&c, "probe", &source, "active_probe", Some(9001));

    for (sequence, betrieb) in [(1, "suspended"), (2, "offline")] {
        assert!(heartbeat_mit_runtime(
            &c,
            "probe",
            &source,
            sequence,
            Some(runtime("insert", betrieb, None, None)),
        ));
        let snapshot: Value =
            serde_json::from_slice(&c.session_snapshot_json(&hex(1), &hex(2))).unwrap();
        assert_eq!(
            mitglied(&snapshot, &source.instance_id)["probe_descriptor"]["betrieb"],
            betrieb
        );
    }
}

#[test]
fn p2_reject_reason_reaches_subscriber_and_keeps_last_valid_frame() {
    let (c, _clock, push) = coordinator();
    let main = adresse(1, 2, 10, 100);
    anmelden(&c, "main", &main, "main", Some(77));
    assert!(heartbeat(&c, "main", &main, 1));
    assert!(abonnieren(&c, "main", &main));

    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 7, 2048, None, None, Some(1)),
    );
    let vor_ablehnung = push.snapshots().len();
    Senke::p2(&c, "probe", b"kein FlatBuffer");
    assert_eq!(
        push.snapshots().len(),
        vor_ablehnung,
        "P2 blockiert nicht auf dem P1-/Store-Abfluss"
    );
    c.liveness_tick();

    let snapshots = push.snapshots();
    assert_eq!(snapshots.len(), vor_ablehnung + 1);
    let source_wire = mitglied(&snapshots.last().unwrap().1, &source.instance_id);
    assert_eq!(
        source_wire["p2_reject"],
        json!({"grund": "feature_batch_ungueltig", "zaehler": 1})
    );
    let messung = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert_eq!(messung.sequence, Some(7));
    assert!(messung.messung_ungueltig);
}

#[test]
fn latest_per_source_no_cross_gap_interpolation() {
    let (c, clock, _push) = coordinator();
    let a = adresse(1, 2, 20, 200);
    let b = adresse(1, 2, 21, 201);
    quelle_anmelden(&c, "a", &a);
    quelle_anmelden(&c, "b", &b);
    Senke::p2(&c, "a", &feature_batch(&a, 1, 2048, None, None, Some(1)));
    Senke::p2(&c, "b", &feature_batch(&b, 10, 16_384, None, None, Some(2)));
    clock.setze_ms(125);
    Senke::p2(&c, "a", &feature_batch(&a, 3, 4096, None, None, Some(1)));

    let sicht_a = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    let sicht_b = c.messsicht(&hex(1), &hex(2), &hex(21)).unwrap();
    assert_eq!(sicht_a.sequence, Some(3));
    assert_eq!(sicht_b.sequence, Some(10));
    assert_eq!(sicht_a.fenster_ms, Some(4096.0 / 48_000.0 * 1000.0));
    assert_eq!(sicht_a.alter_ms, Some(0));
    assert_eq!(sicht_b.alter_ms, Some(125));
    assert_eq!(c.p2_live_frames(), 3);
}

#[test]
fn p2_does_not_revive_control_and_heartbeat_does_not_refresh_measurement() {
    let (c, clock, _push) = coordinator();
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 1, 2048, None, None, Some(1)),
    );
    clock.setze_ms(STALE_NACH_MS + 1);
    c.liveness_tick();
    assert!(c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);

    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 2, 2048, None, None, Some(1)),
    );
    assert!(c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    assert!(heartbeat(&c, "probe", &source, 2));
    assert!(!c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    let messung = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert_eq!(messung.sequence, Some(2));
    assert_eq!(messung.alter_ms, Some(0));
}

#[test]
fn probe_control_end_disconnects_before_next_push() {
    let (c, _clock, push) = coordinator();
    let main = adresse(1, 2, 10, 100);
    anmelden(&c, "main", &main, "main", Some(77));
    assert!(heartbeat(&c, "main", &main, 1));
    assert!(abonnieren(&c, "main", &main));
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 1, 2048, None, None, Some(1)),
    );
    let pushes = push.frames().len();
    let angenommen = c.p2_live_frames();

    c.control_ende("probe");
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 2, 2048, None, None, Some(1)),
    );
    let sicht = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert!(!sicht.control_verbunden);
    assert!(sicht.control_stale);
    assert_eq!(sicht.sequence, Some(1));
    assert_eq!(c.p2_live_frames(), angenommen);
    assert_eq!(push.frames().len(), pushes);
}

#[test]
fn control_end_is_atomic_and_no_push_uses_old_link_after_unlock() {
    let (c, _clock, push) = coordinator();
    let main = adresse(1, 2, 10, 100);
    anmelden(&c, "main", &main, "main", Some(77));
    assert!(heartbeat(&c, "main", &main, 1));
    assert!(abonnieren(&c, "main", &main));
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);
    let frame_pushes = push.frames().len();

    c.control_ende("main");
    assert_eq!(c.subscription_anzahl(), 0);
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 1, 2048, None, None, Some(1)),
    );
    assert_eq!(push.frames().len(), frame_pushes);
    assert!(c.session_push_ziele(&main.session_epoch, &main).is_empty());
}

#[test]
fn main_link_end_cleans_subscription_before_push() {
    for grund in [
        "EOF",
        "Protokollfehler",
        "Timeout",
        "Writefehler",
        "Serverstopp",
    ] {
        let (c, clock, push) = coordinator();
        let main = adresse(1, 2, 10, 100);
        let source = adresse(1, 2, 20, 200);
        anmelden(&c, "main", &main, "main", Some(77));
        assert!(heartbeat(&c, "main", &main, 1));
        quelle_anmelden(&c, "probe", &source);
        assert!(abonnieren(&c, "main", &main));
        // Vier Transportenden muenden in denselben synchronen Hook;
        // server_v3::subscription_cleanup_vor_weiterem_push faehrt sie ueber
        // Probe-Pipes. Der heutige Server besitzt keinen erfundenen
        // Post-Welcome-I/O-Timeout; dessen reale Zeitgrenze ist stale plus
        // Tombstone-Eviction und wird hier ueber die monotone Uhr gefahren.
        if grund == "Timeout" {
            clock.setze_ms(STALE_NACH_MS + 1);
            c.liveness_tick();
            clock.setze_ms(STALE_NACH_MS + 1 + TOMBSTONE_MS);
            c.liveness_tick();
        } else {
            c.control_ende("main");
        }
        assert_eq!(c.subscription_anzahl(), 0, "{grund}");
        let nach_cleanup = push.snapshots().len();
        if grund == "Timeout" {
            let rueckkehr = adresse(1, 2, 21, 201);
            quelle_anmelden(&c, "probe-neu", &rueckkehr);
        } else {
            assert!(heartbeat(&c, "probe", &source, 2), "{grund}");
        }
        assert_eq!(
            push.snapshots().len(),
            nach_cleanup,
            "{grund}: spaeter Push"
        );
    }
}

#[test]
fn restart_has_no_old_live_or_measurement_truth() {
    let (alt, _clock, _push) = coordinator();
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&alt, "probe", &source);
    Senke::p2(
        &alt,
        "probe",
        &feature_batch(&source, 9, 2048, Some(-19.0), Some(0.4), None),
    );
    assert_eq!(alt.p2_live_frames(), 1);

    let (neu, _clock, push) = coordinator();
    assert_eq!(neu.p2_live_frames(), 0);
    assert!(neu.messsicht(&hex(1), &hex(2), &hex(20)).is_none());
    let main = adresse(1, 2, 10, 101);
    anmelden(&neu, "main-neu", &main, "main", Some(77));
    assert!(heartbeat(&neu, "main-neu", &main, 1));
    assert!(abonnieren(&neu, "main-neu", &main));
    let letzter = push.snapshots().last().unwrap().1.clone();
    assert_eq!(letzter["broker_epoch"], hex(0xbeef));
    assert_eq!(letzter["mitglieder"].as_array().unwrap().len(), 1);
    assert!(push.frames().is_empty());
}

#[test]
fn invalid_frame_keeps_last_valid_but_marks_measurement_invalid() {
    let (c, _clock, _push) = coordinator();
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 7, 2048, None, None, Some(1)),
    );
    Senke::p2(&c, "probe", b"kein FlatBuffer");
    let invalid = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert_eq!(invalid.sequence, Some(7));
    assert!(invalid.messung_ungueltig);
    assert_eq!(invalid.verworfene_frames, 1);
    assert_eq!(
        invalid.letzter_fehler.as_deref(),
        Some("feature_batch_ungueltig")
    );

    let fremd = adresse(1, 2, 21, 201);
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&fremd, 99, 4096, None, None, Some(1)),
    );
    let falsch_adressiert = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert_eq!(falsch_adressiert.sequence, Some(7));
    assert!(falsch_adressiert.messung_ungueltig);
    assert_eq!(falsch_adressiert.verworfene_frames, 2);
    assert_eq!(
        falsch_adressiert.letzter_fehler.as_deref(),
        Some("quelladresse_abweichend")
    );

    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 8, 4096, None, None, Some(1)),
    );
    let geheilt = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert_eq!(geheilt.sequence, Some(8));
    assert!(!geheilt.messung_ungueltig);
    assert_eq!(geheilt.verworfene_frames, 2);
}

#[test]
fn loudness_pair_reaches_only_its_source() {
    let (c, _clock, push) = coordinator();
    let main = adresse(1, 2, 10, 100);
    anmelden(&c, "main", &main, "main", Some(77));
    assert!(heartbeat(&c, "main", &main, 1));
    assert!(abonnieren(&c, "main", &main));
    let a = adresse(1, 2, 20, 200);
    let b = adresse(1, 2, 21, 201);
    quelle_anmelden(&c, "a", &a);
    quelle_anmelden(&c, "b", &b);

    Senke::p2(
        &c,
        "a",
        &feature_batch(&a, 1, 2048, Some(-19.5), Some(0.35), None),
    );
    Senke::p2(&c, "b", &feature_batch(&b, 1, 2048, None, None, Some(1)));
    let sicht_a = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    let sicht_b = c.messsicht(&hex(1), &hex(2), &hex(21)).unwrap();
    assert_eq!(sicht_a.lautheitszustand, Lautheitszustand::Paar);
    assert_eq!(sicht_a.lufs_i, Some(-19.5));
    assert_eq!(sicht_a.lufs_i_unsicherheit_lu, Some(0.35));
    assert_eq!(sicht_b.lautheitszustand, Lautheitszustand::Collecting);
    assert_eq!(sicht_b.lufs_i, None);
    assert_eq!(
        push.frames()
            .iter()
            .map(|(_, id, _)| id.clone())
            .collect::<Vec<_>>(),
        vec![a.instance_id.clone(), b.instance_id.clone()]
    );

    Senke::p2(
        &c,
        "a",
        &feature_batch(&a, 2, 4096, Some(-18.0), None, None),
    );
    let invalid_a = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert_eq!(
        invalid_a.sequence,
        Some(2),
        "der restliche Frame bleibt nutzbar"
    );
    assert_eq!(invalid_a.lautheitszustand, Lautheitszustand::Invalid);
    assert_eq!(invalid_a.lufs_i, None);
    assert_eq!(invalid_a.letztes_gueltiges_lufs_i, Some(-19.5));
    assert_eq!(invalid_a.ungueltige_lautheitspaare, 1);
    assert!(!invalid_a.messung_ungueltig);
    assert_eq!(
        c.messsicht(&hex(1), &hex(2), &hex(21))
            .unwrap()
            .lautheitszustand,
        Lautheitszustand::Collecting
    );
    let pushes = push.frames();
    let letzter_payload = &pushes.last().unwrap().2;
    assert!(eqcop_broker::telemetrie::pruefe(letzter_payload).is_empty());
}

#[test]
fn nonfinite_loudness_counts_per_source_but_keeps_rest_of_frame() {
    let (c, _clock, _push) = coordinator();
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 4, 16_384, Some(f32::NAN), Some(0.4), None),
    );
    let sicht = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert_eq!(sicht.sequence, Some(4));
    assert_eq!(sicht.sample_count, Some(16_384));
    assert_eq!(sicht.lautheitszustand, Lautheitszustand::Invalid);
    assert_eq!(sicht.lufs_i, None);
    assert_eq!(sicht.ungueltige_lautheitspaare, 1);
    assert!(!sicht.messung_ungueltig);
}

#[test]
fn every_loudness_contract_violation_is_removed_without_losing_the_rest() {
    for (name, payload) in [
        (
            "half-ohne-wert",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-half-ohne-wert.bin"
            ) as &[u8],
        ),
        (
            "half-ohne-unsicherheit",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-half-ohne-unsicherheit.bin"
            ),
        ),
        (
            "wert-nan",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-wert-nan.bin"
            ),
        ),
        (
            "wert-plus-inf",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-wert-plus-inf.bin"
            ),
        ),
        (
            "wert-minus-inf",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-wert-minus-inf.bin"
            ),
        ),
        (
            "unsicherheit-nan",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-unsicherheit-nan.bin"
            ),
        ),
        (
            "unsicherheit-plus-inf",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-unsicherheit-plus-inf.bin"
            ),
        ),
        (
            "unsicherheit-minus-inf",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-unsicherheit-minus-inf.bin"
            ),
        ),
        (
            "status-0",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-status-0.bin"
            ),
        ),
        (
            "status-3",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-status-3.bin"
            ),
        ),
        (
            "paar-plus-status",
            include_bytes!(
                "../../eq-copilot/fixtures/v3/flatbuffers/ungueltig/loudness-i-pair-plus-status.bin"
            ),
        ),
    ] {
        let batch = eqcop_broker::telemetrie::fuer_broker(payload)
            .unwrap_or_else(|verstoesse| panic!("{name}: {verstoesse:?}"));
        assert_eq!(batch.frames.len(), 1, "{name}");
        assert_eq!(batch.lautheit_ungueltige_instance_ids.len(), 1, "{name}");
        assert!(!batch.verstoesse.is_empty(), "{name}");
        assert!(eqcop_broker::telemetrie::pruefe(&batch.payload).is_empty(), "{name}");
        assert_eq!(batch.frames[0].lufs_i_paar, None, "{name}");
        assert_eq!(batch.frames[0].lufs_i_status, None, "{name}");
    }
}

#[test]
fn telemetry_silence_does_not_change_control_liveness() {
    let (c, clock, _push) = coordinator();
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 1, 2048, None, None, Some(1)),
    );
    for sekunde in 1..=6 {
        clock.setze_ms(sekunde * 1000);
        assert!(heartbeat(&c, "probe", &source, sekunde + 1));
        c.liveness_tick();
    }
    let control = &c.modell_sicht(&hex(1), &hex(2)).clients[0];
    assert!(control.verbunden);
    assert!(!control.stale);
    let messung = c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap();
    assert_eq!(messung.sequence, Some(1));
    assert_eq!(messung.alter_ms, Some(6000));
}

#[test]
fn unbind_removes_confirmed_membership_from_next_absolute_snapshot() {
    let (c, _clock, push) = coordinator();
    let main = adresse(1, 2, 10, 100);
    let source = adresse(1, 2, 20, 200);
    anmelden(&c, "main", &main, "main", Some(77));
    assert!(heartbeat(&c, "main", &main, 1));
    quelle_anmelden(&c, "probe", &source);
    assert!(abonnieren(&c, "main", &main));
    assert!(c.beitritt_bestaetigen(&hex(1), &hex(2), &hex(20)));
    assert!(c.beitritt_aufheben(&hex(1), &hex(2), &hex(20)));

    let source_sicht = c
        .modell_sicht(&hex(1), &hex(2))
        .clients
        .into_iter()
        .find(|client| client.adresse.instance_id == hex(20))
        .unwrap();
    assert!(!source_sicht.bestaetigt);
    let snapshot = push.snapshots().last().unwrap().1.clone();
    assert_eq!(snapshot["type"], "session_snapshot");
    assert_eq!(snapshot["beitritt_bestaetigung_noetig"], true);
}

#[test]
fn handover_snapshot_revokes_old_main_before_new_write() {
    let (c, _clock, push) = coordinator();
    let a = adresse(1, 2, 10, 100);
    let b = adresse(1, 2, 11, 101);
    anmelden(&c, "main-a", &a, "main", Some(77));
    assert!(heartbeat(&c, "main-a", &a, 1));
    anmelden(&c, "main-b", &b, "main", Some(77));
    assert!(heartbeat(&c, "main-b", &b, 1));
    let _ = c.beitritt_bestaetigen(&hex(1), &hex(2), &hex(10));
    let _ = c.beitritt_bestaetigen(&hex(1), &hex(2), &hex(11));
    assert!(abonnieren(&c, "main-a", &a));
    assert!(c.fuehrung_uebergeben(&hex(1), &hex(2), &hex(10)));
    assert!(c.main_darf_schreiben("main-a"));
    assert!(!c.main_darf_schreiben("main-b"));

    assert!(c.fuehrung_uebergeben(&hex(1), &hex(2), &hex(11)));
    assert!(!c.main_darf_schreiben("main-a"));
    assert!(c.main_darf_schreiben("main-b"));
    assert_eq!(
        push.snapshots().last().unwrap().1["fuehrendes_main"],
        b.instance_id
    );
}

#[test]
fn bridge_pid_change_preserves_membership_but_never_old_control_address() {
    let (c, _clock, _push) = coordinator();
    let alt = adresse(1, 2, 20, 200);
    anmelden(&c, "alt", &alt, "active_probe", Some(1000));
    assert!(heartbeat(&c, "alt", &alt, 1));
    Senke::telemetrie_gekoppelt(&c, "alt");
    assert!(c.beitritt_bestaetigen(&hex(1), &hex(2), &hex(20)));
    Senke::p2(
        &c,
        "alt",
        &feature_batch(&alt, 1, 2048, None, None, Some(1)),
    );

    let mut neu = alt.clone();
    neu.runtime_nonce = hex(201);
    let registrierung =
        c.control_hello_registrieren("neu", &hello(&neu, "active_probe", Some(2000)));
    assert!(registrierung.angenommen);
    assert_eq!(registrierung.zu_schliessende_links, vec!["alt"]);
    assert_eq!(
        c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap().sequence,
        None
    );
    Senke::p2(
        &c,
        "alt",
        &feature_batch(&alt, 2, 2048, None, None, Some(1)),
    );
    assert_eq!(
        c.p2_live_frames(),
        1,
        "alte Nonce darf nicht mehr schreiben"
    );

    assert!(heartbeat(&c, "neu", &neu, 1));
    Senke::telemetrie_gekoppelt(&c, "neu");
    Senke::p2(
        &c,
        "neu",
        &feature_batch(&neu, 3, 2048, None, None, Some(1)),
    );
    let modell = c.modell_sicht(&hex(1), &hex(2));
    assert_eq!(modell.clients.len(), 1);
    assert_eq!(modell.clients[0].adresse.runtime_nonce, neu.runtime_nonce);
    assert!(modell.clients[0].bestaetigt);
    assert_eq!(
        c.messsicht(&hex(1), &hex(2), &hex(20)).unwrap().sequence,
        Some(3)
    );
}

#[test]
fn eviction_removes_volatile_measurement_projection() {
    let (c, clock, _push) = coordinator();
    let source = adresse(1, 2, 20, 200);
    quelle_anmelden(&c, "probe", &source);
    Senke::p2(
        &c,
        "probe",
        &feature_batch(&source, 1, 2048, None, None, Some(1)),
    );
    c.control_ende("probe");
    clock.setze_ms(TOMBSTONE_MS);
    c.liveness_tick();
    assert!(c.messsicht(&hex(1), &hex(2), &hex(20)).is_none());
}

#[test]
fn confirm_join_and_unbind_full_roundtrip() {
    let (c, _clock, push) = coordinator();
    let main = adresse(1, 2, 10, 100);
    let source = adresse(1, 2, 20, 200);
    anmelden(&c, "main", &main, "main", Some(77));
    assert!(heartbeat(&c, "main", &main, 1));
    let _ = c.beitritt_bestaetigen(&hex(1), &hex(2), &hex(10));
    assert!(c.fuehrung_uebergeben(&hex(1), &hex(2), &hex(10)));
    anmelden(&c, "probe", &source, "active_probe", Some(9001));
    assert!(heartbeat(&c, "probe", &source, 1));
    assert!(abonnieren(&c, "main", &main));

    let confirm = session_command(&c, "main", "confirm_join", 700, &source, &hex(2));
    assert_eq!(confirm["type"], "command_ack");
    assert_eq!(confirm["ergebnis"], "angewandt");
    assert_eq!(confirm["state_hash"].as_str().unwrap().len(), 64);
    assert!(c
        .modell_sicht(&hex(1), &hex(2))
        .clients
        .iter()
        .find(|client| client.adresse.instance_id == source.instance_id)
        .unwrap()
        .bestaetigt);
    assert_eq!(
        push.snapshots().last().unwrap().1["beitritt_bestaetigung_noetig"],
        false
    );

    let retry = session_command(&c, "main", "confirm_join", 700, &source, &hex(2));
    assert_eq!(retry["ergebnis"], "idempotent_wiederholt");
    assert_eq!(retry["state_hash"], confirm["state_hash"]);

    let unbind = session_command(&c, "main", "unbind_probe", 701, &source, &hex(2));
    assert_eq!(unbind["ergebnis"], "angewandt");
    assert!(!c
        .modell_sicht(&hex(1), &hex(2))
        .clients
        .iter()
        .find(|client| client.adresse.instance_id == source.instance_id)
        .unwrap()
        .bestaetigt);
    assert_eq!(
        push.snapshots().last().unwrap().1["beitritt_bestaetigung_noetig"],
        true
    );

    let wrong_epoch = session_command(&c, "main", "confirm_join", 702, &source, &hex(3));
    assert_eq!(wrong_epoch["ergebnis"], "abgelehnt");
    assert_eq!(wrong_epoch["code"], "unauthorized");
}

#[test]
fn foreign_main_command_is_rejected() {
    let (c, _clock, _push) = coordinator();
    let main_a = adresse(1, 2, 10, 100);
    let main_b = adresse(1, 2, 11, 101);
    let source = adresse(1, 2, 20, 200);
    anmelden(&c, "main-a", &main_a, "main", Some(77));
    anmelden(&c, "main-b", &main_b, "main", Some(77));
    let _ = c.beitritt_bestaetigen(&hex(1), &hex(2), &hex(10));
    let _ = c.beitritt_bestaetigen(&hex(1), &hex(2), &hex(11));
    assert!(c.fuehrung_uebergeben(&hex(1), &hex(2), &hex(10)));
    anmelden(&c, "probe", &source, "active_probe", Some(9001));

    let ack = session_command(&c, "main-b", "confirm_join", 710, &source, &hex(2));
    assert_eq!(ack["ergebnis"], "abgelehnt");
    assert_eq!(ack["code"], "unauthorized");
    assert!(!c
        .modell_sicht(&hex(1), &hex(2))
        .clients
        .iter()
        .find(|client| client.adresse.instance_id == source.instance_id)
        .unwrap()
        .bestaetigt);
}

#[test]
fn store_degraded_travels_only_as_true() {
    let clock = Arc::new(ManualClock::default());
    let writer = StoreWriter::degradiert_ohne_pfad("sonde012-test");
    let c = Coordinator::mit_store(clock, hex(0xbeef), &writer);
    let push = Arc::new(PushProbe::default());
    c.session_push_setzen(push.clone());
    let main = adresse(1, 2, 10, 100);
    anmelden(&c, "main", &main, "main", Some(77));
    assert!(abonnieren(&c, "main", &main));
    let snapshots = push.snapshots();
    assert!(
        !snapshots.is_empty(),
        "diagnostic snapshot was not pushed; disconnect={} payload={}",
        c.verbindung_soll_trennen("main"),
        String::from_utf8_lossy(&c.session_snapshot_json(&hex(1), &hex(2)))
    );
    assert_eq!(snapshots.last().unwrap().1["store_degraded"], true);

    let (normal, _normal_clock, _) = coordinator();
    assert!(normal
        .session_snapshot_json(&hex(1), &hex(2))
        .windows(b"store_degraded".len())
        .all(|w| w != b"store_degraded"));
}
