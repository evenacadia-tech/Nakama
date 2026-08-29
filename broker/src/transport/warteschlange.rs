//! SONDE-010 — die vier Backpressure-Politiken aus Entwurf §53.9.
//!
//! | Queue | Startcap | Ueberlaufpolitik |
//! |---|---:|---|
//! | P0 Control | 64 | nichts verwerfen; Verbindung schliessen |
//! | P1 Zustand/Evidenz | 128 | Snapshots nach Objektschluessel koaleszieren; nicht koaleszierbare Events ueber Reconnect wiederholen |
//! | P2 Live pro Sonde | 2 | aeltesten ungesendeten Frame ersetzen |
//! | Broker-Ingress pro Verbindung | 256 | P2 zuerst droppen; P0-Ueberlauf trennt den Client |
//!
//! Die vier Politiken sind bewusst VIER Typen und nicht ein Typ mit einem
//! Politik-Enum: sie haben verschiedene Rueckgabetypen, weil sie verschiedene
//! Dinge bedeuten. Ein P0-Ueberlauf ist ein Verbindungsende, ein P2-Ueberlauf
//! ein normaler Betriebszustand. Ein gemeinsamer `bool` haette beide zu
//! derselben Frage gemacht — und damit den Unterschied verwischt, um den es
//! im Gate-Text geht ("ohne P0-Starvation").

use std::collections::VecDeque;

use crate::transport::v3::Familie;

pub const CAP_P0: usize = 64;
pub const CAP_P1: usize = 128;
pub const CAP_P2_JE_SONDE: usize = 2;
pub const CAP_INGRESS: usize = 256;

/// P0 laeuft ueber ⇒ die Verbindung ist zu schliessen. Es gibt keinen
/// stillen Verlust einer Steuernachricht.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct P0Ueberlauf;

/// P0-Steuerqueue: feste Kapazitaet, nichts wird verworfen.
#[derive(Debug)]
pub struct P0Warteschlange<T> {
    kapazitaet: usize,
    inhalt: VecDeque<T>,
}

impl<T> P0Warteschlange<T> {
    pub fn neu() -> Self {
        Self::mit_kapazitaet(CAP_P0)
    }

    pub fn mit_kapazitaet(kapazitaet: usize) -> Self {
        Self { kapazitaet, inhalt: VecDeque::with_capacity(kapazitaet) }
    }

    pub fn len(&self) -> usize {
        self.inhalt.len()
    }

    pub fn is_empty(&self) -> bool {
        self.inhalt.is_empty()
    }

    pub fn kapazitaet(&self) -> usize {
        self.kapazitaet
    }

    pub fn einreihen(&mut self, wert: T) -> Result<(), P0Ueberlauf> {
        if self.inhalt.len() >= self.kapazitaet {
            return Err(P0Ueberlauf);
        }
        self.inhalt.push_back(wert);
        Ok(())
    }

    pub fn entnehmen(&mut self) -> Option<T> {
        self.inhalt.pop_front()
    }
}

/// Was mit einer P1-Nachricht passiert ist.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum P1Ergebnis {
    /// Neu angehaengt.
    Eingereiht,
    /// Ein Snapshot desselben Objektschluessels wurde ERSETZT — die Position
    /// in der Reihenfolge bleibt, nur der Inhalt ist der neuere.
    Koalesziert,
    /// Kein Platz und nicht koaleszierbar: die Nachricht liegt jetzt im
    /// Wiederholpuffer und geht nach dem naechsten Reconnect erneut raus.
    ZurWiederholung,
    /// Auch der Wiederholpuffer ist voll — die aelteste Wiederholung faellt.
    /// Ehrlich gezaehlt, nie still.
    WiederholungVerdraengt,
}

/// P1-Queue: Snapshots koaleszieren nach Objektschluessel, Ereignisse nicht.
///
/// Der Wiederholpuffer ist die CLIENT-Haelfte des Outbox-Gedankens aus §53.9.
/// Die eigentliche Outbox im Broker ist SONDE-011; hier liegt nur, was ueber
/// einen Reconnect hinweg erneut gesendet werden muss.
#[derive(Debug)]
pub struct P1Warteschlange<T> {
    kapazitaet: usize,
    wiederhol_kapazitaet: usize,
    inhalt: VecDeque<(Option<String>, T)>,
    wiederholung: VecDeque<T>,
    verdraengte_wiederholungen: u64,
}

impl<T> P1Warteschlange<T> {
    pub fn neu() -> Self {
        Self::mit_kapazitaet(CAP_P1, CAP_P1)
    }

    pub fn mit_kapazitaet(kapazitaet: usize, wiederhol_kapazitaet: usize) -> Self {
        Self {
            kapazitaet,
            wiederhol_kapazitaet,
            inhalt: VecDeque::with_capacity(kapazitaet),
            wiederholung: VecDeque::new(),
            verdraengte_wiederholungen: 0,
        }
    }

    pub fn len(&self) -> usize {
        self.inhalt.len()
    }

    pub fn is_empty(&self) -> bool {
        self.inhalt.is_empty()
    }

    pub fn wiederholungen(&self) -> usize {
        self.wiederholung.len()
    }

    pub fn verdraengte_wiederholungen(&self) -> u64 {
        self.verdraengte_wiederholungen
    }

    /// `schluessel = Some(...)` markiert einen Snapshot: er ersetzt einen
    /// aelteren Snapshot desselben Objekts. `None` ist ein Ereignis und wird
    /// nie ueberschrieben.
    pub fn einreihen(&mut self, schluessel: Option<String>, wert: T) -> P1Ergebnis {
        if let Some(ref s) = schluessel {
            if let Some(platz) = self.inhalt.iter_mut().find(|(k, _)| k.as_deref() == Some(s)) {
                platz.1 = wert;
                return P1Ergebnis::Koalesziert;
            }
        }
        if self.inhalt.len() < self.kapazitaet {
            self.inhalt.push_back((schluessel, wert));
            return P1Ergebnis::Eingereiht;
        }
        // Voll und nicht koaleszierbar.
        if self.wiederholung.len() >= self.wiederhol_kapazitaet {
            self.wiederholung.pop_front();
            self.wiederholung.push_back(wert);
            self.verdraengte_wiederholungen += 1;
            return P1Ergebnis::WiederholungVerdraengt;
        }
        self.wiederholung.push_back(wert);
        P1Ergebnis::ZurWiederholung
    }

    pub fn entnehmen(&mut self) -> Option<T> {
        self.inhalt.pop_front().map(|(_, w)| w)
    }

    /// Nach einem Reconnect: die vorgehaltenen Ereignisse wandern VOR den
    /// laufenden Verkehr zurueck in die Queue, soweit Platz ist. Was nicht
    /// passt, bleibt im Wiederholpuffer und kommt beim naechsten Mal.
    pub fn nach_reconnect_wiederholen(&mut self) -> usize {
        let mut zurueck = 0usize;
        while self.inhalt.len() < self.kapazitaet {
            match self.wiederholung.pop_back() {
                Some(w) => {
                    self.inhalt.push_front((None, w));
                    zurueck += 1;
                }
                None => break,
            }
        }
        zurueck
    }
}

/// Was mit einem P2-Frame passiert ist.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum P2Ergebnis {
    Eingereiht,
    /// Der aelteste noch ungesendete Frame wurde ersetzt (§53.9
    /// "aeltesten ungesendeten Frame ersetzen").
    AeltestenErsetzt,
}

/// P2-Queue je Sonde: Kapazitaet 2, replace-oldest. Verlusttolerant per
/// Vertrag — ihr Ausfall degradiert Analyse, nie Steuerung.
#[derive(Debug)]
pub struct P2Warteschlange<T> {
    kapazitaet: usize,
    inhalt: VecDeque<T>,
    ersetzt: u64,
}

impl<T> P2Warteschlange<T> {
    pub fn neu() -> Self {
        Self::mit_kapazitaet(CAP_P2_JE_SONDE)
    }

    pub fn mit_kapazitaet(kapazitaet: usize) -> Self {
        assert!(kapazitaet >= 1, "eine P2-Queue mit Kapazitaet 0 kann nichts tragen");
        Self { kapazitaet, inhalt: VecDeque::with_capacity(kapazitaet), ersetzt: 0 }
    }

    pub fn len(&self) -> usize {
        self.inhalt.len()
    }

    pub fn is_empty(&self) -> bool {
        self.inhalt.is_empty()
    }

    pub fn ersetzt(&self) -> u64 {
        self.ersetzt
    }

    pub fn einreihen(&mut self, wert: T) -> P2Ergebnis {
        if self.inhalt.len() >= self.kapazitaet {
            self.inhalt.pop_front();
            self.inhalt.push_back(wert);
            self.ersetzt += 1;
            return P2Ergebnis::AeltestenErsetzt;
        }
        self.inhalt.push_back(wert);
        P2Ergebnis::Eingereiht
    }

    pub fn entnehmen(&mut self) -> Option<T> {
        self.inhalt.pop_front()
    }
}

/// Ergebnis einer Ingress-Einreihung im Broker.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum IngressErgebnis {
    Eingereiht,
    /// Platz geschaffen, indem der aelteste P2-Frame gefallen ist.
    P2Verworfen,
    /// P0 laeuft ueber und es gab kein P2 zum Verwerfen ⇒ Client trennen.
    ClientTrennen,
    /// P1/P2 laeuft ueber und es gab kein P2 zum Verwerfen ⇒ diese
    /// Nachricht faellt. P1 wird dabei gezaehlt, nie still verschluckt.
    Verworfen,
}

/// Broker-Ingress je Verbindung: 256 Plaetze fuer alle Familien gemeinsam.
/// P2 wird zuerst gedroppt; laeuft P0 ueber, wird der Client getrennt.
#[derive(Debug)]
pub struct IngressWarteschlange<T> {
    kapazitaet: usize,
    inhalt: VecDeque<(Familie, T)>,
    p2_verworfen: u64,
    p1_verworfen: u64,
}

impl<T> IngressWarteschlange<T> {
    pub fn neu() -> Self {
        Self::mit_kapazitaet(CAP_INGRESS)
    }

    pub fn mit_kapazitaet(kapazitaet: usize) -> Self {
        Self {
            kapazitaet,
            inhalt: VecDeque::with_capacity(kapazitaet),
            p2_verworfen: 0,
            p1_verworfen: 0,
        }
    }

    pub fn len(&self) -> usize {
        self.inhalt.len()
    }

    pub fn is_empty(&self) -> bool {
        self.inhalt.is_empty()
    }

    pub fn p2_verworfen(&self) -> u64 {
        self.p2_verworfen
    }

    pub fn p1_verworfen(&self) -> u64 {
        self.p1_verworfen
    }

    fn aeltesten_p2_verwerfen(&mut self) -> bool {
        if let Some(pos) = self.inhalt.iter().position(|(f, _)| *f == Familie::P2) {
            self.inhalt.remove(pos);
            self.p2_verworfen += 1;
            return true;
        }
        false
    }

    pub fn einreihen(&mut self, familie: Familie, wert: T) -> IngressErgebnis {
        if self.inhalt.len() < self.kapazitaet {
            self.inhalt.push_back((familie, wert));
            return IngressErgebnis::Eingereiht;
        }
        if self.aeltesten_p2_verwerfen() {
            self.inhalt.push_back((familie, wert));
            return IngressErgebnis::P2Verworfen;
        }
        match familie {
            Familie::P0 => IngressErgebnis::ClientTrennen,
            Familie::P1 => {
                self.p1_verworfen += 1;
                IngressErgebnis::Verworfen
            }
            Familie::P2 => {
                self.p2_verworfen += 1;
                IngressErgebnis::Verworfen
            }
        }
    }

    pub fn entnehmen(&mut self) -> Option<(Familie, T)> {
        self.inhalt.pop_front()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn p0_verwirft_nie_und_meldet_ueberlauf() {
        let mut q: P0Warteschlange<u32> = P0Warteschlange::neu();
        assert_eq!(q.kapazitaet(), 64);
        for i in 0..64 {
            assert_eq!(q.einreihen(i), Ok(()));
        }
        assert_eq!(q.einreihen(64), Err(P0Ueberlauf));
        // Und nichts ist dabei verloren gegangen: die 64 kommen der Reihe nach.
        for i in 0..64 {
            assert_eq!(q.entnehmen(), Some(i));
        }
        assert_eq!(q.entnehmen(), None);
    }

    #[test]
    fn p1_koalesziert_snapshots_an_ihrer_position() {
        let mut q: P1Warteschlange<&str> = P1Warteschlange::mit_kapazitaet(4, 4);
        assert_eq!(q.einreihen(Some("a".into()), "a1"), P1Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(None, "ereignis"), P1Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(Some("b".into()), "b1"), P1Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(Some("a".into()), "a2"), P1Ergebnis::Koalesziert);
        assert_eq!(q.len(), 3, "Koaleszieren waechst die Queue nicht");
        // Reihenfolge bleibt: a steht weiter vorn, aber mit neuem Inhalt.
        assert_eq!(q.entnehmen(), Some("a2"));
        assert_eq!(q.entnehmen(), Some("ereignis"));
        assert_eq!(q.entnehmen(), Some("b1"));
    }

    #[test]
    fn p1_haelt_ereignisse_fuer_den_reconnect_vor() {
        let mut q: P1Warteschlange<u32> = P1Warteschlange::mit_kapazitaet(2, 2);
        assert_eq!(q.einreihen(None, 1), P1Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(None, 2), P1Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(None, 3), P1Ergebnis::ZurWiederholung);
        assert_eq!(q.einreihen(None, 4), P1Ergebnis::ZurWiederholung);
        assert_eq!(q.einreihen(None, 5), P1Ergebnis::WiederholungVerdraengt);
        assert_eq!(q.verdraengte_wiederholungen(), 1);
        assert_eq!(q.wiederholungen(), 2);

        // Queue leeren (Verbindung hat gesendet), dann Reconnect.
        assert_eq!(q.entnehmen(), Some(1));
        assert_eq!(q.entnehmen(), Some(2));
        assert_eq!(q.nach_reconnect_wiederholen(), 2);
        assert_eq!(q.entnehmen(), Some(4));
        assert_eq!(q.entnehmen(), Some(5));
        assert_eq!(q.wiederholungen(), 0);
    }

    #[test]
    fn p2_ersetzt_den_aeltesten_ungesendeten() {
        let mut q: P2Warteschlange<u32> = P2Warteschlange::neu();
        assert_eq!(q.einreihen(1), P2Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(2), P2Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(3), P2Ergebnis::AeltestenErsetzt);
        assert_eq!(q.len(), 2);
        assert_eq!(q.entnehmen(), Some(2));
        assert_eq!(q.entnehmen(), Some(3));
        assert_eq!(q.ersetzt(), 1);
    }

    #[test]
    fn ingress_droppt_p2_zuerst_und_trennt_bei_p0() {
        let mut q: IngressWarteschlange<u32> = IngressWarteschlange::mit_kapazitaet(4);
        assert_eq!(q.einreihen(Familie::P2, 1), IngressErgebnis::Eingereiht);
        assert_eq!(q.einreihen(Familie::P0, 2), IngressErgebnis::Eingereiht);
        assert_eq!(q.einreihen(Familie::P2, 3), IngressErgebnis::Eingereiht);
        assert_eq!(q.einreihen(Familie::P1, 4), IngressErgebnis::Eingereiht);
        // Voll: der naechste P0 verdraengt den AELTESTEN P2, nicht sich selbst.
        assert_eq!(q.einreihen(Familie::P0, 5), IngressErgebnis::P2Verworfen);
        assert_eq!(q.p2_verworfen(), 1);
        assert_eq!(q.einreihen(Familie::P0, 6), IngressErgebnis::P2Verworfen);
        // Jetzt liegt kein P2 mehr drin — ein weiterer P0 trennt.
        assert_eq!(q.einreihen(Familie::P0, 7), IngressErgebnis::ClientTrennen);
        // P1 in derselben Lage faellt gezaehlt, ohne die Verbindung zu toeten.
        assert_eq!(q.einreihen(Familie::P1, 8), IngressErgebnis::Verworfen);
        assert_eq!(q.p1_verworfen(), 1);
    }

    /// Der Kern des Gate-Textes: eine P2-Flut darf P0 nicht aushungern.
    /// 10 000 Telemetrieframes gegen eine Ingressqueue — jeder P0 findet
    /// danach noch einen Platz, weil P2 zuerst weicht.
    #[test]
    fn p2_flut_hungert_p0_nicht_aus() {
        let mut q: IngressWarteschlange<u32> = IngressWarteschlange::neu();
        for i in 0..10_000u32 {
            q.einreihen(Familie::P2, i);
        }
        assert_eq!(q.len(), CAP_INGRESS);
        for i in 0..CAP_INGRESS as u32 {
            let e = q.einreihen(Familie::P0, 1_000_000 + i);
            assert!(
                matches!(e, IngressErgebnis::Eingereiht | IngressErgebnis::P2Verworfen),
                "P0 Nr. {i} wurde abgewiesen: {e:?}"
            );
        }
        // Alle 256 P0 liegen drin, kein einziger ging verloren.
        let mut p0 = 0;
        while let Some((f, _)) = q.entnehmen() {
            if f == Familie::P0 {
                p0 += 1;
            }
        }
        assert_eq!(p0, CAP_INGRESS);
    }
}
