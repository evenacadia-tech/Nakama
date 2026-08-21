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
        let roh = std::fs::read_to_string(&pfad).unwrap_or_else(|e| panic!("{name}: {e}"));

        // Der Textriegel laeuft VOR dem Parser, und zwar ueber JEDES Fixture.
        // Die mit `textriegel_lehnt_ab` markierten muessen an ihm fallen; alle
        // uebrigen muessen ihn passieren. Ohne die zweite Haelfte waere der
        // Riegel eine Behauptung, die nur an wenigen Dateien geprueft wird.
        let riegel = eqcop_broker::vertrag::textriegel(&roh);
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
        let daten: Value = match serde_json::from_str(&roh) {
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
