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
         Gestus, Save/Load, Host-Dirty und der erste Audioblock. Seit NAK-283
         Etappe 4 dazu der Host-Reset und das Lebenszyklus-Trio (M-30 bis
         M-32) und die Analysezufuehrung der Sonde: Luecke bei verworfenem
         Tap, gemeinsame Zaehler, ruhender Passthrough, float-Kante der
         Analysekopie (M-35 bis M-37, M-41). Die Analysefaelle halten den
         Worker ueber `mitAngehaltenerAnalyseFuerTest` an und lesen die Queue
         selbst - deterministisch, ohne auf Zeit zu warten. Seit NAK-311
         Etappe 3 (W01) der Host-Reset im Ausblenden (311/M-34, Abschnitt R),
         seit W03 Remove und Neubelegung ohne Audio dazwischen (311/M-40,
         Abschnitt S), seit Etappe 4 Teil b (F12, R-311-4) der RECALL
         eingeschwungener dynamischer Baender (Abschnitt T, 311/M-76 bis
         M-79): der Same-Instance-Ladestart bleibt bitgleich warm, eine neu
         geladene oder neu vorbereitete Instanz haelt ab t_E die bezifferte
         Toleranz von M-84 (Manifest NAK-311 §9.1 F-12), und zwei Sekunden
         Audio lassen die Statebytes unveraendert. Kein Produktcode - die
         Huellkurvenleistung bleibt Audiohistorie. Seit NAK-312 Etappe 2
         (R-312-1, Abschnitt W) misst die Echtzeitwache ab dem Eintritt in
         `SondeProcessor::processBlock`, nicht erst ab `DspKern::verarbeite`:
         ein Testplayhead nimmt je Block eine gemeldete Sperre VOR dem Kern
         (312/M-01), ein Workerzug ausserhalb des Blocks zaehlt nicht
         (312/M-02), und legt das Bein den Bereich wie der Wrapper um
         `setNonRealtime` und `processBlock`, zaehlt er je Aufruf des
         Offlineflags seit Etappe 5 keine Sperre mehr (312/M-04; bis Etappe 4
         je Aufruf `setNonRealtime (true)` eine). Sperren, die JUCE vor dem Plugincode
         nimmt, sieht der Zaehler bauartbedingt nicht. Seit NAK-312 Etappe 3a
         (W02, Abschnitt X und 312/M-14 in Abschnitt O) der Ladestart: Hostwerte,
         die vor ihm in der Mailbox lagen, bleiben wirkungslos, ein Hostwert
         danach wirkt und der Regler zeigt ihn, reset() bewahrt die Mailbox,
         und ein read-only geladener Stand bleibt unter Hostautomation
         audio-neutral. Die Taktsperre (`mitAngehaltenemTaktFuerTest`) haelt den
         Worker dabei an; kein Fall haengt an der Wanduhr. Seit NAK-312 Etappe 3b
         (T3-01-05 Teil a, Abschnitt Y und 312/M-24, 312/M-25 in Abschnitt O) die
         Blockbindung: fuer Input-Trim, Output-Trim, Width und Mix ist ein Lauf
         ohne Kontrolltakt bitgleich zu einem mit Takt nach jedem Block, auch
         offline, bei jeder gefahrenen Blockgroesse und ueber den Zaehlerrand;
         Bandwerte und Schalter bleiben messend taktgebunden (Teil b, NAK-340).
         Seit der Nacharbeit 1 der Etappe 3 (L-1, R-312-16, 312/M-87 in
         Abschnitt Y) setzt der Blockrand nach einem read-only-Ladestart aus
         keinem Hostwert ein Rampenziel - auch nicht im Ausblenden des vorigen
         Standes und nicht waehrend eines beim Laden laufenden Uebergangs -,
         und ein schreibbarer Stand danach laesst ihn ohne Kontrolltakt wieder
         wirken. Seit NAK-312 Etappe 4 (Abschnitt Z, 312/M-38) laufen 4000 und
         mehr Bloecke, waehrend beide v3-Clients ueber den geteilten
         Testserver (`V3TestServer.h`) auf einer Probe-Pipe verbunden sind und
         ihre Provider durch die Besitzschleuse der Sonde laufen: die
         Echtzeitwache bleibt dabei bei 0. Seit NAK-312 Etappe 5 (Abschnitt ZA,
         312/M-42 als Umbau von `offline_render_nutzt_den_bestaetigten_zustand`
         in Abschnitt O) der Offline-Uebergang: das Offlineflag nimmt keine
         Sperre, der erste Block danach traegt ab Sample 0 den bestaetigten
         Zustand - bei Blockgroesse 1, 64 und 256, auch in einen laufenden Fade
         oder Hoerhalt hinein -, der Rueckweg blendet weich, und die Vorschau
         verbucht der naechste Kontrolltakt; gefragt wird sie wie die
         Bankfreigabe nur nach einem ausdruecklichen `kontrollTaktFuerTest()`.
         Seit dem zweiten Aenderungssatz der Etappe 5 (Abschnitt ZB, 312/M-51
         in Abschnitt O) die Parameteranbindung ohne APVTS: das
         Parametergolden des Basis-SHA (`parameterGolden`, erzeugt mit
         `--parameter-golden <text> <bin>`) gilt Zeile fuer Zeile und Byte fuer
         Byte, der Sondenordner nennt die APVTS-Klasse nicht mehr, die
         Listener sind paarweise, und unter Hostautomation aus einem zweiten
         Thread bleibt die Echtzeitwache bei 0. Seit der Nacharbeit 1 der
         Etappe 5 (Abschnitt ZC, Kanonteil von 312/M-88, R-312-19) ein
         Textriegel: kein Quelltext der zwei ausgelieferten Buendel ruft einen
         Weg, der den Hostprozess beendet oder sein Fenster schliesst. Seit
         NAK-312 Etappe 6a (Zusaetze aus der Erstpruefung der Etappe 5) in
         Abschnitt ZA: ein im Render gesetzter Hoerwunsch wirkt als Processed
         (312/M-89), Offline an und aus ohne Block dazwischen schaltet den
         ersten Echtzeitblock nicht hart (312/M-90), 312/M-40 zaehlt auch die
         Allokationen, und der Textriegel liest auch die Vorlagen `*.in`.
         Seit NAK-312 Etappe 7b, Satz 2 (T3-01-09, Karten U48 und U58, Weg E1
         mit K-B; Abschnitt ZD) der zweite Eintritt `processBlockBypassed`:
         die Echtzeitwache beginnt auch dort vor der Stempelbildung
         (312/M-101 (a)), ueber 4000 Bloecke mit Eintrittswechseln,
         Hostautomation, Programmwechseln und den drei Hosteintritten bleiben
         Sperren und Allokationen 0 (312/M-101 (b)); getBypassParameter()
         bleibt nullptr bei 112 Hostparametern (312/M-102); der Hostbypass
         laesst Statebytes, Hash, Revision, Undo-Ring und Host-Dirty
         unberuehrt und zaehlt verarbeitete Samples fuer die Ruhegrenze
         weiter (312/M-103); der Blockrand des Bypasseintritts nimmt
         Hostpunkte ueber den Ueberlauf der Ereigniszaehler (312/M-132);
         Echtzeit und Offline sind bitgleich, auch wenn der Wechsel nach
         offline mitten in die Blende faellt (312/M-104); im Hostbypass haengt
         keine Bank (312/M-108). Dazu die Kostenmessung der Stufe.

    LANDMINE NAK-175: Prozessor, DSP-Kern und Transaktionskern liegen in jeder
    Testfunktion auf dem HEAP (`std::unique_ptr`), nie im Rahmen.

    ALLOKATIONSZAEHLER: thread_local und nur um die gemessene Stelle
    eingeschaltet - den Nachschlag S0 und alles hinter dem Commit-Punkt. Seit
    NAK-312 Etappe 2 (E-312-11) meldet dieselbe Stelle zusaetzlich an
    `RtWache::meldeAllokation()`; `RtWache::allokationen()` zaehlt damit jede
    eingeschaltete Allokation, die im Bereich der Echtzeitwache liegt.

    Exit 0 nur bei "TRANSAKTION OK".
*/

#include "NakamaTransaktion.h"
#include "NakamaPreset.h"
#include "NakamaKanon.h"
#include "DspKern.h"
#include "SondeProcessor.h"
#include "IpcVerbindung.h"
#include "PipeToken.h"
#include "WireEnvelope.h"

#include <juce_cryptography/juce_cryptography.h>   // NAK-312 Etappe 5: SHA-256 im Parametergolden

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// NAK-312 Etappe 4 (312/M-38): der geteilte v3-Testserver braucht die
// Windows-Pipes.
#ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>

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

// NAK-312 Etappe 2 (E-312-11): beide Operatoren melden zusaetzlich an die
// Echtzeitwache, wortgleich nach B6 (`DspGoldenTestMain.cpp`). Der eigene
// Zaehler bleibt; `meldeAllokation()` zaehlt ohnehin nur im Audiopfad, also
// allein innerhalb eines `RtWache::Bereich`.
void* operator new (std::size_t groesse)
{
    if (zaehleAllokationen) { ++allokationen; dsp::RtWache::meldeAllokation(); }
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAllokationen) { ++allokationen; dsp::RtWache::meldeAllokation(); }
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

//==============================================================================
// NAK-312 Etappe 5 (312/M-48, 312/M-49; R-312-4): das PARAMETERGOLDEN.
//
// Der Identitaetsbeweis der Parameteranbindung in drei Teilen, als Text:
// (a) Zahl, Reihenfolge, IDs, Bereiche und Defaults der Hostparameter,
// (b) die Hostsicht - `convertTo0to1` an Minimum, Default und Maximum,
// `convertFrom0to1` und `getText` an 0, 0,37 und 1 -, jede Zahl als Bitmuster,
// (c) die Statebytes ueber speichern, laden, speichern und die Epochenfolge
// einer Automationsfahrt. Erzeugt wird es mit
// `EqCopTransactionTest --parameter-golden <text> <bin>` am Basis-SHA der
// Etappe 5 (`12300f1e`) ueber den UNVERAENDERTEN Prozessor (§7.1: nie von
// Hand); verglichen wird am Endstand mit demselben Code.

std::string bitsVon (float f)
{
    std::uint32_t u = 0;
    std::memcpy (&u, &f, sizeof u);
    std::ostringstream o;
    o << std::hex << std::uppercase << std::setw (8) << std::setfill ('0') << u;
    return o.str();
}

std::string sha256Von (const juce::MemoryBlock& m)
{
    return juce::SHA256 (m.getData(), m.getSize()).toHexString().toStdString();
}

/** Teil (a) und (b): eine Zeile je Hostparameter, Felder durch Tabulator. */
std::string parameterGoldenText (Prozessor& p)
{
    std::ostringstream o;
    const auto& liste = p.getParameters();
    o << "anzahl\t" << liste.size() << "\n";
    o << "gruppen\t" << p.getParameterTree().getSubgroups (true).size() << "\n";
    for (int i = 0; i < liste.size(); ++i)
    {
        auto* q = dynamic_cast<juce::RangedAudioParameter*> (liste[i]);
        if (q == nullptr) { o << "p\t" << i << "\tKEIN_RANGED\n"; continue; }
        std::string typ = "unbekannt", werte;
        if (dynamic_cast<juce::AudioParameterBool*> (q) != nullptr)        typ = "bool";
        else if (dynamic_cast<juce::AudioParameterFloat*> (q) != nullptr)  typ = "float";
        else if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (q))  { typ = "choice"; werte = c->choices.joinIntoString (",").toStdString(); }
        const auto& r = q->getNormalisableRange();
        const float def = q->convertFrom0to1 (q->getDefaultValue());
        o << "p\t" << i << "\t" << q->getParameterIndex() << "\t" << str (q->paramID) << "\t" << str (q->getName (1024))
          << "\t" << typ << "\t" << werte << "\t" << q->getVersionHint()
          << "\t" << bitsVon (r.start) << "\t" << bitsVon (r.end) << "\t" << bitsVon (r.interval)
          << "\t" << bitsVon (r.skew) << "\t" << (r.symmetricSkew ? 1 : 0)
          << "\t" << bitsVon (q->getDefaultValue()) << "\t" << bitsVon (def) << "\t" << bitsVon (q->getValue())
          << "\t" << q->getNumSteps() << "\t" << (q->isDiscrete() ? 1 : 0) << "\t" << (q->isBoolean() ? 1 : 0)
          << "\t" << (q->isAutomatable() ? 1 : 0) << "\t" << (q->isMetaParameter() ? 1 : 0)
          << "\t" << (int) q->getCategory() << "\t" << str (q->getLabel())
          << "\t" << bitsVon (q->convertTo0to1 (r.start)) << "\t" << bitsVon (q->convertTo0to1 (def))
          << "\t" << bitsVon (q->convertTo0to1 (r.end));
        for (const float v : { 0.0f, 0.37f, 1.0f })
            o << "\t" << bitsVon (q->convertFrom0to1 (v)) << "\t" << str (q->getText (v, 1024))
              << "\t" << bitsVon (q->getValueForText (q->getText (v, 1024)));
        o << "\n";
    }
    return o.str();
}

/** Teil (c): der Stand und seine Bytes. `s1` ist der gespeicherte Stand mit
    belegten Slots; er wird zusaetzlich als Binaerdatei eingefroren (B2 liest
    ihn). Der Stand beginnt mit Bytes aus dem Schreiber mit fester
    instance_id, damit keine Zufallskennung in die Bytes geraet. */
struct ZustandsGolden
{
    std::string text;
    juce::MemoryBlock s1;
};

ZustandsGolden zustandsGolden()
{
    ZustandsGolden g;
    auto z0 = state::frisch ("312e5a00000000000000000000000005");
    z0.common.klasse = state::Klasse::active_probe;
    z0.hatParameters = true;
    juce::MemoryBlock s0;
    state::speichere (z0, s0);

    const int trim = param::indexVonId ("v1.global.output_trim_db");
    auto p1 = prozessor (48000.0, 480);
    DirtyZaehler dirty;
    p1->addListener (&dirty);
    p1->setStateInformation (s0.getData(), (int) s0.getSize());
    auto z = mitEq (true);
    setzeBand (z, 0, 120.0, 4.5);
    setzeBand (z, 2, 2500.0, -3.0);
    setzeBand (z, 5, 9000.0, 2.0);
    z.werte[(size_t) trim].zahl = -1.5;
    const auto e1 = setze (*p1, z);
    auto z2 = z;
    z2.werte[(size_t) iBand (2, param::kGainDb)].zahl = -4.0;
    const auto e2 = setze (*p1, z2);
    p1->getStateInformation (g.s1);

    // Die Automationsfahrt unter der Taktsperre: jeder Takt ist ausdruecklich.
    std::string epochen;
    p1->mitAngehaltenemTaktFuerTest ([&]
    {
        epochen += zahl (p1->automationEpoche());
        for (int i = 0; i < 20; ++i)
        {
            hostSchreibt (*p1, iBand (0, param::kGainDb), 4.5f - 0.25f * (float) i);
            hostSchreibt (*p1, trim, -1.5f + 0.1f * (float) i);
            fahreAudio (*p1, 1, 480, 700 + i);
            p1->kontrollTaktFuerTest();
            epochen += "," + zahl (p1->automationEpoche());
        }
        for (int i = 0; i < 30; ++i)
        {
            fahreAudio (*p1, 1, 480, 800 + i);
            p1->kontrollTaktFuerTest();
            epochen += "," + zahl (p1->automationEpoche());
        }
    });
    juce::MemoryBlock s1NachFahrt;
    p1->getStateInformation (s1NachFahrt);
    const int dirtyMeldungen = dirty.nichtParameter;
    p1->removeListener (&dirty);

    auto p2 = prozessor (48000.0, 480);
    p2->setStateInformation (g.s1.getData(), (int) g.s1.getSize());
    juce::MemoryBlock s2;
    p2->getStateInformation (s2);

    std::ostringstream o;
    o << "s0\t" << s0.getSize() << "\t" << sha256Von (s0) << "\n";
    o << "s1\t" << g.s1.getSize() << "\t" << sha256Von (g.s1) << "\n";
    o << "s1_nach_fahrt\t" << s1NachFahrt.getSize() << "\t" << sha256Von (s1NachFahrt) << "\n";
    o << "s2\t" << s2.getSize() << "\t" << sha256Von (s2) << "\n";
    o << "commits\t" << tx::wort (e1.ausgang) << "," << tx::wort (e2.ausgang) << "\n";
    o << "revision\t" << zahl (p1->stateRevision()) << "\t" << zahl (p2->stateRevision()) << "\n";
    o << "hash\t" << str (p1->stateHashText()) << "\t" << str (p2->stateHashText()) << "\n";
    o << "dirty\t" << dirtyMeldungen << "\n";
    o << "epochen\t" << epochen << "\n";
    g.text = o.str();
    return g;
}

/** Das ganze Golden: Kopfzeilen, Teil (a) und (b), Teil (c). */
ZustandsGolden parameterGolden()
{
    auto p = prozessor();
    auto g = zustandsGolden();
    g.text = "# NAK-312 Etappe 5 - Parametergolden (312/M-48, 312/M-49; R-312-4)\n"
             "# Erzeugt von EqCopTransactionTest --parameter-golden ueber den Prozessor; nie von Hand.\n"
           + parameterGoldenText (*p) + g.text;
    return g;
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
            "prozessor_fuehrt_112_parameter_in_vertragsreihenfolge (M-88; bis NAK-312 Etappe 5 apvts_fuehrt_..., seither ohne "
            "APVTS direkt am Prozessor): Kennung, Index, Typ, Grenzen und Default aus parameter::tabelle()",
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
        // NAK-312 Etappe 3b (312/M-24, 312/M-25): derselbe Ablauf auf dem
        // BLOCKGEBUNDENEN Weg (Output-Trim, T3-01-05 Teil a). Der Blockrand
        // setzt nur Rampenziele; Overlay, Epoche und Ruhegrenze bleiben beim
        // Worker, und der Stand bleibt ohne Revision und ohne Dirty.
        auto p = prozessor (48000.0, 480);
        auto z = mitEq (true);
        z.werte[(size_t) param::indexVonId ("v1.global.output_trim_db")].zahl = 3.0;
        setze (*p, z);
        DirtyZaehler dirty;
        p->addListener (&dirty);
        juce::MemoryBlock vorher;
        p->getStateInformation (vorher);
        const auto r  = p->stateRevision();
        const auto e0 = p->automationEpoche();
        for (int i = 0; i < 50; ++i)
        {
            hostSchreibt (*p, param::indexVonId ("v1.global.output_trim_db"), -6.0f + 0.2f * (float) i);
            fahreAudio (*p, 1, 480, 40 + i);          // der Blockrand nimmt den Wert
            p->kontrollTaktFuerTest();                // der Worker fuehrt Overlay und Epoche
        }
        const auto e1 = p->automationEpoche();
        fahreAudio (*p, 20, 480, 97);                 // 9600 Samples: unter der Ruhegrenze (12000)
        p->kontrollTaktFuerTest();
        const auto e2 = p->automationEpoche();
        fahreAudio (*p, 6, 480, 96);                  // 12480 Samples seit dem letzten Punkt
        p->kontrollTaktFuerTest();
        const auto e3 = p->automationEpoche();
        juce::MemoryBlock nachher;
        p->getStateInformation (nachher);
        const int dirtyMeldungen = dirty.nichtParameter;
        p->removeListener (&dirty);
        pruefe (e1 == e0 + 1 && e2 == e1 && e3 == e1 + 1,
                "312/M-24 epoche_wechselt_genau_zweimal_auch_blockgebunden (Teilfall von epoche_wechselt_genau_zweimal_je_geste, "
                "[SONDE-015] M-81): 50 Automationspunkte auf Output-Trim, den der Blockrand uebernimmt - die Epoche wechselt "
                "genau zweimal, die Ruhegrenze zaehlt verarbeitete Samples (unter 12000 kein Ende, nach 12480 das Ende)",
                zahl (e0) + " -> " + zahl (e1) + " -> " + zahl (e2) + " -> " + zahl (e3));
        pruefe (p->stateRevision() == r && dirtyMeldungen == 0 && nachher == vorher,
                "312/M-25 blockgebundene_automation_erzeugt_keine_revision (Teilfall von automation_erzeugt_keine_revision, "
                "[SONDE-015] M-81): dieselbe Fahrt erzeugt keine Revision, kein Host-Dirty, und die Statebytes sind bytegleich "
                "zu denen vor der Fahrt",
                "Revision " + zahl (r) + " -> " + zahl (p->stateRevision()) + ", Dirty " + std::to_string (dirtyMeldungen)
                + ", Statebytes " + (nachher == vorher ? "gleich" : "VERSCHIEDEN"));
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

        // NAK-312 Etappe 5 (312/M-51, R-312-4): nach dem Laden traegt jeder
        // Hostparameter den Wert des geladenen Standes in Hostgenauigkeit -
        // der Abgleich zum Host ueberlebt den Umbau der Parameteranbindung.
        // Das Soll rechnet der Test aus der Vertragszelle und der Hostsicht
        // des Parameters selbst, nicht ueber eine Produktfunktion.
        {
            const auto geladen = b->bestaetigterZustand();
            int gleich = 0, abweichendVomDefault = 0;
            std::string ersteAbweichung;
            for (int i = 0; i < param::kHostParameter; ++i)
            {
                auto& q = hostParam (*b, i);
                const auto& zelle = geladen.werte[(size_t) i];
                float denormiert = 0.0f;
                switch (param::tabelle()[(size_t) i].typ)
                {
                    case param::Typ::boolean:     denormiert = zelle.b ? 1.0f : 0.0f; break;
                    case param::Typ::gleitkomma:  denormiert = (float) zelle.zahl; break;
                    case param::Typ::aufzaehlung: denormiert = (float) zelle.enumIndex; break;
                }
                const float soll = q.convertTo0to1 (denormiert);
                if (q.getValue() == soll) ++gleich;
                else if (ersteAbweichung.empty()) ersteAbweichung = str (q.paramID);
                if (soll != q.getDefaultValue()) ++abweichendVomDefault;
            }
            pruefe (gleich == param::kHostParameter && abweichendVomDefault > 0,
                    "  312/M-51 hostparameter_spiegeln_den_geladenen_stand (Teilfall von reload_rekonstruiert_denselben_audioausgang, "
                    "[SONDE-015] M-84, R-312-4): nach setStateInformation in eine frische Instanz traegt jeder der 112 "
                    "Hostparameter den Wert des geladenen Standes in Hostgenauigkeit",
                    std::to_string (gleich) + " von 112 gleich, " + std::to_string (abweichendVomDefault)
                    + " davon nicht auf dem Default" + (ersteAbweichung.empty() ? std::string() : ", erste Abweichung " + ersteAbweichung));
        }

        // NAK-312 Etappe 3 (312/M-14, Hashhaelfte von [SONDE-015] M-84): derselbe
        // Ladestart in DIESELBE Instanz, waehrend ein Hostwert (Output-Trim
        // -9 dB) noch in der Mailbox liegt. Die Taktsperre haelt den Worker an;
        // gemessen wird nach dem Kontrolltakt.
        const auto bestaetigtVorher = a->bestaetigterZustand();
        const auto hashVorher       = a->stateHashText();
        const auto ringVorher       = a->zustandLesen().undoRing;
        const auto cursorVorher     = a->zustandLesen().undoCursor;
        const auto revisionVorher   = a->stateRevision();
        a->mitAngehaltenemTaktFuerTest ([&]
        {
            hostSchreibt (*a, param::indexVonId ("v1.global.output_trim_db"), -9.0f);
            a->setStateInformation (bytes.getData(), (int) bytes.getSize());
            a->kontrollTaktFuerTest();
        });
        pruefe (a->bestaetigterZustand() == bestaetigtVorher && a->stateHashText() == hashVorher
                    && a->zustandLesen().undoRing == ringVorher && a->zustandLesen().undoCursor == cursorVorher
                    && a->stateRevision() == revisionVorher,
                "312/M-14 ladestart_in_dieselbe_instanz_haelt_den_hash (Teilfall von reload_rekonstruiert_denselben_audioausgang, "
                "[SONDE-015] M-84 Hashhaelfte): nach dem Laden derselben Bytes und einem Kontrolltakt sind bestaetigter "
                "Zustand, state_hash, Undo-Ring, Cursor und Revision die der Quelle",
                "Revision " + zahl (revisionVorher) + " -> " + zahl (a->stateRevision()) + ", Ring "
                + std::to_string (ringVorher.size()) + " -> " + std::to_string (a->zustandLesen().undoRing.size()));
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
        // NAK-312 Etappe 5 (312/M-42, E-312-7): KEINE Verwurfsbloecke mehr -
        // der erste Block nach dem Offlineflag traegt ab Sample 0 den
        // bestaetigten Zustand. Die Vorschau selbst verbucht der naechste
        // Kontrolltakt; `previewAktiv()` wird deshalb erst nach einem
        // ausdruecklichen Takt gefragt (312/M-83), nie am Workertakt.
        a->setNonRealtime (true); b->setNonRealtime (true);
        const bool sofortProcessed = a->gewuenschteHoermatrix() == dsp::Hoermatrix::processed;
        const auto ya = fahreAudio (*a, 100, 256, 52);
        const auto yb = fahreAudio (*b, 100, 256, 52);
        float groessteAbweichung = 0.0f;   // ueber die ersten 256 Samples beider Kanaele
        for (size_t i = 0; i < 512 && i < ya.size() && i < yb.size(); ++i)
            groessteAbweichung = std::max (groessteAbweichung, std::abs (ya[i] - yb[i]));
        pruefe (gesetzt && sofortProcessed && bitgleich (ya, yb) && groessteAbweichung == 0.0f,
                "offline_render_nutzt_den_bestaetigten_zustand (M-120; seit NAK-312 Etappe 5 312/M-42): nach dem Wechsel "
                "in den Offline-Betrieb ist der Render ab Sample 0 bitgleich zum Lauf ohne Vorschau - ohne Verwurfsbloecke, "
                "max |ya - yb| ueber die ersten 256 Samples 0",
                "Grund '" + str (g) + "', gewuenscht sofort " + (sofortProcessed ? "processed" : "NICHT processed")
                + ", groesste Abweichung der ersten 256 Samples " + std::to_string (groessteAbweichung));
        a->kontrollTaktFuerTest();
        pruefe (! a->previewAktiv(),
                "  312/M-83 (Teil dieses Falls): nach einem ausdruecklichen kontrollTaktFuerTest() ist die Vorschau verbucht - previewAktiv() false");
    }
}

//==============================================================================
// NAK-283 Etappe 4: Host-Reset und Analysezufuehrung am echten SondeProcessor

using AnalyseQueue = nakama::echtzeit::StampedAudioQueue<nakama::echtzeit::GenStrom>;

/** Der Resonator aus NAK-283 M-30: +12-dB-Bell bei 1 kHz, Q 10, auf Slot 0. */
param::DspSatz resonator()
{
    auto z = mitEq (true);
    setzeBand (z, 0, 1000.0, 12.0);
    z.werte[(size_t) iBand (0, param::kQ)].zahl = 10.0;
    return z;
}

/** Ein Block Stille ueber den echten processBlock, auf Wunsch mit 1,0 im
    LETZTEN Sample beider Kanaele. Liefert den Ausgang beider Kanaele. */
std::vector<float> stilleBlock (Prozessor& p, int groesse, bool impulsAmEnde = false)
{
    juce::AudioBuffer<float> b (2, groesse);
    b.clear();
    if (impulsAmEnde)
    {
        b.setSample (0, groesse - 1, 1.0f);
        b.setSample (1, groesse - 1, 1.0f);
    }
    juce::MidiBuffer midi;
    p.processBlock (b, midi);
    std::vector<float> aus;
    aus.reserve ((size_t) groesse * 2u);
    for (int k = 0; k < 2; ++k)
        for (int n = 0; n < groesse; ++n)
            aus.push_back (b.getSample (k, n));
    return aus;
}

double spitze (const std::vector<float>& v)
{
    double m = 0.0;
    for (float x : v) m = std::max (m, (double) std::abs (x));
    return m;
}

/** Faehrt der Committed-Pfad eine engagierte, nicht hart gebypasste Bank? */
bool bankRechnet (Prozessor& p)
{
    int aktiv = -1, quelle = -1, kandidat = -1, kandidatQuelle = -1;
    p.dspKernFuerTest().gefahreneSlots (aktiv, quelle, kandidat, kandidatQuelle);
    if (aktiv < 0) return false;
    const auto& prog = p.dspKernFuerTest().pool().bank (aktiv).programm;
    return prog.eqEngagiert && ! prog.hardBypass;
}

/** Der Spiegel von `SondeProcessor::committedRuhtImPassthrough` (privat). */
bool committedRuht (Prozessor& p)
{
    int aktiv = -1, quelle = -1, kandidat = -1, kandidatQuelle = -1;
    p.dspKernFuerTest().gefahreneSlots (aktiv, quelle, kandidat, kandidatQuelle);
    const auto passthrough = [&p] (int slot)
    {
        if (slot < 0) return true;
        const auto& prog = p.dspKernFuerTest().pool().bank (slot).programm;
        return ! prog.eqEngagiert || prog.hardBypass;
    };
    return passthrough (aktiv) && passthrough (quelle);
}

/** Der Aufbau aus M-30: Resonator bestaetigt, Engagier-Fade und Rampen vorbei
    (vier Bloecke Stille je 512 Samples; kFadeSamples und kRampeSamples sind
    256), dann ein Impuls am Blockende, der die Biquad-Zustaende fuellt. */
std::unique_ptr<Prozessor> resonatorMitHistorie (tx::Ergebnis& ergebnis)
{
    auto p = prozessor (48000.0, 512);
    ergebnis = setze (*p, resonator());
    for (int i = 0; i < 4; ++i) stilleBlock (*p, 512);
    stilleBlock (*p, 512, true);
    return p;
}

/** Wie stark der Pfad 1 kHz hebt: ein 1-kHz-Sinus ueber acht Bloecke je 512
    Samples, dB des Ausgangs ueber dem Eingang in den letzten vier Bloecken. */
double resonanzGewinnDb (Prozessor& p)
{
    double ein = 0.0, aus = 0.0;
    juce::MidiBuffer midi;
    std::int64_t n = 0;
    for (int blk = 0; blk < 8; ++blk)
    {
        juce::AudioBuffer<float> b (2, 512);
        for (int i = 0; i < 512; ++i, ++n)
        {
            const float v = (float) (0.05 * std::sin (2.0 * 3.141592653589793 * 1000.0 * (double) n / 48000.0));
            b.setSample (0, i, v);
            b.setSample (1, i, v);
        }
        const juce::AudioBuffer<float> eingang (b);
        p.processBlock (b, midi);
        if (blk < 4) continue;
        for (int i = 0; i < 512; ++i)
        {
            ein += (double) eingang.getSample (0, i) * (double) eingang.getSample (0, i);
            aus += (double) b.getSample (0, i) * (double) b.getSample (0, i);
        }
    }
    return ein > 0.0 && aus > 0.0 ? 10.0 * std::log10 (aus / ein) : -400.0;
}

void nak283Hosteintritte()
{
    abschnitt ("P - NAK-283 F05: Host-Reset und Lebenszyklus-Trio am Prozessor (M-30 bis M-32, R-283-3)");
    {
        tx::Ergebnis e, eg;
        auto p = resonatorMitHistorie (e);
        const bool rechnet = bankRechnet (*p);
        p->reset();                                   // VST3: setProcessing (false) ruft NUR reset
        const auto nach = stilleBlock (*p, 512);      // der ERSTE Block danach
        auto g = resonatorMitHistorie (eg);           // Gegenprobe: derselbe Aufbau ohne reset
        const auto ohne = stilleBlock (*g, 512);
        std::ostringstream d;
        d << std::setprecision (9) << "Peak nach reset " << spitze (nach) << ", ohne reset " << spitze (ohne);
        pruefe (e.ausgang == tx::Ausgang::commit && eg.ausgang == tx::Ausgang::commit && rechnet
                    && spitze (nach) == 0.0 && spitze (ohne) > 0.0,
                "host_reset_beendet_die_audiohistorie (M-30): Resonator 1 kHz, +12 dB, Q 10, Impuls am Blockende, reset() - "
                "der erste Block aus Stille ist auf beiden Kanaelen exakt 0; ohne reset klingt derselbe Aufbau nach",
                d.str());
    }
    {
        tx::Ergebnis e;
        auto p = resonatorMitHistorie (e);
        juce::MemoryBlock vorher;
        p->getStateInformation (vorher);
        const auto revision = p->stateRevision();
        p->reset();
        juce::MemoryBlock nachher;
        p->getStateInformation (nachher);
        const bool bestaetigtAn = p->bestaetigterZustand().werte[(size_t) param::kIndexEqEnabled].b;
        const double gewinn = resonanzGewinnDb (*p);
        const bool kernRechnet = bankRechnet (*p);
        std::ostringstream d;
        d << vorher.getSize() << " Bytes, gleich " << (vorher == nachher ? "ja" : "nein") << ", Revision " << revision
          << " -> " << p->stateRevision() << ", eq_enabled " << bestaetigtAn << ", Bank im Kern " << kernRechnet
          << ", 1 kHz " << std::setprecision (4) << gewinn << " dB";
        pruefe (e.ausgang == tx::Ausgang::commit && vorher == nachher && p->stateRevision() == revision
                    && bestaetigtAn && kernRechnet && gewinn > 6.0,
                "host_reset_laesst_parameter_und_zustand_unberuehrt (M-31, Regressionswache): getStateInformation bytegleich "
                "vor und nach reset(), die Bank bleibt engagiert (bestaetigt und im Kern), und der naechste Block mit "
                "Material traegt die Filterwirkung (1 kHz mehr als 6 dB lauter)",
                d.str());
    }
    {
        const auto weg = [] (int art)
        {
            tx::Ergebnis e;
            auto p = resonatorMitHistorie (e);
            if (art == 0)
            {
                p->releaseResources();
                p->setRateAndBufferSizeDetails (48000.0, 512);
                p->prepareToPlay (48000.0, 512);
            }
            else if (art == 1)
            {
                p->reset();
            }
            else
            {
                p->setRateAndBufferSizeDetails (48000.0, 512);
                p->prepareToPlay (48000.0, 512);
            }
            return stilleBlock (*p, 512);
        };
        const auto a = weg (0), b = weg (1), c = weg (2);
        std::ostringstream d;
        d << std::setprecision (9) << "Peaks " << spitze (a) << " / " << spitze (b) << " / " << spitze (c)
          << ", a=b " << bitgleich (a, b) << ", b=c " << bitgleich (b, c);
        pruefe (spitze (a) == 0.0 && spitze (b) == 0.0 && spitze (c) == 0.0 && bitgleich (a, b) && bitgleich (b, c),
                "die_drei_hosteintritte_enden_in_derselben_audiohistorie (M-32): releaseResources->prepareToPlay, reset "
                "und prepareToPlay allein - nach demselben Impuls je ein Block Stille, Peak exakt 0, untereinander bitgleich",
                d.str());
    }
}

/** Liest alle wartenden Deskriptoren als Kopie und gibt sie frei - nur unter
    angehaltener Analyse (`mitAngehaltenerAnalyseFuerTest`). */
std::vector<nakama::echtzeit::StampedBlock> leereQueue (AnalyseQueue& q)
{
    std::vector<nakama::echtzeit::StampedBlock> aus;
    while (const auto* b = q.spitze())
    {
        aus.push_back (*b);
        q.freigeben();
    }
    return aus;
}

/** Ein Block Rauschen in ±0,1 ueber den echten processBlock. */
void rauschBlock (Prozessor& p, int groesse, juce::Random& w)
{
    juce::AudioBuffer<float> b (2, groesse);
    for (int k = 0; k < 2; ++k)
        for (int n = 0; n < groesse; ++n)
            b.setSample (k, n, w.nextFloat() * 0.2f - 0.1f);
    juce::MidiBuffer midi;
    p.processBlock (b, midi);
}

struct Queuezaehler
{
    std::uint64_t kernVerworfen = 0, ohneAudio = 0, frames = 0, oversize = 0, ueberlauf = 0;
};

Queuezaehler queuezaehler (Prozessor& p)
{
    Queuezaehler z;
    z.kernVerworfen = p.dspKernFuerTest().verworfeneAnalyseframes();
    z.ohneAudio     = p.analyseDropsOhneAudioFuerTest();
    z.frames        = p.analyseVerloreneFramesFuerTest();
    z.oversize      = p.analyseDropsOversizeFuerTest();
    z.ueberlauf     = p.analyseDropsUeberlaufFuerTest();
    return z;
}

std::string beschreibeBlock (const nakama::echtzeit::StampedBlock& b)
{
    return "stromVon " + zahl (b.stromVon) + " n " + std::to_string (b.sampleCount) + " Luecke "
         + ((b.flags & nakama::echtzeit::kFlagLueckeDavor) != 0 ? "ja" : "nein") + " Segment " + zahl (b.segment);
}

/** Alle gelesenen Deskriptoren in Reihenfolge - auch wenn ihre Zahl nicht die
    erwartete ist, damit ein roter Lauf zeigt, was die Queue wirklich trug. */
std::string beschreibeBloecke (const std::vector<nakama::echtzeit::StampedBlock>& bloecke)
{
    std::string text;
    for (size_t i = 0; i < bloecke.size(); ++i)
        text += std::string (i == 0 ? "" : "; ") + "#" + std::to_string (i + 1) + " " + beschreibeBlock (bloecke[i]);
    return text;
}

void nak283Analysezufuehrung()
{
    namespace rt = nakama::echtzeit;
    abschnitt ("Q - NAK-283 F09 und F12: Analysezufuehrung der Sonde (M-35 bis M-37, M-41)");
    {
        // M-35 und M-36: 64, 128 und 64 Samples bei maxBlock 64 und rechnendem Kern.
        auto p = prozessor (48000.0, 64);
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 6.0);
        const auto e = setze (*p, z);
        std::vector<rt::StampedBlock> bloecke;
        Queuezaehler vor, nach;
        bool rechnet = false;
        p->mitAngehaltenerAnalyseFuerTest ([&] (AnalyseQueue& q)
        {
            leereQueue (q);                         // nichts Fremdes vor dem gemessenen Zug
            juce::Random w (35);
            rauschBlock (*p, 64, w);                // Block 1: angenommen
            vor = queuezaehler (*p);
            rauschBlock (*p, 128, w);               // Block 2: groesser als maxBlock, der Kern stueckelt
            rechnet = ! committedRuht (*p);
            nach = queuezaehler (*p);
            rauschBlock (*p, 64, w);                // Block 3
            bloecke = leereQueue (q);
        });
        const bool zwei = bloecke.size() == 2;
        const rt::StampedBlock b1 = zwei ? bloecke[0] : rt::StampedBlock {};
        const rt::StampedBlock b3 = zwei ? bloecke[1] : rt::StampedBlock {};
        pruefe (e.ausgang == tx::Ausgang::commit && rechnet && zwei && b1.stromVon == 0 && b1.sampleCount == 64
                    && b3.stromVon == 192 && b3.sampleCount == 64 && (b3.flags & rt::kFlagLueckeDavor) != 0
                    && b3.segment == b1.segment + 1,
                "ungueltiger_tap_bei_rechnendem_kern_hinterlaesst_eine_luecke (M-35): 64, 128 (> maxBlock 64) und 64 Samples "
                "bei rechnendem Kern - der dritte Block beginnt lokal bei 192, traegt kFlagLueckeDavor, und "
                "continuity_segment steigt um 1",
                std::string ("Kern rechnet: ") + (rechnet ? "ja" : "nein") + "; " + std::to_string (bloecke.size())
                + " Deskriptoren: " + beschreibeBloecke (bloecke));
        pruefe (nach.kernVerworfen == vor.kernVerworfen + 1 && nach.ohneAudio == vor.ohneAudio + 1
                    && nach.frames == vor.frames + 128 && nach.oversize == vor.oversize && nach.ueberlauf == vor.ueberlauf,
                "dsp_und_queuezaehler_beschreiben_dieselbe_verworfene_zeitspanne (M-36): derselbe Block zaehlt im Kern als "
                "ein verworfener Tap und in der Queue als genau 128 verlorene Frames eines Blocks ohne Audio - nicht als oversizeDrops",
                "Kern +" + zahl (nach.kernVerworfen - vor.kernVerworfen) + ", ohne Audio +" + zahl (nach.ohneAudio - vor.ohneAudio)
                + ", Frames +" + zahl (nach.frames - vor.frames) + ", oversize +" + zahl (nach.oversize - vor.oversize)
                + ", Ueberlauf +" + zahl (nach.ueberlauf - vor.ueberlauf));
    }
    {
        // M-37: derselbe Zug, aber der Committed-Pfad ruht im Hard-Bypass.
        auto p = prozessor (48000.0, 64);
        auto z = mitEq (true);
        z.werte[(size_t) param::indexVonId ("v1.global.bypass")].b = true;
        setzeBand (z, 0, 1000.0, 6.0);              // hoerbar dahinter, ruht im Hard-Bypass
        const auto e = setze (*p, z);
        std::vector<rt::StampedBlock> bloecke;
        Queuezaehler vor, nach;
        bool ruht = false;
        p->mitAngehaltenerAnalyseFuerTest ([&] (AnalyseQueue& q)
        {
            leereQueue (q);
            juce::Random w (37);
            rauschBlock (*p, 64, w);
            vor = queuezaehler (*p);
            rauschBlock (*p, 128, w);
            ruht = committedRuht (*p);
            nach = queuezaehler (*p);
            rauschBlock (*p, 64, w);
            bloecke = leereQueue (q);
        });
        const bool drei = bloecke.size() == 3;
        const rt::StampedBlock b1 = drei ? bloecke[0] : rt::StampedBlock {};
        const rt::StampedBlock b2 = drei ? bloecke[1] : rt::StampedBlock {};
        const rt::StampedBlock b3 = drei ? bloecke[2] : rt::StampedBlock {};
        pruefe (e.ausgang == tx::Ausgang::commit && ruht && drei && nach.kernVerworfen == vor.kernVerworfen + 1
                    && nach.ohneAudio == vor.ohneAudio && b2.stromVon == 64 && b2.sampleCount == 128
                    && (b2.flags & rt::kFlagLueckeDavor) == 0 && b2.segment == b1.segment
                    && b3.stromVon == 192 && (b3.flags & rt::kFlagLueckeDavor) == 0 && b3.segment == b1.segment,
                "ruhender_passthrough_erzeugt_keine_luecke (M-37, Regressionswache): im Hard-Bypass liest die Analyse den "
                "uebergrossen Block als unberuehrten Eingang - er wird angenommen, und keine Luecke entsteht",
                std::string ("Committed ruht: ") + (ruht ? "ja" : "nein") + "; " + std::to_string (bloecke.size())
                + " Deskriptoren: " + beschreibeBloecke (bloecke) + "; Kern +" + zahl (nach.kernVerworfen - vor.kernVerworfen)
                + ", ohne Audio +" + zahl (nach.ohneAudio - vor.ohneAudio));
    }
    {
        // M-41: Output-Trim +6 dB, Eingang 0,75 x FLT_MAX, Hoermatrix Dry - der
        // Ausgang ist der rohe Eingang, der Tap post_committed liegt ueber FLT_MAX.
        auto p = prozessor (48000.0, 512);
        auto z = mitEq (true);
        z.werte[(size_t) param::indexVonId ("v1.global.output_trim_db")].zahl = 6.0;
        const auto e = setze (*p, z);
        p->setzeHoermatrix (dsp::Hoermatrix::dry);
        for (int i = 0; i < 4; ++i) stilleBlock (*p, 512);   // Engagier-Fade, Rampe und Hoermatrix-Fade vorbei
        const auto zaehlerVorher = p->dspKernFuerTest().nichtEndlicheEingaenge();
        const float gross = 0.75f * std::numeric_limits<float>::max();
        std::vector<float> analyse;
        bool gelesen = false, tapUeberRand = false, ausgangRoh = true;
        p->mitAngehaltenerAnalyseFuerTest ([&] (AnalyseQueue& q)
        {
            leereQueue (q);
            juce::AudioBuffer<float> b (2, 512);
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < 512; ++n)
                    b.setSample (k, n, gross);
            juce::MidiBuffer midi;
            p->processBlock (b, midi);
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < 512; ++n)
                    if (b.getSample (k, n) != gross) ausgangRoh = false;
            if (const double* t = p->dspKernFuerTest().tap (dsp::Tap::postCommitted, 0))
                tapUeberRand = std::isfinite (t[0]) && std::abs (t[0]) > (double) std::numeric_limits<float>::max();
            if (const auto* blk = q.spitze())
            {
                analyse.resize ((size_t) blk->sampleCount * 2u);
                gelesen = blk->sampleCount == 512 && q.lies (*blk, 0, analyse.data());
                q.freigeben();
            }
        });
        const bool analyseEndlich = gelesen
            && std::all_of (analyse.begin(), analyse.end(), [] (float v) { return std::isfinite (v); });
        const auto delta = p->dspKernFuerTest().nichtEndlicheEingaenge() - zaehlerVorher;
        pruefe (e.ausgang == tx::Ausgang::commit && gelesen && tapUeberRand && ausgangRoh && analyseEndlich && delta == 1024,
                "analysekopie_verengt_nicht_unbemerkt (M-41, R-283-6): der Tap post_committed traegt endliche double ueber "
                "FLT_MAX, die Analyse bekommt 1024 endliche floats, und der Zaehler des Kerns sieht genau die 1024 "
                "verriegelten Werte",
                std::string ("gelesen ") + (gelesen ? "ja" : "nein") + ", Tap ueber FLT_MAX " + (tapUeberRand ? "ja" : "nein")
                + ", Ausgang roh " + (ausgangRoh ? "ja" : "nein") + ", Analyse endlich " + (analyseEndlich ? "ja" : "nein")
                + ", Zaehler +" + zahl (delta));
    }
}

/** Ein Block DC ueber den echten processBlock; liefert den Ausgang beider
    Kanaele. */
std::vector<float> dcBlock (Prozessor& p, int groesse, float wert)
{
    juce::AudioBuffer<float> b (2, groesse);
    for (int k = 0; k < 2; ++k)
        for (int n = 0; n < groesse; ++n)
            b.setSample (k, n, wert);
    juce::MidiBuffer midi;
    p.processBlock (b, midi);
    std::vector<float> aus;
    aus.reserve ((size_t) groesse * 2u);
    for (int k = 0; k < 2; ++k)
        for (int n = 0; n < groesse; ++n)
            aus.push_back (b.getSample (k, n));
    return aus;
}

void nak311Pfadrampen()
{
    abschnitt ("R - NAK-311 W01: Host-Reset im Ausblenden friert kein Rampenziel ein (311/M-34)");
    // Manifest NAK-311 §6.2 M-34: DC 0,5, Blockgroesse 64. Eq an mit
    // Output-Trim +24 dB und eingeschwungen; eq aus und genau EIN Block (das
    // Ausblenden laeuft, Rest 192); reset() - der Uebergang endet in
    // beendeAudiohistorie, der Pfad ruht -; in der Ruhe Output-Trim 0 dB (die
    // ENDE-Marke aendert keine Rampe) und Audio; dann eq an mit 0 dB. Ein
    // ruhender Pfad steht nach reset() auf den Ruhewerten 1,0: das
    // Wiedereinschalten mit 0 dB traegt Ausgang/Eingang 1,0.
    auto p = prozessor (48000.0, 64);
    const int iTrim = param::indexVonId ("v1.global.output_trim_db");
    auto an = mitEq (true);
    an.werte[(size_t) iTrim].zahl = 24.0;
    const auto e1 = setze (*p, an);
    for (int i = 0; i < 32; ++i) dcBlock (*p, 64, 0.5f);          // Engagier-Fade und Rampe vorbei

    auto aus = an;
    aus.werte[(size_t) param::kIndexEqEnabled].b = false;
    const auto e2 = setze (*p, aus);
    dcBlock (*p, 64, 0.5f);                                        // das Ausblenden laeuft, Rest 192
    int cA = -1, cQ = -1, kA = -1, kQ = -1;
    p->dspKernFuerTest().gefahreneSlots (cA, cQ, kA, kQ);
    const bool ausblendend = cA == -1 && cQ >= 0;

    p->reset();
    p->dspKernFuerTest().gefahreneSlots (cA, cQ, kA, kQ);
    const bool ruht = cA == -1 && cQ == -1;

    auto ausNull = aus;
    ausNull.werte[(size_t) iTrim].zahl = 0.0;
    const auto e3 = setze (*p, ausNull);                           // ENDE-Marke in der Ruhe
    for (int i = 0; i < 8; ++i) dcBlock (*p, 64, 0.5f);

    const auto e4 = setze (*p, mitEq (true));                      // eq an mit 0 dB
    double spitze = 0.0;
    for (int i = 0; i < 16; ++i)
        for (float v : dcBlock (*p, 64, 0.5f))
            spitze = std::max (spitze, std::abs ((double) v) / 0.5);

    const bool commits = e1.ausgang == tx::Ausgang::commit && e2.ausgang == tx::Ausgang::commit
                      && e3.ausgang == tx::Ausgang::commit && e4.ausgang == tx::Ausgang::commit;
    std::ostringstream d;
    d << std::setprecision (9) << "vier Commits " << (commits ? "ja" : "nein") << ", vor reset ausblendend "
      << (ausblendend ? "ja" : "nein") << ", nach reset ruhend " << (ruht ? "ja" : "nein")
      << ", Spitze Ausgang/Eingang ueber 1024 Samples des Wiedereinschaltens " << spitze;
    pruefe (commits && ausblendend && ruht && spitze <= 1.0 + 1e-6,
            "311/M-34 reset_im_ausblenden_friert_kein_ziel_ein (NAK-311 T3-15-05): Output-Trim +24 dB, eq aus, ein Block, "
            "reset(), in der Ruhe 0 dB, eq an mit 0 dB - Ausgang/Eingang an jedem Sample hoechstens 1 + 1e-6",
            d.str());
}

void nak311SlotLebenszyklus()
{
    abschnitt ("S - NAK-311 W03: Remove und Neubelegung ohne Audio dazwischen starten kalt (311/M-40)");
    // Manifest NAK-311 §6.3 M-40 (T3-14-02, SONDE-015 M-121): auf dem
    // Pruefstand ein dynamisches Bell 1 kHz Q 2, Range -12 dB, Threshold
    // -40 dB, Attack 0,1 ms, Hold 500 ms, Release 5000 ms in Slot 0, mit einem
    // Quadraturton 0,5 eingeschwungen (Detektor -9 dB: Plateau -12 dB). Dann
    // `remove` fuer Slot 0 und `bandBelegen` mit derselben Konfiguration, beide
    // committet, kein Audioblock dazwischen - die zweite Publikation verdraengt
    // die erste, der Audiothread sieht das Remove nie. Danach nur Stille in
    // Bloecken zu 64. Die neue Belegung beginnt trotzdem kalt: Auslenkung ab dem
    // ersten Block exakt 0,0, der Uebergang ist ein Crossfade, ab Sample
    // kFadeSamples ist der Ausgang exakt 0,0.
    Stand st; Sitzung s;
    const double fs = 48000.0;
    auto z = mitEq (true);
    const auto zelle = [&z] (int feld) -> param::Zelle& { return z.werte[(size_t) iBand (0, feld)]; };
    z.werte[(size_t) param::indexOccupied (0)].b = true;
    zelle (param::kEnabled).b                = true;
    zelle (param::kType).enumIndex           = (int) dsp::Filtertyp::bell;
    zelle (param::kFreqHz).zahl              = 1000.0;
    zelle (param::kQ).zahl                   = 2.0;
    zelle (param::kGainDb).zahl              = 0.0;
    zelle (param::kDynamicEnabled).b         = true;
    zelle (param::kDynamicRangeDb).zahl      = -12.0;
    zelle (param::kThresholdDb).zahl         = -40.0;
    zelle (param::kAttackMs).zahl            = 0.1;
    zelle (param::kHoldMs).zahl              = 500.0;
    zelle (param::kReleaseMs).zahl           = 5000.0;
    zelle (param::kSidechainSource).enumIndex = (int) dsp::Sidechain::internal;
    const auto e1 = fahre (st, s, apply (*st.tk, z, 1));
    wachen (*st.tk, s, "311/M-40 Apply");

    std::vector<float> l (512), r (512);
    float* kan[2] = { l.data(), r.data() };
    const double w = 2.0 * 3.14159265358979323846 * 1000.0 / fs;
    long long n = 0;
    for (int blk = 0; blk < 94; ++blk)
    {
        for (int i = 0; i < 512; ++i)
        {
            l[(size_t) i] = (float) (0.5 * std::sin (w * (double) (n + i)));
            r[(size_t) i] = (float) (0.5 * std::cos (w * (double) (n + i)));
        }
        st.kern->verarbeite (kan, 2, 512);
        n += 512;
    }
    double vorher[param::kSlots];
    st.kern->auslenkungenDb (vorher);
    st.kern->pflege();

    const std::uint64_t uebernahmenVor = st.kern->uebernahmen();
    const auto e2 = fahre (st, s, remove (*st.tk, 0, 2));
    wachen (*st.tk, s, "311/M-40 Remove");
    auto belegen = auftrag (*st.tk, tx::Art::bandBelegen, 3);
    for (int f = 0; f < param::kJeSlot; ++f) belegen.band[(size_t) f] = z.werte[(size_t) iBand (0, f)];
    const auto e3 = fahre (st, s, belegen);
    wachen (*st.tk, s, "311/M-40 Belegen");

    bool kreuzblende = false;
    double slot0Max = 0.0, spitzeNachFade = 0.0;
    for (int blk = 0; blk < 32; ++blk)
    {
        std::fill (l.begin(), l.end(), 0.0f);
        std::fill (r.begin(), r.end(), 0.0f);
        st.kern->verarbeite (kan, 2, 64);
        if (blk == 0)
        {
            int klingend = -1, quelle = -1;
            st.kern->rechnendeSlots (dsp::Pfad::committed, klingend, quelle);
            kreuzblende = quelle >= 0;
        }
        double werte[param::kSlots];
        st.kern->auslenkungenDb (werte);
        if (std::abs (werte[0]) > std::abs (slot0Max)) slot0Max = werte[0];   // betragsgroesster Wert, mit Vorzeichen
        for (int i = 0; i < 64; ++i)
            if (blk * 64 + i >= dsp::kFadeSamples)
                spitzeNachFade = std::max (spitzeNachFade, (double) std::max (std::abs (l[(size_t) i]), std::abs (r[(size_t) i])));
    }
    const std::uint64_t uebernahmen = st.kern->uebernahmen() - uebernahmenVor;

    const bool commits = e1.ausgang == tx::Ausgang::commit && e2.ausgang == tx::Ausgang::commit
                      && e3.ausgang == tx::Ausgang::commit && e3.slot == 0;
    std::ostringstream d;
    d << std::setprecision (9) << "drei Commits (Belegen auf Slot 0) " << (commits ? "ja" : "nein")
      << ", eingeschwungen " << vorher[0] << " dB, Uebernahmen danach +" << uebernahmen
      << ", Uebergang ist ein Crossfade " << (kreuzblende ? "ja" : "nein")
      << ", betragsgroesste Meldung von Slot 0 ab dem ersten Block " << slot0Max << " dB, Spitze bei Stille ab Sample "
      << dsp::kFadeSamples << ": " << spitzeNachFade;
    pruefe (commits && vorher[0] == -12.0 && uebernahmen == 1 && kreuzblende && slot0Max == 0.0 && spitzeNachFade == 0.0,
            "311/M-40 remove_und_belegen_ohne_audio_startet_kalt (NAK-311 T3-14-02, SONDE-015 M-121): Remove und "
            "Neubelegung ohne Audioblock dazwischen, danach Stille - Auslenkung 0,0, Crossfade, ab kFadeSamples exakt 0,0",
            d.str());
}

//==============================================================================
// NAK-311 Etappe 4 Teil b (F12, R-311-4): der Recall eingeschwungener
// dynamischer Baender. KEIN Produktcode - die Huellkurvenleistung bleibt
// Audiohistorie. Gemessen werden die Zahl der Toleranz von M-84 und ihr
// Geltungsbereich (Manifest NAK-311 §9.1 F-12, §6.4 M-76 bis M-80).

constexpr double kPiRecall = 3.14159265358979323846;

/** Ein Lauf des Quadraturtons durch den ECHTEN Prozessor: L = sin, R = cos
    auf der Bandmitte (E-25). Bei einem eingeschwungenen Quadraturton ist der
    Betrag sqrt(L^2 + R^2) je Sample konstant gleich Amplitude mal
    angewandter Verstaerkung - er misst die Verstaerkung ohne Schwebung ueber
    eine Periode und damit sample-genau. */
struct QuadraturLauf
{
    std::vector<float>  ausgang;   ///< L und R je Sample, verschraenkt
    std::vector<double> tap;       ///< `post_committed`, Kanal 0
    std::vector<double> betrag;    ///< sqrt(L^2 + R^2) des Ausgangs
};

QuadraturLauf fahreQuadratur (Prozessor& p, double fs, double f0, double amp,
                              long long& n0, int samples, int groesse, bool aufzeichnen)
{
    QuadraturLauf lauf;
    const int bloecke = (samples + groesse - 1) / groesse;
    if (aufzeichnen)
    {
        lauf.ausgang.reserve ((size_t) bloecke * (size_t) groesse * 2);
        lauf.tap.reserve ((size_t) bloecke * (size_t) groesse);
        lauf.betrag.reserve ((size_t) bloecke * (size_t) groesse);
    }
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> puffer (2, groesse);
    const double w = 2.0 * kPiRecall * f0 / fs;
    for (int b = 0; b < bloecke; ++b)
    {
        for (int i = 0; i < groesse; ++i)
        {
            puffer.setSample (0, i, (float) (amp * std::sin (w * (double) (n0 + i))));
            puffer.setSample (1, i, (float) (amp * std::cos (w * (double) (n0 + i))));
        }
        p.processBlock (puffer, midi);
        if (aufzeichnen)
        {
            const double* t = p.dspKernFuerTest().tap (dsp::Tap::postCommitted, 0);
            for (int i = 0; i < groesse; ++i)
            {
                const double l = (double) puffer.getSample (0, i);
                const double r = (double) puffer.getSample (1, i);
                lauf.ausgang.push_back ((float) l);
                lauf.ausgang.push_back ((float) r);
                lauf.tap.push_back (t != nullptr ? t[i] : 0.0);
                lauf.betrag.push_back (std::sqrt (l * l + r * r));
            }
        }
        n0 += groesse;
    }
    return lauf;
}

/** Ein dynamisches Bell in Slot 0 - Typ `bell`, Kanalmodus `stereo`,
    Sidechain `internal`, also genau der Teilraum aus §9 F-12. */
param::DspSatz nak311DynBand (double f0, double q, double g0, double rangeDb,
                              double thresholdDb, double attackMs,
                              double holdMs = 0.0, double releaseMs = 100.0)
{
    auto z = mitEq (true);
    const auto zelle = [&z] (int feld) -> param::Zelle& { return z.werte[(size_t) iBand (0, feld)]; };
    z.werte[(size_t) param::indexOccupied (0)].b = true;
    zelle (param::kEnabled).b                 = true;
    zelle (param::kType).enumIndex            = (int) dsp::Filtertyp::bell;
    zelle (param::kFreqHz).zahl               = f0;
    zelle (param::kQ).zahl                    = q;
    zelle (param::kGainDb).zahl               = g0;
    zelle (param::kChannelMode).enumIndex     = (int) dsp::Kanalmodus::stereo;
    zelle (param::kDynamicEnabled).b          = true;
    zelle (param::kDynamicRangeDb).zahl       = rangeDb;
    zelle (param::kThresholdDb).zahl          = thresholdDb;
    zelle (param::kAttackMs).zahl             = attackMs;
    zelle (param::kHoldMs).zahl               = holdMs;
    zelle (param::kReleaseMs).zahl            = releaseMs;
    zelle (param::kSidechainSource).enumIndex = (int) dsp::Sidechain::internal;
    return z;
}

/** t_E aus §9 F-12, HIER eigenstaendig ausgeschrieben (Muster M-13: der Test
    ruft keine Produktfunktion als Orakel).

    NAK-311 W35 (R-311-15, 311/M-127, §40.4), datierter Nachtrag zu §9.1 F-12
    vom 20.09.2026: seit dem Aenderungssatz D liegt der PEGELBEGRIFF mit der
    festen Zeitkonstante `kPegelFensterMs` vor der Huellkurve. Die
    Zeitkonstanten zweier Pole in Reihe addieren sich als obere Schranke, der
    Huellkurventerm wird deshalb

        5*(tau_a + tau_m)*fs   statt   5*tau_a*fs,

    also

        t_E = max (kFadeSamples,
                   5*(tau_a + tau_m)*fs + 10*Q*A_max*fs/(pi*f0) + 16)

    mit tau_m = `kPegelFensterMs`. Der Zuwachs ist hoechstens
    5*`kPegelFensterMs`*fs Samples (bei 48 kHz 2400) und damit genau die
    Schranke, die §40.4 nennt. A_max = 10^(max(0, g0, g0 + Range)/40); die
    drei Summanden sind der Huellkurventerm (fuenf Zeitkonstanten der Reihe
    aus Pegelfenster und Attack), der Bandterm (zehn genaeherte
    Zeitkonstanten von Detektor und Band) und die Steuerrate (bis zu 8
    Samples bis zum naechsten Entwurf, 8 Ueberblendung; M-73). Die TOLERANZ
    von 0,1 dB ab t_E bleibt unveraendert - nur der Referenzbeginn wandert. */
double nak311TeE (double fs, double f0, double q, double g0, double rangeDb, double attackMs)
{
    const double aMax = std::pow (10.0, std::max (0.0, std::max (g0, g0 + rangeDb)) / 40.0);
    const double tau  = (attackMs + dsp::kPegelFensterMs) * 0.001;
    return std::max ((double) dsp::kFadeSamples,
                     5.0 * tau * fs + 10.0 * q * aMax * fs / (kPiRecall * f0) + 16.0);
}

/** Der Vorlauf, der Instanz A EINGESCHWUNGEN macht: zehn Zeitkonstanten der
    Reihe aus Pegelfenster und Attack (Rest e^-10 = 4,5e-5, also 0,0004 dB)
    plus das Vierfache des Bandterms plus Reserve. NAK-311 W35: der
    Pegelbegriff ist die dritte Stufe und geht mit `kPegelFensterMs` ein. */
int nak311Vorlauf (double fs, double f0, double q, double attackMs)
{
    return (int) std::ceil (10.0 * ((attackMs + dsp::kPegelFensterMs) * 0.001) * fs)
         + 4 * (int) std::ceil (10.0 * q * fs / (kPiRecall * f0)) + 8000;
}

/** Ein Pruefling aus M-77: Bandwerte, Attack und der Name seines Terms. */
struct RecallFall
{
    const char* pruefling;
    double f0, q, g0, range, attack;
};

/** Faehrt EINEN Vergleich und liefert die groesste Abweichung ab t_E und
    davor. `neueInstanz` waehlt M-77 (Instanz B aus den Bytes von A) oder
    M-78 (`prepareToPlay` auf A selbst, Vergleich gegen C). */
struct RecallMass
{
    double tE = 0.0, maxNachTe = 0.0, maxVorTe = 0.0;
    int    argNachTe = -1;
    bool   aufgebaut = false;
};

RecallMass fahreRecall (const RecallFall& f, double thresholdDb, bool neueInstanz)
{
    const double fs = 48000.0, amp = 0.5;
    const int    blk = 256;
    RecallMass   m;
    m.tE = nak311TeE (fs, f.f0, f.q, f.g0, f.range, f.attack);

    const int tEn     = (int) std::ceil (m.tE);
    const int vorlauf = nak311Vorlauf (fs, f.f0, f.q, f.attack);
    const int mess    = tEn + 4800;

    const auto z = nak311DynBand (f.f0, f.q, f.g0, f.range, thresholdDb, f.attack);

    auto a = prozessor (fs, blk);
    const auto ea = setze (*a, z);
    std::unique_ptr<Prozessor> c;
    tx::Ausgang ec = tx::Ausgang::commit;
    if (! neueInstanz)
    {
        c = prozessor (fs, blk);
        ec = setze (*c, z).ausgang;
    }

    long long nA = 0, nC = 0;
    fahreQuadratur (*a, fs, f.f0, amp, nA, vorlauf, blk, false);
    if (c != nullptr) fahreQuadratur (*c, fs, f.f0, amp, nC, vorlauf, blk, false);

    // t = 0: das erste Sample des ersten Blocks der geladenen (M-77)
    // beziehungsweise neu vorbereiteten (M-78) Instanz. Der Vergleichskern
    // laeuft ununterbrochen weiter und bekommt ab hier denselben Eingang.
    std::unique_ptr<Prozessor> b;
    Prozessor* neu = nullptr;
    Prozessor* ref = nullptr;
    long long nNeu = nA, nRef = nA;
    if (neueInstanz)
    {
        juce::MemoryBlock bytes;
        a->getStateInformation (bytes);
        b = std::make_unique<Prozessor>();
        b->setStateInformation (bytes.getData(), (int) bytes.getSize());
        b->setRateAndBufferSizeDetails (fs, blk);
        b->prepareToPlay (fs, blk);
        neu = b.get();
        ref = a.get();
    }
    else
    {
        a->prepareToPlay (fs, blk);
        neu = a.get();
        ref = c.get();
        nRef = nC;
    }

    const auto lNeu = fahreQuadratur (*neu, fs, f.f0, amp, nNeu, mess, blk, true);
    const auto lRef = fahreQuadratur (*ref, fs, f.f0, amp, nRef, mess, blk, true);

    m.aufgebaut = ea.ausgang == tx::Ausgang::commit && ec == tx::Ausgang::commit
               && lNeu.betrag.size() == lRef.betrag.size() && ! lNeu.betrag.empty();
    if (! m.aufgebaut) return m;

    for (size_t i = 0; i < lNeu.betrag.size(); ++i)
    {
        if (! (lRef.betrag[i] > 0.0)) continue;
        const double ab = std::abs (20.0 * std::log10 (lNeu.betrag[i] / lRef.betrag[i]));
        if ((int) i >= tEn)
        {
            if (ab > m.maxNachTe) { m.maxNachTe = ab; m.argNachTe = (int) i; }
        }
        else if (ab > m.maxVorTe)
        {
            m.maxVorTe = ab;
        }
    }
    return m;
}

void nak311Recall()
{
    abschnitt ("T - NAK-311 F12/R-311-4: Recall eingeschwungener dynamischer Baender (311/M-76 bis 311/M-79)");

    const double fs = 48000.0, amp = 0.5;
    const int    blk = 256;

    // ── 311/M-76: der Same-Instance-Ladestart bleibt WARM ─────────────────
    // Der Pruefling der Phase 16. `setStateInformation` mit den eigenen
    // Bytes: gleiche Belegung, gleiche Topologie, also gleiche Pfad- und
    // Lebenszykluskennungen (W03, §28.2) - der Zustand wandert, es gibt
    // keinen Crossfade und keinen kalten Start.
    {
        const auto z = nak311DynBand (1000.0, 2.0, 0.0, -12.0, -15.0, 500.0);
        auto a = prozessor (fs, blk);
        auto c = prozessor (fs, blk);
        const auto ea = setze (*a, z);
        const auto ec = setze (*c, z);
        long long nA = 0, nC = 0;
        fahreQuadratur (*a, fs, 1000.0, amp, nA, 240000, blk, false);
        fahreQuadratur (*c, fs, 1000.0, amp, nC, 240000, blk, false);

        juce::MemoryBlock bytes;
        a->getStateInformation (bytes);
        a->setStateInformation (bytes.getData(), (int) bytes.getSize());

        const auto la = fahreQuadratur (*a, fs, 1000.0, amp, nA, 48000, blk, true);
        const auto lc = fahreQuadratur (*c, fs, 1000.0, amp, nC, 48000, blk, true);
        const bool ausgangGleich = bitgleich (la.ausgang, lc.ausgang);
        const bool tapGleich = la.tap.size() == lc.tap.size()
            && std::memcmp (la.tap.data(), lc.tap.data(), la.tap.size() * sizeof (double)) == 0;

        std::ostringstream d;
        d << std::setprecision (9) << "Commits " << (ea.ausgang == tx::Ausgang::commit
                                                     && ec.ausgang == tx::Ausgang::commit ? "ja" : "nein")
          << ", " << la.ausgang.size() / 2 << " Samples nach dem Ladestart, Ausgang bitgleich "
          << (ausgangGleich ? "ja" : "nein") << ", Tap bitgleich " << (tapGleich ? "ja" : "nein");
        pruefe (ea.ausgang == tx::Ausgang::commit && ec.ausgang == tx::Ausgang::commit
                    && ausgangGleich && tapGleich,
                "311/M-76 gleicher_ladestart_bleibt_warm (NAK-311 F12, M-84): dynamisches Bell 1 kHz Q 2, "
                "Range -12 dB, Threshold -15 dB, Attack 500 ms, eingeschwungen - `setStateInformation` mit "
                "den EIGENEN Bytes laesst Ausgang und Tap bitgleich zum ununterbrochenen Vergleichskern",
                d.str());
    }

    // ── 311/M-77 und 311/M-78: die Toleranz von M-84 mit ihrer Zahl ───────
    //
    // Geltungsbereich (§9 F-12): dynamisches Bell, Kanalmodus `stereo`,
    // Sidechain `internal`, Quadraturton auf der Bandmitte, Q >= 1,
    // Q*A_min >= 0,5 mit A_min = 10^(min(0, g0, g0 + Range)/40), f0 von
    // 20 Hz bis min(20 kHz, fs/4). Zusage: ab t_E hoechstens 0,1 dB, davor
    // im Betrag hoechstens |g0| + |Range| + 0,1 dB.
    //
    // Drei Prueflinge, je einer fuer einen Term der Formel: Pruefling 1 der
    // Huellkurventerm (drei Attacks), Pruefling 2 der Crossfadeterm (Q 1,0,
    // Q*A_min = 0,501 - dort ueberwiegt `kFadeSamples`), Pruefling 3 der
    // Frequenzterm (f0 = fs/4, Q 24).
    {
        const RecallFall faelle[] = {
            { "1 (Bell 1 kHz Q 2, Huellkurventerm)",        1000.0,  2.0, 0.0, -12.0,   0.1 },
            { "1 (Bell 1 kHz Q 2, Huellkurventerm)",        1000.0,  2.0, 0.0, -12.0,  10.0 },
            { "1 (Bell 1 kHz Q 2, Huellkurventerm)",        1000.0,  2.0, 0.0, -12.0, 500.0 },
            { "2 (Bell 1 kHz Q 1, Crossfadeterm)",          1000.0,  1.0, 0.0, -12.0,   0.1 },
            { "3 (Bell 12 kHz = fs/4 Q 24, Frequenzterm)", 12000.0, 24.0, 0.0, -12.0,   0.1 },
        };
        const double thresholds[] = { -15.0, -40.0 };   // Knie, Plateau
        const char*  lage[]       = { "Knie", "Plateau" };

        for (int teil = 0; teil < 2; ++teil)             // 0 = M-77, 1 = M-78
        {
            const bool neueInstanz = teil == 0;
            for (int gruppe = 0; gruppe < 3; ++gruppe)   // die drei Prueflinge
            {
                bool ok = true;
                std::ostringstream d;
                d << std::setprecision (6) << std::fixed;
                bool erstes = true;
                for (const auto& f : faelle)
                {
                    const int nummer = f.pruefling[0] - '0';
                    if (nummer != gruppe + 1) continue;
                    for (int t = 0; t < 2; ++t)
                    {
                        const auto m = fahreRecall (f, thresholds[t], neueInstanz);
                        const double schrankeVor = std::abs (f.g0) + std::abs (f.range) + 0.1;
                        const bool traegt = m.aufgebaut && m.maxNachTe <= 0.1 && m.maxVorTe <= schrankeVor;
                        ok = ok && traegt;
                        d << (erstes ? "" : "; ") << "Attack " << f.attack << " ms, " << lage[t]
                          << ": t_E " << m.tE << " Samples, ab t_E hoechstens " << m.maxNachTe
                          << " dB (bei Sample " << m.argNachTe << "), davor hoechstens " << m.maxVorTe
                          << " dB gegen " << schrankeVor << (traegt ? "" : " TRAEGT NICHT");
                        erstes = false;
                    }
                }
                const std::string zeile = neueInstanz ? "311/M-77" : "311/M-78";
                const std::string name  = neueInstanz
                    ? "recall_in_neue_instanz_innerhalb_der_toleranz_pruefling_"
                    : "recall_nach_preparetoplay_innerhalb_der_toleranz_pruefling_";
                pruefe (ok, zeile + " " + name + std::to_string (gruppe + 1)
                            + " (NAK-311 R-311-4, M-84, F-12): "
                            + (neueInstanz
                               ? "eine NEUE Instanz aus den Bytes von A"
                               : "`prepareToPlay` auf A selbst gegen die ununterbrochene Instanz C")
                            + " - der Quadraturbetrag weicht ab t_E hoechstens 0,1 dB ab, davor im Betrag "
                              "hoechstens |g0| + |Range| + 0,1 dB",
                        d.str());
            }
        }
    }

    // ── 311/M-79: Audiohistorie ist kein State ────────────────────────────
    // Zwei Sekunden eingeschwungenes Audio aendern kein Byte des States:
    // weder Huellkurven- noch Filterhistorie reisen mit, und es entsteht
    // keine neue Stateversion.
    {
        // Threshold -40 dB: der Ton liegt im PLATEAU, die Auslenkung ist dort
        // die volle Range - die Huellkurve hat sich also nachweislich bewegt,
        // bevor die Bytes zum zweiten Mal gelesen werden.
        const auto z = nak311DynBand (1000.0, 2.0, 0.0, -12.0, -40.0, 500.0);
        auto a = prozessor (fs, blk);
        const auto ea = setze (*a, z);
        juce::MemoryBlock vorher;
        a->getStateInformation (vorher);
        long long n = 0;
        fahreQuadratur (*a, fs, 1000.0, amp, n, 96000, blk, false);
        juce::MemoryBlock nachher;
        a->getStateInformation (nachher);
        double werte[param::kSlots];
        a->dspKernFuerTest().auslenkungenDb (werte);

        // NAK-311 W35 (R-311-15, 311/M-126): der PEGELBEGRIFF und sein
        // Zustand sind abgeleitet wie `kanaele` - Laufzeit, kein Statefeld.
        // Drahtfeld und Revision misst diese Zeile nicht; den Vertrag prueft
        // B3c. Der Pegelzustand hat sich in den zwei Sekunden nachweislich
        // bewegt - endlich und ueber 0 -, und die Statebytes sind trotzdem
        // bytegleich.
        int cA = -1, cQ = -1, kA = -1, kQ = -1;
        a->dspKernFuerTest().gefahreneSlots (cA, cQ, kA, kQ);
        const double pegel = cA >= 0
            ? a->dspKernFuerTest().pool().bank (cA).baender[0].pegel.leistung : -1.0;
        const auto begriff = cA >= 0
            ? a->dspKernFuerTest().pool().bank (cA).programm.baender[0].pegelbegriff
            : dsp::Pegelbegriff::spitze;
        const bool pegelBewegt = pegel > 0.0 && std::isfinite (pegel)
                              && begriff == dsp::Pegelbegriff::durchschnitt;

        std::ostringstream d;
        d << std::setprecision (9) << "Bytes vorher " << vorher.getSize() << ", nachher "
          << nachher.getSize() << ", gleich " << (vorher == nachher ? "ja" : "nein")
          << ", Auslenkung nach 2 s " << werte[0] << " dB, Revision " << a->stateRevision()
          << ", Pegelbegriff " << (begriff == dsp::Pegelbegriff::durchschnitt ? "durchschnitt" : "spitze")
          << ", Pegelzustand " << pegel << " (bewegt: " << (pegelBewegt ? "ja" : "nein") << ")";
        pruefe (ea.ausgang == tx::Ausgang::commit && vorher == nachher && werte[0] == -12.0
                    && pegelBewegt,
                "311/M-79 audiohistorie_ist_kein_state (NAK-311 F12, R-311-4, 311/M-126): zwei "
                "Sekunden eingeschwungenes Audio bewegen Pegelbegriff und Huellkurve auf die volle "
                "Range und lassen die Statebytes UNVERAENDERT",
                d.str());
    }
}

//==============================================================================
// NAK-311 Etappe 5, Aenderungssatz A - F08 (Karte U47), R-311-16 und R-311-20
//
// Unter 44 100 Hz ist der EQ nicht unterstuetzt: `bereiteVor` bereitet nichts
// vor, `verarbeiteStueck` kehrt vor dem ersten Sample zurueck (der Weg des
// AUSGESCHALTETEN EQ), der Grund steht als Zahl am Kern und im Bericht, und
// der Wunsch `eq_enabled` bleibt unberuehrt im Zustand und in den Statebytes.
// Gemessen werden hier die Paarbeziehungen: setzen <-> zuruecknehmen ueber
// 48 -> 32 -> 48 kHz (311/M-135), Kern UND Bericht im dritten Fenster
// (311/M-143, R-311-20) und Latenz, Tail und Callback (311/M-139).

/** Der Pruefling von 311/M-143: EIN High-Cut 20 Hz Q 0,707 mit Auto-Gain an -
    ein Programm, dessen abgeleiteter Ausgleich WEIT von 0 liegt. Gemessen im
    Satz A: +15,688900344 dB (die +42,99 dB der Quellvalidierung Teil B §2.1
    gelten fuer ACHT solcher High-Cuts; berichtigt vom Dirigenten, §52). Der
    Wert liegt UNTER `kAutoGainDeckelDb` = 24,0: der Deckel des Satzes B
    beruehrt diese Zeile nicht, ihre Zahlen bleiben bitgleich. */
param::DspSatz nak311HighCutMitAutoGain()
{
    auto z = mitEq (true);
    setzeBand (z, 0, 20.0, 0.0);
    z.werte[(size_t) iBand (0, param::kType)].enumIndex = (int) dsp::Filtertyp::highCut;
    z.werte[(size_t) iBand (0, param::kQ)].zahl = 0.70710678118654752;
    z.werte[(size_t) param::kIndexAutoGain].b = true;
    return z;
}

/** Faehrt `bloecke` Bloecke festen Rauschens und meldet, ob JEDER Block
    bytegleich herauskam - die Messung des neutralen Wegs. */
bool nak311BleibtBytegleich (Prozessor& p, int bloecke, int groesse, int saat)
{
    bool bytegleich = true;
    juce::Random w (saat);
    juce::MidiBuffer midi;
    for (int b = 0; b < bloecke; ++b)
    {
        juce::AudioBuffer<float> puffer (2, groesse), kopie (2, groesse);
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < groesse; ++n)
                puffer.setSample (k, n, w.nextFloat() * 1.6f - 0.8f);
        kopie.makeCopyOf (puffer);
        p.processBlock (puffer, midi);
        for (int k = 0; k < 2; ++k)
            if (std::memcmp (puffer.getReadPointer (k), kopie.getReadPointer (k),
                             (size_t) groesse * sizeof (float)) != 0)
                bytegleich = false;
    }
    return bytegleich;
}

void nak311Ratenschranke()
{
    abschnitt ("U - NAK-311 F08/R-311-16: unter 44,1 kHz bleibt der EQ neutral (311/M-135, 311/M-139, 311/M-143)");

    const double fs = 48000.0, fsNiedrig = 32000.0;
    const int    blk = 512;

    // ── 311/M-135: setzen <-> zuruecknehmen, speichern <-> laden ──────────
    {
        auto z = mitEq (true);
        setzeBand (z, 0, 14000.0, 12.0);
        z.werte[(size_t) iBand (0, param::kQ)].zahl = 4.0;
        z.werte[(size_t) param::kIndexAutoGain].b = true;

        auto p   = prozessor (fs, blk);
        auto ref = prozessor (fs, blk);   // ein Kern, der NIE abgelehnt hat
        const auto e  = setze (*p, z);
        const auto er = setze (*ref, z);

        DirtyZaehler dirty;
        p->addListener (&dirty);
        fahreAudio (*p, 20, blk, 135);
        fahreAudio (*ref, 20, blk, 135);

        juce::MemoryBlock bytesVor;
        p->getStateInformation (bytesVor);
        const auto revisionVor = p->stateRevision();
        const int  dirtyVor    = dirty.nichtParameter;
        const bool eqVor       = p->bestaetigterZustand().werte[(size_t) param::kIndexEqEnabled].b;

        // --- das dritte Fenster: 32 kHz ---------------------------------
        p->setRateAndBufferSizeDetails (fsNiedrig, blk);
        p->prepareToPlay (fsNiedrig, blk);
        const double abgelehnt = p->dspKernFuerTest().abgelehnteSamplerateHz();
        const bool   neutral   = nak311BleibtBytegleich (*p, 20, blk, 1351);
        const bool   latenzNull = p->getLatencySamples() == 0 && p->getTailLengthSeconds() == 0.0;

        // --- und zurueck auf 48 kHz -------------------------------------
        p->setRateAndBufferSizeDetails (fs, blk);
        p->prepareToPlay (fs, blk);
        ref->setRateAndBufferSizeDetails (fs, blk);
        ref->prepareToPlay (fs, blk);   // dieselbe frische Vorbereitung
        const double zurueck = p->dspKernFuerTest().abgelehnteSamplerateHz();
        const auto nachP   = fahreAudio (*p, 40, blk, 136);
        const auto nachRef = fahreAudio (*ref, 40, blk, 136);
        const bool hoerbar = p->dspKernFuerTest().samplerate() == fs
                          && p->dspKernFuerTest().autoGainDb() != 0.0;

        juce::MemoryBlock bytesNach;
        p->getStateInformation (bytesNach);
        p->removeListener (&dirty);
        const bool eqNach = p->bestaetigterZustand().werte[(size_t) param::kIndexEqEnabled].b;

        std::ostringstream d;
        d << std::setprecision (12) << "abgelehnt im Fenster " << abgelehnt << " Hz, danach " << zurueck
          << ", Ausgang im Fenster bytegleich " << (neutral ? "ja" : "nein")
          << ", nach dem Rueckweg bitgleich zum nie abgelehnten Kern "
          << (bitgleich (nachP, nachRef) ? "ja" : "nein") << ", autoGainDb "
          << p->dspKernFuerTest().autoGainDb() << " dB, Statebytes " << bytesVor.getSize() << " gleich "
          << (bytesVor == bytesNach ? "ja" : "nein") << ", Revision " << revisionVor << " -> "
          << p->stateRevision() << ", Host-Dirty ueber die drei Aufrufe "
          << (dirty.nichtParameter - dirtyVor) << ", eq_enabled " << (eqVor && eqNach ? "bleibt an" : "GEAENDERT");
        pruefe (e.ausgang == tx::Ausgang::commit && er.ausgang == tx::Ausgang::commit
                    && neutral && latenzNull && abgelehnt == fsNiedrig
                    && zurueck == 0.0 && ! std::signbit (zurueck) && hoerbar
                    && bitgleich (nachP, nachRef)
                    && eqVor && eqNach && bytesVor == bytesNach
                    && p->stateRevision() == revisionVor && dirty.nichtParameter == dirtyVor,
                "311/M-135 ratenwechsel_nimmt_den_zustand_zurueck (NAK-311 R-311-16, F08): 48 -> 32 -> 48 kHz - "
                "im 32-kHz-Fenster ist der Ausgang bytegleich zum Eingang und der Getter traegt 32 000, nach dem "
                "dritten prepareToPlay ist er wieder exakt +0,0, das Band ist hoerbar und der Ausgang bitgleich "
                "zu einem Kern, der nie abgelehnt hat; eq_enabled, Statebytes, Revision und Host-Dirty bleiben",
                d.str());
    }

    // ── 311/M-143 (R-311-20): Kern UND Bericht im dritten Fenster ─────────
    {
        const auto z = nak311HighCutMitAutoGain();
        auto p = prozessor (fs, blk);
        const auto e = setze (*p, z);
        fahreAudio (*p, 188, blk, 143);          // rund 2 s bei 48 kHz

        tx::DspBericht vor; juce::String g;
        const bool berichtVor = p->dspBericht (vor, g);
        const double kernVor = p->dspKernFuerTest().autoGainDb();
        const auto uebernahmenVor = p->dspKernFuerTest().uebernahmen();

        // --- 32 kHz: beide nullen, beide melden den Grund ----------------
        p->setRateAndBufferSizeDetails (fsNiedrig, blk);
        p->prepareToPlay (fsNiedrig, blk);
        const bool neutral = nak311BleibtBytegleich (*p, 20, blk, 1431);
        auto& kern = p->dspKernFuerTest();
        const double kernImFenster = kern.autoGainDb();
        const double kernAbgelehnt = kern.abgelehnteSamplerateHz();
        const int    kernKanaele   = kern.busKanaele();
        const auto   uebernahmenImFenster = kern.uebernahmen();
        tx::DspBericht imFenster;
        const bool berichtImFenster = p->dspBericht (imFenster, g);

        // --- zurueck auf 48 kHz ------------------------------------------
        p->setRateAndBufferSizeDetails (fs, blk);
        p->prepareToPlay (fs, blk);
        fahreAudio (*p, 188, blk, 143);
        tx::DspBericht nach;
        const bool berichtNach = p->dspBericht (nach, g);
        const double kernNach = p->dspKernFuerTest().autoGainDb();

        const auto bitGleich = [] (double a, double b)
        { return std::memcmp (&a, &b, sizeof (double)) == 0; };

        std::ostringstream d;
        d << std::setprecision (12) << "vor dem Wechsel Kern " << kernVor << " dB, Bericht " << vor.autoGainDb
          << " dB (unter dem Deckel 24,0, also ungedeckelt), Deckelzustand "
          << (vor.autoGainGedeckelt ? "GESETZT" : "falsch") << "; im Fenster Kern " << kernImFenster
          << " dB (Vorzeichenbit " << (std::signbit (kernImFenster) ? 1 : 0) << "), Bericht "
          << imFenster.autoGainDb << " dB, klemmungen " << imFenster.klemmungen.size()
          << ", abgelehnt Kern " << kernAbgelehnt << " / Bericht " << imFenster.abgelehnteSamplerateHz
          << ", busKanaele " << kernKanaele << ", Uebernahmen " << uebernahmenVor << " -> "
          << uebernahmenImFenster << ", Ausgang bytegleich " << (neutral ? "ja" : "nein")
          << "; danach Kern " << kernNach << " dB, Bericht " << nach.autoGainDb << " dB, abgelehnt "
          << p->dspKernFuerTest().abgelehnteSamplerateHz() << " / " << nach.abgelehnteSamplerateHz;
        pruefe (e.ausgang == tx::Ausgang::commit && berichtVor && berichtImFenster && berichtNach
                    && kernVor != 0.0 && vor.autoGainDb != 0.0 && uebernahmenVor > 0
                    && neutral
                    && kernImFenster == 0.0 && ! std::signbit (kernImFenster)
                    && imFenster.autoGainDb == 0.0 && ! std::signbit (imFenster.autoGainDb)
                    && imFenster.klemmungen.empty()
                    && kernAbgelehnt == fsNiedrig && imFenster.abgelehnteSamplerateHz == fsNiedrig
                    && kernKanaele == 2 && uebernahmenImFenster == 0
                    && imFenster.jcs == vor.jcs && imFenster.revision == vor.revision
                    && imFenster.undoTiefe == vor.undoTiefe
                    && bitGleich (kernNach, kernVor) && bitGleich (nach.autoGainDb, vor.autoGainDb)
                    && p->dspKernFuerTest().abgelehnteSamplerateHz() == 0.0
                    && nach.abgelehnteSamplerateHz == 0.0
                    // NAK-311 Satz B: der Pruefling liegt unter dem Deckel -
                    // diese Zeile bleibt von R-311-14 unberuehrt.
                    && kernVor < dsp::kAutoGainDeckelDb
                    && bitGleich (kernVor, p->dspKernFuerTest().autoGainRohDb())
                    && ! vor.autoGainGedeckelt && ! nach.autoGainGedeckelt
                    && ! imFenster.autoGainGedeckelt,
                "311/M-143 ratenwechsel_nullt_den_ausgleich_und_meldet_beides (NAK-311 R-311-20): High-Cut 20 Hz "
                "Q 0,707 mit Auto-Gain an, 48 -> 32 -> 48 kHz - im 32-kHz-Fenster sind Kern und Bericht exakt "
                "+0,0, klemmungen leer, die Zaehler des alten Fensters genullt, die Kanalzahl auf 2, und BEIDE "
                "tragen die abgelehnte Rate; jcs, Revision und Undo-Tiefe bleiben die des bestaetigten Zustands, "
                "und nach dem Rueckweg tragen beide wieder bitgleich denselben abgeleiteten Wert",
                d.str());
    }

    // ── 311/M-139 (Teilfall): Latenz, Tail und der Callback ───────────────
    {
        auto z = mitEq (true);
        setzeBand (z, 0, 14000.0, 12.0);
        auto p = prozessor (fsNiedrig, blk);
        const auto e = setze (*p, z);

        dsp::RtWache::zuruecksetzen();
        juce::AudioBuffer<float> puffer (2, blk);
        juce::MidiBuffer midi;
        juce::Random w (139);
        std::uint64_t imCallback = 0;
        for (int b = 0; b < 200; ++b)
        {
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < blk; ++n)
                    puffer.setSample (k, n, w.nextFloat() * 1.6f - 0.8f);
            allokationen = 0;
            zaehleAllokationen = true;
            p->processBlock (puffer, midi);
            zaehleAllokationen = false;
            imCallback += allokationen;
        }
        const auto sperren = dsp::RtWache::sperren();
        std::ostringstream d;
        d << "Allokationen im Callback " << imCallback << ", gemeldete Sperren im Audiopfad des Kerns "
          << sperren << ", Latenz " << p->getLatencySamples() << ", Tail " << p->getTailLengthSeconds()
          << " s, Taplaenge " << p->dspKernFuerTest().tapLaenge() << ", verworfene Analyseframes "
          << p->dspKernFuerTest().verworfeneAnalyseframes();
        pruefe (e.ausgang == tx::Ausgang::commit && imCallback == 0 && sperren == 0
                    && p->getLatencySamples() == 0 && p->getTailLengthSeconds() == 0.0
                    && p->dspKernFuerTest().verworfeneAnalyseframes() == 0,
                "311/M-139 der_neutrale_weg_kostet_nichts (Teilfall von 311/M-135, NAK-311 R-311-16): bei 32 kHz "
                "zaehlt der thread-lokale Zaehler ueber 200 Bloecke 0 Allokationen im Callback, die Echtzeitwache "
                "des Kerns 0 gemeldete Sperren, getLatencySamples bleibt 0 und getTailLengthSeconds 0,0",
                d.str());
    }
}

//==============================================================================
// NAK-311 Etappe 5, Aenderungssatz B - R-311-14 (T3-15-09 Teil b, Karte U54)
//
// Der Teilfall von 311/M-117 am ECHTEN Prozessor: `SondeProcessor::dspBericht`
// ist der einzige Produktaufrufer von `baueBericht`. Was er meldet, ist die
// Zahl, die auch der Kern faehrt - der Deckel sitzt vor beiden, in
// `leiteAutoGainAb`. Dazu der Gegenfall 311/M-118 (Auto-Gain aus).

/** Acht High-Cuts 20 Hz Q 0,15 - der Pruefling der Abnahme U54, abgeleitet
    +150,46 dB und damit weit ueber `kAutoGainDeckelDb`. */
param::DspSatz nak311AchtHighCutsQ015 (bool autoGainAn)
{
    auto z = mitEq (true);
    for (int slot = 0; slot < param::kSlots; ++slot)
    {
        setzeBand (z, slot, 20.0, 0.0);
        z.werte[(size_t) iBand (slot, param::kType)].enumIndex = (int) dsp::Filtertyp::highCut;
        z.werte[(size_t) iBand (slot, param::kQ)].zahl = 0.15;
    }
    z.werte[(size_t) param::kIndexAutoGain].b = autoGainAn;
    return z;
}

void nak311AutoGainDeckel()
{
    abschnitt ("V - NAK-311 U54/R-311-14: die Obergrenze des angewandten AUTO-Ausgleichs (311/M-117, 311/M-118)");

    const double fs = 48000.0;
    const int    blk = 512;

    for (const bool autoGainAn : { true, false })
    {
        auto p = prozessor (fs, blk);
        const auto e = setze (*p, nak311AchtHighCutsQ015 (autoGainAn));
        fahreAudio (*p, 20, blk, 117);

        tx::DspBericht b; juce::String g;
        const bool gebaut = p->dspBericht (b, g);
        auto& kern = p->dspKernFuerTest();
        const double kernDb  = kern.autoGainDb();
        const double kernRoh = kern.autoGainRohDb();

        const auto bitGleich = [] (double x, double y)
        { return std::memcmp (&x, &y, sizeof (double)) == 0; };

        std::ostringstream d;
        d << std::setprecision (15) << "Auto-Gain " << (autoGainAn ? "an" : "aus") << ": abgeleitet "
          << kernRoh << " dB, Kern angewandt " << kernDb << " dB, Bericht " << b.autoGainDb
          << " dB, Deckel " << dsp::kAutoGainDeckelDb << " dB; Zustand Kern "
          << (kern.autoGainGedeckelt() ? "gesetzt" : "falsch") << " / Bericht "
          << (b.autoGainGedeckelt ? "gesetzt" : "falsch") << ", klemmungen " << b.klemmungen.size();

        pruefe (e.ausgang == tx::Ausgang::commit && gebaut
                    && kernRoh > 150.0 && kernRoh < 151.0
                    && bitGleich (kernDb, dsp::kAutoGainDeckelDb)
                    && bitGleich (b.autoGainDb, dsp::kAutoGainDeckelDb)
                    && kern.autoGainGedeckelt() == autoGainAn
                    && b.autoGainGedeckelt == autoGainAn
                    && b.klemmungen.empty(),
                std::string (autoGainAn
                    ? "311/M-117 bericht_und_kern_melden_denselben_gedeckelten_wert (Teilfall ueber "
                      "SondeProcessor::dspBericht, NAK-311 R-311-14, U54): acht High-Cuts 20 Hz Q 0,15 - der "
                      "abgeleitete Ausgleich +150,46 dB bleibt lesbar, angewandt und gemeldet wird bitgleich "
                      "kAutoGainDeckelDb, und beide tragen den Zustand"
                    : "311/M-118 auto_gain_aus_deckelt_ohne_zu_melden (Teilfall ueber "
                      "SondeProcessor::dspBericht, NAK-311 F-24, M-35): derselbe Pruefling mit "
                      "v2.global.auto_gain AUS - der Wert bleibt gerechnet und lesbar, angewandt wird nichts, "
                      "und der Zustand ist in Kern und Bericht falsch"),
                d.str());
    }
}

//==============================================================================
// NAK-312 Etappe 2 (R-312-1, E-312-11): die Echtzeitwache ab dem
// Callback-Eintritt der Sonde.
//
// Der Testplayhead sitzt an der einzigen Produktstelle zwischen dem Eintritt in
// `SondeProcessor::processBlock` und `dspKern->verarbeite`, an der Testcode im
// echten Pfad laeuft (`getPlayHead()->getPosition()`). Er nimmt je Aufruf eine
// gemeldete Sperre ueber einen eigenen `std::mutex` und macht den Bereich
// damit lebendig messbar, ohne Produktcode zu verbiegen. Liegt der Bereich erst
// im Kern, laeuft der Playhead davor, und der Zaehler bleibt 0.

struct Testplayhead final : juce::AudioPlayHead
{
    mutable std::mutex schloss;
    mutable int        aufrufe = 0;

    juce::Optional<PositionInfo> getPosition() const override
    {
        const dsp::RtWache::GemeldeteSperre<std::mutex> sperre (schloss);
        ++aufrufe;
        PositionInfo p;
        p.setIsPlaying (true);
        p.setTimeInSamples ((juce::int64) aufrufe * 256);
        return p;
    }
};

void nak312Messgeraete()
{
    abschnitt ("W - NAK-312 Etappe 2: die Echtzeitwache ab dem Callback-Eintritt der Sonde (312/M-01, 312/M-02, 312/M-04)");

    const double fs      = 48000.0;
    const int    blk     = 256;
    const int    bloecke = 200;

    // ── 312/M-01 und 312/M-02: Testplayhead gesetzt ────────────────────────
    {
        auto kopf = std::make_unique<Testplayhead>();   // ueberlebt den Prozessor
        auto p    = prozessor (fs, blk);
        p->setPlayHead (kopf.get());

        juce::AudioBuffer<float> puffer (2, blk);
        juce::MidiBuffer midi;
        juce::Random w (312);
        const auto fuelle = [&]
        {
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < blk; ++n)
                    puffer.setSample (k, n, w.nextFloat() * 1.6f - 0.8f);
        };

        dsp::RtWache::zuruecksetzen();
        std::uint64_t eigene = 0;
        for (int b = 0; b < bloecke; ++b)
        {
            fuelle();
            allokationen = 0;
            zaehleAllokationen = true;
            p->processBlock (puffer, midi);
            zaehleAllokationen = false;
            eigene += allokationen;
        }
        const auto sperren     = dsp::RtWache::sperren();
        const auto rtAllok     = dsp::RtWache::allokationen();
        const int  kopfAufrufe = kopf->aufrufe;
        {
            std::ostringstream d;
            d << "gemeldete Sperren " << sperren << " bei " << kopfAufrufe << " Playhead-Aufrufen in "
              << bloecke << " Bloecken, RtWache::allokationen " << rtAllok << ", eigener Zaehler " << eigene;
            pruefe (sperren == (std::uint64_t) bloecke && kopfAufrufe == bloecke && rtAllok == 0 && eigene == 0,
                    "312/M-01 wache_beginnt_am_callback_eintritt (R-312-1, E-312-11): echter SondeProcessor 48 kHz "
                    "Block 256 mit Testplayhead - ueber 200 Bloecke zaehlt RtWache::sperren() genau 200, eine je "
                    "Block an der Playhead-Stelle VOR dspKern->verarbeite, und RtWache::allokationen() bleibt 0",
                    d.str());
        }

        // 312/M-02: ein Workerzug mit Programmbau ZWISCHEN zwei Bloecken, auf
        // demselben Thread wie der Callback. Die Transaktion baut und
        // publiziert ein Programm, der Takt verarbeitet ein Hostereignis, und
        // der Testplayhead nimmt seine Sperre einmal ausserhalb eines Blocks.
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 6.0);
        const auto sperrenVor = dsp::RtWache::sperren();
        const auto allokVor   = dsp::RtWache::allokationen();
        allokationen = 0;
        zaehleAllokationen = true;
        const auto e = setze (*p, z);
        hostSchreibt (*p, iBand (0, param::kGainDb), 3.0f);
        p->kontrollTaktFuerTest();
        const bool ausserhalb = ! dsp::RtWache::imAudiopfad();
        (void) kopf->getPosition();
        zaehleAllokationen = false;
        const auto zugAllok    = allokationen;
        const auto sperrenZug  = dsp::RtWache::sperren();
        const auto allokZug    = dsp::RtWache::allokationen();
        fuelle();
        p->processBlock (puffer, midi);   // der zweite Block: der Zaehler lebt weiter
        const auto sperrenDanach = dsp::RtWache::sperren();
        {
            std::ostringstream d;
            d << "Workerzug: eigener Zaehler " << zugAllok << " Allokationen, RtWache::allokationen "
              << allokVor << " -> " << allokZug << ", RtWache::sperren " << sperrenVor << " -> " << sperrenZug
              << " (Playhead-Sperre ausserhalb eines Blocks), im Audiopfad " << (ausserhalb ? "nein" : "JA")
              << "; der Block danach " << sperrenZug << " -> " << sperrenDanach;
            pruefe (e.ausgang == tx::Ausgang::commit && ausserhalb && zugAllok > 0
                        && allokZug == allokVor && sperrenZug == sperrenVor
                        && sperrenDanach == sperrenZug + 1 && kopf->aufrufe == bloecke + 2,
                    "312/M-02 der_zaehler_sieht_den_workerzug_nicht (Teilfall von 312/M-01, R-312-1 Satz 2 und 3): "
                    "ein Workerzug mit Programmbau zwischen zwei Bloecken alloziert, und RtWache::allokationen() und "
                    "RtWache::sperren() steigen darin um 0; eine Sperre, die der Testplayhead ausserhalb eines Blocks "
                    "nimmt, zaehlt 0, die im naechsten Block wieder genau 1",
                    d.str());
        }
    }

    // ── 312/M-04: Testplayhead aus, das Bein oeffnet den Bereich wie der
    // Wrapper. Der VST3-Wrapper ruft `setNonRealtime` im selben Callback VOR
    // `processBlock`; der Bereich der Sonde beginnt erst in `processBlock` und
    // saehe eine Sperre des Offlineflags sonst gar nicht. Zwei Haelften mit
    // Etappenwechsel (E-312-10): bis Etappe 4 zaehlte derselbe Lauf 200 - jeder
    // Aufruf `setNonRealtime (true)` nahm `zustandSchloss` ueber den gemeldeten
    // Adapter. Seit Etappe 5 (T3-01-03) nimmt `setNonRealtime` keine Sperre
    // mehr, der Lauf zaehlt 0 und faellt mit 312/M-40 zusammen; dass der
    // Bereich eine Sperre dort ueberhaupt saehe, traegt der Rotbeweis von
    // 312/M-40. ──────────────────────────────────────────────────────────────
    {
        auto p = prozessor (fs, blk);
        juce::AudioBuffer<float> puffer (2, blk);
        juce::MidiBuffer midi;
        juce::Random w (3124);
        const auto lauf = [&] (bool offline)
        {
            dsp::RtWache::zuruecksetzen();
            for (int b = 0; b < bloecke; ++b)
            {
                for (int k = 0; k < 2; ++k)
                    for (int n = 0; n < blk; ++n)
                        puffer.setSample (k, n, w.nextFloat() * 1.6f - 0.8f);
                const dsp::RtWache::Bereich wieDerWrapper;
                p->setNonRealtime (offline);
                p->processBlock (puffer, midi);
            }
            return dsp::RtWache::sperren();
        };
        const auto offline  = lauf (true);
        const auto echtzeit = lauf (false);
        std::ostringstream d;
        d << "gemeldete Sperren ueber " << bloecke << " Bloecke: mit Aufruf setNonRealtime (true) " << offline
          << ", mit Aufruf setNonRealtime (false) " << echtzeit;
        pruefe (offline == 0 && echtzeit == 0,
                "312/M-04 offlineflag_nimmt_keine_gemeldete_sperre_mehr (R-312-1, E-312-10, ab Etappe 5; bis Etappe 4 "
                "offlineflag_nimmt_eine_gemeldete_sperre_je_block mit 200): legt das Bein den Bereich wie der Wrapper um "
                "setNonRealtime und processBlock, zaehlt RtWache::sperren() ueber 200 Bloecke 0 - je Aufruf "
                "setNonRealtime (true) wie je Aufruf setNonRealtime (false); faellt mit 312/M-40 zusammen",
                d.str());
    }
}

//==============================================================================
// NAK-312 Etappe 3, erster Aenderungssatz (W02; R-312-10 zweiter Teil,
// R-312-11): der Ladestart kennt seine Nachbarn - die Hostwert-Mailbox und das
// read-only des geladenen Standes.
//
// Keine Zeile haengt an der Wanduhr: die Taktsperre
// (`mitAngehaltenemTaktFuerTest`) haelt das Zustandsschloss, in dem der Worker
// beweisbar nicht tickt, und jeder Takt ist ein ausdrueckliches
// `kontrollTaktFuerTest()`. Hostwerte kommen ueber `hostSchreibt` wie aus dem
// VST3-Wrapper.

constexpr int kInTrim  = 1;   // v1.global.input_trim_db
constexpr int kOutTrim = 2;   // v1.global.output_trim_db

double dbFaktor (double db) { return std::pow (10.0, db / 20.0); }

/** Ausgang/Eingang gegen einen Sollfaktor, je Sample und Kanal. */
struct Verhaeltnis
{
    double groessteAbweichung = 0.0;
    int    gemessen = 0;
    bool   endlich  = true;
};

/** Faehrt `bloecke` Bloecke eines Signals mit wechselndem Vorzeichen und Betrag
    in [0,2; 0,8] - nie 0, damit Ausgang/Eingang an jedem Sample definiert ist -
    und misst ab Sample `ab` die groesste Abweichung des Verhaeltnisses von `g`.
    Ein nicht endliches Verhaeltnis macht die Abweichung NaN, und jeder
    Vergleich `<= Schranke` faellt dann. */
Verhaeltnis verhaeltnisNach (Prozessor& p, int bloecke, int groesse, int ab, double g, int saat)
{
    Verhaeltnis v;
    juce::Random w (saat);
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> puffer (2, groesse), eingang (2, groesse);
    int index = 0;
    for (int b = 0; b < bloecke; ++b)
    {
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < groesse; ++n)
            {
                const float x = (0.2f + 0.6f * w.nextFloat()) * ((n & 1) != 0 ? -1.0f : 1.0f);
                puffer.setSample (k, n, x);
                eingang.setSample (k, n, x);
            }
        p.processBlock (puffer, midi);
        for (int n = 0; n < groesse; ++n, ++index)
            for (int k = 0; k < 2; ++k)
            {
                const float y = puffer.getSample (k, n);
                if (! std::isfinite (y)) v.endlich = false;
                if (index < ab) continue;
                const double d = std::abs ((double) y / (double) eingang.getSample (k, n) - g);
                if (! (d <= v.groessteAbweichung)) v.groessteAbweichung = d;
                ++v.gemessen;
            }
    }
    return v;
}

/** Ausgang gegen Eingang, bitweise je Sample und Kanal. */
struct Bitvergleich
{
    int  abweichend = 0;
    int  gemessen   = 0;
    bool endlich    = true;
};

Bitvergleich gegenEingang (Prozessor& p, int bloecke, int groesse, int saat)
{
    Bitvergleich v;
    juce::Random w (saat);
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> puffer (2, groesse), eingang (2, groesse);
    for (int b = 0; b < bloecke; ++b)
    {
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < groesse; ++n)
                puffer.setSample (k, n, w.nextFloat() * 1.6f - 0.8f);
        eingang.makeCopyOf (puffer);
        p.processBlock (puffer, midi);
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < groesse; ++n)
            {
                const float y = puffer.getSample (k, n), x = eingang.getSample (k, n);
                if (! std::isfinite (y)) v.endlich = false;
                if (std::memcmp (&y, &x, sizeof (float)) != 0) ++v.abweichend;
                ++v.gemessen;
            }
    }
    return v;
}

void nak312Ladestart()
{
    abschnitt ("X - NAK-312 Etappe 3a: W02 Ladestart - Hostmailbox und read-only (312/M-10 bis 312/M-13, 312/M-15 bis 312/M-19, 312/M-78; 312/M-14 in Abschnitt O)");

    const double fs    = 48000.0;
    const int    blk   = 256;
    const int    ab    = 4 * blk;   // Einschwingen: Rampe und Crossfade sind je 256 Samples
    const int    bloecke = 6;       // gemessen werden 512 Samples je Kanal

    /** Der Stand aus 312/M-10: eq an, Output-Trim +3 dB ueber eine echte
        Transaktion, Audio gefahren, dann die Statebytes. */
    const auto plusDrei = [&] (Prozessor& p, juce::MemoryBlock& bytes)
    {
        auto z = mitEq (true);
        z.werte[(size_t) kOutTrim].zahl = 3.0;
        const auto e = setze (p, z);
        fahreAudio (p, 8, blk, 3100);
        p.getStateInformation (bytes);
        return e.ausgang == tx::Ausgang::commit;
    };
    const auto wirksamDb = [] (Prozessor& p, int index) { return p.wirksamerZustand().werte[(size_t) index].zahl; };
    const auto reglerDb  = [] (Prozessor& p, int index)
    {
        auto& q = hostParam (p, index);
        return (double) q.convertFrom0to1 (q.getValue());
    };
    const auto text = [] (double x) { std::ostringstream s; s << std::setprecision (12) << x; return s.str(); };

    // ── 312/M-10 und 312/M-11 ────────────────────────────────────────────
    {
        auto p = prozessor (fs, blk);
        juce::MemoryBlock bytes;
        const bool commit = plusDrei (*p, bytes);
        double nachTakt = 0.0;
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            hostSchreibt (*p, kOutTrim, -9.0f);   // OHNE Kontrolltakt: liegt in der Mailbox
            p->setStateInformation (bytes.getData(), (int) bytes.getSize());
            p->kontrollTaktFuerTest();
            nachTakt = wirksamDb (*p, kOutTrim);
            // Ein Hostereignis auf einem FREIEN Slot oeffnet die Mailbox
            // wieder (ohne Wirkung auf Klang, [SONDE-015] M-63); der alte Wert
            // darf dabei nicht aufleben.
            hostSchreibt (*p, iBand (5, param::kGainDb), 4.0f);
            p->kontrollTaktFuerTest();
        });
        const double wirksam = wirksamDb (*p, kOutTrim);
        const double regler  = reglerDb (*p, kOutTrim);
        const auto   v       = verhaeltnisNach (*p, bloecke, blk, ab, dbFaktor (3.0), 3101);
        pruefe (commit && nachTakt == 3.0 && wirksam == 3.0 && regler == 3.0 && v.endlich && v.gemessen == 1024
                    && v.groessteAbweichung <= 1.0e-6,
                "312/M-10 ladestart_macht_ausstehende_hostwerte_wirkungslos (T3-05-01, R-312-10 zweiter Teil, [SONDE-015] M-84): "
                "Output-Trim +3 dB committet, danach -9 dB in der Mailbox ohne Kontrolltakt, dieselben Bytes in DIESELBE "
                "Instanz geladen, dann ein Takt - wirksam exakt +3 dB, auch nachdem ein Hostereignis auf einem freien Slot die "
                "Mailbox wieder geoeffnet hat; der Regler zeigt +3 dB, und Ausgang/Eingang liegt ueber 512 Samples nach dem "
                "Einschwingen innerhalb 1e-6 von 10^(3/20)",
                "wirksam nach dem Takt " + text (nachTakt) + " dB, nach der Wiederoeffnung " + text (wirksam) + " dB, Regler "
                + text (regler) + " dB, groesste Abweichung " + text (v.groessteAbweichung) + " ueber "
                + std::to_string (v.gemessen) + " Werte");

    }
    {
        // 312/M-11 in einer eigenen Instanz mit demselben Aufbau: der neue
        // Gestus ist das ERSTE Hostereignis nach dem Laden.
        auto p = prozessor (fs, blk);
        juce::MemoryBlock bytes;
        const bool commit = plusDrei (*p, bytes);
        double geladen = 0.0;
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            hostSchreibt (*p, kOutTrim, -9.0f);
            p->setStateInformation (bytes.getData(), (int) bytes.getSize());
            p->kontrollTaktFuerTest();
            geladen = wirksamDb (*p, kOutTrim);
            hostSchreibt (*p, kOutTrim, -6.0f);
            p->kontrollTaktFuerTest();
        });
        const double neu = wirksamDb (*p, kOutTrim);
        const auto   v   = verhaeltnisNach (*p, bloecke, blk, ab, dbFaktor (-6.0), 3111);
        pruefe (commit && geladen == 3.0 && neu == -6.0 && reglerDb (*p, kOutTrim) == -6.0 && v.endlich
                    && v.groessteAbweichung <= 1.0e-6,
                "312/M-11 neuer_hostgestus_nach_dem_recall_wirkt (Teilfall von 312/M-10, R-312-10 zweiter Teil): "
                "Aufbau wie 312/M-10, danach -6 dB ueber hostSchreibt als erstes Hostereignis NACH dem Laden, dann ein "
                "Takt - wirksam exakt -6 dB, Regler und Klang folgen",
                "geladen " + text (geladen) + " dB, danach wirksam " + text (neu) + " dB, groesste Abweichung von 10^(-6/20) "
                + text (v.groessteAbweichung));
    }

    // ── 312/M-12: ein prepareToPlay zwischen Laden und Takt ────────────────
    {
        auto p = prozessor (fs, blk);
        juce::MemoryBlock bytes;
        const bool commit = plusDrei (*p, bytes);
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            hostSchreibt (*p, kOutTrim, -9.0f);
            p->setStateInformation (bytes.getData(), (int) bytes.getSize());
            p->setRateAndBufferSizeDetails (fs, blk);
            p->prepareToPlay (fs, blk);
            p->kontrollTaktFuerTest();
            hostSchreibt (*p, iBand (5, param::kGainDb), 4.0f);   // oeffnet die Mailbox wieder, wie 312/M-10
            p->kontrollTaktFuerTest();
        });
        const double wirksam = wirksamDb (*p, kOutTrim);
        const auto   v       = verhaeltnisNach (*p, bloecke, blk, ab, dbFaktor (3.0), 3121);
        pruefe (commit && wirksam == 3.0 && v.endlich && v.gemessen == 1024 && v.groessteAbweichung <= 1.0e-6,
                "312/M-12 prepare_nach_dem_laden_publiziert_den_geladenen_stand (Teilfall von 312/M-10, [SONDE-015] M-84): "
                "zwischen Laden und Takt ein prepareToPlay (48000, 256), danach wie 312/M-10 die Wiederoeffnung der Mailbox - "
                "derselbe Ausgang wie 312/M-10, exakt +3 dB",
                "wirksam " + text (wirksam) + " dB, groesste Abweichung " + text (v.groessteAbweichung));
    }

    // ── 312/M-13: reset bewahrt die Mailbox (Regressionswache) ─────────────
    {
        auto p = prozessor (fs, blk);
        juce::MemoryBlock bytes;
        const bool commit = plusDrei (*p, bytes);
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            hostSchreibt (*p, kOutTrim, -9.0f);
            p->reset();
            p->kontrollTaktFuerTest();
        });
        const double wirksam = wirksamDb (*p, kOutTrim);
        const auto   v       = verhaeltnisNach (*p, bloecke, blk, ab, dbFaktor (-9.0), 3131);
        pruefe (commit && wirksam == -9.0 && v.endlich && v.groessteAbweichung <= 1.0e-6,
                "312/M-13 reset_bewahrt_die_hostmailbox (Teilfall von 312/M-10, Regressionswache): -9 dB in der Mailbox, "
                "reset() zwischen Schreiben und Takt, KEIN Ladestart - wirksam danach -9 dB, der Ausgang folgt",
                "wirksam " + text (wirksam) + " dB, groesste Abweichung von 10^(-9/20) " + text (v.groessteAbweichung));
    }

    // ── 312/M-15, 312/M-16, 312/M-17: read-only und der Rueckweg ─────────
    juce::MemoryBlock fremd;
    const bool fremdGelesen = wurzel().getChildFile ("eq-copilot/fixtures/state/schema2/fremdes-major-3.bin").loadFileAsData (fremd);
    {
        auto p = prozessor (fs, blk);
        juce::MemoryBlock bytes;
        const bool commit = plusDrei (*p, bytes);                // vorher engagiert
        p->setStateInformation (fremd.getData(), (int) fremd.getSize());
        fahreAudio (*p, 8, blk, 3150);                            // der Pfad blendet in die Ruhe
        const auto referenz = gegenEingang (*p, 2, blk, 3151);   // vor der Automation
        DirtyZaehler dirty;
        p->addListener (&dirty);
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            hostSchreibt (*p, param::kIndexEqEnabled, 1.0f);
            hostSchreibt (*p, kOutTrim, 6.0f);
            p->kontrollTaktFuerTest();
        });
        const auto nachher = gegenEingang (*p, 2, blk, 3152);
        juce::MemoryBlock zurueck;
        p->getStateInformation (zurueck);
        const bool nurLesen = p->zustandLesen().nurLesen;
        const int  dirtyMeldungen = dirty.nichtParameter;
        p->removeListener (&dirty);
        pruefe (fremdGelesen && commit && referenz.abweichend == 0 && nachher.gemessen == 1024 && nachher.abweichend == 0
                    && nachher.endlich && nurLesen && zurueck == fremd && dirtyMeldungen == 0,
                "312/M-15 read_only_bleibt_audio_neutral (T3-05-02, R-312-10 zweiter Teil, [SONDE-015] M-92): fremdes Major "
                "geladen, danach eq_enabled 1 und Output-Trim +6 dB ueber hostSchreibt und ein Takt - der Ausgang bleibt ueber "
                "512 Samples bitgleich zum Eingang, nurLesen bleibt, getStateInformation liefert die Fixture bytegleich, kein Host-Dirty",
                "vor der Automation " + std::to_string (referenz.abweichend) + ", danach " + std::to_string (nachher.abweichend)
                + " von " + std::to_string (nachher.gemessen) + " Werten abweichend, Dirty " + std::to_string (dirtyMeldungen));

        auto& eq  = hostParam (*p, param::kIndexEqEnabled);
        auto& out = hostParam (*p, kOutTrim);
        pruefe (eq.getValue() == 0.0f && out.getValue() == out.convertTo0to1 (0.0f),
                "312/M-16 read_only_regler_springt_auf_neutral (Teilfall von 312/M-15, [SONDE-015] M-92): nach dem Takt stehen "
                "eq_enabled und Output-Trim wieder auf ihrem neutralen Wert - kein Regler zeigt einen Zustand, den es nicht gibt",
                "eq_enabled " + text (eq.getValue()) + ", Output-Trim " + text (reglerDb (*p, kOutTrim)) + " dB");
    }
    {
        auto quelle = prozessor (fs, blk);
        setze (*quelle, mitEq (true));
        juce::MemoryBlock eigen;
        quelle->getStateInformation (eigen);
        auto p = prozessor (fs, blk);
        p->setStateInformation (fremd.getData(), (int) fremd.getSize());   // read-only ...
        const bool warLesend = p->zustandLesen().nurLesen;
        p->setStateInformation (eigen.getData(), (int) eigen.getSize());   // ... und zurueck
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            hostSchreibt (*p, param::kIndexEqEnabled, 1.0f);
            hostSchreibt (*p, kOutTrim, 6.0f);
            p->kontrollTaktFuerTest();
        });
        const auto v = verhaeltnisNach (*p, bloecke, blk, ab, dbFaktor (6.0), 3171);
        pruefe (fremdGelesen && warLesend && ! p->zustandLesen().nurLesen && wirksamDb (*p, kOutTrim) == 6.0
                    && v.endlich && v.gemessen == 1024 && v.groessteAbweichung <= 1.0e-6,
                "312/M-17 nach_read_only_wirkt_automation_wieder (Teilfall von 312/M-15, Regressionswache): erst das fremde "
                "Major, dann ein gueltiger eigener Stand mit eq an, dieselbe Hostautomation - Ausgang/Eingang innerhalb 1e-6 "
                "von 10^(6/20)",
                "wirksam " + text (wirksamDb (*p, kOutTrim)) + " dB, groesste Abweichung " + text (v.groessteAbweichung));
    }

    // ── 312/M-18: nicht endliche Hostwerte vor und nach dem Ladestart ──────
    {
        const float nan    = std::numeric_limits<float>::quiet_NaN();
        const float inf    = std::numeric_limits<float>::infinity();
        auto p = prozessor (fs, blk);
        juce::MemoryBlock bytes;
        const bool commit = plusDrei (*p, bytes);
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            hostParam (*p, kOutTrim).setValueNotifyingHost (nan);          // vor dem Ladestart
            p->setStateInformation (bytes.getData(), (int) bytes.getSize());
            hostParam (*p, kOutTrim).setValueNotifyingHost (nan);          // danach
            hostParam (*p, kInTrim).setValueNotifyingHost (inf);
            hostParam (*p, param::indexVonId ("v1.global.width")).setValueNotifyingHost (-inf);
            p->kontrollTaktFuerTest();
        });
        const auto w = p->wirksamerZustand();
        const auto v = verhaeltnisNach (*p, bloecke, blk, ab, dbFaktor (3.0), 3181);
        const bool eigenerStand = commit && w.werte[(size_t) kOutTrim].zahl == 3.0 && w.werte[(size_t) kInTrim].zahl == 0.0
                               && w.werte[(size_t) param::indexVonId ("v1.global.width")].zahl == 1.0
                               && v.endlich && v.groessteAbweichung <= 1.0e-6;

        auto r = prozessor (fs, blk);
        r->setStateInformation (fremd.getData(), (int) fremd.getSize());
        fahreAudio (*r, 8, blk, 3182);
        r->mitAngehaltenemTaktFuerTest ([&]
        {
            hostParam (*r, param::kIndexEqEnabled).setValueNotifyingHost (nan);
            hostParam (*r, kOutTrim).setValueNotifyingHost (inf);
            hostParam (*r, kInTrim).setValueNotifyingHost (-inf);
            r->kontrollTaktFuerTest();
        });
        const auto lesend = gegenEingang (*r, 2, blk, 3183);
        pruefe (eigenerStand && fremdGelesen && lesend.endlich && lesend.abweichend == 0,
                "312/M-18 nicht_endlicher_hostwert_bleibt_wirkungslos (Teilfaelle von 312/M-10 und 312/M-15, Regressionswache): "
                "NaN vor und nach dem Ladestart, +Inf und -Inf danach - der wirksame Zustand bleibt der bestaetigte, jedes "
                "Ausgangssample ist endlich; im read-only-Stand bleibt der Ausgang bitgleich zum Eingang",
                "Output-Trim " + text (w.werte[(size_t) kOutTrim].zahl) + " dB, Input-Trim " + text (w.werte[(size_t) kInTrim].zahl)
                + " dB, groesste Abweichung " + text (v.groessteAbweichung) + ", read-only abweichend "
                + std::to_string (lesend.abweichend));
    }

    // ── 312/M-19: der Zaehlerrand der Quittierung ──────────────────────────
    {
        auto p = prozessor (fs, blk);
        juce::MemoryBlock bytes;
        const bool commit = plusDrei (*p, bytes);
        double a = 0.0, b = 0.0, c = 0.0, d = 0.0;
        std::uint64_t e0 = 0, e1 = 0, e2 = 0;
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            p->setzeHostZaehlerFuerTest (kOutTrim, 0xFFFFFFFEu);
            hostSchreibt (*p, kOutTrim, -6.0f);    // Zaehler 0xFFFFFFFF
            p->kontrollTaktFuerTest();
            a = wirksamDb (*p, kOutTrim);
            hostSchreibt (*p, kOutTrim, -9.0f);    // Zaehler 0: der Ueberlauf
            p->kontrollTaktFuerTest();
            b = wirksamDb (*p, kOutTrim);
            fahreAudio (*p, 50, blk, 3190);        // 12 800 Samples: ueber der Ruhegrenze
            p->kontrollTaktFuerTest();             // Ende der Hostgeste
            e0 = p->automationEpoche();
            p->kontrollTaktFuerTest();             // kein neues Ereignis: nichts wird verbraucht
            e1 = p->automationEpoche();
            hostSchreibt (*p, kOutTrim, -12.0f);   // Zaehler 1, OHNE Takt
            p->setStateInformation (bytes.getData(), (int) bytes.getSize());
            p->kontrollTaktFuerTest();
            e2 = p->automationEpoche();             // der Takt nach dem Laden verbraucht nichts
            hostSchreibt (*p, iBand (5, param::kGainDb), 4.0f);   // oeffnet die Mailbox wieder, wie 312/M-10
            p->kontrollTaktFuerTest();
            c = wirksamDb (*p, kOutTrim);
            hostSchreibt (*p, kOutTrim, -3.0f);    // Zaehler 2
            p->kontrollTaktFuerTest();
            d = wirksamDb (*p, kOutTrim);
        });
        pruefe (commit && a == -6.0 && b == -9.0 && e1 == e0 && c == 3.0 && e2 == e1 && d == -3.0,
                "312/M-19 quittierung_ist_ueberlaufsicher (R-312-10 zweiter Teil): Zaehler und Quittierungsstand bei 0xFFFFFFFE, "
                "-6 dB (Zaehler 0xFFFFFFFF) und -9 dB (Zaehler 0) wirken je nach einem Takt, ein Takt ohne Ereignis verbraucht "
                "nichts doppelt (Epoche steht), -12 dB vor dem Ladestart bleibt wirkungslos, auch nach der Wiederoeffnung der "
                "Mailbox, -3 dB danach wirkt",
                text (a) + " / " + text (b) + " / " + text (c) + " / " + text (d) + " dB, Epoche " + zahl (e0) + " -> "
                + zahl (e1) + " -> " + zahl (e2));
    }

    // ── 312/M-78: ein Hostwert WAEHREND des Ladens ─────────────────────────
    {
        auto p = prozessor (fs, blk);
        juce::MemoryBlock bytes;
        const bool commit = plusDrei (*p, bytes);
        bool hakenLief = false;
        p->setzeLadeHakenFuerTest ([&]
        {
            // Die Schranke: setStateInformation steht INNERHALB des
            // Zustandsschlosses, nach der Quittierung und vor dem Abgleich,
            // bis ein zweiter Thread den Hostwert geschrieben hat.
            hakenLief = true;
            std::thread zweiter ([&] { hostSchreibt (*p, kOutTrim, -6.0f); });
            zweiter.join();
        });
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            p->setStateInformation (bytes.getData(), (int) bytes.getSize());
            p->kontrollTaktFuerTest();
        });
        p->setzeLadeHakenFuerTest ({});
        auto& q = hostParam (*p, kOutTrim);
        const bool   reglerGleich = q.getValue() == q.convertTo0to1 (-6.0f);
        const double wirksam      = wirksamDb (*p, kOutTrim);
        const auto   v            = verhaeltnisNach (*p, bloecke, blk, ab, dbFaktor (-6.0), 3781);
        pruefe (commit && hakenLief && reglerGleich && wirksam == -6.0 && v.endlich && v.gemessen == 1024
                    && v.groessteAbweichung <= 1.0e-6,
                "312/M-78 hostwert_nach_der_quittierung_wirkt_und_wird_gezeigt (R-312-11): -6 dB aus einem zweiten Thread, "
                "waehrend setStateInformation nach der Quittierung und vor dem Abgleich steht - wirksam exakt -6 dB, der "
                "Regler liest nach dem Abgleich genau diesen Wert, und kein Sample nach dem Einschwingen klingt nach +3 dB",
                std::string ("Haken ") + (hakenLief ? "lief" : "lief NICHT") + ", Regler " + text (reglerDb (*p, kOutTrim))
                + " dB, wirksam " + text (wirksam) + " dB, groesste Abweichung von 10^(-6/20) " + text (v.groessteAbweichung));
    }
}

//==============================================================================
// NAK-312 Etappe 3, zweiter Aenderungssatz (T3-01-05 Teil a; R-312-10 in der
// Fassung von E-312-5, Schiedsregel E-312-6): die Blockbindung der Hostwerte.
//
// Pruefling A faehrt seine Bloecke unter der Taktsperre OHNE Kontrolltakt -
// der Worker kann dort beweisbar nicht ziehen. Der Referenzlauf B faehrt
// dieselben Bloecke und bekommt nach JEDEM Block `kontrollTaktFuerTest()`:
// das ist die Ausgabe, die der Entwurfssatz "vom vorigen zum letzten
// Blockwert" beschreibt. Beide faehren dasselbe Rauschen und dieselbe
// Eventfolge, beide unter der Taktsperre, also ohne Wettlauf mit dem Worker.
//
// NAK312_GEGENPROBE_OHNE_BLOCKRAND blendet die Faelle aus, die Testzugaenge
// des Mechanismus lesen (Abdeckungstabelle, Blockrandstand, Zielzaehler). Die
// Gegenprobe "heute rot" (Manifest §7.1) setzt ihn nur im Basislauf, damit
// die heute-rot-Faelle gegen den unveraenderten Produktcode bauen.

using Ereignisse = std::function<void (Prozessor&, int)>;

constexpr int kBreite = 3;   // v1.global.width

/** Faehrt `bloecke` Bloecke unter der Taktsperre: vor jedem Block `ereignis`,
    dann das Rauschen des Blocks, danach bei `taktJeBlock` ein Kontrolltakt. */
std::vector<float> fahreFolge (Prozessor& p, int bloecke, int groesse, const Ereignisse& ereignis,
                               bool taktJeBlock, int saat)
{
    std::vector<float> aus;
    aus.reserve ((size_t) bloecke * (size_t) groesse * 2u);
    juce::Random w (saat);
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> puffer (2, groesse);
    p.mitAngehaltenemTaktFuerTest ([&]
    {
        for (int b = 0; b < bloecke; ++b)
        {
            if (ereignis)
                ereignis (p, b);
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < groesse; ++n)
                    puffer.setSample (k, n, w.nextFloat() * 1.6f - 0.8f);
            p.processBlock (puffer, midi);
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < groesse; ++n)
                    aus.push_back (puffer.getSample (k, n));
            if (taktJeBlock)
                p.kontrollTaktFuerTest();
        }
    });
    return aus;
}

int abweichend (const std::vector<float>& a, const std::vector<float>& b)
{
    if (a.size() != b.size())
        return std::numeric_limits<int>::max();
    int n = 0;
    for (size_t i = 0; i < a.size(); ++i)
        if (std::memcmp (&a[i], &b[i], sizeof (float)) != 0)
            ++n;
    return n;
}

/** Ein Pruefling mit dem committeten Stand `z`, eingeschwungen (die
    Publikation des Commits ist genommen, Uebergaenge sind zu Ende). */
std::unique_ptr<Prozessor> blockPruefling (int groesse, const param::DspSatz& z)
{
    auto p = prozessor (48000.0, groesse);
    setze (*p, z);
    fahreAudio (*p, std::max (8, 2048 / groesse), groesse, 3200);
    return p;
}

/** Die Eventfolge aus 312/M-20: Block 10 = Extremwert, 11 = Standardwert,
    12 = Extremwert, 13 = Standardwert. */
Ereignisse folgeM20 (int index, float extrem, float standard)
{
    return [index, extrem, standard] (Prozessor& p, int b)
    {
        if (b == 10 || b == 12) hostSchreibt (p, index, extrem);
        if (b == 11 || b == 13) hostSchreibt (p, index, standard);
    };
}

struct BlockParameter { int index; const char* name; float extrem; float standard; };

/** Die zwei Staende, in denen 312/M-20 bis 312/M-22 fahren: der NEUTRALE
    (eq an, keine Baender - der Kern schreibt dort nur, solange eine Rampe
    laeuft, die Neutralpruefung ist also mitgemessen) und einer mit Band (Bell
    1 kHz +6 dB), in dem auch Mix hoerbar ist. Im neutralen Stand ist Mix
    bauartbedingt ohne Wirkung (Nass gleich Trocken); dort wird fuer Mix nur
    A gegen B verlangt. */
struct Blockstand { const char* name; param::DspSatz satz; };

std::array<Blockstand, 2> blockStaende()
{
    auto mitBand = mitEq (true);
    setzeBand (mitBand, 0, 1000.0, 6.0);
    return { { { "neutral", mitEq (true) }, { "Band", mitBand } } };
}

/** Die vier abgedeckten Parameter mit einem Extremwert ungleich ihrem Standard.
    Mix steht im Standard am oberen Rand (1,0); sein Extremwert ist deshalb der
    untere (0,0). */
const std::array<BlockParameter, 4>& blockParameter()
{
    static const std::array<BlockParameter, 4> t { {
        { kInTrim,          "v1.global.input_trim_db",  24.0f, 0.0f },
        { kOutTrim,         "v1.global.output_trim_db", 24.0f, 0.0f },
        { kBreite,          "v1.global.width",           2.0f, 1.0f },
        { param::kIndexMix, "v2.global.mix",             0.0f, 1.0f } } };
    return t;
}

/** A (Taktsperre, kein Takt) gegen B (Takt nach jedem Block) und C (ohne
    Ereignis): A muss bitgleich zu B sein, und B muss sich von C
    unterscheiden, sonst misst der Fall nichts. */
struct Vergleich { int aGegenB = 0; int bGegenC = 0; std::uint64_t zieleA = 0; };

Vergleich vergleicheAB (const param::DspSatz& stand, int groesse, int bloecke, const Ereignisse& ereignis, int saat,
                        bool offlineA = false)
{
    auto a = blockPruefling (groesse, stand);
    auto b = blockPruefling (groesse, stand);
    auto c = blockPruefling (groesse, stand);
    if (offlineA)
        a->setNonRealtime (true);
#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
    const auto zieleVor = a->dspKernFuerTest().blockrandZiele();
#endif
    const auto ya = fahreFolge (*a, bloecke, groesse, ereignis, false, saat);
    const auto yb = fahreFolge (*b, bloecke, groesse, ereignis, true, saat);
    const auto yc = fahreFolge (*c, bloecke, groesse, {}, true, saat);
    Vergleich v;
    v.aGegenB = abweichend (ya, yb);
    v.bGegenC = abweichend (yb, yc);
#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
    v.zieleA = a->dspKernFuerTest().blockrandZiele() - zieleVor;
#endif
    return v;
}

/** Ausgang/Eingang am letzten Sample eines Blocks, links. */
double letztesVerhaeltnis (const std::vector<float>& aus, const std::vector<float>& ein, int block, int groesse)
{
    const size_t i = (size_t) block * (size_t) groesse * 2u + (size_t) groesse - 1u;
    return (double) aus[i] / (double) ein[i];
}

/** Das Eingangsrauschen von `fahreFolge` mit derselben Saat, im selben Format. */
std::vector<float> eingangVon (int bloecke, int groesse, int saat)
{
    std::vector<float> ein;
    ein.reserve ((size_t) bloecke * (size_t) groesse * 2u);
    juce::Random w (saat);
    for (int b = 0; b < bloecke; ++b)
    {
        std::vector<float> block ((size_t) groesse * 2u);
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < groesse; ++n)
                block[(size_t) k * (size_t) groesse + (size_t) n] = w.nextFloat() * 1.6f - 0.8f;
        ein.insert (ein.end(), block.begin(), block.end());
    }
    return ein;
}

/** Abweichende Samples zweier Ausgaben im Format von `fahreFolge`, je Kanal,
    und das Fenster: erstes und letztes abweichendes Sample, gezaehlt ab dem
    ersten Sample des Laufs (312/M-87). */
struct Abweichungsbild { int links = 0; int rechts = 0; long long erstes = -1; long long letztes = -1; };

Abweichungsbild abweichungsbild (const std::vector<float>& a, const std::vector<float>& b, int groesse)
{
    Abweichungsbild bild;
    if (a.size() != b.size())
    {
        bild.links = bild.rechts = std::numeric_limits<int>::max();
        return bild;
    }
    const size_t jeBlock = (size_t) groesse * 2u;
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (std::memcmp (&a[i], &b[i], sizeof (float)) == 0)
            continue;
        const size_t imBlock = i % jeBlock;
        const bool   rechts  = imBlock >= (size_t) groesse;
        const auto   sample  = (long long) ((i / jeBlock) * (size_t) groesse
                                            + (rechts ? imBlock - (size_t) groesse : imBlock));
        ++(rechts ? bild.rechts : bild.links);
        if (bild.erstes < 0 || sample < bild.erstes) bild.erstes = sample;
        if (sample > bild.letztes) bild.letztes = sample;
    }
    return bild;
}

std::string beschreibe (const Abweichungsbild& b)
{
    std::string t = "links " + std::to_string (b.links) + ", rechts " + std::to_string (b.rechts) + " abweichend";
    if (b.erstes >= 0)
        t += " (Samples " + std::to_string (b.erstes) + " bis " + std::to_string (b.letztes) + " nach dem Laden)";
    return t;
}

/** Das groesste Verhaeltnis Ausgang/Eingang in dB ueber alle Samples mit
    Eingang ungleich 0. Aussagekraeftig nur, solange der Pfad je Sample einen
    reinen Faktor rechnet (Trims, kein Width, kein Mix) - 312/M-87 misst damit
    ohne Bedingung die Spitze, die ein Output-Trim-Hostwert ins Ausblenden
    traegt. */
double groessterFaktorDb (const std::vector<float>& aus, const std::vector<float>& ein)
{
    double groesster = 0.0;
    for (size_t i = 0; i < aus.size() && i < ein.size(); ++i)
        if (ein[i] != 0.0f)
            groesster = std::max (groesster, (double) aus[i] / (double) ein[i]);
    return 20.0 * std::log10 (groesster);
}

void nak312Blockbindung()
{
    abschnitt ("Y - NAK-312 Etappe 3b: Blockbindung der Hostwerte (312/M-20 bis 312/M-23, 312/M-26 bis 312/M-29, 312/M-79 bis 312/M-82, seit der Nacharbeit 1 312/M-87; 312/M-24 und 312/M-25 in Abschnitt O)");

    const int blk = 64;
    const auto bloeckeFuer = [] (int groesse) { return 14 + std::max (6, (2048 + groesse - 1) / groesse); };
    const auto text = [] (double x) { std::ostringstream s; s << std::setprecision (12) << x; return s.str(); };

    /** Die Eventfolge von 312/M-20 fuer die vier Parameter in beiden
        Staenden. Zaehlt die Laeufe, in denen A von B abweicht oder B (wo der
        Parameter hoerbar ist) nicht von C, und summiert die Blockrandziele. */
    struct Menge { int schlecht = 0; std::uint64_t ziele = 0; std::string text; };
    const auto mengeLauf = [&] (int groesse, int saat, bool offline)
    {
        Menge m;
        std::ostringstream d;
        for (const auto& st : blockStaende())
            for (const auto& bp : blockParameter())
            {
                const auto v = vergleicheAB (st.satz, groesse, bloeckeFuer (groesse),
                                             folgeM20 (bp.index, bp.extrem, bp.standard), saat, offline);
                const bool hoerbar = ! (bp.index == param::kIndexMix && std::string (st.name) == "neutral");
                if (v.aGegenB != 0 || (hoerbar && v.bGegenC == 0))
                    ++m.schlecht;
                m.ziele += v.zieleA;
                d << st.name << "/" << bp.name << " A/B " << v.aGegenB << " B/C " << v.bGegenC << "; ";
            }
        m.text = d.str();
        return m;
    };

    // ── 312/M-20: je Parameter der abgedeckten Menge ──────────────────────
    bool vierBitgleich = true;
    {
        const auto m = mengeLauf (blk, 3201, false);
        vierBitgleich = m.schlecht == 0;
        pruefe (m.schlecht == 0,
                "312/M-20 hostwerte_sind_blockgebunden (T3-01-05 Teil a, R-312-10 in der Fassung von E-312-5, [SONDE-015] M-120): "
                "je Parameter der abgedeckten Menge dieselbe Eventfolge (Block 10 Extremwert, 11 Standard, 12 Extremwert, "
                "13 Standard) bei 48 kHz und Blockgroesse 64, im neutralen Stand und mit Band - A unter der Taktsperre ohne "
                "Takt ist BITGLEICH zu B mit Takt nach jedem Block (0 abweichende Samples), und B weicht vom Lauf ohne "
                "Ereignis ab, wo der Parameter hoerbar ist",
                m.text);
    }

    // ── Teilfall von 312/M-20: nicht endliche Hostwerte am Blockrand ───────
    // Der Blockrand rechnet dieselbe Zelle wie `zelleAusHost`: NaN und +-Inf
    // zaehlen als unveraendert (E4-10) - das Rampenziel wird der bestaetigte
    // Wert, der Ausgang bleibt der des Laufs ohne Ereignis.
    {
        const float nan = std::numeric_limits<float>::quiet_NaN();
        const float inf = std::numeric_limits<float>::infinity();
        const Ereignisse nichtEndlich = [nan, inf] (Prozessor& p, int b)
        {
            for (const auto& bp : blockParameter())
            {
                if (b == 10) hostParam (p, bp.index).setValueNotifyingHost (nan);
                if (b == 11) hostParam (p, bp.index).setValueNotifyingHost (inf);
                if (b == 12) hostParam (p, bp.index).setValueNotifyingHost (-inf);
            }
        };
        const auto& mitBand = blockStaende()[1].satz;
        auto a = blockPruefling (blk, mitBand);
        auto c = blockPruefling (blk, mitBand);
#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
        const auto ziele0 = a->dspKernFuerTest().blockrandZiele();
#endif
        const auto ya = fahreFolge (*a, 40, blk, nichtEndlich, false, 3205);
        const auto yc = fahreFolge (*c, 40, blk, {}, false, 3205);
        const bool endlich = std::all_of (ya.begin(), ya.end(), [] (float x) { return std::isfinite (x); });
        bool ok = endlich && abweichend (ya, yc) == 0;
        std::string d = std::string ("Ausgang ") + (endlich ? "endlich" : "NICHT endlich") + ", gegen den Lauf ohne Ereignis "
                      + std::to_string (abweichend (ya, yc)) + " abweichend";
#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
        const auto ziele = a->dspKernFuerTest().blockrandZiele() - ziele0;
        ok = ok && ziele == 12;
        d += ", Blockrandziele " + zahl (ziele) + " (3 Ereignisse x 4 Parameter, je auf den bestaetigten Wert)";
#endif
        pruefe (ok,
                "312/M-20 Teilfall nicht_endliche_hostwerte_am_blockrand (E4-10, NaN-Ehrlichkeit): NaN, +Inf und -Inf auf den "
                "vier abgedeckten Parametern erreichen den Blockrand als bestaetigter Wert - der Ausgang bleibt endlich und "
                "bitgleich zum Lauf ohne Ereignis",
                d);
    }

    // ── 312/M-21: derselbe Lauf offline ────────────────────────────────────
    {
        const auto start = std::chrono::steady_clock::now();
        const auto m = mengeLauf (blk, 3211, true);
        const double sekunden = std::chrono::duration<double> (std::chrono::steady_clock::now() - start).count();
        const double audio = 2.0 * 4.0 * 3.0 * (double) (bloeckeFuer (blk) * blk) / 48000.0;
        pruefe (m.schlecht == 0,
                "312/M-21 offline_gleich_echtzeit_fuer_die_vier (Teilfall von 312/M-20, [SONDE-015] M-120): derselbe Lauf mit "
                "setNonRealtime (true) und ohne Warten zwischen den Bloecken - der Offlineausgang ist fuer die vier "
                "abgedeckten Parameter bitgleich zum Echtzeitausgang desselben Event- und Blockverlaufs",
                m.text + "Geschwindigkeit " + text (audio / std::max (sekunden, 1.0e-9))
                + "-fach Echtzeit (Messung, keine Schranke)");
    }

    // ── 312/M-22: Blockgroessen ──────────────────────────────────────────
    {
        std::ostringstream d;
        bool ok = true;
        // 185: die typische FL-Blockgroesse (identity/host-capabilities-fl-v1.json, Beleg
        // sample_accurate_automation: "~180-190 Samples je Block"); 4096 traegt 312/M-27.
        for (const int groesse : { 1, 64, 185, 240, 256, 512 })
        {
            const auto m = mengeLauf (groesse, 3220 + groesse, false);
            ok = ok && m.schlecht == 0;
#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
            if (groesse == 1)
            {
                ok = ok && m.ziele == 32;
                d << "Blockgroesse 1: " << m.ziele << " Blockrandziele fuer 2 Staende x 4 Parameter x 4 Ereignisse; ";
            }
#endif
            d << "Blockgroesse " << groesse << ": " << m.schlecht << " von 8 Laeufen abweichend; ";
        }
        pruefe (ok,
                "312/M-22 blockbindung_haelt_bei_jeder_blockgroesse (Teilfall von 312/M-20): Blockgroessen 1, 64, 185 (FL), "
                "240, 256 und 512, je im neutralen Stand und mit Band - A ist fuer die vier abgedeckten Parameter bitgleich "
                "zu B; bei Blockgroesse 1 bindet jeder Block genau einen Wert",
                d.str());
    }

    // ── 312/M-23: bewegte Automation und Endwert ──────────────────────────
    {
        const int bloecke = 240;
        const Ereignisse rampe = [] (Prozessor& p, int b)
        {
            if (b < 200)
                hostSchreibt (p, kOutTrim, -12.0f + 24.0f * (float) b / 199.0f);
        };
        auto a = blockPruefling (blk, mitEq (true));
        auto b = blockPruefling (blk, mitEq (true));
        const auto ya = fahreFolge (*a, bloecke, blk, rampe, false, 3231);
        const auto yb = fahreFolge (*b, bloecke, blk, rampe, true, 3231);
        // Endwert: nach dem Lauf ein Takt, dann eingeschwungen messen.
        a->kontrollTaktFuerTest();
        b->kontrollTaktFuerTest();
        const auto va = verhaeltnisNach (*a, 12, blk, 8 * blk, dbFaktor (12.0), 3232);
        const auto vb = verhaeltnisNach (*b, 12, blk, 8 * blk, dbFaktor (12.0), 3232);
        const int ab = abweichend (ya, yb);
        pruefe (ab == 0 && va.endlich && vb.endlich && va.groessteAbweichung <= 1.0e-6 && vb.groessteAbweichung <= 1.0e-6,
                "312/M-23 bewegte_automation_bitgleich_und_endwert_exakt (Teilfall von 312/M-20, Entwurf §44.3): Output-Trim "
                "von -12 nach +12 dB ueber 200 Bloecke mit einem Punkt je Block, danach Halt - A und B sind bitgleich, und "
                "nach dem Ende tragen beide exakt den Endwert +12 dB",
                "A/B " + std::to_string (ab) + " abweichend, Endwert A " + text (va.groessteAbweichung) + ", B "
                + text (vb.groessteAbweichung) + " von 10^(12/20)");
    }

    // ── 312/M-26: echtzeitfest ──────────────────────────────────────────
    {
        auto p = prozessor (48000.0, 512);
        setze (*p, mitEq (true));
        juce::MidiBuffer midi;
        juce::AudioBuffer<float> puffer (2, 512);
        juce::Random w (3260);
        std::mt19937 zufall (3261);
        std::uniform_int_distribution<int> groessen (1, 512);
        dsp::RtWache::zuruecksetzen();
        std::uint64_t eigene = 0;
        for (int blockNr = 0; blockNr < 4000; ++blockNr)
        {
            const int n = groessen (zufall);
            for (const auto& bp : blockParameter())
                hostSchreibt (*p, bp.index, (blockNr % 2) == 0 ? bp.extrem : bp.standard);
            puffer.setSize (2, n, false, false, true);
            for (int k = 0; k < 2; ++k)
                for (int s = 0; s < n; ++s)
                    puffer.setSample (k, s, w.nextFloat() * 1.6f - 0.8f);
            allokationen = 0;
            zaehleAllokationen = true;
            p->processBlock (puffer, midi);
            zaehleAllokationen = false;
            eigene += allokationen;
        }
        const auto sperren = dsp::RtWache::sperren();
        const auto rtAllok = dsp::RtWache::allokationen();
        pruefe (sperren == 0 && rtAllok == 0 && eigene == 0,
                "312/M-26 blockbindung_ist_echtzeitfest (R-312-10 Satz 3, E-312-11): 4000 Bloecke wechselnder Groesse (1 bis 512) "
                "mit Hostautomation auf allen vier abgedeckten Parametern - RtWache::sperren() und RtWache::allokationen() "
                "bleiben ab Callback-Eintritt 0, der eigene Zaehler ebenso",
                "RtWache::sperren " + zahl (sperren) + ", RtWache::allokationen " + zahl (rtAllok) + ", eigener Zaehler " + zahl (eigene));
    }

    // ── 312/M-27: grosse Bloecke, derselbe Block ──────────────────────────
    {
        const int groesse = 4096;
        const int bloecke = 16;
        const auto folge = folgeM20 (kOutTrim, 24.0f, 0.0f);
        auto a = blockPruefling (groesse, mitEq (true));
        auto b = blockPruefling (groesse, mitEq (true));
        const auto ya = fahreFolge (*a, bloecke, groesse, folge, false, 3271);
        const auto yb = fahreFolge (*b, bloecke, groesse, folge, true, 3271);
        const auto ein = eingangVon (bloecke, groesse, 3271);
        const double imBlock10 = letztesVerhaeltnis (ya, ein, 10, groesse);
        const double imBlock9  = letztesVerhaeltnis (ya, ein, 9, groesse);
        const int ab = abweichend (ya, yb);
        pruefe (ab == 0 && std::abs (imBlock10 - dbFaktor (24.0)) <= 1.0e-5 && std::abs (imBlock9 - 1.0) <= 1.0e-6,
                "312/M-27 wert_wirkt_im_block_seines_randes (Teilfall von 312/M-20): Blockgroesse 4096 (85 ms, ueber jedem "
                "Workertakt) - A und B sind bitgleich, und der Wert wirkt in DEMSELBEN Block, an dessen Rand er gelesen "
                "wurde: am Ende von Block 10 steht der Ausgang auf 10^(24/20), am Ende von Block 9 noch auf 1",
                "A/B " + std::to_string (ab) + ", Ende Block 9 " + text (imBlock9) + ", Ende Block 10 " + text (imBlock10));
    }

    // ── 312/M-28: ohne Ereignisse ─────────────────────────────────────────
    {
        const int bloecke = 200;
        auto a = blockPruefling (blk, mitEq (true));
        auto b = blockPruefling (blk, mitEq (true));
        const auto uebernahmenA0 = a->dspKernFuerTest().uebernahmen();
        const auto uebernahmenB0 = b->dspKernFuerTest().uebernahmen();
#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
        const auto zieleA0 = a->dspKernFuerTest().blockrandZiele();
        const auto zieleB0 = b->dspKernFuerTest().blockrandZiele();
#endif
        const auto ya = fahreFolge (*a, bloecke, blk, {}, false, 3281);
        const auto yb = fahreFolge (*b, bloecke, blk, {}, true, 3281);
        bool ok = abweichend (ya, yb) == 0
               && a->dspKernFuerTest().uebernahmen() - uebernahmenA0 == b->dspKernFuerTest().uebernahmen() - uebernahmenB0;
        std::string d = "A/B " + std::to_string (abweichend (ya, yb)) + ", Uebernahmen A +"
                      + zahl (a->dspKernFuerTest().uebernahmen() - uebernahmenA0) + ", B +"
                      + zahl (b->dspKernFuerTest().uebernahmen() - uebernahmenB0);
#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
        const auto zieleA = a->dspKernFuerTest().blockrandZiele() - zieleA0;
        const auto zieleB = b->dspKernFuerTest().blockrandZiele() - zieleB0;
        ok = ok && zieleA == 0 && zieleB == 0;
        d += ", Blockrandziele A +" + zahl (zieleA) + ", B +" + zahl (zieleB);
#endif
        pruefe (ok,
                "312/M-28 ohne_ereignis_setzt_der_blockrand_kein_ziel (Teilfall von 312/M-20, E-312-6): 200 Bloecke ohne "
                "Hostereignis - A und B sind bitgleich, der Blockrand setzt kein Rampenziel, und die Programmuebernahmen "
                "sind in beiden Laeufen gleich",
                d);
    }

    // ── 312/M-29: Ladestart mitten in der Fahrt ────────────────────────────
    {
        auto z = mitEq (true);
        z.werte[(size_t) kOutTrim].zahl = 3.0;
        auto p = blockPruefling (blk, z);
        auto q = blockPruefling (blk, z);
        juce::MemoryBlock bytes;
        p->getStateInformation (bytes);
        const auto hashQuelle = p->stateHashText();
        // Die Fahrt: je Block ein Hostwert auf Output-Trim; nach Block 9 ein
        // Takt, dessen Publikation Block 10 als Rampenuebergang nimmt. Nach
        // Block 10 liegt in P ein alter Wert (-20 dB) in der Mailbox, dann der
        // Ladestart; Q laedt ohne diesen Wert. Ab Block 16 neue Hostwerte.
        const auto fahrt = [&] (Prozessor& x, bool mitAltwert)
        {
            return [&x, mitAltwert, &bytes] (Prozessor&, int b)
            {
                if (b <= 10) hostSchreibt (x, kOutTrim, -6.0f + 0.5f * (float) b);
                if (b == 10) x.kontrollTaktFuerTest();   // vor dem Block: dessen Rand nimmt die Publikation
                if (b == 11)
                {
                    if (mitAltwert) hostSchreibt (x, kOutTrim, -20.0f);
                    x.setStateInformation (bytes.getData(), (int) bytes.getSize());
                }
                if (b == 16) hostSchreibt (x, kOutTrim, 6.0f);
            };
        };
        const int bloecke = 40;
        const auto yp = fahreFolge (*p, bloecke, blk, fahrt (*p, true), false, 3291);
        const auto yq = fahreFolge (*q, bloecke, blk, fahrt (*q, false), false, 3291);
        const auto ein = eingangVon (bloecke, blk, 3291);
        // ab Block 11 (dem Ladestart) muessen P und Q bitgleich sein
        const size_t ab11 = (size_t) 11 * (size_t) blk * 2u;
        const std::vector<float> pNach (yp.begin() + (std::ptrdiff_t) ab11, yp.end());
        const std::vector<float> qNach (yq.begin() + (std::ptrdiff_t) ab11, yq.end());
        const int nachLaden = abweichend (pNach, qNach);
        const double amEnde = letztesVerhaeltnis (yp, ein, bloecke - 1, blk);
        const bool geladen = p->bestaetigterZustand().werte[(size_t) kOutTrim].zahl == 3.0 && p->stateHashText() == hashQuelle;
        pruefe (nachLaden == 0 && std::abs (amEnde - dbFaktor (6.0)) <= 1.0e-5 && geladen,
                "312/M-29 ladestart_und_blockbindung_greifen_ineinander (R-312-10 beide Teile, [SONDE-015] M-84): mitten in "
                "einer Automationsfahrt (ein Rampenuebergang laeuft noch) ein Ladestart in dieselbe Instanz, davor ein alter "
                "Hostwert -20 dB in der Mailbox - ab dem Ladestart ist der Ausgang bitgleich zum Lauf ohne den alten Wert, "
                "ein Hostwert danach (+6 dB) wirkt am naechsten Blockrand, bestaetigter Zustand und Hash sind die der Quelle",
                "nach dem Laden " + std::to_string (nachLaden) + " abweichend, am Ende " + text (amEnde)
                + " (Soll 10^(6/20)), geladen " + (geladen ? "ja" : "NEIN"));
    }

#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
    // ── 312/M-79: die Abdeckung, namentlich ────────────────────────────────
    {
        const auto& tabelle = dsp::DspKern::kBlockrandParameter;
        std::vector<std::string> namen;
        for (const int i : tabelle)
            namen.push_back (str (param::tabelle()[(size_t) i].id));
        const std::vector<std::string> soll { "v1.global.input_trim_db", "v1.global.output_trim_db", "v1.global.width", "v2.global.mix" };
        auto p = blockPruefling (blk, mitEq (true));
        const auto ziele0 = p->dspKernFuerTest().blockrandZiele();
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            for (int i = 0; i < param::kHostParameter; ++i)
                hostParam (*p, i).setValueNotifyingHost (0.75f);
            fahreAudio (*p, 1, blk, 3790);
        });
        const auto ziele = p->dspKernFuerTest().blockrandZiele() - ziele0;
        std::string d = "Tabelle:";
        for (const auto& n : namen) d += " " + n;
        d += "; Blockrandziele nach je einem Ereignis auf allen 112: " + zahl (ziele);
        pruefe (namen == soll && vierBitgleich && ziele == 4,
                "312/M-79 genau_vier_parameter_sind_blockgebunden (E-312-5): die Abdeckungstabelle des Blockrands traegt exakt "
                "input_trim_db, output_trim_db, width und v2.global.mix; fuer diese vier ist A bitgleich zu B (312/M-20), und "
                "nach je einem Hostereignis auf jedem der 112 Hostparameter setzt der Blockrand genau vier Rampenziele",
                d);
    }
#endif

    // ── 312/M-80 und 312/M-81: der taktgebundene Rest ──────────────────────
    {
        // Slot 5 ist im Stand BELEGT (Bell 1 kHz): auf einem freien Slot
        // aenderte gain_db den Klang nicht ([SONDE-015] M-63).
        auto z = mitEq (true);
        setzeBand (z, 5, 1000.0, 0.0);
        const int gainIndex = iBand (5, param::kGainDb);
        const Ereignisse gainFolge = [gainIndex] (Prozessor& p, int b)
        {
            if (b == 10 || b == 12) hostSchreibt (p, gainIndex, 12.0f);
            if (b == 11) hostSchreibt (p, gainIndex, 0.0f);
            if (b == 13) hostSchreibt (p, gainIndex, -6.0f);
        };
        const Ereignisse eqFolge = [] (Prozessor& p, int b)
        {
            if (b == 10 || b == 12) hostSchreibt (p, param::kIndexEqEnabled, 0.0f);
            if (b == 11 || b == 13) hostSchreibt (p, param::kIndexEqEnabled, 1.0f);
        };
        const int bloecke = bloeckeFuer (blk);
        const auto laeufe = [&] (const param::DspSatz& stand, const Ereignisse& folge, int saat,
                                 int& ohneTakt, int& mitTakt, param::DspSatz& endeA, param::DspSatz& endeB)
        {
            auto a  = blockPruefling (blk, stand);
            auto a2 = blockPruefling (blk, stand);
            auto b  = blockPruefling (blk, stand);
            const auto ya  = fahreFolge (*a,  bloecke, blk, folge, false, saat);
            const auto ya2 = fahreFolge (*a2, bloecke, blk, folge, true,  saat);
            const auto yb  = fahreFolge (*b,  bloecke, blk, folge, true,  saat);
            ohneTakt = abweichend (ya, yb);
            mitTakt  = abweichend (ya2, yb);
            a->kontrollTaktFuerTest();
            b->kontrollTaktFuerTest();
            endeA = a->wirksamerZustand();
            endeB = b->wirksamerZustand();
        };
        int ohneTakt = 0, mitTakt = 0;
        param::DspSatz endeA, endeB;
        laeufe (z, gainFolge, 3801, ohneTakt, mitTakt, endeA, endeB);
        bool menge = true;
#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
        for (const int i : dsp::DspKern::kBlockrandParameter)
            if (i == gainIndex) menge = false;
#endif
        pruefe (menge && ohneTakt > 0 && mitTakt == 0 && gain (endeA, 5) == -6.0 && gain (endeB, 5) == -6.0,
                "312/M-80 bandwert_bleibt_taktgebunden (T3-01-05 Teil b, E-312-5, R-312-15): v1.band.5.gain_db steht NICHT in "
                "der Abdeckungstabelle des Blockrands; unter der Taktsperre weicht A von B ab, mit Kontrolltakt nach jedem "
                "Block sind beide bitgleich, und der Endwert ist in beiden Laeufen exakt der letzte gesetzte (-6 dB). Keine "
                "Gleichheitszusage fuer Bandwerte - Teil b geht an NAK-340",
                "ohne Takt " + std::to_string (ohneTakt) + " abweichend, mit Takt " + std::to_string (mitTakt)
                + ", Endwert A " + text (gain (endeA, 5)) + " / B " + text (gain (endeB, 5)) + " dB");

        // Mit Band (Bell 1 kHz +6 dB): im neutralen Stand klaengen An und Aus gleich.
        auto zEq = mitEq (true);
        setzeBand (zEq, 0, 1000.0, 6.0);
        laeufe (zEq, eqFolge, 3811, ohneTakt, mitTakt, endeA, endeB);
        bool mengeEq = true;
#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
        for (const int i : dsp::DspKern::kBlockrandParameter)
            if (i == param::kIndexEqEnabled) mengeEq = false;
#endif
        const bool endeEq = endeA.werte[(size_t) param::kIndexEqEnabled].b && endeB.werte[(size_t) param::kIndexEqEnabled].b;
        pruefe (mengeEq, "312/M-81 Mengenhaelfte eq_enabled_steht_nicht_in_der_abdeckung (T3-01-05 Teil b, E-312-12): "
                "v2.global.eq_enabled steht NICHT in der Abdeckungstabelle des Blockrands");
        pruefe (ohneTakt > 0 && mitTakt == 0 && endeEq,
                "312/M-81 Verhaltenshaelfte schalter_bleibt_taktgebunden (messend, E-312-12): eq_enabled je Block umgeschaltet - "
                "unter der Taktsperre liegt der Umschaltblock von A hinter dem von B, mit Kontrolltakt sind beide bitgleich, "
                "der Endzustand stimmt in beiden Laeufen. Keine Gleichheitszusage fuer die elf booleschen Hostparameter",
                "ohne Takt " + std::to_string (ohneTakt) + " abweichend, mit Takt " + std::to_string (mitTakt));
    }

#if ! defined (NAK312_GEGENPROBE_OHNE_BLOCKRAND)
    // ── 312/M-82: der Zaehlerrand der Schiedsregel ─────────────────────────
    {
        const int bloecke = 30;
        const Ereignisse folge = [] (Prozessor& p, int b)
        {
            if (b == 10) hostSchreibt (p, kOutTrim, 6.0f);     // Zaehler 0xFFFFFFFF
            if (b == 12) hostSchreibt (p, kOutTrim, -6.0f);    // Zaehler 0: der Ueberlauf
            if (b == 14) hostSchreibt (p, kOutTrim, 12.0f);    // Zaehler 1
        };
        auto a = blockPruefling (blk, mitEq (true));
        auto b = blockPruefling (blk, mitEq (true));
        a->setzeHostZaehlerFuerTest (kOutTrim, 0xFFFFFFFEu);
        b->setzeHostZaehlerFuerTest (kOutTrim, 0xFFFFFFFEu);
        const auto ziele0 = a->dspKernFuerTest().blockrandZiele();
        std::vector<double> nachEreignis;
        const auto ya = fahreFolge (*a, bloecke, blk, folge, false, 3821);
        const auto yb = fahreFolge (*b, bloecke, blk, folge, true, 3821);
        const auto ein = eingangVon (bloecke, blk, 3821);
        const auto ziele = a->dspKernFuerTest().blockrandZiele() - ziele0;
        // Rampe 256 Samples = 4 Bloecke: am Ende des jeweils uebernaechsten
        // Ereignisblocks ist der Wert erreicht
        const double nach10 = letztesVerhaeltnis (ya, ein, 11, blk);
        const double nach12 = letztesVerhaeltnis (ya, ein, 13, blk);
        const double amEnde = letztesVerhaeltnis (ya, ein, bloecke - 1, blk);
        const int ab = abweichend (ya, yb);
        pruefe (ab == 0 && ziele == 3 && std::abs (amEnde - dbFaktor (12.0)) <= 1.0e-5
                    && nach12 < nach10,
                "312/M-82 schiedsregel_ist_ueberlaufsicher (E-312-6): Ereigniszaehler und Blockrandstand bei 0xFFFFFFFE, drei "
                "Hostereignisse auf Output-Trim ueber den Ueberlauf hinweg - der Blockrand vergleicht auf Ungleichheit: jedes "
                "Ereignis setzt genau ein Rampenziel (3), das nach dem Ueberlauf wirkt, A ist bitgleich zu B, am Ende +12 dB",
                "A/B " + std::to_string (ab) + ", Blockrandziele " + zahl (ziele) + ", Verhaeltnis nach Block 11 " + text (nach10)
                + ", nach Block 13 " + text (nach12) + ", am Ende " + text (amEnde));
    }

    // ── 312/M-87: nach einem read-only-Ladestart kein Blockrandziel ────────
    // Nacharbeit 1 der Etappe 3 (L-1, E-312-13, R-312-16). A und C tragen
    // denselben eingeschwungenen Stand (eq an, Output-Trim +3 dB committet) und
    // laden dasselbe fremde Major; nur A bekommt danach Hostwerte auf die vier
    // Parameter der Abdeckungstabelle. Im ersten Block nach dem Laden ist die
    // Bank des vorigen Standes noch aktiv und blendet aus - ab dem ersten
    // Sample dieses Blocks muss A bitgleich zu C sein, und der Blockrand von A
    // setzt kein Rampenziel. Jeder Lauf steht ganz unter EINER Taktsperre ohne
    // Kontrolltakt: der Worker zieht weder zwischen Laden und Hostwerten noch
    // vor dem ersten Block.
    {
        juce::MemoryBlock fremd;
        const bool fremdGelesen = wurzel().getChildFile ("eq-copilot/fixtures/state/schema2/fremdes-major-3.bin").loadFileAsData (fremd);
        auto plusDrei = mitEq (true);
        plusDrei.werte[(size_t) kOutTrim].zahl = 3.0;

        using Hostwerte = std::function<void (Prozessor&)>;
        const Hostwerte keine;
        const Hostwerte vier = [] (Prozessor& p)
        {
            // Input-Trim +24 dB, Output-Trim +24 dB, Width 2, Mix 0
            for (const auto& bp : blockParameter())
                hostSchreibt (p, bp.index, bp.extrem);
        };
        const Hostwerte nurAusgang = [] (Prozessor& p) { hostSchreibt (p, kOutTrim, 24.0f); };

        struct Lauf
        {
            std::vector<float> aus;
            bool          commit = true, uebergangLief = false, lesend = false;
            std::uint64_t ziele = 0;
        };
        /** Bei `mitUebergang` zuerst ein Commit Output-Trim +6 dB und genau ein
            Block zu 64 - der Uebergang dieses Commits laeuft beim Laden noch -,
            dann das fremde Major, die Hostwerte und `bloecke` Bloecke zu
            `groesse`. `aus` beginnt mit dem ersten Block nach dem Laden. */
        const auto lauf = [&fremd, &plusDrei] (Prozessor& p, bool mitUebergang, const Hostwerte& hostwerte,
                                               int groesse, int bloecke, int saat)
        {
            Lauf l;
            p.mitAngehaltenemTaktFuerTest ([&]
            {
                if (mitUebergang)
                {
                    auto plusSechs = plusDrei;
                    plusSechs.werte[(size_t) kOutTrim].zahl = 6.0;
                    l.commit = setze (p, plusSechs).ausgang == tx::Ausgang::commit;
                    fahreAudio (p, 1, 64, saat + 1);
                }
                int aktiv = -1, quelle = -1, candidate = -1, candidateQuelle = -1;
                p.dspKernFuerTest().gefahreneSlots (aktiv, quelle, candidate, candidateQuelle);
                l.uebergangLief = quelle >= 0;
                const auto ziele0 = p.dspKernFuerTest().blockrandZiele();
                p.setStateInformation (fremd.getData(), (int) fremd.getSize());
                l.lesend = p.zustandLesen().nurLesen;
                if (hostwerte)
                    hostwerte (p);
                l.aus   = fahreFolge (p, bloecke, groesse, {}, false, saat);   // die Taktsperre ist rekursiv
                l.ziele = p.dspKernFuerTest().blockrandZiele() - ziele0;
            });
            return l;
        };

        // Hauptfall: Bloecke zu 256, der Pfad steht beim Laden.
        {
            const int groesse = 256, bloecke = 4;
            auto a  = blockPruefling (groesse, plusDrei);
            auto a1 = blockPruefling (groesse, plusDrei);
            auto c  = blockPruefling (groesse, plusDrei);
            const auto la  = lauf (*a,  false, vier,       groesse, bloecke, 3871);
            const auto la1 = lauf (*a1, false, nurAusgang, groesse, bloecke, 3871);
            const auto lc  = lauf (*c,  false, keine,      groesse, bloecke, 3871);
            const auto bild = abweichungsbild (la.aus, lc.aus, groesse);
            const auto ein  = eingangVon (bloecke, groesse, 3871);
            pruefe (fremdGelesen && la.lesend && lc.lesend && bild.links == 0 && bild.rechts == 0 && la.ziele == 0,
                    "312/M-87 read_only_ladestart_nimmt_kein_blockrandziel (L-1, R-312-16, R-312-10 zweiter Teil, [SONDE-015] "
                    "M-92, Gate T3-05-02): A und C je eq an und Output-Trim +3 dB committet und eingeschwungen, unter der "
                    "Taktsperre in beiden das fremde Major geladen, nur in A danach Input-Trim +24 dB, Output-Trim +24 dB, "
                    "Width 2 und Mix 0 als Hostwerte, dann dieselben Bloecke zu 256 mit fester Saat (1024 Samples je Kanal) - "
                    "A ist ab dem ersten Sample des ersten Blocks nach dem Laden bitgleich zu C, und die Blockrandziele von A "
                    "steigen um 0",
                    "A gegen C " + beschreibe (bild) + ", Blockrandziele A +" + zahl (la.ziele) + ", read-only A "
                    + (la.lesend ? "ja" : "NEIN") + " / C " + (lc.lesend ? "ja" : "NEIN")
                    + "; Messung ohne Bedingung, nur Output-Trim +24 dB in einer dritten Instanz: groesstes Ausgang/Eingang "
                    + text (groessterFaktorDb (la1.aus, ein)) + " dB, Blockrandziele +" + zahl (la1.ziele) + " (C "
                    + text (groessterFaktorDb (lc.aus, ein)) + " dB)");
        }

        // Teilfall (i): beim Laden laeuft ein Uebergang, danach Bloecke zu 185
        // (FL) und zu 512.
        {
            bool ok = fremdGelesen;
            std::ostringstream d;
            for (const int groesse : { 185, 512 })
            {
                const int bloecke = std::max (4, (2048 + groesse - 1) / groesse);
                auto a = blockPruefling (512, plusDrei);
                auto c = blockPruefling (512, plusDrei);
                const auto la = lauf (*a, true, vier,  groesse, bloecke, 3872 + groesse);
                const auto lc = lauf (*c, true, keine, groesse, bloecke, 3872 + groesse);
                const auto bild = abweichungsbild (la.aus, lc.aus, groesse);
                ok = ok && la.commit && lc.commit && la.uebergangLief && lc.uebergangLief && la.lesend
                        && bild.links == 0 && bild.rechts == 0 && la.ziele == 0;
                d << "Bloecke zu " << groesse << ": Uebergang beim Laden " << (la.uebergangLief ? "lief" : "lief NICHT")
                  << ", A gegen C " << beschreibe (bild) << ", Blockrandziele A +" << la.ziele << "; ";
            }
            pruefe (ok,
                    "312/M-87 Teilfall (i) uebergang_beim_laden (R-312-16): vorher ein Commit Output-Trim +6 dB und genau ein "
                    "Block zu 64 - beim Laden laeuft dessen Uebergang noch -, danach Bloecke zu 185 und zu 512 - dieselbe "
                    "Zusage: A ist ab dem ersten Sample nach dem Laden bitgleich zu C, und die Blockrandziele von A steigen um 0",
                    d.str());
        }

        // Teilfall (ii): der Rueckweg in einen schreibbaren Stand, OHNE
        // Kontrolltakt - dort wirkt der Hostwert nur ueber den Blockrand.
        {
            auto quelle = prozessor (48000.0, 64);
            setze (*quelle, mitEq (true));
            juce::MemoryBlock eigen;
            quelle->getStateInformation (eigen);
            const int groesse = 64, bloecke = 24;
            const auto folge = [&fremd, &eigen] (bool* lesend, bool* schreibbar) -> Ereignisse
            {
                return [&fremd, &eigen, lesend, schreibbar] (Prozessor& x, int b)
                {
                    if (b == 0)
                    {
                        x.setStateInformation (fremd.getData(), (int) fremd.getSize());
                        *lesend = x.zustandLesen().nurLesen;
                        x.setStateInformation (eigen.getData(), (int) eigen.getSize());
                        *schreibbar = ! x.zustandLesen().nurLesen;
                    }
                    if (b == 10) hostSchreibt (x, kOutTrim, 6.0f);
                };
            };
            bool lesendA = false, schreibbarA = false, lesendB = false, schreibbarB = false;
            auto a = prozessor (48000.0, groesse);
            auto b = prozessor (48000.0, groesse);
            const auto ya  = fahreFolge (*a, bloecke, groesse, folge (&lesendA, &schreibbarA), false, 3879);
            const auto yb  = fahreFolge (*b, bloecke, groesse, folge (&lesendB, &schreibbarB), true,  3879);
            const auto ein = eingangVon (bloecke, groesse, 3879);
            const int    ab     = abweichend (ya, yb);
            const double amEnde = letztesVerhaeltnis (ya, ein, bloecke - 1, groesse);
            pruefe (fremdGelesen && lesendA && schreibbarA && lesendB && schreibbarB && ab == 0
                        && std::abs (amEnde - dbFaktor (6.0)) <= 1.0e-5,
                    "312/M-87 Teilfall (ii) rueckweg_ohne_kontrolltakt (R-312-16 Schluss): erst das fremde Major, dann ein "
                    "eigener schreibbarer Stand mit eq an, unter der Taktsperre OHNE Kontrolltakt Output-Trim +6 dB vor Block 10 "
                    "- A ist bitgleich zu einem Lauf mit Takt nach jedem Block, und Ausgang/Eingang steht am Ende bei 10^(6/20)",
                    std::string ("zwischendurch read-only ") + (lesendA ? "ja" : "NEIN") + ", danach schreibbar "
                    + (schreibbarA ? "ja" : "NEIN") + ", A/B " + std::to_string (ab) + " abweichend, am Ende " + text (amEnde)
                    + " (Soll 10^(6/20))");
        }
    }
#endif

    // ── Kosten der Blockbindung (Messung, keine Schranke; Manifest §7.3) ───
    {
        auto p = blockPruefling (blk, mitEq (true));
        const int n = 20000;
        // Gemessen wird allein die Zeit IN processBlock (die Ereignisse schreibt
        // der Test davor, ungemessen). Das Ereignis traegt den bestaetigten
        // Wert: der Blockrand laeuft den ganzen Lesepfad (Zaehler, Quittung,
        // Hostwert, Vergleichswert), und das Rampenziel bleibt, damit die
        // Audioarbeit beider Laeufe gleich ist. Je Lauf das Minimum aus drei
        // Durchgaengen.
        const auto messe = [&] (bool mitEreignis)
        {
            juce::MidiBuffer midi;
            juce::AudioBuffer<float> puffer (2, blk);
            juce::Random w (3290);
            for (int k = 0; k < 2; ++k)
                for (int s = 0; s < blk; ++s)
                    puffer.setSample (k, s, w.nextFloat() * 0.2f - 0.1f);
            double bestes = std::numeric_limits<double>::infinity();
            for (int durchgang = 0; durchgang < 3; ++durchgang)
            {
                double sekunden = 0.0;
                p->mitAngehaltenemTaktFuerTest ([&]
                {
                    for (int i = 0; i < n; ++i)
                    {
                        if (mitEreignis)
                            for (const auto& bp : blockParameter())
                                hostSchreibt (*p, bp.index, bp.standard);
                        const auto start = std::chrono::steady_clock::now();
                        p->processBlock (puffer, midi);
                        sekunden += std::chrono::duration<double> (std::chrono::steady_clock::now() - start).count();
                    }
                });
                bestes = std::min (bestes, sekunden * 1.0e9 / (double) n);
            }
            return bestes;
        };
        const double ohne = messe (false);
        const double mit  = messe (true);
        std::cout << "  MESSUNG 312/Kosten sizeof(dsp::DspKern) " << sizeof (dsp::DspKern)
                  << " Byte, sizeof(SondeProcessor) " << sizeof (Prozessor) << " Byte; " << n
                  << " Bloecke zu 64 Samples, Zeit in processBlock (Minimum aus drei Durchgaengen): ohne Hostereignis "
                  << text (ohne) << " ns je Block, mit je einem Ereignis auf den vier abgedeckten Parametern "
                  << text (mit) << " ns je Block" << std::endl;
    }
}

//==============================================================================
// NAK-312 Etappe 4 (W09, R-312-7, E-312-11): die Besitzschleuse liegt nicht im
// Audiopfad.
//
// Die Provider der v3-Clients laufen auf den Clientthreads durch die
// Besitzschleuse der Sonde; die Echtzeitwache zaehlt nur auf dem Thread, der
// im Bereich von `processBlock` steht (`DspRtWache.h`). 312/M-38 faehrt deshalb
// 4000 und mehr Bloecke, WAEHREND beide Clients ueber einen Testserver auf der
// Probe-Pipe verbunden sind und die Provider nachweislich durch die Schleuse
// laufen (`betreten` steigt im Messfenster). Gemessen wird wie in 312/M-01.
// Die Lebendpruefung des Gen-Panels liegt im Editor (Nachrichtenthread) und
// kann dieses Ziel nicht bauen.

namespace m38
{
using namespace nakama::ipc;
#include "V3TestServer.h"
}

void nak312Besitz()
{
    abschnitt ("Z - NAK-312 Etappe 4: die Besitzschleuse liegt nicht im Audiopfad (312/M-38)");

    const auto pipe = m38::testPipeName ("nak312-m38");
    if (! nakama::ipc::istProbePipename (pipe))
    {
        pruefe (false, "312/M-38: der Testserver liegt im Probe-Namensraum", pipe);
        return;
    }
    m38::TestServer server (pipe);
    if (! server.starten())
    {
        pruefe (false, "312/M-38: der Testserver steht auf der Probe-Pipe");
        return;
    }

    auto p = std::make_unique<Prozessor> (pipe, m38::testExeErwartung());
    {
        // Bindung und erlaubter Messpunkt, sonst weist der Client sein eigenes
        // Hello ab; der Stand traegt Parameter, der Status also Hash und
        // Messpunkt als Zeichenketten.
        auto z = state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        z.common.klasse = state::Klasse::active_probe;
        z.common.position = state::Messposition::insert;
        z.common.projectBindingId = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
        z.hatParameters = true;
        juce::MemoryBlock bytes;
        state::speichere (z, bytes);
        p->setStateInformation (bytes.getData(), (int) bytes.getSize());
    }
    p->setRateAndBufferSizeDetails (48000.0, 512);
    p->prepareToPlay (48000.0, 512);
    setze (*p, mitEq (true));
    const auto schleuse = p->callbackSchleuseFuerTest();

    const auto warte = [] (int ms, const std::function<bool()>& bis)
    {
        for (int i = 0; i < ms / 5; ++i)
        {
            if (bis())
                return true;
            std::this_thread::sleep_for (std::chrono::milliseconds (5));
        }
        return bis();
    };
    p->v3StartFuerTest();
    const bool control = warte (8000, [&] {
        return p->controlV3FuerTest().status == nakama::ipc::ControlClient::Status::verbunden;
    });
    p->v3TelemetrieStartFuerTest();
    const bool tele = warte (8000, [&] {
        return p->telemetryV3FuerTest().status == nakama::ipc::TelemetryClient::Status::verbunden;
    });

    juce::MidiBuffer midi;
    juce::AudioBuffer<float> puffer (2, 512);
    juce::Random w (3380);
    std::mt19937 zufall (3381);
    std::uniform_int_distribution<int> groessen (1, 512);
    const auto betretenVor = schleuse->stand().betreten;
    const auto beginn = std::chrono::steady_clock::now();
    dsp::RtWache::zuruecksetzen();
    std::uint64_t eigene = 0;
    int bloecke = 0;
    // Mindestens 4000 Bloecke, und so lange weiter, bis ein Provider im
    // Messfenster durch die Schleuse gelaufen ist (Heartbeat-Takt 1 s). Die
    // 10 s sind nur eine Obergrenze gegen einen haengenden Lauf.
    while (bloecke < 4000
           || (schleuse->stand().betreten == betretenVor
               && std::chrono::steady_clock::now() - beginn < std::chrono::seconds (10)))
    {
        const int n = groessen (zufall);
        puffer.setSize (2, n, false, false, true);
        for (int k = 0; k < 2; ++k)
            for (int s = 0; s < n; ++s)
                puffer.setSample (k, s, w.nextFloat() * 1.6f - 0.8f);
        allokationen = 0;
        zaehleAllokationen = true;
        p->processBlock (puffer, midi);
        zaehleAllokationen = false;
        eigene += allokationen;
        ++bloecke;
    }
    const auto sperren = dsp::RtWache::sperren();
    const auto rtAllok = dsp::RtWache::allokationen();
    const auto stand   = schleuse->stand();
    const bool nochVerbunden = p->controlV3FuerTest().status == nakama::ipc::ControlClient::Status::verbunden;
    pruefe (control && tele && nochVerbunden && bloecke >= 4000 && stand.betreten > betretenVor
                && ! stand.geschlossen && sperren == 0 && rtAllok == 0 && eigene == 0,
            "312/M-38 besitzschleuse_liegt_nicht_im_audiopfad (W09, Grundgesetz, E-312-11): mindestens 4000 Bloecke "
            "wechselnder Groesse, waehrend beide v3-Clients ueber die Probe-Pipe verbunden sind und ihre Provider "
            "durch die Besitzschleuse laufen - RtWache::sperren() und RtWache::allokationen() bleiben ab "
            "Callback-Eintritt 0, der eigene Zaehler ebenso",
            zahl ((std::uint64_t) bloecke) + " Bloecke, Schleuse betreten " + zahl (betretenVor) + " -> "
                + zahl (stand.betreten) + " im Messfenster, Control verbunden " + (nochVerbunden ? "ja" : "NEIN")
                + ", RtWache::sperren " + zahl (sperren) + ", RtWache::allokationen " + zahl (rtAllok)
                + ", eigener Zaehler " + zahl (eigene));

    p.reset();
    server.stoppen();
}

//==============================================================================
// NAK-312 Etappe 5, erster Aenderungssatz (T3-01-03, T3-01-04; R-312-3,
// E-312-7): der Offline-Uebergang. `setNonRealtime` erkennt den Wechsel an
// einem Atomic, nimmt keine Sperre, schaltet die Hoermatrix hart auf Processed
// und setzt den Offline-Riegel; die Vorschau verbucht der naechste
// Kontrolltakt. Keine Zeile haengt an der Wanduhr: `previewAktiv()` und die
// Bankfreigabe werden nur nach einem ausdruecklichen `kontrollTaktFuerTest()`
// gefragt (der Worker darf frueher ziehen, `beendePreview` ist idempotent).

/** Der Aufbau von 312/M-40 bis 312/M-47: A traegt den bestaetigten Satz (eq an,
    Bell 1 kHz +6 dB) und - mit `mitVorschau` - eine Vorschau (-12 dB) mit dem
    Hoerwunsch `wunsch`; B nur den bestaetigten Satz; C (mit `mitC`) ist ein
    Zwilling von A, der nie offline geht. Alle drei schwingen mit demselben
    Eingang ein; `ya`, `yb`, `yc` sind die Ausgaenge des Einschwingens. */
struct OfflineAufbau
{
    std::unique_ptr<Prozessor> a, b, c;
    std::vector<float> ya, yb, yc;
};

OfflineAufbau offlineAufbau (int block, bool mitVorschau = true, bool mitC = false,
                             dsp::Hoermatrix wunsch = dsp::Hoermatrix::candidate, int einschwingSamples = 2048)
{
    OfflineAufbau x;
    x.a = prozessor (48000.0, block);
    x.b = prozessor (48000.0, block);
    if (mitC) x.c = prozessor (48000.0, block);
    auto z = mitEq (true);
    setzeBand (z, 0, 1000.0, 6.0);
    auto vorschau = z;
    vorschau.werte[(size_t) iBand (0, param::kGainDb)].zahl = -12.0;
    for (auto* p : { x.a.get(), x.b.get(), x.c.get() })
    {
        if (p == nullptr) continue;
        setze (*p, z);
        if (mitVorschau && p != x.b.get())
        {
            juce::String g;
            p->setzePreview (vorschau, g);
            p->setzeHoermatrix (wunsch);
        }
    }
    const int bloecke = std::max (1, einschwingSamples / block);
    x.ya = fahreAudio (*x.a, bloecke, block, 60);
    x.yb = fahreAudio (*x.b, bloecke, block, 60);
    if (x.c != nullptr) x.yc = fahreAudio (*x.c, bloecke, block, 60);
    return x;
}

/** Faehrt `bloecke` Bloecke mit dem Gleichanteil `dc` auf beiden Kanaelen und
    liefert den Ausgang des linken Kanals. */
std::vector<float> fahreGleichanteil (Prozessor& p, int bloecke, int groesse, float dc)
{
    std::vector<float> aus;
    aus.reserve ((size_t) (bloecke * groesse));
    juce::MidiBuffer midi;
    for (int b = 0; b < bloecke; ++b)
    {
        juce::AudioBuffer<float> puffer (2, groesse);
        for (int k = 0; k < 2; ++k)
            for (int n = 0; n < groesse; ++n)
                puffer.setSample (k, n, dc);
        p.processBlock (puffer, midi);
        for (int n = 0; n < groesse; ++n)
            aus.push_back (puffer.getSample (0, n));
    }
    return aus;
}

int candidateSlot (Prozessor& p)
{
    int c = -1, cq = -1, k = -1, kq = -1;
    p.dspKernFuerTest().gefahreneSlots (c, cq, k, kq);
    return k;
}

void nak312OfflineUebergang()
{
    abschnitt ("ZA - NAK-312 Etappe 5: der Offline-Uebergang (312/M-40, 312/M-41, 312/M-83, 312/M-43 bis 312/M-47; "
               "312/M-42 in Abschnitt O, 312/M-04 in Abschnitt W; Zusaetze der Etappe 6a: 312/M-89, 312/M-90)");

    const int fade = nakama::dsp::kFadeSamples;

    // ── 312/M-40 und 312/M-41: 200 Bloecke, das Bein oeffnet den Bereich wie
    // der Wrapper um `setNonRealtime (true)` und `processBlock` (312/M-04). ──
    {
        auto x = offlineAufbau (256);
        const bool candidateHoerbar = x.a->dspKernFuerTest().wirksameHoermatrix() == dsp::Hoermatrix::candidate
                                   && ! bitgleich (x.ya, x.yb);
        x.b->setNonRealtime (true);
        juce::AudioBuffer<float> pa (2, 256), pb (2, 256);
        juce::MidiBuffer midi;
        juce::Random w (3140);
        bool gewuenschtSofort = false, wirksamNachErstemBlock = false, ersterBitgleich = false, alleBitgleich = true;
        dsp::RtWache::zuruecksetzen();
        // NAK-312 Etappe 6a (R-312-22, H-2): der Allokationszaehler ist ueber
        // setNonRealtime und processBlock scharf - er meldet an die
        // Echtzeitwache, und die zaehlt im Bereich wie der Wrapper.
        std::uint64_t eigene = 0;
        for (int blockNr = 0; blockNr < 200; ++blockNr)
        {
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < 256; ++n)
                {
                    const float s = w.nextFloat() * 1.6f - 0.8f;
                    pa.setSample (k, n, s);
                    pb.setSample (k, n, s);
                }
            allokationen = 0;
            zaehleAllokationen = true;
            {
                const dsp::RtWache::Bereich wieDerWrapper;
                x.a->setNonRealtime (true);
                if (blockNr == 0)
                    gewuenschtSofort = x.a->gewuenschteHoermatrix() == dsp::Hoermatrix::processed;
                x.a->processBlock (pa, midi);
            }
            zaehleAllokationen = false;
            eigene += allokationen;
            x.b->processBlock (pb, midi);
            const bool gleich = std::memcmp (pa.getReadPointer (0), pb.getReadPointer (0), 256 * sizeof (float)) == 0
                             && std::memcmp (pa.getReadPointer (1), pb.getReadPointer (1), 256 * sizeof (float)) == 0;
            if (blockNr == 0)
            {
                ersterBitgleich = gleich;
                wirksamNachErstemBlock = x.a->dspKernFuerTest().wirksameHoermatrix() == dsp::Hoermatrix::processed;
            }
            alleBitgleich = alleBitgleich && gleich;
        }
        const auto sperren = dsp::RtWache::sperren();
        const auto rtAllok = dsp::RtWache::allokationen();
        pruefe (candidateHoerbar && sperren == 0 && rtAllok == 0 && eigene == 0,
                "312/M-40 offlineflag_nimmt_keine_sperre (T3-01-03, R-312-3, R-312-22, [SONDE-015] M-47): echter "
                "SondeProcessor mit hoerbarer Vorschau (Hoerwunsch Candidate); legt das Bein den Bereich wie der Wrapper um "
                "setNonRealtime (true) und processBlock, zaehlen RtWache::sperren() und RtWache::allokationen() ueber 200 "
                "Bloecke 0",
                "Candidate vorher hoerbar " + std::string (candidateHoerbar ? "ja" : "NEIN") + ", RtWache::sperren " + zahl (sperren)
                    + ", RtWache::allokationen " + zahl (rtAllok) + ", eigener Zaehler " + zahl (eigene));
        pruefe (candidateHoerbar && gewuenschtSofort && wirksamNachErstemBlock,
                "312/M-41 der_erste_aufruf_stellt_die_hoermatrix_sofort_um (Teilfall von 312/M-40, E-312-7 Hoermatrixhaelfte): "
                "unmittelbar nach der Rueckkehr des ersten setNonRealtime (true) ist gewuenschteHoermatrix() Processed, "
                "und im ersten Block danach wirkt Processed",
                std::string ("gewuenscht sofort ") + (gewuenschtSofort ? "processed" : "NICHT processed")
                + ", wirksam nach dem ersten Block " + (wirksamNachErstemBlock ? "processed" : "NICHT processed"));
        pruefe (candidateHoerbar && ersterBitgleich && alleBitgleich,
                "  312/M-42 im Bereich wie der Wrapper: der erste Block nach dem Flag ist ab Sample 0 bitgleich zum "
                "bestaetigten Lauf, und alle 200 Bloecke sind es",
                std::string ("erster Block ") + (ersterBitgleich ? "bitgleich" : "VERSCHIEDEN")
                + ", alle 200 " + (alleBitgleich ? "bitgleich" : "NICHT bitgleich"));
    }

    // ── 312/M-83 und 312/M-46: die Buchhaltung folgt im Kontrolltakt, und keine
    // Bank bleibt haengen - gemessen nach ausdruecklichen Takten. ────────────
    {
        auto x = offlineAufbau (256);
        const int slot = candidateSlot (*x.a);
        x.a->setNonRealtime (true);
        x.a->kontrollTaktFuerTest();
        const bool verbucht = ! x.a->previewAktiv();
        const int bloecke = (fade + 256 - 1) / 256 + 1;
        fahreAudio (*x.a, bloecke, 256, 61);
        x.a->kontrollTaktFuerTest();
        int c = -1, cq = -1, k = -1, kq = -1;
        x.a->dspKernFuerTest().gefahreneSlots (c, cq, k, kq);
        const bool frei = slot >= 0 && x.a->dspKernFuerTest().pool().zustand (slot) == dsp::BankZustand::frei;
        const std::string detail = "Candidate-Slot " + std::to_string (slot) + ", previewAktiv nach dem ersten Takt "
                                 + (verbucht ? "false" : "TRUE") + ", nach " + std::to_string (bloecke)
                                 + " Bloecken und dem zweiten Takt Slot " + (frei ? "frei" : "BELEGT")
                                 + ", Candidate aktiv/Quelle " + std::to_string (k) + "/" + std::to_string (kq);
        pruefe (slot >= 0 && verbucht && frei,
                "312/M-83 vorschauende_wird_im_kontrolltakt_verbucht (E-312-7 Buchhaltungshaelfte): nach dem ersten "
                "setNonRealtime (true) und einem ausdruecklichen kontrollTaktFuerTest() ist previewAktiv() false; nach "
                "ceil (kFadeSamples / Blockgroesse) + 1 Bloecken und einem zweiten Takt ist die Candidate-Bank frei",
                detail);
        pruefe (slot >= 0 && frei && k == -1 && kq == -1 && ! x.a->dspKernFuerTest().candidateVorhanden(),
                "312/M-46 keine_bank_bleibt_haengen (Teilfall von 312/M-42, E-312-7, [SONDE-015] E-33): dieselbe Folge - "
                "die Candidate-Bank ist frei, der Candidate-Pfad ruht, auch wenn die Hoermatrix nie blendete",
                detail);
    }

    // ── 312/M-46 Teilfall: beim Schalten laeuft ein HOERHALT (die Vorschau
    // endete in Echtzeit, die Hoermatrix blendet von Candidate zurueck). Das
    // harte Schalten beendet den Fade, der Hoerhalt endet am Ende desselben
    // Stuecks, der naechste Takt gibt die Bank frei. ─────────────────────────
    {
        auto x = offlineAufbau (64);
        const int slot = candidateSlot (*x.a);
        x.a->beendePreview();
        fahreAudio (*x.a, 1, 64, 62); fahreAudio (*x.b, 1, 64, 62);   // 64 von 256 Samples der Rueckblende
        int c = -1, cq = -1, k = -1, kq = -1;
        x.a->dspKernFuerTest().gefahreneSlots (c, cq, k, kq);
        const bool hoerHaltLaeuft = slot >= 0 && k == -1 && kq == slot;
        x.a->setNonRealtime (true); x.b->setNonRealtime (true);
        x.a->kontrollTaktFuerTest();
        const auto ya = fahreAudio (*x.a, (fade + 64 - 1) / 64 + 1, 64, 63);
        const auto yb = fahreAudio (*x.b, (fade + 64 - 1) / 64 + 1, 64, 63);
        x.a->kontrollTaktFuerTest();
        x.a->dspKernFuerTest().gefahreneSlots (c, cq, k, kq);
        const bool frei = slot >= 0 && x.a->dspKernFuerTest().pool().zustand (slot) == dsp::BankZustand::frei;
        pruefe (hoerHaltLaeuft && bitgleich (ya, yb) && frei && k == -1 && kq == -1,
                "  312/M-46 Teilfall hoerhalt_beim_schalten: laeuft beim Wechsel nach offline die Rueckblende von Candidate "
                "(Hoerhalt, 64 von 256 Samples), ist der Ausgang ab Sample 0 bitgleich zum bestaetigten Lauf, und nach "
                "Takt, ceil (kFadeSamples / 64) + 1 Bloecken und Takt ist die gehaltene Bank frei",
                std::string ("Hoerhalt vorher ") + (hoerHaltLaeuft ? "ja" : "NEIN") + ", Ausgang "
                + (bitgleich (ya, yb) ? "bitgleich" : "VERSCHIEDEN") + ", Slot " + std::to_string (slot)
                + (frei ? " frei" : " BELEGT") + ", Candidate aktiv/Quelle " + std::to_string (k) + "/" + std::to_string (kq));
    }

    // ── 312/M-42 Teilfall: ein laufender Fade in die GEGENRICHTUNG (Processed
    // nach Candidate, 64 von 256 Samples) endet mit dem harten Schalten. ──────
    {
        auto x = offlineAufbau (64, true, false, dsp::Hoermatrix::processed);
        x.a->setzeHoermatrix (dsp::Hoermatrix::candidate);
        const auto ya0 = fahreAudio (*x.a, 1, 64, 64);
        const auto yb0 = fahreAudio (*x.b, 1, 64, 64);
        const bool fadeLaeuft = ! bitgleich (ya0, yb0);
        x.a->setNonRealtime (true); x.b->setNonRealtime (true);
        const auto ya = fahreAudio (*x.a, 8, 64, 65);
        const auto yb = fahreAudio (*x.b, 8, 64, 65);
        pruefe (fadeLaeuft && bitgleich (ya, yb),
                "  312/M-42 Teilfall fade_in_der_gegenrichtung: blendet die Hoermatrix beim Wechsel nach offline gerade von "
                "Processed nach Candidate, endet dieser Fade mit dem harten Schalten - der Ausgang ist ab Sample 0 bitgleich "
                "zum bestaetigten Lauf",
                std::string ("Fade vorher hoerbar ") + (fadeLaeuft ? "ja" : "NEIN") + ", danach "
                + (bitgleich (ya, yb) ? "bitgleich" : "VERSCHIEDEN"));
    }

    // ── 312/M-43: der Rueckweg blendet weiter WEICH. Gleichanteil 0,3 auf
    // beiden Kanaelen; der bestaetigte Satz ist neutral (Ausgang = Eingang), die
    // Vorschau traegt Output-Trim +6 dB. Nach Offline an, Takt und Offline aus
    // kehrt die Auswahl ueber eine neue Vorschau zu Candidate zurueck; gemessen
    // wird der Nachbarsprung nach [SONDE-015] E-31. ───────────────────────────
    {
        const int trim = param::indexVonId ("v1.global.output_trim_db");
        auto a = prozessor (48000.0, 64);
        const auto z = mitEq (true);
        setze (*a, z);
        auto vorschau = z;
        vorschau.werte[(size_t) trim].zahl = 6.0;
        juce::String g;
        const bool gesetzt = a->setzePreview (vorschau, g);
        a->setzeHoermatrix (dsp::Hoermatrix::candidate);
        fahreGleichanteil (*a, 32, 64, 0.3f);
        const bool candidateHoerbar = a->dspKernFuerTest().wirksameHoermatrix() == dsp::Hoermatrix::candidate;
        a->setNonRealtime (true);
        fahreGleichanteil (*a, 8, 64, 0.3f);
        a->kontrollTaktFuerTest();
        fahreGleichanteil (*a, 8, 64, 0.3f);
        a->kontrollTaktFuerTest();
        a->setNonRealtime (false);
        fahreGleichanteil (*a, 8, 64, 0.3f);
        const bool zurueck = a->setzePreview (vorschau, g);
        a->setzeHoermatrix (dsp::Hoermatrix::candidate);
        const auto y = fahreGleichanteil (*a, 32, 64, 0.3f);
        const double ziel = 0.3 * std::pow (10.0, 6.0 / 20.0);
        const double schritt = (ziel - 0.3) / (double) fade;
        const double toleranz = 1.0 / 8388608.0;   // 2^-23, [SONDE-015] E-31
        double groessterSprung = 0.0, ersterSprung = -1.0;
        for (size_t i = 1; i < y.size(); ++i)
        {
            const double sprung = std::abs ((double) y[i] - (double) y[i - 1]);
            groessterSprung = std::max (groessterSprung, sprung);
            if (ersterSprung < 0.0 && y[i] != y[0]) ersterSprung = sprung;
        }
        const bool amZiel = ! y.empty() && std::abs ((double) y.back() - ziel) < 1.0e-6 && y.front() == 0.3f;
        std::ostringstream d;
        d << std::setprecision (9) << "erster Sprung " << ersterSprung << ", groesster " << groessterSprung
          << ", Fadeschritt " << schritt << " + 2^-23, Anfang " << y.front() << ", Ende " << y.back();
        pruefe (gesetzt && zurueck && candidateHoerbar && amZiel && ersterSprung >= 0.0
                    && ersterSprung <= schritt + toleranz && groessterSprung <= schritt + toleranz,
                "312/M-43 der_rueckweg_blendet_weich (Teilfall von 312/M-42, [SONDE-015] E-31 und M-55): nach Offline an und "
                "aus kehrt die Hoermatrix zu Candidate zurueck, und der Nachbarsprung am Umschaltsample wie ueber den ganzen "
                "Lauf ist hoechstens die Fadeschrittweite plus 2^-23 - die harte Umstellung gilt nur in die Offlinerichtung",
                d.str());
    }

    // ── 312/M-44: Offline an, aus, wieder an. ───────────────────────────────
    {
        auto x = offlineAufbau (256);
        x.a->setNonRealtime (true); x.b->setNonRealtime (true);
        fahreAudio (*x.a, 4, 256, 66); fahreAudio (*x.b, 4, 256, 66);
        x.a->kontrollTaktFuerTest();
        x.a->setNonRealtime (false); x.b->setNonRealtime (false);
        x.a->kontrollTaktFuerTest();
        const bool nachRueckweg = ! x.a->previewAktiv() && x.a->gewuenschteHoermatrix() == dsp::Hoermatrix::processed;
        const auto ya1 = fahreAudio (*x.a, 20, 256, 67);
        const auto yb1 = fahreAudio (*x.b, 20, 256, 67);
        x.a->setNonRealtime (true); x.b->setNonRealtime (true);
        const auto ya2 = fahreAudio (*x.a, 20, 256, 68);
        const auto yb2 = fahreAudio (*x.b, 20, 256, 68);
        x.a->kontrollTaktFuerTest();
        pruefe (nachRueckweg && bitgleich (ya1, yb1) && bitgleich (ya2, yb2) && ! x.a->previewAktiv(),
                "312/M-44 offline_hin_und_zurueck_belebt_die_vorschau_nicht (Teilfall von 312/M-42): nach Offline an, Takt, "
                "Offline aus und Takt ist keine Vorschau aktiv, die Auswahl Processed und der Echtzeitlauf bitgleich zum "
                "bestaetigten; ein zweiter Wechsel nach offline findet keine Vorschau und aendert am Ausgang nichts",
                std::string ("nach dem Rueckweg ") + (nachRueckweg ? "ohne Vorschau, Processed" : "VORSCHAU ODER AUSWAHL BELEBT")
                + ", Echtzeit " + (bitgleich (ya1, yb1) ? "bitgleich" : "VERSCHIEDEN")
                + ", zweites Offline " + (bitgleich (ya2, yb2) ? "bitgleich" : "VERSCHIEDEN"));
    }

    // ── 312/M-45: derselbe Ablauf ohne Vorschau. ────────────────────────────
    {
        auto x = offlineAufbau (256, false);
        x.a->setNonRealtime (true);
        const auto ya = fahreAudio (*x.a, 20, 256, 69);
        const auto yb = fahreAudio (*x.b, 20, 256, 69);
        pruefe (bitgleich (x.ya, x.yb) && bitgleich (ya, yb),
                "312/M-45 ohne_vorschau_ab_sample_0_bitgleich (Teilfall von 312/M-42, Gegenfall): ohne aktive Vorschau ist "
                "der Lauf nach setNonRealtime (true) ab Sample 0 bitgleich zum Lauf ohne Offlineflag");
    }

    // ── 312/M-47: Blockgroessen 1, 64 und 256 - Sampleindex, nicht Blockindex.
    for (const int groesse : { 1, 64, 256 })
    {
        auto x = offlineAufbau (groesse, true, true);
        const size_t n = (size_t) groesse;
        // fahreAudio legt je Block erst den linken, dann den rechten Kanal ab.
        const size_t letzterL = x.ya.size() - 2 * n + (n - 1);
        const size_t letzterR = x.ya.size() - 1;
        const bool vorherUnveraendert = x.ya[letzterL] == x.yc[letzterL] && x.ya[letzterR] == x.yc[letzterR];
        const bool vorherCandidate    = x.ya[letzterL] != x.yb[letzterL] || x.ya[letzterR] != x.yb[letzterR];
        x.a->setNonRealtime (true);
        const auto ya = fahreAudio (*x.a, 1, groesse, 70);
        const auto yb = fahreAudio (*x.b, 1, groesse, 70);
        const auto yc = fahreAudio (*x.c, 1, groesse, 70);
        const bool erstesBestaetigt = ya[0] == yb[0] && ya[n] == yb[n];
        const bool erstesGewechselt = ya[0] != yc[0] || ya[n] != yc[n];
        pruefe (vorherUnveraendert && vorherCandidate && erstesBestaetigt && erstesGewechselt && bitgleich (ya, yb),
                "312/M-47 blockgroesse_" + std::to_string (groesse) + " (Teilfall von 312/M-42, Zahlenrand): das LETZTE Sample "
                "vor dem Flag ist unveraendert (gleich dem Zwilling ohne Flag, noch Candidate), das ERSTE danach ist bestaetigt "
                "(gleich dem Lauf ohne Vorschau, ungleich dem Zwilling)"
                + std::string (groesse == 1 ? " - bei Blockgroesse 1 genau ein Sample" : ""),
                std::string ("vorher unveraendert ") + (vorherUnveraendert ? "ja" : "NEIN") + ", vorher Candidate "
                + (vorherCandidate ? "ja" : "NEIN") + ", erstes bestaetigt " + (erstesBestaetigt ? "ja" : "NEIN")
                + ", erstes gewechselt " + (erstesGewechselt ? "ja" : "NEIN"));
    }

    // ── NAK-312 Etappe 6a, Zusaetze aus der Erstpruefung der Etappe 5 (§34):
    // 312/M-89 (R-312-20) und 312/M-90 (R-312-22). Beide laufen unter der
    // Taktsperre: der Worker verbucht das angeforderte Vorschauende nicht
    // mitten in der Messung, die Vorschau bleibt stehen, und kein Ergebnis
    // haengt an der Wanduhr. ────────────────────────────────────────────────

    // 312/M-89: ein WAEHREND des Renders gesetzter Hoerwunsch (Dry, Delta,
    // Candidate) wirkt als Processed - jeder Offlineblock ist ab Sample 0
    // bitgleich zum bestaetigten Lauf, bei Blockgroesse 1, 64 und 256.
    for (const int groesse : { 1, 64, 256 })
    {
        auto x = offlineAufbau (groesse);
        const bool candidateHoerbar = x.a->dspKernFuerTest().wirksameHoermatrix() == dsp::Hoermatrix::candidate
                                   && ! bitgleich (x.ya, x.yb);
        const int bloecke = std::max (1, 512 / groesse);
        bool alleGleich = true, vorschauStand = false;
        std::string detail;
        x.a->mitAngehaltenemTaktFuerTest ([&]
        {
            x.a->setNonRealtime (true);
            x.b->setNonRealtime (true);
            int saat = 71;
            const std::pair<dsp::Hoermatrix, const char*> wuensche[] = {
                { dsp::Hoermatrix::dry, "Dry" }, { dsp::Hoermatrix::delta, "Delta" },
                { dsp::Hoermatrix::candidate, "Candidate" } };
            for (const auto& [wunsch, name] : wuensche)
            {
                x.a->setzeHoermatrix (wunsch);
                const auto ya = fahreAudio (*x.a, bloecke, groesse, saat);
                const auto yb = fahreAudio (*x.b, bloecke, groesse, saat);
                ++saat;
                const bool gleich = bitgleich (ya, yb);
                alleGleich = alleGleich && gleich;
                detail += std::string (name) + (gleich ? " bitgleich, " : " VERSCHIEDEN, ");
            }
            vorschauStand = x.a->previewAktiv();
        });
        pruefe (candidateHoerbar && vorschauStand && alleGleich,
                "312/M-89 hoerwunsch_im_render_wirkt_als_processed_blockgroesse_" + std::to_string (groesse)
                + " (R-312-20, E-312-7): echter SondeProcessor mit gesetzter Vorschau, nach setNonRealtime (true) im Render "
                "nacheinander Hoerwunsch Dry, Delta und Candidate - jeder Offlineblock ist ab Sample 0 bitgleich zum "
                "bestaetigten Lauf",
                detail + "je " + std::to_string (bloecke) + " Bloecke, Vorschau vorher hoerbar "
                + (candidateHoerbar ? "ja" : "NEIN") + ", am Ende noch gesetzt " + (vorschauStand ? "ja" : "NEIN"));
    }

    // 312/M-90: Offline an und sofort wieder aus, OHNE Block dazwischen - der
    // erste Echtzeitblock schaltet nicht hart. Aufbau wie 312/M-43: Gleichanteil
    // 0,3, neutraler bestaetigter Satz, Vorschau Output-Trim +6 dB, Hoermatrix
    // Processed; danach Wechsel auf Candidate, Nachbarsprung nach [SONDE-015]
    // E-31.
    {
        const int trim = param::indexVonId ("v1.global.output_trim_db");
        auto a = prozessor (48000.0, 64);
        const auto z = mitEq (true);
        setze (*a, z);
        auto vorschau = z;
        vorschau.werte[(size_t) trim].zahl = 6.0;
        juce::String g;
        const bool gesetzt = a->setzePreview (vorschau, g);
        a->setzeHoermatrix (dsp::Hoermatrix::processed);
        const auto vorher = fahreGleichanteil (*a, 32, 64, 0.3f);
        const bool candidateDa = a->dspKernFuerTest().candidateVorhanden();
        std::vector<float> y;
        a->mitAngehaltenemTaktFuerTest ([&]
        {
            a->setNonRealtime (true);
            a->setNonRealtime (false);                 // kein Block dazwischen
            a->setzeHoermatrix (dsp::Hoermatrix::candidate);
            y = fahreGleichanteil (*a, 32, 64, 0.3f);
        });
        const double ziel = 0.3 * std::pow (10.0, 6.0 / 20.0);
        const double schritt = (ziel - 0.3) / (double) fade;
        const double toleranz = 1.0 / 8388608.0;       // 2^-23, [SONDE-015] E-31
        const double ersterSprung = y.empty() || vorher.empty()
            ? -1.0 : std::abs ((double) y.front() - (double) vorher.back());
        double groessterSprung = ersterSprung;
        for (size_t i = 1; i < y.size(); ++i)
            groessterSprung = std::max (groessterSprung, std::abs ((double) y[i] - (double) y[i - 1]));
        const bool amZiel = ! y.empty() && ! vorher.empty() && vorher.back() == 0.3f
                         && std::abs ((double) y.back() - ziel) < 1.0e-6;
        std::ostringstream d;
        d << std::setprecision (9) << "erster Sprung " << ersterSprung << ", groesster " << groessterSprung
          << ", Fadeschritt " << schritt << " + 2^-23, vorher " << (vorher.empty() ? -1.0f : vorher.back())
          << ", Ende " << (y.empty() ? -1.0f : y.back());
        pruefe (gesetzt && candidateDa && amZiel && ersterSprung >= 0.0
                    && ersterSprung <= schritt + toleranz && groessterSprung <= schritt + toleranz,
                "312/M-90 hin_und_zurueck_ohne_block_schaltet_nicht_hart (R-312-22, dient 312/M-43 und [SONDE-015] M-55): "
                "setNonRealtime (true) und sofort setNonRealtime (false) ohne Block dazwischen, danach Echtzeitbloecke mit "
                "Wechsel auf Candidate - der Nachbarsprung am Umschaltsample wie ueber den ganzen Lauf ist hoechstens die "
                "Fadeschrittweite plus 2^-23",
                d.str());
    }
}

//==============================================================================
// NAK-312 Etappe 5, zweiter Aenderungssatz (T3-01-02; R-312-4): die
// Parameteranbindung. Die 112 Hostparameter haengen direkt am Prozessor; der
// APVTS-Adapter und seine Hoerer-Sperre sind weg. Der Identitaetsbeweis ist
// das Parametergolden des Basis-SHA (oben, `parameterGolden`).

/** Die Zeilen eines Golden-Textes: ohne Zeilenendezeichen, ohne Leerzeilen
    und ohne Kommentarzeilen - die Datei darf auf einem Rechner mit CRLF
    ausgecheckt sein. */
std::vector<std::string> goldenZeilen (const std::string& text)
{
    std::vector<std::string> zeilen;
    std::istringstream ein (text);
    std::string z;
    while (std::getline (ein, z))
    {
        if (! z.empty() && z.back() == '\r') z.pop_back();
        if (z.empty() || z[0] == '#') continue;
        zeilen.push_back (z);
    }
    return zeilen;
}

bool istParameterZeile (const std::string& z)
{
    return z.rfind ("p\t", 0) == 0 || z.rfind ("anzahl\t", 0) == 0 || z.rfind ("gruppen\t", 0) == 0;
}

/** Vergleicht eine Teilmenge zweier Golden-Texte Zeile fuer Zeile. */
std::string vergleicheZeilen (const std::vector<std::string>& soll, const std::vector<std::string>& ist,
                              bool parameterTeil, bool& gleich)
{
    std::vector<std::string> a, b;
    for (const auto& z : soll) if (istParameterZeile (z) == parameterTeil) a.push_back (z);
    for (const auto& z : ist)  if (istParameterZeile (z) == parameterTeil) b.push_back (z);
    gleich = ! a.empty() && a == b;
    std::string d = std::to_string (b.size()) + " Zeilen gegen " + std::to_string (a.size()) + " im Golden";
    for (size_t i = 0; i < std::max (a.size(), b.size()); ++i)
    {
        const std::string sa = i < a.size() ? a[i] : "<fehlt>";
        const std::string sb = i < b.size() ? b[i] : "<fehlt>";
        if (sa != sb)
        {
            d += ", erste Abweichung Zeile " + std::to_string (i + 1) + ": Golden '" + sa.substr (0, 120)
               + "' gegen '" + sb.substr (0, 120) + "'";
            break;
        }
    }
    return d;
}

void nak312Parameteranbindung()
{
    abschnitt ("ZB - NAK-312 Etappe 5: die Parameteranbindung ohne APVTS (312/M-48 bis 312/M-50, 312/M-52 bis 312/M-54; "
               "312/M-51 in Abschnitt O)");

    // ── 312/M-48 und 312/M-49: gegen das Golden des Basis-SHA. ───────────────
    {
        const auto textDatei = wurzel().getChildFile ("docs/beweise/roh/NAK-312-parameter-golden.txt");
        const auto binDatei  = wurzel().getChildFile ("docs/beweise/roh/NAK-312-parameter-golden-s1.bin");
        const bool vorhanden = textDatei.existsAsFile() && binDatei.existsAsFile();
        const auto soll = goldenZeilen (textDatei.loadFileAsString().toStdString());
        juce::MemoryBlock s1Golden;
        binDatei.loadFileAsData (s1Golden);
        const auto g = parameterGolden();
        const auto ist = goldenZeilen (g.text);
        bool parameterGleich = false, zustandGleich = false;
        const auto dParameter = vergleicheZeilen (soll, ist, true, parameterGleich);
        const auto dZustand   = vergleicheZeilen (soll, ist, false, zustandGleich);
        pruefe (vorhanden && parameterGleich,
                "312/M-48 hostparameter_gleich_dem_golden_des_basis_sha (T3-01-02, R-312-4, Identitaet NAK-30): Zahl, "
                "Reihenfolge, IDs, Namen, Klassen, Bereiche und Defaults der Hostparameter und ihre Hostsicht "
                "(convertTo0to1 an Minimum, Default, Maximum; convertFrom0to1, getText und getValueForText an 0, 0,37 "
                "und 1) sind Zeile fuer Zeile und Bit fuer Bit die des Goldens von 12300f1e",
                dParameter);
        pruefe (vorhanden && zustandGleich && g.s1 == s1Golden && s1Golden.getSize() > 0,
                "312/M-49 statebytes_und_epochenfolge_gleich_dem_golden (T3-01-02, R-312-4, CLAUDE.md State verlustfrei): "
                "speichern, laden, speichern liefert die Bytes des Basis-SHA (s1 bytegleich zur eingefrorenen Datei, "
                "s1 = s2), und die Automationsfahrt dieselbe Epochenfolge, Revision, state_hash und Host-Dirty",
                dZustand + ", s1 " + std::to_string (g.s1.getSize()) + " Byte, "
                + (g.s1 == s1Golden ? "bytegleich zur Datei" : "VERSCHIEDEN von der Datei"));
    }

    // ── 312/M-50: der Textriegel ueber eq-copilot/plugin/sonde/. ───────────
    {
        const auto ordner = wurzel().getChildFile ("eq-copilot/plugin/sonde");
        int dateien = 0;
        std::string treffer;
        if (ordner.isDirectory())
        {
            for (const auto& f : ordner.findChildFiles (juce::File::findFiles, false))
            {
                ++dateien;
                if (f.loadFileAsString().contains ("AudioProcessorValueTreeState"))
                    treffer += str (f.getFileName()) + "; ";
            }
        }
        pruefe (dateien >= 3 && treffer.empty(),
                "312/M-50 kein_apvts_mehr_in_der_sonde (T3-01-02, R-312-4): kein Quelltext unter eq-copilot/plugin/sonde/ "
                "nennt die APVTS-Klasse - die zweite Sperrfamilie existiert nicht mehr; fail-closed: fehlt der Ordner "
                "oder sind es weniger als 3 Dateien, ist es rot",
                std::to_string (dateien) + " Dateien unter " + str (ordner.getFullPathName())
                + (treffer.empty() ? std::string (", kein Treffer") : ", Treffer: " + treffer));
    }

    // ── 312/M-52: addListener und removeListener paarweise. Gemessen am Ende
    // des Destruktorrumpfs (der Prozessor lebt dort noch, der Worker ist
    // gejoint): ein Wert an jedem der 112 Parameter erreicht einen Zaehler
    // des Beins, den Prozessor nicht mehr. ────────────────────────────────
    {
        struct Zaehler final : juce::AudioProcessorParameter::Listener
        {
            std::atomic<int> werte { 0 };
            void parameterValueChanged (int, float) override { ++werte; }
            void parameterGestureChanged (int, bool) override {}
        } zaehler;
        auto p = prozessor();
        auto* roh = p.get();
        const auto summe = [roh]
        {
            std::uint64_t s = 0;
            for (int i = 0; i < param::kHostParameter; ++i) s += roh->hostEreignisFuerTest (i);
            return s;
        };
        // Gegenprobe vorher: derselbe Weg erreicht den lebenden Prozessor.
        const auto vorLebend = summe();
        hostSchreibt (*p, iBand (3, param::kGainDb), 5.0f);
        const bool lebendErreicht = summe() == vorLebend + 1;
        for (auto* q : p->getParameters())
            q->addListener (&zaehler);
        std::uint64_t vorAbbau = 0, nachAbbau = 0;
        bool offenNachAbbau = true;
        int erreicht = -1;
        p->setzeAbbauEndeHakenFuerTest ([&, roh]
        {
            roh->kontrollTaktFuerTest();   // leert die Mailbox; der Worker ist hier schon gejoint
            vorAbbau = summe();
            for (auto* q : roh->getParameters())
                q->setValueNotifyingHost (q->getValue() >= 0.5f ? 0.0f : 1.0f);
            nachAbbau = summe();
            offenNachAbbau = roh->hostEreignisOffenFuerTest();
            erreicht = zaehler.werte.load();
        });
        p.reset();
        pruefe (lebendErreicht && erreicht == param::kHostParameter && nachAbbau == vorAbbau && ! offenNachAbbau,
                "312/M-52 listener_paarweise_an_und_ab (Teilfall von 312/M-50, CLAUDE.md verbinden-trennen): am Ende "
                "des Destruktorrumpfs erreicht ein Wert an jedem der 112 Hostparameter den Zaehler des Beins, aber "
                "keinen Listener des Prozessors mehr (Mailbox unveraendert); vorher erreichte derselbe Weg ihn",
                std::string ("vorher erreicht ") + (lebendErreicht ? "ja" : "NEIN") + ", Zaehler " + std::to_string (erreicht)
                + ", Mailbox " + zahl (vorAbbau) + " -> " + zahl (nachAbbau) + ", offen " + (offenNachAbbau ? "JA" : "nein"));
    }

    // ── 312/M-53: 4000 Bloecke, Hostautomation aus einem zweiten Thread. ────
    {
        auto p = prozessor (48000.0, 512);
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 3.0);
        setze (*p, z);
        std::atomic<bool> halt { false };
        std::atomic<std::uint64_t> geschrieben { 0 };
        std::thread host ([&]
        {
            juce::Random r (5300);
            while (! halt.load())
            {
                for (const auto& bp : blockParameter())
                    hostSchreibt (*p, bp.index, r.nextBool() ? bp.extrem : bp.standard);
                hostSchreibt (*p, iBand (0, param::kGainDb), r.nextFloat() * 12.0f - 6.0f);
                geschrieben.fetch_add (1);
            }
        });
        while (geschrieben.load() == 0)
            std::this_thread::yield();
        juce::MidiBuffer midi;
        juce::AudioBuffer<float> puffer (2, 512);
        juce::Random w (5301);
        std::mt19937 zufall (5302);
        std::uniform_int_distribution<int> groessen (1, 512);
        const auto geschriebenVor = geschrieben.load();
        dsp::RtWache::zuruecksetzen();
        std::uint64_t eigene = 0;
        for (int blockNr = 0; blockNr < 4000; ++blockNr)
        {
            const int n = groessen (zufall);
            puffer.setSize (2, n, false, false, true);
            for (int k = 0; k < 2; ++k)
                for (int s = 0; s < n; ++s)
                    puffer.setSample (k, s, w.nextFloat() * 1.6f - 0.8f);
            allokationen = 0;
            zaehleAllokationen = true;
            p->processBlock (puffer, midi);
            zaehleAllokationen = false;
            eigene += allokationen;
        }
        const auto sperren = dsp::RtWache::sperren();
        const auto rtAllok = dsp::RtWache::allokationen();
        const auto geschriebenWaehrend = geschrieben.load() - geschriebenVor;
        halt.store (true);
        host.join();
        pruefe (sperren == 0 && rtAllok == 0 && eigene == 0 && geschriebenWaehrend > 0,
                "312/M-53 parameteranbindung_ist_echtzeitfest (T3-01-02, R-312-4 letzter Satz, [SONDE-015] M-47, E-312-11): "
                "4000 Bloecke wechselnder Groesse (1 bis 512), waehrend ein zweiter Thread Hostautomation auf die vier "
                "blockgebundenen Parameter und einen Bandwert schreibt - RtWache::sperren() und RtWache::allokationen() "
                "bleiben ab Callback-Eintritt 0. Gezaehlt wird, was der Prozessor ab processBlock tut; die Sperre, die "
                "JUCE vor dem Plugincode nimmt (sendValueChangedMessageToListeners), sieht der Zaehler bauartbedingt nicht",
                "RtWache::sperren " + zahl (sperren) + ", RtWache::allokationen " + zahl (rtAllok) + ", eigener Zaehler "
                + zahl (eigene) + ", Hostschreibrunden waehrend der Bloecke " + zahl (geschriebenWaehrend));
    }

    // ── 312/M-54: Zahlenrand Parameterindex; der eigene Abgleich meldet nichts.
    {
        auto p = prozessor();
        const auto stand = [&p]
        {
            std::array<std::uint32_t, (size_t) param::kHostParameter> s {};
            for (int i = 0; i < param::kHostParameter; ++i) s[(size_t) i] = p->hostEreignisFuerTest (i);
            return s;
        };
        const auto vor = stand();
        p->parameterValueChangedFuerTest (-1, 0.25f);
        p->parameterValueChangedFuerTest (param::kHostParameter, 0.25f);
        const auto nachUngueltig = stand();
        p->parameterValueChangedFuerTest (0, 0.25f);
        p->parameterValueChangedFuerTest (param::kHostParameter - 1, 0.25f);
        const auto nachGueltig = stand();
        int andereBewegt = 0;
        for (int i = 1; i < param::kHostParameter - 1; ++i)
            if (nachGueltig[(size_t) i] != vor[(size_t) i]) ++andereBewegt;
        const bool randGueltig = nachGueltig[0] == vor[0] + 1
                              && nachGueltig[(size_t) param::kHostParameter - 1] == vor[(size_t) param::kHostParameter - 1] + 1;
        // Der eigene Abgleich (Commit -> hostParameterAbgleichen) schreibt die
        // Regler mit Herkunftstag: abgleichTiefe > 0, keine Rueckmeldung.
        auto z = mitEq (true);
        setzeBand (z, 0, 1000.0, 6.0);
        const auto vorCommit = stand();
        const auto e = setze (*p, z);
        const auto nachCommit = stand();
        auto& gainParam = hostParam (*p, iBand (0, param::kGainDb));
        const bool reglerFolgt = gainParam.getValue() == gainParam.convertTo0to1 (6.0f);
        pruefe (nachUngueltig == vor && randGueltig && andereBewegt == 0 && e.ausgang == tx::Ausgang::commit
                    && reglerFolgt && nachCommit == vorCommit,
                "312/M-54 parameterindex_rand_und_abgleichtiefe (Teilfall von 312/M-50, Zahlenrand): parameterValueChanged "
                "mit Index -1 und 112 laesst die Mailbox unveraendert, 0 und 111 erreichen sie je genau einmal; der "
                "eigene Abgleich nach einem Commit stellt den Regler, ohne ein Hostereignis zu melden (abgleichTiefe > 0)",
                std::string ("ungueltig ") + (nachUngueltig == vor ? "ohne Wirkung" : "WIRKT") + ", Rand 0/111 "
                + (randGueltig ? "je +1" : "FALSCH") + ", andere bewegt " + std::to_string (andereBewegt)
                + ", Abgleich " + (nachCommit == vorCommit ? "ohne Ereignis" : "MIT EREIGNIS") + ", Regler "
                + (reglerFolgt ? "folgt" : "FOLGT NICHT"));
    }
}

/*  NAK-312 Etappe 5, Nacharbeit 1 (R-312-19, 312/M-88 Kanonteil). LZ-1 war ein
    regulaeres Programmende von FL (Exit 0, FL schreibt seine Einstellungen,
    Energie-Ereignis beim Ende - dieselbe Spur wie ein WM_CLOSE an das
    Hauptfenster), gemessen auch am Produktstand der Etappe 4. Ohne Host
    messbar ist davon allein die Pluginseite: kein Quelltext der zwei
    ausgelieferten Buendel ruft einen Weg, der den Hostprozess beendet oder
    sein Fenster schliesst. Textriegel, fail-closed; Kommentare zaehlen mit. */
bool nennt (const std::string& text, const std::string& wort, bool nurAufruf)
{
    const auto istBezeichner = [] (char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
    };
    for (auto pos = text.find (wort); pos != std::string::npos; pos = text.find (wort, pos + 1))
    {
        if (pos > 0 && istBezeichner (text[pos - 1]))
            continue;
        // Ein Mitgliedsaufruf (`schloss.exit()`, `p->exit()`) ist kein Prozessende.
        if (nurAufruf && pos > 0 && (text[pos - 1] == '.' || text[pos - 1] == '>'))
            continue;
        auto nach = pos + wort.size();
        if (nach < text.size() && istBezeichner (text[nach]))
            continue;
        if (! nurAufruf)
            return true;
        while (nach < text.size() && (text[nach] == ' ' || text[nach] == '\t'))
            ++nach;
        if (nach < text.size() && text[nach] == '(')
            return true;
    }
    return false;
}

void nak312HostEnde()
{
    abschnitt ("ZC - NAK-312 Etappe 5 Nacharbeit 1: kein Weg zum Hostende im Pluginquelltext (312/M-88 Kanonteil, R-312-19)");

    // Die Quellordner der Buendel EqCopilot und NakamaProbeeq (CMakeLists.txt:
    // NAKAMA_PROZESSOR_QUELLEN, NAKAMA_KERN_QUELLEN, die Sondenziele) samt der
    // Hostbruecke; tests/, probe/, hostprobe/ und spike/ liefern kein Buendel.
    const char* const ordner[] = { "src", "sonde", "dsp", "state", "vertrag", "core", "hostbridge" };
    // Beendet den Prozess, schliesst sein Fenster oder beendet seine Nachrichtenschleife.
    const char* const woerter[] = { "ExitProcess", "TerminateProcess", "PostQuitMessage", "FatalAppExit",
                                    "quick_exit", "_Exit", "WM_CLOSE", "WM_QUIT", "SC_CLOSE",
                                    "systemRequestedQuit", "JUCEApplicationBase" };
    // Als Aufruf: exit (...) und _exit (...); dazu JUCEs Process::terminate, das ExitProcess (1) ruft.
    // std::terminate bleibt erlaubt (NAK-289): es endet mit WER-Spur, nie still wie LZ-1.
    const char* const aufrufe[] = { "exit", "_exit" };
    int dateien = 0;
    bool alleOrdner = true;
    std::string treffer;
    for (const auto* o : ordner)
    {
        const auto dir = wurzel().getChildFile ("eq-copilot/plugin").getChildFile (o);
        if (! dir.isDirectory()) { alleOrdner = false; treffer += std::string ("Ordner fehlt: ") + o + "; "; continue; }
        // NAK-312 Etappe 6a (H-4): auch die Vorlagen `*.in`, aus denen der Bau
        // Kopfdateien erzeugt (core/ipc, hostbridge) - die Behauptung sagt
        // "kein Quelltext", nicht "keine .cpp und .h".
        for (const auto& f : dir.findChildFiles (juce::File::findFiles, true, "*.cpp;*.h;*.in"))
        {
            ++dateien;
            const auto text = f.loadFileAsString().toStdString();
            const auto name = str (f.getRelativePathFrom (wurzel()));
            for (const auto* w : woerter)
                if (nennt (text, w, false))
                    treffer += name + ": " + w + "; ";
            for (const auto* a : aufrufe)
                if (nennt (text, a, true))
                    treffer += name + ": " + a + " (...); ";
            if (text.find ("Process::terminate") != std::string::npos)
                treffer += name + ": Process::terminate; ";
        }
    }
    pruefe (alleOrdner && dateien >= 100 && treffer.empty(),
            "312/M-88 kein_weg_zum_hostende_im_pluginquelltext (Kanonteil von M-88, R-312-19): kein Quelltext unter "
            "src, sonde, dsp, state, vertrag, core und hostbridge von eq-copilot/plugin ruft ExitProcess, "
            "TerminateProcess, PostQuitMessage, FatalAppExit, exit, _exit, quick_exit, _Exit, Process::terminate oder "
            "systemRequestedQuit oder nennt WM_CLOSE, WM_QUIT, SC_CLOSE oder JUCEApplicationBase; fail-closed: fehlt "
            "ein Ordner oder sind es weniger als 100 Dateien, ist es rot",
            std::to_string (dateien) + " Dateien" + (treffer.empty() ? std::string (", kein Treffer") : ", Treffer: " + treffer));
}

//==============================================================================
// NAK-312 Etappe 7b, Satz 2 (T3-01-09; Karten U48 und U58; Weg E1 mit K-B;
// Manifest §46.1, §47.3): der Hostbypass am echten SondeProcessor - die
// Echtzeitwache ab dem zweiten Eintritt, der Bypassparameter des Wrappers,
// Zustand und Ruhegrenze, der Zaehlerrand am Blockrand, Offline gleich
// Echtzeit, keine haengende Bank. Der Test ruft selbst den Eintritt, den der
// VST3-Wrapper waehlt, und baut dessen Weiche nicht nach
// (juce_audio_plugin_client_VST3.cpp:3906-3909, Bauartefakt). R heisst ein
// zweiter Prozessor mit identischem Stand, der nur processBlock bekommt.

/// Aufbau HB: eq an, Band 0 Bell 1 kHz +12 dB Q 10, Output-Trim +6 dB.
param::DspSatz hbSatz()
{
    auto z = mitEq (true);
    setzeBand (z, 0, 1000.0, 12.0);
    z.werte[(size_t) iBand (0, param::kQ)].zahl = 10.0;
    z.werte[(size_t) kOutTrim].zahl = 6.0;
    return z;
}

std::unique_ptr<Prozessor> hbPruefling (int block, bool& commit)
{
    auto p = prozessor (48000.0, block);
    commit = setze (*p, hbSatz()).ausgang == tx::Ausgang::commit;
    return p;
}

/// Ein Block ueber den gewaehlten Eintritt, in place.
void hbBlock (Prozessor& p, bool bypass, juce::AudioBuffer<float>& b)
{
    juce::MidiBuffer midi;
    if (bypass) p.processBlockBypassed (b, midi);
    else        p.processBlock (b, midi);
}

/// Fuellt `b` mit dem Gleichwert 0,25 (die Bell laesst ihn unveraendert, der
/// Trim hebt ihn auf rund 0,4988) oder mit Rauschen aus `w`.
void hbFuelle (juce::AudioBuffer<float>& b, juce::Random* w)
{
    for (int k = 0; k < b.getNumChannels(); ++k)
        for (int n = 0; n < b.getNumSamples(); ++n)
            b.setSample (k, n, w != nullptr ? w->nextFloat() * 1.6f - 0.8f : 0.25f);
}

/// Faehrt `bypass.size()` Bloecke Gleichwert zu `groesse` ueber die Eintritte
/// `bypass[i]` und liefert den Ausgang (L und R je Block hintereinander, das
/// Format von `fahreFolge`). `vorBlock` laeuft vor jedem Block.
std::vector<float> hbDc (Prozessor& p, const std::vector<bool>& bypass, int groesse,
                         const std::function<void (int)>& vorBlock = {})
{
    std::vector<float> aus;
    juce::AudioBuffer<float> b (2, groesse);
    for (int i = 0; i < (int) bypass.size(); ++i)
    {
        if (vorBlock)
            vorBlock (i);
        hbFuelle (b, nullptr);
        hbBlock (p, bypass[(size_t) i], b);
        for (int k = 0; k < 2; ++k)
            aus.insert (aus.end(), b.getReadPointer (k), b.getReadPointer (k) + groesse);
    }
    return aus;
}

/// Die Blende einer Gleichwertfolge im Format von `hbDc`: groesster
/// Nachbarsprung je Kanal ueber den ganzen Lauf gegen die E-31-Schranke
/// (Fadeschrittweite |verarbeitet - 0,25| / 256 plus 2^-23), und die Samples
/// [von, bis) des Laufs, deren Wert nicht bitgleich 0,25 ist.
struct HbSprung { double groesster = 0.0; double schranke = 0.0; int nichtTrocken = 0; bool endlich = true; };

HbSprung hbSprung (const std::vector<float>& aus, int groesse, float verarbeitet, long long von, long long bis)
{
    HbSprung s;
    s.schranke = std::abs ((double) verarbeitet - 0.25) / (double) dsp::kFadeSamples + 1.0 / 8388608.0;
    const size_t bloecke = aus.size() / ((size_t) groesse * 2u);
    for (int k = 0; k < 2; ++k)
    {
        double vorher = std::numeric_limits<double>::quiet_NaN();
        for (size_t b = 0; b < bloecke; ++b)
            for (int n = 0; n < groesse; ++n)
            {
                const float y = aus[b * (size_t) groesse * 2u + (size_t) k * (size_t) groesse + (size_t) n];
                if (! std::isfinite (y)) s.endlich = false;
                if (std::isfinite (vorher))
                    s.groesster = std::max (s.groesster, std::abs ((double) y - vorher));
                vorher = (double) y;
                const long long idx = (long long) (b * (size_t) groesse) + n;
                if (idx >= von && idx < bis && y != 0.25f) ++s.nichtTrocken;
            }
    }
    return s;
}

void nak312Hostbypass()
{
    abschnitt ("ZD - NAK-312 Etappe 7b, Satz 2: der Hostbypass (312/M-101 bis 312/M-104, 312/M-108, 312/M-132; Kosten)");

    // ── 312/M-101 (a): die Wache beginnt auch am Eintritt processBlockBypassed
    {
        bool commit = false;
        auto kopf = std::make_unique<Testplayhead>();   // ueberlebt den Prozessor
        auto p    = hbPruefling (256, commit);
        p->setPlayHead (kopf.get());
        juce::AudioBuffer<float> puffer (2, 256);
        juce::Random w (10101);
        dsp::RtWache::zuruecksetzen();
        std::uint64_t eigene = 0;
        for (int b = 0; b < 200; ++b)
        {
            hbFuelle (puffer, &w);
            allokationen = 0;
            zaehleAllokationen = true;
            hbBlock (*p, true, puffer);
            zaehleAllokationen = false;
            eigene += allokationen;
        }
        const auto sperren = dsp::RtWache::sperren();
        const auto rtAllok = dsp::RtWache::allokationen();
        pruefe (commit && sperren == 200 && kopf->aufrufe == 200 && rtAllok == 0 && eigene == 0,
                "312/M-101 (a) hostbypass_ist_echtzeitfest: wache_beginnt_am_bypasseintritt (R-312-1, [SONDE-015] M-47): "
                "Aufbau HB mit Testplayhead, 200 Bloecke ueber processBlockBypassed - RtWache::sperren() zaehlt genau "
                "200, eine je Block an der Playhead-Stelle vor der Stempelbildung, und RtWache::allokationen() bleibt 0",
                "gemeldete Sperren " + zahl (sperren) + " bei " + std::to_string (kopf->aufrufe) + " Playhead-Aufrufen, "
                "RtWache::allokationen " + zahl (rtAllok) + ", eigener Zaehler " + zahl (eigene));
        p->setPlayHead (nullptr);
    }

    // ── 312/M-101 (b): 4000 Bloecke mit Eintrittswechseln, Hostautomation aus
    // einem zweiten Thread, Programmwechseln und den drei Hosteintritten
    // reset, releaseResources und prepareToPlay mitten im Lauf. ─────────────
    {
        bool commit = false;
        auto p = hbPruefling (512, commit);
        std::atomic<bool> halt { false };
        std::atomic<std::uint64_t> geschrieben { 0 };
        std::thread host ([&]
        {
            juce::Random r (10102);
            while (! halt.load())
            {
                for (const auto& bp : blockParameter())
                    hostSchreibt (*p, bp.index, r.nextBool() ? bp.extrem : bp.standard);
                hostSchreibt (*p, iBand (0, param::kGainDb), r.nextFloat() * 12.0f - 6.0f);
                geschrieben.fetch_add (1);
            }
        });
        while (geschrieben.load() == 0)
            std::this_thread::yield();
        juce::AudioBuffer<float> puffer (2, 512);
        juce::Random w (10103);
        std::mt19937 zufall (10104);
        std::uniform_int_distribution<int> groessen (1, 512);
        const auto geschriebenVor = geschrieben.load();
        dsp::RtWache::zuruecksetzen();
        std::uint64_t eigene = 0;
        bool bypass = false, latenzNull = true;
        int wechsel = 0, bypassBloecke = 0, programme = 0;
        for (int blockNr = 0; blockNr < 4000; ++blockNr)
        {
            if (blockNr % 37 == 0) { bypass = ! bypass; ++wechsel; }
            if (blockNr % 250 == 125)
            {
                auto z = hbSatz();
                z.werte[(size_t) iBand (0, param::kGainDb)].zahl = (blockNr / 250) % 2 == 0 ? 6.0 : 12.0;
                if (setze (*p, z).ausgang == tx::Ausgang::commit) ++programme;
            }
            if (blockNr == 1500) p->reset();
            if (blockNr == 2500) p->releaseResources();
            if (blockNr == 2520) p->prepareToPlay (48000.0, 512);
            const int n = groessen (zufall);
            puffer.setSize (2, n, false, false, true);
            hbFuelle (puffer, &w);
            allokationen = 0;
            zaehleAllokationen = true;
            hbBlock (*p, bypass, puffer);
            zaehleAllokationen = false;
            eigene += allokationen;
            if (bypass) ++bypassBloecke;
            if (p->getLatencySamples() != 0) latenzNull = false;
        }
        const auto sperren = dsp::RtWache::sperren();
        const auto rtAllok = dsp::RtWache::allokationen();
        const auto geschriebenWaehrend = geschrieben.load() - geschriebenVor;
        halt.store (true);
        host.join();
        pruefe (commit && sperren == 0 && rtAllok == 0 && eigene == 0 && latenzNull && p->getTailLengthSeconds() == 0.0
                    && geschriebenWaehrend > 0 && wechsel > 100 && bypassBloecke > 1000 && programme > 0,
                "312/M-101 (b) hostbypass_ist_echtzeitfest (R-312-1, [SONDE-015] M-47, M-51, E-312-11): 4000 Bloecke "
                "wechselnder Groesse (1 bis 512) mit Eintrittswechseln alle 37 Bloecke, Hostautomation aus einem zweiten "
                "Thread, Programmwechseln, reset(), releaseResources() und prepareToPlay mitten im Lauf - "
                "RtWache::sperren() und RtWache::allokationen() bleiben in beiden Eintritten 0, die gemeldete Latenz 0 "
                "Samples und getTailLengthSeconds() 0,0",
                "RtWache::sperren " + zahl (sperren) + ", RtWache::allokationen " + zahl (rtAllok) + ", eigener Zaehler "
                + zahl (eigene) + ", Eintrittswechsel " + std::to_string (wechsel) + ", Bypassbloecke "
                + std::to_string (bypassBloecke) + ", Programme " + std::to_string (programme)
                + ", Hostschreibrunden waehrend der Bloecke " + zahl (geschriebenWaehrend) + ", Latenz 0 "
                + (latenzNull ? "ja" : "NEIN"));
    }

    // ── 312/M-102: der Bypassparameter bleibt beim Wrapper ──────────────────
    {
        auto p = prozessor();
        const auto* bypassParameter = p->getBypassParameter();
        pruefe (bypassParameter == nullptr && p->getParameters().size() == param::kHostParameter,
                "312/M-102 bypassparameter_bleibt_beim_wrapper (Identitaet NAK-30, R-312-4, §7.7 erste Grenze): "
                "getBypassParameter() liefert nullptr - der VST3-Wrapper legt wie bisher seinen eigenen Parameter byps "
                "mit der VST-ID 0x62797073 an -, und der Prozessor meldet genau 112 Hostparameter (das Parametergolden "
                "misst 312/M-48)",
                std::string ("getBypassParameter ") + (bypassParameter == nullptr ? "nullptr" : "GESETZT") + ", "
                + std::to_string (p->getParameters().size()) + " Hostparameter");
    }

    // ── 312/M-103: Zustand, Hash, Revision, Dirty und die Ruhegrenze ─────────
    {
        struct Lauf { std::uint64_t e0 = 0, e1 = 0, offen = 0, zu = 0; };
        const auto lauf = [] (Prozessor& x, bool mitBypass)
        {
            Lauf l;
            juce::AudioBuffer<float> b (2, 256);
            juce::Random w (10301);
            x.mitAngehaltenemTaktFuerTest ([&]
            {
                for (int i = 0; i < 40; ++i) { hbFuelle (b, &w); hbBlock (x, false, b); }
                x.kontrollTaktFuerTest();
                l.e0 = x.automationEpoche();
                // Muster 312/M-24: je Punkt ein Hostwert, ein Block processBlock, ein Takt.
                for (int i = 0; i < 20; ++i)
                {
                    hostSchreibt (x, kOutTrim, 6.0f - 0.1f * (float) i);
                    hbFuelle (b, &w);
                    hbBlock (x, false, b);
                    x.kontrollTaktFuerTest();
                }
                l.e1 = x.automationEpoche();
                for (int i = 0; i < 46; ++i) { hbFuelle (b, &w); hbBlock (x, mitBypass, b); }   // 11 776 Samples
                x.kontrollTaktFuerTest();
                l.offen = x.automationEpoche();
                hbFuelle (b, &w);
                hbBlock (x, mitBypass, b);                                                       // 12 032 Samples
                x.kontrollTaktFuerTest();
                l.zu = x.automationEpoche();
                for (int i = 0; i < 13; ++i) { hbFuelle (b, &w); hbBlock (x, mitBypass, b); }
                for (int i = 0; i < 40; ++i) { hbFuelle (b, &w); hbBlock (x, false, b); }
                x.kontrollTaktFuerTest();
            });
            return l;
        };
        bool cp = false, cr = false;
        auto p = hbPruefling (256, cp);
        auto r = hbPruefling (256, cr);
        DirtyZaehler dirty;
        p->addListener (&dirty);
        juce::MemoryBlock vorher, nachher;
        p->getStateInformation (vorher);
        const auto hashVor = str (p->stateHashText());
        const auto revVor  = p->stateRevision();
        const auto undoVor = p->zustandLesen().undoRing.size();
        const auto lp = lauf (*p, true);
        const auto lr = lauf (*r, false);
        p->getStateInformation (nachher);
        const int dirtyMeldungen = dirty.nichtParameter;
        p->removeListener (&dirty);
        pruefe (cp && cr && nachher == vorher && str (p->stateHashText()) == hashVor && p->stateRevision() == revVor
                    && p->zustandLesen().undoRing.size() == undoVor && dirtyMeldungen == 0,
                "312/M-103 hostbypass_beruehrt_den_zustand_nicht (Zustand; speichern<->laden, §7.7, CLAUDE.md State): ueber "
                "40 Bloecke processBlock, 20 Hostpunkte auf Output-Trim, 60 Bloecke processBlockBypassed und 40 Bloecke "
                "processBlock sind die Statebytes vorher und nachher bytegleich, state_hash, Revision und Undo-Ring gleich, "
                "0 Host-Dirty-Meldungen",
                "Statebytes " + std::string (nachher == vorher ? "gleich" : "VERSCHIEDEN") + ", Revision " + zahl (revVor) + " -> "
                + zahl (p->stateRevision()) + ", Undo " + std::to_string (undoVor) + " -> "
                + std::to_string (p->zustandLesen().undoRing.size()) + ", Dirty " + std::to_string (dirtyMeldungen));
        pruefe (cp && cr && lp.e1 == lp.e0 + 1 && lp.offen == lp.e1 && lp.zu == lp.e1 + 1
                    && lr.e1 == lr.e0 + 1 && lr.offen == lr.e1 && lr.zu == lr.e1 + 1,
                "312/M-103 hostbypass_beruehrt_den_zustand_nicht (Ruhegrenze; R-312-27, [SONDE-015] M-81): der Hostbypass "
                "zaehlt verarbeitete Samples weiter - nach 11 776 Samples im Hostbypass steht die Epoche der Geste auf e1 "
                "(offen), nach 12 032 auf e1 + 1 (geschlossen), am selben Block wie bei R ohne Hostbypass",
                "Hostbypass " + zahl (lp.e0) + " -> " + zahl (lp.e1) + ", nach 46 Bloecken " + zahl (lp.offen) + ", nach 47 "
                + zahl (lp.zu) + "; R " + zahl (lr.e0) + " -> " + zahl (lr.e1) + ", " + zahl (lr.offen) + ", " + zahl (lr.zu));
    }

    // ── 312/M-132: der Blockrand des Bypasseintritts ueber den Zaehlerueberlauf
    {
        bool commit = false;
        auto p = hbPruefling (256, commit);
        static_assert (dsp::DspKern::kBlockrandParameter[1] == kOutTrim, "Platz 1 der Abdeckungstabelle ist Output-Trim");
        const int platz = 1;
        p->setzeHostZaehlerFuerTest (kOutTrim, 0xFFFFFFFEu);
        std::uint32_t nach10 = 0, nach12 = 0, nach14 = 0;
        juce::AudioBuffer<float> b (2, 256);
        juce::Random w (13201);
        p->mitAngehaltenemTaktFuerTest ([&]
        {
            for (int blockNr = 0; blockNr < 30; ++blockNr)
            {
                if (blockNr == 10) hostSchreibt (*p, kOutTrim, 3.0f);     // Zaehler 0xFFFFFFFF
                if (blockNr == 12) hostSchreibt (*p, kOutTrim, -3.0f);    // Zaehler 0: der Ueberlauf
                if (blockNr == 14) hostSchreibt (*p, kOutTrim, 1.5f);     // Zaehler 1
                hbFuelle (b, &w);
                hbBlock (*p, true, b);
                if (blockNr == 10) nach10 = p->dspKernFuerTest().blockrandStand (platz);
                if (blockNr == 12) nach12 = p->dspKernFuerTest().blockrandStand (platz);
                if (blockNr == 14) nach14 = p->dspKernFuerTest().blockrandStand (platz);
            }
        });
        const auto hex = [] (std::uint32_t v) { std::ostringstream o; o << "0x" << std::hex << std::uppercase << v; return o.str(); };
        pruefe (commit && nach10 == 0xFFFFFFFFu && nach12 == 0u && nach14 == 1u && p->hostEreignisFuerTest (kOutTrim) == 1u,
                "312/M-132 hostbypass_zaehlerrand (R-312-28, E-312-6; Muster 312/M-82): Ereigniszaehler, Quittierungsstand "
                "und Blockrandstand von Output-Trim bei 0xFFFFFFFE, 30 Bloecke ueber processBlockBypassed mit Hostpunkten "
                "an Block 10, 12 und 14 - der Blockrand des Bypasseintritts nimmt jeden Punkt ueber den Ueberlauf genau "
                "einmal: nach Block 10, 12 und 14 steht der Blockrandstand auf 0xFFFFFFFF, 0 und 1",
                "Blockrandstand nach Block 10 " + hex (nach10) + ", nach 12 " + hex (nach12) + ", nach 14 " + hex (nach14)
                + ", Ereigniszaehler " + hex (p->hostEreignisFuerTest (kOutTrim)));
    }

    // ── 312/M-104: Offline gleich Echtzeit ──────────────────────────────────
    {
        // (a) A allein, Hoerwunsch Dry: der Bypasseintritt setzt weder den
        //     Offline-Riegel noch das harte Schalten.
        bool ca = false;
        auto a = hbPruefling (256, ca);
        a->setzeHoermatrix (dsp::Hoermatrix::dry);
        juce::AudioBuffer<float> b (2, 256);
        juce::Random w (10401);
        for (int i = 0; i < 30; ++i)
        {
            hbFuelle (b, &w);
            hbBlock (*a, i >= 10, b);
        }
        const bool riegel = a->dspKernFuerTest().offlineRiegel();
        const bool dry = a->dspKernFuerTest().wirksameHoermatrix() == dsp::Hoermatrix::dry;
        pruefe (ca && ! riegel && dry,
                "312/M-104 hostbypass_offline_gleich_echtzeit (a) (R-312-29, §7.7 zweite Grenze): am Echtzeitprozessor mit "
                "Hoerwunsch Dry ist nach 20 Bloecken processBlockBypassed offlineRiegel() falsch und wirksameHoermatrix() "
                "gleich dem Wunsch Dry - der Bypasseintritt setzt weder den Riegel noch das harte Schalten",
                std::string ("offlineRiegel ") + (riegel ? "WAHR" : "falsch") + ", wirksame Hoermatrix "
                + (dry ? "dry" : "NICHT dry"));

        // (b) und (c): Bloecke zu 64 - die Blende laeuft ueber vier Bloecke.
        // Hostbypass an Block 10 an, an Block 30 aus, Gleichwert.
        const int b64 = 64;
        std::vector<bool> eintritte (50, false);
        for (int i = 10; i < 30; ++i) eintritte[(size_t) i] = true;
        const auto vorlauf = [b64] (Prozessor& x, bool offline)
        {
            juce::AudioBuffer<float> v (2, b64);
            for (int i = 0; i < 200; ++i)
            {
                if (offline) x.setNonRealtime (true);
                hbFuelle (v, nullptr);
                hbBlock (x, false, v);
            }
            float letzter = v.getSample (0, b64 - 1);
            return letzter;
        };
        const auto pruefeOffline = [&] (const char* fall, bool jederBlock)
        {
            bool c1 = false, c2 = false;
            auto echt = hbPruefling (256, c1);
            auto off  = hbPruefling (256, c2);
            const float verarbeitet = vorlauf (*echt, false);
            vorlauf (*off, jederBlock);
            const auto ya = hbDc (*echt, eintritte, b64);
            const auto yb = hbDc (*off, eintritte, b64, [&] (int i)
            {
                if (jederBlock || i == 11)
                    off->setNonRealtime (true);    // (b): vor dem zweiten Bypassblock, mitten in der Blende
            });
            const int ab = abweichend (ya, yb);
            // Die Blende nach trocken endet 256 Samples nach dem Eintritt (Block 14), die zurueck
            // beginnt an Block 30; dazwischen ist der Ausgang der trockene Gleichwert.
            const auto s = hbSprung (yb, b64, verarbeitet, 14LL * b64, 30LL * b64);
            const bool ok = c1 && c2 && ab == 0 && s.endlich && s.groesster <= s.schranke && s.nichtTrocken == 0
                         && off->isNonRealtime();
            std::ostringstream d;
            d << std::fixed << std::setprecision (9) << fall << ": A gegen B " << ab << " abweichend, groesster "
              << "Nachbarsprung in B " << s.groesster << " (Schranke " << s.schranke << "), zwischen Blendenende und "
              << "Austritt " << s.nichtTrocken << " nicht trocken, B offline " << (off->isNonRealtime() ? "ja" : "NEIN");
            return std::make_pair (ok, d.str());
        };
        const auto [okB, textB] = pruefeOffline ("(b)", false);
        pruefe (okB,
                "312/M-104 hostbypass_offline_gleich_echtzeit (b) ([SONDE-015] M-120, R-312-3, R-312-29): Bloecke zu 64, B "
                "bis zum ersten Bypassblock in Echtzeit, setNonRealtime (true) vor dem zweiten Bypassblock - mitten in der "
                "Blende; A (Echtzeit) und B sind bitgleich, die Blende ist auch offline 256 Samples lang und weich "
                "(E-31): der Wechsel nach offline stellt allein die Hoermatrix hart und laesst die Hostbypass-Blende "
                "weiterlaufen",
                textB);
        const auto [okC, textC] = pruefeOffline ("(c)", true);
        pruefe (okC,
                "312/M-104 hostbypass_offline_gleich_echtzeit (c) ([SONDE-015] M-120, R-312-3): B mit setNonRealtime (true) "
                "vor jedem Block - A und B sind bitgleich, die Blende zaehlt Samples, nie Wandzeit",
                textC);
    }

    // ── 312/M-108: Baenke und Programme im Hostbypass ───────────────────────
    {
        struct Lauf { std::vector<float> imBypass, danach, einBypass; int frei = -1; bool alleCommit = true; };
        const auto lauf = [] (Prozessor& x, bool bypass)
        {
            Lauf l;
            juce::AudioBuffer<float> b (2, 256);
            juce::Random w (10801);
            const auto bloecke = [&] (int n, bool umgangen, std::vector<float>* aus, std::vector<float>* ein)
            {
                for (int i = 0; i < n; ++i)
                {
                    hbFuelle (b, &w);
                    if (ein != nullptr)
                        for (int k = 0; k < 2; ++k)
                            ein->insert (ein->end(), b.getReadPointer (k), b.getReadPointer (k) + 256);
                    hbBlock (x, umgangen, b);
                    if (aus != nullptr)
                        for (int k = 0; k < 2; ++k)
                            aus->insert (aus->end(), b.getReadPointer (k), b.getReadPointer (k) + 256);
                }
            };
            x.mitAngehaltenemTaktFuerTest ([&]
            {
                bloecke (10, false, nullptr, nullptr);
                bloecke (4, bypass, nullptr, nullptr);          // Eintritt, die Blende ist danach vorbei
                auto z = hbSatz();
                z.werte[(size_t) iBand (0, param::kGainDb)].zahl = 6.0;
                l.alleCommit = l.alleCommit && setze (x, z).ausgang == tx::Ausgang::commit;       // Band aendern
                x.kontrollTaktFuerTest();
                bloecke (20, bypass, &l.imBypass, &l.einBypass);
                l.alleCommit = l.alleCommit && ohneNutzlast (x, tx::Art::remove, 0).ausgang == tx::Ausgang::commit;   // Band entfernen
                x.kontrollTaktFuerTest();
                bloecke (20, bypass, &l.imBypass, &l.einBypass);
                auto aus = x.bestaetigterZustand();
                aus.werte[(size_t) param::kIndexEqEnabled].b = false;
                l.alleCommit = l.alleCommit && setze (x, aus).ausgang == tx::Ausgang::commit;     // EQ aus ...
                x.kontrollTaktFuerTest();
                bloecke (20, bypass, &l.imBypass, &l.einBypass);
                aus.werte[(size_t) param::kIndexEqEnabled].b = true;
                l.alleCommit = l.alleCommit && setze (x, aus).ausgang == tx::Ausgang::commit;     // ... und wieder an
                x.kontrollTaktFuerTest();
                bloecke (20, bypass, &l.imBypass, &l.einBypass);
                x.kontrollTaktFuerTest();
                l.frei = x.dspKernFuerTest().pool().freieSlots();
                bloecke (20, false, &l.danach, nullptr);        // der Austritt
            });
            return l;
        };
        bool cp = false, cr = false;
        auto p = hbPruefling (256, cp);
        auto r = hbPruefling (256, cr);
        const auto lp = lauf (*p, true);
        const auto lr = lauf (*r, false);
        const int imBypassAbw = abweichend (lp.imBypass, lp.einBypass);
        // Nach dem Austritt: ab Sample 256 des ersten Blocks (ein Block zu 256) bitgleich zu R.
        const std::vector<float> nachP (lp.danach.begin() + 512, lp.danach.end());
        const std::vector<float> nachR (lr.danach.begin() + 512, lr.danach.end());
        const int nachAbw = abweichend (nachP, nachR);
        pruefe (cp && cr && lp.alleCommit && lr.alleCommit && lp.frei == lr.frei && lp.frei >= 0 && imBypassAbw == 0
                    && nachAbw == 0,
                "312/M-108 hostbypass_haelt_keine_bank ([SONDE-015] M-44, E-17; aktivieren<->abklingen): im Hostbypass nach "
                "der Blende vier Transaktionen - Band aendern, Band entfernen, EQ aus und wieder an -, je ein Kontrolltakt "
                "und 20 Bloecke processBlockBypassed: der Kern nimmt die Programme am Blockrand wie ohne Hostbypass, nach "
                "der Folge und einem Takt sind genau so viele Baenke frei wie bei R, der Ausgang bleibt bytegleich zum "
                "Eingang, und nach dem Austritt klingt der zuletzt bestaetigte Stand (ab Sample 256 bitgleich zu R)",
                "freie Baenke " + std::to_string (lp.frei) + " (R " + std::to_string (lr.frei) + "), im Hostbypass "
                + std::to_string (imBypassAbw) + " abweichend vom Eingang, nach dem Austritt " + std::to_string (nachAbw)
                + " abweichend von R, Commits " + (lp.alleCommit ? "alle" : "NICHT alle"));
    }

    // ── Kosten der Hostbypass-Stufe (Messung, keine Schranke; Manifest §47.3,
    // Muster §21.7). Gemessen wird allein die Zeit IM Eintritt; je Block wird
    // derselbe Eingang ungemessen neu eingelegt (der Trim hebt ihn sonst Block
    // fuer Block). Je Eintritt das Minimum aus drei Durchgaengen. ────────────
    {
        bool commit = false;
        auto p = hbPruefling (64, commit);
        const int n = 20000;
        juce::AudioBuffer<float> vorlage (2, 64), puffer (2, 64);
        juce::Random w (3290);
        for (int k = 0; k < 2; ++k)
            for (int s = 0; s < 64; ++s)
                vorlage.setSample (k, s, w.nextFloat() * 0.2f - 0.1f);
        const auto messe = [&] (bool bypass)
        {
            double bestes = std::numeric_limits<double>::infinity();
            for (int durchgang = 0; durchgang < 3; ++durchgang)
            {
                double sekunden = 0.0;
                p->mitAngehaltenemTaktFuerTest ([&]
                {
                    for (int i = 0; i < 8; ++i)      // der Eintritt ist gewechselt, die Blende vorbei
                    {
                        puffer.makeCopyOf (vorlage, true);
                        hbBlock (*p, bypass, puffer);
                    }
                    for (int i = 0; i < n; ++i)
                    {
                        puffer.makeCopyOf (vorlage, true);
                        const auto start = std::chrono::steady_clock::now();
                        hbBlock (*p, bypass, puffer);
                        sekunden += std::chrono::duration<double> (std::chrono::steady_clock::now() - start).count();
                    }
                });
                bestes = std::min (bestes, sekunden * 1.0e9 / (double) n);
            }
            return bestes;
        };
        const double normal = messe (false);
        const double umgangen = messe (true);
        std::ostringstream o;
        o << std::fixed << std::setprecision (1);
        o << "  MESSUNG 312/Kosten-Hostbypass sizeof(dsp::DspKern) " << sizeof (dsp::DspKern) << " Byte, sizeof(SondeProcessor) "
          << sizeof (Prozessor) << " Byte; Aufbau HB, " << n << " Bloecke zu 64 Samples je Eintritt, Zeit im Eintritt "
          << "(Minimum aus drei Durchgaengen): processBlock " << normal << " ns je Block, processBlockBypassed nach der "
          << "Blende " << umgangen << " ns je Block; Transaktion " << (commit ? "commit" : "FEHLER");
        std::cout << o.str() << std::endl;
    }
}

} // namespace

int main (int argc, char* argv[])
{
    // NAK-312 Etappe 5 (312/M-48, 312/M-49): der Erzeuger des Parametergoldens.
    // Er schreibt nur und prueft nichts; der Kanon faehrt das Bein ohne Argument.
    if (argc == 4 && std::string (argv[1]) == "--parameter-golden")
    {
        const auto g = parameterGolden();
        const bool text = juce::File (argv[2]).replaceWithData (g.text.data(), g.text.size());
        const bool bin  = juce::File (argv[3]).replaceWithData (g.s1.getData(), g.s1.getSize());
        std::cout << "Parametergolden: " << argv[2] << " (" << g.text.size() << " Byte), "
                  << argv[3] << " (" << g.s1.getSize() << " Byte)" << std::endl;
        return text && bin ? 0 : 2;
    }

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

    // NAK-283 Etappe 4: Host-Reset, Lebenszyklus-Trio und Analysezufuehrung
    nak283Hosteintritte();
    nak283Analysezufuehrung();

    // NAK-311 Etappe 3 (W01): Host-Reset im Ausblenden
    nak311Pfadrampen();

    // NAK-311 Etappe 3 (W03): Remove und Neubelegung ohne Audio
    nak311SlotLebenszyklus();

    // NAK-311 Etappe 4 Teil b (F12, R-311-4): Recall und Audiohistorie
    nak311Recall();

    // NAK-311 Etappe 5, Satz A (F08, R-311-16, R-311-20): die Ratenschranke
    nak311Ratenschranke();

    // NAK-311 Etappe 5, Satz B (U54, R-311-14): die Obergrenze des Ausgleichs
    nak311AutoGainDeckel();

    // NAK-312 Etappe 2 (R-312-1, E-312-11): die Echtzeitwache ab dem Callback-Eintritt
    nak312Messgeraete();

    // NAK-312 Etappe 3a (W02, R-312-10 zweiter Teil, R-312-11): der Ladestart
    nak312Ladestart();

    // NAK-312 Etappe 3b (T3-01-05 Teil a, E-312-5, E-312-6): die Blockbindung
    nak312Blockbindung();

    // NAK-312 Etappe 4 (W09, R-312-7): die Besitzschleuse liegt nicht im Audiopfad
    nak312Besitz();

    // NAK-312 Etappe 5, erster Aenderungssatz (T3-01-03, T3-01-04; R-312-3, E-312-7): der Offline-Uebergang
    nak312OfflineUebergang();

    // NAK-312 Etappe 5, zweiter Aenderungssatz (T3-01-02; R-312-4): die Parameteranbindung ohne APVTS
    nak312Parameteranbindung();

    // NAK-312 Etappe 5, Nacharbeit 1 (R-312-19): kein Weg zum Hostende im Pluginquelltext
    nak312HostEnde();

    // NAK-312 Etappe 7b, Satz 2 (T3-01-09, U48, U58): der Hostbypass am echten SondeProcessor
    nak312Hostbypass();

    std::cout << std::endl << geprueft << " geprueft, " << fehler << " Fehler" << std::endl;
    std::cout << (fehler == 0 ? "TRANSAKTION OK" : "TRANSAKTION FEHLGESCHLAGEN") << std::endl;
    return fehler == 0 ? 0 : 1;
}
