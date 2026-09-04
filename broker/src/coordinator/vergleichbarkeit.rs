//! Vergleichbarkeit zweier Passagen (SONDE-013 M-28, M-29, M-30; §32.4, §43.2).
//!
//! ## Fuenf Belege, und keiner ersetzt einen anderen
//!
//! §32.4 zaehlt sie abschliessend auf: identischer Projektbereich, gleicher
//! oder sehr aehnlicher Fingerprint, vergleichbare aktive Quellen, gleiche
//! Samplerate und Messpunktklasse, ausreichende gemeinsame Abdeckung. Der
//! Satz, der dieses Modul traegt, steht in M-28:
//!
//! > "KEIN einzelner Hash hebt einen Widerspruch der anderen Belege auf."
//!
//! Das ist keine Formulierungsfrage. Ein Fingerprint ist eine 76-Byte-
//! Zusammenfassung von Sekunden Audio (§10.6) — er ist bewusst nicht
//! injektiv, und zwei verschiedene Passagen koennen ihn teilen. Wer aus einer
//! Fingerprintgleichheit auf Vergleichbarkeit schliesst, hat genau den
//! Fehler gemacht, vor dem §15 warnt: er mittelt zwei Stellen der Musik zu
//! einer Aussage.
//!
//! Deshalb ist die Klasse hier ein MINIMUM ueber die fuenf Belege und kein
//! gewichteter Score — dieselbe Bauform wie `Konfidenzklasse` in
//! `Konfidenz.h` und wie `fingerprintAehnlichkeit` daneben. Ein Deckel laesst
//! sich nicht mitteln.
//!
//! ## Der Broker rechnet, nicht die Sonde
//!
//! §7.1 E-07: "Comparability-Score im Broker." Er haelt Sessiongraph und
//! Passagen (§33.5); Gen haelt nur IDs und bekommt den Score ueber den
//! Snapshotpfad. Eine Sonde koennte ihn gar nicht rechnen — sie sieht ihre
//! eigene Coverage und sonst keine der fuenf Groessen.
//!
//! ## Die Startgates sind Startwerte
//!
//! §43.2 nennt 95 % Zeitueberdeckung, Aktivquellen-Jaccard 0,9 und
//! Upstream-Feature-Cosine 0,95 und sagt ausdruecklich, dass sie am Korpus zu
//! kalibrieren sind. Sie stehen deshalb im Register
//! `eq-copilot/schemas/v3/metriken-v1.json` und nicht als Literal im Pfad;
//! **A5** haelt beide gegeneinander (M-29).

/// Wie stark zwei Passagen vergleichbar sind (§43.2).
///
/// Die Ordnung ist tragend: `min` zweier Klassen ist die Deckelung, um die
/// es in M-28 geht.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Vergleichbarkeit {
    /// Mindestens ein Beleg widerspricht. Kein Vergleich, kein Siegerlabel.
    Unvergleichbar = 0,
    /// Alle fuenf Belege liegen vor, aber mindestens einer unter seinem Gate.
    Schwach = 1,
    /// Alle fuenf Belege liegen vor und ueber ihrem Gate.
    Stark = 2,
}

impl Vergleichbarkeit {
    pub fn name(self) -> &'static str {
        match self {
            Vergleichbarkeit::Stark => "stark",
            Vergleichbarkeit::Schwach => "schwach",
            Vergleichbarkeit::Unvergleichbar => "unvergleichbar",
        }
    }
}

/// Warum eine Vergleichbarkeit nicht `Stark` ist.
///
/// Sie reist MIT, weil M-28 eine benannte Herabstufung verlangt und keine
/// stille: "Keine Herabstufung ist ein stiller Ausschluss." Ein Score ohne
/// Grund liesse den Empfaenger raten, welcher der fuenf Belege fehlt.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Herabstufungsgrund {
    /// Die zwei Passagen liegen nicht im selben Projektbereich.
    ProjektbereichVerschieden,
    /// Der Fingerprint sagt: anderes Material.
    MaterialVerschieden,
    /// Das aktive Quellenset unterscheidet sich zu stark.
    QuellenVerschieden,
    /// Samplerate oder Messpunktklasse stimmen nicht ueberein.
    MesspunktVerschieden,
    /// Die gemeinsame Abdeckung reicht nicht.
    AbdeckungZuGering,
}

impl Herabstufungsgrund {
    pub fn name(self) -> &'static str {
        match self {
            Herabstufungsgrund::ProjektbereichVerschieden => "projektbereich_verschieden",
            Herabstufungsgrund::MaterialVerschieden => "material_verschieden",
            Herabstufungsgrund::QuellenVerschieden => "quellen_verschieden",
            Herabstufungsgrund::MesspunktVerschieden => "messpunkt_verschieden",
            Herabstufungsgrund::AbdeckungZuGering => "abdeckung_zu_gering",
        }
    }
}

/// Die fuenf Belege einer Passage, so wie der Broker sie sieht.
#[derive(Debug, Clone, Default)]
pub struct Passagenbeleg {
    /// Projektbereich in Projektsamples, halboffen `[start, ende)`.
    pub projekt_start: i64,
    pub projekt_ende: i64,
    /// Die drei Fingerprintverlaeufe, wie sie auf der Leitung reisen.
    pub fingerprint: Option<crate::telemetrie::Fingerprintwerte>,
    /// Aktives Quellenset als `instance_id`-Liste.
    pub aktive_quellen: Vec<String>,
    pub samplerate: f64,
    /// Messpunktklasse je Quelle, in derselben Reihenfolge wie
    /// `aktive_quellen`.
    pub messpunktklassen: Vec<String>,
    /// GEMESSENE Signalabdeckung in [0, 1] — nicht die Wanddauer (M-30,
    /// §48.2). Smart Disable und Suspend gelten als FEHLENDE Coverage.
    pub abdeckung: f32,
}

/// Das Ergebnis: eine Klasse, ihre Gruende und die drei Gate-Zahlen.
#[derive(Debug, Clone, PartialEq)]
pub struct Vergleichsurteil {
    pub klasse: Vergleichbarkeit,
    /// Alle Gruende, nicht nur der erste. Ein Empfaenger, der nur den ersten
    /// saehe, behobe ihn und faende den naechsten — die Liste sagt ihm
    /// gleich, wie weit es ist.
    pub gruende: Vec<Herabstufungsgrund>,
    /// Zeitueberdeckung in [0, 1], normiert auf das KUERZERE Fenster.
    pub zeitueberdeckung: f64,
    /// Jaccard-Index der aktiven Quellensets.
    pub quellen_jaccard: f64,
    /// Cosinus-Aehnlichkeit der Fingerprints (Minimum ueber die drei
    /// Verlaeufe, siehe `fingerprint_aehnlichkeit`).
    pub material_cosine: f64,
    /// Die Kalibrierungsfassung, unter der DIESES Urteil entstanden ist.
    ///
    /// Ohne sie waere ein gespeichertes Urteil nach der naechsten
    /// Kalibrierung eine Behauptung ohne Massstab: dieselbe Klasse `Stark`
    /// meint bei anderen Gates etwas anderes. Dasselbe Muster wie das
    /// `version`-Feld im Fingerprint (§10.6) - der Erzeugerstand reist mit,
    /// statt beim Lesen aus der jeweils aktuellen Fassung geholt zu werden.
    pub metrics_version: u32,
}

// ── Die Startgates aus §43.2 ─────────────────────────────────────────────
//
// Sie stehen im Register `metriken-v1.json` und werden von A5 dagegen
// gehalten (M-29). Eine Kalibrierung ist damit ein Versionsschritt statt
// einer stillen Zeilenaenderung — Risiko 5 aus §5.3.

/// Die Fassung des Registers, deren Gates dieses Modul anwendet.
///
/// A5 haelt sie gegen `aktuell` in `metriken-v1.json` (M-29) - dieselbe
/// Bindung, die `kFeatureMetricsVersion` auf der Sondenseite hat. Wer hier
/// eine Zahl aendert und die Fassung stehen laesst, faellt rot.
pub const METRICS_VERSION: u32 = 20260904;

/// Zeitueberdeckung fuer eine STARKE Vergleichbarkeit (§43.2: 95 %).
pub const GATE_ZEITUEBERDECKUNG: f64 = 0.95;
/// Aktivquellen-Jaccard fuer eine starke Vergleichbarkeit (§43.2: 0,9).
pub const GATE_QUELLEN_JACCARD: f64 = 0.9;
/// Upstream-Feature-Cosine fuer eine starke Vergleichbarkeit (§43.2: 0,95).
pub const GATE_MATERIAL_COSINE: f64 = 0.95;
/// Ab welcher gemessenen Abdeckung eine Passage ueberhaupt einen Vergleich
/// traegt (M-30: "Eine Passage mit unzureichender Abdeckung erhaelt kein
/// Siegerlabel").
pub const GATE_ABDECKUNG: f32 = 0.5;

/// Ueberdeckung zweier Projektfenster, normiert auf das KUERZERE.
///
/// ⚠️ Saettigend gerechnet. Zwei gueltige Fenster an entgegengesetzten
/// `i64`-Raendern duerfen weder paniken noch umbrechen und dadurch als
/// deckungsgleich gelten (M-17). `i64::checked_sub` faengt genau das; ohne
/// ihn waere ein Ueberlauf eine Ueberdeckung von 100 %.
fn ueberdeckung(a: (i64, i64), b: (i64, i64)) -> f64 {
    // ⚠️ `saturating_sub`, nicht `checked_sub(..).unwrap_or(0)`. Der
    // Ausfallwert 0 traf genau den Fall, den er schuetzen soll: zwei
    // IDENTISCHE Fenster ueber den ganzen Zahlenbereich ergaben eine
    // Ueberdeckung von 0 statt 1 und damit `ProjektbereichVerschieden` fuer
    // zwei deckungsgleiche Passagen. Gefunden in Etappe F beim Bau der
    // gleichnamigen Rechnung in `prepost.rs` (§10.7); die Korrektur gehoert
    // hierher, weil dieselbe Zahlenreihe hier dieselbe Wirkung hat.
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

/// Jaccard-Index zweier Quellensets: |Schnitt| / |Vereinigung|.
fn jaccard(a: &[String], b: &[String]) -> f64 {
    if a.is_empty() && b.is_empty() {
        // Zwei leere Sets sind NICHT identisch, sondern ohne Aussage. Sonst
        // waere eine Passage ohne bestaetigte Quellen mit jeder anderen
        // maximal vergleichbar.
        return 0.0;
    }
    let schnitt = a.iter().filter(|x| b.contains(x)).count();
    let vereinigung = a.len() + b.len() - schnitt;
    if vereinigung == 0 {
        return 0.0;
    }
    schnitt as f64 / vereinigung as f64
}

/// Das Urteil ueber zwei Passagen (M-28, M-29, M-30).
///
/// Reihenfolge, und jeder Schritt kann allein fallen:
///
/// 1. **Abdeckung** — ohne genug gemessenes Signal gibt es keinen Vergleich,
///    und zwar auf BEIDEN Seiten (M-30).
/// 2. **Projektbereich** — dieselbe Stelle im Projekt, saettigend gerechnet.
/// 3. **Material** — der Fingerprint. Fehlt er auf einer Seite, ist das
///    KEINE Aehnlichkeit, sondern ein fehlender Beleg.
/// 4. **Quellen** — der Jaccard-Index der aktiven Sets.
/// 5. **Messpunkt** — Samplerate und Messpunktklassen.
///
/// Ein widersprochener Beleg ergibt `Unvergleichbar`; alle fuenf da, aber
/// mindestens einer unter seinem Gate, ergibt `Schwach`. `Stark` verlangt
/// alle fuenf ueber ihren Gates.
pub fn beurteile(a: &Passagenbeleg, b: &Passagenbeleg) -> Vergleichsurteil {
    let mut gruende = Vec::new();

    let zeit = ueberdeckung((a.projekt_start, a.projekt_ende), (b.projekt_start, b.projekt_ende));
    let quellen = jaccard(&a.aktive_quellen, &b.aktive_quellen);
    let material = match (&a.fingerprint, &b.fingerprint) {
        (Some(x), Some(y)) => crate::telemetrie::fingerprint_aehnlichkeit(x, y),
        // Ein fehlender Fingerprint ist kein aehnlicher Fingerprint.
        _ => 0.0,
    };

    // 1. Abdeckung — auf beiden Seiten.
    if a.abdeckung < GATE_ABDECKUNG || b.abdeckung < GATE_ABDECKUNG {
        gruende.push(Herabstufungsgrund::AbdeckungZuGering);
    }
    // 2. Projektbereich. Ueberlappung 0 heisst "vermutlich verschiedene
    //    Passagen" und nicht "0 % aehnlich" (M-17).
    if zeit <= 0.0 {
        gruende.push(Herabstufungsgrund::ProjektbereichVerschieden);
    }
    // 3. Material.
    if a.fingerprint.is_none() || b.fingerprint.is_none() {
        gruende.push(Herabstufungsgrund::MaterialVerschieden);
    }
    // 4. Quellen.
    if quellen <= 0.0 {
        gruende.push(Herabstufungsgrund::QuellenVerschieden);
    }
    // 5. Messpunkt: Samplerate UND Klassenmenge.
    let raten_gleich = (a.samplerate - b.samplerate).abs() < 1e-9 && a.samplerate > 0.0;
    let mut klassen_a = a.messpunktklassen.clone();
    let mut klassen_b = b.messpunktklassen.clone();
    klassen_a.sort();
    klassen_b.sort();
    if !raten_gleich || klassen_a != klassen_b || klassen_a.is_empty() {
        gruende.push(Herabstufungsgrund::MesspunktVerschieden);
    }

    // Ein widersprochener Beleg ⇒ unvergleichbar. Kein Hash hebt das auf.
    if !gruende.is_empty() {
        return Vergleichsurteil {
            klasse: Vergleichbarkeit::Unvergleichbar,
            gruende,
            zeitueberdeckung: zeit,
            quellen_jaccard: quellen,
            material_cosine: material,
            metrics_version: METRICS_VERSION,
        };
    }

    // Alle fuenf da: jetzt entscheiden die drei Gates ueber stark/schwach.
    if zeit < GATE_ZEITUEBERDECKUNG {
        gruende.push(Herabstufungsgrund::ProjektbereichVerschieden);
    }
    if quellen < GATE_QUELLEN_JACCARD {
        gruende.push(Herabstufungsgrund::QuellenVerschieden);
    }
    if material < GATE_MATERIAL_COSINE {
        gruende.push(Herabstufungsgrund::MaterialVerschieden);
    }

    Vergleichsurteil {
        klasse: if gruende.is_empty() {
            Vergleichbarkeit::Stark
        } else {
            Vergleichbarkeit::Schwach
        },
        gruende,
        zeitueberdeckung: zeit,
        quellen_jaccard: quellen,
        material_cosine: material,
        metrics_version: METRICS_VERSION,
    }
}
