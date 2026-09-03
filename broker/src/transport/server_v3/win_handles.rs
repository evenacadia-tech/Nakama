//! Win32-Besitz und ueberlappende I/O.
//!
//! Jedes Handle hat genau einen Besitzer, und jeder Besitzer raeumt in genau
//! einem Destruktor auf. Wer hier eine Reihenfolge verschiebt, verschiebt eine
//! Soundness-Zusage.

use super::*;

/// Ein Ereignisobjekt fuer overlapped I/O. JEDER Thread haelt sein eigenes —
/// zwei Threads an einem Event waere genau das Rennen, das die Trennung
/// vermeiden soll.
pub(super) struct Ereignis(pub(super) HANDLE);

// SAFETY: ein Event-HANDLE ist eine prozessweite Kernel-Referenz ohne
// Thread-Affinitaet; dieser Typ gibt es nie zwei Threads gleichzeitig weiter.
unsafe impl Send for Ereignis {}

impl Ereignis {
    pub(super) fn neu() -> Option<Self> {
        // SAFETY: alle Zeiger sind null bzw. gueltig; das Handle wird im Drop
        // genau einmal geschlossen.
        let h = unsafe { CreateEventW(std::ptr::null(), 1, 0, std::ptr::null()) };
        if h.is_null() {
            None
        } else {
            Some(Ereignis(h))
        }
    }
    pub(super) fn roh(&self) -> HANDLE {
        self.0
    }
}

impl Drop for Ereignis {
    fn drop(&mut self) {
        // SAFETY: exklusiver Besitz, genau einmal geschlossen.
        unsafe { CloseHandle(self.0) };
    }
}

/// Eine bereits mit `ConnectNamedPipe` bewaffnete Besitzinstanz. Die
/// `OVERLAPPED`-Adresse liegt in einer Box und bleibt deshalb auch beim
/// Verschieben zwischen Acceptor und Shutdown-Register stabil.
pub(super) struct ListenerInstanz {
    pub(super) h: HANDLE,
    pub(super) ereignis: Ereignis,
    pub(super) ov: Box<OVERLAPPED>,
    pub(super) ausstehend: bool,
    pub(super) sofort_verbunden: bool,
}

// SAFETY: Handle, Event und OVERLAPPED gehoeren exklusiv dieser Instanz. Sie
// werden nie gleichzeitig von zwei Threads veraendert; beim Shutdown wandert
// der ganze Besitzer nach dem Acceptor-Join zum stoppenden Thread.
unsafe impl Send for ListenerInstanz {}

impl ListenerInstanz {
    pub(super) fn neu(name_w: &[u16], attrs: &SECURITY_ATTRIBUTES, erste: bool) -> Result<Self, u32> {
        let flags = PIPE_ACCESS_DUPLEX
            | FILE_FLAG_OVERLAPPED
            | if erste {
                FILE_FLAG_FIRST_PIPE_INSTANCE
            } else {
                0
            };
        // SAFETY: Name ist nullterminiert, attrs und Deskriptor leben ueber den
        // Aufruf. Das Handle wird unmittelbar in diesem RAII-Typ gebunden.
        let h = unsafe {
            CreateNamedPipeW(
                name_w.as_ptr(),
                flags,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                PIPE_INSTANZEN as u32,
                65536,
                65536,
                0,
                attrs,
            )
        };
        if h == INVALID_HANDLE_VALUE {
            // SAFETY: reine Abfrage des Fehlercodes des eigenen Threads; sie
            // fasst kein Handle an und muss unmittelbar nach dem
            // fehlgeschlagenen Aufruf stehen, sonst ist der Code fremd.
            return Err(unsafe { GetLastError() });
        }
        let Some(ereignis) = Ereignis::neu() else {
            // SAFETY: dieselbe reine Abfrage; sie laeuft VOR dem Schliessen,
            // weil CloseHandle den Fehlercode ueberschreiben wuerde.
            let f = unsafe { GetLastError() };
            // SAFETY: `h` ist das gerade erfolgreich erzeugte Pipe-Handle,
            // gehoert noch niemandem sonst und wird hier genau einmal
            // geschlossen - der Konstruktor gibt es nicht heraus.
            unsafe { CloseHandle(h) };
            return Err(f);
        };
        let ov = Box::new(leeres_overlapped(ereignis.roh()));
        Ok(Self {
            h,
            ereignis,
            ov,
            ausstehend: false,
            sofort_verbunden: false,
        })
    }

    pub(super) fn armieren(&mut self) -> Result<(), u32> {
        // SAFETY: Handle/Event/OVERLAPPED gehoeren exklusiv `self`; die Box
        // haelt die Adresse bis Completion oder Drop stabil.
        unsafe {
            ResetEvent(self.ereignis.roh());
            *self.ov = leeres_overlapped(self.ereignis.roh());
            let ok = ConnectNamedPipe(self.h, self.ov.as_mut());
            if ok != 0 {
                self.sofort_verbunden = true;
                SetEvent(self.ereignis.roh());
                return Ok(());
            }
            let f = GetLastError();
            if f == ERROR_PIPE_CONNECTED {
                self.sofort_verbunden = true;
                SetEvent(self.ereignis.roh());
                return Ok(());
            }
            if f == ERROR_IO_PENDING {
                self.ausstehend = true;
                return Ok(());
            }
            Err(f)
        }
    }

    pub(super) fn ereignis(&self) -> HANDLE {
        self.ereignis.roh()
    }

    pub(super) fn verbindung_fertig(&mut self) -> bool {
        if self.sofort_verbunden {
            return true;
        }
        if !self.ausstehend {
            return false;
        }
        let mut n = 0u32;
        // SAFETY: das Event hat signalisiert; OVERLAPPED/Handle leben.
        let ok = unsafe { GetOverlappedResult(self.h, self.ov.as_mut(), &mut n, 0) } != 0;
        if ok {
            self.ausstehend = false;
        }
        ok
    }

    pub(super) fn handle_uebernehmen(mut self) -> HANDLE {
        debug_assert!(!self.ausstehend);
        let h = self.h;
        self.h = std::ptr::null_mut();
        h
    }
}

impl Drop for ListenerInstanz {
    fn drop(&mut self) {
        if self.h.is_null() || self.h == INVALID_HANDLE_VALUE {
            return;
        }
        // Eine OVERLAPPED-Struktur darf erst nach bestaetigter Completion
        // verschwinden. Cancel + blockierendes Result wird erst beim finalen
        // Shutdown ausgefuehrt und kann nicht vom Peer offen gehalten werden.
        if self.ausstehend {
            // Das blockierende GetOverlappedResult wartet die Completion ab,
            // bevor Handle und OVERLAPPED fallen.
            // SAFETY: `self.h` lebt bis zum Ende dieses Destruktors, und
            // `self.ov` liegt in einer Box mit stabiler Adresse.
            unsafe {
                CancelIoEx(self.h, self.ov.as_mut());
                let mut verworfen = 0u32;
                GetOverlappedResult(self.h, self.ov.as_mut(), &mut verworfen, 1);
            }
            self.ausstehend = false;
        }
        // SAFETY: exklusiver Besitz; nach dem Abschnitt darueber ist keine
        // I/O mehr ausstehend, und `self.h` wird genau einmal geschlossen.
        unsafe {
            DisconnectNamedPipe(self.h);
            CloseHandle(self.h);
        }
        self.h = std::ptr::null_mut();
    }
}

/// Dauerhaft gesetztes Lebenszyklus-Signal einer Verbindung. Anders als das
/// private I/O-Event eines `Ereignis` wird dieses Event absichtlich zwischen
/// Reader und Writer geteilt und nach dem Setzen nie zurueckgesetzt.
pub(super) struct EndeSignal(pub(super) HANDLE);

// SAFETY: Win32-Eventhandles sind prozessweite Kernel-Referenzen ohne
// Thread-Affinitaet. `SetEvent` und die Wait-Funktionen duerfen dasselbe Handle
// nebenlaeufig verwenden; `Drop` laeuft erst nach dem letzten `Arc`-Besitzer.
unsafe impl Send for EndeSignal {}

// SAFETY: dieselbe Begruendung wie fuer `Send` eine Zeile hoeher - ein
// Eventhandle vertraegt nebenlaeufige `SetEvent`- und Wait-Aufrufe, und der Typ
// bietet keinen inneren Zustand, der dabei zerfallen koennte.
unsafe impl Sync for EndeSignal {}

impl EndeSignal {
    pub(super) fn neu() -> Option<Self> {
        // SAFETY: alle Zeiger sind null; das manuell zurueckgesetzte Event wird
        // im Drop genau einmal geschlossen.
        let h = unsafe { CreateEventW(std::ptr::null(), 1, 0, std::ptr::null()) };
        if h.is_null() {
            None
        } else {
            Some(Self(h))
        }
    }

    pub(super) fn setzen(&self) {
        // SAFETY: `self.0` bleibt durch den aufrufenden `Arc` gueltig. SetEvent
        // ist idempotent; dieses Signal wird absichtlich nie zurueckgesetzt.
        unsafe { SetEvent(self.0) };
    }

    pub(super) fn gesetzt(&self) -> bool {
        // SAFETY: gueltiges Eventhandle; Timeout 0 blockiert nicht.
        unsafe { WaitForSingleObject(self.0, 0) == WAIT_OBJECT_0 }
    }

    pub(super) fn roh(&self) -> HANDLE {
        self.0
    }
}

impl Drop for EndeSignal {
    fn drop(&mut self) {
        // SAFETY: exklusiver Besitz beim letzten `Arc`, genau einmal geschlossen.
        unsafe { CloseHandle(self.0) };
    }
}

pub(super) fn leeres_overlapped(e: HANDLE) -> OVERLAPPED {
    // SAFETY: OVERLAPPED ist ein reines POD-Feld ohne Invarianten; genullt ist
    // der von Win32 verlangte Startzustand.
    let mut ov: OVERLAPPED = unsafe { std::mem::zeroed() };
    ov.hEvent = e;
    ov
}

/// Ausgang eines overlapped Lesevorgangs.
pub(super) enum IoAusgang {
    Bytes(usize),
    /// Der Peer hat geschlossen.
    Ende,
    /// `CancelIoEx` — Stop, Bootstrapfrist oder Verbindungsende.
    Abgebrochen,
    Fehler(u32),
}

pub(super) fn io_fehler_deuten(f: u32) -> IoAusgang {
    match f {
        ERROR_BROKEN_PIPE | ERROR_PIPE_NOT_CONNECTED | ERROR_NO_DATA => IoAusgang::Ende,
        ERROR_OPERATION_ABORTED => IoAusgang::Abgebrochen,
        andere => IoAusgang::Fehler(andere),
    }
}

/// Ein Lesevorgang. Ohne lokales Ende-Signal wartet er bis zur I/O oder einem
/// externen `CancelIoEx` (Bootstrap/Serverstopp). Im laufenden Verbindungsweg
/// wartet er auf I/O UND das dauerhafte Ende-Signal. Dadurch bleibt auch ein
/// Ende dicht, das zwischen dem letzten Zustandscheck und `ReadFile` eintritt.
pub(super) fn ov_lesen(h: HANDLE, e: HANDLE, ende: Option<&EndeSignal>, ziel: &mut [u8]) -> IoAusgang {
    // `ov`, Handle-Array und `ziel` bleiben gueltig, bis
    // GetOverlappedResult beziehungsweise der Wait zurueck ist.
    // SAFETY: `h` ist ein gueltiges, overlapped geoeffnetes Pipe-Handle, `e`
    // gehoert allein diesem Thread, und ein vorhandenes `ende` lebt ueber den Aufruf.
    unsafe {
        ResetEvent(e);
        let mut ov = leeres_overlapped(e);
        let mut n: u32 = 0;
        let ok = ReadFile(
            h,
            ziel.as_mut_ptr(),
            ziel.len() as u32,
            std::ptr::null_mut(),
            &mut ov,
        );
        if ok == 0 {
            let f = GetLastError();
            if f != ERROR_IO_PENDING {
                return io_fehler_deuten(f);
            }
            let gewartet = if let Some(ende) = ende {
                let ereignisse = [e, ende.roh()];
                WaitForMultipleObjects(ereignisse.len() as u32, ereignisse.as_ptr(), 0, INFINITE)
            } else {
                WaitForSingleObject(e, INFINITE)
            };
            if gewartet != WAIT_OBJECT_0 {
                // Das Ende-Signal (Index 1) oder ein Wait-Fehler gewinnt gegen
                // den noch ausstehenden Read. Die Operation muss abgeschlossen
                // sein, bevor `ov` und `ziel` ihren Gueltigkeitsbereich verlassen.
                CancelIoEx(h, &ov);
                let _ = GetOverlappedResult(h, &ov, &mut n, 1);
                return IoAusgang::Abgebrochen;
            }
        }
        if GetOverlappedResult(h, &ov, &mut n, 1) == 0 {
            return io_fehler_deuten(GetLastError());
        }
        if n == 0 {
            IoAusgang::Ende
        } else {
            IoAusgang::Bytes(n as usize)
        }
    }
}

/// Schreibt ALLE Bytes oder scheitert. Blockiert, solange der Peer nicht
/// abholt — genau deshalb sitzt der Aufruf auf einem eigenen Thread.
pub(super) fn ov_schreiben(h: HANDLE, e: HANDLE, daten: &[u8]) -> bool {
    let mut ab = 0usize;
    while ab < daten.len() {
        // SAFETY: wie in `ov_lesen`.
        unsafe {
            ResetEvent(e);
            let mut ov = leeres_overlapped(e);
            let mut n: u32 = 0;
            let rest = &daten[ab..];
            let ok = WriteFile(
                h,
                rest.as_ptr(),
                rest.len() as u32,
                std::ptr::null_mut(),
                &mut ov,
            );
            if ok == 0 {
                let f = GetLastError();
                if f != ERROR_IO_PENDING {
                    return false;
                }
                if WaitForSingleObject(e, INFINITE) != WAIT_OBJECT_0 {
                    CancelIoEx(h, &ov);
                    let _ = GetOverlappedResult(h, &ov, &mut n, 1);
                    return false;
                }
            }
            if GetOverlappedResult(h, &ov, &mut n, 1) == 0 || n == 0 {
                return false;
            }
            ab += n as usize;
        }
    }
    true
}

/// Besitzt genau ein Verbindungshandle. Alle drei Threads halten einen `Arc`
/// darauf; geschlossen wird es, wenn der letzte geht.
pub(super) struct Verbindungsgriff {
    pub(super) h: HANDLE,
    pub(super) sicherheits_spur: Arc<SicherheitsSpur>,
    /// H-01: Registereintrag und Handle haben EINEN Besitzer. Vorher trug ein
    /// eigener `HandleEintrag` den Austrag; fielen beide in einer Closure, war
    /// ihre Reihenfolge unbestimmt, und das Register konnte kurzzeitig ein
    /// bereits geschlossenes Handle veroeffentlichen.
    pub(super) id: u64,
    pub(super) register: Arc<Mutex<HandleRegister>>,
    /// Testnaht, die den Destruktor im Fenster zwischen Austrag und Schliessen
    /// anhaelt. Im Produktbetrieb ist sie `None` und kostet einen Vergleich.
    pub(super) destruktor_fenster: Option<Arc<V3UebergabeBarriere>>,
}

// SAFETY: Win32-HANDLEs sind prozessweite Kernel-Referenzen ohne
// Thread-Affinitaet; overlapped I/O erlaubt gleichzeitige Operationen.
unsafe impl Send for Verbindungsgriff {}

unsafe impl Sync for Verbindungsgriff {}

impl Drop for Verbindungsgriff {
    fn drop(&mut self) {
        // H-01, die Reihenfolge IST die Zusage: unter der Registersperre
        // austragen, danach schliessen. Umgekehrt saehe ein gleichzeitiges
        // `alle_io_abbrechen` ein Handle, das der Kernel schon wiederverwenden
        // durfte. Der Austrag raeumt beide Mengen, damit der Wachhund aus H-02
        // keine tote ID weiterverfolgt.
        {
            let mut r = self.register.lock().unwrap_or_else(|e| e.into_inner());
            r.offen.retain(|(i, _)| *i != self.id);
            r.abgeloest.remove(&self.id);
        }
        self.sicherheits_spur.push("register_austrag");
        if let Some(fenster) = &self.destruktor_fenster {
            fenster.im_destruktorfenster_warten();
        }
        // SAFETY: exklusiver Besitz ueber den Arc, genau einmal geschlossen;
        // der Registereintrag ist eine Zeile hoeher gefallen.
        unsafe { CloseHandle(self.h) };
        self.sicherheits_spur.push("close");
    }
}

/// Ausschliesslich die Negativtests brauchen eine detaillierte API-Spur.
/// Im Produkt bleibt sie abgeschaltet: kein wachsender Diagnosevektor und
/// kein Trace-Mutex auf dem Verbindungsweg.
pub(super) struct SicherheitsSpur {
    pub(super) aktiv: bool,
    pub(super) inhalt: Mutex<Vec<&'static str>>,
}

impl SicherheitsSpur {
    pub(super) fn neu(aktiv: bool) -> Self {
        Self {
            aktiv,
            inhalt: Mutex::new(Vec::new()),
        }
    }

    pub(super) fn push(&self, schritt: &'static str) {
        if !self.aktiv {
            return;
        }
        if let Ok(mut inhalt) = self.inhalt.lock() {
            inhalt.push(schritt);
        }
    }

    pub(super) fn snapshot(&self) -> Vec<&'static str> {
        if !self.aktiv {
            return Vec::new();
        }
        self.inhalt
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .clone()
    }
}

/// Handles der lebenden Verbindungen — nur, damit `stoppen()` blockierende
/// Reads wirklich loesen kann. Ein Stop, der auf einen stillen Peer wartet,
/// ist kein Stop (wissen/engineering 2026-08-27: "Cancel als Abschluss
/// behandelt").
#[derive(Default)]
pub(super) struct HandleRegister {
    pub(super) offen: Vec<(u64, isize)>,
    /// H-02: Verbindungen, deren Schreiberthread abgeloest wurde. Der Wachhund
    /// bricht sie bei JEDEM Tick erneut ab, bis der Thread endet und sein
    /// geteilter `Verbindungsgriff` faellt. Ohne diesen Weg erreichte ein
    /// abgeloester etablierter Schreiber keinen Abbruch mehr: `BootstrapFrist`
    /// hat seine ID laengst ausgetragen, und der Wachhund sah nur faellige
    /// Bootstrap-IDs.
    pub(super) abgeloest: std::collections::HashSet<u64>,
}

// Das Register haelt ein Handle nur, solange der besitzende Thread seinen
// `Verbindungsgriff` haelt; Eintragen, Austragen und Abbrechen laufen unter
// demselben Mutex.
// SAFETY: Win32-HANDLEs sind prozessweite Kernel-Referenzen ohne Thread-Affinitaet.
unsafe impl Send for HandleRegister {}

/// Bricht die I/O eines Handles ab und wertet den Rueckgabewert aus (H-01).
///
/// `ERROR_NOT_FOUND` ist der Normalfall und heisst nur „da war gerade nichts
/// ausstehendes". Alles andere — allen voran `ERROR_INVALID_HANDLE` — bedeutet,
/// dass das Register ein Handle fuehrte, das es nicht mehr geben durfte; genau
/// das darf nicht verschluckt werden, sonst faellt die Ein-Besitzer-Invariante
/// unbemerkt.
fn abbrechen_und_zaehlen(h: HANDLE, statistik: Option<&V3Statistik>) {
    // SAFETY: der Eintrag lebt nur, solange der besitzende Thread sein Handle
    // haelt; Austragen und Abbrechen laufen unter derselben Registersperre.
    let ok = unsafe { CancelIoEx(h, std::ptr::null_mut()) };
    if ok != 0 {
        return;
    }
    // SAFETY: reine Abfrage des Fehlercodes des eigenen Threads.
    let fehler = unsafe { GetLastError() };
    if fehler == ERROR_NOT_FOUND {
        return;
    }
    if let Some(s) = statistik {
        s.cancel_auf_totem_handle.fetch_add(1, Ordering::SeqCst);
    }
}

/// Derselbe Abbruch fuer einen Aufrufer, der das Handle schon in der Hand hat
/// (der Wachhund haelt die Registersperre bereits).
pub(super) fn abbrechen_und_zaehlen_extern(h: HANDLE, statistik: &V3Statistik) {
    abbrechen_und_zaehlen(h, Some(statistik))
}

pub(super) fn alle_io_abbrechen(handles: &Arc<Mutex<HandleRegister>>) {
    alle_io_abbrechen_gezaehlt(handles, None)
}

pub(super) fn alle_io_abbrechen_gezaehlt(
    handles: &Arc<Mutex<HandleRegister>>,
    statistik: Option<&V3Statistik>,
) {
    if let Ok(reg) = handles.lock() {
        for (_, h) in reg.offen.iter() {
            abbrechen_und_zaehlen(*h as HANDLE, statistik);
        }
    }
}

pub(super) fn io_abbrechen(handles: &Arc<Mutex<HandleRegister>>, id: u64) {
    io_abbrechen_gezaehlt(handles, id, None)
}

pub(super) fn io_abbrechen_gezaehlt(
    handles: &Arc<Mutex<HandleRegister>>,
    id: u64,
    statistik: Option<&V3Statistik>,
) {
    if let Ok(reg) = handles.lock() {
        for (i, h) in reg.offen.iter() {
            if *i == id {
                abbrechen_und_zaehlen(*h as HANDLE, statistik);
            }
        }
    }
}

pub(super) struct TokenGriff(pub(super) HANDLE);

impl Drop for TokenGriff {
    fn drop(&mut self) {
        if !self.0.is_null() {
            // SAFETY: exklusiver Besitz des Tokenhandles; die Nullpruefung
            // darueber schliesst den Fall aus, dass es nie erzeugt wurde, und
            // der Destruktor laeuft genau einmal.
            unsafe { CloseHandle(self.0) };
        }
    }
}
