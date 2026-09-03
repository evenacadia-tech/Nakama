//! Snapshotzustellung, Routing und Konfliktriegel.
//!
//! Der Flush sammelt seine Ziele unter dem Lock und stellt danach ohne Lock zu;
//! ein dauerhafter Konfliktriegel entscheidet fail-closed, bevor irgendetwas
//! geroutet wird.

use super::*;

impl Coordinator {
    pub(super) fn store_degradiert(&self) -> bool {
        self.store
            .as_ref()
            .is_some_and(|store| store.sicht().degradiert)
    }

    pub fn routing_bereit(&self) -> bool {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .routing_bereit
    }

    pub(super) fn routing_fail_closed(&self, _grund: &str) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        stand.routing_bereit = false;
        for link in stand.links.values_mut() {
            link.trennen = true;
        }
    }

    pub(super) fn flush_session(&self, session: &SessionKey, verursacher_link: Option<&str>) {
        let shard = self.session_flush_shard(session);
        let _flush_guard = self.session_flush_schloesser
            [shard % self.session_flush_schloesser.len()]
        .lock()
        .unwrap_or_else(|e| e.into_inner());

        let (payload, ziele) = {
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
            if !stand.dirty_sessions.remove(session) {
                return;
            }
            let payload = self.snapshot_locked(&stand, session);
            let ziele = stand
                .subscriptions
                .iter()
                .filter(|(link_id, sub)| {
                    sub.session_epoch == session.session_epoch
                        && sub.adresse.project_binding_id == session.project_binding_id
                        && stand.links.get(*link_id).is_some_and(|link| {
                            !link.trennen
                                && stand.routing_bereit
                                && !stand
                                    .conflict_guards
                                    .contains_key(&effektive_adresse(&link.adresse))
                                && self.alias_register.session_push_erlaubt(
                                    &link.alias_adressraum,
                                    &link.alias_besitzer,
                                    &link.adresse.instance_id,
                                )
                        })
                })
                .map(|(link_id, sub)| {
                    (
                        link_id.clone(),
                        SnapshotZiel {
                            project_binding_id: session.project_binding_id.clone(),
                            session_epoch: session.session_epoch.clone(),
                            instance_id: sub.adresse.instance_id.clone(),
                            object_key: "session_snapshot".into(),
                        },
                    )
                })
                .collect::<Vec<_>>();
            (payload, ziele)
        };

        let test_haken = self
            .flush_test_haken
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .take();
        if let Some(haken) = test_haken {
            haken.erreichen();
        }

        let mut event_ord = None;
        if let Some(store) = &self.store {
            let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
            let mut event = StoreEvent::session_snapshot(
                &session.project_binding_id,
                &session.session_epoch,
                &self.broker_epoch,
                sequence.min(i64::MAX as u64) as i64,
                payload.clone(),
            );
            event.schema_major = STORE_SCHEMA_MAJOR;
            event.snapshot_ziele = ziele.iter().map(|(_, ziel)| ziel.clone()).collect();
            match store.append(vec![event]) {
                Ok(ausgaenge) => event_ord = ausgaenge.first().map(|a| a.event_ord()),
                Err(_) => {
                    let mut stand = self.stand.lock().expect("Coordinator vergiftet");
                    stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
                    if let Some(link_id) = verursacher_link {
                        if let Some(link) = stand.links.get_mut(link_id) {
                            link.trennen = true;
                        }
                    }
                    return;
                }
            }
        }

        // Die Reihenfolge ist bis einschliesslich Store-/Outbox-Commit
        // serialisiert. Externe Pipe-Arbeit laeuft danach ohne dieses Schloss;
        // eine Senke darf den Coordinator reentrant beobachten, ohne dieselbe
        // Session zu deadlocken.
        drop(_flush_guard);
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        for (link_id, ziel) in ziele {
            let geschrieben = self.push_ziel_noch_gueltig(&link_id, &ziel)
                && push
                    .as_ref()
                    .is_some_and(|push| push.snapshot_schreiben(&link_id, &payload));
            if geschrieben {
                if let (Some(store), Some(ord)) = (&self.store, event_ord) {
                    let _ = store.snapshot_schuld_kompaktieren(ziel, ord);
                }
            }
        }
    }

    pub(super) fn session_flush_shard(&self, session: &SessionKey) -> usize {
        let mut shard = 0usize;
        for byte in session
            .project_binding_id
            .bytes()
            .chain(session.session_epoch.bytes())
        {
            shard = shard.wrapping_mul(16777619) ^ usize::from(byte);
        }
        shard
    }

    pub(super) fn guards_persistieren(&self, guards: Vec<ConflictGuard>) {
        for guard in guards {
            if self
                .store
                .as_ref()
                .is_some_and(|store| store.konflikt_guard_persistieren(guard).is_err())
            {
                self.routing_fail_closed("Konfliktriegel konnte nicht persistiert werden");
            }
        }
    }

    pub fn konflikt_guard_aufloesen(&self, effective_address: &str, derived_id: &str) -> bool {
        if let Some(store) = &self.store {
            if store
                .konflikt_guard_aufloesen(effective_address.into(), derived_id.into())
                .is_err()
            {
                return false;
            }
        }
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        if let Some(ids) = stand.conflict_guards.get_mut(effective_address) {
            ids.remove(derived_id);
            if ids.is_empty() {
                stand.conflict_guards.remove(effective_address);
            }
        }
        true
    }

    pub fn dispatch_fuer_link_erlaubt(&self, link_id: &str) -> bool {
        let stand = self.stand.lock().expect("Coordinator vergiftet");
        stand
            .links
            .get(link_id)
            .is_some_and(|link| self.dispatch_fuer_link_erlaubt_locked(&stand, link))
    }

    pub(super) fn dispatch_fuer_link_erlaubt_locked(&self, stand: &Stand, link: &LinkStand) -> bool {
        stand.routing_bereit
            && !link.trennen
            && !stand
                .conflict_guards
                .contains_key(&effektive_adresse(&link.adresse))
            && self.alias_register.dispatch_erlaubt(
                &link.alias_adressraum,
                &link.alias_besitzer,
                &link.adresse.instance_id,
            )
    }

    pub(super) fn alias_quarantaenisiert(&self, link_id: &str) -> bool {
        let stand = self.stand.lock().expect("Coordinator vergiftet");
        stand.links.get(link_id).is_some_and(|link| {
            !stand.routing_bereit
                || stand
                    .conflict_guards
                    .contains_key(&effektive_adresse(&link.adresse))
                || self
                    .alias_register
                    .ist_quarantaenisiert(&link.alias_adressraum, &link.alias_besitzer)
        })
    }

    pub(super) fn store_verweigert_fuer_link(&self, link_id: &str) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
        if let Some(link) = stand.links.get_mut(link_id) {
            link.trennen = true;
        }
    }
}
