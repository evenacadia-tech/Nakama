#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 — keine JucePlugin_*-Konstante im Kern
#include "NakamaState.h"
#include "NakamaUtf8.h"

#include <cstdint>
#include <cstring>
#include <limits>
#include <new>

namespace nakama::state
{

namespace
{
const juce::Identifier kRoot        ("NakamaState");
const juce::Identifier kAltRoot     ("EqCopilotState");
const juce::Identifier kSchema      ("schema");
const juce::Identifier kCommon      ("Common");
const juce::Identifier kMainProject ("MainProject");
const juce::Identifier kParameters  ("Parameters");
const juce::Identifier kDsp         ("Dsp");
const juce::Identifier kPairing     ("Pairing");
const juce::Identifier kInstanceId  ("instance_id");
const juce::Identifier kKind        ("plugin_kind");
const juce::Identifier kPosition    ("measurement_position");
const juce::Identifier kLabel       ("label");
const juce::Identifier kPairId      ("pair_id");
const juce::Identifier kBinding     ("project_binding_id");
// Schema 1
const juce::Identifier kSensorId    ("sensor_id");
const juce::Identifier kRole        ("role");

constexpr int kRootSchema   = 2;
constexpr int kCommonSchema = 1;
constexpr int kMainSchema   = 1;
constexpr int kParamSchema  = 1;

// `ValueTree::readFromData()` ist absichtlich tolerant: es prueft weder EOF
// noch einen abgebrochenen spaeten Kindbaum und `var::readFromStream()` glaubt
// deklarierte Binaerlaengen. Host-State ist jedoch ein persistenter Vertrag,
// kein Best-Effort-Stream. Dieser kleine, allokationsfreie Vorleser akzeptiert
// deshalb nur genau EINEN vollstaendigen, begrenzten JUCE-ValueTree.
constexpr size_t kMaxStateBytes = 16u * 1024u * 1024u;
constexpr int kMaxStateTiefe = 64;
constexpr int kMaxVariantenTiefe = 64;
constexpr int kMaxEintraegeJeSammlung = 65536;
constexpr int kMaxEintraegeGesamt = 262144;

enum class BytePruefung
{
    ungueltig,
    verlustfrei,
    bekannteWurzelNichtVerlustfrei
};

class ValueTreeByteRiegel
{
public:
    ValueTreeByteRiegel (const void* daten, size_t laenge)
        : anfang (static_cast<const std::uint8_t*> (daten)), pos (anfang),
          ende (anfang != nullptr ? anfang + laenge : nullptr)
    {
    }

    BytePruefung pruefe()
    {
        if (anfang == nullptr || anfang == ende)
            return BytePruefung::ungueltig;

        if (! baum (0) || pos != ende)
            return BytePruefung::ungueltig;
        if (nichtVerlustfrei && bekannteWurzel)
            return BytePruefung::bekannteWurzelNichtVerlustfrei;
        return BytePruefung::verlustfrei;
    }

private:
    bool hat (size_t n) const noexcept
    {
        return n <= static_cast<size_t> (ende - pos);
    }

    bool komprimierteZahl (std::int64_t& aus)
    {
        if (! hat (1))
            return false;

        const auto kopf = *pos++;
        if (kopf == 0)
        {
            aus = 0;
            return true;
        }

        const auto n = static_cast<size_t> (kopf & 0x7fu);
        if (n == 0 || n > 4 || ! hat (n))
            return false;

        std::uint32_t wert = 0;
        for (size_t i = 0; i < n; ++i)
            wert |= static_cast<std::uint32_t> (pos[i]) << (8u * static_cast<unsigned> (i));
        pos += n;

        if ((kopf & 0x80u) != 0)
        {
            if (wert > 0x80000000u)
                return false;
            aus = -static_cast<std::int64_t> (wert);
        }
        else
        {
            if (wert > static_cast<std::uint32_t> (std::numeric_limits<int>::max()))
                return false;
            aus = static_cast<std::int64_t> (wert);
        }
        return true;
    }

    bool utf8CString (bool darfLeerSein, bool istWurzel = false)
    {
        const auto rest = static_cast<size_t> (ende - pos);
        const auto* nul = static_cast<const std::uint8_t*> (std::memchr (pos, 0, rest));
        if (nul == nullptr || (! darfLeerSein && nul == pos))
            return false;

        const auto n = static_cast<size_t> (nul - pos);
        if (n > static_cast<size_t> (std::numeric_limits<int>::max())
            || ! utf8::istGueltig (pos, n))
            return false;

        if (istWurzel)
        {
            const auto gleich = [this, n] (const char* text)
            {
                return n == std::strlen (text)
                    && std::memcmp (pos, text, n) == 0;
            };
            bekannteWurzel = gleich ("NakamaState") || gleich ("EqCopilotState");
        }

        pos = nul + 1;
        return true;
    }

    bool zaehler (int& aus)
    {
        std::int64_t wert = 0;
        if (! komprimierteZahl (wert) || wert < 0 || wert > kMaxEintraegeJeSammlung)
            return false;
        if (eintraegeGesamt > kMaxEintraegeGesamt - static_cast<int> (wert))
            return false;
        eintraegeGesamt += static_cast<int> (wert);
        aus = static_cast<int> (wert);
        return true;
    }

    bool variante (int tiefe)
    {
        if (tiefe >= kMaxVariantenTiefe)
            return false;

        std::int64_t laenge64 = 0;
        if (! komprimierteZahl (laenge64) || laenge64 < 0)
            return false;
        if (laenge64 == 0)
            return true;

        const auto laenge = static_cast<size_t> (laenge64);
        if (! hat (laenge))
            return false;

        const auto* const variantenEnde = pos + laenge;
        const auto marke = *pos++;
        const auto nutzlaenge = laenge - 1u;

        switch (marke)
        {
            case 1: // int32
                if (nutzlaenge != 4u) return false;
                pos += 4;
                break;
            case 2: // bool true
            case 3: // bool false
                if (nutzlaenge != 0u) return false;
                break;
            case 4: // double
            case 6: // int64
                if (nutzlaenge != 8u) return false;
                pos += 8;
                break;
            case 5: // UTF-8 inklusive genau eines abschliessenden NUL
            {
                if (nutzlaenge == 0u || variantenEnde[-1] != 0
                    || std::memchr (pos, 0, nutzlaenge - 1u) != nullptr
                    || nutzlaenge - 1u > static_cast<size_t> (std::numeric_limits<int>::max())
                    || ! utf8::istGueltig (pos, nutzlaenge - 1u))
                    return false;
                pos = variantenEnde;
                break;
            }
            case 7: // Array: eigener, laengenbegrenzter Unterstrom
            {
                const auto* const altesEnde = ende;
                ende = variantenEnde;
                int n = 0;
                const bool kopfOk = zaehler (n);
                bool inhaltOk = kopfOk;
                for (int i = 0; inhaltOk && i < n; ++i)
                    inhaltOk = variante (tiefe + 1);
                const bool genau = inhaltOk && pos == ende;
                ende = altesEnde;
                if (! genau)
                    return false;
                break;
            }
            case 8: // MemoryBlock
                pos = variantenEnde;
                break;
            case 9: // undefined
            default:
                // JUCE 8 schreibt `undefined` als Marker 9, liest Marker 9 und
                // unbekannte Marker aber als void. Ein schreibbarer Load
                // wuerde die Bytes beim naechsten Save still veraendern. Die
                // Struktur ist sicher ueberspringbar, der bekannte State wird
                // deshalb als read-only mit Originalbytes gehalten.
                nichtVerlustfrei = true;
                pos = variantenEnde;
                break;
        }

        return pos == variantenEnde;
    }

    bool baum (int tiefe)
    {
        if (tiefe >= kMaxStateTiefe || ! utf8CString (false, tiefe == 0))
            return false;

        int eigenschaften = 0;
        if (! zaehler (eigenschaften))
            return false;
        for (int i = 0; i < eigenschaften; ++i)
            if (! utf8CString (false) || ! variante (0))
                return false;

        int kinder = 0;
        if (! zaehler (kinder))
            return false;
        for (int i = 0; i < kinder; ++i)
            if (! baum (tiefe + 1))
                return false;
        return true;
    }

    const std::uint8_t* anfang = nullptr;
    const std::uint8_t* pos = nullptr;
    const std::uint8_t* ende = nullptr;
    int eintraegeGesamt = 0;
    bool bekannteWurzel = false;
    bool nichtVerlustfrei = false;
};

BytePruefung pruefeValueTreeBytes (const void* daten, size_t laenge)
{
    if (laenge > kMaxStateBytes)
        return BytePruefung::ungueltig;
    return ValueTreeByteRiegel (daten, laenge).pruefe();
}

bool istHex32 (const juce::String& wert)
{
    if (wert.length() != 32)
        return false;
    for (const auto c : wert)
        if (! ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
            return false;
    return true;
}

bool istInt (const juce::var& v, int erwartet)
{
    return v.isInt() && (int) v == erwartet;
}

bool schemaIst (const juce::ValueTree& t, int erwartet)
{
    return istInt (t.getProperty (kSchema), erwartet);
}

int schemaLesbar (const juce::ValueTree& t)
{
    const auto v = t.getProperty (kSchema);
    return v.isInt() ? (int) v : 0;
}

} // namespace

// ── Woerter ────────────────────────────────────────────────────────────────

const char* wort (Klasse k)
{
    switch (k)
    {
        case Klasse::main:          return "main";
        case Klasse::passive_probe: return "passive_probe";
        case Klasse::active_probe:  return "active_probe";
        case Klasse::legacy:        return "legacy";
    }
    return "legacy";
}

const char* wort (Messposition p)
{
    switch (p)
    {
        case Messposition::insert:                  return "insert";
        case Messposition::pre:                     return "pre";
        case Messposition::post:                    return "post";
        case Messposition::post_fader_contribution: return "post_fader_contribution";
    }
    return "insert";
}

bool klasseAusWort (const juce::String& w, Klasse& aus)
{
    if (w == "main")          { aus = Klasse::main; return true; }
    if (w == "passive_probe") { aus = Klasse::passive_probe; return true; }
    if (w == "active_probe")  { aus = Klasse::active_probe; return true; }
    if (w == "legacy")        { aus = Klasse::legacy; return true; }
    return false;
}

bool positionAusWort (const juce::String& w, Messposition& aus)
{
    if (w == "insert")                  { aus = Messposition::insert; return true; }
    if (w == "pre")                     { aus = Messposition::pre; return true; }
    if (w == "post")                    { aus = Messposition::post; return true; }
    if (w == "post_fader_contribution") { aus = Messposition::post_fader_contribution; return true; }
    return false;
}

/*  §53.6-Capability `contribution_aux`, gemessen und eingefroren in
    identity/host-capabilities-fl-v1.json: **unsupported**. Rohfeld dort:
    "nicht gemessen - kein Geraet misst die Main-Aux-Busse des Receivers".
    Der in §53.6 festgelegte Fallback dazu lautet woertlich "nur Assoziation
    statt exakter Attribution".

    Solange das so steht, kann KEIN Bau dieses Repos die Messposition
    `post_fader_contribution` tragen: §32.2 definiert sie als "post-fader
    Sidechain-only-Send auf einen eigenen diskreten Aux-Bus eines
    Contribution-Receivers", und kein gebautes Bundle hat einen solchen Bus -
    `sonde/SondeProcessor.cpp` deklariert genau einen Stereo-Ein- und einen
    Stereo-Ausgang, das Main-Bundle keine Contribution-Aux-Busse.

    Deshalb ist das hier eine KONSTANTE und kein Schalter: sie wird wahr, wenn
    ein Bau den Bus wirklich hat und die Capability es gemessen sagt (SONDE-011
    haengt daran), nicht wenn jemand sie umstellt. */
constexpr bool kContributionAuxVerfuegbar = false;

/*  Welche Messposition darf eine Instanz dieser Klasse FUEHREN?

    ⚠️ Bis zum 24.08.2026 stand hier `case Klasse::passive_probe: return true;`
    - die passive Sonde war die einzige Klasse ohne Einschraenkung und durfte
    sich damit dauerhaft `post_fader_contribution` nennen, obwohl ihr Bundle
    gar keinen Aux-Bus hat. Der Gate-Lauf G1 (§4.2) hat den Pfad gefahren:
    Host-State-Restore -> lade() -> positionErlaubt -> uebernommen -> beim
    naechsten Speichern wieder hinausgeschrieben. Eine gewoehnliche
    Standard-Insertinstanz konnte sich so als exakter Mastersummenbeitrag
    bezeichnen - Gate 7 aus §49.2 Nr. 7 im Wortlaut.

    🔑 Das war KEINE offene Produktfrage, obwohl es zunaechst so aussah. Zwei
    gemessene Dinge entscheiden sie:
      1. `schemas/state/nakama-state-v2.md` nannte die Erlaubnis ausdruecklich
         "Vorschlag fuer SONDE-007b" - und ein Vorschlag bindet in diesem
         Projekt nichts (CLAUDE.md: "Ein Entscheid existiert nur mit Datum +
         Zitat des Users").
      2. Der eingefrorene Capabilityreport sagt `contribution_aux:
         unsupported` und gibt dem Fall seinen eigenen Fallback.
    Der Code setzte also einen unangenommenen Vorschlag GEGEN eine gemessene
    Capability durch. Das ist dieselbe Fehlerklasse wie §4.1: ein Vertragstext,
    der nie eingelöst wurde.

    Ein Altprojekt geht dabei nicht verloren: `lade()` faellt bei verletzter
    Matrix auf read-only mit den ORIGINALBYTES zurueck (§53.8), sichtbar im
    Editor - es verliert seinen Stand nicht, es darf ihn nur nicht mehr
    behaupten. */
bool positionErlaubt (Klasse k, Messposition p)
{
    // Riegel 1 - die CAPABILITY-Frage: hat ueberhaupt irgendein Bau den Bus?
    // Heute nein, gemessen (siehe oben). Diese Haelfte faellt weg, sobald ein
    // Bau ihn hat.
    if (p == Messposition::post_fader_contribution && ! kContributionAuxVerfuegbar)
        return false;

    // Riegel 2 - die KLASSEN-Frage: WELCHE Klasse darf die Position fuehren,
    // wenn es den Bus gibt? Die beantwortet der Entwurf nicht: §32.2 ordnet
    // Positionen ihren Aussageklassen zu und benennt in :1610 den Receiver,
    // sagt aber nirgends, welche Produktklasse die Position TRAEGT. Der
    // Gate-Bericht G1 §4.2 ist an genau dieser Kante stehengeblieben und nennt
    // sie eine offene Produktfrage - zu Recht.
    //
    // Hier steht deshalb kein Urteil, sondern die fail-closed-Vorgabe: was
    // niemand erlaubt hat, gilt nicht. Sie kostet nichts (die Position ist
    // heute ohnehin unerreichbar) und sie haelt, wenn Riegel 1 spaeter faellt -
    // die beiden Haelften beantworten VERSCHIEDENE Fragen, und keine ersetzt
    // die andere. Wer die Position oeffnet, muss beide anfassen und dabei die
    // Produktfrage beantwortet haben (offene Frage im Register, SONDE-011).
    switch (k)
    {
        case Klasse::main:          return p == Messposition::insert;
        case Klasse::legacy:        return p != Messposition::post_fader_contribution;
        case Klasse::passive_probe: return p != Messposition::post_fader_contribution;
        case Klasse::active_probe:  return p != Messposition::post_fader_contribution;
    }
    return false;
}

bool Common::operator== (const Common& a) const noexcept
{
    return instanceId == a.instanceId && klasse == a.klasse && position == a.position
        && label == a.label && pairId == a.pairId && projectBindingId == a.projectBindingId;
}

bool Bundle::erlaubt (Klasse k) const noexcept
{
    for (auto x : klassen)
        if (x == k)
            return true;
    return false;
}

// ── Bauen ──────────────────────────────────────────────────────────────────

namespace
{

juce::ValueTree baueCommon (const Common& c)
{
    juce::ValueTree t (kCommon);
    t.setProperty (kSchema, kCommonSchema, nullptr);
    t.setProperty (kInstanceId, c.instanceId, nullptr);
    t.setProperty (kKind, juce::String (wort (c.klasse)), nullptr);
    t.setProperty (kPosition, juce::String (wort (c.position)), nullptr);
    t.setProperty (kLabel, c.label, nullptr);
    if (c.pairId.isNotEmpty())
        t.setProperty (kPairId, c.pairId, nullptr);
    if (c.projectBindingId.isNotEmpty())
        t.setProperty (kBinding, c.projectBindingId, nullptr);
    return t;
}

/** Schreibt die typisierten Felder in eine KOPIE des gehaltenen Baums -
    bestehende (auch unbekannte) Eigenschaften bleiben, die Kind-Matrix wird
    hergestellt. */
juce::ValueTree synchronisiert (const Zustand& z)
{
    juce::ValueTree kopie = z.baum.isValid() ? z.baum.createCopy() : juce::ValueTree (kRoot);
    kopie.setProperty (kSchema, kRootSchema, nullptr);

    auto common = kopie.getChildWithName (kCommon);
    if (! common.isValid())
    {
        common = juce::ValueTree (kCommon);
        kopie.addChild (common, 0, nullptr);
    }
    common.setProperty (kSchema, kCommonSchema, nullptr);
    common.setProperty (kInstanceId, z.common.instanceId, nullptr);
    common.setProperty (kKind, juce::String (wort (z.common.klasse)), nullptr);
    common.setProperty (kPosition, juce::String (wort (z.common.position)), nullptr);
    common.setProperty (kLabel, z.common.label, nullptr);
    if (z.common.pairId.isNotEmpty()) common.setProperty (kPairId, z.common.pairId, nullptr);
    else                              common.removeProperty (kPairId, nullptr);
    if (z.common.projectBindingId.isNotEmpty()) common.setProperty (kBinding, z.common.projectBindingId, nullptr);
    else                                        common.removeProperty (kBinding, nullptr);

    auto mainProject = kopie.getChildWithName (kMainProject);
    if (z.common.klasse == Klasse::main)
    {
        if (! mainProject.isValid())
        {
            mainProject = juce::ValueTree (kMainProject);
            kopie.appendChild (mainProject, nullptr);
        }
        mainProject.setProperty (kSchema, kMainSchema, nullptr);
    }
    else if (mainProject.isValid())
    {
        kopie.removeChild (mainProject, nullptr);
    }

    auto parameters = kopie.getChildWithName (kParameters);
    if (z.hatParameters)
    {
        if (! parameters.isValid())
        {
            parameters = juce::ValueTree (kParameters);
            kopie.appendChild (parameters, nullptr);
        }
        parameters.setProperty (kSchema, kParamSchema, nullptr);
        parameter::schreibeInBaum (z.parameters, parameters);
    }
    else if (parameters.isValid())
    {
        kopie.removeChild (parameters, nullptr);
    }

    return kopie;
}

/** Beweist beim Laden, dass jede heute ueber die Produkt-API erreichbare
    Aenderung wieder einen State <= 16 MiB schreibt. Eine pauschale Reserve
    funktioniert hier nicht: sobald der Writer sie verbraucht, laege sein
    eigenes Ergebnis oberhalb derselben Schreibbar-Schwelle. Deshalb wird der
    groesste konkrete Folgezustand gegen den gehaltenen additiven Baum gebaut.

    Die UI-Grenzen gelten in Unicode-Codepunkten. U+10FFFF belegt vier UTF-8-
    Bytes und bildet damit die echte Worst-Case-Groesse fuer 120/60 Zeichen. */
bool hatWriterHeadroom (const Zustand& eingang, const Bundle& bundle)
{
    auto maximalerText = [] (int zeichen)
    {
        juce::String s;
        s.preallocateBytes (zeichen * 4);
        for (int i = 0; i < zeichen; ++i)
            s += juce::String::charToString (static_cast<juce::juce_wchar> (0x10ffff));
        return s;
    };
    auto laenger = [] (const juce::String& a, const juce::String& b)
    {
        return a.getNumBytesAsUTF8() >= b.getNumBytesAsUTF8() ? a : b;
    };
    auto passt = [] (const Zustand& kandidat)
    {
        try
        {
            juce::MemoryBlock bytes;
            juce::MemoryOutputStream strom (bytes, false);
            synchronisiert (kandidat).writeToStream (strom);
            strom.flush();
            return bytes.getSize() <= kMaxStateBytes;
        }
        catch (...)
        {
            return false;
        }
    };

    Zustand kandidat = eingang;
    kandidat.common.instanceId = laenger (eingang.common.instanceId,
                                          "ffffffffffffffffffffffffffffffff");
    kandidat.common.label = laenger (eingang.common.label, maximalerText (120));
    kandidat.common.pairId = laenger (eingang.common.pairId, maximalerText (60));
    kandidat.common.projectBindingId = laenger (
        eingang.common.projectBindingId, "ffffffffffffffffffffffffffffffff");

    // Eqcp kann zwischen main und legacy sowie allen heute erlaubten v2-
    // Positionen wechseln. Fuer Sonden ist die Menge kleiner; die Schleife
    // bleibt trotzdem die eine Wahrheit aus dem Bundlevertrag.
    constexpr Messposition positionen[] = {
        Messposition::insert, Messposition::pre, Messposition::post,
        Messposition::post_fader_contribution
    };
    for (const auto klasse : bundle.klassen)
    {
        for (const auto position : positionen)
        {
            if (! positionErlaubt (klasse, position))
                continue;
            kandidat.common.klasse = klasse;
            kandidat.common.position = position;
            kandidat.hatParameters = klasse == Klasse::active_probe;
            if (! passt (kandidat))
                return false;
        }
    }
    return true;
}

/** Liest einen NakamaState-Baum vollstaendig oder gar nicht. */
bool leseSchema2 (const juce::ValueTree& v, const Bundle& bundle, Zustand& aus, juce::String& grund)
{
    if (! v.hasType (kRoot)) { grund = "not a NakamaState"; return false; }
    if (! schemaIst (v, kRootSchema))
    {
        grund = "NakamaState schema " + v.getProperty (kSchema).toString() + " is unknown to this version (it reads schema 2)";
        return false;
    }

    int nCommon = 0, nMain = 0, nParam = 0, nDsp = 0, nPairing = 0;
    for (int i = 0; i < v.getNumChildren(); ++i)
    {
        const auto kind = v.getChild (i);
        if      (kind.hasType (kCommon))      ++nCommon;
        else if (kind.hasType (kMainProject)) ++nMain;
        else if (kind.hasType (kParameters))  ++nParam;
        else if (kind.hasType (kDsp))         ++nDsp;
        else if (kind.hasType (kPairing))     ++nPairing;
        else { grund = "unknown child " + kind.getType().toString(); return false; }
    }
    if (nCommon != 1)  { grund = "Common is missing or duplicated"; return false; }
    if (nMain > 1 || nParam > 1 || nDsp > 1 || nPairing > 1) { grund = "duplicated child"; return false; }
    if (nDsp > 0)      { grund = "child Dsp is not readable by this version (SONDE-015)"; return false; }
    if (nPairing > 0)  { grund = "child Pairing is not readable by this version (SONDE-016)"; return false; }

    const auto common = v.getChildWithName (kCommon);
    if (! schemaIst (common, kCommonSchema))
    {
        grund = "Common schema " + common.getProperty (kSchema).toString() + " is unknown to this version (it reads schema 1)";
        return false;
    }

    Common c;
    const auto id = common.getProperty (kInstanceId);
    if (! id.isString() || id.toString().isEmpty()) { grund = "Common.instance_id is missing"; return false; }
    c.instanceId = id.toString();

    const auto kindWort = common.getProperty (kKind);
    if (! kindWort.isString() || ! klasseAusWort (kindWort.toString(), c.klasse))
    {
        grund = "Common.plugin_kind unknown: " + kindWort.toString(); return false;
    }
    const auto posWort = common.getProperty (kPosition);
    if (! posWort.isString() || ! positionAusWort (posWort.toString(), c.position))
    {
        grund = "Common.measurement_position unknown: " + posWort.toString(); return false;
    }
    if (! bundle.erlaubt (c.klasse))
    {
        grund = juce::String ("plugin_kind ") + wort (c.klasse) + " does not belong to this bundle"; return false;
    }
    if (! positionErlaubt (c.klasse, c.position))
    {
        grund = juce::String ("measurement_position ") + wort (c.position) + " is not allowed for " + wort (c.klasse); return false;
    }

    const auto label = common.getProperty (kLabel);
    if (common.hasProperty (kLabel) && ! label.isString()) { grund = "Common.label is not a string"; return false; }
    c.label = label.toString();
    const auto pair = common.getProperty (kPairId);
    if (common.hasProperty (kPairId) && (! pair.isString() || pair.toString().isEmpty()))
    {
        grund = "Common.pair_id must be a non-empty string"; return false;
    }
    c.pairId = pair.toString();
    const auto binding = common.getProperty (kBinding);
    if (common.hasProperty (kBinding) && (! binding.isString() || ! istHex32 (binding.toString())))
    {
        grund = "Common.project_binding_id must be lowercase hex32"; return false;
    }
    c.projectBindingId = binding.toString();

    // Kind-Matrix (§2.1 des Vertrags).
    const bool istMain = c.klasse == Klasse::main;
    const bool istAktiv = c.klasse == Klasse::active_probe;
    if (istMain && nMain != 1)   { grund = "main requires exactly one MainProject"; return false; }
    if (! istMain && nMain > 0)  { grund = juce::String ("MainProject is not allowed for ") + wort (c.klasse); return false; }
    if (istAktiv && nParam != 1) { grund = "active_probe requires exactly one Parameters"; return false; }
    if (! istAktiv && nParam > 0){ grund = juce::String ("Parameters is not allowed for ") + wort (c.klasse); return false; }

    if (istMain && ! schemaIst (v.getChildWithName (kMainProject), kMainSchema))
    {
        grund = "MainProject schema is unknown to this version (it reads schema 1)"; return false;
    }

    parameter::Satz satz {};
    if (istAktiv)
    {
        const auto p = v.getChildWithName (kParameters);
        if (! schemaIst (p, kParamSchema)) { grund = "Parameters schema is unknown to this version (it reads schema 1)"; return false; }
        if (! parameter::leseAusBaum (p, satz, grund)) return false;
    }

    aus.baum = v;
    aus.common = c;
    aus.hatParameters = istAktiv;
    aus.parameters = satz;
    aus.nurLesen = false;
    aus.originalBytes.reset();
    aus.grund.clear();
    aus.fremdesMajor = 0;
    return true;
}

} // namespace

Zustand frisch (const juce::String& instanceId)
{
    Zustand z;
    z.common.instanceId = instanceId;
    z.common.klasse = Klasse::legacy;
    z.common.position = Messposition::insert;
    z.baum = juce::ValueTree (kRoot);
    z.baum.setProperty (kSchema, kRootSchema, nullptr);
    z.baum.appendChild (baueCommon (z.common), nullptr);
    z.herkunft = Herkunft::frisch;
    return z;
}

bool migriereSchema1 (const juce::ValueTree& alt, juce::ValueTree& neu, juce::String& grund)
{
    if (! alt.hasType (kAltRoot)) { grund = "not an EqCopilotState"; return false; }
    if (! schemaIst (alt, 1))
    {
        grund = "EqCopilotState schema " + alt.getProperty (kSchema).toString() + " is not schema 1";
        return false;
    }

    // §32.2: hub -> main+insert, sensor -> legacy+insert, pre -> legacy+pre,
    // post -> legacy+post. Fehlende Rolle liest der Schema-1-Leser seit 0.1
    // als "sensor" - das bleibt so; ein UNBEKANNTES Wort ist nicht migrierbar.
    const auto rolle = alt.getProperty (kRole, "sensor").toString();
    Common c;
    if (! ausV2Rolle (rolle, c.klasse, c.position))
    {
        grund = "schema-1 role unknown: " + rolle;
        return false;
    }

    c.instanceId = alt.getProperty (kSensorId, "").toString();
    if (c.instanceId.isEmpty())
        c.instanceId = juce::Uuid().toString();   // einzige nicht-deterministische Stelle: kaputter Altstand
    c.label  = alt.getProperty (kLabel, "").toString();
    c.pairId = alt.getProperty (kPairId, "").toString();
    // project_binding_id wird NICHT erfunden (§32.2).

    neu = juce::ValueTree (kRoot);
    neu.setProperty (kSchema, kRootSchema, nullptr);
    neu.appendChild (baueCommon (c), nullptr);
    if (c.klasse == Klasse::main)
    {
        juce::ValueTree mp (kMainProject);
        mp.setProperty (kSchema, kMainSchema, nullptr);
        neu.appendChild (mp, nullptr);
    }
    return true;
}

LadeErgebnis lade (const void* daten, size_t laenge, const Bundle& bundle, Zustand& aus)
{
    auto nurLesen = [&] (const juce::String& grund, const juce::ValueTree& baum) -> LadeErgebnis
    {
        Zustand z;
        z.baum = baum;
        z.nurLesen = true;
        z.originalBytes = juce::MemoryBlock (daten, laenge);
        z.grund = grund;
        z.herkunft = Herkunft::nurLesen;
        z.fremdesMajor = schemaLesbar (baum);
        aus = z;
        return LadeErgebnis::nurLesen;
    };

    const auto bytePruefung = pruefeValueTreeBytes (daten, laenge);
    if (bytePruefung == BytePruefung::ungueltig)
        return LadeErgebnis::ignoriert;
    if (bytePruefung == BytePruefung::bekannteWurzelNichtVerlustfrei)
        return nurLesen ("variant marker is not losslessly readable by this JUCE version", {});

    juce::ValueTree v;
    try
    {
        v = juce::ValueTree::readFromData (daten, laenge);
    }
    catch (const std::bad_alloc&)
    {
        return LadeErgebnis::ignoriert;
    }
    catch (...)
    {
        return LadeErgebnis::ignoriert;
    }
    if (! v.isValid())
        return LadeErgebnis::ignoriert;

    if (v.hasType (kAltRoot))
    {
        juce::ValueTree neu;
        juce::String grund;
        if (! migriereSchema1 (v, neu, grund))
            return nurLesen (grund, v);
        Zustand z;
        if (! leseSchema2 (neu, bundle, z, grund))
            return nurLesen ("migration did not yield a readable state: " + grund, v);
        if (! hatWriterHeadroom (z, bundle))
            return nurLesen ("state leaves no bounded headroom for a losslessly reloadable save", v);
        z.herkunft = Herkunft::schema1Migriert;
        aus = z;
        return LadeErgebnis::migriert;
    }

    if (v.hasType (kRoot))
    {
        Zustand z;
        juce::String grund;
        if (! leseSchema2 (v, bundle, z, grund))
            return nurLesen (grund, v);
        if (! hatWriterHeadroom (z, bundle))
            return nurLesen ("state leaves no bounded headroom for a losslessly reloadable save", v);
        z.herkunft = Herkunft::schema2Geladen;
        aus = z;
        return LadeErgebnis::geladen;
    }

    return LadeErgebnis::ignoriert;
}

void speichere (const Zustand& z, juce::MemoryBlock& aus)
{
    aus.reset();
    if (z.nurLesen)
    {
        aus = z.originalBytes;
        return;
    }
    juce::MemoryOutputStream strom (aus, false);
    synchronisiert (z).writeToStream (strom);
    strom.flush();
}

juce::String v2Rolle (const Common& c)
{
    if (c.klasse == Klasse::main)
        return "hub";
    switch (c.position)
    {
        case Messposition::pre:  return "pre";
        case Messposition::post: return "post";
        default:                 return "sensor";
    }
}

bool ausV2Rolle (const juce::String& rolle, Klasse& klasse, Messposition& position)
{
    if (rolle == "hub")    { klasse = Klasse::main;   position = Messposition::insert; return true; }
    if (rolle == "sensor") { klasse = Klasse::legacy; position = Messposition::insert; return true; }
    if (rolle == "pre")    { klasse = Klasse::legacy; position = Messposition::pre;    return true; }
    if (rolle == "post")   { klasse = Klasse::legacy; position = Messposition::post;   return true; }
    return false;
}

juce::String alsText (const Zustand& z)
{
    if (z.nurLesen)
        return "read-only (" + z.grund + ")\n" + (z.baum.isValid() ? z.baum.toXmlString() : juce::String ("<kein Baum>"));
    return synchronisiert (z).toXmlString();
}

} // namespace nakama::state

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
