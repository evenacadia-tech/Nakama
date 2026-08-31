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
use std::collections::{HashMap, HashSet};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, Mutex, OnceLock};
use std::time::{Duration, Instant};

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

fn v3_schema() -> &'static crate::vertrag::Schema {
    static SCHEMA: OnceLock<crate::vertrag::Schema> = OnceLock::new();
    SCHEMA.get_or_init(|| {
        let wurzel = serde_json::from_str(include_str!(
            "../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
        ))
        .expect("eingefrorenes v3-Schema ist JSON");
        crate::vertrag::Schema::laden(wurzel).expect("eingefrorenes v3-Schema ist unterstuetzt")
    })
}

fn v3_nachricht_lesen(payload: &[u8], erwarteter_typ: &str) -> Option<Value> {
    crate::vertrag::textriegel_bytes(payload).ok()?;
    let wert: Value = serde_json::from_slice(payload).ok()?;
    (wert.get("type").and_then(Value::as_str) == Some(erwarteter_typ) && v3_schema().gueltig(&wert))
        .then_some(wert)
}

pub trait MonotonicClock: Send + Sync {
    fn jetzt(&self) -> Duration;
}

#[derive(Debug)]
struct InstantClock {
    start: Instant,
}

impl Default for InstantClock {
    fn default() -> Self {
        Self {
            start: Instant::now(),
        }
    }
}

impl MonotonicClock for InstantClock {
    fn jetzt(&self) -> Duration {
        self.start.elapsed()
    }
}

/// Schlaflose Testuhr. Produktion konstruiert den Coordinator ausschliesslich
/// mit `InstantClock`.
#[doc(hidden)]
#[derive(Debug, Default)]
pub struct ManualClock {
    millis: AtomicU64,
}

impl ManualClock {
    pub fn setze_ms(&self, millis: u64) {
        self.millis.store(millis, Ordering::SeqCst);
    }

    pub fn vor(&self, millis: u64) {
        self.millis.fetch_add(millis, Ordering::SeqCst);
    }
}

impl MonotonicClock for ManualClock {
    fn jetzt(&self) -> Duration {
        Duration::from_millis(self.millis.load(Ordering::SeqCst))
    }
}

pub trait SessionPush: Send + Sync {
    /// `true` bedeutet ausschliesslich: der volle Snapshot wurde auf die Pipe
    /// geschrieben. Es ist keine Empfängerwirkung und kein Wire-ACK.
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool;
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct ClientKey {
    logon_sid: String,
    project_binding_id: String,
    session_epoch: String,
    instance_id: String,
}

impl ClientKey {
    fn aus_adresse(adresse: &Adresse) -> Self {
        Self {
            logon_sid: adresse.logon_sid.clone(),
            project_binding_id: adresse.project_binding_id.clone(),
            session_epoch: adresse.session_epoch.clone(),
            instance_id: adresse.instance_id.clone(),
        }
    }

    fn session(&self) -> SessionKey {
        SessionKey {
            project_binding_id: self.project_binding_id.clone(),
            session_epoch: self.session_epoch.clone(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct SessionKey {
    project_binding_id: String,
    session_epoch: String,
}

#[derive(Debug, Clone)]
struct ClientStand {
    adresse: Adresse,
    plugin_kind: String,
    host_pid: Option<u32>,
    current_link: Option<String>,
    current_nonce: String,
    last_seen: Duration,
    stale: bool,
    stale_seit: Option<Duration>,
    join_kandidat: bool,
    bestaetigt: bool,
    explizit_bestaetigt: bool,
    descriptor: Option<Value>,
}

#[derive(Debug, Clone, Default)]
struct SessionStand {
    fuehrendes_main: Option<String>,
}

#[derive(Debug, Clone)]
struct LinkStand {
    adresse: Adresse,
    client_key: ClientKey,
    alias_adressraum: Sitzungsadressraum,
    alias_besitzer: String,
    letzte_event_sequence: Option<u64>,
    verdraengt: bool,
    trennen: bool,
}

#[derive(Debug, Clone)]
struct Subscription {
    adresse: Adresse,
    session_epoch: String,
}

#[derive(Debug, Clone)]
struct Intervention {
    link_id: String,
}

#[derive(Debug)]
struct Stand {
    links: HashMap<String, LinkStand>,
    clients: HashMap<ClientKey, ClientStand>,
    sessions: HashMap<SessionKey, SessionStand>,
    subscriptions: HashMap<String, Subscription>,
    interventionen: HashMap<String, Intervention>,
    intervention_state_unknown: bool,
    tail_samples_offen: u64,
    subscription_cleanups: u64,
    subscription_abweisungen: u64,
    letzter_subscription_grund: String,
    evidence_angenommen: u64,
    evidence_gesperrt: u64,
    cap_abweisungen: u64,
    store_verweigerungen: u64,
    p2_live_frames: u64,
    telemetry_links: HashSet<String>,
    telemetry_kopplungen: u64,
    conflict_guards: HashMap<String, HashSet<String>>,
    routing_bereit: bool,
    dirty_sessions: HashSet<SessionKey>,
}

impl Default for Stand {
    fn default() -> Self {
        Self {
            links: HashMap::new(),
            clients: HashMap::new(),
            sessions: HashMap::new(),
            subscriptions: HashMap::new(),
            interventionen: HashMap::new(),
            intervention_state_unknown: false,
            tail_samples_offen: 0,
            subscription_cleanups: 0,
            subscription_abweisungen: 0,
            letzter_subscription_grund: String::new(),
            evidence_angenommen: 0,
            evidence_gesperrt: 0,
            cap_abweisungen: 0,
            store_verweigerungen: 0,
            p2_live_frames: 0,
            telemetry_links: HashSet::new(),
            telemetry_kopplungen: 0,
            conflict_guards: HashMap::new(),
            routing_bereit: true,
            dirty_sessions: HashSet::new(),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Interventionssicht {
    pub aktive: usize,
    pub tail_samples_offen: u64,
    pub unknown: bool,
    pub starke_evidenz_erlaubt: bool,
}

pub struct Coordinator {
    stand: Mutex<Stand>,
    alias_register: AliasRegister,
    clock: Arc<dyn MonotonicClock>,
    store: Option<StoreHandle>,
    broker_epoch: String,
    event_sequence: AtomicU64,
    push: Mutex<Option<Arc<dyn SessionPush>>>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ControlRegistrierung {
    pub angenommen: bool,
    pub grund: Option<String>,
    pub zu_schliessende_links: Vec<String>,
}

impl ControlRegistrierung {
    fn angenommen(zu_schliessende_links: Vec<String>) -> Self {
        Self {
            angenommen: true,
            grund: None,
            zu_schliessende_links,
        }
    }

    fn abgewiesen(grund: impl Into<String>) -> Self {
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

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SessionModellSicht {
    pub project_binding_id: String,
    pub session_epoch: String,
    pub fuehrendes_main: Option<String>,
    pub beitritt_bestaetigung_noetig: bool,
    pub clients: Vec<ClientModellSicht>,
}

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

    pub fn control_hello_registrieren(
        &self,
        link_id: &str,
        hello: &HelloControl,
    ) -> ControlRegistrierung {
        let jetzt = self.clock.jetzt();
        let adresse = hello.adresse.clone();
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

        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
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
            host_pid: hello.host.as_ref().map(|h| h.pid),
            current_link: Some(link_id.to_owned()),
            current_nonce: adresse.runtime_nonce.clone(),
            last_seen: jetzt,
            stale: false,
            stale_seit: None,
            join_kandidat: geerbt.as_ref().is_some_and(|c| c.join_kandidat),
            bestaetigt: geerbt.as_ref().is_some_and(|c| c.bestaetigt),
            explizit_bestaetigt: geerbt.as_ref().is_some_and(|c| c.explizit_bestaetigt),
            descriptor: geerbt.and_then(|c| c.descriptor),
        };
        stand.clients.insert(key.clone(), client);
        stand.sessions.entry(key.session()).or_default();
        stand.links.insert(
            link_id.to_owned(),
            LinkStand {
                adresse,
                client_key: key,
                alias_adressraum,
                alias_besitzer,
                letzte_event_sequence: None,
                verdraengt: false,
                trennen: false,
            },
        );
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
            if let Some(client) = stand.clients.get_mut(&link.client_key) {
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
        stand.intervention_state_unknown = true;
    }

    fn subscription_abweisen(stand: &mut Stand, grund: &str) {
        stand.subscription_abweisungen = stand.subscription_abweisungen.saturating_add(1);
        stand.letzter_subscription_grund = grund.to_owned();
    }

    pub fn subscribe_json(&self, link_id: &str, payload: &[u8]) -> bool {
        let Ok(wert) = serde_json::from_slice::<Value>(payload) else {
            return false;
        };
        if wert.get("type").and_then(Value::as_str) != Some("subscribe_session") {
            return false;
        }
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
        if link.adresse != adresse {
            Self::subscription_abweisen(&mut stand, "subscribe: fremde effektive Adresse");
            return false;
        }
        if session_epoch != link.adresse.session_epoch || session_epoch != adresse.session_epoch {
            Self::subscription_abweisen(&mut stand, "subscribe: fremde Session");
            return false;
        }
        let session = ClientKey::aus_adresse(&adresse).session();
        stand.subscriptions.insert(
            link_id.to_owned(),
            Subscription {
                adresse,
                session_epoch: session_epoch.to_owned(),
            },
        );
        drop(stand);
        self.resubscribe_snapshot_push(&session, link_id);
        true
    }

    fn resubscribe_snapshot_push(&self, session: &SessionKey, link_id: &str) {
        let (live_payload, ziel) = {
            let stand = self.stand.lock().expect("Coordinator vergiftet");
            let Some(sub) = stand.subscriptions.get(link_id) else {
                return;
            };
            let Some(link) = stand.links.get(link_id) else {
                return;
            };
            if link.trennen
                || !stand.routing_bereit
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
        let projektion = self.store.as_ref().and_then(|store| {
            store
                .session_state_lesen(&session.project_binding_id, &session.session_epoch)
                .ok()
                .flatten()
        });
        let (gedeckt_bis, payload) = projektion
            .map(|(ord, payload)| (Some(ord), payload))
            .unwrap_or((None, live_payload));
        if v3_nachricht_lesen(&payload, "session_snapshot").is_none() {
            // Eine beschaedigte Projektion darf nie als scheinbar gueltiger
            // Wirezustand austreten. Das ist Storedegradation, nicht ein
            // Anlass, einen leeren oder erfundenen Snapshot zu senden.
            self.routing_fail_closed("Sessionprojektion verletzt v3-Vertrag");
            return;
        }
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        let geschrieben = push
            .as_ref()
            .is_some_and(|push| push.snapshot_schreiben(link_id, &payload));
        if geschrieben {
            if let (Some(store), Some(ord)) = (&self.store, gedeckt_bis) {
                let _ = store.snapshot_schuld_kompaktieren(ziel, ord);
            }
        }
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

    pub fn verbindung_soll_trennen(&self, link_id: &str) -> bool {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .links
            .get(link_id)
            .is_some_and(|link| link.trennen)
    }

    pub fn routing_bereit(&self) -> bool {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .routing_bereit
    }

    pub fn cap_abweisungen(&self) -> u64 {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .cap_abweisungen
    }

    pub fn store_verweigerungen(&self) -> u64 {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .store_verweigerungen
    }

    pub fn p2_live_frames(&self) -> u64 {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .p2_live_frames
    }

    pub fn telemetrie_kopplungen(&self) -> (usize, u64) {
        let stand = self.stand.lock().expect("Coordinator vergiftet");
        (stand.telemetry_links.len(), stand.telemetry_kopplungen)
    }

    pub fn client_anzahl(&self) -> usize {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .clients
            .values()
            .filter(|client| client.current_link.is_some())
            .count()
    }

    pub fn subscription_anzahl(&self) -> usize {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .subscriptions
            .len()
    }

    pub fn subscription_cleanups(&self) -> u64 {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .subscription_cleanups
    }

    pub fn subscription_abweisungen(&self) -> u64 {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .subscription_abweisungen
    }

    pub fn letzter_subscription_grund(&self) -> String {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .letzter_subscription_grund
            .clone()
    }

    fn adresse_des_links_passt(stand: &Stand, link_id: &str, adresse: &Adresse) -> bool {
        stand
            .links
            .get(link_id)
            .is_some_and(|link| &link.adresse == adresse)
    }

    fn subscription_entfernen_locked(stand: &mut Stand, link_id: &str) {
        if stand.subscriptions.remove(link_id).is_some() {
            stand.subscription_cleanups = stand.subscription_cleanups.saturating_add(1);
        }
    }

    fn routing_fail_closed(&self, _grund: &str) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        stand.routing_bereit = false;
        for link in stand.links.values_mut() {
            link.trennen = true;
        }
    }

    fn platz_schaffen_locked(
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
        Ok(())
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
        stand.dirty_sessions.insert(session);
        schliessen
    }

    fn stale_aktualisieren_locked(stand: &mut Stand, jetzt: Duration) {
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
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
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
        let session;
        let aktiv = {
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
            let Some(link) = stand.links.get(link_id).cloned() else {
                return false;
            };
            session = link.client_key.session();
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
                    if stand
                        .conflict_guards
                        .entry(effective.clone())
                        .or_default()
                        .insert(derived_id.clone())
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
                false
            } else {
                let plugin_kind = stand
                    .clients
                    .get(&link.client_key)
                    .map(|client| client.plugin_kind.clone())
                    .unwrap_or_default();
                let descriptor =
                    wert.and_then(|v| Self::descriptor_aus_heartbeat(&link, &plugin_kind, v));
                if let Some(client) = stand.clients.get_mut(&link.client_key) {
                    if client.current_link.as_deref() != Some(link_id) {
                        return false;
                    }
                    client.last_seen = jetzt;
                    client.stale = false;
                    client.stale_seit = None;
                    client.join_kandidat = true;
                    if descriptor.is_some() {
                        client.descriptor = descriptor;
                    }
                }
                Self::auto_join_locked(&mut stand, &link.client_key);
                stand.dirty_sessions.insert(session.clone());
                true
            }
        };
        self.guards_persistieren(guards);
        self.flush_session(&session, Some(link_id));
        aktiv
    }

    /// Ein schemafestes `state_report` ist neben dem Heartbeat ein gueltiger
    /// Liveness-Kontakt (E-01/E-02). Es traegt in Phase B keinen neuen
    /// MainProjectState-Ingress: fachlich wirkt er nur auf den schon im Hello
    /// gebundenen Client und durchlaeuft dieselbe Join-Regel wie ein Heartbeat.
    pub fn state_report_json(&self, link_id: &str, payload: &[u8]) -> bool {
        let Some(wert) = v3_nachricht_lesen(payload, "state_report") else {
            return false;
        };
        let Ok(adresse) = serde_json::from_value::<Adresse>(wert["adresse"].clone()) else {
            return false;
        };
        if !self
            .stand
            .lock()
            .expect("Coordinator vergiftet")
            .links
            .get(link_id)
            .is_some_and(|link| link.adresse == adresse)
        {
            return false;
        }
        self.heartbeat_kontakt(link_id, None)
    }

    fn descriptor_aus_heartbeat(
        link: &LinkStand,
        plugin_kind: &str,
        wert: &Value,
    ) -> Option<Value> {
        let capabilities = wert.get("capabilities")?.clone();
        if !matches!(
            plugin_kind,
            "main" | "active_probe" | "passive_probe" | "legacy"
        ) {
            return None;
        }
        Some(serde_json::json!({
            "adresse": link.adresse,
            "plugin_kind": plugin_kind,
            "measurement_position": "insert",
            "aussageklasse": "beobachtend",
            "label": "",
            "capabilities": capabilities,
            "frische": {"stale": false, "letzter_kontakt_ms": 0}
        }))
    }

    pub fn descriptor_setzen(&self, link_id: &str, descriptor: Value) -> bool {
        let session = {
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
            let Some(link) = stand.links.get(link_id).cloned() else {
                return false;
            };
            let Ok(adresse) = serde_json::from_value::<Adresse>(descriptor["adresse"].clone())
            else {
                return false;
            };
            if adresse != link.adresse {
                return false;
            }
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

    fn auto_join_locked(stand: &mut Stand, key: &ClientKey) {
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
                if !client.explizit_bestaetigt {
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

    fn fuehrung_neu_bewerten_locked(stand: &mut Stand, session: &SessionKey) {
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
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
            let key = stand
                .clients
                .keys()
                .find(|key| &key.session() == &session && key.instance_id == instance_id)
                .cloned();
            let Some(key) = key else { return false };
            let client = stand.clients.get_mut(&key).expect("Clientschluessel");
            let geaendert = !client.bestaetigt;
            client.join_kandidat = true;
            client.bestaetigt = true;
            client.explizit_bestaetigt = true;
            Self::fuehrung_neu_bewerten_locked(&mut stand, &session);
            stand.dirty_sessions.insert(session.clone());
            geaendert
        };
        self.flush_session(&session, None);
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
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
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
        let stand = self.stand.lock().expect("Coordinator vergiftet");
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

    fn beitritt_noetig_locked(stand: &Stand, session: &SessionKey) -> bool {
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
        let stand = self.stand.lock().expect("Coordinator vergiftet");
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
        let stand = self.stand.lock().expect("Coordinator vergiftet");
        self.snapshot_locked(&stand, &session)
    }

    fn snapshot_locked(&self, stand: &Stand, session: &SessionKey) -> Vec<u8> {
        let jetzt = self.clock.jetzt();
        let mut mitglieder: Vec<(String, Value)> = stand
            .clients
            .iter()
            .filter(|(key, _)| &key.session() == session)
            .filter_map(|(_, client)| {
                let mut descriptor = client.descriptor.clone()?;
                let objekt = descriptor.as_object_mut()?;
                objekt.insert(
                    "adresse".into(),
                    serde_json::to_value(&client.adresse).ok()?,
                );
                objekt.insert(
                    "plugin_kind".into(),
                    Value::String(client.plugin_kind.clone()),
                );
                objekt.insert(
                    "frische".into(),
                    serde_json::json!({
                        "stale": client.stale,
                        "letzter_kontakt_ms": jetzt
                            .saturating_sub(client.last_seen)
                            .as_millis()
                            .min(u64::MAX as u128) as u64
                    }),
                );
                Some((effektive_adresse(&client.adresse), descriptor))
            })
            .collect();
        mitglieder.sort_by(|a, b| a.0.cmp(&b.0));
        let payload = serde_json::json!({
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
        serde_json::to_vec(&payload).unwrap_or_else(|_| b"{}".to_vec())
    }

    fn flush_session(&self, session: &SessionKey, verursacher_link: Option<&str>) {
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

        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        for (link_id, ziel) in ziele {
            let geschrieben = push
                .as_ref()
                .is_some_and(|push| push.snapshot_schreiben(&link_id, &payload));
            if geschrieben {
                if let (Some(store), Some(ord)) = (&self.store, event_ord) {
                    let _ = store.snapshot_schuld_kompaktieren(ziel, ord);
                }
            }
        }
    }

    fn guards_persistieren(&self, guards: Vec<ConflictGuard>) {
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

    /// `false` bedeutet Luecke oder unbekannter Link. Das Sticky-Bit ist dann
    /// bereits gesetzt. Die normale Ereignisfolge kann es nicht loeschen.
    fn sequenz_annehmen(stand: &mut Stand, link_id: &str, sequence: u64) -> bool {
        let Some(link) = stand.links.get_mut(link_id) else {
            stand.intervention_state_unknown = true;
            return false;
        };
        if let Some(letzte) = link.letzte_event_sequence {
            if letzte.checked_add(1) != Some(sequence) {
                stand.intervention_state_unknown = true;
                return false;
            }
        }
        link.letzte_event_sequence = Some(sequence);
        true
    }

    pub fn intervention_begin(
        &self,
        link_id: &str,
        adresse: &Adresse,
        intervention_id: &str,
        event_sequence: u64,
    ) -> bool {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        if !Self::adresse_des_links_passt(&stand, link_id, adresse)
            || !Self::sequenz_annehmen(&mut stand, link_id, event_sequence)
        {
            stand.intervention_state_unknown = true;
            return false;
        }
        if stand.interventionen.len() >= MAX_AKTIVE_INTERVENTIONEN
            || stand.interventionen.contains_key(intervention_id)
        {
            stand.intervention_state_unknown = true;
            return false;
        }
        stand.interventionen.insert(
            intervention_id.to_owned(),
            Intervention {
                link_id: link_id.to_owned(),
            },
        );
        true
    }

    pub fn intervention_end(
        &self,
        link_id: &str,
        adresse: &Adresse,
        intervention_id: &str,
        event_sequence: u64,
        tail_samples: u64,
    ) -> bool {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        if !Self::adresse_des_links_passt(&stand, link_id, adresse)
            || !Self::sequenz_annehmen(&mut stand, link_id, event_sequence)
        {
            stand.intervention_state_unknown = true;
            return false;
        }
        let passt = stand
            .interventionen
            .get(intervention_id)
            .is_some_and(|i| i.link_id == link_id);
        if !passt {
            return false;
        }
        stand.interventionen.remove(intervention_id);
        stand.tail_samples_offen = stand.tail_samples_offen.max(tail_samples);
        true
    }

    /// Legacy-v2 und v3 teilen denselben `interventionen`-Bestand. Das
    /// synthetische v2-Schluesselwort kann nicht mit einer hex32-v3-ID
    /// kollidieren. `false` entfernt nur den sauber bekannten v2-Eintrag und
    /// loescht niemals Unknown.
    pub fn hoermarkierung_v2(&self, link_id: &str, aktiv: bool) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        let id = format!("v2:{link_id}");
        if aktiv {
            if stand.interventionen.len() >= MAX_AKTIVE_INTERVENTIONEN {
                stand.intervention_state_unknown = true;
            } else {
                stand.interventionen.insert(
                    id,
                    Intervention {
                        link_id: link_id.to_owned(),
                    },
                );
            }
        } else {
            stand.interventionen.remove(&id);
        }
    }

    pub fn intervention_overflow(&self) {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .intervention_state_unknown = true;
    }

    pub fn tail_fortschritt(&self, samples: u64) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        stand.tail_samples_offen = stand.tail_samples_offen.saturating_sub(samples);
    }

    /// Einziger Entsperrweg fuer Unknown. Er ist eine explizite Coordinator-
    /// Lebenszyklusoperation, kein normales End/False und keine neue
    /// Wirefamilie. Die bestaetigte Basis wird fuer den Link gesetzt; der
    /// neutrale Zustand leert aktive IDs und Nachlauf gemeinsam.
    pub fn neutral_resync(&self, link_id: &str, bestaetigte_sequence_basis: u64) -> bool {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        let Some(link) = stand.links.get_mut(link_id) else {
            return false;
        };
        link.letzte_event_sequence = Some(bestaetigte_sequence_basis);
        stand.interventionen.clear();
        stand.tail_samples_offen = 0;
        stand.intervention_state_unknown = false;
        true
    }

    pub fn interventionssicht(&self) -> Interventionssicht {
        let stand = self.stand.lock().expect("Coordinator vergiftet");
        let erlaubt = !stand.intervention_state_unknown
            && stand.interventionen.is_empty()
            && stand.tail_samples_offen == 0;
        Interventionssicht {
            aktive: stand.interventionen.len(),
            tail_samples_offen: stand.tail_samples_offen,
            unknown: stand.intervention_state_unknown,
            starke_evidenz_erlaubt: erlaubt,
        }
    }

    pub fn dispatch_fuer_link_erlaubt(&self, link_id: &str) -> bool {
        let stand = self.stand.lock().expect("Coordinator vergiftet");
        stand
            .links
            .get(link_id)
            .is_some_and(|link| self.dispatch_fuer_link_erlaubt_locked(&stand, link))
    }

    fn dispatch_fuer_link_erlaubt_locked(&self, stand: &Stand, link: &LinkStand) -> bool {
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

    fn alias_quarantaenisiert(&self, link_id: &str) -> bool {
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

    /// Der Riegel liegt VOR dem Evidence-Commit und zaehlt das Urteil unter
    /// demselben Lock. So kann ein Begin nicht erst nach dem Dispatch wirken.
    pub fn evidence_dispatch(&self) -> bool {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        let erlaubt = !stand.intervention_state_unknown
            && stand.interventionen.is_empty()
            && stand.tail_samples_offen == 0;
        if erlaubt {
            stand.evidence_angenommen = stand.evidence_angenommen.saturating_add(1);
        } else {
            stand.evidence_gesperrt = stand.evidence_gesperrt.saturating_add(1);
        }
        erlaubt
    }

    fn p0_json(&self, link_id: &str, payload: &[u8]) -> Option<Vec<u8>> {
        let wert: Value = serde_json::from_slice(payload).ok()?;
        match wert.get("type")?.as_str()? {
            "heartbeat" => {
                let adresse: Adresse = serde_json::from_value(wert["adresse"].clone()).ok()?;
                if !self
                    .stand
                    .lock()
                    .expect("Coordinator vergiftet")
                    .links
                    .get(link_id)
                    .is_some_and(|link| link.adresse == adresse)
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
                if wert.get("art").and_then(Value::as_str) != Some("hoermarkierung") {
                    return None;
                }
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
            _ => None,
        }
    }
}

#[cfg(windows)]
impl crate::transport::server_v3::Senke for Coordinator {
    fn control_verbunden(
        &self,
        link_id: &str,
        hello: &HelloControl,
    ) -> crate::transport::server_v3::ControlAnmeldung {
        let ausgang = self.control_hello_registrieren(link_id, hello);
        crate::transport::server_v3::ControlAnmeldung {
            angenommen: ausgang.angenommen,
            grund: ausgang.grund,
            zu_schliessende_links: ausgang.zu_schliessende_links,
        }
    }

    fn control_schliesst(&self, link_id: &str) {
        self.control_ende(link_id);
    }

    fn control_getrennt(&self, _link_id: &str) {}
    fn telemetrie_gekoppelt(&self, link_id: &str) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        if stand.links.contains_key(link_id) && stand.telemetry_links.insert(link_id.into()) {
            stand.telemetry_kopplungen = stand.telemetry_kopplungen.saturating_add(1);
        }
    }
    fn telemetrie_getrennt(&self, link_id: &str) {
        self.stand
            .lock()
            .expect("Coordinator vergiftet")
            .telemetry_links
            .remove(link_id);
    }

    fn p0(&self, link_id: &str, payload: &[u8]) -> Option<Vec<u8>> {
        self.p0_json(link_id, payload)
    }

    fn p1(&self, link_id: &str, payload: &[u8]) {
        let typ = serde_json::from_slice::<Value>(payload)
            .ok()
            .and_then(|wert| wert.get("type").and_then(Value::as_str).map(str::to_owned));
        match typ.as_deref() {
            Some("subscribe_session") => {
                let _ = self.subscribe_json(link_id, payload);
            }
            Some("state_report") => {
                let _ = self.state_report_json(link_id, payload);
            }
            _ => {}
        }
    }

    fn p2(&self, link_id: &str, _payload: &[u8]) {
        {
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
            stand.p2_live_frames = stand.p2_live_frames.saturating_add(1);
        }
        if self.dispatch_fuer_link_erlaubt(link_id) {
            let _ = self.evidence_dispatch();
        }
    }

    fn abgewiesen(&self, _grund: &str) {}
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
    use super::*;

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
            "sequence": 1
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
