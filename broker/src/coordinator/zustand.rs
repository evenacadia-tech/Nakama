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
    /// Die GESPEICHERTE Form der dauerhaften Konfliktriegel. Ihre Bytes
    /// bleiben exakt wie bisher, damit Altstaende ihre eigenen Riegel
    /// wiederfinden - H-21 aendert nur, wie gesucht wird, nicht was steht.
    pub(super) conflict_guards: HashMap<String, HashSet<String>>,
    /// H-21: der gefaltete Index ueber dieselben Riegel. Ein Peer, der seine
    /// SID anders schreibt, erzeugt eine andere effektive Adresse und
    /// schuettelte damit den dauerhaften fail-closed-Riegel ab. Alle elf
    /// Abfragestellen gehen ueber diesen Index; die drei schreibenden und die
    /// zwei loeschenden pflegen ihn mit, sonst liefen Lesen und Schreiben
    /// auseinander. Der Schluessel ist die ASCII-kleingeschriebene effektive
    /// Adresse; Windows-SIDs sind reines ASCII.
    pub(super) conflict_guards_gefaltet: HashMap<String, HashSet<String>>,
    /// H-16: Links, die eine Nonce-Verdraengung ausgeloest hat und deren
    /// Snapshot-Push noch aussteht. `control_ende` raeumt den Link selbst
    /// bereits ab, bevor `control_getrennt` kommt - der Marker kann deshalb
    /// nicht am Link haengen. Er wird in control_getrennt entnommen und
    /// ausgeloest.
    pub(super) verdraengt_wartet_auf_push: HashSet<String>,
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

impl Stand {
    /// H-21: die eine Abfrage, ueber die alle elf Lesestellen gehen.
    pub(super) fn guard_gesetzt(&self, effective_address: &str) -> bool {
        self.conflict_guards_gefaltet
            .contains_key(&effective_address.to_ascii_lowercase())
    }

    /// H-21: traegt einen Riegel in BEIDE Formen ein. Rueckgabe wie
    /// `HashSet::insert`: true, wenn er neu war.
    pub(super) fn guard_eintragen(&mut self, effective_address: &str, derived_id: &str) -> bool {
        self.conflict_guards_gefaltet
            .entry(effective_address.to_ascii_lowercase())
            .or_default()
            .insert(derived_id.to_owned());
        self.conflict_guards
            .entry(effective_address.to_owned())
            .or_default()
            .insert(derived_id.to_owned())
    }

    /// H-21: entfernt ihn aus beiden Formen. Die gespeicherte Form wird ueber
    /// den gefalteten Schluessel gesucht, damit auch eine anders geschriebene
    /// Aufloesung ihren eigenen Riegel findet.
    pub(super) fn guard_entfernen(&mut self, effective_address: &str, derived_id: &str) {
        let gefaltet = effective_address.to_ascii_lowercase();
        if let Some(ids) = self.conflict_guards_gefaltet.get_mut(&gefaltet) {
            ids.remove(derived_id);
            if ids.is_empty() {
                self.conflict_guards_gefaltet.remove(&gefaltet);
            }
        }
        let schluessel: Vec<String> = self
            .conflict_guards
            .keys()
            .filter(|k| k.to_ascii_lowercase() == gefaltet)
            .cloned()
            .collect();
        for k in schluessel {
            if let Some(ids) = self.conflict_guards.get_mut(&k) {
                ids.remove(derived_id);
                if ids.is_empty() {
                    self.conflict_guards.remove(&k);
                }
            }
        }
    }
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
            conflict_guards_gefaltet: HashMap::new(),
            verdraengt_wartet_auf_push: HashSet::new(),
            conflict_guards: HashMap::new(),
            routing_bereit: true,
            dirty_sessions: HashSet::new(),
            session_commands: HashMap::new(),
            session_command_reihenfolge: VecDeque::new(),
        }
    }
}

#[cfg(test)]
mod h21_tests {
    use super::*;

    /// NAK-121 H-21: die Riegelabfrage ist unempfindlich gegen die Schreibweise
    /// der SID, und die GESPEICHERTE Form bleibt bytegleich.
    ///
    /// Gemessen wird an `Stand` selbst, weil die effektive Adresse dort ihre
    /// endgueltige Form hat: sie entsteht aus der bereits umgeschriebenen
    /// Linkadresse, nicht aus dem, was ein Client im Hello schickt. Ein Test
    /// eine Ebene hoeher muesste diese Form raten und wuerde die Zusage
    /// verfehlen statt sie zu messen.
    #[test]
    fn konfliktriegel_greift_auch_bei_anderer_sid_schreibweise() {
        let mut stand = Stand::default();
        let gross = "S-1-5-21-1-2-3-1001|aa|bb|cc";
        let klein = gross.to_ascii_lowercase();
        assert_ne!(gross, klein);

        assert!(stand.guard_eintragen(gross, "instanz:nonce"));
        assert!(stand.guard_gesetzt(gross));
        assert!(
            stand.guard_gesetzt(&klein),
            "die anders geschriebene SID schuettelt den dauerhaften Riegel ab"
        );
        // Auch gemischt geschrieben, wie ein Peer sie nun einmal schicken darf.
        assert!(stand.guard_gesetzt("s-1-5-21-1-2-3-1001|AA|bb|Cc"));

        // Was ausdruecklich UNVERAENDERT bleibt: die gespeicherten Bytes. Nur
        // so finden Altstaende ihre eigenen Riegel wieder, und weder Wire- noch
        // State-Vertrag wird beruehrt.
        assert!(stand.conflict_guards.contains_key(gross));
        assert!(!stand.conflict_guards.contains_key(&klein));

        // Ein zweiter Riegel derselben Adresse in anderer Schreibweise landet
        // im selben gefalteten Eintrag - sonst waeren es zwei Riegel fuer eine
        // Adresse, und die Aufloesung des einen liesse den anderen stehen.
        assert!(stand.guard_eintragen(&klein, "zweite:nonce"));
        assert_eq!(stand.conflict_guards_gefaltet[&klein].len(), 2);

        // Die Aufloesung findet den Riegel ueber JEDE Schreibweise und raeumt
        // beide Formen.
        stand.guard_entfernen(&klein, "instanz:nonce");
        assert!(stand.guard_gesetzt(gross), "der zweite Riegel fiel mit");
        stand.guard_entfernen("s-1-5-21-1-2-3-1001|Aa|Bb|Cc", "zweite:nonce");
        assert!(!stand.guard_gesetzt(gross));
        assert!(!stand.guard_gesetzt(&klein));
        assert!(
            stand.conflict_guards.is_empty(),
            "die gespeicherte Form behielt einen verwaisten Eintrag: {:?}",
            stand.conflict_guards
        );
    }
}
