#pragma once

#include "../core/analysis/Fft.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace nakama::test::nak380
{

inline constexpr std::uint64_t kW0SaatLinks  = 0x03800001ull;
inline constexpr std::uint64_t kW0SaatRechts = 0x03800002ull;
inline constexpr std::uint64_t kW4Saat       = 0x03800006ull;
inline constexpr std::uint64_t kM1RauschSaat = 0x03800009ull;
inline constexpr std::uint64_t kM1PegelSaat  = 0x0380000Aull;
inline constexpr std::uint64_t kM2RauschSaat = 0x0380000Bull;
inline constexpr std::uint64_t kM2PegelSaat  = 0x0380000Cull;
inline constexpr std::uint64_t kM3RauschSaat = 0x0380000Dull;
inline constexpr std::uint64_t kM3PegelSaat  = 0x0380000Eull;
inline constexpr std::uint64_t kM36aSaat     = 0x03800011ull;
inline constexpr std::uint64_t kM36bSaat     = 0x03800012ull;
inline constexpr std::uint64_t kM36cSaat     = 0x03800013ull;
inline constexpr double kSamplerate = 48000.0;
inline constexpr double kZweiPi = 6.283185307179586476925286766559;

class GleichRauschen
{
public:
    explicit GleichRauschen (std::uint64_t saat) noexcept : zustand (saat) {}

    std::uint64_t bits() noexcept
    {
        zustand += 0x9e3779b97f4a7c15ull;
        auto z = zustand;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
        return z ^ (z >> 31);
    }

    double offen01() noexcept
    {
        return ((double) (bits() >> 11) + 0.5) * (1.0 / 9007199254740992.0);
    }

private:
    std::uint64_t zustand {};
};

/** Deterministisches Gaußrauschen fuer W0/W0h aus §7.2.

    SplitMix64 erzeugt die Gleichverteilung; Box-Muller liefert zwei
    aufeinanderfolgende standardnormalverteilte Werte. Die Saat geht vor der
    ersten Durchmischung in den Zustand ein. */
class GaussRauschen
{
public:
    explicit GaussRauschen (std::uint64_t saat) noexcept : gleich (saat) {}

    double naechstes() noexcept
    {
        if (reserveGueltig)
        {
            reserveGueltig = false;
            return reserve;
        }

        // Offenes Intervall (0, 1): kein log(0), kein exakt gleicher Winkel.
        const double u1 = gleich.offen01();
        const double u2 = gleich.offen01();
        const double radius = std::sqrt (-2.0 * std::log (u1));
        const double winkel = 6.283185307179586476925286766559 * u2;
        reserve = radius * std::sin (winkel);
        reserveGueltig = true;
        return radius * std::cos (winkel);
    }

private:
    GleichRauschen gleich;
    double reserve {};
    bool reserveGueltig { false };
};

struct W0
{
    GaussRauschen links  { kW0SaatLinks };
    GaussRauschen rechts { kW0SaatRechts };
    double sigma { 0.1 };

    void naechstes (float& l, float& r) noexcept
    {
        l = (float) (sigma * links.naechstes());
        r = (float) (sigma * rechts.naechstes());
    }
};

inline float s3 (std::uint64_t sample, double amplitude,
                 double samplerate = kSamplerate) noexcept
{
    return (float) (amplitude * std::sin (kZweiPi * 1030.0
                                          * (double) sample / samplerate));
}

inline float sinus1k (std::uint64_t sample, double amplitude,
                      double samplerate = kSamplerate) noexcept
{
    return (float) (amplitude * std::sin (kZweiPi * 1000.0
                                          * (double) sample / samplerate));
}

inline double l2Amplitude (std::uint64_t sample,
                           double samplerate = kSamplerate) noexcept
{
    const auto sekunde = (std::uint64_t) ((double) sample / samplerate);
    return (sekunde & 1u) == 0u ? 0.1 : 0.01;
}

inline double tabelle1Amplitude (int fall, double sekunden) noexcept
{
    const int abschnitt = (int) std::floor (sekunden / 20.0);
    double db = -20.0;
    if (fall == 1) db = (abschnitt % 2 == 0) ? -20.0 : -30.0;
    if (fall == 2) db = (abschnitt % 2 == 0) ? -20.0 : -15.0;
    if (fall == 3) db = (abschnitt % 2 == 0) ? -40.0 : -20.0;
    if (fall == 4)
    {
        constexpr double pegel[] = { -50.0, -35.0, -20.0, -35.0, -50.0 };
        db = pegel[abschnitt < 0 ? 0 : (abschnitt > 4 ? 4 : abschnitt)];
    }
    return std::pow (10.0, db / 20.0);
}

/** Unabhaengige Tech-3342-§5-Referenz ueber eine analytische 10-Hz-Folge.
    `zellenEnergie` traegt die mittlere Energie je 100-ms-Zelle. Ein gemeinsamer
    K-Gewichtungsfaktor kuerzt sich aus der LRA heraus. */
inline double lraReferenz (const std::vector<double>& zellenEnergie)
{
    std::vector<double> kurz;
    for (std::size_t ende = 30; ende <= zellenEnergie.size(); ++ende)
    {
        double summe = 0.0;
        for (std::size_t i = ende - 30; i < ende; ++i)
            summe += zellenEnergie[i];
        const double lufs = -0.691 + 10.0 * std::log10 (summe / 30.0 + 1.0e-300);
        if (lufs >= -70.0)
            kurz.push_back (lufs);
    }
    if (kurz.empty())
        return 0.0;

    double energie = 0.0;
    for (const double lufs : kurz)
        energie += std::pow (10.0, lufs / 10.0);
    const double relativ = 10.0 * std::log10 (energie / (double) kurz.size()) - 20.0;

    std::vector<double> gegatet;
    for (const double lufs : kurz)
        if (lufs >= relativ)
            gegatet.push_back (lufs);
    std::sort (gegatet.begin(), gegatet.end());
    const auto wert = [&] (double p)
    {
        // EBU Tech 3342 §5: Index round((n-1)*p+1), MATLAB-Index ab 1.
        const auto index1 = (std::size_t) std::llround (
            ((double) gegatet.size() - 1.0) * p + 1.0);
        return gegatet[index1 - 1u];
    };
    return wert (0.95) - wert (0.10);
}

inline std::vector<double> tabelle1Zellenergie (int fall)
{
    const int zellen = fall == 4 ? 1000 : 800;
    std::vector<double> aus;
    aus.reserve ((std::size_t) zellen);
    for (int z = 0; z < zellen; ++z)
    {
        const double a = tabelle1Amplitude (fall, 0.1 * (double) z);
        aus.push_back (0.5 * a * a);
    }
    return aus;
}

/** Analytische Energie je 100-ms-Zelle des L2-Signals (§7 L2, R-380-11).
    Eine Zelle umfasst 4 800 Samples bei 48 kHz, also genau 100 Perioden des
    1-kHz-Sinus, und liegt ganz in einer Sekunde; ihre mittlere Energie ist
    daher 0,5·a² mit a = `l2Amplitude` an ihrem ersten Sample (0,1 in
    geraden, 0,01 in ungeraden Sekunden, laut ab Sample 0). Dieselbe Folge
    speist der Produktlauf ueber `sinus1k (n, l2Amplitude (n))`. */
inline std::vector<double> l2Zellenergie (int zellen)
{
    constexpr std::uint64_t zellenSamples = 4800u;
    std::vector<double> aus;
    aus.reserve ((std::size_t) zellen);
    for (int z = 0; z < zellen; ++z)
    {
        const double a = l2Amplitude ((std::uint64_t) z * zellenSamples);
        aus.push_back (0.5 * a * a);
    }
    return aus;
}

/** NAK-380 M-36 (Fassung §31): Modulationseingang nach dem Muster der
    M-33 bis M-35, aber mit zwei festen Pegelstufen statt gleichverteilter,
    damit die gelesene Spanne P95 - P50 eine hergeleitete ganze Zahl ist.

    Signal L = R: stetiger Sinus 1500 Hz, Amplitude 0,1, ueber Gauss-
    Weissrauschen sigma = 0,01; beide zusammen je 10 s zuerst 7 s im
    Grundpegel, dann 3 s um `tiefeDb` lauter (laut ab 7 s), 60 s bei 48 kHz.
    1500 Hz = 48 000/32: die Periode ist genau 32 Samples, der Ton sitzt auf
    Bin 128 der 4096er-Mittenstufe (128 Perioden je Segment) und damit mit
    Haupt- und beiden Nachbarbins (127 bis 129) ganz in Band 135
    (30·2^(135/24) = 1480,5 Hz bis 30·2^(136/24) = 1523,9 Hz, Bins 127 bis
    130). */
struct M36Pegelrechteck
{
    M36Pegelrechteck (double tiefeDb, std::uint64_t saat) noexcept
        : faktorLaut (std::pow (10.0, tiefeDb / 20.0)), rauschen (saat) {}

    float naechstes (std::uint64_t sample) noexcept
    {
        const bool laut = (sample % 480000u) >= 336000u;
        const double ton = 0.1 * std::sin (kZweiPi * (double) (sample % 32u) / 32.0);
        return (float) ((laut ? faktorLaut : 1.0) * (ton + 0.01 * rauschen.naechstes()));
    }

    double faktorLaut;
    GaussRauschen rauschen;
};

//==============================================================================
// NAK-380 Etappe 4 (§7.2): Null-, Impuls- und Referenzkorpus des Detektors.
// Jedes Signal ist eine reine Funktion von Sampleindex, Parametern und Saat;
// alle Signale sind L = R und werden als float32 gespeichert, weil der
// Engine-Eingang float ist.

inline constexpr std::uint64_t kM47Saat = 0x03800001ull;
inline constexpr std::uint64_t kW1Saat  = 0x03800003ull;
inline constexpr std::uint64_t kW2Saat  = 0x03800004ull;
inline constexpr std::uint64_t kW3Saat  = 0x03800005ull;
inline constexpr std::uint64_t kP1Saat  = 0x03800007ull;
inline constexpr std::uint64_t kP2Saat  = 0x03800008ull;

/** W1 bis W3 (und M-47 (a)): Gauss-Weissrauschen aus SplitMix64 und
    Box-Muller mit Standardabweichung `sigma`, `samples` Werte. */
inline std::vector<float> weissMono (std::uint64_t saat, double sigma, std::uint64_t samples)
{
    GaussRauschen g { saat };
    std::vector<float> aus ((std::size_t) samples);
    for (auto& v : aus)
        v = (float) (sigma * g.naechstes());
    return aus;
}

/** P1/P2: rosa Rauschen nach Paul Kellet (sieben Pole) auf Gauss-Weiss.

    Normierung ANALYTISCH statt am Lauf: `leistungsverstaerkung()` ist die
    Summe der quadrierten Impulsantwort des Filters (fuer Weiss mit Varianz 1
    die Ausgangsvarianz, 9,318 bei diesen Koeffizienten; die Summe laeuft, bis
    der langsamste Pol 0,99886 auf unter 1e-40 abgeklungen ist). Damit haengt
    der Pegel weder an der Saat noch an der Laenge. */
class RosaRauschen
{
public:
    RosaRauschen (std::uint64_t saat, double rms) noexcept
        : weiss (saat), skala (rms / std::sqrt (leistungsverstaerkung())) {}

    double naechstes() noexcept
    {
        const double w = weiss.naechstes();
        return skala * filter (w);
    }

    static double leistungsverstaerkung() noexcept
    {
        Zustand z;
        double summe = 0.0;
        for (int n = 0; n < 100000; ++n)
        {
            const double h = z.schritt (n == 0 ? 1.0 : 0.0);
            summe += h * h;
        }
        return summe;
    }

private:
    struct Zustand
    {
        double b0 {}, b1 {}, b2 {}, b3 {}, b4 {}, b5 {}, b6 {};
        double schritt (double w) noexcept
        {
            b0 = 0.99886 * b0 + w * 0.0555179;
            b1 = 0.99332 * b1 + w * 0.0750759;
            b2 = 0.96900 * b2 + w * 0.1538520;
            b3 = 0.86650 * b3 + w * 0.3104856;
            b4 = 0.55000 * b4 + w * 0.5329522;
            b5 = -0.7616 * b5 - w * 0.0168980;
            const double aus = b0 + b1 + b2 + b3 + b4 + b5 + b6 + w * 0.5362;
            b6 = w * 0.115926;
            return aus;
        }
    };

    double filter (double w) noexcept { return zustand.schritt (w); }

    GaussRauschen weiss;
    Zustand zustand;
    double skala;
};

inline std::vector<float> rosaMono (std::uint64_t saat, double rms, std::uint64_t samples)
{
    RosaRauschen r { saat, rms };
    std::vector<float> aus ((std::size_t) samples);
    for (auto& v : aus)
        v = (float) r.naechstes();
    return aus;
}

/** E-380-13: Selbstpruefung eines Rosa-Signals an der INTEGRIERTEN Groesse.
    Fuer S(f) = C/f ist die Oktavbandleistung P[f/sqrt2, f*sqrt2] = C*ln 2
    konstant; geprueft werden die zehn Oktaven um 1000*2^(i-5) Hz (31,25 Hz
    bis 16 kHz), geschaetzt nach Welch (Hann, 65 536 Punkte, 50 %). Rueckgabe:
    die groesste Abweichung benachbarter Oktaven in dB; das Soll ist <= 1,0 dB.
    Nur die Leistungsdichte faellt um 3,0103 dB je Oktave, die Bandleistung
    nicht. */
inline double rosaGroessteOktavdifferenzDb (const std::vector<float>& x, double fs,
                                            std::vector<double>* oktavenDb = nullptr)
{
    constexpr int n = 65536;
    nakama::analyse::Fft fft;
    fft.vorbereiten (n);
    std::vector<double> fenster ((std::size_t) n), arbeit ((std::size_t) n);
    for (int i = 0; i < n; ++i)
        fenster[(std::size_t) i] = 0.5 - 0.5 * std::cos (kZweiPi * (double) i / (double) n);
    std::vector<double> psd ((std::size_t) (n / 2 + 1), 0.0);
    int segmente = 0;
    for (std::size_t start = 0; start + (std::size_t) n <= x.size(); start += (std::size_t) (n / 2))
    {
        for (int i = 0; i < n; ++i)
            arbeit[(std::size_t) i] = (double) x[start + (std::size_t) i] * fenster[(std::size_t) i];
        fft.transformiere (arbeit.data());
        for (int k = 0; k <= n / 2; ++k)
            psd[(std::size_t) k] += fft.leistung (k);
        ++segmente;
    }
    if (segmente == 0)
        return std::numeric_limits<double>::infinity();
    const double df = fs / (double) n;
    std::vector<double> db;
    for (int i = 0; i < 10; ++i)
    {
        const double mitte = 1000.0 * std::pow (2.0, (double) (i - 5));
        const int von = (int) std::ceil (mitte / std::sqrt (2.0) / df);
        const int bis = (int) std::ceil (mitte * std::sqrt (2.0) / df);
        double summe = 0.0;
        for (int k = von; k < bis && k <= n / 2; ++k)
            summe += psd[(std::size_t) k];
        db.push_back (10.0 * std::log10 (summe / (double) segmente + 1.0e-300));
    }
    double groesste = 0.0;
    for (std::size_t i = 1; i < db.size(); ++i)
        groesste = std::max (groesste, std::abs (db[i] - db[i - 1]));
    if (oktavenDb != nullptr)
        *oktavenDb = db;
    return groesste;
}

/** S1: Sinus 440 Hz, Amplitude 0,5, in double gerechnet, float32 gespeichert. */
inline std::vector<float> s1Sinus (std::uint64_t samples, double fs = kSamplerate)
{
    std::vector<float> aus ((std::size_t) samples);
    for (std::uint64_t n = 0; n < samples; ++n)
        aus[(std::size_t) n] = (float) (0.5 * std::sin (kZweiPi * 440.0 * (double) n / fs));
    return aus;
}

/** S2: Saegezahn 110 Hz aus 20 Obertoenen mit Amplituden 1/h, die Spitze der
    Periode (auf 200 001 Phasenpunkten ausgewertet) auf 0,5 normiert. */
inline std::vector<float> s2Saegezahn (std::uint64_t samples, double fs = kSamplerate)
{
    const auto reihe = [] (double phase)
    {
        double s = 0.0;
        for (int h = 1; h <= 20; ++h)
            s += std::sin (kZweiPi * (double) h * phase) / (double) h;
        return s;
    };
    double spitze = 0.0;
    for (int i = 0; i <= 200000; ++i)
        spitze = std::max (spitze, std::abs (reihe ((double) i / 200000.0)));
    std::vector<float> aus ((std::size_t) samples);
    for (std::uint64_t n = 0; n < samples; ++n)
        aus[(std::size_t) n] = (float) (0.5 * reihe (110.0 * (double) n / fs) / spitze);
    return aus;
}

/** V1: 440 Hz mit sechs Obertoenen (h = 1 bis 7, Amplituden 1/h), Vibrato
    f(t) = 440 * 2^((50/1200) * sin(2*pi*5,5 Hz*t)). Die Phase ist das
    Integral der Momentanfrequenz, als laufende Summe ueber die Samples
    (phi[n] = 2*pi/fs * Summe f[0..n]). Die Teiltoene bewegen sich um
    hoechstens +-50 Cent, je Hop der Hauptstufe (42,67 ms) aber um bis zu
    2*pi*5,5*50*0,04267 = 73,7 Cent. */
inline std::vector<float> v1Vibrato (std::uint64_t samples, double fs = kSamplerate)
{
    std::vector<float> aus ((std::size_t) samples);
    double phase = 0.0;
    for (std::uint64_t n = 0; n < samples; ++n)
    {
        const double t = (double) n / fs;
        const double f = 440.0 * std::pow (2.0, (50.0 / 1200.0) * std::sin (kZweiPi * 5.5 * t));
        phase += kZweiPi * f / fs;
        double s = 0.0;
        for (int h = 1; h <= 7; ++h)
            s += std::sin ((double) h * phase) / (double) h;
        aus[(std::size_t) n] = (float) s;
    }
    return aus;
}

/** I1: Klicks (ein Sample, +0,5) bei t = 2,0 s + 0,25 s * i, i = 0 bis 111. */
inline std::vector<std::uint64_t> i1Klicks()
{
    std::vector<std::uint64_t> aus;
    for (std::uint64_t i = 0; i < 112u; ++i)
        aus.push_back (96000u + 12000u * i);
    return aus;
}

/** I3-Paarabstand bei 48 kHz (NAK-380 R-380-12 (ii)): 150 ms = 7200
    Samples, ueber der Vorframe-Verdeckung der Hauptstufe - ein zweiter Klick
    trifft erst ab N_H + Hop = 4096 + 2048 = 6144 Samples (128,0 ms bei
    48 kHz, 139,3 ms bei 44,1 kHz) auf einen Vorframe ohne den ersten.
    100 ms = 4800 Samples ist die einmalige Beobachtung darunter, ohne
    Zusage. I2 (20 ms = 960 Samples) liegt innerhalb der Sperrzeit. */
inline constexpr std::uint64_t kI3AbstandSamples = 7200u;
inline constexpr std::uint64_t kI3BeobachtungAbstandSamples = 4800u;
static_assert (kI3AbstandSamples >= 4096u + 2048u, "I3 muss ueber N_H + Hop liegen");

/** I2/I3: Klickpaare bei t_j = 2,0 s + 0,5 s * j und t_j + `abstandSamples`,
    j = 0 bis 55; Reihenfolge je Paar erst, dann zweiter Klick. Derselbe
    Erzeuger liefert I2, I3 und die 100-ms-Beobachtung. */
inline std::vector<std::uint64_t> klickPaare (std::uint64_t abstandSamples)
{
    std::vector<std::uint64_t> aus;
    for (std::uint64_t j = 0; j < 56u; ++j)
    {
        aus.push_back (96000u + 24000u * j);
        aus.push_back (96000u + 24000u * j + abstandSamples);
    }
    return aus;
}

inline void klicksEinsetzen (std::vector<float>& x, const std::vector<std::uint64_t>& klicks,
                             double amplitude = 0.5)
{
    for (const auto k : klicks)
        if (k < x.size())
            x[(std::size_t) k] = (float) ((double) x[(std::size_t) k] + amplitude);
}

} // namespace nakama::test::nak380
