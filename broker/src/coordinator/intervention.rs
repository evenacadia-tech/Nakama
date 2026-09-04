//! Der eine fail-closed Interventionszustand aus C-08 — SITZUNGSWEIT.
//!
//! Eine Intervention wird erst angenommen, wenn ihre Sequenz passt; ein
//! Ueberlauf faellt zu und wird gemeldet, statt still zu wachsen.
//!
//! ## Warum je Sitzung und nicht brokerweit (M-62)
//!
//! 🔑 Nacharbeit 1 nach der Erstpruefung 1 (Befund B17): bis dahin lagen
//! `interventionen`, `intervention_state_unknown` und `tail_samples_offen`
//! als DREI GLOBALE Felder im `Stand`. Zwei Folgen, und die zweite ist die
//! gefaehrliche:
//!
//! * ein Hoermarker in Sitzung A sperrte die Evidenz von Sitzung B mit —
//!   ein Fehlalarm, aber ein konservativer;
//! * ein `neutral_resync` aus Sitzung B loeschte die AKTIVEN Interventions-IDs
//!   von Sitzung A und liess dort kontaminierte Evidenz durch. Das ist der
//!   Fall, den §34.2 ausschliesst: „Ein verlorenes Begin oder End darf
//!   niemals eine scheinbar saubere Baseline erzeugen."
//!
//! Der Zustand liegt deshalb in `Stand::taint`, adressiert ueber den
//! `SessionKey`. Jede Operation leitet ihn aus ihrem `link_id` ab; eine
//! Sitzung ohne Eintrag ist sauber.

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
    /// Die Sitzung eines Links. `None` heisst „Link unbekannt" — und ein
    /// unbekannter Link kann keinen Sitzungszustand fuehren.
    pub(super) fn session_des_links(stand: &Stand, link_id: &str) -> Option<SessionKey> {
        stand
            .links
            .get(link_id)
            .map(|link| link.client_key.session())
    }

    /// Der Taintzustand einer Sitzung, angelegt falls noetig.
    pub(super) fn taint_mut<'a>(stand: &'a mut Stand, session: &SessionKey) -> &'a mut Taintstand {
        stand.taint.entry(session.clone()).or_default()
    }

    /// `false` bedeutet Luecke oder unbekannter Link. Das Sticky-Bit ist dann
    /// bereits gesetzt. Die normale Ereignisfolge kann es nicht loeschen.
    pub(super) fn sequenz_annehmen(stand: &mut Stand, link_id: &str, sequence: u64) -> bool {
        let Some(session) = Self::session_des_links(stand, link_id) else {
            // Ohne Link gibt es keine Sitzung, die man sperren koennte. Der
            // Zustand ist damit brokerweit unbekannt: ein Ereignis ohne
            // zuordenbare Herkunft ist genau der Fall, in dem niemand sagen
            // kann, WELCHE Sitzung betroffen ist.
            Self::alle_sitzungen_unbekannt(stand);
            return false;
        };
        let Some(link) = stand.links.get_mut(link_id) else {
            Self::taint_mut(stand, &session).unknown = true;
            return false;
        };
        if let Some(letzte) = link.letzte_event_sequence {
            if letzte.checked_add(1) != Some(sequence) {
                Self::taint_mut(stand, &session).unknown = true;
                return false;
            }
        }
        link.letzte_event_sequence = Some(sequence);
        true
    }

    /// Ein Ereignis ohne zuordenbare Sitzung sperrt ALLE bekannten.
    ///
    /// Das ist die fail-closed-Antwort auf „wir wissen nicht, wen es trifft":
    /// eine Sitzung stellvertretend zu waehlen waere geraten, und keine zu
    /// sperren waere die scheinbar saubere Baseline aus §34.2.
    pub(super) fn alle_sitzungen_unbekannt(stand: &mut Stand) {
        let sessions: Vec<SessionKey> = stand
            .clients
            .keys()
            .map(|key| key.session())
            .chain(stand.taint.keys().cloned())
            .collect();
        for session in sessions {
            Self::taint_mut(stand, &session).unknown = true;
        }
        // Auch wenn der Broker noch gar keine Sitzung kennt, darf der Fall
        // nicht spurlos bleiben: der Platzhalter traegt das Bit, bis eine
        // echte Sitzung entsteht.
        stand.taint.entry(SessionKey::unbekannt()).or_default().unknown = true;
    }

    pub fn intervention_begin(
        &self,
        link_id: &str,
        adresse: &Adresse,
        intervention_id: &str,
        event_sequence: u64,
    ) -> bool {
        self.intervention_begin_mit_art(
            link_id,
            adresse,
            intervention_id,
            event_sequence,
            "hoermarkierung",
            None,
            None,
        )
    }

    /// Derselbe Weg mit ausdruecklicher Art und Experimentbezug (M-59).
    pub fn intervention_begin_mit_art(
        &self,
        link_id: &str,
        adresse: &Adresse,
        intervention_id: &str,
        event_sequence: u64,
        art: &str,
        experiment_id: Option<&str>,
        beginn_projektsample: Option<i64>,
    ) -> bool {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let Some(session) = Self::session_des_links(&stand, link_id) else {
            Self::alle_sitzungen_unbekannt(&mut stand);
            return false;
        };
        if !Self::adresse_des_links_passt(&stand, link_id, adresse)
            || !Self::sequenz_annehmen(&mut stand, link_id, event_sequence)
        {
            Self::taint_mut(&mut stand, &session).unknown = true;
            return false;
        }
        {
            let taint = Self::taint_mut(&mut stand, &session);
            if taint.interventionen.len() >= MAX_AKTIVE_INTERVENTIONEN
                || taint.interventionen.contains_key(intervention_id)
            {
                taint.unknown = true;
                return false;
            }
            taint.interventionen.insert(
                intervention_id.to_owned(),
                Intervention {
                    link_id: link_id.to_owned(),
                    art: art.to_owned(),
                    experiment_id: experiment_id.map(str::to_owned),
                    beginn_projektsample,
                },
            );
        }
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
        self.intervention_end_mit_beginn(
            link_id,
            adresse,
            intervention_id,
            event_sequence,
            tail_samples,
        )
        .0
    }

    /// Derselbe Weg, aber MIT dem Projektzeitstempel des Beginns (M-52, R25).
    ///
    /// Rueckgabe: ob das Ende angenommen wurde, und der Beginn des Bereichs.
    /// `None` beim zweiten Wert heisst „der Beginn trug keine Projektzeit" —
    /// dann gibt es keinen Bereich, und der Aufrufer invalidiert fail-closed
    /// die ganze Sitzung.
    pub fn intervention_end_mit_beginn(
        &self,
        link_id: &str,
        adresse: &Adresse,
        intervention_id: &str,
        event_sequence: u64,
        tail_samples: u64,
    ) -> (bool, Option<i64>) {
        // Befund B1: der Nachlauf braucht seinen Startzeitpunkt. Die Uhr wird
        // VOR dem Lock gelesen — sie ist nicht Teil des Standes.
        let jetzt = self.clock.jetzt();
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let Some(session) = Self::session_des_links(&stand, link_id) else {
            Self::alle_sitzungen_unbekannt(&mut stand);
            return (false, None);
        };
        if !Self::adresse_des_links_passt(&stand, link_id, adresse)
            || !Self::sequenz_annehmen(&mut stand, link_id, event_sequence)
        {
            Self::taint_mut(&mut stand, &session).unknown = true;
            return (false, None);
        }
        let taint = Self::taint_mut(&mut stand, &session);
        let passt = taint
            .interventionen
            .get(intervention_id)
            .is_some_and(|i| i.link_id == link_id);
        let beginn = taint
            .interventionen
            .get(intervention_id)
            .and_then(|i| i.beginn_projektsample);
        if !passt {
            // Ein End ohne bekanntes Begin ist gerade KEIN sauberer
            // Neutralzustand: das Begin kann vor Reconnect/Overflow verloren
            // gegangen sein. Nur `neutral_resync` darf dieses Urteil loesen.
            taint.unknown = true;
            return (false, None);
        }
        taint.interventionen.remove(intervention_id);
        taint.tail_samples_offen = taint.tail_samples_offen.max(tail_samples);
        // 🔑 Nacharbeit 3 (Befund B1): der Nachlauf startet HIER, und der Tick
        // rechnet ab hier. `max` gilt weiter: ein laengerer Nachlauf verdraengt
        // einen kuerzeren, und die Uhr beginnt mit ihm von vorn.
        taint.tail_samples_gesamt = taint.tail_samples_offen;
        taint.tail_seit = (taint.tail_samples_offen > 0).then_some(jetzt);
        // 🔑 Nacharbeit 2 (Befund R02, M-58): der Nachlauf steht in SAMPLES,
        // der Tick laeuft in ZEIT. Die Umrechnung braucht die Rate DERSELBEN
        // Instanz, die den Nachlauf gemeldet hat — er ist ihre Groesse.
        let rate = stand
            .links
            .get(link_id)
            .and_then(|link| stand.clients.get(&link.client_key))
            .map(|client| client.abtastrate)
            .filter(|r| r.is_finite() && *r > 0.0)
            .unwrap_or(48_000.0);
        Self::taint_mut(&mut stand, &session).abtastrate = rate;
        (true, beginn)
    }

    /// Legacy-v2 und v3 teilen denselben `interventionen`-Bestand. Das
    /// synthetische v2-Schluesselwort kann nicht mit einer hex32-v3-ID
    /// kollidieren. `false` entfernt nur den sauber bekannten v2-Eintrag und
    /// loescht niemals Unknown.
    pub fn hoermarkierung_v2(&self, link_id: &str, aktiv: bool) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let session = Self::session_des_links(&stand, link_id).unwrap_or_else(SessionKey::unbekannt);
        let id = format!("v2:{link_id}");
        let taint = Self::taint_mut(&mut stand, &session);
        if aktiv {
            if taint.interventionen.len() >= MAX_AKTIVE_INTERVENTIONEN {
                taint.unknown = true;
            } else {
                taint.interventionen.insert(
                    id,
                    Intervention {
                        link_id: link_id.to_owned(),
                        art: "hoermarkierung".to_owned(),
                        experiment_id: None,
                        // Der v2-Legacypfad kennt keine Projektzeit; fail-closed
                        // heisst dann „die ganze Sitzung" (M-52, R25).
                        beginn_projektsample: None,
                    },
                );
            }
        } else {
            taint.interventionen.remove(&id);
        }
    }

    pub fn hoermarkierung_v2_getrennt(&self, link_id: &str) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let session = Self::session_des_links(&stand, link_id).unwrap_or_else(SessionKey::unbekannt);
        let taint = Self::taint_mut(&mut stand, &session);
        taint
            .interventionen
            .retain(|_, intervention| intervention.link_id != link_id);
        taint.unknown = true;
    }

    /// Ein Ringueberlauf OHNE bekannten Link: er sperrt alle Sitzungen.
    pub fn intervention_overflow(&self) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        Self::alle_sitzungen_unbekannt(&mut stand);
    }

    /// Ein Ringueberlauf MIT bekanntem Link — der Produktpfad (M-39).
    pub fn intervention_overflow_fuer_link(&self, link_id: &str) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        match Self::session_des_links(&stand, link_id) {
            Some(session) => Self::taint_mut(&mut stand, &session).unknown = true,
            None => Self::alle_sitzungen_unbekannt(&mut stand),
        }
    }

    /// Der Nachlauf schrumpft — aus dem Uhr-/Liveness-Tick gerufen (M-58/M-60).
    ///
    /// 🔑 Nacharbeit 1 (Befund B16): diese Funktion hatte ausserhalb der Tests
    /// keinen Aufrufer. Nach einem Endereignis mit positivem Tail blieb der
    /// Evidenzpfad deshalb DAUERHAFT gesperrt, statt nach dem Nachlauf wieder
    /// zu nullen. Sie zaehlt fuer JEDE Sitzung herunter, weil der Nachlauf
    /// eine Zeitgroesse ist und nicht an einer Verbindung haengt.
    pub fn tail_fortschritt(&self, samples: u64) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        for taint in stand.taint.values_mut() {
            taint.tail_samples_offen = taint.tail_samples_offen.saturating_sub(samples);
            // Befund B1: der Bezugspunkt wandert mit. Wer den Rest von Hand
            // kuerzt, hat damit auch die verstrichene Zeit verbraucht — sonst
            // zoege der naechste Zeittick sie ein zweites Mal ab.
            taint.tail_samples_gesamt = taint.tail_samples_offen;
            if taint.tail_samples_offen == 0 {
                taint.tail_seit = None;
            }
        }
    }

    /// Derselbe Fortschritt, aber aus VERSTRICHENER ZEIT gerechnet (M-58).
    ///
    /// 🔑 Nacharbeit 2 (Befund R02): die Runde 1 zog je Tick eine feste
    /// Samplezahl ab, begruendet mit einem Ein-Sekunden-Tick. Der produktive
    /// Supervisor ruft `liveness_tick()` aber alle 100 ms (`lib.rs`). Ein
    /// Nachlauf von 48.000 Samples war damit nach 200 ms frei statt nach einer
    /// Sekunde — fuenfmal zu schnell, und genau in dem Fenster, in dem der
    /// Filterhall des Markers noch in die Messung laeuft. Der alte Test rief
    /// den Tick OHNE Zeitfortschritt und pruefte nur, dass der Wert irgendwann
    /// null wird; er konnte den Fehler nicht sehen.
    ///
    /// Jede Sitzung rechnet mit IHRER Rate: der Nachlauf gehoert der Instanz,
    /// die ihn gemeldet hat.
    /// 🔑 Nacharbeit 3 (Befund B1): gerechnet wird ab dem TAIL-START, nicht
    /// ab dem vorigen Tick.
    ///
    /// Der Aufrufer gibt die aktuelle Uhrzeit, nicht ein Intervall. Damit ist
    /// das Ergebnis unabhaengig von der Phasenlage der Ticks: ein Nachlauf, der
    /// unmittelbar nach einem Tick beginnt, verliert beim naechsten genau die
    /// seither vergangene Zeit — nicht das volle Tickintervall.
    pub(super) fn tail_fortschritt_zeit(&self, jetzt: Duration) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        for taint in stand.taint.values_mut() {
            if taint.tail_samples_offen == 0 || !(taint.abtastrate > 0.0) {
                continue;
            }
            let Some(seit) = taint.tail_seit else {
                continue;             // Ohne Bezugspunkt wird nichts abgezogen.
            };
            let sekunden = jetzt.saturating_sub(seit).as_secs_f64();
            let samples = (sekunden * taint.abtastrate).floor();
            // NaN-Ehrlichkeit: eine nicht endliche Rechnung zieht NICHTS ab.
            // Eine Sperre zu frueh zu loesen ist der teure Fehler, sie zu
            // spaet zu loesen der billige (§34.2).
            let samples = if samples.is_finite() && samples >= 0.0 {
                samples.min(u64::MAX as f64) as u64
            } else {
                0
            };
            taint.tail_samples_offen = taint.tail_samples_gesamt.saturating_sub(samples);
            if taint.tail_samples_offen == 0 {
                taint.tail_seit = None;
                taint.tail_samples_gesamt = 0;
            }
        }
    }

    /// Einziger Entsperrweg fuer Unknown. Er ist eine explizite Coordinator-
    /// Lebenszyklusoperation, kein normales End/False und keine neue
    /// Wirefamilie. Die bestaetigte Basis wird fuer den Link gesetzt; der
    /// neutrale Zustand leert aktive IDs und Nachlauf gemeinsam — und zwar
    /// NUR in der Sitzung dieses Links (M-62).
    pub fn neutral_resync(&self, link_id: &str, bestaetigte_sequence_basis: u64) -> bool {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let Some(session) = Self::session_des_links(&stand, link_id) else {
            return false;
        };
        let Some(link) = stand.links.get_mut(link_id) else {
            return false;
        };
        link.letzte_event_sequence = Some(bestaetigte_sequence_basis);
        {
            let taint = Self::taint_mut(&mut stand, &session);
            taint.interventionen.clear();
            taint.tail_samples_offen = 0;
            taint.unknown = false;
        }
        // ⚠️ Der PLATZHALTER faellt mit.
        //
        // Er traegt, was sich keiner Sitzung zuordnen liess — ein v2-Marker
        // ohne v3-Link, der Disconnect einer noch ungebundenen Probe, ein
        // Ringueberlauf ohne bekannte Herkunft. Genau diese Faelle sind der
        // Grund, warum §34.2 den Resync als AUSDRUECKLICHE, bestaetigte
        // Lebenszyklusoperation fuehrt: er ist die eine Stelle, an der jemand
        // sagt „der Zustand ist jetzt neutral". Ihn den Platzhalter NICHT
        // loesen zu lassen hiesse, den Broker nach dem ersten unzuordenbaren
        // Ereignis dauerhaft zu sperren.
        //
        // Die AKTIVEN Interventionen FREMDER Sitzungen bleiben davon
        // unberuehrt — genau das war der Befund B17.
        Self::taint_mut(&mut stand, &SessionKey::unbekannt()).unknown = false;
        true
    }

    /// Schliesst alle Intervalle einer Art — der Gegenpfad zu einem
    /// Experimentterminal (M-59, Befund B22).
    ///
    /// Rueckgabe: wie viele Intervalle geschlossen wurden. Ein Terminal, das
    /// nichts schliesst, ist kein Fehler: der Versuch kann ohne hoerbaren
    /// Eingriff gelaufen sein.
    pub(super) fn taint_intervalle_schliessen(
        stand: &mut Stand,
        session: &SessionKey,
        art: &str,
        experiment_id: Option<&str>,
    ) -> usize {
        let taint = Self::taint_mut(stand, session);
        let vorher = taint.interventionen.len();
        taint.interventionen.retain(|_, i| {
            !(i.art == art
                && (experiment_id.is_none() || i.experiment_id.as_deref() == experiment_id))
        });
        vorher - taint.interventionen.len()
    }

    /// Schliesst die `art=experiment`-Intervalle eines Versuchs in JEDER
    /// Sitzung, die eines haelt (Befund B4, M-59).
    ///
    /// 🔑 Wiederpruefung 2: `verdraenge_fuer` verdraengt projektuebergreifend,
    /// und die Runde 2 rief die Schliessung mit der Sitzung des NEUEN
    /// Begin-Aufrufers. Verdraengt der Deckel einen Versuch aus einer anderen
    /// Sitzung, blieb dessen Taint dort OFFEN — der Versuch war terminal, sein
    /// Eingriff aber galt weiter, und die Sitzung des Opfers lieferte
    /// dauerhaft keine starke Evidenz mehr.
    ///
    /// Ein Intervall kennt seinen Besitzer: es liegt in der Taintmap GENAU der
    /// Sitzung, die es geoeffnet hat, und `experiment_id` ist global
    /// eindeutig. Die Zuordnung muss deshalb nicht geraten und nicht zusaetzlich
    /// persistiert werden — sie steht da, wo das Intervall steht.
    pub(super) fn taint_intervalle_des_experiments_schliessen(
        stand: &mut Stand,
        experiment_id: &str,
    ) -> usize {
        let mut geschlossen = 0usize;
        for taint in stand.taint.values_mut() {
            let vorher = taint.interventionen.len();
            taint.interventionen.retain(|_, i| {
                !(i.art == "experiment" && i.experiment_id.as_deref() == Some(experiment_id))
            });
            geschlossen += vorher - taint.interventionen.len();
        }
        geschlossen
    }

    /// Schliesst die `art=experiment`-Intervalle EINES Versuchs, ueber einen
    /// Link adressiert (M-59).
    ///
    /// Dieselbe Huellenform wie `invalidierung_wegen_*_fuer_link`: `SessionKey`
    /// ist modulintern, die oeffentliche Flaeche des Coordinators adressiert
    /// ueber `link_id`. Der Experimentpfad ruft die interne Form; dies ist der
    /// Zugang fuer Aufrufer ausserhalb des Coordinators und fuer Beine, die
    /// messen wollen, dass ein Intervall wirklich SEINEM Versuch zugeordnet
    /// wurde.
    pub fn experiment_intervalle_schliessen_fuer_link(
        &self,
        link_id: &str,
        experiment_id: &str,
    ) -> usize {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let Some(session) = Self::session_des_links(&stand, link_id) else {
            return 0;
        };
        Self::taint_intervalle_schliessen(&mut stand, &session, "experiment", Some(experiment_id))
    }

    /// Haelt die Taintmap in ihren Grenzen (M-74).
    ///
    /// Die Map waechst mit jeder Sitzung, die je einen Eingriff gesehen hat —
    /// und Sitzungen entstehen bei jedem FL-Neustart neu. Ein Eintrag ohne
    /// Client und ohne Aussage faellt sofort (`client_eviktieren_locked`);
    /// ein DIRTY Eintrag ohne Client bleibt, weil sein sticky Unknown eine
    /// Aussage ist. Genau die koennen sich anhaeufen.
    ///
    /// Der Deckel ist deshalb derselbe wie fuer die Sessionmap: mehr Sitzungen
    /// als `GLOBAL_SESSION_CAP` fuehrt der Broker ohnehin nicht. Ueber der
    /// Grenze fallen zuerst die sauberen Eintraege; erst wenn davon keiner
    /// mehr da ist, faellt ein dirty — und das ist ausdruecklich ein Verlust
    /// einer Sperre, kein stilles Aufraeumen. Er wird gezaehlt.
    pub(super) fn taint_deckel_halten(&self) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        if stand.taint.len() <= GLOBAL_SESSION_CAP {
            return;
        }
        let lebende: std::collections::HashSet<SessionKey> =
            stand.clients.keys().map(|k| k.session()).collect();
        let sauber: Vec<SessionKey> = stand
            .taint
            .iter()
            .filter(|(session, taint)| taint.erlaubt() && !lebende.contains(*session))
            .map(|(session, _)| session.clone())
            .collect();
        for session in sauber {
            if stand.taint.len() <= GLOBAL_SESSION_CAP {
                return;
            }
            stand.taint.remove(&session);
        }
        while stand.taint.len() > GLOBAL_SESSION_CAP {
            let Some(weg) = stand
                .taint
                .keys()
                .find(|session| !lebende.contains(*session))
                .cloned()
            else {
                // Nur noch lebende Sitzungen: der Deckel greift dann nicht,
                // denn eine lebende Sitzung darf ihre Sperre nicht verlieren.
                return;
            };
            stand.taint.remove(&weg);
            stand.taint_verworfen = stand.taint_verworfen.saturating_add(1);
        }
    }

    /// Wie viele sticky Sperren der Deckel verworfen hat. `> 0` heisst: der
    /// Broker hat mehr tote Sitzungen mit offenem Taint gesehen, als er
    /// fuehren kann — eine Zahl, die niemand ignorieren sollte.
    pub fn taint_verworfen_zaehler(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .taint_verworfen
    }

    /// Die brokerweite Zusammenfassung ueber ALLE Sitzungen.
    ///
    /// Sie bleibt konservativ: unbekannt in EINER Sitzung heisst unbekannt,
    /// und der laengste offene Nachlauf gewinnt. Wer die Aussage FUER EINE
    /// Sitzung braucht, nimmt `interventionssicht_fuer_link`.
    pub fn interventionssicht(&self) -> Interventionssicht {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let mut aktive = 0usize;
        let mut tail = 0u64;
        let mut unknown = false;
        for taint in stand.taint.values() {
            aktive += taint.interventionen.len();
            tail = tail.max(taint.tail_samples_offen);
            unknown |= taint.unknown;
        }
        Interventionssicht {
            aktive,
            tail_samples_offen: tail,
            unknown,
            starke_evidenz_erlaubt: !unknown && aktive == 0 && tail == 0,
        }
    }

    /// Dieselbe Sicht FUER EINE Sitzung, ueber ihren Link adressiert.
    pub fn interventionssicht_fuer_link(&self, link_id: &str) -> Interventionssicht {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let session = Self::session_des_links(&stand, link_id).unwrap_or_else(SessionKey::unbekannt);
        let leer = Taintstand::default();
        let taint = stand.taint.get(&session).unwrap_or(&leer);
        Interventionssicht {
            aktive: taint.interventionen.len(),
            tail_samples_offen: taint.tail_samples_offen,
            unknown: taint.unknown,
            starke_evidenz_erlaubt: taint.erlaubt(),
        }
    }

    /// Der Riegel liegt VOR dem Evidence-Commit und zaehlt das Urteil unter
    /// demselben Lock. So kann ein Begin nicht erst nach dem Dispatch wirken.
    ///
    /// 🔑 Nacharbeit 1 (Befund B12): die frueher oeffentliche `evidence_dispatch()`
    /// nahm ihr Lock selbst und gab es VOR dem Insert wieder frei. Ein
    /// paralleles `intervention_begin` konnte dazwischen laufen, und die
    /// bereits kontaminierte Evidenz wurde trotzdem committet. Die Entscheidung
    /// steht deshalb jetzt als `_locked`-Form da: der Aufrufer haelt das Lock
    /// ueber Pruefung, Zaehlung UND Insert.
    pub(super) fn evidence_dispatch_locked(stand: &mut Stand, session: &SessionKey) -> bool {
        let erlaubt = stand.taint.get(session).map_or(true, Taintstand::erlaubt);
        // Eine Sitzung ohne Eintrag ist sauber — aber der Platzhalter fuer
        // Ereignisse ohne zuordenbare Sitzung sperrt trotzdem.
        let erlaubt = erlaubt
            && stand
                .taint
                .get(&SessionKey::unbekannt())
                .map_or(true, Taintstand::erlaubt);
        if erlaubt {
            stand.evidence_angenommen = stand.evidence_angenommen.saturating_add(1);
        } else {
            stand.evidence_gesperrt = stand.evidence_gesperrt.saturating_add(1);
        }
        erlaubt
    }

    /// Dieselbe Entscheidung BROKERWEIT: erlaubt nur, wenn KEINE Sitzung
    /// gesperrt ist.
    ///
    /// Sie ist die konservative Sicht fuer Aufrufer ohne Link — etwa den
    /// v2-Legacypfad, der gar keine v3-Sitzung kennt. Sie ist ausdruecklich
    /// nicht der Produktweg des Evidenzempfaengers: der kennt seine Sitzung
    /// und darf deshalb genauer sein (M-62).
    pub fn evidence_dispatch_brokerweit(&self) -> bool {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let erlaubt = stand.taint.values().all(Taintstand::erlaubt);
        if erlaubt {
            stand.evidence_angenommen = stand.evidence_angenommen.saturating_add(1);
        } else {
            stand.evidence_gesperrt = stand.evidence_gesperrt.saturating_add(1);
        }
        erlaubt
    }

    /// Dieselbe Entscheidung ueber einen Link — fuer Beine und fuer Aufrufer,
    /// die kein Lock halten.
    pub fn evidence_dispatch_fuer_link(&self, link_id: &str) -> bool {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let session = Self::session_des_links(&stand, link_id).unwrap_or_else(SessionKey::unbekannt);
        Self::evidence_dispatch_locked(&mut stand, &session)
    }
}
