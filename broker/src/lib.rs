//! EQ-Copilot-Broker — M2 (Herkunft: Recherche §8.4, §9.2, §11, heute Archiv unter docs/archiv/; Produktwahrheit: CLAUDE.md).
//!
//! Eigener Scope neben den Katalog-/Bausteinmodellen (Plan §10.2): Named-Pipe-
//! Server für die Sammler-VST3-Instanzen, Sensor-Register mit Last-Seen,
//! Konflikt- und Stale-Sichtbarkeit, PRE/POST-Paar-Auswertung, Profilbindung
//! und der zeitausgerichtete Aggregat-Snapshot. Kein Audio, keine PCM-
//! Übertragung — Messwerte kommen als kompakter Messstand im v2-Heartbeat.
//! Die Verträge liegen in eq-copilot/schemas/.
//!
//! ── DIE FACHBEREICHE DER WURZEL ───────────────────────────────────────────
//!
//! Diese Datei rechnet selbst nichts. Sie meldet die Fachbereiche an und
//! exportiert sie unter DEMSELBEN Pfad, unter dem sie vor der Aufteilung
//! erreichbar waren (`eqcop_broker::<Name>`) — deshalb der Glob je Modul und
//! keine Liste, die beim nächsten `pub fn` still veraltet:
//!
//! | Modul | Fachbereich |
//! |---|---|
//! | `zeit` | Wanduhr für Ausgaben, Monotonie für jede Frist |
//! | `register` | wer meldet sich, wem gehört eine Sensor-ID, was ist frisch |
//! | `sitzung` | welche Sensoren gehören zu demselben Hostprozess |
//! | `paar` | PRE/POST-Paare und die zwei Sperrgründe |
//! | `lebenslauf` | starten, laufen, geordnet enden; Status auch im Fehlerfall |
//! | `probe` | die drei Aufrufe, die von außen hineinreichen |
//!
//! Sie sind bewusst **privat plus Glob-Reexport** und nicht `pub mod`: ein
//! öffentliches Untermodul wäre ein zweiter Pfad auf dieselbe Sache.

pub mod aggregat;
pub mod bindung;
pub mod coordinator;
pub mod dto;
pub mod framing;
pub mod generiert;
pub mod instance_alias;
pub mod protokoll;
#[cfg(windows)]
mod server;
pub mod store;
pub mod telemetrie;
pub mod transport;
pub mod vertrag;

mod lebenslauf;
mod paar;
mod probe;
mod register;
mod sitzung;
mod zeit;
#[cfg(test)]
mod testhilfe;

pub use lebenslauf::*;
pub use paar::*;
pub use probe::*;
pub use register::*;
pub use sitzung::*;
pub use zeit::*;
