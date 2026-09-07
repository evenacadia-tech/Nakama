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
        let neue: Vec<Proposal> = befunde.iter().map(|b| proposal(b, &lage)).collect();
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
        let angebot: Option<Proposal> = befunde
            .iter()
            .zip(neue.iter())
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
            let geschrieben = self.push_ziel_noch_gueltig(&link_id, ziel)
                && push
                    .as_ref()
                    .is_some_and(|p| p.snapshot_schreiben(&link_id, &payload));
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

/// Wie lange ein Angebot gilt (§33.3 `steuerkopf.ttl_ms`).
///
/// Zehn Sekunden: lang genug, dass ein User den Vorschlag liest, kurz genug,
/// dass er nicht eine Passage später noch gilt. §78 nennt keine Zahl; das ist
/// deshalb eine **Produktgrenze** und keine Messung.
pub const DRAFT_OFFER_TTL_MS: u64 = 10_000;

fn zahl(wert: f64, min: f64, max: f64) -> f64 {
    if !wert.is_finite() {
        return 0.0;
    }
    wert.clamp(min, max)
}

fn zahl_wert(wert: f64, min: f64, max: f64) -> Value {
    Value::from(zahl(wert, min, max))
}
