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
const VERTRAG: &str = include_str!("../../eq-copilot/schemas/state/nakama-parameter-v2.json");

/// Hoechstzahl der Schutz-Zonen (SONDE-015 R6). Sie steht auch im Vertrag;
/// `Bestand::bauen` haelt beide gegeneinander, damit eine Aenderung dort
/// hier sofort faellt.
const MAX_ZONEN: usize = 8;

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
    // SONDE-015 R6: drei Gruende, die die LISTE betreffen, nicht einen Wert.
    ZoneAnzahl,
    ZoneDoppelt,
    ZoneSortierung,
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
            Grund::ZoneAnzahl => "zone_anzahl",
            Grund::ZoneDoppelt => "zone_doppelt",
            Grund::ZoneSortierung => "zone_sortierung",
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

        /*  SONDE-015: die Vertragsreihenfolge steht in `ids` und ist NICHT
            mehr `global` + `band_vorlage` x Slots - die drei v2-Globalen
            stehen hinter allen v1-Slotfeldern. Der Bestand wird deshalb ueber
            eine Zwischenkarte gebaut und danach nach `ids` sortiert; die
            Reihenfolge ist Vertrag (siehe Kopf dieses Moduls). */
        let mut nach_id: std::collections::HashMap<String, Art> = std::collections::HashMap::new();
        for g in v["global"].as_array().expect("global fehlt") {
            let id = g["id"].as_str().expect("global ohne id").to_string();
            let art = art_von(g, &id);
            nach_id.insert(id, art);
        }

        let slots = v["slot_anzahl"].as_u64().expect("slot_anzahl fehlt");
        for slot in 0..slots {
            for p in v["band_vorlage"].as_array().expect("band_vorlage fehlt") {
                let name = p["name"].as_str().expect("band_vorlage ohne name");
                let layout = p["layout"].as_str().expect("band_vorlage ohne layout");
                let id = format!("{layout}.band.{slot}.{name}");
                let art = art_von(p, &id);
                nach_id.insert(id, art);
            }
        }

        let mut eintraege = Vec::new();
        for id in v["ids"].as_array().expect("ids fehlt") {
            let id = id.as_str().expect("id ist kein String").to_string();
            let art = nach_id
                .remove(&id)
                .unwrap_or_else(|| panic!("ids nennt {id}, global/band_vorlage nicht"));
            eintraege.push(Eintrag { id, art });
        }
        assert!(
            nach_id.is_empty(),
            "global/band_vorlage tragen Kennungen, die ids nicht nennt: {:?}",
            nach_id.keys().collect::<Vec<_>>()
        );

        // Die Zonengrenze steht im Vertrag; eine zweite Zahl hier waere die
        // Drift, gegen die dieses Modul den Vertrag ueberhaupt liest.
        assert_eq!(
            v["schutz_zonen"]["hoechstens"].as_u64(),
            Some(MAX_ZONEN as u64),
            "nakama-parameter-v2.json nennt eine andere Zonengrenze"
        );

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
    // 1. Byte-/Textriegel des v3-Vertrags in der Variante fuer den danach
    //    folgenden exakt gerundeten DTO-Leser — dieselbe Abbildung wie
    //    NakamaParameter.cpp und erzeuge_state_fixtures.py.
    if let Err(detail) = crate::vertrag::textriegel_bytes_fuer_exakten_zahlenleser(roh) {
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

    // 3. Struktur: genau {dsp_schema_version, parameters, schutz_zonen}.
    let obj = wurzel.as_object().ok_or(Grund::Struktur)?;
    if obj.len() != 3
        || !obj.contains_key("dsp_schema_version")
        || !obj.contains_key("parameters")
        || !obj.contains_key("schutz_zonen")
    {
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
    let zonen = obj["schutz_zonen"].as_array().ok_or(Grund::Struktur)?;

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
                // 🔑 Diese Stufe kann heute nicht mehr feuern, und das ist
                // KEIN Grund, sie wegzulassen. serde_json parst NaN/Infinity
                // gar nicht erst (sie sind kein JSON), der Textriegel hat
                // Literale jenseits 1e308 schon in Stufe 1 gefangen, und
                // `as_f64` liefert fuer JEDE `Number` einen Wert — auch fuer
                // u64/i64, dann verlustbehaftet (gemessen im Test
                // `as_f64_liefert_fuer_jede_number_einen_wert`; eine frueherer
                // Fassung dieses Kommentars behauptete das Gegenteil).
                //
                // Der Zweig bleibt, weil die anderen beiden Beine ihn auch
                // haben: `validiere` in NakamaParameter.cpp und der
                // Python-Referenzvalidator pruefen `isfinite` ausdruecklich.
                // Ein Bein, das eine Stufe der Leiter AUSLAESST, weil sie
                // heute unerreichbar ist, laeuft auseinander, sobald eine
                // Vorstufe sich aendert.
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

    // 10. Schutz-Zonen (SONDE-015 R6) - dieselbe Stufenfolge wie in
    //     `NakamaParameter.cpp` und im Python-Referenzvalidator: erst
    //     Struktur und Typ je Zone, dann Anzahl, Bereich, doppelte id,
    //     Sortierung.
    pruefe_zonen(zonen)
}

/// Die Zonenregeln als eigene Leiter - EINE Stelle, an der sie stehen.
fn pruefe_zonen(zonen: &[Value]) -> Result<(), Grund> {
    let mut ids: Vec<i64> = Vec::with_capacity(zonen.len());
    for z in zonen {
        let o = z.as_object().ok_or(Grund::Struktur)?;
        if o.len() != 4
            || !o.contains_key("enabled")
            || !o.contains_key("high_hz")
            || !o.contains_key("id")
            || !o.contains_key("low_hz")
        {
            return Err(Grund::Struktur);
        }
        if !o["enabled"].is_boolean() {
            return Err(Grund::Typ);
        }
        for name in ["id", "low_hz", "high_hz"] {
            if o[name].is_boolean() || !o[name].is_number() {
                return Err(Grund::Typ);
            }
        }
        // Die `id` ist eine GANZE Zahl. JSON kennt den Unterschied nicht, der
        // Vertrag schon: 3.5 ist keine Zonenidentitaet, sondern ein Fehler.
        let id = o["id"].as_f64().ok_or(Grund::Typ)?;
        if !id.is_finite() || id.fract() != 0.0 {
            return Err(Grund::Bereich);
        }
        ids.push(id as i64);
    }

    if zonen.len() > MAX_ZONEN {
        return Err(Grund::ZoneAnzahl);
    }

    for (i, z) in zonen.iter().enumerate() {
        if !(0..MAX_ZONEN as i64).contains(&ids[i]) {
            return Err(Grund::Bereich);
        }
        let low = z["low_hz"].as_f64().ok_or(Grund::Typ)?;
        let high = z["high_hz"].as_f64().ok_or(Grund::Typ)?;
        if !low.is_finite() || !high.is_finite() {
            return Err(Grund::NichtEndlich);
        }
        // 20 <= low < high <= 20000. Die Nyquistkappung auf 0,45*fs macht die
        // LAUFZEIT, nicht das DTO (R6, Abweichung 2): eine samplerateabhaengige
        // DTO-Grenze machte einen bei 96 kHz gespeicherten Stand bei 44,1 kHz
        // unlesbar.
        if low < 20.0 || low >= high || high > 20_000.0 {
            return Err(Grund::Bereich);
        }
    }

    for i in 0..ids.len() {
        for j in (i + 1)..ids.len() {
            if ids[i] == ids[j] {
                return Err(Grund::ZoneDoppelt);
            }
        }
    }
    for i in 1..ids.len() {
        if ids[i - 1] >= ids[i] {
            return Err(Grund::ZoneSortierung);
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn bestand_kommt_aus_dem_vertrag_und_hat_120_eintraege() {
        assert_eq!(bestand().anzahl(), 120);
        // Die 109 v1-Kennungen stehen VORNE und unveraendert: Layout v2 ist
        // additiv, kein neuer Wert bekommt einen freien Platz im Bestand.
        let b = bestand();
        assert!(b.eintraege[..109].iter().all(|e| e.id.starts_with("v1.")));
        assert!(b.eintraege[109..].iter().all(|e| e.id.starts_with("v2.")));
        assert_eq!(b.eintraege[109].id, "v2.global.eq_enabled");
        assert_eq!(b.eintraege[110].id, "v2.global.mix");
        assert_eq!(b.eintraege[111].id, "v2.global.auto_gain");
        assert_eq!(b.eintraege[112].id, "v2.band.0.occupied");
        assert_eq!(b.eintraege[119].id, "v2.band.7.occupied");
    }

    /// SONDE-015 R6: die drei Zonengruende sind eigene Woerter, keine
    /// Bereichsfehler - sie betreffen die LISTE, nicht einen Wert. Die
    /// Reihenfolge der Leiter ist Vertrag, weil das Manifest je Fixture genau
    /// EINEN Grund haelt.
    #[test]
    fn zonenleiter_meldet_denselben_ersten_grund_wie_die_anderen_beine() {
        let zone = |id: i64, low: f64, high: f64| {
            serde_json::json!({"enabled": true, "high_hz": high, "id": id, "low_hz": low})
        };
        let neun: Vec<Value> = (0..9).map(|i| zone(i, 100.0, 200.0)).collect();
        assert_eq!(pruefe_zonen(&neun), Err(Grund::ZoneAnzahl));
        assert_eq!(
            pruefe_zonen(&[zone(3, 100.0, 200.0), zone(3, 300.0, 400.0)]),
            Err(Grund::ZoneDoppelt)
        );
        assert_eq!(
            pruefe_zonen(&[zone(5, 100.0, 200.0), zone(1, 300.0, 400.0)]),
            Err(Grund::ZoneSortierung)
        );
        assert_eq!(pruefe_zonen(&[zone(0, 200.0, 200.0)]), Err(Grund::Bereich));
        assert_eq!(pruefe_zonen(&[zone(0, 19.9, 200.0)]), Err(Grund::Bereich));
        assert_eq!(pruefe_zonen(&[zone(0, 100.0, 20_000.1)]), Err(Grund::Bereich));
        assert_eq!(pruefe_zonen(&[zone(8, 100.0, 200.0)]), Err(Grund::Bereich));
        assert_eq!(pruefe_zonen(&[]), Ok(()));
        assert_eq!(
            pruefe_zonen(&[zone(0, 40.0, 120.0), zone(7, 900.0, 1100.0)]),
            Ok(())
        );
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
    fn as_f64_liefert_fuer_jede_number_einen_wert() {
        // Selbstaudit 24.08.: der Kommentar an der Nichtendlich-Stufe
        // behauptete, `as_f64` koenne bei einer Ganzzahl jenseits f64 `None`
        // liefern. Gemessen stimmt das NICHT — serde_json wandelt u64/i64
        // immer um, notfalls verlustbehaftet. Der Test haelt die Tatsache
        // fest, statt sie im Fliesstext zu behaupten.
        let gross: Value = serde_json::from_str("18446744073709551615").unwrap();
        assert!(gross.is_number());
        assert!(gross.as_f64().is_some(), "as_f64 auf u64::MAX liefert Some");
        assert!(gross.as_f64().unwrap().is_finite());

        // Und die Gegenrichtung: NaN/Infinity sind kein JSON, der Parser lehnt
        // sie ab, bevor irgendeine Bereichsstufe sie sehen koennte.
        assert!(serde_json::from_str::<Value>("NaN").is_err());
        assert!(serde_json::from_str::<Value>("Infinity").is_err());
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
