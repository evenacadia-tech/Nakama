use eqcop_broker::coordinator::{
    Coordinator, ManualClock, MonotonicClock, FUEHRENDE_MAINS_PRO_SESSION, GLOBAL_CLIENT_CAP,
    GLOBAL_SESSION_CAP,
    HEARTBEAT_INTERVAL_MS, LAST_SONDEN, SESSION_CLIENT_CAP, SESSION_SUBSCRIPTION_EVENT_REPLAY_MAX,
    SICHTBARE_SONDEN_NORMAL, STALE_JITTER_MS, STALE_NACH_MS, STALE_VERPASSTE_INTERVALLE,
    TOMBSTONE_MS,
};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
#[cfg(windows)]
use eqcop_broker::transport::server_v3::Senke;
use serde_json::{json, Value};
use std::sync::atomic::{AtomicI64, AtomicU64, Ordering};
use std::sync::Arc;
use std::time::Duration;

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
            "zaehler": {},
            "runtime": {"messpunkt": "insert", "betrieb": "active"}
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

#[derive(Debug, Default)]
struct GetrennteTestuhren {
    monoton_ms: AtomicU64,
    wanduhr_ms: AtomicI64,
}

impl GetrennteTestuhren {
    fn monoton_setzen(&self, millis: u64) {
        self.monoton_ms.store(millis, Ordering::SeqCst);
    }

    fn wanduhr_setzen(&self, millis: i64) {
        self.wanduhr_ms.store(millis, Ordering::SeqCst);
    }

    fn wanduhr(&self) -> i64 {
        self.wanduhr_ms.load(Ordering::SeqCst)
    }
}

impl MonotonicClock for GetrennteTestuhren {
    fn jetzt(&self) -> Duration {
        Duration::from_millis(self.monoton_ms.load(Ordering::SeqCst))
    }
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
fn probe_ohne_eigene_epoche_uebernimmt_eindeutige_main_sitzung() {
    let (c, _) = coordinator();
    let main = hello(1, 11, 101, 1001, "main", Some(77));
    anmelden(&c, "main", &main);
    assert!(report(&c, "main", &main.adresse));

    // Probeeq kennt beim Hello nur die persistierte Projektbindung. Derselbe
    // hex32-Wert im bestehenden session_epoch-Feld ist der ungebundene
    // Join-Marker, keine selbst erzeugte Sitzungsidentitaet.
    let probe = hello(1, 1, 102, 1002, "active_probe", Some(77));
    assert_eq!(
        probe.adresse.session_epoch,
        probe.adresse.project_binding_id
    );
    anmelden(&c, "probe", &probe);
    assert!(report(&c, "probe", &probe.adresse));

    let sicht = c.modell_sicht(&hex(1), &hex(11));
    assert_eq!(sicht.clients.len(), 2);
    assert_eq!(
        sicht.fuehrendes_main,
        Some(main.adresse.instance_id.clone())
    );
    let probe_sicht = sicht
        .clients
        .iter()
        .find(|client| client.adresse.instance_id == probe.adresse.instance_id)
        .expect("Probeeq ist Mitglied der Main-Sitzung");
    assert!(probe_sicht.bestaetigt);
    assert_eq!(
        probe_sicht.adresse.session_epoch,
        main.adresse.session_epoch
    );
    assert!(c.modell_sicht(&hex(1), &hex(1)).clients.is_empty());

    let snapshot: Value =
        serde_json::from_slice(&c.session_snapshot_json(&hex(1), &hex(11))).unwrap();
    let probe_adresse = snapshot["mitglieder"]
        .as_array()
        .unwrap()
        .iter()
        .find(|client| client["adresse"]["instance_id"] == probe.adresse.instance_id)
        .expect("Probeeq erscheint im bestehenden session_snapshot");
    assert_eq!(
        probe_adresse["adresse"]["session_epoch"],
        main.adresse.session_epoch
    );
}

#[test]
fn join_reconnect_disconnect_setzt_sticky_unknown_bis_resync() {
    let (c, _) = coordinator();
    let probe = hello(1, 1, 102, 1002, "active_probe", Some(77));
    anmelden(&c, "probe-alt", &probe);
    let main = hello(1, 11, 101, 1001, "main", Some(77));
    let main_anmeldung = c.control_hello_registrieren("main", &main);
    assert!(main_anmeldung.angenommen);
    assert_eq!(main_anmeldung.zu_schliessende_links, vec!["probe-alt"]);

    // Auch der vom internen Join angestossene Control-Disconnect ist C-08:
    // kein Begin/End muss vorher angekommen sein, damit Unknown sticky wird.
    c.control_ende("probe-alt");
    assert!(c.interventionssicht().unknown);
    assert!(!c.interventionssicht().starke_evidenz_erlaubt);
    c.hoermarkierung_v2("legacy", false);
    assert!(c.interventionssicht().unknown);
    assert!(c.neutral_resync("main", 0));
    assert!(c.interventionssicht().starke_evidenz_erlaubt);
}

#[test]
fn probe_join_bleibt_reihenfolgefest_und_bei_mehreren_mains_fail_closed() {
    // Probe zuerst: Sobald das Main erscheint, fordert der Coordinator einen
    // normalen Reconnect an. Erst dessen neues Hello wird kanonisch gebunden;
    // der Marker hinterlaesst keinen Phantom-Tombstone.
    let (c, _) = coordinator();
    let probe = hello(1, 1, 102, 1002, "active_probe", Some(77));
    anmelden(&c, "probe-alt", &probe);
    assert!(report(&c, "probe-alt", &probe.adresse));
    let main = hello(1, 11, 101, 1001, "main", Some(77));
    let main_anmeldung = c.control_hello_registrieren("main", &main);
    assert!(main_anmeldung.angenommen);
    assert_eq!(main_anmeldung.zu_schliessende_links, vec!["probe-alt"]);
    c.control_ende("probe-alt");
    assert!(report(&c, "main", &main.adresse));
    anmelden(&c, "probe-neu", &probe);
    assert!(report(&c, "probe-neu", &probe.adresse));
    assert!(c.modell_sicht(&hex(1), &hex(1)).clients.is_empty());
    let sichtbar = c.modell_sicht(&hex(1), &hex(11));
    assert_eq!(sichtbar.clients.len(), 2);
    assert!(!sichtbar.beitritt_bestaetigung_noetig);

    // Zwei Main-Sitzungen derselben Projektkopien im selben Host: kein
    // heuristischer Beitritt. Der Kandidat bleibt intern ungebunden, und beide
    // bestehenden Snapshots zeigen den Bestaetigungsbedarf.
    let (mehrdeutig, _) = coordinator();
    let main_a = hello(2, 21, 201, 2001, "main", Some(88));
    let main_b = hello(2, 22, 202, 2002, "main", Some(88));
    anmelden(&mehrdeutig, "main-a", &main_a);
    assert!(report(&mehrdeutig, "main-a", &main_a.adresse));
    anmelden(&mehrdeutig, "main-b", &main_b);
    assert!(report(&mehrdeutig, "main-b", &main_b.adresse));
    let probe = hello(2, 2, 203, 2003, "active_probe", Some(88));
    let probe_anmeldung = mehrdeutig.control_hello_registrieren("probe", &probe);
    assert!(probe_anmeldung.angenommen);
    assert!(probe_anmeldung.zu_schliessende_links.is_empty());
    assert!(report(&mehrdeutig, "probe", &probe.adresse));
    assert!(
        mehrdeutig
            .modell_sicht(&hex(2), &hex(21))
            .beitritt_bestaetigung_noetig
    );
    assert!(
        mehrdeutig
            .modell_sicht(&hex(2), &hex(22))
            .beitritt_bestaetigung_noetig
    );
    let kandidat = mehrdeutig.modell_sicht(&hex(2), &hex(2));
    assert_eq!(kandidat.clients.len(), 1);
    assert!(!kandidat.clients[0].bestaetigt);
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
    let quelle = include_str!("../src/coordinator/liveness.rs");
    let liveness_anfang = quelle
        .find("fn stale_aktualisieren_locked")
        .expect("Liveness-Quellschnitt beginnt");
    let liveness_ende = quelle[liveness_anfang..]
        .find("    pub fn heartbeat_kontakt")
        .map(|versatz| liveness_anfang + versatz)
        .expect("Liveness-Quellschnitt endet");
    let liveness_quelle = &quelle[liveness_anfang..liveness_ende];
    assert!(liveness_quelle.contains("let jetzt = self.clock.jetzt();"));
    assert!(!liveness_quelle.contains("SystemTime"));
    assert!(!liveness_quelle.contains("UNIX_EPOCH"));
    assert!(!liveness_quelle.contains("persistenz_utc_ms"));

    let clock = Arc::new(GetrennteTestuhren::default());
    let c = Coordinator::mit_uhr(clock.clone(), hex(0xbeef));
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    assert!(report(&c, "a", &h.adresse));

    clock.wanduhr_setzen(i64::MAX);
    assert_eq!(clock.wanduhr(), i64::MAX);
    clock.monoton_setzen(HEARTBEAT_INTERVAL_MS);
    c.liveness_tick();
    let sicht = c.modell_sicht(&hex(1), &hex(2));
    assert!(!sicht.clients[0].stale);
    assert_eq!(sicht.clients[0].letzter_kontakt_ms, HEARTBEAT_INTERVAL_MS);

    clock.wanduhr_setzen(i64::MIN);
    assert_eq!(clock.wanduhr(), i64::MIN);
    clock.monoton_setzen(2 * HEARTBEAT_INTERVAL_MS);
    c.liveness_tick();
    let sicht = c.modell_sicht(&hex(1), &hex(2));
    assert!(!sicht.clients[0].stale);
    assert_eq!(
        sicht.clients[0].letzter_kontakt_ms,
        2 * HEARTBEAT_INTERVAL_MS
    );

    clock.wanduhr_setzen(0);
    assert_eq!(clock.wanduhr(), 0);
    clock.monoton_setzen(STALE_NACH_MS);
    c.liveness_tick();
    assert!(!c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    clock.monoton_setzen(STALE_NACH_MS + 1);
    c.liveness_tick();
    assert!(c.modell_sicht(&hex(1), &hex(2)).clients[0].stale);
    assert!(report(&c, "a", &h.adresse));
    let sicht = c.modell_sicht(&hex(1), &hex(2));
    assert!(!sicht.clients[0].stale);
    assert_eq!(sicht.clients[0].letzter_kontakt_ms, 0);
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

#[cfg(windows)]
#[test]
fn produktive_json_senken_verlangen_den_vollstaendigen_v3_vertrag() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "main", &h);
    let unvollstaendig = serde_json::to_vec(&json!({
        "type": "heartbeat",
        "adresse": h.adresse,
        "sequence": 1
    }))
    .unwrap();
    assert!(Senke::p0(&c, "main", &unvollstaendig).is_none());
    assert!(!c.modell_sicht(&hex(1), &hex(2)).clients[0].join_kandidat);

    let mit_zusatz = serde_json::to_vec(&json!({
        "type": "heartbeat",
        "adresse": h.adresse,
        "sequence": 1,
        "state_revision": 0,
        "capabilities": capabilities(),
        "zaehler": {
            "frames_dropped": 0,
            "parse_errors": 0,
            "queue_overflows": 0
        },
        "cursor": hex(99)
    }))
    .unwrap();
    assert!(Senke::p0(&c, "main", &mit_zusatz).is_none());
    assert!(!c.modell_sicht(&hex(1), &hex(2)).clients[0].join_kandidat);

    let subscribe_mit_cursor = serde_json::to_vec(&json!({
        "type": "subscribe_session",
        "adresse": h.adresse,
        "session_epoch": h.adresse.session_epoch,
        "cursor": hex(88)
    }))
    .unwrap();
    Senke::p1(&c, "main", &subscribe_mit_cursor);
    assert_eq!(c.subscription_anzahl(), 0);
}

#[cfg(windows)]
#[test]
fn alle_schemafesten_interventionsarten_sperren_dieselbe_evidenz() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "active_probe", Some(9));
    anmelden(&c, "probe", &h);
    let mut sequence = 1u64;
    for (index, art) in ["hoermarkierung", "preview", "focus_burst", "experiment"]
        .into_iter()
        .enumerate()
    {
        let id = hex(500 + index);
        let begin = serde_json::to_vec(&json!({
            "type": "audible_intervention_begin",
            "intervention_id": id,
            "adresse": h.adresse,
            "event_sequence": sequence,
            "art": art,
            "project_sample_start": null
        }))
        .unwrap();
        assert!(Senke::p0(&c, "probe", &begin).is_none());
        assert_eq!(c.interventionssicht().aktive, 1, "{art}");
        assert!(!c.evidence_dispatch(), "{art}");
        sequence += 1;
        let ende = serde_json::to_vec(&json!({
            "type": "audible_intervention_end",
            "intervention_id": id,
            "adresse": h.adresse,
            "event_sequence": sequence,
            "project_sample_end": null,
            "tail_samples": 0
        }))
        .unwrap();
        assert!(Senke::p0(&c, "probe", &ende).is_none());
        assert!(c.interventionssicht().starke_evidenz_erlaubt, "{art}");
        sequence += 1;
    }
}

#[cfg(windows)]
#[test]
fn p2_mutiert_erst_nach_flatbuffers_verifikation() {
    let (c, _) = coordinator();
    c.control_registrieren(
        "probe",
        Adresse {
            logon_sid: "S-1-5-21-1111111111-2222222222-3333333333-1001".into(),
            project_binding_id: "1".repeat(32),
            session_epoch: "2".repeat(32),
            instance_id: format!("{:032x}", 3),
            runtime_nonce: "4".repeat(32),
        },
    );
    Senke::telemetrie_gekoppelt(&c, "probe");
    Senke::p2(&c, "probe", b"kein FlatBuffer");
    assert_eq!(c.p2_live_frames(), 0);
    let gueltig =
        include_bytes!("../../eq-copilot/fixtures/v3/flatbuffers/gueltig/live-64-band.bin");
    Senke::p2(&c, "probe", gueltig);
    assert_eq!(c.p2_live_frames(), 1);
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
    fn session_opfer(reihenfolge: &[usize], aeltestes: usize) -> String {
        let (c, clock) = coordinator();
        for &i in reihenfolge {
            clock.setze_ms(if i == aeltestes { 0 } else { 100 });
            anmelden(
                &c,
                &format!("s{i}"),
                &hello(1, 2, 1000 + i, 2000 + i, "active_probe", Some(1)),
            );
        }
        clock.setze_ms(STALE_NACH_MS + 101);
        c.liveness_tick();
        let neu = hello(1, 2, 9999, 9998, "active_probe", Some(1));
        assert!(c.control_hello_registrieren("neu", &neu).angenommen);
        let sicht = c.modell_sicht(&hex(1), &hex(2));
        assert_eq!(sicht.clients.len(), SESSION_CLIENT_CAP);
        (0..SESSION_CLIENT_CAP)
            .map(|i| hex(1000 + i))
            .find(|id| {
                !sicht
                    .clients
                    .iter()
                    .any(|client| &client.adresse.instance_id == id)
            })
            .expect("genau ein Sessionopfer")
    }

    let vorwaerts = (0..SESSION_CLIENT_CAP).collect::<Vec<_>>();
    let rueckwaerts = (0..SESSION_CLIENT_CAP).rev().collect::<Vec<_>>();
    let mut verschachtelt = (0..SESSION_CLIENT_CAP).step_by(2).collect::<Vec<_>>();
    verschachtelt.extend((1..SESSION_CLIENT_CAP).step_by(2));
    for reihenfolge in [&vorwaerts, &rueckwaerts, &verschachtelt] {
        assert_eq!(session_opfer(reihenfolge, 37), hex(1037));
    }

    fn global_opfer(reihenfolge: &[usize]) -> String {
        let (c, clock) = coordinator();
        for &i in reihenfolge {
            let projekt = 10 + i / SESSION_CLIENT_CAP;
            let sitzung = 20 + i / SESSION_CLIENT_CAP;
            anmelden(
                &c,
                &format!("g{i}"),
                &hello(
                    projekt,
                    sitzung,
                    1000 + i,
                    3000 + i,
                    "active_probe",
                    Some(1),
                ),
            );
        }
        clock.setze_ms(STALE_NACH_MS + 1);
        c.liveness_tick();
        assert!(
            c.control_hello_registrieren(
                "global-neu",
                &hello(99, 99, 9999, 9998, "active_probe", Some(1)),
            )
            .angenommen
        );
        for i in 0..GLOBAL_CLIENT_CAP {
            let projekt = 10 + i / SESSION_CLIENT_CAP;
            let sitzung = 20 + i / SESSION_CLIENT_CAP;
            if !c
                .modell_sicht(&hex(projekt), &hex(sitzung))
                .clients
                .iter()
                .any(|client| client.adresse.instance_id == hex(1000 + i))
            {
                return hex(1000 + i);
            }
        }
        panic!("globales stale-Opfer fehlt")
    }

    let global_vorwaerts = (0..GLOBAL_CLIENT_CAP).collect::<Vec<_>>();
    let global_rueckwaerts = (0..GLOBAL_CLIENT_CAP).rev().collect::<Vec<_>>();
    assert_eq!(global_opfer(&global_vorwaerts), hex(1000));
    assert_eq!(global_opfer(&global_rueckwaerts), hex(1000));
}

#[test]
fn eviction_haelt_phase_a_riegel() {
    let (c, clock) = coordinator();
    let alt = hello(1, 2, 10, 100, "main", Some(9));
    let neu = hello(1, 2, 10, 101, "main", Some(9));
    anmelden(&c, "alt", &alt);
    anmelden(&c, "neu", &neu);
    assert!(abonnieren(&c, "neu", &neu.adresse));
    assert!(c.intervention_begin("neu", &neu.adresse, &hex(700), 1));
    assert!(!report(&c, "alt", &alt.adresse));
    assert!(!c.dispatch_fuer_link_erlaubt("alt"));
    assert!(!c.dispatch_fuer_link_erlaubt("neu"));
    assert!(c.session_push_ziele(&hex(2), &neu.adresse).is_empty());
    assert_eq!(c.subscription_anzahl(), 1);
    assert_eq!(c.interventionssicht().aktive, 1);

    clock.setze_ms(STALE_NACH_MS + 1);
    c.liveness_tick();
    clock.setze_ms(STALE_NACH_MS + 1 + TOMBSTONE_MS);
    c.liveness_tick();
    assert!(c.modell_sicht(&hex(1), &hex(2)).clients.is_empty());
    assert_eq!(c.subscription_anzahl(), 0);
    assert_eq!(c.interventionssicht().aktive, 0);
    assert!(!c.dispatch_fuer_link_erlaubt("alt"));
    assert!(c.interventionssicht().unknown);

    let rueckkehr = hello(1, 2, 10, 102, "main", Some(9));
    anmelden(&c, "rueckkehr", &rueckkehr);
    assert!(!c.dispatch_fuer_link_erlaubt("rueckkehr"));
    let neue_id = hello(1, 2, 11, 103, "main", Some(9));
    anmelden(&c, "neue-id", &neue_id);
    assert!(report(&c, "neue-id", &neue_id.adresse));
    assert!(abonnieren(&c, "neue-id", &neue_id.adresse));
    assert!(c.dispatch_fuer_link_erlaubt("neue-id"));
    assert_eq!(
        c.session_push_ziele(&hex(2), &neue_id.adresse),
        vec!["neue-id"]
    );
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
fn unverarbeitete_bytes_aendern_den_sessiongraphen_nicht() {
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

    let (c, _) = coordinator();
    for i in 0..LAST_SONDEN {
        let h = hello(1, 2, 1000 + i, 2000 + i, "active_probe", Some(77));
        anmelden(&c, &format!("sicht-{i}"), &h);
        assert!(report(&c, &format!("sicht-{i}"), &h.adresse));
        if i + 1 == SICHTBARE_SONDEN_NORMAL {
            let snapshot: Value =
                serde_json::from_slice(&c.session_snapshot_json(&hex(1), &hex(2))).unwrap();
            assert_eq!(snapshot["mitglieder"].as_array().unwrap().len(), 16);
        }
    }
    let snapshot: Value =
        serde_json::from_slice(&c.session_snapshot_json(&hex(1), &hex(2))).unwrap();
    assert_eq!(
        snapshot["mitglieder"].as_array().unwrap().len(),
        LAST_SONDEN
    );

    let gleich_host_a = hello(10, 20, 9000, 9100, "main", Some(4444));
    let gleich_host_b = hello(11, 21, 9000, 9100, "main", Some(4444));
    anmelden(&c, "gleich-host-a", &gleich_host_a);
    anmelden(&c, "gleich-host-b", &gleich_host_b);
    assert!(report(&c, "gleich-host-a", &gleich_host_a.adresse));
    assert!(report(&c, "gleich-host-b", &gleich_host_b.adresse));
    assert_eq!(c.modell_sicht(&hex(10), &hex(20)).clients.len(), 1);
    assert_eq!(c.modell_sicht(&hex(11), &hex(21)).clients.len(), 1);

    let identisch = hello(70, 71, 72, 73, "main", Some(4444));
    let vor = Coordinator::mit_uhr(Arc::new(ManualClock::default()), hex(0xaaaa));
    let nach = Coordinator::mit_uhr(Arc::new(ManualClock::default()), hex(0xbbbb));
    anmelden(&vor, "identisch", &identisch);
    anmelden(&nach, "identisch", &identisch);
    assert!(report(&vor, "identisch", &identisch.adresse));
    assert!(report(&nach, "identisch", &identisch.adresse));
    let vor_snapshot: Value = serde_json::from_slice(&vor.session_snapshot_json(
        &identisch.adresse.project_binding_id,
        &identisch.adresse.session_epoch,
    ))
    .unwrap();
    let nach_snapshot: Value = serde_json::from_slice(&nach.session_snapshot_json(
        &identisch.adresse.project_binding_id,
        &identisch.adresse.session_epoch,
    ))
    .unwrap();
    assert_eq!(
        vor_snapshot["session_epoch"],
        nach_snapshot["session_epoch"]
    );
    assert_eq!(vor_snapshot["mitglieder"], nach_snapshot["mitglieder"]);
    assert_ne!(vor_snapshot["broker_epoch"], nach_snapshot["broker_epoch"]);
}

// ── NAK-121 H-03 ───────────────────────────────────────────────────────────
//
// Fremde Senkenarbeit laeuft nie unter dem globalen Standlock. Die Senke greift
// im Push reentrant in den Coordinator zurueck; steht der Lock noch, kommt sie
// nicht durch. Der Rueckgriff laeuft in einem eigenen Thread MIT FRIST, damit
// ein gehaltener Lock den Test ROT macht statt den Lauf aufzuhaengen.

#[derive(Default)]
struct ReentranteSenke {
    ziel: std::sync::Mutex<Option<std::sync::Weak<Coordinator>>>,
    pushes: AtomicU64,
    reentrant_durchgekommen: AtomicU64,
    reentrant_blockiert: AtomicU64,
}

impl ReentranteSenke {
    fn binden(&self, c: &Arc<Coordinator>) {
        *self.ziel.lock().unwrap_or_else(|e| e.into_inner()) = Some(Arc::downgrade(c));
    }

    fn reentrant_versuchen(&self) {
        self.pushes.fetch_add(1, Ordering::SeqCst);
        let Some(schwach) = self
            .ziel
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .clone()
        else {
            return;
        };
        let Some(stark) = schwach.upgrade() else {
            return;
        };
        let (sender, empfang) = std::sync::mpsc::channel::<()>();
        std::thread::spawn(move || {
            // client_anzahl nimmt den Standlock. Haelt der Push ihn noch,
            // steht dieser Thread hier bis zum Prozessende.
            let _ = stark.client_anzahl();
            let _ = sender.send(());
        });
        if empfang
            .recv_timeout(std::time::Duration::from_secs(3))
            .is_ok()
        {
            self.reentrant_durchgekommen.fetch_add(1, Ordering::SeqCst);
        } else {
            self.reentrant_blockiert.fetch_add(1, Ordering::SeqCst);
        }
    }
}

impl eqcop_broker::coordinator::SessionPush for ReentranteSenke {
    fn snapshot_schreiben(&self, _link_id: &str, _payload: &[u8]) -> bool {
        self.reentrant_versuchen();
        true
    }

    fn messframe_schreiben(&self, _link_id: &str, _instance_id: &str, _payload: &[u8]) -> bool {
        self.reentrant_versuchen();
        true
    }
}

#[cfg(windows)]
#[test]
fn senke_haelt_den_standlock_nicht_ueber_den_push() {
    let clock = Arc::new(ManualClock::default());
    let c = Arc::new(Coordinator::mit_uhr(clock.clone(), hex(0xbeef)));
    let senke = Arc::new(ReentranteSenke::default());
    senke.binden(&c);
    c.session_push_setzen(senke.clone());

    // Erst eine Sonde mit einem echten Messframe: ohne ihn liefe die
    // Push-Schleife von messframes_an_subscriber_push leer, und der Test
    // wuerde seine Zusage gar nicht beruehren.
    let sonde = Adresse {
        logon_sid: "S-1-5-21-1111111111-2222222222-3333333333-1001".into(),
        project_binding_id: "1".repeat(32),
        session_epoch: "2".repeat(32),
        instance_id: format!("{:032x}", 3),
        runtime_nonce: "4".repeat(32),
    };
    c.control_registrieren("probe", sonde.clone());
    Senke::telemetrie_gekoppelt(c.as_ref(), "probe");
    let gueltig =
        include_bytes!("../../eq-copilot/fixtures/v3/flatbuffers/gueltig/live-64-band.bin");
    Senke::p2(c.as_ref(), "probe", gueltig);
    assert_eq!(c.p2_live_frames(), 1);

    // Jetzt abonniert ein zweiter Link dieselbe Session. Sein Subscribe loest
    // den Snapshot-Push UND den Messframe-Push mit nicht leerer Liste aus.
    let mut zweiter = sonde.clone();
    zweiter.instance_id = format!("{:032x}", 4);
    zweiter.runtime_nonce = "5".repeat(32);
    c.control_registrieren("b", zweiter.clone());
    let vor_subscribe = senke.pushes.load(Ordering::SeqCst);
    assert!(abonnieren(&c, "b", &zweiter));

    assert!(
        senke.pushes.load(Ordering::SeqCst) > vor_subscribe + 1,
        "es lief nur der Snapshot-Push; messframes_an_subscriber_push blieb leer"
    );
    assert_eq!(
        senke.reentrant_blockiert.load(Ordering::SeqCst),
        0,
        "die Senke lief unter dem Standlock: ein reentranter Zugriff kam nicht durch"
    );
    assert_eq!(
        senke.reentrant_durchgekommen.load(Ordering::SeqCst),
        senke.pushes.load(Ordering::SeqCst)
    );
}

/// Zweite H-03-Stelle: der P2-Pfad der Senke selbst. Er sammelte seine
/// Zielliste unter dem Standlock UND pushte darunter; die Zielliste ist
/// unveraendert eine Kopie, nur der Push liegt jetzt hinter dem Lock.
#[cfg(windows)]
#[test]
fn p2_push_haelt_den_standlock_nicht() {
    let clock = Arc::new(ManualClock::default());
    let c = Arc::new(Coordinator::mit_uhr(clock.clone(), hex(0xbeef)));
    let senke = Arc::new(ReentranteSenke::default());
    senke.binden(&c);
    c.session_push_setzen(senke.clone());

    let adresse = Adresse {
        logon_sid: "S-1-5-21-1111111111-2222222222-3333333333-1001".into(),
        project_binding_id: "1".repeat(32),
        session_epoch: "2".repeat(32),
        instance_id: format!("{:032x}", 3),
        runtime_nonce: "4".repeat(32),
    };
    c.control_registrieren("probe", adresse.clone());
    Senke::telemetrie_gekoppelt(c.as_ref(), "probe");
    assert!(abonnieren(&c, "probe", &adresse));
    let vorher = senke.pushes.load(Ordering::SeqCst);

    let gueltig =
        include_bytes!("../../eq-copilot/fixtures/v3/flatbuffers/gueltig/live-64-band.bin");
    Senke::p2(c.as_ref(), "probe", gueltig);
    assert_eq!(c.p2_live_frames(), 1);

    assert!(
        senke.pushes.load(Ordering::SeqCst) > vorher,
        "der P2-Pfad hat gar nicht gepusht - der Test misst sonst nichts"
    );
    assert_eq!(
        senke.reentrant_blockiert.load(Ordering::SeqCst),
        0,
        "der P2-Push lief unter dem Standlock"
    );
}

// ── NAK-121 H-04 und H-05 ──────────────────────────────────────────────────
//
// Eine einzelne Panik macht keinen dauerhaften Brokerausfall: jeder Sperrgriff
// auf den Coordinatorstand ist vergiftungstolerant, und ein panischer
// Senkenaufruf verlaesst den v2-Destruktor nicht.

#[test]
fn vergifteter_stand_beendet_den_prozess_nicht() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    let vorher = c.client_anzahl();
    assert_eq!(vorher, 1);

    // Den Standlock absichtlich vergiften: ein Thread panisiert, waehrend er
    // ihn haelt. Genau dieser Zustand liess den Broker vor NAK-121 an jedem
    // weiteren Zugriff sterben - 54 expect-Stellen gegen 14 tolerante.
    let c2 = Arc::new(c);
    let geliehen = c2.clone();
    let panik = std::thread::spawn(move || {
        // modell_sicht nimmt den Standlock; die Panik faellt darunter.
        let _sicht = geliehen.modell_sicht(&hex(1), &hex(2));
        panic!("absichtliche Panik unter dem Standlock");
    });
    assert!(panik.join().is_err(), "die Panik ist nicht eingetreten");

    // Der Lock ist jetzt vergiftet. Fachlich sichtbare Zustaende bleiben, was
    // sie waren; nichts wird stillschweigend zurueckgesetzt.
    assert_eq!(c2.client_anzahl(), vorher);
    assert_eq!(c2.modell_sicht(&hex(1), &hex(2)).clients.len(), 1);

    // Trennung und Hoermarkierung laufen weiter - die beiden Wege, die der
    // v2-Destruktor aus H-05 benutzt. Gemessen wird an ihrer Wirkung, nicht
    // daran, dass der Aufruf zurueckkehrt.
    c2.hoermarkierung_v2("a", true);
    assert_eq!(c2.interventionssicht().aktive, 1);
    c2.hoermarkierung_v2_getrennt("a");
    assert_eq!(c2.interventionssicht().aktive, 0);
    c2.control_ende("a");
    assert!(
        c2.modell_sicht(&hex(1), &hex(2)).clients[0].stale
            || c2.modell_sicht(&hex(1), &hex(2)).clients.is_empty(),
        "control_ende blieb auf dem vergifteten Lock wirkungslos"
    );
}

/// Quellwache zu H-04: nach der Umstellung ist auf dem Standlock des
/// Coordinators kein `expect` mehr uebrig. Sie misst die Zusage dauerhaft, auch
/// fuer Wege, die kein Test anfaehrt.
#[test]
fn kein_expect_mehr_auf_dem_standlock() {
    let mut gefunden: Vec<String> = Vec::new();
    let wurzel = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("src/coordinator");
    for eintrag in std::fs::read_dir(&wurzel).expect("coordinator-Modulordner") {
        let pfad = eintrag.expect("Verzeichniseintrag").path();
        if pfad.extension().and_then(|e| e.to_str()) != Some("rs") {
            continue;
        }
        let quelle = std::fs::read_to_string(&pfad).expect("Modulquelle lesbar");
        for (nr, zeile) in quelle.lines().enumerate() {
            if zeile.contains(".expect(\"Coordinator vergiftet\")") {
                gefunden.push(format!("{}:{}", pfad.display(), nr + 1));
            }
        }
    }
    assert!(
        gefunden.is_empty(),
        "expect auf dem Standlock uebrig: {gefunden:?}"
    );
}

// ── NAK-121 H-12 ───────────────────────────────────────────────────────────
//
// verbinden gegen trennen gilt auch fuer die Sessionmap. Vor NAK-121 wuchs sie
// unbegrenzt: eine Suche nach sessions.remove, .retain oder .clear traf im
// ganzen Coordinator nirgends.

#[test]
fn session_faellt_mit_dem_letzten_client() {
    let (c, clock) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    assert_eq!(c.modell_sicht(&hex(1), &hex(2)).clients.len(), 1);

    // Der Tombstone-Weg entfernt den letzten Client der Session. Mit ihm muss
    // die Session selbst fallen - sonst bliebe ein Eintrag ohne jeden Client
    // fuer die Lebensdauer des Brokers stehen.
    c.control_ende("a");
    clock.setze_ms(TOMBSTONE_MS);
    c.liveness_tick();
    assert!(c.modell_sicht(&hex(1), &hex(2)).clients.is_empty());
    assert_eq!(
        c.session_anzahl(),
        0,
        "die Session ueberlebte ihren letzten Client"
    );

    // Gegenprobe: solange ein Client lebt, bleibt seine Session.
    anmelden(&c, "b", &h);
    assert_eq!(c.session_anzahl(), 1);
}

#[test]
fn global_session_cap_weist_am_cap_plus_eins_ab() {
    let (c, _) = coordinator();
    // Bis zum Cap gehen alle durch - je Session ein eigener Client.
    for i in 0..GLOBAL_SESSION_CAP {
        let h = hello(1, 100 + i, 10 + i, 200 + i, "main", Some(9));
        let ausgang = c.control_hello_registrieren(&format!("l{i}"), &h);
        assert!(ausgang.angenommen, "am Cap abgewiesen bei {i}: {:?}", ausgang.grund);
    }
    assert_eq!(c.session_anzahl(), GLOBAL_SESSION_CAP);

    // Cap plus eins faellt fail-closed. Die Grenze greift, weil alle Clients
    // frisch sind: es gibt kein stales Opfer zum Verdraengen.
    let ueber = hello(1, 999, 999, 999, "main", Some(9));
    let ausgang = c.control_hello_registrieren("ueber", &ueber);
    assert!(!ausgang.angenommen);
    assert_eq!(c.session_anzahl(), GLOBAL_SESSION_CAP);
}

// ── NAK-121 H-10 ───────────────────────────────────────────────────────────

/// Ein Hello mit IDENTISCHER Nonce verdraengt den aelteren Link - und laesst
/// dabei den Aliaseintrag stehen.
///
/// C-10 sagt Verdraengung nur fuer die abweichende Nonce zu und schweigt zum
/// identischen Fall; durch diese Luecke konnten zwei lebende Links dieselbe
/// Adresse tragen. Die Ausnahme beim Alias ist keine Bequemlichkeit: bei
/// identischer Nonce ist der Aliasbesitzer beider Links derselbe Schluessel
/// (instance_id:runtime_nonce), und ihn zu entfernen zoege dem UEBERLEBENDEN
/// Link seine Wireadresse weg.
#[test]
fn hello_mit_identischer_nonce_verdraengt_den_alten_link_ohne_den_alias_zu_ziehen() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "alt", &h);
    assert!(abonnieren(&c, "alt", &h.adresse));
    assert_eq!(c.subscription_anzahl(), 1);

    // Dasselbe Hello, derselbe Nonce, neuer Link.
    anmelden(&c, "neu", &h);

    // Der alte Link ist verdraengt: er soll trennen und hat seine Subscription
    // verloren. Vorher lebten beide weiter und trugen dieselbe Adresse.
    assert!(c.verbindung_soll_trennen("alt"));
    assert!(!c.verbindung_soll_trennen("neu"));
    assert_eq!(
        c.subscription_anzahl(),
        0,
        "die Subscription des verdraengten Links blieb stehen"
    );

    // Und der ueberlebende Link hat seine Wireadresse behalten - haette die
    // Verdraengung den gemeinsamen Aliaseintrag gezogen, faende er sie nicht
    // mehr und jeder Dispatch fiele fail-closed.
    assert!(
        c.dispatch_fuer_link_erlaubt("neu"),
        "die Verdraengung hat dem ueberlebenden Link seine Wireadresse weggezogen"
    );
    assert!(abonnieren(&c, "neu", &h.adresse));
}

/// Das Ziel eines P0-Befehls wird ueber die Linkidentitaet aufgeloest, nicht
/// ueber die erste Adressuebereinstimmung in der Linkmap.
///
/// Gemessen mit permutierter Einfuegereihenfolge: haenge die Aufloesung an der
/// HashMap-Reihenfolge, lieferten die beiden Permutationen verschiedenen
/// Zustand - und das append-only Log waere nicht mehr reproduzierbar.
#[test]
fn p0_ziel_ist_bei_zwei_links_derselben_adresse_deterministisch() {
    fn lauf(reihenfolge: [&str; 3]) -> (bool, bool, usize) {
        let (c, _) = coordinator();
        let main = hello(1, 2, 10, 100, "main", Some(9));
        let sonde = hello(1, 2, 11, 200, "active_probe", Some(9));
        for link in reihenfolge {
            match link {
                "main" => anmelden(&c, "main", &main),
                "sonde-a" => anmelden(&c, "sonde-a", &sonde),
                // Zweiter Link derselben Sonde mit demselben Nonce: seit H-10
                // verdraengt er den ersten, statt neben ihm zu leben.
                "sonde-b" => anmelden(&c, "sonde-b", &sonde),
                _ => unreachable!(),
            }
        }
        (
            c.verbindung_soll_trennen("sonde-a"),
            c.verbindung_soll_trennen("sonde-b"),
            c.subscription_anzahl(),
        )
    }

    // Beide Permutationen der beiden Sondenlinks muessen denselben Zustand
    // ergeben: der zuletzt angemeldete lebt, der andere ist verdraengt.
    let vorwaerts = lauf(["main", "sonde-a", "sonde-b"]);
    assert_eq!(vorwaerts, (true, false, 0));

    // Und mit vertauschter Reihenfolge derselben drei Anmeldungen gilt
    // dasselbe, nur spiegelverkehrt - nichts haengt an der Iterationsordnung.
    let (c, _) = coordinator();
    let main = hello(1, 2, 10, 100, "main", Some(9));
    let sonde = hello(1, 2, 11, 200, "active_probe", Some(9));
    anmelden(&c, "main", &main);
    anmelden(&c, "sonde-b", &sonde);
    anmelden(&c, "sonde-a", &sonde);
    assert!(c.verbindung_soll_trennen("sonde-b"));
    assert!(!c.verbindung_soll_trennen("sonde-a"));
}
