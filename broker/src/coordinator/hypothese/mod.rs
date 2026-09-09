//! Kandidatenbildung, Rang und `CauseHypothesis` (SONDE-014 Etappe C).
//!
//! ── WER HIER RECHNET, UND WARUM AUSGERECHNET HIER ─────────────────────────
//!
//! Entscheid **E-08** weist Hypothese, Proposal und Maskierung ausdruecklich
//! dem Broker-Coordinator zu: „dort liegen Evidenzbestand und Store". Der
//! Intent dagegen gehoert dem Main — dieses Modul LIEST ihn nur (ueber
//! `intent::IntentBestand`) und schreibt ihn nie zurueck.
//!
//! Dieses Modul ist eine REINE Rechnung. Es kennt weder Lock noch Store noch
//! Senke; alles, was es braucht, steht in `Aufnahme`. Der Produktpfad — wer
//! die Aufnahme fuellt, wann gerechnet wird und wohin das Ergebnis reist —
//! liegt in `hypothese_verdrahtung.rs`. Das ist Risiko **R1** des Bauplans:
//! ein Test, der nur dieses Modul ruft, kann nicht fallen, wenn die
//! Verdrahtung fehlt.
//!
//! ── DIE FACHBEREICHE UND IHRE REGELN ──────────────────────────────────────
//!
//! Jedes Untermodul traegt die Rechnung EINES Fachbereichs und in seinem Kopf
//! die Regel, die genau diese Rechnung bindet. Wer eine Regel sucht, findet
//! sie dort, wo sie wirkt, und nicht hier in zweiter Fassung:
//!
//! | Modul | Fachbereich | Regel im Kopf |
//! |---|---|---|
//! | `vertrag` | die geschlossenen Mengen des Produzenten | M-14, M-77, M-87 |
//! | `determinismus` | die Groessen der `metrics_version` | M-25, §36.4, E-12 |
//! | `eingang` | was der Rechner ueber eine Quelle weiss | M-07, M-86 |
//! | `messung` | reines Rechenwerkzeug, einzeln messbar | M-07, M-19 |
//! | `screening` | Stufe A, Deckel und die harten Gates | M-18, M-20, M-24 |
//! | `zusammenhang` | Stufe B, Rangkomponenten und Beleg | M-19, M-25 |
//! | `klassenwahl` | Konkurrenzlage, Sicherheit → Zustand | M-15, M-29, M-30 |
//! | `befund` | Befundbau und Sicht | M-16, M-17, M-25 |
//! | `enthaltung` | die Enthaltung mit Ort und ohne | M-27, R4, R8 |
//! | `rechnung` | der Ablauf `hypothesen` und `masteranomalie` | M-18, M-86 |
//!
//! Dieses `mod.rs` selbst rechnet nichts. Es meldet die Fachbereiche an und
//! exportiert sie unter DEMSELBEN Pfad, unter dem sie vor der Aufteilung
//! erreichbar waren (`coordinator::hypothese::…`) — deshalb der Glob je
//! Modul und keine Liste, die beim naechsten `pub fn` still veraltet.
//!
//! ── WAS DIE MODULTESTS MESSEN UND WAS NICHT (§5.3 R1) ─────────────────────
//!
//! Die `#[cfg(test)]`-Faelle der Untermodule rufen ihre Rechnung DIREKT und
//! koennen deshalb nicht fallen, wenn die Verdrahtung fehlt — das ist
//! ausdruecklich ihre Grenze. Sie messen die Zahlenraender, an die ein
//! Verdrahtungsfall nicht herankommt: Bandindex 0 und 220, leere Reihen, NaN,
//! das Fenster GENAU an der Schwelle. Der Rotbeweis faellt am
//! Verdrahtungsbein `broker/tests/sonde014_hypothese.rs`.

mod befund;
mod determinismus;
mod eingang;
mod enthaltung;
mod klassenwahl;
mod messung;
mod rechnung;
mod screening;
mod vertrag;
mod zusammenhang;
#[cfg(test)]
mod testhilfe;

pub use befund::*;
pub use determinismus::*;
pub use eingang::*;
pub use enthaltung::*;
pub use klassenwahl::*;
pub use messung::*;
pub use rechnung::*;
pub use screening::*;
pub use vertrag::*;
pub use zusammenhang::*;
