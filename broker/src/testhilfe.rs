//! Die geteilten Testbauer der Brokerwurzel.
//!
//! Ein Hello, ein Messstand und ein Projektfenster in immer derselben Form.
//! Sie stehen hier, weil die Tests des Registers und die der Paarauswertung
//! dieselben Eingaenge brauchen; zwei Fassungen waeren zwei Gelegenheiten,
//! die Grundwerte verschieden zu waehlen.
//!
//! Nur unter `cfg(test)` gebaut.

use crate::protokoll::{AudioAngabe, Hello, MessStand, ProjektFenster, SensorAngabe};

pub(super) fn hello(id: &str, role: &str, label: &str, pair: Option<&str>, pid: u32) -> Hello {
    Hello {
        protocol_version: 2,
        plugin_version: "0.2.0".into(),
        host_pid: pid,
        sensor: SensorAngabe {
            sensor_id: id.into(),
            instance_nonce: None,
            role: role.into(),
            label: label.into(),
            pair_id: pair.map(|s| s.to_string()),
        },
        audio: AudioAngabe {
            samplerate: 48000.0,
            block_size: 512,
            channels: 2,
        },
    }
}

pub(super) fn messbereit(fenster: Option<ProjektFenster>, aktiv_s: f64) -> MessStand {
    MessStand {
        zustand: "messbereit".into(),
        metrics_version: "m1-2026-08-13".into(),
        aktiv_s,
        gesamt_s: aktiv_s,
        projekt_fenster: fenster,
        ..Default::default()
    }
}

pub(super) fn fenster(von: i64, bis: i64, spruenge: u32) -> ProjektFenster {
    ProjektFenster {
        von_samples: von,
        bis_samples: bis,
        spruenge,
    }
}
