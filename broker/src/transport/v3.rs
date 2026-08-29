//! SONDE-010 — v3-Envelope: der 16-Byte-Kopf nach dem bestehenden
//! `u32 frame_len` (Entwurf §33.1, Zeilen 1783 ff.).
//!
//! ```text
//!  u32 frame_len (little-endian, wie v2)
//!  u8  encoding        0 = JSON, 1 = FlatBuffers
//!  u8  message_family  0 = P0, 1 = P1, 2 = P2
//!  u8  schema_major
//!  u8  schema_minor
//!  u32 flags           heute vollstaendig reserviert => muss 0 sein
//!  u32 payload_len
//!  u32 crc32c          ueber GENAU die payload_len Payloadbytes
//! ```
//!
//! Der Parser entscheidet ALLES VOR dem Payloadparser. Er ist eine reine
//! Funktion ueber Bytes: dieselbe Regelmenge, dieselbe Reihenfolge und
//! dieselben Namen wie `WireEnvelope` auf der C++-Seite. Beide messen gegen
//! `eq-copilot/fixtures/v3/envelope/MANIFEST.json` — stimmen beide mit dem
//! handgeschriebenen Manifest ueberein, stimmen sie transitiv miteinander
//! ueberein.
//!
//! ── Warum drei Stufen und nicht eine flache Menge ──────────────────────────
//!
//! Eine flache Verstossmenge waere nicht wohldefiniert: wer den Rahmen nicht
//! kennt, kann den Payload nicht finden, und wer das Encoding nicht kennt,
//! kann ueber die CRC nichts sagen. Die drei Stufen machen genau diese
//! Abhaengigkeit explizit; innerhalb der letzten Stufe wird alles gesammelt
//! und kanonisch nach dem Regelnamen sortiert, damit der Vergleich nicht von
//! der Auswertungsreihenfolge abhaengt.

use std::fmt;

/// Harte Paketgrenze — identisch mit `framing::MAX_FRAME_BYTES` (v2) und
/// `eqcop::kMaxFrameBytes` im Plugin. Sie gilt fuer `frame_len`, also fuer
/// Kopf **plus** Payload.
pub const MAX_FRAME_BYTES: u32 = 262_144;

/// Laenge des v3-Kopfes hinter dem Laengenpraefix.
pub const KOPF_BYTES: u32 = 16;

/// Groesster heute erlaubter Payload: die Paketgrenze minus Kopf.
pub const MAX_PAYLOAD_BYTES: u32 = MAX_FRAME_BYTES - KOPF_BYTES;

/// Die einzige heute unterstuetzte Major-Version des v3-Vertrags.
pub const SCHEMA_MAJOR: u8 = 3;

/// Bootstrap-Hello: hoechstens 16 KiB, nur `u32`-laengenpraefigiertes JSON
/// (Entwurf §53.9). Groesser ist kein Hello.
pub const MAX_BOOTSTRAP_BYTES: u32 = 16 * 1024;

/// Nutzlastkodierung (`encoding`).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Kodierung {
    Json = 0,
    FlatBuffers = 1,
}

/// Prioritaetsfamilie (`message_family`).
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub enum Familie {
    /// Steuerung: Welcome/Reject, Lease, Apply/Revert, ACK, Heartbeat.
    P0 = 0,
    /// Zustand und Evidenz.
    P1 = 1,
    /// Verlusttolerante Featureframes.
    P2 = 2,
}

impl Familie {
    pub fn aus_byte(b: u8) -> Option<Self> {
        match b {
            0 => Some(Familie::P0),
            1 => Some(Familie::P1),
            2 => Some(Familie::P2),
            _ => None,
        }
    }

    /// Die vom Vertrag vorgeschriebene Kodierung: P0/P1 sind JSON, P2 ist ein
    /// `FeatureBatch` (§33.1 "P0/P1 verwenden JSON; P2 verweist auf ein
    /// FeatureBatch"). Erst dadurch sind "CRC ist fuer P2 Pflicht" und "bei
    /// JSON exakt 0" ueberschneidungsfrei.
    pub fn erwartete_kodierung(self) -> Kodierung {
        match self {
            Familie::P0 | Familie::P1 => Kodierung::Json,
            Familie::P2 => Kodierung::FlatBuffers,
        }
    }
}

impl Kodierung {
    pub fn aus_byte(b: u8) -> Option<Self> {
        match b {
            0 => Some(Kodierung::Json),
            1 => Some(Kodierung::FlatBuffers),
            _ => None,
        }
    }
}

/// Geschlossene Regelmenge. Die Namen sind Vertrag: sie stehen im Manifest,
/// in der C++-Fassung und in `schemas/v3/README.md`.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Verstoss {
    // Stufe 1 — Rahmen
    PraefixUnvollstaendig,
    FrameLenNull,
    FrameLenUnterKopf,
    FrameLenUeberGrenze,
    FrameUnvollstaendig,
    // Stufe 2 — Kopfkonsistenz
    Ueberlaenge,
    Unterlaenge,
    // Stufe 3 — Feldwerte
    CrcBeiJsonNichtNull,
    CrcFalsch,
    EncodingPasstNichtZurFamilie,
    EncodingUnbekannt,
    FlagsReserviert,
    MessageFamilyUnbekannt,
    SchemaMajorUnbekannt,
}

/// Alle Regeln in Deklarationsreihenfolge — die geschlossene Menge, gegen die
/// das Manifest und beide Leser gemessen werden.
pub const ALLE_VERSTOESSE: [Verstoss; 14] = [
    Verstoss::PraefixUnvollstaendig,
    Verstoss::FrameLenNull,
    Verstoss::FrameLenUnterKopf,
    Verstoss::FrameLenUeberGrenze,
    Verstoss::FrameUnvollstaendig,
    Verstoss::Ueberlaenge,
    Verstoss::Unterlaenge,
    Verstoss::CrcBeiJsonNichtNull,
    Verstoss::CrcFalsch,
    Verstoss::EncodingPasstNichtZurFamilie,
    Verstoss::EncodingUnbekannt,
    Verstoss::FlagsReserviert,
    Verstoss::MessageFamilyUnbekannt,
    Verstoss::SchemaMajorUnbekannt,
];

impl Verstoss {
    pub fn name(self) -> &'static str {
        match self {
            Verstoss::PraefixUnvollstaendig => "praefix_unvollstaendig",
            Verstoss::FrameLenNull => "frame_len_null",
            Verstoss::FrameLenUnterKopf => "frame_len_unter_kopf",
            Verstoss::FrameLenUeberGrenze => "frame_len_ueber_grenze",
            Verstoss::FrameUnvollstaendig => "frame_unvollstaendig",
            Verstoss::Ueberlaenge => "ueberlaenge",
            Verstoss::Unterlaenge => "unterlaenge",
            Verstoss::CrcBeiJsonNichtNull => "crc_bei_json_nicht_null",
            Verstoss::CrcFalsch => "crc_falsch",
            Verstoss::EncodingPasstNichtZurFamilie => "encoding_passt_nicht_zur_familie",
            Verstoss::EncodingUnbekannt => "encoding_unbekannt",
            Verstoss::FlagsReserviert => "flags_reserviert",
            Verstoss::MessageFamilyUnbekannt => "message_family_unbekannt",
            Verstoss::SchemaMajorUnbekannt => "schema_major_unbekannt",
        }
    }

    /// Nur fuer die Fixtureseite: aus dem Manifestnamen zurueck. Ein
    /// unbekannter Name ist ein Manifestfehler, kein Parserzustand.
    pub fn aus_name(name: &str) -> Option<Self> {
        ALLE_VERSTOESSE.into_iter().find(|v| v.name() == name)
    }
}

impl fmt::Display for Verstoss {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(self.name())
    }
}

/// Gelesener Kopf eines gueltigen Frames.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Kopf {
    pub encoding: Kodierung,
    pub familie: Familie,
    pub schema_major: u8,
    pub schema_minor: u8,
    pub flags: u32,
    pub payload_len: u32,
    pub crc32c: u32,
}

/// Ein vollstaendig geprueftes Paket.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Rahmen {
    pub kopf: Kopf,
    /// Gesamtlaenge auf dem Draht: 4 + frame_len.
    pub drahtlaenge: usize,
    pub payload: Vec<u8>,
}

/// CRC32C (Castagnoli, reflektiert). Ohne Fremdcrate: der Vertrag braucht
/// genau eine Funktion, und dieselbe Tabelle steht in C++ — eine Crate mehr
/// waere eine zweite Wahrheit ueber ein Polynom.
pub fn crc32c(daten: &[u8]) -> u32 {
    const POLY: u32 = 0x82F6_3B78; // reflektiertes 0x1EDC6F41
    let mut crc: u32 = 0xFFFF_FFFF;
    for &b in daten {
        crc ^= b as u32;
        for _ in 0..8 {
            let low = crc & 1;
            crc >>= 1;
            if low != 0 {
                crc ^= POLY;
            }
        }
    }
    !crc
}

fn u32_le(b: &[u8]) -> u32 {
    u32::from_le_bytes([b[0], b[1], b[2], b[3]])
}

/// Kernpruefung ueber einen Bytepuffer, der mit dem Laengenpraefix beginnt.
///
/// `Ok(Rahmen)` heisst: dieser Frame darf an den Payloadparser. `Err(menge)`
/// traegt die kanonisch sortierte Verstossmenge; sie ist nie leer.
pub fn envelope_pruefen(daten: &[u8]) -> Result<Rahmen, Vec<Verstoss>> {
    // ── Stufe 1: Rahmen ────────────────────────────────────────────────────
    if daten.len() < 4 {
        return Err(vec![Verstoss::PraefixUnvollstaendig]);
    }
    let frame_len = u32_le(&daten[0..4]);
    if frame_len == 0 {
        return Err(vec![Verstoss::FrameLenNull]);
    }
    if frame_len > MAX_FRAME_BYTES {
        return Err(vec![Verstoss::FrameLenUeberGrenze]);
    }
    if frame_len < KOPF_BYTES {
        return Err(vec![Verstoss::FrameLenUnterKopf]);
    }
    let drahtlaenge = 4usize + frame_len as usize;
    if daten.len() < drahtlaenge {
        return Err(vec![Verstoss::FrameUnvollstaendig]);
    }

    // ── Stufe 2: Kopfkonsistenz ───────────────────────────────────────────
    let kopf_bytes = &daten[4..4 + KOPF_BYTES as usize];
    let payload_len = u32_le(&kopf_bytes[8..12]);
    // In u64, NICHT in u32: 16 + 0xFFFFFFFF liefe auf 15 um und ein naiver
    // Parser haette den Frame akzeptiert. Genau dafuer gibt es das Fixture
    // `kopf-payload-len-u32-ueberlauf.bin`.
    let erwartet = KOPF_BYTES as u64 + payload_len as u64;
    let ist = frame_len as u64;
    if ist > erwartet {
        return Err(vec![Verstoss::Ueberlaenge]);
    }
    if ist < erwartet {
        return Err(vec![Verstoss::Unterlaenge]);
    }

    // ── Stufe 3: Feldwerte ────────────────────────────────────────────────
    let mut verstoesse: Vec<Verstoss> = Vec::new();
    let encoding = Kodierung::aus_byte(kopf_bytes[0]);
    let familie = Familie::aus_byte(kopf_bytes[1]);
    let schema_major = kopf_bytes[2];
    let schema_minor = kopf_bytes[3];
    let flags = u32_le(&kopf_bytes[4..8]);
    let crc_soll = u32_le(&kopf_bytes[12..16]);

    if encoding.is_none() {
        verstoesse.push(Verstoss::EncodingUnbekannt);
    }
    if familie.is_none() {
        verstoesse.push(Verstoss::MessageFamilyUnbekannt);
    }
    if schema_major != SCHEMA_MAJOR {
        verstoesse.push(Verstoss::SchemaMajorUnbekannt);
    }
    if flags != 0 {
        verstoesse.push(Verstoss::FlagsReserviert);
    }
    if let (Some(e), Some(f)) = (encoding, familie) {
        if e != f.erwartete_kodierung() {
            verstoesse.push(Verstoss::EncodingPasstNichtZurFamilie);
        }
    }
    // CRC nur bei bekanntem Encoding: ohne Encoding gibt es keine Aussage
    // darueber, ob 0 gefordert oder eine Pruefsumme erwartet ist.
    let payload = &daten[4 + KOPF_BYTES as usize..drahtlaenge];
    if let Some(e) = encoding {
        match e {
            Kodierung::Json => {
                if crc_soll != 0 {
                    verstoesse.push(Verstoss::CrcBeiJsonNichtNull);
                }
            }
            Kodierung::FlatBuffers => {
                if crc_soll != crc32c(payload) {
                    verstoesse.push(Verstoss::CrcFalsch);
                }
            }
        }
    }

    if !verstoesse.is_empty() {
        verstoesse.sort_by_key(|v| v.name());
        verstoesse.dedup();
        return Err(verstoesse);
    }

    Ok(Rahmen {
        kopf: Kopf {
            encoding: encoding.expect("Stufe 3 hat encoding bereits geprueft"),
            familie: familie.expect("Stufe 3 hat familie bereits geprueft"),
            schema_major,
            schema_minor,
            flags,
            payload_len,
            crc32c: crc_soll,
        },
        drahtlaenge,
        payload: payload.to_vec(),
    })
}

/// Schreibt einen v3-Frame. Fuer JSON ist die CRC per Vertrag 0, fuer
/// FlatBuffers wird sie ueber genau die Payloadbytes gerechnet.
pub fn envelope_schreiben(
    familie: Familie,
    schema_minor: u8,
    payload: &[u8],
) -> Result<Vec<u8>, Verstoss> {
    if payload.len() > MAX_PAYLOAD_BYTES as usize {
        return Err(Verstoss::FrameLenUeberGrenze);
    }
    let encoding = familie.erwartete_kodierung();
    let payload_len = payload.len() as u32;
    let crc = match encoding {
        Kodierung::Json => 0,
        Kodierung::FlatBuffers => crc32c(payload),
    };
    let frame_len = KOPF_BYTES + payload_len;
    let mut aus = Vec::with_capacity(4 + frame_len as usize);
    aus.extend_from_slice(&frame_len.to_le_bytes());
    aus.push(encoding as u8);
    aus.push(familie as u8);
    aus.push(SCHEMA_MAJOR);
    aus.push(schema_minor);
    aus.extend_from_slice(&0u32.to_le_bytes()); // flags
    aus.extend_from_slice(&payload_len.to_le_bytes());
    aus.extend_from_slice(&crc.to_le_bytes());
    aus.extend_from_slice(payload);
    Ok(aus)
}

/// Streamender Leser: nimmt beliebig zerteilte Bytes entgegen und liefert
/// ganze Frames. Eine Pipe im Bytemodus liefert keine Nachrichtengrenzen —
/// ein Leser, der nur ganze Puffer versteht, ist im Betrieb keiner.
#[derive(Debug, Default)]
pub struct StromLeser {
    puffer: Vec<u8>,
}

/// Ergebnis eines Leseversuchs.
#[derive(Debug, PartialEq, Eq)]
pub enum LeseErgebnis {
    /// Noch nicht genug Bytes — weiterfuettern.
    Unvollstaendig,
    Frame(Box<Rahmen>),
    /// Verstossmenge; die Verbindung ist danach zu schliessen. Ein
    /// Envelopefehler ist nie "ueberspringbar": nach einem falschen
    /// `frame_len` weiss niemand mehr, wo der naechste Frame beginnt.
    Verstoesse(Vec<Verstoss>),
}

impl StromLeser {
    pub fn neu() -> Self {
        Self { puffer: Vec::new() }
    }

    pub fn fuettern(&mut self, bytes: &[u8]) {
        self.puffer.extend_from_slice(bytes);
    }

    pub fn offen(&self) -> usize {
        self.puffer.len()
    }

    /// Naechster Frame, falls vollstaendig.
    pub fn naechster(&mut self) -> LeseErgebnis {
        match envelope_pruefen(&self.puffer) {
            Ok(rahmen) => {
                self.puffer.drain(0..rahmen.drahtlaenge);
                LeseErgebnis::Frame(Box::new(rahmen))
            }
            Err(v)
                if v == vec![Verstoss::PraefixUnvollstaendig]
                    || v == vec![Verstoss::FrameUnvollstaendig] =>
            {
                LeseErgebnis::Unvollstaendig
            }
            Err(v) => LeseErgebnis::Verstoesse(v),
        }
    }
}

/// Nachrichtenratengrenze (§33.1 "Parser erhalten Tiefen-, Laengen- und
/// Nachrichtenratenlimits"). Gleitendes Fenster ueber eine monotone
/// Millisekundenuhr des Aufrufers — die Struktur liest selbst keine Uhr,
/// damit der Test sie deterministisch fahren kann.
#[derive(Debug)]
pub struct Ratengrenze {
    fenster_ms: u64,
    max_im_fenster: u32,
    zeitpunkte: std::collections::VecDeque<u64>,
}

impl Ratengrenze {
    pub fn neu(max_im_fenster: u32, fenster_ms: u64) -> Self {
        Self {
            fenster_ms,
            max_im_fenster,
            zeitpunkte: std::collections::VecDeque::with_capacity(max_im_fenster as usize + 1),
        }
    }

    /// `true` = darf verarbeitet werden. `false` = Rate ueberschritten.
    pub fn erlaubt(&mut self, jetzt_ms: u64) -> bool {
        while let Some(&aeltester) = self.zeitpunkte.front() {
            if jetzt_ms.saturating_sub(aeltester) >= self.fenster_ms {
                self.zeitpunkte.pop_front();
            } else {
                break;
            }
        }
        if self.zeitpunkte.len() as u32 >= self.max_im_fenster {
            return false;
        }
        self.zeitpunkte.push_back(jetzt_ms);
        true
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn json_frame(text: &str) -> Vec<u8> {
        envelope_schreiben(Familie::P0, 0, text.as_bytes()).unwrap()
    }

    #[test]
    fn crc32c_gegen_bekannte_vektoren() {
        // RFC 3720 Anhang B.4 (iSCSI) — die kanonischen CRC32C-Vektoren.
        assert_eq!(crc32c(b""), 0x0000_0000);
        assert_eq!(crc32c(b"123456789"), 0xE306_9283);
        assert_eq!(crc32c(&[0u8; 32]), 0x8A91_36AA);
        assert_eq!(crc32c(&[0xFFu8; 32]), 0x62A8_AB43);
    }

    #[test]
    fn json_rundlauf() {
        let f = json_frame("{\"type\":\"welcome\"}");
        let r = envelope_pruefen(&f).expect("gueltig");
        assert_eq!(r.kopf.encoding, Kodierung::Json);
        assert_eq!(r.kopf.familie, Familie::P0);
        assert_eq!(r.kopf.crc32c, 0);
        assert_eq!(r.payload, b"{\"type\":\"welcome\"}");
        assert_eq!(r.drahtlaenge, f.len());
    }

    #[test]
    fn p2_traegt_pflicht_crc() {
        let nutz = vec![7u8; 40];
        let f = envelope_schreiben(Familie::P2, 0, &nutz).unwrap();
        let r = envelope_pruefen(&f).unwrap();
        assert_eq!(r.kopf.encoding, Kodierung::FlatBuffers);
        assert_eq!(r.kopf.crc32c, crc32c(&nutz));
        // Ein einziges gekipptes Payloadbyte faellt.
        let mut kaputt = f.clone();
        let letztes = kaputt.len() - 1;
        kaputt[letztes] ^= 0x01;
        assert_eq!(envelope_pruefen(&kaputt), Err(vec![Verstoss::CrcFalsch]));
    }

    #[test]
    fn u32_ueberlauf_wird_nicht_akzeptiert() {
        // payload_len = 0xFFFFFFFF, frame_len = 16: in u32 waere
        // 16 + 0xFFFFFFFF == 15, also "Ueberlaenge" — in u64 ist es
        // Unterlaenge. Der Unterschied im NAMEN entlarvt die naive Rechnung.
        let mut f = vec![0u8; 4 + 16];
        f[0..4].copy_from_slice(&16u32.to_le_bytes());
        f[4] = 0; // JSON
        f[5] = 0; // P0
        f[6] = SCHEMA_MAJOR;
        f[12..16].copy_from_slice(&0xFFFF_FFFFu32.to_le_bytes());
        assert_eq!(envelope_pruefen(&f), Err(vec![Verstoss::Unterlaenge]));
    }

    #[test]
    fn stufe_drei_sammelt_und_sortiert() {
        let mut f = json_frame("{}");
        f[4] = 9; // encoding unbekannt
        f[5] = 9; // family unbekannt
        f[6] = 4; // major unbekannt
        f[8..12].copy_from_slice(&1u32.to_le_bytes()); // flags reserviert
        let v = envelope_pruefen(&f).unwrap_err();
        assert_eq!(
            v.iter().map(|x| x.name()).collect::<Vec<_>>(),
            vec![
                "encoding_unbekannt",
                "flags_reserviert",
                "message_family_unbekannt",
                "schema_major_unbekannt"
            ]
        );
    }

    #[test]
    fn grenzen_des_rahmens() {
        assert_eq!(envelope_pruefen(&[]), Err(vec![Verstoss::PraefixUnvollstaendig]));
        assert_eq!(envelope_pruefen(&[1, 2, 3]), Err(vec![Verstoss::PraefixUnvollstaendig]));
        assert_eq!(
            envelope_pruefen(&0u32.to_le_bytes()),
            Err(vec![Verstoss::FrameLenNull])
        );
        let mut zu_gross = Vec::new();
        zu_gross.extend_from_slice(&(MAX_FRAME_BYTES + 1).to_le_bytes());
        assert_eq!(
            envelope_pruefen(&zu_gross),
            Err(vec![Verstoss::FrameLenUeberGrenze])
        );
        let mut kurz = Vec::new();
        kurz.extend_from_slice(&15u32.to_le_bytes());
        kurz.extend_from_slice(&[0u8; 15]);
        assert_eq!(envelope_pruefen(&kurz), Err(vec![Verstoss::FrameLenUnterKopf]));
        let f = json_frame("{}");
        assert_eq!(
            envelope_pruefen(&f[..f.len() - 1]),
            Err(vec![Verstoss::FrameUnvollstaendig])
        );
    }

    #[test]
    fn stromleser_vertraegt_byteweise_zustellung() {
        let a = json_frame("{\"a\":1}");
        let b = envelope_schreiben(Familie::P2, 0, &[1, 2, 3, 4]).unwrap();
        let mut leser = StromLeser::neu();
        let alles: Vec<u8> = a.iter().chain(b.iter()).copied().collect();
        let mut gefunden = 0;
        for byte in &alles {
            leser.fuettern(&[*byte]);
            loop {
                match leser.naechster() {
                    LeseErgebnis::Frame(_) => gefunden += 1,
                    LeseErgebnis::Unvollstaendig => break,
                    LeseErgebnis::Verstoesse(v) => panic!("unerwartet: {v:?}"),
                }
            }
        }
        assert_eq!(gefunden, 2);
        assert_eq!(leser.offen(), 0);
    }

    #[test]
    fn ratengrenze_haelt_das_fenster() {
        let mut r = Ratengrenze::neu(3, 1000);
        assert!(r.erlaubt(0));
        assert!(r.erlaubt(10));
        assert!(r.erlaubt(20));
        assert!(!r.erlaubt(30), "vierte Nachricht im selben Fenster");
        assert!(r.erlaubt(1000), "Fenster ist weitergerueckt");
    }

    #[test]
    fn jeder_regelname_ist_rueckuebersetzbar() {
        for v in ALLE_VERSTOESSE {
            assert_eq!(Verstoss::aus_name(v.name()), Some(v));
        }
        assert_eq!(Verstoss::aus_name("gibt_es_nicht"), None);
        // Kein Duplikat in der geschlossenen Menge.
        let mut namen: Vec<&str> = ALLE_VERSTOESSE.iter().map(|v| v.name()).collect();
        namen.sort_unstable();
        let anzahl = namen.len();
        namen.dedup();
        assert_eq!(namen.len(), anzahl);
    }
}
