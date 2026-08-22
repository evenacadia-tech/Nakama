// Identitaets-Golden (SONDE-001) + Schema-1-State-Goldens (SONDE-002).
//
// Die VST3-Identitaet ist ab P0 ein Dateiformat: aendert sie sich, verlieren
// bestehende FL-Projekte ihr Plugin. Dieser Test misst das GEBAUTE Bundle
// gegen eq-copilot/identity/plugin-identities-v1.json - nicht das Manifest
// gegen sich selbst.
//
// Er rechnet ausserdem die vier reservierten CIDs der kuenftigen Sondenziele
// aus (Herstellercode, Plugincode) NACH, statt sie zu glauben (Pruefbericht
// Befund C: "in P0 verifizieren statt erzeugen"), und beweist positiv, dass
// JUCE_VST3_CAN_REPLACE_VST2 aus ist: die echte CID ist nicht die, die der
// VST2-Ersatzpfad erzeugen wuerde.
//
// Aufruf:
//   EqCopIdentityTest [--schreibe-goldens]
// Pfade werden relativ zum Arbeitsverzeichnis (Workspace-Wurzel) gesucht.
// Exit 0 nur bei "IDENTITY-TEST OK".
#include "PluginProcessor.h"

#include <cstring>
#include <iostream>

using namespace eqcop;

namespace
{

int fehler = 0;
int bestanden = 0;

void pruefe (bool ok, const juce::String& name, const juce::String& zusatz = {})
{
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << name.toRawUTF8();
    if (zusatz.isNotEmpty())
        std::cout << "  [" << zusatz.toRawUTF8() << "]";
    std::cout << std::endl;
    if (ok) ++bestanden; else ++fehler;
}

/** Sucht eine Datei relativ zum Arbeitsverzeichnis und, falls das danebengeht,
    relativ zur laufenden Programmdatei - damit der Test auch dann etwas
    Verstaendliches sagt, wenn ihn jemand aus dem Bauordner heraus startet. */
juce::File finde (const juce::String& relativ)
{
    auto ausCwd = juce::File::getCurrentWorkingDirectory().getChildFile (relativ);
    if (ausCwd.existsAsFile())
        return ausCwd;

    auto ordner = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
    for (int i = 0; i < 8 && ordner.exists(); ++i)
    {
        auto kandidat = ordner.getChildFile (relativ);
        if (kandidat.existsAsFile())
            return kandidat;
        ordner = ordner.getParentDirectory();
    }
    return ausCwd;   // fuer die Fehlermeldung: der erwartete Ort
}

/** Der VST3-Schreiber setzt Kommas hinter das LETZTE Element eines Objekts
    oder Arrays; JUCEs JSON-Leser ist strenger. Diese Funktion entfernt genau
    diese Kommas und sonst nichts - Zeichenketten bleiben unangetastet, damit
    ein Komma in einem Namen nicht verlorengeht. */
juce::String ohneNachlaufkommas (const juce::String& text)
{
    juce::String sauber;
    bool inText = false;
    bool maskiert = false;
    const auto* z = text.toRawUTF8();

    for (size_t i = 0; z[i] != 0; ++i)
    {
        const char c = z[i];

        if (inText)
        {
            sauber << c;
            if (maskiert)          maskiert = false;
            else if (c == '\\')    maskiert = true;
            else if (c == '"')     inText = false;
            continue;
        }

        if (c == '"')
        {
            inText = true;
            sauber << c;
            continue;
        }

        if (c == ',')
        {
            size_t j = i + 1;
            while (z[j] == ' ' || z[j] == '\t' || z[j] == '\r' || z[j] == '\n')
                ++j;
            if (z[j] == '}' || z[j] == ']')
                continue;          // Nachlaufkomma: weglassen
        }

        sauber << c;
    }

    return sauber;
}

/** Liefert den Textblock eines CMake-Aufrufs ab `marker` bis zur Zeile, die
    nur aus ")" besteht. So wird die Quelle blockgenau geprueft statt global. */
juce::String cmakeBlock (const juce::String& text, const juce::String& marker)
{
    const auto start = text.indexOf (marker);
    if (start < 0)
        return {};
    const auto ende = text.indexOf (start, "\n)");
    return ende < 0 ? text.substring (start) : text.substring (start, ende);
}

juce::String alsHex (const juce::VST3Interface::Id& id)
{
    juce::String s;
    for (auto b : id)
        s << juce::String::toHexString ((int) (unsigned char) b).paddedLeft ('0', 2).toUpperCase();
    return s;
}

/** jucePluginId liefert den TUID so, wie er im Speicher liegt. Unter
    COM_COMPATIBLE (Windows) sind die ersten acht Bytes paarweise vertauscht
    (VST3-SDK, funknownimpl.h, UID::toTUID) - genau diese Vertauschung macht
    JUCE selbst in juce_VST3Interface.h. Die CID-Zeichenkette in
    moduleinfo.json steht dagegen in logischer Reihenfolge. Der Tausch ist
    seine eigene Umkehrung. */
juce::VST3Interface::Id inLogischeReihenfolge (juce::VST3Interface::Id id)
{
   #if JUCE_WINDOWS
    std::swap (id[0], id[3]);
    std::swap (id[1], id[2]);
    std::swap (id[4], id[5]);
    std::swap (id[6], id[7]);
   #endif
    return id;
}

juce::uint32 codeAlsZahl (const juce::String& vierZeichen)
{
    jassert (vierZeichen.length() == 4);
    const auto* z = vierZeichen.toRawUTF8();
    return ((juce::uint32) (unsigned char) z[0] << 24)
         | ((juce::uint32) (unsigned char) z[1] << 16)
         | ((juce::uint32) (unsigned char) z[2] << 8)
         |  (juce::uint32) (unsigned char) z[3];
}

juce::String berechneteCid (const juce::String& herstellerCode,
                            const juce::String& pluginCode,
                            juce::VST3Interface::Type typ)
{
    return alsHex (inLogischeReihenfolge (
        juce::VST3Interface::jucePluginId (codeAlsZahl (herstellerCode),
                                           codeAlsZahl (pluginCode),
                                           typ)));
}

/** Baut einen Schema-1-Zustand mit FESTEN Werten - ohne feste Sensor-ID waere
    das Golden bei jedem Lauf ein anderes. */
juce::MemoryBlock saatZustand (const juce::String& rolle,
                               const juce::String& label,
                               const juce::String& paarId)
{
    juce::ValueTree v ("EqCopilotState");
    v.setProperty ("schema", 1, nullptr);
    v.setProperty ("sensor_id", "11111111-2222-3333-4444-555555555555", nullptr);
    v.setProperty ("role", rolle, nullptr);
    v.setProperty ("label", label, nullptr);
    v.setProperty ("pair_id", paarId, nullptr);

    juce::MemoryBlock block;
    juce::MemoryOutputStream strom (block, false);
    v.writeToStream (strom);
    strom.flush();
    return block;
}

struct Rolle { const char* name; const char* label; const char* paar; };

const Rolle kRollen[] = {
    { "sensor", "Klavier A",  "" },
    { "hub",    "Leitstand",  "" },
    { "pre",    "Chor PRE",   "paar-chor" },
    { "post",   "Chor POST",  "paar-chor" }
};

} // namespace

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceStart;

    bool schreibeGoldens = false;
    for (int i = 1; i < argc; ++i)
        if (juce::String (argv[i]) == "--schreibe-goldens")
            schreibeGoldens = true;

    // -- Manifest laden ---------------------------------------------------
    const auto manifestDatei = finde ("eq-copilot/identity/plugin-identities-v1.json");
    pruefe (manifestDatei.existsAsFile(), "Identitaetsmanifest gefunden",
            manifestDatei.getFullPathName());
    if (! manifestDatei.existsAsFile())
    {
        std::cout << std::endl << "IDENTITY-TEST FEHLGESCHLAGEN - Manifest fehlt" << std::endl;
        return 1;
    }

    const auto manifest = juce::JSON::parse (manifestDatei.loadFileAsString());
    const auto* mObj = manifest.getDynamicObject();
    pruefe (mObj != nullptr, "Manifest ist gueltiges JSON");
    if (mObj == nullptr)
    {
        std::cout << std::endl << "IDENTITY-TEST FEHLGESCHLAGEN" << std::endl;
        return 1;
    }

    const auto herstellerCode = manifest["hersteller"]["code"].toString();
    pruefe (herstellerCode == "Evna", "Herstellercode im Manifest", herstellerCode);

    const auto ziele = manifest["ziele"];
    pruefe (ziele.isArray() && ziele.size() == 3, "Manifest kennt drei Ziele",
            juce::String (ziele.isArray() ? ziele.size() : -1));

    juce::var mainZiel;
    for (int i = 0; i < ziele.size(); ++i)
        if (ziele[i]["id"].toString() == "main")
            mainZiel = ziele[i];
    pruefe (mainZiel.isObject(), "Manifest kennt das heutige Ziel 'main'");

    // -- Die Quelle einfrieren, nicht nur das Artefakt --------------------
    // Ein Freeze, der nur ein gebautes Artefakt prueft, greift genau dann
    // nicht, wenn niemand baut. Deshalb wird zuerst der Quelltext gemessen:
    // wer PLUGIN_CODE aendert oder JUCE_VST3_CAN_REPLACE_VST2 auf 1 stellt,
    // faellt hier auf - auch ohne Bau.
    //
    // S9/SONDE-007b (23.08.2026, NAK-52): Bis dahin fror dieser Block den
    // LITERALEN Text ein ("PLUGIN_CODE Eqcp"). Das war richtig, solange das
    // Bauskript die Identitaet selbst trug - und wurde falsch, als sie ins
    // Manifest zog: der Test haette dann eine Kopie gegen ihr Original
    // gemessen und beim Abweichen nicht sagen koennen, welche Seite luegt.
    // Jetzt misst er das Gegenteil: dass im Zielblock KEINE zweite Wahrheit
    // mehr steht und die Werte aus dem Manifest kommen. Der eigentliche
    // Identitaetsbeweis bleibt das gebaute moduleinfo.json weiter unten -
    // zwei Wege zur selben Zahl.
    const auto cmakeDatei = finde ("eq-copilot/plugin/CMakeLists.txt");
    pruefe (cmakeDatei.existsAsFile(), "plugin/CMakeLists.txt gefunden");

    if (cmakeDatei.existsAsFile())
    {
        // Zeilenenden vereinheitlichen. core.autocrlf=true steht hier in der
        // System-Gitconfig: jeder frische Clone materialisiert die Datei mit
        // CRLF. Ohne diese Zeile faende die Blocksuche den Zielblock nicht und
        // meldete einen Identitaetsbruch, den es gar nicht gibt - ein
        // Fehlalarm auf genau dem Riegel, dem man ab jetzt glauben soll.
        // (Gefunden vom T2-Pruefer, Runde 2.)
        const auto cmakeText = cmakeDatei.loadFileAsString().replace ("\r\n", "\n");
        const auto zielBlock = cmakeBlock (cmakeText, "juce_add_plugin(EqCopilot\n");
        const auto defBlock  = cmakeBlock (cmakeText, "target_compile_definitions(EqCopilot PUBLIC");

        pruefe (zielBlock.isNotEmpty(), "CMake: Zielblock juce_add_plugin(EqCopilot) gefunden");
        pruefe (defBlock.isNotEmpty(), "CMake: Defineblock von EqCopilot gefunden");

        // (1) Das Bauskript liest das Manifest ueberhaupt.
        pruefe (cmakeText.contains ("nakama_identitaet_lesen(main NAKAMA_MAIN)"),
                "CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest");

        // (2) Die vier Identitaetszeilen kommen von dort - nicht aus dieser Datei.
        pruefe (zielBlock.contains ("PLUGIN_CODE ${NAKAMA_MAIN_PLUGINCODE}"),
                "CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest");
        pruefe (zielBlock.contains ("PLUGIN_MANUFACTURER_CODE ${NAKAMA_MAIN_HERSTELLERCODE}"),
                "CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest");
        pruefe (zielBlock.contains ("PRODUCT_NAME \"${NAKAMA_MAIN_PRODUKTNAME}\""),
                "CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest");
        pruefe (zielBlock.contains ("COMPANY_NAME \"${NAKAMA_MAIN_HERSTELLER}\""),
                "CMake-Quelle: COMPANY_NAME kommt aus dem Manifest");

        // (3) Und es steht keine zweite Wahrheit daneben. Ohne diese Zeilen
        //     koennte jemand die Manifestzeile stehen lassen und darunter ein
        //     Literal setzen - CMake naehme das letzte, der Test saehe nichts.
        //     Gesucht wird im ZIELBLOCK, nicht in der Datei: NkSp und NkHp
        //     (Wegwerf-Messgeraete) tragen ihre Codes zu Recht literal, sie
        //     stehen ausdruecklich NICHT im Manifest.
        pruefe (! zielBlock.contains (mainZiel["plugin_code"].toString()),
                "CMake-Quelle: der Viercode steht nicht literal im Zielblock",
                mainZiel["plugin_code"].toString());
        pruefe (! zielBlock.contains (herstellerCode),
                "CMake-Quelle: der Herstellercode steht nicht literal im Zielblock",
                herstellerCode);
        pruefe (! zielBlock.contains ("\"" + mainZiel["produktname"].toString() + "\""),
                "CMake-Quelle: der Produktname steht nicht literal im Zielblock");
        pruefe (defBlock.contains ("JUCE_VST3_CAN_REPLACE_VST2=0"),
                "CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock");
        pruefe (! defBlock.contains ("JUCE_VST3_CAN_REPLACE_VST2=1"),
                "CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet");
        // Und nirgends sonst in der Datei - sonst genuegte ein globales
        // add_compile_definitions(), um am Zielblock vorbei umzuschalten.
        pruefe (! cmakeText.contains ("JUCE_VST3_CAN_REPLACE_VST2=1"),
                "CMake-Quelle: das Define steht auch sonst nirgends auf 1");
    }

    // -- Das gebaute Bundle gegen das Manifest ----------------------------
    const auto moduleinfo = finde ("eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/"
                                   "EQ-Copilot.vst3/Contents/Resources/moduleinfo.json");
    pruefe (moduleinfo.existsAsFile(), "moduleinfo.json des gebauten Bundles gefunden",
            moduleinfo.getFullPathName());

    if (moduleinfo.existsAsFile() && cmakeDatei.existsAsFile())
    {
        // Frischeriegel: ein moduleinfo, das aelter ist als die Bauvorschrift,
        // beweist nichts ueber den heutigen Stand.
        const auto artefaktZeit = moduleinfo.getLastModificationTime();
        const auto quellZeit    = cmakeDatei.getLastModificationTime();
        pruefe (artefaktZeit >= quellZeit,
                "moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt",
                artefaktZeit.toString (true, true) + " vs " + quellZeit.toString (true, true));
    }

    if (moduleinfo.existsAsFile())
    {
        const auto roh = moduleinfo.loadFileAsString();
        const auto gelesen = juce::JSON::parse (ohneNachlaufkommas (roh));
        pruefe (gelesen.isObject(), "moduleinfo.json ist nach dem Kommaputz parsebar");

        const auto componentCid  = mainZiel["component_cid"].toString();
        const auto controllerCid = mainZiel["controller_cid"].toString();
        const auto produktname   = mainZiel["produktname"].toString();

        pruefe (gelesen["Name"].toString() == produktname,
                "moduleinfo: Produktname wie im Manifest", gelesen["Name"].toString());
        pruefe (gelesen["Factory Info"]["Vendor"].toString()
                    == manifest["hersteller"]["name"].toString(),
                "moduleinfo: Vendor wie im Manifest",
                gelesen["Factory Info"]["Vendor"].toString());

        const auto klassen = gelesen["Classes"];
        pruefe (klassen.isArray() && klassen.size() == 2,
                "moduleinfo: genau zwei Klassen (Component + Controller)",
                juce::String (klassen.isArray() ? klassen.size() : -1));

        // Die CID wird an IHRE Klasse gebunden - ein Textfund irgendwo in der
        // Datei wuerde auch dann passen, wenn die IDs vertauscht waeren.
        juce::String cidAudio, cidController;
        for (int i = 0; i < klassen.size(); ++i)
        {
            const auto kategorie = klassen[i]["Category"].toString();
            if (kategorie == "Audio Module Class")            cidAudio      = klassen[i]["CID"].toString();
            else if (kategorie == "Component Controller Class") cidController = klassen[i]["CID"].toString();
        }

        pruefe (cidAudio == componentCid,
                "moduleinfo: die Audio-Module-Klasse traegt die Component-CID", cidAudio);
        pruefe (cidController == controllerCid,
                "moduleinfo: die Controller-Klasse traegt die Controller-CID", cidController);

        const auto kategorien = mainZiel["kategorien"];
        bool alleKategorien = kategorien.isArray() && kategorien.size() > 0;
        for (int i = 0; i < kategorien.size(); ++i)
        {
            bool gefunden = false;
            const auto unter = klassen[0]["Sub Categories"];
            for (int k = 0; k < unter.size(); ++k)
                if (unter[k].toString() == kategorien[i].toString())
                    gefunden = true;
            if (! gefunden)
                alleKategorien = false;
        }
        pruefe (alleKategorien, "moduleinfo: Unterkategorien wie im Manifest");

        // Gegenprobe: KEINE der reservierten Sonden-CIDs darf im heutigen
        // Bundle auftauchen - alle vier, nicht nur eine.
        bool fremdeGefunden = false;
        for (int i = 0; i < ziele.size(); ++i)
        {
            if (ziele[i]["id"].toString() == "main")
                continue;
            if (roh.contains (ziele[i]["component_cid"].toString())
                || roh.contains (ziele[i]["controller_cid"].toString()))
                fremdeGefunden = true;
        }
        pruefe (! fremdeGefunden,
                "moduleinfo: keine der vier reservierten Sonden-CIDs im heutigen Bundle");
    }

    // -- Ableitung nachrechnen statt glauben ------------------------------
    for (int i = 0; i < ziele.size(); ++i)
    {
        const auto ziel = ziele[i];
        const auto id   = ziel["id"].toString();
        const auto code = ziel["plugin_code"].toString();

        const auto erwarteteComponent  = ziel["component_cid"].toString();
        const auto erwarteterController = ziel["controller_cid"].toString();

        const auto istComponent  = berechneteCid (herstellerCode, code,
                                                  juce::VST3Interface::Type::component);
        const auto istController = berechneteCid (herstellerCode, code,
                                                  juce::VST3Interface::Type::controller);

        pruefe (istComponent == erwarteteComponent,
                "CID nachgerechnet: " + id + " Component (" + code + ")", istComponent);
        pruefe (istController == erwarteterController,
                "CID nachgerechnet: " + id + " Controller (" + code + ")", istController);
    }

    // -- Positiver Beweis: JUCE_VST3_CAN_REPLACE_VST2 ist AUS -------------
    {
        const auto code        = mainZiel["plugin_code"].toString();
        const auto produktname = mainZiel["produktname"].toString();
        const auto echteCid    = mainZiel["component_cid"].toString();

        const auto vst2Cid = alsHex (inLogischeReihenfolge (
            juce::VST3Interface::vst2PluginId (codeAlsZahl (code),
                                               produktname.toRawUTF8(),
                                               juce::VST3Interface::Type::component)));

        pruefe (vst2Cid != echteCid,
                "VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus",
                vst2Cid);
        pruefe (echteCid == berechneteCid (herstellerCode, code,
                                           juce::VST3Interface::Type::component),
                "die echte CID stammt aus dem jucePluginId-Pfad");
    }

    // -- Schema-1-State-Goldens (SONDE-002, headless-Teil) ----------------
    {
        const auto goldenOrdner = finde ("eq-copilot/identity/plugin-identities-v1.json")
                                      .getParentDirectory()
                                      .getSiblingFile ("fixtures")
                                      .getChildFile ("identity");
        if (schreibeGoldens)
            goldenOrdner.createDirectory();

        pruefe (goldenOrdner.isDirectory() || schreibeGoldens,
                "Golden-Ordner vorhanden", goldenOrdner.getFullPathName());

        for (const auto& rolle : kRollen)
        {
            EqCopilotProcessor p;
            const auto saat = saatZustand (rolle.name, rolle.label, rolle.paar);
            p.setStateInformation (saat.getData(), (int) saat.getSize());

            pruefe (p.holeRolle() == rolle.name,
                    juce::String ("Rolle '") + rolle.name + "' geladen", p.holeRolle());
            pruefe (p.holeSensorId() == "11111111-2222-3333-4444-555555555555",
                    juce::String ("Rolle '") + rolle.name + "': Sensor-ID unveraendert uebernommen");
            pruefe (p.holeLabel() == rolle.label,
                    juce::String ("Rolle '") + rolle.name + "': Label uebernommen", p.holeLabel());
            pruefe (p.holePaarId() == rolle.paar,
                    juce::String ("Rolle '") + rolle.name + "': Paar-ID uebernommen", p.holePaarId());

            // Die Schema-1-Goldens sind seit SONDE-006 EINGEFRORENE LADE-Fixtures:
            // das Plugin speichert Schema 2 (NakamaState). Der Save-Beweis
            // (Migration bytegleich zum Schema-2-Golden) liegt im
            // EqCopStateMigrationTest; hier bleibt der Riegel, dass die
            // eingefrorene Saat weiter bytegleich auf Platte liegt und dass
            // sie in eine frische Instanz laedt.
            auto golden = goldenOrdner.getChildFile (juce::String ("state-schema1-") + rolle.name + ".bin");
            if (schreibeGoldens)
            {
                golden.replaceWithData (saat.getData(), saat.getSize());
                std::cout << "  geschrieben: " << golden.getFullPathName().toRawUTF8() << std::endl;
                continue;
            }

            juce::MemoryBlock erwartet;
            const bool gelesen = golden.existsAsFile() && golden.loadFileAsData (erwartet);
            pruefe (gelesen, juce::String ("Golden '") + rolle.name + "' gelesen",
                    golden.getFileName());
            if (! gelesen)
                continue;

            pruefe (erwartet.getSize() == saat.getSize()
                        && std::memcmp (erwartet.getData(), saat.getData(), saat.getSize()) == 0,
                    juce::String ("Schema-1-Saat '") + rolle.name + "' ist bytegleich zum eingefrorenen Golden",
                    juce::String ((int) saat.getSize()) + " Bytes");

            juce::MemoryBlock heraus;
            p.getStateInformation (heraus);
            const auto neu = juce::ValueTree::readFromData (heraus.getData(), heraus.getSize());
            pruefe (neu.hasType ("NakamaState") && (int) neu.getProperty ("schema") == 2,
                    juce::String ("Rolle '") + rolle.name + "' speichert Schema 2 (Migration, SONDE-006)");

            // Gegenpfad: das Golden wieder einlesen muss dieselben Felder ergeben.
            EqCopilotProcessor zweite;
            zweite.setStateInformation (erwartet.getData(), (int) erwartet.getSize());
            pruefe (zweite.holeRolle() == rolle.name && zweite.holeLabel() == rolle.label
                        && zweite.holePaarId() == rolle.paar,
                    juce::String ("Golden '") + rolle.name + "' laedt in eine frische Instanz zurueck");
        }
    }

    // -- Fremdes Format darf den Zustand nicht kapern ---------------------
    {
        EqCopilotProcessor p;
        const auto vorher = p.holeSensorId();
        juce::ValueTree fremd ("IrgendeinAnderesPlugin");
        fremd.setProperty ("sensor_id", "gekapert", nullptr);
        juce::MemoryBlock block;
        juce::MemoryOutputStream strom (block, false);
        fremd.writeToStream (strom);
        strom.flush();

        p.setStateInformation (block.getData(), (int) block.getSize());
        pruefe (p.holeSensorId() == vorher,
                "fremder ValueTree-Typ wird abgelehnt, Zustand bleibt");

        const char muell[] = { 'n', 'i', 'c', 'h', 't', 's' };
        p.setStateInformation (muell, (int) sizeof (muell));
        pruefe (p.holeSensorId() == vorher, "Muellbytes aendern den Zustand nicht");
    }

    std::cout << std::endl
              << (fehler == 0 ? "IDENTITY-TEST OK" : "IDENTITY-TEST FEHLGESCHLAGEN")
              << " - " << bestanden << " Pruefungen ok, " << fehler << " Fehler" << std::endl;
    return fehler == 0 ? 0 : 1;
}
