//! SONDE-011: der semantische v3-Coordinator hinter dem byteorientierten
//! Transport.
//!
//! Phase B erweitert den Phase-A-Kern um den allein besessenen fluechtigen
//! Sessiongraphen, monotone Liveness, Nonce-Verdrängung und die Store-/
//! Snapshot-Outbox-Grenze. I/O bleibt byteorientiert; nur dieser Typ mutiert
//! fachlichen Sitzungszustand.
//!
//! * `subscribe_session` gehoert genau dem Control-Link und dessen Hello-
//!   Adresse. Sein einziger Gegenpfad ist das Ende dieses Links (28-B).
//! * v2-Hoermarkierung und v3-Interventionsereignisse speisen genau einen
//!   fail-closed Zustand. Overflow, Sequenzluecke und Control-Ende setzen ein
//!   sticky Unknown-Bit; normale End-/False-Meldungen loeschen es nie.

use crate::instance_alias::{AliasRegister, Registrierung, Sitzungsadressraum};
use crate::store::{
    ConflictGuard, SnapshotZiel, StoreEvent, StoreHandle, StoreWriter, STORE_SCHEMA_MAJOR,
};
use crate::transport::bootstrap::{Adresse, AudioLage, HelloControl};
use serde_json::Value;
use sha2::{Digest, Sha256};
use std::collections::{HashMap, HashSet, VecDeque};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, Condvar, Mutex, OnceLock};
use std::time::Duration;

mod flush;
mod intervention;
mod liveness;
mod mitgliedschaft;
mod schema;
mod senke;
mod sicht;
mod subscription;
mod uhr;
mod zustand;

pub use sicht::{
    ClientModellSicht, ControlRegistrierung, Interventionssicht, Lautheitszustand,
    MessframeSicht, SessionModellSicht,
};
pub use uhr::{ManualClock, MonotonicClock};
use zustand::{
    ClientKey, ClientStand, Intervention, LinkStand, LiveMessframe, P2RejectGrund,
    SessionCommandWirkung, SessionKey, Stand, Subscription, JSON_SAFE_INTEGER_MAX,
};
use schema::{
    projektion_mit_aktuellem_lauf, v3_nachricht_lesen, v3_nachricht_lesen_beliebig_mit_minor,
    v3_nachricht_lesen_mit_minor, JSON_SCHEMA_MINOR_AKTIV,
};
use uhr::InstantClock;

const MAX_AKTIVE_INTERVENTIONEN: usize = 64;
pub const HEARTBEAT_INTERVAL_MS: u64 = 1000;
pub const STALE_VERPASSTE_INTERVALLE: u64 = 2;
pub const STALE_JITTER_MS: u64 = 500;
pub const STALE_NACH_MS: u64 = STALE_VERPASSTE_INTERVALLE * HEARTBEAT_INTERVAL_MS + STALE_JITTER_MS;
pub const TOMBSTONE_MS: u64 = 10_000;
pub const SESSION_CLIENT_CAP: usize = 64;
pub const GLOBAL_CLIENT_CAP: usize = 128;
pub const FUEHRENDE_MAINS_PRO_SESSION: usize = 1;
pub const SICHTBARE_SONDEN_NORMAL: usize = 16;
pub const LAST_SONDEN: usize = 32;
pub const SESSION_SUBSCRIPTION_EVENT_REPLAY_MAX: usize = 0;
const SESSION_COMMAND_REGISTER_MAX: usize = SESSION_CLIENT_CAP * SESSION_CLIENT_CAP;

pub trait SessionPush: Send + Sync {
    /// `true` bedeutet ausschliesslich: der volle Snapshot wurde auf die Pipe
    /// geschrieben. Es ist keine Empfängerwirkung und kein Wire-ACK.
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool;

    /// Nichtblockierender, begrenzter P2-Push. `false` bedeutet, dass der
    /// Subscriber keine gekoppelte Telemetrieausgabe besitzt oder deren
    /// Queue den Frame nicht aufnehmen konnte. Der Broker wartet hier nie auf
    /// Pipe-I/O; Rueckstau darf den Telemetrieeingang nicht blockieren.
    fn messframe_schreiben(&self, _link_id: &str, _instance_id: &str, _payload: &[u8]) -> bool {
        false
    }
}

/// Einmalige, schlaflose Testnaht direkt nach dem Snapshot-Capture. Produktion
/// setzt sie nie; sie macht die sonst mikroskopische Reihenfolge
/// "alt erfasst, neu committed, alt committed" deterministisch pruefbar.
#[doc(hidden)]
#[derive(Clone, Default)]
pub struct CoordinatorFlushTestHaken {
    stand: Arc<(Mutex<(bool, bool)>, Condvar)>,
}

impl CoordinatorFlushTestHaken {
    pub fn warten_bis_erfasst(&self) {
        let (schloss, signal) = &*self.stand;
        let mut stand = schloss.lock().unwrap_or_else(|e| e.into_inner());
        while !stand.0 {
            stand = signal.wait(stand).unwrap_or_else(|e| e.into_inner());
        }
    }

    pub fn freigeben(&self) {
        let (schloss, signal) = &*self.stand;
        let mut stand = schloss.lock().unwrap_or_else(|e| e.into_inner());
        stand.1 = true;
        signal.notify_all();
    }

    fn erreichen(&self) {
        let (schloss, signal) = &*self.stand;
        let mut stand = schloss.lock().unwrap_or_else(|e| e.into_inner());
        stand.0 = true;
        signal.notify_all();
        while !stand.1 {
            stand = signal.wait(stand).unwrap_or_else(|e| e.into_inner());
        }
    }
}

pub struct Coordinator {
    stand: Mutex<Stand>,
    alias_register: AliasRegister,
    clock: Arc<dyn MonotonicClock>,
    store: Option<StoreHandle>,
    broker_epoch: String,
    event_sequence: AtomicU64,
    push: Mutex<Option<Arc<dyn SessionPush>>>,
    /// Feste, begrenzte Shards serialisieren den Capture-/Commit-/Push-Pfad
    /// derselben Session. So kann ein spaeter gestarteter Flush seinen
    /// neueren Stand nie vor einem pausierten aelteren Flush committen.
    session_flush_schloesser: Vec<Mutex<()>>,
    flush_test_haken: Mutex<Option<CoordinatorFlushTestHaken>>,
}

const SESSION_FLUSH_SCHLOSS_ANZAHL: usize = 64;

impl Default for Coordinator {
    fn default() -> Self {
        Self::mit_uhr(
            Arc::new(InstantClock::default()),
            uuid::Uuid::new_v4().simple().to_string(),
        )
    }
}

impl Coordinator {

    pub fn mit_uhr(clock: Arc<dyn MonotonicClock>, broker_epoch: String) -> Self {
        Self {
            stand: Mutex::new(Stand::default()),
            alias_register: AliasRegister::default(),
            clock,
            store: None,
            broker_epoch,
            event_sequence: AtomicU64::new(0),
            push: Mutex::new(None),
            session_flush_schloesser: (0..SESSION_FLUSH_SCHLOSS_ANZAHL)
                .map(|_| Mutex::new(()))
                .collect(),
            flush_test_haken: Mutex::new(None),
        }
    }

    pub fn mit_store(
        clock: Arc<dyn MonotonicClock>,
        broker_epoch: String,
        store_writer: &StoreWriter,
    ) -> Self {
        let mut stand = Stand::default();
        stand.routing_bereit = !store_writer.ist_degradiert();
        if stand.routing_bereit {
            for guard in store_writer.restaurierte_guards() {
                stand
                    .conflict_guards
                    .entry(guard.effective_address.clone())
                    .or_default()
                    .insert(guard.derived_id.clone());
            }
        }
        Self {
            stand: Mutex::new(stand),
            alias_register: AliasRegister::default(),
            clock,
            store: Some(store_writer.handle()),
            broker_epoch,
            event_sequence: AtomicU64::new(0),
            push: Mutex::new(None),
            session_flush_schloesser: (0..SESSION_FLUSH_SCHLOSS_ANZAHL)
                .map(|_| Mutex::new(()))
                .collect(),
            flush_test_haken: Mutex::new(None),
        }
    }

    pub fn instant_mit_store(broker_epoch: String, store_writer: &StoreWriter) -> Self {
        Self::mit_store(
            Arc::new(InstantClock::default()),
            broker_epoch,
            store_writer,
        )
    }

    pub fn session_push_setzen(&self, push: Arc<dyn SessionPush>) {
        *self.push.lock().unwrap_or_else(|e| e.into_inner()) = Some(push);
    }

    #[doc(hidden)]
    pub fn flush_test_haken_setzen(&self, haken: CoordinatorFlushTestHaken) {
        *self
            .flush_test_haken
            .lock()
            .unwrap_or_else(|e| e.into_inner()) = Some(haken);
    }

    /// Kompatibler Phase-A-Einstieg fuer direkte Modelltests.
    pub fn control_registrieren(&self, link_id: &str, adresse: Adresse) {
        let hello = HelloControl {
            typ: "hello".into(),
            connection_kind: "control".into(),
            protocol: 3,
            plugin_version: "test".into(),
            plugin_kind: "active_probe".into(),
            adresse,
            host: None,
            audio: AudioLage {
                samplerate: 48_000.0,
                block_size: 512,
                channels: 2,
            },
        };
        let _ = self.control_hello_registrieren(link_id, &hello);
    }

    fn ist_ungebundene_probe(hello: &HelloControl) -> bool {
        matches!(hello.plugin_kind.as_str(), "active_probe" | "passive_probe")
            && hello.adresse.session_epoch == hello.adresse.project_binding_id
    }

    fn main_sessions_fuer_probe_locked(
        stand: &Stand,
        adresse: &Adresse,
        host_pid: u32,
    ) -> HashSet<SessionKey> {
        stand
            .clients
            .iter()
            .filter(|(key, client)| {
                client.plugin_kind == "main"
                    && client.host_pid == Some(host_pid)
                    && key.logon_sid == adresse.logon_sid
                    && key.project_binding_id == adresse.project_binding_id
            })
            .map(|(key, _)| key.session())
            .collect()
    }

    fn eindeutige_main_session_locked(
        stand: &Stand,
        adresse: &Adresse,
        host_pid: Option<u32>,
    ) -> Option<SessionKey> {
        let sessions = Self::main_sessions_fuer_probe_locked(stand, adresse, host_pid?);
        (sessions.len() == 1).then(|| sessions.into_iter().next().expect("genau eine Session"))
    }

    fn ungebundene_links_mit_eindeutigem_main_locked(stand: &Stand) -> Vec<String> {
        stand
            .links
            .iter()
            .filter(|(_, link)| {
                stand.clients.get(&link.client_key).is_some_and(|client| {
                    client.session_ungebunden
                        && Self::eindeutige_main_session_locked(
                            stand,
                            &link.wire_adresse,
                            client.host_pid,
                        )
                        .is_some()
                })
            })
            .map(|(link_id, _)| link_id.clone())
            .collect()
    }

    pub fn control_hello_registrieren(
        &self,
        link_id: &str,
        hello: &HelloControl,
    ) -> ControlRegistrierung {
        let jetzt = self.clock.jetzt();
        let wire_adresse = hello.adresse.clone();
        let host_pid = hello.host.as_ref().map(|h| h.pid);
        let mut adresse = wire_adresse.clone();
        let mut session_ungebunden = Self::ist_ungebundene_probe(hello);
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        if session_ungebunden {
            if let Some(main_session) =
                Self::eindeutige_main_session_locked(&stand, &wire_adresse, host_pid)
            {
                adresse.session_epoch = main_session.session_epoch;
                session_ungebunden = false;
            }
        }
        let key = ClientKey::aus_adresse(&adresse);
        let alias_adressraum = Sitzungsadressraum::neu(
            &adresse.logon_sid,
            &adresse.project_binding_id,
            &adresse.session_epoch,
        );
        let alias_besitzer = format!("{}:{}", adresse.instance_id, adresse.runtime_nonce);
        let effective = effektive_adresse(&adresse);
        let mut guards_zu_persistieren = Vec::new();
        let mut schliessen = Vec::new();

        Self::stale_aktualisieren_locked(&mut stand, jetzt);
        if !stand.clients.contains_key(&key) {
            if let Err(grund) = self.platz_schaffen_locked(&mut stand, &key, jetzt, &mut schliessen)
            {
                stand.cap_abweisungen = stand.cap_abweisungen.saturating_add(1);
                return ControlRegistrierung::abgewiesen(grund);
            }
        }

        let geerbt = stand.clients.get(&key).cloned();
        if let Some(alt) = &geerbt {
            if alt.current_nonce != adresse.runtime_nonce {
                // Liveframes und ihre Fehler gehoeren zur Runtime-Nonce. Die
                // stabile Mitgliedschaft bleibt am ClientKey, aber ein neuer
                // Prozess darf niemals die Messwahrheit des alten erben.
                stand.messframes.remove(&key);
                stand.messfehler.remove(&key);
                stand.lautheit.remove(&key);
            }
            if let Some(alter_link) = alt.current_link.as_deref() {
                if alter_link != link_id && alt.current_nonce != adresse.runtime_nonce {
                    if let Some(link) = stand.links.get_mut(alter_link) {
                        link.verdraengt = true;
                        link.trennen = true;
                        self.alias_register.entferne(
                            &link.alias_adressraum,
                            &link.alias_besitzer,
                            &link.adresse.instance_id,
                        );
                    }
                    Self::subscription_entfernen_locked(&mut stand, alter_link);
                    schliessen.push(alter_link.to_owned());
                }
            }
        }

        let registrierung = self.alias_register.registriere_wire_zuordnung(
            &alias_adressraum,
            &alias_besitzer,
            &adresse.instance_id,
        );
        if registrierung == Registrierung::KollisionBeideQuarantaenisiert {
            let mut ids = vec![alias_besitzer.clone()];
            if let Some(alt) = &geerbt {
                ids.push(format!("{}:{}", alt.adresse.instance_id, alt.current_nonce));
            }
            for derived_id in ids {
                if stand
                    .conflict_guards
                    .entry(effective.clone())
                    .or_default()
                    .insert(derived_id.clone())
                {
                    guards_zu_persistieren.push(ConflictGuard {
                        effective_address: effective.clone(),
                        derived_id,
                        created_utc_ms: persistenz_utc_ms(),
                    });
                }
            }
        }

        let client = ClientStand {
            adresse: adresse.clone(),
            plugin_kind: hello.plugin_kind.clone(),
            host_pid,
            session_ungebunden,
            current_link: Some(link_id.to_owned()),
            current_nonce: adresse.runtime_nonce.clone(),
            last_seen: jetzt,
            stale: false,
            stale_seit: None,
            join_kandidat: geerbt.as_ref().is_some_and(|c| c.join_kandidat),
            bestaetigt: geerbt.as_ref().is_some_and(|c| c.bestaetigt),
            explizit_bestaetigt: geerbt.as_ref().is_some_and(|c| c.explizit_bestaetigt),
            ausdruecklich_ungebunden: geerbt.as_ref().is_some_and(|c| c.ausdruecklich_ungebunden),
            // Runtime-Felder sind linkgebunden. Auch ein Reconnect derselben
            // Runtime muss den laut E-M01 vorgeschriebenen ersten
            // heartbeat.runtime erneut liefern; bis dahin bleibt die Quelle
            // als Mitglied ohne Messdescriptor sichtbar.
            descriptor: None,
            state_revision: geerbt.as_ref().and_then(|c| c.state_revision),
            state_hash: geerbt.as_ref().and_then(|c| c.state_hash.clone()),
            record_state_valid: geerbt.as_ref().is_some_and(|c| c.record_state_valid),
            recording: geerbt.as_ref().is_some_and(|c| c.recording),
        };
        stand.clients.insert(key.clone(), client);
        stand.sessions.entry(key.session()).or_default();
        stand.links.insert(
            link_id.to_owned(),
            LinkStand {
                wire_adresse,
                adresse,
                client_key: key,
                alias_adressraum,
                alias_besitzer,
                letzte_event_sequence: None,
                verdraengt: false,
                trennen: false,
                join_neuverbinden: false,
            },
        );
        // Kam die Probe vor dem Main, bleibt sie intern ungebundener Kandidat.
        // Sobald genau eine passende Main-Sitzung existiert, schliesst der
        // Coordinator den alten Link kontrolliert. Der normale Reconnect-Hello
        // wird dann direkt mit der Main-Epoche registriert; keine Alias- oder
        // Subscription-Hoheit wird unter einer laufenden Verbindung umgehängt.
        for pending in Self::ungebundene_links_mit_eindeutigem_main_locked(&stand) {
            if let Some(link) = stand.links.get_mut(&pending) {
                if !link.trennen {
                    link.trennen = true;
                    link.join_neuverbinden = true;
                }
            }
            schliessen.push(pending);
        }
        schliessen.sort();
        schliessen.dedup();
        drop(stand);

        for guard in guards_zu_persistieren {
            if self
                .store
                .as_ref()
                .is_some_and(|store| store.konflikt_guard_persistieren(guard).is_err())
            {
                self.routing_fail_closed("Konfliktriegel konnte nicht persistiert werden");
            }
        }
        ControlRegistrierung::angenommen(schliessen)
    }

    /// Der synchrone Server-Hook. Subscription, Link und aktive Ereignisse
    /// werden unter EINEM Lock entfernt; gleichzeitig wird Unknown sticky.
    /// Ein Push teilt diesen Lock und kann den Zwischenzustand nicht sehen.
    pub fn control_ende(&self, link_id: &str) {
        let jetzt = self.clock.jetzt();
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        let mut dirty = None;
        if let Some(link) = stand.links.remove(link_id) {
            self.alias_register.entferne(
                &link.alias_adressraum,
                &link.alias_besitzer,
                &link.adresse.instance_id,
            );
            let join_reconnect_ohne_tombstone = link.join_neuverbinden
                && link.letzte_event_sequence.is_none()
                && !stand
                    .interventionen
                    .values()
                    .any(|intervention| intervention.link_id == link_id);
            if join_reconnect_ohne_tombstone {
                // Der Marker ist keine Sessionidentitaet und darf nach dem
                // kontrollierten Reconnect keinen Phantom-Tombstone erzeugen.
                stand.clients.remove(&link.client_key);
                stand.messframes.remove(&link.client_key);
                stand.messfehler.remove(&link.client_key);
                stand.lautheit.remove(&link.client_key);
            } else if let Some(client) = stand.clients.get_mut(&link.client_key) {
                if client.current_link.as_deref() == Some(link_id) {
                    client.current_link = None;
                    client.stale = true;
                    client.stale_seit = Some(jetzt);
                    dirty = Some(link.client_key.session());
                }
            }
        }
        if let Some(session) = dirty {
            stand.dirty_sessions.insert(session);
        }
        Self::subscription_entfernen_locked(&mut stand, link_id);
        stand.telemetry_links.remove(link_id);
        stand
            .interventionen
            .retain(|_, intervention| intervention.link_id != link_id);
        // C-08 gilt fuer jeden Control-Disconnect, auch wenn der interne
        // Joinpfad den Reconnect angefordert hat. Nur neutral_resync loest.
        stand.intervention_state_unknown = true;
    }

    pub fn verbindung_soll_trennen(&self, link_id: &str) -> bool {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .links
            .get(link_id)
            .is_some_and(|link| link.trennen)
    }

    fn adresse_des_links_passt(stand: &Stand, link_id: &str, adresse: &Adresse) -> bool {
        stand
            .links
            .get(link_id)
            .is_some_and(|link| &link.wire_adresse == adresse)
    }

    fn aktueller_telemetrie_client_locked(stand: &Stand, link_id: &str) -> Option<ClientKey> {
        if !stand.telemetry_links.contains(link_id) {
            return None;
        }
        let link = stand.links.get(link_id)?;
        if link.trennen {
            return None;
        }
        let client = stand.clients.get(&link.client_key)?;
        (client.current_link.as_deref() == Some(link_id)
            && client.current_nonce == link.adresse.runtime_nonce)
            .then(|| link.client_key.clone())
    }

    fn command_ack(
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

    fn persistierte_command_wirkung(payload: &[u8]) -> Option<(Value, u64, String)> {
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

    fn persistenz_p0(&self, link_id: &str, wert: &Value) -> Option<Vec<u8>> {
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

    fn session_command(&self, link_id: &str, wert: &Value) -> Option<Vec<u8>> {
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

    fn p0_json(&self, link_id: &str, payload: &[u8]) -> Option<Vec<u8>> {
        self.p0_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
    }

    fn p0_json_mit_minor(
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

fn effektive_adresse(adresse: &Adresse) -> String {
    format!(
        "{}|{}|{}|{}",
        adresse.logon_sid, adresse.project_binding_id, adresse.session_epoch, adresse.instance_id
    )
}

fn persistenz_utc_ms() -> i64 {
    use std::time::{SystemTime, UNIX_EPOCH};
    SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|dauer| dauer.as_millis().min(i64::MAX as u128) as i64)
        .unwrap_or(0)
}

#[cfg(test)]
mod tests {
    use super::zustand::P2_REJECT_KATALOG;
    use super::*;

    #[test]
    fn p2_reject_katalog_stimmt_mit_dem_strikten_wire_schema() {
        let schema: Value = serde_json::from_str(include_str!(
            "../../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
        ))
        .unwrap();
        let wire = schema["$defs"]["p2_reject"]["properties"]["grund"]["enum"]
            .as_array()
            .unwrap()
            .iter()
            .map(|wert| wert.as_str().unwrap())
            .collect::<HashSet<_>>();
        let broker = P2_REJECT_KATALOG
            .iter()
            .map(|grund| grund.wire())
            .collect::<HashSet<_>>();
        assert_eq!(wire, broker);
    }

    fn adresse(zeichen: char) -> Adresse {
        Adresse {
            logon_sid: "S-1-5-21-1-2-3-1001".into(),
            project_binding_id: zeichen.to_string().repeat(32),
            session_epoch: zeichen.to_string().repeat(32),
            instance_id: zeichen.to_string().repeat(32),
            runtime_nonce: zeichen.to_string().repeat(32),
        }
    }

    fn verbunden() -> (Coordinator, Adresse) {
        let coordinator = Coordinator::default();
        let a = adresse('a');
        coordinator.control_registrieren("link-a", a.clone());
        (coordinator, a)
    }

    fn heartbeat_duplicate(c: &Coordinator, link_id: &str) -> bool {
        let adresse = c
            .stand
            .lock()
            .unwrap()
            .links
            .get(link_id)
            .unwrap()
            .adresse
            .clone();
        let payload = serde_json::to_vec(&serde_json::json!({
            "type": "heartbeat",
            "adresse": adresse,
            "sequence": 1,
            "state_revision": 0,
            "capabilities": {
                "host_context_presence": "unsupported",
                "project_time_samples": "unsupported",
                "sample_accurate_automation": "unsupported",
                "presentation_latency": "unsupported",
                "aux_compare_pre": "unsupported",
                "aux_priority_sidechain": "unsupported",
                "contribution_aux": "unsupported",
                "float64_processing": "unsupported",
                "binary_telemetry": "unsupported",
                "remote_control": "unsupported"
            },
            "zaehler": {
                "frames_dropped": 0,
                "parse_errors": 0,
                "queue_overflows": 0
            },
            "runtime": {"messpunkt": "insert", "betrieb": "active"}
        }))
        .unwrap();
        let ack = c.p0_json(link_id, &payload).expect("heartbeat_ack");
        serde_json::from_slice::<Value>(&ack)
            .expect("heartbeat_ack ist JSON")
            .get("duplicate_instance_id")
            .and_then(Value::as_bool)
            .expect("duplicate_instance_id ist bool")
    }

    #[test]
    fn instance_alias_gleiche_id_in_verschiedenen_sitzungsadressraeumen_kollidiert_nicht() {
        let c = Coordinator::default();
        let a = adresse('a');
        let mut b = a.clone();
        b.session_epoch = "b".repeat(32);
        b.runtime_nonce = "b".repeat(32);

        c.control_registrieren("link-a", a);
        c.control_registrieren("link-b", b);

        assert!(c.dispatch_fuer_link_erlaubt("link-a"));
        assert!(c.dispatch_fuer_link_erlaubt("link-b"));
        assert!(!heartbeat_duplicate(&c, "link-a"));
        assert!(!heartbeat_duplicate(&c, "link-b"));
    }

    #[test]
    fn heartbeat_ack_meldet_alias_quarantaene_fuer_beide_links() {
        let c = Coordinator::default();
        let a = adresse('a');
        let mut b = a.clone();
        b.runtime_nonce = "b".repeat(32);

        c.control_registrieren("link-a", a);
        c.control_registrieren("link-b", b);

        assert!(!c.dispatch_fuer_link_erlaubt("link-a"));
        assert!(c.dispatch_fuer_link_erlaubt("link-b"));
        assert!(heartbeat_duplicate(&c, "link-a"));
        assert!(!c.dispatch_fuer_link_erlaubt("link-a"));
        assert!(!c.dispatch_fuer_link_erlaubt("link-b"));
        assert!(heartbeat_duplicate(&c, "link-b"));
    }

    #[test]
    fn hoermarkierung_v2_v3_gleicher_zustand() {
        let (c, a) = verbunden();
        c.hoermarkierung_v2("link-a", true);
        assert_eq!(c.interventionssicht().aktive, 1);
        assert!(!c.evidence_dispatch());
        c.hoermarkierung_v2("link-a", false);
        assert!(c.interventionssicht().starke_evidenz_erlaubt);

        assert!(c.intervention_begin("link-a", &a, &"1".repeat(32), 4));
        assert_eq!(c.interventionssicht().aktive, 1);
        assert!(!c.evidence_dispatch());
        assert!(c.intervention_end("link-a", &a, &"1".repeat(32), 5, 0));
        assert!(c.interventionssicht().starke_evidenz_erlaubt);
    }

    #[test]
    fn hoermarkierung_vor_evidence_dispatch() {
        let (c, a) = verbunden();
        assert!(c.intervention_begin("link-a", &a, &"1".repeat(32), 1));
        assert!(!c.evidence_dispatch());
        assert_eq!(c.interventionssicht().aktive, 1);
    }

    #[test]
    fn intervention_overflow_setzt_sticky_unknown() {
        let (c, _) = verbunden();
        c.hoermarkierung_v2("link-a", true);
        c.intervention_overflow();
        c.hoermarkierung_v2("link-a", false);
        assert!(c.interventionssicht().unknown);
        assert!(!c.evidence_dispatch());
    }

    #[test]
    fn intervention_control_disconnect_setzt_sticky_unknown() {
        let (c, _) = verbunden();
        c.hoermarkierung_v2("link-a", true);
        c.control_ende("link-a");
        c.hoermarkierung_v2("link-a", false);
        assert!(c.interventionssicht().unknown);
        assert!(!c.evidence_dispatch());
    }

    #[test]
    fn intervention_sequenzluecke_setzt_sticky_unknown() {
        let (c, a) = verbunden();
        assert!(c.intervention_begin("link-a", &a, &"1".repeat(32), 10));
        assert!(!c.intervention_end("link-a", &a, &"1".repeat(32), 12, 0));
        // Ein danach passend aussehendes End und v2-false duerfen die Luecke
        // nicht zur sauberen Baseline umluegen.
        assert!(c.intervention_end("link-a", &a, &"1".repeat(32), 11, 0));
        c.hoermarkierung_v2("link-a", false);
        assert!(c.interventionssicht().unknown);
        assert!(!c.evidence_dispatch());
    }

    #[test]
    fn intervention_neutral_resync_entsperrt() {
        let (c, a) = verbunden();
        assert!(c.intervention_begin("link-a", &a, &"1".repeat(32), 1));
        assert!(!c.intervention_end("link-a", &a, &"1".repeat(32), 3, 0));
        assert!(!c.evidence_dispatch());
        assert!(c.neutral_resync("link-a", 100));
        assert!(c.interventionssicht().starke_evidenz_erlaubt);
        assert!(c.evidence_dispatch());
    }

    #[test]
    fn intervention_ueberlappung_bleibt_bis_alle_end() {
        let (c, a) = verbunden();
        assert!(c.intervention_begin("link-a", &a, &"1".repeat(32), 1));
        assert!(c.intervention_begin("link-a", &a, &"2".repeat(32), 2));
        assert!(c.intervention_end("link-a", &a, &"1".repeat(32), 3, 0));
        assert_eq!(c.interventionssicht().aktive, 1);
        assert!(!c.evidence_dispatch());
        assert!(c.intervention_end("link-a", &a, &"2".repeat(32), 4, 0));
        assert!(c.interventionssicht().starke_evidenz_erlaubt);
    }

    #[test]
    fn intervention_falsche_end_id_bleibt_gesperrt() {
        let (c, a) = verbunden();
        assert!(c.intervention_begin("link-a", &a, &"1".repeat(32), 1));
        assert!(!c.intervention_end("link-a", &a, &"2".repeat(32), 2, 0));
        assert_eq!(c.interventionssicht().aktive, 1);
        assert!(!c.evidence_dispatch());
    }

    #[test]
    fn intervention_unbekanntes_erstes_end_setzt_sticky_unknown() {
        let (c, a) = verbunden();
        assert!(!c.intervention_end("link-a", &a, &"2".repeat(32), 1, 0));
        assert_eq!(c.interventionssicht().aktive, 0);
        assert!(c.interventionssicht().unknown);
        assert!(!c.evidence_dispatch());
        c.hoermarkierung_v2("link-a", false);
        assert!(c.interventionssicht().unknown);
    }

    #[test]
    fn hoermarkierung_tail_sperrt_bis_ende() {
        let (c, a) = verbunden();
        assert!(c.intervention_begin("link-a", &a, &"1".repeat(32), 1));
        assert!(c.intervention_end("link-a", &a, &"1".repeat(32), 2, 96_000));
        assert_eq!(c.interventionssicht().tail_samples_offen, 96_000);
        assert!(!c.evidence_dispatch());
        c.tail_fortschritt(95_999);
        assert!(!c.evidence_dispatch());
        c.tail_fortschritt(1);
        assert!(c.evidence_dispatch());
    }

    #[test]
    fn subscription_cleanup_vor_weiterem_push() {
        // Dieselbe atomare Operation gilt fuer jede Ursache, mit der der
        // Server einen Control-Link beendet. Die Transportintegration prueft
        // zusaetzlich den echten EOF-/Protokollpfad.
        for grund in [
            "EOF",
            "Protokollfehler",
            "Timeout",
            "Writefehler",
            "Serverstopp",
        ] {
            let c = Coordinator::default();
            let a = adresse('a');
            c.control_registrieren("link-a", a.clone());
            let payload = serde_json::json!({
                "type": "subscribe_session",
                "adresse": a,
                "session_epoch": "a".repeat(32)
            });
            assert!(
                c.subscribe_json("link-a", &serde_json::to_vec(&payload).unwrap()),
                "{grund}"
            );
            c.control_ende("link-a");
            assert!(
                c.session_push_ziele(&"a".repeat(32), &adresse('a'))
                    .is_empty(),
                "{grund}"
            );
            assert_eq!(c.subscription_cleanups(), 1, "{grund}");
        }
    }
}
