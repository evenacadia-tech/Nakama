//! Die geschlossenen Mengen des Produzenten (SONDE-014 Etappe C).
//!
//! Jede dieser sechs Mengen lebt an EINER Stelle im Vertrag
//! (`eq-ipc-v3.schema.json`), und
//! `geschlossene_mengen_des_produzenten_decken_sich_mit_dem_vertrag` haelt
//! jede Liste gegen das eingefrorene Schema (M-77). Sie sind hier keine
//! zweite Wahrheit, sondern die Rust-Haelfte derselben.
//!
//! ── JEDER AUSGESCHIEDENE TRAEGT EINEN GRUND (M-87) ────────────────────────
//!
//! `Ausschlussgrund` ist die geschlossene Menge, aus der M-87 seinen Grund
//! nimmt: „JEDER Kandidat, der ausscheidet, traegt einen Grund aus einer
//! geschlossenen Menge; ein kommentarlos entfernter Kandidat ist ein Defekt".
//! Wer hier eine Variante anhaengt, haengt sie ANS ENDE — `wire()` und
//! `aus_wire()` gehen ueber den Index (`ausschlussgruende_sind_index_treu`),
//! und ein Einschub verschoebe jede bestehende Zahl.
//!
//! Welche dieser Gruende die KONKURRENZ unvollstaendig machen, entscheidet
//! nicht diese Datei, sondern `ist_messgrund` zusammen mit `konkurrenzlage`
//! in `klassenwahl.rs`.

use super::Evidenzfenster;

/// Die SIEBEN Ursachenklassen aus Entwurf §8 (M-14).
///
/// Die siebte ist ein REGULAERES Ergebnis und kein Fehlerzweig: im
/// Evaluationskorpus zaehlt sie als Enthaltung, nicht als Fehlschlag (M-27).
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Ursachenklasse {
    QuelleResonanz,
    ZweiQuellenKonkurrenz,
    EffektkettePrePost,
    SummeAufMaster,
    PeakAusTransient,
    StereoAusQuelleOderKette,
    DatenReichenNicht,
}

/// Die Wireform aller sieben, in Vertragsreihenfolge.
pub const URSACHENKLASSEN: [&str; 7] = [
    "quelle_resonanz",
    "zwei_quellen_konkurrenz",
    "effektkette_pre_post",
    "summe_auf_master",
    "peak_aus_transient",
    "stereo_aus_quelle_oder_kette",
    "daten_reichen_nicht",
];

impl Ursachenklasse {
    pub fn wire(self) -> &'static str {
        URSACHENKLASSEN[self as usize]
    }

    /// Fail-closed: ein unbekanntes Wort ist `None`, nie ein Zweig „sonst".
    pub fn aus_wire(wort: &str) -> Option<Self> {
        Some(match wort {
            "quelle_resonanz" => Self::QuelleResonanz,
            "zwei_quellen_konkurrenz" => Self::ZweiQuellenKonkurrenz,
            "effektkette_pre_post" => Self::EffektkettePrePost,
            "summe_auf_master" => Self::SummeAufMaster,
            "peak_aus_transient" => Self::PeakAusTransient,
            "stereo_aus_quelle_oder_kette" => Self::StereoAusQuelleOderKette,
            "daten_reichen_nicht" => Self::DatenReichenNicht,
            _ => return None,
        })
    }

    /// M-27: „mehr Daten noetig" ist ein Ergebnis, kein Fehler. Diese
    /// Funktion ist die einzige Stelle, an der die Unterscheidung faellt.
    pub fn ist_enthaltung(self) -> bool {
        matches!(self, Self::DatenReichenNicht)
    }
}

/// Die drei sichtbar getrennten Aussageklassen aus §36.1 (M-17).
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Aussageklasse {
    Zusammenhang,
    Wirkungsbeleg,
    Ursachenbeleg,
}

pub const AUSSAGEKLASSEN: [&str; 3] = ["zusammenhang", "wirkungsbeleg", "ursachenbeleg"];

impl Aussageklasse {
    pub fn wire(self) -> &'static str {
        AUSSAGEKLASSEN[self as usize]
    }
}

/// Die Zielgroesse eines Befunds, gebunden an das, was der
/// `evidence_snapshot` WIRKLICH traegt.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Zielmetrik {
    BandPegelDb,
    BandSpanneDb,
    PeakEreignisrate,
    FlussEreignisrate,
    StereoSeitenanteilDb,
}

pub const ZIELMETRIKEN: [&str; 5] = [
    "band_pegel_db",
    "band_spanne_db",
    "peak_ereignisrate",
    "fluss_ereignisrate",
    "stereo_seitenanteil_db",
];

impl Zielmetrik {
    pub fn wire(self) -> &'static str {
        ZIELMETRIKEN[self as usize]
    }

    /// Welche Datenklasse ein Beleg tragen MUSS, damit diese Metrik ueberhaupt
    /// gerechnet werden kann. Ohne sie faellt der Kandidat mit
    /// `capability_fehlt` aus dem Ranking — nicht mit einer 0, die wie eine
    /// Messung aussaehe.
    pub(super) fn braucht(self, f: &Evidenzfenster) -> bool {
        match self {
            Self::BandPegelDb => f.hat_baender,
            Self::BandSpanneDb => f.hat_verteilung,
            Self::PeakEreignisrate | Self::FlussEreignisrate => f.hat_baender,
            Self::StereoSeitenanteilDb => f.hat_stereo,
        }
    }
}

/// Die drei Sicherheitsstufen aus §8 „Ergebnisform".
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Sicherheitsklasse {
    Unklar,
    Mittel,
    Hoch,
}

impl Sicherheitsklasse {
    pub fn wire(self) -> &'static str {
        match self {
            Self::Hoch => "hoch",
            Self::Mittel => "mittel",
            Self::Unklar => "unklar",
        }
    }
}

pub const SICHERHEITSKLASSEN: [&str; 3] = ["hoch", "mittel", "unklar"];

/// Der billigste naechste Beweisschritt (§8 Teil 6, §46.2).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum NaechsterTest {
    PassageMessen,
    RoutingBestaetigen,
    PrePostPaarMessen,
    ManuellerVersuch,
    KeineAenderungEmpfohlen,
    MehrDatenSammeln,
}

pub const NAECHSTE_TESTS: [&str; 6] = [
    "passage_messen",
    "routing_bestaetigen",
    "pre_post_paar_messen",
    "manueller_versuch",
    "keine_aenderung_empfohlen",
    "mehr_daten_sammeln",
];

impl NaechsterTest {
    pub fn wire(self) -> &'static str {
        match self {
            Self::PassageMessen => "passage_messen",
            Self::RoutingBestaetigen => "routing_bestaetigen",
            Self::PrePostPaarMessen => "pre_post_paar_messen",
            Self::ManuellerVersuch => "manueller_versuch",
            Self::KeineAenderungEmpfohlen => "keine_aenderung_empfohlen",
            Self::MehrDatenSammeln => "mehr_daten_sammeln",
        }
    }

    pub fn aus_wire(wort: &str) -> Option<Self> {
        NAECHSTE_TESTS
            .iter()
            .position(|w| *w == wort)
            .map(|i| Self::ALLE[i])
    }

    pub const ALLE: [NaechsterTest; 6] = [
        Self::PassageMessen,
        Self::RoutingBestaetigen,
        Self::PrePostPaarMessen,
        Self::ManuellerVersuch,
        Self::KeineAenderungEmpfohlen,
        Self::MehrDatenSammeln,
    ];

    /// Die Zeile `SMALLEST TEST` auf Gen Flaeche 1 (M-34).
    ///
    /// ⚠️ Sie **bildet `next_test` ab** und ist keine Anzeigezusammenfassung.
    /// Deshalb steht sie hier am Enum und nicht als `match` im Erzeuger: eine
    /// zweite Stelle mit derselben Abbildung waere eine zweite Wahrheit, und
    /// die Anzeige muesste raten, welche gilt. Die Abbildung ist **total** —
    /// jeder der sechs Werte hat seinen Satz, keiner faellt auf einen
    /// Sammelzweig.
    pub fn satz(self) -> &'static str {
        match self {
            Self::PassageMessen => "Dieselbe Passage laenger messen.",
            Self::RoutingBestaetigen => "Routing dieser Quelle bestaetigen.",
            Self::PrePostPaarMessen => "PRE/POST-Paar an dieser Quelle messen.",
            Self::ManuellerVersuch => "Die Quelle in dieser Passage kurz absenken und gegenhoeren.",
            Self::KeineAenderungEmpfohlen => "Nichts aendern — der Befund traegt keinen Eingriff.",
            Self::MehrDatenSammeln => "Dieselbe Passage laenger messen.",
        }
    }
}

/// Die ZEHN Ausschlussgruende (M-87, R4). Geschlossen und beidseitig geprueft.
///
/// NAK-213 (Fassung 4) haengt die letzten zwei an — ANS ENDE, weil `wire()`
/// und `aus_wire()` ueber den Index gehen (`ausschlussgruende_sind_index_treu`)
/// und ein Einschub jede bestehende Zahl verschoebe.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Ausschlussgrund {
    CoverageFehlt,
    AlignmentFalsch,
    PassageUnvergleichbar,
    PassageZuKurz,
    IntentVetoGeschuetzt,
    IntentVetoVerschmolzen,
    CapabilityFehlt,
    EvidenzZurueckgenommen,
    /// R1 (NAK-213): jedes Gate bestanden, den Deckel aus M-18 nicht erreicht.
    /// Der Kandidat ist nicht widerlegt, sondern UEBERBOTEN — und er
    /// verschwindet nicht mehr kommentarlos (M-87).
    ScreeningUeberboten,
    /// R3 (NAK-213): der Kandidat misst den Mixerkanal des fuehrenden Mains.
    /// Wer das Mastersignal misst, ist nie dessen Ursache.
    MasterDuplikat,
}

pub const AUSSCHLUSSGRUENDE: [&str; 10] = [
    "coverage_fehlt",
    "alignment_falsch",
    "passage_unvergleichbar",
    "passage_zu_kurz",
    "intent_veto_geschuetzt",
    "intent_veto_verschmolzen",
    "capability_fehlt",
    "evidenz_zurueckgenommen",
    "screening_ueberboten",
    "master_duplikat",
];

impl Ausschlussgrund {
    pub fn wire(self) -> &'static str {
        AUSSCHLUSSGRUENDE[self as usize]
    }

    pub fn aus_wire(wort: &str) -> Option<Self> {
        AUSSCHLUSSGRUENDE
            .iter()
            .position(|w| *w == wort)
            .map(|i| Self::ALLE[i])
    }

    /// Ist dieser Grund ein MESSGRUND im Sinne von R2?
    ///
    /// R2 (NAK-213) nennt woertlich fuenf: `coverage_fehlt`,
    /// `alignment_falsch`, `passage_unvergleichbar`, `passage_zu_kurz`,
    /// `evidenz_zurueckgenommen`. Ein Konkurrent, der an einem von ihnen
    /// faellt, macht die KONKURRENZ unvollstaendig — ueber ihn liegt keine
    /// verwertbare Messung vor, und der Ueberlebende hat sich gegen ihn nicht
    /// behauptet, sondern nur ueberlebt.
    ///
    /// NICHT dabei sind die zwei Vetos: sie sind der ausdrueckliche Wille des
    /// Users (M-03, M-04) und machen nicht die Messung unvollstaendig,
    /// sondern die Frage unzulaessig. Ebenfalls nicht dabei ist
    /// `capability_fehlt` — die Regel nennt ihn nicht (Nebenbefund NB-1 im
    /// Manifest). `screening_ueberboten` ist ueber E2 geregelt, und
    /// `master_duplikat` ist gerade der Nachweis, dass der Kandidat keine
    /// Ursache sein KANN.
    pub fn ist_messgrund(self) -> bool {
        matches!(
            self,
            Self::CoverageFehlt
                | Self::AlignmentFalsch
                | Self::PassageUnvergleichbar
                | Self::PassageZuKurz
                | Self::EvidenzZurueckgenommen
        )
    }

    pub const ALLE: [Ausschlussgrund; 10] = [
        Self::CoverageFehlt,
        Self::AlignmentFalsch,
        Self::PassageUnvergleichbar,
        Self::PassageZuKurz,
        Self::IntentVetoGeschuetzt,
        Self::IntentVetoVerschmolzen,
        Self::CapabilityFehlt,
        Self::EvidenzZurueckgenommen,
        Self::ScreeningUeberboten,
        Self::MasterDuplikat,
    ];
}

#[cfg(test)]
mod tests {
    use super::*;
    use super::super::{BEFUNDZUSTAENDE, ROLLENPRIORITAET};

    /// Die geschlossenen Mengen sind wirklich geschlossen, und jede Wireform
    /// kommt ueber ihren eigenen Rueckweg zurueck.
    #[test]
    fn geschlossene_mengen_sind_rund() {
        assert_eq!(URSACHENKLASSEN.len(), 7);
        // NAK-213 (Fassung 4): zwei Gruende kommen dazu, AM ENDE.
        assert_eq!(AUSSCHLUSSGRUENDE.len(), 10);
        assert_eq!(
            &AUSSCHLUSSGRUENDE[8..],
            &["screening_ueberboten", "master_duplikat"]
        );
        assert_eq!(AUSSAGEKLASSEN.len(), 3);
        assert_eq!(ZIELMETRIKEN.len(), 5);
        assert_eq!(NAECHSTE_TESTS.len(), 6);
        assert_eq!(BEFUNDZUSTAENDE.len(), 3);
        assert_eq!(SICHERHEITSKLASSEN.len(), 3);
        assert_eq!(ROLLENPRIORITAET.len(), 5);
        for (i, wort) in URSACHENKLASSEN.iter().enumerate() {
            let k = Ursachenklasse::aus_wire(wort).expect("bekannt");
            assert_eq!(k.wire(), *wort);
            assert_eq!(k as usize, i, "die Ordnung ist die Vertragsreihenfolge");
        }
        for (i, wort) in AUSSCHLUSSGRUENDE.iter().enumerate() {
            let g = Ausschlussgrund::aus_wire(wort).expect("bekannt");
            assert_eq!(g.wire(), *wort);
            assert_eq!(Ausschlussgrund::ALLE[i], g);
        }
        assert!(Ursachenklasse::aus_wire("QUELLE_RESONANZ").is_none());
        assert!(Ausschlussgrund::aus_wire("coverage").is_none());
        // Die siebte Ursachenklasse ist die EINZIGE Enthaltung.
        let enthaltungen = URSACHENKLASSEN
            .iter()
            .filter(|w| Ursachenklasse::aus_wire(w).unwrap().ist_enthaltung())
            .count();
        assert_eq!(enthaltungen, 1);
    }
}
