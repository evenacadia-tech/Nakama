//! Der v3-Vertrag als Leser, ohne Sessionwissen.
//!
//! Das eingefrorene Schema wird einmal geladen und danach nur noch befragt;
//! die Minorfassungen bleiben nebeneinander lesbar.

use super::*;

pub(super) const JSON_SCHEMA_MINOR_AKTIV: u8 = 1;

pub(super) fn v3_schema_wurzel() -> Value {
    serde_json::from_str(include_str!(
        "../../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
    ))
    .expect("eingefrorenes v3-Schema ist JSON")
}

pub(super) fn v3_schema_minor_0_wurzel() -> Value {
    let mut wurzel = v3_schema_wurzel();
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
    match schema_minor {
        0 => Some(MINOR_0.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_minor_0_wurzel())
                .expect("eingefrorenes v3-Minor-0-Schema ist unterstuetzt")
        })),
        JSON_SCHEMA_MINOR_AKTIV => Some(MINOR_1.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_wurzel())
                .expect("eingefrorenes v3-Minor-1-Schema ist unterstuetzt")
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
