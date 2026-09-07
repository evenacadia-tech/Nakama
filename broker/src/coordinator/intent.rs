//! Empfaenger und **Spiegel** des musikalischen Intents (SONDE-014 E-08/E-10).
//!
//! ── WER HIER DIE WAHRHEIT HAELT, UND WER NICHT ────────────────────────────
//!
//! Entscheid **E-08** legt die Autoritaet fest, und sie liegt NICHT hier:
//! Rollenmenge, Zyklenpruefung und Konfliktaufloesung beim Setzen laufen im
//! **Main**, also im Gen-Plugin in C++, persistent im `MainProjectState`
//! (§33.5). Dieses Modul ist **Spiegel und Vertragsvalidierung beim Empfang**
//! (M-77: dieselbe Rollenmenge, dieselben Enumgrenzen beidseitig) sowie die
//! Auswertung der Konfliktregeln beim **Rechnen** — keine zweite
//! Zustandsmaschine und keine zweite Wahrheit.
//!
//! ── DIE VOLLSTAENDIGKEITSMARKE (M-86) ─────────────────────────────────────
//!
//! Der schaerfste Teil dieses Moduls ist eine Sperre, keine Rechnung: **bis
//! der vollstaendige Bestand mit seiner Marke vorliegt, entsteht kein
//! Proposal.** Grund, woertlich aus E-10: ein Veto — `geschuetzt`, `bewusst
//! verschmolzen` — darf durch Transportverlust nie verschwinden. Ein fehlender
//! Intent saehe sonst aus wie „kein Schutz gewuenscht", und genau das ist der
//! teuerste Irrtum, den dieser Datenweg machen kann.
//!
//! ── WAS DAS SCHEMA NICHT SAGEN KANN ───────────────────────────────────────
//!
//! Die Schluesselwortmenge des v3-Vertrags ist geschlossen und kennt keinen
//! Feldvergleich (`schemas/v3/README.md`). Sechs Regeln stehen deshalb dort
//! in der Consumer-Tabelle und werden HIER durchgesetzt:
//!
//!   1. `bandintervall.von < bis`;
//!   2. `intent_schutz.band` genau dann, wenn `eigenschaft == "band"`;
//!   3. `quelle_a != quelle_b`, je geordnetem Paar genau eine Beziehung;
//!   4. der `fuehrt_vor`-Teilgraph ist zyklenfrei (§37.4);
//!   5. bei `vollstaendig == false` genau EIN Eintrag in genau EINER Liste;
//!   6. Inhalt verlangt `bestand_revision >= 1`.
//!
//! Eine Regel in dieser Liste ist keine Abschwaechung: sie ist genauso
//! bindend wie eine Schemazeile, nur an einer anderen Stelle gemessen.

use super::*;
use std::collections::BTreeMap;

/// Ein gespiegelter `SourceIntent`. Der Broker haelt ihn, um zu RECHNEN —
/// Ranking, Schutz und erlaubte Tests (§37.3). Er schreibt ihn nie zurueck.
#[derive(Debug, Clone, PartialEq)]
pub struct SourceIntentSpiegel {
    pub quelle_id: String,
    /// Leer = globaler Scope. §37.2 Stufe 3: passagespezifisch vor global.
    pub passage_id: String,
    pub rolle: String,
    pub revision: i64,
    pub herkunft: String,
    pub konfidenz: f64,
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct SchutzangabeSpiegel {
    pub quelle_id: String,
    pub eigenschaft: String,
    /// Halboffen `[von, bis)` im 221-Band-Evidenzgitter; `None` bei allen
    /// Eigenschaften ausser `band`.
    pub band: Option<(u32, u32)>,
}

/// Der gespiegelte Bestand EINER Sitzung.
#[derive(Debug, Clone, Default)]
pub struct IntentBestand {
    /// Revision des GANZEN Bestands. 0 heisst „nie etwas gesetzt".
    ///
    /// Sie wird als MAXIMUM fortgeschrieben (Nacharbeit 1, NR-02): ein
    /// Teilupdate, das nach der Koaleszierung verspaetet ankommt, senkt sie
    /// nicht. Sie ordnet Vollberichte; Teilberichte ordnet
    /// `teilrevisionen`.
    pub revision: i64,
    /// Die Vollstaendigkeitsmarke aus M-86. Solange sie fehlt, rechnet der
    /// Broker nicht.
    pub vollstaendig: bool,
    /// Schluessel: (Quelle, Scope). Zwei Passagen derselben Quelle sind zwei
    /// Objekte und verdraengen sich nicht (M-08).
    pub intents: BTreeMap<(String, String), SourceIntentSpiegel>,
    pub schutzangaben: BTreeSet<SchutzangabeSpiegel>,
    /// Schluessel: das GEORDNETE Paar. Je Paar genau eine Art.
    pub beziehungen: BTreeMap<(String, String), String>,
    /// **NR-02 (Nacharbeit 1, 07.09.2026): die Revisionsordnung JE
    /// KOALESZIERUNGSOBJEKT.**
    ///
    /// Die Groesse ist gedeckelt, ohne einen eigenen Deckel zu brauchen:
    /// jeder Schluessel gehoert genau einem Objekt aus `intents`,
    /// `schutzangaben` oder `beziehungen`, und alle drei stehen unter
    /// `INTENT_DECKEL`. Ein Vollbericht leert sie.
    ///
    /// Der Sender koalesziert je `(quelle_id, scope)` — `intent:<quelle>:
    /// <scope>` in `PluginProcessor::sendeIntentFortschreibung`. Wer die
    /// Ablehnung an der GLOBALEN Bestandsrevision festmacht, verwirft ein
    /// fremdes Objekt vollstaendig: Ankunft A/1, B/2, A/3 laesst B/2 an
    /// A/3 scheitern, und die geschuetzte Rolle einer anderen Quelle fehlt
    /// dauerhaft. Deshalb steht hier die zuletzt UEBERNOMMENE Revision je
    /// Objekt; ein Objekt ohne eigenen Eintrag misst sich an
    /// `grundrevision`.
    pub teilrevisionen: BTreeMap<String, i64>,
    /// Die Revision des letzten VOLLBERICHTS. Er ersetzt den Bestand
    /// vollstaendig und ist damit der Grundstand jedes Objekts, das danach
    /// noch nicht einzeln fortgeschrieben wurde.
    pub grundrevision: i64,
    /// **E-14 / WN-02 (Nacharbeit 2, 07.09.2026): die BESTANDSGENERATION.**
    ///
    /// Sie steigt bei JEDER im Broker wirksam uebernommenen Bestandsaenderung
    /// — Vollbericht wie Teilbericht —, unabhaengig davon, welche Zahl der
    /// Sender gezaehlt hat. Genau das ist der Unterschied, der die Luecke
    /// WP1-2 schliesst: nach Vollbestand/0 → A/3 → Befund → verspaetetem B/2
    /// mit `rolle=geschuetzt` bleibt die Sitzungsrevision 3, der Bestand hat
    /// sich aber geaendert. An der Sender-Revision gemessen blieb der READY-
    /// Befund handelbar, obwohl ein hartes Veto inzwischen uebernommen war.
    ///
    /// Maszgeblich ist sie fuer ZWEI Fragen: das Veralten
    /// (`befunde_veralten_locked`) und die Eintragungspruefung
    /// (`ergebnis_ist_noch_gueltig`, NR-03). Die Sender-Revision bleibt
    /// allein die ORDNUNG der Uebernahme (NR-02) und die Zahl, die als
    /// `intent_revision` am Befund ueber den Draht reist.
    ///
    /// Sie ist fluechtig wie der ganze Spiegel: nach einem Brokerneustart
    /// meldet der Main seinen Vollbestand, und der ist die erste Generation.
    pub generation: i64,
}

/// **M-86, der EINE Riegel: fail-closed.**
///
/// `None` sperrt genauso wie ein unvollstaendiger Bestand. Bis zur
/// Nacharbeit 1 stand die Bedingung zweimal im Code — je einmal in
/// `aufnahmen_sammeln` und in `hypothesen()` — und beide lauteten
/// `is_some_and(|i| !i.vollstaendig)`: bei `intent == None` ist das FALSCH,
/// und ohne jede Vollstaendigkeitsmeldung entstanden Findings und
/// Proposals. Genau der Irrtum, den E-10 als den teuersten dieses Datenwegs
/// benennt — ein fehlender Intent sieht aus wie „kein Schutz gewuenscht".
///
/// Die Frage steht deshalb an EINER Stelle, und jeder Rechner stellt sie
/// ueber diese Funktion.
pub fn darf_gerechnet_werden(intent: Option<&IntentBestand>) -> bool {
    intent.is_some_and(|b| b.vollstaendig)
}

/// Die fuenf Rollen, die dieser Leser kennt. Sie stehen hier NICHT als zweite
/// Wahrheit: `geschlossene_mengen_der_fassung_3_stehen_je_einmal_im_vertrag`
/// haelt sie gegen `$defs/intent_rolle` des eingefrorenen Schemas, und das
/// C++-Bein B27 misst dieselbe Menge auf seiner Seite (M-01, M-77).
pub(crate) const ROLLEN: [&str; 5] = [
    "fuehrt",
    "traegt",
    "begleitet",
    "geschuetzt",
    "verschmolzen",
];

impl IntentBestand {
    /// §37.2 Stufe 3: der WIRKENDE Intent einer Quelle in einer Passage.
    /// Ohne Passagenwert faellt er auf den globalen zurueck.
    pub fn wirkend(&self, quelle_id: &str, passage_id: &str) -> Option<&SourceIntentSpiegel> {
        if !passage_id.is_empty() {
            if let Some(s) = self
                .intents
                .get(&(quelle_id.to_string(), passage_id.to_string()))
            {
                return Some(s);
            }
        }
        self.intents.get(&(quelle_id.to_string(), String::new()))
    }

    /// E-02, M-04: darf fuer dieses Paar eine Entmaskierung empfohlen werden?
    ///
    /// Die Rolle `verschmolzen` an einer der beiden Quellen ist ein GLOBALES
    /// Veto; eine ausdrueckliche gerichtete Beziehung `fuehrt_vor` ist das
    /// SPEZIFISCHERE Werkzeug und hebt es NUR fuer dieses Paar auf. Dieselbe
    /// Regel wie in `NakamaState.cpp::entmaskierungErlaubt` — beide Seiten
    /// muessen sie kennen, sonst rechnet der Broker gegen den Willen des
    /// Users (M-77).
    pub fn entmaskierung_erlaubt(&self, a: &str, b: &str, passage_id: &str) -> bool {
        let veto = |q: &str| {
            self.wirkend(q, passage_id)
                .is_some_and(|s| s.rolle == "verschmolzen")
        };
        if !veto(a) && !veto(b) {
            return true;
        }
        self.beziehungen
            .get(&(a.to_string(), b.to_string()))
            .or_else(|| self.beziehungen.get(&(b.to_string(), a.to_string())))
            .is_some_and(|art| art == "fuehrt_vor")
    }

    /// M-03: traegt diese Quelle einen Schutz auf dieser Eigenschaft?
    pub fn geschuetzt(&self, quelle_id: &str, eigenschaft: &str) -> bool {
        self.schutzangaben
            .iter()
            .any(|s| s.quelle_id == quelle_id && s.eigenschaft == eigenschaft)
    }

    /// SONDE-014 M-87: greift ein Schutz dieser Quelle IN diesem Bandbereich?
    ///
    /// Der Unterschied zu `geschuetzt` ist der Ort: `geschuetzt` fragt, ob die
    /// Eigenschaft ueberhaupt geschuetzt ist, diese Funktion fragt, ob der
    /// Schutz das Band beruehrt, um das es gerade geht. Ein Attack-Schutz an
    /// einer Quelle darf einen Befund im Hochton nicht sperren — sonst waere
    /// jede Schutzangabe ein globales Veto, und M-03 fuehrt sie ausdruecklich
    /// ORTHOGONAL zur Rolle.
    ///
    /// Die drei bandlosen Eigenschaften (`attack`, `gewicht`, `breite`) tragen
    /// kein Intervall und gelten deshalb fuer die GANZE Quelle: wer sie
    /// schuetzt, schuetzt eine Eigenschaft ihres Klangs, nicht einen
    /// Ausschnitt ihres Spektrums.
    ///
    /// Halboffen `[von, bis)` auf beiden Seiten — zwei Intervalle beruehren
    /// sich nicht, wenn das eine dort endet, wo das andere beginnt.
    pub fn schutz_verletzt(&self, quelle_id: &str, von: u32, bis: u32) -> bool {
        self.schutzangaben.iter().any(|s| {
            s.quelle_id == quelle_id
                && match s.band {
                    Some((s_von, s_bis)) => s_von < bis && von < s_bis,
                    None => true,
                }
        })
    }
}

/// M-06: hat der `fuehrt_vor`-Teilgraph einen Zyklus?
///
/// Rein und iterativ. `gleichrangig` und `darf_verschmelzen` sind KEINE
/// Prioritaetskanten und zaehlen nicht mit — genau das macht die
/// Gleichrangigkeit zum gueltigen Ausweg aus einem Zyklus statt zu einer
/// zweiten Wahrheit. Die Kantenzahl ist eine EINGABE, deshalb keine Rekursion.
pub(super) fn hat_zyklus(kanten: &BTreeMap<(String, String), String>) -> bool {
    let mut nachfolger: BTreeMap<&str, Vec<&str>> = BTreeMap::new();
    for ((a, b), art) in kanten {
        if art == "fuehrt_vor" {
            nachfolger.entry(a.as_str()).or_default().push(b.as_str());
        }
    }
    #[derive(Clone, Copy, PartialEq)]
    enum Farbe {
        Grau,
        Schwarz,
    }
    let mut farbe: BTreeMap<&str, Farbe> = BTreeMap::new();
    let starts: Vec<&str> = nachfolger.keys().copied().collect();
    for start in starts {
        if farbe.contains_key(start) {
            continue;
        }
        let mut stapel: Vec<(&str, usize)> = vec![(start, 0)];
        farbe.insert(start, Farbe::Grau);
        while let Some((oben, i)) = stapel.pop() {
            let kinder = nachfolger.get(oben).map(Vec::as_slice).unwrap_or(&[]);
            if i >= kinder.len() {
                farbe.insert(oben, Farbe::Schwarz);
                continue;
            }
            stapel.push((oben, i + 1));
            let naechster = kinder[i];
            match farbe.get(naechster) {
                Some(Farbe::Grau) => return true,
                Some(Farbe::Schwarz) => {}
                None => {
                    farbe.insert(naechster, Farbe::Grau);
                    stapel.push((naechster, 0));
                }
            }
        }
    }
    false
}

/// Warum ein `intent_update` abgewiesen wurde. Gezaehlt, nicht verschwiegen.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum IntentAbweisung {
    Vertrag,
    FremdeAdresse,
    KeinLink,
    BandintervallLeer,
    SchutzOhneBand,
    SchutzMitFremdemBand,
    Selbstbeziehung,
    DoppeltesPaar,
    Zyklus,
    TeilmeldungNichtEinzeln,
    InhaltOhneRevision,
    AeltereRevision,
    Deckel,
}

/// Feste Obergrenze wie im Main (§48.1). Sie ist dieselbe Zahl wie
/// `maxSourceIntents` in `NakamaState.h`: ein Bestand, den der Main schreiben
/// kann, muss der Broker spiegeln koennen.
pub const INTENT_DECKEL: usize = 256;

fn hex32(wert: Option<&str>) -> Option<String> {
    let s = wert?;
    (s.len() == 32 && s.bytes().all(|b| b.is_ascii_digit() || (b'a'..=b'f').contains(&b)))
        .then(|| s.to_string())
}

fn band_aus(wert: &Value) -> Result<Option<(u32, u32)>, IntentAbweisung> {
    let Some(band) = wert.get("band") else {
        return Ok(None);
    };
    let von = band.get("von").and_then(Value::as_u64).unwrap_or(u64::MAX);
    let bis = band.get("bis").and_then(Value::as_u64).unwrap_or(0);
    if von >= bis {
        return Err(IntentAbweisung::BandintervallLeer);
    }
    Ok(Some((von as u32, bis as u32)))
}

impl Coordinator {
    /// Nimmt ein `intent_update` an oder verwirft es.
    ///
    /// Fail-closed in dieser Reihenfolge: Vertrag, Adresse, Link, dann die
    /// sechs Consumerregeln. Jede Stufe fuer sich ist ein Grund zu verwerfen;
    /// keine davon fuehrt zu einer abgeschwaechten Annahme.
    pub(super) fn intent_update_json_mit_minor(
        &self,
        link_id: &str,
        payload: &[u8],
        schema_minor: u8,
    ) -> Result<(), IntentAbweisung> {
        let Some(wert) = v3_nachricht_lesen_mit_minor(payload, "intent_update", schema_minor)
        else {
            return Err(IntentAbweisung::Vertrag);
        };
        let Ok(adresse) = serde_json::from_value::<Adresse>(wert["adresse"].clone()) else {
            return Err(IntentAbweisung::Vertrag);
        };

        // Alles, was nur den Payload betrifft, entsteht VOR dem Lock.
        let vollstaendig = wert["vollstaendig"].as_bool().unwrap_or(false);
        let revision = wert["bestand_revision"].as_i64().unwrap_or(-1);
        if revision < 0 {
            return Err(IntentAbweisung::Vertrag);
        }

        let leer: Vec<Value> = Vec::new();
        let liste = |name: &str| -> &[Value] {
            wert.get(name)
                .and_then(Value::as_array)
                .map(Vec::as_slice)
                .unwrap_or(&leer)
        };
        let intents = liste("intents");
        let schutz = liste("schutzangaben");
        let beziehungen = liste("beziehungen");

        // Regel 5: eine Teilmeldung traegt genau EINEN Eintrag in genau EINER
        // Liste. Der P1-Schluessel adressiert genau ein Objekt; eine
        // Sammelnachricht unter demselben Schluessel koennte fremde Objekte
        // verdraengen (M-85).
        if !vollstaendig {
            let belegt = [intents.len(), schutz.len(), beziehungen.len()];
            let gesamt: usize = belegt.iter().sum();
            if gesamt != 1 {
                return Err(IntentAbweisung::TeilmeldungNichtEinzeln);
            }
        }

        // Regel 6: Inhalt verlangt eine Revision. Ohne sie koennte der Broker
        // „vollstaendiger leerer Bestand" nicht von „nie gemeldet" trennen.
        if revision == 0 && !(intents.is_empty() && schutz.is_empty() && beziehungen.is_empty()) {
            return Err(IntentAbweisung::InhaltOhneRevision);
        }

        if intents.len() > INTENT_DECKEL
            || schutz.len() > INTENT_DECKEL
            || beziehungen.len() > INTENT_DECKEL
        {
            return Err(IntentAbweisung::Deckel);
        }

        let mut neue_intents: BTreeMap<(String, String), SourceIntentSpiegel> = BTreeMap::new();
        for eintrag in intents {
            let Some(quelle_id) = hex32(eintrag.get("quelle_id").and_then(Value::as_str)) else {
                return Err(IntentAbweisung::Vertrag);
            };
            let passage_id = eintrag
                .get("passage_id")
                .and_then(Value::as_str)
                .unwrap_or("")
                .to_string();
            let rolle = eintrag["rolle"].as_str().unwrap_or_default().to_string();
            if !ROLLEN.contains(&rolle.as_str()) {
                return Err(IntentAbweisung::Vertrag);
            }
            let objekt_revision = eintrag["revision"].as_i64().unwrap_or(0);
            let konfidenz = eintrag["konfidenz"].as_f64().unwrap_or(f64::NAN);
            if objekt_revision < 1 || !konfidenz.is_finite() {
                return Err(IntentAbweisung::Vertrag);
            }
            neue_intents.insert(
                (quelle_id.clone(), passage_id.clone()),
                SourceIntentSpiegel {
                    quelle_id,
                    passage_id,
                    rolle,
                    revision: objekt_revision,
                    herkunft: eintrag["herkunft"].as_str().unwrap_or_default().to_string(),
                    konfidenz,
                },
            );
        }

        let mut neue_schutzangaben: BTreeSet<SchutzangabeSpiegel> = BTreeSet::new();
        for eintrag in schutz {
            let Some(quelle_id) = hex32(eintrag.get("quelle_id").and_then(Value::as_str)) else {
                return Err(IntentAbweisung::Vertrag);
            };
            let eigenschaft = eintrag["eigenschaft"].as_str().unwrap_or_default().to_string();
            let band = band_aus(eintrag)?;
            // Regel 2: `band` genau dann, wenn die Eigenschaft `band` ist.
            match (eigenschaft.as_str(), band) {
                ("band", None) => return Err(IntentAbweisung::SchutzOhneBand),
                ("band", Some(_)) => {}
                (_, Some(_)) => return Err(IntentAbweisung::SchutzMitFremdemBand),
                (_, None) => {}
            }
            neue_schutzangaben.insert(SchutzangabeSpiegel {
                quelle_id,
                eigenschaft,
                band,
            });
        }

        let mut neue_beziehungen: BTreeMap<(String, String), String> = BTreeMap::new();
        for eintrag in beziehungen {
            let (Some(a), Some(b)) = (
                hex32(eintrag.get("quelle_a").and_then(Value::as_str)),
                hex32(eintrag.get("quelle_b").and_then(Value::as_str)),
            ) else {
                return Err(IntentAbweisung::Vertrag);
            };
            // Regel 3: zwei verschiedene Quellen, je Paar genau eine Art.
            if a == b {
                return Err(IntentAbweisung::Selbstbeziehung);
            }
            let art = eintrag["art"].as_str().unwrap_or_default().to_string();
            if neue_beziehungen.insert((a, b), art).is_some() {
                return Err(IntentAbweisung::DoppeltesPaar);
            }
        }

        // 🔑 NR-02 (Nacharbeit 1, 07.09.2026): der KOALESZIERUNGSSCHLUESSEL
        // einer Teilmeldung, aus dem einen Objekt abgeleitet, das sie traegt.
        //
        // Er bildet nach, was der Sender tut: `intent:<quelle>:<scope>` in
        // `PluginProcessor::sendeIntentFortschreibung`. Schutzangabe und
        // Beziehung reisen heute nur im Vollbestand; ihr Schluessel steht
        // trotzdem hier, weil Regel 5 sie als Teilmeldung ausdruecklich
        // zulaesst und ein Objekt ohne Schluessel sich sonst still an einem
        // fremden messen wuerde.
        let objektschluessel: Option<String> = if vollstaendig {
            None
        } else if let Some((quelle_id, passage_id)) = neue_intents.keys().next() {
            let scope = if passage_id.is_empty() { "global" } else { passage_id.as_str() };
            Some(format!("intent:{quelle_id}:{scope}"))
        } else if let Some(s) = neue_schutzangaben.iter().next() {
            Some(format!("schutz:{}:{}", s.quelle_id, s.eigenschaft))
        } else if let Some((a, b)) = neue_beziehungen.keys().next() {
            Some(format!("beziehung:{a}:{b}"))
        } else {
            // Regel 5 hat bereits genau EIN Objekt verlangt; hier anzukommen
            // hiesse, dass die Zaehlung und der Aufbau auseinanderlaufen.
            // Fail-closed: ohne Schluessel wird nichts uebernommen.
            return Err(IntentAbweisung::TeilmeldungNichtEinzeln);
        };

        // ── DER EINE LOCKABSCHNITT ──────────────────────────────────────
        let (session, veraltet) = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(link) = stand.links.get(link_id).cloned() else {
                return Err(IntentAbweisung::KeinLink);
            };
            // Dieselbe Kante wie beim `state_report` und beim
            // `evidence_snapshot`: die Adresse IM Payload muss die des
            // sendenden Links sein. Ein Peer schreibt sich seine Quelle nicht
            // selbst zu.
            if link.wire_adresse != adresse {
                return Err(IntentAbweisung::FremdeAdresse);
            }
            let session = link.client_key.session();
            let bestand = stand.intent.entry(session.clone()).or_default();

            // M-85: die Revisionsordnung ist TOTAL. Eine aeltere Revision
            // ueberschreibt nach der Koaleszierung nie eine juengere - das
            // ist der Kern dessen, was die Koaleszierung ueberhaupt zulaessig
            // macht.
            //
            // 🔑 NR-02 (Nacharbeit 1): die Ordnung gilt JE
            // KOALESZIERUNGSOBJEKT, nicht global. Der Sender koalesziert je
            // `(quelle_id, scope)`, und `P1Warteschlange::einreihen()`
            // ersetzt einen Eintrag AN SEINER POSITION — die Ankunft A/1,
            // B/2, A/3 ist damit normal. An der globalen Bestandsrevision
            // gemessen scheiterte B/2 an A/3 und die geschuetzte Rolle einer
            // FREMDEN Quelle fehlte dauerhaft im Broker.
            //
            // ⚠️ Fuer ein Objekt zaehlt GLEICH als nicht neuer. Dieselbe
            // Bestandsrevision zweimal fuer dasselbe Objekt ist entweder eine
            // Wiederholung (dann aendert das Verwerfen nichts) oder ein
            // Widerspruch (dann darf sie den uebernommenen Wert nicht
            // ersetzen). Der bestehende M-85-Fall misst genau diesen Rand.
            let zuletzt = match objektschluessel.as_deref() {
                None => bestand.revision,
                Some(key) => bestand
                    .teilrevisionen
                    .get(key)
                    .copied()
                    .unwrap_or(bestand.grundrevision),
            };
            let zu_alt = match objektschluessel {
                None => revision < zuletzt,
                Some(_) => revision <= zuletzt,
            };
            if zu_alt {
                return Err(IntentAbweisung::AeltereRevision);
            }

            // 🔑 E-14 / WN-02 (Nacharbeit 2): die Generation steigt bei JEDER
            // wirksamen Uebernahme — auch bei einem Vollbericht, der den
            // Spiegel ersetzt. Sie ueberlebt das Ersetzen deshalb
            // ausdruecklich: `..Default::default()` setzte sie sonst auf 0
            // zurueck, und ein Befund der vorigen Generation truege danach
            // dieselbe Zahl wie einer der neuen.
            let naechste_generation = bestand.generation.saturating_add(1);
            let mut kandidat = if vollstaendig {
                // Der vollstaendige Bestand ERSETZT den Spiegel. Ein
                // Verschmelzen liesse ein zurueckgenommenes Veto stehen —
                // und mit ihm faellt die ganze Revisionsbuchhaltung der
                // Teilobjekte: der Vollbericht ist ihr neuer Grundstand.
                IntentBestand {
                    revision,
                    vollstaendig: true,
                    grundrevision: revision,
                    generation: naechste_generation,
                    ..Default::default()
                }
            } else {
                let mut k = bestand.clone();
                // Die Sitzungsrevision ist das MAXIMUM. Ein verspaetetes
                // Teilupdate darf sie nicht senken; sonst liesse sich ein
                // spaeter eintreffender alter Vollbericht nicht mehr
                // abweisen.
                k.revision = revision.max(bestand.revision);
                k.generation = naechste_generation;
                k
            };
            kandidat.intents.extend(neue_intents);
            kandidat.schutzangaben.extend(neue_schutzangaben);
            kandidat.beziehungen.extend(neue_beziehungen);

            if kandidat.intents.len() > INTENT_DECKEL
                || kandidat.schutzangaben.len() > INTENT_DECKEL
                || kandidat.beziehungen.len() > INTENT_DECKEL
            {
                return Err(IntentAbweisung::Deckel);
            }
            // Regel 4: §37.4 woertlich - „Zyklische Entmaskierungsprioritaeten
            // koennen nicht angewendet werden". Ein Zyklus erreicht den
            // Spiegel nie; die Aufloesung gehoert dem Main (E-08).
            if hat_zyklus(&kandidat.beziehungen) {
                return Err(IntentAbweisung::Zyklus);
            }

            if let Some(key) = objektschluessel {
                kandidat.teilrevisionen.insert(key, revision);
            }
            *bestand = kandidat;
            // 🔑 SONDE-014 Etappe I (M-86 seit E-11): die Vollstaendigkeitsmarke
            // deckt BEIDE Bestaende — Intent UND aktuellen Schritt.
            //
            // Bis hierher deckte sie nur den Intent, und das riss ein Loch
            // derselben Klasse, die M-86 fuer den Intent schliesst: laedt der
            // Main ein anderes Projekt oder startet er neu, meldet er seinen
            // Vollbestand samt Marke — und hat er KEINEN offenen
            // Assistentenschritt, sendet er auch keinen. Der Spiegel trug dann
            // weiter den Schritt der VORIGEN Main-Generation, und eine Suche
            // oder Crashdiagnose haette einen Schritt gefunden, den es nicht
            // mehr gibt. Genau die Aussage, die §33.5 mit „der Spiegel ist nie
            // autoritativ" verbietet.
            //
            // Die Reihenfolge traegt das: der Main sendet den Vollbestand
            // ZUERST und den Schritt unmittelbar danach
            // (`PluginProcessor::sendeIntentVollbestand`), beide als P1 auf
            // demselben Link. Was nach der Marke kommt, gehoert dieser
            // Generation; was davor stand, ist fort.
            if vollstaendig {
                stand.assistent.remove(&session);
            }
            stand.intent_updates = stand.intent_updates.saturating_add(1);
            // 🔑 SONDE-014 Etappe D (§37.3, M-29): eine gestiegene
            // Bestandsrevision macht jeden Befund darunter SICHTBAR `stale`.
            //
            // §37.3 woertlich: „Steigt sie, geht der Befund sichtbar in
            // `stale`, ohne dass Zahlen nachgerechnet werden." Genau das
            // passiert hier — kein Neurechnen, nur der Zustand. Der Grund ist
            // derselbe wie bei der Evidenzruecknahme: eine Aussage, die unter
            // einer alten Absicht entstanden ist, darf nicht weiter als
            // handelbar dastehen, nur weil niemand sie angefasst hat.
            //
            // UNTER demselben Lock wie die Uebernahme des Bestands. Laege es
            // ausserhalb, koennte eine Neurechnung dazwischen den Befund mit
            // der NEUEN Revision aufbauen und der Veraltungsschritt liefe ins
            // Leere.
            // 🔑 E-14 / WN-02 (Nacharbeit 2, 07.09.2026): das Veralten haengt
            // an der UEBERNAHME, nicht an der Sender-Zahl.
            //
            // Bis hierher stand hier `if revision_gestiegen`, und der
            // Gegenzweig hielt woertlich fest, ein verspaetetes Teilupdate
            // veralte NICHT — §37.3 binde `stale` an eine gestiegene
            // Revision. Genau daraus entstand WP1-2: nach Vollbestand/0 →
            // A/3 → Befund fuer B → verspaetetem B/2 mit `rolle=geschuetzt`
            // wurde das harte Veto uebernommen, die Sitzungsrevision blieb 3,
            // und der READY-Befund stand weiter handelbar da.
            //
            // E-14 entscheidet das: maszgeblich ist JEDE wirksam uebernommene
            // Bestandsaenderung. Die Generation ist soeben gestiegen — jeder
            // Befund einer aelteren Generation geht sichtbar in `stale`,
            // OHNE Nachrechnen (§37.3 woertlich). Die Sender-Revision bleibt
            // allein die Ordnung der Uebernahme (NR-02).
            let veraltet =
                Coordinator::befunde_veralten_locked(&mut stand, &session, naechste_generation);
            // Und die Sitzung wird als neu zu rechnen gefuehrt: beim naechsten
            // Material ruht die Aussage auf dem vollstaendigen Bestand.
            stand.befunde_neu_bilden = true;
            (session, veraltet)
        };

        // 🔑 NR-11 (Nacharbeit 1, 07.09.2026), M-71: der versionierte
        // SQLite-Spiegel des Intents.
        //
        // §33.5 sagt fuer `SourceIntent` einen „versionierten SQLite-Spiegel
        // fuer Suche und Crashdiagnose" zu — und bis hierher gab es ihn nur
        // fuer den AssistantStep. Eine angenommene `intent_update` aenderte
        // ausschliesslich `stand.intent`, und nach einem Vollbericht oder
        // einer Rollenaenderung enthielt SQLite keinen Bestandsspiegel.
        //
        // Derselbe Ein-Writer-Append wie `assistent.rs::schritt_spiegeln`:
        // kein neuer Writer, keine neue Tabelle, `event_type =
        // "intent_update"` und ausdruecklich KEINE Domaenenprojektion in
        // `writer.rs` — es darf keinen zweiten autoritativen Ort geben. Der
        // Spiegel ist nie autoritativ (M-89-Analogie): nach einem Neustart
        // meldet der Main seinen Vollbestand, und der gewinnt.
        //
        // Er steht AUSSERHALB des Locks, wie der Push: ein Store-Append haelt
        // sonst den ganzen Sessiongraphen an.
        self.intent_spiegeln(&session);

        // Der Push liegt AUSSERHALB des Locks (M-72-Muster: der Sessiongraph
        // wird fuer keine Zustellung angehalten).
        if veraltet > 0 {
            {
                let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
                stand.dirty_sessions.insert(session.clone());
            }
            self.flush_session(&session, None);
        }
        Ok(())
    }

    /// **NR-11: der versionierte Spiegel des Intent-Bestands im append-only
    /// `event_log`** (§33.5, M-71).
    ///
    /// Geschrieben wird der Bestand, wie er nach der Uebernahme steht —
    /// Revision, Marke und die drei Listen. Der Spiegel dient Suche und
    /// Crashdiagnose; er wird NIE zurueckgelesen, um den Main zu ueberstimmen.
    fn intent_spiegeln(&self, session: &SessionKey) {
        let Some(store) = self.store.as_ref() else {
            return;
        };
        let bestand = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            match stand.intent.get(session) {
                Some(b) => b.clone(),
                None => return,
            }
        };
        let payload = serde_json::json!({
            "bestand_revision": bestand.revision,
            "vollstaendig": bestand.vollstaendig,
            "intents": bestand
                .intents
                .values()
                .map(|s| serde_json::json!({
                    "quelle_id": s.quelle_id,
                    "passage_id": s.passage_id,
                    "rolle": s.rolle,
                    "revision": s.revision,
                    "herkunft": s.herkunft,
                    "konfidenz": s.konfidenz,
                }))
                .collect::<Vec<Value>>(),
            "schutzangaben": bestand
                .schutzangaben
                .iter()
                .map(|s| serde_json::json!({
                    "quelle_id": s.quelle_id,
                    "eigenschaft": s.eigenschaft,
                    "band": s.band.map(|(von, bis)| serde_json::json!({"von": von, "bis": bis})),
                }))
                .collect::<Vec<Value>>(),
            "beziehungen": bestand
                .beziehungen
                .iter()
                .map(|((a, b), art)| serde_json::json!({
                    "quelle_a": a,
                    "quelle_b": b,
                    "art": art,
                }))
                .collect::<Vec<Value>>(),
        });
        let Ok(payload_jcs) = serde_json_canonicalizer::to_vec(&payload) else {
            return;
        };
        let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
        let mut event = StoreEvent::session_snapshot(
            &session.project_binding_id,
            &session.session_epoch,
            &self.broker_epoch,
            sequence.min(i64::MAX as u64) as i64,
            payload_jcs,
        );
        event.event_type = "intent_update".into();
        if store.append(vec![event]).is_err() {
            // Ein gescheiterter Spiegel NIMMT die Uebernahme NICHT zurueck:
            // der Main-State ist autoritativ, und ein Spiegel, der fehlt, ist
            // ein Diagnoseverlust, kein Datenverlust. Gezaehlt wird er
            // trotzdem — sonst waere „nicht gespiegelt" unbeobachtbar.
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
        }
    }

    /// Der oeffentliche Weg mit der aktiven Fassung.
    pub fn intent_update_json(&self, link_id: &str, payload: &[u8]) -> bool {
        self.intent_update_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
            .is_ok()
    }

    #[doc(hidden)]
    pub fn intent_update_json_grund_fuer_test(
        &self,
        link_id: &str,
        payload: &[u8],
    ) -> Result<(), IntentAbweisung> {
        self.intent_update_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
    }

    #[doc(hidden)]
    pub fn intent_update_json_mit_minor_fuer_test(
        &self,
        link_id: &str,
        payload: &[u8],
        schema_minor: u8,
    ) -> bool {
        self.intent_update_json_mit_minor(link_id, payload, schema_minor)
            .is_ok()
    }

    /// Der gespiegelte Bestand einer Sitzung — Kopie, wie jede andere Sicht.
    pub fn intent_sicht(&self, project_binding_id: &str, session_epoch: &str) -> IntentBestand {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand
            .intent
            .get(&SessionKey {
                project_binding_id: project_binding_id.into(),
                session_epoch: session_epoch.into(),
            })
            .cloned()
            .unwrap_or_default()
    }

    /// **M-86, die Sperre.** Bis die Vollstaendigkeitsmarke vorliegt, entsteht
    /// KEIN Proposal — der Zustand ist `more_data`.
    ///
    /// Sie steht als eigene Frage im Coordinator und nicht als Feld in einer
    /// Rechnung, damit jeder kuenftige Rechner sie stellen MUSS, statt sie
    /// vergessen zu koennen.
    pub fn darf_rechnen(&self, project_binding_id: &str, session_epoch: &str) -> bool {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        // 🔑 NR-02/NR-01 (Nacharbeit 1): DIESELBE Funktion, die auch
        // `aufnahmen_sammeln` und `hypothesen()` stellen. Eine zweite
        // Formulierung derselben Frage war der Defekt EP-01.
        darf_gerechnet_werden(stand.intent.get(&SessionKey {
            project_binding_id: project_binding_id.into(),
            session_epoch: session_epoch.into(),
        }))
    }

    /// Wie viele `intent_update` angenommen wurden. Ein Zaehler, keine
    /// Behauptung: ohne ihn waere „angenommen" von „still verworfen" nicht zu
    /// unterscheiden.
    pub fn intent_updates(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .intent_updates
    }

    /// M-76: Sitzungsende und Brokerneustart loeschen den Spiegel. Er ist
    /// FLUECHTIG und wird aus dem Main-State rekonstruiert; der Broker ist
    /// nicht berechtigt, beim Reconnect seinen Cache auf die Probe zu
    /// druecken.
    ///
    /// Beide Spiegel — Intent und Assistentenschritt — fallen GEMEINSAM. Sie
    /// gehoeren derselben Sitzung, und ein halb geleerter Cache waere
    /// schlimmer als gar keiner: die Vollstaendigkeitsmarke des einen wuerde
    /// eine Rechnung freigeben, fuer die der andere Bestand fehlt.
    pub(super) fn intent_spiegel_leeren_locked(stand: &mut Stand, session: &SessionKey) {
        stand.intent.remove(session);
        stand.assistent.remove(session);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// **M-77, der Riegel gegen die zweite Kopie.**
    ///
    /// `ROLLEN` und `SCHRITTE` sind Kopien geschlossener Mengen, die im
    /// eingefrorenen Schema stehen. Genau solche Kopien sind in diesem
    /// Projekt schon auseinandergelaufen (T2-Runde 2, BF-5: drei
    /// handgepflegte Listen derselben Sache). Dieselbe Form wie
    /// `p2_reject_katalog_stimmt_mit_dem_strikten_wire_schema`: die Abweichung
    /// faellt HIER und nicht an einem Fixture, das zufaellig darauf zielt.
    ///
    /// Ohne diesen Test waere „die Menge lebt an genau einer Stelle im
    /// Vertrag" eine Behauptung: ein Leser mit einer sechsten Rolle nimmt
    /// jedes gueltige Fixture an und faellt an keinem einzigen.
    #[test]
    fn geschlossene_mengen_des_lesers_decken_sich_mit_dem_vertrag() {
        let schema: Value = serde_json::from_str(include_str!(
            "../../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json"
        ))
        .unwrap();

        let aus_vertrag = |name: &str| -> Vec<String> {
            schema["$defs"][name]["enum"]
                .as_array()
                .unwrap_or_else(|| panic!("{name} traegt kein enum"))
                .iter()
                .filter_map(|w| w.as_str().map(str::to_owned))
                .collect()
        };

        assert_eq!(
            ROLLEN.iter().map(|s| s.to_string()).collect::<Vec<_>>(),
            aus_vertrag("intent_rolle"),
            "die Rollenmenge des Lesers weicht vom Vertrag ab"
        );
        assert_eq!(
            super::super::assistent::SCHRITTE
                .iter()
                .map(|s| s.to_string())
                .collect::<Vec<_>>(),
            aus_vertrag("assistant_schritt"),
            "die Schrittmenge des Lesers weicht vom Vertrag ab"
        );
    }

    /// Der Zyklenriegel, pur. Er laeuft auch im Leser, dessen Kantenzahl eine
    /// EINGABE ist — deshalb iterativ und nicht rekursiv (NAK-175).
    #[test]
    fn zyklenriegel_trennt_kette_runde_und_gleichrangigkeit() {
        let kante = |a: &str, b: &str, art: &str| ((a.to_string(), b.to_string()), art.to_string());

        let leer: BTreeMap<(String, String), String> = BTreeMap::new();
        assert!(!hat_zyklus(&leer));

        let kette: BTreeMap<_, _> = [
            kante("a", "b", "fuehrt_vor"),
            kante("b", "c", "fuehrt_vor"),
        ]
        .into_iter()
        .collect();
        assert!(!hat_zyklus(&kette));

        let runde: BTreeMap<_, _> = [
            kante("a", "b", "fuehrt_vor"),
            kante("b", "c", "fuehrt_vor"),
            kante("c", "a", "fuehrt_vor"),
        ]
        .into_iter()
        .collect();
        assert!(hat_zyklus(&runde));

        // Dieselbe Runde mit einer als `gleichrangig` MARKIERTEN Kante ist
        // kein Zyklus - das ist der von Paragraph 37.2 verlangte Ausweg.
        let aufgeloest: BTreeMap<_, _> = [
            kante("a", "b", "fuehrt_vor"),
            kante("b", "c", "fuehrt_vor"),
            kante("c", "a", "gleichrangig"),
        ]
        .into_iter()
        .collect();
        assert!(!hat_zyklus(&aufgeloest));

        // `darf_verschmelzen` ist keine Prioritaetskante.
        let verschmelzung: BTreeMap<_, _> = [
            kante("a", "b", "darf_verschmelzen"),
            kante("b", "a", "darf_verschmelzen"),
        ]
        .into_iter()
        .collect();
        assert!(!hat_zyklus(&verschmelzung));

        // Zwei getrennte Komponenten, von denen NUR die zweite eine Runde
        // traegt: eine Tiefensuche, die nach der ersten Komponente aufhoert,
        // faellt hier.
        let zwei: BTreeMap<_, _> = [
            kante("a", "b", "fuehrt_vor"),
            kante("x", "y", "fuehrt_vor"),
            kante("y", "x", "fuehrt_vor"),
        ]
        .into_iter()
        .collect();
        assert!(hat_zyklus(&zwei));
    }
}
