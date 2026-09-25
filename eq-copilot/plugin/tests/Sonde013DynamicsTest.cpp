/*  EqCopSonde013DynamicsTest — Loudnessfenster, Headroom und Dynamik
    (SONDE-013 M-01, M-03, M-04, M-07, M-09, M-75, M-77).

    §39.1 zaehlt die normgerechte Basis abschliessend auf: Momentary (400 ms),
    Short-term (3 s), Integrated, BS.1770-konformer True Peak, LRA erst nach
    rund 60 s geeignetem Material, PLR und PSR, Crest in MEHREREN Fenstern.
    Vor diesem Ticket trug der Frame davon drei Dinge, und eines davon war
    die schwaechere Zahl unter dem richtigen Namen.

    ── WAS DIESES BEIN MISST, UND WAS NICHT ─────────────────────────────────

    Es misst NICHT, dass die Felder vorhanden sind. Ein Bein, das nur
    Anwesenheit prueft, ist gruen, sobald irgendein Erzeuger irgendetwas
    hineinschreibt. Jeder Fall hier stellt statt dessen zwei Signale
    gegenueber, die sich in EINER Eigenschaft unterscheiden, und misst, dass
    genau das betroffene Fenster mitgeht und die anderen nicht:

    - Momentary gegen Short-term: ein Pegelsprung bewegt das 400-ms-Fenster
      sofort und das 3-s-Fenster erst spaeter. Waeren beide dieselbe Zahl mit
      zwei Namen, faellt dieser Fall.
    - Crest im Rahmen gegen Crest ueber 3 s: dichte kleine Spitzen und eine
      einzelne grosse sehen im 100-ms-Rahmen gleich aus und ueber drei
      Sekunden verschieden.
    - Headroom als Verteilung gegen Headroom als Spitze: ein Signal mit einer
      einzigen lauten Stelle hebt P95 und laesst P50 stehen.
    - LRA: es entsteht ueberhaupt erst nach der 60-s-Schwelle, und der Weg
      dorthin zaehlt GEGATETE Werte, nicht Wanduhr.

    Die zweite Haelfte ist fail-closed. Kein Fenster liefert eine Zahl,
    solange es nicht steht; keine Zahl ueberlebt eine Transportgrenze; und
    ein Ueberlaufblock verwirft die ANALYSE, nie das Audio.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/FeatureEngine.h"
#include "Nak380Pruefsignale.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <utility>
#include <vector>

namespace rt = nakama::echtzeit;
using nakama::analyse::FeatureEngine;
using nakama::analyse::FeatureFrame;

// NAK-283 M-55 (b), Speicherseite: gezaehlt wird jede Allokation dieses
// Prozesses, solange `zaehleAllokationen` steht - dieselbe Bauform wie in
// `LoudnessGoldenTestMain.cpp`. Der Test misst damit, was `vorbereiten()`
// anlegt, und dass der Lauf danach NICHTS mehr anlegt.
namespace
{
    bool          zaehleAllokationen = false;
    std::uint64_t allokationen       = 0;
    std::uint64_t allokierteBytes    = 0;
}

void* operator new (std::size_t groesse)
{
    if (zaehleAllokationen) { ++allokationen; allokierteBytes += groesse; }
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAllokationen) { ++allokationen; allokierteBytes += groesse; }
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

namespace
{
int bestanden = 0;
int fehler = 0;

void pruefe (bool ok, const juce::String& was, const juce::String& zusatz = {})
{
    std::cout << (ok ? "[ok]   " : "[ROT]  ") << was.toRawUTF8();
    if (zusatz.isNotEmpty())
        std::cout << "  [" << zusatz.toRawUTF8() << "]";
    std::cout << std::endl;
    if (ok) ++bestanden; else ++fehler;
}

void abschnitt (const char* text)
{
    std::cout << "\n== " << text << " ==" << std::endl;
}

constexpr double kZweiPi = 6.283185307179586476925286766559;

/** Speist Bloecke und fuehrt Strom und Projektzeit selbst.  Wortgleich mit
    dem Speiser in `Sonde013EventWireTest.cpp` — ein Test, der die
    Buchfuehrung dem Pruefling ueberliesse, pruefte gegen einen selbstgebauten
    Zeitfehler. */
struct Speiser
{
    FeatureEngine& engine;
    double sr { 48000.0 };
    int    frames { 512 };

    std::uint64_t strom { 0 };
    std::int64_t  projekt { 0 };
    std::vector<float> audio;

    explicit Speiser (FeatureEngine& e) : engine (e) {}

    rt::StampedBlock bauen() const
    {
        rt::StampedBlock b;
        b.stromVon = strom;
        b.sampleCount = (std::uint32_t) frames;
        b.segment = 0;
        b.startFolge = 0;
        b.kanaele = 2;
        b.tapMaske = 1;
        b.projectSampleStart = projekt;
        b.sampleRate = sr;
        b.flags = rt::kFlagKontextAnwesend | rt::kFlagSpieltGueltig
                | rt::kFlagSampleRateGueltig | rt::kFlagSpielt | rt::kFlagZeitGueltig;
        return b;
    }

    bool sende (const std::function<float (std::uint64_t)>& f)
    {
        const auto b = bauen();
        audio.resize ((std::size_t) b.sampleCount * 2u);
        for (std::uint32_t i = 0; i < b.sampleCount; ++i)
        {
            const float v = f (strom + i);
            audio[(std::size_t) i * 2u]      = v;
            audio[(std::size_t) i * 2u + 1u] = v;
        }
        const bool r = engine.nimmBlock (b, audio.data());
        strom   += b.sampleCount;
        projekt += (std::int64_t) b.sampleCount;
        return r;
    }

    /** Faehrt `bloecke` Bloecke und gibt den zuletzt entstandenen Frame
        zurueck; `gab` sagt, ob ueberhaupt einer faellig wurde. */
    FeatureFrame fahre (const std::function<float (std::uint64_t)>& f,
                        int bloecke, bool* gab = nullptr)
    {
        bool irgendeiner = false;
        FeatureFrame letzter {};
        for (int i = 0; i < bloecke; ++i)
            if (sende (f))
            {
                irgendeiner = true;
                letzter = engine.frame();
            }
        if (gab != nullptr) *gab = irgendeiner;
        return letzter;
    }

    /** Speist exakt `samples` weitere Samples; alle vollen Blöcke tragen die
        vorgegebene Größe, nur der letzte darf als ehrlicher Teilblock kürzer
        sein. Damit sind Matrixdauern keine aufgerundeten Laufwerte. */
    FeatureFrame fahreGenau (const std::function<float (std::uint64_t)>& f,
                             std::uint64_t samples, bool* gab = nullptr)
    {
        const int volleGroesse = frames;
        const auto ziel = strom + samples;
        bool irgendeiner = false;
        FeatureFrame letzter {};
        while (strom < ziel)
        {
            frames = (int) std::min<std::uint64_t> ((std::uint64_t) volleGroesse,
                                                    ziel - strom);
            if (sende (f))
            {
                irgendeiner = true;
                letzter = engine.frame();
            }
        }
        frames = volleGroesse;
        if (gab != nullptr) *gab = irgendeiner;
        return letzter;
    }
};

std::function<float (std::uint64_t)> sinus (double amplitude, double hz, double sr)
{
    return [amplitude, hz, sr] (std::uint64_t n)
    { return (float) (amplitude * std::sin (kZweiPi * hz * (double) n / sr)); };
}

/** Wie viele Bloecke a `frames` Samples eine Dauer in Sekunden ergibt. */
int bloeckeFuer (double sekunden, double sr, int frames)
{ return (int) std::ceil (sekunden * sr / (double) frames); }
} // namespace

#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
namespace nakama::analyse
{
/** NAK-283 M-59 und M-76: der einzige Testzugang dieser Etappe.

    Die zwei NaN-Zaehler sind `uint32` und saettigen bei `0xFFFFFFFF`. Der
    Rahmenzaehler faellt mit jedem Rahmen (hoechstens rund 5 000 Samples), der
    Evidenzzaehler mit jedem Evidenzfenster (hoechstens 1 s) - ueber die
    oeffentliche Schnittstelle erreicht keiner von beiden je seinen Anschlag.
    Ohne diesen Zugang waere die Zusage "saettigt, wrappt nicht, und die
    Zellenmarke haengt nicht am Zaehlerstand" nicht messbar. Die Klasse
    erklaert ihn in `FeatureEngine.h` zum Freund; das Produkt ruft ihn nie. */
struct FeatureEngineTestzugang
{
    static std::uint64_t lraGezaehlteWerte (const FeatureEngine& e) noexcept
    {
        return e.lraGezaehlt;
    }

    static bool lraLesen (const FeatureEngine& e, double& heraus) noexcept
    {
        return e.lraLu (heraus);
    }

    static void nichtEndlichZaehlerSetzen (FeatureEngine& e, std::uint32_t rahmen,
                                           std::uint32_t evidenz) noexcept
    {
        e.rahmenNichtEndlich  = rahmen;
        e.evidenzNichtEndlich = evidenz;
    }

    /** M-76, nur lesend: die Klassenmitte der groessten belegten Klasse der
        Headroomverteilung einer Passage in dBTP - der hoechste Wert, den die
        Verteilung traegt. P10, P50 und P95 zeigen einen einzelnen Ausreisser
        nicht; diese Zahl zeigt ihn. NaN, wenn die Verteilung leer ist. */
    static double headroomGroessteKlasseDb (const FeatureEngine& e) noexcept
    {
        if (e.headroomRing.empty() || e.headroomRing[0].groesste < 0)
            return std::numeric_limits<double>::quiet_NaN();
        return kHeadroomKlasseUntenDb
             + ((double) e.headroomRing[0].groesste + 0.5) * kHeadroomKlassenBreiteDb;
    }

    static void liveLeeren (FeatureEngine& e) noexcept
    {
        for (auto& v : e.liveAkku) v = { 0.0, 0 };
        for (auto& v : e.liveBreiteAkku) v = { 0.0, 0.0 };
    }

    static void liveBandSetzen (FeatureEngine& e, int band, double dichte,
                                std::uint32_t n, double seite,
                                double gesamt) noexcept
    {
        e.liveAkku[(std::size_t) band] = { dichte * (double) n, n };
        e.liveBreiteAkku[(std::size_t) band] = {
            seite * (double) n, gesamt * (double) n
        };
    }

    static void liveFuellen (const FeatureEngine& e, LiveBaender& baender,
                             float* stereo, std::uint8_t* stereoBitmap) noexcept
    {
        e.fuelleLive (baender, stereo, stereoBitmap);
    }

    // ── NAK-380 Etappe 4 (T-380-11): der Detektor nach T-380-5 ────────────
    /** Filterbreite w_k in Bins (±kFlussFilterCent), wie das Produkt sie rechnet. */
    static int filterBreite (int k) noexcept { return FeatureEngine::flussFilterBreite (k); }

    /** Heap-Bytes von Vorframe und Filterpuffer (2*K*8 B). */
    static std::size_t detektorPufferBytes (const FeatureEngine& e) noexcept
    {
        return e.detektor.empty() ? 0u
            : (e.detektor[0].vorframe.size() + e.detektor[0].filter.size()) * sizeof (double);
    }

    /** NAK-380 M-43 (R-380-12 (i)): Plaetze im Ring der monotonen
        Warteschlange, wie `vorbereiten` ihn anlegt. */
    static std::size_t schlangePlaetze (const FeatureEngine& e) noexcept
    {
        return e.detektor.empty() ? 0u : e.detektor[0].schlange.size();
    }

    /** NAK-380 M-43: rechnet das Maximumfilter des Produkts ueber einen
        Vorframe, dessen Pegel ueber alle Bins streng faellt (Binleistung in
        dBFS: -20 - 0,01*k). Dort entfernt die Warteschlange von hinten nie
        etwas; sie haelt jedes Fenster voll, und das ist der Fall, fuer den
        die Ringgroesse reichen muss. Rueckgabe: Zahl der Detektor-Bins, deren
        Filterwert vom direkt gerechneten Maximum ueber |j - i| <= w_(von+i)
        abweicht. Ein zu kleiner Ring ueberschreibt den Kopf der Schlange
        (Index modulo Ringgroesse, kein Zugriff ausserhalb) und liefert dort
        ein kleineres Maximum. -1: die Vorbedingung fehlt (der Vorframe faellt
        nicht streng, etwa weil der Flussschritt ihn nicht geschrieben hat). */
    static int filterAbweichungenFallend (FeatureEngine& e) noexcept
    {
        auto& s = e.haupt;
        const double df = s.fs / (double) s.punkte;
        for (std::size_t k = 0; k < s.psd.size(); ++k)
            s.psd[k] = std::pow (10.0, (-20.0 - 0.01 * (double) k) / 10.0) / df;
        double sf = 0.0, zentrum = 0.0;
        e.binFlussSchritt (s, sf, zentrum);
        const auto& d = e.detektor[0];
        if (d.binAnzahl < 2 || (int) d.vorframe.size() < d.binAnzahl || (int) d.filter.size() < d.binAnzahl)
            return -1;
        for (int i = 1; i < d.binAnzahl; ++i)
            if (! (d.vorframe[(std::size_t) i] < d.vorframe[(std::size_t) (i - 1)]))
                return -1;
        int abweichend = 0;
        for (int i = 0; i < d.binAnzahl; ++i)
        {
            const int w = FeatureEngine::flussFilterBreite (d.binVon + i);
            double soll = d.vorframe[(std::size_t) std::max (0, i - w)];
            for (int j = std::max (0, i - w); j <= std::min (d.binAnzahl - 1, i + w); ++j)
                soll = std::max (soll, d.vorframe[(std::size_t) j]);
            if (d.filter[(std::size_t) i] != soll)
                ++abweichend;
        }
        return abweichend;
    }

    /** Zwei konstruierte Hauptstufen-Spektren, je Bin k (0..N/2) die
        Binleistung in dBFS; der erste wird Vorframe, der zweite liefert SF. */
    static double binFluss (FeatureEngine& e, const std::vector<double>& vorDb,
                            const std::vector<double>& jetztDb) noexcept
    {
        auto& s = e.haupt;
        const double df = s.fs / (double) s.punkte;
        double sf = 0.0, zentrum = 0.0;
        for (std::size_t k = 0; k < s.psd.size(); ++k)
            s.psd[k] = std::pow (10.0, vorDb[k] / 10.0) / df;
        e.binFlussSchritt (s, sf, zentrum);
        for (std::size_t k = 0; k < s.psd.size(); ++k)
            s.psd[k] = std::pow (10.0, jetztDb[k] / 10.0) / df;
        sf = -1.0;
        if (! e.binFlussSchritt (s, sf, zentrum))
            return -1.0;
        return sf;
    }

    /** Die Historie der aktiven Frames aus `werte` (genau kFlussHistorie). */
    static void historieSetzen (FeatureEngine& e, const std::vector<double>& werte) noexcept
    {
        auto& d = e.detektor[0];
        for (std::size_t i = 0; i < d.historie.size() && i < werte.size(); ++i)
            d.historie[i] = werte[i];
        d.stand = 0;
        d.gefuellt = (int) std::min (werte.size(), d.historie.size());
    }

    static std::pair<double, double> medianUndMad (FeatureEngine& e) noexcept
    {
        double med = 0.0, mad = 0.0;
        e.medianUndMad (med, mad);
        return { med, mad };
    }

    /** Ein aktiver Hauptstufen-Frame mit Binfluss `sf`, Fensteranfang
        `stromStart`; Rueckgabe: neu abgelegte Ereignisse. */
    static int detektorSchritt (FeatureEngine& e, double sf, std::uint64_t stromStart) noexcept
    {
        const int vorher = e.ereignisAnzahlJetzt();
        e.haupt.fensterStromStart = stromStart;
        e.detektorSchritt (e.haupt, sf, 1000.0);
        return e.ereignisAnzahlJetzt() - vorher;
    }

    static float letzteStaerke (const FeatureEngine& e) noexcept
    {
        return e.ereignisAnzahlJetzt() > 0 ? e.ereignis (e.ereignisAnzahlJetzt() - 1).staerke : -1.0f;
    }
};
} // namespace nakama::analyse
#endif

namespace
{
bool nak380Waehlt (const char* nur, const char* id)
{
    return nur == nullptr || std::strcmp (nur, id) == 0;
}

__declspec(noinline) void nak380LraMesskern (const char* nur)
{
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
    using nakama::analyse::FeatureEngineTestzugang;
    namespace sig = nakama::test::nak380;
    constexpr double fs = 48000.0;
    constexpr int block = 512;

    if (nak380Waehlt (nur, "M-21"))
    {
        abschnitt ("380/M-21 lra_zehn_kurzzeitwerte_je_sekunde");
        auto engine = std::make_unique<FeatureEngine>();
        engine->vorbereiten (fs);
        engine->evidenzIntervallSetzen (1.0);
        Speiser speiser { *engine };
        speiser.frames = block;
        speiser.fahreGenau ([] (std::uint64_t n) { return sig::sinus1k (n, 0.1); },
                           (std::uint64_t) (75.0 * fs));
        const auto ist = FeatureEngineTestzugang::lraGezaehlteWerte (*engine);
        // 75 s / 0,1 s = 750 Zellen; Zelle 30 bis 750 einschliesslich:
        // 750 - 30 + 1 = 721 Werte (Tech 3342 §3.1: mindestens 10 Hz).
        pruefe (ist == 721u, "380/M-21 lra_zehn_kurzzeitwerte_je_sekunde",
                "ist " + juce::String ((juce::int64) ist) + ", soll 721");
    }

    if (nak380Waehlt (nur, "M-22"))
    {
        abschnitt ("380/M-22 lra_sechzig_sekunden_bei_zehn_hertz");
        auto engine = std::make_unique<FeatureEngine>();
        engine->vorbereiten (fs);
        engine->evidenzIntervallSetzen (1.0);
        Speiser speiser { *engine };
        speiser.frames = block;
        const auto ton = [] (std::uint64_t n) { return sig::sinus1k (n, 0.1); };
        speiser.fahreGenau (ton, (std::uint64_t) (62.5 * fs));
        double lra = 0.0;
        const auto n596 = FeatureEngineTestzugang::lraGezaehlteWerte (*engine);
        const bool bei596Ohne = ! FeatureEngineTestzugang::lraLesen (*engine, lra);
        speiser.fahreGenau (ton, (std::uint64_t) (0.3 * fs));
        const auto n599 = FeatureEngineTestzugang::lraGezaehlteWerte (*engine);
        const bool bei599Ohne = ! FeatureEngineTestzugang::lraLesen (*engine, lra);
        speiser.fahreGenau (ton, (std::uint64_t) (0.1 * fs));
        const auto n600 = FeatureEngineTestzugang::lraGezaehlteWerte (*engine);
        const bool bei600Mit = FeatureEngineTestzugang::lraLesen (*engine, lra);
        // 625/628/629 Zellen ergeben 596/599/600 Werte: n-30+1; die drei
        // exakten Dauern umfassen 3.000.000/3.014.400/3.019.200 Samples.
        pruefe (n596 == 596u && n599 == 599u && n600 == 600u
                    && speiser.strom == 3'019'200u
                    && bei596Ohne && bei599Ohne && bei600Mit,
                "380/M-22 lra_sechzig_sekunden_bei_zehn_hertz",
                "Werte " + juce::String ((juce::int64) n596) + "/"
                    + juce::String ((juce::int64) n599) + "/"
                    + juce::String ((juce::int64) n600) + "; 62,5 s ohne="
                    + juce::String (bei596Ohne ? "ja" : "nein")
                    + ", 62,8 s ohne=" + juce::String (bei599Ohne ? "ja" : "nein")
                    + ", 62,9 s mit=" + juce::String (bei600Mit ? "ja" : "nein"));
    }

    if (nak380Waehlt (nur, "M-23"))
    {
        abschnitt ("380/M-23 lra_ohne_aliasing");
        auto engine = std::make_unique<FeatureEngine>();
        engine->vorbereiten (fs);
        engine->evidenzIntervallSetzen (1.0);
        Speiser speiser { *engine };
        speiser.frames = block;
        speiser.fahreGenau ([] (std::uint64_t n)
                           { return sig::sinus1k (n, sig::l2Amplitude (n)); },
                           (std::uint64_t) (70.0 * fs));
        double ist = 0.0;
        const bool gesetzt = FeatureEngineTestzugang::lraLesen (*engine, ist);
        // R-380-11 (a): Referenz = lraReferenz (Tech 3342 §5, T-380-3) ueber
        // die analytische 100-ms-Zellenenergie desselben L2-Signals,
        // e_z = 0,5·a(z)², 700 Zellen (sig::l2Zellenergie). Im 3-s-Fenster
        // nimmt der Einschaltanteil nur f = k/30 mit k = 10 … 20 an, die
        // Energie ist ∝ f + (1 − f)·0,01. §5 trifft bei 671 Werten P10 bei
        // k = 11 (Index round(670·0,10 + 1) = 68) und P95 bei k = 20 (Index
        // round(670·0,95 + 1) = round(637,5) = 638, MATLAB-round und
        // llround runden die halbe Stelle weg von null); nachgerechnet
        // 10·log10(0,6700/0,3730) = 2,544 LU. ±0,15 LU deckt das
        // 0,1-LU-Histogrammraster des Produkts und den Einschwingrand des
        // K-Filters. Plausibilitaetsnotiz, keine Referenz: die geschlossene
        // Form ueber eine kontinuierlich gleichverteilte Quote f ∈ [1/3, 2/3]
        // ergaebe 2,435 LU.
        const double referenz = sig::lraReferenz (sig::l2Zellenergie (700));
        // R-380-11 (b): 1-Hz-Werte an ganzen Sekunden treffen nur f = 2/3
        // und 1/3, also Aliaswert = 10·log10((2·e_laut + e_leise)
        // / (e_laut + 2·e_leise)) = 10·log10(0,6700/0,3400) = 2,946 LU; der
        // 10-Hz-Wert liegt mindestens 0,3 LU darunter.
        const double aLaut  = sig::l2Amplitude (0u);
        const double aLeise = sig::l2Amplitude ((std::uint64_t) fs);
        const double eLaut  = 0.5 * aLaut * aLaut;
        const double eLeise = 0.5 * aLeise * aLeise;
        const double alias  = 10.0 * std::log10 ((2.0 * eLaut + eLeise)
                                                 / (eLaut + 2.0 * eLeise));
        const double abweichung = std::abs (ist - referenz);
        const double abstand    = alias - ist;
        const bool nahAnReferenz = gesetzt && abweichung <= 0.15;
        const bool unterAlias    = gesetzt && abstand >= 0.3;
        pruefe (nahAnReferenz && unterAlias, "380/M-23 lra_ohne_aliasing",
                "ist " + juce::String (ist, 3) + " LU | (a) Referenz "
                    + juce::String (referenz, 3) + " LU, Abweichung "
                    + juce::String (abweichung, 3) + " <= 0,15: "
                    + juce::String (nahAnReferenz ? "ja" : "nein")
                    + " | (b) Aliaswert " + juce::String (alias, 3)
                    + " LU, Abstand " + juce::String (abstand, 3) + " >= 0,3: "
                    + juce::String (unterAlias ? "ja" : "nein"));
    }

    if (nak380Waehlt (nur, "M-29"))
    {
        abschnitt ("380/M-29 lra_nan_sperrt_dreissig_werte");
        auto engine = std::make_unique<FeatureEngine>();
        engine->vorbereiten (fs);
        engine->evidenzIntervallSetzen (1.0);
        Speiser speiser { *engine };
        speiser.frames = block;
        speiser.fahreGenau ([] (std::uint64_t n)
        {
            if (n == (std::uint64_t) (40.0 * sig::kSamplerate))
                return std::numeric_limits<float>::quiet_NaN();
            return sig::sinus1k (n, 0.1);
        }, (std::uint64_t) (75.0 * fs));
        const auto ist = FeatureEngineTestzugang::lraGezaehlteWerte (*engine);
        // Ohne Ersatz 721 Werte (M-21); genau die 30 ueberlappenden 3-s-
        // Fenster enthalten die Zelle des NaN-Samples: 721 - 30 = 691.
        pruefe (ist == 691u, "380/M-29 lra_nan_sperrt_dreissig_werte",
                "ist " + juce::String ((juce::int64) ist) + ", soll 691");
    }

    if (nak380Waehlt (nur, "M-28"))
    {
        abschnitt ("380/M-28 lra_below_sixty_seconds_is_not_a_number");
        auto engine = std::make_unique<FeatureEngine>();
        engine->vorbereiten (fs);
        engine->evidenzIntervallSetzen (1.0);
        Speiser speiser { *engine };
        speiser.frames = block;
        const auto wandernd = [] (std::uint64_t n)
        {
            const double t = (double) n / fs;
            const double a = 0.30 * std::pow (10.0,
                (-10.0 + 10.0 * std::sin (kZweiPi * t / 25.0)) / 20.0);
            return sig::sinus1k (n, a);
        };
        speiser.fahreGenau (wandernd, (std::uint64_t) (30.0 * fs));
        double bei30 = 0.0;
        const bool vorzeitig = FeatureEngineTestzugang::lraLesen (*engine, bei30);
        speiser.fahreGenau (wandernd, (std::uint64_t) (45.0 * fs));
        double ist = 0.0;
        const bool gesetzt = FeatureEngineTestzugang::lraLesen (*engine, ist);
        std::vector<double> zellen;
        zellen.reserve (750u);
        for (int z = 0; z < 750; ++z)
        {
            double energie = 0.0;
            for (int j = 0; j < 20; ++j)
            {
                const double t = 0.1 * ((double) z + ((double) j + 0.5) / 20.0);
                const double a = 0.30 * std::pow (10.0,
                    (-10.0 + 10.0 * std::sin (kZweiPi * t / 25.0)) / 20.0);
                energie += 0.5 * a * a;
            }
            zellen.push_back (energie / 20.0);
        }
        const double referenz = sig::lraReferenz (zellen);
        pruefe (! vorzeitig, "380/M-28(a) nach 30 s kein LRA-Wert");
        // ±0,2 LU: 0,1-LU-Histogrammraster plus Zell-/float32-Rand; Referenz
        // ist Tech 3342 §5 über die analytische 10-Hz-Hüllkurvenfolge.
        pruefe (gesetzt && std::abs (ist - referenz) <= 0.2,
                "380/M-28(b) LRA nach 75 s gegen §5-Referenz +/-0,2 LU",
                "ist " + juce::String (ist, 3) + ", Referenz "
                    + juce::String (referenz, 3));
    }

    if (nak380Waehlt (nur, "M-31"))
    {
        abschnitt ("380/M-31 material_ohne_dynamik_bleibt_nahe_null");
        auto engine = std::make_unique<FeatureEngine>();
        engine->vorbereiten (fs);
        engine->evidenzIntervallSetzen (1.0);
        Speiser speiser { *engine };
        speiser.frames = block;
        speiser.fahreGenau ([] (std::uint64_t n) { return sig::sinus1k (n, 0.2); },
                           (std::uint64_t) (75.0 * fs));
        double ist = 0.0;
        const bool gesetzt = FeatureEngineTestzugang::lraLesen (*engine, ist);
        pruefe (gesetzt && ist < 1.0,
                "380/M-31 material_ohne_dynamik_bleibt_nahe_null",
                "ist " + juce::String (ist, 3) + " LU, Grenze <1,0 LU");
    }
#else
    juce::ignoreUnused (nur);
    pruefe (false, "NAK-380 LRA-Testzugang", "NAKAMA_FEATUREENGINE_TESTZUGANG fehlt");
#endif
}

//==============================================================================
// NAK-380 Etappe 4 (M-42 bis M-49): der Detektor nach T-380-5 ueber den
// Testzugang. Jede Zahl ist aus Formel oder Konstruktion hergeleitet (R-380-8);
// Schwellen werden beiderseits geprueft (Lehre Z1 aus §31).
//
//   Binleistung p_k = psd_k * fs/N;  L = 10*log10(p_k + P0), P0 = 10^(-100/10)
//   w_k = max(1, ceil(k*(2^(125/1200) - 1)))  (±125 Cent, SuperFlux Gl. 5,
//                                               an die Hopzeit gebunden und kalibriert, R-380-12 (i))
//   SF = Summe_k max(0, L(n,k) - max_{|j-k|<=w_k} L(n-1,j))
//   T_eff = max(med + kappa*MAD, (1 + rho)*med, T_min), T_min = t_min*K
//
// Bei 48 kHz: Delta f = 48000/4096 = 11,71875 Hz; Detektor-Bins k mit
// 30,36 Hz <= k*Delta f < 17 959,39 Hz, also k = ceil(2,591) = 3 bis 1532,
// K = 1530; Hop 2048 Samples = 42,667 ms.

namespace nak380e4
{
using nakama::analyse::FeatureEngineTestzugang;
constexpr double kFs = 48000.0;
constexpr std::uint64_t kHop = 2048;

double pegelMitP0 (double db)   // 10*log10(10^(db/10) + P0), P0 aus der Konstante
{
    return 10.0 * std::log10 (std::pow (10.0, db / 10.0)
                              + std::pow (10.0, nakama::analyse::kFlussP0Db / 10.0));
}

std::vector<double> spektrum (double grundDb, int tonBin, double tonDb)
{
    std::vector<double> db ((std::size_t) (FeatureEngine::kHauptPunkte / 2 + 1), grundDb);
    if (tonBin >= 0)
        db[(std::size_t) tonBin] = tonDb;
    return db;
}

std::vector<double> historie (double a, int anzahlA, double b)
{
    std::vector<double> h ((std::size_t) nakama::analyse::kFlussHistorie, b);
    for (int i = 0; i < anzahlA && i < (int) h.size(); ++i)
        h[(std::size_t) i] = a;
    return h;
}

/** Frische Engine bei 48 kHz, Historie gesetzt; faehrt die SF-Folge ab
    `stromStart` 0 im Hopraster `hopSamples` und zaehlt die Ereignisse. */
int folge (const std::vector<double>& hist, const std::vector<double>& sf,
           std::uint64_t hopSamples = kHop, double fs = kFs)
{
    auto e = std::make_unique<FeatureEngine>();
    e->vorbereiten (fs);
    FeatureEngineTestzugang::historieSetzen (*e, hist);
    int n = 0;
    for (std::size_t i = 0; i < sf.size(); ++i)
        n += FeatureEngineTestzugang::detektorSchritt (*e, sf[i], (std::uint64_t) i * hopSamples);
    return n;
}
} // namespace nak380e4

__declspec(noinline) void nak380Detektoreinheit (const char* nur)
{
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
    using namespace nak380e4;
    const double tMin48 = nakama::analyse::kFlussTminDbJeBin * 1530.0;

    if (nak380Waehlt (nur, "M-42"))
    {
        abschnitt ("380/M-42 superflux_maximumfilter");
        // 125 Cent (R-380-12 (i): Startwert 100, kalibriert, Manifest §37),
        // Faktor 2^(125/1200) - 1 = 0,074873: w_10 = ceil(0,749) = 1, w_101 =
        // ceil(7,562) = 8, w_1000 = ceil(74,87) = 75, w_1668 = ceil(124,89) = 125.
        const int w10 = FeatureEngineTestzugang::filterBreite (10);
        const int w101 = FeatureEngineTestzugang::filterBreite (101);
        const int w1000 = FeatureEngineTestzugang::filterBreite (1000);
        const int w1668 = FeatureEngineTestzugang::filterBreite (1668);
        pruefe (w10 == 1 && w101 == 8 && w1000 == 75 && w1668 == 125,
                "380/M-42 superflux_maximumfilter: Filterbreite w_10 = 1, w_101 = 8, w_1000 = 75, w_1668 = 125",
                juce::String (w10) + ", " + juce::String (w101) + ", " + juce::String (w1000) + ", "
                    + juce::String (w1668));
        // Vorframe: Teilton in Bin 101 auf -20 dBFS, alle uebrigen Bins -120.
        // d = 1..9: Zielbin 101+d (102 bis 110) hat w = 8 (ceil(7,64) bis
        // ceil(7,94)) bzw. 9 (ceil(8,01) bis ceil(8,24)), sein Fenster enthaelt
        // Bin 101 -> Fluss 0. d = 10: w_111 = ceil(8,31) = 9, das Fenster von
        // Bin 111 ist 102..120 -> genau ein Bin steigt von L(-120) auf L(-20):
        // 79,9568 dB.
        const double sollD10 = pegelMitP0 (-20.0) - pegelMitP0 (-120.0);
        juce::String werte;
        bool nullBisNeun = true;
        double d10 = -1.0;
        for (int d = 1; d <= 10; ++d)
        {
            auto e = std::make_unique<FeatureEngine>();
            e->vorbereiten (kFs);
            const double sf = FeatureEngineTestzugang::binFluss (
                *e, spektrum (-120.0, 101, -20.0), spektrum (-120.0, 101 + d, -20.0));
            werte << (d > 1 ? ", " : "") << "d=" << d << ": " << juce::String (sf, 6);
            if (d <= 9) nullBisNeun = nullBisNeun && sf == 0.0;
            else d10 = sf;
        }
        pruefe (nullBisNeun,
                "380/M-42 superflux_maximumfilter: d = 1 bis 9 ergibt Fluss 0,0 dB (Bin 101 im Fenster)",
                werte);
        pruefe (std::abs (d10 - sollD10) <= 1.0e-9,
                "380/M-42 superflux_maximumfilter: d = 10 ergibt L(-20) - L(-120) = 79,96 dB in genau einem Bin",
                "ist " + juce::String (d10, 6) + ", Soll " + juce::String (sollD10, 6));
    }

    if (nak380Waehlt (nur, "M-43"))
    {
        abschnitt ("380/M-43 detektor_binbereich");
        // 44,1 kHz: Delta f = 10,7666 Hz, k = ceil(2,820) = 3 bis 1668 (1668*10,7666
        // = 17 958,7 < 17 959,39), K = 1666; 48 kHz: 3 bis 1532, K = 1530;
        // 96 kHz: Delta f = 23,4375 Hz, k = ceil(1,295) = 2 bis 766, K = 765.
        struct Soll { double fs; int von; int anzahl; };
        const Soll soll[] = { { 44100.0, 3, 1666 }, { 48000.0, 3, 1530 }, { 96000.0, 2, 765 } };
        for (const auto& s : soll)
        {
            auto e = std::make_unique<FeatureEngine>();
            e->vorbereiten (s.fs);
            const int von = e->detektorBinVon();
            const int anzahl = e->detektorBinAnzahl();
            const auto bytes = FeatureEngineTestzugang::detektorPufferBytes (*e);
            pruefe (von == s.von && anzahl == s.anzahl
                        && bytes == (std::size_t) (2 * s.anzahl * 8) && bytes <= 26656u,
                    "380/M-43 detektor_binbereich: " + juce::String (s.fs / 1000.0, 1) + " kHz, k = "
                        + juce::String (s.von) + " bis " + juce::String (s.von + s.anzahl - 1)
                        + ", K = " + juce::String (s.anzahl) + ", Vorframe und Filterpuffer 2*K*8 B <= 26 656 B",
                    "von " + juce::String (von) + ", K " + juce::String (anzahl) + ", "
                        + juce::String ((juce::int64) bytes) + " B");
        }
        // R-380-12 (i): der Ring der monotonen Warteschlange folgt der
        // Filterbreite. 2*w_max + 2 Plaetze mit w_max = ceil(k_max*(2^(125/1200)
        // - 1)) am obersten Detektor-Bin k_max = von + K - 1: 44,1 kHz k_max 1668,
        // ceil(124,89) = 125, 252 Plaetze; 48 kHz 1532, ceil(114,71) = 115, 232;
        // 96 kHz 766, ceil(57,35) = 58, 118. Dazu reicht er: ueber einen streng
        // fallenden Vorframe haelt die Schlange jedes Fenster voll, hoechstens
        // w_i + w_(i-1) + 2 Indizes (nachgerechnet 236 bei 44,1 kHz, 216 bei
        // 48 kHz, 110 bei 96 kHz), und das Filter muss in JEDEM Detektor-Bin
        // das direkt gerechnete Maximum treffen.
        struct SollRing { double fs; int kMax; int wMax; std::size_t plaetze; };
        const SollRing ringSoll[] = { { 44100.0, 1668, 125, 252u }, { 48000.0, 1532, 115, 232u },
                                      { 96000.0, 766, 58, 118u } };
        for (const auto& r : ringSoll)
        {
            auto e = std::make_unique<FeatureEngine>();
            e->vorbereiten (r.fs);
            const int kMax = e->detektorBinVon() + e->detektorBinAnzahl() - 1;
            const auto plaetze = FeatureEngineTestzugang::schlangePlaetze (*e);
            pruefe (kMax == r.kMax && FeatureEngineTestzugang::filterBreite (kMax) == r.wMax
                        && plaetze == r.plaetze,
                    "380/M-43 detektor_binbereich: " + juce::String (r.fs / 1000.0, 1)
                        + " kHz, Ring der Warteschlange 2*w_max + 2 = " + juce::String ((int) r.plaetze)
                        + " Plaetze (w_max = " + juce::String (r.wMax) + " am Bin " + juce::String (r.kMax) + ")",
                    "k_max " + juce::String (kMax) + ", w_max "
                        + juce::String (FeatureEngineTestzugang::filterBreite (kMax)) + ", "
                        + juce::String ((juce::int64) plaetze) + " Plaetze");
            const int abweichend = FeatureEngineTestzugang::filterAbweichungenFallend (*e);
            pruefe (abweichend == 0,
                    "380/M-43 detektor_binbereich: " + juce::String (r.fs / 1000.0, 1)
                        + " kHz, der Ring reicht - Maximumfilter ueber einen streng fallenden Vorframe "
                          "gleich dem direkten Maximum in jedem Detektor-Bin",
                    abweichend < 0 ? juce::String ("Vorbedingung fehlt: Vorframe faellt nicht streng")
                                   : juce::String (abweichend) + " von " + juce::String (e->detektorBinAnzahl())
                                         + " Bins abweichend");
        }
        // Heap in vorbereiten: danach alloziert der Lauf nichts mehr (auch
        // nicht der Flussschritt). 2 s W1, Block 512, Puffer vorher angelegt.
        auto e = std::make_unique<FeatureEngine>();
        e->vorbereiten (kFs);
        const auto w1 = nakama::test::nak380::weissMono (nakama::test::nak380::kW1Saat, 0.1, 96000u);
        std::vector<float> audio (1024u);
        rt::StampedBlock b;
        b.segment = 0; b.startFolge = 0; b.kanaele = 2; b.tapMaske = 1; b.sampleRate = kFs;
        b.flags = rt::kFlagKontextAnwesend | rt::kFlagSpieltGueltig
                | rt::kFlagSampleRateGueltig | rt::kFlagSpielt | rt::kFlagZeitGueltig;
        b.sampleCount = 512u;
        allokationen = 0;
        zaehleAllokationen = true;
        for (std::uint64_t strom = 0; strom + 512u <= w1.size(); strom += 512u)
        {
            for (std::uint32_t i = 0; i < 512u; ++i)
                audio[(std::size_t) i * 2u] = audio[(std::size_t) i * 2u + 1u] = w1[(std::size_t) (strom + i)];
            b.stromVon = strom;
            b.projectSampleStart = (std::int64_t) strom;
            (void) e->nimmBlock (b, audio.data());
        }
        zaehleAllokationen = false;
        pruefe (allokationen == 0u,
                "380/M-43 detektor_binbereich: nach vorbereiten() alloziert der Lauf nichts (Flussschritt eingeschlossen)",
                juce::String ((juce::int64) allokationen) + " Allokationen in 2 s");
    }

    if (nak380Waehlt (nur, "M-44"))
    {
        abschnitt ("380/M-44 echte_mad_schief");
        // 28 x 1,0 und 4 x 11,0: Median 1,0; Absolutabweichungen 28 x 0 und
        // 4 x 10 -> ihr Median 0,0. Die mittlere Abweichung waere 40/32 = 1,25.
        auto e = std::make_unique<FeatureEngine>();
        e->vorbereiten (kFs);
        FeatureEngineTestzugang::historieSetzen (*e, historie (11.0, 4, 1.0));
        const auto [med, mad] = FeatureEngineTestzugang::medianUndMad (*e);
        pruefe (med == 1.0 && mad == 0.0 && med + nakama::analyse::kFlussKappa * mad == 1.0,
                "380/M-44 echte_mad_schief: Median 1,0, MAD = Median der Absolutabweichungen = 0,0, "
                "med + 3*MAD = 1,0 (nicht 4,75)",
                "Median " + juce::String (med, 6) + ", MAD " + juce::String (mad, 6));
    }

    if (nak380Waehlt (nur, "M-45"))
    {
        abschnitt ("380/M-45 absolute_mindestschwelle");
        // Historie 32 x 0 (med = MAD = 0) -> T_eff = T_min = t_min * 1530;
        // mit dem kalibrierten t_min = 0,10 dB je Bin (R-380-12 (i), Manifest
        // §37) 153,0 dB: 152,9 kein Ereignis, 153,1 genau eines.
        const int unter = folge (historie (0.0, 0, 0.0), { tMin48 - 0.1 });
        const int ueber = folge (historie (0.0, 0, 0.0), { tMin48 + 0.1 });
        pruefe (unter == 0,
                "380/M-45 absolute_mindestschwelle: SF = T_min - 0,1 dB loest nicht aus",
                "T_min = " + juce::String (tMin48, 3) + " dB, Ereignisse " + juce::String (unter));
        pruefe (ueber == 1,
                "380/M-45 absolute_mindestschwelle: SF = T_min + 0,1 dB loest genau ein Ereignis aus",
                "T_min = " + juce::String (tMin48, 3) + " dB, Ereignisse " + juce::String (ueber));
    }

    if (nak380Waehlt (nur, "M-46"))
    {
        abschnitt ("380/M-46 rauschbodenbezug");
        // Historie 16 x 160 und 16 x 200: med = (160 + 200)/2 = 180, alle
        // |x - med| = 20 -> MAD = 20. T_eff = max(180 + 3*20 = 240,
        // (1 + rho)*180 = 360, T_min = 0,10*1530 = 153,0) = 360: 359 kein
        // Ereignis, 361 genau eines (Matrix §6.3; der kalibrierte T_min liegt
        // weiter unter 360).
        const auto h = historie (160.0, 16, 200.0);
        const double teff = std::max ({ 180.0 + nakama::analyse::kFlussKappa * 20.0,
                                        (1.0 + nakama::analyse::kFlussRho) * 180.0, tMin48 });
        const int unter = folge (h, { teff - 1.0 });
        const int ueber = folge (h, { teff + 1.0 });
        pruefe (teff == 360.0 && unter == 0,
                "380/M-46 rauschbodenbezug: T_eff = (1 + rho)*med = 360, SF = 359 loest nicht aus",
                "T_eff " + juce::String (teff, 3) + ", Ereignisse " + juce::String (unter));
        pruefe (ueber == 1,
                "380/M-46 rauschbodenbezug: SF = 361 loest genau ein Ereignis aus",
                "Ereignisse " + juce::String (ueber));
    }

    if (nak380Waehlt (nur, "M-47"))
    {
        abschnitt ("380/M-47 pegelbezug_p0");
        // (a) W (Saat 0x3800001) auf -90 dBFS bis 5 s, dann -70 dBFS bis 10 s:
        // Rahmenenergie sigma^2 = -90 bzw. -70 dB, beide unter dem Aktivgate
        // -60 dB -> kein Ereignis.
        auto x = nakama::test::nak380::weissMono (nakama::test::nak380::kM47Saat, 1.0, 480000u);
        for (std::size_t i = 0; i < x.size(); ++i)
            x[i] = (float) ((double) x[i] * (i < 240000u ? std::pow (10.0, -90.0 / 20.0)
                                                         : std::pow (10.0, -70.0 / 20.0)));
        auto e = std::make_unique<FeatureEngine>();
        e->vorbereiten (kFs);
        Speiser s { *e };
        int ereignisse = 0;
        s.fahreGenau ([&x] (std::uint64_t n) { return x[(std::size_t) n]; }, 480000u);
        ereignisse = e->ereignisAnzahlJetzt();
        pruefe (ereignisse == 0 && e->ereignisseVerworfen() == 0u,
                "380/M-47 pegelbezug_p0 (a): Sprung von -90 auf -70 dBFS unter dem Aktivgate loest nichts aus",
                "Ereignisse " + juce::String (ereignisse));
        // (b) Vorframe alle Bins -130 dBFS, Frame alle Bins -120 dBFS, aktiv:
        // SF = 1530 * 10*log10((1e-12 + 1e-10)/(1e-13 + 1e-10)) = 59,49 dB < T_min.
        auto eb = std::make_unique<FeatureEngine>();
        eb->vorbereiten (kFs);
        const double sf = FeatureEngineTestzugang::binFluss (*eb, spektrum (-130.0, -1, 0.0),
                                                              spektrum (-120.0, -1, 0.0));
        const double sollSf = 1530.0 * (pegelMitP0 (-120.0) - pegelMitP0 (-130.0));
        FeatureEngineTestzugang::historieSetzen (*eb, historie (0.0, 0, 0.0));
        const int n = FeatureEngineTestzugang::detektorSchritt (*eb, sf, 0u);
        pruefe (std::abs (sf - sollSf) <= 1.0e-6 && sf < tMin48 && n == 0,
                "380/M-47 pegelbezug_p0 (b): -130 -> -120 dBFS je Bin ergibt SF = 59,49 dB < T_min, kein Ereignis",
                "SF " + juce::String (sf, 4) + ", Soll " + juce::String (sollSf, 4) + ", T_min "
                    + juce::String (tMin48, 3) + ", Ereignisse " + juce::String (n));
    }

    if (nak380Waehlt (nur, "M-48"))
    {
        abschnitt ("380/M-48 spitzenwahl_lokales_maximum");
        // Historie 32 x 0: med = MAD = 0 -> T_eff = T_min = 153,0 dB (Matrix
        // §6.3 mit dem kalibrierten T_min). SF-Folge 0, 500, 450, 400: alle
        // drei ueber T_eff; 450 liegt
        // 42,67 ms nach 500 (Sperrzeit) und ist kein lokales Maximum, 400 liegt
        // 85,33 ms nach 500 (Sperrzeit frei) und ist kein lokales Maximum ->
        // genau ein Ereignis. Die Historie nimmt die Folge auf; der Median
        // bleibt 0, solange weniger als 16 Werte ungleich 0 sind.
        const int n = folge (historie (0.0, 0, 0.0), { 0.0, 500.0, 450.0, 400.0 });
        pruefe (n == 1,
                "380/M-48 spitzenwahl_lokales_maximum: 0, 500, 450, 400 ergibt genau ein Ereignis (am Frame mit 500)",
                "T_eff = T_min = " + juce::String (tMin48, 3) + ", Ereignisse " + juce::String (n));
    }

    if (nak380Waehlt (nur, "M-49"))
    {
        abschnitt ("380/M-49 sperrzeit_50ms");
        // Historie 32 x 0 -> T_eff = T_min = 153,0 dB bei 48 kHz, 166,6 dB bei
        // 44,1 kHz (Matrix §6.3 mit dem kalibrierten T_min); zwei
        // steigende Ueberschreitungen A = 500 und 1,1A = 550, beide lokale
        // Maxima gegen SF(n-1); dazwischen fuer 2 Hops ein Frame mit 1 dB.
        const double a = 500.0;
        const auto h = historie (0.0, 0, 0.0);
        pruefe (nakama::analyse::kSperrzeitMs == 50.0,
                "380/M-49 sperrzeit_50ms: Konstante kSperrzeitMs = 50,0",
                juce::String (nakama::analyse::kSperrzeitMs, 3));
        // 48 kHz: 1 Hop = 2048/48000 = 42,67 ms < 50 -> ein Ereignis;
        // 2 Hops = 85,33 ms >= 50 -> zwei. 44,1 kHz: 46,44 ms bzw. 92,88 ms.
        const int ein48 = folge (h, { 0.0, a, 1.1 * a });
        const int zwei48 = folge (h, { 0.0, a, 1.0, 1.1 * a });
        const int ein441 = folge (h, { 0.0, a, 1.1 * a }, kHop, 44100.0);
        const int zwei441 = folge (h, { 0.0, a, 1.0, 1.1 * a }, kHop, 44100.0);
        pruefe (ein48 == 1,
                "380/M-49 sperrzeit_50ms: 48 kHz, zwei steigende Ueberschreitungen im Abstand 1 Hop "
                "(42,67 ms) ergeben ein Ereignis",
                "Ereignisse " + juce::String (ein48));
        pruefe (zwei48 == 2,
                "380/M-49 sperrzeit_50ms: 48 kHz, Abstand 2 Hops (85,33 ms) ergibt zwei Ereignisse",
                "Ereignisse " + juce::String (zwei48));
        pruefe (ein441 == 1 && zwei441 == 2,
                "380/M-49 sperrzeit_50ms: 44,1 kHz, 1 Hop (46,44 ms) ein Ereignis, 2 Hops (92,88 ms) zwei",
                juce::String (ein441) + " / " + juce::String (zwei441));
    }
#else
    juce::ignoreUnused (nur);
    pruefe (false, "NAK-380 Detektor-Testzugang", "NAKAMA_FEATUREENGINE_TESTZUGANG fehlt");
#endif
}

__declspec(noinline) void nak380BandStereoUndLeereGruppe()
{
    abschnitt ("NAK-380 Etappe 2  band_stereo und leere Gruppe");
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
    using nakama::analyse::FeatureEngineTestzugang;
    using nakama::analyse::Gitter;
    using nakama::analyse::LiveBaender;
    using nakama::analyse::bitmapLies;

    auto engine = std::make_unique<FeatureEngine>();
    engine->vorbereiten (48000.0);
    FeatureEngineTestzugang::liveLeeren (*engine);
    for (int band = 120; band <= 123; ++band)
        FeatureEngineTestzugang::liveBandSetzen (
            *engine, band, 1.0e-3, 1,
            band == 123 ? 1.0e-6 : 0.0, 1.0e-6);

    LiveBaender live {};
    float stereo[Gitter::liveBaender] {};
    std::uint8_t stereoBitmap[(Gitter::liveBaender + 7) / 8] {};
    FeatureEngineTestzugang::liveFuellen (*engine, live, stereo, stereoBitmap);
    const double breite120 = Gitter::evidenzKante (121) - Gitter::evidenzKante (120);
    const double breite121 = Gitter::evidenzKante (122) - Gitter::evidenzKante (121);
    const double breite122 = Gitter::evidenzKante (123) - Gitter::evidenzKante (122);
    const double breite123 = Gitter::evidenzKante (124) - Gitter::evidenzKante (123);
    // R-380-8: Σs_b w_b / Σt_b w_b = 31,0446383/118,9808076.
    const double soll = breite123 / (breite120 + breite121 + breite122 + breite123);
    pruefe (bitmapLies (stereoBitmap, 35)
                && std::abs ((double) stereo[35] - soll) <= 1e-4,
            "380/M-06 band_stereo_leistungsgewichtet",
            "ist " + juce::String (stereo[35], 6) + ", Referenz 31,0446383/118,9808076="
                + juce::String (soll, 6) + "; ungewichtet 0,250000");

    FeatureEngineTestzugang::liveLeeren (*engine);
    live.leeren();
    std::fill (std::begin (stereo), std::end (stereo), 0.0f);
    nakama::analyse::bitmapNullen (stereoBitmap, Gitter::liveBaender);
    FeatureEngineTestzugang::liveFuellen (*engine, live, stereo, stereoBitmap);
    const bool leerOhneBit = ! bitmapLies (live.bitmap, 10)
                          && live.werte[10] == 0 && ! live.saturated;

    for (int band = Gitter::liveVon (10); band < Gitter::liveBisExkl (10); ++band)
        FeatureEngineTestzugang::liveBandSetzen (*engine, band, 0.0, 3, 0.0, 0.0);
    live.leeren();
    std::fill (std::begin (stereo), std::end (stereo), 0.0f);
    nakama::analyse::bitmapNullen (stereoBitmap, Gitter::liveBaender);
    FeatureEngineTestzugang::liveFuellen (*engine, live, stereo, stereoBitmap);
    const bool nullOhneBit = ! bitmapLies (live.bitmap, 10)
                          && live.werte[10] == 0 && ! live.saturated;
    pruefe (leerOhneBit && nullOhneBit,
            "380/M-07 gruppe_ohne_beleg_ohne_bit",
            "leer=" + juce::String (leerOhneBit ? "ohne Bit" : "FEHLER")
                + ", Summe 0 bei n=3=" + juce::String (nullOhneBit ? "ohne Bit" : "FEHLER"));
#else
    pruefe (false, "380/M-06 band_stereo_leistungsgewichtet",
            "NAKAMA_FEATUREENGINE_TESTZUGANG fehlt");
    pruefe (false, "380/M-07 gruppe_ohne_beleg_ohne_bit",
            "NAKAMA_FEATUREENGINE_TESTZUGANG fehlt");
#endif
}
} // namespace

int main (int argc, char* argv[])
{
    constexpr double fs = 48000.0;
    std::cout << "== Nakama SONDE-013 - Loudnessfenster, Headroom, Dynamik ==" << std::endl;

    if (argc == 3 && std::strcmp (argv[1], "--nak380") == 0)
    {
        nak380LraMesskern (argv[2]);
        nak380Detektoreinheit (argv[2]);
        std::cout << "\n-----------------------------------------" << std::endl;
        std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
        return fehler == 0 ? 0 : 1;
    }

    nak380BandStereoUndLeereGruppe();
    nak380LraMesskern (nullptr);
    nak380Detektoreinheit (nullptr);

    // ── M-01: drei Fenster, nicht ein Fenster mit drei Namen ──────────────
    //
    // Der Fall faehrt erst zwoelf Sekunden leise, dann laut. Kurz nach dem
    // Sprung MUSS Momentary (400 ms) schon oben sein und Short-term (3 s)
    // noch nicht — genau das ist der Unterschied zwischen den beiden, und
    // eine Umsetzung, die den einen aus dem anderen ableitet, kann ihn nicht
    // herstellen.
    abschnitt ("M-01  momentary_short_integrated_are_separate_windows");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };

        const auto leise = sinus (0.02, 1000.0, fs);
        const auto laut  = sinus (0.50, 1000.0, fs);

        const auto vorher = s.fahre (leise, bloeckeFuer (12.0, fs, s.frames));
        pruefe (vorher.lufsMGesetzt && vorher.lufsSGesetzt,
                "nach zwoelf Sekunden stehen beide Fenster",
                "M " + juce::String (vorher.lufsM, 2) + " LUFS, S "
                + juce::String (vorher.lufsS, 2) + " LUFS");
        pruefe (std::abs (vorher.lufsM - vorher.lufsS) < 0.5,
                "bei stehendem Pegel sind sie fast gleich - der Unterschied ist der "
                "VERLAUF, nicht die Rechnung",
                "Abstand " + juce::String (std::abs (vorher.lufsM - vorher.lufsS), 3) + " LU");

        // 0,8 s laut: mehr als zwei Momentary-Fenster, weniger als ein
        // Drittel des Short-term-Fensters.
        const auto nachSprung = s.fahre (laut, bloeckeFuer (0.8, fs, s.frames));
        pruefe (nachSprung.lufsMGesetzt && nachSprung.lufsSGesetzt,
                "0,8 s nach dem Sprung tragen beide weiterhin einen Wert");
        pruefe (nachSprung.lufsM - nachSprung.lufsS > 5.0,
                "Momentary ist dem Sprung gefolgt, Short-term noch nicht - die zwei "
                "Fenster sind wirklich verschieden lang",
                "M " + juce::String (nachSprung.lufsM, 2) + " gegen S "
                + juce::String (nachSprung.lufsS, 2) + " LUFS, Abstand "
                + juce::String (nachSprung.lufsM - nachSprung.lufsS, 2) + " LU");

        // Und nach genug Zeit treffen sie sich wieder. Ohne diese Gegenprobe
        // koennte der Fall oben auch von einem konstanten Versatz kommen.
        const auto spaeter = s.fahre (laut, bloeckeFuer (5.0, fs, s.frames));
        pruefe (std::abs (spaeter.lufsM - spaeter.lufsS) < 0.5,
                "fuenf Sekunden spaeter sind sie wieder beieinander - der Abstand oben "
                "war der Sprung, kein Versatz",
                "Abstand " + juce::String (std::abs (spaeter.lufsM - spaeter.lufsS), 3) + " LU");
    }

    // ── M-01: Momentary steht frueher als Short-term ──────────────────────
    abschnitt ("M-01  jedes Fenster braucht seine eigene Fuellung");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto ton = sinus (0.3, 1000.0, fs);

        // 0,6 s: mehr als 400 ms, weit unter 3 s.
        const auto frueh = s.fahre (ton, bloeckeFuer (0.6, fs, s.frames));
        pruefe (frueh.lufsMGesetzt && ! frueh.lufsSGesetzt,
                "nach 0,6 s traegt der Frame Momentary, aber KEIN Short-term - eine "
                "Kurzzeitlautheit ueber 0,6 s waere eine andere Zahl mit demselben Namen",
                juce::String ("M ") + (frueh.lufsMGesetzt ? "ja" : "nein")
                + ", S " + (frueh.lufsSGesetzt ? "ja" : "nein"));

        const auto spaet = s.fahre (ton, bloeckeFuer (3.0, fs, s.frames));
        pruefe (spaet.lufsSGesetzt,
                "nach weiteren drei Sekunden steht auch Short-term");
    }

    // ── M-09: integration_samples, der Kern von NAK-68 ────────────────────
    abschnitt ("M-09  thin_frame_after_boundary_is_recognisable");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto ton = sinus (0.3, 1000.0, fs);

        const auto voll = s.fahre (ton, 60);
        pruefe (voll.integrationGesetzt && voll.integrationSamples > 0,
                "ein Rahmen im Dauerbetrieb sagt, ueber wie viel Audio er gerechnet wurde",
                juce::String ((int) voll.integrationSamples) + " Samples je Kanal");
        pruefe (voll.integrationSamples >= 4800 && voll.integrationSamples <= 4800 + 512,
                "und die Zahl passt zur Livekadenz von 10 Hz plus hoechstens einem Block",
                juce::String ((int) voll.integrationSamples) + " gegen 4800..5312");

        // Eine Grenze MITTEN im Rahmen ziehen und den ersten Frame danach
        // nehmen: er traegt weniger integriertes Audio. Genau diesen
        // Unterschied konnte ein Empfaenger vor NAK-68 nicht sehen.
        //
        // ⚠️ Die Stelle ist tragend. `liveSamples` ueberlebt eine Grenze
        // bewusst ("die Grenze schneidet den Inhalt, nicht die Uhr", SONDE-009
        // §10.1), also bleibt die KADENZ stehen, waehrend `rahmenSamples`
        // genullt wird. Liegt die Grenze unmittelbar NACH einem Frame, faellt
        // der naechste wieder nach vollen 4800 Samples und ist gar nicht
        // duenn — der Fall haette dann nichts gemessen. Er stellt den Zustand
        // deshalb her, statt ihn auszurechnen: erst bis zum naechsten Frame
        // fahren, dann ein Stueck weiter, dann die Luecke.
        while (! s.sende (ton)) {}
        for (int i = 0; i < 5; ++i)
            (void) s.sende (ton);
        s.strom += 4096;                       // Luecke im Strom = Grenze
        FeatureFrame duenn {};
        bool gab = false;
        for (int i = 0; i < 40 && ! gab; ++i)
            if (s.sende (ton)) { duenn = e.frame(); gab = true; }
        pruefe (gab && duenn.integrationGesetzt,
                "auch der erste Rahmen nach einer Grenze traegt das Feld");
        pruefe (gab && duenn.integrationSamples < voll.integrationSamples,
                "und er sagt ehrlich, dass er ueber WENIGER Audio gerechnet wurde",
                juce::String ((int) duenn.integrationSamples) + " gegen "
                + juce::String ((int) voll.integrationSamples) + " Samples");
    }

    // ── M-03: PSR rechnet gegen das 3-s-True-Peak-Maximum ─────────────────
    abschnitt ("M-03  plr_psr_definitions_and_labels");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto ton = sinus (0.5, 1000.0, fs);
        const auto f = s.fahre (ton, bloeckeFuer (6.0, fs, s.frames));

        pruefe (f.truePeakGesetzt && f.truePeakPassageGesetzt,
                "der Frame traegt True Peak des Rahmens UND der Passage",
                "Rahmen " + juce::String (f.truePeakDb, 3) + " dBTP, Passage "
                + juce::String (f.truePeakPassageDb, 3) + " dBTP");
        pruefe (f.truePeakDb >= f.peakDb - 0.01f,
                "der True Peak liegt nie unter dem Sample-Peak desselben Rahmens",
                "TP " + juce::String (f.truePeakDb, 3) + " gegen SP "
                + juce::String (f.peakDb, 3) + " dB");
        pruefe (f.truePeakPassageDb >= f.truePeakDb - 0.01f,
                "und das Passagenmaximum liegt nie unter dem Rahmenmaximum");

        pruefe (f.psrGesetzt, "PSR steht, sobald das 3-s-Fenster steht");

        // ⚠️ Der eigentliche Fall. Bei einem stationaeren Sinus sind
        // Rahmen-Sample-Peak, Rahmen-True-Peak und 3-s-True-Peak-Maximum fast
        // dieselbe Zahl — dort kann kein Test die drei auseinanderhalten. Der
        // Rotbeweis `SONDE-013-rot-M-03-psr.txt` hat genau das gezeigt: die
        // erste Fassung dieses Falls blieb gruen, als PSR wieder gegen den
        // Sample-Peak des 100-ms-Rahmens rechnete.
        //
        // Das Signal hier trennt sie: ein lauter Impuls, danach zwei Sekunden
        // leise. Der AKTUELLE Rahmen ist leise (sein Peak liegt tief), das
        // 3-s-Fenster enthaelt den Impuls noch. Wer gegen den Rahmen rechnet,
        // bekommt eine um mehr als 20 dB zu kleine Zahl.
        FeatureEngine e2;
        e2.vorbereiten (fs);
        Speiser s2 { e2 };
        const auto leiseTon = sinus (0.02, 1000.0, fs);
        s2.fahre (leiseTon, bloeckeFuer (4.0, fs, s2.frames));
        s2.fahre (sinus (0.9, 1000.0, fs), bloeckeFuer (0.15, fs, s2.frames));
        const auto nachImpuls = s2.fahre (leiseTon, bloeckeFuer (2.0, fs, s2.frames));

        pruefe (nachImpuls.psrGesetzt && nachImpuls.truePeakGesetzt,
                "im Rahmen NACH dem Impuls stehen PSR und der Rahmen-True-Peak");
        pruefe (nachImpuls.truePeakDb < -25.0f,
                "der aktuelle Rahmen ist leise - sein True Peak liegt tief",
                juce::String (nachImpuls.truePeakDb, 2) + " dBTP");
        const double gegenRahmen = (double) nachImpuls.truePeakDb - (double) nachImpuls.lufsS;
        pruefe ((double) nachImpuls.psrDb - gegenRahmen > 20.0,
                "PSR rechnet trotzdem gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters "
                "- der Impuls ist darin noch enthalten",
                "PSR " + juce::String (nachImpuls.psrDb, 2) + " dB; gegen den Rahmen "
                "waeren es " + juce::String (gegenRahmen, 2) + " dB");
        pruefe (std::abs ((double) nachImpuls.psrDb
                          - (-0.92 - (double) nachImpuls.lufsS)) < 1.0,
                "und die Zahl passt zum Impulspegel: 20*log10(0,9) minus LUFS-S",
                "gemessen " + juce::String (nachImpuls.psrDb, 2) + " dB, erwartet "
                + juce::String (-0.92 - (double) nachImpuls.lufsS, 2) + " dB");

        // PLR entsteht erst im Sondenprozessor (er haelt LUFS-I). Die Engine
        // liefert nur die Zutat - und behauptet deshalb kein PLR.
        pruefe (! f.plrGesetzt,
                "die Engine selbst setzt KEIN PLR - LUFS-I kommt aus dem "
                "Loudness-Akkumulator und wird erst im Sondenprozessor zugemischt");
    }

    // ── M-02: der Peak am Rahmenende (Selbstauditfund dieser Etappe) ─────
    //
    // Der Interpolatorkern ist um seine halbe Laenge zentriert und sieht ein
    // Sample deshalb erst zwoelf Samples spaeter. Liegt der EINZIGE
    // Spitzenwert eines Rahmens in dessen letzten Samples, haette der reine
    // Interpolatorwert ihn noch nicht — der Frame truege einen True Peak
    // UNTER seinem eigenen Sample-Peak, und beide Leser lehnten ihn als
    // `true_peak_unter_sample_peak` ab.
    //
    // ⚠️ Zwei Fallen liegen in diesem Fall, und beide sind in der ersten
    // Fassung zugeschlagen (der Rotbeweis kam zweimal gruen zurueck):
    //
    // 1. Der Interpolatorverlauf ueberlebt Blockgrenzen. Ein Impuls in JEDEM
    //    Block wird im naechsten gesehen — also traegt der Rahmen den Peak
    //    trotzdem, nur von einem frueheren Impuls. Es muss GENAU EIN Impuls
    //    im Rahmen sein, und der ganz am Ende.
    // 2. Wo der Rahmen endet, haengt an der Kadenzarithmetik. Der Fall
    //    RECHNET sie nicht aus, sondern MISST sie zuerst — eine Wette auf
    //    4800 gegen 5120 Samples waere genau die Sorte Annahme, an der die
    //    Zwillingsprobe in §10.2 schon einmal vorbeigelaufen ist.
    abschnitt ("M-02  impuls_am_rahmenende");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto leise = sinus (0.02, 1000.0, fs);

        // Schritt 1: messen, nach wie vielen Bloecken ein Rahmen faellt.
        s.fahre (leise, 30);                    // einschwingen
        int bisFrame = 1;
        while (! s.sende (leise))
            ++bisFrame;

        // Schritt 2: den Rahmen leise fuellen und NUR im letzten Block eine
        // Spitze in die letzten vier Samples legen.
        const int blockLaenge = s.frames;
        const auto mitSpitzeAmEnde = [blockLaenge] (std::uint64_t n)
        {
            const std::uint64_t imBlock = n % (std::uint64_t) blockLaenge;
            if (imBlock >= (std::uint64_t) (blockLaenge - 4))
                return 0.95f;
            return (float) (0.02 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
        };

        int gesehen = 0, verletzt = 0;
        float schlimmsteLuecke = 0.0f;
        for (int runde = 0; runde < 8; ++runde)
        {
            for (int i = 0; i < bisFrame - 1; ++i)
                (void) s.sende (leise);
            if (! s.sende (mitSpitzeAmEnde))
            {
                // Die Kadenz ist verrutscht (etwa durch einen Rundungsrest):
                // bis zum naechsten Frame weiterfahren und die Runde
                // verwerfen, statt einen halben Rahmen zu messen.
                while (! s.sende (leise)) {}
                continue;
            }
            const auto& f = e.frame();
            if (! (f.truePeakGesetzt && f.peakGesetzt) || f.peakDb < -3.0f)
                continue;                       // die Spitze lag nicht in diesem Rahmen
            ++gesehen;
            if (f.truePeakDb < f.peakDb)
            {
                ++verletzt;
                schlimmsteLuecke = std::max (schlimmsteLuecke, f.peakDb - f.truePeakDb);
            }
        }

        pruefe (gesehen > 0,
                "der Zustand ist wirklich hergestellt: Rahmen, deren einzige Spitze in "
                "den letzten vier Samples liegt",
                juce::String (gesehen) + " von 8 Runden, Rahmen alle "
                + juce::String (bisFrame) + " Bloecke");
        pruefe (verletzt == 0,
                "in KEINEM davon liegt der True Peak unter dem Sample-Peak - der "
                "Abtastpunkt geht in den True Peak ein, weil er zur rekonstruierten "
                "Wellenform gehoert",
                verletzt == 0 ? juce::String ("0 Verletzungen")
                              : juce::String (verletzt) + " Verletzungen, groesste Luecke "
                                + juce::String (schlimmsteLuecke, 3) + " dB");
    }

    // ── M-03: Headroom ist eine Verteilung, keine Spitze ──────────────────
    abschnitt ("M-03  headroom_is_distribution_not_single_peak");
    {
        // Zwei Signale mit DEMSELBEN Maximum: eines durchgehend laut, eines
        // ueberwiegend leise mit einer einzelnen lauten Stelle. Ein
        // Einzelwert kann sie nicht unterscheiden, drei Perzentile schon.
        FeatureEngine eDurch, eSpitze;
        eDurch.vorbereiten (fs);
        eSpitze.vorbereiten (fs);
        Speiser sD { eDurch }, sS { eSpitze };

        const auto laut  = sinus (0.5,  1000.0, fs);
        const auto leise = sinus (0.02, 1000.0, fs);

        const auto durch = sD.fahre (laut, bloeckeFuer (5.0, fs, sD.frames));
        // Bei der Spitzenvariante: lange leise, kurz laut, wieder leise.
        sS.fahre (leise, bloeckeFuer (3.0, fs, sS.frames));
        sS.fahre (laut,  bloeckeFuer (0.3, fs, sS.frames));
        const auto spitze = sS.fahre (leise, bloeckeFuer (1.7, fs, sS.frames));

        pruefe (durch.headroomGesetzt && spitze.headroomGesetzt,
                "beide tragen eine Headroomverteilung",
                "Fenster " + juce::String ((int) durch.headroomFenster) + " / "
                + juce::String ((int) spitze.headroomFenster) + " Rahmen");
        pruefe (durch.headroomP10Db <= durch.headroomP50Db
                  && durch.headroomP50Db <= durch.headroomP95Db,
                "die drei Punkte sind geordnet - P10 <= P50 <= P95");
        pruefe (std::abs (durch.headroomP95Db - spitze.headroomP95Db) < 1.0,
                "beide haben nahezu dasselbe P95 - die laute Stelle ist in beiden drin",
                "durchgehend " + juce::String (durch.headroomP95Db, 2) + " dBTP, "
                "Spitze " + juce::String (spitze.headroomP95Db, 2) + " dBTP");
        pruefe (durch.headroomP50Db - spitze.headroomP50Db > 10.0,
                "aber ihr P50 liegt weit auseinander - GENAU das kann ein einzelner "
                "Maximalwert nicht ausdruecken",
                "durchgehend " + juce::String (durch.headroomP50Db, 2) + " dBTP, "
                "Spitze " + juce::String (spitze.headroomP50Db, 2) + " dBTP");
        pruefe (spitze.headroomP95Db - spitze.headroomP10Db > 10.0,
                "und die Spanne der Spitzenvariante ist gross - der Headroom SCHWANKT",
                "Spanne " + juce::String (spitze.headroomP95Db - spitze.headroomP10Db, 2)
                + " dB");
    }

    // ── M-04: Crest in mehreren Fenstern ──────────────────────────────────
    abschnitt ("M-04  crest_multiple_windows");
    {
        // Ein Signal, das im 100-ms-Rahmen ruhig aussieht und ueber drei
        // Sekunden nicht: leiser Dauerton mit einem kurzen lauten Impuls
        // etwa alle zwei Sekunden.
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const double impulsPeriode = 2.0 * fs;
        const auto stossig = [impulsPeriode] (std::uint64_t n)
        {
            const double phase = std::fmod ((double) n, impulsPeriode);
            const double grund = 0.03 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0);
            const double impuls = (phase < 64.0)
                ? 0.7 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0) : 0.0;
            return (float) (grund + impuls);
        };

        // Ein Frame, der GERADE KEINEN Impuls enthaelt: sein Rahmencrest ist
        // klein, sein 3-s-Crest gross.
        FeatureFrame ruhig {};
        bool gefunden = false;
        for (int i = 0; i < 600 && ! gefunden; ++i)
            if (s.sende (stossig))
            {
                const auto& f = e.frame();
                if (f.crestGesetzt && f.crestKurzGesetzt && f.crestDb < 6.0f)
                {
                    ruhig = f;
                    gefunden = true;
                }
            }
        pruefe (gefunden,
                "es gibt Rahmen ohne Impuls, in denen BEIDE Crestfenster stehen");
        pruefe (gefunden && ruhig.crestKurzDb - ruhig.crestDb > 10.0f,
                "und dort liegt der 3-s-Crest weit ueber dem des 100-ms-Rahmens - das "
                "ist der Unterschied, den ein einzelnes Fenster nicht sehen kann",
                "Rahmen " + juce::String (ruhig.crestDb, 2) + " dB, 3 s "
                + juce::String (ruhig.crestKurzDb, 2) + " dB");
    }

    // ── M-04: LRA wird nicht vorgetaeuscht ────────────────────────────────
    abschnitt ("M-04  lra_below_sixty_seconds_is_not_a_number");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        // Ein Signal mit echter Dynamik: der Pegel wandert langsam ueber
        // rund 20 LU. Ohne Dynamik waere LRA nahe null, und ein Test, der
        // nur "> 0" fordert, saehe keinen Unterschied zwischen "gerechnet"
        // und "vergessen".
        const auto wandernd = [] (std::uint64_t n)
        {
            const double t = (double) n / 48000.0;
            const double huellkurve = 0.30 * std::pow (10.0,
                (-10.0 + 10.0 * std::sin (kZweiPi * t / 25.0)) / 20.0);
            return (float) (huellkurve * std::sin (kZweiPi * 1000.0 * t));
        };

        const auto bei30 = s.fahre (wandernd, bloeckeFuer (30.0, fs, s.frames));
        pruefe (! bei30.lraGesetzt,
                "nach 30 s gibt es KEINEN LRA-Wert - `nicht belastbar` heisst hier "
                "Abwesenheit, nicht eine Null");

        const auto bei75 = s.fahre (wandernd, bloeckeFuer (45.0, fs, s.frames));
        pruefe (bei75.lraGesetzt,
                "nach insgesamt 75 s steht er");
        pruefe (bei75.lraGesetzt && bei75.lraLu > 0.5f && bei75.lraLu < 40.0f,
                "und traegt eine Dynamikspanne, die zum wandernden Pegel passt",
                juce::String (bei75.lraLu, 2) + " LU");

        // Gegenprobe: ein Signal OHNE Dynamik ergibt eine kleine Spanne. Ohne
        // sie waere oben nur gezeigt, dass irgendeine Zahl entsteht.
        FeatureEngine flach;
        flach.vorbereiten (fs);
        Speiser sf { flach };
        const auto stetig = sinus (0.2, 1000.0, fs);
        const auto lang = sf.fahre (stetig, bloeckeFuer (75.0, fs, sf.frames));
        pruefe (lang.lraGesetzt && lang.lraLu < 1.0f,
                "ein Signal ohne Dynamik ergibt eine LRA nahe null - der Wert misst "
                "wirklich die Spanne und nicht die Laufzeit",
                juce::String (lang.lraLu, 3) + " LU");
    }

    // ── M-04: die 60-s-Regel zaehlt GEEIGNETES Material ───────────────────
    abschnitt ("M-04  die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto stille = [] (std::uint64_t) { return 0.0f; };
        const auto lang = s.fahre (stille, bloeckeFuer (90.0, fs, s.frames));
        pruefe (! lang.lraGesetzt,
                "90 s Stille ergeben KEIN LRA - eine Minute Stille ist keine Minute "
                "Material (§39.1: 'rund 60 s GEEIGNETEN Materials')");
    }

    // ── M-07: nichts Nichtendliches erreicht eine neue Metrik ─────────────
    abschnitt ("M-07  non_finite_never_reaches_wire_for_new_metrics");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto giftig = [] (std::uint64_t n)
        {
            if (n % 997u == 0u) return std::numeric_limits<float>::quiet_NaN();
            if (n % 991u == 0u) return std::numeric_limits<float>::infinity();
            return (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
        };
        const auto f = s.fahre (giftig, bloeckeFuer (8.0, fs, s.frames));

        const struct { const char* name; bool bit; float wert; } neue[] = {
            { "lufsM", f.lufsMGesetzt, f.lufsM },
            { "truePeakDb", f.truePeakGesetzt, f.truePeakDb },
            { "truePeakPassageDb", f.truePeakPassageGesetzt, f.truePeakPassageDb },
            { "psrDb", f.psrGesetzt, f.psrDb },
            { "crestKurzDb", f.crestKurzGesetzt, f.crestKurzDb },
            { "lraLu", f.lraGesetzt, f.lraLu },
            { "headroomP10Db", f.headroomGesetzt, f.headroomP10Db },
            { "headroomP50Db", f.headroomGesetzt, f.headroomP50Db },
            { "headroomP95Db", f.headroomGesetzt, f.headroomP95Db },
        };
        bool allesEndlich = true;
        juce::String schuldig;
        for (const auto& n : neue)
            if (n.bit && ! std::isfinite (n.wert))
            {
                allesEndlich = false;
                schuldig = n.name;
            }
        pruefe (allesEndlich,
                "NaN und Inf im Eingang erzeugen in KEINER neuen Metrik einen "
                "nichtendlichen Wert - der Riegel liegt beim Erzeugen, nicht auf der "
                "Leitung",
                allesEndlich ? juce::String ("alle 9 Traeger endlich") : schuldig);
        // Nacharbeit 1 (2026-09-04, Befund B07): die alte Zusage an dieser
        // Stelle lautete "und die Metriken entstehen ueberhaupt". Genau die war
        // der Fehler. M-07 verlangt beim Erzeugen "Wert 0 mit `gueltig=false`"
        // UND einen Zaehler; ein Rahmen, der ueber stillgelegte Samples
        // rechnet und seine Skalare trotzdem als gesetzt meldet, sieht aus wie
        // eine saubere Messung.
        //
        // 🔑 NAK-283 F10 (R-283-5): der Riegel hat ZWEI Stufen. Der Rahmen mit
        // ersetzten Samples traegt keine sampleabhaengigen Skalare; und jede
        // Loudnesszelle traegt das Merkmal "enthaelt ersetztes Material", also
        // bleiben die Fenstermetriken - Momentary (400 ms), Short-term, PSR und
        // Crest ueber 3 s - ungesetzt, bis ihr Fenster frei davon ist. Kein
        // Dauerschweigen, aber auch keine Freigabe mit dem naechsten Rahmen: der
        // Wert kommt genau an seiner Fensterfrist wieder (M-58, unten).
        pruefe (f.nichtEndlichRahmen > 0,
                "der Rahmen ZAEHLT seine nicht-endlichen Eingangssamples",
                juce::String ((int) f.nichtEndlichRahmen));
        pruefe (! f.truePeakGesetzt && ! f.lufsMGesetzt && ! f.crestKurzGesetzt,
                "und traegt seine sampleabhaengigen Skalare NICHT - 'nicht gemessen' "
                "statt 'gemessen und sauber' (M-07)");

        // Die Gegenprobe: derselbe Aufbau mit sauberem Material traegt sie.
        FeatureEngine rein;
        rein.vorbereiten (fs);
        Speiser s2 { rein };
        const auto sauber = s2.fahre (
            [] (std::uint64_t n)
            { return (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0)); },
            bloeckeFuer (8.0, fs, s2.frames));
        pruefe (sauber.nichtEndlichRahmen == 0 && sauber.truePeakGesetzt,
                "ohne NaN traegt derselbe Aufbau seine Skalare - kein Dauerschweigen");

        // ── NAK-283 M-58: aktivieren <-> abklingen am Fensterrand ─────────
        //
        // Die Vergiftung endet mit Block 750 bei Sample 384 000 - einem Zellen-
        // und Rahmenrand (750 x 512 = 80 x 4800 = 75 x 5120). Ab hier laufen
        // beide Aufbauten mit 480er-Bloecken: zehn Bloecke sind genau eine
        // Zelle und genau ein Rahmen, jeder Rahmen faellt also auf einen
        // Fensterrand. Die vier Messpunkte liegen dort, wo eine Frist eine
        // Zelle zu frueh oder zu spaet kippen wuerde; eine einzelne Messung
        // fuenf Sekunden spaeter traefe keine der beiden Richtungen.
        const auto sauberTon = [] (std::uint64_t n)
        { return (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0)); };
        s.frames = 480;
        s2.frames = 480;
        std::vector<FeatureFrame> nach (32), referenz (32);
        bool takt = s.strom == 384000u && s2.strom == 384000u;
        for (int k = 1; k <= 31; ++k)
        {
            std::uint64_t bei = 0, beiRef = 0;
            for (int b = 0; b < 10; ++b)
            {
                if (s.sende (sauberTon))  { nach[(std::size_t) k] = e.frame();        bei = s.strom; }
                if (s2.sende (sauberTon)) { referenz[(std::size_t) k] = rein.frame(); beiRef = s2.strom; }
            }
            const std::uint64_t soll = 384000u + (std::uint64_t) k * 4800u;
            takt = takt && bei == soll && beiRef == soll;
        }
        pruefe (takt,
                "M-58: jeder Rahmen der Erholungsstrecke endet auf einem Zellen- und Rahmenrand "
                "(384 000 + k x 4 800)");
        pruefe (! nach[3].lufsMGesetzt && nach[4].lufsMGesetzt,
                "NAK-283 M-58 (M-07, Erholungsstrecke): Momentary - bei 0,3 s nach dem Ende "
                "der Ersetzung ungesetzt, bei 0,4 s gesetzt",
                juce::String ("0,3 s ") + (nach[3].lufsMGesetzt ? "gesetzt" : "leer")
                    + ", 0,4 s " + (nach[4].lufsMGesetzt ? "gesetzt" : "leer"));
        pruefe (! nach[29].lufsSGesetzt && nach[30].lufsSGesetzt,
                "NAK-283 M-58 (M-07, Erholungsstrecke): Short-term - bei 2,9 s ungesetzt, bei "
                "3,0 s gesetzt",
                juce::String ("2,9 s ") + (nach[29].lufsSGesetzt ? "gesetzt" : "leer")
                    + ", 3,0 s " + (nach[30].lufsSGesetzt ? "gesetzt" : "leer"));
        const bool beideDa = nach[30].lufsSGesetzt && nach[30].lufsMGesetzt
                          && referenz[30].lufsSGesetzt && referenz[30].lufsMGesetzt;
        const double abstandS = std::abs ((double) nach[30].lufsS - (double) referenz[30].lufsS);
        const double abstandM = std::abs ((double) nach[30].lufsM - (double) referenz[30].lufsM);
        pruefe (beideDa && abstandS <= 0.1 && abstandM <= 0.1
                    && nach[30].nichtEndlichRahmen == 0 && nach[30].nichtEndlichEvidenz == 0,
                "NAK-283 M-58 (M-07, Erholungsstrecke): bei 3,0 s entsprechen beide Werte der "
                "sauberen Referenz (+-0,1 LU), und beide Zaehler sind 0",
                "S " + juce::String (nach[30].lufsS, 4) + " gegen " + juce::String (referenz[30].lufsS, 4)
                    + " LUFS, M " + juce::String (nach[30].lufsM, 4) + " gegen "
                    + juce::String (referenz[30].lufsM, 4) + " LUFS, Zaehler "
                    + juce::String ((int) nach[30].nichtEndlichRahmen) + "/"
                    + juce::String ((int) nach[30].nichtEndlichEvidenz));
    }

    // ── M-75 / M-77: der verworfene Block schliesst auch die neuen Fenster ─
    //
    // ⚠️ Was hier NICHT steht, und warum. M-77 beschreibt einen Hostblock
    // ueber der vorallokierten SLOTKAPAZITAET. Die Kapazitaet gehoert der
    // `StampedAudioQueue`, nicht dieser Engine — die Engine bekommt nur
    // ganze, bereits angenommene Bloecke, und ihr einen Block mit
    // `sampleCount` jenseits des uebergebenen Puffers zu reichen, waere
    // schlicht ein Lesefehler im Test, kein Vertragsfall (gemessen: er
    // stuerzt, und zwar zu Recht). Der Oversizepfad selbst ist B4.
    //
    // Was die Engine SIEHT, ist die Folge: der Block fehlt, im Strom klafft
    // eine Luecke, und daraus wird ein neues `continuity_segment`. Genau das
    // misst dieser Abschnitt — und zwar fuer die Fenster, die es beim
    // Schreiben von §48.1 noch nicht gab.
    abschnitt ("M-75/M-77  der verworfene Analyseblock schliesst auch die neuen Fenster");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto ton = sinus (0.4, 1000.0, fs);
        s.fahre (ton, bloeckeFuer (6.0, fs, s.frames));

        const auto vorher = e.frame();
        pruefe (vorher.truePeakGesetzt && vorher.crestKurzGesetzt && vorher.psrGesetzt,
                "vor dem verworfenen Block stehen die neuen Fenster");

        const auto segmenteVorher = e.segmentwechsel();
        s.strom += 65536;                      // ein verworfener Block: Stromluecke
        // ⚠️ NACH der Luecke laeuft ein DEUTLICH leiseres Signal. Mit
        // demselben Ton wie davor koennte dieser Abschnitt gar nichts zeigen:
        // das Passagenmaximum waere danach wieder derselbe Wert, ob es die
        // Grenze nun ueberbrueckt hat oder neu entstanden ist. Genau die
        // Sorte Gegenprobe, die nichts misst.
        const auto leise = sinus (0.004, 1000.0, fs);
        FeatureFrame danach {};
        bool gab = false;
        for (int i = 0; i < 30 && ! gab; ++i)
            if (s.sende (leise)) { danach = e.frame(); gab = true; }

        pruefe (e.segmentwechsel() > segmenteVorher,
                "die Luecke zieht eine Segmentgrenze - der Verlust ist sichtbar, nicht "
                "interpoliert",
                "Segmentwechsel " + juce::String ((int) e.segmentwechsel()));
        pruefe (gab && ! danach.crestKurzGesetzt && ! danach.psrGesetzt,
                "danach tragen die 3-s-Fenster KEINEN Wert mehr - sie sind geschlossen, "
                "nicht fortgeschrieben");
        pruefe (gab && danach.truePeakPassageDb < vorher.truePeakPassageDb - 20.0f,
                "und das Passagenmaximum hat die Grenze nicht ueberbrueckt - es ist das "
                "der neuen, leisen Passage",
                "vorher " + juce::String (vorher.truePeakPassageDb, 2) + " dBTP, danach "
                + juce::String (danach.truePeakPassageDb, 2) + " dBTP");
    }

    // ── Grenzen: kein neues Fenster ueberbrueckt eine Transportgrenze ─────
    //
    // G13 in B5 misst dasselbe strukturell ueber ALLE Felder. Hier steht die
    // Zusage noch einmal namentlich fuer die neuen Traeger, damit ein
    // Regressionsfall lokalisierbar ist statt nur "irgendein Feld".
    abschnitt ("M-84  jede Grenze trennt auch die neuen Fenster, namentlich");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto laut = sinus (0.6, 1000.0, fs);
        const auto leise = sinus (0.01, 1000.0, fs);

        const auto vorher = s.fahre (laut, bloeckeFuer (6.0, fs, s.frames));
        pruefe (vorher.truePeakPassageGesetzt && vorher.headroomGesetzt,
                "vor der Grenze stehen Passagenmaximum und Headroomverteilung",
                "Passage " + juce::String (vorher.truePeakPassageDb, 2) + " dBTP");

        s.strom += 8192;                       // Stromluecke = Grenze
        const auto nachher = s.fahre (leise, bloeckeFuer (2.0, fs, s.frames));
        pruefe (nachher.truePeakPassageGesetzt
                  && nachher.truePeakPassageDb < vorher.truePeakPassageDb - 20.0f,
                "nach der Grenze ist das Passagenmaximum das der NEUEN Passage - der "
                "laute Peak von vorher ist weg",
                "vorher " + juce::String (vorher.truePeakPassageDb, 2) + " dBTP, nachher "
                + juce::String (nachher.truePeakPassageDb, 2) + " dBTP");
        pruefe (! nachher.headroomGesetzt
                  || nachher.headroomP95Db < vorher.headroomP95Db - 20.0f,
                "und die Headroomverteilung ist entweder leer oder ganz aus dem neuen "
                "Material - sie mischt die zwei Passagen nicht");
        pruefe (! nachher.crestKurzGesetzt && ! nachher.psrGesetzt,
                "die 3-s-Fenster sind nach zwei Sekunden noch nicht wieder voll - sie "
                "wurden wirklich geleert, nicht fortgeschrieben");
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-283 Etappe 5 · F08 (R-283-5, Feinheit 7; U41 vom 13.09.2026)
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("NAK-283 M-53/M-54  headroomverteilung_umfasst_die_ganze_passage / "
               "headroomfenster_nennt_die_passagenlaenge");
    {
        // Uebergabe §4 F08 woertlich: 14 s, erste Haelfte 1-kHz-Ton mit
        // Amplitude 0,5, zweite Haelfte 0,0005 - 60 dB Abstand. 480er-Bloecke:
        // zehn Bloecke sind genau ein Rahmen, 14 s also genau 140 Rahmen.
        const std::int64_t ende = 14 * 48000;
        const auto passage = [ende] (std::uint64_t n)
        {
            const double amp = (std::int64_t) n < ende / 2 ? 0.5 : 0.0005;
            return (float) (amp * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
        };
        const auto lauf = [&passage, ende] (bool binden)
        {
            auto halter = std::make_unique<FeatureEngine>();
            auto& e = *halter;
            e.vorbereiten (48000.0);
            Speiser s { e };
            s.frames = 480;
            if (binden)
                e.setzePassagenfenster (0, ende, e.transportEpocheJetzt());
            FeatureFrame amEnde {};
            int rahmen = 0;
            while (s.projekt < ende)
                if (s.sende (passage)) { amEnde = e.frame(); ++rahmen; }
            return std::make_pair (amEnde, rahmen);
        };
        const auto [mit, rahmenMit] = lauf (true);
        const auto [ohne, rahmenOhne] = lauf (false);
        const auto text = [] (const FeatureFrame& f)
        {
            return "P10 " + juce::String (f.headroomP10Db, 3) + ", P50 "
                 + juce::String (f.headroomP50Db, 3) + ", P95 " + juce::String (f.headroomP95Db, 3)
                 + " dBTP, headroomFenster " + juce::String ((int) f.headroomFenster)
                 + (f.headroomGesetzt ? "" : ", KEIN Bit");
        };
        pruefe (rahmenMit == 140 && rahmenOhne == 140,
                "M-53: 14 s bei 10 Hz sind 140 publizierte Rahmen",
                juce::String (rahmenMit) + " / " + juce::String (rahmenOhne));
        pruefe (mit.headroomGesetzt && std::abs (mit.headroomP95Db + 6.0206f) <= 0.1f
                    && std::abs (mit.headroomP10Db + 66.0206f) <= 0.1f,
                "NAK-283 M-53: headroomverteilung_umfasst_die_ganze_passage - P95 traegt den "
                "fruehen lauten, P10 den spaeten leisen Abschnitt",
                text (mit));
        pruefe (mit.headroomGesetzt && mit.headroomFenster == 140u,
                "NAK-283 M-54: headroomfenster_nennt_die_passagenlaenge - die Zahl der Rahmen der "
                "Passage (140), nicht 64",
                text (mit));
        pruefe (ohne.headroomGesetzt && ohne.headroomFenster == 64u
                    && std::abs (ohne.headroomP95Db + 66.0206f) <= 0.1f
                    && std::abs (ohne.headroomP10Db + 66.0206f) <= 0.1f,
                "NAK-283 U41 (User 13.09.2026): ohne markierte Passage bleibt die Verteilung das "
                "gleitende Fenster der letzten 64 Rahmen (6,4 s), headroomFenster nennt seine Belegung",
                text (ohne));
    }

    abschnitt ("NAK-283 M-55  headroom_unter_vier_rahmen_bleibt_ungesetzt");
    {
        const auto lauf = [] (int rahmen, std::uint32_t& fensterMax)
        {
            auto halter = std::make_unique<FeatureEngine>();
            auto& e = *halter;
            e.vorbereiten (48000.0);
            Speiser s { e };
            s.frames = 480;
            e.setzePassagenfenster (0, (std::int64_t) rahmen * 4800, e.transportEpocheJetzt());
            const auto ton = sinus (0.5, 1000.0, 48000.0);
            bool gesetzt = false;
            fensterMax = 0;
            for (int i = 0; i < (rahmen + 10) * 10; ++i)
                if (s.sende (ton) && e.frame().headroomGesetzt)
                {
                    gesetzt = true;
                    fensterMax = std::max (fensterMax, e.frame().headroomFenster);
                }
            return gesetzt;
        };
        std::uint32_t fenster3 = 0, fenster4 = 0;
        const bool drei = lauf (3, fenster3);
        const bool vier = lauf (4, fenster4);
        pruefe (! drei,
                "NAK-283 M-55: headroom_unter_vier_rahmen_bleibt_ungesetzt (Regressionswache) - eine "
                "Passage mit drei Rahmen traegt keine Headroomverteilung",
                drei ? "gesetzt, Fenster " + juce::String ((int) fenster3) : juce::String ("kein Bit"));
        pruefe (vier && fenster4 == 4u,
                "M-55: Gegenprobe - mit vier Rahmen steht sie, ueber genau vier Rahmen",
                vier ? "Fenster " + juce::String ((int) fenster4) : juce::String ("kein Bit"));

        // (b) Speicherseite, gemessen: was `vorbereiten()` anlegt, und dass
        // eine lange Passage danach nichts mehr anlegt - weder die
        // Bandverteilungen (`kVerteilungPlaetze` bleibt die Ressourcengrenze,
        // der Riegel ist `git grep`) noch die Headroomverteilung der Passage.
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        allokationen = 0;
        allokierteBytes = 0;
        zaehleAllokationen = true;
        e.vorbereiten (fs);
        zaehleAllokationen = false;
        const auto vorbereitungBytes = allokierteBytes;
        const auto vorbereitungAllokationen = allokationen;

        Speiser s { e };
        s.frames = 480;
        const auto ton = sinus (0.5, 1000.0, fs);
        (void) s.sende (ton);                             // der Speiser legt seinen Puffer an
        pruefe (e.setzePassagenfenster (0, 60 * 48000, e.transportEpocheJetzt()),
                "M-55 (b): eine 60-s-Passage wird gebunden");
        allokationen = 0;
        zaehleAllokationen = true;
        std::uint32_t fensterAmEnde = 0;
        for (int i = 0; i < 20 * 100; ++i)                // 20 s, 200 Rahmen
            if (s.sende (ton))
                fensterAmEnde = e.frame().headroomFenster;
        zaehleAllokationen = false;
        pruefe (allokationen == 0,
                "M-55 (b): 20 s Passage legen keinen Speicher an - Band- und Headroomverteilung "
                "sind fest gedeckelt",
                juce::String ((int) allokationen) + " Allokationen, headroomFenster am Ende "
                    + juce::String ((int) fensterAmEnde) + "; vorbereiten() legte "
                    + juce::String ((juce::int64) vorbereitungBytes) + " Byte in "
                    + juce::String ((juce::int64) vorbereitungAllokationen) + " Allokationen an");
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-283 Etappe 5 · F10 (R-283-5, Feinheit 8) - Gueltigkeit je Zelle
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("NAK-283 M-56/M-57  kurzzeitlautheit_bleibt_ungesetzt_solange_ihr_fenster_kontaminiert_ist / "
               "momentan_und_kurzzeit_erholen_sich_mit_eigenen_fristen");
    {
        // Uebergabe §4 F10: drei Sekunden sauber, 0,2 s nicht endlich, danach
        // sauber. 480er-Bloecke: die Ersetzung endet bei 153 600 - einem
        // Zellen- und Rahmenrand -, und nach[k] ist der Rahmen, der k x 0,1 s
        // danach endet.
        const auto mitErsetzung = [] (std::uint64_t n)
        {
            if (n >= 144000u && n < 153600u)
                return std::numeric_limits<float>::quiet_NaN();
            return (float) (0.2 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
        };
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        s.frames = 480;
        std::vector<FeatureFrame> nach (32);
        bool takt = true;
        int ersetzteRahmen = 0;
        while (s.strom < 153600u + 31u * 4800u)
        {
            if (! s.sende (mitErsetzung))
                continue;
            if (e.frame().nichtEndlichRahmen > 0)
                ++ersetzteRahmen;
            if (s.strom <= 153600u)
                continue;
            const std::uint64_t d = s.strom - 153600u;
            if (d % 4800u != 0u || d / 4800u > 31u)
                takt = false;
            else
                nach[(std::size_t) (d / 4800u)] = e.frame();
        }
        const auto bits = [&nach] (int k)
        {
            return juce::String (juce::String (k / 10.0, 1)) + " s: M "
                 + (nach[(std::size_t) k].lufsMGesetzt ? "gesetzt" : "leer") + ", S "
                 + (nach[(std::size_t) k].lufsSGesetzt ? "gesetzt" : "leer");
        };
        pruefe (takt && ersetzteRahmen == 2,
                "M-56: die Ersetzung liegt in genau zwei Rahmen, und jeder Rahmen danach endet "
                "auf einem Zellenrand",
                juce::String (ersetzteRahmen) + " Rahmen mit ersetzten Samples");
        pruefe (nach[1].nichtEndlichRahmen == 0 && ! nach[1].lufsSGesetzt,
                "NAK-283 M-56: kurzzeitlautheit_bleibt_ungesetzt_solange_ihr_fenster_kontaminiert_ist - "
                "der erste saubere Rahmen traegt kein Short-term",
                bits (1));
        pruefe (nach[30].lufsSGesetzt,
                "NAK-283 M-56: kurzzeitlautheit_bleibt_ungesetzt_solange_ihr_fenster_kontaminiert_ist - "
                "nach genau 3 s sauberem Material ist es wieder gesetzt",
                bits (30));
        pruefe (nach[4].lufsMGesetzt && ! nach[4].lufsSGesetzt,
                "NAK-283 M-57: momentan_und_kurzzeit_erholen_sich_mit_eigenen_fristen - bei 0,4 s "
                "traegt derselbe Frame Momentary mit und Short-term ohne Praesenzbit",
                bits (4));
        pruefe (nach[29].lufsMGesetzt && ! nach[29].lufsSGesetzt,
                "NAK-283 M-57: momentan_und_kurzzeit_erholen_sich_mit_eigenen_fristen - bei 2,9 s "
                "gilt das immer noch",
                bits (29));
    }

    abschnitt ("NAK-283 M-59  nichtendlich_zaehler_saettigen_und_die_verriegelung_bleibt");
    {
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
        using nakama::analyse::FeatureEngineTestzugang;
        constexpr std::uint32_t kAnschlag = 0xFFFFFFFFu;
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        s.frames = 480;
        const auto ton = sinus (0.2, 1000.0, fs);
        for (int i = 0; i < 400; ++i)                      // 4 s sauber, Strom bei 192 000
            (void) s.sende (ton);
        // Zwanzig ersetzte Samples am Anfang jeder 4800er-Zelle - in einem
        // Rahmen aus zehn 480er-Bloecken trifft das genau den ersten Block.
        const std::function<float (std::uint64_t)> zwanzigErsetzt = [ton] (std::uint64_t n)
        {
            return (n % 4800u) < 20u ? std::numeric_limits<float>::quiet_NaN() : ton (n);
        };

        // Haelfte 1 - Regressionswache: knapp unter dem Anschlag kommen zwanzig
        // nicht-endliche Samples. Beide Zaehler bleiben bei 0xFFFFFFFF stehen.
        FeatureEngineTestzugang::nichtEndlichZaehlerSetzen (e, kAnschlag - 5u, kAnschlag - 5u);
        FeatureFrame h1 {};
        bool gab1 = false;
        for (int i = 0; i < 10; ++i)
            if (s.sende (i == 0 ? zwanzigErsetzt : ton)) { h1 = e.frame(); gab1 = true; }
        pruefe (gab1 && h1.nichtEndlichRahmen == kAnschlag && h1.nichtEndlichEvidenz == kAnschlag
                    && ! h1.truePeakGesetzt && ! h1.lufsMGesetzt,
                "NAK-283 M-59: nichtendlich_zaehler_saettigen_und_die_verriegelung_bleibt (Regressionswache "
                "fuer die Saettigung) - beide Zaehler bleiben bei 0xFFFFFFFF, der Rahmen bleibt verriegelt",
                juce::String::toHexString ((juce::int64) h1.nichtEndlichRahmen) + " / "
                    + juce::String::toHexString ((juce::int64) h1.nichtEndlichEvidenz));

        // Haelfte 2 - die Zellenmarke haengt NICHT am Zaehlerstand: beide Zaehler
        // stehen schon am Anschlag, die Ersetzung laesst sie nicht mehr steigen.
        FeatureEngineTestzugang::nichtEndlichZaehlerSetzen (e, kAnschlag, kAnschlag);
        std::vector<FeatureFrame> nach (32);
        bool takt = true;
        constexpr std::uint64_t kZellenEnde = 201600u;     // Ende der Zelle mit der Ersetzung
        for (int i = 0; i < 10 * 32; ++i)
        {
            if (! s.sende (i == 0 ? zwanzigErsetzt : ton))
                continue;
            if (s.strom < kZellenEnde || (s.strom - kZellenEnde) % 4800u != 0u
                || (s.strom - kZellenEnde) / 4800u > 31u)
                takt = false;
            else
                nach[(std::size_t) ((s.strom - kZellenEnde) / 4800u)] = e.frame();
        }
        pruefe (takt && nach[0].nichtEndlichRahmen == kAnschlag && ! nach[0].lufsMGesetzt,
                "M-59: der Rahmen mit der Ersetzung haelt den Zaehler am Anschlag und bleibt verriegelt",
                juce::String::toHexString ((juce::int64) nach[0].nichtEndlichRahmen));
        pruefe (! nach[1].lufsMGesetzt && ! nach[1].lufsSGesetzt && ! nach[3].lufsMGesetzt
                    && nach[4].lufsMGesetzt && ! nach[29].lufsSGesetzt && nach[30].lufsSGesetzt,
                "NAK-283 M-59: nichtendlich_zaehler_saettigen_und_die_verriegelung_bleibt - die Zellenmarke "
                "steht, obwohl der Zaehler nicht mehr gestiegen ist: Momentary kommt bei 0,4 s, "
                "Short-term bei 3,0 s wieder",
                juce::String ("0,1 s M ") + (nach[1].lufsMGesetzt ? "gesetzt" : "leer")
                    + " S " + (nach[1].lufsSGesetzt ? "gesetzt" : "leer")
                    + "; 0,3 s M " + (nach[3].lufsMGesetzt ? "gesetzt" : "leer")
                    + "; 0,4 s M " + (nach[4].lufsMGesetzt ? "gesetzt" : "leer")
                    + "; 2,9 s S " + (nach[29].lufsSGesetzt ? "gesetzt" : "leer")
                    + "; 3,0 s S " + (nach[30].lufsSGesetzt ? "gesetzt" : "leer"));
#else
        pruefe (false,
                "M-59: der Testzugang NAKAMA_FEATUREENGINE_TESTZUGANG fehlt - die Zaehler sind ohne "
                "ihn nicht an den Anschlag zu bringen");
#endif
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-283 Etappe 5 · Riegel M-60 - jede Historie traegt ihre eigene Spanne
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("NAK-283 M-60  jede_analysehistorie_traegt_ihre_eigene_spanne");
    {
        // EIN Lauf, drei Pruefungen: dieselbe 14-s-Passage (2 s bis 16 s,
        // vorab gebunden) mit einer lauten Kante direkt vor dem Anfang (fs/4
        // mit 45 Grad, Amplitude 0,9: -0,92 dBTP), einem Pegelsprung in der
        // Mitte (0,5 auf 0,0005) und einer Ersetzung im letzten Drittel
        // (10,8 bis 11,0 s der Passage, jedes zweite Sample). Die Ersetzung
        // laesst jedem Rahmen endliche Samples, damit jeder der 140 Rahmen
        // einen True Peak traegt. Seit M-76 gehen die zwei Rahmen mit der
        // Ersetzung trotzdem nicht in die Headroomverteilung ein: sie traegt
        // 138 Rahmen, und beide Haelften bleiben darin.
        constexpr std::int64_t start = 96000, ende = start + 672000, mitte = start + 336000;
        constexpr std::uint64_t ersetzungVon = 614400u, ersetzungBis = 624000u;
        const auto signal = [] (std::uint64_t nu) -> float
        {
            const auto n = (std::int64_t) nu;
            const double ton = std::sin (kZweiPi * 1000.0 * (double) n / 48000.0);
            if (n >= start - 12 && n < start)
                return (float) (0.9 * std::sin (kZweiPi * 0.25 * (double) (n - (start - 12))
                                                + kZweiPi * 0.125));
            if (n < start) return (float) (0.5 * ton);
            if (nu >= ersetzungVon && nu < ersetzungBis && (nu % 2u) == 1u)
                return std::numeric_limits<float>::quiet_NaN();
            if (n < mitte) return (float) (0.5 * ton);
            if (n < ende)  return (float) (0.0005 * ton);
            return 0.0f;
        };
        auto halter = std::make_unique<FeatureEngine>();
        auto& e = *halter;
        e.vorbereiten (fs);
        Speiser s { e };
        s.frames = 480;
        const bool gebunden = e.setzePassagenfenster (start, ende, e.transportEpocheJetzt());
        FeatureFrame amEnde {}, bei03 {}, bei04 {}, bei29 {}, bei30 {};
        while (s.strom < (std::uint64_t) ende + 48000u)
        {
            if (! s.sende (signal))
                continue;
            const auto& f = e.frame();
            if (s.strom == ersetzungBis + 14400u)  bei03 = f;
            if (s.strom == ersetzungBis + 19200u)  bei04 = f;
            if (s.strom == ersetzungBis + 139200u) bei29 = f;
            if (s.strom == ersetzungBis + 144000u) bei30 = f;
            if (s.strom == (std::uint64_t) ende)   amEnde = f;
        }
        pruefe (gebunden && ersetzungBis + 144000u == (std::uint64_t) ende,
                "M-60: die Passage ist vorab gebunden, und 3,0 s nach der Ersetzung endet sie");
        pruefe (amEnde.truePeakPassageGesetzt && std::abs (amEnde.truePeakPassageDb + 6.0206f) <= 0.1f,
                "NAK-283 M-60: jede_analysehistorie_traegt_ihre_eigene_spanne - True-Peak-Kette: das "
                "Passagenmaximum ist das der Passage, nicht das der Kante davor",
                juce::String (amEnde.truePeakPassageDb, 3) + " dBTP (Kante -0,92 dBTP)");
        pruefe (amEnde.headroomGesetzt && amEnde.headroomFenster == 138u
                    && std::abs (amEnde.headroomP95Db + 6.0206f) <= 0.1f
                    && std::abs (amEnde.headroomP10Db + 66.0206f) <= 0.1f,
                "NAK-283 M-60: jede_analysehistorie_traegt_ihre_eigene_spanne - Headroom: die Verteilung "
                "beschreibt beide Haelften der ganzen Passage (138 Rahmen: die zwei mit ersetzten "
                "Samples fehlen, M-76)",
                "P10 " + juce::String (amEnde.headroomP10Db, 3) + ", P95 "
                    + juce::String (amEnde.headroomP95Db, 3) + " dBTP, headroomFenster "
                    + juce::String ((int) amEnde.headroomFenster));
        pruefe (! bei03.lufsMGesetzt && bei04.lufsMGesetzt && ! bei29.lufsSGesetzt && bei30.lufsSGesetzt,
                "NAK-283 M-60: jede_analysehistorie_traegt_ihre_eigene_spanne - Loudnesszellen: "
                "Momentary kommt 0,4 s, Short-term 3,0 s nach der Ersetzung wieder",
                juce::String ("0,3 s M ") + (bei03.lufsMGesetzt ? "gesetzt" : "leer")
                    + "; 0,4 s M " + (bei04.lufsMGesetzt ? "gesetzt" : "leer")
                    + "; 2,9 s S " + (bei29.lufsSGesetzt ? "gesetzt" : "leer")
                    + "; 3,0 s S " + (bei30.lufsSGesetzt ? "gesetzt" : "leer"));
    }

    // ═══════════════════════════════════════════════════════════════════
    // NAK-283 Etappe 5 · Nacharbeit 1 · M-76 (R-283-5, Regel zu §39.9 Punkt 1 (b))
    // ═══════════════════════════════════════════════════════════════════
    abschnitt ("NAK-283 M-76  rahmen_mit_ersetzten_samples_geht_nicht_in_die_passagenverteilung_ein");
    {
        // Buehne wie M-53: 14 s gebunden, 1-kHz-Ton, erste Haelfte Amplitude
        // 0,5, zweite 0,0005; 480er-Bloecke, also 140 Rahmen zu je 4 800
        // Samples. Der Rahmen 6,9 bis 7,0 s - der letzte der lauten Haelfte -
        // traegt 50 Perioden Ton mit Amplitude 0,9 (-0,92 dBTP, lauter als
        // jeder andere Rahmen), dann 49 Perioden NaN, die der Riegel durch 0
        // ersetzt und zaehlt, dann eine Periode des regulaeren Tons seiner
        // Haelfte. Jeder Wechsel liegt auf einem Nulldurchgang (Vielfaches von
        // 48 Samples).
        //
        // Warum die letzte Periode regulaer ist: der Interpolator gibt ein
        // Sample zwoelf Ticks spaeter aus und liest dafuer 25 Samples zurueck
        // (`TruePeak.h`). Stuende der laute Ton bis zum Rahmenende, truege der
        // naechste, saubere Rahmen dessen Nachklang ins Passagenmaximum
        // (§39.9 Punkt 1 (d), NAK-295) - eine Stelle, die M-76 nicht zusagt.
        // So rechnet der Interpolator in jedem anderen Rahmen ueber dieselben
        // Samples wie im Gegenlauf.
        //
        // Drei Laeufe auf je einer frischen Engine: die Buehne; der Gegenlauf
        // der Zeile (derselbe Rahmen mit dem regulaeren Ton seiner Haelfte);
        // und die Gegenprobe - die Buehne mit echten Nullen statt NaN. Nach dem
        // Riegel rechnen Buehne und Gegenprobe ueber dieselben Samples; nur die
        // Buehne zaehlt und markiert sie als ersetzt.
        constexpr std::int64_t ende = 14 * 48000, haelfte = ende / 2;
        constexpr std::int64_t rahmenVon = haelfte - 4800;
        enum class Art { buehne, gegenlauf, gegenprobe };
        struct Ergebnis
        {
            int rahmen { 0 }, ersetzteRahmen { 0 };
            std::int64_t ersetztBis { -1 };
            bool intakt { false }, headroomGesetzt { false }, maximumGesetzt { false };
            float p10 { 0.0f }, p50 { 0.0f }, p95 { 0.0f }, maximumDb { 0.0f };
            std::uint32_t fenster { 0 };
            double groessteKlasseDb { std::numeric_limits<double>::quiet_NaN() };
        };
        const auto lauf = [] (Art art)
        {
            const auto signal = [art] (std::uint64_t nu) -> float
            {
                const auto n = (std::int64_t) nu;
                const double ton = std::sin (kZweiPi * 1000.0 * (double) n / 48000.0);
                if (art != Art::gegenlauf && n >= rahmenVon && n < rahmenVon + 4752)
                {
                    if (n < rahmenVon + 2400)
                        return (float) (0.9 * ton);
                    return art == Art::buehne ? std::numeric_limits<float>::quiet_NaN() : 0.0f;
                }
                return (float) ((n < haelfte ? 0.5 : 0.0005) * ton);
            };
            auto halter = std::make_unique<FeatureEngine>();
            auto& e = *halter;
            e.vorbereiten (48000.0);
            Speiser s { e };
            s.frames = 480;
            const bool gebunden = e.setzePassagenfenster (0, ende, e.transportEpocheJetzt());
            Ergebnis r;
            while (s.projekt < ende)
            {
                if (! s.sende (signal))
                    continue;
                ++r.rahmen;
                if (e.frame().nichtEndlichRahmen > 0)
                {
                    ++r.ersetzteRahmen;
                    r.ersetztBis = s.projekt;
                }
            }
            const auto& f = e.frame();
            r.intakt = gebunden && e.passagenfensterIntakt();
            r.headroomGesetzt = f.headroomGesetzt;
            r.p10 = f.headroomP10Db;
            r.p50 = f.headroomP50Db;
            r.p95 = f.headroomP95Db;
            r.fenster = f.headroomFenster;
            r.maximumGesetzt = f.truePeakPassageGesetzt;
            r.maximumDb = f.truePeakPassageDb;
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
            r.groessteKlasseDb = nakama::analyse::FeatureEngineTestzugang::headroomGroessteKlasseDb (e);
#endif
            return r;
        };
        const auto buehne = lauf (Art::buehne);
        const auto gegen  = lauf (Art::gegenlauf);
        const auto probe  = lauf (Art::gegenprobe);
        const double sollDb = 20.0 * std::log10 (0.5);     // -6,0206 dBTP, der laute Ton
        const double lautDb = 20.0 * std::log10 (0.9);     // -0,9151 dBTP, der Rahmen 6,9 bis 7,0 s
        const auto text = [] (const Ergebnis& r)
        {
            return "headroomFenster " + juce::String ((int) r.fenster)
                 + ", Passagenmaximum " + juce::String (r.maximumDb, 3)
                 + " dBTP, P10 " + juce::String (r.p10, 3) + ", P50 " + juce::String (r.p50, 3)
                 + ", P95 " + juce::String (r.p95, 3) + " dBTP, groesste Klasse "
                 + juce::String (r.groessteKlasseDb, 3) + " dBTP"
                 + (r.headroomGesetzt ? "" : ", KEIN Headroombit")
                 + (r.maximumGesetzt ? "" : ", KEIN Maximumbit");
        };

        pruefe (buehne.rahmen == 140 && gegen.rahmen == 140 && probe.rahmen == 140
                    && buehne.ersetzteRahmen == 1 && buehne.ersetztBis == haelfte
                    && gegen.ersetzteRahmen == 0 && probe.ersetzteRahmen == 0,
                "M-76: je 140 Rahmen; in der Buehne traegt genau EIN Rahmen ersetzte Samples - der "
                "Rahmen 6,9 bis 7,0 s, ganz im Passagenfenster -, Gegenlauf und Gegenprobe keinen",
                juce::String (buehne.rahmen) + " / " + juce::String (gegen.rahmen) + " / "
                    + juce::String (probe.rahmen) + " Rahmen; mit ersetzten Samples: "
                    + juce::String (buehne.ersetzteRahmen) + " (endet bei Sample "
                    + juce::String ((juce::int64) buehne.ersetztBis) + ") / "
                    + juce::String (gegen.ersetzteRahmen) + " / " + juce::String (probe.ersetzteRahmen));
        pruefe (probe.headroomGesetzt && probe.fenster == 140u && probe.maximumGesetzt
                    && std::abs ((double) probe.maximumDb - lautDb) <= 0.1,
                "M-76: Gegenprobe - derselbe Rahmen mit echten Nullen statt NaN geht ein: "
                "headroomFenster 140, Passagenmaximum rund -0,92 dBTP - sein Beitrag ist erkennbar",
                text (probe));
        pruefe (buehne.headroomGesetzt && buehne.fenster == 139u
                    && gegen.headroomGesetzt && gegen.fenster == 140u,
                "NAK-283 M-76: rahmen_mit_ersetzten_samples_geht_nicht_in_die_passagenverteilung_ein - "
                "headroomFenster zaehlt den Rahmen nicht (139, der Gegenlauf 140)",
                text (buehne) + " | Gegenlauf: " + text (gegen));
        pruefe (buehne.maximumGesetzt && std::abs ((double) buehne.maximumDb - sollDb) <= 0.1,
                "NAK-283 M-76: rahmen_mit_ersetzten_samples_geht_nicht_in_die_passagenverteilung_ein - "
                "das Passagenmaximum bleibt beim gemessenen Material (rund -6,02 dBTP, nicht -0,92)",
                text (buehne));
        pruefe (buehne.headroomGesetzt && gegen.headroomGesetzt
                    && std::abs (buehne.p10 - gegen.p10) < 0.01f
                    && std::abs (buehne.p50 - gegen.p50) < 0.01f
                    && std::abs (buehne.p95 - gegen.p95) < 0.01f,
                "NAK-283 M-76: rahmen_mit_ersetzten_samples_geht_nicht_in_die_passagenverteilung_ein - "
                "P10, P50 und P95 gleichen dem Gegenlauf innerhalb der 0,01-dB-Rasterung",
                text (buehne) + " | Gegenlauf: " + text (gegen));
#if defined (NAKAMA_FEATUREENGINE_TESTZUGANG)
        pruefe (std::isfinite (buehne.groessteKlasseDb) && std::isfinite (gegen.groessteKlasseDb)
                    && std::abs (buehne.groessteKlasseDb - gegen.groessteKlasseDb) < 0.005
                    && std::abs (probe.groessteKlasseDb - lautDb) <= 0.1,
                "NAK-283 M-76: rahmen_mit_ersetzten_samples_geht_nicht_in_die_passagenverteilung_ein - "
                "die Verteilung traegt keinen Wert aus dem teilweise stillgelegten Rahmen: ihre groesste "
                "Klasse ist die des Gegenlaufs (in der Gegenprobe die des lauten Rahmens)",
                "Buehne " + juce::String (buehne.groessteKlasseDb, 3) + ", Gegenlauf "
                    + juce::String (gegen.groessteKlasseDb, 3) + ", Gegenprobe "
                    + juce::String (probe.groessteKlasseDb, 3) + " dBTP");
#else
        pruefe (false,
                "M-76: der Testzugang NAKAMA_FEATUREENGINE_TESTZUGANG fehlt - die groesste Klasse der "
                "Verteilung ist ohne ihn nicht lesbar");
#endif
        pruefe (buehne.intakt && buehne.headroomGesetzt && buehne.maximumGesetzt,
                "NAK-283 M-76: rahmen_mit_ersetzten_samples_geht_nicht_in_die_passagenverteilung_ein - "
                "die Passage selbst bleibt gueltig: Fenster intakt, Verteilung und Passagenmaximum gesetzt",
                text (buehne));
    }

    std::cout << "\n-----------------------------------------" << std::endl;
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
