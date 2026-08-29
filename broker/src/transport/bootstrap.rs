//! SONDE-010 — Bootstrap und Kopplung der zwei v3-Verbindungen (Entwurf
//! §53.9, §33.1).
//!
//! Ablauf, absichtlich klein und eindeutig:
//!
//! 1. Die ERSTE Nachricht jeder Verbindung ist ein hoechstens 16 KiB grosses,
//!    nur `u32`-laengenpraefigiertes JSON-Hello — genau wie heute in v2.
//! 2. `protocol == 2` laeuft danach vollstaendig und unveraendert im
//!    v2-Parser weiter (`transport::legacy_v2`).
//! 3. `protocol == 3` mit `connection_kind = control`: der Broker antwortet
//!    v3-GERAHMT mit `welcome { link_id, challenge }`.
//! 4. Ein zweites Bootstrap-Hello mit `connection_kind = telemetry`,
//!    demselben `runtime_nonce`, derselben `link_id` und der Challenge
//!    koppelt die Telemetrieverbindung. Auch dessen Antwort ist v3-gerahmt.
//! 5. Ein ungekoppelter Telemetry-Connect und jeder Binaerframe anstelle
//!    eines Bootstrap-Hellos werden geschlossen.
//!
//! Nach dem jeweiligen Bootstrap-Request verwenden beide Seiten
//! ausschliesslich den 16-Byte-v3-Kopf.
//!
//! Was hier NICHT passiert: Sessionverwaltung, Eviction, Store, Outbox. Das
//! ist der `Coordinator` aus SONDE-011. Dieses Modul entscheidet Envelope,
//! Grenzen und Authentisierung — mehr nicht (§53.9 "I/O-Worker decodieren nur
//! Envelope, Grenzen und Authentisierung").

use std::collections::HashMap;

use serde::{Deserialize, Serialize};

use crate::transport::v3::MAX_BOOTSTRAP_BYTES;

/// Grund, aus dem eine Verbindung im Bootstrap geschlossen wird. Jeder Grund
/// ist ein eigener Wert, damit der Test nicht "irgendwie abgelehnt" prueft.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum BootstrapFehler {
    /// Laengenpraefix fehlt oder ist unvollstaendig.
    PraefixUnvollstaendig,
    /// Ueber 16 KiB — das ist kein Hello.
    ZuGross(u32),
    /// Laenge 0.
    Leer,
    /// Weniger Bytes da als angekuendigt.
    Unvollstaendig,
    /// Kein UTF-8 — insbesondere: ein Binaerframe statt eines Hellos.
    KeinUtf8,
    /// UTF-8, aber kein gueltiges JSON-Objekt.
    KeinJson(String),
    /// JSON, aber kein bekanntes Hello (falscher Typ, fehlendes Feld,
    /// unbekanntes Feld, falsche Protokollversion).
    KeinHello(String),
    /// Telemetry-Hello ohne passende Kopplung.
    KopplungUnbekannt,
    /// Kopplung existiert, aber Challenge oder Nonce passen nicht.
    KopplungPasstNicht,
    /// Diese Kopplung hat bereits eine Telemetrieverbindung.
    KopplungSchonBelegt,
}

/// Ergebnis des Bootstraps.
#[derive(Debug, Clone, PartialEq)]
pub enum Bootstrap {
    /// v2-Client: der rohe JSON-Text geht unveraendert an den heutigen
    /// v2-Pfad. Dieses Modul fasst ihn nicht an.
    V2 { roh: String },
    /// v3-Control, bereits validiert.
    V3Control(Box<HelloControl>),
    /// v3-Telemetry, bereits validiert (die Kopplung prueft `Kopplungen`).
    V3Telemetry(Box<HelloTelemetry>),
}

/// `adresse` aus `eq-ipc-v3.schema.json`. Keines dieser Felder ist additiv.
#[derive(Debug, Clone, PartialEq, Eq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct Adresse {
    pub logon_sid: String,
    pub project_binding_id: String,
    pub session_epoch: String,
    pub instance_id: String,
    pub runtime_nonce: String,
}

#[derive(Debug, Clone, Copy, PartialEq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct AudioLage {
    pub samplerate: f64,
    pub block_size: u32,
    pub channels: u32,
}

#[derive(Debug, Clone, PartialEq, Eq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct HostAngabe {
    pub pid: u32,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub name: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub version: Option<String>,
}

#[derive(Debug, Clone, PartialEq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct HelloControl {
    #[serde(rename = "type")]
    pub typ: String,
    pub connection_kind: String,
    pub protocol: u32,
    pub plugin_version: String,
    pub plugin_kind: String,
    pub adresse: Adresse,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub host: Option<HostAngabe>,
    pub audio: AudioLage,
}

#[derive(Debug, Clone, PartialEq, Eq, Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
pub struct HelloTelemetry {
    #[serde(rename = "type")]
    pub typ: String,
    pub connection_kind: String,
    pub protocol: u32,
    pub plugin_version: String,
    pub adresse: Adresse,
    pub link_id: String,
    pub challenge: String,
}

/// v3-`welcome` — die erste Antwort auf ein gueltiges Control-Hello, bereits
/// v3-gerahmt.
#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Welcome {
    #[serde(rename = "type")]
    pub typ: String,
    pub protocol: u32,
    pub broker_version: String,
    pub broker_epoch: String,
    pub link_id: String,
    pub challenge: String,
}

pub const PLUGIN_KIND_ERLAUBT: [&str; 4] = ["main", "passive_probe", "active_probe", "legacy"];

/// hex32 aus dem Schema: genau 32 Kleinbuchstaben-Hexziffern.
pub fn ist_hex32(s: &str) -> bool {
    s.len() == 32 && s.bytes().all(|b| b.is_ascii_digit() || (b'a'..=b'f').contains(&b))
}

fn adresse_pruefen(a: &Adresse) -> Result<(), String> {
    if a.logon_sid.is_empty() || a.logon_sid.chars().count() > 184 {
        return Err("logon_sid ausserhalb 1..184".into());
    }
    for (name, wert) in [
        ("project_binding_id", &a.project_binding_id),
        ("session_epoch", &a.session_epoch),
        ("instance_id", &a.instance_id),
        ("runtime_nonce", &a.runtime_nonce),
    ] {
        if !ist_hex32(wert) {
            return Err(format!("{name} ist kein hex32"));
        }
    }
    Ok(())
}

fn plugin_version_pruefen(v: &str) -> Result<(), String> {
    let n = v.chars().count();
    if n == 0 || n > 64 {
        return Err("plugin_version ausserhalb 1..64".into());
    }
    Ok(())
}

/// Liest die erste Nachricht einer Verbindung aus einem Bytepuffer.
///
/// Es gibt bewusst KEINEN Weg, hier einen v3-Binaerframe hineinzureichen: die
/// Funktion verlangt gueltiges UTF-8-JSON. Ein Binaerframe faellt an
/// `KeinUtf8` oder `KeinJson` — das ist die Regel "jeder Binaerframe
/// anstelle eines Bootstrap-Hellos wird geschlossen".
pub fn bootstrap_lesen(daten: &[u8]) -> Result<(Bootstrap, usize), BootstrapFehler> {
    if daten.len() < 4 {
        return Err(BootstrapFehler::PraefixUnvollstaendig);
    }
    let len = u32::from_le_bytes([daten[0], daten[1], daten[2], daten[3]]);
    if len == 0 {
        return Err(BootstrapFehler::Leer);
    }
    if len > MAX_BOOTSTRAP_BYTES {
        return Err(BootstrapFehler::ZuGross(len));
    }
    let ende = 4usize + len as usize;
    if daten.len() < ende {
        return Err(BootstrapFehler::Unvollstaendig);
    }
    let roh = std::str::from_utf8(&daten[4..ende]).map_err(|_| BootstrapFehler::KeinUtf8)?;
    let wert: serde_json::Value = serde_json::from_str(roh)
        .map_err(|e| BootstrapFehler::KeinJson(e.to_string()))?;
    let obj = wert
        .as_object()
        .ok_or_else(|| BootstrapFehler::KeinJson("Wurzel ist kein Objekt".into()))?;

    if obj.get("type").and_then(|v| v.as_str()) != Some("hello") {
        return Err(BootstrapFehler::KeinHello("type != hello".into()));
    }

    // Die Protokollentscheidung faellt VOR jeder Feldpruefung: v2 traegt
    // `protocol_version`, v3 traegt `protocol`. Ein v2-Hello darf hier nie
    // an v3-Regeln scheitern und umgekehrt.
    if let Some(pv) = obj.get("protocol_version").and_then(|v| v.as_u64()) {
        if pv <= crate::protokoll::PROTOKOLL_VERSION as u64 {
            return Ok((Bootstrap::V2 { roh: roh.to_string() }, ende));
        }
        return Err(BootstrapFehler::KeinHello(format!(
            "protocol_version {pv} ist keine v2-Version"
        )));
    }

    match obj.get("protocol").and_then(|v| v.as_u64()) {
        Some(3) => {}
        Some(p) => {
            return Err(BootstrapFehler::KeinHello(format!(
                "protocol {p} ist keine v3-Version"
            )))
        }
        None => return Err(BootstrapFehler::KeinHello("weder protocol noch protocol_version".into())),
    }

    match obj.get("connection_kind").and_then(|v| v.as_str()) {
        Some("control") => {
            let h: HelloControl = serde_json::from_str(roh)
                .map_err(|e| BootstrapFehler::KeinHello(e.to_string()))?;
            plugin_version_pruefen(&h.plugin_version).map_err(BootstrapFehler::KeinHello)?;
            if !PLUGIN_KIND_ERLAUBT.contains(&h.plugin_kind.as_str()) {
                return Err(BootstrapFehler::KeinHello(format!(
                    "plugin_kind {} ist nicht im Vertrag",
                    h.plugin_kind
                )));
            }
            adresse_pruefen(&h.adresse).map_err(BootstrapFehler::KeinHello)?;
            if !(h.audio.samplerate.is_finite()
                && h.audio.samplerate > 0.0
                && h.audio.samplerate <= 768_000.0
                && (1..=65_536).contains(&h.audio.block_size)
                && h.audio.channels <= 64)
            {
                return Err(BootstrapFehler::KeinHello("audio ausserhalb des Vertrags".into()));
            }
            Ok((Bootstrap::V3Control(Box::new(h)), ende))
        }
        Some("telemetry") => {
            let h: HelloTelemetry = serde_json::from_str(roh)
                .map_err(|e| BootstrapFehler::KeinHello(e.to_string()))?;
            plugin_version_pruefen(&h.plugin_version).map_err(BootstrapFehler::KeinHello)?;
            adresse_pruefen(&h.adresse).map_err(BootstrapFehler::KeinHello)?;
            if !ist_hex32(&h.link_id) || !ist_hex32(&h.challenge) {
                return Err(BootstrapFehler::KeinHello("link_id/challenge sind kein hex32".into()));
            }
            Ok((Bootstrap::V3Telemetry(Box::new(h)), ende))
        }
        other => Err(BootstrapFehler::KeinHello(format!(
            "connection_kind {other:?} ist nicht im Vertrag"
        ))),
    }
}

/// Eine offene Kopplung: genau eine Control- und hoechstens eine
/// Telemetrieverbindung, gebunden durch `link_id + runtime_nonce`.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Kopplung {
    pub link_id: String,
    pub challenge: String,
    pub runtime_nonce: String,
    pub telemetrie_verbunden: bool,
    /// Kennung der gekoppelten Telemetrieverbindung, solange eine haengt.
    /// Sie steht hier, damit das Abmelden der Control-Verbindung den
    /// Telemetriearbeiter WIRKLICH erreicht und nicht nur den Registereintrag
    /// entfernt (T2-Befund 2 vom 2026-08-29).
    pub telemetrie_verbindung: Option<u64>,
}

/// Ergebnis von `control_abmelden`: ob es die Kopplung gab und welche
/// Telemetrieverbindung mit ihr faellt.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub struct Abmeldung {
    pub war_da: bool,
    pub telemetrie_verbindung: Option<u64>,
}

/// Register der offenen Kopplungen. Absichtlich schmal: es kennt keine
/// Session, keine Sonde und kein Projekt — das ist SONDE-011.
#[derive(Debug, Default)]
pub struct Kopplungen {
    nach_link: HashMap<String, Kopplung>,
    max: usize,
}

/// Obergrenze offener Kopplungen. §48.1 verlangt feste Obergrenzen; ohne sie
/// waere ein lokaler Slowloris ein Speicherleck.
pub const MAX_KOPPLUNGEN: usize = 64;

impl Kopplungen {
    pub fn neu() -> Self {
        Self { nach_link: HashMap::new(), max: MAX_KOPPLUNGEN }
    }

    pub fn mit_grenze(max: usize) -> Self {
        Self { nach_link: HashMap::new(), max }
    }

    pub fn len(&self) -> usize {
        self.nach_link.len()
    }

    pub fn is_empty(&self) -> bool {
        self.nach_link.is_empty()
    }

    /// Legt zu einem Control-Hello eine Kopplung an und liefert sie. Die
    /// Werte kommen vom Aufrufer, damit der Test sie festlegen kann; im
    /// Betrieb erzeugt sie `neue_kennung()`.
    pub fn control_anmelden(
        &mut self,
        runtime_nonce: &str,
        link_id: String,
        challenge: String,
    ) -> Result<Kopplung, BootstrapFehler> {
        if self.nach_link.len() >= self.max {
            return Err(BootstrapFehler::KopplungSchonBelegt);
        }
        let k = Kopplung {
            link_id: link_id.clone(),
            challenge,
            runtime_nonce: runtime_nonce.to_string(),
            telemetrie_verbunden: false,
            telemetrie_verbindung: None,
        };
        self.nach_link.insert(link_id, k.clone());
        Ok(k)
    }

    /// Koppelt eine Telemetrieverbindung. Alle drei Merkmale muessen passen —
    /// `link_id` allein waere ein Namensraum, kein Nachweis. `verbindung` ist
    /// die Kennung des Telemetriearbeiters; ueber sie erreicht das Abmelden
    /// der Control-Verbindung ihn spaeter wirklich.
    pub fn telemetrie_koppeln(
        &mut self,
        h: &HelloTelemetry,
        verbindung: u64,
    ) -> Result<(), BootstrapFehler> {
        let k = self
            .nach_link
            .get_mut(&h.link_id)
            .ok_or(BootstrapFehler::KopplungUnbekannt)?;
        if k.challenge != h.challenge || k.runtime_nonce != h.adresse.runtime_nonce {
            return Err(BootstrapFehler::KopplungPasstNicht);
        }
        if k.telemetrie_verbunden {
            return Err(BootstrapFehler::KopplungSchonBelegt);
        }
        k.telemetrie_verbunden = true;
        k.telemetrie_verbindung = Some(verbindung);
        Ok(())
    }

    /// Lebt die Kopplung noch, und haengt genau DIESE Telemetrieverbindung
    /// daran? Der Telemetriearbeiter fragt das vor jedem Frame: eine
    /// abgemeldete Control-Verbindung nimmt ihren Telemetrieplatz mit, und
    /// ein Frame nach dem Abmelden gehoert zu keiner Sitzung mehr.
    pub fn telemetrie_lebt(&self, link_id: &str, verbindung: u64) -> bool {
        match self.nach_link.get(link_id) {
            Some(k) => k.telemetrie_verbunden && k.telemetrie_verbindung == Some(verbindung),
            None => false,
        }
    }

    /// Gegenpfad zu `control_anmelden` — gehoert in denselben Aenderungssatz
    /// (Arbeitsregel "verbinden<->trennen"). Ohne ihn waere jede getrennte
    /// Control-Verbindung ein dauerhaft belegter Kopplungsplatz. Das Ergebnis
    /// nennt die Telemetrieverbindung, die mit dieser Kopplung faellt; der
    /// Aufrufer bricht ihre I/O ab, sonst liefe sie als Waise weiter.
    pub fn control_abmelden(&mut self, link_id: &str) -> Abmeldung {
        match self.nach_link.remove(link_id) {
            Some(k) => Abmeldung { war_da: true, telemetrie_verbindung: k.telemetrie_verbindung },
            None => Abmeldung::default(),
        }
    }

    /// Gegenpfad zu `telemetrie_koppeln`: die Kopplung bleibt bestehen (die
    /// Control-Verbindung lebt weiter), nur der Telemetrieplatz wird frei.
    pub fn telemetrie_entkoppeln(&mut self, link_id: &str) -> bool {
        match self.nach_link.get_mut(link_id) {
            Some(k) if k.telemetrie_verbunden => {
                k.telemetrie_verbunden = false;
                k.telemetrie_verbindung = None;
                true
            }
            _ => false,
        }
    }

    pub fn nachschlagen(&self, link_id: &str) -> Option<&Kopplung> {
        self.nach_link.get(link_id)
    }
}

/// Frische 128-Bit-Kennung als hex32 (link_id, challenge, broker_epoch).
pub fn neue_kennung() -> String {
    uuid::Uuid::new_v4().simple().to_string()
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::transport::v3::{envelope_schreiben, Familie};

    fn praefix(json: &str) -> Vec<u8> {
        let b = json.as_bytes();
        let mut aus = (b.len() as u32).to_le_bytes().to_vec();
        aus.extend_from_slice(b);
        aus
    }

    fn adresse_json(nonce: &str) -> String {
        format!(
            "{{\"logon_sid\":\"S-1-5-21-1-2-3-1001\",\"project_binding_id\":\"{p}\",\
             \"session_epoch\":\"{p}\",\"instance_id\":\"{p}\",\"runtime_nonce\":\"{nonce}\"}}",
            p = "0".repeat(32)
        )
    }

    fn control_json(nonce: &str) -> String {
        format!(
            "{{\"type\":\"hello\",\"connection_kind\":\"control\",\"protocol\":3,\
             \"plugin_version\":\"0.3.0\",\"plugin_kind\":\"active_probe\",\
             \"adresse\":{a},\"audio\":{{\"samplerate\":48000,\"block_size\":512,\"channels\":2}}}}",
            a = adresse_json(nonce)
        )
    }

    fn telemetry_json(nonce: &str, link: &str, challenge: &str) -> String {
        format!(
            "{{\"type\":\"hello\",\"connection_kind\":\"telemetry\",\"protocol\":3,\
             \"plugin_version\":\"0.3.0\",\"adresse\":{a},\
             \"link_id\":\"{link}\",\"challenge\":\"{challenge}\"}}",
            a = adresse_json(nonce)
        )
    }

    #[test]
    fn v2_hello_bleibt_v2() {
        let roh = "{\"type\":\"hello\",\"protocol_version\":2,\"plugin_version\":\"0.3.0\"}";
        let (b, n) = bootstrap_lesen(&praefix(roh)).unwrap();
        assert_eq!(n, 4 + roh.len());
        match b {
            Bootstrap::V2 { roh: r } => assert_eq!(r, roh),
            andere => panic!("v2-Hello wurde zu {andere:?}"),
        }
    }

    #[test]
    fn v3_control_wird_vollstaendig_validiert() {
        let n = "a".repeat(32);
        let (b, _) = bootstrap_lesen(&praefix(&control_json(&n))).unwrap();
        match b {
            Bootstrap::V3Control(h) => {
                assert_eq!(h.adresse.runtime_nonce, n);
                assert_eq!(h.plugin_kind, "active_probe");
            }
            andere => panic!("{andere:?}"),
        }
    }

    #[test]
    fn unbekanntes_feld_im_hello_faellt() {
        let mut j = control_json(&"a".repeat(32));
        j = j.replace("\"audio\"", "\"zusatz\":1,\"audio\"");
        assert!(matches!(
            bootstrap_lesen(&praefix(&j)),
            Err(BootstrapFehler::KeinHello(_))
        ));
    }

    #[test]
    fn binaerframe_statt_hello_wird_abgelehnt() {
        // Ein echter v3-Frame, so wie ihn der Envelopeschreiber erzeugt.
        let f = envelope_schreiben(Familie::P2, 0, &[0xDE, 0xAD, 0xBE, 0xEF]).unwrap();
        let e = bootstrap_lesen(&f).unwrap_err();
        assert!(
            matches!(e, BootstrapFehler::KeinUtf8 | BootstrapFehler::KeinJson(_)),
            "Binaerframe wurde nicht als Nicht-Hello erkannt: {e:?}"
        );
        // Auch ein JSON-Payload im v3-Rahmen ist kein Bootstrap: der 16-Byte-
        // Kopf steht davor, also ist der Text kein JSON mehr.
        let f2 = envelope_schreiben(Familie::P0, 0, b"{\"type\":\"hello\"}").unwrap();
        assert!(bootstrap_lesen(&f2).is_err());
    }

    #[test]
    fn hello_ueber_16_kib_faellt_vor_dem_parser() {
        let gross = "x".repeat(MAX_BOOTSTRAP_BYTES as usize + 1);
        let mut daten = ((gross.len()) as u32).to_le_bytes().to_vec();
        daten.extend_from_slice(gross.as_bytes());
        assert_eq!(
            bootstrap_lesen(&daten),
            Err(BootstrapFehler::ZuGross(gross.len() as u32))
        );
    }

    #[test]
    fn kopplung_verlangt_alle_drei_merkmale() {
        let nonce = "b".repeat(32);
        let link = neue_kennung();
        let challenge = neue_kennung();
        let mut k = Kopplungen::neu();
        k.control_anmelden(&nonce, link.clone(), challenge.clone()).unwrap();

        // falsche Challenge
        let (b, _) = bootstrap_lesen(&praefix(&telemetry_json(&nonce, &link, &neue_kennung()))).unwrap();
        let Bootstrap::V3Telemetry(h) = b else { panic!() };
        assert_eq!(k.telemetrie_koppeln(&h, 1), Err(BootstrapFehler::KopplungPasstNicht));

        // falsche Nonce
        let (b, _) = bootstrap_lesen(&praefix(&telemetry_json(&"c".repeat(32), &link, &challenge))).unwrap();
        let Bootstrap::V3Telemetry(h) = b else { panic!() };
        assert_eq!(k.telemetrie_koppeln(&h, 1), Err(BootstrapFehler::KopplungPasstNicht));

        // unbekannte link_id
        let (b, _) = bootstrap_lesen(&praefix(&telemetry_json(&nonce, &neue_kennung(), &challenge))).unwrap();
        let Bootstrap::V3Telemetry(h) = b else { panic!() };
        assert_eq!(k.telemetrie_koppeln(&h, 1), Err(BootstrapFehler::KopplungUnbekannt));

        // alles richtig
        let (b, _) = bootstrap_lesen(&praefix(&telemetry_json(&nonce, &link, &challenge))).unwrap();
        let Bootstrap::V3Telemetry(h) = b else { panic!() };
        assert_eq!(k.telemetrie_koppeln(&h, 7), Ok(()));
        // und genau EINMAL
        assert_eq!(k.telemetrie_koppeln(&h, 1), Err(BootstrapFehler::KopplungSchonBelegt));
    }

    #[test]
    fn koppeln_und_entkoppeln_sind_symmetrisch() {
        let nonce = "d".repeat(32);
        let link = neue_kennung();
        let challenge = neue_kennung();
        let mut k = Kopplungen::neu();
        k.control_anmelden(&nonce, link.clone(), challenge.clone()).unwrap();
        let (b, _) = bootstrap_lesen(&praefix(&telemetry_json(&nonce, &link, &challenge))).unwrap();
        let Bootstrap::V3Telemetry(h) = b else { panic!() };
        assert_eq!(k.telemetrie_koppeln(&h, 7), Ok(()));
        assert!(k.nachschlagen(&link).unwrap().telemetrie_verbunden);

        assert!(k.telemetrie_entkoppeln(&link));
        assert!(!k.nachschlagen(&link).unwrap().telemetrie_verbunden);
        // Nach dem Entkoppeln darf dieselbe Kopplung wieder benutzt werden.
        assert_eq!(k.telemetrie_koppeln(&h, 7), Ok(()));

        let ab = k.control_abmelden(&link);
        assert!(ab.war_da);
        assert_eq!(
            ab.telemetrie_verbindung,
            Some(7),
            "das Abmelden muss die haengende Telemetrieverbindung NENNEN, sonst laeuft sie als Waise weiter"
        );
        assert_eq!(k.len(), 0);
        assert!(!k.control_abmelden(&link).war_da, "zweimal abmelden ist kein Erfolg");
        assert_eq!(k.telemetrie_koppeln(&h, 1), Err(BootstrapFehler::KopplungUnbekannt));
    }

    #[test]
    fn offene_kopplungen_sind_gedeckelt() {
        let mut k = Kopplungen::mit_grenze(2);
        assert!(k.control_anmelden("1", neue_kennung(), neue_kennung()).is_ok());
        assert!(k.control_anmelden("2", neue_kennung(), neue_kennung()).is_ok());
        assert!(k.control_anmelden("3", neue_kennung(), neue_kennung()).is_err());
    }

    #[test]
    fn hex32_riegel() {
        assert!(ist_hex32(&"0".repeat(32)));
        assert!(ist_hex32("0123456789abcdef0123456789abcdef"));
        assert!(!ist_hex32("0123456789ABCDEF0123456789ABCDEF"), "Grossbuchstaben sind kein hex32");
        assert!(!ist_hex32(&"0".repeat(31)));
        assert!(!ist_hex32(&"0".repeat(33)));
        assert!(!ist_hex32(""));
    }
}
