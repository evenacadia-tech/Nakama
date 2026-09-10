//! SONDE-010 — v3-Listener und I/O-Worker.
//!
//! Der Worker entscheidet GENAU drei Dinge (Entwurf §53.9): Envelope,
//! Grenzen und Authentisierung. Alles Weitere geht als typisiertes Ereignis
//! an eine schmale `Senke`. Session, Eviction, Store und Outbox liegen im
//! `Coordinator`; der Transport bleibt auch im produktiven SONDE-011-Pfad
//! auf diese schmale Schnittstelle begrenzt.
//!
//! ── Drei Threads je Verbindung, und warum ──────────────────────────────────
//!
//! §53.9 verlangt woertlich: "Antworten gehen ueber getrennte bounded
//! Writerqueues zurueck; ein blockierender Pipe-Write haelt weder Coordinator
//! noch Storelock." Die erste Fassung dieses Listeners leerte den Ingress nach
//! JEDEM Frame vollstaendig und schrieb die Antwort im Leserthread. Damit
//! konnte die Ingressqueue nie ueber Groesse 1 wachsen, und eine langsame
//! Senke oder ein Peer, der Antworten nicht abholt, hielt den Leser am ersten
//! Frame fest — die Cap-256-, P2-Drop- und P0-Ueberlaufpfade waren im echten
//! Listener unerreichbar (T2-Befund 3 vom 2026-08-29).
//!
//! Deshalb hat jede Verbindung jetzt drei Threads mit genau einer Aufgabe:
//!
//! ```text
//!   Leser      Bytes -> Envelope -> Ingress (bounded 256)   nie Senke, nie write
//!   Verbraucher Ingress -> Senke -> Antwort in die Writerqueue (bounded 256)
//!   Schreiber  Writerqueue -> ein einziger write_all auf der Pipe
//! ```
//!
//! Der Leser blockiert dadurch nie hinter der Senke, und ein blockierender
//! Write blockiert nur den Schreiber. Damit Lesen und Schreiben auf DEMSELBEN
//! Pipe-Handle wirklich nebenlaeufig sind, laufen die Instanzen als
//! `FILE_FLAG_OVERLAPPED`: bei einem synchronen Handle serialisiert der
//! I/O-Manager alle Operationen, ein haengender Read wuerde also einen Write
//! blockieren — genau das, was die Trennung verhindern soll.
//!
//! ── Produktions- und Probe-Namen ───────────────────────────────────────────
//!
//! Der Broker oeffnet in Produktion den SID-gebundenen v3-Endpunkt neben der
//! v2-Legacy-Pipe und verdrahtet ihn mit Coordinator, Store und Outbox. Tests
//! rufen denselben Listener ausschliesslich mit einem Probe-Namen auf — nie
//! mit der Produktions-Pipe (CLAUDE.md, "Bauen und beweisen").
//!
//! Sicherheit ist trotzdem nicht Probe-Qualitaet: derselbe SDDL-Deskriptor
//! ("nur der aktuelle User"), `PIPE_REJECT_REMOTE_CLIENTS` und
//! `FILE_FLAG_FIRST_PIPE_INSTANCE` wie beim v2-Server — die Helfer kommen aus
//! `server.rs`, damit es nur EINE Wahrheit ueber die Pipe-Sicherheit gibt.

use std::collections::{HashMap, VecDeque};
use std::sync::atomic::{AtomicBool, AtomicU64, AtomicUsize, Ordering};
use std::sync::{Arc, Condvar, Mutex};
use std::thread::JoinHandle;
use std::time::{Duration, Instant};

use windows_sys::Win32::Foundation::{
    CloseHandle, GetLastError, ERROR_BROKEN_PIPE, ERROR_INSUFFICIENT_BUFFER, ERROR_IO_PENDING,
    ERROR_NOT_FOUND, ERROR_NO_DATA, ERROR_NO_TOKEN, ERROR_OPERATION_ABORTED, ERROR_PIPE_BUSY,
    ERROR_PIPE_CONNECTED, ERROR_PIPE_NOT_CONNECTED, HANDLE, INVALID_HANDLE_VALUE, WAIT_OBJECT_0,
};
use windows_sys::Win32::Security::{
    CopySid, EqualSid, GetLengthSid, GetTokenInformation, IsValidSid, RevertToSelf, TokenUser,
    SECURITY_ATTRIBUTES, TOKEN_QUERY, TOKEN_USER,
};
use windows_sys::Win32::Storage::FileSystem::{
    FlushFileBuffers, ReadFile, WriteFile, FILE_FLAG_FIRST_PIPE_INSTANCE, FILE_FLAG_OVERLAPPED,
    PIPE_ACCESS_DUPLEX,
};
use windows_sys::Win32::System::Pipes::{
    ConnectNamedPipe, CreateNamedPipeW, DisconnectNamedPipe, ImpersonateNamedPipeClient,
    PIPE_READMODE_BYTE, PIPE_REJECT_REMOTE_CLIENTS, PIPE_TYPE_BYTE, PIPE_WAIT,
};
use windows_sys::Win32::System::Threading::{
    CreateEventW, GetCurrentThread, OpenThreadToken, ResetEvent, SetEvent, WaitForMultipleObjects,
    WaitForSingleObject, INFINITE,
};
use windows_sys::Win32::System::IO::{CancelIoEx, GetOverlappedResult, OVERLAPPED};
use windows_sys::Win32::Foundation::{DuplicateHandle, DUPLICATE_SAME_ACCESS};
use windows_sys::Win32::System::Threading::GetCurrentProcess;

use crate::transport::bootstrap::{
    bootstrap_lesen, neue_kennung, Bootstrap, HelloControl, Kopplungen, Welcome,
};
use crate::transport::v3::{
    envelope_schreiben, Familie, LeseErgebnis, Ratengrenze, StromLeser, MAX_FRAME_BYTES,
};

#[cfg(test)]
use crate::transport::warteschlange::CAP_INGRESS;
use crate::transport::warteschlange::{IngressErgebnis, IngressWarteschlange};

mod auth;
mod griff;
mod listener;
mod queues;
mod senke;
mod trennung;
mod verbindung;
mod win_handles;

// Die Testflaeche des Listeners, je Fachbereich eine Datei. Sie liegen als
// Geschwister DIREKT im Modulverzeichnis, weil drei Faelle mit include_str!
// Nachbardateien und Schemas ueber relative Pfade lesen; ein Unterordner
// verschoebe jeden dieser Pfade (NAK-224, Manifest Paragraph 2.5).
#[cfg(test)]
mod tests_abonnement;
#[cfg(test)]
mod tests_fristen;
#[cfg(test)]
mod tests_hilfe;
#[cfg(test)]
mod tests_kopplung;
#[cfg(test)]
mod tests_lebenszyklus;
#[cfg(test)]
mod tests_rueckstau;

use auth::{client_sid_authentisieren, VerbindungsSicherheitsCleanup};
use griff::{fertige_ernten, join_mit_frist, worker_reservieren, TestHilfsthread};
use queues::{Ausgang, Eingang};
use listener::{
    CANCEL_VOR_READ_BEREIT, CANCEL_VOR_READ_FEHLER,
    CANCEL_VOR_READ_FORTGESETZT, CANCEL_VOR_READ_GELESEN, CANCEL_VOR_READ_INAKTIV,
    CANCEL_VOR_READ_READER, CANCEL_VOR_READ_WRITER,
};
use verbindung::{verbindung_bedienen, Senkenruf};
use trennung::{
    auf_telemetrie_getrennt_warten, kopplung_loesen, melden_getrennt,
    telemetrie_getrennt_gemeldet, trennmelder_anlegen, trennmelder_telemetrie_abgesagt,
    trennmelder_telemetrie_erwartet, TrennRegister,
};
use win_handles::{
    abbrechen_und_zaehlen_extern, alle_io_abbrechen, io_abbrechen, ov_lesen, ov_schreiben,
    EndeSignal, Ereignis, HandleRegister, IoAusgang, ListenerInstanz, SicherheitsSpur, TokenGriff,
    Verbindungsgriff,
};

pub use senke::{ControlAnmeldung, Senke, ZaehlSenke};
pub use griff::{V3Closer, V3Griff, V3Sender, V3Statistik, WorkerPlatzProbe};
pub use listener::{
    v3_server_starten, v3_server_starten_fuer_security_vectors, v3_server_starten_mit_epoch,
    v3_server_starten_mit_epoch_und_sender, V3AuthTestFehler, V3SecurityTestOptionen,
    V3StartTestFehler, V3TotesHandleNaht, V3UebergabeBarriere,
};

/// Hoechstens so viele gleichzeitige Verbindungen. Zwei je Instanz (Control +
/// Telemetry) mal 32 Sonden plus Reserve.
pub const MAX_VERBINDUNGEN: usize = 96;

/// 96 Worker plus zwei jederzeit bewaffnete Besitzlistener. Windows erlaubt
/// hier 1..=255; alle Instanzen desselben Namens muessen denselben Wert nennen.
pub const PIPE_INSTANZEN: usize = MAX_VERBINDUNGEN + 2;

/// Frist fuer das Bootstrap-Hello. Ohne sie haelt ein lokaler Slowloris einen
/// Verbindungsslot beliebig lange (Fehlerlexikon, wissen/engineering
/// 2026-08-27).
pub const BOOTSTRAP_FRIST: Duration = Duration::from_millis(5000);

/// Nachrichtenratengrenze je Verbindung: 4000 Frames pro Sekunde. 32 Sonden
/// bei 10 Hz sind 320 — die Grenze faengt eine Flut, nicht den Betrieb.
pub const RATE_PRO_SEKUNDE: u32 = 4000;

/// Getrennte, begrenzte Writerqueue je Verbindung (§53.9). Laeuft sie ueber,
/// holt der Peer seine Antworten nicht ab — dann faellt die Verbindung, nicht
/// die Antwort still unter den Tisch.
pub const CAP_WRITER: usize = 256;

/// Aktive Vertragsfassungen JE FAMILIE. Die Version lebt ausschliesslich im
/// Wire-Envelope: Descriptor-Hostfelder und LUFS-I-Framefelder wurden in
/// SONDE-012 B1 mit Minor 1 belegt.
///
/// SONDE-013 (04.09.2026) hebt **P0 und P1** auf 2: die drei
/// Experimentfamilien, die belegten Felder `evidence_snapshot.ereignisse`
/// und `.stereo` und die zwei neuen Invalidierungsgruende reisen dort.
///
/// 🔑 **P2 bleibt bei 1**, und das ist kein Versehen. `integration_samples`
/// (Feld-ID 14) liegt auf der FlatBuffers-Flaeche, deren eigene
/// `schema_major`/`schema_minor` im 16-Byte-Envelope leben
/// (`nakama_telemetry_v1.fbs`). Ein neues OPTIONALES FlatBuffers-Feld ist
/// genau der additive Fall, den das Format traegt: ein alter Leser
/// uebergeht es. Waere P2 hier mitgehoben worden, haette der Broker jede
/// heutige Sonde abgewiesen, ohne dass sich an P2 etwas geaendert haette.
///
/// ⚠️ WARUM DIESE DREI ZAHLEN UEBERHAUPT EINZELN STEHEN — gemessen am
/// 04.09.2026: mit `kJsonSchemaMinor = 2` auf der C++-Seite und einer 1
/// hier wies der Server JEDEN P0-Rahmen der Sonde ab, bevor er die Senke
/// erreichte. Der Kanon blieb an `subscription_server_integration` haengen,
/// vier `eqcop-store-crash-worker` warteten 17 Minuten ohne CPU-Last auf ein
/// `command_ack`, das nie kam. Ein Fassungsschritt ist erst vollstaendig,
/// wenn BEIDE Seiten und der Transport dazwischen ihn kennen.
/// NAK-213 (08.09.2026) hebt **P0 und P1** auf 4: die zwei neuen
/// Ausschlussgruende `screening_ueberboten` und `master_duplikat` und die von
/// 32 auf `SESSION_CLIENT_CAP` gehobene Laenge der Ausschlussliste reisen dort.
const P0_SCHEMA_MINOR: u8 = 5;
const P1_SCHEMA_MINOR: u8 = 5;
const P2_SCHEMA_MINOR: u8 = 1;

fn schema_minor_bekannt(familie: Familie, schema_minor: u8) -> bool {
    let hoechster = match familie {
        Familie::P0 => P0_SCHEMA_MINOR,
        Familie::P1 => P1_SCHEMA_MINOR,
        Familie::P2 => P2_SCHEMA_MINOR,
    };
    schema_minor <= hoechster
}

/// Was der I/O-Worker nach oben gibt. Bewusst byteorientiert: die Bedeutung
/// des Payloads kennt erst der Coordinator.
/// Frist, die der Verbindungsschluss einem LAUFENDEN Senkenaufruf noch
/// laesst. Danach wird der Verbraucherthread abgeloest statt gejoint.
pub const SENKE_FRIST: Duration = Duration::from_millis(2000);

/// Takt des Wachhunds. Bis NAK-121 stand hier ein nacktes
/// `Duration::from_millis(100)` in der Schleife; H-02 misst gegen diese Frist,
/// also bekommt sie einen Namen. Modulintern, keine oeffentliche Signatur.
pub(super) const WACHHUND_TAKT: Duration = Duration::from_millis(100);

/// H-07: harte Frist des beschraenkten Abflusses vor dem Schliessen. Derselbe
/// Wert wie `ANTWORT_FLUSH_TIMEOUT` auf der v2-Seite, weil es dieselbe Frage
/// ist: wie lange darf ein nicht lesender Peer den Abbau aufhalten.
pub(super) const FLUSH_FRIST: Duration = Duration::from_millis(250);
