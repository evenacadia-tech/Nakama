//! Das Authentisierungstor.
//!
//! Hier wird aus einer Pipe-Verbindung eine bekannte SID. Jeder unsafe-Block
//! traegt seinen Sicherheitskommentar, weil die Soundness des ganzen Tors an
//! diesen wenigen Zeilen haengt.

use super::*;

pub(super) const _: () = assert!(std::mem::align_of::<TOKEN_USER>() <= std::mem::align_of::<u64>());

pub(super) struct VerbindungsSicherheitsCleanup {
    pub(super) revertiert: bool,
    pub(super) auth_fehler: V3AuthTestFehler,
    pub(super) spur: Arc<SicherheitsSpur>,
}

impl VerbindungsSicherheitsCleanup {
    pub(super) fn spur(&self, schritt: &'static str) {
        self.spur.push(schritt);
    }

    pub(super) fn revertieren(&mut self) {
        if self.revertiert {
            return;
        }
        self.spur("revert");
        // SAFETY: RevertToSelf wirkt nur auf das Token DIESES Threads und
        // nimmt kein Argument; schlaegt es fehl, bricht der Prozess unten ab,
        // statt unter fremdem Token weiterzulaufen.
        if self.auth_fehler == V3AuthTestFehler::Revert || unsafe { RevertToSelf() } == 0 {
            // Unter einem moeglicherweise fremden Threadtoken darf weder ein
            // normaler Drop noch irgendeine Brokerfachlogik weiterlaufen.
            std::process::abort();
        }
        self.revertiert = true;
    }
}

impl Drop for VerbindungsSicherheitsCleanup {
    fn drop(&mut self) {
        self.revertieren();
    }
}

pub(super) fn client_sid_authentisieren(
    h: HANDLE,
    sicherheit: &crate::server::Sicherheit,
    cleanup: &mut VerbindungsSicherheitsCleanup,
) -> bool {
    cleanup.spur("impersonate");
    // SAFETY: `h` ist das gueltige Pipe-Handle dieser Verbindung; der Cleanup
    // revertiert das Threadtoken in JEDEM Ausgang, auch bei Panik.
    if cleanup.auth_fehler == V3AuthTestFehler::Impersonate
        || unsafe { ImpersonateNamedPipeClient(h) } == 0
    {
        return false;
    }

    let mut token_roh: HANDLE = std::ptr::null_mut();
    // SAFETY: `token_roh` ist eine lebende lokale Variable; bei Erfolg
    // uebernimmt der TokenGriff darunter den Besitz und schliesst genau einmal.
    if cleanup.auth_fehler == V3AuthTestFehler::OpenThreadToken
        || unsafe { OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, 0, &mut token_roh) } == 0
    {
        return false;
    }
    let token = TokenGriff(token_roh);

    let mut noetig = 0u32;
    // SAFETY: Groessenabfrage nach Win32-Konvention - Nullpuffer und Laenge 0
    // sind hier ausdruecklich erlaubt; `noetig` ist eine lebende Lokale.
    unsafe {
        GetTokenInformation(token.0, TokenUser, std::ptr::null_mut(), 0, &mut noetig);
    }
    // SAFETY: reine Abfrage des threadlokalen Fehlercodes direkt danach.
    let groessen_fehler = unsafe { GetLastError() };
    if cleanup.auth_fehler == V3AuthTestFehler::TokenGroesse
        || groessen_fehler != ERROR_INSUFFICIENT_BUFFER
        || noetig < std::mem::size_of::<TOKEN_USER>() as u32
    {
        return false;
    }
    let mut puffer = vec![0u64; (noetig as usize).div_ceil(std::mem::size_of::<u64>())];
    if cleanup.auth_fehler == V3AuthTestFehler::TokenInformation
        // SAFETY: der Puffer ist u64-ausgerichtet und mindestens `noetig` Bytes
        // gross - die Vec-Laenge ist genau darauf aufgerundet, und die
        // Ausrichtung reicht fuer TOKEN_USER (statisch zugesichert in mod.rs).
        || unsafe {
            GetTokenInformation(
                token.0,
                TokenUser,
                puffer.as_mut_ptr().cast(),
                noetig,
                &mut noetig,
            )
        } == 0
    {
        return false;
    }
    // SAFETY: der Aufruf oben hat den Puffer mit einer TOKEN_USER-Struktur
    // gefuellt; die Groessenpruefung davor schliesst einen zu kleinen Puffer
    // aus, und `puffer` lebt bis zum Ende dieser Funktion.
    let sid = unsafe { (*puffer.as_ptr().cast::<TOKEN_USER>()).User.Sid };
    cleanup.spur("sid");
    // SAFETY: `sid` zeigt in den noch lebenden Puffer; IsValidSid liest nur und
    // ist die Pruefung, die alle folgenden SID-Aufrufe voraussetzen.
    if cleanup.auth_fehler == V3AuthTestFehler::SidUngueltig || unsafe { IsValidSid(sid) } == 0 {
        return false;
    }

    // B-04 testet keinen erfundenen Bool-Rueckgabewert, sondern einen echten
    // `EqualSid`-Vergleich gegen eine andere, weiterhin gueltige SID. Dazu
    // kopiert die Testnaht die erwartete User-SID und aendert ausschliesslich
    // den letzten SubAuthority-Wert. Der Produktpfad vergleicht unveraendert
    // gegen genau die SID, aus der auch die Pipe-DACL gebaut wurde.
    let mut fremde_sid_speicher = Vec::<u64>::new();
    let erwartete_sid = if cleanup.auth_fehler == V3AuthTestFehler::SidFremd {
        // SAFETY: `user_sid()` liefert die im Sicherheitsobjekt gehaltene,
        // beim Start geprueft gueltige SID-Kopie; sie lebt laenger als dieser Aufruf.
        let laenge = unsafe { GetLengthSid(sicherheit.user_sid()) };
        if laenge < 12 {
            return false;
        }
        fremde_sid_speicher.resize((laenge as usize).div_ceil(std::mem::size_of::<u64>()), 0);
        // SAFETY: das Ziel ist gerade auf mindestens `laenge` Bytes gebracht
        // und u64-ausgerichtet; die Quelle ist die gueltige User-SID.
        if unsafe {
            CopySid(
                laenge,
                fremde_sid_speicher.as_mut_ptr().cast(),
                sicherheit.user_sid(),
            )
        } == 0
        {
            return false;
        }
        let sid_bytes = fremde_sid_speicher.as_mut_ptr().cast::<u8>();
        // SAFETY: Byte 1 einer SID ist ihr SubAuthorityCount; der Puffer ist
        // mindestens 12 Bytes gross (Pruefung oben) und lebt hier noch.
        let anzahl = unsafe { *sid_bytes.add(1) } as usize;
        if anzahl == 0 || 8 + anzahl * 4 > laenge as usize {
            return false;
        }
        // SAFETY: die Zeile darueber hat gerechnet, dass dieser Offset im Puffer
        // liegt; read/write_unaligned setzen KEINE Ausrichtung voraus - noetig,
        // weil SubAuthorities in einer SID nur byteweise ausgerichtet sind.
        let letzte = unsafe { sid_bytes.add(8 + (anzahl - 1) * 4).cast::<u32>() };
        // SAFETY: derselbe geprueft im Puffer liegende Zeiger.
        let wert = unsafe { std::ptr::read_unaligned(letzte) };
        // SAFETY: derselbe Zeiger; der Puffer gehoert allein dieser Funktion.
        unsafe { std::ptr::write_unaligned(letzte, wert.wrapping_add(1)) };
        let fremd = fremde_sid_speicher.as_mut_ptr().cast();
        // SAFETY: `fremd` zeigt auf den eben veraenderten, noch lebenden Puffer;
        // die Pruefung stellt sicher, dass die Aenderung eine gueltige SID
        // hinterlassen hat - sonst waere der Vergleich unten bedeutungslos.
        if unsafe { IsValidSid(fremd) } == 0 {
            return false;
        }
        fremd
    } else {
        sicherheit.user_sid()
    };
    // SAFETY: beide SIDs sind geprueft gueltig und leben bis hierher - `sid` im
    // Tokenpuffer, `erwartete_sid` im Sicherheitsobjekt oder im Testpuffer.
    if unsafe { EqualSid(sid, erwartete_sid) } == 0 {
        return false;
    }
    drop(token);
    cleanup.revertieren();

    // Nach erfolgreichem Revert darf dieser Verbindungsthread kein Token mehr
    // tragen. Ein spaeterer Envelope-/Queue-/Writepfad arbeitet damit
    // nachweislich im Brokerkontext und impersoniert nicht erneut.
    let mut gegenprobe: HANDLE = std::ptr::null_mut();
    // SAFETY: `gegenprobe` ist eine lebende Lokale; ein wider Erwarten
    // geoeffnetes Token wird in der naechsten Zeile genau einmal geschlossen.
    if unsafe { OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, 0, &mut gegenprobe) } != 0 {
        // SAFETY: exklusiver Besitz des gerade geoeffneten Handles.
        unsafe { CloseHandle(gegenprobe) };
        return false;
    }
    // SAFETY: reine Abfrage des threadlokalen Fehlercodes; ERROR_NO_TOKEN ist
    // der Beweis, dass der Revert oben wirklich gegriffen hat.
    if unsafe { GetLastError() } != ERROR_NO_TOKEN {
        return false;
    }
    cleanup.spur("self");
    true
}
