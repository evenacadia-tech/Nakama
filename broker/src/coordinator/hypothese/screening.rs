//! Stufe A — das guenstige Screening, der Deckel und die harten Gates.
//!
//! ── GATE VOR GEWICHTUNG (M-20) ────────────────────────────────────────────
//!
//! Fehlende Coverage und falsches Alignment sind kein Gewicht, das ein
//! anderer Wert ausgleichen koennte. Ein Kandidat, der eines der beiden
//! reisst, verlaesst das Ranking mit einem GRUND aus der geschlossenen
//! Achtermenge (M-87, `vertrag.rs`) — er wird nie kommentarlos entfernt.
//! Die Reihenfolge in `gate` ist Absicht und wird gemessen: erst die Belege,
//! dann die Passage, dann der Wille des Users.
//!
//! ── JEDER AUSGESCHIEDENE TRAEGT EINEN GRUND (M-87) ────────────────────────
//!
//! 🔑 **NAK-213 (08.09.2026).** Bis dahin galt das Gegenteil: der harte
//! Deckel aus M-18 schneide „DANACH" — nach der Gewichtung —, und die
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
//!
//! Das Screening misst auf den **64 Gruppen** des Livegitters, Stufe B
//! (`zusammenhang.rs`) auf dem 221er-Verlauf (M-19). Wer die Zahlen sucht,
//! findet sie in `determinismus.rs`.

use super::messung::{mittel, paarueberdeckung, passagenmaterial, spanne, zeitueberdeckung};
use super::{
    bandpassung, intent_relevanz, koinzidenz_gerichtet, quantisiert, Aufnahme, Ausschlussgrund,
    Bandintervall, Quellprofil, Zielmetrik, BAENDER_FEIN, GRUPPEN_LIVE, RANG_QUANTUM,
};
use crate::coordinator::vergleichbarkeit::{GATE_ABDECKUNG, GATE_MINDEST_FENSTER, GATE_ZEITUEBERDECKUNG};

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
    pub(super) static STUFE_B_AUFRUFE: std::cell::Cell<usize> = const { std::cell::Cell::new(0) };
    /// Die groesste Zahl von Stufe-B-Aufrufen INNERHALB EINER Rechnung.
    ///
    /// Die Zusage gilt je RECHNUNG, nicht je Anlass: ein einziger
    /// Evidenzeingang loest im Produktpfad mehrere Rechnungen aus (Paarjoin,
    /// Zustellung, Vorschlaege), und eine absolute Summe waere nicht deutbar.
    pub(super) static STUFE_B_MAX: std::cell::Cell<usize> = const { std::cell::Cell::new(0) };
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

#[cfg(test)]
mod tests {
    use super::super::testhilfe::*;
    use super::super::{hypothesen, Passagenfenster, KANDIDATEN_DECKEL};
    use super::*;
    use crate::coordinator::intent::{IntentBestand, SchutzangabeSpiegel};

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

    /// Die Gate-Reihenfolge ist Absicht: erst die Belege, dann die Passage,
    /// dann der Wille des Users. Ein Kandidat ohne Evidenz braucht keinen
    /// Intent-Blick.
    #[test]
    fn gate_faellt_in_der_zugesagten_reihenfolge() {
        let master = masterprofil();
        let aufnahme = Aufnahme {
            mains: vec![master.clone()],
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
        let master_auf_kanal_1 = Quellprofil {
            mixerkanal: Some(1),
            ..master.clone()
        };
        let mut auf_dem_masterkanal = Aufnahme {
            mains: vec![master_auf_kanal_1.clone()],
            master: Some(master_auf_kanal_1),
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
            mains: vec![master.clone()],
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
            mains: vec![master.clone()],
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
            mains: vec![master.clone()],
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
            mains: vec![master_kurz.clone()],
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
        let master_fremde_epoche = Quellprofil {
            fenster: andere.fenster.clone(),
            ..master.clone()
        };
        let fremde_passage = Aufnahme {
            mains: vec![master_fremde_epoche.clone()],
            master: Some(master_fremde_epoche),
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
}
