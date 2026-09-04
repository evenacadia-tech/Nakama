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
    Passage, Resultatmessung,
};
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
        let ablehnung = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
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
                        if e.reihenfolge_gebunden()
                            && e.gebundene_reihenfolge_fuer_pruefung() != Some(gemeldet)
                        {
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
                                .map(|s| self.resultatmessung(s))
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

    /// Persistenz und Wirkung — der Teil nach der Vorpruefung.
    fn experiment_p0_weiter(
        &self,
        link_id: &str,
        wert: &Value,
        art: &str,
        experiment_id: &str,
    ) -> Option<Vec<u8>> {
        let kopf = wert.get("kopf")?;
        // ── Persistenz des Befehls (Idempotenz, Autorisierung, Outbox) ──
        let ack = self.persistenz_p0(link_id, wert)?;
        if !Self::ack_ist_angewandt(&ack) {
            // `idempotent_wiederholt`, `konflikt` oder `abgelehnt`: die
            // Wirkung wurde bereits angewandt oder darf es nicht werden.
            return Some(ack);
        }

        // ── Die Wirkung ─────────────────────────────────────────────────
        let session = ClientKey::aus_adresse(
            &serde_json::from_value::<Adresse>(kopf.get("ziel")?.clone()).ok()?,
        )
        .session();
        match art {
            "experiment_begin" => self.experiment_begin_anwenden(wert, experiment_id, &session),
            "experiment_abort" => {
                let grund = match wert.get("grund").and_then(Value::as_str) {
                    Some("verdraengt") => Abbruchgrund::Verdraengt,
                    _ => Abbruchgrund::UserAbbruch,
                };
                self.experiment_terminal_anwenden(experiment_id, &session, |stand| {
                    stand.experimente.schliesse(experiment_id, grund).is_ok()
                });
            }
            "experiment_manual_result" => {
                self.experiment_ergebnis_anwenden(wert, experiment_id, &session);
            }
            _ => {}
        }
        Some(ack)
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

    fn experiment_begin_anwenden(&self, wert: &Value, experiment_id: &str, session: &SessionKey) {
        let Some(passage) = Self::passage_aus_wert(wert) else {
            return;
        };
        let Some(baseline) = Self::referenz_aus_wert(wert.get("referenz")) else {
            return;
        };
        let passage_id = passage.passage_id.clone();
        let passage_kopie = passage.clone();
        let angelegt = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand
                .experimente
                .beginne(
                    experiment_id,
                    &session.project_binding_id,
                    passage,
                    baseline.clone(),
                )
                .is_ok()
        };
        if !angelegt {
            return;
        }
        // Die Passage zuerst: sie ist das Objekt, auf das der Versuch zeigt.
        self.domaene_persistieren(
            session,
            "passage",
            serde_json::json!({
                "passage_id": passage_id,
                "projekt_von": passage_kopie.projekt_von,
                "projekt_bis": passage_kopie.projekt_bis,
                "transport_epoch": passage_kopie.transport_epoch,
                "aktive_quellen": passage_kopie.aktive_quellen,
                "abdeckung": passage_kopie.abdeckung,
                "label": passage_kopie.label,
            }),
        );
        self.domaene_persistieren(
            session,
            "experiment",
            serde_json::json!({
                "experiment_id": experiment_id,
                "ereignis": "begonnen",
                "passage_id": passage_id,
                "match_gain_db": baseline.match_gain_db,
                "aktive_quellen": baseline.aktive_quellen,
                "messpunktklassen": baseline.messpunktklassen,
            }),
        );
    }

    fn experiment_ergebnis_anwenden(&self, wert: &Value, experiment_id: &str, session: &SessionKey) {
        let hoerurteil = match wert.get("hoerurteil").and_then(Value::as_str) {
            Some("baseline") => Hoerurteil::Baseline,
            Some("kandidat") => Hoerurteil::Kandidat,
            Some("kein_unterschied") => Hoerurteil::KeinUnterschied,
            _ => Hoerurteil::Enthaltung,
        };
        let reihenfolge = match wert.get("blindreihenfolge").and_then(Value::as_str) {
            Some("kandidat_zuerst") => Blindreihenfolge::KandidatZuerst,
            _ => Blindreihenfolge::BaselineZuerst,
        };
        let notiz = wert
            .get("notiz")
            .and_then(Value::as_str)
            .map(str::to_owned);
        let werkzeug = wert
            .get("werkzeug")
            .and_then(Value::as_str)
            .map(str::to_owned);

        let messung = self.resultatmessung(session);
        let achsen = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            // Die Blindreihenfolge, die auf der Leitung steht, ist die
            // AUFGEDECKTE. Sie wurde vor dem Urteil gebunden; ist sie das
            // hier noch nicht, bindet dieser Aufruf sie — sonst koennte ein
            // Ergebnis nie abschliessen (M-44).
            let _ = stand
                .experimente
                .binde_reihenfolge(experiment_id, reihenfolge);
            stand
                .experimente
                .ergebnis(experiment_id, hoerurteil, notiz, werkzeug, &messung)
                .ok()
        };
        let Some(achsen) = achsen else {
            // Ohne Resultatmessung gibt es kein Terminal (M-45, Befund B20).
            // Der Befehl ist persistiert, die Wirkung bleibt aus — und genau
            // das ist die ehrliche Antwort: der Versuch bleibt OFFEN.
            return;
        };
        self.taint_von_experiment_schliessen(session, experiment_id);
        self.experiment_terminal_persistieren(session, experiment_id, "ergebnis", Some(&achsen));
    }

    fn experiment_terminal_anwenden(
        &self,
        experiment_id: &str,
        session: &SessionKey,
        wirkung: impl FnOnce(&mut Stand) -> bool,
    ) {
        let ok = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            wirkung(&mut stand)
        };
        if !ok {
            return;
        }
        self.taint_von_experiment_schliessen(session, experiment_id);
        self.experiment_terminal_persistieren(session, experiment_id, "abgebrochen", None);
    }

    /// M-59, Befund B22: JEDES Terminal schliesst die zugehoerigen
    /// `art=experiment`-Taintintervalle.
    ///
    /// Vorher mutierten Resultat, Abbruch und Verdraengung ausschliesslich den
    /// isolierten Store, und Interventionen trugen gar keine
    /// Experimentzuordnung. Die Intervalle blieben nach jedem Terminal offen —
    /// und mit ihnen die Sperre auf starker Evidenz.
    fn taint_von_experiment_schliessen(&self, session: &SessionKey, experiment_id: &str) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        Self::taint_intervalle_schliessen(&mut stand, session, "experiment", Some(experiment_id));
    }

    fn experiment_terminal_persistieren(
        &self,
        session: &SessionKey,
        experiment_id: &str,
        ereignis: &str,
        achsen: Option<&Achsenrechnung>,
    ) {
        let (baseline_ids, resultat_ids) = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand
                .experimente
                .experiment(experiment_id)
                .map(|e| {
                    (
                        e.baseline_evidence_ids.clone(),
                        e.resultat_evidence_ids.clone(),
                    )
                })
                .unwrap_or_default()
        };
        self.domaene_persistieren(
            session,
            "experiment",
            serde_json::json!({
                "experiment_id": experiment_id,
                "ereignis": ereignis,
                // M-51: die Belege reisen MIT dem Terminalereignis.
                "baseline_evidence_ids": baseline_ids,
                "resultat_evidence_ids": resultat_ids,
                "achsen": achsen.map(|a| serde_json::json!({
                    "intervall": a.intervall.map(|(u, o)| vec![u, o]),
                    "signifikante_baender": a.signifikante_baender,
                    "gescannte_baender": a.gescannte_baender,
                    "vergleichbarkeit": a.vergleichbarkeit,
                    "guardrail_abdeckung_delta": a.guardrail_abdeckung_delta,
                    "guardrail_klasse_gefallen": a.guardrail_klasse_gefallen,
                    "effekt_stabil": a.effekt_stabil,
                })),
            }),
        );
    }

    /// Ein Domaenenereignis in den Store (M-47/M-50).
    fn domaene_persistieren(&self, session: &SessionKey, event_type: &str, payload: Value) {
        let Some(store) = self.store.as_ref() else {
            return;
        };
        let Ok(payload_jcs) = serde_json_canonicalizer::to_vec(&payload) else {
            return;
        };
        let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
        let mut event = StoreEvent::session_snapshot(
            &session.project_binding_id,
            &session.session_epoch,
            &self.broker_epoch,
            sequence.min(i64::MAX as u64) as i64,
            payload_jcs,
        );
        event.event_type = event_type.to_owned();
        if store.append(vec![event]).is_err() {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
        }
    }

    /// Die Resultatmessung aus dem laufenden Evidenzbestand (M-45/M-49).
    ///
    /// Baseline ist die AELTERE Haelfte der behaltenen Historie, Resultat die
    /// juengere. Das ist die ehrliche Zuordnung ohne zusaetzliche Wirefelder:
    /// die Baseline wurde vor der Fremdaenderung gemessen, das Resultat
    /// danach, und dazwischen liegt kein weiterer Beleg.
    pub(super) fn resultatmessung(&self, session: &SessionKey) -> Resultatmessung {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let mut messung = Resultatmessung::default();
        // Die Quelle mit der laengsten Historie in dieser Sitzung traegt die
        // Messung. Mehrere zu mitteln waere eine Aussage ueber ein Gemisch.
        let Some((_, historie)) = stand
            .evidenz
            .iter()
            .filter(|(key, _)| key.session() == *session)
            .max_by_key(|(_, h)| h.len())
        else {
            return messung;
        };
        if historie.len() < 4 {
            // Weniger als vier Fenster sind keine zwei Haelften.
            return messung;
        }
        let mitte = historie.len() / 2;
        let alt: Vec<&super::evidenz::Evidenzstand> = historie.iter().take(mitte).collect();
        let neu: Vec<&super::evidenz::Evidenzstand> = historie.iter().skip(mitte).collect();
        let baender = neu[0].p50_db.len();
        let mittel_je_band = |satz: &[&super::evidenz::Evidenzstand]| -> (Vec<f64>, Vec<bool>) {
            let mut summe = vec![0.0f64; baender];
            let mut anzahl = vec![0usize; baender];
            for s in satz {
                if s.p50_db.len() != baender {
                    continue;
                }
                for band in 0..baender {
                    if s.p50_gueltig.get(band).copied().unwrap_or(false) {
                        summe[band] += s.p50_db[band] as f64;
                        anzahl[band] += 1;
                    }
                }
            }
            let mut werte = Vec::with_capacity(baender);
            let mut gueltig = Vec::with_capacity(baender);
            for band in 0..baender {
                if anzahl[band] > 0 {
                    werte.push(summe[band] / anzahl[band] as f64);
                    gueltig.push(true);
                } else {
                    // M-07: kein Beleg heisst Wert 0 mit `gueltig=false`.
                    werte.push(0.0);
                    gueltig.push(false);
                }
            }
            (werte, gueltig)
        };
        let (alt_werte, alt_gueltig) = mittel_je_band(&alt);
        let (neu_werte, neu_gueltig) = mittel_je_band(&neu);
        for band in 0..baender {
            let ok = alt_gueltig[band] && neu_gueltig[band];
            messung
                .band_delta_db
                .push(if ok { neu_werte[band] - alt_werte[band] } else { 0.0 });
            messung.band_gueltig.push(ok);
        }
        // Die zwei Haelften des RESULTATfensters — daraus entsteht die
        // Effektstabilitaet.
        let viertel = neu.len() / 2;
        let (a, _) = mittel_je_band(&neu[..viertel.max(1)]);
        let (b, _) = mittel_je_band(&neu[viertel.max(1)..]);
        messung.erste_haelfte = a;
        messung.zweite_haelfte = b;
        messung.abdeckung_baseline =
            alt.iter().map(|s| s.abdeckung).sum::<f64>() / alt.len().max(1) as f64;
        messung.abdeckung_resultat =
            neu.iter().map(|s| s.abdeckung).sum::<f64>() / neu.len().max(1) as f64;
        messung.klasse_baseline = alt.last().map(|s| s.klasse.clone()).unwrap_or_default();
        messung.klasse_resultat = neu.last().map(|s| s.klasse.clone()).unwrap_or_default();
        messung.baseline_evidence_ids = alt.iter().map(|s| s.evidence_id.clone()).collect();
        messung.resultat_evidence_ids = neu.iter().map(|s| s.evidence_id.clone()).collect();
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

    /// Nur Beine: bindet die Blindreihenfolge direkt am Store.
    ///
    /// Sie hat keinen eigenen Wireweg — gebunden wird sie in Gen, bevor der
    /// User hoert (M-44), und das Ergebnis meldet danach die AUFGEDECKTE.
    /// Ein Bein, das den Widerspruchsfall messen will, braucht deshalb diesen
    /// Zugang; ein Produktpfad braucht ihn nicht.
    #[doc(hidden)]
    pub fn binde_blindreihenfolge_fuer_test(&self, experiment_id: &str, wort: &str) -> bool {
        let reihenfolge = match wort {
            "kandidat_zuerst" => Blindreihenfolge::KandidatZuerst,
            _ => Blindreihenfolge::BaselineZuerst,
        };
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand
            .experimente
            .binde_reihenfolge(experiment_id, reihenfolge)
            .unwrap_or(false)
    }

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
