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

#include <algorithm>
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

bool istHex (const std::string& text, std::size_t laenge)
{
    return text.size() == laenge
        && std::all_of (text.begin(), text.end(), [] (char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
}

juce::File wurzel()
{
    auto d = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    while (d.exists() && ! d.getChildFile ("eq-copilot").isDirectory())
        d = d.getParentDirectory();
    return d;
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

    // -- 5a. Probeeq ist ein gebundener reiner v3-Connector ---------------
    // Die Testschale startet bewusst keine Produktpipe; sie greift aber auf
    // die echten, im Produktprozessor gehaltenen Control-/Telemetry-Clients
    // und deren echte Provider zu. Entfernt man die Verdrahtung, baut bzw.
    // besteht dieser Block nicht mehr.
    {
        nakama::state::Zustand gebunden =
            nakama::state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        gebunden.common.klasse = nakama::state::Klasse::active_probe;
        gebunden.common.projectBindingId = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
        gebunden.hatParameters = true;
        juce::MemoryBlock bytes;
        nakama::state::speichere (gebunden, bytes);

        nakama::sonde::SondeProcessor connector;
        connector.setStateInformation (bytes.getData(), (int) bytes.getSize());
        connector.setRateAndBufferSizeDetails (48000.0, 512);
        connector.prepareToPlay (48000.0, 512);
        const auto hello = connector.v3HelloFuerTest();
        const auto status = connector.v3StatusFuerTest();
        juce::String sollHash, grund;
        const bool hashOk = nakama::parameter::stateHash (
            gebunden.parameters, sollHash, grund);

        pruefe (hello.adresse.projectBindingId
                    == "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
                 && hello.adresse.instanceId
                    == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                 && hello.pluginKind == "active_probe"
                 && hello.adresse.sessionEpoch
                    == hello.adresse.projectBindingId
                 && istHex (hello.adresse.runtimeNonce, 32),
                "Probeeq-Controlprovider erfindet keine eigene Session-Epoche");
        pruefe (status.dspSchemaVersion == nakama::parameter::kDspSchemaVersion
                    && status.stateRevision == 1
                    && hashOk && status.stateHash == sollHash.toStdString()
                    && ! status.recordStateValid && ! status.recording,
                "Probeeq-Stateprovider meldet echten Parameterhash und keinen erfundenen Record-State");
        pruefe (connector.controlV3FuerTest().status
                    == nakama::ipc::ControlClient::Status::getrennt
                    && connector.telemetryV3FuerTest().status
                    == nakama::ipc::TelemetryClient::Status::getrennt
                    && ! connector.darfBrokerStarten(),
                "Testschale belegt beide Connectoren; Probeeq besitzt keinen Startpfad");
        pruefe (connector.v3ProduktstatusVerdrahtetFuerTest(),
                "Probeeq-ControlClient traegt seinen produktiven Statusprovider");

        const auto produktquelle = wurzel()
            .getChildFile ("eq-copilot/plugin/sonde/SondeProcessor.cpp")
            .loadFileAsString();
        pruefe (produktquelle.contains ("controlV3.start();")
                    && produktquelle.contains ("telemetryV3.start();")
                    && ! produktquelle.contains ("BrokerLifecycle"),
                "Probeeq startet produktiv beide v3-Connectoren, aber niemals einen Broker");
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
    // (state/NakamaState.cpp) und hat zwei Haelften: Riegel 1 ist die
    // CAPABILITY-Vorpruefung (`kContributionAuxVerfuegbar`, heute gemessen
    // unsupported), Riegel 2 die Klassenmatrix. Beide lehnen
    // `post_fader_contribution` heute fuer jede Klasse ab; von aussen ist nur
    // das GEMEINSAME Ergebnis sichtbar, nicht welche Haelfte greift. Die
    // VOLLSTAENDIGE Matrix - alle vier Klassen, jede in dem Bundle, das sie
    // zulaesst, `passive_probe` also mit Bundlevertrag `nkpr()` - misst B2
    // `EqCopStateMigrationTest` (Block G8b) auf `lade()`-Ebene.
    //
    // 🔑 Hier faehrt die GANZE Kette durch die echte Sondenschale. Das ist
    // keine Wiederholung: `SondeProcessor::setStateInformation` hat einen
    // eigenen Weg - bei `ignoriert` kehrt er VOR dem Schloss um, bei
    // `nurLesen` nicht -, und das Artefakt dieses Tickets ist das Bundle,
    // nicht `lade()`.
    //
    // ⚠️ WIE WEIT DAS TRAEGT - genau und nicht weiter: die Schale traegt
    // GENAU EINE Klasse je Uebersetzung (`kProduktklasse`), und gebaut wird
    // seit S9b/`SONDE-007c` nur noch `active_probe`. Der Durchgriff durch die
    // Schale ist damit fuer DIESE EINE Klasse gemessen, nicht fuer vier. Dass
    // `positionErlaubt` die Position fuer ALLE VIER Klassen ablehnt, misst
    // Punkt 7 unten (direkt an der Funktion) und B2 auf `lade()`-Ebene - beide
    // messen das gemeinsame Ergebnis beider Riegelhaelften, keine von beiden
    // trennt sie auf. Der urspruengliche G1-§4.2-Traeger - eine passive Sonde
    // als PRODUKT - existiert seit S9b nicht mehr.
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

        // 7) `positionErlaubt` lehnt `post_fader_contribution` fuer ALLE VIER
        //    Klassen ab - gemessen an genau der Funktion, an der Punkt 0-6
        //    oben abbiegen.
        //
        //    ⚠️ WAS DIESE PRUEFUNG IST UND WAS SIE NICHT IST. Gemessen wird das
        //    GEMEINSAME Ergebnis beider Riegelhaelften: Riegel 1 (die
        //    Capability-Vorpruefung `kContributionAuxVerfuegbar`) und Riegel 2
        //    (die Klassenmatrix) lehnen die Position heute beide ab. WELCHE
        //    Haelfte im Einzelfall sperrt, sagt diese Pruefung NICHT -
        //    `positionErlaubt` gibt nur `false` zurueck, und eine Abfrage, die
        //    die Haelften trennt, gibt es im Kern nicht. Die Aussage bleibt
        //    deshalb genau: abgelehnt fuer alle vier Klassen.
        //
        //    Punkt 0-6 fahren die ganze Kette durch das echte Bundle, aber nur
        //    fuer EINE Klasse: `kProduktklasse` ist ein Uebersetzungsschalter
        //    (`plugin/CMakeLists.txt`, `nakama_sonde_nulltest`), und seit
        //    S9b/`SONDE-007c` setzt KEIN Bauziel mehr `NAKAMA_SONDE_PASSIV` -
        //    Nakama Suna ist stillgelegt (`SondeProcessor.h:5-12`). Diese
        //    Schale kann heute also nur `active_probe` bauen.
        //
        //    Der urspruengliche Traeger der G1-§4.2-Regression war aber
        //    gerade `passive_probe`: vor `a2fe0f5` gab es Riegel 1 nicht und
        //    die Matrix trug `case Klasse::passive_probe: return true;`. Setzt
        //    man beides auf jenen Stand zurueck, bleiben Punkt 0-6 alle gruen
        //    - sie werden weiter an der unveraenderten `active_probe`-Zeile
        //    abgewiesen. Ohne die vier Zeilen hier faenge dieses Bein die
        //    Regression, gegen die es antritt, NICHT; mit ihnen faellt genau
        //    die `passive_probe`-Zeile (Mutationsprobe Runde 2, SONDE-007b).
        //
        //    Ein passives Bauziel wiederzubeleben ist der falsche Weg dagegen
        //    (es naehme `SONDE-007c` zurueck), eine Testhintertuer im
        //    Produktcode ebenso. Also wird die Ablehnung direkt an der
        //    oeffentlichen `positionErlaubt` gemessen: derselben Funktion, die
        //    die Schale oben ueber `lade()` aufruft.
        //
        //    Die VOLLSTAENDIGE 16er-Matrix samt Bundlevertraegen - jede Klasse
        //    in dem Bundle, das sie zulaesst, `passive_probe` also in `nkpr()`
        //    - misst B2 `EqCopStateMigrationTest`, Block G8b. Nicht dieses
        //    Bein, und das soll es auch nicht: hier steht die Kette am
        //    gebauten Bundle, dort der Vertrag.
        for (const auto klasse : { nakama::state::Klasse::main,
                                   nakama::state::Klasse::passive_probe,
                                   nakama::state::Klasse::active_probe,
                                   nakama::state::Klasse::legacy })
            pruefe (! nakama::state::positionErlaubt (
                        klasse, nakama::state::Messposition::post_fader_contribution),
                    juce::String ("positionErlaubt (Riegel 1 und Klassenmatrix gemeinsam) "
                                  "lehnt post_fader_contribution ab fuer '")
                        + nakama::state::wort (klasse) + "'");
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
