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
}

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
}

// ── Der Store ───────────────────────────────────────────────────────────

/// Ein append-only Ereignis im Log. Der Store hält den gerechneten Zustand
/// UND die Ereigniskette; die Kette ist die Wahrheit, der Zustand ihre
/// Zusammenfassung (§33.5).
#[derive(Debug, Clone, PartialEq)]
pub enum Ereignis {
    PassageAngelegt { passage_id: String },
    Begonnen { experiment_id: String, passage_id: String },
    KandidatAngelegt { experiment_id: String, nummer: u32 },
    ReihenfolgeGebunden { experiment_id: String },
    Ergebnis { experiment_id: String },
    Abgebrochen { experiment_id: String, grund: Abbruchgrund },
}

/// Der Experimentteil des Stores.
#[derive(Debug, Default)]
pub struct Experimentstore {
    passagen: BTreeMap<String, Passage>,
    experimente: BTreeMap<String, Experiment>,
    log: Vec<Ereignis>,
    naechste_folge: u64,
}

fn ist_hex32(s: &str) -> bool {
    s.len() == 32 && s.bytes().all(|b| b.is_ascii_hexdigit())
}

impl Experimentstore {
    pub fn neu() -> Self {
        Self::default()
    }

    pub fn log(&self) -> &[Ereignis] {
        &self.log
    }

    pub fn passage(&self, id: &str) -> Option<&Passage> {
        self.passagen.get(id)
    }

    pub fn experiment(&self, id: &str) -> Option<&Experiment> {
        self.experimente.get(id)
    }

    pub fn offene(&self) -> impl Iterator<Item = &Experiment> {
        self.experimente.values().filter(|e| e.offen())
    }

    pub fn offene_im_projekt<'a>(&'a self, bindung: &'a str) -> impl Iterator<Item = &'a Experiment> {
        self.offene().filter(move |e| e.projektbindung == bindung)
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
    pub fn beginne(
        &mut self,
        experiment_id: &str,
        projektbindung: &str,
        passage: Passage,
        baseline: Experimentreferenz,
    ) -> Result<(), Anlegefehler> {
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
            self.passagen.insert(passage_id.clone(), passage);
            self.log.push(Ereignis::PassageAngelegt {
                passage_id: passage_id.clone(),
            });
        }

        self.verdraenge_fuer(projektbindung);

        let folge = self.naechste_folge;
        self.naechste_folge += 1;
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
            },
        );
        self.log.push(Ereignis::Begonnen {
            experiment_id: experiment_id.to_string(),
            passage_id,
        });
        Ok(())
    }

    /// Schliesst so viele älteste offene Experimente, dass nach dem Anlegen
    /// beide Deckel eingehalten sind (M-48).
    fn verdraenge_fuer(&mut self, bindung: &str) {
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
            let _ = self.schliesse(&id, Abbruchgrund::Verdraengt);
        }
        while self.offene().count() >= N_GLOBAL {
            let Some(id) = self
                .offene()
                .min_by_key(|e| e.folge)
                .map(|e| e.experiment_id.clone())
            else {
                break;
            };
            let _ = self.schliesse(&id, Abbruchgrund::Verdraengt);
        }
    }

    /// Ein zweiter Durchgang derselben Passage (M-41).
    ///
    /// Er erzeugt einen NEUEN Kandidaten und rührt die Baseline nicht an.
    pub fn neuer_kandidat(
        &mut self,
        experiment_id: &str,
        referenz: Experimentreferenz,
    ) -> Result<u32, Abschlussfehler> {
        let Some(e) = self.experimente.get_mut(experiment_id) else {
            return Err(Abschlussfehler::Unbekannt);
        };
        if e.terminal.is_some() {
            return Err(Abschlussfehler::SchonTerminal);
        }
        let nummer = e.kandidaten.len() as u32 + 1;
        e.kandidaten.push(Kandidat { nummer, referenz });
        self.log.push(Ereignis::KandidatAngelegt {
            experiment_id: experiment_id.to_string(),
            nummer,
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
    ) -> Result<(), Abschlussfehler> {
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
        e.terminal = Some(Terminal::Ergebnis {
            hoerurteil,
            blindreihenfolge: reihenfolge,
            notiz,
            werkzeug,
        });
        self.log.push(Ereignis::Ergebnis {
            experiment_id: experiment_id.to_string(),
        });
        Ok(())
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
            ereignisse: self
                .log
                .iter()
                .filter(|ev| ev.betrifft(experiment_id))
                .cloned()
                .collect(),
        })
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
            | Ereignis::ReihenfolgeGebunden { experiment_id }
            | Ereignis::Ergebnis { experiment_id }
            | Ereignis::Abgebrochen { experiment_id, .. } => experiment_id == id,
        }
    }
}

/// Das Exportpaket (M-51).
#[derive(Debug, Clone, PartialEq)]
pub struct Export {
    pub experiment: Experiment,
    pub passage: Passage,
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
    Some((mittel[u.min(ziehungen - 1)], mittel[o]))
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
