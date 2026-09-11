//! Der PRODUKTPFAD der Proposal-Policy (SONDE-014 Etappe F).
//!
//! ── DER AUSLÖSER ──────────────────────────────────────────────────────────
//!
//! Ein Vorschlag entsteht **mit** seinem Befund und sonst nie. Das ist keine
//! Bequemlichkeit: §42.1 bindet jedes Proposal an `evidence_ids` und
//! `finding_id`, und ein Vorschlag ohne Befund hätte weder das eine noch das
//! andere. Deshalb hängt `vorschlaege_bilden` direkt hinter
//! `befunde_eintragen` — derselbe Anlass, dieselbe Reihenfolge.
//!
//! ── WIE DER VORSCHLAG ZU GEN KOMMT ────────────────────────────────────────
//!
//! Über `draft_offer`, als eigene v3-Familie über den bestehenden Pushpfad —
//! genau wie `evidence_invalidate`. Das ist der einzige Weg, den die
//! **Fassung 3** hergibt: `session_snapshot` hat kein `proposals`-Feld, und
//! ein weiteres Feld wäre ein zweiter Fassungsschritt (§5.3 R4 schließt das
//! aus).
//!
//! Zugestellt wird **nur**, was M-52 zulässt: ein veralteter, ungültiger oder
//! capability-loser Vorschlag wird gar nicht erst als Angebot gebildet. Die
//! Sperre steht in `proposal::darf_draft_offer` und wird hier angewendet —
//! nicht in der Anzeige.
//!
//! ── EIN EINGRIFF JE SCHRITT (M-57, §42.3) ─────────────────────────────────
//!
//! „Höchstens ein musikalischer Eingriff pro Assistentenschritt." Der Deckel
//! ist **strukturell**: zugestellt wird höchstens **ein** `draft_offer` je
//! Sitzung, und zwar das des führenden Befunds. Ein Zähler, der auch 2 tragen
//! könnte, wäre ein anderes Versprechen.

use super::proposal::{darf_draft_offer, proposal, Proposal, Proposallage, Stopbedingung};
use super::*;

impl Coordinator {
    /// Bildet die Vorschläge einer Sitzung neu. Rückgabe: ob sich etwas
    /// geändert hat.
    pub(super) fn vorschlaege_bilden(&self, session: &SessionKey) -> bool {
        let (befunde, lage) = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(befunde) = stand.befunde.get(session).cloned() else {
                // Keine Befunde, keine Vorschläge. Ein alter Vorschlag ohne
                // Befund wäre eine Empfehlung ohne Beleg.
                return false;
            };
            (befunde, Self::proposallage_locked(&stand, session))
        };
        let paare = paare_bilden(&befunde, &lage);
        let neue: Vec<Proposal> = paare.iter().map(|(_, v)| v.clone()).collect();
        let geaendert = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            if stand.vorschlaege.get(session).map(Vec::as_slice) == Some(neue.as_slice()) {
                return false;
            }
            if neue.is_empty() {
                stand.vorschlaege.remove(session);
            } else {
                stand.vorschlaege.insert(session.clone(), neue.clone());
            }
            true
        };
        // M-57/§42.3: HÖCHSTENS EIN Angebot je Sitzung — das des führenden
        // Befunds. Der Deckel ist strukturell und kein Zähler.
        //
        // 🔑 SONDE-014 Etappe I (M-73/M-74): das Angebot wird BESTIMMT, bevor
        // irgendetwas abgelegt wird. Grund ist die Outbox: eine Zustellschuld
        // entsteht nur ZUSAMMEN mit ihrem Ereignis (`snapshot_ziele` am
        // Append, wie `flush_session` und `evidence_invalidate`). Wer erst
        // ablegt und dann zustellt, hat für einen fehlgeschlagenen Push
        // nichts mehr, woran er ihn nachholen könnte — und genau das war der
        // Zustand bis hierher: der Rückgabewert von `snapshot_schreiben`
        // ging in ein `let _`, und ein nicht angenommenes Angebot war fort.
        let angebot: Option<Proposal> = paare
            .iter()
            .find(|(b, v)| darf_draft_offer(b, v, &lage))
            .map(|(_, v)| v.clone());

        for vorschlag in &neue {
            let ziele = match &angebot {
                Some(a) if a.proposal_id == vorschlag.proposal_id => {
                    self.draft_offer_ziele(session, vorschlag)
                }
                _ => Vec::new(),
            };
            let ord = self.vorschlag_persistieren(session, vorschlag, &ziele);
            if !ziele.is_empty() {
                self.draft_offer_zustellen(vorschlag, &ziele, ord);
            }
        }
        geaendert
    }

    /// Die Lage, aus der die Vorschläge dieser Sitzung entstehen.
    ///
    /// ⚠️ `capability_vorhanden` ist in P5 **immer falsch**. Der aktive Kern
    /// entsteht erst in P6 (`SONDE-015`), und ein Broker, der hier optimistisch
    /// `true` schriebe, erzeugte Angebote, die keine Probe ausführen kann. Das
    /// Feld steht trotzdem da, weil M-52 „Capability fehlt" als eigenen Grund
    /// führt — und ein Grund, den niemand setzen kann, ist keiner.
    fn proposallage_locked(stand: &Stand, session: &SessionKey) -> Proposallage {
        Proposallage {
            ziel_instanz: stand
                .sessions
                .get(session)
                .and_then(|s| s.fuehrendes_main.clone())
                .unwrap_or_default(),
            base_revision: 0,
            intent: stand.intent.get(session).cloned(),
            capability_vorhanden: false,
            // M-50: die Guardrails, die der Broker heute WIRKLICH messen kann.
            //
            // `guardrail_loudness_db` und `guardrail_peak_db` sind heute hart
            // `None` (NAK-204), `seitenanteil_db` erreicht den Broker
            // strukturell nie (NAK-193). Sie stehen deshalb NICHT in dieser
            // Liste — und ein Vorschlag, dessen Stopbedingung auf einen von
            // ihnen zeigt, ist nicht handelbar. Das ist die ehrliche
            // Verkettung, die M-50 verlangt: „nicht gemessen", nie
            // „unverändert".
            messbare_guardrails: vec![
                Stopbedingung::GuardrailAbdeckung,
                Stopbedingung::GuardrailGeschuetzt,
                Stopbedingung::KeineWiederholbareMasteraenderung,
            ],
            usergrenze_db: None,
            session_epoch: session.session_epoch.clone(),
            metrics_version: super::vergleichbarkeit::METRICS_VERSION,
        }
    }

    /// Legt EINEN Vorschlag als `event_type = "proposal"` ab.
    ///
    /// Die Projektion `proposals` existiert seit SONDE-011 (`writer.rs`:573)
    /// und hatte bis hier keinen Produzenten (§2.11 L5).
    ///
    /// `ziele` sind die Zustellziele des Angebots (leer: dieser Vorschlag
    /// wird nicht zugestellt). Sie reisen als `snapshot_ziele` MIT dem
    /// Append — so entsteht die Outbox-Schuld im selben Commit wie das
    /// Ereignis, und ein Push, der nicht ankommt, bleibt nachholbar
    /// (M-74). Rückgabe ist die `event_ord`, gegen die kompaktiert wird.
    fn vorschlag_persistieren(
        &self,
        session: &SessionKey,
        vorschlag: &Proposal,
        ziele: &[SnapshotZiel],
    ) -> Option<i64> {
        let store = self.store.as_ref()?;
        let payload_jcs =
            serde_json_canonicalizer::to_vec(&Self::proposal_json(vorschlag)).ok()?;
        let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
        let mut event = StoreEvent::session_snapshot(
            &session.project_binding_id,
            &session.session_epoch,
            &self.broker_epoch,
            sequence.min(i64::MAX as u64) as i64,
            payload_jcs,
        );
        event.event_type = "proposal".into();
        event.snapshot_ziele = ziele.to_vec();
        match store.append(vec![event]) {
            Ok(ausgaenge) => ausgaenge.first().map(|a| a.event_ord()),
            Err(_) => {
                let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
                stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
                None
            }
        }
    }

    /// Die Zustellziele EINES Angebots: die Abonnenten genau dieser Sitzung.
    ///
    /// 🔑 Der `object_key` ist `proposal:<proposal_id>` — der Wortlaut aus
    /// **E-09/M-73**, und seit Etappe I dieselbe Zeichenkette wie auf der
    /// C++-Seite (`IpcTestMain.cpp`). Zwei Schreibweisen desselben
    /// Schlüssels wären zwei Wahrheiten über dieselbe Koaleszierung (M-77).
    /// Ein neuer Entwurf DESSELBEN Vorschlags darf den älteren verdrängen,
    /// ein fremder nie.
    fn draft_offer_ziele(&self, session: &SessionKey, vorschlag: &Proposal) -> Vec<SnapshotZiel> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand
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
                object_key: format!("proposal:{}", vorschlag.proposal_id),
            })
            .collect()
    }

    /// Die WIRE-Nachricht eines Angebots — an EINER Stelle.
    ///
    /// 🔑 Etappe I: Zustellung und Nachspielen aus der Outbox bilden dieselbe
    /// Nachricht aus derselben Funktion. Zwei Bauplätze für dieselbe Form
    /// wären zwei Wahrheiten, und die Wiederholung nach einem
    /// fehlgeschlagenen Push hieße dann „fast dasselbe".
    ///
    /// ⚠️ **`kopf.ziel` ist die vollständige Adresse des EMPFÄNGERS.** Bis
    /// Etappe I standen dort `logon_sid` und `runtime_nonce` als leere
    /// Zeichenketten — der Vertrag verlangt an beiden Stellen `$defs/sid`
    /// beziehungsweise `$defs/hex32`, und `adresse` ist
    /// `additionalProperties: false` mit fünf Pflichtfeldern. Die Nachricht
    /// war damit schemaungültig; kein Leser hätte sie angenommen, und weil
    /// der Zustellweg sie nie gegen den Vertrag hielt, fiel es nirgends auf.
    /// Welche Quelle der Eingriff beträfe, sagt `proposal.target` — dafür ist
    /// der Kopf nicht da.
    pub(super) fn draft_offer_nachricht(empfaenger: &Adresse, proposal: &Value) -> Value {
        serde_json::json!({
            "type": "draft_offer",
            "kopf": {
                "command_id": proposal.get("proposal_id").cloned().unwrap_or(Value::Null),
                "ziel": empfaenger,
                "base_revision": proposal.get("base_revision").cloned().unwrap_or(Value::Null),
                "ttl_ms": DRAFT_OFFER_TTL_MS,
                "schema_major": 3,
                "schema_minor": JSON_SCHEMA_MINOR_AKTIV
            },
            "proposal": proposal
        })
    }

    /// Link und vollständige Adresse des Abonnenten eines Zustellziels.
    ///
    /// `SnapshotZiel` trägt nur Projekt, Sitzung und Instanz — die
    /// Steueradresse braucht zusätzlich `logon_sid` und `runtime_nonce`.
    /// Beide stehen in der Subscription und nirgendwo sonst; sie zu erfinden
    /// wäre eine zweite Adressquelle.
    pub(super) fn abonnent_des_ziels(&self, ziel: &SnapshotZiel) -> Option<(String, Adresse)> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand
            .subscriptions
            .iter()
            .find(|(_, sub)| {
                sub.adresse.instance_id == ziel.instance_id
                    && sub.session_epoch == ziel.session_epoch
                    && sub.adresse.project_binding_id == ziel.project_binding_id
            })
            .map(|(link_id, sub)| (link_id.clone(), sub.adresse.clone()))
    }

    /// Stellt **ein** `draft_offer` zu — über denselben Weg wie
    /// `evidence_invalidate`.
    ///
    /// 🔑 Etappe I (M-73, Prüfliste A „Rückgabewerte und Zähler einer Politik
    /// werden ausgewertet"): der Rückgabewert von `snapshot_schreiben` wird
    /// GELESEN. Nur ein wirklich geschriebenes Angebot kompaktiert seine
    /// Schuld; was nicht ankam, bleibt in der Outbox stehen und wird von der
    /// nächsten Befundbildung oder vom nächsten Subscribe nachgespielt. Der
    /// Zähler `draft_offers` zählt seitdem ZUSTELLUNGEN, nicht Versuche —
    /// ein Zähler, der auch das Verworfene mitzählt, sagt nichts.
    fn draft_offer_zustellen(
        &self,
        vorschlag: &Proposal,
        ziele: &[SnapshotZiel],
        event_ord: Option<i64>,
    ) {
        let proposal = Self::proposal_json(vorschlag);
        let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
        let mut zugestellt = 0u64;
        let mut offen = 0u64;
        for ziel in ziele {
            let Some((link_id, empfaenger)) = self.abonnent_des_ziels(ziel) else {
                offen += 1;
                continue;
            };
            let nachricht = Self::draft_offer_nachricht(&empfaenger, &proposal);
            let Ok(payload) = serde_json::to_vec(&nachricht) else {
                offen += 1;
                continue;
            };
            // 🔑 NAK-246 D5/D9: Schluessel `proposal:<id>` (aus dem Ziel) und
            // Marke. Ohne Ablage - kein Store, Append verweigert - gibt es kein
            // Ordinal; der Empfaenger gibt dem Angebot dann das Hochwasser
            // seines Schluessels.
            let marke = event_ord.unwrap_or(super::flush::MARKE_OHNE_ORDINAL);
            let geschrieben = self.push_ziel_noch_gueltig(&link_id, ziel)
                && push.as_ref().is_some_and(|p| {
                    p.snapshot_schreiben(&link_id, &ziel.object_key, marke, &payload)
                });
            if geschrieben {
                zugestellt += 1;
                if let (Some(store), Some(ord)) = (self.store.as_ref(), event_ord) {
                    let _ = store.snapshot_schuld_kompaktieren(ziel.clone(), ord);
                }
            } else {
                offen += 1;
            }
        }
        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand.draft_offers = stand.draft_offers.saturating_add(zugestellt);
        stand.draft_offer_schuld = stand.draft_offer_schuld.saturating_add(offen);
    }

    /// Ein Vorschlag in der Form von `$defs/proposal`.
    ///
    /// ⚠️ `additionalProperties: false` mit **fünfzehn** Pflichtfeldern. Ein
    /// Objekt, dem eines fehlt, verwirft der Leser GANZ.
    pub(super) fn proposal_json(p: &Proposal) -> Value {
        let mut objekt = serde_json::Map::new();
        objekt.insert("proposal_id".into(), Value::String(p.proposal_id.clone()));
        objekt.insert("proposal_schema".into(), Value::from(p.proposal_schema));
        objekt.insert("target".into(), Value::String(p.target.clone()));
        objekt.insert("base_revision".into(), Value::from(p.base_revision));
        if let Some(passage) = p.passage_id.as_ref() {
            objekt.insert("passage_id".into(), Value::String(passage.clone()));
        }
        if let Some(befund) = p.finding_id.as_ref() {
            objekt.insert("finding_id".into(), Value::String(befund.clone()));
        }
        objekt.insert("action".into(), Value::String(p.action.wire().into()));
        objekt.insert("parameters".into(), Self::parameter_json(p));
        objekt.insert("allowed_bounds".into(), Self::grenzen_json(p));
        objekt.insert(
            "evidence_ids".into(),
            Value::Array(
                p.evidence_ids
                    .iter()
                    .map(|id| Value::String(id.clone()))
                    .collect(),
            ),
        );
        objekt.insert(
            "expected_effect".into(),
            Value::String(p.expected_effect.wire().into()),
        );
        objekt.insert(
            "protected_traits".into(),
            Value::Array(
                p.protected_traits
                    .iter()
                    .map(|s| {
                        let mut o = serde_json::Map::new();
                        o.insert("quelle_id".into(), Value::String(s.quelle_id.clone()));
                        o.insert("eigenschaft".into(), Value::String(s.eigenschaft.clone()));
                        if let Some((von, bis)) = s.band {
                            o.insert(
                                "band".into(),
                                serde_json::json!({ "von": von, "bis": bis }),
                            );
                        }
                        Value::Object(o)
                    })
                    .collect(),
            ),
        );
        objekt.insert("listen_for".into(), Value::String(p.listen_for.clone()));
        objekt.insert(
            "stop_if".into(),
            Value::Array(
                p.stop_if
                    .iter()
                    .map(|s| Value::String(s.wire().into()))
                    .collect(),
            ),
        );
        objekt.insert("execution".into(), Value::String(p.execution.wire().into()));
        objekt.insert(
            "confidence".into(),
            serde_json::json!({
                "class": p.confidence.klasse.wire(),
                "score": zahl(p.confidence.score, 0.0, 1.0)
            }),
        );
        objekt.insert("revert".into(), Value::String(p.revert.wire().into()));
        objekt.insert(
            "intent_revision".into(),
            Value::Number(p.intent_revision.max(0).into()),
        );
        objekt.insert("generatorversion".into(), Value::from(p.generatorversion));
        Value::Object(objekt)
    }

    fn parameter_json(p: &Proposal) -> Value {
        let mut o = serde_json::Map::new();
        if let Some(f) = p.parameters.frequency_hz {
            o.insert("frequency_hz".into(), zahl_wert(f, 1.0, 24000.0));
        }
        if let Some(q) = p.parameters.q {
            o.insert("q".into(), zahl_wert(q, 0.1, 20.0));
        }
        if let Some(g) = p.parameters.max_gain_db {
            o.insert("max_gain_db".into(), zahl_wert(g, -24.0, 24.0));
        }
        Value::Object(o)
    }

    fn grenzen_json(p: &Proposal) -> Value {
        let mut o = serde_json::Map::new();
        let bereich = |b: super::proposal::Wertebereich, min: f64, max: f64| {
            serde_json::json!({
                "von": zahl(b.von, min, max),
                "bis": zahl(b.bis, min, max)
            })
        };
        if let Some(b) = p.allowed_bounds.frequency_hz {
            o.insert("frequency_hz".into(), bereich(b, -1_000_000.0, 1_000_000.0));
        }
        if let Some(b) = p.allowed_bounds.q {
            o.insert("q".into(), bereich(b, -1_000_000.0, 1_000_000.0));
        }
        if let Some(b) = p.allowed_bounds.gain_db {
            o.insert("gain_db".into(), bereich(b, -1_000_000.0, 1_000_000.0));
        }
        Value::Object(o)
    }

    /// Die Vorschläge einer Sitzung, als Kopie.
    pub fn vorschlaege_sicht(&self, project_binding_id: &str, session_epoch: &str) -> Vec<Proposal> {
        let session = SessionKey {
            project_binding_id: project_binding_id.into(),
            session_epoch: session_epoch.into(),
        };
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand.vorschlaege.get(&session).cloned().unwrap_or_default()
    }

    /// Die Wireform eines Vorschlags — fuer Beine.
    ///
    /// Der Produktpfad ruft `proposal_json` intern; ein Bein soll dieselbe
    /// Form messen und sie nicht nachbauen.
    pub fn proposal_json_fuer_test(p: &Proposal) -> Value {
        Self::proposal_json(p)
    }

    /// Wie viele `draft_offer` der Broker zugestellt hat.
    pub fn draft_offers_zaehler(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .draft_offers
    }

    /// Wie oft ein Angebot seinen Abonnenten NICHT erreicht hat (M-73).
    ///
    /// Der Gegenzaehler zu `draft_offers_zaehler`. Ohne ihn waere „zugestellt"
    /// von „eingereiht" nicht zu unterscheiden — genau die Verwechslung, die
    /// `tools/dirigent/pruefliste.md` Abschnitt A meint, wenn sie verlangt,
    /// dass der Rueckgabewert einer Politik ausgewertet wird.
    pub fn draft_offer_schuld_zaehler(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .draft_offer_schuld
    }
}

/// Die `(Befund, Vorschlag)`-Paare einer Sitzung — die eine Zusammenführung.
///
/// 🔑 WN-04 (Nacharbeit 2, 07.09.2026): `filter_map`, nicht `map`.
///
/// Ohne benannte Passage liefert der Erzeuger `None`, und dann gibt es hier
/// auch keinen Eintrag: kein `stand.vorschlaege`, kein
/// `vorschlag_persistieren`, kein `draft_offer`. Der Befund bleibt sichtbar —
/// er behauptet etwas über die Ursache —, aber ein Vorschlag, der nicht sagen
/// kann, WO er gilt, entsteht nicht. Seit NAK-214 fällt hier auch, wem eines
/// der sechs Gate-Felder fehlt (R1, der Schlussriegel in `proposal()`).
///
/// Die Paare bleiben zusammen: `angebot` in `vorschlaege_bilden` braucht den
/// Befund ZU seinem Vorschlag, und ein `zip` über zwei unterschiedlich lange
/// Listen hängte den falschen an.
///
/// 🔑 **NAK-214 (08.09.2026): warum das eine eigene Funktion ist.** `paare`
/// war eine lokale Variable von `vorschlaege_bilden` und damit an keinem
/// Bestand beobachtbar: `neue` übernimmt nur die Vorschlagshälfte, und
/// `angebot` — die einzige Auswertung, die BEIDE Hälften liest — ist in P5
/// immer `None`, weil `capability_vorhanden` fest falsch ist. Die Zusage
/// „die Paare bleiben zusammen" war deshalb seit SONDE-014 Nacharbeit 2
/// zugesagt und nicht gemessen. Die Funktion verschiebt die bestehenden
/// Zeilen unverändert und bleibt modulprivat; sie gibt der Zusage einen
/// Rückgabewert, den ein Test lesen kann.
fn paare_bilden(
    befunde: &[CauseHypothesis],
    lage: &Proposallage,
) -> Vec<(CauseHypothesis, Proposal)> {
    befunde
        .iter()
        .filter_map(|b| proposal(b, lage).map(|v| (b.clone(), v)))
        .collect()
}

/// Wie lange ein Angebot gilt (§33.3 `steuerkopf.ttl_ms`).
///
/// Zehn Sekunden: lang genug, dass ein User den Vorschlag liest, kurz genug,
/// dass er nicht eine Passage später noch gilt. §78 nennt keine Zahl; das ist
/// deshalb eine **Produktgrenze** und keine Messung.
pub const DRAFT_OFFER_TTL_MS: u64 = 10_000;

/// Klemmt eine Zahl in die Vertragsgrenzen, macht aus Nicht-Endlichem eine
/// **0** und deckelt ihre SERIALISIERTE Dezimaldarstellung.
///
/// 🔑 **WN3-02 (Nacharbeit 3, 07.09.2026): auch das Proposal reist im
/// Snapshot.** Der Deckel fehlte hier ganz — die Nacharbeit 2 hatte ihn nur
/// am Befund. `confidence.score` entsteht aus derselben Bruchrechnung wie die
/// Rangkomponenten, und `frequency_hz`, `q` und `max_gain_db` kommen aus
/// `bandmitte_hz` beziehungsweise der Kostenrechnung; jeder von ihnen kann
/// siebzehn signifikante Stellen tragen und den `session_snapshot` fuer den
/// eigenen Leser unlesbar machen. Eine Grenze, zwei Regeln war die Ursache
/// des Befunds; jetzt ist es dieselbe Funktion wie am Befund
/// (`crate::vertrag::wire_zahl`).
fn zahl(wert: f64, min: f64, max: f64) -> f64 {
    if !wert.is_finite() {
        return 0.0;
    }
    crate::vertrag::wire_zahl(wert.clamp(min, max))
}

fn zahl_wert(wert: f64, min: f64, max: f64) -> Value {
    Value::from(zahl(wert, min, max))
}

// ═════════════════════════════════════════════════════════════════════════
// NAK-214 V-12 (a) — die Paarbildung, von innen gemessen
// ═════════════════════════════════════════════════════════════════════════
//
// Beide Fälle liegen HIER und nicht in `broker/tests/`, weil der Zustand,
// den sie brauchen, von außen strukturell unerreichbar ist: `listen_for` ist
// das einzige BEFUNDEIGENE Gate-Feld — `passage_id` kommt aus der Aufnahme
// und ist für alle Befunde einer Sitzung gleich, `target` aus der EINEN
// `Proposallage` je Sitzung —, am Produktpfad wird es nie leer gesetzt, und
// `Stand`, `SessionKey` und `vorschlaege_bilden` sind `pub(super)`.
//
// ZWEI Fälle, nicht einer: ihre Rotbeweise treffen verschiedene Stellen.
// Der Bestandsfall fällt unter dem entfernten Schlussriegel (drei Vorschläge
// statt zwei), der Ausrichtungsfall unter einer `zip`-Paarbildung. Läge
// beides in einem Fall, machte ihn jede der beiden Mutationen rot, und dass
// die Bestandsprüfung unter der `zip`-Mutation grün bleibt, wäre an keinem
// Lauf mehr abzulesen.
#[cfg(test)]
mod tests {
    use super::super::hypothese::{Bandintervall, Beobachtung, Rangkomponenten};
    use super::super::zustand::SessionStand;
    use super::*;
    use std::sync::Arc;

    fn hex(n: usize) -> String {
        format!("{n:032x}")
    }

    fn session() -> SessionKey {
        SessionKey {
            project_binding_id: hex(0x11),
            session_epoch: hex(0x22),
        }
    }

    /// Ein vollständiger Befund mit Passage — die Form, aus der ein Vorschlag
    /// entsteht. `listen_for` reist als Parameter herein: es ist das eine
    /// Gate-Feld, das der Befund selbst trägt.
    fn befund(nr: usize, listen_for: &str) -> CauseHypothesis {
        CauseHypothesis {
            finding_id: hex(0x100 + nr),
            claim_class: Aussageklasse::Zusammenhang,
            ursachenklasse: Ursachenklasse::QuelleResonanz,
            target_metric: Zielmetrik::BandPegelDb,
            candidate_source: hex(0x200 + nr),
            passage_id: Some(hex(0x5001)),
            pre_post: None,
            band_hz: Bandintervall { von: 98, bis: 102 },
            beobachtung: Beobachtung {
                wert_db: 4.0,
                gueltig: true,
            },
            rang: Rangkomponenten {
                bandpassung: 0.8,
                koinzidenz: 0.7,
                uplift: 0.6,
                intent_relevanz: 0.5,
                wiederholbarkeit: 0.9,
                routingqualitaet: 1.0,
            },
            confidence: Befundkonfidenz {
                klasse: Sicherheitsklasse::Hoch,
                score: 0.75,
            },
            evidence_ids: vec![hex(0x1000 + nr)],
            alternatives: Vec::new(),
            ausschluesse: Vec::new(),
            next_test: NaechsterTest::ManuellerVersuch,
            zustand: Befundzustand::ReadyToSend,
            intent_revision: 0,
            intent_generation: 0,
            likely_cause: "Testbefund".into(),
            smallest_test: "Testschritt".into(),
            listen_for: listen_for.into(),
            maskierung: None,
            metrics_version: super::super::vergleichbarkeit::METRICS_VERSION,
        }
    }

    /// Die Lage, wie `proposallage_locked` sie für eine Sitzung mit
    /// eindeutigem führenden Main bildet — dieselben `messbare_guardrails`,
    /// dieselbe feste `capability_vorhanden = false`.
    fn lage() -> Proposallage {
        Proposallage {
            ziel_instanz: hex(1),
            base_revision: 0,
            intent: None,
            capability_vorhanden: false,
            messbare_guardrails: vec![
                Stopbedingung::GuardrailAbdeckung,
                Stopbedingung::GuardrailGeschuetzt,
                Stopbedingung::KeineWiederholbareMasteraenderung,
            ],
            usergrenze_db: None,
            session_epoch: hex(0x22),
            metrics_version: super::super::vergleichbarkeit::METRICS_VERSION,
        }
    }

    /// **V-12 (a), Bestandshälfte.** Drei Befunde einer Sitzung, genau einer
    /// ohne `listen_for`: der Riegel trifft GENAU ihn.
    ///
    /// Gemessen an den Beständen, die der Coordinator wirklich führt —
    /// `stand.vorschlaege` und `vorschlaege_sicht` —, nicht an einer
    /// Behauptung. Der Rotbeweis ist der entfernte Schlussriegel: dann
    /// entstehen DREI Vorschläge statt zwei, und der Befund ohne `listen_for`
    /// bekommt seinen (`NAK-214-rot-V-05b.txt`).
    #[test]
    fn riegel_trifft_nur_den_unvollstaendigen() {
        let c = Coordinator::mit_uhr(Arc::new(ManualClock::default()), hex(0xbeef));
        let session = session();
        {
            let mut stand = c.stand.lock().unwrap();
            stand.sessions.insert(
                session.clone(),
                SessionStand {
                    fuehrendes_main: Some(hex(1)),
                },
            );
            stand.befunde.insert(
                session.clone(),
                vec![
                    befund(0, "Band 3 kHz"),
                    befund(1, ""),
                    befund(2, "Band 5 kHz"),
                ],
            );
        }
        assert!(c.vorschlaege_bilden(&session));

        let sicht = c.vorschlaege_sicht(&session.project_binding_id, &session.session_epoch);
        assert_eq!(
            sicht.len(),
            2,
            "zwei von drei Befunden tragen einen Vorschlag"
        );
        let ohne_feld = hex(0x101);
        for vorschlag in &sicht {
            assert_ne!(
                vorschlag.finding_id.as_deref(),
                Some(ohne_feld.as_str()),
                "der Befund ohne `listen_for` bekommt keinen Vorschlag"
            );
            // Kein Vorschlag ohne Befund - die andere Richtung derselben
            // Zuordnung.
            assert!(vorschlag.finding_id.is_some());
        }
        {
            let stand = c.stand.lock().unwrap();
            let gehalten = stand
                .vorschlaege
                .get(&session)
                .expect("die Sitzung traegt Vorschlaege");
            assert_eq!(gehalten.len(), 2, "derselbe Bestand im Stand");
        }
        // `angebot` laeuft wirklich mit und ist in P5 `None`: die Zusage
        // "das Angebot gehoert zu einem Befund, der einen Vorschlag hat" ist
        // hier LEER erfuellt, und genau das wird gemessen (M-52, V-13).
        assert_eq!(
            c.draft_offers_zaehler(),
            0,
            "in P5 gibt es kein Angebot - die Capability fehlt"
        );
    }

    /// **V-12 (a), Ausrichtungshälfte.** Jedes Tupel trägt den Vorschlag
    /// SEINES Befunds — mit dem herausgefilterten Befund an erster,
    /// mittlerer und letzter Position.
    ///
    /// Der Rotbeweis (`NAK-214-rot-V-12.txt`) fährt die Paarbildung als `zip`
    /// über die Befunde und die gefilterte Vorschlagsliste. Er fällt an den
    /// Teilfällen ERSTE und MITTLERE Position; LETZTE bleibt grün — `zip`
    /// kürzt auf die kürzere Liste, und fällt der letzte Befund heraus,
    /// stehen die übrigen zwei noch an ihrem Platz. Der dritte Teilfall ist
    /// damit der Kontrollteilfall, der zeigt, dass die Prüfung nicht pauschal
    /// rot wird. Die LÄNGE bleibt unter derselben Mutation in allen drei
    /// grün: sie hält eine ausgefallene Filterung, nicht die Verschiebung.
    #[test]
    fn paare_bleiben_bei_ihrem_befund() {
        // 🔑 Die drei Positionen werden GESAMMELT und erst danach gemeinsam
        // geprueft. Bräche der Fall an der ersten Abweichung ab, zeigte der
        // Rotbeweis nur sie — und die Zusage „die LETZTE Position bleibt
        // unter derselben Mutation gruen" waere an keinem Lauf abzulesen.
        let mut laengen: Vec<(usize, usize)> = Vec::new();
        let mut verschoben: Vec<String> = Vec::new();
        let mut mitgelaufen: Vec<String> = Vec::new();
        for position in 0..3usize {
            let befunde: Vec<CauseHypothesis> = (0..3)
                .map(|i| befund(i, if i == position { "" } else { "Band 3 kHz" }))
                .collect();
            let paare = paare_bilden(&befunde, &lage());
            laengen.push((position, paare.len()));
            for (b, v) in &paare {
                if v.finding_id.as_deref() != Some(b.finding_id.as_str()) {
                    verschoben.push(format!(
                        "Position {position}: Befund {} traegt den Vorschlag von {:?}",
                        b.finding_id, v.finding_id
                    ));
                }
                if b.finding_id == hex(0x100 + position) {
                    mitgelaufen.push(format!(
                        "Position {position}: der Befund ohne `listen_for` ist im Paar"
                    ));
                }
            }
        }
        // Die LAENGE haelt eine ausgefallene oder zu weit greifende
        // Filterung — nicht die Verschiebung: unter einer `zip`-Mutation
        // bleibt sie in allen drei Positionen gruen, weil `zip` auf die
        // kuerzere Liste kuerzt. Das trennt die Zeile ausdruecklich.
        assert_eq!(
            laengen,
            vec![(0, 2), (1, 2), (2, 2)],
            "je Position tragen zwei von drei Befunden einen Vorschlag"
        );
        let getroffene: Vec<usize> = (0..3usize)
            .filter(|i| {
                let marke = format!("Position {i}:");
                verschoben.iter().chain(mitgelaufen.iter()).any(|z| z.starts_with(&marke))
            })
            .collect();
        assert!(
            verschoben.is_empty() && mitgelaufen.is_empty(),
            "{} Abweichungen an den Positionen {getroffene:?} von drei geprueften:              {verschoben:?} {mitgelaufen:?}",
            verschoben.len() + mitgelaufen.len()
        );
    }
}
