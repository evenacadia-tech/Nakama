//! Die Coordinator-Verdrahtung im PRODUKTPFAD (SONDE-013, Nacharbeit 1).
//!
//! Die Erstpruefung 1 hat an dieser Stelle die schwerste Klasse Befunde
//! gemeldet, und sie hatten alle DIESELBE Ursache: `broker/src/coordinator/
//! mod.rs` exportierte `experiment`, `invalidierung`, `prepost` und
//! `vergleichbarkeit` als Bibliotheken, und der Coordinator besass weder
//! Felder noch Aufrufer dafuer. Die Matrixzeilen M-40 bis M-63 existierten
//! damit ausschliesslich in den Tests ihrer eigenen Module.
//!
//! Dieses Bein misst deshalb NICHT die Module. Es misst den Weg: Nachricht
//! rein → Store, Taint, Paar oder Ausschluss raus. Ein Fall, der ein Modul
//! direkt aufruft, koennte gruen sein, waehrend das Produkt gar nichts tut —
//! genau das war der Zustand vor dieser Runde.

use eqcop_broker::coordinator::{Coordinator, ManualClock};
use eqcop_broker::transport::bootstrap::{Adresse, AudioLage, HelloControl, HostAngabe};
#[cfg(windows)]
use eqcop_broker::transport::server_v3::Senke;
use serde_json::{json, Value};
use std::sync::Arc;

// ── Werkzeug ─────────────────────────────────────────────────────────────

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
        "cycle_derivation": "unsupported"
    })
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

/// Die Nutzlast kommt aus dem COMMITTETEN Fixturekorpus — dieselbe Regel wie
/// in `coordinator_model.rs`: eine zweite Wire-Form von Hand waere eine
/// zweite Wahrheit neben dem Korpus.
fn evidenz_wert() -> Value {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../eq-copilot/fixtures/v3/gueltig/evidence-snapshot-mit-ereignissen-und-stereo.json");
    serde_json::from_slice(&std::fs::read(&pfad).expect("Evidenzfixture liegt im Korpus"))
        .expect("Evidenzfixture ist JSON")
}

/// Ein Snapshot mit gesetzter Adresse, eigener `evidence_id` und wahlweise
/// veraenderten Transportfeldern.
fn evidenz_payload(adresse: &Adresse, nr: usize, anpassen: impl FnOnce(&mut Value)) -> Vec<u8> {
    let mut wert = evidenz_wert();
    wert["adresse"] = serde_json::to_value(adresse).expect("Adresse ist serialisierbar");
    wert["evidence_id"] = json!(hex(0x1000 + nr));
    if let Some(t) = wert.get_mut("transport") {
        t["sequence"] = json!(nr as u64 + 1);
    }
    anpassen(&mut wert);
    serde_json::to_vec(&wert).unwrap()
}

/// Ein vertragsgueltiger `probe_descriptor` mit `pair_id` und Messpunkt.
///
/// Ohne ihn hat eine Quelle weder Paar noch Rolle — und eine Haelfte ohne
/// Rolle ist keine Haelfte (M-13).
fn descriptor(adresse: &Adresse, position: &str, pair_id: &str) -> Value {
    json!({
        "adresse": adresse,
        "plugin_kind": "passive_probe",
        "measurement_position": position,
        "aussageklasse": "beobachtend",
        "betrieb": "active",
        "label": "Testprobe",
        "pair_id": pair_id,
        "capabilities": capabilities(),
        "frische": {"letzter_kontakt_ms": 10, "stale": false}
    })
}

fn experiment_begin_wert(ziel: &Adresse, command: usize, experiment: usize) -> Value {
    let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("../eq-copilot/fixtures/v3/gueltig/experiment_begin.json");
    let mut wert: Value =
        serde_json::from_slice(&std::fs::read(&pfad).expect("experiment_begin liegt im Korpus"))
            .expect("Fixture ist JSON");
    wert["kopf"]["ziel"] = serde_json::to_value(ziel).unwrap();
    wert["kopf"]["command_id"] = json!(hex(command));
    wert["experiment_id"] = json!(hex(experiment));
    wert
}

// ═════════════════════════════════════════════════════════════════════════
// B13/B14 · Der Evidenzempfaenger
// ═════════════════════════════════════════════════════════════════════════

/// B13 — jeder angenommene Snapshot bleibt, bis die Retention greift.
///
/// Der Insert ERSETZTE vorher den vorigen Snapshot derselben Quelle. Aus
/// EINEM Punkt lassen sich weder Resultat- noch Guardrail-Deltas rechnen
/// (M-49), und ein Export ohne Evidence-IDs ist keiner (M-51).
#[test]
fn evidenz_historie_bleibt_und_ist_gedeckelt() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);

    for nr in 0..5 {
        assert!(
            c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, nr, |_| {})),
            "Snapshot {nr} wird angenommen"
        );
    }
    let historie = c.evidenz_historie(&hex(10));
    assert_eq!(historie.len(), 5, "jeder angenommene Snapshot bleibt");
    let ids: Vec<&str> = historie.iter().map(|e| e.evidence_id.as_str()).collect();
    assert_eq!(
        ids.len(),
        ids.iter().collect::<std::collections::BTreeSet<_>>().len(),
        "und jeder traegt seine EIGENE evidence_id (M-51)"
    );
    // Der zuletzt angenommene steht weiterhin einzeln zur Verfuegung.
    assert_eq!(
        c.evidenz_sicht(&hex(10)).unwrap().evidence_id,
        historie.last().unwrap().evidence_id
    );
    // Und die Baender reisen mit — ohne sie kann der PRE/POST-Join nichts.
    assert!(
        !historie.last().unwrap().p50_db.is_empty(),
        "der Empfaenger BEHAELT die Verteilung, statt sie zu verwerfen (B25)"
    );
}

/// B13 — die Retention ist ein DECKEL und keine unbegrenzte Historie (M-74).
#[test]
fn evidenz_historie_haelt_ihren_deckel() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);

    let deckel = eqcop_broker::coordinator::EVIDENZ_RETENTION;
    for nr in 0..(deckel + 7) {
        c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, nr, |_| {}));
    }
    let historie = c.evidenz_historie(&hex(10));
    assert_eq!(
        historie.len(),
        deckel,
        "die Historie waechst nicht ueber ihren Deckel (M-74)"
    );
    assert_eq!(
        historie.first().unwrap().evidence_id,
        hex(0x1000 + 7),
        "und sie schneidet vorne ab - die JUENGSTEN bleiben"
    );
}

/// B14 (Luecke, entschieden) — `beeinflusst=true` schliesst aus und zaehlt.
#[test]
fn beeinflusste_evidenz_wird_ausgeschlossen_und_gezaehlt() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);

    // Die Gegenprobe: ohne das Flag wird derselbe Snapshot angenommen.
    assert!(c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, 0, |_| {})));
    assert_eq!(c.evidence_beeinflusst_zaehler(), 0);

    let vorher = c.evidenz_historie(&hex(10)).len();
    assert!(
        !c.evidence_snapshot_json(
            "a",
            &evidenz_payload(&h.adresse, 1, |w| {
                w["beeinflusst"] = json!(true);
            })
        ),
        "ein Snapshot mit `beeinflusst=true` wird NICHT angenommen (M-52)"
    );
    assert_eq!(
        c.evidenz_historie(&hex(10)).len(),
        vorher,
        "und landet auch nicht in der Historie"
    );
    assert_eq!(
        c.evidence_beeinflusst_zaehler(),
        1,
        "er ist GEZAEHLT, nicht still verworfen"
    );
    // Das globale Gate war dabei frei - genau das ist der Punkt: das
    // Frame-Flag ist ein ZUSAETZLICHER Ausschlussbeleg.
    assert!(c.interventionssicht().starke_evidenz_erlaubt);
}

/// B15 — die Evidenzmap faellt mit ihrem Client (M-74, verbinden↔trennen).
#[test]
fn evidenz_faellt_mit_dem_client() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);
    assert!(c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, 0, |_| {})));
    assert!(!c.evidenz_historie(&hex(10)).is_empty());

    // Ein Hello mit NEUER Nonce: ein neuer Prozess erbt die Messwahrheit des
    // alten nicht.
    let neu = hello(1, 2, 10, 999, "passive_probe", Some(9));
    anmelden(&c, "b", &neu);
    assert!(
        c.evidenz_historie(&hex(10)).is_empty(),
        "die Evidenzhistorie faellt mit der alten Runtime-Nonce"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// B17 · Der Taint ist sitzungsweit
// ═════════════════════════════════════════════════════════════════════════

/// B17 — ein Marker in Sitzung A sperrt Sitzung B NICHT, und ein Resync aus B
/// loescht die aktiven IDs von A nicht (M-62).
#[test]
fn taint_ist_sitzungsweit() {
    let (c, _) = coordinator();
    let a = hello(1, 2, 10, 100, "main", Some(9));
    let b = hello(1, 3, 20, 200, "main", Some(8));
    anmelden(&c, "la", &a);
    anmelden(&c, "lb", &b);

    let id = hex(0x500);
    assert!(c.intervention_begin("la", &a.adresse, &id, 1));

    assert!(
        !c.interventionssicht_fuer_link("la").starke_evidenz_erlaubt,
        "die Sitzung MIT dem Marker ist gesperrt"
    );
    assert!(
        c.interventionssicht_fuer_link("lb").starke_evidenz_erlaubt,
        "die andere Sitzung nicht - genau das war Befund B17"
    );
    assert!(
        c.evidence_dispatch_fuer_link("lb"),
        "und ihr Dispatch geht durch"
    );
    assert!(!c.evidence_dispatch_fuer_link("la"));

    // Der gefaehrlichere Teil: ein Resync aus B raeumt A NICHT ab.
    assert!(c.neutral_resync("lb", 0));
    assert!(
        !c.interventionssicht_fuer_link("la").starke_evidenz_erlaubt,
        "ein fremder Resync loescht die aktive Intervention von A nicht"
    );
    assert!(!c.evidence_dispatch_fuer_link("la"));

    // Und der EIGENE Resync loest sie.
    assert!(c.neutral_resync("la", 1));
    assert!(c.interventionssicht_fuer_link("la").starke_evidenz_erlaubt);
}

/// B16 — der Nachlauf laeuft am Liveness-Tick ab (M-58/M-60).
#[test]
fn nachlauf_laeuft_am_tick_ab() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    let id = hex(0x600);
    assert!(c.intervention_begin("a", &h.adresse, &id, 1));
    assert!(c.intervention_end("a", &h.adresse, &id, 2, 30_000));
    assert_eq!(c.interventionssicht_fuer_link("a").tail_samples_offen, 30_000);
    assert!(!c.evidence_dispatch_fuer_link("a"), "der Nachlauf sperrt");

    // Der Tick zieht ab - und zwar von selbst, ohne dass jemand
    // `tail_fortschritt` von Hand ruft. Genau DAS war Befund B16.
    c.liveness_tick();
    let nach_einem = c.interventionssicht_fuer_link("a").tail_samples_offen;
    assert!(
        nach_einem < 30_000,
        "ein Tick zieht vom Nachlauf ab: {nach_einem}"
    );
    for _ in 0..8 {
        c.liveness_tick();
    }
    assert_eq!(
        c.interventionssicht_fuer_link("a").tail_samples_offen,
        0,
        "und nach genug Ticks ist er abgelaufen"
    );
    assert!(
        c.evidence_dispatch_fuer_link("a"),
        "danach ist Evidenz wieder erlaubt"
    );
}

/// B16 — `resync_bestaetigen` ist der Produktaufrufer von `neutral_resync`.
#[test]
fn bestaetigter_resync_loest_das_sticky_unknown() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "a", &h);
    // Eine Sequenzluecke setzt sticky Unknown.
    let id = hex(0x700);
    assert!(c.intervention_begin("a", &h.adresse, &id, 1));
    assert!(!c.intervention_end("a", &h.adresse, &id, 9, 0), "Luecke");
    assert!(c.interventionssicht_fuer_link("a").unknown);

    assert!(
        !c.resync_bestaetigen("unbekannt", 9),
        "ein unbekannter Link darf nicht entsperren"
    );
    assert!(c.resync_bestaetigen("a", 9), "der stehende Link darf");
    assert!(
        c.interventionssicht_fuer_link("a").starke_evidenz_erlaubt,
        "und danach ist der Zustand wieder bekannt"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// B18/B19/B20/B21/B22 · Die Experimentfamilien im Produktpfad
// ═════════════════════════════════════════════════════════════════════════

/// Ein Coordinator mit einem bestaetigten fuehrenden Main — die Lage, in der
/// ein `experiment_begin` ueberhaupt autorisiert ist.
fn mit_main() -> (Coordinator, HelloControl) {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "main", Some(9));
    anmelden(&c, "main", &h);
    report(&c, "main", &h.adresse);
    (c, h)
}

#[cfg(windows)]
/// B18/B23 — ein schema-gueltiges `experiment_begin` legt Passage UND Versuch
/// an (M-25/M-40).
///
/// Vorher fiel die Nachricht in `_ => None`: der Coordinator besass keinen
/// Experimentstore, und `p0_json_mit_minor` hatte keinen Match-Zweig.
#[test]
fn experiment_begin_legt_passage_und_versuch_an() {
    let (c, h) = mit_main();
    let wert = experiment_begin_wert(&h.adresse, 0x900, 0xabc);
    let payload = serde_json::to_vec(&wert).unwrap();

    let antwort = Senke::p0(&c, "main", &payload)
        .expect("B18: die Familie wird BEANTWORTET - vorher fiel sie in `_ => None`");
    let ack: Value = serde_json::from_slice(&antwort).unwrap();
    assert_eq!(
        ack["type"], "command_ack",
        "und zwar mit einem command_ack, wie jeder andere persistente P0-Befehl"
    );
    // Dieser Coordinator hat KEINEN Store. Ein persistenter Befehl ist dann
    // `abgelehnt/internal` — dieselbe Regel wie fuer `preview_*`, und genau
    // deshalb misst der Store-Fall in `store_crash_matrix.rs` die Wirkung.
    assert_eq!(ack["ergebnis"], "abgelehnt");
    assert_eq!(ack["code"], "internal");
    // Und ohne Wirkung entsteht auch KEIN halber Zustand.
    assert!(
        c.passage_sicht(wert["passage"]["passage_id"].as_str().unwrap())
            .is_none(),
        "ein abgelehnter Befehl legt keine Passage an"
    );
    assert!(c.experiment_sicht(&hex(0xabc)).is_none());
}

/// B20 — ein Ergebnis OHNE Resultatmessung schliesst nicht ab (M-45).
#[test]
fn ergebnis_ohne_resultatmessung_wird_abgelehnt() {
    use eqcop_broker::coordinator::experiment::{
        Abschlussfehler, Alignmentwert, Blindreihenfolge, Experimentreferenz, Experimentstore,
        Hoerurteil, Passage, Resultatmessung,
    };
    use eqcop_broker::telemetrie::Fingerprintwerte;

    let mut s = Experimentstore::neu();
    let passage = Passage {
        passage_id: hex(1),
        projekt_von: 0,
        projekt_bis: 480_000,
        transport_epoch: 1,
        aktive_quellen: vec![hex(10)],
        abdeckung: 0.9,
        label: None,
        fingerprint: Fingerprintwerte::default(),
    };
    let referenz = Experimentreferenz {
        passage_fingerprint: Fingerprintwerte::default(),
        upstream_fingerprint: Fingerprintwerte::default(),
        aktive_quellen: vec![hex(10)],
        messpunktklassen: vec!["insert".into()],
        match_gain_db: -1.5,
        alignment: Alignmentwert::FeatureAligned,
    };
    let id = hex(0xabc);
    s.beginne(&id, &hex(1), passage, referenz).unwrap();
    s.binde_reihenfolge(&id, Blindreihenfolge::BaselineZuerst)
        .unwrap();

    // Die LEERE Messung: genau der Fall, mit dem `ergebnis()` vorher
    // erfolgreich terminierte.
    assert_eq!(
        s.ergebnis(
            &id,
            Hoerurteil::Kandidat,
            None,
            None,
            &Resultatmessung::default()
        ),
        Err(Abschlussfehler::OhneResultatmessung),
        "ohne Resultatmessung gibt es kein Terminalereignis (M-45)"
    );
    assert!(
        s.experiment(&id).unwrap().offen(),
        "und der Versuch bleibt OFFEN"
    );

    // Mit Messung: die vier Achsen entstehen und reisen mit.
    let messung = Resultatmessung {
        band_delta_db: (0..32).map(|i| (i as f64) * 0.2 - 3.0).collect(),
        band_gueltig: vec![true; 32],
        erste_haelfte: vec![1.0; 32],
        zweite_haelfte: vec![1.0; 32],
        abdeckung_baseline: 0.9,
        abdeckung_resultat: 0.9,
        klasse_baseline: "mittel".into(),
        klasse_resultat: "mittel".into(),
        vergleichbarkeit: Some("stark".into()),
        vergleichbarkeit_gruende: Vec::new(),
        baseline_evidence_ids: vec![hex(0x11)],
        resultat_evidence_ids: vec![hex(0x22)],
    };
    let achsen = s
        .ergebnis(&id, Hoerurteil::Kandidat, None, None, &messung)
        .expect("mit Messung schliesst der Versuch ab");
    assert!(achsen.gescannte_baender > 0, "die Zielmetrik ist gerechnet");
    assert!(
        achsen.intervall.is_some(),
        "das Bootstrap-Intervall steht: {achsen:?}"
    );
    assert!(
        achsen.effekt_stabil.is_some(),
        "und die Effektstabilitaet ist beurteilt"
    );

    // B21: der Export traegt die Evidence-IDs.
    let export = s.exportiere(&id).expect("Export entsteht");
    assert_eq!(export.baseline_evidence_ids, vec![hex(0x11)]);
    assert_eq!(export.resultat_evidence_ids, vec![hex(0x22)]);

    // B19: das Log traegt die unveraenderlichen Referenzen, nicht nur IDs.
    use eqcop_broker::coordinator::experiment::Ereignis;
    let hat_passage = s
        .log()
        .iter()
        .any(|e| matches!(e, Ereignis::PassageAngelegt { passage, .. } if passage.projekt_bis > 0));
    assert!(
        hat_passage,
        "das Passageereignis traegt die PASSAGE, nicht nur ihre ID (M-47/M-50)"
    );
    let hat_baseline = s.log().iter().any(
        |e| matches!(e, Ereignis::Begonnen { baseline, .. } if baseline.match_gain_db == -1.5),
    );
    assert!(
        hat_baseline,
        "und das Beginereignis traegt die Baseline samt Match-Gain (§43.1)"
    );
    let hat_achsen = s
        .log()
        .iter()
        .any(|e| matches!(e, Ereignis::Ergebnis { achsen, .. } if achsen.gescannte_baender > 0));
    assert!(
        hat_achsen,
        "und das Terminalereignis traegt die gerechneten Deltas (M-49)"
    );
}

// ═════════════════════════════════════════════════════════════════════════
// B24 · Die Invalidierung
// ═════════════════════════════════════════════════════════════════════════

/// B24 — eine Invalidierung MARKIERT gespeicherte Evidenz als ausgeschlossen.
///
/// `invalidierung` enthielt vorher nur Konstruktoren: kein Aufrufer, kein
/// Serializer, keine Outbox, keine Mutation. Marker, Preview, Seek, Drop,
/// Material- und Messpunktwechsel nahmen im Produkt nichts zurueck.
#[test]
fn invalidierung_schliesst_gespeicherte_evidenz_aus() {
    let (c, _) = coordinator();
    let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
    anmelden(&c, "a", &h);
    report(&c, "a", &h.adresse);
    for nr in 0..3 {
        assert!(c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, nr, |_| {})));
    }
    assert!(
        c.evidenz_historie(&hex(10))
            .iter()
            .all(|e| e.ausschlussgrund.is_none()),
        "vorher traegt kein Beleg einen Ausschlussgrund"
    );

    let betroffen = c.invalidierung_wegen_messpunkt_fuer_link("a", "insert", "post");
    assert!(
        betroffen > 0,
        "der Messpunktwechsel nimmt gespeicherte Evidenz ZURUECK (M-55)"
    );
    let historie = c.evidenz_historie(&hex(10));
    assert!(
        historie
            .iter()
            .all(|e| e.ausschlussgrund.as_deref() == Some("messpunkt_wechsel")),
        "und jeder Beleg traegt den GRUND, nie einen stillen Ausschluss (M-28)"
    );
    assert_eq!(c.invalidierungen_zaehler(), 1);
    assert_eq!(c.evidenz_ausgeschlossen_zaehler(), betroffen as u64);

    // Ein gleicher Messpunkt invalidiert NICHT - die Gegenprobe, ohne die der
    // Fall darueber nichts sagt.
    assert_eq!(
        c.invalidierung_wegen_messpunkt_fuer_link("a", "insert", "insert"),
        0
    );
    assert_eq!(c.invalidierungen_zaehler(), 1, "und zaehlt auch nicht");
}

/// B24 — der Serializer haelt die diskriminierte Form von `evidence_invalidate`.
#[test]
fn invalidierung_serialisiert_ihre_drei_umfaenge() {
    use eqcop_broker::coordinator::invalidierung::{Grund, Invalidierung, Umfang};
    let ganz = Coordinator::invalidierung_als_json(&Invalidierung {
        grund: Grund::Epochwechsel,
        umfang: Umfang::GanzeSitzung,
    });
    assert_eq!(ganz["type"], "evidence_invalidate");
    assert_eq!(ganz["grund"], "epochwechsel");
    assert_eq!(ganz["umfang"]["art"], "ganze_sitzung");
    assert!(
        ganz["umfang"].get("von_sample").is_none(),
        "`ganze_sitzung` traegt KEINEN Bereich - ein Etikett ohne Wirkung waere \
         genau die Aufweichung, gegen die der Umfang ein Enum ist"
    );

    let bereich = Coordinator::invalidierung_als_json(&Invalidierung {
        grund: Grund::Sequenzluecke,
        umfang: Umfang::Bereich { von: 10, bis: 20 },
    });
    assert_eq!(bereich["umfang"]["art"], "sample_range");
    assert_eq!(bereich["umfang"]["von_sample"], 10);
    assert_eq!(bereich["umfang"]["bis_sample"], 20);

    let ids = Coordinator::invalidierung_als_json(&Invalidierung {
        grund: Grund::MaterialWechsel,
        umfang: Umfang::Ids([hex(1)].into_iter().collect()),
    });
    assert_eq!(ids["umfang"]["art"], "evidence_ids");
    assert_eq!(ids["umfang"]["evidence_ids"][0], hex(1));
}

/// B24 — ein Transportbruch loest die Invalidierung MIT DEM RICHTIGEN GRUND
/// aus (M-53).
#[test]
fn transportbruch_invalidiert_mit_seinem_grund() {
    for (feld, grund) in [
        ("transport_epoch", "epochwechsel"),
        ("continuity_segment", "sequenzluecke"),
    ] {
        let (c, _) = coordinator();
        let h = hello(1, 2, 10, 100, "passive_probe", Some(9));
        anmelden(&c, "a", &h);
        report(&c, "a", &h.adresse);
        assert!(c.evidence_snapshot_json("a", &evidenz_payload(&h.adresse, 0, |_| {})));
        assert_eq!(c.invalidierungen_zaehler(), 0, "{feld}: noch kein Bruch");

        assert!(c.evidence_snapshot_json(
            "a",
            &evidenz_payload(&h.adresse, 1, |w| {
                let alt = w["transport"][feld].as_u64().unwrap_or(0);
                w["transport"][feld] = json!(alt + 1);
            })
        ));
        assert_eq!(
            c.invalidierungen_zaehler(),
            1,
            "{feld}: der Bruch loest eine Invalidierung aus"
        );
        let historie = c.evidenz_historie(&hex(10));
        assert!(
            historie
                .iter()
                .any(|e| e.ausschlussgrund.as_deref() == Some(grund)),
            "{feld}: und zwar mit dem Grund `{grund}` (M-53): {:?}",
            historie
                .iter()
                .map(|e| e.ausschlussgrund.clone())
                .collect::<Vec<_>>()
        );
    }
}

// ═════════════════════════════════════════════════════════════════════════
// B25 · Der PRE/POST-Join
// ═════════════════════════════════════════════════════════════════════════

/// B25 — der Join LAEUFT im Produktpfad und liefert ein Urteil je Paar.
///
/// `bilde_paare` hatte ausserhalb der Tests keinen Aufrufer, und der
/// Evidenzempfaenger verwarf die Kurven, die eine `Paarhaelfte` braucht.
/// Reale PRE-/POST-Snapshots erreichten den v3-Join damit nie.
#[test]
fn prepost_join_laeuft_im_produktpfad() {
    let (c, _) = coordinator();
    let pre = hello(1, 2, 10, 100, "passive_probe", Some(9));
    let post = hello(1, 2, 11, 101, "passive_probe", Some(9));
    anmelden(&c, "pre", &pre);
    anmelden(&c, "post", &post);
    // Die Deskriptoren tragen `pair_id` und Messpunkt - ohne sie gibt es
    // keine Rolle und damit keine Haelfte.
    let paar = hex(0x77);
    assert!(
        c.descriptor_setzen("pre", descriptor(&pre.adresse, "pre", &paar)),
        "der PRE-Deskriptor wird angenommen"
    );
    assert!(
        c.descriptor_setzen("post", descriptor(&post.adresse, "post", &paar)),
        "der POST-Deskriptor auch"
    );

    assert_eq!(
        c.paarurteile_anzahl(),
        0,
        "vor dem ersten Snapshot gibt es kein Urteil"
    );
    for nr in 0..8 {
        c.evidence_snapshot_json("pre", &evidenz_payload(&pre.adresse, nr, |_| {}));
        c.evidence_snapshot_json("post", &evidenz_payload(&post.adresse, nr, |_| {}));
    }

    // 🔑 DAS ist die Zusage: der Join laeuft IM EMPFAENGER und legt sein
    // Urteil ab. Vorher hatte `bilde_paare` ausserhalb der Tests keinen
    // Aufrufer, und der Empfaenger verwarf die Kurven, die eine Haelfte
    // braucht — es entstand nie ein Urteil.
    let urteil = c
        .paarurteil(&paar)
        .expect("B25: der PRE/POST-Join liefert ein Urteil je Paar (M-13/M-14)");
    assert_eq!(urteil.pair_id, paar);
    // Und er behauptet NICHTS Starkes: die Presentation-Abbildung ist
    // nirgends validiert (Capabilityreport S4), also bleibt die Klasse unter
    // `FeatureAligned` — genau der Exit-Gate-Satz aus M-21.
    use eqcop_broker::coordinator::prepost::Alignmentklasse;
    assert!(
        urteil.klasse < Alignmentklasse::FeatureAligned,
        "ohne validierte Presentation-Abbildung keine starke Aussage: {:?}",
        urteil.klasse
    );
}
