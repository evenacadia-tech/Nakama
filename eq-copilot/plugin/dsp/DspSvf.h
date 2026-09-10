#pragma once

/*  SONDE-015 Etappe 3 - der topology-preserving State-Variable-Filter der
    DYNAMISCHEN Baender.

    Entwurf §44.2 nennt die Arbeitsteilung woertlich:

      "minimumphasige RBJ-Biquads als Referenz fuer STATISCHE Bell-, Shelf-,
       Notch- und Cut-Filter; topology-preserving State-Variable-Filter
       beziehungsweise robuste Biquads fuer DYNAMISCHE Baender."

    Warum zwei Familien und nicht eine: ein RBJ-Biquad muss bei jeder
    Gainaenderung vollstaendig neu entworfen werden (sin, cos, pow je
    Koeffizient), und sein Zustand ist beim Koeffizientenwechsel nicht
    stetig - genau das erzeugt das Knacksen, das ein dynamisches Band
    unbrauchbar macht. Der TPT-SVF haelt seine Integratorzustaende beim
    Wechsel stetig und braucht fuer eine Gainaenderung nur `sqrt` und zwei
    Divisionen; `tan` haengt allein an der Frequenz und wird einmal beim
    Programmbau gerechnet.

    Die Form ist die von Andy Simper (Cytomic, "Solving the continuous SVF
    equations using trapezoidal integration"). Bei A = 1 (also 0 dB) ist sie
    BITGENAU neutral: `m0 = 1`, `m1 = m2 = 0` ergibt `y = 1.0*x + 0.0*v1 +
    0.0*v2`. Darauf ruht M-19 ("das Band steht still").

    NICHT gemessen wird der SVF gegen die analytische RBJ-Antwort - das
    waere ein Vergleich zweier verschiedener Filterfamilien. M-10 bis M-13
    messen den statischen Weg; der dynamische Weg wird gegen seine
    KENNLINIE gemessen (M-18 bis M-20).
*/

#include "DspFilter.h"

namespace nakama::dsp
{

/** Koeffizienten eines TPT-SVF in Mischform. `a1..a3` sind die
    Integratorkoeffizienten, `m0..m2` die Mischung aus Eingang, Bandpass
    und Tiefpass. */
struct SvfKoeffizienten
{
    double a1 { 1.0 }, a2 { 0.0 }, a3 { 0.0 };
    double m0 { 1.0 }, m1 { 0.0 }, m2 { 0.0 };

    /*  `g` und `k` stehen mit, weil die Auto-Gain-Ableitung die ANALYTISCHE
        Antwort dieses Filters braucht und sie aus `a1..a3` nur unter
        Ruecknahme einer Division zurueckzurechnen waere. Zwei Doubles je
        Band gegen einen Ruecktransformationsschritt, der bei k = 0 nicht
        eindeutig ist - die Wahl ist leicht. */
    double g  { 0.0 }, k { 0.0 };

    /** Bitgenau neutral - dieselbe Zusage wie `Biquad::istEinheit`. */
    bool istEinheit() const noexcept { return m0 == 1.0 && m1 == 0.0 && m2 == 0.0; }
};

struct SvfZustand
{
    double ic1eq { 0.0 }, ic2eq { 0.0 };

    void nullen() noexcept { ic1eq = ic2eq = 0.0; }

    bool istEndlich() const noexcept { return std::isfinite (ic1eq) && std::isfinite (ic2eq); }

    bool riegleDenormale() noexcept
    {
        bool getroffen = false;
        if (istDenormalKlein (ic1eq)) { ic1eq = 0.0; getroffen = true; }
        if (istDenormalKlein (ic2eq)) { ic2eq = 0.0; getroffen = true; }
        return getroffen;
    }

    double tick (const SvfKoeffizienten& k, double x) noexcept
    {
        const double v3 = x - ic2eq;
        const double v1 = k.a1 * ic1eq + k.a2 * v3;
        const double v2 = ic2eq + k.a2 * ic1eq + k.a3 * v3;
        ic1eq = 2.0 * v1 - ic1eq;
        ic2eq = 2.0 * v2 - ic2eq;
        return k.m0 * x + k.m1 * v1 + k.m2 * v2;
    }
};

/** `g = tan(pi * fc / fs)` - der einzige Teil, der `tan` braucht, und der
    einzige, der sich bei einer Gainaenderung NICHT aendert. Deshalb steht er
    getrennt und wird beim Programmbau einmal gerechnet. */
inline double svfGrundG (double freqHz, double samplerate) noexcept
{
    return std::tan (kPi * freqHz / samplerate);
}

/** Setzt `a1..a3` aus `g` und `k`. */
inline void svfIntegratoren (SvfKoeffizienten& c, double g, double k) noexcept
{
    c.g  = g;
    c.k  = k;
    c.a1 = 1.0 / (1.0 + g * (g + k));
    c.a2 = g * c.a1;
    c.a3 = g * c.a2;
}

inline SvfKoeffizienten svfBell (double grundG, double q, double gainDb) noexcept
{
    const double A = std::pow (10.0, gainDb / 40.0);
    const double k = 1.0 / (q * A);
    SvfKoeffizienten c;
    svfIntegratoren (c, grundG, k);
    c.m0 = 1.0;
    c.m1 = k * (A * A - 1.0);
    c.m2 = 0.0;
    return c;
}

inline SvfKoeffizienten svfLowShelf (double grundG, double q, double gainDb) noexcept
{
    const double A = std::pow (10.0, gainDb / 40.0);
    const double g = grundG / std::sqrt (A);
    const double k = 1.0 / q;
    SvfKoeffizienten c;
    svfIntegratoren (c, g, k);
    c.m0 = 1.0;
    c.m1 = k * (A - 1.0);
    c.m2 = A * A - 1.0;
    return c;
}

inline SvfKoeffizienten svfHighShelf (double grundG, double q, double gainDb) noexcept
{
    const double A = std::pow (10.0, gainDb / 40.0);
    const double g = grundG * std::sqrt (A);
    const double k = 1.0 / q;
    SvfKoeffizienten c;
    svfIntegratoren (c, g, k);
    c.m0 = A * A;
    c.m1 = k * (A - 1.0) * A;
    c.m2 = 1.0 - A * A;
    return c;
}

/** Nur die drei Typen mit Gain haben einen Angriffspunkt fuer die
    dynamische Auslenkung. `notch`, `low_cut` und `high_cut` tragen im
    Vertrag zwar ein `gain_db`, es wirkt dort aber nicht - und was nicht
    wirkt, kann auch nicht dynamisch wirken (Entscheid E-3 im Manifest). */
inline bool typHatGain (Filtertyp t) noexcept
{
    return t == Filtertyp::bell || t == Filtertyp::lowShelf || t == Filtertyp::highShelf;
}

inline SvfKoeffizienten svfEntwurf (Filtertyp typ, double grundG, double q, double gainDb) noexcept
{
    switch (typ)
    {
        case Filtertyp::bell:      return svfBell      (grundG, q, gainDb);
        case Filtertyp::lowShelf:  return svfLowShelf  (grundG, q, gainDb);
        case Filtertyp::highShelf: return svfHighShelf (grundG, q, gainDb);
        default:                   break;
    }
    return {};   // neutral - die drei Typen ohne Gain
}

} // namespace nakama::dsp
