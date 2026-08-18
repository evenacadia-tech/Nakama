//! Zeitausgerichteter Aggregat-Snapshot (Plan §11 M2) — Vertrag:
//! eq-copilot/schemas/eq-aggregat.schema.json.
//!
//! Der Broker friert den zuletzt per Heartbeat gemeldeten Messstand der
//! (gefilterten) Sensoren ein; er misst nichts selbst. Zeitausrichtung ist die
//! EHRLICHE Schnittmenge der Projektzeit-Fenster plus Warnungen — keine
//! samplegenaue Kausalitätsbehauptung (Plan §5.7, PDC-Schätzung gibt es in M2
//! nicht). Sensoren aus einem anderen Profil/Prozess geraten nie still in den
//! Snapshot: der Filter steht im Dokument (Plan §8.4).

use super::{paare_auswerten, SensorEintrag};
use serde_json::{json, Value};
use std::path::PathBuf;

pub fn aggregat_bauen(
    alle: &[SensorEintrag],
    filter_profil: Option<&str>,
    filter_pid: Option<u32>,
    broker_version: &str,
    session_token_kurz: &str,
    jetzt_ms: u64,
) -> Value {
    let sensoren: Vec<&SensorEintrag> = alle
        .iter()
        .filter(|s| filter_profil.is_none_or(|p| s.profil_id.as_deref() == Some(p)))
        .filter(|s| filter_pid.is_none_or(|pid| s.host_pid == pid))
        .collect();

    let mut warnungen: Vec<String> = Vec::new();
    if sensoren.is_empty() {
        warnungen.push("kein Messpunkt entspricht dem Filter".to_string());
    }

    // Gemeinsames Fenster = Schnitt über alle Sensoren MIT Messdaten.
    let mut von: Option<i64> = None;
    let mut bis: Option<i64> = None;
    let mut fenster_vollstaendig = !sensoren.is_empty();
    for s in &sensoren {
        if !s.verbunden {
            warnungen.push(format!("»{}« ist getrennt — Stand ist der letzte bekannte", s.label));
        } else if s.stale {
            warnungen.push(format!("»{}« sendet nicht mehr (letztes Lebenszeichen zu alt)", s.label));
        }
        if s.konflikt {
            warnungen.push(format!(
                "»{}« wird von mehreren Instanzen gemeldet (Kennungs-Konflikt) — Werte stammen von der jüngsten",
                s.label
            ));
        }
        let Some(m) = &s.messung else {
            fenster_vollstaendig = false;
            if s.protokoll_version <= 1 {
                warnungen.push(format!("»{}« liefert keine Messdaten (altes Plugin, v1)", s.label));
            } else {
                warnungen.push(format!("»{}« hat noch keinen Messstand gemeldet", s.label));
            }
            continue;
        };
        if m.zustand != "messbereit" {
            warnungen.push(format!("»{}« sammelt noch — Messstand nicht belastbar", s.label));
        }
        let Some(f) = m.projekt_fenster else {
            fenster_vollstaendig = false;
            warnungen.push(format!("»{}« hat keine Projektzeit-Information (Messung ohne Transport?)", s.label));
            continue;
        };
        if f.spruenge > 0 {
            warnungen.push(format!(
                "»{}« hat {} Loop-/Seek-Sprünge im Messfenster — Fenster enthält Lücken oder Wiederholungen",
                s.label, f.spruenge
            ));
        }
        von = Some(von.map_or(f.von_samples, |v| v.max(f.von_samples)));
        bis = Some(bis.map_or(f.bis_samples, |b| b.min(f.bis_samples)));
    }

    let gemeinsames_fenster = match (von, bis, fenster_vollstaendig) {
        (Some(v), Some(b), true) if b > v => json!({ "von_samples": v, "bis_samples": b }),
        (Some(v), Some(b), true) => {
            let _ = (v, b);
            warnungen.push("die Projektzeit-Fenster überlappen nicht — kein gemeinsames Fenster".to_string());
            Value::Null
        }
        _ => {
            if !sensoren.is_empty() && fenster_vollstaendig {
                // kein Sensor lieferte ein Fenster
                warnungen.push("keine Projektzeit-Fenster vorhanden".to_string());
            } else if !sensoren.is_empty() {
                warnungen.push(
                    "gemeinsames Fenster nicht bestimmbar — nicht alle Messpunkte liefern Projektzeit".to_string(),
                );
            }
            Value::Null
        }
    };

    // Paar-Auswertung auf den GEFILTERTEN Sensoren — dieselbe Logik wie der
    // Live-Status, damit Snapshot und Übersicht nie auseinanderlaufen.
    let gefiltert_owned: Vec<SensorEintrag> = sensoren.iter().map(|s| (*s).clone()).collect();
    let paare = paare_auswerten(&gefiltert_owned);

    let sensoren_json: Vec<Value> = sensoren
        .iter()
        .map(|s| {
            json!({
                "sensor_id": s.sensor_id,
                "role": s.role,
                "label": s.label,
                "pair_id": s.pair_id,
                "host_pid": s.host_pid,
                "profil_id": s.profil_id,
                "verbunden": s.verbunden,
                "stale": s.stale,
                "konflikt": s.konflikt,
                "protokoll_version": s.protokoll_version,
                "plugin_version": s.plugin_version,
                "samplerate": s.samplerate,
                "measurement": s.messung.as_ref().map(|m| serde_json::to_value(m).unwrap_or(Value::Null)),
            })
        })
        .collect();

    json!({
        "schema": "evenacadia.eq-copilot.aggregat.v1",
        "erzeugt_ms": jetzt_ms,
        "broker_version": broker_version,
        "session_token_kurz": session_token_kurz,
        "filter": { "profil_id": filter_profil, "host_pid": filter_pid },
        "gemeinsames_fenster": gemeinsames_fenster,
        "warnungen": warnungen,
        "sensoren": sensoren_json,
        "paare": serde_json::to_value(&paare).unwrap_or(Value::Null),
    })
}

/// Zielordner wie die Plugin-Snapshots: %LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\.
pub fn snapshot_ordner() -> Result<PathBuf, String> {
    let basis = std::env::var_os("LOCALAPPDATA")
        .map(PathBuf::from)
        .ok_or_else(|| "LOCALAPPDATA ist nicht gesetzt".to_string())?;
    Ok(basis.join("evenacadia").join("EQ-Copilot").join("snapshots"))
}

/// Atomisch (tmp + rename), Dateiname sortierbar über den Zeitstempel.
pub fn aggregat_schreiben(dokument: &Value) -> Result<PathBuf, String> {
    let ordner = snapshot_ordner()?;
    std::fs::create_dir_all(&ordner).map_err(|e| format!("{} anlegen: {e}", ordner.display()))?;
    let ms = dokument.get("erzeugt_ms").and_then(Value::as_u64).unwrap_or(0);
    let pfad = ordner.join(format!("aggregat-{ms}.json"));
    let tmp = ordner.join(format!("aggregat-{ms}.tmp-{}", std::process::id()));
    let json = serde_json::to_string_pretty(dokument).map_err(|e| format!("serialisieren: {e}"))?;
    std::fs::write(&tmp, json).map_err(|e| format!("{} schreiben: {e}", tmp.display()))?;
    std::fs::rename(&tmp, &pfad).map_err(|e| {
        let _ = std::fs::remove_file(&tmp);
        format!("{} ersetzen: {e}", pfad.display())
    })?;
    Ok(pfad)
}

#[cfg(test)]
mod tests {
    use super::super::protokoll::{AudioAngabe, Hello, MessStand, ProjektFenster, SensorAngabe};
    use super::super::Register;
    use super::*;

    fn hello(id: &str, role: &str, label: &str, pair: Option<&str>, pid: u32) -> Hello {
        Hello {
            protocol_version: 2,
            plugin_version: "0.2.0".into(),
            host_pid: pid,
            sensor: SensorAngabe {
                sensor_id: id.into(),
                instance_nonce: None,
                role: role.into(),
                label: label.into(),
                pair_id: pair.map(|s| s.to_string()),
            },
            audio: AudioAngabe { samplerate: 48000.0, block_size: 512, channels: 2 },
        }
    }

    fn messung(von: i64, bis: i64, spruenge: u32) -> MessStand {
        MessStand {
            zustand: "messbereit".into(),
            metrics_version: "m1-2026-08-13".into(),
            aktiv_s: 10.0,
            gesamt_s: 12.0,
            lufs_i: Some(-18.0),
            projekt_fenster: Some(ProjektFenster { von_samples: von, bis_samples: bis, spruenge }),
            ..Default::default()
        }
    }

    #[test]
    fn schnittfenster_und_paare_im_dokument() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-pre", "pre", "PIANO VORHER", Some("p1"), 1), "n-1");
        r.sensor_verbinden(&hello("s-post", "post", "PIANO NACHHER", Some("p1"), 1), "n-2");
        r.heartbeat("s-pre", "n-1", None, Some(messung(0, 480_000, 0)));
        r.heartbeat("s-post", "n-2", None, Some(messung(48_000, 500_000, 0)));
        let snap = r.sensoren_snapshot(super::super::jetzt_ms());
        let doc = aggregat_bauen(&snap, None, None, "test", "abcd1234", 42);

        assert_eq!(doc["schema"], "evenacadia.eq-copilot.aggregat.v1");
        assert_eq!(doc["gemeinsames_fenster"]["von_samples"], 48_000);
        assert_eq!(doc["gemeinsames_fenster"]["bis_samples"], 480_000);
        assert_eq!(doc["sensoren"].as_array().unwrap().len(), 2);
        assert_eq!(doc["paare"][0]["status"], "vollstaendig");
        // 90 % Überlappung des kürzeren Fensters → ausgerichtet.
        assert_eq!(doc["paare"][0]["timing"], "ausgerichtet", "grund: {}", doc["paare"][0]["grund"]);
    }

    #[test]
    fn filter_trennt_prozesse_und_v1_bekommt_warnung() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-a", "sensor", "A", None, 111), "n-a");
        let mut alt = hello("s-b", "sensor", "B (alt)", None, 222);
        alt.protocol_version = 1;
        r.sensor_verbinden(&alt, "n-b");
        r.heartbeat("s-a", "n-a", None, Some(messung(0, 100, 0)));
        let snap = r.sensoren_snapshot(super::super::jetzt_ms());

        let nur_111 = aggregat_bauen(&snap, None, Some(111), "test", "t", 1);
        assert_eq!(nur_111["sensoren"].as_array().unwrap().len(), 1);
        assert_eq!(nur_111["sensoren"][0]["sensor_id"], "s-a");

        let alles = aggregat_bauen(&snap, None, None, "test", "t", 1);
        let warnungen = alles["warnungen"].as_array().unwrap();
        assert!(
            warnungen.iter().any(|w| w.as_str().unwrap().contains("v1")),
            "v1-Sensor braucht sichtbare Warnung: {warnungen:?}"
        );
        assert!(alles["gemeinsames_fenster"].is_null());
    }

    #[test]
    fn profilfilter_laesst_fremde_sensoren_nie_still_hinein() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-a", "sensor", "A", None, 1), "n-a");
        r.sensor_verbinden(&hello("s-b", "sensor", "B", None, 1), "n-b");
        r.profil_binden("s-a", Some("profil-even34".into()));
        let snap = r.sensoren_snapshot(super::super::jetzt_ms());
        let doc = aggregat_bauen(&snap, Some("profil-even34"), None, "test", "t", 1);
        let ids: Vec<&str> = doc["sensoren"]
            .as_array()
            .unwrap()
            .iter()
            .map(|s| s["sensor_id"].as_str().unwrap())
            .collect();
        assert_eq!(ids, vec!["s-a"]);
        assert_eq!(doc["filter"]["profil_id"], "profil-even34");
    }

    #[test]
    fn schreiben_erzeugt_datei_im_snapshot_ordner() {
        let doc = json!({
            "schema": "evenacadia.eq-copilot.aggregat.v1",
            "erzeugt_ms": 987_654_321u64,
            "broker_version": "test",
            "warnungen": [],
            "sensoren": [],
            "paare": [],
        });
        let pfad = aggregat_schreiben(&doc).unwrap();
        assert!(pfad.exists());
        let text = std::fs::read_to_string(&pfad).unwrap();
        assert!(text.contains("aggregat.v1"));
        let _ = std::fs::remove_file(&pfad);
    }
}
