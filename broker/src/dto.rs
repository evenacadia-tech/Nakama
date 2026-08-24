//! Rust-Bein des Parameter-DTOs (SONDE-006) — drittes Bein von NAK-42.
//!
//! Gegenstueck zu `eq-copilot/plugin/state/NakamaParameter.cpp`
//! (`param::ausDtoText`) und zum Referenz-Validator
//! `validiere_dto_python` in `tools/eq-copilot/erzeuge_state_fixtures.py`.
//! Alle drei lesen denselben Korpus (`eq-copilot/fixtures/state/dto/`) und
//! muessen ihn IDENTISCH klassifizieren — gemessen gegen dasselbe
//! handgeschriebene `eq-copilot/fixtures/state/MANIFEST.json`.
//!
//! ## Warum es dieses Bein gibt
//!
//! Entwurf §55 verlangt fuer den Ausstieg aus P1 woertlich: „C++ und Rust
//! klassifizieren jedes gueltige und ungueltige Fixture identisch." Fuer den
//! v3-Vertragskorpus war das erfuellt; fuer die State-DTOs war es das **nicht**.
//! `contract_cross_language.rs` iterierte nur ueber `dto_gueltig` (6 Dateien)
//! und rechnete dort Kanon und Hash nach — die **15 ungueltigen** DTOs las auf
//! der Rust-Seite niemand (`grep` ueber `broker/`: 0 Treffer). Sie waren damit
//! von genau zwei Beinen gedeckt, nicht von dreien.
//!
//! Der Gate-Lauf G1 hat das als §55-Exit-Blocker ausgewiesen (NAK-42, „vor G1"
//! datiert). Dieses Modul schliesst es.
//!
//! ## Warum die REIHENFOLGE mitzaehlt
//!
//! Ein Dokument kann mehrere Fehler gleichzeitig tragen. Das Manifest haelt je
//! Fixture genau EINEN `grund` fest — also muessen alle drei Beine bei einem
//! mehrfach kaputten Dokument denselben ERSTEN Grund melden. Die Leiter unten
//! ist deshalb kein Stil, sondern Vertrag:
//!
//! ```text
//! Textriegel -> Parser/doppelter Schluessel -> Struktur -> dsp_schema_version
//!   -> unbekannter Schluessel -> fehlender Schluessel -> Typ (ALLE)
//!   -> nichtendlich -> Bereich/Enum
//! ```
//!
//! `NakamaParameter.h` fuehrt dieselbe Kette; `nakama-state-v2.md` §4 ist ihr
//! Wortlaut.
//!
//! ## Woher der Parameterbestand kommt
//!
//! Aus dem Vertrag selbst, per `include_str!` — nicht aus einer abgeschriebenen
//! Tabelle. Eine dritte handgepflegte Kopie der 109 Parameter waere genau die
//! Drift, gegen die SONDE-005a die Textriegel-Falltabelle in EINE Datei gelegt
//! hat („Drei handgepflegte Kopien driften; eine gelesene Datei kann es
//! nicht"). Der Bestand wird beim ersten Zugriff einmal gebaut.

use std::sync::OnceLock;

use serde::de::{self, Deserialize, Deserializer, MapAccess, SeqAccess, Visitor};
use serde_json::{Map, Value};

/// Der Vertrag selbst — mitkompiliert, damit der Broker keine Datei zur
/// Laufzeit braucht und keine zweite Wahrheit entsteht.
const VERTRAG: &str = include_str!("../../eq-copilot/schemas/state/nakama-parameter-v1.json");

/// Die Gruende, mit denen ein DTO abgelehnt wird.
///
/// Die Woerter sind Vertrag: sie stehen so im MANIFEST und werden von C++ und
/// Python identisch gebildet. Ein neuer Grund ist eine Vertragsaenderung.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Grund {
    KeinJson,
    DoppelterSchluessel,
    Struktur,
    DspSchemaVersion,
    UnbekannterSchluessel,
    FehlenderSchluessel,
    Typ,
    NichtEndlich,
    Bereich,
    Enum,
}

impl Grund {
    pub fn wort(self) -> &'static str {
        match self {
            Grund::KeinJson => "kein_json",
            Grund::DoppelterSchluessel => "doppelter_schluessel",
            Grund::Struktur => "struktur",
            Grund::DspSchemaVersion => "dsp_schema_version",
            Grund::UnbekannterSchluessel => "unbekannter_schluessel",
            Grund::FehlenderSchluessel => "fehlender_schluessel",
            Grund::Typ => "typ",
            Grund::NichtEndlich => "nichtendlich",
            Grund::Bereich => "bereich",
            Grund::Enum => "enum",
        }
    }
}

/// Was ein Parameter sein darf.
#[derive(Debug, Clone)]
enum Art {
    Bool,
    Float { min: f64, max: f64 },
    Enum { werte: Vec<String> },
}

#[derive(Debug, Clone)]
struct Eintrag {
    id: String,
    art: Art,
}

/// Die 109 Parameter in VERTRAGSREIHENFOLGE.
///
/// Die Reihenfolge ist nicht Kosmetik: „fehlender Schluessel" und „Typ" laufen
/// in beiden anderen Beinen ueber die Tabellenreihenfolge. Solange nur EIN
/// Grund gemeldet wird, entscheidet sie zwar nicht ueber das Wort — aber sie
/// haelt die drei Beine auch dann deckungsgleich, wenn spaeter ein Detail
/// (welcher Schluessel fehlt) dazukommt.
#[derive(Debug)]
pub struct Bestand {
    eintraege: Vec<Eintrag>,
    dsp_schema_version: f64,
}

fn zahl(v: &Value, feld: &str, wo: &str) -> f64 {
    v.get(feld)
        .and_then(Value::as_f64)
        .unwrap_or_else(|| panic!("nakama-parameter-v1.json: {wo} ohne {feld}"))
}

impl Bestand {
    fn bauen() -> Bestand {
        let v: Value = serde_json::from_str(VERTRAG).expect("nakama-parameter-v1.json ist kein JSON");

        let art_von = |p: &Value, wo: &str| -> Art {
            match p.get("typ").and_then(Value::as_str) {
                Some("bool") => Art::Bool,
                Some("float") => Art::Float { min: zahl(p, "min", wo), max: zahl(p, "max", wo) },
                Some("enum") => Art::Enum {
                    werte: p["werte"]
                        .as_array()
                        .unwrap_or_else(|| panic!("{wo}: enum ohne werte"))
                        .iter()
                        .map(|w| w.as_str().expect("enum-Wert ist kein String").to_string())
                        .collect(),
                },
                other => panic!("{wo}: unbekannter Typ {other:?}"),
            }
        };

        let mut eintraege = Vec::new();
        for g in v["global"].as_array().expect("global fehlt") {
            let id = g["id"].as_str().expect("global ohne id").to_string();
            let art = art_von(g, &id);
            eintraege.push(Eintrag { id, art });
        }

        let slots = v["slot_anzahl"].as_u64().expect("slot_anzahl fehlt");
        for slot in 0..slots {
            for p in v["band_vorlage"].as_array().expect("band_vorlage fehlt") {
                let name = p["name"].as_str().expect("band_vorlage ohne name");
                let id = format!("v1.band.{slot}.{name}");
                let art = art_von(p, &id);
                eintraege.push(Eintrag { id, art });
            }
        }

        // Der Vertrag nennt seine eigene Zahl. Wenn die Konstruktion sie nicht
        // trifft, ist eine der beiden Seiten falsch — und zwar SOFORT, nicht
        // erst, wenn ein Fixture zufaellig darauf zeigt.
        let soll = v["anzahl_parameter"].as_u64().expect("anzahl_parameter fehlt") as usize;
        assert_eq!(
            eintraege.len(),
            soll,
            "Parameterbestand: {} gebaut, Vertrag sagt {soll}",
            eintraege.len()
        );

        Bestand {
            eintraege,
            dsp_schema_version: v["dsp_schema_version"].as_f64().expect("dsp_schema_version fehlt"),
        }
    }

    fn finde(&self, id: &str) -> Option<&Eintrag> {
        self.eintraege.iter().find(|e| e.id == id)
    }

    pub fn anzahl(&self) -> usize {
        self.eintraege.len()
    }
}

pub fn bestand() -> &'static Bestand {
    static B: OnceLock<Bestand> = OnceLock::new();
    B.get_or_init(Bestand::bauen)
}

// ------------------------------------------------------- Doppelte Schluessel

/// Marke, an der die Doppelschluessel-Ablehnung aus dem Serde-Fehler wieder
/// herausgelesen wird.
///
/// 🔑 `serde_json::Value` kann das nicht: seine `Map` ist eine Map, sie
/// UEBERSCHREIBT den ersten Wert still — genau wie Pythons `json.loads` ohne
/// `object_pairs_hook`. Der C++-Leser (`kanon::lies`) meldet den doppelten
/// Schluessel als eigenen Fehler. Ohne den Umweg hier waere das Rust-Bein an
/// dieser Stelle SCHWAECHER als die anderen beiden und `doppelter-schluessel.json`
/// wuerde als gueltig durchgehen.
const MARKE_DOPPELT: &str = "nakama:doppelter-schluessel";

/// `serde_json::Value`, aber mit Ablehnung doppelter Objektschluessel.
#[derive(Debug)]
struct StrengerWert(Value);

struct WertBesucher;

impl<'de> Visitor<'de> for WertBesucher {
    type Value = StrengerWert;

    fn expecting(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        f.write_str("ein JSON-Wert ohne doppelte Objektschluessel")
    }

    fn visit_bool<E: de::Error>(self, v: bool) -> Result<Self::Value, E> {
        Ok(StrengerWert(Value::Bool(v)))
    }
    fn visit_i64<E: de::Error>(self, v: i64) -> Result<Self::Value, E> {
        Ok(StrengerWert(Value::from(v)))
    }
    fn visit_u64<E: de::Error>(self, v: u64) -> Result<Self::Value, E> {
        Ok(StrengerWert(Value::from(v)))
    }
    fn visit_f64<E: de::Error>(self, v: f64) -> Result<Self::Value, E> {
        Ok(StrengerWert(Value::from(v)))
    }
    fn visit_str<E: de::Error>(self, v: &str) -> Result<Self::Value, E> {
        Ok(StrengerWert(Value::String(v.to_string())))
    }
    fn visit_none<E: de::Error>(self) -> Result<Self::Value, E> {
        Ok(StrengerWert(Value::Null))
    }
    fn visit_unit<E: de::Error>(self) -> Result<Self::Value, E> {
        Ok(StrengerWert(Value::Null))
    }

    fn visit_seq<A: SeqAccess<'de>>(self, mut a: A) -> Result<Self::Value, A::Error> {
        let mut aus = Vec::new();
        while let Some(StrengerWert(w)) = a.next_element()? {
            aus.push(w);
        }
        Ok(StrengerWert(Value::Array(aus)))
    }

    fn visit_map<A: MapAccess<'de>>(self, mut m: A) -> Result<Self::Value, A::Error> {
        let mut obj = Map::new();
        while let Some(k) = m.next_key::<String>()? {
            let StrengerWert(w) = m.next_value()?;
            if obj.contains_key(&k) {
                return Err(de::Error::custom(MARKE_DOPPELT));
            }
            obj.insert(k, w);
        }
        Ok(StrengerWert(Value::Object(obj)))
    }
}

impl<'de> Deserialize<'de> for StrengerWert {
    fn deserialize<D: Deserializer<'de>>(d: D) -> Result<Self, D::Error> {
        d.deserialize_any(WertBesucher)
    }
}

// ------------------------------------------------------------------- Leiter

/// Validiert das DTO `{"dsp_schema_version":1,"parameters":{...}}`.
///
/// `Ok(())` heisst gueltig; `Err(Grund)` traegt den ERSTEN Grund der Leiter.
pub fn pruefe(roh: &[u8]) -> Result<(), Grund> {
    // 1. Byte-/Textriegel des v3-Vertrags — dieselbe Abbildung wie
    //    NakamaParameter.cpp:190-205 und erzeuge_state_fixtures.py:241-248.
    if let Err(detail) = crate::vertrag::textriegel_bytes(roh) {
        if detail.starts_with("Exponent ausserhalb") || detail.starts_with("Zahl ausserhalb") {
            return Err(Grund::NichtEndlich);
        }
        if detail.starts_with("Ganzzahl ausserhalb") {
            return Err(Grund::Bereich);
        }
        return Err(Grund::KeinJson);
    }

    // 2. Parser mit Ablehnung doppelter Schluessel.
    let wurzel: Value = match serde_json::from_slice::<StrengerWert>(roh) {
        Ok(StrengerWert(w)) => w,
        Err(e) => {
            return Err(if e.to_string().contains(MARKE_DOPPELT) {
                Grund::DoppelterSchluessel
            } else {
                Grund::KeinJson
            });
        }
    };

    // 3. Struktur: genau {dsp_schema_version, parameters}.
    let obj = wurzel.as_object().ok_or(Grund::Struktur)?;
    if obj.len() != 2 || !obj.contains_key("dsp_schema_version") || !obj.contains_key("parameters") {
        return Err(Grund::Struktur);
    }

    // 4. dsp_schema_version. `true` ist in serde_json ein Bool und kein
    //    Number — `as_f64` liefert None, also faellt es hier und nicht als Typ.
    //    Genau so verhaelt sich der Python-Referenzvalidator (er schliesst bool
    //    ausdruecklich aus) und der C++-Leser (Art::zahl).
    let b = bestand();
    if obj["dsp_schema_version"].as_f64() != Some(b.dsp_schema_version) {
        return Err(Grund::DspSchemaVersion);
    }

    let par = obj["parameters"].as_object().ok_or(Grund::Struktur)?;

    // 5. Unbekannte Schluessel.
    for k in par.keys() {
        if b.finde(k).is_none() {
            return Err(Grund::UnbekannterSchluessel);
        }
    }

    // 6. Fehlende Schluessel.
    for e in &b.eintraege {
        if !par.contains_key(&e.id) {
            return Err(Grund::FehlenderSchluessel);
        }
    }

    // 7. Typ je Parameter — ALLE, bevor ein WERT geprueft wird.
    //    Die Trennung ist Vertrag: ein Dokument mit falschem Typ an einer und
    //    verletztem Bereich an einer anderen Stelle meldet in allen drei Beinen
    //    `typ`, nie `bereich`.
    for e in &b.eintraege {
        let w = &par[&e.id];
        let ok = match e.art {
            Art::Bool => w.is_boolean(),
            // `is_boolean` zuerst ausschliessen: serde_json macht aus `true`
            // keine Zahl, aber die Absicht steht hier ausdruecklich, weil der
            // Python-Zwilling sie ausdruecken MUSS (bool ist dort ein int).
            Art::Float { .. } => !w.is_boolean() && w.is_number(),
            Art::Enum { .. } => w.is_string(),
        };
        if !ok {
            return Err(Grund::Typ);
        }
    }

    // 8./9. Nichtendlich, dann Bereich/Enum.
    for e in &b.eintraege {
        let w = &par[&e.id];
        match &e.art {
            Art::Bool => {}
            Art::Float { min, max } => {
                // 🔑 serde_json kann NaN/Infinity gar nicht erst parsen (sie
                // sind kein JSON), und der Textriegel hat Literale jenseits
                // 1e308 schon in Stufe 1 gefangen. `as_f64` kann hier trotzdem
                // None liefern — bei einer Ganzzahl jenseits f64. Ein Wert, den
                // dieses Bein nicht als endliche Zahl lesen kann, ist
                // `nicht endlich`, nicht stillschweigend in Ordnung.
                let x = match w.as_f64() {
                    Some(x) if x.is_finite() => x,
                    _ => return Err(Grund::NichtEndlich),
                };
                if x < *min || x > *max {
                    return Err(Grund::Bereich);
                }
            }
            Art::Enum { werte } => {
                let s = w.as_str().unwrap_or_default();
                if !werte.iter().any(|v| v == s) {
                    return Err(Grund::Enum);
                }
            }
        }
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege() {
        assert_eq!(bestand().anzahl(), 109);
    }

    #[test]
    fn doppelter_schluessel_wird_nicht_still_ueberschrieben() {
        // Die Gegenprobe zum Umweg ueber StrengerWert: mit `serde_json::Value`
        // gaebe es hier KEINEN Fehler, der zweite Wert gewaenne still.
        let roh = br#"{"a":1,"a":2}"#;
        assert!(serde_json::from_slice::<Value>(roh).is_ok(), "Value ueberschreibt still");
        let hart = serde_json::from_slice::<StrengerWert>(roh);
        assert!(hart.is_err(), "StrengerWert muss ablehnen");
        assert!(hart.unwrap_err().to_string().contains(MARKE_DOPPELT));
    }

    #[test]
    fn geschachtelter_doppelter_schluessel_faellt_ebenfalls() {
        // Der Besucher muss REKURSIV greifen — sonst deckt er nur die Wurzel,
        // und `parameters` ist genau die Ebene, auf der die Parameter liegen.
        let roh = br#"{"x":{"b":1,"b":2}}"#;
        let hart = serde_json::from_slice::<StrengerWert>(roh);
        assert!(hart.is_err(), "auch geschachtelt muss es fallen");
    }
}
