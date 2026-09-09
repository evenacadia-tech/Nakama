//! Stufe B — die fokussierte Evidenz (M-19, M-20).
//!
//! Hier entsteht der ZUSAMMENHANG zwischen einem Kandidaten und dem Master:
//! die sechs Rangkomponenten aus §36.2 und der gerichtete Beleg, den die
//! Klassenwahl liest. Stufe A (`screening.rs`) rechnet auf den 64 Gruppen des
//! Livegitters, Stufe B auf dem 221er-Verlauf.
//!
//! ── DIE REDUKTIONSREIHENFOLGE IST TEIL DER ZUSAGE (M-25) ──────────────────
//!
//! `RANGKOMPONENTEN` (in `determinismus.rs`) ist Vertragsreihenfolge UND
//! Summenreihenfolge: `Rangkomponenten::rang` addiert in genau dieser Folge.
//! Eine andere Reihenfolge ergaebe eine andere letzte Bitstelle und damit —
//! ohne `RANG_QUANTUM` — eine andere Rangfolge.
//!
//! ── RANG UND KLASSE LESEN NICHT MEHR DISJUNKTE GROESSEN (NAK-212) ─────────
//!
//! Bis zum 07.09.2026 flossen die sechs Rangkomponenten ausschliesslich in
//! `confidence.score`, also nur in die REIHENFOLGE; die Klassenwahl las davon
//! keine einzige. Ein Kandidat ohne jeden Beleg fuer einen Zusammenhang
//! erreichte deshalb `hoch`/READY (G5-Befunde G-D1, E-L3). Seither traegt
//! `Zusammenhangsbeleg` die Groessen MIT VORZEICHEN und als `Option`, und
//! `klassenwahl.rs` liest sie ueber `positiv_belegt` und `gegenbeleg`.

use super::messung::{
    bandmittel, gemeinsame_reihen, korrelation_gerichtet, median, mittel, spannweite,
};
use super::screening::STUFE_B_AUFRUFE;
use super::{
    quantisiert, Aufnahme, Bandintervall, Quellprofil, BAENDER_FEIN, BOOTSTRAP_SAAT,
    PRIORITAET_OHNE_ROLLE, RANGKOMPONENTEN, RANG_QUANTUM, ROLLENPRIORITAET,
};

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
pub(super) fn bandpassung(kandidat: &Quellprofil, band: Bandintervall) -> f64 {
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
pub(super) fn koinzidenz_gerichtet(kandidat: &Quellprofil, master: &Quellprofil) -> Option<f64> {
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
pub(super) fn intent_relevanz(kandidat: &Quellprofil, aufnahme: &Aufnahme) -> f64 {
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
    let p = crate::coordinator::experiment::bootstrap_p_positiv(
        &reihe,
        crate::coordinator::experiment::BOOTSTRAP_BLOCK,
        crate::coordinator::experiment::BOOTSTRAP_ZIEHUNGEN,
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

#[cfg(test)]
mod tests {
    use super::super::testhilfe::*;
    use super::*;

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
}
