//! Der eine fail-closed Interventionszustand aus C-08.
//!
//! Eine Intervention wird erst angenommen, wenn ihre Sequenz passt; ein
//! Ueberlauf faellt zu und wird gemeldet, statt still zu wachsen.

use super::*;

#[cfg(windows)]
impl crate::server::V2Interventionssenke for Coordinator {
    fn hoermarkierung(&self, link_id: &str, aktiv: bool) {
        Coordinator::hoermarkierung_v2(self, link_id, aktiv);
    }

    fn getrennt(&self, link_id: &str) {
        Coordinator::hoermarkierung_v2_getrennt(self, link_id);
    }
}

impl Coordinator {
    /// `false` bedeutet Luecke oder unbekannter Link. Das Sticky-Bit ist dann
    /// bereits gesetzt. Die normale Ereignisfolge kann es nicht loeschen.
    pub(super) fn sequenz_annehmen(stand: &mut Stand, link_id: &str, sequence: u64) -> bool {
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
}
