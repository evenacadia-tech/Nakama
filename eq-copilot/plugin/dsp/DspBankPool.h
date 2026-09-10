#pragma once

/*  SONDE-015 Etappe 3 - die vier vorallokierten Programmbaenke und ihr
    lockfreies Ownership-Protokoll (Entwurf §44.2, R9).

    Der Vertrag woertlich:

      "Die vier Slots besitzen ein explizites lockfreies Ownership-Protokoll
       free -> preparing -> ready(generation) -> audio_active/fading ->
       retired -> free. Der Control-Worker schreibt ausschliesslich
       free-Slots und publiziert Index plus Generation mit Release-Semantik.
       Der Audiothread uebernimmt nur am Blockrand, liest bis Fade-Ende
       unveraendert und meldet die ausgediente Generation ueber einen
       vorallokierten Audio->Control-SPSC-Ring zurueck. Erst nach diesem ACK
       darf der Worker Filterzustand oder Koeffizienten ueberschreiben. Ist
       kein Slot frei, erhaelt der Befehl busy_retry; es gibt weder
       In-place-Ueberschreiben noch Heap-Reclaim, shared_ptr-Destruktor oder
       Deallokation im Callback."

    WARUM JEDER UEBERGANG EIN CAS IST: die Zusage aus M-42 lautet "genau die
    Kette" - ein Uebergang, den der Vertrag nicht nennt, muss SCHEITERN und
    nicht still durchgehen. Ein `store()` koennte jeden Zustand in jeden
    anderen ueberfuehren; ein `compare_exchange` kann nur den einen, der
    dasteht. Der Automat ist damit im Code und nicht nur im Kommentar.

    WARUM DER AUDIOTHREAD NIE FREIGIBT: er kennt den Moment nicht, in dem
    der Worker mit dem Ueberschreiben fertig waere. Er meldet deshalb nur;
    `frei` macht ausschliesslich der Worker (`ernteAcks`).
*/

#include "DspProgramm.h"

#include <atomic>
#include <array>
#include <cstdint>

namespace nakama::dsp
{

/** Zustand EINES Bandes in einer Bank: alles, was zwischen zwei Samples
    ueberlebt. Getrennt vom Programm, weil §44.2 genau diese Trennung
    verlangt.

    Zwei Komponenten je Band, nicht acht: `stereo` bearbeitet L und R,
    jeder andere Modus genau eine Komponente. Mehr braucht kein
    Vertragswert. */
struct BandZustand
{
    BiquadZustand     statisch[2] {};
    SvfZustand        svf[2] {};
    BiquadZustand     detektor[2] {};
    HuellkurveZustand huelle {};

    /*  Die Steuerrate des dynamischen Bandes: `schrittRest` zaehlt bis zum
        naechsten Neuentwurf, `svfVon`/`svfNach` sind die beiden Enden der
        linearen Ueberblendung dazwischen. */
    int              schrittRest { 0 };
    SvfKoeffizienten svfVon {}, svfNach {};
    double           auslenkungDb { 0.0 };   ///< die zuletzt gerechnete, momentane Auslenkung

    void nullen() noexcept;
    bool istEndlich() const noexcept;

    /** Nullt denormal kleine Zustaende. Liefert die Zahl der getroffenen
        Werte, damit der Riegel messbar bleibt. */
    int riegleDenormale() noexcept;
};

/** Eine vorallokierte Bank: ein Programm und der Zustand, den es fuehrt. */
struct DspBank
{
    DspProgramm programm {};
    std::array<BandZustand, (size_t) kSlots> baender {};
    BiquadZustand monoBassZustand {};   ///< der Hochpass auf der Seite

    void zustaendeNullen() noexcept;
    bool zustaendeSindEndlich() const noexcept;
    int  riegleDenormale() noexcept;
};

//==============================================================================
/** Die sechs Zustaende des Ownership-Automaten, in der Reihenfolge des
    Vertrags. */
enum class BankZustand
{
    frei = 0,
    vorbereitend,     ///< preparing - der Worker schreibt
    bereit,           ///< ready(generation) - publiziert, noch nicht uebernommen
    audioAktiv,       ///< audio_active
    verblassend,      ///< fading
    ausgedient        ///< retired - der Audiothread ist fertig, der ACK ist unterwegs
};

/** Die beiden Pfade. Committed ist der Normalbetrieb; Candidate rechnet nur
    beim Vergleich mit und wird nie Baseline (§44.2). */
enum class Pfad { committed = 0, candidate = 1 };
inline constexpr int kPfade = 2;

//==============================================================================
class DspBankPool
{
public:
    /** Vier Baenke - Double-Buffer fuer Committed und Candidate, und im
        schlechtesten Fall traegt jeder Pfad waehrend seines Fades zwei
        (§44.2, M-46). */
    static constexpr int kBaenke = 4;

    /** Der ACK-Ring fasst MEHR Eintraege als es Slots gibt (§44.2 woertlich)
        und droppt nie. Acht bei vier Slots laesst Raum fuer die Fadephase,
        in der bis zu vier Baenke gleichzeitig leben. */
    static constexpr int kAckKapazitaet = 8;

    DspBankPool();

    /** Setzt jede Bank auf `frei`, nullt Zustaende und Generationen. Der
        Speicher ist bereits im Konstruktor vorhanden; hier wird nichts
        alloziert (M-41). */
    void zuruecksetzen() noexcept;

    //== Control-Worker-Seite - laeuft NIE im Audiothread ====================

    /** `frei -> vorbereitend`. Liefert -1, wenn kein Slot frei ist; das ist
        `busy_retry` (M-44). Der Befehl wird NICHT gepuffert und nicht mit
        Gewalt durchgesetzt. */
    int reserviere() noexcept;

    /** Die Bank eines reservierten Slots - nur zwischen `reserviere` und
        `publiziere` gueltig zu beschreiben. */
    DspBank& bank (int slot) noexcept { return baenke[(size_t) slot]; }
    const DspBank& bank (int slot) const noexcept { return baenke[(size_t) slot]; }

    /** `vorbereitend -> bereit(generation)` und Release-Publikation fuer den
        Pfad. Liefert den Slot, der dadurch VERDRAENGT wurde (er war
        publiziert, aber noch nicht uebernommen) oder -1. Ein verdraengter
        Slot war nie `audioAktiv` und braucht deshalb keinen ACK - er geht
        direkt zurueck auf `frei`. */
    int publiziere (Pfad p, int slot) noexcept;

    /** Liest den ACK-Ring leer: `ausgedient -> frei`. Ein Slot, dessen Bit
        in der `reclaimPendingMask` steht, wird dabei NICHT frei - er wartet
        auf `bestaetigeReclaim` (M-45). Liefert die Anzahl freigegebener
        Slots. */
    int ernteAcks() noexcept;

    /** Bestaetigt einen Slot, dessen ACK im Ringueberlauf verlorenging.
        Erst danach kann er wieder `frei` werden. Reclaim-Sicherheit gewinnt
        ueber Verfuegbarkeit. */
    void bestaetigeReclaim (int slot) noexcept;

    //== Audiothread-Seite - kein Lock, keine Allokation, kein IO ============

    /** Acquire-Load der Publikationsstelle. Liefert den bereiten Slot und
        setzt die Stelle zurueck, oder -1. `bereit -> audioAktiv`. */
    int uebernehmeBereiten (Pfad p) noexcept;

    /** `audioAktiv -> verblassend`. */
    bool beginneVerblassen (int slot) noexcept;

    /** `verblassend -> ausgedient` und ACK an den Worker. Schlaegt der
        Ring-Push fehl, wird das Bit in der `reclaimPendingMask` gesetzt und
        der Slot bleibt dauerhaft nicht frei, bis der Worker ihn bestaetigt
        (M-45). */
    void meldeAusgedient (int slot) noexcept;

    //== Beobachtung (fuer Tests und den Bericht) ============================

    BankZustand zustand (int slot) const noexcept
    {
        return zustaende[(size_t) slot].load (std::memory_order_acquire);
    }

    std::uint64_t generation (int slot) const noexcept
    {
        return generationen[(size_t) slot].load (std::memory_order_acquire);
    }

    int freieSlots() const noexcept;

    std::uint64_t reclaimPendingMask() const noexcept
    {
        return reclaimMaske.load (std::memory_order_acquire);
    }

    std::uint64_t ackUeberlaeufe() const noexcept
    {
        return ueberlaeufe.load (std::memory_order_relaxed);
    }

    /** Wie oft ein Slot wegen eines Ringueberlaufs VERRIEGELT wurde.

        Der Ring fasst mehr Eintraege als es Slots gibt und kann im regulaeren
        Betrieb deshalb nicht ueberlaufen - der Overflow-Zweig ist eine Wache.
        Damit sie nicht als strukturelles Null unbemerkt verrotten kann, ist
        sie MESSBAR: `ackUeberlaeufe()` und dieser Zaehler muessen immer
        gleich sein, und genau daran faellt eine Fassung, die den Ueberlauf
        zaehlt, aber den Slot nicht haelt (Pruefliste A). */
    std::uint64_t reclaimVerriegelungen() const noexcept
    {
        return verriegelungen.load (std::memory_order_relaxed);
    }

    /** Die naechste Generation - monoton steigend, nie zurueckgesetzt
        (§5.9 Feinheit 2). Ein Zaehler, der von vorn begaenne, koennte einen
        alten ACK auf eine neue Bank beziehen. */
    std::uint64_t naechsteGeneration() noexcept
    {
        return generationsZaehler.fetch_add (1, std::memory_order_relaxed) + 1;
    }

    /** Wieviele Baenke gerade nicht `frei` sind - fuer M-46. */
    int belegteSlots() const noexcept { return kBaenke - freieSlots(); }

private:
    bool wechsle (int slot, BankZustand von, BankZustand nach) noexcept;

    std::array<DspBank, (size_t) kBaenke> baenke {};
    std::array<std::atomic<BankZustand>,  (size_t) kBaenke> zustaende {};
    std::array<std::atomic<std::uint64_t>, (size_t) kBaenke> generationen {};

    std::array<std::atomic<int>, (size_t) kPfade> veroeffentlicht {};

    // Der SPSC-Ring: Audio schreibt, Control liest. Vorallokiert.
    std::array<int, (size_t) kAckKapazitaet> ackRing {};
    std::atomic<std::uint64_t> ackSchreib { 0 }, ackLese { 0 };

    std::atomic<std::uint64_t> reclaimMaske { 0 };
    std::atomic<std::uint64_t> ueberlaeufe { 0 };
    std::atomic<std::uint64_t> verriegelungen { 0 };
    std::atomic<std::uint64_t> generationsZaehler { 0 };
};

} // namespace nakama::dsp
