//! Die Proposal-Policy (SONDE-014 Etappe F, M-42 bis M-54 und M-63).
//!
//! ── EINE EMPFEHLUNG IST ZUERST EIN OBJEKT ─────────────────────────────────
//!
//! §42.1 wörtlich: „eine Empfehlung ist ZUERST ein validiertes, versioniertes
//! Objekt und erst danach Text." Deshalb entsteht hier ein `Proposal` mit
//! fünfzehn Feldern plus `revert` — und **kein** Satz. Die drei Anzeigezeilen
//! des Befunds sind bereits Felder (Etappe D); ein Vorschlagstext, der eine
//! Zahl nennt, die in keinem Feld steht, wäre die KI-Erklärschicht, die
//! `CLAUDE.md` ausschließt.
//!
//! ── DIE FÜNF SCHRITTE (§42.2) ─────────────────────────────────────────────
//!
//! 1. Der **Befundtyp** wählt eine geprüfte Aktionstemplate.
//! 2. Ein **Constraint-Solver** entfernt Aktionen, die Intent, Capability,
//!    Headroom, Messqualität oder Schutzbereiche verletzen.
//! 3. Für EQ eine **regularisierte Zielfunktion**: gewünschte
//!    Evidenzverbesserung bei möglichst wenig Bändern, Gain und spektraler
//!    Nebenwirkung.
//! 4. **Der kleinste sichere Kandidat gewinnt**; Alternativen bleiben sichtbar.
//! 5. `execution` wird gesetzt — in P5 **immer** `manual`.
//!
//! Die Reihenfolge ist tragend und wird gemessen: ein Solver, der nach der
//! Zielfunktion liefe, könnte einen Vorschlag optimieren, den der Intent gar
//! nicht zulässt, und ihn dann verwerfen — der User sähe die Rechenzeit und
//! nicht das Ergebnis.
//!
//! ── WAS IN P5 EIN VERTRAGSBRUCH IST ───────────────────────────────────────
//!
//! `execution = previewable` und `revert = dsp_revert` setzen beide eine
//! eigene DSP-Fähigkeit voraus, und die entsteht erst in P6 beziehungsweise
//! P7. Beide Werte stehen im Vertrag, weil der Vertrag sie ausdrücken können
//! muss, bevor ein Bau sie trägt; dass sie in P5 nicht vorkommen dürfen,
//! gehört dem Consumer — also diesem Modul. `dsp_revert` steht deshalb gar
//! nicht erst in der Rust-Menge: die geschlossene Menge des Vertrags hat drei
//! Werte, und ein vierter fällt beidseitig (M-77).
//!
//! ── DIE SICHERHEITSBUDGETS SIND PRODUKTGRENZEN ────────────────────────────
//!
//! Die Zahlen aus §42.3 sind **Vertragswerte**, keine Kalibrierung: sie stehen
//! wörtlich im Entwurf und nicht in `metriken-v1.json`. Eine engere
//! **Usergrenze** gewinnt immer — das ist der Punkt, an dem ein Property-Test
//! über zufällige Eingänge fällt, wenn jemand die Reihenfolge dreht.

use super::hypothese::{
    quantisiert, Bandintervall, Befundkonfidenz, Befundzustand, CauseHypothesis, Sicherheitsklasse,
    Ursachenklasse, Zielmetrik,
};
use super::intent::IntentBestand;

// ═════════════════════════════════════════════════════════════════════════
// Die geschlossenen Mengen
// ═════════════════════════════════════════════════════════════════════════

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Aktion {
    StaticEqCut,
    DynamicEqCut,
    Trim,
    ManualGuidance,
    NoChange,
    MoreData,
}

pub const AKTIONEN: [&str; 6] = [
    "static_eq_cut",
    "dynamic_eq_cut",
    "trim",
    "manual_guidance",
    "no_change",
    "more_data",
];

impl Aktion {
    pub fn wire(self) -> &'static str {
        match self {
            Self::StaticEqCut => "static_eq_cut",
            Self::DynamicEqCut => "dynamic_eq_cut",
            Self::Trim => "trim",
            Self::ManualGuidance => "manual_guidance",
            Self::NoChange => "no_change",
            Self::MoreData => "more_data",
        }
    }

    pub fn aus_wire(wort: &str) -> Option<Self> {
        AKTIONEN.iter().position(|w| *w == wort).map(|i| Self::ALLE[i])
    }

    pub const ALLE: [Aktion; 6] = [
        Self::StaticEqCut,
        Self::DynamicEqCut,
        Self::Trim,
        Self::ManualGuidance,
        Self::NoChange,
        Self::MoreData,
    ];

    /// Ob diese Aktion überhaupt am Klang eingreift.
    ///
    /// `no_change` und `more_data` sind **gültige Vorschläge** (M-46) und
    /// keine leeren Rückgabewerte — aber sie sind kein Eingriff, und deshalb
    /// tragen sie weder Parameter noch Grenzen.
    pub fn ist_eingriff(self) -> bool {
        matches!(self, Self::StaticEqCut | Self::DynamicEqCut | Self::Trim)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Wirkung {
    ReduceMaskingPressure,
    ReducePeakPressure,
    RestoreStereoBalance,
    None,
}

pub const WIRKUNGEN: [&str; 4] = [
    "reduce_masking_pressure",
    "reduce_peak_pressure",
    "restore_stereo_balance",
    "none",
];

impl Wirkung {
    pub fn wire(self) -> &'static str {
        WIRKUNGEN[self as usize]
    }
}

/// Der Rückweg als **Feld**, nicht als Versprechen im Text (E-06, M-43).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Rueckweg {
    ExperimentAbort,
    ManualOnly,
    NoneNeeded,
}

pub const RUECKWEGE: [&str; 3] = ["experiment_abort", "manual_only", "none_needed"];

impl Rueckweg {
    pub fn wire(self) -> &'static str {
        RUECKWEGE[self as usize]
    }

    pub fn aus_wire(wort: &str) -> Option<Self> {
        Some(match wort {
            "experiment_abort" => Self::ExperimentAbort,
            "manual_only" => Self::ManualOnly,
            "none_needed" => Self::NoneNeeded,
            // ⚠️ `dsp_revert` gehoert P7 und ist hier KEIN Zweig — nicht
            // „noch nicht unterstuetzt", sondern nicht in der Menge.
            _ => return None,
        })
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Ausfuehrung {
    Manual,
    Previewable,
}

impl Ausfuehrung {
    pub fn wire(self) -> &'static str {
        match self {
            Self::Manual => "manual",
            Self::Previewable => "previewable",
        }
    }
}

/// Die sieben Stopbedingungen aus §42.1.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Stopbedingung {
    GuardrailAbdeckung,
    GuardrailLoudness,
    GuardrailPeak,
    GuardrailTransient,
    GuardrailBreite,
    GuardrailGeschuetzt,
    KeineWiederholbareMasteraenderung,
}

pub const STOPBEDINGUNGEN: [&str; 7] = [
    "guardrail_abdeckung",
    "guardrail_loudness",
    "guardrail_peak",
    "guardrail_transient",
    "guardrail_breite",
    "guardrail_geschuetzt",
    "keine_wiederholbare_masteraenderung",
];

impl Stopbedingung {
    pub fn wire(self) -> &'static str {
        STOPBEDINGUNGEN[self as usize]
    }

    pub fn aus_wire(wort: &str) -> Option<Self> {
        STOPBEDINGUNGEN
            .iter()
            .position(|w| *w == wort)
            .map(|i| Self::ALLE[i])
    }

    pub const ALLE: [Stopbedingung; 7] = [
        Self::GuardrailAbdeckung,
        Self::GuardrailLoudness,
        Self::GuardrailPeak,
        Self::GuardrailTransient,
        Self::GuardrailBreite,
        Self::GuardrailGeschuetzt,
        Self::KeineWiederholbareMasteraenderung,
    ];
}

// ═════════════════════════════════════════════════════════════════════════
// Die Sicherheitsbudgets aus §42.3
// ═════════════════════════════════════════════════════════════════════════
//
// ⚠️ VERTRAGSWERTE, keine Kalibrierung. Sie stehen wörtlich im Entwurf und
// deshalb NICHT im Register `metriken-v1.json`: eine Kalibrierung dieser
// Zahlen wäre eine Produktentscheidung und keine Messung. §42.3 verlangt für
// jede Änderung eine neue Policy-Version und Regressionstests — genau das
// leistet die Benennung hier: wer eine Zahl ändert, ändert eine benannte
// Konstante, und die Property-Prüfung fällt darüber.

/// Höchstens **drei** EQ-Bänder je Vorschlag.
pub const BUDGET_EQ_BAENDER_MAX: u32 = 3;
/// Statischer EQ innerhalb **±3 dB**.
pub const BUDGET_STATISCH_DB: f64 = 3.0;
/// Dynamische Reduktion standardmäßig höchstens **1,5 dB**.
pub const BUDGET_DYNAMISCH_STANDARD_DB: f64 = 1.5;
/// Hard Cap der dynamischen Reduktion: **3 dB**.
pub const BUDGET_DYNAMISCH_HARD_CAP_DB: f64 = 3.0;
/// Höchstens **ein** musikalischer Eingriff je Assistentenschritt.
pub const BUDGET_EINGRIFFE_JE_SCHRITT: usize = 1;

/// Die Version des Erzeugers (§42.4).
///
/// Jeder angezeigte Zahlenwert lässt sich auf Proposal-Feld, Evidenz **und**
/// Generatorversion zurückführen. Ohne diese Zahl wäre der dritte Teil der
/// Zusage nicht einlösbar.
pub const GENERATORVERSION: u32 = 1;

// ═════════════════════════════════════════════════════════════════════════
// Das Bandgitter — die EINE Quelle der Frequenzen
// ═════════════════════════════════════════════════════════════════════════

/// Die Mittenfrequenz eines Bandes des eingefrorenen Evidenzgitters.
///
/// ── WARUM EINE REGEL UND KEINE TABELLE ────────────────────────────────────
///
/// Das Gitter `nakama_1_24_oct_30_18k_v1` trägt seine Erzeugungsregel selbst
/// mit (`herkunft.regel`): IEC 61260-1 mit `G = 2`, `b = 24`,
/// `f_r = 1000 Hz` und `x` von −121 bis 99, also
/// `f_m(x) = f_r · G^((2x+1)/(2b))`. Die Regel ist exakt, und eine 221 Werte
/// lange Tabelle im Broker wäre eine **Kopie** des eingefrorenen Gitters —
/// zwei Wahrheiten, die auseinanderlaufen können.
///
/// Ein eigener Fall hält die Regel gegen die **committete** Datei; damit ist
/// die Kopie vermieden UND die Übereinstimmung gemessen.
pub const GITTER_REFERENZ_HZ: f64 = 1000.0;
pub const GITTER_OKTAVVERHAELTNIS: f64 = 2.0;
pub const GITTER_BANDBREITENKENNWERT: f64 = 24.0;
pub const GITTER_X_MIN: i32 = -121;

pub fn bandmitte_hz(band: u32) -> f64 {
    let x = GITTER_X_MIN as f64 + band as f64;
    GITTER_REFERENZ_HZ
        * GITTER_OKTAVVERHAELTNIS.powf((2.0 * x + 1.0) / (2.0 * GITTER_BANDBREITENKENNWERT))
}

// ═════════════════════════════════════════════════════════════════════════
// Die Objektform
// ═════════════════════════════════════════════════════════════════════════

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Wertebereich {
    pub von: f64,
    pub bis: f64,
}

/// `parameters` aus §42.1 — strikt und ohne Pflichtfeld.
#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Parameter {
    pub frequency_hz: Option<f64>,
    pub q: Option<f64>,
    pub max_gain_db: Option<f64>,
}

/// `allowed_bounds` aus §42.1 — dieselbe Form, ein Intervall je Größe.
#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Grenzen {
    pub frequency_hz: Option<Wertebereich>,
    pub q: Option<Wertebereich>,
    pub gain_db: Option<Wertebereich>,
}

/// Eine geschützte Eigenschaft als **harte Constraint** (M-49), in derselben
/// Form wie der Intent — nicht als Wortliste.
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct Schutzconstraint {
    pub quelle_id: String,
    pub eigenschaft: String,
    pub band: Option<(u32, u32)>,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Proposal {
    pub proposal_id: String,
    /// `const 1` im Vertrag.
    pub proposal_schema: u32,
    pub target: String,
    pub base_revision: u64,
    pub passage_id: Option<String>,
    pub finding_id: Option<String>,
    pub action: Aktion,
    pub parameters: Parameter,
    pub allowed_bounds: Grenzen,
    pub evidence_ids: Vec<String>,
    pub expected_effect: Wirkung,
    pub protected_traits: Vec<Schutzconstraint>,
    pub listen_for: String,
    pub stop_if: Vec<Stopbedingung>,
    pub execution: Ausfuehrung,
    pub confidence: Befundkonfidenz,
    pub revert: Rueckweg,
    pub intent_revision: i64,
    pub generatorversion: u32,
}

/// Die sechs Felder, die das Exit-Gate wörtlich verlangt (M-43).
///
/// „Jedes Proposal nennt Ziel, Passage, Grenzen, Hörziel, Stopbedingung und
/// Rückweg." Die Liste steht hier als **Prüfbarkeit**, nicht als Kommentar:
/// `gate_felder_vollstaendig` geht sie einzeln durch.
pub const GATE_FELDER: [&str; 6] = [
    "target",
    "passage_id",
    "allowed_bounds",
    "listen_for",
    "stop_if",
    "revert",
];

impl Proposal {
    /// M-43: sind alle sechs Gate-Felder belegt?
    ///
    /// `passage_id` ist seit NR-07 **Pflichtfeld des Vertrags**
    /// (`$defs/proposal.required`) und seit WN-04 zusaetzlich Bedingung des
    /// Erzeugers: ohne benannte Passage entsteht gar kein Objekt. Ein
    /// Vorschlag ohne Passage benennt nicht, wo er gilt, und genau das
    /// verlangt das Exit-Gate.
    ///
    /// 🔑 **WN-04 (Nacharbeit 2, 07.09.2026):** hier stand bis zu dieser
    /// Runde „im Vertrag optional“. Das war seit NR-07 falsch, und die
    /// falsche Zusage deckte den Erzeuger, der weiterhin unvollstaendige
    /// Proposals baute und persistierte (WP1-4).
    pub fn gate_felder_vollstaendig(&self) -> [(&'static str, bool); 6] {
        [
            ("target", !self.target.is_empty()),
            ("passage_id", self.passage_id.is_some()),
            (
                "allowed_bounds",
                // Ein Eingriff braucht Grenzen; `no_change` und `more_data`
                // greifen nicht ein und tragen deshalb keine.
                !self.action.ist_eingriff() || self.allowed_bounds != Grenzen::default(),
            ),
            ("listen_for", !self.listen_for.is_empty()),
            ("stop_if", !self.stop_if.is_empty()),
            ("revert", true),
        ]
    }

    /// M-53: hält dieses Proposal alle Sicherheitsbudgets?
    ///
    /// ⚠️ Die Prüfung liest die **Grenzen**, nicht die Parameter: ein
    /// Vorschlag, dessen Parameter im Budget liegt, dessen `allowed_bounds`
    /// aber darüber hinausreichen, erlaubt dem Consumer den Ausbruch. Genau
    /// das ist der Fehler, den ein Budget verhindern soll.
    pub fn budgets_gehalten(&self, usergrenze_db: Option<f64>) -> bool {
        let deckel = match self.action {
            Aktion::StaticEqCut => BUDGET_STATISCH_DB,
            Aktion::DynamicEqCut => BUDGET_DYNAMISCH_HARD_CAP_DB,
            Aktion::Trim => BUDGET_STATISCH_DB,
            _ => return self.allowed_bounds == Grenzen::default(),
        };
        // Eine ENGERE Usergrenze gewinnt immer — nie umgekehrt.
        let wirksam = match usergrenze_db {
            Some(u) if u.is_finite() && u >= 0.0 => deckel.min(u),
            _ => deckel,
        };
        let gain_ok = match self.allowed_bounds.gain_db {
            Some(b) => b.von >= -wirksam - 1e-9 && b.bis <= wirksam + 1e-9 && b.von <= b.bis,
            None => false,
        };
        let param_ok = self
            .parameters
            .max_gain_db
            .is_some_and(|g| g.abs() <= wirksam + 1e-9);
        gain_ok && param_ok && self.baender() <= BUDGET_EQ_BAENDER_MAX
    }

    /// Wie viele Bänder dieser Vorschlag adressiert.
    ///
    /// Aus der Frequenzgrenze abgeleitet und nicht als eigenes Feld geführt:
    /// ein zweites Feld könnte von den Grenzen abweichen.
    pub fn baender(&self) -> u32 {
        let Some(b) = self.allowed_bounds.frequency_hz else {
            return 0;
        };
        if !(b.von.is_finite() && b.bis.is_finite()) || b.von <= 0.0 || b.bis < b.von {
            return 0;
        }
        // Ein Band ist 1/24 Oktave breit; die Zahl der Baender zwischen zwei
        // Frequenzen ist der Logarithmus ihres Verhaeltnisses in dieser
        // Einheit.
        let baender = (b.bis / b.von).log2() * GITTER_BANDBREITENKENNWERT;
        if !baender.is_finite() || baender < 0.0 {
            return 0;
        }
        baender.round().max(1.0) as u32
    }
}

// ═════════════════════════════════════════════════════════════════════════
// Die Lage, aus der ein Vorschlag entsteht
// ═════════════════════════════════════════════════════════════════════════

/// Alles, was der Erzeuger außer dem Befund braucht.
#[derive(Debug, Clone, Default)]
pub struct Proposallage {
    /// Die Instanz, an die der Vorschlag ginge.
    pub ziel_instanz: String,
    /// Die Revision, gegen die der Solver läuft.
    pub base_revision: u64,
    /// Der gespiegelte Intent — Quelle der Schutzangaben **und** der
    /// `intent_revision` (M-54).
    pub intent: Option<IntentBestand>,
    /// Ob die Zielinstanz überhaupt fernsteuerbar wäre.
    ///
    /// In P5 ist sie es nie — der aktive Kern entsteht in P6. Das Feld steht
    /// trotzdem hier, weil M-52 „Capability fehlt" als eigenen Grund führt.
    pub capability_vorhanden: bool,
    /// Welche Guardrails der Broker heute **messen** kann (M-50).
    ///
    /// Ein Guardrail ohne Vertragsteil ist nicht messbar und wird als „nicht
    /// gemessen" ausgewiesen — nie als „unverändert".
    pub messbare_guardrails: Vec<Stopbedingung>,
    /// Eine engere Grenze des Users, falls gesetzt. Sie gewinnt immer.
    pub usergrenze_db: Option<f64>,
    pub session_epoch: String,
    pub metrics_version: u32,
}

// ═════════════════════════════════════════════════════════════════════════
// Schritt 1 bis 5 (§42.2)
// ═════════════════════════════════════════════════════════════════════════

/// Schritt 1: der **Befundtyp** wählt die geprüften Aktionstemplates.
///
/// Rückgabe ist eine geordnete Liste — der kleinste Kandidat steht **vorn**.
/// Die Ordnung ist Teil der Zusage aus §42.2 Punkt 4 („der kleinste sichere
/// Kandidat gewinnt") und kein Zufall der Schreibreihenfolge.
pub fn aktionstemplates(befund: &CauseHypothesis) -> Vec<Aktion> {
    match (befund.ursachenklasse, befund.target_metric) {
        // Die Datenlage trägt gar keine Aussage: es gibt nichts zu wählen.
        (Ursachenklasse::DatenReichenNicht, _) => vec![Aktion::MoreData],
        // Eine Stereoaussage adressiert kein Band — dafür gibt es in P5 keine
        // eigene DSP-Aktion, also bleibt die manuelle Anleitung.
        (_, Zielmetrik::StereoSeitenanteilDb) => vec![Aktion::ManualGuidance],
        // Ein Peakproblem aus einem Transienten: der kleinste Eingriff ist ein
        // dynamischer Schnitt, der nur greift, wenn der Transient kommt.
        (Ursachenklasse::PeakAusTransient, _) => {
            vec![Aktion::DynamicEqCut, Aktion::StaticEqCut, Aktion::ManualGuidance]
        }
        // Zwei Quellen im selben Bereich, eine Resonanz, eine Summe auf dem
        // Master: ein schmaler statischer Schnitt ist der kleinste Eingriff,
        // der die Bandenergie senkt.
        (
            Ursachenklasse::ZweiQuellenKonkurrenz
            | Ursachenklasse::QuelleResonanz
            | Ursachenklasse::SummeAufMaster
            | Ursachenklasse::EffektkettePrePost
            | Ursachenklasse::StereoAusQuelleOderKette,
            _,
        ) => vec![
            Aktion::StaticEqCut,
            Aktion::DynamicEqCut,
            Aktion::Trim,
            Aktion::ManualGuidance,
        ],
    }
}

/// Warum eine Aktion ausscheidet. Geschlossen, damit ein Ausschluss nie
/// kommentarlos passiert — dieselbe Regel wie bei den Kandidaten (M-87).
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Solvergrund {
    IntentSchutz,
    CapabilityFehlt,
    MessqualitaetZuSchwach,
    GuardrailNichtMessbar,
    BefundNichtHandelbar,
}

impl Solvergrund {
    pub fn wort(self) -> &'static str {
        match self {
            Self::IntentSchutz => "intent_schutz",
            Self::CapabilityFehlt => "capability_fehlt",
            Self::MessqualitaetZuSchwach => "messqualitaet_zu_schwach",
            Self::GuardrailNichtMessbar => "guardrail_nicht_messbar",
            Self::BefundNichtHandelbar => "befund_nicht_handelbar",
        }
    }
}

/// Schritt 2: der **Constraint-Solver**.
///
/// Er entfernt Aktionen, die Intent, Capability, Headroom, Messqualität oder
/// Schutzbereiche verletzen — **vor** der Zielfunktion. Rückgabe: die
/// überlebenden Aktionen und die Gründe der Ausgeschiedenen.
pub fn constraint_solver(
    befund: &CauseHypothesis,
    lage: &Proposallage,
    kandidaten: &[Aktion],
) -> (Vec<Aktion>, Vec<(Aktion, Solvergrund)>) {
    let mut bleiben = Vec::new();
    let mut fallen = Vec::new();
    for aktion in kandidaten {
        if let Some(grund) = solververstoss(*aktion, befund, lage) {
            fallen.push((*aktion, grund));
        } else {
            bleiben.push(*aktion);
        }
    }
    (bleiben, fallen)
}

fn solververstoss(
    aktion: Aktion,
    befund: &CauseHypothesis,
    lage: &Proposallage,
) -> Option<Solvergrund> {
    if !aktion.ist_eingriff() {
        // `no_change`, `more_data` und `manual_guidance` greifen nicht ein und
        // koennen deshalb keine Grenze verletzen.
        return None;
    }
    // Der Befund muss handelbar sein (M-52, M-30). Ein `stale` oder
    // `more_data`-Befund erzeugt keinen Eingriff.
    if befund.zustand != Befundzustand::ReadyToSend {
        return Some(Solvergrund::BefundNichtHandelbar);
    }
    // Der Wille des Users (M-49): ein Schutz im Zielband ist eine HARTE
    // Constraint, kein Gewicht.
    if let Some(intent) = lage.intent.as_ref() {
        if intent.schutz_verletzt(&befund.candidate_source, befund.band_hz.von, befund.band_hz.bis)
        {
            return Some(Solvergrund::IntentSchutz);
        }
    }
    // Messqualitaet: eine unsichere Aussage traegt keinen Eingriff.
    if befund.confidence.klasse < Sicherheitsklasse::Hoch {
        return Some(Solvergrund::MessqualitaetZuSchwach);
    }
    // Capability: ohne Fernsteuerbarkeit gibt es keinen ausfuehrbaren
    // Eingriff. In P5 ist das der Normalfall — und deshalb ist
    // `manual_guidance` die Aktion, die uebrig bleibt.
    if !lage.capability_vorhanden {
        return Some(Solvergrund::CapabilityFehlt);
    }
    None
}

/// Schritt 3 und 4: die **regularisierte Zielfunktion** und der kleinste
/// sichere Kandidat.
///
/// Die Kosten eines Kandidaten sind `Bänder + |Gain| + Nebenwirkung`, jede
/// Größe auf ihr eigenes Budget normiert. Ein Kandidat, der dasselbe mit
/// weniger Bändern und weniger Gain erreicht, gewinnt — und bei gleichen
/// Kosten gewinnt der, der in der Templateliste **vorn** steht.
pub fn kosten(aktion: Aktion, parameter: &Parameter, grenzen: &Grenzen) -> f64 {
    let baender = Proposal {
        allowed_bounds: *grenzen,
        ..leerer_rahmen(aktion)
    }
    .baender() as f64;
    let gain = parameter.max_gain_db.unwrap_or(0.0).abs();
    // Die Nebenwirkung eines dynamischen Schnitts ist kleiner als die eines
    // statischen (er greift nur, wenn das Problem da ist), die eines Trims
    // groesser (er bewegt das ganze Band).
    let nebenwirkung = match aktion {
        Aktion::DynamicEqCut => 0.25,
        Aktion::StaticEqCut => 0.5,
        Aktion::Trim => 1.0,
        _ => 0.0,
    };
    quantisiert(
        baender / BUDGET_EQ_BAENDER_MAX as f64 / 3.0
            + gain / BUDGET_STATISCH_DB / 3.0
            + nebenwirkung / 3.0,
    )
}

fn leerer_rahmen(aktion: Aktion) -> Proposal {
    Proposal {
        proposal_id: String::new(),
        proposal_schema: 1,
        target: String::new(),
        base_revision: 0,
        passage_id: None,
        finding_id: None,
        action: aktion,
        parameters: Parameter::default(),
        allowed_bounds: Grenzen::default(),
        evidence_ids: Vec::new(),
        expected_effect: Wirkung::None,
        protected_traits: Vec::new(),
        listen_for: String::new(),
        stop_if: Vec::new(),
        execution: Ausfuehrung::Manual,
        confidence: Befundkonfidenz {
            klasse: Sicherheitsklasse::Unklar,
            score: 0.0,
        },
        revert: Rueckweg::NoneNeeded,
        intent_revision: 0,
        generatorversion: GENERATORVERSION,
    }
}

/// Der vollständige Weg: fünf Schritte, höchstens ein Objekt.
///
/// ⚠️ `no_change` und `more_data` sind **vollständige** Vorschläge, kein
/// leerer Rückgabewert (M-46) — sie tragen Ziel, Passage, Hörziel,
/// Stopbedingung und Rückweg wie jeder andere.
///
/// 🔑 **WN-04 (Nacharbeit 2, 07.09.2026): OHNE benannte Passage entsteht
/// KEIN Objekt.** Bis zu dieser Runde gab der Erzeuger auch dann eines
/// zurück, wenn der Befund keine Passage trug; es landete in
/// `stand.vorschlaege` und über `vorschlag_persistieren` als
/// `event_type = "proposal"` im Store — ein Objekt, dem seit NR-07 ein
/// Pflichtfeld des Vertrags fehlt. Der ausgeführte Test schrieb die
/// Abweichung sogar fest (WP1-4).
///
/// Das `None` ist deshalb keine Ausnahme von M-46, sondern seine Bedingung:
/// entweder ein VOLLSTÄNDIGES Objekt — auch für `no_change` und
/// `more_data` — oder gar keines. Ein Vorschlag, der nicht sagen kann, WO
/// er gilt, ist keiner.
pub fn proposal(befund: &CauseHypothesis, lage: &Proposallage) -> Option<Proposal> {
    // WN-04: die Bedingung steht VOR jeder Rechnung. Was hier fällt, wird
    // nicht gebildet und dann verworfen — es entsteht nicht.
    befund.passage_id.as_ref()?;
    // Schritt 1
    let templates = aktionstemplates(befund);
    // Schritt 2
    let (mut ueberlebende, _gefallen) = constraint_solver(befund, lage, &templates);
    // M-50: eine Stopbedingung auf einem nicht messbaren Guardrail macht den
    // Vorschlag NICHT handelbar. Der Guardrail bleibt trotzdem in `stop_if` —
    // er ist die Bedingung, unter der abgebrochen wird, und ihn wegzulassen
    // hiesse, den Abbruch stillschweigend zu streichen.
    let bedingungen = stopbedingungen(befund);
    let alle_messbar = bedingungen
        .iter()
        .all(|s| lage.messbare_guardrails.contains(s));
    if !alle_messbar {
        // M-50 woertlich: „Ein Proposal, dessen `stop_if` auf einen nicht
        // messbaren Guardrail zeigt, ist NICHT HANDELBAR und meldet
        // `MORE DATA`." Nicht „manuelle Anleitung", nicht „kleinerer
        // Eingriff" — `more_data`. Wer hier eine Ersatzaktion anboete,
        // behauptete, der Abbruch sei ueberwacht.
        ueberlebende.clear();
    }
    // Schritt 3 und 4
    // ⚠️ EIN EINGRIFF SCHLAEGT EINE ANLEITUNG — immer.
    //
    // „Der kleinste sichere Kandidat gewinnt" (§42.2 Punkt 4) meint den
    // kleinsten WIRKSAMEN: `manual_guidance` kostet nichts, weil es nichts
    // tut, und wuerde eine reine Kostenordnung jedes Mal gewinnen. Der
    // Vorschlag waere dann immer „mach es selbst" — ein Advisor, der nie
    // etwas vorschlaegt. Die Anleitung ist der RUECKFALL, wenn kein Eingriff
    // sicher ist, und kein Mitbewerber um den kleinsten Rang.
    let aktion = ueberlebende
        .iter()
        .copied()
        .min_by(|a, b| {
            let (pa, ga) = eingriff(*a, befund, lage);
            let (pb, gb) = eingriff(*b, befund, lage);
            b.ist_eingriff()
                .cmp(&a.ist_eingriff())
                .then_with(|| {
                    kosten(*a, &pa, &ga)
                        .partial_cmp(&kosten(*b, &pb, &gb))
                        .unwrap_or(std::cmp::Ordering::Equal)
                })
                // Bei gleichen Kosten gewinnt der, der in der Templateliste
                // vorn steht — eine stabile Wahl, keine zufaellige (M-44).
                .then_with(|| {
                    templates
                        .iter()
                        .position(|x| x == a)
                        .cmp(&templates.iter().position(|x| x == b))
                })
        })
        .unwrap_or(
            if befund.ursachenklasse == Ursachenklasse::DatenReichenNicht || !alle_messbar {
                Aktion::MoreData
            } else {
                Aktion::NoChange
            },
        );
    let (parameters, allowed_bounds) = eingriff(aktion, befund, lage);
    Some(Proposal {
        proposal_id: proposal_id(befund, lage, aktion),
        proposal_schema: 1,
        target: lage.ziel_instanz.clone(),
        base_revision: lage.base_revision,
        passage_id: befund.passage_id.clone(),
        finding_id: Some(befund.finding_id.clone()),
        action: aktion,
        parameters,
        allowed_bounds,
        // M-54: ohne Evidenz-IDs entsteht kein Objekt — sie kommen aus dem
        // Befund und werden nicht neu gesammelt.
        evidence_ids: befund.evidence_ids.clone(),
        expected_effect: wirkung(aktion, befund),
        protected_traits: schutzconstraints(lage),
        listen_for: befund.listen_for.clone(),
        stop_if: bedingungen,
        // Schritt 5: in P5 ist JEDE Aktion `manual`.
        execution: Ausfuehrung::Manual,
        confidence: befund.confidence,
        revert: rueckweg(aktion),
        // §37.3: die Revision, gegen die der Solver gelaufen ist — nicht die
        // zum Anzeigezeitpunkt.
        intent_revision: lage.intent.as_ref().map_or(0, |i| i.revision),
        generatorversion: GENERATORVERSION,
    })
}

/// Die Parameter und Grenzen eines Eingriffs (M-47).
///
/// ⚠️ Die Frequenz kommt aus dem **Band des Befunds**, nicht aus dem größten
/// Betrag der Banddeltas. Das ist NAK-168 wörtlich, und es ist der Grund,
/// warum `band_hz` überhaupt im Befund steht.
fn eingriff(aktion: Aktion, befund: &CauseHypothesis, lage: &Proposallage) -> (Parameter, Grenzen) {
    if !aktion.ist_eingriff() {
        // `no_change`, `more_data` und `manual_guidance` tragen ein LEERES
        // Objekt — nicht Parameter mit 0.
        return (Parameter::default(), Grenzen::default());
    }
    let band = befund.band_hz;
    let mitte = bandmitte_hz(bandmitte_index(band));
    let von = bandmitte_hz(band.von);
    let bis = bandmitte_hz(band.bis.saturating_sub(1));
    let deckel = match aktion {
        Aktion::DynamicEqCut => BUDGET_DYNAMISCH_STANDARD_DB,
        _ => BUDGET_STATISCH_DB,
    };
    // Eine ENGERE Usergrenze gewinnt immer.
    let wirksam = match lage.usergrenze_db {
        Some(u) if u.is_finite() && u >= 0.0 => deckel.min(u),
        _ => deckel,
    };
    // Der Gain ist eine REDUKTION: negativ, und nie mehr als das Budget.
    let gain = -wirksam;
    // Die Guete folgt der Bandbreite: je schmaler der Bereich, desto hoeher.
    let breite_baender = (band.bis - band.von).max(1) as f64;
    let q = (GITTER_BANDBREITENKENNWERT / breite_baender).clamp(0.1, 20.0);
    (
        Parameter {
            frequency_hz: Some(runde(mitte.clamp(1.0, 24000.0))),
            q: Some(runde(q)),
            max_gain_db: Some(runde(gain)),
        },
        Grenzen {
            frequency_hz: Some(Wertebereich {
                von: runde(von.clamp(1.0, 24000.0)),
                bis: runde(bis.clamp(1.0, 24000.0)),
            }),
            q: Some(Wertebereich {
                von: runde((q / 2.0).clamp(0.1, 20.0)),
                bis: runde((q * 2.0).clamp(0.1, 20.0)),
            }),
            gain_db: Some(Wertebereich {
                von: runde(-wirksam),
                bis: runde(wirksam),
            }),
        },
    )
}

/// Der Bandindex, dessen Mitte der Vorschlag adressiert.
///
/// Das arithmetische Mittel der Bandgrenzen — auf dem logarithmischen Gitter
/// ist das die **geometrische** Mitte der Frequenzen, also genau der Punkt,
/// den ein Musiker als „Mitte des Bereichs" hört.
fn bandmitte_index(band: Bandintervall) -> u32 {
    band.von + (band.bis.saturating_sub(band.von)) / 2
}

/// Rundet auf vier Nachkommastellen.
///
/// Ohne diese Rundung entschiede die letzte Bitstelle einer
/// Gleitkommapotenz über die Gleichheit zweier Läufe — dieselbe Begründung
/// wie `RANG_QUANTUM` in `hypothese.rs` (M-44: hundert Läufe, ein Ergebnis).
fn runde(wert: f64) -> f64 {
    if !wert.is_finite() {
        return 0.0;
    }
    (wert * 10_000.0).round() / 10_000.0
}

/// Die erwartete Wirkung als geschlossene Menge statt als Text.
fn wirkung(aktion: Aktion, befund: &CauseHypothesis) -> Wirkung {
    if !aktion.ist_eingriff() && aktion != Aktion::ManualGuidance {
        return Wirkung::None;
    }
    match befund.target_metric {
        Zielmetrik::StereoSeitenanteilDb => Wirkung::RestoreStereoBalance,
        Zielmetrik::PeakEreignisrate => Wirkung::ReducePeakPressure,
        _ => match befund.ursachenklasse {
            Ursachenklasse::PeakAusTransient => Wirkung::ReducePeakPressure,
            _ => Wirkung::ReduceMaskingPressure,
        },
    }
}

/// Der Rückweg je Vorschlagstyp (E-06, M-43).
///
/// ⚠️ `dsp_revert` gehört P7 und kommt hier nicht vor — nicht „noch nicht",
/// sondern gar nicht: die Menge hat drei Werte.
fn rueckweg(aktion: Aktion) -> Rueckweg {
    match aktion {
        // Ein Eingriff, den der User in einem Versuch fährt, wird über den
        // Abbruch des Versuchs zurückgenommen.
        Aktion::StaticEqCut | Aktion::DynamicEqCut | Aktion::Trim => Rueckweg::ExperimentAbort,
        // Eine Anleitung führt der User selbst aus — und nimmt sie selbst
        // zurück.
        Aktion::ManualGuidance => Rueckweg::ManualOnly,
        // Was nichts ändert, braucht keinen Rückweg.
        Aktion::NoChange | Aktion::MoreData => Rueckweg::NoneNeeded,
    }
}

/// Die Stopbedingungen eines Vorschlags.
///
/// Mindestens eine ist Pflicht (`minItems: 1`). Welche, folgt aus der
/// Zielmetrik: was der Eingriff bewegt, ist auch das, was er kaputtmachen
/// kann.
fn stopbedingungen(befund: &CauseHypothesis) -> Vec<Stopbedingung> {
    let mut aus = vec![Stopbedingung::GuardrailAbdeckung];
    match befund.target_metric {
        Zielmetrik::StereoSeitenanteilDb => aus.push(Stopbedingung::GuardrailBreite),
        Zielmetrik::PeakEreignisrate => aus.push(Stopbedingung::GuardrailPeak),
        _ => aus.push(Stopbedingung::GuardrailLoudness),
    }
    if !befund.ausschluesse.is_empty() {
        // Ein Befund mit Ausschlüssen hatte Konkurrenz; dass die Änderung am
        // Master wiederholbar ist, ist dann keine Selbstverständlichkeit.
        aus.push(Stopbedingung::KeineWiederholbareMasteraenderung);
    }
    aus.sort();
    aus.dedup();
    aus
}

/// Die geschützten Eigenschaften als harte Constraints (M-49).
fn schutzconstraints(lage: &Proposallage) -> Vec<Schutzconstraint> {
    let Some(intent) = lage.intent.as_ref() else {
        return Vec::new();
    };
    let mut aus: Vec<Schutzconstraint> = intent
        .schutzangaben
        .iter()
        .map(|s| Schutzconstraint {
            quelle_id: s.quelle_id.clone(),
            eigenschaft: s.eigenschaft.clone(),
            band: s.band,
        })
        .collect();
    aus.sort();
    aus.truncate(32);
    aus
}

/// Die deterministische `proposal_id` (M-44).
///
/// Gleicher Eingang, gleiche ID — aus Sitzung, Befund, Aktion und
/// Generatorversion, nicht aus einem Zähler und nicht aus der Zeit.
pub fn proposal_id(befund: &CauseHypothesis, lage: &Proposallage, aktion: Aktion) -> String {
    use sha2::{Digest, Sha256};
    let mut hasher = Sha256::new();
    hasher.update(b"nakama.sonde014.proposal.v1\0");
    hasher.update(lage.session_epoch.as_bytes());
    hasher.update(b"\0");
    hasher.update(befund.finding_id.as_bytes());
    hasher.update(b"\0");
    hasher.update(aktion.wire().as_bytes());
    hasher.update(b"\0");
    hasher.update(GENERATORVERSION.to_be_bytes());
    hasher.update(lage.metrics_version.to_be_bytes());
    let digest = hasher.finalize();
    digest[..16].iter().map(|b| format!("{b:02x}")).collect()
}

/// Ob aus diesem Vorschlag ein `draft_offer` werden darf (M-52, M-30).
///
/// ⚠️ Die Sperre liegt **hier**, im Datenweg: ein veralteter, ungültiger oder
/// capability-loser Vorschlag wird gar nicht erst als Angebot gebildet. Er
/// wird nicht gebildet und dann verworfen — der Unterschied ist, dass ein
/// gebildetes Angebot einen Weg nach draußen hätte.
pub fn darf_draft_offer(befund: &CauseHypothesis, vorschlag: &Proposal, lage: &Proposallage) -> bool {
    befund.zustand.erlaubt_draft()
        && vorschlag.action.ist_eingriff()
        // M-43: das Exit-Gate verlangt die PASSAGE woertlich. Ein Vorschlag,
        // der nicht sagen kann, WO er gilt, wird nicht angeboten — auch wenn
        // alles andere stimmt.
        //
        // 🔑 WN-04: seit dieser Runde entsteht ein solcher Vorschlag gar
        // nicht mehr. Die Frage bleibt trotzdem stehen: ein Riegel, dessen
        // Vorbedingung anderswo faellt, ist einer weniger, und dieser hier
        // kostet nichts.
        && vorschlag.passage_id.is_some()
        && lage.capability_vorhanden
        && vorschlag.base_revision == lage.base_revision
        && vorschlag
            .stop_if
            .iter()
            .all(|s| lage.messbare_guardrails.contains(s))
}
