//! NAK-286 Etappe 3 - der Diagnose-Briefkasten des Brokers (Plan S25e (3), R-286-1,
//! R-286-2; Spezifikation docs/beweise/NAK-286.md §2, §4.4, §5.6).
//!
//! Liegt `%LOCALAPPDATA%\evenacadia\nakama\diagnose\anfrage.json`, schreibt der Broker je
//! Anfragekennung genau eine Antwort in den festen Ordner `antwort\`: den Umschlag
//! `nakama.diagnose.antwort.v1` mit dem Aggregat aus `aggregat_bauen` ohne Filter oder, bei
//! einer Sperre aus `aggregat_sperrgrund`, null mit Grund (F-7). Ohne Datei geschieht nichts
//! Messbares: je Takt genau eine Existenzpruefung (F-14).
//!
//! Dieselben Regeln wie im Plugin (eq-copilot/plugin/core/diagnose/Briefkasten.cpp, T-26):
//! der Inhalt der Anfrage wird nie Pfad oder Parameter; ein Reparse-Punkt heisst nichts
//! lesen und nichts schreiben; hoechstens 1 024 Bytes; Kennungsring FIFO 256; hoechstens
//! drei Schreibversuche; jeder Dateizugriff laeuft ueber die zaehlende Fassade. Geschrieben
//! wird in sechs Schritten ohne Ordneranlage, ohne Ersetzen und ohne Aufraeumer;
//! `persistenz::atomar_neu` traegt diese Zusagen nicht und wird nicht benutzt.
//!
//! Sperren (F-15): das Register-Mutex nur fuer die Kopie der Sensoren und des Kuerzels wie
//! in `aggregat_schreiben`; Bau, Serialisierung und Datei-I/O laufen ohne diese Sperre.
//!
//! Lebensdauer (F-12, M-45): Thread `nakama-briefkasten` mit Stoppflag und Weckbedingung,
//! Wartefrist aus der Uhr-Fassade (im Produkt 1 s), Stoppflag vor jedem Takt. Der Stoppweg
//! setzt das Flag unter der Sperre, weckt und wartet den Thread mit 2 s Frist zu Ende; der
//! Takt wartet nie auf ihn. Startfehler (F-1, P-2, M-76): ohne LOCALAPPDATA oder mit einem
//! Reparse-Punkt auf einer vorhandenen Ebene kein Thread; der Grund bleibt im Griff, das
//! Register erfaehrt nichts.

use crate::aggregat::aggregat_bauen;
use crate::lebenslauf::broker_version;
use crate::paar::aggregat_sperrgrund;
use crate::register::{Register, SensorEintrag};
use crate::server::{server_starten, ServerGriff};
use crate::transport::pipetoken::ist_probe_pipename;
use crate::zeit::jetzt_ms;
use serde::Deserialize;
use serde_json::{json, Value};
use sha2::{Digest, Sha256};
use std::ffi::OsString;
use std::fs::{File, OpenOptions};
use std::io::{ErrorKind, Read, Write};
use std::os::windows::ffi::OsStrExt;
use std::os::windows::fs::OpenOptionsExt;
use std::panic::{catch_unwind, AssertUnwindSafe};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, Condvar, Mutex, MutexGuard, PoisonError};
use std::thread::JoinHandle;
use std::time::{Duration, Instant};
use windows_sys::Win32::Storage::FileSystem::{
    GetFileAttributesExW, GetFileExInfoStandard, MoveFileExW, FILE_ATTRIBUTE_DIRECTORY,
    FILE_ATTRIBUTE_REPARSE_POINT, FILE_FLAG_OPEN_REPARSE_POINT, FILE_SHARE_DELETE,
    FILE_SHARE_READ, FILE_SHARE_WRITE, MOVEFILE_WRITE_THROUGH, WIN32_FILE_ATTRIBUTE_DATA,
};

/// F-2: eine groessere Anfrage ist fremd; gelesen werden hoechstens 1 025 Bytes.
const ANFRAGE_GRENZE_BYTES: usize = 1024;
/// F-13: die zuletzt beantworteten Kennungen, FIFO fester Groesse.
const KENNUNGSRING: usize = 256;
/// F-13: danach ist eine Kennung aufgegeben.
const SCHREIBVERSUCHE: u32 = 3;
/// F-12: Frist eines Wartedurchlaufs im Produkt - hoechstens ein Takt je Sekunde.
const TAKTFRIST: Duration = Duration::from_secs(1);
/// F-12: so lange wartet der Stoppweg den Thread zu Ende (Zusage ohne Zeit-Rotbeweis).
const JOIN_FRIST: Duration = Duration::from_secs(2);
const ANFRAGE_FORMAT: &str = "nakama.diagnose.anfrage.v1";
const ANTWORT_FORMAT: &str = "nakama.diagnose.antwort.v1";
const ROLLE: &str = "broker";
/// F-1: die Ebenen unter %LOCALAPPDATA% von oben; die letzte ist der Antwortordner.
const EBENEN: [&str; 4] = ["evenacadia", "nakama", "diagnose", "antwort"];
const THREAD_NAME: &str = "nakama-briefkasten";

static STARTS: AtomicU64 = AtomicU64::new(0);

/// Wie viele Threads `nakama-briefkasten` dieser Prozess gestartet hat (M-46).
pub fn briefkasten_starts() -> u64 {
    STARTS.load(Ordering::SeqCst)
}

fn sperre<T>(mutex: &Mutex<T>) -> MutexGuard<'_, T> {
    mutex.lock().unwrap_or_else(PoisonError::into_inner)
}

fn zaehle(zaehler: &AtomicU64) {
    zaehler.fetch_add(1, Ordering::SeqCst);
}

// ── Kennung, Anfrage, Zeitstempel (F-2, F-3, F-4) ─────────────────────────────────────

/// Eine Anfragekennung: genau 32 Zeichen aus 0-9a-f (F-2).
type Kennung = [u8; 32];

fn kennung_gueltig(zeichen: &[u8]) -> bool {
    zeichen.len() == 32 && zeichen.iter().all(|z| matches!(z, b'0'..=b'9' | b'a'..=b'f'))
}

fn kennung_text(kennung: &Kennung) -> String {
    String::from_utf8_lossy(kennung).into_owned()
}

#[derive(Deserialize)]
#[serde(deny_unknown_fields)]
struct AnfrageDokument {
    format: String,
    anfrage_id: String,
}

/// F-2, P-15: UTF-8 ohne BOM, ein JSON-Objekt mit genau den Schluesseln `format` und
/// `anfrage_id`, je einmal. Alles andere ist fremd geformt.
fn anfrage_lesen(daten: &[u8]) -> Option<Kennung> {
    if daten.starts_with(&[0xEF, 0xBB, 0xBF]) {
        return None;
    }
    // P-15: die Wurzel ist ein Objekt, bevor ein Feld gelesen wird - das erste Byte nach
    // JSON-Leerraum ist `{` wie in Briefkasten.cpp:185-187. Der abgeleitete Leser traegt das
    // nicht: serde_json nimmt fuer ein Struct auch ein Array und belegt die Felder positionell,
    // deny_unknown_fields wirkt nur im Objektzweig (T-37).
    let erstes = daten.iter().copied().find(|byte| !matches!(byte, b' ' | b'\t' | b'\n' | b'\r'));
    if erstes != Some(b'{') {
        return None;
    }
    let dokument: AnfrageDokument = serde_json::from_slice(daten).ok()?;
    if dokument.format != ANFRAGE_FORMAT || !kennung_gueltig(dokument.anfrage_id.as_bytes()) {
        return None;
    }
    dokument.anfrage_id.as_bytes().try_into().ok()
}

/// F-3: die ersten 16 Hex-Zeichen von SHA-256 ueber die UTF-8-Bytes der Instanzkennung.
fn instanz16(instanz_id: &str) -> String {
    let digest = Sha256::digest(instanz_id.as_bytes());
    digest.iter().take(8).map(|byte| format!("{byte:02x}")).collect()
}

/// ISO-8601 in UTC mit Millisekunden und `Z`; vor 1970 und nach dem Jahr 3000 dieselbe
/// Rueckfallzeit wie im Plugin, das dort `_gmtime64_s` rechnen laesst.
fn iso_utc(millisekunden: i64) -> String {
    const RUECKFALL: &str = "1970-01-01T00:00:00.000Z";
    const LETZTE_SEKUNDE: i64 = 32_535_215_999; // 3000-12-31T23:59:59Z
    let (sekunden, milli) = (millisekunden.div_euclid(1000), millisekunden.rem_euclid(1000));
    if !(0..=LETZTE_SEKUNDE).contains(&sekunden) {
        return RUECKFALL.to_string();
    }
    // Tage seit 1970-01-01 im proleptischen gregorianischen Kalender (Hinnant).
    let z = sekunden.div_euclid(86_400) + 719_468;
    let (aera, tag_der_aera) = (z.div_euclid(146_097), z.rem_euclid(146_097));
    let jahr_der_aera =
        (tag_der_aera - tag_der_aera / 1460 + tag_der_aera / 36_524 - tag_der_aera / 146_096) / 365;
    let tag_im_jahr = tag_der_aera - (365 * jahr_der_aera + jahr_der_aera / 4 - jahr_der_aera / 100);
    let m = (5 * tag_im_jahr + 2) / 153;
    let tag = tag_im_jahr - (153 * m + 2) / 5 + 1;
    let monat = if m < 10 { m + 3 } else { m - 9 };
    let jahr = jahr_der_aera + aera * 400 + i64::from(monat <= 2);
    let rest = sekunden.rem_euclid(86_400);
    let (stunde, minute, sekunde) = (rest / 3600, rest % 3600 / 60, rest % 60);
    format!("{jahr:04}-{monat:02}-{tag:02}T{stunde:02}:{minute:02}:{sekunde:02}.{milli:03}Z")
}

// ── Fassaden (F-14) ───────────────────────────────────────────────────────────────────

/// Attribute einer Datei oder eines Ordners, gelesen ohne Oeffnen.
#[derive(Debug, Clone, Copy, Default)]
struct DateiAttribute {
    existiert: bool,
    verzeichnis: bool,
    reparse: bool,
    groesse: u64,
    aenderungszeit: u64,
}

/// F-11: ein Symlink oder eine Junction.
fn ist_verlinkt(attribute: &DateiAttribute) -> bool {
    attribute.reparse
}

/// Ein Pfad samt NUL-terminierter UTF-16-Form, gebaut beim Start und je Schreibversuch: die
/// Existenzpruefung eines Takts legt keinen Speicher an (M-51). Mit NUL ist er keiner.
struct Pfad {
    pfad: PathBuf,
    breit: Vec<u16>,
    gueltig: bool,
}

impl Pfad {
    fn neu(pfad: PathBuf) -> Self {
        let mut breit: Vec<u16> = pfad.as_os_str().encode_wide().collect();
        let gueltig = !breit.contains(&0);
        breit.push(0);
        Self { pfad, breit, gueltig }
    }
}

/// Die Schritte eines Dateisystems. Nur `Dateisystem` ruft sie und zaehlt dabei jeden; ein
/// Zugriff an ihr vorbei saehe kein Zaehler (F-14, §10.3 (d)). Im Test eine Attrappe.
trait DateisystemSchritte: Send + Sync {
    fn attribute(&self, pfad: &Pfad) -> DateiAttribute;
    fn lies(&self, pfad: &Pfad, ziel: &mut [u8]) -> Option<usize>;
    fn lege_exklusiv_an(&self, pfad: &Pfad) -> Option<File>;
    fn schreibe_und_spuele(&self, datei: &mut File, daten: &[u8]) -> bool;
    fn groesse(&self, pfad: &Pfad) -> Option<u64>;
    fn benenne_um_ohne_ersetzen(&self, von: &Pfad, nach: &Pfad) -> bool;
    /// Nur im Testbau: im Produkt legt der Briefkasten nie einen Ordner an (T-4).
    #[cfg(test)]
    fn lege_ordner_an(&self, pfad: &Pfad) -> bool;
    /// Nur im Testbau: im Produkt loescht der Briefkasten nie (F-9).
    #[cfg(test)]
    fn loesche(&self, pfad: &Pfad) -> bool;
}

/// Die echten Win32-Aufrufe (F-8: exklusiv anlegen, Umbenennen ohne Ersetzen; F-11: gelesen
/// wird ohne einem Link zu folgen).
struct EchtesDateisystem;

impl DateisystemSchritte for EchtesDateisystem {
    fn attribute(&self, pfad: &Pfad) -> DateiAttribute {
        if !pfad.gueltig {
            return DateiAttribute::default();
        }
        let mut daten = WIN32_FILE_ATTRIBUTE_DATA::default();
        let puffer = std::ptr::addr_of_mut!(daten).cast();
        // SAFETY: `breit` ist NUL-terminiert und lebt bis nach dem synchronen Aufruf; `puffer`
        // zeigt auf einen beschreibbaren Puffer des Typs, den GetFileExInfoStandard verlangt.
        let ok = unsafe { GetFileAttributesExW(pfad.breit.as_ptr(), GetFileExInfoStandard, puffer) };
        // Oeffnet nichts und folgt einem Link in der letzten Komponente nicht: ein Symlink
        // oder eine Junction meldet sich selbst mit REPARSE_POINT.
        if ok == 0 {
            return DateiAttribute::default();
        }
        let hoch_tief = |hoch: u32, tief: u32| (u64::from(hoch) << 32) | u64::from(tief);
        DateiAttribute {
            existiert: true,
            verzeichnis: daten.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY != 0,
            reparse: daten.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT != 0,
            groesse: hoch_tief(daten.nFileSizeHigh, daten.nFileSizeLow),
            aenderungszeit: hoch_tief(daten.ftLastWriteTime.dwHighDateTime, daten.ftLastWriteTime.dwLowDateTime),
        }
    }

    fn lies(&self, pfad: &Pfad, ziel: &mut [u8]) -> Option<usize> {
        // FILE_FLAG_OPEN_REPARSE_POINT: die Datei selbst, nie das Ziel eines Links (T-6).
        // Geteilt lesbar, schreib- und loeschbar: der Anfragende darf sie derweil ersetzen.
        if !pfad.gueltig {
            return None;
        }
        let teilen = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
        let mut datei =
            OpenOptions::new().read(true).share_mode(teilen).custom_flags(FILE_FLAG_OPEN_REPARSE_POINT).open(&pfad.pfad).ok()?;
        let mut gelesen = 0;
        while gelesen < ziel.len() {
            match datei.read(&mut ziel[gelesen..]) {
                Ok(0) => break,
                Ok(stueck) => gelesen += stueck,
                Err(fehler) if fehler.kind() == ErrorKind::Interrupted => {}
                Err(_) => return None,
            }
        }
        Some(gelesen)
    }

    fn lege_exklusiv_an(&self, pfad: &Pfad) -> Option<File> {
        // CREATE_NEW ohne Teilen: ein vorhandener Name laesst das Anlegen scheitern.
        if !pfad.gueltig {
            return None;
        }
        OpenOptions::new().write(true).create_new(true).share_mode(0).open(&pfad.pfad).ok()
    }

    fn schreibe_und_spuele(&self, datei: &mut File, daten: &[u8]) -> bool {
        datei.write_all(daten).is_ok() && datei.sync_all().is_ok()
    }

    fn groesse(&self, pfad: &Pfad) -> Option<u64> {
        let attribute = self.attribute(pfad);
        attribute.existiert.then_some(attribute.groesse)
    }

    fn benenne_um_ohne_ersetzen(&self, von: &Pfad, nach: &Pfad) -> bool {
        if !von.gueltig || !nach.gueltig {
            return false;
        }
        // SAFETY: beide UTF-16-Puffer sind NUL-terminiert und leben bis nach dem Aufruf. Ohne
        // REPLACE_EXISTING scheitert er an einem vorhandenen Namen (persistenz.rs:78-109).
        unsafe { MoveFileExW(von.breit.as_ptr(), nach.breit.as_ptr(), MOVEFILE_WRITE_THROUGH) != 0 }
    }

    #[cfg(test)]
    fn lege_ordner_an(&self, pfad: &Pfad) -> bool {
        pfad.gueltig && std::fs::create_dir(&pfad.pfad).is_ok()
    }

    #[cfg(test)]
    fn loesche(&self, pfad: &Pfad) -> bool {
        pfad.gueltig && std::fs::remove_file(&pfad.pfad).is_ok()
    }
}

#[derive(Default)]
struct DateisystemZaehler {
    existenz: AtomicU64,
    lese: AtomicU64,
    bytes: AtomicU64,
    anlegen: AtomicU64,
    schreiben: AtomicU64,
    groesse: AtomicU64,
    umbenennen: AtomicU64,
    #[cfg(test)]
    ordner: AtomicU64,
    #[cfg(test)]
    loeschen: AtomicU64,
}

/// Die zaehlende Dateisystem-Fassade: jeder Schritt zaehlt, dann ruft er.
struct Dateisystem {
    schritte: Arc<dyn DateisystemSchritte>,
    z: DateisystemZaehler,
}

impl Dateisystem {
    fn neu(schritte: Arc<dyn DateisystemSchritte>) -> Self {
        Self { schritte, z: DateisystemZaehler::default() }
    }

    fn attribute(&self, pfad: &Pfad) -> DateiAttribute {
        zaehle(&self.z.existenz);
        self.schritte.attribute(pfad)
    }

    fn lies(&self, pfad: &Pfad, ziel: &mut [u8]) -> Option<usize> {
        zaehle(&self.z.lese);
        let gelesen = self.schritte.lies(pfad, ziel);
        self.z.bytes.fetch_add(gelesen.unwrap_or(0) as u64, Ordering::SeqCst);
        gelesen
    }

    fn lege_exklusiv_an(&self, pfad: &Pfad) -> Option<File> {
        zaehle(&self.z.anlegen);
        self.schritte.lege_exklusiv_an(pfad)
    }

    fn schreibe_und_spuele(&self, datei: &mut File, daten: &[u8]) -> bool {
        zaehle(&self.z.schreiben);
        self.schritte.schreibe_und_spuele(datei, daten)
    }

    fn groesse(&self, pfad: &Pfad) -> Option<u64> {
        zaehle(&self.z.groesse);
        self.schritte.groesse(pfad)
    }

    fn benenne_um_ohne_ersetzen(&self, von: &Pfad, nach: &Pfad) -> bool {
        zaehle(&self.z.umbenennen);
        self.schritte.benenne_um_ohne_ersetzen(von, nach)
    }

    #[cfg(test)]
    fn lege_ordner_an(&self, pfad: &Pfad) -> bool {
        zaehle(&self.z.ordner);
        self.schritte.lege_ordner_an(pfad)
    }

    #[cfg(test)]
    fn loesche(&self, pfad: &Pfad) -> bool {
        zaehle(&self.z.loeschen);
        self.schritte.loesche(pfad)
    }
}

/// F-1: die Wurzel %LOCALAPPDATA%. Sie liegt ausserhalb der Dateisystem-Fassade; jede
/// Abfrage zaehlt die Umgebungs-Fassade. Im Produkt die Variable wie aggregat.rs:139-141.
trait UmgebungSchritte: Send + Sync {
    fn localappdata(&self) -> Option<OsString>;
}

struct EchteUmgebung;

impl UmgebungSchritte for EchteUmgebung {
    fn localappdata(&self) -> Option<OsString> {
        std::env::var_os("LOCALAPPDATA")
    }
}

/// Fuer den Probelauf (M-46): eine uebergebene Wurzel statt der Umgebung.
struct FesteWurzel(OsString);

impl UmgebungSchritte for FesteWurzel {
    fn localappdata(&self) -> Option<OsString> {
        Some(self.0.clone())
    }
}

struct Umgebung {
    schritte: Arc<dyn UmgebungSchritte>,
    abfragen: AtomicU64,
}

impl Umgebung {
    fn neu(schritte: Arc<dyn UmgebungSchritte>) -> Self {
        Self { schritte, abfragen: AtomicU64::new(0) }
    }

    fn localappdata(&self) -> Option<OsString> {
        zaehle(&self.abfragen);
        self.schritte.localappdata()
    }
}

/// F-14: `erzeugt_utc` und die Frist eines Wartedurchlaufs. Die Zuordnung einer Antwort
/// haengt nie an der Uhr (M-52).
trait Uhr: Send + Sync {
    fn jetzt_utc_ms(&self) -> i64;
    fn taktfrist(&self) -> Duration;
}

struct EchteUhr;

impl Uhr for EchteUhr {
    fn jetzt_utc_ms(&self) -> i64 {
        i64::try_from(jetzt_ms()).unwrap_or(i64::MAX)
    }

    fn taktfrist(&self) -> Duration {
        TAKTFRIST
    }
}

#[derive(Clone)]
struct Fassaden {
    umgebung: Arc<Umgebung>,
    dateisystem: Arc<Dateisystem>,
    uhr: Arc<dyn Uhr>,
}

impl Fassaden {
    fn echt(umgebung: Arc<dyn UmgebungSchritte>) -> Self {
        Self {
            umgebung: Arc::new(Umgebung::neu(umgebung)),
            dateisystem: Arc::new(Dateisystem::neu(Arc::new(EchtesDateisystem))),
            uhr: Arc::new(EchteUhr),
        }
    }
}

// ── Der Takt (F-2, F-8 bis F-11, F-13) ────────────────────────────────────────────────

/// Warum ein Griff laeuft oder nicht - nur hier gemerkt, nie im Register (§10.2 P-2).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum Startgrund {
    Gestartet,
    /// Das Sitzungskuerzel ergibt keine Laufzeitkennung aus 32 Hex-Zeichen (F-3).
    Konfiguration,
    /// LOCALAPPDATA ist nicht gesetzt oder leer (F-1).
    KeineWurzel,
    /// Eine beim Start vorhandene Ebene ist ein Reparse-Punkt (F-11).
    ReparsePunkt,
    /// Der Thread liess sich nicht anlegen.
    OhneThread,
}

/// Instanzlokale Zaehler, alle monoton (Testzugang).
#[derive(Default)]
struct Zaehlwerk {
    starts: AtomicU64,
    takte: AtomicU64,
    schreibversuche: AtomicU64,
    antworten: AtomicU64,
    fehlerzaehler: AtomicU64,
    ausnahmen: AtomicU64,
    wartedurchlaeufe_nach_stopp: AtomicU64,
    join_frist_verfehlt: AtomicU64,
    join_dauer_us: AtomicU64,
}

/// T-5: eine unveraenderte Datei (Aenderungszeit, Groesse) wird nicht erneut gelesen.
#[derive(Clone, Copy, Default)]
struct Merker {
    gueltig: bool,
    fremd: bool,
    zeit: u64,
    groesse: u64,
    kennung: Kennung,
}

/// Die Kennung, fuer die gerade geschrieben wird, und ihre Versuche (F-13).
#[derive(Clone, Copy, Default)]
struct Offen {
    gueltig: bool,
    aufgegeben: bool,
    versuche: u32,
    kennung: Kennung,
}

struct TaktZustand {
    merker: Merker,
    offen: Offen,
    ring: [Kennung; KENNUNGSRING],
    ring_naechster: usize,
    ring_belegt: usize,
}

impl TaktZustand {
    fn ring_enthaelt(&self, kennung: &Kennung) -> bool {
        self.ring[..self.ring_belegt].contains(kennung)
    }

    /// FIFO fester Groesse: die 257. Kennung verdraengt die aelteste (F-13).
    fn ring_merke(&mut self, kennung: Kennung) {
        self.ring[self.ring_naechster] = kennung;
        self.ring_naechster = (self.ring_naechster + 1) % KENNUNGSRING;
        self.ring_belegt = (self.ring_belegt + 1).min(KENNUNGSRING);
    }
}

/// Die Weckbedingung des Threads (F-12).
#[derive(Default)]
struct Weck {
    stopp: bool,
    /// Testzugang: Takte, die ein Test ausloest; im Produkt immer 0.
    ausloesungen: u64,
    im_warten: bool,
    beendet: bool,
    takte_fertig: u64,
}

enum Lesung {
    /// Nichts gemerkt: der naechste Takt liest erneut (F-10, F-11).
    Spaeter,
    /// Fremd, zu gross oder anders geformt (F-10).
    Fremd,
    Kennung(Kennung),
}

struct Kern {
    register: Arc<Mutex<Register>>,
    fassaden: Fassaden,
    zaehlwerk: Arc<Zaehlwerk>,
    /// F-3: die persistente Instanzkennung des Brokers ist sein Sitzungskuerzel.
    instanz_id: String,
    instanz16: String,
    laufzeit32: String,
    pid: u32,
    /// evenacadia, nakama, diagnose, antwort
    ebenen: [Pfad; 4],
    anfrage: Pfad,
    zustand: Mutex<TaktZustand>,
    weck: Mutex<Weck>,
    /// Hier wartet der Thread; es weckt ihn der Stoppweg (im Test auch eine Ausloesung).
    weckruf: Condvar,
    /// Hier meldet der Thread Warten, Takt und Ende.
    meldung: Condvar,
    #[cfg(test)]
    haken_takt: Mutex<Option<Box<dyn Fn() + Send + Sync>>>,
}

impl Kern {
    /// Der Thread `nakama-briefkasten` (F-12).
    fn lauf(&self) {
        let frist = self.fassaden.uhr.taktfrist();
        let mut weck = sperre(&self.weck);
        let mut frist_ende = Instant::now() + frist;
        loop {
            // Das Stoppflag vor jedem Takt und nach jedem Wecken (M-45).
            if weck.stopp {
                break;
            }
            let jetzt = Instant::now();
            if weck.ausloesungen > 0 || jetzt >= frist_ende {
                if weck.ausloesungen > 0 {
                    weck.ausloesungen -= 1;
                } else {
                    frist_ende = jetzt + frist;
                }
                drop(weck);
                self.takt();
                weck = sperre(&self.weck);
                weck.takte_fertig = weck.takte_fertig.wrapping_add(1);
                self.meldung.notify_all();
                continue;
            }
            weck.im_warten = true;
            self.meldung.notify_all();
            let (neu, ergebnis) =
                self.weckruf.wait_timeout(weck, frist_ende - jetzt).unwrap_or_else(PoisonError::into_inner);
            weck = neu;
            weck.im_warten = false;
            if weck.stopp && ergebnis.timed_out() {
                // Das Warten endete erst mit der Frist, obwohl das Stoppsignal schon stand:
                // der Stoppweg hat nicht geweckt (M-45 (1)).
                zaehle(&self.zaehlwerk.wartedurchlaeufe_nach_stopp);
            }
        }
        weck.beendet = true;
        self.meldung.notify_all();
    }

    /// Ein Takt: genau eine Existenzpruefung, alles Weitere nur mit Anfrage.
    fn takt(&self) {
        #[cfg(test)]
        {
            if let Some(haken) = sperre(&self.haken_takt).as_ref() {
                haken();
            }
        }
        zaehle(&self.zaehlwerk.takte);
        if catch_unwind(AssertUnwindSafe(|| self.rumpf())).is_err() {
            zaehle(&self.zaehlwerk.ausnahmen);
        }
    }

    fn rumpf(&self) {
        let mut zustand = sperre(&self.zustand);
        // GENAU EINE Existenzpruefung je Takt (F-14); ein Reparse-Punkt auf anfrage.json
        // heisst nichts lesen (F-11 (2)).
        let attribute = self.fassaden.dateisystem.attribute(&self.anfrage);
        if !attribute.existiert || attribute.verzeichnis || ist_verlinkt(&attribute) {
            zustand.merker = Merker::default();
            zustand.offen = Offen::default();
            return;
        }
        let merker = zustand.merker;
        let kennung = if merker.gueltig && merker.zeit == attribute.aenderungszeit && merker.groesse == attribute.groesse {
            if merker.fremd {
                return; // unveraendert und fremd: nichts lesen (T-5)
            }
            merker.kennung
        } else {
            let (zeit, groesse) = (attribute.aenderungszeit, attribute.groesse);
            let mut neu = Merker { gueltig: true, fremd: true, zeit, groesse, kennung: [0; 32] };
            match self.anfrage_neu_lesen(&attribute) {
                Lesung::Spaeter => return,
                Lesung::Fremd => {
                    zustand.merker = neu;
                    zustand.offen = Offen::default();
                    return;
                }
                Lesung::Kennung(kennung) => {
                    (neu.fremd, neu.kennung) = (false, kennung);
                    zustand.merker = neu;
                    kennung
                }
            }
        };
        // Genau einmal, solange die Kennung im Ring steht (F-13, §10.2 P-1).
        if zustand.ring_enthaelt(&kennung) {
            zustand.offen = Offen::default();
            return;
        }
        if !zustand.offen.gueltig || zustand.offen.kennung != kennung {
            zustand.offen = Offen { gueltig: true, kennung, ..Offen::default() };
        }
        if zustand.offen.aufgegeben {
            return;
        }
        zustand.offen.versuche += 1;
        zaehle(&self.zaehlwerk.schreibversuche);
        if self.beantworte(&kennung) {
            zustand.ring_merke(kennung); // erst nach dem Umbenennen (F-9)
            zaehle(&self.zaehlwerk.antworten);
            zustand.offen = Offen::default();
        } else if zustand.offen.versuche >= SCHREIBVERSUCHE {
            zustand.offen.aufgegeben = true;
            zaehle(&self.zaehlwerk.fehlerzaehler);
        }
    }

    fn anfrage_neu_lesen(&self, attribute: &DateiAttribute) -> Lesung {
        // Groesse vor dem Oeffnen (F-2): eine zu grosse Datei wird nie gelesen.
        if attribute.groesse > ANFRAGE_GRENZE_BYTES as u64 {
            return Lesung::Fremd;
        }
        // F-11 (3): vor dem Lesen die Ebenen bis diagnose\ erneut.
        if !self.ebenen_intakt(3) {
            return Lesung::Spaeter;
        }
        let mut puffer = [0u8; ANFRAGE_GRENZE_BYTES + 1];
        match self.fassaden.dateisystem.lies(&self.anfrage, &mut puffer) {
            None => Lesung::Spaeter, // gesperrt: kein Merker, der naechste Takt liest erneut (F-10)
            Some(gelesen) if gelesen > ANFRAGE_GRENZE_BYTES => Lesung::Fremd,
            Some(gelesen) => anfrage_lesen(&puffer[..gelesen]).map_or(Lesung::Fremd, Lesung::Kennung),
        }
    }

    /// F-7, F-15: Kopie unter dem Register-Mutex, danach Bau, Serialisierung und Datei-I/O
    /// ohne ihn. `false` ist ein gescheiterter Schreibversuch.
    fn beantworte(&self, kennung: &Kennung) -> bool {
        let (sensoren, token_kurz) = {
            let register = sperre(&self.register);
            (register.sensoren_snapshot(jetzt_ms()), self.instanz_id.chars().take(8).collect::<String>())
        };
        let umschlag = self.umschlag_bauen(kennung, &sensoren, &token_kurz);
        serde_json::to_vec(&umschlag).is_ok_and(|bytes| self.schreibe(kennung, &bytes))
    }

    /// F-4: der Umschlag mit allen Pflichtschluesseln. Der Broker hat weder Snapshot noch
    /// Rahmen; sein Aggregat kommt aus `aggregat_bauen` ohne Filter oder steht bei einer
    /// Sperre als null mit Grund (F-7).
    fn umschlag_bauen(&self, kennung: &Kennung, sensoren: &[SensorEintrag], token_kurz: &str) -> Value {
        let erzeugt_ms = self.fassaden.uhr.jetzt_utc_ms();
        let mut gruende = vec![
            grund("snapshot", "rolle_ohne_snapshot", "der Broker schreibt keinen Snapshot; den schreibt das Plugin"),
            grund("frame", "rolle_ohne_frame", "der Broker baut keinen Rahmen; den Rahmenauszug schreibt das Plugin"),
        ];
        let aggregat = match aggregat_sperrgrund(sensoren) {
            Some(sperrgrund) => {
                gruende.push(grund("aggregat", "aggregat_gesperrt", &sperrgrund));
                Value::Null
            }
            None => {
                let erzeugt = u64::try_from(erzeugt_ms).unwrap_or(0);
                aggregat_bauen(sensoren, None, None, &broker_version(), token_kurz, erzeugt)
            }
        };
        json!({
            "format": ANTWORT_FORMAT,
            "anfrage_id": kennung_text(kennung),
            "rolle": ROLLE,
            "instanz_id": self.instanz_id,
            "laufzeit_id": self.laufzeit32,
            "pid": self.pid,
            "erzeugt_utc": iso_utc(erzeugt_ms),
            "version": broker_version(),
            "snapshot": Value::Null,
            "frame": Value::Null,
            "aggregat": aggregat,
            "gruende": gruende,
        })
    }

    /// F-8, sechs Schritte, jeder ueber die Fassade. `false` ist ein gescheiterter
    /// Schreibversuch; eine liegengebliebene Temp-Datei bleibt liegen (kein Aufraeumer, F-9).
    fn schreibe(&self, kennung: &Kennung, umschlag: &[u8]) -> bool {
        let dateisystem = &self.fassaden.dateisystem;
        let name = format!("{}.{ROLLE}.{}.{}.{}.json", kennung_text(kennung), self.pid, self.instanz16, self.laufzeit32);
        // (1) antwort\ wird nie angelegt; fehlt er oder ist eine Ebene verlinkt, ist das ein
        //     gescheiterter Versuch.
        if umschlag.is_empty() || !self.ebenen_intakt(4) {
            return false;
        }
        let ziel = Pfad::neu(self.ebenen[3].pfad.join(&name));
        let temp = Pfad::neu(self.ebenen[3].pfad.join(format!("{name}.tmp-{}", self.pid)));
        // (2) exklusiv anlegen - ein vorhandener Name ist ein gescheiterter Versuch.
        let Some(mut datei) = dateisystem.lege_exklusiv_an(&temp) else {
            return false;
        };
        // (3) schreiben, spuelen, schliessen.
        let geschrieben = dateisystem.schreibe_und_spuele(&mut datei, umschlag);
        drop(datei);
        // (4) Groesse: groesser 0 und gleich der geschriebenen Bytes.
        if !geschrieben || dateisystem.groesse(&temp) != Some(umschlag.len() as u64) {
            return false;
        }
        // (5) ohne Ersetzen in den Antwortnamen. (6) kein Aufraeumer.
        dateisystem.benenne_um_ohne_ersetzen(&temp, &ziel)
    }

    /// F-11 (3): die Ebenen vor dem Lesen (3) beziehungsweise Schreiben (4).
    fn ebenen_intakt(&self, anzahl: usize) -> bool {
        self.ebenen[..anzahl].iter().all(|ebene| {
            let attribute = self.fassaden.dateisystem.attribute(ebene);
            attribute.existiert && attribute.verzeichnis && !ist_verlinkt(&attribute)
        })
    }
}

fn grund(feld: &str, code: &str, text: &str) -> Value {
    json!({ "feld": feld, "code": code, "text": text })
}

// ── Start, Griff, Stopp (F-1, F-12) ───────────────────────────────────────────────────

/// Der Griff des Brokerlaufs auf den Briefkasten, in `BrokerLauf` in derselben Huelle wie
/// die Geschwister; sein Destruktor stoppt (F-12).
pub(crate) struct BriefkastenGriff {
    grund: Startgrund,
    zaehlwerk: Arc<Zaehlwerk>,
    kern: Option<Arc<Kern>>,
    thread: Option<JoinHandle<()>>,
}

/// Startet den Briefkasten des Brokerlaufs mit den echten Fassaden, nach beiden Servern
/// (F-12). Ein Startfehler startet keinen Thread und beruehrt das Register nicht (M-76).
pub(crate) fn briefkasten_starten(register: Arc<Mutex<Register>>, session_token: &str) -> BriefkastenGriff {
    starten_mit(register, session_token, Fassaden::echt(Arc::new(EchteUmgebung)), true)
}

/// F-1: genau eine Abfrage der Umgebungs-Fassade, danach die Ebenen von oben mit je einer
/// Attributpruefung bis zur ersten fehlenden Ebene oder zum ersten Reparse-Punkt (k + 1
/// Pruefungen). Ohne Wurzel oder mit Reparse-Punkt kein Thread; eine fehlende Ebene hindert
/// den Start nicht, sie legt der Anfragende an.
fn starten_mit(register: Arc<Mutex<Register>>, session_token: &str, fassaden: Fassaden, mit_thread: bool) -> BriefkastenGriff {
    let zaehlwerk = Arc::new(Zaehlwerk::default());
    let mut griff = BriefkastenGriff { grund: Startgrund::Konfiguration, zaehlwerk: Arc::clone(&zaehlwerk), kern: None, thread: None };
    // F-3: die Laufzeitkennung ist das Sitzungskuerzel ohne Bindestriche.
    let laufzeit32: String = session_token.chars().filter(|zeichen| *zeichen != '-').collect();
    if !kennung_gueltig(laufzeit32.as_bytes()) {
        return griff;
    }
    let Some(wurzel) = fassaden.umgebung.localappdata().filter(|wert| !wert.is_empty()) else {
        griff.grund = Startgrund::KeineWurzel;
        return griff;
    };
    let mut ordner = PathBuf::from(wurzel);
    let ebenen = EBENEN.map(|ebene| {
        ordner.push(ebene);
        Pfad::neu(ordner.clone())
    });
    for ebene in &ebenen {
        let vorgefunden = fassaden.dateisystem.attribute(ebene);
        if ist_verlinkt(&vorgefunden) {
            griff.grund = Startgrund::ReparsePunkt;
            return griff;
        }
        if !vorgefunden.existiert || !vorgefunden.verzeichnis {
            break;
        }
    }
    let anfrage = Pfad::neu(ebenen[2].pfad.join("anfrage.json"));
    let ring = [[0; 32]; KENNUNGSRING];
    let zustand = TaktZustand { merker: Merker::default(), offen: Offen::default(), ring, ring_naechster: 0, ring_belegt: 0 };
    let kern = Arc::new(Kern {
        register,
        fassaden,
        zaehlwerk: Arc::clone(&zaehlwerk),
        instanz_id: session_token.to_owned(),
        instanz16: instanz16(session_token),
        laufzeit32,
        pid: std::process::id(),
        ebenen,
        anfrage,
        zustand: Mutex::new(zustand),
        weck: Mutex::new(Weck::default()),
        weckruf: Condvar::new(),
        meldung: Condvar::new(),
        #[cfg(test)]
        haken_takt: Mutex::new(None),
    });
    if mit_thread {
        let fuer_thread = Arc::clone(&kern);
        let gestartet = std::thread::Builder::new().name(THREAD_NAME.to_string()).spawn(move || fuer_thread.lauf());
        let Ok(thread) = gestartet else {
            griff.grund = Startgrund::OhneThread;
            return griff;
        };
        griff.thread = Some(thread);
        zaehle(&zaehlwerk.starts);
        zaehle(&STARTS);
    }
    griff.kern = Some(kern);
    griff.grund = Startgrund::Gestartet;
    griff
}

impl BriefkastenGriff {
    /// F-12: das Stoppflag unter der Sperre setzen, wecken und den Thread mit einer Frist von
    /// 2 s zu Ende warten. Idempotent.
    pub(crate) fn stoppen(&mut self) {
        self.stoppen_mit(false);
    }

    /// `handschlag` (nur Tests, M-45): das Stoppsignal erst setzen, wenn der Thread in der
    /// Wartebedingung steht - unter derselben Sperre.
    fn stoppen_mit(&mut self, handschlag: bool) {
        let Some(kern) = self.kern.as_ref() else {
            return;
        };
        {
            let mut weck = sperre(&kern.weck);
            while handschlag && self.thread.is_some() && !weck.im_warten && !weck.beendet {
                weck = kern.meldung.wait(weck).unwrap_or_else(PoisonError::into_inner);
            }
            weck.stopp = true;
            kern.weckruf.notify_all(); // der Stoppweg weckt (M-45 (1))
        }
        let Some(thread) = self.thread.take() else {
            return;
        };
        let beginn = Instant::now();
        let mut weck = sperre(&kern.weck);
        while !weck.beendet && beginn.elapsed() < JOIN_FRIST {
            let rest = JOIN_FRIST.saturating_sub(beginn.elapsed());
            weck = kern.meldung.wait_timeout(weck, rest).unwrap_or_else(PoisonError::into_inner).0;
        }
        let beendet = weck.beendet;
        drop(weck);
        if beendet {
            let _ = thread.join();
            let dauer = u64::try_from(beginn.elapsed().as_micros()).unwrap_or(u64::MAX);
            kern.zaehlwerk.join_dauer_us.store(dauer, Ordering::SeqCst);
        } else {
            // Frist verstrichen: der Thread bleibt sich selbst ueberlassen. Er haelt nur
            // geteilten Speicher und prueft das Stoppflag vor jedem Takt.
            zaehle(&kern.zaehlwerk.join_frist_verfehlt);
            drop(thread);
        }
    }
}

impl Drop for BriefkastenGriff {
    fn drop(&mut self) {
        self.stoppen();
    }
}

// ── Probelauf fuer den Integrationstest (M-46) ────────────────────────────────────────

/// Ein v2-Server auf einem Namen unter PROBE_PRAEFIX mit eigenem Register und ein Briefkasten
/// auf einer uebergebenen Wurzel, der dieses Register beantwortet: Ende zu Ende ohne die
/// Produktions-Pipe. `probe_lauf` selbst startet keinen Briefkasten.
pub struct BriefkastenProbeLauf {
    briefkasten: BriefkastenGriff,
    server: Option<ServerGriff>,
    register: Arc<Mutex<Register>>,
}

/// Startet den Probelauf; jeder Name ausserhalb des Probe-Namensraums wird verweigert
/// (GRENZE: keine Produktions-Pipe fuer Testbroker).
pub fn briefkasten_probe_lauf_starten(pipe_name: &str, wurzel: &Path) -> Result<BriefkastenProbeLauf, String> {
    if !ist_probe_pipename(pipe_name) {
        return Err(format!("{pipe_name} liegt nicht im Probe-Namensraum"));
    }
    let register = Arc::new(Mutex::new(Register::default()));
    let session_token = uuid::Uuid::new_v4().to_string();
    let server = server_starten(pipe_name, Arc::clone(&register), broker_version(), session_token.clone())?;
    let fassaden = Fassaden::echt(Arc::new(FesteWurzel(wurzel.as_os_str().to_owned())));
    let briefkasten = starten_mit(Arc::clone(&register), &session_token, fassaden, true);
    Ok(BriefkastenProbeLauf { briefkasten, server: Some(server), register })
}

impl BriefkastenProbeLauf {
    /// Die Sensor-IDs im Register dieses Laufs, kopiert unter dem Mutex.
    pub fn sensor_ids(&self) -> Vec<String> {
        let sensoren = sperre(&self.register).sensoren_snapshot(jetzt_ms());
        sensoren.into_iter().map(|sensor| sensor.sensor_id).collect()
    }

    /// Laeuft der Thread des Briefkastens dieses Laufs?
    pub fn briefkasten_gestartet(&self) -> bool {
        let griff = &self.briefkasten;
        griff.grund == Startgrund::Gestartet && griff.thread.is_some() && griff.zaehlwerk.starts.load(Ordering::SeqCst) == 1
    }

    /// Die Laufzeitkennung, die der Briefkasten in Namen und Kopf schreibt (F-3).
    pub fn laufzeit_id(&self) -> Option<String> {
        self.briefkasten.kern.as_ref().map(|kern| kern.laufzeit32.clone())
    }
}

impl Drop for BriefkastenProbeLauf {
    fn drop(&mut self) {
        // Zuerst der Briefkasten, dann der Server - wie im Brokerlauf (M-45).
        self.briefkasten.stoppen();
        drop(self.server.take());
    }
}

#[cfg(test)]
mod tests {
    //! Matrixzeilen M-41 bis M-45, M-47, M-76 und die Rust-Haelften von M-48, M-49 und M-51
    //! bis M-53 (docs/beweise/NAK-286.md §4.4 bis §4.6). Jede Meldung nennt ihre Zeile; das
    //! Rotskript sucht sie in der Rohausgabe.

    use super::*;
    use crate::testhilfe::{fenster, hello, messbereit};
    use std::alloc::{GlobalAlloc, Layout, System};
    use std::cell::Cell;
    use std::collections::BTreeSet;
    use std::sync::atomic::{AtomicI64, AtomicU8};
    use std::sync::OnceLock;
    use std::time::UNIX_EPOCH;

    /// Eine Frist, die in keinem Testlauf ablaeuft: Takte kommen nur ueber Ausloesungen.
    const LANG: Duration = Duration::from_secs(3600);
    /// 2026-09-14T12:00:00.000Z
    const BASIS_MS: i64 = 1_789_387_200_000;
    const VORHER: &str = "NAK-286 Testbuehne: Fehlerliste vor dem Start";

    // M-51: zaehlt Allokationen nur auf dem Thread, der danach fragt.
    struct ZaehlenderAllokator;
    static ALLOKATIONEN: AtomicU64 = AtomicU64::new(0);
    thread_local! {
        static ZAEHLT: Cell<bool> = const { Cell::new(false) };
    }

    // SAFETY: reicht jede Anforderung unveraendert an den Systemallokator weiter und zaehlt nur.
    unsafe impl GlobalAlloc for ZaehlenderAllokator {
        unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
            if ZAEHLT.try_with(Cell::get).unwrap_or(false) {
                zaehle(&ALLOKATIONEN);
            }
            // SAFETY: der Aufrufer haelt den Vertrag von GlobalAlloc ein.
            unsafe { System.alloc(layout) }
        }

        unsafe fn dealloc(&self, zeiger: *mut u8, layout: Layout) {
            // SAFETY: `zeiger` stammt aus `alloc` dieses Allokators mit demselben `layout`.
            unsafe { System.dealloc(zeiger, layout) }
        }
    }

    #[global_allocator]
    static ALLOKATOR: ZaehlenderAllokator = ZaehlenderAllokator;

    fn wert(zaehler: &AtomicU64) -> u64 {
        zaehler.load(Ordering::SeqCst)
    }

    /// n > 0: die naechsten n Aufrufe scheitern; -1: jeder.
    fn injiziert(zaehler: &AtomicI64) -> bool {
        let naechster = |n: i64| if n > 0 { Some(n - 1) } else { None };
        zaehler.fetch_update(Ordering::SeqCst, Ordering::SeqCst, naechster).is_ok() || zaehler.load(Ordering::SeqCst) < 0
    }

    #[derive(Default)]
    struct Injektion {
        lesen: AtomicI64,
        anlegen: AtomicI64,
        schreiben: AtomicI64,
        umbenennen: AtomicI64,
    }

    /// Die Attrappe der Dateisystem-Fassade: echte Aufrufe ueber der Temp-Wurzel,
    /// Fehlerinjektion je Schritt und ein Haken im Schreibschritt (F-14).
    #[derive(Default)]
    struct TestDateisystem {
        wurzel: PathBuf,
        injektion: Injektion,
        ausserhalb: AtomicU64,
        beim_schreiben: Mutex<Option<Box<dyn Fn() + Send + Sync>>>,
    }

    impl TestDateisystem {
        fn pruefe(&self, pfade: &[&Pfad]) {
            for pfad in pfade {
                if !pfad.pfad.starts_with(&self.wurzel) {
                    zaehle(&self.ausserhalb);
                }
            }
        }
    }

    impl DateisystemSchritte for TestDateisystem {
        fn attribute(&self, pfad: &Pfad) -> DateiAttribute {
            self.pruefe(&[pfad]);
            EchtesDateisystem.attribute(pfad)
        }

        fn lies(&self, pfad: &Pfad, ziel: &mut [u8]) -> Option<usize> {
            self.pruefe(&[pfad]);
            if injiziert(&self.injektion.lesen) { None } else { EchtesDateisystem.lies(pfad, ziel) }
        }

        fn lege_exklusiv_an(&self, pfad: &Pfad) -> Option<File> {
            self.pruefe(&[pfad]);
            if injiziert(&self.injektion.anlegen) { None } else { EchtesDateisystem.lege_exklusiv_an(pfad) }
        }

        fn schreibe_und_spuele(&self, datei: &mut File, daten: &[u8]) -> bool {
            if let Some(haken) = sperre(&self.beim_schreiben).as_ref() {
                haken();
            }
            !injiziert(&self.injektion.schreiben) && EchtesDateisystem.schreibe_und_spuele(datei, daten)
        }

        fn groesse(&self, pfad: &Pfad) -> Option<u64> {
            self.pruefe(&[pfad]);
            EchtesDateisystem.groesse(pfad)
        }

        fn benenne_um_ohne_ersetzen(&self, von: &Pfad, nach: &Pfad) -> bool {
            self.pruefe(&[von, nach]);
            !injiziert(&self.injektion.umbenennen) && EchtesDateisystem.benenne_um_ohne_ersetzen(von, nach)
        }

        fn lege_ordner_an(&self, pfad: &Pfad) -> bool {
            self.pruefe(&[pfad]);
            EchtesDateisystem.lege_ordner_an(pfad)
        }

        fn loesche(&self, pfad: &Pfad) -> bool {
            self.pruefe(&[pfad]);
            EchtesDateisystem.loesche(pfad)
        }
    }

    struct TestUmgebung(Option<OsString>);

    impl UmgebungSchritte for TestUmgebung {
        fn localappdata(&self) -> Option<OsString> {
            self.0.clone()
        }
    }

    struct TestUhr {
        ms: AtomicI64,
        frist: Duration,
    }

    impl Uhr for TestUhr {
        fn jetzt_utc_ms(&self) -> i64 {
            self.ms.load(Ordering::SeqCst)
        }

        fn taktfrist(&self) -> Duration {
            self.frist
        }
    }

    /// Der Zaehlerstand der Dateisystem-Fassade.
    #[derive(Debug, Clone, Copy, Default, PartialEq, Eq)]
    struct Stand {
        existenz: u64,
        lese: u64,
        bytes: u64,
        anlegen: u64,
        schreiben: u64,
        groesse: u64,
        umbenennen: u64,
        ordner: u64,
        loeschen: u64,
    }

    impl Stand {
        fn von(fs: &Dateisystem) -> Self {
            let z = &fs.z;
            Self::aus(
                [&z.existenz, &z.lese, &z.bytes, &z.anlegen, &z.schreiben, &z.groesse, &z.umbenennen, &z.ordner, &z.loeschen]
                    .map(wert),
            )
        }

        fn aus(w: [u64; 9]) -> Self {
            let [existenz, lese, bytes, anlegen, schreiben, groesse, umbenennen, ordner, loeschen] = w;
            Self { existenz, lese, bytes, anlegen, schreiben, groesse, umbenennen, ordner, loeschen }
        }

        fn werte(self) -> [u64; 9] {
            let s = self;
            [s.existenz, s.lese, s.bytes, s.anlegen, s.schreiben, s.groesse, s.umbenennen, s.ordner, s.loeschen]
        }

        /// Das Fenster seit `vorher` (Start- gegen Taktfenster, F-14).
        fn seit(self, vorher: Self) -> Self {
            let (jetzt, alt) = (self.werte(), vorher.werte());
            Self::aus(std::array::from_fn(|i| jetzt[i] - alt[i]))
        }

        /// Alle Aufrufe; die gelesenen Bytes sind keine Aufrufzahl.
        fn aufrufe(self) -> u64 {
            self.werte().iter().sum::<u64>() - self.bytes
        }
    }

    /// Das %LOCALAPPDATA% einer Testbuehne. Ordner legt die Buehne an, nie die Instanz (T-4).
    struct TempWurzel {
        wurzel: PathBuf,
    }

    impl TempWurzel {
        fn neu(diagnose: bool, antwort: bool) -> Self {
            let name = format!("nak286-bk-{}-{}", std::process::id(), uuid::Uuid::new_v4().simple());
            let t = Self { wurzel: std::env::temp_dir().join(name) };
            std::fs::create_dir_all(t.ebene(2)).expect("Temp-Wurzel anlegen");
            if diagnose {
                std::fs::create_dir(t.ebene(3)).expect("diagnose anlegen");
            }
            if diagnose && antwort {
                std::fs::create_dir(t.ebene(4)).expect("antwort anlegen");
            }
            t
        }

        /// Die ersten Ebenen unter der Wurzel, von oben.
        fn ebene(&self, anzahl: usize) -> PathBuf {
            EBENEN[..anzahl].iter().fold(self.wurzel.clone(), |pfad, ebene| pfad.join(ebene))
        }

        fn antwort(&self) -> PathBuf {
            self.ebene(4)
        }

        fn anfrage(&self) -> PathBuf {
            self.ebene(3).join("anfrage.json")
        }
    }

    impl Drop for TempWurzel {
        fn drop(&mut self) {
            let _ = std::fs::remove_dir_all(&self.wurzel);
        }
    }

    /// Griff, Fassaden, Register und Temp-Wurzel eines Tests. Der Griff steht zuerst und endet
    /// deshalb vor der Wurzel.
    struct Buehne {
        griff: BriefkastenGriff,
        fassaden: Fassaden,
        fs: Arc<TestDateisystem>,
        uhr: Arc<TestUhr>,
        register: Arc<Mutex<Register>>,
        token: String,
        t: TempWurzel,
    }

    fn buehne_mit(t: TempWurzel, mit_thread: bool, frist: Duration, wurzel: Option<OsString>) -> Buehne {
        let fs = Arc::new(TestDateisystem { wurzel: t.wurzel.clone(), ..TestDateisystem::default() });
        let uhr = Arc::new(TestUhr { ms: AtomicI64::new(BASIS_MS), frist });
        let umgebung = Arc::new(Umgebung::neu(Arc::new(TestUmgebung(wurzel))));
        let fassaden = Fassaden { umgebung, dateisystem: Arc::new(Dateisystem::neu(fs.clone())), uhr: uhr.clone() };
        let mut register = Register::default();
        register.fehler_merken(VORHER.to_string());
        let register = Arc::new(Mutex::new(register));
        let token = uuid::Uuid::new_v4().to_string();
        let griff = starten_mit(Arc::clone(&register), &token, fassaden.clone(), mit_thread);
        Buehne { griff, fassaden, fs, uhr, register, token, t }
    }

    /// Eine Buehne mit allen vier Ebenen.
    fn buehne(mit_thread: bool, frist: Duration) -> Buehne {
        let t = TempWurzel::neu(true, true);
        let wurzel = Some(t.wurzel.clone().into_os_string());
        buehne_mit(t, mit_thread, frist, wurzel)
    }

    impl Buehne {
        /// Takte synchron auf dem Testthread (F-16).
        fn takte(&self, anzahl: usize) {
            let kern = self.griff.kern.as_deref().expect("Briefkasten gestartet");
            (0..anzahl).for_each(|_| kern.takt());
        }

        fn stand(&self) -> Stand {
            Stand::von(&self.fassaden.dateisystem)
        }

        fn z(&self) -> &Zaehlwerk {
            &self.griff.zaehlwerk
        }

        /// Schreibt die Anfrage wie der Runner, mit eigener Aenderungszeit.
        fn anfrage(&self, kennung: &str) {
            schreibe_mit_zeit(&self.t.anfrage(), anfrage_text(kennung).as_bytes());
        }

        fn antworten(&self, kennung: &str) -> Vec<PathBuf> {
            let praefix = format!("{kennung}.{ROLLE}.");
            let passt = |pfad: &PathBuf| name(pfad).starts_with(&praefix) && name(pfad).ends_with(".json");
            dateien_in(&self.t.antwort()).into_iter().filter(passt).collect()
        }

        /// Loest einen Takt im Thread aus (Testzugang der Weckbedingung).
        fn ausloesen(&self) {
            if let Some(kern) = self.griff.kern.as_ref() {
                sperre(&kern.weck).ausloesungen += 1;
                kern.weckruf.notify_all();
            }
        }

        /// Wartet an der Meldung des Threads, bis die Bedingung gilt - nie ein Schlaf.
        fn warte(&self, frist: Duration, bedingung: impl Fn(&Weck) -> bool) -> bool {
            let Some(kern) = self.griff.kern.as_ref() else {
                return false;
            };
            let beginn = Instant::now();
            let mut weck = sperre(&kern.weck);
            while !bedingung(&weck) {
                if beginn.elapsed() >= frist {
                    return false;
                }
                let rest = frist.saturating_sub(beginn.elapsed());
                weck = kern.meldung.wait_timeout(weck, rest).unwrap_or_else(PoisonError::into_inner).0;
            }
            true
        }
    }

    fn neue_kennung() -> String {
        uuid::Uuid::new_v4().simple().to_string()
    }

    /// `format` einer gueltigen Anfrage aus der Vertragsdatei, nie aus der Konstante des Lesers:
    /// sonst saehe kein Test, dass die Konstante vom Vertrag abweicht (Ursache (l), §34).
    fn vertrag_format() -> &'static str {
        static FORMAT: OnceLock<String> = OnceLock::new();
        FORMAT.get_or_init(|| {
            let vertrag = schema("diagnose/nakama-diagnose-anfrage-v1.schema.json");
            vertrag["properties"]["format"]["const"].as_str().expect("format im Anfrageschema").to_string()
        })
    }

    fn anfrage_text(kennung: &str) -> String {
        format!("{{\"format\":\"{}\",\"anfrage_id\":\"{kennung}\"}}", vertrag_format())
    }

    static AENDERUNGEN: AtomicU64 = AtomicU64::new(0);

    /// Schreibt eine Datei mit einer eigenen Aenderungszeit je Aufruf: zwei Anfragen gleicher
    /// Groesse unterscheidet der Merker nur daran (T-5).
    fn schreibe_mit_zeit(pfad: &Path, inhalt: &[u8]) {
        std::fs::write(pfad, inhalt).expect("Datei schreiben");
        let zeit = UNIX_EPOCH + Duration::from_secs(1_800_000_000 + 10 * AENDERUNGEN.fetch_add(1, Ordering::SeqCst));
        let datei = OpenOptions::new().write(true).open(pfad);
        datei.and_then(|datei| datei.set_modified(zeit)).expect("Aenderungszeit setzen");
    }

    fn dateien_in(ordner: &Path) -> Vec<PathBuf> {
        let eintraege = std::fs::read_dir(ordner).into_iter().flatten().filter_map(Result::ok);
        let mut dateien: Vec<PathBuf> = eintraege.map(|eintrag| eintrag.path()).filter(|pfad| pfad.is_file()).collect();
        dateien.sort();
        dateien
    }

    fn leer(ordner: &Path) -> bool {
        std::fs::read_dir(ordner).is_ok_and(|mut eintraege| eintraege.next().is_none())
    }

    fn name(pfad: &Path) -> String {
        pfad.file_name().map(|teil| teil.to_string_lossy().into_owned()).unwrap_or_default()
    }

    fn lies_json(pfad: &Path) -> Value {
        serde_json::from_slice(&std::fs::read(pfad).expect("Antwort lesbar")).expect("Antwort ist JSON")
    }

    fn sha(pfad: &Path) -> String {
        Sha256::digest(std::fs::read(pfad).unwrap_or_default()).iter().map(|byte| format!("{byte:02x}")).collect()
    }

    /// Feld, Code und Text je Grund.
    fn gruende(umschlag: &Value) -> Vec<(String, String, String)> {
        let text = |wert: &Value| wert.as_str().unwrap_or_default().to_string();
        let liste = umschlag["gruende"].as_array().into_iter().flatten();
        liste.map(|g| (text(&g["feld"]), text(&g["code"]), text(&g["text"]))).collect()
    }

    /// Eine Junction ohne Adminrechte, wie im C++-Test (mklink /J).
    fn lege_junction_an(link: &Path, ziel: &Path) -> bool {
        let befehl = std::process::Command::new("cmd").args(["/c", "mklink", "/J"]).arg(link).arg(ziel).output();
        befehl.is_ok_and(|ausgabe| ausgabe.status.success())
            && EchtesDateisystem.attribute(&Pfad::neu(link.to_path_buf())).reparse
    }

    /// Zwei v2-Sensoren mit Messstand (Testhelfer wie aggregat.rs:192-218).
    fn zwei_sensoren(register: &Mutex<Register>) {
        let mut r = sperre(register);
        r.sensor_verbinden(&hello("s-pre", "pre", "PIANO VORHER", Some("p1"), 1), "n-1");
        r.sensor_verbinden(&hello("s-post", "post", "PIANO NACHHER", Some("p1"), 1), "n-2");
        r.heartbeat("s-pre", "n-1", None, Some(messbereit(Some(fenster(0, 480_000, 0)), 10.0)));
        r.heartbeat("s-post", "n-2", None, Some(messbereit(Some(fenster(48_000, 500_000, 0)), 10.0)));
    }

    /// Eine laufende Hoer-Markierung an s-pre sperrt das Aggregat (paar.rs:64-76).
    fn markiere(register: &Mutex<Register>) {
        let mut stand = messbereit(Some(fenster(0, 480_000, 0)), 10.0);
        stand.hoermarkierung = true;
        sperre(register).heartbeat("s-pre", "n-1", None, Some(stand));
    }

    // ── Leser, Zeitstempel, Fassade ───────────────────────────────────────────────────────

    #[test]
    fn anfrage_und_zeitstempel_wie_plugin() {
        let k = neue_kennung();
        assert_eq!(anfrage_lesen(anfrage_text(&k).as_bytes()).map(|x| kennung_text(&x)), Some(k.clone()));
        assert!(anfrage_lesen(format!(" \r\n\t{} \n", anfrage_text(&k)).as_bytes()).is_some(), "Leerraum ist erlaubt");
        let doppelt = format!("{{\"format\":\"{}\",\"anfrage_id\":\"{k}\",\"anfrage_id\":\"{k}\"}}", vertrag_format());
        assert!(anfrage_lesen(doppelt.as_bytes()).is_none(), "ein doppelter Schluessel ist fremd");
        let faelle = [
            (BASIS_MS, "2026-09-14T12:00:00.000Z"),
            (951_782_400_123, "2000-02-29T00:00:00.123Z"),
            (32_535_215_999_999, "3000-12-31T23:59:59.999Z"),
            (32_535_216_000_000, "1970-01-01T00:00:00.000Z"),
            (-1, "1970-01-01T00:00:00.000Z"),
        ];
        for (eingabe, soll) in faelle {
            assert_eq!(iso_utc(eingabe), soll, "iso_utc({eingabe})");
        }
    }

    #[test]
    fn fassade_zaehlt_jeden_schritt() {
        let t = TempWurzel::neu(true, true);
        let fs = Dateisystem::neu(Arc::new(TestDateisystem { wurzel: t.wurzel.clone(), ..TestDateisystem::default() }));
        let pfad = |teil: &str| Pfad::neu(t.antwort().join(teil));
        let (datei, ziel, ordner) = (pfad("probe.json"), pfad("ziel.json"), pfad("ordner"));
        let mut griff = fs.lege_exklusiv_an(&datei).expect("exklusiv anlegen");
        assert!(fs.schreibe_und_spuele(&mut griff, b"12345") && fs.lege_exklusiv_an(&datei).is_none());
        drop(griff);
        let mut puffer = [0u8; 8];
        assert_eq!((fs.lies(&datei, &mut puffer), fs.groesse(&datei)), (Some(5), Some(5)));
        std::fs::write(&ziel.pfad, b"liegt").expect("Ziel belegen");
        assert!(fs.attribute(&datei).existiert && !fs.benenne_um_ohne_ersetzen(&datei, &ziel), "nie ersetzt");
        assert_eq!(std::fs::read(&ziel.pfad).ok(), Some(b"liegt".to_vec()));
        assert!(fs.lege_ordner_an(&ordner) && fs.loesche(&ziel));
        let soll = Stand { existenz: 1, lese: 1, bytes: 5, anlegen: 2, schreiben: 1, groesse: 1, umbenennen: 1, ordner: 1, loeschen: 1 };
        assert_eq!(Stand::von(&fs), soll, "jeder Schritt an der Fassade gezaehlt");
    }

    // ── M-41, M-76: Takt- und Startfenster ────────────────────────────────────────────────

    #[test]
    fn ohne_anfrage_nur_existenzpruefung() {
        let b = buehne(true, LANG);
        let (start, umgebung) = (b.stand(), wert(&b.fassaden.umgebung.abfragen));
        assert_eq!(b.griff.grund, Startgrund::Gestartet, "M-41 ohne_anfrage_nur_existenzpruefung vorbedingung");
        // Startfenster (F-1): vier vorhandene Ebenen, je eine Attributpruefung; eine Umgebungsabfrage.
        let startfenster = (start.existenz, start.aufrufe(), umgebung);
        assert_eq!(startfenster, (4, 4, 1), "M-41 ohne_anfrage_nur_existenzpruefung startfenster: {start:?}");
        for takt in 1..=600 {
            b.ausloesen();
            let fertig = b.warte(Duration::from_secs(10), |weck| weck.takte_fertig >= takt);
            assert!(fertig, "M-41 ohne_anfrage_nur_existenzpruefung: Takt {takt} blieb aus");
        }
        let fenster = b.stand().seit(start);
        let lese = fenster.lese;
        let soll = Stand { existenz: 600, ..Stand::default() };
        assert_eq!(fenster, soll, "M-41 ohne_anfrage_nur_existenzpruefung taktfenster (600 Takte): Leseoeffnungen {lese}");
        let umgebung_takt = wert(&b.fassaden.umgebung.abfragen) - umgebung;
        assert_eq!(umgebung_takt, 0, "M-41 ohne_anfrage_nur_existenzpruefung taktfenster: Umgebungsabfragen");
        assert!(dateien_in(&b.t.antwort()).is_empty(), "M-41 ohne_anfrage_nur_existenzpruefung: Antwortordner leer");
    }

    /// Ein Startfehler: kein Thread, die Registerfehlerliste bleibt, danach kein Zugriff.
    fn pruefe_startfehler(b: &Buehne, durchlauf: &str, grund: Startgrund, startfenster: Stand) {
        let fehler = sperre(&b.register).fehler.clone();
        assert_eq!(fehler, [VORHER], "M-76 startfehler_ohne_registerzeile {durchlauf}: Registerfehlerliste veraendert");
        let start = b.stand();
        assert_eq!(start, startfenster, "M-76 startfehler_ohne_registerzeile {durchlauf} startfenster: {start:?}");
        let (ist_grund, abfragen) = (b.griff.grund, wert(&b.fassaden.umgebung.abfragen));
        assert_eq!((ist_grund, abfragen), (grund, 1), "M-76 startfehler_ohne_registerzeile {durchlauf}: Grund, Umgebungsabfragen");
        let ohne_thread = b.griff.thread.is_none() && b.griff.kern.is_none() && wert(&b.z().starts) == 0;
        assert!(ohne_thread, "M-76 startfehler_ohne_registerzeile {durchlauf}: Startzaehler ungleich 0");
        (0..5).for_each(|_| b.ausloesen());
        assert_eq!(b.stand(), start, "M-76 startfehler_ohne_registerzeile {durchlauf}: Zugriffe nach dem Start");
    }

    #[test]
    fn startfehler_ohne_registerzeile() {
        // Durchlauf 1: LOCALAPPDATA ist nicht gesetzt - es gibt keine Ebene zu pruefen.
        let b = buehne_mit(TempWurzel::neu(true, true), true, LANG, None);
        pruefe_startfehler(&b, "durchlauf 1 (LOCALAPPDATA fehlt)", Startgrund::KeineWurzel, Stand::default());
        // Durchlauf 2: evenacadia\ ist beim Start eine Junction auf einen Baum mit Anfrage.
        let t = TempWurzel::neu(false, false);
        let ziel = t.wurzel.join("ziel-evenacadia");
        std::fs::rename(t.ebene(1), &ziel).expect("evenacadia verschieben");
        let diagnose = ziel.join("nakama").join("diagnose");
        std::fs::create_dir_all(diagnose.join("antwort")).expect("Zielbaum anlegen");
        std::fs::write(diagnose.join("anfrage.json"), anfrage_text(&neue_kennung())).expect("Anfrage schreiben");
        assert!(lege_junction_an(&t.ebene(1), &ziel), "M-76 startfehler_ohne_registerzeile durchlauf 2 vorbedingung");
        let wurzel = Some(t.wurzel.clone().into_os_string());
        let b = buehne_mit(t, true, LANG, wurzel);
        let startfenster = Stand { existenz: 1, ..Stand::default() };
        pruefe_startfehler(&b, "durchlauf 2 (evenacadia reparse)", Startgrund::ReparsePunkt, startfenster);
    }

    // ── M-42, M-44, M-47: Umschlag, Sperre, Register-Mutex ────────────────────────────────

    /// Eine Buehne mit zwei Sensoren, auf Wunsch gesperrt; eine Anfrage, ein Takt.
    fn eine_antwort(gesperrt: bool) -> (Buehne, String, Stand) {
        let b = buehne(false, LANG);
        zwei_sensoren(&b.register);
        if gesperrt {
            markiere(&b.register);
        }
        let kennung = neue_kennung();
        b.anfrage(&kennung);
        let vorher = b.stand();
        b.takte(1);
        (b, kennung, vorher)
    }

    #[test]
    fn anfrage_broker_genau_eine_antwort() {
        let (b, k, vorher) = eine_antwort(false);
        let (laufzeit32, version) = (b.token.replace('-', ""), broker_version());
        let soll_name = format!("{k}.broker.{}.{}.{laufzeit32}.json", std::process::id(), instanz16(&b.token));
        let dateien = dateien_in(&b.t.antwort());
        let namen: Vec<String> = dateien.iter().map(|pfad| name(pfad)).collect();
        assert_eq!(namen, [soll_name], "M-42 anfrage_broker_genau_eine_antwort: genau eine Datei im ersten Takt");
        let bytes = anfrage_text(&k).len() as u64;
        let soll = Stand { existenz: 8, lese: 1, bytes, anlegen: 1, schreiben: 1, groesse: 1, umbenennen: 1, ..Stand::default() };
        assert_eq!(b.stand().seit(vorher), soll, "M-42 anfrage_broker_genau_eine_antwort: jeder Zugriff an der Fassade");
        let u = lies_json(&dateien[0]);
        let kopf = ["format", "anfrage_id", "rolle", "instanz_id", "laufzeit_id", "erzeugt_utc", "version"]
            .map(|schluessel| u[schluessel].as_str().unwrap_or_default().to_string());
        let soll_kopf = [ANTWORT_FORMAT, k.as_str(), "broker", b.token.as_str(), laufzeit32.as_str(), "2026-09-14T12:00:00.000Z", version.as_str()];
        assert_eq!(kopf, soll_kopf, "M-42 anfrage_broker_genau_eine_antwort: Kopf");
        assert_eq!(u["pid"].as_u64(), Some(u64::from(std::process::id())), "M-42 anfrage_broker_genau_eine_antwort: pid");
        let sensoren = sperre(&b.register).sensoren_snapshot(jetzt_ms());
        let aggregat = aggregat_bauen(&sensoren, None, None, &version, &b.token[..8], BASIS_MS as u64);
        assert_eq!(u["aggregat"], aggregat, "M-42 anfrage_broker_genau_eine_antwort: aggregat = aggregat_bauen ohne Filter");
        assert_eq!(u["aggregat"]["sensoren"].as_array().map(Vec::len), Some(2), "M-42 anfrage_broker_genau_eine_antwort");
        assert!(u["snapshot"].is_null() && u["frame"].is_null(), "M-42 anfrage_broker_genau_eine_antwort: null");
        let codes: Vec<(String, String)> = gruende(&u).into_iter().map(|(feld, code, _)| (feld, code)).collect();
        let soll_codes = [("snapshot", "rolle_ohne_snapshot"), ("frame", "rolle_ohne_frame")].map(|(f, c)| (f.to_string(), c.to_string()));
        assert_eq!(codes, soll_codes, "M-42 anfrage_broker_genau_eine_antwort: Gruende");
    }

    fn schema(relativ: &str) -> Value {
        let pfad = Path::new(env!("CARGO_MANIFEST_DIR")).join("../eq-copilot/schemas").join(relativ);
        serde_json::from_str(&std::fs::read_to_string(pfad).expect("Schemadatei lesbar")).expect("Schema ist JSON")
    }

    fn menge(wert: &Value) -> BTreeSet<String> {
        match wert {
            Value::Object(objekt) => objekt.keys().cloned().collect(),
            Value::Array(liste) => liste.iter().filter_map(|x| x.as_str().map(str::to_string)).collect(),
            _ => BTreeSet::new(),
        }
    }

    /// Feldmenge eines Objekts gegen die Pflichtfelder und die erlaubten Felder seines Schemas.
    fn feldmenge(teil: &str, objekt: &Value, schema: &Value, befunde: &mut Vec<String>) {
        let (ist, pflicht, erlaubt) = (menge(objekt), menge(&schema["required"]), menge(&schema["properties"]));
        let fehlend: Vec<&String> = pflicht.difference(&ist).collect();
        let fremd: Vec<&String> = ist.difference(&erlaubt).collect();
        if !objekt.is_object() || !fehlend.is_empty() || !fremd.is_empty() {
            befunde.push(format!("{teil}: Objekt {}, fehlend {fehlend:?}, fremd {fremd:?}", objekt.is_object()));
        }
    }

    #[test]
    fn antwort_feldmenge_wie_schema() {
        let (antwort, aggregat) = (schema("diagnose/nakama-diagnose-antwort-v1.schema.json"), schema("eq-aggregat.schema.json"));
        let kopf = menge(&antwort["required"]);
        assert!(kopf == menge(&antwort["properties"]) && kopf.len() == 12, "M-42 antwort_feldmenge_wie_schema vorbedingung");
        let mut befunde = Vec::new();
        for gesperrt in [false, true] {
            let (b, k, _) = eine_antwort(gesperrt);
            let dateien = b.antworten(&k);
            assert_eq!(dateien.len(), 1, "M-42 antwort_feldmenge_wie_schema vorbedingung: eine Antwort");
            let (u, lage) = (lies_json(&dateien[0]), if gesperrt { "gesperrt" } else { "mit aggregat" });
            feldmenge(&format!("{lage}: umschlag"), &u, &antwort, &mut befunde);
            let grund_schema = &antwort["properties"]["gruende"]["items"];
            for (i, g) in u["gruende"].as_array().into_iter().flatten().enumerate() {
                feldmenge(&format!("{lage}: gruende[{i}]"), g, grund_schema, &mut befunde);
                for feld in ["feld", "code"] {
                    if !menge(&grund_schema["properties"][feld]["enum"]).contains(g[feld].as_str().unwrap_or_default()) {
                        befunde.push(format!("{lage}: gruende[{i}].{feld} ausserhalb des Schemas"));
                    }
                }
            }
            let muster = [&u["anfrage_id"], &u["laufzeit_id"]].iter().all(|w| kennung_gueltig(w.as_str().unwrap_or_default().as_bytes()));
            let rolle = menge(&antwort["properties"]["rolle"]["enum"]).contains("broker");
            if !muster || !rolle || u["pid"].as_u64().is_none() || u["format"] != antwort["properties"]["format"]["const"] {
                befunde.push(format!("{lage}: Kopfwerte ausserhalb des Schemas"));
            }
            let a = &u["aggregat"];
            if gesperrt {
                if !a.is_null() {
                    befunde.push(format!("{lage}: aggregat nicht null"));
                }
                continue;
            }
            let p = &aggregat["properties"];
            feldmenge(&format!("{lage}: aggregat"), a, &aggregat, &mut befunde);
            feldmenge(&format!("{lage}: aggregat.filter"), &a["filter"], &p["filter"], &mut befunde);
            feldmenge(&format!("{lage}: aggregat.gemeinsames_fenster"), &a["gemeinsames_fenster"], &p["gemeinsames_fenster"], &mut befunde);
            if a["schema"] != p["schema"]["const"] {
                befunde.push(format!("{lage}: aggregat.schema"));
            }
            for liste in ["sensoren", "paare"] {
                for (i, eintrag) in a[liste].as_array().into_iter().flatten().enumerate() {
                    feldmenge(&format!("{lage}: aggregat.{liste}[{i}]"), eintrag, &p[liste]["items"], &mut befunde);
                }
            }
        }
        assert!(befunde.is_empty(), "M-42 antwort_feldmenge_wie_schema: {befunde:?}");
    }

    #[test]
    fn gesperrtes_aggregat_antwort_mit_grund() {
        let (b, k, _) = eine_antwort(true);
        let sperrgrund = aggregat_sperrgrund(&sperre(&b.register).sensoren_snapshot(jetzt_ms())).unwrap_or_default();
        assert!(!sperrgrund.is_empty(), "M-44 gesperrtes_aggregat_antwort_mit_grund vorbedingung: Sperrgrund");
        let dateien = b.antworten(&k);
        assert_eq!(dateien.len(), 1, "M-44 gesperrtes_aggregat_antwort_mit_grund: genau eine Antwort");
        let u = lies_json(&dateien[0]);
        assert!(u["aggregat"].is_null(), "M-44 gesperrtes_aggregat_antwort_mit_grund: Aggregat im Umschlag trotz Sperre");
        let soll = ("aggregat".to_string(), "aggregat_gesperrt".to_string(), sperrgrund);
        assert!(gruende(&u).contains(&soll), "M-44 gesperrtes_aggregat_antwort_mit_grund: Grund fehlt: {:?}", gruende(&u));
    }

    #[test]
    fn register_sperre_nie_ueber_dateischreiben() {
        let b = buehne(false, LANG);
        zwei_sensoren(&b.register);
        let beobachtet = Arc::new(AtomicU8::new(0));
        let (register, zeuge) = (Arc::clone(&b.register), Arc::clone(&beobachtet));
        let haken = move || zeuge.store(if register.try_lock().is_ok() { 1 } else { 2 }, Ordering::SeqCst);
        *sperre(&b.fs.beim_schreiben) = Some(Box::new(haken));
        let k = neue_kennung();
        b.anfrage(&k);
        b.takte(1);
        // 1 = frei, 2 = gehalten, 0 = kein Schreibschritt.
        let gesehen = beobachtet.load(Ordering::SeqCst);
        assert_eq!(gesehen, 1, "M-47 register_sperre_nie_ueber_dateischreiben: Register-Mutex im Schreibschritt nicht frei");
        assert_eq!(b.antworten(&k).len(), 1, "M-47 register_sperre_nie_ueber_dateischreiben: Antwort geschrieben");
    }

    // ── M-43: Kennungsregeln wie im Plugin (Falltabelle M-25 bis M-29, M-33 (d)) ────────────

    /// Die Schreibversuche nach jedem von `takte` Takten, als Folge.
    fn versuchsfolge(b: &Buehne, takte: usize) -> String {
        let folge: Vec<String> = (0..takte)
            .map(|_| {
                b.takte(1);
                wert(&b.z().schreibversuche).to_string()
            })
            .collect();
        folge.join(",")
    }

    #[test]
    fn kennungsregeln_wie_plugin() {
        // (F-2) eine Anfrage nach der Vertragsdatei wird gelesen: der Anfragetext der Tests nimmt
        // `format` aus dem Schema, nicht aus der Konstante des Lesers (Ursache (l)).
        let k = neue_kennung();
        let gelesen = anfrage_lesen(anfrage_text(&k).as_bytes()).map(|x| kennung_text(&x));
        assert_eq!(gelesen, Some(k.clone()), "M-43 kennungsregeln_wie_plugin (F-2) anfrage nach der vertragsdatei gelesen");
        // (M-25) gleiche Kennung, neu geschrieben: keine zweite Antwort, genau eine Leseoeffnung.
        let b = buehne(false, LANG);
        b.anfrage(&k);
        b.takte(1);
        assert_eq!(b.antworten(&k).len(), 1, "M-43 kennungsregeln_wie_plugin (M-25) vorbedingung");
        let s0 = b.stand();
        b.takte(150);
        let s1 = b.stand();
        b.anfrage(&k);
        b.takte(150);
        let (d, nachher) = (b.stand().seit(s0), b.stand().seit(s1));
        let einmal = d.anlegen == 0 && d.schreiben == 0 && dateien_in(&b.t.antwort()).len() == 1;
        assert!(einmal && wert(&b.z().fehlerzaehler) == 0, "M-43 kennungsregeln_wie_plugin (M-25) gleiche kennung keine zweite antwort: {d:?}");
        assert!(s1.seit(s0).lese == 0 && nachher.lese == 1, "M-43 kennungsregeln_wie_plugin (M-25) genau eine leseoeffnung");
        // (M-26) neue Kennung: eine neue Datei, die Datei fuer K bleibt byteweise, nichts geloescht.
        let sha_k = sha(&b.antworten(&k)[0]);
        let l = neue_kennung();
        b.anfrage(&l);
        b.takte(1);
        let neu = b.antworten(&l).len() == 1 && dateien_in(&b.t.antwort()).len() == 2 && sha(&b.antworten(&k)[0]) == sha_k;
        assert!(neu && b.stand().loeschen == 0 && b.t.anfrage().is_file(), "M-43 kennungsregeln_wie_plugin (M-26) neue kennung");
        fremde_anfragen();
        gesperrte_anfrage();
        antwortordner_drei_versuche();
        zielname_belegt();
    }

    /// (M-27) je Variante 5 Takte, dann nach einer Aenderung weitere 5: keine Antwort, gelesen
    /// hoechstens einmal je Stand, die zu grosse nie; kein Logeintrag auf keinem Weg; keine
    /// Kennung im Ring, auch nicht aus einer fremden Wurzelform (P-15).
    fn fremde_anfragen() {
        let b = buehne(false, LANG);
        let kern = b.griff.kern.as_deref().expect("Briefkasten gestartet");
        let k = neue_kennung();
        let mit_id = |id: &str| Some(anfrage_text(id).into_bytes());
        let mut gross = anfrage_text(&k).into_bytes();
        gross.resize(1025, b' ');
        let varianten = [
            ("(a) 1025 Bytes", Some(gross), 0),
            ("(b) kein JSON", Some(b"das ist kein JSON".to_vec()), 1),
            ("(c) zusaetzlicher Schluessel", Some(format!("{{\"format\":\"{}\",\"anfrage_id\":\"{k}\",\"x\":1}}", vertrag_format()).into_bytes()), 1),
            ("(d) format falsch", Some(format!("{{\"format\":\"nakama.diagnose.anfrage.v2\",\"anfrage_id\":\"{k}\"}}").into_bytes()), 1),
            ("(e) Grossbuchstaben", mit_id(&format!("ABCDEF{}", &k[6..])), 1),
            ("(e) 31 Zeichen", mit_id(&k[..31]), 1),
            ("(e) 33 Zeichen", mit_id(&format!("{k}a")), 1),
            ("(e) mit ..", mit_id(&format!("..{}", &k[2..])), 1),
            ("(e) mit Backslash", mit_id(&format!("\\\\{}", &k[1..])), 1),
            ("(e) mit /", mit_id(&format!("/{}", &k[1..])), 1),
            ("(e) mit NUL", mit_id(&format!("\\u0000{}", &k[1..])), 1),
            ("(f) Verzeichnis", None, 0),
            ("(g) leere Datei", Some(Vec::new()), 1),
            ("(h) UTF-8 mit BOM", Some([&[0xEF, 0xBB, 0xBF][..], anfrage_text(&k).as_bytes()].concat()), 1),
            ("(i) JSON-Array mit unbeantworteter Kennung", Some(b"[\"nakama.diagnose.anfrage.v1\",\"00000000000000000000000000000000\"]".to_vec()), 1),
            ("(j) JSON-Zeichenkette als Wurzel", Some(b"\"nakama.diagnose.anfrage.v1\"".to_vec()), 1),
        ];
        for (fall, inhalt, lesen) in varianten {
            let pfad = b.t.anfrage();
            let _ = std::fs::remove_file(&pfad).or_else(|_| std::fs::remove_dir(&pfad));
            b.takte(1); // ohne Datei fallen Merker und offene Kennung
            match &inhalt {
                Some(bytes) => schreibe_mit_zeit(&pfad, bytes),
                None => std::fs::create_dir(&pfad).expect("Verzeichnis anlegen"),
            }
            let s0 = b.stand();
            b.takte(5);
            let d1 = b.stand().seit(s0);
            if let Some(bytes) = &inhalt {
                schreibe_mit_zeit(&pfad, bytes); // erst eine geaenderte Datei wird wieder gelesen (T-5)
            }
            let s1 = b.stand();
            b.takte(5);
            let d2 = b.stand().seit(s1);
            let lesen_nach_aenderung = if inhalt.is_some() { lesen } else { 0 };
            let gelesen = d1.lese == lesen && d2.lese == lesen_nach_aenderung && (lesen != 0 || d1.bytes + d2.bytes == 0);
            let ring = sperre(&kern.zustand).ring_belegt;
            let ohne = d1.anlegen + d2.anlegen == 0 && dateien_in(&b.t.antwort()).is_empty() && wert(&b.z().schreibversuche) == 0 && ring == 0;
            assert!(gelesen && ohne, "M-43 kennungsregeln_wie_plugin (M-27) fremde anfrage {fall}: {d1:?} {d2:?}, Ring {ring}");
        }
        let quelle = include_str!("briefkasten.rs");
        let produkt = &quelle[..quelle.find("\nmod tests {").expect("Testmodul")];
        let ohne_log = ["println!", "eprintln!", "print!", "eprint!", "dbg!", "log::"].iter().all(|makro| !produkt.contains(makro));
        assert!(ohne_log, "M-43 kennungsregeln_wie_plugin (M-27): kein Logeintrag auf keinem Weg");
    }

    /// (M-28) beim Lesen gesperrt: kein Merker, der naechste Takt liest und antwortet.
    fn gesperrte_anfrage() {
        let b = buehne(false, LANG);
        let k = neue_kennung();
        b.anfrage(&k);
        b.fs.injektion.lesen.store(1, Ordering::SeqCst); // "Zugriff verweigert" an der Leseoeffnung
        let s0 = b.stand();
        b.takte(1);
        let (s1, ohne) = (b.stand(), dateien_in(&b.t.antwort()).is_empty());
        b.takte(1);
        assert!(s1.seit(s0).lese == 1 && ohne, "M-43 kennungsregeln_wie_plugin (M-28) gesperrt keine antwort");
        let antwort = b.stand().seit(s1).lese == 1 && b.antworten(&k).len() == 1 && wert(&b.z().fehlerzaehler) == 0;
        assert!(antwort, "M-43 kennungsregeln_wie_plugin (M-28) naechster takt liest und antwortet");
    }

    /// (M-29) hoechstens drei Schreibversuche in drei Takten, nie eine Ordneranlage.
    fn antwortordner_drei_versuche() {
        let ohne_antwortordner = || {
            let t = TempWurzel::neu(true, false);
            let wurzel = Some(t.wurzel.clone().into_os_string());
            buehne_mit(t, false, LANG, wurzel)
        };
        let b = ohne_antwortordner(); // (a) antwort\ fehlt
        b.anfrage(&neue_kennung());
        let (folge, s) = (versuchsfolge(&b, 6), b.stand());
        let ok = folge == "1,2,3,3,3,3" && wert(&b.z().fehlerzaehler) == 1 && s.ordner + s.anlegen == 0 && !b.t.antwort().exists();
        assert!(ok, "M-43 kennungsregeln_wie_plugin (M-29) (a) antwort fehlt: Versuche {folge}, {s:?}");
        let b = buehne(false, LANG); // (b) das exklusive Anlegen scheitert in jedem Takt
        b.fs.injektion.anlegen.store(-1, Ordering::SeqCst);
        b.anfrage(&neue_kennung());
        let (folge, s) = (versuchsfolge(&b, 6), b.stand());
        let ok = folge == "1,2,3,3,3,3" && wert(&b.z().fehlerzaehler) == 1 && s.anlegen == 3 && s.ordner == 0;
        assert!(ok && dateien_in(&b.t.antwort()).is_empty(), "M-43 kennungsregeln_wie_plugin (M-29) (b) anlegen scheitert: Versuche {folge}");
        let b = ohne_antwortordner(); // (c) antwort\ erscheint vor dem dritten Versuch
        let k = neue_kennung();
        b.anfrage(&k);
        let erst = versuchsfolge(&b, 2);
        std::fs::create_dir(b.t.antwort()).expect("antwort anlegen"); // die Buehne, nie die Instanz
        let folge = format!("{erst},{}", versuchsfolge(&b, 2));
        let ok = folge == "1,2,3,3" && b.antworten(&k).len() == 1 && wert(&b.z().fehlerzaehler) == 0 && b.stand().ordner == 0;
        assert!(ok, "M-43 kennungsregeln_wie_plugin (M-29) (c) ordner erscheint vor dem dritten versuch: Versuche {folge}");
    }

    /// (M-33 (d)) der Zielname ist durch eine Markendatei belegt: nie ersetzt.
    fn zielname_belegt() {
        let b = buehne(false, LANG);
        let k1 = neue_kennung();
        b.anfrage(&k1);
        b.takte(1);
        let erste = b.antworten(&k1);
        assert_eq!(erste.len(), 1, "M-43 kennungsregeln_wie_plugin (M-33 (d)) vorbedingung");
        let marke = b.t.antwort().join(format!("{}{}", neue_kennung(), &name(&erste[0])[32..]));
        let k2 = name(&marke)[..32].to_string();
        std::fs::write(&marke, "MARKE: dieser Zielname ist belegt").expect("Markendatei");
        let sha_vorher = sha(&marke);
        b.anfrage(&k2);
        let (v0, f0) = (wert(&b.z().schreibversuche), wert(&b.z().fehlerzaehler));
        let folge: Vec<u64> = (0..5)
            .map(|_| {
                b.takte(1);
                wert(&b.z().schreibversuche) - v0
            })
            .collect();
        let namen: Vec<String> = dateien_in(&b.t.antwort()).iter().map(|pfad| name(pfad)).collect();
        let anzahl = |gesucht: String| namen.iter().filter(|n| **n == gesucht).count();
        let temp = format!("{}.tmp-{}", name(&marke), std::process::id());
        let unveraendert = sha(&marke) == sha_vorher;
        let nie_ersetzt = unveraendert && anzahl(name(&marke)) == 1 && anzahl(temp) == 1 && b.stand().loeschen == 0;
        let versuche = folge == [1, 2, 3, 3, 3] && wert(&b.z().fehlerzaehler) - f0 == 1;
        assert!(nie_ersetzt && versuche, "M-43 kennungsregeln_wie_plugin (M-33 (d)) keine datei wird je ersetzt: Marke unveraendert {unveraendert}, Versuche {folge:?}");
    }

    // ── M-45: Lebensdauer ─────────────────────────────────────────────────────────────────

    #[test]
    fn stopp_weckt_ohne_wartedurchlauf() {
        // Eine endliche Frist: ohne Weckruf endet das Warten erst mit ihr.
        let mut b = buehne(true, Duration::from_secs(3));
        b.anfrage(&neue_kennung());
        b.griff.stoppen_mit(true);
        let beendet = b.warte(Duration::from_secs(10), |weck| weck.beendet);
        let z = b.z();
        let (warten, verfehlt, dauer) = (wert(&z.wartedurchlaeufe_nach_stopp), wert(&z.join_frist_verfehlt), wert(&z.join_dauer_us));
        println!("[roh] M-45 stopp_weckt_ohne_wartedurchlauf join_dauer_us={dauer} join_frist_verfehlt={verfehlt}");
        assert!(beendet, "M-45 (1) stopp_weckt_ohne_wartedurchlauf: Thread nicht beendet");
        assert_eq!(warten, 0, "M-45 (1) stopp_weckt_ohne_wartedurchlauf: Wartedurchlaeufe nach dem Stoppsignal {warten} statt 0");
        assert!(verfehlt == 0 && b.griff.thread.is_none(), "M-45 (1) stopp_weckt_ohne_wartedurchlauf: Join-Frist verfehlt");
    }

    #[test]
    fn nach_stopp_keine_lieferung() {
        let mut b = buehne(true, LANG);
        let kanarienwert = Arc::new(AtomicU64::new(0xC0FFEE));
        let (kanarie, kern) = (Arc::clone(&kanarienwert), b.griff.kern.clone().expect("gestartet"));
        *sperre(&kern.haken_takt) = Some(Box::new(move || kanarie.store(0xDEAD, Ordering::SeqCst)));
        let k = neue_kennung();
        b.anfrage(&k);
        b.griff.stoppen();
        let (s0, fertig) = (b.stand(), sperre(&kern.weck).takte_fertig);
        (0..10).for_each(|_| b.ausloesen());
        let _ = b.warte(Duration::from_secs(3), |weck| weck.takte_fertig >= fertig + 10);
        let d = b.stand().seit(s0);
        assert_eq!(d.existenz, 0, "M-45 (2) nach_stopp_keine_lieferung: Existenzpruefungen nach dem Stopp {} statt 0", d.existenz);
        assert!(b.antworten(&k).is_empty() && wert(&b.z().antworten) == 0, "M-45 (2) nach_stopp_keine_lieferung: Antwort nach dem Stopp");
        assert_eq!(kanarienwert.load(Ordering::SeqCst), 0xC0FFEE, "M-45 (2) nach_stopp_keine_lieferung: Kanarienwert beruehrt");
    }

    /// NAK-309 Etappe 4 (T3-04-04, M-66): der erlaubte Nachlauf nach der Join-Frist, gemessen.
    /// Ein Takt steht mitten im Schreibschritt (Haken `beim_schreiben` mit Schranke); der
    /// Stoppweg wartet die ECHTE Join-Frist von 2 s ab (`JOIN_FRIST` bleibt Produktkonstante,
    /// F-20) und zaehlt `join_frist_verfehlt`. Nach der Freigabe beendet der Thread den
    /// begonnenen Schreibschritt genau einmal und beginnt keinen neuen Takt, auch nicht fuer
    /// zehn Ausloesungen nach dem Stopp. Gemessen wird an Zaehlern, nicht an der Uhr. Die
    /// Schranke oeffnet spaetestens nach 30 s von selbst und beim Verlassen des Tests, damit
    /// ein roter Verlauf nie haengt.
    #[test]
    fn nach_join_frist_endet_der_begonnene_schreibschritt_genau_einmal() {
        struct Schranke {
            /// (erreicht, offen)
            zustand: Mutex<(bool, bool)>,
            meldung: Condvar,
        }
        impl Schranke {
            fn oeffne(&self) {
                sperre(&self.zustand).1 = true;
                self.meldung.notify_all();
            }
        }
        struct Oeffner(Arc<Schranke>);
        impl Drop for Oeffner {
            fn drop(&mut self) {
                self.0.oeffne();
            }
        }
        const SCHRANKE_FRIST: Duration = Duration::from_secs(30);
        const NAME: &str = "NAK-309 M-66 nach_join_frist_endet_der_begonnene_schreibschritt_genau_einmal";

        let mut b = buehne(true, LANG);
        let kern = b.griff.kern.clone().expect("gestartet");
        let schranke = Arc::new(Schranke { zustand: Mutex::new((false, false)), meldung: Condvar::new() });
        let oeffner = Oeffner(Arc::clone(&schranke));
        let im_haken = Arc::clone(&schranke);
        *sperre(&b.fs.beim_schreiben) = Some(Box::new(move || {
            let beginn = Instant::now();
            let mut z = sperre(&im_haken.zustand);
            z.0 = true;
            im_haken.meldung.notify_all();
            while !z.1 && beginn.elapsed() < SCHRANKE_FRIST {
                let rest = SCHRANKE_FRIST.saturating_sub(beginn.elapsed());
                z = im_haken.meldung.wait_timeout(z, rest).unwrap_or_else(PoisonError::into_inner).0;
            }
        }));
        let k = neue_kennung();
        b.anfrage(&k);
        b.ausloesen();
        let erreicht = {
            let beginn = Instant::now();
            let frist = Duration::from_secs(10);
            let mut z = sperre(&schranke.zustand);
            while !z.0 && beginn.elapsed() < frist {
                let rest = frist.saturating_sub(beginn.elapsed());
                z = schranke.meldung.wait_timeout(z, rest).unwrap_or_else(PoisonError::into_inner).0;
            }
            z.0
        };
        assert!(erreicht, "{NAME} vorbedingung: der Takt erreicht den Schreibschritt nicht");

        let (s0, fertig0) = (b.stand(), sperre(&kern.weck).takte_fertig);
        let beginn = Instant::now();
        b.griff.stoppen();
        let stopp_ms = beginn.elapsed().as_millis();
        let verfehlt = wert(&b.z().join_frist_verfehlt);
        let vor_freigabe = b.antworten(&k).len();
        (0..10).for_each(|_| b.ausloesen());
        schranke.oeffne();
        let _ = b.warte(Duration::from_secs(10), |weck| weck.beendet || weck.takte_fertig > fertig0 + 1);
        let (beendet, fertig) = {
            let weck = sperre(&kern.weck);
            (weck.beendet, weck.takte_fertig.wrapping_sub(fertig0))
        };
        let d = b.stand().seit(s0);
        let antworten = b.antworten(&k).len();
        println!(
            "[roh] {NAME} join_frist_verfehlt={verfehlt} stopp_ms={stopp_ms} antworten_vor_freigabe={vor_freigabe} \
             takte_fertig_seit_stopp={fertig} antworten={antworten} existenzpruefungen_seit_stopp={} beendet={beendet}",
            d.existenz
        );
        assert_eq!(verfehlt, 1, "{NAME}: join_frist_verfehlt {verfehlt} statt 1");
        assert_eq!(vor_freigabe, 0, "{NAME}: Antwort schon vor der Freigabe - der Schreibschritt stand nicht");
        assert_eq!(fertig, 1, "{NAME}: {fertig} Takte fertig nach dem Stopp statt genau des begonnenen");
        assert_eq!(d.existenz, 0, "{NAME}: {} Existenzpruefungen nach dem Stopp - ein neuer Takt hat begonnen", d.existenz);
        let genau_eine = antworten == 1 && wert(&b.z().antworten) == 1 && wert(&b.z().schreibversuche) == 1;
        assert!(genau_eine, "{NAME}: {antworten} Antwortdatei(en) statt genau einer");
        assert!(beendet, "{NAME}: der Thread endet nach dem begonnenen Takt nicht");
        drop(oeffner);
    }

    struct Teil(&'static str, Arc<Mutex<Vec<&'static str>>>);

    impl Drop for Teil {
        fn drop(&mut self) {
            sperre(&self.1).push(self.0);
        }
    }

    #[test]
    fn stoppreihenfolge_briefkasten_zuerst() {
        let protokoll = Arc::new(Mutex::new(Vec::new()));
        let teil = |name| Mutex::new(Some(Teil(name, Arc::clone(&protokoll))));
        crate::lebenslauf::geordnet_stoppen(&teil("briefkasten"), &teil("supervisor"), &teil("v2"), &teil("v3"), &teil("store"));
        let reihenfolge = sperre(&protokoll).clone();
        let soll = ["briefkasten", "supervisor", "v2", "v3", "store"];
        assert_eq!(reihenfolge, soll, "M-45 (3) stoppreihenfolge_briefkasten_zuerst: Protokoll der gemeinsamen Stoppfunktion");
        // Ein echter Griff als erster Teil: sein Thread endet in derselben Funktion.
        let b = buehne(false, LANG);
        let griff = starten_mit(Arc::clone(&b.register), &b.token, b.fassaden.clone(), true);
        let kern = griff.kern.clone().expect("gestartet");
        crate::lebenslauf::geordnet_stoppen(&Mutex::new(Some(griff)), &teil("s"), &teil("2"), &teil("3"), &teil("st"));
        assert!(sperre(&kern.weck).beendet, "M-45 (3) stoppreihenfolge_briefkasten_zuerst: Thread endet im geordneten Stopp");
        // Im Brokerlauf: Start nach beiden Servern; der Stopp ruft die gemeinsame Funktion.
        let quelle = include_str!("lebenslauf.rs");
        let stelle = |text: &str, nadel: &str| text.find(nadel).unwrap_or_else(|| panic!("M-45 (3) {nadel:?} fehlt"));
        let start = &quelle[stelle(quelle, "pub fn broker_starten")..stelle(quelle, "pub fn broker_geordnet_stoppen")];
        let starts = ["server_starten_mit_interventionssenke(", "v3_server_starten_mit_epoch_und_sender(", "briefkasten_starten("];
        let starts = starts.map(|nadel| stelle(start, nadel));
        assert!(starts.windows(2).all(|p| p[0] < p[1]), "M-45 (3) stoppreihenfolge_briefkasten_zuerst: Start {starts:?}");
        let stopp = &quelle[stelle(quelle, "pub fn broker_geordnet_stoppen")..stelle(quelle, "fn entnehmen")];
        let teile = ["        geordnet_stoppen(", "&lauf.briefkasten", "&lauf._supervisor", "&lauf._griff_v2", "&lauf._griff_v3", "&lauf.store"];
        let teile = teile.map(|nadel| stelle(stopp, nadel));
        assert!(teile.windows(2).all(|p| p[0] < p[1]), "M-45 (3) stoppreihenfolge_briefkasten_zuerst: Brokerlauf {teile:?}");
    }

    // ── M-48, M-49: Sicherheit ────────────────────────────────────────────────────────────

    #[test]
    fn pfadtraversal_unmoeglich() {
        let b = buehne(false, LANG);
        let waechter = [b.t.ebene(3).join("waechter"), b.t.ebene(1).join("waechter")];
        waechter.iter().for_each(|ordner| std::fs::create_dir(ordner).expect("Waechter anlegen"));
        let arabisch16 = "\\u0660".repeat(16);
        let faelle = [
            ("..\\..\\x", "..\\\\..\\\\x".to_string()),
            ("C:\\x", "C:\\\\x".to_string()),
            ("/x", "/x".to_string()),
            ("..\\waechter\\ (32 Zeichen)", "..\\\\waechter\\\\0123456789abcdef0123".to_string()),
            ("..\\..\\..\\waechter\\ (32 Zeichen)", "..\\\\..\\\\..\\\\waechter\\\\0123456789abcd".to_string()),
            ("32 Zeichen mit :", "c:0123456789abcdef0123456789abcd".to_string()),
            ("32 Zeichen mit NUL", "\\u0000123456789abcdef0123456789abcdef".to_string()),
            ("Unicode-Ziffern (16 Zeichen, 32 Bytes)", arabisch16.clone()),
            ("Unicode-Ziffern (32 Zeichen)", arabisch16.repeat(2)),
        ];
        for (fall, id) in faelle {
            let s0 = b.stand();
            schreibe_mit_zeit(&b.t.anfrage(), anfrage_text(&id).as_bytes());
            b.takte(5);
            let d = b.stand().seit(s0);
            let waechter_leer = waechter.iter().all(|ordner| leer(ordner));
            let oben_leer = [b.t.ebene(2), b.t.ebene(1), b.t.wurzel.clone()].iter().all(|ordner| dateien_in(ordner).is_empty());
            let sonst_nichts = oben_leer && dateien_in(&b.t.antwort()).is_empty() && dateien_in(&b.t.ebene(3)).len() == 1;
            let ohne = d.anlegen + d.umbenennen == 0 && wert(&b.z().antworten) == 0 && wert(&b.fs.ausserhalb) == 0;
            assert!(waechter_leer && sonst_nichts && ohne, "M-48 pfadtraversal_unmoeglich {fall}: Waechter leer {waechter_leer}, sonst nichts {sonst_nichts}, {d:?}");
        }
    }

    #[test]
    fn reparse_punkte_werden_ignoriert() {
        // (a) antwort\ wird nach dem Start eine Junction auf einen Waechterordner.
        let b = buehne(false, LANG);
        let waechter = b.t.wurzel.join("waechter-antwort");
        std::fs::create_dir(&waechter).expect("Waechter anlegen");
        let link = std::fs::remove_dir(b.t.antwort()).is_ok() && lege_junction_an(&b.t.antwort(), &waechter);
        b.anfrage(&neue_kennung());
        let s0 = b.stand();
        b.takte(5);
        let d = b.stand().seit(s0);
        let ok = link && leer(&waechter) && d.anlegen == 0 && wert(&b.z().antworten) == 0;
        assert!(ok, "M-49 reparse_punkte_werden_ignoriert (a) antwort ist eine junction: Junction {link}, {d:?}");
        // (b) diagnose\ wird nach dem Start eine Junction auf einen Baum mit Anfrage.
        let b = buehne(false, LANG);
        let waechter = b.t.wurzel.join("waechter-diagnose");
        std::fs::create_dir_all(waechter.join("antwort")).expect("Waechter anlegen");
        std::fs::write(waechter.join("anfrage.json"), anfrage_text(&neue_kennung())).expect("Anfrage im Waechter");
        let link = std::fs::remove_dir_all(b.t.ebene(3)).is_ok() && lege_junction_an(&b.t.ebene(3), &waechter);
        let s0 = b.stand();
        b.takte(5);
        let d = b.stand().seit(s0);
        let ok = link && b.t.anfrage().is_file() && leer(&waechter.join("antwort")) && d.lese + d.anlegen == 0;
        assert!(ok && wert(&b.z().antworten) == 0, "M-49 reparse_punkte_werden_ignoriert (b) diagnose ist eine junction: Junction {link}, {d:?}");
        // (c) anfrage.json ist ein Symlink - nur mit dem Recht dazu, sonst ungemessen.
        let b = buehne(false, LANG);
        let ziel = b.t.wurzel.join("waechter-anfrage.json");
        std::fs::write(&ziel, anfrage_text(&neue_kennung())).expect("Ziel schreiben");
        if let Err(fehler) = std::os::windows::fs::symlink_file(&ziel, b.t.anfrage()) {
            println!("[--] M-49 reparse_punkte_werden_ignoriert (c) symlink nicht erzeugbar [{fehler}] - Lage (c) ungemessen");
            return;
        }
        let s0 = b.stand();
        b.takte(5);
        let d = b.stand().seit(s0);
        let ok = d.lese + d.anlegen == 0 && dateien_in(&b.t.antwort()).is_empty() && wert(&b.z().antworten) == 0;
        assert!(ok, "M-49 reparse_punkte_werden_ignoriert (c) anfrage.json ist ein symlink: {d:?}");
    }

    // ── M-51 bis M-53: Zahlenraender ──────────────────────────────────────────────────────

    #[test]
    fn kennungsring_fifo_256() {
        let b = buehne(false, LANG);
        let utc = |pfad: &Path| lies_json(pfad)["erzeugt_utc"].as_str().unwrap_or_default().to_string();
        let mut kennungen = Vec::new();
        for i in 0..257 {
            kennungen.push(neue_kennung());
            b.anfrage(&kennungen[i]);
            b.takte(1);
            b.uhr.ms.fetch_add(1000, Ordering::SeqCst);
            assert_eq!(b.antworten(&kennungen[i]).len(), 1, "M-51 kennungsring_fifo_256 vorbedingung: Kennung {i}");
        }
        let erstes = utc(&b.antworten(&kennungen[0])[0]);
        // Die Buehne raeumt vor jeder Wiederholung ab, zuerst die Anfrage, dann die Antworten
        // samt .tmp- (P-1, T-2), mit eigenen Aufrufen und nie ueber die Fassade.
        let raeume_ab = || {
            std::fs::remove_file(b.t.anfrage()).expect("Anfrage abraeumen");
            dateien_in(&b.t.antwort()).iter().for_each(|datei| std::fs::remove_file(datei).expect("Antwort abraeumen"));
            b.takte(1);
            b.uhr.ms.fetch_add(60_000, Ordering::SeqCst);
        };
        raeume_ab();
        b.anfrage(&kennungen[255]); // die zweitjuengste
        let s0 = b.stand();
        b.takte(3);
        let d = b.stand().seit(s0);
        let ok = d.anlegen + d.schreiben == 0 && dateien_in(&b.t.antwort()).is_empty();
        assert!(ok, "M-51 kennungsring_fifo_256 zweitjuengste erneut 0 anlegeversuche: {d:?}");
        raeume_ab();
        b.anfrage(&kennungen[0]); // die aelteste, verdraengt
        let s0 = b.stand();
        b.takte(1);
        let (d, neu) = (b.stand().seit(s0), b.antworten(&kennungen[0]));
        let neues = neu.first().map(|pfad| utc(pfad)).unwrap_or_default();
        let ok = d.anlegen == 1 && d.schreiben == 1 && neu.len() == 1 && neues != erstes && wert(&b.z().fehlerzaehler) == 0;
        assert!(ok, "M-51 kennungsring_fifo_256 aelteste verdraengt genau ein anlegen: {d:?}, erzeugt_utc {erstes} -> {neues}");
        // Nach dem Anlauf legen 100 Takte ohne Schreiben keinen Speicher an; geloescht wird nie.
        let vorher = wert(&ALLOKATIONEN);
        ZAEHLT.with(|zaehlt| zaehlt.set(true));
        b.takte(100);
        ZAEHLT.with(|zaehlt| zaehlt.set(false));
        assert_eq!(wert(&ALLOKATIONEN) - vorher, 0, "M-51 kennungsring_fifo_256 keine allokation je takt (100 Takte)");
        assert_eq!(b.stand().loeschen, 0, "M-51 kennungsring_fifo_256 loeschzaehler der instanz 0");
    }

    #[test]
    fn uhrsprung_aendert_zuordnung_nicht() {
        let b = buehne(false, LANG);
        let utc = |kennung: &str| b.antworten(kennung).first().map(|pfad| lies_json(pfad)["erzeugt_utc"].to_string());
        let (k, l) = (neue_kennung(), neue_kennung());
        b.anfrage(&k);
        b.takte(1);
        b.uhr.ms.fetch_sub(2 * 3600 * 1000, Ordering::SeqCst); // die Uhr springt 2 h zurueck
        b.anfrage(&l);
        b.takte(1);
        let (utc_k, utc_l) = (utc(&k), utc(&l));
        let ok = utc_k.as_deref() == Some("\"2026-09-14T12:00:00.000Z\"") && utc_l.as_deref() == Some("\"2026-09-14T10:00:00.000Z\"");
        assert!(ok && wert(&b.z().antworten) == 2, "M-52 uhrsprung_aendert_zuordnung_nicht: K {utc_k:?}, L {utc_l:?}");
    }

    #[test]
    fn keine_leere_antwort() {
        for (lage, vor_dem_ersten_byte) in [("(a) fehler vor dem ersten byte", true), ("(b) fehler beim umbenennen", false)] {
            let b = buehne(false, LANG);
            let injektion = if vor_dem_ersten_byte { &b.fs.injektion.schreiben } else { &b.fs.injektion.umbenennen };
            injektion.store(1, Ordering::SeqCst);
            b.anfrage(&neue_kennung());
            b.takte(5);
            let dateien = dateien_in(&b.t.antwort());
            let endung = format!(".json.tmp-{}", std::process::id());
            let temp: Vec<&PathBuf> = dateien.iter().filter(|pfad| name(pfad).ends_with(&endung)).collect();
            let json = dateien.iter().filter(|pfad| name(pfad).ends_with(".json")).count();
            let bytes = temp.first().and_then(|pfad| std::fs::metadata(pfad).ok()).map(|meta| meta.len());
            let groesse = if vor_dem_ersten_byte { bytes == Some(0) } else { bytes.is_some_and(|n| n > 0) };
            let (loeschen, fehler, anzahl) = (b.stand().loeschen, wert(&b.z().fehlerzaehler), dateien.len());
            let ok = temp.len() == 1 && json == 0 && anzahl == 1 && loeschen == 0 && fehler == 1 && groesse;
            assert!(ok, "M-53 keine_leere_antwort {lage}: Temp {} ({bytes:?} Bytes), json {json}, Dateien {anzahl}, Loesch {loeschen}, Fehler {fehler}", temp.len());
        }
    }
}
