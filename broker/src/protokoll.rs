//! Nachrichten des IPC-Vertrags v2 (eq-copilot/schemas/eq-ipc.schema.json) und
//! die Handshake-Entscheidung — pur und testbar, ohne Pipe.
//!
//! Abwärtskompatibilität: der Broker nimmt v1-hello weiterhin an (Plan §9.2
//! „klarer Kompatibilitätsfehler ODER kompatibel bleiben"); v1-Verbindungen
//! bekommen keine heartbeat_acks und liefern keine Messdaten.

use serde::{Deserialize, Serialize};

pub const PROTOKOLL_VERSION: u32 = 2;
pub const MIN_PROTOKOLL: u32 = 1;

#[derive(Debug, Deserialize)]
#[serde(tag = "type", rename_all = "snake_case")]
pub enum ClientNachricht {
    Hello(Hello),
    Heartbeat(Heartbeat),
    Bye(Bye),
}

#[derive(Debug, Deserialize)]
pub struct Hello {
    pub protocol_version: u32,
    pub plugin_version: String,
    pub host_pid: u32,
    pub sensor: SensorAngabe,
    pub audio: AudioAngabe,
}

#[derive(Debug, Clone, Deserialize, Serialize)]
pub struct SensorAngabe {
    pub sensor_id: String,
    /// Flüchtige Verbindungs-ID pro Plugin-Instanz (v2) — unterscheidet zwei
    /// lebende Verbindungen derselben sensor_id nach FL-Duplikation (Plan §8.2).
    /// v1-Clients senden keine; der Server vergibt dann eine pro Verbindung.
    #[serde(default)]
    pub instance_nonce: Option<String>,
    pub role: String,
    /// User-Label — untrusted data: wird gespeichert und angezeigt, nie interpretiert.
    pub label: String,
    #[serde(default)]
    pub pair_id: Option<String>,
}

#[derive(Debug, Clone, Copy, Deserialize, Serialize)]
pub struct AudioAngabe {
    pub samplerate: f64,
    pub block_size: u32,
    pub channels: u32,
}

#[derive(Debug, Deserialize)]
pub struct Heartbeat {
    pub session_token: String,
    pub seq: u64,
    #[serde(default)]
    pub stats: Option<HeartbeatStats>,
    /// v2: kompakter Messstand der AnalyseEngine. None bei v1 oder wenn das
    /// Plugin (noch) nichts zu melden hat.
    #[serde(default)]
    pub measurement: Option<MessStand>,
}

#[derive(Debug, Clone, Copy, Default, Deserialize, Serialize)]
pub struct HeartbeatStats {
    #[serde(default)]
    pub rms_l: f32,
    #[serde(default)]
    pub rms_r: f32,
    #[serde(default)]
    pub frames_dropped: u64,
    #[serde(default)]
    pub nan_seen: bool,
    #[serde(default)]
    pub transport_playing: Option<bool>,
    #[serde(default)]
    pub project_time_samples: Option<i64>,
}

/// Robustheits-Guard: mehr LTAS-Werte akzeptiert der Broker nicht (Schema
/// maxItems 512; real sind es 221). Ein größeres Array gilt als ungültiges
/// Paket und wird gezählt verworfen — nie still gekürzt.
pub const MAX_LTAS_WERTE: usize = 512;

/// Kompakter Messstand aus dem v2-Heartbeat (Form: `measurement` im Schema).
/// null-Felder heißen immer »nicht gültig/nicht messbar«, nie 0.
#[derive(Debug, Clone, Default, Deserialize, Serialize)]
pub struct MessStand {
    pub zustand: String,
    pub metrics_version: String,
    #[serde(default)]
    pub aktiv_s: f64,
    #[serde(default)]
    pub gesamt_s: f64,
    #[serde(default)]
    pub lufs_i: Option<f64>,
    #[serde(default)]
    pub lufs_s: Option<f64>,
    #[serde(default)]
    pub true_peak_db: Option<f64>,
    #[serde(default)]
    pub crest_db: Option<f64>,
    #[serde(default)]
    pub centroid_hz: Option<f64>,
    #[serde(default)]
    pub low_frac: Option<f64>,
    #[serde(default)]
    pub width: Option<f64>,
    #[serde(default)]
    pub corr: Option<f64>,
    #[serde(default)]
    pub ltas_komposit_db: Option<Vec<Option<f64>>>,
    #[serde(default)]
    pub projekt_fenster: Option<ProjektFenster>,
}

impl MessStand {
    /// Schema-Guards, die serde nicht prüft. false ⇒ Paket verwerfen.
    pub fn ist_gueltig(&self) -> bool {
        if let Some(ltas) = &self.ltas_komposit_db {
            if ltas.len() > MAX_LTAS_WERTE {
                return false;
            }
        }
        matches!(self.zustand.as_str(), "keine_daten" | "sammelt" | "messbereit")
    }
}

/// Projektzeit-Fenster der akkumulierten Messung. spruenge zählt Loop-/Seek-
/// Diskontinuitäten — 0 heißt zusammenhängend erfasst (Plan §5.7).
#[derive(Debug, Clone, Copy, Deserialize, Serialize)]
pub struct ProjektFenster {
    pub von_samples: i64,
    pub bis_samples: i64,
    pub spruenge: u32,
}

#[derive(Debug, Deserialize)]
pub struct Bye {
    #[serde(default)]
    pub session_token: Option<String>,
}

#[derive(Debug, Serialize)]
#[serde(tag = "type", rename_all = "snake_case")]
pub enum BrokerNachricht {
    Welcome {
        protocol_version: u32,
        broker_version: String,
        session_token: String,
    },
    Reject {
        reason: String,
        min_protocol: u32,
        max_protocol: u32,
    },
    /// v2: Antwort auf jeden Heartbeat. konflikt=true ⇒ eine WEITERE lebende
    /// Verbindung meldet dieselbe sensor_id — das Plugin zeigt den Hinweis und
    /// bietet »neue Kennung« an (sichtbare Entscheidung, Plan §8.4).
    HeartbeatAck { seq: u64, konflikt: bool },
}

/// Handshake-Entscheidung: Versionen 1..=2 werden angenommen, das welcome
/// spiegelt die ANGENOMMENE Version (v1-Clients erwarten danach keine ACKs).
/// Alles andere bekommt einen klaren Kompatibilitätsfehler (Plan §9.2).
pub fn hello_pruefen(hello: &Hello, broker_version: &str, session_token: &str) -> BrokerNachricht {
    if hello.protocol_version < MIN_PROTOKOLL || hello.protocol_version > PROTOKOLL_VERSION {
        return BrokerNachricht::Reject {
            reason: format!(
                "Protokollversion {} wird nicht unterstützt",
                hello.protocol_version
            ),
            min_protocol: MIN_PROTOKOLL,
            max_protocol: PROTOKOLL_VERSION,
        };
    }
    BrokerNachricht::Welcome {
        protocol_version: hello.protocol_version,
        broker_version: broker_version.to_string(),
        session_token: session_token.to_string(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn hello_json(version: u32) -> String {
        format!(
            r#"{{"type":"hello","protocol_version":{version},"plugin_version":"0.1.0","host_pid":42,
                "sensor":{{"sensor_id":"s-1","role":"post","label":"PIANO-Bus POST","pair_id":"paar-a"}},
                "audio":{{"samplerate":48000.0,"block_size":512,"channels":2}}}}"#
        )
    }

    #[test]
    fn v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1() {
        let n: ClientNachricht = serde_json::from_str(&hello_json(1)).unwrap();
        let ClientNachricht::Hello(h) = n else { panic!("kein hello") };
        assert_eq!(h.sensor.instance_nonce, None);
        match hello_pruefen(&h, "0.1.0", "tok") {
            BrokerNachricht::Welcome { protocol_version, session_token, .. } => {
                assert_eq!(protocol_version, 1);
                assert_eq!(session_token, "tok");
            }
            _ => panic!("erwartet welcome"),
        }
    }

    #[test]
    fn v2_hello_mit_nonce_wird_angenommen() {
        let json = r#"{"type":"hello","protocol_version":2,"plugin_version":"0.2.0","host_pid":7,
            "sensor":{"sensor_id":"s-2","instance_nonce":"n-abc","role":"pre","label":"PIANO PRE","pair_id":"paar-a"},
            "audio":{"samplerate":48000.0,"block_size":512,"channels":2}}"#;
        let n: ClientNachricht = serde_json::from_str(json).unwrap();
        let ClientNachricht::Hello(h) = n else { panic!() };
        assert_eq!(h.sensor.instance_nonce.as_deref(), Some("n-abc"));
        match hello_pruefen(&h, "0.1.0", "tok") {
            BrokerNachricht::Welcome { protocol_version, .. } => assert_eq!(protocol_version, 2),
            _ => panic!("erwartet welcome"),
        }
    }

    #[test]
    fn version_3_bekommt_reject_mit_spanne() {
        let n: ClientNachricht = serde_json::from_str(&hello_json(3)).unwrap();
        let ClientNachricht::Hello(h) = n else { panic!() };
        match hello_pruefen(&h, "0.1.0", "tok") {
            BrokerNachricht::Reject { min_protocol, max_protocol, .. } => {
                assert_eq!((min_protocol, max_protocol), (1, 2));
            }
            _ => panic!("erwartet reject"),
        }
    }

    #[test]
    fn unbekannter_typ_ist_parsefehler_kein_absturz() {
        assert!(serde_json::from_str::<ClientNachricht>(r#"{"type":"gibtsnicht"}"#).is_err());
    }

    #[test]
    fn heartbeat_v1_ohne_measurement_parst_weiter() {
        let hb = r#"{"type":"heartbeat","session_token":"t","seq":3,
            "stats":{"rms_l":0.1,"rms_r":0.1,"frames_dropped":0,"nan_seen":false,
                     "transport_playing":null,"project_time_samples":null}}"#;
        let n: ClientNachricht = serde_json::from_str(hb).unwrap();
        let ClientNachricht::Heartbeat(h) = n else { panic!() };
        assert_eq!(h.stats.unwrap().transport_playing, None);
        assert!(h.measurement.is_none());
    }

    #[test]
    fn heartbeat_v2_mit_messstand_und_fenster() {
        let hb = r#"{"type":"heartbeat","session_token":"t","seq":9,
            "measurement":{"zustand":"messbereit","metrics_version":"m1-2026-08-13",
                "aktiv_s":12.5,"gesamt_s":14.0,"lufs_i":-18.2,"lufs_s":-17.0,
                "true_peak_db":-3.1,"crest_db":14.2,"centroid_hz":3400.0,"low_frac":0.41,
                "width":0.5,"corr":0.8,
                "ltas_komposit_db":[-40.0,null,-42.5],
                "projekt_fenster":{"von_samples":0,"bis_samples":480000,"spruenge":0}}}"#;
        let n: ClientNachricht = serde_json::from_str(hb).unwrap();
        let ClientNachricht::Heartbeat(h) = n else { panic!() };
        let m = h.measurement.unwrap();
        assert!(m.ist_gueltig());
        assert_eq!(m.zustand, "messbereit");
        assert_eq!(m.lufs_i, Some(-18.2));
        assert_eq!(m.ltas_komposit_db.as_ref().unwrap().len(), 3);
        assert_eq!(m.ltas_komposit_db.as_ref().unwrap()[1], None);
        let f = m.projekt_fenster.unwrap();
        assert_eq!((f.von_samples, f.bis_samples, f.spruenge), (0, 480000, 0));
    }

    #[test]
    fn feindliches_ltas_array_faellt_am_guard() {
        let mut m = MessStand { zustand: "messbereit".into(), ..Default::default() };
        m.ltas_komposit_db = Some(vec![Some(0.0); MAX_LTAS_WERTE + 1]);
        assert!(!m.ist_gueltig());
        m.ltas_komposit_db = Some(vec![Some(0.0); 221]);
        assert!(m.ist_gueltig());
        m.zustand = "quatsch".into();
        assert!(!m.ist_gueltig());
    }
}
