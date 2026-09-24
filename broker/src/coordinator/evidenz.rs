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
    /// Die ANKUNFTSREIHENFOLGE im Broker, monoton ueber alle Quellen.
    ///
    /// 🔑 Nacharbeit 2 (Befund R17, M-49): `resultatmessung` teilte die
    /// Historie einer beliebigen Quelle stumpf in zwei Haelften. Vier bereits
    /// VOR dem `experiment_begin` eingegangene Snapshots genuegten damit fuer
    /// ein sofortiges Resultat. Baseline und Resultat brauchen eine Grenze,
    /// und die einzige, die der Broker selbst kennt und die ueber Quellen
    /// hinweg vergleichbar ist, ist seine eigene Ankunftsreihenfolge. Die
    /// `sequence` daneben ist die des SENDERS und je Quelle eigen.
    pub empfangsfolge: u64,
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
    /// Dasselbe fuer P95 — der PEAK-Guardrail (M-45, Befund R19). Ohne ihn
    /// koennte eine Aenderung die Spitzen anheben, waehrend der Median steht.
    pub p95_db: Vec<f32>,
    pub p95_gueltig: Vec<bool>,
    /// `stereo.seitenanteil_db` — der BREITE-Guardrail (M-45, Befund R19).
    /// `None` heisst „diese Quelle liefert keine Stereoauskunft", nie 0.
    pub seitenanteil_db: Option<f64>,
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
        self.evidence_snapshot_json_mit_minor_wert(link_id, wert)
    }

    /// Die Wertvariante fuer die P1-Weiche (NAK-313 R-313-6, M-44): `wert` hat
    /// Textriegel, strengen Lauf und Schema schon hinter sich und wird hier
    /// nicht noch einmal gelesen.
    pub(super) fn evidence_snapshot_json_mit_minor_wert(&self, link_id: &str, wert: Value) -> bool {
        if wert.get("type").and_then(Value::as_str) != Some("evidence_snapshot") {
            return false;
        }
        let Ok(adresse) = serde_json::from_value::<Adresse>(wert["adresse"].clone()) else {
            return false;
        };

        // Alles, was NUR den Payload betrifft, entsteht VOR dem Lock. Der
        // Lockabschnitt darunter soll so kurz sein wie moeglich — er haelt
        // den gesamten Sessiongraphen an. Ein Ganzzahlfeld, das der Helfer
        // nicht liest, lehnt den Snapshot hier ab (NAK-313 R-313-5, M-66).
        let Some(stand_neu) = Self::evidenzstand_aus_wert(&wert) else {
            return false;
        };
        let beeinflusst = stand_neu.beeinflusst;

        // ── DER EINE LOCKABSCHNITT (M-63) ───────────────────────────────
        let (client_key, empfangsfolge) = {
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
            let mut eintrag = stand_neu;
            // Die Ankunftsreihenfolge wird UNTER DEM LOCK vergeben — sie ist
            // die Grenze, an der `resultatmessung` Baseline und Resultat
            // trennt (M-49, Befund R17).
            eintrag.empfangsfolge = self.evidenz_folge.fetch_add(1, Ordering::SeqCst);
            let folge = eintrag.empfangsfolge;
            historie.push_back(eintrag);
            while historie.len() > EVIDENZ_RETENTION {
                historie.pop_front();
            }
            (key, folge)
        };

        // 🔑 Befund B24: die AUSLOESER der Invalidierung sitzen hier, weil
        // hier die Transportstempel ankommen. Ein Seek oder ein Drop faellt
        // genau zwischen zwei Snapshots derselben Quelle auf — nirgends
        // sonst hat der Broker beide Seiten der Grenze nebeneinander.
        self.invalidierung_aus_transportbruch(&client_key);
        // 🔑 Nacharbeit 2 (Befund R07, G1(c)): ANNAHME UND ABLAGE SIND EINE
        // EINHEIT.
        //
        // Die Runde 1 trug den Snapshot unter dem Lock als angenommen ein und
        // legte ihn danach ab; scheiterte der Append — I/O-Fehler, degradierter
        // Store, voller Kanal —, erhoehte dieser Pfad nur einen Zaehler und
        // meldete weiter ERFOLG. Es existierte damit angenommene Evidenz ohne
        // den zugesagten Store-Event, und genau darauf rechnet `resultatmessung`
        // ihre Baseline. Die Annahme ist deshalb VORLAEUFIG, bis der Append
        // sie festschreibt; scheitert er, wird sie zurueckgenommen und gezaehlt.
        //
        // Die Ablage liegt weiter AUSSERHALB des Locks: sie geht ueber den
        // StoreHandle und darf den Sessiongraphen nicht anhalten.
        if !self.evidenz_persistieren(&client_key, &wert, empfangsfolge) {
            let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
            let evidence_id = wert["evidence_id"].as_str().unwrap_or_default().to_owned();
            if let Some(historie) = stand.evidenz.get_mut(&client_key) {
                // Ueber die `evidence_id` und nicht ueber `pop_back`: zwischen
                // Insert und Ruecknahme kann ein anderer Snapshot derselben
                // Quelle angekommen sein, und der gehoert nicht uns.
                historie.retain(|e| e.evidence_id != evidence_id);
            }
            stand.evidence_angenommen = stand.evidence_angenommen.saturating_sub(1);
            stand.evidence_gesperrt = stand.evidence_gesperrt.saturating_add(1);
            return false;
        }
        // Und der PRE/POST-Join sieht den neuen Beleg (M-13/M-14, Befund B25).
        self.evidenz_paare_bilden();
        // 🔑 SONDE-014 Etappe C: derselbe Anlass traegt die Ursachenrechnung.
        //
        // Sie steht NACH dem Paarjoin, nicht davor: `claim_class` = 2 haengt
        // an einem PRE/POST-Paarurteil (§36.1, M-17), und ein Befund, der vor
        // dem Join gebildet wuerde, saehe das Paar dieses Belegs nie. Die
        // Rechnung nimmt sich ihr eigenes Lock und haelt hier keines.
        self.hypothesen_bilden();
        // 🔑 SONDE-014 Etappe I (M-74): der Abflussweg OHNE Reconnect.
        //
        // `tools/dirigent/pruefliste.md` Abschnitt A woertlich: „Ein Puffer,
        // der ‚spaeter wiederholt‘, hat einen Abflussweg OHNE Reconnect; ‚nur
        // beim Verbindungsaufbau leeren‘ ist ein Befund." Genau dieser Befund
        // stand hier: `offene_outbox_nachspielen` hatte EINEN Aufrufer, und
        // der war der Subscribe. Ein Empfaenger, der einen Push kurz nicht
        // annahm und danach verbunden blieb, bekam seine Schuld nie.
        //
        // Der Anlass ist der richtige: Evidenz kommt an, die Sitzung arbeitet
        // ohnehin, und derselbe Empfaenger ist gerade wieder erreichbar. Ein
        // eigener Zeitgeber waere ein zweiter Takt neben dem Evidenztakt.
        // Nimmt der Empfaenger weiterhin nichts an, BLEIBT die Schuld stehen —
        // kompaktiert wird nur, was wirklich geschrieben wurde.
        self.offene_schuld_der_sitzung_nachspielen(&client_key.session());
        self.heartbeat_kontakt(link_id, None)
    }

    /// Die Zusammenfassung aus dem Wire-Wert. Sie steht als eigene Funktion,
    /// damit der Lockabschnitt oben nichts rechnet.
    ///
    /// 🔑 NAK-313 R-313-5 (M-66): die Ganzzahlfelder liest der Ganzzahlhelfer
    /// — `8241.0` ist die Sequenz 8241, nicht 0, und `3.0` nach `3` ist kein
    /// Epochwechsel. Ein FEHLENDES Feld behaelt die Vorgabe (0 beziehungsweise
    /// kein Startwert); ein VORHANDENES, das der Helfer nicht liest, macht den
    /// ganzen Stand zu `None` — nie 0 als Ersatz.
    pub(super) fn evidenzstand_aus_wert(wert: &Value) -> Option<Evidenzstand> {
        let feld = |zeiger: &str, max: i64| -> Option<i64> {
            crate::vertrag::ganzzahl_optional(wert.pointer(zeiger), 0, max).map(|w| w.unwrap_or(0))
        };
        let max = crate::vertrag::GANZZAHL_MAX;
        let sequence = feld("/transport/sequence", max)? as u64;
        let ereignisse_verloren = feld("/ereignisse/verloren", max)? as u64;
        let verteilung_fenster = feld("/konfidenz/verteilung_fenster", max)? as u64;
        let samples_nicht_endlich = feld("/konfidenz/samples_nicht_endlich", max)? as u64;
        let transport_epoch = feld("/transport/transport_epoch", max)? as u64;
        let continuity_segment = feld("/transport/continuity_segment", max)? as u64;
        let sample_count = feld("/transport/sample_count", i64::from(u32::MAX))? as u32;
        let project_sample_start = crate::vertrag::ganzzahl_optional(
            wert.pointer("/transport/project_sample_start"),
            -max,
            max,
        )?;
        let p50 = Self::perzentil_dekodieren(wert, "p50");
        let p95 = Self::perzentil_dekodieren(wert, "p95");
        let ereignisse = wert
            .pointer("/ereignisse/liste")
            .and_then(Value::as_array)
            .map_or(0, Vec::len);
        Some(Evidenzstand {
            evidence_id: wert["evidence_id"].as_str().unwrap_or_default().to_owned(),
            // Die Ankunftsreihenfolge vergibt der EMPFAENGER, nicht der
            // Payload; sie wird unter dem Lock gesetzt.
            empfangsfolge: 0,
            sequence,
            abdeckung: wert["abdeckung"].as_f64().unwrap_or(0.0),
            konvergenz: wert["konvergenz"].as_f64().unwrap_or(0.0),
            klasse: wert
                .pointer("/konfidenz/klasse")
                .and_then(Value::as_str)
                .unwrap_or_default()
                .to_owned(),
            ereignisse,
            ereignisse_verloren,
            verteilung_fenster,
            samples_nicht_endlich,
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
            p95_db: p95.0,
            p95_gueltig: p95.1,
            seitenanteil_db: wert
                .pointer("/stereo/seitenanteil_db")
                .and_then(Value::as_f64)
                .filter(|v| v.is_finite()),
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
            transport_epoch,
            continuity_segment,
            project_sample_start,
            sample_count,
            sample_rate: wert
                .pointer("/transport/sample_rate")
                .and_then(Value::as_f64)
                .unwrap_or(0.0),
            resolution: wert
                .pointer("/verteilung/p50/werte")
                .and_then(Value::as_array)
                .map_or(0, Vec::len) as u32,
        })
    }

    /// Dekodiert `verteilung.p50` zu dB samt Praesenzbits.
    ///
    /// Die Kodierung steht IM Bandsatz (`encoding`), nicht in einer Annahme
    /// hier: `q_db_0p1_i16` und `q_db_0p01_i16` unterscheiden sich um den
    /// Faktor zehn, und wer den falschen waehlt, bekommt eine plausible
    /// Kurve, die um 20 dB danebenliegt. Ein Band ohne Gueltigkeitsbit
    /// traegt 0 mit `false` — nie den Wert des Vorgaengers (M-07).
    fn perzentil_dekodieren(wert: &Value, name: &str) -> (Vec<f32>, Vec<bool>) {
        let Some(satz) = wert.pointer(&format!("/verteilung/{name}")) else {
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
    /// Rueckgabe: ob die Ablage GELUNGEN ist (Befund R07).
    ///
    /// `true` ohne Store heisst nicht „abgelegt", sondern „ohne Store gibt es
    /// nichts abzulegen": ein fehlender Store ist degradierter Betrieb, kein
    /// Datenverlust an einer Zusage (§53.9). Ein VORHANDENER Store, der den
    /// Append verweigert, ist dagegen genau das — und dann ist der Snapshot
    /// nicht angenommen.
    fn evidenz_persistieren(&self, key: &ClientKey, wert: &Value, empfangsfolge: u64) -> bool {
        let Some(store) = self.store.as_ref() else {
            return true;
        };
        let Some(evidence_id) = wert.get("evidence_id").and_then(Value::as_str) else {
            return false;
        };
        // 🔑 Nacharbeit 3 (Befund B6, M-49/M-50): die ANKUNFTSREIHENFOLGE
        // reist MIT.
        //
        // Sie stand bis dahin nur im fluechtigen Bestand. Nach einem Neustart
        // trug jede restaurierte Evidenz `empfangsfolge = 0`, waehrend die
        // persistierten Begin- und Kandidatengrenzen ihre hohen Werte
        // behielten: die alte Evidenz sah aus, als waere sie VOR jedem Begin
        // eingegangen, und neue Resultatevidenz begann wieder bei 0 und wurde
        // als Baseline eingeordnet oder verworfen. Sie steht NEBEN dem
        // Snapshot, nicht darin — der Snapshot ist die bytegleiche
        // Wire-Wahrheit, und die Reihenfolge ist eine Aussage des Empfaengers.
        let payload = serde_json::json!({
            "evidence_id": evidence_id,
            "empfangsfolge": empfangsfolge,
            "snapshot": wert,
        });
        let Ok(payload_jcs) = serde_json_canonicalizer::to_vec(&payload) else {
            return false;
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
            return false;
        }
        true
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

/// NAK-313 Etappe 5 (R-313-5, M-66): `evidenzstand_aus_wert` an Werten, die
/// von Hand gebaut sind — am Draht fangen Textriegel oder Schema sie vorher.
#[cfg(test)]
mod nak313_tests {
    use super::*;
    use serde_json::json;

    /// M-66, zweite Haelfte: ein VORHANDENES Ganzzahlfeld, das der Helfer
    /// nicht liest, lehnt den ganzen Stand ab (`None`), nie 0; ein FEHLENDES
    /// behaelt die Vorgabe des Vertrags (0, kein Startwert).
    #[test]
    fn nak313_m66_feld_ohne_ganzzahl_lehnt_ab() {
        let basis = json!({
            "type": "evidence_snapshot",
            "evidence_id": "e",
            "transport": {
                "sequence": 8241.0, "transport_epoch": 3.0, "continuity_segment": 2.0,
                "sample_count": 256.0, "project_sample_start": -1024.0
            },
            "ereignisse": { "verloren": 1.0, "liste": [] },
            "konfidenz": { "verteilung_fenster": 4.0, "samples_nicht_endlich": 2.0 }
        });
        let stand = Coordinator::evidenzstand_aus_wert(&basis).expect("die .0-Formen sind Ganzzahlen");
        assert_eq!(
            (
                stand.sequence,
                stand.transport_epoch,
                stand.continuity_segment,
                stand.sample_count,
                stand.project_sample_start,
                stand.ereignisse_verloren,
                stand.verteilung_fenster,
                stand.samples_nicht_endlich
            ),
            (8241, 3, 2, 256, Some(-1024), 1, 4, 2)
        );
        let mut rot = Vec::new();
        for (zeiger, schlecht) in [
            ("/transport/sequence", json!(1.5)),
            ("/transport/transport_epoch", json!(-1)),
            ("/transport/continuity_segment", json!("2")),
            ("/transport/sample_count", json!(4_294_967_296u64)),
            ("/transport/project_sample_start", json!(0.5)),
            ("/ereignisse/verloren", json!(9_007_199_254_740_992u64)),
            ("/konfidenz/verteilung_fenster", json!(true)),
            ("/konfidenz/samples_nicht_endlich", json!(2.5)),
        ] {
            let mut wert = basis.clone();
            *wert.pointer_mut(zeiger).expect("das Feld steht in der Basis") = schlecht.clone();
            if let Some(stand) = Coordinator::evidenzstand_aus_wert(&wert) {
                rot.push(format!(
                    "{zeiger} = {schlecht}: ein Stand (sequence {}, Start {:?}) statt der Ablehnung",
                    stand.sequence, stand.project_sample_start
                ));
            }
        }
        let ohne = json!({ "type": "evidence_snapshot", "evidence_id": "e", "transport": {} });
        let stand = Coordinator::evidenzstand_aus_wert(&ohne).expect("fehlende Felder lehnen nicht ab");
        assert_eq!((stand.sequence, stand.project_sample_start), (0, None));
        assert!(rot.is_empty(), "M-66:\n{}", rot.join("\n"));
    }
}
