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
    ConflictGuard, SnapshotZiel, StoreEvent, StoreHandle, StoreWriter, MAX_KONFLIKT_GUARDS,
    STORE_SCHEMA_MAJOR,
};
use crate::transport::bootstrap::{Adresse, AudioLage, HelloControl};
use serde_json::Value;
use sha2::{Digest, Sha256};
use std::collections::{HashMap, HashSet, VecDeque};
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::{Arc, Condvar, Mutex, OnceLock};
use std::time::Duration;

mod befehl;
mod evidenz;
mod flush;
mod intervention;
mod link;
mod liveness;
mod mitgliedschaft;
mod schema;
mod senke;
mod sicht;
mod subscription;
mod uhr;
pub mod prepost;
pub mod vergleichbarkeit;
mod zustand;

pub use sicht::{
    ClientModellSicht, ControlRegistrierung, Interventionssicht, Lautheitszustand,
    MessframeSicht, SessionModellSicht,
};
pub use evidenz::Evidenzstand;
/// Die aktive JSON-Vertragsfassung, nur fuer Beine sichtbar.
///
/// Der Transport (`server_v3`) und der JSON-Leser muessen dieselbe
/// Fassung fuehren; laufen sie auseinander, weist der Server Rahmen ab,
/// die der Coordinator lesen koennte. Damit das MESSBAR ist und nicht
/// nur kommentiert, ist die Zahl hier lesbar.
pub const JSON_SCHEMA_MINOR_AKTIV_FUER_TEST: u8 = schema::JSON_SCHEMA_MINOR_AKTIV;
pub use uhr::{ManualClock, MonotonicClock};
use zustand::{
    kollisionsriegel_setzen_locked, ClientKey, ClientStand, Deckelausgang, Intervention, LinkStand,
    LiveMessframe, P2RejectGrund, SessionCommandWirkung, SessionKey, Stand, Subscription,
    JSON_SAFE_INTEGER_MAX,
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
/// H-12: Obergrenze der Sessionmap. Eine Session hat immer mindestens einen
/// Client, es kann also nie mehr Sessions als Clients geben - die Grenze ist
/// dieselbe Zahl. Sie ist die Schranke, die ein spaeteres Einfuegen ohne
/// Entfernen ueberlebt, falls je ein Pfad die Aufraeumregel umgeht.
pub const GLOBAL_SESSION_CAP: usize = GLOBAL_CLIENT_CAP;
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
    /// D12 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): scharf
    /// gestellt panisiert der naechste Flush EINMAL, waehrend er den
    /// Standlock haelt. Ohne diese Naht laesst sich H-04 nicht messen:
    /// `modell_sicht` gibt nur Kopien zurueck und laesst seinen Guard fallen,
    /// bevor es zurueckkehrt - eine Panik danach vergiftet nichts.
    panik_unter_standlock: Arc<AtomicBool>,
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

    /// Stellt die Panik unter dem Standlock scharf. Sie faellt genau einmal.
    pub fn panik_unter_standlock_scharf(&self) {
        self.panik_unter_standlock.store(true, Ordering::SeqCst);
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
    /// D12: gespiegelte Scharfstellung aus `CoordinatorFlushTestHaken`. Sie
    /// liegt als nacktes Atomic am Coordinator, damit der Flush sie UNTER dem
    /// Standlock lesen kann, ohne dort einen zweiten Mutex zu nehmen - das
    /// waere eine neue Sperrreihenfolge.
    test_panik_unter_standlock: AtomicBool,
    /// G2-FLOATEDGE-001: wie oft ein abgeleitetes Analysefenster nicht
    /// gebildet werden konnte, weil die `sample_rate` keine normale positive
    /// Zahl war oder die Division nicht endlich blieb. NaN-Ehrlichkeit heisst
    /// verriegeln UND zaehlen, nicht still `inf` ausliefern.
    fenster_nicht_endlich: AtomicU64,
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
            test_panik_unter_standlock: AtomicBool::new(false),
            fenster_nicht_endlich: AtomicU64::new(0),
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
                stand.guard_eintragen(&guard.effective_address.clone(), &guard.derived_id.clone());
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
            test_panik_unter_standlock: AtomicBool::new(false),
            fenster_nicht_endlich: AtomicU64::new(0),
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
        self.test_panik_unter_standlock.store(
            haken.panik_unter_standlock.load(Ordering::SeqCst),
            Ordering::SeqCst,
        );
        *self
            .flush_test_haken
            .lock()
            .unwrap_or_else(|e| e.into_inner()) = Some(haken);
    }

    /// H-04, D12: ist der Standlock vergiftet? Ohne diese Frage waere die
    /// Vergiftung von aussen unbeobachtbar und der Test koennte nur behaupten,
    /// sie herbeigefuehrt zu haben.
    pub fn standlock_vergiftet(&self) -> bool {
        self.stand.is_poisoned()
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
