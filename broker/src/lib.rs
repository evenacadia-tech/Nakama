//! EQ-Copilot-Broker — M2 (Herkunft: Recherche §8.4, §9.2, §11, heute Archiv unter docs/archiv/; Produktwahrheit: CLAUDE.md).
//!
//! Eigener Scope neben den Katalog-/Bausteinmodellen (Plan §10.2): Named-Pipe-
//! Server für die Sammler-VST3-Instanzen, Sensor-Register mit Last-Seen,
//! Konflikt- und Stale-Sichtbarkeit, PRE/POST-Paar-Auswertung, Profilbindung
//! und der zeitausgerichtete Aggregat-Snapshot. Kein Audio, keine PCM-
//! Übertragung — Messwerte kommen als kompakter Messstand im v2-Heartbeat.
//! Die Verträge liegen in eq-copilot/schemas/.

pub mod aggregat;
pub mod bindung;
pub mod dto;
pub mod framing;
pub mod generiert;
pub mod protokoll;
pub mod telemetrie;
pub mod transport;
pub mod vertrag;
#[cfg(windows)]
mod server;

use protokoll::{HeartbeatStats, Hello, MessStand};
use serde::Serialize;
use std::collections::HashMap;
use std::path::PathBuf;
use std::sync::{Arc, Mutex, OnceLock};

/// Fester Pipename v1; Zugriff regelt die ACL (nur aktueller Windows-User).
/// Muss mit eqcop::kPipeName im Plugin übereinstimmen. Der Name bleibt auch
/// unter Protokoll v2 stabil — die Version handelt der Handshake aus.
pub const PIPE_NAME: &str = r"\\.\pipe\evenacadia.eq-copilot.v1";

/// Verbunden, aber länger als das hier ohne Heartbeat ⇒ stale. Sichtbar
/// markiert, nie still entfernt (Plan §11 M2-Abnahme).
pub const STALE_MS: u64 = 5000;
const MAX_MARKIERUNGS_NONCES_PRO_SENSOR: usize = 64;

pub fn jetzt_ms() -> u64 {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_millis() as u64)
        .unwrap_or(0)
}

static MONOTONER_START: OnceLock<std::time::Instant> = OnceLock::new();

fn monoton_ms() -> u64 {
    MONOTONER_START
        .get_or_init(std::time::Instant::now)
        .elapsed()
        .as_millis() as u64
}

#[derive(Debug, Clone, Copy)]
struct Zeitstempel {
    utc_ms: u64,
    monoton_ms: u64,
}

impl Zeitstempel {
    fn jetzt() -> Self {
        Self { utc_ms: jetzt_ms(), monoton_ms: monoton_ms() }
    }
}

#[derive(Debug, Clone)]
struct VerbindungsMetadaten {
    nonce: String,
    hello: Hello,
    verbunden_seit_ms: u64,
    last_seen_ms: u64,
    last_seen_monoton_ms: u64,
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
    markierungs_nonces: Vec<String>,
    /// Mehr unbestätigte Quellen werden nicht unbeschränkt gespeichert. Ein
    /// Überlauf bleibt stattdessen sticky/fail-closed bis zum Brokerneustart,
    /// weil die verworfene Nonce später nicht mehr sicher zuordenbar wäre.
    #[serde(skip)]
    markierung_unaufloesbar: bool,
    /// Erste saubere Beobachtung nach Ende aller fremden Markierungen. Erst
    /// ein späterer Rücklauf von `gesamt_s` beweist einen Reset danach.
    #[serde(skip)]
    entsperr_basis_gesamt_s: Option<f64>,
    /// Vollständige Hello-/Zeit-Metadaten je lebender Instanz. Beim Ende des
    /// jüngsten Owners kann nur daraus der überlebende Owner verlustfrei
    /// wiederhergestellt werden; Heartbeats tragen diese Felder nicht.
    #[serde(skip)]
    verbindungs_metadaten: Vec<VerbindungsMetadaten>,
    /// Besitzer der Schreibrechte auf stats/messung: die Nonce des jüngsten
    /// hello. Bei Konflikt schreibt nur sie — sonst flackerten zwei
    /// Instanzen gegeneinander.
    #[serde(skip)]
    besitzer_nonce: String,
    #[serde(skip)]
    lebende_nonces: Vec<String>,
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
        let Some(e) = self.sensoren.get_mut(sensor_id) else { return };
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
        let Some(e) = self.sensoren.get_mut(sensor_id) else { return };
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
        self.sensoren.get(sensor_id).map(|e| e.konflikt).unwrap_or(false)
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
        s.samplerates.sort_by(|a, b| a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal));
    }
    liste.sort_by_key(|s| s.host_pid);
    liste
}

/// PRE/POST-Paar-Auswertung (Plan §5.7): bei JEDER Unsicherheit wird
/// herabgestuft — »ausgerichtet« ist eine Fenster-Aussage, keine samplegenaue
/// Kausalitätsbehauptung (PDC-Schätzung existiert in M2 nicht).
#[derive(Debug, Clone, Serialize)]
pub struct PaarStatus {
    pub pair_id: String,
    pub pre_sensor_id: Option<String>,
    pub post_sensor_id: Option<String>,
    /// "vollstaendig" | "unvollstaendig"
    pub status: String,
    /// "ausgerichtet" | "wahrscheinlich" | "unklar"
    pub timing: String,
    /// Warum diese Stufe — nie leer.
    pub grund: String,
}

fn hoermarkierungs_sperrgrund(sensoren: &[SensorEintrag]) -> Option<String> {
    sensoren.iter().find(|s| s.hoermarkierung).map(|s| {
        let name = if s.label.is_empty() { &s.sensor_id } else { &s.label };
        if s.markierung_unaufloesbar {
            format!(
                "Hör-Markierung an »{name}« ist nach zu vielen unbestätigten Instanzen nicht mehr eindeutig zuordenbar"
            )
        } else if !s.verbunden {
            format!(
                "Hör-Markierung an »{name}« wurde vor der Trennung nicht als beendet bestätigt"
            )
        } else if s.stale {
            format!(
                "Hör-Markierung an »{name}« meldet sich nicht mehr — ihr Ende ist nicht bestätigt"
            )
        } else {
            format!(
                "Hör-Markierung an mindestens einer Instanz von »{name}« ist aktiv oder ihr Ende nicht bestätigt — fremde Messung ist pausiert"
            )
        }
    })
}

fn aggregat_sperrgrund(sensoren: &[SensorEintrag]) -> Option<String> {
    hoermarkierungs_sperrgrund(sensoren).or_else(|| {
        sensoren
            .iter()
            .find(|s| s.messung_gesperrt_durch_hoermarkierung)
            .map(|sensor| {
                format!(
                    "Messung von »{}« kann Hör-Markierung enthalten — bitte neu messen",
                    sensor.label
                )
            })
    })
}

pub fn paare_auswerten(sensoren: &[SensorEintrag]) -> Vec<PaarStatus> {
    let mut nach_paar: HashMap<&str, (Vec<&SensorEintrag>, Vec<&SensorEintrag>)> = HashMap::new();
    for s in sensoren {
        let Some(pair) = s.pair_id.as_deref().filter(|p| !p.is_empty()) else { continue };
        let eintrag = nach_paar.entry(pair).or_default();
        match s.role.as_str() {
            "pre" => eintrag.0.push(s),
            "post" => eintrag.1.push(s),
            _ => {}
        }
    }

    let mut ergebnis: Vec<PaarStatus> = Vec::new();
    for (pair_id, (pres, posts)) in nach_paar {
        if pres.is_empty() && posts.is_empty() {
            continue;
        }
        let mut p = PaarStatus {
            pair_id: pair_id.to_string(),
            pre_sensor_id: pres.first().map(|s| s.sensor_id.clone()),
            post_sensor_id: posts.first().map(|s| s.sensor_id.clone()),
            status: "unvollstaendig".into(),
            timing: "unklar".into(),
            grund: String::new(),
        };

        if pres.len() > 1 || posts.len() > 1 {
            let rolle = if pres.len() > 1 { "VORHER" } else { "NACHHER" };
            p.grund = format!("mehrere {rolle}-Messpunkte teilen dieselbe Paar-Kennung — bitte eindeutig machen");
            ergebnis.push(p);
            continue;
        }
        let (pre, post) = match (pres.first(), posts.first()) {
            (Some(a), Some(b)) => (*a, *b),
            (Some(_), None) => {
                p.grund = "der NACHHER-Messpunkt fehlt".into();
                ergebnis.push(p);
                continue;
            }
            (None, Some(_)) => {
                p.grund = "der VORHER-Messpunkt fehlt".into();
                ergebnis.push(p);
                continue;
            }
            (None, None) => unreachable!(),
        };
        p.status = "vollstaendig".into();

        // Harte Ausschlüsse → unklar.
        let unklar = |grund: String, p: &mut PaarStatus| {
            p.timing = "unklar".into();
            p.grund = grund;
        };
        if let Some(grund) = hoermarkierungs_sperrgrund(sensoren) {
            unklar(grund, &mut p);
            ergebnis.push(p);
            continue;
        }
        if pre.messung_gesperrt_durch_hoermarkierung
            || post.messung_gesperrt_durch_hoermarkierung
        {
            unklar(
                "Messung nach Hör-Markierung gesperrt — betroffene Messpunkte bitte neu messen"
                    .into(),
                &mut p,
            );
            ergebnis.push(p);
            continue;
        }
        if !pre.verbunden || !post.verbunden {
            let wer = if !pre.verbunden { "VORHER" } else { "NACHHER" };
            unklar(format!("{wer}-Messpunkt ist getrennt"), &mut p);
            ergebnis.push(p);
            continue;
        }
        if pre.stale || post.stale {
            let wer = if pre.stale { "VORHER" } else { "NACHHER" };
            unklar(format!("{wer}-Messpunkt sendet nicht mehr (letztes Lebenszeichen zu alt)"), &mut p);
            ergebnis.push(p);
            continue;
        }
        if pre.samplerate != post.samplerate {
            unklar("die beiden Messpunkte laufen mit verschiedenen Samplerates".into(), &mut p);
            ergebnis.push(p);
            continue;
        }
        let (Some(mp), Some(mq)) = (pre.messung.as_ref(), post.messung.as_ref()) else {
            let wer = if pre.messung.is_none() { pre } else { post };
            unklar(
                if wer.protokoll_version <= 1 {
                    format!("»{}« liefert keine Messdaten (altes Plugin, v1)", wer.label)
                } else {
                    format!("»{}« hat noch keinen Messstand gemeldet", wer.label)
                },
                &mut p,
            );
            ergebnis.push(p);
            continue;
        };
        if mp.zustand != "messbereit" || mq.zustand != "messbereit" {
            let wer = if mp.zustand != "messbereit" { pre } else { post };
            unklar(format!("»{}« sammelt noch — Messung nicht belastbar", wer.label), &mut p);
            ergebnis.push(p);
            continue;
        }
        let (Some(fp), Some(fq)) = (mp.projekt_fenster, mq.projekt_fenster) else {
            unklar("keine Projektzeit-Information — Messung lief ohne Transport?".into(), &mut p);
            ergebnis.push(p);
            continue;
        };
        let len_p = fp.bis_samples.saturating_sub(fp.von_samples);
        let len_q = fq.bis_samples.saturating_sub(fq.von_samples);
        if len_p <= 0 || len_q <= 0 {
            unklar("Messung lief ohne laufenden Transport — keine Projektzeit-Zuordnung".into(), &mut p);
            ergebnis.push(p);
            continue;
        }
        // Projektzeiten sind volle i64-Vertragswerte. Zwei gueltige Fenster an
        // entgegengesetzten Zahlenraendern duerfen weder im Debug-Build paniken
        // noch im Release-Build umbrechen und dadurch als deckungsgleich gelten.
        let overlap = fp
            .bis_samples
            .min(fq.bis_samples)
            .saturating_sub(fp.von_samples.max(fq.von_samples));
        if overlap <= 0 {
            unklar("die Messfenster überlappen nicht — vermutlich verschiedene Passagen".into(), &mut p);
            ergebnis.push(p);
            continue;
        }

        // Herabstufungen → wahrscheinlich (Plan §5.7 wörtlich: »pre/post wahrscheinlich«).
        let kuerzer = len_p.min(len_q);
        let mut wahrscheinlich: Option<String> = None;
        if fp.spruenge > 0 || fq.spruenge > 0 {
            wahrscheinlich = Some("Loop-/Seek-Sprünge im Messfenster — Passagen nicht sicher deckungsgleich".into());
        } else if (overlap as f64) < 0.8 * kuerzer as f64 {
            wahrscheinlich = Some("die Messfenster decken sich nur teilweise".into());
        } else if pre.host_pid != post.host_pid {
            wahrscheinlich = Some("die Messpunkte laufen in verschiedenen Prozessen (Bridge oder zweites FL?)".into());
        } else {
            let (a, g) = (mp.aktiv_s.min(mq.aktiv_s), mp.aktiv_s.max(mq.aktiv_s));
            if g > 0.0 && (g - a) > 0.1 * g {
                wahrscheinlich = Some("die aktive Messzeit unterscheidet sich deutlich (Smart Disable oder Stille auf einem Punkt?)".into());
            }
        }
        match wahrscheinlich {
            Some(grund) => {
                p.timing = "wahrscheinlich".into();
                p.grund = grund;
            }
            None => {
                p.timing = "ausgerichtet".into();
                p.grund = "Messfenster deckungsgleich und ohne Sprünge".into();
            }
        }
        ergebnis.push(p);
    }
    ergebnis.sort_by(|a, b| a.pair_id.cmp(&b.pair_id));
    ergebnis
}

#[derive(Debug, Clone, Serialize)]
pub struct BrokerStatus {
    pub laeuft: bool,
    pub broker_version: String,
    pub protokoll_version: u32,
    pub pipe_name: String,
    /// Nur die ersten 8 Zeichen — genug zum Wiedererkennen in Logs.
    pub session_token_kurz: String,
    pub gestartet_ms: u64,
    pub pakete_verworfen: u64,
    pub fehler: Vec<String>,
    pub sensoren: Vec<SensorEintrag>,
    pub sessions: Vec<SessionInfo>,
    pub paare: Vec<PaarStatus>,
}

struct BrokerLauf {
    #[cfg(windows)]
    _griff: server::ServerGriff,
    register: Arc<Mutex<Register>>,
    session_token: String,
    gestartet_ms: u64,
    bindungen_pfad: Option<PathBuf>,
}

static BROKER: OnceLock<Result<BrokerLauf, String>> = OnceLock::new();

fn broker_version() -> String {
    env!("CARGO_PKG_VERSION").to_string()
}

/// Startet den Broker genau einmal pro Prozess. Ein Fehler bleibt sichtbar im
/// Status stehen — die App läuft weiter, das Plugin misst lokal (Plan §6.5).
/// `bindungen_pfad` ist die Persistenzdatei der Profilbindungen (None = nur
/// im Speicher, z. B. in Tests).
pub fn broker_starten(bindungen_pfad: Option<PathBuf>) -> Result<(), String> {
    let ergebnis = BROKER.get_or_init(|| {
        #[cfg(windows)]
        {
            let mut register = Register::default();
            if let Some(pfad) = &bindungen_pfad {
                match bindung::bindungen_laden(pfad) {
                    Ok(b) => register.bindungen = b,
                    Err(e) => register.fehler_merken(format!("Profilbindungen: {e}")),
                }
            }
            let register = Arc::new(Mutex::new(register));
            let session_token = uuid::Uuid::new_v4().to_string();
            let griff = server::server_starten(
                PIPE_NAME,
                register.clone(),
                broker_version(),
                session_token.clone(),
            )?;
            Ok(BrokerLauf {
                _griff: griff,
                register,
                session_token,
                gestartet_ms: jetzt_ms(),
                bindungen_pfad,
            })
        }
        #[cfg(not(windows))]
        {
            let _ = bindungen_pfad;
            Err("EQ-Copilot-Broker ist V1 nur für Windows gebaut".to_string())
        }
    });
    ergebnis.as_ref().map(|_| ()).map_err(|e| e.clone())
}

pub fn broker_status() -> BrokerStatus {
    match BROKER.get() {
        Some(Ok(lauf)) => {
            let register = lauf.register.lock().expect("Register-Mutex");
            let sensoren = register.sensoren_snapshot(jetzt_ms());
            let sessions = sessions_bilden(&sensoren);
            let paare = paare_auswerten(&sensoren);
            BrokerStatus {
                laeuft: true,
                broker_version: broker_version(),
                protokoll_version: protokoll::PROTOKOLL_VERSION,
                pipe_name: PIPE_NAME.to_string(),
                session_token_kurz: lauf.session_token.chars().take(8).collect(),
                gestartet_ms: lauf.gestartet_ms,
                pakete_verworfen: register.pakete_verworfen,
                fehler: register.fehler.clone(),
                sensoren,
                sessions,
                paare,
            }
        }
        Some(Err(e)) => status_ohne_lauf(vec![e.clone()]),
        None => status_ohne_lauf(vec!["Broker wurde noch nicht gestartet".to_string()]),
    }
}

fn status_ohne_lauf(fehler: Vec<String>) -> BrokerStatus {
    BrokerStatus {
        laeuft: false,
        broker_version: broker_version(),
        protokoll_version: protokoll::PROTOKOLL_VERSION,
        pipe_name: PIPE_NAME.to_string(),
        session_token_kurz: String::new(),
        gestartet_ms: 0,
        pakete_verworfen: 0,
        fehler,
        sensoren: Vec::new(),
        sessions: Vec::new(),
        paare: Vec::new(),
    }
}

/// Prozessgrenzen-Probe: fährt einen ECHTEN Server (eigene ACL, eigenes
/// Register) auf einem FREI WÄHLBAREN Pipenamen — ohne den globalen OnceLock.
/// So kollidiert die Ende-zu-Ende-Probe nie mit dem Broker einer laufenden (bis 18.08.: Hub-App-gestarteten)
/// Hub-App (M2-Fund: zwei Broker auf einem Namen stehlen sich still Clients;
/// die erste Instanz trägt deshalb jetzt FILE_FLAG_FIRST_PIPE_INSTANCE).
/// Liefert am Ende den Status als JSON.
#[cfg(windows)]
pub fn probe_lauf(pipe_name: &str, sekunden: u64) -> Result<String, String> {
    let register = Arc::new(Mutex::new(Register::default()));
    let session_token = uuid::Uuid::new_v4().to_string();
    let griff = server::server_starten(
        pipe_name,
        register.clone(),
        broker_version(),
        session_token.clone(),
    )?;
    std::thread::sleep(std::time::Duration::from_secs(sekunden));
    let status = {
        let r = register.lock().map_err(|_| "Register-Mutex vergiftet".to_string())?;
        let sensoren = r.sensoren_snapshot(jetzt_ms());
        let sessions = sessions_bilden(&sensoren);
        let paare = paare_auswerten(&sensoren);
        BrokerStatus {
            laeuft: true,
            broker_version: broker_version(),
            protokoll_version: protokoll::PROTOKOLL_VERSION,
            pipe_name: pipe_name.to_string(),
            session_token_kurz: session_token.chars().take(8).collect(),
            gestartet_ms: 0,
            pakete_verworfen: r.pakete_verworfen,
            fehler: r.fehler.clone(),
            sensoren,
            sessions,
            paare,
        }
    };
    drop(griff);
    serde_json::to_string_pretty(&status).map_err(|e| format!("Status serialisieren: {e}"))
}

/// Profilbindung setzen/lösen + persistieren. Fehler beim Schreiben werden
/// zurückgegeben UND im Register sichtbar gemerkt — nie still (Plan §8.3).
pub fn profil_binden(sensor_id: &str, profil_id: Option<String>) -> Result<(), String> {
    let Some(Ok(lauf)) = BROKER.get() else {
        return Err("Broker läuft nicht".to_string());
    };
    let mut register = lauf.register.lock().expect("Register-Mutex");
    register.profil_binden(sensor_id, profil_id);
    if let Some(pfad) = &lauf.bindungen_pfad {
        if let Err(e) = bindung::bindungen_schreiben(pfad, &register.bindungen) {
            register.fehler_merken(format!("Profilbindungen schreiben: {e}"));
            return Err(e);
        }
    }
    Ok(())
}

/// Zeitausgerichteter Aggregat-Snapshot aller (gefilterten) Sensoren →
/// eine Datei unter %LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\.
/// Liefert den Pfad der geschriebenen Datei.
pub fn aggregat_schreiben(
    filter_profil: Option<String>,
    filter_pid: Option<u32>,
) -> Result<String, String> {
    let Some(Ok(lauf)) = BROKER.get() else {
        return Err("Broker läuft nicht".to_string());
    };
    let (sensoren, token_kurz) = {
        let register = lauf.register.lock().expect("Register-Mutex");
        (
            register.sensoren_snapshot(jetzt_ms()),
            lauf.session_token.chars().take(8).collect::<String>(),
        )
    };
    if let Some(grund) = aggregat_sperrgrund(&sensoren) {
        return Err(format!("Aggregat gesperrt: {grund}"));
    }
    let dokument = aggregat::aggregat_bauen(
        &sensoren,
        filter_profil.as_deref(),
        filter_pid,
        &broker_version(),
        &token_kurz,
        jetzt_ms(),
    );
    let pfad = aggregat::aggregat_schreiben(&dokument)?;
    Ok(pfad.display().to_string())
}

#[cfg(test)]
mod register_tests {
    use super::protokoll::{AudioAngabe, ProjektFenster, SensorAngabe};
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

    fn messbereit(fenster: Option<ProjektFenster>, aktiv_s: f64) -> MessStand {
        MessStand {
            zustand: "messbereit".into(),
            metrics_version: "m1-2026-08-13".into(),
            aktiv_s,
            gesamt_s: aktiv_s,
            projekt_fenster: fenster,
            ..Default::default()
        }
    }

    fn fenster(von: i64, bis: i64, spruenge: u32) -> ProjektFenster {
        ProjektFenster { von_samples: von, bis_samples: bis, spruenge }
    }

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
        r.heartbeat("s-dup", "n-a", Some(HeartbeatStats { rms_l: 0.9, ..Default::default() }), None);
        r.heartbeat("s-dup", "n-b", Some(HeartbeatStats { rms_l: 0.2, ..Default::default() }), None);
        assert_eq!(r.sensoren["s-dup"].stats.rms_l, 0.2);
        // Eine Verbindung stirbt → Konflikt vorbei, Sensor bleibt verbunden.
        r.sensor_trennen("s-dup", "n-b");
        assert!(!r.konflikt_von("s-dup"));
        assert!(r.sensoren["s-dup"].verbunden);
        // Die überlebende Verbindung erbt die Schreibrechte.
        r.heartbeat("s-dup", "n-a", Some(HeartbeatStats { rms_l: 0.7, ..Default::default() }), None);
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
        a.audio = AudioAngabe { samplerate: 44100.0, block_size: 256, channels: 1 };
        let mut b = hello("s-owner", "post", "B", Some("paar-b"), 22);
        b.plugin_version = "plugin-b".into();
        b.audio = AudioAngabe { samplerate: 96000.0, block_size: 1024, channels: 6 };

        r.sensor_verbinden_zu(&a, "nonce-a", Zeitstempel { utc_ms: 100, monoton_ms: 10 });
        r.sensor_verbinden_zu(&b, "nonce-b", Zeitstempel { utc_ms: 200, monoton_ms: 20 });
        r.heartbeat(
            "s-owner",
            "nonce-b",
            Some(HeartbeatStats { rms_l: 0.2, ..Default::default() }),
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
            Some(HeartbeatStats { rms_l: 0.9, ..Default::default() }),
            Some(messbereit(None, 9.0)),
        );
        assert_eq!(r.sensoren["s-owner"].heartbeats, heartbeats_vorher);
        r.heartbeat(
            "s-owner",
            "nonce-a",
            Some(HeartbeatStats { rms_l: 0.7, ..Default::default() }),
            Some(messbereit(None, 3.0)),
        );
        assert_eq!(r.sensoren["s-owner"].stats.rms_l, 0.7);
        assert_eq!(r.sensoren["s-owner"].messung.as_ref().unwrap().gesamt_s, 3.0);
    }

    #[test]
    fn stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge() {
        let mut r = Register::default();
        r.sensor_verbinden_zu(
            &hello("s-1", "sensor", "CHOR", None, 1),
            "n-1",
            Zeitstempel { utc_ms: 1_000_000, monoton_ms: 100 },
        );
        // Wallclock springt rückwärts, monotone Zeit läuft normal weiter.
        r.heartbeat_zu(
            "s-1",
            "n-1",
            None,
            None,
            Zeitstempel { utc_ms: 10, monoton_ms: 200 },
        );
        let snap = r.sensoren_snapshot_zu(200 + STALE_MS);
        assert!(!snap[0].stale);
        assert_eq!(snap[0].last_seen_ms, 10, "sichtbares UTC bleibt Ausgabezeit");

        // Wallclock springt weit vorwärts: auch das macht nicht sofort stale.
        r.heartbeat_zu(
            "s-1",
            "n-1",
            None,
            None,
            Zeitstempel { utc_ms: u64::MAX - 1, monoton_ms: 300 },
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
        assert!(aggregat_sperrgrund(&snapshot).unwrap().contains("neu messen"));

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
        assert!(snapshot.iter().find(|s| s.sensor_id == "s-pre").unwrap().messung.is_none());
        assert!(aggregat_sperrgrund(&snapshot).unwrap().contains("neu messen"));

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

    #[test]
    fn profilbindung_haelt_ueber_reconnect() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-1", "sensor", "PIANO", None, 1), "n-1");
        r.profil_binden("s-1", Some("profil-even34".into()));
        assert_eq!(r.sensoren["s-1"].profil_id.as_deref(), Some("profil-even34"));
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

    // ── Paar-Auswertung (Plan §5.7: Herabstufung bei jeder Unsicherheit) ──

    fn paar_basis(r: &mut Register) {
        r.sensor_verbinden(&hello("s-pre", "pre", "PIANO VORHER", Some("paar-1"), 1), "n-p");
        r.sensor_verbinden(&hello("s-post", "post", "PIANO NACHHER", Some("paar-1"), 1), "n-q");
    }

    #[test]
    fn paar_ausgerichtet_bei_deckungsgleichen_fenstern() {
        let mut r = Register::default();
        paar_basis(&mut r);
        r.heartbeat("s-pre", "n-p", None, Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)));
        r.heartbeat("s-post", "n-q", None, Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)));
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare.len(), 1);
        assert_eq!(paare[0].status, "vollstaendig");
        assert_eq!(paare[0].timing, "ausgerichtet", "Grund: {}", paare[0].grund);
    }

    #[test]
    fn paar_wird_bei_spruengen_herabgestuft() {
        let mut r = Register::default();
        paar_basis(&mut r);
        r.heartbeat("s-pre", "n-p", None, Some(messbereit(Some(fenster(0, 480000, 2)), 10.0)));
        r.heartbeat("s-post", "n-q", None, Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)));
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "wahrscheinlich");
        assert!(paare[0].grund.contains("Sprünge"), "Grund war: {}", paare[0].grund);
    }

    #[test]
    fn paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar() {
        let mut r = Register::default();
        paar_basis(&mut r);
        // 50 % Überlappung → wahrscheinlich.
        r.heartbeat("s-pre", "n-p", None, Some(messbereit(Some(fenster(0, 100_000, 0)), 10.0)));
        r.heartbeat("s-post", "n-q", None, Some(messbereit(Some(fenster(50_000, 150_000, 0)), 10.0)));
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "wahrscheinlich");
        // Keine Überlappung → unklar.
        r.heartbeat("s-post", "n-q", None, Some(messbereit(Some(fenster(500_000, 600_000, 0)), 10.0)));
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "unklar");
        assert!(paare[0].grund.contains("überlappen nicht"));
    }

    #[test]
    fn paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen() {
        let mut r = Register::default();
        paar_basis(&mut r);
        r.heartbeat(
            "s-pre",
            "n-p",
            None,
            Some(messbereit(Some(fenster(i64::MIN, i64::MIN + 4096, 0)), 1.0)),
        );
        r.heartbeat(
            "s-post",
            "n-q",
            None,
            Some(messbereit(Some(fenster(i64::MAX - 4096, i64::MAX, 0)), 1.0)),
        );
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "unklar");
        assert!(paare[0].grund.contains("überlappen nicht"));
    }

    #[test]
    fn paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-pre", "pre", "PIANO VORHER", Some("paar-1"), 1), "n-p");
        let mut alt = hello("s-post", "post", "PIANO NACHHER", Some("paar-1"), 1);
        alt.protocol_version = 1;
        r.sensor_verbinden(&alt, "n-q");
        r.heartbeat("s-pre", "n-p", None, Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)));
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "unklar");
        assert!(paare[0].grund.contains("v1"), "Grund war: {}", paare[0].grund);
    }

    #[test]
    fn paar_ohne_partner_ist_unvollstaendig() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-pre", "pre", "PIANO VORHER", Some("paar-1"), 1), "n-p");
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare.len(), 1);
        assert_eq!(paare[0].status, "unvollstaendig");
        assert!(paare[0].grund.contains("NACHHER"));
    }

    #[test]
    fn paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich() {
        let mut r = Register::default();
        r.sensor_verbinden(&hello("s-pre", "pre", "V", Some("paar-1"), 111), "n-p");
        r.sensor_verbinden(&hello("s-post", "post", "N", Some("paar-1"), 222), "n-q");
        r.heartbeat("s-pre", "n-p", None, Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)));
        r.heartbeat("s-post", "n-q", None, Some(messbereit(Some(fenster(0, 480000, 0)), 10.0)));
        let paare = paare_auswerten(&r.sensoren_snapshot(jetzt_ms()));
        assert_eq!(paare[0].timing, "wahrscheinlich");
        assert!(paare[0].grund.contains("Prozessen"));
    }
}
