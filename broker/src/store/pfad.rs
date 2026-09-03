//! Wo die Datenbank liegen darf.
//!
//! Der Standardort haengt am bekannten Ordner des Benutzers; ein Netzwerkvolume
//! ist kein zulaessiger Ort fuer die lokale Wahrheit.

use super::{StoreFehler, STORE_DATEINAME, STORE_RELATIVPFAD};
use std::path::{Path, PathBuf};

pub(super) fn wal_pfad(db_pfad: &Path) -> PathBuf {
    let mut roh = db_pfad.as_os_str().to_os_string();
    roh.push("-wal");
    PathBuf::from(roh)
}

pub(super) fn wal_groesse(wal_pfad: &Path) -> u64 {
    std::fs::metadata(wal_pfad).map(|m| m.len()).unwrap_or(0)
}

pub fn store_pfad_unter(local_app_data: &Path) -> PathBuf {
    local_app_data.join(STORE_RELATIVPFAD).join(STORE_DATEINAME)
}

#[cfg(windows)]
pub fn standard_store_pfad() -> Result<PathBuf, StoreFehler> {
    use windows_sys::Win32::Foundation::HANDLE;
    use windows_sys::Win32::System::Com::CoTaskMemFree;
    use windows_sys::Win32::UI::Shell::{
        FOLDERID_LocalAppData, SHGetKnownFolderPath, KF_FLAG_DEFAULT,
    };

    let mut roh: *mut u16 = std::ptr::null_mut();
    // SAFETY: die API schreibt bei Erfolg einen CoTaskMem-String nach `roh`;
    // er wird nach der Kopie genau einmal freigegeben.
    let hr = unsafe {
        SHGetKnownFolderPath(
            &FOLDERID_LocalAppData,
            KF_FLAG_DEFAULT as u32,
            std::ptr::null_mut::<core::ffi::c_void>() as HANDLE,
            &mut roh,
        )
    };
    if hr < 0 || roh.is_null() {
        return Err(StoreFehler::Pfad(format!(
            "FOLDERID_LocalAppData konnte nicht aufgeloest werden (HRESULT {hr:#x})"
        )));
    }
    // SAFETY: `roh` ist ein nullterminierter UTF-16-String aus dem gerade
    // erfolgreichen Aufruf und bleibt bis `CoTaskMemFree` gueltig.
    let root = unsafe {
        let mut len = 0usize;
        while *roh.add(len) != 0 {
            len += 1;
        }
        let pfad = PathBuf::from(String::from_utf16_lossy(std::slice::from_raw_parts(
            roh, len,
        )));
        CoTaskMemFree(roh.cast());
        pfad
    };
    Ok(store_pfad_unter(&root))
}

#[cfg(not(windows))]
pub fn standard_store_pfad() -> Result<PathBuf, StoreFehler> {
    let root = std::env::var_os("LOCALAPPDATA")
        .map(PathBuf::from)
        .ok_or_else(|| {
            StoreFehler::Pfad("FOLDERID_LocalAppData ist nur auf Windows verfuegbar".into())
        })?;
    Ok(store_pfad_unter(&root))
}

#[cfg(windows)]
pub fn store_pfad_ist_remote(pfad: &Path) -> Result<bool, StoreFehler> {
    use std::os::windows::ffi::OsStrExt;
    use windows_sys::Win32::Storage::FileSystem::{GetDriveTypeW, GetVolumePathNameW};
    use windows_sys::Win32::System::WindowsProgramming::DRIVE_REMOTE;

    let mut kandidat = pfad;
    while !kandidat.exists() {
        kandidat = kandidat.parent().ok_or_else(|| {
            StoreFehler::Pfad(format!(
                "kein existierender Vorfahr fuer {}",
                pfad.display()
            ))
        })?;
    }
    let mut wide: Vec<u16> = kandidat.as_os_str().encode_wide().collect();
    wide.push(0);
    let mut volume = vec![0u16; 32768];
    // SAFETY: beide Puffer sind nullterminiert bzw. ausreichend gross; die API
    // schreibt hoechstens die angegebene Zahl UTF-16-Codeunits.
    let ok = unsafe { GetVolumePathNameW(wide.as_ptr(), volume.as_mut_ptr(), volume.len() as u32) };
    if ok == 0 {
        return Err(StoreFehler::Pfad(format!(
            "Volume fuer {} konnte nicht bestimmt werden",
            kandidat.display()
        )));
    }
    // SAFETY: `volume` enthaelt nach Erfolg einen nullterminierten Rootpfad.
    Ok(unsafe { GetDriveTypeW(volume.as_ptr()) } == DRIVE_REMOTE)
}

#[cfg(not(windows))]
pub fn store_pfad_ist_remote(_pfad: &Path) -> Result<bool, StoreFehler> {
    Ok(false)
}
