//! Die schmale Schnittstelle zwischen Transport und Coordinator.
//!
//! Der Transport meldet Ereignisse, der Coordinator antwortet mit Zustand.
//! Mehr Vertrag gibt es zwischen beiden nicht.

use super::*;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ControlAnmeldung {
    pub angenommen: bool,
    pub grund: Option<String>,
    pub zu_schliessende_links: Vec<String>,
}

impl ControlAnmeldung {
    pub fn angenommen() -> Self {
        Self {
            angenommen: true,
            grund: None,
            zu_schliessende_links: Vec::new(),
        }
    }

    pub fn abgewiesen(grund: impl Into<String>) -> Self {
        Self {
            angenommen: false,
            grund: Some(grund.into()),
            zu_schliessende_links: Vec::new(),
        }
    }
}

/// Wohin der Listener seine Nachrichten uebergibt.
///
/// **Senkenvertrag.** `p0`, `p1` und `p2` laufen auf dem Ingressthread der
/// Verbindung und muessen ZUEGIG zurueckkehren. Blockieren duerfen sie, aber
/// nicht unbegrenzt: beim Verbindungsschluss wartet der Broker hoechstens
/// `SENKE_FRIST` auf einen laufenden Aufruf und LOEST den Verbraucherthread
/// danach AB, statt ihn zu joinen (`V3Statistik::senke_abgeloest`). Vorher
/// wartete `stoppen()` unbegrenzt im Fremdaufruf: weder das Schliessen der
/// Queue noch `CancelIoEx` loesen einen Thread, der in fremdem Code steht
/// (T2-Befund 7 vom 2026-08-29). Ein abgeloester Thread haelt nur noch seine
/// `Arc`s und endet von selbst, sobald der Fremdaufruf zurueckkommt.
pub trait Senke: Send + Sync {
    fn control_verbunden(&self, link_id: &str, hello: &HelloControl) -> ControlAnmeldung;
    /// Synchroner, atomarer Gegenpfad fuer linkgebundenen Zustand. Der Server
    /// ruft ihn nach dem Schliessen des Ingress, aber VOR dem Entfernen der
    /// Kopplung auf. Er muss kurz bleiben und darf nicht auf fremde I/O
    /// warten: Subscription-Cleanup und Sticky-Interventionsbit sind reine
    /// Mutex-Mutation. Ein bereits laufender P0/P1-Aufruf serialisiert sich
    /// am selben Coordinator-Lock; ein spaeterer sieht den Link als schliessend.
    fn control_schliesst(&self, _link_id: &str) {}
    fn control_getrennt(&self, link_id: &str);
    fn telemetrie_gekoppelt(&self, link_id: &str);
    fn telemetrie_getrennt(&self, link_id: &str);
    /// P0. Ein `Some(payload)` wird als v3-P0-Frame zurueckgeschrieben.
    fn p0(&self, link_id: &str, payload: &[u8]) -> Option<Vec<u8>>;
    fn p1(&self, link_id: &str, payload: &[u8]);
    fn p2(&self, link_id: &str, payload: &[u8]);
    /// Versionierter Gegenpfad fuer produktiven Ingress. Die Defaults halten
    /// bestehende byteorientierte Senken kompatibel; Vertragsleser wie der
    /// Coordinator ueberschreiben sie und waehlen nach Familie plus Minor.
    fn p0_mit_minor(&self, link_id: &str, _schema_minor: u8, payload: &[u8]) -> Option<Vec<u8>> {
        self.p0(link_id, payload)
    }
    fn p1_mit_minor(&self, link_id: &str, _schema_minor: u8, payload: &[u8]) {
        self.p1(link_id, payload)
    }
    fn p2_mit_minor(&self, link_id: &str, _schema_minor: u8, payload: &[u8]) {
        self.p2(link_id, payload)
    }
    /// Eine Verbindung wurde abgewiesen oder geschlossen; `grund` ist ein
    /// kurzer, maschinennaher Text fuer Diagnose und Manifest.
    fn abgewiesen(&self, grund: &str);
}

/// Zaehlende Standardsenke: beantwortet `heartbeat` mit `heartbeat_ack` und
/// haelt genau die Zahlen, die das Lastbein braucht.
#[derive(Debug, Default)]
pub struct ZaehlSenke {
    pub control_verbindungen: AtomicU64,
    pub telemetrie_verbindungen: AtomicU64,
    pub control_getrennt: AtomicU64,
    pub telemetrie_getrennt: AtomicU64,
    pub p0: AtomicU64,
    pub p0_beantwortet: AtomicU64,
    pub p1: AtomicU64,
    pub p2: AtomicU64,
    pub abweisungen: AtomicU64,
    pub letzter_grund: Mutex<String>,
}

impl Senke for ZaehlSenke {
    fn control_verbunden(&self, _link_id: &str, _hello: &HelloControl) -> ControlAnmeldung {
        self.control_verbindungen.fetch_add(1, Ordering::SeqCst);
        ControlAnmeldung::angenommen()
    }
    fn control_getrennt(&self, _link_id: &str) {
        self.control_getrennt.fetch_add(1, Ordering::SeqCst);
    }
    fn telemetrie_gekoppelt(&self, _link_id: &str) {
        self.telemetrie_verbindungen.fetch_add(1, Ordering::SeqCst);
    }
    fn telemetrie_getrennt(&self, _link_id: &str) {
        self.telemetrie_getrennt.fetch_add(1, Ordering::SeqCst);
    }

    fn p0(&self, _link_id: &str, payload: &[u8]) -> Option<Vec<u8>> {
        self.p0.fetch_add(1, Ordering::SeqCst);
        // Nur `heartbeat` wird beantwortet — und zwar mit genau dem
        // `heartbeat_ack` aus dem v3-Schema (type, sequence,
        // duplicate_instance_id). Eine erfundene Antwortform waere eine
        // Luege ueber den Vertrag.
        let wert: serde_json::Value = serde_json::from_slice(payload).ok()?;
        if wert.get("type")?.as_str()? != "heartbeat" {
            return None;
        }
        let seq = wert.get("sequence")?.as_u64()?;
        self.p0_beantwortet.fetch_add(1, Ordering::SeqCst);
        Some(
            format!(
                "{{\"type\":\"heartbeat_ack\",\"sequence\":{seq},\"duplicate_instance_id\":false}}"
            )
            .into_bytes(),
        )
    }

    fn p1(&self, _link_id: &str, _payload: &[u8]) {
        self.p1.fetch_add(1, Ordering::SeqCst);
    }
    fn p2(&self, _link_id: &str, _payload: &[u8]) {
        self.p2.fetch_add(1, Ordering::SeqCst);
    }
    fn abgewiesen(&self, grund: &str) {
        self.abweisungen.fetch_add(1, Ordering::SeqCst);
        if let Ok(mut g) = self.letzter_grund.lock() {
            *g = grund.to_string();
        }
    }
}
