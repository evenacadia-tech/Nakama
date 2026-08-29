//! SONDE-010 — die v2-Seite der Transportgrenze (Entwurf §53.4
//! `transport/legacy_v2.rs`).
//!
//! Hier wird NICHTS umgebaut. Der v2-Pfad ist der heutige Produktpfad: das
//! `u32`-Laengenpraefix aus `framing`, danach UTF-8-JSON. Dieses Modul tut
//! genau zwei Dinge:
//!
//! 1. es benennt die v2-Haelfte der Grenze, damit `transport::v3` nicht der
//!    einzige Ort ist, an dem "Transport" steht;
//! 2. es NAGELT die Aussage aus §33.1 fest, die sonst nur im Text stuende:
//!    **"kein v2-Parser interpretiert einen Binaerframe als JSON"**.
//!
//! Punkt 2 ist der eigentliche Inhalt. Die Isolation haelt heute strukturell —
//! `frame_lesen` gibt nur gueltiges UTF-8 zurueck und der v2-Server verlangt
//! danach ein `hello`-Objekt — aber "haelt strukturell" ist eine Behauptung,
//! solange kein Test einen echten v3-Binaerframe hineinwirft. Die Tests unten
//! tun genau das, mit den Frames, die `transport::v3` wirklich schreibt.

pub use crate::framing::{frame_lesen, frame_schreiben, FrameFehler, MAX_FRAME_BYTES};

/// Protokollversion, die der v2-Pfad spricht. Bewusst hier gespiegelt, damit
/// die Transportgrenze beide Versionen an einer Stelle nennt.
pub const PROTOKOLL_VERSION: u32 = crate::protokoll::PROTOKOLL_VERSION;

/// Ein v2-Rahmen ist genau dann verarbeitbar, wenn er sich als JSON-Objekt
/// lesen laesst. Der Server verlangt zusaetzlich den Typ `hello`; diese
/// Funktion ist die schmale, testbare Vorstufe davon.
pub fn ist_v2_lesbar(json: &str) -> bool {
    serde_json::from_str::<serde_json::Value>(json)
        .map(|v| v.is_object())
        .unwrap_or(false)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::transport::v3::{envelope_schreiben, Familie};
    use std::io::Cursor;

    /// Der Kern der v2-Isolation: JEDER v3-Binaerframe faellt im v2-Leser —
    /// entweder schon am UTF-8-Riegel oder spaetestens daran, dass das
    /// Ergebnis kein JSON-Objekt ist. Nie wird er als Nachricht angenommen.
    #[test]
    fn v3_binaerframe_wird_nie_als_v2_json_gedeutet() {
        let faelle: Vec<Vec<u8>> = vec![
            envelope_schreiben(Familie::P0, 0, b"{\"type\":\"hello\"}").unwrap(),
            envelope_schreiben(Familie::P1, 0, b"{\"type\":\"state_report\"}").unwrap(),
            envelope_schreiben(Familie::P2, 0, &[0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x7F]).unwrap(),
            envelope_schreiben(Familie::P2, 0, &vec![0u8; 64]).unwrap(),
        ];

        for (i, frame) in faelle.iter().enumerate() {
            let mut c = Cursor::new(frame.clone());
            match frame_lesen(&mut c) {
                // Der haeufigste Ausgang: der 16-Byte-Kopf ist kein UTF-8.
                Err(FrameFehler::Utf8) => {}
                // Oder der Kopf ist zufaellig UTF-8 — dann darf das Ergebnis
                // niemals ein JSON-Objekt sein.
                Ok(text) => assert!(
                    !ist_v2_lesbar(&text),
                    "Fall {i}: v2-Leser haelt einen v3-Binaerframe fuer ein JSON-Objekt"
                ),
                // Jeder andere Fehler ist ebenfalls eine Ablehnung.
                Err(_) => {}
            }
        }
    }

    /// Die Gegenprobe: derselbe Leser nimmt echtes v2-JSON weiterhin an.
    /// Ohne sie waere der Test oben mit einem kaputten Leser gruen.
    #[test]
    fn echtes_v2_json_geht_weiterhin_durch() {
        let mut puffer = Vec::new();
        frame_schreiben(&mut puffer, "{\"type\":\"hello\",\"protocol_version\":2}").unwrap();
        let mut c = Cursor::new(puffer);
        let text = frame_lesen(&mut c).expect("v2-Frame muss lesbar bleiben");
        assert!(ist_v2_lesbar(&text));
    }

    /// Und andersherum: ein v2-Frame ist fuer den v3-Parser kein gueltiger
    /// Envelope. Der 16-Byte-Kopf faellt an seinen eigenen Regeln — die
    /// Grenze ist also in BEIDE Richtungen dicht.
    #[test]
    fn v2_json_ist_kein_gueltiger_v3_envelope() {
        let mut puffer = Vec::new();
        frame_schreiben(&mut puffer, "{\"type\":\"hello\",\"protocol_version\":2}").unwrap();
        let ergebnis = crate::transport::v3::envelope_pruefen(&puffer);
        assert!(
            ergebnis.is_err(),
            "ein v2-JSON-Frame darf den v3-Envelopepruefer nie passieren"
        );
    }
}
