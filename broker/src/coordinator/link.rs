//! Control-Link an und ab.
//!
//! Hier entsteht und endet die Bindung eines Clients an seine Session; die
//! Verdraengung einer alten Nonce laeuft vollstaendig unter dem Lock, bevor der
//! Transport ihren Abbau beginnt.

use super::*;

impl Coordinator {
    /// Kompatibler Phase-A-Einstieg fuer direkte Modelltests.
    pub fn control_registrieren(&self, link_id: &str, adresse: Adresse) {
        let hello = HelloControl {
            typ: "hello".into(),
            connection_kind: "control".into(),
            protocol: 3,
            plugin_version: "test".into(),
            plugin_kind: "active_probe".into(),
            adresse,
            host: None,
            audio: AudioLage {
                samplerate: 48_000.0,
                block_size: 512,
                channels: 2,
            },
        };
        let _ = self.control_hello_registrieren(link_id, &hello);
    }

    pub(super) fn ist_ungebundene_probe(hello: &HelloControl) -> bool {
        matches!(hello.plugin_kind.as_str(), "active_probe" | "passive_probe")
            && hello.adresse.session_epoch == hello.adresse.project_binding_id
    }

    pub(super) fn main_sessions_fuer_probe_locked(
        stand: &Stand,
        adresse: &Adresse,
        host_pid: u32,
    ) -> HashSet<SessionKey> {
        stand
            .clients
            .iter()
            .filter(|(key, client)| {
                client.plugin_kind == "main"
                    && client.host_pid == Some(host_pid)
                    && key.logon_sid == adresse.logon_sid
                    && key.project_binding_id == adresse.project_binding_id
            })
            .map(|(key, _)| key.session())
            .collect()
    }

    pub(super) fn eindeutige_main_session_locked(
        stand: &Stand,
        adresse: &Adresse,
        host_pid: Option<u32>,
    ) -> Option<SessionKey> {
        let sessions = Self::main_sessions_fuer_probe_locked(stand, adresse, host_pid?);
        (sessions.len() == 1).then(|| sessions.into_iter().next().expect("genau eine Session"))
    }

    pub(super) fn ungebundene_links_mit_eindeutigem_main_locked(stand: &Stand) -> Vec<String> {
        stand
            .links
            .iter()
            .filter(|(_, link)| {
                stand.clients.get(&link.client_key).is_some_and(|client| {
                    client.session_ungebunden
                        && Self::eindeutige_main_session_locked(
                            stand,
                            &link.wire_adresse,
                            client.host_pid,
                        )
                        .is_some()
                })
            })
            .map(|(link_id, _)| link_id.clone())
            .collect()
    }

    pub fn control_hello_registrieren(
        &self,
        link_id: &str,
        hello: &HelloControl,
    ) -> ControlRegistrierung {
        let jetzt = self.clock.jetzt();
        let wire_adresse = hello.adresse.clone();
        let host_pid = hello.host.as_ref().map(|h| h.pid);
        let mut adresse = wire_adresse.clone();
        let mut session_ungebunden = Self::ist_ungebundene_probe(hello);
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        if session_ungebunden {
            if let Some(main_session) =
                Self::eindeutige_main_session_locked(&stand, &wire_adresse, host_pid)
            {
                adresse.session_epoch = main_session.session_epoch;
                session_ungebunden = false;
            }
        }
        let key = ClientKey::aus_adresse(&adresse);
        let alias_adressraum = Sitzungsadressraum::neu(
            &adresse.logon_sid,
            &adresse.project_binding_id,
            &adresse.session_epoch,
        );
        let alias_besitzer = format!("{}:{}", adresse.instance_id, adresse.runtime_nonce);
        let effective = effektive_adresse(&adresse);
        let mut guards_zu_persistieren = Vec::new();
        let mut schliessen = Vec::new();

        Self::stale_aktualisieren_locked(&mut stand, jetzt);
        if !stand.clients.contains_key(&key) {
            if let Err(grund) = self.platz_schaffen_locked(&mut stand, &key, jetzt, &mut schliessen)
            {
                stand.cap_abweisungen = stand.cap_abweisungen.saturating_add(1);
                return ControlRegistrierung::abgewiesen(grund);
            }
        }

        // D6, Nachzug: der Aliasdeckel wird geprueft, BEVOR die Verdraengung
        // unten einen lebenden Link auf Trennen setzt. Sonst haette ein Hello,
        // das gleich abgewiesen wird, die bestehende Verbindung noch
        // mitgenommen - fail-closed waere dann teurer als noetig. Die
        // verbindliche Entscheidung faellt weiter unten unter dem Registerlock;
        // diese Abfrage ist die guenstige Vorwegnahme, so wie der Clientdeckel
        // darueber.
        if self.alias_register.deckel_wuerde_reissen() {
            stand.cap_abweisungen = stand.cap_abweisungen.saturating_add(1);
            return ControlRegistrierung::abgewiesen("alias_quarantaene_deckel");
        }

        let geerbt = stand.clients.get(&key).cloned();
        if let Some(alt) = &geerbt {
            if alt.current_nonce != adresse.runtime_nonce {
                // Liveframes und ihre Fehler gehoeren zur Runtime-Nonce. Die
                // stabile Mitgliedschaft bleibt am ClientKey, aber ein neuer
                // Prozess darf niemals die Messwahrheit des alten erben.
                stand.messframes.remove(&key);
                stand.messfehler.remove(&key);
                stand.lautheit.remove(&key);
                // B15: dieselbe Regel fuer die Evidenzhistorie - ein neuer
                // Prozess erbt die Messwahrheit des alten nicht.
                stand.evidenz.remove(&key);
            }
            if let Some(alter_link) = alt.current_link.as_deref() {
                // H-10, zweite Haelfte: auch ein Hello mit IDENTISCHER Nonce
                // verdraengt den aelteren Link. C-10 sagt Verdraengung nur fuer
                // die abweichende Nonce zu und schweigt zum identischen Fall -
                // eine Luecke, durch die zwei lebende Links dieselbe Adresse
                // tragen konnten. Damit entsteht die Voraussetzung des
                // Nichtdeterminismus in persistenz_p0 gar nicht erst.
                if alter_link != link_id {
                    let gleiche_nonce = alt.current_nonce == adresse.runtime_nonce;
                    if let Some(link) = stand.links.get_mut(alter_link) {
                        link.verdraengt = true;
                        link.trennen = true;
                        // Bei IDENTISCHER Nonce ist der Aliasbesitzer beider
                        // Links derselbe Schluessel (instance_id:runtime_nonce).
                        // Ihn hier zu entfernen zoege dem ueberlebenden Link
                        // seine Wireadresse weg - deshalb bleibt der Eintrag in
                        // genau diesem Zweig stehen.
                        //
                        // Ehrlich vermerkt: heute ist diese Ausnahme nicht von
                        // aussen beobachtbar, weil die Registrierung unten
                        // (`registriere_wire_zuordnung`) denselben Eintrag im
                        // selben gesperrten Block sofort wieder anlegt. Sie
                        // bleibt trotzdem, weil sie die Reihenfolge unabhaengig
                        // von dieser Nachbarschaft richtig haelt - verschoebe
                        // jemand die Registrierung, waere der Verlust sonst
                        // still. Ein eigener Rotbeweis existiert deshalb nicht;
                        // der Bau-Verlauf des Manifests nennt den Grund.
                        if !gleiche_nonce {
                            self.alias_register.entferne(
                                &link.alias_adressraum,
                                &link.alias_besitzer,
                                &link.adresse.instance_id,
                            );
                        }
                    }
                    Self::subscription_entfernen_locked(&mut stand, alter_link);
                    schliessen.push(alter_link.to_owned());
                    // H-16, Schritt 1: die verdraengte Session wird unter dem
                    // Lock nur DIRTY markiert, nicht gepusht. Ein Push vor der
                    // Rueckkehr liefe vor dem Abbau, den der Transport erst
                    // danach beginnt (server_v3.rs, Rueckgabe der zu
                    // schliessenden Links) - und damit vor Ingress-Schluss,
                    // Kopplungsloesung, Joins und Trenncallbacks. Das verletzte
                    // C-06. Den Push loest Schritt 3 in control_getrennt aus.
                    stand.dirty_sessions.insert(key.session());
                    stand
                        .verdraengt_wartet_auf_push
                        .insert(alter_link.to_owned());
                }
            }
        }

        let registrierung = self.alias_register.registriere_wire_zuordnung(
            &alias_adressraum,
            &alias_besitzer,
            &adresse.instance_id,
        );
        // D6 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): H-14
        // sagt zu, dass am Deckel die VERBINDUNG abgewiesen wird - der
        // fail-closed-Ausgang, den C-07 ohnehin fordert. Behandelt wurde hier
        // aber nur die Kollisionsvariante; `DeckelErreicht` fiel durch, Client
        // und Link entstanden und der Peer bekam ein Welcome statt der
        // zugesagten Ablehnung. Jetzt endet die Registrierung hier, bevor
        // irgendetwas angelegt ist.
        //
        // R2-2 (Nacharbeit Runde 2, 03.09.2026): BEIDE Deckel - die
        // Aliasreservierung und der persistente Riegelindex - liegen jetzt in
        // `kollisionsriegel_setzen_locked`, weil der Reportpfad eines
        // verdraengten Links (liveness.rs) dieselbe Pruefung braucht. H-14
        // gilt fuer jeden Kollisionspfad, nicht nur fuer das Hello. Der Store
        // wuerde den Schreibversuch ohnehin mit einem Fehler beantworten, aber
        // erst NACH dem Anlegen von Client und Link, und die Antwort waere nur
        // ein Routing-Abschalten statt einer Abweisung.
        let mut ids = vec![alias_besitzer.clone()];
        if registrierung == Registrierung::KollisionBeideQuarantaenisiert {
            if let Some(alt) = &geerbt {
                ids.push(format!("{}:{}", alt.adresse.instance_id, alt.current_nonce));
            }
        }
        match kollisionsriegel_setzen_locked(
            &mut stand,
            registrierung,
            &effective,
            &ids,
            registrierung == Registrierung::KollisionBeideQuarantaenisiert,
        ) {
            Deckelausgang::Deckel(grund) => {
                return ControlRegistrierung::abgewiesen(grund);
            }
            Deckelausgang::Frei(guards) => {
                guards_zu_persistieren.extend(guards);
            }
        }

        let client = ClientStand {
            adresse: adresse.clone(),
            plugin_kind: hello.plugin_kind.clone(),
            host_pid,
            abtastrate: hello.audio.samplerate,
            session_ungebunden,
            current_link: Some(link_id.to_owned()),
            current_nonce: adresse.runtime_nonce.clone(),
            last_seen: jetzt,
            stale: false,
            stale_seit: None,
            join_kandidat: geerbt.as_ref().is_some_and(|c| c.join_kandidat),
            bestaetigt: geerbt.as_ref().is_some_and(|c| c.bestaetigt),
            explizit_bestaetigt: geerbt.as_ref().is_some_and(|c| c.explizit_bestaetigt),
            ausdruecklich_ungebunden: geerbt.as_ref().is_some_and(|c| c.ausdruecklich_ungebunden),
            // Runtime-Felder sind linkgebunden. Auch ein Reconnect derselben
            // Runtime muss den laut E-M01 vorgeschriebenen ersten
            // heartbeat.runtime erneut liefern; bis dahin bleibt die Quelle
            // als Mitglied ohne Messdescriptor sichtbar.
            descriptor: None,
            state_revision: geerbt.as_ref().and_then(|c| c.state_revision),
            state_hash: geerbt.as_ref().and_then(|c| c.state_hash.clone()),
            record_state_valid: geerbt.as_ref().is_some_and(|c| c.record_state_valid),
            recording: geerbt.as_ref().is_some_and(|c| c.recording),
        };
        stand.clients.insert(key.clone(), client);
        stand.sessions.entry(key.session()).or_default();
        stand.links.insert(
            link_id.to_owned(),
            LinkStand {
                wire_adresse,
                adresse,
                client_key: key,
                alias_adressraum,
                alias_besitzer,
                letzte_event_sequence: None,
                verdraengt: false,
                trennen: false,
                join_neuverbinden: false,
            },
        );
        // Kam die Probe vor dem Main, bleibt sie intern ungebundener Kandidat.
        // Sobald genau eine passende Main-Sitzung existiert, schliesst der
        // Coordinator den alten Link kontrolliert. Der normale Reconnect-Hello
        // wird dann direkt mit der Main-Epoche registriert; keine Alias- oder
        // Subscription-Hoheit wird unter einer laufenden Verbindung umgehängt.
        for pending in Self::ungebundene_links_mit_eindeutigem_main_locked(&stand) {
            if let Some(link) = stand.links.get_mut(&pending) {
                if !link.trennen {
                    link.trennen = true;
                    link.join_neuverbinden = true;
                }
            }
            schliessen.push(pending);
        }
        schliessen.sort();
        schliessen.dedup();
        drop(stand);

        for guard in guards_zu_persistieren {
            if self
                .store
                .as_ref()
                .is_some_and(|store| store.konflikt_guard_persistieren(guard).is_err())
            {
                self.routing_fail_closed("Konfliktriegel konnte nicht persistiert werden");
            }
        }
        ControlRegistrierung::angenommen(schliessen)
    }

    /// Der bestaetigte Resync nach einem Reconnect (M-61, Befund B16).
    ///
    /// 🔑 `neutral_resync` hatte ausserhalb der Tests keinen Aufrufer. Nach
    /// dem ersten Control-Disconnect blieb das sticky Unknown deshalb fuer
    /// immer stehen — und mit ihm die Sperre auf starker Evidenz. §34.2 nennt
    /// den Resync ausdruecklich als den EINEN Entsperrweg; ohne Aufrufer war
    /// er keiner.
    ///
    /// Er laeuft NICHT automatisch beim Verbindungsaufbau: ein Reconnect
    /// allein sagt nichts ueber den Interventionszustand. Bestaetigt heisst,
    /// dass der Peer seine Sequenzbasis MITBRINGT — erst damit weiss der
    /// Broker, ab welcher Nummer er wieder lueckenlos zaehlen darf.
    pub fn resync_bestaetigen(&self, link_id: &str, bestaetigte_sequence_basis: u64) -> bool {
        // Nur ein Link, der wirklich steht, darf entsperren. Ein Resync von
        // einem sterbenden Link waere eine Freigabe ohne Gegenueber.
        let steht = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand
                .links
                .get(link_id)
                .is_some_and(|link| !link.trennen && !link.verdraengt)
        };
        if !steht {
            return false;
        }
        self.neutral_resync(link_id, bestaetigte_sequence_basis)
    }

    /// Der synchrone Server-Hook. Subscription, Link und aktive Ereignisse
    /// werden unter EINEM Lock entfernt; gleichzeitig wird Unknown sticky.
    /// Ein Push teilt diesen Lock und kann den Zwischenzustand nicht sehen.
    pub fn control_ende(&self, link_id: &str) {
        let jetzt = self.clock.jetzt();
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let mut dirty = None;
        // M-62: die Sitzung des STERBENDEN Links, bevor er aus der Map faellt.
        // Danach ist sie nicht mehr nachschlagbar, und ein `None` an dieser
        // Stelle wuerde jeden Disconnect brokerweit taint setzen — genau die
        // Vermischung, die B17 aufgeraeumt hat.
        let sterbende_session = Self::session_des_links(&stand, link_id);
        let war_ungebunden = stand
            .links
            .get(link_id)
            .and_then(|l| stand.clients.get(&l.client_key))
            .is_some_and(|c| c.session_ungebunden);
        if let Some(link) = stand.links.remove(link_id) {
            // D3 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026):
            // Diese Entfernung war bedingungslos. Seit H-10 verdraengt auch ein
            // Hello mit IDENTISCHER Nonce den aelteren Link - und beide Links
            // teilen dann denselben Aliasbesitzer im selben Adressraum. Die
            // Registrierung laesst den Aliaseintrag in diesem Zweig deshalb
            // absichtlich stehen; erreichte der alte Link danach `control_ende`,
            // riss diese Zeile genau die GEMEINSAME Zuordnung weg und der
            // ueberlebende Link fiel fail-closed aus. Der Alias faellt jetzt
            // nur, wenn kein anderer lebender Link denselben Besitzer im selben
            // Adressraum traegt. `stand.links` enthaelt den sterbenden Link an
            // dieser Stelle nicht mehr, die Schleife sieht also nur Ueberlebende.
            let alias_wird_geteilt = stand.links.values().any(|anderer| {
                anderer.alias_adressraum == link.alias_adressraum
                    && anderer.alias_besitzer == link.alias_besitzer
            });
            if !alias_wird_geteilt {
                self.alias_register.entferne(
                    &link.alias_adressraum,
                    &link.alias_besitzer,
                    &link.adresse.instance_id,
                );
            }
            let join_reconnect_ohne_tombstone = link.join_neuverbinden
                && link.letzte_event_sequence.is_none()
                && !stand.taint.values().any(|taint| {
                    taint
                        .interventionen
                        .values()
                        .any(|intervention| intervention.link_id == link_id)
                });
            if join_reconnect_ohne_tombstone {
                // Der Marker ist keine Sessionidentitaet und darf nach dem
                // kontrollierten Reconnect keinen Phantom-Tombstone erzeugen.
                stand.clients.remove(&link.client_key);
                stand.messframes.remove(&link.client_key);
                stand.messfehler.remove(&link.client_key);
                stand.lautheit.remove(&link.client_key);
                // 🔑 Nacharbeit 1 (Befund B15): die Evidenzhistorie gehoert in
                // DENSELBEN Remove-Block wie ihre drei Nachbarmaps. Ohne diese
                // Zeile wuchs sie ueber wiederholte Sessionepochen hinweg
                // unbegrenzt weiter und hielt stale Evidenz - trotz aller
                // Client-Deckel (M-74, verbinden↔trennen).
                stand.evidenz.remove(&link.client_key);
            } else if let Some(client) = stand.clients.get_mut(&link.client_key) {
                if client.current_link.as_deref() == Some(link_id) {
                    client.current_link = None;
                    client.stale = true;
                    client.stale_seit = Some(jetzt);
                    dirty = Some(link.client_key.session());
                }
            }
        }
        if let Some(session) = dirty {
            stand.dirty_sessions.insert(session);
        }
        Self::subscription_entfernen_locked(&mut stand, link_id);
        stand.telemetry_links.remove(link_id);
        // C-08 gilt fuer jeden Control-Disconnect, auch wenn der interne
        // Joinpfad den Reconnect angefordert hat. Nur neutral_resync loest.
        //
        // M-62: das trifft die SITZUNG dieses Links, nicht den ganzen Broker.
        match sterbende_session {
            Some(session) => {
                // ⚠️ Hat die Sitzung ueberhaupt noch einen Client? Der
                // Joinreconnect-Zweig oben entfernt ihn ausdruecklich, "damit
                // der Marker keinen Phantom-Tombstone erzeugt". Ein sticky
                // Unknown auf einer Sitzung, die es nicht mehr gibt, WAERE ein
                // solcher Phantom: niemand koennte ihn je wieder loesen, und
                // die Map waechst mit jeder Epoche weiter (M-74). Sie faellt
                // deshalb ganz — es gibt nichts mehr zu schuetzen.
                let hat_clients = stand
                    .clients
                    .keys()
                    .any(|key| key.session() == session);
                if hat_clients {
                    let taint = Self::taint_mut(&mut stand, &session);
                    taint
                        .interventionen
                        .retain(|_, intervention| intervention.link_id != link_id);
                    taint.unknown = true;
                } else {
                    stand.taint.remove(&session);
                }
            }
            // Ein Link ohne auffindbare Sitzung: fail-closed ueber alle.
            None => Self::alle_sitzungen_unbekannt(&mut stand),
        }
        // ⚠️ Eine UNGEBUNDENE Probe hat noch gar keine echte Sitzung - ihr
        // `session_epoch` ist der projektgebundene Joinmarker. Ihr Disconnect
        // taintet deshalb zusaetzlich den Platzhalter: die Frage „welche
        // Sitzung war betroffen" ist hier ehrlich unbeantwortbar, und
        // fail-closed heisst dann ALLE, nicht KEINE (§34.2). Der Platzhalter
        // ist genau der Ort dafuer, und `neutral_resync` loest ihn mit.
        if war_ungebunden {
            Self::taint_mut(&mut stand, &SessionKey::unbekannt()).unknown = true;
        }
    }

    pub fn verbindung_soll_trennen(&self, link_id: &str) -> bool {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .links
            .get(link_id)
            .is_some_and(|link| link.trennen)
    }

    pub(super) fn adresse_des_links_passt(stand: &Stand, link_id: &str, adresse: &Adresse) -> bool {
        stand
            .links
            .get(link_id)
            .is_some_and(|link| &link.wire_adresse == adresse)
    }

    pub(super) fn aktueller_telemetrie_client_locked(stand: &Stand, link_id: &str) -> Option<ClientKey> {
        if !stand.telemetry_links.contains(link_id) {
            return None;
        }
        let link = stand.links.get(link_id)?;
        if link.trennen {
            return None;
        }
        let client = stand.clients.get(&link.client_key)?;
        (client.current_link.as_deref() == Some(link_id)
            && client.current_nonce == link.adresse.runtime_nonce)
            .then(|| link.client_key.clone())
    }
}
