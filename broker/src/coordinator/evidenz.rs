//! Empfaenger des `evidence_snapshot` (SONDE-013 M-05, Etappe B).
//!
//! Der Evidenzpfad war bis zu diesem Ticket vertraglich fertig und ohne
//! Empfaenger: an `ed9bbf7` nennt kein Rust-Modul ausser dem Schema selbst
//! den Namen. §33.2 ordnet die 221 Baender, die Verteilungspunkte, Abdeckung,
//! Konvergenz und die Ereignisse aber ausdruecklich diesem Snapshot zu.
//!
//! ## Was hier NICHT passiert
//!
//! Der Broker rechnet an dieser Stelle nichts aus. Paarauswertung,
//! Comparability und Experimentdeltas gehoeren den Etappen F und G; hier
//! entsteht nur die Annahme samt der Zusammenfassung, an der ein Verbraucher
//! spaeter ansetzt. Ein Empfaenger, der schon rechnete, waere eine zweite
//! Wahrheit neben dem Store.
//!
//! ## Der Riegel
//!
//! Vor der Annahme steht `evidence_dispatch()` — derselbe Automat, den
//! `intervention.rs` fuehrt. Gesperrt heisst GESPERRT: der Snapshot wird
//! verworfen und gezaehlt, nicht abgeschwaecht gespeichert. Ein „leiser Wert"
//! waere genau die falsche starke Aussage, die §34.2 verhindert.

use super::*;

/// Was der Broker von einem angenommenen Evidenzsnapshot behaelt.
///
/// Bewusst klein: die Wire-Wahrheit ist der Payload, nicht diese Sicht. Sie
/// traegt genau das, was ein Verbraucher braucht, um zu entscheiden, ob er
/// den Snapshot ueberhaupt ansieht — und die zwei Zahlen, die eine duenne
/// Messung als duenn ausweisen (`ereignisse_verloren`, `verteilung_fenster`).
#[derive(Debug, Clone, PartialEq, Default)]
pub struct Evidenzstand {
    pub evidence_id: String,
    pub sequence: u64,
    pub abdeckung: f64,
    pub konvergenz: f64,
    pub klasse: String,
    pub ereignisse: usize,
    pub ereignisse_verloren: u64,
    pub verteilung_fenster: u64,
}

impl Coordinator {
    /// Nimmt einen `evidence_snapshot` an oder verwirft ihn.
    ///
    /// Fail-closed in dieser Reihenfolge: Vertrag, Adresse, Link, dann der
    /// Interventionsriegel. Jede Stufe fuer sich ist ein Grund zu verwerfen;
    /// keine davon fuehrt zu einer abgeschwaechten Annahme.
    pub(super) fn evidence_snapshot_json_mit_minor(
        &self,
        link_id: &str,
        payload: &[u8],
        schema_minor: u8,
    ) -> bool {
        let Some(wert) = v3_nachricht_lesen_mit_minor(payload, "evidence_snapshot", schema_minor)
        else {
            return false;
        };
        let Ok(adresse) = serde_json::from_value::<Adresse>(wert["adresse"].clone()) else {
            return false;
        };

        let client_key = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(link) = stand.links.get(link_id).cloned() else {
                return false;
            };
            // Dieselbe Kante wie beim state_report: die Adresse IM Payload muss
            // die des sendenden Links sein. Ein Peer schreibt sich seine
            // Quelle nicht selbst zu.
            if link.wire_adresse != adresse {
                return false;
            }
            let Some(client) = stand.clients.get(&link.client_key) else {
                return false;
            };
            if client.current_link.as_deref() != Some(link_id) {
                return false;
            }
            link.client_key.clone()
        };

        // DER RIEGEL. Er entscheidet und zaehlt unter EINEM Lock
        // (`intervention.rs`), damit ein `audible_intervention_begin` nicht
        // erst nach dem Dispatch wirkt. Gesperrt heisst verworfen.
        if !self.evidence_dispatch() {
            return false;
        }

        let ereignisse = wert
            .pointer("/ereignisse/liste")
            .and_then(Value::as_array)
            .map_or(0, Vec::len);
        let verloren = wert
            .pointer("/ereignisse/verloren")
            .and_then(Value::as_u64)
            .unwrap_or(0);

        let stand_neu = Evidenzstand {
            evidence_id: wert["evidence_id"].as_str().unwrap_or_default().to_owned(),
            sequence: wert
                .pointer("/transport/sequence")
                .and_then(Value::as_u64)
                .unwrap_or(0),
            abdeckung: wert["abdeckung"].as_f64().unwrap_or(0.0),
            konvergenz: wert["konvergenz"].as_f64().unwrap_or(0.0),
            klasse: wert
                .pointer("/konfidenz/klasse")
                .and_then(Value::as_str)
                .unwrap_or_default()
                .to_owned(),
            ereignisse,
            ereignisse_verloren: verloren,
            verteilung_fenster: wert
                .pointer("/konfidenz/verteilung_fenster")
                .and_then(Value::as_u64)
                .unwrap_or(0),
        };

        {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.evidenz.insert(client_key, stand_neu);
        }
        self.heartbeat_kontakt(link_id, None)
    }

    /// Derselbe Weg mit der aktiven Fassung — das oeffentliche Gegenstueck
    /// zu `state_report_json`, damit ein Test nicht die Senke nachbauen muss.
    pub fn evidence_snapshot_json(&self, link_id: &str, payload: &[u8]) -> bool {
        self.evidence_snapshot_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
    }

    /// Derselbe Weg mit einer GEWAEHLTEN Fassung.
    ///
    /// Nur fuer Beine: sie messen damit, dass ein Leser der Fassung 1 die
    /// Fassung 2 wirklich ablehnt. Der Produktpfad waehlt die Fassung
    /// ausschliesslich aus dem Wire-Envelope (`senke.rs`).
    pub fn evidence_snapshot_json_mit_minor_fuer_test(
        &self,
        link_id: &str,
        payload: &[u8],
        schema_minor: u8,
    ) -> bool {
        self.evidence_snapshot_json_mit_minor(link_id, payload, schema_minor)
    }

    /// Der zuletzt ANGENOMMENE Snapshot einer Quelle.
    ///
    /// `None` heisst „von dieser Quelle ist keiner angekommen" — nicht
    /// „einer kam an und war leer". Ein gesperrter Snapshot hinterlaesst
    /// hier nichts; sein Urteil steht im Zaehler `evidence_gesperrt`.
    pub fn evidenz_sicht(&self, instance_id: &str) -> Option<Evidenzstand> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand
            .evidenz
            .iter()
            .find(|(key, _)| key.instance_id == instance_id)
            .map(|(_, wert)| wert.clone())
    }
}
