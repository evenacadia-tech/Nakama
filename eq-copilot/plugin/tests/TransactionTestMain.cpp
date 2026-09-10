/*  Kanon B7 - EqCopTransactionTest (SONDE-015 Etappe 4a, Bauplan §4.4).

    Misst den lokalen Transaktionskern `eq-copilot/plugin/state/NakamaTransaktion.*`
    und seine Anbindung im Prozessor `eq-copilot/plugin/sonde/SondeProcessor.*`
    gegen die Verhaltensmatrix §3 von `docs/beweise/SONDE-015.md`. Die Referenz
    der Transaktion ist die Protokollspezifikation §5.11.4: Stufen S0 bis S8,
    Invarianten I1 bis I6, Falltabelle T1 bis T17.

    ZWEI EBENEN IN EINEM ZIEL, wie bei B4, B5 und B6:
      1. Der Transaktionskern mit dem ECHTEN DSP-Kern der Etappe 3 dahinter -
         die Bankpflicht aus S5 ist damit die des gebauten Pools, keine
         Attrappe. Hier laufen Falltabelle, Einspritzpunkte, der
         10.000er-Lauf, Belegung, Zonen, Undo-Ring und Preset.
      2. DERSELBE Kern im echten `SondeProcessor`: Host-Parameter, Automation,
         Gestus, Save/Load, Host-Dirty und der erste Audioblock.

    LANDMINE NAK-175: Prozessor, DSP-Kern und Transaktionskern liegen in jeder
    Testfunktion auf dem HEAP (`std::unique_ptr`), nie im Rahmen.

    ALLOKATIONSZAEHLER: thread_local und nur um die gemessene Stelle
    eingeschaltet - den Nachschlag S0 und alles hinter dem Commit-Punkt.

    Exit 0 nur bei "TRANSAKTION OK".
*/

#include "NakamaTransaktion.h"
#include "NakamaPreset.h"
#include "NakamaKanon.h"
#include "DspKern.h"
#include "SondeProcessor.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace tx    = nakama::transaktion;
namespace param = nakama::parameter;
namespace dsp   = nakama::dsp;
namespace state = nakama::state;

//==============================================================================
namespace
{
    thread_local bool          zaehleAllokationen = false;
    thread_local std::uint64_t allokationen       = 0;
}

void* operator new (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

//==============================================================================
namespace
{

int fehler = 0, geprueft = 0;

void pruefe (bool ok, const std::string& was, const std::string& detail = {})
{
    ++geprueft;
    if (! ok) ++fehler;
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << was;
    if (! detail.empty()) std::cout << "  [" << detail << "]";
    std::cout << std::endl;
}

void abschnitt (const std::string& titel)
{
    std::cout << std::endl << "== " << titel << " ==" << std::endl;
}

std::string str (const juce::String& s) { return s.toStdString(); }
std::string zahl (std::uint64_t n)      { return std::to_string (n); }

std::string beschreibe (const tx::Ergebnis& e)
{
    std::string t = std::string (tx::wort (e.ausgang)) + " @" + tx::wort (e.stufe)
                  + " rev=" + zahl (e.revision) + (e.memoisiert ? " memo" : " nicht-memo");
    if (e.grund.isNotEmpty()) t += " grund=" + str (e.grund);
    return t;
}

//==============================================================================
// Der Pruefstand: ein echter DSP-Kern, seine Ausfuehrung und der
// Transaktionskern - alle drei auf dem Heap (NAK-175).

struct Stand
{
    std::unique_ptr<dsp::DspKern>            kern;
    std::unique_ptr<tx::DspKernAusfuehrung>  aus;
    std::unique_ptr<tx::Transaktionskern>    tk;

    explicit Stand (double fs = 48000.0)
    {
        kern = std::make_unique<dsp::DspKern>();
        if (fs > 0.0) kern->bereiteVor (fs, 512);
        aus = std::make_unique<tx::DspKernAusfuehrung> (*kern);
        tk  = std::make_unique<tx::Transaktionskern> (*aus);
        tk->setzeSamplerate (fs);
    }
};

/** Was der Test ueber die committeten Transaktionen der laufenden Sitzung
    weiss - die Gegenseite der Wachen I2 und I4. */
struct Sitzung
{
    std::vector<std::pair<tx::Tid, std::uint64_t>> commits;
};

tx::Tid tid (std::uint64_t n) { return { 0xB7u, n }; }

/** Faehrt eine Eingabe und fuehrt die Sitzung nach. */
tx::Ergebnis fahre (Stand& st, Sitzung& s, const tx::Auftrag& a)
{
    auto e = st.tk->fuehreAus (a);
    if (e.ausgang == tx::Ausgang::commit) s.commits.push_back ({ a.tid, e.revision });
    return e;
}

/** I1, I2 (beide Richtungen) und I4 in ihrer Fassung mit r0 (§5.11.4 Teil 3). */
bool wachenHalten (const tx::Transaktionskern& tk, const Sitzung& s, std::string& detail)
{
    const auto r = tk.revision(), r0 = tk.ladeRevision();
    const int soll = (int) std::min<std::uint64_t> (r - r0, (std::uint64_t) tx::kRegisterPlaetze);
    bool i1 = tk.registerBelegung() == soll, i2 = true, i4 = true;

    for (int p = 0; p < tx::kRegisterPlaetze; ++p)
    {
        tx::Tid t; std::uint64_t e = 0; tx::HashText h {};
        if (! tk.registerPlatz (p, t, e, h)) continue;
        if (! (e > r0 && e <= r && r - e <= 31)) i2 = false;
        const bool ausCommit = std::any_of (s.commits.begin(), s.commits.end(),
                                            [&] (const auto& c) { return c.first == t && c.second == e; });
        if (! ausCommit) i4 = false;
    }
    for (const auto& c : s.commits)
    {
        if (! (c.second > r0 && c.second <= r && r - c.second <= 31)) continue;
        bool da = false;
        for (int p = 0; p < tx::kRegisterPlaetze && ! da; ++p)
        {
            tx::Tid t; std::uint64_t e = 0; tx::HashText h {};
            da = tk.registerPlatz (p, t, e, h) && t == c.first && e == c.second;
        }
        if (! da) i2 = false;
    }
    detail = "belegt " + std::to_string (tk.registerBelegung()) + " soll " + std::to_string (soll)
           + ", r=" + zahl (r) + " r0=" + zahl (r0)
           + (i1 ? "" : " I1-BRUCH") + (i2 ? "" : " I2-BRUCH") + (i4 ? "" : " I4-BRUCH");
    return i1 && i2 && i4;
}

void wachen (const tx::Transaktionskern& tk, const Sitzung& s, const std::string& wo)
{
    std::string d;
    const bool ok = wachenHalten (tk, s, d);
    pruefe (ok, "  Wachen I1/I2/I4 nach " + wo, d);
}

//==============================================================================
// Auftraege und Zustaende - alles ueber `nakama::parameter`, keine zweite
// Wahrheit fuer Grenzen, Defaults oder Reihenfolge.

int iBand (int slot, int feld) { return param::indexBandV1 (slot, feld); }

tx::Auftrag auftrag (const tx::Transaktionskern& tk, tx::Art art, std::uint64_t n)
{
    tx::Auftrag a;
    a.tid          = tid (n);
    a.baseRevision = tk.revision();
    a.art          = art;
    return a;
}

tx::Auftrag apply (const tx::Transaktionskern& tk, const param::DspSatz& ziel, std::uint64_t n)
{
    auto a = auftrag (tk, tx::Art::apply, n);
    a.satz = ziel;
    return a;
}

tx::Auftrag remove (const tx::Transaktionskern& tk, int slot, std::uint64_t n)
{
    auto a = auftrag (tk, tx::Art::remove, n);
    a.slot = slot;
    return a;
}

tx::Auftrag revert (const tx::Transaktionskern& tk, std::uint64_t ziel, std::uint64_t n)
{
    auto a = auftrag (tk, tx::Art::revert, n);
    a.zielRevision = ziel;
    return a;
}

tx::Auftrag presetLaden (const tx::Transaktionskern& tk, const juce::MemoryBlock& bytes, std::uint64_t n)
{
    auto a = auftrag (tk, tx::Art::presetLaden, n);
    a.preset = bytes;
    return a;
}

param::DspSatz mitEq (bool an = true)
{
    param::DspSatz s;
    s.werte[(size_t) param::kIndexEqEnabled].b = an;
    return s;
}

void setzeBand (param::DspSatz& s, int slot, double freqHz, double gainDb, bool an = true)
{
    s.werte[(size_t) param::indexOccupied (slot)].b      = true;
    s.werte[(size_t) iBand (slot, param::kEnabled)].b    = an;
    s.werte[(size_t) iBand (slot, param::kFreqHz)].zahl  = freqHz;
    s.werte[(size_t) iBand (slot, param::kGainDb)].zahl  = gainDb;
}

double freq (const param::DspSatz& s, int slot) { return s.werte[(size_t) iBand (slot, param::kFreqHz)].zahl; }
double gain (const param::DspSatz& s, int slot) { return s.werte[(size_t) iBand (slot, param::kGainDb)].zahl; }
bool   belegt (const param::DspSatz& s, int slot) { return s.werte[(size_t) param::indexOccupied (slot)].b; }

/** Die 13 v1-Werte eines Bandes als Nutzlast fuer `bandBelegen`. */
std::array<param::Zelle, (size_t) param::kJeSlot> bandWerte (double freqHz, double gainDb, bool an = true)
{
    param::DspSatz s;
    setzeBand (s, 0, freqHz, gainDb, an);
    std::array<param::Zelle, (size_t) param::kJeSlot> b {};
    for (int f = 0; f < param::kJeSlot; ++f) b[(size_t) f] = s.werte[(size_t) iBand (0, f)];
    return b;
}

/** Grundzustand per Apply: EQ an oder aus, ein belegtes Band auf Slot 0. */
tx::Ergebnis grundzustand (Stand& st, Sitzung& s, std::uint64_t n, bool eqAn = true, double freqHz = 1000.0)
{
    auto z = mitEq (eqAn);
    setzeBand (z, 0, freqHz, 3.0);
    return fahre (st, s, apply (*st.tk, z, n));
}

/** Der bestaetigte Zustand mit geaendertem Gain auf Slot 0. */
param::DspSatz mitGain (const tx::Transaktionskern& tk, double g)
{
    auto z = tk.bestaetigt();
    z.werte[(size_t) iBand (0, param::kGainDb)].zahl = g;
    return z;
}

bool registerTraegt (const tx::Transaktionskern& tk, const tx::Tid& t,
                     std::uint64_t* e = nullptr, tx::HashText* h = nullptr)
{
    for (int p = 0; p < tx::kRegisterPlaetze; ++p)
    {
        tx::Tid x; std::uint64_t ex = 0; tx::HashText hx {};
        if (tk.registerPlatz (p, x, ex, hx) && x == t)
        {
            if (e != nullptr) *e = ex;
            if (h != nullptr) *h = hx;
            return true;
        }
    }
    return false;
}

//==============================================================================
// Der Pool

/** Reserviert jede freie Bank ueber den Worker-Weg (`frei -> vorbereitend`). */
int fuellePool (dsp::DspKern& k)
{
    int n = 0;
    while (k.pool().reserviere() >= 0) ++n;
    return n;
}

/** Gibt EINE reservierte Bank ueber die volle Ownership-Kette frei:
    Publikation, Uebernahme wie am Blockrand, Verblassen, Ausdienen mit ACK in
    den SPSC-Ring, Ernte. Das ist der Audio-ACK aus T7 - auf dem Testthread
    gefahren, damit die Reihenfolge deterministisch ist. */
bool gibEineBankUeberAckFrei (dsp::DspKern& k)
{
    auto& pool = k.pool();
    for (int slot = 0; slot < dsp::DspBankPool::kBaenke; ++slot)
    {
        if (pool.zustand (slot) != dsp::BankZustand::vorbereitend) continue;
        pool.bank (slot).programm.generation = pool.naechsteGeneration();
        pool.publiziere (dsp::Pfad::candidate, slot);
        if (pool.uebernehmeBereiten (dsp::Pfad::candidate) != slot) return false;
        if (! pool.beginneVerblassen (slot)) return false;
        pool.meldeAusgedient (slot);
        return pool.ernteAcks() == 1 && pool.zustand (slot) == dsp::BankZustand::frei;
    }
    return false;
}

//==============================================================================
// §5.11.4 Teil 4 - eine Pruefung je Tabellenzeile ueber ALLE vier Spalten
// (M-125): Ausgang (samt Stufe), `r` danach, memoisiert - am Ergebnis UND am
// Register gemessen - und bei einem Treffer Revision und Hash.

struct Soll
{
    tx::Ausgang   ausgang;
    tx::Stufe     stufe;
    std::uint64_t rDanach;
    bool          memoisiert;
    bool          trefferPruefen = false;
    std::uint64_t trefferRevision = 0;
    tx::HashText  trefferHash {};
};

void pruefeZeile (const char* zeile, const char* test, const tx::Auftrag& a, const tx::Ergebnis& e,
                  const tx::Transaktionskern& tk, const Soll& soll)
{
    const bool imRegister = registerTraegt (tk, a.tid);
    bool ok = e.ausgang == soll.ausgang && e.stufe == soll.stufe
           && tk.revision() == soll.rDanach
           && e.memoisiert == soll.memoisiert && imRegister == soll.memoisiert;
    if (soll.trefferPruefen)
        ok = ok && e.revision == soll.trefferRevision && e.hash == soll.trefferHash;
    pruefe (ok, std::string (zeile) + " " + test,
            beschreibe (e) + ", r danach " + zahl (tk.revision())
            + (imRegister ? ", im Register" : ", nicht im Register")
            + " | soll " + tx::wort (soll.ausgang) + " @" + tx::wort (soll.stufe)
            + " r=" + zahl (soll.rDanach) + (soll.memoisiert ? " memo" : " nicht-memo"));
}

/** Vorlauf fuer T4, T8 und T9: Band auf Slot 0 bei 500 Hz, Zone 0 (900 bis
    1100 Hz) eingeschaltet. Die Transaktion T zieht das Band auf 1000 Hz. */
tx::Auftrag zonenVorlauf (Stand& st, Sitzung& s, bool eqAn)
{
    auto z = mitEq (eqAn);
    setzeBand (z, 0, 500.0, 3.0);
    z.zonen = { { 0, 900.0, 1100.0, true } };
    fahre (st, s, apply (*st.tk, z, 100));                        // r = 1
    auto ziel = st.tk->bestaetigt();
    ziel.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 1000.0;
    return apply (*st.tk, ziel, 1);
}

/** Dieselbe logische Transaktion T gegen den AKTUELLEN Stand gebildet: die
    Aenderung (Band auf 1000 Hz) bleibt, `base_revision` und der uebrige
    Zustand sind neu - das ist die "aktualisierte base_revision" aus T9. */
tx::Auftrag rebasiert (const tx::Transaktionskern& tk, tx::Auftrag t)
{
    auto ziel = tk.bestaetigt();
    ziel.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 1000.0;
    t.satz = ziel;
    t.baseRevision = tk.revision();
    return t;
}

void fallT1()
{
    Stand st; Sitzung s;
    grundzustand (st, s, 100);                                   // r = 1
    const auto t  = apply (*st.tk, mitGain (*st.tk, 6.0), 1);
    const auto eT = fahre (st, s, t);                            // T committet, r = 2
    const auto hashT = st.tk->hash();
    fahre (st, s, apply (*st.tk, mitGain (*st.tk, -6.0), 2));    // U, r = 3: der bestaetigte Hash ist jetzt ein anderer
    wachen (*st.tk, s, "T1 Vorlauf");

    auto wieder = t;                                             // dieselbe tid, beliebige base, beliebige Nutzlast
    wieder.baseRevision = 999;
    wieder.satz = mitEq (false);
    const auto e = fahre (st, s, wieder);
    pruefeZeile ("T1", "wiederholung_im_fenster_liefert_dasselbe_ergebnis", wieder, e, *st.tk,
                 { tx::Ausgang::gespeichertesErgebnis, tx::Stufe::s0, 3, true, true, eT.revision, hashT });
    wachen (*st.tk, s, "T1");
}

void fallT2()
{
    Stand st; Sitzung s;
    grundzustand (st, s, 100);                                   // r = 1
    auto t = apply (*st.tk, mitGain (*st.tk, 6.0), 1);
    t.baseRevision = 0;                                          // veraltet
    const auto vorher = st.tk->bestaetigt();
    const auto e = fahre (st, s, t);
    pruefeZeile ("T2", "veraltete_base_revision_ist_konflikt_ohne_wirkung", t, e, *st.tk,
                 { tx::Ausgang::konflikt, tx::Stufe::s1, 1, false });
    pruefe (st.tk->bestaetigt() == vorher, "T2 der bestaetigte Zustand ist unberuehrt");
    wachen (*st.tk, s, "T2");
    t.baseRevision = st.tk->revision();
    const auto e2 = fahre (st, s, t);
    pruefe (e2.ausgang == tx::Ausgang::commit, "T2 der Konflikt liegt nicht im Register: dieselbe tid mit aktueller base committet", beschreibe (e2));
    wachen (*st.tk, s, "T2 Wiederholung");
}

void fallT3()
{
    Stand st; Sitzung s;
    grundzustand (st, s, 100);                                   // r = 1
    auto t = apply (*st.tk, mitGain (*st.tk, 18.0), 1);          // ausserhalb von ±12 dB
    const auto e = fahre (st, s, t);
    pruefeZeile ("T3", "ungueltige_nutzlast_faellt_und_wird_nicht_gemerkt", t, e, *st.tk,
                 { tx::Ausgang::fehler, tx::Stufe::s2, 1, false });
    pruefe (e.grund.startsWith ("bereich"), "T3 der Grund kommt aus dem DTO-Vertrag", str (e.grund));
    wachen (*st.tk, s, "T3");
    t.satz.werte[(size_t) iBand (0, param::kGainDb)].zahl = 6.0;
    const auto e2 = fahre (st, s, t);
    pruefe (e2.ausgang == tx::Ausgang::commit, "T3 der Fehler ist nicht gemerkt: dieselbe tid mit gueltiger Nutzlast committet", beschreibe (e2));
    wachen (*st.tk, s, "T3 Wiederholung");
}

void fallT4()
{
    Stand st; Sitzung s;
    const auto t = zonenVorlauf (st, s, true);
    const auto e = fahre (st, s, t);
    pruefeZeile ("T4", "zonenriegel_weist_ab_und_merkt_nichts", t, e, *st.tk,
                 { tx::Ausgang::userSchutz, tx::Stufe::s3, 1, false });
    pruefe (freq (st.tk->bestaetigt(), 0) == 500.0, "T4 das Band bleibt ausserhalb der Zone");
    wachen (*st.tk, s, "T4");
}

void fallT5()
{
    Stand st; Sitzung s;
    grundzustand (st, s, 100);                                   // eq an: bankpflichtig
    fuellePool (*st.kern);
    const auto t = apply (*st.tk, mitGain (*st.tk, 6.0), 1);
    const auto e = fahre (st, s, t);
    pruefeZeile ("T5", "busy_retry_wird_nicht_gemerkt", t, e, *st.tk,
                 { tx::Ausgang::busyRetry, tx::Stufe::s5, 1, false });
    pruefe (st.kern->pool().freieSlots() == 0, "T5 der Pool war wirklich voll",
            "frei " + std::to_string (st.kern->pool().freieSlots()));
    wachen (*st.tk, s, "T5");
}

void fallT6()
{
    Stand st; Sitzung s;
    grundzustand (st, s, 100);                                   // r = 1
    const auto t = apply (*st.tk, mitGain (*st.tk, 6.0), 1);
    const auto e = fahre (st, s, t);
    std::uint64_t imRegister = 0; tx::HashText hashImRegister {};
    const bool eintrag = registerTraegt (*st.tk, t.tid, &imRegister, &hashImRegister);
    pruefeZeile ("T6", "commit_erzeugt_genau_eine_revision_und_einen_eintrag", t, e, *st.tk,
                 { tx::Ausgang::commit, tx::Stufe::s8, 2, true });
    pruefe (eintrag && imRegister == 2 && hashImRegister == st.tk->hash() && e.hash == st.tk->hash(),
            "T6 der Eintrag traegt (tid, e = r, Hash des neuen Zustands)", "e=" + zahl (imRegister));
    wachen (*st.tk, s, "T6");
}

void fallT7()
{
    Stand st; Sitzung s;
    grundzustand (st, s, 100);
    fuellePool (*st.kern);
    const auto t  = apply (*st.tk, mitGain (*st.tk, 6.0), 1);
    const auto e5 = fahre (st, s, t);
    pruefe (e5.ausgang == tx::Ausgang::busyRetry, "T7 Vorlauf: T endet in busy_retry (T5)", beschreibe (e5));
    const bool frei = gibEineBankUeberAckFrei (*st.kern);
    pruefe (frei && st.tk->revision() == 1, "T7 ein Audio-ACK gibt eine Bank frei - ohne Revision",
            "frei " + std::to_string (st.kern->pool().freieSlots()) + ", r=" + zahl (st.tk->revision()));
    const auto e = fahre (st, s, t);                             // dieselbe tid, dieselbe base_revision
    pruefeZeile ("T7", "wiederholung_nach_bankfreigabe_committet", t, e, *st.tk,
                 { tx::Ausgang::commit, tx::Stufe::s8, 2, true });
    wachen (*st.tk, s, "T7");
}

void fallT8()
{
    Stand st; Sitzung s;
    const auto t = zonenVorlauf (st, s, true);
    fahre (st, s, t);                                            // T4: User-Schutz
    auto loesen = st.tk->bestaetigt();
    for (auto& zone : loesen.zonen) zone.enabled = false;
    const auto el = fahre (st, s, apply (*st.tk, loesen, 2));    // r = 2
    pruefe (el.ausgang == tx::Ausgang::commit && st.tk->revision() == 2,
            "T8 Vorlauf: das Loesen der Zone ist selbst eine Transaktion (M-71)", beschreibe (el));
    const auto e = fahre (st, s, t);                             // ALTE base_revision = 1
    pruefeZeile ("T8", "retry_nach_zonenloesung_mit_alter_base_revision_ist_konflikt", t, e, *st.tk,
                 { tx::Ausgang::konflikt, tx::Stufe::s1, 2, false });
    wachen (*st.tk, s, "T8");
}

void fallT9()
{
    {
        Stand st; Sitzung s;
        const auto t = zonenVorlauf (st, s, true);
        fahre (st, s, t);
        auto loesen = st.tk->bestaetigt();
        for (auto& zone : loesen.zonen) zone.enabled = false;
        fahre (st, s, apply (*st.tk, loesen, 2));                 // r = 2
        const auto neu = rebasiert (*st.tk, t);
        const auto e = fahre (st, s, neu);
        pruefeZeile ("T9", "retry_nach_zonenloesung_mit_aktueller_base_revision_committet", neu, e, *st.tk,
                     { tx::Ausgang::commit, tx::Stufe::s8, 3, true });
        wachen (*st.tk, s, "T9");
    }
    {
        Stand st; Sitzung s;
        const auto t = zonenVorlauf (st, s, true);                 // bankpflichtig
        fahre (st, s, t);
        auto loesen = st.tk->bestaetigt();
        for (auto& zone : loesen.zonen) zone.enabled = false;
        fahre (st, s, apply (*st.tk, loesen, 2));
        fuellePool (*st.kern);
        const auto neu = rebasiert (*st.tk, t);
        const auto e = fahre (st, s, neu);
        pruefeZeile ("T9", "retry_nach_zonenloesung_bei_vollem_pool_ist_busy_retry (bankpflichtig, nach I6 Ausgang T5)",
                     neu, e, *st.tk, { tx::Ausgang::busyRetry, tx::Stufe::s5, 2, false });
        wachen (*st.tk, s, "T9 bankpflichtig bei vollem Pool");
    }
    {
        Stand st; Sitzung s;
        const auto t = zonenVorlauf (st, s, false);                // eq_enabled = false: bankfrei
        fahre (st, s, t);
        auto loesen = st.tk->bestaetigt();
        for (auto& zone : loesen.zonen) zone.enabled = false;
        fahre (st, s, apply (*st.tk, loesen, 2));
        const int reserviert = fuellePool (*st.kern);
        const auto neu = rebasiert (*st.tk, t);
        const auto e = fahre (st, s, neu);
        pruefeZeile ("T9", "retry_nach_zonenloesung_bei_vollem_pool_ohne_bankpflicht_committet", neu, e, *st.tk,
                     { tx::Ausgang::commit, tx::Stufe::s8, 3, true });
        pruefe (reserviert == dsp::DspBankPool::kBaenke && st.kern->pool().freieSlots() == 0,
                "T9 der Pool war im bankfreien Fall wirklich voll", "reserviert " + std::to_string (reserviert));
        wachen (*st.tk, s, "T9 bankfrei bei vollem Pool");
    }
}

/** Vorlauf fuer T10, T11 und T17: 33 Commits in dieser Sitzung. `hashes[i]`
    ist der Hash nach dem Commit von `auftraege[i]` (e = r0 + i + 1). */
void dreiunddreissigCommits (Stand& st, Sitzung& s, std::vector<tx::Auftrag>& auftraege,
                             std::vector<tx::HashText>& hashes)
{
    auto z = mitEq (true);
    setzeBand (z, 0, 1000.0, 0.0);
    for (int i = 1; i <= 33; ++i)
    {
        z.werte[(size_t) iBand (0, param::kGainDb)].zahl = (double) (i % 24) - 12.0 + 0.25;
        auto a = apply (*st.tk, z, (std::uint64_t) i);
        fahre (st, s, a);
        auftraege.push_back (a);
        hashes.push_back (st.tk->hash());
    }
}

void fallT10()
{
    Stand st; Sitzung s; std::vector<tx::Auftrag> a; std::vector<tx::HashText> h;
    dreiunddreissigCommits (st, s, a, h);
    wachen (*st.tk, s, "T10 Vorlauf (33 Commits)");
    const auto e = fahre (st, s, a[0]);                          // e = r0 + 1, r - e = 32, base = r0
    pruefeZeile ("T10", "fensterrand_erste_transaktion_ist_draussen", a[0], e, *st.tk,
                 { tx::Ausgang::konflikt, tx::Stufe::s1, 33, false });
    wachen (*st.tk, s, "T10");
}

void fallT11()
{
    Stand st; Sitzung s; std::vector<tx::Auftrag> a; std::vector<tx::HashText> h;
    dreiunddreissigCommits (st, s, a, h);
    const auto e = fahre (st, s, a[1]);                          // e = r0 + 2, r - e = 31
    pruefeZeile ("T11", "fensterrand_zweite_transaktion_ist_drinnen", a[1], e, *st.tk,
                 { tx::Ausgang::gespeichertesErgebnis, tx::Stufe::s0, 33, true, true, 2, h[1] });
    wachen (*st.tk, s, "T11");
}

void fallT12()
{
    Stand st; Sitzung s;
    grundzustand (st, s, 100);                                   // r = 1
    const auto t  = apply (*st.tk, mitGain (*st.tk, 6.0), 1);
    const auto eT = fahre (st, s, t);                            // T: e = 2
    fahre (st, s, apply (*st.tk, mitGain (*st.tk, -6.0), 2));    // U: e = 3
    int abgewiesen = 0;
    for (int i = 0; i < 31; ++i)
    {
        auto veraltet = apply (*st.tk, mitGain (*st.tk, 1.0), (std::uint64_t) (1000 + i));
        veraltet.baseRevision = 1;
        if (fahre (st, s, veraltet).ausgang == tx::Ausgang::konflikt) ++abgewiesen;
    }
    pruefe (abgewiesen == 31, "T12 Vorlauf: 31 Abweisungen wegen veralteter base_revision", std::to_string (abgewiesen));
    wachen (*st.tk, s, "T12 Vorlauf");
    const auto e = fahre (st, s, t);
    pruefeZeile ("T12", "abweisungen_verdraengen_keinen_committeten_eintrag", t, e, *st.tk,
                 { tx::Ausgang::gespeichertesErgebnis, tx::Stufe::s0, 3, true, true, eT.revision, eT.hash });
    wachen (*st.tk, s, "T12");
}

/** T13 bis T15: eine Stufe hinter S3 scheitert am Einspritzpunkt. */
void fallEinspritzung (const char* zeile, const char* test, tx::Stufe stufe)
{
    Stand st; Sitzung s;
    grundzustand (st, s, 100);                                   // r = 1
    const auto t = apply (*st.tk, mitGain (*st.tk, 6.0), 1);
    const auto vorher     = st.tk->bestaetigt();
    const auto hashVorher = st.tk->hash();
    const auto ringVorher = st.tk->undoRing();
    st.tk->setzeEinspritzung (stufe);
    const auto e = fahre (st, s, t);
    st.tk->setzeEinspritzung (tx::Stufe::keine);
    pruefeZeile (zeile, test, t, e, *st.tk, { tx::Ausgang::fehler, stufe, 1, false });
    pruefe (st.tk->bestaetigt() == vorher && st.tk->hash() == hashVorher && st.tk->undoRing() == ringVorher,
            std::string (zeile) + " bestaetigter Zustand, Hash und Undo-Ring unveraendert - kein halber Eintrag");
    wachen (*st.tk, s, zeile);
}

void fallT13() { fallEinspritzung ("T13", "fehler_in_S4_laesst_committed_und_hash_unveraendert", tx::Stufe::s4); }
void fallT14() { fallEinspritzung ("T14", "fehler_in_S6_laesst_committed_und_hash_unveraendert", tx::Stufe::s6); }
void fallT15() { fallEinspritzung ("T15", "fehler_in_S7_laesst_committed_und_hash_unveraendert", tx::Stufe::s7); }

void fallT16()
{
    // Der Pflichtweg aus M-61: Band ein -> Zone darueber ein (M-68) -> Remove -> Undo.
    Stand st; Sitzung s;
    grundzustand (st, s, 100);                                   // r = 1, Band bei 1000 Hz
    auto mitZone = st.tk->bestaetigt();
    mitZone.zonen = { { 0, 900.0, 1100.0, true } };
    const auto ez = fahre (st, s, apply (*st.tk, mitZone, 1));   // r = 2
    const auto er = fahre (st, s, remove (*st.tk, 0, 2));        // r = 3
    pruefe (ez.ausgang == tx::Ausgang::commit && er.ausgang == tx::Ausgang::commit,
            "T16 Vorlauf: Zone ueber dem Band (erlaubt) und Remove committen", beschreibe (ez) + " / " + beschreibe (er));
    const auto u = auftrag (*st.tk, tx::Art::undo, 3);
    const auto e = fahre (st, s, u);
    pruefeZeile ("T16", "undo_unter_einer_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung", u, e, *st.tk,
                 { tx::Ausgang::commit, tx::Stufe::s8, 4, true });
    const auto verletzt = tx::verletzteBaender (st.tk->wirksam(), st.tk->samplerate());
    pruefe (verletzt[0] && belegt (st.tk->bestaetigt(), 0) && freq (st.tk->bestaetigt(), 0) == 1000.0,
            "T16 das Band liegt wieder da und gilt als verletzt (M-68)");
    wachen (*st.tk, s, "T16");
}

void fallT17()
{
    Stand a; Sitzung sa; std::vector<tx::Auftrag> auftraege; std::vector<tx::HashText> h;
    dreiunddreissigCommits (a, sa, auftraege, h);                // vorige Sitzung: r = 33
    state::Zustand z;
    a.tk->schreibeIn (z);

    // Dieselbe Instanz laedt den Stand neu: ihr Register traegt noch die
    // vorige Sitzung, und genau das muss der Ladestart leeren (I5).
    Sitzung sb;
    juce::String grund;
    const bool geladen = a.tk->ladestart (z.dspDto(), (std::uint64_t) z.stateRevision, z.undoRing, z.undoCursor, grund);
    pruefe (geladen && a.tk->revision() == 33 && a.tk->ladeRevision() == 33 && a.tk->registerBelegung() == 0,
            "T17 Ladestart: r0 = r = 33, das Register ist leer", str (grund));
    wachen (*a.tk, sb, "T17 Ladestart");
    auto wieder = auftraege[32];                                 // in der VORIGEN Sitzung committet
    wieder.baseRevision = 33;
    const auto e = fahre (a, sb, wieder);
    pruefeZeile ("T17", "wiederholung_ueber_sitzungsgrenze_ist_neuer_versuch", wieder, e, *a.tk,
                 { tx::Ausgang::commit, tx::Stufe::s8, 34, true });
    wachen (*a.tk, sb, "T17");
}

enum class Zeile { T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, anzahl };

struct Fall
{
    Zeile zeile;
    void (*lauf)();
};

const Fall falltabelle[] = {
    { Zeile::T1,  fallT1 },  { Zeile::T2,  fallT2 },  { Zeile::T3,  fallT3 },  { Zeile::T4,  fallT4 },
    { Zeile::T5,  fallT5 },  { Zeile::T6,  fallT6 },  { Zeile::T7,  fallT7 },  { Zeile::T8,  fallT8 },
    { Zeile::T9,  fallT9 },  { Zeile::T10, fallT10 }, { Zeile::T11, fallT11 }, { Zeile::T12, fallT12 },
    { Zeile::T13, fallT13 }, { Zeile::T14, fallT14 }, { Zeile::T15, fallT15 }, { Zeile::T16, fallT16 },
    { Zeile::T17, fallT17 },
};

// M-125: "eine neue Zeile ohne Fall ist ein Uebersetzungsfehler".
static_assert (std::size (falltabelle) == (size_t) Zeile::anzahl,
               "M-125: jede Zeile T1 bis T17 der Falltabelle §5.11.4 hat genau einen Fall");

void falltabelleVollstaendigGefahren()
{
    abschnitt ("A - Falltabelle §5.11.4 Teil 4, tabellengetrieben mit Wachen I1/I2/I4 (M-125, zugleich M-44, M-123, M-124)");
    bool reihenfolge = true;
    for (size_t i = 0; i < std::size (falltabelle); ++i)
    {
        if (falltabelle[i].zeile != (Zeile) i) reihenfolge = false;
        falltabelle[i].lauf();
    }
    pruefe (reihenfolge, "falltabelle_vollstaendig_gefahren: jede Zeile T1 bis T17 genau einmal, in Tabellenreihenfolge",
            std::to_string (std::size (falltabelle)) + " Faelle");
}

//==============================================================================
void fehlerJeStufe()
{
    abschnitt ("B - Fehler in jeder fehlbaren Stufe S1 bis S7, je an ihrer Tabellenzeile (M-75)");
    struct Einspritzung { tx::Stufe stufe; tx::Ausgang ausgang; const char* zeile; };
    const Einspritzung faelle[] = {
        { tx::Stufe::s1, tx::Ausgang::konflikt,   "T2"  },
        { tx::Stufe::s2, tx::Ausgang::fehler,     "T3"  },
        { tx::Stufe::s3, tx::Ausgang::userSchutz, "T4"  },
        { tx::Stufe::s4, tx::Ausgang::fehler,     "T13" },
        { tx::Stufe::s5, tx::Ausgang::busyRetry,  "T5"  },
        { tx::Stufe::s6, tx::Ausgang::fehler,     "T14" },
        { tx::Stufe::s7, tx::Ausgang::fehler,     "T15" },
    };
    Stand st; Sitzung s;
    grundzustand (st, s, 100);
    const auto t = apply (*st.tk, mitGain (*st.tk, 6.0), 1);
    for (const auto& f : faelle)
    {
        const auto vorher = st.tk->bestaetigt();
        const auto hash   = st.tk->hash();
        const auto ring   = st.tk->undoRing();
        const auto r      = st.tk->revision();
        const int  belegt = st.tk->registerBelegung();
        st.tk->setzeEinspritzung (f.stufe);
        const auto e = fahre (st, s, t);
        st.tk->setzeEinspritzung (tx::Stufe::keine);
        const bool ok = e.ausgang == f.ausgang && e.stufe == f.stufe && ! e.memoisiert
                     && st.tk->bestaetigt() == vorher && st.tk->hash() == hash && st.tk->undoRing() == ring
                     && st.tk->revision() == r && st.tk->registerBelegung() == belegt
                     && ! registerTraegt (*st.tk, t.tid);
        pruefe (ok, std::string ("fehler_in_") + tx::wort (f.stufe) + "_laesst_committed_und_hash_unveraendert (" + f.zeile + ")",
                beschreibe (e));
    }
    wachen (*st.tk, s, "den sieben Einspritzungen");
    const auto e = fahre (st, s, t);
    pruefe (e.ausgang == tx::Ausgang::commit && e.revision == 2,
            "fehler_hinterlaesst_keinen_registereintrag: nach sieben Fehlern committet dieselbe tid", beschreibe (e));

    // S0 liest nur das vorallokierte Register (M-75).
    int treffer = 0;
    allokationen = 0;
    zaehleAllokationen = true;
    for (int i = 0; i < 10000; ++i)
        if (st.tk->fuehreAus (t).ausgang == tx::Ausgang::gespeichertesErgebnis) ++treffer;
    zaehleAllokationen = false;
    pruefe (allokationen == 0 && treffer == 10000, "nachschlag_allokiert_nicht_und_schlaegt_nicht_fehl (10.000 Nachschlaege)",
            "Allokationen " + zahl (allokationen) + ", Treffer " + std::to_string (treffer));
}

void zaehleAbS8 (tx::Stufe stufe, void*)
{
    if (stufe == tx::Stufe::s8) { allokationen = 0; zaehleAllokationen = true; }
}

void commitPunktAlloziertNicht()
{
    abschnitt ("C - Hinter dem Commit-Punkt wird nicht alloziert (§5.11.4 Teil 2)");
    Stand st; Sitzung s;
    st.tk->setzeStufenHaken (&zaehleAbS8, nullptr);
    std::uint64_t summe = 0;
    int commits = 0;
    const auto messe = [&] (const tx::Auftrag& a)
    {
        const auto e = st.tk->fuehreAus (a);
        zaehleAllokationen = false;
        if (e.ausgang == tx::Ausgang::commit) { ++commits; summe += allokationen; s.commits.push_back ({ a.tid, e.revision }); }
        allokationen = 0;
    };

    auto z = mitEq (true);
    std::uint64_t n = 1;
    for (int i = 0; i < 40; ++i)   // mehr als 32: Ringueberlauf und Registerverdraengung laufen mit
    {
        setzeBand (z, i % 8, 200.0 + 10.0 * i, (double) (i % 12));
        z.zonen.clear();
        if (i % 2 == 0) z.zonen.push_back ({ 1, 5000.0, 6000.0, true });
        messe (apply (*st.tk, z, n++));
    }
    messe (auftrag (*st.tk, tx::Art::undo, n++));
    messe (auftrag (*st.tk, tx::Art::undo, n++));
    messe (auftrag (*st.tk, tx::Art::redo, n++));
    messe (remove (*st.tk, 3, n++));                           // schneidet den Redo-Zweig ab
    messe (auftrag (*st.tk, tx::Art::neutralisieren, n++));
    messe (revert (*st.tk, st.tk->revision() - 1, n++));
    juce::MemoryBlock p; juce::String g;
    nakama::preset::schreibe (z, p, g);
    messe (presetLaden (*st.tk, p, n++));
    st.tk->setzeStufenHaken (nullptr, nullptr);

    pruefe (summe == 0 && commits == 47, "hinter_dem_commit_punkt_wird_nicht_alloziert",
            "Allokationen ab S8: " + zahl (summe) + " ueber " + std::to_string (commits)
            + " Commits (40 Apply mit Ringueberlauf und Registerverdraengung, Undo, Undo, Redo, Remove, Neutralisieren, Revert, Preset)");
    wachen (*st.tk, s, "den Commit-Messungen");
}

void protokolliere (tx::Stufe stufe, void* kontext)
{
    static_cast<std::vector<tx::Stufe>*> (kontext)->push_back (stufe);
}

void siebenTransaktionen()
{
    abschnitt ("D - Sieben Transaktionen, je genau eine Revision (M-74)");
    Stand st; Sitzung s;
    grundzustand (st, s, 100);                                   // r = 1
    const auto zustandR1 = st.tk->bestaetigt();
    const auto jeEine = [&] (const char* name, const tx::Auftrag& a, int ringZuwachs)
    {
        const auto r      = st.tk->revision();
        const auto ring   = (int) st.tk->undoRing().size();
        const int  belegt = st.tk->registerBelegung();
        const auto e = fahre (st, s, a);
        pruefe (e.ausgang == tx::Ausgang::commit && e.revision == r + 1 && st.tk->revision() == r + 1
                    && st.tk->registerBelegung() == belegt + 1
                    && (int) st.tk->undoRing().size() == ring + ringZuwachs,
                std::string (name) + "_erzeugt_genau_eine_revision",
                beschreibe (e) + ", Ring " + std::to_string (st.tk->undoRing().size())
                + ", Register " + std::to_string (st.tk->registerBelegung()));
        wachen (*st.tk, s, name);
    };
    jeEine ("apply",          apply (*st.tk, mitGain (*st.tk, 6.0), 1), +1);        // r = 2
    jeEine ("remove",         remove (*st.tk, 0, 2), +1);                            // r = 3
    jeEine ("undo",           auftrag (*st.tk, tx::Art::undo, 3), 0);                // r = 4
    jeEine ("redo",           auftrag (*st.tk, tx::Art::redo, 4), 0);                // r = 5
    jeEine ("neutralisieren", auftrag (*st.tk, tx::Art::neutralisieren, 5), +1);     // r = 6
    jeEine ("revert",         revert (*st.tk, 1, 6), +1);                            // r = 7
    pruefe (st.tk->bestaetigt() == zustandR1, "  revert fuehrt exakt auf den Zustand der referenzierten Revision 1");
    auto fremd = mitEq (false);
    setzeBand (fremd, 5, 3000.0, -4.0);
    juce::MemoryBlock p; juce::String g;
    const bool geschrieben = nakama::preset::schreibe (fremd, p, g);
    pruefe (geschrieben, "  das Preset dieses Laufs ist geschrieben", str (g));
    jeEine ("preset_laden",   presetLaden (*st.tk, p, 7), +1);                       // r = 8

    // Die Stufenfolge im Code: S0 bis S8 in dieser Reihenfolge; kein Ausgang
    // ohne Commit erreicht S8.
    std::vector<tx::Stufe> folge;
    st.tk->setzeStufenHaken (&protokolliere, &folge);
    fahre (st, s, apply (*st.tk, mitGain (*st.tk, 2.0), 8));
    const std::vector<tx::Stufe> vollstaendig { tx::Stufe::s0, tx::Stufe::s1, tx::Stufe::s2, tx::Stufe::s3, tx::Stufe::s4,
                                                tx::Stufe::s5, tx::Stufe::s6, tx::Stufe::s7, tx::Stufe::s8 };
    const bool commitFolge = folge == vollstaendig;
    bool keinS8OhneCommit = true;
    const auto t = apply (*st.tk, mitGain (*st.tk, 5.0), 9);
    for (const auto f : { tx::Stufe::s1, tx::Stufe::s2, tx::Stufe::s3, tx::Stufe::s4, tx::Stufe::s5, tx::Stufe::s6, tx::Stufe::s7 })
    {
        folge.clear();
        st.tk->setzeEinspritzung (f);
        const auto e = st.tk->fuehreAus (t);
        st.tk->setzeEinspritzung (tx::Stufe::keine);
        if (e.ausgang == tx::Ausgang::commit || folge.empty() || folge.back() != f
            || std::find (folge.begin(), folge.end(), tx::Stufe::s8) != folge.end())
            keinS8OhneCommit = false;
    }
    st.tk->setzeStufenHaken (nullptr, nullptr);
    pruefe (commitFolge && keinS8OhneCommit, "alle_fehlbaren_stufen_liegen_vor_dem_commit_punkt",
            std::string ("Commit S0..S8 in Reihenfolge: ") + (commitFolge ? "ja" : "nein")
            + ", S8 ohne Commit erreicht: " + (keinS8OhneCommit ? "nie" : "JA"));
}

void zehntausend()
{
    abschnitt ("E - 10.000 doppelte, vertauschte und veraltete Eingaben EINER logischen Transaktion (M-76)");
    Stand vorlauf; Sitzung sv;
    auto z = mitEq (true);
    setzeBand (z, 0, 1000.0, 0.0);
    for (int i = 1; i <= 7; ++i)
    {
        z.werte[(size_t) iBand (0, param::kGainDb)].zahl = (double) i;
        fahre (vorlauf, sv, apply (*vorlauf.tk, z, (std::uint64_t) i));
    }
    state::Zustand gespeichert;
    vorlauf.tk->schreibeIn (gespeichert);

    // Der Lauf beginnt unmittelbar nach dem Ladestart eines Standes mit r0 >= 1.
    Stand st; Sitzung s;
    juce::String grund;
    const bool geladen = st.tk->ladestart (gespeichert.dspDto(), (std::uint64_t) gespeichert.stateRevision,
                                           gespeichert.undoRing, gespeichert.undoCursor, grund);
    const auto r0 = st.tk->revision();
    const auto ausgangszustand = st.tk->bestaetigt();
    auto ziel = ausgangszustand;
    ziel.werte[(size_t) iBand (0, param::kGainDb)].zahl = -3.5;
    setzeBand (ziel, 3, 4000.0, 2.0);
    ziel.zonen = { { 2, 100.0, 200.0, false } };

    std::mt19937_64 zufall (20260910u);
    std::vector<std::uint64_t> basen;
    basen.reserve (10000);
    for (int i = 0; i < 10000; ++i)
        basen.push_back ((i % 3 == 0) ? r0 : (std::uint64_t) (zufall() % r0));   // aktuell oder veraltet
    std::shuffle (basen.begin(), basen.end(), zufall);                           // vertauschte Ankunft

    auto t = apply (*st.tk, ziel, 77);                                           // dieselbe tid, feste Nutzlast
    int t1 = 0, t2 = 0, t6 = 0, andere = 0, mischung = 0, wachenBruch = 0;
    std::string wachenDetail;
    for (const auto b : basen)
    {
        t.baseRevision = b;
        const auto e = fahre (st, s, t);
        if      (e.ausgang == tx::Ausgang::gespeichertesErgebnis && e.stufe == tx::Stufe::s0) ++t1;
        else if (e.ausgang == tx::Ausgang::konflikt && e.stufe == tx::Stufe::s1)             ++t2;
        else if (e.ausgang == tx::Ausgang::commit && e.stufe == tx::Stufe::s8)               ++t6;
        else                                                                                 ++andere;
        const auto& c = st.tk->bestaetigt();
        if (! (c == ausgangszustand || c == ziel)) ++mischung;
        std::string d;
        if (! wachenHalten (*st.tk, s, d)) { ++wachenBruch; wachenDetail = d; }
    }
    pruefe (geladen && r0 == 7 && st.tk->revision() - r0 <= 1 && t6 <= 1 && mischung == 0 && andere == 0,
            "zehntausend_transaktionen_erzeugen_hoechstens_eine_revision",
            "r0=" + zahl (r0) + ", r - r0 = " + zahl (st.tk->revision() - r0) + "; T1 " + std::to_string (t1)
            + ", T2 " + std::to_string (t2) + ", T6 " + std::to_string (t6) + ", andere " + std::to_string (andere)
            + ", Mischzustaende " + std::to_string (mischung));
    pruefe (wachenBruch == 0, "  Wachen I1/I2/I4 nach jeder der 10.000 Eingaben",
            wachenBruch == 0 ? std::string ("keine Verletzung") : wachenDetail);
}

//==============================================================================
bool zellenGleich (int index, const param::Zelle& a, const param::Zelle& b)
{
    switch (param::tabelle()[(size_t) index].typ)
    {
        case param::Typ::boolean:     return a.b == b.b;
        case param::Typ::gleitkomma:  return std::memcmp (&a.zahl, &b.zahl, sizeof (double)) == 0;
        case param::Typ::aufzaehlung: return a.enumIndex == b.enumIndex;
    }
    return false;
}

/** Slot-ID gleich, alle 13 Werte bitgleich, Belegung gleich. */
bool slotGleich (const param::DspSatz& a, const param::DspSatz& b, int slot, bool mitEnabled = true)
{
    for (int f = 0; f < param::kJeSlot; ++f)
    {
        if (! mitEnabled && f == param::kEnabled) continue;
        const int i = iBand (slot, f);
        if (! zellenGleich (i, a.werte[(size_t) i], b.werte[(size_t) i])) return false;
    }
    return belegt (a, slot) == belegt (b, slot);
}

juce::MemoryBlock speichereAus (const tx::Transaktionskern& tk)
{
    auto z = state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    z.common.klasse = state::Klasse::active_probe;
    z.hatParameters = true;
    tk.schreibeIn (z);
    juce::MemoryBlock m;
    state::speichere (z, m);
    return m;
}

bool ladeAus (tx::Transaktionskern& tk, const juce::MemoryBlock& bytes)
{
    state::Zustand z;
    if (state::lade (bytes.getData(), bytes.getSize(), state::Bundle::nkac(), z) != state::LadeErgebnis::geladen)
        return false;
    juce::String grund;
    return tk.ladestart (z.dspDto(), (std::uint64_t) z.stateRevision, z.undoRing, z.undoCursor, grund);
}

param::Zelle zahlZelle (double wert)
{
    param::Zelle z;
    z.zahl = wert;
    return z;
}

//==============================================================================
void wiederholungUndSitzung()
{
    abschnitt ("F - Wiederholung unter einer Zone, Ladestart und Register (M-123, M-124)");
    {
        // Zonenablauf aus MN2: 1000 Hz mit Zone darueber -> T auf 1050 Hz -> 500 Hz -> T wiederholt.
        Stand st; Sitzung s;
        grundzustand (st, s, 100);                               // Band 1000 Hz, r = 1
        auto mitZone = st.tk->bestaetigt();
        mitZone.zonen = { { 0, 1000.0, 1100.0, true } };
        fahre (st, s, apply (*st.tk, mitZone, 101));             // ueber dem Band: erlaubt (M-68), r = 2
        auto auf1050 = st.tk->bestaetigt();
        auf1050.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 1050.0;
        const auto t  = apply (*st.tk, auf1050, 1);
        const auto eT = fahre (st, s, t);                        // bleibt in der Zone: kein Eintritt, r = 3
        auto auf500 = st.tk->bestaetigt();
        auf500.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 500.0;
        fahre (st, s, apply (*st.tk, auf500, 2));                // verlaesst die Zone, r = 4
        const auto e = fahre (st, s, t);                         // am heutigen Zustand waere 500 -> 1050 ein Eintritt
        pruefe (eT.ausgang == tx::Ausgang::commit && e.ausgang == tx::Ausgang::gespeichertesErgebnis
                    && e.stufe == tx::Stufe::s0 && e.revision == eT.revision && e.hash == eT.hash
                    && freq (st.tk->bestaetigt(), 0) == 500.0 && st.tk->revision() == 4,
                "wiederholung_unter_einer_zone_bekommt_nicht_user_schutz", beschreibe (e));
        wachen (*st.tk, s, "dem Zonenablauf");
    }
    {
        Stand a; Sitzung sa; std::vector<tx::Auftrag> auftraege; std::vector<tx::HashText> h;
        dreiunddreissigCommits (a, sa, auftraege, h);
        fahre (a, sa, auftrag (*a.tk, tx::Art::undo, 500));     // r = 34, Cursor 1
        state::Zustand z;
        a.tk->schreibeIn (z);
        const auto ringVorher   = a.tk->undoRing();
        const int  belegtVorher = a.tk->registerBelegung();
        juce::String grund, hex, g2;
        const bool geladen = a.tk->ladestart (z.dspDto(), (std::uint64_t) z.stateRevision, z.undoRing, z.undoCursor, grund);
        const bool hashOk  = param::stateHash (z.dspDto(), hex, g2) && tx::alsText (a.tk->hash()) == hex;
        pruefe (geladen && belegtVorher == 32 && a.tk->registerBelegung() == 0 && a.tk->revision() == 34 && a.tk->ladeRevision() == 34
                    && a.tk->undoRing() == ringVorher && a.tk->undoCursor() == 1 && hashOk,
                "ladestart_hat_leeres_register_und_haelt_die_revision (dieselbe Instanz, vorher 32 Registereintraege)",
                "r=" + zahl (a.tk->revision()) + " r0=" + zahl (a.tk->ladeRevision()) + " belegt "
                + std::to_string (belegtVorher) + " -> " + std::to_string (a.tk->registerBelegung()) + ", Cursor " + std::to_string (a.tk->undoCursor()));
    }
    {
        // I5: das Register ist nicht Teil des Zustands.
        Stand st; Sitzung s;
        grundzustand (st, s, 100);
        const auto t = apply (*st.tk, mitGain (*st.tk, 6.0), 1);
        fahre (st, s, t);
        const bool eintragVorher = registerTraegt (*st.tk, t.tid);
        const auto hashVorher    = st.tk->hash();
        const auto rVorher       = st.tk->revision();
        const auto mitRegister   = speichereAus (*st.tk);
        const bool geladen       = ladeAus (*st.tk, mitRegister);   // dieselbe Instanz
        const auto ohneRegister  = speichereAus (*st.tk);
        pruefe (geladen && eintragVorher && st.tk->registerBelegung() == 0
                    && mitRegister == ohneRegister && st.tk->hash() == hashVorher && st.tk->revision() == rVorher,
                "register_ist_nicht_teil_des_zustands: Speichern schreibt es nicht, der Ladestart leert es, Hash und Revision bleiben",
                std::to_string ((int) mitRegister.getSize()) + " Bytes");
    }
}

void belegungRemoveUndo()
{
    abschnitt ("G - Belegung, Remove und Undo als ein Objekt (M-58 bis M-62)");
    {
        Stand st; Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 2, 700.0, -5.0);
        fahre (st, s, apply (*st.tk, z, 1));
        const auto vorher = st.tk->bestaetigt();
        auto aus = vorher;
        aus.werte[(size_t) iBand (2, param::kEnabled)].b = false;
        const auto e1 = fahre (st, s, apply (*st.tk, aus, 2));
        const auto ausgeschaltet = st.tk->bestaetigt();
        auto an = ausgeschaltet;
        an.werte[(size_t) iBand (2, param::kEnabled)].b = true;
        const auto e2 = fahre (st, s, apply (*st.tk, an, 3));
        pruefe (e1.ausgang == tx::Ausgang::commit && belegt (ausgeschaltet, 2) && slotGleich (ausgeschaltet, vorher, 2, false)
                    && e2.ausgang == tx::Ausgang::commit && slotGleich (st.tk->bestaetigt(), vorher, 2),
                "ausschalten_gibt_keinen_slot_frei (M-59): Slot bleibt belegt, Werte bleiben, Einschalten nimmt dieselben");
    }
    {
        Stand st; Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 3, 300.0, 4.0);
        setzeBand (z, 5, 5000.0, -2.0);
        fahre (st, s, apply (*st.tk, z, 1));
        const auto vorher = st.tk->bestaetigt();
        const auto r = st.tk->revision();
        const auto e = fahre (st, s, remove (*st.tk, 3, 2));
        const auto nachher = st.tk->bestaetigt();
        pruefe (e.ausgang == tx::Ausgang::commit && st.tk->revision() == r + 1 && ! belegt (nachher, 3)
                    && slotGleich (nachher, mitEq (false), 3),
                "remove_ist_eine_transaktion_und_eine_revision (M-60): occupied false UND alle 13 Werte auf Default", beschreibe (e));
        pruefe (belegt (nachher, 5) && slotGleich (nachher, vorher, 5), "remove_nummeriert_nicht_um (M-60): Slot 5 bleibt Slot 5");
    }
    {
        Stand st; Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 4, 2500.0, 7.5);
        z.werte[(size_t) iBand (4, param::kType)].enumIndex            = 2;      // high_shelf
        z.werte[(size_t) iBand (4, param::kChannelMode)].enumIndex     = 4;      // side
        z.werte[(size_t) iBand (4, param::kQ)].zahl                    = 0.7071067811865476;
        z.werte[(size_t) iBand (4, param::kDynamicEnabled)].b          = true;
        z.werte[(size_t) iBand (4, param::kDynamicRangeDb)].zahl       = -6.0;
        z.werte[(size_t) iBand (4, param::kThresholdDb)].zahl          = -31.0;
        z.werte[(size_t) iBand (4, param::kAttackMs)].zahl             = 3.3;
        z.werte[(size_t) iBand (4, param::kHoldMs)].zahl               = 12.0;
        z.werte[(size_t) iBand (4, param::kReleaseMs)].zahl            = 250.0;
        z.werte[(size_t) iBand (4, param::kSidechainSource)].enumIndex = 1;      // internal
        fahre (st, s, apply (*st.tk, z, 1));
        const auto vorher = st.tk->bestaetigt();
        fahre (st, s, remove (*st.tk, 4, 2));
        const auto& objekt = st.tk->undoRing().back();
        const bool einObjekt = objekt.art == state::UndoArt::remove && objekt.slot == 4 && slotGleich (objekt.zustand, vorher, 4);
        const auto r = st.tk->revision();
        const auto e = fahre (st, s, auftrag (*st.tk, tx::Art::undo, 3));
        pruefe (einObjekt && e.ausgang == tx::Ausgang::commit && st.tk->revision() == r + 1 && st.tk->bestaetigt() == vorher,
                "undo_stellt_das_entfernte_band_als_ein_objekt_her (M-61): Slot 4, Typ, channel_mode, Dynamic und alle Werte in EINER Transaktion",
                beschreibe (e));
    }
    {
        Stand st; Sitzung s;
        fahre (st, s, apply (*st.tk, mitEq (true), 1));
        bool achtBelegt = true;
        for (int i = 0; i < 8; ++i)
        {
            auto a = auftrag (*st.tk, tx::Art::bandBelegen, (std::uint64_t) (10 + i));
            a.band = bandWerte (100.0 + 100.0 * i, 1.0);
            const auto e = fahre (st, s, a);
            if (e.ausgang != tx::Ausgang::commit || e.slot != i) achtBelegt = false;
        }
        const auto vorher = st.tk->bestaetigt();
        const auto r = st.tk->revision();
        auto neunte = auftrag (*st.tk, tx::Art::bandBelegen, 99);
        neunte.band = bandWerte (9000.0, 1.0);
        const auto e = fahre (st, s, neunte);
        pruefe (achtBelegt && e.ausgang == tx::Ausgang::fehler && e.stufe == tx::Stufe::s4 && e.grund == "alle_slots_belegt"
                    && st.tk->bestaetigt() == vorher && st.tk->revision() == r,
                "acht_belegte_slots_ueberschreiben_nichts (M-62): eigener Fehler, kein Opfer", beschreibe (e));
    }
    {
        // M-58 am Kern selbst, ohne die Kernsicht des Transaktionskerns: ein
        // freier Slot mit hoerbaren Werten, Dynamik und Auto-Gain an rechnet
        // keinen Filter, keinen Detektor, keinen Auto-Gain-Beitrag und keine
        // Auslenkung.
        auto ohneWerte = mitEq (true);
        setzeBand (ohneWerte, 0, 1000.0, 6.0);
        ohneWerte.werte[(size_t) param::kIndexAutoGain].b = true;
        auto mitWerten = ohneWerte;
        mitWerten.werte[(size_t) iBand (2, param::kEnabled)].b           = true;
        mitWerten.werte[(size_t) iBand (2, param::kFreqHz)].zahl         = 3000.0;
        mitWerten.werte[(size_t) iBand (2, param::kGainDb)].zahl         = 12.0;
        mitWerten.werte[(size_t) iBand (2, param::kDynamicEnabled)].b    = true;
        mitWerten.werte[(size_t) iBand (2, param::kDynamicRangeDb)].zahl = -6.0;
        mitWerten.werte[(size_t) iBand (2, param::kThresholdDb)].zahl    = -60.0;
        const bool frei = ! belegt (mitWerten, 2);
        auto kernOhne = std::make_unique<dsp::DspKern>();
        auto kernMit  = std::make_unique<dsp::DspKern>();
        kernOhne->bereiteVor (48000.0, 256);
        kernMit->bereiteVor (48000.0, 256);
        const bool uebernommen = kernOhne->uebernehmeZustand (ohneWerte) && kernMit->uebernehmeZustand (mitWerten);
        juce::Random w (58);
        std::vector<float> l (256), rr (256), l2 (256), r2 (256);
        bool gleich = true;
        for (int b = 0; b < 200; ++b)
        {
            for (int n = 0; n < 256; ++n) { l[(size_t) n] = w.nextFloat() - 0.5f; rr[(size_t) n] = w.nextFloat() - 0.5f; }
            l2 = l; r2 = rr;
            float* ohneZeiger[2] = { l.data(), rr.data() };
            float* mitZeiger[2]  = { l2.data(), r2.data() };
            kernOhne->verarbeite (ohneZeiger, 2, 256);
            kernMit->verarbeite (mitZeiger, 2, 256);
            if (std::memcmp (l.data(), l2.data(), 256 * sizeof (float)) != 0 || std::memcmp (rr.data(), r2.data(), 256 * sizeof (float)) != 0)
                gleich = false;
        }
        double auslenkung[8] {};
        kernMit->auslenkungenDb (auslenkung);
        auto progOhne = std::make_unique<dsp::DspProgramm>();
        auto progMit  = std::make_unique<dsp::DspProgramm>();
        dsp::baueProgramm (ohneWerte, 48000.0, 0, *progOhne);
        dsp::baueProgramm (mitWerten, 48000.0, 0, *progMit);
        pruefe (frei && uebernommen && gleich && auslenkung[2] == 0.0 && ! progMit->baender[2].aktiv
                    && std::memcmp (&progOhne->autoGainDb, &progMit->autoGainDb, sizeof (double)) == 0,
                "freier_slot_verarbeitet_nichts (M-58): Slot 2 frei mit +12 dB, Dynamik und Auto-Gain an - Ausgang bitgleich, Auto-Gain gleich, keine Auslenkung",
                "Auto-Gain " + std::to_string (progOhne->autoGainDb) + " / " + std::to_string (progMit->autoGainDb)
                + " dB, Auslenkung Slot 2 " + std::to_string (auslenkung[2]) + " dB");
    }
}

//==============================================================================
void schutzZonen()
{
    abschnitt ("H - Schutz-Zonen (M-66 bis M-73, M-126)");
    {
        Stand st; Sitzung s;
        grundzustand (st, s, 100);                               // Band 1000 Hz
        std::uint64_t n = 5000;
        const auto versuche = [&] (param::Schutzzone z)
        {
            auto ziel = st.tk->bestaetigt();
            ziel.zonen = { z };
            const auto e = fahre (st, s, apply (*st.tk, ziel, n++));
            return e.ausgang == tx::Ausgang::fehler && e.stufe == tx::Stufe::s2 && e.grund.startsWith ("bereich");
        };
        const bool gleich = versuche ({ 0, 500.0, 500.0, true });
        const bool hoch   = versuche ({ 0, 500.0, 20001.0, true });
        const bool tief   = versuche ({ 0, 19.9, 500.0, true });
        const bool idAus  = versuche ({ 8, 500.0, 600.0, true });
        auto rand = st.tk->bestaetigt();
        rand.zonen = { { 7, 20.0, 20000.0, false } };
        const auto e = fahre (st, s, apply (*st.tk, rand, n++));
        pruefe (gleich && hoch && tief && idAus && e.ausgang == tx::Ausgang::commit,
                "zone_dto_grenzen (M-66): 20 <= low < high <= 20000 und id 0..7 im DTO geprueft, die Raender selbst gueltig",
                beschreibe (e));
    }
    {
        Stand st (44100.0); Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 0, 19860.0, 2.0);
        fahre (st, s, apply (*st.tk, z, 1));
        auto mitZone = st.tk->bestaetigt();
        mitZone.zonen = { { 0, 19900.0, 20000.0, true } };
        const auto e = fahre (st, s, apply (*st.tk, mitZone, 2));    // ueber dem liegenden Band: erlaubt
        const auto bei441 = tx::verletzteBaender (st.tk->bestaetigt(), 44100.0);
        const auto bei96  = tx::verletzteBaender (st.tk->bestaetigt(), 96000.0);
        state::Zustand gespeichert;
        st.tk->schreibeIn (gespeichert);
        pruefe (e.ausgang == tx::Ausgang::commit && bei441[0] && ! bei96[0] && gespeichert.schutzZonen.size() == 1
                    && gespeichert.schutzZonen[0].lowHz == 19900.0 && gespeichert.schutzZonen[0].highHz == 20000.0,
                "zone_laufzeitkappung_aendert_nichts_persistentes (M-66): bei 44,1 kHz auf 0,45 fs gekappt, persistent 19900-20000 Hz");
    }
    {
        Stand st; Sitzung s;
        const auto t = zonenVorlauf (st, s, true);               // Band 500 Hz, Zone 900-1100 Hz an
        const auto r = st.tk->revision();
        const auto h = st.tk->hash();
        const auto vorher = st.tk->bestaetigt();
        const auto e = fahre (st, s, t);
        pruefe (e.ausgang == tx::Ausgang::userSchutz && e.stufe == tx::Stufe::s3,
                "band_in_zone_ziehen_wird_abgewiesen (M-67)", beschreibe (e));
        pruefe (st.tk->revision() == r && st.tk->hash() == h && st.tk->bestaetigt() == vorher,
                "abweisung_laesst_hash_und_revision_unveraendert (M-67): keine halbe Anwendung");
    }
    {
        Stand st; Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 3.0, false);                    // ausgeschaltet in der Zone: P ist falsch
        z.zonen = { { 0, 900.0, 1100.0, true } };
        const auto e0 = fahre (st, s, apply (*st.tk, z, 1));
        auto an = st.tk->bestaetigt();
        an.werte[(size_t) iBand (0, param::kEnabled)].b = true;
        const auto e = fahre (st, s, apply (*st.tk, an, 2));
        pruefe (e0.ausgang == tx::Ausgang::commit && e.ausgang == tx::Ausgang::userSchutz && e.stufe == tx::Stufe::s3
                    && ! st.tk->bestaetigt().werte[(size_t) iBand (0, param::kEnabled)].b,
                "einschalten_in_der_zone_wird_abgewiesen (M-67)", beschreibe (e));
    }
    {
        Stand st; Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 0, 100.0, 3.0);
        z.zonen = { { 1, 900.0, 1100.0, true } };                // Zone B
        fahre (st, s, apply (*st.tk, z, 1));
        auto mitA = st.tk->bestaetigt();
        mitA.zonen = { { 0, 80.0, 120.0, true }, { 1, 900.0, 1100.0, true } };
        const auto ea = fahre (st, s, apply (*st.tk, mitA, 2)); // Zone A ueber dem Band: erlaubt, Band verletzt
        const bool verletztInA = tx::verletzteBaender (st.tk->bestaetigt(), st.tk->samplerate())[0];
        auto nachB = st.tk->bestaetigt();
        nachB.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 1000.0;
        const auto e = fahre (st, s, apply (*st.tk, nachB, 3));
        pruefe (ea.ausgang == tx::Ausgang::commit && verletztInA && e.ausgang == tx::Ausgang::userSchutz
                    && freq (st.tk->bestaetigt(), 0) == 100.0,
                "von_zone_a_nach_zone_b_wird_abgewiesen (M-67, B-03): je Zone einzeln, kein Sammelzustand", beschreibe (e));
    }
    {
        Stand st; Sitzung s;
        grundzustand (st, s, 100);                               // Band 1000 Hz
        auto mitZone = st.tk->bestaetigt();
        mitZone.zonen = { { 0, 900.0, 1100.0, true } };
        const auto e = fahre (st, s, apply (*st.tk, mitZone, 1));
        tx::DspBericht b; juce::String g;
        const bool gebaut = tx::baueBericht (*st.tk, b, g);
        pruefe (e.ausgang == tx::Ausgang::commit && gebaut && b.verletzteBaender == std::vector<int> { 0 }
                    && belegt (st.tk->bestaetigt(), 0) && gain (st.tk->bestaetigt(), 0) == 3.0,
                "zone_ueber_band_ist_erlaubt_und_meldet_verletzung (M-68): das Band klingt weiter und steht im Bericht", beschreibe (e));
        const auto e69 = fahre (st, s, apply (*st.tk, mitGain (*st.tk, -4.0), 2));
        auto innen = st.tk->bestaetigt();
        innen.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 950.0;       // innerhalb derselben Zone
        const auto e69f = fahre (st, s, apply (*st.tk, innen, 3));
        pruefe (e69.ausgang == tx::Ausgang::commit && gain (st.tk->bestaetigt(), 0) == -4.0
                    && e69f.ausgang == tx::Ausgang::commit && freq (st.tk->bestaetigt(), 0) == 950.0,
                "verletztes_band_bleibt_bedienbar (M-69): Gain und Frequenz innerhalb seiner Zone",
                beschreibe (e69) + " / " + beschreibe (e69f));
    }
    {
        Stand st; Sitzung s;
        zonenVorlauf (st, s, true);                              // Band 500 Hz, Zone 900-1100 Hz, r = 1
        const auto r = st.tk->revision();
        st.tk->automationSchreiben (iBand (0, param::kFreqHz), zahlZelle (1000.0));
        tx::DspBericht b; juce::String g;
        tx::baueBericht (*st.tk, b, g);
        pruefe (st.tk->revision() == r && freq (st.tk->bestaetigt(), 0) == 500.0 && freq (st.tk->wirksam(), 0) == 1000.0
                    && b.verletzteBaender == std::vector<int> { 0 },
                "automation_verletzt_und_meldet_denselben_zustand (M-70): vorbeigeschrieben, keine Revision, dieselbe Meldung");
    }
    {
        Stand st; Sitzung s;
        grundzustand (st, s, 100);
        auto mitZone = st.tk->bestaetigt();
        mitZone.zonen = { { 0, 900.0, 1100.0, true } };
        fahre (st, s, apply (*st.tk, mitZone, 1));
        auto aus = st.tk->bestaetigt();
        for (auto& zone : aus.zonen) zone.enabled = false;
        const auto r1 = st.tk->revision();
        const auto e1 = fahre (st, s, apply (*st.tk, aus, 2));
        const bool keinerVerletzt = ! tx::verletzteBaender (st.tk->wirksam(), st.tk->samplerate())[0];
        auto raus = st.tk->bestaetigt(); raus.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 500.0;
        auto rein = st.tk->bestaetigt(); rein.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 1000.0;
        const auto er = fahre (st, s, apply (*st.tk, raus, 3));
        rein = st.tk->bestaetigt(); rein.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 1000.0;
        const auto ei = fahre (st, s, apply (*st.tk, rein, 4));
        pruefe (e1.ausgang == tx::Ausgang::commit && st.tk->revision() == r1 + 3 && keinerVerletzt
                    && er.ausgang == tx::Ausgang::commit && ei.ausgang == tx::Ausgang::commit,
                "zone_deaktivieren_ist_eine_transaktion (M-71): danach ist kein Band verletzt und der Weg hinein frei", beschreibe (ei));

        auto wieder = st.tk->bestaetigt(); for (auto& zone : wieder.zonen) zone.enabled = true;
        fahre (st, s, apply (*st.tk, wieder, 5));                // ueber dem Band wieder eingeschaltet: erlaubt
        auto entfernt = st.tk->bestaetigt(); entfernt.zonen.clear();
        const auto r2 = st.tk->revision();
        const auto e2 = fahre (st, s, apply (*st.tk, entfernt, 6));
        pruefe (e2.ausgang == tx::Ausgang::commit && st.tk->revision() == r2 + 1 && st.tk->bestaetigt().zonen.empty()
                    && ! tx::verletzteBaender (st.tk->wirksam(), st.tk->samplerate())[0],
                "zone_entfernen_ist_eine_transaktion (M-71)", beschreibe (e2));
    }
    {
        Stand a; Sitzung sa; grundzustand (a, sa, 100);
        Stand b; Sitzung sb; grundzustand (b, sb, 100);
        auto mitZone = b.tk->bestaetigt();
        mitZone.zonen = { { 3, 4000.0, 5000.0, false } };
        fahre (b, sb, apply (*b.tk, mitZone, 1));
        fahre (a, sa, apply (*a.tk, a.tk->bestaetigt(), 1));      // gleiche Werte, gleiche Revision, keine Zone
        juce::MemoryBlock pb; juce::String g;
        nakama::preset::schreibe (b.tk->bestaetigt(), pb, g);
        tx::DspBericht bb;
        tx::baueBericht (*b.tk, bb, g);
        state::Zustand zb;
        b.tk->schreibeIn (zb);
        const auto zonenText = juce::String ("\"schutz_zonen\":[{");
        pruefe (a.tk->hash() != b.tk->hash() && pb.toString().contains (zonenText) && bb.jcs.contains (zonenText)
                    && zb.schutzZonen == b.tk->bestaetigt().zonen,
                "zonen_sind_teil_von_hash_preset_bericht_und_state (M-72): eine Zonenliste, vier Orte");
    }
    {
        Stand st; Sitzung s;
        grundzustand (st, s, 100);
        auto neun = st.tk->bestaetigt();
        neun.zonen.clear();
        for (int i = 0; i < 9; ++i) neun.zonen.push_back ({ i, 100.0 + 1000.0 * i, 200.0 + 1000.0 * i, false });
        const auto e9 = fahre (st, s, apply (*st.tk, neun, 1));
        auto doppelt = st.tk->bestaetigt();
        doppelt.zonen = { { 2, 100.0, 200.0, false }, { 2, 300.0, 400.0, false } };
        const auto ed = fahre (st, s, apply (*st.tk, doppelt, 2));
        pruefe (e9.ausgang == tx::Ausgang::fehler && e9.stufe == tx::Stufe::s2 && e9.grund.startsWith ("zone_anzahl"),
                "neunte_zone_wird_abgewiesen (M-73)", beschreibe (e9));
        pruefe (ed.ausgang == tx::Ausgang::fehler && ed.stufe == tx::Stufe::s2 && ed.grund.startsWith ("zone_doppelt"),
                "doppelte_zonen_id_wird_abgewiesen (M-73)", beschreibe (ed));
    }
    {
        Stand st; Sitzung s;
        auto z = mitEq (true);
        z.zonen = { { 0, 900.0, 1100.0, true } };
        fahre (st, s, apply (*st.tk, z, 1));                      // Zone an, kein Band
        auto voll = z;
        setzeBand (voll, 0, 1000.0, 5.0);                         // das Preset legt ein Band in die Zone
        juce::MemoryBlock p; juce::String g;
        nakama::preset::schreibe (voll, p, g);
        const auto ep = fahre (st, s, presetLaden (*st.tk, p, 2));
        const bool verletztNachPreset = tx::verletzteBaender (st.tk->wirksam(), st.tk->samplerate())[0];
        pruefe (ep.ausgang == tx::Ausgang::commit && ep.stufe == tx::Stufe::s8 && belegt (st.tk->bestaetigt(), 0) && verletztNachPreset,
                "preset_laden_in_eine_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung (M-126, T16)", beschreibe (ep));
        fahre (st, s, auftrag (*st.tk, tx::Art::undo, 3));
        const bool wegNachUndo = ! belegt (st.tk->bestaetigt(), 0);
        const auto er = fahre (st, s, auftrag (*st.tk, tx::Art::redo, 4));
        pruefe (wegNachUndo && er.ausgang == tx::Ausgang::commit && er.stufe == tx::Stufe::s8 && belegt (st.tk->bestaetigt(), 0)
                    && tx::verletzteBaender (st.tk->wirksam(), st.tk->samplerate())[0],
                "redo_in_eine_zone_wird_nicht_abgewiesen_sondern_meldet_verletzung (M-126, T16)", beschreibe (er));
    }
}

//==============================================================================
void undoRing()
{
    abschnitt ("I - Undo-Ring: Tiefe, Redo-Zweig, Neutralisieren (M-78 bis M-80)");
    {
        Stand st; Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 0.0);
        for (int i = 1; i <= 40; ++i)
        {
            z.werte[(size_t) iBand (0, param::kGainDb)].zahl = (double) ((i % 24) - 12) + 0.5;
            fahre (st, s, apply (*st.tk, z, (std::uint64_t) i));
        }
        const auto& ring = st.tk->undoRing();
        const auto bytes = speichereAus (*st.tk);
        Stand b;
        const bool geladen = ladeAus (*b.tk, bytes);
        pruefe (ring.size() == 32 && ring.front().revision == 8 && ring.back().revision == 39 && geladen
                    && b.tk->undoRing() == ring && bytes.getSize() < 16u * 1024u * 1024u,
                "undo_ring_haelt_tiefe_32 (M-78): beim 33. Eintrag faellt der aelteste heraus; der volle Ring reist durch Speichern und Laden",
                "Ring " + std::to_string (ring.size()) + ", aeltester rev " + std::to_string (ring.empty() ? -1 : ring.front().revision)
                + ", juengster rev " + std::to_string (ring.empty() ? -1 : ring.back().revision) + ", " + std::to_string ((int) bytes.getSize()) + " Bytes");
    }
    {
        Stand st; Sitzung s;
        grundzustand (st, s, 100);                               // Ring 1
        fahre (st, s, apply (*st.tk, mitGain (*st.tk, 1.0), 1)); // Ring 2
        fahre (st, s, apply (*st.tk, mitGain (*st.tk, 2.0), 2)); // Ring 3
        fahre (st, s, auftrag (*st.tk, tx::Art::undo, 3));       // Cursor 1
        const bool cursorEins = st.tk->undoCursor() == 1;
        fahre (st, s, apply (*st.tk, mitGain (*st.tk, 9.0), 4)); // neue Transaktion
        const auto e = fahre (st, s, auftrag (*st.tk, tx::Art::redo, 5));
        pruefe (cursorEins && st.tk->undoCursor() == 0 && st.tk->undoRing().size() == 3
                    && e.ausgang == tx::Ausgang::fehler && e.stufe == tx::Stufe::s4 && e.grund == "redo_leer"
                    && gain (st.tk->bestaetigt(), 0) == 9.0,
                "neue_transaktion_schneidet_den_redo_zweig_ab (M-79): ein Cursor im selben Ring", beschreibe (e));
    }
    {
        Stand st; Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 1, 800.0, 5.0);
        z.werte[(size_t) param::indexVonId ("v1.global.input_trim_db")].zahl = 3.0;
        z.werte[(size_t) param::kIndexMix].zahl = 0.5;
        z.zonen = { { 4, 2000.0, 3000.0, false } };
        fahre (st, s, apply (*st.tk, z, 1));
        const auto vorher = st.tk->bestaetigt();
        const auto ring = st.tk->undoRing().size();
        const auto e = fahre (st, s, auftrag (*st.tk, tx::Art::neutralisieren, 2));
        auto erwartet = mitEq (true);
        erwartet.zonen = vorher.zonen;
        const bool neutral = st.tk->bestaetigt() == erwartet;
        const bool ringWaechst = st.tk->undoRing().size() == ring + 1 && st.tk->undoRing().back().art == state::UndoArt::neutralisieren;
        fahre (st, s, auftrag (*st.tk, tx::Art::undo, 3));
        pruefe (e.ausgang == tx::Ausgang::commit && neutral && ringWaechst && st.tk->bestaetigt() == vorher,
                "neutralisieren_loescht_die_historie_nicht (M-80): alle Werte auf Default, Rollenschalter und Zonen bleiben, Undo fuehrt zurueck",
                beschreibe (e));
    }
}

void overlays()
{
    abschnitt ("J - AutomationOverlay und PreviewOverlay am Kern (M-81, M-83)");
    {
        Stand st; Sitzung s;
        grundzustand (st, s, 100);
        const auto r      = st.tk->revision();
        const auto ring   = st.tk->undoRing();
        const int  belegt = st.tk->registerBelegung();
        const auto epoche0 = st.tk->automation().epoche;
        double letzter = 0.0;
        for (int i = 0; i < 100; ++i)
        {
            letzter = -6.0 + 0.1 * i;
            st.tk->automationSchreiben (iBand (0, param::kGainDb), zahlZelle (letzter));
        }
        const auto epoche1 = st.tk->automation().epoche;
        st.tk->automationRuht();
        const auto epoche2 = st.tk->automation().epoche;
        st.tk->automationRuht();
        const auto epoche3 = st.tk->automation().epoche;
        pruefe (st.tk->revision() == r && st.tk->undoRing() == ring && st.tk->registerBelegung() == belegt
                    && gain (st.tk->wirksam(), 0) == letzter && gain (st.tk->bestaetigt(), 0) == 3.0,
                "automation_erzeugt_keine_revision (M-81): 100 Automationspunkte, r, Ring und Register unveraendert");
        pruefe (epoche1 == epoche0 + 1 && epoche2 == epoche0 + 2 && epoche3 == epoche2,
                "epoche_wechselt_genau_zweimal_je_geste (M-81)",
                zahl (epoche0) + " -> " + zahl (epoche1) + " -> " + zahl (epoche2) + " -> " + zahl (epoche3));
    }
    {
        Stand st; Sitzung s;
        grundzustand (st, s, 100);
        const auto ohne = speichereAus (*st.tk);
        auto vorschau = mitGain (*st.tk, -9.0);
        setzeBand (vorschau, 6, 7000.0, 4.0);
        juce::String g;
        const bool gesetzt = st.tk->setzePreview (vorschau, g);
        const auto mit = speichereAus (*st.tk);
        state::Zustand z;
        st.tk->schreibeIn (z);
        pruefe (gesetzt && st.tk->preview().aktiv && ohne == mit && z.dspDto() == st.tk->bestaetigt(),
                "preview_wird_nie_serialisiert (M-83): Speichern waehrend der Vorschau ergibt dieselben Bytes", str (g));
        st.tk->beendePreview();
    }
}

juce::File wurzel()
{
    auto d = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    while (d.exists() && ! d.getChildFile ("eq-copilot").isDirectory())
    {
        const auto oben = d.getParentDirectory();
        if (oben == d) break;
        d = oben;
    }
    return d;
}

void vertragsweg()
{
    abschnitt ("K - Validator, Kanalmodus, Dynamikwerte, Klemmliste, Auto-Gain und Hash (M-14, M-23, M-24, M-28, M-39, M-77)");
    {
        Stand st; Sitzung s;
        grundzustand (st, s, 100);
        auto qKlein = st.tk->bestaetigt();
        qKlein.werte[(size_t) iBand (0, param::kQ)].zahl = 0.1;
        const auto a = fahre (st, s, apply (*st.tk, mitGain (*st.tk, 18.0), 1));
        const auto b = fahre (st, s, apply (*st.tk, mitGain (*st.tk, std::numeric_limits<double>::quiet_NaN()), 2));
        const auto c = fahre (st, s, apply (*st.tk, qKlein, 3));
        pruefe (a.stufe == tx::Stufe::s2 && a.grund.startsWith ("bereich") && b.stufe == tx::Stufe::s2 && b.grund.startsWith ("nichtendlich")
                    && c.stufe == tx::Stufe::s2 && c.grund.startsWith ("bereich") && gain (st.tk->bestaetigt(), 0) == 3.0
                    && st.tk->revision() == 1,
                "apply_mit_bereichsverletzung_wird_abgewiesen (M-14): abgelehnt, nicht geklemmt",
                beschreibe (a) + " / " + beschreibe (b) + " / " + beschreibe (c));
    }
    {
        Stand st; Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 2, 1500.0, 2.0);
        z.werte[(size_t) iBand (2, param::kDynamicEnabled)].b    = true;
        z.werte[(size_t) iBand (2, param::kDynamicRangeDb)].zahl = -7.25;
        z.werte[(size_t) iBand (2, param::kThresholdDb)].zahl    = -33.5;
        z.werte[(size_t) iBand (2, param::kAttackMs)].zahl       = 2.2;
        z.werte[(size_t) iBand (2, param::kHoldMs)].zahl         = 17.0;
        z.werte[(size_t) iBand (2, param::kReleaseMs)].zahl      = 333.0;
        fahre (st, s, apply (*st.tk, z, 1));
        const auto vorher = st.tk->bestaetigt();
        auto aus = vorher;
        aus.werte[(size_t) iBand (2, param::kDynamicEnabled)].b = false;
        fahre (st, s, apply (*st.tk, aus, 2));
        auto an = st.tk->bestaetigt();
        an.werte[(size_t) iBand (2, param::kDynamicEnabled)].b = true;
        fahre (st, s, apply (*st.tk, an, 3));
        bool gleich = true;
        for (const int f : { param::kDynamicRangeDb, param::kThresholdDb, param::kAttackMs, param::kHoldMs, param::kReleaseMs })
            if (! zellenGleich (iBand (2, f), st.tk->bestaetigt().werte[(size_t) iBand (2, f)], vorher.werte[(size_t) iBand (2, f)]))
                gleich = false;
        pruefe (gleich && st.tk->revision() == 3 && st.tk->bestaetigt().werte[(size_t) iBand (2, param::kDynamicEnabled)].b,
                "dynamicwerte_ueberleben_aus_und_ein_bitgleich (M-24, ueber zwei Transaktionen)");
    }
    {
        Stand st; Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 3.0);
        z.werte[(size_t) iBand (0, param::kChannelMode)].enumIndex = 4;           // side
        fahre (st, s, apply (*st.tk, z, 1));
        auto typ = st.tk->bestaetigt();
        typ.werte[(size_t) iBand (0, param::kType)].enumIndex = 1;                // low_shelf
        fahre (st, s, apply (*st.tk, typ, 2));
        const bool ungekoppelt = st.tk->bestaetigt().werte[(size_t) iBand (0, param::kChannelMode)].enumIndex == 4
                              && st.tk->bestaetigt().werte[(size_t) iBand (0, param::kType)].enumIndex == 1;
        auto falsch = st.tk->bestaetigt();
        falsch.werte[(size_t) iBand (0, param::kChannelMode)].enumIndex = 5;      // kein sechster Modus
        const auto ef = fahre (st, s, apply (*st.tk, falsch, 3));
        auto neu = auftrag (*st.tk, tx::Art::bandBelegen, 4);
        neu.band = bandWerte (2000.0, 1.0);
        const auto en = fahre (st, s, neu);
        pruefe (ungekoppelt && ef.ausgang == tx::Ausgang::fehler && ef.stufe == tx::Stufe::s2 && ef.grund.startsWith ("enum"),
                "channel_mode_ist_diskret_und_ungekoppelt (M-28)", beschreibe (ef));
        pruefe (en.ausgang == tx::Ausgang::commit && en.slot == 1
                    && st.tk->bestaetigt().werte[(size_t) iBand (1, param::kChannelMode)].enumIndex == 0
                    && st.tk->bestaetigt().werte[(size_t) iBand (0, param::kChannelMode)].enumIndex == 4,
                "kein_slot_erbt_den_modus (M-28)", beschreibe (en));
    }
    {
        // M-23 und E2-7: bei 44,1 kHz liegt Slot 2 zusaetzlich ueber 0,45 fs
        // und wird gekappt - der Bericht fuehrt trotzdem hoechstens EINE
        // Klemmung je Slot, und das ist die R7-Klemmung (v3 `dsp_klemmung`).
        Stand st (44100.0); Sitzung s;
        auto z = mitEq (true);
        setzeBand (z, 2, 19900.0, 3.0);
        z.werte[(size_t) iBand (2, param::kDynamicEnabled)].b            = true;
        z.werte[(size_t) iBand (2, param::kDynamicRangeDb)].zahl         = -6.0;
        z.werte[(size_t) iBand (2, param::kSidechainSource)].enumIndex   = 2;     // priority_sidechain
        fahre (st, s, apply (*st.tk, z, 1));
        tx::DspBericht b; juce::String g;
        tx::baueBericht (*st.tk, b, g);
        const bool gemeldet = b.klemmungen.size() == 1 && b.klemmungen[0].id == "v1.band.2.sidechain_source"
                           && b.klemmungen[0].gemeldet == "priority_sidechain" && b.klemmungen[0].wirksam == "internal";
        auto prog = std::make_unique<dsp::DspProgramm>();
        dsp::baueProgramm (st.tk->bestaetigt(), 44100.0, 0, *prog);
        pruefe (gemeldet && prog->baender[2].gekapptFreq
                    && st.tk->bestaetigt().werte[(size_t) iBand (2, param::kSidechainSource)].enumIndex == 2,
                "klemmliste_erscheint_im_bericht (M-23, E2-7): geklemmt auf internal, persistent weiter priority_sidechain; ein Eintrag je Slot, obwohl die Frequenz zusaetzlich gekappt ist",
                std::to_string (b.klemmungen.size()) + " Klemmung(en)");
    }
    {
        Stand a (44100.0); Sitzung sa;
        Stand b (96000.0); Sitzung sb;
        auto z = mitEq (true);
        setzeBand (z, 0, 15000.0, 9.0);
        z.werte[(size_t) iBand (0, param::kType)].enumIndex = 2;                 // high_shelf
        z.werte[(size_t) param::kIndexAutoGain].b = true;
        fahre (a, sa, apply (*a.tk, z, 1));
        fahre (b, sb, apply (*b.tk, z, 1));
        tx::DspBericht ba, bb; juce::String g;
        tx::baueBericht (*a.tk, ba, g);
        tx::baueBericht (*b.tk, bb, g);
        pruefe (a.tk->hash() == b.tk->hash() && ba.jcs == bb.jcs && std::abs (ba.autoGainDb - bb.autoGainDb) > 1.0e-6,
                "auto_gain_ist_nicht_im_hash (M-39): derselbe Hash bei verschiedenem abgeleitetem Wert",
                "44,1 kHz " + std::to_string (ba.autoGainDb) + " dB, 96 kHz " + std::to_string (bb.autoGainDb) + " dB");
    }
    {
        const auto fixtures = wurzel().getChildFile ("eq-copilot/fixtures/state");
        const auto manifest = juce::JSON::parse (fixtures.getChildFile ("MANIFEST.json"));
        int gleich = 0, gesamt = 0;
        if (const auto* liste = manifest["dto_gueltig"].getArray())
        {
            for (const auto& eintrag : *liste)
            {
                ++gesamt;
                juce::MemoryBlock roh;
                fixtures.getChildFile (eintrag["datei"].toString()).loadFileAsData (roh);
                param::DspSatz satz; juce::String grund, detail, g2;
                Stand st;
                if (param::ausDtoText (roh.getData(), roh.getSize(), satz, grund, detail)
                    && st.tk->ladestart (satz, 0, {}, 0, g2)
                    && tx::alsText (st.tk->hash()) == eintrag["state_hash"].toString())
                    ++gleich;
            }
        }
        pruefe (gesamt >= 7 && gleich == gesamt,
                "hash_kommt_aus_dem_bestandskanon (M-77): der Kern hasht jedes gueltige DTO-Fixture wie MANIFEST.json",
                std::to_string (gleich) + "/" + std::to_string (gesamt));
    }
}

void presets()
{
    abschnitt ("L - Preset (M-95 bis M-100)");
    auto voll = mitEq (false);                                   // die Quelle des Presets ist NICHT engagiert
    for (int slot = 0; slot < param::kSlots; ++slot)
        setzeBand (voll, slot, 100.0 + 900.0 * slot, -6.0 + slot, slot % 2 == 0);
    voll.werte[(size_t) param::indexVonId ("v1.global.bypass")].b           = true;
    voll.werte[(size_t) param::indexVonId ("v1.global.input_trim_db")].zahl  = 2.5;
    voll.werte[(size_t) param::indexVonId ("v1.global.output_trim_db")].zahl = -1.5;
    voll.werte[(size_t) param::indexVonId ("v1.global.width")].zahl          = 1.25;
    voll.werte[(size_t) param::indexVonId ("v1.global.mono_bass_hz")].zahl   = 120.0;
    voll.werte[(size_t) param::kIndexMix].zahl                               = 0.625;
    voll.werte[(size_t) param::kIndexAutoGain].b                             = true;
    voll.zonen = { { 1, 150.0, 250.0, true }, { 6, 8000.0, 9000.0, false } };

    juce::MemoryBlock p; juce::String g;
    const bool geschrieben = nakama::preset::schreibe (voll, p, g);
    const auto text = p.toString();
    const auto json = juce::JSON::parse (text);
    pruefe (geschrieben && json.hasProperty ("preset_schema_version") && (int) json["preset_schema_version"] == 1
                && json.hasProperty ("dsp_schema_version") && (int) json["dsp_schema_version"] == 2,
            "preset_traegt_zwei_getrennte_versionen (M-95): preset_schema_version 1 neben dsp_schema_version 2", str (g));

    bool ohneIdentitaet = true;
    for (const auto& name : nakama::preset::verboteneFelder())
        if (json.hasProperty (juce::Identifier (name)) || text.contains ("\"" + name + "\""))
            ohneIdentitaet = false;
    pruefe (ohneIdentitaet, "preset_hat_keine_identitaetsfelder (M-97): keines der sechs Felder existiert im geschriebenen Preset");

    Stand st; Sitzung s;
    auto andere = mitEq (true);                                  // das Ziel IST engagiert
    setzeBand (andere, 3, 333.0, 1.0);
    fahre (st, s, apply (*st.tk, andere, 1));
    const auto vorher     = st.tk->bestaetigt();
    const auto r          = st.tk->revision();
    const auto ringVorher = st.tk->undoRing();
    const auto e = fahre (st, s, presetLaden (*st.tk, p, 2));
    auto erwartet = voll;
    erwartet.werte[(size_t) param::kIndexEqEnabled].b = true;    // der Rollenschalter des Ziels bleibt
    pruefe (e.ausgang == tx::Ausgang::commit && st.tk->bestaetigt() == erwartet,
            "preset_traegt_den_klanginhalt_vollstaendig (M-96): 119 Werte und Zonen kommen an", beschreibe (e));
    const auto& ringNachher = st.tk->undoRing();
    pruefe (st.tk->revision() == r + 1 && ringNachher.size() == ringVorher.size() + 1
                && std::equal (ringVorher.begin(), ringVorher.end(), ringNachher.begin())
                && ringNachher.back().art == state::UndoArt::presetLaden,
            "preset_laden_ist_eine_transaktion (M-100): eine Revision, ein Undo-Eintrag; der bestehende Ring wird ergaenzt, nicht ersetzt");
    pruefe (e.ausgang == tx::Ausgang::commit && st.tk->bestaetigt().werte[(size_t) param::kIndexEqEnabled].b,
            "preset_laesst_eq_enabled_unberuehrt (M-100, R12): das Ziel bleibt engagiert, obwohl die Quelle des Presets es nicht war");
    fahre (st, s, auftrag (*st.tk, tx::Art::undo, 3));
    pruefe (st.tk->bestaetigt() == vorher, "undo_nach_preset_stellt_alles_her (M-100)");

    // Die Negativfixtures der Etappe 2, als Preset-Laden gefahren.
    const auto fixtures = wurzel().getChildFile ("eq-copilot/fixtures/state");
    const auto manifest = juce::JSON::parse (fixtures.getChildFile ("MANIFEST.json"));
    int gesamt = 0, richtig = 0, verboten = 0, verbotenAbgelehnt = 0, majorAbgelehnt = 0;
    if (const auto* liste = manifest["preset_ungueltig"].getArray())
    {
        for (const auto& eintrag : *liste)
        {
            juce::MemoryBlock roh;
            fixtures.getChildFile (eintrag["datei"].toString()).loadFileAsData (roh);
            const auto rv = st.tk->revision();
            const auto ef = fahre (st, s, presetLaden (*st.tk, roh, 1000u + (std::uint64_t) gesamt));
            ++gesamt;
            const auto sollGrund = eintrag["grund"].toString();
            const bool fiel = ef.ausgang == tx::Ausgang::fehler && ef.stufe == tx::Stufe::s2
                           && ef.grund.startsWith (sollGrund) && st.tk->revision() == rv;
            if (fiel) ++richtig;
            if (sollGrund == "verbotenes_feld") { ++verboten; if (fiel) ++verbotenAbgelehnt; }
            if (eintrag["datei"].toString().contains ("preset-major-2") && fiel && ef.grund.startsWith ("preset_schema_version"))
                ++majorAbgelehnt;
        }
    }
    pruefe (verboten == 6 && verbotenAbgelehnt == 6,
            "identitaetsfeld_wird_abgelehnt (M-97): je Feld ein Negativfixture, als Preset-Laden in S2 abgewiesen",
            std::to_string (verbotenAbgelehnt) + "/" + std::to_string (verboten));
    pruefe (majorAbgelehnt == 1, "unbekanntes_preset_major_wird_abgelehnt (M-99): klarer Fehler preset_schema_version, keine read-only-Haltung");
    pruefe (gesamt > 0 && richtig == gesamt, "  jedes ungueltige Preset-Fixture faellt mit seinem Manifestgrund",
            std::to_string (richtig) + "/" + std::to_string (gesamt));

    juce::MemoryBlock unbekannt;
    fixtures.getChildFile ("preset/gueltig/unbekanntes-top-level-feld.json").loadFileAsData (unbekannt);
    const auto eu = fahre (st, s, presetLaden (*st.tk, unbekannt, 2000));
    pruefe (eu.ausgang == tx::Ausgang::commit, "unbekanntes_top_level_feld_wird_ignoriert (M-98)", beschreibe (eu));
}

//==============================================================================
// Ebene 2: DERSELBE Kern im echten SondeProcessor

using Prozessor = nakama::sonde::SondeProcessor;

std::unique_ptr<Prozessor> prozessor (double rate = 48000.0, int block = 512)
{
    auto p = std::make_unique<Prozessor>();
    p->setRateAndBufferSizeDetails (rate, block);
    p->prepareToPlay (rate, block);
    return p;
}

tx::Ergebnis setze (Prozessor& p, const param::DspSatz& z)
{
    tx::Auftrag a;
    a.tid          = p.neueTid();
    a.baseRevision = p.stateRevision();
    a.art          = tx::Art::apply;
    a.satz         = z;
    return p.fuehreTransaktionAus (a);
}

tx::Ergebnis ohneNutzlast (Prozessor& p, tx::Art art, int slot = -1)
{
    tx::Auftrag a;
    a.tid          = p.neueTid();
    a.baseRevision = p.stateRevision();
    a.art          = art;
    a.slot         = slot;
    return p.fuehreTransaktionAus (a);
}

/** Faehrt `bloecke` Bloecke eines festen Rauschens und liefert den Ausgang. */
std::vector<float> fahreAudio (Prozessor& p, int bloecke, int groesse, int saat)
{
    std::vector<float> aus;
    aus.reserve ((size_t) (bloecke * groesse * 2));
    juce::Random w (saat);
    juce::MidiBuffer midi;
    for (int b = 0; b < bloecke; ++b)
    {
        juce::AudioBuffer<float> puffer (2, groesse);
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < groesse; ++n)
                puffer.setSample (k, n, w.nextFloat() * 1.6f - 0.8f);
        p.processBlock (puffer, midi);
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < groesse; ++n)
                aus.push_back (puffer.getSample (k, n));
    }
    return aus;
}

bool bitgleich (const std::vector<float>& a, const std::vector<float>& b)
{
    return a.size() == b.size() && std::memcmp (a.data(), b.data(), a.size() * sizeof (float)) == 0;
}

/** Zaehlt Host-Dirty ueber den echten JUCE-Weg: `updateHostDisplay` ruft
    `audioProcessorChanged` jedes Listeners synchron. */
struct DirtyZaehler final : juce::AudioProcessorListener
{
    int nichtParameter = 0;
    void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) override {}
    void audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails& d) override
    {
        if (d.nonParameterStateChanged) ++nichtParameter;
    }
};

juce::RangedAudioParameter& hostParam (Prozessor& p, int index)
{
    return *dynamic_cast<juce::RangedAudioParameter*> (p.getParameters()[index]);
}

/** Ein Hostereignis wie aus dem VST3-Wrapper: `setValueNotifyingHost` ohne
    Herkunftstag, also Automation. */
void hostSchreibt (Prozessor& p, int index, float denormiert)
{
    auto& q = hostParam (p, index);
    q.setValueNotifyingHost (q.convertTo0to1 (denormiert));
}

int aktiveBaenke (Prozessor& p)
{
    int n = 0;
    for (int s = 0; s < dsp::DspBankPool::kBaenke; ++s)
        if (p.dspKernFuerTest().pool().zustand (s) == dsp::BankZustand::audioAktiv) ++n;
    return n;
}

void prozessorParameter()
{
    abschnitt ("M - Prozessor: 112 Host-Parameter in Vertragsreihenfolge (M-64, M-87, M-88)");
    auto p = prozessor();
    const auto& liste = p->getParameters();
    const auto& t = param::tabelle();
    int reihenfolge = 0, typUndGrenzen = 0;
    for (int i = 0; i < liste.size() && i < param::kHostParameter; ++i)
    {
        const auto* mitId = dynamic_cast<juce::AudioProcessorParameterWithID*> (liste[i]);
        const auto& b = t[(size_t) i];
        if (mitId != nullptr && mitId->paramID == b.id && liste[i]->getParameterIndex() == i) ++reihenfolge;
        bool ok = false;
        switch (b.typ)
        {
            case param::Typ::boolean:
                if (auto* x = dynamic_cast<juce::AudioParameterBool*> (liste[i])) ok = x->get() == b.standardBool;
                break;
            case param::Typ::gleitkomma:
                if (auto* x = dynamic_cast<juce::AudioParameterFloat*> (liste[i]))
                    ok = x->getNormalisableRange().start == (float) b.min && x->getNormalisableRange().end == (float) b.max
                      && x->get() == (float) b.standardZahl;
                break;
            case param::Typ::aufzaehlung:
                if (auto* x = dynamic_cast<juce::AudioParameterChoice*> (liste[i])) ok = x->choices == b.werte && x->getIndex() == b.standardIndex;
                break;
        }
        if (ok) ++typUndGrenzen;
    }
    const auto idVon = [&] (int i) { const auto* m = dynamic_cast<juce::AudioProcessorParameterWithID*> (liste[i]); return m != nullptr ? m->paramID : juce::String(); };
    const bool anzahl = liste.size() == param::kHostParameter;
    pruefe (anzahl && reihenfolge == param::kHostParameter && typUndGrenzen == param::kHostParameter,
            "apvts_fuehrt_112_parameter_in_vertragsreihenfolge (M-88): Kennung, Index, Typ, Grenzen und Default aus parameter::tabelle()",
            std::to_string (liste.size()) + " Parameter, Reihenfolge " + std::to_string (reihenfolge) + ", Typ/Grenzen " + std::to_string (typUndGrenzen));
    pruefe (anzahl && idVon (param::kIndexEqEnabled) == "v2.global.eq_enabled" && idVon (param::kIndexMix) == "v2.global.mix"
                && idVon (param::kIndexAutoGain) == "v2.global.auto_gain" && idVon (param::kAnzahlV1 - 1) == "v1.band.7.sidechain_source",
            "v2_haengt_hinten_an (M-88): eq_enabled, mix und auto_gain auf 109, 110 und 111");
    bool occupiedFehlt = true;
    for (int i = 0; i < liste.size(); ++i)
        if (idVon (i).endsWith (".occupied")) occupiedFehlt = false;
    pruefe (occupiedFehlt, "occupied_ist_kein_hostparameter (M-64): nicht in der Hostliste, von aussen nicht automatisierbar");
    int v2Gesamt = 0, v2Richtig = 0;
    for (int i = 0; i < param::kAnzahl; ++i)
    {
        if (t[(size_t) i].layout != 2) continue;
        ++v2Gesamt;
        bool inHostliste = false;
        for (int k = 0; k < liste.size(); ++k)
            if (idVon (k) == t[(size_t) i].id) inHostliste = true;
        if (inHostliste == t[(size_t) i].hostParameter) ++v2Richtig;
    }
    pruefe (v2Gesamt == 11 && v2Richtig == 11,
            "jede_v2_kennung_erklaert_ihren_ort (M-87): host_parameter genau dann, wenn sie in der Hostliste steht",
            std::to_string (v2Richtig) + "/" + std::to_string (v2Gesamt));
}

void prozessorZustand()
{
    abschnitt ("N - Prozessor: Host-Dirty, Kind Dsp, erster Block, Freigabe, Altstand und fremdes Major (M-08, M-09, M-85, M-89, M-90, M-92)");
    {
        auto p = prozessor();
        DirtyZaehler zaehler;
        p->addListener (&zaehler);
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 3.0);
        const auto e1 = setze (*p, z);
        const int nachCommit = zaehler.nichtParameter;
        auto zone = p->bestaetigterZustand();
        zone.zonen = { { 0, 1500.0, 2500.0, true } };
        const auto e2 = setze (*p, zone);
        const int nachZone = zaehler.nichtParameter;
        auto rein = p->bestaetigterZustand();
        rein.werte[(size_t) iBand (0, param::kFreqHz)].zahl = 2000.0;   // in die Zone: User-Schutz
        const auto e3 = setze (*p, rein);
        const int nachAbweisung = zaehler.nichtParameter;
        juce::MemoryBlock bytes;
        p->getStateInformation (bytes);
        auto q = prozessor();
        DirtyZaehler ladeZaehler;
        q->addListener (&ladeZaehler);
        q->setStateInformation (bytes.getData(), (int) bytes.getSize());
        pruefe (e1.ausgang == tx::Ausgang::commit && e2.ausgang == tx::Ausgang::commit && nachCommit == 1 && nachZone == 2,
                "angenommene_transaktion_meldet_dirty (M-85): je Commit genau eine nonParameterStateChanged-Meldung",
                std::to_string (nachCommit) + " / " + std::to_string (nachZone));
        pruefe (e3.ausgang == tx::Ausgang::userSchutz && nachAbweisung == 2 && ladeZaehler.nichtParameter == 0,
                "abgewiesene_meldet_nicht (M-85): die Abweisung am Zonenriegel und das Laden melden kein Dirty",
                beschreibe (e3) + ", Laden " + std::to_string (ladeZaehler.nichtParameter));
        p->removeListener (&zaehler);
        q->removeListener (&ladeZaehler);
    }
    {
        auto a = prozessor();
        auto z = mitEq (true);
        setzeBand (z, 1, 700.0, 4.0);
        setzeBand (z, 6, 9000.0, -2.0);
        z.zonen = { { 3, 100.0, 300.0, false }, { 5, 5000.0, 6000.0, true } };
        setze (*a, z);                                            // r = 1, Ring 1
        ohneNutzlast (*a, tx::Art::remove, 6);                    // r = 2, Ring 2
        ohneNutzlast (*a, tx::Art::undo);                         // r = 3, Cursor 1
        juce::MemoryBlock bytes;
        a->getStateInformation (bytes);
        const auto baum = juce::ValueTree::readFromData (bytes.getData(), bytes.getSize());
        const auto dspKind = baum.getChildWithName ("Dsp");
        auto b = std::make_unique<Prozessor>();
        b->setStateInformation (bytes.getData(), (int) bytes.getSize());
        const auto& zb = b->zustandLesen();
        const auto* ring = dspKind.getProperty ("undo_ring_v1").getArray();
        pruefe (dspKind.isValid() && (juce::int64) dspKind.getProperty ("state_revision") == 3 && ring != nullptr && ring->size() == 2
                    && (int) dspKind.getProperty ("undo_cursor") == 1
                    && b->bestaetigterZustand() == a->bestaetigterZustand() && b->stateRevision() == 3
                    && zb.undoRing == a->zustandLesen().undoRing && zb.undoCursor == 1 && b->stateHashText() == a->stateHashText(),
                "prozessor_speichert_und_laedt_das_kind_dsp (M-89, Prozessorhaelfte): Revision, Belegung, Zonen, Undo-Ring und Cursor kommen an",
                std::to_string ((int) bytes.getSize()) + " Bytes");
    }
    {
        auto a = prozessor();
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 6.0);
        setze (*a, z);
        juce::MemoryBlock bytes;
        a->getStateInformation (bytes);
        auto b = std::make_unique<Prozessor>();
        b->setStateInformation (bytes.getData(), (int) bytes.getSize());   // wie ein Host: Laden VOR prepareToPlay
        const bool eqGeladen = b->bestaetigterZustand().werte[(size_t) param::kIndexEqEnabled].b
                            && hostParam (*b, param::kIndexEqEnabled).getValue() >= 0.5f;
        b->setRateAndBufferSizeDetails (48000.0, 512);
        b->prepareToPlay (48000.0, 512);
        const int vorBlock = aktiveBaenke (*b);
        const auto uebernahmenVor = b->dspKernFuerTest().uebernahmen();
        fahreAudio (*b, 1, 64, 1);
        const int nachBlock = aktiveBaenke (*b);
        pruefe (eqGeladen, "eq_enabled_ueberlebt_speichern_laden (M-08): im bestaetigten Zustand und im Hostparameter");
        pruefe (vorBlock == 0 && uebernahmenVor == 0 && nachBlock == 1,
                "bank_wird_erst_beim_ersten_block_aktiv (M-08): nach Laden und Vorbereitung keine aktive Bank, nach dem ersten Block genau eine",
                "vor " + std::to_string (vorBlock) + ", nach " + std::to_string (nachBlock));
    }
    {
        auto p = prozessor();
        fahreAudio (*p, 4, 64, 2);                                // der Callback lief ...
        const auto vorher = p->dspKernFuerTest().uebernahmen();
        auto z = mitEq (true);                                    // ... und steht jetzt still (Smart Disable)
        setzeBand (z, 0, 1000.0, 12.0);
        const auto e = setze (*p, z);
        const auto ohneCallback = p->dspKernFuerTest().uebernahmen();
        juce::AudioBuffer<float> puffer (2, 64), kopie (2, 64);
        juce::Random w (3);
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < 64; ++n)
                puffer.setSample (k, n, w.nextFloat() - 0.5f);
        kopie.makeCopyOf (puffer);
        juce::MidiBuffer midi;
        p->processBlock (puffer, midi);
        const bool gehoert = std::memcmp (puffer.getReadPointer (0), kopie.getReadPointer (0), 64 * sizeof (float)) != 0;
        pruefe (e.ausgang == tx::Ausgang::commit && ohneCallback == vorher && p->dspKernFuerTest().uebernahmen() == vorher + 1 && gehoert,
                "ohne_callback_bleibt_gespeichert_und_wird_nachgeholt (M-09): bestaetigt ohne Callback, uebernommen vom naechsten Block vor seiner Ausgabe",
                "Uebernahmen " + zahl (vorher) + " -> " + zahl (ohneCallback) + " -> " + zahl (p->dspKernFuerTest().uebernahmen()));
    }
    {
        // prepareToPlay und releaseResources sind ein Paar (E4-22): die Freigabe
        // gibt die Kernpuffer frei, ein Block ohne Vorbereitung laeuft
        // unberuehrt durch, und die naechste Vorbereitung laesst denselben
        // bestaetigten Zustand klingen wie eine frische Instanz.
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 6.0);
        auto a = prozessor (48000.0, 256);
        setze (*a, z);
        fahreAudio (*a, 10, 256, 70);
        a->releaseResources();
        const bool freigegeben = a->dspKernFuerTest().samplerate() == 0.0 && a->dspKernFuerTest().pool().belegteSlots() == 0;
        const auto ohneVorbereitung = fahreAudio (*a, 2, 256, 71);
        std::vector<float> eingang;
        {
            juce::Random w (71);
            for (int blk = 0; blk < 2; ++blk)
                for (int k = 0; k < 2; ++k)
                    for (int n = 0; n < 256; ++n)
                        eingang.push_back (w.nextFloat() * 1.6f - 0.8f);
        }
        a->setRateAndBufferSizeDetails (48000.0, 256);
        a->prepareToPlay (48000.0, 256);
        const auto ya = fahreAudio (*a, 100, 256, 72);
        auto c = prozessor (48000.0, 256);
        setze (*c, z);
        const auto yc = fahreAudio (*c, 100, 256, 72);
        auto d = prozessor (48000.0, 256);
        const auto yd = fahreAudio (*d, 100, 256, 72);
        pruefe (freigegeben && bitgleich (ohneVorbereitung, eingang) && a->stateRevision() == 1 && bitgleich (ya, yc) && ! bitgleich (ya, yd),
                "freigabe_und_vorbereitung_sind_ein_paar (E4-22): releaseResources gibt den Kern frei, ohne Vorbereitung unberuehrt, danach klingt derselbe bestaetigte Zustand",
                std::to_string (ya.size()) + " Samples");
    }
    {
        // M-90 und R5, Prozessorhaelfte: der Writer-Golden eines v1-Altstands
        // aus Etappe 2 laedt im Prozessor und wird der bestaetigte Zustand.
        juce::MemoryBlock v1;
        const bool gelesen = wurzel().getChildFile ("eq-copilot/fixtures/state/schema2/layout-v1.bin").loadFileAsData (v1);
        state::Zustand direkt;
        const bool geladen = state::lade (v1.getData(), v1.getSize(), state::Bundle::nkac(), direkt) == state::LadeErgebnis::geladen;
        auto p = prozessor();
        p->setStateInformation (v1.getData(), (int) v1.getSize());
        const auto& z = p->zustandLesen();
        juce::MemoryBlock bytes;
        p->getStateInformation (bytes);
        const auto par = juce::ValueTree::readFromData (bytes.getData(), bytes.getSize()).getChildWithName ("Parameters");
        pruefe (gelesen && geladen && ! z.nurLesen && z.layoutV1Migriert && p->bestaetigterZustand() == direkt.dspDto()
                    && p->stateRevision() == 0 && belegt (p->bestaetigterZustand(), 4) && (int) par.getProperty ("dsp_schema_version") == 2,
                "altstand_v1_laedt_im_prozessor_verlustfrei (M-90, R5, Prozessorhaelfte): der migrierte Stand wird der bestaetigte Zustand, Revision 0, gespeichert als v2",
                std::to_string ((int) v1.getSize()) + " Bytes v1 -> " + std::to_string ((int) bytes.getSize()) + " Bytes");
    }
    {
        // M-92, Prozessorhaelfte: ein fremdes Major bleibt read-only mit
        // Originalbytes, laedt NEUTRAL und nimmt keine Transaktion an.
        juce::MemoryBlock fremd;
        const bool gelesen = wurzel().getChildFile ("eq-copilot/fixtures/state/schema2/fremdes-major-3.bin").loadFileAsData (fremd);
        auto p = prozessor();
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 9.0);
        const auto e0 = setze (*p, z);                            // vorher engagiert
        p->setStateInformation (fremd.getData(), (int) fremd.getSize());
        const bool neutral = p->bestaetigterZustand() == param::DspSatz {} && p->stateRevision() == 0;
        const auto e = setze (*p, z);
        juce::MemoryBlock zurueck;
        p->getStateInformation (zurueck);
        pruefe (gelesen && e0.ausgang == tx::Ausgang::commit && p->zustandLesen().nurLesen && neutral
                    && e.ausgang == tx::Ausgang::fehler && e.grund == "schreibgeschuetzt"
                    && p->bestaetigterZustand() == param::DspSatz {} && zurueck == fremd,
                "fremdes_major_laedt_neutral_und_bleibt_schreibgeschuetzt (M-92, Prozessorhaelfte): read-only mit Originalbytes, eq_enabled aus, keine Transaktion",
                beschreibe (e) + ", " + std::to_string ((int) zurueck.getSize()) + " Bytes");
    }
}

void prozessorAutomation()
{
    abschnitt ("O - Prozessor: Automation, Gestus, Vorschau, Reload, Offline, nicht-endlicher Hostwert (M-52, M-63, M-81 bis M-84, M-119, M-120)");
    {
        auto mit = prozessor(); auto ohne = prozessor();
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 6.0);
        setze (*mit, z); setze (*ohne, z);
        // Erst uebernimmt der Kern das bestaetigte Programm; eine noch nicht
        // uebernommene Publikation verdeckte sonst eine zweite.
        fahreAudio (*mit, 4, 128, 8); fahreAudio (*ohne, 4, 128, 8);
        hostSchreibt (*mit, iBand (5, param::kGainDb), 12.0f);      // Slot 5 ist frei
        hostSchreibt (*mit, iBand (5, param::kEnabled), 1.0f);
        hostSchreibt (*mit, iBand (5, param::kType), 2.0f);
        hostSchreibt (*mit, iBand (5, param::kFreqHz), 3000.0f);
        mit->kontrollTaktFuerTest();
        const auto a = fahreAudio (*mit, 200, 128, 9);
        const auto b = fahreAudio (*ohne, 200, 128, 9);
        auto& gainParam = hostParam (*mit, iBand (5, param::kGainDb));
        const bool wertFolgt = gainParam.convertFrom0to1 (gainParam.getValue()) == 12.0f && gain (mit->wirksamerZustand(), 5) == 12.0;
        const auto uebernahmenMit  = mit->dspKernFuerTest().uebernahmen();
        const auto uebernahmenOhne = ohne->dspKernFuerTest().uebernahmen();
        pruefe (wertFolgt && ! belegt (mit->bestaetigterZustand(), 5) && ! belegt (mit->wirksamerZustand(), 5) && bitgleich (a, b)
                    && mit->stateRevision() == ohne->stateRevision() && uebernahmenMit == uebernahmenOhne,
                "automation_auf_freien_slot_aendert_werte_nicht_klang (M-63): der Parameter folgt; Belegung, Programm und Audio bleiben gleich",
                std::to_string (a.size()) + " Samples verglichen, Programmuebernahmen " + zahl (uebernahmenMit) + " / " + zahl (uebernahmenOhne));
    }
    {
        auto mit = prozessor(); auto ohne = prozessor();
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 9.0);
        setze (*mit, z); setze (*ohne, z);
        hostSchreibt (*mit, iBand (0, param::kType), 1.0f);         // bell -> low_shelf: topologisch
        hostSchreibt (*mit, iBand (0, param::kChannelMode), 3.0f);  // stereo -> mid: topologisch
        mit->kontrollTaktFuerTest();
        const auto a = fahreAudio (*mit, 200, 128, 10);
        const auto b = fahreAudio (*ohne, 200, 128, 10);
        const auto w = mit->wirksamerZustand();
        auto& typParam = hostParam (*mit, iBand (0, param::kType));
        pruefe (typParam.convertFrom0to1 (typParam.getValue()) == 1.0f && w.werte[(size_t) iBand (0, param::kType)].enumIndex == 0
                    && w.werte[(size_t) iBand (0, param::kChannelMode)].enumIndex == 0 && bitgleich (a, b),
                "capability_entscheidet_den_automationspfad (M-119): sample_accurate_automation ist unsupported - Topologieautomation wirkt nicht, der Parameter folgt trotzdem");
        hostSchreibt (*mit, iBand (0, param::kGainDb), -9.0f);
        mit->kontrollTaktFuerTest();
        const auto c = fahreAudio (*mit, 50, 128, 11);
        const auto d = fahreAudio (*ohne, 50, 128, 11);
        pruefe (gain (mit->wirksamerZustand(), 0) == -9.0 && ! bitgleich (c, d),
                "  Gegenprobe: kontinuierliche Automation (gain_db) wirkt blockweise");
    }
    {
        auto p = prozessor (48000.0, 480);
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 3.0);
        setze (*p, z);
        const auto r = p->stateRevision();
        const auto e0 = p->automationEpoche();
        for (int i = 0; i < 50; ++i)
        {
            hostSchreibt (*p, iBand (0, param::kGainDb), -6.0f + 0.2f * (float) i);
            p->kontrollTaktFuerTest();
            fahreAudio (*p, 1, 480, 20 + i);
        }
        const auto e1 = p->automationEpoche();
        fahreAudio (*p, 20, 480, 99);                             // 9600 Samples: unter der Ruhegrenze (12000)
        p->kontrollTaktFuerTest();
        const auto e2 = p->automationEpoche();
        fahreAudio (*p, 6, 480, 98);                              // 12480 Samples seit dem letzten Punkt
        p->kontrollTaktFuerTest();
        const auto e3 = p->automationEpoche();
        pruefe (p->stateRevision() == r && p->zustandLesen().undoRing.size() == 1,
                "automation_erzeugt_keine_revision (M-81, Prozessor): 50 Automationspunkte ueber den Hostparameter, keine Revision");
        pruefe (e1 == e0 + 1 && e2 == e1 && e3 == e1 + 1,
                "epoche_wechselt_genau_zweimal_je_geste (M-81, Prozessor): Beginn, dann Ende nach 0,25 s verarbeitetem Audio",
                zahl (e0) + " -> " + zahl (e1) + " -> " + zahl (e2) + " -> " + zahl (e3));
    }
    {
        auto p = prozessor();
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 3.0);
        setze (*p, z);
        const auto r = p->stateRevision();
        // Ein Band-Drag auf der Kurve (S31b): Frequenz und Gain in EINER
        // Bedienung, zwei ueberlappende Parametergesten.
        auto& gainParam = hostParam (*p, iBand (0, param::kGainDb));
        auto& freqParam = hostParam (*p, iBand (0, param::kFreqHz));
        gainParam.beginChangeGesture();
        freqParam.beginChangeGesture();
        for (int i = 0; i < 10; ++i)
        {
            gainParam.setValueNotifyingHost (gainParam.convertTo0to1 (-2.0f - (float) i));
            freqParam.setValueNotifyingHost (freqParam.convertTo0to1 (1100.0f + 50.0f * (float) i));
        }
        gainParam.endChangeGesture();
        const auto nachErsterGeste = p->stateRevision();
        for (int i = 0; i < 5; ++i)
            freqParam.setValueNotifyingHost (freqParam.convertTo0to1 (1600.0f + 20.0f * (float) i));
        freqParam.endChangeGesture();
        const auto ring = p->zustandLesen().undoRing;
        pruefe (nachErsterGeste == r && p->stateRevision() == r + 1 && ring.size() == 2 && ring.back().art == state::UndoArt::gestus
                    && std::abs (gain (p->bestaetigterZustand(), 0) + 11.0) < 1.0e-5
                    && std::abs (freq (p->bestaetigterZustand(), 0) - 1680.0) < 0.05,
                "abgeschlossener_gestus_ist_eine_revision (M-82): zwei ueberlappende Gesten mit 25 Zwischenwerten, eine Revision, die Endwerte bestaetigt",
                "r " + zahl (r) + " -> " + zahl (p->stateRevision()) + ", gain " + std::to_string (gain (p->bestaetigterZustand(), 0))
                + ", freq " + std::to_string (freq (p->bestaetigterZustand(), 0)));
    }
    {
        // Ein nicht-endlicher Hostwert (NaN, +Inf) erreicht nie das Programm.
        auto p = prozessor();
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 3.0);
        setze (*p, z);
        const auto r = p->stateRevision();
        auto& gainParam = hostParam (*p, iBand (0, param::kGainDb));
        auto& freqParam = hostParam (*p, iBand (0, param::kFreqHz));
        gainParam.setValueNotifyingHost (std::numeric_limits<float>::quiet_NaN());
        freqParam.setValueNotifyingHost (std::numeric_limits<float>::infinity());
        p->kontrollTaktFuerTest();
        const auto y = fahreAudio (*p, 40, 512, 61);
        const bool endlich = std::all_of (y.begin(), y.end(), [] (float v) { return std::isfinite (v); });
        const auto w = p->wirksamerZustand();
        pruefe (gain (w, 0) == 3.0 && freq (w, 0) == 1000.0 && p->stateRevision() == r && endlich,
                "nichtendlicher_hostwert_erreicht_das_programm_nie (E4-10): NaN und +Inf ueber den Hostparameter lassen den wirksamen Zustand beim bestaetigten Wert, der Ausgang bleibt endlich",
                "gain " + std::to_string (gain (w, 0)) + ", freq " + std::to_string (freq (w, 0)));
    }
    {
        auto p = prozessor();
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 3.0);
        setze (*p, z);
        juce::MemoryBlock ohne;
        p->getStateInformation (ohne);
        auto vorschau = p->bestaetigterZustand();
        vorschau.werte[(size_t) iBand (0, param::kGainDb)].zahl = -12.0;
        setzeBand (vorschau, 4, 5000.0, 8.0);
        juce::String g;
        const bool gesetzt = p->setzePreview (vorschau, g);
        juce::MemoryBlock mit;
        p->getStateInformation (mit);
        pruefe (gesetzt && p->previewAktiv() && ohne == mit,
                "preview_wird_nie_serialisiert (M-83, Prozessor): getStateInformation waehrend der Vorschau liefert dieselben Bytes", str (g));
    }
    {
        auto a = prozessor (48000.0, 256);
        auto z = mitEq (true);
        setzeBand (z, 0, 800.0, 7.0);
        setzeBand (z, 3, 6000.0, -5.0);
        z.werte[(size_t) param::indexVonId ("v1.global.width")].zahl = 1.3;
        z.werte[(size_t) param::kIndexAutoGain].b = true;
        z.zonen = { { 1, 2000.0, 3000.0, false } };               // klanglos, aber Teil des Hashs
        setze (*a, z);
        juce::MemoryBlock bytes;
        a->getStateInformation (bytes);
        const auto ya = fahreAudio (*a, 300, 256, 42);
        auto b = std::make_unique<Prozessor>();
        b->setStateInformation (bytes.getData(), (int) bytes.getSize());
        b->setRateAndBufferSizeDetails (48000.0, 256);
        b->prepareToPlay (48000.0, 256);
        const auto yb = fahreAudio (*b, 300, 256, 42);
        pruefe (b->stateHashText() == a->stateHashText() && bitgleich (ya, yb),
                "reload_rekonstruiert_denselben_audioausgang (M-84): derselbe Hash und derselbe Ausgang (hier bitgleich)",
                std::to_string (ya.size()) + " Samples");
    }
    {
        auto p = prozessor();
        p->setzeHoermatrix (dsp::Hoermatrix::delta);
        const bool vorher = p->gewuenschteHoermatrix() == dsp::Hoermatrix::delta;
        juce::MemoryBlock bytes;
        p->getStateInformation (bytes);
        p->setStateInformation (bytes.getData(), (int) bytes.getSize());
        tx::DspBericht bericht; juce::String g;
        p->dspBericht (bericht, g);
        const char muster[] = "hoer";
        const auto* anfang = static_cast<const char*> (bytes.getData());
        const bool imState = std::search (anfang, anfang + bytes.getSize(), muster, muster + 4) != anfang + bytes.getSize();
        pruefe (vorher && p->gewuenschteHoermatrix() == dsp::Hoermatrix::processed && ! imState && ! bericht.jcs.containsIgnoreCase ("hoer"),
                "nach_laden_immer_processed (M-52): die Auswahl ist transient und steht weder im State noch im DTO");
    }
    {
        auto a = prozessor (48000.0, 256); auto b = prozessor (48000.0, 256);
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 6.0);
        setze (*a, z); setze (*b, z);
        auto vorschau = z;
        vorschau.werte[(size_t) iBand (0, param::kGainDb)].zahl = -12.0;
        juce::String g;
        const bool gesetzt = a->setzePreview (vorschau, g);
        a->setzeHoermatrix (dsp::Hoermatrix::candidate);
        fahreAudio (*a, 20, 256, 50); fahreAudio (*b, 20, 256, 50);
        a->setNonRealtime (true); b->setNonRealtime (true);
        fahreAudio (*a, 8, 256, 51); fahreAudio (*b, 8, 256, 51);
        const auto ya = fahreAudio (*a, 100, 256, 52);
        const auto yb = fahreAudio (*b, 100, 256, 52);
        pruefe (gesetzt && ! a->previewAktiv() && a->gewuenschteHoermatrix() == dsp::Hoermatrix::processed && bitgleich (ya, yb),
                "offline_render_nutzt_den_bestaetigten_zustand (M-120): der Wechsel in den Offline-Betrieb beendet die Vorschau; der Render ist bitgleich zum Lauf ohne Vorschau",
                str (g));
    }
}

} // namespace

int main()
{
    std::cout << "== Nakama SONDE-015 B7 - Transaktionskern und Prozessorseite ==" << std::endl;
    std::cout << "Gate: Manifest SONDE-015 §5.11.4 (S0-S8, I1-I6, T1-T17), Matrix §3.1, §3.7-§3.11, §3.14." << std::endl;

    // Ebene 1: der Transaktionskern mit dem echten DSP-Kern
    falltabelleVollstaendigGefahren();
    fehlerJeStufe();
    commitPunktAlloziertNicht();
    siebenTransaktionen();
    zehntausend();
    wiederholungUndSitzung();
    belegungRemoveUndo();
    schutzZonen();
    undoRing();
    overlays();
    vertragsweg();
    presets();

    // Ebene 2: derselbe Kern im echten SondeProcessor
    prozessorParameter();
    prozessorZustand();
    prozessorAutomation();

    std::cout << std::endl << geprueft << " geprueft, " << fehler << " Fehler" << std::endl;
    std::cout << (fehler == 0 ? "TRANSAKTION OK" : "TRANSAKTION FEHLGESCHLAGEN") << std::endl;
    return fehler == 0 ? 0 : 1;
}
