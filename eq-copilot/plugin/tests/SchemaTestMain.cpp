/*  EqCopSchemaTest — C++-Bein des v3-Vertrags (SONDE-005a).

    Faehrt denselben Fixture-Korpus wie broker/tests/contract_cross_language.rs
    gegen dieselbe Regelmenge und vergleicht mit demselben MANIFEST.json:
    Urteil UND vollstaendige Verletzungsmenge.

    Der Vergleich laeuft ueber das Manifest, nicht gegen eine Ausgabe der
    Rust-Seite. Stimmen beide Engines mit dem Manifest ueberein, stimmen sie
    transitiv miteinander ueberein - und das Manifest ist von Hand
    geschrieben, also nicht die Ausgabe einer der beiden.

    Zusaetzlich: das Bandgitter, der Quantisierungsvertrag und - weil ein
    gruener Test nichts wert ist, solange nicht gezeigt wurde, dass er
    ueberhaupt fallen KANN - eine Reihe Riegelproben.
*/

#include "../vertrag/NakamaVertrag.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace
{
int bestanden = 0;
int fehler = 0;

void pruefe (bool ok, const juce::String& was, const juce::String& zusatz = {})
{
    std::cout << (ok ? "[ok]   " : "[ROT]  ") << was.toRawUTF8();
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
    for (int i = 0; i < 10 && ordner.exists(); ++i)
    {
        auto kandidat = ordner.getChildFile (relativ);
        if (kandidat.existsAsFile())
            return kandidat;
        ordner = ordner.getParentDirectory();
    }
    return ausCwd;
}

juce::var lies (const juce::String& relativ, bool& ok)
{
    const auto datei = finde (relativ);
    if (! datei.existsAsFile())
    {
        std::cout << "[ROT]  Datei fehlt: " << datei.getFullPathName().toRawUTF8() << std::endl;
        ++fehler;
        ok = false;
        return {};
    }
    juce::var wert;
    const auto ergebnis = juce::JSON::parse (datei.loadFileAsString(), wert);
    if (ergebnis.failed())
    {
        std::cout << "[ROT]  JSON kaputt in " << relativ.toRawUTF8()
                  << ": " << ergebnis.getErrorMessage().toRawUTF8() << std::endl;
        ++fehler;
        ok = false;
        return {};
    }
    ok = true;
    return wert;
}

double ausHex64 (const juce::String& h)
{
    const auto bits = static_cast<uint64_t> (h.getHexValue64());
    double d {};
    std::memcpy (&d, &bits, sizeof d);
    return d;
}

uint64_t bitsVon (double d)
{
    uint64_t b {};
    std::memcpy (&b, &d, sizeof b);
    return b;
}

juce::String beschreibe (const nakama::vertrag::Verletzung& v)
{
    return "{" + v.instanz + " | " + v.schema + " | " + v.schluessel + "}";
}

// ------------------------------------------------------------------ Korpus

void fahreKorpus (const nakama::vertrag::Schema& schema)
{
    bool ok = false;
    const auto manifest = lies ("eq-copilot/fixtures/v3/MANIFEST.json", ok);
    if (! ok)
        return;

    auto* liste = manifest.getProperty ("fixtures", {}).getArray();
    if (liste == nullptr)
    {
        pruefe (false, "MANIFEST traegt eine Fixtureliste");
        return;
    }

    int geprueft = 0, abweichungen = 0;
    for (const auto& eintrag : *liste)
    {
        const auto name = eintrag.getProperty ("datei", {}).toString();

        // Skalar-Wurzel: JUCEs JSON-Leser folgt RFC 4627 und verlangt { oder [
        // am Anfang, waehrend serde_json und Python RFC 8259 folgen und jeden
        // Wert als Dokument annehmen. Beide Seiten LEHNEN AB - C++ schon im
        // Parser, Rust erst am Schema. Hier wird deshalb genau das geprueft:
        // dass der Parser es zurueckweist. Das Fixture bleibt im Korpus, damit
        // die Abweichung sichtbar bleibt statt weggeraeumt zu werden.
        if (static_cast<bool> (eintrag.getProperty ("wurzel_skalar", false)))
        {
            juce::var weg;
            const auto ergebnis = juce::JSON::parse (
                finde ("eq-copilot/fixtures/v3/" + name).loadFileAsString(), weg);
            pruefe (ergebnis.failed(),
                    "Skalar-Wurzel wird schon im Parser abgelehnt: " + name);
            ++geprueft;
            continue;
        }

        // Der Textriegel laeuft VOR dem Parser, und zwar ueber JEDES Fixture.
        // Die mit `textriegel_lehnt_ab` markierten muessen an ihm fallen; alle
        // uebrigen muessen ihn passieren. Ohne die zweite Haelfte waere der
        // Riegel eine Behauptung, die nur an elf Dateien geprueft wird.
        const auto rohtext = finde ("eq-copilot/fixtures/v3/" + name).loadFileAsString();
        juce::String riegelfehler;
        const bool sauber = nakama::vertrag::textriegel (rohtext, riegelfehler);
        if (static_cast<bool> (eintrag.getProperty ("textriegel_lehnt_ab", false)))
        {
            pruefe (! sauber, "Textriegel lehnt ab: " + name, riegelfehler);
            ++geprueft;
            continue;
        }
        if (! sauber)
        {
            std::cout << "[ROT]  " << name.toRawUTF8()
                      << ": Textriegel lehnt ein Fixture ab, das er passieren lassen muss: "
                      << riegelfehler.toRawUTF8() << std::endl;
            ++abweichungen;
            continue;
        }

        bool gelesen = false;
        const auto daten = lies ("eq-copilot/fixtures/v3/" + name, gelesen);
        if (! gelesen)
            { ++abweichungen; continue; }

        const auto ist = schema.pruefe (daten);
        const bool sollGueltig = eintrag.getProperty ("urteil", {}).toString() == "gueltig";

        if (ist.isEmpty() != sollGueltig)
        {
            juce::String erste;
            if (! ist.isEmpty())
                erste = beschreibe (ist.getReference (0));
            std::cout << "[ROT]  " << name.toRawUTF8() << ": Urteil "
                      << (sollGueltig ? "gueltig" : "ungueltig") << " erwartet, Engine sagt "
                      << (ist.isEmpty() ? "gueltig" : "ungueltig")
                      << " " << erste.toRawUTF8() << std::endl;
            ++abweichungen;
        }
        else
        {
            auto* sollListe = eintrag.getProperty ("verletzungen", {}).getArray();
            juce::Array<nakama::vertrag::Verletzung> soll;
            if (sollListe != nullptr)
                for (const auto& v : *sollListe)
                    soll.add ({ v.getProperty ("instanz", {}).toString(),
                                v.getProperty ("schema", {}).toString(),
                                v.getProperty ("schluessel", {}).toString() });

            bool gleich = soll.size() == ist.size();
            for (int i = 0; gleich && i < soll.size(); ++i)
                gleich = soll.getReference (i) == ist.getReference (i);

            if (! gleich)
            {
                std::cout << "[ROT]  " << name.toRawUTF8() << ": Verletzungsmenge weicht ab"
                          << std::endl;
                for (const auto& v : soll) std::cout << "         soll " << beschreibe (v).toRawUTF8() << std::endl;
                for (const auto& v : ist)  std::cout << "         ist  " << beschreibe (v).toRawUTF8() << std::endl;
                ++abweichungen;
            }
        }
        ++geprueft;
    }

    pruefe (abweichungen == 0,
            "Korpus klassifiziert wie das Manifest (" + juce::String (geprueft) + " Fixtures)",
            abweichungen == 0 ? juce::String() : juce::String (abweichungen) + " Abweichungen");
    pruefe (geprueft >= 100, "Korpus hat Substanz", juce::String (geprueft) + " Fixtures");

    const int gueltig   = static_cast<int> (manifest.getProperty ("anzahl_gueltig", {}));
    const int ungueltig = static_cast<int> (manifest.getProperty ("anzahl_ungueltig", {}));
    pruefe (gueltig + ungueltig == geprueft, "Manifestzahlen passen zur Fixtureliste");
}

// ------------------------------------------------------------------ Bandgitter

void fahreBandgitter()
{
    bool a = false, b = false;
    const auto fein = lies ("eq-copilot/schemas/v3/bandgitter/nakama_1_24_oct_30_18k_v1.json", a);
    const auto grob = lies ("eq-copilot/schemas/v3/bandgitter/nakama_log64_v1.json", b);
    if (! a || ! b)
        return;

    auto zahlen = [] (const juce::var& block)
    {
        juce::Array<double> out;
        if (auto* liste = block.getProperty ("hex64", {}).getArray())
            for (const auto& h : *liste)
                out.add (ausHex64 (h.toString()));
        return out;
    };

    const auto kanten = zahlen (fein.getProperty ("kanten_hz", {}));
    const auto mitten = zahlen (fein.getProperty ("mitten_hz", {}));

    pruefe (static_cast<int> (fein.getProperty ("band_anzahl", {})) == 221,
            "feines Gitter hat 221 Baender");
    pruefe (kanten.size() == 222 && mitten.size() == 221,
            "222 Kanten, 221 Mitten",
            juce::String (kanten.size()) + "/" + juce::String (mitten.size()));

    bool monoton = true, eingebettet = true, endlich = true, imBand = true;
    for (int i = 0; i < kanten.size() - 1; ++i)
        monoton = monoton && kanten[i] < kanten[i + 1];
    for (int i = 0; i < mitten.size(); ++i)
    {
        eingebettet = eingebettet && kanten[i] < mitten[i] && mitten[i] < kanten[i + 1];
        endlich = endlich && std::isfinite (mitten[i]);
        imBand = imBand && mitten[i] >= 30.0 && mitten[i] <= 18000.0;
    }
    pruefe (monoton, "Kanten streng monoton");
    pruefe (eingebettet, "jede Mitte liegt echt zwischen ihren Kanten");
    pruefe (endlich, "keine Mitte ist NaN oder unendlich");
    pruefe (imBand, "alle Mitten liegen in 30 Hz .. 18 kHz");

    const auto grobkanten = zahlen (grob.getProperty ("kanten_hz", {}));
    auto* gruppen = grob.getProperty ("gruppen", {}).getArray();
    pruefe (gruppen != nullptr && gruppen->size() == 64, "grobes Gitter hat 64 Gruppen");
    pruefe (grobkanten.size() == 65, "65 grobe Kanten");
    if (gruppen == nullptr)
        return;

    int erwartetVon = 0, summe = 0;
    bool partition = true, bitgleich = true, nichtLeer = true;
    for (int j = 0; j < gruppen->size(); ++j)
    {
        const auto& g = (*gruppen)[j];
        const int von = static_cast<int> (g.getProperty ("fein_von", {}));
        const int bis = static_cast<int> (g.getProperty ("fein_bis_exkl", {}));
        partition = partition && von == erwartetVon;
        nichtLeer = nichtLeer && bis > von;
        if (j < grobkanten.size() && von < kanten.size())
            bitgleich = bitgleich && bitsVon (grobkanten[j]) == bitsVon (kanten[von]);
        summe += bis - von;
        erwartetVon = bis;
    }
    pruefe (partition && erwartetVon == 221, "Gruppen sind lueckenlos und ueberlappungsfrei");
    pruefe (nichtLeer, "keine leere Gruppe");
    pruefe (summe == 221, "Gruppen decken genau 221 feine Baender", juce::String (summe));
    pruefe (bitgleich && bitsVon (grobkanten[64]) == bitsVon (kanten[221]),
            "grobe Kanten sind BITGLEICHE Kopien feiner Kanten");
}

// ------------------------------------------------------------------ Quantisierung

/*  Vertrag aus quantisierung-v1.json. std::round ist bereits "halbe Werte von
    null weg" - genau die dort festgelegte Regel. */
void quantisiereI16 (double db, double skalierung, int& wert, bool& gueltig, bool& saturiert)
{
    if (! std::isfinite (db)) { wert = 0; gueltig = false; saturiert = false; return; }
    const double roh = std::round (db * skalierung);
    gueltig = true;
    if (roh > 32767.0)  { wert =  32767; saturiert = true;  return; }
    if (roh < -32768.0) { wert = -32768; saturiert = true;  return; }
    wert = static_cast<int> (roh);
    saturiert = false;
}

/*  Der Ueberlauf wird VOR dem Cast geprueft: eine double-nach-float-Umwandlung
    ausserhalb des darstellbaren Bereichs ist in C++ undefiniert, waehrend Rust
    dort +-inf liefert und Python eine Ausnahme wirft. Drei Meldewege, eine
    Grenze - der Vertrag legt fest, dass alle drei sie ABFANGEN. */
void quantisiereF32 (double db, uint32_t& bits, bool& gueltig, bool& saturiert)
{
    constexpr double kUeberlauf = 3.4028235677973366e38;   // Rundungsgrenze zu inf
    constexpr float  kMaxF32    = 3.4028234663852886e38f;

    if (! std::isfinite (db)) { float n = 0.0f; std::memcpy (&bits, &n, 4); gueltig = false; saturiert = false; return; }
    gueltig = true;
    if (std::abs (db) >= kUeberlauf)
    {
        const float g = db < 0 ? -kMaxF32 : kMaxF32;
        std::memcpy (&bits, &g, 4);
        saturiert = true;
        return;
    }
    const float eng = static_cast<float> (db);
    std::memcpy (&bits, &eng, 4);
    saturiert = false;
}

void fahreQuantisierung()
{
    bool ok = false;
    const auto q = lies ("eq-copilot/schemas/v3/quantisierung-v1.json", ok);
    if (! ok)
        return;

    auto* kodierungen = q.getProperty ("kodierungen", {}).getDynamicObject();
    if (kodierungen == nullptr)
        { pruefe (false, "Quantisierungsvertrag traegt Kodierungen"); return; }

    int geprueft = 0, abweichungen = 0;
    for (const auto& eintrag : kodierungen->getProperties())
    {
        const auto name = eintrag.name.toString();
        const auto k = eintrag.value;
        const double skalierung = static_cast<double> (k.getProperty ("skalierung", {}));
        const bool istI16 = k.getProperty ("traeger", {}).toString() == "i16";

        auto* vektoren = k.getProperty ("vektoren", {}).getArray();
        if (vektoren == nullptr)
            continue;

        for (const auto& v : *vektoren)
        {
            const double ein = ausHex64 (v.getProperty ("ein_hex64", {}).toString());
            const bool sollGueltig = static_cast<bool> (v.getProperty ("gueltig", {}));
            const bool sollSat     = static_cast<bool> (v.getProperty ("saturiert", {}));
            bool gueltig = false, sat = false;
            bool stimmt = false;

            if (istI16)
            {
                int wert = 0;
                quantisiereI16 (ein, skalierung, wert, gueltig, sat);
                stimmt = wert == static_cast<int> (v.getProperty ("wert", {}));
            }
            else
            {
                uint32_t bits = 0;
                quantisiereF32 (ein, bits, gueltig, sat);
                stimmt = juce::String::toHexString (static_cast<int> (bits))
                            .paddedLeft ('0', 8).toUpperCase()
                         == v.getProperty ("wert_hex32", {}).toString();
            }

            if (! (stimmt && gueltig == sollGueltig && sat == sollSat))
            {
                std::cout << "[ROT]  " << name.toRawUTF8() << " "
                          << v.getProperty ("ein_dezimal", {}).toString().toRawUTF8()
                          << ": Wert/Flags weichen ab" << std::endl;
                ++abweichungen;
            }
            ++geprueft;
        }
    }

    pruefe (abweichungen == 0,
            "Quantisierung stimmt mit den Testvektoren (" + juce::String (geprueft) + ")",
            abweichungen == 0 ? juce::String() : juce::String (abweichungen) + " Abweichungen");
    pruefe (geprueft >= 60, "genug Vektoren", juce::String (geprueft));
}

// ------------------------------------------------------------------ Riegelproben

/** Ein gruener Test ist nichts wert, solange nicht gezeigt wurde, dass er
    ueberhaupt fallen kann. Diese Proben bringen jeden Riegel einmal zum
    Fallen - im Test selbst, damit es niemand von Hand nachstellen muss. */
/*  Der Textriegel, Kante fuer Kante.

    Dieselbe Tabelle steht in broker/src/vertrag.rs und in
    tools/eq-copilot/pruefe_v3_vertrag.py. Laufen die drei auseinander, faellt
    genau hier eine von ihnen - und nicht erst, wenn ein Produktivframe eine
    Zahl traegt, die auf zwei Seiten verschieden ankommt.
*/
void fahreTextriegelproben()
{
    struct Fall { const char* text; bool wirdAbgelehnt; };
    static const Fall faelle[] = {
        { R"({"w": 9007199254740991})",        false },
        { R"({"w": 9007199254740992})",        true  },
        { R"({"w": -9007199254740991})",       false },
        { R"({"w": -9007199254740992})",       true  },
        { R"({"w": 18446744073709552016})",    true  },
        { R"({"w": 10000000000000000000})",    true  },
        { R"({"w": 091})",                     true  },
        { R"({"w": -091})",                    true  },
        { R"({"w": 0})",                       false },
        { R"({"w": -0})",                      false },
        { R"({"w": 0.5})",                     false },
        { R"({"w": 1e400})",                   true  },
        { R"({"w": -1e400})",                  true  },
        { R"({"w": 1e-400})",                  false },
        { R"({"w": 1e300})",                   false },
        { R"({"w": 1.5e3})",                   false },
        { R"({"w": "091 nur Text"})",          false },
        { R"({"w": "1e400"})",                 false },
        { R"({"w": "a\u0000b"})",              true  },
        { R"({"w": "😀"})",          false },
        { R"({"w": "\ud83d"})",                true  },
        { R"({"w": "\ude00"})",                true  },
        { R"({"w": "\ud83dx"})",               true  },
        { R"({"": 1})",                        true  },
        { R"({"a": {"": 2}})",                 true  },
        { R"({"w": ""})",                      false },
        { R"({"w" : 1})",                      false },
        { R"({"w": "er sagte \"hallo\""})",    false },
        { R"({"w": "backslash am Ende \\"})",  false },
        { R"({"w": 512, "x": [1,2,3]})",       false },
        { R"({"w": "Doppelpunkt : im Text"})", false },
    };

    int rot = 0;
    for (const auto& f : faelle)
    {
        juce::String fehler;
        const bool sauber = nakama::vertrag::textriegel (juce::String::fromUTF8 (f.text), fehler);
        if (sauber == f.wirdAbgelehnt)
        {
            ++rot;
            pruefe (false, juce::String ("Textriegel: ") + f.text, fehler);
        }
    }
    pruefe (rot == 0, "Textriegel deckt jede gemessene Kante",
            juce::String ((int) (sizeof (faelle) / sizeof (faelle[0]))) + " Faelle");
}

void fahreRiegelproben()
{
    using nakama::vertrag::Schema;

    auto ausText = [] (const char* json)
    {
        juce::var v;
        juce::JSON::parse (juce::String::fromUTF8 (json), v);
        return v;
    };

    Schema s; juce::String f;

    pruefe (! Schema::laden (ausText (R"({"type":"object","multipleOf":2})"), s, f)
            && f.contains ("multipleOf"),
            "unbekanntes Schluesselwort bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"oneOf":[]})"), s, f)
            && f.contains ("x-nakama-discriminator"),
            "oneOf ohne Discriminator bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":"object","additionalProperties":true})"), s, f)
            && f.contains ("maxProperties"),
            "additives Objekt ohne maxProperties bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":"string","pattern":"^S-1-.*$"})"), s, f)
            && f.contains ("Muster"),
            "unbekanntes Muster bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"$ref":"https://fremd/schema.json"})"), s, f)
            && f.contains ("nicht-lokale"),
            "nicht-lokale Referenz bricht den Ladevorgang", f);

    // --- T2-Runde 1 -------------------------------------------------------
    // Beide Riegel schliessen dieselbe Klasse wie "unbekanntes Schluesselwort",
    // nur eine Ebene tiefer: etwas im Schema, das eine Engine anders liest als
    // die andere, ohne dass jemand es merkt.

    pruefe (! Schema::laden (ausText (R"({"$ref":"#/$defs/gibtsnicht","$defs":{"a":{"type":"object"}}})"), s, f)
            && f.contains ("haengende Referenz"),
            "haengende Referenz bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":"string","maxLength":5.0})"), s, f)
            && f.contains ("Werttyp"),
            "maxLength als Gleitkommazahl bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":5})"), s, f)
            && f.contains ("Werttyp"),
            "type als Zahl bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":"object","required":[5]})"), s, f)
            && f.contains ("Werttyp"),
            "required mit Nicht-String bricht den Ladevorgang", f);

    pruefe (! Schema::laden (ausText (R"({"type":"object","x-nakama-discriminator":7,"oneOf":[{"type":"object"}]})"), s, f)
            && f.contains ("Werttyp"),
            "Discriminator als Zahl bricht den Ladevorgang", f);

    // Ein kleines, vollstaendiges Schema fuer die Verhaltensproben.
    const char* klein = R"({
      "x-nakama-discriminator": "type",
      "oneOf": [{ "$ref": "#/$defs/a" }],
      "$defs": { "a": {
        "type": "object",
        "required": ["type", "n"],
        "additionalProperties": false,
        "properties": {
          "type": { "const": "a" },
          "n": { "type": "integer", "minimum": 0 },
          "s": { "type": "string", "maxLength": 3 }
        }
      }}
    })";
    if (! Schema::laden (ausText (klein), s, f))
        { pruefe (false, "Probeschema laedt", f); return; }

    pruefe (s.gueltig (ausText (R"({"type":"a","n":1.0})")),
            "integer akzeptiert 1.0 (draft 2020-12)");
    pruefe (! s.gueltig (ausText (R"({"type":"a","n":1.5})")),
            "integer lehnt 1.5 ab");

    const auto lawine = s.pruefe (ausText (R"({"type":"a","n":1,"s":12345})"));
    pruefe (lawine.size() == 1 && lawine.getReference (0).schluessel == "type",
            "Typfehler erzeugt keine Lawine",
            juce::String (lawine.size()) + " Verletzung(en)");

    const auto fremd = s.pruefe (ausText (R"({"type":"b"})"));
    pruefe (fremd.size() == 1 && fremd.getReference (0).instanz == "/type"
            && fremd.getReference (0).schluessel == "oneOf",
            "unbekannter Discriminator wird abgelehnt");

    const auto wurzelString = s.pruefe (ausText (R"("a")"));
    pruefe (wurzelString.size() == 1 && wurzelString.getReference (0).instanz.isEmpty(),
            "Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type");

    const auto fehlt = s.pruefe (ausText (R"({"type":"a"})"));
    pruefe (fehlt.size() == 1 && fehlt.getReference (0).instanz.isEmpty()
            && fehlt.getReference (0).schema == "#/$defs/a/required/n",
            "fehlendes Pflichtfeld zeigt auf das Elternobjekt");

    // "aeoeue" sind 3 Codepunkte, aber 6 UTF-8-Bytes. maxLength ist 3.
    pruefe (s.gueltig (ausText ("{\"type\":\"a\",\"n\":0,\"s\":\"\xc3\xa4\xc3\xb6\xc3\xbc\"}")),
            "Laenge zaehlt Codepunkte, nicht Bytes");
    pruefe (! s.gueltig (ausText ("{\"type\":\"a\",\"n\":0,\"s\":\"\xc3\xa4\xc3\xb6\xc3\xbcx\"}")),
            "vier Codepunkte fallen bei maxLength 3");

    const auto mehrere = s.pruefe (ausText (R"({"type":"a","x":1,"y":2})"));
    bool sortiert = true;
    for (int i = 1; i < mehrere.size(); ++i)
        sortiert = sortiert && (mehrere.getReference (i - 1) < mehrere.getReference (i));
    pruefe (mehrere.size() == 3 && sortiert,
            "Verletzungen sind kanonisch sortiert und doppelfrei",
            juce::String (mehrere.size()));
}

} // namespace

int main (int, char*[])
{
    // Kein ScopedJuceInitialiser: dieses Ziel linkt nur juce_core (File,
    // String, JSON, var) - es gibt weder Message-Loop noch GUI zu starten.
    std::cout << "EqCopSchemaTest - v3-Vertrag (SONDE-005a)" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;

    fahreTextriegelproben();
    fahreRiegelproben();

    bool ok = false;
    const auto schemaVar = lies ("eq-copilot/schemas/v3/eq-ipc-v3.schema.json", ok);
    if (ok)
    {
        nakama::vertrag::Schema schema;
        juce::String ladefehler;
        if (nakama::vertrag::Schema::laden (schemaVar, schema, ladefehler))
        {
            pruefe (true, "v3-Schema haelt die Engine-Teilmenge ein");
            fahreKorpus (schema);
        }
        else
        {
            pruefe (false, "v3-Schema haelt die Engine-Teilmenge ein", ladefehler);
        }
    }

    fahreBandgitter();
    fahreQuantisierung();

    std::cout << "-----------------------------------------" << std::endl;
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
