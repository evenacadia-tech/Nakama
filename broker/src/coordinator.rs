//! SONDE-011, Phase A: der semantische v3-Coordinator hinter dem
//! byteorientierten Transport.
//!
//! Diese Phase bindet zwei Lebenszyklen, die der I/O-Worker allein nicht
//! kennen kann:
//!
//! * `subscribe_session` gehoert genau dem Control-Link und dessen Hello-
//!   Adresse. Sein einziger Gegenpfad ist das Ende dieses Links (28-B).
//! * v2-Hoermarkierung und v3-Interventionsereignisse speisen genau einen
//!   fail-closed Zustand. Overflow, Sequenzluecke und Control-Ende setzen ein
//!   sticky Unknown-Bit; normale End-/False-Meldungen loeschen es nie.

use crate::instance_alias::{AliasRegister, Sitzungsadressraum};
use crate::transport::bootstrap::{Adresse, HelloControl};
use serde_json::Value;
use std::collections::HashMap;
use std::sync::Mutex;

const MAX_AKTIVE_INTERVENTIONEN: usize = 64;

#[derive(Debug, Clone)]
struct LinkStand {
    adresse: Adresse,
    alias_adressraum: Sitzungsadressraum,
    alias_besitzer: String,
    letzte_event_sequence: Option<u64>,
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

#[derive(Debug, Default)]
struct Stand {
    links: HashMap<String, LinkStand>,
    subscriptions: HashMap<String, Subscription>,
    interventionen: HashMap<String, Intervention>,
    intervention_state_unknown: bool,
    tail_samples_offen: u64,
    subscription_cleanups: u64,
    subscription_abweisungen: u64,
    letzter_subscription_grund: String,
    evidence_angenommen: u64,
    evidence_gesperrt: u64,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Interventionssicht {
    pub aktive: usize,
    pub tail_samples_offen: u64,
    pub unknown: bool,
    pub starke_evidenz_erlaubt: bool,
}

#[derive(Debug, Default)]
pub struct Coordinator {
    stand: Mutex<Stand>,
    alias_register: AliasRegister,
}

impl Coordinator {
    pub fn control_registrieren(&self, link_id: &str, adresse: Adresse) {
        let alias_adressraum = Sitzungsadressraum::neu(
            &adresse.logon_sid,
            &adresse.project_binding_id,
            &adresse.session_epoch,
        );
        let alias_besitzer = format!("{}:{}", adresse.instance_id, adresse.runtime_nonce);
        // Lockreihenfolge im Coordinator ist immer Stand -> AliasRegister.
        // Push, Disconnect und Registrierung koennen dadurch nicht zyklisch
        // aufeinander warten.
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        let _ = self.alias_register.registriere_wire_zuordnung(
            &alias_adressraum,
            &alias_besitzer,
            &adresse.instance_id,
        );
        stand.links.insert(
            link_id.to_owned(),
            LinkStand {
                adresse,
                alias_adressraum,
                alias_besitzer,
                letzte_event_sequence: None,
            },
        );
    }

    /// Der synchrone Server-Hook. Subscription, Link und aktive Ereignisse
    /// werden unter EINEM Lock entfernt; gleichzeitig wird Unknown sticky.
    /// Ein Push teilt diesen Lock und kann den Zwischenzustand nicht sehen.
    pub fn control_ende(&self, link_id: &str) {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");
        if let Some(link) = stand.links.remove(link_id) {
            self.alias_register.entferne(
                &link.alias_adressraum,
                &link.alias_besitzer,
                &link.adresse.instance_id,
            );
        }
        if stand.subscriptions.remove(link_id).is_some() {
            stand.subscription_cleanups = stand.subscription_cleanups.saturating_add(1);
        }
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
        stand.subscriptions.insert(
            link_id.to_owned(),
            Subscription {
                adresse,
                session_epoch: session_epoch.to_owned(),
            },
        );
        true
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
        stand.links.get(link_id).is_some_and(|link| {
            self.alias_register.dispatch_erlaubt(
                &link.alias_adressraum,
                &link.alias_besitzer,
                &link.adresse.instance_id,
            )
        })
    }

    fn alias_quarantaenisiert(&self, link_id: &str) -> bool {
        let stand = self.stand.lock().expect("Coordinator vergiftet");
        stand.links.get(link_id).is_some_and(|link| {
            self.alias_register
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
                if wert
                    .get("intervention_state_unknown")
                    .and_then(Value::as_bool)
                    == Some(true)
                {
                    self.intervention_overflow();
                }
                let sequence = wert.get("sequence")?.as_u64()?;
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
    fn control_verbunden(&self, link_id: &str, hello: &HelloControl) {
        self.control_registrieren(link_id, hello.adresse.clone());
    }

    fn control_schliesst(&self, link_id: &str) {
        self.control_ende(link_id);
    }

    fn control_getrennt(&self, _link_id: &str) {}
    fn telemetrie_gekoppelt(&self, _link_id: &str) {}
    fn telemetrie_getrennt(&self, _link_id: &str) {}

    fn p0(&self, link_id: &str, payload: &[u8]) -> Option<Vec<u8>> {
        self.p0_json(link_id, payload)
    }

    fn p1(&self, link_id: &str, payload: &[u8]) {
        let _ = self.subscribe_json(link_id, payload);
    }

    fn p2(&self, _link_id: &str, _payload: &[u8]) {
        if self.dispatch_fuer_link_erlaubt(_link_id) {
            let _ = self.evidence_dispatch();
        }
    }

    fn abgewiesen(&self, _grund: &str) {}
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
        let ack = c
            .p0_json(link_id, br#"{"type":"heartbeat","sequence":1}"#)
            .expect("heartbeat_ack");
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
        assert!(!c.dispatch_fuer_link_erlaubt("link-b"));
        assert!(heartbeat_duplicate(&c, "link-a"));
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
