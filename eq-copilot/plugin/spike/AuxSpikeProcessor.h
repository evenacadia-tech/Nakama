// EqCopAuxSpike — WEGWERF-MESSGERAET fuer SONDE-004a (User-Termin A).
//
// Zweck: in FL Studio beweisen, ob getrennte Aux-Busse ueberhaupt ankommen,
// in welcher Reihenfolge, mit welchem Versatz (PDC) und ob das Layout einen
// Speicher-/Ladezyklus ueberlebt. Das Ding misst und zeigt — es greift NIE
// ins Audio ein: der Main-Ausgang bleibt bitgleich zum Main-Eingang.
//
// Ausdruecklich KEIN Produktcode. Es teilt keine Quelle mit dem Plugin, traegt
// einen eigenen Plugin-Code (`NkSp`, nicht in der eingefrorenen Identitaets-
// liste §31.2) und wird nach dem Capabilityreport (S4) nicht weitergepflegt.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <vector>

namespace eqcop::spike
{

/** Ab diesem Betrag gilt ein Sample als Impuls. Bewusst hoch: ein Rauschteppich
    oder ein Fade-in soll die Messung nicht ausloesen. */
inline constexpr float kImpulsSchwelle = 0.25f;

/** Ab diesem Betrag gilt ein Sample als "nicht mehr still". */
inline constexpr float kRuheSchwelle = 0.02f;

/** Das Messprotokoll ist: Stille, dann EIN Impuls. Liegt mehr als dieser
    Anteil der Samples ueber der Ruheschwelle, laeuft in Wahrheit ein
    Dauersignal — dann ist "erstes Sample ueber der Schwelle" keine
    Impulsmessung, sondern Zufall. Der Spike verweigert dann den Versatz,
    statt eine glaubwuerdige Zahl zu erfinden. */
inline constexpr double kMaxLautAnteil = 0.01;

/** Vertragsnamen aus Entwurf §48.2 — Active deklariert genau diese zwei. */
inline constexpr const char* kNameAux1 = "priority_sidechain";
inline constexpr const char* kNameAux2 = "compare_pre";

/** Was der Nachrichtenthread ueber einen Bus zu sehen bekommt. */
struct BusBefund
{
    juce::String name;
    bool         aktiv        = false;
    int          kanaele      = 0;
    /** Absolute Sampleposition des ersten Impulses; -1 = noch keiner.
        -1 ist NICHT 0 — "kein Impuls" und "Impuls bei Sample 0" sind
        verschiedene Aussagen und werden nie vermischt. */
    juce::int64  impulsSample = -1;
    float        impulsSpitze = 0.0f;
    /** Hoechster gesehener Betrag ueberhaupt — beantwortet "kommt hier
        irgendetwas an?", auch ohne Impuls. */
    float        spitzeGesamt = 0.0f;
    /** Samples ueber der Ruheschwelle (ueber alle Kanaele gezaehlt). */
    juce::int64  lauteSamples = 0;
    /** Anteil davon an allen gesehenen Samples; -1 = noch nichts gesehen. */
    double       lautAnteil   = -1.0;
    /** Wurde das Messprotokoll "Stille, dann ein Impuls" eingehalten? */
    bool         protokollOk  = true;
};

struct Schnappschuss
{
    juce::String host;
    juce::String wrapper;
    double       samplerate         = 0.0;
    int          blockgroesse       = 0;
    juce::int64  samplesVerarbeitet = 0;
    std::vector<BusBefund> busse;

    /** Versatz eines Aux-Busses gegen den Main-Impuls in Samples. Nur gueltig,
        wenn beide Busse einen Impuls haben UND beide das Messprotokoll
        eingehalten haben — sonst gibt es keinen Wert, nicht etwa 0. */
    bool hatVersatz (size_t busIndex) const;
    /** Klartext, warum es keinen Versatz gibt (leer, wenn es einen gibt). */
    juce::String versatzGrund (size_t busIndex) const;
    juce::int64 versatz (size_t busIndex) const;
};

/** Bericht als JSON. Fehlende Messwerte werden `null`, nie 0 — der Entwurf
    verlangt "null statt erfundener Null". */
juce::String berichtAlsJson (const Schnappschuss& s);

class AuxSpikeProcessor final : public juce::AudioProcessor
{
public:
    AuxSpikeProcessor();
    ~AuxSpikeProcessor() override = default;

    void prepareToPlay (double samplerate, int blockgroesse) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layout) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "EqCop Aux Spike"; }
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

    /** Messstand fuer Anzeige und Bericht — Nachrichtenthread. */
    Schnappschuss schnappschuss() const;

    /** Gegenpfad zur Messung: alles auf Anfang. Der Audiothread raeumt beim
        naechsten Block selbst auf, damit hier keine Sperre noetig ist. */
    void messungZuruecksetzen();

    /** Schreibt den Bericht neben die anderen Nakama-Daten und liefert die
        Datei zurueck. Leeres File-Objekt = fehlgeschlagen. */
    juce::File berichtSchreiben() const;

    /** Zielordner der Berichte (wird bei Bedarf angelegt). */
    static juce::File berichtsOrdner();

private:
    static constexpr int kMaxBusse = 3;   // Main + zwei Aux

    struct BusZaehler
    {
        std::atomic<juce::int64> impulsSample { -1 };
        std::atomic<float>       impulsSpitze { 0.0f };
        std::atomic<float>       spitzeGesamt { 0.0f };
        std::atomic<juce::int64> lauteSamples { 0 };
        std::atomic<juce::int64> samplesGesehen { 0 };
    };

    BusZaehler zaehler[kMaxBusse];
    std::atomic<juce::int64> samplesVerarbeitet { 0 };
    std::atomic<bool>        resetAngefordert   { false };
    std::atomic<double>      samplerateAtomic   { 0.0 };
    std::atomic<int>         blockgroesseAtomic { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AuxSpikeProcessor)
};

} // namespace eqcop::spike
