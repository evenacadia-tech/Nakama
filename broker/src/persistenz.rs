//! Gemeinsame, haltbare Datei-Veröffentlichung für Broker-Zustände.
//!
//! Die temporäre Datei liegt immer im Zielordner, wird exklusiv angelegt und
//! vor der Veröffentlichung auf den Datenträger gespült. Bindungen dürfen ein
//! bestehendes Ziel atomar ersetzen; Snapshots verwenden bewusst die
//! No-Replace-Variante, damit auch konkurrierende Schreiber nichts verlieren.

use std::ffi::OsString;
use std::fs::{File, OpenOptions};
use std::io::{self, Write};
use std::path::{Path, PathBuf};

const TEMP_VERSUCHE: usize = 32;

struct TempPfad {
    pfad: PathBuf,
}

impl Drop for TempPfad {
    fn drop(&mut self) {
        let _ = std::fs::remove_file(&self.pfad);
    }
}

fn elternordner(ziel: &Path) -> &Path {
    ziel.parent()
        .filter(|p| !p.as_os_str().is_empty())
        .unwrap_or_else(|| Path::new("."))
}

fn temp_anlegen(ziel: &Path) -> Result<(TempPfad, File), String> {
    let eltern = elternordner(ziel);
    std::fs::create_dir_all(eltern).map_err(|e| format!("{} anlegen: {e}", eltern.display()))?;
    let dateiname = ziel
        .file_name()
        .ok_or_else(|| format!("{} hat keinen Dateinamen", ziel.display()))?;

    for _ in 0..TEMP_VERSUCHE {
        let mut temp_name = OsString::from(".");
        temp_name.push(dateiname);
        temp_name.push(format!(
            ".tmp-{}-{}",
            std::process::id(),
            uuid::Uuid::new_v4().simple()
        ));
        let pfad = eltern.join(temp_name);
        match OpenOptions::new().write(true).create_new(true).open(&pfad) {
            Ok(datei) => return Ok((TempPfad { pfad }, datei)),
            Err(e) if e.kind() == io::ErrorKind::AlreadyExists => continue,
            Err(e) => return Err(format!("{} exklusiv anlegen: {e}", pfad.display())),
        }
    }

    Err(format!(
        "kein exklusiver temporärer Pfad für {} nach {TEMP_VERSUCHE} Versuchen",
        ziel.display()
    ))
}

fn temp_schreiben(ziel: &Path, daten: &[u8]) -> Result<TempPfad, String> {
    let (temp, mut datei) = temp_anlegen(ziel)?;
    datei
        .write_all(daten)
        .map_err(|e| format!("{} schreiben: {e}", temp.pfad.display()))?;
    datei
        .sync_all()
        .map_err(|e| format!("{} synchronisieren: {e}", temp.pfad.display()))?;
    drop(datei);
    Ok(temp)
}

fn ist_bereits_vorhanden(fehler: &io::Error) -> bool {
    fehler.kind() == io::ErrorKind::AlreadyExists
        || cfg!(windows) && matches!(fehler.raw_os_error(), Some(80 | 183))
}

#[cfg(windows)]
fn verschieben(temp: &Path, ziel: &Path, ersetzen: bool) -> io::Result<()> {
    use std::os::windows::ffi::OsStrExt;
    use windows_sys::Win32::Storage::FileSystem::{
        MoveFileExW, MOVEFILE_REPLACE_EXISTING, MOVEFILE_WRITE_THROUGH,
    };

    fn weit(pfad: &Path) -> io::Result<Vec<u16>> {
        let mut wert: Vec<u16> = pfad.as_os_str().encode_wide().collect();
        if wert.contains(&0) {
            return Err(io::Error::new(
                io::ErrorKind::InvalidInput,
                "Pfad enthält NUL",
            ));
        }
        wert.push(0);
        Ok(wert)
    }

    let temp = weit(temp)?;
    let ziel = weit(ziel)?;
    let mut flags = MOVEFILE_WRITE_THROUGH;
    if ersetzen {
        flags |= MOVEFILE_REPLACE_EXISTING;
    }
    // SAFETY: Beide UTF-16-Puffer sind NUL-terminiert und leben bis nach dem
    // synchronen Win32-Aufruf. Die Datei liegt im selben Ordner/Volume.
    if unsafe { MoveFileExW(temp.as_ptr(), ziel.as_ptr(), flags) } == 0 {
        Err(io::Error::last_os_error())
    } else {
        Ok(())
    }
}

#[cfg(windows)]
fn atomar_ersetzen_windows(temp: &Path, ziel: &Path) -> io::Result<()> {
    const VERSUCHE: usize = 64;
    for versuch in 0..VERSUCHE {
        match verschieben(temp, ziel, true) {
            Ok(()) => return Ok(()),
            // Zwei gleichzeitige Replace-Aufrufe können unter Windows kurz
            // ERROR_ACCESS_DENIED/SHARING_VIOLATION sehen, obwohl kein Handle
            // dauerhaft offen ist. Nur diese klar vorübergehenden Fehler
            // werden begrenzt erneut versucht.
            Err(e) if versuch + 1 < VERSUCHE && matches!(e.raw_os_error(), Some(5 | 32 | 33)) => {
                std::thread::sleep(std::time::Duration::from_millis(1));
            }
            Err(e) => return Err(e),
        }
    }
    unreachable!("die begrenzte Schleife liefert im letzten Versuch zurück")
}

#[cfg(not(windows))]
fn ordner_synchronisieren(ziel: &Path) -> io::Result<()> {
    File::open(elternordner(ziel))?.sync_all()
}

/// Veröffentlicht `daten` haltbar und ersetzt einen vorhandenen Zielstand
/// atomar. Das ist der richtige Modus für den einen aktuellen Bindungsstand.
pub(crate) fn atomar_ersetzen(ziel: &Path, daten: &[u8]) -> Result<(), String> {
    let temp = temp_schreiben(ziel, daten)?;

    #[cfg(windows)]
    atomar_ersetzen_windows(&temp.pfad, ziel)
        .map_err(|e| format!("{} atomar ersetzen: {e}", ziel.display()))?;

    #[cfg(not(windows))]
    {
        std::fs::rename(&temp.pfad, ziel)
            .map_err(|e| format!("{} atomar ersetzen: {e}", ziel.display()))?;
        ordner_synchronisieren(ziel)
            .map_err(|e| format!("Ordner von {} synchronisieren: {e}", ziel.display()))?;
    }

    Ok(())
}

/// Veröffentlicht `daten` nur dann, wenn `ziel` noch nicht existiert.
/// `Ok(false)` ist eine saubere Namenskollision; kein bestehendes Byte wird
/// dabei ersetzt.
pub(crate) fn atomar_neu(ziel: &Path, daten: &[u8]) -> Result<bool, String> {
    let temp = temp_schreiben(ziel, daten)?;

    #[cfg(windows)]
    match verschieben(&temp.pfad, ziel, false) {
        Ok(()) => Ok(true),
        Err(e) if ist_bereits_vorhanden(&e) => Ok(false),
        Err(e) => Err(format!("{} atomar veröffentlichen: {e}", ziel.display())),
    }

    #[cfg(not(windows))]
    {
        match std::fs::hard_link(&temp.pfad, ziel) {
            Ok(()) => {
                std::fs::remove_file(&temp.pfad).map_err(|e| {
                    format!(
                        "{} nach Veröffentlichung entfernen: {e}",
                        temp.pfad.display()
                    )
                })?;
                ordner_synchronisieren(ziel)
                    .map_err(|e| format!("Ordner von {} synchronisieren: {e}", ziel.display()))?;
                Ok(true)
            }
            Err(e) if ist_bereits_vorhanden(&e) => Ok(false),
            Err(e) => Err(format!("{} atomar veröffentlichen: {e}", ziel.display())),
        }
    }
}
