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

/** Liefert die erste Zeile, die `marker` enthaelt - oder einen leeren String.
    Fuer Aufrufe, die auf EINER Zeile stehen: dort ist die Zeile die Einheit,
    nicht der Block. */
juce::String zeileMit (const juce::String& text, const juce::String& marker)
{
    for (const auto& zeile : juce::StringArray::fromLines (text))
        if (zeile.contains (marker))
            return zeile;
    return {};
}

/** Der Manifesteintrag zu einer Ziel-ID, oder ein leeres var. */
juce::var zielMitId (const juce::var& ziele, const juce::String& id)
{
    for (int i = 0; i < ziele.size(); ++i)
        if (ziele[i]["id"].toString() == id)
            return ziele[i];
    return {};
}

/** Traegt dieses Ziel ueberhaupt eine Stilllegungsmarke - unabhaengig davon,
    ob ihr Inhalt lesbar ist?

    S9b/SONDE-007c, Nacharbeit Runde 1 (29.08.2026, T2-Befund P1 Nr. 1):
    hier stand `ziel["stillgelegt"].isObject()`, und das war fail-OPEN. JUCE
    bildet JSON-`null` auf ein leeres `var` ab (juce_JSON.cpp, "case 'n'" ->
    `return {};`) - genau dasselbe, was `DynamicObject::getProperty` fuer einen
    FEHLENDEN Schluessel liefert. Ueber `operator[]` sind "Marke ist null" und
    "keine Marke" damit prinzipiell ununterscheidbar, und ein stillgelegtes
    Ziel mit `"stillgelegt": null` zaehlte als AKTIV. Nur `hasProperty` kann
    die Frage beantworten, die hier wirklich gestellt wird. */
bool hatStilllegungsmarke (const juce::var& ziel)
{
    if (auto* o = ziel.getDynamicObject())
        return o->hasProperty ("stillgelegt");
    return false;
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

/** S9-Nacharbeit 23.08.2026 (Nebenbefund aus T2-2): las bis dahin blind
    z[0..3]. Bei einem leeren oder zu kurzen Code stand das JENSEITS des
    Stringendes - und `jassert` greift nur im Debug, also ausgerechnet nicht im
    Release-Lauf des Kanons. Gelesen wird jetzt nur, was da ist; DASS genau
    vier Zeichen dastehen, sagt der Aufrufer laut (siehe "Viercode ist vier
    Zeichen lang" in main). Ein stiller Puffer-Ueberlauf im Riegel selbst waere
    die schlechteste Stelle fuer undefiniertes Verhalten, die dieser Test
    haben kann. */
juce::uint32 codeAlsZahl (const juce::String& vierZeichen)
{
    const auto* z = vierZeichen.toRawUTF8();
    const auto vorhanden = vierZeichen.getNumBytesAsUTF8();
    juce::uint32 wert = 0;
    for (size_t i = 0; i < 4; ++i)
        wert = (wert << 8) | (juce::uint32) (unsigned char) (i < vorhanden ? z[i] : 0);
    return wert;
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
    // S9-Nacharbeit (T2-2): laut, statt still ueber das Stringende zu lesen.
    // codeAlsZahl() liest jetzt nur, was da ist - DASS vier Zeichen dastehen,
    // muss trotzdem jemand sagen, sonst waere eine gekuerzte Identitaet nur
    // eine andere Zahl statt ein Fehler. Denselben Riegel hat der Configure
    // seit heute auch (cmake/NakamaIdentitaet.cmake); zwei Stellen, weil der
    // Test auch ohne Bau laufen koennen soll.
    pruefe (herstellerCode.length() == 4,
            "Viercode ist vier Zeichen lang: hersteller.code",
            juce::String (herstellerCode.length()) + " Zeichen");

    const auto ziele = manifest["ziele"];
    // S9b/SONDE-007c (28.08.2026): hier stand "genau drei". Diese Zahl war nie
    // die Aussage - sie war die Abkuerzung dafuer, dass jedes Ziel des
    // Manifests hier eine Zeile hat. Seit der Stilllegung von Nakama Suna
    // sind es drei KENNUNGEN und zwei gebaute Ziele, und ein Test, der die
    // alte Zahl festschreibt, misst eine Wunschvorstellung statt den
    // gebauten Stand. Gemessen wird ab hier die Beziehung: aktive Kennungen
    // (ohne `stillgelegt`) zu gebauten Zielen, und stillgelegte zu
    // NICHT-gebauten.
    pruefe (ziele.isArray() && ziele.size() >= 1, "Manifest kennt mindestens ein Ziel",
            juce::String (ziele.isArray() ? ziele.size() : -1));

    // Nacharbeit Runde 1 (29.08.2026): gezaehlt wird die ANWESENHEIT der
    // Marke, nicht ihr Typ - dieselbe Frage wie im Identitaetsleser
    // (cmake/NakamaIdentitaet.cmake), im Installer und in A17.
    int aktiveZiele = 0, stillgelegteZiele = 0, unlesbareMarken = 0;
    for (int i = 0; i < ziele.size(); ++i)
    {
        if (! hatStilllegungsmarke (ziele[i]))
        {
            aktiveZiele += 1;
            continue;
        }
        stillgelegteZiele += 1;
        if (! ziele[i]["stillgelegt"].isObject())
            unlesbareMarken += 1;
    }
    pruefe (aktiveZiele >= 1, "mindestens ein Ziel ist nicht stillgelegt",
            juce::String (aktiveZiele) + " aktiv, " + juce::String (stillgelegteZiele) + " stillgelegt");
    // Der kaputte Inhalt ist ein HARTER Fehler, kein Ruecksprung nach "aktiv":
    // eine Marke, die niemand lesen kann, sperrt trotzdem.
    pruefe (unlesbareMarken == 0,
            "jede vorhandene Stilllegungsmarke ist ein lesbares Objekt",
            juce::String (unlesbareMarken) + " unlesbar");

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
    // Die Zuordnung Manifest-ID -> CMake-Zielname steht hier von Hand. Sie ist
    // die einzige Angabe, die NICHT aus dem Manifest kommen kann (dort steht
    // der Bundlename, nicht der Zielname) - und absichtlich handgeschrieben:
    // ein viertes Ziel im Manifest ohne Zeile hier bringt den Test zum
    // Sprechen, statt still ungemessen zu bleiben. Sie traegt beide Haelften
    // des Beweises: den Quellfrost unten (welcher Bauskriptblock gehoert zu
    // welcher ID) und die Messung am gebauten moduleinfo.json weiter unten.
    struct GebautesZiel { const char* id; const char* cmakeZiel; };
    const GebautesZiel gebaute[] = {
        { "main",          "EqCopilot"     },
        { "active-probe",  "NakamaProbeeq" },
    };

    // S9b/SONDE-007c: dieselbe handgeschriebene Zuordnung, nur fuer die
    // Gegenrichtung. Ein stillgelegtes Ziel behaelt seine Kennung im
    // Manifest (NkPr bleibt gesperrt) - gemessen wird deshalb, dass sein
    // CMake-Ziel WIRKLICH VERSCHWUNDEN ist. Ohne diese Tabelle waere die
    // Stilllegung nur eine geloeschte Zeile, also eine Meinung.
    const GebautesZiel stillgelegte[] = {
        { "passive-probe", "NakamaSuna" },
    };

    const int gebauteZeilen     = (int) (sizeof (gebaute) / sizeof (gebaute[0]));
    const int stillgelegteZeilen = (int) (sizeof (stillgelegte) / sizeof (stillgelegte[0]));

    pruefe (gebauteZeilen == aktiveZiele,
            "jedes AKTIVE Ziel im Manifest hat hier eine Zeile",
            juce::String (gebauteZeilen) + " vs " + juce::String (aktiveZiele));
    pruefe (stillgelegteZeilen == stillgelegteZiele,
            "jedes STILLGELEGTE Ziel im Manifest hat hier eine Zeile",
            juce::String (stillgelegteZeilen) + " vs " + juce::String (stillgelegteZiele));

    // Und die Zeilen zeigen auf das, was sie behaupten: kein aktives Ziel
    // traegt `stillgelegt`, jedes stillgelegte traegt es.
    for (const auto& g : gebaute)
    {
        const auto e = zielMitId (ziele, juce::String (g.id));
        pruefe (e.isObject() && ! hatStilllegungsmarke (e),
                juce::String (g.id) + ": steht als AKTIVES Ziel im Manifest");
    }
    for (const auto& s : stillgelegte)
    {
        const auto e = zielMitId (ziele, juce::String (s.id));
        pruefe (e.isObject(), juce::String (s.id) + ": die Kennung steht weiter im Manifest");
        pruefe (hatStilllegungsmarke (e) && e["stillgelegt"].isObject(),
                juce::String (s.id) + ": und ist dort als stillgelegt markiert");
        // Die Kennung bleibt VOLLSTAENDIG - genau das unterscheidet eine
        // Stilllegung von einem Loeschen. Waere sie halb weg, koennte ein
        // spaeteres Ziel ihre Class-ID erben, ohne dass jemand es merkt.
        for (const auto* feld : { "plugin_code", "bundle", "produktname",
                                  "component_cid", "controller_cid" })
            pruefe (e[feld].toString().isNotEmpty(),
                    juce::String (s.id) + ": " + feld + " ist nicht geloescht",
                    e[feld].toString());
        pruefe (e["stillgelegt"]["am"].toString().isNotEmpty()
                    && e["stillgelegt"]["entscheid"].toString().isNotEmpty(),
                juce::String (s.id) + ": die Stilllegung nennt Datum und Entscheid",
                e["stillgelegt"]["am"].toString() + " / " + e["stillgelegt"]["entscheid"].toString());
    }

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

        // ── S9-Nacharbeit 23.08.2026, T2-Befund T2-3 ──────────────────────
        // Bis hierher deckte der Quellfrost EIN Ziel von dreien. Der Kopf des
        // Bauskripts begruendet NAK-52 gerade damit, dass "drei
        // Bauskriptbloecke mit je vier Identitaetszeilen ... vier Stellen
        // [sind], an denen zwei Wahrheiten auseinanderlaufen koennen" - der
        // Riegel dagegen sah nur den ersten Block. Der T2-Pruefer hat das
        // vorgefuehrt: dasselbe eingeschmuggelte Literal faellt im Main-Block
        // auf und bleibt in nakama_sonde_ziel() unsichtbar.
        //
        // EIN zusaetzlicher Blockfrost genuegt, weil sich beide Sondenziele
        // EINE Funktion teilen. Das ist kein Sparen, sondern Teil der
        // Aussage: dass es nur einen Block gibt, misst die Aufrufpruefung
        // unten mit - jedes Sondenziel muss ueber genau diese Funktion
        // entstehen. Ein kuenftiges viertes Ziel mit eigenem juce_add_plugin
        // haette hier keinen Frost und faellt an der Zeilenpruefung.
        const auto sondeBlock    = cmakeBlock (cmakeText, "juce_add_plugin(${ziel}\n");
        const auto sondeDefBlock = cmakeBlock (cmakeText, "target_compile_definitions(${ziel} PUBLIC");

        pruefe (sondeBlock.isNotEmpty(),
                "CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden");
        pruefe (sondeDefBlock.isNotEmpty(),
                "CMake: Defineblock der Sondenfunktion gefunden");

        // (1) Auch die Sondenfunktion liest das Manifest ueberhaupt.
        pruefe (cmakeText.contains ("nakama_identitaet_lesen(${identitaet} SONDE)"),
                "CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest");

        // (2) Die vier Identitaetszeilen kommen von dort.
        pruefe (sondeBlock.contains ("PLUGIN_CODE ${SONDE_PLUGINCODE}"),
                "CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest");
        pruefe (sondeBlock.contains ("PLUGIN_MANUFACTURER_CODE ${SONDE_HERSTELLERCODE}"),
                "CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest");
        pruefe (sondeBlock.contains ("PRODUCT_NAME \"${SONDE_PRODUKTNAME}\""),
                "CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest");
        pruefe (sondeBlock.contains ("COMPANY_NAME \"${SONDE_HERSTELLER}\""),
                "CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest");

        // (3) Keine zweite Wahrheit im Block - fuer BEIDE Sondenziele, und
        //     jedes Ziel entsteht nachweislich ueber diese eine Funktion.
        pruefe (! sondeBlock.contains (herstellerCode),
                "CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock",
                herstellerCode);

        for (const auto& g : gebaute)
        {
            const juce::String kennung (g.id);
            if (kennung == "main")
                continue;                      // hat seinen eigenen Block oben

            const auto eintrag = zielMitId (ziele, kennung);
            if (! eintrag.isObject())
            {
                pruefe (false, "Manifest kennt Sondenziel '" + kennung + "'");
                continue;
            }

            const auto code        = eintrag["plugin_code"].toString();
            const auto produktname = eintrag["produktname"].toString();

            pruefe (! sondeBlock.contains (code),
                    kennung + ": der Viercode steht nicht literal im Sonden-Zielblock", code);
            pruefe (! sondeBlock.contains ("\"" + produktname + "\""),
                    kennung + ": der Produktname steht nicht literal im Sonden-Zielblock",
                    produktname);

            // Der Aufruf steht auf EINER Zeile; dort ist die Zeile die
            // Einheit. Gemessen wird, dass genau dieses CMake-Ziel mit genau
            // dieser Manifest-ID gebaut wird - sonst traegt ein Bundle die
            // Identitaet des anderen, und das faellt sonst erst am Artefakt
            // auf (also nur, wenn jemand baut).
            const auto aufruf = zeileMit (cmakeText,
                                          juce::String ("nakama_sonde_ziel(") + g.cmakeZiel);
            pruefe (aufruf.isNotEmpty(),
                    kennung + ": " + g.cmakeZiel + " entsteht ueber nakama_sonde_ziel()");
            pruefe (aufruf.contains (kennung),
                    kennung + ": der Aufruf uebergibt genau diese Manifest-ID",
                    aufruf.trim());
        }

        pruefe (sondeDefBlock.contains ("JUCE_VST3_CAN_REPLACE_VST2=0"),
                "CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock");

        // ── S9b/SONDE-007c: die Stilllegung am BAUSKRIPT gemessen ─────────
        // Ein stillgelegtes Ziel darf im Bauskript nicht mehr vorkommen -
        // weder ueber die gemeinsame Sondenfunktion noch ueber einen eigenen
        // juce_add_plugin-Block, mit dem sich jemand an ihr vorbeibauen
        // koennte. Die Kennung bleibt im Manifest; das Ziel darf nicht
        // zurueckkommen, ohne dass es hier auffaellt.
        //
        // Zweiter Riegel derselben Sache liegt in cmake/NakamaIdentitaet.cmake
        // (der Leser bricht bei `stillgelegt` ab). Zwei Stellen, weil dieser
        // Test auch OHNE Bau laufen koennen soll - der Leser laeuft nur beim
        // Configure.
        for (const auto& s : stillgelegte)
        {
            const juce::String ziel (s.cmakeZiel);
            pruefe (zeileMit (cmakeText, "nakama_sonde_ziel(" + ziel).isEmpty(),
                    ziel + ": stillgelegt - kein Aufruf von nakama_sonde_ziel() mehr",
                    zeileMit (cmakeText, "nakama_sonde_ziel(" + ziel).trim());
            pruefe (zeileMit (cmakeText, "juce_add_plugin(" + ziel).isEmpty(),
                    ziel + ": stillgelegt - auch kein eigener juce_add_plugin-Block",
                    zeileMit (cmakeText, "juce_add_plugin(" + ziel).trim());
            // Und der Grund steht da, wo er hingehoert: an der Stelle, an der
            // der Aufruf stand. Eine spurlos geloeschte Zeile ist von einem
            // Versehen nicht zu unterscheiden.
            //
            // Gesucht wird eine Zeile, die BEIDES traegt - das Wort und den
            // Zielnamen. Zwei getrennte contains() ueber die ganze Datei
            // waeren schwaecher: bei einem zweiten stillgelegten Ziel liesse
            // sich der Nachweis mit EINEM Kommentar fuer beide erschleichen.
            bool erklaert = false;
            for (const auto& zeile : juce::StringArray::fromLines (cmakeText))
                if (zeile.contains ("STILLGELEGT") && zeile.contains (ziel))
                    erklaert = true;
            pruefe (erklaert,
                    ziel + ": das Bauskript erklaert die Stilllegung an Ort und Stelle");
        }
    }

    // -- Die gebauten Bundles gegen das Manifest --------------------------
    for (const auto& g : gebaute)
    {
        juce::var eintrag;
        for (int i = 0; i < ziele.size(); ++i)
            if (ziele[i]["id"].toString() == g.id)
                eintrag = ziele[i];

        const juce::String kennung (g.id);
        if (! eintrag.isObject())
        {
            pruefe (false, "Manifest kennt Ziel '" + kennung + "'");
            continue;
        }

        const auto bundle = eintrag["bundle"].toString();
        const auto moduleinfo = finde (juce::String ("eq-copilot/build/plugin/") + g.cmakeZiel
                                       + "_artefacts/Release/VST3/" + bundle
                                       + "/Contents/Resources/moduleinfo.json");
        pruefe (moduleinfo.existsAsFile(), kennung + ": moduleinfo.json des gebauten Bundles gefunden",
                moduleinfo.getFullPathName());

        if (moduleinfo.existsAsFile() && cmakeDatei.existsAsFile())
        {
            // Frischeriegel: ein moduleinfo, das aelter ist als die Bauvorschrift,
            // beweist nichts ueber den heutigen Stand.
            const auto artefaktZeit = moduleinfo.getLastModificationTime();
            const auto quellZeit    = cmakeDatei.getLastModificationTime();
            pruefe (artefaktZeit >= quellZeit,
                    kennung + ": moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt",
                    artefaktZeit.toString (true, true) + " vs " + quellZeit.toString (true, true));
        }

        if (! moduleinfo.existsAsFile())
            continue;

        const auto roh = moduleinfo.loadFileAsString();
        const auto gelesen = juce::JSON::parse (ohneNachlaufkommas (roh));
        pruefe (gelesen.isObject(), kennung + ": moduleinfo.json ist nach dem Kommaputz parsebar");

        const auto componentCid  = eintrag["component_cid"].toString();
        const auto controllerCid = eintrag["controller_cid"].toString();
        const auto produktname   = eintrag["produktname"].toString();

        pruefe (gelesen["Name"].toString() == produktname,
                kennung + ": Produktname wie im Manifest", gelesen["Name"].toString());
        pruefe (gelesen["Factory Info"]["Vendor"].toString()
                    == manifest["hersteller"]["name"].toString(),
                kennung + ": Vendor wie im Manifest",
                gelesen["Factory Info"]["Vendor"].toString());

        const auto klassen = gelesen["Classes"];
        pruefe (klassen.isArray() && klassen.size() == 2,
                kennung + ": genau zwei Klassen (Component + Controller)",
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
                kennung + ": die Audio-Module-Klasse traegt die Component-CID", cidAudio);
        pruefe (cidController == controllerCid,
                kennung + ": die Controller-Klasse traegt die Controller-CID", cidController);

        const auto kategorien = eintrag["kategorien"];
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
        pruefe (alleKategorien, kennung + ": Unterkategorien wie im Manifest");

        // Gegenprobe und zugleich der Kern von §53.4: KEINE FREMDE CID darf in
        // diesem Bundle stehen. Genau das waere passiert, wenn der gemeinsame
        // Kern die Identitaetskonstanten EINES Ziels traegt und sie an alle
        // drei weiterreicht - S8s Riegel misst die Baubeschreibung und die
        // Lib, DIESE Zeile misst das ausgelieferte Artefakt.
        bool fremdeGefunden = false;
        juce::String fremde;
        for (int i = 0; i < ziele.size(); ++i)
        {
            if (ziele[i]["id"].toString() == kennung)
                continue;
            for (const auto* feld : { "component_cid", "controller_cid" })
                if (roh.contains (ziele[i][feld].toString()))
                {
                    fremdeGefunden = true;
                    fremde << ziele[i]["id"].toString() << "." << feld << " ";
                }
        }
        pruefe (! fremdeGefunden,
                kennung + ": keine fremde Ziel-CID im Bundle", fremde.trim());
    }

    // -- Ableitung nachrechnen statt glauben ------------------------------
    for (int i = 0; i < ziele.size(); ++i)
    {
        const auto ziel = ziele[i];
        const auto id   = ziel["id"].toString();
        const auto code = ziel["plugin_code"].toString();

        pruefe (code.length() == 4, "Viercode ist vier Zeichen lang: " + id + ".plugin_code",
                juce::String (code.length()) + " Zeichen");

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
