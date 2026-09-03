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
/// H-11: Rueckstau wird in Slots UND Bytes gemessen. 256 Slots sagen nichts
/// ueber den belegten Speicher: ein Peer, der 256 grosse Frames einreiht,
/// bleibt unter der Slotgrenze und belegt trotzdem beliebig viel. Das Budget
/// ist `CAP_INGRESS` mal die groesste zulaessige Payload aus dem Envelope-
/// Vertrag - mehr kann eine volle Slotqueue ohnehin nie halten, weniger waere
/// eine zweite, strengere Politik als die abgenommene aus Entwurf Paragraph
/// 53.9.
pub const CAP_INGRESS_BYTES: usize = CAP_INGRESS * crate::transport::v3::MAX_PAYLOAD_BYTES as usize;

/// H-11: was ein eingereihter Wert an Rueckstaubytes kostet.
///
/// Ohne diesen Trait muesste `einreihen` eine Bytezahl als dritten Parameter
/// nehmen - eine geaenderte oeffentliche Signatur, die Paragraph 4 nicht
/// vorsieht. So bleibt die Signatur, und die Queue misst selbst.
pub trait Rahmengroesse {
    fn rueckstau_bytes(&self) -> usize;
}

impl Rahmengroesse for (Familie, u8, Vec<u8>) {
    fn rueckstau_bytes(&self) -> usize {
        self.2.len()
    }
}

impl Rahmengroesse for u32 {
    /// Nur fuer die Inline-Tests der Slotpolitik: dort zaehlt die Slotachse,
    /// und ein Wert kostet ein Byte, damit das Bytebudget nie zuerst greift.
    fn rueckstau_bytes(&self) -> usize {
        1
    }
}

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
    /// Wiederholpuffer und fliesst wieder ab, sobald die Hauptqueue Platz hat
    /// (Matrix `A-P1-04`, `A-P1-06`).
    ZurWiederholung,
    /// Hauptqueue UND Wiederholpuffer voll: der NEUZUGANG wird abgewiesen,
    /// gezaehlt, nie still. Was einmal angenommen wurde, bleibt angenommen
    /// (Matrix `A-P1-05`, Regel 3 des Konvergenzentscheids).
    Abgewiesen,
}

/// P1-Queue: Snapshots koaleszieren nach Objektschluessel, Ereignisse nicht.
///
/// Der Wiederholpuffer ist die CLIENT-Haelfte des Outbox-Gedankens aus §53.9.
/// Die eigentliche Outbox im Broker ist SONDE-011; hier liegt nur, was ueber
/// einen Rueckstau hinweg erneut gesendet werden muss.
///
/// ── Warum der Wiederholpuffer SCHLUESSEL traegt (Matrix `A-P1-03`) ─────────
///
/// Die erste Fassung legte nur die Nachricht ab. Damit verlor ein Snapshot im
/// Wiederholpuffer seine Objektidentitaet: ein neuerer Snapshot desselben
/// Objekts fand nichts zum Koaleszieren, wurde abgewiesen, und nach dem
/// Abfluss erschien der ALTE als schluessellsoses Ereignis. §53.9 sagt
/// „Snapshots nach Objektschluessel koaleszieren" ohne Ausnahme fuer
/// Zwischenpuffer — der Schluessel ueberlebt deshalb jeden Puffer.
///
/// ── Warum ein voller Wiederholpuffer den NEUZUGANG abweist (Regel 3) ───────
///
/// Die erste Fassung machte mit `pop_front()` Platz und loeschte damit ein
/// bereits ANGENOMMENES Ereignis — bei Kapazitaet 2/2 und den Ereignissen 1…5
/// genau die Nr. 3. §53.9 verlangt fuer P1 „nicht koaleszierbare Events bei
/// Ueberlauf ueber Reconnect/Outbox WIEDERHOLEN", nicht verdraengen. Eine
/// Annahme, die spaeter still zurueckgenommen wird, ist keine Annahme. Die
/// C++-Haelfte hat das seit NAK-92 so; hier war es offen (NAK-95, Befund 3).
#[derive(Debug)]
pub struct P1Warteschlange<T> {
    kapazitaet: usize,
    wiederhol_kapazitaet: usize,
    inhalt: VecDeque<(Option<String>, T)>,
    wiederholung: VecDeque<(Option<String>, T)>,
    abgewiesene: u64,
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
            abgewiesene: 0,
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

    /// Wie viele NEUE Nachrichten abgewiesen wurden, weil Hauptqueue UND
    /// Wiederholpuffer voll waren. Angenommenes wird nie mitgezaehlt, weil
    /// Angenommenes nie faellt.
    pub fn abgewiesene(&self) -> u64 {
        self.abgewiesene
    }

    /// Abfluss des Wiederholpuffers OHNE Reconnect (Matrix `A-P1-06`,
    /// Regel 1). Er laeuft an jeder Stelle, an der Platz entsteht, und an
    /// jedem `einreihen` VOR dem Urteil ueber den Neuzugang — damit gilt die
    /// Annahmereihenfolge ueber beide Puffer hinweg und ein Neuzugang
    /// ueberholt nie eine bereits angenommene Wiederholung.
    ///
    /// Die aelteste Wiederholung zuerst, ans ENDE der Hauptqueue: alles, was
    /// dort steht, wurde vor ihr angenommen, alles Spaetere kann wegen
    /// `A-P1-07` gar nicht dort stehen.
    fn abfliessen(&mut self) -> usize {
        let mut zurueck = 0usize;
        while self.inhalt.len() < self.kapazitaet {
            match self.wiederholung.pop_front() {
                Some(e) => {
                    self.inhalt.push_back(e);
                    zurueck += 1;
                }
                None => break,
            }
        }
        zurueck
    }

    /// `schluessel = Some(...)` markiert einen Snapshot: er ersetzt einen
    /// aelteren Snapshot desselben Objekts — auch einen im Wiederholpuffer.
    /// `None` ist ein Ereignis und wird nie ueberschrieben.
    pub fn einreihen(&mut self, schluessel: Option<String>, wert: T) -> P1Ergebnis {
        if let Some(ref s) = schluessel {
            if let Some(platz) = self.inhalt.iter_mut().find(|(k, _)| k.as_deref() == Some(s)) {
                platz.1 = wert;
                return P1Ergebnis::Koalesziert;
            }
            // `A-P1-03`: derselbe Schluessel im Wiederholpuffer koalesziert
            // DORT, an seiner Position. Ohne diesen Zweig waere der neuere
            // Snapshot abgewiesen worden, waehrend der aeltere vorgehalten
            // bleibt — die Umkehrung der Zusage aus §53.9.
            if let Some(platz) = self
                .wiederholung
                .iter_mut()
                .find(|(k, _)| k.as_deref() == Some(s))
            {
                platz.1 = wert;
                return P1Ergebnis::Koalesziert;
            }
        }
        self.abfliessen(); // `A-P1-07`: Wiederholungen vor jedem Neuzugang
        if self.inhalt.len() < self.kapazitaet {
            self.inhalt.push_back((schluessel, wert));
            return P1Ergebnis::Eingereiht;
        }
        // Voll und nicht koaleszierbar.
        if self.wiederholung.len() >= self.wiederhol_kapazitaet {
            // Voll heisst: abweisen. Kein `pop_front()` — das loeschte ein
            // bereits angenommenes Ereignis (Regel 3).
            self.abgewiesene += 1;
            return P1Ergebnis::Abgewiesen;
        }
        self.wiederholung.push_back((schluessel, wert));
        P1Ergebnis::ZurWiederholung
    }

    pub fn entnehmen(&mut self) -> Option<T> {
        let e = self.inhalt.pop_front();
        self.abfliessen(); // `A-P1-06`: der frei gewordene Platz wird sofort genutzt
        e.map(|(_, w)| w)
    }

    /// Nach einem Reconnect: derselbe Abfluss wie im laufenden Betrieb
    /// (Matrix `A-P1-11` ist der Sonderfall von `A-P1-06`). Normalerweise ist
    /// der Puffer hier schon leer; die Funktion bleibt, weil der Reconnect der
    /// vom Entwurf ausdruecklich genannte Wiederholweg ist.
    pub fn nach_reconnect_wiederholen(&mut self) -> usize {
        self.abfliessen()
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
    /// P0 ODER P1 laeuft ueber und es gab kein P2 zum Verwerfen ⇒ Client
    /// trennen (Matrix `A-IN-03`, `A-IN-04`).
    ClientTrennen,
    /// P2 laeuft ueber und es gab kein P2 zum Verwerfen ⇒ dieser Frame
    /// faellt. P2 ist per Vertrag verlusttolerant (§33.1).
    Verworfen,
}

/// Broker-Ingress je Verbindung: 256 Plaetze fuer alle Familien gemeinsam.
/// P2 wird zuerst gedroppt; laeuft P0 ueber, wird der Client getrennt.
#[derive(Debug)]
pub struct IngressWarteschlange<T> {
    kapazitaet: usize,
    /// H-11: die zweite Achse. `usize::MAX` heisst „kein Bytebudget" und ist
    /// die Voreinstellung fuer Queues, die ausdruecklich nur Slots messen.
    bytebudget: usize,
    belegte_bytes: usize,
    inhalt: VecDeque<(Familie, T)>,
    p2_verworfen: u64,
    p1_ueberlauf_trennt: u64,
}

impl<T: Rahmengroesse> IngressWarteschlange<T> {
    pub fn neu() -> Self {
        let mut q = Self::mit_kapazitaet(CAP_INGRESS);
        q.bytebudget = CAP_INGRESS_BYTES;
        q
    }

    pub fn mit_kapazitaet(kapazitaet: usize) -> Self {
        Self {
            kapazitaet,
            bytebudget: usize::MAX,
            belegte_bytes: 0,
            inhalt: VecDeque::with_capacity(kapazitaet),
            p2_verworfen: 0,
            p1_ueberlauf_trennt: 0,
        }
    }

    /// H-11-Naht und Produktionsweg zugleich: eine Queue mit ausdruecklichem
    /// Bytebudget. Der Test misst am Budget und am Budget plus eins.
    pub fn mit_kapazitaet_und_bytebudget(kapazitaet: usize, bytebudget: usize) -> Self {
        let mut q = Self::mit_kapazitaet(kapazitaet);
        q.bytebudget = bytebudget;
        q
    }

    /// Wie viele Bytes der Rueckstau gerade belegt.
    pub fn belegte_bytes(&self) -> usize {
        self.belegte_bytes
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

    /// Wie oft ein P1-Ueberlauf ohne P2 zum Verwerfen die Verbindung getrennt
    /// hat (Matrix `A-IN-04`). Ein stilles Verwerfen bei gesund wirkender
    /// Verbindung gibt es nicht: der Sender koennte sonst nicht wiederholen.
    pub fn p1_ueberlauf_trennt(&self) -> u64 {
        self.p1_ueberlauf_trennt
    }

    fn aeltesten_p2_verwerfen(&mut self) -> bool {
        if let Some(pos) = self.inhalt.iter().position(|(f, _)| *f == Familie::P2) {
            if let Some((_, wert)) = self.inhalt.remove(pos) {
                self.belegte_bytes = self.belegte_bytes.saturating_sub(wert.rueckstau_bytes());
            }
            self.p2_verworfen += 1;
            return true;
        }
        false
    }

    pub fn einreihen(&mut self, familie: Familie, wert: T) -> IngressErgebnis {
        // H-11: Rueckstau hat ZWEI Achsen. Ueberschreitet das Bytebudget, wirkt
        // das genau wie Slot-voll und in derselben Reihenfolge aus Entwurf
        // Paragraph 53.9: erst den aeltesten P2 verwerfen, dann bei P0 oder P1
        // den Client trennen. P1 faellt nie still.
        let kosten = wert.rueckstau_bytes();
        let platz_in_slots = self.inhalt.len() < self.kapazitaet;
        let platz_in_bytes = self.belegte_bytes.saturating_add(kosten) <= self.bytebudget;
        if platz_in_slots && platz_in_bytes {
            self.belegte_bytes = self.belegte_bytes.saturating_add(kosten);
            self.inhalt.push_back((familie, wert));
            return IngressErgebnis::Eingereiht;
        }
        // Ein einzelner Frame, der allein schon groesser als das ganze Budget
        // ist, kann durch kein Verwerfen Platz bekommen; er faellt sofort in
        // die Politik unten.
        if kosten <= self.bytebudget && self.aeltesten_p2_verwerfen() {
            // Ein einziges Verwerfen genuegt nicht immer: das Bytebudget kann
            // mehrere aeltere P2 kosten. Solange P2 da ist und der Platz nicht
            // reicht, wird weiter verworfen.
            while self.belegte_bytes.saturating_add(kosten) > self.bytebudget
                && self.aeltesten_p2_verwerfen()
            {}
            if self.inhalt.len() < self.kapazitaet
                && self.belegte_bytes.saturating_add(kosten) <= self.bytebudget
            {
                self.belegte_bytes = self.belegte_bytes.saturating_add(kosten);
                self.inhalt.push_back((familie, wert));
                return IngressErgebnis::P2Verworfen;
            }
        }
        match familie {
            Familie::P0 => IngressErgebnis::ClientTrennen,
            // `A-IN-04`: P1 faellt NICHT still. §53.9 sagt fuer P1
            // „ueber Reconnect/Outbox wiederholen"; ein Verwerfen bei offener,
            // gesund wirkender Verbindung nimmt dem Sender genau diesen Weg.
            // Die Ingresszeile von §53.9 schweigt zu P1, deshalb gilt hier die
            // Regel des Dirigenten: trennen wie bei P0.
            Familie::P1 => {
                self.p1_ueberlauf_trennt += 1;
                IngressErgebnis::ClientTrennen
            }
            Familie::P2 => {
                self.p2_verworfen += 1;
                IngressErgebnis::Verworfen
            }
        }
    }

    pub fn entnehmen(&mut self) -> Option<(Familie, T)> {
        let raus = self.inhalt.pop_front();
        if let Some((_, wert)) = &raus {
            self.belegte_bytes = self.belegte_bytes.saturating_sub(wert.rueckstau_bytes());
        }
        raus
    }

    /// Nur P0, aelteste zuerst. Der Listener bedient P0 auf einem EIGENEN
    /// Verbraucherthread (Matrix `C-LS-07`): eine Senke, die in `p1` steht,
    /// darf die Antwort auf einen bereits eingereihten P0-Frame nicht
    /// aufhalten — genau das ist die Rust-Haelfte von „ohne P0-Starvation".
    pub fn entnehmen_p0(&mut self) -> Option<(Familie, T)> {
        let pos = self.inhalt.iter().position(|(f, _)| *f == Familie::P0)?;
        let raus = self.inhalt.remove(pos);
        if let Some((_, wert)) = &raus {
            self.belegte_bytes = self.belegte_bytes.saturating_sub(wert.rueckstau_bytes());
        }
        raus
    }

    /// Alles ausser P0, aelteste zuerst. Gegenstueck zu `entnehmen_p0`.
    pub fn entnehmen_ohne_p0(&mut self) -> Option<(Familie, T)> {
        let pos = self.inhalt.iter().position(|(f, _)| *f != Familie::P0)?;
        let raus = self.inhalt.remove(pos);
        if let Some((_, wert)) = &raus {
            self.belegte_bytes = self.belegte_bytes.saturating_sub(wert.rueckstau_bytes());
        }
        raus
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

    /// Regel 3 (Matrix `A-P1-05`): voll heisst abweisen des NEUZUGANGS, nie
    /// Loeschen von Akzeptiertem. Die alte Fassung schrieb hier den Verlust
    /// der Nr. 3 fest — der Test kodifizierte den Fehler (NAK-95, Befund 3).
    #[test]
    fn p1_haelt_ereignisse_fuer_den_reconnect_vor() {
        let mut q: P1Warteschlange<u32> = P1Warteschlange::mit_kapazitaet(2, 2);
        assert_eq!(q.einreihen(None, 1), P1Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(None, 2), P1Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(None, 3), P1Ergebnis::ZurWiederholung);
        assert_eq!(q.einreihen(None, 4), P1Ergebnis::ZurWiederholung);
        assert_eq!(q.einreihen(None, 5), P1Ergebnis::Abgewiesen);
        assert_eq!(q.abgewiesene(), 1);
        assert_eq!(q.wiederholungen(), 2, "3 und 4 bleiben angenommen");

        // Senden: jeder frei werdende Platz zieht sofort eine Wiederholung
        // nach (`A-P1-06`) — ein Reconnect ist dafuer nicht noetig.
        assert_eq!(q.entnehmen(), Some(1));
        assert_eq!(q.entnehmen(), Some(2));
        assert_eq!(q.wiederholungen(), 0, "der Puffer floss ohne Reconnect ab");
        assert_eq!(q.nach_reconnect_wiederholen(), 0);
        assert_eq!(q.entnehmen(), Some(3), "JEDES angenommene Ereignis kommt an");
        assert_eq!(q.entnehmen(), Some(4));
        assert_eq!(q.entnehmen(), None);
    }

    /// Matrix `A-P1-06` / Regel 1: der Wiederholpuffer hat einen Abflussweg
    /// OHNE Reconnect. „Nur beim Verbindungsaufbau leeren" ist ein Befund.
    #[test]
    fn p1_wiederholpuffer_fliesst_ohne_reconnect_ab() {
        let mut q: P1Warteschlange<u32> = P1Warteschlange::mit_kapazitaet(1, 4);
        assert_eq!(q.einreihen(None, 1), P1Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(None, 2), P1Ergebnis::ZurWiederholung);
        assert_eq!(q.einreihen(None, 3), P1Ergebnis::ZurWiederholung);
        assert_eq!(q.wiederholungen(), 2);

        // Ein einziges Senden — ohne jeden Reconnect.
        assert_eq!(q.entnehmen(), Some(1));
        assert_eq!(q.wiederholungen(), 1, "genau eine Wiederholung ist nachgerueckt");
        assert_eq!(q.len(), 1);
        assert_eq!(q.entnehmen(), Some(2), "und zwar die AELTESTE zuerst");
        assert_eq!(q.entnehmen(), Some(3));
        assert_eq!(q.wiederholungen(), 0);
    }

    /// Matrix `A-P1-07` / Regel 1: ein Neuzugang ueberholt nie eine bereits
    /// angenommene Wiederholung. Die Annahmereihenfolge gilt ueber beide
    /// Puffer hinweg.
    #[test]
    fn p1_neuzugang_ueberholt_keine_wiederholung() {
        let mut q: P1Warteschlange<u32> = P1Warteschlange::mit_kapazitaet(1, 4);
        assert_eq!(q.einreihen(None, 1), P1Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(None, 2), P1Ergebnis::ZurWiederholung);
        assert_eq!(q.entnehmen(), Some(1)); // Platz frei, 2 rueckt nach
        // Jetzt kommt ein NEUES Ereignis. Es darf sich nicht vor die 2 setzen.
        assert_eq!(q.einreihen(None, 3), P1Ergebnis::ZurWiederholung);
        assert_eq!(q.entnehmen(), Some(2));
        assert_eq!(q.entnehmen(), Some(3));
        assert_eq!(q.entnehmen(), None);
    }

    /// Matrix `A-P1-03` / Regel 2: der Objektschluessel ueberlebt den
    /// Wiederholpuffer, und Koaleszierung gilt auch dort.
    #[test]
    fn p1_wiederholpuffer_haelt_den_schluessel() {
        let mut q: P1Warteschlange<&str> = P1Warteschlange::mit_kapazitaet(1, 2);
        // Ein fremder Eintrag haelt die Hauptqueue belegt.
        assert_eq!(q.einreihen(None, "fremd"), P1Ergebnis::Eingereiht);
        assert_eq!(q.einreihen(Some("k".into()), "alt"), P1Ergebnis::ZurWiederholung);
        // Der NEUERE Snapshot desselben Objekts koalesziert im Puffer, statt
        // abgewiesen zu werden.
        assert_eq!(q.einreihen(Some("k".into()), "neu"), P1Ergebnis::Koalesziert);
        assert_eq!(q.wiederholungen(), 1, "Koaleszieren waechst den Puffer nicht");
        assert_eq!(q.abgewiesene(), 0, "Koaleszieren ist kein Ueberlauf");

        assert_eq!(q.entnehmen(), Some("fremd"));
        assert_eq!(q.entnehmen(), Some("neu"), "der aeltere ist ersetzt, nicht der neuere");
        assert_eq!(q.entnehmen(), None);
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
    }

    /// Matrix `A-IN-04`: P1 faellt bei vollem Ingress ohne P2 NICHT still —
    /// die Verbindung wird getrennt wie bei P0. Ein stiller Verlust bei
    /// gesund wirkender Verbindung naehme dem Sender den Wiederholweg aus
    /// §53.9 („ueber Reconnect/Outbox wiederholen").
    #[test]
    fn ingress_voll_ohne_p2_trennt_auch_bei_p1() {
        let mut q: IngressWarteschlange<u32> = IngressWarteschlange::mit_kapazitaet(2);
        assert_eq!(q.einreihen(Familie::P1, 1), IngressErgebnis::Eingereiht);
        assert_eq!(q.einreihen(Familie::P0, 2), IngressErgebnis::Eingereiht);
        // Voll, kein P2 zum Verwerfen: P1 trennt, genau wie P0.
        assert_eq!(q.einreihen(Familie::P1, 3), IngressErgebnis::ClientTrennen);
        assert_eq!(q.p1_ueberlauf_trennt(), 1);
        assert_eq!(q.einreihen(Familie::P0, 4), IngressErgebnis::ClientTrennen);
        // Und nichts Angenommenes ist dabei verschwunden.
        assert_eq!(q.len(), 2);
        assert_eq!(q.entnehmen(), Some((Familie::P1, 1)));
        assert_eq!(q.entnehmen(), Some((Familie::P0, 2)));
    }

    /// Matrix `C-LS-07`: der Listener holt P0 auf einem eigenen Thread ab.
    /// Beide Sichten entnehmen aelteste zuerst und stehlen einander nichts.
    #[test]
    fn ingress_liefert_p0_getrennt_vom_rest() {
        let mut q: IngressWarteschlange<u32> = IngressWarteschlange::mit_kapazitaet(8);
        q.einreihen(Familie::P1, 1);
        q.einreihen(Familie::P2, 2);
        q.einreihen(Familie::P0, 3);
        q.einreihen(Familie::P1, 4);
        q.einreihen(Familie::P0, 5);
        assert_eq!(q.entnehmen_p0(), Some((Familie::P0, 3)));
        assert_eq!(q.entnehmen_ohne_p0(), Some((Familie::P1, 1)));
        assert_eq!(q.entnehmen_p0(), Some((Familie::P0, 5)));
        assert_eq!(q.entnehmen_p0(), None);
        assert_eq!(q.entnehmen_ohne_p0(), Some((Familie::P2, 2)));
        assert_eq!(q.entnehmen_ohne_p0(), Some((Familie::P1, 4)));
        assert_eq!(q.entnehmen_ohne_p0(), None);
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
