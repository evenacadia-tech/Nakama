// EqCopStateMigrationTest (SONDE-006, Kanon B2): State-Schema 2, fester
// Parameterbestand, reine Schema-1-Migration, RFC-8785-state_hash.
//
// Gate-Text aus Entwurf §65: "Roundtrip, unbekanntes Major, Duplicate und
// Host-Dirty gruen". §66.1 bindet dieses Ziel an "Schema 1→2, Roundtrip,
// Unknown-Major, Host-Dirty". Gemessen wird gegen die handgeschriebenen
// Vertraege (schemas/state/) und den Fixture-Korpus (fixtures/state/) - die
// dritte Partei zwischen C++, Rust und Python.
//
// Das Ziel linkt den ECHTEN Produktcode (plugin/state/, PluginProcessor) -
// keine Testkopie (§66.1).
//
//   EqCopStateMigrationTest.exe                    misst
//   EqCopStateMigrationTest.exe --schreibe-goldens schreibt fixtures/state/schema2/*.bin

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

#include "NakamaKanon.h"
#include "NakamaParameter.h"
#include "NakamaPreset.h"
#include "NakamaState.h"
#include "ControlClient.h"
#include "PluginProcessor.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>

using namespace eqcop;
namespace kanon = nakama::kanon;
namespace param = nakama::parameter;
namespace state = nakama::state;

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

/** Abschnittsurteil: die Kennzeile erscheint NUR gruen, wenn im Abschnitt
    kein Einzelfehler war - der Beweis-Runner greift auf diese Zeile. */
struct Abschnitt
{
    int fehlerVorher;
    explicit Abschnitt() : fehlerVorher (fehler) {}
    void schliesse (const juce::String& kennzeile)
    {
        const bool ok = fehler == fehlerVorher;
        std::cout << (ok ? "  ok      " : "  FEHLER  ") << "== " << kennzeile.toRawUTF8() << std::endl;
        if (ok) ++bestanden; else ++fehler;
    }
};

juce::File finde (const juce::String& relativ)
{
    auto ausCwd = juce::File::getCurrentWorkingDirectory().getChildFile (relativ);
    if (ausCwd.exists())
        return ausCwd;
    auto ordner = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
    for (int i = 0; i < 8 && ordner.exists(); ++i)
    {
        auto kandidat = ordner.getChildFile (relativ);
        if (kandidat.exists())
            return kandidat;
        ordner = ordner.getParentDirectory();
    }
    return ausCwd;
}

juce::var ladeJson (const juce::File& f, bool& ok)
{
    juce::var v;
    const auto r = juce::JSON::parse (f.loadFileAsString(), v);
    ok = r.wasOk();
    return v;
}

juce::MemoryBlock alsBlock (const juce::ValueTree& v)
{
    juce::MemoryBlock b;
    juce::MemoryOutputStream s (b, false);
    v.writeToStream (s);
    s.flush();
    return b;
}

bool gleich (const juce::MemoryBlock& a, const juce::MemoryBlock& b)
{
    return a.getSize() == b.getSize() && std::memcmp (a.getData(), b.getData(), a.getSize()) == 0;
}

const char* ladeErgebnisWort (state::LadeErgebnis ergebnis)
{
    switch (ergebnis)
    {
        case state::LadeErgebnis::geladen:   return "geladen";
        case state::LadeErgebnis::migriert:  return "migriert";
        case state::LadeErgebnis::nurLesen:  return "nurLesen";
        case state::LadeErgebnis::ignoriert: return "ignoriert";
    }
    return "unbekannt";
}

juce::String utf8 (const juce::MemoryBlock& b)
{
    return juce::String::fromUTF8 ((const char*) b.getData(), (int) b.getSize());
}

double ausHex64 (const juce::String& hex)
{
    juce::uint64 bits = 0;
    for (auto c : hex)
    {
        bits <<= 4;
        if (c >= '0' && c <= '9') bits |= (juce::uint64) (c - '0');
        else if (c >= 'a' && c <= 'f') bits |= (juce::uint64) (10 + c - 'a');
        else if (c >= 'A' && c <= 'F') bits |= (juce::uint64) (10 + c - 'A');
    }
    double d;
    std::memcpy (&d, &bits, sizeof (d));
    return d;
}

/** Schema-1-Saat mit FESTEN Werten - dieselbe wie im IdentityTest. */
juce::MemoryBlock saatSchema1 (const juce::String& rolle, const juce::String& label, const juce::String& paarId)
{
    juce::ValueTree v ("EqCopilotState");
    v.setProperty ("schema", 1, nullptr);
    v.setProperty ("sensor_id", "11111111-2222-3333-4444-555555555555", nullptr);
    v.setProperty ("role", rolle, nullptr);
    v.setProperty ("label", label, nullptr);
    v.setProperty ("pair_id", paarId, nullptr);
    return alsBlock (v);
}

struct Rolle { const char* name; const char* label; const char* paar; state::Klasse klasse; state::Messposition pos; };
const Rolle kRollen[] = {
    { "sensor", "Klavier A", "",          state::Klasse::legacy, state::Messposition::insert },
    { "hub",    "Leitstand", "",          state::Klasse::main,   state::Messposition::insert },
    { "pre",    "Chor PRE",  "paar-chor", state::Klasse::legacy, state::Messposition::pre },
    { "post",   "Chor POST", "paar-chor", state::Klasse::legacy, state::Messposition::post }
};

struct DirtyZaehler final : public juce::AudioProcessorListener
{
    int nonParam = 0, gesamt = 0;
    void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) override {}
    void audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails& d) override
    {
        ++gesamt;
        if (d.nonParameterStateChanged) ++nonParam;
    }
};

/** Baut einen gueltigen Schema-2-Baum fuer Experimente. */
juce::ValueTree schema2Baum (const char* klasse, const char* position, bool mitMainProject)
{
    juce::ValueTree v ("NakamaState");
    v.setProperty ("schema", 2, nullptr);
    juce::ValueTree c ("Common");
    c.setProperty ("schema", 1, nullptr);
    c.setProperty ("instance_id", "0123456789abcdef0123456789abcdef", nullptr);
    c.setProperty ("plugin_kind", klasse, nullptr);
    c.setProperty ("measurement_position", position, nullptr);
    c.setProperty ("label", "Probe", nullptr);
    v.appendChild (c, nullptr);
    if (mitMainProject)
    {
        juce::ValueTree m ("MainProject");
        m.setProperty ("schema", 1, nullptr);
        v.appendChild (m, nullptr);
    }
    return v;
}

juce::ValueTree parametersKind (const param::Satz& s)
{
    juce::ValueTree p ("Parameters");
    p.setProperty ("schema", 1, nullptr);
    param::schreibeInBaum (s, p);
    return p;
}

/** Feindlicher ValueTree-Stream: ein winziger Puffer behauptet eine fast
    2-GiB-Binaervariante. Der State-Leser darf dafuer nichts allokieren. */
juce::MemoryBlock riesenVarianteOhneNutzdaten()
{
    juce::MemoryBlock b;
    juce::MemoryOutputStream s (b, false);
    s.writeString ("NakamaState");
    s.writeCompressedInt (1);
    s.writeString ("payload");
    s.writeCompressedInt (std::numeric_limits<int>::max());
    s.writeByte (8); // JUCE varMarker_Binary
    s.flush();
    return b;
}

juce::MemoryBlock zuTieferBaum (int kinder)
{
    juce::ValueTree wurzel ("NakamaState");
    auto cursor = wurzel;
    for (int i = 0; i < kinder; ++i)
    {
        juce::ValueTree kind ("Depth");
        cursor.appendChild (kind, nullptr);
        cursor = kind;
    }
    return alsBlock (wurzel);
}

juce::MemoryBlock baumMitRohVariante (const juce::MemoryBlock& variante)
{
    juce::MemoryBlock b;
    juce::MemoryOutputStream s (b, false);
    auto eigenschaft = [&s] (const char* name, const juce::var& wert)
    {
        s.writeString (name);
        wert.writeToStream (s);
    };

    s.writeString ("NakamaState");
    s.writeCompressedInt (1);
    eigenschaft ("schema", 2);
    s.writeCompressedInt (1);

    s.writeString ("Common");
    s.writeCompressedInt (6);
    eigenschaft ("schema", 1);
    eigenschaft ("instance_id", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    eigenschaft ("plugin_kind", "legacy");
    eigenschaft ("measurement_position", "insert");
    eigenschaft ("label", "Bytegate");
    s.writeString ("future_value");
    s.write (variante.getData(), variante.getSize());
    s.writeCompressedInt (0);
    s.flush();
    return b;
}

juce::MemoryBlock markerVariante (std::uint8_t marker)
{
    juce::MemoryBlock b;
    juce::MemoryOutputStream s (b, false);
    s.writeCompressedInt (1);
    s.writeByte (static_cast<char> (marker));
    s.flush();
    return b;
}

juce::MemoryBlock variantenArray (int verschachtelung)
{
    juce::MemoryBlock inner;
    {
        juce::MemoryOutputStream s (inner, false);
        juce::var (1).writeToStream (s);
        s.flush();
    }

    for (int tiefe = 0; tiefe < verschachtelung; ++tiefe)
    {
        juce::MemoryBlock nutzdaten;
        juce::MemoryOutputStream n (nutzdaten, false);
        n.writeCompressedInt (1);
        n.write (inner.getData(), inner.getSize());
        n.flush();

        juce::MemoryBlock aussen;
        juce::MemoryOutputStream a (aussen, false);
        a.writeCompressedInt (static_cast<int> (nutzdaten.getSize() + 1));
        a.writeByte (7); // JUCE varMarker_Array
        a.write (nutzdaten.getData(), nutzdaten.getSize());
        a.flush();
        inner = aussen;
    }
    return baumMitRohVariante (inner);
}

juce::MemoryBlock stateMitBallast (size_t bytes)
{
    auto v = schema2Baum ("legacy", "insert", false);
    juce::MemoryBlock ballast;
    ballast.setSize (bytes, true);
    v.getChildWithName ("Common").setProperty ("future_ballast", juce::var (ballast), nullptr);
    return alsBlock (v);
}

juce::MemoryBlock stateMitZielgroesse (size_t ziel)
{
    size_t ballast = ziel > 1024u ? ziel - 1024u : 0u;
    juce::MemoryBlock roh;
    for (int versuch = 0; versuch < 8; ++versuch)
    {
        roh = stateMitBallast (ballast);
        if (roh.getSize() == ziel)
            break;
        if (roh.getSize() < ziel)
            ballast += ziel - roh.getSize();
        else
            ballast -= std::min (ballast, roh.getSize() - ziel);
    }
    return roh;
}

juce::MemoryBlock baumMitEigenschaftszahl (int anzahl)
{
    juce::MemoryBlock b;
    juce::MemoryOutputStream s (b, false);
    s.writeString ("NakamaState");
    s.writeCompressedInt (anzahl);
    for (int i = 0; i < anzahl; ++i)
    {
        s.writeString ("x");
        juce::var().writeToStream (s);
    }
    s.writeCompressedInt (0);
    s.flush();
    return b;
}

juce::MemoryBlock baumMitGesamteintraegen (int gesamt)
{
    // Vier Kinder zaehlen selbst als vier Sammlungseintraege. Der Rest wird
    // so verteilt, dass keine einzelne Property-Sammlung 65.536 ueberschreitet.
    juce::MemoryBlock b;
    juce::MemoryOutputStream s (b, false);
    s.writeString ("NakamaState");
    s.writeCompressedInt (0);
    s.writeCompressedInt (4);
    int rest = gesamt - 4;
    for (int kind = 0; kind < 4; ++kind)
    {
        const int n = std::min (65536, rest);
        rest -= n;
        s.writeString ("Future");
        s.writeCompressedInt (n);
        for (int i = 0; i < n; ++i)
        {
            s.writeString ("x");
            juce::var().writeToStream (s);
        }
        s.writeCompressedInt (0);
    }
    s.flush();
    return b;
}

} // namespace

// AudioProcessor-Instanzen werden auch vom Host auf dem Heap erzeugt. Das ist
// hier zugleich ein Sicherheitsriegel: mehrere EqCopilotProcessor als lokale
// main()-Objekte liessen dem bewusst gefahrenen ValueTree-Tiefenrand keinen
// verlaesslichen Stack mehr.
int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceStart;

    bool schreibeGoldens = false;
    for (int i = 1; i < argc; ++i)
        if (juce::String (argv[i]) == "--schreibe-goldens")
            schreibeGoldens = true;

    const auto vertragDatei   = finde ("eq-copilot/schemas/state/nakama-parameter-v2.json");
    const auto vertragV1Datei = finde ("eq-copilot/schemas/state/nakama-parameter-v1.json");
    const auto presetDatei    = finde ("eq-copilot/schemas/state/nakama-preset-v1.json");
    const auto fixtureOrdner  = finde ("eq-copilot/fixtures/state");
    std::cout << "Vertrag:  " << vertragDatei.getFullPathName() << std::endl
              << "Fixtures: " << fixtureOrdner.getFullPathName() << std::endl << std::endl;

    // ══════════════════════════════════════════════════════════════════════
    // G4 · Parametertabelle deckungsgleich mit dem Vertrag (Layout v2)
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        bool ok = false, okV1 = false;
        const auto v   = ladeJson (vertragDatei, ok);
        const auto v1  = ladeJson (vertragV1Datei, okV1);
        pruefe (ok,   "nakama-parameter-v2.json ist gueltiges JSON");
        pruefe (okV1, "nakama-parameter-v1.json ist gueltiges JSON");
        const auto& t = param::tabelle();
        pruefe ((int) v["anzahl_parameter"] == param::kAnzahl, "anzahl_parameter == 120", v["anzahl_parameter"].toString());
        pruefe ((int) v["anzahl_parameter_v1"] == param::kAnzahlV1, "anzahl_parameter_v1 == 109", v["anzahl_parameter_v1"].toString());
        pruefe ((int) v["anzahl_host_parameter"] == param::kHostParameter, "anzahl_host_parameter == 112", v["anzahl_host_parameter"].toString());
        pruefe ((int) v["slot_anzahl"] == param::kSlots, "slot_anzahl == 8");
        pruefe ((int) v["dsp_schema_version"] == param::kDspSchemaVersion, "dsp_schema_version == 2");
        pruefe ((int) v1["dsp_schema_version"] == param::kDspSchemaVersionV1, "v1-Datei bleibt bei dsp_schema_version 1");
        pruefe ((int) v1["anzahl_parameter"] == param::kAnzahlV1, "v1-Datei bleibt bei 109 Kennungen");

        const auto* ids = v["ids"].getArray();
        pruefe (ids != nullptr && ids->size() == param::kAnzahl, "ids-Liste hat 120 Eintraege");
        if (ids != nullptr && ids->size() == param::kAnzahl)
        {
            int gleichIds = 0;
            for (int i = 0; i < param::kAnzahl; ++i)
                if ((*ids)[i].toString() == t[(size_t) i].id) ++gleichIds;
            pruefe (gleichIds == param::kAnzahl, "alle 120 IDs in Vertragsreihenfolge identisch", juce::String (gleichIds));
        }

        // M-86: die 109 v1-Kennungen stehen unveraendert VORNE, in genau der
        // Reihenfolge der eingefrorenen v1-Datei. Ein neuer Wert bekommt nie
        // einen freien Platz im Bestand.
        {
            const auto* idsV1 = v1["ids"].getArray();
            int gleichV1 = 0;
            if (idsV1 != nullptr && idsV1->size() == param::kAnzahlV1 && ids != nullptr)
                for (int i = 0; i < param::kAnzahlV1; ++i)
                    if ((*idsV1)[i].toString() == (*ids)[i].toString()) ++gleichV1;
            pruefe (gleichV1 == param::kAnzahlV1,
                    "die 109 v1-IDs stehen unveraendert am Anfang der v2-Reihenfolge", juce::String (gleichV1));
        }

        // M-88 (Vertragsseite): die 112 Host-Parameter sind das PRAEFIX der
        // Reihenfolge, `occupied` steht dahinter und ist keiner.
        {
            int hostVorne = 0, occupiedHinten = 0;
            for (int i = 0; i < param::kHostParameter; ++i)
                if (t[(size_t) i].hostParameter) ++hostVorne;
            for (int i = param::kHostParameter; i < param::kAnzahl; ++i)
                if (! t[(size_t) i].hostParameter && t[(size_t) i].id.endsWith (".occupied")) ++occupiedHinten;
            pruefe (hostVorne == param::kHostParameter && occupiedHinten == param::kSlots,
                    "112 Host-Parameter als Praefix, acht occupied dahinter",
                    juce::String (hostVorne) + "/" + juce::String (occupiedHinten));
            pruefe (t[(size_t) param::kIndexEqEnabled].id == "v2.global.eq_enabled"
                    && t[(size_t) param::kIndexMix].id == "v2.global.mix"
                    && t[(size_t) param::kIndexAutoGain].id == "v2.global.auto_gain",
                    "die drei v2-Host-Parameter haengen in R2/R3/R4-Reihenfolge hinten an");
        }

        /*  Die flache Vertragsliste: `global` + `band_vorlage` x Slots ergibt
            NICHT die Vertragsreihenfolge (die v2-Globalen stehen hinter allen
            v1-Slots). Deshalb wird ueber die ID gesucht - genau so, wie ein
            Leser es tun muesste. */
        struct Eintrag { juce::String id; juce::var wert; };
        std::vector<Eintrag> flach;
        if (const auto* global = v["global"].getArray())
            for (const auto& g : *global)
                flach.push_back ({ g["id"].toString(), g });
        if (const auto* vorlage = v["band_vorlage"].getArray())
            for (int slot = 0; slot < param::kSlots; ++slot)
                for (const auto& b : *vorlage)
                    flach.push_back ({ b["layout"].toString() + ".band."
                                       + juce::String (slot) + "." + b["name"].toString(), b });

        auto vergleiche = [&] (const juce::var& e, int index, const juce::String& wo)
        {
            const auto& b = t[(size_t) index];
            const auto typ = e["typ"].toString();
            bool gut = true;
            if (typ == "bool")
            {
                gut = b.typ == param::Typ::boolean && (bool) e["default"] == b.standardBool;
            }
            else if (typ == "float")
            {
                gut = b.typ == param::Typ::gleitkomma && (double) e["min"] == b.min && (double) e["max"] == b.max
                      && (double) e["default"] == b.standardZahl;
            }
            else if (typ == "enum")
            {
                gut = b.typ == param::Typ::aufzaehlung;
                const auto* w = e["werte"].getArray();
                if (w == nullptr || w->size() != b.werte.size()) gut = false;
                else for (int k = 0; k < w->size(); ++k) if ((*w)[k].toString() != b.werte[k]) gut = false;
                if (gut) gut = b.werte[b.standardIndex] == e["default"].toString();
            }
            else gut = false;
            const auto wechsel = e["wechsel"].toString();
            gut = gut && ((wechsel == "rampe") == (b.wechsel == param::Wechsel::rampe));
            gut = gut && ((bool) e["topologisch"] == b.topologisch);
            gut = gut && ((bool) e["host_parameter"] == b.hostParameter);
            gut = gut && (e["layout"].toString() == "v" + juce::String (b.layout));
            if (! gut)
                pruefe (false, "Eintrag weicht ab: " + wo, b.id);
            return gut;
        };

        int deckungsgleich = 0;
        for (const auto& e : flach)
        {
            const int index = param::indexVonId (e.id);
            if (index < 0) { pruefe (false, "Vertrags-ID ohne Tabelleneintrag", e.id); continue; }
            if (vergleiche (e.wert, index, e.id)) ++deckungsgleich;
        }
        pruefe (deckungsgleich == param::kAnzahl,
                "120 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch, Host-Attribut) deckungsgleich",
                juce::String (deckungsgleich));

        /*  M-86/M-87, der Rotbeweis dieser Zeile: die v2-Datei ist aus der
            EINGEFRORENEN v1-Datei ABGELEITET. Jeder Eintrag mit `layout` = v1
            ist der v1-Eintrag gleicher ID plus genau die zwei Schluessel
            `layout` und `host_parameter`. Aendert jemand einen v1-Default,
            faellt diese Zeile - nicht ein Kommentar. */
        {
            auto schluessel = [] (const juce::var& o)
            {
                juce::StringArray namen;
                if (auto* d = o.getDynamicObject())
                    for (const auto& e : d->getProperties())
                        namen.add (e.name.toString());
                namen.sort (true);
                return namen;
            };
            auto v1Eintrag = [&] (const juce::String& id) -> juce::var
            {
                if (const auto* global = v1["global"].getArray())
                    for (const auto& g : *global)
                        if (g["id"].toString() == id) return g;
                if (const auto* vorlage = v1["band_vorlage"].getArray())
                    for (const auto& b : *vorlage)
                        if (id.endsWith ("." + b["name"].toString())) return b;
                return {};
            };
            int abgeleitet = 0, erwartetAbgeleitet = 0;
            for (const auto& e : flach)
            {
                if (e.wert["layout"].toString() != "v1") continue;
                ++erwartetAbgeleitet;
                const auto alt = v1Eintrag (e.id);
                auto neuNamen = schluessel (e.wert);
                const auto altNamen = schluessel (alt);
                neuNamen.removeString ("layout");
                neuNamen.removeString ("host_parameter");
                bool gleich = neuNamen == altNamen && altNamen.size() > 0;
                for (const auto& n : altNamen)
                    if (e.wert[juce::Identifier (n)].toString() != alt[juce::Identifier (n)].toString())
                        gleich = false;
                if (gleich) ++abgeleitet;
                else pruefe (false, "v1-Eintrag nicht woertlich uebernommen", e.id);
            }
            pruefe (abgeleitet == erwartetAbgeleitet && erwartetAbgeleitet == param::kAnzahlV1,
                    "jeder v1-Eintrag ist woertlich uebernommen, nur um layout und host_parameter ergaenzt",
                    juce::String (abgeleitet) + "/" + juce::String (erwartetAbgeleitet));
        }

        pruefe (param::indexVonId ("v1.band.7.sidechain_source") == 108 && param::indexVonId ("v1.global.bypass") == 0
                && param::indexVonId ("v2.band.7.occupied") == 119 && param::indexVonId ("v1.band.8.enabled") < 0,
                "indexVonId: erste, letzte v1, letzte v2, nicht vorhandene ID");

        /*  M-95/M-97: das Presetschema. Die sechs verbotenen Namen stehen im
            Vertrag und im C++-Leser; hier wird gemessen, dass sie dieselbe
            Menge sind. Ein siebter Name im Vertrag faellt damit im Test. */
        {
            bool okP = false;
            const auto pv = ladeJson (presetDatei, okP);
            pruefe (okP, "nakama-preset-v1.json ist gueltiges JSON");
            pruefe ((int) pv["preset_schema_version"] == nakama::preset::kPresetSchemaVersion,
                    "preset_schema_version == 1");
            pruefe ((int) pv["geschrieben_fuer_dsp_schema_version"] == param::kDspSchemaVersion,
                    "das Preset nennt das Layout, fuer das es geschrieben ist");
            juce::StringArray ausVertrag;
            if (const auto* felder = pv["verbotene_felder"].getArray())
                for (const auto& f : *felder)
                    ausVertrag.add (f["name"].toString());
            auto ausCode = nakama::preset::verboteneFelder();
            ausVertrag.sort (true); ausCode.sort (true);
            pruefe (ausVertrag == ausCode && ausCode.size() == 6,
                    "die sechs verbotenen Presetfelder sind in Vertrag und Leser dieselbe Menge",
                    ausVertrag.joinIntoString (","));
        }
        a.schliesse ("Parametertabelle deckungsgleich mit nakama-parameter-v2.json, v1 unveraendert abgeleitet");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G5 · RFC 8785: Zahlenvektoren und Dokumente
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        bool ok = false;
        const auto manifest = ladeJson (fixtureOrdner.getChildFile ("MANIFEST.json"), ok);
        pruefe (ok, "fixtures/state/MANIFEST.json gelesen");

        const auto zahlen = ladeJson (fixtureOrdner.getChildFile ("jcs/zahlen.json"), ok);
        pruefe (ok, "jcs/zahlen.json gelesen");
        int vektorenOk = 0, abgelehntOk = 0, gesamt = 0;
        if (const auto* vs = zahlen["vektoren"].getArray())
        {
            for (const auto& e : *vs)
            {
                ++gesamt;
                const double x = ausHex64 (e["hex64"].toString());
                juce::String t;
                const bool geschrieben = kanon::zahlAlsEs6 (x, t);
                if ((bool) e["abgelehnt"])
                {
                    if (! geschrieben) ++abgelehntOk;
                    else pruefe (false, "Zahl haette abgelehnt werden muessen", e["hex64"].toString());
                }
                else if (geschrieben && t == e["erwartet"].toString())
                    ++vektorenOk;
                else
                    pruefe (false, "Zahlenvektor " + e["hex64"].toString(), "ist " + t + " soll " + e["erwartet"].toString());
            }
        }
        pruefe (gesamt == (int) manifest["jcs_zahlen"]["anzahl"] && vektorenOk + abgelehntOk == gesamt,
                "Zahlenvektoren: " + juce::String (vektorenOk) + " bytegleich, " + juce::String (abgelehntOk) + " abgelehnt (NaN/Inf)",
                juce::String (gesamt) + " gesamt");

        juce::String t;
        pruefe (! kanon::zahlAlsEs6 (std::numeric_limits<double>::quiet_NaN(), t)
                && ! kanon::zahlAlsEs6 (std::numeric_limits<double>::infinity(), t)
                && ! kanon::zahlAlsEs6 (-std::numeric_limits<double>::infinity(), t),
                "NaN, +Inf, -Inf werden abgelehnt");
        pruefe (kanon::zahlAlsEs6 (-0.0, t) && t == "0", "-0 wird zu 0");

        int dokOk = 0, dokGesamt = 0;
        if (const auto* ds = manifest["jcs_dokumente"].getArray())
        {
            for (const auto& e : *ds)
            {
                ++dokGesamt;
                const auto datei = fixtureOrdner.getChildFile (e["datei"].toString());
                juce::MemoryBlock roh;
                if (! datei.loadFileAsData (roh)) { pruefe (false, "Dokument fehlt", datei.getFileName()); continue; }
                kanon::Wert w; juce::String f;
                if (! kanon::lies (utf8 (roh), w, f)) { pruefe (false, "Dokument parst nicht", datei.getFileName() + ": " + f); continue; }
                juce::MemoryBlock kanonBytes;
                if (! kanon::kanonisiere (w, kanonBytes, f)) { pruefe (false, "kanonisiere", datei.getFileName() + ": " + f); continue; }
                const auto erwartet = e["kanon"].toString();
                juce::MemoryBlock erwartetBytes (erwartet.toRawUTF8(), erwartet.getNumBytesAsUTF8());
                const bool bytesGleich = gleich (kanonBytes, erwartetBytes);
                const bool hashGleich = kanon::sha256Hex (kanonBytes.getData(), kanonBytes.getSize()) == e["kanon_sha256"].toString();
                const bool laengeGleich = (int) kanonBytes.getSize() == (int) e["kanon_bytes"];
                if (bytesGleich && hashGleich && laengeGleich) ++dokOk;
                else pruefe (false, "Dokument " + e["datei"].toString(),
                             "ist " + utf8 (kanonBytes) + " | soll " + erwartet);
            }
        }
        pruefe (dokGesamt > 0 && dokOk == dokGesamt, "JCS-Dokumente bytegleich + SHA-256 gleich", juce::String (dokOk) + "/" + juce::String (dokGesamt));

        // Der eigene Leser: doppelte Schluessel (auch nach Escape-Aufloesung),
        // Subnormale, leere Schluessel, einsame Surrogate, rohe Steuerzeichen.
        {
            juce::String f; kanon::Wert w;
            pruefe (! kanon::lies ("{\"a\":1,\"a\":2}", w, f) && f.startsWith ("doppelter Schluessel"), "doppelter Schluessel erkannt", f);
            pruefe (! kanon::lies ("{\"\\u0041\":1,\"A\":2}", w, f) && f.startsWith ("doppelter Schluessel"), "doppelter Schluessel nach Escape-Aufloesung erkannt", f);
            pruefe (kanon::lies ("{\"a\":{\"a\":1},\"b\":[{\"a\":1},{\"a\":2}]}", w, f), "gleicher Schluessel in verschiedenen Objekten ist erlaubt");
            pruefe (kanon::lies ("{\"\":1}", w, f) && w.objektSchluessel.size() == 1 && w.objektSchluessel[0].isEmpty(), "leerer Schluessel ist erlaubt (RFC 8785)");
            pruefe (kanon::lies ("5e-324", w, f) && w.art == kanon::Wert::Art::zahl && w.zahl > 0.0 && w.zahl < 1e-300, "Subnormale werden gelesen, nicht geflusht");
            pruefe (! kanon::lies ("1e999", w, f) && f.startsWith ("Zahl nicht darstellbar"), "Ueberlauf ist ein Fehler, kein inf", f);
            pruefe (! kanon::lies ("{\"k\":\"\\ud834\"}", w, f), "hohes Surrogat ohne Paar ist ein Fehler", f);
            pruefe (! kanon::lies ("{\"k\":\"a\tb\"}", w, f), "rohes Steuerzeichen in Zeichenkette ist ein Fehler", f);
            pruefe (! kanon::lies ("01", w, f) && ! kanon::lies ("1.", w, f) && ! kanon::lies ("+1", w, f) && ! kanon::lies ("[1,]", w, f) && ! kanon::lies ("{} x", w, f),
                    "Grammatikfehler werden abgewiesen");
            pruefe (kanon::lies (" [ -0 , 1E+2 , 1e-7 , true , null , \"\\u00e9\\/\" ] ", w, f) && w.liste.size() == 6 && w.liste[5].text == juce::String::fromUTF8 ("\xc3\xa9/"),
                    "Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes");
        }
        a.schliesse ("JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G6 · DSP-DTO: gueltig gehasht, ungueltig vor dem Hash abgelehnt
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        bool ok = false;
        const auto manifest = ladeJson (fixtureOrdner.getChildFile ("MANIFEST.json"), ok);
        int gueltigOk = 0, gueltigGesamt = 0, ungueltigOk = 0, ungueltigGesamt = 0;

        if (const auto* gs = manifest["dto_gueltig"].getArray())
        {
            for (const auto& e : *gs)
            {
                ++gueltigGesamt;
                const auto datei = fixtureOrdner.getChildFile (e["datei"].toString());
                juce::MemoryBlock roh;
                if (! datei.loadFileAsData (roh)) { pruefe (false, "DTO fehlt", datei.getFileName()); continue; }
                param::DspSatz s; juce::String grund, detail;
                if (! param::ausDtoText (roh.getData(), roh.getSize(), s, grund, detail))
                {
                    pruefe (false, "gueltiges DTO abgelehnt: " + e["datei"].toString(), grund + " " + detail);
                    continue;
                }
                juce::String hash;
                if (! param::stateHash (s, hash, grund)) { pruefe (false, "stateHash", grund); continue; }
                juce::MemoryBlock k;
                param::dtoKanon (s, k, grund);
                if (hash == e["state_hash"].toString() && (int) k.getSize() == (int) e["kanon_bytes"]) ++gueltigOk;
                else pruefe (false, "state_hash weicht ab: " + e["datei"].toString(), "ist " + hash + " soll " + e["state_hash"].toString());
            }
        }
        pruefe (gueltigGesamt > 0 && gueltigOk == gueltigGesamt, "gueltige DTOs: state_hash wie im Manifest", juce::String (gueltigOk) + "/" + juce::String (gueltigGesamt));

        // Die Defaults der C++-Tabelle ergeben DENSELBEN Hash wie dto/gueltig/default.json:
        // damit haengen JSON-Defaults und C++-Defaults am selben Beweis.
        {
            juce::String hash, grund, soll;
            if (const auto* gs = manifest["dto_gueltig"].getArray())
                for (const auto& e : *gs)
                    if (e["datei"].toString().endsWith ("default.json")) soll = e["state_hash"].toString();
            pruefe (param::stateHash (param::DspSatz{}, hash, grund) && hash == soll,
                    "standardSatz() ohne Zone hasht wie dto/gueltig/default.json", hash);
        }

        if (const auto* us = manifest["dto_ungueltig"].getArray())
        {
            for (const auto& e : *us)
            {
                ++ungueltigGesamt;
                const auto datei = fixtureOrdner.getChildFile (e["datei"].toString());
                juce::MemoryBlock roh;
                if (! datei.loadFileAsData (roh)) { pruefe (false, "DTO fehlt", datei.getFileName()); continue; }
                param::DspSatz s; juce::String grund, detail;
                const bool angenommen = param::ausDtoText (roh.getData(), roh.getSize(), s, grund, detail);
                if (! angenommen && grund == e["grund"].toString()) ++ungueltigOk;
                else pruefe (false, "ungueltiges DTO: " + e["datei"].toString(),
                             (angenommen ? juce::String ("ANGENOMMEN") : "Grund " + grund + " (" + detail + ")") + " soll " + e["grund"].toString());
            }
        }
        pruefe (ungueltigGesamt > 0 && ungueltigOk == ungueltigGesamt, "ungueltige DTOs mit dem erwarteten Grund abgelehnt", juce::String (ungueltigOk) + "/" + juce::String (ungueltigGesamt));

        // Nichtendlich aus dem TYPISIERTEN Satz (nicht nur aus Text).
        {
            param::DspSatz s;
            s.werte[2].zahl = std::numeric_limits<double>::quiet_NaN();
            juce::String hash, grund;
            pruefe (! param::stateHash (s, hash, grund) && grund == "nichtendlich", "NaN im Satz faellt vor dem Hash", grund);
            s.werte[2].zahl = std::numeric_limits<double>::infinity();
            pruefe (! param::stateHash (s, hash, grund) && grund == "nichtendlich", "Inf im Satz faellt vor dem Hash", grund);
            s.werte[2].zahl = 24.000001;
            pruefe (! param::stateHash (s, hash, grund) && grund == "bereich", "Bereich im Satz faellt vor dem Hash", grund);
            s.werte[2].zahl = 0.0; s.werte[6].enumIndex = 99;
            pruefe (! param::stateHash (s, hash, grund) && grund == "enum", "Enumindex im Satz faellt vor dem Hash", grund);

            // SONDE-015 R6, M-66/M-73: die Zonenregeln fallen an derselben
            // Stelle wie ein Wertfehler - VOR dem Hash, nie erst in einer
            // Oberflaeche.
            param::DspSatz z;
            for (int i = 0; i < param::kMaxZonen + 1; ++i)
                z.zonen.push_back ({ i, 100.0, 200.0, true });
            pruefe (! param::stateHash (z, hash, grund) && grund == "zone_anzahl", "neunte Zone faellt vor dem Hash", grund);
            z.zonen.resize (2); z.zonen[0] = { 3, 100.0, 200.0, true }; z.zonen[1] = { 3, 300.0, 400.0, false };
            pruefe (! param::stateHash (z, hash, grund) && grund == "zone_doppelt", "doppelte Zonen-id faellt vor dem Hash", grund);
            z.zonen[1] = { 1, 300.0, 400.0, false };
            pruefe (! param::stateHash (z, hash, grund) && grund == "zone_sortierung", "unsortierte Zonenliste faellt vor dem Hash", grund);
            z.zonen.resize (1); z.zonen[0] = { 0, 200.0, 200.0, true };
            pruefe (! param::stateHash (z, hash, grund) && grund == "bereich", "low == high faellt vor dem Hash", grund);
            z.zonen[0] = { 0, 19.9, 200.0, true };
            pruefe (! param::stateHash (z, hash, grund) && grund == "bereich", "low unter 20 Hz faellt vor dem Hash", grund);
            z.zonen[0] = { 0, 100.0, 20000.1, true };
            pruefe (! param::stateHash (z, hash, grund) && grund == "bereich", "high ueber 20 kHz faellt vor dem Hash", grund);
            z.zonen[0] = { param::kMaxZonen, 100.0, 200.0, true };
            pruefe (! param::stateHash (z, hash, grund) && grund == "bereich", "Zonen-id 8 faellt vor dem Hash", grund);
        }
        a.schliesse ("DTO: " + juce::String (gueltigOk) + " gueltige gehasht, " + juce::String (ungueltigOk) + " ungueltige vor dem Hash abgelehnt");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G6b · Preset (SONDE-015 R12, M-95 bis M-99)
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        bool ok = false;
        const auto manifest = ladeJson (fixtureOrdner.getChildFile ("MANIFEST.json"), ok);
        pruefe (ok, "MANIFEST.json ist gueltiges JSON");
        int gueltigOk = 0, gueltigGesamt = 0, ungueltigOk = 0, ungueltigGesamt = 0;

        if (const auto* gs = manifest["preset_gueltig"].getArray())
        {
            for (const auto& e : *gs)
            {
                ++gueltigGesamt;
                const auto datei = fixtureOrdner.getChildFile (e["datei"].toString());
                juce::MemoryBlock roh;
                if (! datei.loadFileAsData (roh)) { pruefe (false, "Preset fehlt", datei.getFileName()); continue; }
                param::DspSatz s; juce::String grund, detail;
                if (! nakama::preset::lies (roh.getData(), roh.getSize(), s, grund, detail))
                {
                    pruefe (false, "gueltiges Preset abgelehnt: " + e["datei"].toString(), grund + " " + detail);
                    continue;
                }
                ++gueltigOk;
            }
        }
        pruefe (gueltigGesamt > 0 && gueltigOk == gueltigGesamt, "gueltige Presets angenommen",
                juce::String (gueltigOk) + "/" + juce::String (gueltigGesamt));

        if (const auto* us = manifest["preset_ungueltig"].getArray())
        {
            for (const auto& e : *us)
            {
                ++ungueltigGesamt;
                const auto datei = fixtureOrdner.getChildFile (e["datei"].toString());
                juce::MemoryBlock roh;
                if (! datei.loadFileAsData (roh)) { pruefe (false, "Preset fehlt", datei.getFileName()); continue; }
                param::DspSatz s; juce::String grund, detail;
                const bool angenommen = nakama::preset::lies (roh.getData(), roh.getSize(), s, grund, detail);
                if (! angenommen && grund == e["grund"].toString()) ++ungueltigOk;
                else pruefe (false, "ungueltiges Preset: " + e["datei"].toString(),
                             (angenommen ? juce::String ("ANGENOMMEN") : "Grund " + grund + " (" + detail + ")")
                             + " soll " + e["grund"].toString());
            }
        }
        pruefe (ungueltigGesamt > 0 && ungueltigOk == ungueltigGesamt,
                "ungueltige Presets mit dem erwarteten Grund abgelehnt",
                juce::String (ungueltigOk) + "/" + juce::String (ungueltigGesamt));

        /*  M-95: der Writer erzeugt genau die Bytes des Fixtures. Ein
            Probe-Datensatz "in der Form des Writers" kommt vom Writer
            (Pruefliste E) - hier wird das gemessen, nicht behauptet. */
        {
            juce::MemoryBlock geschrieben;
            juce::String grund;
            param::DspSatz neutral;
            const bool gut = nakama::preset::schreibe (neutral, geschrieben, grund);
            const auto datei = fixtureOrdner.getChildFile ("preset/gueltig/default.json");
            juce::MemoryBlock erwartet;
            const bool gelesen = datei.existsAsFile() && datei.loadFileAsData (erwartet);
            pruefe (gut && gelesen && gleich (geschrieben, erwartet),
                    "der Preset-Writer erzeugt preset/gueltig/default.json bytegleich",
                    grund + " " + juce::String ((int) geschrieben.getSize()) + " Bytes");
        }

        /*  M-97, die eigentliche Zusage: `eq_enabled` bleibt beim Laden
            UNBERUEHRT. Das Preset hat kein Feld dafuer - also darf das Laden
            den Rollenschalter weder ein- noch ausschalten. */
        {
            const auto datei = fixtureOrdner.getChildFile ("preset/gueltig/gemischt.json");
            juce::MemoryBlock roh;
            if (datei.existsAsFile() && datei.loadFileAsData (roh))
            {
                param::DspSatz an;
                an.werte[(size_t) param::kIndexEqEnabled].b = true;
                juce::String grund, detail;
                const bool gut = nakama::preset::lies (roh.getData(), roh.getSize(), an, grund, detail);
                pruefe (gut && an.werte[(size_t) param::kIndexEqEnabled].b,
                        "ein geladenes Preset schaltet den eingeschalteten Rollenschalter nicht aus", grund);
                param::DspSatz aus;
                const bool gut2 = nakama::preset::lies (roh.getData(), roh.getSize(), aus, grund, detail);
                pruefe (gut2 && ! aus.werte[(size_t) param::kIndexEqEnabled].b,
                        "und schaltet den ausgeschalteten nicht ein (nichts Ungefragtes)", grund);
                pruefe (gut && gut2 && an.zonen.size() == 1 && aus.zonen == an.zonen,
                        "der Klanginhalt kommt in beiden Faellen identisch an");
            }
            else pruefe (false, "preset/gueltig/gemischt.json fehlt");
        }

        /*  Der Rueckweg: schreiben -> lesen -> schreiben ist bytegleich, und
            der gelesene Klanginhalt ist derselbe. Speichern und Laden
            gehoeren in denselben Aenderungssatz. */
        {
            param::DspSatz vorher;
            vorher.werte[(size_t) param::indexBandV1 (0, param::kEnabled)].b = true;
            vorher.werte[(size_t) param::indexBandV1 (0, param::kQ)].zahl = 0.7071067811865476;
            vorher.werte[(size_t) param::indexOccupied (0)].b = true;
            vorher.werte[(size_t) param::kIndexMix].zahl = 0.30000000000000004;
            vorher.zonen = { { 1, 55.0, 220.0, true } };
            juce::MemoryBlock a1, a2;
            juce::String grund, detail;
            const bool s1 = nakama::preset::schreibe (vorher, a1, grund);
            param::DspSatz zurueck;
            const bool l1 = s1 && nakama::preset::lies (a1.getData(), a1.getSize(), zurueck, grund, detail);
            const bool s2 = l1 && nakama::preset::schreibe (zurueck, a2, grund);
            pruefe (s1 && l1 && s2 && gleich (a1, a2) && zurueck == vorher,
                    "Preset schreiben->lesen->schreiben ist bytegleich und feldgleich", grund + " " + detail);
        }
        a.schliesse ("Preset: " + juce::String (gueltigOk) + " gueltige gelesen, "
                     + juce::String (ungueltigOk) + " ungueltige mit Grund abgelehnt, Writer bytegleich");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G7 · Migration Schema 1 → 2: rein, deterministisch, Golden
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        const auto goldenOrdner = fixtureOrdner.getChildFile ("schema2");
        if (schreibeGoldens) goldenOrdner.createDirectory();
        int bytegleich = 0;
        for (const auto& r : kRollen)
        {
            const auto saat = saatSchema1 (r.name, r.label, r.paar);
            state::Zustand z;
            const auto erg = state::lade (saat.getData(), saat.getSize(), state::Bundle::eqcp(), z);
            pruefe (erg == state::LadeErgebnis::migriert, juce::String ("Schema-1 '") + r.name + "' migriert");
            pruefe (z.herkunft == state::Herkunft::schema1Migriert, juce::String ("'") + r.name + "': Herkunft schema1Migriert");
            pruefe (z.common.klasse == r.klasse && z.common.position == r.pos,
                    juce::String ("'") + r.name + "' -> " + state::wort (r.klasse) + "+" + state::wort (r.pos),
                    juce::String (state::wort (z.common.klasse)) + "+" + state::wort (z.common.position));
            pruefe (z.common.instanceId == "11111111-2222-3333-4444-555555555555", juce::String ("'") + r.name + "': sensor_id bytegleich als instance_id");
            pruefe (z.common.label == r.label && z.common.pairId == r.paar, juce::String ("'") + r.name + "': label und pair_id bytegleich");
            pruefe (z.common.projectBindingId.isEmpty(), juce::String ("'") + r.name + "': project_binding_id NICHT erfunden");
            pruefe (state::v2Rolle (z.common) == r.name, juce::String ("'") + r.name + "': v2-Rolle rueckwaerts identisch", state::v2Rolle (z.common));
            const bool mainProject = z.baum.getChildWithName ("MainProject").isValid();
            pruefe (mainProject == (r.klasse == state::Klasse::main), juce::String ("'") + r.name + "': MainProject genau fuer main");
            pruefe (z.common.pairId.isEmpty() == ! z.baum.getChildWithName ("Common").hasProperty ("pair_id"),
                    juce::String ("'") + r.name + "': pair_id-Eigenschaft nur wenn nicht leer");

            juce::MemoryBlock heraus;
            state::speichere (z, heraus);

            // Determinismus: zweimal migrieren -> gleiche Bytes.
            state::Zustand z2;
            state::lade (saat.getData(), saat.getSize(), state::Bundle::eqcp(), z2);
            juce::MemoryBlock heraus2;
            state::speichere (z2, heraus2);
            pruefe (gleich (heraus, heraus2), juce::String ("'") + r.name + "': Migration deterministisch");

            // Idempotenz: das Ergebnis erneut laden und speichern ist die Identitaet.
            state::Zustand z3;
            const auto erg3 = state::lade (heraus.getData(), heraus.getSize(), state::Bundle::eqcp(), z3);
            juce::MemoryBlock heraus3;
            state::speichere (z3, heraus3);
            pruefe (erg3 == state::LadeErgebnis::geladen && gleich (heraus, heraus3), juce::String ("'") + r.name + "': Migration der Migration ist Identitaet");

            const auto golden = goldenOrdner.getChildFile (juce::String ("aus-schema1-") + r.name + ".bin");
            if (schreibeGoldens)
            {
                golden.replaceWithData (heraus.getData(), heraus.getSize());
                std::cout << "  geschrieben: " << golden.getFullPathName().toRawUTF8() << std::endl;
                ++bytegleich;
                continue;
            }
            juce::MemoryBlock erwartet;
            const bool gelesen = golden.existsAsFile() && golden.loadFileAsData (erwartet);
            pruefe (gelesen, juce::String ("Golden '") + r.name + "' gelesen", golden.getFileName());
            if (gelesen && gleich (erwartet, heraus)) ++bytegleich;
            else if (gelesen) pruefe (false, juce::String ("Golden '") + r.name + "' weicht ab", juce::String ((int) heraus.getSize()) + " Bytes");
        }
        pruefe (bytegleich == 4, "4 Rollen bytegleich zum Schema-2-Golden", juce::String (bytegleich));

        // Dazu EIN read-only-Fixture (Root-Major 3) fuer den Sichtbeweis mit
        // EqCopShot --state und als Lade-Fixture fuer spaetere Builds.
        {
            auto v = schema2Baum ("legacy", "insert", false);
            v.setProperty ("schema", 3, nullptr);
            v.getChildWithName ("Common").setProperty ("label", "Probe aus der Zukunft", nullptr);
            const auto bytes = alsBlock (v);
            const auto datei = goldenOrdner.getChildFile ("fremdes-major-3.bin");
            if (schreibeGoldens)
            {
                datei.replaceWithData (bytes.getData(), bytes.getSize());
                std::cout << "  geschrieben: " << datei.getFullPathName().toRawUTF8() << std::endl;
            }
            juce::MemoryBlock auf;
            pruefe (datei.existsAsFile() && datei.loadFileAsData (auf) && gleich (auf, bytes), "read-only-Fixture fremdes-major-3.bin bytegleich");
        }

        /*  SONDE-014 Etappe A: ein Golden fuer den musikalischen Intent.

            §5.5 des Bauplans, woertlich: "ein Probe-Datensatz 'in der Form des
            Writers' wird vom Writer erzeugt und eingefroren". Genau deshalb
            steht hier keine handgeschriebene Bytefolge, sondern derselbe
            `state::speichere`, den das Produkt fuehrt - und der Lauf ohne
            `--schreibe-goldens` misst dagegen.

            Der Stand traegt alle vier neuen Eigenschaften mit ihren Raendern:
            zwei Scopes derselben Quelle (M-08), alle drei Herkuenfte, ein
            Bandintervall am vollen Rand [0, 221), eine gerichtete und eine
            als gleichrangig markierte Kante (M-06) sowie die
            Bestandsrevision. */
        {
            const auto q1 = juce::String::repeatedString ("a", 32);
            const auto q2 = juce::String::repeatedString ("b", 32);
            const auto q3 = juce::String::repeatedString ("c", 32);
            const auto pas = juce::String::repeatedString ("d", 32);

            state::Zustand z = state::frisch (juce::String::repeatedString ("1", 32));
            z.common.klasse = state::Klasse::main;
            z.common.position = state::Messposition::insert;
            z.common.label = "Leitstand";
            bool v = false; juce::String g;
            state::setzeIntent (z, q1, {},  state::Rolle::fuehrt,       state::IntentHerkunft::user,       1.0,  v, g);
            state::setzeIntent (z, q1, pas, state::Rolle::begleitet,    state::IntentHerkunft::vorlage,    0.5,  v, g);
            state::setzeIntent (z, q2, {},  state::Rolle::verschmolzen, state::IntentHerkunft::abgeleitet, 0.25, v, g);
            state::setzeIntent (z, q3, {},  state::Rolle::geschuetzt,   state::IntentHerkunft::user,       1.0,  v, g);
            state::setzeSchutzangabe (z, q1, state::Schutzeigenschaft::attack, -1, -1, v, g);
            state::setzeSchutzangabe (z, q2, state::Schutzeigenschaft::band, 0,
                                      state::bandAnzahlEvidenzgitter, v, g);
            state::setzeBeziehung (z, q1, q2, state::Beziehungsart::fuehrtVor, v, g);
            state::speichereAlsGleichrangigkeit (z, q2, q1, v, g);

            juce::MemoryBlock bytes;
            state::speichere (z, bytes);
            const auto datei = goldenOrdner.getChildFile ("main-intent-v1.bin");
            if (schreibeGoldens)
            {
                datei.replaceWithData (bytes.getData(), bytes.getSize());
                std::cout << "  geschrieben: " << datei.getFullPathName().toRawUTF8() << std::endl;
            }
            juce::MemoryBlock auf;
            const bool gelesen = datei.existsAsFile() && datei.loadFileAsData (auf);
            pruefe (gelesen && gleich (auf, bytes),
                    "Intent-Golden main-intent-v1.bin bytegleich zum Writer");

            // Und der Rueckweg am Golden: laden, wieder speichern, bytegleich.
            state::Zustand zurueck;
            const auto ergInt = state::lade (bytes.getData(), bytes.getSize(),
                                             state::Bundle::eqcp(), zurueck);
            pruefe (ergInt == state::LadeErgebnis::geladen && ! zurueck.nurLesen,
                    "Intent-Golden laedt normal");
            pruefe (zurueck.sourceIntents.size() == 4 && zurueck.schutzangaben.size() == 2
                        && zurueck.intentBeziehungen.size() == 2
                        && zurueck.intentBestandRevision == 8,
                    "und traegt vier Intents, zwei Schutzangaben, zwei Kanten, Revision 8");
            juce::MemoryBlock nochmal;
            state::speichere (zurueck, nochmal);
            pruefe (gleich (bytes, nochmal), "Save nach Load des Intent-Goldens ist bytegleich");
        }

        /*  SONDE-015 (M-89, M-90, M-93): zwei Writer-Goldens fuer das Kind
            `Dsp` und fuer das Layout v1.

            `dsp-v2-voll.bin` ist ein active_probe-Stand mit allem, was `Dsp`
            tragen kann; `layout-v1.bin` ist ein Stand OHNE
            `dsp_schema_version` - also genau das, was jeder heutige Build
            schreibt. An ihm faellt die Migrationsregel R5, wenn sie sich
            aendert. Beide kommen aus dem WRITER, nicht aus der Hand
            (Pruefliste E). */
        {
            state::Zustand zv = state::frisch (juce::String::repeatedString ("7", 32));
            zv.common.klasse = state::Klasse::active_probe;
            zv.common.label = "Sonde mit DSP";
            zv.hatParameters = true;
            zv.stateRevision = 12;
            zv.parameters[(size_t) param::indexBandV1 (2, param::kEnabled)].b = true;
            zv.parameters[(size_t) param::indexBandV1 (2, param::kFreqHz)].zahl = 220.5;
            zv.parameters[(size_t) param::indexOccupied (2)].b = true;
            zv.parameters[(size_t) param::kIndexMix].zahl = 0.5;
            zv.schutzZonen = { { 0, 40.0, 120.0, true }, { 3, 900.0, 1100.0, false } };
            {
                state::UndoEintrag u;
                u.art = state::UndoArt::apply;
                u.slot = 2;
                u.revision = 11;
                u.zustand.werte = param::standardSatz();
                zv.undoRing.push_back (std::move (u));
            }
            juce::MemoryBlock bytes;
            state::speichere (zv, bytes);
            const auto datei = goldenOrdner.getChildFile ("dsp-v2-voll.bin");
            if (schreibeGoldens)
            {
                datei.replaceWithData (bytes.getData(), bytes.getSize());
                std::cout << "  geschrieben: " << datei.getFullPathName().toRawUTF8() << std::endl;
            }
            juce::MemoryBlock auf;
            pruefe (datei.existsAsFile() && datei.loadFileAsData (auf) && gleich (auf, bytes),
                    "Dsp-Golden dsp-v2-voll.bin bytegleich zum Writer");

            state::Zustand zurueck;
            pruefe (state::lade (bytes.getData(), bytes.getSize(), state::Bundle::nkac(), zurueck)
                        == state::LadeErgebnis::geladen && ! zurueck.nurLesen,
                    "Dsp-Golden laedt normal", zurueck.grund);
            pruefe (zurueck.hatDsp && zurueck.stateRevision == 12
                        && zurueck.schutzZonen == zv.schutzZonen
                        && zurueck.undoRing == zv.undoRing
                        && zurueck.parameters[(size_t) param::indexOccupied (2)].b,
                    "und traegt Revision 12, zwei Zonen, einen Undo-Eintrag und Slot 2 belegt");
        }

        /*  M-65/M-90: ein Stand im LAYOUT V1 laedt verlustfrei nach v2.

            Der Baum wird aus dem Writer erzeugt und danach genau um das
            zurueckgebaut, was Layout v1 nicht hatte: die Eigenschaft
            `dsp_schema_version` und die drei v2-Host-Parameter. Das ist der
            einzige ehrliche Weg, einen v1-Stand mit einem v2-Writer zu
            erzeugen - eine handgeschriebene Bytefolge waere kein Beweis
            darueber, was frueher wirklich geschrieben wurde. */
        {
            state::Zustand zv = state::frisch (juce::String::repeatedString ("6", 32));
            zv.common.klasse = state::Klasse::active_probe;
            zv.common.label = "Sonde aus Layout v1";
            zv.hatParameters = true;
            // Slot 1: eingeschaltet. Slot 4: AUS, aber die Frequenz ist
            // verstellt - genau der Fall, den R5 nicht verlieren darf.
            zv.parameters[(size_t) param::indexBandV1 (1, param::kEnabled)].b = true;
            zv.parameters[(size_t) param::indexBandV1 (4, param::kFreqHz)].zahl = 3150.0;
            juce::MemoryBlock v2Bytes;
            state::speichere (zv, v2Bytes);

            auto baum = juce::ValueTree::readFromData (v2Bytes.getData(), v2Bytes.getSize());
            auto p = baum.getChildWithName ("Parameters");
            p.removeProperty ("dsp_schema_version", nullptr);
            p.removeProperty ("v2.global.eq_enabled", nullptr);
            p.removeProperty ("v2.global.mix", nullptr);
            p.removeProperty ("v2.global.auto_gain", nullptr);
            const auto bytes = alsBlock (baum);
            const auto datei = goldenOrdner.getChildFile ("layout-v1.bin");
            if (schreibeGoldens)
            {
                datei.replaceWithData (bytes.getData(), bytes.getSize());
                std::cout << "  geschrieben: " << datei.getFullPathName().toRawUTF8() << std::endl;
            }
            juce::MemoryBlock auf;
            pruefe (datei.existsAsFile() && datei.loadFileAsData (auf) && gleich (auf, bytes),
                    "Layout-v1-Golden layout-v1.bin bytegleich");

            state::Zustand z1;
            const auto erg = state::lade (bytes.getData(), bytes.getSize(), state::Bundle::nkac(), z1);
            pruefe (erg == state::LadeErgebnis::geladen && ! z1.nurLesen && z1.layoutV1Migriert,
                    "ein v1-Stand laedt verlustfrei und wird als migriert gemeldet", z1.grund);
            pruefe (! z1.parameters[(size_t) param::kIndexEqEnabled].b
                        && z1.parameters[(size_t) param::kIndexMix].zahl == 1.0
                        && ! z1.parameters[(size_t) param::kIndexAutoGain].b,
                    "die drei v2-Host-Parameter bekommen ihren Default, nichts wird erfunden");
            pruefe (z1.parameters[(size_t) param::indexOccupied (1)].b
                        && z1.parameters[(size_t) param::indexOccupied (4)].b,
                    "occupied := enabled ODER ein Wert weicht ab - das ausgeschaltete Band 4 ueberlebt");
            int frei = 0;
            for (int slot = 0; slot < param::kSlots; ++slot)
                if (! z1.parameters[(size_t) param::indexOccupied (slot)].b) ++frei;
            pruefe (frei == param::kSlots - 2, "sechs nie beruehrte Slots bleiben frei", juce::String (frei));
            pruefe (! z1.hatDsp && z1.stateRevision == 0 && z1.schutzZonen.empty() && z1.undoRing.empty(),
                    "ein v1-Stand bringt kein Dsp-Kind mit: Revision 0, keine Zone, kein Undo");

            /*  Der bitgenaue Vergleich der Migrationsregel: ein Wert, der
                sich nur im Vorzeichen der Null unterscheidet, gilt als
                beruehrt. Der Wert wird ueber den PRODUKTSCHREIBER gesetzt,
                nicht ueber `ValueTree::setProperty` am rohen Baum - `var`
                haelt 0.0 und -0.0 fuer gleich und wuerde den Schreibvorgang
                ueberspringen. Genau daran ist der Writer am 10.09.2026
                gefallen; die Zeile misst jetzt beides. */
            {
                state::Zustand zm = state::frisch (juce::String::repeatedString ("3", 32));
                zm.common.klasse = state::Klasse::active_probe;
                zm.hatParameters = true;
                zm.parameters[(size_t) param::indexBandV1 (6, param::kGainDb)].zahl = -0.0;
                juce::MemoryBlock mb0;
                state::speichere (zm, mb0);
                {
                    const auto v = juce::ValueTree::readFromData (mb0.getData(), mb0.getSize());
                    const double geschrieben = (double) v.getChildWithName ("Parameters")
                                                        .getProperty ("v1.band.6.gain_db");
                    const double minusNull = -0.0;
                    pruefe (std::memcmp (&geschrieben, &minusNull, sizeof (double)) == 0,
                            "der Writer haelt das Vorzeichen der Null (var haelt 0.0 == -0.0)");
                }
                auto b2 = juce::ValueTree::readFromData (mb0.getData(), mb0.getSize());
                auto p2 = b2.getChildWithName ("Parameters");
                p2.removeProperty ("dsp_schema_version", nullptr);
                p2.removeProperty ("v2.global.eq_enabled", nullptr);
                p2.removeProperty ("v2.global.mix", nullptr);
                p2.removeProperty ("v2.global.auto_gain", nullptr);
                const auto mb = alsBlock (b2);
                state::Zustand z6;
                pruefe (state::lade (mb.getData(), mb.getSize(), state::Bundle::nkac(), z6)
                            == state::LadeErgebnis::geladen && z6.layoutV1Migriert
                            && z6.parameters[(size_t) param::indexOccupied (6)].b,
                        "minus null gilt bitgenau als Abweichung vom Default (kein Epsilon)", z6.grund);
            }

            // Speichern schreibt den Stand im Layout v2 - und ein zweiter
            // Roundtrip ist bytegleich (die Migration ist eine Einbahn, aber
            // deterministisch).
            juce::MemoryBlock nachher, nochmal;
            state::speichere (z1, nachher);
            state::Zustand z2;
            pruefe (state::lade (nachher.getData(), nachher.getSize(), state::Bundle::nkac(), z2)
                        == state::LadeErgebnis::geladen && ! z2.layoutV1Migriert,
                    "der gespeicherte Stand ist Layout v2 und migriert nicht erneut");
            state::speichere (z2, nochmal);
            pruefe (gleich (nachher, nochmal), "Save nach Load des migrierten Standes ist bytegleich");
            pruefe (z2.parameters[(size_t) param::indexOccupied (4)].b && z2.hatDsp,
                    "die migrierte Belegung landet im Kind Dsp und ueberlebt");
        }

        /*  M-91/M-92: die zwei Regeln des Kindes `Dsp` nebeneinander.

            Additiv ist der BAUM (eine unbekannte Eigenschaft in `Dsp`
            ueberlebt), exakt ist das DTO. Und `Dsp` bei einer anderen Klasse
            als `active_probe` ist ein Kind-Matrix-Verstoss - der ganze Stand
            wird read-only mit Originalbytes. */
        {
            state::Zustand zv = state::frisch (juce::String::repeatedString ("5", 32));
            zv.common.klasse = state::Klasse::active_probe;
            zv.hatParameters = true;
            zv.stateRevision = 3;
            zv.parameters[(size_t) param::indexOccupied (0)].b = true;
            juce::MemoryBlock bytes;
            state::speichere (zv, bytes);

            auto baum = juce::ValueTree::readFromData (bytes.getData(), bytes.getSize());
            baum.getChildWithName ("Dsp").setProperty ("zukunft_feld", "bleibt", nullptr);
            const auto mitZukunft = alsBlock (baum);
            state::Zustand zz;
            pruefe (state::lade (mitZukunft.getData(), mitZukunft.getSize(), state::Bundle::nkac(), zz)
                        == state::LadeErgebnis::geladen,
                    "unbekannte Eigenschaft im Dsp-Kind macht den Stand NICHT read-only", zz.grund);
            zz.stateRevision = 4;
            juce::MemoryBlock zurueckBytes;
            state::speichere (zz, zurueckBytes);
            const auto zurueck = juce::ValueTree::readFromData (zurueckBytes.getData(), zurueckBytes.getSize());
            pruefe (zurueck.getChildWithName ("Dsp").getProperty ("zukunft_feld").toString() == "bleibt"
                        && (juce::int64) zurueck.getChildWithName ("Dsp").getProperty ("state_revision") == 4,
                    "sie ueberlebt den Roundtrip, und die echte Aenderung kommt an");

            // Dieselbe Eigenschaft im DTO waere dagegen ein Fehler - additiv
            // ist der Baum, exakt das DTO.
            {
                const juce::String dtoText =
                    "{\"dsp_schema_version\":2,\"parameters\":{},\"schutz_zonen\":[],\"zukunft\":1}";
                param::DspSatz s; juce::String g, d;
                pruefe (! param::ausDtoText (dtoText.toRawUTF8(), (size_t) dtoText.getNumBytesAsUTF8(), s, g, d)
                            && g == "struktur",
                        "ein unbekannter Wurzelschluessel im DTO faellt", g + " " + d);
            }

            // Kind-Matrix: `Dsp` bei legacy.
            {
                auto v = schema2Baum ("legacy", "insert", false);
                juce::ValueTree d ("Dsp");
                d.setProperty ("schema", 1, nullptr);
                d.setProperty ("state_revision", juce::var ((juce::int64) 1), nullptr);
                v.appendChild (d, nullptr);
                const auto b = alsBlock (v);
                state::Zustand zl;
                pruefe (state::lade (b.getData(), b.getSize(), state::Bundle::eqcp(), zl)
                            == state::LadeErgebnis::nurLesen && zl.nurLesen
                            && zl.originalBytes.getSize() == b.getSize(),
                        "Dsp bei legacy: read-only mit Originalbytes", zl.grund);
            }

            /*  Ein Layout-v1-Knoten MIT Dsp-Kind ist ein Widerspruch: ein
                Build, der das Kind nicht kannte, kann es nicht geschrieben
                haben. Read-only statt raten - sonst ueberschriebe die
                Migration die gelesene Belegung stumm. */
            {
                auto v = juce::ValueTree::readFromData (bytes.getData(), bytes.getSize());
                auto p2 = v.getChildWithName ("Parameters");
                p2.removeProperty ("dsp_schema_version", nullptr);
                p2.removeProperty ("v2.global.eq_enabled", nullptr);
                p2.removeProperty ("v2.global.mix", nullptr);
                p2.removeProperty ("v2.global.auto_gain", nullptr);
                const auto b = alsBlock (v);
                state::Zustand zw;
                pruefe (state::lade (b.getData(), b.getSize(), state::Bundle::nkac(), zw)
                            == state::LadeErgebnis::nurLesen && zw.nurLesen
                            && zw.originalBytes.getSize() == b.getSize(),
                        "Layout v1 MIT Dsp-Kind ist ein Widerspruch: read-only mit Originalbytes", zw.grund);
            }

            // Ein `Dsp` OHNE state_revision ist kein Teilstate.
            {
                auto v = juce::ValueTree::readFromData (bytes.getData(), bytes.getSize());
                v.getChildWithName ("Dsp").removeProperty ("state_revision", nullptr);
                const auto b = alsBlock (v);
                state::Zustand zo;
                pruefe (state::lade (b.getData(), b.getSize(), state::Bundle::nkac(), zo)
                            == state::LadeErgebnis::nurLesen && zo.nurLesen,
                        "Dsp ohne state_revision macht den Stand read-only", zo.grund);
            }

            // Ein unbekanntes Layout-Major in `Parameters` ebenfalls.
            {
                auto v = juce::ValueTree::readFromData (bytes.getData(), bytes.getSize());
                v.getChildWithName ("Parameters").setProperty ("dsp_schema_version", 3, nullptr);
                const auto b = alsBlock (v);
                state::Zustand zm;
                pruefe (state::lade (b.getData(), b.getSize(), state::Bundle::nkac(), zm)
                            == state::LadeErgebnis::nurLesen && zm.nurLesen
                            && zm.originalBytes.getSize() == b.getSize(),
                        "unbekanntes Layout-Major: read-only mit Originalbytes", zm.grund);
            }

            // Ein FRISCHER Stand traegt gar kein Dsp-Kind - "nichts zu sagen"
            // und "mit leeren Feldern gesagt" waeren in den Bytes sonst
            // dasselbe.
            {
                state::Zustand leer = state::frisch (juce::String::repeatedString ("4", 32));
                leer.common.klasse = state::Klasse::active_probe;
                leer.hatParameters = true;
                juce::MemoryBlock b;
                state::speichere (leer, b);
                const auto v = juce::ValueTree::readFromData (b.getData(), b.getSize());
                pruefe (! v.getChildWithName ("Dsp").isValid(),
                        "ein frischer active_probe-Stand schreibt kein Dsp-Kind");
            }
        }

        // Reine Funktion direkt: unbekanntes Rollenwort ist nicht migrierbar.
        {
            juce::ValueTree alt ("EqCopilotState");
            alt.setProperty ("schema", 1, nullptr);
            alt.setProperty ("sensor_id", "x", nullptr);
            alt.setProperty ("role", "dirigent", nullptr);
            juce::ValueTree neu; juce::String grund;
            pruefe (! state::migriereSchema1 (alt, neu, grund), "unbekannte Schema-1-Rolle ist nicht migrierbar", grund);
            alt.setProperty ("role", "sensor", nullptr);
            alt.setProperty ("schema", 2, nullptr);
            pruefe (! state::migriereSchema1 (alt, neu, grund), "EqCopilotState schema 2 ist kein Schema 1", grund);
        }
        a.schliesse ("Migration: 4 Rollen bytegleich zum Golden, idempotent");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G8 · Roundtrip Schema 2
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        auto z = state::frisch ("0123456789abcdef0123456789abcdef");
        z.common.klasse = state::Klasse::main;
        z.common.label = juce::String::fromUTF8 ("Leitstand \xc3\xa9 \xf0\x9d\x84\x9e");
        juce::MemoryBlock b1, b2;
        state::speichere (z, b1);
        state::Zustand z2;
        pruefe (state::lade (b1.getData(), b1.getSize(), state::Bundle::eqcp(), z2) == state::LadeErgebnis::geladen, "Schema 2 laedt");
        state::speichere (z2, b2);
        pruefe (gleich (b1, b2), "speichern->laden->speichern bytegleich", juce::String ((int) b1.getSize()) + " Bytes");
        pruefe (z2.common == z.common, "Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)");

        // Additive Eigenschaft eines spaeteren Minors ueberlebt.
        {
            juce::ValueTree v = juce::ValueTree::readFromData (b1.getData(), b1.getSize());
            v.getChildWithName ("Common").setProperty ("zukunft_feld", "bleibt", nullptr);
            v.getChildWithName ("MainProject").setProperty ("intent_version", 7, nullptr);
            const auto mitZukunft = alsBlock (v);
            state::Zustand z3;
            pruefe (state::lade (mitZukunft.getData(), mitZukunft.getSize(), state::Bundle::eqcp(), z3) == state::LadeErgebnis::geladen,
                    "Stand mit unbekannter additiver Eigenschaft laedt");
            z3.common.label = "umbenannt";   // eine echte Aenderung dazwischen
            juce::MemoryBlock b3;
            state::speichere (z3, b3);
            const auto zurueck = juce::ValueTree::readFromData (b3.getData(), b3.getSize());
            pruefe (zurueck.getChildWithName ("Common").getProperty ("zukunft_feld").toString() == "bleibt"
                    && (int) zurueck.getChildWithName ("MainProject").getProperty ("intent_version") == 7
                    && zurueck.getChildWithName ("Common").getProperty ("label").toString() == "umbenannt",
                    "unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an");
        }

        // Parameters + Dsp: 120 Werte bit-exakt (active_probe-Bundle).
        {
            // Jeder Gleitkomma-Parameter knapp unter seinem Maximum (ein double,
            // das keine kurze Dezimalform hat), dazu ein paar Schalter/Enums.
            auto s = param::standardSatz();
            for (int i = 0; i < param::kAnzahl; ++i)
                if (param::tabelle()[(size_t) i].typ == param::Typ::gleitkomma)
                    s[(size_t) i].zahl = std::nextafter (param::tabelle()[(size_t) i].max, 0.0);
            s[3].zahl = 0.30000000000000004;   // width: der klassische Nicht-Dezimalwert
            s[5].b = true; s[6].enumIndex = 4; s[17].enumIndex = 2;
            // SONDE-015: die drei v2-Host-Parameter und die acht occupied
            // reisen im selben Roundtrip - `mix` traegt einen Wert ohne kurze
            // Dezimalform, damit die bit-exakte Zusage etwas kostet.
            s[(size_t) param::kIndexEqEnabled].b = true;
            s[(size_t) param::kIndexAutoGain].b = true;
            s[(size_t) param::kIndexMix].zahl = 0.30000000000000004;
            for (int slot = 0; slot < param::kSlots; ++slot)
                s[(size_t) param::indexOccupied (slot)].b = (slot % 2) == 0;
            state::Zustand za = state::frisch ("ffffffffffffffffffffffffffffffff");
            za.common.klasse = state::Klasse::active_probe;
            za.hatParameters = true;
            za.parameters = s;
            za.stateRevision = 4711;
            za.schutzZonen = { { 0, 40.5, 120.25, true }, { 5, 900.0, 1100.0, false } };
            {
                state::UndoEintrag u;
                u.art = state::UndoArt::remove;
                u.slot = 3;
                u.revision = 4710;
                u.zustand.werte = s;
                u.zustand.zonen = za.schutzZonen;
                za.undoRing.push_back (u);
                u.art = state::UndoArt::gestus;
                u.slot = -1;
                u.revision = 4711;
                u.zustand.zonen.clear();
                za.undoRing.push_back (u);
            }
            za.undoCursor = 1;
            juce::MemoryBlock ba, bb;
            state::speichere (za, ba);
            state::Zustand zb;
            pruefe (state::lade (ba.getData(), ba.getSize(), state::Bundle::nkac(), zb) == state::LadeErgebnis::geladen, "active_probe mit Parameters und Dsp laedt (Bundle NkAc)");
            int bitExakt = 0;
            for (int i = 0; i < param::kAnzahl; ++i)
            {
                const auto& x = s[(size_t) i]; const auto& y = zb.parameters[(size_t) i];
                switch (param::tabelle()[(size_t) i].typ)
                {
                    case param::Typ::boolean:     if (x.b == y.b) ++bitExakt; break;
                    case param::Typ::gleitkomma:  if (std::memcmp (&x.zahl, &y.zahl, sizeof (double)) == 0) ++bitExakt; break;
                    case param::Typ::aufzaehlung: if (x.enumIndex == y.enumIndex) ++bitExakt; break;
                }
            }
            pruefe (bitExakt == param::kAnzahl, "120 Parameterwerte bit-exakt durch den Roundtrip", juce::String (bitExakt));
            pruefe (zb.hatDsp && zb.stateRevision == 4711 && zb.schutzZonen == za.schutzZonen
                    && zb.undoRing == za.undoRing && zb.undoCursor == 1 && ! zb.layoutV1Migriert,
                    "Dsp-Kind feldgleich nach dem Roundtrip: Revision, Zonen, Undo-Ring, Cursor");
            state::speichere (zb, bb);
            pruefe (gleich (ba, bb), "Parameters- und Dsp-Roundtrip bytegleich");
            juce::String h1, h2, g;
            pruefe (param::stateHash (za.dspDto(), h1, g) && param::stateHash (zb.dspDto(), h2, g) && h1 == h2, "state_hash ueberlebt den Roundtrip", h1);

            // M-89, gemessen statt behauptet: der Baum bleibt weit unter der
            // 64-Knoten-Grenze des Byte-Riegels, weil `Dsp` flache bzw.
            // verschachtelte Arrays fuehrt und keine Kindknoten.
            {
                const auto v = juce::ValueTree::readFromData (ba.getData(), ba.getSize());
                std::function<int (const juce::ValueTree&)> knoten = [&] (const juce::ValueTree& k)
                {
                    int n = 1;
                    for (int i = 0; i < k.getNumChildren(); ++i) n += knoten (k.getChild (i));
                    return n;
                };
                pruefe (v.isValid() && knoten (v) <= 8, "Dsp nutzt flache Arrays: der Baum bleibt bei wenigen Knoten",
                        juce::String (v.isValid() ? knoten (v) : -1));
            }
        }

        /*  M-24 (Vertragshaelfte) und M-28 (Enumgrenze): die getrennten
            Dynamic-Parameter und der Kanalmodus.

            Ausschalten aendert AUSSCHLIESSLICH `dynamic_enabled`. Die fuenf
            Werte `dynamic_range_db`, `threshold_db`, `attack_ms`, `hold_ms`
            und `release_ms` bleiben BITGLEICH erhalten und stehen beim
            Wiedereinschalten unveraendert bereit - ein Writer, der sie
            "vorsorglich" zuruecksetzt, verliert Userarbeit, die niemand
            geloescht hat. Gemessen ueber den echten Save/Load-Weg, nicht an
            einem In-Memory-Satz: genau dort koennte der Verlust passieren. */
        {
            constexpr int slot = 4;
            const int iDyn   = param::indexBandV1 (slot, param::kDynamicEnabled);
            const int iRange = param::indexBandV1 (slot, param::kDynamicRangeDb);
            const int iThr   = param::indexBandV1 (slot, param::kThresholdDb);
            const int iAtt   = param::indexBandV1 (slot, param::kAttackMs);
            const int iHold  = param::indexBandV1 (slot, param::kHoldMs);
            const int iRel   = param::indexBandV1 (slot, param::kReleaseMs);
            const int iMode  = param::indexBandV1 (slot, param::kChannelMode);
            const int iTyp   = param::indexBandV1 (slot, param::kType);

            state::Zustand z = state::frisch (juce::String::repeatedString ("2", 32));
            z.common.klasse = state::Klasse::active_probe;
            z.hatParameters = true;
            z.parameters[(size_t) param::indexOccupied (slot)].b = true;
            z.parameters[(size_t) iDyn].b = true;
            z.parameters[(size_t) iRange].zahl = -6.25;
            z.parameters[(size_t) iThr].zahl = -18.3;
            z.parameters[(size_t) iAtt].zahl = 0.30000000000000004;
            z.parameters[(size_t) iHold].zahl = 12.5;
            z.parameters[(size_t) iRel].zahl = 4999.9;
            z.parameters[(size_t) iMode].enumIndex = 4;   // side
            const auto vorher = z.parameters;

            auto durchDenState = [&] (const state::Zustand& ein, state::Zustand& aus)
            {
                juce::MemoryBlock b;
                state::speichere (ein, b);
                return state::lade (b.getData(), b.getSize(), state::Bundle::nkac(), aus)
                           == state::LadeErgebnis::geladen;
            };

            state::Zustand an;
            pruefe (durchDenState (z, an), "Slot mit eingeschaltetem Dynamic laedt");

            // AUS - und nur das.
            auto ausSatz = an;
            ausSatz.parameters[(size_t) iDyn].b = false;
            state::Zustand aus;
            pruefe (durchDenState (ausSatz, aus), "Slot mit ausgeschaltetem Dynamic laedt");

            auto bitgleich = [] (const param::Satz& a, const param::Satz& b, int i)
            {
                return std::memcmp (&a[(size_t) i].zahl, &b[(size_t) i].zahl, sizeof (double)) == 0;
            };
            const bool fuenfHalten = bitgleich (vorher, aus.parameters, iRange)
                                  && bitgleich (vorher, aus.parameters, iThr)
                                  && bitgleich (vorher, aus.parameters, iAtt)
                                  && bitgleich (vorher, aus.parameters, iHold)
                                  && bitgleich (vorher, aus.parameters, iRel);
            pruefe (fuenfHalten && ! aus.parameters[(size_t) iDyn].b,
                    "dynamicwerte_ueberleben_aus_und_ein_bitgleich: Ausschalten aendert nur dynamic_enabled");

            // WIEDER AN - dieselben Werte stehen unveraendert bereit.
            auto wiederAn = aus;
            wiederAn.parameters[(size_t) iDyn].b = true;
            state::Zustand zurueck;
            pruefe (durchDenState (wiederAn, zurueck), "Slot mit wieder eingeschaltetem Dynamic laedt");
            const bool wieVorher = bitgleich (vorher, zurueck.parameters, iRange)
                                && bitgleich (vorher, zurueck.parameters, iThr)
                                && bitgleich (vorher, zurueck.parameters, iAtt)
                                && bitgleich (vorher, zurueck.parameters, iHold)
                                && bitgleich (vorher, zurueck.parameters, iRel);
            pruefe (wieVorher && zurueck.parameters[(size_t) iDyn].b,
                    "beim Wiedereinschalten stehen die fuenf Werte unveraendert bereit");

            // M-28: `channel_mode` ist nie mit Typ oder Dynamic gekoppelt, und
            // kein anderer Slot erbt ihn.
            pruefe (zurueck.parameters[(size_t) iMode].enumIndex == 4
                        && zurueck.parameters[(size_t) iTyp].enumIndex == 0,
                    "channel_mode_ist_diskret_und_ungekoppelt: der Typ bleibt bei bell");
            int fremde = 0;
            for (int s = 0; s < param::kSlots; ++s)
                if (s != slot && zurueck.parameters[(size_t) param::indexBandV1 (s, param::kChannelMode)].enumIndex != 0)
                    ++fremde;
            pruefe (fremde == 0, "kein_slot_erbt_den_modus", juce::String (fremde));
        }

        /*  M-78 (Vertragsgrenze): der VOLLE Undo-Ring - 32 Schnappschuesse,
            acht Zonen, alle acht Slots belegt - wird geschrieben, gelesen und
            bytegleich zurueckgeschrieben, und der Stand bleibt weit unter der
            16-MiB-Grenze des State-Lesers. Gemessen, nicht geschaetzt. */
        {
            state::Zustand zv = state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
            zv.common.klasse = state::Klasse::active_probe;
            zv.hatParameters = true;
            zv.stateRevision = 100000;
            for (int slot = 0; slot < param::kSlots; ++slot)
                zv.parameters[(size_t) param::indexOccupied (slot)].b = true;
            for (int i = 0; i < param::kMaxZonen; ++i)
                zv.schutzZonen.push_back ({ i, 20.0 + (double) i * 10.0, 5000.0 + (double) i, i % 2 == 0 });
            for (int i = 0; i < param::kUndoTiefe; ++i)
            {
                state::UndoEintrag u;
                u.art = state::UndoArt::presetLaden;
                u.slot = i % param::kSlots;
                u.revision = 100000 - i;
                u.zustand.werte = zv.parameters;
                u.zustand.zonen = zv.schutzZonen;
                zv.undoRing.push_back (std::move (u));
            }
            zv.undoCursor = param::kUndoTiefe;
            juce::MemoryBlock bv, bv2;
            state::speichere (zv, bv);
            state::Zustand zl;
            const auto erg = state::lade (bv.getData(), bv.getSize(), state::Bundle::nkac(), zl);
            state::speichere (zl, bv2);
            pruefe (erg == state::LadeErgebnis::geladen && gleich (bv, bv2)
                    && (int) zl.undoRing.size() == param::kUndoTiefe
                    && (int) zl.schutzZonen.size() == param::kMaxZonen
                    && zl.undoCursor == param::kUndoTiefe,
                    "voller Undo-Ring (32 Eintraege, 8 Zonen) laedt und ist bytegleich",
                    juce::String ((int) bv.getSize()) + " Bytes");
            pruefe (bv.getSize() < 16u * 1024u * 1024u,
                    "voller Dsp-Stand bleibt weit unter der 16-MiB-Grenze",
                    juce::String ((int) bv.getSize()) + " Bytes");
            // Der 33. Eintrag ist kein Teilbestand, sondern ein Fehler: der
            // Ring waechst nie ueber seine Vertragsgrenze.
            {
                auto v = juce::ValueTree::readFromData (bv.getData(), bv.getSize());
                auto dsp = v.getChildWithName ("Dsp");
                auto ring = dsp.getProperty ("undo_ring_v1");
                auto kopie = *ring.getArray();
                kopie.add (kopie.getReference (0));
                dsp.setProperty ("undo_ring_v1", juce::var (kopie), nullptr);
                const auto zuViel = alsBlock (v);
                state::Zustand zx;
                pruefe (state::lade (zuViel.getData(), zuViel.getSize(), state::Bundle::nkac(), zx) == state::LadeErgebnis::nurLesen
                        && zx.nurLesen,
                        "33. Undo-Eintrag macht den Stand read-only", zx.grund);
            }
        }
        a.schliesse ("Roundtrip: bytegleich, additive Eigenschaft erhalten, 120 Werte bit-exakt, voller Undo-Ring");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G8b · Messposition je Klasse — die GANZE Matrix, in ihrem eigenen Bundle
    //
    // G1-Nacharbeit 24.08.2026 zu Gate-Befund §4.2. Bis dahin gab es fuer diese
    // Matrix genau EINEN Fall (`legacy mit post_fader_contribution`, G9), und
    // der lief ueber das Eqcp-Bundle. Die Luecke, die der Gate-Lauf gefahren
    // hat, lag bei `passive_probe` — einer Klasse, die im Eqcp-Bundle schon aus
    // einem ANDEREN Grund read-only wird (fremdes Bundle). Ein Fall, der aus
    // dem falschen Grund gruen ist, deckt nichts.
    //
    // 🔑 Deshalb faehrt dieser Block jede Klasse in dem Bundle, das sie
    // ueberhaupt zulaesst — sonst misst der Bundlevertrag und nicht die
    // Positionsmatrix. Ohne den Fix in `positionErlaubt` ist die Zeile
    // `passive_probe | post_fader_contribution | false` rot: sie laedt.
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        struct Zeile { state::Klasse k; const char* position; bool erlaubt; };
        // Wortlaut: schemas/state/nakama-state-v2.md §2.2 (Fassung 24.08.).
        const Zeile matrix[] = {
            { state::Klasse::main,          "insert",                  true  },
            { state::Klasse::main,          "pre",                     false },
            { state::Klasse::main,          "post",                    false },
            { state::Klasse::main,          "post_fader_contribution", false },
            { state::Klasse::legacy,        "insert",                  true  },
            { state::Klasse::legacy,        "pre",                     true  },
            { state::Klasse::legacy,        "post",                    true  },
            { state::Klasse::legacy,        "post_fader_contribution", false },
            { state::Klasse::passive_probe, "insert",                  true  },
            { state::Klasse::passive_probe, "pre",                     true  },
            { state::Klasse::passive_probe, "post",                    true  },
            { state::Klasse::passive_probe, "post_fader_contribution", false },
            { state::Klasse::active_probe,  "insert",                  true  },
            { state::Klasse::active_probe,  "pre",                     true  },
            { state::Klasse::active_probe,  "post",                    true  },
            { state::Klasse::active_probe,  "post_fader_contribution", false },
        };

        int ok = 0, geprueft = 0;
        for (const auto& z : matrix)
        {
            // Jede Klasse in IHREM Bundle - sonst antwortet der Bundlevertrag.
            const auto bundle = (z.k == state::Klasse::passive_probe) ? state::Bundle::nkpr()
                              : (z.k == state::Klasse::active_probe)  ? state::Bundle::nkac()
                                                                      : state::Bundle::eqcp();

            auto baum = schema2Baum (state::wort (z.k), z.position,
                                     z.k == state::Klasse::main);
            // Kind-Matrix §2.1: active_probe verlangt GENAU EIN Parameters-Kind.
            // Ohne das waere ein Fall aus dem falschen Grund read-only.
            if (z.k == state::Klasse::active_probe)
                baum.appendChild (parametersKind (param::standardSatz()), nullptr);

            const auto roh = alsBlock (baum);
            state::Zustand zurueck;
            const auto erg = state::lade (roh.getData(), roh.getSize(), bundle, zurueck);
            const auto erwartet = z.erlaubt ? state::LadeErgebnis::geladen
                                            : state::LadeErgebnis::nurLesen;

            bool zustandOk = ! zurueck.nurLesen;
            bool rueckwegOk = true;
            bool hostReadOnlyOk = true;
            bool hostDirtyOk = true;
            juce::MemoryBlock rueckweg;
            if (! z.erlaubt)
            {
                zustandOk = zurueck.nurLesen && zurueck.grund.isNotEmpty();
                state::speichere (zurueck, rueckweg);
                rueckwegOk = gleich (rueckweg, roh);

                auto p = std::make_unique<EqCopilotProcessor>();
                DirtyZaehler dirty;
                p->addListener (&dirty);
                p->setStateInformation (roh.getData(), (int) roh.getSize());
                hostReadOnlyOk = p->stateNurLesen();
                hostDirtyOk = dirty.nonParam == 0;
                p->removeListener (&dirty);
            }

            const bool fallOk = erg == erwartet && zustandOk && rueckwegOk
                                && hostReadOnlyOk && hostDirtyOk;
            ++geprueft;
            if (fallOk)
                ++ok;
            pruefe (fallOk,
                    juce::String ("Matrix Klasse=") + state::wort (z.k) + ", Position=" + z.position
                        + ": erwartet LadeErgebnis::" + ladeErgebnisWort (erwartet)
                        + ", tatsaechlich LadeErgebnis::" + ladeErgebnisWort (erg),
                    juce::String ("nurLesen=") + (zurueck.nurLesen ? "true" : "false")
                        + ", Grund='" + zurueck.grund + "', Rueckweg="
                        + (z.erlaubt ? "nicht gefordert" : (rueckwegOk ? "bytegleich" : "abweichend"))
                        + ", Host-read-only="
                        + (z.erlaubt ? "nicht gefordert" : (hostReadOnlyOk ? "true" : "false"))
                        + ", Host-Dirty="
                        + (z.erlaubt ? "nicht gefordert" : (hostDirtyOk ? "nein" : "JA")));
        }
        pruefe (ok == geprueft,
                juce::String (geprueft) + " Kombinationen aus Klasse x Messposition wie §2.2 - "
                "post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)",
                juce::String (ok));
        pruefe (! state::positionErlaubt (state::Klasse::main, state::Messposition::post_fader_contribution)
                    && ! state::positionErlaubt (state::Klasse::passive_probe, state::Messposition::post_fader_contribution)
                    && ! state::positionErlaubt (state::Klasse::active_probe, state::Messposition::post_fader_contribution)
                    && ! state::positionErlaubt (state::Klasse::legacy, state::Messposition::post_fader_contribution),
                "post_fader_contribution_bleibt_fuer_alle_sondenklassen_gesperrt");
        a.schliesse ("Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G8c · NAK-40: persistente Original-ID ↔ abgeleitete v3-Wireadresse
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        bool fixtureOk = false;
        const auto aliasFixture = ladeJson (
            finde ("eq-copilot/fixtures/v3/instance-address-alias-v1.json"), fixtureOk);
        int vektorenOk = 0;
        if (fixtureOk)
            if (const auto* vektoren = aliasFixture["vectors"].getArray())
                for (const auto& v : *vektoren)
                {
                    const auto original = v["instance_id"].toString().toStdString();
                    const auto erwartet = v["wire_instance_id"].toString().toStdString();
                    if (nakama::ipc::instanceAdresseAusState (original) == erwartet)
                        ++vektorenOk;
                }
        pruefe (fixtureOk && vektorenOk == 6,
                "instance_address_alias_vectors", juce::String (vektorenOk) + "/6");

        const std::string legacy = "11111111-2222-3333-4444-555555555555";
        const std::string wire = "239953e95f122e18abb34ba1add46f9e";
        pruefe (nakama::ipc::instanceAliasZielPasst (legacy, wire),
                "instance_alias_target_recomputed_and_matched");
        pruefe (! nakama::ipc::instanceAliasZielPasst (legacy,
                    "ffffffffffffffffffffffffffffffff")
                    && ! nakama::ipc::instanceAliasZielPasst ("", wire),
                "instance_alias_unknown_fail_closed");

        auto leererAltstand = saatSchema1 ("sensor", "", "");
        {
            auto baum = juce::ValueTree::readFromData (
                leererAltstand.getData(), leererAltstand.getSize());
            baum.removeProperty ("sensor_id", nullptr);
            leererAltstand = alsBlock (baum);
        }
        state::Zustand frischAusLeer;
        const auto leerErgebnis = state::lade (leererAltstand.getData(), leererAltstand.getSize(),
                                               state::Bundle::eqcp(), frischAusLeer);
        const auto frischeId = frischAusLeer.common.instanceId.toStdString();
        pruefe (nakama::ipc::instanceAdresseAusState ("").empty()
                    && leerErgebnis == state::LadeErgebnis::migriert
                    && nakama::ipc::istHex32 (frischeId),
                "instance_address_empty_uses_fresh_uuid", frischAusLeer.common.instanceId);

        auto baum = schema2Baum ("legacy", "insert", false);
        baum.getChildWithName ("Common").setProperty (
            "instance_id", juce::String::fromUTF8 (legacy.c_str()), nullptr);
        const auto originalBytes = alsBlock (baum);
        state::Zustand geladen, erneut;
        juce::MemoryBlock gespeichert, erneutGespeichert;
        const auto geladenErgebnis = state::lade (originalBytes.getData(), originalBytes.getSize(),
                                                  state::Bundle::eqcp(), geladen);
        const auto adressiert = nakama::ipc::instanceAdresseAusState (
            geladen.common.instanceId.toStdString());
        state::speichere (geladen, gespeichert);
        const auto erneutErgebnis = state::lade (gespeichert.getData(), gespeichert.getSize(),
                                                 state::Bundle::eqcp(), erneut);
        state::speichere (erneut, erneutGespeichert);
        pruefe (geladenErgebnis == state::LadeErgebnis::geladen
                    && erneutErgebnis == state::LadeErgebnis::geladen
                    && adressiert == wire
                    && geladen.common.instanceId.toStdString() == legacy
                    && erneut.common.instanceId.toStdString() == legacy
                    && gleich (originalBytes, gespeichert)
                    && gleich (gespeichert, erneutGespeichert),
                "legacy_instance_id_save_load_bytegleich");
        a.schliesse ("NAK-40 Aliasvektoren, Zielvergleich und Save+Load");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G9 · Unbekanntes Major / unzulaessige Kombination → read-only
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        struct Fall { juce::String name; juce::ValueTree baum; };
        std::vector<Fall> faelle;

        { auto v = schema2Baum ("legacy", "insert", false); v.setProperty ("schema", 3, nullptr); faelle.push_back ({ "Root schema 3", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); v.getChildWithName ("Common").setProperty ("schema", 2, nullptr); faelle.push_back ({ "Common schema 2", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); v.appendChild (parametersKind (param::standardSatz()), nullptr); faelle.push_back ({ "Parameters in legacy", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); juce::ValueTree d ("Dsp"); d.setProperty ("schema", 1, nullptr); v.appendChild (d, nullptr); faelle.push_back ({ "Dsp vorhanden", v }); }
        { auto v = schema2Baum ("main", "insert", true); juce::ValueTree p ("Pairing"); p.setProperty ("schema", 1, nullptr); v.appendChild (p, nullptr); faelle.push_back ({ "Pairing vorhanden", v }); }
        { auto v = schema2Baum ("active_probe", "insert", false); v.appendChild (parametersKind (param::standardSatz()), nullptr); faelle.push_back ({ "active_probe im Eqcp-Bundle", v }); }
        { auto v = schema2Baum ("passive_probe", "insert", false); faelle.push_back ({ "passive_probe im Eqcp-Bundle", v }); }
        { juce::ValueTree v ("EqCopilotState"); v.setProperty ("schema", 2, nullptr); v.setProperty ("sensor_id", "x", nullptr); v.setProperty ("role", "sensor", nullptr); faelle.push_back ({ "EqCopilotState schema 2", v }); }
        { juce::ValueTree v ("EqCopilotState"); v.setProperty ("schema", 1, nullptr); v.setProperty ("sensor_id", "x", nullptr); v.setProperty ("role", "dirigent", nullptr); faelle.push_back ({ "Schema-1-Rolle unbekannt", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); juce::ValueTree z ("Zukunft"); v.appendChild (z, nullptr); faelle.push_back ({ "unbekanntes Kind", v }); }
        { auto v = schema2Baum ("main", "insert", false); faelle.push_back ({ "main ohne MainProject", v }); }
        { auto v = schema2Baum ("legacy", "insert", true); faelle.push_back ({ "legacy mit MainProject", v }); }
        { auto v = schema2Baum ("main", "pre", true); faelle.push_back ({ "main mit Messposition pre", v }); }
        { auto v = schema2Baum ("legacy", "post_fader_contribution", false); faelle.push_back ({ "legacy mit post_fader_contribution", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); v.getChildWithName ("Common").removeProperty ("instance_id", nullptr); faelle.push_back ({ "Common ohne instance_id", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); v.getChildWithName ("Common").setProperty ("plugin_kind", "hub", nullptr); faelle.push_back ({ "plugin_kind unbekanntes Wort", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); v.appendChild (schema2Baum ("legacy", "insert", false).getChildWithName ("Common").createCopy(), nullptr); faelle.push_back ({ "Common doppelt", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); v.getChildWithName ("Common").setProperty ("pair_id", "", nullptr); faelle.push_back ({ "pair_id vorhanden aber leer", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); v.getChildWithName ("Common").setProperty ("project_binding_id", "", nullptr); faelle.push_back ({ "project_binding_id vorhanden aber leer", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); v.getChildWithName ("Common").setProperty ("project_binding_id", "abc", nullptr); faelle.push_back ({ "project_binding_id kein hex32", v }); }
        { auto v = schema2Baum ("legacy", "insert", false); v.getChildWithName ("Common").setProperty ("project_binding_id", "ABCDEF0123456789ABCDEF0123456789", nullptr); faelle.push_back ({ "project_binding_id Grossbuchstaben", v }); }

        int readOnly = 0;
        for (const auto& f : faelle)
        {
            const auto bytes = alsBlock (f.baum);

            // Bibliothek
            state::Zustand z;
            const auto erg = state::lade (bytes.getData(), bytes.getSize(), state::Bundle::eqcp(), z);
            juce::MemoryBlock zurueck;
            state::speichere (z, zurueck);
            const bool bibOk = erg == state::LadeErgebnis::nurLesen && z.nurLesen && gleich (zurueck, bytes) && z.grund.isNotEmpty();

            // Produkt: Prozessor wird read-only, gibt Originalbytes zurueck, verweigert Aenderungen ohne Host-Dirty.
            auto p = std::make_unique<EqCopilotProcessor>();
            DirtyZaehler dirty;
            p->addListener (&dirty);
            p->setStateInformation (bytes.getData(), (int) bytes.getSize());
            juce::MemoryBlock heraus;
            p->getStateInformation (heraus);
            const bool verweigert = ! p->setzeBindung ("hub", "Kaperung", "") && ! p->neueSensorId();
            const bool prozOk = p->stateNurLesen() && p->holeStateHerkunft() == state::Herkunft::nurLesen
                                && gleich (heraus, bytes) && verweigert && dirty.nonParam == 0;
            p->removeListener (&dirty);

            if (bibOk && prozOk) ++readOnly;
            else pruefe (false, "read-only-Fall: " + f.name,
                         juce::String ("bib ") + (bibOk ? "ok" : "FALSCH") + " proz " + (prozOk ? "ok" : "FALSCH") + " grund '" + z.grund + "'");
        }
        pruefe (readOnly == (int) faelle.size(), juce::String (faelle.size()) + " Faelle read-only, Bytes unveraendert, Aenderungen verweigert", juce::String (readOnly));

        // Fremder Baumtyp und Muell: Zustand bleibt (heutiges Verhalten).
        {
            state::Zustand z = state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
            juce::ValueTree fremd ("IrgendeinAnderesPlugin");
            fremd.setProperty ("sensor_id", "gekapert", nullptr);
            const auto fb = alsBlock (fremd);
            pruefe (state::lade (fb.getData(), fb.getSize(), state::Bundle::eqcp(), z) == state::LadeErgebnis::ignoriert
                    && z.common.instanceId == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", "fremder Baumtyp wird ignoriert, Zustand bleibt");
            const char muell[] = { 'n', 'i', 'c', 'h', 't', 's' };
            pruefe (state::lade (muell, sizeof (muell), state::Bundle::eqcp(), z) == state::LadeErgebnis::ignoriert, "Muellbytes werden ignoriert");

            auto kaputtesUtf8 = alsBlock (schema2Baum ("legacy", "insert", false));
            auto* kaputteBytes = static_cast<std::uint8_t*> (kaputtesUtf8.getData());
            bool labelGefunden = false;
            for (size_t i = 0; i + 5u <= kaputtesUtf8.getSize(); ++i)
            {
                if (std::memcmp (kaputteBytes + i, "Probe", 5) == 0)
                {
                    kaputteBytes[i] = 0xc3;
                    kaputteBytes[i + 1] = 0x28; // kein 10xxxxxx-Fortsetzungsbyte
                    labelGefunden = true;
                    break;
                }
            }
            pruefe (labelGefunden
                    && state::lade (kaputtesUtf8.getData(), kaputtesUtf8.getSize(), state::Bundle::eqcp(), z)
                        == state::LadeErgebnis::ignoriert,
                    "ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser");

            auto gueltig = alsBlock (schema2Baum ("legacy", "insert", false));
            const std::uint8_t suffix = 0x7f;
            gueltig.append (&suffix, 1);
            pruefe (state::lade (gueltig.getData(), gueltig.getSize(), state::Bundle::eqcp(), z) == state::LadeErgebnis::ignoriert
                    && z.common.instanceId == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                    "gueltiger Praefix mit Suffix wird vollstaendig ignoriert");

            auto abgeschnitten = alsBlock (schema2Baum ("main", "insert", true));
            abgeschnitten.setSize (abgeschnitten.getSize() - 1, false);
            pruefe (state::lade (abgeschnitten.getData(), abgeschnitten.getSize(), state::Bundle::eqcp(), z) == state::LadeErgebnis::ignoriert
                    && z.common.instanceId == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                    "abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen");

            const auto riesig = riesenVarianteOhneNutzdaten();
            pruefe (state::lade (riesig.getData(), riesig.getSize(), state::Bundle::eqcp(), z) == state::LadeErgebnis::ignoriert
                    && z.common.instanceId == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                    "deklarierte Riesenvariante wird vor JUCE-Allokation verworfen");

            const auto tief = zuTieferBaum (64);
            pruefe (state::lade (tief.getData(), tief.getSize(), state::Bundle::eqcp(), z) == state::LadeErgebnis::ignoriert
                    && z.common.instanceId == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                    "zu tiefer ValueTree wird begrenzt und ignoriert");

            const auto arrayTief = variantenArray (64);
            pruefe (state::lade (arrayTief.getData(), arrayTief.getSize(), state::Bundle::eqcp(), z) == state::LadeErgebnis::ignoriert
                    && z.common.instanceId == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                    "zu tief geschachteltes Variantenarray wird begrenzt und ignoriert");

            const std::uint8_t winzig = 0;
            pruefe (state::lade (&winzig, 16u * 1024u * 1024u + 1u, state::Bundle::eqcp(), z) == state::LadeErgebnis::ignoriert
                    && z.common.instanceId == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                    "State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen");
        }

        // Exakte Inklusiv-/Exklusivkanten der neuen Bytegrenzen. Die
        // Grenzfaelle muessen den Byte-Riegel wirklich passieren; ihr spaeteres
        // semantisches Urteil ist getrennt sichtbar.
        {
            const auto baumGrenze = zuTieferBaum (63);
            state::Zustand z;
            pruefe (state::lade (baumGrenze.getData(), baumGrenze.getSize(), state::Bundle::eqcp(), z)
                        == state::LadeErgebnis::nurLesen,
                    "ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel");

            const auto arrayGrenze = variantenArray (63);
            pruefe (state::lade (arrayGrenze.getData(), arrayGrenze.getSize(), state::Bundle::eqcp(), z)
                        == state::LadeErgebnis::geladen,
                    "63 verschachtelte Arrays plus Blatt passieren und laden");

            const auto eintraegeGrenze = baumMitEigenschaftszahl (65536);
            pruefe (state::lade (eintraegeGrenze.getData(), eintraegeGrenze.getSize(), state::Bundle::eqcp(), z)
                        == state::LadeErgebnis::nurLesen,
                    "65.536 Eintraege in einer Sammlung passieren den Byte-Riegel");
            const auto eintragZuViel = baumMitEigenschaftszahl (65537);
            pruefe (state::lade (eintragZuViel.getData(), eintragZuViel.getSize(), state::Bundle::eqcp(), z)
                        == state::LadeErgebnis::ignoriert,
                    "65.537 Eintraege in einer Sammlung werden verworfen");

            const auto gesamtGrenze = baumMitGesamteintraegen (262144);
            pruefe (state::lade (gesamtGrenze.getData(), gesamtGrenze.getSize(), state::Bundle::eqcp(), z)
                        == state::LadeErgebnis::nurLesen,
                    "262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel");
            const auto gesamtZuViel = baumMitGesamteintraegen (262145);
            pruefe (state::lade (gesamtZuViel.getData(), gesamtZuViel.getSize(), state::Bundle::eqcp(), z)
                        == state::LadeErgebnis::ignoriert,
                    "262.145 Eintraege ueber mehrere Sammlungen werden verworfen");
        }

        // Ein schreibbarer Input braucht bis zur absoluten 16-MiB-Grenze genau
        // den Headroom, den der groesste heute erreichbare Folgezustand braucht.
        // Das wird dynamisch am erhaltenen additiven Baum gemessen: eine feste
        // Schwelle waere nach dem ersten Save verbraucht und wuerde das eigene
        // Ergebnis beim naechsten Load faelschlich read-only machen.
        {
            constexpr size_t maxState = 16u * 1024u * 1024u;
            auto istSchreibbar = [] (size_t ziel)
            {
                const auto kandidat = stateMitZielgroesse (ziel);
                state::Zustand probe;
                return kandidat.getSize() == ziel
                    && state::lade (kandidat.getData(), kandidat.getSize(),
                                    state::Bundle::eqcp(), probe)
                        == state::LadeErgebnis::geladen;
            };
            size_t unten = 1024u;
            size_t oben = maxState;
            while (unten < oben)
            {
                const auto mitte = unten + (oben - unten + 1u) / 2u;
                if (istSchreibbar (mitte))
                    unten = mitte;
                else
                    oben = mitte - 1u;
            }
            const auto maxSchreibbarerInput = unten;
            const auto rand = stateMitZielgroesse (maxSchreibbarerInput);
            pruefe (rand.getSize() == maxSchreibbarerInput,
                    "Teststate trifft die schreibbare Bytegrenze exakt",
                    juce::String (static_cast<juce::int64> (rand.getSize())));

            state::Zustand z;
            const auto erg = state::lade (rand.getData(), rand.getSize(), state::Bundle::eqcp(), z);
            juce::String langesLabel, langesPaar;
            const auto vierByte = juce::String::charToString (
                static_cast<juce::juce_wchar> (0x10ffff));
            for (int i = 0; i < 120; ++i) langesLabel += vierByte;
            for (int i = 0; i < 60; ++i) langesPaar += vierByte;
            z.common.klasse = state::Klasse::main;
            z.common.position = state::Messposition::insert;
            z.common.label = langesLabel;
            z.common.pairId = langesPaar;
            z.mainProjectMitglieder.clear();
            for (int i = 0; i < state::maxMainProjectMitglieder; ++i)
                z.mainProjectMitglieder.push_back ({
                    juce::String::toHexString (i + 1).paddedLeft ('0', 32), langesLabel
                });
            juce::MemoryBlock geschrieben;
            state::speichere (z, geschrieben);
            state::Zustand erneut;
            pruefe (erg == state::LadeErgebnis::geladen
                    && geschrieben.getSize() <= maxState
                    && state::lade (geschrieben.getData(), geschrieben.getSize(), state::Bundle::eqcp(), erneut)
                        == state::LadeErgebnis::geladen,
                    "Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze",
                    juce::String (static_cast<juce::int64> (geschrieben.getSize())));

            const auto ohneReserve = stateMitZielgroesse (maxState - 64u);
            state::Zustand gehalten;
            const auto gehaltenErg = state::lade (
                ohneReserve.getData(), ohneReserve.getSize(), state::Bundle::eqcp(), gehalten);
            juce::MemoryBlock wieder;
            state::speichere (gehalten, wieder);
            pruefe (ohneReserve.getSize() == maxState - 64u
                    && gehaltenErg == state::LadeErgebnis::nurLesen
                    && gleich (ohneReserve, wieder),
                    "State ohne konkreten Writer-Headroom bleibt read-only bytegleich");

            const auto exaktMax = stateMitZielgroesse (maxState);
            state::Zustand exaktGehalten;
            const auto exaktErg = state::lade (
                exaktMax.getData(), exaktMax.getSize(), state::Bundle::eqcp(), exaktGehalten);
            juce::MemoryBlock exaktWieder;
            state::speichere (exaktGehalten, exaktWieder);
            pruefe (exaktMax.getSize() == maxState
                    && exaktErg == state::LadeErgebnis::nurLesen
                    && gleich (exaktMax, exaktWieder),
                    "vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich");
        }

        // Marker 9 (`undefined`) und zukuenftige Marker kann JUCE 8 zwar
        // ueberspringen, liest sie aber als void und wuerde sie beim Save
        // veraendern. Ein bekannter State bleibt deshalb read-only bytegleich.
        for (const auto marker : { std::uint8_t { 9 }, std::uint8_t { 10 } })
        {
            const auto roh = baumMitRohVariante (markerVariante (marker));
            state::Zustand z = state::frisch ("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
            const auto erg = state::lade (roh.getData(), roh.getSize(), state::Bundle::eqcp(), z);
            juce::MemoryBlock wieder;
            state::speichere (z, wieder);
            pruefe (erg == state::LadeErgebnis::nurLesen && z.nurLesen && gleich (roh, wieder),
                    "nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich",
                    juce::String (static_cast<int> (marker)));
        }

        // Additive Properties duerfen weiterhin alle von JUCE serialisierten
        // Variantentypen tragen; der Byte-Riegel ist kein neues Produktschema.
        {
            auto v = schema2Baum ("legacy", "insert", false);
            auto common = v.getChildWithName ("Common");
            juce::MemoryBlock blob;
            const std::uint8_t bytes[] = { 0x00, 0x7f, 0xff };
            blob.append (bytes, sizeof (bytes));
            juce::Array<juce::var> liste;
            liste.add (1);
            liste.add ("zukunft");
            common.setProperty ("future_binary", juce::var (blob), nullptr);
            common.setProperty ("future_array", juce::var (liste), nullptr);

            const auto roh = alsBlock (v);
            state::Zustand z;
            const auto erg = state::lade (roh.getData(), roh.getSize(), state::Bundle::eqcp(), z);
            juce::MemoryBlock wieder;
            state::speichere (z, wieder);
            const auto commonWieder = z.baum.getChildWithName ("Common");
            pruefe (erg == state::LadeErgebnis::geladen
                    && commonWieder.getProperty ("future_binary").isBinaryData()
                    && commonWieder.getProperty ("future_array").isArray()
                    && gleich (roh, wieder),
                    "additive Binaer- und Array-Properties bleiben bytegleich lesbar");
        }

        // Ein read-only-Prozessor wird durch einen gueltigen Stand wieder schreibbar.
        {
            auto p = std::make_unique<EqCopilotProcessor>();
            auto v = schema2Baum ("legacy", "insert", false); v.setProperty ("schema", 3, nullptr);
            const auto bytes = alsBlock (v);
            p->setStateInformation (bytes.getData(), (int) bytes.getSize());
            pruefe (p->stateNurLesen(), "read-only gesetzt");
            const auto saat = saatSchema1 ("sensor", "Klavier A", "");
            p->setStateInformation (saat.getData(), (int) saat.getSize());
            pruefe (! p->stateNurLesen() && p->holeLabel() == "Klavier A", "gueltiger Stand hebt read-only wieder auf");
        }
        a.schliesse ("Unbekanntes Major: " + juce::String (readOnly) + " Faelle read-only, Originalbytes bytegleich zurueck");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G10 · Duplicate (Copy/Paste in FL)
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        const auto saat = saatSchema1 ("sensor", "Klavier A", "");
        auto p1 = std::make_unique<EqCopilotProcessor>();
        auto p2 = std::make_unique<EqCopilotProcessor>();
        p1->setStateInformation (saat.getData(), (int) saat.getSize());
        p2->setStateInformation (saat.getData(), (int) saat.getSize());
        pruefe (p1->holeSensorId() == p2->holeSensorId(), "Duplikat: gleiche instance_id (der State IST der Messpunkt)", p1->holeSensorId());
        pruefe (p1->holeRuntimeNonce() != p2->holeRuntimeNonce() && p1->holeRuntimeNonce().isNotEmpty(), "Duplikat: verschiedene runtime_nonce");
        DirtyZaehler dirty;
        p2->addListener (&dirty);
        const auto vorher = p2->holeSensorId();
        pruefe (p2->neueSensorId(), "neueSensorId loest auf");
        pruefe (p2->holeSensorId() != vorher && p2->holeSensorId() != p1->holeSensorId() && p2->holeSensorId().length() == 32,
                "neue instance_id: 32 Hex, verschieden von beiden", p2->holeSensorId());
        pruefe (p2->holeLabel() == "Klavier A" && p2->holeRolle() == "sensor", "Label und Rolle bleiben bei der Aufloesung");
        pruefe (dirty.nonParam == 1, "Aufloesung meldet genau einmal Host-Dirty", juce::String (dirty.nonParam));
        p2->removeListener (&dirty);
        // Die neue ID reist mit dem naechsten Save.
        juce::MemoryBlock b; p2->getStateInformation (b);
        auto p3 = std::make_unique<EqCopilotProcessor>();
        p3->setStateInformation (b.getData(), (int) b.getSize());
        pruefe (p3->holeSensorId() == p2->holeSensorId(), "neue instance_id wird gespeichert und geladen");
        a.schliesse ("Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G11 · Host-Dirty
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        auto p = std::make_unique<EqCopilotProcessor>();
        DirtyZaehler dirty;
        p->addListener (&dirty);

        const auto saat = saatSchema1 ("pre", "Chor PRE", "paar-chor");
        p->setStateInformation (saat.getData(), (int) saat.getSize());
        pruefe (dirty.nonParam == 0, "Laden + Migration melden NICHT dirty", juce::String (dirty.nonParam));
        pruefe (p->holeStateHerkunft() == state::Herkunft::schema1Migriert, "Herkunft schema1Migriert");

        pruefe (! p->setzeBindung ("pre", "Chor PRE", "paar-chor") && dirty.nonParam == 0, "setzeBindung ohne Aenderung: keine Meldung");
        pruefe (p->setzeBindung ("post", "Chor POST", "paar-chor") && dirty.nonParam == 1, "setzeBindung mit Aenderung: genau eine Meldung", juce::String (dirty.nonParam));
        pruefe (p->holeRolle() == "post" && p->holeLabel() == "Chor POST", "Aenderung kam an");
        pruefe (p->setzeBindung ("hub", "Leitstand", "") && dirty.nonParam == 2, "Rollenwechsel zu hub: zweite Meldung");
        juce::MemoryBlock b; p->getStateInformation (b);
        const auto v = juce::ValueTree::readFromData (b.getData(), b.getSize());
        pruefe (v.getChildWithName ("MainProject").isValid() && ! v.getChildWithName ("Common").hasProperty ("pair_id"),
                "hub speichert MainProject und kein pair_id");

        juce::MemoryBlock b2; p->getStateInformation (b2);
        pruefe (dirty.nonParam == 2, "getStateInformation meldet nichts");

        pruefe (! p->setzeBindung ("dirigent", "x", "") && dirty.nonParam == 2, "unbekannte v2-Rolle wird verweigert, keine Meldung");

        juce::String zuLangesLabel, zuLangesPaar;
        for (int i = 0; i < 121; ++i) zuLangesLabel += "L";
        for (int i = 0; i < 61; ++i) zuLangesPaar += "P";
        pruefe (! p->setzeBindung ("hub", zuLangesLabel, "")
                && ! p->setzeBindung ("pre", "ok", zuLangesPaar)
                && dirty.nonParam == 2 && p->holeLabel() == "Leitstand",
                "Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation");

        // read-only verweigert ohne Meldung
        auto ro = schema2Baum ("legacy", "insert", false); ro.setProperty ("schema", 9, nullptr);
        const auto rb = alsBlock (ro);
        p->setStateInformation (rb.getData(), (int) rb.getSize());
        pruefe (p->stateNurLesen() && p->holeStateFremdesMajor() == 9, "read-only mit fremdem Major 9", juce::String (p->holeStateFremdesMajor()));
        pruefe (! p->setzeBindung ("hub", "x", "") && ! p->neueSensorId() && dirty.nonParam == 2, "read-only verweigert setzeBindung und neueSensorId ohne Meldung");
        p->removeListener (&dirty);
        a.schliesse ("Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G12 · Frische Instanz und Browser-Recall
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        auto p = std::make_unique<EqCopilotProcessor>();
        pruefe (p->holeStateHerkunft() == state::Herkunft::frisch && ! p->stateNurLesen(), "nie restauriert: Herkunft frisch");
        pruefe (p->holeRolle() == "sensor" && p->holeLabel().isEmpty() && p->holePaarId().isEmpty(), "frisch: legacy+insert = v2 'sensor', leeres Label");
        pruefe (p->holeSensorId().length() == 32 && p->holeSensorId().containsOnly ("0123456789abcdef"), "frisch: instance_id ist hex32", p->holeSensorId());
        juce::MemoryBlock b; p->getStateInformation (b);
        const auto v = juce::ValueTree::readFromData (b.getData(), b.getSize());
        pruefe (v.hasType ("NakamaState") && (int) v.getProperty ("schema") == 2
                && v.getChildWithName ("Common").getProperty ("plugin_kind").toString() == "legacy",
                "frisch speichert NakamaState schema 2, legacy");

        // Browser-Recall: Schema-2-Golden in eine frische Instanz.
        const auto golden = fixtureOrdner.getChildFile ("schema2/aus-schema1-post.bin");
        juce::MemoryBlock gb;
        if (golden.existsAsFile() && golden.loadFileAsData (gb))
        {
            auto frisch = std::make_unique<EqCopilotProcessor>();
            frisch->setStateInformation (gb.getData(), (int) gb.getSize());
            pruefe (frisch->holeRolle() == "post" && frisch->holeLabel() == "Chor POST" && frisch->holePaarId() == "paar-chor"
                    && frisch->holeSensorId() == "11111111-2222-3333-4444-555555555555"
                    && frisch->holeStateHerkunft() == state::Herkunft::schema2Geladen,
                    "Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz");
            juce::MemoryBlock wieder; frisch->getStateInformation (wieder);
            pruefe (gleich (wieder, gb), "Recall: Save nach Recall ist bytegleich zum Golden");
        }
        else
            pruefe (schreibeGoldens, "Schema-2-Golden fuer Recall vorhanden");
        a.schliesse ("Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich");
    }

    std::cout << std::endl
              << (fehler == 0 ? "STATE-MIGRATION-TEST OK" : "STATE-MIGRATION-TEST FEHLGESCHLAGEN")
              << " - " << bestanden << " Pruefungen ok, " << fehler << " Fehler" << std::endl;
    return fehler == 0 ? 0 : 1;
}
