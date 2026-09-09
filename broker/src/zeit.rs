//! Zeitmessung des Brokers: Wanduhr fuer Ausgaben, Monotonie fuer Fristen.
//!
//! ⚠️ **Zwei Uhren, und sie duerfen nicht ineinander laufen.** `jetzt_ms`
//! liest die Wanduhr — sie steht in Ausgaben und Snapshots, weil ein Mensch
//! sie lesen soll. Jede FRIST dagegen misst `monoton_ms`: eine
//! Sommerzeitumstellung oder ein NTP-Sprung darf einen Sensor nicht schlagartig
//! stale machen oder ewig frisch halten. `Zeitstempel` traegt deshalb beide
//! Zahlen nebeneinander statt einer umgerechneten.

use std::sync::OnceLock;

pub fn jetzt_ms() -> u64 {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_millis() as u64)
        .unwrap_or(0)
}

static MONOTONER_START: OnceLock<std::time::Instant> = OnceLock::new();

pub(crate) fn monoton_ms() -> u64 {
    MONOTONER_START
        .get_or_init(std::time::Instant::now)
        .elapsed()
        .as_millis() as u64
}

#[derive(Debug, Clone, Copy)]
pub(crate) struct Zeitstempel {
    pub(crate) utc_ms: u64,
    pub(crate) monoton_ms: u64,
}

impl Zeitstempel {
    pub(crate) fn jetzt() -> Self {
        Self {
            utc_ms: jetzt_ms(),
            monoton_ms: monoton_ms(),
        }
    }
}
