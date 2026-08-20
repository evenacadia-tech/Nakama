#include "AuxSpikeProcessor.h"
#include "AuxSpikeEditor.h"

#include <cmath>

namespace eqcop::spike
{

// ----------------------------------------------------------- Schnappschuss --

bool Schnappschuss::hatVersatz (size_t busIndex) const
{
    return versatzGrund (busIndex).isEmpty();
}

juce::String Schnappschuss::versatzGrund (size_t busIndex) const
{
    if (busIndex == 0)
        return "Bezugspunkt";
    if (busIndex >= busse.size() || busse.empty())
        return "Bus nicht vorhanden";
    if (! busse[busIndex].aktiv)
        return "Bus ist inaktiv";
    if (busse[0].impulsSample < 0)
        return "Main hat keinen Impuls";
    if (busse[busIndex].impulsSample < 0)
        return "dieser Bus hat keinen Impuls";
    // Ein Dauersignal macht "erstes Sample ueber der Schwelle" zufaellig -
    // dann waere jede Versatzzahl eine Behauptung, keine Messung.
    if (! busse[0].protokollOk || ! busse[busIndex].protokollOk)
        return "Dauersignal statt Stille+Impuls - Messprotokoll verletzt";
    return {};
}

juce::int64 Schnappschuss::versatz (size_t busIndex) const
{
    jassert (hatVersatz (busIndex));
    return busse[busIndex].impulsSample - busse[0].impulsSample;
}

juce::String berichtAlsJson (const Schnappschuss& s)
{
    auto* wurzel = new juce::DynamicObject();
    wurzel->setProperty ("werkzeug", "EqCopAuxSpike");
    wurzel->setProperty ("zweck", "SONDE-004a - FL-Aux-/PDC-/Recall-Spike, Messgeraet fuer User-Termin A");
    wurzel->setProperty ("zeit", juce::Time::getCurrentTime().toISO8601 (true));
    wurzel->setProperty ("host", s.host);
    wurzel->setProperty ("wrapper", s.wrapper);
    wurzel->setProperty ("samplerate", s.samplerate);
    wurzel->setProperty ("blockgroesse", s.blockgroesse);
    wurzel->setProperty ("samples_verarbeitet", s.samplesVerarbeitet);

    juce::Array<juce::var> busListe;
    for (size_t i = 0; i < s.busse.size(); ++i)
    {
        const auto& b = s.busse[i];
        auto* eintrag = new juce::DynamicObject();
        eintrag->setProperty ("index", (int) i);
        eintrag->setProperty ("name", b.name);
        eintrag->setProperty ("aktiv", b.aktiv);
        eintrag->setProperty ("kanaele", b.kanaele);
        eintrag->setProperty ("spitze_gesamt", b.spitzeGesamt);
        eintrag->setProperty ("signal_angekommen", b.spitzeGesamt > 0.0f);
        eintrag->setProperty ("laute_samples", b.lauteSamples);
        eintrag->setProperty ("laut_anteil", b.lautAnteil >= 0.0 ? juce::var (b.lautAnteil) : juce::var());
        eintrag->setProperty ("protokoll_eingehalten", b.protokollOk);

        // Fehlender Messwert wird null, nie 0 (Entwurf: "null statt erfundener Null").
        if (b.impulsSample >= 0)
        {
            eintrag->setProperty ("impuls_sample", b.impulsSample);
            eintrag->setProperty ("impuls_spitze", b.impulsSpitze);
        }
        else
        {
            eintrag->setProperty ("impuls_sample", juce::var());
            eintrag->setProperty ("impuls_spitze", juce::var());
        }

        if (s.hatVersatz (i))
        {
            const auto v = s.versatz (i);
            eintrag->setProperty ("versatz_zu_main_samples", v);
            if (s.samplerate > 0.0)
                eintrag->setProperty ("versatz_zu_main_ms", (double) v * 1000.0 / s.samplerate);
            else
                eintrag->setProperty ("versatz_zu_main_ms", juce::var());
        }
        else
        {
            eintrag->setProperty ("versatz_zu_main_samples", juce::var());
            eintrag->setProperty ("versatz_zu_main_ms", juce::var());
            eintrag->setProperty ("versatz_grund", s.versatzGrund (i));
        }

        busListe.add (juce::var (eintrag));
    }
    wurzel->setProperty ("busse", busListe);

    return juce::JSON::toString (juce::var (wurzel), false);
}

// --------------------------------------------------------------- Prozessor --

AuxSpikeProcessor::AuxSpikeProcessor()
    : juce::AudioProcessor (BusesProperties()
          .withInput  ("Main",      juce::AudioChannelSet::stereo(), true)
          .withOutput ("Main",      juce::AudioChannelSet::stereo(), true)
          // Beide Aux-Busse sind bewusst VORGABE-INAKTIV: FL muss sie
          // ausdruecklich zuschalten, damit die Messung "hat der Host sie
          // ueberhaupt angeboten?" ehrlich beantwortet.
          .withInput  (kNameAux1,   juce::AudioChannelSet::stereo(), false)
          .withInput  (kNameAux2,   juce::AudioChannelSet::stereo(), false))
{
}

void AuxSpikeProcessor::prepareToPlay (double samplerate, int blockgroesse)
{
    samplerateAtomic.store (samplerate);
    blockgroesseAtomic.store (blockgroesse);
    messungZuruecksetzen();
}

bool AuxSpikeProcessor::isBusesLayoutSupported (const BusesLayout& layout) const
{
    const auto ein = layout.getMainInputChannelSet();
    const auto aus = layout.getMainOutputChannelSet();

    // Main: Mono oder Stereo, Ein == Aus. Nichts wird still umgedeutet
    // oder heruntergemischt - der Host bekommt ein klares Nein (Abschnitt 48.2).
    if (ein != aus)
        return false;
    if (ein != juce::AudioChannelSet::mono() && ein != juce::AudioChannelSet::stereo())
        return false;

    // Aux-Busse: abgeschaltet ist erlaubt, sonst Mono oder Stereo.
    for (int i = 1; i < layout.inputBuses.size(); ++i)
    {
        const auto aux = layout.inputBuses[i];
        if (aux.isDisabled())
            continue;
        if (aux != juce::AudioChannelSet::mono() && aux != juce::AudioChannelSet::stereo())
            return false;
    }

    return true;
}

void AuxSpikeProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals schutz;

    // Kein Lock, keine Allokation, kein I/O, kein Logging - der Reset kommt
    // als Flagge herein und wird hier abgeraeumt.
    if (resetAngefordert.exchange (false, std::memory_order_acq_rel))
    {
        for (auto& z : zaehler)
        {
            z.impulsSample.store (-1, std::memory_order_relaxed);
            z.impulsSpitze.store (0.0f, std::memory_order_relaxed);
            z.spitzeGesamt.store (0.0f, std::memory_order_relaxed);
            z.lauteSamples.store (0, std::memory_order_relaxed);
            z.samplesGesehen.store (0, std::memory_order_relaxed);
        }
        samplesVerarbeitet.store (0, std::memory_order_relaxed);
    }

    const auto basis  = samplesVerarbeitet.load (std::memory_order_relaxed);
    const int  anzahl = buffer.getNumSamples();
    const int  busse  = juce::jmin (getBusCount (true), kMaxBusse);

    for (int b = 0; b < busse; ++b)
    {
        auto* bus = getBus (true, b);
        if (bus == nullptr || ! bus->isEnabled())
            continue;

        const auto sicht = getBusBuffer (buffer, true, b);
        const int kanaele = sicht.getNumChannels();
        if (kanaele <= 0)
            continue;

        float spitze = zaehler[b].spitzeGesamt.load (std::memory_order_relaxed);
        int   kandidat = -1;          // frueheste Impulsstelle IN DIESEM Block
        float kandidatWert = 0.0f;
        juce::int64 laute = 0;        // Samples ueber der Ruheschwelle

        for (int k = 0; k < kanaele; ++k)
        {
            const float* daten = sicht.getReadPointer (k);
            int ersterHier = -1;

            for (int i = 0; i < anzahl; ++i)
            {
                const float betrag = std::abs (daten[i]);

                // NaN/Inf loesen nie einen Impuls aus: jeder Vergleich mit NaN
                // ist false, und das ist hier die gewuenschte Antwort
                // (messen, nicht behaupten).
                if (betrag > spitze)
                    spitze = betrag;

                if (betrag >= kRuheSchwelle)
                    ++laute;

                if (ersterHier < 0 && betrag >= kImpulsSchwelle)
                    ersterHier = i;
                // Kein vorzeitiges Abbrechen: sonst waere die Spitze nur bis
                // zum Impuls gemessen und "spitze_gesamt" wuerde luegen.
            }

            if (ersterHier < 0)
                continue;

            // Frueheste Stelle ueber ALLE Kanaele gewinnt - sonst haengt das
            // Ergebnis an der Kanalreihenfolge statt am Signal.
            const float wertHier = std::abs (daten[ersterHier]);
            if (kandidat < 0 || ersterHier < kandidat)
            {
                kandidat = ersterHier;
                kandidatWert = wertHier;
            }
            else if (ersterHier == kandidat)
            {
                kandidatWert = juce::jmax (kandidatWert, wertHier);
            }
        }

        zaehler[b].spitzeGesamt.store (spitze, std::memory_order_relaxed);
        zaehler[b].lauteSamples.fetch_add (laute, std::memory_order_relaxed);
        zaehler[b].samplesGesehen.fetch_add ((juce::int64) anzahl * kanaele,
                                             std::memory_order_relaxed);

        if (kandidat >= 0 && zaehler[b].impulsSample.load (std::memory_order_relaxed) < 0)
        {
            zaehler[b].impulsSpitze.store (kandidatWert, std::memory_order_relaxed);
            zaehler[b].impulsSample.store (basis + kandidat, std::memory_order_release);
        }
    }

    samplesVerarbeitet.store (basis + anzahl, std::memory_order_relaxed);

    // Der Main-Ausgang wird bewusst NICHT angefasst: JUCE reicht Main-Ein und
    // Main-Aus im selben Puffer, unveraendert heisst hier bitgleich.
}

juce::AudioProcessorEditor* AuxSpikeProcessor::createEditor()
{
    return new AuxSpikeEditor (*this);
}

Schnappschuss AuxSpikeProcessor::schnappschuss() const
{
    Schnappschuss s;
    s.host = juce::PluginHostType().getHostDescription();
    s.wrapper = juce::AudioProcessor::getWrapperTypeDescription (wrapperType);
    s.samplerate = samplerateAtomic.load();
    s.blockgroesse = blockgroesseAtomic.load();
    s.samplesVerarbeitet = samplesVerarbeitet.load (std::memory_order_relaxed);

    const int busse = juce::jmin (getBusCount (true), kMaxBusse);
    for (int b = 0; b < busse; ++b)
    {
        BusBefund befund;
        if (auto* bus = getBus (true, b))
        {
            befund.name    = bus->getName();
            befund.aktiv   = bus->isEnabled();
            befund.kanaele = bus->getNumberOfChannels();
        }
        befund.impulsSample = zaehler[b].impulsSample.load (std::memory_order_acquire);
        befund.impulsSpitze = zaehler[b].impulsSpitze.load (std::memory_order_relaxed);
        befund.spitzeGesamt = zaehler[b].spitzeGesamt.load (std::memory_order_relaxed);
        befund.lauteSamples = zaehler[b].lauteSamples.load (std::memory_order_relaxed);

        const auto gesehen = zaehler[b].samplesGesehen.load (std::memory_order_relaxed);
        befund.lautAnteil = gesehen > 0 ? (double) befund.lauteSamples / (double) gesehen : -1.0;
        befund.protokollOk = befund.lautAnteil < 0.0 || befund.lautAnteil <= kMaxLautAnteil;

        s.busse.push_back (befund);
    }
    return s;
}

void AuxSpikeProcessor::messungZuruecksetzen()
{
    resetAngefordert.store (true, std::memory_order_release);
}

juce::File AuxSpikeProcessor::berichtsOrdner()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("evenacadia")
               .getChildFile ("nakama")
               .getChildFile ("spike");
}

juce::File AuxSpikeProcessor::berichtSchreiben() const
{
    auto ordner = berichtsOrdner();
    const auto ergebnis = ordner.createDirectory();
    if (ergebnis.failed())
        return {};

    const auto name = "aux-spike-" + juce::Time::getCurrentTime().formatted ("%Y%m%d-%H%M%S") + ".json";
    auto datei = ordner.getChildFile (name);
    if (! datei.replaceWithText (berichtAlsJson (schnappschuss())))
        return {};

    return datei;
}

} // namespace eqcop::spike
