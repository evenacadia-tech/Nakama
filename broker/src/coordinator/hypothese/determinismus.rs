//! Determinismus (M-25): die Groessen, die zur `metrics_version` gehoeren.
//!
//! **Gleicher Eingang, gleiche Reihenfolge (M-25).** Bootstrap-Saat,
//! Reduktionsreihenfolge, Eingangsquantisierung und Tie-Break-Key stehen als
//! benannte Konstanten hier und gehoeren zur `metrics_version` (§36.4). Sie
//! sind ABSICHTLICH nicht im Register `metriken-v1.json`: Entscheid **E-12**
//! des Dirigenten (06.09.2026) erlaubt genau EINEN Versionsschritt, und der
//! liegt in Etappe H zusammen mit der Kalibrierung. Zwei Schritte fuer eine
//! Sache waeren zwei Kalibrierungen.
//!
//! Dazu stehen hier die beiden Gitterzahlen und der harte Deckel aus M-18 —
//! Groessen, die keine Rechnung sind, sondern die Form, in der gerechnet
//! wird. Die Rechnungen selbst liegen in `screening.rs` (Stufe A) und
//! `zusammenhang.rs` (Stufe B); `quantisiert` ist die einzige Funktion hier,
//! weil jede von ihnen sie ruft.

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

#[cfg(test)]
mod tests {
    use super::*;

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
}
