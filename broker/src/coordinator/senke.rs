//! Der Transportkontakt.
//!
//! Der v3-Transport meldet hier jedes Ereignis einer Control-Verbindung; der
//! Coordinator antwortet mit Zustand, nie mit I/O.

use super::*;

#[cfg(windows)]
impl crate::transport::server_v3::Senke for Coordinator {
    fn control_verbunden(
        &self,
        link_id: &str,
        hello: &HelloControl,
    ) -> crate::transport::server_v3::ControlAnmeldung {
        let ausgang = self.control_hello_registrieren(link_id, hello);
        crate::transport::server_v3::ControlAnmeldung {
            angenommen: ausgang.angenommen,
            grund: ausgang.grund,
            zu_schliessende_links: ausgang.zu_schliessende_links,
        }
    }

    fn control_schliesst(&self, link_id: &str) {
        self.control_ende(link_id);
    }

    /// H-16, Schritt 3: der Push der bei einer Nonce-Verdraengung dirty
    /// markierten Session.
    ///
    /// Hier und nicht frueher, weil erst hier das C-06-Cleanup des verdraengten
    /// Links vollstaendig ist: der Transport meldet `control_getrennt` NACH den
    /// fristbegrenzten Joins und nach dem Warten auf das `telemetrie_getrennt`
    /// der mitfallenden Telemetrieverbindung. `control_ende` waere zu frueh -
    /// es laeuft ueber `control_schliesst` noch vor Kopplungsloesung, Joins und
    /// Trenncallbacks.
    ///
    /// Bis NAK-121 war das ein Leerrumpf, und der Push kam erst mit dem
    /// naechsten Heartbeat - bis zu HEARTBEAT_INTERVAL_MS spaeter. Der
    /// Heartbeat bleibt der Rueckfall, falls kein Transportcallback kommt; die
    /// Session ist dirty markiert und geht nicht verloren.
    fn control_getrennt(&self, link_id: &str) {
        let dirty: Vec<SessionKey> = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            // Nur fuer einen Link, den eine Verdraengung markiert hat - ein
            // gewoehnliches Verbindungsende hat seinen Flush laengst. Der
            // Marker liegt in einer eigenen Menge, weil `control_ende` den Link
            // selbst schon abgeraeumt hat, bevor dieser Callback kommt.
            if !stand.verdraengt_wartet_auf_push.remove(link_id) {
                return;
            }
            stand.dirty_sessions.iter().cloned().collect()
        };
        // Ohne Lock, wie C-09 es verlangt.
        for session in dirty {
            self.flush_session(&session, None);
        }
    }
    fn telemetrie_gekoppelt(&self, link_id: &str) {
        let neu = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let neu =
                stand.links.contains_key(link_id) && stand.telemetry_links.insert(link_id.into());
            if neu {
                stand.telemetry_kopplungen = stand.telemetry_kopplungen.saturating_add(1);
            }
            neu
        };
        if neu {
            self.messframes_an_subscriber_push(link_id);
        }
    }
    fn telemetrie_getrennt(&self, link_id: &str) {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .telemetry_links
            .remove(link_id);
    }

    fn p0(&self, link_id: &str, payload: &[u8]) -> Option<Vec<u8>> {
        self.p0_json(link_id, payload)
    }

    fn p0_mit_minor(&self, link_id: &str, schema_minor: u8, payload: &[u8]) -> Option<Vec<u8>> {
        self.p0_json_mit_minor(link_id, payload, schema_minor)
    }

    fn p1(&self, link_id: &str, payload: &[u8]) {
        self.p1_mit_minor(link_id, JSON_SCHEMA_MINOR_AKTIV, payload);
    }

    fn p1_mit_minor(&self, link_id: &str, schema_minor: u8, payload: &[u8]) {
        let typ = serde_json::from_slice::<Value>(payload)
            .ok()
            .and_then(|wert| wert.get("type").and_then(Value::as_str).map(str::to_owned));
        match typ.as_deref() {
            Some("subscribe_session") => {
                let _ = self.subscribe_json_mit_minor(link_id, payload, schema_minor);
            }
            Some("state_report") => {
                let _ = self.state_report_json_mit_minor(link_id, payload, schema_minor);
            }
            // SONDE-013 M-05: der Evidenzpfad. Das Ergebnis wird bewusst
            // verworfen wie bei den beiden Nachbarn - ein abgelehnter P1
            // schliesst die Verbindung nicht, er wird gezaehlt.
            Some("evidence_snapshot") => {
                let _ = self.evidence_snapshot_json_mit_minor(link_id, payload, schema_minor);
            }
            _ => {}
        }
    }

    fn p2(&self, link_id: &str, payload: &[u8]) {
        let batch = match crate::telemetrie::fuer_broker(payload) {
            Ok(batch) => batch,
            Err(_) => {
                self.messframe_abweisen(link_id, P2RejectGrund::FeatureBatchUngueltig);
                return;
            }
        };
        if batch.frames.len() != 1 {
            self.messframe_abweisen(link_id, P2RejectGrund::QuellframeAnzahlUngueltig);
            return;
        }
        let frame = &batch.frames[0];
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        let mut ziele: Vec<String> = Vec::new();
        let mut instance_id = String::new();
        {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(key) = Self::aktueller_telemetrie_client_locked(&stand, link_id) else {
                return;
            };
            let Some(link) = stand.links.get(link_id) else {
                return;
            };
            if !self.dispatch_fuer_link_erlaubt_locked(&stand, link) {
                let _ = Self::messframe_abweisen_locked(
                    &mut stand,
                    link_id,
                    P2RejectGrund::RoutingNichtFreigegeben,
                );
                return;
            }
            if frame.adresse != link.adresse {
                let _ = Self::messframe_abweisen_locked(
                    &mut stand,
                    link_id,
                    P2RejectGrund::QuelladresseAbweichend,
                );
                return;
            }

            let jetzt = self.clock.jetzt();
            let lautheit_ungueltig = batch
                .lautheit_ungueltige_instance_ids
                .iter()
                .any(|instance_id| instance_id == &key.instance_id);
            let lautheit = stand.lautheit.entry(key.clone()).or_default();
            if lautheit_ungueltig {
                lautheit.zustand = Lautheitszustand::Invalid;
                lautheit.ungueltig_anzahl = lautheit.ungueltig_anzahl.saturating_add(1);
            } else if let Some((lufs_i, unsicherheit)) = frame.lufs_i_paar {
                lautheit.zustand = Lautheitszustand::Paar;
                lautheit.letztes_gueltiges_paar = Some((lufs_i, unsicherheit, jetzt));
            } else {
                match frame.lufs_i_status {
                    Some(1) => lautheit.zustand = Lautheitszustand::Collecting,
                    Some(2) => lautheit.zustand = Lautheitszustand::Gated,
                    _ => {}
                }
            }

            {
                let fehler = stand.messfehler.entry(key.clone()).or_default();
                if lautheit_ungueltig {
                    fehler.anzahl = fehler.anzahl.saturating_add(1).min(JSON_SAFE_INTEGER_MAX);
                    fehler.letzter_grund = Some(P2RejectGrund::LautheitUngueltig);
                }
                // Der restliche Frame ist nach Entfernung der drei LUFS-I-Felder
                // vollstaendig gueltig. Nur eine vollstaendige Ablehnung setzt die
                // allgemeine Messachse auf invalid.
                fehler.aktuell = false;
            }
            if lautheit_ungueltig {
                let session = key.session();
                stand.dirty_sessions.insert(session);
            }
            // G2-FLOATEDGE-001, Nacharbeit Runde 2 (R2-1): das abgeleitete
            // Analysefenster entsteht GENAU HIER, einmal je angenommenem
            // Frame, und `fenster_nicht_endlich` erhoeht sich hier genau
            // einmal je Frame mit nicht-normaler/nicht-positiver Samplerate
            // oder nicht-endlichem Ergebnis. Die Sicht liest nur noch den
            // gespeicherten Wert - sonst zaehlte der Zaehler Lesefrequenz.
            let fenster_ms = self.fenster_ms_bilden(frame.sample_count, frame.sample_rate);
            stand.messframes.insert(
                key.clone(),
                LiveMessframe {
                    adresse: frame.adresse.clone(),
                    payload: batch.payload.clone(),
                    empfangen: jetzt,
                    sequence: frame.sequence,
                    sample_count: frame.sample_count,
                    sample_rate: frame.sample_rate,
                    fenster_ms,
                },
            );
            stand.p2_live_frames = stand.p2_live_frames.saturating_add(1);

            // H-03: unter dem Lock wird nur GESAMMELT. Der Push selbst laeuft
            // unten, nachdem der Standlock gefallen ist - dieselbe Invariante,
            // die `flush_session` seit SONDE-011 in ihrem Kommentar fuehrt.
            if push.is_some() {
                let session = key.session();
                ziele = stand
                    .subscriptions
                    .iter()
                    .filter(|(ziel_link_id, sub)| {
                        sub.session_epoch == session.session_epoch
                            && sub.adresse.project_binding_id == session.project_binding_id
                            && stand.links.get(*ziel_link_id).is_some_and(|ziel_link| {
                                !ziel_link.trennen
                                    && stand.routing_bereit
                                    && self.alias_register.session_push_erlaubt(
                                        &ziel_link.alias_adressraum,
                                        &ziel_link.alias_besitzer,
                                        &ziel_link.adresse.instance_id,
                                    )
                            })
                    })
                    .map(|(ziel_link_id, _)| ziel_link_id.clone())
                    .collect::<Vec<_>>();
                ziele.sort();
                instance_id = key.instance_id.clone();
            }
        }
        // Lock ist gefallen. Erst JETZT fremde Senkenarbeit: eine reentrante
        // Senke kann den Broker damit nicht mehr auf dem globalen Standlock
        // verklemmen.
        if let Some(push) = push.as_ref() {
            for ziel_link_id in ziele {
                let _ = push.messframe_schreiben(&ziel_link_id, &instance_id, &batch.payload);
            }
        }
        if self.dispatch_fuer_link_erlaubt(link_id) {
            // M-62: die Entscheidung gilt der SITZUNG dieses Links.
            let _ = self.evidence_dispatch_fuer_link(link_id);
        }
    }

    fn abgewiesen(&self, _grund: &str) {}
}
