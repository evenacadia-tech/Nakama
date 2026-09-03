//! Der Verbindungsweg vom Bootstrap bis zum letzten Frame.
//!
//! Was hier laeuft, laeuft je Verbindung in ihrem eigenen Thread: annehmen,
//! authentisieren, bootstrappen, bedienen, trennen.

use super::*;

/// Fuehrt die Lebenszyklus- und Diagnoseaufrufe EINER Verbindung mit Frist
/// aus.
///
/// `p0`/`p1`/`p2` laufen auf dem Ingressthread und sind ueber dessen
/// fristbegrenzten Join abgesichert. `control_verbunden`,
/// `telemetrie_gekoppelt`, `control_getrennt`, `telemetrie_getrennt` und
/// `abgewiesen` liefen dagegen unbegrenzt direkt auf dem VERBINDUNGSthread —
/// und `stoppen()` wartet auf genau diesen Thread, bis er fertig ist. Eine
/// Senke, die dort haengt, hielt den Stop unbegrenzt auf; der Haengertest
/// deckte nur `p0/p1/p2` (T2-Befund 5 Runde 3 vom 2026-08-29).
///
/// Jeder Aufruf laeuft deshalb auf einem eigenen, kurzlebigen Thread und wird
/// mit `SENKE_FRIST` gejoint. Laeuft sie ab, wird er ABGELOEST — und die
/// Verbindung sagt ihrer Senke danach nichts mehr: ein zweiter Aufruf liefe
/// neben dem haengenden ersten und erreichte die Senke in falscher
/// Reihenfolge. Ein Thread je Lebenszyklusereignis ist bezahlbar: es sind
/// wenige je Verbindung, und die Verbindung selbst kostet ohnehin drei.
pub(super) struct Senkenruf {
    pub(super) senke: Arc<dyn Senke>,
    pub(super) statistik: Arc<V3Statistik>,
    pub(super) stumm: bool,
}

impl Senkenruf {
    pub(super) fn neu(senke: Arc<dyn Senke>, statistik: Arc<V3Statistik>) -> Self {
        Self {
            senke,
            statistik,
            stumm: false,
        }
    }

    /// `false` = der Aufruf kam nicht binnen Frist zurueck, oder er unterblieb,
    /// weil ein frueherer noch haengt.
    pub(super) fn rufen(&mut self, f: impl FnOnce(&dyn Senke) + Send + 'static) -> bool {
        self.rufen_mit_ergebnis(move |senke| {
            f(senke);
        })
        .is_some()
    }

    pub(super) fn rufen_mit_ergebnis<T: Send + 'static>(
        &mut self,
        f: impl FnOnce(&dyn Senke) -> T + Send + 'static,
    ) -> Option<T> {
        if self.stumm {
            self.statistik
                .lebenszyklus_uebersprungen
                .fetch_add(1, Ordering::SeqCst);
            return None;
        }
        let s = self.senke.clone();
        let (antwort_tx, antwort_rx) = std::sync::mpsc::sync_channel(1);
        let j = match std::thread::Builder::new()
            .name("eqcop-v3-senkenruf".into())
            .spawn(move || {
                let _ = antwort_tx.send(f(s.as_ref()));
            }) {
            Ok(j) => j,
            Err(_) => {
                self.stumm = true;
                return None;
            }
        };
        if join_mit_frist(j, SENKE_FRIST, || {}) {
            return antwort_rx.try_recv().ok();
        }
        self.stumm = true;
        self.statistik
            .lebenszyklus_abgeloest
            .fetch_add(1, Ordering::SeqCst);
        None
    }

    /// Kurzform fuer den haeufigsten Fall.
    pub(super) fn abweisen(&mut self, grund: impl Into<String>) {
        let g = grund.into();
        self.rufen(move |s| s.abgewiesen(&g));
    }
}

pub(super) enum BootstrapRahmenStand {
    Unvollstaendig,
    ZuGross,
    Vollstaendig(usize),
}

pub(super) fn bootstrap_rahmenstand(roh: &[u8]) -> BootstrapRahmenStand {
    if roh.len() < 4 {
        return BootstrapRahmenStand::Unvollstaendig;
    }
    let n = u32::from_le_bytes([roh[0], roh[1], roh[2], roh[3]]) as usize;
    if n > crate::transport::v3::MAX_BOOTSTRAP_BYTES as usize {
        return BootstrapRahmenStand::ZuGross;
    }
    let gesamt = 4usize.saturating_add(n);
    if roh.len() < gesamt {
        BootstrapRahmenStand::Unvollstaendig
    } else {
        BootstrapRahmenStand::Vollstaendig(gesamt)
    }
}

pub(super) fn bootstrap_reject_schreiben(
    h: HANDLE,
    ereignis: HANDLE,
    grund: &str,
    cleanup: &VerbindungsSicherheitsCleanup,
) {
    let payload = serde_json::json!({
        "type": "reject",
        "code": "protocol_mismatch",
        "reason": grund.chars().take(500).collect::<String>()
    });
    if let Ok(payload) = serde_json::to_vec(&payload) {
        let mut rahmen = (payload.len() as u32).to_le_bytes().to_vec();
        rahmen.extend_from_slice(&payload);
        cleanup.spur("reject");
        let _ = ov_schreiben(h, ereignis, &rahmen);
    }
}

pub(super) struct BootstrapFrist {
    pub(super) id: u64,
    pub(super) liste: Arc<Mutex<Vec<(u64, Instant)>>>,
}

impl BootstrapFrist {
    pub(super) fn erfuellt(self) {}
}

impl Drop for BootstrapFrist {
    fn drop(&mut self) {
        if let Ok(mut l) = self.liste.lock() {
            l.retain(|(i, _)| *i != self.id);
        }
    }
}

#[allow(clippy::too_many_arguments)]
pub(super) fn verbindung_bedienen(
    id: u64,
    griff: Arc<Verbindungsgriff>,
    senke: Arc<dyn Senke>,
    kopplungen: Arc<Mutex<Kopplungen>>,
    trennmelder: TrennRegister,
    handles: Arc<Mutex<HandleRegister>>,
    bootstraps: Arc<Mutex<Vec<(u64, Instant)>>>,
    statistik: Arc<V3Statistik>,
    broker_version: String,
    broker_epoch: String,
    stop: Arc<AtomicBool>,
    writer_fehler_erzwungen: Arc<AtomicBool>,
    cancel_vor_read_phase: Arc<AtomicU64>,
    control_ausgaenge: Arc<Mutex<HashMap<String, Arc<Ausgang>>>>,
    telemetrie_ausgaenge: Arc<Mutex<HashMap<String, Arc<Ausgang>>>>,
    erwartete_sicherheit: Arc<crate::server::Sicherheit>,
    auth_fehler: V3AuthTestFehler,
    sicherheits_spur: Arc<SicherheitsSpur>,
    schreiber_haengt: Option<Arc<AtomicBool>>,
) {
    // Der Acceptor hat das Handle schon registriert; der Eintrag traegt es
    // beim Verlassen dieser Funktion wieder aus.
    let mut sicherheits_cleanup = VerbindungsSicherheitsCleanup {
        revertiert: false,
        auth_fehler,
        spur: sicherheits_spur,
    };

    // Und JETZT `stop` pruefen. Kam der Stop im Fenster zwischen Annahme und
    // diesem Punkt, endet der Thread hier — statt sich in einen Bootstrap-Read
    // zu legen, den kein Wachhund mehr aufloest (T2-Befund 6 vom 2026-08-29).
    if stop.load(Ordering::SeqCst) {
        return;
    }

    let leseereignis = match Ereignis::neu() {
        Some(e) => e,
        None => return,
    };
    let ende = match EndeSignal::neu() {
        Some(e) => Arc::new(e),
        None => return,
    };

    // Bootstrap-Frist: der Wachhund bricht die I/O ab, wenn das Hello nicht
    // rechtzeitig ganz da ist.
    if let Ok(mut b) = bootstraps.lock() {
        b.push((id, Instant::now() + BOOTSTRAP_FRIST));
    }
    let frist = BootstrapFrist {
        id,
        liste: bootstraps.clone(),
    };

    let mut roh: Vec<u8> = Vec::with_capacity(4096);
    let mut puffer = [0u8; 4096];

    // ── Bootstrap lesen ───────────────────────────────────────────────────
    let rahmen_laenge = loop {
        match bootstrap_rahmenstand(&roh) {
            BootstrapRahmenStand::Vollstaendig(n) => break n,
            BootstrapRahmenStand::ZuGross => {
                statistik
                    .geschlossen_bootstrap
                    .fetch_add(1, Ordering::SeqCst);
                return;
            }
            BootstrapRahmenStand::Unvollstaendig => {}
        }
        match ov_lesen(griff.h, leseereignis.roh(), None, &mut puffer) {
            IoAusgang::Ende => return,
            IoAusgang::Bytes(n) => {
                roh.extend_from_slice(&puffer[..n]);
                // Die Laengengrenze des Hellos selbst prueft `bootstrap_lesen`
                // (ZuGross) — schon ab dem vierten Byte, also vor jeder
                // Allokation. Hier steht nur der Schutz gegen einen Peer, der
                // pipelined: er darf hinter dem Hello schon v3-Frames
                // mitschicken, aber nicht beliebig viel, bevor das Hello
                // ueberhaupt vollstaendig ist. Ein Puffer von Hellogrenze plus
                // einem Lesehaeppchen ist die ehrliche Schranke; die alte
                // Fassung prueft VOR dem Parsen und haette ein legitimes
                // Hello mit angehaengten Frames in einem einzigen Read
                // abgewiesen.
                if roh.len() > (crate::transport::v3::MAX_BOOTSTRAP_BYTES as usize) + 4 + 4096 {
                    statistik
                        .geschlossen_bootstrap
                        .fetch_add(1, Ordering::SeqCst);
                    return;
                }
            }
            IoAusgang::Abgebrochen | IoAusgang::Fehler(_) => {
                statistik
                    .geschlossen_bootstrap
                    .fetch_add(1, Ordering::SeqCst);
                return;
            }
        }
    };
    sicherheits_cleanup.spur("read");

    // Die fachliche Auswertung ist absichtlich NACH der verbindlichen
    // Impersonation/SID/Revert-Kette. Auch ein vollstaendig gerahmtes, aber
    // syntaktisch oder semantisch abzulehnendes Hello darf vorher weder Reject
    // noch Senke/Coordinator erreichen (B-05).
    if !client_sid_authentisieren(griff.h, &erwartete_sicherheit, &mut sicherheits_cleanup) {
        statistik
            .geschlossen_bootstrap
            .fetch_add(1, Ordering::SeqCst);
        return;
    }
    let (bs, verbraucht) = match bootstrap_lesen(&roh[..rahmen_laenge]) {
        Ok(x) => x,
        Err(e) => {
            statistik
                .geschlossen_bootstrap
                .fetch_add(1, Ordering::SeqCst);
            bootstrap_reject_schreiben(
                griff.h,
                leseereignis.roh(),
                &format!("bootstrap: {e:?}"),
                &sicherheits_cleanup,
            );
            return;
        }
    };
    frist.erfuellt();
    roh.drain(0..verbraucht);

    // Erst nach erfolgreichem Revert existiert ein Fachcallback-Helfer.
    let mut senkenruf = Senkenruf::neu(senke.clone(), statistik.clone());

    // Die Writerqueue existiert bereits vor dem Telemetrie-Kopplungs-Callback:
    // ein Coordinator darf dabei den gehaltenen absoluten Messstand an einen
    // reconnectenden Subscriber einreihen, ohne auf Pipe-I/O zu warten.
    let ausgang = Arc::new(Ausgang::neu());
    let (link_id, ist_control) = match bs {
        Bootstrap::V2 { .. } => {
            // Auf dem v3-Endpunkt gibt es kein v2-Register. Statt still zu
            // scheitern bekommt der Peer einen v2-gerahmten `reject` — er
            // spricht ja noch v2-Framing (§33.3 "klarer Kompatibilitaetsfehler").
            let json = "{\"type\":\"reject\",\"code\":\"protocol_mismatch\",\
                        \"reason\":\"dieser Endpunkt spricht nur v3\"}";
            let mut rahmen = (json.len() as u32).to_le_bytes().to_vec();
            rahmen.extend_from_slice(json.as_bytes());
            sicherheits_cleanup.spur("reject");
            let _ = ov_schreiben(griff.h, leseereignis.roh(), &rahmen);
            statistik
                .geschlossen_bootstrap
                .fetch_add(1, Ordering::SeqCst);
            return;
        }
        Bootstrap::V3Control(h) => {
            sicherheits_cleanup.spur("hello_accept");
            let link = neue_kennung();
            let challenge = neue_kennung();
            {
                let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                if k.control_anmelden(&h.adresse.runtime_nonce, link.clone(), challenge.clone())
                    .is_err()
                {
                    statistik
                        .geschlossen_bootstrap
                        .fetch_add(1, Ordering::SeqCst);
                    senkenruf.abweisen("bootstrap: zu viele offene Kopplungen");
                    return;
                }
            }
            // `C-LS-02` (Regel 5): `control_verbunden` ist ABGESCHLOSSEN,
            // bevor das Welcome den Draht verlaesst. Vorher lief es umgekehrt —
            // der Peer bekam sein Welcome, koppelte seine Telemetrieverbindung,
            // und `telemetrie_gekoppelt` erreichte die Senke auf dem anderen
            // Verbindungsthread VOR `control_verbunden`. Die Senke sah den
            // zweiten Teilnehmer einer Kopplung, die sie noch nicht kannte, und
            // `telemetrie_gekoppelt` traegt nur die `link_id` — den fehlenden
            // Kontext konnte sie nicht rekonstruieren (NAK-95, Befund 5).
            //
            // Kommt die Senke nicht binnen `SENKE_FRIST` zurueck, wird der
            // Aufruf abgeloest; dann verlaesst KEIN Welcome den Draht und diese
            // Verbindung endet (`C-LS-03`). Der Abbau laeuft ueber
            // `kopplung_loesen`, nicht ueber ein blosses `control_abmelden`:
            // die Kopplung steht schon im Register und muss ganz fallen.
            //
            // Ein `control_getrennt` folgt hier bewusst NICHT: der abgeloeste
            // `control_verbunden` steht noch; ein Gegenstueck davor waere eine
            // Luege ueber die Reihenfolge. Die Senke sieht ihren Verbindungs-
            // beginn also ohne Ende — sichtbar an `lebenszyklus_abgeloest`.
            let link_fuer_senke = link.clone();
            let hello_fuer_senke = (*h).clone();
            let Some(anmeldung) = senkenruf.rufen_mit_ergebnis(move |s| {
                s.control_verbunden(&link_fuer_senke, &hello_fuer_senke)
            }) else {
                kopplung_loesen(&kopplungen, &handles, &link, true, &statistik);
                return;
            };
            if !anmeldung.angenommen {
                let grund = anmeldung
                    .grund
                    .unwrap_or_else(|| "Coordinator hat die Verbindung abgewiesen".into());
                let payload = serde_json::json!({
                    "type": "reject",
                    "code": "rate_limited",
                    "reason": grund.chars().take(500).collect::<String>()
                });
                if let Ok(payload) = serde_json::to_vec(&payload) {
                    if let Ok(frame) = envelope_schreiben(Familie::P0, P0_SCHEMA_MINOR, &payload) {
                        let _ = ov_schreiben(griff.h, leseereignis.roh(), &frame);
                    }
                }
                kopplung_loesen(&kopplungen, &handles, &link, true, &statistik);
                return;
            }
            for alter_link in anmeldung.zu_schliessende_links {
                if alter_link != link {
                    kopplung_loesen(&kopplungen, &handles, &alter_link, true, &statistik);
                }
            }
            // Ab jetzt kann eine Telemetrieverbindung koppeln; ihr
            // `telemetrie_getrennt` erwartet diese Seite beim Abbau.
            trennmelder_anlegen(&trennmelder, &link);

            let welcome = Welcome {
                typ: "welcome".into(),
                protocol: 3,
                broker_version: broker_version.clone(),
                broker_epoch: broker_epoch.clone(),
                link_id: link.clone(),
                challenge,
            };
            let payload = match serde_json::to_vec(&welcome) {
                Ok(p) => p,
                Err(_) => return,
            };
            match envelope_schreiben(Familie::P0, P0_SCHEMA_MINOR, &payload) {
                Ok(frame) => {
                    if !ov_schreiben(griff.h, leseereignis.roh(), &frame) {
                        senke.control_schliesst(&link);
                        kopplung_loesen(&kopplungen, &handles, &link, true, &statistik);
                        melden_getrennt(&mut senkenruf, &link, true);
                        trennmelder
                            .lock()
                            .unwrap_or_else(|e| e.into_inner())
                            .remove(&link);
                        return;
                    }
                    // R3-1 (Nacharbeit Runde 3, Wiederpruefung 2, 03.09.2026):
                    // der Marker liegt NACH dem erfolgreichen Write, nicht
                    // davor. `hello_accept` oben wird gesetzt, bevor der
                    // Welcome-Rahmen ueberhaupt gebaut ist; ein Test, der den
                    // Abbau nach `hello_accept` startet, kann den
                    // Verbindungsworker im Descheduling erwischen und misst
                    // dann einen vorzeitig abgebrochenen Bootstrap statt eines
                    // ungelesenen Broker-Writes (H-07).
                    sicherheits_cleanup.spur("welcome_geschrieben");
                }
                Err(_) => return,
            }
            (link, true)
        }
        Bootstrap::V3Telemetry(h) => {
            sicherheits_cleanup.spur("hello_accept");
            let ok = {
                let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                k.telemetrie_koppeln(&h, id)
            };
            if let Err(e) = ok {
                statistik
                    .geschlossen_bootstrap
                    .fetch_add(1, Ordering::SeqCst);
                senkenruf.abweisen(format!("telemetry ungekoppelt: {e:?}"));
                return;
            }
            let welcome = Welcome {
                typ: "welcome".into(),
                protocol: 3,
                broker_version: broker_version.clone(),
                broker_epoch: broker_epoch.clone(),
                link_id: h.link_id.clone(),
                challenge: h.challenge.clone(),
            };
            let payload = match serde_json::to_vec(&welcome) {
                Ok(p) => p,
                Err(_) => return,
            };
            if let Ok(frame) = envelope_schreiben(Familie::P0, P0_SCHEMA_MINOR, &payload) {
                if !ov_schreiben(griff.h, leseereignis.roh(), &frame) {
                    let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                    k.telemetrie_entkoppeln(&h.link_id);
                    return;
                }
            }
            // Die Trennpflicht steht VOR dem Kopplungs-Callback, nicht nach
            // seiner Rueckkehr. Blockiert die Senke hier fristgerecht (etwa
            // 800 ms) und enden beide Clients waehrend des Blocks, so fand die
            // Control-Seite an ihrem Wartepunkt `erwartet == false`,
            // uebersprang ihn und meldete `control_getrennt` VOR dem
            // `telemetrie_getrennt`, das gleich danach folgte — die Kopplung
            // war laengst gekoppelt, nur die Zusage fehlte noch (`C-LS-06`,
            // NAK-104 Runde 2, Pruefbefund vom 2026-08-30).
            //
            // Ab hier ist ein `telemetrie_getrennt` zugesagt. Nur ein
            // ABGELOESTES `telemetrie_gekoppelt` nimmt die Zusage zurueck: dann
            // endet diese Verbindung ohne Gegenstueck (`C-LS-04`), und ohne die
            // Ruecknahme wartete die Control-Seite die volle `SENKE_FRIST` auf
            // ein Ereignis, das nie kommt.
            telemetrie_ausgaenge
                .lock()
                .unwrap_or_else(|e| e.into_inner())
                .insert(h.link_id.clone(), ausgang.clone());
            trennmelder_telemetrie_erwartet(&trennmelder, &h.link_id);
            let link_fuer_senke = h.link_id.clone();
            if !senkenruf.rufen(move |s| s.telemetrie_gekoppelt(&link_fuer_senke)) {
                telemetrie_ausgaenge
                    .lock()
                    .unwrap_or_else(|e| e.into_inner())
                    .remove(&h.link_id);
                trennmelder_telemetrie_abgesagt(&trennmelder, &h.link_id);
                kopplung_loesen(&kopplungen, &handles, &h.link_id, false, &statistik);
                return;
            }
            (h.link_id.clone(), false)
        }
    };

    // ── Ab hier ausschliesslich v3-Frames, auf drei Threads ───────────────
    let eingang = Arc::new(Eingang::neu());
    if ist_control {
        control_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .insert(link_id.clone(), ausgang.clone());
        statistik.aktive_controls.fetch_add(1, Ordering::SeqCst);
    } else {
        telemetrie_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .insert(link_id.clone(), ausgang.clone());
    }

    let schreiber = {
        let schreiber_haengt = schreiber_haengt.clone();
        let griff = griff.clone();
        let ausgang = ausgang.clone();
        let ende = ende.clone();
        let handles = handles.clone();
        let statistik = statistik.clone();
        let cancel_vor_read_phase = cancel_vor_read_phase.clone();
        std::thread::Builder::new()
            .name("eqcop-v3-writer".into())
            .spawn(move || {
                let ereignis = match Ereignis::neu() {
                    Some(e) => e,
                    None => {
                        statistik.geschlossen_writer.fetch_add(1, Ordering::SeqCst);
                        ende.setzen();
                        io_abbrechen(&handles, id, &statistik);
                        return;
                    }
                };
                // H-02-Naht: der Schreiber bleibt ab Threadstart stehen und
                // laesst `join_mit_frist` seine SENKE_FRIST verpassen; die
                // Ablosung ist damit erzwingbar statt von einem langsamen Peer
                // abhaengig. Er endet NUR ueber den Weg, den H-02 zusagt: die
                // eigene ID steht in `HandleRegister::abgeloest`, der Wachhund
                // hat sie also erreicht. Ein echtes `ov_schreiben`, das nur
                // `CancelIoEx` loest, ist im Test nicht herstellbar - der Peer
                // muesste die Pipe offen halten und nicht lesen, und jeder Weg,
                // die Verbindung dann abzubauen, loest das Write ohnehin mit.
                if let Some(haengt) = &schreiber_haengt {
                    // D14 der Nacharbeit Runde 1 (Abschlusspruefung 1,
                    // 03.09.2026): gewartet wird auf den TATSAECHLICHEN
                    // Wachhundabbruch, nicht auf die Markierung in
                    // `abgeloest`. Der Marker wird vor dem naechsten
                    // Wachhundtick gesetzt; wer auf ihn wartet, misst nur sich
                    // selbst und liesse ein Entfernen des zugesagten
                    // wiederholten Abbruchs unbemerkt durchgehen.
                    //
                    // Die Notbremse liegt weit hinter der Frist des H-02-Tests
                    // (10 s): sie haelt einen Regressionslauf am Leben, statt
                    // ihn haengen zu lassen, ohne die Messung zu retten.
                    let notbremse = Instant::now() + Duration::from_secs(30);
                    while haengt.load(Ordering::SeqCst) {
                        let abgebrochen = handles
                            .lock()
                            .unwrap_or_else(|e| e.into_inner())
                            .abgeloest_abbrueche
                            .get(&id)
                            .copied()
                            .unwrap_or(0);
                        if abgebrochen > 0 || Instant::now() >= notbremse {
                            break;
                        }
                        std::thread::sleep(Duration::from_millis(10));
                    }
                }
                while let Some(eintrag) = ausgang.entnehmen() {
                    let erzwungen = writer_fehler_erzwungen.swap(false, Ordering::SeqCst);
                    if erzwungen
                        && cancel_vor_read_phase.load(Ordering::SeqCst) != CANCEL_VOR_READ_INAKTIV
                    {
                        let frist = Instant::now() + Duration::from_secs(5);
                        while cancel_vor_read_phase.load(Ordering::SeqCst) != CANCEL_VOR_READ_READER
                            && Instant::now() < frist
                        {
                            std::thread::yield_now();
                        }
                        if cancel_vor_read_phase.load(Ordering::SeqCst) != CANCEL_VOR_READ_READER {
                            cancel_vor_read_phase.store(CANCEL_VOR_READ_FEHLER, Ordering::SeqCst);
                        }
                    }
                    let geschrieben =
                        !erzwungen && ov_schreiben(griff.h, ereignis.roh(), &eintrag.frame);
                    if let Some(antwort) = eintrag.geschrieben {
                        let _ = antwort.send(geschrieben);
                    }
                    if !geschrieben {
                        statistik.geschlossen_writer.fetch_add(1, Ordering::SeqCst);
                        ende.setzen();
                        io_abbrechen(&handles, id, &statistik);
                        if cancel_vor_read_phase.load(Ordering::SeqCst) == CANCEL_VOR_READ_READER {
                            cancel_vor_read_phase.store(CANCEL_VOR_READ_WRITER, Ordering::SeqCst);
                        }
                        break;
                    }
                }
            })
            .ok()
    };

    // ZWEI Verbraucher, nicht einer (Matrix `C-LS-07`, Regel 4 auf der
    // Rust-Seite). Mit einem einzigen Thread stand jeder eingereihte P0-Frame
    // hinter einer Senke, die gerade in `p1` blockiert — die Antwort kam erst,
    // wenn fremder Code zurueckkehrte. Das ist P0-Starvation, gleich wie
    // schnell der Leser ist. Die Ingressqueue selbst bleibt EINE Queue mit
    // Cap 256 und der Politik aus §53.9; nur die Sicht darauf ist getrennt.
    let verbraucher_p0 = {
        let senke = senke.clone();
        let eingang = eingang.clone();
        let ausgang = ausgang.clone();
        let statistik = statistik.clone();
        let handles = handles.clone();
        let ende = ende.clone();
        let link = link_id.clone();
        std::thread::Builder::new()
            .name("eqcop-v3-ingress-p0".into())
            .spawn(move || {
                while let Some((_, schema_minor, payload)) = eingang.entnehmen_p0() {
                    if let Some(antwort) = senke.p0_mit_minor(&link, schema_minor, &payload) {
                        if let Ok(frame) =
                            envelope_schreiben(Familie::P0, P0_SCHEMA_MINOR, &antwort)
                        {
                            if !ausgang.einreihen(frame) {
                                // Der Peer holt seine Antworten nicht ab.
                                // Still weiterzaehlen waere eine Luege ueber
                                // "nichts verwerfen".
                                statistik.geschlossen_writer.fetch_add(1, Ordering::SeqCst);
                                senke.abgewiesen("writer: Antwortqueue laeuft ueber");
                                ende.setzen();
                                io_abbrechen(&handles, id, &statistik);
                                break;
                            }
                        }
                    }
                }
            })
            .ok()
    };

    let verbraucher_rest = {
        let senke = senke.clone();
        let eingang = eingang.clone();
        let link = link_id.clone();
        std::thread::Builder::new()
            .name("eqcop-v3-ingress-rest".into())
            .spawn(move || {
                while let Some((familie, schema_minor, payload)) = eingang.entnehmen_ohne_p0() {
                    match familie {
                        Familie::P1 => senke.p1_mit_minor(&link, schema_minor, &payload),
                        Familie::P2 => senke.p2_mit_minor(&link, schema_minor, &payload),
                        // `entnehmen_ohne_p0` liefert per Konstruktion kein
                        // P0; der Arm ist der Riegel gegen eine spaetere
                        // Aenderung, nicht toter Code.
                        Familie::P0 => debug_assert!(false, "P0 gehoert dem eigenen Thread"),
                    }
                }
            })
            .ok()
    };

    let mut leser = StromLeser::neu();
    leser.fuettern(&roh);
    let mut rate = Ratengrenze::neu(RATE_PRO_SEKUNDE, 1000);
    let beginn = Instant::now();
    let mut cancel_vor_read_barriere = false;

    'lesen: loop {
        loop {
            match leser.naechster() {
                LeseErgebnis::Unvollstaendig => break,
                LeseErgebnis::Verstoesse(v) => {
                    statistik
                        .geschlossen_envelope
                        .fetch_add(1, Ordering::SeqCst);
                    let namen: Vec<&str> = v.iter().map(|x| x.name()).collect();
                    senkenruf.abweisen(format!("envelope: {}", namen.join(",")));
                    break 'lesen;
                }
                LeseErgebnis::Frame(r) => {
                    let jetzt_ms = beginn.elapsed().as_millis() as u64;
                    if !rate.erlaubt(jetzt_ms) {
                        statistik.geschlossen_rate.fetch_add(1, Ordering::SeqCst);
                        senkenruf.abweisen("rate: Nachrichtenratengrenze ueberschritten");
                        break 'lesen;
                    }
                    let familie = r.kopf.familie;
                    let schema_minor = r.kopf.schema_minor;

                    if !schema_minor_bekannt(familie, schema_minor) {
                        statistik
                            .geschlossen_envelope
                            .fetch_add(1, Ordering::SeqCst);
                        senkenruf.abweisen(format!(
                            "envelope: schema_minor {schema_minor} fuer {familie:?} unbekannt"
                        ));
                        break 'lesen;
                    }

                    // Familienzuordnung des Vertrags: die Control-Verbindung
                    // traegt P0/P1, die Telemetrieverbindung traegt P2
                    // (`eq-ipc-v3.schema.json`, hello_control/hello_telemetry).
                    // Ohne diese Sperre koennte eine Telemetriepipe einen
                    // gueltigen P0-Heartbeat setzen und umgekehrt ein
                    // Featureframe ueber die Steuerleitung laufen.
                    let erlaubt = if ist_control {
                        familie != Familie::P2
                    } else {
                        familie == Familie::P2
                    };
                    if !erlaubt {
                        statistik.geschlossen_familie.fetch_add(1, Ordering::SeqCst);
                        senkenruf.abweisen(format!(
                            "familie: {familie:?} auf einer {}-Verbindung",
                            if ist_control { "control" } else { "telemetry" }
                        ));
                        break 'lesen;
                    }

                    // Die Control-Verbindung BESITZT die Kopplung. Endet sie,
                    // gehoert dieser Telemetrieframe zu keiner Sitzung mehr.
                    if !ist_control {
                        let lebt = {
                            let k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
                            k.telemetrie_lebt(&link_id, id)
                        };
                        if !lebt {
                            statistik
                                .geschlossen_kopplung
                                .fetch_add(1, Ordering::SeqCst);
                            senkenruf.abweisen("kopplung: Control-Verbindung ist fort");
                            break 'lesen;
                        }
                    }

                    match eingang.einreihen(familie, schema_minor, r.payload) {
                        IngressErgebnis::Eingereiht => {}
                        IngressErgebnis::P2Verworfen => {
                            statistik
                                .ingress_p2_verworfen
                                .fetch_add(1, Ordering::SeqCst);
                        }
                        IngressErgebnis::ClientTrennen => {
                            // `A-IN-03`/`A-IN-04`: P0 UND P1 trennen. Ein
                            // stiller P1-Verlust bei gesund wirkender
                            // Verbindung nimmt dem Sender den Wiederholweg.
                            if familie == Familie::P1 {
                                statistik
                                    .ingress_p1_ueberlauf_trennt
                                    .fetch_add(1, Ordering::SeqCst);
                                senkenruf.abweisen("ingress: P1-Ueberlauf");
                            } else {
                                statistik
                                    .geschlossen_p0_ueberlauf
                                    .fetch_add(1, Ordering::SeqCst);
                                senkenruf.abweisen("ingress: P0-Ueberlauf");
                            }
                            break 'lesen;
                        }
                        IngressErgebnis::Verworfen => {
                            statistik
                                .ingress_p2_verworfen
                                .fetch_add(1, Ordering::SeqCst);
                        }
                    }
                    statistik.hoechststand_melden(eingang.laenge());
                }
            }
        }

        if stop.load(Ordering::SeqCst) || ende.gesetzt() {
            break;
        }
        // Harte Obergrenze fuer den Lesepuffer: ein Peer darf nicht beliebig
        // viele Teilbytes anhaeufen, ohne je einen Frame zu vollenden.
        if leser.offen() > MAX_FRAME_BYTES as usize + 4 {
            statistik
                .geschlossen_envelope
                .fetch_add(1, Ordering::SeqCst);
            senkenruf.abweisen("envelope: Teilframe ueber der Paketgrenze");
            break;
        }
        if std::mem::take(&mut cancel_vor_read_barriere) {
            // Deterministische Testbarriere fuer genau das verlorene
            // Cancel-vor-Read-Fenster: Der letzte Ende-Check liegt hinter uns,
            // der naechste Read wurde aber noch nicht abgesetzt.
            if cancel_vor_read_phase
                .compare_exchange(
                    CANCEL_VOR_READ_GELESEN,
                    CANCEL_VOR_READ_READER,
                    Ordering::SeqCst,
                    Ordering::SeqCst,
                )
                .is_err()
            {
                cancel_vor_read_phase.store(CANCEL_VOR_READ_FEHLER, Ordering::SeqCst);
            }
            let frist = Instant::now() + Duration::from_secs(5);
            while cancel_vor_read_phase.load(Ordering::SeqCst) != CANCEL_VOR_READ_WRITER
                && Instant::now() < frist
            {
                std::thread::yield_now();
            }
            let phase = if cancel_vor_read_phase.load(Ordering::SeqCst) == CANCEL_VOR_READ_WRITER {
                CANCEL_VOR_READ_FORTGESETZT
            } else {
                CANCEL_VOR_READ_FEHLER
            };
            cancel_vor_read_phase.store(phase, Ordering::SeqCst);
        }
        match ov_lesen(griff.h, leseereignis.roh(), Some(&ende), &mut puffer) {
            IoAusgang::Bytes(n) => {
                leser.fuettern(&puffer[..n]);
                cancel_vor_read_barriere = cancel_vor_read_phase
                    .compare_exchange(
                        CANCEL_VOR_READ_BEREIT,
                        CANCEL_VOR_READ_GELESEN,
                        Ordering::SeqCst,
                        Ordering::SeqCst,
                    )
                    .is_ok();
            }
            IoAusgang::Ende | IoAusgang::Abgebrochen => break,
            IoAusgang::Fehler(f) => {
                senkenruf.abweisen(format!("lesen: Win32 {f}"));
                break;
            }
        }
    }

    // Gegenpfad zu den drei Threads: erst die Queues schliessen, dann die
    // haengende I/O aufloesen, dann joinen. Ein Schreiber, der auf einen
    // stillen Peer wartet, muss abgebrochen werden — sonst waere `join` ein
    // Hang.
    ende.setzen();
    // 28-B: Kein neuer Ingress darf den atomaren Cleanup ueberholen. Ein
    // schon laufender Coordinator-Aufruf teilt dessen Lock; entweder commitet
    // er vorher und wird hier entfernt, oder er sieht danach `schliessend`.
    eingang.schliessen();
    if ist_control {
        control_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .remove(&link_id);
        statistik.aktive_controls.fetch_sub(1, Ordering::SeqCst);
        senke.control_schliesst(&link_id);
    } else {
        telemetrie_ausgaenge
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .remove(&link_id);
    }
    // Nach dem atomaren semantischen Gegenpfad faellt die KOPPLUNG ebenfalls
    // vor den fristbegrenzten Joins, nicht nach ihnen. Sonst blieb sie bei
    // langsamer Senke bis zu zweimal `SENKE_FRIST`
    // im Register, und ein Telemetrieframe passierte in dieser Zeit weiter
    // `telemetrie_lebt`, obwohl die Control-Verbindung schon fort war
    // (T2-Befund 3 Runde 3 vom 2026-08-29). Der Registereintrag und der
    // Abbruch der Telemetrie-I/O gehoeren dabei zusammen: ohne den Abbruch
    // stuende der Telemetriearbeiter noch in seinem Read.
    kopplung_loesen(&kopplungen, &handles, &link_id, ist_control, &statistik);
    ausgang.schliessen();
    io_abbrechen(&handles, id, &statistik);
    // Alle Joins haben eine FRIST. Steht ein Verbraucher in einem
    // Senkenaufruf oder der Schreiber in einem Write, den `CancelIoEx` nicht
    // loest, wird der Thread abgeloest statt gejoint — sonst waere `stoppen()`
    // ein unbegrenztes Warten auf fremden Code (T2-Befund 7 vom 2026-08-29).
    for j in [verbraucher_p0, verbraucher_rest].into_iter().flatten() {
        if !join_mit_frist(j, SENKE_FRIST, || {}) {
            statistik.senke_abgeloest.fetch_add(1, Ordering::SeqCst);
        }
    }
    if let Some(j) = schreiber {
        if !join_mit_frist(j, SENKE_FRIST, || io_abbrechen(&handles, id, &statistik)) {
            statistik.schreiber_abgeloest.fetch_add(1, Ordering::SeqCst);
            // H-02, und die Reihenfolge ist die Zusage: erst zaehlen, dann die
            // ID in `abgeloest` eintragen. Der Wachhund bricht sie danach bei
            // JEDEM Tick erneut ab, bis der Thread endet; sein geteilter
            // `Verbindungsgriff` faellt dann als letzter und traegt die ID aus
            // beiden Mengen aus, bevor er schliesst. Ohne diesen Eintrag
            // verbrannte eine abgeloeste Verbindung ihre Pipeinstanz bis zum
            // Serverstopp: `BootstrapFrist` hat ihre ID laengst ausgetragen.
            handles
                .lock()
                .unwrap_or_else(|e| e.into_inner())
                .abgeloest
                .insert(id);
        }
    }

    // Erst JETZT die Senke benachrichtigen: waehrend der Joins konnte noch ein
    // `p0`/`p1`/`p2` derselben Verbindung laufen, und ein `*_getrennt` davor
    // waere eine Luege ueber die Reihenfolge.
    //
    // `C-LS-06`: die Control-Seite meldet ihr `control_getrennt` NACH dem
    // `telemetrie_getrennt` der mitfallenden Telemetrieverbindung. Zwischen
    // zwei Verbindungsthreads ist die Reihenfolge sonst unbestimmt; die Senke
    // saehe das Ende der Kopplung vor dem Ende ihres Teilnehmers. Gewartet
    // wird hoechstens `SENKE_FRIST` — laeuft sie ab (weil das
    // `telemetrie_getrennt` der Gegenseite selbst abgeloest wurde), meldet
    // diese Seite TROTZDEM: ein abgeloester Trenn-Callback zaehlt als
    // gelaufen, es entfaellt keiner und keiner laeuft doppelt.
    if ist_control {
        auf_telemetrie_getrennt_warten(&trennmelder, &link_id, SENKE_FRIST, &statistik);
    }
    melden_getrennt(&mut senkenruf, &link_id, ist_control);
    if !ist_control {
        telemetrie_getrennt_gemeldet(&trennmelder, &link_id);
    } else {
        trennmelder
            .lock()
            .unwrap_or_else(|e| e.into_inner())
            .remove(&link_id);
    }
}
