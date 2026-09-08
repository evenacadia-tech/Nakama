//! Der PRODUKTPFAD der Ursachenhypothese (SONDE-014 Etappe C).
//!
//! ── WARUM DIESE DATEI ÜBERHAUPT EXISTIERT ─────────────────────────────────
//!
//! Risiko **R1** des Bauplans, wörtlich aus der Wiederprüfung 1 der Etappe 2
//! von SONDE-013: „die Runde 1 hat Funktionen gebaut und Tests geschrieben,
//! die das Modul direkt aufrufen — der Produktpfad war an vielen Stellen nicht
//! geschlossen." `hypothese.rs` rechnet; diese Datei entscheidet, **wann**
//! gerechnet wird, **woraus**, und **wohin** das Ergebnis reist. Ein Test, der
//! nur `hypothese::hypothesen` ruft, misst keinen dieser drei Punkte.
//!
//! ── DER AUSLÖSER ──────────────────────────────────────────────────────────
//!
//! Gerechnet wird an genau zwei Stellen, und beide sind Evidenzänderungen:
//!
//! 1. nach jedem **angenommenen** `evidence_snapshot` (`evidenz.rs`), direkt
//!    neben `evidenz_paare_bilden` — dort kommt neues Material an;
//! 2. nach jeder **Rücknahme** (`invalidierung_verdrahtung.rs`), weil eine
//!    Hypothese ohne ihre Belege keine mehr ist (M-24, M-28).
//!
//! Ein dritter Auslöser („bei jedem Heartbeat") wäre eine Rechnung ohne
//! Anlass — und §33.5 kennt keinen Zustand, den sie ändern würde.
//!
//! ── DAS LOCK ──────────────────────────────────────────────────────────────
//!
//! Die Rechnung selbst läuft **außerhalb** des Standlocks. Ein Block-Bootstrap
//! über 400 Ziehungen je Kandidat hält sonst den ganzen Sessiongraphen an —
//! derselbe Grund, aus dem `evidenz_paare_bilden` seine Halbzeuge unter dem
//! Lock einsammelt und danach rechnet. Was **unter** dem Lock passieren muss,
//! ist die Invalidierung: M-24 verlangt Auflösen, Markieren und terminales
//! Invalidieren ausdrücklich unter **einem** Lock, sonst könnte zwischen
//! Markierung und Invalidierung eine Neurechnung dieselbe Hypothese wieder
//! aufbauen.
//!
//! ── WAS „ROUTING BEKANNT" HIER HEISST ─────────────────────────────────────
//!
//! Der Deskriptor trägt `host_mixer_index` als **optionales** Feld
//! (`$defs/probe_descriptor_*`). Er trägt **keine** Elternkante — der Broker
//! kennt FL's Routinggraphen nicht. M-22 verlangt aber genau zwei Dinge, und
//! beide sind mit dem messbar, was da ist:
//!
//! * „bei unbekanntem Routing gibt es keine starke Aussage" → ohne
//!   `host_mixer_index` ist `routing_bekannt` falsch, und `routingqualitaet`
//!   wird 0;
//! * „Parent und Child erzeugen nicht zwei starke Behauptungen über dieselbe
//!   Ursache" → zwei Quellen auf **demselben** `host_mixer_index` messen
//!   dasselbe Signal; beide werden als Duplikat geführt und keine erreicht
//!   `hoch`.
//!
//! Eine erfundene Elternkante wäre die teurere Lösung: sie sähe wie Wissen
//! aus, das der Broker nicht hat.

use super::hypothese::{
    hypothesen, Aufnahme, Befundzustand, CauseHypothesis, Evidenzfenster, Passagenfenster,
    Quellprofil,
};
use super::*;
use std::collections::BTreeMap;

impl Coordinator {
    // ═════════════════════════════════════════════════════════════════════
    // Auslöser
    // ═════════════════════════════════════════════════════════════════════

    /// Rechnet die Hypothesen aller Sitzungen neu, deren Evidenz sich
    /// geändert hat.
    ///
    /// Der Weg ist dreiteilig und in dieser Reihenfolge zwingend:
    /// **einsammeln unter dem Lock → rechnen ohne Lock → eintragen,
    /// persistieren, zustellen**. Wer mittendrin das Lock hielte, hielte den
    /// Sessiongraphen für die Dauer von 400 Bootstrapziehungen an.
    pub(super) fn hypothesen_bilden(&self) {
        let aufnahmen = self.aufnahmen_sammeln();
        if aufnahmen.is_empty() {
            return;
        }
        let mut ergebnisse: Vec<(SessionKey, Vec<CauseHypothesis>)> = Vec::new();
        for (session, aufnahme) in aufnahmen {
            let ergebnis = hypothesen(&aufnahme);
            ergebnisse.push((session, ergebnis.befunde));
        }
        // 🔑 NR-03 (Nacharbeit 1, 07.09.2026): der Testhaken sitzt GENAU
        // hier — zwischen der Rechnung ohne Lock und der Eintragung unter
        // dem Lock.
        //
        // Er ist das einzige Mittel, mit dem sich das Interleaving aus EP-03
        // deterministisch messen laesst: waehrend der Rechnung steigt die
        // Intent-Revision oder eine Evidenz wird zurueckgenommen. Zwei
        // Threads mit Barriere maessen dasselbe, aber nicht reproduzierbar —
        // und eine Zusage, deren Beweis flackert, ist keine.
        self.rechen_test_haken_ausloesen();
        for (session, befunde) in ergebnisse {
            let geaendert = self.befunde_eintragen(&session, befunde);
            // 🔑 SONDE-014 Etappe F: der Vorschlag entsteht MIT seinem Befund.
            //
            // Er haengt hier und nicht an einem eigenen Ausloeser: §42.1 bindet
            // jedes Proposal an `evidence_ids` und `finding_id`, und ein
            // Vorschlag ohne Befund haette weder das eine noch das andere.
            // Die Reihenfolge ist zwingend — erst der Befund im Stand, dann
            // der Vorschlag darauf.
            let vorschlaege_neu = self.vorschlaege_bilden(&session);
            if geaendert || vorschlaege_neu {
                self.befunde_zustellen(&session);
            }
        }
    }

    /// Wie `paare_bei_bedarf_bilden`: nur rechnen, wenn eine Änderung ansteht.
    pub(super) fn hypothesen_bei_bedarf_bilden(&self) {
        let noetig = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            std::mem::take(&mut stand.befunde_neu_bilden)
        };
        if noetig {
            self.hypothesen_bilden();
        }
    }

    // ═════════════════════════════════════════════════════════════════════
    // Einsammeln
    // ═════════════════════════════════════════════════════════════════════

    /// Baut je Sitzung eine `Aufnahme` aus dem flüchtigen Bestand.
    ///
    /// ⚠️ **Der Master ist der `main`, nicht der lauteste.** §8 erklärt einen
    /// MASTERbefund; Gen ist als einzige Instanz `plugin_kind = "main"` und
    /// misst laut Vertrag ausschließlich am Insert (`liveness.rs`). Eine
    /// Sitzung ohne Main hat keinen Master und rechnet gar nicht — das ist
    /// kein Fehler, sondern der normale Zustand einer Sondenrunde ohne Gen.
    fn aufnahmen_sammeln(&self) -> Vec<(SessionKey, Aufnahme)> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        // Feste Ordnung: eine `HashMap` hat keine, und zwei Läufe über
        // denselben Stand müssten sonst nicht dieselbe Reihenfolge ergeben
        // (M-25).
        let mut sitzungen: BTreeSet<(String, String)> = BTreeSet::new();
        for key in stand.evidenz.keys() {
            sitzungen.insert((key.project_binding_id.clone(), key.session_epoch.clone()));
        }
        let mut aus: Vec<(SessionKey, Aufnahme)> = Vec::new();
        for (projekt, epoche) in sitzungen {
            let session = SessionKey {
                project_binding_id: projekt,
                session_epoch: epoche,
            };
            // M-86: ohne Vollstaendigkeitsmarke wird nicht gerechnet. Der
            // Riegel steht HIER und nicht erst im Modul: eine Aufnahme, die
            // gar nicht erst entsteht, kann auch nicht versehentlich
            // gerechnet werden.
            //
            // 🔑 NR-01 (Nacharbeit 1, 07.09.2026): FAIL-CLOSED. Bis hierher
            // lautete die Bedingung `is_some_and(|i| !i.vollstaendig)` — bei
            // `intent == None` ist sie FALSCH, und ohne jede
            // Vollstaendigkeitsmeldung entstanden Findings und Proposals.
            // Die Frage steht jetzt an EINER Stelle
            // (`intent::darf_gerechnet_werden`) und `None` sperrt.
            let intent = stand.intent.get(&session).cloned();
            if !super::intent::darf_gerechnet_werden(intent.as_ref()) {
                continue;
            }
            // Die Fuehrungsfrage beantwortet ausschliesslich
            // `mitgliedschaft.rs`: `None` heisst „kein oder mehr als ein
            // bestaetigtes Main" (`fuehrung_neu_bewerten_locked`).
            let fuehrendes_main = stand
                .sessions
                .get(&session)
                .and_then(|s| s.fuehrendes_main.clone());
            let mut master: Option<Quellprofil> = None;
            let mut mains: Vec<String> = Vec::new();
            let mut kandidaten: Vec<Quellprofil> = Vec::new();
            // Wer misst welchen Mixerkanal? Zwei Quellen auf demselben Kanal
            // sind Duplikate (M-22); eine Sonde auf dem Kanal des Masters ist
            // sein Duplikat und nie seine Ursache (NAK-213 R3).
            let mut je_kanal: BTreeMap<i64, Vec<String>> = BTreeMap::new();
            // 🔑 **NAK-213 E5/E6:** iteriert werden die CLIENTS der Sitzung,
            // nicht die Evidenzschluessel.
            //
            // Bis hierher lief die Schleife ueber `stand.evidenz.keys()`. Eine
            // Quelle, die angemeldet und bestaetigt ist, aber noch nie einen
            // Beleg gesendet hat, betrat damit weder die Kandidatenliste noch
            // die Kanaltafel: der einzige Ueberlebende sah aus, als sei er
            // allein, und wurde `hoch` (Gate-Befunde G-D2, G-H1, E-L4).
            //
            // ⚠️ Die SITZUNGSMENGE kommt weiter aus den Evidenzschluesseln
            // (oben). Ohne einen einzigen Beleg gibt es nichts zu rechnen und
            // nichts zu melden — das ist keine verschwiegene Aussage, sondern
            // die Abwesenheit einer Messung (K-19).
            let mut keys: Vec<&ClientKey> = stand
                .clients
                .iter()
                .filter(|(key, client)| {
                    key.session() == session
                        && (client.plugin_kind == "main"
                            // „Angemeldet" heisst fuer eine QUELLE:
                            // bestaetigtes Mitglied der Sitzung (E5). Ein
                            // Join-Kandidat ohne Bestaetigung ist noch keines
                            // und deckelt nichts.
                            //
                            // ⚠️ Fuer das `main` gilt die Bedingung
                            // ausdruecklich NICHT: die Fuehrungsfrage
                            // beantwortet `fuehrendes_main`, und zwei
                            // unbestaetigte `main` sind kein Grund, die
                            // Sitzung aus der Rechnung zu nehmen — sie sind
                            // der Fall aus R4.
                            || (client.bestaetigt
                                // „Liefert nie" ist strukturell beantwortet,
                                // ohne Zahl: faellt der Client auf `stale`,
                                // ist er kein Mitglied mehr. Eine
                                // Kadenzgrenze waere eine geratene Zahl
                                // (M-31).
                                && !client.stale))
                })
                .map(|(key, _)| key)
                .collect();
            keys.sort_by(|a, b| a.instance_id.cmp(&b.instance_id));
            for key in &keys {
                let Some(client) = stand.clients.get(*key) else {
                    continue;
                };
                // Eine Quelle OHNE Historie ist zulaessig: sie wird Kandidat
                // mit LEERER Fensterfolge und faellt in `gate()` Schritt 0 mit
                // `evidenz_zurueckgenommen` — dem Grund, den der Bestand fuer
                // „alles zurueckgenommen ODER nie angekommen" schon fuehrt.
                let historie = stand.evidenz.get(*key);
                let kanal = client
                    .descriptor
                    .as_ref()
                    .and_then(|d| d.get("host_mixer_index"))
                    .and_then(Value::as_i64)
                    // Der Vertrag laesst den Hostindex erst ab 1 zu; darunter
                    // gilt er als NICHT GELIEFERT (`eq-ipc-v3.schema.json`
                    // `minimum: 1`). Er ist damit dasselbe Fehlen, kein
                    // dritter Zustand.
                    .filter(|index| *index >= 1);
                if let Some(index) = kanal {
                    je_kanal
                        .entry(index)
                        .or_default()
                        .push(key.instance_id.clone());
                }
                // Klasse 2 (§36.1) verlangt ein PRE/POST-PAAR mit Ergebnis.
                // Die Zuordnung Quelle → Paar steht im Deskriptor (`pair_id`);
                // das Urteil liegt in `paarurteile`. Ein Paar OHNE Ergebnis
                // ist keine staerkere Evidenz, sondern eine offene Messung.
                let prepost_paar = client
                    .descriptor
                    .as_ref()
                    .and_then(|d| d.get("pair_id"))
                    .and_then(Value::as_str)
                    .is_some_and(|pair_id| {
                        stand
                            .paarurteile
                            .get(&(session.clone(), pair_id.to_string()))
                            .is_some_and(|urteil| urteil.ergebnis.is_some())
                    });
                let profil = Quellprofil {
                    quelle_id: key.instance_id.clone(),
                    fenster: historie.map(Self::fenster_aus_historie).unwrap_or_default(),
                    routing_bekannt: kanal.is_some(),
                    mixerkanal: kanal,
                    parent: None,
                    prepost_paar,
                    // Ein Profil OHNE gültiges Fenster hat keine Belege mehr:
                    // entweder ist alles zurückgenommen oder nie angekommen.
                    // Beides trägt denselben Grund (M-24) — und seit NAK-213
                    // E5 zählt „nie angekommen" ausdrücklich dazu: eine
                    // Quelle ohne Historie ist `zurueckgenommen`.
                    zurueckgenommen: historie
                        .is_none_or(|h| h.iter().all(|e| e.ausschlussgrund.is_some())),
                };
                if client.plugin_kind == "main" {
                    mains.push(key.instance_id.clone());
                    // 🔑 **NAK-213 E7/R4:** der Master ist das FUEHRENDE Main,
                    // nicht „der letzte". Bis hierher stand hier
                    // `master = Some(profil)` fuer JEDEN `main`-Client in
                    // einer nach `instance_id` sortierten Schleife — der
                    // letzte gewann immer, und der erste verschwand spurlos
                    // (Gate-Befund G-D4). `fuehrendes_main` wird von
                    // `mitgliedschaft.rs` gepflegt und bei mehr als einem
                    // bestaetigten Main auf `None` gesetzt; dieses Ticket
                    // LIEST es und erfindet keinen zweiten Fuehrungsbegriff
                    // (§2.9 Nr. 9).
                    if fuehrendes_main.as_deref() == Some(key.instance_id.as_str()) {
                        master = Some(profil);
                    }
                } else {
                    kandidaten.push(profil);
                }
            }
            // Gibt es gar keinen `main`-Client, entsteht keine Aufnahme: die
            // Sitzung hat keinen Master und rechnet nicht (K-19). Das ist kein
            // Fehler, sondern der normale Zustand einer Sondenrunde ohne Gen.
            if mains.is_empty() {
                continue;
            }
            mains.sort();
            mains.dedup();
            // Duplikate eintragen: jede Quelle, die sich einen Mixerkanal mit
            // einer anderen teilt, zeigt auf die andere.
            //
            // 🔑 **NAK-213 E6:** die Schleife laeuft ueber den MASTER UND die
            // Kandidaten. Bis hierher wurde `parent` nur an Kandidaten
            // geschrieben; eine Sonde auf dem Masterkanal blieb unmarkiert.
            let mut master_und_kandidaten: Vec<&mut Quellprofil> = master
                .iter_mut()
                .chain(kandidaten.iter_mut())
                .collect();
            for (_, geteilt) in je_kanal.iter().filter(|(_, v)| v.len() > 1) {
                for quelle in master_und_kandidaten.iter_mut() {
                    if geteilt.contains(&quelle.quelle_id) {
                        quelle.parent = geteilt
                            .iter()
                            .find(|id| **id != quelle.quelle_id)
                            .cloned();
                    }
                }
            }
            let passage = stand
                .experimente
                .juengste_passage_im_projekt(&session.project_binding_id);
            aus.push((
                session.clone(),
                Aufnahme {
                    master,
                    kandidaten,
                    passage: passage.map(|p| Passagenfenster {
                        projekt_von: p.projekt_von,
                        projekt_bis: p.projekt_bis,
                        transport_epoch: p.transport_epoch,
                    }),
                    passage_id: passage.map(|p| p.passage_id.clone()),
                    mains,
                    intent,
                    metrics_version: super::vergleichbarkeit::METRICS_VERSION,
                    session_epoch: session.session_epoch.clone(),
                },
            ));
        }
        aus
    }

    /// Die Fensterfolge einer Quelle, **ohne** zurückgenommene Belege.
    ///
    /// ⚠️ Ein ausgeschlossener Beleg wird hier weggelassen und nicht mit
    /// einem Merker weitergereicht. Das ist die Hälfte von M-28: „eine ID,
    /// deren `evidence`-Zeile fehlt oder ausgeschlossen ist, macht die
    /// Behauptung unsichtbar" — sie darf gar nicht erst in eine Rechnung
    /// eingehen, aus der eine sichtbare Behauptung entsteht.
    fn fenster_aus_historie(
        historie: &std::collections::VecDeque<super::evidenz::Evidenzstand>,
    ) -> Vec<Evidenzfenster> {
        let mut aus: Vec<Evidenzfenster> = historie
            .iter()
            .filter(|e| e.ausschlussgrund.is_none())
            .map(|e| {
                let von = e.project_sample_start.unwrap_or(0);
                Evidenzfenster {
                    evidence_id: e.evidence_id.clone(),
                    empfangsfolge: e.empfangsfolge,
                    projekt_von: von,
                    projekt_bis: von.saturating_add(e.sample_count as i64),
                    transport_epoch: e.transport_epoch,
                    abdeckung: e.abdeckung,
                    p50_db: e.p50_db.clone(),
                    p50_gueltig: e.p50_gueltig.clone(),
                    onset: e.onset,
                    seitenanteil_db: e.seitenanteil_db,
                    hat_baender: e.hat_baender,
                    hat_verteilung: e.hat_verteilung,
                    hat_stereo: e.hat_stereo,
                }
            })
            .collect();
        // Die Reduktionsreihenfolge ist die ANKUNFTSREIHENFOLGE (M-25). Sie
        // ist die einzige, die über Quellen hinweg vergleichbar ist.
        aus.sort_by_key(|f| f.empfangsfolge);
        aus
    }

    // ═════════════════════════════════════════════════════════════════════
    // Eintragen, persistieren, zustellen
    // ═════════════════════════════════════════════════════════════════════

    /// Trägt die neuen Befunde ein. Rückgabe: ob sich etwas geändert hat.
    ///
    /// Ein unveränderter Stand erzeugt **keinen** Store-Event und **keinen**
    /// Push. Bei 1 bis 4 Hz Evidenz wäre das sonst ein Dauerstrom identischer
    /// Snapshots — dieselbe Regel, die `heartbeat` und `session_snapshot`
    /// bereits tragen.
    fn befunde_eintragen(&self, session: &SessionKey, befunde: Vec<CauseHypothesis>) -> bool {
        let neue: Vec<CauseHypothesis> = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            // 🔑 NR-03 (Nacharbeit 1, 07.09.2026): das Ergebnis wird UNTER
            // DEMSELBEN LOCK gegen den Stand geprueft, aus dem es entstanden
            // ist — VOR dem Einsetzen.
            //
            // `hypothesen_bilden` sammelt unter dem Lock, rechnet ausserhalb
            // und trug bis hierher bedingungslos ein. Das Interleaving
            // „Aufnahme mit Intent-Revision 1 gesammelt → waehrend der
            // Rechnung Revision 2 empfangen und die Befunde auf `stale`
            // gesetzt → altes Ergebnis eingetragen" ueberschrieb damit einen
            // bereits sichtbar veralteten Bestand mit einem READY-Befund der
            // ALTEN Revision. Dieselbe Naht traegt die zurueckgenommene
            // Evidenz: eine `evidence_id`, die zwischen Sammeln und Eintragen
            // ausgeschlossen wurde, waere wieder veroeffentlicht worden.
            //
            // Verworfen heisst NICHT verloren: die Sitzung bleibt dirty und
            // wird neu gerechnet. Ein halb uebernommenes Ergebnis gibt es
            // nicht — entweder alle Befunde oder keiner.
            if !Self::ergebnis_ist_noch_gueltig(&stand, session, &befunde) {
                stand.befunde_neu_bilden = true;
                return false;
            }
            let alt = stand.befunde.get(session);
            if alt.map(Vec::as_slice) == Some(befunde.as_slice()) {
                return false;
            }
            if befunde.is_empty() {
                stand.befunde.remove(session);
                Vec::new()
            } else {
                stand.befunde.insert(session.clone(), befunde.clone());
                befunde
            }
        };
        for befund in &neue {
            self.befund_persistieren(session, befund);
        }
        // 🔑 NR-04: „beim Eintragen eines Befunds" — die zweite der beiden
        // Stellen, an denen M-28 die Existenz gegen den Store haelt.
        self.befunde_gegen_store_haerten(session);
        true
    }

    /// Legt EINEN Befund als `event_type = "finding"` ab.
    ///
    /// Die Projektion `findings` existiert seit SONDE-011 (`writer.rs`:572)
    /// und hatte bis hier keinen Produzenten (§2.11 L3). Der Payload trägt die
    /// `finding_id` als Projektionsschlüssel — ohne sie weist der Writer den
    /// Event zurück.
    fn befund_persistieren(&self, session: &SessionKey, befund: &CauseHypothesis) {
        let Some(store) = self.store.as_ref() else {
            return;
        };
        let payload = Self::befund_json(befund);
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
        event.event_type = "finding".into();
        if store.append(vec![event]).is_err() {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
        }
    }

    /// Stoesst den Snapshot-Push an, damit Gen die Befunde sieht.
    ///
    /// Der Rueckweg laeuft ueber den BESTEHENDEN Outbox-/Snapshotpfad und
    /// nicht ueber eine eigene Familie (E-04) — genau wie `experimente` und
    /// `paare` in der Fassung 2. Deshalb steht hier nur die Markierung; das
    /// Zustellen selbst ist `flush_session`, mit seinem eigenen Shardlock und
    /// seiner eigenen Schuldkompaktierung.
    fn befunde_zustellen(&self, session: &SessionKey) {
        {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.dirty_sessions.insert(session.clone());
        }
        self.flush_session(session, None);
    }

    // ═════════════════════════════════════════════════════════════════════
    // Wireform
    // ═════════════════════════════════════════════════════════════════════

    /// Ein Befund in der Form von `$defs/session_finding`.
    ///
    /// ⚠️ `additionalProperties: false`. Jedes Feld hier steht im Vertrag,
    /// und die sechzehn Pflichtfelder sind vollständig — ein Objekt, dem
    /// eines fehlt, verwirft der Leser GANZ.
    pub(super) fn befund_json(b: &CauseHypothesis) -> Value {
        let mut objekt = serde_json::Map::new();
        objekt.insert("finding_id".into(), Value::String(b.finding_id.clone()));
        objekt.insert(
            "claim_class".into(),
            Value::String(b.claim_class.wire().into()),
        );
        objekt.insert(
            "ursachenklasse".into(),
            Value::String(b.ursachenklasse.wire().into()),
        );
        objekt.insert(
            "target_metric".into(),
            Value::String(b.target_metric.wire().into()),
        );
        objekt.insert(
            "candidate_source".into(),
            Value::String(b.candidate_source.clone()),
        );
        if let Some(stelle) = b.pre_post {
            objekt.insert("pre_post".into(), Value::String(stelle.into()));
        }
        // Abwesenheit heisst „die Stelle ist nicht benannt", nie `null`.
        if let Some(passage) = b.passage_id.as_ref() {
            objekt.insert("passage_id".into(), Value::String(passage.clone()));
        }
        objekt.insert(
            "band_hz".into(),
            serde_json::json!({ "von": b.band_hz.von, "bis": b.band_hz.bis }),
        );
        objekt.insert(
            "beobachtung".into(),
            serde_json::json!({
                "wert_db": zahl(b.beobachtung.wert_db, -200.0, 200.0),
                "gueltig": b.beobachtung.gueltig
            }),
        );
        objekt.insert(
            "rang".into(),
            serde_json::json!({
                "bandpassung": zahl(b.rang.bandpassung, 0.0, 1.0),
                "koinzidenz": zahl(b.rang.koinzidenz, 0.0, 1.0),
                "uplift": zahl(b.rang.uplift, 0.0, 1.0),
                "intent_relevanz": zahl(b.rang.intent_relevanz, 0.0, 1.0),
                "wiederholbarkeit": zahl(b.rang.wiederholbarkeit, 0.0, 1.0),
                "routingqualitaet": zahl(b.rang.routingqualitaet, 0.0, 1.0)
            }),
        );
        objekt.insert(
            "confidence".into(),
            serde_json::json!({
                "class": b.confidence.klasse.wire(),
                "score": zahl(b.confidence.score, 0.0, 1.0)
            }),
        );
        objekt.insert(
            "evidence_ids".into(),
            Value::Array(
                b.evidence_ids
                    .iter()
                    .map(|id| Value::String(id.clone()))
                    .collect(),
            ),
        );
        if !b.alternatives.is_empty() {
            objekt.insert(
                "alternatives".into(),
                Value::Array(
                    b.alternatives
                        .iter()
                        .map(|id| Value::String(id.clone()))
                        .collect(),
                ),
            );
        }
        if !b.ausschluesse.is_empty() {
            objekt.insert(
                "ausschluesse".into(),
                Value::Array(
                    b.ausschluesse
                        .iter()
                        .map(|a| {
                            serde_json::json!({
                                "candidate_source": a.candidate_source,
                                "grund": a.grund.wire()
                            })
                        })
                        .collect(),
                ),
            );
        }
        // 🔑 M-36/M-40: der Maskierungswert reist AM Befund, nicht daneben.
        //
        // Abwesenheit heisst „dieser Befund traegt keinen" — nie `null` und nie
        // ein Objekt mit 0 dB. Der Unterschied ist genau das, was die Anzeige
        // braucht, um „hier ist nichts gemessen" von „hier ist nichts" zu
        // trennen.
        if let Some(m) = b.maskierung.as_ref() {
            objekt.insert(
                "maskierung".into(),
                serde_json::json!({
                    "quelle_a": m.quelle_a,
                    "quelle_b": m.quelle_b,
                    "band_von": m.band_von,
                    "band_bis": m.band_bis,
                    "wert_db": zahl(m.wert_db, -200.0, 200.0),
                    "gueltig": m.gueltig,
                    "herabgesetzt": m.herabgesetzt
                }),
            );
        }
        objekt.insert(
            "next_test".into(),
            Value::String(b.next_test.wire().into()),
        );
        objekt.insert("zustand".into(), Value::String(b.zustand.wire().into()));
        objekt.insert(
            "intent_revision".into(),
            Value::Number(b.intent_revision.max(0).into()),
        );
        objekt.insert(
            "likely_cause".into(),
            Value::String(gekuerzt(&b.likely_cause)),
        );
        objekt.insert(
            "smallest_test".into(),
            Value::String(gekuerzt(&b.smallest_test)),
        );
        objekt.insert("listen_for".into(), Value::String(gekuerzt(&b.listen_for)));
        Value::Object(objekt)
    }

    // ═════════════════════════════════════════════════════════════════════
    // Rücknahme (M-24, M-28)
    // ═════════════════════════════════════════════════════════════════════

    /// Invalidiert **unter dem übergebenen Lock** jede Hypothese, deren
    /// Belege gerade zurückgenommen wurden.
    ///
    /// M-24 verlangt die Reihenfolge ausdrücklich unter EINEM Lock: Umfang
    /// auflösen → Evidenz-IDs markieren → abhängige Hypothesen terminal
    /// invalidieren. Läge die Invalidierung außerhalb, könnte zwischen
    /// Markierung und Invalidierung eine Neurechnung dieselbe Hypothese
    /// wieder aufbauen — und der User sähe eine Behauptung über Belege, die
    /// es nicht mehr gibt.
    ///
    /// Die Trennung folgt M-28 wörtlich: sind **alle** Belege fort, wird die
    /// Behauptung **unsichtbar**; sind es einige, wird sie **`STALE`**.
    ///
    /// „Deterministisch" heißt: dieselbe Rücknahme in anderer Reihenfolge
    /// trifft dieselbe Menge. Deshalb ist die Eingangsmenge ein `BTreeSet`
    /// und die Befundliste bleibt in ihrer Rangordnung.
    pub(super) fn befunde_invalidieren_locked(
        stand: &mut Stand,
        session: &SessionKey,
        zurueckgenommen: &BTreeSet<String>,
    ) -> usize {
        if zurueckgenommen.is_empty() {
            return 0;
        }
        let Some(befunde) = stand.befunde.get_mut(session) else {
            return 0;
        };
        let mut getroffen = 0usize;
        let mut unsichtbar: Vec<String> = Vec::new();
        for befund in befunde.iter_mut() {
            let vorher = befund.evidence_ids.len();
            befund
                .evidence_ids
                .retain(|id| !zurueckgenommen.contains(id));
            if befund.evidence_ids.len() == vorher {
                continue;
            }
            getroffen += 1;
            if befund.evidence_ids.is_empty() {
                unsichtbar.push(befund.finding_id.clone());
            } else {
                befund.zustand = Befundzustand::Stale;
            }
        }
        // Unsichtbar heisst wirklich fort — nicht „leer, aber noch da". Ein
        // Befund ohne Beleg, der weiter in der Liste steht, ist genau die
        // Behauptung ohne Beleg, die das Exit-Gate ausschliesst (M-28).
        befunde.retain(|b| !unsichtbar.contains(&b.finding_id));
        // Ein fuehrender Befund, dessen Alternative unsichtbar wurde, darf sie
        // nicht weiter nennen: eine `alternatives`-ID ohne Befund waere ein
        // Verweis ins Leere.
        for befund in befunde.iter_mut() {
            befund.alternatives.retain(|id| !unsichtbar.contains(id));
        }
        if befunde.is_empty() {
            stand.befunde.remove(session);
        }
        getroffen
    }

    /// Setzt jeden Befund mit älterer Intent-Revision auf `STALE` (§37.3,
    /// M-29). Rückgabe: wie viele getroffen wurden.
    ///
    /// **Ohne Nachrechnen.** §37.3 sagt das wörtlich: „Steigt sie, geht der
    /// Befund sichtbar in `stale`, **ohne dass Zahlen nachgerechnet
    /// werden**." Die Rangkomponenten, die Beobachtung und die Evidenz-IDs
    /// bleiben also stehen — nur der Zustand wechselt. Das ist kein
    /// Sparbetrieb, sondern die Zusage: der User soll sehen, dass **seine**
    /// Änderung den Befund entwertet hat, und nicht eine stillschweigend neue
    /// Zahl.
    ///
    /// Ein Befund, der bereits `stale` ist, bleibt es; eine zweite
    /// Intent-Änderung macht ihn nicht „mehr" stale.
    /// **NR-04 (Nacharbeit 1, 07.09.2026), M-28: die Belege der sichtbaren
    /// Befunde gegen den STORE haerten.**
    ///
    /// M-28 sagt woertlich zu, dass „existent" eine Pruefung gegen den Store
    /// ist, nicht gegen den Speicher. Bis zur Nacharbeit 1 filterte die
    /// Erzeugung nur den `ausschlussgrund` des Caches, und
    /// `session_snapshot_json` serialisierte, was dort stand — eine im Store
    /// geloeschte `evidence`-Zeile aenderte an der sichtbaren Behauptung
    /// nichts.
    ///
    /// Die Abfrage laeuft AUSSERHALB des Standlocks (eine SQLite-Leserunde
    /// haelt sonst den ganzen Sessiongraphen an); ihr Ergebnis wird unter dem
    /// Lock angewandt, ueber denselben Weg wie eine Ruecknahme: fehlt eine
    /// ID, faellt sie aus dem Befund; fehlen alle, verschwindet er.
    ///
    /// ⚠️ **Ohne Store gilt ausdruecklich die Speicherpruefung als
    /// Fallback.** Der Testkoordinator `mit_uhr` haelt seinen Bestand
    /// ausschliesslich fluechtig; dort gibt es keine zweite Wahrheit, von der
    /// er abweichen koennte. Dasselbe gilt fuer einen degradierten Store —
    /// eine gescheiterte Leseabfrage darf keine Behauptung loeschen, die noch
    /// belegt ist.
    pub(super) fn befunde_gegen_store_haerten(&self, session: &SessionKey) {
        let Some(store) = self.store.as_ref() else {
            return;
        };
        let ids: Vec<String> = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(befunde) = stand.befunde.get(session) else {
                return;
            };
            let mut alle: BTreeSet<String> = BTreeSet::new();
            for befund in befunde {
                alle.extend(befund.evidence_ids.iter().cloned());
            }
            alle.into_iter().collect()
        };
        if ids.is_empty() {
            return;
        }
        let Ok(vorhanden) = store.evidenz_belegt(&ids) else {
            // Fail-SAFE, nicht fail-closed: der Store hat nicht geantwortet.
            // Das ist keine Aussage darueber, ob die Zeilen existieren.
            return;
        };
        let fehlend: BTreeSet<String> = ids
            .into_iter()
            .filter(|id| !vorhanden.contains(id))
            .collect();
        if fehlend.is_empty() {
            return;
        }
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        Self::befunde_invalidieren_locked(&mut stand, session, &fehlend);
    }

    /// **WN-03 (Nacharbeit 2, 07.09.2026), M-28: dieselbe Existenzabfrage
    /// fuer eine GESPEICHERTE Projektion.**
    ///
    /// `befunde_gegen_store_haerten` haelt den fluechtigen Cache gegen den
    /// Store. Der Re-Subscribe sendet aber nicht den Cache, sondern den
    /// letzten committeten Projektionsschnitt aus `session_state_lesen` -
    /// fertige Bytes, die keine Haertung des Caches mehr erreicht. Genau
    /// dadurch blieb EP-04 fuer erneut abonnierende Clients offen (WP1-3):
    /// Befund bilden, Flush, `DELETE FROM evidence`, `subscribe_session` -
    /// und der gepushte Snapshot trug die Behauptung samt fehlender IDs.
    ///
    /// Die Regel ist dieselbe wie im Speicher (`befunde_invalidieren_locked`):
    /// fehlt eine ID, faellt sie aus dem Befund; fehlen alle, faellt der
    /// Befund aus den gesendeten `findings`; ein Teilverlust macht ihn
    /// `stale`. Ein unsichtbar gewordener Befund verschwindet auch aus den
    /// `alternatives` der uebrigen - eine ID ohne Befund waere ein Verweis
    /// ins Leere.
    ///
    /// 🔑 **Fail-SAFE, nicht fail-closed** (wie NR-04): ohne Store, bei
    /// degradiertem Store und bei einer gescheiterten Leseabfrage bleiben die
    /// Bytes unveraendert. Eine Abfrage, die nicht geantwortet hat, ist keine
    /// Aussage darueber, ob die Zeile existiert.
    pub(super) fn projektion_gegen_store_haerten(&self, payload: Vec<u8>) -> Vec<u8> {
        let Some(store) = self.store.as_ref() else {
            return payload;
        };
        if self.store_degradiert() {
            return payload;
        }
        let Ok(mut wert) = serde_json::from_slice::<Value>(&payload) else {
            return payload;
        };
        let Some(findings) = wert
            .get("findings")
            .and_then(Value::as_array)
            .filter(|f| !f.is_empty())
            .cloned()
        else {
            return payload;
        };
        // Alle referenzierten IDs in EINER Abfrage - dieselbe Zusage, die
        // `evidenz_belegt` fuer den Cache haelt.
        let mut alle: BTreeSet<String> = BTreeSet::new();
        for befund in &findings {
            for id in befund
                .get("evidence_ids")
                .and_then(Value::as_array)
                .into_iter()
                .flatten()
                .filter_map(Value::as_str)
            {
                alle.insert(id.to_owned());
            }
        }
        let ids: Vec<String> = alle.into_iter().collect();
        if ids.is_empty() {
            return payload;
        }
        let Ok(vorhanden) = store.evidenz_belegt(&ids) else {
            return payload;
        };
        if ids.iter().all(|id| vorhanden.contains(id)) {
            // Der Normalfall: nichts fehlt, die Bytes bleiben, wie sie
            // committet wurden.
            return payload;
        }
        let mut behalten: Vec<Value> = Vec::new();
        let mut unsichtbar: Vec<String> = Vec::new();
        for mut befund in findings {
            let eigene: Vec<String> = befund
                .get("evidence_ids")
                .and_then(Value::as_array)
                .into_iter()
                .flatten()
                .filter_map(Value::as_str)
                .map(str::to_owned)
                .collect();
            let da: Vec<String> = eigene
                .iter()
                .filter(|id| vorhanden.contains(*id))
                .cloned()
                .collect();
            if da.is_empty() {
                if let Some(id) = befund.get("finding_id").and_then(Value::as_str) {
                    unsichtbar.push(id.to_owned());
                }
                continue;
            }
            if da.len() != eigene.len() {
                let Some(objekt) = befund.as_object_mut() else {
                    continue;
                };
                objekt.insert(
                    "evidence_ids".into(),
                    Value::Array(da.into_iter().map(Value::String).collect()),
                );
                objekt.insert(
                    "zustand".into(),
                    Value::String(Befundzustand::Stale.wire().into()),
                );
            }
            behalten.push(befund);
        }
        for befund in behalten.iter_mut() {
            let Some(objekt) = befund.as_object_mut() else {
                continue;
            };
            let Some(Value::Array(alternativen)) = objekt.get_mut("alternatives") else {
                continue;
            };
            alternativen.retain(|a| match a.as_str() {
                Some(id) => !unsichtbar.iter().any(|weg| weg == id),
                None => true,
            });
            if alternativen.is_empty() {
                objekt.remove("alternatives");
            }
        }
        let Some(objekt) = wert.as_object_mut() else {
            return payload;
        };
        if behalten.is_empty() {
            // Der Vertrag kennt kein leeres Feld: `sicht.rs` setzt es nur,
            // wenn es Befunde gibt.
            objekt.remove("findings");
        } else {
            objekt.insert("findings".into(), Value::Array(behalten));
        }
        serde_json::to_vec(&wert).unwrap_or(payload)
    }

    /// **NR-03: traegt der Stand das Ergebnis noch, das aus ihm entstanden
    /// ist?**
    ///
    /// Zwei Fragen, beide unter dem Lock des Aufrufers:
    ///
    /// (a) **Generation.** Jeder Befund traegt die Bestandsgeneration, unter
    ///     der er gerechnet wurde. Weicht sie von der aktuellen ab, ist das
    ///     Ergebnis veraltet: es entstand unter einer anderen Absicht (M-10).
    ///     🔑 **E-14 / WN-02 (Nacharbeit 2):** hier stand die
    ///     Sender-Revision. Trifft ein verspaetetes Teilupdate WAEHREND der
    ///     Rechnung ein, stimmt sie weiterhin ueberein, und das veraltete
    ///     Ergebnis wurde eingetragen (WP1-2, zweiter Abschnitt).
    ///
    /// (a, Verlauf) Jeder Befund traegt die Intent-Revision, unter der
    ///     er gerechnet wurde. Weicht sie von der aktuellen Sitzungsrevision
    ///     ab, ist das Ergebnis veraltet — es wurde unter einer anderen
    ///     Absicht gerechnet (M-10).
    ///
    /// (b) **Evidenz.** Jede referenzierte `evidence_id` steht in der
    ///     aktuellen Historie und ist nicht ausgeschlossen. Eine waehrend der
    ///     Rechnung zurueckgenommene ID darf nie wieder sichtbar werden
    ///     (M-24, M-28).
    ///
    /// Ein LEERES Ergebnis ist immer gueltig: es behauptet nichts, und es
    /// ist der Weg, auf dem eine Ruecknahme ihre Befunde raeumt.
    fn ergebnis_ist_noch_gueltig(
        stand: &Stand,
        session: &SessionKey,
        befunde: &[CauseHypothesis],
    ) -> bool {
        if befunde.is_empty() {
            return true;
        }
        let aktuelle_generation = stand
            .intent
            .get(session)
            .map(|b| b.generation)
            .unwrap_or_default();
        if befunde
            .iter()
            .any(|b| b.intent_generation != aktuelle_generation)
        {
            return false;
        }
        // Die gueltigen Belege DIESER Sitzung, aus derselben Historie, aus
        // der `aufnahmen_sammeln` sie genommen hat.
        let mut gueltig: BTreeSet<&str> = BTreeSet::new();
        for (key, historie) in stand.evidenz.iter() {
            if key.session() != *session {
                continue;
            }
            for eintrag in historie.iter() {
                if eintrag.ausschlussgrund.is_none() {
                    gueltig.insert(eintrag.evidence_id.as_str());
                }
            }
        }
        befunde
            .iter()
            .flat_map(|b| b.evidence_ids.iter())
            .all(|id| gueltig.contains(id.as_str()))
    }

    ///
    /// 🔑 **E-14 / WN-02 (Nacharbeit 2, 07.09.2026): gemessen wird die
    /// BESTANDSGENERATION, nicht die Sender-Revision.** Ein verspaetetes
    /// Teilupdate mit hartem Veto hebt die Sitzungsrevision nicht; es
    /// aendert aber den Bestand, unter dem gerechnet wurde. An der
    /// Sender-Zahl gemessen blieb der READY-Befund handelbar (WP1-2).
    pub(super) fn befunde_veralten_locked(
        stand: &mut Stand,
        session: &SessionKey,
        neue_generation: i64,
    ) -> usize {
        let Some(befunde) = stand.befunde.get_mut(session) else {
            return 0;
        };
        let mut getroffen = 0usize;
        for befund in befunde.iter_mut() {
            if befund.intent_generation >= neue_generation
                || befund.zustand == Befundzustand::Stale
            {
                continue;
            }
            befund.zustand = Befundzustand::Stale;
            getroffen += 1;
        }
        getroffen
    }

    // ═════════════════════════════════════════════════════════════════════
    // Sichten für Beine
    // ═════════════════════════════════════════════════════════════════════

    /// Die Befunde einer Sitzung, als Kopie.
    pub fn befunde_sicht(&self, project_binding_id: &str, session_epoch: &str) -> Vec<CauseHypothesis> {
        let session = SessionKey {
            project_binding_id: project_binding_id.into(),
            session_epoch: session_epoch.into(),
        };
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand.befunde.get(&session).cloned().unwrap_or_default()
    }
}

/// Klemmt eine Zahl in die Vertragsgrenzen und macht aus Nicht-Endlichem eine
/// **0**.
///
/// NaN-Ehrlichkeit auf dem Draht: `serde_json` schreibt für NaN und ±inf ein
/// `null`, und `null` ist an keiner dieser Stellen ein gültiger Wert — der
/// Leser verwürfe den ganzen Snapshot. Der Ort, an dem eine nicht gerechnete
/// Zahl ehrlich wird, ist ihr Gültigkeitsbit (`beobachtung.gueltig`), nicht
/// eine stille Sonderform der Zahl selbst.
/// 🔑 **WN3-02 (Nacharbeit 3, 07.09.2026): der Deckel sitzt im Vertrag, nicht
/// hier.** Bis zur Runde 3 stand daneben ein `auf_wirestellen`, das NUMERISCH
/// rundete (`(x * 10^(14-exp)).round() / 10^(14-exp)`). Der Faktor ist ab
/// `exponent <= -9` selbst ungenau: `2.7e-11` kam mit siebzehn signifikanten
/// Stellen heraus, und von 100 000 Werten des Bandes `1e-15..1e-9` wurden
/// 34 535 vertragswidrig — genau die Groessenordnung, die `bandpassung`,
/// `koinzidenz` und `uplift` erzeugen. Der Deckel gilt seither der
/// SERIALISIERTEN Dezimaldarstellung und liegt neben dem Riegel, der sie
/// beurteilt (`crate::vertrag::wire_zahl`, Spiegel von
/// `nakama::wire::wireZahl`).
fn zahl(wert: f64, min: f64, max: f64) -> f64 {
    if !wert.is_finite() {
        return 0.0;
    }
    crate::vertrag::wire_zahl(wert.clamp(min, max))
}

/// Kürzt einen Anzeigetext auf die Vertragslänge (200 Zeichen), in
/// **Codepoints** und nicht in Bytes — dieselbe Zählung, die das Schema
/// meint, und dieselbe, die `descriptor_vertrag_erfuellt` für `label` benutzt.
fn gekuerzt(text: &str) -> String {
    let mut aus: String = text.chars().take(200).collect();
    if aus.is_empty() {
        // `minLength: 1`. Ein leerer Text wäre vertragswidrig, und ein
        // stillschweigend weggelassenes Pflichtfeld ebenso.
        aus.push('—');
    }
    aus
}
