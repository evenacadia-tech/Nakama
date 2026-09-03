//! Monotone Zeit, getrennt von der Anzeigezeit.
//!
//! Der Coordinator misst Fristen ausschliesslich monoton; die Wanduhr taucht
//! nur dort auf, wo ein persistierter Zeitstempel sie verlangt.

use std::sync::atomic::{AtomicU64, Ordering};
use std::time::{Duration, Instant};

pub trait MonotonicClock: Send + Sync {
    fn jetzt(&self) -> Duration;
}

#[derive(Debug)]
pub(super) struct InstantClock {
    start: Instant,
}

impl Default for InstantClock {
    fn default() -> Self {
        Self {
            start: Instant::now(),
        }
    }
}

impl MonotonicClock for InstantClock {
    fn jetzt(&self) -> Duration {
        self.start.elapsed()
    }
}

/// Schlaflose Testuhr. Produktion konstruiert den Coordinator ausschliesslich
/// mit `InstantClock`.
#[doc(hidden)]
#[derive(Debug, Default)]
pub struct ManualClock {
    millis: AtomicU64,
}

impl ManualClock {
    pub fn setze_ms(&self, millis: u64) {
        self.millis.store(millis, Ordering::SeqCst);
    }

    pub fn vor(&self, millis: u64) {
        self.millis.fetch_add(millis, Ordering::SeqCst);
    }
}

impl MonotonicClock for ManualClock {
    fn jetzt(&self) -> Duration {
        Duration::from_millis(self.millis.load(Ordering::SeqCst))
    }
}
