#pragma once

#include <cmath>
#include <cstdint>

namespace nakama::test::nak380
{

inline constexpr std::uint64_t kW0SaatLinks  = 0x03800001ull;
inline constexpr std::uint64_t kW0SaatRechts = 0x03800002ull;

/** Deterministisches Gaußrauschen fuer W0/W0h aus §7.2.

    SplitMix64 erzeugt die Gleichverteilung; Box-Muller liefert zwei
    aufeinanderfolgende standardnormalverteilte Werte. Die Saat geht vor der
    ersten Durchmischung in den Zustand ein. */
class GaussRauschen
{
public:
    explicit GaussRauschen (std::uint64_t saat) noexcept : zustand (saat) {}

    double naechstes() noexcept
    {
        if (reserveGueltig)
        {
            reserveGueltig = false;
            return reserve;
        }

        // Offenes Intervall (0, 1): kein log(0), kein exakt gleicher Winkel.
        const double u1 = ((double) (splitmix64() >> 11) + 0.5)
                        * (1.0 / 9007199254740992.0);
        const double u2 = ((double) (splitmix64() >> 11) + 0.5)
                        * (1.0 / 9007199254740992.0);
        const double radius = std::sqrt (-2.0 * std::log (u1));
        const double winkel = 6.283185307179586476925286766559 * u2;
        reserve = radius * std::sin (winkel);
        reserveGueltig = true;
        return radius * std::cos (winkel);
    }

private:
    std::uint64_t splitmix64() noexcept
    {
        zustand += 0x9e3779b97f4a7c15ull;
        auto z = zustand;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
        return z ^ (z >> 31);
    }

    std::uint64_t zustand {};
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
                 double samplerate = 48000.0) noexcept
{
    constexpr double zweiPi = 6.283185307179586476925286766559;
    return (float) (amplitude * std::sin (zweiPi * 1030.0
                                          * (double) sample / samplerate));
}

} // namespace nakama::test::nak380
