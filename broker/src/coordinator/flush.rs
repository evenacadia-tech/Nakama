//! Snapshotzustellung, Routing und Konfliktriegel.
//!
//! Der Flush sammelt seine Ziele unter dem Lock und stellt danach ohne Lock zu;
//! ein dauerhafter Konfliktriegel entscheidet fail-closed, bevor irgendetwas
//! geroutet wird.

use super::*;

/// NAK-246 D5 (R-D5, Manifest §5.5 Feinheit 1): die Marke eines Pushs ohne
/// eigenes Ordinal - der Livestand bei degradiertem Store und ein Angebot,
/// dessen Ablage scheiterte. Der Empfaenger gibt ihm die hoechste bereits
/// angenommene Marke seines Schluessels (`SessionPush::snapshot_schreiben`);
/// er ist damit nie ein Nachzuegler. Keine neue Zahl: gezogen wird nichts,
/// die Marke ist das Hochwasser.
pub(super) const MARKE_OHNE_ORDINAL: i64 = i64::MIN;

impl Coordinator {
    pub(super) fn store_degradiert(&self) -> bool {
        self.store
            .as_ref()
            .is_some_and(|store| store.sicht().degradiert)
    }

    pub fn routing_bereit(&self) -> bool {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .routing_bereit
    }

    pub(super) fn routing_fail_closed(&self, _grund: &str) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand.routing_bereit = false;
        for link in stand.links.values_mut() {
            link.trennen = true;
        }
    }

    pub(super) fn flush_session(&self, session: &SessionKey, verursacher_link: Option<&str>) {
        let shard = self.session_flush_shard(session);
        let _flush_guard = self.session_flush_schloesser
            [shard % self.session_flush_schloesser.len()]
        .lock()
        .unwrap_or_else(|e| e.into_inner());

        // 🔑 NR-04 (Nacharbeit 1, 07.09.2026), M-28: die Belege werden gegen
        // den Store gehalten, BEVOR der Snapshot entsteht.
        //
        // Das ist die zweite der beiden Stellen aus M-28 — und die
        // PRODUKTSEITIGE: `session_snapshot_json` ist die Lesesicht der Beine,
        // der Push geht hier durch. Eine `evidence`-Zeile, die nach dem
        // Eintragen verschwindet (Kompaktierung, externer Eingriff), wuerde
        // sonst weiter als Beleg einer sichtbaren Behauptung reisen.
        //
        // Die Haertung steht VOR dem Standlock: sie nimmt ihn selbst, und der
        // Store liest ausserhalb. Ohne Befunde in der Sitzung kehrt sie nach
        // einem Lockdurchgang zurueck, ohne SQLite anzufassen — der
        // Normalfall bei 1 bis 4 Hz kostet damit keine Leserunde.
        self.befunde_gegen_store_haerten(session);

        let (payload, ziele, sequence) = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            // D12-Naht (H-04): scharf gestellt panisiert der Flush hier, UNTER
            // dem Standlock - genau das vergiftet ihn. Einmalig, damit die
            // Messung danach ungestoert laeuft.
            if self
                .test_panik_unter_standlock
                .compare_exchange(true, false, Ordering::SeqCst, Ordering::SeqCst)
                .is_ok()
            {
                panic!("D12-Testnaht: absichtliche Panik unter dem Standlock");
            }
            if !stand.dirty_sessions.remove(session) {
                return;
            }
            let payload = self.snapshot_locked(&stand, session);
            // 🔑 NAK-246 D5 (§5.5 Feinheit 1, kein Store): die `event_sequence`
            // wird HIER gezogen - unter dem Standlock, im selben Zug wie die
            // Erfassung und vor dem Store-Zweig. `resubscribe_snapshot_push`
            // zieht seine unter demselben Lock; so folgt die Marke ohne Store
            // der Reihenfolge der Erfassung, und ein frueher erfasster Flush,
            // der spaeter zustellt, traegt die kleinere. Mit Store ist die Marke
            // das Ordinal des Commits, und die Zahl reist wie bisher nur als
            // `sequence` des Ereignisses.
            let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
            let ziele = stand
                .subscriptions
                .iter()
                .filter(|(link_id, sub)| {
                    sub.session_epoch == session.session_epoch
                        && sub.adresse.project_binding_id == session.project_binding_id
                        && stand.links.get(*link_id).is_some_and(|link| {
                            !link.trennen
                                && stand.routing_bereit
                                && !stand.guard_gesetzt(&effektive_adresse(&link.adresse))
                                && self.alias_register.session_push_erlaubt(
                                    &link.alias_adressraum,
                                    &link.alias_besitzer,
                                    &link.adresse.instance_id,
                                )
                        })
                })
                .map(|(link_id, sub)| {
                    (
                        link_id.clone(),
                        SnapshotZiel {
                            project_binding_id: session.project_binding_id.clone(),
                            session_epoch: session.session_epoch.clone(),
                            instance_id: sub.adresse.instance_id.clone(),
                            object_key: "session_snapshot".into(),
                        },
                    )
                })
                .collect::<Vec<_>>();
            (payload, ziele, sequence)
        };

        let test_haken = self
            .flush_test_haken
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .take();
        if let Some(haken) = test_haken {
            haken.erreichen();
        }

        let mut event_ord = None;
        if let Some(store) = &self.store {
            let mut event = StoreEvent::session_snapshot(
                &session.project_binding_id,
                &session.session_epoch,
                &self.broker_epoch,
                sequence.min(i64::MAX as u64) as i64,
                payload.clone(),
            );
            event.schema_major = STORE_SCHEMA_MAJOR;
            event.snapshot_ziele = ziele.iter().map(|(_, ziel)| ziel.clone()).collect();
            match store.append(vec![event]) {
                Ok(ausgaenge) => event_ord = ausgaenge.first().map(|a| a.event_ord()),
                Err(_) => {
                    let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
                    stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
                    if let Some(link_id) = verursacher_link {
                        if let Some(link) = stand.links.get_mut(link_id) {
                            link.trennen = true;
                        }
                    }
                    return;
                }
            }
        }

        // 🔑 NAK-246 D5 (R-D5): die Ordnungsmarke dieses Flushs - das
        // Store-Ordinal seines Commits, ohne Store die oben gezogene
        // `event_sequence`. Keine neue Zahl. Die Commit-Reihenfolge ist unter
        // dem Schloss serialisiert; die Marke traegt sie ueber das Schloss
        // hinaus bis in die Writerqueue, wo die Zustellung sonst ungeordnet
        // waere.
        let marke = match (&self.store, event_ord) {
            (Some(_), Some(ord)) => ord,
            // Ein Commit ohne Ausgang kommt nicht vor (je Ereignis ein
            // `AppendAusgang`); traete er auf, gaebe es kein Ordinal.
            (Some(_), None) => MARKE_OHNE_ORDINAL,
            (None, _) => sequence.min(i64::MAX as u64) as i64,
        };

        // Die Reihenfolge ist bis einschliesslich Store-/Outbox-Commit
        // serialisiert. Externe Pipe-Arbeit laeuft danach ohne dieses Schloss;
        // eine Senke darf den Coordinator reentrant beobachten, ohne dieselbe
        // Session zu deadlocken.
        drop(_flush_guard);

        // 🔑 NAK-246 D5 (§5.5 Feinheit 6): der ZWEITE Haken - nach der Freigabe
        // des Schlosses, vor der Zustellung. Er macht die Szene des Audits
        // deterministisch: dieser Flush hat committet und steht; ein spaeterer
        // derselben Sitzung committet und stellt zu, was nur geht, weil das
        // Schloss hier frei ist; danach reiht dieser seinen aelteren Stand ein.
        // Er nutzt denselben Platz wie der erste Haken: wer ihn setzt, waehrend
        // dieser Flush am ersten steht, trifft ihn hier. Produktion setzt ihn nie.
        let zustell_haken = self
            .flush_test_haken
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .take();
        if let Some(haken) = zustell_haken {
            haken.erreichen();
        }

        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        for (link_id, ziel) in ziele {
            let geschrieben = self.push_ziel_noch_gueltig(&link_id, &ziel)
                && push.as_ref().is_some_and(|push| {
                    push.snapshot_schreiben(&link_id, &ziel.object_key, marke, &payload)
                });
            if geschrieben {
                if let (Some(store), Some(ord)) = (&self.store, event_ord) {
                    let _ = store.snapshot_schuld_kompaktieren(ziel, ord);
                }
            }
        }
    }

    pub(super) fn session_flush_shard(&self, session: &SessionKey) -> usize {
        let mut shard = 0usize;
        for byte in session
            .project_binding_id
            .bytes()
            .chain(session.session_epoch.bytes())
        {
            shard = shard.wrapping_mul(16777619) ^ usize::from(byte);
        }
        shard
    }

    pub(super) fn guards_persistieren(&self, guards: Vec<ConflictGuard>) {
        for guard in guards {
            if self
                .store
                .as_ref()
                .is_some_and(|store| store.konflikt_guard_persistieren(guard).is_err())
            {
                self.routing_fail_closed("Konfliktriegel konnte nicht persistiert werden");
            }
        }
    }

    pub fn konflikt_guard_aufloesen(&self, effective_address: &str, derived_id: &str) -> bool {
        if let Some(store) = &self.store {
            if store
                .konflikt_guard_aufloesen(effective_address.into(), derived_id.into())
                .is_err()
            {
                return false;
            }
        }
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand.guard_entfernen(effective_address, derived_id);
        // H-14: die Aufloesung raeumt den passenden Aliaseintrag mit. Bis
        // NAK-121 lief sie nur ueber Store und Coordinatorstand, waehrend die
        // Quarantaene des Aliasregisters stehen blieb - der Riegel war fort,
        // aber jede Registrierung derselben Instanz fiel weiter an ihr.
        // C-07 bleibt unveraendert: die Aufloesung geschieht ausschliesslich
        // ueber diese explizite Neu-ID, nie ueber Zeit.
        //
        // Die derived_id traegt die Form `<instance_id>:<runtime_nonce>`
        // (vergeben in link.rs); die Quarantaene fuehrt denselben Wert als
        // Besitzerschluessel.
        //
        // D8 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): der
        // Adressraum wurde ueber die LEBENDEN Links gesucht. Trennten sich
        // beide kollidierenden Links vor der Aufloesung, blieb die Liste leer -
        // Store- und Coordinator-Guard fielen, die Methode meldete Erfolg, und
        // der Aliasbesitzer blieb quarantaenisiert. Riegel und Alias liefen
        // auseinander. Das Quarantaeneregister kennt seinen Besitzer selbst;
        // ueber es wird jetzt gesucht, damit beide nie mehr auseinanderlaufen.
        drop(stand);
        self.alias_register.quarantaene_aufloesen_ueberall(derived_id);
        true
    }

    pub fn dispatch_fuer_link_erlaubt(&self, link_id: &str) -> bool {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand
            .links
            .get(link_id)
            .is_some_and(|link| self.dispatch_fuer_link_erlaubt_locked(&stand, link))
    }

    pub(super) fn dispatch_fuer_link_erlaubt_locked(&self, stand: &Stand, link: &LinkStand) -> bool {
        stand.routing_bereit
            && !link.trennen
            && !stand.guard_gesetzt(&effektive_adresse(&link.adresse))
            && self.alias_register.dispatch_erlaubt(
                &link.alias_adressraum,
                &link.alias_besitzer,
                &link.adresse.instance_id,
            )
    }

    pub(super) fn alias_quarantaenisiert(&self, link_id: &str) -> bool {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand.links.get(link_id).is_some_and(|link| {
            !stand.routing_bereit
                || stand.guard_gesetzt(&effektive_adresse(&link.adresse))
                || self
                    .alias_register
                    .ist_quarantaenisiert(&link.alias_adressraum, &link.alias_besitzer)
        })
    }

    pub(super) fn store_verweigert_fuer_link(&self, link_id: &str) {
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
        if let Some(link) = stand.links.get_mut(link_id) {
            link.trennen = true;
        }
    }
}
