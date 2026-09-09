//! Der Ablauf: von der Aufnahme zur Hypothese.
//!
//! `hypothesen` ist die einzige Rechnung, die alle Fachbereiche in ihrer
//! Reihenfolge ruft — Vollstaendigkeitsmarke, Masteranomalie, Screening und
//! Deckel, Gates, Stufe B, Klassenwahl, Befundbau. Sie steht bewusst als EINE
//! Funktion: die Reihenfolge IST die Zusage, und eine Aufteilung in Schritte
//! ohne eigene Bedeutung machte sie schwerer nachzulesen, nicht leichter.
//!
//! ── DREI LAGEN, NICHT ZWEI (NAK-213 E7/R4) ────────────────────────────────
//!
//! 1. Kein eindeutig fuehrendes Main → Enthaltung OHNE Ort. Bis dahin nahm
//!    die Aufnahme den letzten `main` nach `instance_id`, und der erste
//!    verschwand spurlos (G-D4).
//! 2. Fuehrendes Main ohne verwertbaren Beleg → ebenfalls Enthaltung ohne
//!    Ort. Bis dahin gab `masteranomalie` `None`, die Rechnung endete vor den
//!    Gates, und die Sitzung SCHWIEG (G-L4) — M-27 verlangt aber ein
//!    Ergebnis.
//! 3. Sonst rechnet die Kette Zeile fuer Zeile.
//!
//! ⚠️ **M-86: ohne Vollstaendigkeitsmarke wird gar nicht gerechnet.** Der
//! Riegel kommt aus DERSELBEN Funktion wie in `aufnahmen_sammeln`
//! (`intent::darf_gerechnet_werden`, NR-01) — zwei Kopien einer Bedingung
//! waeren zwei Gelegenheiten, sie verschieden zu formulieren, und genau das
//! war schon einmal passiert.
//!
//! `masteranomalie` misst auf den **64 Gruppen** des Livegitters (M-18) und
//! gibt das FEINE Intervall der staerksten Gruppe zurueck, weil Stufe B auf
//! dem 221er-Verlauf rechnet (M-19).

use super::messung::{fenster_ganz_in_passage, median};
use super::screening::{STUFE_B_AUFRUFE, STUFE_B_MAX};
use super::{
    zustand_aus_sicherheit,
    bandintervall_der_gruppe, baue_befund, enthaltung, enthaltung_ohne_ort, gate, getrennt,
    gruppe_von_band, konkurrenzlage, rang_und_beleg, rang_quantisiert, screening_quantisiert,
    screeningrang, Aufnahme, Ausschluss, Ausschlussgrund, Bandintervall,
    Beobachtung, CauseHypothesis, Enthaltungsgrund, Gateurteil, Passagenfenster, Quellprofil,
    Rangkomponenten, Sicherheitsklasse, Zielmetrik, Zusammenhangsbeleg, BAENDER_FEIN,
    GRUPPEN_LIVE, KANDIDATEN_DECKEL,
};

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
    if !crate::coordinator::intent::darf_gerechnet_werden(aufnahme.intent.as_ref()) {
        return Rechenergebnis::default();
    }

    // 🔑 **NAK-213 E7/R4: DREI Lagen statt zwei.**
    //
    // (1) Kein eindeutig fuehrendes Main → Enthaltung OHNE ORT. Bis hierher
    //     nahm die Aufnahme den letzten `main` nach `instance_id`, und der
    //     erste verschwand spurlos (G-D4).
    // (2) Fuehrendes Main ohne verwertbaren Beleg → ebenfalls Enthaltung ohne
    //     Ort. Bis hierher gab `masteranomalie` `None`, die Rechnung endete
    //     vor den Gates, und die Sitzung SCHWIEG (G-L4) — M-27 verlangt aber
    //     ein Ergebnis.
    // (3) Sonst: die Kette rechnet wie bisher, Zeile fuer Zeile.
    //
    // ⚠️ Beide Enthaltungen setzen nach R8 mindestens EINEN gueltigen Beleg
    // der Sitzung voraus; `enthaltung_ohne_ort` gibt sonst `None`, und dann
    // entsteht gar kein Befund.
    let Some(master) = aufnahme.master.as_ref() else {
        return Rechenergebnis {
            befunde: enthaltung_ohne_ort(aufnahme, Enthaltungsgrund::KeineEindeutigeFuehrung)
                .into_iter()
                .collect(),
            ausschluesse: Vec::new(),
        };
    };
    let Some((metrik, band, beobachtung, gruppe)) =
        masteranomalie(master, aufnahme.passage.as_ref())
    else {
        return Rechenergebnis {
            befunde: enthaltung_ohne_ort(aufnahme, Enthaltungsgrund::KeinMasterbeleg)
                .into_iter()
                .collect(),
            ausschluesse: Vec::new(),
        };
    };

    // Parent-Duplikate erkennen, BEVOR gerangt wird (M-22).
    //
    // 🔑 **NAK-213 E6:** die Mengen tragen den MASTER mit. Bis hierher lasen
    // beide ausschliesslich `aufnahme.kandidaten` — eine Sonde, deren Parent
    // der Master ist, bekam deshalb `duplikat = false`, volle
    // Routingqualitaet und `hoch` (Gate-Befund E-D1/A3).
    let ids: std::collections::BTreeSet<&str> = aufnahme
        .kandidaten
        .iter()
        .chain(std::iter::once(master))
        .map(|k| k.quelle_id.as_str())
        .collect();
    let ist_parent = |k: &Quellprofil| {
        aufnahme
            .kandidaten
            .iter()
            .chain(std::iter::once(master))
            .any(|anderer| anderer.parent.as_deref() == Some(k.quelle_id.as_str()))
    };

    let mut ausschluesse: Vec<Ausschluss> = Vec::new();
    // Schritt 1: die Gates. Wer faellt, faellt mit Grund (M-87).
    //
    // ⚠️ Die Duplikatmarke steht VOR dem Screening und geht NICHT in es ein:
    // sie wirkt ueber `routingqualitaet` in Stufe B, und wuerde sie die
    // Screeningordnung verschieben, entschiede eine Aussage ueber das Routing
    // darueber, WER ueberhaupt gerechnet wird.
    let mut gate_ueberlebende: Vec<(&Quellprofil, bool)> = Vec::new();
    for kandidat in &aufnahme.kandidaten {
        match gate(kandidat, aufnahme, master, metrik, band) {
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
                gate_ueberlebende.push((kandidat, duplikat));
            }
        }
    }

    // 🔑 Schritt 2: das SCREENING (M-18, NAK-213 E1). Der Deckel schneidet
    // HIER — vor Stufe B — und jeder Abgeschnittene traegt
    // `screening_ueberboten`. Die Ordnung ist deterministisch: quantisierter
    // Screeningrang absteigend, dann `candidate_source` aufsteigend
    // (derselbe `TIE_BREAK_KEY` wie die Anzeigeordnung).
    let mut gescreent: Vec<(i64, &Quellprofil, bool)> = gate_ueberlebende
        .iter()
        .map(|(kandidat, duplikat)| {
            (
                screening_quantisiert(&screeningrang(kandidat, aufnahme, master, gruppe)),
                *kandidat,
                *duplikat,
            )
        })
        .collect();
    gescreent.sort_by(|a, b| b.0.cmp(&a.0).then_with(|| a.1.quelle_id.cmp(&b.1.quelle_id)));

    // E2: verglichen wird der FUENFTE mit dem SECHSTEN — nicht der Fuehrende
    // mit dem Sechsten. Sind sie quantengleich, entschiede der Tie-Break, WER
    // gar nicht erst gerechnet wird; M-18 laesst den Deckel trotzdem hart
    // schneiden, aber M-26 verbietet, dass daraus eine starke Aussage wird.
    let deckel_ohne_abstand = gescreent.len() > KANDIDATEN_DECKEL
        && gescreent[KANDIDATEN_DECKEL - 1].0 == gescreent[KANDIDATEN_DECKEL].0;
    for (_, kandidat, _) in gescreent.iter().skip(KANDIDATEN_DECKEL) {
        ausschluesse.push(Ausschluss {
            candidate_source: kandidat.quelle_id.clone(),
            grund: Ausschlussgrund::ScreeningUeberboten,
        });
    }
    gescreent.truncate(KANDIDATEN_DECKEL);

    ausschluesse.sort();
    ausschluesse.dedup();
    let lage = konkurrenzlage(&ausschluesse, deckel_ohne_abstand);

    // 🔑 NAK-212 E1: der Zusammenhangsbeleg reist MIT. Klassenwahl (R1, R2)
    // und Trennung (R3, E6) lesen ihn; er wird EINMAL gerechnet.
    //
    // Schritt 3: Stufe B, fuer die verbliebenen hoechstens fuenf.
    let vor_stufe_b = STUFE_B_AUFRUFE.with(|z| z.get());
    let mut ueberlebende: Vec<(Rangkomponenten, Zusammenhangsbeleg, &Quellprofil, bool)> =
        gescreent
            .iter()
            .map(|(_, kandidat, duplikat)| {
                let (rang, beleg) = rang_und_beleg(kandidat, aufnahme, master, band, *duplikat);
                (rang, beleg, *kandidat, *duplikat)
            })
            .collect();
    // M-18/K-07: die Aufrufe DIESER Rechnung, gemessen an den echten
    // Aufrufen des Zaehlers — nicht an der Laenge der Schleife.
    let dieser_lauf = STUFE_B_AUFRUFE.with(|z| z.get()).saturating_sub(vor_stufe_b);
    STUFE_B_MAX.with(|z| z.set(z.get().max(dieser_lauf)));

    // Schritt 4: die ANZEIGEORDNUNG (M-25) — quantisierter Gesamtrang
    // absteigend, dann `candidate_source` aufsteigend. Die AUSWAHL hat das
    // Screening getroffen; hier wird nichts mehr abgeschnitten.
    ueberlebende.sort_by(|a, b| {
        let ra = rang_quantisiert(&a.0);
        let rb = rang_quantisiert(&b.0);
        rb.cmp(&ra).then_with(|| a.2.quelle_id.cmp(&b.2.quelle_id))
    });

    if ueberlebende.is_empty() {
        // M-27: kein Kandidat ist ein ERGEBNIS. Der Befund traegt die siebte
        // Ursachenklasse, keinen Fehlercode — und er bleibt sichtbar, damit
        // der User sieht, dass gerechnet wurde.
        let befund = enthaltung(aufnahme, master, metrik, band, beobachtung, ausschluesse.clone());
        return Rechenergebnis {
            befunde: vec![befund],
            ausschluesse,
        };
    }

    let mehrere = ueberlebende.len() > 1;

    // ⚠️ GETRENNT HEISST BEIDES ZUGLEICH — UND GEGEN JEDEN (NAK-212 E6).
    //
    // (1) Der quantisierte GESAMTRANG muss verschieden sein. Verschiedene
    //     quantisierte Komponenten koennen denselben Rang ergeben, weil
    //     `rang()` durch sechs teilt und ERNEUT quantisiert: Koinzidenz
    //     0,500000 gegen 0,500001 liefert beide Male 0,35. Ohne diese
    //     Bedingung entschiede der Tie-Break (aufsteigende
    //     `candidate_source`) darueber, WELCHER Kandidat stark wird — und
    //     M-26 sagt ausdruecklich, er entscheide „nur die Anzeigereihenfolge,
    //     nicht die Auswahl".
    //
    // (2) Mindestens eine ZUSAMMENHANGSKOMPONENTE muss verschieden sein.
    //     Bandpassung und Intent-Relevanz gehen in den Rang ein, tragen aber
    //     keine Ursachenbehauptung: 0,1 dB in einem Band weit ausserhalb des
    //     Befundintervalls trennten zwei Kandidaten um 321 Quanten (G-H3),
    //     und `rolle = fuehrt` auf dem Distraktor erklaerte messtechnisch
    //     identische Quellen fuer getrennt (A1).
    //
    // (3) Der Fuehrende wird gegen JEDEN anderen Ueberlebenden geprueft, nicht
    //     nur gegen den Zweitplatzierten. Ein Kandidat mit abweichender
    //     Intent-Relevanz kann sich zwischen zwei messtechnisch identische
    //     schieben; der Vergleich nur mit dem Nachbarn uebersieht das.
    //
    // Ein EINZIGER Ueberlebender bleibt getrennt — er hat niemanden, von dem
    // er sich abheben muesste. Die Lage „allein im Rennen neben einem
    // Messausschluss" ist ein eigener Befund und gehoert NAK-213.
    let getrennt = match ueberlebende.split_first() {
        Some((erster, weitere)) => weitere
            .iter()
            .all(|anderer| getrennt((&erster.0, &erster.1), (&anderer.0, &anderer.1))),
        None => true,
    };

    let mut befunde: Vec<CauseHypothesis> = ueberlebende
        .iter()
        .enumerate()
        .map(|(platz, (rang, beleg, kandidat, duplikat))| {
            baue_befund(
                aufnahme,
                master,
                kandidat,
                *rang,
                beleg,
                metrik,
                band,
                beobachtung,
                *duplikat,
                mehrere,
                lage,
                // Nur der FUEHRENDE traegt die Trennungsfrage; die uebrigen
                // fallen ohnehin am Riegel darunter auf `mittel` (M-21).
                platz == 0 && getrennt,
            )
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

    // Der fuehrende Befund traegt die IDs der uebrigen als Alternativen und
    // die Ausschluesse (M-21, M-87). Die uebrigen sind EIGENE Befunde mit
    // eigenem Zustand — nicht sein Anhang.
    let alle_ids: Vec<String> = befunde.iter().map(|b| b.finding_id.clone()).collect();
    // 🔑 NAK-212 R3: „jeder Alternative des anderen". Bis hierher trug NUR der
    // fuehrende Befund Alternativen; ein ungetrennter zweiter Platz zeigte auf
    // niemanden und sah aus wie ein Befund ohne Konkurrenz.
    for (platz, befund) in befunde.iter_mut().enumerate() {
        befund.alternatives = alle_ids
            .iter()
            .enumerate()
            .filter(|(i, _)| *i != platz)
            .map(|(_, id)| id.clone())
            .collect();
    }
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
/// 🔑 **NAK-213 E1:** die Rueckgabe traegt zusaetzlich die GRUPPENNUMMER.
/// Sie wird hier ohnehin bestimmt (die staerkste der 64), und das Screening
/// braucht sie. Sie ein zweites Mal zu suchen waeren zwei Gelegenheiten, sie
/// verschieden zu runden — dieselbe Lehre wie bei `rang_quantisiert`.
pub fn masteranomalie(
    master: &Quellprofil,
    passage: Option<&Passagenfenster>,
) -> Option<(Zielmetrik, Bandintervall, Beobachtung, usize)> {
    // 🔑 **NAK-212 R5/E5 (07.09.2026).** Fuehrt die Sitzung eine Passage,
    // wird die Anomalie aus dem letzten Fenster INNERHALB der Passage
    // bestimmt. Bis hierher las die Funktion `fenster.last()` ohne
    // Passagenbezug: lieferte der Master ein Fenster HINTER der Passage mit
    // einer Anomalie in einem anderen Band, nannte der Befund die alte
    // Passage und behauptete deren fremde Anomalie mit `hoch`/READY
    // (G5-Befund A4).
    //
    // ⚠️ Gibt es kein Fenster in der Passage, faellt die Funktion auf das
    // juengste Fenster UEBERHAUPT zurueck — mit UNGUELTIGER Beobachtung. Ein
    // `None` beendete die Rechnung VOR den Gates (`hypothesen`), und die
    // Sitzung schwiege; M-27 verlangt aber ein Ergebnis. Mit dem Rueckfall
    // laufen die Gates: `fenster_in_passage(master)` ist dann 0, also faellt
    // jeder Kandidat am Passagenmaterial, und das Ergebnis ist die Enthaltung
    // mit Grund. Ein BEFUND mit ungueltiger Beobachtung kann so nicht
    // entstehen.
    let (letztes, gemessen) = match passage {
        Some(p) => match fenster_ganz_in_passage(master, p).last().copied() {
            Some(f) => (f, true),
            None => (master.fenster.last()?, false),
        },
        None => (master.fenster.last()?, true),
    };
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
    let (gruppe, (abweichung, beobachtetes_band)) = je_gruppe
        .iter()
        .enumerate()
        .filter(|(_, (a, _))| a.is_finite())
        .max_by(|a, b| {
            a.1 .0
                .partial_cmp(&b.1 .0)
                .unwrap_or(std::cmp::Ordering::Equal)
                .then(b.0.cmp(&a.0))
        })?;
    let band = bandintervall_der_gruppe(gruppe);
    // 🔑 NR-06: das Band der BEOBACHTUNG liegt im zurueckgegebenen Intervall.
    // Der Ort und die Zahl, die ihn belegt, gehoeren zusammen (M-16); fielen
    // sie auseinander, zeigte die Zone etwas anderes als der Befund misst.
    debug_assert!(
        (band.von as usize) <= *beobachtetes_band && *beobachtetes_band < band.bis as usize,
        "Band {beobachtetes_band} liegt nicht in [{}, {})",
        band.von,
        band.bis
    );
    let beobachtung = Beobachtung {
        wert_db: basis + abweichung,
        // `gueltig: false` heisst: die Zahl steht ausserhalb der benannten
        // Passage und ist deshalb keine Messung ueber sie (M-07-Muster).
        gueltig: gemessen,
    };
    Some((Zielmetrik::BandPegelDb, band, beobachtung, gruppe))
}

#[cfg(test)]
mod tests {
    use super::super::testhilfe::*;
    use super::*;
    use crate::coordinator::intent::IntentBestand;

    /// **K-12, K-13 (R2/E4).** Ein einziger Ueberlebender neben einem
    /// PASSAGENausschluss traegt hoechstens `mittel`.
    ///
    /// Beide Gruende brauchen eine benannte Passage. `passage_zu_kurz` misst
    /// zusaetzlich `sonde014_gegenbeispiele.rs` am Produktpfad;
    /// `passage_unvergleichbar` ist dort nicht sauber zu bauen, weil Gate 3
    /// (Alignment) dieselbe Schwelle prueft und DAVOR steht — der verbleibende
    /// Hebel ist die fremde Transportepoche, und die loest auf dem Produktweg
    /// zuerst die Materialinvalidierung aus. HIER faellt sie an `gate()`
    /// selbst.
    #[test]
    fn einzelueberlebender_neben_passagenausschluss() {
        let gruppe = gruppe_von_band(100);
        let intervall = bandintervall_der_gruppe(gruppe);
        let spanne = (intervall.von as usize, intervall.bis as usize);
        // 🔑 Die Passage ist DOPPELT so lang wie die Masterfenster: nur
        // dann kann ein Kandidat, der jedes Masterfenster deckt, die
        // PASSAGE trotzdem verfehlen. `zeitueberdeckung` normiert auf das
        // KUERZERE Intervall — liegt eines ganz im anderen, ist das
        // Verhaeltnis immer 1,0, und Gate 4a waere unerreichbar.
        let passage = Passagenfenster {
            projekt_von: 1000,
            projekt_bis: 1000 + 20 * 500,
            transport_epoch: 1,
        };

        // Master und Ueberlebender liegen in der ZWEITEN Haelfte der Passage.
        let master = quelle_ab("m", 10, 10, spanne, 12.0);
        let ueberlebender = quelle_ab(&"a".repeat(32), 10, 10, spanne, 12.0);

        // ── Fall c: der Konkurrent deckt JEDES Masterfenster, ragt aber am
        //    Ende ueber die Passage hinaus → `passage_unvergleichbar`.
        //
        // 🔑 Der Aufbau ist enger, als er aussieht. Eine fremde
        // Transportepoche reisst schon `ueberlappt` und damit Gate 3
        // (Alignment), das VOR dem Passagengate steht — der Kandidat fiele
        // dann mit `alignment_falsch`, und die Zeile maesse einen anderen
        // Grund als den benannten (im ersten Aufbau genau so gemessen).
        // `paarueberdeckung` nimmt das MAXIMUM beider Richtungen: solange
        // jedes MASTERfenster einen Partner hat, bleibt das Alignment 1,0.
        // Die SPANNE des Kandidaten ueberlappt die Passage dann nur zur
        // Haelfte — und genau die liest Gate 4a.
        let fremd = quelle_ab(&"b".repeat(32), 10, 20, spanne, 12.0);
        // ── Fall d: nur vier Fenster in der Passage → `passage_zu_kurz`.
        let kurz = quelle_ab(&"c".repeat(32), 10, 4, spanne, 12.0);

        for (fall, konkurrent, grund) in [
            ("c", fremd.clone(), Ausschlussgrund::PassageUnvergleichbar),
            ("d", kurz.clone(), Ausschlussgrund::PassageZuKurz),
        ] {
            let mut aufnahme = aufnahme_mit(master.clone(), vec![ueberlebender.clone()]);
            aufnahme.passage = Some(passage);
            aufnahme.passage_id = Some("p".repeat(32));

            // Vorbedingung: OHNE den Konkurrenten haelt der Ueberlebende
            // jedes Gate — sonst maesse der Fall etwas anderes.
            let master_ref = aufnahme.master.as_ref().expect("die Aufnahme traegt einen Master");
            let (metrik, band, _, _) = masteranomalie(master_ref, aufnahme.passage.as_ref())
                .expect("der Master traegt eine Anomalie");
            assert_eq!(
                gate(&ueberlebender, &aufnahme, master_ref, metrik, band),
                Gateurteil::Bleibt,
                "Fall {fall}: der Ueberlebende besteht jedes Gate"
            );
            // Und der Konkurrent faellt mit GENAU dem benannten Grund.
            assert_eq!(
                gate(&konkurrent, &aufnahme, master_ref, metrik, band),
                Gateurteil::Faellt(grund),
                "Fall {fall}: der Konkurrent faellt mit {grund:?}"
            );

            aufnahme.kandidaten.push(konkurrent);
            let ergebnis = hypothesen(&aufnahme);
            let fuehrend = ergebnis.befunde.first().expect("ein Befund entsteht");
            assert!(
                ergebnis.ausschluesse.iter().any(|a| a.grund == grund),
                "Fall {fall}: der Ausschluss bleibt sichtbar (M-87)"
            );
            assert!(
                fuehrend.confidence.klasse < Sicherheitsklasse::Hoch,
                "Fall {fall}: hoechstens `mittel` neben einem Messausschluss — \
                 heutiger Stand: `getrennt` ist bei einem einzigen \
                 Ueberlebenden trivial wahr: {:?}",
                fuehrend.confidence
            );
        }
    }

    /// Ohne Vollstaendigkeitsmarke rechnet das Modul gar nicht (M-86). Der
    /// Riegel steht doppelt — hier und in der Verdrahtung.
    #[test]
    fn ohne_vollstaendigkeitsmarke_rechnet_das_modul_nicht() {
        let main = Quellprofil {
            quelle_id: "m".into(),
            fenster: vec![fenster(0, 512, 1)],
            routing_bekannt: true,
            ..Default::default()
        };
        let aufnahme = Aufnahme {
            mains: vec![main.clone()],
            master: Some(main),
            intent: Some(IntentBestand {
                vollstaendig: false,
                revision: 1,
                ..Default::default()
            }),
            ..Default::default()
        };
        assert!(hypothesen(&aufnahme).befunde.is_empty());
    }
}
