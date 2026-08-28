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

/// Gemeinsame Obergrenze des C++-/Rust-/Python-Textriegels. Der Pipe-Framer
/// ist mit 256 KiB enger; direkte DTO-/Dateiaufrufer bleiben dennoch begrenzt.
pub const MAX_DOKUMENT_BYTES: usize = 16 * 1024 * 1024;

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
fn hex_kette(wert: &str, laenge: usize) -> bool {
    wert.len() == laenge && wert.bytes().all(|b| b.is_ascii_digit() || (b'a'..=b'f').contains(&b))
}

/// `alphabet` Zeichen aus dem Base64-Alphabet, dann EIN Zeichen aus `schluss`,
/// dann `fuell` Gleichheitszeichen.
///
/// `schluss` ist nicht Kosmetik: das letzte Alphabetzeichen einer gepolsterten
/// Base64-Kette traegt weniger als sechs echte Bits, der Rest sind FUELLBITS.
/// Ein Restbyte laesst vier frei, zwei Restbytes zwei. Nur die Zeichen, in
/// denen diese Bits null sind, sind kanonisch — sonst erzeugen zwei Sender fuer
/// dieselbe Bitmap zwei verschiedene Zeichenketten.
fn base64_kette(wert: &str, alphabet: usize, schluss: &[u8], fuell: usize) -> bool {
    let b = wert.as_bytes();
    b.len() == alphabet + 1 + fuell
        && b[..alphabet]
            .iter()
            .all(|&c| c.is_ascii_alphanumeric() || c == b'+' || c == b'/')
        && schluss.contains(&b[alphabet])
        && b[alphabet + 1..].iter().all(|&c| c == b'=')
}

/// Geschlossene Mustertabelle — Spiegel von `musterPasst` in NakamaVertrag.cpp.
///
/// 🔑 Jedes dieser Muster steht im Schema NEBEN einem festen `minLength ==
/// maxLength`. Das ist keine Doppelung: Pythons `re` laesst `$` auch VOR einem
/// abschliessenden Zeilenumbruch passen, diese Handschleifen nicht. Weil die
/// Laengenschranke den Umbruch schon faengt, kommen alle drei Beine trotzdem
/// zum selben URTEIL (gemessen in der G1-Nacharbeit).
fn muster_passt(muster: &str, wert: &str) -> Option<bool> {
    match muster {
        "^[0-9a-f]{32}$" => Some(hex_kette(wert, 32)),
        "^[0-9a-f]{64}$" => Some(hex_kette(wert, 64)),
        "^[A-Za-z0-9+/]{37}[AQgw]==$" => Some(base64_kette(wert, 37, b"AQgw", 2)),
        "^[A-Za-z0-9+/]{10}[AEIMQUYcgkosw048]=$" => {
            Some(base64_kette(wert, 10, b"AEIMQUYcgkosw048", 1))
        }
        _ => None,
    }
}

// ------------------------------------------------------------------ Textriegel

/// Groesste ganze Zahl, die binary64 noch exakt traegt: 2^53 - 1.
pub const SICHERE_GANZZAHL: u64 = 9_007_199_254_740_991;

/// Betragsgrenze der Gleitkommazahlen des Vertrags: |x| < 1e308.
pub const DEZ_GRENZE: i64 = 308;

fn ist_hexziffer(c: char) -> bool {
    c.is_ascii_digit() || ('a'..='f').contains(&c) || ('A'..='F').contains(&c)
}

/// Entscheidet AUS DEM LITERAL, ob eine Zahl im Vertragsbereich liegt.
///
/// T2-Runde 2, Blocker BL-1/BL-2: die erste Fassung fragte hier
/// `lit.parse::<f64>()` und auf der C++-Seite `getDoubleValue()`. Genau das
/// war der Fehler — `getDoubleValue()` ist derselbe Leser, gegen dessen
/// Ueberlauf der Riegel schuetzen soll. `juce_CharacterFunctions.h`
/// akkumuliert den Exponenten in einem `int` OHNE Schranke, und der
/// `max_exponent10`-Riegel laeuft DANACH; `1e4294967296` kam dort als **1.0**
/// an, waehrend Rust und Python `inf` lasen.
///
/// Die Lehre, die ueber diesen Fall hinausgeht: **ein Riegel darf nie die
/// Bibliothek befragen, gegen deren Verhalten er schuetzt.** Die erste
/// Ganzzahlregel erfasste nur die Form ohne Punkt/Exponent; heute werden Wert,
/// Ganzzahligkeit und Praezision fuer alle Schreibweisen lexikalisch bestimmt.
/// Die Endlichkeitsregel war delegiert und hat ebenfalls nicht gehalten.
fn zahl_pruefen(ganz: &str, bruch: &str, exp_ziffern: &str, exp_negativ: bool,
                lit: &str, schema_ganzzahl_sichern: bool) -> Result<(), String> {
    if bruch.is_empty() && exp_ziffern.is_empty() {
        let zu_gross = ganz.len() > 16
            || (ganz.len() == 16 && ganz.parse::<u64>().unwrap_or(u64::MAX) > SICHERE_GANZZAHL);
        if zu_gross {
            return Err(format!("Ganzzahl ausserhalb 2^53-1: {lit}"));
        }
        return Ok(());
    }

    // Der Exponent selbst: mehr als drei Ziffern liegen schon ausserhalb, und
    // so wird er auch nie gross genug, um irgendwo ueberzulaufen.
    let ohne_null = exp_ziffern.trim_start_matches('0');
    if ohne_null.len() > 3 {
        return Err(format!("Exponent ausserhalb +/-{DEZ_GRENZE}: {}", kurz(lit)));
    }
    let mut exp: i64 = if ohne_null.is_empty() { 0 } else { ohne_null.parse().unwrap_or(0) };
    if exp_negativ {
        exp = -exp;
    }

    let alle: String = format!("{ganz}{bruch}");
    let signifikant = alle.trim_start_matches('0');
    if signifikant.is_empty() {
        return Ok(()); // der Wert ist exakt 0
    }

    // Die Endlichkeitsgrenze hat Vorrang vor der engeren Ganzzahlregel, damit
    // ein 1e308-Ueberlauf sprachuebergreifend ein Zahlenbereichsfehler bleibt.
    let fuehrende = (alle.len() - signifikant.len()) as i64;
    let dez = (ganz.len() as i64 - fuehrende - 1) + exp;
    if dez >= DEZ_GRENZE || dez <= -DEZ_GRENZE {
        return Err(format!("Zahl ausserhalb +/-1e{DEZ_GRENZE}: {}", kurz(lit)));
    }

    // JSON Schema beurteilt den mathematischen Wert: auch 5.0 und 5e0 sind
    // Integer. Die sichere Ganzzahlgrenze muss deshalb jede exakt
    // ganzzahlige Dezimal-/Exponentialschreibweise vor dem f64-Parser sehen.
    let skala = exp - bruch.len() as i64; // alle * 10^skala
    let (ist_ganzzahl, ganzzahl_zu_gross) = if skala >= 0 {
        let stellen = signifikant.len() as i64 + skala;
        if stellen > 16 {
            (true, true)
        } else if stellen == 16 {
            let mut normalisiert = signifikant.to_owned();
            normalisiert.extend(std::iter::repeat_n('0', skala as usize));
            (true, normalisiert.as_str() > "9007199254740991")
        } else {
            (true, false)
        }
    } else {
        let abzuschneiden = (-skala) as usize;
        if abzuschneiden <= alle.len()
            && alle.as_bytes()[alle.len() - abzuschneiden..]
                .iter()
                .all(|&z| z == b'0')
        {
            let normalisiert = alle[..alle.len() - abzuschneiden].trim_start_matches('0');
            (true, normalisiert.len() > 16
                || (normalisiert.len() == 16 && normalisiert > "9007199254740991"))
        } else {
            (false, false)
        }
    };
    if ganzzahl_zu_gross {
        return Err(format!("Ganzzahl ausserhalb 2^53-1: {lit}"));
    }
    // Mehr als 15 signifikante Dezimalziffern koennen beim f64-Lesen eine
    // nichtganzzahlige Eingabe auf eine Ganzzahl runden. Exakte Integer haben
    // oben bewusst die weitere 2^53-Grenze.
    let signifikante_stellen = signifikant.trim_end_matches('0').len();
    if schema_ganzzahl_sichern && !ist_ganzzahl && signifikante_stellen > 15 {
        return Err(format!(
            "Zahl mit mehr als 15 signifikanten Dezimalziffern: {}",
            kurz(lit)
        ));
    }

    Ok(())
}

fn kurz(s: &str) -> String {
    s.chars().take(40).collect()
}

/// Der Riegel auf BYTE-Ebene — so, wie ein Dokument wirklich ankommt.
///
/// Vier Regeln lassen sich nur hier ausdruecken (T2-Runde 2, BF-6/BF-7 und
/// der Roh-NUL-Gegenpfad):
///
/// * **BOM.** RFC 8259 §8.1: `serde_json` und Pythons `json` lehnen ein BOM
///   ab, JUCEs `loadFileAsString` streift es und parst weiter.
/// * **Kaputtes UTF-8.** Gemessen liefen die drei Beine hier voellig
///   auseinander: das Python-Bein warf eine ungefangene `UnicodeDecodeError`,
///   dieses hier panickte beim Lesen, und JUCE ersetzte das Byte still.
/// * **Rohes NUL.** Terminatorbasierte C++-Leser duerfen keinen gueltigen
///   Praefix annehmen und die restliche Bytefolge ignorieren.
/// * **Groesse.** Direkte DTO-/Datei-Caller sind wie C++ und Python auf
///   inklusive 16 MiB begrenzt; der Pipe-Framer ist mit 256 KiB enger.
pub fn textriegel_bytes(roh: &[u8]) -> Result<(), String> {
    textriegel_bytes_mit_zahlenpolitik(roh, true)
}

/// Byte-/Textregeln fuer den eigenen korrekt gerundeten DTO-Zahlenleser.
/// Anders als die binary64-Schema-Engines braucht er keinen globalen
/// 15-Ziffern-Riegel, weil er Typ und Bereich feldgenau auswertet.
pub(crate) fn textriegel_bytes_fuer_exakten_zahlenleser(roh: &[u8]) -> Result<(), String> {
    textriegel_bytes_mit_zahlenpolitik(roh, false)
}

fn textriegel_bytes_mit_zahlenpolitik(
    roh: &[u8],
    schema_ganzzahl_sichern: bool,
) -> Result<(), String> {
    if roh.len() > MAX_DOKUMENT_BYTES {
        return Err("Dokument zu gross".into());
    }
    if roh.starts_with(&[0xEF, 0xBB, 0xBF]) {
        return Err("BOM am Dokumentanfang".into());
    }
    if roh.contains(&0) {
        return Err("rohes NUL im Dokument".into());
    }
    match std::str::from_utf8(roh) {
        Ok(text) => textriegel_mit_zahlenpolitik(text, schema_ganzzahl_sichern),
        Err(e) => Err(format!("kein gueltiges UTF-8 an Byte {}", e.valid_up_to())),
    }
}

/// Prueft die Zeichen eines v3-Dokuments, BEVOR ein Parser sie sieht.
///
/// Neun Regeln, jede gegen eine GEMESSENE Abweichung zwischen den Beinen.
/// Auslegung und Begruendung: `eq-copilot/schemas/v3/README.md`.
///
/// Gezaehlt wird in CODEPUNKTEN, damit die Positionsangabe in allen drei
/// Beinen dieselbe ist.
pub fn textriegel(text: &str) -> Result<(), String> {
    textriegel_mit_zahlenpolitik(text, true)
}

fn textriegel_mit_zahlenpolitik(
    text: &str,
    schema_ganzzahl_sichern: bool,
) -> Result<(), String> {
    let z: Vec<char> = text.chars().collect();
    let n = z.len();
    let mut i = 0usize;

    while i < n {
        let c = z[i];

        if c == '"' {
            let mut j = i + 1;
            let mut hoch: Option<u32> = None;
            let ende;
            loop {
                if j >= n {
                    return Err("unbeendete Zeichenkette".into());
                }
                let d = z[j];
                if d == '\\' {
                    if j + 5 < n && z[j + 1] == 'u' {
                        // GENAU vier ASCII-Hexziffern (T2-Runde 2, BF-2/BF-3):
                        // `u32::from_str_radix` naehme ein Vorzeichen, Pythons
                        // `int(roh,16)` zusaetzlich Leerzeichen, `0x`-Praefix,
                        // Ziffern-Trenner und arabisch-indische Ziffern.
                        // Drei Hex-Grammatiken waeren drei Urteile.
                        let roh: String = z[j + 2..j + 6].iter().collect();
                        if roh.chars().count() != 4 || !roh.chars().all(ist_hexziffer) {
                            return Err(format!(
                                "kein 4-stelliges Hex-Escape an Position {j}"));
                        }
                        let cp = u32::from_str_radix(&roh, 16).unwrap_or(0);
                        if cp == 0 {
                            return Err(format!("NUL-Escape in Zeichenkette an Position {j}"));
                        }
                        let tief = (0xDC00..=0xDFFF).contains(&cp);
                        if tief && hoch.is_none() {
                            return Err(format!(
                                "einsames tiefes Surrogat U+{cp:04X} an Position {j}"));
                        }
                        if hoch.is_some() && !tief {
                            return Err(format!("hohes Surrogat ohne Paar an Position {j}"));
                        }
                        hoch = if (0xD800..=0xDBFF).contains(&cp) { Some(cp) } else { None };
                        j += 6;
                        continue;
                    }
                    if hoch.is_some() {
                        return Err(format!("hohes Surrogat ohne Paar an Position {j}"));
                    }
                    j += 2;
                    continue;
                }
                if hoch.is_some() {
                    return Err(format!("hohes Surrogat ohne Paar an Position {j}"));
                }
                if d == '"' {
                    ende = j;
                    break;
                }
                if (d as u32) < 0x20 {
                    return Err(format!(
                        "rohes Steuerzeichen U+{:04X} an Position {j}", d as u32));
                }
                j += 1;
            }
            if hoch.is_some() {
                return Err("hohes Surrogat ohne Paar am Zeichenkettenende".into());
            }
            let leer = ende == i + 1;
            let mut k = ende + 1;
            while k < n && matches!(z[k], ' ' | '\t' | '\r' | '\n') {
                k += 1;
            }
            if leer && k < n && z[k] == ':' {
                return Err(format!("leerer Objektschluessel an Position {i}"));
            }
            i = ende + 1;
            continue;
        }

        // JSON kennt ausserhalb von Zeichenketten genau drei alphabetische
        // Literale. Python akzeptiert zusaetzlich NaN/Infinity, JUCE und
        // serde_json nicht. Das optionale Minus gehoert zur Position und zum
        // gemeldeten Literal, damit -Infinity in allen drei Beinen gleich
        // klassifiziert wird.
        let vorzeichen_vor_literal = c == '-' && i + 1 < n && z[i + 1].is_ascii_alphabetic();
        if c.is_ascii_alphabetic() || vorzeichen_vor_literal {
            let mut j = i + if c == '-' { 1 } else { 0 };
            while j < n && z[j].is_ascii_alphabetic() {
                j += 1;
            }
            let literal: String = z[i..j].iter().collect();
            if !matches!(literal.as_str(), "true" | "false" | "null") {
                return Err(format!("unbekanntes Literal {literal} an Position {i}"));
            }
            i = j;
            continue;
        }

        if c == '-' || c.is_ascii_digit() {
            let mut j = i;
            if z[j] == '-' {
                j += 1;
            }
            let anfang = j;
            while j < n && z[j].is_ascii_digit() {
                j += 1;
            }
            let ganz: String = z[anfang..j].iter().collect();
            if ganz.is_empty() {
                return Err(format!("Zahl ohne Ziffern an Position {i}"));
            }
            if ganz.len() > 1 && ganz.starts_with('0') {
                let lit: String = z[i..j].iter().take(20).collect();
                return Err(format!("fuehrende Null in {lit:?} an Position {i}"));
            }

            let mut bruch = String::new();
            if j < n && z[j] == '.' {
                j += 1;
                let a = j;
                while j < n && z[j].is_ascii_digit() {
                    j += 1;
                }
                bruch = z[a..j].iter().collect();
                if bruch.is_empty() {
                    return Err(format!("Dezimalpunkt ohne Nachkommaziffern an Position {i}"));
                }
            }

            let mut exp_ziffern = String::new();
            let mut exp_negativ = false;
            if j < n && (z[j] == 'e' || z[j] == 'E') {
                j += 1;
                if j < n && (z[j] == '+' || z[j] == '-') {
                    exp_negativ = z[j] == '-';
                    j += 1;
                }
                let a = j;
                while j < n && z[j].is_ascii_digit() {
                    j += 1;
                }
                exp_ziffern = z[a..j].iter().collect();
                if exp_ziffern.is_empty() {
                    // BF-1: getDoubleValue("1e") liefert 1.0.
                    return Err(format!("Exponent ohne Ziffern an Position {i}"));
                }
            }

            let lit: String = z[i..j].iter().collect();
            zahl_pruefen(
                &ganz,
                &bruch,
                &exp_ziffern,
                exp_negativ,
                &lit,
                schema_ganzzahl_sichern,
            )?;
            i = j;
            continue;
        }

        i += 1;
    }
    Ok(())
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
                && x.iter().all(|(k, p)| y.get(k).is_some_and(|q| gleich(p, q)))
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
            Value::Array(a) => a.iter().any(|x| x.as_str().is_some_and(|s| typ_passt(s, daten))),
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
                if !deklariert.is_some_and(|d| d.contains_key(name)) {
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
    fn textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python() {
        let roh = vec![b' '; MAX_DOKUMENT_BYTES + 1];
        assert_eq!(textriegel_bytes(&roh), Err("Dokument zu gross".into()));
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
