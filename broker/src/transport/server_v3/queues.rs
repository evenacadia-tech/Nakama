//! Ein- und Ausgang je Verbindung.
//!
//! Rueckstau wird hier gemessen und hier entschieden: welche Prioritaet faellt,
//! welche trennt, und wo ein Snapshot einen aelteren gleicher Adresse ersetzt.

use super::*;

/// Ingress je Verbindung (Cap 256, §53.9). Der Leser reiht ein, der
/// Verbraucher entnimmt — die Politik selbst liegt in `warteschlange.rs`.
pub(super) struct Eingang {
    pub(super) inhalt: Mutex<(IngressWarteschlange<(Familie, u8, Vec<u8>)>, bool)>,
    pub(super) signal: Condvar,
}

impl Eingang {
    pub(super) fn neu() -> Self {
        Self {
            inhalt: Mutex::new((IngressWarteschlange::neu(), false)),
            signal: Condvar::new(),
        }
    }

    pub(super) fn einreihen(&self, familie: Familie, schema_minor: u8, payload: Vec<u8>) -> IngressErgebnis {
        let e = {
            let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
            g.0.einreihen(familie, (familie, schema_minor, payload))
        };
        // `notify_all`, weil ZWEI Verbraucher warten (P0 und der Rest). Ein
        // `notify_one` koennte den falschen wecken; der P0-Thread schliefe
        // dann bis zum Wecktakt, obwohl seine Antwort schon anliegt.
        self.signal.notify_all();
        e
    }

    /// Blockiert, bis ein Eintrag da ist oder die Queue geschlossen wurde.
    ///
    /// Das SCHLIESSFLAG steht VOR dem Inhaltstest. Vorher lief der Verbraucher
    /// nach dem Schliessen noch durch den Restbestand — und rief P0/P1 fuer
    /// eine Verbindung, deren Kopplung schon abgemeldet war. Die Frames
    /// gehoeren zu einer Sitzung, die es nicht mehr gibt; sie fallen mit ihr
    /// (T2-Befund 4 Runde 3 vom 2026-08-29).
    /// Nur P0. Der P0-Verbraucher laeuft auf einem EIGENEN Thread
    /// (Matrix `C-LS-07`): steht die Senke in `p1`, muss ein bereits
    /// eingereihter P0-Frame trotzdem beantwortet werden — sonst haengt der
    /// Antwortweg hinter fremdem Code, und genau das ist P0-Starvation.
    pub(super) fn entnehmen_p0(&self) -> Option<(Familie, u8, Vec<u8>)> {
        self.entnehmen_nach(true)
    }

    /// Alles ausser P0. Gegenstueck zu `entnehmen_p0`.
    pub(super) fn entnehmen_ohne_p0(&self) -> Option<(Familie, u8, Vec<u8>)> {
        self.entnehmen_nach(false)
    }

    pub(super) fn entnehmen_nach(&self, p0: bool) -> Option<(Familie, u8, Vec<u8>)> {
        let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
        loop {
            if g.1 {
                return None;
            }
            let treffer = if p0 {
                g.0.entnehmen_p0()
            } else {
                g.0.entnehmen_ohne_p0()
            };
            if let Some((_, wert)) = treffer {
                return Some(wert);
            }
            let (neu, _) = self
                .signal
                .wait_timeout(g, Duration::from_millis(50))
                .unwrap_or_else(|x| x.into_inner());
            g = neu;
        }
    }

    pub(super) fn laenge(&self) -> usize {
        self.inhalt
            .lock()
            .unwrap_or_else(|x| x.into_inner())
            .0
            .len()
    }

    pub(super) fn schliessen(&self) {
        {
            let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
            g.1 = true;
        }
        self.signal.notify_all();
    }
}

/// Writerqueue je Verbindung (Cap `CAP_WRITER`). P0-Antworten haben Vorrang;
/// P1-Snapshots koaleszieren nach Objektschluessel, P2-Livestaende auf der
/// getrennten Telemetrieverbindung nach `instance_id`.
///
/// 🔑 NAK-246 D5 (R-D5, Manifest §5.5 Feinheiten 2 und 3): ein Snapshot traegt
/// seine Ordnungsmarke im Eintrag, und der Ausgang merkt sich unter seinem
/// Mutex je Objektschluessel die hoechste angenommene Marke - die
/// Hochwassermarke. Sie ueberlebt die Entnahme: ein aelterer Nachzuegler wird
/// auch dann verworfen, wenn der neuere schon geschrieben und die Queue
/// dazwischen leer war. Die Karte haelt einen Eintrag je Objektschluessel, den
/// dieser Link je angenommen hat, und stirbt mit dem Ausgang, also mit der
/// Verbindung.
pub(super) struct Ausgang {
    /// Eintraege, geschlossen, Hochwassermarke je Objektschluessel.
    pub(super) inhalt: Mutex<(VecDeque<AusgangEintrag>, bool, HashMap<String, i64>)>,
    pub(super) signal: Condvar,
    /// NAK-246 D5 (§5.5 Feinheit 3): wie viele Snapshots als aelterer
    /// Nachzuegler verworfen wurden (`marke < hochwasser`). Gezaehlt unter dem
    /// Mutex, lesbar ohne ihn.
    pub(super) nachzuegler_verworfen: AtomicU64,
    /// NAK-246 R-E5-1: wie viele Einreihentscheidungen dieser Ausgang getroffen
    /// hat - je Aufruf von `einreihen_eintrag` eine, gleich ob angenommen,
    /// ersetzt, als Nachzuegler verworfen oder abgewiesen. Ein Test wartet auf
    /// diese Zahl, bevor er entnimmt; die Rueckkehr eines Aufrufers belegt keine
    /// Einreihung (`V3Sender::snapshot_schreiben` kehrt auch nach `SENKE_FRIST`
    /// zurueck). Testinfrastruktur, nur im Testbau: im Produkt gibt es weder
    /// das Feld noch seine Erhoehung.
    #[cfg(test)]
    pub(super) einreihentscheidungen: AtomicU64,
}

/// Die Marke eines Snapshots ohne eigenes Ordinal. Dieselbe Zahl nennt der
/// Vertrag `SessionPush::snapshot_schreiben` (`coordinator/mod.rs`), und der
/// Coordinator fuehrt sie als `MARKE_OHNE_ORDINAL` (`coordinator/flush.rs`).
/// Der Ausgang ersetzt sie beim Einreihen durch das Hochwasser des Schluessels.
pub(super) const MARKE_OHNE_ORDINAL: i64 = i64::MIN;

#[derive(Debug, PartialEq, Eq)]
pub(super) enum Ausgangsart {
    P0,
    Snapshot(String),
    Messframe(String),
}

pub(super) struct AusgangEintrag {
    pub(super) art: Ausgangsart,
    /// NAK-246 D5: die Ordnungsmarke eines Snapshots - das Store-Ordinal des
    /// Commits oder die `event_sequence` der Erfassung. Sie lebt hier und nie
    /// im Frame; die Wire-Form ist unberuehrt. P0 und Messframes tragen 0 und
    /// werden nie nach ihr beurteilt.
    pub(super) marke: i64,
    pub(super) frame: Vec<u8>,
    pub(super) geschrieben: Option<std::sync::mpsc::SyncSender<bool>>,
}

impl Ausgang {
    pub(super) fn neu() -> Self {
        Self {
            inhalt: Mutex::new((VecDeque::with_capacity(16), false, HashMap::new())),
            signal: Condvar::new(),
            nachzuegler_verworfen: AtomicU64::new(0),
            #[cfg(test)]
            einreihentscheidungen: AtomicU64::new(0),
        }
    }

    /// `false` = die Queue ist voll oder geschlossen. Voll heisst: der Peer
    /// holt seine Antworten nicht ab.
    pub(super) fn einreihen(&self, frame: Vec<u8>) -> bool {
        self.einreihen_eintrag(AusgangEintrag {
            art: Ausgangsart::P0,
            marke: 0,
            frame,
            geschrieben: None,
        })
    }

    /// `None` = nicht aufgenommen: die Queue ist geschlossen oder voll, oder
    /// der Snapshot ist ein aelterer Nachzuegler seines Schluessels (NAK-246 D5).
    pub(super) fn snapshot_einreihen_mit_antwort(
        &self,
        objekt_schluessel: &str,
        marke: i64,
        frame: Vec<u8>,
    ) -> Option<std::sync::mpsc::Receiver<bool>> {
        let (tx, rx) = std::sync::mpsc::sync_channel(1);
        self.einreihen_eintrag(AusgangEintrag {
            art: Ausgangsart::Snapshot(objekt_schluessel.to_owned()),
            marke,
            frame,
            geschrieben: Some(tx),
        })
        .then_some(rx)
    }

    pub(super) fn messframe_einreihen(&self, instance_id: &str, frame: Vec<u8>) -> bool {
        self.einreihen_eintrag(AusgangEintrag {
            art: Ausgangsart::Messframe(instance_id.to_owned()),
            marke: 0,
            frame,
            geschrieben: None,
        })
    }

    pub(super) fn einreihen_eintrag(&self, mut eintrag: AusgangEintrag) -> bool {
        let mut ersetzt = None;
        let ok = {
            let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
            if g.1 {
                false
            } else if let Ausgangsart::Snapshot(objekt_schluessel) = &eintrag.art {
                // 🔑 NAK-246 D5 (R-D5, §5.5 Feinheit 2, R-M2-1): die Annahmeregel.
                // Verworfen wird nur, was KLEINER ist als das Hochwasser seines
                // Schluessels - auch in eine leere Queue nicht; der Aufrufer
                // bekommt `false`, der Zaehler waechst. Gleiche Marke ist derselbe
                // Commit-Stand und kein Nachzuegler: sie ersetzt wie eine groessere
                // den Eintrag gleichen Schluessels an Ort und Stelle oder wird
                // aufgenommen. Das Hochwasser steigt nur mit einer ANGENOMMENEN
                // Marke.
                let schluessel = objekt_schluessel.clone();
                let hochwasser = g.2.get(&schluessel).copied();
                if eintrag.marke == MARKE_OHNE_ORDINAL {
                    // Kein eigenes Ordinal: der Snapshot traegt das Hochwasser
                    // seines Schluessels und ist damit nie ein Nachzuegler.
                    eintrag.marke = hochwasser.unwrap_or(0);
                }
                if hochwasser.is_some_and(|hoch| eintrag.marke < hoch) {
                    self.nachzuegler_verworfen.fetch_add(1, Ordering::SeqCst);
                    false
                } else if let Some(position) = g.0.iter().position(|alt| {
                    matches!(&alt.art, Ausgangsart::Snapshot(alt_schluessel)
                        if *alt_schluessel == schluessel)
                }) {
                    g.2.insert(schluessel, eintrag.marke);
                    ersetzt = Some(std::mem::replace(&mut g.0[position], eintrag));
                    true
                } else if g.0.len() >= CAP_WRITER {
                    false
                } else {
                    g.2.insert(schluessel, eintrag.marke);
                    g.0.push_back(eintrag);
                    true
                }
            } else if let Ausgangsart::Messframe(instance_id) = &eintrag.art {
                if let Some(position) = g.0.iter().position(|alt| {
                    matches!(&alt.art, Ausgangsart::Messframe(alt_instance_id)
                        if alt_instance_id == instance_id)
                }) {
                    ersetzt = Some(std::mem::replace(&mut g.0[position], eintrag));
                    true
                } else if g.0.len() >= CAP_WRITER {
                    false
                } else {
                    g.0.push_back(eintrag);
                    true
                }
            } else if g.0.len() >= CAP_WRITER {
                false
            } else {
                g.0.push_back(eintrag);
                true
            }
        };
        if let Some(alt) = ersetzt {
            // Der alte Snapshot ist absichtlich NICHT geschrieben. Seine
            // Store-Schuld bleibt bestehen, bis der neuere absolute Stand
            // erfolgreich geschrieben und bis zu dessen event_ord gedeckt ist.
            if let Some(antwort) = alt.geschrieben {
                let _ = antwort.send(false);
            }
        }
        if ok {
            self.signal.notify_one();
        }
        // NAK-246 R-E5-1: gezaehlt am Ende des Aufrufs - die Entscheidung steht
        // in der Queue, und ein ersetzter Aufrufer hat sein `false`.
        #[cfg(test)]
        self.einreihentscheidungen.fetch_add(1, Ordering::SeqCst);
        ok
    }

    pub(super) fn entnehmen(&self) -> Option<AusgangEintrag> {
        let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
        loop {
            if let Some(position) =
                g.0.iter()
                    .position(|eintrag| eintrag.art == Ausgangsart::P0)
            {
                return g.0.remove(position);
            }
            if let Some(eintrag) = g.0.pop_front() {
                return Some(eintrag);
            }
            if g.1 {
                return None;
            }
            let (neu, _) = self
                .signal
                .wait_timeout(g, Duration::from_millis(50))
                .unwrap_or_else(|x| x.into_inner());
            g = neu;
        }
    }

    pub(super) fn schliessen(&self) {
        let offen = {
            let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
            g.1 = true;
            std::mem::take(&mut g.0)
        };
        for eintrag in offen {
            if let Some(antwort) = eintrag.geschrieben {
                let _ = antwort.send(false);
            }
        }
        self.signal.notify_all();
    }

    /// Die Hochwassermarke eines Objektschluessels (NAK-246 M-22 bis M-24).
    #[cfg(test)]
    pub(super) fn hochwasser(&self, objekt_schluessel: &str) -> Option<i64> {
        self.inhalt
            .lock()
            .unwrap_or_else(|x| x.into_inner())
            .2
            .get(objekt_schluessel)
            .copied()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Schluessel, Marke und Frame der Eintraege in Queue-Reihenfolge.
    fn belegung(ausgang: &Ausgang) -> Vec<(String, i64, Vec<u8>)> {
        ausgang
            .inhalt
            .lock()
            .unwrap_or_else(|x| x.into_inner())
            .0
            .iter()
            .map(|eintrag| {
                let schluessel = match &eintrag.art {
                    Ausgangsart::Snapshot(s) | Ausgangsart::Messframe(s) => s.clone(),
                    Ausgangsart::P0 => "P0".to_owned(),
                };
                (schluessel, eintrag.marke, eintrag.frame.clone())
            })
            .collect()
    }

    fn eintrag(schluessel: &str, marke: i64, frame: &[u8]) -> (String, i64, Vec<u8>) {
        (schluessel.to_owned(), marke, frame.to_vec())
    }

    /// Der Writer entnimmt den naechsten Eintrag und meldet ihn als
    /// geschrieben; zurueck kommen Marke und Frame.
    fn schreiben(ausgang: &Ausgang) -> (i64, Vec<u8>) {
        let eintrag = ausgang.entnehmen().unwrap();
        eintrag.geschrieben.unwrap().send(true).unwrap();
        (eintrag.marke, eintrag.frame)
    }

    fn verworfen(ausgang: &Ausgang) -> u64 {
        ausgang.nachzuegler_verworfen.load(Ordering::SeqCst)
    }

    /// NAK-246 M-28 (Regressionswache): zwei Vollsnapshots UND zwei
    /// Ruecknahmen desselben Links koaleszieren weiter - innerhalb eines
    /// Schluessels ersetzt die juengere Marke die aeltere.
    #[test]
    fn writerqueue_snapshot_koalesziert_nach_objektschluessel() {
        for schluessel in ["session_snapshot", "evidence_invalidate"] {
            let ausgang = Ausgang::neu();
            let alt = ausgang
                .snapshot_einreihen_mit_antwort(schluessel, 1, b"alt".to_vec())
                .unwrap();
            let neu = ausgang
                .snapshot_einreihen_mit_antwort(schluessel, 2, b"neu".to_vec())
                .unwrap();
            assert!(!alt.recv_timeout(Duration::from_secs(1)).unwrap(), "{schluessel}");
            let eintrag = ausgang.entnehmen().unwrap();
            assert_eq!(eintrag.frame, b"neu", "{schluessel}");
            eintrag.geschrieben.unwrap().send(true).unwrap();
            assert!(neu.recv_timeout(Duration::from_secs(1)).unwrap(), "{schluessel}");
        }
    }

    /// NAK-246 M-21 (R-D5): ein aelterer Nachzuegler ersetzt keinen neueren
    /// Snapshot. Er wird verworfen, gezaehlt und bekommt `false`; der Eintrag
    /// mit Marke 12 bleibt an seiner Position. Gleiche Marke 12 und groessere
    /// Marke 13 ersetzen ihn an Ort und Stelle, der Ersetzte bekommt `false`.
    #[test]
    fn aelterer_nachzuegler_ersetzt_keinen_neueren_snapshot() {
        let ausgang = Ausgang::neu();
        let ruecknahme = ausgang
            .snapshot_einreihen_mit_antwort("evidence_invalidate", 5, b"ruecknahme".to_vec())
            .unwrap();
        let zwoelf = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", 12, b"zwoelf".to_vec())
            .unwrap();
        let angebot = ausgang
            .snapshot_einreihen_mit_antwort("proposal:m21", 7, b"angebot".to_vec())
            .unwrap();

        let zehn = ausgang.snapshot_einreihen_mit_antwort("session_snapshot", 10, b"zehn".to_vec());
        assert!(
            zehn.is_none(),
            "M-21: der aeltere Nachzuegler (Marke 10) wird nicht aufgenommen und bekommt false"
        );
        assert_eq!(
            belegung(&ausgang),
            vec![
                eintrag("evidence_invalidate", 5, b"ruecknahme"),
                eintrag("session_snapshot", 12, b"zwoelf"),
                eintrag("proposal:m21", 7, b"angebot"),
            ],
            "M-21: 10 ersetzt 12 nicht; 12 bleibt an seiner Position"
        );
        assert_eq!(verworfen(&ausgang), 1, "M-21: der Nachzuegler ist gezaehlt");
        assert!(zwoelf.try_recv().is_err(), "M-21: 12 ist nicht verdraengt");

        // Gleiche Marke ist kein Nachzuegler (R-M2-1): sie ersetzt 12 an Ort und Stelle.
        let zwoelf_gleich = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", 12, b"zwoelf-gleich".to_vec())
            .expect("M-21: gleiche Marke 12 wird angenommen");
        assert!(
            !zwoelf.recv_timeout(Duration::from_secs(1)).unwrap(),
            "M-21: der ersetzte Eintrag mit 12 bekommt false"
        );
        // Die groessere Marke 13 ersetzt ebenso an Ort und Stelle.
        let dreizehn = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", 13, b"dreizehn".to_vec())
            .expect("M-21: Marke 13 wird angenommen");
        assert!(
            !zwoelf_gleich.recv_timeout(Duration::from_secs(1)).unwrap(),
            "M-21: der ersetzte Eintrag mit 12 bekommt false"
        );
        assert_eq!(
            belegung(&ausgang),
            vec![
                eintrag("evidence_invalidate", 5, b"ruecknahme"),
                eintrag("session_snapshot", 13, b"dreizehn"),
                eintrag("proposal:m21", 7, b"angebot"),
            ],
            "M-21: 12 und 13 ersetzen an Ort und Stelle"
        );
        assert_eq!(verworfen(&ausgang), 1, "M-21: Gleichheit und groessere Marke zaehlen nicht");
        assert_eq!(ausgang.hochwasser("session_snapshot"), Some(13));

        // Der Writer entnimmt in Einreihreihenfolge; jeder angenommene Aufrufer bekommt true.
        for antwort in [&ruecknahme, &dreizehn, &angebot] {
            schreiben(&ausgang);
            assert!(antwort.recv_timeout(Duration::from_secs(1)).unwrap());
        }
    }

    /// NAK-246 M-22 (§5.5 Feinheit 2): die Hochwassermarke ueberlebt die
    /// Entnahme. Nach dem GESCHRIEBENEN 12 wird 10 auch in die leere Queue
    /// nicht aufgenommen.
    #[test]
    fn hochwassermarke_ueberlebt_die_entnahme() {
        let ausgang = Ausgang::neu();
        let zwoelf = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", 12, b"zwoelf".to_vec())
            .unwrap();
        assert_eq!(schreiben(&ausgang), (12, b"zwoelf".to_vec()));
        assert!(zwoelf.recv_timeout(Duration::from_secs(1)).unwrap());
        assert!(belegung(&ausgang).is_empty(), "die Queue ist leer");

        let zehn = ausgang.snapshot_einreihen_mit_antwort("session_snapshot", 10, b"zehn".to_vec());
        assert!(
            zehn.is_none(),
            "M-22: 10 nach dem geschriebenen 12 wird auch in die leere Queue nicht aufgenommen (false)"
        );
        assert!(belegung(&ausgang).is_empty(), "M-22: nichts aufgenommen");
        assert_eq!(verworfen(&ausgang), 1, "M-22: der Nachzuegler ist gezaehlt");
        assert_eq!(
            ausgang.hochwasser("session_snapshot"),
            Some(12),
            "M-22: das Hochwasser ueberlebt die Entnahme"
        );
    }

    /// NAK-246 M-22, zweiter Schritt (R-M2-1, §5.5 Feinheit 2): gleiche Marke
    /// ist kein Nachzuegler. 12 nach dem geschriebenen 12 wird angenommen,
    /// entnommen und geschrieben; der Zaehler bleibt, wo ihn der Nachzuegler 10
    /// liess.
    #[test]
    fn gleiche_marke_ist_kein_nachzuegler() {
        let ausgang = Ausgang::neu();
        let zwoelf = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", 12, b"zwoelf".to_vec())
            .unwrap();
        assert_eq!(schreiben(&ausgang), (12, b"zwoelf".to_vec()));
        assert!(zwoelf.recv_timeout(Duration::from_secs(1)).unwrap());
        assert!(ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", 10, b"zehn".to_vec())
            .is_none());
        assert_eq!(verworfen(&ausgang), 1);

        let wieder = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", 12, b"zwoelf-wieder".to_vec());
        assert!(
            wieder.is_some(),
            "M-22: gleiche Marke 12 nach dem geschriebenen 12 wird angenommen - Gleichheit ist kein Nachzuegler"
        );
        assert_eq!(verworfen(&ausgang), 1, "M-22: der Zaehler bleibt unveraendert");
        assert_eq!(
            schreiben(&ausgang),
            (12, b"zwoelf-wieder".to_vec()),
            "M-22: der Eintrag wird entnommen"
        );
        assert!(
            wieder.unwrap().recv_timeout(Duration::from_secs(1)).unwrap(),
            "M-22: und geschrieben"
        );
    }

    /// NAK-246 M-24 (§5.5 Feinheit 1, dritter Fall): ein Snapshot ohne eigenes
    /// Ordinal traegt das Hochwasser seines Schluessels - ohne weiteren Commit
    /// genau die Marke des zuletzt angenommenen - und ist nie ein Nachzuegler.
    /// Er senkt das Hochwasser nicht: ein aelterer Commit bleibt danach ein
    /// Nachzuegler, ein juengerer folgt ungehindert.
    #[test]
    fn snapshot_ohne_ordinal_traegt_das_hochwasser_seines_schluessels() {
        let ausgang = Ausgang::neu();
        let leer = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", MARKE_OHNE_ORDINAL, b"live-leer".to_vec())
            .expect("ohne Hochwasser wird der Livestand angenommen");
        assert_eq!(schreiben(&ausgang), (0, b"live-leer".to_vec()), "ohne Hochwasser traegt er 0");
        assert!(leer.recv_timeout(Duration::from_secs(1)).unwrap());

        let zwoelf = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", 12, b"zwoelf".to_vec())
            .unwrap();
        assert_eq!(schreiben(&ausgang), (12, b"zwoelf".to_vec()));
        assert!(zwoelf.recv_timeout(Duration::from_secs(1)).unwrap());

        let live = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", MARKE_OHNE_ORDINAL, b"live".to_vec());
        assert!(
            live.is_some(),
            "M-24: der Livestand ohne Ordinal wird nach dem geschriebenen 12 angenommen"
        );
        assert_eq!(
            schreiben(&ausgang),
            (12, b"live".to_vec()),
            "M-24: er traegt das Hochwasser 12"
        );
        assert!(live.unwrap().recv_timeout(Duration::from_secs(1)).unwrap());
        assert_eq!(verworfen(&ausgang), 0, "M-24: er ist kein Nachzuegler");
        assert_eq!(ausgang.hochwasser("session_snapshot"), Some(12));

        assert!(
            ausgang
                .snapshot_einreihen_mit_antwort("session_snapshot", 11, b"elf".to_vec())
                .is_none(),
            "das Hochwasser ist nicht gesunken: 11 bleibt ein Nachzuegler"
        );
        assert!(ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", 13, b"dreizehn".to_vec())
            .is_some());
        assert_eq!(verworfen(&ausgang), 1);
    }

    #[test]
    fn writerqueue_p0_ueberholt_snapshot_ohne_snapshotverlust() {
        let ausgang = Ausgang::neu();
        let snapshot = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", 1, b"snapshot".to_vec())
            .unwrap();
        assert!(ausgang.einreihen(b"p0".to_vec()));
        let p0 = ausgang.entnehmen().unwrap();
        assert_eq!(p0.art, Ausgangsart::P0);
        assert_eq!(p0.frame, b"p0");
        let p1 = ausgang.entnehmen().unwrap();
        assert_eq!(p1.frame, b"snapshot");
        p1.geschrieben.unwrap().send(true).unwrap();
        assert!(snapshot.recv_timeout(Duration::from_secs(1)).unwrap());
    }

    #[test]
    fn writerqueue_messframe_ersetzt_nur_dieselbe_quelle() {
        let ausgang = Ausgang::neu();
        assert!(ausgang.messframe_einreihen("quelle-a", b"a-alt".to_vec()));
        assert!(ausgang.messframe_einreihen("quelle-b", b"b".to_vec()));
        assert!(ausgang.messframe_einreihen("quelle-a", b"a-neu".to_vec()));
        let a = ausgang.entnehmen().unwrap();
        let b = ausgang.entnehmen().unwrap();
        assert_eq!(a.art, Ausgangsart::Messframe("quelle-a".into()));
        assert_eq!(a.frame, b"a-neu");
        assert_eq!(b.art, Ausgangsart::Messframe("quelle-b".into()));
        assert_eq!(b.frame, b"b");
    }

    #[test]
    fn writerqueue_cap_und_cap_plus_eins() {
        let ausgang = Ausgang::neu();
        for i in 0..CAP_WRITER {
            assert!(ausgang.einreihen(vec![(i & 0xff) as u8]));
        }
        assert!(!ausgang.einreihen(b"cap-plus-eins".to_vec()));
    }
}
