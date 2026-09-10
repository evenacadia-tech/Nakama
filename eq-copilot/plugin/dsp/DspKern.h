#pragma once

/*  SONDE-015 Etappe 3 - der aktive DSP-Kern als Bibliothek.

    Diese Klasse IST der Audiopfad aus §3.0 des Manifests:

        Eingang
          -> Tap pre_nakama
             -> eq_enabled == false  ------------------> Ausgang (nichts geschrieben)
             -> v1.global.bypass == true --------------> Ausgang (nichts geschrieben)
             -> Input-Trim  (Rampe, Unity-Kurzschluss bei 0 dB)
                M/S-Stufe   (Width, Mono-Bass; Kurzschluss bei width 1 und mono_bass 0)
                8 Band-Slots in Reihenfolge 0 -> 7, je Kanalzuordnung
                Auto-Gain   (abgeleitet, Rampe wie Output-Trim)
                Mix         (linear gegen den Tap pre_nakama)
                Output-Trim (Rampe, Unity-Kurzschluss bei 0 dB)
                -> Tap post_committed
                   -> Hoermatrix (Dry / Processed / Delta / Candidate)
                      -> Ausgang

    ZWEI ZUSAGEN, DIE DIE FORM BESTIMMEN:

    1. Ausgeschaltet wird NICHTS geschrieben. Nicht "mit 1,0 multipliziert",
       nicht "durch neutrale Biquads gerechnet" - der Puffer wird nicht
       angefasst (M-01, M-05). Deshalb hat jede neutrale Stufe einen
       Kurzschluss, und deshalb prueft der Kurzschluss den dB-Wert und nicht
       den linearen Faktor.

    2. Der Passthrough SANITISIERT NICHTS (M-50). Ein NaN im ausgeschalteten
       Zustand kommt unveraendert heraus. Ein Passthrough, der Werte
       repariert, waere eine Verarbeitung, die niemand eingeschaltet hat -
       und der Nulltest waere keine Bitgleichheit mehr, sondern eine
       Behauptung.

    WAS HIER NICHT LIEGT: Transaktionen, Revisionen, `state_hash`, Undo,
    Preset, APVTS, Telemetrie und die Schutz-Zonen. Der Kern nimmt einen
    fertig validierten `DspSatz` entgegen und gibt Audio und Zaehler zurueck;
    alles andere ist Etappe 4.
*/

#include "DspBankPool.h"
#include "DspRtWache.h"

#include <atomic>
#include <vector>

namespace nakama::dsp
{

/** Die drei Analysetaps (§44.2 letzter Absatz). */
enum class Tap { preNakama = 0, postCommitted, postCandidate };
inline constexpr int kTaps = 3;

class DspKern
{
public:
    DspKern();

    /** Vorallokation. Nach diesem Aufruf allokiert der Kern nicht mehr -
        auch nicht beim ersten Programmwechsel (M-41). `maxBlock` ist die
        groesste Blockgroesse, fuer die die Taps Platz haben; ein groesserer
        Block laeuft trotzdem durch (Audio faellt nie aus), nur der Tap
        dieses Blocks gilt als verworfen und wird gezaehlt (M-48). */
    void bereiteVor (double samplerate, int maxBlock);

    /** Gibt die Puffer frei und setzt alle Zustaende zurueck. */
    void freigeben();

    double samplerate() const noexcept { return abtastrate; }

    //== Control-Worker-Seite - laeuft NIE im Audiothread ====================

    /** Baut aus dem (bereits validierten) DTO ein Programm, legt es in eine
        freie Bank und publiziert sie fuer den Pfad.

        Liefert false, wenn keine Bank frei ist - das ist `busy_retry`
        (M-44). Der Aufrufer wiederholt; gepuffert oder erzwungen wird
        nichts. */
    bool uebernehmeZustand (const nakama::parameter::DspSatz& satz, Pfad p = Pfad::committed);

    /** Beendet den Candidate-Pfad neutral: die Auswahl `Candidate` faellt
        danach sichtbar auf Processed zurueck (M-46, M-56). */
    void beendeCandidate();

    /** Erntet die ACKs des Audiothreads und gibt Baenke frei. Ohne diesen
        Aufruf laeuft der Pool nach vier Wechseln in `busy_retry` - genau
        so, wie der Vertrag es will. */
    int pflege() noexcept;

    DspBankPool&       pool() noexcept       { return baenke; }
    const DspBankPool& pool() const noexcept { return baenke; }

    //== Audiothread-Seite ==================================================

    /** Verarbeitet einen Block in-place. `kanaele` zeigt auf 1 oder 2
        Kanaele. Schreibt bei ausgeschaltetem Kern KEINEN Sample.

        Ein Block groesser als `maxBlock` laeuft in Stuecken durch: verworfen
        wird die ANALYSE (der Tap dieses Blocks), nie Audio (M-48). */
    void verarbeite (float* const* kanaele, int numKanaele, int numSamples) noexcept;

    //== Hoermatrix (R10) ===================================================

    /** Transient - kein Parameter, kein Feld im DTO, nichts im
        `state_hash`. Nach dem Anlegen und nach `freigeben` steht sie auf
        Processed. */
    void setzeHoermatrix (Hoermatrix h) noexcept { hoerwunsch.store (h, std::memory_order_release); }
    Hoermatrix gewuenschteHoermatrix() const noexcept { return hoerwunsch.load (std::memory_order_acquire); }

    /** Der WIRKSAME Zustand - nicht der gewuenschte. `Candidate` ohne
        Kandidat faellt hier sichtbar auf Processed zurueck (M-56); ein
        Getter, der den Wunsch meldet, waere ein totes Element. */
    Hoermatrix wirksameHoermatrix() const noexcept { return hoerwirksam.load (std::memory_order_acquire); }

    //== Taps ===============================================================

    /** Zeiger auf den Tapinhalt des zuletzt verarbeiteten Blocks, oder
        nullptr. Immer `tapLaenge()` Samples lang. */
    const double* tap (Tap t, int kanal) const noexcept;
    int tapLaenge() const noexcept { return tapGueltig; }
    int tapKanaele() const noexcept { return letzteKanaele; }

    /** Liegt ueberhaupt ein Candidate-Programm an? */
    bool candidateVorhanden() const noexcept { return candidateAktiv.load (std::memory_order_acquire); }

    /** Die Slots, die der Audiothread gerade FAEHRT - je -1, wenn keiner.

        Ohne sie ist die Zusage aus M-122 nicht pruefbar: dass kein Reclaim
        vor dem ACK geschieht, zeigt sich daran, dass ein Slot, den der
        Audiothread noch liest, nicht `frei` oder `vorbereitend` wird. Die
        Zustandszaehlung allein sieht das nicht - vier Baenke bleiben vier
        Baenke, gleichgueltig wer sie haelt. */
    void gefahreneSlots (int& committed, int& verblassendSlot, int& candidate) const noexcept
    {
        committed       = lageCommitted.aktiv;
        verblassendSlot = lageCommitted.verblassend;
        candidate       = lageCandidate.aktiv;
    }

    //== Zaehler und abgeleitete Werte ======================================

    /** Nicht-endliche EINGANGSsamples im aktiven Pfad, verriegelt und
        gezaehlt (R9). Der Passthrough zaehlt hier NICHT mit. */
    std::uint64_t nichtEndlicheEingaenge() const noexcept
    { return zaehlerEingaenge.load (std::memory_order_relaxed); }

    /** Am Blockrand geheilte Filterzustaende (§5.9 Feinheit 3 und 4). Zwei
        Zaehler statt einem, weil ein gemeinsamer die beiden Ursachen nicht
        trennen koennte. */
    std::uint64_t geheilteFilterzustaende() const noexcept
    { return zaehlerZustaende.load (std::memory_order_relaxed); }

    /** Bloecke, deren Tap nicht gefuellt werden konnte, weil der Block
        groesser als `maxBlock` war. Verworfen wird die ANALYSE, nie Audio
        (M-48). */
    std::uint64_t verworfeneAnalyseframes() const noexcept
    { return zaehlerVerworfen.load (std::memory_order_relaxed); }

    /** Der abgeleitete Auto-Gain-Betrag des bestaetigten Programms in dB.
        IMMER lesbar, auch wenn der Schalter aus ist (M-35). */
    double autoGainDb() const noexcept { return autoGainBericht.load (std::memory_order_relaxed); }

    /** Die Quelle fuer `Frame.band_dynamic_gain_db` (R14): acht Werte in
        Slot-Reihenfolge, die MOMENTANE Auslenkung in dB. Freie,
        ausgeschaltete und nicht dynamische Slots liefern exakt 0,0; ein
        nicht-endlicher Wert wird zu 0 und gezaehlt. Nie aus Einstellwerten
        hergeleitet. */
    void auslenkungenDb (double* achtWerte) const noexcept;

    /** Meldet mindestens ein Slot Dynamik? Bestimmt, ob das Feld ueberhaupt
        reist (M-114). */
    bool dynamikVorhanden() const noexcept { return dynamikAktiv.load (std::memory_order_acquire); }

    /** Am Blockrand genullte DENORMALE Zustandswerte. Der Selbstaudit
        dieses Tickets nennt Denormals ausdruecklich; ein Zaehler macht den
        Riegel messbar statt behauptet (Pruefliste A). */
    std::uint64_t geriegelteDenormale() const noexcept
    { return zaehlerDenormale.load (std::memory_order_relaxed); }

    /** Nicht-endliche Auslenkungen, die vor der Meldung zu 0 wurden
        (M-113). */
    std::uint64_t nichtEndlicheAuslenkungen() const noexcept
    { return zaehlerAuslenkung.load (std::memory_order_relaxed); }

    void zaehlerZuruecksetzen() noexcept;

private:
    struct Rampe
    {
        double aktuell { 1.0 }, ziel { 1.0 }, schritt { 0.0 };
        int    rest { 0 };

        void setzeSofort (double z) noexcept { aktuell = ziel = z; schritt = 0.0; rest = 0; }

        void setzeZiel (double z) noexcept
        {
            if (z == ziel) return;
            ziel    = z;
            schritt = (z - aktuell) / (double) kRampeSamples;
            rest    = kRampeSamples;
        }

        double tick() noexcept
        {
            if (rest > 0)
            {
                aktuell += schritt;
                // Das letzte Sample setzt BITGENAU das Ziel. Ohne diese
                // Zeile endete eine Rampe auf 1,0 bei 0,99999994 - und die
                // Bitidentitaet aus M-02 waere keine.
                if (--rest == 0) aktuell = ziel;
            }
            return aktuell;
        }

        bool ruhtBei (double wert) const noexcept { return rest == 0 && aktuell == wert && ziel == wert; }
    };

    struct PfadLage
    {
        int aktiv       { -1 };   ///< Slot in `audioAktiv`, oder -1 (Ruhe = Passthrough)
        int verblassend { -1 };   ///< Slot in `verblassend`, oder -1
        int fadeRest    { 0 };    ///< Samples bis Fadeende
    };

    /** Der eigentliche Weg fuer ein Stueck, das in `maxBlock` passt. */
    void verarbeiteStueck (float* const* kanaele, int numKanaele, int numSamples) noexcept;

    void blockrand (Pfad p, PfadLage& lage) noexcept;
    void setzeRampenziele (const DspProgramm& p) noexcept;
    void heileZustaende (int slot) noexcept;

    /*  Beide bekommen ihre ZIELPUFFER uebergeben. Committed, die
        verblassende Bank und Candidate laufen sonst nacheinander auf
        denselben Arbeitspuffern und ueberschrieben einander - der
        Candidate-Pfad haette das Committed-Ergebnis verloren. */
    void verarbeiteBank (int slot, double* L, double* R, int numSamples) noexcept;
    void verarbeiteBand (DspBank& bank, int slotIndex, double* L, double* R, int numSamples) noexcept;

    /** Linearer Crossfade `nach = (1-t)*von + t*nach`. Erreicht der
        Fadezaehler 0, bricht die Schleife ab und der Rest des Blocks steht
        BITGENAU auf dem Ziel - das ist der Unterschied zwischen
        "bitidentisch" und "numerisch nahe" (M-04). */
    void mischeFade (const double* vonL, const double* vonR,
                     double* nachL, double* nachR, int numSamples, PfadLage& lage) noexcept;

    double* tapZeiger (Tap t, int kanal) noexcept;
    const DspProgramm& programmVon (int slot) const noexcept;

    DspBankPool baenke;

    double abtastrate  { 0.0 };
    int    maxBlockGroesse { 0 };

    // Arbeitspuffer, vorallokiert. `arbeitL/R` traegt den laufenden Pfad,
    // `mischL/R` den zweiten waehrend eines Fades.
    std::vector<double> arbeitL, arbeitR, mischL, mischR, dryL, dryR, candL, candR;
    std::vector<double> tapPuffer;   ///< 3 Taps x 2 Kanaele x maxBlock

    PfadLage lageCommitted, lageCandidate;

    Rampe rInputTrim, rOutputTrim, rMix, rWidth, rAutoGain;

    std::atomic<Hoermatrix> hoerwunsch  { Hoermatrix::processed };
    std::atomic<Hoermatrix> hoerwirksam { Hoermatrix::processed };

    /*  Der eigene Uebergang der Hoermatrix (M-55). Sie liegt HINTER allen
        drei Taps und faehrt bei keinem Bankwechsel mit - ohne diesen Fade
        spraenge der Ausgang beim Wechsel Processed -> Dry um die volle
        Differenz beider Wege. Nur der Audiothread liest und schreibt sie;
        deshalb sind es einfache Felder und keine Atomics. */
    Hoermatrix hoerLaufend  { Hoermatrix::processed };
    Hoermatrix hoerVorher   { Hoermatrix::processed };
    int        hoerFadeRest { 0 };
    std::atomic<bool>       candidateAktiv { false };
    std::atomic<bool>       dynamikAktiv   { false };

    std::atomic<std::uint64_t> zaehlerEingaenge  { 0 };
    std::atomic<std::uint64_t> zaehlerZustaende  { 0 };
    std::atomic<std::uint64_t> zaehlerVerworfen  { 0 };
    std::atomic<std::uint64_t> zaehlerAuslenkung { 0 };
    std::atomic<std::uint64_t> zaehlerDenormale  { 0 };
    std::atomic<double>        autoGainBericht   { 0.0 };

    /*  Die acht Auslenkungen, lockfrei lesbar. `double` ist auf x64
        lock-free; der `static_assert` in der .cpp haelt das fest, damit ein
        anderer Zielrechner nicht still eine Sperre einzieht. */
    std::array<std::atomic<double>, (size_t) kSlots> auslenkungen {};

    int tapGueltig    { 0 };
    int letzteKanaele { 0 };
};

} // namespace nakama::dsp
