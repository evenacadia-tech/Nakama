//! Kopplung und Trennreihenfolge — die Zeilen der Verhaltensmatrix.
//!
//! Die Reihenfolge beim Verbinden ist festgelegt und wird hier gemessen:
//! anmelden, Callback „verbunden" abschliessen, erst dann Welcome nach
//! aussen. Beim Trennen dieselbe Regel rueckwaerts — zuerst entkoppeln, dann
//! Fristen und Joins.
//!
//! ⚠️ **Jeder Callback genau einmal, auch wenn eine Seite abgeloest wird.**
//! Die vier `abgeloest…`-Faelle messen die Lage, in der ein Telemetrie-Callback
//! haengt: das darf `control_getrennt` weder aufhalten noch verschlucken. Die
//! Senke `ReihenfolgeSenke` protokolliert dafuer die Callbacks in ihrer
//! Reihenfolge und kann einzelne von ihnen gezielt blockieren.

use super::tests_hilfe::*;
use super::*;

//== Ursachenrunde 2026-08-30: die Zeilen der Verhaltensmatrix ============

/// Protokolliert die Lebenszyklus-Callbacks in ihrer Reihenfolge und kann
/// in genau einem von ihnen blockieren. Ohne Protokoll misst ein Test
/// ueber Reihenfolge nur seine eigene Hoffnung.
#[derive(Default)]
struct ReihenfolgeSenke {
    zaehl: ZaehlSenke,
    log: Mutex<Vec<String>>,
    /// Was BETRETEN wurde — vor dem Blockieren. `log` traegt erst den
    /// Austritt; ohne diese zweite Spur koennte ein Test nicht abwarten,
    /// dass die Gegenseite wirklich IN ihrem Callback steht, und muesste
    /// die Taktung raten.
    betreten: Mutex<Vec<String>>,
    blockiert_in: Mutex<String>,
    blockdauer_ms: AtomicU64,
}

impl ReihenfolgeSenke {
    fn notieren(&self, was: &str) {
        self.betreten
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .push(was.to_string());
        let blockieren = {
            let b = self.blockiert_in.lock().unwrap_or_else(|e| e.into_inner());
            *b == was
        };
        if blockieren {
            let ms = self.blockdauer_ms.load(Ordering::SeqCst);
            std::thread::sleep(Duration::from_millis(ms));
        }
        self.log
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .push(was.to_string());
    }
    fn eintraege(&self) -> Vec<String> {
        self.log.lock().unwrap_or_else(|e| e.into_inner()).clone()
    }
    fn anzahl(&self, was: &str) -> usize {
        self.eintraege()
            .iter()
            .filter(|x| x.as_str() == was)
            .count()
    }
    fn stelle(&self, was: &str) -> Option<usize> {
        self.eintraege().iter().position(|x| x.as_str() == was)
    }
    fn betreten_anzahl(&self, was: &str) -> usize {
        self.betreten
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .iter()
            .filter(|x| x.as_str() == was)
            .count()
    }
}

impl Senke for ReihenfolgeSenke {
    fn control_verbunden(&self, l: &str, h: &HelloControl) -> ControlAnmeldung {
        let ausgang = self.zaehl.control_verbunden(l, h);
        self.notieren("control_verbunden");
        ausgang
    }
    fn control_getrennt(&self, l: &str) {
        self.zaehl.control_getrennt(l);
        self.notieren("control_getrennt");
    }
    fn telemetrie_gekoppelt(&self, l: &str) {
        self.zaehl.telemetrie_gekoppelt(l);
        self.notieren("telemetrie_gekoppelt");
    }
    fn telemetrie_getrennt(&self, l: &str) {
        self.zaehl.telemetrie_getrennt(l);
        self.notieren("telemetrie_getrennt");
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

/// Matrix `C-LS-02`/`C-LS-04` (Regel 5): `control_verbunden` ist
/// ABGESCHLOSSEN, bevor das Welcome den Draht verlaesst — sonst kann
/// `telemetrie_gekoppelt` auf dem anderen Verbindungsthread vorlaufen.
#[test]
fn welcome_folgt_dem_abgeschlossenen_control_verbunden() {
    let pipe = probe_pipe("cvvorwelcome");
    let senke = Arc::new(ReihenfolgeSenke::default());
    *senke.blockiert_in.lock().unwrap() = "control_verbunden".into();
    senke.blockdauer_ms.store(400, Ordering::SeqCst);
    let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

    let steuer = Testclient::neu(&pipe).unwrap();
    let t0 = Instant::now();
    assert!(steuer.schreiben(&control_hello(&"c".repeat(32))));
    let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
    let dauer = t0.elapsed();
    assert!(
        dauer >= Duration::from_millis(350),
        "das Welcome kam schon nach {dauer:?} — also VOR dem abgeschlossenen \
         control_verbunden (die Senke haelt es 400 ms)"
    );
    assert_eq!(
        senke.eintraege().first().map(String::as_str),
        Some("control_verbunden"),
        "erster Lebenszyklusaufruf muss control_verbunden sein"
    );

    let tele = Testclient::neu(&pipe).unwrap();
    assert!(tele.schreiben(&telemetry_hello(&"c".repeat(32), &link, &challenge)));
    assert!(welcome_lesen(&tele).is_some());
    assert!(warte_auf(4000, || senke.anzahl("telemetrie_gekoppelt") == 1));
    assert!(
        senke.stelle("control_verbunden") < senke.stelle("telemetrie_gekoppelt"),
        "telemetrie_gekoppelt lief vor control_verbunden: {:?}",
        senke.eintraege()
    );
    drop(steuer);
    drop(tele);
    drop(griff);
}

/// Matrix `C-LS-02`/`C-LS-04`: je `link_id` genau ein
/// `control_verbunden` und hoechstens ein `telemetrie_gekoppelt`. Ein
/// zweites Telemetry-Hello auf dieselbe Kopplung wird abgewiesen, OHNE den
/// Callback erneut auszuloesen.
#[test]
fn connect_callbacks_je_kopplung_genau_einmal() {
    let pipe = probe_pipe("connecteinmal");
    let senke = Arc::new(ReihenfolgeSenke::default());
    let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

    let steuer = Testclient::neu(&pipe).unwrap();
    assert!(steuer.schreiben(&control_hello(&"d".repeat(32))));
    let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
    let tele = Testclient::neu(&pipe).unwrap();
    assert!(tele.schreiben(&telemetry_hello(&"d".repeat(32), &link, &challenge)));
    assert!(welcome_lesen(&tele).is_some());
    assert!(warte_auf(4000, || senke.anzahl("telemetrie_gekoppelt") == 1));

    // Zweite Telemetrieverbindung auf dieselbe link_id: abgewiesen.
    let zweite = Testclient::neu(&pipe).unwrap();
    assert!(zweite.schreiben(&telemetry_hello(&"d".repeat(32), &link, &challenge)));
    assert!(warte_auf(4000, || griff
        .statistik
        .geschlossen_bootstrap
        .load(Ordering::SeqCst)
        > 0));
    std::thread::sleep(Duration::from_millis(200));
    assert_eq!(senke.anzahl("control_verbunden"), 1);
    assert_eq!(
        senke.anzahl("telemetrie_gekoppelt"),
        1,
        "die abgewiesene zweite Verbindung darf den Callback nicht erneut ausloesen: {:?}",
        senke.eintraege()
    );
    drop(steuer);
    drop(tele);
    drop(zweite);
    drop(griff);
}

/// Matrix `C-LS-06` (Regel 5), Normalfall: Kopplung loesen →
/// `telemetrie_getrennt` → `control_getrennt`, je genau einmal.
#[test]
fn trennreihenfolge_je_callback_genau_einmal() {
    let pipe = probe_pipe("trennreihe");
    let senke = Arc::new(ReihenfolgeSenke::default());
    let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

    let steuer = Testclient::neu(&pipe).unwrap();
    assert!(steuer.schreiben(&control_hello(&"e".repeat(32))));
    let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
    let tele = Testclient::neu(&pipe).unwrap();
    assert!(tele.schreiben(&telemetry_hello(&"e".repeat(32), &link, &challenge)));
    assert!(welcome_lesen(&tele).is_some());
    assert!(warte_auf(4000, || senke.anzahl("telemetrie_gekoppelt") == 1));

    drop(steuer); // Control-Ende reisst die Telemetrie mit
    assert!(
        warte_auf(8000, || senke.anzahl("control_getrennt") == 1),
        "control_getrennt fehlt: {:?}",
        senke.eintraege()
    );
    std::thread::sleep(Duration::from_millis(300));
    let log = senke.eintraege();
    assert_eq!(senke.anzahl("telemetrie_getrennt"), 1, "{log:?}");
    assert_eq!(senke.anzahl("control_getrennt"), 1, "{log:?}");
    assert!(
        senke.stelle("telemetrie_getrennt") < senke.stelle("control_getrennt"),
        "control_getrennt kam vor telemetrie_getrennt: {log:?}"
    );
    drop(tele);
    drop(griff);
}

/// Matrix `C-LS-06`, Fristfall (Nachtrag der Wiederpruefung): ein
/// abgeloestes `telemetrie_getrennt` haelt `control_getrennt` NICHT auf.
/// Ein abgeloester Trenn-Callback zaehlt als gelaufen; es entfaellt keiner
/// und keiner laeuft doppelt.
#[test]
fn abgeloestes_telemetrie_getrennt_haelt_control_getrennt_nicht_auf() {
    let pipe = probe_pipe("trennfrist");
    let senke = Arc::new(ReihenfolgeSenke::default());
    *senke.blockiert_in.lock().unwrap() = "telemetrie_getrennt".into();
    senke
        .blockdauer_ms
        .store(SENKE_FRIST.as_millis() as u64 * 2, Ordering::SeqCst);
    let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

    let steuer = Testclient::neu(&pipe).unwrap();
    assert!(steuer.schreiben(&control_hello(&"f".repeat(32))));
    let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
    let tele = Testclient::neu(&pipe).unwrap();
    assert!(tele.schreiben(&telemetry_hello(&"f".repeat(32), &link, &challenge)));
    assert!(welcome_lesen(&tele).is_some());
    assert!(warte_auf(4000, || senke.anzahl("telemetrie_gekoppelt") == 1));

    let beginn = Instant::now();
    drop(steuer);
    assert!(
        warte_auf(SENKE_FRIST.as_millis() as u64 * 4, || senke
            .anzahl("control_getrennt")
            == 1),
        "control_getrennt blieb aus, obwohl nur telemetrie_getrennt haengt: {:?}",
        senke.eintraege()
    );
    let dauer = beginn.elapsed();
    assert!(
        dauer < SENKE_FRIST * 3,
        "control_getrennt kam erst nach {dauer:?} — es haengt an der vollen \
         Blockdauer statt an SENKE_FRIST ({SENKE_FRIST:?})"
    );
    assert!(
        warte_auf(SENKE_FRIST.as_millis() as u64 * 3, || griff
            .statistik
            .lebenszyklus_abgeloest
            .load(Ordering::SeqCst)
            >= 1),
        "der Fristfall muss als lebenszyklus_abgeloest sichtbar sein"
    );
    assert_eq!(senke.anzahl("control_getrennt"), 1, "nie doppelt");
    drop(tele);
    drop(griff);
}

/// Matrix `C-LS-06` (NAK-104): die TELEMETRIE endet ZUERST, ihr
/// Trenn-Callback laeuft noch fristgerecht — und trotzdem meldet die
/// Control-Seite ihr `control_getrennt` erst danach.
///
/// Alle vorhandenen Proben beenden Control zuerst. Genau deshalb fiel nicht
/// auf, dass der Wartepunkt am Kopplungsregister haengt: die
/// Telemetrieverbindung nimmt ihren Eintrag schon VOR den Joins heraus,
/// also bevor sie ihr `telemetrie_getrennt` ueberhaupt meldet. Die
/// Control-Seite sah danach "keine Telemetrie" und uebersprang den
/// Wartepunkt.
#[test]
fn telemetrie_endet_zuerst_control_getrennt_folgt_trotzdem() {
    let pipe = probe_pipe("trenntelezuerst");
    let senke = Arc::new(ReihenfolgeSenke::default());
    *senke.blockiert_in.lock().unwrap() = "telemetrie_getrennt".into();
    // KUERZER als SENKE_FRIST: das hier ist der fristgerechte Fall, nicht
    // der Abloesefall (`abgeloestes_telemetrie_getrennt_...`).
    senke.blockdauer_ms.store(800, Ordering::SeqCst);
    assert!(
        Duration::from_millis(800) < SENKE_FRIST,
        "die Blockdauer muss unter SENKE_FRIST liegen, sonst misst der Test den Fristfall"
    );
    let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

    let steuer = Testclient::neu(&pipe).unwrap();
    assert!(steuer.schreiben(&control_hello(&"1".repeat(32))));
    let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
    let tele = Testclient::neu(&pipe).unwrap();
    assert!(tele.schreiben(&telemetry_hello(&"1".repeat(32), &link, &challenge)));
    assert!(welcome_lesen(&tele).is_some());
    assert!(warte_auf(4000, || senke.anzahl("telemetrie_gekoppelt") == 1));

    // 1) Die Telemetrie geht zuerst und steht danach in ihrem
    //    Trenn-Callback. Die Vorbedingung wird gemessen, nicht gehofft.
    drop(tele);
    assert!(
        warte_auf(4000, || senke.betreten_anzahl("telemetrie_getrennt") == 1),
        "die Telemetrieseite muss wirklich in ihrem Trenn-Callback stehen: {:?}",
        senke.eintraege()
    );

    // 2) Erst JETZT endet Control — waehrend drueben der Callback laeuft.
    drop(steuer);
    assert!(
        warte_auf(8000, || senke.anzahl("control_getrennt") == 1),
        "control_getrennt fehlt: {:?}",
        senke.eintraege()
    );
    std::thread::sleep(Duration::from_millis(300));
    let log = senke.eintraege();
    // Erst die REIHENFOLGE, dann die Zahlen — und ausgepackt verglichen:
    // `Option::cmp` haelt `None` fuer kleiner als jedes `Some`, ein ganz
    // fehlendes `telemetrie_getrennt` saehe im blossen Vergleich also wie
    // die richtige Reihenfolge aus.
    let stelle_tele = senke.stelle("telemetrie_getrennt");
    let stelle_ctrl = senke.stelle("control_getrennt");
    assert!(
        matches!((stelle_tele, stelle_ctrl), (Some(t), Some(c)) if t < c),
        "control_getrennt lief vor telemetrie_getrennt (oder telemetrie_getrennt \
         fehlt ganz), obwohl die Telemetrie ihre Frist hielt: {log:?}"
    );
    assert_eq!(senke.anzahl("telemetrie_getrennt"), 1, "{log:?}");
    assert_eq!(senke.anzahl("control_getrennt"), 1, "{log:?}");
    assert_eq!(
        griff
            .statistik
            .lebenszyklus_reihenfolge_verletzt
            .load(Ordering::SeqCst),
        0,
        "der fristgerechte Fall darf keine Reihenfolgeverletzung zaehlen"
    );
    drop(griff);
}

/// Matrix `C-LS-06` (NAK-104 Runde 2): das FENSTER des Kopplungs-Callbacks.
///
/// Die Telemetrie ist gekoppelt und ihr Welcome ist auf dem Draht, aber
/// `telemetrie_gekoppelt` steht noch bei der Senke. Enden beide Clients
/// genau jetzt, muss die Reihenfolge trotzdem halten. Vorher wurde die
/// Trennpflicht erst NACH Rueckkehr des Callbacks gesetzt: die
/// Control-Seite fand `erwartet == false`, uebersprang den Wartepunkt und
/// meldete `control_getrennt` VOR dem `telemetrie_getrennt`, das kurz
/// danach folgte.
///
/// Der Vorgaengertest `telemetrie_endet_zuerst_...` blockiert erst im
/// TRENN-Callback und wartet bis `telemetrie_gekoppelt` durch ist — er
/// betritt dieses Fenster nie.
#[test]
fn trennreihenfolge_haelt_im_kopplungs_callback_fenster() {
    let pipe = probe_pipe("trennkoppelfenster");
    let senke = Arc::new(ReihenfolgeSenke::default());
    *senke.blockiert_in.lock().unwrap() = "telemetrie_gekoppelt".into();
    // FRISTGERECHT: unter `SENKE_FRIST`, sonst misst der Test den
    // Abloesefall statt des Fensters.
    senke.blockdauer_ms.store(800, Ordering::SeqCst);
    assert!(
        Duration::from_millis(800) < SENKE_FRIST,
        "die Blockdauer muss unter SENKE_FRIST liegen, sonst misst der Test den Fristfall"
    );
    let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

    let steuer = Testclient::neu(&pipe).unwrap();
    assert!(steuer.schreiben(&control_hello(&"2".repeat(32))));
    let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
    let tele = Testclient::neu(&pipe).unwrap();
    assert!(tele.schreiben(&telemetry_hello(&"2".repeat(32), &link, &challenge)));
    // Das Welcome der Telemetrie geht VOR `telemetrie_gekoppelt` raus —
    // der Client ist also schon bedient, waehrend die Senke noch haelt.
    assert!(welcome_lesen(&tele).is_some());

    // Vorbedingung messen, nicht hoffen: die Senke steht IM
    // Kopplungs-Callback (`betreten` traegt den Eintritt, `log` erst den
    // Austritt).
    assert!(
        warte_auf(4000, || senke.betreten_anzahl("telemetrie_gekoppelt") == 1),
        "die Senke muss wirklich in telemetrie_gekoppelt stehen: {:?}",
        senke.eintraege()
    );
    assert_eq!(
        senke.anzahl("telemetrie_gekoppelt"),
        0,
        "der Callback darf hier noch NICHT zurueck sein — sonst ist das Fenster \
         schon vorbei und der Test misst nichts"
    );

    // Beide Clients enden IM Fenster: erst die Telemetrie, dann Control.
    drop(tele);
    drop(steuer);
    let beginn = Instant::now();
    assert!(
        warte_auf(8000, || senke.anzahl("control_getrennt") == 1),
        "control_getrennt fehlt: {:?}",
        senke.eintraege()
    );
    let dauer = beginn.elapsed();
    std::thread::sleep(Duration::from_millis(300));
    let log = senke.eintraege();
    // Ausgepackt vergleichen: `Option::cmp` haelt `None` fuer kleiner als
    // jedes `Some`, ein ganz fehlendes `telemetrie_getrennt` saehe sonst
    // wie die richtige Reihenfolge aus.
    let stelle_tele = senke.stelle("telemetrie_getrennt");
    let stelle_ctrl = senke.stelle("control_getrennt");
    assert!(
        matches!((stelle_tele, stelle_ctrl), (Some(t), Some(c)) if t < c),
        "control_getrennt lief vor telemetrie_getrennt (oder telemetrie_getrennt \
         fehlt ganz), obwohl beide Clients im Kopplungs-Callback endeten: {log:?}"
    );
    assert_eq!(senke.anzahl("telemetrie_getrennt"), 1, "{log:?}");
    assert_eq!(senke.anzahl("control_getrennt"), 1, "{log:?}");
    // Das `telemetrie_getrennt` kann fruehestens nach dem 800-ms-Block
    // laufen. Kommt `control_getrennt` schon vorher, hat die Control-Seite
    // den Wartepunkt uebersprungen — genau der Defekt.
    assert!(
        dauer >= Duration::from_millis(500),
        "control_getrennt kam schon nach {dauer:?} — die Control-Seite hat den \
         Wartepunkt uebersprungen, statt das telemetrie_getrennt nach dem \
         800-ms-Block abzuwarten: {log:?}"
    );
    assert_eq!(
        griff
            .statistik
            .lebenszyklus_reihenfolge_verletzt
            .load(Ordering::SeqCst),
        0,
        "der fristgerechte Fall darf keine Reihenfolgeverletzung zaehlen"
    );
    drop(griff);
}

/// Gegenstueck zum Fenster (`C-LS-04`/`C-LS-06`, NAK-104 Runde 2): wird das
/// `telemetrie_gekoppelt` ABGELOEST, kommt kein `telemetrie_getrennt` — die
/// vorgezogene Trennpflicht muss dann zurueckfallen. Sonst haengt die
/// Control-Seite die volle `SENKE_FRIST` und zaehlt eine
/// Reihenfolgeverletzung, die keine ist.
#[test]
fn abgeloestes_telemetrie_gekoppelt_laesst_control_getrennt_nicht_warten() {
    let pipe = probe_pipe("koppelabloese");
    let senke = Arc::new(ReihenfolgeSenke::default());
    *senke.blockiert_in.lock().unwrap() = "telemetrie_gekoppelt".into();
    senke
        .blockdauer_ms
        .store(SENKE_FRIST.as_millis() as u64 * 2, Ordering::SeqCst);
    let griff = v3_server_starten(&pipe, senke.clone(), "test".into()).unwrap();

    let steuer = Testclient::neu(&pipe).unwrap();
    assert!(steuer.schreiben(&control_hello(&"3".repeat(32))));
    let (link, challenge) = welcome_lesen(&steuer).expect("welcome");
    let tele = Testclient::neu(&pipe).unwrap();
    assert!(tele.schreiben(&telemetry_hello(&"3".repeat(32), &link, &challenge)));
    assert!(welcome_lesen(&tele).is_some());
    assert!(
        warte_auf(SENKE_FRIST.as_millis() as u64 * 3, || griff
            .statistik
            .lebenszyklus_abgeloest
            .load(Ordering::SeqCst)
            >= 1),
        "das telemetrie_gekoppelt muss wirklich abgeloest worden sein"
    );

    let beginn = Instant::now();
    drop(steuer);
    assert!(
        warte_auf(SENKE_FRIST.as_millis() as u64 * 4, || senke
            .anzahl("control_getrennt")
            == 1),
        "control_getrennt fehlt: {:?}",
        senke.eintraege()
    );
    let dauer = beginn.elapsed();
    assert!(
        dauer < SENKE_FRIST / 2,
        "control_getrennt kam erst nach {dauer:?} — es wartet auf ein \
         telemetrie_getrennt, das nach der Abloesung nie kommt (SENKE_FRIST \
         {SENKE_FRIST:?})"
    );
    assert_eq!(
        griff
            .statistik
            .lebenszyklus_reihenfolge_verletzt
            .load(Ordering::SeqCst),
        0,
        "ein abgesagtes Gegenstueck ist keine Reihenfolgeverletzung"
    );
    drop(tele);
    drop(griff);
}
