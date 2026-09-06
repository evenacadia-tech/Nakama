//! Der laufende, je Frequenzbereich auflösbare Maskierungswert (SONDE-014
//! Etappe E, M-36 bis M-41).
//!
//! ── DAS WORT ──────────────────────────────────────────────────────────────
//!
//! **Maskierung**, nicht `ueberdeckung`. Letzteres heißt im Code seit
//! SONDE-013 die **Zeit**überdeckung zweier Passagenfenster
//! (`vergleichbarkeit.rs`) und ist etwas völlig anderes; der Blueprint löst
//! die Kollision selbst auf und nennt die Sache **Maskierungszone**
//! (§2.12 des Manifests). Wer die beiden Wörter mischt, mischt eine Zeit- mit
//! einer Frequenzgröße.
//!
//! ── WAS AUS ANTWORT U16 FOLGT ─────────────────────────────────────────────
//!
//! Der Befund über eine Überdeckung zweier Quellen bleibt nicht bei Worten: er
//! wird zusätzlich im Kurvenbild gezeigt. Dieses Ticket liefert dafür den
//! **Datenweg** — einen laufenden, je Frequenzbereich auflösbaren Wert, der
//! mit der Anzeigekadenz mithält. Die farbige Zone selbst ist Anzeige und
//! gehört S31b.
//!
//! Drei Dinge folgen daraus, und alle drei stehen als Struktur hier:
//!
//! 1. **Der Wert hängt AM BEFUND** (E-04) — nicht in einer zweiten Liste.
//!    Zwei Listen könnten auseinanderlaufen, und die Zone wäre eine zweite
//!    Wahrheit neben dem Befundtext. Deshalb gibt dieses Modul kein
//!    eigenständiges Objekt aus, sondern eines, das `hypothese.rs` in seinen
//!    Befund einsetzt.
//! 2. **Er benennt einen Frequenzbereich des bestehenden Gitters** — Bandindex
//!    von/bis im eingefrorenen 221er-Raster, keine zweite Frequenzachse.
//! 3. **Er trägt keine Zeichenanweisung** (M-80): keine Farbe, keine Achse,
//!    keinen Zoom. Die Engine kennt keine Optik, und der Broker auch nicht.
//!
//! ── KEINE EIGENE SCHWELLE (M-41) ──────────────────────────────────────────
//!
//! Dieses Modul kennt **keinen** Grenzwert. Es rechnet einen Wert und sagt, ob
//! er gemessen ist; ob und ab wann Gen ihn zeigt, ist Anzeige. Eine Schwelle
//! hier wäre eine zweite Detektion neben dem Befund — genau das, was der
//! P5-Bedienvertrag ausschließt („keine eigene Detektion und keine
//! konkurrierende Schwelle").

use super::hypothese::{Bandintervall, Evidenzfenster, Quellprofil};

/// Der Maskierungswert eines Befunds, in der Form von `$defs/maskierung`.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Bandbereich {
    pub von: u32,
    pub bis: u32,
}

#[derive(Debug, Clone, PartialEq)]
pub struct Maskierung {
    /// Die **maskierte** Quelle — das Ziel des Befunds.
    pub quelle_a: String,
    /// Die **maskierende** Quelle — der Kandidat.
    pub quelle_b: String,
    pub band_von: u32,
    pub band_bis: u32,
    /// Wie weit `quelle_b` im Bandbereich über `quelle_a` liegt, in dB.
    ///
    /// ⚠️ Ohne `gueltig` ist die Zahl **keine Messung**: ein Bereich ohne
    /// Messung trägt 0 mit `gueltig = false`, nie `NaN` (NaN-Ehrlichkeit).
    pub wert_db: f64,
    pub gueltig: bool,
    /// Die Kadenz einer der beiden Quellen ist reduziert (M-37).
    ///
    /// Ein **Kennzeichen am Wert**, nie ein interpolierter Zwischenwert. Der
    /// Unterschied ist die ganze Zusage: ein interpolierter Wert sähe aus wie
    /// eine Messung, die es nicht gab.
    pub herabgesetzt: bool,
}

/// Rechnet den Maskierungswert zweier Quellen in einem Bandbereich.
///
/// ── DIE ZEITLICHE ZUSAGE (M-37) ───────────────────────────────────────────
///
/// Gerechnet wird aus dem **jüngsten** Fenster jeder Seite. Damit ist der Wert
/// „so aktuell wie die jüngste Evidenz beider Quellen" — genau die Formel aus
/// M-37, und sie ist eine Aussage über das **Alter relativ zur Evidenz**, nicht
/// über eine Wanduhrrate. Paint-FPS ist nicht Datenkadenz.
///
/// Ein Mittel über die ganze Historie wäre hier falsch: der Wert soll
/// **laufend** sein, und ein Mittelwert über acht Sekunden läuft nicht.
///
/// ── WARUM EINE DIFFERENZ UND KEIN MODELL ──────────────────────────────────
///
/// `wert_db = pegel_b − pegel_a` im Bandbereich. Das ist bewusst die einfache,
/// nachvollziehbare Größe und kein psychoakustisches Maskierungsmodell: ein
/// Modell brächte Konstanten mit, die niemand kalibriert hat, und §42.4
/// verlangt, dass jeder angezeigte Zahlenwert auf ein Feld und eine Evidenz
/// zurückführbar bleibt. Zwei Bandpegel sind das; eine Modellausgabe wäre es
/// nicht.
pub fn maskierung(
    maskiert: &Quellprofil,
    maskierer: &Quellprofil,
    band: Bandintervall,
) -> Option<Maskierung> {
    if !band.gueltig() {
        return None;
    }
    let (Some(a), Some(b)) = (maskiert.fenster.last(), maskierer.fenster.last()) else {
        // Ohne Fenster auf einer der beiden Seiten gibt es gar keinen Wert —
        // ausdruecklich nicht „0 dB". Die Abwesenheit des Objekts heisst
        // „nichts", und das ist etwas anderes als „keine Maskierung".
        return None;
    };
    let pegel_a = bandpegel(a, band);
    let pegel_b = bandpegel(b, band);
    let (wert_db, gueltig) = match (pegel_a, pegel_b) {
        (Some(x), Some(y)) if (y - x).is_finite() => (y - x, true),
        // Ein Bereich ohne Messung traegt 0 mit `gueltig = false`. Der Wert
        // reist trotzdem: die Anzeige soll den Unterschied zwischen „hier ist
        // nichts gemessen" und „hier ist nichts" sehen koennen.
        _ => (0.0, false),
    };
    Some(Maskierung {
        quelle_a: maskiert.quelle_id.clone(),
        quelle_b: maskierer.quelle_id.clone(),
        band_von: band.von,
        band_bis: band.bis,
        wert_db,
        gueltig,
        herabgesetzt: kadenz_reduziert(maskiert) || kadenz_reduziert(maskierer),
    })
}

/// Der Bandpegel eines Fensters im Bereich, in dB — als **Leistungssumme**,
/// nicht als dB-Mittel.
///
/// ⚠️ dB zu mitteln ist ein geometrisches Mittel und unterschätzt genau die
/// Spitze, um die es bei einer Maskierung geht. Zwei Bänder mit −20 und −40 dB
/// ergeben gemittelt −30 dB, summiert aber −19,96 dB — und gehört wird die
/// Summe.
fn bandpegel(fenster: &Evidenzfenster, band: Bandintervall) -> Option<f64> {
    let mut leistung = 0.0f64;
    let mut gemessen = false;
    for index in band.von as usize..band.bis as usize {
        let Some(db) = fenster.band(index) else {
            continue;
        };
        let anteil = 10f64.powf(db / 10.0);
        if !anteil.is_finite() {
            continue;
        }
        leistung += anteil;
        gemessen = true;
    }
    if !gemessen || leistung <= 0.0 {
        return None;
    }
    let db = 10.0 * leistung.log10();
    db.is_finite().then_some(db)
}

/// Ob die Evidenzkadenz dieser Quelle reduziert ist (M-37).
///
/// ── WORAN DER BROKER DAS SIEHT ────────────────────────────────────────────
///
/// Die Kadenzreduktion passiert beim **Sender**: bei P1-Rückstau sendet die
/// Sonde seltener, bis hinunter auf 1 Hz. Der Broker bekommt davon keinen
/// eigenen Zähler — der Heartbeat trägt `queue_overflows`, aber der Coordinator
/// hält ihn heute nicht, und ihn dafür einzuführen wäre eine zweite Wahrheit
/// über dieselbe Sache.
///
/// Was der Broker **selbst** misst, ist die Folge: die Fenster liegen nicht
/// mehr aneinander. Im Normalbetrieb schließt jedes Fenster an das vorige an;
/// bei reduzierter Kadenz entsteht eine Lücke. Genau das wird hier gemessen —
/// an den beiden **jüngsten** Fenstern, weil der Wert eine Aussage über
/// **jetzt** ist und nicht über die Historie.
///
/// ⚠️ Eine Epochengrenze ist **keine** Kadenzreduktion: dort beginnt die
/// Projektzeit neu, und der Abstand ist bedeutungslos. Ohne diese Zeile
/// meldete jeder Seek eine Herabsetzung.
fn kadenz_reduziert(quelle: &Quellprofil) -> bool {
    let n = quelle.fenster.len();
    if n < 2 {
        // Ein einzelnes Fenster hat keine Kadenz. „Unbekannt" ist hier
        // ausdruecklich NICHT „herabgesetzt": ein Kennzeichen, das schon beim
        // ersten Beleg steht, sagt nichts mehr aus.
        return false;
    }
    let vorher = &quelle.fenster[n - 2];
    let jetzt = &quelle.fenster[n - 1];
    if vorher.transport_epoch != jetzt.transport_epoch {
        return false;
    }
    let luecke = jetzt.projekt_von.saturating_sub(vorher.projekt_bis);
    let fensterlaenge = jetzt.projekt_bis.saturating_sub(jetzt.projekt_von).max(1);
    luecke > fensterlaenge
}

#[cfg(test)]
mod tests {
    use super::*;

    fn fenster(von: i64, bis: i64, epoche: u64, db: f32) -> Evidenzfenster {
        Evidenzfenster {
            evidence_id: format!("{von:032x}"),
            empfangsfolge: von.unsigned_abs(),
            projekt_von: von,
            projekt_bis: bis,
            transport_epoch: epoche,
            abdeckung: 0.9,
            p50_db: vec![db; super::super::hypothese::BAENDER_FEIN],
            p50_gueltig: vec![true; super::super::hypothese::BAENDER_FEIN],
            onset: 1.0,
            seitenanteil_db: None,
            hat_baender: true,
            hat_verteilung: true,
            hat_stereo: false,
        }
    }

    fn profil(id: &str, fenster: Vec<Evidenzfenster>) -> Quellprofil {
        Quellprofil {
            quelle_id: id.into(),
            fenster,
            routing_bekannt: true,
            ..Default::default()
        }
    }

    const BAND: Bandintervall = Bandintervall { von: 10, bis: 14 };

    /// Der Wert ist die Differenz zweier Bandpegel — und er benennt den
    /// Bereich, aus dem er kommt.
    #[test]
    fn wert_ist_die_differenz_und_benennt_den_bereich() {
        let a = profil("a", vec![fenster(0, 512, 1, -30.0)]);
        let b = profil("b", vec![fenster(0, 512, 1, -20.0)]);
        let m = maskierung(&a, &b, BAND).expect("beide Seiten messen");
        assert_eq!(m.quelle_a, "a");
        assert_eq!(m.quelle_b, "b");
        assert_eq!((m.band_von, m.band_bis), (10, 14));
        assert!(m.gueltig);
        // Vier Baender à −20 gegen vier à −30: die Differenz der
        // Leistungssummen ist wieder 10 dB.
        assert!((m.wert_db - 10.0).abs() < 1e-9, "{}", m.wert_db);
        assert!(!m.herabgesetzt);
        // Die Gegenrichtung ist das Negative — der Wert ist gerichtet.
        let umgekehrt = maskierung(&b, &a, BAND).unwrap();
        assert!((umgekehrt.wert_db + 10.0).abs() < 1e-9);
    }

    /// Ohne Messung gibt es 0 mit `gueltig = false` — nie `NaN` und nie ein
    /// stilles Nichts.
    #[test]
    fn ohne_messung_null_mit_gueltig_falsch() {
        let mut ohne = fenster(0, 512, 1, -20.0);
        for i in BAND.von as usize..BAND.bis as usize {
            ohne.p50_gueltig[i] = false;
        }
        let a = profil("a", vec![ohne]);
        let b = profil("b", vec![fenster(0, 512, 1, -20.0)]);
        let m = maskierung(&a, &b, BAND).expect("das Objekt entsteht trotzdem");
        assert!(!m.gueltig);
        assert_eq!(m.wert_db, 0.0);
        assert!(m.wert_db.is_finite());
    }

    /// Ohne Fenster gibt es gar keinen Wert — ausdruecklich nicht „0 dB".
    #[test]
    fn ohne_fenster_gibt_es_kein_objekt() {
        let a = profil("a", Vec::new());
        let b = profil("b", vec![fenster(0, 512, 1, -20.0)]);
        assert!(maskierung(&a, &b, BAND).is_none());
        assert!(maskierung(&b, &a, BAND).is_none());
        // Und ein ungueltiges Bandintervall ebenso.
        let c = profil("c", vec![fenster(0, 512, 1, -20.0)]);
        assert!(maskierung(&b, &c, Bandintervall { von: 5, bis: 5 }).is_none());
        assert!(maskierung(&b, &c, Bandintervall { von: 0, bis: 222 }).is_none());
    }

    /// Eine Luecke zwischen den beiden juengsten Fenstern ist eine
    /// Kadenzreduktion; eine Epochengrenze ist keine.
    #[test]
    fn herabgesetzt_misst_die_luecke_und_nicht_den_seek() {
        let dicht = profil(
            "a",
            vec![fenster(0, 512, 1, -30.0), fenster(512, 1024, 1, -30.0)],
        );
        let b = profil("b", vec![fenster(0, 512, 1, -20.0)]);
        assert!(!maskierung(&dicht, &b, BAND).unwrap().herabgesetzt);

        // Eine Luecke von mehr als einer Fensterlaenge.
        let duenn = profil(
            "a",
            vec![fenster(0, 512, 1, -30.0), fenster(4096, 4608, 1, -30.0)],
        );
        assert!(maskierung(&duenn, &b, BAND).unwrap().herabgesetzt);

        // Dieselbe Luecke ueber eine Epochengrenze: KEINE Reduktion.
        let seek = profil(
            "a",
            vec![fenster(0, 512, 1, -30.0), fenster(4096, 4608, 2, -30.0)],
        );
        assert!(!maskierung(&seek, &b, BAND).unwrap().herabgesetzt);

        // Das Kennzeichen der ZWEITEN Quelle zaehlt genauso.
        assert!(maskierung(&b, &duenn, BAND).unwrap().herabgesetzt);

        // Ein einzelnes Fenster hat keine Kadenz — und meldet keine Reduktion.
        let einzeln = profil("a", vec![fenster(0, 512, 1, -30.0)]);
        assert!(!maskierung(&einzeln, &b, BAND).unwrap().herabgesetzt);
    }

    /// Der Bandpegel ist eine LEISTUNGSSUMME. Ein dB-Mittel unterschaetzte die
    /// Spitze, um die es geht.
    #[test]
    fn bandpegel_summiert_leistung_statt_dezibel_zu_mitteln() {
        let mut gemischt = fenster(0, 512, 1, -40.0);
        gemischt.p50_db[10] = -20.0;
        let pegel = bandpegel(&gemischt, BAND).unwrap();
        // Ein dB-Mittel ergaebe (−20 −40 −40 −40)/4 = −35 dB.
        // Die Leistungssumme liegt bei rund −19,87 dB.
        assert!(pegel > -20.0 && pegel < -19.0, "{pegel}");
    }

    /// Der Wert traegt keine Zeichenanweisung. Das ist eine Aussage ueber den
    /// TYP: er hat kein Feld dafuer, und ein Feld liesse sich nicht
    /// nachtraeglich fuellen.
    #[test]
    fn der_wert_traegt_keine_zeichenanweisung() {
        let a = profil("a", vec![fenster(0, 512, 1, -30.0)]);
        let b = profil("b", vec![fenster(0, 512, 1, -20.0)]);
        let m = maskierung(&a, &b, BAND).unwrap();
        // Sieben Felder, und keines davon ist Optik. Die Zahl steht hier als
        // Literal, damit ein achtes Feld diesen Fall bricht.
        let als_text = format!("{m:?}");
        for optik in ["farbe", "color", "achse", "zoom", "ausschnitt", "alpha"] {
            assert!(!als_text.contains(optik), "{optik} steht im Wert");
        }
        assert_eq!(als_text.matches(':').count(), 7, "genau sieben Felder");
    }
}
