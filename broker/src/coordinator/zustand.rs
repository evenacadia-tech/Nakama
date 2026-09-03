//! Der fluechtige Sessiongraph als Datenmodell.
//!
//! Hier steht, was der Coordinator ueber Clients, Links, Sessions und ihre
//! Interventionen weiss - ohne Verhalten. Persistiert wird nichts davon; der
//! Store ist die dauerhafte Wahrheit.

use super::*;

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(super) struct ClientKey {
    pub(super) logon_sid: String,
    pub(super) project_binding_id: String,
    pub(super) session_epoch: String,
    pub(super) instance_id: String,
}

impl ClientKey {
    pub(super) fn aus_adresse(adresse: &Adresse) -> Self {
        Self {
            logon_sid: adresse.logon_sid.clone(),
            project_binding_id: adresse.project_binding_id.clone(),
            session_epoch: adresse.session_epoch.clone(),
            instance_id: adresse.instance_id.clone(),
        }
    }

    pub(super) fn session(&self) -> SessionKey {
        SessionKey {
            project_binding_id: self.project_binding_id.clone(),
            session_epoch: self.session_epoch.clone(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub(super) struct SessionKey {
    pub(super) project_binding_id: String,
    pub(super) session_epoch: String,
}

#[derive(Debug, Clone)]
pub(super) struct ClientStand {
    pub(super) adresse: Adresse,
    pub(super) plugin_kind: String,
    pub(super) host_pid: Option<u32>,
    pub(super) session_ungebunden: bool,
    pub(super) current_link: Option<String>,
    pub(super) current_nonce: String,
    pub(super) last_seen: Duration,
    pub(super) stale: bool,
    pub(super) stale_seit: Option<Duration>,
    pub(super) join_kandidat: bool,
    pub(super) bestaetigt: bool,
    pub(super) explizit_bestaetigt: bool,
    pub(super) ausdruecklich_ungebunden: bool,
    pub(super) descriptor: Option<Value>,
    pub(super) state_revision: Option<u64>,
    pub(super) state_hash: Option<String>,
    pub(super) record_state_valid: bool,
    pub(super) recording: bool,
}

#[derive(Debug, Clone, Default)]
pub(super) struct SessionStand {
    pub(super) fuehrendes_main: Option<String>,
}

#[derive(Debug, Clone)]
pub(super) struct LinkStand {
    /// Adresse, die der Peer auf dem Draht sendet. Bei einer Probe vor dem
    /// Join traegt sie den projektgebundenen Join-Marker.
    pub(super) wire_adresse: Adresse,
    /// Effektive Adresse im Sessiongraphen. Nach eindeutigem Auto-Join traegt
    /// sie die vom Main erzeugte Session-Epoche.
    pub(super) adresse: Adresse,
    pub(super) client_key: ClientKey,
    pub(super) alias_adressraum: Sitzungsadressraum,
    pub(super) alias_besitzer: String,
    pub(super) letzte_event_sequence: Option<u64>,
    pub(super) verdraengt: bool,
    pub(super) trennen: bool,
    pub(super) join_neuverbinden: bool,
}

#[derive(Debug, Clone)]
pub(super) struct Subscription {
    pub(super) adresse: Adresse,
    pub(super) session_epoch: String,
}

#[derive(Debug, Clone)]
pub(super) struct Intervention {
    pub(super) link_id: String,
}

#[derive(Debug, Clone)]
pub(super) struct LiveMessframe {
    pub(super) adresse: Adresse,
    pub(super) payload: Vec<u8>,
    pub(super) empfangen: Duration,
    pub(super) sequence: u64,
    pub(super) sample_count: u32,
    pub(super) sample_rate: f64,
}

#[derive(Debug, Clone, Default)]
pub(super) struct Messfehler {
    pub(super) anzahl: u64,
    pub(super) aktuell: bool,
    pub(super) letzter_grund: Option<P2RejectGrund>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub(super) enum P2RejectGrund {
    FeatureBatchUngueltig,
    QuellframeAnzahlUngueltig,
    RoutingNichtFreigegeben,
    QuelladresseAbweichend,
    LautheitUngueltig,
}

impl P2RejectGrund {
    pub(super) const fn wire(self) -> &'static str {
        match self {
            Self::FeatureBatchUngueltig => "feature_batch_ungueltig",
            Self::QuellframeAnzahlUngueltig => "quellframe_anzahl_ungueltig",
            Self::RoutingNichtFreigegeben => "routing_nicht_freigegeben",
            Self::QuelladresseAbweichend => "quelladresse_abweichend",
            Self::LautheitUngueltig => "lautheit_ungueltig",
        }
    }
}

pub(super) const JSON_SAFE_INTEGER_MAX: u64 = 9_007_199_254_740_991;

#[cfg(test)]
pub(super) const P2_REJECT_KATALOG: [P2RejectGrund; 5] = [
    P2RejectGrund::FeatureBatchUngueltig,
    P2RejectGrund::QuellframeAnzahlUngueltig,
    P2RejectGrund::RoutingNichtFreigegeben,
    P2RejectGrund::QuelladresseAbweichend,
    P2RejectGrund::LautheitUngueltig,
];

#[derive(Debug, Clone, Default)]
pub(super) struct Lautheitsstand {
    pub(super) zustand: Lautheitszustand,
    pub(super) letztes_gueltiges_paar: Option<(f32, f32, Duration)>,
    pub(super) ungueltig_anzahl: u64,
}

#[derive(Debug)]
pub(super) struct Stand {
    pub(super) links: HashMap<String, LinkStand>,
    pub(super) clients: HashMap<ClientKey, ClientStand>,
    pub(super) sessions: HashMap<SessionKey, SessionStand>,
    pub(super) subscriptions: HashMap<String, Subscription>,
    pub(super) interventionen: HashMap<String, Intervention>,
    pub(super) intervention_state_unknown: bool,
    pub(super) tail_samples_offen: u64,
    pub(super) subscription_cleanups: u64,
    pub(super) subscription_abweisungen: u64,
    pub(super) letzter_subscription_grund: String,
    pub(super) evidence_angenommen: u64,
    pub(super) evidence_gesperrt: u64,
    pub(super) cap_abweisungen: u64,
    pub(super) store_verweigerungen: u64,
    pub(super) p2_live_frames: u64,
    pub(super) messframes: HashMap<ClientKey, LiveMessframe>,
    pub(super) messfehler: HashMap<ClientKey, Messfehler>,
    pub(super) lautheit: HashMap<ClientKey, Lautheitsstand>,
    pub(super) telemetry_links: HashSet<String>,
    pub(super) telemetry_kopplungen: u64,
    pub(super) conflict_guards: HashMap<String, HashSet<String>>,
    pub(super) routing_bereit: bool,
    pub(super) dirty_sessions: HashSet<SessionKey>,
    pub(super) session_commands: HashMap<String, SessionCommandWirkung>,
    pub(super) session_command_reihenfolge: VecDeque<String>,
}

#[derive(Debug, Clone)]
pub(super) struct SessionCommandWirkung {
    pub(super) kanonischer_auftrag: Vec<u8>,
    pub(super) state_revision: u64,
    pub(super) state_hash: String,
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
