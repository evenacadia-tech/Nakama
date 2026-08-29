//! `contract_cross_language` (Entwurf §66.2) — Rust-Bein von SONDE-005a.
//!
//! Faehrt den gesamten v3-Fixture-Korpus gegen die Engine in
//! `eqcop_broker::vertrag` und vergleicht mit `MANIFEST.json`: Urteil UND
//! vollstaendige Verletzungsmenge.
//!
//! Der Vergleich laeuft ueber das Manifest, nicht gegen eine Ausgabe der
//! C++-Seite. Stimmen beide Engines mit dem Manifest ueberein, stimmen sie
//! transitiv miteinander ueberein — und das Manifest ist von Hand
//! geschrieben, also nicht die Ausgabe einer der beiden.

use eqcop_broker::vertrag::{Schema, Verletzung};
use serde_json::Value;
use std::path::{Path, PathBuf};

fn wurzel() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).parent().unwrap().to_path_buf()
}

fn lies(pfad: &Path) -> Value {
    let roh = std::fs::read_to_string(pfad)
        .unwrap_or_else(|e| panic!("{}: {e}", pfad.display()));
    serde_json::from_str(&roh).unwrap_or_else(|e| panic!("{}: {e}", pfad.display()))
}

fn schema() -> Schema {
    let pfad = wurzel().join("eq-copilot/schemas/v3/eq-ipc-v3.schema.json");
    Schema::laden(lies(&pfad)).expect("Schema haelt die Engine-Teilmenge nicht ein")
}

fn erwartete(eintrag: &Value) -> Vec<Verletzung> {
    eintrag["verletzungen"]
        .as_array()
        .unwrap()
        .iter()
        .map(|v| Verletzung {
            instanz: v["instanz"].as_str().unwrap().to_string(),
            schema: v["schema"].as_str().unwrap().to_string(),
            schluessel: v["schluessel"].as_str().unwrap().to_string(),
        })
        .collect()
}

#[test]
fn korpus_klassifiziert_wie_das_manifest() {
    let w = wurzel();
    let fixtures = w.join("eq-copilot/fixtures/v3");
    let manifest = lies(&fixtures.join("MANIFEST.json"));
    let s = schema();

    let mut geprueft = 0usize;
    let mut abweichungen: Vec<String> = Vec::new();

    for eintrag in manifest["fixtures"].as_array().unwrap() {
        let name = eintrag["datei"].as_str().unwrap();
        let pfad = fixtures.join(name);
        // BYTES, nicht Text: T2-Runde 2 (BF-6) hat gemessen, dass ein Fixture
        // mit kaputtem UTF-8 hier PANICKTE, waehrend das Python-Bein eine
        // ungefangene Ausnahme warf und JUCE das Byte still ersetzte - drei
        // Ausgaenge fuer dieselbe Datei. Der Riegel sieht die Bytes.
        let roh_bytes = std::fs::read(&pfad).unwrap_or_else(|e| panic!("{name}: {e}"));

        // Der Textriegel laeuft VOR dem Parser, und zwar ueber JEDES Fixture.
        // Die mit `textriegel_lehnt_ab` markierten muessen an ihm fallen; alle
        // uebrigen muessen ihn passieren. Ohne die zweite Haelfte waere der
        // Riegel eine Behauptung, die nur an wenigen Dateien geprueft wird.
        let riegel = eqcop_broker::vertrag::textriegel_bytes(&roh_bytes);
        let soll_abgelehnt = eintrag["textriegel_lehnt_ab"].as_bool().unwrap_or(false);
        if soll_abgelehnt {
            if riegel.is_ok() {
                abweichungen.push(format!("{name}: Textriegel laesst es durch, soll ablehnen"));
            }
            geprueft += 1;
            continue;
        }
        if let Err(e) = riegel {
            abweichungen.push(format!("{name}: Textriegel lehnt ab, soll passieren lassen: {e}"));
            geprueft += 1;
            continue;
        }

        // T2-Runde 1: hier stand ein `panic!`. Ein nicht lesbares Fixture brach
        // damit den GANZEN Lauf ab, statt eine benannte Abweichung zu werden —
        // das Gegenstueck zum `wurzel_skalar`-Zweig der C++-Seite fehlte.
        let daten: Value = match serde_json::from_slice(&roh_bytes) {
            Ok(v) => v,
            Err(e) => {
                let skalar = eintrag["wurzel_skalar"].as_bool().unwrap_or(false);
                abweichungen.push(if skalar {
                    format!("{name}: als Skalar-Wurzel markiert, aber serde_json liest es nicht: {e}")
                } else {
                    format!("{name}: nicht lesbar: {e}")
                });
                geprueft += 1;
                continue;
            }
        };
        let ist = s.pruefe(&daten);
        let soll_gueltig = eintrag["urteil"].as_str().unwrap() == "gueltig";

        if ist.is_empty() != soll_gueltig {
            abweichungen.push(format!(
                "{name}: Urteil {} erwartet, Engine sagt {} ({:?})",
                if soll_gueltig { "gueltig" } else { "ungueltig" },
                if ist.is_empty() { "gueltig" } else { "ungueltig" },
                ist
            ));
        } else {
            let soll = erwartete(eintrag);
            if ist != soll {
                abweichungen.push(format!(
                    "{name}: Verletzungsmenge weicht ab\n  soll {soll:#?}\n  ist  {ist:#?}"
                ));
            }
        }
        geprueft += 1;
    }

    assert!(
        abweichungen.is_empty(),
        "{} von {geprueft} Fixtures weichen ab:\n{}",
        abweichungen.len(),
        abweichungen.join("\n")
    );
    assert_eq!(
        geprueft,
        manifest["anzahl_gueltig"].as_u64().unwrap() as usize
            + manifest["anzahl_ungueltig"].as_u64().unwrap() as usize,
        "Manifestzahlen passen nicht zur Fixtureliste"
    );
    assert!(geprueft >= 100, "Korpus zu klein: {geprueft}");
    println!("{geprueft} Fixtures gegen das Manifest geprueft");
}

/// Die GEMEINSAME Falltabelle des Textriegels.
///
/// T2-Runde 2, Befund BF-5: vorher trug jedes der drei Beine eine eigene
/// Kopie — gezaehlt 31, 32 und 33 Faelle —, waehrend das Beweismanifest
/// „dieselbe 31-Faelle-Tabelle" behauptete. Drei handgepflegte Kopien driften;
/// eine gelesene Datei kann es nicht. Der Text steht hex-kodiert, weil die
/// Tabelle NUL-Escapes, rohe Steuerzeichen, kaputtes UTF-8 und ein BOM
/// enthaelt.
#[test]
fn textriegel_deckt_die_gemeinsame_falltabelle() {
    let tabelle = lies(&wurzel().join("eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json"));
    let faelle = tabelle["faelle"].as_array().unwrap();
    let mut rot: Vec<String> = Vec::new();

    for fall in faelle {
        let hex = fall["text_hex"].as_str().unwrap();
        let roh: Vec<u8> = (0..hex.len())
            .step_by(2)
            .map(|i| u8::from_str_radix(&hex[i..i + 2], 16).unwrap())
            .collect();
        let abgelehnt = eqcop_broker::vertrag::textriegel_bytes(&roh).is_err();
        if abgelehnt != fall["wird_abgelehnt"].as_bool().unwrap() {
            rot.push(format!(
                "#{} {}",
                fall["nr"].as_u64().unwrap(),
                fall["zeigetext"].as_str().unwrap_or("")
            ));
        }
    }

    assert!(rot.is_empty(), "{} Faelle weichen ab: {}", rot.len(), rot.join(" | "));
    assert!(faelle.len() >= 50, "Falltabelle zu klein: {}", faelle.len());
    println!("{} Textriegel-Faelle gegen die gemeinsame Tabelle geprueft", faelle.len());
}

/// SONDE-005b: derselbe Vergleich fuer den BINAEREN Teil des Vertrags.
///
/// Die C++-Gegenseite steht in `SchemaTestMain.cpp`. Beide messen gegen
/// `fixtures/v3/flatbuffers/MANIFEST.json` — Urteil UND vollstaendige
/// Verstossmenge —, und das Manifest ist von Hand geschrieben, also nicht die
/// Ausgabe eines der beiden Leser.
#[test]
fn fb_korpus_klassifiziert_wie_das_manifest() {
    use eqcop_broker::telemetrie::{pruefe, Verstoss};

    let w = wurzel();
    let fixtures = w.join("eq-copilot/fixtures/v3/flatbuffers");
    let manifest = lies(&fixtures.join("MANIFEST.json"));

    let mut geprueft = 0usize;
    let mut abweichungen: Vec<String> = Vec::new();

    for eintrag in manifest["fixtures"].as_array().unwrap() {
        let name = eintrag["datei"].as_str().unwrap();
        let roh = std::fs::read(fixtures.join(name)).unwrap_or_else(|e| panic!("{name}: {e}"));
        let ist = pruefe(&roh);
        let soll_gueltig = eintrag["urteil"].as_str().unwrap() == "gueltig";

        if ist.is_empty() != soll_gueltig {
            abweichungen.push(format!(
                "{name}: Urteil {} erwartet, Leser sagt {} ({:?})",
                if soll_gueltig { "gueltig" } else { "ungueltig" },
                if ist.is_empty() { "gueltig" } else { "ungueltig" },
                ist
            ));
            geprueft += 1;
            continue;
        }

        let soll: Vec<Verstoss> = eintrag["verstoesse"]
            .as_array()
            .unwrap()
            .iter()
            .map(|v| Verstoss {
                pfad: v["pfad"].as_str().unwrap().to_string(),
                regel: v["regel"].as_str().unwrap().to_string(),
            })
            .collect();
        if ist != soll {
            abweichungen.push(format!(
                "{name}: Verstossmenge weicht ab\n  soll {soll:#?}\n  ist  {ist:#?}"
            ));
        }
        geprueft += 1;
    }

    assert!(
        abweichungen.is_empty(),
        "{} von {geprueft} Fixtures weichen ab:\n{}",
        abweichungen.len(),
        abweichungen.join("\n")
    );
    assert_eq!(
        geprueft,
        manifest["anzahl_gueltig"].as_u64().unwrap() as usize
            + manifest["anzahl_ungueltig"].as_u64().unwrap() as usize,
        "Manifestzahlen passen nicht zur Fixtureliste"
    );
    // Substanzriegel: mit geleerter Liste ginge der Test sonst gruen durch.
    assert!(geprueft >= 30, "Binaerkorpus zu klein: {geprueft}");
    println!("{geprueft} Binaerfixtures gegen das Manifest geprueft");
}

#[test]
fn bandgitter_ist_lesbar_und_in_sich_stimmig() {
    let w = wurzel();
    let fein = lies(&w.join("eq-copilot/schemas/v3/bandgitter/nakama_1_24_oct_30_18k_v1.json"));
    let grob = lies(&w.join("eq-copilot/schemas/v3/bandgitter/nakama_log64_v1.json"));

    let kanten = hex_liste(&fein["kanten_hz"]["hex64"]);
    let mitten = hex_liste(&fein["mitten_hz"]["hex64"]);
    assert_eq!(fein["band_anzahl"].as_u64().unwrap(), 221);
    assert_eq!(kanten.len(), 222);
    assert_eq!(mitten.len(), 221);

    for i in 0..kanten.len() - 1 {
        assert!(kanten[i] < kanten[i + 1], "Kanten nicht monoton bei {i}");
    }
    for i in 0..mitten.len() {
        assert!(
            kanten[i] < mitten[i] && mitten[i] < kanten[i + 1],
            "Mitte {i} liegt nicht zwischen ihren Kanten"
        );
        assert!(mitten[i].is_finite());
        assert!((30.0..=18000.0).contains(&mitten[i]), "Mitte {i} ausserhalb 30..18k");
    }

    // Das grobe Gitter ist eine exakte Partition — und seine Kanten sind
    // BITGLEICHE Kopien, nicht neu gerechnete Werte.
    let grobkanten = hex_liste(&grob["kanten_hz"]["hex64"]);
    let gruppen = grob["gruppen"].as_array().unwrap();
    assert_eq!(gruppen.len(), 64);
    assert_eq!(grobkanten.len(), 65);

    let mut erwartet_von = 0u64;
    let mut summe = 0u64;
    for (j, g) in gruppen.iter().enumerate() {
        let von = g["fein_von"].as_u64().unwrap();
        let bis = g["fein_bis_exkl"].as_u64().unwrap();
        assert_eq!(von, erwartet_von, "Gruppe {j} beginnt nicht luckenlos");
        assert!(bis > von, "Gruppe {j} ist leer");
        assert_eq!(g["anzahl"].as_u64().unwrap(), bis - von);
        assert_eq!(
            grobkanten[j].to_bits(),
            kanten[von as usize].to_bits(),
            "grobe Kante {j} ist keine bitgleiche Kopie"
        );
        summe += bis - von;
        erwartet_von = bis;
    }
    assert_eq!(summe, 221, "die Gruppen decken nicht genau 221 Baender ab");
    assert_eq!(grobkanten[64].to_bits(), kanten[221].to_bits());
}

/// T2-Runde 3, Befund 8: die Bandwertgrenzen der beiden Leser standen nur im
/// Quelltext, waehrend README und Beweismanifest `bereich_db` als ihre Quelle
/// nannten — ein Feld, das etwas ganz anderes bedeutet (den Traegerumfang
/// +/-32767). Der Vertrag traegt die Grenze jetzt als
/// `plausibler_bereich_db`, und dieser Test macht aus der Quellenangabe eine
/// Pruefung. Die C++-Seite tut dasselbe.
#[test]
fn bandwertgrenzen_stimmen_mit_dem_vertrag() {
    use eqcop_broker::telemetrie::{Q_0P01_MAX, Q_0P01_MIN, Q_0P1_MAX, Q_0P1_MIN};

    let q = lies(&wurzel().join("eq-copilot/schemas/v3/quantisierung-v1.json"));
    let schema = lies(&wurzel().join("eq-copilot/schemas/v3/eq-ipc-v3.schema.json"));
    let g = &q["plausibler_bereich_db"]["traegergrenzen"];
    assert_eq!(g["q_db_0p1_i16"][0].as_i64().unwrap() as i16, Q_0P1_MIN);
    assert_eq!(g["q_db_0p1_i16"][1].as_i64().unwrap() as i16, Q_0P1_MAX);
    assert_eq!(g["q_db_0p01_i16"][0].as_i64().unwrap() as i16, Q_0P01_MIN);
    assert_eq!(g["q_db_0p01_i16"][1].as_i64().unwrap() as i16, Q_0P01_MAX);

    // Und die Traegergrenzen muessen wirklich aus den dB-Werten folgen.
    let db = q["plausibler_bereich_db"]["wert"].as_array().unwrap();
    let (lo, hi) = (db[0].as_f64().unwrap(), db[1].as_f64().unwrap());
    assert_eq!((lo * 10.0) as i16, Q_0P1_MIN);
    assert_eq!((hi * 100.0) as i16, Q_0P01_MAX);

    // 28.08.2026: Die JSON-Zweige werden gegen dieselbe Quelle gemessen wie
    // die Binaerleser. Damit sind die Grenzen im Schema keine unbewachte
    // dritte Kopie.
    fn schema_grenzen(schema: &Value, definition: &str, encoding: &str) -> (i64, i64) {
        let zweige = schema["$defs"][definition]["oneOf"].as_array().unwrap();
        let zweig = zweige
            .iter()
            .find(|z| z["properties"]["encoding"]["const"] == encoding)
            .unwrap_or_else(|| panic!("{definition}: Encoding {encoding} fehlt"));
        let items = &zweig["properties"]["werte"]["items"];
        assert_eq!(items["type"], "integer", "{definition}/{encoding}");
        (
            items["minimum"].as_i64().unwrap(),
            items["maximum"].as_i64().unwrap(),
        )
    }

    for definition in ["bandwerte_fein", "bandwerte_grob"] {
        assert_eq!(
            schema_grenzen(&schema, definition, "q_db_0p1_i16"),
            (
                g["q_db_0p1_i16"][0].as_i64().unwrap(),
                g["q_db_0p1_i16"][1].as_i64().unwrap(),
            )
        );
        assert_eq!(
            schema_grenzen(&schema, definition, "q_db_0p01_i16"),
            (
                g["q_db_0p01_i16"][0].as_i64().unwrap(),
                g["q_db_0p01_i16"][1].as_i64().unwrap(),
            )
        );
    }
}

#[test]
fn quantisierung_stimmt_mit_den_testvektoren() {
    let w = wurzel();
    let q = lies(&w.join("eq-copilot/schemas/v3/quantisierung-v1.json"));

    let mut geprueft = 0usize;
    for (name, k) in q["kodierungen"].as_object().unwrap() {
        let skalierung = k["skalierung"].as_f64().unwrap();
        for v in k["vektoren"].as_array().unwrap() {
            let ein = f64::from_bits(u64::from_str_radix(v["ein_hex64"].as_str().unwrap(), 16).unwrap());
            let soll_gueltig = v["gueltig"].as_bool().unwrap();
            let soll_sat = v["saturiert"].as_bool().unwrap();

            if k["traeger"] == "i16" {
                let (wert, gueltig, sat) = quantisiere_i16(ein, skalierung);
                assert_eq!(gueltig, soll_gueltig, "{name} {}", v["ein_dezimal"]);
                assert_eq!(sat, soll_sat, "{name} {}", v["ein_dezimal"]);
                assert_eq!(wert as i64, v["wert"].as_i64().unwrap(), "{name} {}", v["ein_dezimal"]);
            } else {
                let (bits, gueltig, sat) = quantisiere_f32(ein);
                assert_eq!(gueltig, soll_gueltig, "{name} {}", v["ein_dezimal"]);
                assert_eq!(sat, soll_sat, "{name} {}", v["ein_dezimal"]);
                assert_eq!(
                    format!("{bits:08X}"),
                    v["wert_hex32"].as_str().unwrap(),
                    "{name} {}",
                    v["ein_dezimal"]
                );
            }
            geprueft += 1;
        }
    }
    assert!(geprueft >= 60, "zu wenige Vektoren: {geprueft}");
    println!("{geprueft} Quantisierungsvektoren geprueft");
}

fn hex_liste(v: &Value) -> Vec<f64> {
    v.as_array()
        .unwrap()
        .iter()
        .map(|h| f64::from_bits(u64::from_str_radix(h.as_str().unwrap(), 16).unwrap()))
        .collect()
}

/// Vertrag aus `quantisierung-v1.json`. `f64::round` ist bereits
/// "halbe Werte von null weg" — genau die dort festgelegte Regel.
fn quantisiere_i16(db: f64, skalierung: f64) -> (i16, bool, bool) {
    if !db.is_finite() {
        return (0, false, false);
    }
    let roh = (db * skalierung).round();
    if roh > i16::MAX as f64 {
        return (i16::MAX, true, true);
    }
    if roh < i16::MIN as f64 {
        return (i16::MIN, true, true);
    }
    (roh as i16, true, false)
}

fn quantisiere_f32(db: f64) -> (u32, bool, bool) {
    if !db.is_finite() {
        return (0f32.to_bits(), false, false);
    }
    let eng = db as f32;
    if !eng.is_finite() {
        return (f32::MAX.copysign(db as f32).to_bits(), true, true);
    }
    (eng.to_bits(), true, false)
}

// ── SONDE-006: RFC-8785-Kanon (state_hash) ───────────────────────────────────
//
// Dritte Partei ist eq-copilot/fixtures/state/MANIFEST.json: die Zahlen-
// vektoren tragen den Text, den RFC 8785 Anhang B DRUCKT, die Dokumente eine
// von Hand nach ECMA-262 hergeleitete Erwartung, die rfc8785 (Python)
// bestaetigt hat. Stimmt `serde_json_canonicalizer` damit ueberein, stimmen
// C++ (NakamaKanon) und Rust transitiv ueberein - und der Broker kann ab
// SONDE-016 einen `state_hash` mit denselben Bytes nachrechnen.

#[test]
fn jcs_fixtures_stimmen_mit_manifest() {
    let w = wurzel();
    let fixtures = w.join("eq-copilot/fixtures/state");
    let manifest = lies(&fixtures.join("MANIFEST.json"));
    let zahlen = lies(&fixtures.join("jcs/zahlen.json"));

    // Zahlenvektoren: IEEE-754-Hex -> f64 -> Kanon.
    let mut ok = 0usize;
    let mut abgelehnt = 0usize;
    let mut abweichungen: Vec<String> = Vec::new();
    for e in zahlen["vektoren"].as_array().unwrap() {
        let hex = e["hex64"].as_str().unwrap();
        let x = f64::from_bits(u64::from_str_radix(hex, 16).unwrap());
        if e["abgelehnt"].as_bool().unwrap_or(false) {
            // serde_json kann NaN/Infinity nicht einmal als Value tragen:
            // `Number::from_f64` liefert None. Das IST die Ablehnung.
            if serde_json::Number::from_f64(x).is_none() {
                abgelehnt += 1;
            } else {
                abweichungen.push(format!("{hex}: haette abgelehnt werden muessen"));
            }
            continue;
        }
        let ist = serde_json_canonicalizer::to_string(&Value::from(x)).unwrap();
        let soll = e["erwartet"].as_str().unwrap();
        if ist == soll {
            ok += 1;
        } else {
            abweichungen.push(format!("{hex}: ist {ist} soll {soll}"));
        }
    }
    assert_eq!(
        ok + abgelehnt,
        manifest["jcs_zahlen"]["anzahl"].as_u64().unwrap() as usize,
        "Zahlenvektoren: {abweichungen:?}"
    );
    assert_eq!(abgelehnt, 2, "NaN und Infinity");
    assert!(abweichungen.is_empty(), "{abweichungen:#?}");

    // Dokumente: Eingabetext -> serde_json -> Kanon, bytegleich zum Manifest.
    let mut dok_ok = 0usize;
    for e in manifest["jcs_dokumente"].as_array().unwrap() {
        let pfad = fixtures.join(e["datei"].as_str().unwrap());
        let roh = std::fs::read_to_string(&pfad).unwrap();
        let wert: Value = serde_json::from_str(&roh).unwrap_or_else(|f| panic!("{}: {f}", pfad.display()));
        let ist = serde_json_canonicalizer::to_vec(&wert).unwrap();
        let soll = e["kanon"].as_str().unwrap().as_bytes();
        assert_eq!(
            ist,
            soll,
            "{}: ist {:?} soll {:?}",
            pfad.display(),
            String::from_utf8_lossy(&ist),
            e["kanon"]
        );
        assert_eq!(ist.len() as u64, e["kanon_bytes"].as_u64().unwrap(), "{}", pfad.display());
        dok_ok += 1;
    }
    assert_eq!(dok_ok, manifest["jcs_dokumente"].as_array().unwrap().len());

    // Gueltige DTOs: Kanon -> SHA-256 == state_hash im Manifest. Das ist der
    // Hash, den der Broker ab SONDE-016 fuer apply_transaction nachrechnet.
    use sha2::{Digest, Sha256};
    let mut dto_ok = 0usize;
    for e in manifest["dto_gueltig"].as_array().unwrap() {
        let pfad = fixtures.join(e["datei"].as_str().unwrap());
        let roh = std::fs::read_to_string(&pfad).unwrap();
        let wert: Value = serde_json::from_str(&roh).unwrap();
        let ist = serde_json_canonicalizer::to_vec(&wert).unwrap();
        assert_eq!(ist.len() as u64, e["kanon_bytes"].as_u64().unwrap(), "{}", pfad.display());
        let hash = Sha256::digest(&ist);
        let hex: String = hash.iter().map(|b| format!("{b:02x}")).collect();
        assert_eq!(hex, e["state_hash"].as_str().unwrap(), "state_hash {}", pfad.display());
        dto_ok += 1;
    }
    assert_eq!(dto_ok, manifest["dto_gueltig"].as_array().unwrap().len());
    println!("JCS: {ok} Zahlenvektoren, {abgelehnt} abgelehnt, {dok_ok} Dokumente, {dto_ok} DTOs bytegleich");
}

/// §55 Klausel 4 fuer die State-DTOs — NAK-42.
///
/// Bis zum 24.08.2026 endete die Rust-Seite bei `dto_gueltig`: sechs Dateien,
/// Kanon und Hash nachgerechnet. Die **15 ungueltigen** DTOs las hier niemand
/// (`grep` ueber `broker/`: 0 Treffer), obwohl das Manifest fuer jede einen
/// `grund` fuehrt. Der Exit-Satz aus Entwurf §55 — „C++ und Rust klassifizieren
/// jedes gueltige und ungueltige Fixture identisch" — war fuer diesen Korpus
/// also nicht erfuellt, und der Gate-Lauf G1 hat ihn als Exit-Blocker
/// ausgewiesen.
///
/// 🔑 Gemessen wird gegen das MANIFEST, nicht gegen die C++-Ausgabe. Das ist
/// derselbe Aufbau wie beim v3-Korpus und aus demselben Grund: stimmen beide
/// Beine je mit der dritten Partei ueberein, stimmen sie transitiv miteinander
/// ueberein — und eine GEMEINSAME Auslassung faellt trotzdem auf. Zwei Beine
/// gegeneinander zu halten koennte sie nicht finden (die Lehre aus §4.3
/// desselben Gates).
#[test]
fn dto_korpus_klassifiziert_wie_das_manifest() {
    let fixtures = wurzel().join("eq-copilot/fixtures/state");
    let manifest: Value =
        serde_json::from_str(&std::fs::read_to_string(fixtures.join("MANIFEST.json")).unwrap())
            .unwrap();

    // Der Bestand kommt aus dem Vertrag, nicht aus einer dritten Kopie.
    assert_eq!(
        eqcop_broker::dto::bestand().anzahl(),
        109,
        "Parameterbestand aus nakama-parameter-v1.json"
    );

    let mut ja = 0usize;
    for e in manifest["dto_gueltig"].as_array().unwrap() {
        let pfad = fixtures.join(e["datei"].as_str().unwrap());
        let roh = std::fs::read(&pfad).unwrap();
        match eqcop_broker::dto::pruefe(&roh) {
            Ok(()) => ja += 1,
            Err(g) => panic!("{}: gueltig erwartet, Rust sagt {}", pfad.display(), g.wort()),
        }
    }
    assert_eq!(ja, manifest["dto_gueltig"].as_array().unwrap().len());

    let mut nein = 0usize;
    for e in manifest["dto_ungueltig"].as_array().unwrap() {
        let pfad = fixtures.join(e["datei"].as_str().unwrap());
        let soll = e["grund"].as_str().unwrap();
        let roh = std::fs::read(&pfad).unwrap();
        match eqcop_broker::dto::pruefe(&roh) {
            Ok(()) => panic!("{}: ANGENOMMEN, Manifest sagt {soll}", pfad.display()),
            Err(g) => {
                assert_eq!(g.wort(), soll, "{}: falscher Grund", pfad.display());
                nein += 1;
            }
        }
    }
    assert_eq!(nein, manifest["dto_ungueltig"].as_array().unwrap().len());

    // Der Korpus muss Substanz haben — sonst waere ein Lauf mit geleerter
    // Liste gruen und diese Pruefung eine, die nicht fehlschlagen kann
    // (dieselbe Lehre wie MINDESTKORPUS im v3-Referenzbein).
    assert!(nein >= 15, "nur {nein} ungueltige DTOs - Korpus geschrumpft?");
    println!("DTO: {ja} gueltige, {nein} ungueltige wie im Manifest klassifiziert");
}

/// SONDE-010 — der Envelope-Korpus. Dieselbe Form wie die beiden aelteren
/// Korpora: Urteil UND vollstaendige Verstossmenge gegen das handgeschriebene
/// Manifest. Die C++-Haelfte misst denselben Korpus im Bein B10.
#[test]
fn envelope_korpus_klassifiziert_wie_das_manifest() {
    use eqcop_broker::transport::v3::{envelope_pruefen, Verstoss, ALLE_VERSTOESSE};

    let w = wurzel();
    let fixtures = w.join("eq-copilot/fixtures/v3/envelope");
    let manifest = lies(&fixtures.join("MANIFEST.json"));

    // Die Regelliste des Manifests muss die geschlossene Menge des Lesers
    // sein — nicht eine Teilmenge davon. Sonst koennte eine Regel im Code
    // stehen, die kein Fixture je ausloest.
    let regeln: Vec<&str> = manifest["regeln"]
        .as_array()
        .unwrap()
        .iter()
        .map(|v| v.as_str().unwrap())
        .collect();
    let leser: Vec<&str> = ALLE_VERSTOESSE.iter().map(|v| v.name()).collect();
    assert_eq!(regeln, leser, "Regelmenge von Manifest und Leser weichen ab");

    let mut geprueft = 0usize;
    let mut gedeckt: std::collections::BTreeSet<&str> = std::collections::BTreeSet::new();
    let mut abweichungen: Vec<String> = Vec::new();

    for eintrag in manifest["fixtures"].as_array().unwrap() {
        let name = eintrag["datei"].as_str().unwrap();
        let roh = std::fs::read(fixtures.join(name)).unwrap_or_else(|e| panic!("{name}: {e}"));
        let soll_gueltig = eintrag["urteil"].as_str().unwrap() == "gueltig";
        let soll: Vec<Verstoss> = eintrag["verstoesse"]
            .as_array()
            .unwrap()
            .iter()
            .map(|v| {
                let n = v.as_str().unwrap();
                Verstoss::aus_name(n).unwrap_or_else(|| panic!("{name}: unbekannte Regel {n}"))
            })
            .collect();
        for v in &soll {
            gedeckt.insert(v.name());
        }

        match envelope_pruefen(&roh) {
            Ok(rahmen) => {
                if !soll_gueltig {
                    abweichungen.push(format!("{name}: Leser sagt gueltig, soll {soll:?}"));
                } else if rahmen.drahtlaenge != roh.len() {
                    abweichungen.push(format!(
                        "{name}: Drahtlaenge {} != Dateigroesse {}",
                        rahmen.drahtlaenge,
                        roh.len()
                    ));
                }
            }
            Err(ist) => {
                if soll_gueltig {
                    abweichungen.push(format!("{name}: Leser sagt {ist:?}, soll gueltig"));
                } else if ist != soll {
                    abweichungen.push(format!(
                        "{name}: Verstossmenge weicht ab\n  soll {soll:?}\n  ist  {ist:?}"
                    ));
                }
            }
        }
        geprueft += 1;
    }

    assert!(
        abweichungen.is_empty(),
        "{} von {geprueft} Envelope-Fixtures weichen ab:\n{}",
        abweichungen.len(),
        abweichungen.join("\n")
    );
    assert_eq!(
        geprueft,
        manifest["anzahl_gueltig"].as_u64().unwrap() as usize
            + manifest["anzahl_ungueltig"].as_u64().unwrap() as usize,
        "Manifestzahlen passen nicht zur Fixtureliste"
    );
    // Jede Regel des Lesers hat mindestens ein Negativfixture — sonst stuende
    // sie nur im Code (Lehre aus SONDE-005a).
    let fehlend: Vec<&str> = leser.iter().copied().filter(|r| !gedeckt.contains(r)).collect();
    assert!(fehlend.is_empty(), "Regeln ohne Negativfixture: {fehlend:?}");
    // Substanzriegel: mit geleerter Liste ginge der Test sonst gruen durch.
    assert!(geprueft >= 30, "Envelopekorpus zu klein: {geprueft}");
    println!("{geprueft} Envelope-Fixtures gegen das Manifest geprueft");
}
