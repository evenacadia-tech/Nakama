//! Die Enthaltung — mit Ort und ohne.
//!
//! **M-27: „mehr Daten noetig" ist ein ERGEBNIS, kein Fehlerzweig.** Im
//! Evaluationskorpus zaehlt die Enthaltung als Enthaltung, nicht als
//! Fehlschlag. Sie hat deshalb dieselbe Form wie jeder andere Befund und
//! reist denselben Weg.
//!
//! Zwei Formen, und der Unterschied ist der ORT:
//!
//! * `enthaltung` traegt einen Ort — die Sitzung hat ein fuehrendes Main mit
//!   Beleg, aber kein Kandidat ueberlebt die Gates.
//! * `enthaltung_ohne_ort` traegt KEINEN Ort und KEINEN Wert (NAK-213 R4).
//!   Bis dahin endete `hypothesen()` in dieser Lage vor den Gates, und die
//!   Sitzung SCHWIEG (Gate-Befunde G-D4, G-L4).
//!
//! 🔑 **R8: beide setzen mindestens EINEN gueltigen Beleg voraus.**
//! `evidence_ids` traegt `minItems: 1` (`eq-ipc-v3.schema.json`), und der
//! Eintragungsriegel laesst nur nicht zurueckgenommene IDs zu (M-28). Ohne
//! einen solchen Beleg entsteht KEIN Befund, auch keine Enthaltung — die
//! Sitzung ist dann still, weil nichts gemessen ist, nicht weil etwas
//! verschwiegen wuerde.

use super::{
    finding_id, zustand_aus_sicherheit, Aufnahme, Aussageklasse, Ausschluss, Bandintervall,
    Befundkonfidenz, Beobachtung, CauseHypothesis, NaechsterTest, Quellprofil, Rangkomponenten,
    Sicherheitsklasse, Ursachenklasse, Zielmetrik, BAENDER_FEIN,
};

/// Warum die Rechnung GAR NICHT ERST laufen kann (R4).
///
/// ⚠️ Der Grund reist im festen Satz von `likely_cause` — dem Feld, das
/// §36.3 fuer genau diese Zeile vorsieht. Ein eigenes Vertragsfeld
/// `enthaltungsgrund` waere eine neue geschlossene Menge mit einem Consumer,
/// den dieses Ticket nicht bauen darf (§2.9 Nr. 8); ob und wie Gen ihn zeigt,
/// ist S31b.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Enthaltungsgrund {
    /// `fuehrendes_main = None` — kein oder mehr als ein fuehrendes Main (R4).
    KeineEindeutigeFuehrung,
    /// Das fuehrende Main hat keinen verwertbaren Beleg (R4).
    KeinMasterbeleg,
}

impl Enthaltungsgrund {
    /// Der feste Satz. Regelbasiert, keine Erklaerschicht.
    pub fn satz(self) -> &'static str {
        match self {
            Self::KeineEindeutigeFuehrung => {
                "Zwei Instanzen fuehren diese Sitzung — bis eine von ihnen fuehrt, wird nicht gerechnet."
            }
            Self::KeinMasterbeleg => {
                "Das fuehrende Main hat noch keinen verwertbaren Beleg gesendet."
            }
        }
    }
}

/// Die Enthaltung, wenn die Rechnung gar nicht erst laufen kann (R4).
///
/// Sie behauptet KEINEN Ort und KEINEN Wert — und schweigt trotzdem nicht.
/// M-27 nennt „mehr Daten noetig" ein regulaeres Ergebnis; bis NAK-213 endete
/// `hypothesen()` in dieser Lage vor den Gates, und die Sitzung SCHWIEG
/// (Gate-Befunde G-D4, G-L4).
///
/// 🔑 **R8 (Dirigent, 08.09.2026):** sie setzt MINDESTENS EINEN gueltigen
/// Beleg der Sitzung voraus. `evidence_ids` traegt `minItems: 1`
/// (`eq-ipc-v3.schema.json`:1889), und der Eintragungsriegel laesst nur nicht
/// zurueckgenommene IDs zu (M-28). Ohne einen solchen Beleg kann die
/// Pflichtliste gar nicht gueltig gefuellt werden — dann entsteht KEIN
/// Befund, auch keine Enthaltung. Die bestehenden Befunde der Sitzung sind zu
/// diesem Zeitpunkt ueber den unveraenderten M-24-Weg schon fort.
///
/// `None` heisst deshalb: die Sitzung ist still, weil nichts gemessen ist —
/// nicht, weil etwas verschwiegen wuerde.
pub(super) fn enthaltung_ohne_ort(aufnahme: &Aufnahme, grund: Enthaltungsgrund) -> Option<CauseHypothesis> {
    // Die deterministische Adresse der Sitzung: die lexikographisch kleinste
    // `instance_id` der `main`-Clients. Sie behauptet KEINE Fuehrung.
    let quelle = aufnahme.mains.first()?.quelle_id.clone();

    // Die juengsten GUELTIGEN Belege der Sitzung, ueber ALLE Quellen. Die
    // Fensterfolgen sind bereits um zurueckgenommene Belege bereinigt
    // (`fenster_aus_historie`), und `empfangsfolge` ordnet sie.
    //
    // 🔑 **Nacharbeit 1 (08.09.2026, Erstpruefungsbefund 1):** gesammelt wird
    // ueber `mains`, nicht ueber `master`. In der Lage, fuer die
    // `KeineEindeutigeFuehrung` steht, ist `master` per Definition `None` —
    // eine Sammlung ueber ihn kann dort gar nichts finden, und eine Sitzung
    // mit gueltig messendem Main schwieg. `mains` traegt die Profile ALLER
    // Main-Clients und schliesst den Master ein, wenn es einen gibt (er ist
    // ein Klon daraus); doppelte IDs faengt der `dedup_by` unten.
    let mut belege: Vec<(u64, &str)> = aufnahme
        .kandidaten
        .iter()
        .chain(aufnahme.mains.iter())
        .flat_map(|q| {
            q.fenster
                .iter()
                .map(|f| (f.empfangsfolge, f.evidence_id.as_str()))
        })
        .collect();
    belege.sort();
    belege.dedup_by(|a, b| a.1 == b.1);
    // Der Vertrag deckelt bei 32. Gekappt wird am ALTEN Ende, wie im Befund:
    // die juengsten Belege sind die, um die es geht.
    if belege.len() > 32 {
        let ab = belege.len() - 32;
        belege.drain(..ab);
    }
    let evidence_ids: Vec<String> = belege.iter().map(|(_, id)| (*id).to_string()).collect();
    // R8: ohne einen einzigen gueltigen Beleg entsteht kein Befund.
    if evidence_ids.is_empty() {
        return None;
    }

    let metrik = Zielmetrik::BandPegelDb;
    // Das VOLLE Gitter heisst „nicht eingegrenzt", nicht „ueberall". Ein
    // Teilintervall waere eine Ortsbehauptung ohne Messung.
    let band = Bandintervall {
        von: 0,
        bis: BAENDER_FEIN as u32,
    };
    Some(CauseHypothesis {
        finding_id: finding_id(aufnahme, metrik, band, &quelle),
        claim_class: Aussageklasse::Zusammenhang,
        ursachenklasse: Ursachenklasse::DatenReichenNicht,
        target_metric: metrik,
        candidate_source: quelle,
        passage_id: aufnahme.passage_id.clone(),
        pre_post: None,
        band_hz: band,
        // Dasselbe Muster wie NAK-212 E5/N-29: ohne das Bit ist die Zahl keine
        // Messung. Erst BEIDE Felder zusammen sind eindeutig.
        beobachtung: Beobachtung {
            wert_db: 0.0,
            gueltig: false,
        },
        rang: Rangkomponenten::default(),
        confidence: Befundkonfidenz {
            klasse: Sicherheitsklasse::Unklar,
            score: 0.0,
        },
        evidence_ids,
        alternatives: Vec::new(),
        ausschluesse: Vec::new(),
        next_test: NaechsterTest::MehrDatenSammeln,
        zustand: zustand_aus_sicherheit(Sicherheitsklasse::Unklar, false),
        intent_revision: aufnahme.intent.as_ref().map_or(0, |i| i.revision),
        intent_generation: aufnahme.intent.as_ref().map_or(0, |i| i.generation),
        likely_cause: grund.satz().into(),
        smallest_test: NaechsterTest::MehrDatenSammeln.satz().into(),
        listen_for: "Noch nichts — erst mehr Material sammeln.".into(),
        maskierung: None,
        metrics_version: aufnahme.metrics_version,
    })
}

/// Der Befund, wenn kein Kandidat ueberlebt hat (M-27).
pub(super) fn enthaltung(
    aufnahme: &Aufnahme,
    master: &Quellprofil,
    metrik: Zielmetrik,
    band: Bandintervall,
    beobachtung: Beobachtung,
    ausschluesse: Vec<Ausschluss>,
) -> CauseHypothesis {
    let quelle = master.quelle_id.clone();
    let finding_id = finding_id(aufnahme, metrik, band, &quelle);
    CauseHypothesis {
        finding_id,
        claim_class: Aussageklasse::Zusammenhang,
        ursachenklasse: Ursachenklasse::DatenReichenNicht,
        target_metric: metrik,
        candidate_source: quelle,
        passage_id: aufnahme.passage_id.clone(),
        pre_post: None,
        band_hz: band,
        beobachtung,
        rang: Rangkomponenten::default(),
        confidence: Befundkonfidenz {
            klasse: Sicherheitsklasse::Unklar,
            score: 0.0,
        },
        evidence_ids: master
            .fenster
            .iter()
            .map(|f| f.evidence_id.clone())
            .collect(),
        alternatives: Vec::new(),
        ausschluesse,
        next_test: NaechsterTest::MehrDatenSammeln,
        zustand: zustand_aus_sicherheit(Sicherheitsklasse::Unklar, false),
        intent_revision: aufnahme.intent.as_ref().map_or(0, |i| i.revision),
        // E-14/WN-02: die Generation des Bestands, aus dem die Aufnahme kam.
        intent_generation: aufnahme.intent.as_ref().map_or(0, |i| i.generation),
        likely_cause: "Die Datenlage traegt noch keine Ursachenaussage.".into(),
        smallest_test: NaechsterTest::MehrDatenSammeln.satz().into(),
        listen_for: "Noch nichts — erst mehr Material sammeln.".into(),
        // Eine Enthaltung hat keinen Kandidaten und damit kein Paar, ueber
        // dessen Maskierung sich etwas sagen liesse.
        maskierung: None,
        metrics_version: aufnahme.metrics_version,
    }
}
