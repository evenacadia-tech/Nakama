//! P0-Befehle und ihre Persistenz.
//!
//! Ein Befehl wird erst angenommen, dann persistiert, dann quittiert; das
//! append-only Log bleibt dabei reproduzierbar.

use super::*;

/// Ein Domaenenereignis, das im SELBEN Append landet wie sein Befehl.
///
/// 🔑 Nacharbeit 2 (Befund R08): `persistenz_p0` committete den Befehl, und die
/// Wirkung schrieb danach ihre eigenen Ereignisse. Starb der Broker dazwischen,
/// lag der Befehlsriegel im Log und die Wirkung nicht — der Retry bekam
/// `idempotent_wiederholt` und uebersprang sie DAUERHAFT. Befehlsriegel und
/// Domaenenereignis gehoeren in EINE Transaktion.
pub(super) struct Domaenenereignis {
    pub(super) event_type: String,
    pub(super) payload: Value,
    pub(super) ziele: Vec<SnapshotZiel>,
}

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

    /// Der bereits committete Befehl derselben `command_id`, falls es einen
    /// gibt (M-47, Befund R08).
    ///
    /// Sie steht VOR jeder fachlichen Vorpruefung: nach einer vollstaendig
    /// erfolgreichen Ausfuehrung wuerde die Fachlogik einen Retry sonst als
    /// `revision_conflict` beziehungsweise `schon_terminal` ablehnen — obwohl
    /// der Sender nur seine Antwort nicht bekommen hat.
    pub(super) fn bekannter_befehl(&self, wert: &Value) -> Option<Vec<u8>> {
        let kopf = wert.get("kopf")?;
        let command_id = kopf.get("command_id")?.as_str()?;
        let store = self.store.as_ref()?;
        let payload = store.command_event_lesen(command_id).ok()??;
        let (alt, revision, hash) = Self::persistierte_command_wirkung(&payload)?;
        if alt == *wert {
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
        }
    }

    /// Steht in diesem `command_ack` „angewandt"?
    pub(super) fn ack_ist_angewandt_p0(ack: &[u8]) -> bool {
        serde_json::from_slice::<Value>(ack)
            .ok()
            .and_then(|v| {
                v.get("ergebnis")
                    .and_then(Value::as_str)
                    .map(|e| e == "angewandt")
            })
            .unwrap_or(false)
    }

    pub(super) fn persistenz_p0(&self, link_id: &str, wert: &Value) -> Option<Vec<u8>> {
        self.persistenz_p0_mit_domaene(link_id, wert, Vec::new())
    }

    pub(super) fn persistenz_p0_mit_domaene(
        &self,
        link_id: &str,
        wert: &Value,
        domaene: Vec<Domaenenereignis>,
    ) -> Option<Vec<u8>> {
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
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
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
            // H-10: das Ziel wird ueber die LINKIDENTITAET des Zielclients
            // aufgeloest, die Adresse danach verifiziert. Bis NAK-121 durchsuchte
            // diese Stelle die Linkmap und nahm die ERSTE Adressuebereinstimmung -
            // bei zwei lebenden Links derselben Adresse haengt das an der
            // HashMap-Reihenfolge, und damit erzeugte gleiches Clientverhalten
            // verschiedenen persistierten Zustand. Genau daran haengt die
            // Reproduzierbarkeit des append-only Logs.
            } else if let Some((ziel_link_id, ziel_link)) = stand
                .clients
                .get(&ClientKey::aus_adresse(&ziel))
                .and_then(|client| client.current_link.as_deref())
                .and_then(|link_id| stand.links.get_key_value(link_id))
                .filter(|(_, link)| {
                    link.adresse == ziel
                        && link.client_key.session() == sender_link.client_key.session()
                        && !link.trennen
                })
            {
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
                                            && !stand.guard_gesetzt(&effektive_adresse(&link.adresse))
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
        // 🔑 Nacharbeit 2 (Befund R08): Befehl UND Wirkung in EINEM Append.
        //
        // Der Writer zieht bis zu `COMMIT_BATCH_MAX` Ereignisse in EINE
        // Transaktion; ein Befehl mit seinen Domaenenereignissen liegt weit
        // darunter und wird deshalb als Ganzes committet oder gar nicht. Ein
        // Absturz kann damit nicht mehr den Befehlsriegel ohne seine Wirkung
        // hinterlassen — genau der Zustand, in dem der Retry
        // `idempotent_wiederholt` bekam und die Wirkung fuer immer ausblieb.
        let mut ereignisse = vec![event];
        for d in domaene {
            let Ok(payload_jcs) = serde_json_canonicalizer::to_vec(&d.payload) else {
                return None;
            };
            let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
            let mut e = StoreEvent::session_snapshot(
                &ziel.project_binding_id,
                &ziel.session_epoch,
                &self.broker_epoch,
                sequence.min(i64::MAX as u64) as i64,
                payload_jcs,
            );
            e.event_type = d.event_type;
            e.snapshot_ziele = d.ziele;
            ereignisse.push(e);
        }
        match store.append(ereignisse) {
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
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
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
                    .unwrap_or_else(|e| e.into_inner())
                    .links
                    .get(link_id)
                    .is_some_and(|link| link.wire_adresse == adresse)
                {
                    return None;
                }
                match wert
                    .get("intervention_state_unknown")
                    .and_then(Value::as_bool)
                {
                    Some(true) => {
                        // M-39/M-62: der gemeldete Ueberlauf trifft die Sitzung
                        // dieses Links, nicht den ganzen Broker.
                        self.intervention_overflow_fuer_link(link_id);
                    }
                    // 🔑 Nacharbeit 2 (Befund R01, M-61): DER Produktaufrufer
                    // von `resync_bestaetigen`.
                    //
                    // Er hatte ausserhalb der Tests keinen: das sticky Unknown
                    // wurde gesetzt und nie wieder geloest. Der Riegel ist eng —
                    // NUR der ERSTE Heartbeat eines Links, der noch keine
                    // Ereignissequenz gemeldet hat, gilt als bestaetigter
                    // Neuaufbau. Ein spaeterer Heartbeat mit `false` ist die
                    // normale Meldung, und die loescht Unknown NIE (§34.2).
                    //
                    // Die Gegenseite ist `EqCopilotProcessor::v3ControlLink`:
                    // sie erklaert Neutralitaet nur, wenn der Ring leer und
                    // kein Marker hoerbar ist.
                    Some(false) if self.link_ohne_ereignissequenz(link_id) => {
                        let _ = self.resync_bestaetigen(link_id, 0);
                    }
                    _ => {}
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
                // 🔑 Nacharbeit 2 (Befund R22, M-59): `art` und
                // `experiment_id` reisen DURCH.
                //
                // Der echte Dispatch rief bis hierher `intervention_begin`,
                // und der schrieb JEDE Intervention als `hoermarkierung` ohne
                // Experimentbezug fest. Ein schema-gueltiges
                // `art=experiment`-Intervall konnte damit nie seinem Versuch
                // zugeordnet und von dessen Terminal geschlossen werden — die
                // Sperre auf starker Evidenz blieb nach jedem Resultat offen.
                let art = wert.get("art").and_then(Value::as_str).unwrap_or("hoermarkierung");
                let experiment_id = wert.get("experiment_id").and_then(Value::as_str);
                // Die Kante, die das Schema nicht ausdruecken kann (geschlossene
                // Schluesselwortmenge, kein Feldvergleich): `art=experiment`
                // OHNE `experiment_id` ist ein Intervall, das kein Terminal je
                // schliessen koennte. Es als Hoermarkierung zu fuehren waere die
                // stille Umdeutung, die §34.2 ausschliesst — fail-closed.
                if art == "experiment" && experiment_id.is_none() {
                    self.intervention_overflow_fuer_link(link_id);
                    return None;
                }
                self.intervention_begin_mit_art(
                    link_id,
                    &adresse,
                    wert.get("intervention_id")?.as_str()?,
                    wert.get("event_sequence")?.as_u64()?,
                    art,
                    experiment_id,
                    // 🔑 Nacharbeit 2 (Befund R25, M-52): der Beginn wird
                    // GESPEICHERT. Ohne ihn invalidierte das Ende pauschal ab
                    // `i64::MIN / 2` und schloss auch saemtliche aeltere,
                    // nicht ueberlappende Evidenz aus.
                    wert.get("project_sample_start").and_then(Value::as_i64),
                );
                None
            }
            "audible_intervention_end" => {
                let adresse: Adresse = serde_json::from_value(wert["adresse"].clone()).ok()?;
                let tail = wert.get("tail_samples")?.as_u64()?;
                let (angenommen, beginn) = self.intervention_end_mit_beginn(
                    link_id,
                    &adresse,
                    wert.get("intervention_id")?.as_str()?,
                    wert.get("event_sequence")?.as_u64()?,
                    tail,
                );
                // 🔑 M-52, Befund B24: ein hoerbarer Eingriff NIMMT die Evidenz
                // seines Bereichs ZURUECK. Ihn nur fuer die Zukunft zu sperren
                // liesse die waehrend des Eingriffs angenommenen Belege stehen
                // — und die sehen aus wie jede andere Messung.
                //
                // Der Bereich endet erst NACH dem Nachlauf: der Filterhall des
                // Markers laeuft in die folgende Messung hinein (§34.2).
                if angenommen {
                    let session = ClientKey::aus_adresse(&adresse).session();
                    let ende = wert.get("project_sample_end").and_then(Value::as_i64);
                    match (beginn, ende) {
                        // 🔑 Nacharbeit 2 (Befund R25, M-52): der Bereich ist
                        // EXAKT Begin bis Ende plus Nachlauf. Die Runde 1
                        // begann bei `i64::MIN / 2` und nahm damit auch
                        // Evidenz zurueck, die der Marker nie beruehrt hat.
                        (Some(von), Some(bis)) => {
                            self.invalidierung_wegen_intervention(
                                &session,
                                von,
                                bis.saturating_add(tail.min(i64::MAX as u64) as i64),
                            );
                        }
                        // Fehlt eine der beiden Grenzen, gibt es keinen
                        // Bereich. Die Runde 1 invalidierte dann GAR NICHTS —
                        // fail-OPEN, obwohl §32.3 an dieser Stelle die ganze
                        // Sitzung verlangt: der Marker hat gefaerbt, und
                        // niemand weiss wo.
                        _ => {
                            self.invalidierung_wegen_intervention_ganze_sitzung(&session);
                        }
                    }
                }
                None
            }
            "session_command" => self.session_command(link_id, &wert),
            "preview_begin" | "preview_renew" | "preview_end" => {
                let ack = self.persistenz_p0(link_id, &wert)?;
                // 🔑 Nacharbeit 2 (Befund R24, M-52): eine PREVIEW nimmt die
                // Evidenz ihrer Sitzung zurueck.
                //
                // Die drei Familien liefen bis hierher NUR durch
                // `persistenz_p0`; der Invalidierungszaehler blieb unveraendert,
                // und die waehrend der Vorschau gemessene Evidenz sah aus wie
                // jede andere. M-52 zaehlt die Preview ausdruecklich als
                // Ausloeser auf.
                //
                // Der Umfang ist die GANZE Sitzung, und das ist keine
                // Bequemlichkeit: die drei Nachrichten tragen keinen Bereich in
                // Projektzeit, und ein geratenes Fenster waere schlimmer als ein
                // zu grosses (§32.3). Der Grund ist `intervention` — eine
                // Vorschau IST ein hoerbarer Eingriff.
                if Self::ack_ist_angewandt_p0(&ack) {
                    if let Ok(ziel) = serde_json::from_value::<Adresse>(
                        wert.pointer("/kopf/ziel").cloned().unwrap_or(Value::Null),
                    ) {
                        let session = ClientKey::aus_adresse(&ziel).session();
                        self.invalidierung_wegen_preview(&session);
                    }
                }
                Some(ack)
            }
            // 🔑 Nacharbeit 1 (Befund B18): die drei Experimentfamilien fielen
            // vorher in `_ => None`. Schema-gueltige Produktnachrichten
            // bewirkten damit NICHTS, und M-40/M-47/M-49 existierten nur in
            // ihren eigenen Tests.
            // 🔑 Nacharbeit 2 (Befunde R16/R21): `experiment_candidate` ist der
            // Schritt ZWISCHEN Begin und Ergebnis. Er erfasst den Kandidaten
            // und bindet die Blindreihenfolge append-only, BEVOR der User
            // hoert. Ohne ihn konnte ein Ergebnis ohne einen einzigen
            // Kandidaten terminieren, und die Reihenfolge wurde erst zusammen
            // mit dem Hoerurteil gebunden — der Sender konnte sie also nach
            // dem Hoeren waehlen.
            "experiment_begin"
            | "experiment_candidate"
            | "experiment_abort"
            | "experiment_manual_result" => self.experiment_p0(link_id, &wert),
            _ => None,
        }
    }
}
