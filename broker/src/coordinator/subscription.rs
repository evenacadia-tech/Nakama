//! Subscription und Messframe-Push.
//!
//! Ein Abonnent bekommt seinen absoluten Snapshot beim Anmelden und danach die
//! laufenden Messframes; die Zielliste entsteht unter dem Lock, der Push nicht.

use super::*;

impl Coordinator {
    pub(super) fn subscription_abweisen(stand: &mut Stand, grund: &str) {
        stand.subscription_abweisungen = stand.subscription_abweisungen.saturating_add(1);
        stand.letzter_subscription_grund = grund.to_owned();
    }

    pub fn subscribe_json(&self, link_id: &str, payload: &[u8]) -> bool {
        self.subscribe_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
    }

    pub(super) fn subscribe_json_mit_minor(&self, link_id: &str, payload: &[u8], schema_minor: u8) -> bool {
        let Some(wert) = v3_nachricht_lesen_mit_minor(payload, "subscribe_session", schema_minor)
        else {
            return false;
        };
        let Ok(adresse) = serde_json::from_value::<Adresse>(wert["adresse"].clone()) else {
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
            Self::subscription_abweisen(&mut stand, "subscribe: Adresse ungueltig");
            return false;
        };
        let Some(session_epoch) = wert.get("session_epoch").and_then(Value::as_str) else {
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
            Self::subscription_abweisen(&mut stand, "subscribe: session_epoch fehlt");
            return false;
        };

        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        let Some(link) = stand.links.get(link_id) else {
            Self::subscription_abweisen(&mut stand, "subscribe: Control-Link unbekannt");
            return false;
        };
        if !self.alias_register.session_push_erlaubt(
            &link.alias_adressraum,
            &link.alias_besitzer,
            &link.adresse.instance_id,
        ) {
            Self::subscription_abweisen(
                &mut stand,
                "subscribe: Alias unbekannt oder quarantinisiert",
            );
            return false;
        }
        if link.wire_adresse != adresse {
            Self::subscription_abweisen(&mut stand, "subscribe: fremde effektive Adresse");
            return false;
        }
        if session_epoch != link.wire_adresse.session_epoch
            || session_epoch != adresse.session_epoch
        {
            Self::subscription_abweisen(&mut stand, "subscribe: fremde Session");
            return false;
        }
        let session = link.client_key.session();
        let effektive_adresse = link.adresse.clone();
        stand.subscriptions.insert(
            link_id.to_owned(),
            Subscription {
                adresse: effektive_adresse,
                session_epoch: session.session_epoch.clone(),
            },
        );
        drop(stand);
        self.resubscribe_snapshot_push(&session, link_id);
        true
    }

    pub(super) fn resubscribe_snapshot_push(&self, session: &SessionKey, link_id: &str) {
        let (live_payload, ziel) = {
            let stand = self.stand.lock().expect("Coordinator vergiftet");
            let Some(sub) = stand.subscriptions.get(link_id) else {
                return;
            };
            let Some(link) = stand.links.get(link_id) else {
                return;
            };
            if link.trennen
                || (!stand.routing_bereit && !self.store_degradiert())
                || stand
                    .conflict_guards
                    .contains_key(&effektive_adresse(&link.adresse))
                || !self.alias_register.session_push_erlaubt(
                    &link.alias_adressraum,
                    &link.alias_besitzer,
                    &link.adresse.instance_id,
                )
            {
                return;
            }
            (
                self.snapshot_locked(&stand, session),
                SnapshotZiel {
                    project_binding_id: session.project_binding_id.clone(),
                    session_epoch: session.session_epoch.clone(),
                    instance_id: sub.adresse.instance_id.clone(),
                    object_key: "session_snapshot".into(),
                },
            )
        };

        // Nach einem Brokerneustart ist der fluechtige Graph vor dem ersten
        // Report naturgemaess leer. Der Re-Subscribe darf diesen Zustand
        // weder als neues Event persistieren noch ueber die bereits
        // committed Projektion schreiben: der letzte Projektionsschnitt ist
        // genau der haltbare absolute Resync-Stand aus L-10/K-04/K-07.
        let projektion = if self.store_degradiert() {
            None
        } else {
            match &self.store {
                Some(store) => match store
                    .session_state_lesen(&session.project_binding_id, &session.session_epoch)
                {
                    Ok(projektion) => projektion,
                    Err(_) => {
                        self.routing_fail_closed("Sessionprojektion konnte nicht gelesen werden");
                        return;
                    }
                },
                None => None,
            }
        };
        let (gedeckt_bis, payload) = match projektion {
            Some((ord, gespeichert)) => {
                let Some(wert) = v3_nachricht_lesen(&gespeichert, "session_snapshot") else {
                    self.routing_fail_closed("Sessionprojektion verletzt v3-Vertrag");
                    return;
                };
                // Fuehrung, Mitglieder und broker_epoch gehoeren zum Lauf.
                // Nur sie werden aus dem aktuellen Graphen eingesetzt. Der
                // uebrige absolute Projektionsschnitt bleibt die haltbare
                // Wirkung fuer K-04/K-06/K-07.
                if wert.get("broker_epoch").and_then(Value::as_str)
                    != Some(self.broker_epoch.as_str())
                {
                    let Some(aktualisiert) =
                        projektion_mit_aktuellem_lauf(&gespeichert, &live_payload)
                    else {
                        self.routing_fail_closed("Sessionprojektion konnte nicht auf den aktuellen Lauf abgebildet werden");
                        return;
                    };
                    (Some(ord), aktualisiert)
                } else {
                    (Some(ord), gespeichert)
                }
            }
            None => (None, live_payload),
        };
        if v3_nachricht_lesen(&payload, "session_snapshot").is_none() {
            // Eine beschaedigte Projektion darf nie als scheinbar gueltiger
            // Wirezustand austreten. Das ist Storedegradation, nicht ein
            // Anlass, einen leeren oder erfundenen Snapshot zu senden.
            self.routing_fail_closed("Sessionprojektion verletzt v3-Vertrag");
            return;
        }
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        let geschrieben = self.push_ziel_noch_gueltig(link_id, &ziel)
            && push
                .as_ref()
                .is_some_and(|push| push.snapshot_schreiben(link_id, &payload));
        if geschrieben {
            if let (Some(store), Some(ord)) = (&self.store, gedeckt_bis) {
                let _ = store.snapshot_schuld_kompaktieren(ziel, ord);
            }
            self.messframes_an_subscriber_push(link_id);
        }
    }

    /// Liefert dem neu verbundenen/gekoppelten Subscriber die aktuell
    /// gehaltenen P2-Staende seiner Session. Snapshot und Liveframes bleiben
    /// getrennte Familien; beide sind absolut, es gibt keinen Event-Replay.
    pub(super) fn messframes_an_subscriber_push(&self, link_id: &str) {
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        let Some(push) = push else { return };
        // H-03: sammeln unter dem Lock, pushen ohne. Die Frameliste war schon
        // vorher eine Kopie; der Lock stand nur noch waehrend der fremden
        // Senkenarbeit, ohne dafuer gebraucht zu werden.
        let frames = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(sub) = stand.subscriptions.get(link_id) else {
                return;
            };
            let Some(ziel_link) = stand.links.get(link_id) else {
                return;
            };
            if ziel_link.trennen
                || !stand.routing_bereit
                || ziel_link.adresse != sub.adresse
                || !self.alias_register.session_push_erlaubt(
                    &ziel_link.alias_adressraum,
                    &ziel_link.alias_besitzer,
                    &ziel_link.adresse.instance_id,
                )
            {
                return;
            }
            let session = ziel_link.client_key.session();
            let mut frames = stand
                .messframes
                .iter()
                .filter(|(key, _)| key.session() == session)
                .map(|(key, frame)| (key.instance_id.clone(), frame.payload.clone()))
                .collect::<Vec<_>>();
            frames.sort_by(|a, b| a.0.cmp(&b.0));
            frames
        };
        for (instance_id, payload) in frames {
            let _ = push.messframe_schreiben(link_id, &instance_id, &payload);
        }
    }

    /// Zweite Zielpruefung unmittelbar vor der externen Pipe-Arbeit. Die
    /// erste Ermittlung ist nur ein Kandidat: Eviction, Disconnect,
    /// Nonce-Verdraengung oder Aliasquarantaene koennen ihn danach entziehen.
    pub(super) fn push_ziel_noch_gueltig(&self, link_id: &str, ziel: &SnapshotZiel) -> bool {
        let stand = self.stand.lock().expect("Coordinator vergiftet");
        let Some(link) = stand.links.get(link_id) else {
            return false;
        };
        let Some(sub) = stand.subscriptions.get(link_id) else {
            return false;
        };
        !link.trennen
            && (stand.routing_bereit || self.store_degradiert())
            && sub.adresse == link.adresse
            && sub.adresse.project_binding_id == ziel.project_binding_id
            && sub.session_epoch == ziel.session_epoch
            && sub.adresse.instance_id == ziel.instance_id
            && stand
                .clients
                .get(&link.client_key)
                .is_some_and(|client| client.current_link.as_deref() == Some(link_id))
            && !stand
                .conflict_guards
                .contains_key(&effektive_adresse(&link.adresse))
            && self.alias_register.session_push_erlaubt(
                &link.alias_adressraum,
                &link.alias_besitzer,
                &link.adresse.instance_id,
            )
    }

    /// Liefert die Linkziele eines Pushs unter demselben Lock wie der
    /// Cleanup. Nach `control_ende` kann deshalb kein alter Link mehr in einem
    /// spaeter gestarteten Push auftauchen.
    pub fn session_push_ziele(&self, session_epoch: &str, adresse: &Adresse) -> Vec<String> {
        let stand = self.stand.lock().expect("Coordinator vergiftet");
        stand
            .subscriptions
            .iter()
            .filter(|(link_id, sub)| {
                sub.session_epoch == session_epoch
                    && &sub.adresse == adresse
                    && stand.links.contains_key(*link_id)
                    && stand.links.get(*link_id).is_some_and(|link| {
                        self.alias_register.session_push_erlaubt(
                            &link.alias_adressraum,
                            &link.alias_besitzer,
                            &link.adresse.instance_id,
                        )
                    })
            })
            .map(|(link_id, _)| link_id.clone())
            .collect()
    }

    pub(super) fn messframe_abweisen_locked(
        stand: &mut Stand,
        link_id: &str,
        grund: P2RejectGrund,
    ) -> Option<SessionKey> {
        let Some(key) = Self::aktueller_telemetrie_client_locked(stand, link_id) else {
            return None;
        };
        let session = key.session();
        let fehler = stand.messfehler.entry(key).or_default();
        fehler.anzahl = fehler.anzahl.saturating_add(1).min(JSON_SAFE_INTEGER_MAX);
        fehler.aktuell = true;
        fehler.letzter_grund = Some(grund);
        stand.dirty_sessions.insert(session.clone());
        Some(session)
    }

    pub(super) fn messframe_abweisen(&self, link_id: &str, grund: P2RejectGrund) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        let _ = Self::messframe_abweisen_locked(&mut stand, link_id, grund);
    }

    pub(super) fn subscription_entfernen_locked(stand: &mut Stand, link_id: &str) {
        if stand.subscriptions.remove(link_id).is_some() {
            stand.subscription_cleanups = stand.subscription_cleanups.saturating_add(1);
        }
    }
}
