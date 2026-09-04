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
///
/// ── Warum der entnommene Eintrag RESERVIERT bleibt ────────────────────────
///
/// Die erste Fassung nahm ihn beim `entnehmen` restlos aus der Kapazitaets-
/// rechnung. Blockierte danach der Write und liefen 64 neue Befehle ein, war
/// die Queue wieder voll — und `zuruecklegen` fand keinen Platz mehr. Sein
/// `false` wurde im Client ignoriert, der Befehl war weg, und der oeffentliche
/// Ueberlaufzaehler meldete den Verlust nicht (T2-Befund 1 vom 2026-08-29).
/// Das ist genau der Bruch der P0-Zusage "nichts verwerfen".
///
/// Deshalb zaehlt der entnommene Eintrag bis `bestaetigen()` oder
/// `zuruecklegen()` WEITER gegen die Kapazitaet. Die Invariante lautet
/// `inhalt.size() + reserviert <= kap`; damit hat das Zuruecklegen immer
/// Platz und kann gar nicht scheitern. Der Preis ist ein Platz weniger
/// waehrend eines laufenden Writes — und genau dieser Platz ist es, der
/// vorher still verlorenging.
class P0Warteschlange
{
public:
    explicit P0Warteschlange (std::size_t kapazitaet = kCapP0) : kap (kapazitaet) {}

    bool einreihen (std::string nachricht)
    {
        if (inhalt.size() + reserviert >= kap)
        {
            ++ueberlaeufe;
            return false;
        }
        inhalt.push_back (std::move (nachricht));
        return true;
    }

    /// Entnimmt UND reserviert. Der Aufrufer schuldet danach genau ein
    /// `bestaetigen()` (auf dem Draht) oder `zuruecklegen()` (nicht auf dem
    /// Draht).
    bool entnehmen (std::string& ziel)
    {
        if (inhalt.empty())
            return false;
        ziel = std::move (inhalt.front());
        inhalt.pop_front();
        ++reserviert;
        return true;
    }

    /// Der Eintrag ist auf dem Draht: der reservierte Platz wird frei.
    void bestaetigen() noexcept
    {
        if (reserviert > 0)
            --reserviert;
    }

    /// Der Sender hat entnommen, aber NICHT geschrieben. "Nichts verwerfen"
    /// gilt auch fuer den Weg zwischen Queue und Pipe: der Eintrag geht an
    /// seinen Platz zurueck. Dank der Reservierung ist dort Platz — dieser
    /// Weg hat keinen Fehlerfall.
    void zuruecklegen (std::string nachricht)
    {
        inhalt.push_front (std::move (nachricht));
        if (reserviert > 0)
            --reserviert;
    }

    std::size_t groesse()     const noexcept { return inhalt.size(); }
    /// Wie viele Eintraege gerade zwischen Queue und Pipe unterwegs sind.
    std::size_t inFlug()      const noexcept { return reserviert; }
    std::size_t kapazitaet()  const noexcept { return kap; }
    std::uint64_t ueberlauf() const noexcept { return ueberlaeufe; }

private:
    std::size_t kap;
    std::size_t reserviert = 0;
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
    /// fliesst wieder ab, sobald die Hauptqueue Platz hat (Matrix `A-P1-04`,
    /// `A-P1-06`) — ein Reconnect ist dafuer nicht noetig.
    zurWiederholung,
    /// Auch der Wiederholpuffer ist voll: die NEUE Nachricht wird abgewiesen,
    /// gezaehlt, nie still. Was einmal angenommen wurde, bleibt angenommen.
    abgewiesen,
    /// Groesser, als ein v3-Frame tragen kann. Sie wird an der TUER abgewiesen
    /// statt eingereiht: eine Nachricht, die nie auf den Draht passt, wuerde
    /// den Sender sonst bei jedem Verbindungsaufbau erneut scheitern lassen.
    zuGross,
};

/** Gilt eine P1-Nachricht mit diesem Ergebnis als UEBERGEBEN? (SONDE-013 M-05)

    Die Frage entscheidet, ob ein Sender seine Quelle leeren darf. Sie ist als
    Regel geschrieben und nicht als Bedingung an der Aufrufstelle, weil genau
    diese Unterscheidung schon einmal gefehlt hat: der Evidenzsender leerte
    seinen Ereignisring bedingungslos und loeschte damit Ereignisse, die er
    gerade NICHT transportiert hatte — ein stiller Verlust ohne Zaehler.

    `zurWiederholung` zaehlt als uebergeben: der Wiederholpuffer wirft
    ausdruecklich nichts weg („Was einmal angenommen wurde, bleibt
    angenommen", SONDE-010). `abgewiesen` und `zuGross` dagegen sind ein
    echter Verlust — dort bleibt die Quelle stehen. */
constexpr bool p1Uebergeben (P1Ergebnis e) noexcept
{
    return e == P1Ergebnis::eingereiht
        || e == P1Ergebnis::koalesziert
        || e == P1Ergebnis::zurWiederholung;
}

/// P1-Queue: Snapshots koaleszieren nach Objektschluessel, Ereignisse nicht.
/// Der Wiederholpuffer ist die CLIENT-Haelfte des Outbox-Gedankens aus §53.9;
/// die Outbox im Broker ist SONDE-011.
///
/// ── Warum ein voller Wiederholpuffer das NEUE Ereignis abweist ─────────────
///
/// Die erste Fassung machte im vollen Wiederholpuffer Platz, indem sie mit
/// `pop_front()` die AELTESTE Wiederholung wegwarf. Damit verschwand ein
/// bereits angenommenes, nicht koaleszierbares Ereignis — bei Kapazitaet 2/2
/// und den Ereignissen 1…5 genau die Nr. 3 (T2-Befund 1 Runde 3 vom
/// 2026-08-29). §53.9 sagt fuer P1 aber „nicht koaleszierbare Events bei
/// Ueberlauf ueber Reconnect/Outbox WIEDERHOLEN" — wiederholen, nicht
/// verdraengen. Eine Annahme, die spaeter still zurueckgenommen wird, ist
/// keine Annahme.
///
/// Deshalb faellt jetzt der NEUZUGANG (`P1Ergebnis::abgewiesen`, gezaehlt und
/// dem Aufrufer gemeldet), und alles, was schon angenommen ist, ueberlebt bis
/// zum Reconnect.
///
/// **Warum kein erzwungener Reconnect.** Naheliegend waere, den vollen
/// Wiederholpuffer als Verbindungsende zu behandeln — dann gingen die
/// vorgehaltenen Ereignisse sofort wieder raus. P1 teilt sich die
/// Control-Verbindung aber mit P0. Ein Reconnect wegen P1 risse damit die
/// P0-Steuerung mit, deren Zusage „nichts verwerfen" heisst; das Gate dieses
/// Tickets lautet ausdruecklich „ohne P0-Starvation". Der Rueckstau bleibt
/// deshalb lokal und sichtbar, statt die Steuerleitung mitzureissen.
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
        {
            for (auto& e : inhalt)
                if (e.schluessel == schluessel)
                {
                    e.nachricht = std::move (nachricht);
                    return P1Ergebnis::koalesziert;
                }
            // Matrix `A-P1-03`: derselbe Schluessel im WIEDERHOLPUFFER
            // koalesziert dort, an seiner Position. Die alte Fassung legte den
            // Snapshot ohne Schluessel ab; ein neuerer Snapshot desselben
            // Objekts fand nichts zum Koaleszieren, wurde abgewiesen, und nach
            // dem Abfluss erschien der AELTERE als Ereignis. Das kehrt die
            // Zusage aus §53.9 um (NAK-95, Befund 2).
            for (auto& e : wiederholung)
                if (e.schluessel == schluessel)
                {
                    e.nachricht = std::move (nachricht);
                    return P1Ergebnis::koalesziert;
                }
        }

        abfliessen();   // `A-P1-07`: Wiederholungen vor jedem Neuzugang

        if (inhalt.size() + reserviert < kap)
        {
            inhalt.push_back ({ schluessel, std::move (nachricht) });
            return P1Ergebnis::eingereiht;
        }
        if (wiederholung.size() >= wkap)
        {
            // Voll heisst: abweisen. Kein `pop_front()` — das loeschte ein
            // bereits angenommenes Ereignis (T2-Befund 1 Runde 3).
            ++abgewiesenZaehler;
            return P1Ergebnis::abgewiesen;
        }
        wiederholung.push_back ({ schluessel, std::move (nachricht) });
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
        ++reserviert;   // zaehlt bis bestaetigen()/zuruecklegen() gegen die Kapazitaet
        return true;
    }

    /// Der Eintrag ist auf dem Draht: der reservierte Platz wird frei — und
    /// genau dort fliesst der Wiederholpuffer nach (`A-P1-06`). `entnehmen`
    /// allein schafft keinen Platz: es RESERVIERT ihn.
    void bestaetigen() noexcept
    {
        if (reserviert > 0)
            --reserviert;
        abfliessen();
    }

    /// Gegenstueck zu `entnehmen`: der Sender hat den Eintrag NICHT auf die
    /// Pipe bekommen. Ohne diesen Weg verschwand ein nicht koaleszierbares
    /// P1-Ereignis endgueltig, sobald der Broker zwischen Entnahme und Write
    /// schloss — der Reconnect-Vertrag aus §53.9 kannte nur Ueberlaeufe
    /// (T2-Befund 4 vom 2026-08-29).
    ///
    /// Ein SNAPSHOT geht nur zurueck, wenn kein neuerer desselben Objekts
    /// wartet: der neuere ist die Wahrheit, der alte waere ein Rueckschritt.
    /// Ein EREIGNIS geht an seinen Platz zurueck — dank der Reservierung aus
    /// `entnehmen` ist dort immer Platz. Der frueher noetige Umweg ueber den
    /// Wiederholpuffer entfaellt damit; der Eintrag konnte auf diesem Weg auch
    /// nie verlorengehen (T2-Befund 1 vom 2026-08-29, P1-Haelfte).
    P1Ergebnis zuruecklegen (const std::string& schluessel, std::string nachricht)
    {
        if (! schluessel.empty())
        {
            for (auto& e : inhalt)
                if (e.schluessel == schluessel)
                {
                    if (reserviert > 0)
                        --reserviert;
                    abfliessen();
                    return P1Ergebnis::koalesziert;  // der neuere steht schon da
                }
            // Auch der Wiederholpuffer traegt jetzt Schluessel (`A-P1-03`);
            // ein neuerer Snapshot kann dort liegen.
            for (auto& e : wiederholung)
                if (e.schluessel == schluessel)
                {
                    if (reserviert > 0)
                        --reserviert;
                    abfliessen();
                    return P1Ergebnis::koalesziert;
                }
        }

        inhalt.push_front ({ schluessel, std::move (nachricht) });
        if (reserviert > 0)
            --reserviert;
        return P1Ergebnis::eingereiht;
    }

    /// Nach einem Reconnect: derselbe Abfluss wie im laufenden Betrieb
    /// (`A-P1-11` ist der Sonderfall von `A-P1-06`). Normalerweise ist der
    /// Puffer hier schon leer; die Funktion bleibt, weil der Reconnect der vom
    /// Entwurf ausdruecklich genannte Wiederholweg ist.
    std::size_t nachReconnectWiederholen()
    {
        return abfliessen();
    }

    std::size_t groesse()        const noexcept { return inhalt.size(); }
    /// Wie viele Eintraege gerade zwischen Queue und Pipe unterwegs sind.
    std::size_t inFlug()         const noexcept { return reserviert; }
    std::size_t wiederholungen() const noexcept { return wiederholung.size(); }
    /// Wie viele NEUE Nachrichten abgewiesen wurden, weil Queue UND
    /// Wiederholpuffer voll waren. Angenommenes wird nie mitgezaehlt, weil
    /// Angenommenes nie faellt.
    std::uint64_t abgewiesene()  const noexcept { return abgewiesenZaehler; }

private:
    struct Eintrag { std::string schluessel; std::string nachricht; };

    /// Abfluss des Wiederholpuffers OHNE Reconnect (Matrix `A-P1-06`,
    /// Regel 1). Er laeuft an jeder Stelle, an der Platz entsteht
    /// (`bestaetigen`, `zuruecklegen` mit Koaleszierung), und an jedem
    /// `einreihen` VOR dem Urteil ueber den Neuzugang (`A-P1-07`).
    ///
    /// Die AELTESTE Wiederholung zuerst, ans ENDE der Hauptqueue: alles, was
    /// dort steht, wurde vor ihr angenommen, und dank `A-P1-07` kann nichts
    /// Spaeteres dort stehen. Damit gilt die Annahmereihenfolge ueber beide
    /// Puffer hinweg.
    ///
    /// Vor der Ursachenrunde war `nachReconnectWiederholen()` der EINZIGE
    /// Abfluss: bei nur voruebergehendem Rueckstau blieben akzeptierte
    /// Ereignisse unbegrenzt liegen, waehrend spaetere sie ueberholten
    /// (NAK-95, Befund 1).
    std::size_t abfliessen()
    {
        std::size_t zurueck = 0;
        while (inhalt.size() + reserviert < kap && ! wiederholung.empty())
        {
            inhalt.push_back (std::move (wiederholung.front()));
            wiederholung.pop_front();
            ++zurueck;
        }
        return zurueck;
    }

    std::size_t kap, wkap;
    std::size_t reserviert = 0;
    std::deque<Eintrag> inhalt;
    std::deque<Eintrag> wiederholung;
    std::uint64_t abgewiesenZaehler = 0;
};

/// Kein Testhaken — die PRODUKTIONSFASSUNG des zweiten Templateparameters von
/// `P2Schleuse`.
///
/// Der Haken laeuft vor jedem Anspruchsversuch des Erzeugers. In der
/// Produktion ist er diese leere Struktur: der Aufruf hat keinen Rumpf, keinen
/// Zweig und keinen Zeiger und verschwindet restlos — der Erzeugerpfad bleibt
/// allokations-, lock- und wartefrei (§48.1). Ein Laufzeitzeiger haette
/// dagegen in JEDER Veroeffentlichung eine Pruefung gekostet, nur damit ein
/// Test etwas einhaengen kann.
struct KeinP2Haken
{
    template <class Schleuse>
    void vorAnspruch (Schleuse&, std::uint64_t) noexcept {}
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
///
/// ── Und warum die Kollision ein LOCH erzeugt, statt den Frame zu opfern ───
///
/// Die erste Fassung dieses Besitzmodells liess den Erzeuger bei einer
/// Kollision einfach `false` liefern: der gerade erzeugte, NEUESTE Frame fiel.
/// Das ist die Umkehrung der Politik. §53.9 sagt fuer P2 „aeltesten
/// ungesendeten Frame ersetzen" — der neueste ist der einzige, der niemals
/// fallen darf, denn er traegt den aktuellen Messwert (T2-Befund 2 Runde 3 vom
/// 2026-08-29).
///
/// Der Erzeuger ueberspringt die kollidierende Position deshalb: sie bleibt
/// ein LOCH, das nie beschrieben wird, und der Frame geht in den naechsten
/// Platz. Der Verbraucher erkennt das Loch daran, dass die Folgenummer im
/// Platz nicht die erwartete ist — genau die Pruefung, die es ohnehin schon
/// gibt —, und geht weiter. Dass am Ende IMMER ein Platz frei wird, ist keine
/// Zaehlfrage, sondern bewiesen: der Verbraucher raeumt nur vorwaerts und nur
/// unterhalb von `schreib`, sein Vorrat an Positionen ist also endlich (der
/// Beweis steht bei der Schleife in `veroeffentlichen`).
/// Der Verlust liegt damit wieder dort, wo die Politik ihn haben will, beim
/// aeltesten wartenden Frame.
template <std::size_t SlotBytes = 8192, class Haken = KeinP2Haken>
class P2Schleuse
{
public:
    static constexpr std::size_t kSlots = kCapP2JeSonde + 1;

    /// Der Testhaken (siehe `KeinP2Haken`). In der Produktion leer.
    Haken haken {};

    /// Besitzzustaende eines Slots.
    static constexpr std::uint32_t kFrei        = 0;
    static constexpr std::uint32_t kVerbraucher = 1;
    static constexpr std::uint32_t kErzeuger    = 2;

    /// So viele Fehlversuche OHNE jeden Fortschritt des Verbrauchers gelten
    /// als gebrochene SPSC-Zusage (Begruendung in `veroeffentlichen`). Ein
    /// lebender Verbraucher kann sie nicht erreichen: um kSlots Kandidaten
    /// nacheinander zu blockieren, muss er kSlots-1 mal freigeben und neu
    /// beanspruchen, und jede Freigabe meldet einen hoeheren Stand. Vier volle
    /// Ringe ohne eine einzige Meldung sind kein Zeitfenster mehr, sondern ein
    /// Stillstand.
    static constexpr std::size_t kOhneFortschrittMax = kSlots * 4;

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
        std::uint64_t p = schreib.load (std::memory_order_relaxed);
        Slot* s = nullptr;

        // Zuerst den Platz holen — VOR jeder Buchfuehrung. Wer den Boden
        // schon angehoben haette und dann nicht schreiben kann, haette einen
        // Frame zweimal verloren.
        //
        // ── Warum GARANTIERT, nicht "begrenzt versucht" (Matrix `A-P2-04`) ─
        //
        // Die erste Fassung versuchte `p` und dann `p+1`. Der Verbraucher
        // haelt zwar hoechstens EINEN Platz — aber er kann ihn zwischen den
        // beiden Versuchen wechseln: er gibt `p` frei und beansprucht `p+1`,
        // und beide Versuche scheitern. Dann fiel der NEUESTE Frame, also
        // genau die Umkehrung von "aeltesten ungesendeten Frame ersetzen"
        // (§53.9). Unter Baulast trat der Fall wirklich ein und riss B10 mit
        // "[2 neueste wegen fremden Anspruchs verworfen]" (NAK-98).
        //
        // Die zweite Fassung erhoehte auf `kSlots * 2` feste Versuche. Auch das
        // ist nur begrenztes Versuchen: haelt der Verbraucher einen Platz
        // lange, lassen die Kollisionsloecher das Fenster `schreib - boden`
        // wachsen; liegt er danach mindestens sechs Positionen zurueck, kann er
        // zwischen JEDEN zwei Versuchen den naechsten Kandidaten beanspruchen,
        // und alle sechs scheitern (NAK-104, Pruefbefund vom 2026-08-30).
        //
        // Deshalb steht hier keine Zahl mehr, sondern eine BEWIESEN endende
        // Schleife. Sie sperrt nicht, sie schlaeft nicht, sie allokiert nicht;
        // sie gewinnt je Runde nachweislich Boden:
        //
        //   1. Es gibt genau EINEN Verbraucher, und er haelt je Zeitpunkt
        //      hoechstens EINEN Platz (`abholen`: beanspruchen, kopieren,
        //      freigeben).
        //   2. Er beansprucht nur Positionen UNTERHALB von `schreib`; `schreib`
        //      bewegt allein der Erzeuger, und der steht waehrend dieses
        //      Aufrufs hier.
        //   3. Scheitert der Versuch an Position `p+k`, dann haelt der
        //      Verbraucher in diesem Augenblick eine Position `c` mit
        //      `c ≡ p+k (mod kSlots)` und `c < schreib`.
        //   4. Scheitert auch `p+k+1`, so ist die dann gehaltene Position
        //      `c' ≢ c (mod kSlots)`, also `c' != c`; und weil der Verbraucher
        //      ausschliesslich VORWAERTS raeumt (`lese = c + 1`), gilt
        //      `c' > c`.
        //   5. Die gehaltenen Positionen wachsen damit STRENG MONOTON und
        //      bleiben unter `schreib`. Der Vorrat ist endlich: nach hoechstens
        //      `schreib - Verbraucherstand` Fehlversuchen findet der
        //      Verbraucher nichts mehr, haelt nichts mehr, und der naechste
        //      Versuch gelingt.
        //
        // Im eingeschwungenen Betrieb ist dieser Vorrat `kCapP2JeSonde`, also
        // zwei — die Schleife endet praktisch beim ersten oder zweiten Versuch.
        //
        // Bricht die SPSC-Zusage (zwei Verbraucher, oder einer, der einen Platz
        // nie freigibt), traegt Punkt 5 nicht mehr. Dann darf hier nicht ewig
        // gedreht werden: bleibt der gemeldete Verbraucherstand ueber
        // `kOhneFortschrittMax` Fehlversuche hinweg stehen, ist bewiesen, dass
        // niemand Boden gewinnt — der Frame faellt gezaehlt, statt den
        // erzeugenden Worker festzuhalten. Die Zahl muss 0 bleiben; sie ist die
        // Wache ueber genau diese Zusage.
        //
        // Der Verbraucherstand wird ERST beim ersten Fehlversuch gelesen. Der
        // haeufige Weg — Platz frei, ein Versuch — kostet damit kein einziges
        // Atomic mehr als vorher.
        std::uint64_t standZuletzt = 0;
        std::size_t   ohneFortschritt = 0;
        for (;; ++p)
        {
            haken.vorAnspruch (*this, p);
            Slot* kandidat = &slots[static_cast<std::size_t> (p % kSlots)];
            std::uint32_t erwartet = kFrei;
            if (kandidat->beansprucht.compare_exchange_strong (erwartet, kErzeuger,
                                                               std::memory_order_acq_rel,
                                                               std::memory_order_relaxed))
            {
                s = kandidat;
                break;
            }
            // Der Verbraucher kopiert gerade daraus. Ein beanspruchter Slot
            // ist unveraenderlich — aber der NEUESTE Frame ist nicht der, der
            // dafuer faellt. Position `p` wird zum LOCH und der Frame geht in
            // den naechsten Platz; der Verbraucher ueberspringt das Loch an
            // der nicht passenden Folgenummer.
            kollisionsLoecherZaehler.fetch_add (1, std::memory_order_relaxed);

            const std::uint64_t stand = verbraucherstand();
            if (ohneFortschritt == 0 || stand != standZuletzt)
            {
                standZuletzt = stand;
                ohneFortschritt = 1;
                continue;
            }
            if (++ohneFortschritt < kOhneFortschrittMax)
                continue;
            // Unerreichbar, solange es genau EINEN Verbraucher gibt und er
            // jeden Platz wieder freigibt. Die Zahl ist der laufende Beleg
            // dafuer — sie muss 0 bleiben.
            beanspruchtVerworfenZaehler.fetch_add (1, std::memory_order_relaxed);
            return false;
        }

        // Der aelteste NOCH WARTENDE Frame ist der spaetere von "was der
        // Verbraucher schon geholt hat" und "was der Erzeuger schon verworfen
        // hat" — genau `verbraucherstand()`. Ohne den ersten Teil zaehlte jede
        // Veroeffentlichung nach den ersten beiden als Ersetzung, auch wenn der
        // Verbraucher laengst leergeraeumt hat — die Zahl waere eine Luege ueber
        // den Rueckstau (gemessen am Lastbein: 133 056 "Ersetzungen" bei
        // 133 120 Veroeffentlichungen und 92 857 wirklich gesendeten Frames).
        const std::uint64_t c = verbraucherstand();
        if (p - c >= kCapP2JeSonde)
        {
            // Voll: der AELTESTE ungesendete Frame weicht.
            boden.store (c + 1, std::memory_order_release);
            ersetzt.fetch_add (1, std::memory_order_relaxed);
        }
        std::memcpy (s->bytes, daten, laenge);
        s->laenge = laenge;
        s->folge.store (p + 1, std::memory_order_release);  // 0 = nie beschrieben
        s->beansprucht.store (kFrei, std::memory_order_release);
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
    /// beansprucht hatte — und die Position deshalb als LOCH uebersprungen
    /// hat. Die Zahl ist der laufende Beleg dafuer, dass der Fall unter Flut
    /// wirklich eintritt; ohne sie spraeche die Aussage unten ueber nichts.
    std::uint64_t kollisionsLoecher() const noexcept
    {
        return kollisionsLoecherZaehler.load (std::memory_order_relaxed);
    }

    /// Wie oft der Erzeuger GAR KEINEN Platz beanspruchen konnte und den
    /// neuen Frame verwerfen musste. Das setzt einen Verbraucher voraus, der
    /// ueber `kOhneFortschrittMax` Fehlversuche hinweg keinen Fortschritt
    /// meldet — also einen stehenden oder einen zweiten. Bei EINEM lebenden
    /// Verbraucher ist es unerreichbar; die Zahl muss 0 bleiben und ist damit
    /// die Wache ueber „der neueste Frame faellt nie wegen einer Kollision".
    std::uint64_t beanspruchtVerworfen() const noexcept
    {
        return beanspruchtVerworfenZaehler.load (std::memory_order_relaxed);
    }
    std::size_t   slotGroesse()    const noexcept { return SlotBytes; }

    /// ── Testhaken fuer die Slot-Kollision (Matrix `A-P2-04`) ─────────────
    ///
    /// Sie beanspruchen und geben einen Platz frei wie der Verbraucher es
    /// tut. Der Kollisionsfall wird damit DETERMINISTISCH erzwungen, statt
    /// unter Last erwartet: NAK-98 fiel genau deshalb nur sporadisch auf, und
    /// eine Probe, die vom Zufall der Taktung abhaengt, ist kein Beweis.
    ///
    /// Sie sind kein toter Code und keine Hintertuer: sie benutzen dasselbe
    /// Besitz-Atomic wie beide echten Seiten und koennen an der Politik nichts
    /// aendern — ein beanspruchter Platz ist fuer JEDEN unveraenderlich.
    bool testSlotBeanspruchen (std::uint64_t position) noexcept
    {
        std::uint32_t erwartet = kFrei;
        return slots[static_cast<std::size_t> (position % kSlots)]
            .beansprucht.compare_exchange_strong (erwartet, kVerbraucher,
                                                  std::memory_order_acq_rel,
                                                  std::memory_order_relaxed);
    }

    void testSlotFreigeben (std::uint64_t position) noexcept
    {
        slots[static_cast<std::size_t> (position % kSlots)]
            .beansprucht.store (kFrei, std::memory_order_release);
    }

    /// Naechste Schreibposition — der Test muss wissen, WELCHEN Platz der
    /// Erzeuger als naechstes nimmt, sonst beansprucht er den falschen.
    std::uint64_t testSchreibstand() const noexcept
    {
        return schreib.load (std::memory_order_acquire);
    }

    /// Aelteste Position, die der Verbraucher noch beanspruchen darf. Der
    /// wandernde Testverbraucher aus `A-P2-04` braucht sie, um dort
    /// anzufangen, wo ein echter Verbraucher anfinge.
    std::uint64_t testBodenstand() const noexcept
    {
        return boden.load (std::memory_order_acquire);
    }

    /// Fortschritt melden wie `abholen` es nach jeder Freigabe tut. Ohne diese
    /// Meldung waere der Testverbraucher kein wandernder Verbraucher, sondern
    /// ein STEHENDER — und ein stehender ist genau der Fall, den die Wache
    /// `beanspruchtVerworfen` abfangen soll.
    void testVerbrauchtMelden (std::uint64_t position) noexcept
    {
        verbraucht.store (position, std::memory_order_release);
    }

private:
    /// Aelteste Position, die noch auf den Verbraucher wartet: das spaetere
    /// von "schon geholt" und "schon verworfen". Beide Seiten der Buchfuehrung
    /// lesen dieselbe Groesse — zwei Fassungen davon liefen bisher
    /// auseinander.
    std::uint64_t verbraucherstand() const noexcept
    {
        const std::uint64_t geholt = verbraucht.load (std::memory_order_acquire);
        const std::uint64_t unten  = boden.load (std::memory_order_acquire);
        return geholt > unten ? geholt : unten;
    }

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
    std::atomic<std::uint64_t> kollisionsLoecherZaehler { 0 };
    std::atomic<std::uint64_t> beanspruchtVerworfenZaehler { 0 };
    std::uint64_t lese = 0;   // gehoert allein dem Verbraucher
};

} // namespace nakama::ipc
