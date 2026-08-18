//! Persistenz der Profilbindungen sensor_id → profile_id (Plan §11 M2
//! „Profilbindung", §8.3 Speicherregeln: atomisch über temporäre Datei plus
//! Rename, beschädigte Dateien sichtbar melden, nie still überspringen).

use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::path::Path;

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
    if let Some(eltern) = pfad.parent() {
        std::fs::create_dir_all(eltern).map_err(|e| format!("{} anlegen: {e}", eltern.display()))?;
    }
    let datei = BindungsDatei { schema: SCHEMA.into(), bindungen: bindungen.clone() };
    let json = serde_json::to_string_pretty(&datei).map_err(|e| format!("serialisieren: {e}"))?;
    let tmp = pfad.with_extension(format!("tmp-{}", std::process::id()));
    std::fs::write(&tmp, json).map_err(|e| format!("{} schreiben: {e}", tmp.display()))?;
    std::fs::rename(&tmp, pfad).map_err(|e| {
        let _ = std::fs::remove_file(&tmp);
        format!("{} ersetzen: {e}", pfad.display())
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    fn tmp_pfad(name: &str) -> std::path::PathBuf {
        std::env::temp_dir().join(format!("eqcop-bindung-test-{}-{name}", std::process::id()))
    }

    #[test]
    fn runde_laden_schreiben_laden() {
        let pfad = tmp_pfad("a.json");
        let _ = std::fs::remove_file(&pfad);
        assert!(bindungen_laden(&pfad).unwrap().is_empty(), "fehlende Datei = leerer Start");

        let mut b = HashMap::new();
        b.insert("s-1".to_string(), "profil-even34".to_string());
        bindungen_schreiben(&pfad, &b).unwrap();
        let geladen = bindungen_laden(&pfad).unwrap();
        assert_eq!(geladen.get("s-1").map(String::as_str), Some("profil-even34"));
        let _ = std::fs::remove_file(&pfad);
    }

    #[test]
    fn beschaedigte_datei_ist_sichtbarer_fehler() {
        let pfad = tmp_pfad("kaputt.json");
        std::fs::write(&pfad, "{nicht json").unwrap();
        assert!(bindungen_laden(&pfad).is_err());
        std::fs::write(&pfad, r#"{"schema":"falsch.v9","bindungen":{}}"#).unwrap();
        assert!(bindungen_laden(&pfad).unwrap_err().contains("falsch.v9"));
        let _ = std::fs::remove_file(&pfad);
    }
}
