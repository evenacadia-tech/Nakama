//! Was der Coordinator nach aussen zeigt.
//!
//! Die Sichttypen sind Kopien eines konsistenten Standes; sie halten keinen
//! Lock und keine Referenz in den Sessiongraphen hinein.

use super::*;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Interventionssicht {
    pub aktive: usize,
    pub tail_samples_offen: u64,
    pub unknown: bool,
    pub starke_evidenz_erlaubt: bool,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ControlRegistrierung {
    pub angenommen: bool,
    pub grund: Option<String>,
    pub zu_schliessende_links: Vec<String>,
}

impl ControlRegistrierung {
    pub(super) fn angenommen(zu_schliessende_links: Vec<String>) -> Self {
        Self {
            angenommen: true,
            grund: None,
            zu_schliessende_links,
        }
    }

    pub(super) fn abgewiesen(grund: impl Into<String>) -> Self {
        Self {
            angenommen: false,
            grund: Some(grund.into()),
            zu_schliessende_links: Vec::new(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ClientModellSicht {
    pub adresse: Adresse,
    pub plugin_kind: String,
    pub verbunden: bool,
    pub stale: bool,
    pub letzter_kontakt_ms: u64,
    pub join_kandidat: bool,
    pub bestaetigt: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum Lautheitszustand {
    #[default]
    Missing,
    Paar,
    Collecting,
    Gated,
    Invalid,
}

#[derive(Debug, Clone, PartialEq)]
pub struct MessframeSicht {
    pub adresse: Adresse,
    pub sequence: Option<u64>,
    pub sample_count: Option<u32>,
    pub sample_rate: Option<f64>,
    pub fenster_ms: Option<f64>,
    pub alter_ms: Option<u64>,
    pub control_verbunden: bool,
    pub control_stale: bool,
    pub messung_ungueltig: bool,
    pub verworfene_frames: u64,
    pub letzter_fehler: Option<String>,
    pub lautheitszustand: Lautheitszustand,
    pub lufs_i: Option<f32>,
    pub lufs_i_unsicherheit_lu: Option<f32>,
    pub letztes_gueltiges_lufs_i: Option<f32>,
    pub letztes_gueltiges_lufs_i_unsicherheit_lu: Option<f32>,
    pub letztes_gueltiges_lufs_i_alter_ms: Option<u64>,
    pub ungueltige_lautheitspaare: u64,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SessionModellSicht {
    pub project_binding_id: String,
    pub session_epoch: String,
    pub fuehrendes_main: Option<String>,
    pub beitritt_bestaetigung_noetig: bool,
    pub clients: Vec<ClientModellSicht>,
}

impl Coordinator {
    pub fn cap_abweisungen(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .cap_abweisungen
    }

    pub fn store_verweigerungen(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .store_verweigerungen
    }

    pub fn p2_live_frames(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .p2_live_frames
    }

    pub fn messsicht(
        &self,
        project_binding_id: &str,
        session_epoch: &str,
        instance_id: &str,
    ) -> Option<MessframeSicht> {
        let jetzt = self.clock.jetzt();
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let (key, client) = stand.clients.iter().find(|(key, _)| {
            key.project_binding_id == project_binding_id
                && key.session_epoch == session_epoch
                && key.instance_id == instance_id
        })?;
        let frame = stand.messframes.get(key);
        let fehler = stand.messfehler.get(key).cloned().unwrap_or_default();
        let lautheit = stand.lautheit.get(key).cloned().unwrap_or_default();
        let (letztes_lufs_i, letzte_unsicherheit, letztes_alter) = lautheit
            .letztes_gueltiges_paar
            .map(|(lufs_i, unsicherheit, empfangen)| {
                (
                    Some(lufs_i),
                    Some(unsicherheit),
                    Some(
                        jetzt
                            .saturating_sub(empfangen)
                            .as_millis()
                            .min(u64::MAX as u128) as u64,
                    ),
                )
            })
            .unwrap_or((None, None, None));
        let (lufs_i, lufs_i_unsicherheit_lu) = if lautheit.zustand == Lautheitszustand::Paar {
            (letztes_lufs_i, letzte_unsicherheit)
        } else {
            (None, None)
        };
        Some(MessframeSicht {
            adresse: frame
                .map(|frame| frame.adresse.clone())
                .unwrap_or_else(|| client.adresse.clone()),
            sequence: frame.map(|frame| frame.sequence),
            sample_count: frame.map(|frame| frame.sample_count),
            sample_rate: frame.map(|frame| frame.sample_rate),
            fenster_ms: frame.map(|frame| (frame.sample_count as f64 / frame.sample_rate) * 1000.0),
            alter_ms: frame.map(|frame| {
                jetzt
                    .saturating_sub(frame.empfangen)
                    .as_millis()
                    .min(u64::MAX as u128) as u64
            }),
            control_verbunden: client.current_link.is_some(),
            control_stale: client.stale,
            messung_ungueltig: fehler.aktuell,
            verworfene_frames: fehler.anzahl,
            letzter_fehler: fehler.letzter_grund.map(|grund| grund.wire().to_owned()),
            lautheitszustand: lautheit.zustand,
            lufs_i,
            lufs_i_unsicherheit_lu,
            letztes_gueltiges_lufs_i: letztes_lufs_i,
            letztes_gueltiges_lufs_i_unsicherheit_lu: letzte_unsicherheit,
            letztes_gueltiges_lufs_i_alter_ms: letztes_alter,
            ungueltige_lautheitspaare: lautheit.ungueltig_anzahl,
        })
    }

    pub fn telemetrie_kopplungen(&self) -> (usize, u64) {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        (stand.telemetry_links.len(), stand.telemetry_kopplungen)
    }

    /// H-21: steht auf dieser effektiven Adresse ein dauerhafter
    /// Konfliktriegel? Genau diese Frage stellen die elf Guard-Stellen intern;
    /// ohne sie waere die Unempfindlichkeit gegen die Schreibweise der SID von
    /// aussen unbeobachtbar.
    pub fn konfliktriegel_gesetzt(&self, effective_address: &str) -> bool {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .guard_gesetzt(effective_address)
    }

    /// H-12: Groesse der Sessionmap. Ohne sie waere die Zusage
    /// „Session faellt mit dem letzten Client" von aussen unbeobachtbar - die
    /// Modellsicht zeigt Clients, nicht die Map selbst.
    pub fn session_anzahl(&self) -> usize {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .sessions
            .len()
    }

    pub fn client_anzahl(&self) -> usize {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .clients
            .values()
            .filter(|client| client.current_link.is_some())
            .count()
    }

    pub fn subscription_anzahl(&self) -> usize {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .subscriptions
            .len()
    }

    pub fn subscription_cleanups(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .subscription_cleanups
    }

    pub fn subscription_abweisungen(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .subscription_abweisungen
    }

    pub fn letzter_subscription_grund(&self) -> String {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .letzter_subscription_grund
            .clone()
    }

    pub fn modell_sicht(
        &self,
        project_binding_id: &str,
        session_epoch: &str,
    ) -> SessionModellSicht {
        let jetzt = self.clock.jetzt();
        let session = SessionKey {
            project_binding_id: project_binding_id.into(),
            session_epoch: session_epoch.into(),
        };
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let mut clients: Vec<ClientModellSicht> = stand
            .clients
            .iter()
            .filter(|(key, _)| &key.session() == &session)
            .map(|(_, client)| ClientModellSicht {
                adresse: client.adresse.clone(),
                plugin_kind: client.plugin_kind.clone(),
                verbunden: client.current_link.is_some(),
                stale: client.stale,
                letzter_kontakt_ms: jetzt
                    .saturating_sub(client.last_seen)
                    .as_millis()
                    .min(u64::MAX as u128) as u64,
                join_kandidat: client.join_kandidat,
                bestaetigt: client.bestaetigt,
            })
            .collect();
        clients.sort_by(|a, b| effektive_adresse(&a.adresse).cmp(&effektive_adresse(&b.adresse)));
        SessionModellSicht {
            project_binding_id: session.project_binding_id.clone(),
            session_epoch: session.session_epoch.clone(),
            fuehrendes_main: stand
                .sessions
                .get(&session)
                .and_then(|s| s.fuehrendes_main.clone()),
            beitritt_bestaetigung_noetig: Self::beitritt_noetig_locked(&stand, &session),
            clients,
        }
    }

    pub fn session_snapshot_json(&self, project_binding_id: &str, session_epoch: &str) -> Vec<u8> {
        let session = SessionKey {
            project_binding_id: project_binding_id.into(),
            session_epoch: session_epoch.into(),
        };
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        self.snapshot_locked(&stand, &session)
    }

    pub(super) fn snapshot_locked(&self, stand: &Stand, session: &SessionKey) -> Vec<u8> {
        let jetzt = self.clock.jetzt();
        let mut mitglieder: Vec<(String, Value)> = stand
            .clients
            .iter()
            .filter(|(key, _)| &key.session() == session)
            .map(|(key, client)| {
                let adresse = serde_json::to_value(&client.adresse)
                    .expect("Adresse ist immer JSON-serialisierbar");
                let frische = serde_json::json!({
                    "stale": client.stale,
                    "letzter_kontakt_ms": jetzt
                        .saturating_sub(client.last_seen)
                        .as_millis()
                        .min(u64::MAX as u128) as u64
                });
                let mut mitglied = serde_json::json!({
                    "adresse": adresse,
                    "plugin_kind": client.plugin_kind,
                    "frische": frische
                });
                let mitglied_objekt = mitglied
                    .as_object_mut()
                    .expect("session_mitglied wird als Objekt erzeugt");

                if !client.ausdruecklich_ungebunden {
                    if let Some(mut descriptor) = client.descriptor.clone() {
                        if let Some(objekt) = descriptor.as_object_mut() {
                            objekt.insert(
                                "adresse".into(),
                                serde_json::to_value(&client.adresse)
                                    .expect("Adresse ist immer JSON-serialisierbar"),
                            );
                            objekt.insert(
                                "plugin_kind".into(),
                                Value::String(client.plugin_kind.clone()),
                            );
                            objekt.insert("frische".into(), frische.clone());
                            mitglied_objekt.insert("probe_descriptor".into(), descriptor);
                        }
                    }
                }
                if let Some(fehler) = stand.messfehler.get(key) {
                    if let Some(grund) = fehler.letzter_grund {
                        mitglied_objekt.insert(
                            "p2_reject".into(),
                            serde_json::json!({
                                "grund": grund.wire(),
                                "zaehler": fehler.anzahl
                            }),
                        );
                    }
                }
                (effektive_adresse(&client.adresse), mitglied)
            })
            .collect();
        mitglieder.sort_by(|a, b| a.0.cmp(&b.0));
        let mut payload = serde_json::json!({
            "type": "session_snapshot",
            "session_epoch": session.session_epoch,
            "broker_epoch": self.broker_epoch,
            "fuehrendes_main": stand
                .sessions
                .get(session)
                .and_then(|s| s.fuehrendes_main.clone()),
            "beitritt_bestaetigung_noetig": Self::beitritt_noetig_locked(stand, session),
            "mitglieder": mitglieder.into_iter().map(|(_, wert)| wert).collect::<Vec<_>>()
        });
        if self.store_degradiert() {
            payload
                .as_object_mut()
                .expect("session_snapshot ist ein Objekt")
                .insert("store_degraded".into(), Value::Bool(true));
        }
        serde_json::to_vec(&payload).unwrap_or_else(|_| b"{}".to_vec())
    }
}
