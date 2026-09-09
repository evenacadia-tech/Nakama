//! Kandidatenbildung, Rang und `CauseHypothesis` (SONDE-014 Etappe C).
//!
//! ── WER HIER RECHNET, UND WARUM AUSGERECHNET HIER ─────────────────────────
//!
//! Entscheid **E-08** weist Hypothese, Proposal und Maskierung ausdruecklich
//! dem Broker-Coordinator zu: „dort liegen Evidenzbestand und Store". Der
//! Intent dagegen gehoert dem Main — dieses Modul LIEST ihn nur (ueber
//! `intent::IntentBestand`) und schreibt ihn nie zurueck.
//!
//! Dieses Modul ist eine REINE Rechnung. Es kennt weder Lock noch Store noch
//! Senke; alles, was es braucht, steht in `Aufnahme`. Der Produktpfad — wer
//! die Aufnahme fuellt, wann gerechnet wird und wohin das Ergebnis reist —
//! liegt in `hypothese_verdrahtung.rs`. Das ist Risiko **R1** des Bauplans:
//! ein Test, der nur dieses Modul ruft, kann nicht fallen, wenn die
//! Verdrahtung fehlt.
//!
//! ── DIE DREI HARTEN REGELN DIESES MODULS ──────────────────────────────────
//!
//! 1. **Gate vor Gewichtung (M-20).** Fehlende Coverage und falsches
//!    Alignment sind kein Gewicht, das ein anderer Wert ausgleichen koennte.
//!    Ein Kandidat, der eines der beiden reisst, verlaesst das Ranking mit
//!    einem GRUND aus der geschlossenen Achtermenge (M-87) — er wird nie
//!    kommentarlos entfernt.
//! 2. **Aus paralleler Telemetrie entsteht nie Klasse 2 oder 3 (M-17).** Der
//!    Wirkungsbeleg verlangt ein PRE/POST-Paar, der Ursachenbeleg eine
//!    kontrollierte Preview — und die gibt es in P5 gar nicht (§42.2 Punkt 5).
//!    `aussageklasse()` kann `Ursachenbeleg` in diesem Modul nicht erzeugen.
//! 3. **Gleicher Eingang, gleiche Reihenfolge (M-25).** Die Regel und die
//!    Groessen, die sie tragen, stehen in `determinismus.rs`.
//!
//! ── JEDER AUSGESCHIEDENE TRAEGT EINEN GRUND (M-87) ────────────────────────
//!
//! 🔑 **NAK-213 (08.09.2026).** Bis hierher stand hier das Gegenteil: der
//! harte Deckel aus M-18 schneide „DANACH" — nach der Gewichtung —, und die
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

use super::vergleichbarkeit::GATE_MINDEST_FENSTER;

mod determinismus;
mod eingang;
mod messung;
mod screening;
mod vertrag;
mod zusammenhang;
#[cfg(test)]
mod testhilfe;

pub use determinismus::*;
pub use eingang::*;
pub use messung::*;
pub use screening::*;
pub use vertrag::*;
pub use zusammenhang::*;

// ═════════════════════════════════════════════════════════════════════════
// Die Ausgangsformen
// ═════════════════════════════════════════════════════════════════════════

/// Halboffenes Bandintervall `[von, bis)` im 221er-Gitter.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub struct Bandintervall {
    pub von: u32,
    pub bis: u32,
}

impl Bandintervall {
    pub fn gueltig(&self) -> bool {
        self.von < self.bis && (self.bis as usize) <= BAENDER_FEIN
    }
}

/// Ein Messwert mit seinem Gueltigkeitsbit. Ohne das Bit ist die Zahl keine
/// Messung.
#[derive(Debug, Clone, Copy, PartialEq, Default)]
pub struct Beobachtung {
    pub wert_db: f64,
    pub gueltig: bool,
}

/// `class` UND `score` — zwei Felder. Die Klasse wird NIE aus dem Score
/// gerundet (M-15): sonst waere die harte Gate-Regel aus M-20 durch einen
/// schoenen Mittelwert zu ueberstimmen.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Befundkonfidenz {
    pub klasse: Sicherheitsklasse,
    pub score: f64,
}

/// Ein ausgeschiedener Kandidat samt Grund (M-87).
#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct Ausschluss {
    pub candidate_source: String,
    pub grund: Ausschlussgrund,
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

/// Die `CauseHypothesis` aus §36.3 — zehn Felder — samt den sechs Teilen der
/// Ergebnisform aus §8 und den drei Anzeigezeilen aus Abnahme U21.
#[derive(Debug, Clone, PartialEq)]
pub struct CauseHypothesis {
    pub finding_id: String,
    pub claim_class: Aussageklasse,
    pub ursachenklasse: Ursachenklasse,
    pub target_metric: Zielmetrik,
    pub candidate_source: String,
    pub passage_id: Option<String>,
    /// §8 Teil 1, zweite Haelfte: die optionale PRE/POST-Stelle des Orts.
    pub pre_post: Option<&'static str>,
    pub band_hz: Bandintervall,
    pub beobachtung: Beobachtung,
    pub rang: Rangkomponenten,
    pub confidence: Befundkonfidenz,
    /// Exit-Gate §59 woertlich: jede sichtbare Behauptung referenziert
    /// EXISTENTE Evidenz-IDs. Leer heisst: der Befund ist nicht sichtbar.
    pub evidence_ids: Vec<String>,
    /// Abnahme U21: die IDs EIGENER Befunde, kein Text.
    pub alternatives: Vec<String>,
    pub ausschluesse: Vec<Ausschluss>,
    pub next_test: NaechsterTest,
    pub zustand: Befundzustand,
    pub intent_revision: i64,
    /// **E-14 / WN-02 (Nacharbeit 2, 07.09.2026): die Bestandsgeneration,
    /// unter der dieser Befund gerechnet wurde.**
    ///
    /// Sie reist NICHT ueber den Draht: der Vertrag traegt weiterhin die
    /// `intent_revision` (M-10), und die Fassung 3 wird ausserhalb ihrer
    /// eigenen Etappe nicht angefasst. Sie ist die brokerinterne Antwort auf
    /// die Frage, ob eine Aussage noch auf dem Bestand ruht, aus dem sie
    /// entstanden ist: das Veralten und die Eintragungspruefung messen sie,
    /// weil eine unveraenderte Sender-Revision einen geaenderten Bestand
    /// nicht ausschliesst (WP1-2).
    pub intent_generation: i64,
    pub likely_cause: String,
    pub smallest_test: String,
    pub listen_for: String,
    /// Der laufende Maskierungswert AM Befund (E-04, Etappe E).
    ///
    /// `None` heisst „dieser Befund traegt keinen" — nie „0 dB". Das Objekt
    /// haengt hier und nicht in einer zweiten Liste: zwei Listen koennten
    /// auseinanderlaufen, und die Zone waere eine zweite Wahrheit neben dem
    /// Befundtext.
    pub maskierung: Option<super::maskierung::Maskierung>,
    /// Die Fassung, unter der dieser Befund entstanden ist.
    pub metrics_version: u32,
}

/// Die SECHS Teile der Ergebnisform aus §8, in ihrer Vertragsreihenfolge.
///
/// Sie sind Datenmodell und Regelbasis — KEINE Vorgabe, sechs Werte
/// anzuzeigen (Abnahme U21: Gen zeigt genau drei Zeilen).
pub const ERGEBNISTEILE: [&str; 6] = [
    "ort",
    "beobachtung",
    "zusammenhang",
    "alternativen",
    "sicherheit",
    "naechster_beweisschritt",
];

impl CauseHypothesis {
    /// M-16: die sechs Teile, jeder mit der Stelle, an der er im Objekt
    /// wirklich steht. Ein Teil, den das Objekt nicht traegt, ist `false` —
    /// und genau daran faellt der Rotbeweis.
    pub fn sechs_teile(&self) -> [(&'static str, bool); 6] {
        [
            // 1. Ort: der Bus, optional mit PRE/POST-Stelle.
            ("ort", !self.candidate_source.is_empty()),
            // 2. Beobachtung: WAS gemessen wurde, mit Gueltigkeitsbit.
            (
                "beobachtung",
                self.band_hz.gueltig() && self.beobachtung.wert_db.is_finite(),
            ),
            // 3. Zusammenhang: Aussageklasse, Ursachenklasse und Rang.
            (
                "zusammenhang",
                self.rang.werte().iter().all(|w| w.is_finite()),
            ),
            // 4. Alternativen: eigene Befunde, als IDs. Eine LEERE Liste ist
            //    ein gueltiger Teil („es gibt keine"); eine fehlende Liste
            //    waere keiner. Die Unterscheidung traegt der Typ.
            ("alternativen", self.alternatives.len() <= 8),
            // 5. Sicherheit: Klasse UND Score, zwei Felder.
            (
                "sicherheit",
                self.confidence.score.is_finite() && (0.0..=1.0).contains(&self.confidence.score),
            ),
            // 6. Naechster Beweisschritt.
            (
                "naechster_beweisschritt",
                NAECHSTE_TESTS.contains(&self.next_test.wire()),
            ),
        ]
    }

    /// Der Tie-Break-Key (M-25/M-26). Er entscheidet die ANZEIGEREIHENFOLGE.
    pub fn tie_break(&self) -> (i64, &str) {
        // Absteigend nach Rang: das negierte Ganzzahlbild der quantisierten
        // Zahl. Ueber `f64` zu sortieren waere keine totale Ordnung.
        let stufen = (quantisiert(self.rang.rang()) / RANG_QUANTUM).round() as i64;
        (-stufen, self.candidate_source.as_str())
    }
}

/// Warum die Rechnung GAR NICHT ERST laufen kann (R4).
///
/// ⚠️ Der Grund reist im festen Satz von `likely_cause` — dem Feld, das
/// §36.3 fuer genau diese Zeile vorsieht. Ein eigenes Vertragsfeld
/// `enthaltungsgrund` waere eine neue geschlossene Menge mit einem Consumer,
/// den dieses Ticket nicht bauen darf (§2.9 Nr. 8); ob und wie Gen ihn zeigt,
/// ist S31b.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Enthaltungsgrund {
    /// `fuehrendes_main = None` — kein oder mehr als ein fuehrendes Main (R4).
    KeineEindeutigeFuehrung,
    /// Das fuehrende Main hat keinen verwertbaren Beleg (R4).
    KeinMasterbeleg,
}

impl Enthaltungsgrund {
    /// Der feste Satz. Regelbasiert, keine Erklaerschicht.
    pub fn satz(self) -> &'static str {
        match self {
            Self::KeineEindeutigeFuehrung => {
                "Zwei Instanzen fuehren diese Sitzung — bis eine von ihnen fuehrt, wird nicht gerechnet."
            }
            Self::KeinMasterbeleg => {
                "Das fuehrende Main hat noch keinen verwertbaren Beleg gesendet."
            }
        }
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

// ═════════════════════════════════════════════════════════════════════════
// Die Hypothese
// ═════════════════════════════════════════════════════════════════════════

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
    if !super::intent::darf_gerechnet_werden(aufnahme.intent.as_ref()) {
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

/// Die Enthaltung, wenn die Rechnung gar nicht erst laufen kann (R4).
///
/// Sie behauptet KEINEN Ort und KEINEN Wert — und schweigt trotzdem nicht.
/// M-27 nennt „mehr Daten noetig" ein regulaeres Ergebnis; bis NAK-213 endete
/// `hypothesen()` in dieser Lage vor den Gates, und die Sitzung SCHWIEG
/// (Gate-Befunde G-D4, G-L4).
///
/// 🔑 **R8 (Dirigent, 08.09.2026):** sie setzt MINDESTENS EINEN gueltigen
/// Beleg der Sitzung voraus. `evidence_ids` traegt `minItems: 1`
/// (`eq-ipc-v3.schema.json`:1889), und der Eintragungsriegel laesst nur nicht
/// zurueckgenommene IDs zu (M-28). Ohne einen solchen Beleg kann die
/// Pflichtliste gar nicht gueltig gefuellt werden — dann entsteht KEIN
/// Befund, auch keine Enthaltung. Die bestehenden Befunde der Sitzung sind zu
/// diesem Zeitpunkt ueber den unveraenderten M-24-Weg schon fort.
///
/// `None` heisst deshalb: die Sitzung ist still, weil nichts gemessen ist —
/// nicht, weil etwas verschwiegen wuerde.
fn enthaltung_ohne_ort(aufnahme: &Aufnahme, grund: Enthaltungsgrund) -> Option<CauseHypothesis> {
    // Die deterministische Adresse der Sitzung: die lexikographisch kleinste
    // `instance_id` der `main`-Clients. Sie behauptet KEINE Fuehrung.
    let quelle = aufnahme.mains.first()?.quelle_id.clone();

    // Die juengsten GUELTIGEN Belege der Sitzung, ueber ALLE Quellen. Die
    // Fensterfolgen sind bereits um zurueckgenommene Belege bereinigt
    // (`fenster_aus_historie`), und `empfangsfolge` ordnet sie.
    //
    // 🔑 **Nacharbeit 1 (08.09.2026, Erstpruefungsbefund 1):** gesammelt wird
    // ueber `mains`, nicht ueber `master`. In der Lage, fuer die
    // `KeineEindeutigeFuehrung` steht, ist `master` per Definition `None` —
    // eine Sammlung ueber ihn kann dort gar nichts finden, und eine Sitzung
    // mit gueltig messendem Main schwieg. `mains` traegt die Profile ALLER
    // Main-Clients und schliesst den Master ein, wenn es einen gibt (er ist
    // ein Klon daraus); doppelte IDs faengt der `dedup_by` unten.
    let mut belege: Vec<(u64, &str)> = aufnahme
        .kandidaten
        .iter()
        .chain(aufnahme.mains.iter())
        .flat_map(|q| {
            q.fenster
                .iter()
                .map(|f| (f.empfangsfolge, f.evidence_id.as_str()))
        })
        .collect();
    belege.sort();
    belege.dedup_by(|a, b| a.1 == b.1);
    // Der Vertrag deckelt bei 32. Gekappt wird am ALTEN Ende, wie im Befund:
    // die juengsten Belege sind die, um die es geht.
    if belege.len() > 32 {
        let ab = belege.len() - 32;
        belege.drain(..ab);
    }
    let evidence_ids: Vec<String> = belege.iter().map(|(_, id)| (*id).to_string()).collect();
    // R8: ohne einen einzigen gueltigen Beleg entsteht kein Befund.
    if evidence_ids.is_empty() {
        return None;
    }

    let metrik = Zielmetrik::BandPegelDb;
    // Das VOLLE Gitter heisst „nicht eingegrenzt", nicht „ueberall". Ein
    // Teilintervall waere eine Ortsbehauptung ohne Messung.
    let band = Bandintervall {
        von: 0,
        bis: BAENDER_FEIN as u32,
    };
    Some(CauseHypothesis {
        finding_id: finding_id(aufnahme, metrik, band, &quelle),
        claim_class: Aussageklasse::Zusammenhang,
        ursachenklasse: Ursachenklasse::DatenReichenNicht,
        target_metric: metrik,
        candidate_source: quelle,
        passage_id: aufnahme.passage_id.clone(),
        pre_post: None,
        band_hz: band,
        // Dasselbe Muster wie NAK-212 E5/N-29: ohne das Bit ist die Zahl keine
        // Messung. Erst BEIDE Felder zusammen sind eindeutig.
        beobachtung: Beobachtung {
            wert_db: 0.0,
            gueltig: false,
        },
        rang: Rangkomponenten::default(),
        confidence: Befundkonfidenz {
            klasse: Sicherheitsklasse::Unklar,
            score: 0.0,
        },
        evidence_ids,
        alternatives: Vec::new(),
        ausschluesse: Vec::new(),
        next_test: NaechsterTest::MehrDatenSammeln,
        zustand: zustand_aus_sicherheit(Sicherheitsklasse::Unklar, false),
        intent_revision: aufnahme.intent.as_ref().map_or(0, |i| i.revision),
        intent_generation: aufnahme.intent.as_ref().map_or(0, |i| i.generation),
        likely_cause: grund.satz().into(),
        smallest_test: NaechsterTest::MehrDatenSammeln.satz().into(),
        listen_for: "Noch nichts — erst mehr Material sammeln.".into(),
        maskierung: None,
        metrics_version: aufnahme.metrics_version,
    })
}

/// Der Befund, wenn kein Kandidat ueberlebt hat (M-27).
fn enthaltung(
    aufnahme: &Aufnahme,
    master: &Quellprofil,
    metrik: Zielmetrik,
    band: Bandintervall,
    beobachtung: Beobachtung,
    ausschluesse: Vec<Ausschluss>,
) -> CauseHypothesis {
    let quelle = master.quelle_id.clone();
    let finding_id = finding_id(aufnahme, metrik, band, &quelle);
    CauseHypothesis {
        finding_id,
        claim_class: Aussageklasse::Zusammenhang,
        ursachenklasse: Ursachenklasse::DatenReichenNicht,
        target_metric: metrik,
        candidate_source: quelle,
        passage_id: aufnahme.passage_id.clone(),
        pre_post: None,
        band_hz: band,
        beobachtung,
        rang: Rangkomponenten::default(),
        confidence: Befundkonfidenz {
            klasse: Sicherheitsklasse::Unklar,
            score: 0.0,
        },
        evidence_ids: master
            .fenster
            .iter()
            .map(|f| f.evidence_id.clone())
            .collect(),
        alternatives: Vec::new(),
        ausschluesse,
        next_test: NaechsterTest::MehrDatenSammeln,
        zustand: zustand_aus_sicherheit(Sicherheitsklasse::Unklar, false),
        intent_revision: aufnahme.intent.as_ref().map_or(0, |i| i.revision),
        // E-14/WN-02: die Generation des Bestands, aus dem die Aufnahme kam.
        intent_generation: aufnahme.intent.as_ref().map_or(0, |i| i.generation),
        likely_cause: "Die Datenlage traegt noch keine Ursachenaussage.".into(),
        smallest_test: NaechsterTest::MehrDatenSammeln.satz().into(),
        listen_for: "Noch nichts — erst mehr Material sammeln.".into(),
        // Eine Enthaltung hat keinen Kandidaten und damit kein Paar, ueber
        // dessen Maskierung sich etwas sagen liesse.
        maskierung: None,
        metrics_version: aufnahme.metrics_version,
    }
}

#[allow(clippy::too_many_arguments)]
fn baue_befund(
    aufnahme: &Aufnahme,
    master: &Quellprofil,
    kandidat: &Quellprofil,
    rang: Rangkomponenten,
    zusammenhang: &Zusammenhangsbeleg,
    metrik: Zielmetrik,
    band: Bandintervall,
    beobachtung: Beobachtung,
    parent_duplikat: bool,
    mehrere_kandidaten: bool,
    lage: Konkurrenzlage,
    getrennt: bool,
) -> CauseHypothesis {
    // M-17: aus paralleler Telemetrie allein entsteht NIE Klasse 2 oder 3.
    // `Ursachenbeleg` verlangt eine kontrollierte Preview — die es in P5
    // nicht gibt; dieses Modul kann den Wert deshalb gar nicht erzeugen.
    //
    // 🔑 **NAK-214 R2/R6:** die Aussageklasse haengt an der WIRKUNGSmarke,
    // nicht an der blossen Paarexistenz. `Probable`-Paare, Paare ohne ein
    // einziges gemessenes Band und identische PRE/POST-Paare tragen sie
    // nicht — sie bleiben Klasse 1.
    let claim_class = if kandidat.prepost_wirkungsbeleg {
        Aussageklasse::Wirkungsbeleg
    } else {
        Aussageklasse::Zusammenhang
    };
    let ursachenklasse = if mehrere_kandidaten {
        Ursachenklasse::ZweiQuellenKonkurrenz
    } else if kandidat.prepost_wirkungsbeleg {
        // Dieselbe Aussage in der Ursachenachse: „die Ursache ist die KETTE
        // dieser Quelle". Sie ohne Wirkungsbeleg zu behaupten waere derselbe
        // Bruch wie bei der Aussageklasse (NAK-214 E4).
        Ursachenklasse::EffektkettePrePost
    } else {
        Ursachenklasse::QuelleResonanz
    };
    // 🔑 NR-05 (Nacharbeit 1, 07.09.2026): fuehrt die Sitzung eine Passage,
    // zaehlt das Material BEIDER beteiligter Quellen darin — sonst das des
    // Kandidaten ueber seine ganze Historie.
    //
    // Ohne die Passagenform stuende hier eine Zahl, die das Passagengate
    // bereits als zu duenn verworfen haette, und die Sicherheit `hoch` haette
    // eine andere Grundlage als der Ausschlussgrund `passage_zu_kurz`.
    let fenster = match aufnahme.passage.as_ref() {
        Some(p) => passagenmaterial(kandidat, master, p),
        None => unabhaengige_fenster(&kandidat.fenster.iter().collect::<Vec<_>>()),
    };
    // Die Klasse entsteht STRUKTURELL, nicht aus dem Score (M-15). Die
    // numerische Schwelle hoch/mittel/unklar ist Ausgabe des Korpus
    // (M-31, Etappe H) und steht bewusst nirgends als Konstante.
    //
    // ⚠️ Die Sicherheit haengt AUSDRUECKLICH nicht an der Aussageklasse.
    // Beides sind zwei Achsen: `claim_class` sagt, WIE STARK die Art des
    // Belegs ist (Zusammenhang, Wirkungsbeleg, Ursachenbeleg), `confidence`
    // sagt, wie sicher DIESE Aussage ist. Wer die Sicherheit `hoch` an ein
    // PRE/POST-Paar knuepfte, machte sie in einer rein passiven Sitzung
    // strukturell unerreichbar — und genau dort verlangt M-23 ihren
    // Rotbeweis: „dieselbe Session mit einer Passage UNTER
    // `GATE_MINDEST_FENSTER` liefert READY TO SEND" ist der gebrochene
    // Zustand, also muss dieselbe Session MIT genug Fenstern ihn erreichen.
    //
    // 🔑 NAK-212 R1/R2/R3 (07.09.2026): vier Terme kommen dazu, und alle vier
    // sind PRAEDIKATE UEBER EINE BENANNTE STRUKTUR, kein Vergleich gegen eine
    // kalibrierte Zahl. `positiv_belegt()` fragt „ist eine Komponente UEBER
    // NULL", nicht „ueber 0,3"; die Vorzeichengrenze null ist die Definition
    // von „positiv" und steht deshalb nirgends als abstimmbarer Wert. M-15
    // („die Klasse wird nicht aus dem Score gerundet") und M-31 („Schwelle
    // kalibriert, nie geraten") bleiben unberuehrt — dieselbe Lesart, die
    // schon `parent_duplikat` und `routing_bekannt` tragen.
    let bandpassung_ist_null = !(rang.bandpassung > 0.0);
    let klasse = if fenster < GATE_MINDEST_FENSTER
        || !kandidat.routing_bekannt
        || parent_duplikat
        // R1 (a): keine gemessene Energie im Befundband. Gemeint ist ein
        // Kandidat, dessen Baender im Intervall KEIN Gueltigkeitsbit tragen —
        // nicht einer mit wenig Energie. Fail-closed gegen NaN, dieselbe
        // negierte Form wie in `gate`.
        || bandpassung_ist_null
        // R1 (b): kein Beleg fuer einen Zusammenhang. Entwurf §36.1 Klasse 1
        // verlangt „Quelle und Masterproblem treten im selben Bereich UND
        // Zeitfenster auf" — ohne bedingten Uplift (M-19) und ohne
        // Onset-Koinzidenz ist davon nichts gemessen.
        || !zusammenhang.positiv_belegt()
        // R2: ein Gegenbeleg schliesst `hoch` aus. Der Kandidat bleibt
        // sichtbar und wird als Alternative gefuehrt — er ist nicht
        // widerlegt, er spricht dagegen.
        || zusammenhang.gegenbeleg()
        // R3/E6: ein ungetrennter erster Platz behauptet eine Unterscheidung,
        // die die Messung nicht hergibt.
        || !getrennt
        // 🔑 NAK-213 R1 (E2) und R2 (E4): die Konkurrenz dieser Rechnung ist
        // unvollstaendig. Entweder wurde am Deckel ohne Abstand geschnitten,
        // oder ein Konkurrent ist aus einem MESSGRUND ausgeschieden — in
        // beiden Lagen hat sich der Fuehrende nicht gegen die Konkurrenz
        // behauptet, sondern sie nur ueberlebt. Die Lage gilt der RECHNUNG
        // und damit jedem Befund aus ihr (K-21).
        || lage != Konkurrenzlage::Vollstaendig
    {
        // M-22/M-23: unbekanntes Routing, ein Parent-Duplikat oder zu wenig
        // Material tragen keine STARKE Aussage.
        Sicherheitsklasse::Mittel
    } else {
        Sicherheitsklasse::Hoch
    };
    // M-29: die EINE Abbildung im Datenweg. Frisch gerechnet ist ein Befund
    // nie veraltet — `stale` entsteht erst, wenn sich unter ihm etwas aendert
    // (Intent-Revision gestiegen, Belege zurueckgenommen).
    let zustand = zustand_aus_sicherheit(klasse, false);
    let next_test = if !kandidat.routing_bekannt {
        NaechsterTest::RoutingBestaetigen
    } else if !kandidat.prepost_wirkungsbeleg {
        // NAK-214 R2 woertlich: „`next_test` verlangt das Paar weiterhin".
        // Das gilt bei fehlendem Paar, bei einem Paar unterhalb der Schwelle
        // UND bei einem Paar ohne gemessene Veraenderung — der naechste
        // Beweisschritt ist in allen dreien derselbe (E5, NB-2).
        NaechsterTest::PrePostPaarMessen
    } else if fenster < GATE_MINDEST_FENSTER {
        NaechsterTest::PassageMessen
    } else {
        NaechsterTest::ManuellerVersuch
    };
    let mut evidence_ids: Vec<String> = kandidat
        .fenster
        .iter()
        .map(|f| f.evidence_id.clone())
        .collect();
    evidence_ids.dedup();
    // Der Vertrag deckelt bei 32 (`session_finding.evidence_ids.maxItems`).
    // Gekappt wird am ALTEN Ende, wie beim Snapshot: die juengsten Belege
    // sind die, um die es geht.
    if evidence_ids.len() > 32 {
        let ab = evidence_ids.len() - 32;
        evidence_ids.drain(..ab);
    }
    CauseHypothesis {
        finding_id: finding_id(aufnahme, metrik, band, &kandidat.quelle_id),
        claim_class,
        ursachenklasse,
        target_metric: metrik,
        candidate_source: kandidat.quelle_id.clone(),
        passage_id: aufnahme.passage_id.clone(),
        pre_post: kandidat.prepost_paar.then_some("post"),
        band_hz: band,
        beobachtung,
        rang,
        confidence: Befundkonfidenz {
            klasse,
            score: rang.rang(),
        },
        evidence_ids,
        alternatives: Vec::new(),
        ausschluesse: Vec::new(),
        next_test,
        zustand,
        intent_revision: aufnahme.intent.as_ref().map_or(0, |i| i.revision),
        // E-14/WN-02: die Generation des Bestands, aus dem die Aufnahme kam.
        intent_generation: aufnahme.intent.as_ref().map_or(0, |i| i.generation),
        likely_cause: format!(
            "{} draengt im Bandbereich {}..{} gegen den Master.",
            kurz(&kandidat.quelle_id),
            band.von,
            band.bis
        ),
        // M-34: die Zeile BILDET `next_test` ab; die Abbildung lebt am Enum.
        smallest_test: next_test.satz().into(),
        listen_for: "Ob der Master im markierten Bereich Luft bekommt.".into(),
        // M-36/M-41: der Wert faellt AUS dem Befund heraus — er entsteht mit
        // ihm, aus denselben zwei Quellen und demselben Bandbereich, und es
        // gibt keinen Weg, ihn ohne Befund zu bekommen.
        maskierung: super::maskierung::maskierung(master, kandidat, band),
        metrics_version: aufnahme.metrics_version,
    }
}

/// Die deterministische `finding_id` (M-25): gleicher Eingang, gleiche ID.
///
/// Sie entsteht aus Sitzung, Metrik, Band, Quelle und Kalibrierungsfassung —
/// nicht aus einem Zaehler und nicht aus der Zeit. Ein Zaehler machte zwei
/// Laeufe desselben Standes unterscheidbar, und genau das schliesst M-25 aus.
pub fn finding_id(
    aufnahme: &Aufnahme,
    metrik: Zielmetrik,
    band: Bandintervall,
    quelle: &str,
) -> String {
    use sha2::{Digest, Sha256};
    let mut hasher = Sha256::new();
    hasher.update(b"nakama.sonde014.finding.v1\0");
    hasher.update(aufnahme.session_epoch.as_bytes());
    hasher.update(b"\0");
    hasher.update(metrik.wire().as_bytes());
    hasher.update(b"\0");
    hasher.update(band.von.to_be_bytes());
    hasher.update(band.bis.to_be_bytes());
    hasher.update(quelle.as_bytes());
    hasher.update(b"\0");
    hasher.update(aufnahme.metrics_version.to_be_bytes());
    let digest = hasher.finalize();
    digest[..16].iter().map(|b| format!("{b:02x}")).collect()
}

fn kurz(id: &str) -> String {
    id.chars().take(8).collect()
}

// ═════════════════════════════════════════════════════════════════════════
// Modulebene (§5.3 R1)
// ═════════════════════════════════════════════════════════════════════════
//
// Diese Faelle rufen das Modul DIREKT und koennen deshalb nicht fallen, wenn
// die Verdrahtung fehlt — das ist ausdruecklich ihre Grenze. Sie messen die
// Zahlenraender, an die ein Verdrahtungsfall nicht herankommt: Bandindex 0
// und 220, leere Reihen, NaN, das Fenster GENAU an der Schwelle. Der
// Rotbeweis faellt am Verdrahtungsbein `broker/tests/sonde014_hypothese.rs`.
#[cfg(test)]
mod tests {
    use super::testhilfe::*;
    use super::*;
    use crate::coordinator::intent::IntentBestand;

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

    /// Die `finding_id` ist eine Funktion ihres Eingangs — und nur davon.
    #[test]
    fn finding_id_ist_deterministisch_und_trennt() {
        let mut aufnahme = Aufnahme {
            session_epoch: "22".into(),
            metrics_version: 20_260_904,
            ..Default::default()
        };
        let band = Bandintervall { von: 10, bis: 14 };
        let a = finding_id(&aufnahme, Zielmetrik::BandPegelDb, band, "q1");
        let b = finding_id(&aufnahme, Zielmetrik::BandPegelDb, band, "q1");
        assert_eq!(a, b, "gleicher Eingang, gleiche ID");
        assert_eq!(a.len(), 32, "hex32");
        assert!(a.chars().all(|c| c.is_ascii_hexdigit() && !c.is_uppercase()));
        assert_ne!(a, finding_id(&aufnahme, Zielmetrik::BandPegelDb, band, "q2"));
        assert_ne!(
            a,
            finding_id(&aufnahme, Zielmetrik::BandSpanneDb, band, "q1")
        );
        assert_ne!(
            a,
            finding_id(
                &aufnahme,
                Zielmetrik::BandPegelDb,
                Bandintervall { von: 10, bis: 15 },
                "q1"
            )
        );
        aufnahme.session_epoch = "23".into();
        assert_ne!(
            a,
            finding_id(&aufnahme, Zielmetrik::BandPegelDb, band, "q1"),
            "zwei Sitzungen teilen keine ID"
        );
        aufnahme.session_epoch = "22".into();
        aufnahme.metrics_version = 20_260_905;
        assert_ne!(
            a,
            finding_id(&aufnahme, Zielmetrik::BandPegelDb, band, "q1"),
            "eine andere Kalibrierung ist ein anderer Befund"
        );
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

    /// Die sechs Teile aus §8 sind sechs, in ihrer Reihenfolge, und jeder
    /// zeigt auf eine Stelle im Objekt.
    #[test]
    fn sechs_teile_sind_sechs_und_benannt() {
        let befund = CauseHypothesis {
            finding_id: "a".repeat(32),
            claim_class: Aussageklasse::Zusammenhang,
            ursachenklasse: Ursachenklasse::QuelleResonanz,
            target_metric: Zielmetrik::BandPegelDb,
            candidate_source: "q".into(),
            passage_id: None,
            pre_post: None,
            band_hz: Bandintervall { von: 0, bis: 4 },
            beobachtung: Beobachtung {
                wert_db: -12.0,
                gueltig: true,
            },
            rang: Rangkomponenten::default(),
            confidence: Befundkonfidenz {
                klasse: Sicherheitsklasse::Mittel,
                score: 0.5,
            },
            evidence_ids: vec!["b".repeat(32)],
            alternatives: Vec::new(),
            ausschluesse: Vec::new(),
            next_test: NaechsterTest::PassageMessen,
            zustand: Befundzustand::MoreData,
            intent_revision: 0,
            intent_generation: 0,
            likely_cause: "x".into(),
            smallest_test: "y".into(),
            listen_for: "z".into(),
            maskierung: None,
            metrics_version: 1,
        };
        let teile = befund.sechs_teile();
        assert_eq!(teile.len(), 6);
        for (i, (name, vorhanden)) in teile.iter().enumerate() {
            assert_eq!(*name, ERGEBNISTEILE[i]);
            assert!(vorhanden, "{name}");
        }
        // Nimmt man einen Teil heraus, faellt genau er — und nicht die
        // Nachbarn.
        let mut ohne_ort = befund.clone();
        ohne_ort.candidate_source = String::new();
        assert!(!ohne_ort.sechs_teile()[0].1);
        assert!(ohne_ort.sechs_teile()[1].1);
        let mut ohne_beobachtung = befund.clone();
        ohne_beobachtung.band_hz = Bandintervall { von: 4, bis: 4 };
        assert!(!ohne_beobachtung.sechs_teile()[1].1);

        // Der Tie-Break ist total geordnet und haengt am Rang, nicht an der
        // Reihenfolge des Aufrufers.
        let mut stark = befund.clone();
        stark.rang.bandpassung = 1.0;
        stark.candidate_source = "z".into();
        assert!(stark.tie_break() < befund.tie_break(), "hoeherer Rang zuerst");
    }
}
