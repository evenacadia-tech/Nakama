//! Der PRE/POST-Join (SONDE-013 M-13 bis M-17, M-19, M-21 bis M-23; §38).
//!
//! Der Satz, den dieses Bein trägt, steht im Exit-Gate von M-21:
//!
//! > "Kein unbekannter Zeitpfad erzeugt eine starke Cross-Probe- oder
//! >  PRE/POST-Aussage."
//!
//! ## Warum die Hüllkurven hier synthetisch sind
//!
//! §38.5 verlangt "verteilte Feature-Goldens mit 0 bis 2 s bekannter
//! Verzögerung". Bekannt heisst: die Verzögerung muss beim Bau des Korpus
//! feststehen, sonst prüft der Test seine eigene Schätzung gegen sich selbst.
//! Eine synthetische Hüllkurve, um N Frames verschoben, hat diese Eigenschaft
//! exakt — ein durch eine echte Kette gelaufenes Audiosignal hat sie nur
//! ungefähr. Was der Broker sieht, sind ohnehin genau diese Hüllkurven und
//! nichts anderes; die Audioseite misst B24.

use eqcop_broker::coordinator::prepost::{
    beurteile_paar, bilde_paare, dreifachergebnis, frameschluessel, kettenbefund, schaetze_restlag,
    suchraum_frames, ueberlappung, Alignmentklasse, Ausschlussgrund, Frameschluessel,
    Herabstufungsgrund, Kettenbefund, Paarhaelfte, Rolle, FEATURE_HOP_MS, GATE_UEBERLAPPUNG,
    METRICS_VERSION,
};

const RATE: f64 = 48_000.0;
/// 30 s Material bei 100 ms Hop.
const FRAMES: usize = 300;

/// Eine reproduzierbare, musikähnliche Hüllkurve: Grundpuls plus Oberwelle
/// plus ein deterministisches Rauschen. Wichtig ist die APERIODIZITÄT — eine
/// reine Sinushüllkurve korreliert bei JEDER Periodenverschiebung gleich gut
/// und liefert deshalb kein eindeutiges Maximum.
fn huelle(laenge: usize, saat: u64, takt: f64) -> Vec<f32> {
    let mut aus = Vec::with_capacity(laenge);
    let mut x = saat.wrapping_mul(6_364_136_223_846_793_005).wrapping_add(1);
    for i in 0..laenge {
        x = x.wrapping_mul(6_364_136_223_846_793_005).wrapping_add(1_442_695_040_888_963_407);
        let rauschen = ((x >> 33) as f64 / (1u64 << 31) as f64) - 0.5;
        let t = i as f64;
        let puls = (t / takt * std::f64::consts::TAU).sin().max(0.0).powi(3);
        let lang = (t / (takt * 7.3) * std::f64::consts::TAU).sin() * 0.3;
        aus.push((0.35 + puls * 0.5 + lang * 0.2 + rauschen * 0.12).max(0.01) as f32);
    }
    aus
}

/// Verschiebt eine Spur um `lag` Frames nach hinten (positiv = später).
fn verzoegere(spur: &[f32], lag: i64) -> Vec<f32> {
    let mut aus = vec![0.01f32; spur.len()];
    for i in 0..spur.len() {
        let j = i as i64 - lag;
        if j >= 0 && (j as usize) < spur.len() {
            aus[i] = spur[j as usize];
        }
    }
    aus
}

fn haelfte(instance: &str, huellkurven: Vec<Vec<f32>>, onsets: Vec<f32>) -> Paarhaelfte {
    Paarhaelfte {
        instance_id: instance.to_string(),
        host_pid: 4242,
        transport_epoch: 7,
        sample_rate: RATE,
        resolution: 4096,
        verbunden: true,
        stale: false,
        messbereit: true,
        projekt_fenster: Some((0, (FRAMES as i64) * (FEATURE_HOP_MS * RATE as i64 / 1000))),
        spruenge: 0,
        aktiv_s: FRAMES as f64 * FEATURE_HOP_MS as f64 / 1000.0,
        huellkurven,
        onsets,
        // Nacharbeit 1 (B26): der Presentation-Nachweis ist Teil der Haelfte.
        // Die Vorgabe fuer die bestehenden Faelle ist `true` - sie messen
        // ausdruecklich das ALIGNMENT, nicht den Nachweis; der eigene Fall
        // dafuer steht unten.
        presentation_validiert: true,
        session_epoch: 11,
        timeline_epoch: 7,
    }
}

/// Ein Paar mit bekannter Verzögerung und sonst perfekter Lage.
fn paar_mit_lag(lag: i64) -> (Paarhaelfte, Paarhaelfte) {
    let baender: Vec<Vec<f32>> = (0..4)
        .map(|b| huelle(FRAMES, 11 + b as u64, 5.0 + b as f64 * 2.0))
        .collect();
    let onsets = huelle(FRAMES, 99, 4.0);
    let post_baender = baender.iter().map(|b| verzoegere(b, lag)).collect();
    let post_onsets = verzoegere(&onsets, lag);
    (
        haelfte("aaaa", baender, onsets),
        haelfte("bbbb", post_baender, post_onsets),
    )
}

fn perfekt() -> (Paarhaelfte, Paarhaelfte) {
    paar_mit_lag(0)
}

// ═════════════════════════════════════════════════════════════════════════
// M-13 · pairing_requires_exactly_one_pre_and_one_post
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn pairing_requires_exactly_one_pre_and_one_post() {
    let (pre, post) = perfekt();

    // Die Gegenprobe zuerst: ohne sie sagt kein Fall darunter etwas.
    let gut = bilde_paare(&[
        ("paar-a".into(), Rolle::Pre, pre.clone()),
        ("paar-a".into(), Rolle::Post, post.clone()),
    ]);
    assert_eq!(gut.len(), 1);
    assert_eq!(gut[0].klasse, Alignmentklasse::FeatureAligned, "{gut:?}");

    // Eine fehlende Hälfte ergibt einen BENANNTEN unvollständigen Zustand,
    // nie eine halbe Aussage.
    for nur in [Rolle::Pre, Rolle::Post] {
        let h = if nur == Rolle::Pre { pre.clone() } else { post.clone() };
        let aus = bilde_paare(&[("paar-a".into(), nur, h)]);
        assert_eq!(aus.len(), 1);
        assert_eq!(aus[0].klasse, Alignmentklasse::Unclear);
        assert_eq!(aus[0].ausschluss, Some(Ausschlussgrund::HaelfteFehlt));
        assert!(aus[0].ergebnis.is_none(), "keine halbe Aussage: {:?}", aus[0]);
    }

    // Zwei verschiedene pair_ids bleiben zwei Paare.
    let zwei = bilde_paare(&[
        ("paar-a".into(), Rolle::Pre, pre.clone()),
        ("paar-a".into(), Rolle::Post, post.clone()),
        ("paar-b".into(), Rolle::Pre, pre.clone()),
        ("paar-b".into(), Rolle::Post, post.clone()),
    ]);
    assert_eq!(zwei.len(), 2);
    assert!(zwei.iter().all(|u| u.klasse == Alignmentklasse::FeatureAligned));
}

// ═════════════════════════════════════════════════════════════════════════
// M-13 · duplicate_half_is_conflict_not_first_wins
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn duplicate_half_is_conflict_not_first_wins() {
    let (pre, post) = perfekt();
    let mut zweiter_pre = pre.clone();
    zweiter_pre.instance_id = "cccc".into();

    let aus = bilde_paare(&[
        ("paar-a".into(), Rolle::Pre, pre.clone()),
        ("paar-a".into(), Rolle::Pre, zweiter_pre),
        ("paar-a".into(), Rolle::Post, post.clone()),
    ]);
    assert_eq!(aus.len(), 1);
    assert_eq!(aus[0].ausschluss, Some(Ausschlussgrund::Paarkonflikt));
    assert_eq!(aus[0].klasse, Alignmentklasse::Unclear);
    // Der Punkt: KEIN Zufallssieger. `first()` zu nehmen hiesse, dem User
    // eine Aussage über einen Messpunkt zu geben, den er nicht gewählt hat.
    assert!(aus[0].ergebnis.is_none());

    let mut zweiter_post = post.clone();
    zweiter_post.instance_id = "dddd".into();
    let aus2 = bilde_paare(&[
        ("paar-a".into(), Rolle::Pre, pre),
        ("paar-a".into(), Rolle::Post, post),
        ("paar-a".into(), Rolle::Post, zweiter_post),
    ]);
    assert_eq!(aus2[0].ausschluss, Some(Ausschlussgrund::Paarkonflikt));
}

// ═════════════════════════════════════════════════════════════════════════
// M-14 · three_results_are_never_collapsed
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn three_results_are_never_collapsed() {
    // Eine Kette, die NUR den Pegel hebt: +6 dB breitbandig, kein Lag.
    let (pre, mut post) = perfekt();
    for band in post.huellkurven.iter_mut() {
        for v in band.iter_mut() {
            *v *= 2.0;
        }
    }
    let u = beurteile_paar("paar-a", &pre, &post);
    assert_eq!(u.klasse, Alignmentklasse::FeatureAligned, "{u:?}");
    let e = u.ergebnis.as_ref().expect("Ergebnis");

    // 1. Roh sieht überall rund +6 dB.
    assert!(
        e.roh_db.iter().all(|v| (*v - 6.02).abs() < 0.1),
        "roh: {:?}",
        e.roh_db
    );
    // 2. Ausgerichtet zieht den Breitbandgain ab und sieht rund 0.
    let ausg = e.ausgerichtet_db.as_ref().expect("ausgerichtet");
    assert!(ausg.iter().all(|v| v.abs() < 0.1), "ausgerichtet: {ausg:?}");
    assert!((e.match_gain_db.unwrap() - 6.02).abs() < 0.1);
    // 3. Die Wirkung sagt "breitbandig gleichmäßig" - mehr Pegel ist NICHT
    //    mehr Höhen. Genau diese Verwechslung schliesst die Dreiteilung aus.
    assert!(
        e.wirkung.as_ref().unwrap().contains("breitbandig"),
        "{:?}",
        e.wirkung
    );

    // Und die Gegenprobe: eine Kette, die die oberen Bänder hebt, sieht roh
    // GENAUSO aus wie oben minus Gain - aber die Wirkung ist eine andere.
    let (pre2, mut post2) = perfekt();
    let n = post2.huellkurven.len();
    for (i, band) in post2.huellkurven.iter_mut().enumerate() {
        let f = 1.0 + i as f32 / n as f32;
        for v in band.iter_mut() {
            *v *= f;
        }
    }
    let u2 = beurteile_paar("paar-b", &pre2, &post2);
    let e2 = u2.ergebnis.as_ref().unwrap();
    assert!(
        e2.wirkung.as_ref().unwrap().contains("hebt die oberen"),
        "{:?}",
        e2.wirkung
    );

    // Ohne sicheres Alignment gibt es KEIN ausgerichtetes Delta. Ein Delta
    // ohne Ausrichtung wäre eine Laufzeitänderung, die wie ein EQ aussieht.
    let (pre3, mut post3) = perfekt();
    post3.host_pid = 9999;
    let u3 = beurteile_paar("paar-c", &pre3, &post3);
    assert_eq!(u3.klasse, Alignmentklasse::Probable);
    let e3 = u3.ergebnis.as_ref().unwrap();
    assert!(!e3.roh_db.is_empty(), "die rohe Differenz bleibt");
    assert!(e3.ausgerichtet_db.is_none(), "aber das ausgerichtete Delta nicht");
    assert!(e3.wirkung.is_none(), "und erst recht keine interpretierte Wirkung");
}

// ═════════════════════════════════════════════════════════════════════════
// M-15 · feature_alignment_search_space_and_pdc_is_hint_only
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn feature_alignment_search_space_and_pdc_is_hint_only() {
    // Der Suchraum: ±2 s bei mindestens 6 s Capture, darunter proportional.
    assert_eq!(suchraum_frames(30.0), 20, "±2 s sind 20 Feature-Hops");
    assert_eq!(suchraum_frames(6.0), 20, "genau die Mindestdauer reicht");
    assert_eq!(suchraum_frames(3.0), 10, "die Hälfte des Captures, die Hälfte des Raums");
    assert_eq!(suchraum_frames(0.0), 0, "ohne Capture kein Suchraum");

    // Der Golden-Korpus aus §38.5: 0 bis 2 s bekannte Verzögerung, Treffer
    // innerhalb EINES Feature-Hops.
    for erwartet in [0i64, 1, 3, 7, 12, 20] {
        let (pre, post) = paar_mit_lag(erwartet);
        let capture = FRAMES as f64 * FEATURE_HOP_MS as f64 / 1000.0;
        let lag = schaetze_restlag(&pre, &post, capture).expect("Lag");
        assert!(
            (lag.frames - erwartet).abs() <= 1,
            "erwartet {erwartet}, gefunden {} (±1 Hop erlaubt)",
            lag.frames
        );
        // Die Auflösung wird MITGETEILT und keine Samplegenauigkeit
        // behauptet - das ist die Zusage aus M-15.
        assert_eq!(lag.aufloesung_ms, FEATURE_HOP_MS);
    }

    // Jenseits des Suchraums wird nichts gefunden, statt etwas Falsches: ein
    // Lag von 30 Frames liegt ausserhalb der ±20, und der beste Treffer
    // INNERHALB des Raums darf nicht als Ergebnis durchgehen.
    let (pre, post) = paar_mit_lag(30);
    let capture = FRAMES as f64 * FEATURE_HOP_MS as f64 / 1000.0;
    let lag = schaetze_restlag(&pre, &post, capture).expect("eine Schätzung gibt es");
    assert!(
        lag.frames.abs() <= 20,
        "der Suchraum wird nicht überschritten: {}",
        lag.frames
    );
    let u = beurteile_paar("paar-a", &pre, &post);
    assert_ne!(
        u.klasse,
        Alignmentklasse::FeatureAligned,
        "ein Lag ausserhalb des Suchraums darf nie stark ausgerichtet heissen: {u:?}"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-16 · alignment_class_has_four_values_and_each_criterion_falls_alone
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn alignment_class_has_four_values_and_each_criterion_falls_alone() {
    // Vier Werte, und ihre Rangfolge ist die Aussage.
    assert!(Alignmentklasse::Unclear < Alignmentklasse::Probable);
    assert!(Alignmentklasse::Probable < Alignmentklasse::AudioAligned);
    assert!(Alignmentklasse::AudioAligned < Alignmentklasse::FeatureAligned);

    let (pre, post) = perfekt();
    let gut = beurteile_paar("paar-a", &pre, &post);
    assert_eq!(gut.klasse, Alignmentklasse::FeatureAligned, "{gut:?}");
    assert!(gut.herabstufungen.is_empty());

    // Jedes Kriterium fällt ALLEIN, bei sonst perfekter Lage.

    // (1) Überlappung unter 80 %.
    let (a1, mut b1) = perfekt();
    let laenge = a1.projekt_fenster.unwrap().1;
    b1.projekt_fenster = Some((laenge / 2, laenge + laenge / 2));
    let u1 = beurteile_paar("paar-a", &a1, &b1);
    assert_eq!(u1.klasse, Alignmentklasse::Probable);
    assert!(u1.herabstufungen.contains(&Herabstufungsgrund::UeberlappungGering));

    // (2) Aktive Messzeit klafft.
    let (a2, mut b2) = perfekt();
    b2.aktiv_s = a2.aktiv_s * 0.5;
    let u2 = beurteile_paar("paar-a", &a2, &b2);
    assert_eq!(u2.klasse, Alignmentklasse::Probable);
    assert!(u2.herabstufungen.contains(&Herabstufungsgrund::AktivzeitKlafft));

    // (3) Verschiedene Prozesse.
    let (a3, mut b3) = perfekt();
    b3.host_pid = 1;
    let u3 = beurteile_paar("paar-a", &a3, &b3);
    assert_eq!(u3.klasse, Alignmentklasse::Probable);
    assert!(u3.herabstufungen.contains(&Herabstufungsgrund::VerschiedeneHostPids));

    // (4) Wechselnde Latenz: die zweite Hälfte trägt einen anderen Lag.
    let (a4, mut b4) = perfekt();
    let halb = FRAMES / 2;
    for band in b4.huellkurven.iter_mut() {
        let hinten: Vec<f32> = band[halb..].to_vec();
        let verschoben = verzoegere(&hinten, 5);
        band.truncate(halb);
        band.extend(verschoben);
    }
    let u4 = beurteile_paar("paar-a", &a4, &b4);
    assert_ne!(
        u4.klasse,
        Alignmentklasse::FeatureAligned,
        "eine wechselnde Latenz darf nie stark ausgerichtet heissen: {u4:?}"
    );

    // (5) Peak-to-Sidelobe: EXAKT periodisches Material in allen Spuren.
    //
    // Der Fall, in dem nur DIESES Kriterium fällt, und er ist alles andere
    // als konstruiert: ein geloopter Takt, in dem jedes Band demselben Puls
    // folgt. Die Spitze ist hoch (1,0), alle Spuren sind sich einig, der Lag
    // ist über Teilfenster stabil — und trotzdem ist er MEHRDEUTIG, weil
    // jede Taktperiode ein gleich hohes Nebenmaximum trägt. Ohne das
    // Peak-to-Sidelobe-Verhältnis aus §38.2 hiesse so ein Paar stark
    // ausgerichtet, obwohl niemand sagen kann, um wie viele Takte POST
    // verschoben ist.
    let takt: Vec<f32> = (0..FRAMES)
        .map(|i| if i % 5 == 0 { 1.0f32 } else { 0.02f32 })
        .collect();
    let a5 = haelfte("aaaa", vec![takt.clone(); 4], takt.clone());
    let b5 = haelfte("bbbb", vec![takt.clone(); 4], takt.clone());
    let u5 = beurteile_paar("paar-a", &a5, &b5);
    let lag5 = u5.restlag.as_ref().expect("ein Lag wird gefunden");
    assert!(lag5.spitze > 0.9, "die Spitze ist hoch: {}", lag5.spitze);
    assert!(
        lag5.konsistente_spuren >= 3,
        "und alle Spuren sind sich einig: {}",
        lag5.konsistente_spuren
    );
    // Bei mehrdeutigem Lag gewinnt der SPARSAMSTE: wo kein Versatz noetig
    // ist, wird keiner behauptet. Ohne diese Regel entschiede die
    // Aufloesungsreihenfolge, ob der Broker 0 oder 2 s Versatz meldet - bei
    // exakt demselben Signal.
    assert_eq!(
        lag5.frames, 0,
        "der sparsamste der gleich hohen Lags gewinnt, nicht der erste oder letzte"
    );
    // Und der Lag ist STABIL - genau das macht den Fall scharf: es faellt
    // wirklich nur das eine Kriterium. Zwei mathematisch gleich hohe Maxima
    // duerfen sich nicht in der letzten Nachkommastelle unterscheiden und
    // dadurch in zwei Teilfenstern verschieden ausfallen.
    assert_eq!(
        lag5.lag_zweite_haelfte, Some(lag5.frames),
        "der Gleichstand wird in beiden Teilfenstern gleich aufgeloest"
    );
    assert!(
        lag5.peak_to_sidelobe < 2.0,
        "aber der Lag ist mehrdeutig - jede Taktperiode traegt ein gleich hohes \
         Nebenmaximum: PSR {}",
        lag5.peak_to_sidelobe
    );
    assert_eq!(
        u5.klasse,
        Alignmentklasse::Probable,
        "ein mehrdeutiger Lag darf nie stark ausgerichtet heissen: {u5:?}"
    );
    assert!(u5.herabstufungen.contains(&Herabstufungsgrund::AlignmentSchwach));

    // `AudioAligned` ist in P4 unerreichbar: es verlangt PRE und POST als
    // Audiopuffer in derselben Instanz, und Compare-Routing gibt es hier
    // nicht. Kein Eingabewert der verteilten Seite vergibt sie.
    for lag in 0..=20 {
        let (p, q) = paar_mit_lag(lag);
        assert_ne!(beurteile_paar("x", &p, &q).klasse, Alignmentklasse::AudioAligned);
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-16 · noise_yields_unclear
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn noise_yields_unclear() {
    // Zwei unabhängige Rauschspuren: es GIBT keinen gemeinsamen Lag.
    let pre = haelfte(
        "aaaa",
        (0..4).map(|b| huelle(FRAMES, 1000 + b, 3.0)).collect(),
        huelle(FRAMES, 1500, 3.0),
    );
    let post = haelfte(
        "bbbb",
        (0..4).map(|b| huelle(FRAMES, 2000 + b, 3.7)).collect(),
        huelle(FRAMES, 2500, 3.7),
    );
    let u = beurteile_paar("paar-a", &pre, &post);
    assert_ne!(
        u.klasse,
        Alignmentklasse::FeatureAligned,
        "unabhängiges Material darf nie stark ausgerichtet heissen: {u:?}"
    );
    assert!(
        u.ergebnis.as_ref().map_or(true, |e| e.ausgerichtet_db.is_none()),
        "und kein ausgerichtetes Delta tragen"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-17 · coverage_is_saturating_and_normalised_to_shorter_window
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn coverage_is_saturating_and_normalised_to_shorter_window() {
    // Auf das KÜRZERE Fenster normiert: ein kurzes Fenster, das ganz im
    // langen liegt, ist voll abgedeckt.
    assert_eq!(ueberlappung((0, 100), (0, 1000)), 1.0);
    assert_eq!(ueberlappung((0, 1000), (0, 100)), 1.0);
    assert!((ueberlappung((0, 100), (50, 1000)) - 0.5).abs() < 1e-12);

    // Keine Überlappung heisst "vermutlich verschiedene Passagen", nicht
    // "0 % ähnlich" - und das Paar wird ausgeschlossen, nicht herabgestuft.
    assert_eq!(ueberlappung((0, 100), (100, 200)), 0.0);
    let (mut a, mut b) = perfekt();
    a.projekt_fenster = Some((0, 100_000));
    b.projekt_fenster = Some((100_000, 200_000));
    let u = beurteile_paar("paar-a", &a, &b);
    assert_eq!(u.ausschluss, Some(Ausschlussgrund::KeineUeberlappung));

    // ⚠️ Die Zahlenränder. Zwei GÜLTIGE Fenster an entgegengesetzten
    // i64-Rändern dürfen weder paniken noch umbrechen und dadurch als
    // deckungsgleich gelten.
    assert_eq!(ueberlappung((i64::MIN, i64::MIN + 100), (i64::MAX - 100, i64::MAX)), 0.0);
    assert_eq!(ueberlappung((i64::MIN, i64::MAX), (i64::MIN, i64::MAX)), 1.0);
    assert_eq!(ueberlappung((0, i64::MAX), (i64::MIN, 0)), 0.0);
    // Leere und rückwärts laufende Fenster ergeben 0, nicht 1.
    assert_eq!(ueberlappung((5, 5), (5, 5)), 0.0);
    assert_eq!(ueberlappung((10, 5), (0, 100)), 0.0);

    // Und die 80-%-Grenze fällt genau an ihrem Wert, nicht daneben.
    let einheit = FEATURE_HOP_MS * RATE as i64 / 1000;
    let laenge = FRAMES as i64 * einheit;
    for (versatz, erwartet_stark) in [(0i64, true), (laenge / 5 - einheit, true), (laenge / 5 + einheit, false)] {
        let (mut p, mut q) = perfekt();
        p.projekt_fenster = Some((0, laenge));
        q.projekt_fenster = Some((versatz, laenge + versatz));
        let u = beurteile_paar("paar-a", &p, &q);
        let stark = !u.herabstufungen.contains(&Herabstufungsgrund::UeberlappungGering);
        assert_eq!(
            stark, erwartet_stark,
            "Versatz {versatz}: Überlappung {} gegen Gate {GATE_UEBERLAPPUNG}",
            u.ueberlappung
        );
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-19 · distributed_pair_never_produces_transfer_function
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn distributed_pair_never_produces_transfer_function() {
    // Selbst bei perfekter Lage und linearer Kette entsteht KEIN
    // Frequenzgang, sondern eine Banddifferenz. §38.3 erlaubt H1(f) nur im
    // lokalen Compare-Routing, das es in P4 nicht gibt.
    let (pre, mut post) = perfekt();
    for band in post.huellkurven.iter_mut() {
        for v in band.iter_mut() {
            *v *= 1.5;
        }
    }
    let u = beurteile_paar("paar-a", &pre, &post);
    assert_eq!(u.klasse, Alignmentklasse::FeatureAligned);
    let e = u.ergebnis.as_ref().unwrap();
    // Es gibt genau so viele Werte wie Bänder - keine Frequenzachse feiner
    // als die übertragene Bandauflösung.
    assert_eq!(e.roh_db.len(), pre.huellkurven.len());
    assert_eq!(e.ausgerichtet_db.as_ref().unwrap().len(), pre.huellkurven.len());

    // Und die Markierung nichtlinearer Ketten (M-18) greift im selben Pfad.
    // Ein Kompressor: die Relation hängt am Pegel.
    let (p2, mut q2) = perfekt();
    for band in q2.huellkurven.iter_mut() {
        for v in band.iter_mut() {
            // 6:1 über der Schwelle - lauter wird stärker gedämpft. Die
            // Schwelle liegt UNTER dem Mittel der Hüllkurve, sonst griffe der
            // Kompressor nur in seltenen Spitzen und wäre kein adversarialer
            // Fall, sondern ein fast lineares Gerät.
            let x = *v as f64;
            *v = if x > 0.35 { (0.35 + (x - 0.35) / 6.0) as f32 } else { x as f32 };
        }
    }
    let u2 = beurteile_paar("paar-b", &p2, &q2);
    assert_ne!(
        u2.kettenbefund,
        Kettenbefund::Stationaer,
        "eine komprimierende Kette muss markiert werden: {:?}",
        u2.kettenbefund
    );
    assert!(
        u2.ergebnis
            .as_ref()
            .and_then(|e| e.wirkung.clone())
            .map_or(true, |w| w.contains("wahrscheinlich")),
        "und darf keine kausale Behauptung tragen"
    );

    // Ein Tremolo: die Relation schwankt über die Zeit, ohne am Pegel zu
    // hängen.
    let (p3, mut q3) = perfekt();
    for band in q3.huellkurven.iter_mut() {
        for (i, v) in band.iter_mut().enumerate() {
            let m = 1.0 + 0.7 * ((i as f64 / 4.0) * std::f64::consts::TAU).sin();
            *v = (*v as f64 * m.max(0.05)) as f32;
        }
    }
    let u3 = beurteile_paar("paar-c", &p3, &q3);
    assert_ne!(u3.kettenbefund, Kettenbefund::Stationaer, "{:?}", u3.kettenbefund);

    // Die Gegenprobe: eine lineare Kette bleibt stationär, sonst wäre die
    // Markierung wertlos.
    assert_eq!(u.kettenbefund, Kettenbefund::Stationaer);
}

// ═════════════════════════════════════════════════════════════════════════
// M-21 · unknown_time_path_can_never_reach_strong
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn unknown_time_path_can_never_reach_strong() {
    let (pre, post) = perfekt();
    let u = beurteile_paar("paar-a", &pre, &post);
    assert_eq!(u.klasse, Alignmentklasse::FeatureAligned);

    // Ohne validierte Presentation-Abbildung bleibt der Schlüssel ROH, und
    // zwar auch bei perfektem Alignment. Das ist das Exit-Gate.
    let roh = frameschluessel(&pre, 48_000, false, 1, 2, u.klasse, 0);
    assert!(
        matches!(roh, Frameschluessel::Roh { .. }),
        "unvalidierter Zeitpfad: {roh:?}"
    );

    // Und mit Abbildung, aber schwachem Alignment ebenso.
    let roh2 = frameschluessel(&pre, 48_000, true, 1, 2, Alignmentklasse::Probable, 0);
    assert!(matches!(roh2, Frameschluessel::Roh { .. }), "{roh2:?}");
    let roh3 = frameschluessel(&pre, 48_000, true, 1, 2, Alignmentklasse::Unclear, 0);
    assert!(matches!(roh3, Frameschluessel::Roh { .. }), "{roh3:?}");

    // Erst BEIDES zusammen ergibt den starken Schlüssel.
    let stark = frameschluessel(&pre, 48_000, true, 1, 2, Alignmentklasse::FeatureAligned, 0);
    assert!(matches!(stark, Frameschluessel::Ausgerichtet { .. }), "{stark:?}");

    // Der rohe Schlüssel ist an die INSTANZ gebunden - das ist sein Zweck.
    // Zwei Instanzen bekommen nie denselben, auch bei identischer Zeit.
    let a = frameschluessel(&pre, 48_000, false, 1, 2, u.klasse, 0);
    let b = frameschluessel(&post, 48_000, false, 1, 2, u.klasse, 0);
    assert_ne!(a, b, "der rohe Schlüssel behauptet keine Inhaltsgleichheit");

    // Der starke Schlüssel dagegen ist über die Instanzgrenze hinweg gleich -
    // genau dafür braucht er die zwei Nachweise.
    let sa = frameschluessel(&pre, 48_000, true, 1, 2, Alignmentklasse::FeatureAligned, 0);
    let sb = frameschluessel(&post, 48_000, true, 1, 2, Alignmentklasse::FeatureAligned, 0);
    assert_eq!(sa, sb);
}

// ═════════════════════════════════════════════════════════════════════════
// M-22 · hard_exclusions_each_name_their_reason
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn hard_exclusions_each_name_their_reason() {
    let faelle: Vec<(&str, Box<dyn Fn(&mut Paarhaelfte)>, Ausschlussgrund)> = vec![
        ("getrennt", Box::new(|h: &mut Paarhaelfte| h.verbunden = false), Ausschlussgrund::HaelfteGetrennt),
        ("stale", Box::new(|h: &mut Paarhaelfte| h.stale = true), Ausschlussgrund::HaelfteStale),
        ("sammelt noch", Box::new(|h: &mut Paarhaelfte| h.messbereit = false), Ausschlussgrund::NichtMessbereit),
        ("andere Rate", Box::new(|h: &mut Paarhaelfte| h.sample_rate = 44_100.0), Ausschlussgrund::SamplerateVerschieden),
        ("ohne Transport", Box::new(|h: &mut Paarhaelfte| h.projekt_fenster = None), Ausschlussgrund::KeineProjektzeit),
        ("leeres Fenster", Box::new(|h: &mut Paarhaelfte| h.projekt_fenster = Some((5, 5))), Ausschlussgrund::FensterLeer),
        ("Sprung", Box::new(|h: &mut Paarhaelfte| h.spruenge = 1), Ausschlussgrund::SprungImFenster),
    ];

    for (name, kaputt, grund) in faelle {
        // Beide Seiten: der Riegel darf nicht an der Reihenfolge hängen.
        for seite in 0..2 {
            let (mut a, mut b) = perfekt();
            if seite == 0 { kaputt(&mut a) } else { kaputt(&mut b) }
            let u = beurteile_paar("paar-a", &a, &b);
            assert_eq!(u.klasse, Alignmentklasse::Unclear, "{name} (Seite {seite})");
            assert_eq!(u.ausschluss, Some(grund), "{name} (Seite {seite})");
            assert!(u.ergebnis.is_none(), "{name}: keine schwache Zahl");
            assert!(u.restlag.is_none(), "{name}: und kein Lag");
        }
    }

    // Eine Samplerate von 0 ist kein "gleich wie die andere".
    let (mut a, mut b) = perfekt();
    a.sample_rate = 0.0;
    b.sample_rate = 0.0;
    assert_eq!(
        beurteile_paar("paar-a", &a, &b).ausschluss,
        Some(Ausschlussgrund::SamplerateVerschieden)
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-23 · each_downgrade_reason_falls_alone
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn each_downgrade_reason_falls_alone() {
    // Genau DREI Gründe im v3-Pfad, und der vierte des v2-Vorläufers ist zum
    // Ausschluss geworden: ein Sprung invalidiert, statt herabzustufen.
    let (mut a, b) = perfekt();
    a.spruenge = 1;
    let u = beurteile_paar("paar-a", &a, &b);
    assert_eq!(
        u.ausschluss,
        Some(Ausschlussgrund::SprungImFenster),
        "ein Sprung ist im v3-Pfad ein Epochenbruch, keine Herabstufung"
    );
    assert!(u.herabstufungen.is_empty());

    // Alle Gründe reisen mit, nicht nur der erste.
    let (mut p, mut q) = perfekt();
    q.host_pid = 1;
    q.aktiv_s = p.aktiv_s * 0.5;
    let laenge = p.projekt_fenster.unwrap().1;
    p.projekt_fenster = Some((0, laenge));
    q.projekt_fenster = Some((laenge / 2, laenge + laenge / 2));
    let u2 = beurteile_paar("paar-a", &p, &q);
    assert_eq!(u2.klasse, Alignmentklasse::Probable);
    assert!(u2.herabstufungen.contains(&Herabstufungsgrund::VerschiedeneHostPids));
    assert!(u2.herabstufungen.contains(&Herabstufungsgrund::AktivzeitKlafft));
    assert!(u2.herabstufungen.contains(&Herabstufungsgrund::UeberlappungGering));

    // Die 10-%-Grenze der Aktivzeit fällt an ihrem Wert.
    for (faktor, erwartet) in [(0.95f64, false), (0.85, true)] {
        let (x, mut y) = perfekt();
        y.aktiv_s = x.aktiv_s * faktor;
        let u = beurteile_paar("paar-a", &x, &y);
        assert_eq!(
            u.herabstufungen.contains(&Herabstufungsgrund::AktivzeitKlafft),
            erwartet,
            "Faktor {faktor}"
        );
    }
}

// ═════════════════════════════════════════════════════════════════════════
// Die Kalibrierungsfassung reist mit
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn urteil_traegt_seine_kalibrierungsfassung() {
    let (pre, post) = perfekt();
    assert_eq!(beurteile_paar("paar-a", &pre, &post).metrics_version, METRICS_VERSION);
    let (mut a, b) = perfekt();
    a.stale = true;
    assert_eq!(beurteile_paar("paar-a", &a, &b).metrics_version, METRICS_VERSION);

    let register = std::fs::read_to_string("../eq-copilot/schemas/v3/metriken-v1.json")
        .expect("Metrikregister lesbar");
    assert!(register.contains(&format!("\"aktuell\": {METRICS_VERSION}")));
}

// ═════════════════════════════════════════════════════════════════════════
// Die Helfer einzeln - sie tragen die Zusagen oben
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn kettenbefund_und_dreifachergebnis_haengen_nicht_am_urteil() {
    // `kettenbefund` und `dreifachergebnis` sind öffentlich, weil der
    // Snapshotpfad sie einzeln braucht. Also werden sie auch einzeln
    // gemessen - sonst hinge ihre Zusage an `beurteile_paar`.
    let (pre, post) = perfekt();
    assert_eq!(kettenbefund(&pre, &post, None), Kettenbefund::Stationaer);

    let e = dreifachergebnis(&pre, &post, Alignmentklasse::Unclear, None, Kettenbefund::Stationaer);
    assert!(e.ausgerichtet_db.is_none(), "Unclear trägt kein ausgerichtetes Delta");
    assert!(e.wirkung.is_none());
    let e2 = dreifachergebnis(
        &pre,
        &post,
        Alignmentklasse::FeatureAligned,
        None,
        Kettenbefund::PegelabhaengigMarkiert,
    );
    assert!(e2.ausgerichtet_db.is_some());
    assert!(
        e2.wirkung.as_ref().unwrap().contains("wahrscheinlich"),
        "eine markierte Kette trägt keine kausale Behauptung: {:?}",
        e2.wirkung
    );
}

// ═════════════════════════════════════════════════════════════════════════
// Selbstaudit: nicht-endliche Werte dürfen keine Aussage tragen
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn nicht_endliche_werte_ergeben_keine_stationaere_kette() {
    // ⚠️ Der Fund, für den dieser Fall existiert: JEDER Vergleich mit NaN ist
    // false. `x <= 1e-9` liess ein NaN durch, die Relation wurde NaN, die
    // Streuung wurde NaN — und `NaN > GATE` ist wieder false. Die Kette galt
    // damit als STATIONÄR und durfte einen festen Übertragungsgang tragen.
    // Ein einziger kaputter Frame hätte genau die Behauptung erlaubt, die
    // M-18 verbietet. Gefunden im Selbstaudit von Etappe H.
    for kaputt in [f32::NAN, f32::INFINITY, f32::NEG_INFINITY] {
        let (pre, mut post) = perfekt();
        for band in post.huellkurven.iter_mut() {
            for v in band.iter_mut() {
                *v = kaputt;
            }
        }
        let befund = kettenbefund(&pre, &post, None);
        assert_ne!(
            befund,
            Kettenbefund::Stationaer,
            "{kaputt} darf keine stationäre Kette ergeben: {befund:?}"
        );
        assert_eq!(befund, Kettenbefund::NichtBeurteilbar, "{kaputt}");

        // Und es entsteht ueberhaupt keine Wirkungsaussage: ohne verwertbare
        // Relation gibt es kein ausgerichtetes Delta, und ohne Delta keine
        // Wirkung. Das ist die staerkere Antwort als ein vorsichtig
        // formulierter Satz.
        let e = dreifachergebnis(
            &pre,
            &post,
            Alignmentklasse::FeatureAligned,
            None,
            befund,
        );
        assert!(e.ausgerichtet_db.is_none(), "{kaputt}: {:?}", e.ausgerichtet_db);
        assert!(e.wirkung.is_none(), "{kaputt}: {:?}", e.wirkung);
    }

    // Der Textzweig fuer `NichtBeurteilbar` greift dort, wo ein Delta zwar
    // entsteht, die Kette aber nicht beurteilt werden konnte. Er darf nicht
    // „zeitvariabel oder nichtlinear" sagen - das weiss er nicht.
    let (gut_pre, gut_post) = perfekt();
    let e = dreifachergebnis(
        &gut_pre,
        &gut_post,
        Alignmentklasse::FeatureAligned,
        None,
        Kettenbefund::NichtBeurteilbar,
    );
    assert!(e.ausgerichtet_db.is_some());
    let w = e.wirkung.as_ref().expect("Wirkung");
    assert!(w.contains("nicht beurteilbar"), "{w}");
    assert!(!w.contains("zeitvariabel"), "{w}");

    // Ein EINZELNER kaputter Frame reicht schon, um die betroffene Stelle
    // auszuschliessen - aber der Rest des Bandes bleibt verwertbar.
    let (pre, mut post) = perfekt();
    post.huellkurven[0][7] = f32::NAN;
    let befund = kettenbefund(&pre, &post, None);
    assert_eq!(
        befund,
        Kettenbefund::Stationaer,
        "ein einzelner kaputter Frame macht die Kette nicht unbeurteilbar"
    );

    // Und dasselbe für die Ausrichtung: NaN macht Spitze und
    // Peak-to-Sidelobe zu NaN, und `NaN < GATE` ist false - die Kriterien
    // griffen dann NICHT und ein kaputter Frame erzeugte eine starke
    // Ausrichtung.
    let (a, mut b) = perfekt();
    for band in b.huellkurven.iter_mut() {
        for v in band.iter_mut() {
            *v = f32::NAN;
        }
    }
    for v in b.onsets.iter_mut() {
        *v = f32::NAN;
    }
    let u = beurteile_paar("paar-a", &a, &b);
    assert_ne!(
        u.klasse,
        Alignmentklasse::FeatureAligned,
        "unbrauchbares Material darf nie stark ausgerichtet heissen: {u:?}"
    );
}


// ═════════════════════════════════════════════════════════════════════════
// Nacharbeit 1 nach der Erstpruefung 1 (2026-09-04)

/// B26 - `FeatureAligned` verlangt den Presentation-Nachweis (M-21).
///
/// `beurteile_paar` vergab die Klasse allein aus der Featurekorrelation, und
/// `dreifachergebnis` behandelte sie danach als sicher und erzeugte
/// ausgerichtete Deltas. Der `frameschluessel`-Riegel, der die Abbildung
/// prueft, wurde dabei nie gerufen - obwohl das Exit-Gate ausdruecklich sagt:
/// "Kein unbekannter Zeitpfad erzeugt eine starke PRE/POST-Aussage."
#[test]
fn feature_aligned_verlangt_den_presentation_nachweis() {
    // Die Gegenprobe zuerst: mit Nachweis ist die starke Klasse erreichbar.
    let (pre, post) = paar_mit_lag(0);
    let mit = beurteile_paar("p1", &pre, &post);
    assert_eq!(
        mit.klasse,
        Alignmentklasse::FeatureAligned,
        "mit validierter Abbildung ist die starke Klasse erreichbar: {:?}",
        mit.herabstufungen
    );
    assert!(
        mit.ergebnis.as_ref().unwrap().ausgerichtet_db.is_some(),
        "und das ausgerichtete Delta entsteht"
    );

    // Und jetzt OHNE Nachweis - bei sonst identischer Lage.
    for (a_ok, b_ok) in [(false, true), (true, false), (false, false)] {
        let (mut p2, mut q2) = paar_mit_lag(0);
        p2.presentation_validiert = a_ok;
        q2.presentation_validiert = b_ok;
        let u = beurteile_paar("p2", &p2, &q2);
        assert_eq!(
            u.klasse,
            Alignmentklasse::Probable,
            "ohne Presentation-Nachweis ({a_ok}/{b_ok}) bleibt es bei probable"
        );
        assert!(
            u.herabstufungen
                .contains(&Herabstufungsgrund::KeinPresentationNachweis),
            "und der Grund ist benannt: {:?}",
            u.herabstufungen
        );
        assert!(
            u.ergebnis.as_ref().unwrap().ausgerichtet_db.is_none(),
            "kein ausgerichtetes Delta ohne sichere Zeitachse"
        );
    }
}

/// B27 - ein FEHLENDER Teilfenster-Lag wird nicht durch den Gesamtlag ersetzt
/// (M-16).
///
/// `unwrap_or(zentral)` setzte den fehlenden Beleg auf genau den Wert, gegen
/// den er geprueft wird. Das Stabilitaetsgate galt danach immer als bestanden.
#[test]
fn fehlender_teilfenster_lag_stuft_herab() {
    // Ein Paar, dessen zweite Haelfte zu kurz fuer eine eigene Messung ist:
    // `zweite_haelfte_lag` verlangt mindestens vier Frames je Spur.
    let kurz = 6usize;
    let baender: Vec<Vec<f32>> = (0..4)
        .map(|b| huelle(kurz, 11 + b as u64, 3.0 + b as f64))
        .collect();
    let onsets = huelle(kurz, 99, 2.0);
    let mut pre = haelfte("aaaa", baender.clone(), onsets.clone());
    let mut post = haelfte("bbbb", baender, onsets);
    let fenster = (0i64, (kurz as i64) * (FEATURE_HOP_MS * RATE as i64 / 1000));
    pre.projekt_fenster = Some(fenster);
    post.projekt_fenster = Some(fenster);
    pre.aktiv_s = kurz as f64 * FEATURE_HOP_MS as f64 / 1000.0;
    post.aktiv_s = pre.aktiv_s;

    let capture_s = (fenster.1 - fenster.0) as f64 / RATE;
    if let Some(lag) = schaetze_restlag(&pre, &post, capture_s) {
        assert!(
            lag.lag_zweite_haelfte.is_none(),
            "auf sechs Frames laesst sich kein zweiter Teilfensterlag messen"
        );
        let u = beurteile_paar("kurz", &pre, &post);
        assert_eq!(
            u.klasse,
            Alignmentklasse::Probable,
            "ohne zweiten Messpunkt bleibt es bei probable: {:?}",
            u.herabstufungen
        );
        assert!(
            u.herabstufungen
                .contains(&Herabstufungsgrund::TeilfensterLagFehlt),
            "und der fehlende Beleg wird BENANNT, nicht durch den Gesamtlag \
             ersetzt: {:?}",
            u.herabstufungen
        );
    } else {
        // Auch das ist ein gueltiges Ergebnis: ohne benennbaren Lag gibt es
        // gar keine Aussage. Dann misst der Fall die Klasse direkt.
        let u = beurteile_paar("kurz", &pre, &post);
        assert_eq!(
            u.klasse,
            Alignmentklasse::Unclear,
            "ohne benennbaren Lag gibt es keinen Zeitbezug"
        );
    }
}

/// B28 - kein NaN in den Banddeltas (M-07).
///
/// Fuer ein leeres oder stilles Band schrieb `mittlere_relation` ein
/// oeffentliches `f64::NAN` in `roh_db`; bei sicherem Alignment stand
/// derselbe NaN auch im ausgerichteten Vektor. Das ist eine nicht
/// serialisierbare, ungekennzeichnete Metrik statt "Wert 0 plus
/// Ungueltigkeit und Zaehler".
#[test]
fn leere_baender_erzeugen_kein_nan() {
    let (mut pre, mut post) = paar_mit_lag(0);
    // Ein Band ohne einen einzigen Frame - genau der Fall, der NaN erzeugte.
    pre.huellkurven.push(Vec::new());
    post.huellkurven.push(Vec::new());

    let u = beurteile_paar("nan", &pre, &post);
    let e = u.ergebnis.as_ref().expect("ein Ergebnis entsteht");
    assert!(
        e.roh_db.iter().all(|v| v.is_finite()),
        "kein roher Banddelta ist nichtendlich: {:?}",
        e.roh_db
    );
    if let Some(aus) = &e.ausgerichtet_db {
        assert!(
            aus.iter().all(|v| v.is_finite()),
            "und kein ausgerichteter: {aus:?}"
        );
    }
    assert_eq!(
        e.roh_gueltig.len(),
        e.roh_db.len(),
        "jedes Band traegt sein Praesenzbit"
    );
    assert!(
        e.baender_ohne_messung >= 1,
        "und das leere Band ist GEZAEHLT: {}",
        e.baender_ohne_messung
    );
    assert!(
        !e.roh_gueltig[e.roh_gueltig.len() - 1],
        "genau das leere Band traegt kein Praesenzbit"
    );
}

/// B29 - der Peak-to-Sidelobe bleibt endlich (M-07).
///
/// Eine eindeutige Spitze ohne positives Nebenmaximum erzeugte
/// `f64::INFINITY`; der Wert reiste als `Restlag.peak_to_sidelobe` weiter und
/// konnte sogar ein `FeatureAligned` tragen.
#[test]
fn peak_to_sidelobe_bleibt_endlich() {
    // Eine Spur, die ausserhalb der Sperrzone ueberall null ist: ein einzelner
    // Impuls. Sein Korrelationsmaximum steht allein.
    let mut impuls = vec![0.0f32; 64];
    impuls[32] = 1.0;
    let pre = haelfte("aaaa", vec![impuls.clone(); 4], impuls.clone());
    let post = haelfte("bbbb", vec![impuls.clone(); 4], impuls);

    let capture_s = 64.0 * FEATURE_HOP_MS as f64 / 1000.0;
    let lag = schaetze_restlag(&pre, &post, capture_s).expect("ein Lag entsteht");
    assert!(
        lag.peak_to_sidelobe.is_finite(),
        "der PSR ist endlich: {}",
        lag.peak_to_sidelobe
    );
    assert!(
        lag.psr_gedeckelt,
        "und der Fall 'kein Nebenmaximum' wird BENANNT statt als Unendlich \
         ausgedrueckt"
    );
    assert!(
        (lag.peak_to_sidelobe - eqcop_broker::coordinator::prepost::PSR_DECKEL).abs() < 1e-9,
        "im gedeckelten Fall steht der Deckel: {}",
        lag.peak_to_sidelobe
    );
}
