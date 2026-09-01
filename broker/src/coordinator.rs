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
const SESSION_COMMAND_REGISTER_MAX: usize = SESSION_CLIENT_CAP * SESSION_CLIENT_CAP;

const JSON_SCHEMA_MINOR_AKTIV: u8 = 1;

fn v3_schema_wurzel() -> Value {
    serde_json::from_str(include_str!(
        "../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
    ))
    .expect("eingefrorenes v3-Schema ist JSON")
}

fn v3_schema_minor_0_wurzel() -> Value {
    let mut wurzel = v3_schema_wurzel();
    {
        let defs = wurzel["$defs"]
            .as_object_mut()
            .expect("v3-$defs ist ein Objekt");
        for name in [
            "probe_descriptor_insert",
            "probe_descriptor_pre",
            "probe_descriptor_post",
            "probe_descriptor_beitrag",
        ] {
            let zweig = defs[name]
                .as_object_mut()
                .expect("Probe-Descriptor-Zweig ist ein Objekt");
            zweig["required"]
                .as_array_mut()
                .expect("Descriptor-required ist ein Array")
                .retain(|wert| wert.as_str() != Some("betrieb"));
            let props = zweig["properties"]
                .as_object_mut()
                .expect("Descriptor-properties ist ein Objekt");
            props.remove("betrieb");
            props.remove("host_bus_name");
            props.remove("host_mixer_index");
        }
        defs["heartbeat"]["properties"]
            .as_object_mut()
            .expect("Heartbeat-properties ist ein Objekt")
            .remove("runtime");
        defs["session_snapshot"]["properties"]
            .as_object_mut()
            .expect("Snapshot-properties ist ein Objekt")
            .remove("store_degraded");
        defs["session_snapshot"]["properties"]["mitglieder"]["items"] =
            serde_json::json!({"$ref": "#/$defs/probe_descriptor"});
        defs.remove("session_command");
    }
    wurzel["oneOf"]
        .as_array_mut()
        .expect("v3-oneOf ist ein Array")
        .retain(|zweig| {
            zweig.get("$ref").and_then(Value::as_str) != Some("#/$defs/session_command")
        });
    wurzel
}

fn v3_schema(schema_minor: u8) -> Option<&'static crate::vertrag::Schema> {
    static MINOR_0: OnceLock<crate::vertrag::Schema> = OnceLock::new();
    static MINOR_1: OnceLock<crate::vertrag::Schema> = OnceLock::new();
    match schema_minor {
        0 => Some(MINOR_0.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_minor_0_wurzel())
                .expect("eingefrorenes v3-Minor-0-Schema ist unterstuetzt")
        })),
        JSON_SCHEMA_MINOR_AKTIV => Some(MINOR_1.get_or_init(|| {
            crate::vertrag::Schema::laden(v3_schema_wurzel())
                .expect("eingefrorenes v3-Minor-1-Schema ist unterstuetzt")
        })),
        _ => None,
    }
}

fn v3_nachricht_lesen_mit_minor(
    payload: &[u8],
    erwarteter_typ: &str,
    schema_minor: u8,
) -> Option<Value> {
    let wert = v3_nachricht_lesen_beliebig_mit_minor(payload, schema_minor)?;
    (wert.get("type").and_then(Value::as_str) == Some(erwarteter_typ)).then_some(wert)
}

fn v3_nachricht_lesen_beliebig_mit_minor(payload: &[u8], schema_minor: u8) -> Option<Value> {
    crate::vertrag::textriegel_bytes(payload).ok()?;
    let wert: Value = serde_json::from_slice(payload).ok()?;
    v3_schema(schema_minor)?.gueltig(&wert).then_some(wert)
}

fn v3_nachricht_lesen(payload: &[u8], erwarteter_typ: &str) -> Option<Value> {
    v3_nachricht_lesen_mit_minor(payload, erwarteter_typ, JSON_SCHEMA_MINOR_AKTIV)
}

fn projektion_mit_aktuellem_lauf(gespeichert: &[u8], live: &[u8]) -> Option<Vec<u8>> {
    let mut persistiert = v3_nachricht_lesen(gespeichert, "session_snapshot")?;
    let live = v3_nachricht_lesen(live, "session_snapshot")?;
    let beitritt_bestaetigung_noetig =
        persistiert.get("beitritt_bestaetigung_noetig")?.as_bool()?
            || live.get("beitritt_bestaetigung_noetig")?.as_bool()?;
    let objekt = persistiert.as_object_mut()?;
    // Diese drei Felder gehoeren ausschliesslich zum aktuellen Brokerlauf.
    // Der Bestaetigungsbedarf hat dagegen zwei sichere Quellen: committierte
    // Projektion und fluechtiger Join-Graph. Nur bei zweimal false wird false
    // veroeffentlicht; persistiertes true loescht erst der regulaere C-03-Weg.
    // Der uebrige Projektionsschnitt bleibt die committierte Wirkung.
    for feld in ["broker_epoch", "fuehrendes_main", "mitglieder"] {
        objekt.insert(feld.into(), live.get(feld)?.clone());
    }
    objekt.insert(
        "beitritt_bestaetigung_noetig".into(),
        Value::Bool(beitritt_bestaetigung_noetig),
    );
    serde_json::to_vec(&persistiert).ok()
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
    session_ungebunden: bool,
    current_link: Option<String>,
    current_nonce: String,
    last_seen: Duration,
    stale: bool,
    stale_seit: Option<Duration>,
    join_kandidat: bool,
    bestaetigt: bool,
    explizit_bestaetigt: bool,
    ausdruecklich_ungebunden: bool,
    descriptor: Option<Value>,
    state_revision: Option<u64>,
    state_hash: Option<String>,
    record_state_valid: bool,
    recording: bool,
}

#[derive(Debug, Clone, Default)]
struct SessionStand {
    fuehrendes_main: Option<String>,
}

#[derive(Debug, Clone)]
struct LinkStand {
    /// Adresse, die der Peer auf dem Draht sendet. Bei einer Probe vor dem
    /// Join traegt sie den projektgebundenen Join-Marker.
    wire_adresse: Adresse,
    /// Effektive Adresse im Sessiongraphen. Nach eindeutigem Auto-Join traegt
    /// sie die vom Main erzeugte Session-Epoche.
    adresse: Adresse,
    client_key: ClientKey,
    alias_adressraum: Sitzungsadressraum,
    alias_besitzer: String,
    letzte_event_sequence: Option<u64>,
    verdraengt: bool,
    trennen: bool,
    join_neuverbinden: bool,
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

#[derive(Debug, Clone)]
struct LiveMessframe {
    adresse: Adresse,
    payload: Vec<u8>,
    empfangen: Duration,
    sequence: u64,
    sample_count: u32,
    sample_rate: f64,
}

#[derive(Debug, Clone, Default)]
struct Messfehler {
    anzahl: u64,
    aktuell: bool,
    letzter_grund: Option<P2RejectGrund>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum P2RejectGrund {
    FeatureBatchUngueltig,
    QuellframeAnzahlUngueltig,
    RoutingNichtFreigegeben,
    QuelladresseAbweichend,
    LautheitUngueltig,
}

impl P2RejectGrund {
    const fn wire(self) -> &'static str {
        match self {
            Self::FeatureBatchUngueltig => "feature_batch_ungueltig",
            Self::QuellframeAnzahlUngueltig => "quellframe_anzahl_ungueltig",
            Self::RoutingNichtFreigegeben => "routing_nicht_freigegeben",
            Self::QuelladresseAbweichend => "quelladresse_abweichend",
            Self::LautheitUngueltig => "lautheit_ungueltig",
        }
    }
}

const JSON_SAFE_INTEGER_MAX: u64 = 9_007_199_254_740_991;
#[cfg(test)]
const P2_REJECT_KATALOG: [P2RejectGrund; 5] = [
    P2RejectGrund::FeatureBatchUngueltig,
    P2RejectGrund::QuellframeAnzahlUngueltig,
    P2RejectGrund::RoutingNichtFreigegeben,
    P2RejectGrund::QuelladresseAbweichend,
    P2RejectGrund::LautheitUngueltig,
];

#[derive(Debug, Clone, Default)]
struct Lautheitsstand {
    zustand: Lautheitszustand,
    letztes_gueltiges_paar: Option<(f32, f32, Duration)>,
    ungueltig_anzahl: u64,
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
    messframes: HashMap<ClientKey, LiveMessframe>,
    messfehler: HashMap<ClientKey, Messfehler>,
    lautheit: HashMap<ClientKey, Lautheitsstand>,
    telemetry_links: HashSet<String>,
    telemetry_kopplungen: u64,
    conflict_guards: HashMap<String, HashSet<String>>,
    routing_bereit: bool,
    dirty_sessions: HashSet<SessionKey>,
    session_commands: HashMap<String, SessionCommandWirkung>,
    session_command_reihenfolge: VecDeque<String>,
}

#[derive(Debug, Clone)]
struct SessionCommandWirkung {
    kanonischer_auftrag: Vec<u8>,
    state_revision: u64,
    state_hash: String,
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
            messframes: HashMap::new(),
            messfehler: HashMap::new(),
            lautheit: HashMap::new(),
            telemetry_links: HashSet::new(),
            telemetry_kopplungen: 0,
            conflict_guards: HashMap::new(),
            routing_bereit: true,
            dirty_sessions: HashSet::new(),
            session_commands: HashMap::new(),
            session_command_reihenfolge: VecDeque::new(),
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
    /// Feste, begrenzte Shards serialisieren den Capture-/Commit-/Push-Pfad
    /// derselben Session. So kann ein spaeter gestarteter Flush seinen
    /// neueren Stand nie vor einem pausierten aelteren Flush committen.
    session_flush_schloesser: Vec<Mutex<()>>,
    flush_test_haken: Mutex<Option<CoordinatorFlushTestHaken>>,
}

const SESSION_FLUSH_SCHLOSS_ANZAHL: usize = 64;

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

impl Default for Coordinator {
    fn default() -> Self {
        Self::mit_uhr(
            Arc::new(InstantClock::default()),
            uuid::Uuid::new_v4().simple().to_string(),
        )
    }
}

impl Coordinator {
    fn store_degradiert(&self) -> bool {
        self.store
            .as_ref()
            .is_some_and(|store| store.sicht().degradiert)
    }

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

    fn subscription_abweisen(stand: &mut Stand, grund: &str) {
        stand.subscription_abweisungen = stand.subscription_abweisungen.saturating_add(1);
        stand.letzter_subscription_grund = grund.to_owned();
    }

    pub fn subscribe_json(&self, link_id: &str, payload: &[u8]) -> bool {
        self.subscribe_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
    }

    fn subscribe_json_mit_minor(&self, link_id: &str, payload: &[u8], schema_minor: u8) -> bool {
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
    fn messframes_an_subscriber_push(&self, link_id: &str) {
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        let Some(push) = push else { return };
        let stand = self.stand.lock().expect("Coordinator vergiftet");
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
        for (instance_id, payload) in frames {
            let _ = push.messframe_schreiben(link_id, &instance_id, &payload);
        }
    }

    /// Zweite Zielpruefung unmittelbar vor der externen Pipe-Arbeit. Die
    /// erste Ermittlung ist nur ein Kandidat: Eviction, Disconnect,
    /// Nonce-Verdraengung oder Aliasquarantaene koennen ihn danach entziehen.
    fn push_ziel_noch_gueltig(&self, link_id: &str, ziel: &SnapshotZiel) -> bool {
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

    pub fn messsicht(
        &self,
        project_binding_id: &str,
        session_epoch: &str,
        instance_id: &str,
    ) -> Option<MessframeSicht> {
        let jetzt = self.clock.jetzt();
        let stand = self.stand.lock().expect("Coordinator vergiftet");
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

    fn messframe_abweisen_locked(
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

    fn messframe_abweisen(&self, link_id: &str, grund: P2RejectGrund) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        let _ = Self::messframe_abweisen_locked(&mut stand, link_id, grund);
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
            for pending in Self::ungebundene_links_mit_eindeutigem_main_locked(&stand) {
                if let Some(link) = stand.links.get_mut(&pending) {
                    if !link.trennen {
                        link.trennen = true;
                        link.join_neuverbinden = true;
                    }
                }
                schliessen.insert(pending);
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
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
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

    fn state_report_json_mit_minor(&self, link_id: &str, payload: &[u8], schema_minor: u8) -> bool {
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
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
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
        let session = {
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
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
            Self::beitritt_bestaetigen_locked(&mut stand, &session, &key)
        };
        self.flush_session(&session, None);
        geaendert
    }

    fn beitritt_bestaetigen_locked(
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
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
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

    fn beitritt_aufheben_locked(
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

    fn flush_session(&self, session: &SessionKey, verursacher_link: Option<&str>) {
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

    fn session_flush_shard(&self, session: &SessionKey) -> usize {
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
            // Ein End ohne bekanntes Begin ist gerade KEIN sauberer
            // Neutralzustand: das Begin kann vor Reconnect/Overflow verloren
            // gegangen sein. Nur `neutral_resync` darf dieses Urteil loesen.
            stand.intervention_state_unknown = true;
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

    pub fn hoermarkierung_v2_getrennt(&self, link_id: &str) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        stand
            .interventionen
            .retain(|_, intervention| intervention.link_id != link_id);
        stand.intervention_state_unknown = true;
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

    fn store_verweigert_fuer_link(&self, link_id: &str) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
        if let Some(link) = stand.links.get_mut(link_id) {
            link.trennen = true;
        }
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

#[cfg(windows)]
impl crate::server::V2Interventionssenke for Coordinator {
    fn hoermarkierung(&self, link_id: &str, aktiv: bool) {
        Coordinator::hoermarkierung_v2(self, link_id, aktiv);
    }

    fn getrennt(&self, link_id: &str) {
        Coordinator::hoermarkierung_v2_getrennt(self, link_id);
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
        let neu = {
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
            let neu =
                stand.links.contains_key(link_id) && stand.telemetry_links.insert(link_id.into());
            if neu {
                stand.telemetry_kopplungen = stand.telemetry_kopplungen.saturating_add(1);
            }
            neu
        };
        if neu {
            self.messframes_an_subscriber_push(link_id);
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

    fn p0_mit_minor(&self, link_id: &str, schema_minor: u8, payload: &[u8]) -> Option<Vec<u8>> {
        self.p0_json_mit_minor(link_id, payload, schema_minor)
    }

    fn p1(&self, link_id: &str, payload: &[u8]) {
        self.p1_mit_minor(link_id, JSON_SCHEMA_MINOR_AKTIV, payload);
    }

    fn p1_mit_minor(&self, link_id: &str, schema_minor: u8, payload: &[u8]) {
        let typ = serde_json::from_slice::<Value>(payload)
            .ok()
            .and_then(|wert| wert.get("type").and_then(Value::as_str).map(str::to_owned));
        match typ.as_deref() {
            Some("subscribe_session") => {
                let _ = self.subscribe_json_mit_minor(link_id, payload, schema_minor);
            }
            Some("state_report") => {
                let _ = self.state_report_json_mit_minor(link_id, payload, schema_minor);
            }
            _ => {}
        }
    }

    fn p2(&self, link_id: &str, payload: &[u8]) {
        let batch = match crate::telemetrie::fuer_broker(payload) {
            Ok(batch) => batch,
            Err(_) => {
                self.messframe_abweisen(link_id, P2RejectGrund::FeatureBatchUngueltig);
                return;
            }
        };
        if batch.frames.len() != 1 {
            self.messframe_abweisen(link_id, P2RejectGrund::QuellframeAnzahlUngueltig);
            return;
        }
        let frame = &batch.frames[0];
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        {
            let mut stand = self.stand.lock().expect("Coordinator vergiftet");
            let Some(key) = Self::aktueller_telemetrie_client_locked(&stand, link_id) else {
                return;
            };
            let Some(link) = stand.links.get(link_id) else {
                return;
            };
            if !self.dispatch_fuer_link_erlaubt_locked(&stand, link) {
                let _ = Self::messframe_abweisen_locked(
                    &mut stand,
                    link_id,
                    P2RejectGrund::RoutingNichtFreigegeben,
                );
                return;
            }
            if frame.adresse != link.adresse {
                let _ = Self::messframe_abweisen_locked(
                    &mut stand,
                    link_id,
                    P2RejectGrund::QuelladresseAbweichend,
                );
                return;
            }

            let jetzt = self.clock.jetzt();
            let lautheit_ungueltig = batch
                .lautheit_ungueltige_instance_ids
                .iter()
                .any(|instance_id| instance_id == &key.instance_id);
            let lautheit = stand.lautheit.entry(key.clone()).or_default();
            if lautheit_ungueltig {
                lautheit.zustand = Lautheitszustand::Invalid;
                lautheit.ungueltig_anzahl = lautheit.ungueltig_anzahl.saturating_add(1);
            } else if let Some((lufs_i, unsicherheit)) = frame.lufs_i_paar {
                lautheit.zustand = Lautheitszustand::Paar;
                lautheit.letztes_gueltiges_paar = Some((lufs_i, unsicherheit, jetzt));
            } else {
                match frame.lufs_i_status {
                    Some(1) => lautheit.zustand = Lautheitszustand::Collecting,
                    Some(2) => lautheit.zustand = Lautheitszustand::Gated,
                    _ => {}
                }
            }

            {
                let fehler = stand.messfehler.entry(key.clone()).or_default();
                if lautheit_ungueltig {
                    fehler.anzahl = fehler.anzahl.saturating_add(1).min(JSON_SAFE_INTEGER_MAX);
                    fehler.letzter_grund = Some(P2RejectGrund::LautheitUngueltig);
                }
                // Der restliche Frame ist nach Entfernung der drei LUFS-I-Felder
                // vollstaendig gueltig. Nur eine vollstaendige Ablehnung setzt die
                // allgemeine Messachse auf invalid.
                fehler.aktuell = false;
            }
            if lautheit_ungueltig {
                let session = key.session();
                stand.dirty_sessions.insert(session);
            }
            stand.messframes.insert(
                key.clone(),
                LiveMessframe {
                    adresse: frame.adresse.clone(),
                    payload: batch.payload.clone(),
                    empfangen: jetzt,
                    sequence: frame.sequence,
                    sample_count: frame.sample_count,
                    sample_rate: frame.sample_rate,
                },
            );
            stand.p2_live_frames = stand.p2_live_frames.saturating_add(1);

            if let Some(push) = push.as_ref() {
                let session = key.session();
                let mut ziele = stand
                    .subscriptions
                    .iter()
                    .filter(|(ziel_link_id, sub)| {
                        sub.session_epoch == session.session_epoch
                            && sub.adresse.project_binding_id == session.project_binding_id
                            && stand.links.get(*ziel_link_id).is_some_and(|ziel_link| {
                                !ziel_link.trennen
                                    && stand.routing_bereit
                                    && self.alias_register.session_push_erlaubt(
                                        &ziel_link.alias_adressraum,
                                        &ziel_link.alias_besitzer,
                                        &ziel_link.adresse.instance_id,
                                    )
                            })
                    })
                    .map(|(ziel_link_id, _)| ziel_link_id.clone())
                    .collect::<Vec<_>>();
                ziele.sort();
                for ziel_link_id in ziele {
                    let _ =
                        push.messframe_schreiben(&ziel_link_id, &key.instance_id, &batch.payload);
                }
            }
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

    #[test]
    fn p2_reject_katalog_stimmt_mit_dem_strikten_wire_schema() {
        let schema: Value = serde_json::from_str(include_str!(
            "../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
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
