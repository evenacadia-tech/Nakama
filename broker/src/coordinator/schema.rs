//! Der v3-Vertrag als Leser, ohne Sessionwissen.
//!
//! Das eingefrorene Schema wird einmal geladen und danach nur noch befragt;
//! die Minorfassungen bleiben nebeneinander lesbar.

use super::*;

pub(super) const JSON_SCHEMA_MINOR_AKTIV: u8 = 4;

/// Die zwei Ausschlussgruende, die die **Fassung 4** ausmachen (NAK-213 R1/R3).
///
/// Sie stehen hier als DATEN, nicht als Kommentar: `v3_schema_minor_3_wurzel`
/// baut daraus zurueck, und der Register-Riegel haelt dieselbe Liste gegen
/// `wire_envelope_schema_minor.fassungen."4".ausschlussgrund_erweitert`. Ein
/// Rueckbau, der nur als Kommentar existiert, ist keine Fassung.
const FASSUNG_4_GRUENDE: [&str; 2] = ["screening_ueberboten", "master_duplikat"];

/// Die Laengengrenze der Ausschlussliste in der **Fassung 3** (NAK-213 R6).
///
/// 🔑 Der Rueckbau nimmt nicht nur die zwei Enumwerte zurueck, sondern AUCH
/// diese Zahl. M-77 verlangt dieselbe Regel in beiden Sprachen ausdruecklich
/// fuer „jede Laengen- oder Enumgrenze"; ohne den zweiten Griff wiese der
/// Leser der Fassung 3 einen Befund mit 33 Ausschluessen NICHT ab, obwohl das
/// Fassung-3-Schema und der C++-Leser ihn abweisen — und die Regel haenge
/// wieder an nur einer Seite.
const FASSUNG_3_AUSSCHLUSS_MAX: u64 = 32;

/// Die Familien, die die **Fassung 3** ausmachen (SONDE-014/P5).
///
/// Zwei entstehen NEU (`intent_update`, `assistant_step_update`), zwei waren
/// reserviert und werden belegt (`draft_offer`, `user_verdict`). Fuer den
/// Rueckbau ist der Unterschied gleichgueltig: ein Leser der Fassung 2 kannte
/// keinen von ihnen und muss jeden ablehnen.
const FASSUNG_3_FAMILIEN: [&str; 4] = [
    "intent_update",
    "assistant_step_update",
    "draft_offer",
    "user_verdict",
];

/// Hilfsdefinitionen, die ausschliesslich von der Fassung 3 gebraucht werden.
/// Sie tragen keinen eigenen `oneOf`-Zweig und stehen deshalb nicht im
/// Register der Familien; ohne ihre Fassung sind sie aber unerreichbar, und
/// ein `$ref` ins Leere bricht den Ladelauf.
///
/// ⚠️ `session_finding` und `experiment_ziel` gehoeren AUCH hierher, obwohl
/// sie an bestehenden Familien haengen: `session_snapshot.findings` und
/// `experiment_begin.ziel` verschwinden im Rueckbau mit, und eine
/// haengengebliebene Definition waere ein `$ref` auf nichts.
const FASSUNG_3_HILFSDEFS: [&str; 33] = [
    "intent_rolle",
    "intent_herkunft",
    "intent_beziehungsart",
    "intent_schutzeigenschaft",
    "bandintervall",
    "source_intent",
    "intent_schutz",
    "intent_beziehung",
    "assistant_schritt",
    "ursachenklasse",
    "aussageklasse",
    "sicherheitsklasse",
    "befund_zustand",
    "befund_konfidenz",
    "zielmetrik",
    "naechster_test",
    "ausschlussgrund",
    "finding_ausschluss",
    "maskierung",
    "rangkomponenten",
    "beobachtung",
    "session_finding",
    "proposal_aktion",
    "proposal_execution",
    "proposal_revert",
    "proposal_wirkung",
    "proposal_stopbedingung",
    "wertebereich",
    "proposal_parameter",
    "proposal_grenzen",
    "proposal",
    "user_urteil",
    "experiment_ziel",
];

/// Die Familien und Felder, die die **Fassung 2** ausmachen (SONDE-013).
///
/// Sie stehen hier NICHT als Kommentar, sondern als Daten: der Test
/// `fassung_2_liste_deckt_sich_mit_dem_register` vergleicht sie mit dem
/// Eintrag `wire_envelope_schema_minor.fassungen."2"` in
/// `reservierte-nachrichten-v1.json`. Drei handgepflegte Kopien derselben
/// Liste sind in diesem Projekt schon einmal auseinandergelaufen
/// (T2-Runde 2, BF-5) — hier faellt die Abweichung am Riegel statt an einem
/// Fixture, das zufaellig darauf zielt.
const FASSUNG_2_FAMILIEN: [&str; 4] = [
    "experiment_begin",
    // Nacharbeit 2 (R16/R21): der Schritt zwischen Begin und Ergebnis.
    "experiment_candidate",
    "experiment_abort",
    "experiment_manual_result",
];

/// Hilfsdefinitionen, die ausschliesslich von der Fassung 2 gebraucht werden.
/// Sie tragen keinen eigenen `oneOf`-Zweig und stehen deshalb nicht im
/// Register; ohne ihre Familien sind sie aber unerreichbar, und ein `$ref`
/// ins Leere bricht den Ladelauf.
const FASSUNG_2_HILFSDEFS: [&str; 12] = [
    // Nacharbeit 2 (Befunde R14/R32): der `session_snapshot` traegt die
    // Versuche und die Paarurteile der Sitzung. Beide Teildefinitionen
    // gehoeren zur Fassung 2 und fallen mit ihr - sonst haetten sie im
    // zurueckgebauten Schema haengende Referenzen auf `alignment_klasse`.
    "session_experiment",
    "session_paar",
    // Nacharbeit 1 (Befund B23): `experiment_begin` traegt die VOLLSTAENDIGE
    // Passage. Sie gehoert damit zur Fassung 2 und muss beim Rueckbau auf die
    // Fassung 1 mit verschwinden - sonst laese ein Leser der alten Fassung
    // einen Typ, den es dort nie gab.
    //
    // ⚠️ Die `passage_id` hat ausdruecklich KEINE eigene Definition. Sie waere
    // ein blosser `$ref` auf `hex32` — und A5 verlangt zu Recht je Definition
    // ein Negativfixture, das sich von dem der hex32 unterscheidet. Eine
    // Definition, deren Gegenbeispiel dasselbe ist wie das ihres Ziels, traegt
    // keine eigene Aussage.
    "passage",
    "experiment_referenz",
    "alignment_klasse",
    "fingerprint",
    "evidence_ereignisse",
    "dynamics_ereignis",
    "stereo_evidenz",
    "stereo_bandwerte",
    "stereo_bandwerte_normiert",
    "stereo_bandwerte_phase",
];

const FASSUNG_2_EVIDENZFELDER: [&str; 2] = ["ereignisse", "stereo"];
/// Felder des `session_snapshot`, die zur Fassung 2 gehoeren (R14/R32).
const FASSUNG_2_SNAPSHOTFELDER: [&str; 2] = ["experimente", "paare"];
const FASSUNG_2_GRUENDE: [&str; 2] = ["material_wechsel", "messpunkt_wechsel"];
/// Die Fehlercodes der Fassung 2 (SONDE-013 Nacharbeit 1). Sie benennen die
/// Produktregeln der Experimentfamilien; ein Leser der Fassung 1 kennt sie
/// nicht und muss sie ABLEHNEN, statt sie still auf `internal` abzubilden.
const FASSUNG_2_FEHLERCODES: [&str; 6] = [
    "abdeckung_zu_gering",
    "schon_terminal",
    "ohne_lautheitsabgleich",
    "ohne_resultatmessung",
    "blindreihenfolge_widerspruch",
    // Nacharbeit 2 (Befund R21): die Bindung hat einen eigenen Befehlszweig,
    // also hat ihr Fehlen einen eigenen Namen.
    "reihenfolge_nicht_gebunden",
];

/// Baut aus der committeten **Fassung 2** die **Fassung 1** zurueck.
///
/// Der Vertrag ist an Discriminator, Zieladresse, Revision und Capability
/// NICHT additiv erweiterbar: ein Leser der alten Fassung muss jede Neuerung
/// ABLEHNEN, statt sie still auf einen bekannten Zweig abzubilden. Genau das
/// kann er nur, wenn seine Fassung wirklich existiert — eine Konstante
/// `MINOR_1`, die in Wahrheit das neueste Schema laedt, waere das Gegenteil
/// der Zusage.
/// Baut aus der committeten **Fassung 3** die **Fassung 2** zurueck.
///
/// Derselbe Grund wie eine Fassung tiefer: ein Leser der alten Fassung muss
/// jede Neuerung ABLEHNEN, statt sie still auf einen bekannten Zweig
/// abzubilden — und das kann er nur, wenn seine Fassung wirklich existiert.
/// Baut aus der committeten **Fassung 4** die **Fassung 3** zurueck.
///
/// Zwei Griffe, beide aus NAK-213: die zwei neuen Ausschlussgruende fallen aus
/// der geschlossenen Menge, und `session_finding.ausschluesse.maxItems` faellt
/// von `SESSION_CLIENT_CAP` (64) auf 32 zurueck. Ein Leser der Fassung 3 muss
/// BEIDES ablehnen — den unbekannten Grund wie die zu lange Liste —, statt sie
/// still durchzulassen (M-77: „jede Laengen- oder Enumgrenze").
pub(super) fn v3_schema_minor_3_wurzel() -> Value {
    let mut wurzel = v3_schema_wurzel();
    {
        let defs = wurzel["$defs"]
            .as_object_mut()
            .expect("v3-$defs ist ein Objekt");
        defs["ausschlussgrund"]["enum"]
            .as_array_mut()
            .expect("ausschlussgrund-enum ist ein Array")
            .retain(|wert| {
                wert.as_str()
                    .is_none_or(|g| !FASSUNG_4_GRUENDE.contains(&g))
            });
        defs["session_finding"]["properties"]["ausschluesse"]["maxItems"] =
            Value::from(FASSUNG_3_AUSSCHLUSS_MAX);
    }
    wurzel
}

pub(super) fn v3_schema_minor_2_wurzel() -> Value {
    // Die Rueckbauten sind eine KETTE (siehe `v3_schema_minor_0_wurzel`):
    // stuende hier `v3_schema_wurzel()`, truege der Leser der Fassung 2 die
    // zwei Ausschlussgruende und die gehobene Laengengrenze der Fassung 4 mit.
    let mut wurzel = v3_schema_minor_3_wurzel();
    {
        let defs = wurzel["$defs"]
            .as_object_mut()
            .expect("v3-$defs ist ein Objekt");
        for name in FASSUNG_3_FAMILIEN.iter().chain(FASSUNG_3_HILFSDEFS.iter()) {
            defs.remove(*name);
        }
        defs["session_snapshot"]["properties"]
            .as_object_mut()
            .expect("session_snapshot-properties ist ein Objekt")
            .remove("findings");
        defs["experiment_begin"]["properties"]
            .as_object_mut()
            .expect("experiment_begin-properties ist ein Objekt")
            .remove("ziel");
    }
    wurzel["oneOf"]
        .as_array_mut()
        .expect("v3-oneOf ist ein Array")
        .retain(|zweig| {
            zweig
                .get("$ref")
                .and_then(Value::as_str)
                .and_then(|r| r.strip_prefix("#/$defs/"))
                .is_none_or(|n| !FASSUNG_3_FAMILIEN.contains(&n))
        });
    wurzel
}

pub(super) fn v3_schema_minor_1_wurzel() -> Value {
    // Baut auf der Fassung 2 auf, NICHT auf der committeten Fassung. Die
    // Rueckbauten sind eine KETTE (siehe `v3_schema_minor_0_wurzel`): stuende
    // hier `v3_schema_wurzel()`, truege der Leser der Fassung 1 alle
    // Neuerungen der Fassung 3 mit.
    let mut wurzel = v3_schema_minor_2_wurzel();
    {
        let defs = wurzel["$defs"]
            .as_object_mut()
            .expect("v3-$defs ist ein Objekt");
        for name in FASSUNG_2_FAMILIEN.iter().chain(FASSUNG_2_HILFSDEFS.iter()) {
            defs.remove(*name);
        }
        let evidenz = defs["evidence_snapshot"]["properties"]
            .as_object_mut()
            .expect("evidence_snapshot-properties ist ein Objekt");
        for feld in FASSUNG_2_EVIDENZFELDER {
            evidenz.remove(feld);
        }
        let sitzung = defs["session_snapshot"]["properties"]
            .as_object_mut()
            .expect("session_snapshot-properties ist ein Objekt");
        for feld in FASSUNG_2_SNAPSHOTFELDER {
            sitzung.remove(feld);
        }
        defs["evidence_invalidate"]["properties"]["grund"]["enum"]
            .as_array_mut()
            .expect("grund-enum ist ein Array")
            .retain(|wert| {
                wert.as_str()
                    .is_none_or(|g| !FASSUNG_2_GRUENDE.contains(&g))
            });
        defs["fehlercode"]["enum"]
            .as_array_mut()
            .expect("fehlercode-enum ist ein Array")
            .retain(|wert| {
                wert.as_str()
                    .is_none_or(|c| !FASSUNG_2_FEHLERCODES.contains(&c))
            });
        defs["konfidenz"]["properties"]
            .as_object_mut()
            .expect("konfidenz-properties ist ein Objekt")
            .remove("verteilung_fenster");
    }
    wurzel["oneOf"]
        .as_array_mut()
        .expect("v3-oneOf ist ein Array")
        .retain(|zweig| {
            zweig
                .get("$ref")
                .and_then(Value::as_str)
                .and_then(|r| r.strip_prefix("#/$defs/"))
                .is_none_or(|n| !FASSUNG_2_FAMILIEN.contains(&n))
        });
    wurzel
}

pub(super) fn v3_schema_wurzel() -> Value {
    serde_json::from_str(include_str!(
        "../../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
    ))
    .expect("eingefrorenes v3-Schema ist JSON")
}

pub(super) fn v3_schema_minor_0_wurzel() -> Value {
    // Baut auf der Fassung 1 auf, NICHT auf der committeten Fassung. Die
    // Rueckbauten sind eine Kette: jede Fassung nimmt genau ihre eigenen
    // Zusaetze zurueck. Stuende hier `v3_schema_wurzel()`, truege der
    // aelteste Leser alle Neuerungen aller spaeteren Fassungen mit — der
    // Fehler, gegen den die Kette existiert.
    let mut wurzel = v3_schema_minor_1_wurzel();
    {
        let defs = wurzel["$defs"]
            .as_object_mut()
            .expect("v3-$defs ist ein Objekt");
        for name in [
            "probe_descriptor_insert",
            "probe_descriptor_pre",
            "probe_descriptor_post",
            "probe_descriptor_beitrag",
        ] {
            let zweig = defs[name]
                .as_object_mut()
                .expect("Probe-Descriptor-Zweig ist ein Objekt");
            zweig["required"]
                .as_array_mut()
                .expect("Descriptor-required ist ein Array")
                .retain(|wert| wert.as_str() != Some("betrieb"));
            let props = zweig["properties"]
                .as_object_mut()
                .expect("Descriptor-properties ist ein Objekt");
            props.remove("betrieb");
            props.remove("host_bus_name");
            props.remove("host_mixer_index");
        }
        defs["heartbeat"]["properties"]
            .as_object_mut()
            .expect("Heartbeat-properties ist ein Objekt")
            .remove("runtime");
        defs["session_snapshot"]["properties"]
            .as_object_mut()
            .expect("Snapshot-properties ist ein Objekt")
            .remove("store_degraded");
        defs["session_snapshot"]["properties"]["mitglieder"]["items"] =
            serde_json::json!({"$ref": "#/$defs/probe_descriptor"});
        defs.remove("session_command");
    }
    wurzel["oneOf"]
        .as_array_mut()
        .expect("v3-oneOf ist ein Array")
        .retain(|zweig| {
            zweig.get("$ref").and_then(Value::as_str) != Some("#/$defs/session_command")
        });
    wurzel
}

pub(super) fn v3_schema(schema_minor: u8) -> Option<&'static crate::vertrag::Schema> {
    static MINOR_0: OnceLock<crate::vertrag::Schema> = OnceLock::new();
    static MINOR_1: OnceLock<crate::vertrag::Schema> = OnceLock::new();
    static MINOR_2: OnceLock<crate::vertrag::Schema> = OnceLock::new();
    static MINOR_3: OnceLock<crate::vertrag::Schema> = OnceLock::new();
    static MINOR_4: OnceLock<crate::vertrag::Schema> = OnceLock::new();
    match schema_minor {
        0 => Some(MINOR_0.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_minor_0_wurzel())
                .expect("eingefrorenes v3-Minor-0-Schema ist unterstuetzt")
        })),
        1 => Some(MINOR_1.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_minor_1_wurzel())
                .expect("eingefrorenes v3-Minor-1-Schema ist unterstuetzt")
        })),
        2 => Some(MINOR_2.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_minor_2_wurzel())
                .expect("eingefrorenes v3-Minor-2-Schema ist unterstuetzt")
        })),
        3 => Some(MINOR_3.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_minor_3_wurzel())
                .expect("eingefrorenes v3-Minor-3-Schema ist unterstuetzt")
        })),
        JSON_SCHEMA_MINOR_AKTIV => Some(MINOR_4.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_wurzel())
                .expect("eingefrorenes v3-Minor-4-Schema ist unterstuetzt")
        })),
        _ => None,
    }
}

pub(super) fn v3_nachricht_lesen_mit_minor(
    payload: &[u8],
    erwarteter_typ: &str,
    schema_minor: u8,
) -> Option<Value> {
    let wert = v3_nachricht_lesen_beliebig_mit_minor(payload, schema_minor)?;
    (wert.get("type").and_then(Value::as_str) == Some(erwarteter_typ)).then_some(wert)
}

pub(super) fn v3_nachricht_lesen_beliebig_mit_minor(payload: &[u8], schema_minor: u8) -> Option<Value> {
    crate::vertrag::textriegel_bytes(payload).ok()?;
    let wert: Value = serde_json::from_slice(payload).ok()?;
    v3_schema(schema_minor)?.gueltig(&wert).then_some(wert)
}

pub(super) fn v3_nachricht_lesen(payload: &[u8], erwarteter_typ: &str) -> Option<Value> {
    v3_nachricht_lesen_mit_minor(payload, erwarteter_typ, JSON_SCHEMA_MINOR_AKTIV)
}

pub(super) fn projektion_mit_aktuellem_lauf(gespeichert: &[u8], live: &[u8]) -> Option<Vec<u8>> {
    let mut persistiert = v3_nachricht_lesen(gespeichert, "session_snapshot")?;
    let live = v3_nachricht_lesen(live, "session_snapshot")?;
    let beitritt_bestaetigung_noetig =
        persistiert.get("beitritt_bestaetigung_noetig")?.as_bool()?
            || live.get("beitritt_bestaetigung_noetig")?.as_bool()?;
    let objekt = persistiert.as_object_mut()?;
    // Diese drei Felder gehoeren ausschliesslich zum aktuellen Brokerlauf.
    // Der Bestaetigungsbedarf hat dagegen zwei sichere Quellen: committierte
    // Projektion und fluechtiger Join-Graph. Nur bei zweimal false wird false
    // veroeffentlicht; persistiertes true loescht erst der regulaere C-03-Weg.
    // Der uebrige Projektionsschnitt bleibt die committierte Wirkung.
    for feld in ["broker_epoch", "fuehrendes_main", "mitglieder"] {
        objekt.insert(feld.into(), live.get(feld)?.clone());
    }
    objekt.insert(
        "beitritt_bestaetigung_noetig".into(),
        Value::Bool(beitritt_bestaetigung_noetig),
    );
    serde_json::to_vec(&persistiert).ok()
}

#[cfg(test)]
mod fassungsleiter_tests {
    use super::*;

    fn register() -> Value {
        serde_json::from_str(include_str!(
            "../../../eq-copilot/schemas/v3/reservierte-nachrichten-v1.json"
        ))
        .expect("Register ist JSON")
    }

    fn fixture(name: &str) -> Value {
        let pfad = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
            .join("../eq-copilot/fixtures/v3/gueltig")
            .join(format!("{name}.json"));
        serde_json::from_slice(&std::fs::read(&pfad).expect("Fixture liegt im Korpus"))
            .expect("Fixture ist JSON")
    }

    /// Die Rueckbauliste ist eine KOPIE der Wahrheit aus dem Register. Drei
    /// handgepflegte Kopien derselben Liste sind hier schon einmal
    /// auseinandergelaufen (T2-Runde 2, BF-5) — deshalb faellt die Abweichung
    /// an diesem Riegel und nicht an einem Fixture, das zufaellig darauf zielt.
    /// SONDE-014: dieselbe Regel eine Fassung weiter. Die Rueckbauliste der
    /// Fassung 3 ist eine KOPIE der Wahrheit aus dem Register; laufen sie
    /// auseinander, faellt es hier statt an einem Fixture.
    #[test]
    fn fassung_3_liste_deckt_sich_mit_dem_register() {
        let reg = register();
        let version = &reg["wire_envelope_schema_minor"];
        assert_eq!(
            version["aktuell"].as_u64(),
            Some(u64::from(JSON_SCHEMA_MINOR_AKTIV)),
            "das Register muss dieselbe aktive Fassung fuehren wie der Leser"
        );
        // NAK-213: der Fassungsschritt 4 hebt beide Zahlen um eins. Sie stehen
        // hier, damit ein STILLES Heben faellt — nicht, weil 2/3 richtiger
        // waere als 3/4.
        assert_eq!(version["vorher"].as_u64(), Some(3));

        let fassung = &version["fassungen"]["3"];
        for familie in FASSUNG_3_FAMILIEN {
            assert_eq!(
                fassung[familie].as_bool(),
                Some(true),
                "{familie} fehlt im Fassungseintrag 3"
            );
        }
        assert!(fassung["session_snapshot_findings"].is_string());
        assert!(fassung["experiment_begin_ziel"].is_string());

        let genannt: HashSet<&str> = fassung["neue_definitionen"]
            .as_array()
            .expect("neue_definitionen ist ein Array")
            .iter()
            .filter_map(Value::as_str)
            .collect();
        let gebaut: HashSet<&str> = FASSUNG_3_FAMILIEN
            .iter()
            .chain(FASSUNG_3_HILFSDEFS.iter())
            .copied()
            .collect();
        assert_eq!(genannt, gebaut, "Register und Rueckbauliste laufen auseinander");
    }

    /// Der Kern der Zusage, eine Fassung weiter: der ECHTE Leser der Fassung 2
    /// lehnt JEDE Neuerung der Fassung 3 ab.
    #[test]
    fn minor_2_leser_lehnt_die_fassung_3_ab() {
        let alt = v3_schema(2).expect("Minor 2 ist bekannt");
        // NAK-213: hier steht ausdruecklich `v3_schema(3)` und nicht die
        // aktive Fassung. Die Zusage lautet „Fassung 2 lehnt die Neuerungen
        // der Fassung 3 ab" — sie gegen die jeweils NEUESTE Fassung zu messen
        // waere derselbe Fehler, den SONDE-013 an `MINOR_1` gefunden hat.
        let neu = v3_schema(3).expect("Minor 3 ist bekannt");

        for familie in FASSUNG_3_FAMILIEN {
            let wert = fixture(familie);
            assert!(neu.gueltig(&wert), "{familie} muss in Fassung 3 gelten");
            assert!(
                !alt.gueltig(&wert),
                "{familie} darf in Fassung 2 NICHT gelten"
            );
        }

        let mit_findings = fixture("session-snapshot-mit-findings");
        assert!(neu.gueltig(&mit_findings));
        assert!(!alt.gueltig(&mit_findings));

        let mit_ziel = fixture("experiment-begin-mit-ziel");
        assert!(neu.gueltig(&mit_ziel));
        assert!(!alt.gueltig(&mit_ziel));

        // Gegenprobe: der Rueckbau ist nicht einfach kaputt. Was schon in
        // Fassung 2 galt, gilt dort weiter.
        for name in ["session_snapshot", "experiment_begin", "evidence_snapshot"] {
            let wert = fixture(name);
            assert!(alt.gueltig(&wert), "{name} muss in Fassung 2 weiter gelten");
            assert!(neu.gueltig(&wert));
        }
    }

    /// NAK-213 K-46/K-50, R6/R7: der ECHTE Leser der Fassung 3 lehnt BEIDE
    /// Neuerungen der Fassung 4 ab — den unbekannten Grund wie die zu lange
    /// Ausschlussliste. M-77 verlangt dieselbe Regel in beiden Sprachen
    /// ausdruecklich fuer „jede Laengen- oder Enumgrenze"; die C++-Haelfte
    /// misst B28 (`Sonde014BefundTest`).
    #[test]
    fn fassung_3_kennt_die_neuen_gruende_nicht() {
        let alt = v3_schema(3).expect("Minor 3 ist bekannt");
        let neu = v3_schema(JSON_SCHEMA_MINOR_AKTIV).expect("Minor 4 ist bekannt");

        for name in [
            "finding-ausschlussgrund-screening-ueberboten-in-fassung-3",
            "finding-ausschlussgrund-master-duplikat-in-fassung-3",
        ] {
            let wert = fixture(name);
            assert!(neu.gueltig(&wert), "{name} muss in Fassung 4 gelten");
            assert!(
                !alt.gueltig(&wert),
                "{name} darf in Fassung 3 NICHT gelten — sonst urteilen Rust und C++ verschieden"
            );
        }

        // Die Laengengrenze ist der zweite Griff desselben Rueckbaus (R6).
        // 33 ist der kleinste Ueberlauf ueber die Grenze der Fassung 3.
        let dreiunddreissig = fixture("finding-33-ausschluesse");
        assert!(
            neu.gueltig(&dreiunddreissig),
            "33 Ausschluesse muessen in Fassung 4 gelten"
        );
        assert!(
            !alt.gueltig(&dreiunddreissig),
            "33 Ausschluesse duerfen in Fassung 3 NICHT gelten"
        );

        // Gegenprobe: der Rueckbau ist nicht einfach kaputt. Was schon in
        // Fassung 3 galt, gilt dort weiter — ein zerstoerter Rueckbau saehe
        // sonst wie eine erfuellte Zusage aus.
        for name in ["session-snapshot-mit-findings", "session_snapshot"] {
            let wert = fixture(name);
            assert!(alt.gueltig(&wert), "{name} muss in Fassung 3 weiter gelten");
            assert!(neu.gueltig(&wert));
        }
    }

    /// Die Rueckbauliste der Fassung 4 ist eine KOPIE der Wahrheit aus dem
    /// Register. Laufen sie auseinander, faellt es hier — nicht an einem
    /// Fixture, das zufaellig darauf zielt.
    #[test]
    fn fassung_4_liste_deckt_sich_mit_dem_register() {
        let reg = register();
        let fassung = &reg["wire_envelope_schema_minor"]["fassungen"]["4"];
        let gruende: Vec<&str> = fassung["ausschlussgrund_erweitert"]
            .as_array()
            .expect("Gruendeliste ist ein Array")
            .iter()
            .filter_map(Value::as_str)
            .collect();
        assert_eq!(gruende, FASSUNG_4_GRUENDE.to_vec());
        assert_eq!(
            fassung["session_finding_ausschluesse_max"].as_u64(),
            Some(64),
            "R6: das Register fuehrt dieselbe Grenze wie das Schema"
        );
        // Und der Rueckbau setzt sie wirklich zurueck — sonst waere
        // `FASSUNG_3_AUSSCHLUSS_MAX` eine Konstante ohne Wirkung.
        assert_eq!(
            v3_schema_minor_3_wurzel()["$defs"]["session_finding"]["properties"]["ausschluesse"]
                ["maxItems"]
                .as_u64(),
            Some(FASSUNG_3_AUSSCHLUSS_MAX)
        );
    }

    /// Die Kette reicht bis nach unten: der Leser der Fassung 1 und der
    /// aelteste Leser kennen die Fassung 3 erst recht nicht.
    #[test]
    fn fassung_1_und_0_erben_den_rueckbau_der_fassung_3() {
        let eins = v3_schema(1).expect("Minor 1 ist bekannt");
        let null = v3_schema(0).expect("Minor 0 ist bekannt");
        for familie in FASSUNG_3_FAMILIEN {
            let wert = fixture(familie);
            assert!(!eins.gueltig(&wert), "{familie} darf in Fassung 1 NICHT gelten");
            assert!(!null.gueltig(&wert), "{familie} darf in Fassung 0 NICHT gelten");
        }
    }

    /// M-01/M-77: die geschlossene Rollenmenge lebt an EINER Stelle im
    /// Vertrag, und der Rust-Leser prueft gegen dieselbe wie der C++-Leser
    /// des Main-States (Bein B27). Dasselbe Muster wie
    /// `p2_reject_katalog_stimmt_mit_dem_strikten_wire_schema`.
    #[test]
    fn geschlossene_mengen_der_fassung_3_stehen_je_einmal_im_vertrag() {
        let wurzel = v3_schema_wurzel();
        let defs = &wurzel["$defs"];

        let rollen: Vec<&str> = defs["intent_rolle"]["enum"]
            .as_array()
            .unwrap()
            .iter()
            .filter_map(Value::as_str)
            .collect();
        assert_eq!(
            rollen,
            vec!["fuehrt", "traegt", "begleitet", "geschuetzt", "verschmolzen"],
            "U22: genau fuenf Rollen, in dieser Reihenfolge"
        );

        let gruende: Vec<&str> = defs["ausschlussgrund"]["enum"]
            .as_array()
            .unwrap()
            .iter()
            .filter_map(Value::as_str)
            .collect();
        assert_eq!(
            gruende.len(),
            10,
            "R4 plus NAK-213 R1/R3: zehn Ausschlussgruende"
        );
        assert!(gruende.contains(&"passage_zu_kurz"));
        assert!(gruende.contains(&"evidenz_zurueckgenommen"));
        // Die zwei neuen stehen AM ENDE, in derselben Reihenfolge wie in
        // `AUSSCHLUSSGRUENDE` — `wire()`/`aus_wire()` gehen ueber den Index.
        assert_eq!(&gruende[8..], &["screening_ueberboten", "master_duplikat"]);
        assert_eq!(
            defs["session_finding"]["properties"]["ausschluesse"]["maxItems"].as_u64(),
            Some(64),
            "R6: die Ausschlussliste wird nie gekappt, ihre Grenze ist SESSION_CLIENT_CAP"
        );

        assert_eq!(
            defs["ursachenklasse"]["enum"].as_array().unwrap().len(),
            7,
            "Entwurf Paragraph 8 zaehlt sieben Ursachenklassen ab"
        );
        assert_eq!(
            defs["proposal_revert"]["enum"]
                .as_array()
                .unwrap()
                .iter()
                .filter_map(Value::as_str)
                .collect::<Vec<_>>(),
            vec!["experiment_abort", "manual_only", "none_needed"],
            "E-06: drei Rueckwege, dsp_revert gehoert P7"
        );
    }

    #[test]
    fn fassung_2_liste_deckt_sich_mit_dem_register() {
        let reg = register();
        let fassung = &reg["wire_envelope_schema_minor"]["fassungen"]["2"];
        for familie in FASSUNG_2_FAMILIEN {
            assert_eq!(
                fassung[familie].as_bool(),
                Some(true),
                "{familie} fehlt im Fassungseintrag 2"
            );
        }
        let gruende: Vec<&str> = fassung["evidence_invalidate_grund_erweitert"]
            .as_array()
            .expect("Gruendeliste ist ein Array")
            .iter()
            .filter_map(Value::as_str)
            .collect();
        assert_eq!(gruende, FASSUNG_2_GRUENDE.to_vec());
        assert_eq!(fassung["evidence_snapshot_ereignisse"].as_bool(), Some(true));
        assert_eq!(fassung["evidence_snapshot_stereo"].as_bool(), Some(true));
    }

    /// Der Kern der Zusage „nicht additiv erweiterbar": der ECHTE Leser der
    /// Fassung 1 — derselbe, den `v3_schema(1)` liefert — lehnt jede Neuerung
    /// der Fassung 2 ab, statt sie still auf einen bekannten Zweig
    /// abzubilden. Ohne diesen Test waere die Regel eine Behauptung: bis
    /// SONDE-013 lud `MINOR_1` in Wahrheit das jeweils NEUESTE Schema.
    #[test]
    fn minor_1_leser_lehnt_die_fassung_2_ab() {
        let alt = v3_schema(1).expect("Minor 1 ist bekannt");
        // SONDE-014: hier steht ausdruecklich `v3_schema(2)` und nicht die
        // aktive Fassung. Die Zusage lautet „Fassung 1 lehnt die Neuerungen
        // der Fassung 2 ab" — sie gegen die jeweils NEUESTE Fassung zu messen
        // waere derselbe Fehler, den SONDE-013 an `MINOR_1` gefunden hat, nur
        // auf der anderen Seite des Vergleichs.
        let neu = v3_schema(2).expect("Minor 2 ist bekannt");

        for familie in FASSUNG_2_FAMILIEN {
            let wert = fixture(familie);
            assert!(neu.gueltig(&wert), "{familie} muss in Fassung 2 gelten");
            assert!(
                !alt.gueltig(&wert),
                "{familie} darf in Fassung 1 NICHT gelten"
            );
        }

        for grund in FASSUNG_2_GRUENDE {
            let mut wert = fixture("evidence_invalidate");
            wert["grund"] = Value::String(grund.into());
            assert!(neu.gueltig(&wert), "{grund} muss in Fassung 2 gelten");
            assert!(!alt.gueltig(&wert), "{grund} darf in Fassung 1 NICHT gelten");
        }

        let mit_inhalt = fixture("evidence-snapshot-mit-ereignissen-und-stereo");
        assert!(neu.gueltig(&mit_inhalt));
        assert!(!alt.gueltig(&mit_inhalt));

        // Gegenprobe: der alte Leser ist nicht einfach kaputt. Was schon in
        // Fassung 1 galt, gilt dort weiter — sonst saehe ein zerstoerter
        // Rueckbau wie eine erfuellte Zusage aus.
        let alt_gueltig = fixture("evidence_snapshot");
        assert!(alt.gueltig(&alt_gueltig));
        assert!(neu.gueltig(&alt_gueltig));
    }

    /// Die Rueckbauten sind eine KETTE. Wer `v3_schema_minor_0_wurzel` auf die
    /// committete Fassung aufsetzte, gaebe dem AELTESTEN Leser alle Neuerungen
    /// aller spaeteren Fassungen mit.
    #[test]
    fn minor_0_erbt_den_rueckbau_der_fassung_1() {
        let null = v3_schema(0).expect("Minor 0 ist bekannt");
        for familie in FASSUNG_2_FAMILIEN {
            assert!(
                !null.gueltig(&fixture(familie)),
                "{familie} darf in Fassung 0 NICHT gelten"
            );
        }
        let mut wert = fixture("evidence_invalidate");
        wert["grund"] = Value::String(FASSUNG_2_GRUENDE[0].into());
        assert!(!null.gueltig(&wert));
    }

    /// Eine Fassung, die es nicht gibt, ist kein Leser — und wird nicht zur
    /// naechstbesten gemacht.
    #[test]
    fn unbekannte_fassung_hat_keinen_leser() {
        assert!(v3_schema(JSON_SCHEMA_MINOR_AKTIV + 1).is_none());
        assert!(v3_schema(200).is_none());
    }
}
