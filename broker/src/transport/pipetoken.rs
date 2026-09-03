//! SONDE-010 — v3-Pipename als reine Funktion (Entwurf §48.3).
//!
//! `base32(first_128_bits(SHA-256("evenacadia.nakama|v3|" + uppercase_sid_utf8)))`
//! ergibt den Token, daraus wird `\\.\pipe\evenacadia.nakama.v3.<token>`.
//! RFC-4648-Base32, grossgeschrieben, ohne Padding.
//!
//! Der Token vermeidet, die rohe Windows-User-SID im Pipenamen offenzulegen —
//! er ist KEIN Geheimnis und ersetzt keine DACL. Die Zugriffsentscheidung
//! trifft weiterhin die Sicherheitsbeschreibung der Pipe (§48.4).
//!
//! Das Golden aus §48.3 bindet drei Dinge gleichzeitig fest, die man einzeln
//! nicht sieht: die UTF-8-Bytefolge des Praefix, die Digestreihenfolge (erste
//! 128 Bit = erste 16 Bytes) und das Alphabet. Dieselbe Funktion und dasselbe
//! Golden stehen in `eq-copilot/plugin/core/ipc/PipeToken.{h,cpp}`.

use sha2::{Digest, Sha256};

/// Praefix des Digests. Aendert er sich, aendern sich ALLE Pipenamen — das ist
/// eine Protokollaenderung, keine Kosmetik.
pub const DOMAENE: &str = "evenacadia.nakama|v3|";

/// Namensraum der v3-Pipes.
pub const PIPE_PRAEFIX: &str = r"\\.\pipe\evenacadia.nakama.v3.";

/// Namensraum der PROBE-Pipes. Alles darunter gehoert Tests und Beweislaeufen;
/// nichts darunter kann je ein produktiver v3-Endpunkt sein, weil ein
/// SID-Token ein 26-stelliges Base32-Wort ist und nie mit `probe.` beginnt.
pub const PROBE_PRAEFIX: &str = r"\\.\pipe\evenacadia.nakama.v3.probe.";

/// Darf ein Probeprogramm diesen Pipenamen oeffnen?
///
/// Die Frage ist bewusst als ERLAUBNIS gestellt und nicht als Sperrliste: eine
/// Sperrliste kennt nur die Namen, an die jemand gedacht hat — sie liess den
/// produktiven v3-Namensraum aus §48.3 durch (T2-Befund 7 vom 2026-08-29).
/// Erlaubt ist genau `\\.\pipe\evenacadia.nakama.v3.probe.<etwas>`; der Rest
/// hinter dem Praefix darf nicht leer sein, damit `probe.` allein keine
/// Sammelpipe wird.
pub fn ist_probe_pipename(name: &str) -> bool {
    match normalisierter_pipename(name).strip_prefix(&PROBE_PRAEFIX.to_ascii_lowercase()) {
        Some(rest) => !rest.is_empty(),
        None => false,
    }
}

/// G3-STRCMP-001: bringt einen Pipenamen in die Form, in der ueber ihn
/// entschieden werden darf.
///
/// Windows behandelt Pipenamen ohne Ruecksicht auf Gross- und Kleinschreibung
/// und akzeptiert die Praefixform `\\?\pipe\` als gleichwertig zu `\\.\pipe\`.
/// Ein roher Praefixvergleich auf dem nicht normalisierten Namen entscheidet
/// deshalb ueber eine andere Zeichenkette, als das Betriebssystem spaeter
/// oeffnet: `\\.\PIPE\evenacadia.nakama.v3.PROBE.x` waere fuer den Riegel kein
/// Probename und fuer den Kernel dieselbe Pipe. Die Erlaubnisform bleibt, was
/// der Kommentar oben zusagt - eine Erlaubnis, keine Sperrliste.
fn normalisierter_pipename(name: &str) -> String {
    let klein = name.to_ascii_lowercase();
    match klein.strip_prefix(r"\\?\pipe\") {
        Some(rest) => format!(r"\\.\pipe\{rest}"),
        None => klein,
    }
}

const ALPHABET: &[u8; 32] = b"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

/// RFC-4648-Base32 ohne Padding, Grossbuchstaben.
fn base32(daten: &[u8]) -> String {
    let mut aus = String::with_capacity(daten.len().div_ceil(5) * 8);
    let mut puffer: u32 = 0;
    let mut bits: u32 = 0;
    for &b in daten {
        puffer = (puffer << 8) | b as u32;
        bits += 8;
        while bits >= 5 {
            bits -= 5;
            aus.push(ALPHABET[((puffer >> bits) & 0x1F) as usize] as char);
        }
    }
    if bits > 0 {
        // Restbits linksbuendig auffuellen — genau das tut RFC 4648, bevor
        // das Padding drankaeme; wir lassen das Padding weg.
        aus.push(ALPHABET[((puffer << (5 - bits)) & 0x1F) as usize] as char);
    }
    aus
}

/// Pipe-Token zu einer Logon-SID.
pub fn pipe_token(sid: &str) -> String {
    let mut hasher = Sha256::new();
    hasher.update(DOMAENE.as_bytes());
    hasher.update(sid.to_uppercase().as_bytes());
    let digest = hasher.finalize();
    base32(&digest[..16])
}

/// Vollstaendiger v3-Pipename zu einer Logon-SID.
pub fn pipe_name_v3(sid: &str) -> String {
    format!("{PIPE_PRAEFIX}{}", pipe_token(sid))
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Das Golden aus Entwurf §48.3, woertlich.
    #[test]
    fn golden_aus_dem_entwurf() {
        assert_eq!(
            pipe_token("S-1-5-21-111111111-222222222-333333333-1001"),
            "BNSM62JZZCCXIDV3PJZAEHMZPA"
        );
        assert_eq!(
            pipe_name_v3("S-1-5-21-111111111-222222222-333333333-1001"),
            r"\\.\pipe\evenacadia.nakama.v3.BNSM62JZZCCXIDV3PJZAEHMZPA"
        );
    }

    /// Der Riegel der Probeprogramme. Er muss den GOLDEN-Namen abweisen —
    /// genau den hat die alte, nur auf v1 gerichtete Sperre durchgelassen.
    #[test]
    fn probe_namensraum_laesst_nur_probe_namen_durch() {
        assert!(!ist_probe_pipename(&pipe_name_v3(
            "S-1-5-21-111111111-222222222-333333333-1001"
        )));
        assert!(!ist_probe_pipename(
            r"\\.\pipe\evenacadia.nakama.v3.BNSM62JZZCCXIDV3PJZAEHMZPA"
        ));
        assert!(!ist_probe_pipename(r"\\.\pipe\evenacadia.eq-copilot.v1"));
        assert!(!ist_probe_pipename(r"\\.\pipe\evenacadia.nakama.v3.last.4711"));
        assert!(!ist_probe_pipename(PROBE_PRAEFIX), "`probe.` allein ist keine Pipe");
        assert!(ist_probe_pipename(
            r"\\.\pipe\evenacadia.nakama.v3.probe.last.4711.1756400000"
        ));
    }

    /// G3-STRCMP-001: der Riegel entscheidet ueber den Namen, den Windows
    /// spaeter oeffnet - nicht ueber die rohe Zeichenkette. Beide Richtungen
    /// zaehlen: ein anders geschriebener Probename wurde vorher abgewiesen,
    /// obwohl es dieselbe Pipe ist, und ein gross geschriebener
    /// PRODUKTIONSname darf die Erlaubnis auch normalisiert nicht passieren.
    #[test]
    fn probe_riegel_ist_unempfindlich_gegen_schreibweise_und_praefixform() {
        assert!(ist_probe_pipename(
            r"\\.\PIPE\EVENACADIA.NAKAMA.V3.PROBE.last.4711"
        ));
        assert!(ist_probe_pipename(
            r"\\?\pipe\evenacadia.nakama.v3.probe.last.4711"
        ));
        assert!(ist_probe_pipename(r"\\?\PIPE\Evenacadia.Nakama.V3.Probe.x"));

        // Die Erlaubnis bleibt eine Erlaubnis: der Produktionsnamensraum faellt
        // in JEDER Schreibweise und Praefixform.
        assert!(!ist_probe_pipename(
            r"\\.\PIPE\EVENACADIA.NAKAMA.V3.BNSM62JZZCCXIDV3PJZAEHMZPA"
        ));
        assert!(!ist_probe_pipename(
            r"\\?\pipe\evenacadia.nakama.v3.bnsm62jzzccxidv3pjzaehmzpa"
        ));
        assert!(!ist_probe_pipename(r"\\?\PIPE\EVENACADIA.EQ-COPILOT.V1"));
        // `probe.` allein bleibt keine Pipe, auch normalisiert nicht.
        assert!(!ist_probe_pipename(r"\\?\PIPE\EVENACADIA.NAKAMA.V3.PROBE."));
    }

    /// Die SID wird VOR dem Hashen grossgeschrieben — sonst haetten zwei
    /// Schreibweisen derselben SID zwei verschiedene Pipes.
    #[test]
    fn schreibweise_der_sid_ist_egal() {
        let a = pipe_token("s-1-5-21-111111111-222222222-333333333-1001");
        let b = pipe_token("S-1-5-21-111111111-222222222-333333333-1001");
        assert_eq!(a, b);
    }

    #[test]
    fn verschiedene_sids_verschiedene_token() {
        let a = pipe_token("S-1-5-21-111111111-222222222-333333333-1001");
        let b = pipe_token("S-1-5-21-111111111-222222222-333333333-1002");
        assert_ne!(a, b);
    }

    /// 128 Bit ergeben in Base32 genau 26 Zeichen aus dem Alphabet — kein
    /// Padding, keine Kleinbuchstaben, nichts Ausgefallenes im Pipenamen.
    #[test]
    fn tokenform_ist_stabil() {
        let t = pipe_token("S-1-5-21-1-2-3-4");
        assert_eq!(t.len(), 26);
        assert!(t.bytes().all(|c| ALPHABET.contains(&c)));
    }

    /// Base32 gegen die Testvektoren aus RFC 4648 §10 (ohne Padding).
    #[test]
    fn base32_gegen_rfc_4648() {
        assert_eq!(base32(b""), "");
        assert_eq!(base32(b"f"), "MY");
        assert_eq!(base32(b"fo"), "MZXQ");
        assert_eq!(base32(b"foo"), "MZXW6");
        assert_eq!(base32(b"foob"), "MZXW6YQ");
        assert_eq!(base32(b"fooba"), "MZXW6YTB");
        assert_eq!(base32(b"foobar"), "MZXW6YTBOI");
    }
}
