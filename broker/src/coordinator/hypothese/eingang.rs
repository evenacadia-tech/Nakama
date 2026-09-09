//! Die Eingangsformen: was der Rechner ueber eine Quelle weiss.
//!
//! Alles hier ist eine SICHT, keine zweite Ablage. Die Verdrahtung
//! (`hypothese_verdrahtung.rs`) baut diese Formen unter dem Lock und gibt
//! sie in die Rechnung hinein, damit kein Rechenweg den Sessiongraphen
//! anhaelt.
//!
//! Zwei Zusagen leben in den Typen selbst und nicht in einer Rechnung, die
//! sie prueft:
//!
//! * **M-07 (SONDE-013): ein Band ohne Bit hat keinen Wert.**
//!   `Evidenzfenster::band` gibt `Option`, nie 0 und nie NaN — die einzige
//!   Stelle, an der ein Bandwert aus dem Rohfeld entsteht.
//! * **M-86: ohne Vollstaendigkeitsmarke wird nicht gerechnet.**
//!   `Aufnahme::intent` ist `Option`; `None` heisst „noch keine Marke", nicht
//!   „kein Schutz gewuenscht". Die Sperre selbst liegt in `rechnung.rs` und in
//!   der Verdrahtung, aus DERSELBEN Funktion (NR-01).

use crate::coordinator::intent::IntentBestand;

/// Ein einzelnes Evidenzfenster, so wie der Empfaenger es abgelegt hat.
///
/// Das ist eine SICHT auf `evidenz::Evidenzstand`, keine zweite Ablage: die
/// Verdrahtung baut sie unter dem Lock und gibt sie hier hinein, damit dieses
/// Modul den Sessiongraphen nicht anhaelt.
#[derive(Debug, Clone, Default, PartialEq)]
pub struct Evidenzfenster {
    pub evidence_id: String,
    /// Die Ankunftsreihenfolge des Brokers — die einzige Ordnung, die ueber
    /// Quellen hinweg vergleichbar ist (M-49-Muster aus SONDE-013).
    pub empfangsfolge: u64,
    pub projekt_von: i64,
    pub projekt_bis: i64,
    pub transport_epoch: u64,
    pub abdeckung: f64,
    pub p50_db: Vec<f32>,
    pub p50_gueltig: Vec<bool>,
    pub onset: f32,
    pub seitenanteil_db: Option<f64>,
    pub hat_baender: bool,
    pub hat_verteilung: bool,
    pub hat_stereo: bool,
}

impl Evidenzfenster {
    /// Der P50 eines Bandes, falls er GEMESSEN ist. Ein Band ohne Bit hat
    /// keinen Wert — nie 0, nie NaN (SONDE-013 M-07).
    pub fn band(&self, index: usize) -> Option<f64> {
        let gueltig = *self.p50_gueltig.get(index)?;
        if !gueltig {
            return None;
        }
        let wert = *self.p50_db.get(index)? as f64;
        wert.is_finite().then_some(wert)
    }
}

/// Alles, was der Rechner ueber EINE Quelle weiss.
#[derive(Debug, Clone, Default, PartialEq)]
pub struct Quellprofil {
    /// `instance_id` als hex32 — die `candidate_source` des Vertrags.
    pub quelle_id: String,
    /// Aufsteigend nach `empfangsfolge`. Die Verdrahtung sortiert; dieses
    /// Modul verlaesst sich darauf und misst es (M-25, Reduktionsreihenfolge).
    pub fenster: Vec<Evidenzfenster>,
    /// Ob das Routing dieser Quelle bekannt ist. `false` heisst „unbekannt"
    /// und deckelt die Aussage (M-22) — es heisst nie „kein Routing".
    pub routing_bekannt: bool,
    /// Der gemeldete Mixerkanal, falls die Quelle einen nennt (NAK-213 R3).
    ///
    /// 🔑 Der Vertrag laesst ihn erst ab **1** zu (`eq-ipc-v3.schema.json`
    /// `minimum: 1`, „Ausserhalb gilt der Hostwert als nicht geliefert").
    /// `None` heisst deshalb GENAU EINES: die Quelle nennt keinen Messpunkt.
    /// Es gibt keinen dritten Zustand und keinen Zahlenwert, der „unbekannt"
    /// bedeutet — `routing_bekannt` bleibt daneben stehen, weil es die
    /// benannte Frage ist, die M-22 stellt.
    pub mixerkanal: Option<i64>,
    /// Die Quelle, in die diese hineinlaeuft, falls bekannt. Sie traegt das
    /// Parent-Duplikat aus M-22.
    pub parent: Option<String>,
    /// Ob fuer diese Quelle ein PRE/POST-Paarurteil MIT Ergebnis vorliegt.
    ///
    /// 🔑 **NAK-214 E4 (08.09.2026): diese Marke traegt seit dem Ticket nur
    /// noch die ORTSANGABE.** Sie setzt `pre_post = "post"` — die Stelle, an
    /// der gemessen wurde (§8 Teil 1) — und sonst nichts. Eine Quelle misst
    /// an einem POST-Punkt, auch wenn ihr Paar nur `Probable` erreicht; das
    /// zu verschweigen waere eine Falschaussage in die Gegenrichtung.
    pub prepost_paar: bool,
    /// **NAK-214 R2/R6: traegt das Paar dieser Quelle einen GEMESSENEN
    /// Wirkungsbeleg?** (`Paarurteil::wirkungsbeleg`, `prepost.rs`.)
    ///
    /// Die STAERKE der Aussage haengt an dieser Marke, nicht an
    /// `prepost_paar`: `claim_class`, die Ursachenklasse
    /// `EffektkettePrePost` und `next_test` lesen sie. Ein einziges,
    /// strenger gemachtes Feld haette `pre_post` mitgerissen und damit eine
    /// Zusage geaendert, die R2 nicht nennt — zwei Felder halten „Begriffe
    /// nicht vermischen" ein, und der Name sagt jeweils, was er misst.
    pub prepost_wirkungsbeleg: bool,
    /// Ob ALLE Belege dieser Quelle zurueckgenommen wurden (M-24).
    pub zurueckgenommen: bool,
}

/// Das Fenster der benannten Passage, falls die Sitzung eine fuehrt.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Passagenfenster {
    pub projekt_von: i64,
    pub projekt_bis: i64,
    pub transport_epoch: u64,
}

/// Der vollstaendige Eingang einer Rechnung.
#[derive(Debug, Clone, Default)]
pub struct Aufnahme {
    /// Der Master — die Quelle, deren Befund erklaert werden soll (§8).
    ///
    /// 🔑 **NAK-213 E7/R4:** `None` heisst „die Sitzung hat kein EINDEUTIG
    /// fuehrendes Main". Bis hierher nahm `aufnahmen_sammeln` bei zwei
    /// `main`-Clients schlicht den letzten nach `instance_id` — der erste
    /// verschwand spurlos, und eine Sonde trug einen Befund im Band des
    /// ANDEREN Masters (Gate-Befund G-D4). Entwurf `:1669` sagt „genau ein
    /// fuehrendes Main pro aktiver Sitzung"; ohne das rechnet die Kette
    /// nicht, sondern enthaelt sich MIT GRUND.
    ///
    /// ⚠️ Ist er `Some`, ist er ein KLON eines Eintrags aus `mains` — dieselbe
    /// Wahrheit in einer zweiten Sicht, an EINER Stelle gebildet
    /// (`aufnahmen_sammeln`). Wer hier liest, liest nie einen Master, der
    /// nicht auch unter `mains` steht.
    pub master: Option<Quellprofil>,
    /// Die Profile ALLER `main`-Clients der Sitzung, aufsteigend nach
    /// `quelle_id` sortiert und darueber dedupliziert.
    ///
    /// Sie traegt die Adresse der Enthaltung ohne Ort: `candidate_source` ist
    /// die `quelle_id` des ERSTEN — dieselbe stabile Wahl, die
    /// `fuehrung_neu_bewerten_locked` trifft (`mains.sort(); mains[0]`). Sie
    /// behauptet KEINE Fuehrung, sie ist die deterministische Adresse der
    /// Sitzung (M-25).
    ///
    /// 🔑 **Nacharbeit 1 (08.09.2026, Erstpruefungsbefund 1): sie traegt die
    /// PROFILE, nicht nur die IDs.** Bis hierher hiess das Feld
    /// `Vec<String>`, und `aufnahmen_sammeln` legte ein Main-Profil nur ab,
    /// wenn es das FUEHRENDE war. Bei `fuehrendes_main = None` verschwanden
    /// damit saemtliche Main-Belege aus der Aufnahme: `enthaltung_ohne_ort`
    /// sammelte anschliessend nur ueber Kandidaten und Master und lieferte
    /// trotz gueltiger Sitzungsbelege `None` — die Sitzung SCHWIEG, genau in
    /// der Lage, fuer die R4 die Enthaltung erfunden hat. Die Belegsammlung
    /// aus E7 gilt der SITZUNG; wer in ihr misst, gehoert hinein, unabhaengig
    /// davon, wer sie fuehrt.
    pub mains: Vec<Quellprofil>,
    pub kandidaten: Vec<Quellprofil>,
    pub passage: Option<Passagenfenster>,
    pub passage_id: Option<String>,
    /// Der gespiegelte Intent. `None` heisst „noch keine
    /// Vollstaendigkeitsmarke" — dann rechnet gar nichts (M-86); die Sperre
    /// liegt in der Verdrahtung, dieses Modul misst sie mit.
    pub intent: Option<IntentBestand>,
    /// Die Kalibrierungsfassung, unter der DIESE Rechnung entsteht.
    pub metrics_version: u32,
    /// Die Sitzungsepoche — Teil des Schluessels, aus dem die `finding_id`
    /// entsteht. Zwei Sitzungen erzeugen nie dieselbe ID.
    pub session_epoch: String,
}

#[cfg(test)]
mod tests {
    use super::super::testhilfe::fenster;
    use super::super::BAENDER_FEIN;

    /// Ein Band ohne Bit hat KEINEN Wert — nie 0, nie NaN.
    #[test]
    fn band_ohne_bit_traegt_keinen_wert() {
        let mut f = fenster(0, 512, 1);
        f.p50_gueltig[3] = false;
        f.p50_db[7] = f32::NAN;
        assert_eq!(f.band(0), Some(-20.0));
        assert_eq!(f.band(3), None, "ohne Bit gibt es keinen Wert");
        assert_eq!(f.band(7), None, "und NaN ist auch keiner");
        assert_eq!(f.band(BAENDER_FEIN), None, "und ausserhalb erst recht");
    }
}
