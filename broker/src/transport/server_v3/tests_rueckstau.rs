//! Rueckstau und Prioritaetsklassen: der Leser haengt nie hinter der Senke.
//!
//! §53.9 verlangt woertlich: „Antworten gehen ueber getrennte bounded
//! Writerqueues zurueck; ein blockierender Pipe-Write haelt weder Coordinator
//! noch Storelock." Die erste Fassung des Listeners leerte den Ingress nach
//! jedem Frame und schrieb die Antwort im Leserthread — die Ingressqueue
//! konnte nie ueber Groesse 1 wachsen, und die Cap-256-, P2-Drop- und
//! P0-Ueberlaufpfade waren im echten Listener unerreichbar (T2-Befund 3 vom
//! 2026-08-29).
//!
//! Diese Faelle messen das Gegenteil: eine Senke, die haelt, blockiert den
//! Leser nicht; ein voller P0-Eingang TRENNT die Verbindung, statt still
//! Akzeptiertes zu verwerfen; P0 wird beantwortet, waehrend P1 dieselbe Senke
//! blockiert; und nach dem Schliessen des Eingangs kommt nichts mehr.

use super::tests_hilfe::*;
use super::*;

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
