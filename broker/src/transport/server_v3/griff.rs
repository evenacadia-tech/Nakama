//! Serverhandle, Sender, Statistik und Workerplaetze.
//!
//! Der Griff ist das, was ein Aufrufer vom laufenden v3-Server in der Hand
//! haelt: stoppen, senden, zaehlen. Er nimmt keine Verbindung selbst an.

use super::*;

//==============================================================================
// Overlapped-I/O: die kleinste Menge Win32, die drei Threads auf einem Handle
// erlaubt.

//==============================================================================
// Die zwei bounded Queues zwischen den drei Threads.

//==============================================================================

pub struct V3Griff {
    pub(super) stop: Arc<AtomicBool>,
    pub(super) pipe_name: String,
    pub(super) acceptor_stop: Arc<EndeSignal>,
    pub(super) acceptor: Option<JoinHandle<()>>,
    pub(super) rest_listener: Arc<Mutex<Vec<ListenerInstanz>>>,
    pub(super) verbindungen: Arc<Mutex<Vec<JoinHandle<()>>>>,
    pub(super) handles: Arc<Mutex<HandleRegister>>,
    /// Deadlines der noch nicht abgeschlossenen Bootstraps.
    pub(super) bootstraps: Arc<Mutex<Vec<(u64, Instant)>>>,
    pub(super) closer: V3Closer,
    pub(super) wachhund: Option<JoinHandle<()>>,
    pub(super) sender: V3Sender,
    pub(super) sicherheits_spur: Arc<SicherheitsSpur>,
    pub(super) uebergabe_barriere: Option<Arc<V3UebergabeBarriere>>,
    /// Die EINZIGE Wahrheit ueber die Zahl lebender Worker: derselbe Zaehler,
    /// den `worker_reservieren` per `compare_exchange` besetzt und
    /// `WorkerPlatz::drop` per `fetch_sub` freigibt. Eine zweite, aus
    /// gelesenen Vorwerten fortgeschriebene Kopie gab es bis NAK-123 R1 in
    /// `V3Statistik`; sie konnte bei verschraenkten Freigaben dauerhaft
    /// auseinanderlaufen und `broker_soll_idle_enden` belogen.
    pub(super) worker_zaehler: Arc<AtomicUsize>,
    pub statistik: Arc<V3Statistik>,
}

#[derive(Clone)]
pub struct V3Closer {
    pub(super) kopplungen: Arc<Mutex<Kopplungen>>,
    pub(super) handles: Arc<Mutex<HandleRegister>>,
    /// D11: auch dieser Abbruchweg zaehlt unerwartete `CancelIoEx`-Fehler.
    /// Ohne die Statistik am Closer bliebe genau der Pfad ungezaehlt, den
    /// H-01 fuer das Verbindungsende zusagt.
    pub(super) statistik: Arc<V3Statistik>,
}

impl V3Closer {
    pub fn link_schliessen(&self, link_id: &str) {
        kopplung_loesen(&self.kopplungen, &self.handles, link_id, true, &self.statistik);
    }
}

#[derive(Clone)]
pub struct V3Sender {
    pub(super) control_ausgaenge: Arc<Mutex<HashMap<String, Arc<Ausgang>>>>,
    pub(super) telemetrie_ausgaenge: Arc<Mutex<HashMap<String, Arc<Ausgang>>>>,
}

impl V3Sender {
    pub fn neu() -> Self {
        Self {
            control_ausgaenge: Arc::new(Mutex::new(HashMap::new())),
            telemetrie_ausgaenge: Arc::new(Mutex::new(HashMap::new())),
        }
    }

    pub fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
        let frame = match envelope_schreiben(Familie::P1, P1_SCHEMA_MINOR, payload) {
            Ok(frame) => frame,
            Err(_) => return false,
        };
        let ausgang = self
            .control_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .get(link_id)
            .cloned();
        let Some(ausgang) = ausgang else {
            return false;
        };
        let Some(antwort) = ausgang.snapshot_einreihen_mit_antwort("session_snapshot", frame)
        else {
            return false;
        };
        antwort.recv_timeout(SENKE_FRIST).unwrap_or(false)
    }

    pub fn messframe_schreiben(&self, link_id: &str, instance_id: &str, payload: &[u8]) -> bool {
        let frame = match envelope_schreiben(Familie::P2, P2_SCHEMA_MINOR, payload) {
            Ok(frame) => frame,
            Err(_) => return false,
        };
        let ausgang = self
            .telemetrie_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .get(link_id)
            .cloned();
        ausgang.is_some_and(|ausgang| ausgang.messframe_einreihen(instance_id, frame))
    }
}

impl crate::coordinator::SessionPush for V3Sender {
    fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
        V3Sender::snapshot_schreiben(self, link_id, payload)
    }

    fn messframe_schreiben(&self, link_id: &str, instance_id: &str, payload: &[u8]) -> bool {
        V3Sender::messframe_schreiben(self, link_id, instance_id, payload)
    }
}

/// Zahlen des Listeners selbst (nicht der Senke).
#[derive(Debug, Default)]
pub struct V3Statistik {
    pub angenommen: AtomicU64,
    pub geschlossen_bootstrap: AtomicU64,
    pub geschlossen_envelope: AtomicU64,
    pub geschlossen_rate: AtomicU64,
    pub geschlossen_p0_ueberlauf: AtomicU64,
    /// Familie auf der falschen Verbindungsart (Control traegt P0/P1,
    /// Telemetry traegt P2 — `eq-ipc-v3.schema.json`).
    pub geschlossen_familie: AtomicU64,
    /// Telemetrieframe, dessen Control-Verbindung nicht mehr lebt.
    pub geschlossen_kopplung: AtomicU64,
    /// Der Peer holt seine Antworten nicht ab; die Writerqueue lief ueber.
    pub geschlossen_writer: AtomicU64,
    /// Wie oft der Acceptor an der Verbindungsgrenze auf einen freien Platz
    /// warten musste. Ist die Zahl 0, WAR die Grenze nie erreicht — ein Test
    /// darueber spraeche dann ueber nichts.
    pub acceptor_wartet_auf_instanz: AtomicU64,
    /// Zwei nach erfolgreichem Start, null erst beim finalen Listener-Drop.
    pub bewaffnete_listener: AtomicU64,
    pub am_worker_cap_abgewiesen: AtomicU64,
    pub listener_fehler: AtomicU64,
    pub worker_uebergaben: AtomicU64,
    pub ingress_p2_verworfen: AtomicU64,
    /// Wie oft ein P1-Ueberlauf ohne P2 zum Verwerfen die Verbindung getrennt
    /// hat (Matrix `A-IN-04`). P1 faellt nie still: §53.9 gibt ihm den
    /// Wiederholweg ueber Reconnect/Outbox, und den nimmt ihm ein stiller
    /// Verlust bei offener Verbindung.
    pub ingress_p1_ueberlauf_trennt: AtomicU64,
    /// Hoechststand der Ingressqueue ueber alle Verbindungen. Er ist der
    /// Beleg, dass der Leser nicht am ersten Frame haengt.
    pub ingress_hoechststand: AtomicU64,
    /// Wie oft ein Verbraucherthread beim Verbindungsschluss ABGELOEST wurde,
    /// weil sein Senkenaufruf laenger als `SENKE_FRIST` stand. Die Zahl ist
    /// der sichtbare Preis dafuer, dass `stoppen()` trotzdem endet.
    pub senke_abgeloest: AtomicU64,
    /// Dasselbe fuer den Schreiberthread, dessen `ov_schreiben` trotz
    /// wiederholtem `CancelIoEx` nicht zurueckkam.
    pub schreiber_abgeloest: AtomicU64,
    /// H-01: `CancelIoEx` scheiterte mit etwas anderem als `ERROR_NOT_FOUND` —
    /// im Regelfall `ERROR_INVALID_HANDLE`. Im Normalbetrieb strukturell null:
    /// das Register fuehrt ein Handle nur, solange sein Besitzer lebt. Ein Wert
    /// groesser null heisst, dass Austrag und Schliessen auseinandergelaufen
    /// sind, und wird deshalb gezaehlt statt verschluckt.
    pub cancel_auf_totem_handle: AtomicU64,
    /// H-19: eine Panik in einem v3-Verbindungsthread, beim Ernten am
    /// Joinergebnis erkannt. Im Normalbetrieb strukturell null - ein Wert
    /// groesser null ist ein Befund, kein Betriebsrauschen.
    pub verbindungsthread_panik: AtomicU64,
    /// H-07: wie oft der beschraenkte Abfluss vor dem Schliessen seine Frist
    /// verpasst hat. Der Wert macht sichtbar, was sonst still verschwaende -
    /// ein Snapshot oder ein Ablehnungsgrund im Ausgabepuffer eines Peers, der
    /// nicht liest.
    pub flush_abgelaufen: AtomicU64,
    /// Wie oft ein LEBENSZYKLUS-Aufruf der Senke (`control_verbunden`,
    /// `telemetrie_gekoppelt`, `*_getrennt`, `abgewiesen`) laenger als
    /// `SENKE_FRIST` stand und deshalb abgeloest wurde. Diese Aufrufe liefen
    /// frueher unbegrenzt auf dem Verbindungsthread — und genau auf den
    /// wartet `stoppen()` (T2-Befund 5 Runde 3 vom 2026-08-29).
    pub lebenszyklus_abgeloest: AtomicU64,
    /// Wie viele weitere Lebenszyklusaufrufe DESHALB unterblieben. Nach einem
    /// abgeloesten Aufruf schweigt die Verbindung gegenueber ihrer Senke,
    /// statt neben dem haengenden Aufruf in falscher Reihenfolge weiterzureden.
    pub lebenszyklus_uebersprungen: AtomicU64,
    /// Wie oft die Control-Seite ihr `control_getrennt` melden musste, OHNE
    /// dass das `telemetrie_getrennt` der Gegenseite binnen `SENKE_FRIST` kam
    /// (Matrix `C-LS-06`). Die Meldung faellt nie aus — sie ist dann nur
    /// nicht mehr geordnet, und genau das steht hier.
    pub lebenszyklus_reihenfolge_verletzt: AtomicU64,
    pub aktive_controls: AtomicU64,
}

impl V3Statistik {
    pub(super) fn hoechststand_melden(&self, stand: usize) {
        let stand = stand as u64;
        let mut alt = self.ingress_hoechststand.load(Ordering::Relaxed);
        while stand > alt {
            match self.ingress_hoechststand.compare_exchange_weak(
                alt,
                stand,
                Ordering::Relaxed,
                Ordering::Relaxed,
            ) {
                Ok(_) => break,
                Err(a) => alt = a,
            }
        }
    }
}

impl V3Griff {
    pub fn pipe_name(&self) -> &str {
        &self.pipe_name
    }

    /// Wie viele Verbindungs-Threadhandles der Listener gerade haelt. Sie
    /// werden laufend geerntet; die Zahl darf ueber viele Verbindungszyklen
    /// nicht wachsen (T2-Befund 8 vom 2026-08-29).
    pub fn gehaltene_verbindungen(&self) -> usize {
        self.verbindungen
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .len()
    }

    /// Wie viele Verbindungshandles das Abbruchregister gerade fuehrt. Ein
    /// Handle steht dort, seit der Acceptor die Verbindung angenommen hat —
    /// NICHT erst, seit sein Thread laeuft (T2-Befund 6 vom 2026-08-29).
    pub fn gehaltene_handles(&self) -> usize {
        self.handles
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .offen
            .len()
    }

    /// R2-4 (Nacharbeit Runde 2, 03.09.2026): die Groesse ALLER DREI
    /// Registermengen - `offen`, `abgeloest`, `abgeloest_abbrueche`.
    ///
    /// `gehaltene_handles()` misst nur die erste. Der mit D14 eingefuehrte
    /// Zaehler `abgeloest_abbrueche` bekam keinen Gegenweg im Destruktor und
    /// wuchs damit ueber die gesamte Brokerlaufzeit; ohne diese Beobachtung
    /// waere sein Abraeumen von aussen unbeobachtbar - dieselbe Begruendung wie
    /// bei `gehaltene_handles()` selbst.
    pub fn registermengen(&self) -> (usize, usize, usize) {
        let r = self.handles.lock().unwrap_or_else(|e| e.into_inner());
        (r.offen.len(), r.abgeloest.len(), r.abgeloest_abbrueche.len())
    }

    /// R2-4: wie oft der Wachhund fuer eine ID TATSAECHLICH abgebrochen hat,
    /// 0 wenn nichts (mehr) gefuehrt wird. Nur so ist messbar, dass die
    /// D14-Naht ihren Abbruch noch SIEHT, bevor der Destruktor abraeumt.
    pub fn abgeloest_abbrueche_gesamt(&self) -> u64 {
        self.handles
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .abgeloest_abbrueche
            .values()
            .sum()
    }

    pub fn sender(&self) -> V3Sender {
        self.sender.clone()
    }

    pub fn closer(&self) -> V3Closer {
        self.closer.clone()
    }

    pub fn aktive_controls(&self) -> u64 {
        self.statistik.aktive_controls.load(Ordering::SeqCst)
    }

    /// Alle bereits angenommenen v3-Verbindungen, einschliesslich eines noch
    /// laufenden Bootstrap-Hellos. Auch dieser Zustand ist nicht idle: Der
    /// Prozess darf einem gerade authentisierenden Client nicht unter dem
    /// Handle wegsterben, nur weil der Coordinator ihn noch nicht kennt.
    pub fn aktive_worker(&self) -> u64 {
        self.worker_zaehler.load(Ordering::SeqCst) as u64
    }

    /// Nur fuer Tests: belegt einen Workerplatz auf DEMSELBEN Zaehler, den der
    /// Acceptor benutzt, ueber genau dieselbe `worker_reservieren`. Der
    /// optionale Haltepunkt laeuft im `drop` nach der Zaehlerfreigabe; damit
    /// verschraenkt ein Test zwei Freigaben ohne Sleep-Raten (NAK-123 R1, P1).
    #[doc(hidden)]
    pub fn worker_platz_probe(
        &self,
        freigabe_haltepunkt: Option<Arc<dyn Fn() + Send + Sync>>,
    ) -> Option<WorkerPlatzProbe> {
        let mut platz = worker_reservieren(&self.worker_zaehler)?;
        platz.freigabe_haltepunkt = freigabe_haltepunkt;
        Some(WorkerPlatzProbe(platz))
    }

    /// Ein Listener-/Ersatzfehler beendet den Acceptor fail-closed. Der
    /// Prozess-Lifecycle liest dieses Signal und faehrt den Griff sofort
    /// geordnet herunter; bis dahin bleiben die Restlistener in Besitz.
    pub fn fataler_listenerfehler(&self) -> bool {
        self.statistik.listener_fehler.load(Ordering::SeqCst) != 0
    }

    #[doc(hidden)]
    pub fn sicherheits_spur(&self) -> Vec<&'static str> {
        self.sicherheits_spur.snapshot()
    }

    /// Gegenpfad zu `v3_server_starten`. Setzt Stop, weckt den parkenden
    /// Acceptor ueber sein Event und bricht die I/O aller lebenden
    /// Verbindungen ab. Die beiden Besitzlistener bleiben waehrend aller
    /// Joins offen und fallen als letzte Pipehandles.
    pub fn stoppen(&mut self) {
        self.stop.store(true, Ordering::SeqCst);
        self.acceptor_stop.setzen();
        if let Some(barriere) = &self.uebergabe_barriere {
            barriere.freigeben();
        }
        alle_io_abbrechen(&self.handles, &self.statistik);
        if let Some(j) = self.acceptor.take() {
            let _ = j.join();
        }
        if let Some(j) = self.wachhund.take() {
            let _ = j.join();
        }
        loop {
            let naechster = {
                let mut v = self.verbindungen.lock().unwrap_or_else(|e| e.into_inner());
                v.pop()
            };
            match naechster {
                Some(j) => {
                    // WIEDERHOLT abbrechen, nicht einmal. Ein einzelnes
                    // `CancelIoEx` verpufft, wenn der Thread seinen Read erst
                    // danach absetzt — und `join` waere dann ein Hang.
                    while !j.is_finished() {
                        alle_io_abbrechen(&self.handles, &self.statistik);
                        std::thread::sleep(Duration::from_millis(5));
                    }
                    let _ = j.join();
                }
                None => break,
            }
        }
        let _ = self.bootstraps.lock().map(|mut b| b.clear());
        let listener_geschlossen = {
            let mut listener = self.rest_listener.lock().unwrap_or_else(|e| e.into_inner());
            let vorhanden = !listener.is_empty();
            listener.clear();
            vorhanden
        };
        self.statistik
            .bewaffnete_listener
            .store(0, Ordering::SeqCst);
        if listener_geschlossen {
            // A-09: erst nachdem Acceptor, Wachhund und alle normalen
            // Verbindungsworker beendet und deren Handles geschlossen sind,
            // fallen die beiden letzten Besitzlistener.
            self.sicherheits_spur.push("listeners_close");
        }
    }
}

impl Drop for V3Griff {
    fn drop(&mut self) {
        self.stoppen();
    }
}

/// Joint mit Frist. Laeuft sie ab, wird der Thread ABGELOEST statt gejoint:
/// das `JoinHandle` faellt, der Thread haelt nur noch seine `Arc`s und endet
/// von selbst, sobald sein Fremdaufruf zurueckkommt. `false` = abgeloest.
///
/// `zwischendurch` laeuft in jeder Warterunde — der Schreiber braucht dort
/// sein wiederholtes `CancelIoEx`, der Verbraucher nichts.
pub(super) fn join_mit_frist(j: JoinHandle<()>, frist: Duration, mut zwischendurch: impl FnMut()) -> bool {
    let bis = Instant::now() + frist;
    while !j.is_finished() {
        if Instant::now() >= bis {
            return false;
        }
        zwischendurch();
        std::thread::sleep(Duration::from_millis(2));
    }
    let _ = j.join();
    true
}

/// Beendete Verbindungsthreads joinen und aus der Liste nehmen. Ohne das
/// waechst der Vektor — und mit ihm die nativen Threadhandles — bei jedem
/// Verbinden/Trennen unbegrenzt (T2-Befund 8 vom 2026-08-29).
pub(super) fn fertige_ernten(
    verbindungen: &Arc<Mutex<Vec<JoinHandle<()>>>>,
    statistik: &V3Statistik,
) {
    let fertig: Vec<JoinHandle<()>> = {
        let mut v = verbindungen.lock().unwrap_or_else(|e| e.into_inner());
        let mut raus = Vec::new();
        let mut i = 0;
        while i < v.len() {
            if v[i].is_finished() {
                raus.push(v.remove(i));
            } else {
                i += 1;
            }
        }
        raus
    };
    for j in fertig {
        if j.join().is_err() {
            // H-19: eine Panik in einem Verbindungsthread wird GEZAEHLT. Ohne
            // sie entfiele genau das Erkennungssignal fuer die Fehlerklasse,
            // die dieser Endpunkt jagt - und zwar dort, wo angreiferkontrollierte
            // Bytes geparst werden. Das v2-Ende wertet sein Joinergebnis
            // laengst aus; hier fiel es bis NAK-121 auf den Boden.
            statistik.verbindungsthread_panik.fetch_add(1, Ordering::SeqCst);
        }
    }
}

pub(super) struct WorkerPlatz {
    pub(super) zaehler: Arc<AtomicUsize>,
    /// Ausschliesslich `worker_platz_probe` setzt das: laeuft im `drop`
    /// unmittelbar NACH der Zaehlerfreigabe und macht das Interleaving zweier
    /// Freigaben bzw. Freigabe gegen Reserve deterministisch messbar. Im
    /// Acceptor-Pfad ist der Wert immer `None`.
    pub(super) freigabe_haltepunkt: Option<Arc<dyn Fn() + Send + Sync>>,
}

/// Ein von `V3Griff::worker_platz_probe` belegter Workerplatz. Er haelt den
/// ECHTEN `WorkerPlatz` und gibt ihn beim Fallenlassen ueber genau denselben
/// `Drop` frei, den auch ein Verbindungsthread benutzt.
#[doc(hidden)]
pub struct WorkerPlatzProbe(#[allow(dead_code)] WorkerPlatz);

pub(super) struct TestHilfsthread {
    pub(super) zaehler: Option<Arc<AtomicUsize>>,
}

impl TestHilfsthread {
    pub(super) fn neu(zaehler: Option<Arc<AtomicUsize>>) -> Self {
        if let Some(z) = &zaehler {
            z.fetch_add(1, Ordering::SeqCst);
        }
        Self { zaehler }
    }
}

impl Drop for TestHilfsthread {
    fn drop(&mut self) {
        if let Some(z) = &self.zaehler {
            z.fetch_sub(1, Ordering::SeqCst);
        }
    }
}

impl Drop for WorkerPlatz {
    fn drop(&mut self) {
        // Die Freigabe IST dieses `fetch_sub` — danach folgt kein zweiter
        // Schreibschritt mehr. Ein `store(vorher-1)` aus dem gelesenen Vorwert
        // war bis NAK-123 R1 genau die Luecke, in der zwei verschraenkte
        // Freigaben den sichtbaren Stand dauerhaft falsch stehen liessen (P1).
        let vorher = self.zaehler.fetch_sub(1, Ordering::SeqCst);
        debug_assert!(vorher > 0);
        if let Some(haltepunkt) = &self.freigabe_haltepunkt {
            haltepunkt();
        }
    }
}

pub(super) fn worker_reservieren(zaehler: &Arc<AtomicUsize>) -> Option<WorkerPlatz> {
    let mut stand = zaehler.load(Ordering::SeqCst);
    loop {
        if stand >= MAX_VERBINDUNGEN {
            return None;
        }
        match zaehler.compare_exchange_weak(stand, stand + 1, Ordering::SeqCst, Ordering::SeqCst) {
            // Der geglueckte Tausch ist die Reservierung. Keine abgeleitete
            // Kopie: `V3Griff::aktive_worker` liest genau diesen Zaehler.
            Ok(_) => {
                return Some(WorkerPlatz {
                    zaehler: zaehler.clone(),
                    freigabe_haltepunkt: None,
                })
            }
            Err(neu) => stand = neu,
        }
    }
}
