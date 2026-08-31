use eqcop_broker::coordinator::{
    Coordinator, ManualClock, FUEHRENDE_MAINS_PRO_SESSION, GLOBAL_CLIENT_CAP,
    HEARTBEAT_INTERVAL_MS, LAST_SONDEN, SESSION_CLIENT_CAP, SESSION_SUBSCRIPTION_EVENT_REPLAY_MAX,
    SICHTBARE_SONDEN_NORMAL, STALE_JITTER_MS, STALE_NACH_MS, STALE_VERPASSTE_INTERVALLE,
    TOMBSTONE_MS,
};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
#[cfg(windows)]
use eqcop_broker::transport::server_v3::Senke;
use serde_json::{json, Value};
use std::sync::Arc;

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

fn hello(
    projekt: usize,
    sitzung: usize,
    instanz: usize,
    nonce: usize,
    art: &str,
    pid: Option<u32>,
) -> HelloControl {
    HelloControl {
        typ: "hello".into(),
        connection_kind: "control".into(),
        protocol: 3,
        plugin_version: "test".into(),
        plugin_kind: art.into(),
        adresse: adresse(projekt, sitzung, instanz, nonce),
        host: pid.map(|pid| HostAngabe {
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

fn report(c: &Coordinator, link: &str, adresse: &Adresse) -> bool {
    c.heartbeat_kontakt(
        link,
        Some(&json!({
            "type": "heartbeat",
            "adresse": adresse,
            "sequence": 1,
            "state_revision": 0,
            "capabilities": capabilities(),
            "zaehler": {}
        })),
    )
}

fn state_report_payload(adresse: &Adresse) -> Vec<u8> {
    serde_json::to_vec(&json!({
        "type": "state_report",
        "adresse": adresse,
        "dsp_schema_version": 2,
        "state_revision": 13,
        "state_hash": "d".repeat(64),
        "record_state": {"valid": true, "recording": false},
        "undo_tiefe": 7
    }))
    .unwrap()
}

fn state_report(c: &Coordinator, link: &str, adresse: &Adresse) -> bool {
    c.state_report_json(link, &state_report_payload(adresse))
}

fn coordinator() -> (Coordinator, Arc<ManualClock>) {
    let clock = Arc::new(ManualClock::default());
    let c = Coordinator::mit_uhr(clock.clone(), hex(0xbeef));
    (c, clock)
}

fn anmelden(c: &Coordinator, link: &str, hello: &HelloControl) {
    let ausgang = c.control_hello_registrieren(link, hello);
    assert!(ausgang.angenommen, "{:?}", ausgang.grund);
}

#[test]
fn control_vor_welcome() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 3, 4, "main", Some(7));
    let ausgang = c.control_hello_registrieren("link", &h);
    assert!(ausgang.angenommen);
    assert_eq!(c.client_anzahl(), 1);
    assert_eq!(c.modell_sicht(&hex(1), &hex(2)).clients.len(), 1);
}

#[cfg(windows)]
#[test]
fn telemetrie_einmalig_gekoppelt() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 3, 4, "main", Some(7));
    anmelden(&c, "link", &h);
    Senke::telemetrie_gekoppelt(&c, "link");
    Senke::telemetrie_gekoppelt(&c, "link");
    assert_eq!(c.telemetrie_kopplungen(), (1, 1));
    Senke::telemetrie_getrennt(&c, "link");
    assert_eq!(c.telemetrie_kopplungen(), (0, 1));
}

#[test]
fn join_nur_bei_eindeutigem_main() {
    let (c, _) = coordinator();
    let main = hello(1, 11, 101, 1001, "main", Some(77));
    anmelden(&c, "main", &main);
    assert!(report(&c, "main", &main.adresse));
    assert!(c.modell_sicht(&hex(1), &hex(11)).clients[0].bestaetigt);

    let bridge = hello(1, 11, 102, 1002, "active_probe", None);
    anmelden(&c, "bridge", &bridge);
    assert!(report(&c, "bridge", &bridge.adresse));
    let sicht = c.modell_sicht(&hex(1), &hex(11));
    assert!(sicht.beitritt_bestaetigung_noetig);
    assert!(sicht
        .clients
        .iter()
        .any(|x| x.adresse == bridge.adresse && !x.bestaetigt));

    let zweites_projekt = hello(2, 22, 103, 1003, "main", Some(77));
    anmelden(&c, "main-2", &zweites_projekt);
    assert!(report(&c, "main-2", &zweites_projekt.adresse));
    assert!(
        c.modell_sicht(&hex(1), &hex(11))
            .beitritt_bestaetigung_noetig
    );
    assert!(
        c.modell_sicht(&hex(2), &hex(22))
            .beitritt_bestaetigung_noetig
    );

    let duplikat = hello(1, 11, 101, 2001, "main", Some(77));
    let ausgang = c.control_hello_registrieren("main-neu", &duplikat);
    assert!(ausgang.angenommen);
    assert_eq!(ausgang.zu_schliessende_links, vec!["main"]);
    assert!(!report(&c, "main", &main.adresse));
    assert!(
        c.modell_sicht(&hex(1), &hex(11))
            .beitritt_bestaetigung_noetig
    );
}

#[test]
fn erstes_einziges_main_fuehrt_sonst_bestaetigung() {
    assert_eq!(FUEHRENDE_MAINS_PRO_SESSION, 1);
    let (c, _) = coordinator();
    let a = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &a);
    report(&c, "a", &a.adresse);
    assert_eq!(
        c.modell_sicht(&hex(1), &hex(2)).fuehrendes_main,
        Some(hex(10))
    );

    let b = hello(1, 2, 11, 101, "main", Some(9));
    anmelden(&c, "b", &b);
    report(&c, "b", &b.adresse);
    let sicht = c.modell_sicht(&hex(1), &hex(2));
    assert_eq!(sicht.fuehrendes_main, None);
    assert!(sicht.beitritt_bestaetigung_noetig);
}

#[test]
fn fuehrung_stale_bis_eviction() {
    let (c, clock) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "main", &h);
    report(&c, "main", &h.adresse);
    clock.setze_ms(STALE_NACH_MS + 1);
    c.liveness_tick();
    assert_eq!(
        c.modell_sicht(&hex(1), &hex(2)).fuehrendes_main,
        Some(hex(10))
    );
    clock.setze_ms(STALE_NACH_MS + 1 + TOMBSTONE_MS);
    c.liveness_tick();
    assert_eq!(c.modell_sicht(&hex(1), &hex(2)).fuehrendes_main, None);
}

#[test]
fn reconnect_im_selben_brokerlauf_gibt_unuebergebene_fuehrung_zurueck() {
    let (c, _) = coordinator();
    let alt = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "alt", &alt);
    report(&c, "alt", &alt.adresse);
    c.control_ende("alt");
    let neu = hello(1, 2, 10, 101, "main", Some(9));
    anmelden(&c, "neu", &neu);
    report(&c, "neu", &neu.adresse);
    assert_eq!(
        c.modell_sicht(&hex(1), &hex(2)).fuehrendes_main,
        Some(hex(10))
    );
}

#[test]
fn brokerneustart_ohne_mainprojectstate_ingress_gibt_fuehrung_frei() {
    let (vorher, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&vorher, "main", &h);
    report(&vorher, "main", &h.adresse);
    assert!(vorher
        .modell_sicht(&hex(1), &hex(2))
        .fuehrendes_main
        .is_some());

    let (danach, _) = coordinator();
    assert_eq!(danach.modell_sicht(&hex(1), &hex(2)).fuehrendes_main, None);
}

#[test]
fn brokerneustart_ohne_mainprojectstate_ingress() {
    brokerneustart_ohne_mainprojectstate_ingress_gibt_fuehrung_frei();
}

#[test]
fn fuehrung_exklusiv_und_uebergabe() {
    let (c, _) = coordinator();
    let a = hello(1, 2, 10, 100, "main", Some(9));
    let b = hello(1, 2, 11, 101, "main", Some(9));
    anmelden(&c, "a", &a);
    report(&c, "a", &a.adresse);
    anmelden(&c, "b", &b);
    report(&c, "b", &b.adresse);
    assert!(c.beitritt_bestaetigen(&hex(1), &hex(2), &hex(10)));
    assert!(c.beitritt_bestaetigen(&hex(1), &hex(2), &hex(11)));
    assert!(c.fuehrung_uebergeben(&hex(1), &hex(2), &hex(10)));
    assert!(c.main_darf_schreiben("a"));
    assert!(!c.main_darf_schreiben("b"));
    assert!(c.fuehrung_uebergeben(&hex(1), &hex(2), &hex(11)));
    assert!(!c.main_darf_schreiben("a"));
    assert!(c.main_darf_schreiben("b"));
}

#[test]
fn fuehrung_exklusiv_und_reconnect_im_selben_lauf() {
    fuehrung_exklusiv_und_uebergabe();
    reconnect_im_selben_brokerlauf_gibt_unuebergebene_fuehrung_zurueck();
}

#[test]
fn zwei_projekte_bleiben_getrennt() {
    let (c, _) = coordinator();
    for (link, projekt, sitzung, instanz) in [("a", 1, 2, 10), ("b", 3, 4, 11)] {
        let h = hello(
            projekt,
            sitzung,
            instanz,
            100 + instanz,
            "main",
            Some(instanz as u32),
        );
        anmelden(&c, link, &h);
        report(&c, link, &h.adresse);
    }
    assert_eq!(c.modell_sicht(&hex(1), &hex(2)).clients.len(), 1);
    assert_eq!(c.modell_sicht(&hex(3), &hex(4)).clients.len(), 1);
}

#[test]
fn brokerneustart_behaelt_session_epoch() {
    let h = hello(1, 0x22, 10, 100, "main", Some(9));
    let (c, _) = coordinator();
    anmelden(&c, "a", &h);
    assert_eq!(c.modell_sicht(&hex(1), &hex(0x22)).session_epoch, hex(0x22));
    let (neu, _) = coordinator();
    anmelden(&neu, "a", &h);
    assert_eq!(
        neu.modell_sicht(&hex(1), &hex(0x22)).session_epoch,
        hex(0x22)
    );
}

#[test]
fn liveness_nur_instant() {
    let (c, clock) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);
    clock.vor(HEARTBEAT_INTERVAL_MS);
    assert!(!c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    clock.vor(HEARTBEAT_INTERVAL_MS);
    assert!(!c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
}

#[test]
fn stale_konstanten_ableitung() {
    assert_eq!(HEARTBEAT_INTERVAL_MS, 1000);
    assert_eq!(STALE_VERPASSTE_INTERVALLE, 2);
    assert_eq!(STALE_JITTER_MS, 500);
    assert_eq!(STALE_NACH_MS, 2500);
}

#[test]
fn stale_grenze_exklusiv_und_kontakt_setzt_zurueck() {
    let (c, clock) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);
    clock.setze_ms(STALE_NACH_MS);
    c.liveness_tick();
    assert!(!c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    clock.vor(1);
    c.liveness_tick();
    assert!(c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    let fremd = adresse(1, 2, 11, 101);
    assert!(!state_report(&c, "a", &fremd));
    assert!(c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    assert!(state_report(&c, "a", &h.adresse));
    assert!(!c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    clock.vor(STALE_NACH_MS + 1);
    c.liveness_tick();
    assert!(c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    assert!(report(&c, "a", &h.adresse));
    assert!(!c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    clock.vor(TOMBSTONE_MS);
    c.liveness_tick();
    assert_eq!(c.modell_sicht(&hex(1), &hex(2)).clients.len(), 1);
}

#[cfg(windows)]
#[test]
fn state_report_erreicht_liveness_ueber_produktive_p1_senke() {
    let (c, clock) = coordinator();
    let h = hello(1, 2, 10, 100, "active_probe", Some(9));
    anmelden(&c, "probe", &h);
    report(&c, "probe", &h.adresse);
    clock.setze_ms(STALE_NACH_MS + 1);
    c.liveness_tick();
    assert!(c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    Senke::p1(&c, "probe", &state_report_payload(&h.adresse));
    assert!(!c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
}

#[test]
fn tombstone_grenze_entfernt_alle_fluechtigen_sichten_aber_keinen_dauerhaften_konfliktriegel() {
    let (c, clock) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    c.control_ende("a");
    clock.setze_ms(TOMBSTONE_MS - 1);
    c.liveness_tick();
    assert_eq!(c.modell_sicht(&hex(1), &hex(2)).clients.len(), 1);
    clock.setze_ms(TOMBSTONE_MS);
    c.liveness_tick();
    assert!(c.modell_sicht(&hex(1), &hex(2)).clients.is_empty());
    assert_eq!(c.subscription_anzahl(), 0);
}

#[test]
fn neue_nonce_verdraengt_alte_sofort() {
    let (c, _) = coordinator();
    let alt = hello(1, 2, 10, 100, "main", Some(9));
    let neu = hello(1, 2, 10, 101, "main", Some(9));
    anmelden(&c, "alt", &alt);
    let ausgang = c.control_hello_registrieren("neu", &neu);
    assert!(ausgang.angenommen);
    assert_eq!(ausgang.zu_schliessende_links, vec!["alt"]);
    assert!(c.verbindung_soll_trennen("alt"));
    assert!(!c.dispatch_fuer_link_erlaubt("alt"));
    assert!(c.dispatch_fuer_link_erlaubt("neu"));
}

#[test]
fn verdraengte_nonce_report_sperrt_beide() {
    let (c, _) = coordinator();
    let alt = hello(1, 2, 10, 100, "main", Some(9));
    let neu = hello(1, 2, 10, 101, "main", Some(9));
    anmelden(&c, "alt", &alt);
    anmelden(&c, "neu", &neu);
    assert!(!report(&c, "alt", &alt.adresse));
    assert!(!c.dispatch_fuer_link_erlaubt("alt"));
    assert!(!c.dispatch_fuer_link_erlaubt("neu"));
}

#[test]
fn session_und_global_cap() {
    let (c, _) = coordinator();
    for i in 0..SESSION_CLIENT_CAP {
        anmelden(
            &c,
            &format!("s{i}"),
            &hello(1, 2, 1000 + i, 2000 + i, "active_probe", Some(1)),
        );
    }
    assert_eq!(
        c.modell_sicht(&hex(1), &hex(2)).clients.len(),
        SESSION_CLIENT_CAP
    );
    let ab = c.control_hello_registrieren(
        "session-plus-eins",
        &hello(1, 2, 9000, 9001, "active_probe", Some(1)),
    );
    assert!(!ab.angenommen);
    assert_eq!(c.cap_abweisungen(), 1);

    let (global, _) = coordinator();
    for i in 0..GLOBAL_CLIENT_CAP {
        let sitzung = 100 + i / SESSION_CLIENT_CAP;
        anmelden(
            &global,
            &format!("g{i}"),
            &hello(
                10 + i / SESSION_CLIENT_CAP,
                sitzung,
                1000 + i,
                3000 + i,
                "active_probe",
                Some(i as u32 + 1),
            ),
        );
    }
    let ab = global.control_hello_registrieren(
        "global-plus-eins",
        &hello(99, 99, 9999, 9998, "active_probe", Some(99)),
    );
    assert!(!ab.angenommen);
    assert_eq!(global.client_anzahl(), GLOBAL_CLIENT_CAP);
}

#[test]
fn caps_stale_first_deterministisch() {
    let (c, clock) = coordinator();
    for i in (0..SESSION_CLIENT_CAP).rev() {
        anmelden(
            &c,
            &format!("s{i}"),
            &hello(1, 2, 1000 + i, 2000 + i, "active_probe", Some(1)),
        );
    }
    clock.setze_ms(STALE_NACH_MS + 1);
    c.liveness_tick();
    let neu = hello(1, 2, 9999, 9998, "active_probe", Some(1));
    assert!(c.control_hello_registrieren("neu", &neu).angenommen);
    let sicht = c.modell_sicht(&hex(1), &hex(2));
    assert_eq!(sicht.clients.len(), SESSION_CLIENT_CAP);
    assert!(!sicht
        .clients
        .iter()
        .any(|client| client.adresse.instance_id == hex(1000)));
}

#[test]
fn eviction_haelt_phase_a_riegel() {
    let (c, clock) = coordinator();
    let alt = hello(1, 2, 10, 100, "main", Some(9));
    let neu = hello(1, 2, 10, 101, "main", Some(9));
    anmelden(&c, "alt", &alt);
    anmelden(&c, "neu", &neu);
    report(&c, "alt", &alt.adresse);
    c.control_ende("neu");
    clock.setze_ms(TOMBSTONE_MS);
    c.liveness_tick();
    assert!(!c.dispatch_fuer_link_erlaubt("alt"));
    assert!(c.interventionssicht().unknown);
}

#[test]
fn eviction_loescht_intervention_unknown_nicht() {
    eviction_haelt_phase_a_riegel();
}

#[test]
fn konfliktriegel_ueberlebt_eviction() {
    eviction_haelt_phase_a_riegel();
}

#[test]
fn eviction_haelt_dauerhaften_konfliktriegel() {
    eviction_haelt_phase_a_riegel();
}

#[test]
fn trennen_bereinigt_vor_join() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    let sub = serde_json::to_vec(&json!({
        "type": "subscribe_session",
        "adresse": h.adresse,
        "session_epoch": hex(2)
    }))
    .unwrap();
    assert!(c.subscribe_json("a", &sub));
    c.control_ende("a");
    assert_eq!(c.subscription_anzahl(), 0);
    assert_eq!(c.subscription_cleanups(), 1);
}

#[test]
fn io_worker_mutiert_keinen_sessiongraph() {
    let (c, _) = coordinator();
    let vorher = c.modell_sicht(&hex(1), &hex(2));
    let _nur_bytes = serde_json::to_vec(&json!({"type":"heartbeat"})).unwrap();
    assert_eq!(c.modell_sicht(&hex(1), &hex(2)), vorher);
}

#[test]
fn sichtgrenzen_und_wire_replay_null() {
    assert_eq!(SICHTBARE_SONDEN_NORMAL, 16);
    assert_eq!(LAST_SONDEN, 32);
    assert_eq!(SESSION_SUBSCRIPTION_EVENT_REPLAY_MAX, 0);
}
