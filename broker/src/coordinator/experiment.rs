//! Das Experiment `manual_external` und die Passage, die es trägt
//! (SONDE-013 M-25, M-32, M-40 bis M-51; §43).
//!
//! ## Die Passage entsteht mit dem Versuch, der sie nennt
//!
//! M-25 verlangt ein Passageobjekt im Store, aber SONDE-013 hat keinen
//! eigenen Wire-Weg, eines anzulegen: `session_command` ist ein geschlossener
//! Discriminator, und §3.8 lässt genau einen Fassungsschritt zu, der in
//! Etappe A verbraucht ist. `"passage"` kommt im Schema nur als `passage_id`
//! und `passage_fingerprint` vor — beide in `experiment_begin` und
//! `experiment_referenz`.
//!
//! Also legt der Broker die Passage beim **ersten `experiment_begin` an, das
//! sie nennt**. Das ist auch fachlich stimmig: eine Passage ohne Versuch ist
//! ein Projektintent in Gen (M-25 wörtlich: „`MainProjectState` hält die
//! manuellen Passagen als Projektintent"), kein Evidenzobjekt. Erst der
//! Versuch macht sie zu Evidenz.
//!
//! ## Append-only, und das ist keine Speicherform, sondern die Zusage
//!
//! §43.1 verlangt unveränderliche Referenzen. Der Grund steht in M-41: nur so
//! ist der Vergleich nach Reconnect und UI-Neustart **rekonstruierbar**. Ein
//! überschriebener Baselinewert wäre nicht bloss verloren — er machte jedes
//! spätere Urteil unprüfbar, weil niemand mehr sagen kann, wogegen verglichen
//! wurde. Deshalb gibt es hier kein `&mut Experimentreferenz` und keinen
//! Setter; ein zweiter Versuch erzeugt einen neuen **Kandidaten**.
//!
//! ## Der häufigste Realfall ist der Abbruch
//!
//! M-47 sagt es ausdrücklich: der User misst einen Versuch meistens nie nach.
//! `aborted` ist deshalb Teil des Vertrags und kein Fehlerpfad. Und die
//! Auslöser sind **genau zwei** — expliziter Userabbruch und Verdrängung
//! durch einen Bestandsdeckel. Sitzungsende, Projektwechsel, Reconnect,
//! UI-Neustart und Brokerneustart brechen **nicht** ab: ein offenes
//! Experiment überdauert sie.

use std::collections::BTreeMap;

use crate::telemetrie::Fingerprintwerte;

// ── Die Wireworte, eins zu eins aus dem Schema ───────────────────────────

/// `experiment_begin.execution_mode`. In P4 gibt es genau einen Wert; das
/// Schema hat ihn als `const`, und ein `enum` mit einer Variante hält diese
/// Aussage im Typ statt in einer Prüfung.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Ausfuehrungsart {
    ManualExternal,
}

/// `experiment_begin.reproduzierbarkeit`.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Reproduzierbarkeit {
    /// M-42: „klar als `manuell · nicht automatisch wiederherstellbar`
    /// markiert, erlaubt **kein** Nakama-Revert". Die Klasse ist niedriger
    /// als jede automatische — und das ist eine Aussage über den VERSUCH,
    /// nicht über seine Messung: die bleibt gültig, wenn Passage,
    /// Upstream-Fingerprint und Coverage passen.
    ManuellNichtWiederherstellbar,
}

/// `experiment_abort.grund`. Genau zwei, und mehr darf es nicht geben (M-47).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Abbruchgrund {
    UserAbbruch,
    Verdraengt,
}

/// `experiment_manual_result.hoerurteil`. Ausdrücklich USERDATEN, keine
/// Messung (M-45).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Hoerurteil {
    Baseline,
    Kandidat,
    KeinUnterschied,
    Enthaltung,
}

/// `experiment_manual_result.blindreihenfolge`.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Blindreihenfolge {
    BaselineZuerst,
    KandidatZuerst,
}

/// `experiment_referenz.alignment` — dieselben vier Klassen wie in
/// `prepost::Alignmentklasse`, hier als Wirewert.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Alignmentwert {
    Unclear = 0,
    Probable = 1,
    AudioAligned = 2,
    FeatureAligned = 3,
}

// ── Die Bestandsdeckel (M-48) ───────────────────────────────────────────
//
// „beide sind benannte Konstanten des Stores, kein Literal im Pfad, vom Bau
// begründet gewählt."

/// Offene Experimente je Projektbindung. M-48 verlangt mindestens 2 — mit
/// weniger könnte der User nicht einmal zwei Änderungen nebeneinander offen
/// halten, und genau das ist der normale Arbeitsfluss beim Vergleichen.
/// Acht deckt eine Arbeitssitzung ab, ohne dass eine vergessene Zeile ewig
/// mitreist.
pub const N_PROJEKT: usize = 8;

/// Offene Experimente über alle Projektbindungen. M-48 verlangt mindestens
/// `N_PROJEKT`. Vier Projekte gleichzeitig offen zu haben ist realistisch
/// (Bridge, zweites FL, Vorlage, Bounce), fünf nicht mehr.
pub const N_GLOBAL: usize = 32;

// ── Die Passage (M-25) ──────────────────────────────────────────────────

/// Das Passageobjekt im Store. Genau die sechs Felder aus M-25, und jedes
/// einzeln benannt: eine Passage, die eines davon still weglässt, ist kein
/// Evidenzobjekt mehr, sondern ein Zeitfenster mit Namen.
#[derive(Debug, Clone, PartialEq)]
pub struct Passage {
    pub passage_id: String,
    /// Grenzen in Projektsamples, halboffen `[von, bis)`.
    pub projekt_von: i64,
    pub projekt_bis: i64,
    /// Die Transportepoche, an die sie gebunden ist (§32.4). Eine Passage
    /// überbrückt keine Epochengrenze.
    pub transport_epoch: u64,
    /// Aktives Quellenset als `instance_id`-Liste, eingefroren.
    pub aktive_quellen: Vec<String>,
    /// Je Quelle ihre Messposition, in DERSELBEN Reihenfolge wie
    /// `aktive_quellen` (M-28/M-55).
    ///
    /// 🔑 Nacharbeit 2 (Befund R23): das Schema verlangte `messpunktklassen`
    /// parallel zu `aktive_quellen` schon in Fassung 2, aber `Passage` hatte
    /// keinen Platz dafuer und `passage_aus_wert` las das Feld nicht. Die
    /// Zuordnung Quelle→Messpunkt ging damit bereits beim Wire-Lesen verloren
    /// — und mit ihr Gate 7: ein Messpunktwechsel ist ein Herabstufungsgrund,
    /// und wer die Ausgangsklasse nie gespeichert hat, kann keinen Wechsel
    /// erkennen.
    pub messpunktklassen: Vec<String>,
    /// GEMESSENE Signalabdeckung in [0, 1], nicht die Wanddauer (M-30).
    pub abdeckung: f32,
    /// Das User-Wort, nie interpretiert. `None` = die Passage hat keines.
    pub label: Option<String>,
    pub fingerprint: Fingerprintwerte,
}

// ── Die unveränderlichen Referenzen (M-40, M-41, M-43) ──────────────────

/// `experiment_referenz` — die Baseline beziehungsweise ein Kandidat.
///
/// Es gibt bewusst keinen Setter und kein `&mut` darauf. Eine Referenz wird
/// angelegt und danach nur noch gelesen.
#[derive(Debug, Clone, PartialEq)]
pub struct Experimentreferenz {
    pub passage_fingerprint: Fingerprintwerte,
    pub upstream_fingerprint: Fingerprintwerte,
    pub aktive_quellen: Vec<String>,
    pub messpunktklassen: Vec<String>,
    /// Der VORAB gemessene und eingefrorene Match-Gain (M-43). Er ist Teil
    /// der unveränderlichen Referenz und keine laufende Größe.
    pub match_gain_db: f64,
    /// 🔑 Nacharbeit 3 (Befund C5, M-07): wie viele nicht-endliche Samples der
    /// Vergleichspegel dieser Passage gesehen hat.
    ///
    /// `None` heisst „der Sender kennt das Feld nicht" — ausdruecklich NICHT
    /// `Some(0)`. Der Unterschied ist die ganze Zusage: 0 ist ein Messwert,
    /// „unbekannt" ist keiner, und beides gleichzusetzen waere genau die
    /// stille Beschoenigung, gegen die M-07 steht.
    pub nicht_endliche_samples: Option<u64>,
    pub alignment: Alignmentwert,
}

/// Ein Kandidat: eine spätere Erfassung derselben Passage (M-41).
#[derive(Debug, Clone, PartialEq)]
pub struct Kandidat {
    /// Fortlaufend ab 1. Die Baseline ist nicht Kandidat 0 — sie ist etwas
    /// anderes, und eine gemeinsame Nummerierung lüde dazu ein, sie zu
    /// überschreiben.
    pub nummer: u32,
    pub referenz: Experimentreferenz,
    /// Die Evidenz-Ankunftsreihenfolge, bei der dieser Kandidat erfasst wurde
    /// (M-49, Befund R17). Alles, was DANACH ankommt, ist Resultatevidenz;
    /// alles davor gehoert noch zum alten Stand.
    pub evidenzfolge: u64,
}

/// Die VIER Achsen, die der Broker vor dem Terminalereignis rechnet (M-45).
///
/// 🔑 Nacharbeit 1 (Befund B20): `ergebnis()` konnte vorher unmittelbar nach
/// Begin und Reihenfolgebindung terminieren — ohne einen einzigen Kandidaten,
/// ohne Bootstrap, ohne Comparability, ohne Guardrails. `Terminal::Ergebnis`
/// trug nur die Userfelder. §43.2 und M-49 verlangen dagegen ausdruecklich
/// den BROKERSEITIGEN Rechenweg: Zielmetrik, Guardrail-Deltas und
/// Effektstabilitaet entstehen aus Baseline- und Resultatevidenz im Store.
///
/// Jede Achse traegt ihre eigene Gueltigkeit. Eine Achse, die nicht gerechnet
/// werden konnte, ist `None` — nie eine 0, die wie ein Messergebnis aussieht.
#[derive(Debug, Clone, PartialEq, Default)]
pub struct Achsenrechnung {
    /// 1. Zielmetrik: das Konfidenzintervall des mittleren Banddeltas aus dem
    ///    Block-Bootstrap, und wie viele Baender nach FDR signifikant bleiben.
    pub intervall: Option<(f64, f64)>,
    pub signifikante_baender: usize,
    pub gescannte_baender: usize,
    /// 2. Comparability: das Urteil ueber Baseline- und Resultatpassage.
    pub vergleichbarkeit: Option<String>,
    pub vergleichbarkeit_gruende: Vec<String>,
    /// 3. Guardrails: die Deltas, die NICHT das Ziel sind und trotzdem
    ///    ueberwacht werden. M-45 zaehlt sie ausdruecklich auf: Loudness,
    ///    Peak, Transient, Breite und geschuetzte Bereiche — zusaetzlich zu
    ///    Abdeckung und Konfidenzklasse (Befund R19). Jede ist `Option`, weil
    ///    „nicht gemessen" etwas anderes ist als „unveraendert".
    pub guardrail_abdeckung_delta: Option<f64>,
    pub guardrail_klasse_gefallen: bool,
    pub guardrail_loudness_db: Option<f64>,
    pub guardrail_peak_db: Option<f64>,
    pub guardrail_transient: Option<f64>,
    pub guardrail_breite_db: Option<f64>,
    pub guardrail_geschuetzt_db: Option<f64>,
    /// Befund B11: die Namen der Guardrails, die diese Evidenz nicht traegt.
    pub guardrail_nicht_gemessen: Vec<String>,
    /// 4. Effektstabilitaet: streut das Delta ueber die Teilfenster?
    ///    `None` heisst „nicht beurteilbar", nicht „stabil".
    pub effekt_stabil: Option<bool>,
}

/// Die Messung, gegen die ein Ergebnis abgeschlossen wird (M-45/M-49).
///
/// Sie kommt aus dem Evidenzbestand des Coordinators — Baseline- und
/// Resultatfenster derselben Passage. Ohne sie gibt es KEIN Ergebnis: ein
/// Terminal ohne Resultatmessung waere ein Urteil ohne Gegenprobe.
#[derive(Debug, Clone, PartialEq, Default)]
pub struct Resultatmessung {
    /// Banddeltas Resultat minus Baseline in dB, je Band mit Praesenzbit.
    pub band_delta_db: Vec<f64>,
    pub band_gueltig: Vec<bool>,
    /// 🔑 Nacharbeit 2 (Befund R20): die ZEITREIHE, `fenster_delta_db[fenster][band]`.
    ///
    /// Der Block-Bootstrap zieht laut eigener Definition Bloecke benachbarter,
    /// zeitlich korrelierter Analysefenster. Die Runde 1 uebergab ihm statt
    /// dessen EINEN ueber alle Fenster gemittelten Wert je Band — er zog also
    /// Bloecke von Baendern, und die zufaellige Bandreihenfolge konnte ein
    /// Konfidenzintervall erzeugen. `NaN` heisst „dieses Band hat in diesem
    /// Fenster nichts gesagt"; der Bootstrap laesst es fallen.
    pub fenster_delta_db: Vec<Vec<f64>>,
    /// Die zwei Haelften des Deltas — daraus entsteht die Effektstabilitaet.
    pub erste_haelfte: Vec<f64>,
    pub zweite_haelfte: Vec<f64>,
    // ── Die Guardrails aus M-45 (Befund R19) ────────────────────────────
    //
    // 🔑 Die Runde 1 modellierte ausschliesslich Abdeckung und
    // Konfidenzklasse. Verschlechterungen von Loudness, Peak, Transient,
    // Breite und geschuetzten Bereichen konnten weder eingelesen noch
    // gespeichert werden und erschienen bei unveraenderter Coverage als
    // STABIL. Jede ist `Option`: „nicht gemessen" ist etwas anderes als
    // „unveraendert".
    /// Mittleres Bandniveau, Resultat minus Baseline, in dB.
    pub guardrail_loudness_db: Option<f64>,
    /// Hoechstes P95 ueber alle Baender, Resultat minus Baseline, in dB.
    pub guardrail_peak_db: Option<f64>,
    /// Onsetstaerke, Resultat minus Baseline — die zweite, unabhaengige Spur.
    pub guardrail_transient: Option<f64>,
    /// Seitenanteil in dB, Resultat minus Baseline.
    pub guardrail_breite_db: Option<f64>,
    /// Groesste Bewegung AUSSERHALB des staerksten Zielbandes, in dB.
    pub guardrail_geschuetzt_db: Option<f64>,
    /// 🔑 Nacharbeit 3 (Befund B11, M-45/M-07): welche Guardrails NICHT
    /// gemessen werden konnten — mit Grund.
    ///
    /// M-45 nennt Loudness und True Peak ausdruecklich. Die Runde 2 lieferte
    /// unter diesen Namen Ersatzgroessen: das Mittel spektraler P50-dB-Werte
    /// und das hoechste gemittelte Band-P95. Weder ist integrierte Lautheit
    /// noch True Peak; eine spektrale Umverteilung oder ein
    /// Intersample-Ueberschwinger blieb damit unentdeckt, waehrend die Zahl
    /// unter dem echten Namen reiste. `evidence_snapshot` traegt beide
    /// Groessen heute nicht — also steht hier ihr Grund, und die Achse faellt
    /// auf „nicht beurteilbar". Eine Ersatzrechnung unter fremdem Namen waere
    /// schlimmer als eine fehlende Zahl.
    pub guardrail_nicht_gemessen: Vec<String>,
    /// Abdeckung beider Fenster.
    pub abdeckung_baseline: f64,
    pub abdeckung_resultat: f64,
    /// Konfidenzklassen beider Fenster, wie sie auf der Leitung standen.
    pub klasse_baseline: String,
    pub klasse_resultat: String,
    /// Das Comparability-Urteil, falls der Broker eines rechnen konnte.
    pub vergleichbarkeit: Option<String>,
    pub vergleichbarkeit_gruende: Vec<String>,
    /// Die Evidence-IDs, aus denen die Messung entstand (M-51).
    pub baseline_evidence_ids: Vec<String>,
    pub resultat_evidence_ids: Vec<String>,
}

impl Resultatmessung {
    /// Traegt die Messung ueberhaupt ein Resultat?
    ///
    /// Ohne gueltiges Band gibt es nichts zu rechnen, und ein Abschluss darauf
    /// waere genau der Fall, den B20 gefunden hat.
    pub fn hat_resultat(&self) -> bool {
        self.band_gueltig.iter().any(|g| *g)
            && !self.resultat_evidence_ids.is_empty()
            && !self.baseline_evidence_ids.is_empty()
    }

    /// Rechnet die vier Achsen. Reine Funktion, kein Zustand.
    pub fn achsen(&self, klassenordnung: &[&str]) -> Achsenrechnung {
        let endliche: Vec<f64> = self
            .band_delta_db
            .iter()
            .zip(self.band_gueltig.iter())
            .filter(|(_, g)| **g)
            .map(|(v, _)| *v)
            .collect();
        // 1. Zielmetrik — aus der ZEITREIHE, nicht aus der Bandachse.
        //
        // 🔑 Nacharbeit 2 (Befund R20): die Runde 1 gab dem Block-Bootstrap
        // EINEN ueber alle Fenster gemittelten Wert je Band. Er zog damit
        // Bloecke von BAENDERN, obwohl seine Definition zeitlich benachbarte
        // Analysefenster verlangt — und die zufaellige Bandreihenfolge konnte
        // ein Intervall erzeugen. Die p-Werte waren zusaetzlich als
        // `exp(-|delta|)` ERFUNDEN, ohne Streuung und ohne Stichprobenzahl:
        // die Effektgroesse allein machte ein Band „signifikant".
        //
        // Jetzt traegt jedes Resultatfenster seine eigene Zeile. Die
        // Zielmetrik bootstrappt die Reihe der FENSTERMITTEL, und jedes Band
        // bekommt seinen p-Wert aus SEINER eigenen Bootstrapverteilung.
        let fenstermittel: Vec<f64> = self
            .fenster_delta_db
            .iter()
            .filter_map(|zeile| {
                let e: Vec<f64> = zeile.iter().copied().filter(|x| x.is_finite()).collect();
                (!e.is_empty()).then(|| e.iter().sum::<f64>() / e.len() as f64)
            })
            .collect();
        let intervall = block_bootstrap(
            &fenstermittel,
            BOOTSTRAP_BLOCK,
            BOOTSTRAP_ZIEHUNGEN,
            0.05,
            42,
        );
        // Je Band die eigene Zeitreihe — und daraus der p-Wert.
        let mut p_werte = Vec::new();
        let mut baender_mit_reihe = 0usize;
        for band in 0..self.band_gueltig.len() {
            if !self.band_gueltig[band] {
                continue;
            }
            let reihe: Vec<f64> = self
                .fenster_delta_db
                .iter()
                .filter_map(|z| z.get(band).copied())
                .filter(|x| x.is_finite())
                .collect();
            if reihe.is_empty() {
                continue;
            }
            baender_mit_reihe += 1;
            p_werte.push(bootstrap_p(
                &reihe,
                BOOTSTRAP_BLOCK,
                BOOTSTRAP_ZIEHUNGEN,
                42 + band as u64,
            ));
        }
        let signifikant = fdr_signifikant(&p_werte, 0.05);
        // 3. Guardrails.
        let guardrail_abdeckung_delta = if self.abdeckung_baseline.is_finite()
            && self.abdeckung_resultat.is_finite()
        {
            Some(self.abdeckung_resultat - self.abdeckung_baseline)
        } else {
            None
        };
        let rang = |k: &str| klassenordnung.iter().position(|x| *x == k);
        let guardrail_klasse_gefallen = match (
            rang(&self.klasse_baseline),
            rang(&self.klasse_resultat),
        ) {
            (Some(a), Some(b)) => b < a,
            // Eine unbekannte Klasse ist kein „nicht gefallen": sie ist
            // unbekannt, und fail-closed heisst hier gefallen.
            _ => true,
        };
        // 4. Effektstabilitaet.
        let mittel = |v: &[f64]| {
            let e: Vec<f64> = v.iter().copied().filter(|x| x.is_finite()).collect();
            if e.is_empty() {
                None
            } else {
                Some(e.iter().sum::<f64>() / e.len() as f64)
            }
        };
        let effekt_stabil = match (mittel(&self.erste_haelfte), mittel(&self.zweite_haelfte)) {
            (Some(a), Some(b)) => Some((a - b).abs() <= 1.0),
            _ => None,
        };
        Achsenrechnung {
            intervall,
            signifikante_baender: signifikant.iter().filter(|s| **s).count(),
            gescannte_baender: baender_mit_reihe.max(endliche.len()),
            vergleichbarkeit: self.vergleichbarkeit.clone(),
            vergleichbarkeit_gruende: self.vergleichbarkeit_gruende.clone(),
            guardrail_abdeckung_delta,
            guardrail_klasse_gefallen,
            guardrail_loudness_db: self.guardrail_loudness_db,
            guardrail_peak_db: self.guardrail_peak_db,
            guardrail_transient: self.guardrail_transient,
            guardrail_breite_db: self.guardrail_breite_db,
            guardrail_geschuetzt_db: self.guardrail_geschuetzt_db,
            guardrail_nicht_gemessen: self.guardrail_nicht_gemessen.clone(),
            effekt_stabil,
        }
    }
}

impl Achsenrechnung {
    /// Die QUALITATIVEN vier Achsen aus M-45/M-46 — die Form, aus der
    /// `urteile()` die fuenf zulaessigen Aussagen bildet.
    ///
    /// Die Umrechnung steht hier und nicht beim Aufrufer: es gibt genau EINEN
    /// Weg von den gerechneten Zahlen zur Achsenaussage, und zwei waeren zwei
    /// Wahrheiten ueber dasselbe Ergebnis. Jede Achse faellt fuer sich auf
    /// `NichtBeurteilbar`, wenn ihre Grundlage fehlt — nie auf `Unveraendert`:
    /// „ich weiss es nicht" ist nicht „es hat sich nichts geaendert".
    pub fn befunde(&self, hoerurteil: Option<Hoerurteil>) -> Achsen {
        let zielmetrik = match self.intervall {
            // Ein Intervall, das die Null NICHT enthaelt, ist eine Aenderung.
            // Seine Richtung ist das Vorzeichen; die Bedeutung „besser" gehoert
            // dem User, nicht der Zahl — deshalb steht hier `Verbessert` fuer
            // ein Delta nach oben und nichts darueber hinaus.
            Some((unten, oben)) if self.signifikante_baender > 0 && unten > 0.0 => {
                Achsenbefund::Verbessert
            }
            Some((unten, oben)) if self.signifikante_baender > 0 && oben < 0.0 => {
                let _ = unten;
                Achsenbefund::Verschlechtert
            }
            Some(_) => Achsenbefund::Unveraendert,
            None => Achsenbefund::NichtBeurteilbar,
        };
        // 🔑 Befund R19: ALLE Guardrails aus M-45, nicht nur Coverage und
        // Klasse. Die Achse faellt, sobald EINE messbare Groesse ihre Schwelle
        // reisst; sie ist nur dann `NichtBeurteilbar`, wenn KEINE messbar war.
        // Ein „unveraendert", das auf einer nie gemessenen Groesse beruht,
        // waere genau die stille Beschoenigung, die M-07 ausschliesst.
        let guardrails = if self.guardrail_klasse_gefallen {
            Achsenbefund::Verschlechtert
        } else {
            let kandidaten: [(Option<f64>, f64); 6] = [
                (self.guardrail_abdeckung_delta, -GUARDRAIL_ABDECKUNG),
                // Lauter, spitzer, breiter oder unruhiger: jede Richtung, in
                // der eine NICHT adressierte Groesse davonlaeuft, zaehlt.
                (self.guardrail_loudness_db.map(f64::abs), GUARDRAIL_LOUDNESS_DB),
                (self.guardrail_peak_db.map(f64::abs), GUARDRAIL_PEAK_DB),
                (self.guardrail_transient.map(f64::abs), GUARDRAIL_TRANSIENT),
                (self.guardrail_breite_db.map(f64::abs), GUARDRAIL_BREITE_DB),
                (self.guardrail_geschuetzt_db, GUARDRAIL_GESCHUETZT_DB),
            ];
            let mut messbar = false;
            let mut gerissen = false;
            for (wert, schwelle) in kandidaten {
                let Some(w) = wert.filter(|x| x.is_finite()) else {
                    continue;
                };
                messbar = true;
                // Die Abdeckung ist die einzige mit VORZEICHEN: weniger
                // Abdeckung ist schlechter, mehr ist nie schlechter.
                if schwelle < 0.0 {
                    if w < schwelle {
                        gerissen = true;
                    }
                } else if w > schwelle {
                    gerissen = true;
                }
            }
            match (messbar, gerissen) {
                (_, true) => Achsenbefund::Verschlechtert,
                // 🔑 Nacharbeit 3 (Befund B11): fehlt eine der von M-45
                // VERLANGTEN Groessen, ist die Achse nicht beurteilbar — auch
                // wenn andere messbar waren und stillhielten. „Die zwei, auf
                // die es ankommt, habe ich nie gesehen" ist kein
                // „unveraendert".
                (true, false) if !self.guardrail_nicht_gemessen.is_empty() => {
                    Achsenbefund::NichtBeurteilbar
                }
                (true, false) => Achsenbefund::Unveraendert,
                (false, false) => Achsenbefund::NichtBeurteilbar,
            }
        };
        let effektstabilitaet = match self.effekt_stabil {
            Some(true) => Achsenbefund::Unveraendert,
            Some(false) => Achsenbefund::Verschlechtert,
            None => Achsenbefund::NichtBeurteilbar,
        };
        Achsen {
            zielmetrik,
            guardrails,
            effektstabilitaet,
            hoerurteil,
        }
    }
}

// ── Die Guardrail-Schwellen (M-45, Befund R19) ──────────────────────────
//
// Startwerte, am Korpus zu kalibrieren — dieselbe Klasse wie die
// Comparability-Gates. Sie stehen als benannte Konstanten hier und nicht als
// Literale im Pfad: eine Kalibrierung hebt sonst die eine Stelle und laesst
// den stillen Zwilling entscheiden.

/// Abdeckung darf um hoechstens so viel FALLEN. Mehr Abdeckung ist nie
/// schlechter, deshalb wirkt diese Schwelle nur nach unten.
pub const GUARDRAIL_ABDECKUNG: f64 = 0.1;
/// Mittleres Bandniveau. Ein Zehntel Dezibel hoert niemand; ein ganzes schon.
pub const GUARDRAIL_LOUDNESS_DB: f64 = 1.0;
/// Spitzenwert. Enger als die Loudness: eine angehobene Spitze bei gleichem
/// Median ist genau der Fall, den ein Guardrail fangen soll.
pub const GUARDRAIL_PEAK_DB: f64 = 0.5;
/// Onsetstaerke. Die Groesse ist eine Summe von MAD-Vielfachen, keine dB.
pub const GUARDRAIL_TRANSIENT: f64 = 2.0;
/// Seitenanteil in dB — die Stereobreite.
pub const GUARDRAIL_BREITE_DB: f64 = 1.5;
/// Bewegung ausserhalb des Zielbandes. Sie ist die eigentliche Zusage von
/// „geschuetzte Bereiche": das Ziel darf sich bewegen, der Rest nicht.
pub const GUARDRAIL_GESCHUETZT_DB: f64 = 1.0;

/// Blocklaenge des Bootstraps in Analysefenstern.
pub const BOOTSTRAP_BLOCK: usize = 4;
/// Ziehungen je Bootstrap.
pub const BOOTSTRAP_ZIEHUNGEN: usize = 400;

/// Die Ordnung der Konfidenzklassen, schwaechste zuerst (§34.3).
pub const KLASSENORDNUNG: [&str; 4] = ["unbrauchbar", "schwach", "mittel", "stark"];

/// Wie ein Experiment endet. `None` heisst offen (M-48).
#[derive(Debug, Clone, PartialEq)]
pub enum Terminal {
    /// `experiment_manual_result`. Trägt NUR, was das Schema erlaubt:
    /// Hörurteil, aufgedeckte Blindreihenfolge, Notiz, Werkzeugname. Keine
    /// State-Hashes, keine Candidate-Revision, kein Match-Gain (M-49).
    Ergebnis {
        hoerurteil: Hoerurteil,
        blindreihenfolge: Blindreihenfolge,
        notiz: Option<String>,
        werkzeug: Option<String>,
        /// Die vom BROKER gerechneten vier Achsen (M-45/M-49). Sie stehen im
        /// Terminalereignis, nicht daneben: ein Ergebnis ohne seine Deltas
        /// waere ein Urteil, dessen Grundlage niemand mehr nachvollziehen
        /// kann. `Achsenrechnung::befunde` macht daraus die qualitative Form.
        achsen: Achsenrechnung,
    },
    Abgebrochen {
        grund: Abbruchgrund,
    },
}

/// Ein Experiment im Store.
#[derive(Debug, Clone, PartialEq)]
pub struct Experiment {
    pub experiment_id: String,
    pub projektbindung: String,
    pub passage_id: String,
    pub art: Ausfuehrungsart,
    pub reproduzierbarkeit: Reproduzierbarkeit,
    /// Die BASELINE, verriegelt beim `experiment_begin` (M-40).
    pub baseline: Experimentreferenz,
    pub kandidaten: Vec<Kandidat>,
    pub terminal: Option<Terminal>,
    /// Monoton steigende Anlegereihenfolge — sie entscheidet, wer beim
    /// Bestandsdeckel verdrängt wird (M-48: „ältestes offenes Experiment").
    pub folge: u64,
    /// Die Blindreihenfolge wird VOR dem Urteil gebunden (M-44). Sie steht
    /// hier und nicht in `Terminal`, weil sie schon existiert, bevor ein
    /// Urteil vorliegt — und genau das ist die Zusage.
    gebundene_reihenfolge: Option<Blindreihenfolge>,
    /// Die Evidenz-Ankunftsreihenfolge beim `experiment_begin` (M-49, R17).
    /// Alles, was DAVOR ankam, ist Baselineevidenz.
    pub begin_evidenzfolge: u64,
    /// Die Evidence-IDs, aus denen Baseline und Resultat entstanden (M-51).
    ///
    /// 🔑 Nacharbeit 1 (Befund B21): weder `Experiment` noch `Passage`,
    /// `Experimentreferenz` oder `Export` trugen sie. Der Export konnte sie
    /// deshalb unabhaengig vom Aufrufer gar nicht liefern — und ein Export
    /// ohne die Belege, auf denen er beruht, ist kein Export, sondern ein
    /// Verweis auf einen Store, den der Empfaenger nicht hat.
    pub baseline_evidence_ids: Vec<String>,
    pub resultat_evidence_ids: Vec<String>,
}

impl Experiment {
    pub fn offen(&self) -> bool {
        self.terminal.is_none()
    }

    /// Die gebundene Blindreihenfolge — aber nur NACH dem Urteil (M-44).
    ///
    /// Das ist die Kante, die verhindert, dass die Reihenfolge nachträglich
    /// zum Urteil passend erzählt wird. Ein Leser, der sie vorher bekäme,
    /// könnte sie der UI zeigen, und der Blindvergleich wäre keiner mehr.
    /// Der Riegel liegt deshalb im Typ und nicht in der Disziplin des
    /// Aufrufers: das Feld ist privat, und dies ist der einzige Weg heraus.
    pub fn aufgedeckte_reihenfolge(&self) -> Option<Blindreihenfolge> {
        match &self.terminal {
            Some(Terminal::Ergebnis { .. }) => self.gebundene_reihenfolge,
            _ => None,
        }
    }

    /// Ob überhaupt eine Reihenfolge gebunden ist — ohne sie zu verraten.
    /// Die UI darf wissen, dass ein Blindvergleich läuft; sie darf nur nicht
    /// wissen, welche Seite welche ist.
    pub fn reihenfolge_gebunden(&self) -> bool {
        self.gebundene_reihenfolge.is_some()
    }

    /// Die gebundene Reihenfolge — AUSSCHLIESSLICH zum Abgleich mit einer
    /// gemeldeten (M-44).
    ///
    /// ⚠️ Das ist ausdruecklich KEIN zweiter Weg zu `aufgedeckte_reihenfolge`.
    /// Der Unterschied liegt im Verwendungszweck, und er ist tragend: hier
    /// wird geprueft, ob eine von aussen GEMELDETE Reihenfolge zur gebundenen
    /// passt — der Aufrufer bringt seine Antwort schon mit und erfaehrt
    /// nichts, was er nicht schon wusste. `aufgedeckte_reihenfolge` gibt sie
    /// dagegen HERAUS, und genau das darf erst nach dem Urteil geschehen.
    ///
    /// Ein Widerspruch ist der Fall, den M-44 ausschliesst: „damit sich die
    /// Reihenfolge nicht nachtraeglich zum Urteil passend erzaehlen laesst."
    pub fn gebundene_reihenfolge_fuer_pruefung(&self) -> Option<Blindreihenfolge> {
        self.gebundene_reihenfolge
    }
}

/// Was beim Anlegen schiefgehen kann.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Anlegefehler {
    /// Dieselbe `experiment_id` gibt es schon. Append-only heisst auch: kein
    /// Überschreiben durch Wiederholung.
    IdVergeben,
    /// Die `experiment_id` ist keine hex32.
    IdUngueltig,
    /// Die Passage trägt zu wenig gemessenes Signal (M-30).
    AbdeckungZuGering,
}

/// Was beim Abschliessen schiefgehen kann.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Abschlussfehler {
    Unbekannt,
    /// Schon terminal. Append-only: ein zweites Terminalereignis würde das
    /// erste umdeuten.
    SchonTerminal,
    /// Ein Klangurteil ohne vorherigen Lautheitsabgleich ist unzulässig
    /// (M-43, §15).
    OhneLautheitsabgleich,
    /// Ein Ergebnis ohne gebundene Blindreihenfolge (M-44).
    ReihenfolgeNichtGebunden,
    /// Ein Abschluss ohne Resultatmessung (M-45, Befund B20). Ein Urteil ohne
    /// Gegenprobe ist kein Ergebnis, sondern eine Meinung.
    OhneResultatmessung,
    /// Ein Abschluss ohne einen einzigen nach dem Begin erfassten KANDIDATEN
    /// (M-41, Befund R16).
    ///
    /// 🔑 Nacharbeit 2: die Abschlusswache der Runde 1 verlangte nur
    /// irgendeine nichtleere `Resultatmessung`. Ein Experiment konnte damit
    /// direkt nach Begin und Reihenfolgebindung mit synthetischer Messung
    /// terminieren — und der Test dazu FUHR genau diesen Ablauf und ERWARTETE
    /// Erfolg. Ohne Kandidat gibt es nichts, wogegen die Baseline stuende.
    OhneKandidat,
}

// ── Der Store ───────────────────────────────────────────────────────────

/// Ein append-only Ereignis im Log. Der Store hält den gerechneten Zustand
/// UND die Ereigniskette; die Kette ist die Wahrheit, der Zustand ihre
/// Zusammenfassung (§33.5).
#[derive(Debug, Clone, PartialEq)]
pub enum Ereignis {
    /// 🔑 Nacharbeit 1 (Befund B19): jedes Ereignis traegt die
    /// UNVERAENDERLICHEN Referenzen aus §43.1 und nicht nur IDs.
    ///
    /// Vorher stand im Log ausschliesslich, DASS etwas geschah. Selbst ein
    /// persistiertes Log haette den Zustand damit nicht rekonstruieren
    /// koennen: nach einem Brokerneustart fehlten Passage, Baseline,
    /// Kandidat, Match-Gain und Blindreihenfolge. M-47 und M-50 verlangen
    /// aber ausdruecklich, dass ein offener Versuch den Neustart ueberdauert
    /// und danach REKONSTRUIERBAR ist — und rekonstruierbar heisst: aus dem
    /// Log allein.
    PassageAngelegt {
        passage_id: String,
        passage: Box<Passage>,
    },
    Begonnen {
        experiment_id: String,
        passage_id: String,
        projektbindung: String,
        baseline: Box<Experimentreferenz>,
    },
    KandidatAngelegt {
        experiment_id: String,
        nummer: u32,
        referenz: Box<Experimentreferenz>,
    },
    ReihenfolgeGebunden {
        experiment_id: String,
        reihenfolge: Blindreihenfolge,
    },
    Ergebnis {
        experiment_id: String,
        hoerurteil: Hoerurteil,
        blindreihenfolge: Blindreihenfolge,
        achsen: Box<Achsenrechnung>,
        baseline_evidence_ids: Vec<String>,
        resultat_evidence_ids: Vec<String>,
    },
    Abgebrochen { experiment_id: String, grund: Abbruchgrund },
}

/// Der Rueckweg eines einzelnen Experimentbefehls (Befund B3).
///
/// Er wird VOR der Wirkung gebaut und nur benutzt, wenn der gemeinsame Append
/// von Befehl und Wirkung scheitert. Er nennt ausdruecklich, was dieser
/// Befehl angefasst hat — nicht den Zustand der Welt.
#[derive(Debug, Clone, Default, PartialEq)]
pub struct Ruecknahme {
    /// Ein von diesem Befehl NEU angelegter Versuch. Er wird entfernt.
    pub angelegt: Option<String>,
    /// Versuche, deren Terminal dieser Befehl gesetzt hat (Verdraengung).
    pub terminale: Vec<String>,
    /// Der Zustand des adressierten Versuchs VOR dem Befehl.
    pub vorher: Option<(String, Experiment)>,
    /// Eine von diesem Befehl NEU angelegte Passage.
    pub passage: Option<String>,
    pub log_laenge: usize,
    pub naechste_folge: u64,
}


/// Der Experimentteil des Stores.
///
/// Die Wirkung eines Experimentbefehls wird VORLAEUFIG angewandt, damit ihre
/// Ereignisse aus dem entstandenen Zustand entstehen und mit dem Befehl in
/// EINE Transaktion gehen (Befund R08). Scheitert der Append, wird sie
/// zurueckgenommen — ein Speicher, der dem Log voraus ist, waere genau die
/// zweite Wahrheit, die §33.5 ausschliesst.
///
/// 🔑 Nacharbeit 3 (Befund B3): der Rueckweg laeuft ueber `Ruecknahme` und
/// fasst NUR an, was der Befehl geaendert hat. Die Runde 2 klonte den ganzen
/// Store und ersetzte ihn im Fehlerfall; damit loeschte ein scheiternder
/// Befehl die bereits persistierte Wirkung PARALLELER Befehle aus dem
/// Speicher. `Clone` bleibt fuer Tests und Sichten, ist aber kein Rueckfall
/// mehr.
#[derive(Debug, Default, Clone)]
pub struct Experimentstore {
    passagen: BTreeMap<String, Passage>,
    experimente: BTreeMap<String, Experiment>,
    log: Vec<Ereignis>,
    naechste_folge: u64,
}

fn ist_hex32(s: &str) -> bool {
    s.len() == 32 && s.bytes().all(|b| b.is_ascii_hexdigit())
}

impl Experiment {
    /// Baut ein Experiment aus einer GESPEICHERTEN Zeile (M-50, Befund R12).
    ///
    /// Der einzige Weg an `gebundene_reihenfolge` vorbei am Riegel von M-44 —
    /// und er ist zulaessig, weil hier nichts gebunden, sondern etwas bereits
    /// Gebundenes WIEDERHERGESTELLT wird. Ohne ihn koennte ein Neustart einen
    /// offenen Versuch nur ohne seine Blindreihenfolge zurueckbringen, und der
    /// naechste Abschluss faende sie nicht gebunden vor.
    #[allow(clippy::too_many_arguments)]
    pub fn aus_store(
        experiment_id: String,
        projektbindung: String,
        passage_id: String,
        baseline: Experimentreferenz,
        kandidaten: Vec<Kandidat>,
        terminal: Option<Terminal>,
        folge: u64,
        gebundene_reihenfolge: Option<Blindreihenfolge>,
        begin_evidenzfolge: u64,
        baseline_evidence_ids: Vec<String>,
        resultat_evidence_ids: Vec<String>,
    ) -> Self {
        Self {
            experiment_id,
            projektbindung,
            passage_id,
            art: Ausfuehrungsart::ManualExternal,
            reproduzierbarkeit: Reproduzierbarkeit::ManuellNichtWiederherstellbar,
            baseline,
            kandidaten,
            terminal,
            folge,
            gebundene_reihenfolge,
            begin_evidenzfolge,
            baseline_evidence_ids,
            resultat_evidence_ids,
        }
    }
}

impl Experimentstore {
    pub fn neu() -> Self {
        Self::default()
    }

    /// Der Store aus GESPEICHERTEN Zeilen (M-47/M-50, Befund R12).
    ///
    /// 🔑 Nacharbeit 2: `Coordinator::mit_store` restaurierte ausschliesslich
    /// die Konfliktriegel; Experimentstore und Passagen starteten immer leer.
    /// Nach Drop und Neuerzeugung lieferte `experiment_sicht(id)` deshalb
    /// `None`, obwohl die SQLite-Zeile existierte — waehrend M-47 ausdruecklich
    /// zusagt, dass ein Brokerneustart einen offenen Versuch NICHT abbricht.
    ///
    /// Das Ereignislog wird NICHT rekonstruiert: es ist die Kette dieses
    /// Laufs. Die haltbare Kette liegt im `event_log` und im Index
    /// `experiment_events`; sie hier zu erfinden waere eine zweite, aermere
    /// Kopie derselben Wahrheit.
    pub fn wiederherstellen(passagen: Vec<Passage>, experimente: Vec<Experiment>) -> Self {
        let mut store = Self::default();
        for p in passagen {
            store.passagen.insert(p.passage_id.clone(), p);
        }
        for e in experimente {
            store.naechste_folge = store.naechste_folge.max(e.folge.saturating_add(1));
            store.experimente.insert(e.experiment_id.clone(), e);
        }
        store
    }

    pub fn log(&self) -> &[Ereignis] {
        &self.log
    }

    /// Setzt die aus dem Store restaurierte Transitionshistorie (Befund B5).
    ///
    /// Sie wird NICHT erfunden: jede Zeile kommt aus `experiment_events` und
    /// dem daran haengenden Domaenenereignis. Ohne sie liefert
    /// `exportiere` nach einem Neustart eine leere Kette, obwohl M-51
    /// „vollstaendig exportiert" zusagt.
    pub fn log_setzen(&mut self, log: Vec<Ereignis>) {
        self.log = log;
    }

    pub fn passage(&self, id: &str) -> Option<&Passage> {
        self.passagen.get(id)
    }

    pub fn experiment(&self, id: &str) -> Option<&Experiment> {
        self.experimente.get(id)
    }

    /// Alle Versuche, offen wie terminal (Befund B6): auch ein abgeschlossener
    /// haelt Grenzen, hinter die der Evidenzzaehler zurueckfallen koennte.
    pub fn alle(&self) -> impl Iterator<Item = &Experiment> {
        self.experimente.values()
    }

    pub fn offene(&self) -> impl Iterator<Item = &Experiment> {
        self.experimente.values().filter(|e| e.offen())
    }

    pub fn offene_im_projekt<'a>(&'a self, bindung: &'a str) -> impl Iterator<Item = &'a Experiment> {
        self.offene().filter(move |e| e.projektbindung == bindung)
    }

    /// ALLE Versuche einer Projektbindung, offene wie abgeschlossene.
    ///
    /// 🔑 Nacharbeit 2 (Befund R14): der Sessionsnapshot traegt sie zu Gen.
    /// Ein abgeschlossener gehoert ausdruecklich dazu — sein Resultat ist der
    /// Grund, warum es den Versuch ueberhaupt gab.
    pub fn alle_im_projekt<'a>(&'a self, bindung: &'a str) -> impl Iterator<Item = &'a Experiment> {
        self.experimente
            .values()
            .filter(move |e| e.projektbindung == bindung)
    }

    /// `experiment_begin` (M-40, und damit auch M-25).
    ///
    /// Die Reihenfolge ist die aus M-40 und jeder Schritt hat seinen Grund:
    ///
    /// 1. **Passage bestimmen** — sie entsteht hier, falls sie neu ist;
    /// 2. **Baseline verriegeln** — die Referenz wird kopiert, nicht geliehen;
    /// 3. **Bestandsdeckel prüfen** (M-48) und gegebenenfalls das älteste
    ///    offene Experiment des betroffenen Bereichs mit `verdraengt`
    ///    schliessen;
    /// 4. **anlegen** und das Ereignis anhängen.
    ///
    /// Schritt 3 kommt VOR Schritt 4, sonst überschritte der Store seinen
    /// eigenen Deckel für die Dauer eines Aufrufs — und ein Absturz genau
    /// dort liesse ihn überschritten zurück.
    #[allow(clippy::too_many_arguments)]
    /// Rueckgabe: die IDs der VERDRAENGTEN Versuche (M-48, Befund R10).
    ///
    /// 🔑 Nacharbeit 2: `beginne` gab `Result<(), _>` zurueck und schloss die
    /// verdraengten Versuche NUR intern. Der Wrapper im Coordinator
    /// persistierte danach ausschliesslich die neue Anlage — weder ein
    /// terminales Store-Ereignis noch die Schliessung der Taintintervalle des
    /// Verdraengten. Ein verdraengter Versuch blieb damit auf der Leitung und
    /// im Store OFFEN, und seine `art=experiment`-Intervalle sperrten weiter
    /// starke Evidenz. Wer sie schliessen soll, muss ihre IDs kennen.
    pub fn beginne(
        &mut self,
        experiment_id: &str,
        projektbindung: &str,
        passage: Passage,
        baseline: Experimentreferenz,
        begin_evidenzfolge: u64,
    ) -> Result<Vec<String>, Anlegefehler> {
        if !ist_hex32(experiment_id) {
            return Err(Anlegefehler::IdUngueltig);
        }
        if self.experimente.contains_key(experiment_id) {
            return Err(Anlegefehler::IdVergeben);
        }
        // M-30: eine Passage ohne genug gemessenes Signal trägt keinen
        // Vergleich, also auch keinen Versuch.
        if !(passage.abdeckung >= 0.5) {
            return Err(Anlegefehler::AbdeckungZuGering);
        }

        let passage_id = passage.passage_id.clone();
        if !self.passagen.contains_key(&passage_id) {
            self.log.push(Ereignis::PassageAngelegt {
                passage_id: passage_id.clone(),
                passage: Box::new(passage.clone()),
            });
            self.passagen.insert(passage_id.clone(), passage);
        }

        let verdraengt = self.verdraenge_fuer(projektbindung);

        let folge = self.naechste_folge;
        self.naechste_folge += 1;
        // Die Baseline reist ins Log, nicht nur in den gerechneten Zustand.
        let baseline_kopie = baseline.clone();
        self.experimente.insert(
            experiment_id.to_string(),
            Experiment {
                experiment_id: experiment_id.to_string(),
                projektbindung: projektbindung.to_string(),
                passage_id: passage_id.clone(),
                art: Ausfuehrungsart::ManualExternal,
                reproduzierbarkeit: Reproduzierbarkeit::ManuellNichtWiederherstellbar,
                baseline,
                kandidaten: Vec::new(),
                terminal: None,
                folge,
                gebundene_reihenfolge: None,
                begin_evidenzfolge,
                baseline_evidence_ids: Vec::new(),
                resultat_evidence_ids: Vec::new(),
            },
        );
        self.log.push(Ereignis::Begonnen {
            experiment_id: experiment_id.to_string(),
            passage_id,
            projektbindung: projektbindung.to_string(),
            baseline: Box::new(baseline_kopie),
        });
        Ok(verdraengt)
    }

    /// Schliesst so viele älteste offene Experimente, dass nach dem Anlegen
    /// beide Deckel eingehalten sind (M-48).
    ///
    /// Rueckgabe: welche Versuche dabei verdraengt wurden — der Aufrufer muss
    /// ihr Terminal persistieren und ihre Taintintervalle schliessen (R10).
    fn verdraenge_fuer(&mut self, bindung: &str) -> Vec<String> {
        let mut verdraengt = Vec::new();
        // Erst der Bereichsdeckel, dann der globale: ein Projekt, das seinen
        // eigenen Deckel sprengt, soll nicht die Zeilen anderer Projekte
        // verdrängen.
        while self.offene_im_projekt(bindung).count() >= N_PROJEKT {
            let Some(id) = self
                .offene_im_projekt(bindung)
                .min_by_key(|e| e.folge)
                .map(|e| e.experiment_id.clone())
            else {
                break;
            };
            if self.schliesse(&id, Abbruchgrund::Verdraengt).is_ok() {
                verdraengt.push(id);
            } else {
                break;
            }
        }
        while self.offene().count() >= N_GLOBAL {
            let Some(id) = self
                .offene()
                .min_by_key(|e| e.folge)
                .map(|e| e.experiment_id.clone())
            else {
                break;
            };
            if self.schliesse(&id, Abbruchgrund::Verdraengt).is_ok() {
                verdraengt.push(id);
            } else {
                break;
            }
        }
        verdraengt
    }

    /// Ein zweiter Durchgang derselben Passage (M-41).
    ///
    /// Er erzeugt einen NEUEN Kandidaten und rührt die Baseline nicht an.
    pub fn neuer_kandidat(
        &mut self,
        experiment_id: &str,
        referenz: Experimentreferenz,
        evidenzfolge: u64,
    ) -> Result<u32, Abschlussfehler> {
        let Some(e) = self.experimente.get_mut(experiment_id) else {
            return Err(Abschlussfehler::Unbekannt);
        };
        if e.terminal.is_some() {
            return Err(Abschlussfehler::SchonTerminal);
        }
        let nummer = e.kandidaten.len() as u32 + 1;
        let referenz_kopie = referenz.clone();
        e.kandidaten.push(Kandidat {
            nummer,
            referenz,
            evidenzfolge,
        });
        self.log.push(Ereignis::KandidatAngelegt {
            experiment_id: experiment_id.to_string(),
            nummer,
            referenz: Box::new(referenz_kopie),
        });
        Ok(nummer)
    }

    /// Bindet die Blindreihenfolge, BEVOR ein Urteil vorliegt (M-44).
    ///
    /// Ein zweiter Aufruf ändert sie nicht: sonst liesse sich die Bindung
    /// nach dem Hören noch drehen, und das append-only Ereignis wäre eine
    /// Behauptung ohne Wert.
    pub fn binde_reihenfolge(
        &mut self,
        experiment_id: &str,
        reihenfolge: Blindreihenfolge,
    ) -> Result<bool, Abschlussfehler> {
        let Some(e) = self.experimente.get_mut(experiment_id) else {
            return Err(Abschlussfehler::Unbekannt);
        };
        if e.terminal.is_some() {
            return Err(Abschlussfehler::SchonTerminal);
        }
        if e.gebundene_reihenfolge.is_some() {
            return Ok(false);
        }
        e.gebundene_reihenfolge = Some(reihenfolge);
        self.log.push(Ereignis::ReihenfolgeGebunden {
            experiment_id: experiment_id.to_string(),
            reihenfolge,
        });
        Ok(true)
    }

    /// `experiment_manual_result` (M-49).
    ///
    /// Zwei Riegel davor, und beide sind Zusagen und keine Bequemlichkeit:
    /// ohne gebundene Blindreihenfolge gibt es kein Urteil (M-44), und ohne
    /// eingefrorenen Match-Gain in der Baseline auch nicht (M-43, §15:
    /// „Eine Klangwertung ohne vorherigen Lautheitsabgleich ist unzulässig").
    pub fn ergebnis(
        &mut self,
        experiment_id: &str,
        hoerurteil: Hoerurteil,
        notiz: Option<String>,
        werkzeug: Option<String>,
        messung: &Resultatmessung,
    ) -> Result<Achsenrechnung, Abschlussfehler> {
        let Some(e) = self.experimente.get_mut(experiment_id) else {
            return Err(Abschlussfehler::Unbekannt);
        };
        if e.terminal.is_some() {
            return Err(Abschlussfehler::SchonTerminal);
        }
        if !e.baseline.match_gain_db.is_finite() {
            return Err(Abschlussfehler::OhneLautheitsabgleich);
        }
        let Some(reihenfolge) = e.gebundene_reihenfolge else {
            return Err(Abschlussfehler::ReihenfolgeNichtGebunden);
        };
        // 🔑 Nacharbeit 1 (Befund B20): OHNE Resultatmessung kein Abschluss.
        //
        // Vorher konnte `ergebnis()` unmittelbar nach Begin und
        // Reihenfolgebindung terminieren — ohne einen einzigen Kandidaten und
        // ohne eine einzige gerechnete Zahl. Das Terminalereignis trug dann
        // ausschliesslich Userfelder, und §43.2 („Zielmetrik, Guardrail-Deltas
        // und Effektstabilitaet rechnet der Broker") war ein Satz ohne Code.
        // 🔑 Befund R16 (M-41): OHNE einen erfassten Kandidaten gibt es kein
        // Ergebnis. Er steht VOR der Resultatmessung, weil er die staerkere
        // Aussage ist: eine Messung ohne Kandidat misst zweimal denselben
        // Zustand.
        if e.kandidaten.is_empty() {
            return Err(Abschlussfehler::OhneKandidat);
        }
        if !messung.hat_resultat() {
            return Err(Abschlussfehler::OhneResultatmessung);
        }
        // Die vier Achsen entstehen VOR dem Terminalereignis und reisen MIT
        // ihm (M-45/M-49).
        let achsen = messung.achsen(&KLASSENORDNUNG);
        e.baseline_evidence_ids = messung.baseline_evidence_ids.clone();
        e.resultat_evidence_ids = messung.resultat_evidence_ids.clone();
        e.terminal = Some(Terminal::Ergebnis {
            hoerurteil,
            blindreihenfolge: reihenfolge,
            notiz,
            werkzeug,
            achsen: achsen.clone(),
        });
        self.log.push(Ereignis::Ergebnis {
            experiment_id: experiment_id.to_string(),
            hoerurteil,
            blindreihenfolge: reihenfolge,
            achsen: Box::new(achsen.clone()),
            baseline_evidence_ids: messung.baseline_evidence_ids.clone(),
            resultat_evidence_ids: messung.resultat_evidence_ids.clone(),
        });
        Ok(achsen)
    }

    /// `experiment_abort` (M-47).
    pub fn schliesse(
        &mut self,
        experiment_id: &str,
        grund: Abbruchgrund,
    ) -> Result<(), Abschlussfehler> {
        let Some(e) = self.experimente.get_mut(experiment_id) else {
            return Err(Abschlussfehler::Unbekannt);
        };
        if e.terminal.is_some() {
            return Err(Abschlussfehler::SchonTerminal);
        }
        e.terminal = Some(Terminal::Abgebrochen { grund });
        self.log.push(Ereignis::Abgebrochen {
            experiment_id: experiment_id.to_string(),
            grund,
        });
        Ok(())
    }

    /// Ein vollständiger Export inklusive Evidenz-IDs (M-51).
    ///
    /// „Vollständig" heisst: die Passage reist mit, nicht nur ihre ID. Ein
    /// Export, der auf einen Store zeigt, den der Empfänger nicht hat, ist
    /// kein Export.
    pub fn exportiere(&self, experiment_id: &str) -> Option<Export> {
        let e = self.experimente.get(experiment_id)?;
        let p = self.passagen.get(&e.passage_id)?;
        Some(Export {
            experiment: e.clone(),
            passage: p.clone(),
            // M-51: die Evidence-IDs reisen MIT. Der Export kann sie jetzt
            // unabhaengig vom Aufrufer liefern, weil das Experiment sie fuehrt.
            baseline_evidence_ids: e.baseline_evidence_ids.clone(),
            resultat_evidence_ids: e.resultat_evidence_ids.clone(),
            ereignisse: self
                .log
                .iter()
                .filter(|ev| ev.betrifft(experiment_id))
                .cloned()
                .collect(),
        })
    }

    /// Nimmt GENAU die Wirkung EINES Befehls zurueck (Befund B3).
    ///
    /// 🔑 Wiederpruefung 2: Die Runde 2 klonte vor jedem Experimentbefehl den
    /// GANZEN Store und ersetzte bei gescheitertem Append den gesamten Stand
    /// durch diese Kopie. Committet in derselben Zeit eine andere Sitzung
    /// erfolgreich, loescht so ein Rollback deren Wirkung aus dem Speicher,
    /// obwohl sie persistiert ist — der fluechtige Stand faellt hinter das Log
    /// zurueck, und genau das schliesst §33.5 aus.
    ///
    /// Diese Form fasst nur an, was der Befehl selbst geaendert hat: den
    /// angelegten oder veraenderten Eintrag, die Terminale der von IHM
    /// verdraengten Versuche, eine von IHM angelegte Passage und die Laenge
    /// seines Logs. Alles andere bleibt stehen.
    pub fn zuruecknehmen(&mut self, r: &Ruecknahme) {
        if let Some(id) = &r.angelegt {
            self.experimente.remove(id);
        }
        for id in &r.terminale {
            // Eine Verdraengung setzt AUSSCHLIESSLICH das Terminal. Der
            // Rueckweg loest genau das wieder — eine vollstaendige Kopie des
            // Opfers waere mehr, als der Befehl angefasst hat.
            if let Some(e) = self.experimente.get_mut(id) {
                e.terminal = None;
            }
        }
        if let Some((id, vorher)) = &r.vorher {
            self.experimente.insert(id.clone(), vorher.clone());
        }
        if let Some(passage_id) = &r.passage {
            self.passagen.remove(passage_id);
        }
        self.log.truncate(r.log_laenge);
        self.naechste_folge = r.naechste_folge;
    }

    /// Der Stand, auf den ein Rueckweg zurueckfuehrt.
    pub fn marke(&self) -> (usize, u64) {
        (self.log.len(), self.naechste_folge)
    }

    /// Der Gegenpfad zum Anlegen (M-51, CLAUDE.md Änderungssatzregel).
    ///
    /// Die Passage bleibt: sie kann weitere Experimente tragen, und sie zu
    /// löschen hiesse, fremde Zeilen mitzunehmen. Das Ereignislog behält
    /// seine Einträge — es ist append-only, und ein gelöschtes Experiment ist
    /// eine Tatsache über die Vergangenheit, keine Lücke darin.
    pub fn loesche(&mut self, experiment_id: &str) -> bool {
        self.experimente.remove(experiment_id).is_some()
    }
}

impl Ereignis {
    fn betrifft(&self, id: &str) -> bool {
        match self {
            Ereignis::PassageAngelegt { .. } => false,
            Ereignis::Begonnen { experiment_id, .. }
            | Ereignis::KandidatAngelegt { experiment_id, .. }
            | Ereignis::ReihenfolgeGebunden { experiment_id, .. }
            | Ereignis::Ergebnis { experiment_id, .. }
            | Ereignis::Abgebrochen { experiment_id, .. } => experiment_id == id,
        }
    }
}

/// Das Exportpaket (M-51).
#[derive(Debug, Clone, PartialEq)]
pub struct Export {
    pub experiment: Experiment,
    pub passage: Passage,
    /// Die Belege, auf denen das Urteil beruht (M-51).
    pub baseline_evidence_ids: Vec<String>,
    pub resultat_evidence_ids: Vec<String>,
    pub ereignisse: Vec<Ereignis>,
}

// ── Die Auswertung (M-45, M-46) ─────────────────────────────────────────

/// Wie eine Achse ausgefallen ist. `NichtBeurteilbar` ist kein Zwischenwert
/// zwischen besser und schlechter, sondern die vierte, eigenständige Antwort
/// — dieselbe fail-closed-Form wie die Präsenzbits der Evidenz: „nie gesagt"
/// ist etwas anderes als „gemeldete 0".
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Achsenbefund {
    Verbessert,
    Unveraendert,
    Verschlechtert,
    NichtBeurteilbar,
}

/// Die vier getrennten Achsen aus M-45. Es gibt bewusst keine Methode, die
/// sie zu einer Note verrechnet — die gäbe es nur, damit jemand sie benutzt.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Achsen {
    /// Hat sich der adressierte Befund verändert?
    pub zielmetrik: Achsenbefund,
    /// Wurden Loudness, Peak, Transient, Breite oder geschützte Bereiche
    /// schlechter?
    pub guardrails: Achsenbefund,
    /// Bleiben Richtung und Größenordnung über Teilfenster?
    pub effektstabilitaet: Achsenbefund,
    /// Ausdrücklich USERDATEN, keine Messung. `None` = der User hat nicht
    /// geurteilt.
    pub hoerurteil: Option<Hoerurteil>,
}

/// Die fünf zulässigen Aussagen aus M-46. Mehr gibt es nicht, und
/// „objektiv besser" ist keine davon.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Urteil {
    /// (1) Ziel verbessert und Guardrails stabil.
    ZielVerbessertGuardrailsStabil,
    /// (2) Messbar anders, musikalisches Urteil offen.
    MessbarAndersUrteilOffen,
    /// (3) Keine belastbare Änderung.
    KeineBelastbareAenderung,
    /// (4) Ziel verbessert, aber geschützte Eigenschaft verschlechtert.
    ZielVerbessertGeschuetztesSchlechter,
    /// (5) Vergleich nicht gültig.
    VergleichNichtGueltig,
}

/// Das Urteil aus den vier Achsen (M-46).
///
/// `vergleichbar` kommt von aussen — aus `vergleichbarkeit::beurteile`. Es
/// steht VOR allem anderen: „Ein nicht vergleichbares Experiment erhält kein
/// starkes Siegerurteil" ist der harte Gate 6 aus §49.2, und ein Gate, das
/// erst nach der Auswertung greift, ist eine Empfehlung.
pub fn urteile(achsen: &Achsen, vergleichbar: bool) -> Urteil {
    if !vergleichbar {
        return Urteil::VergleichNichtGueltig;
    }
    // Eine Achse, die niemand beurteilen kann, macht den Vergleich ungültig
    // und nicht etwa neutral. Sonst hiesse „ich weiss es nicht" im Ergebnis
    // dasselbe wie „es hat sich nichts geändert".
    if achsen.zielmetrik == Achsenbefund::NichtBeurteilbar
        || achsen.guardrails == Achsenbefund::NichtBeurteilbar
    {
        return Urteil::VergleichNichtGueltig;
    }
    // Ein instabiler Effekt ist keine belastbare Änderung, egal wie gross er
    // im Mittel aussieht (§43.2, Effektstabilität).
    if achsen.effektstabilitaet == Achsenbefund::Verschlechtert
        || achsen.effektstabilitaet == Achsenbefund::NichtBeurteilbar
    {
        return Urteil::KeineBelastbareAenderung;
    }
    match (achsen.zielmetrik, achsen.guardrails) {
        (Achsenbefund::Verbessert, Achsenbefund::Verschlechtert) => {
            Urteil::ZielVerbessertGeschuetztesSchlechter
        }
        (Achsenbefund::Verbessert, _) => Urteil::ZielVerbessertGuardrailsStabil,
        (Achsenbefund::Unveraendert, Achsenbefund::Unveraendert) => {
            Urteil::KeineBelastbareAenderung
        }
        // Alles andere hat sich messbar bewegt, ohne dass die Messung sagen
        // könnte, ob es besser ist. Genau dafür ist diese Aussage da.
        _ => Urteil::MessbarAndersUrteilOffen,
    }
}

/// `Behalten` ist nur mit ausdrücklicher Userbestätigung möglich (M-46).
///
/// Und auch dann nicht bei einem ungültigen Vergleich: eine Bestätigung
/// kann eine fehlende Grundlage nicht ersetzen.
pub fn behalten_erlaubt(urteil: Urteil, user_bestaetigt: bool) -> bool {
    user_bestaetigt && urteil != Urteil::VergleichNichtGueltig
}

// ── Block-Bootstrap und FDR (M-45) ──────────────────────────────────────

/// Ein deterministischer Zufallsgenerator. Bootstrap braucht Ziehungen, und
/// ein Golden braucht Reproduzierbarkeit — beides zugleich geht nur mit einer
/// gesetzten Saat.
struct Wuerfel(u64);

impl Wuerfel {
    fn neu(saat: u64) -> Self {
        Wuerfel(saat.wrapping_mul(6_364_136_223_846_793_005).wrapping_add(1))
    }
    fn naechste(&mut self, grenze: usize) -> usize {
        self.0 = self
            .0
            .wrapping_mul(6_364_136_223_846_793_005)
            .wrapping_add(1_442_695_040_888_963_407);
        if grenze == 0 {
            return 0;
        }
        ((self.0 >> 33) as usize) % grenze
    }
}

/// Block-Bootstrap für ein Konfidenzintervall des Mittelwerts (M-45).
///
/// ⚠️ Warum BLOCK-Bootstrap und nicht der gewöhnliche: die Deltas
/// benachbarter Analysefenster sind korreliert — dasselbe Material, dieselbe
/// Kette, überlappende Fenster. Ein Bootstrap, der einzelne Werte zieht,
/// nimmt Unabhängigkeit an, die es nicht gibt, und liefert ein VIEL zu enges
/// Intervall. Genau daraus entstünde eine „belastbare" Änderung, die keine
/// ist.
///
/// Rückgabe: `(untere, obere)` Grenze des Intervalls zum Niveau `1 - alpha`.
pub fn block_bootstrap(
    deltas: &[f64],
    blocklaenge: usize,
    ziehungen: usize,
    alpha: f64,
    saat: u64,
) -> Option<(f64, f64)> {
    let n = deltas.len();
    if n == 0 || blocklaenge == 0 || ziehungen == 0 {
        return None;
    }
    // 🔑 Nacharbeit 1 (Befund B34): FAIL-CLOSED vor der ersten Addition.
    //
    // `block_bootstrap(&[f64::NAN], …)` akkumulierte NaN, sortierte danach
    // eine nicht geordnete Menge (`partial_cmp` faellt auf `Equal` zurueck)
    // und lieferte ein GESETZTES NaN-Intervall. Ein Aufrufer sah damit eine
    // Unsicherheitsaussage, wo gar keine gerechnet werden konnte. M-07 und
    // M-45 verlangen an dieser Stelle „ungueltig", nicht „ein Intervall".
    if deltas.iter().any(|d| !d.is_finite()) || !alpha.is_finite() {
        return None;
    }
    let bl = blocklaenge.min(n);
    let bloecke = n - bl + 1;
    let mut w = Wuerfel::neu(saat);
    let mut mittel = Vec::with_capacity(ziehungen);
    for _ in 0..ziehungen {
        let mut summe = 0.0;
        let mut gezogen = 0usize;
        while gezogen < n {
            let start = w.naechste(bloecke);
            for k in 0..bl {
                if gezogen >= n {
                    break;
                }
                summe += deltas[start + k];
                gezogen += 1;
            }
        }
        mittel.push(summe / n as f64);
    }
    mittel.sort_by(|a, b| a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal));
    let u = ((alpha / 2.0) * ziehungen as f64).floor() as usize;
    let o = (((1.0 - alpha / 2.0) * ziehungen as f64).ceil() as usize).min(ziehungen - 1);
    let (unten, oben) = (mittel[u.min(ziehungen - 1)], mittel[o]);
    // Der Riegel steht auch am AUSGANG: eine Summe endlicher Werte kann
    // ueberlaufen, und ein ueberlaufenes Intervall ist keines.
    if !unten.is_finite() || !oben.is_finite() {
        return None;
    }
    Some((unten, oben))
}

/// Der zweiseitige p-Wert einer Zeitreihe aus IHRER Bootstrapverteilung (M-45).
///
/// 🔑 Nacharbeit 2 (Befund R20): die Runde 1 erfand p-Werte als
/// `exp(-|delta|)` — ohne Streuung, ohne Stichprobenzahl. Die Effektgroesse
/// allein machte ein Band damit „signifikant", und drei Messfenster mit
/// grossem Ausschlag zaehlten wie dreissig mit demselben. Der p-Wert kommt
/// jetzt aus dem Anteil der Bootstrapmittel auf der ANDEREN Seite der Null:
/// eine Reihe, deren Mittel je nach Ziehung das Vorzeichen wechselt, ist
/// keine Aussage, so gross ihr Mittelwert auch sein mag.
///
/// `1.0` heisst „kein Beleg" — nie 0.
pub fn bootstrap_p(reihe: &[f64], blocklaenge: usize, ziehungen: usize, saat: u64) -> f64 {
    let n = reihe.len();
    if n == 0 || blocklaenge == 0 || ziehungen == 0 || reihe.iter().any(|x| !x.is_finite()) {
        return 1.0;
    }
    let bl = blocklaenge.min(n);
    let bloecke = n - bl + 1;
    let mut w = Wuerfel::neu(saat);
    let mut nicht_positiv = 0usize;
    let mut nicht_negativ = 0usize;
    for _ in 0..ziehungen {
        let mut summe = 0.0;
        let mut gezogen = 0usize;
        while gezogen < n {
            let start = w.naechste(bloecke);
            for k in 0..bl {
                if gezogen >= n {
                    break;
                }
                summe += reihe[start + k];
                gezogen += 1;
            }
        }
        let mittel = summe / n as f64;
        if !(mittel > 0.0) {
            nicht_positiv += 1;
        }
        if !(mittel < 0.0) {
            nicht_negativ += 1;
        }
    }
    let kleiner = nicht_positiv.min(nicht_negativ) as f64;
    // Zweiseitig, und mit dem ueblichen +1/+1 gegen einen p-Wert von exakt 0:
    // eine endliche Zahl Ziehungen kann „nie" nicht belegen.
    (2.0 * (kleiner + 1.0) / (ziehungen as f64 + 1.0)).min(1.0)
}

/// Benjamini-Hochberg-Korrektur für viele gleichzeitige Tests (M-45).
///
/// Bei 221 gleichzeitig gescannten Bändern sind bei α = 0,05 rund elf
/// Fehlalarme zu erwarten, wenn sich NICHTS geändert hat. Ohne Korrektur
/// meldete Nakama also fast immer irgendein „verändertes" Band — und der
/// User lernte, die Meldung zu ignorieren.
///
/// Rückgabe: je Eingabe, ob sie nach der Korrektur signifikant bleibt.
pub fn fdr_signifikant(p_werte: &[f64], q: f64) -> Vec<bool> {
    let m = p_werte.len();
    let mut aus = vec![false; m];
    if m == 0 {
        return aus;
    }
    let mut sortiert: Vec<(usize, f64)> = p_werte.iter().cloned().enumerate().collect();
    sortiert.sort_by(|a, b| a.1.partial_cmp(&b.1).unwrap_or(std::cmp::Ordering::Equal));
    // Der grösste Rang k mit p_(k) <= k/m * q; alle Ränge bis dahin gelten.
    let mut grenze: Option<usize> = None;
    for (rang, (_, p)) in sortiert.iter().enumerate() {
        let k = rang + 1;
        if *p <= (k as f64 / m as f64) * q {
            grenze = Some(rang);
        }
    }
    if let Some(g) = grenze {
        for (idx, _) in sortiert.iter().take(g + 1) {
            aus[*idx] = true;
        }
    }
    aus
}

/// Zusammenhängende Cluster signifikanter Bänder (M-45).
///
/// Die Alternative zur FDR-Korrektur, und sie beantwortet eine andere Frage:
/// FDR sagt „welche EINZELNEN Bänder halten der Mehrfachtestung stand",
/// Cluster sagt „wo liegt ein zusammenhängender Bereich". Ein EQ-Eingriff
/// wirkt auf benachbarte Bänder; drei verstreute Einzelbänder sind eher ein
/// Artefakt als eine Klangänderung.
pub fn cluster(signifikant: &[bool], mindestlaenge: usize) -> Vec<(usize, usize)> {
    let mut aus = Vec::new();
    let mut start: Option<usize> = None;
    for (i, s) in signifikant.iter().enumerate() {
        match (s, start) {
            (true, None) => start = Some(i),
            (false, Some(a)) => {
                if i - a >= mindestlaenge {
                    aus.push((a, i));
                }
                start = None;
            }
            _ => {}
        }
    }
    if let Some(a) = start {
        if signifikant.len() - a >= mindestlaenge {
            aus.push((a, signifikant.len()));
        }
    }
    aus
}
