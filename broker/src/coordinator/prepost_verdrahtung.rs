//! Der PRE/POST-Join im PRODUKTPFAD (SONDE-013 M-13 bis M-24, Befund B25).
//!
//! ## Warum dieses Modul ueberhaupt existiert
//!
//! 🔑 Nacharbeit 1 nach der Erstpruefung 1: `prepost` war als freies Modul
//! exportiert, und `bilde_paare` hatte ausserhalb seiner eigenen Tests keinen
//! Aufrufer. Reale PRE-/POST-Snapshots erreichten damit nie den v3-Join.
//! Schlimmer noch: selbst mit einem Aufrufer haetten sie ihn nicht bedienen
//! koennen — der Evidenzempfaenger verwarf genau die Kurven und
//! Deskriptordaten, die eine `Paarhaelfte` braucht.
//!
//! ## Woher die Huellkurve kommt
//!
//! Eine `Paarhaelfte` verlangt `huellkurven[band][frame]`. Ein einzelner
//! `evidence_snapshot` ist ein PUNKT je Band, kein Verlauf. Die Kurve
//! entsteht deshalb aus der FOLGE der behaltenen Snapshots einer Quelle
//! (`EVIDENZ_RETENTION`): Snapshot n liefert die Stuetzstelle n. Das ist
//! keine Notloesung, sondern die Bauform, die §33.2 vorgibt — die
//! Evidenzkadenz IST die Framekadenz dieses Pfades.
//!
//! ## Was der Join hier NICHT behauptet
//!
//! Die Presentation-Abbildung ist heute nirgends validiert: kein Host liefert
//! `cycle_derivation = validated_block_mapping` (Capabilityreport S4). Jede
//! Paarhaelfte traegt deshalb `presentation_validiert = false`, und die
//! Klasse bleibt hoechstens `Probable`. Genau das ist der Exit-Gate-Satz aus
//! M-21: „Kein unbekannter Zeitpfad erzeugt eine starke Cross-Probe- oder
//! PRE/POST-Aussage." Der Join LAEUFT — er behauptet nur nichts Starkes.

use super::*;
use crate::coordinator::prepost::{bilde_paare, Paarhaelfte, Paarurteil, Rolle};

/// Wie viele Paarurteile der Coordinator im Speicher haelt.
///
/// Ein Deckel wie ueberall sonst (M-74): die Urteile sind eine Sicht auf die
/// laufende Sitzung, kein Archiv. Das Archiv ist der Store.
pub const PAARURTEIL_DECKEL: usize = 64;

/// Woraus eine Haelfte entsteht: die Historie EINER Quelle plus ihr
/// Deskriptor. Beides liegt im Coordinator; dieses Modul liest nur.
struct Halbzeug {
    /// 🔑 Nacharbeit 2 (Befund R31, M-13): die SITZUNG gehoert zum Schluessel.
    /// Ohne sie sammelte die Schleife Evidenz ALLER Sitzungen in eine Liste
    /// und gruppierte nur ueber `pair_id` — eine PRE-Haelfte aus Sitzung A und
    /// eine POST-Haelfte aus Sitzung B bildeten damit ein Paar.
    session: SessionKey,
    pair_id: String,
    rolle: Rolle,
    haelfte: Paarhaelfte,
}

impl Coordinator {
    /// Baut aus dem laufenden Evidenzbestand die Paare und beurteilt sie.
    ///
    /// Der Aufruf steht im Evidenzempfaenger, nach jeder ANGENOMMENEN
    /// Annahme: ein Paar ist erst dann neu zu beurteilen, wenn eine seiner
    /// Haelften gewachsen ist.
    pub(super) fn evidenz_paare_bilden(&self) {
        let halbzeuge = {
            let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let mut aus: Vec<Halbzeug> = Vec::new();
            for (key, historie) in stand.evidenz.iter() {
                if historie.is_empty() {
                    continue;
                }
                let Some(client) = stand.clients.get(key) else {
                    continue;
                };
                let Some(descriptor) = client.descriptor.as_ref() else {
                    // Ohne Deskriptor gibt es weder `pair_id` noch Messpunkt.
                    // Eine Haelfte ohne Rolle ist keine Haelfte.
                    continue;
                };
                let Some(pair_id) = descriptor.get("pair_id").and_then(Value::as_str) else {
                    continue;
                };
                let rolle = match descriptor
                    .get("measurement_position")
                    .and_then(Value::as_str)
                {
                    Some("pre") => Rolle::Pre,
                    Some("post") => Rolle::Post,
                    // `insert` und `post_fader_contribution` sind keine
                    // PRE/POST-Rollen. Sie stillschweigend auf eine der beiden
                    // abzubilden waere eine erfundene Zuordnung.
                    _ => continue,
                };
                let verbunden = client.current_link.is_some();
                aus.push(Halbzeug {
                    session: key.session(),
                    pair_id: pair_id.to_string(),
                    rolle,
                    haelfte: Self::haelfte_aus_historie(
                        key,
                        client.host_pid.unwrap_or(0),
                        verbunden,
                        !client.stale,
                        historie,
                    ),
                });
            }
            aus
        };
        if halbzeuge.is_empty() {
            return;
        }

        // 🔑 Nacharbeit 2 (Befund R31, M-13): erst nach SITZUNG gruppieren,
        // dann `bilde_paare` je Sitzung. `bilde_paare` gruppiert selbst
        // ausschliesslich ueber `pair_id`; bekaeme es die Haelften aller
        // Sitzungen in einer Liste, koennte es die Sitzungsgrenze gar nicht
        // sehen. Die Regel gehoert deshalb hierher, wo die Sitzung noch
        // bekannt ist, und nicht in das reine Modul darunter.
        let mut je_sitzung: std::collections::BTreeMap<
            (String, String),
            (SessionKey, Vec<(String, Rolle, Paarhaelfte)>),
        > = std::collections::BTreeMap::new();
        for h in halbzeuge {
            let schluessel = (
                h.session.project_binding_id.clone(),
                h.session.session_epoch.clone(),
            );
            je_sitzung
                .entry(schluessel)
                .or_insert_with(|| (h.session.clone(), Vec::new()))
                .1
                .push((h.pair_id, h.rolle, h.haelfte));
        }

        let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        for (_, (session, eingaben)) in je_sitzung {
            // `bilde_paare` beurteilt selbst - es ist der EINE Weg von Haelften
            // zu Urteilen, samt Paarkonflikt und fehlender Haelfte. Ihn hier
            // nachzubauen waere eine zweite Wahrheit ueber dieselbe Frage.
            for urteil in bilde_paare(&eingaben) {
                stand
                    .paarurteile
                    .insert((session.clone(), urteil.pair_id.clone()), urteil);
            }
        }
        // ⚠️ Der Deckel ist eine SPEICHERGRENZE, keine Auswahlregel: welches
        // Paar hier faellt, ist nicht bestimmt, und dieser Kommentar sagt das
        // ausdruecklich, statt eine Ordnung zu behaupten, die eine `HashMap`
        // nicht hat. Ein Paar, das wieder Evidenz bekommt, kommt beim naechsten
        // Snapshot ohnehin zurueck — der Verlust kostet eine Runde, keine
        // Aussage. Ein Deckel ganz ohne Regel waere trotzdem falsch; deshalb
        // steht die Zahl als benannte Konstante da (M-74).
        while stand.paarurteile.len() > PAARURTEIL_DECKEL {
            let Some(weg) = stand.paarurteile.keys().next().cloned() else {
                break;
            };
            stand.paarurteile.remove(&weg);
        }
    }

    /// Die Historie einer Quelle als `Paarhaelfte`.
    fn haelfte_aus_historie(
        key: &ClientKey,
        host_pid: u32,
        verbunden: bool,
        messbereit: bool,
        historie: &std::collections::VecDeque<super::evidenz::Evidenzstand>,
    ) -> Paarhaelfte {
        let letzter = historie.back().expect("nicht leer, oben geprueft");
        let baender = letzter.p50_db.len();
        // `huellkurven[band][frame]`: die Folge der Snapshots ist die Zeitachse.
        let mut huellkurven: Vec<Vec<f32>> = vec![Vec::with_capacity(historie.len()); baender];
        let mut onsets: Vec<f32> = Vec::with_capacity(historie.len());
        let mut spruenge = 0u32;
        let mut aktiv_s = 0.0f64;
        let mut von: Option<i64> = None;
        let mut bis: Option<i64> = None;
        let mut vorige_epoche: Option<u64> = None;
        let mut voriges_segment: Option<u64> = None;
        for stand in historie.iter() {
            if stand.p50_db.len() != baender {
                // Ein Snapshot mit anderem Gitter gehoert nicht in dieselbe
                // Kurve. Ihn einzupassen hiesse, zwei Aufloesungen zu mischen.
                continue;
            }
            for (band, wert) in stand.p50_db.iter().enumerate() {
                let gueltig = stand.p50_gueltig.get(band).copied().unwrap_or(false);
                // Ein Band ohne Bit traegt 0 — der Korrelationspfad rechnet
                // ueber Differenzen, und 0 ist dort das neutrale Element.
                huellkurven[band].push(if gueltig { *wert } else { 0.0 });
            }
            onsets.push(stand.onset);
            if let Some(vorher) = vorige_epoche {
                if vorher != stand.transport_epoch {
                    spruenge = spruenge.saturating_add(1);
                }
            }
            if let Some(vorher) = voriges_segment {
                if vorher != stand.continuity_segment {
                    spruenge = spruenge.saturating_add(1);
                }
            }
            vorige_epoche = Some(stand.transport_epoch);
            voriges_segment = Some(stand.continuity_segment);
            if stand.sample_rate > 0.0 {
                aktiv_s += stand.sample_count as f64 / stand.sample_rate;
            }
            if let Some(start) = stand.project_sample_start {
                von = Some(von.map_or(start, |v: i64| v.min(start)));
                let ende = start.saturating_add(stand.sample_count as i64);
                bis = Some(bis.map_or(ende, |v: i64| v.max(ende)));
            }
        }
        Paarhaelfte {
            instance_id: key.instance_id.clone(),
            host_pid,
            transport_epoch: letzter.transport_epoch,
            sample_rate: letzter.sample_rate,
            resolution: letzter.resolution,
            verbunden,
            stale: !messbereit,
            messbereit,
            projekt_fenster: match (von, bis) {
                (Some(a), Some(b)) if b > a => Some((a, b)),
                // Ohne gueltige Projektzeit gibt es kein Fenster — und das
                // ist ausdruecklich NICHT „Fenster ab null" (§32.3).
                _ => None,
            },
            spruenge,
            aktiv_s,
            huellkurven,
            onsets,
            // M-21: heute validiert kein Host die Presentation-Abbildung.
            // Ein `true` an dieser Stelle waere die Behauptung, die das
            // Exit-Gate ausschliesst.
            presentation_validiert: false,
            session_epoch: 0,
            timeline_epoch: letzter.transport_epoch,
        }
    }

    /// Das Urteil ueber ein Paar EINER Sitzung, ueber ihren Link adressiert.
    ///
    /// Das ist der genaue Weg: eine `pair_id` allein identifiziert kein Paar
    /// (M-13). `paarurteil` daneben bleibt der bequeme Weg fuer den
    /// Einzelsitzungsfall und sagt ausdruecklich nichts, wenn er mehrdeutig
    /// waere.
    pub fn paarurteil_fuer_link(&self, link_id: &str, pair_id: &str) -> Option<Paarurteil> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let session = Self::session_des_links(&stand, link_id)?;
        stand
            .paarurteile
            .get(&(session, pair_id.to_owned()))
            .cloned()
    }

    /// Das Urteil ueber ein Paar, wie der Produktpfad es gerechnet hat.
    ///
    /// ⚠️ Nur EINDEUTIG: fuehren zwei Sitzungen dieselbe `pair_id`, ist die
    /// Frage falsch gestellt, und die Antwort ist `None` statt eine der beiden
    /// zu waehlen (M-13, Befund R31). Wer die Sitzung kennt, nimmt
    /// `paarurteil_fuer_link`.
    pub fn paarurteil(&self, pair_id: &str) -> Option<Paarurteil> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        let mut treffer = stand
            .paarurteile
            .iter()
            .filter(|((_, id), _)| id == pair_id)
            .map(|(_, urteil)| urteil);
        let erster = treffer.next()?;
        treffer.next().is_none().then(|| erster.clone())
    }

    /// Wie viele Paare der Produktpfad zurzeit fuehrt.
    pub fn paarurteile_anzahl(&self) -> usize {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .paarurteile
            .len()
    }
}
