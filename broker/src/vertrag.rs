//! Rust-Bein des v3-Vertrags (SONDE-005a).
//!
//! Eine JSON-Schema-Engine auf genau der Teilmenge, die
//! `eq-copilot/schemas/v3/README.md` festhaelt — mehr nicht, und alles
//! ausserhalb bricht beim LADEN statt still zu wirken.
//!
//! Warum keine fertige Schema-Crate: die Anforderung ist nicht "irgendwie
//! validieren", sondern "C++ und Rust klassifizieren jedes Fixture IDENTISCH"
//! (Entwurf §65). Zwei fremde Bibliotheken waeren sich in Randfaellen einig
//! oder auch nicht, und niemand koennte sagen, welche recht hat. Eine kleine
//! Engine gegen einen geschriebenen Regelsatz ist pruefbar; eine grosse
//! fremde Engine ist es nicht. Der Preis ist diese Datei, der Gewinn ist ein
//! Vertrag, dessen Auslegung im Repository steht.
//!
//! Die C++-Gegenseite ist `eq-copilot/plugin/vertrag/NakamaVertrag.*`. Beide
//! messen gegen `eq-copilot/fixtures/v3/MANIFEST.json`.

use serde_json::Value;
use std::collections::BTreeSet;

/// Eine einzelne Vertragsverletzung.
///
/// `schema` ist der AUFGELOESTE Pfad — ueber `$ref` hinweg. Sonst haetten die
/// beiden Engines fuer denselben Fehler zwei verschiedene Texte, je nachdem
/// ob sie die Referenz vor oder nach dem Bilden des Pfades aufloesen.
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct Verletzung {
    pub instanz: String,
    pub schema: String,
    pub schluessel: String,
}

impl Verletzung {
    fn neu(instanz: &str, schema: &str, schluessel: &str) -> Self {
        Verletzung {
            instanz: instanz.to_string(),
            schema: schema.to_string(),
            schluessel: schluessel.to_string(),
        }
    }
}

/// Genau die Liste aus dem README. Wer sie erweitert, erweitert auch die
/// C++-Engine und das Referenzbein — sonst faellt eine der drei Pruefungen.
const SCHLUESSELWOERTER: &[&str] = &[
    "$ref", "type", "const", "enum", "required", "properties",
    "additionalProperties", "maxProperties", "minimum", "maximum",
    "exclusiveMinimum", "exclusiveMaximum", "minLength", "maxLength",
    "pattern", "items", "minItems", "maxItems", "oneOf",
    "x-nakama-discriminator",
];

const ANMERKUNGEN: &[&str] = &["$schema", "$id", "title", "description", "$comment", "$defs"];

/// Geschlossene Mustertabelle. Regex ist zwischen Python, C++ und Rust nicht
/// in jeder Ecke gleich; deshalb gibt es hier keine Regex-Auswertung, sondern
/// benannte Muster. Ein unbekanntes Muster bricht den Ladevorgang.
fn muster_passt(muster: &str, wert: &str) -> Option<bool> {
    match muster {
        "^[0-9a-f]{32}$" => Some(
            wert.len() == 32 && wert.bytes().all(|b| b.is_ascii_digit() || (b'a'..=b'f').contains(&b)),
        ),
        _ => None,
    }
}

#[derive(Debug)]
pub struct Schema {
    wurzel: Value,
}

impl Schema {
    /// Laedt und prueft das Schema selbst. Ein Schluesselwort oder Muster
    /// ausserhalb der Teilmenge ist ein FEHLER, kein zu ignorierender Zusatz —
    /// JSON Schema uebergeht Unbekanntes absichtlich, und genau dadurch
    /// verschwaende eine spaeter ergaenzte Einschraenkung hier still.
    pub fn laden(wurzel: Value) -> Result<Schema, String> {
        let mut fehler = Vec::new();
        pruefe_teilschema(&wurzel, &wurzel, "#", &mut fehler);
        if fehler.is_empty() {
            Ok(Schema { wurzel })
        } else {
            Err(fehler.join("; "))
        }
    }

    /// Alle Verletzungen des gewaehlten Zweiges, kanonisch sortiert.
    pub fn pruefe(&self, daten: &Value) -> Vec<Verletzung> {
        let mut out = Vec::new();
        pruefe_wert(&self.wurzel, "#", daten, "", &self.wurzel, &mut out);
        let mengen: BTreeSet<Verletzung> = out.into_iter().collect();
        mengen.into_iter().collect()
    }

    pub fn gueltig(&self, daten: &Value) -> bool {
        self.pruefe(daten).is_empty()
    }
}

// ---------------------------------------------------------------- Ladepruefung

/// Welchen Werttyp verlangt ein Schluesselwort?
///
/// Die Ladepruefung sah bis T2-Runde 1 nur NAMEN. Gemessen: `"maxLength": 5.0`
/// wurde hier still verworfen (`as_u64()` auf einer Float-Zahl ist `None`) und
/// von der C++-Engine durchgesetzt — dieselbe Fehlerklasse wie ein unbekanntes
/// Schluesselwort, nur eine Ebene tiefer.
fn werttyp_passt(name: &str, wert: &Value) -> bool {
    match name {
        // `type` ist String ODER nicht-leeres Array von Strings. "beliebig"
        // waere hier ein neues Divergenzloch: bei `"type": 5` liest die
        // C++-Engine ueber toString() die Zeichenkette "5", diese hier
        // bekaeme von as_str() ein None — zwei Engines, zwei Urteile.
        "type" => {
            wert.is_string()
                || wert
                    .as_array()
                    .is_some_and(|a| !a.is_empty() && a.iter().all(|e| e.is_string()))
        }
        // `const` ist bewusst beliebig: es vergleicht gegen einen Wert, und
        // jeder JSON-Wert ist ein zulaessiges Vergleichsziel.
        "const" => true,
        "enum" | "oneOf" => wert.as_array().is_some_and(|a| !a.is_empty()),
        "required" => wert
            .as_array()
            .is_some_and(|a| a.iter().all(|e| e.is_string())),
        "properties" | "$defs" | "items" => wert.is_object(),
        "additionalProperties" => wert.is_boolean(),
        // `5.0` ist hier bewusst KEINE Ganzzahl: JSON Schema erlaubt sie, aber
        // die beiden Engines lesen sie verschieden. Wir verlangen die Form,
        // ueber die sie sich nicht streiten koennen.
        "maxProperties" | "minLength" | "maxLength" | "minItems" | "maxItems" => {
            wert.as_u64().is_some()
        }
        "minimum" | "maximum" | "exclusiveMinimum" | "exclusiveMaximum" => wert.is_number(),
        "pattern" | "$ref" | "x-nakama-discriminator" => wert.is_string(),
        _ => true,
    }
}

fn pruefe_teilschema(wurzel: &Value, knoten: &Value, pfad: &str, fehler: &mut Vec<String>) {
    let Some(obj) = knoten.as_object() else { return };

    for (name, wert) in obj {
        if ANMERKUNGEN.contains(&name.as_str()) {
            continue;
        }
        if !SCHLUESSELWOERTER.contains(&name.as_str()) {
            fehler.push(format!("unbekanntes Schluesselwort {pfad}/{name}"));
            continue;
        }
        if !werttyp_passt(name, wert) {
            fehler.push(format!("falscher Werttyp fuer {pfad}/{name}"));
        }
    }

    if let Some(m) = obj.get("pattern").and_then(|v| v.as_str()) {
        if muster_passt(m, "").is_none() {
            fehler.push(format!("unbekanntes Muster {pfad}: {m}"));
        }
    }
    if obj.contains_key("oneOf") && !obj.contains_key("x-nakama-discriminator") {
        fehler.push(format!("oneOf ohne x-nakama-discriminator bei {pfad}"));
    }
    if let Some(r) = obj.get("$ref").and_then(|v| v.as_str()) {
        if !r.starts_with("#/$defs/") {
            fehler.push(format!("nicht-lokale Referenz bei {pfad}: {r}"));
        } else {
            // T2-Runde 1: bis hierher wurde nur das PRAEFIX geprueft. Ein
            // haengender $ref liess `aufloesen()` auf den Originalknoten
            // zurueckfallen — und damit blieb der ganze Teilbaum still
            // UNGEPRUEFT. Dieselbe Klasse wie ein uebergangenes
            // Schluesselwort, nur schlimmer: es verschwindet nicht eine
            // Einschraenkung, sondern jede des Zieles.
            let name = &r["#/$defs/".len()..];
            if wurzel.get("$defs").and_then(|d| d.get(name)).is_none() {
                fehler.push(format!("haengende Referenz bei {pfad}: {r} hat kein Ziel"));
            }
        }
    }
    if let Some(a) = obj.get("additionalProperties") {
        if a.as_bool() == Some(true) && !obj.contains_key("maxProperties") {
            fehler.push(format!("additives Objekt {pfad} ohne maxProperties"));
        }
    }

    for (name, wert) in obj {
        match name.as_str() {
            "properties" | "$defs" => {
                if let Some(kinder) = wert.as_object() {
                    for (k, v) in kinder {
                        pruefe_teilschema(wurzel, v, &format!("{pfad}/{name}/{k}"), fehler);
                    }
                }
            }
            "items" => pruefe_teilschema(wurzel, wert, &format!("{pfad}/items"), fehler),
            "oneOf" => {
                if let Some(zweige) = wert.as_array() {
                    for (i, v) in zweige.iter().enumerate() {
                        pruefe_teilschema(wurzel, v, &format!("{pfad}/oneOf/{i}"), fehler);
                    }
                }
            }
            _ => {}
        }
    }
}

// ------------------------------------------------------------------ Aufloesung

/// Loest `$ref` auf und liefert (Zielknoten, Zielpfad). Ein Knoten mit `$ref`
/// wird VOLLSTAENDIG ersetzt; danebenstehende Anmerkungen ($comment) tragen
/// keine Bedeutung.
fn aufloesen<'a>(wurzel: &'a Value, knoten: &'a Value, pfad: &str) -> (&'a Value, String) {
    let Some(r) = knoten.get("$ref").and_then(|v| v.as_str()) else {
        return (knoten, pfad.to_string());
    };
    let name = &r["#/$defs/".len()..];
    match wurzel.get("$defs").and_then(|d| d.get(name)) {
        Some(ziel) => (ziel, format!("#/$defs/{name}")),
        // Kann nach der Ladepruefung nicht mehr vorkommen; kein Panic im
        // Bibliothekspfad.
        None => (knoten, pfad.to_string()),
    }
}

/// Welchen Wert der Eigenschaft `disc` legt dieser Zweig fest?
///
/// Normalerweise steht er als `properties/<disc>/const`. Ein Zweig darf aber
/// selbst wieder eine diskriminierte Union sein — `hello` ist genau das: es
/// haelt keine eigenen `properties`, sondern zwei Varianten, die BEIDE
/// `type: "hello"` festlegen. Ohne diesen Abstieg fiele der aeussere
/// Discriminator ins Leere und die ganze Familie waere unerreichbar.
///
/// Der Wert gilt nur, wenn ALLE Untervarianten denselben festlegen; sonst
/// waere die Zuordnung mehrdeutig und der Zweig kommt nicht in Frage.
fn diskriminatorwert(wurzel: &Value, zweig: &Value, disc: &str) -> Option<String> {
    if let Some(c) = zweig
        .get("properties")
        .and_then(|p| p.get(disc))
        .and_then(|d| d.get("const"))
        .and_then(|c| c.as_str())
    {
        return Some(c.to_string());
    }
    let unter = zweig.get("oneOf")?.as_array()?;
    let mut gemeinsam: Option<String> = None;
    for u in unter {
        let (ziel, _) = aufloesen(wurzel, u, "#");
        let w = diskriminatorwert(wurzel, ziel, disc)?;
        match &gemeinsam {
            None => gemeinsam = Some(w),
            Some(g) if *g == w => {}
            Some(_) => return None,
        }
    }
    gemeinsam
}

// ------------------------------------------------------------------ Vergleiche

/// JSON-Gleichheit mit ZAHLENSEMANTIK: 3 und 3.0 sind derselbe Wert.
/// serde_json unterscheidet sie strukturell, JSON Schema nicht.
fn gleich(a: &Value, b: &Value) -> bool {
    match (a, b) {
        (Value::Number(x), Value::Number(y)) => match (x.as_f64(), y.as_f64()) {
            (Some(p), Some(q)) => p == q,
            _ => x == y,
        },
        (Value::Array(x), Value::Array(y)) => {
            x.len() == y.len() && x.iter().zip(y).all(|(p, q)| gleich(p, q))
        }
        (Value::Object(x), Value::Object(y)) => {
            x.len() == y.len()
                && x.iter().all(|(k, p)| y.get(k).map_or(false, |q| gleich(p, q)))
        }
        _ => a == b,
    }
}

fn typ_passt(name: &str, wert: &Value) -> bool {
    match name {
        "object" => wert.is_object(),
        "array" => wert.is_array(),
        "string" => wert.is_string(),
        "boolean" => wert.is_boolean(),
        "null" => wert.is_null(),
        "number" => wert.is_number(),
        // draft 2020-12: jede Zahl OHNE Nachkommateil ist ein integer,
        // also auch 1.0. Nicht `is_i64()` — das waere enger als der Standard.
        "integer" => wert
            .as_f64()
            .map(|f| f.is_finite() && f.fract() == 0.0)
            .unwrap_or(false),
        _ => false,
    }
}

/// Zeichenlaenge nach JSON-Schema-Regel: CODEPUNKTE, nicht Bytes und nicht
/// UTF-16-Einheiten. `"Klavier-Bus"` ist 11, `"Übergänge"` ist 9 — in UTF-8
/// waeren es 11 Bytes.
fn codepunkte(s: &str) -> usize {
    s.chars().count()
}

fn pfad_plus(pfad: &str, teil: &str) -> String {
    format!("{pfad}/{}", teil.replace('~', "~0").replace('/', "~1"))
}

// ------------------------------------------------------------------ Pruefwerk

fn pruefe_wert(
    knoten: &Value,
    schema_pfad: &str,
    daten: &Value,
    instanz: &str,
    wurzel: &Value,
    out: &mut Vec<Verletzung>,
) {
    let (knoten, schema_pfad_besitzt) = aufloesen(wurzel, knoten, schema_pfad);
    let sp = schema_pfad_besitzt.as_str();
    let Some(obj) = knoten.as_object() else { return };

    // --- oneOf mit Discriminator ---------------------------------------
    if let Some(zweige) = obj.get("oneOf").and_then(|v| v.as_array()) {
        let disc = obj
            .get("x-nakama-discriminator")
            .and_then(|v| v.as_str())
            .unwrap_or("type");
        let wert = daten.get(disc).and_then(|v| v.as_str());
        // Den aufgeloesten Zweig gleich MITNEHMEN statt ihn aus dem Pfad
        // zurueckzurechnen: ein Zweig ohne `$ref` haette sonst einen Pfad
        // ohne "#/$defs/"-Praefix, und das Zurueckschneiden waere ein Panic
        // im Bibliothekspfad.
        let treffer = wert.and_then(|w| {
            zweige.iter().enumerate().find_map(|(i, z)| {
                let (ziel, zpfad) = aufloesen(wurzel, z, &format!("{sp}/oneOf/{i}"));
                (diskriminatorwert(wurzel, ziel, disc) == Some(w.to_string()))
                    .then_some((ziel, zpfad))
            })
        });
        match treffer {
            Some((ziel, zpfad)) => pruefe_wert(ziel, &zpfad, daten, instanz, wurzel, out),
            None => {
                // Ist die Instanz gar kein Objekt, gibt es keine Eigenschaft,
                // auf die der Pfad zeigen koennte — dann zeigt er auf die
                // Instanz selbst. Ein "/type" an einem blossen String waere
                // ein Pfad, den es nicht gibt.
                let ort = if daten.is_object() {
                    pfad_plus(instanz, disc)
                } else {
                    instanz.to_string()
                };
                out.push(Verletzung::neu(&ort, &format!("{sp}/oneOf"), "oneOf"));
            }
        }
        return;
    }

    // --- type ----------------------------------------------------------
    if let Some(t) = obj.get("type") {
        let ok = match t {
            Value::String(s) => typ_passt(s, daten),
            Value::Array(a) => a.iter().any(|x| x.as_str().map_or(false, |s| typ_passt(s, daten))),
            _ => false,
        };
        if !ok {
            out.push(Verletzung::neu(instanz, &format!("{sp}/type"), "type"));
            // KEIN Abstieg nach einem Typfehler: sonst haengt an einem
            // falschen Typ eine Lawine von Folgefehlern, ueber deren
            // Reihenfolge sich beide Engines einigen muessten.
            return;
        }
    }

    // --- const / enum ---------------------------------------------------
    if let Some(c) = obj.get("const") {
        if !gleich(c, daten) {
            out.push(Verletzung::neu(instanz, &format!("{sp}/const"), "const"));
        }
    }
    if let Some(e) = obj.get("enum").and_then(|v| v.as_array()) {
        if !e.iter().any(|x| gleich(x, daten)) {
            out.push(Verletzung::neu(instanz, &format!("{sp}/enum"), "enum"));
        }
    }

    // --- Zahlen ----------------------------------------------------------
    if let Some(z) = daten.as_f64() {
        for (schluessel, pruefung) in [
            ("minimum", (|a: f64, b: f64| a >= b) as fn(f64, f64) -> bool),
            ("maximum", |a, b| a <= b),
            ("exclusiveMinimum", |a, b| a > b),
            ("exclusiveMaximum", |a, b| a < b),
        ] {
            if let Some(g) = obj.get(schluessel).and_then(|v| v.as_f64()) {
                if !pruefung(z, g) {
                    out.push(Verletzung::neu(instanz, &format!("{sp}/{schluessel}"), schluessel));
                }
            }
        }
    }

    // --- Strings ----------------------------------------------------------
    if let Some(s) = daten.as_str() {
        let n = codepunkte(s);
        if let Some(g) = obj.get("minLength").and_then(|v| v.as_u64()) {
            if (n as u64) < g {
                out.push(Verletzung::neu(instanz, &format!("{sp}/minLength"), "minLength"));
            }
        }
        if let Some(g) = obj.get("maxLength").and_then(|v| v.as_u64()) {
            if (n as u64) > g {
                out.push(Verletzung::neu(instanz, &format!("{sp}/maxLength"), "maxLength"));
            }
        }
        if let Some(m) = obj.get("pattern").and_then(|v| v.as_str()) {
            if muster_passt(m, s) == Some(false) {
                out.push(Verletzung::neu(instanz, &format!("{sp}/pattern"), "pattern"));
            }
        }
    }

    // --- Objekte -----------------------------------------------------------
    if let Some(o) = daten.as_object() {
        if let Some(pflicht) = obj.get("required").and_then(|v| v.as_array()) {
            for feld in pflicht.iter().filter_map(|v| v.as_str()) {
                if !o.contains_key(feld) {
                    // Ein fehlender Wert hat keinen eigenen Instanzpfad —
                    // deshalb zeigt `instanz` auf das ELTERNobjekt und der
                    // Name steht im Schemapfad.
                    out.push(Verletzung::neu(
                        instanz,
                        &format!("{sp}/required/{feld}"),
                        "required",
                    ));
                }
            }
        }
        if let Some(g) = obj.get("maxProperties").and_then(|v| v.as_u64()) {
            if o.len() as u64 > g {
                out.push(Verletzung::neu(
                    instanz,
                    &format!("{sp}/maxProperties"),
                    "maxProperties",
                ));
            }
        }
        let deklariert = obj.get("properties").and_then(|v| v.as_object());
        if obj.get("additionalProperties").and_then(|v| v.as_bool()) == Some(false) {
            for name in o.keys() {
                if !deklariert.map_or(false, |d| d.contains_key(name)) {
                    out.push(Verletzung::neu(
                        &pfad_plus(instanz, name),
                        &format!("{sp}/additionalProperties"),
                        "additionalProperties",
                    ));
                }
            }
        }
        if let Some(d) = deklariert {
            for (name, teil) in d {
                if let Some(wert) = o.get(name) {
                    pruefe_wert(
                        teil,
                        &format!("{sp}/properties/{name}"),
                        wert,
                        &pfad_plus(instanz, name),
                        wurzel,
                        out,
                    );
                }
            }
        }
    }

    // --- Arrays -------------------------------------------------------------
    if let Some(a) = daten.as_array() {
        if let Some(g) = obj.get("minItems").and_then(|v| v.as_u64()) {
            if (a.len() as u64) < g {
                out.push(Verletzung::neu(instanz, &format!("{sp}/minItems"), "minItems"));
            }
        }
        if let Some(g) = obj.get("maxItems").and_then(|v| v.as_u64()) {
            if a.len() as u64 > g {
                out.push(Verletzung::neu(instanz, &format!("{sp}/maxItems"), "maxItems"));
            }
        }
        if let Some(teil) = obj.get("items") {
            for (i, wert) in a.iter().enumerate() {
                pruefe_wert(
                    teil,
                    &format!("{sp}/items"),
                    wert,
                    &format!("{instanz}/{i}"),
                    wurzel,
                    out,
                );
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    fn schema() -> Schema {
        Schema::laden(json!({
            "x-nakama-discriminator": "type",
            "oneOf": [{ "$ref": "#/$defs/a" }],
            "$defs": {
                "a": {
                    "type": "object",
                    "required": ["type", "n"],
                    "additionalProperties": false,
                    "properties": {
                        "type": { "const": "a" },
                        "n": { "type": "integer", "minimum": 0 },
                        "s": { "type": "string", "maxLength": 3 }
                    }
                }
            }
        }))
        .unwrap()
    }

    #[test]
    fn unbekanntes_schluesselwort_bricht_das_laden() {
        let f = Schema::laden(json!({ "type": "object", "multipleOf": 2 })).unwrap_err();
        assert!(f.contains("multipleOf"), "{f}");
    }

    // --- T2-Runde 1 -------------------------------------------------------
    // Beide Riegel schliessen dieselbe Klasse wie "unbekanntes Schluesselwort",
    // nur eine Ebene tiefer: etwas im Schema, das eine Engine anders liest als
    // die andere, ohne dass jemand es merkt. Die C++-Gegenstuecke stehen in
    // SchemaTestMain.cpp / fahreRiegelproben().

    #[test]
    fn haengende_referenz_bricht_das_laden() {
        let f = Schema::laden(json!({
            "$ref": "#/$defs/gibtsnicht",
            "$defs": { "a": { "type": "object" } }
        }))
        .unwrap_err();
        assert!(f.contains("haengende Referenz"), "{f}");
    }

    #[test]
    fn maxlength_als_gleitkommazahl_bricht_das_laden() {
        let f = Schema::laden(json!({ "type": "string", "maxLength": 5.0 })).unwrap_err();
        assert!(f.contains("Werttyp"), "{f}");
    }

    #[test]
    fn type_als_zahl_bricht_das_laden() {
        let f = Schema::laden(json!({ "type": 5 })).unwrap_err();
        assert!(f.contains("Werttyp"), "{f}");
    }

    #[test]
    fn required_mit_nicht_string_bricht_das_laden() {
        let f = Schema::laden(json!({ "type": "object", "required": [5] })).unwrap_err();
        assert!(f.contains("Werttyp"), "{f}");
    }

    #[test]
    fn discriminator_als_zahl_bricht_das_laden() {
        let f = Schema::laden(json!({
            "type": "object",
            "x-nakama-discriminator": 7,
            "oneOf": [{ "type": "object" }]
        }))
        .unwrap_err();
        assert!(f.contains("Werttyp"), "{f}");
    }

    #[test]
    fn oneof_ohne_discriminator_bricht_das_laden() {
        let f = Schema::laden(json!({ "oneOf": [] })).unwrap_err();
        assert!(f.contains("x-nakama-discriminator"), "{f}");
    }

    #[test]
    fn additiv_ohne_maxproperties_bricht_das_laden() {
        let f = Schema::laden(json!({ "type": "object", "additionalProperties": true })).unwrap_err();
        assert!(f.contains("maxProperties"), "{f}");
    }

    #[test]
    fn integer_akzeptiert_1punkt0() {
        assert!(schema().gueltig(&json!({ "type": "a", "n": 1.0 })));
        assert!(!schema().gueltig(&json!({ "type": "a", "n": 1.5 })));
    }

    #[test]
    fn typfehler_erzeugt_keine_lawine() {
        // `s` ist kein String -> genau EINE Verletzung, kein maxLength dazu.
        let v = schema().pruefe(&json!({ "type": "a", "n": 1, "s": 12345 }));
        assert_eq!(v.len(), 1, "{v:?}");
        assert_eq!(v[0].schluessel, "type");
    }

    #[test]
    fn unbekannter_discriminator_wird_abgelehnt() {
        let v = schema().pruefe(&json!({ "type": "b" }));
        assert_eq!(v, vec![Verletzung::neu("/type", "#/oneOf", "oneOf")]);
    }

    #[test]
    fn fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt() {
        let v = schema().pruefe(&json!({ "type": "a" }));
        assert_eq!(v, vec![Verletzung::neu("", "#/$defs/a/required/n", "required")]);
    }

    #[test]
    fn laenge_zaehlt_codepunkte_nicht_bytes() {
        // "äöü" sind 3 Codepunkte, aber 6 UTF-8-Bytes. maxLength ist 3.
        assert!(schema().gueltig(&json!({ "type": "a", "n": 0, "s": "äöü" })));
        assert!(!schema().gueltig(&json!({ "type": "a", "n": 0, "s": "äöüx" })));
    }

    #[test]
    fn zahlengleichheit_ist_numerisch() {
        assert!(gleich(&json!(3), &json!(3.0)));
        assert!(!gleich(&json!(3), &json!("3")));
    }

    #[test]
    fn verletzungen_sind_kanonisch_sortiert_und_doppelfrei() {
        let v = schema().pruefe(&json!({ "type": "a", "x": 1, "y": 2 }));
        let sortiert = {
            let mut k = v.clone();
            k.sort();
            k
        };
        assert_eq!(v, sortiert);
        assert_eq!(v.len(), 3); // required/n + zwei unbekannte Eigenschaften
    }
}
