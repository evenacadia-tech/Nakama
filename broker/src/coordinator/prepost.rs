//! Der PRE/POST-Join zweier Sonden (SONDE-013 M-13 bis M-24; §38).
//!
//! ## Was hier NICHT entsteht: ein Differenzwert
//!
//! §38.1 zählt drei getrennte Ergebnisse auf, und M-14 verbietet, sie zu einem
//! zusammenzuziehen: die **rohe** Messdifferenz derselben Projektfenster, das
//! **ausgerichtete** pegelbezogene Delta nach sicherer Restlag- und
//! Gainschätzung, und die **interpretierte** Wirkung. Die Trennung ist keine
//! Darstellungsfrage. Ohne sie erschiene mehr Pegel als mehr Höhen und eine
//! Laufzeitänderung als EQ-Effekt — zwei Fehlaussagen, die genau dann
//! entstehen, wenn man die drei Stufen zu einer Zahl mittelt.
//!
//! ## Der verteilte Zweig ist in P4 der einzige
//!
//! §38.2 kennt zwei Qualitätsstufen: verteiltes Feature-Alignment über die
//! übertragenen Features, und lokales Audio-Alignment mit GCC-PHAT, das PRE
//! und POST als echte Audiopuffer in einer Instanz verlangt. In P4 gibt es
//! kein Compare-Routing (M-19, M-24), also ist `AudioAligned` eine Klasse, die
//! dieses Modul **nie vergibt**. Sie steht trotzdem im Typ: sie wegzulassen
//! hieße, den Vertrag der vier Klassen (M-16) zu brechen und später
//! nachrüsten zu müssen.
//!
//! Aus demselben Grund entsteht hier **keine** Transferfunktion. §38.3 erlaubt
//! `H₁(f) = Sᵧₓ/Sₓₓ` ausdrücklich nur im lokalen Compare-Routing; bei
//! verteilten Probes zeigt Nakama robuste P10/P50/P95-Differenzen statt eines
//! vermeintlichen Frequenzgangs.
//!
//! ## Ein unbekannter Zeitpfad erzeugt keine starke Aussage
//!
//! M-21 zitiert das Exit-Gate wörtlich. Solange FLs PDC-Abbildung nicht
//! bewiesen ist, trägt ein Frame den **rohen** Schlüssel und behauptet damit
//! keine Inhaltsgleichheit zwischen Instanzen. Erst eine validierte
//! Presentation-Abbildung *plus* Alignmentqualität erzeugt den ausgerichteten
//! Schlüssel. Beides sind hier verschiedene Typen und nicht zwei Zustände
//! desselben — ein `if` an dieser Stelle wäre genau die Verwechslung, die das
//! Gate verbietet.

use std::collections::BTreeSet;

// ── Die Klassen ──────────────────────────────────────────────────────────

/// Die vier Alignmentklassen aus §38.2 (M-16).
///
/// Die Reihenfolge ist die Rangfolge: ein Vergleich `<` sagt „schwächer als".
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Alignmentklasse {
    /// Kein belastbarer Zeitbezug. Harte Ausschlüsse landen hier, und ebenso
    /// Material, aus dem sich gar kein Lag benennen lässt (Rauschen).
    Unclear = 0,
    /// Ein Lag ist da, aber mindestens ein Kriterium fehlt — oder das Paar
    /// trägt einen der drei Herabstufungsgründe aus M-23.
    Probable = 1,
    /// Lokales Audio-Alignment mit GCC-PHAT. In P4 unerreichbar: es verlangt
    /// PRE und POST als Audiopuffer in derselben Instanz.
    AudioAligned = 2,
    /// Verteiltes Feature-Alignment, alle Kriterien erfüllt.
    FeatureAligned = 3,
}

/// Warum ein Paar gar keine Aussage trägt (M-22). Jeder Grund ist benannt;
/// eine schwache Zahl wäre schlimmer als keine.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Ausschlussgrund {
    /// Eine der beiden Hälften fehlt ganz.
    HaelfteFehlt,
    /// Zwei PRE oder zwei POST teilen dieselbe `pair_id` (M-13). Kein
    /// Zufallssieger: das Paar ist unbrauchbar, bis der User es eindeutig
    /// macht.
    Paarkonflikt,
    HaelfteGetrennt,
    HaelfteStale,
    /// Eine Hälfte sammelt noch — der Zustand ist nicht messbereit.
    NichtMessbereit,
    /// Keine Projektzeit: die Messung lief ohne Transport.
    KeineProjektzeit,
    /// Fensterlänge ≤ 0.
    FensterLeer,
    /// Verschiedene Samplerates (§38.2: ein Akzeptanzkriterium).
    SamplerateVerschieden,
    /// Die Messfenster überlappen nicht — „vermutlich verschiedene Passagen",
    /// nicht „0 % ähnlich" (M-17).
    KeineUeberlappung,
    /// Ein Sprung im Messfenster. Im v3-Pfad INVALIDIERT er, statt
    /// herabzustufen: eine Passage bindet an genau eine Transportepoche
    /// (§32.4), also ist ein Loop- oder Seeksprung ein Epochenbruch (M-23).
    SprungImFenster,
}

/// Warum ein vollständiges Paar nur `Probable` trägt (M-23). Im v3-Pfad sind
/// es genau drei; der vierte Grund des v2-Vorläufers ist zum Ausschluss
/// geworden, siehe `Ausschlussgrund::SprungImFenster`.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Herabstufungsgrund {
    /// Überlappung unter 80 % des kürzeren Fensters.
    UeberlappungGering,
    /// Aktive Messzeit um mehr als 10 % verschieden (Smart Disable oder
    /// Stille auf einem Punkt).
    AktivzeitKlafft,
    /// Die Hälften laufen in verschiedenen Prozessen (Bridge oder zweites FL).
    VerschiedeneHostPids,
    /// Ein Alignmentkriterium aus §38.2 ist nicht erfüllt.
    AlignmentSchwach,
}

/// Warum eine Kette keine statische EQ-Behauptung trägt (M-18).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Kettenbefund {
    /// Pegel- und Spektralrelationen sind über die Zeit konsistent.
    Stationaer,
    /// Die Bandrelationen schwanken über die Zeit (Tremolo, Modulation).
    ZeitvariabelMarkiert,
    /// Die Bandrelation hängt vom Pegel ab (Kompression, Saturation).
    PegelabhaengigMarkiert,
    /// Der geschätzte Lag ist über Teilfenster nicht stabil.
    LatenzWechseltMarkiert,
    /// Es gibt zu wenig verwertbares Material, um die Kette zu beurteilen.
    ///
    /// ⚠️ Das ist ausdrücklich NICHT `Stationaer`. Eine Kette, über die
    /// niemand etwas sagen kann, darf keinen festen Übertragungsgang tragen —
    /// „nie beurteilt" und „stationär" sind zwei verschiedene Aussagen, und
    /// nur die zweite erlaubt eine EQ-Behauptung.
    NichtBeurteilbar,
}

// ── Die Eingaben ─────────────────────────────────────────────────────────

/// Eine Hälfte des Paars, so wie der Broker sie aus der Telemetrie füllt.
#[derive(Debug, Clone)]
pub struct Paarhaelfte {
    pub instance_id: String,
    pub host_pid: u32,
    pub transport_epoch: u64,
    pub sample_rate: f64,
    /// Bandauflösung des Frames — Teil beider Frameschlüssel (M-21).
    pub resolution: u32,
    pub verbunden: bool,
    pub stale: bool,
    pub messbereit: bool,
    /// Projektfenster in Samples, halboffen `[von, bis)`. `None` = ohne
    /// Transport gemessen.
    pub projekt_fenster: Option<(i64, i64)>,
    /// Loop-/Seeksprünge im Messfenster.
    pub spruenge: u32,
    /// Aktive Messzeit in Sekunden — nicht die Wanddauer.
    pub aktiv_s: f64,
    /// Bandbegrenzte Hüllkurven über die Zeit: `huellkurven[band][frame]`.
    /// §38.2 verlangt MEHRERE Bänder, damit die Konsistenzprüfung überhaupt
    /// etwas zu prüfen hat.
    pub huellkurven: Vec<Vec<f32>>,
    /// Onsetstärke je Frame — die zweite, unabhängige Spur aus §38.2.
    pub onsets: Vec<f32>,
}

// ── Die Startgates ───────────────────────────────────────────────────────
//
// Wie die Vergleichbarkeitsgates (M-29) stehen sie im Register
// `metriken-v1.json` und werden von A5 dagegen gehalten. §38.3 sagt es für
// die Kohärenzschwelle ausdrücklich: "Diese Startschwellen werden mit
// Goldens kalibriert."

/// Die Fassung, deren Gates dieses Modul anwendet.
pub const METRICS_VERSION: u32 = 20260904;

/// Framekadenz der Feature-Telemetrie: ein Hop sind 100 ms.
pub const FEATURE_HOP_MS: i64 = 100;
/// Maximaler Suchraum in Sekunden (§38.2: "auf ±2 s begrenzt").
pub const GATE_SUCHRAUM_S: f64 = 2.0;
/// Capture, das dieser Suchraum verlangt (§38.2: "benötigt dafür mindestens
/// 6 s Capture, damit nach dem Shift noch verwertbares Material bleibt").
pub const GATE_MINDESTCAPTURE_S: f64 = 6.0;
/// Normierte Korrelationsspitze, ab der ein Lag überhaupt benannt wird.
pub const GATE_KORRELATIONSSPITZE: f64 = 0.5;
/// Peak-to-Sidelobe-Verhältnis (§38.2).
pub const GATE_PEAK_TO_SIDELOBE: f64 = 2.0;
/// Wie viele Spuren einen konsistenten Lag liefern müssen.
pub const GATE_KONSISTENTE_SPUREN: usize = 3;
/// Überlappung, unter der ein Paar herabgestuft wird (M-23).
pub const GATE_UEBERLAPPUNG: f64 = 0.8;
/// Relative Differenz der aktiven Messzeit, ab der herabgestuft wird (M-23).
pub const GATE_AKTIVZEIT_DIFFERENZ: f64 = 0.1;
/// Schwankung der Bandrelation POST/PRE, ab der eine Kette als zeitvariabel
/// oder nichtlinear markiert wird (M-18).
///
/// Der Wert liegt bewusst tief. Eine LINEARE Kette hält die Relation exakt
/// konstant — ihre Streuung ist bis auf die Quantisierung des Livepfads
/// (0,1 dB) null. Alles darüber ist keine Messstreuung, sondern Wirkung.
/// 1,5 dB liegt eine Größenordnung über dem Quantisierungsraster und unter
/// der Wirkung jedes ernstzunehmenden Kompressors; ein Startwert, wie §38.3
/// ihn für diese Grenze vorsieht.
pub const GATE_RELATIONSSCHWANKUNG_DB: f64 = 1.5;

// ── Die Ergebnisse ───────────────────────────────────────────────────────

/// Der geschätzte Restlag mit allem, was ihn beurteilbar macht (M-16:
/// "Geschätzter Lag, seine Auflösung und seine Streuung bleiben sichtbar").
#[derive(Debug, Clone, PartialEq)]
pub struct Restlag {
    /// Lag in Feature-Frames, positiv = POST liegt später.
    pub frames: i64,
    /// Auflösung in Millisekunden. Ausdrücklich KEINE Samplegenauigkeit
    /// (M-15) — ein Feature-Hop ist die feinste Aussage, die dieser Zweig
    /// treffen kann.
    pub aufloesung_ms: i64,
    /// Streuung der Einzelspuren um den Median, in Frames.
    pub streuung_frames: f64,
    pub spitze: f64,
    pub peak_to_sidelobe: f64,
    pub konsistente_spuren: usize,
    /// Der auf dem zweiten Teilfenster gemessene Lag. Weicht er ab, wechselt
    /// die Latenz (M-18).
    pub lag_zweite_haelfte: i64,
}

/// Ein Frameschlüssel. Die beiden Formen sind verschiedene Typen und keine
/// zwei Zustände desselben (M-21).
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Frameschluessel {
    /// Behauptet KEINE Inhaltsgleichheit zwischen Instanzen. Er ist an genau
    /// eine Instanz gebunden — das ist der Punkt.
    Roh {
        instance_id: String,
        transport_epoch: u64,
        sample_rate_milli: u64,
        resolution: u32,
        frame_start: i64,
    },
    /// Starke Cross-Probe-Evidenz. Nur mit validierter Presentation-Abbildung
    /// UND ausreichender Alignmentqualität.
    Ausgerichtet {
        session_epoch: u64,
        timeline_epoch: u64,
        sample_rate_milli: u64,
        resolution: u32,
        aligned_start: i64,
    },
}

/// Die drei getrennten Ergebnisse aus §38.1 (M-14).
#[derive(Debug, Clone, PartialEq)]
pub struct Dreifachergebnis {
    /// 1. Rohe Messdifferenz derselben Projektfenster, je Band in dB.
    pub roh_db: Vec<f64>,
    /// 2. Ausgerichtetes, pegelbezogenes Delta — erst nach sicherer Restlag-
    ///    und Gainschätzung. `None`, solange die nicht vorliegt: ein Delta
    ///    ohne Ausrichtung wäre eine Laufzeitänderung, die wie ein EQ aussieht.
    pub ausgerichtet_db: Option<Vec<f64>>,
    /// Der beim Ausrichten abgezogene Breitbandgain in dB.
    pub match_gain_db: Option<f64>,
    /// 3. Interpretierte Wirkung — nur als Wort, nie als Zahl.
    pub wirkung: Option<String>,
}

/// Das vollständige Urteil über ein Paar.
#[derive(Debug, Clone, PartialEq)]
pub struct Paarurteil {
    pub pair_id: String,
    pub klasse: Alignmentklasse,
    /// Bei `Unclear`: warum. Nie leer, wenn die Klasse `Unclear` ist.
    pub ausschluss: Option<Ausschlussgrund>,
    /// Bei `Probable`: alle Gründe, nicht nur der erste.
    pub herabstufungen: Vec<Herabstufungsgrund>,
    pub kettenbefund: Kettenbefund,
    pub restlag: Option<Restlag>,
    pub ueberlappung: f64,
    pub ergebnis: Option<Dreifachergebnis>,
    pub metrics_version: u32,
}

// ── Die Rechnungen ───────────────────────────────────────────────────────

/// Überlappung zweier Projektfenster, normiert auf das KÜRZERE (M-17).
///
/// ⚠️ Sättigend. Zwei gültige Fenster an entgegengesetzten `i64`-Rändern
/// dürfen weder paniken noch umbrechen und dadurch als deckungsgleich gelten.
/// Dieselbe Rechnung wie in `vergleichbarkeit::ueberdeckung`; sie steht
/// zweimal, weil die Eingaben verschieden sind und ein gemeinsamer Helfer die
/// beiden Verträge aneinanderbinden würde.
pub fn ueberlappung(a: (i64, i64), b: (i64, i64)) -> f64 {
    // `saturating_sub` und nicht `checked_sub(..).unwrap_or(0)`: zwei
    // IDENTISCHE Fenster über den ganzen Zahlenbereich hätten sonst eine
    // Überlappung von 0 statt 1 — der Ausfallwert 0 trifft dort genau den
    // Fall, den er schützen soll, und macht aus perfekter Deckung keine.
    // Sättigend ist beides zugleich richtig: ein Überlauf nach oben gibt
    // `i64::MAX`, einer nach unten `i64::MIN`, und `.max(0)` fängt den.
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

/// Der Suchraum in Frames (§38.2). Bei kürzerem Capture proportional kleiner.
pub fn suchraum_frames(capture_s: f64) -> i64 {
    let voll = (GATE_SUCHRAUM_S * 1000.0 / FEATURE_HOP_MS as f64).round() as i64;
    if capture_s >= GATE_MINDESTCAPTURE_S {
        return voll;
    }
    if capture_s <= 0.0 {
        return 0;
    }
    let anteil = capture_s / GATE_MINDESTCAPTURE_S;
    ((voll as f64) * anteil).floor().max(0.0) as i64
}

/// Normierte Kreuzkorrelation zweier Spuren bei einem Lag.
///
/// Zwei Eigenschaften, und beide sind beim Bau teuer bezahlt worden:
///
/// **Normiert wird auf den ÜBERLAPPENDEN Ausschnitt**, nicht auf die ganzen
/// Spuren. Sonst sänke der Wert allein deshalb, weil ein größerer Lag weniger
/// gemeinsame Frames lässt, und der Suchlauf bevorzugte systematisch Lag 0.
///
/// ⚠️ **Und sie ist MITTELWERTFREI.** Hüllkurven sind nichtnegativ; ihr
/// Kosinuswinkel liegt deshalb für JEDEN Lag nahe eins, weil beide Vektoren
/// im positiven Orthanten liegen und der Gleichanteil alles dominiert. Ein
/// Suchlauf über solche Werte findet zwar noch das richtige Maximum, aber das
/// Peak-to-Sidelobe-Verhältnis aus §38.2 wird dabei zu 1,00x und lehnt
/// jede Ausrichtung ab — auch die perfekte. Gemessen beim Bau von Etappe F:
/// identische Spuren ergaben Spitze 1,000 und PSR 1,008, und der Join stufte
/// sich selbst auf `Probable` herab.
fn korrelation(a: &[f32], b: &[f32], lag: i64) -> f64 {
    // Erster Durchlauf: die Mittelwerte des überlappenden Ausschnitts.
    let (mut sa, mut sb, mut n) = (0.0f64, 0.0f64, 0usize);
    for i in 0..a.len() {
        let j = i as i64 + lag;
        if j < 0 || j as usize >= b.len() {
            continue;
        }
        let (x, y) = (a[i] as f64, b[j as usize] as f64);
        // Derselbe Fund wie in `relation_db`: ein NaN macht Spitze und
        // Peak-to-Sidelobe zu NaN, und `NaN < GATE` ist false — die
        // Alignmentkriterien griffen dann NICHT und ein kaputter Frame
        // erzeugte eine starke Ausrichtung.
        if !x.is_finite() || !y.is_finite() {
            continue;
        }
        sa += x;
        sb += y;
        n += 1;
    }
    if n < 2 {
        return 0.0;
    }
    let (ma, mb) = (sa / n as f64, sb / n as f64);

    // Zweiter Durchlauf: Pearson über denselben Ausschnitt.
    let (mut summe, mut ea, mut eb) = (0.0f64, 0.0f64, 0.0f64);
    for i in 0..a.len() {
        let j = i as i64 + lag;
        if j < 0 || j as usize >= b.len() {
            continue;
        }
        let (rx, ry) = (a[i] as f64, b[j as usize] as f64);
        if !rx.is_finite() || !ry.is_finite() {
            continue;
        }
        let (x, y) = (rx - ma, ry - mb);
        summe += x * y;
        ea += x * x;
        eb += y * y;
    }
    if ea <= 1e-12 || eb <= 1e-12 {
        // Eine konstante Spur hat keine Struktur, an der sich etwas
        // ausrichten liesse. Das ist nicht "perfekt korreliert".
        return 0.0;
    }
    summe / (ea.sqrt() * eb.sqrt())
}

/// Wählt aus einer Lagkurve das Maximum.
///
/// ⚠️ Bei Gleichstand gewinnt der betragsmässig KLEINSTE Lag, und diese Regel
/// muss überall dieselbe sein. Exakt periodisches Material — ein geloopter
/// Takt — hat mehrere gleich hohe Maxima; solange die Einzelspur das erste
/// und die Summenkurve das letzte wählte, waren sich beide bei genau dem
/// Material uneinig, für das die Konsistenzprüfung gedacht ist. Gemessen beim
/// Bau von Etappe F: `konsistente_spuren` fiel auf 0, obwohl alle Spuren
/// dasselbe Signal trugen.
///
/// Der kleinste Betrag ist zugleich die sparsamste Erklärung: wo kein Versatz
/// nötig ist, wird keiner behauptet.
fn maximum(kurve: &[(i64, f64)]) -> Option<(i64, f64)> {
    let mut bester: Option<(i64, f64)> = None;
    for &(lag, wert) in kurve {
        let besser = match bester {
            None => true,
            // Die Toleranz ist nicht Kosmetik: zwei mathematisch gleich hohe
            // Maxima unterscheiden sich in der letzten Nachkommastelle, und
            // ohne sie entschiede diese Stelle, welcher Lag gewinnt. Beim Bau
            // von Etappe F gemessen: dasselbe periodische Signal ergab im
            // ganzen Fenster Lag 0 und in der zweiten Haelfte -5, und das
            // Stabilitaetskriterium schlug auf reines Rauschen an.
            Some((bl, bw)) => wert > bw + 1e-9
                || ((wert - bw).abs() <= 1e-9 && lag.abs() < bl.abs()),
        };
        if besser {
            bester = Some((lag, wert));
        }
    }
    bester
}

/// Der beste Lag EINER Spur, ohne Bewertung. Die Bewertung passiert auf der
/// Summenkurve, siehe `summenkurve`.
fn bester_lag(a: &[f32], b: &[f32], max_lag: i64) -> Option<i64> {
    let kurve: Vec<(i64, f64)> = (-max_lag..=max_lag)
        .map(|lag| (lag, korrelation(a, b, lag)))
        .collect();
    maximum(&kurve).filter(|(_, v)| *v > 0.0).map(|(l, _)| l)
}

/// Die über ALLE Spuren summierte Korrelationskurve, und ihre Bewertung.
///
/// ⚠️ Spitze und Peak-to-Sidelobe gehören auf DIESE Kurve und nicht auf die
/// einzelne Spur. Musik ist rhythmisch periodisch: die Korrelationskurve
/// einer einzelnen Bandhüllkurve hat bei jedem Vielfachen ihrer Taktperiode
/// ein fast gleich hohes Nebenmaximum, und ihr PSR liegt deshalb auch bei
/// perfekter Ausrichtung nahe 1,0. Gemessen beim Bau von Etappe F: identische
/// Spuren ergaben je Spur einen PSR von 1,07 und hätten jede Ausrichtung
/// abgelehnt.
///
/// Über mehrere Spuren mit VERSCHIEDENEN Periodizitäten summieren sich die
/// Nebenmaxima nicht auf, die echte Spitze schon. Genau das meint §38.2 mit
/// „mehrere Bänder einen konsistenten Lag liefern" — die Mehrdeutigkeit einer
/// einzelnen Spur wird durch die anderen aufgelöst, nicht durch eine
/// schärfere Schwelle.
fn summenkurve(spuren: &[(&[f32], &[f32])], max_lag: i64) -> Option<(i64, f64, f64)> {
    if spuren.is_empty() || max_lag <= 0 {
        return None;
    }
    let mut kurve: Vec<(i64, f64)> = Vec::with_capacity((2 * max_lag + 1) as usize);
    for lag in -max_lag..=max_lag {
        let summe: f64 = spuren.iter().map(|(a, b)| korrelation(a, b, lag)).sum();
        kurve.push((lag, summe / spuren.len() as f64));
    }
    let (bester, spitze) = maximum(&kurve)?;
    if spitze <= 0.0 {
        return None;
    }
    // Sidelobe: das größte Maximum AUSSERHALB einer Sperrzone um die Spitze.
    // Ohne sie wäre die Nachbarzelle das Sidelobe, und das Verhältnis misste
    // die Breite der Spitze statt ihrer Eindeutigkeit.
    let mut sidelobe = 0.0f64;
    for &(lag, wert) in &kurve {
        if (lag - bester).abs() > 2 {
            sidelobe = sidelobe.max(wert);
        }
    }
    let psr = if sidelobe > 1e-9 {
        spitze / sidelobe
    } else {
        // Kein positives Nebenmaximum: die Spitze steht allein. Das ist der
        // eindeutigste Fall, den es gibt.
        f64::INFINITY
    };
    Some((bester, spitze, psr))
}


/// Schätzt den Restlag über alle Hüllkurven und die Onsetfolge (M-15).
///
/// Rückgabe `None`, wenn sich gar kein Lag benennen lässt — Rauschen oder
/// anderes Material. Das ist ausdrücklich nicht dasselbe wie „Lag 0".
pub fn schaetze_restlag(pre: &Paarhaelfte, post: &Paarhaelfte, capture_s: f64) -> Option<Restlag> {
    let max_lag = suchraum_frames(capture_s);
    if max_lag <= 0 {
        return None;
    }

    // Alle Spuren: die Bandhüllkurven UND die Onsetfolge. §38.2 nennt beide,
    // und sie beantworten verschiedene Fragen — eine Hüllkurve findet einen
    // Lag auch in gehaltenen Tönen, eine Onsetfolge auch bei stark
    // verändertem Klang.
    let mut spuren: Vec<(&[f32], &[f32])> = Vec::new();
    for (a, b) in pre.huellkurven.iter().zip(post.huellkurven.iter()) {
        spuren.push((a.as_slice(), b.as_slice()));
    }
    spuren.push((pre.onsets.as_slice(), post.onsets.as_slice()));
    if spuren.is_empty() {
        return None;
    }

    // Spitze und PSR auf der Summenkurve, die Konsistenz je Spur.
    let (zentral, spitze, psr) = summenkurve(&spuren, max_lag)?;

    let lags: Vec<i64> = spuren
        .iter()
        .filter_map(|(a, b)| bester_lag(a, b, max_lag))
        .collect();
    if lags.is_empty() {
        return None;
    }
    let konsistent = lags.iter().filter(|l| (**l - zentral).abs() <= 1).count();
    let streuung = if lags.len() > 1 {
        let mittel = lags.iter().map(|l| *l as f64).sum::<f64>() / lags.len() as f64;
        (lags.iter().map(|l| (*l as f64 - mittel).powi(2)).sum::<f64>() / lags.len() as f64).sqrt()
    } else {
        0.0
    };

    // Stabilität über Teilfenster: dieselbe Rechnung auf der zweiten Hälfte.
    // Eine wechselnde Latenz zeigt sich genau hier und nirgends sonst.
    let zweite = zweite_haelfte_lag(&spuren, max_lag).unwrap_or(zentral);

    Some(Restlag {
        frames: zentral,
        aufloesung_ms: FEATURE_HOP_MS,
        streuung_frames: streuung,
        spitze,
        peak_to_sidelobe: psr,
        konsistente_spuren: konsistent,
        lag_zweite_haelfte: zweite,
    })
}

fn zweite_haelfte_lag(spuren: &[(&[f32], &[f32])], max_lag: i64) -> Option<i64> {
    let hinten: Vec<(&[f32], &[f32])> = spuren
        .iter()
        .filter_map(|(a, b)| {
            let (ma, mb) = (a.len() / 2, b.len() / 2);
            if a.len() - ma < 4 || b.len() - mb < 4 {
                None
            } else {
                Some((&a[ma..], &b[mb..]))
            }
        })
        .collect();
    summenkurve(&hinten, max_lag).map(|(lag, _, _)| lag)
}

// ── Der Kettenbefund (M-18) ──────────────────────────────────────────────

/// Mittlere Relation POST/PRE je Band in dB, über die gemeinsamen Frames.
fn relation_db(pre: &[f32], post: &[f32], lag: i64) -> Vec<f64> {
    let mut aus = Vec::new();
    for i in 0..pre.len() {
        let j = i as i64 + lag;
        if j < 0 || j as usize >= post.len() {
            continue;
        }
        let (x, y) = (pre[i] as f64, post[j as usize] as f64);
        // ⚠️ Die Endlichkeitsprüfung steht VOR dem Vergleich, und das ist
        // kein Stil. Jeder Vergleich mit NaN ist false — `x <= 1e-9` lässt
        // ein NaN also durch, die Relation wird NaN, die Streuung wird NaN,
        // und `NaN > GATE` ist wieder false: die Kette gälte als STATIONÄR
        // und dürfte einen festen Übertragungsgang tragen. Ein einziger
        // kaputter Frame hätte damit genau die Behauptung erlaubt, die M-18
        // verbietet. Gefunden im Selbstaudit von Etappe H.
        if !x.is_finite() || !y.is_finite() {
            continue;
        }
        // Ohne PRE-Energie gibt es keine Relation. Sie auf 0 dB zu setzen
        // hiesse, Stille als "die Kette tut nichts" zu lesen.
        if x <= 1e-9 || y <= 1e-9 {
            continue;
        }
        aus.push(20.0 * (y / x).log10());
    }
    aus
}

fn standardabweichung(werte: &[f64]) -> f64 {
    if werte.len() < 2 {
        return 0.0;
    }
    let mittel = werte.iter().sum::<f64>() / werte.len() as f64;
    (werte.iter().map(|v| (v - mittel).powi(2)).sum::<f64>() / werte.len() as f64).sqrt()
}

/// Pearson-Korrelation zweier gleich langer Folgen.
fn pearson(a: &[f64], b: &[f64]) -> f64 {
    let n = a.len().min(b.len());
    if n < 3 {
        return 0.0;
    }
    let ma = a[..n].iter().sum::<f64>() / n as f64;
    let mb = b[..n].iter().sum::<f64>() / n as f64;
    let (mut zaehler, mut sa, mut sb) = (0.0, 0.0, 0.0);
    for i in 0..n {
        let (x, y) = (a[i] - ma, b[i] - mb);
        zaehler += x * y;
        sa += x * x;
        sb += y * y;
    }
    if sa <= 1e-12 || sb <= 1e-12 {
        return 0.0;
    }
    zaehler / (sa.sqrt() * sb.sqrt())
}

/// Markiert eine zeitvariable oder nichtlineare Kette (M-18).
///
/// Die Reihenfolge ist die von der spezifischsten zur allgemeinsten Ursache.
/// Eine wechselnde Latenz kommt zuerst, weil sie JEDE andere Messung
/// verfälscht: eine um Frames verschobene Hüllkurve sieht aus wie eine
/// pegelabhängige Kette, ohne eine zu sein.
pub fn kettenbefund(pre: &Paarhaelfte, post: &Paarhaelfte, restlag: Option<&Restlag>) -> Kettenbefund {
    let lag = restlag.map(|r| r.frames).unwrap_or(0);

    if let Some(r) = restlag {
        if r.lag_zweite_haelfte != r.frames {
            return Kettenbefund::LatenzWechseltMarkiert;
        }
    }

    let mut pegelabhaengig = false;
    let mut zeitvariabel = false;
    let mut beurteilbare_baender = 0usize;
    for (a, b) in pre.huellkurven.iter().zip(post.huellkurven.iter()) {
        let rel = relation_db(a, b, lag);
        if rel.len() < 3 {
            continue;
        }
        // Der PRE-Pegel derselben Frames, in dB — die zweite Achse der
        // Pegelabhängigkeit.
        let mut pegel = Vec::new();
        for i in 0..a.len() {
            let j = i as i64 + lag;
            if j < 0 || j as usize >= b.len() {
                continue;
            }
            let (x, y) = (a[i] as f64, b[j as usize] as f64);
            if x <= 1e-9 || y <= 1e-9 {
                continue;
            }
            pegel.push(20.0 * x.log10());
        }
        beurteilbare_baender += 1;
        let schwankung = standardabweichung(&rel);
        if schwankung > GATE_RELATIONSSCHWANKUNG_DB {
            zeitvariabel = true;
            // Hängt die Relation am Pegel, ist das die genauere Aussage.
            if pearson(&pegel, &rel).abs() > 0.7 {
                pegelabhaengig = true;
            }
        }
    }

    if pegelabhaengig {
        Kettenbefund::PegelabhaengigMarkiert
    } else if zeitvariabel {
        Kettenbefund::ZeitvariabelMarkiert
    } else if beurteilbare_baender == 0 {
        // Kein einziges Band lieferte genug endliche Relationswerte. Ohne
        // Beleg gibt es keine Aussage - und schon gar keine, die einen
        // Frequenzgang trägt.
        Kettenbefund::NichtBeurteilbar
    } else {
        Kettenbefund::Stationaer
    }
}

// ── Die drei Ergebnisse (M-14) ───────────────────────────────────────────

fn mittlere_relation(pre: &Paarhaelfte, post: &Paarhaelfte, lag: i64) -> Vec<f64> {
    let mut aus = Vec::new();
    for (a, b) in pre.huellkurven.iter().zip(post.huellkurven.iter()) {
        let rel = relation_db(a, b, lag);
        aus.push(if rel.is_empty() {
            f64::NAN
        } else {
            rel.iter().sum::<f64>() / rel.len() as f64
        });
    }
    aus
}

/// Baut die drei getrennten Ergebnisse (M-14).
///
/// Das ausgerichtete Delta entsteht NUR bei sicherem Alignment. Ohne das wäre
/// es eine Laufzeitänderung, die wie ein EQ-Effekt aussieht — genau die
/// Verwechslung, die §38.1 mit der Dreiteilung ausschliesst. Und die
/// interpretierte Wirkung entsteht nur zusätzlich bei stationärer Kette:
/// über eine markierte Kette darf Nakama keinen festen Übertragungsgang
/// behaupten (M-18).
pub fn dreifachergebnis(
    pre: &Paarhaelfte,
    post: &Paarhaelfte,
    klasse: Alignmentklasse,
    restlag: Option<&Restlag>,
    befund: Kettenbefund,
) -> Dreifachergebnis {
    // 1. Roh: dieselben Projektfenster, OHNE Lagkorrektur.
    let roh = mittlere_relation(pre, post, 0);

    let sicher = klasse >= Alignmentklasse::AudioAligned;
    let (ausgerichtet, gain) = if sicher {
        let lag = restlag.map(|r| r.frames).unwrap_or(0);
        let voll = mittlere_relation(pre, post, lag);
        let endlich: Vec<f64> = voll.iter().cloned().filter(|v| v.is_finite()).collect();
        if endlich.is_empty() {
            (None, None)
        } else {
            // Der Breitbandgain ist der Median, nicht der Mittelwert: ein
            // einzelnes stark verändertes Band soll ihn nicht ziehen.
            let mut sortiert = endlich.clone();
            sortiert.sort_by(|a, b| a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal));
            let g = sortiert[sortiert.len() / 2];
            (
                Some(voll.iter().map(|v| v - g).collect::<Vec<f64>>()),
                Some(g),
            )
        }
    } else {
        (None, None)
    };

    let wirkung = match (&ausgerichtet, befund) {
        (Some(delta), Kettenbefund::Stationaer) => Some(beschreibe_wirkung(delta)),
        (Some(_), Kettenbefund::NichtBeurteilbar) => Some(
            "wahrscheinliche PRE/POST-Wirkung (Kette nicht beurteilbar)".to_string(),
        ),
        (Some(_), _) => Some(
            "wahrscheinliche PRE/POST-Wirkung (Kette zeitvariabel oder nichtlinear markiert)"
                .to_string(),
        ),
        (None, _) => None,
    };

    Dreifachergebnis {
        roh_db: roh,
        ausgerichtet_db: ausgerichtet,
        match_gain_db: gain,
        wirkung,
    }
}

/// Ein Wort für die Wirkung, nie eine Zahl (§38.1: "interpretierte Wirkung").
fn beschreibe_wirkung(delta: &[f64]) -> String {
    let n = delta.len();
    if n < 3 {
        return "zu wenige Bänder für eine Aussage".into();
    }
    let drittel = n / 3;
    let mittel = |v: &[f64]| {
        let e: Vec<f64> = v.iter().cloned().filter(|x| x.is_finite()).collect();
        if e.is_empty() {
            0.0
        } else {
            e.iter().sum::<f64>() / e.len() as f64
        }
    };
    let tief = mittel(&delta[..drittel]);
    let hoch = mittel(&delta[n - drittel..]);
    if (hoch - tief).abs() < 1.0 {
        "Kette wirkt breitbandig gleichmäßig".into()
    } else if hoch > tief {
        "Kette hebt die oberen Bänder gegenüber den unteren".into()
    } else {
        "Kette senkt die oberen Bänder gegenüber den unteren".into()
    }
}

// ── Die Frameschlüssel (M-21) ────────────────────────────────────────────

/// Baut den Frameschlüssel für eine Hälfte.
///
/// Das Exit-Gate wörtlich: „Kein unbekannter Zeitpfad erzeugt eine starke
/// Cross-Probe- oder PRE/POST-Aussage." Der ausgerichtete Schlüssel verlangt
/// deshalb BEIDES — eine validierte Presentation-Abbildung und
/// Alignmentqualität. Eines allein genügt nicht: eine validierte Abbildung
/// ohne Alignment weiß nicht, welche Frames zusammengehören, und ein gutes
/// Alignment ohne Abbildung weiß nicht, ob die Zeitachsen überhaupt dasselbe
/// meinen.
pub fn frameschluessel(
    haelfte: &Paarhaelfte,
    frame_start: i64,
    presentation_validiert: bool,
    session_epoch: u64,
    timeline_epoch: u64,
    klasse: Alignmentklasse,
    lag_frames: i64,
) -> Frameschluessel {
    let rate = (haelfte.sample_rate * 1000.0).round().max(0.0) as u64;
    if presentation_validiert && klasse >= Alignmentklasse::AudioAligned {
        let versatz = lag_frames
            .checked_mul(FEATURE_HOP_MS)
            .and_then(|ms| ms.checked_mul(haelfte.sample_rate.round() as i64))
            .map(|x| x / 1000)
            .unwrap_or(0);
        return Frameschluessel::Ausgerichtet {
            session_epoch,
            timeline_epoch,
            sample_rate_milli: rate,
            resolution: haelfte.resolution,
            aligned_start: frame_start.saturating_add(versatz),
        };
    }
    Frameschluessel::Roh {
        instance_id: haelfte.instance_id.clone(),
        transport_epoch: haelfte.transport_epoch,
        sample_rate_milli: rate,
        resolution: haelfte.resolution,
        frame_start,
    }
}

// ── Das Urteil ───────────────────────────────────────────────────────────

fn ausschluss(pair_id: &str, grund: Ausschlussgrund, ueberlappung: f64) -> Paarurteil {
    Paarurteil {
        pair_id: pair_id.to_string(),
        klasse: Alignmentklasse::Unclear,
        ausschluss: Some(grund),
        herabstufungen: Vec::new(),
        kettenbefund: Kettenbefund::Stationaer,
        restlag: None,
        ueberlappung,
        ergebnis: None,
        metrics_version: METRICS_VERSION,
    }
}

/// Das Urteil über ein vollständiges Paar (M-13 bis M-23).
///
/// Reihenfolge, und jeder Schritt kann allein fallen:
///
/// 1. **harte Ausschlüsse** (M-22) — jeder mit benanntem Grund, nie eine
///    schwache Zahl;
/// 2. **Überlappung** (M-17), sättigend und auf das kürzere Fenster normiert;
/// 3. **Restlag** (M-15) über Hüllkurven und Onsetfolge;
/// 4. **Alignmentkriterien** (M-16), jedes einzeln;
/// 5. **Herabstufungen** (M-23) — genau drei Gründe im v3-Pfad;
/// 6. **Kettenbefund** (M-18) und die drei Ergebnisse (M-14).
pub fn beurteile_paar(pair_id: &str, pre: &Paarhaelfte, post: &Paarhaelfte) -> Paarurteil {
    // 1. Harte Ausschlüsse.
    if !pre.verbunden || !post.verbunden {
        return ausschluss(pair_id, Ausschlussgrund::HaelfteGetrennt, 0.0);
    }
    if pre.stale || post.stale {
        return ausschluss(pair_id, Ausschlussgrund::HaelfteStale, 0.0);
    }
    if !pre.messbereit || !post.messbereit {
        return ausschluss(pair_id, Ausschlussgrund::NichtMessbereit, 0.0);
    }
    if pre.sample_rate != post.sample_rate || !(pre.sample_rate > 0.0) {
        return ausschluss(pair_id, Ausschlussgrund::SamplerateVerschieden, 0.0);
    }
    let (Some(fp), Some(fq)) = (pre.projekt_fenster, post.projekt_fenster) else {
        return ausschluss(pair_id, Ausschlussgrund::KeineProjektzeit, 0.0);
    };
    let len_p = fp.1.checked_sub(fp.0).unwrap_or(i64::MAX);
    let len_q = fq.1.checked_sub(fq.0).unwrap_or(i64::MAX);
    if len_p <= 0 || len_q <= 0 {
        return ausschluss(pair_id, Ausschlussgrund::FensterLeer, 0.0);
    }
    // Ein Sprung invalidiert im v3-Pfad, statt herabzustufen (M-23): eine
    // Passage bindet an genau eine Transportepoche (§32.4).
    if pre.spruenge > 0 || post.spruenge > 0 {
        return ausschluss(pair_id, Ausschlussgrund::SprungImFenster, 0.0);
    }

    // 2. Überlappung.
    let ueber = ueberlappung(fp, fq);
    if ueber <= 0.0 {
        return ausschluss(pair_id, Ausschlussgrund::KeineUeberlappung, 0.0);
    }

    // 3. Restlag auf dem gemeinsamen Material.
    let kuerzer = len_p.min(len_q) as f64;
    let capture_s = kuerzer / pre.sample_rate;
    let restlag = schaetze_restlag(pre, post, capture_s);

    // Ohne benennbaren Lag gibt es keinen Zeitbezug — Rauschen oder anderes
    // Material (M-16). Das ist NICHT "Lag 0".
    let Some(lag) = restlag else {
        let befund = kettenbefund(pre, post, None);
        return Paarurteil {
            pair_id: pair_id.to_string(),
            klasse: Alignmentklasse::Unclear,
            ausschluss: None,
            herabstufungen: Vec::new(),
            kettenbefund: befund,
            restlag: None,
            ueberlappung: ueber,
            ergebnis: None,
            metrics_version: METRICS_VERSION,
        };
    };

    // 4. Die Alignmentkriterien aus §38.2 — jedes fällt allein.
    let mut gruende = Vec::new();
    if lag.spitze < GATE_KORRELATIONSSPITZE
        || lag.peak_to_sidelobe < GATE_PEAK_TO_SIDELOBE
        || lag.konsistente_spuren < GATE_KONSISTENTE_SPUREN
        || lag.lag_zweite_haelfte != lag.frames
    {
        gruende.push(Herabstufungsgrund::AlignmentSchwach);
    }

    // 5. Die drei Herabstufungsgründe aus M-23.
    if ueber < GATE_UEBERLAPPUNG {
        gruende.push(Herabstufungsgrund::UeberlappungGering);
    }
    let (klein, gross) = (pre.aktiv_s.min(post.aktiv_s), pre.aktiv_s.max(post.aktiv_s));
    if gross > 0.0 && (gross - klein) > GATE_AKTIVZEIT_DIFFERENZ * gross {
        gruende.push(Herabstufungsgrund::AktivzeitKlafft);
    }
    if pre.host_pid != post.host_pid {
        gruende.push(Herabstufungsgrund::VerschiedeneHostPids);
    }

    // `AudioAligned` bleibt in P4 unerreichbar: es verlangt PRE und POST als
    // Audiopuffer in derselben Instanz, und Compare-Routing gibt es hier
    // nicht (M-19, M-24).
    let klasse = if gruende.is_empty() {
        Alignmentklasse::FeatureAligned
    } else {
        Alignmentklasse::Probable
    };

    // 6. Kette und Ergebnisse.
    let befund = kettenbefund(pre, post, Some(&lag));
    let ergebnis = dreifachergebnis(pre, post, klasse, Some(&lag), befund);

    Paarurteil {
        pair_id: pair_id.to_string(),
        klasse,
        ausschluss: None,
        herabstufungen: gruende,
        kettenbefund: befund,
        restlag: Some(lag),
        ueberlappung: ueber,
        ergebnis: Some(ergebnis),
        metrics_version: METRICS_VERSION,
    }
}

/// Welche Rolle eine Hälfte im Paar hat.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Rolle {
    Pre,
    Post,
}

/// Bildet Paare aus einer Menge von Hälften (M-13).
///
/// Ein Paar entsteht NUR aus genau einer PRE- und genau einer POST-Hälfte
/// derselben `pair_id`. Zwei PRE oder zwei POST sind ein Konflikt und kein
/// Zufallssieger — `first()` zu nehmen hiesse, dem User eine Aussage über
/// einen Messpunkt zu geben, den er nicht gewählt hat.
pub fn bilde_paare(haelften: &[(String, Rolle, Paarhaelfte)]) -> Vec<Paarurteil> {
    let ids: BTreeSet<&str> = haelften.iter().map(|(id, _, _)| id.as_str()).collect();
    let mut aus = Vec::new();
    for id in ids {
        let pres: Vec<&Paarhaelfte> = haelften
            .iter()
            .filter(|(i, r, _)| i == id && *r == Rolle::Pre)
            .map(|(_, _, h)| h)
            .collect();
        let posts: Vec<&Paarhaelfte> = haelften
            .iter()
            .filter(|(i, r, _)| i == id && *r == Rolle::Post)
            .map(|(_, _, h)| h)
            .collect();
        if pres.len() > 1 || posts.len() > 1 {
            aus.push(ausschluss(id, Ausschlussgrund::Paarkonflikt, 0.0));
            continue;
        }
        match (pres.first(), posts.first()) {
            (Some(p), Some(q)) => aus.push(beurteile_paar(id, p, q)),
            _ => aus.push(ausschluss(id, Ausschlussgrund::HaelfteFehlt, 0.0)),
        }
    }
    aus
}
