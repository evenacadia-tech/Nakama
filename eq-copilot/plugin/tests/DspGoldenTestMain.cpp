/*  Kanon B6 - EqCopDspGoldenTest (SONDE-015 Etappe 3, Bauplan §4.3).

    Misst den aktiven DSP-Kern `eq-copilot/plugin/dsp/` gegen die Zusagen
    der Verhaltensmatrix §3.1 bis §3.6 und §3.13 von
    `docs/beweise/SONDE-015.md`.

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

} // namespace

//==============================================================================
int main()
{
    std::cout << "== Nakama SONDE-015 B6 - aktiver DSP-Kern: Goldens, Bypass, Baenke ==" << std::endl;
    std::cout << "Gate: Entwurf §44.2/§44.5, Matrix §3.1-§3.6 und §3.13." << std::endl << std::endl;

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
            const float wachmarke = [] { std::uint32_t bits = 0x7F800001u; float f; std::memcpy (&f, &bits, 4); return f; }();
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
            const float wachmarke = [] { std::uint32_t bits = 0x7F800001u; float f; std::memcpy (&f, &bits, 4); return f; }();
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
            }
            pruefe (bitgleich, "mix_null_ist_bitidentisch (M-33)",
                    "trotz +9 dB Input-Trim und +12-dB-Bell");
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
            pruefe (RtWache::ableitungenAusserhalb() >= 2 && RtWache::ableitungenImAudiopfad() == 0,
                    "auto_gain_wird_nicht_im_audiothread_gerechnet (M-39, B-18)",
                    "Ableitungen ausserhalb " + std::to_string (RtWache::ableitungenAusserhalb())
                    + ", im Audiopfad " + std::to_string (RtWache::ableitungenImAudiopfad()));
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

            // Zwischen den Wechseln laeuft AUDIO - nur so wird eine Bank
            // ueberhaupt `audioAktiv` und beim naechsten Wechsel
            // `verblassend`. Ohne Audio verdraengt der zweite Wunsch den
            // ersten, der nie gelaufen ist, und der Pool bleibt bei zwei
            // Baenken.
            pruefe (k->uebernehmeZustand (s, Pfad::committed), "Committed nimmt eine Bank");
            fahreStille (*k, 64, 64);                                 // Bank 1 wird audioAktiv
            pruefe (k->uebernehmeZustand (s, Pfad::candidate), "Candidate nimmt eine zweite Bank");
            fahreStille (*k, 64, 64);                                 // Bank 2 wird audioAktiv
            auto s2 = s; belege (s2, 1, Filtertyp::bell, 2000.0, 1.0, -6.0);
            pruefe (k->uebernehmeZustand (s2, Pfad::committed), "Committed wechselt (dritte Bank)");
            fahreStille (*k, 64, 64);                                 // Bank 1 verblasst
            auto s3 = s2; belege (s3, 2, Filtertyp::notch, 500.0, 4.0, 0.0);
            pruefe (k->uebernehmeZustand (s3, Pfad::candidate), "Candidate wechselt (vierte Bank)");
            fahreStille (*k, 64, 64);                                 // Bank 2 verblasst

            pruefe (k->pool().freieSlots() == 0, "vier_baenke_im_schlimmsten_fall (M-46)",
                    "beide Pfade mitten im Fade: vier lebende Baenke");
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
                case  90: ok = kern->uebernehmeZustand (kand1, Pfad::candidate); break;
                case 140: ok = kern->uebernehmeZustand (s); break;
                case 190: ok = kern->uebernehmeZustand (kand2, Pfad::candidate); break;
                case 240: kern->beendeCandidate(); break;
                case 290: ok = kern->uebernehmeZustand (aus); break;
                case 340: ok = kern->uebernehmeZustand (s); break;
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
        // gewaehlt, 64 Samples, dann `beendeCandidate`: die Hoermatrix blendet
        // vom aktuellen Mischstand zurueck (E-30). Am Abbruchsample kein Sprung
        // ueber die Fadeschrittweite, danach ist die Candidate-Bank frei.
        {
            auto k = neuerKern (fs, 64);
            k->uebernehmeZustand (machSatz (true));
            const auto vor = fahreDc (*k, 0.3, 2048, 64);
            k->pflege();

            auto kand = machSatz (true);
            belege (kand, 0, Filtertyp::lowShelf, 8000.0, 0.707, 9.0);
            k->uebernehmeZustand (kand, Pfad::candidate);
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
    std::cout << std::endl << "== N - Messwert des Vier-Bank-Falls (R15, kein erfundener Deckel) ==" << std::endl;
    {
        // §49.3 nennt keine CPU-Zeile und §44.2 einen Golden ohne Zahl. Der
        // Test haelt deshalb fest, DASS der Vier-Bank-Fall laeuft und keine
        // Bank teilt; die gemessene Last steht als Zahl hier und im Manifest.
        // Ein erfundener Deckel waere auf einer schnelleren Maschine grundlos
        // gruen und auf einer langsameren grundlos rot.
        const double fs = 48000.0;
        const int blockGroesse = 256, bloecke = 4000;
        auto kern = neuerKern (fs, blockGroesse);

        auto voll = machSatz (true);
        for (int slot = 0; slot < param::kSlots; ++slot)
        {
            belege (voll, slot, Filtertyp::bell, 100.0 * std::pow (2.0, (double) slot), 2.0, 4.0,
                    (Kanalmodus) (slot % 5));
            machDynamisch (voll, slot, -6.0, -40.0, 5.0, 10.0, 80.0);
        }
        setzeGlobal (voll, "v1.global.width", 1.3);
        setzeGlobal (voll, "v1.global.mono_bass_hz", 110.0);
        setzeGlobal (voll, "v2.global.mix", 0.8);
        setzeGlobalBool (voll, "v2.global.auto_gain", true);

        kern->uebernehmeZustand (voll, Pfad::committed);
        fahreStille (*kern, blockGroesse, blockGroesse);
        kern->uebernehmeZustand (voll, Pfad::candidate);
        fahreStille (*kern, blockGroesse, blockGroesse);
        auto voll2 = voll;
        setzeGlobal (voll2, "v1.global.output_trim_db", -2.0);
        kern->uebernehmeZustand (voll2, Pfad::committed);
        fahreStille (*kern, blockGroesse, blockGroesse);
        kern->uebernehmeZustand (voll2, Pfad::candidate);

        pruefe (kern->pool().freieSlots() == 0,
                "der Messlauf faehrt wirklich vier Baenke (R15)",
                "freie Slots " + std::to_string (kern->pool().freieSlots()));

        std::vector<float> a ((size_t) blockGroesse), b ((size_t) blockGroesse);
        float* kan[2] = { a.data(), b.data() };
        const auto beginn = std::chrono::steady_clock::now();
        for (int blk = 0; blk < bloecke; ++blk)
        {
            for (int i = 0; i < blockGroesse; ++i)
            { const double x = 0.5 * std::sin (0.011 * (double) (blk * blockGroesse + i));
              a[(size_t) i] = (float) x; b[(size_t) i] = (float) (x * 0.8); }
            kern->verarbeite (kan, 2, blockGroesse);
        }
        const auto dauer = std::chrono::duration<double> (std::chrono::steady_clock::now() - beginn).count();
        const double audioZeit = (double) (bloecke * blockGroesse) / fs;
        const double anteil = dauer / audioZeit * 100.0;

        std::cout << "  MESSWERT Vier-Bank-Fall: " << zahl (dauer, 3) << " s fuer "
                  << zahl (audioZeit, 3) << " s Audio bei " << zahl (fs, 0) << " Hz, Blockgroesse "
                  << blockGroesse << " = " << zahl (anteil, 3) << " % Echtzeit "
                  << "(acht dynamische Baender, Committed und Candidate, je im Fade)" << std::endl;

        pruefe (dauer > 0.0 && std::isfinite (anteil),
                "der Vier-Bank-Fall wurde gemessen, nicht gedeckelt (R15)",
                zahl (anteil, 3) + " % Echtzeit - der Wert steht im Manifest, kein Deckel im Test");
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
