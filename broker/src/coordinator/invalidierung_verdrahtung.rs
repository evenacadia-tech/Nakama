//! Die Invalidierung im PRODUKTPFAD (SONDE-013 M-52 bis M-57, Befund B24).
//!
//! ## Warum dieses Modul existiert
//!
//! 🔑 Nacharbeit 1 nach der Erstpruefung 1: `invalidierung` enthielt nur reine
//! Konstruktoren. Ausserhalb seiner eigenen Tests gab es KEINEN Aufrufer,
//! KEINEN Serializer, KEINE Outbox-Einreihung und KEINE Mutation gespeicherter
//! Evidenz. Marker, Preview, Seek, Drop, Material- und Messpunktwechsel nahmen
//! im Produkt also nichts zurueck — obwohl M-52 bis M-57 genau diesen Sender-
//! und Ausschlusspfad verlangen.
//!
//! ## Was eine Invalidierung tut, und in welcher Reihenfolge
//!
//! 1. Sie MARKIERT die betroffene Evidenz im laufenden Bestand als
//!    ausgeschlossen — mit ihrem Grund, nie still.
//! 2. Sie geht als append-only `StoreEvent` in den Store, damit der Ausschluss
//!    einen Neustart ueberdauert.
//! 3. Sie wird fuer jeden Abonnenten der Sitzung in die OUTBOX eingereiht.
//!    Der Rueckweg zu Gen laeuft ueber denselben Snapshot-/Outboxpfad wie
//!    jede andere Sitzungsaenderung (§53.9) — eine eigene Zustellung waere
//!    ein zweiter Weg fuer dieselbe Zusage.
//!
//! Die Reihenfolge ist tragend: erst der lokale Ausschluss, dann die Ablage,
//! dann die Zustellung. Wer zuerst zustellte, koennte einen Ausschluss melden,
//! den sein eigener Bestand noch nicht kennt.

use super::*;
use crate::coordinator::invalidierung::{Grund, Invalidierung, Umfang};

impl Coordinator {
    /// Der EINE Weg, eine Invalidierung wirksam zu machen.
    ///
    /// Rueckgabe: wie viele gespeicherte Evidenzstaende sie ausgeschlossen
    /// hat. 0 ist kein Fehler — ein Seek, der keine Evidenz trifft, ist eine
    /// Tatsache und keine Panne.
    pub(super) fn invalidierung_anwenden(
        &self,
        session: &SessionKey,
        invalidierung: &Invalidierung,
    ) -> usize {
        if !invalidierung.gueltig() {
            // Eine leere ID-Menge oder ein leerer Bereich nehmen nichts
            // zurueck. Sie zu senden hiesse, dem Empfaenger eine Ruecknahme zu
            // melden, die nichts zurueckninmt, und ihn danach glauben zu
            // lassen, es sei aufgeraeumt (M-57).
            return 0;
        }
        // 1. Der lokale Ausschluss.
        let (betroffen, ziele) = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let mut betroffen = 0usize;
            let keys: Vec<ClientKey> = stand
                .evidenz
                .keys()
                .filter(|key| key.session() == *session)
                .cloned()
                .collect();
            for key in keys {
                let Some(historie) = stand.evidenz.get_mut(&key) else {
                    continue;
                };
                for eintrag in historie.iter_mut() {
                    if eintrag.ausschlussgrund.is_some() {
                        // Schon ausgeschlossen: der ERSTE Grund bleibt stehen.
                        // Ihn zu ueberschreiben hiesse, die Geschichte des
                        // Ausschlusses umzuschreiben.
                        continue;
                    }
                    let von = eintrag.project_sample_start.unwrap_or(i64::MIN);
                    let bis = von.saturating_add(eintrag.sample_count as i64);
                    if invalidierung
                        .umfang
                        .erfasst(&eintrag.evidence_id, von, bis)
                    {
                        eintrag.ausschlussgrund = Some(invalidierung.grund.wort().to_string());
                        betroffen += 1;
                    }
                }
            }
            stand.invalidierungen = stand.invalidierungen.saturating_add(1);
            stand.evidenz_ausgeschlossen =
                stand.evidenz_ausgeschlossen.saturating_add(betroffen as u64);
            // Die Abonnenten DIESER Sitzung — sie bekommen den Ausschluss.
            let ziele: Vec<SnapshotZiel> = stand
                .subscriptions
                .values()
                .filter(|sub| {
                    sub.session_epoch == session.session_epoch
                        && sub.adresse.project_binding_id == session.project_binding_id
                })
                .map(|sub| SnapshotZiel {
                    project_binding_id: session.project_binding_id.clone(),
                    session_epoch: session.session_epoch.clone(),
                    instance_id: sub.adresse.instance_id.clone(),
                    object_key: "evidence_invalidate".into(),
                })
                .collect();
            (betroffen, ziele)
        };

        // 2. und 3.: Ablage und Outbox in EINEM Store-Append. Der Writer legt
        // die Outboxzeilen im selben Commit an wie das Ereignis — Eventwahrheit
        // und Zustellschuld koennen so nicht auseinanderlaufen.
        self.invalidierung_persistieren(session, invalidierung, betroffen, ziele);
        betroffen
    }

    /// Die Wireform von `evidence_invalidate` (M-57).
    ///
    /// Sie steht hier und nicht beim Aufrufer: der Umfang ist ueber `art`
    /// diskriminiert, und jeder Zweig traegt genau die Felder seiner Art. Ein
    /// zweiter Serializer wuerde diese Kante frueher oder spaeter aufweichen.
    pub fn invalidierung_als_json(invalidierung: &Invalidierung) -> Value {
        let umfang = match &invalidierung.umfang {
            Umfang::Ids(ids) => serde_json::json!({
                "art": "evidence_ids",
                "evidence_ids": ids.iter().cloned().collect::<Vec<String>>(),
            }),
            Umfang::Bereich { von, bis } => serde_json::json!({
                "art": "sample_range",
                "von_sample": von,
                "bis_sample": bis,
            }),
            Umfang::GanzeSitzung => serde_json::json!({ "art": "ganze_sitzung" }),
        };
        serde_json::json!({
            "type": "evidence_invalidate",
            "grund": invalidierung.grund.wort(),
            "umfang": umfang,
        })
    }

    fn invalidierung_persistieren(
        &self,
        session: &SessionKey,
        invalidierung: &Invalidierung,
        betroffen: usize,
        ziele: Vec<SnapshotZiel>,
    ) {
        let Some(store) = self.store.as_ref() else {
            return;
        };
        let mut payload = Self::invalidierung_als_json(invalidierung);
        if let Some(objekt) = payload.as_object_mut() {
            // Die Zahl der wirklich ausgeschlossenen Belege reist MIT: ein
            // Empfaenger soll „nichts getroffen" von „alles getroffen"
            // unterscheiden koennen, ohne selbst nachzuzaehlen.
            objekt.insert("ausgeschlossen".into(), serde_json::json!(betroffen));
        }
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
        // `event_type = "session"` heisst: die Projektion schreibt den
        // Sessionsnapshot fort. Eine eigene Domaenentabelle bekaeme die
        // Invalidierung nur, wenn sie ein eigenes Objekt WAERE — sie ist aber
        // eine Aussage UEBER Evidenz, kein zweites Evidenzobjekt.
        event.snapshot_ziele = ziele;
        if store.append(vec![event]).is_err() {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
        }
    }

    /// Der Auslöser „Hoermarker oder Preview" (M-52).
    ///
    /// Er wird aus dem Interventionspfad gerufen: ein hoerbarer Eingriff nimmt
    /// die Evidenz seines Bereichs zurueck, statt sie nur fuer die Zukunft zu
    /// sperren.
    pub(super) fn invalidierung_wegen_intervention(
        &self,
        session: &SessionKey,
        von: i64,
        bis: i64,
    ) -> usize {
        self.invalidierung_anwenden(
            session,
            &Invalidierung {
                grund: Grund::Intervention,
                umfang: Umfang::Bereich { von, bis },
            },
        )
    }

    /// Erkennt einen Transportbruch zwischen den zwei juengsten Snapshots
    /// einer Quelle und invalidiert mit dem RICHTIGEN Grund (M-53).
    ///
    /// Die Zuordnung steht in M-53 und ist keine Geschmacksfrage:
    ///
    /// * eine neue `transport_epoch` heisst Seek oder Loop-Wrap →
    ///   `epochwechsel`;
    /// * ein neues `continuity_segment` bei gleicher Epoche heisst
    ///   verlorenes Audio → `sequenzluecke`.
    ///
    /// Beide invalidieren ausdruecklich NUR den Bereich VOR dem Bruch: was
    /// danach kommt, ist neues, gueltiges Material. Eine Invalidierung ueber
    /// die Grenze hinaus naehme Belege zurueck, die der Bruch gar nicht
    /// betrifft.
    pub(super) fn invalidierung_aus_transportbruch(&self, key: &ClientKey) {
        let bruch = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(historie) = stand.evidenz.get(key) else {
                return;
            };
            if historie.len() < 2 {
                return;
            }
            let neu = &historie[historie.len() - 1];
            let alt = &historie[historie.len() - 2];
            let grund = if neu.transport_epoch != alt.transport_epoch {
                Some(Grund::Epochwechsel)
            } else if neu.continuity_segment != alt.continuity_segment {
                Some(Grund::Sequenzluecke)
            } else {
                None
            };
            grund.map(|g| (g, alt.project_sample_start, alt.sample_count))
        };
        let Some((grund, start, anzahl)) = bruch else {
            return;
        };
        let session = key.session();
        let umfang = match start {
            Some(von) => Umfang::Bereich {
                von: i64::MIN / 2,
                bis: von.saturating_add(anzahl as i64),
            },
            // Ohne Projektzeit laesst sich kein Bereich benennen. Fail-closed
            // heisst dann: die ganze Sitzung (§32.3) — nicht „nichts".
            None => Umfang::GanzeSitzung,
        };
        self.invalidierung_anwenden(&session, &Invalidierung { grund, umfang });
    }

    /// Der Auslöser „Materialwechsel" (M-54).
    ///
    /// Er kommt NICHT aus einer Zeitheuristik, sondern aus dem Vergleich
    /// zweier ueber dasselbe Fenster gerechneter Fingerprints — genau das
    /// sagt M-31. Ein fehlender Fingerprint auf einer Seite ist kein
    /// „gleiches Material", sondern ein fehlender Beleg, und fail-closed
    /// heisst dann invalidieren.
    pub(super) fn invalidierung_wegen_material(
        &self,
        session: &SessionKey,
        vorher: Option<&crate::telemetrie::Fingerprintwerte>,
        jetzt: Option<&crate::telemetrie::Fingerprintwerte>,
    ) -> usize {
        match crate::coordinator::invalidierung::material_wechsel(
            vorher,
            jetzt,
            Umfang::GanzeSitzung,
        ) {
            Some(inv) => self.invalidierung_anwenden(session, &inv),
            None => 0,
        }
    }

    /// Der Auslöser „Messpunktwechsel" (M-55) — die Kante zu Gate 7.
    pub(super) fn invalidierung_wegen_messpunkt(
        &self,
        session: &SessionKey,
        alte_klasse: &str,
        neue_klasse: &str,
    ) -> usize {
        match crate::coordinator::invalidierung::messpunkt_wechsel(
            alte_klasse,
            neue_klasse,
            Umfang::GanzeSitzung,
        ) {
            Some(inv) => self.invalidierung_anwenden(session, &inv),
            None => 0,
        }
    }

    /// Dieselben Wege ueber einen LINK statt ueber den internen Sitzungstyp.
    ///
    /// `SessionKey` ist modulintern; die oeffentliche Flaeche des Coordinators
    /// adressiert seit jeher ueber `link_id`. Diese Huellen sind der Zugang
    /// fuer Beine und fuer Aufrufer ausserhalb des Coordinators.
    pub fn invalidierung_wegen_intervention_fuer_link(
        &self,
        link_id: &str,
        von: i64,
        bis: i64,
    ) -> usize {
        let Some(session) = self.session_fuer_link(link_id) else {
            return 0;
        };
        self.invalidierung_wegen_intervention(&session, von, bis)
    }

    pub fn invalidierung_wegen_material_fuer_link(
        &self,
        link_id: &str,
        vorher: Option<&crate::telemetrie::Fingerprintwerte>,
        jetzt: Option<&crate::telemetrie::Fingerprintwerte>,
    ) -> usize {
        let Some(session) = self.session_fuer_link(link_id) else {
            return 0;
        };
        self.invalidierung_wegen_material(&session, vorher, jetzt)
    }

    pub fn invalidierung_wegen_messpunkt_fuer_link(
        &self,
        link_id: &str,
        alte_klasse: &str,
        neue_klasse: &str,
    ) -> usize {
        let Some(session) = self.session_fuer_link(link_id) else {
            return 0;
        };
        self.invalidierung_wegen_messpunkt(&session, alte_klasse, neue_klasse)
    }

    fn session_fuer_link(&self, link_id: &str) -> Option<SessionKey> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        Self::session_des_links(&stand, link_id)
    }

    /// Wie viele Invalidierungen der Broker gesendet hat.
    pub fn invalidierungen_zaehler(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .invalidierungen
    }

    /// Wie viele Evidenzstaende dabei ausgeschlossen wurden.
    pub fn evidenz_ausgeschlossen_zaehler(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .evidenz_ausgeschlossen
    }
}
