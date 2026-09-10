#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 — keine JucePlugin_*-Konstante im Kern
#include "NakamaParameter.h"
#include "NakamaKanon.h"
#include "NakamaVertrag.h"

#include <cmath>
#include <cstring>

namespace nakama::parameter
{

namespace
{

Beschreibung boolean (const juce::String& id, bool standard, bool topologisch,
                      bool hostParameter = true, int layout = 1)
{
    Beschreibung b;
    b.id = id; b.typ = Typ::boolean; b.standardBool = standard;
    b.wechsel = Wechsel::blockrand; b.topologisch = topologisch;
    b.hostParameter = hostParameter; b.layout = layout;
    return b;
}

Beschreibung gleitkomma (const juce::String& id, double min, double max, double standard,
                         bool hostParameter = true, int layout = 1)
{
    Beschreibung b;
    b.id = id; b.typ = Typ::gleitkomma; b.min = min; b.max = max; b.standardZahl = standard;
    b.wechsel = Wechsel::rampe; b.topologisch = false;
    b.hostParameter = hostParameter; b.layout = layout;
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
    // Spiegel von nakama-parameter-v2.json - Reihenfolge und Werte sind
    // Vertrag: 109 v1, dann die drei v2-Host-Parameter, dann acht occupied.
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
    jassert (i == kAnzahlV1);

    // SONDE-015 R2/R3/R4: die drei v2-HOST-Parameter, hinten angehaengt.
    t[i++] = boolean    ("v2.global.eq_enabled", false, false, true, 2);
    t[i++] = gleitkomma ("v2.global.mix",   0.0, 1.0, 1.0, true, 2);
    t[i++] = boolean    ("v2.global.auto_gain", false, false, true, 2);
    jassert (i == kHostParameter);

    // SONDE-015 R5: die acht `occupied` - persistenter Zustand im Kind `Dsp`,
    // KEIN Host-Parameter. Topologisch: eine Belegung aendert die Bank.
    for (int slot = 0; slot < kSlots; ++slot)
        t[i++] = boolean ("v2.band." + juce::String (slot) + ".occupied", false, true, false, 2);

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

bool DspSatz::operator== (const DspSatz& a) const noexcept
{
    const auto& t = tabelle();
    for (int i = 0; i < kAnzahl; ++i)
    {
        const auto& x = werte[(size_t) i];
        const auto& y = a.werte[(size_t) i];
        switch (t[(size_t) i].typ)
        {
            // Gleitkomma BITGENAU: 0.0 und -0.0 sind zwei verschiedene
            // Zustaende dieses Vertrags (Migrationsregel R5).
            case Typ::gleitkomma:  if (std::memcmp (&x.zahl, &y.zahl, sizeof (double)) != 0) return false; break;
            case Typ::boolean:     if (x.b != y.b) return false; break;
            case Typ::aufzaehlung: if (x.enumIndex != y.enumIndex) return false; break;
        }
    }
    return zonen == a.zonen;
}

bool weichtVomDefaultAb (int index, const Zelle& z)
{
    if (index < 0 || index >= kAnzahl)
        return false;
    const auto& b = tabelle()[(size_t) index];
    switch (b.typ)
    {
        case Typ::boolean:     return z.b != b.standardBool;
        case Typ::aufzaehlung: return z.enumIndex != b.standardIndex;
        case Typ::gleitkomma:
            // BITGENAU (R5): ein Epsilonvergleich laese einen minimal
            // verstellten Wert als unberuehrt und verloere ein Band.
            return std::memcmp (&z.zahl, &b.standardZahl, sizeof (double)) != 0;
    }
    return false;
}

void setzeOccupiedAusV1 (Satz& s)
{
    for (int slot = 0; slot < kSlots; ++slot)
    {
        bool belegt = s[(size_t) indexBandV1 (slot, kEnabled)].b;
        for (int feld = 0; feld < kJeSlot && ! belegt; ++feld)
        {
            const int i = indexBandV1 (slot, feld);
            belegt = weichtVomDefaultAb (i, s[(size_t) i]);
        }
        s[(size_t) indexOccupied (slot)].b = belegt;
    }
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

bool validiereZonen (const std::vector<Schutzzone>& z, juce::String& grund, juce::String& woId)
{
    // 1. Anzahl. Die neunte Zone wird ABGEWIESEN, nicht stillschweigend
    //    verworfen - und die Begrenzung sitzt hier, nicht erst in einer
    //    Oberflaeche (R6, M-73).
    if ((int) z.size() > kMaxZonen)
    {
        grund = "zone_anzahl";
        woId = "schutz_zonen";
        return false;
    }

    // 2. Werte je Zone.
    for (size_t i = 0; i < z.size(); ++i)
    {
        const auto& e = z[i];
        const juce::String wo = "schutz_zonen[" + juce::String ((int) i) + "]";
        if (e.id < 0 || e.id >= kMaxZonen)      { grund = "bereich"; woId = wo + ".id";      return false; }
        if (! std::isfinite (e.lowHz) || ! std::isfinite (e.highHz))
        {
            grund = "nichtendlich";
            woId = wo + (std::isfinite (e.lowHz) ? ".high_hz" : ".low_hz");
            return false;
        }
        // 20 <= low < high <= 20000. Gleichheit ist eine Zone ueber nichts.
        // Die Nyquistkappung auf 0,45*fs macht die LAUFZEIT (R6, Abweichung 2):
        // eine samplerateabhaengige DTO-Grenze machte einen bei 96 kHz
        // gespeicherten Stand bei 44,1 kHz unlesbar.
        if (e.lowHz < 20.0 || e.lowHz >= e.highHz) { grund = "bereich"; woId = wo + ".low_hz";  return false; }
        if (e.highHz > 20000.0)                    { grund = "bereich"; woId = wo + ".high_hz"; return false; }
    }

    // 3. Doppelte id - zwei Zonen mit derselben Identitaet waeren zwei
    //    Wahrheiten ueber dieselbe Zone.
    for (size_t i = 0; i < z.size(); ++i)
        for (size_t j = i + 1; j < z.size(); ++j)
            if (z[i].id == z[j].id)
            {
                grund = "zone_doppelt";
                woId = "schutz_zonen[" + juce::String ((int) j) + "].id";
                return false;
            }

    // 4. Streng aufsteigend nach id. Eine Menge hat keine Reihenfolge; der
    //    Vertrag legt eine fest, damit dieselbe Zonenmenge immer denselben
    //    kanonischen Text und damit denselben state_hash ergibt.
    for (size_t i = 1; i < z.size(); ++i)
        if (z[i - 1].id >= z[i].id)
        {
            grund = "zone_sortierung";
            woId = "schutz_zonen[" + juce::String ((int) i) + "].id";
            return false;
        }

    woId.clear();
    return true;
}

bool validiere (const DspSatz& s, juce::String& grund, juce::String& woId)
{
    if (! validiere (s.werte, grund, woId))
        return false;
    return validiereZonen (s.zonen, grund, woId);
}

// ── DTO ────────────────────────────────────────────────────────────────────

namespace
{

kanon::Wert dtoWert (const DspSatz& s)
{
    const auto& t = tabelle();
    auto parameters = kanon::Wert::leeresObjekt();
    for (int i = 0; i < kAnzahl; ++i)
    {
        const auto& b = t[(size_t) i];
        const auto& z = s.werte[(size_t) i];
        kanon::Wert w;
        switch (b.typ)
        {
            case Typ::boolean:     w = kanon::Wert::boolean (z.b); break;
            case Typ::gleitkomma:  w = kanon::Wert::nummer (z.zahl); break;
            case Typ::aufzaehlung: w = kanon::Wert::zeichen (b.werte[z.enumIndex]); break;
        }
        parameters.objektSetze (b.id, std::move (w));
    }

    auto zonen = kanon::Wert::leereListe();
    for (const auto& e : s.zonen)
    {
        auto o = kanon::Wert::leeresObjekt();
        // Einfuegereihenfolge ist gleichgueltig - RFC 8785 sortiert selbst
        // (enabled < high_hz < id < low_hz nach UTF-16-Code-Units).
        o.objektSetze ("enabled", kanon::Wert::boolean (e.enabled));
        o.objektSetze ("high_hz", kanon::Wert::nummer (e.highHz));
        o.objektSetze ("id",      kanon::Wert::nummer ((double) e.id));
        o.objektSetze ("low_hz",  kanon::Wert::nummer (e.lowHz));
        zonen.liste.push_back (std::move (o));
    }

    auto dto = kanon::Wert::leeresObjekt();
    dto.objektSetze ("dsp_schema_version", kanon::Wert::nummer ((double) kDspSchemaVersion));
    dto.objektSetze ("parameters", std::move (parameters));
    dto.objektSetze ("schutz_zonen", std::move (zonen));
    return dto;
}

} // namespace

bool dtoKanon (const DspSatz& s, juce::MemoryBlock& utf8, juce::String& grund)
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

bool stateHash (const DspSatz& s, juce::String& hex, juce::String& grund)
{
    juce::MemoryBlock utf8;
    if (! dtoKanon (s, utf8, grund))
        return false;
    hex = kanon::sha256Hex (utf8.getData(), utf8.getSize());
    return true;
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

/** Liest die Zonenliste aus dem geparsten DTO. Struktur- und Typfehler tragen
    dieselben Manifestwoerter wie ueberall sonst; die INHALTLICHEN Regeln
    (Anzahl, Bereich, doppelte id, Sortierung) prueft danach
    `validiereZonen` - genau EINE Stelle, an der sie stehen. */
bool leseZonen (const kanon::Wert& liste, std::vector<Schutzzone>& aus,
                juce::String& grund, juce::String& detail)
{
    if (liste.art != kanon::Wert::Art::liste)
    {
        grund = "struktur"; detail = "schutz_zonen ist kein Array";
        return false;
    }
    aus.clear();
    aus.reserve (liste.liste.size());
    for (size_t i = 0; i < liste.liste.size(); ++i)
    {
        const auto& e = liste.liste[i];
        const juce::String wo = "schutz_zonen[" + juce::String ((int) i) + "]";
        if (e.art != kanon::Wert::Art::objekt)
        {
            grund = "struktur"; detail = wo + " ist kein Objekt";
            return false;
        }
        if (e.objektSchluessel.size() != 4)
        {
            grund = "struktur"; detail = wo + " braucht genau enabled, high_hz, id, low_hz";
            return false;
        }
        const auto* enabled = finde (e, "enabled");
        const auto* highHz  = finde (e, "high_hz");
        const auto* id      = finde (e, "id");
        const auto* lowHz   = finde (e, "low_hz");
        if (enabled == nullptr || highHz == nullptr || id == nullptr || lowHz == nullptr)
        {
            grund = "struktur"; detail = wo + " braucht genau enabled, high_hz, id, low_hz";
            return false;
        }
        if (enabled->art != kanon::Wert::Art::boolean) { grund = "typ"; detail = wo + ".enabled"; return false; }
        if (id->art      != kanon::Wert::Art::zahl)    { grund = "typ"; detail = wo + ".id";      return false; }
        if (lowHz->art   != kanon::Wert::Art::zahl)    { grund = "typ"; detail = wo + ".low_hz";  return false; }
        if (highHz->art  != kanon::Wert::Art::zahl)    { grund = "typ"; detail = wo + ".high_hz"; return false; }

        // Die `id` ist eine GANZE Zahl. JSON kennt den Unterschied nicht, der
        // Vertrag schon: 3.5 ist keine Zonenidentitaet, sondern ein Fehler.
        if (! std::isfinite (id->zahl) || id->zahl != std::floor (id->zahl))
        {
            grund = "bereich"; detail = wo + ".id";
            return false;
        }
        // D8 (Codeaudit 10.09.2026): der BEREICH am double, VOR jeder
        // Konvertierung. Eine ganze Zahl jenseits des int-Bereichs
        // (2147483648, 9007199254740991) in einen int zu wandeln waere
        // undefiniert; auf x64 ergibt sie INT_MIN, das `validiereZonen` nur
        // zufaellig ebenfalls abweist. Konvertiert wird deshalb gar nicht: die
        // id gilt genau dann, wenn sie einer der Slotnummern 0..7 GLEICHT.
        // Jede andere wird -1, und `validiereZonen` meldet sie an ihrer Stelle
        // der Pruefreihenfolge (nach der Anzahl, nakama-state-v2.md §4) als
        // `bereich` - mit derselben Fundstelle wie bisher.
        int ganzeId = -1;
        for (int k = 0; k < kMaxZonen; ++k)
            if (id->zahl == (double) k) { ganzeId = k; break; }
        Schutzzone z;
        z.enabled = enabled->b;
        z.id      = ganzeId;
        z.lowHz   = lowHz->zahl;
        z.highHz  = highHz->zahl;
        aus.push_back (z);
    }
    return true;
}

} // namespace

bool berichtDtoPruefen (const juce::String& jcs, const juce::String& stateHashHex,
                        DspSatz& aus, juce::String& grund, juce::String& detail)
{
    grund.clear(); detail.clear();
    const auto* bytes = jcs.toRawUTF8();
    const auto laenge = (size_t) jcs.getNumBytesAsUTF8();

    // Der Hash ZUERST: eine Zeichenkette, deren Hash nicht stimmt, ist kein
    // bestaetigter Zustand, und ihren Inhalt zu deuten waere gegenstandslos.
    const auto ist = kanon::sha256Hex (bytes, laenge);
    if (stateHashHex.isEmpty() || ist != stateHashHex)
    {
        grund = "state_hash";
        detail = "SHA-256(dsp.jcs) = " + ist + ", state_report.state_hash = " + stateHashHex;
        return false;
    }
    return ausDtoText (bytes, laenge, aus, grund, detail);
}

bool ausDtoText (const void* utf8, size_t laenge, DspSatz& aus, juce::String& grund, juce::String& detail)
{
    grund.clear(); detail.clear();

    // 1. Byte-/Textriegel des v3-Vertrags (textriegelBytes faehrt beide Stufen):
    //    BOM, UTF-8, fuehrende Nullen, Zahlenbereich AUS DEM LITERAL, Escapes,
    //    NUL, Surrogate, leere Schluessel.
    // Der DTO-Kanon besitzt danach einen eigenen korrekt gerundeten
    // Zahlenleser und eine feldgenaue Typ-/Bereichspruefung. Der zusaetzliche
    // 15-Ziffern-Riegel des binary64-Schemawegs ist hier deshalb weder noetig
    // noch zulaessig (z. B. Q = 0.7071067811865476).
    if (! vertrag::textriegelBytes (utf8, laenge, detail, false))
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

    // 3. Struktur: genau {dsp_schema_version, parameters, schutz_zonen}.
    if (wurzel.art != kanon::Wert::Art::objekt) { grund = "struktur"; detail = "Wurzel ist kein Objekt"; return false; }
    const auto* version = finde (wurzel, "dsp_schema_version");
    const auto* par     = finde (wurzel, "parameters");
    const auto* zonen   = finde (wurzel, "schutz_zonen");
    if (wurzel.objektSchluessel.size() != 3 || version == nullptr || par == nullptr || zonen == nullptr)
    {
        grund = "struktur"; detail = "Wurzel braucht genau dsp_schema_version, parameters und schutz_zonen";
        return false;
    }

    // 4. dsp_schema_version. Layout v1 ist Verlauf; dieser Leser nimmt es
    //    nicht mehr an - ein v1-STAND migriert im State-Leser, ein v1-DTO auf
    //    dem Draht ist eine andere Sprache.
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
    DspSatz s;
    for (int i = 0; i < kAnzahl; ++i)
    {
        const auto& b = t[(size_t) i];
        const auto* w = finde (*par, b.id);
        auto& z = s.werte[(size_t) i];
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

    // 8./9. Nichtendlich, Bereich, Enum - erst die Werte.
    juce::String wo;
    if (! validiere (s.werte, grund, wo))
    {
        detail = wo;
        return false;
    }

    // 10. Zonen: erst Struktur und Typ, dann die inhaltlichen Regeln.
    if (! leseZonen (*zonen, s.zonen, grund, detail))
        return false;
    if (! validiereZonen (s.zonen, grund, wo))
    {
        detail = wo;
        return false;
    }

    aus = std::move (s);
    return true;
}

// ── ValueTree ──────────────────────────────────────────────────────────────

namespace
{
const juce::Identifier kDspSchemaVersionId ("dsp_schema_version");
}

void schreibeInBaum (const Satz& s, juce::ValueTree& parameters)
{
    // Die LAYOUTVERSION der Werte in diesem Knoten. Sie ist additiv - ein
    // Build, der sie nicht kennt, liest die 109 v1-Eigenschaften weiter und
    // ignoriert den Rest (nakama-state-v2.md §2.1). Ohne sie waere "Layout
    // v1" von "Layout v2, in dem zufaellig alle v2-Werte fehlen" nicht zu
    // unterscheiden, und die Migration muesste raten.
    parameters.setProperty (kDspSchemaVersionId, kDspSchemaVersion, nullptr);

    const auto& t = tabelle();
    for (int i = 0; i < kHostParameter; ++i)
    {
        const auto& b = t[(size_t) i];
        const auto& z = s[(size_t) i];
        const juce::Identifier id (b.id);
        switch (b.typ)
        {
            case Typ::boolean:     parameters.setProperty (id, z.b, nullptr); break;
            case Typ::gleitkomma:
            {
                /*  SONDE-015, am eigenen Diff gefunden: `ValueTree::setProperty`
                    ueberspringt den Schreibvorgang, wenn der vorhandene Wert
                    `var::operator==` erfuellt - und fuer `var` sind 0.0 und
                    -0.0 GLEICH. Der Vertrag hier unterscheidet sie aber
                    bitgenau: die Migrationsregel R5 liest "weicht vom Default
                    ab" bitgenau, und `speichere(lade(x)) == x` soll fuer jeden
                    Wert gelten, den dieser Schreiber selbst geschrieben hat.
                    Ohne diesen Umweg verloere ein gespeicherter Stand das
                    Vorzeichen der Null, und ein belegter Slot faende sich beim
                    naechsten Laden als frei wieder.

                    Der Umweg ueber einen leeren `var` erzwingt den
                    Schreibvorgang, ohne die POSITION der Eigenschaft im Knoten
                    zu aendern (`NamedValueSet::set` ersetzt an Ort und
                    Stelle) - `removeProperty` haette sie ans Ende gehaengt und
                    die Bytegleichheit gebrochen. */
                const auto vorhanden = parameters.getProperty (id);
                bool bitGleich = false;
                if (vorhanden.isDouble())
                {
                    const double alt = (double) vorhanden;
                    bitGleich = std::memcmp (&alt, &z.zahl, sizeof (double)) == 0;
                }
                if (! bitGleich)
                {
                    if (vorhanden.isDouble())
                        parameters.setProperty (id, juce::var(), nullptr);
                    parameters.setProperty (id, z.zahl, nullptr);
                }
                break;
            }
            case Typ::aufzaehlung:
            {
                const int idx = juce::jlimit (0, b.werte.size() - 1, z.enumIndex);
                parameters.setProperty (id, b.werte[idx], nullptr);
                break;
            }
        }
    }
}

bool leseAusBaum (const juce::ValueTree& parameters, Satz& aus, bool& layoutV1, juce::String& grund)
{
    layoutV1 = false;
    int bis = kHostParameter;
    if (! parameters.hasProperty (kDspSchemaVersionId))
    {
        // Kein Versionsfeld = Layout v1: dieser Knoten wurde von einem Build
        // geschrieben, der die elf v2-Kennungen nicht kannte.
        layoutV1 = true;
        bis = kAnzahlV1;
    }
    else
    {
        const auto w = parameters.getProperty (kDspSchemaVersionId);
        if (! w.isInt() && ! w.isInt64())
        {
            grund = "Parameters: dsp_schema_version is not an integer";
            return false;
        }
        const auto version = (juce::int64) w;
        if (version == kDspSchemaVersionV1)
        {
            layoutV1 = true;
            bis = kAnzahlV1;
        }
        else if (version != kDspSchemaVersion)
        {
            grund = "Parameters dsp_schema_version " + juce::String (version)
                  + " is unknown to this version (it reads layout 1 and 2)";
            return false;
        }
    }

    const auto& t = tabelle();
    Satz s = standardSatz();
    for (int i = 0; i < bis; ++i)
    {
        const auto& b = t[(size_t) i];
        const juce::Identifier id (b.id);
        if (! parameters.hasProperty (id)) { grund = "Parameters: " + b.id + " is missing"; return false; }
        const auto w = parameters.getProperty (id);
        auto& z = s[(size_t) i];
        switch (b.typ)
        {
            case Typ::boolean:
                if (! w.isBool()) { grund = "Parameters: " + b.id + " is not a bool"; return false; }
                z.b = (bool) w;
                break;
            case Typ::gleitkomma:
                if (! w.isDouble()) { grund = "Parameters: " + b.id + " is not a double"; return false; }
                z.zahl = (double) w;
                break;
            case Typ::aufzaehlung:
            {
                if (! w.isString()) { grund = "Parameters: " + b.id + " is not an enum word"; return false; }
                const int idx = b.werte.indexOf (w.toString());
                if (idx < 0) { grund = "Parameters: " + b.id + " unknown enum word " + w.toString(); return false; }
                z.enumIndex = idx;
                break;
            }
        }
    }

    // Die restlichen Kennungen behalten ihren Default: bei Layout v1 die drei
    // v2-Host-Parameter, immer die acht `occupied` (sie stehen im Kind `Dsp`).
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

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
