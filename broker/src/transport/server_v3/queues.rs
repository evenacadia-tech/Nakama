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
pub(super) struct Ausgang {
    pub(super) inhalt: Mutex<(VecDeque<AusgangEintrag>, bool)>,
    pub(super) signal: Condvar,
}

#[derive(Debug, PartialEq, Eq)]
pub(super) enum Ausgangsart {
    P0,
    Snapshot(String),
    Messframe(String),
}

pub(super) struct AusgangEintrag {
    pub(super) art: Ausgangsart,
    pub(super) frame: Vec<u8>,
    pub(super) geschrieben: Option<std::sync::mpsc::SyncSender<bool>>,
}

impl Ausgang {
    pub(super) fn neu() -> Self {
        Self {
            inhalt: Mutex::new((VecDeque::with_capacity(16), false)),
            signal: Condvar::new(),
        }
    }

    /// `false` = die Queue ist voll oder geschlossen. Voll heisst: der Peer
    /// holt seine Antworten nicht ab.
    pub(super) fn einreihen(&self, frame: Vec<u8>) -> bool {
        self.einreihen_eintrag(AusgangEintrag {
            art: Ausgangsart::P0,
            frame,
            geschrieben: None,
        })
    }

    pub(super) fn snapshot_einreihen_mit_antwort(
        &self,
        objekt_schluessel: &str,
        frame: Vec<u8>,
    ) -> Option<std::sync::mpsc::Receiver<bool>> {
        let (tx, rx) = std::sync::mpsc::sync_channel(1);
        self.einreihen_eintrag(AusgangEintrag {
            art: Ausgangsart::Snapshot(objekt_schluessel.to_owned()),
            frame,
            geschrieben: Some(tx),
        })
        .then_some(rx)
    }

    pub(super) fn messframe_einreihen(&self, instance_id: &str, frame: Vec<u8>) -> bool {
        self.einreihen_eintrag(AusgangEintrag {
            art: Ausgangsart::Messframe(instance_id.to_owned()),
            frame,
            geschrieben: None,
        })
    }

    pub(super) fn einreihen_eintrag(&self, eintrag: AusgangEintrag) -> bool {
        let mut ersetzt = None;
        let ok = {
            let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
            if g.1 {
                false
            } else if let Ausgangsart::Snapshot(objekt_schluessel) = &eintrag.art {
                if let Some(position) = g.0.iter().position(|alt| {
                    matches!(&alt.art, Ausgangsart::Snapshot(alt_schluessel)
                        if alt_schluessel == objekt_schluessel)
                }) {
                    ersetzt = Some(std::mem::replace(&mut g.0[position], eintrag));
                    true
                } else if g.0.len() >= CAP_WRITER {
                    false
                } else {
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
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn writerqueue_snapshot_koalesziert_nach_objektschluessel() {
        let ausgang = Ausgang::neu();
        let alt = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", b"alt".to_vec())
            .unwrap();
        let neu = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", b"neu".to_vec())
            .unwrap();
        assert!(!alt.recv_timeout(Duration::from_secs(1)).unwrap());
        let eintrag = ausgang.entnehmen().unwrap();
        assert_eq!(eintrag.frame, b"neu");
        eintrag.geschrieben.unwrap().send(true).unwrap();
        assert!(neu.recv_timeout(Duration::from_secs(1)).unwrap());
    }

    #[test]
    fn writerqueue_p0_ueberholt_snapshot_ohne_snapshotverlust() {
        let ausgang = Ausgang::neu();
        let snapshot = ausgang
            .snapshot_einreihen_mit_antwort("session_snapshot", b"snapshot".to_vec())
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
