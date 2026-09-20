/*  Kanon B6 - EqCopDspGoldenTest (SONDE-015 Etappe 3, Bauplan §4.3).

    Misst den aktiven DSP-Kern `eq-copilot/plugin/dsp/` gegen die Zusagen
    der Verhaltensmatrix §3.1 bis §3.6 und §3.13 von
    `docs/beweise/SONDE-015.md`, dazu aus §3.14 den Messwert des
    Vier-Bank-Falls (M-118, Abschnitt N) und die Haelfte des Kerns von M-120
    (Abschnitt O) - beides seit der Audit-Nacharbeit vom 10.09.2026. Seit
    NAK-311 Etappe 2 dazu die Neutralpruefung des Kerns (Manifest NAK-311
    §6.1: 311/M-12, M-13 mit M-14, M-18, der neutrale Schritt in M-22 und die
    Tapvergleiche M-90, M-91). Seit NAK-311 Etappe 3 (W01) Abschnitt P: die
    Pfadrampen in der Ruhe gegen einen frischen Kern als Orakel (§6.2:
    311/M-23 bis M-38; M-34 misst B7 am Prozessor). Seit W03 der
    Slot-Lebenszyklus gegen einen Referenzkern ohne Wechsel (§6.3): 311/M-43
    bis M-45, M-54, M-55 in Abschnitt E, 311/M-41 im M-121-Block von L, der
    Fremdslotwechsel im Allokationslauf von J (M-56) und Abschnitt Q
    (311/M-46 bis M-51, M-53, M-94, M-95); M-40 misst B7. Seit NAK-311
    Etappe 4 Teil a (R-311-3, T3-16-04) der neue Abschnitt H2: im Monobus
    (`bereiteVor` mit Kanalzahl 1) rechnet die Auto-Gain-Ableitung die
    geordnete Kaskade und wertet nur den ausgegebenen Kanal aus - 311/M-57
    bis M-61, M-64 und M-65 gegen eine HIER eigenstaendig ausgeschriebene
    Monoformel und gegen Hexwerte, die der unveraenderte Kern am Basisstand
    der Etappe ausgegeben hat; 311/M-66 (Rechenort auch im Monokern) steht
    im M-39-Fall des Abschnitts H. Seit Etappe 4 Teil b (T3-15-11) misst
    Abschnitt F die STEUERRATE gegen einen Referenzkern mit Range 0
    (311/M-73 bis M-75, §6.4): der erste Entwurf wirkt 1 bis 8 Samples
    anteilig und 8 bis 15 Samples voll - erlaubtes Verhalten, gemessen, kein
    geaendertes Verhalten.

    WIE DER FILTERGOLDEN MISST - und warum nicht anders (M-13, §5.15):

      Der Golden misst den AUDIOPFAD, nicht die Koeffizientenformel. Je
      Pruefling laeuft ein Impuls durch den echten `DspKern`; abgegriffen
      wird der Tap `post_committed` (er traegt `double`, nicht den
      float-Ausgang), und die gemessene Uebertragung wird gegen eine HIER
      eigenstaendig ausgeschriebene RBJ-Formel gehalten. Ein Vergleich der
      Produktkoeffizienten mit `biquadLeistung()` derselben Koeffizienten
      bliebe gruen, selbst wenn der Audiopfad die Bank gar nicht anwendet -
      genau die Klasse ungueltiger Rotbeweise, gegen die die Matrixpruefung
      von NAK-182 zweimal fand.

      Dass die Impulsantwort dasselbe misst wie ein eingeschwungener Sinus,
      wird nicht behauptet, sondern in Abschnitt C1 an drei Stellen je
      Pruefling GEGENGEPRUEFT (Abweichung 1 im Manifest §9.6).

    LANDMINE NAK-175: der MSVC-Standardstack ist 1 MiB. `DspKern` traegt
    vier Baenke mit acht Baendern in `double`; jede Testfunktion legt ihn
    deshalb auf den HEAP (`std::unique_ptr`), nie im Rahmen. Der Fund zeigt
    sich sonst erst im vollen Kanon, waehrend das Bein allein gruen laeuft.

    ALLOKATIONSZAEHLER: thread_local wie in B4. Der Control-Worker DARF
    allozieren (er baut Programme und rechnet 121 Gitterstellen); der
    Audiothread nicht. Ein globaler Zaehler koennte beide nicht trennen.
    Der Ueberschreiber verdrahtet zusaetzlich `RtWache::meldeAllokation`,
    damit der Zaehler DES KERNS echte Allokationen sieht und kein
    strukturelles Null bleibt.

    Exit 0 nur bei "DSP-GOLDEN OK".
*/

#include "DspKern.h"
#include "NakamaParameter.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace nakama::dsp;
namespace param = nakama::parameter;

//==============================================================================
namespace
{
    thread_local bool          zaehleAllokationen = false;
    thread_local std::uint64_t allokationen       = 0;
}

void* operator new (std::size_t groesse)
{
    if (zaehleAllokationen) { ++allokationen; RtWache::meldeAllokation(); }
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAllokationen) { ++allokationen; RtWache::meldeAllokation(); }
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
    if (ok)
    {
        std::cout << "  ok      " << was;
        if (! detail.empty()) std::cout << "  [" << detail << "]";
        std::cout << std::endl;
    }
    else
    {
        ++fehler;
        std::cout << "  FEHLER  " << was;
        if (! detail.empty()) std::cout << "  [" << detail << "]";
        std::cout << std::endl;
    }
}

std::string zahl (double d, int stellen = 6)
{
    std::ostringstream o;
    o << std::fixed << std::setprecision (stellen) << d;
    return o.str();
}

/** Ein `float` aus seinem Bitmuster - ohne Konstantfaltung, und genau darauf
    kommt es an.

    MSVC haelt Gleitkommakonstanten intern als `double`. Ein KONSTANTGEFALTETES
    float-sNaN laeuft dabei durch float -> double -> float und wird dabei
    RUHIG: aus 0x7F800001 wird 0x7FC00001. Die Wachmarke waere dann schon
    ruhig, bevor der Kern sie sieht - und jede Probe "hat der Kern
    geschrieben?" saehe faelschlich "nein", weil ein ruhiger NaN durch den
    Rueckweg float -> double -> float bytegleich zurueckkommt. Ob der Compiler
    faltet, haengt vom Inline-Pfad der umgebenden Funktion ab; eine Aenderung
    an ganz anderer Stelle desselben `main` kippt es (gemessen NAK-311 Etappe 5
    Satz B, 20.09.2026: 311/M-13 und 311/M-18 fielen mit `marke=7fc00001`).

    Der `volatile` Zwischenschritt verbietet die Faltung: das Bitmuster wird
    zur Laufzeit gelesen und mit `memcpy` uebernommen, danach traegt jede
    float-Kopie (`movss`) es unveraendert weiter. */
float ausBitmuster (std::uint32_t bits) noexcept
{
    volatile std::uint32_t fluechtig = bits;
    const std::uint32_t    gelesen   = fluechtig;
    float f = 0.0f;
    std::memcpy (&f, &gelesen, sizeof (f));
    return f;
}

//==============================================================================
// Der DTO-Bau: alles laeuft ueber `nakama::parameter`, es gibt keine zweite
// Wahrheit fuer Grenzen, Defaults oder Reihenfolge.

param::DspSatz machSatz (bool eqAn = true)
{
    param::DspSatz s;
    s.werte[(size_t) param::kIndexEqEnabled].b = eqAn;
    return s;
}

void setzeGlobal (param::DspSatz& s, const char* id, double wert)
{
    const int i = param::indexVonId (id);
    if (i >= 0) s.werte[(size_t) i].zahl = wert;
}

void setzeGlobalBool (param::DspSatz& s, const char* id, bool wert)
{
    const int i = param::indexVonId (id);
    if (i >= 0) s.werte[(size_t) i].b = wert;
}

/** Belegt einen Slot und schaltet ihn ein. `occupied` UND `enabled` - ein
    freier Slot verarbeitet nichts, gleichgueltig was seine Werte sagen. */
void belege (param::DspSatz& s, int slot, Filtertyp typ, double f, double q, double gainDb,
             Kanalmodus modus = Kanalmodus::stereo)
{
    s.werte[(size_t) param::indexOccupied (slot)].b = true;
    s.werte[(size_t) param::indexBandV1 (slot, param::kEnabled)].b = true;
    s.werte[(size_t) param::indexBandV1 (slot, param::kType)].enumIndex = (int) typ;
    s.werte[(size_t) param::indexBandV1 (slot, param::kFreqHz)].zahl = f;
    s.werte[(size_t) param::indexBandV1 (slot, param::kQ)].zahl = q;
    s.werte[(size_t) param::indexBandV1 (slot, param::kGainDb)].zahl = gainDb;
    s.werte[(size_t) param::indexBandV1 (slot, param::kChannelMode)].enumIndex = (int) modus;
}

void machDynamisch (param::DspSatz& s, int slot, double rangeDb, double thresholdDb,
                    double attackMs = 10.0, double holdMs = 0.0, double releaseMs = 100.0,
                    Sidechain quelle = Sidechain::internal)
{
    s.werte[(size_t) param::indexBandV1 (slot, param::kDynamicEnabled)].b = true;
    s.werte[(size_t) param::indexBandV1 (slot, param::kDynamicRangeDb)].zahl = rangeDb;
    s.werte[(size_t) param::indexBandV1 (slot, param::kThresholdDb)].zahl = thresholdDb;
    s.werte[(size_t) param::indexBandV1 (slot, param::kAttackMs)].zahl = attackMs;
    s.werte[(size_t) param::indexBandV1 (slot, param::kHoldMs)].zahl = holdMs;
    s.werte[(size_t) param::indexBandV1 (slot, param::kReleaseMs)].zahl = releaseMs;
    s.werte[(size_t) param::indexBandV1 (slot, param::kSidechainSource)].enumIndex = (int) quelle;
}

//==============================================================================
// Die analytische RBJ-Referenz - EIGENSTAENDIG ausgeschrieben (M-13).
//
// Sie steht hier bewusst noch einmal und nicht als Aufruf von
// `nakama::dsp::entwurf()`: ein Golden, der die Produktformel gegen sich
// selbst haelt, ist eine Tautologie. Quelle ist dieselbe wie dort - das
// RBJ Audio EQ Cookbook -, aber der Text ist unabhaengig.

struct RefBiquad { double b0, b1, b2, a1, a2; };

constexpr double kPiRef = 3.14159265358979323846;

RefBiquad refEntwurf (Filtertyp typ, double fs, double f0, double q, double gainDb)
{
    const double w0 = 2.0 * kPiRef * f0 / fs;
    const double cw = std::cos (w0);
    const double sw = std::sin (w0);
    const double al = sw / (2.0 * q);
    RefBiquad r { 1.0, 0.0, 0.0, 0.0, 0.0 };

    if (typ == Filtertyp::bell)
    {
        const double A  = std::sqrt (std::pow (10.0, gainDb / 20.0));
        const double a0 = 1.0 + al / A;
        r.b0 = (1.0 + al * A) / a0;
        r.b1 = (-2.0 * cw)    / a0;
        r.b2 = (1.0 - al * A) / a0;
        r.a1 = (-2.0 * cw)    / a0;
        r.a2 = (1.0 - al / A) / a0;
    }
    else if (typ == Filtertyp::lowShelf || typ == Filtertyp::highShelf)
    {
        const double A  = std::sqrt (std::pow (10.0, gainDb / 20.0));
        const double sq = std::sqrt (A);
        const double tw = 2.0 * sq * al;
        if (typ == Filtertyp::lowShelf)
        {
            const double a0 = (A + 1.0) + (A - 1.0) * cw + tw;
            r.b0 =        A * ((A + 1.0) - (A - 1.0) * cw + tw) / a0;
            r.b1 =  2.0 * A * ((A - 1.0) - (A + 1.0) * cw)      / a0;
            r.b2 =        A * ((A + 1.0) - (A - 1.0) * cw - tw) / a0;
            r.a1 = -2.0     * ((A - 1.0) + (A + 1.0) * cw)      / a0;
            r.a2 =           ((A + 1.0) + (A - 1.0) * cw - tw)  / a0;
        }
        else
        {
            const double a0 = (A + 1.0) - (A - 1.0) * cw + tw;
            r.b0 =        A * ((A + 1.0) + (A - 1.0) * cw + tw) / a0;
            r.b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cw)      / a0;
            r.b2 =        A * ((A + 1.0) + (A - 1.0) * cw - tw) / a0;
            r.a1 =  2.0     * ((A - 1.0) - (A + 1.0) * cw)      / a0;
            r.a2 =           ((A + 1.0) - (A - 1.0) * cw - tw)  / a0;
        }
    }
    else
    {
        const double a0 = 1.0 + al;
        if (typ == Filtertyp::notch)
        {
            r.b0 = 1.0            / a0;
            r.b1 = (-2.0 * cw)    / a0;
            r.b2 = 1.0            / a0;
        }
        else if (typ == Filtertyp::lowCut)
        {
            r.b0 =  (1.0 + cw) / 2.0 / a0;
            r.b1 = -(1.0 + cw)       / a0;
            r.b2 =  (1.0 + cw) / 2.0 / a0;
        }
        else
        {
            r.b0 = (1.0 - cw) / 2.0 / a0;
            r.b1 = (1.0 - cw)       / a0;
            r.b2 = (1.0 - cw) / 2.0 / a0;
        }
        r.a1 = (-2.0 * cw)  / a0;
        r.a2 = (1.0 - al)   / a0;
    }
    return r;
}

double refDb (const RefBiquad& r, double fs, double f)
{
    const double w = 2.0 * kPiRef * f / fs;
    const std::complex<double> z (std::cos (-w), std::sin (-w));
    const std::complex<double> z2 = z * z;
    const auto h = (r.b0 + r.b1 * z + r.b2 * z2) / (1.0 + r.a1 * z + r.a2 * z2);
    const double betrag = std::abs (h);
    return betrag > 0.0 ? 20.0 * std::log10 (betrag) : -400.0;
}

/** Die dB-Abweichung einer Gitterstelle. An einer NULLSTELLE der Referenz -
    der exakten Mittenfrequenz eines Notch (B-24 fuegt sie ins Gitter) - hat
    ein dB-Abstand keinen Massstab: die Referenz liegt bei -300 dB, die
    gemessene Impulsantwort an ihrem numerischen Boden. Unter -60 dB gilt
    deshalb: gemessen muss ebenfalls unter -60 dB liegen (Entscheid E-27). */
double abweichungDb (double gemessenDb, double sollDb)
{
    const double boden = -60.0;
    if (sollDb < boden) return gemessenDb < boden ? 0.0 : std::abs (gemessenDb - boden);
    return std::abs (gemessenDb - sollDb);
}

//==============================================================================
// Der Messweg durch den ECHTEN Audiopfad.

/** Faehrt `samples` Stille durch den Kern - fuer Fades und Einschwingen. */
void fahreStille (DspKern& kern, int samples, int blockGroesse = 256)
{
    std::vector<float> l ((size_t) blockGroesse, 0.0f), r ((size_t) blockGroesse, 0.0f);
    float* kanaele[2] = { l.data(), r.data() };
    int rest = samples;
    while (rest > 0)
    {
        const int n = rest < blockGroesse ? rest : blockGroesse;
        std::fill (l.begin(), l.end(), 0.0f);
        std::fill (r.begin(), r.end(), 0.0f);
        kern.verarbeite (kanaele, 2, n);
        rest -= n;
    }
}

/** Misst die Impulsantwort AM TAP `post_committed` - im echten Audiopfad,
    in `double`. Der Vorlauf faehrt den Crossfade zu Ende, damit die
    Antwort nicht die Fadeflanke traegt. */
std::vector<double> impulsantwort (DspKern& kern, int laenge, int kanal = 0,
                                   int blockGroesse = 256, int vorlauf = 1024)
{
    fahreStille (kern, vorlauf, blockGroesse);

    std::vector<double> ir;
    ir.reserve ((size_t) laenge);

    std::vector<float> l ((size_t) blockGroesse, 0.0f), r ((size_t) blockGroesse, 0.0f);
    float* kanaele[2] = { l.data(), r.data() };

    bool impulsGesetzt = false;
    while ((int) ir.size() < laenge)
    {
        std::fill (l.begin(), l.end(), 0.0f);
        std::fill (r.begin(), r.end(), 0.0f);
        if (! impulsGesetzt) { l[0] = 1.0f; r[0] = 1.0f; impulsGesetzt = true; }

        kern.verarbeite (kanaele, 2, blockGroesse);

        const double* tap = kern.tap (Tap::postCommitted, kanal);
        if (tap == nullptr) break;
        for (int i = 0; i < blockGroesse && (int) ir.size() < laenge; ++i)
            ir.push_back (tap[i]);
    }
    return ir;
}

/** |H(f)| in dB aus einer Impulsantwort. */
double irDb (const std::vector<double>& ir, double fs, double f)
{
    const double w = 2.0 * kPiRef * f / fs;
    double re = 0.0, im = 0.0;
    for (size_t n = 0; n < ir.size(); ++n)
    {
        const double phase = w * (double) n;
        re += ir[n] * std::cos (phase);
        im -= ir[n] * std::sin (phase);
    }
    const double betrag = std::sqrt (re * re + im * im);
    return betrag > 0.0 ? 20.0 * std::log10 (betrag) : -400.0;
}

/** Der eingeschwungene Sinus - die Gegenprobe zur Impulsantwort (C1).
    Gemessen wird ueber eine GANZE Zahl von Perioden, damit keine Leckage
    das Ergebnis verschiebt. */
double sinusDb (DspKern& kern, double fs, double f, int perioden = 64, int kanal = 0)
{
    const int blockGroesse = 512;
    const int einschwingen = 8192;
    const int messSamples  = (int) std::llround ((double) perioden * fs / f);

    std::vector<float> l ((size_t) blockGroesse), r ((size_t) blockGroesse);
    float* kanaele[2] = { l.data(), r.data() };

    const double w = 2.0 * kPiRef * f / fs;
    long long n = 0;
    double summe = 0.0;
    int gemessen = 0;

    while (gemessen < messSamples)
    {
        for (int i = 0; i < blockGroesse; ++i)
        {
            const double x = std::sin (w * (double) (n + i));
            l[(size_t) i] = (float) x;
            r[(size_t) i] = (float) x;
        }
        kern.verarbeite (kanaele, 2, blockGroesse);
        const double* tap = kern.tap (Tap::postCommitted, kanal);
        if (tap == nullptr) return -400.0;

        if (n >= einschwingen)
            for (int i = 0; i < blockGroesse && gemessen < messSamples; ++i, ++gemessen)
                summe += tap[i] * tap[i];

        n += blockGroesse;
    }

    const double amplitude = std::sqrt (2.0 * summe / (double) messSamples);
    return amplitude > 0.0 ? 20.0 * std::log10 (amplitude) : -400.0;
}

/** Das Gitter aus §5.15 Feinheit 3: 1/24 Oktave von 20 Hz bis
    min(20 kHz, 0,45 fs). */
std::vector<double> gitter (double fs)
{
    const double von = 20.0;
    const double bis = std::min (20000.0, fs * kNyquistAnteil);
    std::vector<double> g;
    const double schritt = std::pow (2.0, 1.0 / 24.0);
    for (double f = von; f <= bis * 1.0000001; f *= schritt) g.push_back (f);
    if (g.empty() || g.back() < bis) g.push_back (bis);
    return g;
}

/** B-24, §5.15 Feinheit 3 woertlich: das Gitter JE PRUEFLING - zusaetzlich die
    exakte Eck- oder Mittenfrequenz und bei Shelves die Plateaupunkte f/4 und
    4f, soweit sie in 20 Hz..min(20 kHz, 0,45 fs) liegen. */
std::vector<double> gitterFuer (double fs, Filtertyp typ, double f0)
{
    auto g = gitter (fs);
    const double bis = std::min (20000.0, fs * kNyquistAnteil);
    auto dazu = [&] (double f) { if (f >= 20.0 && f <= bis) g.push_back (f); };
    dazu (f0);
    if (typ == Filtertyp::lowShelf || typ == Filtertyp::highShelf) { dazu (f0 / 4.0); dazu (f0 * 4.0); }
    std::sort (g.begin(), g.end());
    g.erase (std::unique (g.begin(), g.end()), g.end());
    return g;
}

/** Traegt das Gitter die Pflichtpunkte des Prueflings? */
bool gitterEnthaelt (const std::vector<double>& g, double fs, Filtertyp typ, double f0)
{
    const double bis = std::min (20000.0, fs * kNyquistAnteil);
    auto hat = [&] (double f) { return f < 20.0 || f > bis || std::find (g.begin(), g.end(), f) != g.end(); };
    bool ok = hat (f0);
    if (typ == Filtertyp::lowShelf || typ == Filtertyp::highShelf) ok = ok && hat (f0 / 4.0) && hat (f0 * 4.0);
    return ok;
}

/** Faehrt einen STEREO-Ton durch den Kern: L = a*sin, R = a*cos. Nach dem
    Detektor-Bandpass auf der Tonfrequenz ist die gemeinsame Leistung beider
    Komponenten (E-5) damit KONSTANT a^2/2 - ohne die Welligkeit eines
    einkanaligen Sinus, die jede Zeit- und Kniemessung verschmieren wuerde
    (Entscheid E-25). `aufzeichnung` nimmt den Tap post_committed L auf. */
void fahreStereoTon (DspKern& k, double fs, double f, double amplitude, long long& n0, int samples,
                     int blockGroesse, std::vector<double>* aufzeichnung = nullptr,
                     std::vector<double>* auslenkung = nullptr, int slot = 0)
{
    std::vector<float> a ((size_t) blockGroesse), b ((size_t) blockGroesse);
    float* kan[2] = { a.data(), b.data() };
    const double w = 2.0 * kPiRef * f / fs;
    int rest = samples;
    while (rest > 0)
    {
        const int m = rest < blockGroesse ? rest : blockGroesse;
        for (int i = 0; i < m; ++i)
        {
            a[(size_t) i] = (float) (amplitude * std::sin (w * (double) (n0 + i)));
            b[(size_t) i] = (float) (amplitude * std::cos (w * (double) (n0 + i)));
        }
        k.verarbeite (kan, 2, m);
        if (aufzeichnung != nullptr)
        {
            const double* t = k.tap (Tap::postCommitted, 0);
            for (int i = 0; i < m; ++i) aufzeichnung->push_back (t != nullptr ? t[i] : 0.0);
        }
        if (auslenkung != nullptr)
        {
            double werte[param::kSlots];
            k.auslenkungenDb (werte);
            for (int i = 0; i < m; ++i) auslenkung->push_back (werte[slot]);
        }
        n0 += m;
        rest -= m;
    }
}

/** Amplitude eines eingeschwungenen Tons aus den letzten `perioden` ganzen
    Perioden einer Aufzeichnung. */
double tonAmplitude (const std::vector<double>& x, double fs, double f, int perioden = 32)
{
    const size_t laenge = (size_t) std::llround ((double) perioden * fs / f);
    if (x.size() < laenge) return 0.0;
    double summe = 0.0;
    for (size_t i = x.size() - laenge; i < x.size(); ++i) summe += x[i] * x[i];
    return std::sqrt (2.0 * summe / (double) laenge);
}

/** Groesster Sprung zwischen zwei Folgesamples, einschliesslich des letzten
    Werts VOR der Aufzeichnung. */
double groessterSprung (double vorher, const std::vector<double>& x)
{
    double m = x.empty() ? 0.0 : std::abs (x[0] - vorher);
    for (size_t i = 1; i < x.size(); ++i) m = std::max (m, std::abs (x[i] - x[i - 1]));
    return m;
}

/** W-4 (Entscheid E-31): die Rundungstoleranz einer Nachbarsample-Differenz
    am float-Ausgang. Jeder Wert unter 1,0 liegt auf einem float-Raster von
    hoechstens 2^-24; zwei gerundete Nachbarn verschieben ihre Differenz um
    hoechstens zwei Rasterschritte, also 2^-23 = 1,19e-7 - vier
    Groessenordnungen unter den Fadeschrittweiten dieser Proben (um 2e-3).
    Die double-Rechnung davor liegt bei 1e-16. */
constexpr double kRundungFloat = 1.0 / 8388608.0;

/** W-4 (Entscheid E-31): die Schranke der WACHE ueber einen ganzen
    Crossfade-Lauf, hergeleitet aus dem Einschwingen der KALT startenden
    Zielbank. Die Quellbank steht eingeschwungen auf `quelle`; die Zielbank
    startet mit Zustand 0 auf dem konstanten Eingang `x` und liefert y(n) -
    gerechnet mit der eigenstaendigen RBJ-Referenz dieses Tests. Mit
    t = n/K ist der Ausgang o(n) = quelle*(1 - t) + y(n)*t, also

        o(n) - o(n-1) = (y(n-1) - quelle)/K + t*(y(n) - y(n-1))   fuer n <= K
        o(n) - o(n-1) = y(n) - y(n-1)                             danach.

    Die Schranke ist das Maximum des Betrags je Sample nach der
    Dreiecksungleichung - kein Faktor. */
double kaltSchranke (Filtertyp typ, double fs, double f0, double q, double gainDb, double quelle, double x)
{
    const auto c = refEntwurf (typ, fs, f0, q, gainDb);
    const double K = (double) kFadeSamples;
    double z1 = 0.0, z2 = 0.0, yVor = 0.0, m = 0.0;
    for (int n = 0; n < 2 * kFadeSamples; ++n)
    {
        const double y = c.b0 * x + z1;
        z1 = c.b1 * x - c.a1 * y + z2;
        z2 = c.b2 * x - c.a2 * y;
        if (n >= 1)
        {
            const double t      = n >= kFadeSamples ? 1.0 : (double) n / K;
            const double schritt = n > kFadeSamples ? 0.0 : std::abs (yVor - quelle) / K;
            m = std::max (m, schritt + t * std::abs (y - yVor));
        }
        yVor = y;
    }
    return m;
}

/** Faehrt DC durch den Kern und zeichnet den AUSGANG (float) auf. */
std::vector<double> fahreDc (DspKern& k, double wert, int samples, int blockGroesse)
{
    std::vector<float> a ((size_t) blockGroesse), b ((size_t) blockGroesse);
    float* kan[2] = { a.data(), b.data() };
    std::vector<double> aus;
    int rest = samples;
    while (rest > 0)
    {
        const int m = rest < blockGroesse ? rest : blockGroesse;
        std::fill (a.begin(), a.end(), (float) wert);
        std::fill (b.begin(), b.end(), (float) wert);
        k.verarbeite (kan, 2, m);
        for (int i = 0; i < m; ++i) aus.push_back ((double) a[(size_t) i]);
        rest -= m;
    }
    return aus;
}

std::unique_ptr<DspKern> neuerKern (double fs, int maxBlock = 2048)
{
    // NAK-175: auf den HEAP. Vier Baenke mit acht Baendern in `double`
    // gehoeren nicht in einen 1-MiB-Rahmen.
    auto k = std::make_unique<DspKern>();
    k->bereiteVor (fs, maxBlock);
    return k;
}

const char* typName (Filtertyp t)
{
    switch (t)
    {
        case Filtertyp::bell:      return "bell";
        case Filtertyp::lowShelf:  return "low_shelf";
        case Filtertyp::highShelf: return "high_shelf";
        case Filtertyp::notch:     return "notch";
        case Filtertyp::lowCut:    return "low_cut";
        case Filtertyp::highCut:   return "high_cut";
    }
    return "?";
}

//==============================================================================
// NAK-311 W03 (Manifest NAK-311 §6.3, R-311-1): die Messwege des
// Slot-Lebenszyklus. Das MASS (F-9) ist der groesste Betrag von
// 20*log10 (Perioden-RMS mit Wechsel / Perioden-RMS des Referenzkerns) ueber
// volle Perioden des Prueftons auf beiden Kanaelen, ab dem ersten Sample des
// Umschaltblocks, der auf einer Periodengrenze beginnt. Der Referenzkern hat
// dieselbe Vorgeschichte und bekommt denselben Eingang in derselben
// Blockteilung, nur ohne den Wechsel. Alles hier ist eigenstaendig
// ausgeschrieben; der Kern liefert nur Ausgang, Taps und Auslenkungen.

/** Was ein Lauf ab dem Umschaltblock aufzeichnet: den float-Ausgang und den
    Tap post_candidate (beide L und R verschraenkt, ein fehlender Tap als
    NaN), je Block die acht Committed-Auslenkungen und die rechnende
    Crossfade-Quelle des Committed-Pfades (-1 = keine, also Rampe oder kein
    Uebergang). */
struct W03Lauf
{
    std::vector<double> aus, kandidat;
    std::vector<std::array<double, (size_t) param::kSlots>> auslenkungen;
    std::vector<int> kreuzQuelle;
};

/** Faehrt `samples` Samples aus `quelle (n, l, r)` ab dem Stromindex `n0` in
    Bloecken zu `blockGroesse` (der letzte kuerzer); zeichnet in `lauf` auf,
    wenn gesetzt. */
template <typename Quelle>
void fahreW03 (DspKern& k, const Quelle& quelle, long long& n0, int samples, int blockGroesse, W03Lauf* lauf)
{
    std::vector<float> a ((size_t) blockGroesse), b ((size_t) blockGroesse);
    float* kan[2] = { a.data(), b.data() };
    for (int rest = samples; rest > 0;)
    {
        const int m = std::min (rest, blockGroesse);
        for (int i = 0; i < m; ++i) quelle (n0 + i, a[(size_t) i], b[(size_t) i]);
        k.verarbeite (kan, 2, m);
        if (lauf != nullptr)
        {
            const double* cl = k.tap (Tap::postCandidate, 0);
            const double* cr = k.tap (Tap::postCandidate, 1);
            for (int i = 0; i < m; ++i)
            {
                lauf->aus.push_back ((double) a[(size_t) i]);
                lauf->aus.push_back ((double) b[(size_t) i]);
                lauf->kandidat.push_back (cl != nullptr ? cl[i] : std::nan (""));
                lauf->kandidat.push_back (cr != nullptr ? cr[i] : std::nan (""));
            }
            std::array<double, (size_t) param::kSlots> w {};
            k.auslenkungenDb (w.data());
            lauf->auslenkungen.push_back (w);
            int klingend = -1, q = -1;
            k.rechnendeSlots (Pfad::committed, klingend, q);
            lauf->kreuzQuelle.push_back (q);
        }
        n0 += m;
        rest -= m;
    }
}

/** Zwei Kerne mit identischer Vorgeschichte: `vorher` im Pfad `pfad` (dazu
    `committedDazu` im Committed-Pfad, wenn gesetzt), `vorgeschichte` Samples
    aus `vor` in Bloecken zu `vorBlock`. Dann uebernimmt NUR der Pruefkern
    `nachher`, und beide fahren `fenster` Samples aus `nach` in Bloecken zu
    `blockGroesse`. `uebernahmen` zaehlt die Blockrand-Uebernahmen des
    Fensters (B-11). */
struct W03Paar
{
    W03Lauf test, ref;
    std::uint64_t uebernahmenTest = 0, uebernahmenRef = 0;
    std::array<double, (size_t) param::kSlots> auslenkungVorher {};
};

template <typename QuelleVor, typename QuelleNach>
W03Paar fahrePaar (double fs, int maxBlock, const param::DspSatz& vorher, const param::DspSatz& nachher, Pfad pfad,
                   const QuelleVor& vor, int vorgeschichte, int vorBlock,
                   const QuelleNach& nach, int fenster, int blockGroesse,
                   const param::DspSatz* committedDazu = nullptr)
{
    W03Paar e;
    auto test = neuerKern (fs, maxBlock);
    auto ref  = neuerKern (fs, maxBlock);
    for (auto* k : { test.get(), ref.get() })
    {
        if (committedDazu != nullptr) k->uebernehmeZustand (*committedDazu);
        k->uebernehmeZustand (vorher, pfad);
        long long n = 0;
        fahreW03 (*k, vor, n, vorgeschichte, vorBlock, nullptr);
        k->pflege();
    }
    test->auslenkungenDb (e.auslenkungVorher.data());
    const std::uint64_t uT = test->uebernahmen(), uR = ref->uebernahmen();
    test->uebernehmeZustand (nachher, pfad);
    long long nT = vorgeschichte, nR = vorgeschichte;
    fahreW03 (*test, nach, nT, fenster, blockGroesse, &e.test);
    fahreW03 (*ref,  nach, nR, fenster, blockGroesse, &e.ref);
    e.uebernahmenTest = test->uebernahmen() - uT;
    e.uebernahmenRef  = ref->uebernahmen() - uR;
    return e;
}

/** Das Mass (F-9) ueber `perioden` Perioden zu je `jePeriode` Werten ab Index
    0. Ein Referenz-RMS 0 oder ein nicht endlicher Wert ergibt +inf: das Mass
    faellt dann, statt still zu bestehen. `erste` erhaelt den Wert der ersten
    Periode mit Vorzeichen, `schlimmste` die Periode des Maximums. */
double periodenMass (const std::vector<double>& test, const std::vector<double>& ref, size_t jePeriode,
                     int perioden, double* erste = nullptr, int* schlimmste = nullptr)
{
    const double unendlich = std::numeric_limits<double>::infinity();
    double mass = 0.0;
    for (int p = 0; p < perioden; ++p)
    {
        const size_t von = (size_t) p * jePeriode, bis = von + jePeriode;
        if (bis > test.size() || bis > ref.size()) return unendlich;
        double st = 0.0, sr = 0.0;
        for (size_t i = von; i < bis; ++i) { st += test[i] * test[i]; sr += ref[i] * ref[i]; }
        double db = 10.0 * std::log10 (st / sr);   // 20*log10 des RMS-Verhaeltnisses
        if (! std::isfinite (db)) db = unendlich;
        if (p == 0 && erste != nullptr) *erste = db;
        if (p == 0 || std::abs (db) > mass)
        {
            mass = std::max (mass, std::abs (db));
            if (schlimmste != nullptr) *schlimmste = p;
        }
    }
    return mass;
}

/** Erster Index ab `von`, an dem zwei Aufzeichnungen im Bitmuster
    abweichen; -1 heisst bytegleich. */
long long ersteBitAbweichung (const std::vector<double>& a, const std::vector<double>& b, size_t von)
{
    if (a.size() != b.size()) return (long long) std::min (a.size(), b.size());
    for (size_t i = von; i < a.size(); ++i)
        if (std::memcmp (&a[i], &b[i], sizeof (double)) != 0) return (long long) i;
    return -1;
}

/** R-311-10 (Manifest NAK-311 §27): die Rundungstoleranz von E-31. Unter 1,0
    zwei Schritte des float-Rasters 2^-24, also 2^-23; darueber zwei Schritte
    des float-Rasters am groessten Betrag des Laufs - dieselbe Herleitung. */
double rundungR31110 (double groessterBetrag)
{
    if (! (groessterBetrag >= 1.0)) return kRundungFloat;
    int e = 0;
    (void) std::frexp (groessterBetrag, &e);            // Betrag in [2^(e-1), 2^e)
    return 2.0 * std::ldexp (1.0, e - 1 - 23);           // float: 23 Mantissenbits
}

} // namespace

//==============================================================================
int main()
{
    std::cout << "== Nakama SONDE-015 B6 - aktiver DSP-Kern: Goldens, Bypass, Baenke ==" << std::endl;
    std::cout << "Gate: Entwurf §44.2/§44.5, Matrix §3.1-§3.6, §3.13 und aus §3.14 M-118 und M-120 (Kern)." << std::endl << std::endl;

    const double sampleraten[] = { 44100.0, 48000.0, 96000.0, 192000.0 };

    //==========================================================================
    std::cout << "== A - Passthrough und Hard-Bypass (M-01, M-02, M-05, M-50) ==" << std::endl;
    {
        // M-01: ausgeschaltet wird KEIN Sample geschrieben.
        for (double fs : sampleraten)
        {
            auto kern = neuerKern (fs);
            kern->uebernehmeZustand (machSatz (false));

            std::vector<float> l, r;
            const int bloecke = 250;
            bool bitgleich = true;
            std::uint64_t summe = 0;
            for (int b = 0; b < bloecke && bitgleich; ++b)
            {
                const int n = 1 + (b * 37) % 1024;
                l.assign ((size_t) n, 0.0f); r.assign ((size_t) n, 0.0f);
                for (int i = 0; i < n; ++i)
                {
                    l[(size_t) i] = (float) std::sin (0.01 * (double) (b * 1024 + i));
                    r[(size_t) i] = (float) std::cos (0.013 * (double) (b * 1024 + i));
                }
                std::vector<float> lK = l, rK = r;
                float* kanaele[2] = { l.data(), r.data() };
                kern->verarbeite (kanaele, 2, n);
                for (int i = 0; i < n; ++i)
                {
                    if (std::memcmp (&l[(size_t) i], &lK[(size_t) i], sizeof (float)) != 0
                     || std::memcmp (&r[(size_t) i], &rK[(size_t) i], sizeof (float)) != 0)
                        bitgleich = false;
                }
                summe += (std::uint64_t) n;
            }
            pruefe (bitgleich, std::string ("default_ist_bitidentisch bei ") + zahl (fs, 0) + " Hz",
                    std::to_string (summe) + " Samples");
            pruefe (kern->pool().belegteSlots() == 0,
                    std::string ("ausgeschaltet ist keine Bank belegt bei ") + zahl (fs, 0) + " (M-01, M-07, B-10)",
                    "belegt=" + std::to_string (kern->pool().belegteSlots()));
        }

        // B-10 / M-07: Vorbereiten -> eingeschaltet -> Audio -> AUSgeschaltet
        // -> Audio ueber den Fade hinaus -> Pflege: danach ist JEDE Bank frei.
        // Die alte Toleranz `<= 1` liess genau die Bank durch, die ein
        // ausgeschalteter Zustand reserviert und nie wieder hergegeben hat.
        {
            auto kern = neuerKern (48000.0, 512);
            auto ein = machSatz (true);
            belege (ein, 0, Filtertyp::bell, 1000.0, 1.0, 6.0);
            kern->uebernehmeZustand (ein);
            fahreStille (*kern, 2048, 512);
            kern->pflege();
            const int belegtEin = kern->pool().belegteSlots();

            kern->uebernehmeZustand (machSatz (false));
            fahreStille (*kern, kFadeSamples + 1024, 512);
            kern->pflege();

            int aktive = 0;
            for (int i = 0; i < DspBankPool::kBaenke; ++i)
                if (kern->pool().zustand (i) == BankZustand::audioAktiv) ++aktive;
            pruefe (belegtEin == 1 && kern->pool().belegteSlots() == 0 && aktive == 0,
                    "ausschalten_gibt_jede_bank_frei (M-07, B-10)",
                    "eingeschaltet belegt=" + std::to_string (belegtEin) + ", nach aus/Fade/Pflege belegt="
                    + std::to_string (kern->pool().belegteSlots()) + ", audio_active=" + std::to_string (aktive));
        }

        // M-02: eq an, bypass aus, alles neutral -> trotzdem BITIDENTISCH.
        // Der volle Weg aus §3.0 ist durchlaufen; jede neutrale Stufe ist ein
        // Unity-Kurzschluss und keine Multiplikation.
        {
            auto kern = neuerKern (48000.0);

            // Der Weg zu 0 dB fuehrt UEBER eine Rampe: erst +6 dB, dann
            // zurueck auf 0. Genau darauf zielt die Zusage - "einschliesslich
            // der Rampe, die auf dem Weg zu 1,0 durch 0.99999994f laufen
            // kann". Ohne diesen Umweg stuende die Rampe von Anfang an auf
            // 1,0, es liefe nie eine, und der Kurzschluss waere ungeprueft.
            auto mitTrim = machSatz (true);
            setzeGlobal (mitTrim, "v1.global.input_trim_db",  6.0);
            setzeGlobal (mitTrim, "v1.global.output_trim_db", 6.0);
            kern->uebernehmeZustand (mitTrim);
            fahreStille (*kern, kRampeSamples + 1024);
            kern->pflege();
            kern->uebernehmeZustand (machSatz (true));
            fahreStille (*kern, kRampeSamples + 2048);
            kern->pflege();

            std::vector<float> l (512), r (512), lK, rK;
            bool bitgleich = true;
            for (int b = 0; b < 40; ++b)
            {
                for (int i = 0; i < 512; ++i)
                {
                    l[(size_t) i] = (float) (0.5 * std::sin (0.02 * (double) (b * 512 + i)));
                    r[(size_t) i] = (float) (0.5 * std::cos (0.017 * (double) (b * 512 + i)));
                }
                lK = l; rK = r;
                float* kanaele[2] = { l.data(), r.data() };
                kern->verarbeite (kanaele, 2, 512);
                for (int i = 0; i < 512; ++i)
                    if (std::memcmp (&l[(size_t) i], &lK[(size_t) i], sizeof (float)) != 0
                     || std::memcmp (&r[(size_t) i], &rK[(size_t) i], sizeof (float)) != 0)
                        bitgleich = false;
            }
            pruefe (bitgleich, "unity_kurzschluss_statt_multiplikation (M-02)",
                    "eq an, bypass aus, 120 Werte auf Default");
        }

        // NAK-311 311/M-12 (T3-01-01): der engagiert-neutrale Kern SCHREIBT
        // NICHT. Hier ohne DAZ, am Kern allein: die Wachmarke (sNaN mit
        // Nutzlast) haelt das unabhaengig von der Rueckwandlung - jeder
        // Schreibzugriff machte sie ruhig oder, ueber den Riegel, zu 0,0. Der
        // Riegel laeuft fuer den inneren Weg weiter (SONDE-015 M-49): er zaehlt
        // jeden der sechs Werte einmal, und der Tap post_committed traegt dort
        // 0,0 (Manifest NAK-311 §9 F-3).
        {
            auto kern = neuerKern (48000.0);
            kern->uebernehmeZustand (machSatz (true));
            fahreStille (*kern, kFadeSamples + kRampeSamples + 2048);
            kern->pflege();

            const auto bits = [] (std::uint32_t b) { return ausBitmuster (b); };
            struct Stelle { int n; int kanal; std::uint32_t b; };
            const Stelle stellen[] = { { 16, 0, 0x7F800001u }, { 32, 1, 0x7F800001u }, { 48, 0, 0x7F800001u },
                                       { 64, 1, 0x7FC00000u }, { 80, 0, 0x7F800000u }, { 96, 1, 0xFF800000u } };
            const std::uint32_t muster[] = { 0x00000001u, 0x80000001u, 0x007FFFFFu, 0x807FFFFFu,
                                             0x00800000u, 0x80800000u, 0x00000000u, 0x80000000u };
            std::vector<float> l (512), r (512);
            for (int i = 0; i < 512; ++i)
            {
                l[(size_t) i] = (float) (0.4 * std::sin (0.021 * (double) i));
                r[(size_t) i] = (float) (0.3 * std::cos (0.017 * (double) i));
            }
            for (int j = 0; j < 8; ++j) { l[(size_t) (200 + j)] = bits (muster[j]); r[(size_t) (200 + j)] = bits (muster[j]); }
            for (const auto& s : stellen) (s.kanal == 0 ? l : r)[(size_t) s.n] = bits (s.b);
            const auto lK = l, rK = r;
            const std::uint64_t zaehlerVorher = kern->nichtEndlicheEingaenge();
            float* kanaele[2] = { l.data(), r.data() };
            kern->verarbeite (kanaele, 2, 512);

            int geschrieben = 0, tapNull = 0;
            for (int i = 0; i < 512; ++i)
                if (std::memcmp (&l[(size_t) i], &lK[(size_t) i], sizeof (float)) != 0
                 || std::memcmp (&r[(size_t) i], &rK[(size_t) i], sizeof (float)) != 0)
                    ++geschrieben;
            for (const auto& s : stellen)
            {
                const double* t = kern->tap (Tap::postCommitted, s.kanal);
                if (t != nullptr && t[s.n] == 0.0 && ! std::signbit (t[s.n])) ++tapNull;
            }
            const std::uint64_t gezaehlt = kern->nichtEndlicheEingaenge() - zaehlerVorher;
            pruefe (geschrieben == 0 && tapNull == 6 && gezaehlt == 6,
                    "311/M-12 neutraler_kern_schreibt_nicht (NAK-311 T3-01-01)",
                    "eq an, sonst Default, nach Fade und Rampen: geaenderte Samples " + std::to_string (geschrieben)
                    + " von 512 (Wachmarke, NaN, +-Inf, Bitmuster), Tap 0,0 an " + std::to_string (tapNull)
                    + " von 6 Stellen, Riegel +" + std::to_string (gezaehlt));
        }

        // NAK-311 §9 F-4 (Selbstaudit der Etappe 2): ein 0-dB-Bell ist nach der
        // RBJ-Formel KEIN Einheitsbiquad - b0 = 1, aber b1 = a1 = -2 cos(w0)/a0.
        // Ein Programm mit einem aktiven Band ist damit nie "neutral", und der
        // Kern schreibt weiter; den Wert selbst aendert das Band bei Zustand 0
        // nicht (b == a), die Wachmarke wird ueber den Riegel zu 0,0.
        {
            const float wachmarke = ausBitmuster (0x7F800001u);   // NAK-311: nie konstantgefaltet
            auto kern = neuerKern (48000.0);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
            kern->uebernehmeZustand (s);
            fahreStille (*kern, kFadeSamples + kRampeSamples + 2048);
            int ck = -1, cq = -1, kk = -1, kq = -1;
            kern->gefahreneSlots (ck, cq, kk, kq);
            const bool einheit = ck >= 0 && kern->pool().bank (ck).programm.baender[0].statischIstEinheit;

            std::vector<float> l (64, 0.25f), r (64, 0.25f);
            l[10] = wachmarke; r[20] = wachmarke;
            float* kanaele[2] = { l.data(), r.data() };
            kern->verarbeite (kanaele, 2, 64);
            const bool wachmarkeGeschrieben = std::memcmp (&l[10], &wachmarke, sizeof (float)) != 0
                                           && std::memcmp (&r[20], &wachmarke, sizeof (float)) != 0;
            int wertAnders = 0;
            for (int i = 0; i < 64; ++i)
            {
                if (i != 10 && l[(size_t) i] != 0.25f) ++wertAnders;
                if (i != 20 && r[(size_t) i] != 0.25f) ++wertAnders;
            }
            pruefe (ck >= 0 && ! einheit && wachmarkeGeschrieben && wertAnders == 0,
                    "311/F-4 null_db_bell_ist_kein_einheitsband_und_haelt_den_kern_schreibend (NAK-311 §9 F-4, Selbstaudit)",
                    std::string ("statischIstEinheit ") + (einheit ? "ja" : "nein") + ", Wachmarken ueberschrieben: "
                    + (wachmarkeGeschrieben ? "ja" : "nein") + ", uebrige Werte veraendert: " + std::to_string (wertAnders));
        }

        // M-05: Hard-Bypass VOR M/S und Filterbank - auch mit Baendern und
        // width != 1 bleibt der Ausgang bitgenau der Eingang.
        {
            auto kern = neuerKern (48000.0);
            auto s = machSatz (true);
            setzeGlobalBool (s, "v1.global.bypass", true);
            setzeGlobal (s, "v1.global.width", 1.7);
            setzeGlobal (s, "v1.global.input_trim_db", 6.0);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 12.0);
            kern->uebernehmeZustand (s);
            fahreStille (*kern, 2048);

            std::vector<float> l (512), r (512), lK, rK;
            bool bitgleich = true;
            for (int b = 0; b < 20; ++b)
            {
                for (int i = 0; i < 512; ++i)
                {
                    l[(size_t) i] = (float) (0.4 * std::sin (0.03 * (double) (b * 512 + i)));
                    r[(size_t) i] = (float) (0.3 * std::cos (0.011 * (double) (b * 512 + i)));
                }
                lK = l; rK = r;
                float* kanaele[2] = { l.data(), r.data() };
                kern->verarbeite (kanaele, 2, 512);
                for (int i = 0; i < 512; ++i)
                    if (std::memcmp (&l[(size_t) i], &lK[(size_t) i], sizeof (float)) != 0
                     || std::memcmp (&r[(size_t) i], &rK[(size_t) i], sizeof (float)) != 0)
                        bitgleich = false;
            }
            pruefe (bitgleich, "hard_bypass_schreibt_nicht (M-05)",
                    "width 1,7 und ein +12-dB-Bell liegen an und schlagen NICHT durch");
        }

        // M-50: der Passthrough sanitisiert NICHTS.
        {
            auto kern = neuerKern (48000.0);
            kern->uebernehmeZustand (machSatz (false));
            fahreStille (*kern, 1024);

            std::vector<float> l (8, 0.0f), r (8, 0.0f);
            l[2] = std::numeric_limits<float>::quiet_NaN();
            l[5] = std::numeric_limits<float>::infinity();
            r[3] = -std::numeric_limits<float>::infinity();
            float* kanaele[2] = { l.data(), r.data() };
            kern->verarbeite (kanaele, 2, 8);

            const bool nanDa = std::isnan (l[2]);
            const bool infDa = std::isinf (l[5]) && std::isinf (r[3]);
            pruefe (nanDa && infDa, "passthrough_sanitisiert_nichts (M-50)",
                    "NaN und beide Inf kommen unveraendert heraus");
            pruefe (kern->nichtEndlicheEingaenge() == 0,
                    "passthrough zaehlt keinen nicht-endlichen Eingang (M-50)",
                    "Zaehler=" + std::to_string (kern->nichtEndlicheEingaenge()));
        }
    }

    //==========================================================================
    std::cout << std::endl << "== B - Crossfades: engagieren, ausschalten, Bypasswechsel (M-03, M-04, M-06) ==" << std::endl;
    {
        // M-03/M-04: der Wechsel ist klickfrei, und NACH dem Fade ist der
        // Passthrough wieder BITIDENTISCH - nicht "numerisch nahe".
        auto kern = neuerKern (48000.0);
        kern->uebernehmeZustand (machSatz (false));
        fahreStille (*kern, 1024);

        auto an = machSatz (true);
        belege (an, 0, Filtertyp::bell, 1000.0, 1.0, 12.0);

        // Einschalten bei laufendem Audio: der groesste Sprung zwischen zwei
        // Samples darf die Fadeschrittweite nicht ueberschreiten.
        const int n = 2048;
        std::vector<float> l ((size_t) n), r ((size_t) n);
        auto fuelle = [&] (long long versatz)
        {
            for (int i = 0; i < n; ++i)
            {
                const double x = 0.5 * std::sin (2.0 * 3.14159265358979 * 220.0
                                                 * (double) (versatz + i) / 48000.0);
                l[(size_t) i] = (float) x; r[(size_t) i] = (float) x;
            }
        };

        fuelle (0);
        float* kanaele[2] = { l.data(), r.data() };
        kern->verarbeite (kanaele, 2, n);
        double vorher = l[(size_t) (n - 1)];

        kern->uebernehmeZustand (an);
        fuelle (n);
        std::vector<float> eingang = l;
        kern->verarbeite (kanaele, 2, n);

        double maxSprung = std::abs ((double) l[0] - vorher);
        for (int i = 1; i < n; ++i)
            maxSprung = std::max (maxSprung, std::abs ((double) l[(size_t) i] - (double) l[(size_t) (i - 1)]));

        // Der Sinus selbst springt je Sample um bis zu 2*pi*f/fs * A.
        const double eigensprung = 0.5 * 2.0 * 3.14159265358979 * 220.0 / 48000.0;
        pruefe (maxSprung < 12.0 * eigensprung,
                "engagieren_ist_klickfrei_und_endet_deterministisch (M-03)",
                "max Sprung " + zahl (maxSprung, 5) + " gegen Eigensprung " + zahl (eigensprung, 5));

        // Und der Fade ist in SAMPLES gezaehlt, nicht in Bloecken: derselbe
        // Wechsel bei Blockgroesse 1 und 2048 endet nach derselben Zahl.
        {
            auto k1 = neuerKern (48000.0, 2048);
            k1->uebernehmeZustand (machSatz (false));
            fahreStille (*k1, 1024, 1);
            k1->uebernehmeZustand (an);

            auto k2 = neuerKern (48000.0, 2048);
            k2->uebernehmeZustand (machSatz (false));
            fahreStille (*k2, 1024, 2048);
            k2->uebernehmeZustand (an);

            // Beide bekommen DC 1,0; nach kFadeSamples muss der Wert gleich sein.
            auto fahreDc = [] (DspKern& k, int blockGroesse, int samples)
            {
                std::vector<float> a ((size_t) blockGroesse), b ((size_t) blockGroesse);
                float* kan[2] = { a.data(), b.data() };
                std::vector<double> aus;
                int rest = samples;
                while (rest > 0)
                {
                    const int m = rest < blockGroesse ? rest : blockGroesse;
                    std::fill (a.begin(), a.end(), 1.0f);
                    std::fill (b.begin(), b.end(), 1.0f);
                    k.verarbeite (kan, 2, m);
                    for (int i = 0; i < m; ++i) aus.push_back ((double) a[(size_t) i]);
                    rest -= m;
                }
                return aus;
            };
            const auto a1 = fahreDc (*k1, 1,    kFadeSamples + 64);
            const auto a2 = fahreDc (*k2, 2048, kFadeSamples + 64);

            int abw = 0;
            for (int i = 0; i < kFadeSamples + 64 && i < (int) a1.size() && i < (int) a2.size(); ++i)
                if (std::abs (a1[(size_t) i] - a2[(size_t) i]) > 1e-6) ++abw;
            pruefe (abw == 0, "fade_ist_in_samples_gezaehlt_nicht_in_bloecken (M-03)",
                    "Blockgroesse 1 gegen 2048: " + std::to_string (abw) + " Abweichungen");
        }

        // Ausschalten: nach dem Fade wieder BITIDENTISCH.
        {
            auto k = neuerKern (48000.0);
            k->uebernehmeZustand (an);
            fahreStille (*k, 2048);
            k->pflege();
            k->uebernehmeZustand (machSatz (false));

            // Die Blockgroesse 300 ist KEIN Teiler von kFadeSamples: der Fade
            // endet damit MITTEN im Block, und die Samples danach fallen in
            // denselben Aufruf. Bei einer Blockgroesse, die genau aufgeht,
            // laeuft der Weg nach Fadeende in diesem Block gar nicht, und
            // eine Mutation genau dort bliebe unsichtbar. Ausserdem laeuft
            // SIGNAL statt Stille - an einer Null ist jeder Faktor gleich.
            const int bg = 300;
            std::vector<float> a ((size_t) bg), b ((size_t) bg), aK, bK;
            bool bitgleich = true;
            long long n0 = 0;
            for (int blk = 0; blk < 12; ++blk)
            {
                for (int i = 0; i < bg; ++i)
                {
                    a[(size_t) i] = (float) (0.4 * std::sin (0.02 * (double) (n0 + i)));
                    b[(size_t) i] = (float) (0.4 * std::cos (0.02 * (double) (n0 + i)));
                }
                aK = a; bK = b;
                float* kan[2] = { a.data(), b.data() };
                k->verarbeite (kan, 2, bg);
                // Der erste Block traegt den Fade selbst - erst ab dem zweiten
                // ist die Zusage "nach dem Fade" ueberhaupt gemeint. Die
                // Samples NACH dem Fadeende im ersten Block gehoeren dazu.
                const int von = (blk == 0) ? kFadeSamples : 0;
                for (int i = von; i < bg; ++i)
                    if (std::memcmp (&a[(size_t) i], &aK[(size_t) i], sizeof (float)) != 0
                     || std::memcmp (&b[(size_t) i], &bK[(size_t) i], sizeof (float)) != 0)
                        bitgleich = false;
                n0 += bg;
            }
            pruefe (bitgleich, "nach_dem_fade_wieder_bitidentisch (M-04)",
                    "nicht nur numerisch nahe - bitgleich, auch im Fadeblock selbst");
        }

        // M-06: der Bypasswechsel faehrt denselben Weg.
        {
            auto k = neuerKern (48000.0);
            auto ein = machSatz (true);
            // Ein Low-Shelf mit hoher Ecke hebt DC um seinen vollen Gain -
            // erst dann ist die Differenz zwischen Bypass und Kette am
            // DC-Signal ueberhaupt messbar. Ein High-Shelf bei 4 kHz
            // laesst DC unveraendert, und der Test maesse nichts.
            belege (ein, 0, Filtertyp::lowShelf, 8000.0, 0.707, 9.0);
            k->uebernehmeZustand (ein);
            fahreStille (*k, 2048);
            k->pflege();

            auto mitBypass = ein;
            setzeGlobalBool (mitBypass, "v1.global.bypass", true);

            // Der Uebergang selbst: mit DC am Eingang ist jeder Sprung im
            // Ausgang ein Sprung der Kette. Ohne Crossfade springt der
            // Ausgang in EINEM Sample von der gefilterten Antwort auf den
            // Eingang - Bitgleichheit NACH dem Fade sieht das nicht, denn
            // ohne Fade ist der Ausgang sofort bitgleich.
            std::vector<float> a (256), b (256), aK, bK;
            float* kanD[2] = { a.data(), b.data() };
            std::fill (a.begin(), a.end(), 0.4f); std::fill (b.begin(), b.end(), 0.4f);
            k->verarbeite (kanD, 2, 256);
            const double vorDemWechsel = a[255];

            k->uebernehmeZustand (mitBypass);
            std::fill (a.begin(), a.end(), 0.4f); std::fill (b.begin(), b.end(), 0.4f);
            k->verarbeite (kanD, 2, 256);
            double maxBypassSprung = std::abs ((double) a[0] - vorDemWechsel);
            for (int i = 1; i < 256; ++i)
                maxBypassSprung = std::max (maxBypassSprung,
                                            std::abs ((double) a[(size_t) i] - (double) a[(size_t) (i - 1)]));
            const double bypassStufe = std::abs (vorDemWechsel - 0.4) / (double) kFadeSamples;
            pruefe (maxBypassSprung <= 4.0 * bypassStufe + 1e-9,
                    "bypasswechsel_ist_klickfrei (M-06)",
                    "max Sprung " + zahl (maxBypassSprung, 8) + " gegen Fadeschritt "
                    + zahl (bypassStufe, 8));

            fahreStille (*k, kFadeSamples + 512);
            k->pflege();

            bool bitgleich = true;
            for (int blk = 0; blk < 8; ++blk)
            {
                for (int i = 0; i < 256; ++i)
                {
                    a[(size_t) i] = (float) (0.3 * std::sin (0.05 * (double) (blk * 256 + i)));
                    b[(size_t) i] = a[(size_t) i];
                }
                aK = a; bK = b;
                float* kan[2] = { a.data(), b.data() };
                k->verarbeite (kan, 2, 256);
                for (int i = 0; i < 256; ++i)
                    if (std::memcmp (&a[(size_t) i], &aK[(size_t) i], sizeof (float)) != 0)
                        bitgleich = false;
            }
            pruefe (bitgleich, "bypasswechsel_endet_bitgleich (M-06)");
        }

        // B-3 / M-05: EQ-aus und Hard-Bypass ueberbruecken JEDE Hoermatrix.
        // Waehrend des Fades blenden auch Delta und Candidate klickfrei auf
        // den unveraenderten Eingang; danach wird kein Sample geschrieben. Der
        // Schreibnachweis ist eine WACHMARKE: ein signalisierender NaN mit
        // Nutzlast ueberlebt jede float->double->float-Wandlung nicht (sie
        // macht ihn ruhig) - bitgleich bleibt er nur, wenn niemand schreibt.
        {
            const float wachmarke = ausBitmuster (0x7F800001u);   // NAK-311: nie konstantgefaltet
            const char* namen[] = { "delta + eq_enabled aus", "delta + hard-bypass",
                                    "candidate + eq_enabled aus", "candidate + hard-bypass" };
            for (int fall = 0; fall < 4; ++fall)
            {
                const bool mitBypass = (fall % 2) == 1;
                const Hoermatrix wahl = fall < 2 ? Hoermatrix::delta : Hoermatrix::candidate;

                auto k = neuerKern (48000.0, 512);
                auto ein = machSatz (true);
                setzeGlobal (ein, "v1.global.output_trim_db", 6.0);
                k->uebernehmeZustand (ein);
                if (wahl == Hoermatrix::candidate)
                {
                    auto kand = machSatz (true);
                    belege (kand, 0, Filtertyp::lowShelf, 8000.0, 0.707, 9.0);
                    k->uebernehmeZustand (kand, Pfad::candidate);
                }
                fahreStille (*k, kRampeSamples + 2048, 512);
                k->setzeHoermatrix (wahl);
                const auto vorlauf = fahreDc (*k, 0.3, 2048, 512);
                const double vorher = vorlauf.back();

                auto aus = ein;
                if (mitBypass) setzeGlobalBool (aus, "v1.global.bypass", true);
                else           aus.werte[(size_t) param::kIndexEqEnabled].b = false;
                k->uebernehmeZustand (aus);

                const auto fade = fahreDc (*k, 0.3, kFadeSamples, 64);
                const double sprung = groessterSprung (vorher, fade);
                const double stufe  = std::abs (vorher - 0.3) / (double) kFadeSamples;
                pruefe (std::abs (vorher - 0.3) > 0.1 && sprung <= 4.0 * stufe + 1e-9
                        && std::abs (fade.back() - 0.3) <= 2.0 * stufe + 1e-6,
                        std::string ("hoermatrix_blendet_bei_aus_klickfrei_auf_den_eingang (M-05, B-3): ") + namen[fall],
                        "vorher " + zahl (vorher, 5) + ", max Sprung " + zahl (sprung, 7) + " gegen Fadeschritt "
                        + zahl (stufe, 7) + ", Fadeende " + zahl (fade.back(), 6));

                std::vector<float> l (512), r (512), lK, rK;
                bool unberuehrt = true;
                for (int blk = 0; blk < 4; ++blk)
                {
                    for (int i = 0; i < 512; ++i)
                    {
                        l[(size_t) i] = (float) (0.3 * std::sin (0.02 * (double) (blk * 512 + i)));
                        r[(size_t) i] = l[(size_t) i];
                    }
                    l[7] = wachmarke; r[9] = wachmarke;
                    lK = l; rK = r;
                    float* kan[2] = { l.data(), r.data() };
                    k->verarbeite (kan, 2, 512);
                    if (std::memcmp (l.data(), lK.data(), 512 * sizeof (float)) != 0
                     || std::memcmp (r.data(), rK.data(), 512 * sizeof (float)) != 0)
                        unberuehrt = false;
                }
                pruefe (unberuehrt,
                        std::string ("nach_dem_fade_schreibt_keine_hoermatrix_ein_sample (M-05, B-3): ") + namen[fall],
                        "Wachmarke und Signal bitgleich ueber 4 Bloecke");
            }
        }

        // W-1 / B-3 / M-05: der Schreibverzicht beginnt EXAKT am Sample nach
        // dem Fade-Ende - auch in dem Teilstueck, in dem der Fade endet. Ein
        // 512-Sample-Block direkt nach dem Ausschalten: 0..255 tragen den Fade,
        // ab 256 wird kein Sample geschrieben. Die Wachmarken (sNaN mit
        // Nutzlast) liegen HINTER dem Fade-Ende im SELBEN Teilstueck; ein
        // float->double->float-Ruecklauf machte sie ruhig.
        {
            const float wachmarke = ausBitmuster (0x7F800001u);   // NAK-311: nie konstantgefaltet
            const char* namen[] = { "processed + eq_enabled aus", "processed + hard-bypass",
                                    "delta + eq_enabled aus", "delta + hard-bypass" };
            for (int fall = 0; fall < 4; ++fall)
            {
                const bool mitBypass = (fall % 2) == 1;
                auto k = neuerKern (48000.0, 512);
                auto ein = machSatz (true);
                setzeGlobal (ein, "v1.global.output_trim_db", 6.0);
                k->uebernehmeZustand (ein);
                fahreStille (*k, kRampeSamples + 2048, 512);
                k->setzeHoermatrix (fall < 2 ? Hoermatrix::processed : Hoermatrix::delta);
                fahreDc (*k, 0.3, 2048, 512);
                k->pflege();

                auto aus = ein;
                if (mitBypass) setzeGlobalBool (aus, "v1.global.bypass", true);
                else           aus.werte[(size_t) param::kIndexEqEnabled].b = false;
                k->uebernehmeZustand (aus);

                std::vector<float> wl (512, 0.3f), wr (512, 0.3f);
                for (size_t i : { (size_t) 256, (size_t) 300, (size_t) 511 }) { wl[i] = wachmarke; wr[i] = wachmarke; }
                const auto wlK = wl, wrK = wr;
                float* wkan[2] = { wl.data(), wr.data() };
                k->verarbeite (wkan, 2, 512);

                int hintenGeschrieben = 0;
                for (size_t i = 256; i < 512; ++i)
                    if (std::memcmp (&wl[i], &wlK[i], sizeof (float)) != 0 || std::memcmp (&wr[i], &wrK[i], sizeof (float)) != 0)
                        ++hintenGeschrieben;
                const bool fadeGeschrieben = std::memcmp (&wl[0], &wlK[0], sizeof (float)) != 0;
                pruefe (fadeGeschrieben && hintenGeschrieben == 0,
                        std::string ("schreibverzicht_beginnt_exakt_am_fade_ende (M-05, B-3, W-1): ") + namen[fall],
                        std::string ("der Fade schreibt Sample 0: ") + (fadeGeschrieben ? "ja" : "nein")
                        + ", geaenderte Samples hinter dem Fade-Ende im selben Teilstueck: "
                        + std::to_string (hintenGeschrieben) + " von 256, Wachmarken an 256, 300 und 511");
            }
        }

        // NAK-311 311/M-13 (T3-01-01, Muster E-32): engagiert der Kern aus der
        // Ruhe ein NEUTRALES Programm, endet der Schreibzugriff EXAKT am ersten
        // Sample nach dem Crossfade - auch mitten im Teilstueck, bei jeder
        // Blockgroesse und bei einem Block ueber maxBlock (M-48). Die Samples 0
        // bis 255 tragen den Fade (dort wird die Wachmarke ruhig), ab 256
        // schreibt der Kern keinen Sample mehr (Wachmarken an 256, 257, 511 und
        // am letzten Sample bytegleich).
        {
            const float wachmarke = ausBitmuster (0x7F800001u);   // NAK-311: nie konstantgefaltet
            for (const int bg : { 1, 255, 256, 257, 180, 4096 })
            {
                auto k = neuerKern (48000.0, 512);
                k->uebernehmeZustand (machSatz (false));
                fahreStille (*k, 1024, 512);
                k->pflege();
                k->uebernehmeZustand (machSatz (true));

                const int gesamt = ((512 + bg - 1) / bg) * bg;
                std::vector<float> wl ((size_t) gesamt, 0.3f), wr ((size_t) gesamt, 0.3f);
                for (const int i : { 0, 255, 256, 257, 511, gesamt - 1 }) { wl[(size_t) i] = wachmarke; wr[(size_t) i] = wachmarke; }
                const auto wlK = wl, wrK = wr;
                for (int ab = 0; ab < gesamt; ab += bg)
                {
                    float* wkan[2] = { wl.data() + ab, wr.data() + ab };
                    k->verarbeite (wkan, 2, std::min (bg, gesamt - ab));
                }
                const auto anders = [&] (int i)
                {
                    return std::memcmp (&wl[(size_t) i], &wlK[(size_t) i], sizeof (float)) != 0
                        || std::memcmp (&wr[(size_t) i], &wrK[(size_t) i], sizeof (float)) != 0;
                };
                int hinten = 0;
                for (int i = kFadeSamples; i < gesamt; ++i) if (anders (i)) ++hinten;
                const bool fadeSchreibt = anders (0) && anders (kFadeSamples - 1);
                pruefe (fadeSchreibt && hinten == 0,
                        "311/M-13 schreibende_exakt_am_uebergangsende_in_neutral (NAK-311 T3-01-01, E-32): Blockgroesse "
                        + std::to_string (bg),
                        std::string ("der Fade schreibt Sample 0 und 255: ") + (fadeSchreibt ? "ja" : "nein")
                        + ", geaenderte Samples ab 256: " + std::to_string (hinten) + " von "
                        + std::to_string (gesamt - kFadeSamples) + ", Wachmarken an 256, 257 und 511"
                        + (gesamt > 512 ? " und am letzten Sample " + std::to_string (gesamt - 1) : std::string()));
            }

            // M-14 (Teilfall): eine Rampe auf das neutrale Programm ist
            // Verarbeitung. Output-Trim +6 dB -> 0 dB: die 256 Rampensamples
            // schreibt der Kern entlang der hier ausgeschriebenen Ideallinie,
            // ab dem ersten Sample nach dem Rampenende keinen mehr.
            {
                auto k = neuerKern (48000.0, 512);
                auto laut = machSatz (true);
                setzeGlobal (laut, "v1.global.output_trim_db", 6.0);
                k->uebernehmeZustand (laut);
                fahreStille (*k, kRampeSamples + 2048, 512);
                k->pflege();
                k->uebernehmeZustand (machSatz (true));

                std::vector<float> wl (512, 0.25f), wr (512, 0.25f);
                for (const int i : { 256, 257, 511 }) { wl[(size_t) i] = wachmarke; wr[(size_t) i] = wachmarke; }
                const auto wlK = wl, wrK = wr;
                float* wkan[2] = { wl.data(), wr.data() };
                k->verarbeite (wkan, 2, 512);

                const double von = std::pow (10.0, 6.0 / 20.0);
                double maxAbw = 0.0;
                for (int i = 0; i < kRampeSamples; ++i)
                {
                    // Tick 1 ist das erste Sample der Rampe, Tick 256 steht auf 1,0.
                    const double soll = 0.25 * (von + (1.0 - von) * (double) (i + 1) / (double) kRampeSamples);
                    maxAbw = std::max ({ maxAbw, std::abs ((double) wl[(size_t) i] - soll), std::abs ((double) wr[(size_t) i] - soll) });
                }
                int hinten = 0;
                for (int i = kRampeSamples; i < 512; ++i)
                    if (std::memcmp (&wl[(size_t) i], &wlK[(size_t) i], sizeof (float)) != 0
                     || std::memcmp (&wr[(size_t) i], &wrK[(size_t) i], sizeof (float)) != 0)
                        ++hinten;
                pruefe (maxAbw < 1e-6 && hinten == 0,
                        "311/M-14 rampe_auf_neutral_schreibt_bis_zum_rampenende (Teilfall von 311/M-13)",
                        "Output-Trim +6 -> 0 dB: groesste Abweichung der 256 Rampensamples von der Ideallinie "
                        + zahl (maxAbw, 9) + ", geaenderte Samples ab 256: " + std::to_string (hinten)
                        + " von 256, Wachmarken an 256, 257 und 511");
            }

            // Selbstaudit NAK-311 Etappe 2 (aktivieren <-> abklingen, Zahlenrand
            // Mix): (a) aus der schreibfreien Ruhe heraus setzt das Schreiben am
            // ERSTEN Sample des Uebergangs wieder ein, neutral -> wirksam per
            // Crossfade wie per Rampe; (b) Mix 1e-9 ist nicht Mix 0 - der Kern
            // schreibt.
            {
                const auto erstesSampleGeschrieben = [&] (const param::DspSatz& danach)
                {
                    auto k = neuerKern (48000.0, 512);
                    k->uebernehmeZustand (machSatz (true));
                    fahreStille (*k, kFadeSamples + kRampeSamples + 2048, 512);
                    k->pflege();
                    k->uebernehmeZustand (danach);
                    std::vector<float> wl (512, 0.25f), wr (512, 0.25f);
                    wl[0] = wachmarke; wr[0] = wachmarke;
                    float* wkan[2] = { wl.data(), wr.data() };
                    k->verarbeite (wkan, 2, 512);
                    return std::memcmp (&wl[0], &wachmarke, sizeof (float)) != 0
                        && std::memcmp (&wr[0], &wachmarke, sizeof (float)) != 0;
                };
                auto mitBand = machSatz (true);
                belege (mitBand, 0, Filtertyp::bell, 1000.0, 1.0, 12.0);   // Topologie: Crossfade
                auto mitTrim = machSatz (true);
                setzeGlobal (mitTrim, "v1.global.output_trim_db", 6.0);     // Wert: Rampe
                const bool perCrossfade = erstesSampleGeschrieben (mitBand);
                const bool perRampe     = erstesSampleGeschrieben (mitTrim);
                pruefe (perCrossfade && perRampe,
                        "311/M-13 Selbstaudit schreiben_setzt_am_ersten_uebergangssample_wieder_ein",
                        std::string ("neutral -> +12-dB-Bell (Crossfade): ") + (perCrossfade ? "ja" : "nein")
                        + ", neutral -> Output-Trim +6 dB (Rampe): " + (perRampe ? "ja" : "nein")
                        + " - Wachmarke an Sample 0 des Uebergangs geschrieben");

                auto k = neuerKern (48000.0, 512);
                auto fastNull = machSatz (true);
                setzeGlobal (fastNull, "v2.global.mix", 1e-9);
                k->uebernehmeZustand (fastNull);
                fahreStille (*k, kFadeSamples + kRampeSamples + 2048, 512);
                std::vector<float> wl (512, 0.25f), wr (512, 0.25f);
                wl[300] = wachmarke; wr[300] = wachmarke;
                float* wkan[2] = { wl.data(), wr.data() };
                k->verarbeite (wkan, 2, 512);
                const bool geschrieben = std::memcmp (&wl[300], &wachmarke, sizeof (float)) != 0
                                      && std::memcmp (&wr[300], &wachmarke, sizeof (float)) != 0;
                pruefe (geschrieben, "311/M-13 Selbstaudit mix_ein_milliardstel_ist_nicht_mix_null",
                        std::string ("Mix 1e-9 eingeschwungen, Wachmarke an 300 geschrieben: ") + (geschrieben ? "ja" : "nein"));
            }
        }

        // B-5: ein zweiter Wechsel 64 Samples nach dem ersten, bei einem
        // 256-Sample-Fade und drei verschiedenen Kurven. Beide Wechsel sind
        // TOPOLOGISCH (Typ, dann Typ und Kanalmodus) und laufen deshalb als
        // Crossfade zwischen zwei vollstaendigen Programmen - ein reiner
        // Rampenwechsel (E-19) haette keine verblassende Bank. Der laufende Fade
        // wird zu Ende gefuehrt, der neue Wechsel danach genommen (E-17): kein
        // Sprung am Umschaltsample groesser als die Fadeschrittweite.
        {
            auto k = neuerKern (48000.0, 64);
            auto sa = machSatz (true); belege (sa, 0, Filtertyp::lowShelf,  8000.0, 0.707, 9.0);
            auto sb = machSatz (true); belege (sb, 0, Filtertyp::highShelf, 8000.0, 0.707, 9.0);
            auto sc = machSatz (true); belege (sc, 0, Filtertyp::lowShelf,  8000.0, 0.707, 3.0, Kanalmodus::mid);
            k->uebernehmeZustand (sa);
            const auto vor = fahreDc (*k, 0.3, 2048, 64);
            k->pflege();

            k->uebernehmeZustand (sb);
            auto lauf = fahreDc (*k, 0.3, 64, 64);
            k->uebernehmeZustand (sc);
            const auto rest = fahreDc (*k, 0.3, 1024, 64);
            lauf.insert (lauf.end(), rest.begin(), rest.end());

            const double A = vor.back();                          // Low-Shelf +9 dB bei DC
            const double B = 0.3;                                 // ein High-Shelf traegt DC mit 0 dB
            const double C = 0.3 * std::pow (10.0, 3.0 / 20.0);   // Low-Shelf +3 dB auf Mid, L = R
            const double stufe  = std::max (std::abs (B - A), std::abs (C - B)) / (double) kFadeSamples;
            // W-4 (E-31): gemessen AM UMSCHALTSAMPLE, dem ersten Sample jedes
            // neuen Fades - lauf[0] (A -> B) und lauf[kFadeSamples], wo der
            // wartende Wechsel B -> C beginnt (E-17). Erlaubt ist die
            // Fadeschrittweite plus die Rundung des float-Ausgangs.
            const size_t zweiter   = (size_t) kFadeSamples;
            const double amErsten  = std::abs (lauf[0] - A);
            const double amZweiten = std::abs (lauf[zweiter] - lauf[zweiter - 1]);
            pruefe (amErsten <= stufe + kRundungFloat && amZweiten <= stufe + kRundungFloat,
                    "zweiter_wechsel_im_laufenden_fade_springt_nicht (M-03, M-06, B-5, W-4)",
                    "am Umschaltsample " + zahl (amErsten, 9) + " und " + zahl (amZweiten, 9)
                    + " gegen Fadeschritt " + zahl (stufe, 9));
            // Die Wache ueber den ganzen Lauf traegt die aus dem Einschwingen
            // der kalt startenden Zielbank hergeleitete Schranke (E-31).
            const double x      = (double) 0.3f;
            const double wache  = std::max (kaltSchranke (Filtertyp::highShelf, 48000.0, 8000.0, 0.707, 9.0, A, x),
                                            kaltSchranke (Filtertyp::lowShelf,  48000.0, 8000.0, 0.707, 3.0, B, x));
            const double sprung = groessterSprung (A, lauf);
            pruefe (sprung <= wache + kRundungFloat,
                    "wache_der_kalt_startenden_bank_ueber_den_ganzen_lauf (B-5, W-4)",
                    "max Sprung " + zahl (sprung, 9) + " gegen hergeleitete Schranke " + zahl (wache, 9));
            pruefe (std::abs (lauf.back() - C) < 1e-4,
                    "der_zweite_wechsel_wird_nach_dem_fade_uebernommen (B-5, E-17)",
                    "Ende " + zahl (lauf.back(), 6) + " gegen " + zahl (C, 6));
        }

        // M-07: ausgeschaltet rechnet NICHTS und startet kalt.
        {
            auto k = neuerKern (48000.0);
            auto ein = machSatz (true);
            belege (ein, 0, Filtertyp::bell, 100.0, 12.0, 12.0);   // hohe Guete, langes Ausklingen
            k->uebernehmeZustand (ein);

            // Einen kraeftigen Impuls hineingeben, dann sofort ausschalten.
            std::vector<float> a (512, 0.0f), b (512, 0.0f);
            float* kan[2] = { a.data(), b.data() };
            fahreStille (*k, 1024);
            a[0] = 1.0f; b[0] = 1.0f;
            k->verarbeite (kan, 2, 512);
            k->pflege();
            k->uebernehmeZustand (machSatz (false));
            fahreStille (*k, kFadeSamples + 1024);
            k->pflege();

            // Wieder einschalten: die Zustaende muessen bei 0 starten. Bei
            // Stille am Eingang darf danach NICHTS herauskommen.
            k->uebernehmeZustand (ein);
            std::vector<float> c (2048, 0.0f), d (2048, 0.0f);
            float* kan2[2] = { c.data(), d.data() };
            k->verarbeite (kan2, 2, 2048);
            double maxAus = 0.0;
            for (int i = 0; i < 2048; ++i) maxAus = std::max (maxAus, std::abs ((double) c[(size_t) i]));
            pruefe (maxAus == 0.0, "ausgeschaltet_rechnet_nichts_und_startet_kalt (M-07)",
                    "max nach Wiedereinschalten " + zahl (maxAus, 12));
        }
    }

    //==========================================================================
    std::cout << std::endl << "== C - Filtergoldens gegen die analytische RBJ-Antwort (M-10 bis M-13) ==" << std::endl;
    {
        struct Pruefling { Filtertyp typ; double f; double q; double gain; };
        const Pruefling pruef[] = {
            { Filtertyp::bell,      1000.0, 1.0,   6.0 },
            { Filtertyp::bell,      1000.0, 12.0, 12.0 },   // Extrempunkt: Scheitel bei Q >= 12
            { Filtertyp::lowShelf,   200.0, 0.707, 6.0 },
            { Filtertyp::highShelf, 5000.0, 0.707, -6.0 },
            { Filtertyp::notch,     1000.0, 4.0,   0.0 },
            { Filtertyp::lowCut,     100.0, 0.707, 0.0 },
            { Filtertyp::highCut,   8000.0, 0.707, 0.0 },
        };

        const int irLaenge = 32768;

        for (double fs : sampleraten)
        {
            for (const auto& p : pruef)
            {
                const auto g = gitterFuer (fs, p.typ, p.f);
                pruefe (gitterEnthaelt (g, fs, p.typ, p.f),
                        std::string ("gitter_traegt_die_prueflingspunkte (R15 Feinheit 3, B-24) ") + typName (p.typ)
                        + " " + zahl (p.f, 0) + " Hz @" + zahl (fs, 0),
                        "Eck-/Mittenfrequenz" + std::string ((p.typ == Filtertyp::lowShelf || p.typ == Filtertyp::highShelf)
                                                             ? " und f/4, 4f" : ""));
                auto kern = neuerKern (fs, 512);
                auto s = machSatz (true);
                belege (s, 0, p.typ, p.f, p.q, p.gain);
                kern->uebernehmeZustand (s);

                const auto ir = impulsantwort (*kern, irLaenge, 0, 512, 2048);
                const auto ref = refEntwurf (p.typ, fs, p.f, p.q, p.gain);

                double maxAbwTypisch = 0.0, maxAbwRand = 0.0;
                double schlimmsteF = 0.0;
                for (size_t i = 0; i < g.size(); ++i)
                {
                    const double f = g[i];
                    const double gemessen = irDb (ir, fs, f);
                    const double soll     = refDb (ref, fs, f);
                    const double abw      = abweichungDb (gemessen, soll);
                    // Extrempunkte im Sinne der 0,1 dB: die Gitterstellen
                    // unmittelbar an beiden Enden und der Scheitel eines
                    // Bells mit Q >= 12 (§5.15 Feinheit 3).
                    const bool rand = (i == 0) || (i + 1 >= g.size())
                                      || (p.q >= 12.0 && std::abs (f - p.f) < p.f * 0.05);
                    if (rand) { maxAbwRand = std::max (maxAbwRand, abw); }
                    else if (abw > maxAbwTypisch) { maxAbwTypisch = abw; schlimmsteF = f; }
                }

                const std::string wo = std::string (typName (p.typ)) + " @" + zahl (fs, 0);
                pruefe (maxAbwTypisch <= 0.05 && maxAbwRand <= 0.1,
                        std::string (typName (p.typ)) + "_gegen_analytische_antwort (M-10/M-11) " + zahl (fs, 0) + " Hz",
                        "typisch " + zahl (maxAbwTypisch, 4) + " dB bei " + zahl (schlimmsteF, 1)
                        + " Hz, Rand " + zahl (maxAbwRand, 4) + " dB, " + std::to_string (g.size()) + " Stellen");
            }
        }

        // C1 - die Messart selbst (M-13, Abweichung 1 in §9.6): dass die
        // Impulsantwort dasselbe misst wie ein eingeschwungener Sinus, wird
        // nicht behauptet, sondern gemessen.
        {
            const double fs = 48000.0;
            const double stellen[] = { 200.0, 1000.0, 5000.0 };
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 6.0);
            const auto ref = refEntwurf (Filtertyp::bell, fs, 1000.0, 1.0, 6.0);

            auto kernIr = neuerKern (fs, 512);
            kernIr->uebernehmeZustand (s);
            const auto ir = impulsantwort (*kernIr, 32768, 0, 512, 2048);

            double maxAbw = 0.0;
            for (double f : stellen)
            {
                auto kernSin = neuerKern (fs, 512);
                kernSin->uebernehmeZustand (s);
                const double vomSinus = sinusDb (*kernSin, fs, f);
                const double vomIr    = irDb (ir, fs, f);
                const double soll     = refDb (ref, fs, f);
                maxAbw = std::max (maxAbw, std::abs (vomSinus - vomIr));
                maxAbw = std::max (maxAbw, std::abs (vomSinus - soll));
            }
            pruefe (maxAbw <= 0.05, "impulsantwort_misst_wie_ein_eingeschwungener_sinus (M-13)",
                    "groesste Abweichung " + zahl (maxAbw, 4) + " dB ueber drei Stellen");
        }

        // C2 - B-2 / M-19: die RUHEantwort eines dynamischen Bandes (Auslenkung
        // exakt 0: kein Detektor) klingt wie sein statischer Zustand. Der
        // TPT-SVF ist unter der bilinearen Abbildung derselbe Prototyp wie der
        // RBJ-Entwurf, gemessen gegen dieselbe eigenstaendige Formel und
        // dasselbe Gitter mit denselben Toleranzen wie M-10/M-11.
        {
            struct DynPruefling { Filtertyp typ; double f; double q; double gain; };
            const DynPruefling dyn[] = {
                { Filtertyp::highShelf, 1000.0, 0.707,   6.0 },
                { Filtertyp::highShelf, 3000.0, 1.0,   -12.0 },
                { Filtertyp::lowShelf,   300.0, 0.707,   6.0 },
                { Filtertyp::bell,      1000.0, 2.0,    -9.0 },
            };
            for (double fs : sampleraten)
                for (const auto& p : dyn)
                {
                    auto kern = neuerKern (fs, 512);
                    auto s = machSatz (true);
                    belege (s, 0, p.typ, p.f, p.q, p.gain);
                    machDynamisch (s, 0, -12.0, -60.0, 1.0, 0.0, 20.0, Sidechain::none);
                    kern->uebernehmeZustand (s);

                    const auto ir  = impulsantwort (*kern, 32768, 0, 512, 2048);
                    const auto ref = refEntwurf (p.typ, fs, p.f, p.q, p.gain);
                    const auto g   = gitterFuer (fs, p.typ, p.f);

                    double maxTypisch = 0.0, maxRand = 0.0;
                    for (size_t i = 0; i < g.size(); ++i)
                    {
                        const double abw = abweichungDb (irDb (ir, fs, g[i]), refDb (ref, fs, g[i]));
                        if (i == 0 || i + 1 >= g.size()) maxRand = std::max (maxRand, abw);
                        else                             maxTypisch = std::max (maxTypisch, abw);
                    }
                    pruefe (maxTypisch <= 0.05 && maxRand <= 0.1,
                            std::string ("dynamisches_") + typName (p.typ) + "_ruhe_gegen_rbj (M-19, B-2) "
                            + zahl (p.gain, 0) + " dB @" + zahl (fs, 0),
                            "typisch " + zahl (maxTypisch, 4) + " dB, Rand " + zahl (maxRand, 4) + " dB, "
                            + std::to_string (g.size()) + " Stellen");
                }

            // -24 dB Gesamtgain (gain_db -12, Auslenkung -12): dort bricht ein
            // falsches Vorzeichen des Mischterms am sichtbarsten. Der Detektor
            // steht im Plateau (Pegel weit ueber Threshold), der Koeffizientensatz
            // ist damit konstant, und der eingeschwungene Ton misst den Gang.
            const double fs = 48000.0;
            double maxAbw = 0.0;
            bool plateau = true;
            std::string detail;
            for (double f : { 250.0, 1000.0, 4000.0 })
            {
                auto kern = neuerKern (fs, 512);
                auto s = machSatz (true);
                belege (s, 0, Filtertyp::highShelf, 1000.0, 0.707, -12.0);
                machDynamisch (s, 0, -12.0, -60.0, 1.0, 0.0, 20.0);
                kern->uebernehmeZustand (s);
                long long n0 = 0;
                std::vector<double> aufz;
                fahreStereoTon (*kern, fs, f, 0.5, n0, 48000, 512);
                fahreStereoTon (*kern, fs, f, 0.5, n0, 16384, 512, &aufz);
                double w[param::kSlots];
                kern->auslenkungenDb (w);
                if (w[0] != -12.0) plateau = false;
                const double gemessen = 20.0 * std::log10 (tonAmplitude (aufz, fs, f) / 0.5);
                const double soll = refDb (refEntwurf (Filtertyp::highShelf, fs, 1000.0, 0.707, -24.0), fs, f);
                maxAbw = std::max (maxAbw, std::abs (gemessen - soll));
                detail += zahl (f, 0) + " Hz: " + zahl (gemessen, 3) + " gegen " + zahl (soll, 3) + " dB; ";
            }
            pruefe (plateau && maxAbw <= 0.05,
                    "dynamischer_high_shelf_bei_minus_24_db_gegen_rbj (M-19, U15, B-2)", detail);
        }

        // M-12: Nyquist-Kappung bei 0,45 fs, und der PERSISTENTE Wert bleibt.
        {
            auto kern = neuerKern (44100.0, 512);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 20000.0, 1.0, 6.0);
            kern->uebernehmeZustand (s);
            kern->pflege();

            // Die Bank, die gerade publiziert wurde, traegt die Kappung.
            bool gekappt = false; double wirksam = 0.0;
            for (int i = 0; i < DspBankPool::kBaenke; ++i)
            {
                const auto& b = kern->pool().bank (i).programm.baender[0];
                if (b.aktiv) { gekappt = b.gekapptFreq; wirksam = b.freqHzWirksam; }
            }
            pruefe (gekappt && std::abs (wirksam - 44100.0 * 0.45) < 1e-9,
                    "nyquist_kappung_bei_045_fs (M-12)",
                    "wirksam " + zahl (wirksam, 3) + " Hz statt 20000");
            pruefe (s.werte[(size_t) param::indexBandV1 (0, param::kFreqHz)].zahl == 20000.0,
                    "kappung_aendert_den_persistenten_wert_nicht (M-12)",
                    "DTO traegt weiter 20000 Hz");

            // Bei 96 kHz greift der Deckel nicht.
            auto kern96 = neuerKern (96000.0, 512);
            kern96->uebernehmeZustand (s);
            bool gekappt96 = true;
            for (int i = 0; i < DspBankPool::kBaenke; ++i)
            {
                const auto& b = kern96->pool().bank (i).programm.baender[0];
                if (b.aktiv) gekappt96 = b.gekapptFreq;
            }
            pruefe (! gekappt96, "bei_96_khz_greift_der_deckel_nicht (M-12)",
                    "0,45 * 96000 = 43200 Hz liegt ueber 20000");
        }

        // M-16: Koeffizienten und Zustaende sind double.
        {
            pruefe (sizeof (Biquad::b0) == 8 && sizeof (BiquadZustand::z1) == 8
                    && sizeof (SvfZustand::ic1eq) == 8,
                    "koeffizienten_und_zustaende_sind_double (M-16)",
                    "je 8 Byte");
        }
    }

    //==========================================================================
    std::cout << std::endl << "== D - Kaskade und Richtung (M-15) ==" << std::endl;
    {
        const double fs = 48000.0;

        // D1 - Kaskade statt Parallelsumme: zwei +6-dB-Bells auf derselben
        // Frequenz ergeben kaskadiert +12 dB, parallel rund +9,5 dB.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 6.0);
            belege (s, 1, Filtertyp::bell, 1000.0, 1.0, 6.0);
            kern->uebernehmeZustand (s);
            const auto ir = impulsantwort (*kern, 32768, 0, 512, 2048);

            const auto ref = refEntwurf (Filtertyp::bell, fs, 1000.0, 1.0, 6.0);
            const double sollProdukt = 2.0 * refDb (ref, fs, 1000.0);
            const double gemessen    = irDb (ir, fs, 1000.0);
            pruefe (std::abs (gemessen - sollProdukt) <= 0.05,
                    "kaskade_gegen_produkt_der_einzelantworten (M-15)",
                    "gemessen " + zahl (gemessen, 4) + " dB, Produkt " + zahl (sollProdukt, 4) + " dB");
        }

        // D2 - Richtung 0 -> 7 gegen ein UNABHAENGIG gerechnetes, GEORDNETES
        // Referenzergebnis. Zwei Anordnungen, die sich unterscheiden, legen
        // die Richtung nicht fest (MN2, B-09) - eine rueckwaerts laufende
        // Bank bestuende diese Probe genauso.
        {
            auto s = machSatz (true);
            belege (s, 1, Filtertyp::bell, 800.0, 2.0, 9.0, Kanalmodus::left);
            belege (s, 4, Filtertyp::bell, 800.0, 2.0, -6.0, Kanalmodus::mid);

            auto kern = neuerKern (fs, 512);
            kern->uebernehmeZustand (s);
            const auto irL = impulsantwort (*kern, 16384, 0, 512, 2048);

            // Dieselbe Rechnung im TEST, Sample fuer Sample, Slot 1 vor
            // Slot 4 - mit einer hier ausgeschriebenen Biquad-Kette.
            const auto rLeft = refEntwurf (Filtertyp::bell, fs, 800.0, 2.0, 9.0);
            const auto rMid  = refEntwurf (Filtertyp::bell, fs, 800.0, 2.0, -6.0);

            struct RefZ { double z1 = 0.0, z2 = 0.0;
                          double tick (const RefBiquad& f, double x)
                          { const double y = f.b0 * x + z1;
                            z1 = f.b1 * x - f.a1 * y + z2;
                            z2 = f.b2 * x - f.a2 * y; return y; } };

            auto referenz = [&] (bool vorwaerts)
            {
                RefZ zLeft, zMid;
                std::vector<double> aus;
                aus.reserve (16384);
                for (int n = 0; n < 16384; ++n)
                {
                    double L = (n == 0) ? 1.0 : 0.0;
                    double R = L;
                    auto slotLinks = [&] { L = zLeft.tick (rLeft, L); };
                    auto slotMid   = [&] { const double m = (L + R) * 0.5, sd = (L - R) * 0.5;
                                           const double y = zMid.tick (rMid, m);
                                           L = y + sd; R = y - sd; };
                    if (vorwaerts) { slotLinks(); slotMid(); }
                    else           { slotMid();   slotLinks(); }
                    aus.push_back (L);
                }
                return aus;
            };

            const auto vorwaerts = referenz (true);
            const auto rueckwaerts = referenz (false);

            double abwVor = 0.0, abwRueck = 0.0;
            for (size_t i = 0; i < irL.size() && i < vorwaerts.size(); ++i)
            {
                abwVor   = std::max (abwVor,   std::abs (irL[i] - vorwaerts[i]));
                abwRueck = std::max (abwRueck, std::abs (irL[i] - rueckwaerts[i]));
            }
            pruefe (abwVor < 1e-9,
                    "richtung_null_nach_sieben_gegen_geordnetes_golden (M-15)",
                    "Abweichung vorwaerts " + zahl (abwVor, 12));
            pruefe (abwRueck > 1e-6,
                    "ein_rueckwaertslauf_wuerde_das_golden_reissen (M-15)",
                    "Abweichung rueckwaerts " + zahl (abwRueck, 9) + " - die Probe trennt die Richtung");
        }
    }

    //==========================================================================
    std::cout << std::endl << "== E - Rampe und Zipper (M-17) ==" << std::endl;
    {
        // Ein kontinuierlicher Wert wird bewegt: der Output-Trim laeuft
        // ueber GENAU EINE definierte Rampe, sample-genau. Gemessen wird
        // gegen die im Test ausgeschriebene Ideallinie; eine blockweise
        // Aktualisierung erzeugt Treppen weit ueber -100 dBFS.
        const double fs = 48000.0;
        auto kern = neuerKern (fs, 64);
        auto s = machSatz (true);
        kern->uebernehmeZustand (s);
        fahreStille (*kern, 1024, 64);
        kern->pflege();

        // DC 1,0 hinein: jeder Sprung im Ausgang ist ein Sprung im Gain. Der
        // letzte Wert VOR dem Wechsel gehoert dazu - ein Sofortsprung liegt
        // genau dort, und ein Test, der erst ab dem ersten Sample NACH dem
        // Wechsel misst, sieht ihn nicht.
        std::vector<float> a (64), b (64);
        float* kan[2] = { a.data(), b.data() };
        std::fill (a.begin(), a.end(), 1.0f);
        std::fill (b.begin(), b.end(), 1.0f);
        kern->verarbeite (kan, 2, 64);
        std::vector<double> aus { (double) a[63] };

        auto lauter = s;
        setzeGlobal (lauter, "v1.global.output_trim_db", 6.0);
        kern->uebernehmeZustand (lauter);

        for (int blk = 0; blk < 16; ++blk)
        {
            std::fill (a.begin(), a.end(), 1.0f);
            std::fill (b.begin(), b.end(), 1.0f);
            kern->verarbeite (kan, 2, 64);
            for (int i = 0; i < 64; ++i) aus.push_back ((double) a[(size_t) i]);
        }

        double maxStufe = 0.0;
        const double erwarteterSchritt = (std::pow (10.0, 6.0 / 20.0) - 1.0) / (double) kRampeSamples;
        for (size_t i = 1; i < aus.size(); ++i)
            maxStufe = std::max (maxStufe, std::abs (aus[i] - aus[i - 1]));

        // Ein sample-genauer Lauf bleibt beim Schritt der Rampe; eine
        // blockweise Aktualisierung oder ein Sofortsprung springt um ein
        // Vielfaches davon.
        const double deckel = 6.0 * erwarteterSchritt;
        pruefe (maxStufe <= deckel, "zipperresiduen_unter_minus_100_dbfs (M-17)",
                "groesste Stufe " + zahl (maxStufe, 8) + " gegen Rampenschritt "
                + zahl (erwarteterSchritt, 8));

        // Und die FORM: die Kurve deckt sich mit der HIER ausgeschriebenen
        // Ideallinie. Eine Stufenmessung allein liesse eine Rampe durch, die
        // zu frueh oder zu spaet am Ziel ist - der Crossfade der neuen Bank
        // verdeckt einen Sofortsprung sonst, weil beide Baenke dieselbe
        // globale Rampe lesen.
        {
            const double ziel = std::pow (10.0, 6.0 / 20.0);
            double maxAbw = 0.0;
            for (size_t i = 1; i < aus.size(); ++i)
            {
                const double n = (double) i;   // Tick 1 ist das erste Sample nach dem Wechsel
                const double t = n >= (double) kRampeSamples ? 1.0 : n / (double) kRampeSamples;
                maxAbw = std::max (maxAbw, std::abs (aus[i] - (1.0 + (ziel - 1.0) * t)));
            }
            pruefe (maxAbw < 1e-5, "die_rampe_folgt_der_ausgeschriebenen_ideallinie (M-17)",
                    "groesste Abweichung " + zahl (maxAbw, 9));
        }

        // Und: die Rampe endet BITGENAU auf dem Ziel.
        const double ziel = std::pow (10.0, 6.0 / 20.0);
        const double letzter = aus.back();
        pruefe (std::abs (letzter - ziel) < 1e-6, "rampe_endet_bitgenau_auf_dem_ziel (M-02)",
                "Ende " + zahl (letzter, 9) + " gegen Ziel " + zahl (ziel, 9));

        // B-4 / M-17 / R8: Bandwerte mit `wechsel = rampe` laufen ueber EINE
        // Rampe, ohne neue kalte Bank und ohne Zustandsreset. Die Referenz ist
        // hier ausgeschrieben: ein DF2T-Biquad, dessen Zustand ueber den GANZEN
        // Lauf stetig bleibt und dessen RBJ-Koeffizienten ab dem Blockrand des
        // Wechsels ueber kRampeSamples linear laufen (das erste Sample traegt
        // 1/256, das letzte 1). Ein Sprung auf den Zielwert, ein Crossfade
        // oder ein kalter Neustart weichen davon um Groessenordnungen ab.
        {
            struct Sprung { const char* name; int feld; double von; double nach; };
            const Sprung spruenge[] = {
                { "gain_db", param::kGainDb, 3.0,    9.0 },
                { "freq_hz", param::kFreqHz, 1000.0, 1400.0 },
                { "q",       param::kQ,      1.0,    3.0 },
            };
            const double tonHz = 1100.0;
            const int bg = 64, wechselBei = 2048, laenge = 4096;
            for (const auto& sp : spruenge)
            {
                auto k = neuerKern (fs, bg);
                auto sa = machSatz (true);
                belege (sa, 0, Filtertyp::bell, 1000.0, 1.0, 3.0);
                sa.werte[(size_t) param::indexBandV1 (0, sp.feld)].zahl = sp.von;
                auto sb = sa;
                sb.werte[(size_t) param::indexBandV1 (0, sp.feld)].zahl = sp.nach;
                const auto wert = [] (const param::DspSatz& s, int feld)
                { return s.werte[(size_t) param::indexBandV1 (0, feld)].zahl; };
                const auto refA = refEntwurf (Filtertyp::bell, fs, wert (sa, param::kFreqHz), wert (sa, param::kQ), wert (sa, param::kGainDb));
                const auto refB = refEntwurf (Filtertyp::bell, fs, wert (sb, param::kFreqHz), wert (sb, param::kQ), wert (sb, param::kGainDb));

                k->uebernehmeZustand (sa);
                std::vector<float> l ((size_t) bg), r ((size_t) bg);
                float* kanR[2] = { l.data(), r.data() };
                double z1 = 0.0, z2 = 0.0, maxRes = 0.0;
                for (int n0 = 0; n0 < laenge; n0 += bg)
                {
                    if (n0 == wechselBei) { k->pflege(); k->uebernehmeZustand (sb); }
                    for (int i = 0; i < bg; ++i)
                    {
                        l[(size_t) i] = (float) (0.5 * std::sin (2.0 * kPiRef * tonHz * (double) (n0 + i) / fs));
                        r[(size_t) i] = l[(size_t) i];
                    }
                    k->verarbeite (kanR, 2, bg);
                    const double* t = k->tap (Tap::postCommitted, 0);
                    for (int i = 0; i < bg; ++i)
                    {
                        const int n = n0 + i;
                        RefBiquad c = refA;
                        if (n >= wechselBei)
                        {
                            const int m = n - wechselBei;
                            const double w = (m + 1 >= kRampeSamples) ? 1.0 : (double) (m + 1) / (double) kRampeSamples;
                            c.b0 = refA.b0 + (refB.b0 - refA.b0) * w;
                            c.b1 = refA.b1 + (refB.b1 - refA.b1) * w;
                            c.b2 = refA.b2 + (refB.b2 - refA.b2) * w;
                            c.a1 = refA.a1 + (refB.a1 - refA.a1) * w;
                            c.a2 = refA.a2 + (refB.a2 - refA.a2) * w;
                        }
                        const double x = (double) (float) (0.5 * std::sin (2.0 * kPiRef * tonHz * (double) n / fs));
                        const double y = c.b0 * x + z1;
                        z1 = c.b1 * x - c.a1 * y + z2;
                        z2 = c.b2 * x - c.a2 * y;
                        // Ab dem Ende des Einblend-Crossfades der ersten Bank
                        // ist der Tap der reine Filterausgang.
                        if (n >= kFadeSamples && t != nullptr) maxRes = std::max (maxRes, std::abs (t[i] - y));
                    }
                }
                pruefe (maxRes < 1e-5,
                        std::string ("bandwert_") + sp.name + "_rampt_ohne_zustandsreset (M-17, R8, B-4)",
                        "groesstes Residuum gegen die stetige Rampe " + zahl (maxRes, 12) + " (-100 dBFS = 1e-5)");
            }
        }

        // NAK-311 W03 (Manifest NAK-311 §6.3 M-43 bis M-45, M-54, M-55; R-311-1,
        // T3-15-08): ein Wechsel an einem ANDEREN Slot laesst die Historie eines
        // unveraenderten Slots stehen. Slot 0 traegt ein Band, Slot 5 ist belegt,
        // `gain_db` 0 und aus. Nur `v1.band.5.enabled` wird wahr (M-43), oder
        // Slot 5 wechselt eingeschaltet den Typ Bell -> Low-Shelf mit 0 dB (M-44).
        // Beide Slot-5-Entwuerfe sind mit Zustand 0 bitgenau die Einheit
        // (b0 = 1, b1 = a1, b2 = a2): wird Slot 0 uebertragen, ist der Ausgang ab
        // Fade-Ende bytegleich zum Referenzkern. Vorgeschichte mindestens 2 s bis
        // zu einer Periodengrenze, Mass nach F-9 ueber 100 Perioden.
        {
            enum class Wechsel { einschalten, typwechsel };
            struct Ergebnis
            {
                double mass = 0.0, erste = 0.0;
                int schlimmste = -1, periode = 0;
                long long abw = -2;
                std::uint64_t uT = 0, uR = 0;
            };
            const auto fall = [] (double rate, double f0, double q, double gainDb, double tonHz, double amplitude,
                                  Wechsel wechsel, int blockGroesse, int maxBlock)
            {
                Ergebnis erg;
                erg.periode = (int) std::llround (rate / tonHz);
                const int perioden = (int) std::ceil (2.0 * rate / (double) erg.periode);
                int vorBlock = erg.periode;
                for (int t = 1; t <= erg.periode; ++t)
                    if (erg.periode % t == 0 && erg.periode / t <= std::min (maxBlock, 512)) { vorBlock = erg.periode / t; break; }

                auto vorher = machSatz (true);
                belege (vorher, 0, Filtertyp::bell, f0, q, gainDb);
                belege (vorher, 5, Filtertyp::bell, 1000.0, 1.0, 0.0);
                vorher.werte[(size_t) param::indexBandV1 (5, param::kEnabled)].b = (wechsel == Wechsel::typwechsel);
                auto nachher = vorher;
                if (wechsel == Wechsel::einschalten)
                    nachher.werte[(size_t) param::indexBandV1 (5, param::kEnabled)].b = true;
                else
                    nachher.werte[(size_t) param::indexBandV1 (5, param::kType)].enumIndex = (int) Filtertyp::lowShelf;

                const double w = 2.0 * kPiRef * tonHz / rate;
                const auto ton = [w, amplitude] (long long n, float& l, float& r)
                { l = r = (float) (amplitude * std::sin (w * (double) n)); };
                const auto e = fahrePaar (rate, maxBlock, vorher, nachher, Pfad::committed, ton, perioden * erg.periode,
                                          vorBlock, ton, 100 * erg.periode, blockGroesse);
                erg.mass = periodenMass (e.test.aus, e.ref.aus, 2u * (size_t) erg.periode, 100, &erg.erste, &erg.schlimmste);
                erg.abw  = ersteBitAbweichung (e.test.aus, e.ref.aus, 2u * (size_t) kFadeSamples);
                erg.uT   = e.uebernahmenTest;
                erg.uR   = e.uebernahmenRef;
                return erg;
            };
            const auto text = [] (const Ergebnis& e)
            {
                return "Mass " + zahl (e.mass, 6) + " dB gegen 0,5 dB (Perioden-RMS beider Kanaele, Periode "
                     + std::to_string (e.periode) + " Samples, 100 Perioden ab dem Umschaltblock; erste Periode "
                     + zahl (e.erste, 6) + " dB, groesste Abweichung in Periode " + std::to_string (e.schlimmste)
                     + "); ab Fade-Ende bytegleich zum Referenzkern: "
                     + (e.abw == -1 ? std::string ("ja") : "nein, erste Abweichung an Sample " + std::to_string (e.abw / 2))
                     + "; Blockrand-Uebernahmen Pruefkern " + std::to_string (e.uT) + ", Referenzkern " + std::to_string (e.uR);
            };
            const auto haelt = [] (const Ergebnis& e) { return e.mass <= 0.5 && e.abw == -1 && e.uT == 1 && e.uR == 0; };
            const std::string name = "unveraenderter_slot_behaelt_historie_bei_fremdslotwechsel";

            const auto m43 = fall (48000.0, 50.0, 8.0, 12.0, 50.0, 0.25, Wechsel::einschalten, 64, 512);
            pruefe (haelt (m43), "311/M-43 " + name + " (NAK-311 R-311-1, T3-15-08): Slot 0 Bell 50 Hz Q 8 +12 dB, "
                    "nur v1.band.5.enabled wird wahr, Slot 0 wird uebertragen", text (m43));
            const auto m44 = fall (48000.0, 50.0, 8.0, 12.0, 50.0, 0.25, Wechsel::typwechsel, 64, 512);
            pruefe (haelt (m44), "311/M-44 " + name + " (Teilfall von 311/M-43): Typwechsel an Slot 5 Bell -> Low-Shelf mit 0 dB",
                    text (m44));
            const auto m45 = fall (48000.0, 1000.0, 2.0, 12.0, 1000.0, 0.25, Wechsel::einschalten, 64, 512);
            pruefe (m45.mass <= 0.5, "311/M-45 " + name + " (Teilfall von 311/M-43, Gegenfall): Slot 0 Bell 1 kHz Q 2 +12 dB "
                    "mit 1-kHz-Sinus, Mass hoechstens 0,5 dB", text (m45));
            for (const double rate : { 44100.0, 48000.0, 96000.0 })
            {
                const auto e = fall (rate, 50.0, 8.0, 12.0, 50.0, 0.25, Wechsel::einschalten, 64, 2048);
                pruefe (haelt (e), "311/M-54 " + name + " (Teilfall von 311/M-43): " + zahl (rate / 1000.0, 1) + " kHz", text (e));
            }
            for (const int bg : { 1, 255, 256, 257, 180, 4096 })
            {
                const auto e = fall (48000.0, 50.0, 8.0, 12.0, 50.0, 0.25, Wechsel::einschalten, bg, 512);
                pruefe (haelt (e), "311/M-55 " + name + " (Teilfall von 311/M-43, B-11): Blockgroesse " + std::to_string (bg)
                        + " (maxBlock 512), genau eine Uebernahme", text (e));
            }
        }
    }

    //==========================================================================
    std::cout << std::endl << "== F - Dynamische Baender (M-18 bis M-22, M-25 bis M-27) ==" << std::endl;
    {
        const double fs = 48000.0;

        // M-19/M-20: unter Threshold und bei Range 0 steht das Band EXAKT
        // still - nicht "nahe null".
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (s, 0, -9.0, -10.0);
            kern->uebernehmeZustand (s);

            // Sehr leises Signal: der Detektor bleibt weit unter -10 dB.
            std::vector<float> a (512), b (512);
            float* kan[2] = { a.data(), b.data() };
            for (int blk = 0; blk < 40; ++blk)
            {
                for (int i = 0; i < 512; ++i)
                {
                    const double x = 1e-6 * std::sin (2.0 * kPiRef * 1000.0 * (double) (blk * 512 + i) / fs);
                    a[(size_t) i] = (float) x; b[(size_t) i] = (float) x;
                }
                kern->verarbeite (kan, 2, 512);
            }
            double werte[param::kSlots];
            kern->auslenkungenDb (werte);
            pruefe (werte[0] == 0.0, "unter_threshold_steht_das_band_exakt_still (M-19)",
                    "Auslenkung " + zahl (werte[0], 15));
        }

        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (s, 0, 0.0, -60.0);   // Range 0
            kern->uebernehmeZustand (s);
            kern->pflege();

            bool detektorLaeuft = true;
            for (int i = 0; i < DspBankPool::kBaenke; ++i)
            {
                const auto& bp = kern->pool().bank (i).programm.baender[0];
                if (bp.aktiv) detektorLaeuft = bp.detektorLaeuft;
            }
            pruefe (! detektorLaeuft, "range_null_rechnet_keinen_detektor (M-20)");

            std::vector<float> a (512), b (512);
            float* kan[2] = { a.data(), b.data() };
            for (int blk = 0; blk < 20; ++blk)
            {
                for (int i = 0; i < 512; ++i)
                { a[(size_t) i] = 0.8f; b[(size_t) i] = 0.8f; }
                kern->verarbeite (kan, 2, 512);
            }
            double werte[param::kSlots];
            kern->auslenkungenDb (werte);
            pruefe (werte[0] == 0.0, "range_null_ergibt_exakt_null_auslenkung (M-20)",
                    zahl (werte[0], 15));
        }

        // M-22: `none` heisst kein Detektor.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (s, 0, -9.0, -60.0, 10.0, 0.0, 100.0, Sidechain::none);
            kern->uebernehmeZustand (s);
            kern->pflege();
            bool laeuft = true;
            for (int i = 0; i < DspBankPool::kBaenke; ++i)
            {
                const auto& bp = kern->pool().bank (i).programm.baender[0];
                if (bp.aktiv) laeuft = bp.detektorLaeuft;
            }
            pruefe (! laeuft, "sidechain_none_liefert_keine_auslenkung (M-22)");
        }

        // M-23: `priority_sidechain` wird auf `internal` GEKLEMMT und
        // gemeldet; der persistente Wert bleibt stehen.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (s, 0, -9.0, -30.0, 10.0, 0.0, 100.0, Sidechain::prioritySidechain);
            kern->uebernehmeZustand (s);
            kern->pflege();

            bool geklemmt = false, laeuft = false;
            for (int i = 0; i < DspBankPool::kBaenke; ++i)
            {
                const auto& bp = kern->pool().bank (i).programm.baender[0];
                if (bp.aktiv) { geklemmt = bp.geklemmtSidechain; laeuft = bp.detektorLaeuft; }
            }
            pruefe (geklemmt && laeuft, "priority_sidechain_wird_geklemmt_und_gemeldet (M-23)",
                    "das Programm rechnet internal, die Klemmung steht im Bericht");
            pruefe (s.werte[(size_t) param::indexBandV1 (0, param::kSidechainSource)].enumIndex
                    == (int) Sidechain::prioritySidechain,
                    "der persistente Wert bleibt priority_sidechain (M-23)");
        }

        // M-18: die Kennlinie. Ueber Threshold folgt die Auslenkung
        // g = r * min(1, (e - t)/12) und bleibt durch die Konstruktion
        // innerhalb +/-|r|.
        {
            const double range = -9.0, thresh = -30.0;
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (s, 0, range, thresh, 1.0, 0.0, 5.0);
            kern->uebernehmeZustand (s);

            // Lautes Signal weit ueber dem Threshold: die Auslenkung muss
            // ins Plateau laufen und dort exakt `range` erreichen.
            std::vector<float> a (512), b (512);
            float* kan[2] = { a.data(), b.data() };
            for (int blk = 0; blk < 200; ++blk)
            {
                for (int i = 0; i < 512; ++i)
                {
                    const double x = 0.9 * std::sin (2.0 * kPiRef * 1000.0 * (double) (blk * 512 + i) / fs);
                    a[(size_t) i] = (float) x; b[(size_t) i] = (float) x;
                }
                kern->verarbeite (kan, 2, 512);
            }
            double werte[param::kSlots];
            kern->auslenkungenDb (werte);
            pruefe (std::abs (werte[0] - range) < 1e-9,
                    "dynamische_kennlinie_golden_plateau (M-18)",
                    "Auslenkung " + zahl (werte[0], 6) + " gegen range " + zahl (range, 6));
            pruefe (std::abs (werte[0]) <= std::abs (range) + 1e-12,
                    "auslenkung_bleibt_innerhalb_range (M-18)");

        }

        // B-14 / M-18: der Kniepunkt wird ANGEREGT, und gemessen wird die
        // WIRKUNG im Audiopfad - nicht nur der Berichtswert. Ein Stereoton auf
        // der Bandmitte (L sin, R cos) haelt die Detektorleistung konstant auf
        // a^2/2; Attack und Release 500 ms glaetten die Restwelligkeit. Bei
        // 6 dB ueber Threshold muss die Auslenkung range/2 betragen, und der
        // Bell auf der Mitte muss gain_db PLUS diese Auslenkung wirken.
        {
            const double range = -9.0, thresh = -30.0, g0 = 3.0, tonHz = 1000.0;
            auto miss = [&] (double ueber, double& auslenkung, double& wirkungDb)
            {
                auto k = neuerKern (fs, 512);
                auto s = machSatz (true);
                belege (s, 0, Filtertyp::bell, tonHz, 1.0, g0);
                machDynamisch (s, 0, range, thresh, 500.0, 0.0, 500.0);
                k->uebernehmeZustand (s);
                const double a = std::sqrt (2.0 * std::pow (10.0, (thresh + ueber) / 10.0));
                long long n0 = 0;
                std::vector<double> aufz;
                fahreStereoTon (*k, fs, tonHz, a, n0, (int) (6.0 * fs), 512);
                fahreStereoTon (*k, fs, tonHz, a, n0, 16384, 512, &aufz);
                double w[param::kSlots];
                k->auslenkungenDb (w);
                auslenkung = w[0];
                wirkungDb  = 20.0 * std::log10 (tonAmplitude (aufz, fs, tonHz) / a);
            };
            double knieAus = 0.0, knieWirk = 0.0, plateauAus = 0.0, plateauWirk = 0.0;
            miss (6.0,  knieAus,    knieWirk);
            miss (24.0, plateauAus, plateauWirk);
            const double sollKnie = range * 0.5;
            pruefe (std::abs (knieAus - sollKnie) < 0.02 && std::abs (knieWirk - (g0 + sollKnie)) < 0.05,
                    "kennlinie_im_knie_wirkt_im_audiopfad (M-18, B-14)",
                    "6 dB ueber Threshold: Auslenkung " + zahl (knieAus, 4) + " (soll " + zahl (sollKnie, 2)
                    + "), Wirkung " + zahl (knieWirk, 4) + " dB (soll " + zahl (g0 + sollKnie, 2) + ")");
            pruefe (std::abs (plateauAus - range) < 1e-9 && std::abs (plateauWirk - (g0 + range)) < 0.05,
                    "plateau_wirkt_zusaetzlich_zu_gain_db_im_audiopfad (M-18, B-14)",
                    "24 dB ueber Threshold: Auslenkung " + zahl (plateauAus, 4) + ", Wirkung "
                    + zahl (plateauWirk, 4) + " dB (soll " + zahl (g0 + range, 2) + ")");
        }

        // M-21: der Detektor hoert das BANDGEFILTERTE Signal VOR dem Band.
        // Ein Ton weit ausserhalb des Bandes darf die Dynamik nicht ausloesen
        // - sonst waere der Detektor breitbandig -, und die Regelung darf
        // nicht aufschwingen - sonst haengt er HINTER dem Band.
        {
            const double range = -9.0, thresh = -40.0;
            auto imBand   = neuerKern (fs, 512);
            auto abseits  = neuerKern (fs, 512);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 4.0, 0.0);
            machDynamisch (s, 0, range, thresh, 1.0, 0.0, 20.0);
            imBand ->uebernehmeZustand (s);
            abseits->uebernehmeZustand (s);

            auto fahreTon = [&] (DspKern& k, double tonHz)
            {
                std::vector<float> a (512), b (512);
                float* kan[2] = { a.data(), b.data() };
                for (int blk = 0; blk < 120; ++blk)
                {
                    for (int i = 0; i < 512; ++i)
                    { const double x = 0.9 * std::sin (2.0 * kPiRef * tonHz * (double) (blk * 512 + i) / fs);
                      a[(size_t) i] = (float) x; b[(size_t) i] = (float) x; }
                    k.verarbeite (kan, 2, 512);
                }
                double w[param::kSlots]; k.auslenkungenDb (w); return w[0];
            };
            const double innen  = fahreTon (*imBand,  1000.0);
            const double aussen = fahreTon (*abseits,   60.0);
            pruefe (std::abs (innen) > 1.0 && std::abs (aussen) < 0.5 * std::abs (innen),
                    "detektor_ist_bandbegrenzt (M-21)",
                    "1000 Hz " + zahl (innen, 4) + " dB, 60 Hz " + zahl (aussen, 4) + " dB");

            // Vor dem Band, nicht dahinter: mit einem stark ABSENKENDEN Band
            // wuerde ein Detektor hinter dem Band sich selbst herunterregeln
            // und die Auslenkung liefe von ihrem Plateau weg. Sie bleibt.
            auto vorherNachher = neuerKern (fs, 512);
            auto tief = machSatz (true);
            belege (tief, 0, Filtertyp::bell, 1000.0, 4.0, -12.0);
            machDynamisch (tief, 0, -12.0, -40.0, 1.0, 0.0, 20.0);
            vorherNachher->uebernehmeZustand (tief);
            const double mitAbsenkung = fahreTon (*vorherNachher, 1000.0);
            pruefe (std::abs (mitAbsenkung - (-12.0)) < 1e-6,
                    "detektor_hoert_vor_dem_band (M-21)",
                    "trotz -12 dB Absenkung bleibt die Auslenkung im Plateau: "
                    + zahl (mitAbsenkung, 6) + " dB");
        }

        // M-25: `dynamic_enabled` wechselt nur am validierten Blockrand ueber
        // ein vorbereitetes Crossfade zwischen zwei vollstaendigen Programmen
        // - nie samplegenau mitten im Block.
        {
            auto k = neuerKern (fs, 256);
            auto statisch = machSatz (true);
            belege (statisch, 0, Filtertyp::bell, 900.0, 2.0, 9.0);
            k->uebernehmeZustand (statisch);
            fahreStille (*k, 2048, 256);
            k->pflege();

            auto dynamisch = statisch;
            machDynamisch (dynamisch, 0, -9.0, -50.0, 1.0, 0.0, 20.0);

            // Ein DC-Signal regt einen Bandpass bei 900 Hz nicht an - der
            // Detektor bliebe still und statisch klaenge wie dynamisch. Es
            // laeuft deshalb ein SINUS auf der Bandmitte, und der Massstab
            // ist die ZWEITE Differenz: ein glatter Sinus haelt sie bei
            // A*omega^2, ein Sprung hebt sie auf Sprunghoehe. Das ist der
            // saubere Klickmassstab, wo das Signal selbst schon steigt.
            std::vector<float> a (256), b (256);
            float* kan[2] = { a.data(), b.data() };
            auto fuelle = [&] (long long v) {
                for (int i = 0; i < 256; ++i)
                { const double x = 0.6 * std::sin (2.0 * kPiRef * 900.0 * (double) (v + i) / fs);
                  a[(size_t) i] = (float) x; b[(size_t) i] = (float) x; } };

            std::vector<double> lauf;
            for (int blk = 0; blk < 8; ++blk)
            { fuelle ((long long) blk * 256); k->verarbeite (kan, 2, 256);
              for (int i = 0; i < 256; ++i) lauf.push_back ((double) a[(size_t) i]); }
            const size_t vorWechsel = lauf.size();
            k->uebernehmeZustand (dynamisch);
            for (int blk = 8; blk < 10; ++blk)
            { fuelle ((long long) blk * 256); k->verarbeite (kan, 2, 256);
              for (int i = 0; i < 256; ++i) lauf.push_back ((double) a[(size_t) i]); }

            auto maxZweiteDifferenz = [&] (size_t von, size_t bis)
            {
                double m = 0.0;
                for (size_t i = von; i + 1 < bis && i >= 2; ++i)
                    m = std::max (m, std::abs (lauf[i] - 2.0 * lauf[i - 1] + lauf[i - 2]));
                return m;
            };
            const double ruhig = maxZweiteDifferenz (4, vorWechsel);
            const double amWechsel = maxZweiteDifferenz (vorWechsel - 2, lauf.size());
            pruefe (amWechsel <= 4.0 * ruhig + 1e-12,
                    "dynamic_enabled_wechselt_nur_am_blockrand (M-25)",
                    "zweite Differenz am Wechsel " + zahl (amWechsel, 9)
                    + " gegen " + zahl (ruhig, 9) + " im ruhigen Lauf");
        }

        // M-27: die gemeldete Auslenkung ist GEMESSEN, nicht aus den
        // Einstellwerten hergeleitet - ein stillstehendes Band meldet 0,
        // freie und nicht dynamische Slots ebenfalls exakt 0.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 3.0);      // statisch
            belege (s, 3, Filtertyp::bell, 2000.0, 1.0, 0.0);
            machDynamisch (s, 3, -6.0, -20.0);
            kern->uebernehmeZustand (s);
            fahreStille (*kern, 4096, 512);

            double werte[param::kSlots];
            kern->auslenkungenDb (werte);
            bool alleNull = true;
            for (int i = 0; i < param::kSlots; ++i) if (werte[i] != 0.0) alleNull = false;
            pruefe (alleNull, "auslenkung_ist_gemessen_nicht_gerechnet (M-27)",
                    "bei Stille meldet auch das dynamische Band exakt 0");
        }

        // M-26: Attack, Hold und Release sind DREI Stufen. Mit langem Hold
        // bleibt der Wert nach dem Pegelabfall stehen.
        {
            auto ohneHold = neuerKern (fs, 512);
            auto mitHold  = neuerKern (fs, 512);
            auto s1 = machSatz (true), s2 = machSatz (true);
            belege (s1, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
            belege (s2, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
            // Threshold und Release sind so gewaehlt, dass der Pegel nach
            // der Stille WIRKLICH unter den Threshold faellt - mit -40 dB
            // und 50 ms Release standen beide Laeufe im Plateau und die
            // Probe mass gar nichts.
            machDynamisch (s1, 0, -9.0, -10.0, 1.0,   0.0, 20.0);
            machDynamisch (s2, 0, -9.0, -10.0, 1.0, 400.0, 20.0);
            ohneHold->uebernehmeZustand (s1);
            mitHold ->uebernehmeZustand (s2);

            auto fahre = [&] (DspKern& k)
            {
                std::vector<float> a (512), b (512);
                float* kan[2] = { a.data(), b.data() };
                // laut ...
                for (int blk = 0; blk < 60; ++blk)
                {
                    for (int i = 0; i < 512; ++i)
                    { const double x = 0.9 * std::sin (2.0 * kPiRef * 1000.0 * (double) (blk * 512 + i) / fs);
                      a[(size_t) i] = (float) x; b[(size_t) i] = (float) x; }
                    k.verarbeite (kan, 2, 512);
                }
                // ... dann still, kurz
                for (int blk = 0; blk < 8; ++blk)
                {
                    std::fill (a.begin(), a.end(), 0.0f); std::fill (b.begin(), b.end(), 0.0f);
                    k.verarbeite (kan, 2, 512);
                }
                double w[param::kSlots]; k.auslenkungenDb (w); return w[0];
            };
            const double a1 = fahre (*ohneHold);
            const double a2 = fahre (*mitHold);
            pruefe (std::abs (a2) > std::abs (a1) + 0.5,
                    "attack_hold_release_sind_drei_stufen (M-26)",
                    "ohne Hold " + zahl (a1, 4) + " dB, mit 400 ms Hold " + zahl (a2, 4) + " dB");
        }

        // B-15 / M-26: die SPRUNGANTWORT bei 44,1 / 48 / 96 / 192 kHz. Ein
        // Stereoton auf der Bandmitte haelt die Detektorleistung konstant auf
        // Pss = Threshold + 12 dB; mit range -12 dB bildet die Kennlinie im
        // Knie jedes dB Leistung auf ein dB Auslenkung ab. Damit ist gemessen:
        //   Attack  = Zeit ab Tonbeginn, bis die Leistung 1 - 1/e von Pss
        //             erreicht (Auslenkung -10,013 dB);
        //   Hold    = Zeit ab Tonende, bis die Auslenkung das Plateau um mehr
        //             als 0,01 dB verlaesst;
        //   Release = Zeit ab Holdende, bis die Leistung auf 1/e gefallen ist
        //             (Auslenkung -7,657 dB).
        // Toleranz 1 ms je Stufe (E-25): die Gruppenlaufzeit des Detektor-
        // Bandpasses (Q 0,707 bei 1 kHz: 0,23 ms), die Steuerrate von acht
        // Samples (hoechstens 0,18 ms) und die Schwellenaufloesung liegen
        // darunter; eine auf 48 kHz festgeschriebene Umrechnung verfehlt jede
        // andere Rate um mindestens 1,77 ms (44,1 kHz) und 96 kHz um 20 ms.
        {
            const double attackMs = 20.0, holdMs = 30.0, releaseMs = 50.0, toleranzMs = 1.0;
            const double thresh = -30.0;
            const double pss = std::pow (10.0, (thresh + 12.0) / 10.0);
            const double amp = std::sqrt (2.0 * pss);
            const double attackSchwelle  = -(12.0 + 10.0 * std::log10 (1.0 - std::exp (-1.0)));
            const double releaseSchwelle = -(12.0 - 10.0 / std::log (10.0));
            double zeiten[4][3] = {};
            bool gefunden = true;
            std::string detail;
            for (int ri = 0; ri < 4; ++ri)
            {
                const double rate = sampleraten[ri];
                auto k = neuerKern (rate, 512);
                auto s = machSatz (true);
                belege (s, 0, Filtertyp::bell, 1000.0, 0.707, 0.0);
                machDynamisch (s, 0, -12.0, thresh, attackMs, holdMs, releaseMs);
                k->uebernehmeZustand (s);
                fahreStille (*k, 4096, 512);

                long long n0 = 0;
                std::vector<double> aus;
                fahreStereoTon (*k, rate, 1000.0, amp, n0,
                                (int) std::llround (10.0 * attackMs * 0.001 * rate), 1, nullptr, &aus);
                const size_t stufe = aus.size();
                fahreStereoTon (*k, rate, 1000.0, 0.0, n0,
                                (int) std::llround ((holdMs + 5.0 * releaseMs) * 0.001 * rate), 1, nullptr, &aus);

                size_t nAttack = aus.size(), nHold = aus.size(), nRelease = aus.size();
                for (size_t i = 0; i < stufe; ++i)
                    if (aus[i] <= attackSchwelle) { nAttack = i; break; }
                const double plateauWert = aus[stufe - 1];
                for (size_t i = stufe; i < aus.size(); ++i)
                    if (aus[i] > plateauWert + 0.01) { nHold = i; break; }
                for (size_t i = nHold; i < aus.size(); ++i)
                    if (aus[i] >= releaseSchwelle) { nRelease = i; break; }
                if (nAttack >= aus.size() || nHold >= aus.size() || nRelease >= aus.size()) gefunden = false;

                zeiten[ri][0] = (double) nAttack * 1000.0 / rate;
                zeiten[ri][1] = (double) (nHold - stufe) * 1000.0 / rate;
                zeiten[ri][2] = (double) (nRelease - nHold) * 1000.0 / rate;
                detail += zahl (rate / 1000.0, 1) + " kHz: A " + zahl (zeiten[ri][0], 2) + " H "
                          + zahl (zeiten[ri][1], 2) + " R " + zahl (zeiten[ri][2], 2) + " ms; ";
            }
            const double soll[3] = { attackMs, holdMs, releaseMs };
            bool innerhalb = gefunden, gleich = gefunden;
            for (int st = 0; st < 3; ++st)
            {
                double lo = 1e9, hi = -1e9;
                for (int ri = 0; ri < 4; ++ri)
                {
                    if (std::abs (zeiten[ri][st] - soll[st]) > toleranzMs) innerhalb = false;
                    lo = std::min (lo, zeiten[ri][st]);
                    hi = std::max (hi, zeiten[ri][st]);
                }
                if (hi - lo > toleranzMs) gleich = false;
            }
            pruefe (innerhalb, "attack_hold_release_als_sprungantwort_bei_vier_raten (M-26, B-15)",
                    "soll A 20 H 30 R 50 ms +/- 1 ms; " + detail);
            pruefe (gleich, "dieselbe_ms_angabe_ergibt_bei_jeder_rate_dieselbe_zeit (M-26, B-15)",
                    "Spanne je Stufe ueber vier Raten <= 1 ms");
        }

        // W-2 / B-4 / M-17 / R8: attack_ms, hold_ms und release_ms tragen
        // `wechsel = rampe`. Im Rampenuebergang laufen ihre Koeffizienten ueber
        // kRampeSamples, der Huellkurvenzustand wandert mit (E-28). Gemessen
        // wird Sample fuer Sample gegen die HIER ausgeschriebene Idealrampe:
        // die Huellkurvenleistung (ihre einzigen Koeffizienten sind die zwei
        // Pole), daraus die gemeldete Auslenkung an jedem Steuerschritt, und
        // beim Hold der Holdzaehler, den jede steigende Probe mit der Haltezeit
        // laedt. Ein Stereoton auf der Bandmitte haelt die Detektorleistung
        // konstant auf amp^2/2 (E-25); Threshold -20 dB legt den Pegel ins Knie.
        {
            struct Fall { const char* name; int feld; double von, nach; bool ton; };
            const Fall faelle[] = {
                { "attack_ms",  param::kAttackMs,  500.0, 0.1,   true  },
                { "hold_ms",    param::kHoldMs,    400.0, 100.0, true  },
                { "release_ms", param::kReleaseMs, 500.0, 5.0,   false },
            };
            const double amp = 0.5, pss = amp * amp * 0.5, thresh = -20.0, range = -6.0;
            const auto pol = [&] (double ms) { return std::exp (-1.0 / (fs * ms * 0.001)); };
            const auto kennlinieRef = [&] (double leistung)
            {
                const double ueber = 10.0 * std::log10 (leistung) - thresh;
                return ueber <= 0.0 ? 0.0 : range * std::min (1.0, ueber / 12.0);
            };
            for (const auto& f : faelle)
            {
                auto k = neuerKern (fs, 64);
                auto sa = machSatz (true);
                belege (sa, 0, Filtertyp::bell, 1000.0, 0.707, 0.0);
                machDynamisch (sa, 0, range, thresh, 500.0, 0.0, 500.0);
                sa.werte[(size_t) param::indexBandV1 (0, f.feld)].zahl = f.von;
                auto sb = sa;
                sb.werte[(size_t) param::indexBandV1 (0, f.feld)].zahl = f.nach;
                const auto wert = [] (const param::DspSatz& s, int feld)
                { return s.werte[(size_t) param::indexBandV1 (0, feld)].zahl; };

                k->uebernehmeZustand (sa);
                long long n0 = 0;
                fahreStereoTon (*k, fs, 1000.0, amp, n0, 9600, 64);                // 200 ms Ton: die Huellkurve steigt
                if (! f.ton) fahreStereoTon (*k, fs, 1000.0, 0.0, n0, 4800, 64);  // 100 ms Stille: Release laeuft
                k->pflege();

                int cA = -1, cQ = -1, kA = -1, kQ = -1;
                k->gefahreneSlots (cA, cQ, kA, kQ);
                double lRef = k->pool().bank (cA).baender[0].huelle.leistung;
                const double l0 = lRef;
                k->uebernehmeZustand (sb);

                const double hsVon  = (double) std::llround (wert (sa, param::kHoldMs) * 0.001 * fs);
                const double hsNach = (double) std::llround (wert (sb, param::kHoldMs) * 0.001 * fs);
                double maxResL = 0.0, maxResG = 0.0, maxResHold = 0.0, gRef = 0.0;
                for (int m = 0; m < 2 * kRampeSamples; ++m)
                {
                    const long long n = n0;
                    std::vector<double> aus;
                    fahreStereoTon (*k, fs, 1000.0, f.ton ? amp : 0.0, n0, 1, 1, nullptr, &aus);
                    k->gefahreneSlots (cA, cQ, kA, kQ);
                    const auto& h = k->pool().bank (cA).baender[0].huelle;

                    // Das erste Sample des Uebergangs traegt 1/256, ab dem 256. gilt das Ziel.
                    const double w  = (m + 1 >= kRampeSamples) ? 1.0 : (double) (m + 1) / (double) kRampeSamples;
                    const double pa = pol (wert (sa, param::kAttackMs))  + (pol (wert (sb, param::kAttackMs))  - pol (wert (sa, param::kAttackMs)))  * w;
                    const double pr = pol (wert (sa, param::kReleaseMs)) + (pol (wert (sb, param::kReleaseMs)) - pol (wert (sa, param::kReleaseMs))) * w;
                    lRef = f.ton ? pa * lRef + (1.0 - pa) * pss : pr * lRef;
                    if (n % kDynamikSchritt == 0) gRef = kennlinieRef (lRef);

                    maxResL = std::max (maxResL, std::abs (h.leistung - lRef) / pss);
                    maxResG = std::max (maxResG, std::abs (aus[0] - gRef));
                    if (f.ton)
                        maxResHold = std::max (maxResHold, std::abs ((double) h.holdRest
                                                                     - (double) std::llround (hsVon + (hsNach - hsVon) * w)));
                }
                pruefe (maxResL < 1e-6 && maxResG < 1e-4 && maxResHold <= 1.0,
                        std::string ("huellkurvenwert_") + f.name + "_rampt_ohne_zustandsreset (M-17, R8, B-4, W-2)",
                        "Residuum gegen die Idealrampe: Leistung " + zahl (maxResL, 12) + " von Pss, Auslenkung "
                        + zahl (maxResG, 9) + " dB, Holdzaehler "
                        + (f.ton ? zahl (maxResHold, 0) + " Samples" : std::string ("ungemessen (kein Anstieg)"))
                        + "; Leistung beim Wechsel "
                        + zahl (l0 / pss, 4) + " Pss");
                // NAK-289 Etappe 1 (bugprone-incorrect-roundings): die Haltezeit
                // der Rampe ist auf das Sample genau die gerundete Mischung.
                // Die Zeile darueber laesst einen Holdzaehler Abstand zu; ein
                // Abschneiden statt Runden faellt erst hier.
                if (f.ton)
                    pruefe (maxResHold == 0.0,
                            std::string ("huellkurvenwert_") + f.name + "_holdzaehler_rundet_exakt (NAK-289)",
                            "groesster Abstand zu llround der Idealrampe: " + zahl (maxResHold, 0) + " Samples");
            }
        }

        // W-3 / B-4 / R8 / M-17: dynamic_range_db -12 -> 0 und 0 -> -12 an
        // einem Stereoton weit ueber Threshold plus Knie. Im Plateau IST die
        // Auslenkung die Range; sie folgt deshalb an jedem Steuerschritt der
        // HIER ausgeschriebenen Idealrampe der Range (E-29). Residuum unter
        // 1e-6 dB: als Gain 1,2e-7 des Signals, bei Vollaussteuerung -138 dBFS
        // und damit unter -100 dBFS. Danach ist der Detektor aus (-12 -> 0,
        // Leistung exakt 0) beziehungsweise laeuft (0 -> -12).
        for (int richtung = 0; richtung < 2; ++richtung)
        {
            const double rVon = richtung == 0 ? -12.0 : 0.0, rNach = richtung == 0 ? 0.0 : -12.0;
            auto k = neuerKern (fs, 64);
            auto sa = machSatz (true);
            belege (sa, 0, Filtertyp::bell, 1000.0, 0.707, 0.0);
            machDynamisch (sa, 0, rVon, -60.0, 0.1, 0.0, 100.0);
            auto sb = sa;
            sb.werte[(size_t) param::indexBandV1 (0, param::kDynamicRangeDb)].zahl = rNach;
            k->uebernehmeZustand (sa);
            long long n0 = 0;
            fahreStereoTon (*k, fs, 1000.0, 0.9, n0, 4096, kDynamikSchritt);
            k->pflege();
            k->uebernehmeZustand (sb);

            double maxRes = 0.0, amErsten = 0.0;
            for (int j = 0; j < 48; ++j)
            {
                // Ein Block von kDynamikSchritt Samples: sein erstes Sample ist
                // der Steuerschritt, der Bericht danach traegt dessen Wert.
                std::vector<double> aus;
                fahreStereoTon (*k, fs, 1000.0, 0.9, n0, kDynamikSchritt, kDynamikSchritt, nullptr, &aus);
                const int m = j * kDynamikSchritt;
                const double w = (m + 1 >= kRampeSamples) ? 1.0 : (double) (m + 1) / (double) kRampeSamples;
                const double soll = w >= 1.0 ? rNach : rVon + (rNach - rVon) * w;
                if (j == 0) amErsten = aus.back();
                maxRes = std::max (maxRes, std::abs (aus.back() - soll));
            }
            int cA = -1, cQ = -1, kA = -1, kQ = -1;
            k->gefahreneSlots (cA, cQ, kA, kQ);
            const double leistungNachRampe = k->pool().bank (cA).baender[0].huelle.leistung;
            fahreStereoTon (*k, fs, 1000.0, 0.9, n0, 1024, 64);
            k->gefahreneSlots (cA, cQ, kA, kQ);
            const auto& bn = k->pool().bank (cA);
            const bool laeuft = bn.programm.baender[0].detektorLaeuft;
            const double leistungSpaeter = bn.baender[0].huelle.leistung;
            const bool danach = richtung == 0 ? (! laeuft && leistungNachRampe == 0.0 && leistungSpaeter == 0.0)
                                              : (laeuft && leistungSpaeter > 0.0);
            pruefe (maxRes < 1e-6 && danach,
                    std::string ("range_") + (richtung == 0 ? "minus_12_nach_0" : "0_nach_minus_12")
                    + "_rampt_ueber_die_volle_rampe (M-17, R8, B-4, W-3)",
                    "erster Steuerschritt " + zahl (amErsten, 6) + " dB (Idealrampe "
                    + zahl (rVon + (rNach - rVon) / (double) kRampeSamples, 6) + "), groesstes Residuum "
                    + zahl (maxRes, 12) + " dB; danach Detektor " + (laeuft ? "an" : "aus") + ", Leistung "
                    + zahl (leistungSpaeter, 9));
        }

        // M-24-nahe Zusage im Kern: `dynamic_enabled` aus laesst die fuenf
        // Werte unberuehrt - der Kern liest sie beim Wiedereinschalten
        // unveraendert aus demselben DTO (die Persistenz misst B2).
        {
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (s, 0, -7.5, -22.5, 12.5, 33.0, 250.0);
            auto aus = s;
            aus.werte[(size_t) param::indexBandV1 (0, param::kDynamicEnabled)].b = false;
            bool gleich = true;
            for (int feld : { param::kDynamicRangeDb, param::kThresholdDb, param::kAttackMs,
                              param::kHoldMs, param::kReleaseMs })
            {
                const double x = s  .werte[(size_t) param::indexBandV1 (0, feld)].zahl;
                const double y = aus.werte[(size_t) param::indexBandV1 (0, feld)].zahl;
                if (std::memcmp (&x, &y, sizeof (double)) != 0) gleich = false;
            }
            pruefe (gleich, "ausschalten_aendert_nur_dynamic_enabled (M-24 im Kern)",
                    "die fuenf Werte bleiben bitgleich");
        }

        // ── NAK-311 T3-15-11: die Steuerrate, gemessen ────────────────────
        // Manifest NAK-311 §6.4, Zeilen 311/M-73 bis 311/M-75, §9 F-15.
        // ERLAUBTES VERHALTEN, gemessen - kein Verhalten aendert sich,
        // `kDynamikSchritt` bleibt 8. Berichtigt wird nur die BEGRUENDUNG
        // (DspProgramm.h und Entscheid E-6 sagten, 8 Samples seien "feiner
        // als die kuerzeste Attack"; sie sind groeber).
        //
        // Pruefling und Referenzkern unterscheiden sich in EINEM Wert: der
        // Referenzkern traegt Range 0. Derselbe SVF-Weg laeuft in beiden
        // (`nutztSvf` haengt nicht an der Range, DspProgramm.cpp), nur der
        // Detektor bleibt dort stehen. Beide fahren dasselbe Signal: erst
        // 8*m + p Samples Stille - das Steuerraster zaehlt ab dem ersten
        // Sample des Kerns, m = 32 haelt den Einblend-Crossfade aus der Ruhe
        // (kFadeSamples) vor dem Einsatz fertig -, dann den Quadraturton 0,5
        // auf der Bandmitte. Threshold -60 dB (Vertragsminimum) legt den
        // Pegel ins Plateau, die Auslenkung ist dort die volle Range.
        //
        // Gemessen wird der Tap `post_committed` Sample fuer Sample, dazu am
        // Bandzustand `schrittRest` (wo faellt der Entwurf?) und `svfVon`
        // (welcher Satz wirkt mit Gewicht 1?).
        {
            const int  phasen[] = { 0, 1, 3, 7 };
            const int  vorlaufBloecke = 32;        // 8 * 32 = 256 >= kFadeSamples
            const int  messen = 40;                // deckt d + 8 <= 15 mit Rand
            const double f0 = 1000.0, amp = 0.5;

            struct Fall { double rate; const char* zeile; const char* was; };
            const Fall faelle[] = {
                { 48000.0, "311/M-73", "erster_entwurf_wirkt_voll_nach_8_bis_15_samples" },
                { 44100.0, "311/M-74", "dieselben_samplezahlen_bei_44_1_khz" },
                { 96000.0, "311/M-75", "dieselben_samplezahlen_bei_96_khz" },
            };

            for (const auto& f : faelle)
            {
                int minErste = 1000, maxErste = -1, minVoll = 1000, maxVoll = -1;
                bool allesTraegt = true;
                std::ostringstream d;
                d << std::setprecision (9);

                for (int pi = 0; pi < 4; ++pi)
                {
                    const int p = phasen[pi];
                    const int dSoll = (kDynamikSchritt - p) % kDynamikSchritt;

                    const auto machKern = [&] (double range)
                    {
                        auto k = neuerKern (f.rate, 64);
                        auto s = machSatz (true);
                        belege (s, 0, Filtertyp::bell, f0, 0.707, 0.0);
                        machDynamisch (s, 0, range, -60.0, 0.1, 0.0, 100.0);
                        k->uebernehmeZustand (s);
                        return k;
                    };
                    auto kDyn = machKern (-12.0);
                    auto kRef = machKern (0.0);

                    long long nD = 0, nR = 0;
                    const int vorlauf = kDynamikSchritt * vorlaufBloecke + p;
                    fahreStereoTon (*kDyn, f.rate, f0, 0.0, nD, vorlauf, 1);
                    fahreStereoTon (*kRef, f.rate, f0, 0.0, nR, vorlauf, 1);

                    std::vector<double> tapD, tapR;
                    std::vector<int> rest ((size_t) messen, -1);
                    std::vector<SvfKoeffizienten> svfVon ((size_t) messen), svfNach ((size_t) messen);
                    for (int i = 0; i < messen; ++i)
                    {
                        fahreStereoTon (*kDyn, f.rate, f0, amp, nD, 1, 1, &tapD);
                        fahreStereoTon (*kRef, f.rate, f0, amp, nR, 1, 1, &tapR);
                        int cA = -1, cQ = -1, kA = -1, kQ = -1;
                        kDyn->gefahreneSlots (cA, cQ, kA, kQ);
                        const auto& z = kDyn->pool().bank (cA).baender[0];
                        rest[(size_t) i]    = z.schrittRest;
                        svfVon[(size_t) i]  = z.svfVon;
                        svfNach[(size_t) i] = z.svfNach;
                    }

                    int erste = -1;
                    for (int i = 0; i < messen && erste < 0; ++i)
                        if (std::memcmp (&tapD[(size_t) i], &tapR[(size_t) i], sizeof (double)) != 0)
                            erste = i;

                    // Der Entwurf faellt genau auf den Samples i == d (mod 8):
                    // danach steht `schrittRest` auf kDynamikSchritt - 1.
                    bool rasterStimmt = true;
                    for (int i = 0; i < messen; ++i)
                        if ((rest[(size_t) i] == kDynamikSchritt - 1)
                            != (i % kDynamikSchritt == dSoll % kDynamikSchritt))
                            rasterStimmt = false;

                    // Volle Wirkung: bei d + 8 ist `tSchritt` wieder 0, und
                    // `svfVon` traegt dort GENAU den Satz, der bei d entworfen
                    // wurde - der erste Entwurf wirkt mit Gewicht 1.
                    const int voll = dSoll + kDynamikSchritt;
                    const bool volleWirkung =
                           voll < messen
                        && rest[(size_t) voll] == kDynamikSchritt - 1
                        && std::memcmp (&svfVon[(size_t) voll], &svfNach[(size_t) dSoll],
                                        sizeof (SvfKoeffizienten)) == 0;

                    const bool traegt = erste == dSoll + 1 && rasterStimmt && volleWirkung;
                    allesTraegt = allesTraegt && traegt;
                    minErste = std::min (minErste, erste);
                    maxErste = std::max (maxErste, erste);
                    minVoll  = std::min (minVoll, voll);
                    maxVoll  = std::max (maxVoll, voll);

                    d << (pi > 0 ? "; " : "") << "p=" << p << " d=" << dSoll
                      << " erste Abweichung " << erste << " (soll " << (dSoll + 1) << ")"
                      << " volle Wirkung " << voll << (traegt ? "" : " TRAEGT NICHT");
                }

                d << "; erste Wirkung " << minErste << " bis " << maxErste << " Samples = "
                  << (1000.0 * (double) minErste / f.rate) << " bis "
                  << (1000.0 * (double) maxErste / f.rate) << " ms, volle Wirkung "
                  << minVoll << " bis " << maxVoll << " Samples = "
                  << (1000.0 * (double) minVoll / f.rate) << " bis "
                  << (1000.0 * (double) maxVoll / f.rate) << " ms";

                pruefe (allesTraegt && minErste == 1 && maxErste == kDynamikSchritt
                            && minVoll == kDynamikSchritt && maxVoll == 2 * kDynamikSchritt - 1,
                        std::string (f.zeile) + " " + f.was
                            + " (NAK-311 T3-15-11): der Tap bleibt bis einschliesslich Sample "
                              "d = (8 - p) mod 8 bitgleich zum Referenzkern, der Entwurf faellt auf d "
                              "und wirkt ab d + 8 mit Gewicht 1 - erste Wirkung 1 bis 8, volle 8 bis 15 Samples",
                        d.str());
            }
        }
    }

    //==========================================================================
    std::cout << std::endl << "== G - Kanalmodus, M/S, Trims, Mix (M-29 bis M-34) ==" << std::endl;
    {
        const double fs = 48000.0;

        // M-30: width 1,0 ist ein Unity-Kurzschluss - bitidentisch.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobal (s, "v1.global.width", 1.0);
            kern->uebernehmeZustand (s);
            fahreStille (*kern, 2048, 512);

            std::vector<float> a (512), b (512), aK, bK;
            bool bitgleich = true;
            for (int blk = 0; blk < 10; ++blk)
            {
                for (int i = 0; i < 512; ++i)
                { a[(size_t) i] = (float) (0.4 * std::sin (0.021 * (double) (blk * 512 + i)));
                  b[(size_t) i] = (float) (0.3 * std::cos (0.013 * (double) (blk * 512 + i))); }
                aK = a; bK = b;
                float* kan[2] = { a.data(), b.data() };
                kern->verarbeite (kan, 2, 512);
                for (int i = 0; i < 512; ++i)
                    if (std::memcmp (&a[(size_t) i], &aK[(size_t) i], sizeof (float)) != 0
                     || std::memcmp (&b[(size_t) i], &bK[(size_t) i], sizeof (float)) != 0)
                        bitgleich = false;
            }
            pruefe (bitgleich, "width_eins_ist_bitidentisch (M-30)",
                    "Ein- und Ausgang bitgleich ueber 10 Bloecke");
            // B-19: der Unity-Kurzschluss als EIGENE Zusage. Die Bitgleichheit
            // haelt in `double` auch ohne ihn; ob die Stufe laeuft, zeigt nur
            // ihr Zaehler.
            pruefe (kern->msStufenLaeufe() == 0, "width_eins_rechnet_die_ms_stufe_nicht (M-30, B-19)",
                    "M/S-Laeufe=" + std::to_string (kern->msStufenLaeufe()));
        }

        // B-19: die zweite Zusage - die M/S-Stufe selbst ist bei width 1,0
        // bitgenau reversibel, weil sie in `double` rechnet. Gemessen dort, wo
        // die Stufe bei width 1,0 WIRKLICH laeuft: am Ende einer Width-Rampe
        // von 1,5 auf 1,0. Ab dem 256. Rampensample ist w exakt 1,0, und die
        // Stufe rechnet den ganzen Block. Die Vorbedingung haelt fest, dass
        // `float`-Arithmetik an diesen Eingaengen NICHT bitgleich waere.
        {
            auto kern = neuerKern (fs, 512);
            auto breit = machSatz (true);
            setzeGlobal (breit, "v1.global.width", 1.5);
            kern->uebernehmeZustand (breit);
            fahreStille (*kern, kRampeSamples + 2048, 512);
            kern->pflege();
            kern->uebernehmeZustand (machSatz (true));
            const std::uint64_t laeufeVorher = kern->msStufenLaeufe();

            std::vector<float> a (512), b (512);
            for (int i = 0; i < 512; ++i)
            {
                a[(size_t) i] = (float) (0.1 + 0.37 * std::sin (0.013 * (double) i));
                b[(size_t) i] = (float) (0.3 * std::cos (0.029 * (double) i));
            }
            const std::vector<float> aK = a, bK = b;
            int floatWaereAnders = 0;
            for (int i = kRampeSamples - 1; i < 512; ++i)
            {
                const double m = (double) (aK[(size_t) i] + bK[(size_t) i]) * 0.5;
                const double s = ((double) aK[(size_t) i] - (double) bK[(size_t) i]) * 0.5;
                if ((float) (m + s) != aK[(size_t) i]) ++floatWaereAnders;
            }
            float* kan[2] = { a.data(), b.data() };
            kern->verarbeite (kan, 2, 512);
            bool bitgleich = true;
            for (int i = kRampeSamples - 1; i < 512; ++i)
                if (std::memcmp (&a[(size_t) i], &aK[(size_t) i], sizeof (float)) != 0
                 || std::memcmp (&b[(size_t) i], &bK[(size_t) i], sizeof (float)) != 0)
                    bitgleich = false;
            pruefe (kern->msStufenLaeufe() > laeufeVorher && floatWaereAnders > 0 && bitgleich,
                    "ms_stufe_ist_bei_width_eins_bitgenau_reversibel (M-30, B-19)",
                    "Stufe lief, " + std::to_string (floatWaereAnders)
                    + " Samples waeren in float anders, Samples ab Rampenende bitgleich");

            // NAK-311 M-90 (R-311-6): seit der Neutralpruefung schreibt der Kern
            // ab Sample 256 nicht mehr - der Ausgangsvergleich sieht dort nur
            // "unberuehrt". Die Rechnung selbst misst der Tap post_committed
            // ueber dasselbe Fenster: exakt der Eingang als double.
            int tapAnders = 0;
            for (int kanal = 0; kanal < 2; ++kanal)
            {
                const double* t = kern->tap (Tap::postCommitted, kanal);
                const auto& ein = kanal == 0 ? aK : bK;
                for (int i = kRampeSamples - 1; i < 512; ++i)
                {
                    const double soll = (double) ein[(size_t) i];
                    if (t == nullptr || std::memcmp (t + i, &soll, sizeof (double)) != 0) ++tapAnders;
                }
            }
            pruefe (tapAnders == 0,
                    "311/M-90 ms_stufe_rechnet_am_tap_bitgenau_reversibel (NAK-311 R-311-6, M-30, B-19)",
                    "Tap post_committed ueber die Samples 255 bis 511 beider Kanaele: " + std::to_string (tapAnders)
                    + " von 514 Stellen ungleich dem Eingang als double");
        }

        // M-30: width 0 ist mono.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobal (s, "v1.global.width", 0.0);
            kern->uebernehmeZustand (s);
            fahreStille (*kern, kRampeSamples + 2048, 512);

            std::vector<float> a (512), b (512);
            for (int i = 0; i < 512; ++i) { a[(size_t) i] = 0.6f; b[(size_t) i] = -0.2f; }
            float* kan[2] = { a.data(), b.data() };
            kern->verarbeite (kan, 2, 512);
            double maxDiff = 0.0;
            for (int i = 0; i < 512; ++i)
                maxDiff = std::max (maxDiff, std::abs ((double) a[(size_t) i] - (double) b[(size_t) i]));
            pruefe (maxDiff < 1e-6, "width_null_ist_mono (M-30)",
                    "groesste L/R-Differenz " + zahl (maxDiff, 9));
        }

        // M-31: Trims auf 0 dB sind Kurzschluesse (in M-02 mitgemessen);
        // hier die Kettenreihenfolge Input vor Filterbank.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobal (s, "v1.global.input_trim_db", 6.0);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);   // neutral
            kern->uebernehmeZustand (s);
            fahreStille (*kern, kRampeSamples + 2048, 512);

            std::vector<float> a (512, 0.25f), b (512, 0.25f);
            float* kan[2] = { a.data(), b.data() };
            kern->verarbeite (kan, 2, 512);
            const double erwartet = 0.25 * std::pow (10.0, 6.0 / 20.0);
            pruefe (std::abs ((double) a[500] - erwartet) < 1e-5,
                    "input_trim_wirkt_mit_seinem_gesamtgain (M-31)",
                    zahl ((double) a[500], 6) + " gegen " + zahl (erwartet, 6));
        }

        // B-16 (a): die REIHENFOLGE mit unterscheidbaren Wegen. Ein
        // dynamisches Band nahe Threshold: der Eingang liegt bei -33 dBFS und
        // damit unter dem Threshold -30; nur ein Input-Trim VOR der Bank hebt
        // ihn auf -27 dBFS, 3 dB darueber - die Auslenkung wird -3 dB. Hinter
        // der Bank liesse derselbe Trim den Gesamtgain gleich und die
        // Auslenkung bei 0.
        {
            const double tonHz = 1000.0;
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobal (s, "v1.global.input_trim_db", 6.0);
            belege (s, 0, Filtertyp::bell, tonHz, 1.0, 0.0);
            machDynamisch (s, 0, -12.0, -30.0, 500.0, 0.0, 500.0);
            kern->uebernehmeZustand (s);
            const double a = std::sqrt (2.0 * std::pow (10.0, -33.0 / 10.0));
            long long n0 = 0;
            fahreStereoTon (*kern, fs, tonHz, a, n0, (int) (6.0 * fs), 512);
            double w[param::kSlots];
            kern->auslenkungenDb (w);
            pruefe (std::abs (w[0] - (-3.0)) < 0.02,
                    "input_trim_liegt_vor_der_bank_der_detektor_hoert_ihn (M-31, B-16)",
                    "Eingang -33 dBFS, Trim +6 dB, Threshold -30: Auslenkung " + zahl (w[0], 4) + " dB (soll -3)");
        }

        // B-16 (b): der Output-Trim liegt HINTER Mix - bei Mix 0 wirkt er
        // trotzdem, auf den Dry-Zweig.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobal (s, "v2.global.mix", 0.0);
            setzeGlobal (s, "v1.global.output_trim_db", 6.0);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 12.0);
            kern->uebernehmeZustand (s);
            fahreStille (*kern, kRampeSamples + 4096, 512);
            std::vector<float> a (512, 0.25f), b (512, 0.25f);
            float* kan[2] = { a.data(), b.data() };
            kern->verarbeite (kan, 2, 512);
            const double erwartet = 0.25 * std::pow (10.0, 6.0 / 20.0);
            pruefe (std::abs ((double) a[500] - erwartet) < 1e-5,
                    "output_trim_liegt_hinter_mix (M-31, B-16)",
                    "Mix 0, Output-Trim +6 dB: " + zahl ((double) a[500], 6) + " gegen " + zahl (erwartet, 6));
        }

        // M-32/M-33/M-34: Mix.
        {
            // Mix 0 ist BITIDENTISCH zum Eingang - der Dry-Zweig ist der Tap
            // pre_nakama, also VOR Input-Trim und Filterbank.
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobal (s, "v2.global.mix", 0.0);
            setzeGlobal (s, "v1.global.input_trim_db", 9.0);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 12.0);
            kern->uebernehmeZustand (s);
            fahreStille (*kern, kRampeSamples + 4096, 512);

            std::vector<float> a (512), b (512), aK, bK;
            bool bitgleich = true;
            int tapAnders = 0;   // NAK-311 M-91 (R-311-6)
            for (int blk = 0; blk < 8; ++blk)
            {
                for (int i = 0; i < 512; ++i)
                { a[(size_t) i] = (float) (0.4 * std::sin (0.019 * (double) (blk * 512 + i)));
                  b[(size_t) i] = (float) (0.4 * std::cos (0.023 * (double) (blk * 512 + i))); }
                aK = a; bK = b;
                float* kan[2] = { a.data(), b.data() };
                kern->verarbeite (kan, 2, 512);
                for (int i = 0; i < 512; ++i)
                    if (std::memcmp (&a[(size_t) i], &aK[(size_t) i], sizeof (float)) != 0
                     || std::memcmp (&b[(size_t) i], &bK[(size_t) i], sizeof (float)) != 0)
                        bitgleich = false;
                for (int kanal = 0; kanal < 2; ++kanal)
                {
                    const double* t = kern->tap (Tap::postCommitted, kanal);
                    const auto& ein = kanal == 0 ? aK : bK;
                    for (int i = 0; i < 512; ++i)
                    {
                        const double soll = (double) ein[(size_t) i];
                        if (t == nullptr || std::memcmp (t + i, &soll, sizeof (double)) != 0) ++tapAnders;
                    }
                }
            }
            pruefe (bitgleich, "mix_null_ist_bitidentisch (M-33)",
                    "trotz +9 dB Input-Trim und +12-dB-Bell");
            // NAK-311 M-91 (R-311-6): im M-33-Zustand schreibt der Kern nicht
            // mehr; die Rechnung - Dry ist pre_nakama, vor Input-Trim und
            // Filterbank - misst der Tap post_committed ueber dieselben Bloecke.
            pruefe (tapAnders == 0,
                    "311/M-91 mix_null_traegt_am_tap_den_eingang (NAK-311 R-311-6, M-33)",
                    "Tap post_committed ueber 8 Bloecke zu 512 beider Kanaele: " + std::to_string (tapAnders)
                    + " von 8192 Stellen ungleich dem Eingang als double");
        }

        {
            // M-32: bei Mix 1,0 wird der Dry-Zweig NICHT gerechnet. Der
            // Beweis: ein nicht-endlicher Wert im Dry-Zweig schlaegt nicht
            // durch - er wuerde es, wenn `1*wet + 0*dry` gerechnet wuerde,
            // denn `0 * NaN` ist NaN.
            auto mitEins = neuerKern (fs, 256);
            auto sHalb = machSatz (true);
            belege (sHalb, 0, Filtertyp::bell, 1000.0, 1.0, 6.0);
            setzeGlobal (sHalb, "v2.global.mix", 0.5);
            auto sEins = sHalb;
            setzeGlobal (sEins, "v2.global.mix", 1.0);

            // Erst 0,5, dann zurueck auf 1,0: nur so laeuft die Rampe
            // ueberhaupt, und nur dann ist die Frage "wird der Dry-Zweig
            // gerechnet" gestellt. Steht sie von Anfang an auf 1,0, ruht sie,
            // und der ganze Mix-Block wird uebersprungen.
            mitEins->uebernehmeZustand (sHalb);
            fahreStille (*mitEins, kRampeSamples + 1024, 256);
            mitEins->pflege();
            mitEins->uebernehmeZustand (sEins);
            // Gemessen wird bei RUHENDER Rampe auf 1,0 - das ist der
            // Zustand, den M-32 zusagt. Waehrend die Rampe laeuft, ist der
            // Mix kleiner als 1, und dann MUSS der Dry-Zweig gerechnet
            // werden; er traegt dort den rohen Eingang, wie M-33 es fuer
            // Mix 0 ausdruecklich verlangt.
            fahreStille (*mitEins, kRampeSamples + 2048, 256);
            std::vector<float> a (256, 0.2f), b (256, 0.2f);
            a[100] = std::numeric_limits<float>::quiet_NaN();
            float* kan[2] = { a.data(), b.data() };
            mitEins->verarbeite (kan, 2, 256);
            bool endlich = true;
            for (int i = 0; i < 256; ++i) if (! std::isfinite (a[(size_t) i])) endlich = false;
            pruefe (endlich, "mix_eins_umgeht_den_dry_zweig (M-32)",
                    "der NaN wird im Wet-Zweig verriegelt und kommt nicht ueber den Dry-Zweig zurueck");
        }

        {
            // Mix 0,5 ist LINEAR, nicht equal-power: bei identischem Dry und
            // Wet bleibt der Ausgang gleich laut, nicht 3 dB lauter.
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobal (s, "v2.global.mix", 0.5);
            kern->uebernehmeZustand (s);   // kein Band: Wet == Dry
            fahreStille (*kern, kRampeSamples + 4096, 512);

            std::vector<float> a (512, 0.5f), b (512, 0.5f);
            float* kan[2] = { a.data(), b.data() };
            kern->verarbeite (kan, 2, 512);
            pruefe (std::abs ((double) a[500] - 0.5) < 1e-6,
                    "mix_ist_linear_nicht_equal_power (M-34)",
                    "0,5*wet + 0,5*dry bei gleichem Signal = " + zahl ((double) a[500], 8));
        }

        // M-29: `channel_mode`-Wechsel bei laufendem Audio ist klickfrei.
        {
            auto kern = neuerKern (fs, 256);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 9.0, Kanalmodus::stereo);
            kern->uebernehmeZustand (s);
            fahreStille (*kern, 2048, 256);
            kern->pflege();

            auto gewechselt = s;
            gewechselt.werte[(size_t) param::indexBandV1 (0, param::kChannelMode)].enumIndex
                = (int) Kanalmodus::mid;

            std::vector<float> a (256), b (256);
            float* kan[2] = { a.data(), b.data() };
            // Sinus auf der Bandmitte mit ungleichem L und R: erst dann
            // aendert der Wechsel stereo -> mid die Kette wirklich. Massstab
            // ist wie bei M-25 die zweite Differenz.
            auto fuelle = [&] (long long v) {
                for (int i = 0; i < 256; ++i)
                { const double x = 0.5 * std::sin (2.0 * kPiRef * 1000.0 * (double) (v + i) / fs);
                  a[(size_t) i] = (float) x; b[(size_t) i] = (float) (x * 0.3); } };

            std::vector<double> lauf;
            for (int blk = 0; blk < 8; ++blk)
            { fuelle ((long long) blk * 256); kern->verarbeite (kan, 2, 256);
              for (int i = 0; i < 256; ++i) lauf.push_back ((double) a[(size_t) i]); }
            const size_t vorWechsel = lauf.size();
            kern->uebernehmeZustand (gewechselt);
            for (int blk = 8; blk < 10; ++blk)
            { fuelle ((long long) blk * 256); kern->verarbeite (kan, 2, 256);
              for (int i = 0; i < 256; ++i) lauf.push_back ((double) a[(size_t) i]); }

            auto maxZweiteDifferenz = [&] (size_t von, size_t bis)
            {
                double m = 0.0;
                for (size_t i = von; i + 1 < bis && i >= 2; ++i)
                    m = std::max (m, std::abs (lauf[i] - 2.0 * lauf[i - 1] + lauf[i - 2]));
                return m;
            };
            const double ruhig = maxZweiteDifferenz (4, vorWechsel);
            const double amWechsel = maxZweiteDifferenz (vorWechsel - 2, lauf.size());
            pruefe (amWechsel <= 4.0 * ruhig + 1e-12,
                    "channel_mode_wechselt_am_blockrand_klickfrei (M-29)",
                    "zweite Differenz am Wechsel " + zahl (amWechsel, 9)
                    + " gegen " + zahl (ruhig, 9) + " im ruhigen Lauf");
        }
    }

    //==========================================================================
    std::cout << std::endl << "== H - Auto-Gain (M-35 bis M-40) ==" << std::endl;
    {
        const double fs = 48000.0;

        // M-36: flache Kurve ergibt EXAKT 0,0 dB.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobalBool (s, "v2.global.auto_gain", true);
            kern->uebernehmeZustand (s);
            pruefe (kern->autoGainDb() == 0.0 && ! std::signbit (kern->autoGainDb()),
                    "flache_kurve_ergibt_exakt_null_db (M-36)",
                    "bitgenau +0,0, nicht -0,0 aus einem Gitterlauf: " + zahl (kern->autoGainDb(), 15));

            // Auch mit einem belegten, aber neutralen Bell.
            auto mitBand = s;
            belege (mitBand, 2, Filtertyp::bell, 1000.0, 1.0, 0.0);
            auto k2 = neuerKern (fs, 512);
            k2->uebernehmeZustand (mitBand);
            pruefe (k2->autoGainDb() == 0.0 && ! std::signbit (k2->autoGainDb()),
                    "neutraler_bell_ergibt_exakt_null_db (M-36)",
                    "bitgenau +0,0: " + zahl (k2->autoGainDb(), 15));
        }

        // M-37: ein +6-dB-High-Shelf ueber dem ganzen Gitter ergibt -6 dB.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobalBool (s, "v2.global.auto_gain", true);
            // Der Pruefling steht an der unteren Vertragsgrenze (20 Hz, die
            // kleinste zulaessige `freq_hz`) mit Q 1,0. Erst dieser Q laesst
            // das Plateau das Gitter praktisch vollstaendig ueberstreichen:
            // mit Q 0,707 liegen die ersten rund 16 % der Gitterstellen noch
            // im Uebergang, und der Energiemittelwert landet bei -5,86 dB.
            // Das ist keine Toleranzfrage, sondern die Frage, ob der
            // Pruefling die Zusage "ueberstreicht das ganze Gitter"
            // ueberhaupt erfuellt.
            belege (s, 0, Filtertyp::highShelf, 20.0, 1.0, 6.0);
            kern->uebernehmeZustand (s);
            pruefe (std::abs (kern->autoGainDb() + 6.0) <= 0.1,
                    "shelf_plus_sechs_ergibt_minus_sechs (M-37)",
                    zahl (kern->autoGainDb(), 4) + " dB");
        }

        // M-37, B-04-Regel: ein IDENTISCHES Mid/Side-Paar faellt exakt auf
        // den Stereo-Fall zusammen. Die vorige Faltung haette es wie +12 dB
        // kompensiert.
        {
            // Gemessen wird gegen den STEREO-Fall selbst, nicht gegen -6 dB:
            // die Zusage lautet "faellt exakt auf den Stereo-Fall zusammen",
            // und die haelt fuer JEDEN Pruefling. Die vorige Faltung
            // multiplizierte H_M und H_S in beide Seiten und haette das Paar
            // wie +12 dB kompensiert (MN1, B-04).
            auto stereo = neuerKern (fs, 512);
            auto sStereo = machSatz (true);
            setzeGlobalBool (sStereo, "v2.global.auto_gain", true);
            belege (sStereo, 0, Filtertyp::highShelf, 20.0, 0.707, 6.0, Kanalmodus::stereo);
            stereo->uebernehmeZustand (sStereo);

            auto paar = neuerKern (fs, 512);
            auto sPaar = machSatz (true);
            setzeGlobalBool (sPaar, "v2.global.auto_gain", true);
            belege (sPaar, 0, Filtertyp::highShelf, 20.0, 0.707, 6.0, Kanalmodus::mid);
            belege (sPaar, 1, Filtertyp::highShelf, 20.0, 0.707, 6.0, Kanalmodus::side);
            paar->uebernehmeZustand (sPaar);

            const double aStereo = stereo->autoGainDb();
            const double aPaar   = paar->autoGainDb();
            pruefe (std::abs (aStereo - aPaar) < 1e-9,
                    "identisches_mid_side_paar_faellt_auf_stereo_zusammen (M-37)",
                    "stereo " + zahl (aStereo, 6) + " dB, mid+side " + zahl (aPaar, 6) + " dB");
            pruefe (std::abs (aPaar - 2.0 * aStereo) > 1.0,
                    "und es wird NICHT wie der doppelte Gain kompensiert (M-37)",
                    "das Doppelte waere " + zahl (2.0 * aStereo, 4) + " dB");
        }

        // M-35: aus wirkt nicht, ist aber trotzdem lesbar.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::highShelf, 20.0, 1.0, 6.0);
            setzeGlobalBool (s, "v2.global.auto_gain", false);
            kern->uebernehmeZustand (s);
            pruefe (std::abs (kern->autoGainDb() + 6.0) <= 0.1,
                    "auto_gain_aus_wirkt_nicht_und_ist_trotzdem_lesbar (M-35)",
                    "abgeleitet " + zahl (kern->autoGainDb(), 4) + " dB");

            // Und der Ausgang traegt den Ausgleich NICHT.
            fahreStille (*kern, kRampeSamples + 4096, 512);
            std::vector<float> a (512, 0.0f), b (512, 0.0f);
            a[0] = 1.0f; b[0] = 1.0f;
            float* kan[2] = { a.data(), b.data() };
            kern->verarbeite (kan, 2, 512);
            // Ein High-Shelf mit Eckfrequenz 20 Hz hebt praktisch alles um
            // 6 dB: der erste Sample der Impulsantwort liegt deutlich ueber 1.
            pruefe ((double) a[0] > 1.5, "der_ausgleich_wirkt_bei_aus_nicht (M-35)",
                    "erster IR-Sample " + zahl ((double) a[0], 5));
        }

        // M-38: der dynamische Anteil geht NICHT in die Ableitung ein.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobalBool (s, "v2.global.auto_gain", true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (s, 0, -12.0, -60.0, 1.0, 0.0, 5.0);
            kern->uebernehmeZustand (s);
            const double vorher = kern->autoGainDb();

            std::vector<float> a (512), b (512);
            float* kan[2] = { a.data(), b.data() };
            for (int blk = 0; blk < 100; ++blk)
            {
                for (int i = 0; i < 512; ++i)
                { const double x = 0.95 * std::sin (2.0 * kPiRef * 1000.0 * (double) (blk * 512 + i) / fs);
                  a[(size_t) i] = (float) x; b[(size_t) i] = (float) x; }
                kern->verarbeite (kan, 2, 512);
            }
            pruefe (kern->autoGainDb() == vorher, "dynamik_bewegt_den_auto_gain_nicht (M-38)",
                    "vorher " + zahl (vorher, 9) + ", nachher " + zahl (kern->autoGainDb(), 9));

            double werte[param::kSlots];
            kern->auslenkungenDb (werte);
            pruefe (std::abs (werte[0]) > 1.0, "und die Dynamik hat sich wirklich bewegt (M-38)",
                    "Auslenkung " + zahl (werte[0], 4) + " dB");
        }

        // M-39: der Ausgleich wird NICHT im Audiothread gerechnet - er steht
        // schon fest, bevor ein einziger Block gelaufen ist.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobalBool (s, "v2.global.auto_gain", true);
            belege (s, 0, Filtertyp::highShelf, 20.0, 1.0, 6.0);
            kern->uebernehmeZustand (s);
            const double sofort = kern->autoGainDb();
            fahreStille (*kern, 4096, 512);
            pruefe (sofort == kern->autoGainDb() && std::abs (sofort + 6.0) <= 0.1,
                    "auto_gain_steht_vor_dem_ersten_block_fest (M-39)",
                    "vor dem ersten Block bereits " + zahl (sofort, 4) + " dB");
        }

        // B-18 / M-39: der RECHENORT, threadbezogen gezaehlt. Die Ableitung
        // meldet sich bei jeder Rechnung; im Audiopfad (RtWache-Bereich) steigt
        // ein eigener Zaehler. Gefahren werden zwei Programmwechsel samt
        // Blockrandubernahme, Fade und Pflege.
        {
            auto kern = neuerKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobalBool (s, "v2.global.auto_gain", true);
            belege (s, 0, Filtertyp::highShelf, 20.0, 1.0, 6.0);
            RtWache::zuruecksetzen();
            kern->uebernehmeZustand (s);
            fahreStille (*kern, 4096, 512);
            kern->pflege();
            auto s2 = s;
            belege (s2, 1, Filtertyp::bell, 2000.0, 2.0, -4.0);
            kern->uebernehmeZustand (s2);
            fahreStille (*kern, 4096, 512);
            kern->pflege();

            // NAK-311 311/M-66 (R-311-3): derselbe Zaehler am MONOKERN. Der
            // Monozweig rechnet dieselben 121 Gitterstellen, nur eine andere
            // Formel - er darf den Rechenort nicht verschieben. Gefahren wird
            // wieder ein Programmwechsel samt Blockrandubernahme und Pflege,
            // diesmal ueber den einen Kanal des Monobusses.
            auto mono = std::make_unique<DspKern>();
            mono->bereiteVor (fs, 512, 1);
            auto monoStille = [] (DspKern& k, int samples, int blockGroesse)
            {
                std::vector<float> a ((size_t) blockGroesse, 0.0f);
                float* kan[1] = { a.data() };
                for (int rest = samples; rest > 0;)
                {
                    const int m = rest < blockGroesse ? rest : blockGroesse;
                    std::fill (a.begin(), a.begin() + m, 0.0f);
                    k.verarbeite (kan, 1, m);
                    rest -= m;
                }
            };
            mono->uebernehmeZustand (s);
            monoStille (*mono, 4096, 512);
            mono->pflege();
            mono->uebernehmeZustand (s2);
            monoStille (*mono, 4096, 512);
            mono->pflege();

            pruefe (RtWache::ableitungenAusserhalb() >= 4 && RtWache::ableitungenImAudiopfad() == 0,
                    "auto_gain_wird_nicht_im_audiothread_gerechnet (M-39, B-18; NAK-311 311/M-66 Monokern)",
                    "Ableitungen ausserhalb " + std::to_string (RtWache::ableitungenAusserhalb())
                    + ", im Audiopfad " + std::to_string (RtWache::ableitungenImAudiopfad())
                    + " (Stereo- und Monokern zusammen)");
        }

        // M-40: Auto-Gain ersetzt den Output-Trim nicht - beide wirken.
        {
            auto ohne = neuerKern (fs, 512);
            auto mit  = neuerKern (fs, 512);
            auto s1 = machSatz (true), s2 = machSatz (true);
            setzeGlobalBool (s1, "v2.global.auto_gain", true);
            setzeGlobalBool (s2, "v2.global.auto_gain", true);
            belege (s1, 0, Filtertyp::highShelf, 20.0, 1.0, 6.0);
            belege (s2, 0, Filtertyp::highShelf, 20.0, 1.0, 6.0);
            setzeGlobal (s2, "v1.global.output_trim_db", 6.0);
            ohne->uebernehmeZustand (s1);
            mit ->uebernehmeZustand (s2);

            auto dcAus = [&] (DspKern& k)
            {
                fahreStille (k, kRampeSamples + 8192, 512);
                std::vector<float> a (512, 0.25f), b (512, 0.25f);
                float* kan[2] = { a.data(), b.data() };
                k.verarbeite (kan, 2, 512);
                return (double) a[500];
            };
            const double x1 = dcAus (*ohne), x2 = dcAus (*mit);
            const double verhaeltnis = x2 / x1;
            pruefe (std::abs (verhaeltnis - std::pow (10.0, 6.0 / 20.0)) < 0.01,
                    "auto_gain_und_output_trim_addieren_sich (M-40)",
                    "Verhaeltnis " + zahl (verhaeltnis, 5) + " gegen 1,99526");
        }
    }

    //==========================================================================
    // NAK-311 Etappe 4 Teil a (T3-16-04, R-311-3): Auto-Gain im Monobus.
    // Matrixzeilen 311/M-57 bis M-61, M-64 und M-65 (Manifest NAK-311 §6.4).
    std::cout << std::endl << "== H2 - Auto-Gain im Monobus (NAK-311 R-311-3) ==" << std::endl;
    {
        const double fs = 48000.0;

        // ---- 311/M-61: die Zweikanalformel bleibt bitgleich zum Basisstand --
        // Regressionswache. Die Hexwerte hat der UNVERAENDERTE Kern am
        // Basis-SHA der Etappe 4 ausgegeben (Golden-Regel §7.1: Goldens
        // entstehen nur ueber ihren Erzeuger, nie ueber den geaenderten Kern);
        // Rohbeleg `docs/beweise/roh/NAK-311-etappe4-autogain-hex.txt`.
        // Geprueft wird mit `memcmp` - bitgleich, nicht "nahe".
        const char* const m61Namen[] = {
            "H_flache_kurve", "H_neutraler_bell", "H_shelf_plus_sechs", "H_stereo_q0707",
            "H_mid_side_paar", "H_dynamisches_bell", "H_auto_gain_aus", "H_mit_output_trim",
            "modus_stereo", "modus_left", "modus_right", "modus_mid", "modus_side",
            "M47_gemischtes_programm" };
        constexpr int kM61Zahl = 14;

        auto m61Pruefling = [] (int i)
        {
            auto s = machSatz (true);
            setzeGlobalBool (s, "v2.global.auto_gain", true);
            switch (i)
            {
                case 0: break;                                                      // flache Kurve (M-36)
                case 1: belege (s, 2, Filtertyp::bell, 1000.0, 1.0, 0.0); break;    // neutraler Bell (M-36)
                case 2: belege (s, 0, Filtertyp::highShelf, 20.0, 1.0, 6.0); break; // M-37
                case 3: belege (s, 0, Filtertyp::highShelf, 20.0, 0.707, 6.0, Kanalmodus::stereo); break;
                case 4: belege (s, 0, Filtertyp::highShelf, 20.0, 0.707, 6.0, Kanalmodus::mid);
                        belege (s, 1, Filtertyp::highShelf, 20.0, 0.707, 6.0, Kanalmodus::side); break;
                case 5: belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 0.0);
                        machDynamisch (s, 0, -12.0, -60.0, 1.0, 0.0, 5.0); break;   // M-38
                case 6: belege (s, 0, Filtertyp::highShelf, 20.0, 1.0, 6.0);
                        setzeGlobalBool (s, "v2.global.auto_gain", false); break;   // M-35
                case 7: belege (s, 0, Filtertyp::highShelf, 20.0, 1.0, 6.0);
                        setzeGlobal (s, "v1.global.output_trim_db", 6.0); break;    // M-40
                case 8:  belege (s, 0, Filtertyp::lowShelf, 1000.0, 1.0, 6.0, Kanalmodus::stereo); break;
                case 9:  belege (s, 0, Filtertyp::lowShelf, 1000.0, 1.0, 6.0, Kanalmodus::left);   break;
                case 10: belege (s, 0, Filtertyp::lowShelf, 1000.0, 1.0, 6.0, Kanalmodus::right);  break;
                case 11: belege (s, 0, Filtertyp::lowShelf, 1000.0, 1.0, 6.0, Kanalmodus::mid);    break;
                case 12: belege (s, 0, Filtertyp::lowShelf, 1000.0, 1.0, 6.0, Kanalmodus::side);   break;
                default:                                                            // der M-47-Lauf
                    belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 6.0, Kanalmodus::mid);
                    belege (s, 3, Filtertyp::lowShelf, 200.0, 0.707, -4.0);
                    machDynamisch (s, 3, -6.0, -30.0);
                    setzeGlobal (s, "v1.global.width", 1.4);
                    setzeGlobal (s, "v1.global.mono_bass_hz", 120.0);
                    setzeGlobal (s, "v2.global.mix", 0.7);
                    break;
            }
            return s;
        };

        auto bitsVon = [] (double d)
        {
            std::uint64_t b = 0;
            std::memcpy (&b, &d, sizeof (b));
            return b;
        };

        auto hex64 = [] (std::uint64_t b)
        {
            std::ostringstream o;
            o << "0x" << std::hex << std::uppercase << std::setw (16) << std::setfill ('0') << b;
            return o.str();
        };

        // Die vierzehn Bitmuster des Basisstands `df14a648`, Lauf vom
        // 20.09.2026 (Rohdatei oben).
        const std::uint64_t kM61Bits[kM61Zahl] = {
            0x0000000000000000ull, 0x0000000000000000ull, 0xC017FCF4DCC41CCBull, 0xC017714584703D0Eull,
            0xC017714584703D0Eull, 0x0000000000000000ull, 0xC017FCF4DCC41CCBull, 0xC017FCF4DCC41CCBull,
            0xC0113FE32BC1B7DAull, 0xC0055DD001A5F001ull, 0xC0055DD001A5F001ull, 0xC0022A8D8F930CABull,
            0xC0022A8D8F930CABull, 0x3FCD799DE6A48DB0ull };

        int m61Gleich = 0, m61Gedeckelt = 0, m61UeberDemDeckel = 0, m61NichtBitgleich = 0;
        double m61Kleinster = 0.0, m61Groesster = 0.0;
        std::string m61Erste;
        for (int i = 0; i < kM61Zahl; ++i)
        {
            auto k = neuerKern (fs, 512);            // Kanalzahl 2 (Vorgabe von bereiteVor)
            k->uebernehmeZustand (m61Pruefling (i));
            const double wert = k->autoGainDb();
            const std::uint64_t bits = bitsVon (wert);
            std::cout << "  hexgolden  311/M-61 " << m61Namen[i] << "  " << hex64 (bits)
                      << "  (" << zahl (wert, 12) << " dB)" << std::endl;
            if (std::memcmp (&bits, &kM61Bits[i], sizeof (std::uint64_t)) == 0) ++m61Gleich;
            else if (m61Erste.empty())
                m61Erste = std::string (m61Namen[i]) + ": " + hex64 (bits) + " statt " + hex64 (kM61Bits[i]);

            // NAK-311 Etappe 5, Aenderungssatz B - 311/M-119 (Regressionswache):
            // der einseitige Deckel beruehrt KEINEN dieser vierzehn Prueflinge.
            // Je Pruefling gemessen, nicht nur behauptet: sein ABGELEITETER Wert
            // liegt unter `kAutoGainDeckelDb`, der Deckel laesst ihn bitgleich
            // durch, und der Zustand bleibt falsch.
            const double roh = k->autoGainRohDb();
            if (i == 0 || roh < m61Kleinster) m61Kleinster = roh;
            if (i == 0 || roh > m61Groesster) m61Groesster = roh;
            if (roh > kAutoGainDeckelDb) ++m61UeberDemDeckel;
            if (bitsVon (gedeckelterAutoGainDb (roh)) != bitsVon (roh)) ++m61NichtBitgleich;
            if (k->autoGainGedeckelt()) ++m61Gedeckelt;
        }
        pruefe (m61Gleich == kM61Zahl, "311/M-61 zweikanalformel_bitgleich_zum_basisstand (R-311-3)",
                m61Erste.empty() ? (std::to_string (m61Gleich) + " von " + std::to_string (kM61Zahl)
                                    + " Prueflingen bitgleich (memcmp)")
                                 : ("erste Abweichung " + m61Erste));
        pruefe (m61UeberDemDeckel == 0 && m61NichtBitgleich == 0 && m61Gedeckelt == 0
                    && m61Gleich == kM61Zahl,
                "311/M-119 alle_vierzehn_goldens_liegen_auf_der_ungedeckelten_seite (NAK-311 R-311-14, Gate GRENZE)",
                std::to_string (kM61Zahl) + " Prueflinge von " + zahl (m61Kleinster, 12) + " bis "
                    + zahl (m61Groesster, 12) + " dB, Deckel " + zahl (kAutoGainDeckelDb, 1)
                    + " dB; darueber " + std::to_string (m61UeberDemDeckel) + ", nicht bitgleich durch den Deckel "
                    + std::to_string (m61NichtBitgleich) + ", Zustand gesetzt " + std::to_string (m61Gedeckelt));

        // ---- Die Monoformel, hier EIGENSTAENDIG ausgeschrieben ---------------
        // Muster M-13 von SONDE-015: kein Aufruf von `leiteAutoGainAb` als
        // Orakel. Je Gitterstelle startet das Paar (a_L, a_R) = (1, 1) - der
        // Monoeingang, den der Kern in beide Komponenten legt
        // (`DspKern::verarbeiteStueck`). Die aktiven Baender wirken in
        // SLOTREIHENFOLGE mit ihrer statischen Ruheantwort aus `refEntwurf`
        // (der unabhaengig ausgeschriebenen RBJ-Formel dieses Tests); `mid`
        // und `side` gehen ueber dieselbe Rueckfuehrung in L und R wie
        // `DspKern::verarbeiteBand`. Ausgewertet wird nur a_L - der Kanal,
        // den der Kern im Monobus schreibt. Dann pink-gewichtetes Mittel
        // ueber die 121 Gitterstellen und -10*log10.
        struct MonoBand { Filtertyp typ; double f, q, gain; Kanalmodus modus; };

        auto monoReferenzDb = [] (const std::vector<MonoBand>& baender, double rate)
        {
            if (baender.empty()) return 0.0;
            double summe = 0.0;
            for (int i = 0; i < kAutoGainStellen; ++i)
            {
                const double w = 2.0 * kPiRef * autoGainGitterHz (i) / rate;
                const std::complex<double> z (std::cos (-w), std::sin (-w));
                const std::complex<double> z2 = z * z;
                std::complex<double> aL { 1.0, 0.0 }, aR { 1.0, 0.0 };
                for (const auto& b : baender)
                {
                    const auto r = refEntwurf (b.typ, rate, b.f, b.q, b.gain);
                    const auto h = (r.b0 + r.b1 * z + r.b2 * z2) / (1.0 + r.a1 * z + r.a2 * z2);
                    switch (b.modus)
                    {
                        case Kanalmodus::stereo: aL *= h; aR *= h; break;
                        case Kanalmodus::left:   aL *= h; break;
                        case Kanalmodus::right:  aR *= h; break;
                        case Kanalmodus::mid:
                        {
                            const auto m = (aL + aR) * 0.5, s = (aL - aR) * 0.5;
                            aL = m * h + s; aR = m * h - s; break;
                        }
                        case Kanalmodus::side:
                        {
                            const auto m = (aL + aR) * 0.5, s = (aL - aR) * 0.5;
                            aL = m + s * h; aR = m - s * h; break;
                        }
                    }
                }
                summe += std::norm (aL);
            }
            const double mittel = summe / (double) kAutoGainStellen;
            if (! (mittel > 0.0) || ! std::isfinite (mittel)) return 0.0;
            return -10.0 * std::log10 (mittel);
        };

        /** Ein Kern auf einem MONObus: `bereiteVor` mit Kanalzahl 1.
            NAK-175: auf den Heap. */
        auto neuerMonoKern = [] (double rate, int maxBlock, int kanaele = 1)
        {
            auto k = std::make_unique<DspKern>();
            k->bereiteVor (rate, maxBlock, kanaele);
            return k;
        };

        /** Faehrt einen Sinus ueber den EINEN Kanal des Monobusses und gibt
            den float-Ausgang zurueck - derselbe Aufruf, den der Prozessor im
            Monobus macht (`verarbeite (kanaele, 1, n)`). */
        auto fahreMonoTon = [] (DspKern& k, double rate, double f, double amplitude,
                                int samples, int blockGroesse)
        {
            std::vector<float> a ((size_t) blockGroesse, 0.0f);
            float* kan[1] = { a.data() };
            std::vector<float> aus;
            aus.reserve ((size_t) samples);
            const double w = 2.0 * kPiRef * f / rate;
            long long n0 = 0;
            for (int rest = samples; rest > 0;)
            {
                const int m = rest < blockGroesse ? rest : blockGroesse;
                for (int i = 0; i < m; ++i)
                    a[(size_t) i] = (float) (amplitude * std::sin (w * (double) (n0 + i)));
                k.verarbeite (kan, 1, m);
                for (int i = 0; i < m; ++i) aus.push_back (a[(size_t) i]);
                n0 += m;
                rest -= m;
            }
            return aus;
        };

        /** Der Monopruefling der Phase 16: ein Low-Shelf 20 kHz / +12 dB /
            Q 0,707 im gegebenen Kanalmodus, Auto-Gain nach Wunsch. */
        auto monoSatz = [] (Kanalmodus modus, bool autoGainAn)
        {
            auto s = machSatz (true);
            setzeGlobalBool (s, "v2.global.auto_gain", autoGainAn);
            belege (s, 0, Filtertyp::lowShelf, 20000.0, 0.707, 12.0, modus);
            return s;
        };

        // ---- 311/M-57 und M-58: `right` und `side` gleichen NICHTS aus ------
        // Heute rot: Phase 16 mass am echten Kern -9,177564 dB (`right`) und
        // -7,839905 dB (`side`) - eine hoerbare Absenkung eines Weges, an dem
        // das Band gar nicht wirkt. Die Abnahme vom 24.08.2026 sagt "den
        // Pegelgewinn der aktuellen Kurve ... am Ausgang ab": im Monobus
        // schreibt der Kern nur Kanal 0, und dort aendert weder ein
        // `right`- noch ein `side`-Band etwas.
        for (const auto modus : { Kanalmodus::right, Kanalmodus::side })
        {
            const bool istRight = (modus == Kanalmodus::right);
            const std::string name = istRight ? "311/M-57 mono_right_band_ergibt_null_db"
                                              : "311/M-58 mono_side_band_ergibt_null_db (Teilfall von 311/M-57)";

            auto mit  = neuerMonoKern (fs, 512);
            mit->uebernehmeZustand (monoSatz (modus, true));
            const double wert = mit->autoGainDb();
            pruefe (wert == 0.0 && ! std::signbit (wert), name + ", Wert",
                    "exakt +0,0 (kein -0,0): " + zahl (wert, 15)
                        + "; Phase 16 mass " + (istRight ? "-9,177564" : "-7,839905") + " dB");

            auto ohne = neuerMonoKern (fs, 512);
            ohne->uebernehmeZustand (monoSatz (modus, false));
            const auto aMit  = fahreMonoTon (*mit,  fs, 250.0, 0.25, 48000, 512);
            const auto aOhne = fahreMonoTon (*ohne, fs, 250.0, 0.25, 48000, 512);
            const bool bitgleich = aMit.size() == aOhne.size()
                                && std::memcmp (aMit.data(), aOhne.data(), aMit.size() * sizeof (float)) == 0;
            pruefe (bitgleich, name + ", Ausgang",
                    "48 000 Samples 250-Hz-Sinus 0,25 im Monobus: mit Auto-Gain bytegleich zum Lauf ohne");
        }

        // ---- 311/M-60: `left` und `mid` kompensieren VOLL -------------------
        // Heute rot: die Zweikanalformel rechnet fuer `left` das Mittel
        // 0,5*(|H|^2 + 1) und fuer `mid` den Diagonalterm. Im Monobus wirken
        // beide auf den ausgegebenen Kanal wie ein `stereo`-Band - die Zeile
        // nennt den Kern im Modus `stereo` ausdruecklich als Orakel.
        {
            auto stereoModus = neuerMonoKern (fs, 512);
            auto s = machSatz (true);
            setzeGlobalBool (s, "v2.global.auto_gain", true);
            belege (s, 0, Filtertyp::lowShelf, 1000.0, 1.0, 6.0, Kanalmodus::stereo);
            stereoModus->uebernehmeZustand (s);
            const double soll = stereoModus->autoGainDb();

            for (const auto modus : { Kanalmodus::left, Kanalmodus::mid })
            {
                auto k = neuerMonoKern (fs, 512);
                auto sm = machSatz (true);
                setzeGlobalBool (sm, "v2.global.auto_gain", true);
                belege (sm, 0, Filtertyp::lowShelf, 1000.0, 1.0, 6.0, modus);
                k->uebernehmeZustand (sm);
                const double ist = k->autoGainDb();
                const double ref = monoReferenzDb ({ { Filtertyp::lowShelf, 1000.0, 1.0, 6.0, modus } }, fs);
                pruefe (std::abs (ist - soll) <= 1e-9 && std::abs (ist - ref) <= 1e-9,
                        std::string ("311/M-60 mono_") + (modus == Kanalmodus::left ? "left" : "mid")
                            + "_kompensiert_voll (Teilfall von 311/M-57)",
                        zahl (ist, 12) + " dB gegen Modus stereo " + zahl (soll, 12)
                            + " dB und gegen die Testreferenz " + zahl (ref, 12) + " dB");
            }
        }

        // ---- 311/M-59: die Kaskade laeuft in SLOTREIHENFOLGE ----------------
        // Heute rot (heute Zweikanalformel). Der Kreuzweg ist der Punkt: ein
        // `right`-Band wirkt im Monobus nicht fuer sich, aber eine spaetere
        // `mid`-Stufe mischt den gedachten rechten Kanal wieder nach links -
        // deshalb zaehlt die Reihenfolge, und deshalb faltet die Monoformel
        // nicht, sondern rechnet die Kaskade (§9 F-11).
        {
            const MonoBand shelfRechts { Filtertyp::highShelf, 5000.0, 0.707, 6.0, Kanalmodus::right };
            const MonoBand bellMid     { Filtertyp::bell,      1000.0, 1.0,   6.0, Kanalmodus::mid   };

            auto baue = [&] (const MonoBand& erst, const MonoBand& zweit)
            {
                auto s = machSatz (true);
                setzeGlobalBool (s, "v2.global.auto_gain", true);
                belege (s, 0, erst.typ,  erst.f,  erst.q,  erst.gain,  erst.modus);
                belege (s, 1, zweit.typ, zweit.f, zweit.q, zweit.gain, zweit.modus);
                return s;
            };

            auto vorwaerts = neuerMonoKern (fs, 512);
            vorwaerts->uebernehmeZustand (baue (shelfRechts, bellMid));
            auto rueckwaerts = neuerMonoKern (fs, 512);
            rueckwaerts->uebernehmeZustand (baue (bellMid, shelfRechts));

            const double istV = vorwaerts->autoGainDb(), istR = rueckwaerts->autoGainDb();
            const double refV = monoReferenzDb ({ shelfRechts, bellMid }, fs);
            const double refR = monoReferenzDb ({ bellMid, shelfRechts }, fs);
            pruefe (std::abs (istV - refV) <= 1e-9 && std::abs (istR - refR) <= 1e-9
                        && std::abs (istV - istR) > 1e-9,
                    "311/M-59 mono_kaskade_in_slotreihenfolge (R-311-3, §9 F-11)",
                    "Slot 0 right, Slot 1 mid: " + zahl (istV, 12) + " dB gegen Referenz "
                        + zahl (refV, 12) + "; vertauscht: " + zahl (istR, 12) + " dB gegen Referenz "
                        + zahl (refR, 12) + "; Abstand der beiden Reihenfolgen "
                        + zahl (std::abs (istV - istR), 9) + " dB");
        }

        // ---- 311/M-64: nur die 1 waehlt die Monoformel ----------------------
        // Das Layout laesst nur Mono und Stereo zu
        // (`SondeProcessor::isBusesLayoutSupported`), 0 und 3 erreicht der
        // Prozessor nie; sie fallen auf die Zweikanalformel, weil die der
        // bisherige Stand ist (§9 F-10).
        {
            auto satz = [] ()
            {
                auto s = machSatz (true);
                setzeGlobalBool (s, "v2.global.auto_gain", true);
                belege (s, 0, Filtertyp::lowShelf, 1000.0, 1.0, 6.0, Kanalmodus::right);
                return s;
            };
            // Derselbe Pruefling wie `modus_right` im Hexgolden: sein
            // Zweikanalwert steht dort bitgenau.
            const std::uint64_t kZweikanal = 0xC0055DD001A5F001ull;

            std::string bericht;
            bool alleRichtig = true;
            for (const int n : { 0, 1, 2, 3 })
            {
                auto k = neuerMonoKern (fs, 512, n);
                k->uebernehmeZustand (satz());
                const double wert = k->autoGainDb();
                const std::uint64_t bits = bitsVon (wert);
                const bool ok = (n == 1)
                                  ? (wert == 0.0 && ! std::signbit (wert))
                                  : (std::memcmp (&bits, &kZweikanal, sizeof (std::uint64_t)) == 0);
                if (! ok) alleRichtig = false;
                bericht += (bericht.empty() ? "" : ", ") + std::to_string (n) + " -> " + hex64 (bits);
            }
            pruefe (alleRichtig, "311/M-64 kanalzahl_ausser_eins_ist_zweikanal (R-311-3)",
                    "Kanalzahl " + bericht + "; erwartet 1 -> 0x0000000000000000, sonst "
                        + hex64 (kZweikanal));
        }

        // ---- 311/M-65: Slotzahl 0 und 8 im Monobus --------------------------
        // (a) Regressionswache: OHNE aktives Band traegt der Kurzschluss die
        //     Zusage. Liefe der Gitterlauf, kaeme an jeder der 121 Stellen
        //     |a_L|^2 = 1,0 heraus, die Summe waere exakt 121,0, das Mittel
        //     exakt 1,0 - und -10*log10(1,0) ist -0,0, nicht +0,0.
        // (b) Heute rot: acht Low-Shelves +12 dB tragen unter 1 kHz praktisch
        //     die reelle Verstaerkung G = 3,981; die geordnete Monokaskade
        //     ergibt dort |a_L|^2 = G^12, die Zweikanalformel G^8*((G^2+1)/2)^2
        //     - 5,49 dB je Gitterstelle weniger.
        {
            auto ohneBand = neuerMonoKern (fs, 512);
            auto s0 = machSatz (true);
            setzeGlobalBool (s0, "v2.global.auto_gain", true);
            ohneBand->uebernehmeZustand (s0);
            const double leer = ohneBand->autoGainDb();
            pruefe (leer == 0.0 && ! std::signbit (leer),
                    "311/M-65 (a) mono_ohne_band_ist_exakt_null (Regressionswache, M-36)",
                    "exakt +0,0 (kein -0,0): " + zahl (leer, 15));

            const Kanalmodus acht[] = { Kanalmodus::stereo, Kanalmodus::left, Kanalmodus::right,
                                        Kanalmodus::mid,    Kanalmodus::stereo, Kanalmodus::left,
                                        Kanalmodus::right,  Kanalmodus::mid };
            auto s8 = machSatz (true);
            setzeGlobalBool (s8, "v2.global.auto_gain", true);
            std::vector<MonoBand> refBaender;
            for (int i = 0; i < 8; ++i)
            {
                belege (s8, i, Filtertyp::lowShelf, 1000.0, 8.0, 12.0, acht[(size_t) i]);
                refBaender.push_back ({ Filtertyp::lowShelf, 1000.0, 8.0, 12.0, acht[(size_t) i] });
            }
            auto achtKern = neuerMonoKern (fs, 512);
            achtKern->uebernehmeZustand (s8);
            const double ist = achtKern->autoGainDb();
            const double ref = monoReferenzDb (refBaender, fs);

            auto stereoKern = neuerKern (fs, 512);   // Kanalzahl 2: die alte Rechnung
            stereoKern->uebernehmeZustand (s8);
            const double zweikanal = stereoKern->autoGainDb();

            pruefe (std::isfinite (ist) && std::abs (ist - ref) <= 1e-9,
                    "311/M-65 (b) mono_acht_baender_folgt_der_kaskade (R-311-3)",
                    zahl (ist, 12) + " dB gegen die Testreferenz " + zahl (ref, 12)
                        + " dB; die Zweikanalformel gaebe " + zahl (zweikanal, 12) + " dB (Abstand "
                        + zahl (std::abs (ist - zweikanal), 6) + " dB)");
        }

        // ---- Selbstaudit: zwei Raender, die keine Matrixzeile allein trifft --
        // Keine Matrixzeilen und ohne eigenen Rotbeweis (Muster NAK-311 §22.5
        // Punkt 7); sie kamen aus dem Selbstaudit des Aenderungssatzes.

        // (1) Ein `side`-Band HINTER einem `left`-Band. M-58 misst den
        //     side-Zweig nur am symmetrischen Paar, wo die Seitenkomponente
        //     exakt 0 ist und er gar nichts tut. Erst hinter einem
        //     einseitigen Band ist sie ungleich 0 - dort muss die
        //     Rueckfuehrung stimmen.
        {
            const MonoBand links { Filtertyp::lowShelf,  1000.0, 1.0, 6.0, Kanalmodus::left };
            const MonoBand seite { Filtertyp::highShelf, 5000.0, 1.0, 6.0, Kanalmodus::side };
            auto s = machSatz (true);
            setzeGlobalBool (s, "v2.global.auto_gain", true);
            belege (s, 0, links.typ, links.f, links.q, links.gain, links.modus);
            belege (s, 1, seite.typ, seite.f, seite.q, seite.gain, seite.modus);
            auto k = neuerMonoKern (fs, 512);
            k->uebernehmeZustand (s);
            const double ist       = k->autoGainDb();
            const double ref       = monoReferenzDb ({ links, seite }, fs);
            const double ohneSeite = monoReferenzDb ({ links }, fs);
            pruefe (std::abs (ist - ref) <= 1e-9 && std::abs (ref - ohneSeite) > 1e-6,
                    "311/M-59 Selbstaudit side_hinter_left_wirkt",
                    zahl (ist, 12) + " dB gegen die Testreferenz " + zahl (ref, 12)
                        + " dB; ohne das side-Band waeren es " + zahl (ohneSeite, 12) + " dB");
        }

        // (2) Die beiden Wachen des Monozweigs, direkt gemessen (Zusage von
        //     M-65 (b): "ein nicht endliches oder nicht positives Mittel endet
        //     im Monozweig wie im Zweikanalzweig bei 0,0"). Im
        //     vertragsgueltigen Parameterraum ist beides unerreichbar - acht
        //     Shelves zu +12 dB ergeben 1,6e7 -, deshalb wird das Programm hier
        //     von Hand gebaut und `leiteAutoGainAb` einzeln gerufen.
        {
            auto mitAntwort = [] (double b0)
            {
                DspProgramm p;
                p.kanaele    = 1;
                p.samplerate = 48000.0;
                auto& b = p.baender[0];
                b.aktiv              = true;
                b.modus              = Kanalmodus::stereo;
                b.nutztSvf           = false;
                b.statischIstEinheit = false;
                b.statisch           = Biquad { b0, 0.0, 0.0, 0.0, 0.0 };
                return leiteAutoGainAb (p);
            };
            const double unendlich = mitAntwort (std::numeric_limits<double>::infinity());
            const double nullwert  = mitAntwort (0.0);
            pruefe (unendlich == 0.0 && ! std::signbit (unendlich)
                        && nullwert == 0.0 && ! std::signbit (nullwert),
                    "311/M-65 Selbstaudit monozweig_wacht_wie_der_zweikanalzweig",
                    "nicht endliches Mittel -> " + zahl (unendlich, 15)
                        + ", nicht positives Mittel -> " + zahl (nullwert, 15));
        }

        //======================================================================
        // NAK-311 Etappe 5, Aenderungssatz B (T3-15-09 Teil b, R-311-14, Karte
        // U54): die Obergrenze des ANGEWANDTEN Auto-Gain-Ausgleichs.
        //
        // Am Basisstand ist der abgeleitete Auto-Gain nach oben ungeklemmt:
        // acht High-Cuts 20 Hz Q 0,15 ergeben +150,46 dB, dieselben mit Q 0,707
        // +42,99 dB - beides vertragsgueltig, und beides trifft Gleichanteil
        // und Infraschall, die das Gitter ab 20 Hz gar nicht erfasst. Seit
        // R-311-14 deckelt `gedeckelterAutoGainDb` den ANGEWANDTEN Ausgleich
        // EINSEITIG auf `kAutoGainDeckelDb`, und zwar an den zwei
        // Rueckgabezeilen von `leiteAutoGainAb` - der einen Stelle, an der
        // Programm, Kern und Bericht dieselbe Zahl lesen (M-117). Die
        // Absenkungsseite bleibt unberuehrt (M-114).
        //
        // Das MASS der Audiofolge ist ein Verhaeltnis zweier Laeufe, die sich
        // NUR in `v2.global.auto_gain` unterscheiden: die Baender bekommen
        // beidemal denselben Eingang und tragen denselben Zustand, der
        // Auto-Gain wirkt danach als Faktor. Das Verhaeltnis ihrer Taps IST
        // deshalb der angewandte Faktor - unabhaengig davon, wie weit die
        // 20-Hz-Kaskade schon eingeschwungen ist.
        //
        // Matrixzeilen 311/M-112, M-113, M-115, M-116 und die Teilfaelle zu
        // 311/M-114 und 311/M-118 (Manifest NAK-311 §39.2). 311/M-119 steht
        // oben im M-61-Lauf, 311/M-117 in B3c und B7.
        {
            const double deckelLin = dbInLinear (kAutoGainDeckelDb);

            /** Acht gleiche High-Cuts 20 Hz im Modus `stereo`, Auto-Gain nach
                Wunsch - der Pruefling der Abnahme U54. Ueber `q` faehrt der
                abgeleitete Ausgleich von +150 dB (Q 0,15) ueber +43 dB
                (Q 0,707) bis unter den Deckel (Q 1,0); Typ, Kanalmodus und
                Belegung bleiben dabei gleich, ein Wechsel der Guete allein ist
                also ein reiner Rampenwechsel. */
            auto achtHighCuts = [] (double q, bool autoGainAn)
            {
                auto s = machSatz (true);
                setzeGlobalBool (s, "v2.global.auto_gain", autoGainAn);
                for (int slot = 0; slot < param::kSlots; ++slot)
                    belege (s, slot, Filtertyp::highCut, 20.0, q, 0.0, Kanalmodus::stereo);
                return s;
            };

            /** Acht gleiche Low-Shelves 1 kHz +12 dB - der Gegenfall auf der
                ABSENKUNGSseite (M-114, derselbe Pruefling wie 311/M-67). */
            auto achtLowShelves = [] (double q)
            {
                auto s = machSatz (true);
                setzeGlobalBool (s, "v2.global.auto_gain", true);
                for (int slot = 0; slot < param::kSlots; ++slot)
                    belege (s, slot, Filtertyp::lowShelf, 1000.0, q, 12.0, Kanalmodus::stereo);
                return s;
            };

            /** Faehrt Gleichanteil 0,25 plus 10-Hz-Ton 0,25 auf beiden Kanaelen
                - genau das Material, das die acht High-Cuts stehen lassen und
                das der Auto-Gain deshalb voll aufdreht. `tapAus` nimmt, wenn
                gesetzt, den Tap `post_committed` (Kanal 0) Sample fuer Sample
                auf; `spitze` meldet den groessten Betrag des LETZTEN Blocks. */
            auto fahreGleichanteil = [&fs] (DspKern& k, long long& n0, int bloecke, int blockGroesse,
                                            std::vector<double>* tapAus)
            {
                std::vector<float> a ((size_t) blockGroesse), b ((size_t) blockGroesse);
                float* kan[2] = { a.data(), b.data() };
                const double w = 2.0 * kPiRef * 10.0 / fs;
                double spitze = 0.0;
                for (int block = 0; block < bloecke; ++block)
                {
                    for (int i = 0; i < blockGroesse; ++i)
                    {
                        const auto v = (float) (0.25 + 0.25 * std::sin (w * (double) (n0 + i)));
                        a[(size_t) i] = v;
                        b[(size_t) i] = v;
                    }
                    k.verarbeite (kan, 2, blockGroesse);
                    if (const double* t = k.tap (Tap::postCommitted, 0))
                    {
                        if (tapAus != nullptr)
                            for (int i = 0; i < blockGroesse; ++i) tapAus->push_back (t[i]);
                        if (block + 1 == bloecke)
                        {
                            spitze = 0.0;
                            for (int i = 0; i < blockGroesse; ++i) spitze = std::max (spitze, std::abs (t[i]));
                        }
                    }
                    n0 += blockGroesse;
                }
                return spitze;
            };

            /** Das Programm der GEFAHRENEN Committed-Bank - die Zahlen, die der
                Audiothread wirklich sieht, nicht die eines Vorbaus. */
            auto gefahrenesProgramm = [] (DspKern& k) -> const DspProgramm*
            {
                int cA = -1, cQ = -1, kA = -1, kQ = -1;
                k.gefahreneSlots (cA, cQ, kA, kQ);
                return cA >= 0 ? &k.pool().bank (cA).programm : nullptr;
            };

            // ---- 311/M-112 mit dem Teilfall 311/M-118 ----------------------
            // Heute rot: ohne Deckel faehrt der Kern +150,46 dB, also Faktor
            // 3,3e7 auf Gleichanteil und Infraschall.
            {
                auto mit = neuerKern (fs, 512);
                mit->uebernehmeZustand (achtHighCuts (0.15, true));
                long long nMit = 0;
                const double spitzeMit = fahreGleichanteil (*mit, nMit, 48, 512, nullptr);

                auto ohne = neuerKern (fs, 512);
                ohne->uebernehmeZustand (achtHighCuts (0.15, false));
                long long nOhne = 0;
                const double spitzeOhne = fahreGleichanteil (*ohne, nOhne, 48, 512, nullptr);

                const auto* prog = gefahrenesProgramm (*mit);
                const double roh       = mit->autoGainRohDb();
                const double angewandt = mit->autoGainDb();
                const double faktor = spitzeOhne > 0.0 ? spitzeMit / spitzeOhne
                                                       : std::numeric_limits<double>::infinity();

                const bool zahlen = prog != nullptr
                                 && bitsVon (prog->autoGainDb)    == bitsVon (kAutoGainDeckelDb)
                                 && bitsVon (prog->autoGainLin)   == bitsVon (deckelLin)
                                 && bitsVon (prog->autoGainRohDb) == bitsVon (roh)
                                 && bitsVon (angewandt) == bitsVon (kAutoGainDeckelDb)
                                 && roh > 150.0 && roh < 151.0
                                 && mit->autoGainGedeckelt();

                pruefe (zahlen && std::abs (faktor - deckelLin) <= 1e-9 * deckelLin,
                        "311/M-112 auto_gain_deckel_greift_und_meldet (NAK-311 R-311-14, T3-15-09 Teil b, U54)",
                        "acht High-Cuts 20 Hz Q 0,15 bei 48 kHz: abgeleitet " + zahl (roh, 9)
                            + " dB (lesbar ueber autoGainRohDb), ANGEWANDT " + zahl (angewandt, 15)
                            + " dB = kAutoGainDeckelDb bitgenau " + std::string (zahlen ? "ja" : "NEIN")
                            + ", autoGainLin " + zahl (prog != nullptr ? prog->autoGainLin : 0.0, 12)
                            + " gegen dbInLinear(24) " + zahl (deckelLin, 12)
                            + "; Spitze am Tap post_committed mit Auto-Gain " + zahl (spitzeMit, 9)
                            + " gegen denselben Lauf ohne " + zahl (spitzeOhne, 9) + " = Faktor "
                            + zahl (faktor, 9) + " (Deckel laesst hoechstens " + zahl (deckelLin, 9)
                            + " zu; ungedeckelt waere der Faktor " + zahl (dbInLinear (roh), 1)
                            + "); Zustand " + (mit->autoGainGedeckelt() ? "gesetzt" : "NICHT gesetzt"));

                // 311/M-118 (Teilfall): derselbe Pruefling mit AUSGESCHALTETEM
                // Auto-Gain. Gerechnet und lesbar ist der Wert wie seit R4 -
                // ANGEWANDT wird nichts (die Spitze bleibt unter dem Eingang
                // 0,5; mit dem Deckel waeren es 7,9), und genau deshalb ist der
                // Zustand FALSCH: gemeldet wird ein gedeckelter Ausgleich nur,
                // wenn einer wirkt (F-24).
                const auto* progOhne = gefahrenesProgramm (*ohne);
                const double rohOhne = ohne->autoGainRohDb();
                pruefe (progOhne != nullptr
                            && bitsVon (progOhne->autoGainDb)    == bitsVon (kAutoGainDeckelDb)
                            && bitsVon (progOhne->autoGainRohDb) == bitsVon (rohOhne)
                            && rohOhne > 150.0 && rohOhne < 151.0
                            && ! ohne->autoGainGedeckelt()
                            && spitzeOhne < 0.5001 && spitzeOhne > 0.0,
                        "311/M-118 auto_gain_aus_deckelt_ohne_zu_melden (Teilfall von 311/M-112, F-24, M-35)",
                        "autoGainDb " + zahl (ohne->autoGainDb(), 15) + " dB (gedeckelt), autoGainRohDb "
                            + zahl (rohOhne, 9) + " dB (abgeleitet), Zustand "
                            + (ohne->autoGainGedeckelt() ? "GESETZT" : "falsch")
                            + "; Spitze am Tap post_committed " + zahl (spitzeOhne, 9)
                            + " unter dem Eingangsgleichanteil - angewandt wird nichts");
            }

            // ---- 311/M-114 (Teilfall): die Absenkungsseite bleibt, wie sie ist
            // Regressionswache. Der Deckel ist EINSEITIG: -199,77 dB und
            // -92,29 dB kommen BITGLEICH durch ihn, und der Zustand ist falsch.
            // Die Berichtsklemmung auf -120 ist eine zweite, getrennte Stufe
            // und wird in B3c gemessen (311/M-67, 311/M-68).
            {
                auto tief = neuerKern (fs, 512);
                tief->uebernehmeZustand (achtLowShelves (8.0));
                auto flach = neuerKern (fs, 512);
                flach->uebernehmeZustand (achtLowShelves (0.70710678118654752));

                const double tiefDb  = tief->autoGainDb(),  tiefRoh  = tief->autoGainRohDb();
                const double flachDb = flach->autoGainDb(), flachRoh = flach->autoGainRohDb();

                pruefe (bitsVon (tiefDb) == bitsVon (tiefRoh) && bitsVon (flachDb) == bitsVon (flachRoh)
                            && tiefDb < -190.0 && tiefDb > -210.0
                            && flachDb < -90.0 && flachDb > -95.0
                            && ! tief->autoGainGedeckelt() && ! flach->autoGainGedeckelt(),
                        "311/M-114 absenkungsseite_bleibt_ungedeckelt (Regressionswache, NAK-311 R-311-14, M-71)",
                        "acht Low-Shelves 1 kHz +12 dB Q 8: angewandt " + zahl (tiefDb, 9)
                            + " dB, abgeleitet " + zahl (tiefRoh, 9) + " dB, bitgleich "
                            + (bitsVon (tiefDb) == bitsVon (tiefRoh) ? "ja" : "NEIN") + "; Q 0,707: "
                            + zahl (flachDb, 9) + " dB gegen " + zahl (flachRoh, 9) + " dB, bitgleich "
                            + (bitsVon (flachDb) == bitsVon (flachRoh) ? "ja" : "NEIN") + "; Zustand "
                            + (tief->autoGainGedeckelt() || flach->autoGainGedeckelt() ? "GESETZT" : "falsch"));
            }

            // ---- 311/M-113 (a): die Deckelfunktion allein an drei Kanten ----
            // STRIKT groesser entscheidet; kein std::clamp, keine
            // Multiplikation, kein Runden (Muster `berichtsAutoGainDb`).
            {
                const double ueber = std::nextafter (kAutoGainDeckelDb, std::numeric_limits<double>::infinity());
                const double unter = std::nextafter (kAutoGainDeckelDb, -std::numeric_limits<double>::infinity());
                const double minusNull = -0.0, subnormal = std::numeric_limits<double>::denorm_min();

                const bool kanten = bitsVon (gedeckelterAutoGainDb (unter)) == bitsVon (unter)
                                 && bitsVon (gedeckelterAutoGainDb (kAutoGainDeckelDb)) == bitsVon (kAutoGainDeckelDb)
                                 && bitsVon (gedeckelterAutoGainDb (ueber)) == bitsVon (kAutoGainDeckelDb)
                                 && bitsVon (gedeckelterAutoGainDb (24.5)) == bitsVon (kAutoGainDeckelDb);
                const bool durchlass = bitsVon (gedeckelterAutoGainDb (-199.767783)) == bitsVon (-199.767783)
                                    && bitsVon (gedeckelterAutoGainDb (0.0)) == bitsVon (0.0)
                                    && bitsVon (gedeckelterAutoGainDb (minusNull)) == bitsVon (minusNull)
                                    && bitsVon (gedeckelterAutoGainDb (subnormal)) == bitsVon (subnormal)
                                    && bitsVon (gedeckelterAutoGainDb (-subnormal)) == bitsVon (-subnormal);
                pruefe (kanten && durchlass,
                        "311/M-113 (a) deckel_an_seinen_drei_kanten (NAK-311 R-311-14, Muster berichtsAutoGainDb)",
                        "Deckel " + zahl (kAutoGainDeckelDb, 1) + " dB; -1 ULP " + zahl (unter, 17) + " -> "
                            + zahl (gedeckelterAutoGainDb (unter), 17) + " (bitgleich), genau -> "
                            + zahl (gedeckelterAutoGainDb (kAutoGainDeckelDb), 17) + " (bitgleich), +1 ULP "
                            + zahl (ueber, 17) + " -> " + zahl (gedeckelterAutoGainDb (ueber), 17)
                            + " (auf den Deckel); Absenkungsseite, +0,0, -0,0 und das kleinste Subnormal "
                            "kommen bitgleich zurueck " + (durchlass ? "ja" : "NEIN"));
            }

            // ---- 311/M-113 (b): Zahlenrand Slotzahl 0 ----------------------
            // Ohne aktives Band traegt der Kurzschluss die Zusage - exakt +0,0
            // (kein -0,0) in BEIDEN Werten, und der Zustand ist falsch (M-36).
            {
                auto leer = neuerKern (fs, 512);
                auto s0 = machSatz (true);
                setzeGlobalBool (s0, "v2.global.auto_gain", true);
                leer->uebernehmeZustand (s0);
                const double db = leer->autoGainDb(), roh = leer->autoGainRohDb();
                pruefe (db == 0.0 && ! std::signbit (db) && roh == 0.0 && ! std::signbit (roh)
                            && ! leer->autoGainGedeckelt(),
                        "311/M-113 (b) ohne_aktives_band_bleibt_der_kurzschluss (Teilfall, M-36)",
                        "autoGainDb " + zahl (db, 15) + ", autoGainRohDb " + zahl (roh, 15)
                            + " (Vorzeichenbits " + std::to_string (std::signbit (db) ? 1 : 0)
                            + std::to_string (std::signbit (roh) ? 1 : 0) + "), Zustand "
                            + (leer->autoGainGedeckelt() ? "GESETZT" : "falsch"));
            }

            // ---- 311/M-113 (c): das PROGRAMM an der Grenze -----------------
            // Ein von Hand gebautes Programm mit konstanter Betragsantwort b0
            // traegt an jeder Gitterstelle |H|^2 = b0^2; der abgeleitete Wert
            // ist damit -20*log10(b0) und ueber `b0` bis auf das letzte Bit
            // feiner einstellbar als ueber jeden Filterentwurf (Muster des
            // M-65-Selbstaudits oben). Auch damit ist die Grenze nicht
            // BITGENAU abzuleiten - der Lauf unten misst genau das -, deshalb
            // traegt die Kante "genau auf der Grenze" die Funktion (a) und das
            // Praedikat `DspProgramm::autoGainGedeckelt()`.
            {
                auto programmMit = [&fs] (double b0)
                {
                    DspProgramm p;
                    p.kanaele    = 2;
                    p.samplerate = fs;
                    p.autoGainAn = true;
                    auto& b = p.baender[0];
                    b.aktiv              = true;
                    b.modus              = Kanalmodus::stereo;
                    b.nutztSvf           = false;
                    b.statischIstEinheit = false;
                    b.statisch           = Biquad { b0, 0.0, 0.0, 0.0, 0.0 };
                    double roh = 0.0;
                    p.autoGainDb    = leiteAutoGainAb (p, &roh);
                    p.autoGainRohDb = roh;
                    return p;
                };

                // 800 aufeinanderfolgende doubles um den analytischen Wert:
                // waechst b0, faellt der abgeleitete Wert monoton. GEMESSEN
                // wird dabei auch, dass die Ableitung die Grenze ueberspringt:
                // benachbarte `mittel`-Werte liegen im log10 rund 4,44e-16
                // auseinander, mal zehn also 4,44e-15 - mehr als ein ULP von
                // 24,0 (3,55e-15). Kein Programm dieser Familie leitet die
                // Grenze bitgenau ab; die Kante "genau auf der Grenze" traegt
                // deshalb die Funktion (a) und das Praedikat unten.
                double b0Genau = 0.0, b0Ueber = 0.0, b0Unter = 0.0;
                {
                    double b = std::pow (10.0, -kAutoGainDeckelDb / 20.0);
                    for (int i = 0; i < 400; ++i) b = std::nextafter (b, 0.0);
                    for (int i = 0; i < 800; ++i)
                    {
                        const double roh = programmMit (b).autoGainRohDb;
                        if (roh >  kAutoGainDeckelDb) b0Ueber = b;
                        if (roh == kAutoGainDeckelDb && b0Genau == 0.0) b0Genau = b;
                        if (roh <  kAutoGainDeckelDb && b0Unter == 0.0) b0Unter = b;
                        b = std::nextafter (b, 1.0);
                    }
                }

                const bool klammer = b0Ueber != 0.0 && b0Unter != 0.0;
                const auto pUeber = programmMit (klammer ? b0Ueber : 1.0);
                const auto pUnter = programmMit (klammer ? b0Unter : 1.0);

                // Das Praedikat selbst an seinen drei Kanten, mit dem
                // abgeleiteten Wert direkt gesetzt - hier ist "genau auf der
                // Grenze" erreichbar. Strikt groesser entscheidet, und ohne
                // eingeschalteten Auto-Gain ist keine Kante gesetzt (F-24).
                const double kanteUnter = std::nextafter (kAutoGainDeckelDb,
                                                          -std::numeric_limits<double>::infinity());
                const double kanteUeber = std::nextafter (kAutoGainDeckelDb,
                                                          std::numeric_limits<double>::infinity());
                auto praedikat = [] (double rohDb, bool an)
                {
                    DspProgramm p;
                    p.autoGainAn    = an;
                    p.autoGainRohDb = rohDb;
                    p.autoGainDb    = gedeckelterAutoGainDb (rohDb);
                    return p.autoGainGedeckelt();
                };
                const bool kanten = ! praedikat (kanteUnter, true)
                                 && ! praedikat (kAutoGainDeckelDb, true)
                                 &&   praedikat (kanteUeber, true)
                                 && ! praedikat (kanteUnter, false)
                                 && ! praedikat (kAutoGainDeckelDb, false)
                                 && ! praedikat (kanteUeber, false);

                const bool richtig = klammer && kanten && b0Genau == 0.0
                    // knapp DARUEBER: bitgenau auf den Deckel gesetzt, Zustand gesetzt
                    && bitsVon (pUeber.autoGainDb) == bitsVon (kAutoGainDeckelDb)
                    && pUeber.autoGainRohDb > kAutoGainDeckelDb && pUeber.autoGainGedeckelt()
                    // knapp DARUNTER: bitgleich durch, Zustand falsch
                    && bitsVon (pUnter.autoGainDb) == bitsVon (pUnter.autoGainRohDb)
                    && pUnter.autoGainRohDb < kAutoGainDeckelDb && ! pUnter.autoGainGedeckelt();
                pruefe (richtig,
                        "311/M-113 (c) programm_und_praedikat_an_der_grenze (NAK-311 R-311-14, F-24)",
                        "abgeleitet knapp darueber " + zahl (pUeber.autoGainRohDb, 17) + " -> angewandt "
                            + zahl (pUeber.autoGainDb, 17) + " (Zustand "
                            + (pUeber.autoGainGedeckelt() ? "gesetzt" : "FALSCH") + "), knapp darunter "
                            + zahl (pUnter.autoGainRohDb, 17) + " -> " + zahl (pUnter.autoGainDb, 17)
                            + " (Zustand " + (pUnter.autoGainGedeckelt() ? "GESETZT" : "falsch")
                            + "); die Ableitung UEBERSPRINGT die Grenze - exakte Treffer in 800 "
                            "benachbarten doubles: " + std::to_string (b0Genau == 0.0 ? 0 : 1)
                            + "; Praedikat an -1 ULP / genau / +1 ULP mit Auto-Gain an: "
                            + (praedikat (kanteUnter, true) ? "1" : "0")
                            + (praedikat (kAutoGainDeckelDb, true) ? "1" : "0")
                            + (praedikat (kanteUeber, true) ? "1" : "0") + ", mit Auto-Gain aus: "
                            + (praedikat (kanteUnter, false) ? "1" : "0")
                            + (praedikat (kAutoGainDeckelDb, false) ? "1" : "0")
                            + (praedikat (kanteUeber, false) ? "1" : "0") + " (erwartet 001 und 000)");
            }

            // ---- 311/M-113 (d): Zahlenrand Slotzahl 8 am echten Bauweg -----
            // Zwei benachbarte Guetewerte der acht High-Cuts, deren abgeleitete
            // Werte den Deckel einschliessen: der Zustand wird genau dann
            // gesetzt, wenn der Deckel greift.
            {
                auto rohVonQ = [&] (double q)
                {
                    DspProgramm p;
                    baueProgramm (achtHighCuts (q, true), fs, 0, p, 2);
                    return p.autoGainRohDb;
                };

                double qUeber = 0.9, qUnter = 1.0;   // ueber bzw. unter dem Deckel
                const bool klammerStimmt = rohVonQ (qUeber) > kAutoGainDeckelDb
                                        && rohVonQ (qUnter) < kAutoGainDeckelDb;
                for (int i = 0; i < 80 && klammerStimmt; ++i)
                {
                    const double mitteQ = 0.5 * (qUeber + qUnter);
                    if (mitteQ == qUeber || mitteQ == qUnter) break;
                    if (rohVonQ (mitteQ) > kAutoGainDeckelDb) qUeber = mitteQ; else qUnter = mitteQ;
                }

                auto kUeber = neuerKern (fs, 512);
                kUeber->uebernehmeZustand (achtHighCuts (qUeber, true));
                auto kUnter = neuerKern (fs, 512);
                kUnter->uebernehmeZustand (achtHighCuts (qUnter, true));

                pruefe (klammerStimmt
                            && bitsVon (kUeber->autoGainDb()) == bitsVon (kAutoGainDeckelDb)
                            && kUeber->autoGainRohDb() > kAutoGainDeckelDb && kUeber->autoGainGedeckelt()
                            && bitsVon (kUnter->autoGainDb()) == bitsVon (kUnter->autoGainRohDb())
                            && kUnter->autoGainRohDb() < kAutoGainDeckelDb && ! kUnter->autoGainGedeckelt(),
                        "311/M-113 (d) acht_high_cuts_beidseits_der_grenze (Teilfall, NAK-311 R-311-14)",
                        "Q " + zahl (qUeber, 15) + " -> abgeleitet " + zahl (kUeber->autoGainRohDb(), 12)
                            + " dB, angewandt " + zahl (kUeber->autoGainDb(), 12) + " dB, Zustand "
                            + (kUeber->autoGainGedeckelt() ? "gesetzt" : "FALSCH") + "; Q "
                            + zahl (qUnter, 15) + " -> abgeleitet " + zahl (kUnter->autoGainRohDb(), 12)
                            + " dB, angewandt " + zahl (kUnter->autoGainDb(), 12) + " dB, Zustand "
                            + (kUnter->autoGainGedeckelt() ? "GESETZT" : "falsch"));
            }

            // ---- 311/M-115: der Deckel steht in BEIDEN Rueckgabezeilen ------
            // Acht `stereo`-Baender rechnen im Monozweig dieselbe Kaskade und
            // denselben Wert wie im Zweikanalzweig - der Unterschied liegt
            // allein in der Rueckgabezeile. Stuende der Deckel nur in einer,
            // fuehre der andere Bus +150,46 dB.
            {
                auto mono = std::make_unique<DspKern>();   // NAK-175: auf den HEAP
                mono->bereiteVor (fs, 512, 1);
                mono->uebernehmeZustand (achtHighCuts (0.15, true));
                {
                    std::vector<float> a (512, 0.25f);
                    float* kan[1] = { a.data() };
                    for (int block = 0; block < 4; ++block)
                    {
                        std::fill (a.begin(), a.end(), 0.25f);
                        mono->verarbeite (kan, 1, 512);
                    }
                }

                auto stereo = neuerKern (fs, 512);         // Kanalzahl 2
                stereo->uebernehmeZustand (achtHighCuts (0.15, true));
                long long nStereo = 0;
                fahreGleichanteil (*stereo, nStereo, 4, 512, nullptr);

                const auto* progMono   = gefahrenesProgramm (*mono);
                const auto* progStereo = gefahrenesProgramm (*stereo);
                pruefe (progMono != nullptr && progStereo != nullptr
                            && progMono->kanaele == 1 && progStereo->kanaele == 2
                            && bitsVon (mono->autoGainDb()) == bitsVon (kAutoGainDeckelDb)
                            && bitsVon (stereo->autoGainDb()) == bitsVon (kAutoGainDeckelDb)
                            && bitsVon (mono->autoGainRohDb()) == bitsVon (stereo->autoGainRohDb())
                            && mono->autoGainGedeckelt() && stereo->autoGainGedeckelt(),
                        "311/M-115 deckel_in_beiden_zweigen (NAK-311 R-311-14, R-311-3, §32.2)",
                        "Monozweig (Programm-Kanalzahl "
                            + std::to_string (progMono != nullptr ? progMono->kanaele : -1)
                            + ") abgeleitet " + zahl (mono->autoGainRohDb(), 12) + " dB -> angewandt "
                            + zahl (mono->autoGainDb(), 15) + " dB; Zweikanalzweig (Kanalzahl "
                            + std::to_string (progStereo != nullptr ? progStereo->kanaele : -1)
                            + ") abgeleitet " + zahl (stereo->autoGainRohDb(), 12) + " dB -> angewandt "
                            + zahl (stereo->autoGainDb(), 15) + " dB; beide Zustaende "
                            + (mono->autoGainGedeckelt() && stereo->autoGainGedeckelt() ? "gesetzt" : "NICHT gesetzt"));
            }

            // ---- 311/M-116: der Zustand faellt zurueck ----------------------
            // setzen <-> zuruecknehmen am selben Kern: Q 0,15 (+150 dB, ueber
            // dem Deckel) -> Q 0,707 (+43 dB, immer noch darueber) -> Q 1,0
            // (unter dem Deckel). Typ, Kanalmodus und Belegung bleiben gleich,
            // der Wechsel ist also ein Rampenwechsel: die Auto-Gain-Rampe laeuft
            // ueber `kRampeSamples` auf das neue Ziel, sie springt nicht (M-02).
            // Gemessen am Verhaeltnis zweier Laeufe, die sich NUR in
            // `v2.global.auto_gain` unterscheiden - es ist der angewandte
            // Faktor selbst.
            {
                auto mit  = neuerKern (fs, 512);
                auto ohne = neuerKern (fs, 512);
                long long nMit = 0, nOhne = 0;

                mit ->uebernehmeZustand (achtHighCuts (0.15, true));
                ohne->uebernehmeZustand (achtHighCuts (0.15, false));
                fahreGleichanteil (*mit,  nMit,  48, 512, nullptr);
                fahreGleichanteil (*ohne, nOhne, 48, 512, nullptr);
                const bool   zustand1 = mit->autoGainGedeckelt();
                const double db1      = mit->autoGainDb();

                mit ->uebernehmeZustand (achtHighCuts (0.70710678118654752, true));
                ohne->uebernehmeZustand (achtHighCuts (0.70710678118654752, false));
                fahreGleichanteil (*mit,  nMit,  48, 512, nullptr);
                fahreGleichanteil (*ohne, nOhne, 48, 512, nullptr);
                const bool   zustand2 = mit->autoGainGedeckelt();
                const double db2 = mit->autoGainDb(), roh2 = mit->autoGainRohDb();

                mit ->uebernehmeZustand (achtHighCuts (1.0, true));
                ohne->uebernehmeZustand (achtHighCuts (1.0, false));
                std::vector<double> tapMit, tapOhne;
                fahreGleichanteil (*mit,  nMit,  2, 512, &tapMit);
                fahreGleichanteil (*ohne, nOhne, 2, 512, &tapOhne);
                const bool   zustand3 = mit->autoGainGedeckelt();
                const double db3 = mit->autoGainDb(), roh3 = mit->autoGainRohDb();

                const double neuLin = dbInLinear (roh3);
                const double faktorErst = tapOhne[0]   != 0.0 ? tapMit[0]   / tapOhne[0]   : 0.0;
                const double faktorSpaet = tapOhne[300] != 0.0 ? tapMit[300] / tapOhne[300] : 0.0;
                const bool keinSprung = std::abs (faktorErst - deckelLin) < 0.01 * deckelLin
                                     && std::abs (faktorSpaet - neuLin)   < 1e-6 * neuLin
                                     && faktorErst > neuLin * 1.05;

                pruefe (zustand1 && zustand2 && ! zustand3
                            && bitsVon (db1) == bitsVon (kAutoGainDeckelDb)
                            && bitsVon (db2) == bitsVon (kAutoGainDeckelDb) && roh2 > kAutoGainDeckelDb
                            && bitsVon (db3) == bitsVon (roh3) && roh3 < kAutoGainDeckelDb
                            && keinSprung,
                        "311/M-116 deckelzustand_faellt_zurueck (NAK-311 R-311-14, aktivieren<->abklingen)",
                        "Q 0,15: angewandt " + zahl (db1, 12) + " dB, Zustand "
                            + (zustand1 ? "gesetzt" : "FALSCH") + "; Q 0,707: abgeleitet " + zahl (roh2, 9)
                            + " dB, angewandt " + zahl (db2, 12) + " dB, Zustand "
                            + (zustand2 ? "gesetzt" : "FALSCH") + "; Q 1,0: abgeleitet " + zahl (roh3, 9)
                            + " dB, angewandt " + zahl (db3, 9) + " dB (bitgleich "
                            + (bitsVon (db3) == bitsVon (roh3) ? "ja" : "NEIN") + "), Zustand "
                            + (zustand3 ? "GESETZT" : "falsch")
                            + "; angewandter Faktor am ersten Sample nach dem Wechsel " + zahl (faktorErst, 9)
                            + " (altes Ziel " + zahl (deckelLin, 9) + "), nach " + std::to_string (300)
                            + " Samples " + zahl (faktorSpaet, 9) + " (neues Ziel " + zahl (neuLin, 9)
                            + ", Rampe " + std::to_string (kRampeSamples) + " Samples)");
            }
        }
    }

    //==========================================================================
    std::cout << std::endl << "== I - Bankpool und Ownership (M-41 bis M-46) ==" << std::endl;
    {
        auto kern = neuerKern (48000.0, 512);
        auto& pool = kern->pool();

        pruefe (DspBankPool::kBaenke == 4, "vier_baenke_vorallokiert (M-41)",
                std::to_string (DspBankPool::kBaenke));
        pruefe (DspBankPool::kAckKapazitaet > DspBankPool::kBaenke,
                "der ACK-Ring fasst mehr Eintraege als es Slots gibt (M-45)",
                std::to_string (DspBankPool::kAckKapazitaet) + " > " + std::to_string (DspBankPool::kBaenke));

        // M-42: jeder der sechs Uebergaenge einzeln - und ein Uebergang, den
        // der Vertrag NICHT nennt, scheitert.
        {
            pool.zuruecksetzen();
            const int slot = pool.reserviere();
            pruefe (slot >= 0 && pool.zustand (slot) == BankZustand::vorbereitend,
                    "uebergang_frei_nach_vorbereitend (M-42)");

            pool.bank (slot).programm.generation = pool.naechsteGeneration();
            pool.publiziere (Pfad::committed, slot);
            pruefe (pool.zustand (slot) == BankZustand::bereit,
                    "uebergang_vorbereitend_nach_bereit (M-42)");

            const int genommen = pool.uebernehmeBereiten (Pfad::committed);
            pruefe (genommen == slot && pool.zustand (slot) == BankZustand::audioAktiv,
                    "uebergang_bereit_nach_audio_aktiv (M-42)");

            pruefe (pool.beginneVerblassen (slot) && pool.zustand (slot) == BankZustand::verblassend,
                    "uebergang_audio_aktiv_nach_verblassend (M-42)");

            pool.meldeAusgedient (slot);
            pruefe (pool.zustand (slot) == BankZustand::ausgedient,
                    "uebergang_verblassend_nach_ausgedient (M-42)");

            pruefe (pool.ernteAcks() == 1 && pool.zustand (slot) == BankZustand::frei,
                    "uebergang_ausgedient_nach_frei_erst_nach_ack (M-42, M-43)");

            // Ein nicht vorgesehener Uebergang scheitert.
            pruefe (! pool.beginneVerblassen (slot),
                    "ein_nicht_genannter_uebergang_scheitert (M-42)",
                    "frei -> verblassend ist im Automaten nicht moeglich");
        }

        // B-1 / M-42: das Interleaving aus dem Befund, deterministisch. Der
        // Audiothread liest die Publikation A; bevor er uebernimmt, ersetzt der
        // Worker A durch B, gibt A frei, belegt A neu und publiziert es fuer
        // Candidate. Die Uebernahme mit der alten Beobachtung darf A NICHT
        // nehmen; B wird Committed, A bleibt Candidate.
        {
            auto p = std::make_unique<DspBankPool>();
            auto mitGeneration = [&] (int s) { p->bank (s).programm.generation = p->naechsteGeneration(); };

            const int a = p->reserviere();
            mitGeneration (a);
            p->publiziere (Pfad::committed, a);
            const std::uint64_t beobachtet = p->publikation (Pfad::committed);

            const int b = p->reserviere();
            mitGeneration (b);
            const int verdraengt = p->publiziere (Pfad::committed, b);
            const int wieder = p->reserviere();
            mitGeneration (wieder);
            p->publiziere (Pfad::candidate, wieder);

            const int genommen    = p->uebernehme (Pfad::committed, beobachtet);
            const bool aBleibt    = p->zustand (a) == BankZustand::bereit;
            const int committed   = p->uebernehmeBereiten (Pfad::committed);
            const int candidate   = p->uebernehmeBereiten (Pfad::candidate);
            pruefe (verdraengt == a && wieder == a && genommen == -1 && aBleibt && committed == b && candidate == a,
                    "uebernahme_nur_in_der_publizierten_generation (M-42, B-1)",
                    "verdraengt " + std::to_string (verdraengt) + ", neu belegt " + std::to_string (wieder)
                    + ", Uebernahme mit alter Beobachtung " + std::to_string (genommen) + ", Committed "
                    + std::to_string (committed) + ", Candidate " + std::to_string (candidate));
        }

        // B-12 / R9 Feinheit 2: der Generationszaehler ueberlebt den Neuanlauf
        // (freigeben -> bereiteVor), und ein ACK einer ALTEN Generation gibt
        // danach keine Bank frei.
        {
            auto k = neuerKern (48000.0, 256);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 6.0);
            auto hoechsteGeneration = [&]
            {
                std::uint64_t g = 0;
                for (int i = 0; i < DspBankPool::kBaenke; ++i) g = std::max (g, k->pool().generation (i));
                return g;
            };
            k->uebernehmeZustand (s);
            const std::uint64_t g1 = hoechsteGeneration();
            k->freigeben();
            k->bereiteVor (48000.0, 256);
            k->uebernehmeZustand (s);
            const std::uint64_t g2 = hoechsteGeneration();
            pruefe (g1 > 0 && g2 > g1, "generation_waechst_ueber_den_neuanlauf (R9 Feinheit 2, B-12)",
                    "vor dem Neuanlauf " + std::to_string (g1) + ", danach " + std::to_string (g2));

            fahreStille (*k, 256, 256);
            auto& kp = k->pool();
            int s2 = -1;
            for (int i = 0; i < DspBankPool::kBaenke; ++i)
                if (kp.zustand (i) == BankZustand::audioAktiv && kp.generation (i) == g2) s2 = i;
            bool ok = s2 >= 0 && kp.beginneVerblassen (s2) && kp.ackEinreihen (s2, g1);
            kp.meldeAusgedient (s2);
            const int ersteErnte = kp.ernteAcks (1);
            const bool nochAusgedient = s2 >= 0 && kp.zustand (s2) == BankZustand::ausgedient;
            const int zweiteErnte = kp.ernteAcks();
            ok = ok && ersteErnte == 0 && nochAusgedient && zweiteErnte == 1 && kp.zustand (s2) == BankZustand::frei;
            pruefe (ok, "ack_einer_alten_generation_gibt_keine_bank_frei (B-12)",
                    "alter ACK geerntet: " + std::to_string (ersteErnte) + " frei, eigener ACK: "
                    + std::to_string (zweiteErnte) + " frei");
        }

        // M-43: Reclaim erst NACH dem ACK.
        {
            pool.zuruecksetzen();
            const int slot = pool.reserviere();
            pool.bank (slot).programm.generation = pool.naechsteGeneration();
            pool.publiziere (Pfad::committed, slot);
            pool.uebernehmeBereiten (Pfad::committed);
            pool.beginneVerblassen (slot);
            pruefe (pool.ernteAcks() == 0 && pool.zustand (slot) == BankZustand::verblassend,
                    "reclaim_erst_nach_ack (M-43)",
                    "ohne ACK bleibt die Bank verblassend und wird nie frei");
            pool.meldeAusgedient (slot);
            pruefe (pool.ernteAcks() == 1, "nach dem ACK wird sie frei (M-43)");
        }

        // M-44: busy_retry, wenn kein Slot frei ist - und die Wiederholung
        // kommt nach der Freigabe durch.
        {
            pool.zuruecksetzen();
            int belegt = 0;
            for (int i = 0; i < DspBankPool::kBaenke; ++i)
                if (pool.reserviere() >= 0) ++belegt;
            pruefe (belegt == DspBankPool::kBaenke, "alle vier Baenke reservierbar (M-41)");
            pruefe (pool.reserviere() == -1, "busy_retry_wenn_kein_slot_frei (M-44)",
                    "der fuenfte Wunsch verdraengt keine aktive Bank");

            // Freigabe eines Slots -> der naechste Versuch kommt durch.
            pool.bank (0).programm.generation = pool.naechsteGeneration();
            pool.publiziere (Pfad::committed, 0);
            pool.uebernehmeBereiten (Pfad::committed);
            pool.beginneVerblassen (0);
            pool.meldeAusgedient (0);
            pool.ernteAcks();
            pruefe (pool.reserviere() == 0, "wiederholung_nach_bankfreigabe_gelingt (M-44)");
        }

        // M-45: der Ring FASST mehr Eintraege als es Slots gibt und kann im
        // regulaeren Betrieb strukturell nicht ueberlaufen - hoechstens vier
        // Baenke koennen gleichzeitig ausdienen. Der Overflow-Zweig ist damit
        // eine WACHE, und eine Wache, die strukturell 0 bleibt, braucht einen
        // Test (Pruefliste A). Gemessen wird deshalb die Kopplung: jeder
        // gezaehlte Ueberlauf hat GENAU EINE Verriegelung erzeugt. Eine
        // Fassung, die den Ueberlauf zaehlt, den Slot aber nicht haelt,
        // faellt daran - auch dann, wenn beide Zahlen 0 sind, sobald ein Lauf
        // sie auseinandertreibt.
        {
            pool.zuruecksetzen();
            for (int runde = 0; runde < 3 * DspBankPool::kAckKapazitaet; ++runde)
            {
                const int slot = pool.reserviere();
                if (slot < 0) { pool.ernteAcks(); continue; }
                pool.bank (slot).programm.generation = pool.naechsteGeneration();
                pool.publiziere (Pfad::committed, slot);
                pool.uebernehmeBereiten (Pfad::committed);
                pool.beginneVerblassen (slot);
                pool.meldeAusgedient (slot);
            }
            pruefe (pool.ackUeberlaeufe() == pool.reclaimVerriegelungen(),
                    "ueberlaeufe_und_verriegelungen_sind_gekoppelt (M-45)",
                    "Ueberlaeufe=" + std::to_string (pool.ackUeberlaeufe())
                    + ", Verriegelungen=" + std::to_string (pool.reclaimVerriegelungen()));
            pruefe (pool.ackUeberlaeufe() == 0,
                    "der Ring droppt bei regulaerem Betrieb nie (M-45)",
                    "der Ring fasst " + std::to_string (DspBankPool::kAckKapazitaet)
                    + " Eintraege bei " + std::to_string (DspBankPool::kBaenke) + " Slots");
        }

        // B-17 / M-45: ein ERZWUNGENER Ueberlauf an Maske und Slotzustand. Ein
        // Pool mit Ringkapazitaet 2 dient drei Baenke ohne Ernte aus: der
        // dritte ACK findet keinen Platz. Sein Slot muss verriegelt sein (Bit
        // gesetzt, Zustand ausgedient), die Ernte darf ihn nicht freigeben,
        // und erst die Bestaetigung des Workers gibt ihn frei und loescht das
        // Bit. Eine Bestaetigung an einem Slot OHNE Bit gibt nichts frei (E-23).
        {
            auto p = std::make_unique<DspBankPool> (2);
            auto ausdienen = [&]
            {
                const int s = p->reserviere();
                p->bank (s).programm.generation = p->naechsteGeneration();
                p->publiziere (Pfad::committed, s);
                p->uebernehmeBereiten (Pfad::committed);
                p->beginneVerblassen (s);
                p->meldeAusgedient (s);
                return s;
            };
            ausdienen();
            ausdienen();
            const int dritter = ausdienen();
            const std::uint64_t bit = (std::uint64_t) (1ull << dritter);

            const bool verriegelt = (p->reclaimPendingMask() & bit) != 0
                                    && p->zustand (dritter) == BankZustand::ausgedient
                                    && p->reclaimVerriegelungen() == 1;
            const int geerntet   = p->ernteAcks();
            const bool haelt     = p->zustand (dritter) == BankZustand::ausgedient;
            const bool bestaetigt = p->bestaetigeReclaim (dritter);
            const bool frei      = p->zustand (dritter) == BankZustand::frei && (p->reclaimPendingMask() & bit) == 0;
            pruefe (verriegelt && geerntet == 2 && haelt && bestaetigt && frei,
                    "erzwungener_ueberlauf_verriegelt_den_slot_bis_zur_bestaetigung (M-45, B-17)",
                    std::string ("verriegelt=") + (verriegelt ? "ja" : "nein") + ", geerntet="
                    + std::to_string (geerntet) + ", haelt=" + (haelt ? "ja" : "nein") + ", bestaetigt="
                    + (bestaetigt ? "ja" : "nein") + ", danach frei=" + (frei ? "ja" : "nein"));

            const int ohneBit = ausdienen();
            const bool keineFreigabe = ! p->bestaetigeReclaim (ohneBit) && p->zustand (ohneBit) == BankZustand::ausgedient;
            const int regulaer = p->ernteAcks();
            pruefe (keineFreigabe && regulaer == 1 && p->zustand (ohneBit) == BankZustand::frei,
                    "bestaetigung_ohne_verriegelung_gibt_nichts_frei (M-45, E-23)",
                    "der regulaere ACK gibt ihn frei");
        }

        // M-46: vier Baenke im schlimmsten Fall; Candidate endet neutral.
        {
            auto k = neuerKern (48000.0, 64);
            auto s = machSatz (true);
            belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 6.0);

            // D10 (Codeaudit 10.09.2026): der Fall ist der GLEICHZEITIGE
            // Topologiewechsel beider Pfade (§44.2). Bis dahin kamen die zwei
            // Wechsel, waehrend beide Pfade noch aus der Ruhe einblendeten; nach
            // E-17 warteten sie als `bereit`, und `freieSlots() == 0` zaehlte
            // sie als "vier lebende Baenke", waehrend zwei rechneten (Manifest
            // §11.7). Jetzt laufen beide Pfade zuerst eingeschwungen auf je
            // einer Bank, dann liegen beide rampeninkompatiblen Wuensche vor
            // DEMSELBEN Blockrand, und gemessen wird mitten im Crossfade.
            pruefe (k->uebernehmeZustand (s, Pfad::committed), "Committed nimmt eine Bank");
            pruefe (k->uebernehmeZustand (s, Pfad::candidate), "Candidate nimmt eine zweite Bank");
            fahreStille (*k, kFadeSamples, 64);   // beide Einblendungen aus der Ruhe laufen zu Ende
            k->pflege();
            auto s2 = s; belege (s2, 1, Filtertyp::bell, 2000.0, 1.0, -6.0);   // ein Band mehr: rampeninkompatibel
            auto s3 = s; belege (s3, 2, Filtertyp::notch, 500.0, 4.0, 0.0);
            pruefe (k->uebernehmeZustand (s2, Pfad::committed), "Committed wechselt (dritte Bank)");
            pruefe (k->uebernehmeZustand (s3, Pfad::candidate), "Candidate wechselt (vierte Bank)");
            fahreStille (*k, 64, 64);             // beide Pfade mitten im Crossfade

            int ck = -1, cq = -1, kk = -1, kq = -1;
            k->rechnendeSlots (Pfad::committed, ck, cq);
            k->rechnendeSlots (Pfad::candidate, kk, kq);
            const int ids[4] = { ck, cq, kk, kq };
            bool vier = true;
            for (int i = 0; i < 4; ++i)
            {
                vier = vier && ids[i] >= 0;
                for (int j = i + 1; j < 4; ++j) vier = vier && ids[i] != ids[j];
            }
            pruefe (vier && k->pool().freieSlots() == 0, "vier_baenke_im_schlimmsten_fall (M-46)",
                    "beide Pfade mitten im Crossfade: Committed rechnet " + std::to_string (ck) + " mit Quelle "
                    + std::to_string (cq) + ", Candidate " + std::to_string (kk) + " mit Quelle " + std::to_string (kq)
                    + ", freie Slots " + std::to_string (k->pool().freieSlots()));
            pruefe (! k->uebernehmeZustand (s, Pfad::committed),
                    "und der fuenfte Wunsch bekommt busy_retry (M-44/M-46)");

            k->beendeCandidate();
            pruefe (! k->candidateVorhanden(), "candidate_endet_neutral (M-46)");
        }
    }

    //==========================================================================
    std::cout << std::endl << "== J - Audiothread-Invarianten (M-47, M-48) ==" << std::endl;
    {
        auto kern = neuerKern (48000.0, 2048);
        auto s = machSatz (true);
        belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 6.0, Kanalmodus::mid);
        belege (s, 3, Filtertyp::lowShelf, 200.0, 0.707, -4.0);
        machDynamisch (s, 3, -6.0, -30.0);
        setzeGlobal (s, "v1.global.width", 1.4);
        setzeGlobal (s, "v1.global.mono_bass_hz", 120.0);
        setzeGlobal (s, "v2.global.mix", 0.7);
        setzeGlobalBool (s, "v2.global.auto_gain", true);
        // B-25 / M-47 / M-41: die Zaehler sind UEBER DEN GANZEN LAUF scharf -
        // ab dem ERSTEN Programmwechsel. Im Lauf liegen: Wechsel mit Crossfade,
        // reine Rampenwechsel, Ausschalten und Wiedereinschalten, Candidate-
        // Uebernahmen, Candidate-Wechsel und Candidate-Ende samt Fade, Ernte und
        // Reclaim, dazu die TRANSPORTKANTEN der Bibliothek (Entscheid E-26):
        // eine Aufrufpause, in der der Worker publiziert oder erntet, die Kante
        // Signal <-> Stille und der Wechsel der Blockgroesse zwischen 1,
        // Bloecken bis maxBlock und einem uebergrossen, stueckelnden Block.
        // Der Testzaehler zaehlt nur waehrend `verarbeite`; der Kernzaehler nur
        // im RtWache-Bereich - Worker-Aufrufe liegen ausserhalb beider.
        RtWache::zuruecksetzen();
        std::vector<float> a (4096), b (4096);
        float* kan[2] = { a.data(), b.data() };

        auto rampe = s;
        s.werte[(size_t) param::indexBandV1 (0, param::kGainDb)].zahl = 6.0;
        rampe.werte[(size_t) param::indexBandV1 (0, param::kGainDb)].zahl = 2.0;
        auto kand1 = s;
        setzeGlobal (kand1, "v1.global.output_trim_db", -3.0);
        belege (kand1, 5, Filtertyp::notch, 3000.0, 4.0, 0.0);
        auto kand2 = kand1;
        belege (kand2, 6, Filtertyp::highShelf, 6000.0, 0.707, -4.0, Kanalmodus::side);
        auto aus = s;
        aus.werte[(size_t) param::kIndexEqEnabled].b = false;
        // NAK-311 M-22: ein neutraler Schritt im Zyklus - danach laufen die
        // Bloecke bis zum naechsten Wechsel durch die Neutralpruefung, auch
        // die uebergrossen, stueckelnden.
        const auto neutral = machSatz (true);
        // NAK-311 M-56 (W03): ein Committed-Fremdslotwechsel im Zyklus - Slot 6
        // kommt zur Rampenbelegung hinzu; die Slots 0 und 3 bleiben
        // topologisch gleich und wandern im Crossfade in die neue Bank. Bei 140
        // geht Slot 6 wieder, zugleich mit einem Wertewechsel an Slot 0.
        auto fremd = rampe;
        belege (fremd, 6, Filtertyp::notch, 2500.0, 3.0, 0.0);

        const std::uint64_t uebernahmenVorher = kern->uebernahmen();
        std::uint64_t gesamt = 0, testAllokationen = 0;
        int geerntet = 0, busy = 0;
        kern->uebernehmeZustand (s);   // der ERSTE Programmwechsel - im Lauf
        for (int blk = 0; blk < 4000; ++blk)
        {
            bool ok = true;
            switch (blk % 400)
            {
                case  40: ok = kern->uebernehmeZustand (rampe); break;
                case  65: ok = kern->uebernehmeZustand (fremd); break;
                case  90: ok = kern->uebernehmeZustand (kand1, Pfad::candidate); break;
                case 140: ok = kern->uebernehmeZustand (s); break;
                case 190: ok = kern->uebernehmeZustand (kand2, Pfad::candidate); break;
                case 240: kern->beendeCandidate(); break;
                case 290: ok = kern->uebernehmeZustand (aus); break;
                case 340: ok = kern->uebernehmeZustand (s); break;
                case 370: ok = kern->uebernehmeZustand (neutral); break;
                default: break;
            }
            if (! ok) ++busy;
            if (blk % 20 == 0) geerntet += kern->pflege();

            int n = 1 + (blk * 61) % 2048;
            if (blk % 97 == 13) n = 1;
            if (blk % 97 == 14) n = 4096;
            const bool still = (blk / 50) % 7 == 3;
            for (int i = 0; i < n; ++i)
            {
                const double x = still ? 0.0 : 0.5 * std::sin (0.013 * (double) (blk * 2048 + i));
                a[(size_t) i] = (float) x;
                b[(size_t) i] = (float) (x * 0.8);
            }
            zaehleAllokationen = true;
            allokationen = 0;
            kern->verarbeite (kan, 2, n);
            zaehleAllokationen = false;
            testAllokationen += allokationen;
            gesamt += (std::uint64_t) n;
        }
        const std::uint64_t uebernahmen = kern->uebernahmen() - uebernahmenVorher;

        pruefe (testAllokationen == 0 && RtWache::allokationen() == 0 && uebernahmen >= 60 && geerntet >= 30,
                "null_allokationen_im_callback_samt_programmwechseln (M-41, M-47, B-25)",
                "4000 Bloecke, " + std::to_string (gesamt) + " Samples, " + std::to_string (uebernahmen)
                + " Blockrand-Uebernahmen, " + std::to_string (geerntet) + " Baenke geerntet, busy_retry "
                + std::to_string (busy) + ", Testzaehler " + std::to_string (testAllokationen)
                + ", Kernzaehler " + std::to_string (RtWache::allokationen()));
        pruefe (RtWache::sperren() == 0, "null_sperren_im_callback (M-47)",
                "Sperrenzaehler=" + std::to_string (RtWache::sperren()));

        // B-13: der Sperrzaehler ist VERDRAHTET. Lebendigkeit: eine Sperre ueber
        // den Wrapper zaehlt im Audiopfad genau einmal, ausserhalb gar nicht.
        {
            std::mutex m;
            RtWache::zuruecksetzen();
            { RtWache::GemeldeteSperre<std::mutex> ausserhalb (m); }
            const std::uint64_t ausserhalbGezaehlt = RtWache::sperren();
            { RtWache::Bereich audio; RtWache::GemeldeteSperre<std::mutex> innen (m); }
            pruefe (ausserhalbGezaehlt == 0 && RtWache::sperren() == 1,
                    "sperrzaehler_sieht_eine_sperre_im_audiopfad (M-47, B-13)",
                    "ausserhalb " + std::to_string (ausserhalbGezaehlt) + ", im Audiopfad "
                    + std::to_string (RtWache::sperren()));
            RtWache::zuruecksetzen();
        }

        // B-13: und VOLLSTAENDIG - kein Quelltext unter plugin/dsp nennt eine
        // Sperrklasse oder einen Spin am Wrapper vorbei. Fail-closed: fehlt das
        // Verzeichnis oder sind es weniger Dateien als der Kern hat, ist es rot.
        {
            namespace dateisystem = std::filesystem;
            const dateisystem::path dsp = dateisystem::path (__FILE__).parent_path().parent_path() / "dsp";
            const char* verboten[] = { "std::mutex", "std::recursive_mutex", "std::timed_mutex",
                                       "std::shared_mutex", "std::lock_guard", "std::unique_lock",
                                       "std::scoped_lock", "std::shared_lock", "std::condition_variable",
                                       "CriticalSection", "SpinLock", "atomic_flag", "pthread_mutex",
                                       "WaitForSingleObject", "SRWLOCK" };
            int dateien = 0;
            std::string treffer;
            std::error_code fehlerCode;
            if (dateisystem::is_directory (dsp, fehlerCode))
            {
                for (const auto& eintrag : dateisystem::directory_iterator (dsp, fehlerCode))
                {
                    if (! eintrag.is_regular_file()) continue;
                    std::ifstream ein (eintrag.path(), std::ios::binary);
                    const std::string text ((std::istreambuf_iterator<char> (ein)), std::istreambuf_iterator<char>());
                    ++dateien;
                    for (const char* v : verboten)
                        if (text.find (v) != std::string::npos)
                            treffer += eintrag.path().filename().string() + ": " + v + "; ";
                }
            }
            pruefe (dateien >= 9 && treffer.empty(),
                    "keine_sperre_am_wrapper_vorbei_im_kern (M-47, B-13)",
                    std::to_string (dateien) + " Dateien unter " + dsp.string()
                    + (treffer.empty() ? std::string (", kein Treffer") : ", Treffer: " + treffer));
        }

        // Die Gegenprobe: der WORKER darf allozieren, und der Zaehler des
        // Kerns sieht das NICHT - sonst waere er global statt am Audiopfad.
        {
            RtWache::zuruecksetzen();
            zaehleAllokationen = true;
            allokationen = 0;
            std::vector<double> ballast (4096, 1.0);
            kern->uebernehmeZustand (s);
            zaehleAllokationen = false;
            pruefe (allokationen > 0 && RtWache::allokationen() == 0,
                    "der_zaehler_trennt_worker_und_audiothread (M-47)",
                    "Testzaehler " + std::to_string (allokationen) + ", Kernzaehler "
                    + std::to_string (RtWache::allokationen()));
        }

        // M-48: Ueberlast verwirft die ANALYSE, nie Audio.
        {
            auto k = neuerKern (48000.0, 256);
            auto ein = machSatz (true);
            belege (ein, 0, Filtertyp::bell, 1000.0, 1.0, 6.0);
            k->uebernehmeZustand (ein);
            fahreStille (*k, 1024, 256);

            // B-22: der Referenzlauf - derselbe Kern, derselbe Zustand, derselbe
            // Eingang, aber in Bloecken bis maxBlock, also OHNE Tap-Ueberlast.
            auto referenz = neuerKern (48000.0, 256);
            referenz->uebernehmeZustand (ein);
            fahreStille (*referenz, 1024, 256);

            std::vector<float> gross (1024), grossR (1024);
            for (int i = 0; i < 1024; ++i)
            {
                gross[(size_t) i]  = (float) (0.3 * std::sin (0.021 * (double) i));
                grossR[(size_t) i] = (float) (0.2 * std::cos (0.017 * (double) i));
            }
            std::vector<float> refL = gross, refR = grossR;
            float* kanG[2] = { gross.data(), grossR.data() };
            const auto verworfenVorher = k->verworfeneAnalyseframes();
            k->verarbeite (kanG, 2, 1024);

            for (int versatz = 0; versatz < 1024; versatz += 256)
            {
                float* teil[2] = { refL.data() + versatz, refR.data() + versatz };
                referenz->verarbeite (teil, 2, 256);
            }
            const bool gleich = std::memcmp (gross.data(), refL.data(), 1024 * sizeof (float)) == 0
                             && std::memcmp (grossR.data(), refR.data(), 1024 * sizeof (float)) == 0;
            bool veraendert = false;
            for (int i = 0; i < 1024; ++i)
                if (gross[(size_t) i] != (float) (0.3 * std::sin (0.021 * (double) i))) veraendert = true;

            pruefe (k->verworfeneAnalyseframes() == verworfenVorher + 1 && k->tapLaenge() == 0,
                    "ueberlast_verwirft_analyse (M-48)",
                    "Block 1024 > maxBlock 256: Tap verworfen und gezaehlt");
            pruefe (gleich && veraendert,
                    "der_uebergrosse_block_gleicht_sample_exakt_dem_lauf_ohne_ueberlast (M-48, B-22)",
                    "alle 1024 Samples beider Kanaele bitgleich zum Referenzlauf in 4 x 256");
        }

        // B-11 / M-25: die Uebernahme laeuft genau EINMAL je aeusserem Aufruf.
        // Ein Testhaken publiziert ein neues Programm NACH dem ersten von vier
        // Teilstuecken (1024 Samples, maxBlock 256): in diesem Aufruf darf es
        // nicht wirken - der Ausgang bleibt ueber alle 1024 Samples konstant -,
        // erst der naechste Aufruf nimmt es.
        {
            auto k = neuerKern (48000.0, 256);
            auto ein = machSatz (true);
            belege (ein, 0, Filtertyp::lowShelf, 8000.0, 0.707, 9.0);
            k->uebernehmeZustand (ein);
            fahreDc (*k, 0.3, 4096, 256);
            k->pflege();

            auto anders = machSatz (true);
            belege (anders, 0, Filtertyp::lowShelf, 8000.0, 0.707, -9.0);
            struct Haken { DspKern* kern; const param::DspSatz* satz; int aufrufe; };
            Haken haken { k.get(), &anders, 0 };
            k->setzeTeilstueckHaken ([] (void* c)
            {
                auto* h = static_cast<Haken*> (c);
                if (h->aufrufe++ == 0) h->kern->uebernehmeZustand (*h->satz);
            }, &haken);

            const std::uint64_t vorher = k->uebernahmen();
            const auto block = fahreDc (*k, 0.3, 1024, 1024);
            const std::uint64_t nachErstem = k->uebernahmen();
            double abweichung = 0.0;
            for (double x : block) abweichung = std::max (abweichung, std::abs (x - block[0]));
            k->setzeTeilstueckHaken (nullptr, nullptr);

            const auto naechster = fahreDc (*k, 0.3, 1024, 1024);
            const double ziel = 0.3 * std::pow (10.0, -9.0 / 20.0);
            pruefe (haken.aufrufe == 3 && nachErstem == vorher && abweichung < 1e-6
                    && k->uebernahmen() == vorher + 1 && std::abs (naechster.back() - ziel) < 1e-4,
                    "uebernahme_nur_am_aeusseren_blockrand (M-25, R9, B-11)",
                    "Haken " + std::to_string (haken.aufrufe) + " mal, Uebernahmen im Aufruf "
                    + std::to_string (nachErstem - vorher) + ", groesste Abweichung im Block "
                    + zahl (abweichung, 9) + ", naechster Aufruf endet bei " + zahl (naechster.back(), 6));
        }

        // Der Denormal-Riegel (Entscheid E-14, Selbstaudit des Auftrags).
        // Ein abklingender Filterzustand laeuft nach genuegend Stille in den
        // denormalen Bereich; auf x86 kostet jede Rechnung damit ein
        // Vielfaches. Gemessen wird, DASS der Riegel greift - ein Zaehler,
        // der strukturell 0 bliebe, waere eine stille Wache (Pruefliste A).
        {
            auto k = neuerKern (48000.0, 512);
            auto ein = machSatz (true);
            belege (ein, 0, Filtertyp::bell, 1000.0, 1.0, 12.0);
            k->uebernehmeZustand (ein);
            fahreStille (*k, 2048, 512);

            // Ein kraeftiger Impuls, dann lange Stille: der Zustand faellt
            // exponentiell und passiert dabei den denormalen Bereich.
            std::vector<float> a (512, 0.0f), b (512, 0.0f);
            a[0] = 1.0f; b[0] = 1.0f;
            float* kan[2] = { a.data(), b.data() };
            k->verarbeite (kan, 2, 512);
            fahreStille (*k, 200000, 512);

            pruefe (k->geriegelteDenormale() > 0,
                    "denormale_filterzustaende_werden_am_blockrand_geriegelt (E-14)",
                    "Riegel griff " + std::to_string (k->geriegelteDenormale()) + " mal");

            // Und er aendert nichts Hoerbares: nach dem Riegel ist der
            // Zustand 0, und bei Stille bleibt der Ausgang 0.
            std::vector<float> c (512, 0.0f), d (512, 0.0f);
            float* kanC[2] = { c.data(), d.data() };
            k->verarbeite (kanC, 2, 512);
            double maxAus = 0.0;
            for (int i = 0; i < 512; ++i) maxAus = std::max (maxAus, std::abs ((double) c[(size_t) i]));
            pruefe (maxAus == 0.0, "der_denormal_riegel_aendert_nichts_hoerbares (E-14)",
                    "max " + zahl (maxAus, 12));
        }
    }

    //==========================================================================
    std::cout << std::endl << "== K - Nicht-Endlich-Riegel (M-49, M-113) ==" << std::endl;
    {
        auto kern = neuerKern (48000.0, 512);
        auto s = machSatz (true);
        belege (s, 0, Filtertyp::bell, 1000.0, 12.0, 12.0);
        // Ein DYNAMISCHES Band auf einem zweiten Slot: nur dann wird
        // ueberhaupt eine Auslenkung gerechnet, und nur dann ist der Riegel
        // aus M-113 gepruefter Weg statt ungepruefte Wache.
        belege (s, 4, Filtertyp::bell, 2000.0, 2.0, 0.0);
        machDynamisch (s, 4, -6.0, -40.0, 1.0, 0.0, 20.0);
        kern->uebernehmeZustand (s);
        fahreStille (*kern, 1024, 512);

        std::vector<float> a (512, 0.0f), b (512, 0.0f);
        a[10] = std::numeric_limits<float>::quiet_NaN();
        a[20] = std::numeric_limits<float>::infinity();
        b[30] = -std::numeric_limits<float>::infinity();
        float* kan[2] = { a.data(), b.data() };
        kern->verarbeite (kan, 2, 512);

        pruefe (kern->nichtEndlicheEingaenge() == 3,
                "nichtendlicher_eingang_wird_verriegelt_und_gezaehlt (M-49)",
                "Zaehler=" + std::to_string (kern->nichtEndlicheEingaenge()));

        bool ausgangEndlich = true;
        for (int i = 0; i < 512; ++i)
            if (! std::isfinite (a[(size_t) i]) || ! std::isfinite (b[(size_t) i])) ausgangEndlich = false;
        pruefe (ausgangEndlich, "kein_nan_vergiftet_den_filterzustand (M-49)");

        // Und der Zustand bleibt dauerhaft endlich: 100 weitere Bloecke.
        for (int blk = 0; blk < 100; ++blk)
        {
            for (int i = 0; i < 512; ++i)
            { a[(size_t) i] = (float) (0.2 * std::sin (0.03 * (double) (blk * 512 + i)));
              b[(size_t) i] = a[(size_t) i]; }
            kern->verarbeite (kan, 2, 512);
            for (int i = 0; i < 512; ++i)
                if (! std::isfinite (a[(size_t) i])) ausgangEndlich = false;
        }
        pruefe (ausgangEndlich, "der_pfad_bleibt_dauerhaft_endlich (M-49)",
                "100 Bloecke nach dem NaN");

        // B-20 / M-49: der ZWEITE Riegel, eigenstaendig vom Eingangsriegel.
        // Zwei Filterzustaende einer aktiven Bank werden gezielt nicht-endlich
        // gesetzt; der naechste Blockrand nullt sie, BEVOR ein Sample sie liest,
        // der Zaehler steigt um genau zwei, der Ausgang bleibt endlich, und der
        // Eingangszaehler bleibt stehen - es kam kein NaN ueber den Eingang.
        {
            auto k2 = neuerKern (48000.0, 512);
            auto s2 = machSatz (true);
            belege (s2, 0, Filtertyp::bell, 1000.0, 2.0, 6.0);
            belege (s2, 2, Filtertyp::lowShelf, 200.0, 0.707, 4.0);
            k2->uebernehmeZustand (s2);
            fahreStille (*k2, 2048, 512);
            int aktiv = -1, q1 = -1, q2 = -1, q3 = -1;
            k2->gefahreneSlots (aktiv, q1, q2, q3);
            if (aktiv >= 0)
            {
                auto& bank = k2->pool().bank (aktiv);
                bank.baender[0].statisch[0].z1 = std::numeric_limits<double>::quiet_NaN();
                bank.baender[2].statisch[1].z2 = std::numeric_limits<double>::infinity();
            }
            const auto geheiltVorher   = k2->geheilteFilterzustaende();
            const auto eingaengeVorher = k2->nichtEndlicheEingaenge();
            std::vector<float> e (512), f (512);
            for (int i = 0; i < 512; ++i)
            { e[(size_t) i] = (float) (0.3 * std::sin (0.02 * (double) i)); f[(size_t) i] = e[(size_t) i]; }
            float* kanE[2] = { e.data(), f.data() };
            k2->verarbeite (kanE, 2, 512);
            bool endlich = true;
            for (int i = 0; i < 512; ++i)
                if (! std::isfinite (e[(size_t) i]) || ! std::isfinite (f[(size_t) i])) endlich = false;
            pruefe (aktiv >= 0 && endlich && k2->geheilteFilterzustaende() == geheiltVorher + 2
                    && k2->nichtEndlicheEingaenge() == eingaengeVorher,
                    "nichtendlicher_filterzustand_wird_am_blockrand_geheilt (M-49, B-20)",
                    "geheilt +" + std::to_string (k2->geheilteFilterzustaende() - geheiltVorher)
                    + ", Eingangszaehler +" + std::to_string (k2->nichtEndlicheEingaenge() - eingaengeVorher)
                    + ", Ausgang endlich: " + (endlich ? "ja" : "nein"));
        }

        // M-113: eine nicht-endliche Auslenkung wird 0 und gezaehlt - der
        // Wert verlaesst den Kern nie nicht-endlich.
        {
            // Der Detektor muss arbeiten, sonst bleibt `auslenkungDb` auf 0
            // und der Riegel ist ungeprueft.
            std::vector<float> c (512), d (512);
            float* kanC[2] = { c.data(), d.data() };
            for (int blk = 0; blk < 80; ++blk)
            {
                for (int i = 0; i < 512; ++i)
                { const double x = 0.9 * std::sin (2.0 * kPiRef * 2000.0 * (double) (blk * 512 + i) / 48000.0);
                  c[(size_t) i] = (float) x; d[(size_t) i] = (float) x; }
                kern->verarbeite (kanC, 2, 512);
            }

            double werte[param::kSlots];
            kern->auslenkungenDb (werte);
            bool alleEndlich = true;
            for (int i = 0; i < param::kSlots; ++i)
                if (! std::isfinite (werte[i])) alleEndlich = false;

            // Die Kopplung: JEDER Wert ist endlich, UND der Zaehler nennt
            // genau die Faelle, in denen der Riegel gegriffen hat. Im
            // regulaeren Betrieb kann kein nicht-endlicher Wert entstehen -
            // die Pruefung ist damit eine Wache, und eine Wache, die
            // strukturell 0 bleibt, braucht einen Test (Pruefliste A). Der
            // Rotbeweis stellt den Fall her und nimmt den Riegel weg.
            pruefe (alleEndlich && kern->nichtEndlicheAuslenkungen() == 0,
                    "im_regulaeren_betrieb_ist_jede_auslenkung_endlich (M-113)",
                    "Riegel griff " + std::to_string (kern->nichtEndlicheAuslenkungen()) + " mal");

            // B-21: der Fehlerfall EXAKT. Vorbedingung (keine Produktmutation):
            // die Kennlinie des dynamischen Slots 4 bekommt einen NaN als Range,
            // der Pegel liegt ueber dem Threshold - die gerechnete Auslenkung ist
            // damit in jedem Block nicht-endlich. Gemeldet werden muss exakt 0,0,
            // und der Zaehler steigt um genau einen je Block.
            int aktiv = -1, q1 = -1, q2 = -1, q3 = -1;
            kern->gefahreneSlots (aktiv, q1, q2, q3);
            if (aktiv >= 0)
                kern->pool().bank (aktiv).programm.baender[4].rangeDb = std::numeric_limits<double>::quiet_NaN();
            const std::uint64_t vorher = kern->nichtEndlicheAuslenkungen();
            const int bloecke = 10;
            for (int blk = 0; blk < bloecke; ++blk)
            {
                for (int i = 0; i < 512; ++i)
                { const double x = 0.9 * std::sin (2.0 * kPiRef * 2000.0 * (double) (blk * 512 + i) / 48000.0);
                  c[(size_t) i] = (float) x; d[(size_t) i] = (float) x; }
                kern->verarbeite (kanC, 2, 512);
            }
            double nachher[param::kSlots];
            kern->auslenkungenDb (nachher);
            pruefe (aktiv >= 0 && nachher[4] == 0.0 && ! std::signbit (nachher[4])
                    && kern->nichtEndlicheAuslenkungen() == vorher + (std::uint64_t) bloecke,
                    "nichtendliche_auslenkung_wird_null_und_gezaehlt (M-113, B-21)",
                    "gemeldet " + zahl (nachher[4], 15) + ", Zaehler +"
                    + std::to_string (kern->nichtEndlicheAuslenkungen() - vorher) + " bei "
                    + std::to_string (bloecke) + " Bloecken");
        }
    }

    // NAK-283 F12 (R-283-6, M-39 und M-42 Haelfte a): die float-Kante des
    // Ausgangs. Ein endlicher Eingang, der erst bei der Verengung auf float
    // nicht endlich wuerde, wird verriegelt und im Eingangszaehler gezaehlt;
    // endliches Material in ±1,0 beruehrt der Riegel an keinem Bit.
    {
        // M-39: Output-Trim +6 dB, Eingang abwechselnd 0,75 und 0,25 x FLT_MAX.
        auto k = neuerKern (48000.0, 512);
        auto s = machSatz (true);
        setzeGlobal (s, "v1.global.output_trim_db", 6.0);
        k->uebernehmeZustand (s);
        fahreStille (*k, 2048, 512);   // Engagier-Fade und Trimrampe sind vorbei

        const float fmax = std::numeric_limits<float>::max();
        std::vector<float> a (512), b (512);
        for (int i = 0; i < 512; ++i)
        {
            a[(size_t) i] = (i % 2 == 0) ? 0.75f * fmax : 0.25f * fmax;
            b[(size_t) i] = (i % 3 == 0) ? -a[(size_t) i] : a[(size_t) i];
        }
        const auto vorher = k->nichtEndlicheEingaenge();
        float* kan[2] = { a.data(), b.data() };
        k->verarbeite (kan, 2, 512);

        // Die Erwartung kommt aus dem double-Tap `post_committed`, nicht aus
        // dem Ausgang: welcher Wert wuerde bei der Verengung nicht endlich?
        const double* tl = k->tap (Tap::postCommitted, 0);
        const double* tr = k->tap (Tap::postCommitted, 1);
        std::uint64_t betroffen = 0, abweichend = 0, nichtEndlich = 0;
        const bool tapsDa = tl != nullptr && tr != nullptr;
        for (int i = 0; i < 512 && tapsDa; ++i)
        {
            const double taps[2] = { tl[i], tr[i] };
            const float  aus[2]  = { a[(size_t) i], b[(size_t) i] };
            for (int kanal = 0; kanal < 2; ++kanal)
            {
                const float roh = (float) taps[kanal];
                const bool ueberRand = std::isfinite (taps[kanal]) && ! std::isfinite (roh);
                if (ueberRand) ++betroffen;
                const float soll = ueberRand ? 0.0f : roh;
                if (std::memcmp (&aus[kanal], &soll, sizeof (float)) != 0) ++abweichend;
                if (! std::isfinite (aus[kanal])) ++nichtEndlich;
            }
        }
        pruefe (tapsDa && betroffen == 512 && nichtEndlich == 0 && abweichend == 0
                    && k->nichtEndlicheEingaenge() == vorher + betroffen,
                "endlicher_eingang_erzeugt_keinen_nichtendlichen_floatausgang (M-39, R-283-6)",
                "ueber dem float-Rand " + std::to_string (betroffen) + " von 1024 Werten, am Ausgang nicht endlich "
                + std::to_string (nichtEndlich) + ", abweichend von der Erwartung " + std::to_string (abweichend)
                + ", Zaehler +" + std::to_string (k->nichtEndlicheEingaenge() - vorher));
    }
    {
        // M-42 Haelfte a (Regressionswache): Bell +9 dB und Output-Trim +6 dB
        // auf Rauschen in ±1,0 - jeder geschriebene float ist bitgleich die
        // Verengung des double-Taps, und der Zaehler bleibt stehen.
        auto k = neuerKern (48000.0, 512);
        auto s = machSatz (true);
        belege (s, 0, Filtertyp::bell, 1000.0, 2.0, 9.0);
        setzeGlobal (s, "v1.global.output_trim_db", 6.0);
        k->uebernehmeZustand (s);
        fahreStille (*k, 2048, 512);

        std::uint32_t saat = 0x4d2a91u;
        const auto zufallswert = [&saat]
        {
            saat = saat * 1664525u + 1013904223u;
            return ((float) (saat >> 8) / (float) 0x00ffffff) * 2.0f - 1.0f;
        };
        const auto vorher = k->nichtEndlicheEingaenge();
        std::vector<float> a (512), b (512);
        float* kan[2] = { a.data(), b.data() };
        std::uint64_t verglichen = 0, abweichend = 0, ueberEins = 0;
        bool tapsDa = true;
        for (int blk = 0; blk < 200 && tapsDa; ++blk)
        {
            for (int i = 0; i < 512; ++i) { a[(size_t) i] = zufallswert(); b[(size_t) i] = zufallswert(); }
            k->verarbeite (kan, 2, 512);
            const double* tl = k->tap (Tap::postCommitted, 0);
            const double* tr = k->tap (Tap::postCommitted, 1);
            if (tl == nullptr || tr == nullptr) { tapsDa = false; break; }
            for (int i = 0; i < 512; ++i)
            {
                const float sl = (float) tl[i], sr = (float) tr[i];
                if (std::memcmp (&a[(size_t) i], &sl, sizeof (float)) != 0) ++abweichend;
                if (std::memcmp (&b[(size_t) i], &sr, sizeof (float)) != 0) ++abweichend;
                if (std::abs (sl) >= 1.0f) ++ueberEins;
                if (std::abs (sr) >= 1.0f) ++ueberEins;
                verglichen += 2;
            }
        }
        pruefe (tapsDa && abweichend == 0 && verglichen == 200u * 512u * 2u && ueberEins > 0
                    && k->nichtEndlicheEingaenge() == vorher,
                "ausgangsriegel_beruehrt_endliches_material_nicht (M-42 Haelfte a, Regressionswache)",
                std::to_string (verglichen) + " Werte verglichen, davon nicht bitgleich zur Verengung des Taps "
                + std::to_string (abweichend) + ", mit Betrag >= 1 " + std::to_string (ueberEins)
                + ", Zaehler +" + std::to_string (k->nichtEndlicheEingaenge() - vorher));
    }

    //==========================================================================
    std::cout << std::endl << "== L - Hoermatrix und Taps (M-52 bis M-57, M-114) ==" << std::endl;
    {
        const double fs = 48000.0;
        auto kern = neuerKern (fs, 512);

        pruefe (kern->wirksameHoermatrix() == Hoermatrix::processed,
                "nach_dem_anlegen_immer_processed (M-52)");

        auto s = machSatz (true);
        belege (s, 0, Filtertyp::bell, 1000.0, 1.0, 9.0);
        kern->uebernehmeZustand (s);
        fahreStille (*kern, 2048, 512);

        // M-53: Dry ist BITIDENTISCH zum Eingang.
        {
            kern->setzeHoermatrix (Hoermatrix::dry);
            // Der Wechsel der Hoermatrix hat seit M-55 einen eigenen Fade -
            // die Zusage "bitidentisch" gilt NACH ihm, wie bei jedem anderen
            // Uebergang des Kerns auch.
            fahreStille (*kern, kFadeSamples + 512, 512);
            std::vector<float> a (512), b (512), aK, bK;
            bool bitgleich = true;
            for (int blk = 0; blk < 8; ++blk)
            {
                for (int i = 0; i < 512; ++i)
                { a[(size_t) i] = (float) (0.4 * std::sin (0.021 * (double) (blk * 512 + i)));
                  b[(size_t) i] = (float) (0.4 * std::cos (0.017 * (double) (blk * 512 + i))); }
                aK = a; bK = b;
                float* kan[2] = { a.data(), b.data() };
                kern->verarbeite (kan, 2, 512);
                for (int i = 0; i < 512; ++i)
                    if (std::memcmp (&a[(size_t) i], &aK[(size_t) i], sizeof (float)) != 0
                     || std::memcmp (&b[(size_t) i], &bK[(size_t) i], sizeof (float)) != 0)
                        bitgleich = false;
            }
            pruefe (bitgleich, "hoermatrix_dry_ist_bitidentisch (M-53)");
        }

        // M-54: Delta bei Gleichheit ist EXAKT 0.
        {
            auto k = neuerKern (fs, 512);
            k->uebernehmeZustand (machSatz (true));   // neutral: Processed == Dry
            fahreStille (*k, 2048, 512);
            k->setzeHoermatrix (Hoermatrix::delta);
            fahreStille (*k, kFadeSamples + 512, 512);   // den Uebergang aus M-55 abwarten

            std::vector<float> a (512), b (512);
            for (int i = 0; i < 512; ++i)
            { a[(size_t) i] = (float) (0.5 * std::sin (0.03 * (double) i)); b[(size_t) i] = a[(size_t) i]; }
            float* kan[2] = { a.data(), b.data() };
            k->verarbeite (kan, 2, 512);
            bool exaktNull = true;
            for (int i = 0; i < 512; ++i) if (a[(size_t) i] != 0.0f || b[(size_t) i] != 0.0f) exaktNull = false;
            pruefe (exaktNull, "delta_bei_gleichheit_ist_exakt_null (M-54)");
        }

        // NAK-311 311/M-18 (T3-01-01, §9 F-4): im engagiert-neutralen Kern
        // lassen nur Processed und Dry ohne laufenden Hoermatrix-Fade den Puffer
        // unberuehrt. Delta schreibt (Processed - Dry) * Makeup - bei endlichem
        // Eingang +0,0, an der Wachmarke den ungezaehmten Dry-Anteil -,
        // Candidate schreibt den Candidate-Ausgang, und der Fade der Hoermatrix
        // schreibt bis zu seinem Ende, auch mitten im Teilstueck.
        {
            const float wachmarke = ausBitmuster (0x7F800001u);   // NAK-311: nie konstantgefaltet
            const auto material = [&] (std::vector<float>& l, std::vector<float>& r)
            {
                for (int i = 0; i < 512; ++i)
                {
                    l[(size_t) i] = (float) (0.4 * std::sin (0.021 * (double) i));
                    r[(size_t) i] = (float) (0.3 * std::cos (0.017 * (double) i));
                }
                l[100] = wachmarke; r[300] = wachmarke;
            };
            const auto istWachmarke = [&] (float f) { return std::memcmp (&f, &wachmarke, sizeof (float)) == 0; };
            const Hoermatrix wahl[] = { Hoermatrix::processed, Hoermatrix::dry, Hoermatrix::delta, Hoermatrix::candidate };
            const char* namen[] = { "processed", "dry", "delta", "candidate" };
            for (int f = 0; f < 4; ++f)
            {
                auto k = neuerKern (fs, 512);
                k->uebernehmeZustand (machSatz (true));
                if (wahl[f] == Hoermatrix::candidate)
                {
                    auto kand = machSatz (true);
                    belege (kand, 0, Filtertyp::bell, 1000.0, 1.0, 12.0);   // ein hoerbares Candidate-Programm
                    k->uebernehmeZustand (kand, Pfad::candidate);
                }
                fahreStille (*k, kFadeSamples + kRampeSamples + 2048, 512);
                k->setzeHoermatrix (wahl[f]);
                fahreStille (*k, kFadeSamples + 512, 512);   // den Fade der Hoermatrix abwarten (M-55)

                std::vector<float> l (512), r (512);
                material (l, r);
                const auto lK = l, rK = r;
                float* kan[2] = { l.data(), r.data() };
                k->verarbeite (kan, 2, 512);

                int geaendert = 0, deltaNichtNull = 0;
                for (int i = 0; i < 512; ++i)
                {
                    const bool anders = std::memcmp (&l[(size_t) i], &lK[(size_t) i], sizeof (float)) != 0
                                     || std::memcmp (&r[(size_t) i], &rK[(size_t) i], sizeof (float)) != 0;
                    if (anders) ++geaendert;
                    const std::uint32_t nullBits = 0;
                    if (i != 100 && std::memcmp (&l[(size_t) i], &nullBits, sizeof (float)) != 0) ++deltaNichtNull;
                    if (i != 300 && std::memcmp (&r[(size_t) i], &nullBits, sizeof (float)) != 0) ++deltaNichtNull;
                }
                const bool wachmarkenBleiben = istWachmarke (l[100]) && istWachmarke (r[300]);
                const bool wirksam = k->wirksameHoermatrix() == wahl[f];
                bool ok = wirksam;
                std::string was;
                if (wahl[f] == Hoermatrix::processed || wahl[f] == Hoermatrix::dry)
                {
                    ok = ok && geaendert == 0;
                    was = "laesst_den_puffer_unberuehrt";
                }
                else if (wahl[f] == Hoermatrix::delta)
                {
                    ok = ok && geaendert > 0 && deltaNichtNull == 0 && std::isnan (l[100]) && std::isnan (r[300])
                            && ! istWachmarke (l[100]) && ! istWachmarke (r[300]);
                    was = "schreibt_plus_null_und_den_ungezaehmten_dry_anteil";
                }
                else
                {
                    ok = ok && geaendert > 0 && std::isfinite (l[100]) && std::isfinite (r[300]);
                    was = "schreibt_den_candidate_ausgang";
                }
                pruefe (ok, std::string ("311/M-18 neutral_mit_delta_und_candidate_schreibt: ") + namen[f] + " " + was,
                        std::string ("wirksam ") + (wirksam ? "ja" : "nein") + ", geaenderte Samples "
                        + std::to_string (geaendert) + " von 512, Wachmarken bytegleich " + (wachmarkenBleiben ? "ja" : "nein")
                        + (wahl[f] == Hoermatrix::delta ? ", Delta ungleich +0,0 an " + std::to_string (deltaNichtNull) + " endlichen Stellen" : ""));
            }

            // Der Fade der Hoermatrix (Processed -> Dry) schreibt bis zu seinem
            // Ende, ab dem ersten Sample danach nicht mehr - im selben Stueck.
            {
                auto k = neuerKern (fs, 512);
                k->uebernehmeZustand (machSatz (true));
                fahreStille (*k, kFadeSamples + kRampeSamples + 2048, 512);
                k->setzeHoermatrix (Hoermatrix::dry);
                std::vector<float> l (512), r (512);
                material (l, r);
                const auto lK = l, rK = r;
                float* kan[2] = { l.data(), r.data() };
                k->verarbeite (kan, 2, 512);
                int hinten = 0;
                for (int i = kFadeSamples; i < 512; ++i)
                    if (std::memcmp (&l[(size_t) i], &lK[(size_t) i], sizeof (float)) != 0
                     || std::memcmp (&r[(size_t) i], &rK[(size_t) i], sizeof (float)) != 0)
                        ++hinten;
                const bool fadeSchreibt = ! istWachmarke (l[100]);
                pruefe (fadeSchreibt && hinten == 0 && istWachmarke (r[300]),
                        "311/M-18 neutral_mit_delta_und_candidate_schreibt: der hoermatrix_fade_schreibt_bis_zu_seinem_ende",
                        std::string ("Wachmarke an 100 im Fade geschrieben: ") + (fadeSchreibt ? "ja" : "nein")
                        + ", geaenderte Samples ab 256: " + std::to_string (hinten) + " von 256");
            }
        }

        // M-54: der Abgleich ist FEST, nicht materialabhaengig.
        {
            auto k = neuerKern (fs, 512);
            auto ein = machSatz (true);
            setzeGlobal (ein, "v1.global.output_trim_db", 6.0);
            k->uebernehmeZustand (ein);
            fahreStille (*k, kRampeSamples + 4096, 512);
            k->setzeHoermatrix (Hoermatrix::delta);
            fahreStille (*k, kFadeSamples + 512, 512);

            auto miss = [&] (double amplitude)
            {
                std::vector<float> a (512), b (512);
                for (int i = 0; i < 512; ++i)
                { a[(size_t) i] = (float) amplitude; b[(size_t) i] = (float) amplitude; }
                float* kan[2] = { a.data(), b.data() };
                k->verarbeite (kan, 2, 512);
                return (double) a[500] / amplitude;
            };
            const double v1 = miss (0.1), v2 = miss (0.5);
            // B-23: gegen den UNABHAENGIG gerechneten Sollwert. Output-Trim
            // +6 dB macht Processed = Dry * 10^(6/20); Delta ist
            // (Processed - Dry) * 10^(12/20). Ein stummes, vertauschtes oder
            // falsch skaliertes Delta faellt - und beide Amplituden treffen
            // denselben Faktor, der Abgleich folgt dem Material nicht.
            const double soll = (std::pow (10.0, 6.0 / 20.0) - 1.0) * std::pow (10.0, 12.0 / 20.0);
            pruefe (std::abs (v1 - soll) < 1e-5 * soll && std::abs (v2 - soll) < 1e-5 * soll,
                    "delta_ist_differenz_mit_festem_abgleich (M-54, B-23)",
                    "Verhaeltnis bei 0,1 und 0,5: " + zahl (v1, 6) + " / " + zahl (v2, 6)
                    + " gegen Soll " + zahl (soll, 6));
        }

        // M-55: der Wechsel zwischen zwei Hoermatrix-Zustaenden ist
        // klickfrei, und ein A/B-Wechsel zwischen zwei IDENTISCHEN Zustaenden
        // nullt innerhalb numerischer Toleranz.
        {
            // (a) Klickfrei: gemessen an einem Kern, bei dem Processed und
            //     Dry WIRKLICH verschieden sind. An einem neutralen Kern ist
            //     jeder Uebergang spurlos, und die Zusage bliebe ungeprueft.
            auto k = neuerKern (fs, 256);
            auto laut = machSatz (true);
            setzeGlobal (laut, "v1.global.output_trim_db", 12.0);
            k->uebernehmeZustand (laut);
            fahreStille (*k, kRampeSamples + 4096, 256);

            std::vector<float> a (256), b (256);
            float* kan[2] = { a.data(), b.data() };
            auto fuelleDc = [&] { std::fill (a.begin(), a.end(), 0.2f); std::fill (b.begin(), b.end(), 0.2f); };

            fuelleDc(); k->verarbeite (kan, 2, 256);
            const double letzter = a[255];
            k->setzeHoermatrix (Hoermatrix::dry);
            fuelleDc(); k->verarbeite (kan, 2, 256);

            double maxSprung = std::abs ((double) a[0] - letzter), spanne = 0.0;
            for (int i = 1; i < 256; ++i)
            {
                maxSprung = std::max (maxSprung, std::abs ((double) a[(size_t) i] - (double) a[(size_t) (i - 1)]));
                spanne    = std::max (spanne, std::abs ((double) a[(size_t) i] - letzter));
            }
            pruefe (maxSprung <= 4.0 * spanne / (double) kFadeSamples + 1e-9,
                    "hoermatrixwechsel_ist_klickfrei (M-55)",
                    "max Sprung " + zahl (maxSprung, 8) + " gegen Fadeschritt "
                    + zahl (spanne / (double) kFadeSamples, 8));

            // (b) Und ein A/B-Wechsel zwischen zwei IDENTISCHEN Zustaenden
            //     nullt: an einem neutralen Kern ist Dry gleich Processed.
            auto neutral = neuerKern (fs, 256);
            neutral->uebernehmeZustand (machSatz (true));
            fahreStille (*neutral, 2048, 256);
            std::vector<float> c (256), d (256);
            float* kanN[2] = { c.data(), d.data() };
            auto fuelleTon = [&] (long long v) {
                for (int i = 0; i < 256; ++i)
                { const double x = 0.5 * std::sin (2.0 * kPiRef * 400.0 * (double) (v + i) / fs);
                  c[(size_t) i] = (float) x; d[(size_t) i] = (float) x; } };
            fuelleTon (0); neutral->verarbeite (kanN, 2, 256);
            neutral->setzeHoermatrix (Hoermatrix::dry);
            double maxAbw = 0.0;
            for (int blk = 1; blk <= 4; ++blk)
            {
                fuelleTon ((long long) blk * 256);
                neutral->verarbeite (kanN, 2, 256);
                for (int i = 0; i < 256; ++i)
                {
                    const double soll = 0.5 * std::sin (2.0 * kPiRef * 400.0 * (double) (blk * 256 + i) / fs);
                    maxAbw = std::max (maxAbw, std::abs ((double) c[(size_t) i] - soll));
                }
            }
            pruefe (maxAbw < 1e-6, "identische_zustaende_nullen_beim_wechsel (M-55)",
                    "groesste Abweichung zum Eingang " + zahl (maxAbw, 9));
        }

        // E-30 / M-55: ein Wechsel zu einem DRITTEN Hoermatrix-Zustand waehrend
        // eines laufenden Hoermatrix-Fades wartet dessen Ende ab. Processed
        // (+6 dB) -> Dry laeuft 64 Samples, dann Delta: der Fade zu Dry endet
        // bei Sample 256, erst dann blendet Delta ein - kein Sprung am
        // Umschaltsample, keiner im ganzen Lauf ueber die Fadeschrittweite.
        {
            auto k = neuerKern (fs, 64);
            auto laut = machSatz (true);
            setzeGlobal (laut, "v1.global.output_trim_db", 6.0);
            k->uebernehmeZustand (laut);
            const auto vor = fahreDc (*k, 0.2, kRampeSamples + 4096, 64);
            k->setzeHoermatrix (Hoermatrix::dry);
            auto lauf = fahreDc (*k, 0.2, 64, 64);
            k->setzeHoermatrix (Hoermatrix::delta);
            const auto rest = fahreDc (*k, 0.2, 1024, 64);
            lauf.insert (lauf.end(), rest.begin(), rest.end());

            const double D  = (double) 0.2f;
            const double P  = vor.back();                                  // Processed mit +6 dB
            const double De = (P - D) * std::pow (10.0, kDeltaMakeupDb / 20.0);
            const double stufe = std::max (std::abs (D - P), std::abs (De - D)) / (double) kFadeSamples;
            const size_t k256 = (size_t) kFadeSamples;
            const double amWechsel = std::abs (lauf[64] - lauf[63]);
            const double sprung = groessterSprung (P, lauf);
            const bool wartet = std::abs (lauf[k256 - 1] - (P + (D - P) * 255.0 / 256.0)) <= kRundungFloat;
            pruefe (amWechsel <= stufe + kRundungFloat && sprung <= stufe + kRundungFloat && wartet
                    && std::abs (lauf.back() - De) <= 1e-6,
                    "hoermatrix_wechsel_zu_drittem_zustand_wartet_das_fade_ende_ab (M-55, E-30)",
                    "am Umschaltsample " + zahl (amWechsel, 9) + ", max Sprung " + zahl (sprung, 9)
                    + " gegen Fadeschritt " + zahl (stufe, 9) + ", Fade zu Dry laeuft zu Ende: "
                    + (wartet ? "ja" : "nein") + ", Ende " + zahl (lauf.back(), 6) + " gegen Delta " + zahl (De, 6));
        }

        // M-56: Candidate ohne Kandidat faellt SICHTBAR zurueck.
        {
            auto k = neuerKern (fs, 512);
            k->uebernehmeZustand (machSatz (true));
            k->setzeHoermatrix (Hoermatrix::candidate);
            fahreStille (*k, 512, 512);
            pruefe (k->gewuenschteHoermatrix() == Hoermatrix::candidate
                    && k->wirksameHoermatrix() == Hoermatrix::processed,
                    "candidate_ohne_kandidat_faellt_sichtbar_zurueck (M-56)",
                    "der Getter meldet den WIRKSAMEN Zustand");

            // Mit Kandidat: die Auswahl greift.
            auto c = machSatz (true);
            belege (c, 0, Filtertyp::bell, 1000.0, 1.0, 12.0);
            k->uebernehmeZustand (c, Pfad::candidate);
            fahreStille (*k, 1024, 512);
            pruefe (k->wirksameHoermatrix() == Hoermatrix::candidate,
                    "mit_kandidat_greift_die_auswahl (M-56)");
        }

        // B-6 / §3.0: der Candidate-Pfad fuehrt EIGENE Rampen (hier der
        // Output-Trim) und einen eigenen Auto-Gain aus SEINER Kurve.
        {
            auto k = neuerKern (fs, 512);
            k->uebernehmeZustand (machSatz (true));
            auto kand = machSatz (true);
            setzeGlobal (kand, "v1.global.output_trim_db", 6.0);
            k->uebernehmeZustand (kand, Pfad::candidate);
            fahreStille (*k, kRampeSamples + 2048, 512);
            std::vector<float> a (512, 0.25f), b (512, 0.25f);
            float* kan[2] = { a.data(), b.data() };
            k->verarbeite (kan, 2, 512);
            const double* pc = k->tap (Tap::postCandidate, 0);
            const double* pk = k->tap (Tap::postCommitted, 0);
            const double sollTrim = 0.25 * std::pow (10.0, 6.0 / 20.0);
            pruefe (pc != nullptr && pk != nullptr && std::abs (pc[500] - sollTrim) < 1e-6 && pk[500] == 0.25,
                    "candidate_fuehrt_eigene_rampen (§3.0, B-6)",
                    "Candidate " + zahl (pc != nullptr ? pc[500] : 0.0, 6) + " gegen " + zahl (sollTrim, 6)
                    + ", Committed " + zahl (pk != nullptr ? pk[500] : 0.0, 6));

            auto k2 = neuerKern (fs, 512);
            auto neutral = machSatz (true);
            setzeGlobalBool (neutral, "v2.global.auto_gain", true);
            k2->uebernehmeZustand (neutral);
            auto shelf = neutral;
            belege (shelf, 0, Filtertyp::highShelf, 20.0, 1.0, 6.0);
            k2->uebernehmeZustand (shelf, Pfad::candidate);
            // DC lange genug, dass der 20-Hz-Shelf eingeschwungen ist.
            fahreDc (*k2, 0.25, 32768, 512);
            std::vector<float> c (512, 0.25f), d (512, 0.25f);
            float* kan2[2] = { c.data(), d.data() };
            k2->verarbeite (kan2, 2, 512);
            const double* pc2 = k2->tap (Tap::postCandidate, 0);
            // Bei DC traegt der High-Shelf 0 dB; am Tap bleibt allein der
            // angewandte Ausgleich des Candidate.
            const double sollAg = 0.25 * std::pow (10.0, k2->autoGainCandidateDb() / 20.0);
            pruefe (std::abs (k2->autoGainCandidateDb() + 6.0) <= 0.1 && k2->autoGainDb() == 0.0
                    && pc2 != nullptr && std::abs (pc2[500] - sollAg) < 1e-4,
                    "candidate_auto_gain_aus_eigener_kurve (§3.0, B-6)",
                    "Candidate " + zahl (k2->autoGainCandidateDb(), 4) + " dB, Committed "
                    + zahl (k2->autoGainDb(), 4) + " dB, Candidate-Tap bei DC "
                    + zahl (pc2 != nullptr ? pc2[500] : 0.0, 6) + " gegen " + zahl (sollAg, 6));
        }

        // B-7 / §44.2: ein Candidate-Wechsel blendet von der BISHERIGEN
        // Candidate-Bank auf die neue, nicht vom Eingang. Beide Wechsel sind
        // TOPOLOGISCH (Kanalmodus, dann Typ) und laufen damit als Crossfade
        // zwischen zwei Candidate-Baenken; ein reiner Rampenwechsel (E-19)
        // haette keine Quellbank, und die Probe maesse nichts. "Dieselbe Kurve"
        // ist derselbe Low-Shelf auf Mid statt Stereo - bei L = R klingt er
        // identisch.
        {
            auto k = neuerKern (fs, 64);
            k->uebernehmeZustand (machSatz (true));
            auto c1 = machSatz (true);
            belege (c1, 0, Filtertyp::lowShelf, 8000.0, 0.707, 9.0);
            k->uebernehmeZustand (c1, Pfad::candidate);
            k->setzeHoermatrix (Hoermatrix::candidate);
            const auto vor = fahreDc (*k, 0.3, 4096, 64);
            k->pflege();

            auto gleicheKurve = machSatz (true);
            belege (gleicheKurve, 0, Filtertyp::lowShelf, 8000.0, 0.707, 9.0, Kanalmodus::mid);
            k->uebernehmeZustand (gleicheKurve, Pfad::candidate);
            const auto gleich = fahreDc (*k, 0.3, 1024, 64);
            k->pflege();

            auto c2 = machSatz (true);
            belege (c2, 0, Filtertyp::highShelf, 8000.0, 0.707, 9.0);
            k->uebernehmeZustand (c2, Pfad::candidate);
            const auto anders = fahreDc (*k, 0.3, 1024, 64);
            const double B = 0.3;   // ein High-Shelf traegt DC mit 0 dB
            const double stufe = std::abs (B - gleich.back()) / (double) kFadeSamples;

            // W-4 (E-31): AM UMSCHALTSAMPLE gemessen, dem ersten Sample des
            // neuen Fades. Dieselbe Kurve hat die Fadeschrittweite 0 - dort
            // bleibt allein die Rundung des float-Ausgangs; aus Dry waere es der
            // ganze Abstand zum Eingang.
            const double amGleich  = std::abs (gleich[0] - vor.back());
            const double amAnders  = std::abs (anders[0] - gleich.back());
            pruefe (amGleich <= kRundungFloat && amAnders <= stufe + kRundungFloat && std::abs (anders.back() - B) < 1e-4,
                    "candidate_wechsel_blendet_aus_der_bisherigen_candidate_bank (§44.2, B-7, W-4)",
                    "am Umschaltsample: dieselbe Kurve " + zahl (amGleich, 9) + " (aus Dry waeren es "
                    + zahl (vor.back() - 0.3, 4) + "), andere Kurve " + zahl (amAnders, 9)
                    + " gegen Fadeschritt " + zahl (stufe, 9));

            // Die Wache ueber beide Laeufe: Schranke aus dem Einschwingen der
            // kalt startenden Candidate-Bank (E-31). "Dieselbe Kurve" ist der
            // Low-Shelf auf Mid - bei L = R rechnet er wie auf L.
            const double x = (double) 0.3f;
            const double wacheGleich  = kaltSchranke (Filtertyp::lowShelf,  fs, 8000.0, 0.707, 9.0, vor.back(), x);
            const double wacheAnders  = kaltSchranke (Filtertyp::highShelf, fs, 8000.0, 0.707, 9.0, gleich.back(), x);
            const double sprungGleich = groessterSprung (vor.back(), gleich);
            const double sprungAnders = groessterSprung (gleich.back(), anders);
            pruefe (sprungGleich <= wacheGleich + kRundungFloat && sprungAnders <= wacheAnders + kRundungFloat,
                    "wache_der_kalt_startenden_candidate_bank_ueber_den_ganzen_lauf (B-7, W-4)",
                    "dieselbe Kurve max " + zahl (sprungGleich, 9) + " gegen " + zahl (wacheGleich, 9)
                    + ", andere Kurve max " + zahl (sprungAnders, 9) + " gegen " + zahl (wacheAnders, 9));
        }

        // B-8 / M-46: Abbruch MITTEN im Candidate-Fade bei gewaehlter Hoermatrix
        // Candidate. Danach sind beide Candidate-Baenke ueber den ACK frei, der
        // Ausgang springt nicht und ist ab Fadeende gleich Processed.
        {
            auto k = neuerKern (fs, 64);
            auto com = machSatz (true);
            belege (com, 0, Filtertyp::lowShelf, 8000.0, 0.707, 6.0);
            k->uebernehmeZustand (com);
            auto c1 = machSatz (true);
            belege (c1, 0, Filtertyp::lowShelf, 8000.0, 0.707, -9.0);
            k->uebernehmeZustand (c1, Pfad::candidate);
            k->setzeHoermatrix (Hoermatrix::candidate);
            const auto vor = fahreDc (*k, 0.3, 4096, 64);
            k->pflege();

            auto c2 = machSatz (true);
            belege (c2, 0, Filtertyp::highShelf, 8000.0, 0.707, 9.0);   // Typwechsel: ein echter Crossfade
            k->uebernehmeZustand (c2, Pfad::candidate);
            auto lauf = fahreDc (*k, 0.3, 128, 64);
            k->beendeCandidate();
            const auto rest = fahreDc (*k, 0.3, 2048, 64);
            lauf.insert (lauf.end(), rest.begin(), rest.end());
            const int geerntet = k->pflege();

            std::vector<float> a (64, 0.3f), b (64, 0.3f);
            float* kan[2] = { a.data(), b.data() };
            k->verarbeite (kan, 2, 64);
            const double* pk = k->tap (Tap::postCommitted, 0);
            bool gleichProcessed = pk != nullptr;
            for (int i = 0; i < 64 && pk != nullptr; ++i)
                if (a[(size_t) i] != (float) pk[i]) gleichProcessed = false;
            int cA = -1, cQ = -1, kA = -1, kQ = -1;
            k->gefahreneSlots (cA, cQ, kA, kQ);

            const double A  = vor.back();
            const double C2 = 0.3;   // ein High-Shelf traegt DC mit 0 dB
            const double P  = 0.3 * std::pow (10.0, 6.0 / 20.0);
            const double stufe = std::max ({ std::abs (C2 - A), std::abs (P - C2), std::abs (P - A) }) / (double) kFadeSamples;
            const double sprung = groessterSprung (A, lauf);
            pruefe (geerntet >= 2 && k->pool().belegteSlots() == 1 && kA == -1 && kQ == -1
                    && gleichProcessed && sprung <= 4.0 * stufe + 1e-9 && ! k->candidateVorhanden(),
                    "candidate_ende_blendet_aus_und_gibt_beide_baenke_frei (M-46, B-8)",
                    "geerntet " + std::to_string (geerntet) + ", belegt " + std::to_string (k->pool().belegteSlots())
                    + ", Candidate-Slots " + std::to_string (kA) + "/" + std::to_string (kQ) + ", max Sprung "
                    + zahl (sprung, 7) + " gegen Fadeschritt " + zahl (stufe, 7) + ", danach gleich Processed: "
                    + (gleichProcessed ? "ja" : "nein"));
        }

        // W-5 / B-8 / M-46: Abbruch des Candidate WAEHREND des Hoermatrix-
        // Fades. Committed neutral, Candidate (Low-Shelf +9 dB) publiziert und
        // eingeblendet, dann gewaehlt, 64 Samples, dann `beendeCandidate`: die
        // Hoermatrix blendet vom aktuellen Mischstand zurueck (E-30). Am
        // Abbruchsample kein Sprung ueber die Fadeschrittweite, danach ist die
        // Candidate-Bank frei. Der Pfad blendet VOR der Wahl ein: aus der Ruhe
        // heraus wirkt die Auswahl erst nach seinem Einblenden (E-34), und der
        // Abbruch laege sonst gar nicht im Hoermatrix-Fade.
        {
            auto k = neuerKern (fs, 64);
            k->uebernehmeZustand (machSatz (true));
            const auto vor = fahreDc (*k, 0.3, 2048, 64);
            k->pflege();

            auto kand = machSatz (true);
            belege (kand, 0, Filtertyp::lowShelf, 8000.0, 0.707, 9.0);
            k->uebernehmeZustand (kand, Pfad::candidate);
            fahreDc (*k, 0.3, 1024, 64);
            k->setzeHoermatrix (Hoermatrix::candidate);
            auto lauf = fahreDc (*k, 0.3, 64, 64);
            k->beendeCandidate();
            const auto rest = fahreDc (*k, 0.3, 2048, 64);
            lauf.insert (lauf.end(), rest.begin(), rest.end());
            const int geerntet = k->pflege();
            int cA = -1, cQ = -1, kA = -1, kQ = -1;
            k->gefahreneSlots (cA, cQ, kA, kQ);

            const double P = vor.back();                          // Committed neutral traegt den Eingang
            const double C = P * std::pow (10.0, 9.0 / 20.0);     // Low-Shelf +9 dB bei DC
            const double stufe = std::abs (C - P) / (double) kFadeSamples;
            const double amAbbruch = std::abs (lauf[64] - lauf[63]);
            const double sprung = groessterSprung (P, lauf);
            pruefe (amAbbruch <= stufe + kRundungFloat && sprung <= stufe + kRundungFloat
                    && geerntet == 1 && k->pool().belegteSlots() == 1 && kA == -1 && kQ == -1
                    && std::abs (lauf.back() - P) <= kRundungFloat && ! k->candidateVorhanden(),
                    "candidate_abbruch_im_hoermatrix_fade_blendet_vom_mischstand_zurueck (M-46, B-8, W-5)",
                    "am Abbruchsample " + zahl (amAbbruch, 9) + ", max Sprung " + zahl (sprung, 9)
                    + " gegen Fadeschritt " + zahl (stufe, 9) + ", geerntet " + std::to_string (geerntet)
                    + ", belegt " + std::to_string (k->pool().belegteSlots()) + ", Candidate-Slots "
                    + std::to_string (kA) + "/" + std::to_string (kQ));
        }

        // X-1 / M-55 / M-46 / B-8: Candidate-Abbruch bei IDENTISCHEN A/B-
        // Zustaenden. Committed und Candidate tragen beide Output-Trim +6 dB
        // und sind eingeschwungen, die Hoermatrix waehlt Candidate; dann endet
        // der Candidate zu drei Zeitpunkten: (a) 64 Samples in das Einblenden
        // der Hoermatrix, (b) nach abgeschlossenem Fade, (c) 64 Samples in das
        // Ausblenden zurueck auf Processed. Die Hoerrueckblende ist die EINZIGE
        // Blende (Entscheid E-33): der Ausgang bleibt ueber den ganzen Lauf
        // innerhalb der float-Rundung gleich Processed; die Candidate-Bank
        // klingt bis zum Ende der Rueckblende unveraendert und dient erst
        // danach ueber den ACK aus.
        {
            const char* namen[] = { "(a) im Einblenden", "(b) nach abgeschlossenem Fade", "(c) im Ausblenden" };
            for (int fall = 0; fall < 3; ++fall)
            {
                auto k = neuerKern (fs, 64);
                auto laut = machSatz (true);
                setzeGlobal (laut, "v1.global.output_trim_db", 6.0);
                k->uebernehmeZustand (laut);
                k->uebernehmeZustand (laut, Pfad::candidate);
                const auto vor = fahreDc (*k, 0.3, 4096, 64);
                k->pflege();

                k->setzeHoermatrix (Hoermatrix::candidate);
                auto lauf = fahreDc (*k, 0.3, fall == 0 ? 64 : 1024, 64);
                if (fall == 2)
                {
                    k->setzeHoermatrix (Hoermatrix::processed);
                    const auto zurueck = fahreDc (*k, 0.3, 64, 64);
                    lauf.insert (lauf.end(), zurueck.begin(), zurueck.end());
                }
                k->beendeCandidate();

                // 32 Samples nach dem Abbruch laeuft die Rueckblende in allen
                // drei Faellen noch (Rest 31, 224, 160): die Bank ist gehalten,
                // verblasst und hat noch keinen ACK.
                const auto mitte = fahreDc (*k, 0.3, 32, 64);
                lauf.insert (lauf.end(), mitte.begin(), mitte.end());
                int mA = -1, mQ = -1, mKA = -1, mKQ = -1;
                k->gefahreneSlots (mA, mQ, mKA, mKQ);
                const bool gehalten = mKA == -1 && mKQ >= 0
                                      && k->pool().zustand (mKQ) == BankZustand::verblassend;
                const int ernteMitte = k->pflege();

                const auto rest = fahreDc (*k, 0.3, 2048, 64);
                lauf.insert (lauf.end(), rest.begin(), rest.end());
                const int geerntet = k->pflege();
                int cA = -1, cQ = -1, kA = -1, kQ = -1;
                k->gefahreneSlots (cA, cQ, kA, kQ);

                const double P = vor.back();
                double maxAbw = 0.0;
                size_t anStelle = 0;
                for (size_t i = 0; i < lauf.size(); ++i)
                    if (std::abs (lauf[i] - P) > maxAbw) { maxAbw = std::abs (lauf[i] - P); anStelle = i; }
                pruefe (maxAbw <= kRundungFloat,
                        std::string ("candidate_abbruch_bei_identischen_zustaenden_nullt (M-55, X-1) ") + namen[fall],
                        "groesste Abweichung zu Processed " + zahl (maxAbw, 9) + " an Sample " + std::to_string (anStelle)
                        + " von " + std::to_string (lauf.size()) + ", Toleranz " + zahl (kRundungFloat, 9));
                pruefe (gehalten && ernteMitte == 0 && geerntet == 1 && k->pool().belegteSlots() == 1
                        && kA == -1 && kQ == -1 && ! k->candidateVorhanden(),
                        std::string ("candidate_bank_klingt_bis_zum_ende_der_rueckblende_und_dient_dann_aus (M-46, X-1) ")
                        + namen[fall],
                        std::string ("in der Rueckblende gehalten: ") + (gehalten ? "ja" : "nein") + ", ACK dort "
                        + std::to_string (ernteMitte) + ", danach geerntet " + std::to_string (geerntet) + ", belegt "
                        + std::to_string (k->pool().belegteSlots()) + ", Candidate-Slots " + std::to_string (kA) + "/"
                        + std::to_string (kQ));
            }
        }

        // E-34 / M-55: das Gegenstueck zu X-1 beim EINBLENDEN. Die Hoermatrix
        // steht auf Candidate, bevor es einen gibt (M-56: sie faellt sichtbar
        // auf Processed zurueck); dann kommt ein Candidate, der dem Committed
        // gleicht. Der Pfad blendet aus der Ruhe ein, und die Hoermatrix blendet
        // erst DANACH auf Candidate - nie ueber seinen Fade von Dry her. Der
        // Ausgang bleibt ueber den ganzen Lauf innerhalb der float-Rundung
        // gleich Processed, und am Ende ist die Auswahl wirksam.
        {
            auto k = neuerKern (fs, 64);
            auto laut = machSatz (true);
            setzeGlobal (laut, "v1.global.output_trim_db", 6.0);
            k->uebernehmeZustand (laut);
            const auto vor = fahreDc (*k, 0.3, 4096, 64);
            k->setzeHoermatrix (Hoermatrix::candidate);
            auto lauf = fahreDc (*k, 0.3, 256, 64);
            k->uebernehmeZustand (laut, Pfad::candidate);
            const auto rest = fahreDc (*k, 0.3, 2048, 64);
            lauf.insert (lauf.end(), rest.begin(), rest.end());

            const double P = vor.back();
            double maxAbw = 0.0;
            size_t anStelle = 0;
            for (size_t i = 0; i < lauf.size(); ++i)
                if (std::abs (lauf[i] - P) > maxAbw) { maxAbw = std::abs (lauf[i] - P); anStelle = i; }
            const bool wirksam = k->wirksameHoermatrix() == Hoermatrix::candidate;
            pruefe (maxAbw <= kRundungFloat && wirksam,
                    "candidate_einblendung_bei_identischen_zustaenden_nullt (M-55, E-34)",
                    "groesste Abweichung zu Processed " + zahl (maxAbw, 9) + " an Sample " + std::to_string (anStelle)
                    + " von " + std::to_string (lauf.size()) + ", Auswahl am Ende wirksam: " + (wirksam ? "ja" : "nein"));
        }

        // B-9 / M-27 / R14: die Auslenkungen liegen JE PFAD. Ein Candidate mit
        // freiem Slot nullt die bewegte Committed-Auslenkung nicht, und ein
        // Candidate mit groesserer Range treibt sie nicht ueber die Grenze des
        // Committed-Programms.
        {
            auto k = neuerKern (fs, 512);
            auto com = machSatz (true);
            belege (com, 3, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (com, 3, -3.0, -50.0, 1.0, 0.0, 20.0);
            k->uebernehmeZustand (com);
            auto kandFrei = machSatz (true);
            belege (kandFrei, 0, Filtertyp::bell, 500.0, 1.0, 2.0);
            k->uebernehmeZustand (kandFrei, Pfad::candidate);
            long long n0 = 0;
            fahreStereoTon (*k, fs, 1000.0, 0.9, n0, 48000, 512);
            double wc[param::kSlots], wk[param::kSlots];
            k->auslenkungenDb (wc);
            k->auslenkungenCandidateDb (wk);
            const bool freiNulltNicht = std::abs (wc[3] + 3.0) < 1e-9 && wk[3] == 0.0;
            const double committed1 = wc[3];
            k->pflege();

            auto kandWeit = machSatz (true);
            belege (kandWeit, 3, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (kandWeit, 3, -12.0, -50.0, 1.0, 0.0, 20.0);
            k->uebernehmeZustand (kandWeit, Pfad::candidate);
            fahreStereoTon (*k, fs, 1000.0, 0.9, n0, 48000, 512);
            k->auslenkungenDb (wc);
            k->auslenkungenCandidateDb (wk);
            const bool grenzeHaelt = std::abs (wc[3]) <= 3.0 + 1e-12 && std::abs (wc[3] + 3.0) < 1e-9
                                     && std::abs (wk[3] + 12.0) < 1e-9;
            pruefe (freiNulltNicht && grenzeHaelt,
                    "auslenkungen_liegen_je_pfad_getrennt (M-27, R14, B-9)",
                    "Committed bei freiem Candidate-Slot " + zahl (committed1, 4) + " dB; mit Candidate-Range -12: Committed "
                    + zahl (wc[3], 4) + " dB, Candidate " + zahl (wk[3], 4) + " dB");
        }

        // M-57: die drei Taps sind kohaerent, und die Hoermatrix liegt
        // HINTER ihnen - ein Delta-Hoerzustand veraendert den Tap nicht.
        {
            auto k = neuerKern (fs, 512);
            auto ein = machSatz (true);
            belege (ein, 0, Filtertyp::bell, 1000.0, 1.0, 9.0);
            k->uebernehmeZustand (ein);
            fahreStille (*k, 2048, 512);

            // Der Ton traegt GENAU acht Perioden je 512 Samples: damit ist
            // jeder Block identisch, und der eingeschwungene Filterzustand
            // liefert blockweise dieselbe Tapenergie. Ein Ton, dessen
            // Periode die Blockgroesse nicht teilt, mass hier zwei
            // verschiedene Zustaende und nicht die Zusage.
            auto fuelleTon = [] (std::vector<float>& a, std::vector<float>& b)
            {
                for (int i = 0; i < 512; ++i)
                { a[(size_t) i] = (float) (0.4 * std::sin (2.0 * kPiRef * 8.0 * (double) i / 512.0));
                  b[(size_t) i] = a[(size_t) i]; }
            };
            auto tapSumme = [&] (Hoermatrix h)
            {
                k->setzeHoermatrix (h);
                std::vector<float> a (512), b (512);
                float* kan[2] = { a.data(), b.data() };
                // einschwingen, dann messen
                for (int blk = 0; blk < 12; ++blk) { fuelleTon (a, b); k->verarbeite (kan, 2, 512); }
                fuelleTon (a, b);
                k->verarbeite (kan, 2, 512);
                const double* post = k->tap (Tap::postCommitted, 0);
                double sum = 0.0;
                if (post) for (int i = 0; i < 512; ++i) sum += post[i] * post[i];
                return sum;
            };
            const double sProc  = tapSumme (Hoermatrix::processed);
            const double sDelta = tapSumme (Hoermatrix::delta);
            pruefe (std::abs (sProc - sDelta) / (sProc > 0.0 ? sProc : 1.0) < 1e-9,
                    "hoermatrix_liegt_hinter_den_taps (M-57)",
                    "Tapenergie processed " + zahl (sProc, 6) + " gegen delta " + zahl (sDelta, 6));

            k->setzeHoermatrix (Hoermatrix::processed);
            std::vector<float> a (512), b (512);
            for (int i = 0; i < 512; ++i)
            { a[(size_t) i] = (float) (0.4 * std::sin (0.02 * (double) i)); b[(size_t) i] = a[(size_t) i]; }
            float* kan[2] = { a.data(), b.data() };
            k->verarbeite (kan, 2, 512);
            pruefe (k->tapLaenge() == 512 && k->tap (Tap::preNakama, 0) != nullptr
                    && k->tap (Tap::postCommitted, 0) != nullptr,
                    "taps_sind_kohaerent (M-57)",
                    "gleiches Zeitfenster, gleiche Laenge");
        }

        // M-114: Dynamik ist nur da, wenn wirklich ein Band dynamisch ist.
        {
            auto ohne = neuerKern (fs, 512);
            ohne->uebernehmeZustand (machSatz (true));
            pruefe (! ohne->dynamikVorhanden(), "feld_reist_nur_bei_vorhandener_dynamik (M-114)",
                    "kein dynamisches Band: keine Meldung");

            auto mit = neuerKern (fs, 512);
            auto s2 = machSatz (true);
            belege (s2, 5, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (s2, 5, -6.0, -30.0);
            mit->uebernehmeZustand (s2);
            pruefe (mit->dynamikVorhanden(), "mit dynamischem Band reist das Feld (M-114)");

            auto ausgeschaltet = neuerKern (fs, 512);
            auto s3 = s2;
            s3.werte[(size_t) param::kIndexEqEnabled].b = false;
            ausgeschaltet->uebernehmeZustand (s3);
            pruefe (! ausgeschaltet->dynamikVorhanden(),
                    "bei ausgeschaltetem EQ reist es nicht (M-114)");
        }

        // M-121: ein Programmwechsel verwirft die alte Auslenkung.
        {
            auto k = neuerKern (fs, 512);
            auto dyn = machSatz (true);
            belege (dyn, 2, Filtertyp::bell, 1000.0, 1.0, 0.0);
            machDynamisch (dyn, 2, -9.0, -50.0, 1.0, 0.0, 5.0);
            k->uebernehmeZustand (dyn);

            std::vector<float> a (512), b (512);
            float* kan[2] = { a.data(), b.data() };
            for (int blk = 0; blk < 100; ++blk)
            {
                for (int i = 0; i < 512; ++i)
                { const double x = 0.9 * std::sin (2.0 * kPiRef * 1000.0 * (double) (blk * 512 + i) / fs);
                  a[(size_t) i] = (float) x; b[(size_t) i] = (float) x; }
                k->verarbeite (kan, 2, 512);
            }
            double vorher[param::kSlots];
            k->auslenkungenDb (vorher);
            k->pflege();

            // (a) REMOVE: der Slot wird frei. `verarbeiteBand` kehrt dann
            //     frueh zurueck und schreibt die Auslenkung NIE - der alte,
            //     endliche Wert bliebe stehen und bestuende Laenge,
            //     Endlichkeit und Anwesenheitspruefung (MN1, B-06).
            auto entfernt = machSatz (true);
            belege (entfernt, 0, Filtertyp::bell, 500.0, 1.0, 3.0);   // anderes Band
            k->uebernehmeZustand (entfernt);
            fahreStille (*k, kFadeSamples + 2048, 512);

            double nachRemove[param::kSlots];
            k->auslenkungenDb (nachRemove);
            pruefe (std::abs (vorher[2]) > 1.0 && nachRemove[2] == 0.0,
                    "remove_verwirft_die_alte_auslenkung (M-121)",
                    "vorher " + zahl (vorher[2], 4) + " dB, nach dem Remove "
                    + zahl (nachRemove[2], 15));

            // (b) NEUBELEGUNG desselben Slots mit einem statischen Band: der
            //     neue Slot erbt den Wert seines Vorgaengers nicht.
            auto neu = machSatz (true);
            belege (neu, 2, Filtertyp::notch, 3000.0, 4.0, 0.0);   // statisch
            k->uebernehmeZustand (neu);
            fahreStille (*k, kFadeSamples + 2048, 512);

            double nachher[param::kSlots];
            k->auslenkungenDb (nachher);
            pruefe (nachher[2] == 0.0,
                    "remove_mit_neubelegung_verwirft_die_alte_auslenkung (M-121)",
                    "nachher " + zahl (nachher[2], 15));
        }

        // (c) NAK-311 W03 (Manifest NAK-311 §6.3 M-41, T3-14-02): eine
        //     VERDRAENGTE Zwischenpublikation bricht den Lebenszyklus. Belegung A
        //     (dynamisches Bell 1 kHz Q 2, Range -12 dB, Threshold -40 dB, Attack
        //     0,1 ms, Hold 500 ms, Release 5000 ms) mit einem Quadraturton 0,5
        //     eingeschwungen (Detektor -9 dB, 31 dB ueber Threshold: Plateau
        //     -12 dB). Dann "leer" (Slot 0 frei) und B (gleich A) vor DEMSELBEN
        //     Blockrand, ohne Audio dazwischen: der Audiothread nimmt nur A und B,
        //     und B muss trotzdem kalt beginnen - der Remove lag in der
        //     verdraengten Zwischenpublikation (R-311-1).
        {
            auto k = neuerKern (fs, 512);
            auto belegungA = machSatz (true);
            belege (belegungA, 0, Filtertyp::bell, 1000.0, 2.0, 0.0);
            machDynamisch (belegungA, 0, -12.0, -40.0, 0.1, 500.0, 5000.0);
            k->uebernehmeZustand (belegungA);
            const double w = 2.0 * kPiRef * 1000.0 / fs;
            long long n = 0;
            fahreW03 (*k, [w] (long long i, float& l, float& r)
                      { l = (float) (0.5 * std::sin (w * (double) i)); r = (float) (0.5 * std::cos (w * (double) i)); },
                      n, 48000, 512, nullptr);
            double vorherA[param::kSlots];
            k->auslenkungenDb (vorherA);
            k->pflege();

            const std::uint64_t uebernahmenVor = k->uebernahmen();
            k->uebernehmeZustand (machSatz (true));   // "leer": Slot 0 frei - wird verdraengt
            k->uebernehmeZustand (belegungA);         // B = A, vor demselben Blockrand
            W03Lauf stille;
            fahreW03 (*k, [] (long long, float& l, float& r) { l = 0.0f; r = 0.0f; }, n, 2048, 64, &stille);
            const std::uint64_t uebernahmen = k->uebernahmen() - uebernahmenVor;

            double slot0Max = 0.0;   // betragsgroesster gemeldeter Wert, mit Vorzeichen
            for (const auto& a : stille.auslenkungen) if (std::abs (a[0]) > std::abs (slot0Max)) slot0Max = a[0];
            double spitzeNachFade = 0.0;
            for (size_t i = 2u * (size_t) kFadeSamples; i < stille.aus.size(); ++i)
                spitzeNachFade = std::max (spitzeNachFade, std::abs (stille.aus[i]));
            pruefe (vorherA[0] == -12.0 && uebernahmen == 1 && slot0Max == 0.0 && spitzeNachFade == 0.0,
                    "311/M-41 verdraengte_zwischenpublikation_bricht_den_lebenszyklus (NAK-311 T3-14-02, M-121, R-311-1)",
                    "A eingeschwungen " + zahl (vorherA[0], 4) + " dB; leer und B vor demselben Blockrand, Uebernahmen +"
                    + std::to_string (uebernahmen) + "; betragsgroesste Meldung von Slot 0 ab dem ersten Block "
                    + zahl (slot0Max, 6) + " dB (ueber 32 Bloecke Stille); Spitze bei Stille ab Sample "
                    + std::to_string (kFadeSamples) + ": " + zahl (spitzeNachFade, 9));
        }
    }

    //==========================================================================
    std::cout << std::endl << "== M - Bankpool unter echter Parallelitaet (M-122) ==" << std::endl;
    {
        // Entwurf §44.5 verlangt "ThreadSanitizer- beziehungsweise
        // AEQUIVALENTE Stresslaeufe". Die MSVC-Toolchain dieses Projekts
        // traegt keinen ThreadSanitizer; der aequivalente Stresslauf IST
        // deshalb der Nachweis, und diese Zeile sagt das ausdruecklich statt
        // ein Werkzeug zu behaupten, das es hier nicht gibt.
        // Blockgroesse 64 bei kFadeSamples 256: eine Bank bleibt VIER Bloecke
        // lang `verblassend`. Bei 256 endet der Fade in EINEM Block, und das
        // Fenster, in dem ein vorgezogenes Reclaim ueberhaupt sichtbar waere,
        // ist praktisch null - der Stresslauf liefe dann an der Zusage vorbei.
        auto kern = neuerKern (48000.0, 64);
        auto& pool = kern->pool();

        std::atomic<bool> laeuft { true };
        std::atomic<std::uint64_t> verletzungen { 0 }, uebergaenge { 0 };
        std::atomic<std::uint64_t> letzteGeneration { 0 };
        std::atomic<std::uint64_t> generationsBruch { 0 };
        std::atomic<std::uint64_t> pfadKreuzungen { 0 };

        std::thread audio ([&]
        {
            std::vector<float> a (64), b (64);
            float* kan[2] = { a.data(), b.data() };
            std::uint64_t vorige[2] = { 0, 0 };

            // (1) Der Slot, den der Audiothread GERADE FAEHRT, gehoert ihm.
            //     Wird er `frei` oder `vorbereitend`, hat der Worker ihn vor
            //     dem ACK zurueckgenommen und schreibt in eine Bank, die hier
            //     noch gelesen wird - genau das Reclaim vor dem ACK, gegen das
            //     §44.2 gebaut ist. Die blosse Zustandszaehlung saehe das
            //     NICHT: vier Baenke bleiben vier Baenke, gleichgueltig wer
            //     sie haelt. Geprueft wird VOR und NACH dem Block, damit das
            //     Fenster nicht an der Blockgrenze verlorengeht.
            auto pruefeGefahrene = [&]
            {
                int sC = -1, sV = -1, sK = -1, sKV = -1;
                kern->gefahreneSlots (sC, sV, sK, sKV);
                for (int s : { sC, sV, sK, sKV })
                {
                    if (s < 0) continue;
                    const auto z = pool.zustand (s);
                    if (z == BankZustand::frei || z == BankZustand::vorbereitend)
                        verletzungen.fetch_add (1, std::memory_order_relaxed);
                }
                // B-1: keine Bank liegt je in ZWEI Pfaden.
                for (int x : { sC, sV })
                    for (int y : { sK, sKV })
                        if (x >= 0 && x == y) pfadKreuzungen.fetch_add (1, std::memory_order_relaxed);
            };

            while (laeuft.load (std::memory_order_acquire))
            {
                pruefeGefahrene();
                for (int i = 0; i < 64; ++i)
                { a[(size_t) i] = (float) (0.3 * std::sin (0.01 * (double) i)); b[(size_t) i] = a[(size_t) i]; }
                kern->verarbeite (kan, 2, 64);
                pruefeGefahrene();

                // (2) Nie mehr aktive Baenke als Pfade, und nie mehr belegte
                //     als es Slots gibt.
                int aktive = 0, vorbereitende = 0;
                for (int s = 0; s < DspBankPool::kBaenke; ++s)
                {
                    const auto z = pool.zustand (s);
                    if (z == BankZustand::audioAktiv)   ++aktive;
                    if (z == BankZustand::vorbereitend) ++vorbereitende;
                }
                if (aktive > 2) verletzungen.fetch_add (1, std::memory_order_relaxed);
                if (aktive + vorbereitende > DspBankPool::kBaenke)
                    verletzungen.fetch_add (1, std::memory_order_relaxed);

                // Generationen je Pfad streng monoton - die beiden Pfade
                // nehmen ihre Programme unabhaengig voneinander.
                int gC = -1, gCQ = -1, gK = -1, gKQ = -1;
                kern->gefahreneSlots (gC, gCQ, gK, gKQ);
                const int aktivJePfad[2] = { gC, gK };
                for (int p = 0; p < 2; ++p)
                {
                    if (aktivJePfad[p] < 0) continue;
                    const auto g = pool.generation (aktivJePfad[p]);
                    if (g < vorige[p]) generationsBruch.fetch_add (1, std::memory_order_relaxed);
                    vorige[p] = g;
                    letzteGeneration.store (g, std::memory_order_relaxed);
                }
                uebergaenge.fetch_add (1, std::memory_order_relaxed);
            }
        });

        auto s = machSatz (true);
        for (int runde = 0; runde < 3000; ++runde)
        {
            auto v = s;
            belege (v, runde % param::kSlots, Filtertyp::bell,
                    200.0 + (double) (runde % 40) * 100.0, 1.0 + (double) (runde % 5), 3.0);
            // B-1: BEIDE Pfade publizieren, dazu Candidate-Enden - erst dann
            // kann ein verdraengter Slot fuer den anderen Pfad neu belegt werden.
            bool ok = true;
            if (runde % 7 == 3)      ok = kern->uebernehmeZustand (v, Pfad::candidate);
            else if (runde % 7 == 5) kern->beendeCandidate();
            else                     ok = kern->uebernehmeZustand (v);
            if (! ok) kern->pflege();   // busy_retry: ernten und weiter
            kern->pflege();
        }
        laeuft.store (false, std::memory_order_release);
        audio.join();

        pruefe (verletzungen.load() == 0, "bankpool_parallel_ohne_rennen (M-122)",
                std::to_string (uebergaenge.load()) + " Bloecke, 3000 Publikationen, "
                + std::to_string (verletzungen.load()) + " Invariantenbrueche");
        pruefe (generationsBruch.load() == 0, "generationen_bleiben_streng_monoton (M-122)",
                "je Pfad; letzte Generation " + std::to_string (letzteGeneration.load()));
        pruefe (pfadKreuzungen.load() == 0, "keine_bank_liegt_je_in_zwei_pfaden (M-122, B-1)",
                "Committed und Candidate publizieren parallel, " + std::to_string (pfadKreuzungen.load())
                + " Kreuzungen");
        pruefe (pool.ackUeberlaeufe() == pool.reclaimVerriegelungen(),
                "kein stiller Ringverlust unter Last (M-43, M-45)",
                "Ueberlaeufe=" + std::to_string (pool.ackUeberlaeufe())
                + ", Verriegelungen=" + std::to_string (pool.reclaimVerriegelungen())
                + ", Maske=" + std::to_string (pool.reclaimPendingMask()));
    }

    //==========================================================================
    std::cout << std::endl << "== N - Messwert des Vier-Bank-Falls (M-118, R15, kein erfundener Deckel) ==" << std::endl;
    {
        // §49.3 nennt keine CPU-Zeile und §44.2 einen Golden ohne Zahl. Der
        // Test haelt deshalb fest, DASS der Vier-Bank-Fall laeuft und keine
        // Bank teilt; die gemessene Last steht als Zahl hier und im Manifest.
        // Ein erfundener Deckel waere auf einer schnelleren Maschine grundlos
        // gruen und auf einer langsameren grundlos rot.
        //
        // D10 (Codeaudit 10.09.2026): bis dahin aenderte das zweite Programm
        // nur den Output-Trim - rampenkompatibel, also kein Crossfade und kein
        // zweiter Bankdurchlauf -, `freieSlots() == 0` zaehlte bereite und
        // ausgediente Baenke mit, und die Zeitmessung trug den Aufbau des
        // Testsignals. Gemessen wurden durchgehend ZWEI rechnende Baenke.
        // Jetzt gilt:
        //  - ein ECHTER gleichzeitiger Topologiewechsel auf beiden Pfaden: jeder
        //    Wechsel dreht den channel_mode aller acht dynamischen Baender weiter
        //    (rampenINkompatibel), und beide Wuensche liegen vor demselben
        //    Blockrand;
        //  - Uebergang und stationaerer Zustand werden GETRENNT gemessen, je
        //    Callback nur `verarbeite`: das Signal liegt vorher fertig im
        //    Speicher, Publikation und Ernte des Workers liegen ausserhalb;
        //  - dass vier Baenke RECHNEN, faellt an den Bankidentitaeten
        //    (`rechnendeSlots`: je Pfad die klingende Bank und die Quelle des
        //    Crossfades, vier verschiedene, keine Passthrough) und an den echten
        //    Bankdurchlaeufen (`msStufenLaeufe`: bei width 1,3 rechnet jeder
        //    Durchlauf die M/S-Stufe genau einmal).
        const double fs = 48000.0;
        const int blockGroesse = 128;
        // Die Wiederholungen kommen aus kFadeSamples und der Blockgroesse: ein
        // Uebergang rechnet in genau so vielen Callbacks vier Baenke. Die
        // Blockgroesse liegt unter kFadeSamples, damit der Crossfade ueber einen
        // Callbackrand laeuft - nur dort sind die Bankidentitaeten von aussen
        // lesbar.
        const int uebergangsCallbacks = (kFadeSamples + blockGroesse - 1) / blockGroesse;
        // Je Region wird dieselbe Audiozeit gewertet wie im bisherigen Messwert
        // (4000 Callbacks zu 256 Samples): die Zahl bleibt vergleichbar, und
        // p99 und Maximum stehen auf Tausenden Callbacks.
        const int gewerteteSamples = 4000 * 256;
        const int uebergaenge = (gewerteteSamples + uebergangsCallbacks * blockGroesse - 1)
                              / (uebergangsCallbacks * blockGroesse);
        const int callbacksJeRegion = uebergaenge * uebergangsCallbacks;
        const double audioJeRegion = (double) (callbacksJeRegion * blockGroesse) / fs;
        auto kern = neuerKern (fs, blockGroesse);

        const auto programm = [] (int modusVersatz, double gainVersatz)
        {
            auto s = machSatz (true);
            for (int slot = 0; slot < param::kSlots; ++slot)
            {
                belege (s, slot, Filtertyp::bell, 100.0 * std::pow (2.0, (double) slot), 2.0, 4.0 + gainVersatz,
                        (Kanalmodus) ((slot + modusVersatz) % 5));
                machDynamisch (s, slot, -6.0, -40.0, 5.0, 10.0, 80.0);
            }
            setzeGlobal (s, "v1.global.width", 1.3);
            setzeGlobal (s, "v1.global.mono_bass_hz", 110.0);
            setzeGlobal (s, "v2.global.mix", 0.8);
            setzeGlobalBool (s, "v2.global.auto_gain", true);
            return s;
        };
        // Committed wechselt zwischen [0] und [1], Candidate zwischen [2] und
        // [3]: zwei Programme desselben Pfades unterscheiden sich im
        // channel_mode JEDES Bandes.
        const std::vector<param::DspSatz> programme { programm (0, 0.0), programm (1, 0.0),
                                                      programm (2, -2.0), programm (3, -2.0) };

        // Das Signal entsteht VOR der Messung.
        const int vorlaufCallbacks = (2 * kFadeSamples + 4096) / blockGroesse;
        const size_t signalLaenge = (size_t) ((vorlaufCallbacks + 2 * callbacksJeRegion) * blockGroesse);
        std::vector<float> signalL (signalLaenge), signalR (signalLaenge);
        for (size_t i = 0; i < signalLaenge; ++i)
        {
            const double x = 0.5 * std::sin (0.011 * (double) i);
            signalL[i] = (float) x;
            signalR[i] = (float) (x * 0.8);
        }
        std::vector<float> a ((size_t) blockGroesse), b ((size_t) blockGroesse);
        float* kan[2] = { a.data(), b.data() };
        size_t lesekopf = 0;
        const auto ladeBlock = [&]
        {
            std::memcpy (a.data(), signalL.data() + lesekopf, (size_t) blockGroesse * sizeof (float));
            std::memcpy (b.data(), signalR.data() + lesekopf, (size_t) blockGroesse * sizeof (float));
            lesekopf += (size_t) blockGroesse;
        };
        // Nur `verarbeite` liegt in der Messung.
        const auto gemessenerCallback = [&]
        {
            ladeBlock();
            const auto beginn = std::chrono::steady_clock::now();
            kern->verarbeite (kan, 2, blockGroesse);
            return std::chrono::duration<double> (std::chrono::steady_clock::now() - beginn).count();
        };
        std::vector<double> dauerUebergang, dauerStationaer;
        dauerUebergang.reserve ((size_t) callbacksJeRegion);
        dauerStationaer.reserve ((size_t) callbacksJeRegion);

        // Vorlauf, nicht gewertet: beide Pfade blenden aus der Ruhe ein, die
        // Huellkurven schwingen ein.
        int abgewiesen = 0;
        if (! kern->uebernehmeZustand (programme[0], Pfad::committed)) ++abgewiesen;
        if (! kern->uebernehmeZustand (programme[2], Pfad::candidate)) ++abgewiesen;
        for (int i = 0; i < vorlaufCallbacks; ++i)
        {
            ladeBlock();
            kern->verarbeite (kan, 2, blockGroesse);
        }
        kern->pflege();

        const auto& pool = kern->pool();
        int vierRechnend = 0, durchlaeufeUebergang = 0, zweiRechnend = 0;
        std::string bruchIdentitaet, bruchDurchlauf, bruchStationaer;
        for (int u = 0; u < uebergaenge; ++u)
        {
            const size_t ziel = (size_t) ((u + 1) % 2);
            if (! kern->uebernehmeZustand (programme[ziel], Pfad::committed))     ++abgewiesen;
            if (! kern->uebernehmeZustand (programme[2 + ziel], Pfad::candidate)) ++abgewiesen;

            // Die Uebergangsregion: beide Pfade im Crossfade.
            for (int c = 0; c < uebergangsCallbacks; ++c)
            {
                const auto laeufeVorher = kern->msStufenLaeufe();
                dauerUebergang.push_back (gemessenerCallback());
                const auto laeufe = kern->msStufenLaeufe() - laeufeVorher;
                if (laeufe == 4) ++durchlaeufeUebergang;
                else if (bruchDurchlauf.empty())
                    bruchDurchlauf = "Uebergang " + std::to_string (u) + ", Callback " + std::to_string (c)
                                   + ": " + std::to_string (laeufe) + " Bankdurchlaeufe";
                if (c + 1 == uebergangsCallbacks) continue;

                // Mitten im Crossfade: je Pfad zwei rechnende Baenke, vier verschiedene.
                int ck = -1, cq = -1, kk = -1, kq = -1;
                kern->rechnendeSlots (Pfad::committed, ck, cq);
                kern->rechnendeSlots (Pfad::candidate, kk, kq);
                const int ids[4] = { ck, cq, kk, kq };
                bool vier = true;
                for (int i = 0; i < 4; ++i)
                {
                    vier = vier && ids[i] >= 0;
                    for (int j = i + 1; j < 4; ++j) vier = vier && ids[i] != ids[j];
                }
                vier = vier && pool.zustand (ck) == BankZustand::audioAktiv && pool.zustand (kk) == BankZustand::audioAktiv
                            && pool.zustand (cq) == BankZustand::verblassend && pool.zustand (kq) == BankZustand::verblassend;
                if (vier) ++vierRechnend;
                else if (bruchIdentitaet.empty())
                    bruchIdentitaet = "Uebergang " + std::to_string (u) + ": Committed rechnet " + std::to_string (ck)
                                    + " mit Quelle " + std::to_string (cq) + ", Candidate " + std::to_string (kk)
                                    + " mit Quelle " + std::to_string (kq);
            }
            kern->pflege();   // die zwei ausgedienten Quellbaenke kommen frei

            // Die stationaere Region: gleich viele Callbacks, je Pfad eine Bank.
            for (int c = 0; c < uebergangsCallbacks; ++c)
            {
                const auto laeufeVorher = kern->msStufenLaeufe();
                dauerStationaer.push_back (gemessenerCallback());
                const auto laeufe = kern->msStufenLaeufe() - laeufeVorher;
                int ck = -1, cq = -1, kk = -1, kq = -1;
                kern->rechnendeSlots (Pfad::committed, ck, cq);
                kern->rechnendeSlots (Pfad::candidate, kk, kq);
                if (laeufe == 2 && ck >= 0 && kk >= 0 && ck != kk && cq < 0 && kq < 0) ++zweiRechnend;
                else if (bruchStationaer.empty())
                    bruchStationaer = "Uebergang " + std::to_string (u) + ", Callback " + std::to_string (c) + ": "
                                    + std::to_string (laeufe) + " Bankdurchlaeufe, Committed " + std::to_string (ck)
                                    + "/" + std::to_string (cq) + ", Candidate " + std::to_string (kk) + "/" + std::to_string (kq);
            }
        }

        // Anteil der Echtzeit je Callback: Dauer durch Blockgroesse/fs.
        struct Last { double mittel = 0.0, p99 = 0.0, maximum = 0.0; };
        const auto kennzahlen = [&] (std::vector<double> d)
        {
            Last l;
            if (d.empty()) return l;
            const double budget = (double) blockGroesse / fs;
            double summe = 0.0;
            for (const double x : d) summe += x;
            std::sort (d.begin(), d.end());
            const size_t p99 = (size_t) std::ceil (0.99 * (double) d.size()) - 1;
            l.mittel  = summe / ((double) d.size() * budget) * 100.0;
            l.p99     = d[p99] / budget * 100.0;
            l.maximum = d.back() / budget * 100.0;
            return l;
        };
        const Last lastUebergang = kennzahlen (dauerUebergang);
        const Last lastStationaer = kennzahlen (dauerStationaer);

        std::cout << "  MESSWERT Uebergangslast (vier rechnende Baenke, beide Pfade im Crossfade): "
                  << zahl (lastUebergang.mittel, 3) << " % Echtzeit im Mittel, p99 " << zahl (lastUebergang.p99, 3)
                  << " %, Maximum " << zahl (lastUebergang.maximum, 3) << " % - " << callbacksJeRegion
                  << " Callbacks zu " << blockGroesse << " Samples bei " << zahl (fs, 0) << " Hz ("
                  << zahl (audioJeRegion, 3) << " s Audio; " << uebergaenge << " Uebergaenge zu je "
                  << uebergangsCallbacks << " Callbacks)" << std::endl;
        std::cout << "  MESSWERT stationaere Last (zwei rechnende Baenke, je Pfad eine): "
                  << zahl (lastStationaer.mittel, 3) << " % Echtzeit im Mittel, p99 " << zahl (lastStationaer.p99, 3)
                  << " %, Maximum " << zahl (lastStationaer.maximum, 3) << " % - " << callbacksJeRegion
                  << " Callbacks zu " << blockGroesse << " Samples (" << zahl (audioJeRegion, 3) << " s Audio)"
                  << std::endl;

        pruefe (abgewiesen == 0, "jede Publikation des Messlaufs kam durch (kein busy_retry)",
                std::to_string (abgewiesen) + " abgewiesen");
        const int stichproben = uebergaenge * (uebergangsCallbacks - 1);
        pruefe (stichproben > 0 && vierRechnend == stichproben,
                "worst_case_vier_baenke_laeuft (M-118): mitten in jedem Uebergang rechnen vier verschiedene Baenke - zwei je Pfad, keine Passthrough, keine geteilt",
                std::to_string (vierRechnend) + "/" + std::to_string (stichproben) + " Stichproben"
                    + (bruchIdentitaet.empty() ? std::string() : "; erster Bruch: " + bruchIdentitaet));
        pruefe (durchlaeufeUebergang == callbacksJeRegion,
                "jeder Uebergangscallback rechnet vier echte Bankdurchlaeufe (M-118)",
                std::to_string (durchlaeufeUebergang) + "/" + std::to_string (callbacksJeRegion)
                    + (bruchDurchlauf.empty() ? std::string() : "; erster Bruch: " + bruchDurchlauf));
        pruefe (zweiRechnend == callbacksJeRegion,
                "jeder stationaere Callback rechnet zwei Baenke, je Pfad eine (M-118)",
                std::to_string (zweiRechnend) + "/" + std::to_string (callbacksJeRegion)
                    + (bruchStationaer.empty() ? std::string() : "; erster Bruch: " + bruchStationaer));
        pruefe (lastUebergang.mittel > 0.0 && std::isfinite (lastUebergang.mittel) && std::isfinite (lastUebergang.maximum),
                "die Uebergangslast wurde gemessen, nicht gedeckelt (M-118, R15)",
                zahl (lastUebergang.mittel, 3) + " % Echtzeit im Mittel - der Wert steht im Manifest, kein Deckel im Test");
        pruefe (lastStationaer.mittel > 0.0 && std::isfinite (lastStationaer.mittel) && std::isfinite (lastStationaer.maximum),
                "die stationaere Last wurde gemessen, nicht gedeckelt (M-118, R15)",
                zahl (lastStationaer.mittel, 3) + " % Echtzeit im Mittel - der Wert steht im Manifest, kein Deckel im Test");
    }

    //==========================================================================
    std::cout << std::endl << "== O - Realtime und Offline gleich (M-120, Haelfte des Kerns) ==" << std::endl;
    {
        // M-120: Realtime- und Offline-Render erzeugen bei gleichem Event- und
        // Blockverlauf denselben Parameterverlauf und damit denselben Ausgang.
        // Dass Offline den bestaetigten Zustand und nie eine Vorschau nutzt,
        // misst B7 am Prozessor. Hier die Haelfte des Kerns: sein Verlauf
        // haengt an SAMPLES, nie an der Wanduhr. Zwei frische Kerne fahren
        // dieselbe Folge aus Bloecken wechselnder Groesse mit denselben
        // Worker-Aufrufen an denselben Blockraendern - der eine im Takt der
        // Wanduhr wie ein Host in Echtzeit, der andere ohne Takt wie ein
        // Offline-Render. Die Toleranz ist 0: derselbe Build rechnet dieselben
        // Samples, jede Abweichung waere eine Abhaengigkeit von der Zeit.
        const double fs = 48000.0;
        const int maxBlock = 512;
        const int groessen[] = { 256, 128, 480, 64, 512, 1, 333, 256, 200, 17 };
        const int bloecke = 180;

        auto basis = machSatz (true);
        belege (basis, 0, Filtertyp::bell, 250.0, 1.2, 5.0);
        belege (basis, 3, Filtertyp::highShelf, 5000.0, 0.707, -3.0, Kanalmodus::mid);
        // Das dynamische Band liegt auf dem Ton des Signals, und dessen Huellkurve
        // laeuft durch Threshold und Knie: sein Verlauf bewegt sich wirklich
        // (Wache unten) - ein Band unter dem Threshold staende exakt still (M-19),
        // und ein Vergleich seiner Auslenkungen verglieche nur Nullen.
        belege (basis, 5, Filtertyp::bell, 100.0, 1.0, 2.0);
        machDynamisch (basis, 5, -6.0, -20.0, 5.0, 10.0, 80.0);
        setzeGlobal (basis, "v1.global.width", 1.2);
        setzeGlobal (basis, "v2.global.mix", 0.9);
        setzeGlobalBool (basis, "v2.global.auto_gain", true);

        auto rampe = basis;   // nur kontinuierliche Werte: Rampenuebergang und globale Rampen
        setzeGlobal (rampe, "v1.global.output_trim_db", -4.0);
        setzeGlobal (rampe, "v1.global.input_trim_db", 2.0);
        setzeGlobal (rampe, "v2.global.mix", 0.6);
        rampe.werte[(size_t) param::indexBandV1 (0, param::kGainDb)].zahl = -2.0;

        auto topologie = rampe;   // ein Band mehr und ein anderer Kanalmodus: Crossfade
        belege (topologie, 6, Filtertyp::notch, 3000.0, 4.0, 0.0);
        topologie.werte[(size_t) param::indexBandV1 (3, param::kChannelMode)].enumIndex = (int) Kanalmodus::side;

        auto kandidat = basis;
        setzeGlobal (kandidat, "v1.global.output_trim_db", -1.5);
        belege (kandidat, 2, Filtertyp::lowShelf, 150.0, 0.707, 4.0);

        auto aus = topologie;
        aus.werte[(size_t) param::kIndexEqEnabled].b = false;

        struct Lauf
        {
            std::vector<float>  ausgang;        ///< L und R je Sample verschraenkt
            std::vector<double> auslenkungen;   ///< acht je Block
            std::vector<double> autoGain;       ///< einer je Block
            std::uint64_t uebernahmen = 0;
            double sekunden = 0.0, audioSekunden = 0.0, groessteLuecke = 0.0;
        };

        const auto fahre = [&] (bool imTaktDerWanduhr)
        {
            Lauf l;
            auto k = neuerKern (fs, maxBlock);
            std::vector<float> a ((size_t) maxBlock), b ((size_t) maxBlock);
            float* kan[2] = { a.data(), b.data() };
            size_t n = 0;
            const auto beginn = std::chrono::steady_clock::now();
            auto letztesEnde = beginn;
            for (int blk = 0; blk < bloecke; ++blk)
            {
                // Dieselben Worker-Aufrufe an denselben Blockraendern.
                switch (blk)
                {
                    case 0:   k->uebernehmeZustand (basis); break;
                    case 30:  k->uebernehmeZustand (rampe); break;
                    case 50:  k->uebernehmeZustand (kandidat, Pfad::candidate); break;
                    case 70:  k->setzeHoermatrix (Hoermatrix::candidate); break;
                    case 90:  k->uebernehmeZustand (topologie); break;
                    case 110: k->setzeHoermatrix (Hoermatrix::processed); break;
                    case 125: k->beendeCandidate(); break;
                    case 150: k->uebernehmeZustand (aus); break;
                    default:  break;
                }
                const int m = groessen[blk % (int) std::size (groessen)];
                for (int i = 0; i < m; ++i)
                {
                    // Ein Ton bei 99 Hz, dessen Amplitude langsam zwischen
                    // 0,05 und 0,55 pendelt: die Huellkurve des Bandes laeuft
                    // ueber Threshold und Knie hin und her.
                    const double t = (double) (n + (size_t) i);
                    const double x = (0.3 + 0.25 * std::sin (0.0021 * t)) * std::sin (0.013 * t);
                    a[(size_t) i] = (float) x;
                    b[(size_t) i] = (float) (0.7 * x);
                }
                // Wie ein Host in Echtzeit: der Block kommt erst, wenn die
                // Wanduhr seine Audiozeit erreicht hat.
                if (imTaktDerWanduhr)
                    std::this_thread::sleep_until (beginn + std::chrono::duration<double> ((double) n / fs));
                l.groessteLuecke = std::max (l.groessteLuecke,
                                             std::chrono::duration<double> (std::chrono::steady_clock::now() - letztesEnde).count());
                k->verarbeite (kan, 2, m);
                letztesEnde = std::chrono::steady_clock::now();
                for (int i = 0; i < m; ++i)
                {
                    l.ausgang.push_back (a[(size_t) i]);
                    l.ausgang.push_back (b[(size_t) i]);
                }
                double acht[8] {};
                k->auslenkungenDb (acht);
                l.auslenkungen.insert (l.auslenkungen.end(), acht, acht + 8);
                l.autoGain.push_back (k->autoGainDb());
                k->pflege();
                n += (size_t) m;
            }
            l.sekunden = std::chrono::duration<double> (std::chrono::steady_clock::now() - beginn).count();
            l.audioSekunden = (double) n / fs;
            l.uebernahmen = k->uebernahmen();
            return l;
        };

        const Lauf realtime = fahre (true);
        const Lauf offline  = fahre (false);

        long long ersteAbweichung = -1;
        const bool gleicheLaenge = realtime.ausgang.size() == offline.ausgang.size();
        for (size_t i = 0; gleicheLaenge && i < realtime.ausgang.size(); ++i)
            if (std::memcmp (&realtime.ausgang[i], &offline.ausgang[i], sizeof (float)) != 0)
            {
                ersteAbweichung = (long long) i;
                break;
            }
        const bool verlaufGleich =
               realtime.auslenkungen.size() == offline.auslenkungen.size()
            && realtime.autoGain.size() == offline.autoGain.size()
            && std::memcmp (realtime.auslenkungen.data(), offline.auslenkungen.data(),
                            realtime.auslenkungen.size() * sizeof (double)) == 0
            && std::memcmp (realtime.autoGain.data(), offline.autoGain.data(),
                            realtime.autoGain.size() * sizeof (double)) == 0;

        pruefe (realtime.sekunden + (double) maxBlock / fs >= realtime.audioSekunden && realtime.groessteLuecke > 0.001,
                "der Realtime-Lauf folgte der Wanduhr (M-120)",
                zahl (realtime.sekunden, 3) + " s fuer " + zahl (realtime.audioSekunden, 3)
                    + " s Audio, groesste Luecke zwischen zwei Bloecken " + zahl (realtime.groessteLuecke * 1000.0, 2) + " ms");
        pruefe (offline.sekunden < offline.audioSekunden,
                "der Offline-Lauf lief ohne Takt, schneller als Echtzeit (M-120)",
                zahl (offline.sekunden, 3) + " s fuer " + zahl (offline.audioSekunden, 3)
                    + " s Audio, groesste Luecke " + zahl (offline.groessteLuecke * 1000.0, 3) + " ms");
        pruefe (realtime.uebernahmen == offline.uebernahmen && realtime.uebernahmen == 6,
                "beide Laeufe nehmen dieselben sechs Publikationen (Einschalten, Rampe, Candidate, Crossfade, Candidate-Ende, Ausschalten)",
                std::to_string (realtime.uebernahmen) + " und " + std::to_string (offline.uebernahmen));
        pruefe (gleicheLaenge && ersteAbweichung < 0,
                "realtime_und_offline_gleich (M-120): derselbe Ausgang, bitgleich ueber "
                    + std::to_string (realtime.ausgang.size() / 2) + " Samples in " + std::to_string (bloecke) + " Bloecken",
                ersteAbweichung < 0 ? std::string ("Toleranz 0")
                                    : "erste Abweichung bei Sample " + std::to_string (ersteAbweichung / 2)
                                          + (ersteAbweichung % 2 == 0 ? " links" : " rechts"));
        // Wache: der Parameterverlauf muss sich bewegen, sonst verglichen die
        // zwei Laeufe nur Nullen und die folgende Pruefung saehe nichts.
        double auslenkungMin = 0.0, auslenkungMax = 0.0;
        for (size_t blk = 0; blk * 8 + 5 < realtime.auslenkungen.size(); ++blk)
        {
            auslenkungMin = std::min (auslenkungMin, realtime.auslenkungen[blk * 8 + 5]);
            auslenkungMax = std::max (auslenkungMax, realtime.auslenkungen[blk * 8 + 5]);
        }
        pruefe (auslenkungMax - auslenkungMin > 1.0,
                "die dynamische Auslenkung bewegte sich im Lauf - sonst verglich der Parameterverlauf nur Nullen (M-120)",
                "Slot 5 zwischen " + zahl (auslenkungMin, 2) + " und " + zahl (auslenkungMax, 2) + " dB");
        pruefe (verlaufGleich,
                "realtime_und_offline_gleich (M-120): derselbe Parameterverlauf - Auslenkungen und Auto-Gain je Block bitgleich");
    }

    //==========================================================================
    std::cout << std::endl << "== P - Pfadrampen in der Ruhe (NAK-311 W01) ==" << std::endl;
    {
        // NAK-311 T3-15-05 (Manifest NAK-311 §6.2, M-23 bis M-38): ein Pfad, der
        // in die Ruhe geht, traegt danach die Rampen des Ruheprogramms, also
        // 1,0 - dieselben Werte wie nach `bereiteVor`. Das Einschalten aus der
        // Ruhe ist damit das Einschalten eines frischen Kerns (SONDE-015 M-07:
        // die Sonde beginnt neu). Das ORAKEL ist deshalb ein frischer Kern
        // (`neuerKern`), der dasselbe Zielprogramm am selben Blockrand uebernimmt
        // und ab dort denselben Eingang in derselben Blockteilung bekommt;
        // verglichen wird der float-Ausgang (beim Candidate der Tap
        // post_candidate) bytegleich. Das Verhaeltnis Ausgang/Eingang und das
        // Sprungmass E-31 schreibt der Test selbst aus, nie aus dem Kern gelesen.
        // Gemeinsamer Aufbau (§6.2): 48 kHz, Blockgroesse 64, DC 0,5 auf beiden
        // Kanaelen.
        static_assert (kFadeSamples == 256 && kRampeSamples == 256,
                       "311/M-28, M-29 und M-35 schreiben ihre Kurven fuer K = 256 aus");
        const double fs  = 48000.0;
        const int    bs  = 64;
        const int    mb  = 512;
        const double x   = 0.5;
        const double K   = (double) kFadeSamples;
        const double g24 = std::pow (10.0, 24.0 / 20.0);

        const auto dc = [] (long long, float& l, float& r) { l = 0.5f; r = 0.5f; };

        // Faehrt `samples` Samples aus `quelle` in Bloecken zu `blockGroesse`
        // (der letzte kuerzer) und liefert den Ausgang, L und R verschraenkt.
        const auto fahre = [] (DspKern& k, const auto& quelle, long long& n0, int samples, int blockGroesse)
        {
            std::vector<float> a ((size_t) blockGroesse), b ((size_t) blockGroesse), aus;
            aus.reserve ((size_t) samples * 2u);
            float* kan[2] = { a.data(), b.data() };
            for (int rest = samples; rest > 0;)
            {
                const int m = std::min (rest, blockGroesse);
                for (int i = 0; i < m; ++i) quelle (n0 + i, a[(size_t) i], b[(size_t) i]);
                k.verarbeite (kan, 2, m);
                for (int i = 0; i < m; ++i) { aus.push_back (a[(size_t) i]); aus.push_back (b[(size_t) i]); }
                n0 += m;
                rest -= m;
            }
            return aus;
        };

        // Wie `fahre`, liefert aber die Taps post_committed und post_candidate
        // beider Kanaele (verschraenkt, double) aus DEMSELBEN Lauf. Fehlt ein
        // Tap, steht dort NaN - `endlich` unten faengt das.
        struct TapLauf { std::vector<double> committed, candidate; };
        const auto fahreTaps = [] (DspKern& k, const auto& quelle, long long& n0, int samples, int blockGroesse)
        {
            std::vector<float> a ((size_t) blockGroesse), b ((size_t) blockGroesse);
            TapLauf aus;
            float* kan[2] = { a.data(), b.data() };
            const auto haenge = [&k] (std::vector<double>& v, Tap t, int m)
            {
                const double* l = k.tap (t, 0);
                const double* r = k.tap (t, 1);
                for (int i = 0; i < m; ++i)
                {
                    v.push_back (l != nullptr ? l[i] : std::nan (""));
                    v.push_back (r != nullptr ? r[i] : std::nan (""));
                }
            };
            for (int rest = samples; rest > 0;)
            {
                const int m = std::min (rest, blockGroesse);
                for (int i = 0; i < m; ++i) quelle (n0 + i, a[(size_t) i], b[(size_t) i]);
                k.verarbeite (kan, 2, m);
                haenge (aus.committed, Tap::postCommitted, m);
                haenge (aus.candidate, Tap::postCandidate, m);
                n0 += m;
                rest -= m;
            }
            return aus;
        };

        // Der Eingang derselben Quelle ab `n0`, verschraenkt wie `fahre`.
        const auto eingang = [] (const auto& quelle, long long n0, int samples)
        {
            std::vector<float> ein;
            ein.reserve ((size_t) samples * 2u);
            for (int i = 0; i < samples; ++i)
            {
                float l = 0.0f, r = 0.0f;
                quelle (n0 + i, l, r);
                ein.push_back (l);
                ein.push_back (r);
            }
            return ein;
        };

        // Das Orakel: ein frischer Kern uebernimmt `ziel` am Blockrand und
        // bekommt ab dort denselben Eingang in derselben Blockteilung.
        const auto frischerKern = [&] (double rate, int maxBlock, const param::DspSatz& ziel, const auto& quelle,
                                       long long n0, int samples, int blockGroesse)
        {
            auto k = neuerKern (rate, maxBlock);
            k->uebernehmeZustand (ziel);
            return fahre (*k, quelle, n0, samples, blockGroesse);
        };

        const auto ohneEq = [] (param::DspSatz s)
        {
            s.werte[(size_t) param::kIndexEqEnabled].b = false;
            return s;
        };

        struct Wiedereinschalten
        {
            std::vector<float> aus, frisch, ein;   ///< ab dem Umschaltblock, L und R verschraenkt
            double    vorher   = 0.0;              ///< der letzte Ausgang (L) vor dem Umschaltblock
            long long umschalt = 0;                ///< Index des Umschaltsamples in der Quelle
        };

        // Der Aufbau aus M-23: `vorher` 1024 Samples eingeschwungen,
        // ausgeschaltet, Ausblenden und 512 Samples Ruhe; in der Ruhe jede
        // Publikation aus `inDerRuhe` (ENDE-Marken), je gefolgt von 256
        // Samples, damit ein Blockrand sie nimmt; dann `ziel` aus der Ruhe und
        // `laenge` Samples ab diesem Umschaltblock.
        const auto ausUndEin = [&] (double rate, int blockGroesse, int maxBlock, const param::DspSatz& vorher,
                                    const std::vector<param::DspSatz>& inDerRuhe, const param::DspSatz& ziel,
                                    const auto& quelle, int laenge)
        {
            Wiedereinschalten w;
            auto k = neuerKern (rate, maxBlock);
            long long n = 0;
            k->uebernehmeZustand (vorher);
            fahre (*k, quelle, n, 1024, blockGroesse);
            k->pflege();
            k->uebernehmeZustand (ohneEq (vorher));
            auto lauf = fahre (*k, quelle, n, kFadeSamples + 512, blockGroesse);
            k->pflege();
            for (const auto& s : inDerRuhe)
            {
                k->uebernehmeZustand (s);
                lauf = fahre (*k, quelle, n, 256, blockGroesse);
                k->pflege();
            }
            w.vorher   = (double) lauf[lauf.size() - 2];
            w.umschalt = n;
            k->uebernehmeZustand (ziel);
            w.aus    = fahre (*k, quelle, n, laenge, blockGroesse);
            w.frisch = frischerKern (rate, maxBlock, ziel, quelle, w.umschalt, laenge, blockGroesse);
            w.ein    = eingang (quelle, w.umschalt, laenge);
            return w;
        };

        // Erste bytegleich abweichende Stelle (Sample), -1 bei Gleichheit.
        const auto ersteAbweichung = [] (const std::vector<float>& a, const std::vector<float>& b) -> long long
        {
            if (a.size() != b.size() || a.empty()) return 0;
            for (size_t i = 0; i < a.size(); ++i)
                if (std::memcmp (&a[i], &b[i], sizeof (float)) != 0) return (long long) (i / 2);
            return -1;
        };
        const auto ersteTapAbweichung = [] (const std::vector<double>& a, const std::vector<double>& b) -> long long
        {
            if (a.size() != b.size() || a.empty()) return 0;
            for (size_t i = 0; i < a.size(); ++i)
                if (std::memcmp (&a[i], &b[i], sizeof (double)) != 0) return (long long) (i / 2);
            return -1;
        };
        const auto endlich = [] (const std::vector<double>& v)
        {
            return std::all_of (v.begin(), v.end(), [] (double d) { return std::isfinite (d); });
        };
        const auto abweichungText = [] (long long abw)
        {
            return abw < 0 ? std::string ("ja") : "nein, erste Abweichung an Sample " + std::to_string (abw);
        };

        // Groesstes Verhaeltnis |Ausgang/Eingang| und groesste Abweichung des
        // Verhaeltnisses von 1,0 (beide Kanaele, nur wo der Eingang nicht 0 ist).
        const auto spitzeVerhaeltnis = [] (const std::vector<float>& aus, const std::vector<float>& ein)
        {
            double m = 0.0;
            for (size_t i = 0; i < aus.size() && i < ein.size(); ++i)
                if (ein[i] != 0.0f) m = std::max (m, std::abs ((double) aus[i] / (double) ein[i]));
            return m;
        };
        const auto abweichungVonEins = [] (const std::vector<float>& aus, const std::vector<float>& ein)
        {
            double m = 0.0;
            for (size_t i = 0; i < aus.size() && i < ein.size(); ++i)
                if (ein[i] != 0.0f) m = std::max (m, std::abs ((double) aus[i] / (double) ein[i] - 1.0));
            return m;
        };

        // Groesster Nachbarsprung auf L ueber die ersten `bis` Samples, der
        // erste gegen `vorher`.
        const auto sprungL = [] (const std::vector<float>& aus, double vorher, size_t bis)
        {
            double m = 0.0, v = vorher;
            for (size_t i = 0; i < bis && 2 * i < aus.size(); ++i)
            {
                m = std::max (m, std::abs ((double) aus[2 * i] - v));
                v = (double) aus[2 * i];
            }
            return m;
        };
        const auto betragMax = [] (const std::vector<float>& aus)
        {
            double m = 0.0;
            for (float v : aus) m = std::max (m, std::abs ((double) v));
            return m;
        };

        // E-31 (W-4): die Rundungstoleranz einer Nachbarsample-Differenz am
        // float-Ausgang ist zwei Rasterschritte des float-Rasters. Unter 1,0 ist
        // das `kRundungFloat` (2^-23), wie die Matrix es nennt; ueber 1,0 waechst
        // das Raster mit dem Betrag - bei +24 dB und x = 0,5 liegen die
        // Ausgaenge bis 7,92, Raster 2^-21. Dieselbe Herleitung, am groessten
        // Betrag des Laufs.
        const auto rundung = [] (double betrag)
        {
            return betrag < 1.0 ? kRundungFloat : std::ldexp (1.0, std::ilogb (betrag) - 22);
        };

        auto an0  = machSatz (true);                              // Ziel: 0 dB, sonst nichts
        auto an24 = machSatz (true);
        setzeGlobal (an24, "v1.global.output_trim_db", 24.0);

        // Die Zusage aus M-23 (auch M-24): ab dem Umschaltblock bytegleich zum
        // frischen Kern, Ausgang/Eingang 1,0 an jedem Sample, E-31 mit
        // Fadeschrittweite 0 am Umschaltsample und ueber den ganzen Lauf.
        const auto pruefeWieM23 = [&] (const Wiedereinschalten& w, const std::string& name, const std::string& aufbau)
        {
            const long long abw  = ersteAbweichung (w.aus, w.frisch);
            const double spitze  = spitzeVerhaeltnis (w.aus, w.ein);
            const double vonEins = abweichungVonEins (w.aus, w.ein);
            const double amUmschalt = std::abs ((double) w.aus[0] - w.vorher);
            const double ueberAlles = sprungL (w.aus, w.vorher, w.aus.size() / 2);
            pruefe (abw < 0 && spitze <= 1.0 + 1e-6 && vonEins <= 1e-6
                        && amUmschalt <= kRundungFloat && ueberAlles <= kRundungFloat,
                    name, aufbau + ": bytegleich zum frischen Kern: " + abweichungText (abw)
                    + ", Spitze Ausgang/Eingang " + zahl (spitze, 7) + ", groesste Abweichung von 1,0 " + zahl (vonEins, 9)
                    + ", Sprung am Umschaltsample " + zahl (amUmschalt, 9) + ", groesster Sprung ueber "
                    + std::to_string (w.aus.size() / 2) + " Samples " + zahl (ueberAlles, 9) + " gegen 0 + 2^-23");
        };

        // M-23 Output-Trim, mit den Zahlenraendern der Rampenlaenge (M-35).
        {
            const auto w = ausUndEin (fs, bs, mb, an24, { ohneEq (an0) }, an0, dc, 1024);
            pruefeWieM23 (w, "311/M-23 wiedereinschalten_nach_ruhe_wie_frischer_kern (NAK-311 T3-15-05, M-03, E-31, M-07)",
                          "Output-Trim +24 dB, aus, in der Ruhe 0 dB, ein");

            bool exakt = true;
            std::string werte;
            for (const int i : { 0, 1, 127, 128, 255, 256 })
            {
                const double l = (double) w.aus[(size_t) (2 * i)] / x;
                const double r = (double) w.aus[(size_t) (2 * i + 1)] / x;
                exakt = exakt && l == 1.0 && r == 1.0;
                werte += (werte.empty() ? "" : " / ") + zahl (l, 7);
            }
            pruefe (exakt, "311/M-35 rampenlaenge_256_an_den_raendern (Teilfall von 311/M-23)",
                    "Ausgang/Eingang an den Indizes 0, 1, 127, 128, 255, 256: " + werte + " - verlangt exakt 1,0");
        }

        // M-24 Input-Trim.
        {
            auto in24 = machSatz (true);
            setzeGlobal (in24, "v1.global.input_trim_db", 24.0);
            const auto w = ausUndEin (fs, bs, mb, in24, { ohneEq (an0) }, an0, dc, 1024);
            pruefeWieM23 (w, "311/M-24 wiedereinschalten_nach_ruhe_wie_frischer_kern_input_trim (Teilfall von 311/M-23)",
                          "Input-Trim +24 dB, aus, in der Ruhe 0 dB, ein");
        }

        // M-25 Mix: +12-dB-Bell 1 kHz Q 1, Mix 0, 1-kHz-Sinus 0,25; in der Ruhe
        // Mix 1. Am Umschaltsample zaehlt der Sprung, den der Kern ZUSAETZLICH
        // zum Eigensprung des Sinus macht; die Fadeschrittweite ist der Abstand
        // des eingeblendeten Programms zum Eingang durch K, gemessen am frischen
        // Kern nach dem Fade.
        {
            const auto sinus = [fs] (long long n, float& l, float& r)
            {
                l = r = (float) (0.25 * std::sin (2.0 * kPiRef * 1000.0 * (double) n / fs));
            };
            auto band = machSatz (true);
            belege (band, 0, Filtertyp::bell, 1000.0, 1.0, 12.0);
            auto mix0 = band;
            setzeGlobal (mix0, "v2.global.mix", 0.0);
            const auto w = ausUndEin (fs, bs, mb, mix0, { ohneEq (band) }, band, sinus, 1024);

            float lv = 0.0f, rv = 0.0f;
            sinus (w.umschalt - 1, lv, rv);
            const double zusatz = std::abs (((double) w.aus[0] - w.vorher) - ((double) w.ein[0] - (double) lv));
            double abstand = 0.0;
            for (size_t i = (size_t) kFadeSamples; i < 2 * (size_t) kFadeSamples; ++i)
                abstand = std::max (abstand, std::abs ((double) w.frisch[2 * i] - (double) w.ein[2 * i]));
            const double stufe = abstand / K;
            const long long abw = ersteAbweichung (w.aus, w.frisch);
            pruefe (abw < 0 && zusatz <= stufe + rundung (betragMax (w.aus)) && abstand > 0.5,
                    "311/M-25 wiedereinschalten_nach_ruhe_wie_frischer_kern_mix (Teilfall von 311/M-23)",
                    "Bell +12 dB, Mix 0, aus, in der Ruhe Mix 1, ein: bytegleich zum frischen Kern mit Mix 1: "
                    + abweichungText (abw) + ", Zusatzsprung am Umschaltsample " + zahl (zusatz, 9)
                    + " gegen Fadeschritt " + zahl (stufe, 9) + " (Abstand Programm zu Eingang " + zahl (abstand, 5) + ")");
        }

        // M-26 Width: 2,0 auf reinem Seitensignal (L = 0,5 sin, R = -L); in der
        // Ruhe Width 1,0.
        {
            const auto seite = [fs] (long long n, float& l, float& r)
            {
                l = (float) (0.5 * std::sin (2.0 * kPiRef * 1000.0 * (double) n / fs));
                r = -l;
            };
            auto breit = machSatz (true);
            setzeGlobal (breit, "v1.global.width", 2.0);
            const auto w = ausUndEin (fs, bs, mb, breit, { ohneEq (an0) }, an0, seite, 1024);
            const long long abw = ersteAbweichung (w.aus, w.frisch);
            const double spitze = betragMax (w.aus);
            pruefe (abw < 0 && spitze <= 0.5 + 1e-6,
                    "311/M-26 wiedereinschalten_nach_ruhe_wie_frischer_kern_width (Teilfall von 311/M-23)",
                    "Width 2,0 auf Seitensignal, aus, in der Ruhe Width 1,0, ein: bytegleich zum frischen Kern: "
                    + abweichungText (abw) + ", Spitze " + zahl (spitze, 7) + " gegen 0,5 + 1e-6");
        }

        // M-27 Auto-Gain: High-Shelf 20 Hz Q 1 -12 dB (Ausgleich rund +12 dB,
        // Spiegel des M-37-Pruefllings aus H); in der Ruhe gain_db 0 - die
        // flache Kurve, Ausgleich 0 dB (M-36).
        {
            auto ag = machSatz (true);
            setzeGlobalBool (ag, "v2.global.auto_gain", true);
            belege (ag, 0, Filtertyp::highShelf, 20.0, 1.0, -12.0);
            auto flach = ag;
            flach.werte[(size_t) param::indexBandV1 (0, param::kGainDb)].zahl = 0.0;
            auto probe = neuerKern (fs, mb);
            probe->uebernehmeZustand (ag);
            const double ausgleichVorher = probe->autoGainDb();
            probe->uebernehmeZustand (flach);
            const double ausgleichFlach = probe->autoGainDb();

            const auto w = ausUndEin (fs, bs, mb, ag, { ohneEq (flach) }, flach, dc, 1024);
            const long long abw  = ersteAbweichung (w.aus, w.frisch);
            const double spitze  = spitzeVerhaeltnis (w.aus, w.ein);
            const double vonEins = abweichungVonEins (w.aus, w.ein);
            pruefe (abw < 0 && spitze <= 1.0 + 1e-6 && vonEins <= 1e-6 && ausgleichVorher > 11.0
                        && std::abs (ausgleichFlach) <= 1e-9,
                    "311/M-27 wiedereinschalten_nach_ruhe_wie_frischer_kern_auto_gain (Teilfall von 311/M-23)",
                    "Ausgleich vorher " + zahl (ausgleichVorher, 4) + " dB, flach " + zahl (ausgleichFlach, 4)
                    + " dB; bytegleich zum frischen Kern mit der flachen Kurve: " + abweichungText (abw)
                    + ", Spitze Ausgang/Eingang " + zahl (spitze, 7) + ", groesste Abweichung von 1,0 " + zahl (vonEins, 9));
        }

        // M-28 Rueckweg auf den alten Wert: in der Ruhe erst 0 dB, dann wieder
        // +24 dB; ein mit +24 dB. Die Kurve des frischen Kerns, ausgeschrieben:
        // Rampe ab 1,0 (erster Tick 1 + (g - 1)/K) mit dem Crossfade t = n/K,
        // also Ausgang/Eingang 1 + n(n + 1)(g - 1)/K^2 fuer n < K, danach g.
        {
            const auto w = ausUndEin (fs, bs, mb, an24, { ohneEq (an0), ohneEq (an24) }, an24, dc, 1024);
            const double tol = rundung (betragMax (w.aus));

            const double amUmschalt = std::abs ((double) w.aus[0] - w.vorher);
            const double schranke   = 2.0 * (K - 1.0) * (g24 - 1.0) * x / (K * K);
            const double ueberAlles = sprungL (w.aus, w.vorher, w.aus.size() / 2);
            pruefe (amUmschalt <= kRundungFloat && ueberAlles <= schranke + tol,
                    "311/M-28 wiedereinschalten_nach_ruhe_wie_frischer_kern_rueckweg_alter_wert: E-31-Mass (Teilfall von 311/M-23)",
                    "Sprung am Umschaltsample " + zahl (amUmschalt, 9) + " gegen 2^-23, groesster Sprung ueber den Lauf "
                    + zahl (ueberAlles, 9) + " gegen 2(K-1)(g-1)x/K^2 = " + zahl (schranke, 9) + " + Rundung " + zahl (tol, 9));

            const long long abw = ersteAbweichung (w.aus, w.frisch);
            double kurve = 0.0;
            for (size_t i = 0; i < w.aus.size(); ++i)
            {
                const double n = (double) (i / 2);
                const double soll = n < K ? 1.0 + n * (n + 1.0) * (g24 - 1.0) / (K * K) : g24;
                kurve = std::max (kurve, std::abs ((double) w.aus[i] - x * soll));
            }
            pruefe (abw < 0 && kurve <= tol,
                    "311/M-28 wiedereinschalten_nach_ruhe_wie_frischer_kern_rueckweg_alter_wert: bytegleich (Teilfall von 311/M-23)",
                    "in der Ruhe 0 dB, dann +24 dB, ein mit +24 dB: bytegleich zum frischen Kern: " + abweichungText (abw)
                    + ", groesste Abweichung von x(1 + n(n+1)(g-1)/K^2) " + zahl (kurve, 9) + " gegen " + zahl (tol, 9)
                    + "; Ausgang/Eingang bei n = 1: " + zahl ((double) w.aus[2] / x, 7) + " (Kurve 1,000453)");
        }

        // M-29 Ausschalten im laufenden Crossfade: aus der Ruhe ein mit +24 dB,
        // 64 Samples spaeter aus (die ENDE-Marke wartet, E-17), nach dem
        // Ausblenden in der Ruhe 0 dB, dann ein mit 0 dB. Die Kurve des ganzen
        // Laufs ist hier ausgeschrieben: Einblenden 1 + n(n + 1)(g - 1)/K^2,
        // ab K das Ausblenden g(1 - m/K) + m/K mit m = n - K, ab 2K der Eingang.
        {
            auto k = neuerKern (fs, mb);
            long long n = 0;
            fahre (*k, dc, n, 512, bs);
            k->uebernehmeZustand (an24);
            auto lauf = fahre (*k, dc, n, 64, bs);
            k->uebernehmeZustand (ohneEq (an24));
            const auto weiter = fahre (*k, dc, n, 1024 - 64, bs);
            lauf.insert (lauf.end(), weiter.begin(), weiter.end());
            k->pflege();
            k->uebernehmeZustand (ohneEq (an0));
            const auto ruhe = fahre (*k, dc, n, 256, bs);
            k->pflege();
            const double vorher = (double) ruhe[ruhe.size() - 2];
            const long long umschalt = n;
            k->uebernehmeZustand (an0);
            const auto wieder = fahre (*k, dc, n, 1024, bs);
            const auto frisch = frischerKern (fs, mb, an0, dc, umschalt, 1024, bs);

            const double tol   = rundung (betragMax (lauf));
            const double stufe = (g24 - 1.0) * x / K;                       // Fadeschritt von Ein- und Ausblenden
            double kurve = 0.0;
            for (size_t i = 0; i < lauf.size(); ++i)
            {
                const double s = (double) (i / 2);
                double soll = 1.0;
                if (s < K)            soll = 1.0 + s * (s + 1.0) * (g24 - 1.0) / (K * K);
                else if (s < 2.0 * K) soll = g24 * (1.0 - (s - K) / K) + (s - K) / K;
                kurve = std::max (kurve, std::abs ((double) lauf[i] - x * soll));
            }
            const size_t k1 = (size_t) kFadeSamples, k2 = 2 * (size_t) kFadeSamples;
            const double amEinblenden  = std::abs ((double) lauf[0] - x);
            const double amAusblenden  = std::abs ((double) lauf[2 * k1] - (double) lauf[2 * k1 - 2]);
            const double amRuhebeginn  = std::abs ((double) lauf[2 * k2] - (double) lauf[2 * k2 - 2]);
            const double amWiederein   = std::abs ((double) wieder[0] - vorher);
            const double ueberAlles    = sprungL (lauf, x, lauf.size() / 2);
            const double wache         = std::max (2.0 * (K - 1.0) * (g24 - 1.0) * x / (K * K), stufe);
            pruefe (amEinblenden <= stufe + tol && amAusblenden <= stufe + tol && amRuhebeginn <= stufe + tol
                        && amWiederein <= kRundungFloat && ueberAlles <= wache + tol && kurve <= tol,
                    "311/M-29 wiedereinschalten_nach_ruhe_wie_frischer_kern_aus_im_laufenden_crossfade: E-17-Folge (Teilfall von 311/M-23)",
                    "Spruenge am Umschaltsample: Einblenden " + zahl (amEinblenden, 9) + ", Ende Einblenden = Beginn Ausblenden "
                    + zahl (amAusblenden, 9) + ", Ruhebeginn " + zahl (amRuhebeginn, 9) + " gegen Fadeschritt " + zahl (stufe, 9)
                    + " + " + zahl (tol, 9) + "; Wiedereinschalten " + zahl (amWiederein, 9) + " gegen 2^-23; groesster Sprung "
                    + zahl (ueberAlles, 9) + " gegen " + zahl (wache, 9) + "; das Ausblenden beginnt bei Sample 256 "
                    "(groesste Abweichung von der ausgeschriebenen Kurve " + zahl (kurve, 9) + ")");

            const long long abw = ersteAbweichung (wieder, frisch);
            const double spitze = spitzeVerhaeltnis (wieder, eingang (dc, umschalt, 1024));
            pruefe (abw < 0 && spitze <= 1.0 + 1e-6,
                    "311/M-29 wiedereinschalten_nach_ruhe_wie_frischer_kern_aus_im_laufenden_crossfade: Wiedereinschalten (Teilfall von 311/M-23)",
                    "ein mit 0 dB nach dem Ausblenden: bytegleich zum frischen Kern: " + abweichungText (abw)
                    + ", Spitze Ausgang/Eingang " + zahl (spitze, 7));
        }

        // M-30 Gegenfall Hard-Bypass (M-06 unveraendert): im Hard-Bypass bleibt
        // eine Bank aktiv; der Trimwechsel dort ist ein Crossfade zwischen zwei
        // Passthrough-Baenken, und die Rampen laufen auf das neue Ziel, weil der
        // nicht rechnende Pfad sie tickt.
        {
            auto k = neuerKern (fs, mb);
            long long n = 0;
            k->uebernehmeZustand (an24);
            fahre (*k, dc, n, 1024, bs);
            k->pflege();
            auto by24 = an24;
            setzeGlobalBool (by24, "v1.global.bypass", true);
            k->uebernehmeZustand (by24);
            fahre (*k, dc, n, kFadeSamples + 512, bs);
            k->pflege();
            auto by0 = an0;
            setzeGlobalBool (by0, "v1.global.bypass", true);
            k->uebernehmeZustand (by0);
            const auto imBypass = fahre (*k, dc, n, kFadeSamples + 512, bs);
            k->pflege();
            const double vorher = (double) imBypass[imBypass.size() - 2];
            const long long umschalt = n;
            k->uebernehmeZustand (an0);
            const auto aus = fahre (*k, dc, n, 1024, bs);
            const double spitze = spitzeVerhaeltnis (aus, eingang (dc, umschalt, 1024));
            const double amUmschalt = std::abs ((double) aus[0] - vorher);
            pruefe (spitze <= 1.0 + 1e-6 && amUmschalt <= kRundungFloat,
                    "311/M-30 trimwechsel_im_hard_bypass (Gegenfall M-06, E-7)",
                    "+24 dB, Hard-Bypass, im Bypass 0 dB, Bypass aus: Spitze Ausgang/Eingang " + zahl (spitze, 7)
                    + " gegen 1 + 1e-6, Sprung am Umschaltsample " + zahl (amUmschalt, 9) + " gegen 0 + 2^-23");
        }

        // M-31 Gegenfall Hoermatrix-Halt (E-33, M-55 unveraendert): der Aufbau
        // der X-1-Faelle aus L (Committed und Candidate Output-Trim +6 dB, DC
        // 0,3, Blockgroesse 64), Candidate-Ende 64 Samples in das Einblenden der
        // Hoermatrix (Fall (a)). Im Halt klingt die endende Bank mit IHREN Rampen
        // - der Tap post_candidate gleicht post_committed. Nach dem Ausdienen
        // kommt ein zweiter Candidate mit 0 dB aus der Ruhe.
        {
            const auto dc3 = [] (long long, float& l, float& r) { l = 0.3f; r = 0.3f; };
            auto laut = machSatz (true);
            setzeGlobal (laut, "v1.global.output_trim_db", 6.0);
            auto k = neuerKern (fs, 64);
            long long n = 0;
            k->uebernehmeZustand (laut);
            k->uebernehmeZustand (laut, Pfad::candidate);
            fahre (*k, dc3, n, 4096, 64);
            k->pflege();
            k->setzeHoermatrix (Hoermatrix::candidate);
            fahre (*k, dc3, n, 64, 64);
            k->beendeCandidate();

            const auto halt = fahreTaps (*k, dc3, n, 32, 64);
            double haltAbw = 0.0;
            for (size_t i = 0; i < halt.candidate.size() && i < halt.committed.size(); ++i)
                haltAbw = std::max (haltAbw, std::abs (halt.candidate[i] - halt.committed[i]));
            int cA = -1, cQ = -1, kA = -1, kQ = -1;
            k->gefahreneSlots (cA, cQ, kA, kQ);
            const bool gehalten = kA == -1 && kQ >= 0;
            pruefe (gehalten && endlich (halt.candidate) && endlich (halt.committed) && haltAbw <= 1e-12,
                    "311/M-31 zweiter_candidate_nach_hoerhalt_wie_frisch: Hoerhalt (Gegenfall E-33)",
                    std::string ("32 Samples nach dem Candidate-Ende gehalten: ") + (gehalten ? "ja" : "nein")
                    + ", groesste Abweichung post_candidate zu post_committed " + zahl (haltAbw, 12)
                    + " - die endende Bank klingt mit ihren Rampen (+6 dB)");

            fahre (*k, dc3, n, 2048, 64);
            k->pflege();
            k->gefahreneSlots (cA, cQ, kA, kQ);
            const bool ausgedient = kA == -1 && kQ == -1;
            auto kand0 = machSatz (true);
            const long long umschalt = n;
            k->uebernehmeZustand (kand0, Pfad::candidate);
            const auto tap = fahreTaps (*k, dc3, n, 1024, 64).candidate;

            auto frisch = neuerKern (fs, 64);
            frisch->uebernehmeZustand (laut);
            frisch->uebernehmeZustand (kand0, Pfad::candidate);
            long long nf = umschalt;
            const auto tapFrisch = fahreTaps (*frisch, dc3, nf, 1024, 64).candidate;
            const long long abw = ersteTapAbweichung (tap, tapFrisch);
            pruefe (ausgedient && endlich (tap) && abw < 0,
                    "311/M-31 zweiter_candidate_nach_hoerhalt_wie_frisch: zweiter Candidate",
                    std::string ("nach dem Halt ausgedient: ") + (ausgedient ? "ja" : "nein")
                    + "; zweiter Candidate 0 dB aus der Ruhe, Tap post_candidate bytegleich zum frischen Kern: "
                    + abweichungText (abw) + (tap.size() > 2 ? ", Sample 1: " + zahl (tap[2] / 0.3, 7) + " x Eingang" : std::string()));
        }

        // M-32 und M-33: `beendeAudiohistorie` 64 Samples in das Ausblenden - der
        // Uebergang endet dort, der Pfad ruht. M-32 publiziert danach in der Ruhe
        // eine Aenderung (ENDE-Marke), M-33 schaltet gleich wieder ein.
        for (int fall = 0; fall < 2; ++fall)
        {
            auto k = neuerKern (fs, mb);
            long long n = 0;
            k->uebernehmeZustand (an24);
            fahre (*k, dc, n, 1024, bs);
            k->pflege();
            k->uebernehmeZustand (ohneEq (an24));
            fahre (*k, dc, n, 64, bs);
            int cA = -1, cQ = -1, kA = -1, kQ = -1;
            k->gefahreneSlots (cA, cQ, kA, kQ);
            const bool ausblendend = cA == -1 && cQ >= 0;
            k->beendeAudiohistorie();
            k->gefahreneSlots (cA, cQ, kA, kQ);
            const bool ruht = cA == -1 && cQ == -1;
            if (fall == 0)
            {
                fahre (*k, dc, n, 512, bs);
                k->pflege();
                k->uebernehmeZustand (ohneEq (an0));
                fahre (*k, dc, n, 256, bs);
                k->pflege();
            }
            const long long umschalt = n;
            k->uebernehmeZustand (an0);
            const auto aus    = fahre (*k, dc, n, 1024, bs);
            const auto frisch = frischerKern (fs, mb, an0, dc, umschalt, 1024, bs);
            const long long abw = ersteAbweichung (aus, frisch);
            const double spitze = spitzeVerhaeltnis (aus, eingang (dc, umschalt, 1024));
            pruefe (ausblendend && ruht && abw < 0 && spitze <= 1.0 + 1e-6,
                    fall == 0 ? "311/M-32 wiedereinschalten_nach_ruhe_wie_frischer_kern_aenderung_in_der_ruhe_nach_beende_audiohistorie (Teilfall von 311/M-23)"
                              : "311/M-33 wiedereinschalten_nach_ruhe_wie_frischer_kern_beende_audiohistorie_im_ausblenden (Teilfall von 311/M-23)",
                    std::string ("Output-Trim +24 dB, aus, beendeAudiohistorie 64 Samples ins Ausblenden (ausblendend: ")
                    + (ausblendend ? "ja" : "nein") + ", danach ruhend: " + (ruht ? "ja" : "nein")
                    + (fall == 0 ? "), 512 Samples Ruhe, in der Ruhe 0 dB, ein mit 0 dB" : "), gleich ein mit 0 dB")
                    + ": bytegleich zum frischen Kern: " + abweichungText (abw) + ", Spitze Ausgang/Eingang " + zahl (spitze, 7));
        }

        // M-36 Blockgroessen 1, 255, 256, 257, 180 und 4096 (maxBlock 512, 4096
        // in Stuecken): ab dem Umschaltblock 4096 Samples, bytegleich zum
        // frischen Kern derselben Blockteilung und zur Blockgroesse 1.
        {
            std::vector<float> ersteAus;
            for (const int b : { 1, 255, 256, 257, 180, 4096 })
            {
                const auto w = ausUndEin (fs, b, mb, an24, { ohneEq (an0) }, an0, dc, 4096);
                if (ersteAus.empty()) ersteAus = w.aus;
                const long long abw  = ersteAbweichung (w.aus, w.frisch);
                const long long abw1 = ersteAbweichung (w.aus, ersteAus);
                pruefe (abw < 0 && abw1 < 0,
                        "311/M-36 wiedereinschalten_nach_ruhe_wie_frischer_kern (Teilfall von 311/M-23): Blockgroesse "
                        + std::to_string (b),
                        "bytegleich zum frischen Kern: " + abweichungText (abw) + ", zur Blockgroesse 1: " + abweichungText (abw1)
                        + ", Spitze Ausgang/Eingang " + zahl (spitzeVerhaeltnis (w.aus, w.ein), 7));
            }
        }

        // M-37 Sampleraten 44,1, 48 und 96 kHz: die Rampe zaehlt Samples.
        for (const double rate : { 44100.0, 48000.0, 96000.0 })
        {
            const auto w = ausUndEin (rate, bs, mb, an24, { ohneEq (an0) }, an0, dc, 1024);
            const long long abw = ersteAbweichung (w.aus, w.frisch);
            pruefe (abw < 0,
                    "311/M-37 wiedereinschalten_nach_ruhe_wie_frischer_kern (Teilfall von 311/M-23): " + zahl (rate / 1000.0, 1) + " kHz",
                    "bytegleich zum frischen Kern: " + abweichungText (abw) + ", Spitze Ausgang/Eingang "
                    + zahl (spitzeVerhaeltnis (w.aus, w.ein), 7));
        }

        // M-38 Candidate-Pfad (E-16): Candidate +24 dB bei Hoermatrix Processed,
        // `beendeCandidate` blendet ihn in die Ruhe; ein neuer Candidate mit
        // 0 dB. Der Committed-Pfad (+6 dB) laeuft daneben unberuehrt: sein Tap
        // bleibt ueber den ganzen Candidate-Wechsel bytegleich auf dem Wert davor.
        {
            auto com = machSatz (true);
            setzeGlobal (com, "v1.global.output_trim_db", 6.0);
            auto k = neuerKern (fs, mb);
            long long n = 0;
            k->uebernehmeZustand (com);
            k->uebernehmeZustand (an24, Pfad::candidate);
            fahre (*k, dc, n, 2048, bs);
            k->pflege();
            const double* pk = k->tap (Tap::postCommitted, 0);
            const double comVorher = pk != nullptr ? pk[bs - 1] : std::nan ("");
            k->beendeCandidate();
            auto comLauf = fahreTaps (*k, dc, n, kFadeSamples + 512, bs).committed;
            k->pflege();
            int cA = -1, cQ = -1, kA = -1, kQ = -1;
            k->gefahreneSlots (cA, cQ, kA, kQ);
            const bool ruht = kA == -1 && kQ == -1;
            const long long umschalt = n;
            k->uebernehmeZustand (an0, Pfad::candidate);
            const auto wieder = fahreTaps (*k, dc, n, 1024, bs);
            const auto& tap = wieder.candidate;
            comLauf.insert (comLauf.end(), wieder.committed.begin(), wieder.committed.end());
            bool comUnberuehrt = std::isfinite (comVorher);
            for (double v : comLauf) if (std::memcmp (&v, &comVorher, sizeof (double)) != 0) comUnberuehrt = false;

            auto frisch = neuerKern (fs, mb);
            frisch->uebernehmeZustand (com);
            frisch->uebernehmeZustand (an0, Pfad::candidate);
            long long nf = umschalt;
            const auto tapFrisch = fahreTaps (*frisch, dc, nf, 1024, bs).candidate;
            const long long abw = ersteTapAbweichung (tap, tapFrisch);
            pruefe (ruht && endlich (tap) && abw < 0 && comUnberuehrt,
                    "311/M-38 candidate_nach_ende_wie_frisch (E-16, B-6, B-8)",
                    std::string ("Candidate +24 dB beendet, ruhend: ") + (ruht ? "ja" : "nein")
                    + "; neuer Candidate 0 dB, Tap post_candidate bytegleich zum frischen Kern: " + abweichungText (abw)
                    + (tap.size() > 2 ? " (Sample 1: " + zahl (tap[2] / x, 7) + " x Eingang)" : std::string())
                    + "; Committed-Tap ueber " + std::to_string (comLauf.size() / 2) + " Samples bytegleich auf "
                    + zahl (comVorher, 9) + ": " + (comUnberuehrt ? "ja" : "nein"));
        }
    }

    //==========================================================================
    std::cout << std::endl << "== Q - Slot-Lebenszyklus (NAK-311 W03) ==" << std::endl;
    {
        // NAK-311 T3-14-02 und T3-15-08 (Manifest NAK-311 §6.3, R-311-1, R-311-8,
        // R-311-9): ein Slot, dessen Belegung und Topologie ueber jede Publikation
        // seit dem gefahrenen Programm gleich blieben, behaelt im Crossfade seinen
        // Filter-, Detektor- und Huellkurvenzustand; ein geaenderter Slot und jeder
        // Slot nach einem globalen Wechsel startet kalt und meldet 0,0 (SONDE-015
        // M-121, E-8). Verglichen wird gegen einen Referenzkern mit identischer
        // Vorgeschichte ohne Wechsel; Mass und Fenster wie M-43 (F-9). Die Faelle
        // M-43 bis M-45, M-54 und M-55 stehen in Abschnitt E, M-41 im M-121-Block
        // von Abschnitt L, M-56 im Allokationslauf von Abschnitt J.
        const double fs = 48000.0;
        const int    bs = 64;
        const int    mb = 512;
        const int    periode50 = 960;
        const double w1k = 2.0 * kPiRef * 1000.0 / fs;
        const double w50 = 2.0 * kPiRef * 50.0 / fs;

        // Der Aufbau aus M-46: Slot 0 dynamisches Bell wie M-40 (1 kHz Q 2,
        // Range -12 dB, Threshold -40 dB, Attack 0,1 ms, Hold 500 ms, Release
        // 5000 ms), Slot 1 statisches Bell 50 Hz Q 8 +12 dB. Vorgeschichte: der
        // 1-kHz-Quadraturton 0,5 (Detektor -9 dB, Plateau -12 dB) und der
        // 50-Hz-Sinus 0,25 auf beiden Kanaelen, 2 s bis zu einer Periodengrenze.
        auto basis = machSatz (true);
        belege (basis, 0, Filtertyp::bell, 1000.0, 2.0, 0.0);
        machDynamisch (basis, 0, -12.0, -40.0, 0.1, 500.0, 5000.0);
        belege (basis, 1, Filtertyp::bell, 50.0, 8.0, 12.0);

        const auto beideToene = [w1k, w50] (long long n, float& l, float& r)
        {
            const double t50 = 0.25 * std::sin (w50 * (double) n);
            l = (float) (0.5 * std::sin (w1k * (double) n) + t50);
            r = (float) (0.5 * std::cos (w1k * (double) n) + t50);
        };
        const auto nur50  = [w50] (long long n, float& l, float& r) { l = r = (float) (0.25 * std::sin (w50 * (double) n)); };
        const auto stille = [] (long long, float& l, float& r) { l = 0.0f; r = 0.0f; };
        const int vorgeschichte = 100 * periode50;   // 2 s, endet auf einer Periodengrenze

        // Die betragsgroesste gemeldete Auslenkung eines Slots ueber alle Bloecke
        // des Laufs, mit Vorzeichen; 0,0 heisst: jeder Block meldete 0,0.
        const auto slotMax = [] (const W03Lauf& l, int slot)
        {
            double m = 0.0;
            for (const auto& a : l.auslenkungen)
                if (std::abs (a[(size_t) slot]) > std::abs (m)) m = a[(size_t) slot];
            return m;
        };
        const auto spitzeAb = [] (const std::vector<double>& x, size_t von)
        {
            double m = 0.0;
            for (size_t i = von; i < x.size(); ++i) m = std::max (m, std::abs (x[i]));
            return m;
        };
        const auto mitTyp = [] (param::DspSatz s, int slot, Filtertyp t)
        {
            s.werte[(size_t) param::indexBandV1 (slot, param::kType)].enumIndex = (int) t;
            return s;
        };

        // M-46: Typwechsel an Slot 0 (Bell -> Low-Shelf, dynamisch bleibt); ab dem
        // Umschaltblock nur noch der 50-Hz-Sinus - der Detektor von Slot 0 hoert
        // ihn rund 32 dB gedaempft (-47 dB Leistung, unter Threshold -40 dB).
        {
            const auto e = fahrePaar (fs, mb, basis, mitTyp (basis, 0, Filtertyp::lowShelf), Pfad::committed,
                                      beideToene, vorgeschichte, bs, nur50, 100 * periode50, bs);
            const double slot0 = slotMax (e.test, 0);
            double erste = 0.0;
            int schlimmste = -1;
            const double mass = periodenMass (e.test.aus, e.ref.aus, 2u * (size_t) periode50, 100, &erste, &schlimmste);
            pruefe (e.auslenkungVorher[0] == -12.0 && slot0 == 0.0 && e.uebernahmenTest == 1,
                    "311/M-46 geaenderter_slot_kalt_unveraenderter_warm: Slot 0 kalt (SONDE-015 M-121, E-8)",
                    "Slot 0 vorher " + zahl (e.auslenkungVorher[0], 4) + " dB, nach dem Typwechsel betragsgroesste Meldung ab dem ersten Block "
                    + zahl (slot0, 6) + " dB ueber " + std::to_string (e.test.auslenkungen.size()) + " Bloecke");
            pruefe (mass <= 0.5,
                    "311/M-46 geaenderter_slot_kalt_unveraenderter_warm: Slot 1 warm (R-311-1)",
                    "Mass " + zahl (mass, 6) + " dB gegen 0,5 dB (erste Periode " + zahl (erste, 6)
                    + " dB, groesste Abweichung in Periode " + std::to_string (schlimmste) + ")");
        }

        // M-47: (a) Remove an Slot 0, (b) dynamic_enabled aus an Slot 0; Slot 1
        // wie M-46, derselbe Eingang.
        {
            auto entfernt = basis;
            entfernt.werte[(size_t) param::indexOccupied (0)].b = false;
            entfernt.werte[(size_t) param::indexBandV1 (0, param::kEnabled)].b = false;
            auto ohneDynamik = basis;
            ohneDynamik.werte[(size_t) param::indexBandV1 (0, param::kDynamicEnabled)].b = false;
            struct Unterfall { const char* name; const param::DspSatz* satz; };
            const Unterfall faelle[] = { { "(a) Remove", &entfernt }, { "(b) dynamic_enabled aus", &ohneDynamik } };
            for (const auto& u : faelle)
            {
                const auto e = fahrePaar (fs, mb, basis, *u.satz, Pfad::committed, beideToene, vorgeschichte, bs,
                                          nur50, 100 * periode50, bs);
                const double slot0 = slotMax (e.test, 0);
                double erste = 0.0;
                int schlimmste = -1;
                const double mass = periodenMass (e.test.aus, e.ref.aus, 2u * (size_t) periode50, 100, &erste, &schlimmste);
                pruefe (e.auslenkungVorher[0] == -12.0 && slot0 == 0.0 && e.uebernahmenTest == 1,
                        std::string ("311/M-47 remove_und_dynamik_aus_melden_null: ") + u.name + ", Meldung 0 (SONDE-015 M-121)",
                        "Slot 0 vorher " + zahl (e.auslenkungVorher[0], 4) + " dB, danach betragsgroesste Meldung ab dem ersten Block "
                        + zahl (slot0, 6) + " dB");
                pruefe (mass <= 0.5,
                        std::string ("311/M-47 remove_und_dynamik_aus_melden_null: ") + u.name + ", Slot 1 warm (R-311-1)",
                        "Mass " + zahl (mass, 6) + " dB gegen 0,5 dB (erste Periode " + zahl (erste, 6)
                        + " dB, groesste Abweichung in Periode " + std::to_string (schlimmste) + ")");
            }
        }

        // M-48: globale Wechsel machen jeden Slot kalt - (a) Mono-Bass 0 -> 120 Hz
        // (Crossfade, E-20), (b) `bereiteVor` mit neuer Samplerate 44,1 -> 48 kHz
        // und demselben Programm. Ab dem Umschaltblock Stille.
        {
            auto monoBass = basis;
            setzeGlobal (monoBass, "v1.global.mono_bass_hz", 120.0);
            const auto e = fahrePaar (fs, mb, basis, monoBass, Pfad::committed, beideToene, vorgeschichte, bs,
                                      stille, 4096, bs);
            double alle = 0.0;
            for (int slot = 0; slot < param::kSlots; ++slot) if (std::abs (slotMax (e.test, slot)) > std::abs (alle)) alle = slotMax (e.test, slot);
            const double spitze = spitzeAb (e.test.aus, 2u * (size_t) kFadeSamples);
            pruefe (e.auslenkungVorher[0] == -12.0 && alle == 0.0 && spitze == 0.0 && e.uebernahmenTest == 1,
                    "311/M-48 globaler_wechsel_macht_alle_slots_kalt: (a) Mono-Bass 0 -> 120 Hz (R-311-1, E-20)",
                    "Slot 0 vorher " + zahl (e.auslenkungVorher[0], 4) + " dB; danach betragsgroesste Auslenkung aller Slots ab dem ersten Block "
                    + zahl (alle, 6) + " dB, Spitze bei Stille ab Sample " + std::to_string (kFadeSamples) + ": " + zahl (spitze, 9));

            auto k = neuerKern (44100.0, mb);
            k->uebernehmeZustand (basis);
            const double w1k441 = 2.0 * kPiRef * 1000.0 / 44100.0;
            const double w50441 = 2.0 * kPiRef * 50.0 / 44100.0;
            long long n = 0;
            fahreW03 (*k, [w1k441, w50441] (long long i, float& l, float& r)
                      {
                          const double t50 = 0.25 * std::sin (w50441 * (double) i);
                          l = (float) (0.5 * std::sin (w1k441 * (double) i) + t50);
                          r = (float) (0.5 * std::cos (w1k441 * (double) i) + t50);
                      }, n, 88200, bs, nullptr);
            double vorher[param::kSlots];
            k->auslenkungenDb (vorher);
            k->bereiteVor (48000.0, mb);
            k->uebernehmeZustand (basis);
            W03Lauf lauf;
            fahreW03 (*k, stille, n, 4096, bs, &lauf);
            double alleB = 0.0;
            for (int slot = 0; slot < param::kSlots; ++slot) if (std::abs (slotMax (lauf, slot)) > std::abs (alleB)) alleB = slotMax (lauf, slot);
            const double spitzeB = spitzeAb (lauf.aus, 0);
            pruefe (vorher[0] == -12.0 && alleB == 0.0 && spitzeB == 0.0,
                    "311/M-48 globaler_wechsel_macht_alle_slots_kalt: (b) bereiteVor 44,1 -> 48 kHz, dasselbe Programm (R-311-1)",
                    "Slot 0 vorher " + zahl (vorher[0], 4) + " dB; danach betragsgroesste Auslenkung aller Slots " + zahl (alleB, 6)
                    + " dB, Spitze bei Stille " + zahl (spitzeB, 9));
        }

        // M-49: Zahlenrand Slotzahl 0 und 8. Acht dynamische Bells (Range -12 dB,
        // Threshold -40 dB, Attack 500 ms, Hold 500 ms, Release 5000 ms, Q 2,
        // Mitten eine Oktave auseinander), je mit eigenem Quadraturton 0,15 auf
        // der Bandmitte eingeschwungen (Plateau -12 dB); ab dem Umschaltblock
        // Stille. Die Range steht ausdruecklich: mit der Vorgabe 0 dB liefe kein
        // Detektor, und (b) und (c) verglichen nur Nullen. Die ATTACK nennt die
        // Matrixzeile nicht; sie steht hier auf 500 ms, weil der kalt startende
        // Slot 7 in (b) HINTER sieben uebertragenen Baendern liegt: deren
        // Ausklingen traegt bei Stille noch Energie auf seiner Bandmitte
        // (gemessen: Detektorleistung 0,0056, rund -22 dB), und eine 0,1-ms-
        // Huellkurve stuende damit binnen weniger Samples wieder auf dem
        // Plateau - "Slot 7 meldet 0,0" waere nicht messbar. Mit 500 ms bleibt
        // seine Huellkurve im Fenster unter dem Threshold.
        {
            const double mitten[param::kSlots] = { 62.5, 125.0, 250.0, 500.0, 1000.0, 2000.0, 4000.0, 8000.0 };
            auto acht = machSatz (true);
            for (int slot = 0; slot < param::kSlots; ++slot)
            {
                belege (acht, slot, Filtertyp::bell, mitten[slot], 2.0, 0.0);
                machDynamisch (acht, slot, -12.0, -40.0, 500.0, 500.0, 5000.0);
            }
            const auto achtToene = [&mitten, fs] (long long n, float& l, float& r)
            {
                double a = 0.0, b = 0.0;
                for (double f : mitten)
                {
                    const double w = 2.0 * kPiRef * f / fs;
                    a += 0.15 * std::sin (w * (double) n);
                    b += 0.15 * std::cos (w * (double) n);
                }
                l = (float) a;
                r = (float) b;
            };

            // (a) Programm ohne aktives Band; ein Band kommt hinzu.
            {
                auto eines = machSatz (true);
                belege (eines, 4, Filtertyp::bell, 1000.0, 2.0, 0.0);
                machDynamisch (eines, 4, -12.0, -40.0, 0.1, 500.0, 5000.0);
                const auto e = fahrePaar (fs, mb, machSatz (true), eines, Pfad::committed, achtToene, vorgeschichte, bs,
                                          stille, 4096, bs);
                double alle = 0.0;
                for (int slot = 0; slot < param::kSlots; ++slot) if (std::abs (slotMax (e.test, slot)) > std::abs (alle)) alle = slotMax (e.test, slot);
                const double spitze = spitzeAb (e.test.aus, 2u * (size_t) kFadeSamples);
                pruefe (alle == 0.0 && spitze == 0.0 && e.uebernahmenTest == 1,
                        "311/M-49 slotzahl_null_und_acht: (a) null Baender, eines kommt hinzu und startet kalt",
                        "betragsgroesste Auslenkung aller Slots ab dem ersten Block " + zahl (alle, 6) + " dB, Spitze bei Stille ab Sample "
                        + std::to_string (kFadeSamples) + ": " + zahl (spitze, 9));
            }

            // (b) Typwechsel nur an Slot 7 (Bell -> High-Shelf, dynamisch bleibt).
            {
                const auto e = fahrePaar (fs, mb, acht, mitTyp (acht, 7, Filtertyp::highShelf), Pfad::committed, achtToene,
                                          vorgeschichte, bs, stille, 4096, bs);
                bool vorbedingung = true;
                for (double a : e.auslenkungVorher) if (a != -12.0) vorbedingung = false;
                bool bitgleich = e.test.auslenkungen.size() == e.ref.auslenkungen.size();
                long long ersterBlock = -1;
                for (size_t blk = 0; bitgleich && blk < e.test.auslenkungen.size(); ++blk)
                    if (std::memcmp (e.test.auslenkungen[blk].data(), e.ref.auslenkungen[blk].data(), 7 * sizeof (double)) != 0)
                    { bitgleich = false; ersterBlock = (long long) blk; }
                const double slot7 = slotMax (e.test, 7);
                pruefe (vorbedingung && bitgleich && slot7 == 0.0 && e.uebernahmenTest == 1,
                        "311/M-49 slotzahl_null_und_acht: (b) Typwechsel nur an Slot 7 - Slots 0 bis 6 uebertragen, Slot 7 kalt",
                        std::string ("vorher alle acht bei -12 dB: ") + (vorbedingung ? "ja" : "nein")
                        + "; Auslenkungen der Slots 0 bis 6 je Block bitgleich zum Referenzkern ueber "
                        + std::to_string (e.test.auslenkungen.size()) + " Bloecke: "
                        + (bitgleich ? std::string ("ja") : "nein, erster Block " + std::to_string (ersterBlock))
                        + " (Slot 0 danach " + zahl (e.test.auslenkungen.empty() ? 0.0 : e.test.auslenkungen[0][0], 4)
                        + " dB, Referenz " + zahl (e.ref.auslenkungen.empty() ? 0.0 : e.ref.auslenkungen[0][0], 4)
                        + " dB); betragsgroesste Meldung von Slot 7 " + zahl (slot7, 6) + " dB");
            }

            // (c) alle acht Slots zugleich topologisch geaendert (Bell -> High-Shelf).
            {
                auto alleNeu = acht;
                for (int slot = 0; slot < param::kSlots; ++slot) alleNeu = mitTyp (alleNeu, slot, Filtertyp::highShelf);
                const auto e = fahrePaar (fs, mb, acht, alleNeu, Pfad::committed, achtToene, vorgeschichte, bs,
                                          stille, 4096, bs);
                double alle = 0.0;
                for (int slot = 0; slot < param::kSlots; ++slot) if (std::abs (slotMax (e.test, slot)) > std::abs (alle)) alle = slotMax (e.test, slot);
                const double spitze = spitzeAb (e.test.aus, 2u * (size_t) kFadeSamples);
                pruefe (e.auslenkungVorher[7] == -12.0 && alle == 0.0 && spitze == 0.0 && e.uebernahmenTest == 1,
                        "311/M-49 slotzahl_null_und_acht: (c) alle acht Slots zugleich geaendert - keiner uebertragen",
                        "betragsgroesste Auslenkung aller Slots ab dem ersten Block " + zahl (alle, 6) + " dB, Spitze bei Stille ab Sample "
                        + std::to_string (kFadeSamples) + ": " + zahl (spitze, 9));
            }
        }

        // M-50 (a), Matrixpruefung 1 H-8: nur Output-Trim -3 dB an einem
        // eingeschwungenen dynamischen Slot - Rampenuebergang mit
        // Zustandsuebernahme wie heute (E-19); ab dem Umschaltblock Stille, die
        // Auslenkung bleibt im Hold bei -12 dB.
        {
            auto dyn = machSatz (true);
            belege (dyn, 0, Filtertyp::bell, 1000.0, 2.0, 0.0);
            machDynamisch (dyn, 0, -12.0, -40.0, 0.1, 500.0, 5000.0);
            auto leiser = dyn;
            setzeGlobal (leiser, "v1.global.output_trim_db", -3.0);
            const auto quad = [w1k] (long long n, float& l, float& r)
            { l = (float) (0.5 * std::sin (w1k * (double) n)); r = (float) (0.5 * std::cos (w1k * (double) n)); };
            const auto e = fahrePaar (fs, mb, dyn, leiser, Pfad::committed, quad, 96000, bs, stille, 4096, bs);
            bool bleibt = ! e.test.auslenkungen.empty();
            for (const auto& a : e.test.auslenkungen) if (a[0] != -12.0) bleibt = false;
            const bool rampe = ! e.test.kreuzQuelle.empty() && e.test.kreuzQuelle[0] == -1;
            pruefe (e.auslenkungVorher[0] == -12.0 && bleibt && rampe && e.uebernahmenTest == 1,
                    "311/M-50 output_trim_haelt_die_historie (Teilfall (a), E-19, REGEL B-4)",
                    "vorher " + zahl (e.auslenkungVorher[0], 4) + " dB; nach Output-Trim -3 dB in jedem von "
                    + std::to_string (e.test.auslenkungen.size()) + " Bloecken Stille -12 dB: " + (bleibt ? "ja" : "nein")
                    + "; Rampe ohne Crossfade-Quelle: " + (rampe ? "ja" : "nein"));
        }

        // M-51, neuer Fall: Crossfade MIT Uebertragung haelt E-31. DC
        // x = (double) 0,3f, Blockgroesse 64; Slot 0 Low-Shelf 8 kHz Q 0,707
        // +9 dB eingeschwungen (Ausgang x0 = x * 10^(9/20), konstant), Slot 1
        // High-Shelf 8 kHz Q 0,707 +3 dB (bei DC die Einheit); Slot 1 wechselt
        // auf Low-Shelf +3 dB, Slot 0 bleibt. Der uebertragene Slot 0 liefert
        // konstant x0: genau ein Band laeuft kalt auf konstantem Eingang x0
        // gegen die konstante Quelle x0 - der Fall, fuer den `kaltSchranke`
        // hergeleitet ist. Toleranz nach R-311-10.
        {
            auto k = neuerKern (fs, bs);
            auto sa = machSatz (true);
            belege (sa, 0, Filtertyp::lowShelf,  8000.0, 0.707, 9.0);
            belege (sa, 1, Filtertyp::highShelf, 8000.0, 0.707, 3.0);
            k->uebernehmeZustand (sa);
            const auto vor = fahreDc (*k, 0.3, 2048, bs);
            k->pflege();
            k->uebernehmeZustand (mitTyp (sa, 1, Filtertyp::lowShelf));
            const auto lauf = fahreDc (*k, 0.3, 1024, bs);

            const double x0      = vor.back();
            const double schritt = x0 * (std::pow (10.0, 3.0 / 20.0) - 1.0) / (double) kFadeSamples;
            double betrag = 0.0;
            for (double v : vor)  betrag = std::max (betrag, std::abs (v));
            for (double v : lauf) betrag = std::max (betrag, std::abs (v));
            const double toleranz   = rundungR31110 (betrag);
            const double amUmschalt = std::abs (lauf[0] - x0);
            const double wache      = kaltSchranke (Filtertyp::lowShelf, fs, 8000.0, 0.707, 3.0, x0, x0);
            const double sprung     = groessterSprung (x0, lauf);
            const std::string rundung = "Toleranz R-311-10 " + zahl (toleranz, 9) + " (groesster Betrag des Laufs "
                                      + zahl (betrag, 7) + ")";
            pruefe (amUmschalt <= schritt + toleranz,
                    "311/M-51 crossfade_mit_uebertragung_haelt_e31: Umschaltsample (E-31, R-311-10)",
                    "Sprung am Umschaltsample " + zahl (amUmschalt, 9) + " gegen Fadeschritt x0(10^(3/20) - 1)/256 = "
                    + zahl (schritt, 9) + " + " + rundung + "; x0 = " + zahl (x0, 7));
            pruefe (sprung <= wache + toleranz,
                    "311/M-51 crossfade_mit_uebertragung_haelt_e31: Wache ueber den Lauf (E-31, kaltSchranke)",
                    "groesster Sprung " + zahl (sprung, 9) + " gegen kaltSchranke (Low-Shelf 8 kHz Q 0,707 +3 dB, x0, x0) = "
                    + zahl (wache, 9) + " + " + rundung + "; Ende " + zahl (lauf.back(), 6) + " gegen "
                    + zahl (x0 * std::pow (10.0, 3.0 / 20.0), 6));
        }

        // M-53: der Candidate-Pfad (E-16) - Kennungen je Pfad getrennt. Candidate
        // mit Slot 0 Bell 50 Hz Q 8 +12 dB am 50-Hz-Sinus eingeschwungen, Slot 5
        // belegt und aus; die Candidate-Publikation schaltet Slot 5 (0 dB) ein.
        // Gemessen am Tap post_candidate.
        {
            auto kandVor = machSatz (true);
            belege (kandVor, 0, Filtertyp::bell, 50.0, 8.0, 12.0);
            belege (kandVor, 5, Filtertyp::bell, 1000.0, 1.0, 0.0);
            kandVor.werte[(size_t) param::indexBandV1 (5, param::kEnabled)].b = false;
            auto kandNach = kandVor;
            kandNach.werte[(size_t) param::indexBandV1 (5, param::kEnabled)].b = true;
            auto committed = machSatz (true);
            setzeGlobal (committed, "v1.global.output_trim_db", -2.0);
            const auto e = fahrePaar (fs, mb, kandVor, kandNach, Pfad::candidate, nur50, vorgeschichte, bs,
                                      nur50, 100 * periode50, bs, &committed);
            double erste = 0.0;
            int schlimmste = -1;
            const double mass = periodenMass (e.test.kandidat, e.ref.kandidat, 2u * (size_t) periode50, 100, &erste, &schlimmste);
            const long long abw = ersteBitAbweichung (e.test.kandidat, e.ref.kandidat, 2u * (size_t) kFadeSamples);
            pruefe (mass <= 0.5 && abw == -1 && e.uebernahmenTest == 1,
                    "311/M-53 candidate_slot_behaelt_historie (E-16, R-311-1)",
                    "Mass am Tap post_candidate " + zahl (mass, 6) + " dB gegen 0,5 dB (erste Periode " + zahl (erste, 6)
                    + " dB, groesste Abweichung in Periode " + std::to_string (schlimmste)
                    + "); Tap ab Fade-Ende bytegleich zum Referenzkern: "
                    + (abw == -1 ? std::string ("ja") : "nein, erste Abweichung an Sample " + std::to_string (abw / 2)));
        }

        // M-94 (R-311-8): Uebertragung bei gleichzeitiger Wertaenderung. 48 kHz,
        // Blockgroesse 64, Quadraturton auf der Bandmitte (L sin, R cos, E-25);
        // gemessen am Quadraturbetrag m(n) = sqrt (L^2 + R^2) des Ausgangs, der
        // beim eingeschwungenen Ton konstant ist. Slot 5 belegt, 0 dB, aus. Eine
        // Publikation: Slot 5 schaltet ein und zugleich (a) statisch gain_db
        // +12 -> +6 dB, (b) dynamisch Range -12 -> -6 dB. Referenzkern: dieselbe
        // Wertaenderung ohne Fremdslotwechsel als Rampe (E-19). Die Zahlen sind
        // Schranken (R-311-8): (1) Sprung am Umschaltsample hoechstens
        // Fadeschritt + Toleranz R-311-10, (2) Spitze ueber 4800 Samples
        // hoechstens 0,5 dB ueber der groesseren eingeschwungenen Spitze, (3) ab
        // Fade-Ende plus T_E hoechstens 0,5 dB neben dem Referenzkern.
        {
            struct Unterfall
            {
                const char* name;
                double amplitude, mVor, mNach;
                int abSample;   ///< kFadeSamples + T_E
                bool dynamisch;
            };
            const Unterfall faelle[] = {
                { "(a) statisch, gain_db +12 -> +6 dB", 0.25, 0.25 * std::pow (10.0, 12.0 / 20.0), 0.25 * std::pow (10.0, 6.0 / 20.0),
                  kFadeSamples + 626, false },
                { "(b) dynamisch, Range -12 -> -6 dB", 0.5, 0.5 * std::pow (10.0, -12.0 / 20.0), 0.5 * std::pow (10.0, -6.0 / 20.0),
                  kFadeSamples + 322, true },
            };
            for (const auto& u : faelle)
            {
                auto vorher = machSatz (true);
                if (u.dynamisch)
                {
                    belege (vorher, 0, Filtertyp::bell, 1000.0, 2.0, 0.0);
                    machDynamisch (vorher, 0, -12.0, -40.0, 0.1, 0.0, 100.0);
                }
                else
                {
                    belege (vorher, 0, Filtertyp::bell, 1000.0, 2.0, 12.0);
                }
                belege (vorher, 5, Filtertyp::bell, 1000.0, 1.0, 0.0);
                vorher.werte[(size_t) param::indexBandV1 (5, param::kEnabled)].b = false;
                auto nurWert = vorher;
                if (u.dynamisch) nurWert.werte[(size_t) param::indexBandV1 (0, param::kDynamicRangeDb)].zahl = -6.0;
                else             nurWert.werte[(size_t) param::indexBandV1 (0, param::kGainDb)].zahl = 6.0;
                auto mitFremd = nurWert;
                mitFremd.werte[(size_t) param::indexBandV1 (5, param::kEnabled)].b = true;

                const double a = u.amplitude;
                const auto quad = [w1k, a] (long long n, float& l, float& r)
                { l = (float) (a * std::sin (w1k * (double) n)); r = (float) (a * std::cos (w1k * (double) n)); };

                // Pruefkern und Referenzkern: dieselbe Vorgeschichte, je ihre Publikation.
                auto test = neuerKern (fs, mb);
                auto ref  = neuerKern (fs, mb);
                W03Lauf vorT, lT, lR;
                long long nT = 0, nR = 0;
                test->uebernehmeZustand (vorher);
                ref->uebernehmeZustand (vorher);
                fahreW03 (*test, quad, nT, 96000, bs, &vorT);
                fahreW03 (*ref,  quad, nR, 96000, bs, nullptr);
                test->pflege();
                ref->pflege();
                test->uebernehmeZustand (mitFremd);
                ref->uebernehmeZustand (nurWert);
                fahreW03 (*test, quad, nT, 4800, bs, &lT);
                fahreW03 (*ref,  quad, nR, 4800, bs, &lR);

                const auto betragM = [] (const std::vector<double>& x, size_t i) { return std::sqrt (x[2 * i] * x[2 * i] + x[2 * i + 1] * x[2 * i + 1]); };
                const size_t nVor = vorT.aus.size() / 2;
                const double mVorher = betragM (vorT.aus, nVor - 1);
                double groessterBetrag = 0.0;
                for (double v : lT.aus) groessterBetrag = std::max (groessterBetrag, std::abs (v));
                const double toleranz = rundungR31110 (groessterBetrag);
                const double fadeschritt = std::abs (u.mNach - u.mVor) / (double) kFadeSamples;
                const double amUmschalt  = std::abs (betragM (lT.aus, 0) - mVorher);
                double spitze = 0.0;
                for (size_t i = 0; i < 4800; ++i) spitze = std::max (spitze, betragM (lT.aus, i));
                const double spitzenSchranke = std::max (u.mVor, u.mNach) * std::pow (10.0, 0.5 / 20.0);
                double referenzMass = 0.0;
                size_t referenzAn = 0;
                for (size_t i = (size_t) u.abSample; i < 4800; ++i)
                {
                    double db = std::abs (20.0 * std::log10 (betragM (lT.aus, i) / betragM (lR.aus, i)));
                    if (! std::isfinite (db)) db = std::numeric_limits<double>::infinity();
                    if (db > referenzMass) { referenzMass = db; referenzAn = i; }
                }
                const double mEnde = betragM (lT.aus, 4799);
                pruefe (amUmschalt <= fadeschritt + toleranz,
                        std::string ("311/M-94 uebertragung_mit_wertaenderung_haelt_e31_spitze_und_referenz: ") + u.name
                        + " - (1) Umschaltsample (E-31, R-311-10)",
                        "Sprung von m am Umschaltsample " + zahl (amUmschalt, 9) + " gegen Fadeschritt " + zahl (fadeschritt, 9)
                        + " + Toleranz " + zahl (toleranz, 9) + " (groesster Betrag " + zahl (groessterBetrag, 7) + "); m vorher "
                        + zahl (mVorher, 6) + " (Soll " + zahl (u.mVor, 4) + ")");
                pruefe (spitze <= spitzenSchranke,
                        std::string ("311/M-94 uebertragung_mit_wertaenderung_haelt_e31_spitze_und_referenz: ") + u.name
                        + " - (2) Spitze ueber 100 Perioden (R-311-8)",
                        "Spitze von m " + zahl (spitze, 6) + " gegen " + zahl (spitzenSchranke, 6) + " (0,5 dB ueber "
                        + zahl (std::max (u.mVor, u.mNach), 4) + ")");
                pruefe (referenzMass <= 0.5,
                        std::string ("311/M-94 uebertragung_mit_wertaenderung_haelt_e31_spitze_und_referenz: ") + u.name
                        + " - (3) ab Fade-Ende plus T_E neben dem Referenzkern (R-311-8)",
                        "ab Sample " + std::to_string (u.abSample) + " bis 4799 hoechstens " + zahl (referenzMass, 6)
                        + " dB neben der Rampe (an Sample " + std::to_string (referenzAn) + "); m am Fensterende "
                        + zahl (mEnde, 6) + " (Soll " + zahl (u.mNach, 4) + ")");
            }
        }

        // M-95 (R-311-9): der Mono-Bass-Zustand wandert mit. Mono-Bass 500 Hz in
        // beiden Programmen, kein aktives Band, Slot 5 belegt, 0 dB, aus; reines
        // Seitensignal L = 0,5 cos (2 pi 15 n / fs), R = -L, 2 s eingeschwungen;
        // der Umschaltblock beginnt auf einer Periodengrenze (3200 Samples =
        // 50 Bloecke). Nur v1.band.5.enabled wird wahr. Mass wie M-43 an der
        // Seitenkomponente (L - R)/2 ueber 100 Perioden.
        {
            const int periode15 = 3200;
            const double w15 = 2.0 * kPiRef * 15.0 / fs;
            auto vorher = machSatz (true);
            setzeGlobal (vorher, "v1.global.mono_bass_hz", 500.0);
            belege (vorher, 5, Filtertyp::bell, 1000.0, 1.0, 0.0);
            vorher.werte[(size_t) param::indexBandV1 (5, param::kEnabled)].b = false;
            auto nachher = vorher;
            nachher.werte[(size_t) param::indexBandV1 (5, param::kEnabled)].b = true;
            const auto seite = [w15] (long long n, float& l, float& r)
            {
                const float x = (float) (0.5 * std::cos (w15 * (double) n));
                l = x;
                r = -x;
            };
            const auto e = fahrePaar (fs, mb, vorher, nachher, Pfad::committed, seite, 30 * periode15, bs,
                                      seite, 100 * periode15, bs);
            const auto seiteVon = [] (const std::vector<double>& x)
            {
                std::vector<double> s (x.size() / 2);
                for (size_t i = 0; i < s.size(); ++i) s[i] = (x[2 * i] - x[2 * i + 1]) * 0.5;
                return s;
            };
            double erste = 0.0;
            int schlimmste = -1;
            const double mass = periodenMass (seiteVon (e.test.aus), seiteVon (e.ref.aus), (size_t) periode15, 100,
                                              &erste, &schlimmste);
            const long long abw = ersteBitAbweichung (e.test.aus, e.ref.aus, 2u * (size_t) kFadeSamples);
            pruefe (mass <= 0.5 && abw == -1 && e.uebernahmenTest == 1,
                    "311/M-95 mono_bass_zustand_wird_uebertragen (R-311-9, F-8)",
                    "Mass an der Seitenkomponente " + zahl (mass, 6) + " dB gegen 0,5 dB (Periode 3200 Samples, 100 Perioden; "
                    "erste Periode " + zahl (erste, 6) + " dB, groesste Abweichung in Periode " + std::to_string (schlimmste)
                    + "); ab Fade-Ende bytegleich zum Referenzkern: "
                    + (abw == -1 ? std::string ("ja") : "nein, erste Abweichung an Sample " + std::to_string (abw / 2)));
        }
    }

    //==========================================================================
    std::cout << std::endl;
    std::cout << geprueft << " geprueft, " << fehler << " Fehler" << std::endl;
    if (fehler == 0)
    {
        std::cout << "DSP-GOLDEN OK" << std::endl;
        return 0;
    }
    std::cout << "DSP-GOLDEN FEHLGESCHLAGEN" << std::endl;
    return 1;
}
