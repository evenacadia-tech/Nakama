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

    WARUM ZUSTAND UND GENERATION EIN WORT SIND (Nacharbeit 1, B-1): liefe
    die Generation neben dem Zustand, koennte der Worker einen publizierten,
    noch nicht uebernommenen Slot zwischen dem Lesen der Publikation und dem
    Zustands-CAS des Audiothreads verdraengen, freigeben und fuer den
    anderen Pfad neu belegen - der CAS `bereit -> audioAktiv` gelaenge dann
    auf dem FREMDEN Programm. Mit Zustand und Generation in einem Wort
    erwartet der CAS genau die beobachtete Generation, und ein neu belegter
    Slot traegt eine andere. Dasselbe Wort traegt der ACK: ein ACK einer
    aelteren Generation findet seinen Slot nie mehr in `ausgedient` dieser
    Generation und gibt keine Bank frei.

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
    /** Vier Baenke - je zwei rechnende fuer Committed und Candidate. Mehr
        rechnet kein Pfad: waehrend seines Uebergangs traegt er die aktive und
        die verblassende Bank, und ein weiterer Wechsel wartet, bis der
        Uebergang endet (§44.2, M-46; Entscheid E-17). Eine wartende
        Publikation belegt ihren Slot trotzdem; reicht der Pool dafuer nicht,
        erhaelt der naechste Wunsch `busy_retry` (M-44). */
    static constexpr int kBaenke = 4;

    /** Der ACK-Ring fasst MEHR Eintraege als es Slots gibt (§44.2 woertlich)
        und droppt nie. Acht bei vier Slots laesst Raum fuer die Fadephase,
        in der bis zu vier Baenke gleichzeitig leben. */
    static constexpr int kAckKapazitaet = 8;

    /** Rueckgabe von `uebernehmeBereiten`, wenn statt einer Bank die
        ENDE-Marke des Pfades uebernommen wurde: der Pfad blendet in die Ruhe
        und belegt danach keine Bank (Entscheid E-18, B-8, B-10). */
    static constexpr int kEnde = -2;

    /** `ackKapazitaet` ist nur fuer B6 kleiner als `kAckKapazitaet`: der Ring
        kann im regulaeren Betrieb nicht ueberlaufen, und die Wache aus M-45
        braucht einen erzwungenen Ueberlauf (B-17). Der Speicher ist in
        beiden Faellen derselbe vorallokierte Ring. */
    explicit DspBankPool (int ackKapazitaet = kAckKapazitaet);

    /** Setzt jede Bank auf `frei` und nullt Zustaende, Ring und Maske.

        Der GENERATIONSZAEHLER bleibt stehen (R9 Feinheit 2, B-12): er ist ein
        Lebenszeitzaehler, kein Ressourcenzustand. Begaenne er beim Neuanlauf
        von vorn, koennte ein alter ACK eine neue Bank derselben Nummer
        freigeben. Hier wird nichts alloziert (M-41). */
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

    /** Die naechste Generation - monoton steigend, nie zurueckgesetzt
        (§5.9 Feinheit 2). */
    std::uint64_t naechsteGeneration() noexcept
    {
        return generationsZaehler.fetch_add (1, std::memory_order_relaxed) + 1;
    }

    /** `vorbereitend -> bereit(generation)` mit der Generation, die das
        Programm der Bank traegt, und Release-Publikation fuer den Pfad.
        Liefert den Slot, der dadurch VERDRAENGT und freigegeben wurde (er war
        publiziert, aber noch nicht uebernommen), oder -1. Ein verdraengter
        Slot war nie `audioAktiv` und braucht deshalb keinen ACK. */
    int publiziere (Pfad p, int slot) noexcept;

    /** Publiziert die ENDE-Marke des Pfades mit eigener Generation - ohne
        Bank. Ein ausgeschalteter Zustand und das Candidate-Ende laufen damit
        ueber denselben Blockrand wie jedes Programm, reservieren aber nichts
        (E-18). Liefert wie `publiziere` einen verdraengten Slot oder -1. */
    int publiziereEnde (Pfad p) noexcept;

    /** Liest den ACK-Ring leer: `ausgedient(g) -> frei` fuer genau die
        gemeldete Generation. Ein Slot, dessen Bit in der
        `reclaimPendingMask` steht, wird dabei NICHT frei - er wartet auf
        `bestaetigeReclaim` (M-45). Liefert die Anzahl freigegebener Slots.
        `hoechstens` begrenzt die Zahl der gelesenen Eintraege (negativ: alle) -
        eine Arbeitsgrenze fuer den Worker, und fuer B6 der Weg, einen
        veralteten ACK einzeln zu ernten (B-12). */
    int ernteAcks (int hoechstens = -1) noexcept;

    /** Bestaetigt einen Slot, dessen ACK im Ringueberlauf verlorenging.
        Wirkt NUR auf einen verriegelten Slot (Entscheid E-23): ein Slot ohne
        Bit hat seinen ACK im Ring, und eine Bestaetigung an ihm vorbei waere
        ein zweiter Freigabeweg. Liefert true, wenn der Slot frei wurde. */
    bool bestaetigeReclaim (int slot) noexcept;

    //== Audiothread-Seite - kein Lock, keine Allokation, kein IO ============

    /** Das publizierte Wort des Pfades: Generation und Slot, 0 = nichts. */
    std::uint64_t publikation (Pfad p) const noexcept
    {
        return veroeffentlicht[(size_t) p].load (std::memory_order_acquire);
    }

    /** Uebernimmt, was `beobachtet` zusagt - `bereit(g) -> audioAktiv(g)`
        fuer genau diese Generation. Liefert den Slot, `kEnde` fuer die
        ENDE-Marke, oder -1, wenn die Beobachtung veraltet ist. Getrennt von
        `uebernehmeBereiten`, damit B6 die Verdraengung zwischen Lesen und
        Uebernahme deterministisch einschieben kann (B-1). */
    int uebernehme (Pfad p, std::uint64_t beobachtet) noexcept;

    /** Acquire-Load der Publikationsstelle und Uebernahme in einem Schritt. */
    int uebernehmeBereiten (Pfad p) noexcept { return uebernehme (p, publikation (p)); }

    /** `audioAktiv -> verblassend`. */
    bool beginneVerblassen (int slot) noexcept;

    /** `verblassend -> ausgedient` und ACK (Slot plus Generation) an den
        Worker. Schlaegt der Ring-Push fehl, wird das Bit in der
        `reclaimPendingMask` gesetzt und der Slot bleibt dauerhaft nicht frei,
        bis der Worker ihn bestaetigt (M-45). */
    void meldeAusgedient (int slot) noexcept;

    /** Reiht einen ACK ein. Im Produkt ruft ihn nur `meldeAusgedient`;
        oeffentlich, damit B6 einen ACK einer ALTEN Generation einspielen und
        zeigen kann, dass er keine Bank freigibt (B-12). Liefert false bei
        vollem Ring. */
    bool ackEinreihen (int slot, std::uint64_t generation) noexcept;

    //== Beobachtung (fuer Tests und den Bericht) ============================

    BankZustand zustand (int slot) const noexcept
    {
        return zustandAus (zustaende[(size_t) slot].load (std::memory_order_acquire));
    }

    std::uint64_t generation (int slot) const noexcept
    {
        return generationAus (zustaende[(size_t) slot].load (std::memory_order_acquire));
    }

    int freieSlots() const noexcept;

    /** Wieviele Baenke gerade nicht `frei` sind - fuer M-46 und M-07. */
    int belegteSlots() const noexcept { return kBaenke - freieSlots(); }

    std::uint64_t reclaimPendingMask() const noexcept
    {
        return reclaimMaske.load (std::memory_order_acquire);
    }

    std::uint64_t ackUeberlaeufe() const noexcept
    {
        return ueberlaeufe.load (std::memory_order_relaxed);
    }

    /** Wie oft ein Slot wegen eines Ringueberlaufs VERRIEGELT wurde. */
    std::uint64_t reclaimVerriegelungen() const noexcept
    {
        return verriegelungen.load (std::memory_order_relaxed);
    }

    int ackKapazitaet() const noexcept { return ringKapazitaet; }

    //== Die beiden Worte =====================================================

    static constexpr int kZustandsBits = 3;

    static std::uint64_t zustandsWort (std::uint64_t generation, BankZustand z) noexcept
    {
        return (generation << kZustandsBits) | (std::uint64_t) z;
    }
    static BankZustand zustandAus (std::uint64_t wort) noexcept
    {
        return (BankZustand) (wort & ((1u << kZustandsBits) - 1u));
    }
    static std::uint64_t generationAus (std::uint64_t wort) noexcept
    {
        return wort >> kZustandsBits;
    }

    /** Publikations- und ACK-Wort: Generation und `slot + 1`; 0 heisst leer.
        `slot == kBaenke` ist die ENDE-Marke. */
    static std::uint64_t publikationsWort (std::uint64_t generation, int slot) noexcept
    {
        return (generation << kZustandsBits) | (std::uint64_t) (slot + 1);
    }
    static int slotAus (std::uint64_t wort) noexcept
    {
        return (int) (wort & ((1u << kZustandsBits) - 1u)) - 1;
    }

private:
    bool wechsle (int slot, std::uint64_t von, std::uint64_t nach) noexcept;
    int  verdraengeBereiten (std::uint64_t altesWort) noexcept;

    std::array<DspBank, (size_t) kBaenke> baenke {};
    std::array<std::atomic<std::uint64_t>, (size_t) kBaenke> zustaende {};

    std::array<std::atomic<std::uint64_t>, (size_t) kPfade> veroeffentlicht {};

    // Der SPSC-Ring: Audio schreibt, Control liest. Vorallokiert.
    int ringKapazitaet { kAckKapazitaet };
    std::array<std::uint64_t, (size_t) kAckKapazitaet> ackRing {};
    std::atomic<std::uint64_t> ackSchreib { 0 }, ackLese { 0 };

    std::atomic<std::uint64_t> reclaimMaske { 0 };
    std::atomic<std::uint64_t> ueberlaeufe { 0 };
    std::atomic<std::uint64_t> verriegelungen { 0 };
    std::atomic<std::uint64_t> generationsZaehler { 0 };
};

} // namespace nakama::dsp
