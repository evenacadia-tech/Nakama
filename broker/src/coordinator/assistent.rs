//! Empfaenger und **versionierter Spiegel** des `AssistantStep`
//! (SONDE-014 E-11, Regel R3 der Matrixnacharbeit 1).
//!
//! ── WARUM ES DIESE DATEI GIBT ─────────────────────────────────────────────
//!
//! Die Matrixpruefung 1 hat gefunden, dass der von Entwurf §33.5 (Zeile 2147)
//! zugesagte **versionierte SQLite-Spiegel des aktuellen `AssistantStep`**
//! keinen Produktpfad hatte: `state_report` ist `additionalProperties: false`
//! mit sieben Feldern und traegt keinen Schritt, `intent_update` transportiert
//! nur den Intent, und fuer keine Erweiterung der Fassung 3 war der Schritt
//! als Nutzlast festgelegt. Der Entscheid E-11 schliesst die Luecke mit der
//! Schwesterfamilie `assistant_step_update` und diesem Modul.
//!
//! ── DER SPIEGEL IST NIE AUTORITATIV ───────────────────────────────────────
//!
//! Die Zustandsmaschine des Assistenten laeuft im **Main** (E-08), persistent
//! im `MainProjectState`. Hier liegt ein Spiegel fuer **Suche und
//! Crashdiagnose** (§33.5) — und bei Widerspruch gewinnt der neuere
//! Main-State. Ein alter Datenbankeintrag setzt NIE einen neueren
//! Plugin-State zurueck (M-59, M-76, M-89).
//!
//! ── WARUM KEINE STORE-MIGRATION 2 ─────────────────────────────────────────
//!
//! Migration 1 legt `event_log` als **append-only** Tabelle mit `event_ord`,
//! `sequence`, `schema_major`/`schema_minor` und einem UPDATE-/DELETE-Trigger
//! an. Das IST ein versionierter Spiegel: die Reihenfolge steht im
//! `event_ord`, die Fassung im Envelope, die Revision im Payload, und der
//! Index `event_log_session_ord` macht ihn je Sitzung durchsuchbar. Eine
//! Domaenentabelle `assistant_steps` haette eine Migration 2 verlangt und
//! damit die Pruefsumme von `MIGRATION_1_SQL` beruehrt — fuer eine Zusage,
//! die der bestehende Ein-Writer-Pfad bereits einloest.

use super::*;

/// Die acht Zustaende aus §46.1. `preview` bleibt in der Menge: der Zustand
/// existiert, P5 fuehrt nur keine Kante dorthin, und ein gespeicherter
/// Schritt mit `preview` ist in P5 ein **Lesefehler**, kein stiller Sprung
/// (E-07, M-55). Ihn hier wegzulassen hiesse, ihn in P6 neu erfinden zu
/// muessen.
pub(super) const SCHRITTE: [&str; 8] = [
    "coverage",
    "finding",
    "evidence",
    "listen",
    "proposal",
    "preview",
    "remeasure",
    "verdict",
];

/// Der Schritt, wie der Broker ihn spiegelt.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct AssistentSpiegel {
    pub step_id: String,
    pub schritt: String,
    pub revision: i64,
    /// `false` heisst terminal — abgeschlossen ODER abgebrochen. Verwerfen ist
    /// ein terminales Ereignis, kein Loeschen der Historie (§46.1).
    pub offen: bool,
    pub finding_id: Option<String>,
    pub proposal_id: Option<String>,
    pub experiment_id: Option<String>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SchrittAbweisung {
    Vertrag,
    FremdeAdresse,
    KeinLink,
    AeltereRevision,
    StoreVerweigert,
}

impl Coordinator {
    /// Nimmt ein `assistant_step_update` an, spiegelt es und persistiert es
    /// ueber den bestehenden Ein-Writer-Pfad.
    ///
    /// Reihenfolge, und sie ist die Zusage: Vertrag → Adresse → Link →
    /// Revisionsordnung → fluechtiger Spiegel → Store. Der Store kommt
    /// ZULETZT, weil ein abgewiesener Schritt nichts hinterlassen darf; und
    /// er kommt ueberhaupt, weil ein Spiegel ohne Persistenz keine
    /// Crashdiagnose traegt.
    pub(super) fn assistant_step_update_json_mit_minor(
        &self,
        link_id: &str,
        payload: &[u8],
        schema_minor: u8,
    ) -> Result<(), SchrittAbweisung> {
        let Some(wert) =
            v3_nachricht_lesen_mit_minor(payload, "assistant_step_update", schema_minor)
        else {
            return Err(SchrittAbweisung::Vertrag);
        };
        let Ok(adresse) = serde_json::from_value::<Adresse>(wert["adresse"].clone()) else {
            return Err(SchrittAbweisung::Vertrag);
        };

        let schritt = wert["schritt"].as_str().unwrap_or_default().to_string();
        if !SCHRITTE.contains(&schritt.as_str()) {
            return Err(SchrittAbweisung::Vertrag);
        }
        let revision = wert["revision"].as_i64().unwrap_or(0);
        if revision < 1 {
            return Err(SchrittAbweisung::Vertrag);
        }
        let Some(step_id) = wert["step_id"].as_str().map(str::to_owned) else {
            return Err(SchrittAbweisung::Vertrag);
        };
        let bezug = |name: &str| wert.get(name).and_then(Value::as_str).map(str::to_owned);
        let neu = AssistentSpiegel {
            step_id,
            schritt,
            revision,
            offen: wert["offen"].as_bool().unwrap_or(false),
            finding_id: bezug("finding_id"),
            proposal_id: bezug("proposal_id"),
            experiment_id: bezug("experiment_id"),
        };

        // ── DER EINE LOCKABSCHNITT ──────────────────────────────────────
        let session = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(link) = stand.links.get(link_id).cloned() else {
                return Err(SchrittAbweisung::KeinLink);
            };
            if link.wire_adresse != adresse {
                return Err(SchrittAbweisung::FremdeAdresse);
            }
            let session = link.client_key.session();
            // M-88: die Revisionsordnung ist TOTAL. Eine aeltere Revision
            // ueberschreibt nach der Koaleszierung nie eine juengere.
            if let Some(alt) = stand.assistent.get(&session) {
                if revision < alt.revision {
                    return Err(SchrittAbweisung::AeltereRevision);
                }
            }
            stand.assistent.insert(session.clone(), neu.clone());
            stand.assistent_updates = stand.assistent_updates.saturating_add(1);
            session
        };

        if !self.schritt_spiegeln(&session, &neu) {
            return Err(SchrittAbweisung::StoreVerweigert);
        }
        Ok(())
    }

    /// Der versionierte Spiegel im append-only `event_log` (§33.5).
    ///
    /// Kein neuer Writer und keine neue Tabelle: derselbe `store.append`, den
    /// Evidenz, Passage und Experiment nehmen. `event_type` ist
    /// `assistant_step` und steht ABSICHTLICH nicht in der Domaenenzuordnung
    /// von `writer.rs` — es gibt keine Projektionstabelle, weil es keinen
    /// zweiten autoritativen Ort geben darf.
    fn schritt_spiegeln(&self, session: &SessionKey, schritt: &AssistentSpiegel) -> bool {
        let Some(store) = self.store.as_ref() else {
            return true;
        };
        let payload = serde_json::json!({
            "step_id": schritt.step_id,
            "schritt": schritt.schritt,
            "revision": schritt.revision,
            "offen": schritt.offen,
            "finding_id": schritt.finding_id,
            "proposal_id": schritt.proposal_id,
            "experiment_id": schritt.experiment_id,
        });
        let Ok(payload_jcs) = serde_json_canonicalizer::to_vec(&payload) else {
            return false;
        };
        let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
        let mut event = StoreEvent::session_snapshot(
            &session.project_binding_id,
            &session.session_epoch,
            &self.broker_epoch,
            sequence.min(i64::MAX as u64) as i64,
            payload_jcs,
        );
        event.event_type = "assistant_step".into();
        if store.append(vec![event]).is_err() {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
            return false;
        }
        true
    }

    pub fn assistant_step_update_json(&self, link_id: &str, payload: &[u8]) -> bool {
        self.assistant_step_update_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
            .is_ok()
    }

    #[doc(hidden)]
    pub fn assistant_step_update_grund_fuer_test(
        &self,
        link_id: &str,
        payload: &[u8],
    ) -> Result<(), SchrittAbweisung> {
        self.assistant_step_update_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
    }

    #[doc(hidden)]
    pub fn assistant_step_update_json_mit_minor_fuer_test(
        &self,
        link_id: &str,
        payload: &[u8],
        schema_minor: u8,
    ) -> bool {
        self.assistant_step_update_json_mit_minor(link_id, payload, schema_minor)
            .is_ok()
    }

    /// Der gespiegelte Schritt einer Sitzung. `None` heisst „der Main hat
    /// keinen gemeldet" — nie „es gibt keinen".
    pub fn assistent_sicht(
        &self,
        project_binding_id: &str,
        session_epoch: &str,
    ) -> Option<AssistentSpiegel> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand
            .assistent
            .get(&SessionKey {
                project_binding_id: project_binding_id.into(),
                session_epoch: session_epoch.into(),
            })
            .cloned()
    }

    pub fn assistent_updates(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .assistent_updates
    }
}
