//! Klassenwahl und Konkurrenzlage: von der Sicherheit zum Zustand.
//!
//! Zwei Dinge stehen hier, und sie gehoeren zusammen, weil das eine das
//! andere senkt:
//!
//! * **Die Konkurrenzlage (NAK-213 R1/R2).** Sie sagt, warum die Konkurrenz
//!   EINER Rechnung unvollstaendig ist — am Deckel ohne Abstand geschnitten
//!   oder ein Konkurrent aus einem MESSGRUND ausgeschieden. Die Praezedenz
//!   zwischen beiden steht als eine Zeile in `konkurrenzlage` und nicht an
//!   zwei Aufrufstellen (K-21).
//! * **Die Abbildung Sicherheit → Zustand (M-29).** Abnahme U21 woertlich:
//!   „SICHERHEIT wird nicht als Wert ausgegeben, sondern auf den Zustand des
//!   Befunds abgebildet". Die Anzeige liest den Zustand und raet nie.
//!
//! ⚠️ **Die Klasse entsteht STRUKTURELL, nie aus dem Score (M-15).** Jede
//! Bedingung, die `hoch` ausschliesst, ist ein Praedikat ueber eine benannte
//! Struktur — nie ein Vergleich gegen eine kalibrierte Zahl. Wo die
//! numerische Grenze zwischen `mittel` und `unklar` laege, ist Ausgabe des
//! Korpus (M-31, Etappe H) und steht bewusst nirgends als Konstante. Die
//! Rechnung, die diese Praedikate zusammenfuehrt, liegt in `befund.rs`.

use super::{Ausschluss, Sicherheitsklasse};

/// `class` UND `score` — zwei Felder. Die Klasse wird NIE aus dem Score
/// gerundet (M-15): sonst waere die harte Gate-Regel aus M-20 durch einen
/// schoenen Mittelwert zu ueberstimmen.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Befundkonfidenz {
    pub klasse: Sicherheitsklasse,
    pub score: f64,
}

/// Die drei Zustaende aus Abnahme U21. Die AUTORITATIVE Abbildung
/// Sicherheit → Zustand baut Etappe D; hier steht die strukturelle
/// Vorbelegung, damit der Vertrag (`session_finding.zustand` ist Pflicht)
/// von der ersten Hypothese an erfuellt ist.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Befundzustand {
    ReadyToSend,
    MoreData,
    Stale,
}

pub const BEFUNDZUSTAENDE: [&str; 3] = ["ready_to_send", "more_data", "stale"];

impl Befundzustand {
    pub fn wire(self) -> &'static str {
        match self {
            Self::ReadyToSend => "ready_to_send",
            Self::MoreData => "more_data",
            Self::Stale => "stale",
        }
    }

    pub fn aus_wire(wort: &str) -> Option<Self> {
        Some(match wort {
            "ready_to_send" => Self::ReadyToSend,
            "more_data" => Self::MoreData,
            "stale" => Self::Stale,
            _ => return None,
        })
    }

    /// M-30: **nur** `ready_to_send` erlaubt `HOLD TO AUDITION`.
    ///
    /// Die Sperre haengt am ZUSTAND und damit am Datenweg — nicht an einer
    /// ausgegrauten Schaltflaeche, die trotzdem sendet. Beide Seiten fragen
    /// dieselbe Funktion: hier der Broker, in `SourcesModel::Befund` das
    /// Plugin (M-77).
    pub fn erlaubt_audition(self) -> bool {
        matches!(self, Self::ReadyToSend)
    }

    /// M-30: **nur** `ready_to_send` erlaubt `SEND DRAFT → EQ`.
    pub fn erlaubt_draft(self) -> bool {
        matches!(self, Self::ReadyToSend)
    }
}

/// Die Abbildung Sicherheit → Zustand (M-29).
///
/// ── EINE FUNKTION IM DATENWEG, KEINE ANZEIGEENTSCHEIDUNG ──────────────────
///
/// Abnahme U21 sagt es woertlich: „SICHERHEIT wird nicht als Wert ausgegeben,
/// sondern auf den Zustand des Befunds abgebildet". Die Anzeige liest den
/// Zustand und raet nie — deshalb steht die Abbildung hier, an genau einer
/// Stelle, und nicht in `baue_befund` und `enthaltung` zweimal.
///
/// `veraltet` kommt von aussen und nicht aus der Klasse: ein Befund wird
/// `stale`, weil sich unter ihm etwas GEAENDERT hat (Intent-Revision gestiegen,
/// Belege zurueckgenommen), nicht weil seine Sicherheit gesunken waere. Die
/// beiden Gruende sind verschieden und duerfen nicht ineinander laufen.
///
/// ⚠️ **Der Produktpfad erreicht `stale` NICHT ueber diesen Parameter**
/// (NAK-233, NB-1 im Manifest `docs/beweise/NAK-224.md`). Alle vier
/// produktiven Aufrufe uebergeben `false`: der Vorschlagspfad, die zwei
/// Enthaltungen und `baue_befund` — frisch gerechnet ist ein Befund nie
/// veraltet. `stale` entsteht spaeter und in der Verdrahtung, an genau zwei
/// Stellen in `hypothese_verdrahtung.rs`:
///
/// * `befunde_invalidieren_locked` setzt ihn beim TEILVERLUST der Evidenz —
///   einige Beleg-IDs sind zurueckgenommen, aber nicht alle; ist die Liste
///   leer, verschwindet der Befund ganz statt `stale` zu werden (M-24, M-28).
/// * `befunde_veralten_locked` setzt ihn, wenn die Intent-Generation unter
///   dem Befund gestiegen ist (E-14/WN-02).
///
/// Der Parameter bleibt trotzdem in der Signatur, und der Zweig `true` ist
/// gemessen: `broker/tests/sonde014_befund.rs` prueft ueber alle drei
/// Sicherheitsstufen, dass `veraltet` JEDE von ihnen schlaegt — das ist die
/// Zusage, die M-29 an dieser Funktion festhaelt. Wer die Signatur aendern
/// will, aendert damit einen Test und braucht einen eigenen Schritt.
pub fn zustand_aus_sicherheit(klasse: Sicherheitsklasse, veraltet: bool) -> Befundzustand {
    if veraltet {
        return Befundzustand::Stale;
    }
    match klasse {
        Sicherheitsklasse::Hoch => Befundzustand::ReadyToSend,
        // `mittel` und `unklar` sind BEIDE nicht handelbar. Wo die numerische
        // Grenze zwischen ihnen liegt, ist Ausgabe des Korpus (M-31, Etappe H)
        // und steht bewusst nirgends als Konstante.
        Sicherheitsklasse::Mittel | Sicherheitsklasse::Unklar => Befundzustand::MoreData,
    }
}

/// Warum die Konkurrenz dieser Rechnung UNVOLLSTAENDIG ist (R1, R2).
///
/// Jede Variante ist eine benannte Struktur, keine Zahl: M-15 („die Klasse
/// wird nicht aus dem Score gerundet") und M-31 („Schwelle kalibriert, nie
/// geraten") bleiben unberuehrt — dieselbe Lesart, die schon
/// `parent_duplikat` und `routing_bekannt` tragen.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Konkurrenzlage {
    /// Jeder Kandidat der Sitzung wurde vollstaendig gerechnet.
    Vollstaendig,
    /// Am Deckel wurde geschnitten, wo der Screeningrang nicht trennt (E2).
    DeckelOhneAbstand,
    /// Ein Konkurrent ist aus einem MESSGRUND ausgeschieden (R2).
    Messausschluss,
}

/// Bildet die Konkurrenzlage EINER Rechnung — an genau EINER Stelle.
///
/// ⚠️ Die Praezedenz steht hier als eine Zeile und nicht an zwei
/// Aufrufstellen (K-21): `Messausschluss` schlaegt `DeckelOhneAbstand`. Fuer
/// die Klasse ist das gleichgueltig — beide senken auf `mittel` —, fuer den
/// Leser nicht: ein gefallener Konkurrent ist die konkretere Aussage ueber
/// die Datenlage als ein willkuerlicher Schnitt.
pub fn konkurrenzlage(ausschluesse: &[Ausschluss], deckel_ohne_abstand: bool) -> Konkurrenzlage {
    if ausschluesse.iter().any(|a| a.grund.ist_messgrund()) {
        Konkurrenzlage::Messausschluss
    } else if deckel_ohne_abstand {
        Konkurrenzlage::DeckelOhneAbstand
    } else {
        Konkurrenzlage::Vollstaendig
    }
}

#[cfg(test)]
mod tests {
    use super::super::Ausschlussgrund;
    use super::*;

    /// **K-10 bis K-16 (R2/E4), geschlossene Menge.** Genau FUENF der zehn
    /// Ausschlussgruende sind Messgruende — und die `Konkurrenzlage` liest
    /// genau sie.
    ///
    /// Die drei Faelle, die auf dem Produktpfad erreichbar sind
    /// (`coverage_fehlt`, `alignment_falsch`, `evidenz_zurueckgenommen`),
    /// misst `sonde014_hypothese.rs::einzelueberlebender_neben_messausschluss`;
    /// die zwei Passagengruende brauchen eine benannte Passage und liegen im
    /// Passagenharnisch von `sonde014_gegenbeispiele.rs`. HIER faellt die
    /// Zuordnung selbst — eine Zeile, die nur an drei Beispielen gemessen
    /// waere, liesse die anderen sieben Werte offen.
    #[test]
    fn messgruende_sind_genau_die_fuenf_aus_r2() {
        let messgruende: Vec<Ausschlussgrund> = Ausschlussgrund::ALLE
            .into_iter()
            .filter(|g| g.ist_messgrund())
            .collect();
        assert_eq!(
            messgruende,
            vec![
                Ausschlussgrund::CoverageFehlt,
                Ausschlussgrund::AlignmentFalsch,
                Ausschlussgrund::PassageUnvergleichbar,
                Ausschlussgrund::PassageZuKurz,
                Ausschlussgrund::EvidenzZurueckgenommen,
            ],
            "R2 nennt woertlich diese fuenf"
        );

        let aus = |grund: Ausschlussgrund| {
            vec![Ausschluss {
                candidate_source: "a".repeat(32),
                grund,
            }]
        };

        // Jeder der fuenf setzt `Messausschluss` — einzeln gemessen.
        for grund in messgruende {
            assert_eq!(
                konkurrenzlage(&aus(grund), false),
                Konkurrenzlage::Messausschluss,
                "{grund:?} ist ein Messgrund"
            );
        }
        // Und keiner der uebrigen fuenf.
        for grund in [
            Ausschlussgrund::IntentVetoGeschuetzt,
            Ausschlussgrund::IntentVetoVerschmolzen,
            // NB-1: R2 nennt ihn nicht. NB-6: auf dem Produktpfad ist er
            // ohnehin unerreichbar, weil `masteranomalie` nur
            // `band_pegel_db` liefert und `baender` ein Pflichtfeld ist.
            Ausschlussgrund::CapabilityFehlt,
            // Ueber E2 geregelt.
            Ausschlussgrund::ScreeningUeberboten,
            // Gerade der Nachweis, dass der Kandidat keine Ursache sein KANN.
            Ausschlussgrund::MasterDuplikat,
        ] {
            assert_eq!(
                konkurrenzlage(&aus(grund), false),
                Konkurrenzlage::Vollstaendig,
                "{grund:?} macht die Konkurrenz nicht unvollstaendig"
            );
        }

        // Die Praezedenz steht an EINER Stelle (K-21): `Messausschluss`
        // schlaegt `DeckelOhneAbstand`.
        assert_eq!(
            konkurrenzlage(&[], true),
            Konkurrenzlage::DeckelOhneAbstand
        );
        assert_eq!(
            konkurrenzlage(&aus(Ausschlussgrund::CoverageFehlt), true),
            Konkurrenzlage::Messausschluss,
            "beide Lagen zugleich: der gefallene Konkurrent ist die konkretere Aussage"
        );
        assert_eq!(konkurrenzlage(&[], false), Konkurrenzlage::Vollstaendig);
    }
}
