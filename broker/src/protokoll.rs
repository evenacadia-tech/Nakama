//! Nachrichten des IPC-Vertrags v2 (eq-copilot/schemas/eq-ipc.schema.json) und
//! die Handshake-Entscheidung — pur und testbar, ohne Pipe.
//!
//! Abwärtskompatibilität: der Broker nimmt v1-hello weiterhin an (Plan §9.2
//! „klarer Kompatibilitätsfehler ODER kompatibel bleiben"); v1-Verbindungen
//! bekommen keine heartbeat_acks und liefern keine Messdaten.

use serde::{Deserialize, Serialize};

pub const PROTOKOLL_VERSION: u32 = 2;
pub const MIN_PROTOKOLL: u32 = 1;

const MAX_SENSOR_ID_ZEICHEN: usize = 64;
const MAX_INSTANCE_NONCE_ZEICHEN: usize = 64;
const MAX_PLUGIN_VERSION_ZEICHEN: usize = 64;
const MAX_LABEL_ZEICHEN: usize = 120;
const MAX_PAIR_ID_ZEICHEN: usize = 64;
const MAX_SAMPLERATE: f64 = 768_000.0;
const MAX_BLOCK_SIZE: u32 = 65_536;
const MAX_CHANNELS: u32 = 64;
const MAX_SESSION_TOKEN_ZEICHEN: usize = 256;

#[derive(Debug, Deserialize)]
#[serde(tag = "type", rename_all = "snake_case")]
pub enum ClientNachricht {
    Hello(Hello),
    Heartbeat(Heartbeat),
    Bye(Bye),
}

#[derive(Debug, Clone, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Hello {
    pub protocol_version: u32,
    pub plugin_version: String,
    pub host_pid: u32,
    pub sensor: SensorAngabe,
    pub audio: AudioAngabe,
}

#[derive(Debug, Clone, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
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
#[serde(deny_unknown_fields)]
pub struct AudioAngabe {
    pub samplerate: f64,
    pub block_size: u32,
    pub channels: u32,
}

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Heartbeat {
    pub session_token: String,
    /// Gemeinsame Wire-Breite mit JUCE `var`: nichtnegative i64.
    pub seq: i64,
    #[serde(default)]
    pub stats: Option<HeartbeatStats>,
    /// v2: kompakter Messstand der AnalyseEngine. None bei v1 oder wenn das
    /// Plugin (noch) nichts zu melden hat.
    #[serde(default)]
    pub measurement: Option<MessStand>,
}

#[derive(Debug, Clone, Copy, Default, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct HeartbeatStats {
    #[serde(default)]
    pub rms_l: f32,
    #[serde(default)]
    pub rms_r: f32,
    #[serde(default)]
    /// Der C++-Writer serialisiert seinen uint64-Zähler als juce::int64.
    pub frames_dropped: i64,
    #[serde(default)]
    pub nan_seen: bool,
    #[serde(default)]
    pub transport_playing: Option<bool>,
    #[serde(default)]
    pub project_time_samples: Option<i64>,
}

impl Heartbeat {
    /// Wire-Grenzen, die serde-Typen allein nicht ausdrücken (nichtnegative
    /// signed Zähler und endliche RMS-Werte).
    pub fn ist_gueltig(&self) -> bool {
        !self.session_token.is_empty()
            && self.session_token.chars().count() <= MAX_SESSION_TOKEN_ZEICHEN
            && self.seq >= 0
            && self.stats.as_ref().is_none_or(HeartbeatStats::ist_gueltig)
    }
}

impl HeartbeatStats {
    fn ist_gueltig(&self) -> bool {
        self.rms_l.is_finite()
            && self.rms_l >= 0.0
            && self.rms_r.is_finite()
            && self.rms_r >= 0.0
            && self.frames_dropped >= 0
    }
}

/// Robustheits-Guard: mehr LTAS-Werte akzeptiert der Broker nicht (Schema
/// maxItems 512; real sind es 221). Ein größeres Array gilt als ungültiges
/// Paket und wird gezählt verworfen — nie still gekürzt.
pub const MAX_LTAS_WERTE: usize = 512;
pub const MAX_METRICS_VERSION_ZEICHEN: usize = 64;

/// Kompakter Messstand aus dem v2-Heartbeat (Form: `measurement` im Schema).
/// null-Felder heißen immer »nicht gültig/nicht messbar«, nie 0.
#[derive(Debug, Clone, Default, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct MessStand {
    pub zustand: String,
    pub metrics_version: String,
    /// Diese Plugin-Instanz färbt ihr hörbares Ausgangssignal gerade ein. Ihr
    /// eigener Messabgriff liegt davor und bleibt sauber; fremde Messpunkte
    /// können das gefärbte Signal sehen und müssen brokerseitig pausieren.
    #[serde(default)]
    pub hoermarkierung: bool,
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
        if self.metrics_version.is_empty()
            || self.metrics_version.chars().count() > MAX_METRICS_VERSION_ZEICHEN
            || !self.aktiv_s.is_finite()
            || self.aktiv_s < 0.0
            || !self.gesamt_s.is_finite()
            || self.gesamt_s < 0.0
        {
            return false;
        }
        let optionale_werte = [
            self.lufs_i,
            self.lufs_s,
            self.true_peak_db,
            self.crest_db,
            self.centroid_hz,
            self.low_frac,
            self.width,
            self.corr,
        ];
        if optionale_werte
            .into_iter()
            .flatten()
            .any(|wert| !wert.is_finite())
        {
            return false;
        }
        if let Some(ltas) = &self.ltas_komposit_db {
            if ltas.len() > MAX_LTAS_WERTE || ltas.iter().flatten().any(|wert| !wert.is_finite()) {
                return false;
            }
        }
        matches!(
            self.zustand.as_str(),
            "keine_daten" | "sammelt" | "messbereit"
        )
    }
}

/// Projektzeit-Fenster der akkumulierten Messung. spruenge zählt Loop-/Seek-
/// Diskontinuitäten — 0 heißt zusammenhängend erfasst (Plan §5.7).
#[derive(Debug, Clone, Copy, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct ProjektFenster {
    pub von_samples: i64,
    pub bis_samples: i64,
    pub spruenge: u32,
}

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
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
    HeartbeatAck { seq: i64, konflikt: bool },
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

    let reject = |reason: &str| BrokerNachricht::Reject {
        reason: reason.to_string(),
        min_protocol: MIN_PROTOKOLL,
        max_protocol: PROTOKOLL_VERSION,
    };
    let zeichen = |text: &str| text.chars().count();

    if hello.sensor.sensor_id.is_empty() || zeichen(&hello.sensor.sensor_id) > MAX_SENSOR_ID_ZEICHEN
    {
        return reject("sensor_id ist leer oder zu lang");
    }
    if hello.plugin_version.is_empty()
        || zeichen(&hello.plugin_version) > MAX_PLUGIN_VERSION_ZEICHEN
    {
        return reject("plugin_version ist leer oder zu lang");
    }
    if hello
        .sensor
        .instance_nonce
        .as_deref()
        .is_some_and(|wert| wert.is_empty() || zeichen(wert) > MAX_INSTANCE_NONCE_ZEICHEN)
    {
        return reject("instance_nonce ist leer oder zu lang");
    }
    if zeichen(&hello.sensor.label) > MAX_LABEL_ZEICHEN {
        return reject("label ist zu lang");
    }
    if hello
        .sensor
        .pair_id
        .as_deref()
        .is_some_and(|wert| wert.is_empty() || zeichen(wert) > MAX_PAIR_ID_ZEICHEN)
    {
        return reject("pair_id ist leer oder zu lang");
    }
    if !matches!(
        hello.sensor.role.as_str(),
        "sensor" | "hub" | "pre" | "post"
    ) {
        return reject("role ist unbekannt");
    }
    if !hello.audio.samplerate.is_finite()
        || !(0.0..=MAX_SAMPLERATE).contains(&hello.audio.samplerate)
    {
        return reject("samplerate liegt außerhalb der erlaubten Spanne");
    }
    if hello.audio.block_size > MAX_BLOCK_SIZE {
        return reject("block_size liegt außerhalb der erlaubten Spanne");
    }
    if hello.audio.channels > MAX_CHANNELS {
        return reject("channels liegt außerhalb der erlaubten Spanne");
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

    fn hello(version: u32) -> Hello {
        let n: ClientNachricht = serde_json::from_str(&hello_json(version)).unwrap();
        let ClientNachricht::Hello(h) = n else {
            panic!("kein hello")
        };
        h
    }

    fn wird_abgelehnt(hello: &Hello) -> bool {
        matches!(
            hello_pruefen(hello, "0.1.0", "tok"),
            BrokerNachricht::Reject { .. }
        )
    }

    #[test]
    fn v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1() {
        let n: ClientNachricht = serde_json::from_str(&hello_json(1)).unwrap();
        let ClientNachricht::Hello(h) = n else {
            panic!("kein hello")
        };
        assert_eq!(h.sensor.instance_nonce, None);
        match hello_pruefen(&h, "0.1.0", "tok") {
            BrokerNachricht::Welcome {
                protocol_version,
                session_token,
                ..
            } => {
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
        let ClientNachricht::Hello(h) = n else {
            panic!()
        };
        assert_eq!(h.sensor.instance_nonce.as_deref(), Some("n-abc"));
        match hello_pruefen(&h, "0.1.0", "tok") {
            BrokerNachricht::Welcome {
                protocol_version, ..
            } => assert_eq!(protocol_version, 2),
            _ => panic!("erwartet welcome"),
        }
    }

    #[test]
    fn version_3_bekommt_reject_mit_spanne() {
        let n: ClientNachricht = serde_json::from_str(&hello_json(3)).unwrap();
        let ClientNachricht::Hello(h) = n else {
            panic!()
        };
        match hello_pruefen(&h, "0.1.0", "tok") {
            BrokerNachricht::Reject {
                min_protocol,
                max_protocol,
                ..
            } => {
                assert_eq!((min_protocol, max_protocol), (1, 2));
            }
            _ => panic!("erwartet reject"),
        }
    }

    #[test]
    fn hello_textfelder_haben_codepunktgenaue_harte_grenzen() {
        let mut h = hello(2);
        h.sensor.label = "ä".repeat(MAX_LABEL_ZEICHEN);
        assert!(!wird_abgelehnt(&h));
        h.sensor.label.push('ä');
        assert!(wird_abgelehnt(&h));

        let mut h = hello(2);
        h.sensor.sensor_id = "s".repeat(MAX_SENSOR_ID_ZEICHEN + 1);
        assert!(wird_abgelehnt(&h));
        h.sensor.sensor_id.clear();
        assert!(wird_abgelehnt(&h));

        let mut h = hello(2);
        h.plugin_version = "v".repeat(MAX_PLUGIN_VERSION_ZEICHEN + 1);
        assert!(wird_abgelehnt(&h));
        h.plugin_version.clear();
        assert!(wird_abgelehnt(&h));

        let mut h = hello(2);
        h.sensor.instance_nonce = Some("n".repeat(MAX_INSTANCE_NONCE_ZEICHEN + 1));
        assert!(wird_abgelehnt(&h));
        h.sensor.instance_nonce = Some(String::new());
        assert!(wird_abgelehnt(&h));

        let mut h = hello(2);
        h.sensor.pair_id = Some("p".repeat(MAX_PAIR_ID_ZEICHEN + 1));
        assert!(wird_abgelehnt(&h));
        h.sensor.pair_id = Some(String::new());
        assert!(wird_abgelehnt(&h));
    }

    #[test]
    fn hello_enum_und_audiofelder_werden_semantisch_begrenzt() {
        // Nullwerte bleiben v2-kompatibel: das Plugin kann schon vor
        // prepareToPlay verbinden und aktualisiert sie beim Reconnect.
        let mut h = hello(2);
        h.audio = AudioAngabe {
            samplerate: 0.0,
            block_size: 0,
            channels: 0,
        };
        assert!(!wird_abgelehnt(&h));

        h.audio.samplerate = f64::NAN;
        assert!(wird_abgelehnt(&h));

        let mut h = hello(2);
        h.audio.samplerate = MAX_SAMPLERATE + 1.0;
        assert!(wird_abgelehnt(&h));

        let mut h = hello(2);
        h.audio.block_size = MAX_BLOCK_SIZE + 1;
        assert!(wird_abgelehnt(&h));

        let mut h = hello(2);
        h.audio.channels = MAX_CHANNELS + 1;
        assert!(wird_abgelehnt(&h));

        let mut h = hello(2);
        h.sensor.role = "admin".into();
        assert!(wird_abgelehnt(&h));
    }

    #[test]
    fn unbekannter_typ_ist_parsefehler_kein_absturz() {
        assert!(serde_json::from_str::<ClientNachricht>(r#"{"type":"gibtsnicht"}"#).is_err());
    }

    #[test]
    fn unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt() {
        let faelle = [
            r#"{"type":"heartbeat","session_token":"t","seq":1,"extra":true}"#,
            r#"{"type":"heartbeat","session_token":"t","seq":1,"stats":{"extra":true}}"#,
            r#"{"type":"heartbeat","session_token":"t","seq":1,"measurement":{"zustand":"sammelt","metrics_version":"m1","extra":true}}"#,
            r#"{"type":"heartbeat","session_token":"t","seq":1,"measurement":{"zustand":"sammelt","metrics_version":"m1","projekt_fenster":{"von_samples":0,"bis_samples":1,"spruenge":0,"extra":true}}}"#,
            r#"{"type":"hello","protocol_version":2,"plugin_version":"v","host_pid":1,"extra":true,"sensor":{"sensor_id":"s","role":"sensor","label":"L"},"audio":{"samplerate":0,"block_size":0,"channels":0}}"#,
            r#"{"type":"hello","protocol_version":2,"plugin_version":"v","host_pid":1,"sensor":{"sensor_id":"s","role":"sensor","label":"L","extra":true},"audio":{"samplerate":0,"block_size":0,"channels":0}}"#,
            r#"{"type":"hello","protocol_version":2,"plugin_version":"v","host_pid":1,"sensor":{"sensor_id":"s","role":"sensor","label":"L"},"audio":{"samplerate":0,"block_size":0,"channels":0,"extra":true}}"#,
            r#"{"type":"bye","session_token":"t","extra":true}"#,
        ];
        for json in faelle {
            assert!(
                serde_json::from_str::<ClientNachricht>(json).is_err(),
                "Zusatzfeld wurde akzeptiert: {json}"
            );
        }
    }

    #[test]
    fn option_felder_akzeptieren_explizites_null_wie_das_schema() {
        let hello = r#"{"type":"hello","protocol_version":2,"plugin_version":"v","host_pid":1,
            "sensor":{"sensor_id":"s","instance_nonce":null,"role":"sensor","label":"L","pair_id":null},
            "audio":{"samplerate":0,"block_size":0,"channels":0}}"#;
        let ClientNachricht::Hello(h) = serde_json::from_str(hello).unwrap() else {
            panic!()
        };
        assert!(h.sensor.instance_nonce.is_none());
        assert!(h.sensor.pair_id.is_none());

        let heartbeat =
            r#"{"type":"heartbeat","session_token":"t","seq":1,"stats":null,"measurement":null}"#;
        let ClientNachricht::Heartbeat(h) = serde_json::from_str(heartbeat).unwrap() else {
            panic!()
        };
        assert!(h.stats.is_none());
        assert!(h.measurement.is_none());
    }

    #[test]
    fn gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt() {
        let pid_max =
            hello_json(2).replace(r#""host_pid":42"#, &format!(r#""host_pid":{}"#, u32::MAX));
        assert!(serde_json::from_str::<ClientNachricht>(&pid_max).is_ok());
        let pid_zu_gross = hello_json(2).replace(
            r#""host_pid":42"#,
            &format!(r#""host_pid":{}"#, u64::from(u32::MAX) + 1),
        );
        assert!(serde_json::from_str::<ClientNachricht>(&pid_zu_gross).is_err());

        let hb_max = format!(
            r#"{{"type":"heartbeat","session_token":"t","seq":{},"stats":{{"frames_dropped":{},"project_time_samples":{}}}}}"#,
            i64::MAX,
            i64::MAX,
            i64::MIN
        );
        let ClientNachricht::Heartbeat(h) = serde_json::from_str(&hb_max).unwrap() else {
            panic!()
        };
        assert!(h.ist_gueltig());
        assert_eq!(h.stats.unwrap().project_time_samples, Some(i64::MIN));

        for json in [
            r#"{"type":"heartbeat","session_token":"t","seq":9223372036854775808}"#,
            r#"{"type":"heartbeat","session_token":"t","seq":0,"stats":{"frames_dropped":9223372036854775808}}"#,
            r#"{"type":"heartbeat","session_token":"t","seq":0,"stats":{"project_time_samples":9223372036854775808}}"#,
            r#"{"type":"heartbeat","session_token":"t","seq":0,"stats":{"project_time_samples":-9223372036854775809}}"#,
        ] {
            assert!(
                serde_json::from_str::<ClientNachricht>(json).is_err(),
                "Breitenüberlauf wurde akzeptiert: {json}"
            );
        }
        for json in [
            r#"{"type":"heartbeat","session_token":"t","seq":-1}"#,
            r#"{"type":"heartbeat","session_token":"t","seq":0,"stats":{"frames_dropped":-1}}"#,
        ] {
            let ClientNachricht::Heartbeat(h) = serde_json::from_str(json).unwrap() else {
                panic!()
            };
            assert!(
                !h.ist_gueltig(),
                "negativer Zähler wurde akzeptiert: {json}"
            );
        }

        let fenster_kanten = format!(
            r#"{{"type":"heartbeat","session_token":"t","seq":0,"measurement":{{"zustand":"sammelt","metrics_version":"m1","projekt_fenster":{{"von_samples":{},"bis_samples":{},"spruenge":{}}}}}}}"#,
            i64::MIN,
            i64::MAX,
            u32::MAX
        );
        assert!(serde_json::from_str::<ClientNachricht>(&fenster_kanten).is_ok());
        for wert in ["-9223372036854775809", "9223372036854775808"] {
            let json = format!(
                r#"{{"type":"heartbeat","session_token":"t","seq":0,"measurement":{{"zustand":"sammelt","metrics_version":"m1","projekt_fenster":{{"von_samples":{wert},"bis_samples":0,"spruenge":0}}}}}}"#
            );
            assert!(serde_json::from_str::<ClientNachricht>(&json).is_err());
        }
        let spruenge_zu_gross = r#"{"type":"heartbeat","session_token":"t","seq":0,
            "measurement":{"zustand":"sammelt","metrics_version":"m1",
            "projekt_fenster":{"von_samples":0,"bis_samples":1,"spruenge":4294967296}}}"#;
        assert!(serde_json::from_str::<ClientNachricht>(spruenge_zu_gross).is_err());
    }

    #[test]
    fn heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema() {
        for (laenge, gueltig) in [
            (0, false),
            (MAX_SESSION_TOKEN_ZEICHEN, true),
            (MAX_SESSION_TOKEN_ZEICHEN + 1, false),
        ] {
            let token = "ä".repeat(laenge);
            let json = format!(r#"{{"type":"heartbeat","session_token":"{token}","seq":0}}"#);
            let ClientNachricht::Heartbeat(h) = serde_json::from_str(&json).unwrap() else {
                panic!()
            };
            assert_eq!(h.ist_gueltig(), gueltig, "Tokenlänge {laenge}");
        }
    }

    #[test]
    fn gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow() {
        let stats_max = r#"{"type":"heartbeat","session_token":"t","seq":0,
            "stats":{"rms_l":3.4028234663852886e38,"rms_r":0}}"#;
        let ClientNachricht::Heartbeat(h) = serde_json::from_str(stats_max).unwrap() else {
            panic!()
        };
        assert!(h.ist_gueltig());
        assert_eq!(h.stats.unwrap().rms_l, f32::MAX);

        let stats_over = r#"{"type":"heartbeat","session_token":"t","seq":0,
            "stats":{"rms_l":3.4028236e38}}"#;
        match serde_json::from_str::<ClientNachricht>(stats_over) {
            Ok(ClientNachricht::Heartbeat(h)) => assert!(!h.ist_gueltig()),
            Err(_) => {}
            Ok(_) => panic!(),
        }

        let messung_max = r#"{"type":"heartbeat","session_token":"t","seq":0,
            "measurement":{"zustand":"sammelt","metrics_version":"m1",
                "aktiv_s":1.7976931348623157e308,"gesamt_s":0,
                "lufs_i":-1.7976931348623157e308}}"#;
        let ClientNachricht::Heartbeat(h) = serde_json::from_str(messung_max).unwrap() else {
            panic!()
        };
        assert!(h.measurement.unwrap().ist_gueltig());

        let messung_over = r#"{"type":"heartbeat","session_token":"t","seq":0,
            "measurement":{"zustand":"sammelt","metrics_version":"m1",
                "aktiv_s":1.7976931348623159e308,"gesamt_s":0}}"#;
        match serde_json::from_str::<ClientNachricht>(messung_over) {
            Ok(ClientNachricht::Heartbeat(h)) => {
                assert!(!h.measurement.unwrap().ist_gueltig())
            }
            Err(_) => {}
            Ok(_) => panic!(),
        }
    }

    #[test]
    fn heartbeat_v1_ohne_measurement_parst_weiter() {
        let hb = r#"{"type":"heartbeat","session_token":"t","seq":3,
            "stats":{"rms_l":0.1,"rms_r":0.1,"frames_dropped":0,"nan_seen":false,
                     "transport_playing":null,"project_time_samples":null}}"#;
        let n: ClientNachricht = serde_json::from_str(hb).unwrap();
        let ClientNachricht::Heartbeat(h) = n else {
            panic!()
        };
        assert_eq!(h.stats.unwrap().transport_playing, None);
        assert!(h.measurement.is_none());
    }

    #[test]
    fn heartbeat_v2_mit_messstand_und_fenster() {
        let hb = r#"{"type":"heartbeat","session_token":"t","seq":9,
            "measurement":{"zustand":"messbereit","metrics_version":"m1-2026-08-13",
                "hoermarkierung":true,"aktiv_s":12.5,"gesamt_s":14.0,"lufs_i":-18.2,"lufs_s":-17.0,
                "true_peak_db":-3.1,"crest_db":14.2,"centroid_hz":3400.0,"low_frac":0.41,
                "width":0.5,"corr":0.8,
                "ltas_komposit_db":[-40.0,null,-42.5],
                "projekt_fenster":{"von_samples":0,"bis_samples":480000,"spruenge":0}}}"#;
        let n: ClientNachricht = serde_json::from_str(hb).unwrap();
        let ClientNachricht::Heartbeat(h) = n else {
            panic!()
        };
        let m = h.measurement.unwrap();
        assert!(m.ist_gueltig());
        assert_eq!(m.zustand, "messbereit");
        assert!(m.hoermarkierung);
        assert_eq!(m.lufs_i, Some(-18.2));
        assert_eq!(m.ltas_komposit_db.as_ref().unwrap().len(), 3);
        assert_eq!(m.ltas_komposit_db.as_ref().unwrap()[1], None);
        let f = m.projekt_fenster.unwrap();
        assert_eq!((f.von_samples, f.bis_samples, f.spruenge), (0, 480000, 0));
    }

    #[test]
    fn fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false() {
        let hb = r#"{"type":"heartbeat","session_token":"t","seq":10,
            "measurement":{"zustand":"sammelt","metrics_version":"m1","gesamt_s":1.0}}"#;
        let n: ClientNachricht = serde_json::from_str(hb).unwrap();
        let ClientNachricht::Heartbeat(h) = n else {
            panic!()
        };
        assert!(!h.measurement.unwrap().hoermarkierung);
    }

    #[test]
    fn feindliches_ltas_array_faellt_am_guard() {
        let mut m = MessStand {
            zustand: "messbereit".into(),
            metrics_version: "m1".into(),
            ..Default::default()
        };
        m.ltas_komposit_db = Some(vec![Some(0.0); MAX_LTAS_WERTE + 1]);
        assert!(!m.ist_gueltig());
        m.ltas_komposit_db = Some(vec![Some(0.0); 221]);
        assert!(m.ist_gueltig());
        m.zustand = "quatsch".into();
        assert!(!m.ist_gueltig());
    }

    #[test]
    fn messstand_guard_prueft_minima_endlichkeit_und_textcap() {
        let basis = MessStand {
            zustand: "sammelt".into(),
            metrics_version: "m".repeat(MAX_METRICS_VERSION_ZEICHEN),
            aktiv_s: 1.0,
            gesamt_s: 2.0,
            ..Default::default()
        };
        assert!(basis.ist_gueltig());

        let mut m = basis.clone();
        m.aktiv_s = -0.1;
        assert!(!m.ist_gueltig());
        let mut m = basis.clone();
        m.gesamt_s = f64::INFINITY;
        assert!(!m.ist_gueltig());
        let mut m = basis.clone();
        m.lufs_i = Some(f64::NAN);
        assert!(!m.ist_gueltig());
        let mut m = basis.clone();
        m.ltas_komposit_db = Some(vec![Some(f64::NEG_INFINITY)]);
        assert!(!m.ist_gueltig());
        let mut m = basis;
        m.metrics_version.push('m');
        assert!(!m.ist_gueltig());
    }
}
