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

    Der Candidate-Pfad ist derselbe Aufbau auf eigenen Baenken; sein Tap
    heisst post_candidate.

    ZWEI ZUSAGEN, DIE DIE FORM BESTIMMEN:

    1. Ausgeschaltet wird NICHTS geschrieben. Nicht "mit 1,0 multipliziert",
       nicht "durch neutrale Biquads gerechnet" - der Puffer wird nicht
       angefasst (M-01, M-05), und zwar unabhaengig davon, was die
       Hoermatrix gerade waehlt (B-3): die Hoermatrix liegt HINTER dem
       engagierten Pfad, nicht vor dem Ausgang eines ausgeschalteten.

    2. Der Passthrough SANITISIERT NICHTS (M-50). Ein NaN im ausgeschalteten
       Zustand kommt unveraendert heraus.

    DER LEBENSZYKLUS IST EIN GESCHLOSSENER AUTOMAT (Nacharbeit 1, Ursache
    der Befunde B-1 und B-5 bis B-12):

    - Die Uebernahme kennt die Generation (`DspBankPool`, B-1, B-12).
    - Jeder Pfad fuehrt einen VOLLSTAENDIGEN eigenen Zustand (`PfadZustand`):
      aktive Bank, Quellbank seines Uebergangs, Uebergangsart und -rest,
      eigene fuenf Rampen und eigene acht Auslenkungen (B-6, B-7, B-9).
    - Ein Uebergang endet IMMER: ein Wechsel, der waehrend eines laufenden
      Uebergangs eintrifft, bleibt publiziert und wird am ersten Blockrand
      NACH dessen Ende genommen (B-5, Entscheid E-17). Jeder Uebergang faengt
      damit beim zuletzt ausgegebenen Signal an.
    - Ausschalten und Candidate-Ende laufen ueber den regulaeren Weg: die
      ENDE-Marke wird publiziert, am Blockrand genommen, die Bank blendet aus
      und dient mit ACK aus; reserviert wird dafuer nichts (B-8, B-10, E-18).
    - Kein A/B-Wechsel laeuft ueber einen zweiten Fade nach Dry (M-55): mischt
      die Hoermatrix beim Candidate-Ende noch Candidate-Anteile, ist IHRE
      Rueckblende die einzige Blende - die Candidate-Bank klingt bis zu deren
      Ende unveraendert und dient erst dann mit ACK aus, ohne eigene Blende,
      weil keine Auswahl sie dann noch hoert (X-1, E-33); und auf
      einen Candidate, der gerade aus der Ruhe einblendet, blendet die
      Hoermatrix erst nach diesem Einblenden (E-34).
    - Uebernommen wird genau einmal je aeusserem `verarbeite`-Aufruf (B-11).

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

    /** Gibt die Puffer frei und setzt alle Zustaende zurueck. Der
        Generationszaehler des Pools bleibt stehen (B-12). */
    void freigeben();

    double samplerate() const noexcept { return abtastrate; }

    //== Control-Worker-Seite - laeuft NIE im Audiothread ====================

    /** Baut aus dem (bereits validierten) DTO ein Programm, legt es in eine
        freie Bank und publiziert sie fuer den Pfad.

        Ein Zustand mit `eq_enabled = false` reserviert KEINE Bank: er
        publiziert die ENDE-Marke des Pfades (M-07, B-10). Der abgeleitete
        Auto-Gain-Wert wird trotzdem gerechnet und ist lesbar (M-35).

        Liefert false, wenn keine Bank frei ist - das ist `busy_retry`
        (M-44). Der Aufrufer wiederholt; gepuffert oder erzwungen wird
        nichts. */
    bool uebernehmeZustand (const nakama::parameter::DspSatz& satz, Pfad p = Pfad::committed);

    /** Beendet den Candidate-Pfad als gekoppelte Lebenszyklusoperation
        (M-46, B-8): die ENDE-Marke wird publiziert und am naechsten
        Blockrand genommen, seine Baenke dienen ueber den ACK aus. Mischt die
        Hoermatrix dann noch Candidate-Anteile, blendet nur SIE zurueck: die
        Bank klingt bis zum Ende dieser Rueckblende unveraendert und dient
        erst danach aus (X-1). Sonst blendet der Pfad klickfrei in die Ruhe.
        Die Auswahl `Candidate` faellt sofort sichtbar auf Processed zurueck
        (M-56). */
    void beendeCandidate();

    /** Erntet die ACKs des Audiothreads und gibt Baenke frei. Ohne diesen
        Aufruf laeuft der Pool in `busy_retry` - genau so, wie der Vertrag es
        will. */
    int pflege() noexcept;

    DspBankPool&       pool() noexcept       { return baenke; }
    const DspBankPool& pool() const noexcept { return baenke; }

    //== Audiothread-Seite ==================================================

    /** Verarbeitet einen Block in-place. `kanaele` zeigt auf 1 oder 2
        Kanaele. Schreibt bei ausgeschaltetem Kern KEINEN Sample.

        Die Programmuebernahme beider Pfade laeuft genau EINMAL je Aufruf,
        vor dem ersten Sample (B-11). Ein Block groesser als `maxBlock` laeuft
        in Stuecken durch: die Stueckelung begrenzt nur den Puffer, sie
        erzeugt keine Uebernahmegrenze; verworfen wird die ANALYSE, nie Audio
        (M-48). */
    void verarbeite (float* const* kanaele, int numKanaele, int numSamples) noexcept;

    /** NUR fuer B6 (B-11): wird zwischen zwei Teilstuecken eines
        uebergrossen Blocks gerufen, damit der Test deterministisch eine
        Publikation MITTEN in einen aeusseren Aufruf legen kann. Im Produkt
        ist er nie gesetzt. */
    void setzeTeilstueckHaken (void (*haken) (void*), void* kontext) noexcept
    {
        teilstueckHaken = haken;
        teilstueckKontext = kontext;
    }

    //== Hoermatrix (R10) ===================================================

    /** Transient - kein Parameter, kein Feld im DTO, nichts im
        `state_hash`. Nach dem Anlegen und nach `freigeben` steht sie auf
        Processed. */
    void setzeHoermatrix (Hoermatrix h) noexcept { hoerwunsch.store (h, std::memory_order_release); }
    Hoermatrix gewuenschteHoermatrix() const noexcept { return hoerwunsch.load (std::memory_order_acquire); }

    /** Der WIRKSAME Zustand - nicht der gewuenschte. `Candidate` ohne
        Kandidat faellt hier sichtbar auf Processed zurueck (M-56), ebenso
        solange der Kandidat noch aus der Ruhe einblendet (E-34). */
    Hoermatrix wirksameHoermatrix() const noexcept { return hoerwirksam.load (std::memory_order_acquire); }

    //== Taps ===============================================================

    /** Zeiger auf den Tapinhalt des zuletzt verarbeiteten Blocks, oder
        nullptr. Immer `tapLaenge()` Samples lang. */
    const double* tap (Tap t, int kanal) const noexcept;
    int tapLaenge() const noexcept { return tapGueltig; }
    int tapKanaele() const noexcept { return letzteKanaele; }

    /** Liegt ein Candidate-Programm an (Wunsch des Workers)? */
    bool candidateVorhanden() const noexcept { return candidateAktiv.load (std::memory_order_acquire); }

    /** Die Slots, die der Audiothread gerade FAEHRT - je -1, wenn keiner:
        je Pfad die aktive Bank und die Quellbank des laufenden Uebergangs.
        Ohne sie ist die Zusage aus M-122 nicht pruefbar. */
    void gefahreneSlots (int& committed, int& committedQuelle, int& candidate, int& candidateQuelle) const noexcept
    {
        committed       = pfade[0].aktiv;
        committedQuelle = pfade[0].quelle;
        candidate       = pfade[1].aktiv;
        candidateQuelle = pfade[1].quelle;
    }

    //== Zaehler und abgeleitete Werte ======================================

    /** Nicht-endliche EINGANGSsamples im aktiven Pfad, verriegelt und
        gezaehlt (R9). Der Passthrough zaehlt hier NICHT mit. */
    std::uint64_t nichtEndlicheEingaenge() const noexcept
    { return zaehlerEingaenge.load (std::memory_order_relaxed); }

    /** Am Blockrand geheilte Filterzustaende (§5.9 Feinheit 3 und 4), je
        geheiltem Bandzustand bzw. Mono-Bass-Zustand einer. */
    std::uint64_t geheilteFilterzustaende() const noexcept
    { return zaehlerZustaende.load (std::memory_order_relaxed); }

    /** Bloecke, deren Tap nicht gefuellt werden konnte (M-48). */
    std::uint64_t verworfeneAnalyseframes() const noexcept
    { return zaehlerVerworfen.load (std::memory_order_relaxed); }

    /** Der abgeleitete Auto-Gain-Betrag des Committed-Programms in dB. IMMER
        lesbar, auch wenn der Schalter aus ist (M-35). */
    double autoGainDb() const noexcept { return autoGainBericht[0].load (std::memory_order_relaxed); }

    /** Derselbe Wert des Candidate-Programms, aus SEINER Kurve (B-6). */
    double autoGainCandidateDb() const noexcept { return autoGainBericht[1].load (std::memory_order_relaxed); }

    /** Die Quelle fuer `Frame.band_dynamic_gain_db` (R14): die acht
        momentanen Auslenkungen des COMMITTED-Pfads in Slot-Reihenfolge,
        kohaerent zu `post_committed` (B-9). Freie, ausgeschaltete und nicht
        dynamische Slots liefern exakt 0,0; ein nicht-endlicher Wert wird zu
        0 und gezaehlt. Nie aus Einstellwerten hergeleitet. */
    void auslenkungenDb (double* achtWerte) const noexcept;

    /** Dieselben acht Werte des Candidate-Pfads - getrennt gerechnet und
        getrennt gehalten (B-9). */
    void auslenkungenCandidateDb (double* achtWerte) const noexcept;

    /** Meldet mindestens ein Slot des Committed-Programms Dynamik (M-114)? */
    bool dynamikVorhanden() const noexcept { return dynamikAktiv.load (std::memory_order_acquire); }

    /** Am Blockrand genullte DENORMALE Zustandswerte (E-14). */
    std::uint64_t geriegelteDenormale() const noexcept
    { return zaehlerDenormale.load (std::memory_order_relaxed); }

    /** Nicht-endliche Auslenkungen, die vor der Meldung zu 0 wurden
        (M-113). */
    std::uint64_t nichtEndlicheAuslenkungen() const noexcept
    { return zaehlerAuslenkung.load (std::memory_order_relaxed); }

    /** Bloecke beziehungsweise Teilstuecke, in denen die M/S-Stufe
        GERECHNET wurde (M-30, B-19): bei width 1,0 und mono_bass 0 bleibt er
        stehen. */
    std::uint64_t msStufenLaeufe() const noexcept
    { return zaehlerMsStufe.load (std::memory_order_relaxed); }

    /** Am Blockrand genommene Publikationen (Bank oder ENDE-Marke), beide
        Pfade zusammen (B-11). */
    std::uint64_t uebernahmen() const noexcept
    { return zaehlerUebernahmen.load (std::memory_order_relaxed); }

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
                // Das letzte Sample setzt BITGENAU das Ziel (M-02).
                if (--rest == 0) aktuell = ziel;
            }
            return aktuell;
        }

        bool ruhtBei (double wert) const noexcept { return rest == 0 && aktuell == wert && ziel == wert; }
    };

    struct Rampen
    {
        Rampe input, output, mix, width, autoGain;
        void setzeSofort (double z) noexcept { input.setzeSofort (z); output.setzeSofort (z); mix.setzeSofort (z); width.setzeSofort (z); autoGain.setzeSofort (z); }
        void tick() noexcept { input.tick(); width.tick(); autoGain.tick(); mix.tick(); output.tick(); }
    };

    /** `hoerHalt` (X-1, Entscheid E-33): die endende Candidate-Bank liegt in
        `quelle` und klingt UNVERAENDERT - ohne Mischgewicht und ohne Rest -,
        solange die Hoermatrix Candidate-Anteile mischt. Das Ende setzt
        `beendeHoerHalt`, kein Samplezaehler. */
    enum class Uebergang { keiner = 0, crossfade, rampe, hoerHalt };

    /** Der vollstaendige Zustand EINES Pfades (B-6, B-7, B-9). Nur der
        Audiothread schreibt; die Auslenkungen sind atomar, weil der Bericht
        sie von aussen liest. */
    struct PfadZustand
    {
        int       aktiv     { -1 };                 ///< Slot `audioAktiv`, -1 = Ruhe
        int       quelle    { -1 };                 ///< Quellbank des Uebergangs (im Hoerhalt die endende Bank), -1 = Ruhe
        Uebergang uebergang { Uebergang::keiner };
        int       rest      { 0 };                  ///< Samples bis Uebergangsende
        Rampen    rampen    {};
        std::array<std::atomic<double>, (size_t) kSlots> auslenkungen {};

        void ruhe() noexcept;
    };

    void blockrand (Pfad p) noexcept;
    void heileZustaende (int slot) noexcept;

    /** Mischt die Hoermatrix gerade Candidate-Anteile - als Ziel ODER als
        noch nicht ausgeblendete Quelle ihres laufenden Fades? Nur der
        Audiothread fragt (X-1). */
    bool hoermatrixMischtCandidate() const noexcept;

    /** Beendet den Hoerhalt des Candidate-Pfades, sobald die Hoermatrix keine
        Candidate-Anteile mehr mischt: die Bank dient ueber den regulaeren Weg
        aus (ACK, danach Reclaim durch den Worker). Gerufen, wenn der
        Hoermatrix-Zustand eines Stuecks feststeht (X-1). */
    void beendeHoerHalt() noexcept;
    void verarbeiteStueck (float* const* kanaele, int numKanaele, int numSamples) noexcept;

    /** Rechnet EINE Bank eines Pfades auf `L`/`R` (in-place, bereits
        verriegelter Eingang). `rampeQuelle` ist bei einem Rampenuebergang
        die Quellbank, sonst -1; `rampeRest` der Uebergangsrest am Stueckbeginn. */
    void verarbeiteBank (PfadZustand& z, int slot, int rampeQuelle, int rampeRest,
                         double* L, double* R, int numSamples) noexcept;
    void verarbeiteBand (PfadZustand& z, DspBank& bank, const DspBank* quelle, int rampeRest,
                         int slotIndex, double* L, double* R, int numSamples) noexcept;

    /** Rechnet den ganzen Pfad fuer ein Stueck: `nachL/R` traegt die aktive
        Bank (oder Dry in Ruhe), `vonL/R` die Quellbank eines Crossfades
        (oder Dry), `ausL/R` das gemischte Ergebnis; `gewicht` das
        Crossfadegewicht je Sample (1,0 ausserhalb eines Crossfades). Fuehrt
        den Uebergangsrest und das Ausdienen der Quellbank. */
    void verarbeitePfad (Pfad p, const double* eingangL, const double* eingangR, int numSamples,
                         double* vonL, double* vonR, double* nachL, double* nachR,
                         double* ausL, double* ausR, double* gewicht) noexcept;

    bool istPassthrough (int slot) const noexcept;
    bool pfadRechnet (Pfad p) const noexcept;
    double* tapZeiger (Tap t, int kanal) noexcept;
    const DspProgramm& programmVon (int slot) const noexcept;
    void meldeProgramm (Pfad p, const DspProgramm& prog) noexcept;

    DspBankPool baenke;

    double abtastrate  { 0.0 };
    int    maxBlockGroesse { 0 };

    // Arbeitspuffer, vorallokiert.
    std::vector<double> dryL, dryR, eingL, eingR;
    std::vector<double> cVonL, cVonR, cNachL, cNachR, cAusL, cAusR, cGewicht;   ///< Committed
    std::vector<double> kVonL, kVonR, kNachL, kNachR, kAusL, kAusR, kGewicht;   ///< Candidate
    std::vector<double> tapPuffer;   ///< 3 Taps x 2 Kanaele x maxBlock

    std::array<PfadZustand, (size_t) kPfade> pfade {};

    /** Nur der Worker schreibt: der Bauplatz fuer einen ausgeschalteten
        Zustand, dessen abgeleitete Werte lesbar bleiben, ohne eine Bank zu
        belegen (M-35, B-10). */
    DspProgramm arbeitsProgramm {};

    std::atomic<Hoermatrix> hoerwunsch  { Hoermatrix::processed };
    std::atomic<Hoermatrix> hoerwirksam { Hoermatrix::processed };

    /*  Der eigene Uebergang der Hoermatrix (M-55). Nur der Audiothread liest
        und schreibt ihn. */
    Hoermatrix hoerLaufend  { Hoermatrix::processed };
    Hoermatrix hoerVorher   { Hoermatrix::processed };
    int        hoerFadeRest { 0 };
    std::atomic<bool>       candidateAktiv { false };
    std::atomic<bool>       dynamikAktiv   { false };

    std::atomic<std::uint64_t> zaehlerEingaenge   { 0 };
    std::atomic<std::uint64_t> zaehlerZustaende   { 0 };
    std::atomic<std::uint64_t> zaehlerVerworfen   { 0 };
    std::atomic<std::uint64_t> zaehlerAuslenkung  { 0 };
    std::atomic<std::uint64_t> zaehlerDenormale   { 0 };
    std::atomic<std::uint64_t> zaehlerMsStufe     { 0 };
    std::atomic<std::uint64_t> zaehlerUebernahmen { 0 };
    std::array<std::atomic<double>, (size_t) kPfade> autoGainBericht {};

    void (*teilstueckHaken) (void*) { nullptr };
    void* teilstueckKontext { nullptr };

    int tapGueltig    { 0 };
    int letzteKanaele { 0 };
};

} // namespace nakama::dsp
