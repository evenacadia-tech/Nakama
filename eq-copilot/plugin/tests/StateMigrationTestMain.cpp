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

#include <cstring>
#include <iostream>

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
            const bool geladen = (erg == state::LadeErgebnis::geladen);
            ++geprueft;
            if (geladen == z.erlaubt)
                ++ok;
            else
                pruefe (false, juce::String ("Matrix ") + state::wort (z.k) + " / " + z.position,
                        juce::String (z.erlaubt ? "sollte laden" : "sollte read-only sein")
                        + ", Grund '" + zurueck.grund + "'");
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
