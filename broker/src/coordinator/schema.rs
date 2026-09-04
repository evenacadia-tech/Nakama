//! Der v3-Vertrag als Leser, ohne Sessionwissen.
//!
//! Das eingefrorene Schema wird einmal geladen und danach nur noch befragt;
//! die Minorfassungen bleiben nebeneinander lesbar.

use super::*;

pub(super) const JSON_SCHEMA_MINOR_AKTIV: u8 = 2;

/// Die Familien und Felder, die die **Fassung 2** ausmachen (SONDE-013).
///
/// Sie stehen hier NICHT als Kommentar, sondern als Daten: der Test
/// `fassung_2_liste_deckt_sich_mit_dem_register` vergleicht sie mit dem
/// Eintrag `wire_envelope_schema_minor.fassungen."2"` in
/// `reservierte-nachrichten-v1.json`. Drei handgepflegte Kopien derselben
/// Liste sind in diesem Projekt schon einmal auseinandergelaufen
/// (T2-Runde 2, BF-5) — hier faellt die Abweichung am Riegel statt an einem
/// Fixture, das zufaellig darauf zielt.
const FASSUNG_2_FAMILIEN: [&str; 3] = [
    "experiment_begin",
    "experiment_abort",
    "experiment_manual_result",
];

/// Hilfsdefinitionen, die ausschliesslich von der Fassung 2 gebraucht werden.
/// Sie tragen keinen eigenen `oneOf`-Zweig und stehen deshalb nicht im
/// Register; ohne ihre Familien sind sie aber unerreichbar, und ein `$ref`
/// ins Leere bricht den Ladelauf.
const FASSUNG_2_HILFSDEFS: [&str; 10] = [
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
const FASSUNG_2_GRUENDE: [&str; 2] = ["material_wechsel", "messpunkt_wechsel"];
/// Die Fehlercodes der Fassung 2 (SONDE-013 Nacharbeit 1). Sie benennen die
/// Produktregeln der Experimentfamilien; ein Leser der Fassung 1 kennt sie
/// nicht und muss sie ABLEHNEN, statt sie still auf `internal` abzubilden.
const FASSUNG_2_FEHLERCODES: [&str; 5] = [
    "abdeckung_zu_gering",
    "schon_terminal",
    "ohne_lautheitsabgleich",
    "ohne_resultatmessung",
    "blindreihenfolge_widerspruch",
];

/// Baut aus der committeten **Fassung 2** die **Fassung 1** zurueck.
///
/// Der Vertrag ist an Discriminator, Zieladresse, Revision und Capability
/// NICHT additiv erweiterbar: ein Leser der alten Fassung muss jede Neuerung
/// ABLEHNEN, statt sie still auf einen bekannten Zweig abzubilden. Genau das
/// kann er nur, wenn seine Fassung wirklich existiert — eine Konstante
/// `MINOR_1`, die in Wahrheit das neueste Schema laedt, waere das Gegenteil
/// der Zusage.
pub(super) fn v3_schema_minor_1_wurzel() -> Value {
    let mut wurzel = v3_schema_wurzel();
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
    match schema_minor {
        0 => Some(MINOR_0.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_minor_0_wurzel())
                .expect("eingefrorenes v3-Minor-0-Schema ist unterstuetzt")
        })),
        1 => Some(MINOR_1.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_minor_1_wurzel())
                .expect("eingefrorenes v3-Minor-1-Schema ist unterstuetzt")
        })),
        JSON_SCHEMA_MINOR_AKTIV => Some(MINOR_2.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_wurzel())
                .expect("eingefrorenes v3-Minor-2-Schema ist unterstuetzt")
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
    #[test]
    fn fassung_2_liste_deckt_sich_mit_dem_register() {
        let reg = register();
        let fassung = &reg["wire_envelope_schema_minor"]["fassungen"]["2"];
        assert_eq!(
            reg["wire_envelope_schema_minor"]["aktuell"].as_u64(),
            Some(u64::from(JSON_SCHEMA_MINOR_AKTIV))
        );
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
        let neu = v3_schema(JSON_SCHEMA_MINOR_AKTIV).expect("Minor 2 ist bekannt");

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
