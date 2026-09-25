//! Bandbreiten des eingefrorenen 221er-Feinbandgitters (NAK-380 T-380-2).
//!
//! Die einzige Quelle ist `kanten_hz.hex64` im Gitter-JSON. `include_str!`
//! bindet sie in das Binary; `OnceLock` parst sie genau einmal. Damit gibt es
//! weder eine zweite Zahlenbank noch Dateizugriff zur Laufzeit.

use std::sync::OnceLock;

use super::determinismus::BAENDER_FEIN;

const GITTER_JSON: &str = include_str!(
    "../../../../eq-copilot/schemas/v3/bandgitter/nakama_1_24_oct_30_18k_v1.json"
);

static BREITEN: OnceLock<[f64; BAENDER_FEIN]> = OnceLock::new();

fn tafel() -> &'static [f64; BAENDER_FEIN] {
    BREITEN.get_or_init(|| {
        let gitter: serde_json::Value =
            serde_json::from_str(GITTER_JSON).expect("eingebettetes Feinbandgitter ist JSON");
        let hex64 = gitter["kanten_hz"]["hex64"]
            .as_array()
            .expect("kanten_hz.hex64 ist eine Liste");
        assert_eq!(hex64.len(), BAENDER_FEIN + 1, "222 Gitterkanten");
        let kanten: Vec<f64> = hex64
            .iter()
            .map(|wert| {
                let text = wert.as_str().expect("hex64-Kante ist Text");
                let bits = u64::from_str_radix(text, 16).expect("hex64-Kante hat 16 Hexziffern");
                f64::from_bits(bits)
            })
            .collect();
        std::array::from_fn(|index| kanten[index + 1] - kanten[index])
    })
}

pub(crate) fn breite(index: usize) -> f64 {
    tafel()[index]
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn nak380_m11_bandbreiten_aus_gitter() {
        let werte = tafel();
        assert_eq!(werte.len(), 221);
        assert!(werte.iter().all(|wert| wert.is_finite() && *wert > 0.0));
        let summe: f64 = werte.iter().sum();
        // R-380-8: Kante 221 minus Kante 0 aus kanten_hz.hex64.
        assert!((summe - 17_929.032_399_788_92).abs() < 1e-9, "{summe:.12}");
        assert_eq!(breite(0).to_bits(), 0x3fec_77d2_b45d_de00);
        assert_eq!(breite(120).to_bits(), 0x403c_77d2_b45d_ddc0);
        assert_eq!(breite(220).to_bits(), 0x407f_f44e_07b1_8980);
    }
}
