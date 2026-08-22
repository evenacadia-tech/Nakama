#include "NakamaParameter.h"
#include "NakamaKanon.h"
#include "NakamaVertrag.h"

#include <cmath>

namespace nakama::parameter
{

namespace
{

Beschreibung boolean (const juce::String& id, bool standard, bool topologisch)
{
    Beschreibung b;
    b.id = id; b.typ = Typ::boolean; b.standardBool = standard;
    b.wechsel = Wechsel::blockrand; b.topologisch = topologisch;
    return b;
}

Beschreibung gleitkomma (const juce::String& id, double min, double max, double standard)
{
    Beschreibung b;
    b.id = id; b.typ = Typ::gleitkomma; b.min = min; b.max = max; b.standardZahl = standard;
    b.wechsel = Wechsel::rampe; b.topologisch = false;
    return b;
}

Beschreibung aufzaehlung (const juce::String& id, const juce::StringArray& werte, int standardIndex, bool topologisch)
{
    Beschreibung b;
    b.id = id; b.typ = Typ::aufzaehlung; b.werte = werte; b.standardIndex = standardIndex;
    b.wechsel = Wechsel::blockrand; b.topologisch = topologisch;
    return b;
}

std::array<Beschreibung, kAnzahl> baueTabelle()
{
    // Spiegel von nakama-parameter-v1.json - Reihenfolge und Werte sind Vertrag.
    std::array<Beschreibung, kAnzahl> t;
    int i = 0;
    t[i++] = boolean    ("v1.global.bypass", false, false);
    t[i++] = gleitkomma ("v1.global.input_trim_db",  -24.0, 24.0, 0.0);
    t[i++] = gleitkomma ("v1.global.output_trim_db", -24.0, 24.0, 0.0);
    t[i++] = gleitkomma ("v1.global.width",            0.0,  2.0, 1.0);
    t[i++] = gleitkomma ("v1.global.mono_bass_hz",     0.0, 500.0, 0.0);

    const juce::StringArray typen        { "bell", "low_shelf", "high_shelf", "notch", "low_cut", "high_cut" };
    const juce::StringArray kanalmodi    { "stereo", "left", "right", "mid", "side" };
    const juce::StringArray sidechains   { "none", "internal", "priority_sidechain" };

    for (int slot = 0; slot < kSlots; ++slot)
    {
        const juce::String p = "v1.band." + juce::String (slot) + ".";
        t[i++] = boolean    (p + "enabled", false, false);
        t[i++] = aufzaehlung (p + "type", typen, 0, true);
        t[i++] = gleitkomma (p + "freq_hz",          20.0, 20000.0, 1000.0);
        t[i++] = gleitkomma (p + "q",                 0.15,    24.0,    1.0);
        t[i++] = gleitkomma (p + "gain_db",         -12.0,    12.0,    0.0);
        t[i++] = aufzaehlung (p + "channel_mode", kanalmodi, 0, true);
        t[i++] = boolean    (p + "dynamic_enabled", false, true);
        t[i++] = gleitkomma (p + "dynamic_range_db", -12.0,   12.0,    0.0);
        t[i++] = gleitkomma (p + "threshold_db",     -60.0,    0.0,  -20.0);
        t[i++] = gleitkomma (p + "attack_ms",          0.1,  500.0,   10.0);
        t[i++] = gleitkomma (p + "hold_ms",            0.0,  500.0,    0.0);
        t[i++] = gleitkomma (p + "release_ms",         5.0, 5000.0,  100.0);
        t[i++] = aufzaehlung (p + "sidechain_source", sidechains, 0, true);
    }
    jassert (i == kAnzahl);
    return t;
}

} // namespace

const std::array<Beschreibung, kAnzahl>& tabelle()
{
    static const std::array<Beschreibung, kAnzahl> t = baueTabelle();
    return t;
}

int indexVonId (const juce::String& id)
{
    const auto& t = tabelle();
    for (int i = 0; i < kAnzahl; ++i)
        if (t[(size_t) i].id == id)
            return i;
    return -1;
}

Satz standardSatz()
{
    Satz s;
    const auto& t = tabelle();
    for (int i = 0; i < kAnzahl; ++i)
    {
        const auto& b = t[(size_t) i];
        auto& z = s[(size_t) i];
        switch (b.typ)
        {
            case Typ::boolean:     z.b = b.standardBool; break;
            case Typ::gleitkomma:  z.zahl = b.standardZahl; break;
            case Typ::aufzaehlung: z.enumIndex = b.standardIndex; break;
        }
    }
    return s;
}

bool validiere (const Satz& s, juce::String& grund, juce::String& woId)
{
    const auto& t = tabelle();
    for (int i = 0; i < kAnzahl; ++i)
    {
        const auto& b = t[(size_t) i];
        const auto& z = s[(size_t) i];
        woId = b.id;
        switch (b.typ)
        {
            case Typ::boolean: break;
            case Typ::gleitkomma:
                if (! std::isfinite (z.zahl))           { grund = "nichtendlich"; return false; }
                if (z.zahl < b.min || z.zahl > b.max)   { grund = "bereich";      return false; }
                break;
            case Typ::aufzaehlung:
                if (z.enumIndex < 0 || z.enumIndex >= b.werte.size()) { grund = "enum"; return false; }
                break;
        }
    }
    woId.clear();
    return true;
}

// ── DTO ────────────────────────────────────────────────────────────────────

namespace
{

kanon::Wert dtoWert (const Satz& s)
{
    const auto& t = tabelle();
    auto parameters = kanon::Wert::leeresObjekt();
    for (int i = 0; i < kAnzahl; ++i)
    {
        const auto& b = t[(size_t) i];
        const auto& z = s[(size_t) i];
        kanon::Wert w;
        switch (b.typ)
        {
            case Typ::boolean:     w = kanon::Wert::boolean (z.b); break;
            case Typ::gleitkomma:  w = kanon::Wert::nummer (z.zahl); break;
            case Typ::aufzaehlung: w = kanon::Wert::zeichen (b.werte[z.enumIndex]); break;
        }
        parameters.objektSetze (b.id, std::move (w));
    }
    auto dto = kanon::Wert::leeresObjekt();
    dto.objektSetze ("dsp_schema_version", kanon::Wert::nummer ((double) kDspSchemaVersion));
    dto.objektSetze ("parameters", std::move (parameters));
    return dto;
}

} // namespace

bool dtoKanon (const Satz& s, juce::MemoryBlock& utf8, juce::String& grund)
{
    juce::String wo;
    if (! validiere (s, grund, wo))
        return false;
    juce::String fehler;
    if (! kanon::kanonisiere (dtoWert (s), utf8, fehler))
    {
        grund = "nichtendlich";
        return false;
    }
    return true;
}

bool stateHash (const Satz& s, juce::String& hex, juce::String& grund)
{
    juce::MemoryBlock utf8;
    if (! dtoKanon (s, utf8, grund))
        return false;
    hex = kanon::sha256Hex (utf8.getData(), utf8.getSize());
    return true;
}

bool ausDtoText (const void* utf8, size_t laenge, Satz& aus, juce::String& grund, juce::String& detail)
{
    grund.clear(); detail.clear();

    // 1. Byte-/Textriegel des v3-Vertrags (textriegelBytes faehrt beide Stufen):
    //    BOM, UTF-8, fuehrende Nullen, Zahlenbereich AUS DEM LITERAL, Escapes,
    //    NUL, Surrogate, leere Schluessel.
    if (! vertrag::textriegelBytes (utf8, laenge, detail))
    {
        // Ein Literal jenseits von 1e308 liest Python als inf, serde_json lehnt
        // ab - fuer das DTO ist beides "nichtendlich". Eine Ganzzahl jenseits
        // 2^53 liegt ausserhalb jedes Parameterbereichs.
        if (detail.startsWith ("Exponent ausserhalb") || detail.startsWith ("Zahl ausserhalb"))
            grund = "nichtendlich";
        else if (detail.startsWith ("Ganzzahl ausserhalb"))
            grund = "bereich";
        else
            grund = "kein_json";
        return false;
    }

    // 2. Der eigene Leser (kanon::lies): korrekt gerundete Zahlen, doppelte
    //    Schluessel als eigener Fehler.
    const juce::String text = juce::String::fromUTF8 ((const char*) utf8, (int) laenge);
    kanon::Wert wurzel;
    if (! kanon::lies (text, wurzel, detail))
    {
        grund = detail.startsWith ("doppelter Schluessel") ? "doppelter_schluessel" : "kein_json";
        return false;
    }

    // 3. Struktur: genau {dsp_schema_version, parameters}.
    auto finde = [] (const kanon::Wert& o, const juce::String& k) -> const kanon::Wert*
    {
        for (size_t i = 0; i < o.objektSchluessel.size(); ++i)
            if (o.objektSchluessel[i] == k)
                return &o.objektWerte[i];
        return nullptr;
    };
    if (wurzel.art != kanon::Wert::Art::objekt) { grund = "struktur"; detail = "Wurzel ist kein Objekt"; return false; }
    const auto* version = finde (wurzel, "dsp_schema_version");
    const auto* par     = finde (wurzel, "parameters");
    if (wurzel.objektSchluessel.size() != 2 || version == nullptr || par == nullptr)
    {
        grund = "struktur"; detail = "Wurzel braucht genau dsp_schema_version und parameters";
        return false;
    }

    // 4. dsp_schema_version.
    if (version->art != kanon::Wert::Art::zahl || version->zahl != (double) kDspSchemaVersion)
    {
        grund = "dsp_schema_version"; detail = "erwartet " + juce::String (kDspSchemaVersion);
        return false;
    }
    if (par->art != kanon::Wert::Art::objekt) { grund = "struktur"; detail = "parameters ist kein Objekt"; return false; }

    // 5. Unbekannte Schluessel.
    for (const auto& k : par->objektSchluessel)
        if (indexVonId (k) < 0)
        {
            grund = "unbekannter_schluessel"; detail = k;
            return false;
        }

    // 6. Fehlende Schluessel.
    const auto& t = tabelle();
    for (int i = 0; i < kAnzahl; ++i)
        if (finde (*par, t[(size_t) i].id) == nullptr)
        {
            grund = "fehlender_schluessel"; detail = t[(size_t) i].id;
            return false;
        }

    // 7. Typ je Parameter - ALLE, bevor ein Wert geprueft wird.
    Satz s;
    for (int i = 0; i < kAnzahl; ++i)
    {
        const auto& b = t[(size_t) i];
        const auto* w = finde (*par, b.id);
        auto& z = s[(size_t) i];
        switch (b.typ)
        {
            case Typ::boolean:
                if (w->art != kanon::Wert::Art::boolean) { grund = "typ"; detail = b.id; return false; }
                z.b = w->b;
                break;
            case Typ::gleitkomma:
                if (w->art != kanon::Wert::Art::zahl) { grund = "typ"; detail = b.id; return false; }
                z.zahl = w->zahl;
                break;
            case Typ::aufzaehlung:
                if (w->art != kanon::Wert::Art::text) { grund = "typ"; detail = b.id; return false; }
                z.enumIndex = b.werte.indexOf (w->text);
                break;
        }
    }

    // 8./9. Nichtendlich, Bereich, Enum.
    juce::String wo;
    if (! validiere (s, grund, wo))
    {
        detail = wo;
        return false;
    }
    aus = s;
    return true;
}

// ── ValueTree ──────────────────────────────────────────────────────────────

void schreibeInBaum (const Satz& s, juce::ValueTree& parameters)
{
    const auto& t = tabelle();
    for (int i = 0; i < kAnzahl; ++i)
    {
        const auto& b = t[(size_t) i];
        const auto& z = s[(size_t) i];
        const juce::Identifier id (b.id);
        switch (b.typ)
        {
            case Typ::boolean:     parameters.setProperty (id, z.b, nullptr); break;
            case Typ::gleitkomma:  parameters.setProperty (id, z.zahl, nullptr); break;
            case Typ::aufzaehlung:
            {
                const int idx = juce::jlimit (0, b.werte.size() - 1, z.enumIndex);
                parameters.setProperty (id, b.werte[idx], nullptr);
                break;
            }
        }
    }
}

bool leseAusBaum (const juce::ValueTree& parameters, Satz& aus, juce::String& grund)
{
    const auto& t = tabelle();
    Satz s;
    for (int i = 0; i < kAnzahl; ++i)
    {
        const auto& b = t[(size_t) i];
        const juce::Identifier id (b.id);
        if (! parameters.hasProperty (id)) { grund = "Parameters: " + b.id + " fehlt"; return false; }
        const auto w = parameters.getProperty (id);
        auto& z = s[(size_t) i];
        switch (b.typ)
        {
            case Typ::boolean:
                if (! w.isBool()) { grund = "Parameters: " + b.id + " ist kein bool"; return false; }
                z.b = (bool) w;
                break;
            case Typ::gleitkomma:
                if (! w.isDouble()) { grund = "Parameters: " + b.id + " ist kein double"; return false; }
                z.zahl = (double) w;
                break;
            case Typ::aufzaehlung:
            {
                if (! w.isString()) { grund = "Parameters: " + b.id + " ist kein Enumwort"; return false; }
                const int idx = b.werte.indexOf (w.toString());
                if (idx < 0) { grund = "Parameters: " + b.id + " unbekanntes Enumwort " + w.toString(); return false; }
                z.enumIndex = idx;
                break;
            }
        }
    }
    juce::String wo;
    if (! validiere (s, grund, wo))
    {
        grund = "Parameters: " + wo + " " + grund;
        return false;
    }
    aus = s;
    return true;
}

} // namespace nakama::parameter
