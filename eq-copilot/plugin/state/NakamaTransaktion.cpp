#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 - keine JucePlugin_*-Konstante im Kern
#include "NakamaTransaktion.h"
#include "NakamaPreset.h"
#include "DspKern.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <utility>

namespace nakama::transaktion
{

namespace param = nakama::parameter;

//==============================================================================
const char* wort (Stufe s) noexcept
{
    switch (s)
    {
        case Stufe::s0:    return "S0";
        case Stufe::s1:    return "S1";
        case Stufe::s2:    return "S2";
        case Stufe::s3:    return "S3";
        case Stufe::s4:    return "S4";
        case Stufe::s5:    return "S5";
        case Stufe::s6:    return "S6";
        case Stufe::s7:    return "S7";
        case Stufe::s8:    return "S8";
        case Stufe::keine: return "keine";
    }
    return "?";
}

const char* wort (Art a) noexcept
{
    switch (a)
    {
        case Art::apply:          return "apply";
        case Art::revert:         return "revert";
        case Art::neutralisieren: return "neutralisieren";
        case Art::remove:         return "remove";
        case Art::undo:           return "undo";
        case Art::redo:           return "redo";
        case Art::presetLaden:    return "preset_laden";
        case Art::gestus:         return "gestus";
        case Art::bandBelegen:    return "band_belegen";
    }
    return "?";
}

const char* wort (Ausgang a) noexcept
{
    switch (a)
    {
        case Ausgang::commit:                return "commit";
        case Ausgang::gespeichertesErgebnis: return "gespeichertes_ergebnis";
        case Ausgang::konflikt:              return "konflikt";
        case Ausgang::fehler:                return "fehler";
        case Ausgang::userSchutz:            return "user_schutz";
        case Ausgang::busyRetry:             return "busy_retry";
    }
    return "?";
}

bool istGanzzustand (Art a) noexcept
{
    return a == Art::undo || a == Art::redo || a == Art::presetLaden;
}

juce::String alsText (const HashText& h)
{
    return juce::String (h.data(), h.size());
}

bool ausHex (const juce::String& hex, HashText& aus) noexcept
{
    if (hex.length() != (int) aus.size()) return false;
    for (int i = 0; i < (int) aus.size(); ++i)
    {
        const auto c = hex[i];
        if (! ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return false;
        aus[(size_t) i] = (char) c;
    }
    return true;
}

int AutomationOverlay::anzahl() const noexcept
{
    return (int) std::count (gesetzt.begin(), gesetzt.end(), true);
}

//==============================================================================
namespace
{

/** Der Vertragsdefault EINER Zelle - dieselbe Form wie `standardSatz()`,
    damit der bitgenaue Satzvergleich (`DspSatz::operator==`) sie als gleich
    sieht. */
param::Zelle standardZelle (int index)
{
    const auto& b = param::tabelle()[(size_t) index];
    param::Zelle z;
    switch (b.typ)
    {
        case param::Typ::boolean:     z.b = b.standardBool; break;
        case param::Typ::gleitkomma:  z.zahl = b.standardZahl; break;
        case param::Typ::aufzaehlung: z.enumIndex = b.standardIndex; break;
    }
    return z;
}

/** Bitgenau je Typ - dieselbe Gleichheit wie `DspSatz::operator==`. */
bool zelleGleich (int index, const param::Zelle& a, const param::Zelle& b) noexcept
{
    switch (param::tabelle()[(size_t) index].typ)
    {
        case param::Typ::boolean:     return a.b == b.b;
        case param::Typ::gleitkomma:  return std::memcmp (&a.zahl, &b.zahl, sizeof (double)) == 0;
        case param::Typ::aufzaehlung: return a.enumIndex == b.enumIndex;
    }
    return false;
}

bool werteGleich (const param::Satz& a, const param::Satz& b) noexcept
{
    for (int i = 0; i < param::kAnzahl; ++i)
        if (! zelleGleich (i, a[(size_t) i], b[(size_t) i])) return false;
    return true;
}

const param::Schutzzone* zoneMitId (const param::DspSatz& s, int id) noexcept
{
    for (const auto& z : s.zonen)
        if (z.id == id) return &z;
    return nullptr;
}

} // namespace

//== Schutz-Zonen ==============================================================
bool frequenzInZone (double freqHz, const param::Schutzzone& z, double samplerate) noexcept
{
    double f = freqHz, unten = z.lowHz, oben = z.highHz;
    if (samplerate > 0.0)
    {
        // Dieselbe Kappung wie beim Programmbau (`dsp::kappeNyquist`): das Band
        // klingt bei hoechstens 0,45*fs, darueber gibt es keine hoerbare
        // Zonenflaeche mehr (M-12, M-66). Persistent bleibt alles, wie es war.
        const double deckel = samplerate * dsp::kNyquistAnteil;
        f     = std::min (f, deckel);
        unten = std::min (unten, deckel);
        oben  = std::min (oben, deckel);
    }
    return f >= unten && f <= oben;
}

bool liegtInZone (const param::DspSatz& s, int slot, const param::Schutzzone& z, double samplerate) noexcept
{
    if (slot < 0 || slot >= param::kSlots || ! z.enabled) return false;
    const auto& w = s.werte;
    return w[(size_t) param::indexOccupied (slot)].b
        && w[(size_t) param::indexBandV1 (slot, param::kEnabled)].b
        && frequenzInZone (w[(size_t) param::indexBandV1 (slot, param::kFreqHz)].zahl, z, samplerate);
}

int zoneneintritt (const param::DspSatz& vorher, const param::DspSatz& nachher, double samplerate) noexcept
{
    for (int slot = 0; slot < param::kSlots; ++slot)
    {
        const auto iBelegt = (size_t) param::indexOccupied (slot);
        const auto iAn     = (size_t) param::indexBandV1 (slot, param::kEnabled);
        const auto iFreq   = (size_t) param::indexBandV1 (slot, param::kFreqHz);

        // Nur eine BEDIENAENDERUNG an diesem Slot kann ihn in eine Zone fuehren:
        // `occupied`, `enabled` oder `freq_hz` (§5.6.3 Feinheit 3). Eine Zone,
        // die ueber einem liegenden Band eingeschaltet wird, aendert keines
        // davon - sie ist erlaubt und meldet Verletzung (M-68).
        const bool bedient = vorher.werte[iBelegt].b != nachher.werte[iBelegt].b
                          || vorher.werte[iAn].b     != nachher.werte[iAn].b
                          || std::memcmp (&vorher.werte[iFreq].zahl, &nachher.werte[iFreq].zahl, sizeof (double)) != 0;
        if (! bedient) continue;

        // JE ZONE einzeln (B-03): ein unter Zone A verletzt liegendes Band tritt
        // nach Zone B NEU ein, obwohl "liegt in irgendeiner" wahr bleibt.
        for (const auto& z : nachher.zonen)
        {
            if (! liegtInZone (nachher, slot, z, samplerate)) continue;
            const auto* alt = zoneMitId (vorher, z.id);
            if (alt == nullptr || ! liegtInZone (vorher, slot, *alt, samplerate))
                return slot;
        }
    }
    return -1;
}

std::array<bool, (size_t) param::kSlots> verletzteBaender (const param::DspSatz& s, double samplerate) noexcept
{
    std::array<bool, (size_t) param::kSlots> v {};
    for (int slot = 0; slot < param::kSlots; ++slot)
        for (const auto& z : s.zonen)
            if (liegtInZone (s, slot, z, samplerate)) { v[(size_t) slot] = true; break; }
    return v;
}

void setzeFreieSlotsNeutral (param::Satz& werte) noexcept
{
    for (int slot = 0; slot < param::kSlots; ++slot)
    {
        if (werte[(size_t) param::indexOccupied (slot)].b) continue;
        for (int feld = 0; feld < param::kJeSlot; ++feld)
        {
            const int i = param::indexBandV1 (slot, feld);
            werte[(size_t) i] = standardZelle (i);
        }
    }
}

//==============================================================================
Transaktionskern::Transaktionskern (Ausfuehrung& ausfuehrung)
    : aus (ausfuehrung)
{
    // S8 schiebt Eintraege in den Ring und darf dabei nicht allozieren: die
    // volle Tiefe steht ab hier bereit.
    ring.reserve ((size_t) param::kUndoTiefe);

    juce::String hex, grund;
    const bool ok = param::stateHash (committed, hex, grund) && ausHex (hex, committedHash);
    jassert (ok);   // der neutrale Zustand ist gueltig - B2 misst das
    juce::ignoreUnused (ok);
}

bool Transaktionskern::ladestart (const param::DspSatz& satz, std::uint64_t revision,
                                  const std::vector<state::UndoEintrag>& undoRing, int undoCursor,
                                  juce::String& grund)
{
    juce::String wo, hex;
    if (! param::validiere (satz, grund, wo)) return false;

    HashText h {};
    if (! param::stateHash (satz, hex, grund) || ! ausHex (hex, h))
    {
        if (grund.isEmpty()) grund = "state_hash";
        return false;
    }
    if (revision > kHoechsteRevision)                     { grund = "state_revision"; return false; }
    if ((int) undoRing.size() > param::kUndoTiefe
        || undoCursor < 0 || undoCursor > (int) undoRing.size()) { grund = "undo_ring"; return false; }
    for (const auto& e : undoRing)
        if (! param::validiere (e.zustand, grund, wo)) return false;

    committed     = satz;
    committedHash = h;
    r      = revision;
    r0     = revision;   // §5.11.4 Teil 1: der Ladestart setzt r0 = r, keine Revision entsteht
    ring   = undoRing;
    ring.reserve ((size_t) param::kUndoTiefe);
    cursor = undoCursor;

    // I5: das Register ist transient und beginnt leer. Eine Wiederholung aus
    // der vorigen Sitzung ist ein neuer Versuch (T17).
    reg.fill (RegisterEintrag {});

    // Beide Overlays sind fluechtig (§44.4) und ueberleben keinen Ladestart.
    automationOverlay.gesetzt.fill (false);
    if (automationOverlay.laeuft) { automationOverlay.laeuft = false; ++automationOverlay.epoche; }
    beendePreview();
    return true;
}

void Transaktionskern::schreibeIn (state::Zustand& z) const
{
    z.parameters    = committed.werte;
    z.schutzZonen   = committed.zonen;
    z.stateRevision = (juce::int64) r;
    z.undoRing      = ring;
    z.undoCursor    = cursor;
}

int Transaktionskern::registerBelegung() const noexcept
{
    int n = 0;
    for (const auto& x : reg)
        if (x.belegt) ++n;
    return n;
}

bool Transaktionskern::registerPlatz (int platz, Tid& tid, std::uint64_t& e, HashText& h) const noexcept
{
    if (platz < 0 || platz >= kRegisterPlaetze || ! reg[(size_t) platz].belegt) return false;
    const auto& x = reg[(size_t) platz];
    tid = x.tid;
    e   = x.e;
    h   = x.hash;
    return true;
}

bool Transaktionskern::setzePreview (const param::DspSatz& satz, juce::String& grund)
{
    juce::String wo;
    if (! param::validiere (satz, grund, wo)) return false;
    if (! aus.publizierePreview (satz)) { grund = "busy_retry"; return false; }
    vorschau.aktiv = true;
    vorschau.satz  = satz;
    return true;
}

void Transaktionskern::beendePreview() noexcept
{
    if (! vorschau.aktiv) return;
    vorschau.aktiv = false;
    aus.beendePreview();
}

void Transaktionskern::automationSchreiben (int index, const param::Zelle& wert) noexcept
{
    if (index < 0 || index >= param::kHostParameter) return;
    auto& o = automationOverlay;
    if (! o.laeuft) { o.laeuft = true; ++o.epoche; }   // Beginn einer Hostgeste
    const bool gleich = zelleGleich (index, wert, committed.werte[(size_t) index]);
    o.gesetzt[(size_t) index] = ! gleich;
    o.werte[(size_t) index]   = wert;
}

void Transaktionskern::automationRuht() noexcept
{
    auto& o = automationOverlay;
    if (! o.laeuft) return;
    o.laeuft = false;
    ++o.epoche;   // Ende der Hostgeste
}

void Transaktionskern::automationLoesen (int index) noexcept
{
    if (index < 0 || index >= param::kHostParameter) return;
    automationOverlay.gesetzt[(size_t) index] = false;
}

param::DspSatz Transaktionskern::wirksam() const
{
    auto s = committed;
    const auto& t = param::tabelle();
    for (int i = 0; i < param::kHostParameter; ++i)
        if (automationOverlay.gesetzt[(size_t) i] && (samplegenau || ! t[(size_t) i].topologisch))
            s.werte[(size_t) i] = automationOverlay.werte[(size_t) i];
    return s;
}

void Transaktionskern::setzeSamplerate (double samplerate) noexcept
{
    fs = (std::isfinite (samplerate) && samplerate > 0.0) ? samplerate : 0.0;
}

Ergebnis Transaktionskern::ohneCommit (Ausgang a, Stufe s) const noexcept
{
    // Kein Ausgang ohne Commit hinterlaesst etwas (I4): `r`, der Hash, der
    // Ring und das Register bleiben, wie sie sind.
    Ergebnis e;
    e.ausgang    = a;
    e.stufe      = s;
    e.revision   = r;
    e.hash       = committedHash;
    e.memoisiert = false;
    return e;
}

int Transaktionskern::ersterFreierSlot() const noexcept
{
    for (int slot = 0; slot < param::kSlots; ++slot)
        if (! committed.werte[(size_t) param::indexOccupied (slot)].b) return slot;
    return -1;
}

const param::DspSatz* Transaktionskern::revertZiel (std::uint64_t ziel) const noexcept
{
    if (ziel == r) return &committed;
    for (auto it = ring.rbegin(); it != ring.rend(); ++it)
        if (it->revision >= 0 && (std::uint64_t) it->revision == ziel) return &it->zustand;
    return nullptr;
}

const param::DspSatz* Transaktionskern::zonenziel (const Auftrag& a) const noexcept
{
    // Der Zustand, den eine Transaktion herstellen will - fuer den Zonenriegel.
    // Remove und Neutralisieren geben nur Slots frei; `bandBelegen` prueft S3
    // selbst am neuen Band.
    switch (a.art)
    {
        case Art::apply:
        case Art::gestus:         return &a.satz;
        case Art::revert:         return revertZiel (a.zielRevision);
        case Art::undo:           return cursor < (int) ring.size() ? &ring[ring.size() - 1 - (size_t) cursor].zustand : nullptr;
        case Art::redo:           return cursor > 0 ? &ring[ring.size() - (size_t) cursor].zustand : nullptr;
        case Art::presetLaden:    return &kandidat;   // in S2 gelesen
        case Art::remove:
        case Art::neutralisieren:
        case Art::bandBelegen:    return nullptr;
    }
    return nullptr;
}

//==============================================================================
Ergebnis Transaktionskern::fuehreAus (const Auftrag& a)
{
    // ── S0: Nachschlag. Liest nur das vorallokierte Register, alloziert nicht
    //    und kann nicht scheitern (M-75). Ein Treffer ist T1.
    melde (Stufe::s0);
    for (const auto& x : reg)
    {
        if (! x.belegt || x.tid != a.tid) continue;
        // I2 als Wache: ein Eintrag ausserhalb des Fensters waere ein Bruch -
        // er zaehlt als Fehltreffer, nie als falsches Ergebnis.
        if (! (x.e > r0 && x.e <= r && r - x.e <= (std::uint64_t) (kRegisterPlaetze - 1))) continue;
        Ergebnis e;
        e.ausgang    = Ausgang::gespeichertesErgebnis;
        e.stufe      = Stufe::s0;
        e.revision   = x.e;
        e.hash       = x.hash;
        e.memoisiert = true;
        return e;
    }

    // ── S1: base_revision gegen r (T2, T8, T10) - VOR der Validierung: eine
    //    veraltete Nutzlast wurde gegen einen Zustand gebildet, den es nicht
    //    mehr gibt.
    melde (Stufe::s1);
    if (einspritzung == Stufe::s1 || a.baseRevision != r)
        return ohneCommit (Ausgang::konflikt, Stufe::s1);

    // ── S2: die Nutzlast validieren (T3), Reihenfolge des DTO-Vertrags.
    melde (Stufe::s2);
    {
        juce::String grund, wo;
        bool ok = true;
        switch (a.art)
        {
            case Art::apply:
            case Art::gestus:
                ok = param::validiere (a.satz, grund, wo);
                break;
            case Art::bandBelegen:
            {
                // Die 13 Werte gegen die Vorlage des Slots 0 - jeder Slot traegt
                // dieselben Grenzen (nakama-parameter-v2.json).
                param::Satz probe = param::standardSatz();
                for (int f = 0; f < param::kJeSlot; ++f)
                    probe[(size_t) param::indexBandV1 (0, f)] = a.band[(size_t) f];
                ok = param::validiere (probe, grund, wo);
                break;
            }
            case Art::remove:
                if (a.slot < 0 || a.slot >= param::kSlots) { ok = false; grund = "bereich"; wo = "slot"; }
                break;
            case Art::revert:
                if (a.zielRevision > r) { ok = false; grund = "bereich"; wo = "ziel_revision"; }
                break;
            case Art::presetLaden:
                // R12: das Preset laesst `eq_enabled` unberuehrt - gelesen wird in
                // eine Kopie des bestaetigten Zustands.
                kandidat = committed;
                ok = preset::lies (a.preset.getData(), a.preset.getSize(), kandidat, grund, wo);
                break;
            case Art::neutralisieren:
            case Art::undo:
            case Art::redo:
                break;   // keine Nutzlast
        }
        if (einspritzung == Stufe::s2) { ok = false; grund = "einspritzung"; wo.clear(); }
        if (! ok)
        {
            auto e = ohneCommit (Ausgang::fehler, Stufe::s2);
            e.grund = wo.isNotEmpty() ? grund + " @ " + wo : grund;
            return e;
        }
    }

    // ── S3: Zonenriegel, NUR fuer Bedienaenderungen (T4). Undo, Redo und
    //    Preset-Laden sind Ganzzustands-Wiederherstellungen und laufen nicht
    //    hindurch (§5.6.3 Feinheit 3, T16). Remove und Neutralisieren geben
    //    Slots frei - sie koennen kein Band in eine Zone fuehren.
    melde (Stufe::s3);
    {
        bool sperre = false;
        // DIE Ausnahme aus §5.6.3 Feinheit 3 (T16) steht in genau dieser Zeile:
        // eine Ganzzustands-Wiederherstellung ist keine Bedienaenderung.
        if (! istGanzzustand (a.art))
        {
            sperre = einspritzung == Stufe::s3;
            if (a.art == Art::bandBelegen)
            {
                // `occupied` kippt auf true: P kippt fuer jede eingeschaltete
                // Zone, in der das NEUE, eingeschaltete Band liegt.
                if (ersterFreierSlot() >= 0 && a.band[(size_t) param::kEnabled].b)
                    for (const auto& z : committed.zonen)
                        if (z.enabled && frequenzInZone (a.band[(size_t) param::kFreqHz].zahl, z, fs)) { sperre = true; break; }
            }
            else if (const auto* ziel = zonenziel (a))
            {
                sperre = sperre || zoneneintritt (committed, *ziel, fs) >= 0;
            }
        }
        if (sperre) return ohneCommit (Ausgang::userSchutz, Stufe::s3);
    }

    // ── S4: das Kandidaten-DTO vollstaendig bauen (T13).
    melde (Stufe::s4);
    {
        const char* grund = nullptr;
        belegterSlot = -1;
        if (r >= kHoechsteRevision)
        {
            grund = "revision_erschoepft";   // hinter dem Commit-Punkt darf r + 1 nicht ueberlaufen
        }
        else
        {
            switch (a.art)
            {
                case Art::apply:
                case Art::gestus:
                    kandidat = a.satz;
                    break;
                case Art::presetLaden:
                    break;   // in S2 gelesen
                case Art::bandBelegen:
                {
                    const int slot = ersterFreierSlot();
                    if (slot < 0) { grund = "alle_slots_belegt"; break; }   // M-62: kein Opfer
                    kandidat = committed;
                    for (int f = 0; f < param::kJeSlot; ++f)
                        kandidat.werte[(size_t) param::indexBandV1 (slot, f)] = a.band[(size_t) f];
                    kandidat.werte[(size_t) param::indexOccupied (slot)].b = true;
                    belegterSlot = slot;
                    break;
                }
                case Art::remove:
                {
                    if (! committed.werte[(size_t) param::indexOccupied (a.slot)].b) { grund = "slot_frei"; break; }
                    // R5: occupied = false UND alle dreizehn v1-Werte auf Default;
                    // kein anderer Slot wird beruehrt oder umnummeriert.
                    kandidat = committed;
                    const int iBelegt = param::indexOccupied (a.slot);
                    kandidat.werte[(size_t) iBelegt] = standardZelle (iBelegt);
                    for (int f = 0; f < param::kJeSlot; ++f)
                    {
                        const int i = param::indexBandV1 (a.slot, f);
                        kandidat.werte[(size_t) i] = standardZelle (i);
                    }
                    belegterSlot = a.slot;
                    break;
                }
                case Art::neutralisieren:
                {
                    // Alle Werte auf den Vertragsdefault. Der Rollenschalter und die
                    // Schutz-Zonen bleiben (Manifest §10.2).
                    const bool eqAn = committed.werte[(size_t) param::kIndexEqEnabled].b;
                    kandidat.werte = param::standardSatz();
                    kandidat.werte[(size_t) param::kIndexEqEnabled].b = eqAn;
                    kandidat.zonen = committed.zonen;
                    break;
                }
                case Art::revert:
                    if (const auto* ziel = revertZiel (a.zielRevision)) kandidat = *ziel;
                    else grund = "revision_nicht_erreichbar";
                    break;
                case Art::undo:
                    if (cursor < (int) ring.size()) kandidat = ring[ring.size() - 1 - (size_t) cursor].zustand;
                    else grund = "undo_leer";
                    break;
                case Art::redo:
                    if (cursor > 0) kandidat = ring[ring.size() - (size_t) cursor].zustand;
                    else grund = "redo_leer";
                    break;
            }
        }

        juce::String vGrund, wo;
        if (grund == nullptr && ! param::validiere (kandidat, vGrund, wo)) grund = "kandidat_ungueltig";
        if (grund == nullptr && einspritzung == Stufe::s4)                grund = "einspritzung";
        if (grund != nullptr)
        {
            auto e = ohneCommit (Ausgang::fehler, Stufe::s4);
            e.grund = vGrund.isNotEmpty() ? juce::String (grund) + " " + vGrund + " @ " + wo : juce::String (grund);
            e.slot  = belegterSlot;
            return e;
        }
    }

    // ── S5: Programm bauen, OHNE eine Bank zu belegen (T5). Nur ein
    //    bankpflichtiger Kandidat kann hier scheitern.
    melde (Stufe::s5);
    if (einspritzung == Stufe::s5 || ! aus.baueVor (kandidat))
        return ohneCommit (Ausgang::busyRetry, Stufe::s5);

    // ── S6: state_hash ueber das Kandidaten-DTO (T14) - fehlbar und
    //    allozierend, deshalb VOR dem Commit-Punkt (B-01).
    melde (Stufe::s6);
    {
        juce::String hex, grund;
        const bool ok = einspritzung != Stufe::s6
                     && param::stateHash (kandidat, hex, grund)
                     && ausHex (hex, kandidatHash);
        if (! ok)
        {
            auto e = ohneCommit (Ausgang::fehler, Stufe::s6);
            e.grund = einspritzung == Stufe::s6 ? juce::String ("einspritzung") : "state_hash " + grund;
            return e;
        }
    }

    // ── S7: Undo-Eintrag vorbereiten - der Schnappschuss des NOCH bestaetigten
    //    Zustands (T15). Undo und Redo legen keinen Eintrag an; fuer sie ist es
    //    der Zustand, der an die Stelle unter dem Cursor wandert.
    melde (Stufe::s7);
    if (einspritzung == Stufe::s7)
    {
        auto e = ohneCommit (Ausgang::fehler, Stufe::s7);
        e.grund = "einspritzung";
        return e;
    }
    if (ring.capacity() < (size_t) param::kUndoTiefe)
        ring.reserve ((size_t) param::kUndoTiefe);
    eintrag.zustand  = committed;
    eintrag.revision = (juce::int64) r;
    eintrag.slot     = belegterSlot;
    switch (a.art)
    {
        case Art::apply:
        case Art::bandBelegen:    eintrag.art = state::UndoArt::apply; break;
        case Art::revert:         eintrag.art = state::UndoArt::revert; break;
        case Art::neutralisieren: eintrag.art = state::UndoArt::neutralisieren; break;
        case Art::remove:         eintrag.art = state::UndoArt::remove; break;
        case Art::presetLaden:    eintrag.art = state::UndoArt::presetLaden; break;
        case Art::gestus:         eintrag.art = state::UndoArt::gestus; break;
        case Art::undo:
        case Art::redo:           break;   // Art und Slot des bewegten Eintrags bleiben
    }

    // ═════════════════════════════ COMMIT-PUNKT ══════════════════════════════
    // Ab hier scheitert nichts und nichts alloziert (§5.11.4 Teil 2).

    // ── S8
    melde (Stufe::s8);
    const std::uint64_t alt = r;
    const std::uint64_t e   = r + 1;   // S4 hat r < kHoechsteRevision gesichert

    std::swap (committed, kandidat);   // Vektoren tauschen Puffer; das Wertefeld wird kopiert
    committedHash = kandidatHash;

    switch (a.art)
    {
        case Art::undo:
        {
            auto& x = ring[ring.size() - 1 - (size_t) cursor];
            x.zustand  = std::move (eintrag.zustand);
            x.revision = (juce::int64) alt;
            ++cursor;
            break;
        }
        case Art::redo:
        {
            auto& x = ring[ring.size() - (size_t) cursor];
            x.zustand  = std::move (eintrag.zustand);
            x.revision = (juce::int64) alt;
            --cursor;
            break;
        }
        default:
            // M-79: eine neue Transaktion schneidet den Redo-Zweig ab.
            if (cursor > 0)
            {
                ring.erase (ring.end() - cursor, ring.end());
                cursor = 0;
            }
            // M-78: beim 33. Eintrag faellt der aelteste heraus.
            if ((int) ring.size() >= param::kUndoTiefe)
                ring.erase (ring.begin());
            ring.push_back (std::move (eintrag));   // Kapazitaet steht seit S7
            break;
    }

    r = e;

    // Der Automationsueberzug gehoert dem ALTEN bestaetigten Zustand; der
    // Besitzer synchronisiert die Hostparameter auf den neuen (§44.3).
    automationOverlay.gesetzt.fill (false);

    // Registereintrag - ausschliesslich hier (I4). Platz (e - r0 - 1) mod 32:
    // ab dem 33. Commit der Sitzung ueberschreibt er genau den Eintrag mit
    // e - 32, der das Fenster in diesem Moment verlaesst (§5.11.4 Teil 1).
    auto& platz = reg[(size_t) ((e - r0 - 1) % (std::uint64_t) kRegisterPlaetze)];
    platz.belegt = true;
    platz.tid    = a.tid;
    platz.e      = e;
    platz.hash   = committedHash;

    aus.publiziereVorbau();
    aus.commitGemeldet (e);

    Ergebnis ergebnis;
    ergebnis.ausgang    = Ausgang::commit;
    ergebnis.stufe      = Stufe::s8;
    ergebnis.revision   = e;
    ergebnis.hash       = committedHash;
    ergebnis.memoisiert = true;
    ergebnis.slot       = belegterSlot;
    return ergebnis;
}

//==============================================================================
bool DspKernAusfuehrung::baueVor (const param::DspSatz& kandidat)
{
    // Vor der ersten Vorbereitung gibt es keine Samplerate und kein Programm:
    // der bestaetigte Zustand wartet, und die Vorbereitung publiziert ihn
    // (M-09). Ein unvorbereiteter Kern hat jede Bank frei - auch ein
    // bankpflichtiger Kandidat scheitert hier deshalb nicht.
    vorbauGebaut = kern.samplerate() > 0.0;
    if (! vorbauGebaut) return true;

    sichtCommit.werte = kandidat.werte;   // Wertefeld ohne Heap; die Zonen bleiben leer
    setzeFreieSlotsNeutral (sichtCommit.werte);
    return kern.baueVor (sichtCommit, dsp::Pfad::committed);
}

void DspKernAusfuehrung::publiziereVorbau() noexcept
{
    if (! vorbauGebaut) { zuletztGueltig = false; return; }
    kern.publiziereVorbau (dsp::Pfad::committed);
    zuletzt        = sichtCommit.werte;
    zuletztGueltig = true;
}

void DspKernAusfuehrung::commitGemeldet (std::uint64_t) noexcept
{
    commits.fetch_add (1);
    dirty.store (true);
    bericht.store (true);
}

bool DspKernAusfuehrung::publizierePreview (const param::DspSatz& satz)
{
    if (kern.samplerate() <= 0.0) return true;   // die Vorbereitung publiziert sie
    sichtWeg.werte = satz.werte;
    setzeFreieSlotsNeutral (sichtWeg.werte);
    return kern.uebernehmeZustand (sichtWeg, dsp::Pfad::candidate);
}

void DspKernAusfuehrung::beendePreview() noexcept
{
    kern.beendeCandidate();
}

bool DspKernAusfuehrung::publiziereWirksam (const param::DspSatz& wirksam, bool erzwingen)
{
    if (kern.samplerate() <= 0.0) return true;
    sichtWeg.werte = wirksam.werte;
    setzeFreieSlotsNeutral (sichtWeg.werte);
    if (! erzwingen && zuletztGueltig && werteGleich (sichtWeg.werte, zuletzt)) return true;
    if (! kern.uebernehmeZustand (sichtWeg, dsp::Pfad::committed)) return false;
    zuletzt        = sichtWeg.werte;
    zuletztGueltig = true;
    return true;
}

bool DspKernAusfuehrung::zuletztPubliziert (param::Satz& aus) const noexcept
{
    if (! zuletztGueltig) return false;
    aus = zuletzt;
    return true;
}

//==============================================================================
bool baueBericht (const Transaktionskern& tk, DspBericht& aus, juce::String& grund)
{
    const auto& c = tk.bestaetigt();

    // M-103: die Nutzlast ist GENAU der Text, ueber den der Hash gebildet
    // wurde - derselbe Kanonisierer, keine zweite Form.
    juce::MemoryBlock text;
    if (! param::dtoKanon (c, text, grund)) return false;
    aus.revision = tk.revision();
    aus.hash     = tk.hash();
    aus.jcs      = juce::String::fromUTF8 ((const char*) text.getData(), (int) text.getSize());

    aus.autoGainDb = 0.0;
    aus.klemmungen.clear();
    const double fs = tk.samplerate();
    if (fs > 0.0)
    {
        // Das Programm des BESTAETIGTEN Zustands in seiner Kernsicht - dieselbe
        // Ableitung, die der Audiothread bekommt (R4, R7).
        auto sicht = c;
        setzeFreieSlotsNeutral (sicht.werte);
        auto prog = std::make_unique<dsp::DspProgramm>();
        dsp::baueProgramm (sicht, fs, 0, *prog);
        aus.autoGainDb = prog->autoGainDb;
        for (int slot = 0; slot < param::kSlots; ++slot)
        {
            const auto& b = prog->baender[(size_t) slot];
            const juce::String praefix = "v1.band." + juce::String (slot) + ".";
            // Nur die R7-Klemmung: v3 `dsp_klemmung` beschreibt genau sie, und
            // E2-7 begrenzt die Liste auf hoechstens EINE je Slot. Die
            // Nyquist-Kappung (M-12) traegt das Programm als `gekapptFreq`.
            if (b.geklemmtSidechain)
                aus.klemmungen.push_back ({ praefix + "sidechain_source", "priority_sidechain", "internal" });
        }
    }

    aus.verletzteBaender.clear();
    const auto verletzt = verletzteBaender (tk.wirksam(), fs);
    for (int slot = 0; slot < param::kSlots; ++slot)
        if (verletzt[(size_t) slot]) aus.verletzteBaender.push_back (slot);

    aus.undoTiefe = (int) tk.undoRing().size() - tk.undoCursor();
    return true;
}

} // namespace nakama::transaktion

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
