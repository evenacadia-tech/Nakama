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

/// Was eine vorbereitete Invalidierung bereits GETAN hat — und was noetig
/// ist, um es rueckgaengig zu machen (Befund B16).
///
/// 🔑 Wiederpruefung 2: Die Runde 2 schloss die Evidenz lokal aus, zaehlte
/// einen gescheiterten Store-Append nur und kehrte mit `()` zurueck. Der
/// Ausschluss blieb, der Befehl galt als angewandt, und ein Neustart liess
/// dieselbe Evidenz wieder zu — der fluechtige Stand und das Log sagten
/// Verschiedenes. Der Ausschluss ist deshalb VORLAEUFIG, bis sein Append steht.
pub(super) struct Invalidierungswirkung {
    pub(super) betroffen: usize,
    pub(super) ziele: Vec<SnapshotZiel>,
    pub(super) nachricht: Value,
    /// Genau die Eintraege, die DIESE Invalidierung markiert hat - benannt
    /// ueber ihre STABILE `evidence_id`, nie ueber ihre Position.
    ///
    /// 🔑 NAK-283 F14 (NAK-163, SONDE-013 §14): hier stand der Deque-Index.
    /// Zwischen Vorbereitung und Ruecknahme liegt eine Lock-Luecke (der
    /// Append), und in ihr kann die Retention (`pop_front()`, `evidenz.rs`)
    /// jede Position verschieben. Der Rollback loeschte dann den Ausschluss
    /// eines FREMDEN Belegs und liess den eigenen stehen; Cache und Projektion
    /// liefen auseinander. Die Luecke darf bleiben - die Zuordnung ueberlebt sie.
    zurueck: Vec<(ClientKey, String)>,
}

impl Coordinator {
    /// Der EINE Weg, eine Invalidierung wirksam zu machen.
    ///
    /// Rueckgabe: wie viele gespeicherte Evidenzstaende sie ausgeschlossen
    /// hat. `Err` heisst „der Store hat den Append verweigert" — dann ist der
    /// lokale Ausschluss zurueckgenommen und der Auslöser muss ablehnen.
    /// `Ok(0)` ist kein Fehler: ein Seek, der keine Evidenz trifft, ist eine
    /// Tatsache und keine Panne.
    pub(super) fn invalidierung_anwenden(
        &self,
        session: &SessionKey,
        invalidierung: &Invalidierung,
    ) -> Result<usize, ()> {
        // Kein Ausloeser ohne eigenen Rueckweg: die Bedeutung ist unveraendert.
        self.invalidierung_anwenden_mit_rueckweg(session, invalidierung, |_| {})
    }

    /// Derselbe Weg — mit dem RUECKWEG DES AUSLOESERS (Erstpruefungsbefund 2,
    /// Nacharbeit 1, 08.09.2026).
    ///
    /// 🔑 **Der Fehlerzweig rechnet neu. Wer VOR dieser Invalidierung schon
    /// etwas veraendert hat, muss es VORHER zuruecknehmen.**
    ///
    /// Der Kanalwechsel ist genau so ein Ausloeser: `heartbeat_kontakt`
    /// ersetzt den Deskriptor UNTER dem Lock (`liveness.rs`:431), die
    /// Invalidierung laeuft wie der Positionsvergleich NACH dem Lock.
    /// Verweigert der Store den Append, nahm der Fehlerzweig bis hierher nur
    /// die Evidenzausschluesse zurueck und rechnete sofort neu — mit dem
    /// BEREITS gewechselten Kanal. Die alten Fenster zaehlten dann unter dem
    /// neuen Kanal, bis hin zu `hoch`, und das anschliessende
    /// `link.trennen = true` verhinderte diese Neuberechnung nicht mehr: sie
    /// war gelaufen. Genau das verbietet R5.
    ///
    /// `rueckweg` laeuft AUSSCHLIESSLICH im `Err`-Fall und als ERSTES —
    /// vor der Ruecknahme der Ausschluesse und vor der Neubildung. Er laeuft
    /// OHNE gehaltenen Standlock; er darf ihn selbst nehmen.
    pub(super) fn invalidierung_anwenden_mit_rueckweg(
        &self,
        session: &SessionKey,
        invalidierung: &Invalidierung,
        rueckweg: impl FnOnce(&Self),
    ) -> Result<usize, ()> {
        let Some(wirkung) = self.invalidierung_vorbereiten(session, invalidierung) else {
            return Ok(0);
        };
        let betroffen = wirkung.betroffen;
        match self.invalidierung_alleine_persistieren(session, &wirkung) {
            // Ein Broker OHNE Store haelt seinen Bestand ausschliesslich
            // fluechtig; dort gibt es nichts, wovon der Speicher abweichen
            // koennte. Der Ausschluss steht, und es gibt keine Zustellschuld.
            Ok(None) => {
                self.paare_bei_bedarf_bilden();
                self.befunde_nach_ruecknahme_zustellen(session, betroffen);
                Ok(betroffen)
            }
            Ok(Some(event_ord)) => {
                self.invalidierung_zustellen(&wirkung, event_ord);
                self.paare_bei_bedarf_bilden();
                self.befunde_nach_ruecknahme_zustellen(session, betroffen);
                Ok(betroffen)
            }
            Err(()) => {
                // ZUERST der Rueckweg des Ausloesers: was diese Invalidierung
                // begleitet hat, gilt erst, wenn sie selbst gilt. Erst danach
                // darf gerechnet werden.
                rueckweg(self);
                self.invalidierung_ruecknehmen(wirkung);
                // Die Ruecknahme hat die Befunde als neu zu bilden markiert;
                // eingeloest wird sie HIER und nicht erst beim naechsten
                // Evidenzsnapshot. Sonst bliebe der Befund `stale`, solange
                // keine neue Evidenz kommt — und genau in dem Zustand ist der
                // Store degradiert, in dem sie ausbleiben kann.
                self.hypothesen_bei_bedarf_bilden();
                Err(())
            }
        }
    }

    /// Schliesst die Evidenz VORLAEUFIG aus und sagt, was das getroffen hat.
    ///
    /// `None` heisst „diese Invalidierung nimmt nichts zurueck" — eine leere
    /// ID-Menge oder ein leerer Bereich. Sie zu senden hiesse, dem Empfaenger
    /// eine Ruecknahme zu melden, die nichts zuruecknimmt, und ihn danach
    /// glauben zu lassen, es sei aufgeraeumt (M-57).
    pub(super) fn invalidierung_vorbereiten(
        &self,
        session: &SessionKey,
        invalidierung: &Invalidierung,
    ) -> Option<Invalidierungswirkung> {
        if !invalidierung.gueltig() {
            return None;
        }
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let mut zurueck: Vec<(ClientKey, String)> = Vec::new();
        // 🔑 SONDE-014 M-24: die Menge der markierten Evidenz-IDs.
        //
        // Sie ist ein `BTreeSet` und kein `Vec`, weil M-24 „deterministisch"
        // woertlich so definiert: „dieselbe Ruecknahme in anderer Reihenfolge
        // trifft dieselbe Menge". Eine Liste haette eine Reihenfolge, die
        // niemand braucht und an der ein spaeterer Leser sich orientieren
        // koennte.
        let mut genommene_ids: BTreeSet<String> = BTreeSet::new();
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
                    genommene_ids.insert(eintrag.evidence_id.clone());
                    // NAK-283 F14: die stabile ID, nicht die Position.
                    zurueck.push((key.clone(), eintrag.evidence_id.clone()));
                }
            }
        }
        let betroffen = zurueck.len();
        // 🔑 Nacharbeit 3 (Befund B18, M-22): eine Ruecknahme ist eine
        // Evidenzaenderung. Die Runde 2 stiess KEINE Neubildung an und liess
        // das alte volle Paarurteil stehen — ein Urteil ueber Belege, die es
        // gerade zurueckgenommen hatte.
        if betroffen > 0 {
            stand.paare_neu_bilden = true;
        }
        // 🔑 SONDE-014 M-24/M-28: die abhaengigen Hypothesen fallen UNTER
        // DEMSELBEN LOCK, in dem die Evidenz markiert wurde.
        //
        // Reihenfolge woertlich aus M-24: Umfang aufloesen → Evidenz-IDs
        // markieren → jede Hypothese, deren `evidence_ids` eine davon
        // enthaelt, terminal invalidieren. Laege der letzte Schritt
        // ausserhalb, koennte eine gleichzeitige Neurechnung dieselbe
        // Hypothese zwischen Markierung und Invalidierung wieder aufbauen —
        // und Gen saehe eine Behauptung ueber Belege, die es nicht mehr gibt.
        //
        // ⚠️ Und genau deshalb wird hier NICHT `befunde_neu_bilden` gesetzt.
        // „Terminal" heisst: diese Ruecknahme laesst die Hypothese nicht
        // gueltig zurueck. Eine Neurechnung unmittelbar danach rechnete sie
        // aus den VERBLIEBENEN Belegen frisch auf und haette den Zustand
        // `stale` in derselben Bewegung wieder weggeraeumt — die
        // Invalidierung waere ein Flackern gewesen, kein Zustand. Neu
        // gerechnet wird erst, wenn NEUES Material ankommt; dann ruht die
        // Aussage auch wirklich auf neuem Material.
        Self::befunde_invalidieren_locked(&mut stand, session, &genommene_ids);
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
        Some(Invalidierungswirkung {
            betroffen,
            ziele,
            nachricht: Self::invalidierung_als_json(invalidierung),
            zurueck,
        })
    }

    /// Nimmt einen vorlaeufigen Ausschluss zurueck (Befund B16).
    ///
    /// Zurueckgesetzt wird GENAU, was diese Invalidierung markiert hat — nicht
    /// jeder Ausschluss der Sitzung. Ein aelterer Grund gehoert einer anderen
    /// Ruecknahme und bleibt stehen.
    ///
    /// 🔑 NAK-283 F14 (M-21 bis M-23): gesucht wird ueber die stabile
    /// `evidence_id`. Zwischen Vorbereitung und diesem Aufruf liegt die
    /// Lock-Luecke des Appends; in ihr kann die Retention Belege entfernen und
    /// alle Positionen verschieben. Ein Beleg, den sie entfernt hat, wird
    /// uebersprungen, statt an einem Nachbarn zu wirken.
    pub(super) fn invalidierung_ruecknehmen(&self, wirkung: Invalidierungswirkung) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        for (key, evidence_id) in &wirkung.zurueck {
            let Some(historie) = stand.evidenz.get_mut(key) else {
                continue;
            };
            let Some(eintrag) = historie
                .iter_mut()
                .find(|eintrag| eintrag.evidence_id == *evidence_id)
            else {
                // Von der Retention entfernt: uebersprungen.
                continue;
            };
            eintrag.ausschlussgrund = None;
        }
        // Der Merker faellt mit: die Aenderung, die ihn setzte, gibt es nicht
        // mehr. Ein stehengebliebener Merker triebe eine Neubildung ohne Anlass.
        stand.paare_neu_bilden = false;
        // 🔑 SONDE-014 M-24, Gegenrichtung: die Befunde muessen SEHR WOHL neu
        // gebildet werden.
        //
        // Der Unterschied zu den Paaren ist die Richtung der Wirkung. Die
        // Paare waren unveraendert, weil ihre Neubildung nie lief; die
        // Befunde dagegen wurden unter demselben Lock bereits terminal
        // invalidiert. Sie hier stehen zu lassen hiesse: die Evidenz ist
        // zurueck, die Behauptung darueber bleibt `stale` — eine Sperre ohne
        // Grund, die kein Ereignis mehr aufheben wuerde. Die Rechnung ist
        // eine reine Funktion des Evidenzbestands und stellt den alten Stand
        // deterministisch wieder her.
        stand.befunde_neu_bilden = true;
        stand.invalidierungen = stand.invalidierungen.saturating_sub(1);
        stand.evidenz_ausgeschlossen = stand
            .evidenz_ausgeschlossen
            .saturating_sub(wirkung.betroffen as u64);
        stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
    }

    /// Die Invalidierung als Domaenenereignis fuer den Append eines BEFEHLS
    /// (Befund B14).
    ///
    /// Damit liegen Befehlsriegel und Ruecknahme in derselben Transaktion. Ein
    /// Crash dazwischen kann es nicht mehr geben, und ein Retry bekommt nicht
    /// `idempotent_wiederholt` fuer einen Befehl, dessen Wirkung fehlt.
    pub(super) fn invalidierung_als_domaenenereignis(
        &self,
        wirkung: &Invalidierungswirkung,
    ) -> Domaenenereignis {
        Domaenenereignis {
            event_type: "evidence_invalidate".into(),
            payload: serde_json::json!({
                "nachricht": wirkung.nachricht,
                "ausgeschlossen": wirkung.betroffen,
            }),
            ziele: wirkung.ziele.clone(),
        }
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
            // 🔑 Nacharbeit 2 (Befund R26): die Feldnamen sind `sample_start`
            // und `sample_end`, nicht `von_sample`/`bis_sample`. `invalidate_bereich`
            // ist `additionalProperties: false` — der alte Payload war damit
            // nicht etwa nur anders benannt, sondern SCHEMAUNGUELTIG, und der
            // Test schrieb den Vertragsbruch fest, statt an ihm zu fallen.
            //
            // Beide Grenzen haben `minimum: 0`. Ein negativer Rand — etwa der
            // frueher benutzte `i64::MIN / 2` als „von Anfang an" — ist im
            // Vertrag kein Bereich, sondern eine Verletzung; die Projektzeit
            // beginnt bei 0, und genau das ist der Anfang.
            Umfang::Bereich { von, bis } => serde_json::json!({
                "art": "sample_range",
                "sample_start": (*von).max(0),
                "sample_end": (*bis).max(0),
            }),
            Umfang::GanzeSitzung => serde_json::json!({ "art": "ganze_sitzung" }),
        };
        serde_json::json!({
            "type": "evidence_invalidate",
            "grund": invalidierung.grund.wort(),
            "umfang": umfang,
        })
    }

    /// Der eigene Append einer Invalidierung, die zu KEINEM Befehl gehoert
    /// (Marker, Transportbruch, Messpunktwechsel).
    ///
    /// `Err` heisst „der Store hat verweigert"; der Aufrufer nimmt dann den
    /// vorlaeufigen Ausschluss zurueck. `Ok(None)` heisst „dieser Broker
    /// fuehrt gar keinen Store" — dann gibt es nichts zu persistieren und
    /// nichts zuzustellen, und der fluechtige Ausschluss ist die ganze
    /// Wahrheit.
    fn invalidierung_alleine_persistieren(
        &self,
        session: &SessionKey,
        wirkung: &Invalidierungswirkung,
    ) -> Result<Option<i64>, ()> {
        let Some(store) = self.store.as_ref() else {
            return Ok(None);
        };
        // 🔑 Nacharbeit 2 (Befund R26): der WIRE-Payload bleibt genau die
        // Nachricht. `evidence_invalidate` ist `additionalProperties: false`;
        // ein Top-Level `ausgeschlossen` machte den Payload ungueltig, und der
        // Subscriber haette ihn abweisen muessen. Die Zahl der getroffenen
        // Belege ist eine Aussage UEBER die Ruecknahme, kein Feld IN ihr — sie
        // steht deshalb im Store-Ereignis NEBEN der Nachricht.
        let payload = serde_json::json!({
            "nachricht": wirkung.nachricht,
            "ausgeschlossen": wirkung.betroffen,
        });
        let Ok(payload_jcs) = serde_json_canonicalizer::to_vec(&payload) else {
            return Err(());
        };
        let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
        let mut event = StoreEvent::session_snapshot(
            &session.project_binding_id,
            &session.session_epoch,
            &self.broker_epoch,
            sequence.min(i64::MAX as u64) as i64,
            payload_jcs,
        );
        // 🔑 Nacharbeit 2 (Befund R27): EIGENER Ereignistyp.
        //
        // `StoreEvent::session_snapshot` legt `event_type = "session"` an, und
        // dieser Pfad hat ihn nie ueberschrieben. Der Writer ersetzte damit
        // `sessions.state_jcs` durch den Invalidierungspayload; beim naechsten
        // Subscribe erwartet `subscription.rs` dort einen `session_snapshot`,
        // verwirft die Projektion und setzt Routing fail-closed. Eine
        // Ruecknahme von Evidenz nahm so die ganze Sitzungsprojektion mit.
        event.event_type = "evidence_invalidate".into();
        event.snapshot_ziele = wirkung.ziele.clone();
        let Ok(ausgaenge) = store.append(vec![event]) else {
            return Err(());
        };
        match ausgaenge.first() {
            Some(a) => Ok(Some(a.event_ord())),
            None => Err(()),
        }
    }

    /// Die ZUSTELLUNG (Befund R28) — nach dem Append, nie davor.
    ///
    /// Die WIRE-Nachricht ist die Invalidierung selbst; sie ist eine eigene
    /// v3-Familie mit eigenem Leser im Plugin, kein Sessionschnitt. Was
    /// zugestellt wurde, wird kompaktiert: eine Schuld, die niemand abtraegt,
    /// waechst. Was NICHT ankommt, bleibt als Schuld stehen und wird beim
    /// naechsten Subscribe nachgespielt (Befund B17).
    pub(super) fn invalidierung_zustellen(
        &self,
        wirkung: &Invalidierungswirkung,
        event_ord: i64,
    ) {
        let Some(store) = self.store.as_ref() else {
            return;
        };
        let Ok(payload) = serde_json::to_vec(&wirkung.nachricht) else {
            return;
        };
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        for ziel in &wirkung.ziele {
            let Some(link_id) = self.link_des_abonnenten(ziel) else {
                continue;
            };
            // 🔑 NAK-246 D9 (R-D9): der Schluessel der Ruecknahme
            // (`evidence_invalidate`, aus dem Ziel) und ihr Ordinal reisen bis
            // in die Writerqueue. Bis hierher machte der Sender daraus
            // `session_snapshot`, und ein spaeterer Vollsnapshot verdraengte
            // die Ruecknahme dort.
            let geschrieben = self.push_ziel_noch_gueltig(&link_id, ziel)
                && push.as_ref().is_some_and(|push| {
                    push.snapshot_schreiben(&link_id, &ziel.object_key, event_ord, &payload)
                });
            if geschrieben {
                let _ = store.snapshot_schuld_kompaktieren(ziel.clone(), event_ord);
            }
        }
    }

    /// Schiebt den `session_snapshot` nach, wenn eine Ruecknahme Befunde
    /// getroffen hat (SONDE-014 M-24/M-28).
    ///
    /// Die `evidence_invalidate`-Nachricht daneben sagt, WELCHE Evidenz
    /// zurueckgenommen wurde; sie sagt nicht, welche Behauptung damit
    /// unsichtbar oder `stale` geworden ist. Ohne diesen Push zeigte Gen
    /// einen Befund weiter, dessen Belege der Broker gerade verworfen hat —
    /// genau die Behauptung ohne Beleg, die das Exit-Gate ausschliesst.
    fn befunde_nach_ruecknahme_zustellen(&self, session: &SessionKey, betroffen: usize) {
        if betroffen == 0 {
            return;
        }
        {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.dirty_sessions.insert(session.clone());
        }
        self.flush_session(session, None);
    }

    /// Der Link, ueber den ein Abonnent dieses Ziels erreichbar ist.
    pub(super) fn link_des_abonnenten(&self, ziel: &SnapshotZiel) -> Option<String> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand
            .subscriptions
            .iter()
            .find(|(_, sub)| {
                sub.adresse.instance_id == ziel.instance_id
                    && sub.session_epoch == ziel.session_epoch
                    && sub.adresse.project_binding_id == ziel.project_binding_id
            })
            .map(|(link_id, _)| link_id.clone())
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
    ) -> Result<usize, ()> {
        self.invalidierung_anwenden(
            session,
            &Invalidierung {
                grund: Grund::Intervention,
                umfang: Umfang::Bereich { von, bis },
            },
        )
    }

    /// Der Mixerkanal ist Teil des Messpunkts (NAK-213 R5).
    ///
    /// Eine Quelle, die den Kanal wechselt, misst danach etwas anderes als
    /// davor. Ihre Belege werden zurueckgenommen — die der ANDEREN Quellen
    /// nicht: R5 sagt woertlich „Befunde, DIE DIE QUELLE TRAGEN", und der
    /// Mixerkanal ist eine Eigenschaft dieser einen Quelle. Der bestehende
    /// Positionswechsel (`measurement_position`) behaelt seinen Umfang
    /// `GanzeSitzung`; er ist SONDE-013 R24 und wird hier nicht angefasst.
    ///
    /// ⚠️ KEIN neuer Invalidierungsgrund. `Grund` ist eine eigene geschlossene
    /// Menge des Vertrags, und ein Kanalwechsel IST ein Messpunktwechsel —
    /// genau die Aussage, die R5 trifft. Ein dritter Wert waere ein zweiter
    /// Fassungsschritt fuer dieselbe Sache.
    ///
    /// Zahlenraender: `alt == neu` ist kein Wechsel. `None -> Some(k)` ist
    /// einer (aus „Routing unbekannt" wird ein Messpunkt), `Some(k) -> None`
    /// ebenfalls (der Messpunkt ist unbekannt geworden). Eine Quelle OHNE
    /// Belege ergibt eine leere ID-Menge, und eine leere Menge ist KEINE
    /// Invalidierung (`Umfang::gueltig`) — es wird nichts gesendet. Eine
    /// Ruecknahme, die nichts zurueckninmt, liesse den Empfaenger glauben, es
    /// sei aufgeraeumt (M-57).
    pub(super) fn invalidierung_wegen_kanalwechsel(
        &self,
        session: &SessionKey,
        key: &ClientKey,
        alt: Option<i64>,
        neu: Option<i64>,
    ) -> Result<usize, ()> {
        if alt == neu {
            return Ok(0);
        }
        let ids: std::collections::BTreeSet<String> = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand
                .evidenz
                .get(key)
                .map(|historie| {
                    historie
                        .iter()
                        .filter(|e| e.ausschlussgrund.is_none())
                        .map(|e| e.evidence_id.clone())
                        .collect()
                })
                .unwrap_or_default()
        };
        let invalidierung = Invalidierung {
            grund: Grund::MesspunktWechsel,
            umfang: Umfang::Ids(ids),
        };
        if !invalidierung.gueltig() {
            return Ok(0);
        }
        // 🔑 **Nacharbeit 1 (08.09.2026, Erstpruefungsbefund 2): der
        // Kanalwechsel wird erst wirksam, wenn seine Invalidierung ANGENOMMEN
        // ist.**
        //
        // Beide Ingresse ersetzen den Deskriptor, BEVOR sie hierher kommen —
        // `heartbeat_kontakt` unter dem Lock (`liveness.rs`:431),
        // `descriptor_setzen` ebenso. Scheitert der Store-Append, rechnet der
        // Fehlerzweig neu; ohne diesen Rueckweg taete er das mit dem NEUEN
        // Kanal, und Cs zwoelf alte Fenster zaehlten unter Kanal 9 — bis hin
        // zu `hoch`. Der Rueckweg steht HIER und nicht an den zwei
        // Aufrufstellen: eine Regel an nur einer von ihnen waere an der
        // anderen still ausser Kraft (dieselbe Lehre wie E8).
        self.invalidierung_anwenden_mit_rueckweg(session, &invalidierung, |koordinator| {
            koordinator.kanal_im_deskriptor_zuruecksetzen(key, alt);
        })
    }

    /// Derselbe Ausloeser OHNE bekannten Bereich (M-52, Befund R25).
    ///
    /// Ein Hoermarker, dessen Beginn oder Ende keine Projektzeit trug, hat
    /// gefaerbt — nur weiss niemand wo. Fail-closed heisst dann die ganze
    /// Sitzung, nicht „nichts" (§32.3).
    pub(super) fn invalidierung_wegen_intervention_ganze_sitzung(
        &self,
        session: &SessionKey,
    ) -> Result<usize, ()> {
        self.invalidierung_anwenden(
            session,
            &Invalidierung {
                grund: Grund::Intervention,
                umfang: Umfang::GanzeSitzung,
            },
        )
    }

    /// Der Ausloeser „Preview" (M-52, Befund R24).
    ///
    /// Er wird aus dem P0-Pfad der drei `preview_*`-Familien gerufen: eine
    /// Vorschau ist ein hoerbarer Eingriff, und die waehrend ihr gemessene
    /// Evidenz gehoert nicht in einen Vergleich. Der Umfang ist die ganze
    /// Sitzung, weil die Nachrichten keinen Bereich in Projektzeit tragen —
    /// ein geratenes Fenster waere schlimmer als ein zu grosses (§32.3).
    ///
    /// 🔑 Nacharbeit 3 (Befund B14): sie wird nur VORBEREITET. Die Runde 2
    /// committete den Befehl ueber `persistenz_p0` und invalidierte erst
    /// DANACH in einem zweiten Append. Stuerzte der Broker dazwischen, bekam
    /// der Retry `idempotent_wiederholt` — und die Bedingung „nur bei
    /// angewandt" uebersprang die Ruecknahme fuer immer. Die Vorschau war
    /// persistiert, ihre Evidenz blieb gueltig, und niemand sah es.
    pub(super) fn preview_invalidierung_vorbereiten(
        &self,
        session: &SessionKey,
    ) -> Option<Invalidierungswirkung> {
        self.invalidierung_vorbereiten(
            session,
            &Invalidierung {
                grund: Grund::Intervention,
                umfang: Umfang::GanzeSitzung,
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
            // Nacharbeit 2 (Befund R26): der Anfang ist 0 und nicht
            // `i64::MIN / 2`. Beide erfassen dieselbe Menge — Evidenz ohne
            // Projektzeit traegt `i64::MIN` und faellt bei BEIDEN Raendern aus
            // dem Fenster —, aber nur 0 ist eine Zahl, die der Vertrag als
            // `sample_start` kennt (`minimum: 0`). Der frueher gesendete
            // negative Rand machte den Payload schemaungueltig.
            Some(von) => Umfang::Bereich {
                von: 0,
                bis: von.saturating_add(anzahl as i64),
            },
            // Ohne Projektzeit laesst sich kein Bereich benennen. Fail-closed
            // heisst dann: die ganze Sitzung (§32.3) — nicht „nichts".
            None => Umfang::GanzeSitzung,
        };
        let _ = self.invalidierung_anwenden(&session, &Invalidierung { grund, umfang });
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
    ) -> Result<usize, ()> {
        match crate::coordinator::invalidierung::material_wechsel(
            vorher,
            jetzt,
            Umfang::GanzeSitzung,
        ) {
            Some(inv) => self.invalidierung_anwenden(session, &inv),
            None => Ok(0),
        }
    }

    /// Dieselbe Feststellung, aber nur VORBEREITET (Befund B14).
    ///
    /// Der Materialwechsel nach `experiment_begin` ist Wirkung DIESES Befehls
    /// und geht in denselben Append; er darf deshalb keinen eigenen bekommen.
    pub(super) fn invalidierung_wegen_material_vorbereiten(
        &self,
        session: &SessionKey,
        vorher: Option<&crate::telemetrie::Fingerprintwerte>,
        jetzt: Option<&crate::telemetrie::Fingerprintwerte>,
    ) -> Option<Invalidierungswirkung> {
        let inv = crate::coordinator::invalidierung::material_wechsel(
            vorher,
            jetzt,
            Umfang::GanzeSitzung,
        )?;
        self.invalidierung_vorbereiten(session, &inv)
    }

    /// Der Auslöser „Messpunktwechsel" (M-55) — die Kante zu Gate 7.
    pub(super) fn invalidierung_wegen_messpunkt(
        &self,
        session: &SessionKey,
        alte_klasse: &str,
        neue_klasse: &str,
    ) -> Result<usize, ()> {
        match crate::coordinator::invalidierung::messpunkt_wechsel(
            alte_klasse,
            neue_klasse,
            Umfang::GanzeSitzung,
        ) {
            Some(inv) => self.invalidierung_anwenden(session, &inv),
            None => Ok(0),
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
            .unwrap_or(0)
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
            .unwrap_or(0)
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
            .unwrap_or(0)
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
