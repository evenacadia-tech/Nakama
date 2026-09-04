//! Das Experiment `manual_external` (SONDE-013 M-25, M-32, M-40 bis M-51;
//! §43).
//!
//! Der Satz, den dieses Bein trägt, steht in M-47:
//!
//! > „Das ist ausdrücklich der **häufigste Realfall** und Teil des Vertrags,
//! >  nicht ein Fehlerpfad."
//!
//! Gemeint ist der Abbruch. Ein Versuchsmodell, das den Abschluss als
//! Normalfall behandelt und den Abbruch als Ausnahme, misst am Verhalten des
//! Users vorbei — und lässt offene Zeilen liegen, bis der Store voll ist.

use eqcop_broker::coordinator::experiment::{
    behalten_erlaubt, block_bootstrap, cluster, fdr_signifikant, urteile, Abbruchgrund,
    Abschlussfehler, Achsen, Achsenbefund, Alignmentwert, Anlegefehler, Ausfuehrungsart,
    Blindreihenfolge, Ereignis, Experimentreferenz, Experimentstore, Hoerurteil, Passage,
    Reproduzierbarkeit, Terminal, Urteil, N_GLOBAL, N_PROJEKT,
};
use eqcop_broker::telemetrie::Fingerprintwerte;

fn hex32(n: u32) -> String {
    format!("{n:032x}")
}

fn fp(saat: u8) -> Fingerprintwerte {
    let mut f = Fingerprintwerte {
        version: 1,
        band_energie: [0u8; 32],
        chroma: [0u8; 12],
        onset: [0u8; 32],
    };
    for (i, v) in f.band_energie.iter_mut().enumerate() {
        *v = saat.wrapping_add(i as u8);
    }
    for (i, v) in f.chroma.iter_mut().enumerate() {
        *v = saat.wrapping_mul(3).wrapping_add(i as u8);
    }
    for (i, v) in f.onset.iter_mut().enumerate() {
        *v = saat.wrapping_mul(7).wrapping_add(i as u8);
    }
    f
}

fn passage(n: u32) -> Passage {
    Passage {
        passage_id: hex32(1000 + n),
        projekt_von: 480_000,
        projekt_bis: 960_000,
        transport_epoch: 7,
        aktive_quellen: vec![hex32(11), hex32(12)],
        abdeckung: 0.9,
        label: Some("Refrain".into()),
        fingerprint: fp(5),
    }
}

fn referenz(gain: f64) -> Experimentreferenz {
    Experimentreferenz {
        passage_fingerprint: fp(5),
        upstream_fingerprint: fp(9),
        aktive_quellen: vec![hex32(11), hex32(12)],
        messpunktklassen: vec!["pre".into(), "post".into()],
        match_gain_db: gain,
        alignment: Alignmentwert::FeatureAligned,
    }
}

/// Ein Store mit genau einem offenen Experiment.
fn store_mit_einem() -> (Experimentstore, String) {
    let mut s = Experimentstore::neu();
    let id = hex32(1);
    s.beginne(&id, "projekt-a", passage(1), referenz(-2.5))
        .expect("beginne");
    (s, id)
}

// ═════════════════════════════════════════════════════════════════════════
// M-25 · passage_carries_all_six_fields
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn passage_carries_all_six_fields() {
    let (s, id) = store_mit_einem();
    let e = s.experiment(&id).expect("Experiment");
    let p = s.passage(&e.passage_id).expect("Passage");

    // Die sechs Felder aus M-25, einzeln - eine Passage, die eines still
    // weglaesst, ist kein Evidenzobjekt mehr, sondern ein Zeitfenster mit
    // Namen.
    assert_eq!((p.projekt_von, p.projekt_bis), (480_000, 960_000));
    assert_eq!(p.transport_epoch, 7);
    assert_eq!(p.aktive_quellen.len(), 2);
    assert!(p.abdeckung >= 0.5);
    assert_eq!(p.label.as_deref(), Some("Refrain"));
    assert_eq!(p.fingerprint, fp(5));

    // Die Passage entsteht MIT dem Versuch, der sie nennt (§10.6), und das
    // Log haelt beide Ereignisse in dieser Reihenfolge.
    let log = s.log();
    assert!(matches!(log[0], Ereignis::PassageAngelegt { .. }));
    assert!(matches!(log[1], Ereignis::Begonnen { .. }));

    // Ein zweiter Versuch auf DERSELBEN Passage legt sie nicht noch einmal
    // an - sonst haette dasselbe Stueck Musik zwei Evidenzobjekte.
    let mut s2 = s;
    s2.beginne(&hex32(2), "projekt-a", passage(1), referenz(-2.5))
        .expect("zweiter Versuch");
    assert_eq!(
        s2.log()
            .iter()
            .filter(|e| matches!(e, Ereignis::PassageAngelegt { .. }))
            .count(),
        1
    );

    // Und eine Passage ohne genug gemessenes Signal traegt keinen Versuch
    // (M-30).
    let mut duenn = passage(9);
    duenn.abdeckung = 0.2;
    let mut s3 = Experimentstore::neu();
    assert_eq!(
        s3.beginne(&hex32(3), "projekt-a", duenn, referenz(-2.5)),
        Err(Anlegefehler::AbdeckungZuGering)
    );
    assert!(s3.log().is_empty(), "und hinterlaesst keine halbe Zeile");
}

// ═════════════════════════════════════════════════════════════════════════
// M-40 · manual_external_begin_locks_baseline
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn manual_external_begin_locks_baseline() {
    let (s, id) = store_mit_einem();
    let e = s.experiment(&id).expect("Experiment");

    assert_eq!(e.art, Ausfuehrungsart::ManualExternal);
    assert_eq!(
        e.reproduzierbarkeit,
        Reproduzierbarkeit::ManuellNichtWiederherstellbar
    );
    // Die Baseline traegt ALLE Referenzfelder aus M-40.
    assert_eq!(e.baseline.passage_fingerprint, fp(5));
    assert_eq!(e.baseline.upstream_fingerprint, fp(9));
    assert_eq!(e.baseline.aktive_quellen.len(), 2);
    assert_eq!(e.baseline.messpunktklassen.len(), 2);
    assert_eq!(e.baseline.match_gain_db, -2.5);
    assert_eq!(e.baseline.alignment, Alignmentwert::FeatureAligned);
    assert!(e.offen());

    // Dieselbe experiment_id ein zweites Mal ueberschreibt nichts: append-only
    // heisst auch, dass eine Wiederholung keine Umdeutung ist.
    let mut s2 = s;
    assert_eq!(
        s2.beginne(&id, "projekt-a", passage(1), referenz(-99.0)),
        Err(Anlegefehler::IdVergeben)
    );
    assert_eq!(s2.experiment(&id).unwrap().baseline.match_gain_db, -2.5);

    // Und eine ID, die keine hex32 ist, legt gar nichts an.
    assert_eq!(
        s2.beginne("kurz", "projekt-a", passage(2), referenz(0.0)),
        Err(Anlegefehler::IdUngueltig)
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-41 · second_change_creates_new_candidate_not_a_new_baseline
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn second_change_creates_new_candidate_not_a_new_baseline() {
    let (mut s, id) = store_mit_einem();
    let baseline_vorher = s.experiment(&id).unwrap().baseline.clone();

    assert_eq!(s.neuer_kandidat(&id, referenz(-1.0)), Ok(1));
    assert_eq!(s.neuer_kandidat(&id, referenz(-0.5)), Ok(2));

    let e = s.experiment(&id).unwrap();
    assert_eq!(e.baseline, baseline_vorher, "die Baseline bleibt unberuehrt");
    assert_eq!(e.kandidaten.len(), 2);
    assert_eq!(e.kandidaten[0].nummer, 1);
    assert_eq!(e.kandidaten[1].nummer, 2);
    // Die Baseline ist NICHT Kandidat 0 - eine gemeinsame Nummerierung luede
    // dazu ein, sie zu ueberschreiben.
    assert!(e.kandidaten.iter().all(|k| k.nummer > 0));

    // Nach dem Terminalereignis gibt es keinen neuen Kandidaten mehr.
    s.binde_reihenfolge(&id, Blindreihenfolge::BaselineZuerst).unwrap();
    s.ergebnis(&id, Hoerurteil::Kandidat, None, None).unwrap();
    assert_eq!(
        s.neuer_kandidat(&id, referenz(0.0)),
        Err(Abschlussfehler::SchonTerminal)
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-42 · manual_external_has_no_state_hash_and_no_revert
//        lower_reproducibility_class_is_explicit
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn manual_external_has_no_state_hash_and_no_revert() {
    let (mut s, id) = store_mit_einem();
    s.binde_reihenfolge(&id, Blindreihenfolge::KandidatZuerst).unwrap();
    s.ergebnis(
        &id,
        Hoerurteil::Kandidat,
        Some("Kompressor haerter gefahren".into()),
        Some("FabFilter Pro-C".into()),
    )
    .unwrap();

    let e = s.experiment(&id).unwrap();
    let Some(Terminal::Ergebnis {
        notiz, werkzeug, ..
    }) = &e.terminal
    else {
        panic!("Ergebnis erwartet: {:?}", e.terminal);
    };
    // Statt State-Hashes: Usernotiz und optionaler Werkzeugname (M-42).
    assert_eq!(notiz.as_deref(), Some("Kompressor haerter gefahren"));
    assert_eq!(werkzeug.as_deref(), Some("FabFilter Pro-C"));

    // Beide sind OPTIONAL - ein Versuch ohne Notiz ist gueltig, und ihn zu
    // erzwingen hiesse, den User zu einer Erfindung zu draengen.
    let (mut s2, id2) = store_mit_einem();
    s2.binde_reihenfolge(&id2, Blindreihenfolge::BaselineZuerst).unwrap();
    assert!(s2.ergebnis(&id2, Hoerurteil::Enthaltung, None, None).is_ok());
}

#[test]
fn lower_reproducibility_class_is_explicit() {
    let (s, id) = store_mit_einem();
    let e = s.experiment(&id).unwrap();
    // Die niedrigere Klasse ist ein eigener Wert im Typ und keine fehlende
    // Angabe: `manuell · nicht automatisch wiederherstellbar` steht da, wo
    // sonst eine automatische Klasse staende.
    assert_eq!(
        e.reproduzierbarkeit,
        Reproduzierbarkeit::ManuellNichtWiederherstellbar
    );
    // Die MESSUNG bleibt trotzdem gueltig (M-42): Passage, Fingerprints und
    // Alignment sind vollstaendig da.
    assert_eq!(e.baseline.alignment, Alignmentwert::FeatureAligned);
    assert_eq!(e.baseline.passage_fingerprint, fp(5));
}

// ═════════════════════════════════════════════════════════════════════════
// M-43 · match_gain_is_frozen_in_the_immutable_reference
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn match_gain_is_frozen_in_the_immutable_reference() {
    let (mut s, id) = store_mit_einem();
    assert_eq!(s.experiment(&id).unwrap().baseline.match_gain_db, -2.5);

    // Ein Kandidat traegt seinen EIGENEN eingefrorenen Wert; er ersetzt den
    // der Baseline nicht.
    s.neuer_kandidat(&id, referenz(-1.75)).unwrap();
    let e = s.experiment(&id).unwrap();
    assert_eq!(e.baseline.match_gain_db, -2.5);
    assert_eq!(e.kandidaten[0].referenz.match_gain_db, -1.75);

    // Ohne Lautheitsabgleich gibt es kein Klangurteil (§15, M-43). Ein NaN
    // ist hier genau die Form von "nie gemessen": keine Zahl, kein Urteil.
    let mut s2 = Experimentstore::neu();
    let id2 = hex32(77);
    s2.beginne(&id2, "projekt-a", passage(2), referenz(f64::NAN))
        .unwrap();
    s2.binde_reihenfolge(&id2, Blindreihenfolge::BaselineZuerst).unwrap();
    assert_eq!(
        s2.ergebnis(&id2, Hoerurteil::Kandidat, None, None),
        Err(Abschlussfehler::OhneLautheitsabgleich)
    );
    // Abbrechen geht trotzdem - sonst haenge so ein Versuch fuer immer offen.
    assert!(s2.schliesse(&id2, Abbruchgrund::UserAbbruch).is_ok());
}

// ═════════════════════════════════════════════════════════════════════════
// M-44 · blind_order_is_bound_before_the_verdict_and_revealed_after
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn blind_order_is_bound_before_the_verdict_and_revealed_after() {
    let (mut s, id) = store_mit_einem();

    // Vor der Bindung: nichts zu sehen und nichts gebunden.
    assert!(!s.experiment(&id).unwrap().reihenfolge_gebunden());
    assert_eq!(s.experiment(&id).unwrap().aufgedeckte_reihenfolge(), None);

    // Gebunden - aber weiterhin verborgen. DAS ist die Kante aus M-44.
    assert_eq!(
        s.binde_reihenfolge(&id, Blindreihenfolge::KandidatZuerst),
        Ok(true)
    );
    assert!(s.experiment(&id).unwrap().reihenfolge_gebunden());
    assert_eq!(
        s.experiment(&id).unwrap().aufgedeckte_reihenfolge(),
        None,
        "vor dem Urteil darf sie niemand lesen"
    );

    // Ein zweiter Bindeversuch dreht sie nicht - sonst liesse sie sich nach
    // dem Hoeren noch zum Urteil passend erzaehlen.
    assert_eq!(
        s.binde_reihenfolge(&id, Blindreihenfolge::BaselineZuerst),
        Ok(false)
    );

    // Nach dem Urteil: aufgedeckt, und zwar genau die gebundene.
    s.ergebnis(&id, Hoerurteil::Baseline, None, None).unwrap();
    assert_eq!(
        s.experiment(&id).unwrap().aufgedeckte_reihenfolge(),
        Some(Blindreihenfolge::KandidatZuerst)
    );

    // Ohne Bindung gibt es kein Ergebnis.
    let (mut s2, id2) = store_mit_einem();
    assert_eq!(
        s2.ergebnis(&id2, Hoerurteil::Kandidat, None, None),
        Err(Abschlussfehler::ReihenfolgeNichtGebunden)
    );

    // Ein ABGEBROCHENER Versuch deckt sie nicht auf: es gab kein Urteil, also
    // gibt es nichts aufzudecken.
    let (mut s3, id3) = store_mit_einem();
    s3.binde_reihenfolge(&id3, Blindreihenfolge::BaselineZuerst).unwrap();
    s3.schliesse(&id3, Abbruchgrund::UserAbbruch).unwrap();
    assert!(s3.experiment(&id3).unwrap().reihenfolge_gebunden());
    assert_eq!(s3.experiment(&id3).unwrap().aufgedeckte_reihenfolge(), None);
}

// ═════════════════════════════════════════════════════════════════════════
// M-45 · four_axes_are_reported_separately
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn four_axes_are_reported_separately() {
    // Die vier Achsen sind vier Felder und es gibt KEINE Methode, die sie zu
    // einer Note verrechnet - die gaebe es nur, damit jemand sie benutzt.
    let a = Achsen {
        zielmetrik: Achsenbefund::Verbessert,
        guardrails: Achsenbefund::Verschlechtert,
        effektstabilitaet: Achsenbefund::Unveraendert,
        hoerurteil: Some(Hoerurteil::Baseline),
    };
    // Zielmetrik besser UND Guardrail schlechter ergibt eine EIGENE Aussage,
    // keine Verrechnung zu "unentschieden".
    assert_eq!(
        urteile(&a, true),
        Urteil::ZielVerbessertGeschuetztesSchlechter
    );

    // Das Hoerurteil ist Userdaten und geht in keine Messachse ein: dasselbe
    // Messbild mit anderem Hoerurteil ergibt dasselbe Urteil.
    let mut b = a;
    b.hoerurteil = Some(Hoerurteil::Kandidat);
    assert_eq!(urteile(&a, true), urteile(&b, true));
    let mut c = a;
    c.hoerurteil = None;
    assert_eq!(urteile(&a, true), urteile(&c, true));

    // `NichtBeurteilbar` ist die vierte, eigenstaendige Antwort - nicht ein
    // Zwischenwert zwischen besser und schlechter.
    let unklar = Achsen {
        zielmetrik: Achsenbefund::NichtBeurteilbar,
        guardrails: Achsenbefund::Unveraendert,
        effektstabilitaet: Achsenbefund::Unveraendert,
        hoerurteil: None,
    };
    assert_eq!(urteile(&unklar, true), Urteil::VergleichNichtGueltig);

    // Ein instabiler Effekt ist keine belastbare Aenderung, egal wie gross
    // er im Mittel aussieht.
    let instabil = Achsen {
        zielmetrik: Achsenbefund::Verbessert,
        guardrails: Achsenbefund::Unveraendert,
        effektstabilitaet: Achsenbefund::Verschlechtert,
        hoerurteil: None,
    };
    assert_eq!(urteile(&instabil, true), Urteil::KeineBelastbareAenderung);
}

#[test]
fn block_bootstrap_und_fdr_halten_ihre_zusagen() {
    // ── Block-Bootstrap ───────────────────────────────────────────────────
    // Deterministisch: dieselbe Saat, dasselbe Intervall. Ein Golden ueber
    // einen Zufallsprozess ist sonst keiner.
    // ⚠️ Zwei ungleiche Perioden (5 und 7), beide teilerfremd zur Blocklänge
    // 8. Bei Periode 8 hätte jeder Block denselben Mittelwert, jede Ziehung
    // dieselbe Zahl, und das Intervall wäre trivial null — der Golden wäre
    // grün und misste nichts. Dieselbe Folge steht in
    // `Sonde013ExperimentGoldenTest.cpp`.
    let deltas: Vec<f64> = (0..64)
        .map(|i| 1.0 + 0.1 * ((i % 5) as f64 - 2.0) + 0.05 * ((i % 7) as f64 - 3.0))
        .collect();
    let a = block_bootstrap(&deltas, 8, 400, 0.05, 42).expect("Intervall");
    let b = block_bootstrap(&deltas, 8, 400, 0.05, 42).expect("Intervall");
    assert_eq!(a, b);

    // ⚠️ DIESELBEN ZAHLEN wie `Sonde013ExperimentGoldenTest` (B25). Gen zeigt
    // die Statistik live, der Broker schreibt sie in den Store; beide
    // benutzen denselben Generator mit denselben Konstanten und dieselbe
    // Ziehungsreihenfolge. Ein Golden, der nur Eigenschaften prüft, liesse
    // die beiden Fassungen auseinanderdriften, ohne dass es auffiele.
    assert!(
        (a.0 - 0.979688).abs() < 1e-6 && (a.1 - 1.016406).abs() < 1e-6,
        "sprachuebergreifender Golden: {a:?} statt (0.979688, 1.016406)"
    );

    // Das Intervall enthaelt den Mittelwert.
    let mittel = deltas.iter().sum::<f64>() / deltas.len() as f64;
    assert!(a.0 <= mittel && mittel <= a.1, "{a:?} enthaelt {mittel} nicht");

    // ⚠️ Und es ist BREITER als eines aus Einzelwerten. Benachbarte Deltas
    // sind korreliert - dasselbe Material, dieselbe Kette, ueberlappende
    // Fenster. Ein Bootstrap ueber Einzelwerte nimmt Unabhaengigkeit an, die
    // es nicht gibt, und macht aus Rauschen eine belastbare Aenderung.
    let sprung: Vec<f64> = (0..64).map(|i| if (i / 8) % 2 == 0 { 2.0 } else { -2.0 }).collect();
    let block = block_bootstrap(&sprung, 8, 400, 0.05, 7).expect("block");
    let einzeln = block_bootstrap(&sprung, 1, 400, 0.05, 7).expect("einzeln");
    assert!(
        (block.1 - block.0) > (einzeln.1 - einzeln.0),
        "Block {block:?} muss breiter sein als Einzelwert {einzeln:?}"
    );

    // Ein groesseres alpha ergibt ein engeres Intervall.
    let eng = block_bootstrap(&deltas, 8, 400, 0.5, 42).expect("eng");
    assert!(
        (eng.1 - eng.0) < (a.1 - a.0),
        "alpha 0,5 muss enger sein als 0,05: {eng:?} gegen {a:?}"
    );

    // Leere Eingaben ergeben kein Intervall, statt eines um null.
    assert_eq!(block_bootstrap(&[], 8, 100, 0.05, 1), None);
    assert_eq!(block_bootstrap(&deltas, 0, 100, 0.05, 1), None);
    assert_eq!(block_bootstrap(&deltas, 8, 0, 0.05, 1), None);

    // ── FDR ───────────────────────────────────────────────────────────────
    // Bei 221 Baendern und alpha 0,05 waeren rund elf Fehlalarme zu erwarten,
    // wenn sich NICHTS geaendert hat. Gleichverteilte p-Werte sind genau
    // dieser Fall.
    let rein: Vec<f64> = (0..221).map(|i| (i as f64 + 0.5) / 221.0).collect();
    let sig = fdr_signifikant(&rein, 0.05);
    assert_eq!(
        sig.iter().filter(|s| **s).count(),
        0,
        "reines Rauschen darf kein einziges Band melden"
    );

    // Und ein echter Effekt kommt durch.
    let mut echt = rein.clone();
    for v in echt.iter_mut().take(20) {
        *v = 0.0001;
    }
    let sig2 = fdr_signifikant(&echt, 0.05);
    assert!(
        sig2.iter().filter(|s| **s).count() >= 20,
        "20 klare Treffer muessen durchkommen: {}",
        sig2.iter().filter(|s| **s).count()
    );

    // ── Cluster ───────────────────────────────────────────────────────────
    // Drei verstreute Einzelbaender sind eher ein Artefakt als eine
    // Klangaenderung; ein EQ-Eingriff wirkt auf benachbarte Baender.
    let verstreut = vec![true, false, true, false, true, false];
    assert!(cluster(&verstreut, 3).is_empty());
    let zusammen = vec![false, true, true, true, true, false];
    assert_eq!(cluster(&zusammen, 3), vec![(1, 5)]);
    // Auch am rechten Rand.
    let am_rand = vec![false, false, true, true, true];
    assert_eq!(cluster(&am_rand, 3), vec![(2, 5)]);
}

// ═════════════════════════════════════════════════════════════════════════
// M-46 · verdict_vocabulary_is_closed_and_keep_needs_user
//        gate6_incomparable_never_gets_a_strong_winner
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn verdict_vocabulary_is_closed_and_keep_needs_user() {
    // Der Raum aller Achsenkombinationen wird VOLLSTAENDIG abgefahren, und
    // jedes Ergebnis ist eine der fuenf Aussagen. "Objektiv besser" ist keine
    // davon, und es gibt keinen sechsten Wert, in den etwas fallen koennte.
    let werte = [
        Achsenbefund::Verbessert,
        Achsenbefund::Unveraendert,
        Achsenbefund::Verschlechtert,
        Achsenbefund::NichtBeurteilbar,
    ];
    let mut gesehen = std::collections::BTreeSet::new();
    let mut faelle = 0;
    for z in werte {
        for g in werte {
            for e in werte {
                for h in [None, Some(Hoerurteil::Baseline), Some(Hoerurteil::Enthaltung)] {
                    let a = Achsen {
                        zielmetrik: z,
                        guardrails: g,
                        effektstabilitaet: e,
                        hoerurteil: h,
                    };
                    let u = urteile(&a, true);
                    gesehen.insert(format!("{u:?}"));
                    faelle += 1;
                    // `Behalten` verlangt IMMER die Userbestaetigung.
                    assert!(!behalten_erlaubt(u, false), "{a:?} -> {u:?}");
                }
            }
        }
    }
    assert_eq!(faelle, 4 * 4 * 4 * 3);
    assert!(
        gesehen.len() >= 4,
        "die Aussagen werden tatsaechlich erreicht: {gesehen:?}"
    );

    // Mit Bestaetigung geht `Behalten` - ausser bei ungueltigem Vergleich.
    assert!(behalten_erlaubt(Urteil::ZielVerbessertGuardrailsStabil, true));
    assert!(behalten_erlaubt(Urteil::MessbarAndersUrteilOffen, true));
    assert!(
        !behalten_erlaubt(Urteil::VergleichNichtGueltig, true),
        "eine Bestaetigung ersetzt keine fehlende Grundlage"
    );
}

#[test]
fn gate6_incomparable_never_gets_a_strong_winner() {
    // Der harte Gate 6 aus §49.2: ein nicht vergleichbares Experiment erhaelt
    // KEIN starkes Siegerurteil - und zwar VOR allem anderen, denn ein Gate,
    // das erst nach der Auswertung greift, ist eine Empfehlung.
    let bestens = Achsen {
        zielmetrik: Achsenbefund::Verbessert,
        guardrails: Achsenbefund::Unveraendert,
        effektstabilitaet: Achsenbefund::Unveraendert,
        hoerurteil: Some(Hoerurteil::Kandidat),
    };
    assert_eq!(
        urteile(&bestens, true),
        Urteil::ZielVerbessertGuardrailsStabil
    );
    assert_eq!(urteile(&bestens, false), Urteil::VergleichNichtGueltig);
    assert!(!behalten_erlaubt(urteile(&bestens, false), true));
}

// ═════════════════════════════════════════════════════════════════════════
// M-47 · abort_writes_terminal_event_for_each_trigger
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn abort_writes_terminal_event_for_each_trigger() {
    // Genau ZWEI Ausloeser, und beide schreiben ein terminales Ereignis.
    for grund in [Abbruchgrund::UserAbbruch, Abbruchgrund::Verdraengt] {
        let (mut s, id) = store_mit_einem();
        assert!(s.schliesse(&id, grund).is_ok());
        let e = s.experiment(&id).unwrap();
        assert!(!e.offen());
        assert_eq!(e.terminal, Some(Terminal::Abgebrochen { grund }));
        assert!(s
            .log()
            .iter()
            .any(|ev| matches!(ev, Ereignis::Abgebrochen { grund: g, .. } if *g == grund)));

        // Ein zweites Terminalereignis wuerde das erste umdeuten.
        let mut s2 = s;
        assert_eq!(
            s2.schliesse(&id, Abbruchgrund::UserAbbruch),
            Err(Abschlussfehler::SchonTerminal)
        );
    }

    // Und der andere Weg: `manual_result` ist ebenfalls terminal.
    let (mut s, id) = store_mit_einem();
    s.binde_reihenfolge(&id, Blindreihenfolge::BaselineZuerst).unwrap();
    s.ergebnis(&id, Hoerurteil::KeinUnterschied, None, None).unwrap();
    assert!(!s.experiment(&id).unwrap().offen());
    assert_eq!(
        s.schliesse(&id, Abbruchgrund::UserAbbruch),
        Err(Abschlussfehler::SchonTerminal)
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-48 · open_cap_per_project_at_n_and_n_plus_one
//        open_cap_global_at_n_and_n_plus_one
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn open_cap_per_project_at_n_and_n_plus_one() {
    let mut s = Experimentstore::neu();
    for i in 0..N_PROJEKT {
        s.beginne(&hex32(i as u32), "projekt-a", passage(i as u32), referenz(-1.0))
            .expect("beginne");
    }
    assert_eq!(s.offene_im_projekt("projekt-a").count(), N_PROJEKT, "genau N");
    assert!(s.experiment(&hex32(0)).unwrap().offen(), "das aelteste lebt noch");

    // N+1: das AELTESTE wird mit `verdraengt` geschlossen, nicht das neueste
    // abgelehnt - der User soll weiterarbeiten koennen.
    s.beginne(
        &hex32(N_PROJEKT as u32),
        "projekt-a",
        passage(99),
        referenz(-1.0),
    )
    .expect("N+1");
    assert_eq!(s.offene_im_projekt("projekt-a").count(), N_PROJEKT);
    let aeltestes = s.experiment(&hex32(0)).unwrap();
    assert_eq!(
        aeltestes.terminal,
        Some(Terminal::Abgebrochen {
            grund: Abbruchgrund::Verdraengt
        })
    );
    assert!(s.experiment(&hex32(N_PROJEKT as u32)).unwrap().offen());

    // Ein anderes Projekt hat seinen EIGENEN Deckel - ein volles Projekt
    // verdraengt keine fremden Zeilen.
    s.beginne(&hex32(500), "projekt-b", passage(500), referenz(-1.0))
        .expect("anderes Projekt");
    assert_eq!(s.offene_im_projekt("projekt-b").count(), 1);
    assert_eq!(s.offene_im_projekt("projekt-a").count(), N_PROJEKT);
}

#[test]
fn open_cap_global_at_n_and_n_plus_one() {
    let mut s = Experimentstore::neu();
    // Auf so viele Projekte verteilt, dass der Bereichsdeckel nie greift.
    let projekte = N_GLOBAL / (N_PROJEKT - 1) + 1;
    let mut angelegt = 0u32;
    'aussen: for p in 0..projekte {
        for _ in 0..(N_PROJEKT - 1) {
            if angelegt as usize >= N_GLOBAL {
                break 'aussen;
            }
            s.beginne(
                &hex32(angelegt),
                &format!("projekt-{p}"),
                passage(angelegt),
                referenz(-1.0),
            )
            .expect("beginne");
            angelegt += 1;
        }
    }
    assert_eq!(s.offene().count(), N_GLOBAL, "genau N global");
    assert!(s.experiment(&hex32(0)).unwrap().offen());

    s.beginne(&hex32(9000), "projekt-neu", passage(9000), referenz(-1.0))
        .expect("N+1 global");
    assert_eq!(s.offene().count(), N_GLOBAL);
    assert_eq!(
        s.experiment(&hex32(0)).unwrap().terminal,
        Some(Terminal::Abgebrochen {
            grund: Abbruchgrund::Verdraengt
        })
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-49 · manual_result_writes_terminal_event_and_deltas
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn manual_result_writes_terminal_event_and_deltas() {
    let (mut s, id) = store_mit_einem();
    s.binde_reihenfolge(&id, Blindreihenfolge::KandidatZuerst).unwrap();
    s.ergebnis(&id, Hoerurteil::Kandidat, Some("lauter".into()), None)
        .unwrap();

    // Das Ergebnis traegt NUR, was das Schema erlaubt: Hoerurteil,
    // aufgedeckte Reihenfolge, Notiz, Werkzeug. Keine State-Hashes, keine
    // Candidate-Revision, keinen Match-Gain (M-49).
    let e = s.experiment(&id).unwrap();
    let Some(Terminal::Ergebnis {
        hoerurteil,
        blindreihenfolge,
        notiz,
        werkzeug,
    }) = &e.terminal
    else {
        panic!("Ergebnis erwartet");
    };
    assert_eq!(*hoerurteil, Hoerurteil::Kandidat);
    assert_eq!(*blindreihenfolge, Blindreihenfolge::KandidatZuerst);
    assert_eq!(notiz.as_deref(), Some("lauter"));
    assert_eq!(*werkzeug, None);

    // Der Match-Gain steht in der REFERENZ, nicht im Ergebnis - der Broker
    // rechnet die Deltas aus dem Store, Gen schickt sie nicht.
    assert_eq!(e.baseline.match_gain_db, -2.5);

    // Und das Ereignis steht append-only im Log.
    assert!(s.log().iter().any(|ev| matches!(ev, Ereignis::Ergebnis { .. })));
}

// ═════════════════════════════════════════════════════════════════════════
// M-50 · experiment_survives_restart_without_silent_continuation
// M-32 · passage_survives_restart_and_missing_db_degrades_gracefully
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn experiment_survives_restart_without_silent_continuation() {
    let (mut s, id) = store_mit_einem();
    s.neuer_kandidat(&id, referenz(-1.0)).unwrap();
    s.binde_reihenfolge(&id, Blindreihenfolge::BaselineZuerst).unwrap();

    // Der "Neustart": ein neuer Store, aus dem Ereignislog und den Referenzen
    // wiederhergestellt. Der Test misst die REKONSTRUKTIONSFORM - dass jedes
    // Feld, das nach dem Neustart gebraucht wird, auch gespeichert ist.
    let export = s.exportiere(&id).expect("Export");
    assert_eq!(export.experiment.baseline.match_gain_db, -2.5);
    assert_eq!(export.experiment.kandidaten.len(), 1);
    assert_eq!(export.passage.transport_epoch, 7);
    assert!(export.experiment.offen(), "der offene Zustand reist mit");

    // KEINE stillschweigende Fortsetzung: die gebundene Reihenfolge bleibt
    // gebunden UND verborgen, und es gibt kein Ergebnis, das aus dem Nichts
    // entstuende.
    assert!(export.experiment.reihenfolge_gebunden());
    assert_eq!(export.experiment.aufgedeckte_reihenfolge(), None);
    assert_eq!(export.experiment.terminal, None);

    // Sitzungsende, Projektwechsel, Reconnect, UI- und Brokerneustart brechen
    // NICHT ab (M-47): es gibt in diesem Typ gar keinen Weg, ein Terminal
    // ohne einen der zwei Gruende zu schreiben.
    assert!(s.experiment(&id).unwrap().offen());
}

#[test]
fn passage_survives_restart_and_missing_db_degrades_gracefully() {
    let (s, id) = store_mit_einem();
    let export = s.exportiere(&id).expect("Export");
    // Die Passage reist VOLLSTAENDIG mit, nicht nur ihre ID: ein Export, der
    // auf einen Store zeigt, den der Empfaenger nicht hat, ist keiner.
    assert_eq!(export.passage.passage_id, s.experiment(&id).unwrap().passage_id);
    assert_eq!(export.passage.fingerprint, fp(5));
    assert_eq!(export.passage.aktive_quellen.len(), 2);

    // Fehlt die Datenbank ganz, bleibt Projekt-Recall vollstaendig - nur
    // historische Passagen und Experimente fehlen (M-32). Ein leerer Store
    // liefert deshalb `None` statt zu paniken oder etwas zu erfinden.
    let leer = Experimentstore::neu();
    assert!(leer.experiment(&id).is_none());
    assert!(leer.passage(&hex32(1001)).is_none());
    assert!(leer.exportiere(&id).is_none());
    assert_eq!(leer.offene().count(), 0);

    // Und ein alter Eintrag setzt keinen neueren zurueck: `beginne` mit einer
    // vergebenen ID wird abgelehnt, statt zu ueberschreiben.
    let mut s2 = s;
    assert_eq!(
        s2.beginne(&id, "projekt-a", passage(1), referenz(-99.0)),
        Err(Anlegefehler::IdVergeben)
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-51 · export_is_complete_and_delete_leaves_no_pcm
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn export_is_complete_and_delete_leaves_no_pcm() {
    let (mut s, id) = store_mit_einem();
    s.neuer_kandidat(&id, referenz(-1.0)).unwrap();
    s.binde_reihenfolge(&id, Blindreihenfolge::BaselineZuerst).unwrap();
    s.ergebnis(&id, Hoerurteil::Baseline, Some("dumpfer".into()), None)
        .unwrap();

    let export = s.exportiere(&id).expect("Export");
    // Vollstaendig INKLUSIVE Evidenz-IDs.
    assert_eq!(export.experiment.experiment_id, id);
    assert_eq!(export.passage.passage_id, export.experiment.passage_id);
    assert!(!export.ereignisse.is_empty());
    // Und nur die Ereignisse DIESES Experiments.
    let (mut s2, id2) = (Experimentstore::neu(), hex32(2));
    s2.beginne(&id2, "projekt-b", passage(2), referenz(0.0)).unwrap();
    let fremd = s2.exportiere(&id2).unwrap();
    assert_eq!(fremd.ereignisse.len(), 1);

    // Kein PCM - und das ist keine Behauptung ueber die Speicherform, sondern
    // ueber den Fingerprint: 76 Byte fuer Sekunden Audio (§10.6), und alle
    // drei Verlaeufe sind Energien ohne Phase.
    let f = &export.passage.fingerprint;
    assert_eq!(f.band_energie.len() + f.chroma.len() + f.onset.len(), 76);

    // Loeschen ist der Gegenpfad zum Anlegen und gehoert in denselben
    // Aenderungssatz (CLAUDE.md).
    assert!(s.loesche(&id));
    assert!(s.experiment(&id).is_none());
    assert!(s.exportiere(&id).is_none());
    assert!(!s.loesche(&id), "zweimal loeschen ist kein Fehler, aber auch kein Erfolg");

    // Die PASSAGE bleibt: sie kann weitere Experimente tragen, und sie
    // mitzuloeschen hiesse, fremde Zeilen mitzunehmen.
    assert!(s.passage(&hex32(1001)).is_some());

    // Das Log behaelt seine Eintraege - es ist append-only, und ein
    // geloeschtes Experiment ist eine Tatsache ueber die Vergangenheit,
    // keine Luecke darin.
    assert!(s.log().iter().any(|ev| matches!(ev, Ereignis::Ergebnis { .. })));
}
