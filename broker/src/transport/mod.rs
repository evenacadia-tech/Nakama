//! SONDE-010 — die Transportgrenze des Brokers (Entwurf §53.4).
//!
//! ```text
//!   legacy_v2   der heutige v2-Pfad, unveraendert, plus der Beweis der Isolation
//!   v3          16-Byte-Envelope, CRC32C, Stromleser, Ratengrenze
//!   bootstrap   Bootstrap-Hello, Protokollteilung, Kopplung Control<->Telemetry
//!   warteschlange  die vier Backpressure-Politiken aus §53.9
//!   pipetoken   SID -> v3-Pipename als reine Funktion (§48.3, mit Golden)
//!   server_v3   Listener + I/O-Worker: Envelope, Grenzen, Authentisierung
//! ```
//!
//! Was hier bewusst NICHT liegt: Session, Eviction, Store und Outbox. Der
//! `Coordinator` ist SONDE-011. Die I/O-Worker geben typisierte Ereignisse an
//! eine schmale `Senke` weiter und wissen nichts ueber deren Bedeutung
//! (§53.9 "I/O-Worker decodieren nur Envelope, Grenzen und Authentisierung").

pub mod bootstrap;
pub mod legacy_v2;
pub mod pipetoken;
pub mod v3;
pub mod warteschlange;

#[cfg(windows)]
pub mod server_v3;
