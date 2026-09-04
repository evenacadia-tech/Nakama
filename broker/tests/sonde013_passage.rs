//! Vergleichbarkeit zweier Passagen (SONDE-013 M-28, M-29, M-30; §32.4, §43.2).
//!
//! Der Satz, den dieses Bein misst, steht in M-28:
//!
//!     "Vergleichbarkeit entsteht aus FUENF Belegen, nicht aus einem. KEIN
//!      einzelner Hash hebt einen Widerspruch der anderen Belege auf."
//!
//! Das ist keine Formulierungsfrage. Ein Fingerprint ist eine 76-Byte-
//! Zusammenfassung von Sekunden Audio und bewusst nicht injektiv (§10.6);
//! zwei verschiedene Passagen koennen ihn teilen. Wer aus einer
//! Fingerprintgleichheit auf Vergleichbarkeit schliesst, mittelt zwei Stellen
//! der Musik zu einer Aussage — genau der Fehler, vor dem §15 warnt.
//!
//! ## Warum je Beleg ein Gegenbeispiel
//!
//! M-28 verlangt es woertlich: "mit einem Gegenbeispiel je Beleg". Ein Test,
//! der nur den Gutfall und einen Schlechtfall faehrt, koennte gruen sein,
//! weil ein einziger Beleg alles entscheidet — und genau das soll er
//! ausschliessen. Jeder der fuenf faellt hier ALLEIN, bei sonst perfekter
//! Lage.

use eqcop_broker::coordinator::vergleichbarkeit::{
    beurteile, Herabstufungsgrund, Passagenbeleg, Vergleichbarkeit, GATE_MATERIAL_COSINE,
    GATE_QUELLEN_JACCARD, GATE_ZEITUEBERDECKUNG,
};
use eqcop_broker::coordinator::vergleichbarkeit;
use eqcop_broker::telemetrie::Fingerprintwerte;

/// Ein Fingerprint mit einem erkennbaren Muster — nicht konstant, weil ein
/// konstanter Vektor zu jedem anderen konstanten die Cosinus-Aehnlichkeit 1
/// haette und der Test damit nichts von der Rechnung sehen wuerde.
fn fp(saat: u8) -> Fingerprintwerte {
    let mut f = Fingerprintwerte::default();
    for i in 0..32 {
        f.band_energie[i] = saat.wrapping_add((i * 7) as u8);
        f.onset[i] = saat.wrapping_add((i * 3) as u8);
    }
    for i in 0..12 {
        f.chroma[i] = saat.wrapping_add((i * 11) as u8);
    }
    f
}

/// Eine Lage, in der alle fuenf Belege perfekt sind — der einzige
/// Ausgangspunkt, von dem aus `Stark` erreichbar ist.
fn perfekt() -> (Passagenbeleg, Passagenbeleg) {
    let bau = |f: Fingerprintwerte| Passagenbeleg {
        projekt_start: 0,
        projekt_ende: 480_000,
        fingerprint: Some(f),
        aktive_quellen: vec!["a".repeat(32), "b".repeat(32)],
        samplerate: 48_000.0,
        messpunktklassen: vec!["insert".into(), "post".into()],
        abdeckung: 0.95,
    };
    (bau(fp(40)), bau(fp(40)))
}

#[test]
fn comparability_needs_all_five_and_hash_alone_never_wins() {
    // Die Gegenprobe zuerst: ohne sie sagt kein Fall darunter etwas.
    let (a, b) = perfekt();
    let gut = beurteile(&a, &b);
    assert_eq!(
        gut.klasse,
        Vergleichbarkeit::Stark,
        "eine perfekte Lage muss stark sein, sonst misst der Rest nichts: {:?}",
        gut
    );
    assert!(gut.gruende.is_empty(), "und ohne Grund: {:?}", gut.gruende);

    // ── Jeder der fuenf Belege faellt ALLEIN ────────────────────────────
    //
    // Bei sonst perfekter Lage, IDENTISCHEM Fingerprint und damit maximaler
    // Materialaehnlichkeit. Genau das ist die Zusage: der Hash rettet nichts.
    //
    // ⚠️ Die Proben fragen `> 0.99` und nicht `== 1.0`, und das ist kein
    // schlampiges Epsilon: der Cosinus zweier identischer Fingerprints landet
    // numerisch knapp UNTER eins. Wer hier eine exakte Eins erwartet — auch in
    // einem Rotbeweis, der einen Fehler an `material < 1.0` haengt —, baut
    // einen Fall, der gar nicht erst eintritt (siehe
    // `docs/beweise/roh/SONDE-013-rot-M-28.txt`).

    // (1) Projektbereich: dieselbe Laenge, andere Stelle.
    let (a1, mut b1) = perfekt();
    b1.projekt_start = 960_000;
    b1.projekt_ende = 1_440_000;
    let u1 = beurteile(&a1, &b1);
    assert_eq!(u1.klasse, Vergleichbarkeit::Unvergleichbar);
    assert!(u1.gruende.contains(&Herabstufungsgrund::ProjektbereichVerschieden));
    assert!(
        u1.material_cosine > 0.99,
        "der Fingerprint ist dabei IDENTISCH ({}) - und hebt den Widerspruch trotzdem nicht auf",
        u1.material_cosine
    );

    // (2) Material.
    let (a2, mut b2) = perfekt();
    b2.fingerprint = Some(fp(200));
    let u2 = beurteile(&a2, &b2);
    assert_ne!(u2.klasse, Vergleichbarkeit::Stark);
    assert!(u2.gruende.contains(&Herabstufungsgrund::MaterialVerschieden));

    // (3) Quellen: disjunkte Sets.
    let (a3, mut b3) = perfekt();
    b3.aktive_quellen = vec!["c".repeat(32), "d".repeat(32)];
    let u3 = beurteile(&a3, &b3);
    assert_eq!(u3.klasse, Vergleichbarkeit::Unvergleichbar);
    assert!(u3.gruende.contains(&Herabstufungsgrund::QuellenVerschieden));
    assert!(u3.material_cosine > 0.99, "wieder mit identischem Fingerprint");

    // (4) Messpunkt: andere Samplerate.
    let (a4, mut b4) = perfekt();
    b4.samplerate = 44_100.0;
    let u4 = beurteile(&a4, &b4);
    assert_eq!(u4.klasse, Vergleichbarkeit::Unvergleichbar);
    assert!(u4.gruende.contains(&Herabstufungsgrund::MesspunktVerschieden));

    // (4b) Messpunkt: andere Klassenmenge bei gleicher Rate.
    let (a4b, mut b4b) = perfekt();
    b4b.messpunktklassen = vec!["insert".into(), "pre".into()];
    let u4b = beurteile(&a4b, &b4b);
    assert_eq!(u4b.klasse, Vergleichbarkeit::Unvergleichbar);
    assert!(u4b.gruende.contains(&Herabstufungsgrund::MesspunktVerschieden));

    // (5) Abdeckung.
    let (a5, mut b5) = perfekt();
    b5.abdeckung = 0.2;
    let u5 = beurteile(&a5, &b5);
    assert_eq!(u5.klasse, Vergleichbarkeit::Unvergleichbar);
    assert!(u5.gruende.contains(&Herabstufungsgrund::AbdeckungZuGering));
    assert!(u5.material_cosine > 0.99);
}

#[test]
fn each_start_gate_falls_alone_at_its_threshold() {
    // Muster C-09: je Gate der Gleichheitsfall UND das erste Unterschreiten.
    // Ein Test, der nur weit darunter prueft, findet eine um eins
    // verschobene Grenze nicht.

    // ── Zeitueberdeckung ────────────────────────────────────────────────
    //
    // Die Ueberdeckung ist gemeinsamer Bereich / kuerzeres Fenster. Bei
    // gleich langen Fenstern verschiebt ein Versatz von n Samples sie um
    // n / laenge.
    let laenge = 480_000i64;
    for (versatz, soll_stark) in [(24_000i64, true), (24_001i64, false)] {
        let (a, mut b) = perfekt();
        b.projekt_start = versatz;
        b.projekt_ende = versatz + laenge;
        let u = beurteile(&a, &b);
        let erwartet = 1.0 - (versatz as f64 / laenge as f64);
        assert!(
            (u.zeitueberdeckung - erwartet).abs() < 1e-9,
            "die Ueberdeckung ist gemeinsamer Bereich durch kuerzeres Fenster: {} gegen {}",
            u.zeitueberdeckung,
            erwartet
        );
        assert_eq!(
            u.klasse == Vergleichbarkeit::Stark,
            soll_stark,
            "Versatz {versatz}: Ueberdeckung {} gegen Gate {}",
            u.zeitueberdeckung,
            GATE_ZEITUEBERDECKUNG
        );
    }

    // ── Quellen-Jaccard ─────────────────────────────────────────────────
    //
    // Zwei Sets mit n gemeinsamen von n+1 Elementen haben Jaccard
    // n / (n+1). Fuer 0,9 braucht es 9 von 10.
    {
        let gemeinsam: Vec<String> = (0..9).map(|i| format!("{i:032}")).collect();
        let (mut a, mut b) = perfekt();
        a.aktive_quellen = gemeinsam.clone();
        b.aktive_quellen = gemeinsam.clone();
        b.aktive_quellen.push("f".repeat(32));
        let u = beurteile(&a, &b);
        assert!(
            (u.quellen_jaccard - 0.9).abs() < 1e-9,
            "9 gemeinsame von 10 ergeben Jaccard 0,9: {}",
            u.quellen_jaccard
        );
        assert_eq!(
            u.klasse,
            Vergleichbarkeit::Stark,
            "genau am Gate {GATE_QUELLEN_JACCARD} ist stark noch moeglich"
        );

        // Ein Element mehr auf der anderen Seite: 9 von 11 = 0,818.
        a.aktive_quellen.push("g".repeat(32));
        let u2 = beurteile(&a, &b);
        assert!(u2.quellen_jaccard < GATE_QUELLEN_JACCARD);
        assert_eq!(u2.klasse, Vergleichbarkeit::Schwach);
        assert!(u2.gruende.contains(&Herabstufungsgrund::QuellenVerschieden));
    }

    // ── Material-Cosine ─────────────────────────────────────────────────
    //
    // Hier wird die Grenze nicht konstruiert, sondern GESUCHT: die
    // Cosinus-Aehnlichkeit haengt an den Bytewerten, und ein von Hand
    // gerechneter Grenzfall waere eine zweite Wahrheit ueber die Formel.
    // Der Fall prueft statt dessen, dass ES eine Grenze gibt und dass sie
    // GENAU beim Gate liegt.
    {
        let (a, mut b) = perfekt();
        let mut letzte_starke: Option<f64> = None;
        let mut erste_schwache: Option<f64> = None;
        for stufe in 0..=60u8 {
            b.fingerprint = Some(fp(40u8.wrapping_add(stufe * 3)));
            let u = beurteile(&a, &b);
            if u.klasse == Vergleichbarkeit::Stark {
                letzte_starke = Some(u.material_cosine);
            } else if erste_schwache.is_none() && u.gruende.len() == 1 {
                erste_schwache = Some(u.material_cosine);
            }
        }
        let stark = letzte_starke.expect("es muss starke Faelle geben");
        let schwach = erste_schwache.expect("und schwache");
        assert!(
            stark >= GATE_MATERIAL_COSINE,
            "jeder starke Fall liegt am oder ueber dem Gate: {stark} gegen {GATE_MATERIAL_COSINE}"
        );
        assert!(
            schwach < GATE_MATERIAL_COSINE,
            "und jeder schwache darunter: {schwach}"
        );
    }
}

#[test]
fn coverage_is_measured_signal_not_wall_duration() {
    // M-30: "Abdeckung ist die tatsaechlich gemessene Signalabdeckung der
    // Passage, nicht ihre Wanddauer." Der Beleg dafuer ist strukturell: das
    // Feld traegt einen Anteil in [0, 1], keine Dauer — eine Passage kann
    // beliebig lang sein und trotzdem null Abdeckung haben.
    let (mut a, b) = perfekt();
    a.projekt_ende = a.projekt_start + 48_000_000; // 1000 Sekunden Wanddauer
    a.abdeckung = 0.1; // aber fast kein Signal
    let u = beurteile(&a, &b);
    assert_eq!(
        u.klasse,
        Vergleichbarkeit::Unvergleichbar,
        "eine sehr lange Passage mit fast keinem Signal traegt keinen Vergleich - \
         die Wanddauer rettet nichts"
    );
    assert!(u.gruende.contains(&Herabstufungsgrund::AbdeckungZuGering));
}

#[test]
fn suspend_counts_as_missing_coverage() {
    // M-30 zweite Haelfte: "Smart Disable und Suspend gelten als FEHLENDE
    // Coverage." Das ist eine Zusage ueber den ERZEUGER der Zahl — sie muss
    // beim Aussetzen sinken, nicht stehenbleiben. Hier wird die Folge
    // gemessen: eine gesunkene Abdeckung nimmt der Passage ihr Siegerlabel.
    let (a, mut b) = perfekt();
    // Der Erzeuger hat waehrend der halben Passage ausgesetzt.
    b.abdeckung = 0.45;
    let u = beurteile(&a, &b);
    assert_eq!(u.klasse, Vergleichbarkeit::Unvergleichbar);
    assert!(u.gruende.contains(&Herabstufungsgrund::AbdeckungZuGering));

    // Und die Gegenprobe: knapp darueber traegt sie wieder.
    b.abdeckung = 0.55;
    assert_ne!(beurteile(&a, &b).klasse, Vergleichbarkeit::Unvergleichbar);
}

#[test]
fn kein_beleg_wird_still_uebergangen() {
    // M-28: "Keine Herabstufung ist ein stiller Ausschluss." Jede
    // Herabstufung traegt IHREN Grund, und mehrere Gruende reisen alle —
    // ein Empfaenger, der nur den ersten saehe, behobe ihn und faende den
    // naechsten.
    let (a, mut b) = perfekt();
    b.projekt_start = 960_000;
    b.projekt_ende = 1_440_000;
    b.abdeckung = 0.1;
    b.samplerate = 44_100.0;
    let u = beurteile(&a, &b);
    assert_eq!(u.klasse, Vergleichbarkeit::Unvergleichbar);
    assert!(
        u.gruende.len() >= 3,
        "alle drei Widersprueche reisen, nicht nur der erste: {:?}",
        u.gruende
    );
    // Und jeder Grund hat einen maschinenlesbaren Namen.
    for g in &u.gruende {
        let name: &str = g.name();
        assert!(!name.is_empty());
        assert!(name.chars().all(|c: char| c.is_ascii_lowercase() || c == '_'));
    }
}

#[test]
fn zahlenraender_saettigen_statt_umzubrechen() {
    // M-17 in der Passagenrechnung: zwei gueltige Fenster an
    // entgegengesetzten i64-Raendern duerfen weder paniken noch umbrechen
    // und dadurch als deckungsgleich gelten.
    let (mut a, mut b) = perfekt();
    a.projekt_start = i64::MIN;
    a.projekt_ende = i64::MIN + 480_000;
    b.projekt_start = i64::MAX - 480_000;
    b.projekt_ende = i64::MAX;
    let u = beurteile(&a, &b);
    assert_eq!(
        u.zeitueberdeckung, 0.0,
        "zwei Fenster an den entgegengesetzten i64-Raendern ueberdecken sich NICHT"
    );
    assert_eq!(u.klasse, Vergleichbarkeit::Unvergleichbar);

    // Ein Fenster der Laenge 0 ist keine Passage.
    let (mut a2, b2) = perfekt();
    a2.projekt_ende = a2.projekt_start;
    let u2 = beurteile(&a2, &b2);
    assert_eq!(u2.zeitueberdeckung, 0.0);
    assert_eq!(u2.klasse, Vergleichbarkeit::Unvergleichbar);
}

#[test]
fn leere_quellensets_sind_nicht_identisch() {
    // Ohne diese Regel waere eine Passage ohne bestaetigte Quellen mit jeder
    // anderen maximal vergleichbar — der teuerste Fehler, den ein
    // Jaccard-Index machen kann.
    let (mut a, mut b) = perfekt();
    a.aktive_quellen.clear();
    b.aktive_quellen.clear();
    let u = beurteile(&a, &b);
    assert_eq!(u.quellen_jaccard, 0.0);
    assert_eq!(u.klasse, Vergleichbarkeit::Unvergleichbar);
    assert!(u.gruende.contains(&Herabstufungsgrund::QuellenVerschieden));
}

#[test]
fn fehlender_fingerprint_ist_keine_aehnlichkeit() {
    let (a, mut b) = perfekt();
    b.fingerprint = None;
    let u = beurteile(&a, &b);
    assert_eq!(u.material_cosine, 0.0);
    assert_eq!(u.klasse, Vergleichbarkeit::Unvergleichbar);
    assert!(u.gruende.contains(&Herabstufungsgrund::MaterialVerschieden));

    // Und beidseitig fehlend erst recht nicht.
    let (mut a2, mut b2) = perfekt();
    a2.fingerprint = None;
    b2.fingerprint = None;
    assert_eq!(beurteile(&a2, &b2).material_cosine, 0.0);
}

#[test]
fn urteil_traegt_seine_kalibrierungsfassung() {
    // M-29: die Gates sind Startwerte, am Korpus zu kalibrieren. Ein Urteil,
    // das seine Fassung nicht mitfuehrt, ist nach der ersten Kalibrierung
    // eine Klasse ohne Massstab — dieselbe `Stark` meint dann etwas anderes.
    // Dasselbe Muster wie das `version`-Feld im Fingerprint.
    let (a, b) = perfekt();
    let u = beurteile(&a, &b);
    assert_eq!(u.metrics_version, vergleichbarkeit::METRICS_VERSION);

    // Auch ein Urteil, das an einem Beleg scheitert, sagt unter WELCHEN
    // Gates es gescheitert ist. Sonst waere gerade die interessante
    // Herabstufung die einzige ohne Massstab.
    let (a2, mut b2) = perfekt();
    b2.samplerate = 44_100.0;
    let schlecht = beurteile(&a2, &b2);
    assert_eq!(schlecht.klasse, Vergleichbarkeit::Unvergleichbar);
    assert_eq!(schlecht.metrics_version, vergleichbarkeit::METRICS_VERSION);

    // Und sie ist die Fassung des Registers, nicht irgendeine Zahl.
    let register = std::fs::read_to_string("../eq-copilot/schemas/v3/metriken-v1.json")
        .expect("Metrikregister lesbar");
    let marke = format!("\"aktuell\": {}", vergleichbarkeit::METRICS_VERSION);
    assert!(register.contains(&marke), "Register nennt eine andere Fassung als {marke}");
}
