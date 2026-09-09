//! Befundbau und Sicht: die `CauseHypothesis` und was sie traegt.
//!
//! Hier laufen die Fachbereiche zusammen. `baue_befund` liest den Rang und
//! den Beleg aus `zusammenhang.rs`, die Konkurrenzlage aus `klassenwahl.rs`
//! und das Passagenmaterial aus `messung.rs` — und macht daraus die EINE
//! Form, die der Vertrag §36.3 kennt.
//!
//! ── AUS PARALLELER TELEMETRIE ENTSTEHT NIE KLASSE 2 ODER 3 (M-17) ─────────
//!
//! Der Wirkungsbeleg verlangt ein PRE/POST-Paar, der Ursachenbeleg eine
//! kontrollierte Preview — und die gibt es in P5 gar nicht (§42.2 Punkt 5).
//! `baue_befund` kann `Ursachenbeleg` deshalb gar nicht erzeugen, und
//! `Wirkungsbeleg` haengt an der WIRKUNGSmarke des Paars, nicht an seiner
//! blossen Existenz (NAK-214 R2/R6).
//!
//! ── SECHS TEILE, JEDER AN SEINER STELLE (M-16) ────────────────────────────
//!
//! `sechs_teile` nennt fuer jeden Teil der Ergebnisform die Stelle, an der er
//! im Objekt wirklich steht. Ein Teil, den das Objekt nicht traegt, ist
//! `false` — und genau daran faellt der Rotbeweis. Die sechs sind Datenmodell
//! und Regelbasis, KEINE Vorgabe, sechs Werte anzuzeigen: Abnahme U21 zeigt
//! genau drei Zeilen.
//!
//! Die `finding_id` ist deterministisch (M-25): gleicher Eingang, gleiche ID —
//! aus Sitzung, Metrik, Band, Quelle und Kalibrierungsfassung, nie aus einem
//! Zaehler und nie aus der Zeit.

use super::messung::{passagenmaterial, unabhaengige_fenster};
use super::{
    quantisiert, zustand_aus_sicherheit, Aufnahme, Aussageklasse, Ausschlussgrund,
    Befundkonfidenz, Befundzustand, Konkurrenzlage, NaechsterTest, Quellprofil, Rangkomponenten,
    Sicherheitsklasse, Ursachenklasse, Zielmetrik, Zusammenhangsbeleg, BAENDER_FEIN,
    NAECHSTE_TESTS, RANG_QUANTUM,
};
use crate::coordinator::vergleichbarkeit::GATE_MINDEST_FENSTER;

/// Halboffenes Bandintervall `[von, bis)` im 221er-Gitter.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub struct Bandintervall {
    pub von: u32,
    pub bis: u32,
}

impl Bandintervall {
    pub fn gueltig(&self) -> bool {
        self.von < self.bis && (self.bis as usize) <= BAENDER_FEIN
    }
}

/// Ein Messwert mit seinem Gueltigkeitsbit. Ohne das Bit ist die Zahl keine
/// Messung.
#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Beobachtung {
    pub wert_db: f64,
    pub gueltig: bool,
}

/// Ein ausgeschiedener Kandidat samt Grund (M-87).
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct Ausschluss {
    pub candidate_source: String,
    pub grund: Ausschlussgrund,
}

/// Die `CauseHypothesis` aus §36.3 — zehn Felder — samt den sechs Teilen der
/// Ergebnisform aus §8 und den drei Anzeigezeilen aus Abnahme U21.
#[derive(Debug, Clone, PartialEq)]
pub struct CauseHypothesis {
    pub finding_id: String,
    pub claim_class: Aussageklasse,
    pub ursachenklasse: Ursachenklasse,
    pub target_metric: Zielmetrik,
    pub candidate_source: String,
    pub passage_id: Option<String>,
    /// §8 Teil 1, zweite Haelfte: die optionale PRE/POST-Stelle des Orts.
    pub pre_post: Option<&'static str>,
    pub band_hz: Bandintervall,
    pub beobachtung: Beobachtung,
    pub rang: Rangkomponenten,
    pub confidence: Befundkonfidenz,
    /// Exit-Gate §59 woertlich: jede sichtbare Behauptung referenziert
    /// EXISTENTE Evidenz-IDs. Leer heisst: der Befund ist nicht sichtbar.
    pub evidence_ids: Vec<String>,
    /// Abnahme U21: die IDs EIGENER Befunde, kein Text.
    pub alternatives: Vec<String>,
    pub ausschluesse: Vec<Ausschluss>,
    pub next_test: NaechsterTest,
    pub zustand: Befundzustand,
    pub intent_revision: i64,
    /// **E-14 / WN-02 (Nacharbeit 2, 07.09.2026): die Bestandsgeneration,
    /// unter der dieser Befund gerechnet wurde.**
    ///
    /// Sie reist NICHT ueber den Draht: der Vertrag traegt weiterhin die
    /// `intent_revision` (M-10), und die Fassung 3 wird ausserhalb ihrer
    /// eigenen Etappe nicht angefasst. Sie ist die brokerinterne Antwort auf
    /// die Frage, ob eine Aussage noch auf dem Bestand ruht, aus dem sie
    /// entstanden ist: das Veralten und die Eintragungspruefung messen sie,
    /// weil eine unveraenderte Sender-Revision einen geaenderten Bestand
    /// nicht ausschliesst (WP1-2).
    pub intent_generation: i64,
    pub likely_cause: String,
    pub smallest_test: String,
    pub listen_for: String,
    /// Der laufende Maskierungswert AM Befund (E-04, Etappe E).
    ///
    /// `None` heisst „dieser Befund traegt keinen" — nie „0 dB". Das Objekt
    /// haengt hier und nicht in einer zweiten Liste: zwei Listen koennten
    /// auseinanderlaufen, und die Zone waere eine zweite Wahrheit neben dem
    /// Befundtext.
    pub maskierung: Option<crate::coordinator::maskierung::Maskierung>,
    /// Die Fassung, unter der dieser Befund entstanden ist.
    pub metrics_version: u32,
}

/// Die SECHS Teile der Ergebnisform aus §8, in ihrer Vertragsreihenfolge.
///
/// Sie sind Datenmodell und Regelbasis — KEINE Vorgabe, sechs Werte
/// anzuzeigen (Abnahme U21: Gen zeigt genau drei Zeilen).
pub const ERGEBNISTEILE: [&str; 6] = [
    "ort",
    "beobachtung",
    "zusammenhang",
    "alternativen",
    "sicherheit",
    "naechster_beweisschritt",
];

impl CauseHypothesis {
    /// M-16: die sechs Teile, jeder mit der Stelle, an der er im Objekt
    /// wirklich steht. Ein Teil, den das Objekt nicht traegt, ist `false` —
    /// und genau daran faellt der Rotbeweis.
    pub fn sechs_teile(&self) -> [(&'static str, bool); 6] {
        [
            // 1. Ort: der Bus, optional mit PRE/POST-Stelle.
            ("ort", !self.candidate_source.is_empty()),
            // 2. Beobachtung: WAS gemessen wurde, mit Gueltigkeitsbit.
            (
                "beobachtung",
                self.band_hz.gueltig() && self.beobachtung.wert_db.is_finite(),
            ),
            // 3. Zusammenhang: Aussageklasse, Ursachenklasse und Rang.
            (
                "zusammenhang",
                self.rang.werte().iter().all(|w| w.is_finite()),
            ),
            // 4. Alternativen: eigene Befunde, als IDs. Eine LEERE Liste ist
            //    ein gueltiger Teil („es gibt keine"); eine fehlende Liste
            //    waere keiner. Die Unterscheidung traegt der Typ.
            ("alternativen", self.alternatives.len() <= 8),
            // 5. Sicherheit: Klasse UND Score, zwei Felder.
            (
                "sicherheit",
                self.confidence.score.is_finite() && (0.0..=1.0).contains(&self.confidence.score),
            ),
            // 6. Naechster Beweisschritt.
            (
                "naechster_beweisschritt",
                NAECHSTE_TESTS.contains(&self.next_test.wire()),
            ),
        ]
    }

    /// Der Tie-Break-Key (M-25/M-26). Er entscheidet die ANZEIGEREIHENFOLGE.
    pub fn tie_break(&self) -> (i64, &str) {
        // Absteigend nach Rang: das negierte Ganzzahlbild der quantisierten
        // Zahl. Ueber `f64` zu sortieren waere keine totale Ordnung.
        let stufen = (quantisiert(self.rang.rang()) / RANG_QUANTUM).round() as i64;
        (-stufen, self.candidate_source.as_str())
    }
}

#[allow(clippy::too_many_arguments)]
pub(super) fn baue_befund(
    aufnahme: &Aufnahme,
    master: &Quellprofil,
    kandidat: &Quellprofil,
    rang: Rangkomponenten,
    zusammenhang: &Zusammenhangsbeleg,
    metrik: Zielmetrik,
    band: Bandintervall,
    beobachtung: Beobachtung,
    parent_duplikat: bool,
    mehrere_kandidaten: bool,
    lage: Konkurrenzlage,
    getrennt: bool,
) -> CauseHypothesis {
    // M-17: aus paralleler Telemetrie allein entsteht NIE Klasse 2 oder 3.
    // `Ursachenbeleg` verlangt eine kontrollierte Preview — die es in P5
    // nicht gibt; dieses Modul kann den Wert deshalb gar nicht erzeugen.
    //
    // 🔑 **NAK-214 R2/R6:** die Aussageklasse haengt an der WIRKUNGSmarke,
    // nicht an der blossen Paarexistenz. `Probable`-Paare, Paare ohne ein
    // einziges gemessenes Band und identische PRE/POST-Paare tragen sie
    // nicht — sie bleiben Klasse 1.
    let claim_class = if kandidat.prepost_wirkungsbeleg {
        Aussageklasse::Wirkungsbeleg
    } else {
        Aussageklasse::Zusammenhang
    };
    let ursachenklasse = if mehrere_kandidaten {
        Ursachenklasse::ZweiQuellenKonkurrenz
    } else if kandidat.prepost_wirkungsbeleg {
        // Dieselbe Aussage in der Ursachenachse: „die Ursache ist die KETTE
        // dieser Quelle". Sie ohne Wirkungsbeleg zu behaupten waere derselbe
        // Bruch wie bei der Aussageklasse (NAK-214 E4).
        Ursachenklasse::EffektkettePrePost
    } else {
        Ursachenklasse::QuelleResonanz
    };
    // 🔑 NR-05 (Nacharbeit 1, 07.09.2026): fuehrt die Sitzung eine Passage,
    // zaehlt das Material BEIDER beteiligter Quellen darin — sonst das des
    // Kandidaten ueber seine ganze Historie.
    //
    // Ohne die Passagenform stuende hier eine Zahl, die das Passagengate
    // bereits als zu duenn verworfen haette, und die Sicherheit `hoch` haette
    // eine andere Grundlage als der Ausschlussgrund `passage_zu_kurz`.
    let fenster = match aufnahme.passage.as_ref() {
        Some(p) => passagenmaterial(kandidat, master, p),
        None => unabhaengige_fenster(&kandidat.fenster.iter().collect::<Vec<_>>()),
    };
    // Die Klasse entsteht STRUKTURELL, nicht aus dem Score (M-15). Die
    // numerische Schwelle hoch/mittel/unklar ist Ausgabe des Korpus
    // (M-31, Etappe H) und steht bewusst nirgends als Konstante.
    //
    // ⚠️ Die Sicherheit haengt AUSDRUECKLICH nicht an der Aussageklasse.
    // Beides sind zwei Achsen: `claim_class` sagt, WIE STARK die Art des
    // Belegs ist (Zusammenhang, Wirkungsbeleg, Ursachenbeleg), `confidence`
    // sagt, wie sicher DIESE Aussage ist. Wer die Sicherheit `hoch` an ein
    // PRE/POST-Paar knuepfte, machte sie in einer rein passiven Sitzung
    // strukturell unerreichbar — und genau dort verlangt M-23 ihren
    // Rotbeweis: „dieselbe Session mit einer Passage UNTER
    // `GATE_MINDEST_FENSTER` liefert READY TO SEND" ist der gebrochene
    // Zustand, also muss dieselbe Session MIT genug Fenstern ihn erreichen.
    //
    // 🔑 NAK-212 R1/R2/R3 (07.09.2026): vier Terme kommen dazu, und alle vier
    // sind PRAEDIKATE UEBER EINE BENANNTE STRUKTUR, kein Vergleich gegen eine
    // kalibrierte Zahl. `positiv_belegt()` fragt „ist eine Komponente UEBER
    // NULL", nicht „ueber 0,3"; die Vorzeichengrenze null ist die Definition
    // von „positiv" und steht deshalb nirgends als abstimmbarer Wert. M-15
    // („die Klasse wird nicht aus dem Score gerundet") und M-31 („Schwelle
    // kalibriert, nie geraten") bleiben unberuehrt — dieselbe Lesart, die
    // schon `parent_duplikat` und `routing_bekannt` tragen.
    let bandpassung_ist_null = !(rang.bandpassung > 0.0);
    let klasse = if fenster < GATE_MINDEST_FENSTER
        || !kandidat.routing_bekannt
        || parent_duplikat
        // R1 (a): keine gemessene Energie im Befundband. Gemeint ist ein
        // Kandidat, dessen Baender im Intervall KEIN Gueltigkeitsbit tragen —
        // nicht einer mit wenig Energie. Fail-closed gegen NaN, dieselbe
        // negierte Form wie in `gate`.
        || bandpassung_ist_null
        // R1 (b): kein Beleg fuer einen Zusammenhang. Entwurf §36.1 Klasse 1
        // verlangt „Quelle und Masterproblem treten im selben Bereich UND
        // Zeitfenster auf" — ohne bedingten Uplift (M-19) und ohne
        // Onset-Koinzidenz ist davon nichts gemessen.
        || !zusammenhang.positiv_belegt()
        // R2: ein Gegenbeleg schliesst `hoch` aus. Der Kandidat bleibt
        // sichtbar und wird als Alternative gefuehrt — er ist nicht
        // widerlegt, er spricht dagegen.
        || zusammenhang.gegenbeleg()
        // R3/E6: ein ungetrennter erster Platz behauptet eine Unterscheidung,
        // die die Messung nicht hergibt.
        || !getrennt
        // 🔑 NAK-213 R1 (E2) und R2 (E4): die Konkurrenz dieser Rechnung ist
        // unvollstaendig. Entweder wurde am Deckel ohne Abstand geschnitten,
        // oder ein Konkurrent ist aus einem MESSGRUND ausgeschieden — in
        // beiden Lagen hat sich der Fuehrende nicht gegen die Konkurrenz
        // behauptet, sondern sie nur ueberlebt. Die Lage gilt der RECHNUNG
        // und damit jedem Befund aus ihr (K-21).
        || lage != Konkurrenzlage::Vollstaendig
    {
        // M-22/M-23: unbekanntes Routing, ein Parent-Duplikat oder zu wenig
        // Material tragen keine STARKE Aussage.
        Sicherheitsklasse::Mittel
    } else {
        Sicherheitsklasse::Hoch
    };
    // M-29: die EINE Abbildung im Datenweg. Frisch gerechnet ist ein Befund
    // nie veraltet — `stale` entsteht erst, wenn sich unter ihm etwas aendert
    // (Intent-Revision gestiegen, Belege zurueckgenommen).
    let zustand = zustand_aus_sicherheit(klasse, false);
    let next_test = if !kandidat.routing_bekannt {
        NaechsterTest::RoutingBestaetigen
    } else if !kandidat.prepost_wirkungsbeleg {
        // NAK-214 R2 woertlich: „`next_test` verlangt das Paar weiterhin".
        // Das gilt bei fehlendem Paar, bei einem Paar unterhalb der Schwelle
        // UND bei einem Paar ohne gemessene Veraenderung — der naechste
        // Beweisschritt ist in allen dreien derselbe (E5, NB-2).
        NaechsterTest::PrePostPaarMessen
    } else if fenster < GATE_MINDEST_FENSTER {
        NaechsterTest::PassageMessen
    } else {
        NaechsterTest::ManuellerVersuch
    };
    let mut evidence_ids: Vec<String> = kandidat
        .fenster
        .iter()
        .map(|f| f.evidence_id.clone())
        .collect();
    evidence_ids.dedup();
    // Der Vertrag deckelt bei 32 (`session_finding.evidence_ids.maxItems`).
    // Gekappt wird am ALTEN Ende, wie beim Snapshot: die juengsten Belege
    // sind die, um die es geht.
    if evidence_ids.len() > 32 {
        let ab = evidence_ids.len() - 32;
        evidence_ids.drain(..ab);
    }
    CauseHypothesis {
        finding_id: finding_id(aufnahme, metrik, band, &kandidat.quelle_id),
        claim_class,
        ursachenklasse,
        target_metric: metrik,
        candidate_source: kandidat.quelle_id.clone(),
        passage_id: aufnahme.passage_id.clone(),
        pre_post: kandidat.prepost_paar.then_some("post"),
        band_hz: band,
        beobachtung,
        rang,
        confidence: Befundkonfidenz {
            klasse,
            score: rang.rang(),
        },
        evidence_ids,
        alternatives: Vec::new(),
        ausschluesse: Vec::new(),
        next_test,
        zustand,
        intent_revision: aufnahme.intent.as_ref().map_or(0, |i| i.revision),
        // E-14/WN-02: die Generation des Bestands, aus dem die Aufnahme kam.
        intent_generation: aufnahme.intent.as_ref().map_or(0, |i| i.generation),
        likely_cause: format!(
            "{} draengt im Bandbereich {}..{} gegen den Master.",
            kurz(&kandidat.quelle_id),
            band.von,
            band.bis
        ),
        // M-34: die Zeile BILDET `next_test` ab; die Abbildung lebt am Enum.
        smallest_test: next_test.satz().into(),
        listen_for: "Ob der Master im markierten Bereich Luft bekommt.".into(),
        // M-36/M-41: der Wert faellt AUS dem Befund heraus — er entsteht mit
        // ihm, aus denselben zwei Quellen und demselben Bandbereich, und es
        // gibt keinen Weg, ihn ohne Befund zu bekommen.
        maskierung: crate::coordinator::maskierung::maskierung(master, kandidat, band),
        metrics_version: aufnahme.metrics_version,
    }
}

/// Die deterministische `finding_id` (M-25): gleicher Eingang, gleiche ID.
///
/// Sie entsteht aus Sitzung, Metrik, Band, Quelle und Kalibrierungsfassung —
/// nicht aus einem Zaehler und nicht aus der Zeit. Ein Zaehler machte zwei
/// Laeufe desselben Standes unterscheidbar, und genau das schliesst M-25 aus.
pub fn finding_id(
    aufnahme: &Aufnahme,
    metrik: Zielmetrik,
    band: Bandintervall,
    quelle: &str,
) -> String {
    use sha2::{Digest, Sha256};
    let mut hasher = Sha256::new();
    hasher.update(b"nakama.sonde014.finding.v1\0");
    hasher.update(aufnahme.session_epoch.as_bytes());
    hasher.update(b"\0");
    hasher.update(metrik.wire().as_bytes());
    hasher.update(b"\0");
    hasher.update(band.von.to_be_bytes());
    hasher.update(band.bis.to_be_bytes());
    hasher.update(quelle.as_bytes());
    hasher.update(b"\0");
    hasher.update(aufnahme.metrics_version.to_be_bytes());
    let digest = hasher.finalize();
    digest[..16].iter().map(|b| format!("{b:02x}")).collect()
}

pub(super) fn kurz(id: &str) -> String {
    id.chars().take(8).collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Die `finding_id` ist eine Funktion ihres Eingangs — und nur davon.
    #[test]
    fn finding_id_ist_deterministisch_und_trennt() {
        let mut aufnahme = Aufnahme {
            session_epoch: "22".into(),
            metrics_version: 20_260_904,
            ..Default::default()
        };
        let band = Bandintervall { von: 10, bis: 14 };
        let a = finding_id(&aufnahme, Zielmetrik::BandPegelDb, band, "q1");
        let b = finding_id(&aufnahme, Zielmetrik::BandPegelDb, band, "q1");
        assert_eq!(a, b, "gleicher Eingang, gleiche ID");
        assert_eq!(a.len(), 32, "hex32");
        assert!(a.chars().all(|c| c.is_ascii_hexdigit() && !c.is_uppercase()));
        assert_ne!(a, finding_id(&aufnahme, Zielmetrik::BandPegelDb, band, "q2"));
        assert_ne!(
            a,
            finding_id(&aufnahme, Zielmetrik::BandSpanneDb, band, "q1")
        );
        assert_ne!(
            a,
            finding_id(
                &aufnahme,
                Zielmetrik::BandPegelDb,
                Bandintervall { von: 10, bis: 15 },
                "q1"
            )
        );
        aufnahme.session_epoch = "23".into();
        assert_ne!(
            a,
            finding_id(&aufnahme, Zielmetrik::BandPegelDb, band, "q1"),
            "zwei Sitzungen teilen keine ID"
        );
        aufnahme.session_epoch = "22".into();
        aufnahme.metrics_version = 20_260_905;
        assert_ne!(
            a,
            finding_id(&aufnahme, Zielmetrik::BandPegelDb, band, "q1"),
            "eine andere Kalibrierung ist ein anderer Befund"
        );
    }

    /// Die sechs Teile aus §8 sind sechs, in ihrer Reihenfolge, und jeder
    /// zeigt auf eine Stelle im Objekt.
    #[test]
    fn sechs_teile_sind_sechs_und_benannt() {
        let befund = CauseHypothesis {
            finding_id: "a".repeat(32),
            claim_class: Aussageklasse::Zusammenhang,
            ursachenklasse: Ursachenklasse::QuelleResonanz,
            target_metric: Zielmetrik::BandPegelDb,
            candidate_source: "q".into(),
            passage_id: None,
            pre_post: None,
            band_hz: Bandintervall { von: 0, bis: 4 },
            beobachtung: Beobachtung {
                wert_db: -12.0,
                gueltig: true,
            },
            rang: Rangkomponenten::default(),
            confidence: Befundkonfidenz {
                klasse: Sicherheitsklasse::Mittel,
                score: 0.5,
            },
            evidence_ids: vec!["b".repeat(32)],
            alternatives: Vec::new(),
            ausschluesse: Vec::new(),
            next_test: NaechsterTest::PassageMessen,
            zustand: Befundzustand::MoreData,
            intent_revision: 0,
            intent_generation: 0,
            likely_cause: "x".into(),
            smallest_test: "y".into(),
            listen_for: "z".into(),
            maskierung: None,
            metrics_version: 1,
        };
        let teile = befund.sechs_teile();
        assert_eq!(teile.len(), 6);
        for (i, (name, vorhanden)) in teile.iter().enumerate() {
            assert_eq!(*name, ERGEBNISTEILE[i]);
            assert!(vorhanden, "{name}");
        }
        // Nimmt man einen Teil heraus, faellt genau er — und nicht die
        // Nachbarn.
        let mut ohne_ort = befund.clone();
        ohne_ort.candidate_source = String::new();
        assert!(!ohne_ort.sechs_teile()[0].1);
        assert!(ohne_ort.sechs_teile()[1].1);
        let mut ohne_beobachtung = befund.clone();
        ohne_beobachtung.band_hz = Bandintervall { von: 4, bis: 4 };
        assert!(!ohne_beobachtung.sechs_teile()[1].1);

        // Der Tie-Break ist total geordnet und haengt am Rang, nicht an der
        // Reihenfolge des Aufrufers.
        let mut stark = befund.clone();
        stark.rang.bandpassung = 1.0;
        stark.candidate_source = "z".into();
        assert!(stark.tie_break() < befund.tie_break(), "hoeherer Rang zuerst");
    }
}
