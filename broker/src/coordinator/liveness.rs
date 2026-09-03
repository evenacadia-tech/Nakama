//! Frische, Caps, Eviction, Heartbeat und Deskriptor.
//!
//! Der Liveness-Tick ist der einzige Ort, an dem eine Session ohne Zutun ihres
//! Clients ihren Zustand verliert; die Reihenfolge Eviction vor Zustellung ist
//! die abgenommene Zusage E-03.

use super::*;

impl Coordinator {
    pub(super) fn platz_schaffen_locked(
        &self,
        stand: &mut Stand,
        neu: &ClientKey,
        jetzt: Duration,
        schliessen: &mut Vec<String>,
    ) -> Result<(), String> {
        while stand
            .clients
            .keys()
            .filter(|k| {
                k.project_binding_id == neu.project_binding_id
                    && k.session_epoch == neu.session_epoch
            })
            .count()
            >= SESSION_CLIENT_CAP
        {
            let opfer =
                Self::stales_opfer(stand, Some(&neu.session()), jetzt).ok_or_else(|| {
                    "Sessioncap erreicht; nur fresh Clients, Neuzugang abgewiesen".to_string()
                })?;
            schliessen.extend(self.client_eviktieren_locked(stand, &opfer));
        }
        while stand.clients.len() >= GLOBAL_CLIENT_CAP {
            let opfer = Self::stales_opfer(stand, None, jetzt).ok_or_else(|| {
                "Globalcap erreicht; nur fresh Clients, Neuzugang abgewiesen".to_string()
            })?;
            schliessen.extend(self.client_eviktieren_locked(stand, &opfer));
        }
        // H-12: die Sessionmap hat eine eigene Schranke. Sie greift erst, wenn
        // die Aufraeumregel unten je umgangen wuerde - dann aber fail-closed
        // statt unbegrenzt wachsend.
        if !stand.sessions.contains_key(&neu.session()) && stand.sessions.len() >= GLOBAL_SESSION_CAP
        {
            return Err("Session-Globalcap erreicht; Neuzugang abgewiesen".to_string());
        }
        Ok(())
    }

    /// H-12: verbinden gegen trennen gilt auch fuer die Sessionmap. Eine
    /// Session ohne verbleibenden Client wird beim selben Vorgang entfernt, der
    /// ihren letzten Client entfernt. Die dauerhaften Konfliktriegel liegen in
    /// einer eigenen Map und bleiben unberuehrt - E-03 gilt unveraendert.
    pub(super) fn session_ohne_clients_entfernen_locked(stand: &mut Stand, session: &SessionKey) {
        if stand
            .clients
            .keys()
            .any(|key| &key.session() == session)
        {
            return;
        }
        stand.sessions.remove(session);
    }

    fn stales_opfer(
        stand: &Stand,
        session: Option<&SessionKey>,
        jetzt: Duration,
    ) -> Option<ClientKey> {
        stand
            .clients
            .iter()
            .filter(|(key, client)| {
                session.is_none_or(|s| &key.session() == s)
                    && (client.stale
                        || jetzt.saturating_sub(client.last_seen)
                            > Duration::from_millis(STALE_NACH_MS))
            })
            .min_by(|(_, a), (_, b)| {
                a.last_seen
                    .cmp(&b.last_seen)
                    .then_with(|| effektive_adresse(&a.adresse).cmp(&effektive_adresse(&b.adresse)))
            })
            .map(|(key, _)| key.clone())
    }

    fn client_eviktieren_locked(&self, stand: &mut Stand, key: &ClientKey) -> Vec<String> {
        let Some(client) = stand.clients.remove(key) else {
            return Vec::new();
        };
        stand.messframes.remove(key);
        stand.messfehler.remove(key);
        stand.lautheit.remove(key);
        let link_ids: Vec<String> = stand
            .links
            .iter()
            .filter(|(_, link)| &link.client_key == key)
            .map(|(id, _)| id.clone())
            .collect();
        let mut schliessen = Vec::new();
        for link_id in link_ids {
            if let Some(link) = stand.links.remove(&link_id) {
                self.alias_register.entferne(
                    &link.alias_adressraum,
                    &link.alias_besitzer,
                    &link.adresse.instance_id,
                );
                Self::subscription_entfernen_locked(stand, &link_id);
                stand.telemetry_links.remove(&link_id);
                let vorher = stand.interventionen.len();
                stand.interventionen.retain(|_, i| i.link_id != link_id);
                if vorher != stand.interventionen.len() {
                    stand.intervention_state_unknown = true;
                }
                schliessen.push(link_id);
            }
        }
        let session = key.session();
        if stand
            .sessions
            .get(&session)
            .and_then(|s| s.fuehrendes_main.as_deref())
            == Some(client.adresse.instance_id.as_str())
        {
            if let Some(s) = stand.sessions.get_mut(&session) {
                s.fuehrendes_main = None;
            }
        }
        Self::fuehrung_neu_bewerten_locked(stand, &session);
        stand.dirty_sessions.insert(session.clone());
        // H-12: derselbe Vorgang, der den letzten Client entfernt, entfernt
        // auch seine Session. Die dirty-Markierung steht bewusst DAVOR, damit
        // der Flush den nun leeren Stand noch an seine Abonnenten meldet.
        Self::session_ohne_clients_entfernen_locked(stand, &session);
        schliessen
    }

    pub(super) fn stale_aktualisieren_locked(stand: &mut Stand, jetzt: Duration) {
        let mut dirty = Vec::new();
        for (key, client) in &mut stand.clients {
            if !client.stale
                && jetzt.saturating_sub(client.last_seen) > Duration::from_millis(STALE_NACH_MS)
            {
                client.stale = true;
                client.stale_seit = Some(jetzt);
                dirty.push(key.session());
            }
        }
        stand.dirty_sessions.extend(dirty);
    }

    pub fn liveness_tick(&self) -> Vec<String> {
        let jetzt = self.clock.jetzt();
        let (schliessen, dirty) = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            Self::stale_aktualisieren_locked(&mut stand, jetzt);
            let opfer: Vec<ClientKey> = stand
                .clients
                .iter()
                .filter(|(_, client)| {
                    client.stale_seit.is_some_and(|seit| {
                        jetzt.saturating_sub(seit) >= Duration::from_millis(TOMBSTONE_MS)
                    })
                })
                .map(|(key, _)| key.clone())
                .collect();
            // `trennen` wird auch ausserhalb der Eviction gesetzt: bei
            // Storeverweigerung, fehlgeschlagenem Guard-Persist und einem
            // Report der verdraengten Nonce. Der produktive Supervisor
            // schliesst genau die Links, die dieser Tick zurueckgibt; ein
            // blosses Flag waere deshalb kein fail-closed Gegenpfad.
            let mut schliessen: HashSet<String> = stand
                .links
                .iter()
                .filter(|(_, link)| link.trennen)
                .map(|(link_id, _)| link_id.clone())
                .collect();
            for key in opfer {
                schliessen.extend(self.client_eviktieren_locked(&mut stand, &key));
            }
            for pending in Self::ungebundene_links_mit_eindeutigem_main_locked(&stand) {
                if let Some(link) = stand.links.get_mut(&pending) {
                    if !link.trennen {
                        link.trennen = true;
                        link.join_neuverbinden = true;
                    }
                }
                schliessen.insert(pending);
            }
            // H-12: dasselbe im Sweep - auch eine Session, deren letzter Client
            // ueber einen anderen Weg als die Eviction verschwand, faellt hier.
            let verwaist: Vec<SessionKey> = stand
                .sessions
                .keys()
                .filter(|session| !stand.clients.keys().any(|key| &key.session() == *session))
                .cloned()
                .collect();
            for session in verwaist {
                stand.sessions.remove(&session);
            }
            let dirty = stand.dirty_sessions.iter().cloned().collect::<Vec<_>>();
            let mut schliessen = schliessen.into_iter().collect::<Vec<_>>();
            schliessen.sort();
            (schliessen, dirty)
        };
        for session in dirty {
            self.flush_session(&session, None);
        }
        schliessen
    }

    pub fn heartbeat_kontakt(&self, link_id: &str, wert: Option<&Value>) -> bool {
        let jetzt = self.clock.jetzt();
        let mut guards = Vec::new();
        let (aktiv, dirty_sessions) = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(link) = stand.links.get(link_id).cloned() else {
                return false;
            };
            let session = link.client_key.session();
            let mut dirty_sessions = vec![session.clone()];
            if link.verdraengt {
                let effective = effektive_adresse(&link.adresse);
                let neuer_owner = stand.clients.get(&link.client_key).map(|client| {
                    format!("{}:{}", client.adresse.instance_id, client.current_nonce)
                });
                let _ = self.alias_register.registriere_wire_zuordnung(
                    &link.alias_adressraum,
                    &link.alias_besitzer,
                    &link.adresse.instance_id,
                );
                let mut ids = vec![link.alias_besitzer.clone()];
                if let Some(owner) = neuer_owner {
                    ids.push(owner);
                }
                for derived_id in ids {
                    if stand.guard_eintragen(&effective.clone(), &derived_id.clone())
                    {
                        guards.push(ConflictGuard {
                            effective_address: effective.clone(),
                            derived_id,
                            created_utc_ms: persistenz_utc_ms(),
                        });
                    }
                }
                if let Some(client) = stand.clients.get_mut(&link.client_key) {
                    client.bestaetigt = false;
                }
                if let Some(link) = stand.links.get_mut(link_id) {
                    link.trennen = true;
                }
                if let Some(s) = stand.sessions.get_mut(&session) {
                    if s.fuehrendes_main.as_deref() == Some(link.adresse.instance_id.as_str()) {
                        s.fuehrendes_main = None;
                    }
                }
                stand.dirty_sessions.insert(session.clone());
                (false, dirty_sessions)
            } else {
                let plugin_kind = stand
                    .clients
                    .get(&link.client_key)
                    .map(|client| client.plugin_kind.clone())
                    .unwrap_or_default();
                let runtime_gemeldet = wert.is_some_and(|v| v.get("runtime").is_some());
                let descriptor =
                    wert.and_then(|v| Self::descriptor_aus_heartbeat(&link, &plugin_kind, v));
                let capabilities = wert.and_then(|v| v.get("capabilities")).cloned();
                if let Some(client) = stand.clients.get_mut(&link.client_key) {
                    if client.current_link.as_deref() != Some(link_id) {
                        return false;
                    }
                    client.last_seen = jetzt;
                    client.stale = false;
                    client.stale_seit = None;
                    client.join_kandidat = true;
                    if runtime_gemeldet {
                        client.descriptor = descriptor;
                    } else if let (Some(descriptor), Some(capabilities)) =
                        (client.descriptor.as_mut(), capabilities)
                    {
                        if let Some(objekt) = descriptor.as_object_mut() {
                            objekt.insert("capabilities".into(), capabilities);
                        }
                    }
                }
                Self::auto_join_locked(&mut stand, &link.client_key);
                if stand
                    .clients
                    .get(&link.client_key)
                    .is_some_and(|client| client.session_ungebunden)
                {
                    if let Some(host_pid) = stand
                        .clients
                        .get(&link.client_key)
                        .and_then(|client| client.host_pid)
                    {
                        dirty_sessions.extend(Self::main_sessions_fuer_probe_locked(
                            &stand,
                            &link.wire_adresse,
                            host_pid,
                        ));
                    }
                }
                dirty_sessions.sort_by(|a, b| {
                    a.project_binding_id
                        .cmp(&b.project_binding_id)
                        .then_with(|| a.session_epoch.cmp(&b.session_epoch))
                });
                dirty_sessions.dedup();
                stand.dirty_sessions.extend(dirty_sessions.iter().cloned());
                stand.dirty_sessions.insert(session.clone());
                (true, dirty_sessions)
            }
        };
        self.guards_persistieren(guards);
        for session in dirty_sessions {
            self.flush_session(&session, Some(link_id));
        }
        aktiv
    }

    /// Ein schemafestes `state_report` ist neben dem Heartbeat ein gueltiger
    /// Liveness-Kontakt (E-01/E-02). Es traegt in Phase B keinen neuen
    /// MainProjectState-Ingress: fachlich wirkt er nur auf den schon im Hello
    /// gebundenen Client und durchlaeuft dieselbe Join-Regel wie ein Heartbeat.
    pub fn state_report_json(&self, link_id: &str, payload: &[u8]) -> bool {
        self.state_report_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
    }

    pub(super) fn state_report_json_mit_minor(&self, link_id: &str, payload: &[u8], schema_minor: u8) -> bool {
        let Some(wert) = v3_nachricht_lesen_mit_minor(payload, "state_report", schema_minor) else {
            return false;
        };
        let Ok(adresse) = serde_json::from_value::<Adresse>(wert["adresse"].clone()) else {
            return false;
        };
        let revision = wert.get("state_revision").and_then(Value::as_u64);
        let state_hash = wert
            .get("state_hash")
            .and_then(Value::as_str)
            .map(str::to_owned);
        let record_valid = wert.pointer("/record_state/valid").and_then(Value::as_bool);
        let recording = wert
            .pointer("/record_state/recording")
            .and_then(Value::as_bool);
        {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(link) = stand.links.get(link_id).cloned() else {
                return false;
            };
            if link.wire_adresse != adresse {
                return false;
            }
            let Some(client) = stand.clients.get_mut(&link.client_key) else {
                return false;
            };
            if client.current_link.as_deref() != Some(link_id) {
                return false;
            }
            client.state_revision = revision;
            client.state_hash = state_hash;
            client.record_state_valid = record_valid == Some(true);
            client.recording = recording == Some(true);
        }
        self.heartbeat_kontakt(link_id, None)
    }

    fn descriptor_aus_heartbeat(
        link: &LinkStand,
        plugin_kind: &str,
        wert: &Value,
    ) -> Option<Value> {
        let capabilities = wert.get("capabilities")?.clone();
        let runtime = wert.get("runtime")?.as_object()?;
        let messpunkt = runtime.get("messpunkt")?.as_str()?;
        let betrieb = runtime.get("betrieb")?.as_str()?;
        if !matches!(
            plugin_kind,
            "main" | "active_probe" | "passive_probe" | "legacy"
        ) {
            return None;
        }
        if plugin_kind == "main" && messpunkt != "insert" {
            return None;
        }
        if !matches!(messpunkt, "insert" | "pre" | "post")
            || !matches!(betrieb, "active" | "suspended" | "offline")
        {
            return None;
        }
        let label = runtime
            .get("label")
            .and_then(Value::as_str)
            .unwrap_or_default();
        let mut descriptor = serde_json::json!({
            "adresse": link.adresse,
            "plugin_kind": plugin_kind,
            "measurement_position": messpunkt,
            "aussageklasse": "beobachtend",
            "betrieb": betrieb,
            "label": label,
            "capabilities": capabilities,
            "frische": {"stale": false, "letzter_kontakt_ms": 0}
        });
        let objekt = descriptor.as_object_mut()?;
        for feld in ["host_bus_name", "host_mixer_index"] {
            if let Some(wert) = runtime.get(feld) {
                objekt.insert(feld.into(), wert.clone());
            }
        }
        Some(descriptor)
    }

    pub fn descriptor_setzen(&self, link_id: &str, mut descriptor: Value) -> bool {
        // H-17: der Setter prueft den Deskriptor gegen den v3-Vertrag, bevor er
        // ihn uebernimmt - nicht nur die Adresse. Damit gilt fuer JEDEN Weg
        // dieselbe Haertung, die descriptor_aus_heartbeat bisher allein trug,
        // und kein spaeterer Aufrufer kann sie umgehen. Der Vertrag pinnt die
        // Aussageklasse per Konstante auf `beobachtend`; ein Beitragsdeskriptor
        // faellt hier ab, bevor er in den Snapshot geraet. Ein cfg(test)-Zaun
        // schied aus: descriptor_setzen wird von einer Integrationstestdatei
        // gerufen, die die Bibliothek ohne Testkonfiguration bindet.
        if descriptor.get("aussageklasse").and_then(Value::as_str) != Some("beobachtend") {
            return false;
        }
        let session = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(link) = stand.links.get(link_id).cloned() else {
                return false;
            };
            let Ok(adresse) = serde_json::from_value::<Adresse>(descriptor["adresse"].clone())
            else {
                return false;
            };
            if adresse != link.wire_adresse {
                return false;
            }
            let Ok(effektive_adresse) = serde_json::to_value(&link.adresse) else {
                return false;
            };
            descriptor["adresse"] = effektive_adresse;
            if let Some(client) = stand.clients.get_mut(&link.client_key) {
                client.descriptor = Some(descriptor);
                client.join_kandidat = true;
            }
            Self::auto_join_locked(&mut stand, &link.client_key);
            let session = link.client_key.session();
            stand.dirty_sessions.insert(session.clone());
            session
        };
        self.flush_session(&session, Some(link_id));
        true
    }
}
