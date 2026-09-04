//! Rücknahme eingegangener Evidenz (SONDE-013 M-52 bis M-57, M-59, M-62;
//! §34.2).
//!
//! ## Der Grund auf dem Draht ist je Auslöser eindeutig
//!
//! M-53 legt ihn fest und lässt ihn ausdrücklich nicht der Implementierung:
//! Seek, Loop-Wrap **innerhalb** der Passage und jeder andere
//! Kontinuitätsbruch tragen `epochwechsel`, ein Drop trägt `sequenzluecke`.
//! Das folgt der Zeitvertragsregel aus SONDE-009 — „Drop zählt als Segment,
//! alles andere als Epoche".
//!
//! Ein vertauschter Grund wäre kein Schönheitsfehler. Der Empfänger
//! entscheidet an ihm, ob er eine Zeitreihe fortsetzen darf, und ein Drop,
//! der als Epochwechsel ankommt, kostet ihn eine ganze Epoche Evidenz;
//! umgekehrt setzte er über eine echte Epochengrenze hinweg fort.
//!
//! ## Und ein Loop-Wrap AN der Passagengrenze ist gar kein Bruch
//!
//! Deckt der Loop die Passage vollständig ab, beendet der Wrap einen
//! Durchlauf und beginnt den nächsten. Das ist weder Invalidierung noch
//! Herabstufung (M-53, Abgrenzung zu M-23). Genau diese Unterscheidung ist
//! der Grund, warum `Bruchart` zwischen `LoopWrapInnerhalb` und
//! `LoopWrapAnGrenze` trennt statt einen Wahrheitswert zu tragen.

use std::collections::BTreeSet;

use crate::telemetrie::{fingerprint_aehnlichkeit, Fingerprintwerte};

/// Die geschlossene Gründemenge aus `evidence_invalidate.grund`.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Grund {
    Intervention,
    RoutingUnbekannt,
    Sequenzluecke,
    Epochwechsel,
    StateRevision,
    MaterialWechsel,
    MesspunktWechsel,
}

impl Grund {
    /// Das Wireworte, eins zu eins aus dem Schema.
    pub fn wort(self) -> &'static str {
        match self {
            Grund::Intervention => "intervention",
            Grund::RoutingUnbekannt => "routing_unbekannt",
            Grund::Sequenzluecke => "sequenzluecke",
            Grund::Epochwechsel => "epochwechsel",
            Grund::StateRevision => "state_revision",
            Grund::MaterialWechsel => "material_wechsel",
            Grund::MesspunktWechsel => "messpunkt_wechsel",
        }
    }
}

/// Der über `art` diskriminierte Umfang (M-57).
///
/// Als Enum und nicht als Struct mit `art`-Feld plus optionalen Werten: mit
/// einem gemeinsamen Objekt wäre `art=sample_range` OHNE Bereich gültig
/// gewesen und `art=ganze_sitzung` MIT Bereich ebenfalls — ein Etikett ohne
/// Wirkung. Jeder Zweig trägt genau die Felder, die seine Art braucht, und
/// keine anderen; das Schema sagt es genauso.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Umfang {
    /// Genau diese Evidenz-IDs.
    Ids(BTreeSet<String>),
    /// Ein Samplebereich, halboffen `[von, bis)`.
    Bereich { von: i64, bis: i64 },
    /// Die ganze Sitzung.
    GanzeSitzung,
}

impl Umfang {
    pub fn art(&self) -> &'static str {
        match self {
            Umfang::Ids(_) => "evidence_ids",
            Umfang::Bereich { .. } => "sample_range",
            Umfang::GanzeSitzung => "ganze_sitzung",
        }
    }

    /// Ob dieser Umfang überhaupt etwas erfasst.
    ///
    /// Eine leere ID-Menge und ein leerer Bereich sind KEINE Invalidierung —
    /// sie zu senden hiesse, dem Empfänger eine Rücknahme zu melden, die
    /// nichts zurücknimmt, und ihn danach glauben zu lassen, es sei aufgeräumt.
    pub fn gueltig(&self) -> bool {
        match self {
            Umfang::Ids(ids) => !ids.is_empty(),
            Umfang::Bereich { von, bis } => bis > von,
            Umfang::GanzeSitzung => true,
        }
    }

    /// Ob eine Evidenz in diesen Umfang fällt.
    pub fn erfasst(&self, id: &str, von: i64, bis: i64) -> bool {
        match self {
            Umfang::Ids(ids) => ids.contains(id),
            Umfang::Bereich { von: a, bis: b } => {
                // Halboffene Fenster überlappen, wenn beide Ränder sich
                // kreuzen. Sättigend, damit zwei gültige Fenster an
                // entgegengesetzten `i64`-Rändern nicht durch einen Überlauf
                // als überlappend gelten.
                von.max(*a) < bis.min(*b)
            }
            Umfang::GanzeSitzung => true,
        }
    }
}

/// Eine gesendete Invalidierung.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Invalidierung {
    pub grund: Grund,
    pub umfang: Umfang,
}

impl Invalidierung {
    pub fn gueltig(&self) -> bool {
        self.umfang.gueltig()
    }
}

// ── M-53: der Grund je Auslöser ─────────────────────────────────────────

/// Was die Zeitreihe unterbrochen hat.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Bruchart {
    Seek,
    /// Der Loop ist KÜRZER als die Passage: der Wrap fällt mitten hinein.
    LoopWrapInnerhalb,
    /// Der Loop deckt die Passage vollständig ab: der Wrap beendet einen
    /// Durchlauf und beginnt den nächsten.
    LoopWrapAnGrenze,
    Drop,
    /// Transportkante, Sampleratewechsel, Neuanlauf, Beweislagewechsel.
    AndererKontinuitaetsbruch,
}

/// Der Grund für einen Bruch — oder `None`, wenn er keiner ist (M-53).
///
/// `LoopWrapAnGrenze` liefert bewusst `None`: er ist weder Invalidierung noch
/// Herabstufung. Ein `Some(Epochwechsel)` an dieser Stelle nähme dem User bei
/// jedem Loopdurchlauf seine Evidenz — und zwar genau bei der Arbeitsweise,
/// für die Passagen gedacht sind.
pub fn grund_fuer_bruch(art: Bruchart) -> Option<Grund> {
    match art {
        // „Drop zählt als Segment, alles andere als Epoche" (SONDE-009).
        Bruchart::Drop => Some(Grund::Sequenzluecke),
        Bruchart::Seek | Bruchart::LoopWrapInnerhalb | Bruchart::AndererKontinuitaetsbruch => {
            Some(Grund::Epochwechsel)
        }
        Bruchart::LoopWrapAnGrenze => None,
    }
}

// ── M-52: Quarantäne nach einer Intervention ────────────────────────────

/// Der Umfang einer Interventionsquarantäne (M-52).
///
/// Bei unbekanntem Routing ist es die GANZE Sitzung — und der Grund heisst
/// dann auch so. Ein `intervention` mit halbem Bereich wäre die gefährlichere
/// Meldung: der Empfänger nähme an, der Rest sei sauber.
pub fn quarantaene_fuer_intervention(
    routing_bekannt: bool,
    bereich: Option<(i64, i64)>,
) -> Invalidierung {
    match (routing_bekannt, bereich) {
        (true, Some((von, bis))) if bis > von => Invalidierung {
            grund: Grund::Intervention,
            umfang: Umfang::Bereich { von, bis },
        },
        // Kein Bereich trotz bekanntem Routing heisst: wir wissen, WO die
        // Kette läuft, aber nicht WANN. Auch das ist die ganze Sitzung.
        _ => Invalidierung {
            grund: if routing_bekannt {
                Grund::Intervention
            } else {
                Grund::RoutingUnbekannt
            },
            umfang: Umfang::GanzeSitzung,
        },
    }
}

// ── M-54: Materialwechsel über den Fingerprint ──────────────────────────

/// Ab welcher Fingerprintähnlichkeit dasselbe Material gilt.
///
/// Derselbe Startwert wie das Materialgate der Vergleichbarkeit (§43.2:
/// 0,95). Er steht im Register `metriken-v1.json`.
pub const GATE_MATERIAL_GLEICH: f64 = 0.95;

/// Erkennt einen Materialwechsel über den Fingerprintvergleich (M-54).
///
/// ⚠️ Ausdrücklich NICHT über eine Zeitheuristik. „Seit zehn Sekunden anderes
/// Material" ist keine Messung, sondern eine Vermutung über den User; der
/// Fingerprint ist eine über dasselbe Fenster gerechnete Größe (M-31).
///
/// Ein fehlender Fingerprint auf einer Seite ist KEIN „gleiches Material":
/// ohne Beleg gibt es keine Aussage, und fail-closed heisst hier
/// invalidieren.
pub fn material_wechsel(
    vorher: Option<&Fingerprintwerte>,
    jetzt: Option<&Fingerprintwerte>,
    umfang: Umfang,
) -> Option<Invalidierung> {
    let gleich = match (vorher, jetzt) {
        (Some(a), Some(b)) => fingerprint_aehnlichkeit(a, b) >= GATE_MATERIAL_GLEICH,
        _ => false,
    };
    if gleich {
        return None;
    }
    Some(Invalidierung {
        grund: Grund::MaterialWechsel,
        umfang,
    })
}

// ── M-55: Messpunktwechsel ──────────────────────────────────────────────

/// Erkennt einen Messpunkt- oder Klassenwechsel (M-55).
///
/// Das ist die Kante zu Gate 7 (§49.2): eine Standard-Insertprobe wird nie
/// als exakter Mastersummenbeitrag bezeichnet — folglich darf Evidenz von
/// `insert` nach einem Wechsel auf `post_fader_contribution` nicht
/// mitwandern.
pub fn messpunkt_wechsel(
    alte_klasse: &str,
    neue_klasse: &str,
    umfang: Umfang,
) -> Option<Invalidierung> {
    if alte_klasse == neue_klasse {
        return None;
    }
    Some(Invalidierung {
        grund: Grund::MesspunktWechsel,
        umfang,
    })
}

// ── M-56: ein angewendeter Zustand segmentiert, statt zu tainten ────────

/// Was ein `apply_transaction` auslöst (M-56).
///
/// Ein **angewendeter** DSP-Zustand ist ausdrücklich KEIN dauerhafter Taint.
/// Ihn als solchen zu führen hiesse, dass jede vom User bestätigte Änderung
/// die Messung für immer entwertet — und der Advisor nie wieder eine starke
/// Aussage träfe, sobald der User ihm einmal gefolgt ist.
#[derive(Debug, Clone, PartialEq)]
pub struct Segmentierung {
    /// Die alte Baseline ist geschlossen — nicht gelöscht.
    pub alte_baseline_geschlossen: bool,
    /// Abhängige Findings sind stale, nicht falsch.
    pub findings_stale: bool,
    /// Nach dieser Zeit ist eine neue `post_committed`-Baseline dieser
    /// Revision gültig.
    pub warmup_samples: u64,
    pub neue_revision: u64,
    /// Der Grund, der auf dem Draht mitreist.
    pub grund: Grund,
}

/// Warm-up nach einem Apply, in Samples bei 48 kHz: 500 ms.
///
/// Lang genug, dass die längsten Zeitkonstanten der Kette (LUFS-Momentanwert
/// 400 ms) eingeschwungen sind, und kurz genug, dass der User die Wirkung
/// seiner Änderung noch mit ihr in Verbindung bringt.
pub const WARMUP_SAMPLES_48K: u64 = 24_000;

pub fn apply_transaction(neue_revision: u64, samplerate: f64) -> Segmentierung {
    let warmup = if samplerate > 0.0 {
        ((WARMUP_SAMPLES_48K as f64) * (samplerate / 48_000.0)).round() as u64
    } else {
        WARMUP_SAMPLES_48K
    };
    Segmentierung {
        alte_baseline_geschlossen: true,
        findings_stale: true,
        warmup_samples: warmup,
        neue_revision,
        grund: Grund::StateRevision,
    }
}
