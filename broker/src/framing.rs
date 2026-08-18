//! Framing des EQ-Copilot-IPC (eq-copilot/schemas/eq-ipc.schema.json):
//! u32-Little-Endian-Längenpräfix, dann UTF-8-JSON genau dieser Länge.
//! Harte Paketgrenze — größer beendet die Verbindung, still verworfen wird nie.

use std::io::{Read, Write};

/// Harte Paketgrenze aus dem Schema. Muss mit eqcop::kMaxFrameBytes im Plugin
/// übereinstimmen — Änderung nur über eine neue Protokollversion.
pub const MAX_FRAME_BYTES: u32 = 262_144;

#[derive(Debug, PartialEq, Eq)]
pub enum FrameFehler {
    /// Sauberes Verbindungsende zwischen zwei Frames.
    Ende,
    /// Längenpräfix 0 oder über der Grenze — Verbindung ist zu beenden.
    Grenze(u32),
    /// Abbruch mitten im Frame oder I/O-Fehler.
    Io(String),
    /// Payload ist kein UTF-8.
    Utf8,
}

pub fn frame_schreiben<W: Write>(ziel: &mut W, json: &str) -> Result<(), FrameFehler> {
    let bytes = json.as_bytes();
    if bytes.len() as u64 > MAX_FRAME_BYTES as u64 {
        return Err(FrameFehler::Grenze(bytes.len() as u32));
    }
    let len = (bytes.len() as u32).to_le_bytes();
    ziel.write_all(&len).map_err(|e| FrameFehler::Io(e.to_string()))?;
    ziel.write_all(bytes).map_err(|e| FrameFehler::Io(e.to_string()))?;
    ziel.flush().map_err(|e| FrameFehler::Io(e.to_string()))
}

pub fn frame_lesen<R: Read>(quelle: &mut R) -> Result<String, FrameFehler> {
    let mut kopf = [0u8; 4];
    // Erste Leseoperation: EOF hier ist ein geordnetes Ende.
    let mut gelesen = 0usize;
    while gelesen < 4 {
        match quelle.read(&mut kopf[gelesen..]) {
            Ok(0) if gelesen == 0 => return Err(FrameFehler::Ende),
            Ok(0) => return Err(FrameFehler::Io("EOF im Längenpräfix".into())),
            Ok(n) => gelesen += n,
            Err(e) => {
                return if gelesen == 0 && ist_pipe_ende(&e) {
                    Err(FrameFehler::Ende)
                } else {
                    Err(FrameFehler::Io(e.to_string()))
                }
            }
        }
    }
    let len = u32::from_le_bytes(kopf);
    if len == 0 || len > MAX_FRAME_BYTES {
        return Err(FrameFehler::Grenze(len));
    }
    let mut daten = vec![0u8; len as usize];
    let mut gelesen = 0usize;
    while gelesen < daten.len() {
        match quelle.read(&mut daten[gelesen..]) {
            Ok(0) => return Err(FrameFehler::Io("EOF im Frame".into())),
            Ok(n) => gelesen += n,
            Err(e) => return Err(FrameFehler::Io(e.to_string())),
        }
    }
    String::from_utf8(daten).map_err(|_| FrameFehler::Utf8)
}

/// Windows meldet ein zugezogenes Pipe-Ende als Fehler 109 (BROKEN_PIPE),
/// nicht als EOF — für uns ist beides dasselbe geordnete Ende.
fn ist_pipe_ende(e: &std::io::Error) -> bool {
    matches!(e.raw_os_error(), Some(109) | Some(233))
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Cursor;

    #[test]
    fn hin_und_zurueck() {
        let mut puffer = Vec::new();
        frame_schreiben(&mut puffer, r#"{"type":"hello"}"#).unwrap();
        frame_schreiben(&mut puffer, "zwei").unwrap();
        let mut c = Cursor::new(puffer);
        assert_eq!(frame_lesen(&mut c).unwrap(), r#"{"type":"hello"}"#);
        assert_eq!(frame_lesen(&mut c).unwrap(), "zwei");
        assert_eq!(frame_lesen(&mut c), Err(FrameFehler::Ende));
    }

    #[test]
    fn laengengrenze_beidseitig() {
        let gross = "x".repeat(MAX_FRAME_BYTES as usize + 1);
        let mut puffer = Vec::new();
        assert!(matches!(frame_schreiben(&mut puffer, &gross), Err(FrameFehler::Grenze(_))));

        // Feindliches Präfix: 0xFFFFFFFF darf keinen Riesen-Alloc auslösen.
        let mut c = Cursor::new(vec![0xff, 0xff, 0xff, 0xff, b'x']);
        assert!(matches!(frame_lesen(&mut c), Err(FrameFehler::Grenze(_))));
        let mut c = Cursor::new(vec![0, 0, 0, 0]);
        assert!(matches!(frame_lesen(&mut c), Err(FrameFehler::Grenze(0))));
    }

    #[test]
    fn abbruch_mitten_im_frame() {
        let mut puffer = Vec::new();
        frame_schreiben(&mut puffer, "vollstaendig").unwrap();
        puffer.truncate(7); // Präfix + 3 Bytes
        let mut c = Cursor::new(puffer);
        assert!(matches!(frame_lesen(&mut c), Err(FrameFehler::Io(_))));
    }

    #[test]
    fn kein_utf8_wird_verworfen() {
        let mut puffer = vec![2, 0, 0, 0, 0xC3, 0x28];
        let mut c = Cursor::new(&mut puffer);
        assert_eq!(frame_lesen(&mut c), Err(FrameFehler::Utf8));
    }
}
