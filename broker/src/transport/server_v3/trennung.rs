//! Die Trennreihenfolge aus C-06 als eigene Einheit.
//!
//! Eine Control-Verbindung faellt nie vor ihrer gekoppelten Telemetrie; wer
//! diese Reihenfolge aendert, bricht eine abgenommene Zusage.

use super::*;

/// Stand einer Kopplung fuer die Trennreihenfolge (Matrix `C-LS-06`).
///
/// Beide Tatsachen liegen unter EINEM Schloss, weil die Control-Seite sie
/// zusammen liest: "kommt ueberhaupt ein `telemetrie_getrennt`" und "ist es
/// schon da".
#[derive(Default)]
pub(super) struct TrennStand {
    /// Eine Telemetrieverbindung hat sich auf diese `link_id` gekoppelt und
    /// wird deshalb ein `telemetrie_getrennt` melden.
    ///
    /// Diese Tatsache MUSS getrennt vom Kopplungsregister stehen. Endet die
    /// Telemetrieverbindung zuerst, nimmt sie ihren Registereintrag schon vor
    /// den Joins heraus; `kopplung_loesen` der Control-Seite faende danach
    /// keine Telemetrie mehr und uebersprang den Wartepunkt, obwohl deren
    /// Trenn-Callback noch lief — `control_getrennt` konnte vor
    /// `telemetrie_getrennt` laufen (NAK-104, Pruefbefund vom 2026-08-30).
    ///
    /// Sie gilt ab der KOPPLUNG, nicht erst ab der Rueckkehr von
    /// `telemetrie_gekoppelt`: waehrend dieses Callbacks ist die Kopplung
    /// bereits vollzogen, und beide Clients koennen in diesem Fenster enden
    /// (NAK-104 Runde 2). Zurueckgenommen wird sie nur, wenn der Callback
    /// ABGELOEST wurde — dann folgt kein Gegenstueck (`C-LS-04`).
    pub(super) erwartet: bool,
    /// Ihr `telemetrie_getrennt` ist gemeldet — oder abgeloest, was nach
    /// `C-LS-06` als gelaufen zaehlt.
    pub(super) gemeldet: bool,
}

/// Wartepunkt fuer die Trennreihenfolge einer Kopplung (Matrix `C-LS-06`).
/// Die Telemetrieseite setzt ihn, nachdem sie `telemetrie_getrennt` gemeldet
/// hat; die Control-Seite wartet darauf, bevor sie `control_getrennt` meldet.
#[derive(Default)]
pub(super) struct TrennMelder {
    pub(super) stand: Mutex<TrennStand>,
    pub(super) signal: Condvar,
}

/// Ein Melder je lebender Kopplung. Er wird mit `control_verbunden` angelegt
/// und mit `control_getrennt` entfernt — derselbe Aenderungssatz wie das
/// Register selbst.
pub(super) type TrennRegister = Arc<Mutex<HashMap<String, Arc<TrennMelder>>>>;

pub(super) fn trennmelder_anlegen(reg: &TrennRegister, link_id: &str) {
    reg.lock()
        .unwrap_or_else(|e| e.into_inner())
        .insert(link_id.to_string(), Arc::new(TrennMelder::default()));
}

/// Ab jetzt wartet die Control-Seite beim Abbau auf ein `telemetrie_getrennt`.
/// Gesetzt wird das, sobald die Telemetrie gekoppelt ist und ihr
/// `telemetrie_gekoppelt` laufen KANN — also VOR dem Callback, nicht nach
/// seiner Rueckkehr (`C-LS-06`, NAK-104 Runde 2).
pub(super) fn trennmelder_telemetrie_erwartet(reg: &TrennRegister, link_id: &str) {
    let m = reg
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .get(link_id)
        .cloned();
    if let Some(m) = m {
        m.stand.lock().unwrap_or_else(|e| e.into_inner()).erwartet = true;
        m.signal.notify_all();
    }
}

/// Das Gegenstueck zu `trennmelder_telemetrie_erwartet`: das
/// `telemetrie_gekoppelt` wurde ABGELOEST, diese Verbindung meldet deshalb kein
/// `telemetrie_getrennt` (`C-LS-04`). Ohne die Ruecknahme haengt die
/// Control-Seite die volle `SENKE_FRIST` und zaehlt eine Reihenfolge-
/// verletzung, die keine ist.
pub(super) fn trennmelder_telemetrie_abgesagt(reg: &TrennRegister, link_id: &str) {
    let m = reg
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .get(link_id)
        .cloned();
    if let Some(m) = m {
        m.stand.lock().unwrap_or_else(|e| e.into_inner()).erwartet = false;
        m.signal.notify_all();
    }
}

pub(super) fn telemetrie_getrennt_gemeldet(reg: &TrennRegister, link_id: &str) {
    let m = reg
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .get(link_id)
        .cloned();
    if let Some(m) = m {
        m.stand.lock().unwrap_or_else(|e| e.into_inner()).gemeldet = true;
        m.signal.notify_all();
    }
}

/// `C-LS-06`: hoechstens `frist` warten, dann in jedem Fall weitermelden.
pub(super) fn auf_telemetrie_getrennt_warten(
    reg: &TrennRegister,
    link_id: &str,
    frist: Duration,
    statistik: &Arc<V3Statistik>,
) {
    let m = match reg
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .get(link_id)
        .cloned()
    {
        Some(m) => m,
        None => return,
    };
    let bis = Instant::now() + frist;
    let mut g = m.stand.lock().unwrap_or_else(|e| e.into_inner());
    // Nie eine Telemetrieverbindung gehabt — oder die Zusage faellt waehrend
    // des Wartens zurueck, weil ihr `telemetrie_gekoppelt` abgeloest wurde:
    // beides heisst "es kommt kein Gegenstueck", und beides ist KEINE
    // Reihenfolgeverletzung. Die Bedingung gehoert deshalb in die Schleife und
    // nicht nur davor.
    while g.erwartet && !g.gemeldet {
        let rest = bis.saturating_duration_since(Instant::now());
        if rest.is_zero() {
            statistik
                .lebenszyklus_reihenfolge_verletzt
                .fetch_add(1, Ordering::SeqCst);
            return;
        }
        let (neu, _) = m
            .signal
            .wait_timeout(g, rest)
            .unwrap_or_else(|e| e.into_inner());
        g = neu;
    }
}

/// Nimmt die Kopplung aus dem Register und bricht die I/O der mitfallenden
/// Telemetrieverbindung ab. Beruehrt die Senke NICHT — die Meldung folgt
/// getrennt, nach den Joins (`melden_getrennt`).
///
/// Ob die Control-Seite auf ein `telemetrie_getrennt` warten muss, entscheidet
/// diese Funktion NICHT: das Register sagt nur, ob die Telemetrie JETZT NOCH
/// haengt. Endete sie zuerst, ist ihr Eintrag laengst fort, waehrend ihr
/// Trenn-Callback noch laeuft. Diese Frage beantwortet allein `TrennStand`
/// (`C-LS-06`, NAK-104).
pub(super) fn kopplung_loesen(
    kopplungen: &Arc<Mutex<Kopplungen>>,
    handles: &Arc<Mutex<HandleRegister>>,
    link_id: &str,
    ist_control: bool,
    statistik: &V3Statistik,
) {
    if ist_control {
        // Die Control-Verbindung besitzt die Kopplung: geht sie, geht auch
        // der Telemetrieplatz. Sonst bliebe eine halb offene Kopplung stehen
        // (Fehlerlexikon: "zwei Pipes halb verbunden"). Den Registereintrag zu
        // entfernen genuegt nicht — der Telemetriearbeiter laeuft weiter und
        // liefert P2 zu einer Sitzung, die es nicht mehr gibt. Er wird deshalb
        // hier ABGEBROCHEN (T2-Befund 2 vom 2026-08-29).
        let ab = {
            let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
            k.control_abmelden(link_id)
        };
        if let Some(v) = ab.telemetrie_verbindung {
            io_abbrechen(handles, v, statistik);
        }
    } else {
        let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
        k.telemetrie_entkoppeln(link_id);
    }
}

/// Das Gegenstueck der Verbindungsmeldung, ueber den fristbegrenzten Pfad.
///
/// Unbedingt melden: die Senke hat ein `control_verbunden` bzw.
/// `telemetrie_gekoppelt` bekommen und braucht sein Gegenstueck auch dann,
/// wenn die Control-Verbindung die Kopplung schon aus dem Register genommen
/// hat. Nur ein frueher ABGELOESTER Aufruf laesst diesen hier ausfallen — dann
/// steht die Senke ohnehin noch im vorigen und wuerde ihn in falscher
/// Reihenfolge sehen; der Ausfall ist als `lebenszyklus_uebersprungen`
/// sichtbar.
pub(super) fn melden_getrennt(senkenruf: &mut Senkenruf, link_id: &str, ist_control: bool) {
    let l = link_id.to_string();
    if ist_control {
        senkenruf.rufen(move |s| s.control_getrennt(&l));
    } else {
        senkenruf.rufen(move |s| s.telemetrie_getrennt(&l));
    }
}
