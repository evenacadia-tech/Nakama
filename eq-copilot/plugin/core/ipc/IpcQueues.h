// SONDE-010 — die client-seitigen Backpressure-Politiken aus Entwurf §53.9.
//
// | Queue            | Startcap | Ueberlaufpolitik                                    |
// |------------------|---------:|-----------------------------------------------------|
// | P0 Control       |       64 | nichts verwerfen; Verbindung schliessen              |
// | P1 Zustand       |      128 | Snapshots nach Objektschluessel koaleszieren;        |
// |                  |          | nicht koaleszierbare Ereignisse ueber Reconnect wdh. |
// | P2 Live je Sonde |        2 | aeltesten ungesendeten Frame ersetzen                |
//
// Die vierte Politik der Tabelle — Broker-Ingress 256, P2 zuerst droppen,
// P0-Ueberlauf trennt den Client — sitzt im Broker
// (broker/src/transport/warteschlange.rs) und hat hier bewusst kein
// Gegenstueck: ein Client, der seinen eigenen Ingress verwaltet, wuerde eine
// Entscheidung nachbilden, die der Server trifft.
//
// Drei Typen statt eines mit Politik-Enum, weil sie verschiedene Dinge
// BEDEUTEN: ein P0-Ueberlauf ist ein Verbindungsende, ein P2-Ueberlauf ein
// normaler Betriebszustand. Ein gemeinsamer `bool` haette beide zu derselben
// Frage gemacht — und damit den Unterschied verwischt, um den es im Gate-Text
// geht ("ohne P0-Starvation").
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <string>
#include <vector>

namespace nakama::ipc
{

inline constexpr std::size_t kCapP0        = 64;
inline constexpr std::size_t kCapP1        = 128;
inline constexpr std::size_t kCapP2JeSonde = 2;

/// P0-Steuerqueue: feste Kapazitaet, nichts wird verworfen. `einreihen`
/// liefert false ⇒ die Verbindung ist zu schliessen (§53.9).
class P0Warteschlange
{
public:
    explicit P0Warteschlange (std::size_t kapazitaet = kCapP0) : kap (kapazitaet) {}

    bool einreihen (std::string nachricht)
    {
        if (inhalt.size() >= kap)
        {
            ++ueberlaeufe;
            return false;
        }
        inhalt.push_back (std::move (nachricht));
        return true;
    }

    bool entnehmen (std::string& ziel)
    {
        if (inhalt.empty())
            return false;
        ziel = std::move (inhalt.front());
        inhalt.pop_front();
        return true;
    }

    /// Der Sender hat entnommen, aber NICHT geschrieben. "Nichts verwerfen"
    /// gilt auch fuer den Weg zwischen Queue und Pipe: der Eintrag geht an
    /// seinen Platz zurueck. `false` heisst, dass inzwischen kein Platz mehr
    /// ist — dann ist es ein Ueberlauf wie jeder andere.
    bool zuruecklegen (std::string nachricht)
    {
        if (inhalt.size() >= kap)
        {
            ++ueberlaeufe;
            return false;
        }
        inhalt.push_front (std::move (nachricht));
        return true;
    }

    std::size_t groesse()     const noexcept { return inhalt.size(); }
    std::size_t kapazitaet()  const noexcept { return kap; }
    std::uint64_t ueberlauf() const noexcept { return ueberlaeufe; }

private:
    std::size_t kap;
    std::deque<std::string> inhalt;
    std::uint64_t ueberlaeufe = 0;
};

/// Was mit einer P1-Nachricht passiert ist.
enum class P1Ergebnis
{
    eingereiht,
    /// Ein Snapshot desselben Objektschluessels wurde ERSETZT; die Position in
    /// der Reihenfolge bleibt, nur der Inhalt ist der neuere.
    koalesziert,
    /// Kein Platz und nicht koaleszierbar: liegt jetzt im Wiederholpuffer und
    /// geht nach dem naechsten Reconnect erneut raus.
    zurWiederholung,
    /// Auch der Wiederholpuffer ist voll — die aelteste Wiederholung faellt,
    /// gezaehlt, nie still.
    wiederholungVerdraengt,
};

/// P1-Queue: Snapshots koaleszieren nach Objektschluessel, Ereignisse nicht.
/// Der Wiederholpuffer ist die CLIENT-Haelfte des Outbox-Gedankens aus §53.9;
/// die Outbox im Broker ist SONDE-011.
class P1Warteschlange
{
public:
    explicit P1Warteschlange (std::size_t kapazitaet = kCapP1,
                              std::size_t wiederholKapazitaet = kCapP1)
        : kap (kapazitaet), wkap (wiederholKapazitaet) {}

    /// Leerer `schluessel` = Ereignis (wird nie ueberschrieben).
    P1Ergebnis einreihen (const std::string& schluessel, std::string nachricht)
    {
        if (! schluessel.empty())
            for (auto& e : inhalt)
                if (e.schluessel == schluessel)
                {
                    e.nachricht = std::move (nachricht);
                    return P1Ergebnis::koalesziert;
                }

        if (inhalt.size() < kap)
        {
            inhalt.push_back ({ schluessel, std::move (nachricht) });
            return P1Ergebnis::eingereiht;
        }
        if (wiederholung.size() >= wkap)
        {
            wiederholung.pop_front();
            wiederholung.push_back (std::move (nachricht));
            ++verdraengt;
            return P1Ergebnis::wiederholungVerdraengt;
        }
        wiederholung.push_back (std::move (nachricht));
        return P1Ergebnis::zurWiederholung;
    }

    bool entnehmen (std::string& ziel)
    {
        std::string schluessel;
        return entnehmen (schluessel, ziel);
    }

    /// Entnimmt MIT Objektschluessel. Nur wer den Schluessel kennt, kann den
    /// Eintrag nach einem gescheiterten Write wieder zuruecklegen, ohne aus
    /// einem Snapshot ein Ereignis zu machen.
    bool entnehmen (std::string& schluessel, std::string& ziel)
    {
        if (inhalt.empty())
            return false;
        schluessel = std::move (inhalt.front().schluessel);
        ziel = std::move (inhalt.front().nachricht);
        inhalt.pop_front();
        return true;
    }

    /// Gegenstueck zu `entnehmen`: der Sender hat den Eintrag NICHT auf die
    /// Pipe bekommen. Ohne diesen Weg verschwand ein nicht koaleszierbares
    /// P1-Ereignis endgueltig, sobald der Broker zwischen Entnahme und Write
    /// schloss — der Reconnect-Vertrag aus §53.9 kannte nur Ueberlaeufe
    /// (T2-Befund 4 vom 2026-08-29).
    ///
    /// Ein SNAPSHOT geht nur zurueck, wenn kein neuerer desselben Objekts
    /// wartet: der neuere ist die Wahrheit, der alte waere ein Rueckschritt.
    /// Ein EREIGNIS geht an seinen Platz zurueck oder — wenn dort kein Platz
    /// mehr ist — in den Wiederholpuffer, aus dem der naechste Reconnect es
    /// holt.
    P1Ergebnis zuruecklegen (const std::string& schluessel, std::string nachricht)
    {
        if (! schluessel.empty())
        {
            for (auto& e : inhalt)
                if (e.schluessel == schluessel)
                    return P1Ergebnis::koalesziert;  // der neuere steht schon da
            if (inhalt.size() < kap)
            {
                inhalt.push_front ({ schluessel, std::move (nachricht) });
                return P1Ergebnis::eingereiht;
            }
        }
        else if (inhalt.size() < kap)
        {
            inhalt.push_front ({ std::string(), std::move (nachricht) });
            return P1Ergebnis::eingereiht;
        }

        if (wiederholung.size() >= wkap)
        {
            wiederholung.pop_front();
            wiederholung.push_back (std::move (nachricht));
            ++verdraengt;
            return P1Ergebnis::wiederholungVerdraengt;
        }
        wiederholung.push_back (std::move (nachricht));
        return P1Ergebnis::zurWiederholung;
    }

    /// Nach einem Reconnect: die vorgehaltenen Ereignisse wandern VOR den
    /// laufenden Verkehr zurueck in die Queue, soweit Platz ist.
    std::size_t nachReconnectWiederholen()
    {
        std::size_t zurueck = 0;
        while (inhalt.size() < kap && ! wiederholung.empty())
        {
            inhalt.push_front ({ std::string(), std::move (wiederholung.back()) });
            wiederholung.pop_back();
            ++zurueck;
        }
        return zurueck;
    }

    std::size_t groesse()        const noexcept { return inhalt.size(); }
    std::size_t wiederholungen() const noexcept { return wiederholung.size(); }
    std::uint64_t verdraengte()  const noexcept { return verdraengt; }

private:
    struct Eintrag { std::string schluessel; std::string nachricht; };

    std::size_t kap, wkap;
    std::deque<Eintrag> inhalt;
    std::deque<std::string> wiederholung;
    std::uint64_t verdraengt = 0;
};

/// P2-Schleuse: vorallokierte SPSC-Struktur zwischen dem erzeugenden Worker
/// und dem Telemetriethread (§48.1 "Audio → Worker ausschliesslich ueber
/// vorallokierte SPSC-Strukturen"; §53.9 "P2 je Sonde Cap 2, aeltesten
/// ungesendeten Frame ersetzen").
///
/// KEINE Allokation, KEIN Lock, KEIN Warten auf der Erzeugerseite. Drei Slots
/// tragen zwei wartende Frames plus den, in den gerade geschrieben wird.
///
/// ── Warum jeder Slot einen BESITZER hat ───────────────────────────────────
///
/// Bei "replace-oldest" laeuft der Erzeuger nach einem Umlauf wieder auf den
/// Platz, aus dem der Verbraucher gerade kopiert. Die erste Fassung liess das
/// zu und erkannte es HINTERHER an einer Sequenz je Slot: der Verbraucher las
/// sie vor und nach dem Kopieren und verwarf den Frame, wenn sie sich geaendert
/// hatte. Das verhindert einen zerrissenen Frame auf dem Draht — aber es
/// beseitigt das Datenrennen nicht: zwei Threads schrieben und lasen dieselben
/// nicht-atomaren `bytes`/`laenge`, und das ist in C++ undefiniertes Verhalten,
/// gleich was danach geprueft wird (T2-Befund 1 vom 2026-08-29).
///
/// Deshalb hat jeder Slot jetzt ein `beansprucht` mit genau drei Zustaenden:
/// 0 = frei, 1 = der Verbraucher kopiert daraus, 2 = der Erzeuger schreibt
/// hinein. BEIDE Seiten holen sich den Platz mit einem
/// `compare_exchange_strong` — ein einziges Atomic entscheidet, wer ihn hat.
/// Ein beanspruchter Slot ist damit bis zur Freigabe unveraenderlich, und ein
/// Schreibversuch auf ihn findet nicht statt, sondern faellt gezaehlt aus.
/// Beides bleibt lockfrei und wartefrei: keine Seite schleift, beide gehen
/// weiter (der Erzeuger verwirft, der Verbraucher ueberspringt) — genau die
/// Freiheit, die die verlusttolerante P2-Politik ohnehin gewaehrt.
///
/// Die Sequenz je Slot bleibt, aber in ihrer zweiten Rolle: sie sagt, WELCHE
/// Folgenummer im Platz liegt. Der Verbraucher nimmt den Inhalt nur an, wenn
/// es die ist, die er erwartet.
template <std::size_t SlotBytes = 8192>
class P2Schleuse
{
public:
    static constexpr std::size_t kSlots = kCapP2JeSonde + 1;

    /// Besitzzustaende eines Slots.
    static constexpr std::uint32_t kFrei        = 0;
    static constexpr std::uint32_t kVerbraucher = 1;
    static constexpr std::uint32_t kErzeuger    = 2;

    P2Schleuse()
    {
        for (auto& s : slots) { s.folge.store (0); s.beansprucht.store (kFrei); s.laenge = 0; }
    }

    /// Erzeugerseite. `false` heisst: dieser Frame wurde NICHT uebernommen —
    /// entweder ist er groesser als ein Slot, oder sein Platz gehoerte gerade
    /// dem Verbraucher. Beides wird gezaehlt, nie halb geschrieben.
    bool veroeffentlichen (const std::uint8_t* daten, std::size_t laenge) noexcept
    {
        if (laenge > SlotBytes)
        {
            zuGross.fetch_add (1, std::memory_order_relaxed);
            return false;
        }
        const std::uint64_t p = schreib.load (std::memory_order_relaxed);
        Slot& s = slots[static_cast<std::size_t> (p % kSlots)];

        // Zuerst den Platz holen — VOR jeder Buchfuehrung. Wer den Boden
        // schon angehoben haette und dann nicht schreiben kann, haette einen
        // Frame zweimal verloren.
        std::uint32_t erwartet = kFrei;
        if (! s.beansprucht.compare_exchange_strong (erwartet, kErzeuger,
                                                     std::memory_order_acq_rel,
                                                     std::memory_order_relaxed))
        {
            // Der Verbraucher kopiert gerade daraus. Ein beanspruchter Slot ist
            // unveraenderlich; dieser Frame faellt, sichtbar gezaehlt.
            beanspruchtVerworfenZaehler.fetch_add (1, std::memory_order_relaxed);
            return false;
        }

        // Der aelteste NOCH WARTENDE Frame ist der spaetere von "was der
        // Verbraucher schon geholt hat" und "was der Erzeuger schon verworfen
        // hat". Ohne den ersten Teil zaehlte jede Veroeffentlichung nach den
        // ersten beiden als Ersetzung, auch wenn der Verbraucher laengst
        // leergeraeumt hat — die Zahl waere eine Luege ueber den Rueckstau
        // (gemessen am Lastbein: 133 056 "Ersetzungen" bei 133 120
        // Veroeffentlichungen und 92 857 wirklich gesendeten Frames).
        const std::uint64_t geholt = verbraucht.load (std::memory_order_acquire);
        const std::uint64_t unten = boden.load (std::memory_order_acquire);
        const std::uint64_t c = geholt > unten ? geholt : unten;
        if (p - c >= kCapP2JeSonde)
        {
            // Voll: der AELTESTE ungesendete Frame weicht.
            boden.store (c + 1, std::memory_order_release);
            ersetzt.fetch_add (1, std::memory_order_relaxed);
        }
        std::memcpy (s.bytes, daten, laenge);
        s.laenge = laenge;
        s.folge.store (p + 1, std::memory_order_release);  // 0 = nie beschrieben
        s.beansprucht.store (kFrei, std::memory_order_release);
        schreib.store (p + 1, std::memory_order_release);
        return true;
    }

    /// Verbraucherseite. Kopiert in `ziel` und liefert die Laenge, oder 0,
    /// wenn nichts (mehr) anliegt.
    std::size_t abholen (std::uint8_t* ziel, std::size_t zielGroesse) noexcept
    {
        for (;;)
        {
            const std::uint64_t p = schreib.load (std::memory_order_acquire);
            std::uint64_t c = lese;
            const std::uint64_t b = boden.load (std::memory_order_acquire);
            if (c < b)
                c = b;               // der Erzeuger hat Aelteres verworfen
            if (c >= p)
            {
                lese = c;
                verbraucht.store (c, std::memory_order_release);
                return 0;
            }
            Slot& s = slots[static_cast<std::size_t> (c % kSlots)];

            std::uint32_t erwartet = kFrei;
            if (! s.beansprucht.compare_exchange_strong (erwartet, kVerbraucher,
                                                         std::memory_order_acq_rel,
                                                         std::memory_order_relaxed))
            {
                // Der Erzeuger besitzt den Platz: er gehoert schon dem
                // naechsten Frame, der alte ist weg.
                lese = c + 1;
                verbraucht.store (lese, std::memory_order_release);
                continue;
            }

            // Ab hier gehoert der Platz UNS. Kein zweiter Thread beruehrt
            // `folge`, `laenge` oder `bytes`, bis wir ihn freigeben.
            const std::uint64_t folge = s.folge.load (std::memory_order_acquire);
            const std::size_t laenge = s.laenge;
            const bool gueltig = (folge == c + 1) && laenge <= zielGroesse;
            if (gueltig)
                std::memcpy (ziel, s.bytes, laenge);
            s.beansprucht.store (kFrei, std::memory_order_release);

            lese = c + 1;
            verbraucht.store (lese, std::memory_order_release);
            if (! gueltig)
                continue;            // schon ersetzt oder passt nicht ins Ziel
            return laenge;
        }
    }

    /// Obere Schranke der verworfenen Frames, keine exakte Zahl. Der Erzeuger
    /// entscheidet "voll" aus zwei getrennt gelesenen Atomics; raeumt der
    /// Verbraucher genau dazwischen ab, zaehlt der Erzeuger eine Ersetzung,
    /// die der Verbraucher noch abholen konnte. Gemessen am Lastbein liegt der
    /// Ueberschuss bei etwa einem Promille (133 024 veroeffentlicht, 93 030
    /// gesendet, 40 131 gezaehlte Ersetzungen). Die Richtung ist die
    /// vorsichtige: die Zahl behauptet nie WENIGER Verlust als eingetreten ist.
    std::uint64_t ersetzteFrames() const noexcept { return ersetzt.load (std::memory_order_relaxed); }
    std::uint64_t zuGrosseFrames() const noexcept { return zuGross.load (std::memory_order_relaxed); }

    /// Wie oft der Erzeuger einen Platz vorfand, den der Verbraucher gerade
    /// beansprucht hatte — und ihn deshalb NICHT beschrieben hat. Die Zahl ist
    /// der laufende Beleg fuer die Invariante: der Fall tritt unter Flut ein,
    /// und er endet jedes Mal ohne Schreibzugriff.
    std::uint64_t beanspruchtVerworfen() const noexcept
    {
        return beanspruchtVerworfenZaehler.load (std::memory_order_relaxed);
    }
    std::size_t   slotGroesse()    const noexcept { return SlotBytes; }

private:
    struct Slot
    {
        /// Folgenummer PLUS EINS des Frames, der hier liegt; 0 = nie
        /// beschrieben.
        std::atomic<std::uint64_t> folge { 0 };
        /// 0 frei · 1 der Verbraucher kopiert · 2 der Erzeuger schreibt.
        std::atomic<std::uint32_t> beansprucht { 0 };
        std::size_t   laenge = 0;
        std::uint8_t  bytes[SlotBytes] {};
    };

    Slot slots[kSlots] {};
    std::atomic<std::uint64_t> schreib { 0 };
    std::atomic<std::uint64_t> boden   { 0 };
    /// Fortschritt des Verbrauchers, damit der Erzeuger "voll" von
    /// "leergeraeumt" unterscheiden kann.
    std::atomic<std::uint64_t> verbraucht { 0 };
    std::atomic<std::uint64_t> ersetzt { 0 };
    std::atomic<std::uint64_t> zuGross { 0 };
    std::atomic<std::uint64_t> beanspruchtVerworfenZaehler { 0 };
    std::uint64_t lese = 0;   // gehoert allein dem Verbraucher
};

} // namespace nakama::ipc
