//! Empfaenger des `evidence_snapshot` (SONDE-013 M-05, Etappe B).
//!
//! Der Evidenzpfad war bis zu diesem Ticket vertraglich fertig und ohne
//! Empfaenger: an `ed9bbf7` nennt kein Rust-Modul ausser dem Schema selbst
//! den Namen. §33.2 ordnet die 221 Baender, die Verteilungspunkte, Abdeckung,
//! Konvergenz und die Ereignisse aber ausdruecklich diesem Snapshot zu.
//!
//! ## Was hier NICHT passiert
//!
//! Der Broker rechnet an dieser Stelle nichts aus. Comparability und
//! Experimentdeltas gehoeren den Etappen F und G; hier entsteht die Annahme,
//! die vollstaendige Ablage und die Paarbildung. Ein Empfaenger, der schon
//! WERTETE, waere eine zweite Wahrheit neben dem Store.
//!
//! ## Der Riegel und das eine Lock (M-63)
//!
//! 🔑 Nacharbeit 1 (Befund B12): die Taintpruefung nahm ihr eigenes Lock und
//! gab es VOR dem Insert wieder frei. Ein paralleles `intervention_begin`
//! konnte genau dazwischen laufen — die bereits kontaminierte Evidenz wurde
//! trotzdem committet. M-63 verlangt Pruefung, Zaehlung und tatsaechlichen
//! Commit unter DEMSELBEN Lock, und genau so steht es jetzt hier: ein
//! einziger `stand`-Guard umschliesst alle drei.

use super::*;

/// Wie viele angenommene Snapshots je Quelle im Arbeitsspeicher bleiben.
///
/// 🔑 Nacharbeit 1 (Befund B13): der Insert ERSETZTE vorher den vorigen
/// Snapshot derselben Quelle. Bandwerte, Perzentile, Stereo- und
/// Transportdetails und alle frueheren Evidence-IDs gingen damit verloren —
/// aus EINEM Punkt lassen sich weder Resultat- noch Guardrail-Deltas rechnen
/// (M-49), und ein Export ohne Evidence-IDs ist keiner (M-51).
///
/// Die Zahl ist gedeckelt und nicht „alle": eine unbegrenzte Historie waere
/// derselbe Fehler mit umgekehrtem Vorzeichen (M-74). Bei 1 bis 4 Hz sind 32
/// Snapshots rund acht bis dreissig Sekunden — genug fuer ein Paar aus
/// Baseline und Resultat, wenig genug fuer einen festen Speicherdeckel. Die
/// VOLLSTAENDIGEN Bytes liegen ohnehin im Store, nicht hier.
pub const EVIDENZ_RETENTION: usize = 32;

/// Was der Broker von einem angenommenen Evidenzsnapshot im Speicher behaelt.
///
/// Die Wire-Wahrheit ist der Payload im Store; diese Sicht traegt, was ein
/// Verbraucher braucht, um zu entscheiden, ob er den Snapshot ueberhaupt
/// ansieht — und die Zahlen, die eine duenne Messung als duenn ausweisen.
#[derive(Debug, Clone, PartialEq, Default)]
pub struct Evidenzstand {
    pub evidence_id: String,
    pub sequence: u64,
    pub abdeckung: f64,
    pub konvergenz: f64,
    pub klasse: String,
    pub ereignisse: usize,
    pub ereignisse_verloren: u64,
    pub verteilung_fenster: u64,
    /// SONDE-013 M-07: nicht-endliche Eingangssamples im Fenster dieses
    /// Belegs. `> 0` heisst: die Engine hat dort Stille gerechnet.
    pub samples_nicht_endlich: u64,
    /// Das Frame-Flag aus dem Snapshot (M-52, Luecke B14). Ein Beleg mit
    /// `true` ist ausgeschlossen — er kommt hier gar nicht an, aber die
    /// Struktur traegt das Feld, damit ein Leser die Regel sieht.
    pub beeinflusst: bool,
    /// Ob der Snapshot Baender, Perzentile und Stereo mitgebracht hat. Ohne
    /// diese Auskunft saehe ein duenner Beleg wie ein vollstaendiger aus.
    pub hat_baender: bool,
    pub hat_verteilung: bool,
    pub hat_stereo: bool,
    /// Der Ausschlussgrund, falls der Sender selbst einen nennt.
    pub ausschlussgrund: Option<String>,

    // ── Was der PRE/POST-Join braucht (M-13/M-14, Befund B25) ──────────
    //
    // 🔑 Der Empfaenger VERWARF diese Groessen vorher. `bilde_paare` hatte
    // ausserhalb der Tests keinen Aufrufer, und selbst mit einem haette er
    // nichts vorgefunden: eine `Paarhaelfte` braucht Huellkurven ueber die
    // ZEIT, und ein einzelner Snapshot ist ein Punkt. Genau deshalb behaelt
    // der Empfaenger jetzt eine Historie (`EVIDENZ_RETENTION`) — die Folge
    // der P50-Saetze IST die Huellkurve.
    /// Der P50-Verlauf je Band in dB. Ein Band ohne Bit traegt `f32::NAN`
    /// nicht: es traegt den Wert des Vorgaengers nicht und faellt hier auf
    /// 0 mit `false` in `p50_gueltig` (M-07).
    pub p50_db: Vec<f32>,
    pub p50_gueltig: Vec<bool>,
    /// Onsetstaerke dieses Fensters: die Summe der Ereignisstaerken. Sie ist
    /// die zweite, unabhaengige Spur aus §38.2.
    pub onset: f32,
    pub transport_epoch: u64,
    pub continuity_segment: u64,
    pub project_sample_start: Option<i64>,
    pub sample_count: u32,
    pub sample_rate: f64,
    /// Bandzahl des Gitters — Teil beider Frameschluessel (M-21).
    pub resolution: u32,
}

impl Coordinator {
    /// Nimmt einen `evidence_snapshot` an oder verwirft ihn.
    ///
    /// Fail-closed in dieser Reihenfolge: Vertrag, Adresse, Link, Frame-Flag,
    /// dann der Interventionsriegel. Jede Stufe fuer sich ist ein Grund zu
    /// verwerfen; keine davon fuehrt zu einer abgeschwaechten Annahme.
    pub(super) fn evidence_snapshot_json_mit_minor(
        &self,
        link_id: &str,
        payload: &[u8],
        schema_minor: u8,
    ) -> bool {
        let Some(wert) = v3_nachricht_lesen_mit_minor(payload, "evidence_snapshot", schema_minor)
        else {
            return false;
        };
        let Ok(adresse) = serde_json::from_value::<Adresse>(wert["adresse"].clone()) else {
            return false;
        };

        // Alles, was NUR den Payload betrifft, entsteht VOR dem Lock. Der
        // Lockabschnitt darunter soll so kurz sein wie moeglich — er haelt
        // den gesamten Sessiongraphen an.
        let stand_neu = Self::evidenzstand_aus_wert(&wert);
        let beeinflusst = stand_neu.beeinflusst;

        // ── DER EINE LOCKABSCHNITT (M-63) ───────────────────────────────
        let client_key = {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let Some(link) = stand.links.get(link_id).cloned() else {
                return false;
            };
            // Dieselbe Kante wie beim state_report: die Adresse IM Payload muss
            // die des sendenden Links sein. Ein Peer schreibt sich seine
            // Quelle nicht selbst zu.
            if link.wire_adresse != adresse {
                return false;
            }
            let Some(client) = stand.clients.get(&link.client_key) else {
                return false;
            };
            if client.current_link.as_deref() != Some(link_id) {
                return false;
            }
            let key = link.client_key.clone();
            let session = key.session();

            // 🔑 Luecke B14, entschieden: der Broker LIEST `beeinflusst`.
            //
            // Ein schema-gueltiger Snapshot mit `beeinflusst=true` wurde vorher
            // ohne dieses Flag zusammengefasst und bei momentan freiem Gate
            // normal angenommen. Bei verspaetetem oder verlorenem
            // Interventions-P0 diente er damit als saubere Baseline. M-52
            // fuehrt das Frame-Flag ausdruecklich als ZUSAETZLICHEN
            // Ausschlussbeleg — unabhaengig vom globalen Gate und nie als
            // alleinige Garantie. Er wird gezaehlt, nicht still verworfen.
            if beeinflusst {
                stand.evidence_beeinflusst = stand.evidence_beeinflusst.saturating_add(1);
                stand.evidence_gesperrt = stand.evidence_gesperrt.saturating_add(1);
                return false;
            }

            // Riegel UND Zaehlung UND Insert unter DEMSELBEN Lock (M-63).
            if !Self::evidence_dispatch_locked(&mut stand, &session) {
                return false;
            }
            let historie = stand.evidenz.entry(key.clone()).or_default();
            historie.push_back(stand_neu);
            while historie.len() > EVIDENZ_RETENTION {
                historie.pop_front();
            }
            key
        };

        // 🔑 Befund B24: die AUSLOESER der Invalidierung sitzen hier, weil
        // hier die Transportstempel ankommen. Ein Seek oder ein Drop faellt
        // genau zwischen zwei Snapshots derselben Quelle auf — nirgends
        // sonst hat der Broker beide Seiten der Grenze nebeneinander.
        self.invalidierung_aus_transportbruch(&client_key);
        // Die VOLLSTAENDIGE Ablage liegt ausserhalb des Locks: sie geht ueber
        // den StoreHandle und darf den Sessiongraphen nicht anhalten.
        self.evidenz_persistieren(&client_key, &wert);
        // Und der PRE/POST-Join sieht den neuen Beleg (M-13/M-14, Befund B25).
        self.evidenz_paare_bilden();
        self.heartbeat_kontakt(link_id, None)
    }

    /// Die Zusammenfassung aus dem Wire-Wert. Sie steht als eigene Funktion,
    /// damit der Lockabschnitt oben nichts rechnet.
    fn evidenzstand_aus_wert(wert: &Value) -> Evidenzstand {
        let p50 = Self::p50_dekodieren(wert);
        let ereignisse = wert
            .pointer("/ereignisse/liste")
            .and_then(Value::as_array)
            .map_or(0, Vec::len);
        Evidenzstand {
            evidence_id: wert["evidence_id"].as_str().unwrap_or_default().to_owned(),
            sequence: wert
                .pointer("/transport/sequence")
                .and_then(Value::as_u64)
                .unwrap_or(0),
            abdeckung: wert["abdeckung"].as_f64().unwrap_or(0.0),
            konvergenz: wert["konvergenz"].as_f64().unwrap_or(0.0),
            klasse: wert
                .pointer("/konfidenz/klasse")
                .and_then(Value::as_str)
                .unwrap_or_default()
                .to_owned(),
            ereignisse,
            ereignisse_verloren: wert
                .pointer("/ereignisse/verloren")
                .and_then(Value::as_u64)
                .unwrap_or(0),
            verteilung_fenster: wert
                .pointer("/konfidenz/verteilung_fenster")
                .and_then(Value::as_u64)
                .unwrap_or(0),
            samples_nicht_endlich: wert
                .pointer("/konfidenz/samples_nicht_endlich")
                .and_then(Value::as_u64)
                .unwrap_or(0),
            beeinflusst: wert["beeinflusst"].as_bool().unwrap_or(false),
            hat_baender: wert.get("baender").is_some(),
            hat_verteilung: wert.get("verteilung").is_some(),
            hat_stereo: wert.get("stereo").is_some(),
            ausschlussgrund: wert
                .get("ausschlussgrund")
                .and_then(Value::as_str)
                .map(str::to_owned),
            p50_db: p50.0,
            p50_gueltig: p50.1,
            onset: wert
                .pointer("/ereignisse/liste")
                .and_then(Value::as_array)
                .map(|liste| {
                    liste
                        .iter()
                        .filter_map(|e| e.get("staerke_mad").and_then(Value::as_f64))
                        .filter(|v| v.is_finite())
                        .sum::<f64>() as f32
                })
                .unwrap_or(0.0),
            transport_epoch: wert
                .pointer("/transport/transport_epoch")
                .and_then(Value::as_u64)
                .unwrap_or(0),
            continuity_segment: wert
                .pointer("/transport/continuity_segment")
                .and_then(Value::as_u64)
                .unwrap_or(0),
            project_sample_start: wert
                .pointer("/transport/project_sample_start")
                .and_then(Value::as_i64),
            sample_count: wert
                .pointer("/transport/sample_count")
                .and_then(Value::as_u64)
                .unwrap_or(0) as u32,
            sample_rate: wert
                .pointer("/transport/sample_rate")
                .and_then(Value::as_f64)
                .unwrap_or(0.0),
            resolution: wert
                .pointer("/verteilung/p50/werte")
                .and_then(Value::as_array)
                .map_or(0, Vec::len) as u32,
        }
    }

    /// Dekodiert `verteilung.p50` zu dB samt Praesenzbits.
    ///
    /// Die Kodierung steht IM Bandsatz (`encoding`), nicht in einer Annahme
    /// hier: `q_db_0p1_i16` und `q_db_0p01_i16` unterscheiden sich um den
    /// Faktor zehn, und wer den falschen waehlt, bekommt eine plausible
    /// Kurve, die um 20 dB danebenliegt. Ein Band ohne Gueltigkeitsbit
    /// traegt 0 mit `false` — nie den Wert des Vorgaengers (M-07).
    fn p50_dekodieren(wert: &Value) -> (Vec<f32>, Vec<bool>) {
        let Some(satz) = wert.pointer("/verteilung/p50") else {
            return (Vec::new(), Vec::new());
        };
        let Some(werte) = satz.get("werte").and_then(Value::as_array) else {
            return (Vec::new(), Vec::new());
        };
        let teiler = match satz.get("encoding").and_then(Value::as_str) {
            Some("q_db_0p01_i16") => 100.0,
            Some("q_db_0p1_i16") => 10.0,
            // `float32` traegt dB direkt; alles andere ist unbekannt und
            // damit ungueltig statt geraten.
            Some("float32") => 1.0,
            _ => return (Vec::new(), Vec::new()),
        };
        let bitmap = satz
            .get("gueltig_bitmap")
            .and_then(Value::as_str)
            .map(Self::base64_bits)
            .unwrap_or_default();
        let mut db = Vec::with_capacity(werte.len());
        let mut gueltig = Vec::with_capacity(werte.len());
        for (i, v) in werte.iter().enumerate() {
            let ok = bitmap.get(i).copied().unwrap_or(false);
            let zahl = v.as_f64().map(|x| x / teiler).unwrap_or(f64::NAN);
            if ok && zahl.is_finite() {
                db.push(zahl as f32);
                gueltig.push(true);
            } else {
                db.push(0.0);
                gueltig.push(false);
            }
        }
        (db, gueltig)
    }

    /// Base64 zu Bits, LSB-first je Byte — dieselbe Ordnung wie im Vertrag.
    fn base64_bits(text: &str) -> Vec<bool> {
        const ALPHABET: &[u8; 64] =
            b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        let mut bytes: Vec<u8> = Vec::new();
        let mut akku: u32 = 0;
        let mut bits = 0u32;
        for zeichen in text.bytes() {
            if zeichen == b'=' {
                break;
            }
            let Some(index) = ALPHABET.iter().position(|c| *c == zeichen) else {
                return Vec::new();
            };
            akku = (akku << 6) | index as u32;
            bits += 6;
            if bits >= 8 {
                bits -= 8;
                bytes.push(((akku >> bits) & 0xFF) as u8);
            }
        }
        let mut aus = Vec::with_capacity(bytes.len() * 8);
        for byte in bytes {
            for bit in 0..8 {
                aus.push((byte >> bit) & 1 == 1);
            }
        }
        aus
    }

    /// Schreibt den VOLLSTAENDIGEN Snapshot in den Store (M-49/M-51).
    ///
    /// Der Payload geht bytegleich in `evidence.state_jcs` — Baender,
    /// Perzentile, Stereo, Transport und die `evidence_id`. Erst damit kann
    /// der Broker Resultat- und Guardrail-Deltas rechnen, einen
    /// vollstaendigen Export liefern und nach einem Neustart rekonstruieren.
    ///
    /// Ohne Store laeuft der Broker weiter: der fluechtige Stand oben traegt
    /// die Zusammenfassung, und ein fehlender Store ist ein degradierter
    /// Betrieb, kein Datenverlust an einer Zusage (§53.9).
    fn evidenz_persistieren(&self, key: &ClientKey, wert: &Value) {
        let Some(store) = self.store.as_ref() else {
            return;
        };
        let Some(evidence_id) = wert.get("evidence_id").and_then(Value::as_str) else {
            return;
        };
        let payload = serde_json::json!({
            "evidence_id": evidence_id,
            "snapshot": wert,
        });
        let Ok(payload_jcs) = serde_json_canonicalizer::to_vec(&payload) else {
            return;
        };
        let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
        let mut event = StoreEvent::session_snapshot(
            &key.project_binding_id,
            &key.session_epoch,
            &self.broker_epoch,
            sequence.min(i64::MAX as u64) as i64,
            payload_jcs,
        );
        event.event_type = "evidence".into();
        if store.append(vec![event]).is_err() {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
        }
    }

    /// Derselbe Weg mit der aktiven Fassung — das oeffentliche Gegenstueck
    /// zu `state_report_json`, damit ein Test nicht die Senke nachbauen muss.
    pub fn evidence_snapshot_json(&self, link_id: &str, payload: &[u8]) -> bool {
        self.evidence_snapshot_json_mit_minor(link_id, payload, JSON_SCHEMA_MINOR_AKTIV)
    }

    /// Derselbe Weg mit einer GEWAEHLTEN Fassung.
    ///
    /// Nur fuer Beine: sie messen damit, dass ein Leser der Fassung 1 die
    /// Fassung 2 wirklich ablehnt. Der Produktpfad waehlt die Fassung
    /// ausschliesslich aus dem Wire-Envelope (`senke.rs`).
    pub fn evidence_snapshot_json_mit_minor_fuer_test(
        &self,
        link_id: &str,
        payload: &[u8],
        schema_minor: u8,
    ) -> bool {
        self.evidence_snapshot_json_mit_minor(link_id, payload, schema_minor)
    }

    /// Der zuletzt ANGENOMMENE Snapshot einer Quelle.
    ///
    /// `None` heisst „von dieser Quelle ist keiner angekommen" — nicht
    /// „einer kam an und war leer". Ein gesperrter Snapshot hinterlaesst
    /// hier nichts; sein Urteil steht im Zaehler `evidence_gesperrt`.
    pub fn evidenz_sicht(&self, instance_id: &str) -> Option<Evidenzstand> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand
            .evidenz
            .iter()
            .find(|(key, _)| key.instance_id == instance_id)
            .and_then(|(_, wert)| wert.back().cloned())
    }

    /// Die ganze behaltene Historie einer Quelle, aelteste zuerst (M-49).
    pub fn evidenz_historie(&self, instance_id: &str) -> Vec<Evidenzstand> {
        let stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
        stand
            .evidenz
            .iter()
            .find(|(key, _)| key.instance_id == instance_id)
            .map(|(_, wert)| wert.iter().cloned().collect())
            .unwrap_or_default()
    }

    /// Wie viele Snapshots wegen des Frame-Flags `beeinflusst` ausgeschlossen
    /// wurden (Luecke B14).
    pub fn evidence_beeinflusst_zaehler(&self) -> u64 {
        self.stand
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .evidence_beeinflusst
    }
}
