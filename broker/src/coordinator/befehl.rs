//! P0-Befehle und ihre Persistenz.
//!
//! Ein Befehl wird erst angenommen, dann persistiert, dann quittiert; das
//! append-only Log bleibt dabei reproduzierbar.

use super::*;

impl Coordinator {
    pub(super) fn command_ack(
        command_id: &str,
        ergebnis: &str,
        state_revision: u64,
        state_hash: Option<&str>,
        code: Option<&str>,
    ) -> Option<Vec<u8>> {
        let mut objekt = serde_json::Map::from_iter([
            ("type".into(), Value::String("command_ack".into())),
            ("command_id".into(), Value::String(command_id.into())),
            ("ergebnis".into(), Value::String(ergebnis.into())),
            ("state_revision".into(), Value::from(state_revision)),
        ]);
        if let Some(hash) = state_hash {
            objekt.insert("state_hash".into(), Value::String(hash.into()));
        }
        if let Some(code) = code {
            objekt.insert("code".into(), Value::String(code.into()));
        }
        let payload = serde_json::to_vec(&Value::Object(objekt)).ok()?;
        v3_nachricht_lesen(&payload, "command_ack")
            .is_some()
            .then_some(payload)
    }

    pub(super) fn persistierte_command_wirkung(payload: &[u8]) -> Option<(Value, u64, String)> {
        let wert: Value = serde_json::from_slice(payload).ok()?;
        if wert.get("type").and_then(Value::as_str) != Some("internal_p0_command") {
            return None;
        }
        Some((
            wert.get("command")?.clone(),
            wert.get("state_revision")?.as_u64()?,
            wert.get("state_hash")?.as_str()?.to_owned(),
        ))
    }

    pub(super) fn persistenz_p0(&self, link_id: &str, wert: &Value) -> Option<Vec<u8>> {
        let kopf = wert.get("kopf")?;
        let command_id = kopf.get("command_id")?.as_str()?;
        let base_revision = kopf.get("base_revision")?.as_u64()?;
        let ziel: Adresse = serde_json::from_value(kopf.get("ziel")?.clone()).ok()?;
        let session = ClientKey::aus_adresse(&ziel).session();
        let shard = self.session_flush_shard(&session);
        let _commit_guard = self.session_flush_schloesser
            [shard % self.session_flush_schloesser.len()]
        .lock()
        .unwrap_or_else(|e| e.into_inner());
        let Some(store) = &self.store else {
            return Self::command_ack(
                command_id,
                "abgelehnt",
                base_revision,
                None,
                Some("internal"),
            );
        };

        // Idempotenz wird VOR der heutigen Link-/Record-Lage aufgeloest: ein
        // bereits committeter Befehl bleibt auch nach Reconnect dieselbe
        // Wirkung. Eine Wiederverwendung derselben ID fuer andere Bytes ist
        // dagegen ein sichtbarer Konflikt.
        match store.command_event_lesen(command_id) {
            Ok(Some(payload)) => {
                let (alt, revision, hash) = Self::persistierte_command_wirkung(&payload)?;
                return if alt == *wert {
                    Self::command_ack(
                        command_id,
                        "idempotent_wiederholt",
                        revision,
                        Some(&hash),
                        None,
                    )
                } else {
                    Self::command_ack(
                        command_id,
                        "konflikt",
                        revision,
                        Some(&hash),
                        Some("revision_conflict"),
                    )
                };
            }
            Ok(None) => {}
            Err(_) => {
                self.store_verweigert_fuer_link(link_id);
                return None;
            }
        }

        let zielstand: Result<
            (u64, String, Value, Vec<SnapshotZiel>),
            (u64, Option<String>, &'static str, &'static str),
        > = {
            let stand = self.stand.lock().expect("Coordinator vergiftet");
            let Some(sender_link) = stand.links.get(link_id) else {
                return None;
            };
            let sender_erlaubt = stand
                .clients
                .get(&sender_link.client_key)
                .is_some_and(|client| {
                    client.plugin_kind == "main"
                        && client.bestaetigt
                        && stand
                            .sessions
                            .get(&sender_link.client_key.session())
                            .and_then(|session| session.fuehrendes_main.as_deref())
                            == Some(sender_link.client_key.instance_id.as_str())
                        && self.dispatch_fuer_link_erlaubt_locked(&stand, sender_link)
                });
            if !sender_erlaubt {
                Err((base_revision, None, "abgelehnt", "unauthorized"))
            } else if let Some((ziel_link_id, ziel_link)) = stand.links.iter().find(|(_, link)| {
                link.adresse == ziel
                    && link.client_key.session() == sender_link.client_key.session()
                    && !link.trennen
            }) {
                let Some(client) = stand.clients.get(&ziel_link.client_key) else {
                    return Self::command_ack(
                        command_id,
                        "abgelehnt",
                        base_revision,
                        None,
                        Some("unknown_target"),
                    );
                };
                if client.current_link.as_deref() != Some(ziel_link_id.as_str())
                    || client.stale
                    || !self.dispatch_fuer_link_erlaubt_locked(&stand, ziel_link)
                {
                    Err((base_revision, None, "abgelehnt", "unknown_target"))
                } else if !client.record_state_valid || client.state_revision.is_none() {
                    Err((
                        client.state_revision.unwrap_or(base_revision),
                        client.state_hash.clone(),
                        "abgelehnt",
                        "record_state_unknown",
                    ))
                } else if client.recording {
                    Err((
                        client.state_revision.unwrap_or(base_revision),
                        client.state_hash.clone(),
                        "abgelehnt",
                        "recording_active",
                    ))
                } else if client.state_hash.is_none() {
                    Err((
                        client.state_revision.unwrap_or(base_revision),
                        None,
                        "abgelehnt",
                        "record_state_unknown",
                    ))
                } else {
                    let revision = client.state_revision.unwrap_or(base_revision);
                    let hash = client.state_hash.clone().expect("oben geprueft");
                    if revision != base_revision {
                        Err((revision, Some(hash), "konflikt", "revision_conflict"))
                    } else {
                        let session_snapshot: Value =
                            serde_json::from_slice(&self.snapshot_locked(&stand, &session))
                                .expect("interner Session-Snapshot ist JSON");
                        let snapshot_ziele = stand
                            .subscriptions
                            .iter()
                            .filter(|(abo_link_id, sub)| {
                                sub.session_epoch == session.session_epoch
                                    && sub.adresse.project_binding_id == session.project_binding_id
                                    && stand.links.get(*abo_link_id).is_some_and(|link| {
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
                            .map(|(_, sub)| SnapshotZiel {
                                project_binding_id: session.project_binding_id.clone(),
                                session_epoch: session.session_epoch.clone(),
                                instance_id: sub.adresse.instance_id.clone(),
                                object_key: "session_snapshot".into(),
                            })
                            .collect();
                        Ok((revision, hash, session_snapshot, snapshot_ziele))
                    }
                }
            } else {
                Err((base_revision, None, "abgelehnt", "unknown_target"))
            }
        };
        let (revision, hash, session_snapshot, snapshot_ziele) = match zielstand {
            Ok(wirkung) => wirkung,
            Err((revision, hash, ergebnis, code)) => {
                return Self::command_ack(
                    command_id,
                    ergebnis,
                    revision,
                    hash.as_deref(),
                    Some(code),
                );
            }
        };

        let intern = serde_json::json!({
            "type": "internal_p0_command",
            "command": wert,
            "state_revision": revision,
            "state_hash": hash,
            "session_snapshot": session_snapshot,
        });
        let payload_jcs = serde_json_canonicalizer::to_vec(&intern).ok()?;
        let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
        let mut event = StoreEvent::session_snapshot(
            &ziel.project_binding_id,
            &ziel.session_epoch,
            &self.broker_epoch,
            sequence.min(i64::MAX as u64) as i64,
            payload_jcs,
        );
        event.command_id = Some(command_id.to_owned());
        event.event_type = "command".into();
        event.snapshot_ziele = snapshot_ziele;
        match store.append(vec![event]) {
            Ok(ausgaenge) => match ausgaenge.first()? {
                crate::store::AppendAusgang::Angewandt { .. } => {
                    Self::command_ack(command_id, "angewandt", revision, Some(&hash), None)
                }
                crate::store::AppendAusgang::IdempotentWiederholt { .. } => {
                    let payload = store.command_event_lesen(command_id).ok()??;
                    let (alt, revision, hash) = Self::persistierte_command_wirkung(&payload)?;
                    if alt != *wert {
                        Self::command_ack(
                            command_id,
                            "konflikt",
                            revision,
                            Some(&hash),
                            Some("revision_conflict"),
                        )
                    } else {
                        Self::command_ack(
                            command_id,
                            "idempotent_wiederholt",
                            revision,
                            Some(&hash),
                            None,
                        )
                    }
                }
            },
            Err(_) => {
                self.store_verweigert_fuer_link(link_id);
                None
            }
        }
    }

    pub(super) fn session_command(&self, link_id: &str, wert: &Value) -> Option<Vec<u8>> {
        let command_id = wert.get("command_id")?.as_str()?;
        let command = wert.get("command")?.as_str()?;
        let session_epoch = wert.get("session_epoch")?.as_str()?;
        let ziel: Adresse = serde_json::from_value(wert.get("ziel")?.clone()).ok()?;
        let kanonischer_auftrag = serde_json_canonicalizer::to_vec(wert).ok()?;

        let (session, revision, hash) = {
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
            let Some(sender_link) = stand.links.get(link_id) else {
                return None;
            };
            let sender_session = sender_link.client_key.session();
            let sender_instance = sender_link.client_key.instance_id.clone();
            let sender_erlaubt = stand
                .clients
                .get(&sender_link.client_key)
                .is_some_and(|client| {
                    client.plugin_kind == "main"
                        && client.bestaetigt
                        && stand
                            .sessions
                            .get(&sender_session)
                            .and_then(|session| session.fuehrendes_main.as_deref())
                            == Some(sender_instance.as_str())
                        && self.dispatch_fuer_link_erlaubt_locked(&stand, sender_link)
                });
            if !sender_erlaubt {
                return Self::command_ack(
                    command_id,
                    "abgelehnt",
                    0,
                    None,
                    Some("unauthorized"),
                );
            }
            if session_epoch != sender_session.session_epoch
                || ziel.project_binding_id != sender_session.project_binding_id
                || ziel.session_epoch != sender_session.session_epoch
                || ziel.logon_sid != sender_link.adresse.logon_sid
            {
                return Self::command_ack(
                    command_id,
                    "abgelehnt",
                    0,
                    None,
                    Some("unauthorized"),
                );
            }

            if let Some(alt) = stand.session_commands.get(command_id) {
                return if alt.kanonischer_auftrag == kanonischer_auftrag {
                    Self::command_ack(
                        command_id,
                        "idempotent_wiederholt",
                        alt.state_revision,
                        Some(&alt.state_hash),
                        None,
                    )
                } else {
                    Self::command_ack(
                        command_id,
                        "konflikt",
                        alt.state_revision,
                        Some(&alt.state_hash),
                        Some("revision_conflict"),
                    )
                };
            }

            let ziel_key = stand
                .clients
                .iter()
                .find(|(key, client)| {
                    &key.session() == &sender_session
                        && client.adresse == ziel
                        && matches!(client.plugin_kind.as_str(), "active_probe" | "passive_probe")
                })
                .map(|(key, _)| key.clone());
            let Some(ziel_key) = ziel_key else {
                return Self::command_ack(
                    command_id,
                    "abgelehnt",
                    0,
                    None,
                    Some("unknown_target"),
                );
            };
            match command {
                "confirm_join" => {
                    Self::beitritt_bestaetigen_locked(
                        &mut stand,
                        &sender_session,
                        &ziel_key,
                    );
                }
                "unbind_probe" => {
                    Self::beitritt_aufheben_locked(&mut stand, &sender_session, &ziel_key);
                }
                _ => return None, // Der strikte v3-Zweig haette das bereits abgelehnt.
            }

            let snapshot = self.snapshot_locked(&stand, &sender_session);
            let snapshot_wert: Value = serde_json::from_slice(&snapshot).ok()?;
            let snapshot_jcs = serde_json_canonicalizer::to_vec(&snapshot_wert).ok()?;
            let hash = format!("{:x}", Sha256::digest(&snapshot_jcs));
            let revision = self
                .event_sequence
                .fetch_add(1, Ordering::SeqCst)
                .saturating_add(1)
                .min(9_007_199_254_740_991);
            while stand.session_command_reihenfolge.len() >= SESSION_COMMAND_REGISTER_MAX {
                if let Some(alt) = stand.session_command_reihenfolge.pop_front() {
                    stand.session_commands.remove(&alt);
                }
            }
            stand.session_command_reihenfolge.push_back(command_id.to_owned());
            stand.session_commands.insert(
                command_id.to_owned(),
                SessionCommandWirkung {
                    kanonischer_auftrag,
                    state_revision: revision,
                    state_hash: hash.clone(),
                },
            );
            (sender_session, revision, hash)
        };

        // Die absolute Sicht wird vor dem ACK erzeugt und in P1 eingereiht.
        // P0 darf sie nach der bestehenden Rueckstauregel auf dem Draht
        // ueberholen; beide Nachrichten tragen deshalb einen absoluten bzw.
        // idempotenten Stand und setzen keine Zustellreihenfolge voraus.
        self.flush_session(&session, Some(link_id));
        Self::command_ack(command_id, "angewandt", revision, Some(&hash), None)
    }

    pub(super) fn p0_json(&self, link_id: &str, payload: &[u8]) -> Option<Vec<u8>> {
        self.p0_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
    }

    pub(super) fn p0_json_mit_minor(
        &self,
        link_id: &str,
        payload: &[u8],
        schema_minor: u8,
    ) -> Option<Vec<u8>> {
        let wert = v3_nachricht_lesen_beliebig_mit_minor(payload, schema_minor)?;
        match wert.get("type")?.as_str()? {
            "heartbeat" => {
                let adresse: Adresse = serde_json::from_value(wert["adresse"].clone()).ok()?;
                if !self
                    .stand
                    .lock()
                    .expect("Coordinator vergiftet")
                    .links
                    .get(link_id)
                    .is_some_and(|link| link.wire_adresse == adresse)
                {
                    return None;
                }
                if wert
                    .get("intervention_state_unknown")
                    .and_then(Value::as_bool)
                    == Some(true)
                {
                    self.intervention_overflow();
                }
                let sequence = wert.get("sequence")?.as_u64()?;
                let _ = self.heartbeat_kontakt(link_id, Some(&wert));
                let duplicate_instance_id = self.alias_quarantaenisiert(link_id);
                Some(
                    format!(
                        "{{\"type\":\"heartbeat_ack\",\"sequence\":{sequence},\"duplicate_instance_id\":{duplicate_instance_id}}}"
                    )
                    .into_bytes(),
                )
            }
            "audible_intervention_begin" => {
                let adresse: Adresse = serde_json::from_value(wert["adresse"].clone()).ok()?;
                self.intervention_begin(
                    link_id,
                    &adresse,
                    wert.get("intervention_id")?.as_str()?,
                    wert.get("event_sequence")?.as_u64()?,
                );
                None
            }
            "audible_intervention_end" => {
                let adresse: Adresse = serde_json::from_value(wert["adresse"].clone()).ok()?;
                self.intervention_end(
                    link_id,
                    &adresse,
                    wert.get("intervention_id")?.as_str()?,
                    wert.get("event_sequence")?.as_u64()?,
                    wert.get("tail_samples")?.as_u64()?,
                );
                None
            }
            "session_command" => self.session_command(link_id, &wert),
            "preview_begin" | "preview_renew" | "preview_end" => self.persistenz_p0(link_id, &wert),
            _ => None,
        }
    }
}
