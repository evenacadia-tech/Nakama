//! Sitzungen: welche Sensoren gehoeren zu demselben Hostprozess.
//!
//! Eine Sitzung ist der HOSTPROZESS, nicht das Projekt. Zwei gleichzeitig
//! laufende FL-Instanzen bleiben deshalb getrennte Sitzungen, auch wenn sie
//! dasselbe Projekt geladen haben — sonst mischten sich ihre Messstaende zu
//! einem Bild, das an keinem der beiden Rechner so existiert.

use crate::register::SensorEintrag;
use serde::Serialize;
use std::collections::HashMap;

/// Ein FL-Prozess (host_pid) als Sichtgruppe — mehrere FL-Prozesse bleiben in
/// der Übersicht getrennt (Plan §8.4, M2-Abnahme). Bridged Plugins desselben
/// Projekts können als eigene PID erscheinen; die Paar-Auswertung stuft solche
/// Paare deshalb höchstens auf »wahrscheinlich«.
#[derive(Debug, Clone, Serialize)]
pub struct SessionInfo {
    pub host_pid: u32,
    pub sensoren: u32,
    pub verbundene: u32,
    /// Dedupliziert und sortiert; mehr als ein Wert ⇒ uneinheitlich (sichtbar).
    pub samplerates: Vec<f64>,
}

pub fn sessions_bilden(sensoren: &[SensorEintrag]) -> Vec<SessionInfo> {
    let mut map: HashMap<u32, SessionInfo> = HashMap::new();
    for s in sensoren {
        let e = map.entry(s.host_pid).or_insert(SessionInfo {
            host_pid: s.host_pid,
            sensoren: 0,
            verbundene: 0,
            samplerates: Vec::new(),
        });
        e.sensoren += 1;
        if s.verbunden {
            e.verbundene += 1;
        }
        if s.samplerate > 0.0 && !e.samplerates.contains(&s.samplerate) {
            e.samplerates.push(s.samplerate);
        }
    }
    let mut liste: Vec<SessionInfo> = map.into_values().collect();
    for s in &mut liste {
        s.samplerates
            .sort_by(|a, b| a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal));
    }
    liste.sort_by_key(|s| s.host_pid);
    liste
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::testhilfe::*;
    use crate::{jetzt_ms, Register};

    #[test]
    fn zwei_fl_prozesse_bleiben_getrennte_sessions() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-a", "sensor", "A", None, 111), "n-a");
        r.sensor_verbinden(&hello("s-b", "sensor", "B", None, 111), "n-b");
        r.sensor_verbinden(&hello("s-c", "sensor", "C", None, 222), "n-c");
        let snap = r.sensoren_snapshot(jetzt_ms());
        let sessions = sessions_bilden(&snap);
        assert_eq!(sessions.len(), 2);
        assert_eq!(sessions[0].host_pid, 111);
        assert_eq!(sessions[0].sensoren, 2);
        assert_eq!(sessions[1].host_pid, 222);
        assert_eq!(sessions[1].sensoren, 1);
    }
}
