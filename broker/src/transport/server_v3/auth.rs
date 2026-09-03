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
    if cleanup.auth_fehler == V3AuthTestFehler::Impersonate
        || unsafe { ImpersonateNamedPipeClient(h) } == 0
    {
        return false;
    }

    let mut token_roh: HANDLE = std::ptr::null_mut();
    if cleanup.auth_fehler == V3AuthTestFehler::OpenThreadToken
        || unsafe { OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, 0, &mut token_roh) } == 0
    {
        return false;
    }
    let token = TokenGriff(token_roh);

    let mut noetig = 0u32;
    unsafe {
        GetTokenInformation(token.0, TokenUser, std::ptr::null_mut(), 0, &mut noetig);
    }
    let groessen_fehler = unsafe { GetLastError() };
    if cleanup.auth_fehler == V3AuthTestFehler::TokenGroesse
        || groessen_fehler != ERROR_INSUFFICIENT_BUFFER
        || noetig < std::mem::size_of::<TOKEN_USER>() as u32
    {
        return false;
    }
    let mut puffer = vec![0u64; (noetig as usize).div_ceil(std::mem::size_of::<u64>())];
    if cleanup.auth_fehler == V3AuthTestFehler::TokenInformation
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
    let sid = unsafe { (*puffer.as_ptr().cast::<TOKEN_USER>()).User.Sid };
    cleanup.spur("sid");
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
        let laenge = unsafe { GetLengthSid(sicherheit.user_sid()) };
        if laenge < 12 {
            return false;
        }
        fremde_sid_speicher.resize((laenge as usize).div_ceil(std::mem::size_of::<u64>()), 0);
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
        let anzahl = unsafe { *sid_bytes.add(1) } as usize;
        if anzahl == 0 || 8 + anzahl * 4 > laenge as usize {
            return false;
        }
        let letzte = unsafe { sid_bytes.add(8 + (anzahl - 1) * 4).cast::<u32>() };
        let wert = unsafe { std::ptr::read_unaligned(letzte) };
        unsafe { std::ptr::write_unaligned(letzte, wert.wrapping_add(1)) };
        let fremd = fremde_sid_speicher.as_mut_ptr().cast();
        if unsafe { IsValidSid(fremd) } == 0 {
            return false;
        }
        fremd
    } else {
        sicherheit.user_sid()
    };
    if unsafe { EqualSid(sid, erwartete_sid) } == 0 {
        return false;
    }
    drop(token);
    cleanup.revertieren();

    // Nach erfolgreichem Revert darf dieser Verbindungsthread kein Token mehr
    // tragen. Ein spaeterer Envelope-/Queue-/Writepfad arbeitet damit
    // nachweislich im Brokerkontext und impersoniert nicht erneut.
    let mut gegenprobe: HANDLE = std::ptr::null_mut();
    if unsafe { OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, 0, &mut gegenprobe) } != 0 {
        unsafe { CloseHandle(gegenprobe) };
        return false;
    }
    if unsafe { GetLastError() } != ERROR_NO_TOKEN {
        return false;
    }
    cleanup.spur("self");
    true
}
