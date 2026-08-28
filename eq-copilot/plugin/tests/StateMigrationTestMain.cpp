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
#include "NakamaState.h"
#include "PluginProcessor.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>

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

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceStart;

    bool schreibeGoldens = false;
    for (int i = 1; i < argc; ++i)
        if (juce::String (argv[i]) == "--schreibe-goldens")
            schreibeGoldens = true;

    const auto vertragDatei = finde ("eq-copilot/schemas/state/nakama-parameter-v1.json");
    const auto fixtureOrdner = finde ("eq-copilot/fixtures/state");
    std::cout << "Vertrag:  " << vertragDatei.getFullPathName() << std::endl
              << "Fixtures: " << fixtureOrdner.getFullPathName() << std::endl << std::endl;

    // ══════════════════════════════════════════════════════════════════════
    // G4 · Parametertabelle deckungsgleich mit dem Vertrag
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        bool ok = false;
        const auto v = ladeJson (vertragDatei, ok);
        pruefe (ok, "nakama-parameter-v1.json ist gueltiges JSON");
        const auto& t = param::tabelle();
        pruefe ((int) v["anzahl_parameter"] == param::kAnzahl, "anzahl_parameter == 109", v["anzahl_parameter"].toString());
        pruefe ((int) v["slot_anzahl"] == param::kSlots, "slot_anzahl == 8");
        pruefe ((int) v["dsp_schema_version"] == param::kDspSchemaVersion, "dsp_schema_version == 1");

        const auto* ids = v["ids"].getArray();
        pruefe (ids != nullptr && ids->size() == param::kAnzahl, "ids-Liste hat 109 Eintraege");
        if (ids != nullptr && ids->size() == param::kAnzahl)
        {
            int gleichIds = 0;
            for (int i = 0; i < param::kAnzahl; ++i)
                if ((*ids)[i].toString() == t[(size_t) i].id) ++gleichIds;
            pruefe (gleichIds == param::kAnzahl, "alle 109 IDs in Vertragsreihenfolge identisch", juce::String (gleichIds));
        }

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
            if (! gut)
                pruefe (false, "Eintrag weicht ab: " + wo, b.id);
            return gut;
        };

        int deckungsgleich = 0;
        const auto* global = v["global"].getArray();
        const auto* vorlage = v["band_vorlage"].getArray();
        if (global != nullptr && vorlage != nullptr && global->size() == param::kGlobal && vorlage->size() == param::kJeSlot)
        {
            for (int i = 0; i < param::kGlobal; ++i)
                if (vergleiche ((*global)[i], i, (*global)[i]["id"].toString())) ++deckungsgleich;
            for (int slot = 0; slot < param::kSlots; ++slot)
                for (int k = 0; k < param::kJeSlot; ++k)
                    if (vergleiche ((*vorlage)[k], param::kGlobal + slot * param::kJeSlot + k,
                                    "Slot " + juce::String (slot) + " " + (*vorlage)[k]["name"].toString()))
                        ++deckungsgleich;
        }
        pruefe (deckungsgleich == param::kAnzahl, "109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich",
                juce::String (deckungsgleich));
        pruefe (param::indexVonId ("v1.band.7.sidechain_source") == 108 && param::indexVonId ("v1.global.bypass") == 0
                && param::indexVonId ("v1.band.8.enabled") < 0, "indexVonId: erste, letzte, nicht vorhandene ID");
        a.schliesse ("Parametertabelle deckungsgleich mit nakama-parameter-v1.json");
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
                param::Satz s; juce::String grund, detail;
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
            pruefe (param::stateHash (param::standardSatz(), hash, grund) && hash == soll,
                    "standardSatz() hasht wie dto/gueltig/default.json", hash);
        }

        if (const auto* us = manifest["dto_ungueltig"].getArray())
        {
            for (const auto& e : *us)
            {
                ++ungueltigGesamt;
                const auto datei = fixtureOrdner.getChildFile (e["datei"].toString());
                juce::MemoryBlock roh;
                if (! datei.loadFileAsData (roh)) { pruefe (false, "DTO fehlt", datei.getFileName()); continue; }
                param::Satz s; juce::String grund, detail;
                const bool angenommen = param::ausDtoText (roh.getData(), roh.getSize(), s, grund, detail);
                if (! angenommen && grund == e["grund"].toString()) ++ungueltigOk;
                else pruefe (false, "ungueltiges DTO: " + e["datei"].toString(),
                             (angenommen ? juce::String ("ANGENOMMEN") : "Grund " + grund + " (" + detail + ")") + " soll " + e["grund"].toString());
            }
        }
        pruefe (ungueltigGesamt > 0 && ungueltigOk == ungueltigGesamt, "ungueltige DTOs mit dem erwarteten Grund abgelehnt", juce::String (ungueltigOk) + "/" + juce::String (ungueltigGesamt));

        // Nichtendlich aus dem TYPISIERTEN Satz (nicht nur aus Text).
        {
            auto s = param::standardSatz();
            s[2].zahl = std::numeric_limits<double>::quiet_NaN();
            juce::String hash, grund;
            pruefe (! param::stateHash (s, hash, grund) && grund == "nichtendlich", "NaN im Satz faellt vor dem Hash", grund);
            s[2].zahl = std::numeric_limits<double>::infinity();
            pruefe (! param::stateHash (s, hash, grund) && grund == "nichtendlich", "Inf im Satz faellt vor dem Hash", grund);
            s[2].zahl = 24.000001;
            pruefe (! param::stateHash (s, hash, grund) && grund == "bereich", "Bereich im Satz faellt vor dem Hash", grund);
            s[2].zahl = 0.0; s[6].enumIndex = 99;
            pruefe (! param::stateHash (s, hash, grund) && grund == "enum", "Enumindex im Satz faellt vor dem Hash", grund);
        }
        a.schliesse ("DTO: " + juce::String (gueltigOk) + " gueltige gehasht, " + juce::String (ungueltigOk) + " ungueltige vor dem Hash abgelehnt");
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

        // Parameters: 109 Werte bit-exakt (active_probe-Bundle).
        {
            // Jeder Gleitkomma-Parameter knapp unter seinem Maximum (ein double,
            // das keine kurze Dezimalform hat), dazu ein paar Schalter/Enums.
            auto s = param::standardSatz();
            for (int i = 0; i < param::kAnzahl; ++i)
                if (param::tabelle()[(size_t) i].typ == param::Typ::gleitkomma)
                    s[(size_t) i].zahl = std::nextafter (param::tabelle()[(size_t) i].max, 0.0);
            s[3].zahl = 0.30000000000000004;   // width: der klassische Nicht-Dezimalwert
            s[5].b = true; s[6].enumIndex = 4; s[17].enumIndex = 2;
            state::Zustand za = state::frisch ("ffffffffffffffffffffffffffffffff");
            za.common.klasse = state::Klasse::active_probe;
            za.hatParameters = true;
            za.parameters = s;
            juce::MemoryBlock ba, bb;
            state::speichere (za, ba);
            state::Zustand zb;
            pruefe (state::lade (ba.getData(), ba.getSize(), state::Bundle::nkac(), zb) == state::LadeErgebnis::geladen, "active_probe mit Parameters laedt (Bundle NkAc)");
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
            pruefe (bitExakt == param::kAnzahl, "109 Parameterwerte bit-exakt durch den Roundtrip", juce::String (bitExakt));
            state::speichere (zb, bb);
            pruefe (gleich (ba, bb), "Parameters-Roundtrip bytegleich");
            juce::String h1, h2, g;
            pruefe (param::stateHash (s, h1, g) && param::stateHash (zb.parameters, h2, g) && h1 == h2, "state_hash ueberlebt den Roundtrip", h1);
        }
        a.schliesse ("Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt");
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

                EqCopilotProcessor p;
                DirtyZaehler dirty;
                p.addListener (&dirty);
                p.setStateInformation (roh.getData(), (int) roh.getSize());
                hostReadOnlyOk = p.stateNurLesen();
                hostDirtyOk = dirty.nonParam == 0;
                p.removeListener (&dirty);
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
        a.schliesse ("Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle");
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
            EqCopilotProcessor p;
            DirtyZaehler dirty;
            p.addListener (&dirty);
            p.setStateInformation (bytes.getData(), (int) bytes.getSize());
            juce::MemoryBlock heraus;
            p.getStateInformation (heraus);
            const bool verweigert = ! p.setzeBindung ("hub", "Kaperung", "") && ! p.neueSensorId();
            const bool prozOk = p.stateNurLesen() && p.holeStateHerkunft() == state::Herkunft::nurLesen
                                && gleich (heraus, bytes) && verweigert && dirty.nonParam == 0;
            p.removeListener (&dirty);

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
            constexpr size_t maxSchreibbarerInput = maxState - 4096u;
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
            z.common.label = langesLabel;
            z.common.pairId = langesPaar;
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
            EqCopilotProcessor p;
            auto v = schema2Baum ("legacy", "insert", false); v.setProperty ("schema", 3, nullptr);
            const auto bytes = alsBlock (v);
            p.setStateInformation (bytes.getData(), (int) bytes.getSize());
            pruefe (p.stateNurLesen(), "read-only gesetzt");
            const auto saat = saatSchema1 ("sensor", "Klavier A", "");
            p.setStateInformation (saat.getData(), (int) saat.getSize());
            pruefe (! p.stateNurLesen() && p.holeLabel() == "Klavier A", "gueltiger Stand hebt read-only wieder auf");
        }
        a.schliesse ("Unbekanntes Major: " + juce::String (readOnly) + " Faelle read-only, Originalbytes bytegleich zurueck");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G10 · Duplicate (Copy/Paste in FL)
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        const auto saat = saatSchema1 ("sensor", "Klavier A", "");
        EqCopilotProcessor p1, p2;
        p1.setStateInformation (saat.getData(), (int) saat.getSize());
        p2.setStateInformation (saat.getData(), (int) saat.getSize());
        pruefe (p1.holeSensorId() == p2.holeSensorId(), "Duplikat: gleiche instance_id (der State IST der Messpunkt)", p1.holeSensorId());
        pruefe (p1.holeRuntimeNonce() != p2.holeRuntimeNonce() && p1.holeRuntimeNonce().isNotEmpty(), "Duplikat: verschiedene runtime_nonce");
        DirtyZaehler dirty;
        p2.addListener (&dirty);
        const auto vorher = p2.holeSensorId();
        pruefe (p2.neueSensorId(), "neueSensorId loest auf");
        pruefe (p2.holeSensorId() != vorher && p2.holeSensorId() != p1.holeSensorId() && p2.holeSensorId().length() == 32,
                "neue instance_id: 32 Hex, verschieden von beiden", p2.holeSensorId());
        pruefe (p2.holeLabel() == "Klavier A" && p2.holeRolle() == "sensor", "Label und Rolle bleiben bei der Aufloesung");
        pruefe (dirty.nonParam == 1, "Aufloesung meldet genau einmal Host-Dirty", juce::String (dirty.nonParam));
        p2.removeListener (&dirty);
        // Die neue ID reist mit dem naechsten Save.
        juce::MemoryBlock b; p2.getStateInformation (b);
        EqCopilotProcessor p3; p3.setStateInformation (b.getData(), (int) b.getSize());
        pruefe (p3.holeSensorId() == p2.holeSensorId(), "neue instance_id wird gespeichert und geladen");
        a.schliesse ("Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G11 · Host-Dirty
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        EqCopilotProcessor p;
        DirtyZaehler dirty;
        p.addListener (&dirty);

        const auto saat = saatSchema1 ("pre", "Chor PRE", "paar-chor");
        p.setStateInformation (saat.getData(), (int) saat.getSize());
        pruefe (dirty.nonParam == 0, "Laden + Migration melden NICHT dirty", juce::String (dirty.nonParam));
        pruefe (p.holeStateHerkunft() == state::Herkunft::schema1Migriert, "Herkunft schema1Migriert");

        pruefe (! p.setzeBindung ("pre", "Chor PRE", "paar-chor") && dirty.nonParam == 0, "setzeBindung ohne Aenderung: keine Meldung");
        pruefe (p.setzeBindung ("post", "Chor POST", "paar-chor") && dirty.nonParam == 1, "setzeBindung mit Aenderung: genau eine Meldung", juce::String (dirty.nonParam));
        pruefe (p.holeRolle() == "post" && p.holeLabel() == "Chor POST", "Aenderung kam an");
        pruefe (p.setzeBindung ("hub", "Leitstand", "") && dirty.nonParam == 2, "Rollenwechsel zu hub: zweite Meldung");
        juce::MemoryBlock b; p.getStateInformation (b);
        const auto v = juce::ValueTree::readFromData (b.getData(), b.getSize());
        pruefe (v.getChildWithName ("MainProject").isValid() && ! v.getChildWithName ("Common").hasProperty ("pair_id"),
                "hub speichert MainProject und kein pair_id");

        juce::MemoryBlock b2; p.getStateInformation (b2);
        pruefe (dirty.nonParam == 2, "getStateInformation meldet nichts");

        pruefe (! p.setzeBindung ("dirigent", "x", "") && dirty.nonParam == 2, "unbekannte v2-Rolle wird verweigert, keine Meldung");

        juce::String zuLangesLabel, zuLangesPaar;
        for (int i = 0; i < 121; ++i) zuLangesLabel += "L";
        for (int i = 0; i < 61; ++i) zuLangesPaar += "P";
        pruefe (! p.setzeBindung ("hub", zuLangesLabel, "")
                && ! p.setzeBindung ("pre", "ok", zuLangesPaar)
                && dirty.nonParam == 2 && p.holeLabel() == "Leitstand",
                "Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation");

        // read-only verweigert ohne Meldung
        auto ro = schema2Baum ("legacy", "insert", false); ro.setProperty ("schema", 9, nullptr);
        const auto rb = alsBlock (ro);
        p.setStateInformation (rb.getData(), (int) rb.getSize());
        pruefe (p.stateNurLesen() && p.holeStateFremdesMajor() == 9, "read-only mit fremdem Major 9", juce::String (p.holeStateFremdesMajor()));
        pruefe (! p.setzeBindung ("hub", "x", "") && ! p.neueSensorId() && dirty.nonParam == 2, "read-only verweigert setzeBindung und neueSensorId ohne Meldung");
        p.removeListener (&dirty);
        a.schliesse ("Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert");
    }

    // ══════════════════════════════════════════════════════════════════════
    // G12 · Frische Instanz und Browser-Recall
    // ══════════════════════════════════════════════════════════════════════
    {
        Abschnitt a;
        EqCopilotProcessor p;
        pruefe (p.holeStateHerkunft() == state::Herkunft::frisch && ! p.stateNurLesen(), "nie restauriert: Herkunft frisch");
        pruefe (p.holeRolle() == "sensor" && p.holeLabel().isEmpty() && p.holePaarId().isEmpty(), "frisch: legacy+insert = v2 'sensor', leeres Label");
        pruefe (p.holeSensorId().length() == 32 && p.holeSensorId().containsOnly ("0123456789abcdef"), "frisch: instance_id ist hex32", p.holeSensorId());
        juce::MemoryBlock b; p.getStateInformation (b);
        const auto v = juce::ValueTree::readFromData (b.getData(), b.getSize());
        pruefe (v.hasType ("NakamaState") && (int) v.getProperty ("schema") == 2
                && v.getChildWithName ("Common").getProperty ("plugin_kind").toString() == "legacy",
                "frisch speichert NakamaState schema 2, legacy");

        // Browser-Recall: Schema-2-Golden in eine frische Instanz.
        const auto golden = fixtureOrdner.getChildFile ("schema2/aus-schema1-post.bin");
        juce::MemoryBlock gb;
        if (golden.existsAsFile() && golden.loadFileAsData (gb))
        {
            EqCopilotProcessor frisch;
            frisch.setStateInformation (gb.getData(), (int) gb.getSize());
            pruefe (frisch.holeRolle() == "post" && frisch.holeLabel() == "Chor POST" && frisch.holePaarId() == "paar-chor"
                    && frisch.holeSensorId() == "11111111-2222-3333-4444-555555555555"
                    && frisch.holeStateHerkunft() == state::Herkunft::schema2Geladen,
                    "Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz");
            juce::MemoryBlock wieder; frisch.getStateInformation (wieder);
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
