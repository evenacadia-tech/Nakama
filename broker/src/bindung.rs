//! Persistenz der Profilbindungen sensor_id → profile_id (Plan §11 M2
//! „Profilbindung", §8.3 Speicherregeln: atomisch über temporäre Datei plus
//! Rename, beschädigte Dateien sichtbar melden, nie still überspringen).

use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::path::Path;

#[path = "persistenz.rs"]
pub(crate) mod persistenz;

const SCHEMA: &str = "evenacadia.eq-copilot.bindungen.v1";

#[derive(Debug, Serialize, Deserialize)]
struct BindungsDatei {
    schema: String,
    bindungen: HashMap<String, String>,
}

/// Fehlende Datei ist ein normaler Erststart (leer); eine unlesbare Datei ist
/// ein sichtbarer Fehler — der Aufrufer merkt ihn im Register an.
pub fn bindungen_laden(pfad: &Path) -> Result<HashMap<String, String>, String> {
    let text = match std::fs::read_to_string(pfad) {
        Ok(t) => t,
        Err(e) if e.kind() == std::io::ErrorKind::NotFound => return Ok(HashMap::new()),
        Err(e) => return Err(format!("{} lesen: {e}", pfad.display())),
    };
    let datei: BindungsDatei =
        serde_json::from_str(&text).map_err(|e| format!("{} beschädigt: {e}", pfad.display()))?;
    if datei.schema != SCHEMA {
        return Err(format!(
            "{}: unbekanntes Schema »{}« (erwartet {SCHEMA})",
            pfad.display(),
            datei.schema
        ));
    }
    Ok(datei.bindungen)
}

pub fn bindungen_schreiben(pfad: &Path, bindungen: &HashMap<String, String>) -> Result<(), String> {
    let datei = BindungsDatei { schema: SCHEMA.into(), bindungen: bindungen.clone() };
    let json = serde_json::to_string_pretty(&datei).map_err(|e| format!("serialisieren: {e}"))?;
    persistenz::atomar_ersetzen(pfad, json.as_bytes())
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::{Arc, Barrier};

    fn tmp_ordner(name: &str) -> std::path::PathBuf {
        std::env::temp_dir().join(format!(
            "eqcop-bindung-test-{}-{name}-{}",
            std::process::id(),
            uuid::Uuid::new_v4().simple()
        ))
    }

    #[test]
    fn runde_laden_schreiben_laden() {
        let ordner = tmp_ordner("runde");
        let pfad = ordner.join("a.json");
        assert!(bindungen_laden(&pfad).unwrap().is_empty(), "fehlende Datei = leerer Start");

        let mut b = HashMap::new();
        b.insert("s-1".to_string(), "profil-even34".to_string());
        bindungen_schreiben(&pfad, &b).unwrap();
        let geladen = bindungen_laden(&pfad).unwrap();
        assert_eq!(geladen.get("s-1").map(String::as_str), Some("profil-even34"));
        std::fs::remove_dir_all(ordner).unwrap();
    }

    #[test]
    fn beschaedigte_datei_ist_sichtbarer_fehler() {
        let ordner = tmp_ordner("kaputt");
        std::fs::create_dir_all(&ordner).unwrap();
        let pfad = ordner.join("kaputt.json");
        std::fs::write(&pfad, "{nicht json").unwrap();
        assert!(bindungen_laden(&pfad).is_err());
        std::fs::write(&pfad, r#"{"schema":"falsch.v9","bindungen":{}}"#).unwrap();
        assert!(bindungen_laden(&pfad).unwrap_err().contains("falsch.v9"));
        std::fs::remove_dir_all(ordner).unwrap();
    }

    #[test]
    fn wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand() {
        let ordner = tmp_ordner("wiederholt");
        let pfad = ordner.join("bindungen.json");

        for nummer in 0..4 {
            let bindungen = HashMap::from([("sensor".to_string(), format!("profil-{nummer}"))]);
            bindungen_schreiben(&pfad, &bindungen).unwrap();
        }

        assert_eq!(bindungen_laden(&pfad).unwrap()["sensor"], "profil-3");
        std::fs::remove_dir_all(ordner).unwrap();
    }

    #[test]
    fn parallele_schreiber_hinterlassen_genau_einen_intakten_stand() {
        let ordner = tmp_ordner("parallel");
        let pfad = ordner.join("bindungen.json");
        let anzahl = 12;
        let start = Arc::new(Barrier::new(anzahl));
        let mut threads = Vec::new();

        for nummer in 0..anzahl {
            let pfad = pfad.clone();
            let start = Arc::clone(&start);
            threads.push(std::thread::spawn(move || {
                let bindungen = HashMap::from([
                    ("sensor".to_string(), format!("profil-{nummer}")),
                    (format!("beleg-{nummer}"), "vollstaendig".to_string()),
                ]);
                start.wait();
                bindungen_schreiben(&pfad, &bindungen).map(|_| bindungen)
            }));
        }

        let kandidaten: Vec<HashMap<String, String>> = threads
            .into_iter()
            .map(|t| t.join().unwrap().unwrap())
            .collect();
        let geladen = bindungen_laden(&pfad).unwrap();
        assert!(
            kandidaten.contains(&geladen),
            "kein zerrissener Mischstand: {geladen:?}"
        );
        assert_eq!(
            std::fs::read_dir(&ordner)
                .unwrap()
                .filter_map(Result::ok)
                .filter(|e| e.file_name().to_string_lossy().contains(".tmp-"))
                .count(),
            0,
            "keine temporären Restdateien"
        );
        std::fs::remove_dir_all(ordner).unwrap();
    }

    #[test]
    fn fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben() {
        let ordner = tmp_ordner("rest");
        std::fs::create_dir_all(&ordner).unwrap();
        let pfad = ordner.join("bindungen.json");
        let rest = pfad.with_extension(format!("tmp-{}", std::process::id()));
        std::fs::write(&rest, b"fremder Rest").unwrap();

        let bindungen = HashMap::from([("sensor".to_string(), "neu".to_string())]);
        bindungen_schreiben(&pfad, &bindungen).unwrap();

        assert_eq!(std::fs::read(&rest).unwrap(), b"fremder Rest");
        assert_eq!(bindungen_laden(&pfad).unwrap(), bindungen);
        std::fs::remove_dir_all(ordner).unwrap();
    }
}
