#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 — keine JucePlugin_*-Konstante im Kern
#include "NakamaPreset.h"
#include "NakamaKanon.h"
#include "NakamaVertrag.h"

#include <cmath>

namespace nakama::preset
{

const juce::StringArray& verboteneFelder()
{
    /*  Die sechs Namen aus `nakama-preset-v1.json`. Nicht "Identitaetsfelder"
        allgemein, sondern GENAU diese sechs: die Regel unterscheidet zwischen
        einem unbekannten Feld (ignorieren) und einem verbotenen (ablehnen),
        und dieser Unterschied braucht eine geschlossene Menge. */
    static const juce::StringArray namen {
        "adresse", "instance_id", "host_bus_name", "label", "eq_enabled", "undo_ring"
    };
    return namen;
}

bool gehoertInsPreset (int index) noexcept
{
    return index >= 0 && index < parameter::kAnzahl && index != parameter::kIndexEqEnabled;
}

namespace
{

const kanon::Wert* finde (const kanon::Wert& o, const juce::String& k)
{
    for (size_t i = 0; i < o.objektSchluessel.size(); ++i)
        if (o.objektSchluessel[i] == k)
            return &o.objektWerte[i];
    return nullptr;
}

} // namespace

bool schreibe (const parameter::DspSatz& dto, juce::MemoryBlock& utf8, juce::String& grund)
{
    // Ein Preset traegt nur einen Zustand, der auch bestaetigt werden koennte.
    juce::String wo;
    if (! parameter::validiere (dto, grund, wo))
        return false;

    const auto& t = parameter::tabelle();
    auto parameters = kanon::Wert::leeresObjekt();
    for (int i = 0; i < parameter::kAnzahl; ++i)
    {
        if (! gehoertInsPreset (i))
            continue;
        const auto& b = t[(size_t) i];
        const auto& z = dto.werte[(size_t) i];
        kanon::Wert w;
        switch (b.typ)
        {
            case parameter::Typ::boolean:     w = kanon::Wert::boolean (z.b); break;
            case parameter::Typ::gleitkomma:  w = kanon::Wert::nummer (z.zahl); break;
            case parameter::Typ::aufzaehlung: w = kanon::Wert::zeichen (b.werte[z.enumIndex]); break;
        }
        parameters.objektSetze (b.id, std::move (w));
    }

    auto zonen = kanon::Wert::leereListe();
    for (const auto& e : dto.zonen)
    {
        auto o = kanon::Wert::leeresObjekt();
        o.objektSetze ("enabled", kanon::Wert::boolean (e.enabled));
        o.objektSetze ("high_hz", kanon::Wert::nummer (e.highHz));
        o.objektSetze ("id",      kanon::Wert::nummer ((double) e.id));
        o.objektSetze ("low_hz",  kanon::Wert::nummer (e.lowHz));
        zonen.liste.push_back (std::move (o));
    }

    auto wurzel = kanon::Wert::leeresObjekt();
    wurzel.objektSetze ("dsp_schema_version", kanon::Wert::nummer ((double) parameter::kDspSchemaVersion));
    wurzel.objektSetze ("parameters", std::move (parameters));
    wurzel.objektSetze ("preset_schema_version", kanon::Wert::nummer ((double) kPresetSchemaVersion));
    wurzel.objektSetze ("schutz_zonen", std::move (zonen));

    juce::String fehler;
    if (! kanon::kanonisiere (wurzel, utf8, fehler))
    {
        grund = "nichtendlich";
        return false;
    }
    return true;
}

bool lies (const void* utf8, size_t laenge, parameter::DspSatz& ausInOut,
           juce::String& grund, juce::String& detail)
{
    grund.clear(); detail.clear();

    // 1. Byte-/Textriegel. Wie beim DSP-DTO OHNE den 15-Ziffern-Riegel des
    //    binary64-Schemawegs: ein Preset traegt dieselben exakten Werte
    //    (q = 0.7071067811865476).
    if (! vertrag::textriegelBytes (utf8, laenge, detail, false))
    {
        if (detail.startsWith ("Exponent ausserhalb") || detail.startsWith ("Zahl ausserhalb"))
            grund = "nichtendlich";
        else if (detail.startsWith ("Ganzzahl ausserhalb"))
            grund = "bereich";
        else
            grund = "kein_json";
        return false;
    }

    const juce::String text = juce::String::fromUTF8 ((const char*) utf8, (int) laenge);
    kanon::Wert wurzel;
    if (! kanon::lies (text, wurzel, detail))
    {
        grund = detail.startsWith ("doppelter Schluessel") ? "doppelter_schluessel" : "kein_json";
        return false;
    }
    if (wurzel.art != kanon::Wert::Art::objekt)
    {
        grund = "struktur"; detail = "Wurzel ist kein Objekt";
        return false;
    }

    // 2. Verbotene Felder - FRUEH, bevor irgendein Inhalt gedeutet wird. Ein
    //    Preset, das die Verdrahtung setzen will, wird abgewiesen und nicht
    //    teilweise gelesen.
    for (const auto& name : verboteneFelder())
        if (finde (wurzel, name) != nullptr)
        {
            grund = "verbotenes_feld"; detail = name;
            return false;
        }

    // 3. Die zwei Versionen. `preset_schema_version` sagt, wie die Datei
    //    gebaut ist; `dsp_schema_version`, welches Layout ihr Inhalt spricht.
    const auto* presetVersion = finde (wurzel, "preset_schema_version");
    if (presetVersion == nullptr || presetVersion->art != kanon::Wert::Art::zahl
        || presetVersion->zahl != (double) kPresetSchemaVersion)
    {
        grund = "preset_schema_version"; detail = "erwartet " + juce::String (kPresetSchemaVersion);
        return false;
    }
    const auto* dspVersion = finde (wurzel, "dsp_schema_version");
    if (dspVersion == nullptr || dspVersion->art != kanon::Wert::Art::zahl
        || dspVersion->zahl != (double) parameter::kDspSchemaVersion)
    {
        grund = "dsp_schema_version"; detail = "erwartet " + juce::String (parameter::kDspSchemaVersion);
        return false;
    }

    const auto* par   = finde (wurzel, "parameters");
    const auto* zonen = finde (wurzel, "schutz_zonen");
    if (par == nullptr || par->art != kanon::Wert::Art::objekt)
    {
        grund = "struktur"; detail = "parameters fehlt oder ist kein Objekt";
        return false;
    }
    if (zonen == nullptr || zonen->art != kanon::Wert::Art::liste)
    {
        grund = "struktur"; detail = "schutz_zonen fehlt oder ist kein Array";
        return false;
    }
    // Jedes WEITERE Top-Level-Feld ist additiv und wird ignoriert - keine
    // Pruefung auf die exakte Schluesselmenge der Wurzel.

    // 4. `parameters` ist dagegen EXAKT: unbekannt und fehlend fallen beide.
    //    `v2.global.eq_enabled` faellt hier als unbekannter Schluessel - es hat
    //    im Preset kein Feld, auch nicht innerhalb von `parameters`.
    const auto& t = parameter::tabelle();
    for (const auto& k : par->objektSchluessel)
    {
        const int i = parameter::indexVonId (k);
        if (i < 0 || ! gehoertInsPreset (i))
        {
            grund = "unbekannter_schluessel"; detail = k;
            return false;
        }
    }
    for (int i = 0; i < parameter::kAnzahl; ++i)
        if (gehoertInsPreset (i) && finde (*par, t[(size_t) i].id) == nullptr)
        {
            grund = "fehlender_schluessel"; detail = t[(size_t) i].id;
            return false;
        }

    // 5. Typ je Parameter - ALLE, bevor ein Wert geprueft wird.
    parameter::DspSatz s = ausInOut;   // eq_enabled bleibt, was es war
    for (int i = 0; i < parameter::kAnzahl; ++i)
    {
        if (! gehoertInsPreset (i))
            continue;
        const auto& b = t[(size_t) i];
        const auto* w = finde (*par, b.id);
        auto& z = s.werte[(size_t) i];
        switch (b.typ)
        {
            case parameter::Typ::boolean:
                if (w->art != kanon::Wert::Art::boolean) { grund = "typ"; detail = b.id; return false; }
                z.b = w->b;
                break;
            case parameter::Typ::gleitkomma:
                if (w->art != kanon::Wert::Art::zahl) { grund = "typ"; detail = b.id; return false; }
                z.zahl = w->zahl;
                break;
            case parameter::Typ::aufzaehlung:
                if (w->art != kanon::Wert::Art::text) { grund = "typ"; detail = b.id; return false; }
                z.enumIndex = b.werte.indexOf (w->text);
                break;
        }
    }

    juce::String wo;
    if (! parameter::validiere (s.werte, grund, wo))
    {
        detail = wo;
        return false;
    }

    // 6. Zonen: dieselbe Form und dieselben Regeln wie im DSP-DTO.
    s.zonen.clear();
    for (size_t i = 0; i < zonen->liste.size(); ++i)
    {
        const auto& e = zonen->liste[i];
        const juce::String woZone = "schutz_zonen[" + juce::String ((int) i) + "]";
        if (e.art != kanon::Wert::Art::objekt || e.objektSchluessel.size() != 4)
        {
            grund = "struktur"; detail = woZone + " braucht genau enabled, high_hz, id, low_hz";
            return false;
        }
        const auto* enabled = finde (e, "enabled");
        const auto* highHz  = finde (e, "high_hz");
        const auto* id      = finde (e, "id");
        const auto* lowHz   = finde (e, "low_hz");
        if (enabled == nullptr || highHz == nullptr || id == nullptr || lowHz == nullptr)
        {
            grund = "struktur"; detail = woZone + " braucht genau enabled, high_hz, id, low_hz";
            return false;
        }
        if (enabled->art != kanon::Wert::Art::boolean) { grund = "typ"; detail = woZone + ".enabled"; return false; }
        if (id->art      != kanon::Wert::Art::zahl)    { grund = "typ"; detail = woZone + ".id";      return false; }
        if (lowHz->art   != kanon::Wert::Art::zahl)    { grund = "typ"; detail = woZone + ".low_hz";  return false; }
        if (highHz->art  != kanon::Wert::Art::zahl)    { grund = "typ"; detail = woZone + ".high_hz"; return false; }
        if (! std::isfinite (id->zahl) || id->zahl != std::floor (id->zahl))
        {
            grund = "bereich"; detail = woZone + ".id";
            return false;
        }
        s.zonen.push_back ({ (int) id->zahl, lowHz->zahl, highHz->zahl, enabled->b });
    }
    if (! parameter::validiereZonen (s.zonen, grund, wo))
    {
        detail = wo;
        return false;
    }

    ausInOut = std::move (s);
    return true;
}

} // namespace nakama::preset

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
