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
        // 🔑 NAK-313 R-313-6 (M-44): EIN strenger Lauf je P1-Nachricht.
        //
        // Bis Etappe 4 las die Weiche den Typ aus einem ungeprueften
        // `serde_json::Value` — vor dem Textriegel und mit „letzter gewinnt"
        // bei doppelten Namen — und der Leser dahinter parste dieselben Bytes
        // ein zweites Mal. Jetzt laufen Textriegel, strenger Lauf und Schema
        // hier genau einmal; der Typ kommt aus dem GEPRUEFTEN Wert, und die
        // Leser bekommen den Wert, nicht die Bytes. Eine Ablehnung endet hier
        // still wie vorher am Leser (Manifest NAK-313 §8.0, B-2).
        let Ok(wert) = v3_nachricht_lesen_beliebig_mit_minor(payload, schema_minor) else {
            return;
        };
        let typ = wert.get("type").and_then(Value::as_str).map(str::to_owned);
        match typ.as_deref() {
            Some("subscribe_session") => {
                let _ = self.subscribe_json_mit_minor_wert(link_id, wert);
            }
            Some("state_report") => {
                let _ = self.state_report_json_mit_minor_wert(link_id, wert);
            }
            // SONDE-013 M-05: der Evidenzpfad. Das Ergebnis wird bewusst
            // verworfen wie bei den beiden Nachbarn - ein abgelehnter P1
            // schliesst die Verbindung nicht.
            Some("evidence_snapshot") => {
                let _ = self.evidence_snapshot_json_mit_minor_wert(link_id, wert);
            }
            // SONDE-014 E-10/E-11: Intent und Assistentenschritt reisen als
            // P1 vom Main zum Broker. Dasselbe Muster wie bei den drei
            // Nachbarn - ein abgelehnter P1 schliesst die Verbindung nicht.
            // Der GRUND der Ablehnung bleibt hier bewusst unbenutzt: er ist
            // die Testnaht, nicht der Produktweg.
            Some("intent_update") => {
                let _ = self.intent_update_json_mit_minor_wert(link_id, wert);
            }
            Some("assistant_step_update") => {
                let _ = self.assistant_step_update_json_mit_minor_wert(link_id, wert);
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

/// NAK-313 Etappe 4 (R-313-6, R-313-13; M-44, M-49, M-51 bis M-53): die
/// P1-Weiche an der Tabelle der Produkteingaenge. Die Senke gibt es nur unter
/// Windows, deshalb auch diesen Testbau.
#[cfg(all(test, windows))]
mod nak313_tests {
    use super::*;
    use crate::coordinator::schema::lesestufe;
    use crate::transport::bootstrap::{AudioLage, HelloControl};
    use crate::transport::server_v3::Senke;
    use crate::vertrag::{laeufe, produkteingaenge as tabelle};

    /// Ein Link der Art und Adresse aus der Einspeisung, angemeldet und mit
    /// einem Heartbeatkontakt — dieselbe Lage wie `anmelden` und `report` in
    /// `broker/tests/sonde013_verdrahtung.rs`.
    fn link_anmelden(c: &Coordinator, art: &str, adresse: &Adresse) {
        let hello = HelloControl {
            typ: "hello".into(),
            connection_kind: "control".into(),
            protocol: 3,
            plugin_version: "test".into(),
            plugin_kind: art.into(),
            adresse: adresse.clone(),
            host: None,
            audio: AudioLage { samplerate: 48_000.0, block_size: 512, channels: 2 },
        };
        assert!(c.control_hello_registrieren("link-p1", &hello).angenommen);
        if art == "main" {
            let _ = c.resync_bestaetigen("link-p1", 0);
        }
        let mut kontakt = serde_json::json!({
            "type": "heartbeat",
            "adresse": adresse,
            "sequence": 1,
            "state_revision": 0,
            "capabilities": {
                "host_context_presence": "supported",
                "project_time_samples": "supported",
                "sample_accurate_automation": "supported",
                "presentation_latency": "supported",
                "aux_compare_pre": "unsupported",
                "aux_priority_sidechain": "unsupported",
                "contribution_aux": "unsupported",
                "float64_processing": "supported",
                "binary_telemetry": "supported",
                "remote_control": "unsupported"
            },
            "zaehler": {}
        });
        if art != "main" {
            kontakt["runtime"] = serde_json::json!({"messpunkt": "insert", "betrieb": "active"});
        }
        let _ = c.heartbeat_kontakt("link-p1", Some(&kontakt));
    }

    /// Was ein P1-Eintrag bewirken kann: angenommene Evidenz, uebernommener
    /// Intent und die Sichten beider.
    fn wirkungsstand(c: &Coordinator, adresse: &Adresse) -> (u64, u64, bool, i64, i64) {
        let evidenz = c.stand.lock().unwrap_or_else(|e| e.into_inner()).evidence_angenommen;
        let intent = c.intent_sicht(&adresse.project_binding_id, &adresse.session_epoch);
        (
            evidenz,
            c.intent_updates(),
            c.evidenz_sicht(&adresse.instance_id).is_some(),
            intent.revision,
            intent.generation,
        )
    }

    /// M-44, M-49, M-51 bis M-53: jeder Eintrag von `rust_p1` als eigener
    /// Fall durch die ECHTE Weiche — Urteil und Stufe, Wirkung, und genau ein
    /// strenger Lauf ueber genau diese Bytes; zuletzt die Zaehlpruefung.
    #[test]
    fn nak313_m44_p1_weiche_ein_lauf() {
        let kopf = tabelle::kopf();
        let links = &kopf["eingaenge"]["rust_p1"]["einspeisung"]["links"];
        let mut rot: Vec<String> = Vec::new();
        let mut gefahren = 0usize;
        for fall in tabelle::faelle(&kopf, "rust_p1") {
            gefahren += 1;
            let id = fall["id"].as_str().unwrap_or("?").to_owned();
            let nachricht = fall["nachricht"].as_str().unwrap_or("");
            let Some(art) = links[nachricht]["link_art"].as_str() else {
                rot.push(format!("{id}: die Einspeisung kennt keinen Link fuer {nachricht}"));
                continue;
            };
            let adresse: Adresse = serde_json::from_value(links[nachricht]["adresse"].clone())
                .expect("die Einspeisung traegt eine Adresse");
            let bytes = tabelle::bytes(&fall);
            let c = Coordinator::default();
            link_anmelden(&c, art, &adresse);
            let vorher = wirkungsstand(&c, &adresse);
            laeufe::vergessen();
            Senke::p1_mit_minor(&c, "link-p1", JSON_SCHEMA_MINOR_AKTIV, &bytes);
            let laeufe_ueber_diese_bytes = laeufe::ueber(&bytes);
            let nachher = wirkungsstand(&c, &adresse);
            let angenommen = nachher.0 > vorher.0 || nachher.1 > vorher.1;
            let ist = match (angenommen, lesestufe(&bytes)) {
                (true, _) => ("gueltig".to_owned(), None),
                (false, Some(stufe)) => ("ungueltig".to_owned(), Some(stufe.to_owned())),
                (false, None) => ("ungueltig".to_owned(), Some("feldregel".to_owned())),
            };
            let soll = tabelle::urteil(&fall, "produkt");
            if ist != soll {
                rot.push(format!("{id}: Urteil/Stufe {ist:?}, soll {soll:?}"));
            }
            for wirkung in tabelle::wirkung(&fall) {
                let gehalten = match wirkung.as_str() {
                    "annahme" => angenommen,
                    "ablehnung" => !angenommen,
                    "keine_teilmutation" => nachher == vorher,
                    _ => false,
                };
                if !gehalten {
                    rot.push(format!("{id}: Wirkung {wirkung} nicht gehalten ({vorher:?} -> {nachher:?})"));
                }
            }
            if laeufe_ueber_diese_bytes != 1 {
                rot.push(format!(
                    "{id}: {laeufe_ueber_diese_bytes} strenge Laeufe ueber dieselben Bytes, zugesagt ist genau einer"
                ));
            }
        }
        let soll_anzahl = kopf["anzahl_je_eingang"]["rust_p1"].as_u64().unwrap_or(0) as usize;
        assert!(
            gefahren == soll_anzahl && gefahren > 0,
            "M-49 Zaehlpruefung rust_p1: {gefahren} Eintraege gefahren, der Kopf nennt {soll_anzahl}"
        );
        assert!(rot.is_empty(), "rust_p1 weicht von `produkt` ab:\n{}", rot.join("\n"));
    }
}
