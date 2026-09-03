//! Beitritt und Fuehrung.
//!
//! Wer einer Session beitritt und wer in ihr schreiben darf, entscheidet sich
//! hier - und nur hier.

use super::*;

impl Coordinator {
    pub(super) fn auto_join_locked(stand: &mut Stand, key: &ClientKey) {
        let Some(client) = stand.clients.get(key) else {
            return;
        };
        let Some(host_pid) = client.host_pid.filter(|pid| *pid != 0) else {
            return;
        };
        let effective = effektive_adresse(&client.adresse);
        if stand.conflict_guards.contains_key(&effective) || !stand.routing_bereit {
            return;
        }
        let main_sessions: HashSet<SessionKey> = stand
            .clients
            .iter()
            .filter(|(_, c)| c.plugin_kind == "main" && c.host_pid == Some(host_pid))
            .map(|(k, _)| k.session())
            .collect();
        let eindeutig = main_sessions.len() == 1 && main_sessions.contains(&key.session());
        let main_anzahl_in_session = stand
            .clients
            .iter()
            .filter(|(client_key, client)| {
                client_key.session() == key.session() && client.plugin_kind == "main"
            })
            .count();
        let betroffene: Vec<ClientKey> = stand
            .clients
            .iter()
            .filter(|(_, client)| client.host_pid == Some(host_pid))
            .map(|(key, _)| key.clone())
            .collect();
        let mut sessions = HashSet::new();
        for betroffen in betroffene {
            if let Some(client) = stand.clients.get_mut(&betroffen) {
                if !client.explizit_bestaetigt && !client.ausdruecklich_ungebunden {
                    client.bestaetigt = eindeutig
                        && betroffen.session() == key.session()
                        && (client.plugin_kind != "main"
                            || main_anzahl_in_session == FUEHRENDE_MAINS_PRO_SESSION);
                }
                sessions.insert(betroffen.session());
            }
        }
        for session in sessions {
            Self::fuehrung_neu_bewerten_locked(stand, &session);
        }
    }

    pub(super) fn fuehrung_neu_bewerten_locked(stand: &mut Stand, session: &SessionKey) {
        let bisher = stand
            .sessions
            .get(session)
            .and_then(|s| s.fuehrendes_main.clone());
        let bisher_gueltig = bisher.as_ref().is_some_and(|instance_id| {
            stand.clients.iter().any(|(key, client)| {
                &key.session() == session
                    && &key.instance_id == instance_id
                    && client.plugin_kind == "main"
                    && client.bestaetigt
                    && !stand
                        .conflict_guards
                        .contains_key(&effektive_adresse(&client.adresse))
            })
        });
        if bisher_gueltig {
            return;
        }
        let mut mains: Vec<String> = stand
            .clients
            .iter()
            .filter(|(key, client)| {
                &key.session() == session
                    && client.plugin_kind == "main"
                    && client.bestaetigt
                    && !stand
                        .conflict_guards
                        .contains_key(&effektive_adresse(&client.adresse))
            })
            .map(|(key, _)| key.instance_id.clone())
            .collect();
        mains.sort();
        mains.dedup();
        let neu = (mains.len() == FUEHRENDE_MAINS_PRO_SESSION).then(|| mains[0].clone());
        stand
            .sessions
            .entry(session.clone())
            .or_default()
            .fuehrendes_main = neu;
    }

    pub fn beitritt_bestaetigen(
        &self,
        project_binding_id: &str,
        session_epoch: &str,
        instance_id: &str,
    ) -> bool {
        let session = SessionKey {
            project_binding_id: project_binding_id.into(),
            session_epoch: session_epoch.into(),
        };
        let geaendert = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let key = stand
                .clients
                .keys()
                .find(|key| &key.session() == &session && key.instance_id == instance_id)
                .cloned();
            let Some(key) = key else { return false };
            Self::beitritt_bestaetigen_locked(&mut stand, &session, &key)
        };
        self.flush_session(&session, None);
        geaendert
    }

    pub(super) fn beitritt_bestaetigen_locked(
        stand: &mut Stand,
        session: &SessionKey,
        key: &ClientKey,
    ) -> bool {
        let client = stand.clients.get_mut(key).expect("Clientschluessel");
        let geaendert = !client.bestaetigt;
        client.join_kandidat = true;
        client.bestaetigt = true;
        client.explizit_bestaetigt = true;
        client.ausdruecklich_ungebunden = false;
        Self::fuehrung_neu_bewerten_locked(stand, session);
        stand.dirty_sessions.insert(session.clone());
        geaendert
    }

    /// Fluechtiger Broker-Gegenpfad zur bestaetigten Mitgliedschaft. Die
    /// persistente MainProject-/Host-Dirty-Wirkung gehoert der Plugin-Etappe;
    /// hier wird ausschliesslich der aktuelle Sessiongraph atomar entbunden
    /// und als absoluter Snapshot weitergegeben.
    pub fn beitritt_aufheben(
        &self,
        project_binding_id: &str,
        session_epoch: &str,
        instance_id: &str,
    ) -> bool {
        let session = SessionKey {
            project_binding_id: project_binding_id.into(),
            session_epoch: session_epoch.into(),
        };
        let geaendert = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let key = stand
                .clients
                .keys()
                .find(|key| &key.session() == &session && key.instance_id == instance_id)
                .cloned();
            let Some(key) = key else { return false };
            Self::beitritt_aufheben_locked(&mut stand, &session, &key)
        };
        self.flush_session(&session, None);
        geaendert
    }

    pub(super) fn beitritt_aufheben_locked(
        stand: &mut Stand,
        session: &SessionKey,
        key: &ClientKey,
    ) -> bool {
        let client = stand.clients.get_mut(key).expect("Clientschluessel");
        let geaendert =
            client.bestaetigt || client.explizit_bestaetigt || !client.ausdruecklich_ungebunden;
        client.bestaetigt = false;
        client.explizit_bestaetigt = false;
        client.ausdruecklich_ungebunden = true;
        client.join_kandidat = true;
        Self::fuehrung_neu_bewerten_locked(stand, session);
        stand.dirty_sessions.insert(session.clone());
        geaendert
    }

    pub fn fuehrung_uebergeben(
        &self,
        project_binding_id: &str,
        session_epoch: &str,
        neue_instance_id: &str,
    ) -> bool {
        let session = SessionKey {
            project_binding_id: project_binding_id.into(),
            session_epoch: session_epoch.into(),
        };
        {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let erlaubt = stand.clients.iter().any(|(key, client)| {
                &key.session() == &session
                    && key.instance_id == neue_instance_id
                    && client.plugin_kind == "main"
                    && client.bestaetigt
                    && !stand
                        .conflict_guards
                        .contains_key(&effektive_adresse(&client.adresse))
            });
            if !erlaubt {
                return false;
            }
            let sitzung = stand.sessions.entry(session.clone()).or_default();
            // Unter demselben Lock: zuerst entziehen, dann genau einen neuen
            // Besitzer setzen. Kein Leser kann einen Doppelstand beobachten.
            sitzung.fuehrendes_main = None;
            sitzung.fuehrendes_main = Some(neue_instance_id.to_owned());
            stand.dirty_sessions.insert(session.clone());
        }
        self.flush_session(&session, None);
        true
    }

    pub fn main_darf_schreiben(&self, link_id: &str) -> bool {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let Some(link) = stand.links.get(link_id) else {
            return false;
        };
        let Some(client) = stand.clients.get(&link.client_key) else {
            return false;
        };
        client.plugin_kind == "main"
            && client.bestaetigt
            && stand
                .sessions
                .get(&link.client_key.session())
                .and_then(|session| session.fuehrendes_main.as_deref())
                == Some(link.client_key.instance_id.as_str())
            && self.dispatch_fuer_link_erlaubt_locked(&stand, link)
    }

    pub(super) fn beitritt_noetig_locked(stand: &Stand, session: &SessionKey) -> bool {
        let clients: Vec<&ClientStand> = stand
            .clients
            .iter()
            .filter(|(key, _)| &key.session() == session)
            .map(|(_, client)| client)
            .collect();
        if clients.iter().any(|client| {
            (client.join_kandidat && !client.bestaetigt)
                || stand
                    .conflict_guards
                    .contains_key(&effektive_adresse(&client.adresse))
        }) {
            return true;
        }
        let main_hosts: HashSet<u32> = stand
            .clients
            .iter()
            .filter(|(key, client)| &key.session() == session && client.plugin_kind == "main")
            .filter_map(|(_, client)| client.host_pid)
            .collect();
        if stand.clients.iter().any(|(key, client)| {
            client.session_ungebunden
                && key.project_binding_id == session.project_binding_id
                && client.host_pid.is_some_and(|pid| main_hosts.contains(&pid))
        }) {
            return true;
        }
        let mains: HashSet<&str> = clients
            .iter()
            .filter(|client| client.plugin_kind == "main" && client.bestaetigt)
            .map(|client| client.adresse.instance_id.as_str())
            .collect();
        stand
            .sessions
            .get(session)
            .and_then(|s| s.fuehrendes_main.as_ref())
            .is_none()
            && mains.len() > FUEHRENDE_MAINS_PRO_SESSION
    }
}
