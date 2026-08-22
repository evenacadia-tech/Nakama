#include "NakamaState.h"

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

bool positionErlaubt (Klasse k, Messposition p)
{
    switch (k)
    {
        case Klasse::main:          return p == Messposition::insert;
        case Klasse::legacy:        return p != Messposition::post_fader_contribution;
        case Klasse::passive_probe: return true;
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
    if (common.hasProperty (kPairId) && ! pair.isString()) { grund = "Common.pair_id is not a string"; return false; }
    c.pairId = pair.toString();
    const auto binding = common.getProperty (kBinding);
    if (common.hasProperty (kBinding) && ! binding.isString()) { grund = "Common.project_binding_id is not a string"; return false; }
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
    const auto v = juce::ValueTree::readFromData (daten, laenge);
    if (! v.isValid())
        return LadeErgebnis::ignoriert;

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

    if (v.hasType (kAltRoot))
    {
        juce::ValueTree neu;
        juce::String grund;
        if (! migriereSchema1 (v, neu, grund))
            return nurLesen (grund, v);
        Zustand z;
        if (! leseSchema2 (neu, bundle, z, grund))
            return nurLesen ("migration did not yield a readable state: " + grund, v);
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
