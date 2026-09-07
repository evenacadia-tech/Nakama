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
//! ── AUSSCHLUSS IST NICHT RANGSCHNITT ──────────────────────────────────────
//!
//! Die geschlossene Menge in `Ausschlussgrund` kennt bewusst keinen Wert
//! „Rang zu niedrig". Ein Kandidat scheidet AUS DEM RANKING aus, wenn er ein
//! Gate reisst; das ist ein Befund und traegt einen Grund. Der harte Deckel
//! aus M-18 (hoechstens fuenf je Befund) schneidet DANACH und trifft nur
//! Kandidaten, die jedes Gate bestanden haben — sie sind nicht ausgeschlossen,
//! sondern ueberboten, und die deterministische Rangfolge ist ihr Protokoll.

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

/// Die ACHT Ausschlussgruende (M-87, R4). Geschlossen und beidseitig geprueft.
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
}

pub const AUSSCHLUSSGRUENDE: [&str; 8] = [
    "coverage_fehlt",
    "alignment_falsch",
    "passage_unvergleichbar",
    "passage_zu_kurz",
    "intent_veto_geschuetzt",
    "intent_veto_verschmolzen",
    "capability_fehlt",
    "evidenz_zurueckgenommen",
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

    pub const ALLE: [Ausschlussgrund; 8] = [
        Self::CoverageFehlt,
        Self::AlignmentFalsch,
        Self::PassageUnvergleichbar,
        Self::PassageZuKurz,
        Self::IntentVetoGeschuetzt,
        Self::IntentVetoVerschmolzen,
        Self::CapabilityFehlt,
        Self::EvidenzZurueckgenommen,
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
    pub master: Quellprofil,
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
    metrik: Zielmetrik,
    band: Bandintervall,
) -> Gateurteil {
    // 0. Rueckgenommene Evidenz (M-24). Sie steht vor allem anderen: ein
    //    Kandidat ohne gueltigen Beleg hat keine Groesse, die man messen
    //    koennte.
    if kandidat.zurueckgenommen || kandidat.fenster.is_empty() {
        return Gateurteil::Faellt(Ausschlussgrund::EvidenzZurueckgenommen);
    }

    // 1. Capability: traegt der Beleg ueberhaupt die Datenklasse der Metrik?
    if !kandidat.fenster.iter().any(|f| metrik.braucht(f)) {
        return Gateurteil::Faellt(Ausschlussgrund::CapabilityFehlt);
    }

    // 2. Coverage (M-20). Das MITTEL ueber die Fenster, nicht das Maximum:
    //    ein einziges dichtes Fenster macht eine duenne Messung nicht dicht.
    let abdeckung = mittel(kandidat.fenster.iter().map(|f| f.abdeckung));
    if !(abdeckung >= GATE_ABDECKUNG as f64) {
        return Gateurteil::Faellt(Ausschlussgrund::CoverageFehlt);
    }

    // 3. Alignment (M-20): liegen die Fenster des Kandidaten wirklich dort,
    //    wo der Master seinen Befund hat? Gemessen als Zeitueberdeckung der
    //    beiden Fensterspannen, mit demselben Gate wie §43.2.
    let alignment = zeitueberdeckung(spanne(&kandidat.fenster), spanne(&aufnahme.master.fenster));
    if !(alignment >= GATE_ZEITUEBERDECKUNG) {
        return Gateurteil::Faellt(Ausschlussgrund::AlignmentFalsch);
    }

    // 4. Die Passage, falls die Sitzung eine fuehrt (M-23).
    if let Some(p) = aufnahme.passage {
        // 4a. VERSCHOBEN oder ANDERS: eine andere Transportepoche ist eine
        //     andere Passage, und eine verschobene reisst die Zeitueberdeckung.
        let in_passage: Vec<&Evidenzfenster> = kandidat
            .fenster
            .iter()
            .filter(|f| {
                f.transport_epoch == p.transport_epoch
                    && f.projekt_bis > p.projekt_von
                    && f.projekt_von < p.projekt_bis
            })
            .collect();
        let ueberdeckung = zeitueberdeckung(
            spanne(&kandidat.fenster),
            (p.projekt_von, p.projekt_bis),
        );
        if kandidat.fenster.iter().any(|f| f.transport_epoch != p.transport_epoch)
            || !(ueberdeckung >= GATE_ZEITUEBERDECKUNG)
        {
            return Gateurteil::Faellt(Ausschlussgrund::PassageUnvergleichbar);
        }
        // 4b. ZU KURZ (R1): die absolute Groesse, die die vier relativen
        //     Gates nicht messen koennen. Gezaehlt werden UNABHAENGIGE
        //     Fenster — zwei Belege desselben Projektfensters sind einer.
        if unabhaengige_fenster(&in_passage) < GATE_MINDEST_FENSTER {
            return Gateurteil::Faellt(Ausschlussgrund::PassageZuKurz);
        }
    }

    // 5. Der Wille des Users (M-03, M-04). Er steht am Ende, weil er die
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
        if !intent.entmaskierung_erlaubt(&aufnahme.master.quelle_id, &kandidat.quelle_id, &passage_id)
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
    band: Bandintervall,
    parent_duplikat: bool,
) -> Rangkomponenten {
    Rangkomponenten {
        bandpassung: bandpassung(kandidat, band),
        koinzidenz: koinzidenz(kandidat, &aufnahme.master),
        uplift: uplift(kandidat, &aufnahme.master, band),
        intent_relevanz: intent_relevanz(kandidat, aufnahme),
        wiederholbarkeit: wiederholbarkeit(kandidat, &aufnahme.master, band),
        routingqualitaet: routingqualitaet(kandidat, parent_duplikat),
    }
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

/// Zeitliche Koinzidenz der Onsetspuren von Kandidat und Master.
///
/// Die Pearson-Korrelation der beiden Onsetreihen ueber die gemeinsamen
/// Fenster, negativ auf 0 geklemmt: eine gegenlaeufige Spur ist KEIN
/// Zusammenhang, sondern ein Gegenbeleg — und ein Gegenbeleg darf keinen Rang
/// erzeugen.
fn koinzidenz(kandidat: &Quellprofil, master: &Quellprofil) -> f64 {
    let (a, b) = gemeinsame_reihen(kandidat, master, |f| f.onset as f64);
    korrelation(&a, &b).max(0.0)
}

/// Der BEDINGTE Uplift (M-19): die Zielmetrik des Masters in Fenstern MIT
/// aktivem Kandidaten gegen aehnliche Fenster OHNE diese Aktivitaet.
///
/// ⚠️ Ohne Vergleichsfenster gibt es keinen Uplift — nicht 0, sondern gar
/// keinen. Genau daran faellt der Rotbeweis von M-19: wer den Uplift ohne die
/// Menge „ohne die Quelle" rechnet, misst nur den Pegel des Masters.
fn uplift(kandidat: &Quellprofil, master: &Quellprofil, band: Bandintervall) -> f64 {
    let reihen = upliftreihe(kandidat, master, band);
    let Some((mit, ohne)) = reihen else {
        return 0.0;
    };
    if mit.is_empty() || ohne.is_empty() {
        return 0.0;
    }
    let differenz = mittel(mit.iter().copied()) - mittel(ohne.iter().copied());
    if !differenz.is_finite() || differenz <= 0.0 {
        return 0.0;
    }
    // Normiert auf die eigene Spanne des Masters in diesem Band: eine
    // Erhoehung um 3 dB heisst wenig, wenn der Master dort ohnehin um 20 dB
    // schwankt, und viel, wenn er still steht. Eine feste dB-Schwelle waere
    // eine unkalibrierte Konstante.
    let alle: Vec<f64> = mit.iter().chain(ohne.iter()).copied().collect();
    let spanne = spannweite(&alle);
    if spanne <= 0.0 {
        return 0.0;
    }
    (differenz / spanne).clamp(0.0, 1.0)
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
    let p = super::experiment::bootstrap_p(
        &reihe,
        super::experiment::BOOTSTRAP_BLOCK,
        super::experiment::BOOTSTRAP_ZIEHUNGEN,
        BOOTSTRAP_SAAT,
    );
    (1.0 - p).clamp(0.0, 1.0)
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
    let Some((metrik, band, beobachtung)) = masteranomalie(&aufnahme.master) else {
        return Rechenergebnis::default();
    };

    // Parent-Duplikate erkennen, BEVOR gerangt wird (M-22).
    let ids: std::collections::BTreeSet<&str> = aufnahme
        .kandidaten
        .iter()
        .map(|k| k.quelle_id.as_str())
        .collect();
    let ist_parent = |k: &Quellprofil| {
        aufnahme
            .kandidaten
            .iter()
            .any(|anderer| anderer.parent.as_deref() == Some(k.quelle_id.as_str()))
    };

    let mut ausschluesse: Vec<Ausschluss> = Vec::new();
    let mut ueberlebende: Vec<(Rangkomponenten, &Quellprofil, bool)> = Vec::new();
    for kandidat in &aufnahme.kandidaten {
        match gate(kandidat, aufnahme, metrik, band) {
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
                let rang = rangkomponenten(kandidat, aufnahme, band, duplikat);
                ueberlebende.push((rang, kandidat, duplikat));
            }
        }
    }
    ausschluesse.sort();
    ausschluesse.dedup();

    // Deterministische Rangfolge (M-25): quantisierter Rang absteigend, dann
    // `candidate_source` aufsteigend. Erst DANACH der harte Deckel aus M-18.
    ueberlebende.sort_by(|a, b| {
        let ra = (quantisiert(a.0.rang()) / RANG_QUANTUM).round() as i64;
        let rb = (quantisiert(b.0.rang()) / RANG_QUANTUM).round() as i64;
        rb.cmp(&ra).then_with(|| a.1.quelle_id.cmp(&b.1.quelle_id))
    });
    ueberlebende.truncate(KANDIDATEN_DECKEL);

    if ueberlebende.is_empty() {
        // M-27: kein Kandidat ist ein ERGEBNIS. Der Befund traegt die siebte
        // Ursachenklasse, keinen Fehlercode — und er bleibt sichtbar, damit
        // der User sieht, dass gerechnet wurde.
        let befund = enthaltung(aufnahme, metrik, band, beobachtung, ausschluesse.clone());
        return Rechenergebnis {
            befunde: vec![befund],
            ausschluesse,
        };
    }

    let mehrere = ueberlebende.len() > 1;
    let mut befunde: Vec<CauseHypothesis> = ueberlebende
        .iter()
        .map(|(rang, kandidat, duplikat)| {
            baue_befund(aufnahme, kandidat, *rang, metrik, band, beobachtung, *duplikat, mehrere)
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

    // ⚠️ EIN UNGETRENNTER ERSTER PLATZ TRAEGT AUCH KEINE STARKE AUSSAGE.
    //
    // Der Riegel darueber allein reicht nicht, und der P5-Korpus hat genau
    // das gezeigt: in `korrelierter_distraktor` sind zwei Kandidaten im
    // Material NICHT unterscheidbar — gleiches Band, gleiche Anhebung,
    // gleiche Fenster. Wer dann fuehrt, entscheidet der Gleichstands-
    // schluessel, die aufsteigende `candidate_source`. Eine starke Aussage
    // auf diesem Platz behauptet eine Unterscheidung, die die Messung nicht
    // hergibt; sie waere richtig oder falsch, je nachdem wie die Kennungen
    // zufaellig liegen. Das ist die „ueberzeugende falsche Ursache" aus
    // §49.4 und die falsche starke Behauptung aus §36.4 Satz 1.
    //
    // GETRENNT heisst: der Abstand ist groesser als das Quantum, in dem
    // Raenge ueberhaupt verglichen werden (`RANG_QUANTUM`, M-25) — dieselbe
    // Aufloesung, die auch die Sortierung oben benutzt. Zwei Kandidaten, die
    // die Sortierung nicht trennen konnte, darf die Sicherheit nicht trennen.
    // Faellt der Abstand, faellt auch der fuehrende Befund auf `mittel`;
    // beide bleiben sichtbar, jeder als Alternative des anderen.
    let getrennt = match ueberlebende.as_slice() {
        [erster, zweiter, ..] => {
            let a = (quantisiert(erster.0.rang()) / RANG_QUANTUM).round() as i64;
            let b = (quantisiert(zweiter.0.rang()) / RANG_QUANTUM).round() as i64;
            a > b
        }
        // Ein einziger Kandidat hat niemanden, von dem er sich abheben muesste.
        _ => true,
    };
    if !getrennt && befunde[0].confidence.klasse >= Sicherheitsklasse::Hoch {
        befunde[0].confidence.klasse = Sicherheitsklasse::Mittel;
        befunde[0].zustand = zustand_aus_sicherheit(Sicherheitsklasse::Mittel, false);
    }

    // Der fuehrende Befund traegt die IDs der uebrigen als Alternativen und
    // die Ausschluesse (M-21, M-87). Die uebrigen sind EIGENE Befunde mit
    // eigenem Zustand — nicht sein Anhang.
    let weitere: Vec<String> = befunde[1..].iter().map(|b| b.finding_id.clone()).collect();
    befunde[0].alternatives = weitere;
    befunde[0].ausschluesse = ausschluesse.clone();

    Rechenergebnis {
        befunde,
        ausschluesse,
    }
}

/// Die Anomalie des Masters: welche Metrik, welches Band, welcher Wert.
///
/// Gemessen wird auf den **64 Gruppen** des Livegitters (M-18) — die Gruppe
/// mit der groessten Abweichung vom eigenen Bandmedian. Die Rueckgabe traegt
/// das FEINE Intervall dieser Gruppe, weil Stufe B auf dem 221er-Verlauf
/// rechnet (M-19).
pub fn masteranomalie(master: &Quellprofil) -> Option<(Zielmetrik, Bandintervall, Beobachtung)> {
    let letztes = master.fenster.last()?;
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
    let (gruppe, (abweichung, _)) = je_gruppe
        .iter()
        .enumerate()
        .filter(|(_, (a, _))| a.is_finite())
        .max_by(|a, b| {
            a.1 .0
                .partial_cmp(&b.1 .0)
                .unwrap_or(std::cmp::Ordering::Equal)
                .then(b.0.cmp(&a.0))
        })?;
    let von = (gruppe * BAENDER_FEIN / GRUPPEN_LIVE) as u32;
    let bis = (((gruppe + 1) * BAENDER_FEIN / GRUPPEN_LIVE).max(von as usize + 1)) as u32;
    let band = Bandintervall {
        von,
        bis: bis.min(BAENDER_FEIN as u32),
    };
    let beobachtung = Beobachtung {
        wert_db: basis + abweichung,
        gueltig: true,
    };
    Some((Zielmetrik::BandPegelDb, band, beobachtung))
}

/// Der Befund, wenn kein Kandidat ueberlebt hat (M-27).
fn enthaltung(
    aufnahme: &Aufnahme,
    metrik: Zielmetrik,
    band: Bandintervall,
    beobachtung: Beobachtung,
    ausschluesse: Vec<Ausschluss>,
) -> CauseHypothesis {
    let quelle = aufnahme.master.quelle_id.clone();
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
        evidence_ids: aufnahme
            .master
            .fenster
            .iter()
            .map(|f| f.evidence_id.clone())
            .collect(),
        alternatives: Vec::new(),
        ausschluesse,
        next_test: NaechsterTest::MehrDatenSammeln,
        zustand: zustand_aus_sicherheit(Sicherheitsklasse::Unklar, false),
        intent_revision: aufnahme.intent.as_ref().map_or(0, |i| i.revision),
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
    kandidat: &Quellprofil,
    rang: Rangkomponenten,
    metrik: Zielmetrik,
    band: Bandintervall,
    beobachtung: Beobachtung,
    parent_duplikat: bool,
    mehrere_kandidaten: bool,
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
    let fenster = unabhaengige_fenster(&kandidat.fenster.iter().collect::<Vec<_>>());
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
    let klasse = if fenster < GATE_MINDEST_FENSTER
        || !kandidat.routing_bekannt
        || parent_duplikat
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
        maskierung: super::maskierung::maskierung(&aufnahme.master, kandidat, band),
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
        let Some(mf) = master.fenster.iter().find(|m| {
            m.transport_epoch == kf.transport_epoch
                && m.projekt_bis > kf.projekt_von
                && m.projekt_von < kf.projekt_bis
        }) else {
            continue;
        };
        a.push(hole(kf));
        b.push(hole(mf));
    }
    (a, b)
}

/// Pearson-Korrelation zweier gleich langer Reihen. `0.0`, wenn eine der
/// beiden keine Streuung hat — nicht 1,0: zwei Konstanten sind kein
/// Zusammenhang.
fn korrelation(a: &[f64], b: &[f64]) -> f64 {
    if a.len() != b.len() || a.len() < 2 {
        return 0.0;
    }
    let paare: Vec<(f64, f64)> = a
        .iter()
        .zip(b.iter())
        .filter(|(x, y)| x.is_finite() && y.is_finite())
        .map(|(x, y)| (*x, *y))
        .collect();
    if paare.len() < 2 {
        return 0.0;
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
    if var_a <= 0.0 || var_b <= 0.0 {
        return 0.0;
    }
    let r = zaehler / (var_a.sqrt() * var_b.sqrt());
    if r.is_finite() {
        r
    } else {
        0.0
    }
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
        assert_eq!(AUSSCHLUSSGRUENDE.len(), 8);
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
    #[test]
    fn korrelation_ist_streng() {
        assert_eq!(korrelation(&[1.0, 1.0, 1.0], &[2.0, 2.0, 2.0]), 0.0);
        assert_eq!(korrelation(&[1.0], &[2.0]), 0.0);
        assert_eq!(korrelation(&[], &[]), 0.0);
        assert_eq!(korrelation(&[1.0, 2.0], &[1.0, 2.0, 3.0]), 0.0);
        let r = korrelation(&[1.0, 2.0, 3.0], &[2.0, 4.0, 6.0]);
        assert!((r - 1.0).abs() < 1e-9, "{r}");
        let g = korrelation(&[1.0, 2.0, 3.0], &[6.0, 4.0, 2.0]);
        assert!((g + 1.0).abs() < 1e-9, "{g}");
        // NaN in einem Paar nimmt das Paar heraus, statt die Reihe zu
        // vergiften.
        assert!(korrelation(&[1.0, 2.0, f64::NAN], &[2.0, 4.0, 8.0]).is_finite());
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
            master: Quellprofil {
                quelle_id: "m".into(),
                fenster: vec![fenster(0, 512, 1)],
                routing_bekannt: true,
                ..Default::default()
            },
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
            master: master.clone(),
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
            gate(&leer, &aufnahme, Zielmetrik::BandPegelDb, band),
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
            gate(&kandidat, &aufnahme, Zielmetrik::StereoSeitenanteilDb, band),
            Gateurteil::Faellt(Ausschlussgrund::CapabilityFehlt)
        );

        // Duenne Messung: Coverage.
        let mut duenn = kandidat.clone();
        for f in duenn.fenster.iter_mut() {
            f.abdeckung = 0.1;
        }
        assert_eq!(
            gate(&duenn, &aufnahme, Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::CoverageFehlt)
        );

        // GENAU an der Coverage-Kante: 0,5 bleibt drin, knapp darunter nicht.
        let mut kante = kandidat.clone();
        for f in kante.fenster.iter_mut() {
            f.abdeckung = GATE_ABDECKUNG as f64;
        }
        assert_eq!(
            gate(&kante, &aufnahme, Zielmetrik::BandPegelDb, band),
            Gateurteil::Bleibt
        );
        for f in kante.fenster.iter_mut() {
            f.abdeckung = GATE_ABDECKUNG as f64 - 1e-6;
        }
        assert_eq!(
            gate(&kante, &aufnahme, Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::CoverageFehlt)
        );

        // Anderes Zeitfenster: Alignment.
        let mut verschoben = kandidat.clone();
        for f in verschoben.fenster.iter_mut() {
            f.projekt_von += 1_000_000;
            f.projekt_bis += 1_000_000;
        }
        assert_eq!(
            gate(&verschoben, &aufnahme, Zielmetrik::BandPegelDb, band),
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
            master: master.clone(),
            intent: Some(intent),
            metrics_version: 1,
            ..Default::default()
        };
        assert_eq!(
            gate(&kandidat, &mit_schutz, Zielmetrik::BandPegelDb, band),
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
            master,
            intent: Some(daneben),
            metrics_version: 1,
            ..Default::default()
        };
        assert_eq!(
            gate(&kandidat, &mit_fernem_schutz, Zielmetrik::BandPegelDb, band),
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
            master: master.clone(),
            passage: Some(passage),
            passage_id: Some("p".into()),
            metrics_version: 1,
            ..Default::default()
        };
        assert_eq!(
            gate(&kandidat, &mit_passage, Zielmetrik::BandPegelDb, band),
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
            master: master_kurz,
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
            gate(&kurz, &kurze_passage, Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::PassageZuKurz),
            "drei identisch kurze Fenster bestehen alle vier relativen Gates"
        );

        // ANDERE Epoche: unvergleichbar, nicht „zu kurz".
        let andere = Quellprofil {
            quelle_id: "k".into(),
            fenster: (0..12)
                .map(|i| fenster(i * 512, (i + 1) * 512, 9))
                .collect(),
            routing_bekannt: true,
            ..Default::default()
        };
        assert_eq!(
            gate(&andere, &mit_passage, Zielmetrik::BandPegelDb, band),
            Gateurteil::Faellt(Ausschlussgrund::PassageUnvergleichbar)
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
