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

/// **NAK-214 R4/E8: eine Vorbedingung, die NACH dem Idempotenzblock und VOR
/// dem Append ausgewertet wird.**
///
/// Die Reihenfolge ist die ganze Zusage. `persistenz_p0_intern` loest die
/// Idempotenz VOR jeder Zustandsfrage auf — „ein bereits committeter Befehl
/// bleibt auch nach Reconnect dieselbe Wirkung". Eine Vorbedingung davor
/// braeche das: ein Urteil zu Befund F wird angenommen, F wird
/// zurueckgenommen, der Client wiederholt seinen Befehl — und bekaeme
/// `abgelehnt` auf einen Befehl, der bereits angewandt IST.
pub(super) enum P0Vorbedingung<'a> {
    Keine,
    /// Die Zielsitzung muss einen Befund mit dieser `finding_id` fuehren.
    ///
    /// Der ZUSTAND des Befunds ist keine Bedingung: `stale` und `more_data`
    /// sind existente Befunde, und ein Urteil ueber einen Befund, den der
    /// User gesehen hat, ist gueltige Userarbeit.
    BefundExistiert(&'a str),
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

    /// Derselbe Weg, aber mit den Ereignisordnungen der geschriebenen Zeilen.
    ///
    /// 🔑 Nacharbeit 3 (Befund B14): eine Invalidierung, die als
    /// Domaenenereignis MIT einem Befehl committet, braucht ihre `event_ord`,
    /// um die zugestellte Outbox-Schuld zu kompaktieren. Ohne sie bliebe eine
    /// bereits ausgelieferte Ruecknahme als Schuld stehen und wuerde beim
    /// naechsten Subscribe erneut ausgespielt.
    pub(super) fn persistenz_p0_mit_domaene_und_ords(
        &self,
        link_id: &str,
        wert: &Value,
        domaene: Vec<Domaenenereignis>,
    ) -> (Option<Vec<u8>>, Vec<i64>) {
        let mut ords = Vec::new();
        let ack =
            self.persistenz_p0_intern(link_id, wert, domaene, P0Vorbedingung::Keine, &mut ords);
        (ack, ords)
    }

    /// Derselbe Weg MIT einer Vorbedingung (NAK-214 R4).
    ///
    /// Sie steht als eigener Einstieg da, damit die Signatur des bestehenden
    /// buchstaeblich unveraendert bleibt: `user_verdict_p0` ist der einzige
    /// Aufrufer, der eine Vorbedingung mitgibt.
    pub(super) fn persistenz_p0_mit_vorbedingung(
        &self,
        link_id: &str,
        wert: &Value,
        domaene: Vec<Domaenenereignis>,
        vorbedingung: P0Vorbedingung<'_>,
    ) -> Option<Vec<u8>> {
        let mut ords = Vec::new();
        self.persistenz_p0_intern(link_id, wert, domaene, vorbedingung, &mut ords)
    }

    fn persistenz_p0_intern(
        &self,
        link_id: &str,
        wert: &Value,
        domaene: Vec<Domaenenereignis>,
        vorbedingung: P0Vorbedingung<'_>,
        ords: &mut Vec<i64>,
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

        // 🔑 WN-03 (Nacharbeit 2): auch dieser Weg serialisiert `findings`.
        //
        // Der P0-Befehl baut seinen `session_snapshot` unter dem Standlock
        // und stellt ihn den Abonnenten zu. Die Haertung steht deshalb VOR
        // dem Lock - sie nimmt ihn selbst.
        self.befunde_gegen_store_haerten(&session);

        // 🔑 **NAK-214 R4 (08.09.2026): DER VORBEDINGUNGSRIEGEL.**
        //
        // Er steht HINTER dem Idempotenzblock, und die Reihenfolge ist die
        // ganze Zusage: ein bereits committeter Befehl bleibt auch nach
        // Reconnect dieselbe Wirkung. Ein Riegel DAVOR braeche genau das —
        // ein Urteil zu Befund F wird angenommen, F wird zurueckgenommen,
        // der Client wiederholt seinen Befehl, und bekaeme
        // `abgelehnt/unknown_target` auf einen Befehl, der bereits angewandt
        // ist. Zwei verschiedene Antworten auf denselben `command_id`, und
        // das persistierte Urteil laege trotzdem im Store.
        //
        // Er steht zugleich HINTER `befunde_gegen_store_haerten`: die
        // Haertung laeuft im P0-Pfad ohnehin und bleibt unveraendert, und
        // der Riegel liest danach den Bestand, aus dem der Snapshot entsteht
        // — den, den der User gesehen hat.
        if let P0Vorbedingung::BefundExistiert(finding_id) = vorbedingung {
            let existiert = {
                let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
                stand.befunde.get(&session).is_some_and(|liste| {
                    liste.iter().any(|b| b.finding_id == finding_id)
                })
            };
            if !existiert {
                // `unknown_target` und KEIN neuer Code: er bedeutet im
                // gebauten Broker bereits „das im Befehl adressierte
                // Domaenenobjekt existiert nicht" — dieselbe Antwort geben
                // die drei Experimentzweige fuer eine unbekannte
                // `experiment_id`. Ein eigener Code waere ein
                // Fassungsschritt fuer eine Unterscheidung, die der Vertrag
                // schon trifft, und der C++-Leser wiese ihn ab (NB-3).
                return Self::command_ack(
                    command_id,
                    "abgelehnt",
                    base_revision,
                    None,
                    Some("unknown_target"),
                );
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
            Ok(ausgaenge) => {
                ords.extend(ausgaenge.iter().map(|a| a.event_ord()));
                match ausgaenge.first()? {
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
                }
            }
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
        // 🔑 WN-03 (Nacharbeit 2): der letzte Weg, auf dem `findings`
        // hinausgehen. Die Sitzung des Ziels ist die des Senders - der Befehl
        // wird sonst unten als `unauthorized` abgewiesen -, und eine Sitzung
        // ohne Befunde kostet die Haertung nur einen Lockdurchgang.
        self.befunde_gegen_store_haerten(&ClientKey::aus_adresse(&ziel).session());

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

    /// **NR-10: `user_verdict` als persistenzpflichtiger P0** (M-73, E-09).
    ///
    /// Das Urteil KOALESZIERT NICHT — ein Userurteil, das ein anderes
    /// verdraengt, waere verlorene Userarbeit. Es reist deshalb als P0 und
    /// wird hier zusammen mit seinem Befehlsriegel in EINEN Append gelegt:
    /// ein Crash zwischen Befehl und Projektion liesse den Retry sonst
    /// `idempotent_wiederholt` melden und das Urteil fuer immer ausfallen
    /// (dieselbe Lehre wie Befund B14 im Preview-Pfad).
    fn user_verdict_p0(&self, link_id: &str, wert: &Value) -> Option<Vec<u8>> {
        let user_verdict_id = wert.get("user_verdict_id")?.as_str()?.to_owned();
        let finding_id = wert.get("finding_id")?.as_str()?.to_owned();
        let urteil = wert.get("urteil")?.as_str()?.to_owned();
        // Die Projektion `user_verdicts` schluesselt ueber `user_verdict_id`;
        // ohne sie weist der Writer den Event zurueck.
        let payload = serde_json::json!({
            "user_verdict_id": user_verdict_id,
            "finding_id": finding_id,
            "proposal_id": wert.get("proposal_id").and_then(Value::as_str),
            "urteil": urteil,
            // User-Wort, untrusted, nie interpretiert (§59). Es reist
            // unveraendert mit, damit ein Replay das Urteil verlustfrei
            // wiederherstellt.
            "notiz": wert.get("notiz").and_then(Value::as_str),
        });
        let domaene = vec![Domaenenereignis {
            event_type: "user_verdict".into(),
            payload,
            ziele: Vec::new(),
        }];
        // 🔑 **NAK-214 R4:** das Urteil wird nur angenommen, wenn die
        // Sitzung diesen Befund WIRKLICH fuehrt. Der ZUSTAND des Befunds
        // spielt dabei keine Rolle — `stale` und `more_data` sind existente
        // Befunde, und das Urteil des Users ueber einen Befund, den er
        // gesehen hat, ist gueltige Userarbeit. R4 nennt ausschliesslich die
        // Existenz.
        self.persistenz_p0_mit_vorbedingung(
            link_id,
            wert,
            domaene,
            P0Vorbedingung::BefundExistiert(&finding_id),
        )
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
                // 🔑 NAK-180 Nacharbeit 1 (EP-10/E4): "erster Heartbeat" ist
                // eine eigene Tatsache, keine Ableitung aus der
                // Ereignissequenz. Die Frage wird HIER gestellt - vor der
                // Auswertung des Feldes -, damit auch ein erster Heartbeat
                // ohne das Feld als erster zaehlt: Schweigen loest nichts
                // (N-32), verbraucht den Erstling aber, sonst koennte ein
                // spaeteres `false` den R1-Zweig noch ausloesen.
                let ist_erster_heartbeat = self.ersten_heartbeat_markieren(link_id);
                match wert
                    .get("intervention_state_unknown")
                    .and_then(Value::as_bool)
                {
                    Some(true) => {
                        // M-39/M-62: der gemeldete Ueberlauf trifft die Sitzung
                        // dieses Links, nicht den ganzen Broker.
                        self.intervention_overflow_fuer_link(link_id);
                        // 🔑 NAK-180 R2/E4: ein ausdrueckliches `true` im ERSTEN
                        // Heartbeat eines Links ist der nicht neutrale
                        // Neuaufbau - der Prozessor meldet, dass beim Aufbau ein
                        // Marker lief oder der Ring nicht leer war. Er darf
                        // seinen Bericht spaeter mit genau EINEM `false`
                        // abschliessen, sobald er wieder neutral ist.
                        //
                        // Ein SPAETERES `true` (Ringueberlauf im Betrieb)
                        // oeffnet nichts: nur der erste Heartbeat des Links
                        // kann den Bericht oeffnen.
                        self.neuaufbau_bericht_oeffnen(link_id, ist_erster_heartbeat);
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
                    //
                    // 🔑 Nacharbeit 1 (EP-10): der Riegel ist der ERSTE
                    // Heartbeat, nicht "noch keine Ereignissequenz gemeldet".
                    // Replay und erster Heartbeat duerfen ungeordnet kommen
                    // (§2.1); setzte das Replay die Sequenz zuerst, verlor der
                    // R1-Weg seinen einzigen Ausloeser.
                    Some(false) if ist_erster_heartbeat => {
                        let _ = self.resync_bestaetigen(link_id, 0);
                    }
                    // 🔑 NAK-180 R2/E4: der zweite, ENGERE Zweig - der
                    // Abschluss des Neuaufbau-Berichts.
                    //
                    // Der Zweig darueber bleibt Zeichen fuer Zeichen stehen; er
                    // ist der R1-Weg (bestaetigt neutraler Aufbau). Dieser hier
                    // gilt dem R2-Weg: der erste Heartbeat trug `true`, das
                    // Replay hat den wahren Zustand hergestellt, der Marker ist
                    // geendet, der Nachlauf abgelaufen - und ERST JETZT sagt der
                    // Prozessor "neutral".
                    //
                    // M-61 bleibt woertlich erhalten: fuer jeden Link ohne
                    // offenen Bericht und fuer jedes zweite `false` desselben
                    // Links loest nichts. Und §34.2 bleibt gewahrt - der Broker
                    // heilt nichts von selbst, er prueft eine ausdrueckliche
                    // Aussage gegen seinen eigenen Bestand und verwirft sie,
                    // sobald er selbst eine Luecke gesehen hat.
                    //
                    // 🔑 Nacharbeit 1 (EP-11/EP-12): Pruefung, Flag und
                    // Freigabe liegen unter EINEM Lock, und die Sequenzbasis
                    // bleibt stehen - der Pluginzaehler wird im R2-Weg
                    // ausdruecklich nicht zurueckgesetzt.
                    Some(false) if self.nachbericht_abschliessen_und_bestaetigen(link_id) => {}
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
                            // Befund B16: scheitert der Append, bleibt KEIN
                            // lokaler Ausschluss stehen; der Link wird als
                            // storeverweigert gefuehrt.
                            if self
                                .invalidierung_wegen_intervention(
                                    &session,
                                    von,
                                    bis.saturating_add(tail.min(i64::MAX as u64) as i64),
                                )
                                .is_err()
                            {
                                self.store_verweigert_fuer_link(link_id);
                            }
                        }
                        // Fehlt eine der beiden Grenzen, gibt es keinen
                        // Bereich. Die Runde 1 invalidierte dann GAR NICHTS —
                        // fail-OPEN, obwohl §32.3 an dieser Stelle die ganze
                        // Sitzung verlangt: der Marker hat gefaerbt, und
                        // niemand weiss wo.
                        _ => {
                            if self
                                .invalidierung_wegen_intervention_ganze_sitzung(&session)
                                .is_err()
                            {
                                self.store_verweigert_fuer_link(link_id);
                            }
                        }
                    }
                }
                None
            }
            "session_command" => self.session_command(link_id, &wert),
            "preview_begin" | "preview_renew" | "preview_end" => {
                // 🔑 Nacharbeit 2 (Befund R24, M-52): eine PREVIEW nimmt die
                // Evidenz ihrer Sitzung zurueck.
                //
                // Die drei Familien liefen bis dahin NUR durch
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
                //
                // 🔑 Nacharbeit 3 (Befund B14): Befehlsriegel und Ruecknahme
                // gehen in EINEN Append. Die Runde 2 committete erst den Befehl
                // und invalidierte danach; ein Crash dazwischen liess den Retry
                // `idempotent_wiederholt` melden und die Invalidierung fuer
                // immer ausfallen.
                let session = serde_json::from_value::<Adresse>(
                    wert.pointer("/kopf/ziel").cloned().unwrap_or(Value::Null),
                )
                .ok()
                .map(|ziel| ClientKey::aus_adresse(&ziel).session());
                let wirkung = session
                    .as_ref()
                    .and_then(|s| self.preview_invalidierung_vorbereiten(s));
                let domaene = wirkung
                    .as_ref()
                    .map(|w| vec![self.invalidierung_als_domaenenereignis(w)])
                    .unwrap_or_default();
                let (ack, ords) =
                    self.persistenz_p0_mit_domaene_und_ords(link_id, &wert, domaene);
                match ack {
                    Some(a) if Self::ack_ist_angewandt_p0(&a) => {
                        if let (Some(w), Some(ord)) = (wirkung, ords.last().copied()) {
                            self.invalidierung_zustellen(&w, ord);
                        }
                        // Selbstaudit der Runde 3 (Befund B18): eine Ruecknahme
                        // ist eine Evidenzaenderung — auch die, die MIT einem
                        // Befehl committet. Ohne diese Zeile bliebe das alte
                        // volle Paarurteil ausgerechnet auf dem Weg stehen, der
                        // die Evidenz gerade zurueckgenommen hat.
                        self.paare_bei_bedarf_bilden();
                        Some(a)
                    }
                    andere => {
                        // Kein angewandter Befehl, keine Ruecknahme: der
                        // fluechtige Ausschluss faellt mit ihm.
                        if let Some(w) = wirkung {
                            self.invalidierung_ruecknehmen(w);
                        }
                        andere
                    }
                }
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
            // 🔑 NR-10 (Nacharbeit 1, 07.09.2026), M-73/E-09: das USERURTEIL
            // hat einen Produktpfad.
            //
            // Die Familie fiel bis hierher in `_ => None`: der Schema-Leser
            // nahm sie an, der Coordinator kannte sie nicht, und weder
            // Persistenz noch ACK entstanden. Der gruene B10-Fall bekam sein
            // ACK vom Testserver, nicht vom Empfaenger — er belegte den
            // generischen Transport, nicht das zugesagte dauerhaft
            // wiederholbare Urteil. Die Projektion `user_verdicts`
            // (`writer.rs`:571) hatte entsprechend keinen Produzenten.
            //
            // Der Weg ist derselbe wie bei den Experimentfamilien: Append mit
            // `event_type = "user_verdict"`, Projektion ueber
            // `user_verdict_id`, ACK erst NACH dem Append, und eine
            // Wiederholung unter derselben `command_id` bleibt idempotent
            // (`persistenz_p0_intern` loest sie vor jeder Wirkung auf).
            "user_verdict" => self.user_verdict_p0(link_id, &wert),
            _ => None,
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-214 V-34 (zweiter Teilfall) — der fehlende Sitzungseintrag, von INNEN
// ═════════════════════════════════════════════════════════════════════════
//
// Der Fall liegt HIER und nicht nur in `broker/tests/`, weil die
// Unterscheidung, die V-34 verlangt, von außen strukturell unerreichbar ist:
// die einzige öffentliche Sicht auf den Bestand ist `befunde_sicht`, und ihr
// `unwrap_or_default()` (`hypothese_verdrahtung.rs`:1037) führt „kein
// Eintrag“ und „Eintrag mit leerer Liste“ zu derselben leeren Liste
// zusammen. `Stand` ist `pub(super)` (`zustand.rs`:286) und
// `Coordinator.stand` privat (`mod.rs`:193); ein Integrationstest kann
// `stand.befunde.get(&session)` deshalb nicht lesen und misst mit
// `is_empty()` auf einer Sicht beide Fälle gleich. Dasselbe Muster trägt
// seit der Matrixnacharbeit 3 V-12 (a) in `proposal_verdrahtung.rs`:526.
//
// Der Integrationstest
// `sonde014_verdrahtung.rs::user_verdict_ohne_sitzungsbefunde_wird_abgewiesen`
// bleibt und fährt denselben Fall über den vollen Produktpfad samt
// Projektionen; er trägt die Abwesenheit im Store. GEMESSEN wird die
// Unterscheidung `None` gegen `Some(vec![])` allein hier.
#[cfg(all(test, windows))]
mod tests {
    use super::*;
    use crate::store::{StoreKonfiguration, StoreWriter, STORE_DATEINAME};
    use crate::transport::bootstrap::HostAngabe;
    use crate::transport::server_v3::Senke;
    use serde_json::json;

    fn hex(n: usize) -> String {
        format!("{n:032x}")
    }

    /// ⚠️ Löscht seinen Pfad im `Drop` — der Rückgabewert muss bis zum
    /// Testende leben (dieselbe Falle wie in `sonde014_verdrahtung.rs`:911).
    struct TestOrdner(std::path::PathBuf);

    impl TestOrdner {
        fn neu(name: &str) -> Self {
            let pfad = std::env::temp_dir().join(format!(
                "nakama-befehl-{name}-{}-{}",
                std::process::id(),
                uuid::Uuid::new_v4().simple()
            ));
            std::fs::create_dir_all(&pfad).unwrap();
            Self(pfad)
        }

        fn db(&self) -> std::path::PathBuf {
            self.0.join(STORE_DATEINAME)
        }
    }

    impl Drop for TestOrdner {
        fn drop(&mut self) {
            let _ = std::fs::remove_dir_all(&self.0);
        }
    }

    fn adresse() -> Adresse {
        Adresse {
            logon_sid: "S-1-5-21-1-2-3-1001".into(),
            project_binding_id: hex(0x11),
            session_epoch: hex(0x22),
            instance_id: hex(1),
            runtime_nonce: hex(0x40),
        }
    }

    /// Ein Coordinator MIT echtem Store. Ohne Store kehrt
    /// `persistenz_p0_intern` schon VOR dem Riegel mit `abgelehnt/internal`
    /// zurück (`befehl.rs`:175) — der Fall mäße dann nicht den Riegel,
    /// sondern seine Abwesenheit.
    fn coordinator_mit_store(name: &str) -> (Coordinator, StoreWriter, TestOrdner) {
        let ordner = TestOrdner::neu(name);
        let mut k = StoreKonfiguration::fuer_pfad(ordner.db());
        k.remote_volume_override = Some(false);
        let writer = StoreWriter::starten(k);
        assert!(!writer.ist_degradiert(), "{:?}", writer.handle().sicht());
        let c = Coordinator::mit_store(Arc::new(ManualClock::default()), hex(0xbeef), &writer);
        (c, writer, ordner)
    }

    /// Ein vollwertiger Sender: angemeldet, bestätigt, führendes Main, mit
    /// gemeldetem `record_state`. Derselbe Weg wie
    /// `sonde014_verdrahtung.rs::anmelden_fuer_passage` — ohne den Bericht
    /// wiese der Broker jeden persistenzpflichtigen Befehl mit
    /// `record_state_unknown` ab, und der Fall mäße nicht den Riegel.
    fn main_anmelden(c: &Coordinator, link: &str, a: &Adresse) {
        let hello = HelloControl {
            typ: "hello".into(),
            connection_kind: "control".into(),
            protocol: 3,
            plugin_version: "test".into(),
            plugin_kind: "main".into(),
            adresse: a.clone(),
            host: Some(HostAngabe {
                pid: 4711,
                name: Some("FL Studio".into()),
                version: None,
            }),
            audio: AudioLage {
                samplerate: 48_000.0,
                block_size: 512,
                channels: 2,
            },
        };
        let ausgang = c.control_hello_registrieren(link, &hello);
        assert!(ausgang.angenommen, "{:?}", ausgang.grund);
        let _ = c.resync_bestaetigen(link, 0);
        assert!(c.descriptor_setzen(
            link,
            json!({
                "adresse": a,
                "plugin_kind": "main",
                "measurement_position": "insert",
                "aussageklasse": "beobachtend",
                "betrieb": "active",
                "label": "Testquelle",
                "capabilities": {
                    "host_context": true, "project_time": true, "fine_automation": false,
                    "double_precision": false, "latency_report": false, "aux_send": false,
                    "aux_return": false, "compare_routing": false, "sidechain": false,
                    "offline_render": false
                },
                "frische": {"letzter_kontakt_ms": 10, "stale": false}
            })
        ));
        let bericht = json!({
            "type": "state_report",
            "adresse": a,
            "dsp_schema_version": 1,
            "state_revision": 0,
            "state_hash": "a".repeat(64),
            "record_state": {"valid": true, "recording": false}
        });
        assert!(c.state_report_json(link, &serde_json::to_vec(&bericht).unwrap()));
    }

    /// Ein `user_verdict` aus dem committeten Korpus — dieselbe Grundform,
    /// die der Integrationstest fährt, damit nicht zwei Wire-Wahrheiten
    /// nebeneinander stehen.
    fn user_verdict_wert(ziel: &Adresse, command: usize, finding_id: &str) -> Value {
        let mut wert: Value = serde_json::from_str(include_str!(
            "../../../eq-copilot/fixtures/v3/gueltig/user_verdict.json"
        ))
        .expect("Fixture ist JSON");
        wert["kopf"]["ziel"] = serde_json::to_value(ziel).unwrap();
        wert["kopf"]["command_id"] = json!(hex(command));
        wert["kopf"]["base_revision"] = json!(0);
        wert["finding_id"] = json!(finding_id);
        wert["user_verdict_id"] = json!(hex(0xd0d0 + command));
        wert
    }

    /// **V-34, zweiter Teilfall — die Vorbedingung UNTERSCHEIDEND gemessen.**
    ///
    /// Die Lage: ein Main meldet sich an und meldet State; keine Sonde, keine
    /// Evidenz, kein Befund. `befunde_eintragen` läuft nie, und weil seine
    /// einzige `insert`-Stelle nur eine NICHTLEERE Liste einträgt
    /// (`hypothese_verdrahtung.rs`:471–476; beide Räumstellen `remove`n den
    /// Schlüssel, `:472` und `:732`), hat `stand.befunde` für diese Sitzung
    /// keinen Eintrag.
    ///
    /// Das wird hier VOR dem Urteil ausdrücklich geprüft — an
    /// `stand.befunde.get(&session)`, nicht an einer Sicht: `befunde_sicht`
    /// macht aus `None` und `Some(vec![])` dieselbe leere Liste, und
    /// `is_empty()` auf ihr wäre auch dann grün, wenn der `None`-Zweig nie
    /// gefahren würde.
    ///
    /// Dann fällt das Urteil auf eine wohlgeformte, unbekannte `finding_id`:
    /// `abgelehnt` mit `unknown_target` — derselbe Code wie V-33, kein
    /// dritter Zustand — und im Store liegt nichts.
    ///
    /// Rotbeweis `NAK-214-rot-V-34.txt`, zweiter Abschnitt: dem `None`-Zweig
    /// „existent“ beigebracht (`map_or(true, ..)`) — der Fall fällt. Die
    /// Gegenprobe im selben Beweis trägt ein `Some(vec![])` von Hand ein: die
    /// Vorbedingung schlägt an, und damit ist belegt, dass diese Zusicherung
    /// die beiden Fälle wirklich trennt.
    #[test]
    fn ohne_sitzungseintrag_wird_das_urteil_abgewiesen() {
        let (c, writer, _ordner) = coordinator_mit_store("nak214-v34-none");
        let main = adresse();
        main_anmelden(&c, "main", &main);
        let session = ClientKey::aus_adresse(&main).session();

        // 🔑 DIE unterscheidende Zusicherung. Sie liest den Bestand selbst
        // und haelt ihr Lock nur fuer diese eine Frage: `Senke::p0` nimmt
        // dasselbe Lock, ein gehaltenes waere ein Deadlock.
        {
            let stand = c.stand.lock().unwrap_or_else(|e| e.into_inner());
            assert!(
                stand.befunde.get(&session).is_none(),
                "Vorbedingung: die Sitzung hat GAR KEINEN Eintrag in `stand.befunde` - \
                 nicht einen mit leerer Liste"
            );
        }

        // Wohlgeformt und unbekannt: 32 gueltige Hexzeichen.
        let unbekannt = hex(0xbeec);
        assert_eq!(
            unbekannt.len(),
            32,
            "der Zahlenrand: gueltige Form, unbekannt"
        );
        let command = 0x9450;
        let wert = user_verdict_wert(&main, command, &unbekannt);
        let antwort = Senke::p0(&c, "main", &serde_json::to_vec(&wert).unwrap())
            .expect("das Urteil wird beantwortet");
        let ack: Value = serde_json::from_slice(&antwort).expect("das Ack ist JSON");
        assert_eq!(
            ack["ergebnis"], "abgelehnt",
            "V-34: eine Sitzung OHNE Eintrag fuehrt diesen Befund nicht: {ack:?}"
        );
        assert_eq!(
            ack["code"], "unknown_target",
            "und zwar mit demselben Code wie V-33 (E7) - kein dritter Zustand: {ack:?}"
        );

        // Der Abweisungszweig committet nichts - weder den Befehlsriegel noch
        // das Domaenenereignis.
        assert!(
            writer
                .handle()
                .command_event_lesen(&hex(command))
                .expect("Store ist lesbar")
                .is_none(),
            "kein command_event"
        );
        let conn = rusqlite::Connection::open(writer.handle().db_pfad()).expect("Store ist lesbar");
        let urteile: i64 = conn
            .query_row(
                "SELECT COUNT(*) FROM event_log WHERE event_type = ?1",
                ["user_verdict"],
                |row| row.get(0),
            )
            .expect("das event_log ist lesbar");
        assert_eq!(
            urteile, 0,
            "kein `user_verdict`-Domaenenereignis im append-only Log"
        );
        // Gegenprobe zur Abwesenheit (Pruefliste E) - in zwei Haelften, weil
        // eine Abwesenheit ohne sie nichts beweist:
        //   (1) HIER: derselbe Ausdruck ueber dieselbe Tabelle findet Zeilen,
        //       wenn welche da sind - die Anmeldung dieser Buehne hat
        //       geschrieben.
        //   (2) Der Wert `user_verdict` ist der richtige `event_type`: der
        //       V-32-Fall in `sonde014_verdrahtung.rs` findet mit demselben
        //       Ausdruck GENAU EINE Zeile, wenn das Urteil angenommen wird.
        let gesamt: i64 = conn
            .query_row("SELECT COUNT(*) FROM event_log", [], |row| row.get(0))
            .expect("das event_log ist lesbar");
        assert!(
            gesamt > 0,
            "Gegenprobe: der Ausdruck findet die Zeilen der Anmeldung, zaehlt aber              0 - dann bewiese die Zeile darueber nichts"
        );

        // Und die Sitzung hat auch NACH der Abweisung keinen Eintrag: der
        // Riegel legt keinen an.
        {
            let stand = c.stand.lock().unwrap_or_else(|e| e.into_inner());
            assert!(
                stand.befunde.get(&session).is_none(),
                "der Riegel traegt nichts ein"
            );
        }
    }
}
