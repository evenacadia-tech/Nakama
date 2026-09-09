//! Das Sensorregister: wer meldet sich, wem gehoert eine Sensor-ID, was ist
//! noch frisch.
//!
//! Der Broker vergibt Sensor-IDs, haelt je Sensor den letzten Messstand und
//! macht Konflikte sichtbar. Drei Zusagen liegen hier und nirgends sonst:
//!
//! * **Stale misst monoton** (`STALE_MS` gegen `monoton_ms`), nie die
//!   Wanduhr — ein Zeitsprung darf keinen Sensor schlagartig alt machen.
//! * **Eigentum wechselt vollstaendig oder gar nicht.** Uebernimmt eine
//!   zweite Verbindung eine Sensor-ID, kommen Hello-Metadaten UND Rechte
//!   mit; ein halb uebernommener Eintrag waere ein Konflikt, den niemand
//!   sieht.
//! * **Die Hoermarkierung ist fail-closed** (`MAX_MARKIERUNGS_NONCES_PRO_SENSOR`):
//!   unbestaetigte Nonces wachsen nicht unbegrenzt, und nach einem
//!   Verbindungsabbruch bleibt die Sperre, bis ein neues Reset sie loest.

use crate::protokoll::{HeartbeatStats, Hello, MessStand};
use crate::SENSOR_ID_RUECKGABE_MS;
use crate::zeit::{jetzt_ms, monoton_ms, Zeitstempel};
use serde::Serialize;
use std::collections::HashMap;

/// Verbunden, aber länger als das hier ohne Heartbeat ⇒ stale. Sichtbar
/// markiert, nie still entfernt (Plan §11 M2-Abnahme).
pub const STALE_MS: u64 = 5000;

const MAX_MARKIERUNGS_NONCES_PRO_SENSOR: usize = 64;

#[derive(Debug, Clone)]
pub(crate) struct VerbindungsMetadaten {
    nonce: String,
    hello: Hello,
    verbunden_seit_ms: u64,
    last_seen_ms: u64,
    pub(crate) last_seen_monoton_ms: u64,
}

#[derive(Debug, Clone, Serialize)]
pub struct SensorEintrag {
    pub sensor_id: String,
    pub role: String,
    pub label: String,
    pub pair_id: Option<String>,
    /// Bindung an ein Projektprofil (stabile profile_id der App), per Command
    /// gesetzt und in bindung.rs persistiert. None = ungebunden.
    pub profil_id: Option<String>,
    pub plugin_version: String,
    /// Ausgehandelte Protokollversion der jüngsten Verbindung (1 = altes
    /// Plugin ohne Messdaten — die Übersicht sagt das dazu).
    pub protokoll_version: u32,
    pub host_pid: u32,
    pub samplerate: f64,
    pub block_size: u32,
    pub channels: u32,
    pub verbunden: bool,
    /// Mehr als eine lebende Verbindung meldet diese Sensor-ID (z. B. nach
    /// FL-Duplikation). Wird sichtbar gemacht und per heartbeat_ack an die
    /// Plugins gemeldet — neu gebunden wird nur durch sichtbare User-
    /// Entscheidung im Plugin, nie still (Plan §8.4).
    pub konflikt: bool,
    /// Verbunden, aber seit > STALE_MS kein Heartbeat. Wird bei jeder
    /// Status-Abfrage aus dem internen monotonen Zeitstempel berechnet.
    pub stale: bool,
    /// Lebende Verbindungen dieser Sensor-ID (1 = normal, >1 = Konflikt).
    pub lebende: u32,
    /// Kumulative Verbindungszählung über die Broker-Laufzeit.
    pub verbindungen: u32,
    pub heartbeats: u64,
    pub last_seen_ms: u64,
    pub verbunden_seit_ms: u64,
    /// Ein getrennter Sensor verschwindet nie still — er bleibt mit Zeitstempel.
    pub getrennt_seit_ms: Option<u64>,
    pub stats: HeartbeatStats,
    /// Letzter per v2-Heartbeat gemeldeter Messstand. None bei v1-Plugins
    /// oder vor dem ersten Heartbeat mit Messdaten.
    pub messung: Option<MessStand>,
    pub messung_ms: Option<u64>,
    /// Letzter gemeldeter Zustand der hörbaren Markierung. `true` bleibt bei
    /// stale/disconnect bewusst stehen, bis dieselbe Verbindung explizit
    /// `false` meldet: fehlende Telemetrie beweist kein Markierungsende.
    pub hoermarkierung: bool,
    /// Fremde Messung wurde während einer bekannten Hör-Markierung gesehen
    /// oder könnte wegen der 1-Hz-Reihenfolge betroffen sein. Solange dieses
    /// Bit steht, gibt der Broker den gespeicherten Messstand nicht als
    /// Evidenz aus; Freigabe braucht einen nachweisbaren Messreset.
    pub messung_gesperrt_durch_hoermarkierung: bool,
    /// Frische ist reine Laufzeitlogik. UTC bleibt nur für die sichtbaren
    /// `*_ms`-Felder; ein Wallclock-Sprung darf stale nicht beeinflussen.
    #[serde(skip)]
    last_seen_monoton_ms: u64,
    /// Jede lebende oder ohne bestätigtes `false` getrennte Marker-Verbindung
    /// bleibt einzeln sichtbar, auch bei duplizierter Sensor-ID.
    #[serde(skip)]
    pub(crate) markierungs_nonces: Vec<String>,
    /// Mehr unbestätigte Quellen werden nicht unbeschränkt gespeichert. Ein
    /// Überlauf bleibt stattdessen sticky/fail-closed bis zum Brokerneustart,
    /// weil die verworfene Nonce später nicht mehr sicher zuordenbar wäre.
    #[serde(skip)]
    pub(crate) markierung_unaufloesbar: bool,
    /// Erste saubere Beobachtung nach Ende aller fremden Markierungen. Erst
    /// ein späterer Rücklauf von `gesamt_s` beweist einen Reset danach.
    #[serde(skip)]
    pub(crate) entsperr_basis_gesamt_s: Option<f64>,
    /// Vollständige Hello-/Zeit-Metadaten je lebender Instanz. Beim Ende des
    /// jüngsten Owners kann nur daraus der überlebende Owner verlustfrei
    /// wiederhergestellt werden; Heartbeats tragen diese Felder nicht.
    #[serde(skip)]
    pub(crate) verbindungs_metadaten: Vec<VerbindungsMetadaten>,
    /// Besitzer der Schreibrechte auf stats/messung: die Nonce des jüngsten
    /// hello. Bei Konflikt schreibt nur sie — sonst flackerten zwei
    /// Instanzen gegeneinander.
    #[serde(skip)]
    pub(crate) besitzer_nonce: String,
    #[serde(skip)]
    pub(crate) lebende_nonces: Vec<String>,
}

fn hello_metadaten_uebernehmen(eintrag: &mut SensorEintrag, hello: &Hello) {
    eintrag.role = hello.sensor.role.clone();
    eintrag.label = hello.sensor.label.clone();
    eintrag.pair_id = hello.sensor.pair_id.clone();
    eintrag.plugin_version = hello.plugin_version.clone();
    eintrag.protokoll_version = hello.protocol_version;
    eintrag.host_pid = hello.host_pid;
    eintrag.samplerate = hello.audio.samplerate;
    eintrag.block_size = hello.audio.block_size;
    eintrag.channels = hello.audio.channels;
}

#[derive(Debug, Default)]
pub struct Register {
    pub sensoren: HashMap<String, SensorEintrag>,
    /// Persistente Profilbindungen sensor_id → profile_id (bindung.rs lädt/schreibt).
    pub bindungen: HashMap<String, String>,
    pub pakete_verworfen: u64,
    pub fehler: Vec<String>,
}

impl Register {
    /// `nonce` ist die effektive Verbindungs-Nonce: die des v2-Clients oder
    /// eine server-vergebene für v1 (damit die Konfliktzählung einheitlich ist).
    pub fn sensor_verbinden(&mut self, hello: &Hello, nonce: &str) {
        self.sensor_verbinden_zu(hello, nonce, Zeitstempel::jetzt());
    }

    fn sensor_verbinden_zu(&mut self, hello: &Hello, nonce: &str, zeit: Zeitstempel) {
        let fremde_markierung = self.hat_fremde_hoermarkierung(&hello.sensor.sensor_id, nonce);
        let jetzt = zeit.utc_ms;
        let profil = self.bindungen.get(&hello.sensor.sensor_id).cloned();
        let eintrag = self
            .sensoren
            .entry(hello.sensor.sensor_id.clone())
            .or_insert_with(|| SensorEintrag {
                sensor_id: hello.sensor.sensor_id.clone(),
                role: String::new(),
                label: String::new(),
                pair_id: None,
                profil_id: None,
                plugin_version: String::new(),
                protokoll_version: 0,
                host_pid: 0,
                samplerate: 0.0,
                block_size: 0,
                channels: 0,
                verbunden: false,
                konflikt: false,
                stale: false,
                lebende: 0,
                verbindungen: 0,
                heartbeats: 0,
                last_seen_ms: jetzt,
                verbunden_seit_ms: jetzt,
                getrennt_seit_ms: None,
                stats: HeartbeatStats::default(),
                messung: None,
                messung_ms: None,
                hoermarkierung: false,
                messung_gesperrt_durch_hoermarkierung: false,
                last_seen_monoton_ms: zeit.monoton_ms,
                markierungs_nonces: Vec::new(),
                markierung_unaufloesbar: false,
                entsperr_basis_gesamt_s: None,
                verbindungs_metadaten: Vec::new(),
                besitzer_nonce: String::new(),
                lebende_nonces: Vec::new(),
            });
        if !eintrag.lebende_nonces.iter().any(|n| n == nonce) {
            eintrag.lebende_nonces.push(nonce.to_string());
            eintrag.verbindungs_metadaten.push(VerbindungsMetadaten {
                nonce: nonce.to_string(),
                hello: hello.clone(),
                verbunden_seit_ms: jetzt,
                last_seen_ms: jetzt,
                last_seen_monoton_ms: zeit.monoton_ms,
            });
            eintrag.verbindungen = eintrag.verbindungen.saturating_add(1);
        } else if let Some(metadaten) = eintrag
            .verbindungs_metadaten
            .iter_mut()
            .find(|m| m.nonce == nonce)
        {
            // Der Server weist eine zweite lebende gleiche Nonce ab. Diese
            // idempotente Sicherung hält direkte Register-Nutzer trotzdem
            // konsistent, ohne eine zweite Lebenszählung zu erfinden.
            metadaten.hello = hello.clone();
            metadaten.last_seen_ms = jetzt;
            metadaten.last_seen_monoton_ms = zeit.monoton_ms;
        }
        eintrag.lebende = eintrag.lebende_nonces.len() as u32;
        eintrag.konflikt = eintrag.lebende > 1;
        // Das jüngste hello gewinnt die Schreibrechte und die Metadaten.
        eintrag.besitzer_nonce = nonce.to_string();
        hello_metadaten_uebernehmen(eintrag, hello);
        eintrag.profil_id = profil;
        eintrag.verbunden = true;
        eintrag.verbunden_seit_ms = jetzt;
        eintrag.getrennt_seit_ms = None;
        eintrag.last_seen_ms = jetzt;
        eintrag.last_seen_monoton_ms = zeit.monoton_ms;
        if fremde_markierung {
            eintrag.messung_gesperrt_durch_hoermarkierung = true;
            eintrag.entsperr_basis_gesamt_s = None;
        }
    }

    pub fn heartbeat(
        &mut self,
        sensor_id: &str,
        nonce: &str,
        stats: Option<HeartbeatStats>,
        messung: Option<MessStand>,
    ) {
        self.heartbeat_zu(sensor_id, nonce, stats, messung, Zeitstempel::jetzt());
    }

    fn heartbeat_zu(
        &mut self,
        sensor_id: &str,
        nonce: &str,
        stats: Option<HeartbeatStats>,
        messung: Option<MessStand>,
        zeit: Zeitstempel,
    ) {
        let Some(e) = self.sensoren.get_mut(sensor_id) else {
            return;
        };
        let Some(verbindung) = e
            .verbindungs_metadaten
            .iter_mut()
            .find(|m| m.nonce == nonce)
        else {
            return;
        };
        verbindung.last_seen_ms = zeit.utc_ms;
        verbindung.last_seen_monoton_ms = zeit.monoton_ms;

        // Eigener Zähler statt seq-Übernahme: seq beginnt nach jedem
        // Reconnect wieder bei 0, der Eintrag lebt über Verbindungen hinweg.
        e.heartbeats = e.heartbeats.saturating_add(1);
        e.last_seen_ms = zeit.utc_ms;
        e.last_seen_monoton_ms = zeit.monoton_ms;
        if e.besitzer_nonce.is_empty() {
            e.besitzer_nonce = nonce.to_string();
        }
        let ist_besitzer = e.besitzer_nonce == nonce;

        // Markierungszustand gehört zur Verbindung, nicht zu den exklusiven
        // Mess-Schreibrechten: auch eine duplizierte, nicht besitzende Instanz
        // kann hörbar färben. Ein fehlender Messblock ändert den letzten
        // Zustand nicht. Explizites false entfernt nur DIESE Nonce.
        let neue_markierung = messung.as_ref().map(|m| m.hoermarkierung);
        let markierung_begann = if neue_markierung == Some(true) {
            if e.markierung_unaufloesbar || e.markierungs_nonces.iter().any(|n| n == nonce) {
                false
            } else if e.markierungs_nonces.len() < MAX_MARKIERUNGS_NONCES_PRO_SENSOR {
                e.markierungs_nonces.push(nonce.to_string());
                true
            } else {
                e.markierung_unaufloesbar = true;
                true
            }
        } else {
            if neue_markierung == Some(false) {
                e.markierungs_nonces.retain(|n| n != nonce);
            }
            false
        };
        e.hoermarkierung = e.markierung_unaufloesbar || !e.markierungs_nonces.is_empty();

        if markierung_begann {
            self.fremde_messungen_sperren(sensor_id, nonce);
        }

        let fremde_markierung = self.hat_fremde_hoermarkierung(sensor_id, nonce);
        let Some(e) = self.sensoren.get_mut(sensor_id) else {
            return;
        };
        if !ist_besitzer {
            return;
        }
        if let Some(s) = stats {
            e.stats = s;
        }
        let Some(m) = messung else { return };

        if fremde_markierung {
            e.messung_gesperrt_durch_hoermarkierung = true;
            // Ein Reset während der Färbung ist kein Sauberkeitsbeweis. Die
            // erste Basis darf erst nach dem bestätigten Ende entstehen.
            e.entsperr_basis_gesamt_s = None;
            return;
        }

        if e.messung_gesperrt_durch_hoermarkierung {
            let leerer_reset = m.zustand == "keine_daten" && m.gesamt_s == 0.0 && m.aktiv_s == 0.0;
            let reset_nach_freier_basis = e
                .entsperr_basis_gesamt_s
                .is_some_and(|vorher| m.gesamt_s < vorher);
            if !leerer_reset && !reset_nach_freier_basis {
                e.entsperr_basis_gesamt_s = Some(m.gesamt_s);
                return;
            }
            e.messung_gesperrt_durch_hoermarkierung = false;
            e.entsperr_basis_gesamt_s = None;
        }

        e.messung_ms = Some(e.last_seen_ms);
        e.messung = Some(m);
    }

    fn hat_fremde_hoermarkierung(&self, sensor_id: &str, nonce: &str) -> bool {
        self.sensoren.values().any(|e| {
            e.markierung_unaufloesbar
                || e.markierungs_nonces
                    .iter()
                    .any(|marker_nonce| e.sensor_id != sensor_id || marker_nonce != nonce)
        })
    }

    fn fremde_messungen_sperren(&mut self, sensor_id: &str, nonce: &str) {
        for e in self.sensoren.values_mut() {
            // Nur der Messabgriff genau der färbenden Instanz liegt garantiert
            // vor ihrer eigenen Färbung. Jede andere ID oder Duplikat-Nonce
            // könnte das gefärbte Signal empfangen.
            if e.sensor_id != sensor_id || e.besitzer_nonce != nonce {
                e.messung_gesperrt_durch_hoermarkierung = true;
                e.entsperr_basis_gesamt_s = None;
            }
        }
    }

    /// Liefert das Konflikt-Flag für das heartbeat_ack der Verbindung.
    pub fn konflikt_von(&self, sensor_id: &str) -> bool {
        self.sensoren
            .get(sensor_id)
            .map(|e| e.konflikt)
            .unwrap_or(false)
    }

    pub(crate) fn verbindung_ist_lebend(&self, sensor_id: &str, nonce: &str) -> bool {
        self.sensoren
            .get(sensor_id)
            .is_some_and(|e| e.lebende_nonces.iter().any(|n| n == nonce))
    }

    /// Ein semantisch ungültiger Messblock darf den zuvor gültigen Stand des
    /// sendenden Owners nicht als scheinbar frische Evidenz zurücklassen.
    pub(crate) fn messung_verwerfen_von(&mut self, sensor_id: &str, nonce: &str) {
        if let Some(e) = self
            .sensoren
            .get_mut(sensor_id)
            .filter(|e| e.besitzer_nonce == nonce)
        {
            e.messung = None;
            e.messung_ms = None;
        }
    }

    pub fn sensor_trennen(&mut self, sensor_id: &str, nonce: &str) {
        self.sensor_trennen_zu(sensor_id, nonce, jetzt_ms());
    }

    fn sensor_trennen_zu(&mut self, sensor_id: &str, nonce: &str, utc_ms: u64) {
        if let Some(e) = self.sensoren.get_mut(sensor_id) {
            e.lebende_nonces.retain(|n| n != nonce);
            e.verbindungs_metadaten.retain(|m| m.nonce != nonce);
            e.lebende = e.lebende_nonces.len() as u32;
            e.konflikt = e.lebende > 1;
            if e.besitzer_nonce == nonce {
                if let Some(metadaten) = e.verbindungs_metadaten.last().cloned() {
                    // Hello-Metadaten existieren nur dort vollständig. Der
                    // Heartbeat des Überlebenden könnte Label/Paar/Rolle nie
                    // reparieren, wenn hier die Daten des Abgängers blieben.
                    e.besitzer_nonce = metadaten.nonce;
                    hello_metadaten_uebernehmen(e, &metadaten.hello);
                    e.verbunden_seit_ms = metadaten.verbunden_seit_ms;
                    e.last_seen_ms = metadaten.last_seen_ms;
                    e.last_seen_monoton_ms = metadaten.last_seen_monoton_ms;
                    // Messung/Stats gehörten dem alten Owner. Bis zum nächsten
                    // Heartbeat des neuen Owners sind sie keine Evidenz.
                    e.stats = HeartbeatStats::default();
                    e.messung = None;
                    e.messung_ms = None;
                } else {
                    e.besitzer_nonce.clear();
                }
            }
            if e.lebende == 0 {
                e.verbunden = false;
                e.getrennt_seit_ms = Some(utc_ms);
            }
            // `markierungs_nonces` absichtlich NICHT entfernen: ein Pipe-Ende
            // beweist nicht, dass die Audiofärbung schon neutral ist. Dieselbe
            // Instanz-Nonce kann nach Reconnect explizit false melden.
        }
    }

    /// Profilbindung setzen/lösen. Persistiert wird außerhalb (bindung.rs) —
    /// das Register bleibt pur testbar.
    pub fn profil_binden(&mut self, sensor_id: &str, profil_id: Option<String>) {
        match &profil_id {
            Some(p) => {
                self.bindungen.insert(sensor_id.to_string(), p.clone());
            }
            None => {
                self.bindungen.remove(sensor_id);
            }
        }
        if let Some(e) = self.sensoren.get_mut(sensor_id) {
            e.profil_id = profil_id;
        }
    }

    pub fn paket_verworfen(&mut self) {
        self.pakete_verworfen = self.pakete_verworfen.saturating_add(1);
    }

    pub fn fehler_merken(&mut self, text: String) {
        // Begrenzte Liste — sichtbar, aber kein unbegrenztes Wachstum (Plan §8.3).
        if self.fehler.len() >= 50 {
            self.fehler.remove(0);
        }
        self.fehler.push(text);
    }

    /// Momentaufnahme aller Sensoren mit frisch berechnetem stale-Flag,
    /// sortiert: verbundene zuerst, dann nach Label.
    pub fn sensoren_snapshot(&self, _jetzt_utc_ms: u64) -> Vec<SensorEintrag> {
        self.sensoren_snapshot_zu(monoton_ms())
    }

    /// H-13: gibt Sensor-IDs frei, deren Trennung die Rueckgabefrist erreicht
    /// hat, und liefert die Zahl der freigewordenen Plaetze.
    ///
    /// Der v2-Pool hatte bis NAK-121 keinen Rueckgabeweg: eine Suche nach
    /// `sensoren.remove`, `.retain` oder `.clear` traf im ganzen Crate
    /// nirgends. Einen periodischen v2-Tick gibt es auch nicht - die Schleifen
    /// in `server.rs` sind Accept- und Flushwartezeiten. Der Sweep laeuft
    /// deshalb im Gate selbst, unter demselben Lock wie die Grenzpruefung und
    /// VOR ihr.
    ///
    /// Die Grenze ist INKLUSIV: frei wird, wessen `getrennt_seit_ms`
    /// `jetzt - getrennt_seit_ms >= SENSOR_ID_RUECKGABE_MS` erfuellt. Der Wert
    /// ist nicht frei gewaehlt, sondern der Leerlaufhorizont, den der Broker
    /// fuer sich selbst fuehrt (`BROKER_IDLE_ENDE_MS`): wer laenger fort ist,
    /// als der Broker ohne jeden Sensor weiterlaeuft, hat keinen Anspruch mehr
    /// auf einen Platz. Verbundene Eintraege bleiben unberuehrt.
    pub fn getrennte_ids_freigeben_zu(&mut self, jetzt_utc_ms: u64) -> usize {
        let vorher = self.sensoren.len();
        self.sensoren.retain(|_, e| {
            if e.verbunden || e.lebende > 0 {
                return true;
            }
            match e.getrennt_seit_ms {
                Some(seit) => jetzt_utc_ms.saturating_sub(seit) < SENSOR_ID_RUECKGABE_MS,
                None => true,
            }
        });
        vorher - self.sensoren.len()
    }

    fn sensoren_snapshot_zu(&self, jetzt_monoton_ms: u64) -> Vec<SensorEintrag> {
        let mut sensoren: Vec<SensorEintrag> = self
            .sensoren
            .values()
            .cloned()
            .map(|mut e| {
                e.stale = e.verbunden
                    && jetzt_monoton_ms.saturating_sub(e.last_seen_monoton_ms) > STALE_MS;
                if e.messung_gesperrt_durch_hoermarkierung {
                    // Intern bleibt der letzte angenommene Stand für Diagnose
                    // und Resetvergleich erhalten; nach außen ist er solange
                    // ausdrücklich KEINE verwendbare Evidenz.
                    e.messung = None;
                    e.messung_ms = None;
                }
                e
            })
            .collect();
        sensoren.sort_by(|a, b| {
            b.verbunden
                .cmp(&a.verbunden)
                .then_with(|| a.label.cmp(&b.label))
                .then_with(|| a.sensor_id.cmp(&b.sensor_id))
        });
        sensoren
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::protokoll::AudioAngabe;
    use crate::testhilfe::*;
    use crate::{
        aggregat_sperrgrund, hoermarkierungs_sperrgrund, paare_auswerten, BROKER_IDLE_ENDE_MS,
    };

    #[test]
    fn sechzehn_sensoren_werden_korrekt_benannt() {
        let mut r = Register::default();
        for i in 0..16 {
            r.sensor_verbinden(
                &hello(&format!("s-{i}"), "sensor", &format!("Spur {i}"), None, 100),
                &format!("n-{i}"),
            );
        }
        let snap = r.sensoren_snapshot(jetzt_ms());
        assert_eq!(snap.len(), 16);
        for s in &snap {
            let i: u32 = s.sensor_id.trim_start_matches("s-").parse().unwrap();
            assert_eq!(s.label, format!("Spur {i}"));
            assert!(s.verbunden);
            assert!(!s.konflikt);
        }
    }

    #[test]
    fn snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id() {
        let mut r = Register::default();
        for id in ["s-c", "s-a", "s-b"] {
            r.sensor_verbinden(
                &hello(id, "sensor", "GLEICH", None, 100),
                &format!("n-{id}"),
            );
        }
        let ids: Vec<_> = r
            .sensoren_snapshot(jetzt_ms())
            .into_iter()
            .map(|s| s.sensor_id)
            .collect();
        assert_eq!(ids, ["s-a", "s-b", "s-c"]);
    }

    #[test]
    fn konflikt_kommt_und_geht_mit_der_zweiten_verbindung() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-dup", "post", "PIANO", None, 1), "n-a");
        assert!(!r.konflikt_von("s-dup"));
        r.sensor_verbinden(&hello("s-dup", "post", "PIANO Kopie", None, 1), "n-b");
        assert!(r.konflikt_von("s-dup"));
        // Nur der Besitzer (jüngstes hello = n-b) schreibt Werte.
        r.heartbeat(
            "s-dup",
            "n-a",
            Some(HeartbeatStats {
                rms_l: 0.9,
                ..Default::default()
            }),
            None,
        );
        r.heartbeat(
            "s-dup",
            "n-b",
            Some(HeartbeatStats {
                rms_l: 0.2,
                ..Default::default()
            }),
            None,
        );
        assert_eq!(r.sensoren["s-dup"].stats.rms_l, 0.2);
        // Eine Verbindung stirbt → Konflikt vorbei, Sensor bleibt verbunden.
        r.sensor_trennen("s-dup", "n-b");
        assert!(!r.konflikt_von("s-dup"));
        assert!(r.sensoren["s-dup"].verbunden);
        // Die überlebende Verbindung erbt die Schreibrechte.
        r.heartbeat(
            "s-dup",
            "n-a",
            Some(HeartbeatStats {
                rms_l: 0.7,
                ..Default::default()
            }),
            None,
        );
        assert_eq!(r.sensoren["s-dup"].stats.rms_l, 0.7);
        r.sensor_trennen("s-dup", "n-a");
        assert!(!r.sensoren["s-dup"].verbunden);
        assert!(r.sensoren["s-dup"].getrennt_seit_ms.is_some());
    }

    #[test]
    fn owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her() {
        let mut r = Register::default();
        let mut a = hello("s-owner", "pre", "A", Some("paar-a"), 11);
        a.plugin_version = "plugin-a".into();
        a.audio = AudioAngabe {
            samplerate: 44100.0,
            block_size: 256,
            channels: 1,
        };
        let mut b = hello("s-owner", "post", "B", Some("paar-b"), 22);
        b.plugin_version = "plugin-b".into();
        b.audio = AudioAngabe {
            samplerate: 96000.0,
            block_size: 1024,
            channels: 6,
        };

        r.sensor_verbinden_zu(
            &a,
            "nonce-a",
            Zeitstempel {
                utc_ms: 100,
                monoton_ms: 10,
            },
        );
        r.sensor_verbinden_zu(
            &b,
            "nonce-b",
            Zeitstempel {
                utc_ms: 200,
                monoton_ms: 20,
            },
        );
        r.heartbeat(
            "s-owner",
            "nonce-b",
            Some(HeartbeatStats {
                rms_l: 0.2,
                ..Default::default()
            }),
            Some(messbereit(None, 2.0)),
        );
        assert_eq!(r.sensoren["s-owner"].label, "B");

        r.sensor_trennen_zu("s-owner", "nonce-b", 300);
        let sensor = &r.sensoren["s-owner"];
        assert!(sensor.verbunden);
        assert_eq!(sensor.lebende, 1);
        assert!(!sensor.konflikt);
        assert_eq!(sensor.role, "pre");
        assert_eq!(sensor.label, "A");
        assert_eq!(sensor.pair_id.as_deref(), Some("paar-a"));
        assert_eq!(sensor.plugin_version, "plugin-a");
        assert_eq!(sensor.protokoll_version, 2);
        assert_eq!(sensor.host_pid, 11);
        assert_eq!(sensor.samplerate, 44100.0);
        assert_eq!(sensor.block_size, 256);
        assert_eq!(sensor.channels, 1);
        assert_eq!(sensor.stats.rms_l, 0.0);
        assert!(sensor.messung.is_none());

        // Der getrennte ehemalige Owner darf nicht weiter schreiben; A schon.
        let heartbeats_vorher = sensor.heartbeats;
        r.heartbeat(
            "s-owner",
            "nonce-b",
            Some(HeartbeatStats {
                rms_l: 0.9,
                ..Default::default()
            }),
            Some(messbereit(None, 9.0)),
        );
        assert_eq!(r.sensoren["s-owner"].heartbeats, heartbeats_vorher);
        r.heartbeat(
            "s-owner",
            "nonce-a",
            Some(HeartbeatStats {
                rms_l: 0.7,
                ..Default::default()
            }),
            Some(messbereit(None, 3.0)),
        );
        assert_eq!(r.sensoren["s-owner"].stats.rms_l, 0.7);
        assert_eq!(
            r.sensoren["s-owner"].messung.as_ref().unwrap().gesamt_s,
            3.0
        );
    }

    #[test]
    fn stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge() {
        let mut r = Register::default();
        r.sensor_verbinden_zu(
            &hello("s-1", "sensor", "CHOR", None, 1),
            "n-1",
            Zeitstempel {
                utc_ms: 1_000_000,
                monoton_ms: 100,
            },
        );
        // Wallclock springt rückwärts, monotone Zeit läuft normal weiter.
        r.heartbeat_zu(
            "s-1",
            "n-1",
            None,
            None,
            Zeitstempel {
                utc_ms: 10,
                monoton_ms: 200,
            },
        );
        let snap = r.sensoren_snapshot_zu(200 + STALE_MS);
        assert!(!snap[0].stale);
        assert_eq!(
            snap[0].last_seen_ms, 10,
            "sichtbares UTC bleibt Ausgabezeit"
        );

        // Wallclock springt weit vorwärts: auch das macht nicht sofort stale.
        r.heartbeat_zu(
            "s-1",
            "n-1",
            None,
            None,
            Zeitstempel {
                utc_ms: u64::MAX - 1,
                monoton_ms: 300,
            },
        );
        assert!(!r.sensoren_snapshot_zu(300)[0].stale);
        let snap = r.sensoren_snapshot_zu(300 + STALE_MS + 1);
        assert!(snap[0].stale, "verbunden ohne Heartbeats muss stale werden");
        assert!(snap[0].verbunden, "stale heißt sichtbar, nicht weg");
    }

    #[test]
    fn hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false() {
        let mut r = Register::default();
        paar_basis(&mut r);
        r.sensor_verbinden(&hello("s-marker", "sensor", "MARKER", None, 1), "n-m");
        r.heartbeat(
            "s-pre",
            "n-p",
            None,
            Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)),
        );
        r.heartbeat(
            "s-post",
            "n-q",
            None,
            Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)),
        );

        let mut marker = messbereit(None, 4.0);
        marker.hoermarkierung = true;
        marker.lufs_i = Some(-12.0);
        r.heartbeat("s-marker", "n-m", None, Some(marker));
        assert_eq!(
            r.sensoren["s-marker"].messung.as_ref().unwrap().lufs_i,
            Some(-12.0),
            "eigener Tap vor der Färbung bleibt verwendbar"
        );

        // Der bereits vorher angenommene fremde Stand wird sofort verborgen;
        // auch der nächste kumulative Stand während true wird nicht übernommen.
        let alter_gesamtstand = r.sensoren["s-pre"].messung.as_ref().unwrap().gesamt_s;
        r.heartbeat(
            "s-pre",
            "n-p",
            None,
            Some(messbereit(Some(fenster(0, 576000, 0)), 12.0)),
        );
        assert_eq!(
            r.sensoren["s-pre"].messung.as_ref().unwrap().gesamt_s,
            alter_gesamtstand
        );
        let snapshot = r.sensoren_snapshot(jetzt_ms());
        let pre = snapshot.iter().find(|s| s.sensor_id == "s-pre").unwrap();
        assert!(pre.messung_gesperrt_durch_hoermarkierung);
        assert!(pre.messung.is_none());
        let paar = &paare_auswerten(&snapshot)[0];
        assert_eq!(paar.timing, "unklar");
        assert!(paar.grund.contains("Hör-Markierung"));
        assert!(aggregat_sperrgrund(&snapshot).unwrap().contains("aktiv"));

        let mut marker_aus = messbereit(None, 5.0);
        marker_aus.hoermarkierung = false;
        r.heartbeat("s-marker", "n-m", None, Some(marker_aus));
        let snapshot = r.sensoren_snapshot(jetzt_ms());
        assert!(hoermarkierungs_sperrgrund(&snapshot).is_none());
        assert!(aggregat_sperrgrund(&snapshot)
            .unwrap()
            .contains("neu messen"));

        // Der erste Stand nach beobachtetem false ist nur eine Basis. Erst ein
        // danach beobachteter Reset (leer oder Zählerrücklauf) beweist Sauberkeit.
        r.heartbeat("s-pre", "n-p", None, Some(messbereit(None, 13.0)));
        r.heartbeat("s-post", "n-q", None, Some(messbereit(None, 13.0)));
        assert!(r.sensoren["s-pre"].messung_gesperrt_durch_hoermarkierung);
        let reset = MessStand {
            zustand: "keine_daten".into(),
            metrics_version: "m1-2026-08-13".into(),
            ..Default::default()
        };
        r.heartbeat("s-pre", "n-p", None, Some(reset));
        // POST beweist den Reset alternativ durch Rücklauf nach der freien Basis.
        r.heartbeat("s-post", "n-q", None, Some(messbereit(None, 1.0)));
        assert!(!r.sensoren["s-pre"].messung_gesperrt_durch_hoermarkierung);
        assert!(!r.sensoren["s-post"].messung_gesperrt_durch_hoermarkierung);

        r.heartbeat(
            "s-pre",
            "n-p",
            None,
            Some(messbereit(Some(fenster(0, 48000, 0)), 1.0)),
        );
        r.heartbeat(
            "s-post",
            "n-q",
            None,
            Some(messbereit(Some(fenster(0, 48000, 0)), 1.0)),
        );
        let snapshot = r.sensoren_snapshot(jetzt_ms());
        assert!(aggregat_sperrgrund(&snapshot).is_none());
        assert_eq!(paare_auswerten(&snapshot)[0].timing, "ausgerichtet");
    }

    #[test]
    fn marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset() {
        let mut r = Register::default();
        paar_basis(&mut r);
        r.sensor_verbinden(&hello("s-marker", "sensor", "MARKER", None, 1), "n-m");
        r.heartbeat("s-pre", "n-p", None, Some(messbereit(None, 10.0)));
        r.heartbeat("s-post", "n-q", None, Some(messbereit(None, 10.0)));
        let mut marker = messbereit(None, 1.0);
        marker.hoermarkierung = true;
        r.heartbeat("s-marker", "n-m", None, Some(marker));

        // Ein Reset während true kann zeitlich nicht als sauber bewiesen werden.
        let reset_waehrend_true = MessStand {
            zustand: "keine_daten".into(),
            metrics_version: "m1-2026-08-13".into(),
            ..Default::default()
        };
        r.heartbeat("s-pre", "n-p", None, Some(reset_waehrend_true));
        let marker_mono = r.sensoren["s-marker"].last_seen_monoton_ms;
        let stale = r.sensoren_snapshot_zu(marker_mono + STALE_MS + 1);
        assert!(hoermarkierungs_sperrgrund(&stale)
            .unwrap()
            .contains("Ende ist nicht bestätigt"));

        r.sensor_trennen("s-marker", "n-m");
        assert!(!r.verbindung_ist_lebend("s-marker", "n-m"));
        let getrennt = r.sensoren_snapshot(jetzt_ms());
        assert!(hoermarkierungs_sperrgrund(&getrennt)
            .unwrap()
            .contains("Trennung nicht als beendet bestätigt"));

        // Dieselbe nicht mehr lebende Instanz darf ihre unbestätigte true-
        // Markierung nach Reconnect explizit beenden.
        r.sensor_verbinden(&hello("s-marker", "sensor", "MARKER", None, 1), "n-m");
        let mut marker_aus = messbereit(None, 2.0);
        marker_aus.hoermarkierung = false;
        r.heartbeat("s-marker", "n-m", None, Some(marker_aus));
        assert!(hoermarkierungs_sperrgrund(&r.sensoren_snapshot(jetzt_ms())).is_none());

        // Der frühere Reset bleibt unbrauchbar: erste/steigende Stände nach
        // false lassen die sichtbare Quarantäne bestehen.
        r.heartbeat("s-pre", "n-p", None, Some(messbereit(None, 1.0)));
        r.heartbeat("s-pre", "n-p", None, Some(messbereit(None, 2.0)));
        assert!(r.sensoren["s-pre"].messung_gesperrt_durch_hoermarkierung);
        let snapshot = r.sensoren_snapshot(jetzt_ms());
        assert!(snapshot
            .iter()
            .find(|s| s.sensor_id == "s-pre")
            .unwrap()
            .messung
            .is_none());
        assert!(aggregat_sperrgrund(&snapshot)
            .unwrap()
            .contains("neu messen"));

        let reset_nach_false = MessStand {
            zustand: "keine_daten".into(),
            metrics_version: "m1-2026-08-13".into(),
            ..Default::default()
        };
        r.heartbeat("s-pre", "n-p", None, Some(reset_nach_false));
        assert!(!r.sensoren["s-pre"].messung_gesperrt_durch_hoermarkierung);
    }

    #[test]
    fn unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt() {
        let mut r = Register::default();
        for i in 0..(MAX_MARKIERUNGS_NONCES_PRO_SENSOR + 3) {
            let nonce = format!("marker-{i}");
            r.sensor_verbinden(&hello("s-marker-cap", "sensor", "MARKER", None, 1), &nonce);
            let mut marker = messbereit(None, 1.0);
            marker.hoermarkierung = true;
            r.heartbeat("s-marker-cap", &nonce, None, Some(marker));
            r.sensor_trennen("s-marker-cap", &nonce);
        }

        let sensor = &r.sensoren["s-marker-cap"];
        assert_eq!(
            sensor.markierungs_nonces.len(),
            MAX_MARKIERUNGS_NONCES_PRO_SENSOR
        );
        assert!(sensor.markierung_unaufloesbar);
        assert!(sensor.hoermarkierung);

        // Auch ein späteres false einer bekannten Quelle darf den nicht mehr
        // zuordenbaren Überlauf nicht fälschlich als vollständig beendet werten.
        r.sensor_verbinden(
            &hello("s-marker-cap", "sensor", "MARKER", None, 1),
            "marker-0",
        );
        let mut marker_aus = messbereit(None, 2.0);
        marker_aus.hoermarkierung = false;
        r.heartbeat("s-marker-cap", "marker-0", None, Some(marker_aus));
        assert!(r.sensoren["s-marker-cap"].hoermarkierung);
        assert!(aggregat_sperrgrund(&r.sensoren_snapshot(jetzt_ms()))
            .unwrap()
            .contains("nicht mehr eindeutig zuordenbar"));
    }

    #[test]
    fn profilbindung_haelt_ueber_reconnect() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-1", "sensor", "PIANO", None, 1), "n-1");
        r.profil_binden("s-1", Some("profil-even34".into()));
        assert_eq!(
            r.sensoren["s-1"].profil_id.as_deref(),
            Some("profil-even34")
        );
        r.sensor_trennen("s-1", "n-1");
        r.sensor_verbinden(&hello("s-1", "sensor", "PIANO", None, 1), "n-2");
        assert_eq!(
            r.sensoren["s-1"].profil_id.as_deref(),
            Some("profil-even34"),
            "Bindung muss den Reconnect überleben"
        );
        r.profil_binden("s-1", None);
        assert_eq!(r.sensoren["s-1"].profil_id, None);
        assert!(r.bindungen.is_empty());
    }

    /// NAK-121 H-13: der v2-Sensor-ID-Pool hat einen Rueckgabeweg mit benannter
    /// Frist. Bis dahin gab es gar keinen - eine Suche nach `sensoren.remove`,
    /// `.retain` oder `.clear` traf im ganzen Crate nirgends, und ein Client
    /// konnte den Pool seriell mit Tombstones fuellen.
    ///
    /// Gemessen ueber das `_zu`-Zeitseam, an den drei Kanten aus Paragraph 3.1:
    /// Frist minus eins, exakt Frist, und alles verbunden.
    #[test]
    fn v2_sensorpool_gibt_getrennte_ids_zurueck() {
        let basis: u64 = 1_700_000_000_000;

        // Kante 1: eine Millisekunde vor der Frist bleibt der Eintrag.
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-1", "sensor", "A", None, 11), "n-1");
        r.sensor_trennen_zu("s-1", "n-1", basis);
        assert_eq!(r.sensoren.len(), 1);
        assert_eq!(
            r.getrennte_ids_freigeben_zu(basis + SENSOR_ID_RUECKGABE_MS - 1),
            0
        );
        assert_eq!(r.sensoren.len(), 1, "vor der Frist bleibt der Platz belegt");

        // Kante 2: exakt auf der Frist faellt er - die Grenze ist INKLUSIV.
        assert_eq!(r.getrennte_ids_freigeben_zu(basis + SENSOR_ID_RUECKGABE_MS), 1);
        assert!(r.sensoren.is_empty(), "auf der Frist wird der Platz frei");

        // Kante 3: verbundene Eintraege sind unantastbar, egal wie alt. Der
        // harte Reject bleibt genau dann, wenn alle Plaetze wirklich verbunden
        // sind.
        let mut voll = Register::default();
        for i in 0..8 {
            let id = format!("s-v{i}");
            voll.sensor_verbinden(&hello(&id, "sensor", "V", None, 20 + i as u32), "n-v");
        }
        assert_eq!(voll.sensoren.len(), 8);
        assert_eq!(
            voll.getrennte_ids_freigeben_zu(basis + SENSOR_ID_RUECKGABE_MS * 10),
            0,
            "verbundene Sensoren geben ihren Platz nie zurueck"
        );
        assert_eq!(voll.sensoren.len(), 8);

        // Und die Frist ist der Leerlaufhorizont des Brokers selbst, nicht eine
        // frei gewaehlte Zahl.
        assert_eq!(SENSOR_ID_RUECKGABE_MS, BROKER_IDLE_ENDE_MS);
    }
}
