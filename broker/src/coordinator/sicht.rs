//! Was der Coordinator nach aussen zeigt.
//!
//! Die Sichttypen sind Kopien eines konsistenten Standes; sie halten keinen
//! Lock und keine Referenz in den Sessiongraphen hinein.

use super::*;

/// Wie viele Versuche ein `session_snapshot` traegt.
///
/// SONDE-013 Nacharbeit 2 (Befund R14): der Wert steht im Vertrag
/// (`session_snapshot.experimente.maxItems`) und ist hier benannt statt als
/// Literal im Pfad. Er ist bewusst gleich `N_GLOBAL` — mehr OFFENE kann es
/// nach M-48 nie geben; gekappt wird also ausschliesslich Historie.
const SNAPSHOT_VERSUCHE_MAX: usize = crate::coordinator::experiment::N_GLOBAL;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Interventionssicht {
    pub aktive: usize,
    pub tail_samples_offen: u64,
    pub unknown: bool,
    pub starke_evidenz_erlaubt: bool,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ControlRegistrierung {
    pub angenommen: bool,
    pub grund: Option<String>,
    pub zu_schliessende_links: Vec<String>,
}

impl ControlRegistrierung {
    pub(super) fn angenommen(zu_schliessende_links: Vec<String>) -> Self {
        Self {
            angenommen: true,
            grund: None,
            zu_schliessende_links,
        }
    }

    pub(super) fn abgewiesen(grund: impl Into<String>) -> Self {
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

impl Coordinator {
    pub fn cap_abweisungen(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .cap_abweisungen
    }

    pub fn store_verweigerungen(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .store_verweigerungen
    }

    pub fn p2_live_frames(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .p2_live_frames
    }

    pub fn messsicht(
        &self,
        project_binding_id: &str,
        session_epoch: &str,
        instance_id: &str,
    ) -> Option<MessframeSicht> {
        let jetzt = self.clock.jetzt();
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
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
            // G2-FLOATEDGE-001, Nacharbeit Runde 2 (R2-1, 03.09.2026): die
            // Eingangspruefung in telemetrie.rs laesst `sr.is_finite() && sr >
            // 0.0` durch - eine POSITIVE SUBNORMALE Samplerate besteht sie und
            // liess die Division bei einer von null verschiedenen Samplezahl
            // zu `inf` ueberlaufen. Verriegelt UND gezaehlt wird das jetzt
            // dort, wo der Frame in den Stand kommt (`senke.rs`). Die Sicht
            // LIEST nur den gespeicherten Wert und zaehlt nichts - sonst
            // erhoehte jede Abfrage denselben Zaehler erneut und er misst
            // Lesefrequenz statt Grenzfaelle.
            fenster_ms: frame.and_then(|frame| frame.fenster_ms),
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

    /// G2-FLOATEDGE-001: das abgeleitete Analysefenster entsteht nur aus einer
    /// NORMALEN positiven Samplerate, und nur wenn das Ergebnis endlich
    /// bleibt. Subnormale, Null, negative Werte, `inf` und `NaN` liefern
    /// `None` und erhoehen `fenster_nicht_endlich`.
    ///
    /// `is_normal()` ist hier die richtige Frage und nicht `is_finite()`:
    /// subnormale Zahlen sind endlich, taugen aber nicht als Divisor - genau
    /// an ihnen lief die Division ueber.
    ///
    /// R2-1 (03.09.2026): Aufrufer ist ausschliesslich die Aufnahme eines
    /// Frames in den Stand (`senke.rs`), damit je Frame genau einmal gezaehlt
    /// wird. `pub(super)` und nicht `pub`: der Weg bleibt crate-intern.
    pub(super) fn fenster_ms_bilden(&self, sample_count: u32, sample_rate: f64) -> Option<f64> {
        if !sample_rate.is_normal() || sample_rate <= 0.0 {
            self.fenster_nicht_endlich.fetch_add(1, Ordering::Relaxed);
            return None;
        }
        let fenster = (sample_count as f64 / sample_rate) * 1000.0;
        if !fenster.is_finite() {
            self.fenster_nicht_endlich.fetch_add(1, Ordering::Relaxed);
            return None;
        }
        Some(fenster)
    }

    /// Wie oft ein abgeleitetes Analysefenster verriegelt wurde, weil die
    /// Samplerate oder das Ergebnis nicht tragfaehig war.
    pub fn fenster_nicht_endlich(&self) -> u64 {
        self.fenster_nicht_endlich.load(Ordering::Relaxed)
    }

    pub fn telemetrie_kopplungen(&self) -> (usize, u64) {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        (stand.telemetry_links.len(), stand.telemetry_kopplungen)
    }

    /// H-21: steht auf dieser effektiven Adresse ein dauerhafter
    /// Konfliktriegel? Genau diese Frage stellen die elf Guard-Stellen intern;
    /// ohne sie waere die Unempfindlichkeit gegen die Schreibweise der SID von
    /// aussen unbeobachtbar.
    pub fn konfliktriegel_gesetzt(&self, effective_address: &str) -> bool {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .guard_gesetzt(effective_address)
    }

    /// H-12: Groesse der Sessionmap. Ohne sie waere die Zusage
    /// „Session faellt mit dem letzten Client" von aussen unbeobachtbar - die
    /// Modellsicht zeigt Clients, nicht die Map selbst.
    pub fn session_anzahl(&self) -> usize {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .sessions
            .len()
    }

    pub fn client_anzahl(&self) -> usize {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .clients
            .values()
            .filter(|client| client.current_link.is_some())
            .count()
    }

    pub fn subscription_anzahl(&self) -> usize {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .subscriptions
            .len()
    }

    pub fn subscription_cleanups(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .subscription_cleanups
    }

    pub fn subscription_abweisungen(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .subscription_abweisungen
    }

    pub fn letzter_subscription_grund(&self) -> String {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .letzter_subscription_grund
            .clone()
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
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
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
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        self.snapshot_locked(&stand, &session)
    }

    /// Ein Versuch in der Form, die Gen anzeigt (M-46/M-49, Befund R14).
    ///
    /// Wie viele davon der Snapshot traegt, steht in `SNAPSHOT_VERSUCHE_MAX`.
    ///
    /// Er traegt NUR, was Gen zeigt: welche Transition zuletzt geschah, ob er
    /// noch offen ist, das Userurteil, die aufgedeckte Reihenfolge, die
    /// Comparability und die eine der fuenf zulaessigen Aussagen. Die
    /// Rohdeltas und die Evidence-IDs bleiben im Store — ein Snapshot ist eine
    /// Sicht, kein Export.
    fn session_experiment_json(e: &crate::coordinator::experiment::Experiment) -> Value {
        use crate::coordinator::experiment::{Abbruchgrund, Terminal};
        let mut objekt = serde_json::Map::new();
        objekt.insert("experiment_id".into(), Value::String(e.experiment_id.clone()));
        objekt.insert("offen".into(), Value::Bool(e.offen()));
        let ereignis = match &e.terminal {
            Some(Terminal::Ergebnis { .. }) => "ergebnis",
            Some(Terminal::Abgebrochen {
                grund: Abbruchgrund::Verdraengt,
            }) => "verdraengt",
            Some(Terminal::Abgebrochen { .. }) => "abgebrochen",
            None if !e.kandidaten.is_empty() => "kandidat",
            None => "begonnen",
        };
        objekt.insert("ereignis".into(), Value::String(ereignis.into()));
        if let Some(Terminal::Ergebnis {
            hoerurteil,
            achsen,
            ..
        }) = &e.terminal
        {
            objekt.insert(
                "hoerurteil".into(),
                Value::String(
                    match hoerurteil {
                        crate::coordinator::experiment::Hoerurteil::Baseline => "baseline",
                        crate::coordinator::experiment::Hoerurteil::Kandidat => "kandidat",
                        crate::coordinator::experiment::Hoerurteil::KeinUnterschied => {
                            "kein_unterschied"
                        }
                        crate::coordinator::experiment::Hoerurteil::Enthaltung => "enthaltung",
                    }
                    .into(),
                ),
            );
            // Die AUFGEDECKTE Reihenfolge — sie kommt ausschliesslich ueber
            // `aufgedeckte_reihenfolge()`, und das gibt sie erst nach dem
            // Urteil heraus (M-44).
            if let Some(r) = e.aufgedeckte_reihenfolge() {
                objekt.insert(
                    "blindreihenfolge".into(),
                    Value::String(
                        match r {
                            crate::coordinator::experiment::Blindreihenfolge::KandidatZuerst => {
                                "kandidat_zuerst"
                            }
                            crate::coordinator::experiment::Blindreihenfolge::BaselineZuerst => {
                                "baseline_zuerst"
                            }
                        }
                        .into(),
                    ),
                );
            }
            if let Some(v) = &achsen.vergleichbarkeit {
                objekt.insert("vergleichbarkeit".into(), Value::String(v.clone()));
            }
            // Gate 6 (§49.2): ein nicht vergleichbares Experiment erhaelt kein
            // starkes Siegerurteil — und `urteile` setzt genau das durch.
            let vergleichbar = achsen.vergleichbarkeit.as_deref() == Some("stark");
            let urteil = crate::coordinator::experiment::urteile(
                &achsen.befunde(Some(*hoerurteil)),
                vergleichbar,
            );
            objekt.insert(
                "urteil".into(),
                Value::String(
                    match urteil {
                        crate::coordinator::experiment::Urteil::ZielVerbessertGuardrailsStabil => {
                            "ziel_verbessert_guardrails_stabil"
                        }
                        crate::coordinator::experiment::Urteil::MessbarAndersUrteilOffen => {
                            "messbar_anders_urteil_offen"
                        }
                        crate::coordinator::experiment::Urteil::KeineBelastbareAenderung => {
                            "keine_belastbare_aenderung"
                        }
                        crate::coordinator::experiment::Urteil::ZielVerbessertGeschuetztesSchlechter => {
                            "ziel_verbessert_geschuetztes_schlechter"
                        }
                        crate::coordinator::experiment::Urteil::VergleichNichtGueltig => {
                            "vergleich_nicht_gueltig"
                        }
                    }
                    .into(),
                ),
            );
        }
        Value::Object(objekt)
    }

    /// Ein Paarurteil in der Form, die Gen anzeigt (M-13/M-22, Befund R32).
    fn session_paar_json(u: &crate::coordinator::prepost::Paarurteil) -> Value {
        use crate::coordinator::prepost::{Alignmentklasse, Ausschlussgrund, Kettenbefund};
        let mut objekt = serde_json::Map::new();
        objekt.insert("pair_id".into(), Value::String(u.pair_id.clone()));
        objekt.insert(
            "klasse".into(),
            Value::String(
                match u.klasse {
                    Alignmentklasse::FeatureAligned => "feature_aligned",
                    Alignmentklasse::AudioAligned => "audio_aligned",
                    Alignmentklasse::Probable => "probable",
                    Alignmentklasse::Unclear => "unclear",
                }
                .into(),
            ),
        );
        objekt.insert(
            "kettenbefund".into(),
            Value::String(
                match u.kettenbefund {
                    Kettenbefund::Stationaer => "stationaer",
                    Kettenbefund::ZeitvariabelMarkiert => "zeitvariabel_markiert",
                    Kettenbefund::PegelabhaengigMarkiert => "pegelabhaengig_markiert",
                    Kettenbefund::LatenzWechseltMarkiert => "latenz_wechselt_markiert",
                    Kettenbefund::NichtBeurteilbar => "nicht_beurteilbar",
                }
                .into(),
            ),
        );
        if let Some(a) = u.ausschluss {
            objekt.insert(
                "ausschluss".into(),
                Value::String(
                    match a {
                        Ausschlussgrund::HaelfteFehlt => "haelfte_fehlt",
                        Ausschlussgrund::Paarkonflikt => "paarkonflikt",
                        Ausschlussgrund::HaelfteGetrennt => "haelfte_getrennt",
                        Ausschlussgrund::HaelfteStale => "haelfte_stale",
                        Ausschlussgrund::NichtMessbereit => "nicht_messbereit",
                        Ausschlussgrund::KeineProjektzeit => "keine_projektzeit",
                        Ausschlussgrund::FensterLeer => "fenster_leer",
                        Ausschlussgrund::SamplerateVerschieden => "samplerate_verschieden",
                        Ausschlussgrund::KeineUeberlappung => "keine_ueberlappung",
                        Ausschlussgrund::SprungImFenster => "sprung_im_fenster",
                    }
                    .into(),
                ),
            );
        }
        Value::Object(objekt)
    }

    pub(super) fn snapshot_locked(&self, stand: &Stand, session: &SessionKey) -> Vec<u8> {
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
        // 🔑 SONDE-013 Nacharbeit 2 (Befunde R14/R32): die Versuche und die
        // PRE/POST-Paarurteile der Sitzung reisen MIT.
        //
        // Das Terminal bekam bis dahin gar keine `snapshot_ziele`, und das
        // Paarurteil endete in einer fluechtigen Map — beide erreichten Gen
        // NIE. Der Rueckweg laeuft ueber den bestehenden Outbox-/Snapshot-Pfad
        // statt ueber eine weitere Familie; genau das sagt die Beschreibung von
        // `experiment_manual_result` im Vertrag zu (§53.9).
        let objekt = payload
            .as_object_mut()
            .expect("session_snapshot ist ein Objekt");
        // Der Deckel des Vertrages ist 32 (`session_snapshot.experimente`),
        // und der Bestandsdeckel M-48 deckelt nur die OFFENEN. Abgeschlossene
        // sammeln sich in derselben Ablage: eine lange Sitzung uebersteigt 32
        // ohne Weiteres. Ein Snapshot mit 33 Versuchen waere vertragswidrig,
        // und der Leser wuerde ihn GANZ verwerfen - eine ungedeckelte Liste
        // haette den Rueckweg genau dann gekappt, wenn er am meisten traegt.
        //
        // Gekappt wird am ALTEN Ende: die juengsten Versuche sind die, an
        // denen der User arbeitet. Die Ordnung im Snapshot bleibt die
        // Anlegereihenfolge — ein Schnitt, der auch die Reihenfolge dreht,
        // waere zweimal Wahrheitsverlust.
        let mut versuche: Vec<&crate::coordinator::experiment::Experiment> = stand
            .experimente
            .alle_im_projekt(&session.project_binding_id)
            .collect();
        versuche.sort_by_key(|e| e.folge);
        let ueberzaehlig = versuche.len().saturating_sub(SNAPSHOT_VERSUCHE_MAX);
        let experimente: Vec<Value> = versuche[ueberzaehlig..]
            .iter()
            .map(|e| Self::session_experiment_json(e))
            .collect();
        if !experimente.is_empty() {
            objekt.insert("experimente".into(), Value::Array(experimente));
        }
        let mut paare: Vec<(String, Value)> = stand
            .paarurteile
            .iter()
            .filter(|((s, _), _)| s == session)
            .map(|((_, id), urteil)| (id.clone(), Self::session_paar_json(urteil)))
            .collect();
        // Feste Ordnung: eine `HashMap` hat keine, und ein Snapshot, dessen
        // Reihenfolge sich von Lauf zu Lauf dreht, waere nicht reproduzierbar.
        paare.sort_by(|a, b| a.0.cmp(&b.0));
        if !paare.is_empty() {
            objekt.insert(
                "paare".into(),
                Value::Array(paare.into_iter().map(|(_, w)| w).collect()),
            );
        }
        if self.store_degradiert() {
            objekt.insert("store_degraded".into(), Value::Bool(true));
        }
        serde_json::to_vec(&payload).unwrap_or_else(|_| b"{}".to_vec())
    }
}
