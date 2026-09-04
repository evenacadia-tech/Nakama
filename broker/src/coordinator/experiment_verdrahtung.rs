//! Die Experimentfamilien im PRODUKTPFAD (SONDE-013 M-25, M-40 bis M-51).
//!
//! ## Warum dieses Modul existiert
//!
//! 🔑 Nacharbeit 1 nach der Erstpruefung 1 (Befunde B18 bis B22): `experiment`
//! war nur ein oeffentlich exportiertes Modul. Der Coordinator besass keinen
//! `Experimentstore`, und `p0_json_mit_minor` hatte fuer `experiment_begin`,
//! `experiment_abort` und `experiment_manual_result` keinen Match-Zweig —
//! schema-gueltige Produktnachrichten fielen in `_ => None` und bewirkten
//! NICHTS. M-40, M-47 und M-49 existierten damit nicht im Produktpfad,
//! sondern nur in ihren eigenen Tests.
//!
//! ## Wie ein Versuch den Neustart ueberdauert (M-47/M-50)
//!
//! Jede Zustandsaenderung geht als append-only `StoreEvent` in dieselbe
//! SQLite-Ablage wie alles andere: `event_type = "experiment"` projiziert in
//! die Tabelle `experiments`, `"passage"` in `passages`. Der Payload traegt
//! die UNVERAENDERLICHEN Referenzen aus §43.1 — Passage samt Fingerprint,
//! Baseline, Kandidat, Match-Gain, Blindreihenfolge und die Evidence-IDs.
//! Erst damit ist ein offener Versuch nach einem Brokerneustart wirklich
//! rekonstruierbar; ein Log aus IDs allein waere es nicht.

use super::*;
use crate::coordinator::experiment::{
    Abbruchgrund, Achsenrechnung, Alignmentwert, Blindreihenfolge, Experimentreferenz, Hoerurteil,
    Passage, Resultatmessung, Terminal,
};
use crate::coordinator::vergleichbarkeit::Passagenbeleg;
use crate::telemetrie::Fingerprintwerte;

impl Coordinator {
    /// Der Einstieg fuer alle drei Experimentfamilien.
    ///
    /// Reihenfolge, und sie ist der Grund fuer die Bauform: erst die
    /// FACHLICHE Vorpruefung, dann die Persistenz des Befehls, dann die
    /// Wirkung. Wer zuerst persistierte, schriebe einen Befehl fest, den die
    /// Fachlogik gleich darauf ablehnt.
    pub(super) fn experiment_p0(&self, link_id: &str, wert: &Value) -> Option<Vec<u8>> {
        let art = wert.get("type")?.as_str()?;
        let kopf = wert.get("kopf")?;
        let command_id = kopf.get("command_id")?.as_str()?;
        let base_revision = kopf.get("base_revision")?.as_u64()?;
        let experiment_id = wert.get("experiment_id")?.as_str()?.to_owned();

        // 🔑 Nacharbeit 2 (Befund R08): die IDEMPOTENZ steht VOR der
        // fachlichen Vorpruefung.
        //
        // Nach einer vollstaendig erfolgreichen Ausfuehrung lehnte die
        // Fachlogik einen Retry als `revision_conflict` beziehungsweise
        // `schon_terminal` ab — obwohl der Sender nur seine Antwort nicht
        // bekommen hat. Ein bereits committeter Befehl ist keine neue
        // Absicht, sondern dieselbe: er bekommt dieselbe Antwort.
        if let Some(ack) = self.bekannter_befehl(wert) {
            return Some(ack);
        }

        // ── Vorpruefung, ohne etwas zu aendern ──────────────────────────
        //
        // 🔑 Sie prueft ALLES, was die Wirkung unten ablehnen koennte. Der
        // Grund ist die Ehrlichkeit des ACK: `persistenz_p0` schreibt den
        // Befehl fest und antwortet `angewandt`, BEVOR die Wirkung laeuft.
        // Was hier durchrutscht, bekaeme also ein `angewandt` und taete
        // nichts — ein totes Element auf der Leitung.
        let session_vorab = serde_json::from_value::<Adresse>(kopf.get("ziel")?.clone())
            .ok()
            .map(|a| ClientKey::aus_adresse(&a).session());
        // 🔑 Nacharbeit 2 (Befund R09, E-03/M-48): die Vorpruefung findet ein
        // Experiment ueber die GLOBALE `experiment_id`. Ohne den Vergleich mit
        // der Projektbindung der ZIELSITZUNG konnte ein autorisiertes Main aus
        // Projekt B mit der ID eines Versuchs aus Projekt A dessen Abbruch oder
        // Resultat ausloesen — und Persistenz und Taintschliessung wurden
        // danach sogar Projekt B zugeordnet. Ein Versuch gehoert seinem
        // Projekt; ein fremdes ist ein Konflikt, keine Zustaendigkeit.
        let fremdes_projekt = |stand: &Stand| -> bool {
            let (Some(ziel), Some(e)) = (
                session_vorab.as_ref(),
                stand.experimente.experiment(&experiment_id),
            ) else {
                return false;
            };
            e.projektbindung != ziel.project_binding_id
        };
        let ablehnung = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            if fremdes_projekt(&stand) {
                return Self::command_ack(
                    command_id,
                    "abgelehnt",
                    base_revision,
                    None,
                    Some("revision_conflict"),
                );
            }
            match art {
                "experiment_begin" => {
                    if stand.experimente.experiment(&experiment_id).is_some() {
                        // Append-only: dieselbe ID zweimal ist kein
                        // Ueberschreiben, sondern ein Konflikt.
                        Some("revision_conflict")
                    } else {
                        match Self::passage_aus_wert(wert) {
                            None => Some("schema_violation"),
                            // M-30: eine Passage ohne genug gemessenes Signal
                            // traegt keinen Vergleich, also auch keinen
                            // Versuch. `Experimentstore::beginne` lehnt sie ab
                            // — der Sender soll das ERFAHREN.
                            Some(p) if !(p.abdeckung >= 0.5) => Some("abdeckung_zu_gering"),
                            Some(_) => None,
                        }
                    }
                }
                // 🔑 Nacharbeit 2 (R16/R21): der Kandidat und die Bindung der
                // Blindreihenfolge. Ein zweiter Kandidat ist zulaessig (M-41:
                // „ein zweiter Versuch erzeugt einen neuen Kandidaten"); die
                // Reihenfolge bleibt dabei die ZUERST gebundene — sie zu
                // aendern waere genau die nachtraegliche Wahl, die M-44
                // ausschliesst.
                "experiment_candidate" => match stand.experimente.experiment(&experiment_id) {
                    None => Some("unknown_target"),
                    Some(e) if !e.offen() => Some("schon_terminal"),
                    Some(_) if Self::referenz_aus_wert(wert.get("referenz")).is_none() => {
                        Some("schema_violation")
                    }
                    Some(_) => None,
                },
                "experiment_abort" => match stand.experimente.experiment(&experiment_id) {
                    None => Some("unknown_target"),
                    Some(e) if !e.offen() => Some("schon_terminal"),
                    Some(_) => None,
                },
                "experiment_manual_result" => match stand.experimente.experiment(&experiment_id) {
                    None => Some("unknown_target"),
                    Some(e) if !e.offen() => Some("schon_terminal"),
                    Some(e) => {
                        // M-44: die auf der Leitung stehende Reihenfolge ist
                        // die AUFGEDECKTE. Widerspricht sie der gebundenen,
                        // waere genau das passiert, was M-44 ausschliesst —
                        // die Reihenfolge nachtraeglich zum Urteil passend
                        // erzaehlen. Fail-closed.
                        let gemeldet = match wert.get("blindreihenfolge").and_then(Value::as_str) {
                            Some("kandidat_zuerst") => Blindreihenfolge::KandidatZuerst,
                            _ => Blindreihenfolge::BaselineZuerst,
                        };
                        if !e.reihenfolge_gebunden() {
                            // 🔑 Nacharbeit 2 (Befund R21, M-44): die Bindung
                            // hat seit dieser Runde einen EIGENEN Befehlszweig
                            // (`experiment_candidate`), der VOR dem Hoeren
                            // laeuft. Die Runde 1 band die vom Sender ZUSAMMEN
                            // MIT dem Hoerurteil gemeldete Reihenfolge
                            // unmittelbar vor dem Terminal — der Sender konnte
                            // sie also nach dem Hoeren waehlen. Fehlt sie
                            // jetzt, ist das ein eigener, benennbarer Fall und
                            // kein generisches `internal`.
                            Some("reihenfolge_nicht_gebunden")
                        } else if e.gebundene_reihenfolge_fuer_pruefung() != Some(gemeldet) {
                            Some("blindreihenfolge_widerspruch")
                        } else if !e.baseline.match_gain_db.is_finite() {
                            Some("ohne_lautheitsabgleich")
                        } else {
                            // M-45: ohne Resultatmessung gibt es kein
                            // Terminal. Das steht hier UND im Store — hier,
                            // damit der Sender ein ehrliches `abgelehnt`
                            // bekommt, dort, weil die Regel dem Store gehoert.
                            drop(stand);
                            let messung = session_vorab
                                .as_ref()
                                .map(|s| self.resultatmessung(&experiment_id, s))
                                .unwrap_or_default();
                            return if messung.hat_resultat() {
                                self.experiment_p0_weiter(link_id, wert, art, &experiment_id)
                            } else {
                                Self::command_ack(
                                    command_id,
                                    "abgelehnt",
                                    base_revision,
                                    None,
                                    Some("ohne_resultatmessung"),
                                )
                            };
                        }
                    }
                },
                _ => Some("internal"),
            }
        };
        if let Some(code) = ablehnung {
            return Self::command_ack(command_id, "abgelehnt", base_revision, None, Some(code));
        }
        self.experiment_p0_weiter(link_id, wert, art, &experiment_id)
    }

    /// Persistenz und Wirkung — EIN Append, und die Wirkung ist bis dahin
    /// vorlaeufig (Befund R08).
    ///
    /// Die Reihenfolge ist die Korrektur der Runde 1:
    ///
    /// 1. Die Wirkung wird VORLAEUFIG auf den fluechtigen Stand angewandt.
    ///    Nur so entstehen die Domaenenereignisse aus dem TATSAECHLICHEN
    ///    Zustand danach und nicht aus einer zweiten, nachgebauten Rechnung.
    /// 2. Befehl und Domaenenereignisse gehen in EINEN `store.append` — der
    ///    Writer zieht sie in eine Transaktion.
    /// 3. Scheitert der Append oder ist der Befehl nicht `angewandt`, wird der
    ///    Stand ZURUECKGENOMMEN. Ein Speicher, der dem Log voraus ist, waere
    ///    die zweite Wahrheit aus §33.5.
    fn experiment_p0_weiter(
        &self,
        link_id: &str,
        wert: &Value,
        art: &str,
        experiment_id: &str,
    ) -> Option<Vec<u8>> {
        let kopf = wert.get("kopf")?;
        let session = ClientKey::aus_adresse(
            &serde_json::from_value::<Adresse>(kopf.get("ziel")?.clone()).ok()?,
        )
        .session();
        let command_id = kopf.get("command_id")?.as_str()?;
        let base_revision = kopf.get("base_revision")?.as_u64()?;

        // Die Resultatmessung nimmt ihr eigenes Lock — deshalb VOR dem Block
        // unten, nicht darin.
        let messung = if art == "experiment_manual_result" {
            self.resultatmessung(experiment_id, &session)
        } else {
            Resultatmessung::default()
        };

        // Die Grenze, an der Baseline und Resultat auseinandergehen: die
        // Ankunftsreihenfolge der Evidenz IN DIESEM Moment (M-49, Befund R17).
        let evidenzfolge = self.evidenz_folge.load(Ordering::SeqCst);

        // ── 1. Die Wirkung, vorlaeufig ─────────────────────────────────
        let (rueckfall, ereignisse) = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let rueckfall = stand.experimente.clone();
            let ereignisse = match art {
                "experiment_begin" => {
                    Self::begin_anwenden_locked(
                        &mut stand,
                        wert,
                        experiment_id,
                        &session,
                        evidenzfolge,
                    )
                }
                "experiment_candidate" => {
                    Self::kandidat_anwenden_locked(&mut stand, wert, experiment_id, evidenzfolge)
                }
                "experiment_abort" => {
                    let grund = match wert.get("grund").and_then(Value::as_str) {
                        Some("verdraengt") => Abbruchgrund::Verdraengt,
                        _ => Abbruchgrund::UserAbbruch,
                    };
                    Self::abbruch_anwenden_locked(&mut stand, experiment_id, grund)
                }
                "experiment_manual_result" => {
                    Self::ergebnis_anwenden_locked(&mut stand, wert, experiment_id, &messung)
                }
                _ => None,
            };
            match ereignisse {
                Some(e) => (rueckfall, e),
                None => {
                    stand.experimente = rueckfall;
                    return Self::command_ack(
                        command_id,
                        "abgelehnt",
                        base_revision,
                        None,
                        Some("internal"),
                    );
                }
            }
        };

        // ── 2. Befehl UND Wirkung in EINEM Append ──────────────────────
        let ack = self.persistenz_p0_mit_domaene(link_id, wert, ereignisse);
        let angewandt = ack.as_deref().is_some_and(Self::ack_ist_angewandt);
        if !angewandt {
            // ── 3. Ruecknahme ──────────────────────────────────────────
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.experimente = rueckfall;
            return ack;
        }

        // Der Taint gehoert zur Wirkung, aber nicht in den Rueckfall: er
        // schliesst Intervalle, die es ohne die Wirkung gar nicht gaebe.
        if art != "experiment_begin" {
            self.taint_von_experiment_schliessen(&session, experiment_id);
        }
        // 🔑 Nacharbeit 2 (Befund R14, M-49): das Ergebnis ERREICHT Gen.
        //
        // Das Terminal rief bis dahin nur `domaene_persistieren`; dieser Event
        // bekam keine `snapshot_ziele`, und die einzige Outbox-Schuld stammte
        // vom vorher committeten Befehl, dessen Sessionsnapshot VOR der Wirkung
        // erstellt wurde und ohnehin keine Experimentfelder besass. Ein
        // gerechnetes Resultat erreichte Gen deshalb NIE.
        //
        // Der Rueckweg laeuft ueber den bestehenden Outbox-/Snapshot-Pfad
        // statt ueber eine weitere Familie — genau das sagt die Beschreibung
        // von `experiment_manual_result` im Vertrag zu (§53.9). Der Snapshot
        // traegt seit dieser Runde `experimente`; hier wird er FAELLIG.
        {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.dirty_sessions.insert(session.clone());
        }
        self.flush_session(&session, Some(link_id));
        // 🔑 Nacharbeit 2 (Befund R24, M-54/M-31): DER Produktaufrufer der
        // MATERIALinvalidierung.
        //
        // `invalidierung_wegen_material` hatte ausserhalb seiner Huelle keinen.
        // Der Vergleich gehoert hierher, weil hier zwei ueber DASSELBE Fenster
        // gerechnete Fingerprints nebeneinander stehen: der gespeicherte der
        // Passage und der, den dieses `experiment_begin` fuer dieselbe
        // `passage_id` mitbringt. M-31 sagt ausdruecklich, dass der Wechsel aus
        // dem Fingerprintvergleich kommt und nicht aus einer Zeitheuristik.
        //
        // `beginne` legt eine bereits bekannte Passage NICHT neu an — der neue
        // Fingerprint faellt also weg. Genau deshalb darf die Abweichung nicht
        // still bleiben: fail-closed heisst hier invalidieren.
        if art == "experiment_begin" {
            if let Some(neu) = Self::passage_aus_wert(wert) {
                let alt = self
                    .stand
                    .lock()
                    .unwrap_or_else(|e| e.into_inner())
                    .experimente
                    .passage(&neu.passage_id)
                    .map(|p| p.fingerprint.clone());
                if let Some(alt) = alt {
                    if alt != neu.fingerprint {
                        self.invalidierung_wegen_material(
                            &session,
                            Some(&alt),
                            Some(&neu.fingerprint),
                        );
                    }
                }
            }
        }
        ack
    }

    /// Steht in diesem `command_ack` „angewandt"?
    fn ack_ist_angewandt(ack: &[u8]) -> bool {
        serde_json::from_slice::<Value>(ack)
            .ok()
            .and_then(|v| {
                v.get("ergebnis")
                    .and_then(Value::as_str)
                    .map(|e| e == "angewandt")
            })
            .unwrap_or(false)
    }

    /// `experiment_begin` auf dem gehaltenen Lock (M-25/M-40, R10/R11).
    fn begin_anwenden_locked(
        stand: &mut Stand,
        wert: &Value,
        experiment_id: &str,
        session: &SessionKey,
        evidenzfolge: u64,
    ) -> Option<Vec<Domaenenereignis>> {
        let passage = Self::passage_aus_wert(wert)?;
        let baseline = Self::referenz_aus_wert(wert.get("referenz"))?;
        let passage_id = passage.passage_id.clone();
        let passage_neu = stand.experimente.passage(&passage_id).is_none();
        let passage_kopie = passage.clone();
        let verdraengt = stand
            .experimente
            .beginne(
                experiment_id,
                &session.project_binding_id,
                passage,
                baseline,
                evidenzfolge,
            )
            .ok()?;

        let mut aus = Vec::new();
        // Die Passage zuerst: sie ist das Objekt, auf das der Versuch zeigt.
        // Sie wird nur beim ERSTEN Versuch angelegt, der sie nennt; ein
        // zweiter Versuch derselben Passage schreibt sie nicht um.
        if passage_neu {
            aus.push(Domaenenereignis {
                event_type: "passage".into(),
                payload: Self::passage_json(&passage_kopie),
                ziele: Vec::new(),
            });
        }
        // 🔑 Befund R10: JEDE Verdraengung bekommt ihr eigenes Terminal.
        //
        // `beginne` schloss das aelteste offene Experiment intern mit
        // `verdraengt` und gab davon nichts zurueck. Der Wrapper persistierte
        // ausschliesslich die neue Anlage — der verdraengte Versuch blieb im
        // Store OFFEN, und seine Taintintervalle blieben es mit ihm.
        for weg in &verdraengt {
            if let Some(e) = stand.experimente.experiment(weg) {
                let payload =
                    Self::experiment_json(e, stand.experimente.passage(&e.passage_id), "verdraengt");
                aus.push(Domaenenereignis {
                    event_type: "experiment".into(),
                    payload,
                    ziele: Vec::new(),
                });
            }
            Self::taint_intervalle_schliessen(stand, session, "experiment", Some(weg));
        }
        let e = stand.experimente.experiment(experiment_id)?;
        let payload =
            Self::experiment_json(e, stand.experimente.passage(&e.passage_id), "begonnen");
        aus.push(Domaenenereignis {
            event_type: "experiment".into(),
            payload,
            ziele: Vec::new(),
        });
        Some(aus)
    }

    /// `experiment_candidate` auf dem gehaltenen Lock (M-41/M-44, R16/R21).
    ///
    /// Der Kandidat wird ERFASST und die Blindreihenfolge GEBUNDEN — in
    /// dieser Reihenfolge und in einem Schritt. Die Bindung ist append-only:
    /// ein zweiter Kandidat aendert sie nicht, sonst liesse sie sich nach dem
    /// Hoeren noch drehen.
    fn kandidat_anwenden_locked(
        stand: &mut Stand,
        wert: &Value,
        experiment_id: &str,
        evidenzfolge: u64,
    ) -> Option<Vec<Domaenenereignis>> {
        let referenz = Self::referenz_aus_wert(wert.get("referenz"))?;
        stand
            .experimente
            .neuer_kandidat(experiment_id, referenz, evidenzfolge)
            .ok()?;
        let reihenfolge = match wert.get("blindreihenfolge").and_then(Value::as_str) {
            Some("kandidat_zuerst") => Blindreihenfolge::KandidatZuerst,
            _ => Blindreihenfolge::BaselineZuerst,
        };
        // `Ok(false)` heisst „war schon gebunden" und ist kein Fehler: die
        // ERSTE Bindung gilt.
        stand
            .experimente
            .binde_reihenfolge(experiment_id, reihenfolge)
            .ok()?;
        let e = stand.experimente.experiment(experiment_id)?;
        let payload = Self::experiment_json(e, stand.experimente.passage(&e.passage_id), "kandidat");
        Some(vec![Domaenenereignis {
            event_type: "experiment".into(),
            payload,
            ziele: Vec::new(),
        }])
    }

    /// `experiment_abort` auf dem gehaltenen Lock (M-47).
    fn abbruch_anwenden_locked(
        stand: &mut Stand,
        experiment_id: &str,
        grund: Abbruchgrund,
    ) -> Option<Vec<Domaenenereignis>> {
        stand.experimente.schliesse(experiment_id, grund).ok()?;
        let e = stand.experimente.experiment(experiment_id)?;
        let payload =
            Self::experiment_json(e, stand.experimente.passage(&e.passage_id), "abgebrochen");
        Some(vec![Domaenenereignis {
            event_type: "experiment".into(),
            payload,
            ziele: Vec::new(),
        }])
    }

    /// `experiment_manual_result` auf dem gehaltenen Lock (M-44/M-45/M-49).
    fn ergebnis_anwenden_locked(
        stand: &mut Stand,
        wert: &Value,
        experiment_id: &str,
        messung: &Resultatmessung,
    ) -> Option<Vec<Domaenenereignis>> {
        let hoerurteil = match wert.get("hoerurteil").and_then(Value::as_str) {
            Some("baseline") => Hoerurteil::Baseline,
            Some("kandidat") => Hoerurteil::Kandidat,
            Some("kein_unterschied") => Hoerurteil::KeinUnterschied,
            _ => Hoerurteil::Enthaltung,
        };
        let notiz = wert.get("notiz").and_then(Value::as_str).map(str::to_owned);
        let werkzeug = wert
            .get("werkzeug")
            .and_then(Value::as_str)
            .map(str::to_owned);
        // 🔑 Befund R21/M-44: die Reihenfolge wird hier NICHT mehr gebunden.
        //
        // Die Runde 1 uebernahm die vom Sender ZUSAMMEN MIT dem Hoerurteil
        // gemeldete Reihenfolge und band sie unmittelbar vor dem Terminal —
        // der Sender konnte sie damit erst nach dem Hoeren waehlen. Gebunden
        // wird sie im eigenen Befehlszweig, VOR dem Urteil; hier wird nur noch
        // abgeschlossen. Die Vorpruefung haelt daneben fest, dass die GEMELDETE
        // Reihenfolge zur gebundenen passt.
        stand
            .experimente
            .ergebnis(experiment_id, hoerurteil, notiz, werkzeug, messung)
            .ok()?;
        let e = stand.experimente.experiment(experiment_id)?;
        let payload =
            Self::experiment_json(e, stand.experimente.passage(&e.passage_id), "ergebnis");
        Some(vec![Domaenenereignis {
            event_type: "experiment".into(),
            payload,
            ziele: Vec::new(),
        }])
    }

    /// M-59, Befund B22: JEDES Terminal schliesst die zugehoerigen
    /// `art=experiment`-Taintintervalle.
    fn taint_von_experiment_schliessen(&self, session: &SessionKey, experiment_id: &str) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        Self::taint_intervalle_schliessen(&mut stand, session, "experiment", Some(experiment_id));
    }

    // ── Die Domaenenform: der VOLLSTAENDIGE Zustand, nicht die Transition ──
    //
    // 🔑 Befund R11/R15: der Begin-Payload trug nur Passage-ID, Match-Gain,
    // Quellen und Klassen; Fingerprints, Alignment und Reproduzierbarkeit
    // fehlten, und der Terminal-Payload ERSETZTE ihn in `experiments`, ohne
    // Hoerurteil, Blindreihenfolge, Notiz, Werkzeug und Ausfuehrungsart zu
    // tragen. Aus diesen Zeilen war weder ein offener Versuch noch ein
    // abgeschlossener rekonstruierbar.
    //
    // Die Projektion `experiments.state_jcs` traegt deshalb den VOLLSTAENDIGEN
    // Zustand nach der Transition; `ereignis` benennt daneben, WELCHE
    // Transition ihn erzeugt hat, und `experiment_events` indiziert jede
    // einzelne. Ein Ueberschreiben des Begin-Zustands kann es damit nicht
    // geben: der neue Zustand ENTHAELT ihn.

    pub(super) fn fingerprint_json(f: &Fingerprintwerte) -> Value {
        serde_json::json!({
            "version": f.version,
            "band_energie": f.band_energie.to_vec(),
            "chroma": f.chroma.to_vec(),
            "onset": f.onset.to_vec(),
        })
    }

    pub(super) fn passage_json(p: &Passage) -> Value {
        serde_json::json!({
            "passage_id": p.passage_id,
            "projekt_von": p.projekt_von,
            "projekt_bis": p.projekt_bis,
            "transport_epoch": p.transport_epoch,
            "aktive_quellen": p.aktive_quellen,
            "messpunktklassen": p.messpunktklassen,
            "abdeckung": p.abdeckung,
            "label": p.label,
            "fingerprint": Self::fingerprint_json(&p.fingerprint),
        })
    }

    fn referenz_json(r: &Experimentreferenz) -> Value {
        serde_json::json!({
            "passage_fingerprint": Self::fingerprint_json(&r.passage_fingerprint),
            "upstream_fingerprint": Self::fingerprint_json(&r.upstream_fingerprint),
            "aktive_quellen": r.aktive_quellen,
            "messpunktklassen": r.messpunktklassen,
            "match_gain_db": r.match_gain_db,
            "nicht_endliche_samples": r.nicht_endliche_samples,
            "alignment": Self::alignment_wort(r.alignment),
        })
    }

    fn alignment_wort(a: Alignmentwert) -> &'static str {
        match a {
            Alignmentwert::FeatureAligned => "feature_aligned",
            Alignmentwert::AudioAligned => "audio_aligned",
            Alignmentwert::Probable => "probable",
            Alignmentwert::Unclear => "unclear",
        }
    }

    fn hoerurteil_wort(h: Hoerurteil) -> &'static str {
        match h {
            Hoerurteil::Baseline => "baseline",
            Hoerurteil::Kandidat => "kandidat",
            Hoerurteil::KeinUnterschied => "kein_unterschied",
            Hoerurteil::Enthaltung => "enthaltung",
        }
    }

    fn reihenfolge_wort(r: Blindreihenfolge) -> &'static str {
        match r {
            Blindreihenfolge::KandidatZuerst => "kandidat_zuerst",
            Blindreihenfolge::BaselineZuerst => "baseline_zuerst",
        }
    }

    fn achsen_json(a: &Achsenrechnung) -> Value {
        serde_json::json!({
            "intervall": a.intervall.map(|(u, o)| vec![u, o]),
            "signifikante_baender": a.signifikante_baender,
            "gescannte_baender": a.gescannte_baender,
            "vergleichbarkeit": a.vergleichbarkeit,
            "vergleichbarkeit_gruende": a.vergleichbarkeit_gruende,
            "guardrail_abdeckung_delta": a.guardrail_abdeckung_delta,
            "guardrail_klasse_gefallen": a.guardrail_klasse_gefallen,
            // Befund R19: die fuenf Guardrails aus M-45 reisen MIT dem
            // Terminalereignis. Ohne sie waere „stabil" eine Behauptung.
            "guardrail_loudness_db": a.guardrail_loudness_db,
            "guardrail_peak_db": a.guardrail_peak_db,
            "guardrail_transient": a.guardrail_transient,
            "guardrail_breite_db": a.guardrail_breite_db,
            "guardrail_geschuetzt_db": a.guardrail_geschuetzt_db,
            "effekt_stabil": a.effekt_stabil,
        })
    }

    /// Der vollstaendige Zustand eines Versuchs nach §43.1 (M-40/M-49/M-50).
    pub(super) fn experiment_json(
        e: &crate::coordinator::experiment::Experiment,
        passage: Option<&Passage>,
        ereignis: &str,
    ) -> Value {
        let terminal = match &e.terminal {
            Some(Terminal::Ergebnis {
                hoerurteil,
                blindreihenfolge,
                notiz,
                werkzeug,
                achsen,
            }) => serde_json::json!({
                "art": "ergebnis",
                // 🔑 Befund R15: die NUTZERDATEN reisen mit. Ohne sie kann ein
                // Replay das Urteil nicht verlustfrei wiederherstellen.
                "hoerurteil": Self::hoerurteil_wort(*hoerurteil),
                "blindreihenfolge": Self::reihenfolge_wort(*blindreihenfolge),
                "notiz": notiz,
                "werkzeug": werkzeug,
                "achsen": Self::achsen_json(achsen),
            }),
            Some(Terminal::Abgebrochen { grund }) => serde_json::json!({
                "art": "abgebrochen",
                "grund": match grund {
                    Abbruchgrund::Verdraengt => "verdraengt",
                    Abbruchgrund::UserAbbruch => "user_abbruch",
                },
            }),
            None => Value::Null,
        };
        serde_json::json!({
            "experiment_id": e.experiment_id,
            "ereignis": ereignis,
            "projektbindung": e.projektbindung,
            "passage_id": e.passage_id,
            // §43.1 nennt beide ausdruecklich; sie sind ableitbar und stehen
            // trotzdem da — derselbe Grund wie bei `probe_descriptor.aussageklasse`.
            "execution_mode": "manual_external",
            "reproduzierbarkeit": "manuell_nicht_wiederherstellbar",
            "folge": e.folge,
            // Die Evidenzgrenzen aus Befund R17: ohne sie koennte ein
            // wiederhergestellter Versuch Baseline und Resultat nicht mehr
            // trennen.
            "begin_evidenzfolge": e.begin_evidenzfolge,
            "passage": passage.map(Self::passage_json),
            "baseline": Self::referenz_json(&e.baseline),
            "kandidaten": e
                .kandidaten
                .iter()
                .map(|k| serde_json::json!({
                    "nummer": k.nummer,
                    "evidenzfolge": k.evidenzfolge,
                    "referenz": Self::referenz_json(&k.referenz),
                }))
                .collect::<Vec<Value>>(),
            "blindreihenfolge_gebunden": e
                .gebundene_reihenfolge_fuer_pruefung()
                .map(Self::reihenfolge_wort),
            "baseline_evidence_ids": e.baseline_evidence_ids,
            "resultat_evidence_ids": e.resultat_evidence_ids,
            "terminal": terminal,
        })
    }


    /// Die Resultatmessung EINES Versuchs aus dem Evidenzbestand (M-45/M-49).
    ///
    /// 🔑 Nacharbeit 2 (Befund R17): die Runde 1 nahm die Quelle mit der
    /// laengsten Historie IRGENDEINER Quelle der Sitzung und teilte die
    /// gesamte Retention stumpf in zwei Haelften. Vier bereits VOR dem
    /// `experiment_begin` eingegangene Snapshots genuegten damit fuer ein
    /// sofortiges Resultat; Passage, Begin-Grenze, Quellenset, Messpunktklasse
    /// und Kandidat wurden nicht geprueft.
    ///
    /// Die Bindung steht jetzt in fuenf Riegeln, und jeder faellt fuer sich:
    ///
    /// 1. **Passage** — nur Evidenz IM Fenster der Passage und in IHRER
    ///    Transportepoche (§32.4). Ein Beleg von anderswo misst anderes.
    /// 2. **Quellen** — nur die im `experiment_begin` eingefrorenen.
    /// 3. **Messpunktklasse** — je Quelle die, die zur Passage gehoert
    ///    (M-28/M-55). Ein Wechsel macht den Beleg unbrauchbar, nicht schwach.
    /// 4. **Grenzen** — Baseline ist, was VOR dem Begin ankam; Resultat, was
    ///    NACH dem erfassten Kandidaten ankam. Dazwischen liegt die
    ///    Fremdaenderung, und Belege von dort gehoeren keiner Seite.
    /// 5. **Ausschluss** — was eine Invalidierung zurueckgenommen hat, zaehlt
    ///    nicht mehr (M-52, Befund R29).
    pub(super) fn resultatmessung(
        &self,
        experiment_id: &str,
        session: &SessionKey,
    ) -> Resultatmessung {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let mut messung = Resultatmessung::default();
        let Some(e) = stand.experimente.experiment(experiment_id) else {
            return messung;
        };
        let Some(passage) = stand.experimente.passage(&e.passage_id) else {
            return messung;
        };
        // Befund R16: ohne erfassten Kandidaten gibt es keine Resultatgrenze.
        let Some(kandidat) = e.kandidaten.last() else {
            return messung;
        };
        let begin_folge = e.begin_evidenzfolge;
        let kandidat_folge = kandidat.evidenzfolge;

        // Die Messpunktklasse JE QUELLE, wie die Passage sie eingefroren hat.
        let klasse_der_quelle = |instanz: &str| -> Option<&String> {
            let i = passage.aktive_quellen.iter().position(|q| q == instanz)?;
            passage.messpunktklassen.get(i)
        };

        let mut baseline: Vec<&super::evidenz::Evidenzstand> = Vec::new();
        let mut resultat: Vec<&super::evidenz::Evidenzstand> = Vec::new();
        for (key, historie) in stand.evidenz.iter() {
            if key.session() != *session {
                continue;
            }
            let Some(soll) = klasse_der_quelle(&key.instance_id) else {
                continue;                 // Quelle gehoert nicht zur Passage.
            };
            // Die HEUTIGE Klasse dieser Quelle. Weicht sie ab, ist der Beleg
            // nicht vergleichbar — Gate 7 (§49.2).
            let ist = stand
                .clients
                .get(key)
                .and_then(|c| c.descriptor.as_ref())
                .and_then(|d| d.get("measurement_position"))
                .and_then(Value::as_str);
            if ist != Some(soll.as_str()) {
                continue;
            }
            for eintrag in historie.iter() {
                if eintrag.ausschlussgrund.is_some() {
                    continue;             // R29: zurueckgenommen zaehlt nicht.
                }
                if eintrag.transport_epoch != passage.transport_epoch {
                    continue;
                }
                let Some(von) = eintrag.project_sample_start else {
                    // Ohne Projektzeit laesst sich der Beleg der Passage nicht
                    // zuordnen. Ihn trotzdem zu nehmen waere geraten (§32.3).
                    continue;
                };
                let bis = von.saturating_add(eintrag.sample_count as i64);
                if von < passage.projekt_von || bis > passage.projekt_bis {
                    continue;
                }
                if eintrag.empfangsfolge < begin_folge {
                    baseline.push(eintrag);
                } else if eintrag.empfangsfolge > kandidat_folge {
                    resultat.push(eintrag);
                }
            }
        }
        if baseline.is_empty() || resultat.is_empty() {
            return messung;
        }
        baseline.sort_by_key(|e| e.empfangsfolge);
        resultat.sort_by_key(|e| e.empfangsfolge);

        let baender = resultat
            .iter()
            .map(|e| e.p50_db.len())
            .min()
            .unwrap_or(0)
            .min(baseline.iter().map(|e| e.p50_db.len()).min().unwrap_or(0));
        if baender == 0 {
            return messung;
        }

        let mittel_je_band = |satz: &[&super::evidenz::Evidenzstand],
                              p95: bool|
         -> (Vec<f64>, Vec<bool>) {
            let mut summe = vec![0.0f64; baender];
            let mut anzahl = vec![0usize; baender];
            for s in satz {
                let (werte, gueltig) = if p95 {
                    (&s.p95_db, &s.p95_gueltig)
                } else {
                    (&s.p50_db, &s.p50_gueltig)
                };
                for band in 0..baender {
                    if gueltig.get(band).copied().unwrap_or(false) {
                        if let Some(v) = werte.get(band) {
                            summe[band] += *v as f64;
                            anzahl[band] += 1;
                        }
                    }
                }
            }
            let mut w = Vec::with_capacity(baender);
            let mut g = Vec::with_capacity(baender);
            for band in 0..baender {
                if anzahl[band] > 0 {
                    w.push(summe[band] / anzahl[band] as f64);
                    g.push(true);
                } else {
                    // M-07: kein Beleg heisst Wert 0 mit `gueltig = false`.
                    w.push(0.0);
                    g.push(false);
                }
            }
            (w, g)
        };

        let (basis_p50, basis_gueltig) = mittel_je_band(&baseline, false);
        let (res_p50, res_gueltig) = mittel_je_band(&resultat, false);
        for band in 0..baender {
            let ok = basis_gueltig[band] && res_gueltig[band];
            messung
                .band_delta_db
                .push(if ok { res_p50[band] - basis_p50[band] } else { 0.0 });
            messung.band_gueltig.push(ok);
        }

        // 🔑 Befund R20: die ZEITREIHE je Fenster. Der Block-Bootstrap zieht
        // Bloecke benachbarter Analysefenster; ohne diese Reihe blieb ihm nur
        // die Bandachse, und die ist keine Zeit.
        for s in &resultat {
            let mut zeile = Vec::with_capacity(baender);
            for band in 0..baender {
                let gueltig = s.p50_gueltig.get(band).copied().unwrap_or(false)
                    && basis_gueltig[band];
                zeile.push(if gueltig {
                    s.p50_db[band] as f64 - basis_p50[band]
                } else {
                    f64::NAN
                });
            }
            messung.fenster_delta_db.push(zeile);
        }

        messung.abdeckung_baseline =
            baseline.iter().map(|s| s.abdeckung).sum::<f64>() / baseline.len() as f64;
        messung.abdeckung_resultat =
            resultat.iter().map(|s| s.abdeckung).sum::<f64>() / resultat.len() as f64;
        messung.klasse_baseline = baseline.last().map(|s| s.klasse.clone()).unwrap_or_default();
        messung.klasse_resultat = resultat.last().map(|s| s.klasse.clone()).unwrap_or_default();
        messung.baseline_evidence_ids =
            baseline.iter().map(|s| s.evidence_id.clone()).collect();
        messung.resultat_evidence_ids =
            resultat.iter().map(|s| s.evidence_id.clone()).collect();

        // ── Die Guardrails (M-45, Befund R19) ───────────────────────────
        //
        // Die Runde 1 modellierte ausschliesslich Abdeckung und
        // Konfidenzklasse; Verschlechterungen von Loudness, Peak, Transient,
        // Breite und geschuetzten Bereichen konnten weder eingelesen noch
        // gespeichert werden und erschienen bei unveraenderter Coverage als
        // STABIL. Jede Groesse ist `Option`: „nicht gemessen" ist etwas
        // anderes als „unveraendert".
        let mittel = |v: &[f64]| -> Option<f64> {
            let e: Vec<f64> = v.iter().copied().filter(|x| x.is_finite()).collect();
            (!e.is_empty()).then(|| e.iter().sum::<f64>() / e.len() as f64)
        };
        // LOUDNESS: das mittlere Bandniveau in dB.
        let basis_laut = mittel(
            &basis_p50
                .iter()
                .zip(basis_gueltig.iter())
                .filter(|(_, g)| **g)
                .map(|(v, _)| *v)
                .collect::<Vec<f64>>(),
        );
        let res_laut = mittel(
            &res_p50
                .iter()
                .zip(res_gueltig.iter())
                .filter(|(_, g)| **g)
                .map(|(v, _)| *v)
                .collect::<Vec<f64>>(),
        );
        messung.guardrail_loudness_db = match (basis_laut, res_laut) {
            (Some(a), Some(b)) => Some(b - a),
            _ => None,
        };
        // PEAK: das hoechste P95 ueber alle Baender - der Spitzenwert des
        // Fensters. Er kann steigen, waehrend der Median steht.
        let (basis_p95, basis_p95_g) = mittel_je_band(&baseline, true);
        let (res_p95, res_p95_g) = mittel_je_band(&resultat, true);
        let hoechstes = |w: &[f64], g: &[bool]| -> Option<f64> {
            w.iter()
                .zip(g.iter())
                .filter(|(_, ok)| **ok)
                .map(|(v, _)| *v)
                .filter(|v| v.is_finite())
                .fold(None, |m: Option<f64>, v| Some(m.map_or(v, |x| x.max(v))))
        };
        messung.guardrail_peak_db = match (
            hoechstes(&basis_p95, &basis_p95_g),
            hoechstes(&res_p95, &res_p95_g),
        ) {
            (Some(a), Some(b)) => Some(b - a),
            _ => None,
        };
        // TRANSIENT: die Onsetstaerke - die zweite, unabhaengige Spur (§38.2).
        let onset = |satz: &[&super::evidenz::Evidenzstand]| -> Option<f64> {
            mittel(&satz.iter().map(|s| s.onset as f64).collect::<Vec<f64>>())
        };
        messung.guardrail_transient = match (onset(&baseline), onset(&resultat)) {
            (Some(a), Some(b)) => Some(b - a),
            _ => None,
        };
        // BREITE: der Seitenanteil in dB. `None` heisst „diese Quellen liefern
        // keine Stereoauskunft", nie „unveraendert".
        let breite = |satz: &[&super::evidenz::Evidenzstand]| -> Option<f64> {
            mittel(
                &satz
                    .iter()
                    .filter_map(|s| s.seitenanteil_db)
                    .collect::<Vec<f64>>(),
            )
        };
        messung.guardrail_breite_db = match (breite(&baseline), breite(&resultat)) {
            (Some(a), Some(b)) => Some(b - a),
            _ => None,
        };
        // GESCHUETZTE BEREICHE: die groesste Bewegung AUSSERHALB der staerksten
        // Zielbewegung. Das Ziel ist das Band mit dem groessten Betrag; alles
        // andere soll stehen bleiben, und was sich dort am weitesten bewegt,
        // ist die Zahl, die niemand ignorieren darf.
        let mut sortiert: Vec<(usize, f64)> = messung
            .band_delta_db
            .iter()
            .enumerate()
            .filter(|(b, _)| messung.band_gueltig.get(*b).copied().unwrap_or(false))
            .map(|(b, d)| (b, d.abs()))
            .collect();
        if sortiert.len() >= 2 {
            sortiert.sort_by(|a, b| b.1.partial_cmp(&a.1).unwrap_or(std::cmp::Ordering::Equal));
            let ziel = sortiert[0].0;
            messung.guardrail_geschuetzt_db = sortiert
                .iter()
                .filter(|(b, _)| *b != ziel)
                .map(|(_, d)| *d)
                .fold(None, |m: Option<f64>, v| Some(m.map_or(v, |x| x.max(v))));
        }

        // ── Comparability im PRODUKTPFAD (M-46, Befund R18) ─────────────
        //
        // `vergleichbarkeit::beurteile` hatte ausserhalb seiner eigenen Tests
        // keinen Aufrufer; `Achsenrechnung::vergleichbarkeit` blieb
        // `Default::default()`, und weil `hat_resultat()` sie nicht verlangte,
        // konnte ein Terminal mit `vergleichbarkeit = None` entstehen — Gate 6
        // wirkte damit gar nicht.
        let beleg = |satz: &[&super::evidenz::Evidenzstand], abdeckung: f64| Passagenbeleg {
            projekt_start: satz
                .iter()
                .filter_map(|s| s.project_sample_start)
                .min()
                .unwrap_or(passage.projekt_von),
            projekt_ende: satz
                .iter()
                .filter_map(|s| {
                    s.project_sample_start
                        .map(|v| v.saturating_add(s.sample_count as i64))
                })
                .max()
                .unwrap_or(passage.projekt_bis),
            fingerprint: Some(passage.fingerprint.clone()),
            aktive_quellen: passage.aktive_quellen.clone(),
            samplerate: satz.last().map(|s| s.sample_rate).unwrap_or(0.0),
            messpunktklassen: passage.messpunktklassen.clone(),
            abdeckung: abdeckung as f32,
        };
        let urteil = crate::coordinator::vergleichbarkeit::beurteile(
            &beleg(&baseline, messung.abdeckung_baseline),
            &beleg(&resultat, messung.abdeckung_resultat),
        );
        messung.vergleichbarkeit = Some(urteil.klasse.name().to_owned());
        messung.vergleichbarkeit_gruende = urteil
            .gruende
            .iter()
            .map(|g| format!("{g:?}"))
            .collect();
        messung
    }

    // ── Wire zu Domaene ────────────────────────────────────────────────

    /// Die vollstaendige Passage aus `experiment_begin.passage` (M-25, B23).
    pub(super) fn passage_aus_wert(wert: &Value) -> Option<Passage> {
        let p = wert.get("passage")?;
        let quellen: Vec<String> = p
            .get("aktive_quellen")?
            .as_array()?
            .iter()
            .filter_map(|v| v.as_str().map(str::to_owned))
            .collect();
        if quellen.is_empty() {
            return None;
        }
        // 🔑 Nacharbeit 2 (Befund R23): die Messpunktklassen werden GELESEN.
        //
        // Das Schema fuehrt sie parallel zu `aktive_quellen` und in DERSELBEN
        // Reihenfolge — die Zuordnung Quelle→Messpunkt ist Teil des Belegs
        // (M-28/M-55). Eine Liste anderer Laenge ist deshalb keine
        // Zuordnung, sondern zwei unabhaengige Listen: fail-closed.
        let klassen: Vec<String> = p
            .get("messpunktklassen")?
            .as_array()?
            .iter()
            .filter_map(|v| v.as_str().map(str::to_owned))
            .collect();
        if klassen.len() != quellen.len() {
            return None;
        }
        let von = p.get("projekt_von")?.as_i64()?;
        let bis = p.get("projekt_bis")?.as_i64()?;
        if bis <= von {
            return None;
        }
        Some(Passage {
            passage_id: p.get("passage_id")?.as_str()?.to_owned(),
            projekt_von: von,
            projekt_bis: bis,
            transport_epoch: p.get("transport_epoch")?.as_u64()?,
            aktive_quellen: quellen,
            messpunktklassen: klassen,
            abdeckung: p.get("abdeckung")?.as_f64()? as f32,
            label: p.get("label").and_then(Value::as_str).map(str::to_owned),
            fingerprint: Self::fingerprint_aus_wert(p.get("fingerprint"))?,
        })
    }

    fn referenz_aus_wert(wert: Option<&Value>) -> Option<Experimentreferenz> {
        let r = wert?;
        Some(Experimentreferenz {
            passage_fingerprint: Self::fingerprint_aus_wert(r.get("passage_fingerprint"))?,
            upstream_fingerprint: Self::fingerprint_aus_wert(r.get("upstream_fingerprint"))?,
            aktive_quellen: r
                .get("aktive_quellen")?
                .as_array()?
                .iter()
                .filter_map(|v| v.as_str().map(str::to_owned))
                .collect(),
            messpunktklassen: r
                .get("messpunktklassen")?
                .as_array()?
                .iter()
                .filter_map(|v| v.as_str().map(str::to_owned))
                .collect(),
            match_gain_db: r.get("match_gain_db")?.as_f64()?,
            // Befund C5: fehlt das Feld, bleibt die Zahl UNBEKANNT. Ein
            // Vorgabewert 0 behauptete „nachweislich keines".
            nicht_endliche_samples: r.get("nicht_endliche_samples").and_then(Value::as_u64),
            alignment: match r.get("alignment").and_then(Value::as_str) {
                Some("feature_aligned") => Alignmentwert::FeatureAligned,
                Some("audio_aligned") => Alignmentwert::AudioAligned,
                Some("probable") => Alignmentwert::Probable,
                _ => Alignmentwert::Unclear,
            },
        })
    }

    fn fingerprint_aus_wert(wert: Option<&Value>) -> Option<Fingerprintwerte> {
        let f = wert?;
        let byte_feld = |name: &str, n: usize| -> Option<Vec<u8>> {
            let a = f.get(name)?.as_array()?;
            if a.len() != n {
                return None;
            }
            a.iter()
                .map(|v| v.as_u64().and_then(|x| u8::try_from(x).ok()))
                .collect()
        };
        let mut werte = Fingerprintwerte {
            version: f.get("version").and_then(Value::as_u64).unwrap_or(1) as u16,
            ..Default::default()
        };
        let band = byte_feld("band_energie", werte.band_energie.len())?;
        let chroma = byte_feld("chroma", werte.chroma.len())?;
        let onset = byte_feld("onset", werte.onset.len())?;
        werte.band_energie.copy_from_slice(&band);
        werte.chroma.copy_from_slice(&chroma);
        werte.onset.copy_from_slice(&onset);
        Some(werte)
    }

    // ── Wiederherstellung aus dem Store (M-47/M-50, Befund R12) ──────────
    //
    // 🔑 Nacharbeit 2: `Coordinator::mit_store` restaurierte ausschliesslich
    // die Konfliktriegel. Experimentstore, Passagen und Evidenzhistorie
    // starteten IMMER leer, und es gab keinen einzigen Leser der Tabellen
    // `passages`, `experiments` und `evidence`. Nach Drop und Neuerzeugung
    // lieferte `experiment_sicht(id)` deshalb `None`, obwohl die SQLite-Zeile
    // existierte — waehrend M-47 ausdruecklich zusagt, dass ein Brokerneustart
    // einen offenen Versuch NICHT abbricht.
    //
    // Fail-closed heisst hier: eine Zeile, die sich nicht vollstaendig lesen
    // laesst, wird UEBERGANGEN und nicht halb wiederhergestellt. Ein halber
    // Versuch waere schlimmer als keiner: er saehe aus wie ein ganzer.

    pub(super) fn stand_aus_store_wiederherstellen(stand: &mut Stand, store: &StoreHandle) {
        let passagen: Vec<Passage> = store
            .domaene_lesen(Domaenentabelle::Passages)
            .unwrap_or_default()
            .iter()
            .filter_map(|bytes| serde_json::from_slice::<Value>(bytes).ok())
            .filter_map(|w| Self::passage_aus_gespeichertem(&w))
            .collect();
        let experimente: Vec<crate::coordinator::experiment::Experiment> = store
            .domaene_lesen(Domaenentabelle::Experiments)
            .unwrap_or_default()
            .iter()
            .filter_map(|bytes| serde_json::from_slice::<Value>(bytes).ok())
            .filter_map(|w| Self::experiment_aus_gespeichertem(&w))
            .collect();
        stand.experimente = crate::coordinator::experiment::Experimentstore::wiederherstellen(
            passagen,
            experimente,
        );

        // Die Evidenzhistorie derselben Ablage. Sie traegt die Belege, aus
        // denen `resultatmessung` Baseline und Resultat bildet — ohne sie
        // koennte ein wiederhergestellter offener Versuch nach dem Neustart
        // nicht abgeschlossen werden.
        for bytes in store
            .domaene_lesen(Domaenentabelle::Evidence)
            .unwrap_or_default()
        {
            let Ok(zeile) = serde_json::from_slice::<Value>(&bytes) else {
                continue;
            };
            let Some(snapshot) = zeile.get("snapshot") else {
                continue;
            };
            let Ok(adresse) = serde_json::from_value::<Adresse>(snapshot["adresse"].clone()) else {
                continue;
            };
            let mut eintrag = Self::evidenzstand_aus_wert(snapshot);
            // Der Ausschlussgrund steht NEBEN dem Snapshot: er ist eine
            // Aussage ueber den Beleg, nicht Teil der Wire-Wahrheit.
            if let Some(grund) = zeile.get("ausschlussgrund").and_then(Value::as_str) {
                eintrag.ausschlussgrund = Some(grund.to_owned());
            }
            let key = ClientKey::aus_adresse(&adresse);
            let historie = stand.evidenz.entry(key).or_default();
            historie.push_back(eintrag);
            while historie.len() > EVIDENZ_RETENTION {
                historie.pop_front();
            }
        }
    }

    fn fingerprint_aus_gespeichertem(wert: Option<&Value>) -> Option<Fingerprintwerte> {
        Self::fingerprint_aus_wert(wert)
    }

    fn passage_aus_gespeichertem(w: &Value) -> Option<Passage> {
        let quellen: Vec<String> = w
            .get("aktive_quellen")?
            .as_array()?
            .iter()
            .filter_map(|v| v.as_str().map(str::to_owned))
            .collect();
        let klassen: Vec<String> = w
            .get("messpunktklassen")?
            .as_array()?
            .iter()
            .filter_map(|v| v.as_str().map(str::to_owned))
            .collect();
        Some(Passage {
            passage_id: w.get("passage_id")?.as_str()?.to_owned(),
            projekt_von: w.get("projekt_von")?.as_i64()?,
            projekt_bis: w.get("projekt_bis")?.as_i64()?,
            transport_epoch: w.get("transport_epoch")?.as_u64()?,
            aktive_quellen: quellen,
            messpunktklassen: klassen,
            abdeckung: w.get("abdeckung")?.as_f64()? as f32,
            label: w.get("label").and_then(Value::as_str).map(str::to_owned),
            fingerprint: Self::fingerprint_aus_gespeichertem(w.get("fingerprint"))?,
        })
    }

    fn referenz_aus_gespeichertem(w: Option<&Value>) -> Option<Experimentreferenz> {
        let r = w?;
        Some(Experimentreferenz {
            passage_fingerprint: Self::fingerprint_aus_gespeichertem(
                r.get("passage_fingerprint"),
            )?,
            upstream_fingerprint: Self::fingerprint_aus_gespeichertem(
                r.get("upstream_fingerprint"),
            )?,
            aktive_quellen: r
                .get("aktive_quellen")?
                .as_array()?
                .iter()
                .filter_map(|v| v.as_str().map(str::to_owned))
                .collect(),
            messpunktklassen: r
                .get("messpunktklassen")?
                .as_array()?
                .iter()
                .filter_map(|v| v.as_str().map(str::to_owned))
                .collect(),
            match_gain_db: r.get("match_gain_db")?.as_f64()?,
            // Befund C5: der restaurierte Stand traegt die Zahl mit; fehlt sie,
            // bleibt sie UNBEKANNT statt still 0 zu werden.
            nicht_endliche_samples: r.get("nicht_endliche_samples").and_then(Value::as_u64),
            alignment: match r.get("alignment").and_then(Value::as_str) {
                Some("feature_aligned") => Alignmentwert::FeatureAligned,
                Some("audio_aligned") => Alignmentwert::AudioAligned,
                Some("probable") => Alignmentwert::Probable,
                _ => Alignmentwert::Unclear,
            },
        })
    }

    fn reihenfolge_aus_wort(wort: Option<&str>) -> Option<Blindreihenfolge> {
        match wort {
            Some("kandidat_zuerst") => Some(Blindreihenfolge::KandidatZuerst),
            Some("baseline_zuerst") => Some(Blindreihenfolge::BaselineZuerst),
            _ => None,
        }
    }

    fn experiment_aus_gespeichertem(
        w: &Value,
    ) -> Option<crate::coordinator::experiment::Experiment> {
        let terminal = match w.pointer("/terminal/art").and_then(Value::as_str) {
            Some("ergebnis") => Some(Terminal::Ergebnis {
                hoerurteil: match w.pointer("/terminal/hoerurteil").and_then(Value::as_str) {
                    Some("baseline") => Hoerurteil::Baseline,
                    Some("kandidat") => Hoerurteil::Kandidat,
                    Some("kein_unterschied") => Hoerurteil::KeinUnterschied,
                    _ => Hoerurteil::Enthaltung,
                },
                blindreihenfolge: Self::reihenfolge_aus_wort(
                    w.pointer("/terminal/blindreihenfolge").and_then(Value::as_str),
                )
                .unwrap_or(Blindreihenfolge::BaselineZuerst),
                notiz: w
                    .pointer("/terminal/notiz")
                    .and_then(Value::as_str)
                    .map(str::to_owned),
                werkzeug: w
                    .pointer("/terminal/werkzeug")
                    .and_then(Value::as_str)
                    .map(str::to_owned),
                achsen: Self::achsen_aus_gespeichertem(w.pointer("/terminal/achsen")),
            }),
            Some("abgebrochen") => Some(Terminal::Abgebrochen {
                grund: match w.pointer("/terminal/grund").and_then(Value::as_str) {
                    Some("verdraengt") => Abbruchgrund::Verdraengt,
                    _ => Abbruchgrund::UserAbbruch,
                },
            }),
            _ => None,
        };
        let kandidaten = w
            .get("kandidaten")
            .and_then(Value::as_array)
            .map(|a| {
                a.iter()
                    .filter_map(|k| {
                        Some(crate::coordinator::experiment::Kandidat {
                            nummer: k.get("nummer")?.as_u64()? as u32,
                            referenz: Self::referenz_aus_gespeichertem(k.get("referenz"))?,
                            evidenzfolge: k
                                .get("evidenzfolge")
                                .and_then(Value::as_u64)
                                .unwrap_or(0),
                        })
                    })
                    .collect::<Vec<_>>()
            })
            .unwrap_or_default();
        let ids = |feld: &str| -> Vec<String> {
            w.get(feld)
                .and_then(Value::as_array)
                .map(|a| {
                    a.iter()
                        .filter_map(|v| v.as_str().map(str::to_owned))
                        .collect()
                })
                .unwrap_or_default()
        };
        Some(crate::coordinator::experiment::Experiment::aus_store(
            w.get("experiment_id")?.as_str()?.to_owned(),
            w.get("projektbindung")?.as_str()?.to_owned(),
            w.get("passage_id")?.as_str()?.to_owned(),
            Self::referenz_aus_gespeichertem(w.get("baseline"))?,
            kandidaten,
            terminal,
            w.get("folge").and_then(Value::as_u64).unwrap_or(0),
            Self::reihenfolge_aus_wort(
                w.get("blindreihenfolge_gebunden").and_then(Value::as_str),
            ),
            w.get("begin_evidenzfolge")
                .and_then(Value::as_u64)
                .unwrap_or(0),
            ids("baseline_evidence_ids"),
            ids("resultat_evidence_ids"),
        ))
    }

    fn achsen_aus_gespeichertem(w: Option<&Value>) -> Achsenrechnung {
        let Some(a) = w else {
            return Achsenrechnung::default();
        };
        let liste = |feld: &str| -> Vec<String> {
            a.get(feld)
                .and_then(Value::as_array)
                .map(|x| {
                    x.iter()
                        .filter_map(|v| v.as_str().map(str::to_owned))
                        .collect()
                })
                .unwrap_or_default()
        };
        Achsenrechnung {
            intervall: a.get("intervall").and_then(Value::as_array).and_then(|v| {
                Some((v.first()?.as_f64()?, v.get(1)?.as_f64()?))
            }),
            signifikante_baender: a
                .get("signifikante_baender")
                .and_then(Value::as_u64)
                .unwrap_or(0) as usize,
            gescannte_baender: a
                .get("gescannte_baender")
                .and_then(Value::as_u64)
                .unwrap_or(0) as usize,
            vergleichbarkeit: a
                .get("vergleichbarkeit")
                .and_then(Value::as_str)
                .map(str::to_owned),
            vergleichbarkeit_gruende: liste("vergleichbarkeit_gruende"),
            guardrail_abdeckung_delta: a
                .get("guardrail_abdeckung_delta")
                .and_then(Value::as_f64),
            guardrail_klasse_gefallen: a
                .get("guardrail_klasse_gefallen")
                .and_then(Value::as_bool)
                .unwrap_or(true),
            guardrail_loudness_db: a.get("guardrail_loudness_db").and_then(Value::as_f64),
            guardrail_peak_db: a.get("guardrail_peak_db").and_then(Value::as_f64),
            guardrail_transient: a.get("guardrail_transient").and_then(Value::as_f64),
            guardrail_breite_db: a.get("guardrail_breite_db").and_then(Value::as_f64),
            guardrail_geschuetzt_db: a.get("guardrail_geschuetzt_db").and_then(Value::as_f64),
            effekt_stabil: a.get("effekt_stabil").and_then(Value::as_bool),
        }
    }

    /// Der Experimentstand fuer Beine und Sichten.
    pub fn experiment_sicht(
        &self,
        experiment_id: &str,
    ) -> Option<crate::coordinator::experiment::Experiment> {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .experimente
            .experiment(experiment_id)
            .cloned()
    }

    /// Die Passage, wie der Broker sie aus dem Wire angelegt hat (M-25).
    pub fn passage_sicht(&self, passage_id: &str) -> Option<Passage> {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .experimente
            .passage(passage_id)
            .cloned()
    }

    // 🔑 Nacharbeit 2 (Befund R21): `binde_blindreihenfolge_fuer_test` ist
    // ENTFALLEN. Die Naht existierte, weil die Bindung keinen Wireweg hatte —
    // ein Bein musste sie am Store vorbei setzen, und genau deshalb konnte
    // kein Bein an der fehlenden Bindung fallen. Sie hat jetzt ihren eigenen
    // Befehlszweig (`experiment_candidate`), und der ist der einzige Weg.

    /// Der vollstaendige Export eines Versuchs (M-51).
    pub fn experiment_export(
        &self,
        experiment_id: &str,
    ) -> Option<crate::coordinator::experiment::Export> {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .experimente
            .exportiere(experiment_id)
    }
}
