//! Kandidatenbildung, Rang und `CauseHypothesis` (SONDE-014 Etappe C).
//!
//! ── WER HIER RECHNET, UND WARUM AUSGERECHNET HIER ─────────────────────────
//!
//! Entscheid **E-08** weist Hypothese, Proposal und Maskierung ausdruecklich
//! dem Broker-Coordinator zu: „dort liegen Evidenzbestand und Store". Der
//! Intent dagegen gehoert dem Main — dieses Modul LIEST ihn nur (ueber
//! `intent::IntentBestand`) und schreibt ihn nie zurueck.
//!
//! Dieses Modul ist eine REINE Rechnung. Es kennt weder Lock noch Store noch
//! Senke; alles, was es braucht, steht in `Aufnahme`. Der Produktpfad — wer
//! die Aufnahme fuellt, wann gerechnet wird und wohin das Ergebnis reist —
//! liegt in `hypothese_verdrahtung.rs`. Das ist Risiko **R1** des Bauplans:
//! ein Test, der nur dieses Modul ruft, kann nicht fallen, wenn die
//! Verdrahtung fehlt.
//!
//! ── DIE DREI HARTEN REGELN DIESES MODULS ──────────────────────────────────
//!
//! 1. **Gate vor Gewichtung (M-20).** Fehlende Coverage und falsches
//!    Alignment sind kein Gewicht, das ein anderer Wert ausgleichen koennte.
//!    Ein Kandidat, der eines der beiden reisst, verlaesst das Ranking mit
//!    einem GRUND aus der geschlossenen Achtermenge (M-87) — er wird nie
//!    kommentarlos entfernt.
//! 2. **Aus paralleler Telemetrie entsteht nie Klasse 2 oder 3 (M-17).** Der
//!    Wirkungsbeleg verlangt ein PRE/POST-Paar, der Ursachenbeleg eine
//!    kontrollierte Preview — und die gibt es in P5 gar nicht (§42.2 Punkt 5).
//!    `aussageklasse()` kann `Ursachenbeleg` in diesem Modul nicht erzeugen.
//! 3. **Gleicher Eingang, gleiche Reihenfolge (M-25).** Bootstrap-Saat,
//!    Reduktionsreihenfolge, Eingangsquantisierung und Tie-Break-Key stehen
//!    als benannte Konstanten hier und gehoeren zur `metrics_version`
//!    (§36.4). Sie sind ABSICHTLICH nicht im Register `metriken-v1.json`:
//!    Entscheid **E-12** des Dirigenten (06.09.2026) erlaubt genau EINEN
//!    Versionsschritt, und der liegt in Etappe H zusammen mit der
//!    Kalibrierung. Zwei Schritte fuer eine Sache waeren zwei Kalibrierungen.
//!
//! ── JEDER AUSGESCHIEDENE TRAEGT EINEN GRUND (M-87) ────────────────────────
//!
//! 🔑 **NAK-213 (08.09.2026).** Bis hierher stand hier das Gegenteil: der
//! harte Deckel aus M-18 schneide „DANACH" — nach der Gewichtung —, und die
//! Abgeschnittenen seien „nicht ausgeschlossen, sondern ueberboten, und die
//! deterministische Rangfolge ist ihr Protokoll". Genau diese Lesart war der
//! Gate-Befund G-D3/A7: eine Rangfolge, in der ein Kandidat gar nicht
//! auftaucht, ist kein Protokoll ueber ihn, sondern sein spurloses
//! Verschwinden — und M-87 sagt woertlich „JEDER Kandidat, der ausscheidet,
//! traegt einen Grund aus einer geschlossenen Menge; ein kommentarlos
//! entfernter Kandidat ist ein Defekt".
//!
//! Deshalb schneidet der Deckel jetzt **vor** Stufe B, auf einer eigenen
//! Screeningordnung aus Stufe-A-Groessen (M-18: 64 Gruppen, paarweise
//! Ueberlappung, Onset-/Peak-Koinzidenz, musikalische Prioritaet), und jeder
//! Abgeschnittene traegt `screening_ueberboten`. „Ueberboten" bleibt die
//! richtige Beschreibung seiner LAGE — sie ist jetzt nur ein Grund im
//! Vertrag statt einer Luecke im Protokoll. Die Liste dieser Gruende wird
//! nie gekappt (R6): ihre Obergrenze ist `SESSION_CLIENT_CAP`, also die Zahl
//! der Quellen, die eine Sitzung ueberhaupt tragen kann.

use super::intent::IntentBestand;
use super::vergleichbarkeit::{GATE_ABDECKUNG, GATE_MINDEST_FENSTER, GATE_ZEITUEBERDECKUNG};

// ═════════════════════════════════════════════════════════════════════════
// Die geschlossenen Mengen des Vertrags
// ═════════════════════════════════════════════════════════════════════════
//
// Jede lebt an EINER Stelle im Vertrag (`eq-ipc-v3.schema.json`), und
// `geschlossene_mengen_des_produzenten_decken_sich_mit_dem_vertrag` haelt
// jede dieser Listen gegen das eingefrorene Schema (M-77). Sie sind hier
// keine zweite Wahrheit, sondern die Rust-Haelfte derselben.

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
    fn braucht(self, f: &Evidenzfenster) -> bool {
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

// ═════════════════════════════════════════════════════════════════════════
// Determinismus (M-25): die vier Groessen, die zur `metrics_version` gehoeren
// ═════════════════════════════════════════════════════════════════════════

/// Die Saat des Block-Bootstraps dieses Moduls.
///
/// Sie ist BENANNT und nicht als Literal im Pfad: §36.4 zaehlt den Seed
/// ausdruecklich zur `metrics_version`, und eine Zahl, die im Rechenweg
/// steht, laesst sich nicht versionieren.
pub const BOOTSTRAP_SAAT: u64 = 20_260_906;

/// Die kanonische Eingangsquantisierung jeder Rangkomponente.
///
/// Ohne sie entschiede die letzte Bitstelle einer Gleitkommasumme ueber die
/// Reihenfolge zweier praktisch gleicher Kandidaten — und M-26 verlangt
/// ausdruecklich, dass bei Gleichstand BEIDE gezeigt werden und die
/// Reihenfolge zwischen zwei Laeufen nicht wechselt.
pub const RANG_QUANTUM: f64 = 1e-6;

/// Der stabile Tie-Break-Key (M-25, M-26).
///
/// Er entscheidet NUR die Anzeigereihenfolge, nie die Auswahl. Der zweite
/// Teil ist die `candidate_source` — eine hex32 und damit total geordnet;
/// zwei Kandidaten koennen sie nicht teilen.
pub const TIE_BREAK_KEY: &str = "rang_quantisiert_absteigend_dann_candidate_source_aufsteigend";

/// Die Reduktionsreihenfolge der sechs Rangkomponenten (§36.2).
///
/// Sie ist Vertragsreihenfolge und zugleich Summenreihenfolge: `rang()`
/// addiert in genau dieser Folge. Eine andere Reihenfolge ergaebe eine andere
/// letzte Bitstelle und damit — ohne `RANG_QUANTUM` — eine andere Rangfolge.
pub const RANGKOMPONENTEN: [&str; 6] = [
    "bandpassung",
    "koinzidenz",
    "uplift",
    "intent_relevanz",
    "wiederholbarkeit",
    "routingqualitaet",
];

/// Die musikalische Prioritaet je Rolle (§36.2 „musikalische Prioritaet").
///
/// Eine ABGELEITETE Tafel ueber der geschlossenen Fuenfermenge aus U22,
/// dieselbe Bauform wie `belegung()` in `NakamaState.cpp`. Sie ist eine
/// Ordnung, kein Messwert: `fuehrt` ist die Quelle, um die es dem User geht,
/// `geschuetzt` die, an der er nichts geaendert haben will. Eine Quelle OHNE
/// Rolle bekommt die Mitte — nicht 0, denn „keine Angabe" heisst nicht
/// „unwichtig" (das waere derselbe Irrtum wie „kein Intent heisst kein
/// Schutz", vor dem E-10 warnt).
pub const ROLLENPRIORITAET: [(&str, f64); 5] = [
    ("fuehrt", 1.0),
    ("traegt", 0.75),
    ("begleitet", 0.5),
    ("verschmolzen", 0.25),
    ("geschuetzt", 0.0),
];

/// Die Prioritaet einer Quelle ohne gesetzte Rolle.
pub const PRIORITAET_OHNE_ROLLE: f64 = 0.5;

/// Der harte Deckel aus M-18: hoechstens fuenf Kandidaten je Befund.
pub const KANDIDATEN_DECKEL: usize = 5;

/// Bandzahl des Evidenzgitters `nakama_1_24_oct_30_18k_v1`.
pub const BAENDER_FEIN: usize = 221;

/// Gruppenzahl des Livegitters `nakama_log64_v1` — das Screening aus M-18
/// arbeitet auf DIESEN 64 Gruppen, nicht auf dem 221er-Gitter.
pub const GRUPPEN_LIVE: usize = 64;

/// Quantisiert einen Wert auf `RANG_QUANTUM` und klemmt ihn in [0, 1].
///
/// ⚠️ Nicht-endliche Werte werden zu 0. NaN-Ehrlichkeit heisst hier: eine
/// Komponente, die nicht gerechnet werden konnte, TRAEGT NICHT BEI — sie
/// vergiftet aber auch nicht die Summe und wird nie zu einer Eins.
pub fn quantisiert(wert: f64) -> f64 {
    if !wert.is_finite() {
        return 0.0;
    }
    let geklemmt = wert.clamp(0.0, 1.0);
    (geklemmt / RANG_QUANTUM).round() * RANG_QUANTUM
}

// ═════════════════════════════════════════════════════════════════════════
// Die Eingangsformen
// ═════════════════════════════════════════════════════════════════════════

/// Ein einzelnes Evidenzfenster, so wie der Empfaenger es abgelegt hat.
///
/// Das ist eine SICHT auf `evidenz::Evidenzstand`, keine zweite Ablage: die
/// Verdrahtung baut sie unter dem Lock und gibt sie hier hinein, damit dieses
/// Modul den Sessiongraphen nicht anhaelt.
#[derive(Debug, Clone, Default, PartialEq)]
pub struct Evidenzfenster {
    pub evidence_id: String,
    /// Die Ankunftsreihenfolge des Brokers — die einzige Ordnung, die ueber
    /// Quellen hinweg vergleichbar ist (M-49-Muster aus SONDE-013).
    pub empfangsfolge: u64,
    pub projekt_von: i64,
    pub projekt_bis: i64,
    pub transport_epoch: u64,
    pub abdeckung: f64,
    pub p50_db: Vec<f32>,
    pub p50_gueltig: Vec<bool>,
    pub onset: f32,
    pub seitenanteil_db: Option<f64>,
    pub hat_baender: bool,
    pub hat_verteilung: bool,
    pub hat_stereo: bool,
}

impl Evidenzfenster {
    /// Der P50 eines Bandes, falls er GEMESSEN ist. Ein Band ohne Bit hat
    /// keinen Wert — nie 0, nie NaN (SONDE-013 M-07).
    pub fn band(&self, index: usize) -> Option<f64> {
        let gueltig = *self.p50_gueltig.get(index)?;
        if !gueltig {
            return None;
        }
        let wert = *self.p50_db.get(index)? as f64;
        wert.is_finite().then_some(wert)
    }
}

/// Alles, was der Rechner ueber EINE Quelle weiss.
#[derive(Debug, Clone, Default, PartialEq)]
pub struct Quellprofil {
    /// `instance_id` als hex32 — die `candidate_source` des Vertrags.
    pub quelle_id: String,
    /// Aufsteigend nach `empfangsfolge`. Die Verdrahtung sortiert; dieses
    /// Modul verlaesst sich darauf und misst es (M-25, Reduktionsreihenfolge).
    pub fenster: Vec<Evidenzfenster>,
    /// Ob das Routing dieser Quelle bekannt ist. `false` heisst „unbekannt"
    /// und deckelt die Aussage (M-22) — es heisst nie „kein Routing".
    pub routing_bekannt: bool,
    /// Der gemeldete Mixerkanal, falls die Quelle einen nennt (NAK-213 R3).
    ///
    /// 🔑 Der Vertrag laesst ihn erst ab **1** zu (`eq-ipc-v3.schema.json`
    /// `minimum: 1`, „Ausserhalb gilt der Hostwert als nicht geliefert").
    /// `None` heisst deshalb GENAU EINES: die Quelle nennt keinen Messpunkt.
    /// Es gibt keinen dritten Zustand und keinen Zahlenwert, der „unbekannt"
    /// bedeutet — `routing_bekannt` bleibt daneben stehen, weil es die
    /// benannte Frage ist, die M-22 stellt.
    pub mixerkanal: Option<i64>,
    /// Die Quelle, in die diese hineinlaeuft, falls bekannt. Sie traegt das
    /// Parent-Duplikat aus M-22.
    pub parent: Option<String>,
    /// Ob fuer diese Quelle ein PRE/POST-Paarurteil vorliegt (§36.1 Klasse 2).
    pub prepost_paar: bool,
    /// Ob ALLE Belege dieser Quelle zurueckgenommen wurden (M-24).
    pub zurueckgenommen: bool,
}

/// Das Fenster der benannten Passage, falls die Sitzung eine fuehrt.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Passagenfenster {
    pub projekt_von: i64,
    pub projekt_bis: i64,
    pub transport_epoch: u64,
}

/// Der vollstaendige Eingang einer Rechnung.
#[derive(Debug, Clone, Default)]
pub struct Aufnahme {
    /// Der Master — die Quelle, deren Befund erklaert werden soll (§8).
    ///
    /// 🔑 **NAK-213 E7/R4:** `None` heisst „die Sitzung hat kein EINDEUTIG
    /// fuehrendes Main". Bis hierher nahm `aufnahmen_sammeln` bei zwei
    /// `main`-Clients schlicht den letzten nach `instance_id` — der erste
    /// verschwand spurlos, und eine Sonde trug einen Befund im Band des
    /// ANDEREN Masters (Gate-Befund G-D4). Entwurf `:1669` sagt „genau ein
    /// fuehrendes Main pro aktiver Sitzung"; ohne das rechnet die Kette
    /// nicht, sondern enthaelt sich MIT GRUND.
    pub master: Option<Quellprofil>,
    /// Die `instance_id` aller `main`-Clients der Sitzung, aufsteigend
    /// sortiert und dedupliziert.
    ///
    /// Sie traegt die Adresse der Enthaltung ohne Ort: `candidate_source` ist
    /// die lexikographisch KLEINSTE — dieselbe stabile Wahl, die
    /// `fuehrung_neu_bewerten_locked` trifft (`mains.sort(); mains[0]`). Sie
    /// behauptet KEINE Fuehrung, sie ist die deterministische Adresse der
    /// Sitzung (M-25).
    pub mains: Vec<String>,
    pub kandidaten: Vec<Quellprofil>,
    pub passage: Option<Passagenfenster>,
    pub passage_id: Option<String>,
    /// Der gespiegelte Intent. `None` heisst „noch keine
    /// Vollstaendigkeitsmarke" — dann rechnet gar nichts (M-86); die Sperre
    /// liegt in der Verdrahtung, dieses Modul misst sie mit.
    pub intent: Option<IntentBestand>,
    /// Die Kalibrierungsfassung, unter der DIESE Rechnung entsteht.
    pub metrics_version: u32,
    /// Die Sitzungsepoche — Teil des Schluessels, aus dem die `finding_id`
    /// entsteht. Zwei Sitzungen erzeugen nie dieselbe ID.
    pub session_epoch: String,
}

// ═════════════════════════════════════════════════════════════════════════
// Die Ausgangsformen
// ═════════════════════════════════════════════════════════════════════════

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

/// `class` UND `score` — zwei Felder. Die Klasse wird NIE aus dem Score
/// gerundet (M-15): sonst waere die harte Gate-Regel aus M-20 durch einen
/// schoenen Mittelwert zu ueberstimmen.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Befundkonfidenz {
    pub klasse: Sicherheitsklasse,
    pub score: f64,
}

/// Die sechs Rangkomponenten aus §36.2, einzeln (§42.4: jeder angezeigte Wert
/// ist auf ein Feld zurueckfuehrbar).
#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Rangkomponenten {
    pub bandpassung: f64,
    pub koinzidenz: f64,
    pub uplift: f64,
    pub intent_relevanz: f64,
    pub wiederholbarkeit: f64,
    pub routingqualitaet: f64,
}

impl Rangkomponenten {
    /// In der Reihenfolge von `RANGKOMPONENTEN` — die Reduktionsreihenfolge.
    pub fn werte(&self) -> [f64; 6] {
        [
            self.bandpassung,
            self.koinzidenz,
            self.uplift,
            self.intent_relevanz,
            self.wiederholbarkeit,
            self.routingqualitaet,
        ]
    }

    /// Der Rangwert: das quantisierte Mittel der sechs.
    ///
    /// ⚠️ GLEICHGEWICHTET, und das ist eine Entscheidung: §36.2 nennt sechs
    /// Komponenten und KEINE Gewichte. Ein erfundenes Gewicht waere eine
    /// unkalibrierte Zahl im Rechenweg — genau das, was `metriken-v1.json`
    /// verhindern soll. Die Kalibrierung gehoert Etappe H.
    pub fn rang(&self) -> f64 {
        let mut summe = 0.0;
        for wert in self.werte() {
            summe += quantisiert(wert);
        }
        quantisiert(summe / RANGKOMPONENTEN.len() as f64)
    }
}

/// Der gemessene Zusammenhang zwischen Kandidat und Master — MIT Vorzeichen.
///
/// 🔑 **NAK-212 R1/R2/E1 (07.09.2026).** Bis hierher lasen Rang und Klasse
/// DISJUNKTE Groessen: die sechs Rangkomponenten messen den Zusammenhang und
/// fliessen ausschliesslich in `confidence.score`, also nur in die
/// REIHENFOLGE; die Klassenwahl las davon keine einzige. Ein Kandidat mit
/// `koinzidenz = uplift = wiederholbarkeit = 0` — also ohne jeden Beleg fuer
/// einen Zusammenhang — erreichte deshalb `hoch`/READY (G5-Befunde G-D1,
/// E-L3, `g5_unbeteiligte_quelle`, `eigen4`).
///
/// ⚠️ **`Option` und nicht 0,0.** `uplift()` gab bisher in DREI verschiedenen
/// Lagen 0,0 zurueck: „kein Vergleichsfenster" (M-19), „Differenz nicht
/// positiv" und „Master steht still". Drei Bedeutungen unter einer Zahl sind
/// genau der Fehler, den SONDE-013 M-07 an anderer Stelle schon verboten hat
/// („ein Band ohne Bit hat keinen Wert — nie 0, nie NaN"). R1 braucht „nicht
/// messbar" getrennt von „gemessen 0", R2 braucht es getrennt von „negativ".
#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Zusammenhangsbeleg {
    /// Bedingter Uplift MIT Vorzeichen, normiert wie die Rangkomponente,
    /// in [-1, 1]. `None` heisst „nicht messbar" (M-19).
    pub uplift: Option<f64>,
    /// Onset-Korrelation MIT Vorzeichen, in [-1, 1]. `None` heisst „keine
    /// Streuung in einer der beiden Reihen" — zwei Konstanten sind kein
    /// Zusammenhang, aber auch kein Gegenbeleg.
    pub koinzidenz: Option<f64>,
    /// Stabilitaet eines POSITIVEN Uplifts, EINSEITIG gemessen, in [0, 1].
    pub wiederholbarkeit: f64,
}

impl Zusammenhangsbeleg {
    /// R1: ist mindestens eine Komponente POSITIV belegt?
    ///
    /// Das ist eine Vorzeichenfrage, keine Groessenfrage — die Grenze null
    /// ist die Definition von „positiv" und keine kalibrierte Schwelle. Sie
    /// steht deshalb nicht in `metriken-v1.json` und ist nicht abstimmbar
    /// (M-15, M-31 bleiben unberuehrt).
    pub fn positiv_belegt(&self) -> bool {
        self.uplift.is_some_and(|u| u > 0.0) || self.koinzidenz.is_some_and(|k| k > 0.0)
    }

    /// R2: ist mindestens eine Komponente ein GEGENBELEG?
    pub fn gegenbeleg(&self) -> bool {
        self.uplift.is_some_and(|u| u < 0.0) || self.koinzidenz.is_some_and(|k| k < 0.0)
    }

    /// Die drei Groessen, quantisiert auf `RANG_QUANTUM` — die Form, in der
    /// zwei Belege verglichen werden (E6). `None` ist ein eigener Wert und
    /// gleich nur sich selbst.
    fn vergleichsform(&self) -> (Option<i64>, Option<i64>, i64) {
        let q = |w: Option<f64>| w.map(|x| (x.clamp(-1.0, 1.0) / RANG_QUANTUM).round() as i64);
        (
            q(self.uplift),
            q(self.koinzidenz),
            (quantisiert(self.wiederholbarkeit) / RANG_QUANTUM).round() as i64,
        )
    }
}

/// Sind zwei Kandidaten in ihrem ZUSAMMENHANG unterscheidbar? (R3, E6)
pub fn zusammenhang_verschieden(a: &Zusammenhangsbeleg, b: &Zusammenhangsbeleg) -> bool {
    a.vergleichsform() != b.vergleichsform()
}

/// Ein ausgeschiedener Kandidat samt Grund (M-87).
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct Ausschluss {
    pub candidate_source: String,
    pub grund: Ausschlussgrund,
}

/// Die drei Zustaende aus Abnahme U21. Die AUTORITATIVE Abbildung
/// Sicherheit → Zustand baut Etappe D; hier steht die strukturelle
/// Vorbelegung, damit der Vertrag (`session_finding.zustand` ist Pflicht)
/// von der ersten Hypothese an erfuellt ist.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Befundzustand {
    ReadyToSend,
    MoreData,
    Stale,
}

pub const BEFUNDZUSTAENDE: [&str; 3] = ["ready_to_send", "more_data", "stale"];

impl Befundzustand {
    pub fn wire(self) -> &'static str {
        match self {
            Self::ReadyToSend => "ready_to_send",
            Self::MoreData => "more_data",
            Self::Stale => "stale",
        }
    }

    pub fn aus_wire(wort: &str) -> Option<Self> {
        Some(match wort {
            "ready_to_send" => Self::ReadyToSend,
            "more_data" => Self::MoreData,
            "stale" => Self::Stale,
            _ => return None,
        })
    }

    /// M-30: **nur** `ready_to_send` erlaubt `HOLD TO AUDITION`.
    ///
    /// Die Sperre haengt am ZUSTAND und damit am Datenweg — nicht an einer
    /// ausgegrauten Schaltflaeche, die trotzdem sendet. Beide Seiten fragen
    /// dieselbe Funktion: hier der Broker, in `SourcesModel::Befund` das
    /// Plugin (M-77).
    pub fn erlaubt_audition(self) -> bool {
        matches!(self, Self::ReadyToSend)
    }

    /// M-30: **nur** `ready_to_send` erlaubt `SEND DRAFT → EQ`.
    pub fn erlaubt_draft(self) -> bool {
        matches!(self, Self::ReadyToSend)
    }
}

/// Die Abbildung Sicherheit → Zustand (M-29).
///
/// ── EINE FUNKTION IM DATENWEG, KEINE ANZEIGEENTSCHEIDUNG ──────────────────
///
/// Abnahme U21 sagt es woertlich: „SICHERHEIT wird nicht als Wert ausgegeben,
/// sondern auf den Zustand des Befunds abgebildet". Die Anzeige liest den
/// Zustand und raet nie — deshalb steht die Abbildung hier, an genau einer
/// Stelle, und nicht in `baue_befund` und `enthaltung` zweimal.
///
/// `veraltet` kommt von aussen und nicht aus der Klasse: ein Befund wird
/// `stale`, weil sich unter ihm etwas GEAENDERT hat (Intent-Revision gestiegen,
/// Belege zurueckgenommen), nicht weil seine Sicherheit gesunken waere. Die
/// beiden Gruende sind verschieden und duerfen nicht ineinander laufen.
pub fn zustand_aus_sicherheit(klasse: Sicherheitsklasse, veraltet: bool) -> Befundzustand {
    if veraltet {
        return Befundzustand::Stale;
    }
    match klasse {
        Sicherheitsklasse::Hoch => Befundzustand::ReadyToSend,
        // `mittel` und `unklar` sind BEIDE nicht handelbar. Wo die numerische
        // Grenze zwischen ihnen liegt, ist Ausgabe des Korpus (M-31, Etappe H)
        // und steht bewusst nirgends als Konstante.
        Sicherheitsklasse::Mittel | Sicherheitsklasse::Unklar => Befundzustand::MoreData,
    }
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
    pub maskierung: Option<super::maskierung::Maskierung>,
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

// ═════════════════════════════════════════════════════════════════════════
// Stufe A — das guenstige Screening (M-18)
// ═════════════════════════════════════════════════════════════════════════

/// Das Ergebnis der Gate-Pruefung EINES Kandidaten.
#[derive(Debug, Clone, PartialEq)]
pub enum Gateurteil {
    /// Er bleibt im Ranking.
    Bleibt,
    /// Er scheidet aus, mit Grund.
    Faellt(Ausschlussgrund),
}

/// Die Bandgruppe des Livegitters, in der eine feine Bandnummer liegt.
///
/// Das Screening arbeitet auf den **64 Gruppen** (M-18: „64 Bänder"), Stufe B
/// auf dem 221er-Verlauf (M-19). Die Zuordnung ist die eingefrorene Partition
/// aus `schemas/v3/bandgitter/nakama_log64_v1.json`; sie wird hier NICHT
/// nachgebaut, sondern als monotone Aufteilung gerechnet, die dieselbe Anzahl
/// Gruppen ergibt. Wer die exakten Kanten braucht, liest das Gitter.
pub fn gruppe_von_band(band: usize) -> usize {
    (band * GRUPPEN_LIVE / BAENDER_FEIN).min(GRUPPEN_LIVE - 1)
}

/// Das feine Bandintervall EINER Gruppe — die echte Umkehrung von
/// `gruppe_von_band`.
///
/// 🔑 **NR-06 (Nacharbeit 1, 07.09.2026).** Bis hierher rechnete die
/// Rueckabbildung `g * 221 / 64` mit derselben abrundenden Division wie die
/// Gruppierung. Das ist keine Umkehrung: `gruppe_von_band` rundet ab, also
/// gilt `g <= b*64/221 < g+1` und damit `g*221/64 <= b < (g+1)*221/64` — die
/// UNTERE Grenze muss deshalb AUFrunden. Mit der abrundenden Form lagen
/// **63 der 221 Baender** ausserhalb des Intervalls ihrer eigenen Gruppe;
/// Band 100 etwa bekam `[96, 100)`, ein halboffenes Intervall, das genau
/// dieses Band ausschliesst. Die Beobachtung stammte dann aus Band 100,
/// waehrend Rang, Maskierung und Proposal-Zielbereich andere Baender lasen —
/// ein Bruch an §1.5 („Die Zone zeigt nur, was der Befund belegt hat") und
/// an M-16 (Ort und Beobachtung).
///
/// `ceil(x/y)` steht als `(x + y - 1) / y`; beide Grenzen rechnen so, und
/// damit gilt fuer JEDES Band `von(gruppe_von_band(b)) <= b < bis(...)`.
pub fn bandintervall_der_gruppe(gruppe: usize) -> Bandintervall {
    let ceil_div = |zaehler: usize| (zaehler + GRUPPEN_LIVE - 1) / GRUPPEN_LIVE;
    let von = ceil_div(gruppe * BAENDER_FEIN);
    let bis = ceil_div((gruppe + 1) * BAENDER_FEIN).min(BAENDER_FEIN);
    // 221/64 ist groesser als 3: zwischen zwei Gruppengrenzen liegen immer
    // mindestens drei Baender. Die Wache steht trotzdem — ein leeres
    // Intervall waere ein Ort ohne Ausdehnung.
    debug_assert!(von < bis, "Gruppe {gruppe} haette ein leeres Intervall");
    Bandintervall {
        von: von as u32,
        bis: bis.max(von + 1) as u32,
    }
}

/// Der Screeningrang aus Stufe-A-Groessen (M-18) — das GUENSTIGE Mass, das
/// entscheidet, WER Stufe B erreicht. Es entscheidet nie eine Klasse.
///
/// 🔑 **NAK-213 E1 (08.09.2026).** Bis hierher lief `rang_und_beleg` fuer
/// JEDEN Gate-Ueberlebenden, und der Deckel schnitt danach — spurlos (M-87
/// gebrochen, Gate-Befund G-D3/A7). M-18 nennt woertlich vier Groessen fuer
/// das Screening, und Entwurf §36.2 (`:2371–2373`) nennt dieselben: die
/// Energie in den **64 Gruppen** des Livegitters, die paarweise Ueberlappung,
/// die Onset-/Peak-Koinzidenz und die musikalische Prioritaet.
///
/// ⚠️ Das ist KEINE siebte Rangkomponente. Die Ordnung entscheidet die
/// AUSWAHL vor Stufe B; `RANGKOMPONENTEN`, `confidence.score` und die
/// Reduktionsreihenfolge bleiben unberuehrt (M-25, Manifest §2.9 Nr. 3).
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Screeningrang {
    /// Anteil der Leistung des Kandidaten in der BEFUNDGRUPPE des 64er-
    /// Livegitters. M-18 woertlich: „64 Baender" — deshalb das Intervall der
    /// GRUPPE (`bandintervall_der_gruppe`) und nicht ein feineres.
    pub gruppenenergie: f64,
    /// Die im Gate bereits gerechnete paarweise Ueberlappung (NAK-212 E4).
    pub alignment: f64,
    /// Die musikalische Prioritaet aus der Rollentafel (M-18).
    pub intent_relevanz: f64,
    /// Die gerichtete Onset-/Peak-Koinzidenz gegen den Master, aus
    /// `koinzidenz_gerichtet` — DERSELBEN Funktion, die die Rangkomponente
    /// bildet, auf DENSELBEN Reihen. `None` (keine Streuung) und der negative
    /// Zweig zaehlen hier wie dort als 0: das Screening ordnet, es widerlegt
    /// nicht.
    pub koinzidenz: f64,
}

impl Screeningrang {
    fn werte(&self) -> [f64; 4] {
        [
            self.gruppenenergie,
            self.alignment,
            self.intent_relevanz,
            self.koinzidenz,
        ]
    }
}

/// Bildet den Screeningrang EINES Kandidaten in der Befundgruppe.
pub fn screeningrang(
    kandidat: &Quellprofil,
    aufnahme: &Aufnahme,
    master: &Quellprofil,
    gruppe: usize,
) -> Screeningrang {
    Screeningrang {
        // Dieselbe Rechnung wie `bandpassung`, aber auf dem Intervall der
        // GRUPPE. Sie ein zweites Mal auszuschreiben waeren zwei
        // Gelegenheiten, sie verschieden zu runden.
        gruppenenergie: bandpassung(kandidat, bandintervall_der_gruppe(gruppe)),
        alignment: paarueberdeckung(&kandidat.fenster, &master.fenster),
        intent_relevanz: intent_relevanz(kandidat, aufnahme),
        koinzidenz: koinzidenz_gerichtet(kandidat, master).unwrap_or(0.0).max(0.0),
    }
}

/// Der Vergleichswert, quantisiert auf `RANG_QUANTUM` (M-25).
///
/// Dieselbe Aufloesung wie der Gesamtrang und dieselbe fail-closed Behandlung
/// nicht-endlicher Werte (`quantisiert`): ein unquantisierter Vergleich
/// liesse die letzte Bitstelle einer Gleitkommasumme entscheiden, wer Stufe B
/// erreicht.
pub fn screening_quantisiert(rang: &Screeningrang) -> i64 {
    let werte = rang.werte();
    let mut summe = 0.0;
    for wert in werte {
        summe += quantisiert(wert);
    }
    (quantisiert(summe / werte.len() as f64) / RANG_QUANTUM).round() as i64
}

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

/// Warum die Konkurrenz dieser Rechnung UNVOLLSTAENDIG ist (R1, R2).
///
/// Jede Variante ist eine benannte Struktur, keine Zahl: M-15 („die Klasse
/// wird nicht aus dem Score gerundet") und M-31 („Schwelle kalibriert, nie
/// geraten") bleiben unberuehrt — dieselbe Lesart, die schon
/// `parent_duplikat` und `routing_bekannt` tragen.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Konkurrenzlage {
    /// Jeder Kandidat der Sitzung wurde vollstaendig gerechnet.
    Vollstaendig,
    /// Am Deckel wurde geschnitten, wo der Screeningrang nicht trennt (E2).
    DeckelOhneAbstand,
    /// Ein Konkurrent ist aus einem MESSGRUND ausgeschieden (R2).
    Messausschluss,
}

/// Bildet die Konkurrenzlage EINER Rechnung — an genau EINER Stelle.
///
/// ⚠️ Die Praezedenz steht hier als eine Zeile und nicht an zwei
/// Aufrufstellen (K-21): `Messausschluss` schlaegt `DeckelOhneAbstand`. Fuer
/// die Klasse ist das gleichgueltig — beide senken auf `mittel` —, fuer den
/// Leser nicht: ein gefallener Konkurrent ist die konkretere Aussage ueber
/// die Datenlage als ein willkuerlicher Schnitt.
pub fn konkurrenzlage(ausschluesse: &[Ausschluss], deckel_ohne_abstand: bool) -> Konkurrenzlage {
    if ausschluesse.iter().any(|a| a.grund.ist_messgrund()) {
        Konkurrenzlage::Messausschluss
    } else if deckel_ohne_abstand {
        Konkurrenzlage::DeckelOhneAbstand
    } else {
        Konkurrenzlage::Vollstaendig
    }
}

// ── Der Nachweis, dass der Deckel VOR Stufe B schneidet (M-18, K-07) ──────
//
// 🔑 M-18 verlangt den Nachweis woertlich als ZAHL: „Ein sechster Kandidat
// erreicht Stufe B" ist der Rotbeweis, und eine Behauptung ohne Zaehler waere
// nicht gemessen (Pruefliste §E). Der Zaehler steht deshalb im Produktcode
// und nicht hinter `cfg(test)`: die messende Zeile K-01 ist ein
// INTEGRATIONSTEST, und der sieht nur, was die Bibliothek auch produktiv
// traegt.
//
// ⚠️ THREAD-LOKAL, nicht global. `cargo test` faehrt die Faelle eines Beins
// PARALLEL im selben Prozess; ein globaler Zaehler zaehlte fremde Rechnungen
// mit und meldete beim ersten Lauf sechs Aufrufe fuer fuenf Kandidaten. Die
// Rechnung laeuft synchron auf dem Thread, der die Evidenz eingereicht hat
// (`evidenz.rs` ruft `hypothesen_bilden` direkt) — die Zahl gehoert deshalb
// diesem Thread. Ein `Cell` ist ausserdem billiger als ein Atomic.
thread_local! {
    static STUFE_B_AUFRUFE: std::cell::Cell<usize> = const { std::cell::Cell::new(0) };
    /// Die groesste Zahl von Stufe-B-Aufrufen INNERHALB EINER Rechnung.
    ///
    /// Die Zusage gilt je RECHNUNG, nicht je Anlass: ein einziger
    /// Evidenzeingang loest im Produktpfad mehrere Rechnungen aus (Paarjoin,
    /// Zustellung, Vorschlaege), und eine absolute Summe waere nicht deutbar.
    static STUFE_B_MAX: std::cell::Cell<usize> = const { std::cell::Cell::new(0) };
}

/// Setzt beide Zaehler dieses Threads zurueck (K-01, K-07).
pub fn stufe_b_zaehler_zuruecksetzen() {
    STUFE_B_AUFRUFE.with(|z| z.set(0));
    STUFE_B_MAX.with(|z| z.set(0));
}

/// Die groesste Zahl von Stufe-B-Aufrufen einer einzelnen Rechnung seit dem
/// Zuruecksetzen — gemessen an den ECHTEN Aufrufen, nicht an der Laenge der
/// Schleife, die sie selbst behauptet.
pub fn stufe_b_max_je_rechnung() -> usize {
    STUFE_B_MAX.with(|z| z.get())
}

/// Die Gesamtzahl der Stufe-B-Aufrufe dieses Threads seit dem Zuruecksetzen.
pub fn stufe_b_aufrufe() -> usize {
    STUFE_B_AUFRUFE.with(|z| z.get())
}

/// Prueft die harten Gates eines Kandidaten — VOR jeder Gewichtung (M-20).
///
/// Reihenfolge ist Absicht und wird gemessen: erst die Belege, dann die
/// Passage, dann der Wille des Users. Ein Kandidat ohne Evidenz braucht
/// keinen Intent-Blick, und ein vom User geschuetzter braucht keine Rechnung.
//
// ⚠️ `!(x >= gate)` und nicht `x < gate`. Der Unterschied ist NaN: `NAN < g`
// ist falsch und liesse einen Kandidaten mit unbrauchbarer Zahl durch,
// `!(NAN >= g)` ist wahr und laesst ihn fallen. Fail-closed ist an einem Gate
// die einzig zulaessige Richtung — deshalb steht die negierte Form hier
// bewusst und mit Erlaubnis, nicht aus Versehen.
#[allow(clippy::neg_cmp_op_on_partial_ord)]
pub fn gate(
    kandidat: &Quellprofil,
    aufnahme: &Aufnahme,
    master: &Quellprofil,
    metrik: Zielmetrik,
    band: Bandintervall,
) -> Gateurteil {
    // 0. Rueckgenommene Evidenz (M-24). Sie steht vor allem anderen: ein
    //    Kandidat ohne gueltigen Beleg hat keine Groesse, die man messen
    //    koennte.
    if kandidat.zurueckgenommen || kandidat.fenster.is_empty() {
        return Gateurteil::Faellt(Ausschlussgrund::EvidenzZurueckgenommen);
    }

    // 1. Der Mixerkanal des fuehrenden Mains (NAK-213 R3).
    //
    // 🔑 Eine Quelle, die den Mixerkanal des Masters misst, misst dessen
    // SIGNAL. Der Befundsatz lautet „X draengt im Bandbereich a..b GEGEN den
    // Master" — ueber sie ist er nicht schwaecher wahr, sondern FALSCH.
    // Deshalb faellt sie aus dem Ranking, statt nur gedeckelt zu werden; ein
    // `mittel`-Befund waere genau die irrefuehrende Behauptung, die der
    // Gate-Text auf jedem Gegenbeispiel verbietet (M-69).
    //
    // ⚠️ Die Stelle ist Absicht und wird gemessen: NACH der Evidenz, VOR der
    // Capability. Eine STUMME Sonde auf dem Masterkanal faellt weiter mit
    // `evidenz_zurueckgenommen` — sie dupliziert nichts. Dass eine Quelle das
    // Mastersignal misst, ist danach eine Frage ueber die Quelle selbst und
    // braucht keine Messguete-Pruefung.
    if let (Some(kanal), Some(master_kanal)) = (kandidat.mixerkanal, master.mixerkanal) {
        if kanal == master_kanal {
            return Gateurteil::Faellt(Ausschlussgrund::MasterDuplikat);
        }
    }

    // 2. Capability: traegt der Beleg ueberhaupt die Datenklasse der Metrik?
    if !kandidat.fenster.iter().any(|f| metrik.braucht(f)) {
        return Gateurteil::Faellt(Ausschlussgrund::CapabilityFehlt);
    }

    // 3. Coverage (M-20). Das MITTEL ueber die Fenster, nicht das Maximum:
    //    ein einziges dichtes Fenster macht eine duenne Messung nicht dicht.
    let abdeckung = mittel(kandidat.fenster.iter().map(|f| f.abdeckung));
    if !(abdeckung >= GATE_ABDECKUNG as f64) {
        return Gateurteil::Faellt(Ausschlussgrund::CoverageFehlt);
    }

    // 4. Alignment (M-20): liegen die Fenster des Kandidaten wirklich dort,
    //    wo der Master seinen Befund hat?
    //
    // 🔑 **NAK-212 R4/E4 (07.09.2026).** Gemessen wird die PAARWEISE
    // Ueberlappung, nicht die Ueberdeckung der beiden SPANNEN. `spanne` ist
    // ein lueckenblindes min/max: Sondenfenster 20 000 Samples VOR und 40 000
    // Samples NACH den Masterfenstern umklammern deren Spanne vollstaendig,
    // `zeitueberdeckung` meldete 1,0 — und das Gate hielt, obwohl KEIN
    // einziges Fenster ein gemeinsames Sample mit dem Master hat (G5-Befund
    // E-D4). Die Schwelle bleibt dieselbe.
    let alignment = paarueberdeckung(&kandidat.fenster, &master.fenster);
    if !(alignment >= GATE_ZEITUEBERDECKUNG) {
        return Gateurteil::Faellt(Ausschlussgrund::AlignmentFalsch);
    }

    // 5. Die Passage, falls die Sitzung eine fuehrt (M-23).
    if let Some(p) = aufnahme.passage {
        // 5a. VERSCHOBEN oder ANDERS: eine andere Transportepoche ist eine
        //     andere Passage, und eine verschobene reisst die Zeitueberdeckung.
        let ueberdeckung = zeitueberdeckung(
            spanne(&kandidat.fenster),
            (p.projekt_von, p.projekt_bis),
        );
        if kandidat.fenster.iter().any(|f| f.transport_epoch != p.transport_epoch)
            || !(ueberdeckung >= GATE_ZEITUEBERDECKUNG)
        {
            return Gateurteil::Faellt(Ausschlussgrund::PassageUnvergleichbar);
        }
        // 5b. ZU KURZ (R1): die absolute Groesse, die die vier relativen
        //     Gates nicht messen koennen. Gezaehlt werden UNABHAENGIGE
        //     Fenster — zwei Belege desselben Projektfensters sind einer.
        //
        // 🔑 NR-05 (Nacharbeit 1): JE BETEILIGTER Quelle, also auch fuer den
        // Master. Bis hierher zaehlte nur der Kandidat; acht seiner Fenster
        // gegen ein einziges ueberlappendes Masterfenster bestanden alle vier
        // relativen Gates, weil `ueberdeckung` auf das kuerzere Intervall
        // normiert — und ein einzelner Kandidat erreichte `hoch`/READY.
        if passagenmaterial(kandidat, master, &p) < GATE_MINDEST_FENSTER {
            return Gateurteil::Faellt(Ausschlussgrund::PassageZuKurz);
        }
    }

    // 6. Der Wille des Users (M-03, M-04). Er steht am Ende, weil er die
    //    Rechnung nicht braucht — aber er ueberstimmt sie vollstaendig.
    if let Some(intent) = aufnahme.intent.as_ref() {
        let passage_id = aufnahme.passage_id.clone().unwrap_or_default();
        if intent.schutz_verletzt(&kandidat.quelle_id, band.von, band.bis) {
            return Gateurteil::Faellt(Ausschlussgrund::IntentVetoGeschuetzt);
        }
        if intent
            .wirkend(&kandidat.quelle_id, &passage_id)
            .is_some_and(|s| s.rolle == "geschuetzt")
        {
            return Gateurteil::Faellt(Ausschlussgrund::IntentVetoGeschuetzt);
        }
        if !intent.entmaskierung_erlaubt(&master.quelle_id, &kandidat.quelle_id, &passage_id)
        {
            return Gateurteil::Faellt(Ausschlussgrund::IntentVetoVerschmolzen);
        }
    }

    Gateurteil::Bleibt
}

// ═════════════════════════════════════════════════════════════════════════
// Stufe B — die fokussierte Evidenz (M-19, M-20)
// ═════════════════════════════════════════════════════════════════════════

/// Die sechs Rangkomponenten eines Kandidaten.
///
/// Jede ist eine eigene Groesse in [0, 1] und wird einzeln gerechnet — keine
/// entsteht aus einer anderen. Genau deshalb kann M-20 messen, dass keine die
/// andere ausgleicht.
/// ⚠️ Die `Zielmetrik` geht hier bewusst NICHT ein. Sie waehlt im Gate, welche
/// Datenklasse ein Beleg tragen muss (`Zielmetrik::braucht`); die sechs
/// Rangkomponenten sind fuer alle fuenf Metriken dieselben Groessen. Eine
/// metrikspezifische Gewichtung waere eine Kalibrierung, und die gehoert
/// Etappe H — hier waere sie eine unbelegte Zahl im Rechenweg.
pub fn rangkomponenten(
    kandidat: &Quellprofil,
    aufnahme: &Aufnahme,
    master: &Quellprofil,
    band: Bandintervall,
    parent_duplikat: bool,
) -> Rangkomponenten {
    rang_und_beleg(kandidat, aufnahme, master, band, parent_duplikat).0
}

/// Rang UND Zusammenhangsbeleg aus EINER Rechnung (E1).
///
/// ⚠️ Die Rangkomponenten `koinzidenz` und `uplift` bleiben in [0, 1]: ein
/// Gegenbeleg wird fuer den RANG auf 0 geklemmt und wirkt ausschliesslich
/// ueber die Klasse (R2). Damit bleiben Zahl, Name, Wertebereich und
/// Reduktionsreihenfolge der sechs Komponenten unveraendert — M-25 und der
/// Vertrag sind unberuehrt.
pub fn rang_und_beleg(
    kandidat: &Quellprofil,
    aufnahme: &Aufnahme,
    master: &Quellprofil,
    band: Bandintervall,
    parent_duplikat: bool,
) -> (Rangkomponenten, Zusammenhangsbeleg) {
    // M-18/K-07: der Nachweis, dass Stufe B hoechstens `KANDIDATEN_DECKEL`-mal
    // je Rechnung laeuft, ist eine ZAHL — hier entsteht sie.
    STUFE_B_AUFRUFE.with(|z| z.set(z.get().saturating_add(1)));
    let beleg = zusammenhang(kandidat, master, band);
    let rang = Rangkomponenten {
        bandpassung: bandpassung(kandidat, band),
        koinzidenz: beleg.koinzidenz.unwrap_or(0.0).max(0.0),
        uplift: beleg.uplift.unwrap_or(0.0).max(0.0),
        intent_relevanz: intent_relevanz(kandidat, aufnahme),
        wiederholbarkeit: beleg.wiederholbarkeit,
        routingqualitaet: routingqualitaet(kandidat, parent_duplikat),
    };
    (rang, beleg)
}

/// Wie gut die Energie des Kandidaten im Befundband sitzt.
///
/// Der Anteil seiner LINEAREN Leistung im Bandintervall an seiner
/// Gesamtleistung. In dB zu mitteln waere ein geometrisches Mittel und
/// unterschaetzte genau die Spitzen, um die es hier geht.
fn bandpassung(kandidat: &Quellprofil, band: Bandintervall) -> f64 {
    let mut im_band = 0.0f64;
    let mut gesamt = 0.0f64;
    for fenster in &kandidat.fenster {
        for index in 0..BAENDER_FEIN {
            let Some(db) = fenster.band(index) else {
                continue;
            };
            let leistung = 10f64.powf(db / 10.0);
            if !leistung.is_finite() {
                continue;
            }
            gesamt += leistung;
            if index >= band.von as usize && index < band.bis as usize {
                im_band += leistung;
            }
        }
    }
    if gesamt <= 0.0 {
        return 0.0;
    }
    im_band / gesamt
}

/// Zeitliche Koinzidenz der Onsetspuren von Kandidat und Master, MIT
/// Vorzeichen.
///
/// Die Pearson-Korrelation der beiden Onsetreihen ueber die gemeinsamen
/// Fenster. `None`, wenn eine der beiden Reihen keine Streuung hat — zwei
/// Konstanten sind kein Zusammenhang, aber auch kein Gegenbeleg, und die
/// beiden Lagen muessen fuer R1 und R2 unterscheidbar bleiben.
///
/// ⚠️ Die RANGKOMPONENTE klemmt den negativen Zweig weiterhin auf 0
/// (`rang_und_beleg`): ein Gegenbeleg darf keinen Rang erzeugen. Er wirkt
/// ueber die KLASSE (R2), nicht ueber die Reihenfolge — sonst aenderte sich
/// der Wertebereich einer Rangkomponente und mit ihm die Reduktionsreihen-
/// folge aus M-25.
fn koinzidenz_gerichtet(kandidat: &Quellprofil, master: &Quellprofil) -> Option<f64> {
    let (a, b) = gemeinsame_reihen(kandidat, master, |f| f.onset as f64);
    korrelation_gerichtet(&a, &b)
}

/// Der BEDINGTE Uplift (M-19): die Zielmetrik des Masters in Fenstern MIT
/// aktivem Kandidaten gegen aehnliche Fenster OHNE diese Aktivitaet.
///
/// ⚠️ Ohne Vergleichsfenster gibt es keinen Uplift — nicht 0, sondern gar
/// keinen. Genau daran faellt der Rotbeweis von M-19: wer den Uplift ohne die
/// Menge „ohne die Quelle" rechnet, misst nur den Pegel des Masters.
fn uplift_gerichtet(
    kandidat: &Quellprofil,
    master: &Quellprofil,
    band: Bandintervall,
) -> Option<f64> {
    // Lage 1 und 2 (M-19): weniger als zwei gemeinsame Fenster, oder kein
    // Band mit Gueltigkeitsbit im Befundintervall.
    let (mit, ohne) = upliftreihe(kandidat, master, band)?;
    // Lage 3: alle Fenster liegen auf EINER Seite des eigenen Medians — es
    // gibt keine Vergleichsmenge „ohne die Quelle". Genau daran faellt der
    // Rotbeweis von M-19: wer den Uplift ohne diese Menge rechnet, misst nur
    // den Pegel des Masters.
    if mit.is_empty() || ohne.is_empty() {
        return None;
    }
    let differenz = mittel(mit.iter().copied()) - mittel(ohne.iter().copied());
    if !differenz.is_finite() {
        return None;
    }
    // Normiert auf die eigene Spanne des Masters in diesem Band: eine
    // Erhoehung um 3 dB heisst wenig, wenn der Master dort ohnehin um 20 dB
    // schwankt, und viel, wenn er still steht. Eine feste dB-Schwelle waere
    // eine unkalibrierte Konstante.
    let alle: Vec<f64> = mit.iter().chain(ohne.iter()).copied().collect();
    let spanne = spannweite(&alle);
    if spanne <= 0.0 {
        // Der Master steht in beiden Mengen still. Die Differenz ist dann
        // nicht normierbar — das ist keine Null, sondern keine Messung.
        return None;
    }
    Some((differenz / spanne).clamp(-1.0, 1.0))
}

/// Die beiden Messreihen des Masters, aufgeteilt danach, ob der Kandidat im
/// jeweiligen Fenster ueber seinem eigenen Median lag.
///
/// „Aktiv" ist damit eine Aussage ueber den Kandidaten selbst und keine
/// absolute Pegelgrenze — die waere wieder eine unkalibrierte Zahl.
fn upliftreihe(
    kandidat: &Quellprofil,
    master: &Quellprofil,
    band: Bandintervall,
) -> Option<(Vec<f64>, Vec<f64>)> {
    let (kandidatenreihe, masterreihe) =
        gemeinsame_reihen(kandidat, master, |f| bandmittel(f, band).unwrap_or(f64::NAN));
    if kandidatenreihe.len() < 2 {
        return None;
    }
    let endlich: Vec<f64> = kandidatenreihe.iter().copied().filter(|x| x.is_finite()).collect();
    if endlich.is_empty() {
        return None;
    }
    let schwelle = median(&endlich);
    let mut mit = Vec::new();
    let mut ohne = Vec::new();
    for (k, m) in kandidatenreihe.iter().zip(masterreihe.iter()) {
        if !k.is_finite() || !m.is_finite() {
            continue;
        }
        if *k >= schwelle {
            mit.push(*m);
        } else {
            ohne.push(*m);
        }
    }
    Some((mit, ohne))
}

/// Die musikalische Prioritaet der Quelle aus dem Intent (§36.2, §37.3).
fn intent_relevanz(kandidat: &Quellprofil, aufnahme: &Aufnahme) -> f64 {
    let Some(intent) = aufnahme.intent.as_ref() else {
        return PRIORITAET_OHNE_ROLLE;
    };
    let passage_id = aufnahme.passage_id.clone().unwrap_or_default();
    let Some(spiegel) = intent.wirkend(&kandidat.quelle_id, &passage_id) else {
        return PRIORITAET_OHNE_ROLLE;
    };
    ROLLENPRIORITAET
        .iter()
        .find(|(name, _)| *name == spiegel.rolle)
        .map(|(_, wert)| *wert)
        .unwrap_or(PRIORITAET_OHNE_ROLLE)
}

/// Stabilitaet des Uplifts ueber einen BLOCK-Bootstrap (M-19).
///
/// Ein Bootstrap ueber Einzelwerte naehme Unabhaengigkeit an, die es zwischen
/// benachbarten Analysefenstern nicht gibt — dieselbe Begruendung wie in
/// `experiment.rs::block_bootstrap`. Der Rueckgabewert ist `1 - p`: eine
/// Reihe, deren Mittel je nach Ziehung das Vorzeichen wechselt, ist keine
/// Aussage, so gross ihr Mittelwert auch sein mag.
fn wiederholbarkeit(kandidat: &Quellprofil, master: &Quellprofil, band: Bandintervall) -> f64 {
    let Some((mit, ohne)) = upliftreihe(kandidat, master, band) else {
        return 0.0;
    };
    if mit.is_empty() || ohne.is_empty() {
        return 0.0;
    }
    let basis = mittel(ohne.iter().copied());
    if !basis.is_finite() {
        return 0.0;
    }
    let reihe: Vec<f64> = mit.iter().map(|x| x - basis).collect();
    // 🔑 NAK-212 R2: EINSEITIG. `bootstrap_p` nimmt das Minimum aus beiden
    // Seiten und gibt einer stabil NEGATIVEN Reihe denselben kleinen p-Wert
    // wie einer stabil positiven — der Gegenbeleg wurde nicht ignoriert,
    // sondern BELOHNT (G-D5: `eigen3` erreichte 0,995 und Rang 0,4358).
    let p = super::experiment::bootstrap_p_positiv(
        &reihe,
        super::experiment::BOOTSTRAP_BLOCK,
        super::experiment::BOOTSTRAP_ZIEHUNGEN,
        BOOTSTRAP_SAAT,
    );
    (1.0 - p).clamp(0.0, 1.0)
}

/// Der vollstaendige Zusammenhangsbeleg eines Kandidaten (R1, R2, E1).
///
/// EINE Rechnung fuer drei Verwendungen: die Rangkomponenten (geklemmt), die
/// Klassenwahl (Vorzeichen) und die Trennung (E6). Drei getrennte Aufrufe
/// waeren drei Gelegenheiten, dieselbe Groesse verschieden zu rechnen.
pub fn zusammenhang(
    kandidat: &Quellprofil,
    master: &Quellprofil,
    band: Bandintervall,
) -> Zusammenhangsbeleg {
    Zusammenhangsbeleg {
        uplift: uplift_gerichtet(kandidat, master, band),
        koinzidenz: koinzidenz_gerichtet(kandidat, master),
        wiederholbarkeit: wiederholbarkeit(kandidat, master, band),
    }
}

/// Routingqualitaet (M-20, M-22).
///
/// Drei Stufen, und jede hat eine Folge: unbekanntes Routing traegt NIE eine
/// starke Aussage, ein Parent-Duplikat ebenfalls nicht — sonst behaupteten
/// Bus und Parent zweimal dieselbe Ursache.
fn routingqualitaet(kandidat: &Quellprofil, parent_duplikat: bool) -> f64 {
    if !kandidat.routing_bekannt {
        return 0.0;
    }
    if parent_duplikat {
        return 0.5;
    }
    1.0
}

// ═════════════════════════════════════════════════════════════════════════
// Die Hypothese
// ═════════════════════════════════════════════════════════════════════════

/// Was `hypothesen` zurueckgibt: die Befunde und die Ausschluesse.
#[derive(Debug, Clone, Default, PartialEq)]
pub struct Rechenergebnis {
    pub befunde: Vec<CauseHypothesis>,
    /// Alle ausgeschiedenen Kandidaten, sortiert. Sie stehen zusaetzlich am
    /// FUEHRENDEN Befund, damit sie mit ihm reisen (M-87).
    pub ausschluesse: Vec<Ausschluss>,
}

/// Die vollstaendige Rechnung: Screening, Gates, Rang, Hypothesen.
///
/// Rueckgabe ist eine Liste, weil Abnahme U21 Alternativen als EIGENE Befunde
/// mit eigenem Zustand fuehrt. Der erste Eintrag ist der fuehrende Befund;
/// seine `alternatives` tragen die IDs der uebrigen.
pub fn hypothesen(aufnahme: &Aufnahme) -> Rechenergebnis {
    // M-86: ohne Vollstaendigkeitsmarke wird nicht gerechnet. Ein fehlender
    // Intent saehe aus wie „kein Schutz gewuenscht" — der teuerste Irrtum,
    // den dieser Datenweg machen kann.
    //
    // 🔑 NR-01 (Nacharbeit 1, 07.09.2026): derselbe Riegel wie in
    // `aufnahmen_sammeln`, aus DERSELBEN Funktion. Zwei Kopien einer
    // Bedingung sind zwei Gelegenheiten, sie verschieden zu formulieren —
    // und genau das war passiert: `None` sperrte an keiner der beiden
    // Stellen.
    if !super::intent::darf_gerechnet_werden(aufnahme.intent.as_ref()) {
        return Rechenergebnis::default();
    }

    // 🔑 **NAK-213 E7/R4: DREI Lagen statt zwei.**
    //
    // (1) Kein eindeutig fuehrendes Main → Enthaltung OHNE ORT. Bis hierher
    //     nahm die Aufnahme den letzten `main` nach `instance_id`, und der
    //     erste verschwand spurlos (G-D4).
    // (2) Fuehrendes Main ohne verwertbaren Beleg → ebenfalls Enthaltung ohne
    //     Ort. Bis hierher gab `masteranomalie` `None`, die Rechnung endete
    //     vor den Gates, und die Sitzung SCHWIEG (G-L4) — M-27 verlangt aber
    //     ein Ergebnis.
    // (3) Sonst: die Kette rechnet wie bisher, Zeile fuer Zeile.
    //
    // ⚠️ Beide Enthaltungen setzen nach R8 mindestens EINEN gueltigen Beleg
    // der Sitzung voraus; `enthaltung_ohne_ort` gibt sonst `None`, und dann
    // entsteht gar kein Befund.
    let Some(master) = aufnahme.master.as_ref() else {
        return Rechenergebnis {
            befunde: enthaltung_ohne_ort(aufnahme, Enthaltungsgrund::KeineEindeutigeFuehrung)
                .into_iter()
                .collect(),
            ausschluesse: Vec::new(),
        };
    };
    let Some((metrik, band, beobachtung, gruppe)) =
        masteranomalie(master, aufnahme.passage.as_ref())
    else {
        return Rechenergebnis {
            befunde: enthaltung_ohne_ort(aufnahme, Enthaltungsgrund::KeinMasterbeleg)
                .into_iter()
                .collect(),
            ausschluesse: Vec::new(),
        };
    };

    // Parent-Duplikate erkennen, BEVOR gerangt wird (M-22).
    //
    // 🔑 **NAK-213 E6:** die Mengen tragen den MASTER mit. Bis hierher lasen
    // beide ausschliesslich `aufnahme.kandidaten` — eine Sonde, deren Parent
    // der Master ist, bekam deshalb `duplikat = false`, volle
    // Routingqualitaet und `hoch` (Gate-Befund E-D1/A3).
    let ids: std::collections::BTreeSet<&str> = aufnahme
        .kandidaten
        .iter()
        .chain(std::iter::once(master))
        .map(|k| k.quelle_id.as_str())
        .collect();
    let ist_parent = |k: &Quellprofil| {
        aufnahme
            .kandidaten
            .iter()
            .chain(std::iter::once(master))
            .any(|anderer| anderer.parent.as_deref() == Some(k.quelle_id.as_str()))
    };

    let mut ausschluesse: Vec<Ausschluss> = Vec::new();
    // Schritt 1: die Gates. Wer faellt, faellt mit Grund (M-87).
    //
    // ⚠️ Die Duplikatmarke steht VOR dem Screening und geht NICHT in es ein:
    // sie wirkt ueber `routingqualitaet` in Stufe B, und wuerde sie die
    // Screeningordnung verschieben, entschiede eine Aussage ueber das Routing
    // darueber, WER ueberhaupt gerechnet wird.
    let mut gate_ueberlebende: Vec<(&Quellprofil, bool)> = Vec::new();
    for kandidat in &aufnahme.kandidaten {
        match gate(kandidat, aufnahme, master, metrik, band) {
            Gateurteil::Faellt(grund) => ausschluesse.push(Ausschluss {
                candidate_source: kandidat.quelle_id.clone(),
                grund,
            }),
            Gateurteil::Bleibt => {
                // Ein Parent ist genau dann ein Duplikat, wenn sein Child
                // ebenfalls im Rennen ist. Das Child kennt sein Parent, nicht
                // umgekehrt — deshalb die Suche ueber die Kandidatenliste.
                let duplikat = ist_parent(kandidat)
                    || kandidat
                        .parent
                        .as_deref()
                        .is_some_and(|p| ids.contains(p));
                gate_ueberlebende.push((kandidat, duplikat));
            }
        }
    }

    // 🔑 Schritt 2: das SCREENING (M-18, NAK-213 E1). Der Deckel schneidet
    // HIER — vor Stufe B — und jeder Abgeschnittene traegt
    // `screening_ueberboten`. Die Ordnung ist deterministisch: quantisierter
    // Screeningrang absteigend, dann `candidate_source` aufsteigend
    // (derselbe `TIE_BREAK_KEY` wie die Anzeigeordnung).
    let mut gescreent: Vec<(i64, &Quellprofil, bool)> = gate_ueberlebende
        .iter()
        .map(|(kandidat, duplikat)| {
            (
                screening_quantisiert(&screeningrang(kandidat, aufnahme, master, gruppe)),
                *kandidat,
                *duplikat,
            )
        })
        .collect();
    gescreent.sort_by(|a, b| b.0.cmp(&a.0).then_with(|| a.1.quelle_id.cmp(&b.1.quelle_id)));

    // E2: verglichen wird der FUENFTE mit dem SECHSTEN — nicht der Fuehrende
    // mit dem Sechsten. Sind sie quantengleich, entschiede der Tie-Break, WER
    // gar nicht erst gerechnet wird; M-18 laesst den Deckel trotzdem hart
    // schneiden, aber M-26 verbietet, dass daraus eine starke Aussage wird.
    let deckel_ohne_abstand = gescreent.len() > KANDIDATEN_DECKEL
        && gescreent[KANDIDATEN_DECKEL - 1].0 == gescreent[KANDIDATEN_DECKEL].0;
    for (_, kandidat, _) in gescreent.iter().skip(KANDIDATEN_DECKEL) {
        ausschluesse.push(Ausschluss {
            candidate_source: kandidat.quelle_id.clone(),
            grund: Ausschlussgrund::ScreeningUeberboten,
        });
    }
    gescreent.truncate(KANDIDATEN_DECKEL);

    ausschluesse.sort();
    ausschluesse.dedup();
    let lage = konkurrenzlage(&ausschluesse, deckel_ohne_abstand);

    // 🔑 NAK-212 E1: der Zusammenhangsbeleg reist MIT. Klassenwahl (R1, R2)
    // und Trennung (R3, E6) lesen ihn; er wird EINMAL gerechnet.
    //
    // Schritt 3: Stufe B, fuer die verbliebenen hoechstens fuenf.
    let vor_stufe_b = STUFE_B_AUFRUFE.with(|z| z.get());
    let mut ueberlebende: Vec<(Rangkomponenten, Zusammenhangsbeleg, &Quellprofil, bool)> =
        gescreent
            .iter()
            .map(|(_, kandidat, duplikat)| {
                let (rang, beleg) = rang_und_beleg(kandidat, aufnahme, master, band, *duplikat);
                (rang, beleg, *kandidat, *duplikat)
            })
            .collect();
    // M-18/K-07: die Aufrufe DIESER Rechnung, gemessen an den echten
    // Aufrufen des Zaehlers — nicht an der Laenge der Schleife.
    let dieser_lauf = STUFE_B_AUFRUFE.with(|z| z.get()).saturating_sub(vor_stufe_b);
    STUFE_B_MAX.with(|z| z.set(z.get().max(dieser_lauf)));

    // Schritt 4: die ANZEIGEORDNUNG (M-25) — quantisierter Gesamtrang
    // absteigend, dann `candidate_source` aufsteigend. Die AUSWAHL hat das
    // Screening getroffen; hier wird nichts mehr abgeschnitten.
    ueberlebende.sort_by(|a, b| {
        let ra = rang_quantisiert(&a.0);
        let rb = rang_quantisiert(&b.0);
        rb.cmp(&ra).then_with(|| a.2.quelle_id.cmp(&b.2.quelle_id))
    });

    if ueberlebende.is_empty() {
        // M-27: kein Kandidat ist ein ERGEBNIS. Der Befund traegt die siebte
        // Ursachenklasse, keinen Fehlercode — und er bleibt sichtbar, damit
        // der User sieht, dass gerechnet wurde.
        let befund = enthaltung(aufnahme, master, metrik, band, beobachtung, ausschluesse.clone());
        return Rechenergebnis {
            befunde: vec![befund],
            ausschluesse,
        };
    }

    let mehrere = ueberlebende.len() > 1;

    // ⚠️ GETRENNT HEISST BEIDES ZUGLEICH — UND GEGEN JEDEN (NAK-212 E6).
    //
    // (1) Der quantisierte GESAMTRANG muss verschieden sein. Verschiedene
    //     quantisierte Komponenten koennen denselben Rang ergeben, weil
    //     `rang()` durch sechs teilt und ERNEUT quantisiert: Koinzidenz
    //     0,500000 gegen 0,500001 liefert beide Male 0,35. Ohne diese
    //     Bedingung entschiede der Tie-Break (aufsteigende
    //     `candidate_source`) darueber, WELCHER Kandidat stark wird — und
    //     M-26 sagt ausdruecklich, er entscheide „nur die Anzeigereihenfolge,
    //     nicht die Auswahl".
    //
    // (2) Mindestens eine ZUSAMMENHANGSKOMPONENTE muss verschieden sein.
    //     Bandpassung und Intent-Relevanz gehen in den Rang ein, tragen aber
    //     keine Ursachenbehauptung: 0,1 dB in einem Band weit ausserhalb des
    //     Befundintervalls trennten zwei Kandidaten um 321 Quanten (G-H3),
    //     und `rolle = fuehrt` auf dem Distraktor erklaerte messtechnisch
    //     identische Quellen fuer getrennt (A1).
    //
    // (3) Der Fuehrende wird gegen JEDEN anderen Ueberlebenden geprueft, nicht
    //     nur gegen den Zweitplatzierten. Ein Kandidat mit abweichender
    //     Intent-Relevanz kann sich zwischen zwei messtechnisch identische
    //     schieben; der Vergleich nur mit dem Nachbarn uebersieht das.
    //
    // Ein EINZIGER Ueberlebender bleibt getrennt — er hat niemanden, von dem
    // er sich abheben muesste. Die Lage „allein im Rennen neben einem
    // Messausschluss" ist ein eigener Befund und gehoert NAK-213.
    let getrennt = match ueberlebende.split_first() {
        Some((erster, weitere)) => weitere
            .iter()
            .all(|anderer| getrennt((&erster.0, &erster.1), (&anderer.0, &anderer.1))),
        None => true,
    };

    let mut befunde: Vec<CauseHypothesis> = ueberlebende
        .iter()
        .enumerate()
        .map(|(platz, (rang, beleg, kandidat, duplikat))| {
            baue_befund(
                aufnahme,
                master,
                kandidat,
                *rang,
                beleg,
                metrik,
                band,
                beobachtung,
                *duplikat,
                mehrere,
                lage,
                // Nur der FUEHRENDE traegt die Trennungsfrage; die uebrigen
                // fallen ohnehin am Riegel darunter auf `mittel` (M-21).
                platz == 0 && getrennt,
            )
        })
        .collect();

    // ⚠️ NUR DER FUEHRENDE BEFUND DARF STARK SEIN (M-21).
    //
    // „Der Distraktor erzeugt KEINE starke Ursachenbehauptung. Er darf als
    // Alternative erscheinen." Genau das ist die Lage hier: die Alternativen
    // sind Kandidaten, die das Ranking NICHT als Ursache ausgewiesen hat —
    // bei gleichem Material sogar solche, die es gar nicht unterscheiden
    // konnte. Zwei starke Behauptungen ueber dieselbe Ursache sind dieselbe
    // Klasse Fehler wie das Parent-Duplikat aus M-22.
    //
    // Gefunden hat das der P5-Korpus (Etappe H): die Sitzung
    // `korrelierter_distraktor` lieferte zwei Kandidaten mit `hoch`, und
    // genau das ist die falsche starke Behauptung, die §36.4 Satz 1 verbietet.
    for befund in befunde.iter_mut().skip(1) {
        if befund.confidence.klasse >= Sicherheitsklasse::Hoch {
            befund.confidence.klasse = Sicherheitsklasse::Mittel;
            befund.zustand = zustand_aus_sicherheit(Sicherheitsklasse::Mittel, false);
        }
    }

    // Der fuehrende Befund traegt die IDs der uebrigen als Alternativen und
    // die Ausschluesse (M-21, M-87). Die uebrigen sind EIGENE Befunde mit
    // eigenem Zustand — nicht sein Anhang.
    let alle_ids: Vec<String> = befunde.iter().map(|b| b.finding_id.clone()).collect();
    // 🔑 NAK-212 R3: „jeder Alternative des anderen". Bis hierher trug NUR der
    // fuehrende Befund Alternativen; ein ungetrennter zweiter Platz zeigte auf
    // niemanden und sah aus wie ein Befund ohne Konkurrenz.
    for (platz, befund) in befunde.iter_mut().enumerate() {
        befund.alternatives = alle_ids
            .iter()
            .enumerate()
            .filter(|(i, _)| *i != platz)
            .map(|(_, id)| id.clone())
            .collect();
    }
    befunde[0].ausschluesse = ausschluesse.clone();

    Rechenergebnis {
        befunde,
        ausschluesse,
    }
}

/// Sind zwei Kandidaten GETRENNT? (R3, E6 — beides zugleich.)
///
/// (1) Der quantisierte GESAMTRANG muss verschieden sein. Verschiedene
///     quantisierte Komponenten koennen denselben Rang ergeben, weil `rang()`
///     durch sechs teilt und ERNEUT quantisiert: Koinzidenz 0,500000 gegen
///     0,500001 liefert beide Male 0,35, und ein Uplift von 0,000001 gegen
///     `None` beide Male 0,266667. Ohne diese Bedingung entschiede der
///     Tie-Break (aufsteigende `candidate_source`) darueber, WELCHER Kandidat
///     stark wird — und M-26 sagt, er entscheide „nur die Anzeigereihenfolge,
///     nicht die Auswahl".
///
/// (2) Mindestens eine ZUSAMMENHANGSKOMPONENTE muss verschieden sein.
///     Bandpassung, Routingqualitaet und Intent-Relevanz gehen in den Rang
///     ein, tragen aber keine Ursachenbehauptung: 0,1 dB in einem Band weit
///     ausserhalb des Befundintervalls trennten zwei Kandidaten um 321
///     Quanten (G-H3), und `rolle = fuehrt` auf dem Distraktor erklaerte
///     messtechnisch identische Quellen fuer getrennt (A1).
///
/// Die Funktion steht BENANNT und nicht als Ausdruck in `hypothesen`, weil
/// eine Regel, die nur inline existiert, keinen isolierten Rotbeweis hat:
/// beide Bedingungen muessen einzeln zuruecknehmbar und einzeln messbar sein.
pub fn getrennt(
    a: (&Rangkomponenten, &Zusammenhangsbeleg),
    b: (&Rangkomponenten, &Zusammenhangsbeleg),
) -> bool {
    rang_quantisiert(a.0) != rang_quantisiert(b.0) && zusammenhang_verschieden(a.1, b.1)
}

/// Der quantisierte Gesamtrang als ganze Zahl von Quanten (M-25).
///
/// Die Form, in der Raenge ueberhaupt verglichen werden — von der Sortierung
/// und von der Trennungsfrage (E6) gemeinsam benutzt. Zwei Aufrufstellen mit
/// derselben Rechnung waeren zwei Gelegenheiten, sie verschieden zu runden.
pub fn rang_quantisiert(rang: &Rangkomponenten) -> i64 {
    (quantisiert(rang.rang()) / RANG_QUANTUM).round() as i64
}

/// Die Anomalie des Masters: welche Metrik, welches Band, welcher Wert.
///
/// Gemessen wird auf den **64 Gruppen** des Livegitters (M-18) — die Gruppe
/// mit der groessten Abweichung vom eigenen Bandmedian. Die Rueckgabe traegt
/// das FEINE Intervall dieser Gruppe, weil Stufe B auf dem 221er-Verlauf
/// rechnet (M-19).
/// 🔑 **NAK-213 E1:** die Rueckgabe traegt zusaetzlich die GRUPPENNUMMER.
/// Sie wird hier ohnehin bestimmt (die staerkste der 64), und das Screening
/// braucht sie. Sie ein zweites Mal zu suchen waeren zwei Gelegenheiten, sie
/// verschieden zu runden — dieselbe Lehre wie bei `rang_quantisiert`.
pub fn masteranomalie(
    master: &Quellprofil,
    passage: Option<&Passagenfenster>,
) -> Option<(Zielmetrik, Bandintervall, Beobachtung, usize)> {
    // 🔑 **NAK-212 R5/E5 (07.09.2026).** Fuehrt die Sitzung eine Passage,
    // wird die Anomalie aus dem letzten Fenster INNERHALB der Passage
    // bestimmt. Bis hierher las die Funktion `fenster.last()` ohne
    // Passagenbezug: lieferte der Master ein Fenster HINTER der Passage mit
    // einer Anomalie in einem anderen Band, nannte der Befund die alte
    // Passage und behauptete deren fremde Anomalie mit `hoch`/READY
    // (G5-Befund A4).
    //
    // ⚠️ Gibt es kein Fenster in der Passage, faellt die Funktion auf das
    // juengste Fenster UEBERHAUPT zurueck — mit UNGUELTIGER Beobachtung. Ein
    // `None` beendete die Rechnung VOR den Gates (`hypothesen`), und die
    // Sitzung schwiege; M-27 verlangt aber ein Ergebnis. Mit dem Rueckfall
    // laufen die Gates: `fenster_in_passage(master)` ist dann 0, also faellt
    // jeder Kandidat am Passagenmaterial, und das Ergebnis ist die Enthaltung
    // mit Grund. Ein BEFUND mit ungueltiger Beobachtung kann so nicht
    // entstehen.
    let (letztes, gemessen) = match passage {
        Some(p) => match fenster_ganz_in_passage(master, p).last().copied() {
            Some(f) => (f, true),
            None => (master.fenster.last()?, false),
        },
        None => (master.fenster.last()?, true),
    };
    let werte: Vec<(usize, f64)> = (0..BAENDER_FEIN)
        .filter_map(|i| letztes.band(i).map(|w| (i, w)))
        .collect();
    if werte.is_empty() {
        return None;
    }
    let basis = median(&werte.iter().map(|(_, w)| *w).collect::<Vec<_>>());
    let mut je_gruppe: Vec<(f64, usize)> = vec![(f64::NEG_INFINITY, 0); GRUPPEN_LIVE];
    for (index, wert) in &werte {
        let g = gruppe_von_band(*index);
        let abweichung = wert - basis;
        if abweichung > je_gruppe[g].0 {
            je_gruppe[g] = (abweichung, *index);
        }
    }
    // Die staerkste Gruppe. Bei Gleichstand die kleinere Gruppennummer — eine
    // stabile Wahl, keine zufaellige (M-25).
    let (gruppe, (abweichung, beobachtetes_band)) = je_gruppe
        .iter()
        .enumerate()
        .filter(|(_, (a, _))| a.is_finite())
        .max_by(|a, b| {
            a.1 .0
                .partial_cmp(&b.1 .0)
                .unwrap_or(std::cmp::Ordering::Equal)
                .then(b.0.cmp(&a.0))
        })?;
    let band = bandintervall_der_gruppe(gruppe);
    // 🔑 NR-06: das Band der BEOBACHTUNG liegt im zurueckgegebenen Intervall.
    // Der Ort und die Zahl, die ihn belegt, gehoeren zusammen (M-16); fielen
    // sie auseinander, zeigte die Zone etwas anderes als der Befund misst.
    debug_assert!(
        (band.von as usize) <= *beobachtetes_band && *beobachtetes_band < band.bis as usize,
        "Band {beobachtetes_band} liegt nicht in [{}, {})",
        band.von,
        band.bis
    );
    let beobachtung = Beobachtung {
        wert_db: basis + abweichung,
        // `gueltig: false` heisst: die Zahl steht ausserhalb der benannten
        // Passage und ist deshalb keine Messung ueber sie (M-07-Muster).
        gueltig: gemessen,
    };
    Some((Zielmetrik::BandPegelDb, band, beobachtung, gruppe))
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
fn enthaltung_ohne_ort(aufnahme: &Aufnahme, grund: Enthaltungsgrund) -> Option<CauseHypothesis> {
    // Die deterministische Adresse der Sitzung: die lexikographisch kleinste
    // `instance_id` der `main`-Clients. Sie behauptet KEINE Fuehrung.
    let quelle = aufnahme.mains.first()?.clone();

    // Die juengsten GUELTIGEN Belege der Sitzung, ueber alle Quellen. Die
    // Fensterfolgen sind bereits um zurueckgenommene Belege bereinigt
    // (`fenster_aus_historie`), und `empfangsfolge` ordnet sie.
    let mut belege: Vec<(u64, &str)> = aufnahme
        .kandidaten
        .iter()
        .chain(aufnahme.master.iter())
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
fn enthaltung(
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

#[allow(clippy::too_many_arguments)]
fn baue_befund(
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
    let claim_class = if kandidat.prepost_paar {
        Aussageklasse::Wirkungsbeleg
    } else {
        Aussageklasse::Zusammenhang
    };
    let ursachenklasse = if mehrere_kandidaten {
        Ursachenklasse::ZweiQuellenKonkurrenz
    } else if kandidat.prepost_paar {
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
    } else if !kandidat.prepost_paar {
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
        maskierung: super::maskierung::maskierung(master, kandidat, band),
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

fn kurz(id: &str) -> String {
    id.chars().take(8).collect()
}

// ═════════════════════════════════════════════════════════════════════════
// Rechenwerkzeug — klein, rein und einzeln messbar
// ═════════════════════════════════════════════════════════════════════════

fn mittel(werte: impl Iterator<Item = f64>) -> f64 {
    let mut summe = 0.0;
    let mut n = 0usize;
    for wert in werte {
        if !wert.is_finite() {
            continue;
        }
        summe += wert;
        n += 1;
    }
    if n == 0 {
        return 0.0;
    }
    summe / n as f64
}

fn median(werte: &[f64]) -> f64 {
    let mut endlich: Vec<f64> = werte.iter().copied().filter(|x| x.is_finite()).collect();
    if endlich.is_empty() {
        return 0.0;
    }
    endlich.sort_by(|a, b| a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal));
    let mitte = endlich.len() / 2;
    if endlich.len() % 2 == 1 {
        endlich[mitte]
    } else {
        (endlich[mitte - 1] + endlich[mitte]) / 2.0
    }
}

fn spannweite(werte: &[f64]) -> f64 {
    let endlich: Vec<f64> = werte.iter().copied().filter(|x| x.is_finite()).collect();
    if endlich.len() < 2 {
        return 0.0;
    }
    let mut min = f64::INFINITY;
    let mut max = f64::NEG_INFINITY;
    for wert in endlich {
        min = min.min(wert);
        max = max.max(wert);
    }
    (max - min).max(0.0)
}

/// Das Bandmittel eines Fensters im Intervall, in dB. `None`, wenn kein
/// einziges Band darin ein Bit traegt.
fn bandmittel(fenster: &Evidenzfenster, band: Bandintervall) -> Option<f64> {
    let mut summe = 0.0;
    let mut n = 0usize;
    for index in band.von as usize..band.bis as usize {
        if let Some(wert) = fenster.band(index) {
            summe += wert;
            n += 1;
        }
    }
    (n > 0).then(|| summe / n as f64)
}

/// Die Projektspanne einer Fensterfolge, `(von, bis)`.
///
/// ⚠️ Saettigend: zwei Fenster an entgegengesetzten `i64`-Raendern duerfen
/// weder paniken noch umbrechen (derselbe Grund wie in
/// `vergleichbarkeit::ueberdeckung`).
fn spanne(fenster: &[Evidenzfenster]) -> (i64, i64) {
    let mut von = i64::MAX;
    let mut bis = i64::MIN;
    for f in fenster {
        von = von.min(f.projekt_von);
        bis = bis.max(f.projekt_bis);
    }
    if von > bis {
        return (0, 0);
    }
    (von, bis)
}

/// Ueberlappen zwei Fenster wirklich? (NAK-212 R4/E4)
///
/// Dieselbe Zeitachse UND eine gemeinsame Spanne GROESSER NULL. Die zweite
/// Haelfte ist nicht selbstverstaendlich: `sample_count` hat vertraglich
/// `minimum: 0`, ein Fenster kann also leer sein. Die naheliegende Form
/// `f.bis > g.von && f.von < g.bis` haelt fuer `f = [5, 5)` gegen
/// `g = [0, 10)` BEIDSEITIG, obwohl kein einziges Sample gemeinsam ist.
///
/// ⚠️ Maximum-Minimum statt Subtraktion: die Form vergleicht und rechnet
/// nicht, kann also auf `i64` nicht ueberlaufen.
///
/// Das Praedikat steht an EINER Stelle — `paarueberdeckung` und
/// `gemeinsame_reihen` lesen dieselbe Definition. Zwei Kopien waeren zwei
/// Gelegenheiten, sie verschieden zu formulieren (Lehre NR-01).
fn ueberlappt(f: &Evidenzfenster, g: &Evidenzfenster) -> bool {
    f.transport_epoch == g.transport_epoch
        && f.projekt_von.max(g.projekt_von) < f.projekt_bis.min(g.projekt_bis)
}

/// Der Anteil der Fenster der KUERZEREN Seite, die einen Partner haben (R4).
///
/// Normiert wird auf die kuerzere Seite — dieselbe Entscheidung, die
/// `zeitueberdeckung` fuer Intervalle trifft. „Anteil der MASTERfenster"
/// waere zu streng und braeche eine bestehende Zusage: acht Kandidatenfenster
/// gegen zwoelf Masterfenster ergaeben 0,667 und fielen am Gate — dabei ist
/// genau das der gemessene Randwert von M-23 („mindestens acht"). Schlimmer:
/// eine Quelle, die nur in EINEM TEIL der Zeit aktiv ist, ist der Normalfall
/// und zugleich die Voraussetzung des bedingten Uplifts aus M-19, der Fenster
/// OHNE die Quelle braucht. Ein Gate, das Teilaktivitaet bestraft, machte R1
/// unerfuellbar.
///
/// ⚠️ LEERE Fenster zaehlen weder im Zaehler noch im Nenner. Sie haben nie
/// einen Partner; als „Fenster ohne Partner" gezaehlt senkten sie den Anteil,
/// obwohl sie gar keine Messung sind.
fn paarueberdeckung(a: &[Evidenzfenster], b: &[Evidenzfenster]) -> f64 {
    let anteil = |x: &[Evidenzfenster], y: &[Evidenzfenster]| {
        let zaehlbar: Vec<&Evidenzfenster> =
            x.iter().filter(|f| f.projekt_bis > f.projekt_von).collect();
        if zaehlbar.is_empty() {
            return 0.0;
        }
        let mit_partner = zaehlbar
            .iter()
            .filter(|f| y.iter().any(|g| ueberlappt(f, g)))
            .count();
        mit_partner as f64 / zaehlbar.len() as f64
    };
    anteil(a, b).max(anteil(b, a))
}

/// Zeitueberdeckung zweier Projektfenster, normiert auf das KUERZERE.
///
/// Dieselbe Rechnung wie `vergleichbarkeit::ueberdeckung`; sie steht dort
/// privat, und ein `pub` dafuer waere eine Vertragsaenderung an einem fremden
/// Modul. Die Zahlen kommen aus demselben Gate.
fn zeitueberdeckung(a: (i64, i64), b: (i64, i64)) -> f64 {
    let laenge_a = a.1.saturating_sub(a.0).max(0);
    let laenge_b = b.1.saturating_sub(b.0).max(0);
    let kuerzer = laenge_a.min(laenge_b);
    if kuerzer <= 0 {
        return 0.0;
    }
    let start = a.0.max(b.0);
    let ende = a.1.min(b.1);
    let gemeinsam = ende.saturating_sub(start).max(0);
    (gemeinsam as f64 / kuerzer as f64).clamp(0.0, 1.0)
}

/// Wie viele UNABHAENGIGE Evidenzfenster eine Folge traegt (M-23, R1).
///
/// Zwei Belege, die dasselbe Projektfenster beschreiben, sind EINER. Ohne
/// diese Verdichtung liesse sich das Gate durch blosses Nachsenden desselben
/// Fensters ueberlisten — und genau das waere wieder ein relativer Riegel.
/// **NR-05 (Nacharbeit 1, 07.09.2026): das Passagenmaterial EINER Quelle.**
///
/// M-23 verlangt woertlich „mindestens acht unabhaengige Fenster **je
/// beteiligter Quelle** innerhalb der Passage". Gezaehlt wurde bis hierher
/// nur der Kandidat — und weil `ueberdeckung` relativ rechnet und auf das
/// kuerzere Intervall normiert, bestanden acht Kandidatenfenster gegen ein
/// EINZIGES ueberlappendes Masterfenster alle vier relativen Gates und
/// erreichten `hoch`/READY. Die absolute Zahl ist das einzige Mittel gegen
/// diesen Fall, und sie muss deshalb fuer beide Seiten gelten.
///
/// „In der Passage" heisst: dieselbe Transportepoche und ein ueberlappendes
/// Projektfenster — dieselbe Bedingung, mit der `gate` die Kandidatenfenster
/// auswaehlt.
pub fn fenster_in_passage(profil: &Quellprofil, passage: &Passagenfenster) -> usize {
    unabhaengige_fenster(&fenster_ganz_in_passage(profil, passage))
}

/// Die Fenster einer Quelle, die VOLLSTAENDIG in der Passage liegen (R5, E5).
///
/// 🔑 **NAK-212 R5 (07.09.2026).** M-23 sagt woertlich „innerhalb der
/// Passage". Bis hierher zaehlte jede Beruehrung ab EINEM Sample: vier
/// Fenster weit vor der Passage plus zehn mit je einem Sample Ueberlappung
/// ergaben `hoch`/READY (G5-Befund E-D3). Ein Fenster, das zu 99,98 %
/// draussen liegt, ist nicht innerhalb.
///
/// Die Passage ist halboffen `[von, bis)` — wie jedes Evidenzfenster. Ein
/// Fenster, das exakt am Passagenende endet, liegt drin; eines, das exakt am
/// Passagenanfang endet, nicht.
///
/// Die Definition steht an EINER Stelle: `fenster_in_passage` und
/// `masteranomalie` lesen sie beide.
fn fenster_ganz_in_passage<'a>(
    profil: &'a Quellprofil,
    passage: &Passagenfenster,
) -> Vec<&'a Evidenzfenster> {
    profil
        .fenster
        .iter()
        .filter(|f| {
            f.transport_epoch == passage.transport_epoch
                && f.projekt_von >= passage.projekt_von
                && f.projekt_bis <= passage.projekt_bis
        })
        .collect()
}

/// Das Passagenmaterial, das ZAEHLT: das Minimum ueber Kandidat und Master.
///
/// Eine Passage ist nur so lang wie ihre duennste beteiligte Quelle. Der
/// Master gehoert dazu — er traegt die Anomalie, die erklaert werden soll
/// (§8).
pub fn passagenmaterial(
    kandidat: &Quellprofil,
    master: &Quellprofil,
    passage: &Passagenfenster,
) -> usize {
    fenster_in_passage(kandidat, passage).min(fenster_in_passage(master, passage))
}

pub fn unabhaengige_fenster(fenster: &[&Evidenzfenster]) -> usize {
    let mut gesehen: std::collections::BTreeSet<(u64, i64, i64)> = Default::default();
    for f in fenster {
        gesehen.insert((f.transport_epoch, f.projekt_von, f.projekt_bis));
    }
    gesehen.len()
}

/// Zwei Reihen ueber die GEMEINSAMEN Fenster von Kandidat und Master.
///
/// Gemeinsam heisst: dieselbe Transportepoche und ueberlappende
/// Projektfenster. Zwei Reihen, die nur der Reihenfolge nach gepaart wuerden,
/// verglichen verschiedene Zeitpunkte — genau der Fehler, den M-19 an einem
/// Uplift ohne Vergleichsfenster meint.
fn gemeinsame_reihen(
    kandidat: &Quellprofil,
    master: &Quellprofil,
    hole: impl Fn(&Evidenzfenster) -> f64,
) -> (Vec<f64>, Vec<f64>) {
    let mut a = Vec::new();
    let mut b = Vec::new();
    for kf in &kandidat.fenster {
        // 🔑 NAK-212 E4: DASSELBE Praedikat wie `paarueberdeckung`. Bis
        // hierher stand die Bedingung hier ein zweites Mal ausgeschrieben —
        // und liess leere Fenster als Partner durchgehen.
        let Some(mf) = master.fenster.iter().find(|m| ueberlappt(m, kf)) else {
            continue;
        };
        a.push(hole(kf));
        b.push(hole(mf));
    }
    (a, b)
}

/// Pearson-Korrelation MIT Vorzeichen, oder `None`.
///
/// `None` heisst genau eine Sache: die Korrelation ist NICHT MESSBAR. Vier
/// Lagen fuehren dorthin, und jede steht unten einzeln benannt — ungleich
/// lange oder zu kurze Reihen, weniger als zwei ENDLICHE Paare, eine Reihe
/// ohne Streuung, ein nicht endliches Ergebnis.
///
/// Eine messbare Korrelation gibt `Some(r)` zurueck, EINSCHLIESSLICH
/// `Some(0.0)`: zwei Reihen mit Streuung, die nichts miteinander zu tun
/// haben, sind gemessen null — weder Beleg noch Gegenbeleg (R1, R2), aber
/// ein MESSWERT und damit von „nicht gemessen" unterscheidbar (E6).
///
/// 🔑 **NAK-212 Nacharbeit 1, NR-01 (07.09.2026).** Bis hierher rief diese
/// Funktion eine `f64`-Fassung `korrelation` und bildete deren `0.0` auf
/// `None` ab. Jene gab aber in FUENF Lagen 0,0 zurueck — den vier oben
/// genannten und der echten Nullkorrelation; wer die vier an der Zahl
/// wiedererkennen will, trifft zwangslaeufig auch die fuenfte. Onsetreihen
/// wie `[1,1,3,3]` gegen `[1,3,1,3]` (beide mit Streuung, Korrelation exakt
/// null) wurden damit zu `None`, und `zusammenhang_verschieden` (E6) hielt
/// ein getrenntes Paar fuer ungetrennt: der Fuehrende blieb `mittel` statt
/// `hoch`. Die Rechnung steht deshalb JETZT HIER, und die `f64`-Fassung ist
/// ersatzlos entfallen — sie hatte im Produkt keinen Aufrufer mehr. Eine
/// Rechnung, eine Stelle: dieselbe Lehre wie bei `ueberlappt` (E4).
fn korrelation_gerichtet(a: &[f64], b: &[f64]) -> Option<f64> {
    // Lage 1: zu kurz oder ungleich lang.
    if a.len() != b.len() || a.len() < 2 {
        return None;
    }
    // NaN und Inf nehmen ihr PAAR heraus, statt die Reihe zu vergiften.
    let paare: Vec<(f64, f64)> = a
        .iter()
        .zip(b.iter())
        .filter(|(x, y)| x.is_finite() && y.is_finite())
        .map(|(x, y)| (*x, *y))
        .collect();
    // Lage 2: nach dem Filtern bleiben weniger als zwei Paare.
    if paare.len() < 2 {
        return None;
    }
    let mittel_a = paare.iter().map(|(x, _)| *x).sum::<f64>() / paare.len() as f64;
    let mittel_b = paare.iter().map(|(_, y)| *y).sum::<f64>() / paare.len() as f64;
    let mut zaehler = 0.0;
    let mut var_a = 0.0;
    let mut var_b = 0.0;
    for (x, y) in &paare {
        let dx = x - mittel_a;
        let dy = y - mittel_b;
        zaehler += dx * dy;
        var_a += dx * dx;
        var_b += dy * dy;
    }
    // Lage 3: eine der beiden Reihen steht still. Zwei Konstanten sind kein
    // Zusammenhang — und auch kein Gegenbeleg.
    if var_a <= 0.0 || var_b <= 0.0 {
        return None;
    }
    let r = zaehler / (var_a.sqrt() * var_b.sqrt());
    // Lage 4: die Rechnung selbst ist nicht endlich (Ueberlauf im Zaehler
    // oder in einer der Varianzen bei sehr grossen Werten).
    r.is_finite().then_some(r)
}

// ═════════════════════════════════════════════════════════════════════════
// Modulebene (§5.3 R1)
// ═════════════════════════════════════════════════════════════════════════
//
// Diese Faelle rufen das Modul DIREKT und koennen deshalb nicht fallen, wenn
// die Verdrahtung fehlt — das ist ausdruecklich ihre Grenze. Sie messen die
// Zahlenraender, an die ein Verdrahtungsfall nicht herankommt: Bandindex 0
// und 220, leere Reihen, NaN, das Fenster GENAU an der Schwelle. Der
// Rotbeweis faellt am Verdrahtungsbein `broker/tests/sonde014_hypothese.rs`.
#[cfg(test)]
mod tests {
    use super::*;
    use crate::coordinator::intent::SchutzangabeSpiegel;

    fn fenster(von: i64, bis: i64, epoche: u64) -> Evidenzfenster {
        Evidenzfenster {
            evidence_id: format!("{von:032x}"),
            empfangsfolge: von.unsigned_abs(),
            projekt_von: von,
            projekt_bis: bis,
            transport_epoch: epoche,
            abdeckung: 0.9,
            p50_db: vec![-20.0; BAENDER_FEIN],
            p50_gueltig: vec![true; BAENDER_FEIN],
            onset: 1.0,
            seitenanteil_db: None,
            hat_baender: true,
            hat_verteilung: true,
            hat_stereo: false,
        }
    }

    /// Ein Rangkomponentensatz aus den sechs Zahlen, in Vertragsreihenfolge.
    fn rang(bp: f64, koinz: f64, upl: f64, intent: f64, wdh: f64, routing: f64) -> Rangkomponenten {
        Rangkomponenten {
            bandpassung: bp,
            koinzidenz: koinz,
            uplift: upl,
            intent_relevanz: intent,
            wiederholbarkeit: wdh,
            routingqualitaet: routing,
        }
    }

    fn beleg(upl: Option<f64>, koinz: Option<f64>, wdh: f64) -> Zusammenhangsbeleg {
        Zusammenhangsbeleg {
            uplift: upl,
            koinzidenz: koinz,
            wiederholbarkeit: wdh,
        }
    }

    // ═════════════════════════════════════════════════════════════════════
    // NAK-213 · die Screeningordnung (E1, E2)
    // ═════════════════════════════════════════════════════════════════════

    /// Ein Quellprofil mit gesetzten Baendern in `band` und der Onsetreihe
    /// `onsets`. Die uebrigen Baender bleiben auf dem Grundpegel.
    fn quelle(
        id: &str,
        anzahl: usize,
        band: (usize, usize),
        db: f32,
        onsets: &[f32],
    ) -> Quellprofil {
        let mut fenster_liste = Vec::new();
        for i in 0..anzahl {
            let von = 1000 + (i as i64) * 500;
            let mut f = fenster(von, von + 500, 1);
            for index in band.0..band.1.min(BAENDER_FEIN) {
                f.p50_db[index] = -20.0 + db;
            }
            f.onset = onsets.get(i).copied().unwrap_or(1.0);
            fenster_liste.push(f);
        }
        Quellprofil {
            quelle_id: id.into(),
            fenster: fenster_liste,
            routing_bekannt: true,
            ..Default::default()
        }
    }

    /// Wie `quelle`, aber ab einem gegebenen Fensterindex und ohne Onsetreihe.
    fn quelle_ab(
        id: &str,
        ab: usize,
        anzahl: usize,
        band: (usize, usize),
        db: f32,
    ) -> Quellprofil {
        let mut fenster_liste = Vec::new();
        for i in ab..ab + anzahl {
            let von = 1000 + (i as i64) * 500;
            let mut f = fenster(von, von + 500, 1);
            for index in band.0..band.1.min(BAENDER_FEIN) {
                f.p50_db[index] = -20.0 + db;
            }
            fenster_liste.push(f);
        }
        Quellprofil {
            quelle_id: id.into(),
            fenster: fenster_liste,
            routing_bekannt: true,
            ..Default::default()
        }
    }

    fn aufnahme_mit(master: Quellprofil, kandidaten: Vec<Quellprofil>) -> Aufnahme {
        Aufnahme {
            mains: vec![master.quelle_id.clone()],
            master: Some(master),
            kandidaten,
            passage: None,
            passage_id: None,
            // M-86 ist fail-closed: ohne Vollstaendigkeitsmarke rechnet gar
            // nichts (`darf_gerechnet_werden`). Die Marke gehoert deshalb in
            // die Grundform jeder Aufnahme dieses Abschnitts.
            intent: Some(crate::coordinator::intent::IntentBestand {
                vollstaendig: true,
                ..Default::default()
            }),
            metrics_version: 1,
            session_epoch: "e".repeat(32),
        }
    }

    /// **K-06.** Der Screeningrang misst auf der GRUPPE, nicht auf einem
    /// feineren Intervall.
    ///
    /// M-18 nennt fuer das Screening ausdruecklich die 64 Gruppen und fuehrt
    /// „das Screening nutzt das 221er-Gitter statt der 64 Gruppen" als
    /// eigenen Rotbeweis. Zwei Kandidaten mit derselben FEINEN Bandpassung in
    /// IHREM jeweiligen Band rangieren deshalb verschieden, sobald das eine
    /// Band in der Befundgruppe liegt und das andere in der Nachbargruppe.
    #[test]
    fn screeningrang_misst_auf_der_gruppe() {
        let gruppe = gruppe_von_band(100);
        let eigen = bandintervall_der_gruppe(gruppe);
        let nachbar = bandintervall_der_gruppe(gruppe + 1);
        assert_ne!(eigen, nachbar, "Vorbedingung: zwei verschiedene Gruppen");

        let master = quelle("m", 4, (eigen.von as usize, eigen.bis as usize), 12.0, &[]);
        let drin = quelle("a", 4, (eigen.von as usize, eigen.bis as usize), 12.0, &[]);
        let daneben = quelle(
            "b",
            4,
            (nachbar.von as usize, nachbar.bis as usize),
            12.0,
            &[],
        );
        // Vorbedingung, ausdruecklich: BEIDE tragen in IHREM eigenen Band
        // dieselbe Energie — die Aufbauten unterscheiden sich allein in der
        // GRUPPE. Ein exakt gleicher Anteil ist nicht zu haben: die Gruppen
        // des 64er-Gitters sind drei oder vier Baender breit, und
        // `bandpassung` normiert auf die Gesamtleistung.
        assert!(
            bandpassung(&drin, eigen) > 0.0 && bandpassung(&daneben, nachbar) > 0.0,
            "Vorbedingung: beide tragen Energie in ihrem eigenen Band"
        );
        assert!(
            bandpassung(&daneben, eigen) < bandpassung(&daneben, nachbar),
            "Vorbedingung: der Nachbar traegt seine Energie NICHT in der Befundgruppe"
        );

        let aufnahme = aufnahme_mit(master, vec![drin.clone(), daneben.clone()]);
        let master_ref = aufnahme.master.as_ref().expect("die Aufnahme traegt einen Master");
        let rang_drin = screeningrang(&drin, &aufnahme, master_ref, gruppe);
        let rang_daneben = screeningrang(&daneben, &aufnahme, master_ref, gruppe);
        assert!(
            rang_drin.gruppenenergie > rang_daneben.gruppenenergie,
            "die Gruppenenergie misst auf der BEFUNDGRUPPE: {} gegen {}",
            rang_drin.gruppenenergie,
            rang_daneben.gruppenenergie
        );
        assert!(
            screening_quantisiert(&rang_drin) > screening_quantisiert(&rang_daneben),
            "und entscheidet damit die Ordnung"
        );
    }

    /// **K-52 (Modulebene).** Die Onset-/Peak-Koinzidenz ist eine der vier
    /// Screeninggroessen — aus DERSELBEN Funktion wie die Rangkomponente.
    #[test]
    fn screeningrang_traegt_die_koinzidenz() {
        let gruppe = gruppe_von_band(100);
        let band = bandintervall_der_gruppe(gruppe);
        let spanne = (band.von as usize, band.bis as usize);
        let onsets_master = [1.0, 4.0, 1.0, 4.0, 1.0, 4.0];
        let master = quelle("m", 6, spanne, 12.0, &onsets_master);
        // Gleichlaeufig: die Koinzidenz ist positiv.
        let gleich = quelle("a", 6, spanne, 12.0, &onsets_master);
        // Gegenlaeufig: `koinzidenz_gerichtet` ist negativ und zaehlt im
        // Screening wie in der Rangkomponente als 0.
        let gegen = quelle("b", 6, spanne, 12.0, &[4.0, 1.0, 4.0, 1.0, 4.0, 1.0]);
        // Konstant: keine Streuung, also `None` — ebenfalls 0.
        let flach = quelle("c", 6, spanne, 12.0, &[2.0; 6]);

        let aufnahme = aufnahme_mit(
            master.clone(),
            vec![gleich.clone(), gegen.clone(), flach.clone()],
        );
        let master_ref = aufnahme.master.as_ref().expect("die Aufnahme traegt einen Master");
        let r_gleich = screeningrang(&gleich, &aufnahme, master_ref, gruppe);
        let r_gegen = screeningrang(&gegen, &aufnahme, master_ref, gruppe);
        let r_flach = screeningrang(&flach, &aufnahme, master_ref, gruppe);

        assert!(
            r_gleich.koinzidenz > 0.0,
            "gleichlaeufig traegt eine positive Koinzidenz: {}",
            r_gleich.koinzidenz
        );
        assert_eq!(r_gegen.koinzidenz, 0.0, "der negative Zweig zaehlt als 0");
        assert_eq!(r_flach.koinzidenz, 0.0, "und `None` ebenfalls");
        // Dieselbe Funktion, dieselben Reihen wie die Rangkomponente.
        assert_eq!(
            r_gleich.koinzidenz,
            koinzidenz_gerichtet(&gleich, &master).unwrap_or(0.0).max(0.0),
            "das Screening ruft `koinzidenz_gerichtet` — keine zweite Definition"
        );
        // Die drei unterscheiden sich in KEINER anderen Screeninggroesse.
        assert_eq!(r_gleich.gruppenenergie, r_gegen.gruppenenergie);
        assert_eq!(r_gleich.alignment, r_gegen.alignment);
        assert_eq!(r_gleich.intent_relevanz, r_gegen.intent_relevanz);
        assert!(screening_quantisiert(&r_gleich) > screening_quantisiert(&r_gegen));
    }

    /// **K-07.** Stufe B laeuft hoechstens `KANDIDATEN_DECKEL`-mal je
    /// Rechnung — gemessen mit einem ZAEHLER, nicht behauptet.
    ///
    /// M-18 verlangt den Nachweis woertlich: „Ein sechster Kandidat erreicht
    /// Stufe B" ist der Rotbeweis. Ohne den Zaehler waere die Zusage nicht
    /// gemessen (Pruefliste §E).
    #[test]
    fn stufe_b_laeuft_hoechstens_fuenfmal() {
        let gruppe = gruppe_von_band(100);
        let band = bandintervall_der_gruppe(gruppe);
        let spanne = (band.von as usize, band.bis as usize);
        let master = quelle("m", 8, spanne, 12.0, &[]);

        for anzahl in [6usize, 9] {
            let kandidaten: Vec<Quellprofil> = (0..anzahl)
                // Paarweise verschiedene Gruppenenergie ueber den Pegel.
                .map(|i| quelle(&format!("{i:032x}"), 8, spanne, 8.0 + i as f32 * 0.7, &[]))
                .collect();
            let aufnahme = aufnahme_mit(master.clone(), kandidaten);
            stufe_b_zaehler_zuruecksetzen();
            let ergebnis = hypothesen(&aufnahme);
            let laeufe = stufe_b_aufrufe();
            assert_eq!(
                stufe_b_max_je_rechnung(),
                laeufe,
                "auf der Modulebene ist der Anlass EINE Rechnung"
            );
            assert!(
                laeufe <= KANDIDATEN_DECKEL,
                "{anzahl} Kandidaten: Stufe B lief {laeufe}-mal, erlaubt sind hoechstens \
                 {KANDIDATEN_DECKEL} (heutiger Stand vor NAK-213: {anzahl})"
            );
            assert_eq!(
                ergebnis.befunde.len(),
                KANDIDATEN_DECKEL,
                "und genau fuenf Befunde bleiben sichtbar"
            );
            // M-87: die Abgeschnittenen verschwinden nicht — sie tragen ihren
            // Grund. Die Liste wird NICHT gekappt (R6).
            let ueberboten = ergebnis
                .ausschluesse
                .iter()
                .filter(|a| a.grund == Ausschlussgrund::ScreeningUeberboten)
                .count();
            assert_eq!(ueberboten, anzahl - KANDIDATEN_DECKEL);
        }
    }

    /// **K-10 bis K-16 (R2/E4), geschlossene Menge.** Genau FUENF der zehn
    /// Ausschlussgruende sind Messgruende — und die `Konkurrenzlage` liest
    /// genau sie.
    ///
    /// Die drei Faelle, die auf dem Produktpfad erreichbar sind
    /// (`coverage_fehlt`, `alignment_falsch`, `evidenz_zurueckgenommen`),
    /// misst `sonde014_hypothese.rs::einzelueberlebender_neben_messausschluss`;
    /// die zwei Passagengruende brauchen eine benannte Passage und liegen im
    /// Passagenharnisch von `sonde014_gegenbeispiele.rs`. HIER faellt die
    /// Zuordnung selbst — eine Zeile, die nur an drei Beispielen gemessen
    /// waere, liesse die anderen sieben Werte offen.
    #[test]
    fn messgruende_sind_genau_die_fuenf_aus_r2() {
        let messgruende: Vec<Ausschlussgrund> = Ausschlussgrund::ALLE
            .into_iter()
            .filter(|g| g.ist_messgrund())
            .collect();
        assert_eq!(
            messgruende,
            vec![
                Ausschlussgrund::CoverageFehlt,
                Ausschlussgrund::AlignmentFalsch,
                Ausschlussgrund::PassageUnvergleichbar,
                Ausschlussgrund::PassageZuKurz,
                Ausschlussgrund::EvidenzZurueckgenommen,
            ],
            "R2 nennt woertlich diese fuenf"
        );

        let aus = |grund: Ausschlussgrund| {
            vec![Ausschluss {
                candidate_source: "a".repeat(32),
                grund,
            }]
        };

        // Jeder der fuenf setzt `Messausschluss` — einzeln gemessen.
        for grund in messgruende {
            assert_eq!(
                konkurrenzlage(&aus(grund), false),
                Konkurrenzlage::Messausschluss,
                "{grund:?} ist ein Messgrund"
            );
        }
        // Und keiner der uebrigen fuenf.
        for grund in [
            Ausschlussgrund::IntentVetoGeschuetzt,
            Ausschlussgrund::IntentVetoVerschmolzen,
            // NB-1: R2 nennt ihn nicht. NB-6: auf dem Produktpfad ist er
            // ohnehin unerreichbar, weil `masteranomalie` nur
            // `band_pegel_db` liefert und `baender` ein Pflichtfeld ist.
            Ausschlussgrund::CapabilityFehlt,
            // Ueber E2 geregelt.
            Ausschlussgrund::ScreeningUeberboten,
            // Gerade der Nachweis, dass der Kandidat keine Ursache sein KANN.
            Ausschlussgrund::MasterDuplikat,
        ] {
            assert_eq!(
                konkurrenzlage(&aus(grund), false),
                Konkurrenzlage::Vollstaendig,
                "{grund:?} macht die Konkurrenz nicht unvollstaendig"
            );
        }

        // Die Praezedenz steht an EINER Stelle (K-21): `Messausschluss`
        // schlaegt `DeckelOhneAbstand`.
        assert_eq!(
            konkurrenzlage(&[], true),
            Konkurrenzlage::DeckelOhneAbstand
        );
        assert_eq!(
            konkurrenzlage(&aus(Ausschlussgrund::CoverageFehlt), true),
            Konkurrenzlage::Messausschluss,
            "beide Lagen zugleich: der gefallene Konkurrent ist die konkretere Aussage"
        );
        assert_eq!(konkurrenzlage(&[], false), Konkurrenzlage::Vollstaendig);
    }

    /// **K-12, K-13 (R2/E4).** Ein einziger Ueberlebender neben einem
    /// PASSAGENausschluss traegt hoechstens `mittel`.
    ///
    /// Beide Gruende brauchen eine benannte Passage. `passage_zu_kurz` misst
    /// zusaetzlich `sonde014_gegenbeispiele.rs` am Produktpfad;
    /// `passage_unvergleichbar` ist dort nicht sauber zu bauen, weil Gate 3
    /// (Alignment) dieselbe Schwelle prueft und DAVOR steht — der verbleibende
    /// Hebel ist die fremde Transportepoche, und die loest auf dem Produktweg
    /// zuerst die Materialinvalidierung aus. HIER faellt sie an `gate()`
    /// selbst.
    #[test]
    fn einzelueberlebender_neben_passagenausschluss() {
        let gruppe = gruppe_von_band(100);
        let intervall = bandintervall_der_gruppe(gruppe);
        let spanne = (intervall.von as usize, intervall.bis as usize);
        // 🔑 Die Passage ist DOPPELT so lang wie die Masterfenster: nur
        // dann kann ein Kandidat, der jedes Masterfenster deckt, die
        // PASSAGE trotzdem verfehlen. `zeitueberdeckung` normiert auf das
        // KUERZERE Intervall — liegt eines ganz im anderen, ist das
        // Verhaeltnis immer 1,0, und Gate 4a waere unerreichbar.
        let passage = Passagenfenster {
            projekt_von: 1000,
            projekt_bis: 1000 + 20 * 500,
            transport_epoch: 1,
        };

        // Master und Ueberlebender liegen in der ZWEITEN Haelfte der Passage.
        let master = quelle_ab("m", 10, 10, spanne, 12.0);
        let ueberlebender = quelle_ab(&"a".repeat(32), 10, 10, spanne, 12.0);

        // ── Fall c: der Konkurrent deckt JEDES Masterfenster, ragt aber am
        //    Ende ueber die Passage hinaus → `passage_unvergleichbar`.
        //
        // 🔑 Der Aufbau ist enger, als er aussieht. Eine fremde
        // Transportepoche reisst schon `ueberlappt` und damit Gate 3
        // (Alignment), das VOR dem Passagengate steht — der Kandidat fiele
        // dann mit `alignment_falsch`, und die Zeile maesse einen anderen
        // Grund als den benannten (im ersten Aufbau genau so gemessen).
        // `paarueberdeckung` nimmt das MAXIMUM beider Richtungen: solange
        // jedes MASTERfenster einen Partner hat, bleibt das Alignment 1,0.
        // Die SPANNE des Kandidaten ueberlappt die Passage dann nur zur
        // Haelfte — und genau die liest Gate 4a.
        let fremd = quelle_ab(&"b".repeat(32), 10, 20, spanne, 12.0);
        // ── Fall d: nur vier Fenster in der Passage → `passage_zu_kurz`.
        let kurz = quelle_ab(&"c".repeat(32), 10, 4, spanne, 12.0);

        for (fall, konkurrent, grund) in [
            ("c", fremd.clone(), Ausschlussgrund::PassageUnvergleichbar),
            ("d", kurz.clone(), Ausschlussgrund::PassageZuKurz),
        ] {
            let mut aufnahme = aufnahme_mit(master.clone(), vec![ueberlebender.clone()]);
            aufnahme.passage = Some(passage);
            aufnahme.passage_id = Some("p".repeat(32));

            // Vorbedingung: OHNE den Konkurrenten haelt der Ueberlebende
            // jedes Gate — sonst maesse der Fall etwas anderes.
            let master_ref = aufnahme.master.as_ref().expect("die Aufnahme traegt einen Master");
            let (metrik, band, _, _) = masteranomalie(master_ref, aufnahme.passage.as_ref())
                .expect("der Master traegt eine Anomalie");
            assert_eq!(
                gate(&ueberlebender, &aufnahme, master_ref, metrik, band),
                Gateurteil::Bleibt,
                "Fall {fall}: der Ueberlebende besteht jedes Gate"
            );
            // Und der Konkurrent faellt mit GENAU dem benannten Grund.
            assert_eq!(
                gate(&konkurrent, &aufnahme, master_ref, metrik, band),
                Gateurteil::Faellt(grund),
                "Fall {fall}: der Konkurrent faellt mit {grund:?}"
            );

            aufnahme.kandidaten.push(konkurrent);
            let ergebnis = hypothesen(&aufnahme);
            let fuehrend = ergebnis.befunde.first().expect("ein Befund entsteht");
            assert!(
                ergebnis.ausschluesse.iter().any(|a| a.grund == grund),
                "Fall {fall}: der Ausschluss bleibt sichtbar (M-87)"
            );
            assert!(
                fuehrend.confidence.klasse < Sicherheitsklasse::Hoch,
                "Fall {fall}: hoechstens `mittel` neben einem Messausschluss — \
                 heutiger Stand: `getrennt` ist bei einem einzigen \
                 Ueberlebenden trivial wahr: {:?}",
                fuehrend.confidence
            );
        }
    }

    /// **N-41 und N-47 (NAK-212 D1, D7).** Getrennt heisst BEIDES zugleich:
    /// verschiedener quantisierter Gesamtrang UND verschiedener Zusammenhang.
    ///
    /// Der Grund ist eine Eigenschaft von `rang()`: es mittelt sechs
    /// Komponenten und quantisiert ERNEUT, sechstelt also jede
    /// Komponentendifferenz. Zwei Kandidaten koennen sich in einer Komponente
    /// unterscheiden und trotzdem denselben Rang tragen — dann entschiede die
    /// Sortierung ueber die aufsteigende `candidate_source`, WELCHER von
    /// beiden stark wird, und M-26 sagt ausdruecklich, der Tie-Break
    /// entscheide „nur die Anzeigereihenfolge, nicht die Auswahl".
    #[test]
    fn getrennt_verlangt_rang_und_zusammenhang() {
        // N-41: Koinzidenz 0,500000 gegen 0,500001 — quantisiert VERSCHIEDEN,
        // im Gesamtrang identisch.
        let a = rang(0.1, 0.500000, 0.0, 0.5, 0.0, 1.0);
        let b = rang(0.1, 0.500001, 0.0, 0.5, 0.0, 1.0);
        assert_ne!(
            (quantisiert(0.500000) / RANG_QUANTUM).round() as i64,
            (quantisiert(0.500001) / RANG_QUANTUM).round() as i64,
            "die Komponenten sind quantisiert verschieden"
        );
        assert_eq!(
            rang_quantisiert(&a),
            rang_quantisiert(&b),
            "der Gesamtrang ist trotzdem gleich — das ist der Kern von D1"
        );
        assert!(
            zusammenhang_verschieden(
                &beleg(None, Some(0.500000), 0.0),
                &beleg(None, Some(0.500001), 0.0)
            ),
            "die zweite Bedingung allein haelt — und genuegt deshalb nicht"
        );
        assert!(
            !getrennt(
                (&a, &beleg(None, Some(0.500000), 0.0)),
                (&b, &beleg(None, Some(0.500001), 0.0))
            ),
            "und `getrennt` sagt deshalb NEIN — das ist D1"
        );

        // N-47: `Some(0,000001)` gegen `None`. Der Zusammenhang ist
        // verschieden UND nach R1 positiv belegt, der Gesamtrang aber gleich.
        let mit = rang(0.1, 0.0, 0.000001, 0.5, 0.0, 1.0);
        let ohne = rang(0.1, 0.0, 0.0, 0.5, 0.0, 1.0);
        assert_eq!(
            rang_quantisiert(&mit),
            rang_quantisiert(&ohne),
            "ein Uplift unterhalb der Sichtbarkeitsgrenze bewegt den Rang nicht"
        );
        assert!(beleg(Some(0.000001), None, 0.0).positiv_belegt());
        assert!(zusammenhang_verschieden(
            &beleg(Some(0.000001), None, 0.0),
            &beleg(None, None, 0.0)
        ));
        assert!(
            !getrennt(
                (&mit, &beleg(Some(0.000001), None, 0.0)),
                (&ohne, &beleg(None, None, 0.0))
            ),
            "positiv belegt, aber nicht getrennt — das ist D7"
        );

        // Gegenprobe N-17: ein Uplift in der Groessenordnung 0,6 bewegt den
        // Rang sichtbar — 100 000 Quanten.
        let stark = rang(0.1, 0.0, 0.6, 0.5, 0.0, 1.0);
        assert_eq!(rang_quantisiert(&stark) - rang_quantisiert(&ohne), 100_000);
        assert!(
            getrennt(
                (&stark, &beleg(Some(0.6), None, 0.9)),
                (&ohne, &beleg(None, None, 0.0))
            ),
            "N-17: beide Bedingungen halten, also getrennt"
        );

        // Und die andere Richtung: gleicher Zusammenhang, verschiedener Rang
        // (Bandpassung, Routing oder Intent-Relevanz allein) trennt NICHT.
        let laut = rang(0.2, 0.0, 0.6, 0.5, 0.0, 1.0);
        assert_ne!(rang_quantisiert(&laut), rang_quantisiert(&stark));
        assert!(
            !getrennt(
                (&laut, &beleg(Some(0.6), None, 0.9)),
                (&stark, &beleg(Some(0.6), None, 0.9))
            ),
            "R3: die Bandpassung allein trennt nicht"
        );
    }

    /// **R1/R2 (NAK-212).** Die beiden Praedikate des Zusammenhangsbelegs
    /// lesen VORZEICHEN, nicht Groessen.
    #[test]
    fn belegpraedikate_lesen_vorzeichen() {
        assert!(beleg(Some(0.000001), None, 0.0).positiv_belegt());
        assert!(!beleg(Some(0.000001), None, 0.0).gegenbeleg());
        assert!(beleg(Some(-0.000001), None, 0.0).gegenbeleg());
        assert!(!beleg(Some(-0.000001), None, 0.0).positiv_belegt());
        // „Nicht messbar" ist weder das eine noch das andere.
        assert!(!beleg(None, None, 1.0).positiv_belegt());
        assert!(!beleg(None, None, 1.0).gegenbeleg());
        // Exakt null ist ein gemessener Wert ohne Richtung.
        assert!(!beleg(Some(0.0), Some(0.0), 0.0).positiv_belegt());
        assert!(!beleg(Some(0.0), Some(0.0), 0.0).gegenbeleg());
        // Eine positive Koinzidenz genuegt — R1 verlangt „mindestens eine".
        assert!(beleg(None, Some(0.4), 0.0).positiv_belegt());
        // Ein Gegenbeleg in EINER Komponente genuegt fuer R2.
        assert!(beleg(Some(0.9), Some(-0.4), 0.99).gegenbeleg());
    }

    /// **N-10 und N-11 (NAK-212 R2).** Der einseitige Bootstrap misst die
    /// Stabilitaet eines POSITIVEN Effekts — und kostet den richtigen Fall
    /// nichts.
    #[test]
    fn bootstrap_einseitig_trennt_die_richtung() {
        use crate::coordinator::experiment::{
            bootstrap_p, bootstrap_p_positiv, BOOTSTRAP_BLOCK, BOOTSTRAP_ZIEHUNGEN,
        };
        let positiv: Vec<f64> = (0..24).map(|i| 3.0 + (i % 3) as f64 * 0.1).collect();
        let negativ: Vec<f64> = positiv.iter().map(|x| -x).collect();

        // Zweiseitig sind beide Reihen gleich „stabil" — genau das belohnte
        // den Gegenbeleg (G-D5).
        let zwei_pos = bootstrap_p(&positiv, BOOTSTRAP_BLOCK, BOOTSTRAP_ZIEHUNGEN, BOOTSTRAP_SAAT);
        let zwei_neg = bootstrap_p(&negativ, BOOTSTRAP_BLOCK, BOOTSTRAP_ZIEHUNGEN, BOOTSTRAP_SAAT);
        assert_eq!(zwei_pos, zwei_neg, "zweiseitig ist die Richtung unsichtbar");
        assert!(1.0 - zwei_neg > 0.9, "und die Stabilitaet waere hoch");

        // Einseitig: die positive Reihe behaelt ihren Wert, die negative
        // faellt auf „kein Beleg".
        let ein_pos =
            bootstrap_p_positiv(&positiv, BOOTSTRAP_BLOCK, BOOTSTRAP_ZIEHUNGEN, BOOTSTRAP_SAAT);
        let ein_neg =
            bootstrap_p_positiv(&negativ, BOOTSTRAP_BLOCK, BOOTSTRAP_ZIEHUNGEN, BOOTSTRAP_SAAT);
        assert_eq!(ein_pos, zwei_pos, "der richtige Fall kostet nichts (N-11)");
        assert_eq!(ein_neg, 1.0, "ein Gegenbeleg traegt keine Wiederholbarkeit (N-10)");
        assert_eq!((1.0f64 - ein_neg).clamp(0.0, 1.0), 0.0);

        // Leere und nicht-endliche Eingaben: „kein Beleg", nie 0.
        assert_eq!(bootstrap_p_positiv(&[], 4, 400, 7), 1.0);
        assert_eq!(bootstrap_p_positiv(&[f64::NAN, 1.0], 4, 400, 7), 1.0);
        assert_eq!(bootstrap_p_positiv(&positiv, 0, 400, 7), 1.0);
        assert_eq!(bootstrap_p_positiv(&positiv, 4, 0, 7), 1.0);
    }

    /// **N-18, N-19, N-20 bis N-22, N-44 (NAK-212 R4).** Das Alignment misst
    /// die PAARWEISE Ueberlappung, nicht die Ueberdeckung zweier Spannen.
    #[test]
    fn alignment_misst_paarweise_ueberlappung() {
        let reihe = |ab: i64, n: i64, epoche: u64| -> Vec<Evidenzfenster> {
            (0..n).map(|i| fenster(ab + i * 512, ab + (i + 1) * 512, epoche)).collect()
        };

        // N-18: die Sondenspanne UMKLAMMERT die des Masters, null Fenster
        // ueberlappen paarweise. `zeitueberdeckung` der Spannen meldet 1,0.
        let master = reihe(0, 4, 1);
        let umklammernd = [reihe(-20_000, 1, 1), reihe(40_000, 1, 1)].concat();
        assert_eq!(
            zeitueberdeckung(spanne(&umklammernd), spanne(&master)),
            1.0,
            "die alte Groesse haelt — genau das ist der Befund E-D4"
        );
        assert_eq!(
            paarueberdeckung(&umklammernd, &master),
            0.0,
            "paarweise ueberlappt nichts"
        );

        // N-19: disjunkt, ohne Umklammerung.
        assert_eq!(paarueberdeckung(&reihe(-40_000, 4, 1), &master), 0.0);

        // N-20: Teilaktivitaet HAELT — normiert auf die kuerzere Seite.
        // Mit „Anteil der Masterfenster" waere der Wert 8/12 = 0,667 und der
        // Randwert 8 aus M-23 nicht mehr messbar.
        let acht = reihe(0, 8, 1);
        let zwoelf = reihe(0, 12, 1);
        assert_eq!(paarueberdeckung(&acht, &zwoelf), 1.0);
        assert!(paarueberdeckung(&acht, &zwoelf) >= GATE_ZEITUEBERDECKUNG);

        // N-21: Versatz um die halbe Reihe faellt.
        assert!(paarueberdeckung(&reihe(6 * 512, 12, 1), &zwoelf) < GATE_ZEITUEBERDECKUNG);

        // N-22: fremde Transportepoche hat keinen Partner.
        assert_eq!(paarueberdeckung(&reihe(0, 12, 9), &zwoelf), 0.0);

        // N-44: ein LEERES Fenster hat keinen Partner und zaehlt in keiner
        // Richtung. `f = [5, 5)` gegen `g = [0, 10)`: die naheliegende Form
        // `f.bis > g.von && f.von < g.bis` haelt beidseitig.
        let leer = vec![fenster(5, 5, 1)];
        let voll = vec![fenster(0, 10, 1)];
        assert!(leer[0].projekt_bis > voll[0].projekt_von && leer[0].projekt_von < voll[0].projekt_bis);
        assert!(!ueberlappt(&leer[0], &voll[0]), "kein gemeinsames Sample");
        assert_eq!(paarueberdeckung(&leer, &voll), 0.0, "nicht 1,0");
        // Und ein leeres Fenster NEBEN echten senkt den Anteil nicht.
        let mit_leerem = [reihe(0, 8, 1), vec![fenster(5, 5, 1)]].concat();
        assert_eq!(paarueberdeckung(&mit_leerem, &zwoelf), 1.0);
    }

    /// **N-26 bis N-29 (NAK-212 R5).** Passagenmaterial zaehlt nur Fenster,
    /// die VOLLSTAENDIG innerhalb liegen — und die Masteranomalie liest nur
    /// solche Fenster.
    #[test]
    fn passagenmaterial_zaehlt_nur_ganz_innenliegende_fenster() {
        let passage = Passagenfenster {
            projekt_von: 1000,
            projekt_bis: 5000,
            transport_epoch: 1,
        };
        let profil = |f: Vec<Evidenzfenster>| Quellprofil {
            quelle_id: "k".into(),
            fenster: f,
            routing_bekannt: true,
            ..Default::default()
        };

        // Ganz innen — beide Raender halboffen wie das Fenster selbst.
        assert_eq!(fenster_in_passage(&profil(vec![fenster(1000, 1500, 1)]), &passage), 1);
        assert_eq!(fenster_in_passage(&profil(vec![fenster(4500, 5000, 1)]), &passage), 1,
                   "exakt am Passagenende endend liegt INNERHALB");

        // N-26: ragt am Anfang hinaus. N-27: ragt am Ende hinaus.
        assert_eq!(fenster_in_passage(&profil(vec![fenster(900, 1500, 1)]), &passage), 0);
        assert_eq!(fenster_in_passage(&profil(vec![fenster(4500, 5100, 1)]), &passage), 0);
        // Ein Sample Beruehrung — die alte Regel zaehlte es.
        assert_eq!(fenster_in_passage(&profil(vec![fenster(999, 1001, 1)]), &passage), 0);
        // Fremde Epoche zaehlt nie.
        assert_eq!(fenster_in_passage(&profil(vec![fenster(1000, 1500, 9)]), &passage), 0);

        // N-28: die Masteranomalie liest nur Fenster IN der Passage. Das
        // Aussenfenster traegt eine Anomalie in einem anderen Band.
        let mut innen = fenster(1000, 1500, 1);
        innen.p50_db[100] = 20.0;
        let mut aussen = fenster(5000, 5500, 1);
        aussen.p50_db[200] = 40.0;
        let master = profil(vec![innen.clone(), aussen.clone()]);
        let (_, band_mit, beob_mit, gruppe_mit) =
            masteranomalie(&master, Some(&passage)).expect("eine Anomalie");
        assert!(
            (band_mit.von..band_mit.bis).contains(&100),
            "das Band stammt aus dem Fenster IN der Passage: {band_mit:?}"
        );
        assert!(beob_mit.gueltig);
        // 🔑 NAK-213 E1: die Gruppe reist MIT, und sie ist genau die des
        // zurueckgegebenen Intervalls. Sie zweimal zu suchen waeren zwei
        // Gelegenheiten, sie verschieden zu runden.
        assert_eq!(gruppe_mit, gruppe_von_band(100));
        assert_eq!(bandintervall_der_gruppe(gruppe_mit), band_mit);
        // Ohne Passage bleibt es beim juengsten Fenster — unveraendert.
        let (_, band_ohne, _, _) = masteranomalie(&master, None).expect("eine Anomalie");
        assert!((band_ohne.von..band_ohne.bis).contains(&200));

        // N-29: kein Fenster in der Passage → Rueckfall auf das juengste,
        // aber die Beobachtung ist UNGUELTIG. Kein `None`, sonst schwiege die
        // Sitzung vor den Gates (M-27).
        let nur_aussen = profil(vec![aussen]);
        let (_, _, beob, _) = masteranomalie(&nur_aussen, Some(&passage)).expect("ein Rueckfall");
        assert!(!beob.gueltig, "ausserhalb der Passage gemessen ist keine Messung ueber sie");
        assert_eq!(fenster_in_passage(&nur_aussen, &passage), 0);
        // Gar keine Fenster bleibt `None` — das ist NAK-213 R4.
        assert!(masteranomalie(&profil(vec![]), Some(&passage)).is_none());
    }

    /// Die Quantisierung ist die Klammer um jede Rangkomponente. Sie klemmt,
    /// rastet und macht aus Nicht-Endlichem eine 0 — nie eine 1.
    #[test]
    fn quantisierung_klemmt_rastet_und_ist_nan_ehrlich() {
        assert_eq!(quantisiert(f64::NAN), 0.0);
        assert_eq!(quantisiert(f64::INFINITY), 0.0);
        assert_eq!(quantisiert(f64::NEG_INFINITY), 0.0);
        assert_eq!(quantisiert(-1.0), 0.0);
        assert_eq!(quantisiert(2.0), 1.0);
        // Zwei Werte innerhalb eines Quants sind nach der Rasterung gleich —
        // genau das haelt die Reihenfolge zweier praktisch gleicher
        // Kandidaten stabil (M-26).
        assert_eq!(
            quantisiert(0.5 + RANG_QUANTUM / 4.0),
            quantisiert(0.5 - RANG_QUANTUM / 4.0)
        );
        // Und zwei Werte ueber einem Quant sind es NICHT — sonst waere die
        // Rasterung eine Gleichmacherei statt einer Stabilisierung.
        assert_ne!(quantisiert(0.5), quantisiert(0.5 + 2.0 * RANG_QUANTUM));
    }

    /// Der Rang addiert in der Reihenfolge von `RANGKOMPONENTEN` und mittelt
    /// gleichgewichtet. Eine fehlende Komponente traegt 0 bei, keine 1.
    #[test]
    fn rang_ist_das_quantisierte_mittel_der_sechs() {
        let voll = Rangkomponenten {
            bandpassung: 1.0,
            koinzidenz: 1.0,
            uplift: 1.0,
            intent_relevanz: 1.0,
            wiederholbarkeit: 1.0,
            routingqualitaet: 1.0,
        };
        assert_eq!(voll.rang(), 1.0);
        assert_eq!(Rangkomponenten::default().rang(), 0.0);
        assert_eq!(voll.werte().len(), RANGKOMPONENTEN.len());
        let eine = Rangkomponenten {
            bandpassung: 1.0,
            ..Default::default()
        };
        // Eine einzelne Eins ergibt ein Sechstel — sie kann die anderen fuenf
        // nicht ersetzen (M-20 in Zahlen).
        assert!((eine.rang() - 1.0 / 6.0).abs() < 1e-5);
        // NaN in einer Komponente vergiftet den Rang nicht.
        let vergiftet = Rangkomponenten {
            bandpassung: f64::NAN,
            koinzidenz: 1.0,
            ..Default::default()
        };
        assert!(vergiftet.rang().is_finite());
    }

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

    /// Die Bandraender: 0 ist ein gueltiger Index, 221 ein gueltiges `bis`,
    /// und ein leeres Intervall ist keines.
    #[test]
    fn bandraender_fallen_von_beiden_seiten() {
        assert!(Bandintervall { von: 0, bis: 1 }.gueltig());
        assert!(Bandintervall { von: 220, bis: 221 }.gueltig());
        assert!(!Bandintervall { von: 5, bis: 5 }.gueltig());
        assert!(!Bandintervall { von: 6, bis: 5 }.gueltig());
        assert!(!Bandintervall { von: 0, bis: 222 }.gueltig());
        // Jede feine Bandnummer liegt in genau einer der 64 Gruppen, und die
        // Abbildung verlaesst den Wertebereich an keinem Rand.
        for band in 0..BAENDER_FEIN {
            assert!(gruppe_von_band(band) < GRUPPEN_LIVE);
        }
        assert_eq!(gruppe_von_band(0), 0);
        assert_eq!(gruppe_von_band(BAENDER_FEIN - 1), GRUPPEN_LIVE - 1);
    }

    /// „Unabhaengig" heisst: dasselbe Projektfenster zweimal ist EINS. Ohne
    /// diese Verdichtung liesse sich das Gate durch blosses Nachsenden
    /// desselben Fensters ueberlisten.
    #[test]
    fn unabhaengige_fenster_verdichten_wiederholungen() {
        let a = fenster(0, 512, 1);
        let b = fenster(0, 512, 1);
        let c = fenster(512, 1024, 1);
        let d = fenster(0, 512, 2); // andere Epoche = anderes Fenster
        assert_eq!(unabhaengige_fenster(&[&a, &b]), 1);
        assert_eq!(unabhaengige_fenster(&[&a, &c]), 2);
        assert_eq!(unabhaengige_fenster(&[&a, &d]), 2);
        assert_eq!(unabhaengige_fenster(&[]), 0);
        // GENAU an der Schwelle: acht verschiedene Fenster sind acht.
        let acht: Vec<Evidenzfenster> = (0..8)
            .map(|i| fenster(i * 512, (i + 1) * 512, 1))
            .collect();
        assert_eq!(
            unabhaengige_fenster(&acht.iter().collect::<Vec<_>>()),
            GATE_MINDEST_FENSTER
        );
    }

    /// Ein Band ohne Bit hat KEINEN Wert — nie 0, nie NaN.
    #[test]
    fn band_ohne_bit_traegt_keinen_wert() {
        let mut f = fenster(0, 512, 1);
        f.p50_gueltig[3] = false;
        f.p50_db[7] = f32::NAN;
        assert_eq!(f.band(0), Some(-20.0));
        assert_eq!(f.band(3), None, "ohne Bit gibt es keinen Wert");
        assert_eq!(f.band(7), None, "und NaN ist auch keiner");
        assert_eq!(f.band(BAENDER_FEIN), None, "und ausserhalb erst recht");
    }

    /// Die Korrelation ist keine Gefaelligkeit: zwei Konstanten sind kein
    /// Zusammenhang, und eine zu kurze Reihe ist keiner.
    ///
    /// *(NAK-212 Nacharbeit 1, NR-01: dieselbe Zusage, gemessen auf
    /// `korrelation_gerichtet`. Der frühere `f64`-Wrapper `korrelation` ist
    /// mit dem Fix arbeitslos geworden — er hätte „nicht messbar" und
    /// „gemessen null" wieder unter einer Zahl zusammengefasst, und niemand
    /// im Produkt hat noch danach gefragt.)*
    #[test]
    fn korrelation_ist_streng() {
        assert_eq!(korrelation_gerichtet(&[1.0, 1.0, 1.0], &[2.0, 2.0, 2.0]), None);
        assert_eq!(korrelation_gerichtet(&[1.0], &[2.0]), None);
        assert_eq!(korrelation_gerichtet(&[], &[]), None);
        assert_eq!(korrelation_gerichtet(&[1.0, 2.0], &[1.0, 2.0, 3.0]), None);
        let r = korrelation_gerichtet(&[1.0, 2.0, 3.0], &[2.0, 4.0, 6.0]).expect("messbar");
        assert!((r - 1.0).abs() < 1e-9, "{r}");
        let g = korrelation_gerichtet(&[1.0, 2.0, 3.0], &[6.0, 4.0, 2.0]).expect("messbar");
        assert!((g + 1.0).abs() < 1e-9, "{g}");
        // NaN in einem Paar nimmt das Paar heraus, statt die Reihe zu
        // vergiften.
        assert!(korrelation_gerichtet(&[1.0, 2.0, f64::NAN], &[2.0, 4.0, 8.0])
            .is_some_and(f64::is_finite));
    }

    /// 🔑 **NAK-212 Nacharbeit 1, NR-01.** Eine MESSBARE Nullkorrelation ist
    /// `Some(0.0)`, keine Enthaltung.
    ///
    /// `korrelation` faellt in fuenf Lagen auf 0,0 zurueck — vier davon sind
    /// „nicht messbar", die fuenfte ist ein Messwert. Wer die Lagen an der
    /// Zahl 0,0 wiedererkennen will, trifft zwangslaeufig auch die echte
    /// Nullkorrelation: `zusammenhang_verschieden` (E6) haelt dann ein
    /// getrenntes Paar fuer ungetrennt, und der Fuehrende bleibt `mittel`.
    #[test]
    fn korrelation_gerichtet_trennt_gemessene_null_von_nicht_messbar() {
        // Beide Reihen STREUEN, ihre Korrelation ist exakt null. Das ist ein
        // Messwert — der Fall aus dem Erstpruefungsurteil.
        let a: Vec<f64> = [1.0, 1.0, 3.0, 3.0].repeat(3);
        let b: Vec<f64> = [1.0, 3.0, 1.0, 3.0].repeat(3);
        assert_eq!(
            korrelation_gerichtet(&a, &b),
            Some(0.0),
            "zwei streuende Reihen ohne Zusammenhang sind GEMESSEN null"
        );
        // Und die vier Lagen, in denen wirklich nichts messbar ist.
        assert_eq!(korrelation_gerichtet(&[1.0, 1.0, 1.0], &[2.0, 4.0, 6.0]), None,
                   "eine Reihe ohne Streuung");
        assert_eq!(korrelation_gerichtet(&[2.0, 4.0, 6.0], &[1.0, 1.0, 1.0]), None,
                   "die ANDERE Reihe ohne Streuung");
        assert_eq!(korrelation_gerichtet(&[1.0], &[2.0]), None, "Laenge 1");
        assert_eq!(korrelation_gerichtet(&[], &[]), None, "leer");
        assert_eq!(korrelation_gerichtet(&[1.0, 2.0], &[1.0, 2.0, 3.0]), None,
                   "ungleich lang");
        assert_eq!(korrelation_gerichtet(&[1.0, f64::NAN], &[2.0, 4.0]), None,
                   "nach dem NaN-Filter bleibt EIN Paar");
        assert_eq!(korrelation_gerichtet(&[f64::INFINITY, 1.0, 2.0],
                                         &[1.0, f64::NAN, 3.0]), None,
                   "Inf und NaN nehmen ihre Paare heraus — eines bleibt");
        // Vorzeichen bleiben, wie sie waren.
        let r = korrelation_gerichtet(&[1.0, 2.0, 3.0], &[2.0, 4.0, 6.0]).expect("messbar");
        assert!((r - 1.0).abs() < 1e-9, "{r}");
        let g = korrelation_gerichtet(&[1.0, 2.0, 3.0], &[6.0, 4.0, 2.0]).expect("messbar");
        assert!((g + 1.0).abs() < 1e-9, "{g}");
        // Fuer den RANG bleibt beides null: `koinzidenz` klemmt auf [0, 1],
        // und `None` wie `Some(0.0)` tragen dort denselben Wert. Der
        // Unterschied wirkt allein ueber die KLASSE und die Trennung (E6) —
        // die Reduktionsreihenfolge aus M-25 ist unberuehrt.
    }

    /// 🔑 **NAK-212 Nacharbeit 1, NR-01, Folge in E6.** `Some(0.0)` und
    /// `None` sind fuer die Trennung ZWEI Werte.
    ///
    /// Das ist die Stelle, an der die Enthaltung der Korrelation wirklich
    /// weh tat: zwei Kandidaten mit gleichem positivem Uplift, deren einer
    /// eine gemessene Nullkoinzidenz traegt und deren anderer gar keine,
    /// sind im Zusammenhang unterscheidbar.
    #[test]
    fn gemessene_null_trennt_vom_nicht_gemessenen() {
        let mit_null = Zusammenhangsbeleg {
            uplift: Some(0.6),
            koinzidenz: Some(0.0),
            wiederholbarkeit: 0.5,
        };
        let ohne_messung = Zusammenhangsbeleg {
            uplift: Some(0.6),
            koinzidenz: None,
            wiederholbarkeit: 0.5,
        };
        assert!(
            zusammenhang_verschieden(&mit_null, &ohne_messung),
            "gemessen null ist nicht dasselbe wie nicht gemessen"
        );
        // Und beide sind positiv belegt (ueber den Uplift), keiner ein
        // Gegenbeleg — die Nullkoinzidenz aendert R1 und R2 nicht.
        assert!(mit_null.positiv_belegt() && ohne_messung.positiv_belegt());
        assert!(!mit_null.gegenbeleg() && !ohne_messung.gegenbeleg());
    }

    /// Die Projektspanne saettigt an den `i64`-Raendern, statt zu panieren
    /// oder umzubrechen.
    #[test]
    fn spanne_saettigt_an_den_raendern() {
        let a = fenster(i64::MIN, i64::MIN + 512, 1);
        let b = fenster(i64::MAX - 512, i64::MAX, 1);
        let (von, bis) = spanne(&[a, b]);
        assert_eq!(von, i64::MIN);
        assert_eq!(bis, i64::MAX);
        assert_eq!(spanne(&[]), (0, 0));
        // Zwei IDENTISCHE Fenster ueber den ganzen Zahlenbereich haben eine
        // Ueberdeckung von 1, nicht 0 — derselbe Fehler, den
        // `vergleichbarkeit::ueberdeckung` einmal gemacht hat.
        assert_eq!(
            zeitueberdeckung((i64::MIN, i64::MAX), (i64::MIN, i64::MAX)),
            1.0
        );
        assert_eq!(zeitueberdeckung((0, 0), (0, 10)), 0.0);
    }

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

    /// Ohne Vollstaendigkeitsmarke rechnet das Modul gar nicht (M-86). Der
    /// Riegel steht doppelt — hier und in der Verdrahtung.
    #[test]
    fn ohne_vollstaendigkeitsmarke_rechnet_das_modul_nicht() {
        let aufnahme = Aufnahme {
            mains: vec!["m".into()],
            master: Some(Quellprofil {
                quelle_id: "m".into(),
                fenster: vec![fenster(0, 512, 1)],
                routing_bekannt: true,
                ..Default::default()
            }),
            intent: Some(IntentBestand {
                vollstaendig: false,
                revision: 1,
                ..Default::default()
            }),
            ..Default::default()
        };
        assert!(hypothesen(&aufnahme).befunde.is_empty());
    }

    fn masterprofil() -> Quellprofil {
        Quellprofil {
            quelle_id: "m".into(),
            fenster: (0..12)
                .map(|i| fenster(i * 512, (i + 1) * 512, 1))
                .collect(),
            routing_bekannt: true,
            ..Default::default()
        }
    }

    /// Die Gate-Reihenfolge ist Absicht: erst die Belege, dann die Passage,
    /// dann der Wille des Users. Ein Kandidat ohne Evidenz braucht keinen
    /// Intent-Blick.
    #[test]
    fn gate_faellt_in_der_zugesagten_reihenfolge() {
        let master = masterprofil();
        let aufnahme = Aufnahme {
            mains: vec![master.quelle_id.clone()],
            master: Some(master.clone()),
            metrics_version: 1,
            ..Default::default()
        };
        let band = Bandintervall { von: 0, bis: 4 };

        // Ohne Fenster: Ruecknahme, nicht Coverage.
        let leer = Quellprofil {
            quelle_id: "k".into(),
            ..Default::default()
        };
        assert_eq!(
            gate(&leer, &aufnahme, &master, Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::EvidenzZurueckgenommen)
        );

        // Ohne Stereo: Capability, nicht Coverage.
        let kandidat = Quellprofil {
            quelle_id: "k".into(),
            fenster: master.fenster.clone(),
            routing_bekannt: true,
            ..Default::default()
        };
        assert_eq!(
            gate(&kandidat, &aufnahme, &master, Zielmetrik::StereoSeitenanteilDb, band),
            Gateurteil::Faellt(Ausschlussgrund::CapabilityFehlt)
        );

        // 🔑 **NAK-213 K-22 (R3/E6):** der Mixerkanal des Masters steht an
        // Position 1 — NACH der Evidenz, VOR der Capability.
        let mut auf_dem_masterkanal = Aufnahme {
            mains: vec![master.quelle_id.clone()],
            master: Some(Quellprofil {
                mixerkanal: Some(1),
                ..master.clone()
            }),
            metrics_version: 1,
            ..Default::default()
        };
        let masterkanal_ref =
            |a: &Aufnahme| a.master.clone().expect("die Aufnahme traegt einen Master");
        let sonde_auf_master = Quellprofil {
            mixerkanal: Some(1),
            ..kandidat.clone()
        };
        assert_eq!(
            gate(
                &sonde_auf_master,
                &auf_dem_masterkanal,
                &masterkanal_ref(&auf_dem_masterkanal),
                // Selbst mit einer Metrik, deren Capability FEHLT, faellt sie
                // zuerst am Masterkanal: die Reihenfolge ist Absicht.
                Zielmetrik::StereoSeitenanteilDb,
                band
            ),
            Gateurteil::Faellt(Ausschlussgrund::MasterDuplikat)
        );
        // Aber Schritt 0 bleibt VORN: eine STUMME Sonde auf dem Masterkanal
        // faellt weiter mit `evidenz_zurueckgenommen` — sie dupliziert nichts.
        let stumm_auf_master = Quellprofil {
            mixerkanal: Some(1),
            fenster: Vec::new(),
            ..kandidat.clone()
        };
        assert_eq!(
            gate(
                &stumm_auf_master,
                &auf_dem_masterkanal,
                &masterkanal_ref(&auf_dem_masterkanal),
                Zielmetrik::BandPegelDb,
                band
            ),
            Gateurteil::Faellt(Ausschlussgrund::EvidenzZurueckgenommen)
        );
        // **K-24:** Kanal 1 IST ein Kanal — der kleinste, den der Vertrag
        // kennt. Eine Sonde auf einem anderen bleibt.
        let anderer_kanal = Quellprofil {
            mixerkanal: Some(2),
            ..kandidat.clone()
        };
        assert_eq!(
            gate(&anderer_kanal, &auf_dem_masterkanal, &masterkanal_ref(&auf_dem_masterkanal),
                 Zielmetrik::BandPegelDb, band),
            Gateurteil::Bleibt
        );
        // **K-25:** ohne Masterkanal gibt es keine Masterkanalgruppe. Ein
        // Rueckfall auf einen Standardkanal erklaerte jede Sonde ohne Not zum
        // Duplikat.
        auf_dem_masterkanal.master.as_mut().expect("Master").mixerkanal = None;
        assert_eq!(
            gate(&sonde_auf_master, &auf_dem_masterkanal, &masterkanal_ref(&auf_dem_masterkanal),
                 Zielmetrik::BandPegelDb, band),
            Gateurteil::Bleibt
        );
        // Und umgekehrt: eine Sonde OHNE Kanal teilt keinen.
        auf_dem_masterkanal.master.as_mut().expect("Master").mixerkanal = Some(1);
        let ohne_kanal = Quellprofil {
            mixerkanal: None,
            ..kandidat.clone()
        };
        assert_eq!(
            gate(&ohne_kanal, &auf_dem_masterkanal, &masterkanal_ref(&auf_dem_masterkanal),
                 Zielmetrik::BandPegelDb, band),
            Gateurteil::Bleibt
        );

        // Duenne Messung: Coverage.
        let mut duenn = kandidat.clone();
        for f in duenn.fenster.iter_mut() {
            f.abdeckung = 0.1;
        }
        assert_eq!(
            gate(&duenn, &aufnahme, &master, Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::CoverageFehlt)
        );

        // GENAU an der Coverage-Kante: 0,5 bleibt drin, knapp darunter nicht.
        let mut kante = kandidat.clone();
        for f in kante.fenster.iter_mut() {
            f.abdeckung = GATE_ABDECKUNG as f64;
        }
        assert_eq!(
            gate(&kante, &aufnahme, &master, Zielmetrik::BandPegelDb, band),
            Gateurteil::Bleibt
        );
        for f in kante.fenster.iter_mut() {
            f.abdeckung = GATE_ABDECKUNG as f64 - 1e-6;
        }
        assert_eq!(
            gate(&kante, &aufnahme, &master, Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::CoverageFehlt)
        );

        // Anderes Zeitfenster: Alignment.
        let mut verschoben = kandidat.clone();
        for f in verschoben.fenster.iter_mut() {
            f.projekt_von += 1_000_000;
            f.projekt_bis += 1_000_000;
        }
        assert_eq!(
            gate(&verschoben, &aufnahme, &master, Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::AlignmentFalsch)
        );

        // Der Wille des Users steht am Ende — und ueberstimmt alles.
        let mut intent = IntentBestand {
            vollstaendig: true,
            revision: 1,
            ..Default::default()
        };
        intent.schutzangaben.insert(SchutzangabeSpiegel {
            quelle_id: "k".into(),
            eigenschaft: "band".into(),
            band: Some((0, 4)),
        });
        let mit_schutz = Aufnahme {
            mains: vec![master.quelle_id.clone()],
            master: Some(master.clone()),
            intent: Some(intent),
            metrics_version: 1,
            ..Default::default()
        };
        assert_eq!(
            gate(&kandidat, &mit_schutz, mit_schutz.master.as_ref().expect("die Aufnahme traegt einen Master"), Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::IntentVetoGeschuetzt)
        );

        // Ein Schutz NEBEN dem Band sperrt nicht: er ist orthogonal (M-03).
        let mut daneben = IntentBestand {
            vollstaendig: true,
            revision: 1,
            ..Default::default()
        };
        daneben.schutzangaben.insert(SchutzangabeSpiegel {
            quelle_id: "k".into(),
            eigenschaft: "band".into(),
            band: Some((100, 120)),
        });
        let mit_fernem_schutz = Aufnahme {
            mains: vec![master.quelle_id.clone()],
            master: Some(master),
            intent: Some(daneben),
            metrics_version: 1,
            ..Default::default()
        };
        assert_eq!(
            gate(&kandidat, &mit_fernem_schutz, mit_fernem_schutz.master.as_ref().expect("die Aufnahme traegt einen Master"), Zielmetrik::BandPegelDb, band),
            Gateurteil::Bleibt
        );
    }

    /// Die Passagengates: verschoben, andere Epoche, zu kurz — drei Faelle,
    /// drei eigene Gruende (M-23).
    #[test]
    fn passagengates_trennen_verschoben_anders_und_zu_kurz() {
        let master = masterprofil();
        let kandidat = Quellprofil {
            quelle_id: "k".into(),
            fenster: master.fenster.clone(),
            routing_bekannt: true,
            ..Default::default()
        };
        let band = Bandintervall { von: 0, bis: 4 };
        let passage = Passagenfenster {
            projekt_von: 0,
            projekt_bis: 12 * 512,
            transport_epoch: 1,
        };
        let mit_passage = Aufnahme {
            mains: vec![master.quelle_id.clone()],
            master: Some(master.clone()),
            passage: Some(passage),
            passage_id: Some("p".into()),
            metrics_version: 1,
            ..Default::default()
        };
        assert_eq!(
            gate(&kandidat, &mit_passage, mit_passage.master.as_ref().expect("die Aufnahme traegt einen Master"), Zielmetrik::BandPegelDb, band),
            Gateurteil::Bleibt
        );

        // ZU KURZ: drei Fenster in derselben Passage.
        let kurz = Quellprofil {
            quelle_id: "k".into(),
            fenster: (0..3)
                .map(|i| fenster(i * 512, (i + 1) * 512, 1))
                .collect(),
            routing_bekannt: true,
            ..Default::default()
        };
        let master_kurz = Quellprofil {
            fenster: kurz.fenster.clone(),
            ..master.clone()
        };
        let kurze_passage = Aufnahme {
            mains: vec![master_kurz.quelle_id.clone()],
            master: Some(master_kurz),
            passage: Some(Passagenfenster {
                projekt_von: 0,
                projekt_bis: 3 * 512,
                transport_epoch: 1,
            }),
            passage_id: Some("p".into()),
            metrics_version: 1,
            ..Default::default()
        };
        assert_eq!(
            gate(&kurz, &kurze_passage, kurze_passage.master.as_ref().expect("die Aufnahme traegt einen Master"), Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::PassageZuKurz),
            "drei identisch kurze Fenster bestehen alle vier relativen Gates"
        );

        // ANDERE Epoche: unvergleichbar, nicht „zu kurz".
        //
        // 🔑 **NAK-212 R4 (07.09.2026).** Kandidat UND Master liegen auf
        // Epoche 9, die Passage auf Epoche 1. Bis hierher lag nur der
        // Kandidat auf einer fremden Epoche — seit das Alignment paarweise
        // misst (`ueberlappt` prueft die Epoche mit), faellt so ein Kandidat
        // schon an Schritt 3 mit `AlignmentFalsch`, und der Fall maesse das
        // Passagengate gar nicht mehr. Beide Gruende stehen in der
        // geschlossenen Menge (M-87), die Zusage aus M-23 („eine ANDERE
        // Passage traegt keine starke Aussage") haelt in beiden Formen — der
        // Aufbau hier isoliert sie am richtigen Gate.
        let andere = Quellprofil {
            quelle_id: "k".into(),
            fenster: (0..12)
                .map(|i| fenster(i * 512, (i + 1) * 512, 9))
                .collect(),
            routing_bekannt: true,
            ..Default::default()
        };
        let fremde_passage = Aufnahme {
            mains: vec![master.quelle_id.clone()],
            master: Some(Quellprofil {
                fenster: andere.fenster.clone(),
                ..master.clone()
            }),
            passage: Some(passage),
            passage_id: Some("p".into()),
            metrics_version: 1,
            ..Default::default()
        };
        assert_eq!(
            gate(&andere, &fremde_passage, fremde_passage.master.as_ref().expect("die Aufnahme traegt einen Master"), Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::PassageUnvergleichbar)
        );

        // Und die Gegenprobe zu R4: liegt NUR der Kandidat auf einer fremden
        // Epoche, hat er mit dem Master kein gemeinsames Zeitfenster — das
        // ist falsches Alignment, nicht eine andere Passage (N-22).
        assert_eq!(
            gate(&andere, &mit_passage, mit_passage.master.as_ref().expect("die Aufnahme traegt einen Master"), Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::AlignmentFalsch)
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
