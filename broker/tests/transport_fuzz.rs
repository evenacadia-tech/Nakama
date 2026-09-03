//! `transport_fuzz` (Entwurf §66.2) — Laengen, Header, Flags, CRC,
//! Fragmentierung und Ratengrenzen des v3-Envelope.
//!
//! Der Zufall ist deterministisch (xorshift64* mit festem Saatgut): ein Fund
//! muss reproduzierbar sein, sonst ist er kein Beweis, sondern eine Anekdote.
//! Ein Fehlschlag druckt Saatgut und Bytes.
//!
//! Der Test prueft NICHT, dass bestimmte Eingaben fallen — das tut der
//! Fixture-Korpus. Er prueft die INVARIANTEN: der Parser panickt nie, er
//! alloziert nie nach einer angekuendigten Groesse, und was er annimmt,
//! erfuellt jede Kopfregel.

use eqcop_broker::transport::v3::{
    crc32c, envelope_pruefen, envelope_schreiben, Familie, Kodierung, LeseErgebnis, Ratengrenze,
    StromLeser, Verstoss, KOPF_BYTES, MAX_FRAME_BYTES, MAX_PAYLOAD_BYTES, SCHEMA_MAJOR,
};

struct Zufall(u64);

impl Zufall {
    fn neu(saat: u64) -> Self {
        Self(saat | 1)
    }
    fn naechste(&mut self) -> u64 {
        // xorshift64*
        let mut x = self.0;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        self.0 = x;
        x.wrapping_mul(0x2545_F491_4F6C_DD1D)
    }
    fn bis(&mut self, n: u64) -> u64 {
        if n == 0 {
            0
        } else {
            self.naechste() % n
        }
    }
    fn bytes(&mut self, n: usize) -> Vec<u8> {
        (0..n).map(|_| (self.naechste() & 0xFF) as u8).collect()
    }
}

fn gueltiger_frame(z: &mut Zufall) -> Vec<u8> {
    let familie = match z.bis(3) {
        0 => Familie::P0,
        1 => Familie::P1,
        _ => Familie::P2,
    };
    let laenge = z.bis(200) as usize;
    let payload = match familie.erwartete_kodierung() {
        Kodierung::Json => {
            let mut p = b"{\"f\":\"".to_vec();
            p.extend(std::iter::repeat_n(b'x', laenge));
            p.extend_from_slice(b"\"}");
            p
        }
        Kodierung::FlatBuffers => z.bytes(laenge + 4),
    };
    envelope_schreiben(familie, (z.bis(256)) as u8, &payload).expect("Laenge im Rahmen")
}

/// Alles, was der Pruefer annimmt, erfuellt jede einzelne Kopfregel. Diese
/// Behauptung ist die eigentliche Sicherheitsaussage: ein Payloadparser darf
/// sich darauf verlassen.
fn angenommenes_ist_wohlgeformt(roh: &[u8]) {
    if let Ok(r) = envelope_pruefen(roh) {
        let frame_len = u32::from_le_bytes([roh[0], roh[1], roh[2], roh[3]]);
        assert!(frame_len >= KOPF_BYTES && frame_len <= MAX_FRAME_BYTES);
        assert_eq!(frame_len as u64, KOPF_BYTES as u64 + r.kopf.payload_len as u64);
        assert_eq!(r.payload.len(), r.kopf.payload_len as usize);
        assert_eq!(r.kopf.flags, 0);
        assert_eq!(r.kopf.schema_major, SCHEMA_MAJOR);
        assert_eq!(r.kopf.encoding, r.kopf.familie.erwartete_kodierung());
        match r.kopf.encoding {
            Kodierung::Json => assert_eq!(r.kopf.crc32c, 0),
            Kodierung::FlatBuffers => assert_eq!(r.kopf.crc32c, crc32c(&r.payload)),
        }
        assert_eq!(r.drahtlaenge, 4 + frame_len as usize);
    }
}

#[test]
fn zufallsbytes_bringen_den_pruefer_nie_aus_dem_tritt() {
    let mut z = Zufall::neu(0x0000_0010_5017_DE01);
    for _ in 0..20_000 {
        let n = z.bis(40) as usize;
        let roh = z.bytes(n);
        angenommenes_ist_wohlgeformt(&roh);
    }
}

#[test]
fn einzelne_gekippte_bytes_werden_nie_still_angenommen() {
    let mut z = Zufall::neu(0x9E37_79B9_7F4A_7C15);
    let mut angenommen = 0u32;
    let mut abgelehnt = 0u32;
    for _ in 0..20_000 {
        let gut = gueltiger_frame(&mut z);
        let mut kaputt = gut.clone();
        let pos = z.bis(kaputt.len() as u64) as usize;
        let bit = 1u8 << z.bis(8);
        kaputt[pos] ^= bit;
        angenommenes_ist_wohlgeformt(&kaputt);
        match envelope_pruefen(&kaputt) {
            Ok(_) => angenommen += 1,
            Err(v) => {
                assert!(!v.is_empty(), "Ablehnung ohne Regel");
                abgelehnt += 1;
            }
        }
    }
    // Ein gekipptes Bit IM JSON-PAYLOAD eines P0/P1-Frames aendert den
    // Envelope nicht — der Envelope urteilt nie ueber Inhalt. Deshalb ist
    // "manche werden angenommen" richtig; die Invariante oben ist die Aussage.
    assert!(abgelehnt > 0 && angenommen > 0, "{angenommen} angenommen, {abgelehnt} abgelehnt");
    println!("{angenommen} angenommen (JSON-Payloadbits), {abgelehnt} abgelehnt");
}

#[test]
fn jedes_gekippte_payloadbit_eines_p2_frames_faellt_an_der_crc() {
    let mut z = Zufall::neu(0xDEAD_BEEF_0000_0001);
    for _ in 0..3_000 {
        let n = 1 + z.bis(120) as usize;
        let payload = z.bytes(n);
        let gut = envelope_schreiben(Familie::P2, 0, &payload).unwrap();
        let mut kaputt = gut.clone();
        let pos = 4 + KOPF_BYTES as usize + z.bis(payload.len() as u64) as usize;
        kaputt[pos] ^= 1u8 << z.bis(8);
        assert_eq!(
            envelope_pruefen(&kaputt),
            Err(vec![Verstoss::CrcFalsch]),
            "gekipptes P2-Payloadbit an {pos} nicht gefangen"
        );
    }
}

#[test]
fn feindliche_laengen_enden_in_der_erwarteten_fehlerklasse() {
    // Ein 0xFFFFFFFF-Praefix darf keinen Riesenpuffer anfordern — dieselbe
    // Aussage wie im v2-Framing, hier fuer den v3-Kopf.
    //
    // Was dieser Test MISST (Matrix `D-A21-01`, Regel 7): die Fehlerklasse.
    // Er zaehlt KEINE Allokationen. Der alte Name behauptete mehr als die
    // Messung trug — `envelope_pruefen` erzeugt auf diesen Pfaden je ein
    // `Vec<Verstoss>`, und eine neue, begrenzte Allokation anhand von
    // `frame_len` bliebe hier gruen. Belegt ist: kein Absturz und keine
    // Eingabeallokation in Groesse der behaupteten Laenge, weil die Laenge vor
    // jeder Pufferanforderung faellt.
    for frame_len in [
        0u32,
        1,
        15,
        MAX_FRAME_BYTES + 1,
        0x7FFF_FFFF,
        0xFFFF_FFFF,
    ] {
        let mut roh = frame_len.to_le_bytes().to_vec();
        roh.extend_from_slice(&[0u8; 16]);
        let e = envelope_pruefen(&roh).unwrap_err();
        assert!(
            matches!(
                e.as_slice(),
                [Verstoss::FrameLenNull]
                    | [Verstoss::FrameLenUnterKopf]
                    | [Verstoss::FrameLenUeberGrenze]
            ),
            "frame_len {frame_len} liefert {e:?}"
        );
    }
    // Und eine payload_len an der u32-Grenze wird in u64 gerechnet.
    for payload_len in [0xFFFF_FFFFu32, 0xFFFF_FFF0, 0x8000_0000, MAX_PAYLOAD_BYTES + 1] {
        let mut roh = 16u32.to_le_bytes().to_vec();
        roh.extend_from_slice(&[0, 0, SCHEMA_MAJOR, 0]);
        roh.extend_from_slice(&0u32.to_le_bytes());
        roh.extend_from_slice(&payload_len.to_le_bytes());
        roh.extend_from_slice(&0u32.to_le_bytes());
        assert_eq!(
            envelope_pruefen(&roh),
            Err(vec![Verstoss::Unterlaenge]),
            "payload_len {payload_len}"
        );
    }
}

#[test]
fn fragmentierung_aendert_das_ergebnis_nie() {
    let mut z = Zufall::neu(0x00C0_FFEE_0000_0007);
    for runde in 0..300 {
        let anzahl = 1 + z.bis(5) as usize;
        let frames: Vec<Vec<u8>> = (0..anzahl).map(|_| gueltiger_frame(&mut z)).collect();
        let strom: Vec<u8> = frames.iter().flat_map(|f| f.iter().copied()).collect();

        // 1) Byteweise
        let mut leser = StromLeser::neu();
        let mut gelesen: Vec<Vec<u8>> = Vec::new();
        for b in &strom {
            leser.fuettern(&[*b]);
            loop {
                match leser.naechster() {
                    LeseErgebnis::Frame(r) => gelesen.push(r.payload.clone()),
                    LeseErgebnis::Unvollstaendig => break,
                    LeseErgebnis::Verstoesse(v) => panic!("Runde {runde}: {v:?}"),
                }
            }
        }
        let soll: Vec<Vec<u8>> = frames
            .iter()
            .map(|f| envelope_pruefen(f).unwrap().payload)
            .collect();
        assert_eq!(gelesen, soll, "Runde {runde}: byteweise");

        // 2) In zufaelligen Haeppchen
        let mut leser = StromLeser::neu();
        let mut gelesen2: Vec<Vec<u8>> = Vec::new();
        let mut i = 0usize;
        while i < strom.len() {
            let n = 1 + z.bis(37) as usize;
            let ende = (i + n).min(strom.len());
            leser.fuettern(&strom[i..ende]);
            i = ende;
            loop {
                match leser.naechster() {
                    LeseErgebnis::Frame(r) => gelesen2.push(r.payload.clone()),
                    LeseErgebnis::Unvollstaendig => break,
                    LeseErgebnis::Verstoesse(v) => panic!("Runde {runde}: {v:?}"),
                }
            }
        }
        assert_eq!(gelesen2, soll, "Runde {runde}: Haeppchen");
        assert_eq!(leser.offen(), 0, "Runde {runde}: Rest im Puffer");
    }
}

#[test]
fn ein_kaputter_frame_im_strom_beendet_den_strom() {
    // Nach einem falschen frame_len weiss niemand mehr, wo der naechste Frame
    // beginnt. Der Leser darf deshalb nicht "resynchronisieren" — er muss die
    // Verbindung fallen lassen.
    let gut = envelope_schreiben(Familie::P0, 0, b"{}").unwrap();
    let mut kaputt = gut.clone();
    kaputt[4] = 9; // encoding unbekannt
    let mut strom = kaputt.clone();
    strom.extend_from_slice(&gut);

    let mut leser = StromLeser::neu();
    leser.fuettern(&strom);
    match leser.naechster() {
        LeseErgebnis::Verstoesse(v) => assert_eq!(v, vec![Verstoss::EncodingUnbekannt]),
        andere => panic!("erwartet Verstoss, bekam {andere:?}"),
    }
}

#[test]
fn ratengrenze_haelt_unter_flut() {
    let mut r = Ratengrenze::neu(1000, 1000);
    let mut durch = 0u32;
    // 5000 Nachrichten in derselben Millisekunde: genau 1000 duerfen durch.
    for _ in 0..5000 {
        if r.erlaubt(0) {
            durch += 1;
        }
    }
    assert_eq!(durch, 1000);
    // Ein volles Fenster spaeter wieder 1000.
    let mut durch2 = 0u32;
    for _ in 0..5000 {
        if r.erlaubt(1000) {
            durch2 += 1;
        }
    }
    assert_eq!(durch2, 1000);
    // Gleitend: bei 1 Nachricht je Millisekunde laeuft es unbegrenzt.
    let mut r2 = Ratengrenze::neu(10, 100);
    for t in 0..10_000u64 {
        assert!(r2.erlaubt(t * 10), "t={t}");
    }
}

#[test]
fn schreiben_und_pruefen_sind_zueinander_invers() {
    let mut z = Zufall::neu(0x1234_5678_9ABC_DEF0);
    for _ in 0..5_000 {
        let f = gueltiger_frame(&mut z);
        let r = envelope_pruefen(&f).expect("selbst geschriebener Frame muss gueltig sein");
        let neu = envelope_schreiben(r.kopf.familie, r.kopf.schema_minor, &r.payload).unwrap();
        assert_eq!(neu, f, "Schreiben ist nicht invers zum Lesen");
    }
}

#[test]
fn payload_an_der_paketgrenze_wird_angenommen_darueber_nicht() {
    let genau = vec![0u8; MAX_PAYLOAD_BYTES as usize];
    let f = envelope_schreiben(Familie::P2, 0, &genau).unwrap();
    assert_eq!(f.len(), 4 + MAX_FRAME_BYTES as usize);
    assert!(envelope_pruefen(&f).is_ok());

    let zu_gross = vec![0u8; MAX_PAYLOAD_BYTES as usize + 1];
    assert_eq!(
        envelope_schreiben(Familie::P2, 0, &zu_gross),
        Err(Verstoss::FrameLenUeberGrenze)
    );
}

// ── NAK-121 H-19 ───────────────────────────────────────────────────────────
//
// Das Joinergebnis eines v3-Verbindungsthreads wird ausgewertet. Ohne diesen
// Zaehler entfiele das Erkennungssignal fuer genau die Fehlerklasse, die dieser
// Endpunkt jagt - und zwar dort, wo angreiferkontrollierte Bytes geparst
// werden. Das v2-Ende wertet sein Joinergebnis laengst aus.

#[cfg(windows)]
#[test]
fn panik_im_verbindungsthread_wird_gezaehlt() {
    use eqcop_broker::transport::pipetoken::PROBE_PRAEFIX;
    use eqcop_broker::transport::server_v3::{
        v3_server_starten_fuer_security_vectors, V3SecurityTestOptionen, ZaehlSenke,
    };
    use std::sync::atomic::Ordering;
    use std::sync::Arc;
    use std::time::{Duration, Instant};

    let pipe = format!("{PROBE_PRAEFIX}fuzz-h19-{}", std::process::id());
    let senke = Arc::new(ZaehlSenke::default());
    let mut griff = v3_server_starten_fuer_security_vectors(
        &pipe,
        senke,
        V3SecurityTestOptionen {
            verbindungsthread_panik: true,
            ..V3SecurityTestOptionen::default()
        },
    )
    .expect("Probe-Listener starten");

    // Der Panik-Hook wird bewusst NICHT ersetzt: die Meldung auf stderr gehoert
    // zum Beweis, dass die Panik wirklich fiel und nicht wegoptimiert wurde.
    let client = std::fs::OpenOptions::new()
        .read(true)
        .write(true)
        .open(&pipe)
        .expect("Probe-Client verbindet");

    let ende = Instant::now() + Duration::from_secs(10);
    while Instant::now() < ende
        && griff
            .statistik
            .verbindungsthread_panik
            .load(Ordering::SeqCst)
            == 0
    {
        std::thread::sleep(Duration::from_millis(20));
    }
    assert_eq!(
        griff
            .statistik
            .verbindungsthread_panik
            .load(Ordering::SeqCst),
        1,
        "die Panik im Verbindungsthread wurde beim Ernten nicht gezaehlt"
    );
    drop(client);
    griff.stoppen();
}

// ── NAK-121 H-11 ───────────────────────────────────────────────────────────
//
// Rueckstau wird in Slots UND Bytes gemessen. 256 Slots sagen nichts ueber den
// belegten Speicher: ein Peer, der grosse Frames einreiht, bleibt unter der
// Slotgrenze und belegt trotzdem beliebig viel. Ueberschreitet das Bytebudget,
// gilt dieselbe Politik wie bei Slot-voll (Entwurf Paragraph 53.9): erst den
// aeltesten P2 verwerfen, dann bei P0 oder P1 den Client trennen.

#[test]
fn ingress_bytebudget_verwirft_p2_zuerst() {
    use eqcop_broker::transport::warteschlange::{
        IngressErgebnis, IngressWarteschlange, CAP_INGRESS, CAP_INGRESS_BYTES,
    };

    // Das Produktionsbudget haengt an der groessten zulaessigen Payload; eine
    // volle Slotqueue kann nie mehr halten.
    assert_eq!(
        CAP_INGRESS_BYTES,
        CAP_INGRESS * eqcop_broker::transport::v3::MAX_PAYLOAD_BYTES as usize
    );

    let rahmen = |bytes: usize| (Familie::P2, 1u8, vec![0u8; bytes]);
    // Reichlich Slots, knappes Bytebudget: so misst der Test die BYTE-Achse
    // und nicht versehentlich die Slotachse.
    let mut q: IngressWarteschlange<(Familie, u8, Vec<u8>)> =
        IngressWarteschlange::mit_kapazitaet_und_bytebudget(64, 1000);

    // Am Budget geht es noch.
    assert_eq!(
        q.einreihen(Familie::P2, rahmen(600)),
        IngressErgebnis::Eingereiht
    );
    assert_eq!(q.belegte_bytes(), 600);
    assert_eq!(
        q.einreihen(Familie::P2, rahmen(400)),
        IngressErgebnis::Eingereiht
    );
    assert_eq!(q.belegte_bytes(), 1000, "genau am Budget");

    // Budget plus eins: der aelteste P2 faellt, der Neuzugang kommt rein.
    assert_eq!(q.len(), 2);
    assert_eq!(
        q.einreihen(Familie::P2, rahmen(1)),
        IngressErgebnis::P2Verworfen
    );
    assert_eq!(q.p2_verworfen(), 1);
    assert!(q.belegte_bytes() <= 1000);

    // P0 im vollen Bytebudget trennt, statt still zu verwerfen - und P1
    // ebenso, weil sein Wiederholweg ueber Reconnect/Outbox laeuft.
    let mut voll: IngressWarteschlange<(Familie, u8, Vec<u8>)> =
        IngressWarteschlange::mit_kapazitaet_und_bytebudget(64, 100);
    assert_eq!(
        voll.einreihen(Familie::P0, (Familie::P0, 1, vec![0u8; 100])),
        IngressErgebnis::Eingereiht
    );
    assert_eq!(
        voll.einreihen(Familie::P0, (Familie::P0, 1, vec![0u8; 1])),
        IngressErgebnis::ClientTrennen,
        "P0 ueber dem Bytebudget muss trennen"
    );
    assert_eq!(
        voll.einreihen(Familie::P1, (Familie::P1, 1, vec![0u8; 1])),
        IngressErgebnis::ClientTrennen,
        "P1 faellt nie still"
    );
    assert_eq!(voll.p1_ueberlauf_trennt(), 1);

    // Entnehmen gibt die Bytes wieder frei - sonst waere das Budget eine
    // Einbahnstrasse und die Verbindung nach einem Rueckstau tot.
    let mut frei: IngressWarteschlange<(Familie, u8, Vec<u8>)> =
        IngressWarteschlange::mit_kapazitaet_und_bytebudget(64, 100);
    assert_eq!(
        frei.einreihen(Familie::P2, rahmen(100)),
        IngressErgebnis::Eingereiht
    );
    assert_eq!(frei.belegte_bytes(), 100);
    assert!(frei.entnehmen().is_some());
    assert_eq!(frei.belegte_bytes(), 0);
    assert_eq!(
        frei.einreihen(Familie::P2, rahmen(100)),
        IngressErgebnis::Eingereiht
    );
}
