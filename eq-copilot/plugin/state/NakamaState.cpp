#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 — keine JucePlugin_*-Konstante im Kern
#include "NakamaState.h"
#include "NakamaUtf8.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <map>
#include <new>
#include <set>
#include <utility>

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
const juce::Identifier kMainMitglieder ("confirmed_members_v1");
// SONDE-013 M-69: neues persistentes Feld, und der Name traegt seine
// Fassung. Ein alter Build kennt es nicht, laesst es aber unangetastet im
// Baum stehen und schreibt es beim Speichern zurueck - genau die additive
// Erweiterung, die der Vertrag verlangt.
const juce::Identifier kMainPassagen ("manual_passages_v1");
// SONDE-014 M-02/M-03/M-06: der musikalische Intent. Drei additive
// Eigenschaften plus die Bestandsrevision. Jeder Name traegt seine Fassung;
// ein alter Build laesst sie unangetastet im Baum stehen und schreibt sie
// beim Speichern zurueck.
const juce::Identifier kMainIntents      ("source_intents_v1");
const juce::Identifier kMainSchutz       ("intent_protections_v1");
const juce::Identifier kMainBeziehungen  ("intent_relations_v1");
const juce::Identifier kMainIntentRev    ("intent_revision_v1");
// SONDE-014 Etappe G (§46.1, M-55 bis M-62): der aktuelle Assistentenschritt.
// EINE additive Eigenschaft mit Fassung im Namen — derselbe Weg wie der
// Intent, und derselbe Grund: ein alter Build laesst sie unangetastet stehen.
const juce::Identifier kMainAssistent    ("assistant_step_v1");
// SONDE-015: der Inhalt des Kindes `Dsp`. Vier Listen tragen ihre Fassung im
// Namen, `state_revision` und `undo_cursor` nicht - sie sind Skalare, deren
// Bedeutung sich nicht additiv erweitern kann.
const juce::Identifier kDspRevision  ("state_revision");
const juce::Identifier kDspOccupied  ("occupied_v1");
const juce::Identifier kDspZonen     ("schutz_zonen_v1");
const juce::Identifier kDspUndoRing  ("undo_ring_v1");
const juce::Identifier kDspUndoCursor("undo_cursor");
// Schema 1
const juce::Identifier kSensorId    ("sensor_id");
const juce::Identifier kRole        ("role");

constexpr int kRootSchema   = 2;
constexpr int kCommonSchema = 1;
constexpr int kMainSchema   = 1;
constexpr int kParamSchema  = 1;
constexpr int kDspSchema    = 1;

/*  Laenge eines Undo-Eintrags im Baum: Kopf (art, slot, revision) plus die
    120 Werte plus vier Zahlen je Zone. Die Zonenzahl steht NICHT zusaetzlich
    als Feld daneben - sie folgt aus der Laenge, und ein zweites Feld waere
    eine zweite Wahrheit, die von der Laenge abweichen koennte. */
constexpr int kUndoKopf = 3;
constexpr int kUndoBasis = kUndoKopf + parameter::kAnzahl;   // 123

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

/*  SONDE-014 M-03: der einzige Riegel fuer ein Schutz-Bandintervall - und er
    steht hier EINMAL, damit Produkt-API und Leser wirklich denselben
    benutzen. Zwei Kopien derselben Grenze sind in diesem Projekt schon
    auseinandergelaufen.

    Die Raender sind ausdruecklich benannt: Band 0 ist gueltig, Band 220 ist
    das letzte, `bis` ist halboffen und darf deshalb 221 sein, und ein leeres
    Intervall (`bis <= von`) ist keine Angabe, sondern ein Fehler. Wer keine
    Bandeigenschaft schuetzt, fuehrt -1/-1 - nicht 0/0, denn 0 IST ein Band. */
bool schutzbereichGueltig (Schutzeigenschaft eigenschaft, int bandVon, int bandBis)
{
    if (eigenschaft != Schutzeigenschaft::band)
        return bandVon == -1 && bandBis == -1;
    return bandVon >= 0 && bandVon < bandBis && bandBis <= bandAnzahlEvidenzgitter;
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

// ── SONDE-014: Woerter des Intents ─────────────────────────────────────────
//
// Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte (U22). Ein
// sechster Wert wird abgewiesen, nicht auf einen bekannten Zweig abgebildet -
// deshalb steht in `rolleAusWort` kein `else`-Zweig mit Rueckfall, und `wort`
// hat keinen Standardwert im `switch`, sondern nur eine erreichbare Zeile je
// Wert. Der Rueckfall hinter dem `switch` ist die MSVC-Pflichtzeile fuer
// einen aus dem Wertebereich gefallenen Enum, kein Produktverhalten.

const char* wort (Rolle r)
{
    switch (r)
    {
        case Rolle::fuehrt:       return "fuehrt";
        case Rolle::traegt:       return "traegt";
        case Rolle::begleitet:    return "begleitet";
        case Rolle::geschuetzt:   return "geschuetzt";
        case Rolle::verschmolzen: return "verschmolzen";
    }
    return "traegt";
}

const char* wort (Prominenz p)
{
    switch (p)
    {
        case Prominenz::foreground: return "foreground";
        case Prominenz::middle:     return "middle";
        case Prominenz::background: return "background";
    }
    return "middle";
}

const char* wort (Funktionstag t)
{
    switch (t)
    {
        case Funktionstag::keiner:     return "";
        case Funktionstag::lead:       return "lead";
        case Funktionstag::foundation: return "foundation";
        case Funktionstag::texture:    return "texture";
    }
    return "";
}

const char* wort (Veto v)
{
    switch (v)
    {
        case Veto::keins:        return "";
        case Veto::schutz:       return "schutz";
        case Veto::verschmolzen: return "verschmolzen";
    }
    return "";
}

const char* wort (IntentHerkunft h)
{
    switch (h)
    {
        case IntentHerkunft::user:       return "user";
        case IntentHerkunft::vorlage:    return "template";
        case IntentHerkunft::abgeleitet: return "inferred";
    }
    return "user";
}

const char* wort (Schutzeigenschaft e)
{
    switch (e)
    {
        case Schutzeigenschaft::attack:   return "attack";
        case Schutzeigenschaft::breite:   return "breite";
        case Schutzeigenschaft::ausklang: return "ausklang";
        case Schutzeigenschaft::band:     return "band";
    }
    return "attack";
}

const char* wort (Beziehungsart a)
{
    switch (a)
    {
        case Beziehungsart::fuehrtVor:        return "fuehrt_vor";
        case Beziehungsart::darfVerschmelzen: return "darf_verschmelzen";
        case Beziehungsart::gleichrangig:     return "gleichrangig";
    }
    return "fuehrt_vor";
}

bool rolleAusWort (const juce::String& w, Rolle& aus)
{
    if (w == "fuehrt")       { aus = Rolle::fuehrt; return true; }
    if (w == "traegt")       { aus = Rolle::traegt; return true; }
    if (w == "begleitet")    { aus = Rolle::begleitet; return true; }
    if (w == "geschuetzt")   { aus = Rolle::geschuetzt; return true; }
    if (w == "verschmolzen") { aus = Rolle::verschmolzen; return true; }
    return false;
}

bool intentHerkunftAusWort (const juce::String& w, IntentHerkunft& aus)
{
    if (w == "user")     { aus = IntentHerkunft::user; return true; }
    if (w == "template") { aus = IntentHerkunft::vorlage; return true; }
    if (w == "inferred") { aus = IntentHerkunft::abgeleitet; return true; }
    return false;
}

bool schutzeigenschaftAusWort (const juce::String& w, Schutzeigenschaft& aus)
{
    if (w == "attack")   { aus = Schutzeigenschaft::attack; return true; }
    if (w == "breite")   { aus = Schutzeigenschaft::breite; return true; }
    if (w == "ausklang") { aus = Schutzeigenschaft::ausklang; return true; }
    if (w == "band")     { aus = Schutzeigenschaft::band; return true; }
    return false;
}

bool beziehungsartAusWort (const juce::String& w, Beziehungsart& aus)
{
    if (w == "fuehrt_vor")        { aus = Beziehungsart::fuehrtVor; return true; }
    if (w == "darf_verschmelzen") { aus = Beziehungsart::darfVerschmelzen; return true; }
    if (w == "gleichrangig")      { aus = Beziehungsart::gleichrangig; return true; }
    return false;
}

/*  SONDE-015: die geschlossene Menge der Undo-Arten. `undo` und `redo` fehlen
    absichtlich - sie bewegen den Cursor im selben Ring, statt einen neuen
    Eintrag abzulegen (§5.11 Feinheit 3). */
const char* wort (UndoArt a) noexcept
{
    switch (a)
    {
        case UndoArt::apply:          return "apply";
        case UndoArt::revert:         return "revert";
        case UndoArt::neutralisieren: return "neutralisieren";
        case UndoArt::remove:         return "remove";
        case UndoArt::presetLaden:    return "preset_laden";
        case UndoArt::gestus:         return "gestus";
    }
    return "apply";
}

bool undoArtAusWort (const juce::String& w, UndoArt& aus)
{
    if (w == "apply")          { aus = UndoArt::apply; return true; }
    if (w == "revert")         { aus = UndoArt::revert; return true; }
    if (w == "neutralisieren") { aus = UndoArt::neutralisieren; return true; }
    if (w == "remove")         { aus = UndoArt::remove; return true; }
    if (w == "preset_laden")   { aus = UndoArt::presetLaden; return true; }
    if (w == "gestus")         { aus = UndoArt::gestus; return true; }
    return false;
}

parameter::DspSatz Zustand::dspDto() const
{
    parameter::DspSatz d;
    d.werte = parameters;
    d.zonen = schutzZonen;
    return d;
}

/*  E-01a, M-02: die ABGELEITETE Belegung.

    Der Vorschlagstext der Etappe 1 liess `prominence` fuer `geschuetzt` und
    `verschmolzen` "unveraendert (orthogonal)". Das ist keine Belegung, aus
    der sich die Rolle zurueckrechnen laesst - der Entscheid hat es deshalb
    korrigiert: beide liegen auf dem festen Neutralwert `middle` mit leerem
    Funktionstag und tragen ihr Veto-Kennzeichen ALS TEIL der Belegung. Erst
    damit ist die Abbildung total UND injektiv, und §33.5 ("keine zweite
    Wahrheit") ist eingehalten. */
Belegung belegung (Rolle r)
{
    switch (r)
    {
        case Rolle::fuehrt:       return { Prominenz::foreground, Funktionstag::lead,       Veto::keins };
        case Rolle::traegt:       return { Prominenz::middle,     Funktionstag::foundation, Veto::keins };
        case Rolle::begleitet:    return { Prominenz::background, Funktionstag::texture,    Veto::keins };
        case Rolle::geschuetzt:   return { Prominenz::middle,     Funktionstag::keiner,     Veto::schutz };
        case Rolle::verschmolzen: return { Prominenz::middle,     Funktionstag::keiner,     Veto::verschmolzen };
    }
    return { Prominenz::middle, Funktionstag::foundation, Veto::keins };
}

bool rolleAus (const Belegung& b, Rolle& aus)
{
    for (const auto r : { Rolle::fuehrt, Rolle::traegt, Rolle::begleitet,
                          Rolle::geschuetzt, Rolle::verschmolzen })
    {
        if (belegung (r) == b) { aus = r; return true; }
    }
    return false;
}

/*  §37.2, M-05: eine GEORDNETE LISTE, kein Score.

    Der Unterschied ist messbar und nicht kosmetisch: ein Score liesse sich
    aus mehreren niedrigen Stufen aufsummieren, bis er eine hohe schlaegt.
    Diese Funktion vergleicht ausschliesslich die Stufenzahl; es gibt keinen
    Additionsweg. Innerhalb derselben Stufe gewinnt der SPEZIFISCHERE
    Anspruch (E-02: die paarweise Beziehung vor dem globalen Rollenveto) -
    dieselbe Regel, die Stufe 3 fuer den Scope ausformuliert. */
int vergleicheAnsprueche (const Anspruch& a, const Anspruch& b)
{
    const auto sa = static_cast<int> (a.stufe);
    const auto sb = static_cast<int> (b.stufe);
    if (sa != sb) return sa < sb ? -1 : 1;
    if (a.spezifisch != b.spezifisch) return a.spezifisch ? -1 : 1;
    return 0;
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

/*  ── SONDE-015: das Kind `Dsp` schreiben ───────────────────────────────── */

/** Traegt der Zustand ueberhaupt einen DSP-Inhalt? Nur dann entsteht das Kind. */
bool dspTraegtEtwas (const Zustand& z)
{
    if (z.stateRevision != 0 || ! z.schutzZonen.empty() || ! z.undoRing.empty())
        return true;
    for (int slot = 0; slot < parameter::kSlots; ++slot)
        if (z.parameters[(size_t) parameter::indexOccupied (slot)].b)
            return true;
    return false;
}

/** Traegt der GEHALTENE `Dsp`-Knoten eine Eigenschaft ausserhalb der bekannten
    Menge?

    🔑 SONDE-015 Etappe 2, Nacharbeit 1 (Befund B-03): `dspTraegtEtwas` kennt
    nur BEKANNTE Inhalte. Ein gueltiger Stand mit Revision 0, freien Slots,
    leeren Listen und einer unbekannten Eigenschaft im Kind laed damit
    schreibbar - und das Weglassen entfernte beim naechsten Speichern das ganze
    Kind samt der unbekannten Eigenschaft. M-91 sagt aber ohne Einschraenkung
    "der Leser ignoriert sie und ERHAELT sie beim Speichern", und `CLAUDE.md`
    verlangt "State bleibt verlustfrei". Die Weglassregel gilt deshalb nur,
    wenn auch nichts Unbekanntes dranhaengt. */
bool dspTraegtUnbekanntes (const juce::ValueTree& dsp)
{
    if (! dsp.isValid())
        return false;
    static const juce::Identifier* const bekannt[] = {
        &kSchema, &kDspRevision, &kDspOccupied, &kDspZonen, &kDspUndoRing, &kDspUndoCursor
    };
    for (int i = 0; i < dsp.getNumProperties(); ++i)
    {
        const auto name = dsp.getPropertyName (i);
        bool gefunden = false;
        for (const auto* b : bekannt)
            if (name == *b)
                gefunden = true;
        if (! gefunden)
            return true;
    }
    // Ein Kindknoten unter `Dsp` ist ebenfalls etwas, das dieser Build nicht
    // kennt (§2.0: flache Arrays, keine Kindknoten) - und ihn wegzuwerfen
    // waere derselbe Verlust.
    return dsp.getNumChildren() > 0;
}

/** Zonen als flache Vierergruppen [id, low_hz, high_hz, enabled, ...]. */
juce::Array<juce::var> zonenFlach (const std::vector<parameter::Schutzzone>& zonen)
{
    juce::Array<juce::var> flach;
    for (const auto& e : zonen)
    {
        flach.add (e.id);
        flach.add (e.lowHz);
        flach.add (e.highHz);
        flach.add (e.enabled);
    }
    return flach;
}

/** Ein Undo-Eintrag als EIGENES Array: [art, slot, revision, 120 Werte,
    4 x z Zonenwerte]. Verschachtelt statt flach aneinandergereiht, weil die
    Eintragsgrenze damit aus der Struktur folgt und nicht aus einer
    mitgeschriebenen Zaehlung, die von ihr abweichen koennte. */
juce::Array<juce::var> undoFlach (const UndoEintrag& e)
{
    juce::Array<juce::var> a;
    a.add (juce::String (wort (e.art)));
    a.add (e.slot);
    a.add (juce::var (e.revision));
    const auto& t = parameter::tabelle();
    for (int i = 0; i < parameter::kAnzahl; ++i)
    {
        const auto& z = e.zustand.werte[(size_t) i];
        switch (t[(size_t) i].typ)
        {
            case parameter::Typ::boolean:     a.add (z.b); break;
            case parameter::Typ::gleitkomma:  a.add (z.zahl); break;
            case parameter::Typ::aufzaehlung: a.add (z.enumIndex); break;
        }
    }
    a.addArray (zonenFlach (e.zustand.zonen));
    return a;
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
        if (z.mainProjectMitglieder.empty())
        {
            mainProject.removeProperty (kMainMitglieder, nullptr);
        }
        else
        {
            auto mitglieder = z.mainProjectMitglieder;
            std::sort (mitglieder.begin(), mitglieder.end(), [] (const auto& a, const auto& b)
            {
                return a.instanceId.compare (b.instanceId) < 0;
            });
            juce::Array<juce::var> flach;
            flach.ensureStorageAllocated (static_cast<int> (mitglieder.size() * 2));
            for (const auto& m : mitglieder)
            {
                flach.add (m.instanceId);
                flach.add (m.label);
            }
            mainProject.setProperty (kMainMitglieder, juce::var (flach), nullptr);
        }

        // Die manuellen Passagen (M-69). Leere Liste heisst: Eigenschaft weg,
        // nicht leeres Array - sonst unterschieden sich ein Projekt ohne
        // Passagen und eines, dessen letzte geloescht wurde, in den Bytes.
        if (z.manuellePassagen.empty())
        {
            mainProject.removeProperty (kMainPassagen, nullptr);
        }
        else
        {
            auto passagen = z.manuellePassagen;
            std::sort (passagen.begin(), passagen.end(), [] (const auto& a, const auto& b)
            {
                if (a.projektStart != b.projektStart) return a.projektStart < b.projektStart;
                if (a.projektEnde  != b.projektEnde)  return a.projektEnde  < b.projektEnde;
                return a.passageId.compare (b.passageId) < 0;
            });
            juce::Array<juce::var> flach;
            flach.ensureStorageAllocated (static_cast<int> (passagen.size() * 4));
            for (const auto& s : passagen)
            {
                flach.add (s.passageId);
                flach.add (s.label);
                flach.add (juce::var (s.projektStart));
                flach.add (juce::var (s.projektEnde));
            }
            mainProject.setProperty (kMainPassagen, juce::var (flach), nullptr);
        }

        /*  SONDE-014 Etappe A: der musikalische Intent (§37.1).

            Dieselbe Regel wie bei den Passagen: eine leere Liste heisst
            Eigenschaft WEG, nicht leeres Array - sonst unterschieden sich
            ein Projekt ohne Intent und eines, dessen letzter geloescht
            wurde, in den Bytes. Und dieselbe Regel wie bei den Mitgliedern:
            die Reihenfolge auf der Leitung ist sortiert und traegt keine
            Eingabereihenfolge. */
        if (z.sourceIntents.empty())
        {
            mainProject.removeProperty (kMainIntents, nullptr);
        }
        else
        {
            auto intents = z.sourceIntents;
            std::sort (intents.begin(), intents.end(), [] (const auto& a, const auto& b)
            {
                const auto q = a.quelleId.compare (b.quelleId);
                if (q != 0) return q < 0;
                return a.passageId.compare (b.passageId) < 0;
            });
            juce::Array<juce::var> flach;
            flach.ensureStorageAllocated (static_cast<int> (intents.size() * 6));
            for (const auto& s : intents)
            {
                flach.add (s.quelleId);
                flach.add (s.passageId);
                flach.add (juce::String (wort (s.rolle)));
                flach.add (juce::var (s.revision));
                flach.add (juce::String (wort (s.herkunft)));
                flach.add (juce::var (s.konfidenz));
            }
            mainProject.setProperty (kMainIntents, juce::var (flach), nullptr);
        }

        if (z.schutzangaben.empty())
        {
            mainProject.removeProperty (kMainSchutz, nullptr);
        }
        else
        {
            auto schutz = z.schutzangaben;
            std::sort (schutz.begin(), schutz.end(), [] (const auto& a, const auto& b)
            {
                const auto q = a.quelleId.compare (b.quelleId);
                if (q != 0) return q < 0;
                if (a.eigenschaft != b.eigenschaft)
                    return static_cast<int> (a.eigenschaft) < static_cast<int> (b.eigenschaft);
                if (a.bandVon != b.bandVon) return a.bandVon < b.bandVon;
                return a.bandBis < b.bandBis;
            });
            juce::Array<juce::var> flach;
            flach.ensureStorageAllocated (static_cast<int> (schutz.size() * 4));
            for (const auto& s : schutz)
            {
                flach.add (s.quelleId);
                flach.add (juce::String (wort (s.eigenschaft)));
                flach.add (juce::var (s.bandVon));
                flach.add (juce::var (s.bandBis));
            }
            mainProject.setProperty (kMainSchutz, juce::var (flach), nullptr);
        }

        if (z.intentBeziehungen.empty())
        {
            mainProject.removeProperty (kMainBeziehungen, nullptr);
        }
        else
        {
            auto kanten = z.intentBeziehungen;
            std::sort (kanten.begin(), kanten.end(), [] (const auto& a, const auto& b)
            {
                const auto qa = a.quelleA.compare (b.quelleA);
                if (qa != 0) return qa < 0;
                const auto qb = a.quelleB.compare (b.quelleB);
                if (qb != 0) return qb < 0;
                return static_cast<int> (a.art) < static_cast<int> (b.art);
            });
            juce::Array<juce::var> flach;
            flach.ensureStorageAllocated (static_cast<int> (kanten.size() * 3));
            for (const auto& k : kanten)
            {
                flach.add (k.quelleA);
                flach.add (k.quelleB);
                flach.add (juce::String (wort (k.art)));
            }
            mainProject.setProperty (kMainBeziehungen, juce::var (flach), nullptr);
        }

        /*  Die Bestandsrevision. Revision 0 heisst "nie etwas gesetzt" und
            reist deshalb GAR NICHT - der Rand, an dem ein leerer Bestand
            und ein nie beschriebener sich sonst in den Bytes glichen und der
            Empfaenger die Vollstaendigkeitsmarke aus M-86 nicht bilden
            koennte. */
        if (z.intentBestandRevision <= 0)
            mainProject.removeProperty (kMainIntentRev, nullptr);
        else
            mainProject.setProperty (kMainIntentRev, juce::var (z.intentBestandRevision), nullptr);

        /*  SONDE-014 Etappe G: der Assistentenschritt. Acht Werte in einer
            flachen Liste — dieselbe Form wie die drei Intentlisten daneben.

            Ein NICHT GESETZTER Schritt reist gar nicht: „noch nie einen
            Assistenten benutzt" und „einen Schritt mit leeren Feldern" waeren
            in den Bytes sonst dasselbe, und der Broker koennte den Spiegel
            nicht davon unterscheiden (M-88). */
        if (! z.assistent.gesetzt)
        {
            mainProject.removeProperty (kMainAssistent, nullptr);
        }
        else
        {
            juce::Array<juce::var> flach;
            flach.add (z.assistent.stepId);
            flach.add (juce::String (wort (z.assistent.schritt)));
            flach.add (juce::var (z.assistent.revision));
            flach.add (juce::var (z.assistent.offen));
            flach.add (z.assistent.findingId);
            flach.add (z.assistent.proposalId);
            flach.add (z.assistent.experimentId);
            flach.add (juce::String (wort (z.assistent.ergebnis)));
            mainProject.setProperty (kMainAssistent, juce::var (flach), nullptr);
        }
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

    /*  SONDE-015: das Kind `Dsp`.

        Es wird nur geschrieben, wenn es etwas zu sagen gibt - Revision,
        Belegung, Zone oder Undo-Eintrag. Ein frischer, unberuehrter Stand
        bleibt damit fuer einen Build lesbar, der `Dsp` noch nicht kennt.
        Dieselbe Regel wie beim nie gesetzten `assistant_step_v1`: "noch nie
        benutzt" und "mit leeren Feldern benutzt" waeren in den Bytes sonst
        dasselbe.

        🔑 Nacharbeit 1 (B-03): weggelassen wird NUR, wenn auch der gehaltene
        Knoten nichts Unbekanntes traegt. Sonst wird das Kind mit den bekannten
        Werten zurueckgeschrieben und die unbekannte Eigenschaft bleibt stehen -
        M-91 kennt dafuer keine Ausnahme. */
    auto dsp = kopie.getChildWithName (kDsp);
    if (z.hatParameters && (dspTraegtEtwas (z) || dspTraegtUnbekanntes (dsp)))
    {
        if (! dsp.isValid())
        {
            dsp = juce::ValueTree (kDsp);
            kopie.appendChild (dsp, nullptr);
        }
        dsp.setProperty (kSchema, kDspSchema, nullptr);
        dsp.setProperty (kDspRevision, juce::var (z.stateRevision), nullptr);

        juce::Array<juce::var> belegt;
        for (int slot = 0; slot < parameter::kSlots; ++slot)
            belegt.add (z.parameters[(size_t) parameter::indexOccupied (slot)].b);
        dsp.setProperty (kDspOccupied, juce::var (belegt), nullptr);

        if (z.schutzZonen.empty())
            dsp.removeProperty (kDspZonen, nullptr);
        else
            dsp.setProperty (kDspZonen, juce::var (zonenFlach (z.schutzZonen)), nullptr);

        if (z.undoRing.empty())
        {
            dsp.removeProperty (kDspUndoRing, nullptr);
            dsp.removeProperty (kDspUndoCursor, nullptr);
        }
        else
        {
            juce::Array<juce::var> ring;
            for (const auto& e : z.undoRing)
                ring.add (juce::var (undoFlach (e)));
            dsp.setProperty (kDspUndoRing, juce::var (ring), nullptr);
            dsp.setProperty (kDspUndoCursor, z.undoCursor, nullptr);
        }
    }
    else if (dsp.isValid())
    {
        kopie.removeChild (dsp, nullptr);
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
    kandidat.mainProjectMitglieder.clear();
    for (int i = 0; i < maxMainProjectMitglieder; ++i)
    {
        kandidat.mainProjectMitglieder.push_back ({
            juce::String::toHexString (i + 1).paddedLeft ('0', 32), maximalerText (120)
        });
    }
    // Und die manuellen Passagen in ihrer groessten erreichbaren Form: volle
    // Liste, volle Labels, `int64`-Grenzen am oberen Rand. Ohne diesen Zusatz
    // versprache der Headroomriegel etwas ueber einen Stand, den die
    // Produkt-API laengst uebertreffen kann (M-69).
    kandidat.manuellePassagen.clear();
    for (int i = 0; i < maxManuellePassagen; ++i)
    {
        kandidat.manuellePassagen.push_back ({
            juce::String::toHexString (i + 1).paddedLeft ('0', 32), maximalerText (120),
            std::numeric_limits<juce::int64>::max() - 2,
            std::numeric_limits<juce::int64>::max() - 1
        });
    }

    /*  SONDE-014: derselbe Zusatz fuer den Intent-Bestand in seiner groessten
        erreichbaren Form. Ohne ihn versprache der Headroomriegel etwas ueber
        einen Stand, den die Produkt-API laengst uebertreffen kann - genau die
        Luecke, die M-69 fuer die Passagen geschlossen hat.

        Die Kanten sind bewusst eine KETTE (i -> i+1) und kein Stern: eine
        Kette hat die volle Kantenzahl und bleibt trotzdem zyklenfrei, also
        laedt der erzeugte Stand auch wirklich. */
    kandidat.sourceIntents.clear();
    for (int i = 0; i < maxSourceIntents; ++i)
    {
        kandidat.sourceIntents.push_back ({
            juce::String::toHexString (i + 1).paddedLeft ('0', 32),
            juce::String::toHexString (i + 1).paddedLeft ('f', 32),
            Rolle::verschmolzen,
            std::numeric_limits<juce::int64>::max(),
            IntentHerkunft::abgeleitet,
            1.0
        });
    }
    kandidat.schutzangaben.clear();
    for (int i = 0; i < maxSchutzangaben; ++i)
    {
        kandidat.schutzangaben.push_back ({
            juce::String::toHexString (i + 1).paddedLeft ('0', 32),
            Schutzeigenschaft::band, 0, bandAnzahlEvidenzgitter
        });
    }
    kandidat.intentBeziehungen.clear();
    for (int i = 0; i < maxIntentBeziehungen; ++i)
    {
        kandidat.intentBeziehungen.push_back ({
            juce::String::toHexString (i + 1).paddedLeft ('0', 32),
            juce::String::toHexString (i + 2).paddedLeft ('0', 32),
            Beziehungsart::darfVerschmelzen
        });
    }
    kandidat.intentBestandRevision = std::numeric_limits<juce::int64>::max();
    // SONDE-014 Etappe G: der Assistentenschritt gehoert in den Headroomriegel.
    // Er ist EIN Objekt und kostet wenig, aber „wenig" ist keine Messung.
    kandidat.assistent = {
        true,
        juce::String::toHexString (0xa55e5).paddedLeft ('0', 32),
        Assistentenschritt::verdict,
        std::numeric_limits<juce::int64>::max(),
        true,
        juce::String::toHexString (0xf1d6).paddedLeft ('0', 32),
        juce::String::toHexString (0x9005a1).paddedLeft ('0', 32),
        juce::String::toHexString (0xe89e21).paddedLeft ('0', 32),
        Assistentenergebnis::keineAenderungEmpfohlen
    };

    /*  SONDE-015: das Kind `Dsp` in seiner groessten erreichbaren Form -
        Revision am `int64`-Rand, alle acht Slots belegt, acht Zonen und ein
        VOLLER Undo-Ring aus 32 Schnappschuessen. Ohne diesen Zusatz
        versprache der Headroomriegel etwas ueber einen Stand, den die
        Produkt-API laengst uebertreffen kann - dieselbe Luecke, die M-69 fuer
        die Passagen und SONDE-014 fuer den Intent geschlossen hat. Und genau
        das misst M-78: der volle Ring reisst die 16-MiB-Grenze NICHT. */
    kandidat.stateRevision = std::numeric_limits<juce::int64>::max();
    for (int slot = 0; slot < parameter::kSlots; ++slot)
        kandidat.parameters[(size_t) parameter::indexOccupied (slot)].b = true;
    kandidat.schutzZonen.clear();
    for (int i = 0; i < parameter::kMaxZonen; ++i)
        kandidat.schutzZonen.push_back ({ i, 20.0 + (double) i, 19000.0 + (double) i, true });
    kandidat.undoRing.clear();
    for (int i = 0; i < parameter::kUndoTiefe; ++i)
    {
        UndoEintrag u;
        u.art = UndoArt::presetLaden;   // laengstes Wort der geschlossenen Menge
        u.slot = parameter::kSlots - 1;
        u.revision = std::numeric_limits<juce::int64>::max() - i;
        u.zustand.werte = kandidat.parameters;
        u.zustand.zonen = kandidat.schutzZonen;
        kandidat.undoRing.push_back (std::move (u));
    }
    kandidat.undoCursor = parameter::kUndoTiefe;

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

/*  ── SONDE-015: das Kind `Dsp` lesen ───────────────────────────────────────

    Was der Schreiber garantiert, prueft der Leser NACH: Anzahl, Typ, Bereich,
    doppelte Zonen-id, Sortierung, Ringtiefe, Eintragslaenge und Cursorlage.
    Ein Leser, der weniger prueft als sein Schreiber zusagt, ist das Loch, durch
    das eine von Hand veraenderte Projektdatei den Zustand vergiftet. Ein
    Verstoss macht den GANZEN State read-only - kein Teilstate (§53.8). */
bool leseZonenFlach (const juce::var& wert, std::vector<parameter::Schutzzone>& aus, juce::String& grund)
{
    const auto* flach = wert.getArray();
    if (flach == nullptr || flach->size() % 4 != 0
        || flach->size() > parameter::kMaxZonen * 4)
    {
        grund = "Dsp.schutz_zonen_v1 must be an array of quadruples with at most 8 entries";
        return false;
    }
    aus.clear();
    for (int i = 0; i < flach->size(); i += 4)
    {
        const auto idWert   = flach->getReference (i);
        const auto lowWert  = flach->getReference (i + 1);
        const auto highWert = flach->getReference (i + 2);
        const auto anWert   = flach->getReference (i + 3);
        if (! idWert.isInt() || ! lowWert.isDouble() || ! highWert.isDouble() || ! anWert.isBool())
        {
            grund = "Dsp.schutz_zonen_v1 quadruple must be int, double, double, bool";
            return false;
        }
        parameter::Schutzzone z;
        z.id      = (int) idWert;
        z.lowHz   = (double) lowWert;
        z.highHz  = (double) highWert;
        z.enabled = (bool) anWert;
        aus.push_back (z);
    }
    juce::String zoneGrund, wo;
    if (! parameter::validiereZonen (aus, zoneGrund, wo))
    {
        grund = "Dsp.schutz_zonen_v1 " + zoneGrund + " at " + wo;
        return false;
    }
    return true;
}

bool leseDspKind (const juce::ValueTree& d, parameter::Satz& satz, juce::int64& revision,
                  std::vector<parameter::Schutzzone>& zonen, std::vector<UndoEintrag>& ring,
                  int& cursor, juce::String& grund)
{
    // 1. `state_revision` ist PFLICHT. Ein `Dsp`-Kind ohne sie waere ein
    //    Zustand ohne Zeitachse - und der Broker koennte einen verspaeteten
    //    Bericht nicht von einem aktuellen unterscheiden (§44.4).
    if (! d.hasProperty (kDspRevision)) { grund = "Dsp.state_revision is missing"; return false; }
    {
        const auto w = d.getProperty (kDspRevision);
        if (! w.isInt() && ! w.isInt64()) { grund = "Dsp.state_revision is not an integer"; return false; }
        revision = (juce::int64) w;
        if (revision < 0) { grund = "Dsp.state_revision must not be negative"; return false; }
    }

    // 2. Die acht `occupied`. Fehlt die Eigenschaft, ist kein Slot belegt.
    if (d.hasProperty (kDspOccupied))
    {
        const auto* flach = d.getProperty (kDspOccupied).getArray();
        if (flach == nullptr || flach->size() != parameter::kSlots)
        {
            grund = "Dsp.occupied_v1 must be an array of exactly 8 booleans";
            return false;
        }
        for (int slot = 0; slot < parameter::kSlots; ++slot)
        {
            const auto w = flach->getReference (slot);
            if (! w.isBool()) { grund = "Dsp.occupied_v1 entry is not a bool"; return false; }
            satz[(size_t) parameter::indexOccupied (slot)].b = (bool) w;
        }
    }

    // 3. Die Schutz-Zonen.
    zonen.clear();
    if (d.hasProperty (kDspZonen) && ! leseZonenFlach (d.getProperty (kDspZonen), zonen, grund))
        return false;

    // 4. Der Undo-Ring. Jeder Eintrag ist ein eigenes Array
    //    [art, slot, revision, 120 Werte, 4 x z Zonenwerte].
    ring.clear();
    cursor = 0;
    if (d.hasProperty (kDspUndoRing))
    {
        const auto* eintraege = d.getProperty (kDspUndoRing).getArray();
        if (eintraege == nullptr || eintraege->size() > parameter::kUndoTiefe)
        {
            grund = "Dsp.undo_ring_v1 must be an array of at most 32 entries";
            return false;
        }
        const auto& t = parameter::tabelle();
        for (int n = 0; n < eintraege->size(); ++n)
        {
            const auto* e = eintraege->getReference (n).getArray();
            if (e == nullptr || e->size() < kUndoBasis
                || (e->size() - kUndoBasis) % 4 != 0
                || (e->size() - kUndoBasis) / 4 > parameter::kMaxZonen)
            {
                grund = "Dsp.undo_ring_v1 entry has an invalid length";
                return false;
            }
            UndoEintrag u;
            if (! e->getReference (0).isString()
                || ! undoArtAusWort (e->getReference (0).toString(), u.art))
            {
                grund = "Dsp.undo_ring_v1 entry has an unknown art";
                return false;
            }
            if (! e->getReference (1).isInt()) { grund = "Dsp.undo_ring_v1 slot is not an integer"; return false; }
            u.slot = (int) e->getReference (1);
            if (u.slot < -1 || u.slot >= parameter::kSlots)
            {
                grund = "Dsp.undo_ring_v1 slot is out of range";
                return false;
            }
            const auto rw = e->getReference (2);
            if (! rw.isInt() && ! rw.isInt64()) { grund = "Dsp.undo_ring_v1 revision is not an integer"; return false; }
            u.revision = (juce::int64) rw;
            if (u.revision < 0) { grund = "Dsp.undo_ring_v1 revision must not be negative"; return false; }

            for (int i = 0; i < parameter::kAnzahl; ++i)
            {
                const auto w = e->getReference (kUndoKopf + i);
                auto& z = u.zustand.werte[(size_t) i];
                switch (t[(size_t) i].typ)
                {
                    case parameter::Typ::boolean:
                        if (! w.isBool()) { grund = "Dsp.undo_ring_v1 value is not a bool"; return false; }
                        z.b = (bool) w;
                        break;
                    case parameter::Typ::gleitkomma:
                        if (! w.isDouble()) { grund = "Dsp.undo_ring_v1 value is not a double"; return false; }
                        z.zahl = (double) w;
                        break;
                    case parameter::Typ::aufzaehlung:
                        if (! w.isInt()) { grund = "Dsp.undo_ring_v1 value is not an enum index"; return false; }
                        z.enumIndex = (int) w;
                        break;
                }
            }
            for (int i = kUndoBasis; i < e->size(); i += 4)
            {
                const auto idWert   = e->getReference (i);
                const auto lowWert  = e->getReference (i + 1);
                const auto highWert = e->getReference (i + 2);
                const auto anWert   = e->getReference (i + 3);
                if (! idWert.isInt() || ! lowWert.isDouble() || ! highWert.isDouble() || ! anWert.isBool())
                {
                    grund = "Dsp.undo_ring_v1 zone quadruple must be int, double, double, bool";
                    return false;
                }
                u.zustand.zonen.push_back ({ (int) idWert, (double) lowWert, (double) highWert, (bool) anWert });
            }
            // Ein Undo-Eintrag ist ein ZUSTAND, den dieser Build wiederherstellen
            // koennen muss. Er wird deshalb nach denselben Regeln geprueft wie
            // der bestaetigte Zustand - sonst legte ein Undo einen Zustand her,
            // den der eigene Validator ablehnt.
            juce::String eGrund, wo;
            if (! parameter::validiere (u.zustand, eGrund, wo))
            {
                grund = "Dsp.undo_ring_v1 entry " + eGrund + " at " + wo;
                return false;
            }
            ring.push_back (std::move (u));
        }

        if (d.hasProperty (kDspUndoCursor))
        {
            const auto w = d.getProperty (kDspUndoCursor);
            if (! w.isInt()) { grund = "Dsp.undo_cursor is not an integer"; return false; }
            cursor = (int) w;
            if (cursor < 0 || cursor > (int) ring.size())
            {
                grund = "Dsp.undo_cursor is out of range";
                return false;
            }
        }
    }
    else if (d.hasProperty (kDspUndoCursor))
    {
        // Ein Cursor ohne Ring zeigt auf nichts. Ihn stumm auf 0 zu setzen
        // hiesse, einen Rueckweg zu behaupten, den es nicht gibt.
        grund = "Dsp.undo_cursor without undo_ring_v1";
        return false;
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
    // SONDE-015: `Dsp` ist seit diesem Ticket lesbar - fuer `active_probe`.
    // Die Klassenpruefung steht weiter unten bei der Kind-Matrix, weil die
    // Klasse erst aus `Common` kommt.
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
    // SONDE-015: `Dsp` ist fuer `active_probe` optional und fuer jede andere
    // Klasse verboten - der Zustand eines EQ-Kerns, den es dort nicht gibt.
    if (! istAktiv && nDsp > 0)  { grund = juce::String ("Dsp is not allowed for ") + wort (c.klasse); return false; }

    if (istMain && ! schemaIst (v.getChildWithName (kMainProject), kMainSchema))
    {
        grund = "MainProject schema is unknown to this version (it reads schema 1)"; return false;
    }

    std::vector<MainProjectMitglied> mainMitglieder;
    if (istMain)
    {
        const auto mainProject = v.getChildWithName (kMainProject);
        if (mainProject.hasProperty (kMainMitglieder))
        {
            const auto wert = mainProject.getProperty (kMainMitglieder);
            const auto* flach = wert.getArray();
            if (flach == nullptr || flach->size() % 2 != 0
                || flach->size() > maxMainProjectMitglieder * 2)
            {
                grund = "MainProject.confirmed_members_v1 must be an even array with at most 64 pairs";
                return false;
            }
            std::set<std::string> gesehen;
            for (int i = 0; i < flach->size(); i += 2)
            {
                const auto idWert = flach->getReference (i);
                const auto labelWert = flach->getReference (i + 1);
                if (! idWert.isString() || ! labelWert.isString()
                    || ! istHex32 (idWert.toString()) || labelWert.toString().length() > 120)
                {
                    grund = "MainProject.confirmed_members_v1 contains an invalid instance_id or label";
                    return false;
                }
                const auto idBytes = idWert.toString().toStdString();
                if (! gesehen.insert (idBytes).second)
                {
                    grund = "MainProject.confirmed_members_v1 contains a duplicate instance_id";
                    return false;
                }
                mainMitglieder.push_back ({ idWert.toString(), labelWert.toString() });
            }
        }
    }

    /*  Die manuellen Passagen (M-69). Derselbe fail-closed-Riegel wie fuer die
        Mitglieder, um vier zusaetzliche Fragen erweitert:

        - Start und Ende sind GANZE Zahlen. `var` haelt auch Doubles, und ein
          Projektsample 44100.5 gibt es nicht; still zu runden hiesse, eine
          Passagengrenze zu erfinden.
        - Ende liegt echt hinter Start. Das Fenster ist halboffen; eine leere
          Passage waere eine Markierung ueber nichts.
        - Start ist nicht negativ. Projektsamples zaehlen ab Projektbeginn.
        - Keine doppelte passage_id, sonst zeigten zwei Intents auf dasselbe
          Objekt im Store. */
    std::vector<ManuellePassage> mainPassagen;
    if (istMain)
    {
        const auto mainProject = v.getChildWithName (kMainProject);
        if (mainProject.hasProperty (kMainPassagen))
        {
            const auto wert = mainProject.getProperty (kMainPassagen);
            const auto* flach = wert.getArray();
            if (flach == nullptr || flach->size() % 4 != 0
                || flach->size() > maxManuellePassagen * 4)
            {
                grund = "MainProject.manual_passages_v1 must be an array of quadruples with at most 64 entries";
                return false;
            }
            std::set<std::string> gesehen;
            for (int i = 0; i < flach->size(); i += 4)
            {
                const auto idWert    = flach->getReference (i);
                const auto labelWert = flach->getReference (i + 1);
                const auto vonWert   = flach->getReference (i + 2);
                const auto bisWert   = flach->getReference (i + 3);
                if (! idWert.isString() || ! labelWert.isString()
                    || ! istHex32 (idWert.toString()) || labelWert.toString().length() > 120)
                {
                    grund = "MainProject.manual_passages_v1 contains an invalid passage_id or label";
                    return false;
                }
                if (! (vonWert.isInt() || vonWert.isInt64())
                    || ! (bisWert.isInt() || bisWert.isInt64()))
                {
                    grund = "MainProject.manual_passages_v1 bounds must be integers";
                    return false;
                }
                const auto von = static_cast<juce::int64> (vonWert);
                const auto bis = static_cast<juce::int64> (bisWert);
                if (von < 0 || bis <= von)
                {
                    grund = "MainProject.manual_passages_v1 bounds must satisfy 0 <= start < end";
                    return false;
                }
                if (! gesehen.insert (idWert.toString().toStdString()).second)
                {
                    grund = "MainProject.manual_passages_v1 contains a duplicate passage_id";
                    return false;
                }
                mainPassagen.push_back ({ idWert.toString(), labelWert.toString(), von, bis });
            }
        }
    }

    /*  SONDE-014 Etappe A: der Intent-Bestand, mit denselben Riegeln wie die
        Produkt-API. Was `setzeIntent` ablehnt, muss auch hier fallen -
        fail-closed als read-only, nie still korrigiert (M-06, M-07, M-13).

        - hex32 fuer Quelle; hex32 ODER leer fuer die Passage (leer = global).
        - Rollen- und Herkunftswort aus ihrer geschlossenen Menge. Ein
          sechstes Rollenwort wird ABGEWIESEN, nicht auf einen bekannten
          Zweig abgebildet (M-01).
        - Revision >= 1 und ganzzahlig; Revision 0 gibt es nicht.
        - Konfidenz ENDLICH und in [0,1]. NaN und Inf faellt hier, nicht
          spaeter beim Rechnen (M-82).
        - Genau ein Objekt je (Quelle, Scope) - zwei waeren zwei Wahrheiten
          fuer dieselbe Frage (E-01). */
    std::vector<SourceIntent> mainIntents;
    std::vector<Schutzangabe> mainSchutz;
    std::vector<IntentBeziehung> mainKanten;
    juce::int64 mainIntentRevision = 0;
    Assistentenzustand mainAssistent {};
    if (istMain)
    {
        const auto mainProject = v.getChildWithName (kMainProject);
        if (mainProject.hasProperty (kMainIntents))
        {
            const auto wert = mainProject.getProperty (kMainIntents);
            const auto* flach = wert.getArray();
            if (flach == nullptr || flach->size() % 6 != 0
                || flach->size() > maxSourceIntents * 6)
            {
                grund = "MainProject.source_intents_v1 must be an array of sextuples with at most 256 entries";
                return false;
            }
            std::set<std::string> gesehen;
            for (int i = 0; i < flach->size(); i += 6)
            {
                const auto quelle   = flach->getReference (i);
                const auto passage  = flach->getReference (i + 1);
                const auto rolle    = flach->getReference (i + 2);
                const auto revision = flach->getReference (i + 3);
                const auto herkunft = flach->getReference (i + 4);
                const auto konf     = flach->getReference (i + 5);
                if (! quelle.isString() || ! istHex32 (quelle.toString()))
                {
                    grund = "MainProject.source_intents_v1 contains an invalid source id";
                    return false;
                }
                if (! passage.isString()
                    || (passage.toString().isNotEmpty() && ! istHex32 (passage.toString())))
                {
                    grund = "MainProject.source_intents_v1 contains an invalid passage scope";
                    return false;
                }
                SourceIntent eintrag;
                eintrag.quelleId  = quelle.toString();
                eintrag.passageId = passage.toString();
                if (! rolle.isString() || ! rolleAusWort (rolle.toString(), eintrag.rolle))
                {
                    grund = "MainProject.source_intents_v1 contains an unknown role: " + rolle.toString();
                    return false;
                }
                if (! (revision.isInt() || revision.isInt64()))
                {
                    grund = "MainProject.source_intents_v1 revision must be an integer";
                    return false;
                }
                eintrag.revision = static_cast<juce::int64> (revision);
                if (eintrag.revision < 1)
                {
                    grund = "MainProject.source_intents_v1 revision must be at least 1";
                    return false;
                }
                if (! herkunft.isString() || ! intentHerkunftAusWort (herkunft.toString(), eintrag.herkunft))
                {
                    grund = "MainProject.source_intents_v1 contains an unknown origin: " + herkunft.toString();
                    return false;
                }
                if (! (konf.isDouble() || konf.isInt() || konf.isInt64()))
                {
                    grund = "MainProject.source_intents_v1 confidence must be a number";
                    return false;
                }
                eintrag.konfidenz = static_cast<double> (konf);
                if (! std::isfinite (eintrag.konfidenz)
                    || eintrag.konfidenz < 0.0 || eintrag.konfidenz > 1.0)
                {
                    grund = "MainProject.source_intents_v1 confidence must be finite within [0,1]";
                    return false;
                }
                const auto schluessel = (eintrag.quelleId + "|" + eintrag.passageId).toStdString();
                if (! gesehen.insert (schluessel).second)
                {
                    grund = "MainProject.source_intents_v1 contains two intents for the same source and scope";
                    return false;
                }
                mainIntents.push_back (eintrag);
            }
        }

        /*  Die Schutzangaben. Orthogonal zur Rolle (M-03): sie brauchen
            keinen Intent und werden von keinem Rollenwechsel beruehrt.
            `band` traegt ein halboffenes Intervall des 221er-Gitters; die
            drei uebrigen Eigenschaften fuehren beide Grenzen auf -1 statt
            auf 0, weil 0 ein GUELTIGER Bandindex ist. */
        if (mainProject.hasProperty (kMainSchutz))
        {
            const auto wert = mainProject.getProperty (kMainSchutz);
            const auto* flach = wert.getArray();
            if (flach == nullptr || flach->size() % 4 != 0
                || flach->size() > maxSchutzangaben * 4)
            {
                grund = "MainProject.intent_protections_v1 must be an array of quadruples with at most 256 entries";
                return false;
            }
            std::set<std::string> gesehen;
            for (int i = 0; i < flach->size(); i += 4)
            {
                const auto quelle = flach->getReference (i);
                const auto eig    = flach->getReference (i + 1);
                const auto von    = flach->getReference (i + 2);
                const auto bis    = flach->getReference (i + 3);
                if (! quelle.isString() || ! istHex32 (quelle.toString()))
                {
                    grund = "MainProject.intent_protections_v1 contains an invalid source id";
                    return false;
                }
                Schutzangabe eintrag;
                eintrag.quelleId = quelle.toString();
                if (! eig.isString() || ! schutzeigenschaftAusWort (eig.toString(), eintrag.eigenschaft))
                {
                    grund = "MainProject.intent_protections_v1 contains an unknown trait: " + eig.toString();
                    return false;
                }
                if (! von.isInt() || ! bis.isInt())
                {
                    grund = "MainProject.intent_protections_v1 band bounds must be integers";
                    return false;
                }
                eintrag.bandVon = static_cast<int> (von);
                eintrag.bandBis = static_cast<int> (bis);
                if (! schutzbereichGueltig (eintrag.eigenschaft, eintrag.bandVon, eintrag.bandBis))
                {
                    grund = "MainProject.intent_protections_v1 band bounds are outside the evidence grid";
                    return false;
                }
                const auto schluessel = (eintrag.quelleId + "|" + juce::String (wort (eintrag.eigenschaft))
                                         + "|" + juce::String (eintrag.bandVon)
                                         + "|" + juce::String (eintrag.bandBis)).toStdString();
                if (! gesehen.insert (schluessel).second)
                {
                    grund = "MainProject.intent_protections_v1 contains a duplicate entry";
                    return false;
                }
                mainSchutz.push_back (eintrag);
            }
        }

        /*  Die gerichteten Beziehungen. Der scharfe Riegel steht am Ende:
            ein Zyklus im `fuehrt_vor`-Teilgraphen macht den Stand
            read-only. §37.4 woertlich: "Zyklische Entmaskierungsprioritaeten
            koennen nicht angewendet werden" - ein Zyklus, den der Leser
            annaehme, waere genau das. Als `gleichrangig` markierte Kanten
            zaehlen dabei nicht mit; sie SIND der aufgeloeste Zyklus. */
        if (mainProject.hasProperty (kMainBeziehungen))
        {
            const auto wert = mainProject.getProperty (kMainBeziehungen);
            const auto* flach = wert.getArray();
            if (flach == nullptr || flach->size() % 3 != 0
                || flach->size() > maxIntentBeziehungen * 3)
            {
                grund = "MainProject.intent_relations_v1 must be an array of triples with at most 256 entries";
                return false;
            }
            std::set<std::string> gesehen;
            for (int i = 0; i < flach->size(); i += 3)
            {
                const auto a   = flach->getReference (i);
                const auto b   = flach->getReference (i + 1);
                const auto art = flach->getReference (i + 2);
                if (! a.isString() || ! istHex32 (a.toString())
                    || ! b.isString() || ! istHex32 (b.toString()))
                {
                    grund = "MainProject.intent_relations_v1 contains an invalid source id";
                    return false;
                }
                if (a.toString() == b.toString())
                {
                    grund = "MainProject.intent_relations_v1 contains a self relation";
                    return false;
                }
                IntentBeziehung kante;
                kante.quelleA = a.toString();
                kante.quelleB = b.toString();
                if (! art.isString() || ! beziehungsartAusWort (art.toString(), kante.art))
                {
                    grund = "MainProject.intent_relations_v1 contains an unknown relation: " + art.toString();
                    return false;
                }
                const auto schluessel = (kante.quelleA + "|" + kante.quelleB).toStdString();
                if (! gesehen.insert (schluessel).second)
                {
                    grund = "MainProject.intent_relations_v1 contains two relations for the same ordered pair";
                    return false;
                }
                mainKanten.push_back (kante);
            }
            if (hatZyklus (mainKanten))
            {
                grund = "MainProject.intent_relations_v1 contains a cycle in fuehrt_vor";
                return false;
            }
        }

        if (mainProject.hasProperty (kMainIntentRev))
        {
            const auto wert = mainProject.getProperty (kMainIntentRev);
            if (! (wert.isInt() || wert.isInt64()))
            {
                grund = "MainProject.intent_revision_v1 must be an integer";
                return false;
            }
            mainIntentRevision = static_cast<juce::int64> (wert);
            if (mainIntentRevision < 1)
            {
                grund = "MainProject.intent_revision_v1 must be at least 1";
                return false;
            }
        }
        /*  Ein Bestand ohne Revision, der Inhalt traegt, ist kein gueltiger
            Stand: die Vollstaendigkeitsmarke aus M-86 haette keine Zahl.
            Umgekehrt ist eine Revision ohne Inhalt zulaessig - das ist der
            leere Bestand, den M-86 ausdruecklich gemeldet haben will. */
        if (mainIntentRevision == 0
            && (! mainIntents.empty() || ! mainSchutz.empty() || ! mainKanten.empty()))
        {
            grund = "MainProject carries intent data without intent_revision_v1";
            return false;
        }

        /*  SONDE-014 Etappe G: der Assistentenschritt (§46.1, E-07).

            ⚠️ Der schaerfste Riegel dieser Etappe steht hier: ein GESPEICHERTER
            Schritt mit `preview` ist in P5 ein LESEFEHLER — kein stiller
            Sprung auf `proposal` oder `remeasure`. Der Zustand ist im Vertrag
            und wird in P6 gebraucht; ihn beim Laden auf einen Nachbarn
            abzubilden hiesse, dem User einen Zustand zu erzaehlen, den er nie
            hatte. */
        if (mainProject.hasProperty (kMainAssistent))
        {
            const auto* liste = mainProject.getProperty (kMainAssistent).getArray();
            if (liste == nullptr || liste->size() != 8)
            {
                grund = "MainProject.assistant_step_v1 must be a list of eight values";
                return false;
            }
            const auto stepId = (*liste)[0];
            const auto schrittWort = (*liste)[1];
            const auto revision = (*liste)[2];
            const auto offen = (*liste)[3];
            if (! stepId.isString() || ! istHex32 (stepId.toString()))
            {
                grund = "MainProject.assistant_step_v1 contains an invalid step id";
                return false;
            }
            if (! schrittWort.isString()
                || ! assistentenschrittAus (schrittWort.toString(), mainAssistent.schritt))
            {
                grund = "MainProject.assistant_step_v1 contains an unknown step";
                return false;
            }
            if (mainAssistent.schritt == Assistentenschritt::preview)
            {
                grund = "MainProject.assistant_step_v1 carries preview, which P5 cannot reach";
                return false;
            }
            if (! (revision.isInt() || revision.isInt64())
                || static_cast<juce::int64> (revision) < 1)
            {
                grund = "MainProject.assistant_step_v1 revision must be at least 1";
                return false;
            }
            if (! offen.isBool())
            {
                grund = "MainProject.assistant_step_v1 open flag must be boolean";
                return false;
            }
            const char* namen[3] = { "finding id", "proposal id", "experiment id" };
            juce::String* ziele[3] = { &mainAssistent.findingId,
                                       &mainAssistent.proposalId,
                                       &mainAssistent.experimentId };
            for (int i = 0; i < 3; ++i)
            {
                const auto wert = (*liste)[4 + i];
                if (! wert.isString())
                {
                    grund = juce::String ("MainProject.assistant_step_v1 ") + namen[i]
                          + " must be a string";
                    return false;
                }
                const auto text = wert.toString();
                // Leer heisst „nicht gesetzt" — das ist etwas anderes als eine
                // ungueltige Kennung und muss unterscheidbar bleiben.
                if (text.isNotEmpty() && ! istHex32 (text))
                {
                    grund = juce::String ("MainProject.assistant_step_v1 ") + namen[i]
                          + " must be 32 lowercase hex digits or empty";
                    return false;
                }
                *ziele[i] = text;
            }
            const auto ergebnisWort = (*liste)[7];
            bool ergebnisBekannt = false;
            for (auto e : { Assistentenergebnis::schritt,
                            Assistentenergebnis::passageMessen,
                            Assistentenergebnis::routingBestaetigen,
                            Assistentenergebnis::keineAenderungEmpfohlen })
            {
                if (ergebnisWort.isString() && ergebnisWort.toString() == wort (e))
                {
                    mainAssistent.ergebnis = e;
                    ergebnisBekannt = true;
                    break;
                }
            }
            if (! ergebnisBekannt)
            {
                grund = "MainProject.assistant_step_v1 contains an unknown result";
                return false;
            }
            mainAssistent.gesetzt = true;
            mainAssistent.stepId = stepId.toString();
            mainAssistent.revision = static_cast<juce::int64> (revision);
            mainAssistent.offen = static_cast<bool> (offen);
        }
    }

    parameter::Satz satz {};
    bool layoutV1 = false;
    bool hatDsp = false;
    juce::int64 stateRevision = 0;
    std::vector<parameter::Schutzzone> zonen;
    std::vector<UndoEintrag> undoRing;
    int undoCursor = 0;
    if (istAktiv)
    {
        const auto p = v.getChildWithName (kParameters);
        if (! schemaIst (p, kParamSchema)) { grund = "Parameters schema is unknown to this version (it reads schema 1)"; return false; }
        if (! parameter::leseAusBaum (p, satz, layoutV1, grund)) return false;

        /*  SONDE-015: das Kind `Dsp`.

            Es ist optional. Fehlt es, ist der Zustand frisch (Revision 0,
            keine Zone, kein Undo) - und bei Layout v1 setzt die Migration
            danach `occupied` aus `enabled` und den Werten (R5). Ein Stand im
            Layout v2 OHNE `Dsp` heisst dagegen ausdruecklich "kein Slot
            belegt": dort hat der Schreiber das Kind weggelassen, weil es
            nichts zu sagen hatte. */
        const auto d = v.getChildWithName (kDsp);
        if (d.isValid())
        {
            /*  Ein Knoten OHNE `dsp_schema_version` stammt von einem Build,
                der das Kind `Dsp` nicht kannte - er kann es nicht geschrieben
                haben. Beides zusammen ist ein Widerspruch, und die Migration
                unten wuerde die gelesene Belegung stumm ueberschreiben.
                Read-only mit Originalbytes ist die ehrliche Antwort: kein
                Teilstate, kein Raten (§53.8). */
            if (layoutV1)
            {
                grund = "Parameters is layout 1 but a Dsp child is present";
                return false;
            }
            hatDsp = true;
            if (! schemaIst (d, kDspSchema)) { grund = "Dsp schema is unknown to this version (it reads schema 1)"; return false; }
            if (! leseDspKind (d, satz, stateRevision, zonen, undoRing, undoCursor, grund)) return false;
        }

        if (layoutV1)
        {
            // R5, Migration v1 -> v2: occupied := enabled ODER mindestens ein
            // Wert des Slots weicht BITGENAU vom Vertragsdefault ab. Ein
            // v1-Stand verliert damit kein vom User gesetztes Band.
            parameter::setzeOccupiedAusV1 (satz);
        }
    }

    aus.baum = v;
    aus.common = c;
    aus.mainProjectMitglieder = std::move (mainMitglieder);
    aus.manuellePassagen      = std::move (mainPassagen);
    aus.sourceIntents         = std::move (mainIntents);
    aus.schutzangaben         = std::move (mainSchutz);
    aus.intentBeziehungen     = std::move (mainKanten);
    aus.intentBestandRevision = mainIntentRevision;
    aus.assistent = mainAssistent;
    aus.hatParameters = istAktiv;
    aus.parameters = satz;
    aus.hatDsp = hatDsp;
    aus.stateRevision = stateRevision;
    aus.schutzZonen = std::move (zonen);
    aus.undoRing = std::move (undoRing);
    aus.undoCursor = undoCursor;
    aus.layoutV1Migriert = layoutV1;
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

// ── SONDE-014 Etappe A: die Produkt-API des Intents ────────────────────────

namespace
{
/*  Die Bestandsrevision steigt bei JEDER persistenten Aenderung an einem der
    drei Bestandteile - und zwar genau einmal je Aenderung. Sie ist die Zahl,
    die die Vollstaendigkeitsmarke aus M-86 traegt; ein Bestand, dessen
    Revision nicht steigt, saehe fuer den Broker aus wie "nichts passiert".
    Der obere Rand ist kein Ueberlauf, sondern ein Halt: `int64` reicht fuer
    9,2 Trillionen Aenderungen, und ein Wrap waere eine ruecklaufende
    Revision - genau das, was M-85 verbietet. */
bool bestandsrevisionHeben (Zustand& z, juce::String& grund)
{
    if (z.intentBestandRevision >= std::numeric_limits<juce::int64>::max())
    {
        grund = "intent revision would overflow";
        return false;
    }
    ++z.intentBestandRevision;
    return true;
}

bool passageScopeGueltig (const juce::String& passageId)
{
    return passageId.isEmpty() || istHex32 (passageId);
}
} // namespace

bool setzeIntent (Zustand& z, const juce::String& quelleId, const juce::String& passageId,
                  Rolle rolle, IntentHerkunft herkunft, double konfidenz,
                  bool& veraendert, juce::String& grund)
{
    veraendert = false;
    if (! istHex32 (quelleId))         { grund = "source id is not hex32"; return false; }
    if (! passageScopeGueltig (passageId)) { grund = "passage scope is neither empty nor hex32"; return false; }
    if (! std::isfinite (konfidenz) || konfidenz < 0.0 || konfidenz > 1.0)
    {
        grund = "confidence must be finite within [0,1]"; return false;
    }

    auto treffer = std::find_if (z.sourceIntents.begin(), z.sourceIntents.end(),
        [&] (const SourceIntent& s) { return s.quelleId == quelleId && s.passageId == passageId; });

    if (treffer == z.sourceIntents.end())
    {
        if (static_cast<int> (z.sourceIntents.size()) >= maxSourceIntents)
        {
            grund = "source_intents_v1 is full"; return false;
        }
        if (! bestandsrevisionHeben (z, grund)) return false;
        z.sourceIntents.push_back ({ quelleId, passageId, rolle, 1, herkunft, konfidenz });
        veraendert = true;
        return true;
    }

    /*  M-07: eine abgeleitete Vermutung ueberschreibt einen Userwert NIE.
        Das ist kein Fehler des Aufrufers, sondern die Regel - deshalb `true`
        mit `veraendert = false`, und deshalb meldet der Aufrufer kein
        Host-Dirty. Ein `false` haette dieselbe Wirkung, waere aber eine
        Fehlermeldung ueber ein korrektes Verhalten. */
    if (herkunft == IntentHerkunft::abgeleitet && treffer->herkunft == IntentHerkunft::user)
        return true;

    if (treffer->rolle == rolle && treffer->herkunft == herkunft && treffer->konfidenz == konfidenz)
        return true;   // No-op: keine Revision, kein Dirty.

    if (treffer->revision >= std::numeric_limits<juce::int64>::max())
    {
        grund = "intent revision would overflow"; return false;
    }
    if (! bestandsrevisionHeben (z, grund)) return false;
    treffer->rolle     = rolle;
    treffer->herkunft  = herkunft;
    treffer->konfidenz = konfidenz;
    ++treffer->revision;
    veraendert = true;
    return true;
}

bool entferneIntent (Zustand& z, const juce::String& quelleId, const juce::String& passageId,
                     bool& veraendert, juce::String& grund)
{
    veraendert = false;
    if (! istHex32 (quelleId))             { grund = "source id is not hex32"; return false; }
    if (! passageScopeGueltig (passageId)) { grund = "passage scope is neither empty nor hex32"; return false; }

    const auto vorher = z.sourceIntents.size();
    z.sourceIntents.erase (std::remove_if (z.sourceIntents.begin(), z.sourceIntents.end(),
        [&] (const SourceIntent& s) { return s.quelleId == quelleId && s.passageId == passageId; }),
        z.sourceIntents.end());
    if (z.sourceIntents.size() == vorher)
        return true;
    if (! bestandsrevisionHeben (z, grund)) return false;
    veraendert = true;
    return true;
}

const SourceIntent* findeIntent (const Zustand& z, const juce::String& quelleId,
                                 const juce::String& passageId)
{
    for (const auto& s : z.sourceIntents)
        if (s.quelleId == quelleId && s.passageId == passageId)
            return &s;
    return nullptr;
}

const SourceIntent* wirkenderIntent (const Zustand& z, const juce::String& quelleId,
                                     const juce::String& passageId)
{
    // §37.2 Stufe 3, woertlich: passagespezifischer VOR globalem Intent.
    if (passageId.isNotEmpty())
        if (const auto* speziell = findeIntent (z, quelleId, passageId))
            return speziell;
    return findeIntent (z, quelleId, {});
}

bool setzeSchutzangabe (Zustand& z, const juce::String& quelleId, Schutzeigenschaft eigenschaft,
                        int bandVon, int bandBis, bool& veraendert, juce::String& grund)
{
    veraendert = false;
    if (! istHex32 (quelleId)) { grund = "source id is not hex32"; return false; }
    if (! schutzbereichGueltig (eigenschaft, bandVon, bandBis))
    {
        grund = "band bounds are outside the evidence grid"; return false;
    }
    const Schutzangabe eintrag { quelleId, eigenschaft, bandVon, bandBis };
    if (std::find (z.schutzangaben.begin(), z.schutzangaben.end(), eintrag) != z.schutzangaben.end())
        return true;   // No-op.
    if (static_cast<int> (z.schutzangaben.size()) >= maxSchutzangaben)
    {
        grund = "intent_protections_v1 is full"; return false;
    }
    if (! bestandsrevisionHeben (z, grund)) return false;
    z.schutzangaben.push_back (eintrag);
    veraendert = true;
    return true;
}

bool entferneSchutzangabe (Zustand& z, const juce::String& quelleId, Schutzeigenschaft eigenschaft,
                           int bandVon, int bandBis, bool& veraendert, juce::String& grund)
{
    veraendert = false;
    if (! istHex32 (quelleId)) { grund = "source id is not hex32"; return false; }
    const Schutzangabe eintrag { quelleId, eigenschaft, bandVon, bandBis };
    const auto vorher = z.schutzangaben.size();
    z.schutzangaben.erase (std::remove (z.schutzangaben.begin(), z.schutzangaben.end(), eintrag),
                           z.schutzangaben.end());
    if (z.schutzangaben.size() == vorher)
        return true;
    if (! bestandsrevisionHeben (z, grund)) return false;
    veraendert = true;
    return true;
}

/*  M-06: die Zyklenpruefung.

    Tiefensuche mit drei Farben ueber den `fuehrtVor`-Teilgraphen. `gleichrangig`
    und `darfVerschmelzen` sind KEINE Prioritaetskanten und zaehlen nicht mit -
    genau das macht `speichereAlsGleichrangigkeit` zum gueltigen Ausweg aus
    einem Zyklus statt zu einer zweiten Wahrheit.

    Rein und ohne Zustand, damit Leser und Schreiber denselben Riegel rufen. */
bool hatZyklus (const std::vector<IntentBeziehung>& kanten)
{
    std::map<juce::String, std::vector<juce::String>> nachfolger;
    for (const auto& k : kanten)
        if (k.art == Beziehungsart::fuehrtVor)
            nachfolger[k.quelleA].push_back (k.quelleB);

    enum class Farbe { weiss, grau, schwarz };
    std::map<juce::String, Farbe> farbe;

    // Iterative Tiefensuche: eine rekursive risse bei 256 Kanten zwar nicht
    // den 1-MiB-Stack von MSVC, aber der Riegel laeuft auch im Leser - und
    // dort ist die Kantenzahl eine EINGABE (NAK-175, R2 des Bauplans).
    std::vector<std::pair<juce::String, size_t>> stapel;
    for (const auto& start : nachfolger)
    {
        if (farbe[start.first] != Farbe::weiss)
            continue;
        stapel.push_back ({ start.first, 0 });
        farbe[start.first] = Farbe::grau;
        while (! stapel.empty())
        {
            auto& oben = stapel.back();
            const auto it = nachfolger.find (oben.first);
            if (it == nachfolger.end() || oben.second >= it->second.size())
            {
                farbe[oben.first] = Farbe::schwarz;
                stapel.pop_back();
                continue;
            }
            const auto naechster = it->second[oben.second++];
            const auto f = farbe.count (naechster) ? farbe[naechster] : Farbe::weiss;
            if (f == Farbe::grau)   return true;
            if (f == Farbe::schwarz) continue;
            farbe[naechster] = Farbe::grau;
            stapel.push_back ({ naechster, 0 });
        }
    }
    return false;
}

bool setzeBeziehung (Zustand& z, const juce::String& quelleA, const juce::String& quelleB,
                     Beziehungsart art, bool& veraendert, juce::String& grund)
{
    veraendert = false;
    if (! istHex32 (quelleA) || ! istHex32 (quelleB))
    {
        grund = "source id is not hex32"; return false;
    }
    if (quelleA == quelleB) { grund = "a relation needs two different sources"; return false; }

    auto treffer = std::find_if (z.intentBeziehungen.begin(), z.intentBeziehungen.end(),
        [&] (const IntentBeziehung& k) { return k.quelleA == quelleA && k.quelleB == quelleB; });
    if (treffer != z.intentBeziehungen.end() && treffer->art == art)
        return true;   // No-op.

    // Reihenfolge nach M-06: PRUEFEN, dann entscheiden, dann persistieren.
    // Der Kandidat entsteht als Kopie; erst wenn er zyklenfrei ist, wird der
    // Bestand angefasst. Ein Zyklus erreicht die Persistenz nie unmarkiert.
    auto kandidat = z.intentBeziehungen;
    auto kt = std::find_if (kandidat.begin(), kandidat.end(),
        [&] (const IntentBeziehung& k) { return k.quelleA == quelleA && k.quelleB == quelleB; });
    if (kt != kandidat.end()) kt->art = art;
    else
    {
        if (static_cast<int> (kandidat.size()) >= maxIntentBeziehungen)
        {
            grund = "intent_relations_v1 is full"; return false;
        }
        kandidat.push_back ({ quelleA, quelleB, art });
    }
    if (hatZyklus (kandidat))
    {
        grund = "this relation would create a cycle in fuehrt_vor";
        return false;
    }
    if (! bestandsrevisionHeben (z, grund)) return false;
    z.intentBeziehungen = std::move (kandidat);
    veraendert = true;
    return true;
}

bool speichereAlsGleichrangigkeit (Zustand& z, const juce::String& quelleA,
                                   const juce::String& quelleB,
                                   bool& veraendert, juce::String& grund)
{
    return setzeBeziehung (z, quelleA, quelleB, Beziehungsart::gleichrangig, veraendert, grund);
}

bool entferneBeziehung (Zustand& z, const juce::String& quelleA, const juce::String& quelleB,
                        bool& veraendert, juce::String& grund)
{
    veraendert = false;
    if (! istHex32 (quelleA) || ! istHex32 (quelleB))
    {
        grund = "source id is not hex32"; return false;
    }
    const auto vorher = z.intentBeziehungen.size();
    z.intentBeziehungen.erase (std::remove_if (z.intentBeziehungen.begin(), z.intentBeziehungen.end(),
        [&] (const IntentBeziehung& k) { return k.quelleA == quelleA && k.quelleB == quelleB; }),
        z.intentBeziehungen.end());
    if (z.intentBeziehungen.size() == vorher)
        return true;
    if (! bestandsrevisionHeben (z, grund)) return false;
    veraendert = true;
    return true;
}

/*  E-02, M-04: das Veto und sein spezifischeres Gegenstueck.

    Die Rolle `verschmolzen` an einer der beiden Quellen ist ein GLOBALES
    Veto gegen jede Entmaskierungsempfehlung mit Beteiligung dieser Quelle.
    Eine ausdrueckliche gerichtete Beziehung `A fuehrt vor B` ist das
    SPEZIFISCHERE Werkzeug und hebt das Veto NUR FUER DIESES PAAR auf - beide
    liegen auf Stufe 2, spezifisch vor global.

    Die Richtung zaehlt in beiden Leserichtungen: sowohl `A fuehrt vor B` als
    auch `B fuehrt vor A` ist eine ausdrueckliche Aussage ueber genau dieses
    Paar. Was NICHT gilt, ist eine Beziehung, an der eine dritte Quelle
    beteiligt ist - sonst hoebe eine Kante das Veto fuer ein fremdes Paar mit
    auf, und genau das ist der zweite Rotbeweis von M-04. */
bool entmaskierungErlaubt (const Zustand& z, const juce::String& quelleA,
                           const juce::String& quelleB, const juce::String& passageId)
{
    const auto vetoAn = [&] (const juce::String& quelle)
    {
        const auto* intent = wirkenderIntent (z, quelle, passageId);
        return intent != nullptr && belegung (intent->rolle).veto == Veto::verschmolzen;
    };
    if (! vetoAn (quelleA) && ! vetoAn (quelleB))
        return true;

    for (const auto& k : z.intentBeziehungen)
    {
        if (k.art != Beziehungsart::fuehrtVor)
            continue;
        const auto passt = (k.quelleA == quelleA && k.quelleB == quelleB)
                        || (k.quelleA == quelleB && k.quelleB == quelleA);
        if (passt)
            return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════════════════
// SONDE-014 Etappe G: der `AssistantStep` (§46.1, M-55 bis M-62)
// ═══════════════════════════════════════════════════════════════════════════

const char* wort (Assistentenschritt s) noexcept
{
    switch (s)
    {
        case Assistentenschritt::coverage:  return "coverage";
        case Assistentenschritt::finding:   return "finding";
        case Assistentenschritt::evidence:  return "evidence";
        case Assistentenschritt::listen:    return "listen";
        case Assistentenschritt::proposal:  return "proposal";
        case Assistentenschritt::preview:   return "preview";
        case Assistentenschritt::remeasure: return "remeasure";
        case Assistentenschritt::verdict:   return "verdict";
    }
    return "coverage";
}

bool assistentenschrittAus (const juce::String& w, Assistentenschritt& aus) noexcept
{
    static const std::pair<const char*, Assistentenschritt> tafel[] = {
        { "coverage",  Assistentenschritt::coverage  },
        { "finding",   Assistentenschritt::finding   },
        { "evidence",  Assistentenschritt::evidence  },
        { "listen",    Assistentenschritt::listen    },
        { "proposal",  Assistentenschritt::proposal  },
        { "preview",   Assistentenschritt::preview   },
        { "remeasure", Assistentenschritt::remeasure },
        { "verdict",   Assistentenschritt::verdict   },
    };
    for (const auto& [name, wert] : tafel)
        if (w == name) { aus = wert; return true; }
    return false;
}

const char* wort (Assistentenergebnis e) noexcept
{
    switch (e)
    {
        case Assistentenergebnis::schritt:                 return "schritt";
        case Assistentenergebnis::passageMessen:           return "passage_messen";
        case Assistentenergebnis::routingBestaetigen:      return "routing_bestaetigen";
        case Assistentenergebnis::keineAenderungEmpfohlen: return "keine_aenderung_empfohlen";
    }
    return "schritt";
}

const char* wort (Userurteil u) noexcept
{
    switch (u)
    {
        case Userurteil::angenommen: return "angenommen";
        case Userurteil::abgelehnt:  return "abgelehnt";
        case Userurteil::spaeter:    return "spaeter";
        case Userurteil::enthaltung: return "enthaltung";
    }
    return "enthaltung";
}

bool Schrittvertrag::vollstaendig() const noexcept
{
    // Fuenf Angaben, keine optional (M-56). Ein Zustand ohne Timeout oder ohne
    // Rueckkante ist ein Vertragsbruch — und `rueckkante` ist deshalb kein
    // `optional`, sondern immer ein Zustand.
    return eintritt != nullptr && *eintritt != '\0'
        && useraktion != nullptr && *useraktion != '\0'
        && timeoutMs > 0
        && static_cast<int> (rueckkante) >= 0;
}

Schrittvertrag schrittvertrag (Assistentenschritt s) noexcept
{
    // ⚠️ Die Timeouts sind PRODUKTGRENZEN, keine Messung. Sie stehen als
    // Zahlen hier und nicht in `metriken-v1.json`: eine Kalibrierung waere
    // eine Entscheidung ueber Bedienung, nicht ueber Messgenauigkeit. Die
    // Groessenordnung folgt der Handlung — hoeren dauert laenger als lesen.
    switch (s)
    {
        case Assistentenschritt::coverage:
            return { s, "sitzung hat mindestens eine bestaetigte quelle", false,
                     "quellen bestaetigen", 60000, Assistentenschritt::coverage };
        case Assistentenschritt::finding:
            return { s, "coverage abgeschlossen", true,
                     "befund waehlen", 60000, Assistentenschritt::coverage };
        case Assistentenschritt::evidence:
            return { s, "ein befund ist gewaehlt", true,
                     "beleg pruefen", 60000, Assistentenschritt::finding };
        case Assistentenschritt::listen:
            return { s, "beleg gesichtet", true,
                     "hoeren", 120000, Assistentenschritt::evidence };
        case Assistentenschritt::proposal:
            return { s, "hoereindruck liegt vor", true,
                     "vorschlag pruefen", 60000, Assistentenschritt::listen };
        case Assistentenschritt::preview:
            // ⚠️ Der Zustand EXISTIERT und traegt seinen vollstaendigen
            // Vertrag — P5 fuehrt nur keine Kante dorthin (E-07). Ihn hier
            // leer zu lassen hiesse, ihn halb zu streichen.
            return { s, "eigene dsp-faehigkeit vorhanden (ab P6)", true,
                     "preview hoeren", 60000, Assistentenschritt::proposal };
        case Assistentenschritt::remeasure:
            return { s, "aenderung ausgefuehrt", true,
                     "nachmessen", 120000, Assistentenschritt::proposal };
        case Assistentenschritt::verdict:
            return { s, "nachmessung liegt vor", true,
                     "urteilen", 120000, Assistentenschritt::remeasure };
    }
    return { s, "", false, "", 0, s };
}

bool p5Naechster (Assistentenschritt von, Assistentenschritt& aus) noexcept
{
    // Die P5-Folge, wortgleich mit E-07:
    //   coverage → finding → evidence → listen → proposal → remeasure → verdict
    // OHNE `preview`. Der Zustand steht in der Menge, die Tabelle fuehrt
    // keine Kante dorthin.
    switch (von)
    {
        case Assistentenschritt::coverage:  aus = Assistentenschritt::finding;   return true;
        case Assistentenschritt::finding:   aus = Assistentenschritt::evidence;  return true;
        case Assistentenschritt::evidence:  aus = Assistentenschritt::listen;    return true;
        case Assistentenschritt::listen:    aus = Assistentenschritt::proposal;  return true;
        case Assistentenschritt::proposal:  aus = Assistentenschritt::remeasure; return true;
        case Assistentenschritt::remeasure: aus = Assistentenschritt::verdict;   return true;
        case Assistentenschritt::verdict:   return false;   // terminal
        case Assistentenschritt::preview:   return false;   // in P5 unerreichbar
    }
    return false;
}

bool p5UebergangErlaubt (Assistentenschritt von, Assistentenschritt nach) noexcept
{
    // Kein Weg nach `preview` — weder vorwaerts noch als Rueckkante.
    if (nach == Assistentenschritt::preview || von == Assistentenschritt::preview)
        return false;
    Assistentenschritt weiter {};
    if (p5Naechster (von, weiter) && weiter == nach)
        return true;
    // Die sichere Rueckkante ist ebenfalls eine Kante der Tabelle: ohne sie
    // waere „Zurueck" ein Sprung ausserhalb des Automaten.
    return schrittvertrag (von).rueckkante == nach && von != nach;
}

namespace
{
/// Der gemeinsame Weg jeder angenommenen Aenderung am Schritt.
///
/// Die Revision steigt GENAU EINMAL je Aenderung; ein No-op meldet nichts
/// (M-13: „No-op, abgewiesener Wert, Laden und read-only schweigen").
void schrittAendern (Assistentenzustand& a, Assistentenschritt neu, bool offen,
                     bool& veraendert)
{
    if (a.gesetzt && a.schritt == neu && a.offen == offen)
    {
        veraendert = false;
        return;
    }
    a.schritt = neu;
    a.offen = offen;
    a.revision += 1;
    veraendert = true;
}
} // namespace

bool setzeAssistentenschritt (Zustand& z, const juce::String& stepId,
                              Assistentenschritt schritt, bool& veraendert,
                              juce::String& grund)
{
    veraendert = false;
    if (! istHex32 (stepId))
    {
        grund = "assistant step id must be 32 lowercase hex digits";
        return false;
    }
    // E-07/M-55: `preview` ist in P5 nicht erreichbar — auch nicht als
    // Startzustand. Ein Automat, der ihn betreten koennte, haette die Kante,
    // die die Tabelle ausdruecklich nicht fuehrt.
    if (schritt == Assistentenschritt::preview)
    {
        grund = "assistant step preview has no P5 transition";
        return false;
    }

    auto& a = z.assistent;
    const bool neuerSchritt = ! a.gesetzt || a.stepId != stepId || ! a.offen;
    if (neuerSchritt)
    {
        // M-57: EIN Slot. Ein zweiter Startversuch bei OFFENEM Schritt wird
        // abgewiesen, nicht eingereiht.
        if (a.gesetzt && a.offen && a.stepId != stepId)
        {
            grund = "another assistant step is still open";
            return false;
        }
        // Ein neuer Schritt beginnt bei `coverage` — der Anfang der Folge.
        if (schritt != Assistentenschritt::coverage)
        {
            grund = "a new assistant step must start at coverage";
            return false;
        }
        a.gesetzt = true;
        a.stepId = stepId;
        a.schritt = schritt;
        a.offen = true;
        a.revision += 1;
        a.ergebnis = Assistentenergebnis::schritt;
        a.findingId.clear();
        a.proposalId.clear();
        a.experimentId.clear();
        veraendert = true;
        return true;
    }

    if (! p5UebergangErlaubt (a.schritt, schritt))
    {
        grund = juce::String ("no P5 transition from ") + wort (a.schritt)
              + " to " + wort (schritt);
        return false;
    }
    schrittAendern (a, schritt, true, veraendert);
    return true;
}

bool assistentAbbrechen (Zustand& z, bool& veraendert, juce::String& grund)
{
    veraendert = false;
    auto& a = z.assistent;
    if (! a.gesetzt || ! a.offen)
    {
        grund = "no open assistant step to abort";
        return false;
    }
    // ⚠️ TERMINAL, nicht geloescht. §46.1: „Verwerfen ist ein terminales
    // Ereignis, kein Loeschen der Historie." Der Schritt bleibt stehen und
    // traegt `offen = false`; wer ihn entfernte, naehme dem User die Spur
    // seiner eigenen Entscheidung.
    schrittAendern (a, a.schritt, false, veraendert);
    return true;
}

bool assistentZurueck (Zustand& z, bool& veraendert, juce::String& grund)
{
    veraendert = false;
    auto& a = z.assistent;
    if (! a.gesetzt || ! a.offen)
    {
        grund = "no open assistant step to step back";
        return false;
    }
    const auto ziel = schrittvertrag (a.schritt).rueckkante;
    if (ziel == a.schritt)
    {
        // Der erste Zustand zeigt auf sich selbst — die sichere Rueckkante
        // heisst dort „bleib, wo du bist". Das ist kein Fehler, aber auch
        // keine Aenderung.
        return true;
    }
    schrittAendern (a, ziel, true, veraendert);
    return true;
}

bool assistentUeberspringen (Zustand& z, bool& veraendert, juce::String& grund)
{
    veraendert = false;
    auto& a = z.assistent;
    if (! a.gesetzt || ! a.offen)
    {
        grund = "no open assistant step to skip";
        return false;
    }
    Assistentenschritt weiter {};
    if (! p5Naechster (a.schritt, weiter))
    {
        grund = "the last assistant step cannot be skipped";
        return false;
    }
    schrittAendern (a, weiter, true, veraendert);
    return true;
}

bool assistentResume (const Zustand& z, Assistentenzustand& aus)
{
    // Resume ist eine FRAGE, keine Aenderung: es setzt nichts und hebt keine
    // Revision. Ein terminaler Schritt wird nicht fortgesetzt — er wird neu
    // begonnen oder bleibt Historie.
    if (! z.assistent.gesetzt || ! z.assistent.offen)
        return false;
    aus = z.assistent;
    return true;
}

bool setzeAssistentenergebnis (Zustand& z, Assistentenergebnis ergebnis,
                               bool& veraendert, juce::String& grund)
{
    veraendert = false;
    auto& a = z.assistent;
    if (! a.gesetzt || ! a.offen)
    {
        grund = "no open assistant step to answer";
        return false;
    }
    if (a.ergebnis == ergebnis)
        return true;
    // §46.2: die drei Antworten sind eigene, benannte ERGEBNISSE mit Objekt —
    // der Schritt bleibt stehen, bekommt aber seine Antwort und eine neue
    // Revision. Ein Leerzustand haette keine.
    a.ergebnis = ergebnis;
    a.revision += 1;
    veraendert = true;
    return true;
}

double schrittrang (const Schrittkandidat& k) noexcept
{
    const auto klemme = [] (double v) { return std::isfinite (v) ? std::clamp (v, 0.0, 1.0) : 0.0; };
    // Gleichgewichtet ueber fuenf Groessen; `messkosten` invers, weil §46.2
    // ausdruecklich mit dem KLEINSTEN hochrelevanten, reversiblen Test
    // beginnen will. Ein erfundenes Gewicht waere eine unkalibrierte Zahl.
    const double summe = klemme (k.erwarteterNutzen)
                       + klemme (k.intentRelevanz)
                       + klemme (k.konfidenz)
                       + klemme (k.reversibilitaet)
                       + (1.0 - klemme (k.messkosten));
    return summe / 5.0;
}

std::vector<Schrittkandidat> ordneSchritte (const std::vector<Schrittkandidat>& kandidaten)
{
    std::vector<Schrittkandidat> aus;
    aus.reserve (kandidaten.size());
    for (const auto& k : kandidaten)
    {
        // ⚠️ HARTE GATES VOR DER GEWICHTUNG (M-60). Ein Kandidat mit
        // perfektem Nutzen und gerissener Vergleichbarkeit erreicht die
        // Gewichtung NIE — sie wird fuer ihn gar nicht gerechnet.
        if (! k.vergleichbar || ! k.sicher || k.bereitsErfolglos)
            continue;
        aus.push_back (k);
    }
    std::sort (aus.begin(), aus.end(), [] (const auto& a, const auto& b)
    {
        const auto ra = schrittrang (a);
        const auto rb = schrittrang (b);
        if (ra != rb) return ra > rb;
        // Bei Gleichstand die kleinere `findingId` — eine stabile Wahl, keine
        // zufaellige.
        return a.findingId.compare (b.findingId) < 0;
    });
    return aus;
}

juce::String alsText (const Zustand& z)
{
    if (z.nurLesen)
        return "read-only (" + z.grund + ")\n" + (z.baum.isValid() ? z.baum.toXmlString() : juce::String ("<kein Baum>"));
    return synchronisiert (z).toXmlString();
}

} // namespace nakama::state

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
