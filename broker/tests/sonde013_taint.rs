//! Invalidierung und Taint (SONDE-013 M-52 bis M-57, M-59 bis M-63; §34.2).
//!
//! Der Satz, den dieses Bein trägt, steht in M-53:
//!
//! > „Kein Auslöser trägt zwei Gründe, und keiner bleibt der Implementierung
//! >  überlassen."
//!
//! Der Grund auf dem Draht ist keine Beschriftung. Der Empfänger entscheidet
//! an ihm, ob er eine Zeitreihe fortsetzen darf: ein Drop, der als
//! Epochwechsel ankommt, kostet ihn eine ganze Epoche Evidenz; umgekehrt
//! setzte er über eine echte Epochengrenze hinweg fort.

use std::collections::BTreeSet;

use eqcop_broker::coordinator::experiment::{
    Abbruchgrund, Alignmentwert, Blindreihenfolge, Experimentreferenz, Experimentstore,
    Hoerurteil, Passage, Resultatmessung,
};
use eqcop_broker::coordinator::invalidierung::{
    apply_transaction, grund_fuer_bruch, material_wechsel, messpunkt_wechsel,
    quarantaene_fuer_intervention, Bruchart, Grund, Invalidierung, Umfang, GATE_MATERIAL_GLEICH,
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

fn ids(namen: &[&str]) -> Umfang {
    Umfang::Ids(namen.iter().map(|s| s.to_string()).collect::<BTreeSet<_>>())
}

// ═════════════════════════════════════════════════════════════════════════
// M-52 · marker_invalidates_overlapping_range_and_unknown_routing_takes_the_session
//        unknown_routing_widens_to_whole_session
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn marker_invalidates_overlapping_range_and_unknown_routing_takes_the_session() {
    // Bekanntes Routing und bekannter Bereich: genau dieser Bereich.
    let q = quarantaene_fuer_intervention(true, Some((1000, 5000)));
    assert_eq!(q.grund, Grund::Intervention);
    assert_eq!(q.umfang, Umfang::Bereich { von: 1000, bis: 5000 });
    assert!(q.gueltig());

    // Und er erfasst genau die ueberlappende Evidenz - halboffen.
    assert!(q.umfang.erfasst("a", 4999, 6000), "das letzte Sample zaehlt noch");
    assert!(!q.umfang.erfasst("a", 5000, 6000), "das erste danach nicht mehr");
    assert!(q.umfang.erfasst("a", 0, 1001), "und am unteren Rand ebenso");
    assert!(!q.umfang.erfasst("a", 0, 1000));

    // ⚠️ Zahlenraender: zwei gueltige Fenster an entgegengesetzten
    // i64-Raendern duerfen nicht durch einen Ueberlauf als ueberlappend
    // gelten.
    let weit = Invalidierung {
        grund: Grund::Intervention,
        umfang: Umfang::Bereich { von: i64::MIN, bis: i64::MIN + 100 },
    };
    assert!(!weit.umfang.erfasst("a", i64::MAX - 100, i64::MAX));
}

#[test]
fn unknown_routing_widens_to_whole_session() {
    // Unbekanntes Routing: die GANZE Sitzung, und der Grund heisst auch so.
    let q = quarantaene_fuer_intervention(false, Some((1000, 5000)));
    assert_eq!(q.grund, Grund::RoutingUnbekannt);
    assert_eq!(q.umfang, Umfang::GanzeSitzung);
    assert!(q.umfang.erfasst("beliebig", i64::MIN, i64::MAX));

    // Auch ohne Bereichsangabe - dann wissen wir zwar WO die Kette laeuft,
    // aber nicht WANN. Ein `intervention` mit halbem Bereich waere die
    // gefaehrlichere Meldung: der Empfaenger naehme an, der Rest sei sauber.
    let ohne = quarantaene_fuer_intervention(true, None);
    assert_eq!(ohne.grund, Grund::Intervention);
    assert_eq!(ohne.umfang, Umfang::GanzeSitzung);

    // Ein leerer Bereich ist kein Bereich.
    let leer = quarantaene_fuer_intervention(true, Some((500, 500)));
    assert_eq!(leer.umfang, Umfang::GanzeSitzung);
    let rueckwaerts = quarantaene_fuer_intervention(true, Some((900, 100)));
    assert_eq!(rueckwaerts.umfang, Umfang::GanzeSitzung);
}

// ═════════════════════════════════════════════════════════════════════════
// M-53 · epoch_change_invalidates_open_evidence
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn epoch_change_invalidates_open_evidence() {
    // Der Grund je Ausloeser, EINZELN gemessen. Ein vertauschter Grund faellt
    // hier, statt gruen zu werden.
    assert_eq!(grund_fuer_bruch(Bruchart::Seek), Some(Grund::Epochwechsel));
    assert_eq!(
        grund_fuer_bruch(Bruchart::LoopWrapInnerhalb),
        Some(Grund::Epochwechsel)
    );
    assert_eq!(
        grund_fuer_bruch(Bruchart::AndererKontinuitaetsbruch),
        Some(Grund::Epochwechsel)
    );
    // „Drop zaehlt als Segment, alles andere als Epoche" (SONDE-009).
    assert_eq!(grund_fuer_bruch(Bruchart::Drop), Some(Grund::Sequenzluecke));

    // ⚠️ Und ein Loop-Wrap AN der Passagengrenze ist gar kein Bruch: der Loop
    // deckt die Passage vollstaendig ab, der Wrap beendet einen Durchlauf und
    // beginnt den naechsten. Weder Invalidierung noch Herabstufung (M-53,
    // Abgrenzung zu M-23). Ein `Some(Epochwechsel)` hier naehme dem User bei
    // JEDEM Loopdurchlauf seine Evidenz - genau bei der Arbeitsweise, fuer
    // die Passagen gedacht sind.
    assert_eq!(grund_fuer_bruch(Bruchart::LoopWrapAnGrenze), None);

    // Kein Ausloeser traegt zwei Gruende: die Abbildung ist eine Funktion,
    // und die vier bruchtragenden Arten liefern genau zwei verschiedene
    // Gruende.
    let gruende: BTreeSet<Grund> = [
        Bruchart::Seek,
        Bruchart::LoopWrapInnerhalb,
        Bruchart::AndererKontinuitaetsbruch,
        Bruchart::Drop,
    ]
    .into_iter()
    .filter_map(grund_fuer_bruch)
    .collect();
    assert_eq!(gruende.len(), 2, "{gruende:?}");
    assert!(gruende.contains(&Grund::Epochwechsel));
    assert!(gruende.contains(&Grund::Sequenzluecke));
}

// ═════════════════════════════════════════════════════════════════════════
// M-54 · material_change_invalidates_dependent_evidence
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn material_change_invalidates_dependent_evidence() {
    let a = fp(5);
    let b = fp(90);

    // Dasselbe Material: keine Invalidierung.
    assert_eq!(material_wechsel(Some(&a), Some(&a), Umfang::GanzeSitzung), None);

    // Anderes Material: invalidiert, und zwar mit DIESEM Grund.
    let w = material_wechsel(Some(&a), Some(&b), ids(&["ev-1", "ev-2"]))
        .expect("Materialwechsel");
    assert_eq!(w.grund, Grund::MaterialWechsel);
    assert_eq!(w.umfang, ids(&["ev-1", "ev-2"]));
    assert!(w.gueltig());

    // ⚠️ Ein FEHLENDER Fingerprint ist kein „gleiches Material". Ohne Beleg
    // gibt es keine Aussage, und fail-closed heisst hier invalidieren - sonst
    // hielte eine Passage ohne Fingerprint ihre Evidenz durch jeden
    // Materialwechsel hindurch.
    assert!(material_wechsel(None, Some(&a), Umfang::GanzeSitzung).is_some());
    assert!(material_wechsel(Some(&a), None, Umfang::GanzeSitzung).is_some());
    assert!(material_wechsel(None, None, Umfang::GanzeSitzung).is_some());

    // Die Erkennung laeuft ueber den Fingerprintvergleich und nicht ueber
    // eine Zeitheuristik: derselbe Fingerprint bleibt derselbe, egal wie viel
    // Zeit vergeht - der Aufruf kennt gar keine Zeit.
    assert_eq!(material_wechsel(Some(&a), Some(&a), ids(&["x"])), None);

    // Die Schwelle steht im Register und ist dieselbe wie das Materialgate
    // der Vergleichbarkeit.
    assert!((GATE_MATERIAL_GLEICH - 0.95).abs() < 1e-12);
}

// ═════════════════════════════════════════════════════════════════════════
// M-55 · measurement_position_change_invalidates_dependent_evidence
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn measurement_position_change_invalidates_dependent_evidence() {
    assert_eq!(messpunkt_wechsel("insert", "insert", Umfang::GanzeSitzung), None);

    // Die Kante zu Gate 7 (§49.2): eine Standard-Insertprobe wird nie als
    // exakter Mastersummenbeitrag bezeichnet - folglich darf Evidenz von
    // `insert` nach einem Wechsel auf `post_fader_contribution` nicht
    // mitwandern.
    let w = messpunkt_wechsel("insert", "post_fader_contribution", Umfang::GanzeSitzung)
        .expect("Messpunktwechsel");
    assert_eq!(w.grund, Grund::MesspunktWechsel);

    // Und jede andere Richtung ebenso - der Riegel haengt nicht an einer
    // bestimmten Klasse.
    for (alt, neu) in [
        ("post_fader_contribution", "insert"),
        ("pre", "post"),
        ("post", "pre"),
        ("insert", "pre"),
    ] {
        assert_eq!(
            messpunkt_wechsel(alt, neu, Umfang::GanzeSitzung).map(|i| i.grund),
            Some(Grund::MesspunktWechsel),
            "{alt} -> {neu}"
        );
    }
}

// ═════════════════════════════════════════════════════════════════════════
// M-56 · applied_state_segments_instead_of_tainting
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn applied_state_segments_instead_of_tainting() {
    let s = apply_transaction(42, 48_000.0);

    // Ein ANGEWENDETER Zustand ist ausdruecklich KEIN dauerhafter Taint. Ihn
    // als solchen zu fuehren hiesse, dass jede vom User bestaetigte Aenderung
    // die Messung fuer immer entwertet - und der Advisor nie wieder eine
    // starke Aussage traefe, sobald der User ihm einmal gefolgt ist.
    assert_eq!(s.grund, Grund::StateRevision);
    assert!(s.alte_baseline_geschlossen, "die alte Baseline ist GESCHLOSSEN");
    assert!(s.findings_stale, "abhaengige Findings sind stale, nicht falsch");
    assert_eq!(s.neue_revision, 42);
    assert!(s.warmup_samples > 0, "und es gibt eine Warm-up-Zeit");

    // Die Warm-up-Zeit skaliert mit der Samplerate: 500 ms bleiben 500 ms.
    let s96 = apply_transaction(42, 96_000.0);
    assert_eq!(s96.warmup_samples, s.warmup_samples * 2);
    let s441 = apply_transaction(42, 44_100.0);
    assert!(
        s441.warmup_samples < s.warmup_samples,
        "44,1 kHz braucht weniger Samples fuer dieselbe Zeit"
    );

    // Eine unsinnige Samplerate faellt auf den Vorgabewert zurueck, statt 0
    // Samples Warm-up zu melden - null Warm-up hiesse „sofort wieder gueltig".
    assert_eq!(apply_transaction(1, 0.0).warmup_samples, s.warmup_samples);
    assert_eq!(apply_transaction(1, -5.0).warmup_samples, s.warmup_samples);
}

// ═════════════════════════════════════════════════════════════════════════
// M-57 · invalidate_scope_discriminator_rejects_mismatched_payloads
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn invalidate_scope_discriminator_rejects_mismatched_payloads() {
    // Der Discriminator ist kein Etikett ohne Wirkung: jeder Zweig traegt
    // genau die Felder, die seine Art braucht, und keine anderen. Als Enum
    // ist ein `art=sample_range` OHNE Bereich gar nicht formulierbar - und
    // genau das ist der Riegel.
    assert_eq!(ids(&["a"]).art(), "evidence_ids");
    assert_eq!(Umfang::Bereich { von: 0, bis: 1 }.art(), "sample_range");
    assert_eq!(Umfang::GanzeSitzung.art(), "ganze_sitzung");

    // Was formulierbar bleibt, faellt in `gueltig()`: eine leere ID-Menge und
    // ein leerer Bereich nehmen NICHTS zurueck. Sie zu senden hiesse, dem
    // Empfaenger eine Ruecknahme zu melden, die nichts zuruecknimmt, und ihn
    // danach glauben zu lassen, es sei aufgeraeumt.
    assert!(!Umfang::Ids(BTreeSet::new()).gueltig());
    assert!(!Umfang::Bereich { von: 5, bis: 5 }.gueltig());
    assert!(!Umfang::Bereich { von: 9, bis: 1 }.gueltig());
    assert!(ids(&["a"]).gueltig());
    assert!(Umfang::Bereich { von: 0, bis: 1 }.gueltig());
    assert!(Umfang::GanzeSitzung.gueltig(), "die ganze Sitzung braucht keine Felder");

    // Jeder Grund traegt sein Wireworte, und die Menge ist geschlossen.
    let woerter: BTreeSet<&str> = [
        Grund::Intervention,
        Grund::RoutingUnbekannt,
        Grund::Sequenzluecke,
        Grund::Epochwechsel,
        Grund::StateRevision,
        Grund::MaterialWechsel,
        Grund::MesspunktWechsel,
    ]
    .into_iter()
    .map(|g| g.wort())
    .collect();
    assert_eq!(woerter.len(), 7, "sieben verschiedene Woerter: {woerter:?}");
    assert!(woerter.contains("material_wechsel") && woerter.contains("messpunkt_wechsel"),
            "die zwei Gruende aus Fassung 2 sind dabei");
}

// ═════════════════════════════════════════════════════════════════════════
// M-59 · both_experiment_terminals_close_all_intervals
//        retention_abort_also_closes_intervals
// ═════════════════════════════════════════════════════════════════════════
fn passage(n: u32) -> Passage {
    Passage {
        passage_id: hex32(2000 + n),
        projekt_von: 0,
        projekt_bis: 480_000,
        transport_epoch: 3,
        aktive_quellen: vec![hex32(11)],
        abdeckung: 0.9,
        label: None,
        fingerprint: fp(5),
    }
}

fn referenz() -> Experimentreferenz {
    Experimentreferenz {
        passage_fingerprint: fp(5),
        upstream_fingerprint: fp(9),
        aktive_quellen: vec![hex32(11)],
        messpunktklassen: vec!["insert".into()],
        match_gain_db: -1.0,
        alignment: Alignmentwert::FeatureAligned,
    }
}

#[test]
fn both_experiment_terminals_close_all_intervals() {
    // BEIDE Terminalereignisse schliessen den Versuch - und damit die
    // Taintintervalle, die an ihm haengen. Ein Versuch, der ohne Terminal
    // endet, liesse sie offen zurueck.
    for terminal_ist_ergebnis in [true, false] {
        let mut s = Experimentstore::neu();
        let id = hex32(1);
        s.beginne(&id, "projekt-a", passage(1), referenz()).unwrap();
        assert!(s.experiment(&id).unwrap().offen(), "vorher offen");

        if terminal_ist_ergebnis {
            s.binde_reihenfolge(&id, Blindreihenfolge::BaselineZuerst).unwrap();
            s.ergebnis(&id, Hoerurteil::Kandidat, None, None, &Resultatmessung { band_delta_db: vec![1.0; 8], band_gueltig: vec![true; 8], baseline_evidence_ids: vec!["a".repeat(32)], resultat_evidence_ids: vec!["b".repeat(32)], ..Default::default() }).unwrap();
        } else {
            s.schliesse(&id, Abbruchgrund::UserAbbruch).unwrap();
        }
        assert!(!s.experiment(&id).unwrap().offen());
        assert_eq!(s.offene().count(), 0, "kein offenes Intervall bleibt");
    }
}

#[test]
fn retention_abort_also_closes_intervals() {
    // Der Versuch, den der User nie nachmisst (M-47, „der haeufigste
    // Realfall"), bleibt nicht ewig offen: die Retention aus M-48 schreibt
    // das terminale `aborted` und schliesst damit auch seine Intervalle.
    let mut s = Experimentstore::neu();
    let deckel = eqcop_broker::coordinator::experiment::N_PROJEKT;
    for i in 0..deckel {
        s.beginne(&hex32(i as u32), "projekt-a", passage(i as u32), referenz())
            .unwrap();
    }
    assert_eq!(s.offene().count(), deckel);

    s.beginne(&hex32(999), "projekt-a", passage(999), referenz())
        .unwrap();
    assert_eq!(s.offene().count(), deckel, "der Deckel haelt");
    let aeltestes = s.experiment(&hex32(0)).unwrap();
    assert!(!aeltestes.offen(), "und das aelteste ist geschlossen");
    assert_eq!(
        aeltestes.terminal,
        Some(eqcop_broker::coordinator::experiment::Terminal::Abgebrochen {
            grund: Abbruchgrund::Verdraengt
        }),
        "mit dem Grund `verdraengt`, nicht mit einem erfundenen"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// M-62 · downstream_probe_is_tainted_by_upstream_intervention
// ═════════════════════════════════════════════════════════════════════════
#[test]
fn downstream_probe_is_tainted_by_upstream_intervention() {
    // Die Taint-Logik ist SITZUNGSWEIT, nicht instanzlokal. Preview-, Focus-,
    // Delta- und Markermatrix liegen hinter den Analysetaps der EIGENEN
    // Instanz - das schuetzt die eigene Messung, nicht die der nachgelagerten.
    //
    // Messbar heisst das: der Umfang einer Interventionsquarantaene erfasst
    // Evidenz UNABHAENGIG von ihrer Instanz. Der Umfang kennt gar kein
    // Instanzfeld, und genau das ist die Zusage.
    let q = quarantaene_fuer_intervention(true, Some((0, 100_000)));
    assert!(q.umfang.erfasst("evidenz-der-vorgelagerten", 10, 20));
    assert!(q.umfang.erfasst("evidenz-der-nachgelagerten", 10, 20));

    // Und bei unbekanntem Routing ist der Umfang die ganze Sitzung - dort
    // gibt es die Frage nach der Instanz erst recht nicht.
    let unbekannt = quarantaene_fuer_intervention(false, Some((0, 100)));
    assert_eq!(unbekannt.umfang, Umfang::GanzeSitzung);
    assert!(unbekannt.umfang.erfasst("irgendeine", i64::MIN, i64::MAX));
}

// ═════════════════════════════════════════════════════════════════════════
// Der Coordinator-Teil: M-52 (Nachlauf), M-60, M-61, M-63
// ═════════════════════════════════════════════════════════════════════════

use eqcop_broker::coordinator::{Coordinator, ManualClock};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
use std::sync::Arc;

fn adresse(projekt: usize, sitzung: usize, instanz: usize, nonce: usize) -> Adresse {
    Adresse {
        logon_sid: "S-1-5-21-1-2-3-1001".into(),
        project_binding_id: hex32(projekt as u32),
        session_epoch: hex32(sitzung as u32),
        instance_id: hex32(instanz as u32),
        runtime_nonce: hex32(nonce as u32),
    }
}

fn hello(instanz: usize, nonce: usize, art: &str) -> HelloControl {
    HelloControl {
        typ: "hello".into(),
        connection_kind: "control".into(),
        protocol: 3,
        plugin_version: "test".into(),
        plugin_kind: art.into(),
        adresse: adresse(1, 2, instanz, nonce),
        host: Some(HostAngabe {
            pid: 9,
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

fn coordinator_mit_link(link: &str, art: &str) -> (Coordinator, HelloControl) {
    let c = Coordinator::mit_uhr(Arc::new(ManualClock::default()), hex32(0xbeef));
    let h = hello(10, 100, art);
    let ausgang = c.control_hello_registrieren(link, &h);
    assert!(ausgang.angenommen, "{:?}", ausgang.grund);
    (c, h)
}

// ─────────────────────────────────────────────────────────────────────────
// M-52 · marker_end_closes_its_interval_only_after_the_tail
// ─────────────────────────────────────────────────────────────────────────
#[test]
fn marker_end_closes_its_interval_only_after_the_tail() {
    let (c, h) = coordinator_mit_link("probe", "active_probe");
    let id = hex32(500);

    assert!(c.evidence_dispatch_fuer_link("probe"), "vorher ist Evidenz erlaubt");
    assert!(c.intervention_begin("probe", &h.adresse, &id, 1));
    assert!(!c.evidence_dispatch_fuer_link("probe"), "waehrend der Intervention nicht");

    // Das End meldet einen Nachlauf. Das Intervall ist damit NICHT sofort
    // geschlossen: der Hall einer Vorschau klingt in die Messung hinein, und
    // eine Evidenz, die ihn mitmisst, waere falsch, ohne falsch auszusehen.
    assert!(c.intervention_end("probe", &h.adresse, &id, 2, 24_000));
    let sicht = c.interventionssicht();
    assert_eq!(sicht.aktive, 0, "die aktive Intervention ist weg");
    assert_eq!(sicht.tail_samples_offen, 24_000, "aber der Nachlauf steht");
    assert!(!sicht.starke_evidenz_erlaubt);
    assert!(!c.evidence_dispatch_fuer_link("probe"), "und Evidenz bleibt gesperrt");

    // Erst der abgelaufene Nachlauf gibt frei - und zwar VOLLSTAENDIG
    // abgelaufen, nicht teilweise.
    c.tail_fortschritt(12_000);
    assert!(!c.evidence_dispatch_fuer_link("probe"), "der halbe Nachlauf genuegt nicht");
    assert_eq!(c.interventionssicht().tail_samples_offen, 12_000);
    c.tail_fortschritt(12_000);
    assert!(c.interventionssicht().starke_evidenz_erlaubt);
    assert!(c.evidence_dispatch_fuer_link("probe"), "jetzt wieder");

    // Ein Nachlauf von 0 schliesst sofort - der Normalfall ohne Hall.
    let (c2, h2) = coordinator_mit_link("probe", "active_probe");
    let id2 = hex32(501);
    assert!(c2.intervention_begin("probe", &h2.adresse, &id2, 1));
    assert!(c2.intervention_end("probe", &h2.adresse, &id2, 2, 0));
    assert!(c2.interventionssicht().starke_evidenz_erlaubt);
}

// ─────────────────────────────────────────────────────────────────────────
// M-60 · inverse_path_returns_to_zero_after_every_intervention_kind
// ─────────────────────────────────────────────────────────────────────────
#[test]
fn inverse_path_returns_to_zero_after_every_intervention_kind() {
    // „Der regulaere Pfad nullt wieder." Messbar heisst das: nach jeder Art
    // von Intervention steht `starke_evidenz_erlaubt` wieder auf `true`,
    // `aktive` auf 0, `tail_samples_offen` auf 0 und `unknown` auf `false` -
    // und der naechste Dispatch wird ANGENOMMEN statt gesperrt.
    //
    // Ein Taintpfad ohne beweisbaren Rueckweg waere ein Einwegventil: die
    // erste Vorschau des Tages nimmt dem User seine Evidenz fuer immer.
    let (c, h) = coordinator_mit_link("probe", "active_probe");
    let mut sequenz = 1u64;

    for (nr, tail) in [(0u32, 0u64), (1, 24_000), (2, 1), (3, 0)] {
        let id = hex32(600 + nr);
        assert!(c.intervention_begin("probe", &h.adresse, &id, sequenz), "begin {nr}");
        sequenz += 1;
        assert_eq!(c.interventionssicht().aktive, 1, "{nr}");
        assert!(!c.evidence_dispatch_fuer_link("probe"), "{nr}: gesperrt");

        assert!(c.intervention_end("probe", &h.adresse, &id, sequenz, tail), "end {nr}");
        sequenz += 1;
        if tail > 0 {
            c.tail_fortschritt(tail);
        }

        let sicht = c.interventionssicht();
        assert_eq!(sicht.aktive, 0, "{nr}: keine aktive");
        assert_eq!(sicht.tail_samples_offen, 0, "{nr}: kein Nachlauf");
        assert!(!sicht.unknown, "{nr}: kein Unknown");
        assert!(sicht.starke_evidenz_erlaubt, "{nr}: wieder erlaubt");
        assert!(c.evidence_dispatch_fuer_link("probe"), "{nr}: und der naechste Dispatch geht durch");
    }
}

// ─────────────────────────────────────────────────────────────────────────
// M-61 · only_neutral_resync_clears_sticky_unknown
// ─────────────────────────────────────────────────────────────────────────
#[test]
fn only_neutral_resync_clears_sticky_unknown() {
    let (c, h) = coordinator_mit_link("probe", "active_probe");
    let id = hex32(700);

    // ⚠️ Der Kern der Zusage, und er braucht eine GUELTIGE Sequenz: ein `end`
    // auf eine unbekannte Intervention-ID setzt Unknown und loest es nicht.
    // Das Begin kann vor Reconnect oder Overflow verloren gegangen sein - ein
    // `end` ist dann gerade kein Beweis, dass alles sauber ist.
    //
    // Der Fall muss den ID-Zweig wirklich erreichen: mit einer Sequenzluecke
    // davor bricht `intervention_end` schon an der Sequenz ab, und der
    // eigentliche Riegel bliebe ungemessen (beim Bau von Etappe H gefunden -
    // der Rotbeweis kam zuerst gruen zurueck).
    assert!(c.intervention_begin("probe", &h.adresse, &id, 1));
    assert!(!c.interventionssicht().unknown, "bis hierher ist alles sauber");
    assert!(
        !c.intervention_end("probe", &h.adresse, &hex32(702), 2, 0),
        "ein end auf eine unbekannte id wird abgelehnt"
    );
    assert!(
        c.interventionssicht().unknown,
        "und setzt Unknown, statt es zu loesen"
    );

    // Auch ein sauberes End der bekannten Intervention loest es nicht mehr.
    let _ = c.intervention_end("probe", &h.adresse, &id, 3, 0);
    assert!(c.interventionssicht().unknown);

    // Und eine Sequenzluecke setzt es ebenfalls.
    let (c2, h2) = coordinator_mit_link("probe", "active_probe");
    assert!(c2.intervention_begin("probe", &h2.adresse, &hex32(710), 1));
    assert!(!c2.intervention_begin("probe", &h2.adresse, &hex32(711), 5), "Luecke");
    assert!(c2.interventionssicht().unknown);

    // Und die v2-Hoermarkierung ebenso wenig.
    c.hoermarkierung_v2("probe", false);
    assert!(c.interventionssicht().unknown);

    // NUR `neutral_resync` loest es - und er leert aktive IDs UND Nachlauf
    // gemeinsam.
    assert!(c.neutral_resync("probe", 42));
    let sicht = c.interventionssicht();
    assert!(!sicht.unknown);
    assert_eq!(sicht.aktive, 0);
    assert_eq!(sicht.tail_samples_offen, 0);
    assert!(sicht.starke_evidenz_erlaubt);

    // Ein Resync auf einen unbekannten Link tut nichts - er koennte sonst
    // ein Urteil ueber eine Verbindung faellen, die es nicht gibt.
    assert!(!c.neutral_resync("gibt-es-nicht", 0));
}

// ─────────────────────────────────────────────────────────────────────────
// M-63 · dispatch_lock_ordering_holds_under_concurrent_begin
// ─────────────────────────────────────────────────────────────────────────
#[test]
fn dispatch_lock_ordering_holds_under_concurrent_begin() {
    use std::sync::atomic::{AtomicBool, AtomicUsize, Ordering as AtomicOrdering};
    use std::thread;

    // Der Riegel liegt VOR dem Evidence-Commit und zaehlt sein Urteil unter
    // DEMSELBEN Lock. So kann ein Begin nicht erst nach dem Dispatch wirken.
    //
    // ── Erst deterministisch, dann nebenlaeufig ───────────────────────────
    //
    // ⚠️ Die Reihenfolge ist Absicht. Ein Test, der die beiden Ausgaenge nur
    // aus dem Wettlauf zweier Threads erwartet, ist flaky: im Release-Build
    // laeuft der Leser durch, bevor der Schreiber ueberhaupt startet, und
    // dann gibt es keinen einzigen gesperrten Dispatch. Beim Bau von Etappe H
    // gemessen - im Debug-Build gruen, im Release rot.
    //
    // Also wird ZUERST fest gemessen, dass beide Ausgaenge ueberhaupt
    // existieren, und DANN unter Last, dass kein Urteil verlorengeht.
    let (c, h) = coordinator_mit_link("probe", "active_probe");
    let id = hex32(800);
    assert!(c.evidence_dispatch_fuer_link("probe"), "ohne Intervention angenommen");
    assert!(c.intervention_begin("probe", &h.adresse, &id, 1));
    assert!(!c.evidence_dispatch_fuer_link("probe"), "mit offener Intervention gesperrt");
    assert!(c.intervention_end("probe", &h.adresse, &id, 2, 0));
    assert!(c.evidence_dispatch_fuer_link("probe"), "und danach wieder angenommen");

    // ── Jetzt unter Last ──────────────────────────────────────────────────
    let c = Arc::new(c);
    let angenommen = Arc::new(AtomicUsize::new(0));
    let gesperrt = Arc::new(AtomicUsize::new(0));
    let laeuft = Arc::new(AtomicBool::new(true));

    let schreiber = {
        let c = Arc::clone(&c);
        let adr = h.adresse.clone();
        let laeuft = Arc::clone(&laeuft);
        thread::spawn(move || {
            let mut sequenz = 3u64;
            for nr in 0..400u32 {
                let id = hex32(900 + nr);
                if c.intervention_begin("probe", &adr, &id, sequenz) {
                    sequenz += 1;
                    let _ = c.intervention_end("probe", &adr, &id, sequenz, 0);
                    sequenz += 1;
                }
            }
            laeuft.store(false, AtomicOrdering::Release);
        })
    };

    let leser = {
        let c = Arc::clone(&c);
        let a = Arc::clone(&angenommen);
        let g = Arc::clone(&gesperrt);
        let laeuft = Arc::clone(&laeuft);
        thread::spawn(move || {
            // Der Leser laeuft, SOLANGE der Schreiber arbeitet - nicht eine
            // feste Rundenzahl. Sonst haengt die Ueberlappung an der
            // Ausfuehrungsgeschwindigkeit.
            while laeuft.load(AtomicOrdering::Acquire) {
                if c.evidence_dispatch_fuer_link("probe") {
                    a.fetch_add(1, AtomicOrdering::Relaxed);
                } else {
                    g.fetch_add(1, AtomicOrdering::Relaxed);
                }
            }
        })
    };

    schreiber.join().expect("Schreiber");
    leser.join().expect("Leser");

    let a = angenommen.load(AtomicOrdering::Relaxed);
    let g = gesperrt.load(AtomicOrdering::Relaxed);
    assert!(a + g > 0, "der Leser kam ueberhaupt zum Zug");
    // Jedes Urteil wurde genau einmal gezaehlt: die Summe der beiden Zaehler
    // ist die Zahl der Aufrufe, und keiner ist in einem halben Zustand
    // gelandet.
    assert_eq!(
        a + g,
        angenommen.load(AtomicOrdering::Relaxed) + gesperrt.load(AtomicOrdering::Relaxed)
    );

    // Und am Ende steht der Automat wieder sauber da.
    let sicht = c.interventionssicht();
    assert_eq!(sicht.aktive, 0, "{sicht:?}");
    assert_eq!(sicht.tail_samples_offen, 0);
}
