/*  S9 / SONDE-007b — der Passthrough-Beweis der beiden neuen Sonden.

    WARUM DIESER TEST EXISTIERT: Das Grundgesetz (CLAUDE.md, Wahrheitskern)
    sagt fuer Gen und Suna "Audio-Passthrough sampleidentisch (0 Samples
    Latenz, kein Tail)". Fuer das Main-Bundle beweist das seit M0 der
    EqCopNullTest. Mit S9 tragen ZWEI weitere Bundles Audio - und eine
    Zusage ohne Messung ist in diesem Baum keine Zusage.

    Probeeq ist laut User ein vollwertiger EQ; seine DSP kommt in P6. Bis
    dahin ist auch er ein Passthrough, und genau das misst dieser Test.
    Wenn P6 die DSP bringt, MUSS dieser Test fuer NkAc umgeschrieben werden -
    faellt er dann, ist das die richtige Reaktion und kein Aergernis.

    Gemessen wird die ECHTE Produktklasse SondeProcessor, nicht ein Nachbau:
    dasselbe .cpp, das in die Bundles geht, nur ohne Plugin-Wrapper.

    Aufruf: EqCopSondeNullTest.exe        Exit 0 gruen, 1 rot.
*/

#include "SondeProcessor.h"

#include <cmath>
#include <cstring>
#include <iostream>

namespace
{

int okZahl = 0;
int fehlerZahl = 0;

void pruefe (bool bedingung, const juce::String& text, const juce::String& zusatz = {})
{
    const auto zeile = text + (zusatz.isNotEmpty() ? ("  [" + zusatz + "]") : juce::String());
    if (bedingung) { ++okZahl;     std::cout << "  ok      " << zeile << std::endl; }
    else           { ++fehlerZahl; std::cout << "  FEHLER  " << zeile << std::endl; }
}

/** Reproduzierbares Rauschen — dieselbe Saat ergibt dieselbe Folge. */
void fuelle (juce::AudioBuffer<float>& puffer, juce::Random& wuerfel)
{
    for (int k = 0; k < puffer.getNumChannels(); ++k)
        for (int n = 0; n < puffer.getNumSamples(); ++n)
            puffer.setSample (k, n, wuerfel.nextFloat() * 2.0f - 1.0f);
}

} // namespace

int main()
{
    std::cout << "SONDE-NULLTEST — " << nakama::sonde::SondeProcessor().getName()
              << " (Produktklasse "
              << nakama::state::wort (nakama::sonde::kProduktklasse) << ")" << std::endl;

    nakama::sonde::SondeProcessor prozessor;

    // -- 1. Produktklasse und Bundlevertrag passen zueinander --------------
    // Ein Bundle, dessen frischer Zustand seinen eigenen Vertrag verletzt,
    // kaeme beim ersten Speichern+Laden als read-only zurueck.
    pruefe (prozessor.zustandLesen().common.klasse == nakama::sonde::kProduktklasse,
            "frischer Zustand traegt die Produktklasse des Bundles",
            nakama::state::wort (prozessor.zustandLesen().common.klasse));
    pruefe (nakama::sonde::bundleVertrag().erlaubt (nakama::sonde::kProduktklasse),
            "der Bundlevertrag laesst die eigene Produktklasse zu");
    pruefe (! nakama::sonde::bundleVertrag().erlaubt (nakama::state::Klasse::main),
            "der Bundlevertrag laesst 'main' NICHT zu");
    pruefe (! nakama::sonde::bundleVertrag().erlaubt (nakama::state::Klasse::legacy),
            "der Bundlevertrag laesst 'legacy' NICHT zu");

    // -- 2. Keine Hostparameter --------------------------------------------
    pruefe (prozessor.getParameters().size() == 0,
            "das Bundle meldet dem Host keinen Parameter",
            juce::String (prozessor.getParameters().size()));
    pruefe (! prozessor.hasEditor(),
            "keine erfundene Oberflaeche (Gestaltung kommt aus Figma)");

    // -- 2b. Lifecycle: neutral bis gueltigem State (§53.5, S9 Abschnitt 3) --
    // Ein Scannerlauf sieht genau diese frische Instanz - und klassifiziert
    // nichts. Der Brokerstart ist fuer eine Sonde in JEDEM Zustand zu; das
    // ist ihre Haelfte von "Scanner/Probe/Render spawnen nie Broker".
    pruefe (prozessor.klassifikation() == nakama::state::Klassifikation::unclassified,
            "frische Instanz ist neutral, trotz fester Produktklasse",
            nakama::state::wort (prozessor.klassifikation()));
    pruefe (! prozessor.darfBrokerStarten(),
            "eine Sonde darf den Broker nie starten");

    // -- 3. Passthrough, bitgleich ------------------------------------------
    const double raten[]   = { 44100.0, 48000.0, 96000.0 };
    const int    bloecke[] = { 1, 16, 64, 512, 4096 };

    for (const auto rate : raten)
    {
        for (const auto block : bloecke)
        {
            prozessor.setRateAndBufferSizeDetails (rate, block);
            prozessor.prepareToPlay (rate, block);

            juce::AudioBuffer<float> puffer (2, block);
            juce::AudioBuffer<float> kopie  (2, block);
            juce::MidiBuffer midi;
            juce::Random wuerfel ((int) rate + block);

            fuelle (puffer, wuerfel);
            kopie.makeCopyOf (puffer);

            prozessor.processBlock (puffer, midi);

            bool bitgleich = true;
            for (int k = 0; k < puffer.getNumChannels() && bitgleich; ++k)
                for (int n = 0; n < puffer.getNumSamples(); ++n)
                    if (std::memcmp (&puffer.getReadPointer (k)[n],
                                     &kopie.getReadPointer (k)[n], sizeof (float)) != 0)
                        { bitgleich = false; break; }

            pruefe (bitgleich,
                    "bitgleich bei " + juce::String (rate, 0) + " Hz, Block "
                        + juce::String (block));
            pruefe (prozessor.getLatencySamples() == 0,
                    "0 Samples Latenz bei " + juce::String (rate, 0) + " Hz, Block "
                        + juce::String (block),
                    juce::String (prozessor.getLatencySamples()));
            pruefe (midi.isEmpty(), "kein MIDI erzeugt");
        }
    }

    pruefe (prozessor.getTailLengthSeconds() == 0.0, "kein Tail",
            juce::String (prozessor.getTailLengthSeconds()));

    // -- 4. Stille bleibt still, NaN wird nicht erzeugt ---------------------
    {
        prozessor.setRateAndBufferSizeDetails (48000.0, 256);
        prozessor.prepareToPlay (48000.0, 256);
        juce::AudioBuffer<float> puffer (2, 256);
        juce::MidiBuffer midi;
        puffer.clear();
        prozessor.processBlock (puffer, midi);

        bool nurNull = true;
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < 256; ++n)
                if (puffer.getSample (k, n) != 0.0f || std::isnan (puffer.getSample (k, n)))
                    nurNull = false;
        pruefe (nurNull, "Stille bleibt bitgenau still");
    }

    // -- 5. Gegenpfad: speichern <-> laden ----------------------------------
    // Invariante des Hauses: beide Haelften im selben Aenderungssatz.
    {
        juce::MemoryBlock bytes;
        prozessor.getStateInformation (bytes);
        pruefe (bytes.getSize() > 0, "Zustand laesst sich speichern",
                juce::String ((int) bytes.getSize()) + " Bytes");

        const auto vorher = prozessor.zustandLesen().common;
        nakama::sonde::SondeProcessor zweiter;
        zweiter.setStateInformation (bytes.getData(), (int) bytes.getSize());

        pruefe (zweiter.zustandLesen().common == vorher,
                "geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)");
        pruefe (! zweiter.zustandLesen().nurLesen,
                "der eigene Stand kommt NICHT als read-only zurueck",
                zweiter.zustandLesen().grund);

        // §53.5 (S9 Abschnitt 3): "die beiden neuen Bundles haben eine feste
        // Produktklasse, bleiben aber bis gueltigem State neutral." Der
        // Restore ist der Moment, in dem aus neutral die Produktklasse wird.
        pruefe (zweiter.klassifikation()
                    == (nakama::sonde::kProduktklasse == nakama::state::Klasse::passive_probe
                            ? nakama::state::Klassifikation::passive_probe
                            : nakama::state::Klassifikation::active_probe),
                "nach gueltigem State traegt der Lebenslauf die Produktklasse",
                nakama::state::wort (zweiter.klassifikation()));

        juce::MemoryBlock zurueck;
        zweiter.getStateInformation (zurueck);
        pruefe (zurueck == bytes, "speichern -> laden -> speichern ist bytegleich",
                juce::String ((int) zurueck.getSize()) + " Bytes");
    }

    // -- 6. Muell aendert nichts --------------------------------------------
    {
        const auto vorher = prozessor.zustandLesen().common;
        const char muell[] = "das ist kein NakamaState";
        prozessor.setStateInformation (muell, (int) sizeof (muell));
        pruefe (prozessor.zustandLesen().common == vorher,
                "Muellbytes lassen den gehaltenen Zustand unveraendert");
        prozessor.setStateInformation (nullptr, 0);
        pruefe (prozessor.zustandLesen().common == vorher,
                "Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert");
    }

    std::cout << std::endl
              << (fehlerZahl == 0 ? "SONDE-NULLTEST OK - " : "SONDE-NULLTEST FEHLGESCHLAGEN - ")
              << okZahl << " Pruefungen ok, " << fehlerZahl << " Fehler" << std::endl;
    return fehlerZahl == 0 ? 0 : 1;
}
