//! Rechenwerkzeug — klein, rein und einzeln messbar.
//!
//! Jede Funktion hier ist eine reine Rechnung ueber Zahlen und Fenster: kein
//! Zustand, kein Lock, keine Vertragsform. Sie stehen zusammen, weil sie von
//! mehreren Fachbereichen gerufen werden — das Screening misst mit ihnen die
//! Ueberdeckung, Stufe B den Zusammenhang, der Befundbau das Passagenmaterial.
//!
//! ⚠️ **Zwei Bedeutungen nie unter einer Zahl** (M-07-Muster aus SONDE-013).
//! `bandmittel` und `korrelation_gerichtet` geben `Option`: „nicht messbar"
//! ist ein anderer Zustand als „gemessen 0", und eine 0 an dieser Stelle
//! saehe aus wie ein Ergebnis. `zeitueberdeckung` und `paarueberdeckung`
//! messen dagegen zwei VERSCHIEDENE Fragen ueber dieselben Fenster — die
//! Spannen und die einzelnen Ueberlappungen — und duerfen nicht ineinander
//! laufen.

use super::{Bandintervall, Evidenzfenster, Passagenfenster, Quellprofil};

pub(super) fn mittel(werte: impl Iterator<Item = f64>) -> f64 {
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

pub(super) fn median(werte: &[f64]) -> f64 {
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

pub(super) fn spannweite(werte: &[f64]) -> f64 {
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
pub(super) fn bandmittel(fenster: &Evidenzfenster, band: Bandintervall) -> Option<f64> {
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
pub(super) fn spanne(fenster: &[Evidenzfenster]) -> (i64, i64) {
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
pub(super) fn ueberlappt(f: &Evidenzfenster, g: &Evidenzfenster) -> bool {
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
pub(super) fn paarueberdeckung(a: &[Evidenzfenster], b: &[Evidenzfenster]) -> f64 {
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
pub(super) fn zeitueberdeckung(a: (i64, i64), b: (i64, i64)) -> f64 {
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
pub(super) fn fenster_ganz_in_passage<'a>(
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
pub(super) fn gemeinsame_reihen(
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
pub(super) fn korrelation_gerichtet(a: &[f64], b: &[f64]) -> Option<f64> {
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

#[cfg(test)]
mod tests {
    use super::super::testhilfe::fenster;
    use super::super::{bandintervall_der_gruppe, gruppe_von_band, masteranomalie};
    use super::*;
    use crate::coordinator::vergleichbarkeit::{GATE_MINDEST_FENSTER, GATE_ZEITUEBERDECKUNG};

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
}
