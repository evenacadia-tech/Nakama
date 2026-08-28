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

    // -- 5b. Gate 7 auf State-Ebene, gemessen AM BUNDLE (G1 §4.2) ----------
    // Der Gate-Lauf G1 vom 24.08.2026 ist genau diese Kette gefahren:
    // Host-State-Restore -> lade() -> positionAusWort -> positionErlaubt ->
    // uebernommen -> beim naechsten Speichern wieder hinausgeschrieben. Eine
    // Sonde mit genau einem Stereo-Bus (SondeProcessor.cpp:7-9) konnte sich so
    // dauerhaft `post_fader_contribution` nennen - die exakte Bezeichnung
    // eines Mastersummenbeitrags auf einem Aux-Bus, den dieses Bundle nicht
    // hat. Das ist Gate 7 aus §49.2 Nr. 7 im Wortlaut.
    //
    // Der Riegel dagegen sitzt seit a2fe0f5 in `positionErlaubt`
    // (state/NakamaState.cpp): Riegel 1 sperrt die Position fuer JEDE Klasse,
    // solange kein Bau den Aux-Bus hat (`kContributionAuxVerfuegbar`, gemessen
    // unsupported), Riegel 2 ist die Klassenmatrix. Gemessen wurde er bisher
    // nur auf `lade()`-Ebene (EqCopStateMigrationTest G8b) und am Eqcp-Bundle.
    //
    // 🔑 Hier faehrt die GANZE Kette durch die echte Sondenschale. Das ist
    // keine Wiederholung: `SondeProcessor::setStateInformation` hat einen
    // eigenen Weg - bei `ignoriert` kehrt er VOR dem Schloss um, bei
    // `nurLesen` nicht -, und das Artefakt dieses Tickets ist das Bundle,
    // nicht `lade()`.
    {
        // Ein sonst GUELTIGER Stand dieses Bundles: eigene Produktklasse,
        // eigener Bundlevertrag, richtige Kind-Matrix (§2.1) - nur die
        // Messposition ist die verbotene. Gebaut ueber den oeffentlichen
        // State-Weg (`speichere`), nicht ueber eine Hintertuer im Produktcode.
        nakama::state::Zustand gebastelt =
            nakama::state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        gebastelt.common.klasse   = nakama::sonde::kProduktklasse;
        gebastelt.common.position = nakama::state::Messposition::post_fader_contribution;
        gebastelt.hatParameters   = (nakama::sonde::kProduktklasse
                                        == nakama::state::Klasse::active_probe);

        juce::MemoryBlock verboten;
        nakama::state::speichere (gebastelt, verboten);

        // Ohne diese Probe waere der ganze Block aus dem FALSCHEN Grund gruen:
        // haette `speichere` die Position stillschweigend begradigt, gaebe es
        // unten gar nichts mehr abzuwehren.
        const auto probe = juce::ValueTree::readFromData (verboten.getData(), verboten.getSize());
        const auto probeWort = probe.isValid()
            ? probe.getChildWithName ("Common").getProperty ("measurement_position").toString()
            : juce::String ("<kein Baum>");
        pruefe (probeWort == "post_fader_contribution",
                "die Probebytes tragen wirklich measurement_position=post_fader_contribution",
                probeWort);

        nakama::sonde::SondeProcessor opfer;
        opfer.setStateInformation (verboten.getData(), (int) verboten.getSize());

        // 1) Der Stand wird NICHT als eigener uebernommen: `leseSchema2`
        //    bricht an `positionErlaubt` ab, `lade()` faellt auf read-only mit
        //    Grund, und der Prozessor haelt genau diesen Zustand.
        pruefe (opfer.zustandLesen().nurLesen,
                "der Stand kommt als read-only zurueck, nicht als eigener",
                opfer.zustandLesen().grund);
        pruefe (opfer.zustandLesen().grund.isNotEmpty(),
                "read-only nennt seinen Grund");
        pruefe (opfer.zustandLesen().common.position
                    != nakama::state::Messposition::post_fader_contribution,
                "das Bundle FUEHRT die verbotene Position nicht",
                nakama::state::wort (opfer.zustandLesen().common.position));

        // 2) Die Instanz bleibt neutral - keine Klassifikation auf die
        //    Produktklasse, kein Brokerstart. §53.5: read-only ist das
        //    Gegenteil eines vollstaendigen State-Restore.
        pruefe (opfer.klassifikation() == nakama::state::Klassifikation::unclassified,
                "die Instanz klassifiziert NICHT auf die Produktklasse, sie bleibt neutral",
                nakama::state::wort (opfer.klassifikation()));
        pruefe (! opfer.darfBrokerStarten(),
                "eine read-only-Sonde darf den Broker nicht starten");

        // 3) §53.8 verlustfrei: die Originalbytes reisen unveraendert zum Host
        //    zurueck. Ein Altprojekt verliert seinen Stand nicht - es darf ihn
        //    nur nicht mehr behaupten.
        juce::MemoryBlock zurueck;
        opfer.getStateInformation (zurueck);
        pruefe (zurueck == verboten,
                "Vertrag 53.8: dieselben Originalbytes gehen unveraendert an den Host zurueck",
                juce::String ((int) zurueck.getSize()) + " Bytes");

        // 4) Und der Rueckweg waescht nichts: eine DRITTE frische Instanz, die
        //    genau die herausgegebenen Bytes laedt, kommt wieder read-only und
        //    neutral. Damit gibt es keinen Umweg, ueber den das Bundle die
        //    Position doch als gueltigen eigenen Stand fuehrt.
        nakama::sonde::SondeProcessor dritte;
        dritte.setStateInformation (zurueck.getData(), (int) zurueck.getSize());
        pruefe (dritte.zustandLesen().nurLesen,
                "der Rueckweg waescht nichts: erneut geladen bleibt read-only",
                dritte.zustandLesen().grund);
        pruefe (dritte.klassifikation() == nakama::state::Klassifikation::unclassified,
                "und erneut geladen bleibt die Instanz neutral",
                nakama::state::wort (dritte.klassifikation()));

        // 5) Gegenprobe: DERSELBE Stand mit einer fuer die Klasse erlaubten
        //    Position laedt normal und klassifiziert. Ohne sie wuesste
        //    niemand, ob oben die POSITION abgewiesen wurde oder irgendetwas
        //    anderes am Bastelstand.
        gebastelt.common.position = nakama::state::Messposition::insert;
        juce::MemoryBlock erlaubteBytes;
        nakama::state::speichere (gebastelt, erlaubteBytes);

        nakama::sonde::SondeProcessor gegenprobe;
        gegenprobe.setStateInformation (erlaubteBytes.getData(), (int) erlaubteBytes.getSize());
        pruefe (! gegenprobe.zustandLesen().nurLesen,
                "Gegenprobe: derselbe Stand mit erlaubter Position laedt normal",
                gegenprobe.zustandLesen().grund);
        pruefe (gegenprobe.zustandLesen().common == gebastelt.common,
                "Gegenprobe: der geladene Common ist derselbe (Position insert)",
                nakama::state::wort (gegenprobe.zustandLesen().common.position));
        pruefe (gegenprobe.klassifikation()
                    == (nakama::sonde::kProduktklasse == nakama::state::Klasse::passive_probe
                            ? nakama::state::Klassifikation::passive_probe
                            : nakama::state::Klassifikation::active_probe),
                "Gegenprobe: nach gueltigem Stand traegt der Lebenslauf die Produktklasse",
                nakama::state::wort (gegenprobe.klassifikation()));

        // 6) Und der Weg, den FL wirklich geht: die Instanz steht laengst im
        //    Projekt und ist klassifiziert, DANN reicht der Host ihr den
        //    verbotenen Stand nach (Preset-Browser, Copy/Paste, geoeffnetes
        //    Altprojekt). Punkt 1-5 haben nur frische Instanzen gemessen -
        //    haette der Riegel hier eine Luecke, waere sie die einzige, die im
        //    Betrieb ueberhaupt erreichbar ist.
        //
        //    §53.5: read-only ist das Gegenteil eines vollstaendigen Restore,
        //    also faellt auch eine ZUVOR positiv klassifizierte Instanz auf
        //    neutral zurueck. Sie darf ihre Rechte nicht behalten, nur weil sie
        //    sie einmal hatte.
        pruefe (gegenprobe.klassifikation() != nakama::state::Klassifikation::unclassified,
                "Nachreichen: die Instanz ist VOR dem verbotenen Stand klassifiziert",
                nakama::state::wort (gegenprobe.klassifikation()));

        gegenprobe.setStateInformation (verboten.getData(), (int) verboten.getSize());

        pruefe (gegenprobe.zustandLesen().nurLesen,
                "Nachreichen: der verbotene Stand kommt auch bei einer laufenden Instanz read-only",
                gegenprobe.zustandLesen().grund);
        pruefe (gegenprobe.klassifikation() == nakama::state::Klassifikation::unclassified,
                "Nachreichen: die Klassifikation faellt zurueck auf neutral, alte Rechte bleiben nicht",
                nakama::state::wort (gegenprobe.klassifikation()));

        // §53.8 auch hier: der Prozessor erfindet keinen Stand und faellt auch
        // nicht auf den vorherigen zurueck - er gibt heraus, was der Host ihm
        // gegeben hat. Alles andere waere ein stiller Stand-Tausch hinter dem
        // Ruecken des Projekts.
        juce::MemoryBlock nachReichen;
        gegenprobe.getStateInformation (nachReichen);
        pruefe (nachReichen == verboten,
                "Nachreichen: der Host bekommt genau die Bytes zurueck, die er gab - kein stiller Tausch",
                juce::String ((int) nachReichen.getSize()) + " Bytes");
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
