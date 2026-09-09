//! Lebenszyklus: starten, stoppen, ernten — und nie in sich selbst joinen.
//!
//! Der Acceptor ueberlebt die Verbindungsgrenze, beendete Verbindungen
//! werden geerntet, und `stoppen` endet auch dann, wenn eine Senke oder ein
//! Lebenszyklus-Callback gerade haengt. Jeder Join hat eine Frist; ein
//! Verbraucherthread wird nach `SENKE_FRIST` abgeloest statt gejoint.
//!
//! ⚠️ **Das Stopp-Fenster ist ein Test, kein Zufall.** `stop_im_fenster_vor_der_bedienung_haengt_nicht`
//! misst genau die Lage, in der der Stop beginnt, waehrend eine Verbindung
//! schon angenommen, aber noch nicht bedient ist — und
//! `kopplung_faellt_mit_dem_leserende_nicht_erst_nach_den_joins` misst, dass
//! die Entkopplung VOR den Joins faellt: nichts bleibt registriert, waehrend
//! auf einen Verbraucher gewartet wird.

use super::listener::v3_server_starten_intern;
use super::tests_hilfe::*;
use super::*;

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
