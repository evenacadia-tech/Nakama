//! Die geteilten Testbauer der Ursachenrechnung.
//!
//! Sie stehen hier und nicht in einem der Fachmodule, weil die Tests
//! mehrerer Fachbereiche dieselben Eingangsformen brauchen: ein Fenster mit
//! Grundpegel, ein Quellprofil mit gesetztem Band, eine vollstaendige
//! Aufnahme. Eine zweite Fassung je Modul waeren mehrere Gelegenheiten, den
//! Grundpegel verschieden zu waehlen — und damit Tests, die dasselbe messen
//! und sich doch widersprechen koennten.
//!
//! Nur unter `cfg(test)` gebaut; kein Produktpfad ruft hier etwas.

use super::*;
use crate::coordinator::intent::SchutzangabeSpiegel;

pub(super) fn fenster(von: i64, bis: i64, epoche: u64) -> Evidenzfenster {
    Evidenzfenster {
        evidence_id: format!("{von:032x}"),
        empfangsfolge: von.unsigned_abs(),
        projekt_von: von,
        projekt_bis: bis,
        transport_epoch: epoche,
        abdeckung: 0.9,
        p50_db: vec![-20.0; BAENDER_FEIN],
        p50_gueltig: vec![true; BAENDER_FEIN],
        onset: 1.0,
        seitenanteil_db: None,
        hat_baender: true,
        hat_verteilung: true,
        hat_stereo: false,
    }
}

/// Ein Rangkomponentensatz aus den sechs Zahlen, in Vertragsreihenfolge.
pub(super) fn rang(bp: f64, koinz: f64, upl: f64, intent: f64, wdh: f64, routing: f64) -> Rangkomponenten {
    Rangkomponenten {
        bandpassung: bp,
        koinzidenz: koinz,
        uplift: upl,
        intent_relevanz: intent,
        wiederholbarkeit: wdh,
        routingqualitaet: routing,
    }
}

pub(super) fn beleg(upl: Option<f64>, koinz: Option<f64>, wdh: f64) -> Zusammenhangsbeleg {
    Zusammenhangsbeleg {
        uplift: upl,
        koinzidenz: koinz,
        wiederholbarkeit: wdh,
    }
}

/// Ein Quellprofil mit gesetzten Baendern in `band` und der Onsetreihe
/// `onsets`. Die uebrigen Baender bleiben auf dem Grundpegel.
pub(super) fn quelle(
    id: &str,
    anzahl: usize,
    band: (usize, usize),
    db: f32,
    onsets: &[f32],
) -> Quellprofil {
    let mut fenster_liste = Vec::new();
    for i in 0..anzahl {
        let von = 1000 + (i as i64) * 500;
        let mut f = fenster(von, von + 500, 1);
        for index in band.0..band.1.min(BAENDER_FEIN) {
            f.p50_db[index] = -20.0 + db;
        }
        f.onset = onsets.get(i).copied().unwrap_or(1.0);
        fenster_liste.push(f);
    }
    Quellprofil {
        quelle_id: id.into(),
        fenster: fenster_liste,
        routing_bekannt: true,
        ..Default::default()
    }
}

/// Wie `quelle`, aber ab einem gegebenen Fensterindex und ohne Onsetreihe.
pub(super) fn quelle_ab(
    id: &str,
    ab: usize,
    anzahl: usize,
    band: (usize, usize),
    db: f32,
) -> Quellprofil {
    let mut fenster_liste = Vec::new();
    for i in ab..ab + anzahl {
        let von = 1000 + (i as i64) * 500;
        let mut f = fenster(von, von + 500, 1);
        for index in band.0..band.1.min(BAENDER_FEIN) {
            f.p50_db[index] = -20.0 + db;
        }
        fenster_liste.push(f);
    }
    Quellprofil {
        quelle_id: id.into(),
        fenster: fenster_liste,
        routing_bekannt: true,
        ..Default::default()
    }
}

pub(super) fn aufnahme_mit(master: Quellprofil, kandidaten: Vec<Quellprofil>) -> Aufnahme {
    Aufnahme {
        mains: vec![master.clone()],
        master: Some(master),
        kandidaten,
        passage: None,
        passage_id: None,
        // M-86 ist fail-closed: ohne Vollstaendigkeitsmarke rechnet gar
        // nichts (`darf_gerechnet_werden`). Die Marke gehoert deshalb in
        // die Grundform jeder Aufnahme dieses Abschnitts.
        intent: Some(crate::coordinator::intent::IntentBestand {
            vollstaendig: true,
            ..Default::default()
        }),
        metrics_version: 1,
        session_epoch: "e".repeat(32),
    }
}

pub(super) fn masterprofil() -> Quellprofil {
    Quellprofil {
        quelle_id: "m".into(),
        fenster: (0..12)
            .map(|i| fenster(i * 512, (i + 1) * 512, 1))
            .collect(),
        routing_bekannt: true,
        ..Default::default()
    }
}
