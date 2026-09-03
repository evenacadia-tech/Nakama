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

/// G2-LOSSYSTR-001, Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): die
/// EINE Stelle, an der UTF-16 aus einer Win32-API zu einem Pfad wird.
///
/// `String::from_utf16_lossy` ersetzt ungepaarte Surrogate durch U+FFFD - der
/// geprueft Pfad waere dann ein anderer als der geoeffnete. `OsString::from_wide`
/// behaelt die UTF-16-Einheiten unveraendert. Der Pfad bleibt von hier bis zum
/// Oeffnen ein `PathBuf`; lossy gewandelt wird nur fuer die ANZEIGE in
/// Fehlertexten (`Path::display`).
///
/// Der Test speist ein ungepaartes Surrogat ein und vergleicht byteweise ueber
/// `OsStrExt::encode_wide`; ohne diese Funktion waere die Wandlung nur ueber
/// einen echten LocalAppData-Pfad beobachtbar, und der ist wohlgeformt.
#[cfg(windows)]
pub fn pfad_aus_utf16(einheiten: &[u16]) -> PathBuf {
    use std::os::windows::ffi::OsStringExt;
    PathBuf::from(std::ffi::OsString::from_wide(einheiten))
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
        // G2-LOSSYSTR-001: verlustfrei wandeln, ueber die EINE gemeinsame
        // Stelle (`pfad_aus_utf16`).
        let pfad = pfad_aus_utf16(std::slice::from_raw_parts(roh, len));
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

/// G2-TOCTOU-002: Traegt eine existierende Komponente das Reparse-Attribut?
///
/// Ein Reparse-Punkt (Junction, Symlink, Mount Point) laesst denselben Namen
/// auf ein anderes Objekt zeigen, als der Aufrufer meint - und zwar zwischen
/// Pruefung und Oeffnen umlenkbar. Der Store weist ihn deshalb ab, statt ihn
/// zu klassifizieren. Gemessen wird mit `GetFileAttributesW`, das dem Reparse-
/// Punkt selbst folgt und nicht seinem Ziel.
#[cfg(windows)]
fn traegt_reparse_punkt(pfad: &Path) -> Result<bool, StoreFehler> {
    use std::os::windows::ffi::OsStrExt;
    use windows_sys::Win32::Storage::FileSystem::{
        GetFileAttributesW, FILE_ATTRIBUTE_REPARSE_POINT, INVALID_FILE_ATTRIBUTES,
    };

    let mut wide: Vec<u16> = pfad.as_os_str().encode_wide().collect();
    wide.push(0);
    // SAFETY: der Puffer ist nullterminiert; die API liest nur.
    let attribute = unsafe { GetFileAttributesW(wide.as_ptr()) };
    if attribute == INVALID_FILE_ATTRIBUTES {
        return Err(StoreFehler::Pfad(format!(
            "Attribute von {} nicht lesbar",
            pfad.display()
        )));
    }
    Ok(attribute & FILE_ATTRIBUTE_REPARSE_POINT != 0)
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

    // G2-TOCTOU-002: JEDE existierende Komponente vom Kandidaten aufwaerts wird
    // auf das Reparse-Attribut geprueft, bevor irgendetwas klassifiziert wird.
    // Vorher klassifizierte die Funktion einen VORFAHREN und oeffnete danach
    // ein anderes Objekt - genau die Luecke, durch die eine untergeschobene
    // Junction die Remote-Abweisung umging.
    let mut pruefling = Some(kandidat);
    while let Some(teil) = pruefling {
        if traegt_reparse_punkt(teil)? {
            return Err(StoreFehler::Pfad(format!(
                "Reparse-Punkt im Storepfad: {}",
                teil.display()
            )));
        }
        pruefling = teil.parent().filter(|eltern| !eltern.as_os_str().is_empty());
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

/// G2-TOCTOU-002: die Klassifikation ueber ein HANDLE, das der Aufrufer haelt.
///
/// Der Kernel nennt den endgueltigen Pfad des Objekts hinter dem Handle
/// (`GetFinalPathNameByHandleW`); erst dieser Pfad wird zum Volume gefaltet und
/// klassifiziert. Zwischen dem Oeffnen des Objekts und dieser Frage kann kein
/// Name mehr umgelenkt werden - das Handle haelt genau das Objekt fest.
///
/// Das Handle bleibt dem Aufrufer: diese Funktion schliesst es NICHT. Beim
/// SQLite-Weg gehoert es der Datenbankverbindung.
#[cfg(windows)]
pub(crate) fn volume_am_handle(
    griff: windows_sys::Win32::Foundation::HANDLE,
    wofuer: &str,
) -> Result<(PathBuf, bool), StoreFehler> {
    use std::os::windows::ffi::OsStringExt;
    use windows_sys::Win32::Storage::FileSystem::{
        GetDriveTypeW, GetFinalPathNameByHandleW, GetVolumePathNameW, FILE_NAME_NORMALIZED,
        VOLUME_NAME_DOS,
    };
    use windows_sys::Win32::System::WindowsProgramming::DRIVE_REMOTE;

    let mut endgueltig = vec![0u16; 32768];
    // SAFETY: `griff` ist ein gueltiges, vom Aufrufer gehaltenes Dateihandle;
    // der Puffer ist gross genug und seine Laenge wird korrekt uebergeben.
    let laenge = unsafe {
        GetFinalPathNameByHandleW(
            griff,
            endgueltig.as_mut_ptr(),
            endgueltig.len() as u32,
            FILE_NAME_NORMALIZED | VOLUME_NAME_DOS,
        )
    };
    if laenge == 0 || laenge as usize >= endgueltig.len() {
        return Err(StoreFehler::Pfad(format!(
            "endgueltiger Pfad von {wofuer} nicht bestimmbar"
        )));
    }
    endgueltig.truncate(laenge as usize);
    let endgueltig_pfad = PathBuf::from(std::ffi::OsString::from_wide(&endgueltig));

    // `GetDriveTypeW` will einen Wurzelpfad ohne erweiterte Praefixe. Der
    // UNC-Fall wird auf die gewohnte Form zurueckgefaltet, damit er als
    // DRIVE_REMOTE erkannt wird.
    //
    // G2-LOSSYSTR-001 gilt auch hier: die Faltung laeuft auf den
    // UTF-16-Einheiten, nicht ueber einen `to_string_lossy`-Umweg. Ein
    // ungepaartes Surrogat im endgueltigen Pfad wuerde sonst zu U+FFFD und der
    // klassifizierte Pfad waere ein anderer als der geoeffnete.
    let unc: Vec<u16> = r"\\?\UNC\".encode_utf16().collect();
    let erweitert: Vec<u16> = r"\\?\".encode_utf16().collect();
    let mut wide_final: Vec<u16> = if endgueltig.starts_with(&unc) {
        let mut gefaltet: Vec<u16> = r"\\".encode_utf16().collect();
        gefaltet.extend_from_slice(&endgueltig[unc.len()..]);
        gefaltet
    } else if endgueltig.starts_with(&erweitert) {
        endgueltig[erweitert.len()..].to_vec()
    } else {
        endgueltig.clone()
    };
    wide_final.push(0);
    let mut volume = vec![0u16; 32768];
    // SAFETY: beide Puffer sind nullterminiert bzw. ausreichend gross.
    let ok = unsafe {
        GetVolumePathNameW(wide_final.as_ptr(), volume.as_mut_ptr(), volume.len() as u32)
    };
    if ok == 0 {
        return Err(StoreFehler::Pfad(format!(
            "Volume des geoeffneten Objekts {} nicht bestimmbar",
            endgueltig_pfad.display()
        )));
    }
    // SAFETY: `volume` enthaelt nach Erfolg einen nullterminierten Rootpfad.
    let remote = unsafe { GetDriveTypeW(volume.as_ptr()) } == DRIVE_REMOTE;
    Ok((endgueltig_pfad, remote))
}

/// G2-TOCTOU-002, Nacharbeit Runde 2 (R2-3, 03.09.2026): DIE
/// Volumenentscheidung faellt auf dem Handle, das SQLITE SELBST haelt.
///
/// Nacharbeit Runde 1 hatte die Entscheidung zwar auf ein geoeffnetes Objekt
/// verlegt - aber auf ein ZWEITES, das `geoeffnete_db_volume` per Namen
/// oeffnete. `Connection::open_with_flags` oeffnet zuerst die eigentliche
/// Datenbank, das zweite `CreateFileW` denselben Namen; wird der Pfad oder eine
/// Junction dazwischen umgehaengt, haelt SQLite Objekt A, waehrend die
/// Volumenentscheidung Objekt B klassifiziert. Der urspruengliche TOCTOU blieb
/// damit bestehen.
///
/// SQLite gibt das Handle seiner Hauptdatenbank ueber
/// `SQLITE_FCNTL_WIN32_GET_HANDLE` heraus. Darauf laufen dieselben Schritte wie
/// bisher - es gibt kein zweites Oeffnen und damit kein Fenster mehr.
/// `geoeffnete_db_volume` bleibt Vorpruefung und Testhelfer, ist aber nicht
/// mehr die Entscheidung.
#[cfg(windows)]
pub fn volume_am_sqlite_handle(
    conn: &rusqlite::Connection,
) -> Result<(PathBuf, bool), StoreFehler> {
    use windows_sys::Win32::Foundation::{HANDLE, INVALID_HANDLE_VALUE};

    let mut griff: HANDLE = std::ptr::null_mut();
    // `SQLITE_FCNTL_WIN32_GET_HANDLE` deutet sein Argument als `HANDLE*` -
    // genau der Typ, dessen Adresse hier uebergeben wird. Geschrieben wird nur
    // in `griff`; das Handle bleibt Eigentum von SQLite und wird hier NICHT
    // geschlossen.
    //
    // SAFETY: `conn` lebt fuer die Dauer des Aufrufs, `handle()` liefert daher
    // einen gueltigen `sqlite3*`; der Name "main" ist nullterminiert und der
    // Ausgabezeiger zeigt auf ein lebendes `HANDLE` auf dem Stack.
    let rc = unsafe {
        rusqlite::ffi::sqlite3_file_control(
            conn.handle(),
            c"main".as_ptr(),
            rusqlite::ffi::SQLITE_FCNTL_WIN32_GET_HANDLE,
            &mut griff as *mut HANDLE as *mut std::ffi::c_void,
        )
    };
    if rc != rusqlite::ffi::SQLITE_OK {
        return Err(StoreFehler::Pfad(format!(
            "SQLite gab das Handle seiner Hauptdatenbank nicht heraus (rc={rc})"
        )));
    }
    if griff.is_null() || griff == INVALID_HANDLE_VALUE {
        return Err(StoreFehler::Pfad(
            "SQLite meldete kein gueltiges Handle fuer die Hauptdatenbank".into(),
        ));
    }
    volume_am_handle(griff, "der geoeffneten SQLite-Hauptdatenbank")
}

#[cfg(not(windows))]
pub fn volume_am_sqlite_handle(
    conn: &rusqlite::Connection,
) -> Result<(PathBuf, bool), StoreFehler> {
    Ok((PathBuf::from(conn.path().unwrap_or_default()), false))
}

/// G2-TOCTOU-002, Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): die
/// Klassifikation eines Namens ueber ein eigens geoeffnetes Objekt.
///
/// `store_pfad_ist_remote` sucht per `exists()` den naechsten vorhandenen
/// VORFAHREN und klassifiziert dessen Pfadnamen; beim ersten Start mit
/// fehlenden Komponenten urteilt sie damit ueber ein anderes Objekt. Diese
/// Funktion oeffnet die Datei selbst und klassifiziert IHR Objekt.
///
/// Seit R2-3 (Nacharbeit Runde 2, 03.09.2026) ist sie NICHT mehr die
/// Entscheidung des Stores: dafuer gilt `volume_am_sqlite_handle` auf dem
/// Handle, das SQLite haelt. Ein zweites `CreateFileW` nach demselben Namen
/// kann ein anderes Objekt treffen als das, in dem die Datenbank liegt.
///
/// Rueckgabe: der aufgeloeste Pfad des geoeffneten Objekts und ob er auf einem
/// Netzwerkvolume liegt.
#[cfg(windows)]
pub fn geoeffnete_db_volume(pfad: &Path) -> Result<(PathBuf, bool), StoreFehler> {
    use std::os::windows::ffi::OsStrExt;
    use windows_sys::Win32::Foundation::{CloseHandle, INVALID_HANDLE_VALUE};
    use windows_sys::Win32::Storage::FileSystem::{
        CreateFileW, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_DELETE, FILE_SHARE_READ, FILE_SHARE_WRITE,
        OPEN_EXISTING,
    };

    let mut wide: Vec<u16> = pfad.as_os_str().encode_wide().collect();
    wide.push(0);
    // Zugriffsmaske 0: es wird nichts gelesen und nichts geschrieben, nur die
    // Identitaet des Objekts abgefragt. Alle drei Share-Modi, damit SQLite
    // parallel damit arbeiten kann.
    // SAFETY: der Puffer ist nullterminiert; alle Zeiger sind gueltig oder null.
    let griff = unsafe {
        CreateFileW(
            wide.as_ptr(),
            0,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            std::ptr::null(),
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            std::ptr::null_mut(),
        )
    };
    if griff == INVALID_HANDLE_VALUE {
        return Err(StoreFehler::Pfad(format!(
            "{} konnte zur Volumenpruefung nicht geoeffnet werden",
            pfad.display()
        )));
    }
    let ausgang = volume_am_handle(griff, &pfad.display().to_string());
    // SAFETY: das Handle stammt aus dem erfolgreichen CreateFileW darueber und
    // wird genau einmal geschlossen.
    unsafe { CloseHandle(griff) };
    ausgang
}

#[cfg(not(windows))]
pub fn geoeffnete_db_volume(pfad: &Path) -> Result<(PathBuf, bool), StoreFehler> {
    if !pfad.exists() {
        return Err(StoreFehler::Pfad(format!(
            "{} existiert nicht",
            pfad.display()
        )));
    }
    Ok((pfad.to_path_buf(), false))
}
