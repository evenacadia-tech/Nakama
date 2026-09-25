#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
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

} // namespace nakama::test::nak380
