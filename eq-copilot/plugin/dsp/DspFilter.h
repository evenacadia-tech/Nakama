#pragma once

/*  SONDE-015 Etappe 3 - die Filterprimitiven des aktiven DSP-Kerns.

    Alles hier ist `double` (Entwurf §44.2: "64-Bit-Koeffizienten und
    -Filterzustaende"), auch wenn der Hostpuffer 32 Bit traegt: der
    Float-Callback konvertiert am RAND, nicht in der Kette (Matrix M-16).

    WARUM HEADER-ONLY: die Matrixzeile M-15 verlangt fuer den
    Richtungsbeweis (Slot 0 zuerst) ein unabhaengig gerechnetes, GEORDNETES
    Referenzergebnis "mit denselben Filterprimitiven". Der Golden muss die
    Primitiven also benutzen duerfen, ohne die Bank zu fahren.

    UND WOFUER SIE AUSDRUECKLICH NICHT DA SIND: der Frequenzgang-Golden
    (M-10 bis M-13) vergleicht NICHT gegen `entwurf()` von hier, sondern
    gegen eine im Test eigenstaendig ausgeschriebene RBJ-Formel. Ein
    Vergleich der Produktkoeffizienten mit der Produktformel bliebe gruen,
    selbst wenn der Audiopfad die Bank gar nicht anwendet.

    Quelle der Entwuerfe: RBJ Audio EQ Cookbook, dieselbe Fassung wie
    `core/analysis/KGewichtung.h` und `src/HoerMarkierung.h` sie tragen.
*/

#include <cmath>
#include <cstdint>

namespace nakama::dsp
{

inline constexpr double kPi = 3.14159265358979323846;

/** Anteil der Samplerate, ab dem eine Frequenz gekappt wird (§44.2:
    "20 Hz bis min(20 kHz, 0,45 fs)"). */
inline constexpr double kNyquistAnteil = 0.45;

//==============================================================================
/** Direct-Form-II-transponierter Biquad, `double`.

    Bewusst dieselbe Anordnung der Rechenschritte wie
    `core/analysis/KGewichtung.h:39-49`: Gleitkommaaddition ist nicht
    assoziativ, eine andere Klammerung waere eine andere Zahl, und die
    Bitgleichheit eines Goldens haengt daran. */
struct Biquad
{
    double b0 { 1.0 }, b1 { 0.0 }, b2 { 0.0 }, a1 { 0.0 }, a2 { 0.0 };

    /** Neutral heisst hier BITGENAU neutral: `y = 1.0 * x + 0` und die
        Zustandsfortschreibung bleibt 0. Ein "fast neutraler" Biquad haette
        die Bitidentitaet aus M-02 gekostet. */
    bool istEinheit() const noexcept
    {
        return b0 == 1.0 && b1 == 0.0 && b2 == 0.0 && a1 == 0.0 && a2 == 0.0;
    }
};

/** Der Zustand EINES Biquads. Getrennt von den Koeffizienten, weil §44.2
    genau diese Trennung verlangt: der Worker darf Koeffizienten erst nach
    dem Audio-ACK ueberschreiben, und ein Zustand, der in demselben Objekt
    liegt, waere dabei nicht auseinanderzuhalten. */
/** Schwelle des Denormal-Riegels: ein Zustandswert darunter traegt hoerbar
    nichts (unter -6000 dBFS) und wird am Blockrand genullt. Denormale
    Zwischenwerte kosten auf x86 ein Vielfaches jeder Rechnung - ein Kern,
    der im Leerlauf langsamer wird als unter Last, verletzt §44.5. */
inline constexpr double kDenormalSchwelle = 1e-300;

inline bool istDenormalKlein (double x) noexcept
{
    return x != 0.0 && std::abs (x) < kDenormalSchwelle;
}

struct BiquadZustand
{
    double z1 { 0.0 }, z2 { 0.0 };

    void nullen() noexcept { z1 = z2 = 0.0; }

    bool istEndlich() const noexcept { return std::isfinite (z1) && std::isfinite (z2); }

    /** Nullt denormal kleine Zustaende. Liefert true, wenn etwas genullt
        wurde - der Kern zaehlt das, damit der Riegel keine stille Wache
        bleibt (Pruefliste A). */
    bool riegleDenormale() noexcept
    {
        bool getroffen = false;
        if (istDenormalKlein (z1)) { z1 = 0.0; getroffen = true; }
        if (istDenormalKlein (z2)) { z2 = 0.0; getroffen = true; }
        return getroffen;
    }

    double tick (const Biquad& f, double x) noexcept
    {
        const double y = f.b0 * x + z1;
        z1 = f.b1 * x - f.a1 * y + z2;
        z2 = f.b2 * x - f.a2 * y;
        return y;
    }
};

//==============================================================================
// Die sechs Vertragstypen aus `nakama-parameter-v2.json` (Enumreihenfolge
// bell, low_shelf, high_shelf, notch, low_cut, high_cut). Die Reihenfolge
// ist Vertrag, nicht Geschmack: `NakamaParameter` speichert den Enumindex.

enum class Filtertyp { bell = 0, lowShelf, highShelf, notch, lowCut, highCut };

/** Kappt eine Frequenz auf `min(20 kHz, 0,45 fs)` und meldet, OB gekappt
    wurde (M-12: der gekappte Wert wird als geklemmt gemeldet, nicht still
    angewandt; der persistente Wert bleibt unberuehrt). */
inline double kappeNyquist (double freqHz, double samplerate, bool& gekappt) noexcept
{
    const double deckel = samplerate * kNyquistAnteil;
    gekappt = freqHz > deckel;
    return gekappt ? deckel : freqHz;
}

inline Biquad entwurfBell (double samplerate, double freqHz, double q, double gainDb) noexcept
{
    const double A     = std::pow (10.0, gainDb / 40.0);
    const double w0    = 2.0 * kPi * freqHz / samplerate;
    const double alpha = std::sin (w0) / (2.0 * q);
    const double c     = std::cos (w0);
    const double a0    = 1.0 + alpha / A;

    Biquad f;
    f.b0 = (1.0 + alpha * A) / a0;
    f.b1 = (-2.0 * c)        / a0;
    f.b2 = (1.0 - alpha * A) / a0;
    f.a1 = (-2.0 * c)        / a0;
    f.a2 = (1.0 - alpha / A) / a0;
    return f;
}

inline Biquad entwurfLowShelf (double samplerate, double freqHz, double q, double gainDb) noexcept
{
    const double A     = std::pow (10.0, gainDb / 40.0);
    const double w0    = 2.0 * kPi * freqHz / samplerate;
    const double alpha = std::sin (w0) / (2.0 * q);
    const double c     = std::cos (w0);
    const double zwei  = 2.0 * std::sqrt (A) * alpha;
    const double a0    = (A + 1.0) + (A - 1.0) * c + zwei;

    Biquad f;
    f.b0 =        A * ((A + 1.0) - (A - 1.0) * c + zwei) / a0;
    f.b1 =  2.0 * A * ((A - 1.0) - (A + 1.0) * c)        / a0;
    f.b2 =        A * ((A + 1.0) - (A - 1.0) * c - zwei) / a0;
    f.a1 = -2.0     * ((A - 1.0) + (A + 1.0) * c)        / a0;
    f.a2 =           ((A + 1.0) + (A - 1.0) * c - zwei)  / a0;
    return f;
}

inline Biquad entwurfHighShelf (double samplerate, double freqHz, double q, double gainDb) noexcept
{
    const double A     = std::pow (10.0, gainDb / 40.0);
    const double w0    = 2.0 * kPi * freqHz / samplerate;
    const double alpha = std::sin (w0) / (2.0 * q);
    const double c     = std::cos (w0);
    const double zwei  = 2.0 * std::sqrt (A) * alpha;
    const double a0    = (A + 1.0) - (A - 1.0) * c + zwei;

    Biquad f;
    f.b0 =        A * ((A + 1.0) + (A - 1.0) * c + zwei) / a0;
    f.b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * c)        / a0;
    f.b2 =        A * ((A + 1.0) + (A - 1.0) * c - zwei) / a0;
    f.a1 =  2.0     * ((A - 1.0) - (A + 1.0) * c)        / a0;
    f.a2 =           ((A + 1.0) - (A - 1.0) * c - zwei)  / a0;
    return f;
}

inline Biquad entwurfNotch (double samplerate, double freqHz, double q) noexcept
{
    const double w0    = 2.0 * kPi * freqHz / samplerate;
    const double alpha = std::sin (w0) / (2.0 * q);
    const double c     = std::cos (w0);
    const double a0    = 1.0 + alpha;

    Biquad f;
    f.b0 =  1.0            / a0;
    f.b1 = (-2.0 * c)      / a0;
    f.b2 =  1.0            / a0;
    f.a1 = (-2.0 * c)      / a0;
    f.a2 = (1.0 - alpha)   / a0;
    return f;
}

/** `low_cut` = Hochpass: schneidet UNTERHALB der Eckfrequenz. Der Vertragsname
    beschreibt, was verschwindet, nicht die Filterfamilie. */
inline Biquad entwurfLowCut (double samplerate, double freqHz, double q) noexcept
{
    const double w0    = 2.0 * kPi * freqHz / samplerate;
    const double alpha = std::sin (w0) / (2.0 * q);
    const double c     = std::cos (w0);
    const double a0    = 1.0 + alpha;

    Biquad f;
    f.b0 =  (1.0 + c) * 0.5 / a0;
    f.b1 = -(1.0 + c)       / a0;
    f.b2 =  (1.0 + c) * 0.5 / a0;
    f.a1 = (-2.0 * c)       / a0;
    f.a2 = (1.0 - alpha)    / a0;
    return f;
}

/** `high_cut` = Tiefpass. */
inline Biquad entwurfHighCut (double samplerate, double freqHz, double q) noexcept
{
    const double w0    = 2.0 * kPi * freqHz / samplerate;
    const double alpha = std::sin (w0) / (2.0 * q);
    const double c     = std::cos (w0);
    const double a0    = 1.0 + alpha;

    Biquad f;
    f.b0 =  (1.0 - c) * 0.5 / a0;
    f.b1 =  (1.0 - c)       / a0;
    f.b2 =  (1.0 - c) * 0.5 / a0;
    f.a1 = (-2.0 * c)       / a0;
    f.a2 = (1.0 - alpha)    / a0;
    return f;
}

/** Bandpass mit konstanter Spitzenverstaerkung (RBJ "constant 0 dB peak
    gain") - der Detektor des dynamischen Bandes (§5.7 Feinheit 3). Er hoert
    das bandgefilterte Signal VOR dem Band; ein Detektor mit
    Q-abhaengiger Spitze machte die Kennlinie Q-abhaengig. */
inline Biquad entwurfBandpass (double samplerate, double freqHz, double q) noexcept
{
    const double w0    = 2.0 * kPi * freqHz / samplerate;
    const double alpha = std::sin (w0) / (2.0 * q);
    const double c     = std::cos (w0);
    const double a0    = 1.0 + alpha;

    Biquad f;
    f.b0 =  alpha         / a0;
    f.b1 =  0.0;
    f.b2 = -alpha         / a0;
    f.a1 = (-2.0 * c)     / a0;
    f.a2 = (1.0 - alpha)  / a0;
    return f;
}

/** Der Entwurf eines der sechs Vertragstypen. `gainDb` gilt nur fuer bell
    und die beiden Shelves; notch und die beiden Cuts ignorieren ihn (der
    Vertrag laesst den Wert stehen, er wirkt nur nicht). */
inline Biquad entwurf (Filtertyp typ, double samplerate, double freqHz, double q, double gainDb) noexcept
{
    switch (typ)
    {
        case Filtertyp::bell:      return entwurfBell      (samplerate, freqHz, q, gainDb);
        case Filtertyp::lowShelf:  return entwurfLowShelf  (samplerate, freqHz, q, gainDb);
        case Filtertyp::highShelf: return entwurfHighShelf (samplerate, freqHz, q, gainDb);
        case Filtertyp::notch:     return entwurfNotch     (samplerate, freqHz, q);
        case Filtertyp::lowCut:    return entwurfLowCut    (samplerate, freqHz, q);
        case Filtertyp::highCut:   return entwurfHighCut   (samplerate, freqHz, q);
    }
    return {};
}

//==============================================================================
/** Die Huellkurve des dynamischen Bandes: RMS mit Attack, Hold und Release
    als DREI getrennte Stufen (M-26).

    Die Koeffizienten sind `exp(-1 / (fs * tau))` (§5.7 Feinheit 3) und
    damit samplerateunabhaengig in Millisekunden definiert. Gerechnet wird
    auf der MOMENTANLEISTUNG `x*x`; der dB-Wert entsteht erst am Ende als
    `10 * log10(leistung)`, nicht als `20 * log10(betrag)` einer gemittelten
    Amplitude - das waere kein RMS. */
struct HuellkurveKoeffizienten
{
    double attackPol  { 0.0 };   ///< exp(-1/(fs*tau_a))
    double releasePol { 0.0 };   ///< exp(-1/(fs*tau_r))
    std::int64_t holdSamples { 0 };
};

inline double huellkurvePol (double ms, double samplerate) noexcept
{
    const double tau = ms * 0.001;
    if (! (tau > 0.0) || ! (samplerate > 0.0)) return 0.0;   // 0 ms = sofort
    return std::exp (-1.0 / (samplerate * tau));
}

inline HuellkurveKoeffizienten huellkurveEntwurf (double attackMs, double holdMs, double releaseMs,
                                                 double samplerate) noexcept
{
    HuellkurveKoeffizienten k;
    k.attackPol   = huellkurvePol (attackMs,  samplerate);
    k.releasePol  = huellkurvePol (releaseMs, samplerate);
    k.holdSamples = (std::int64_t) std::llround (holdMs * 0.001 * samplerate);
    return k;
}

struct HuellkurveZustand
{
    double       leistung { 0.0 };
    std::int64_t holdRest { 0 };

    void nullen() noexcept { leistung = 0.0; holdRest = 0; }

    bool istEndlich() const noexcept { return std::isfinite (leistung); }

    bool riegleDenormale() noexcept
    {
        if (istDenormalKlein (leistung)) { leistung = 0.0; return true; }
        return false;
    }

    /** Ein Sample Detektorleistung hinein, geglaettete Leistung heraus.

        Reihenfolge (M-26): steigt der Wert, folgt er der Attack-Konstante
        und laedt den Hold-Zaehler neu; steht der Hold-Zaehler, bleibt der
        Wert; erst danach faellt er mit Release. Ein Hold, der die Attack
        NICHT neu laedt, hielte nach dem ersten Anstieg und nicht "nach dem
        letzten". */
    double tick (const HuellkurveKoeffizienten& k, double leistungEin) noexcept
    {
        if (leistungEin > leistung)
        {
            leistung = k.attackPol * leistung + (1.0 - k.attackPol) * leistungEin;
            holdRest = k.holdSamples;
        }
        else if (holdRest > 0)
        {
            --holdRest;
        }
        else
        {
            leistung = k.releasePol * leistung + (1.0 - k.releasePol) * leistungEin;
        }
        return leistung;
    }
};

/** Leistung -> dBFS. Stille ergibt einen sehr kleinen, ENDLICHEN Wert statt
    -inf: ein -inf im Detektor liefe durch die Kennlinie und machte den
    Nicht-Endlich-Riegel zum Dauergast. */
inline constexpr double kStilleDb = -240.0;

inline double leistungInDb (double leistung) noexcept
{
    if (! (leistung > 0.0) || ! std::isfinite (leistung)) return kStilleDb;
    const double db = 10.0 * std::log10 (leistung);
    return db < kStilleDb ? kStilleDb : db;
}

//==============================================================================
/** Die feste Kniebreite der dynamischen Kennlinie (§5.7 Feinheit 1).

    Technikkonstante mit Golden: der Entwurf nennt keine. */
inline constexpr double kKniebreiteDb = 12.0;

/** `u = min(1, max(0, e_db - t) / 12)` und `g_dyn = r * u` (§5.7).

    Die Auslenkung ist damit DURCH DIE KONSTRUKTION auf +/-|r| begrenzt; ein
    zweiter Klemmschritt waere eine zweite Wahrheit. Unter Threshold ist das
    Ergebnis EXAKT 0,0 - `max(0, negativ)` ist bitgenau null, und `r * 0.0`
    ist es auch (M-19). */
inline double dynamischeKennlinie (double pegelDb, double thresholdDb, double rangeDb) noexcept
{
    const double ueber = pegelDb - thresholdDb;
    if (! (ueber > 0.0)) return 0.0;
    const double u = ueber >= kKniebreiteDb ? 1.0 : ueber / kKniebreiteDb;
    return rangeDb * u;
}

//==============================================================================
/** dB -> linearer Faktor. Genau 0 dB ergibt BITGENAU 1,0 (`pow(10, 0)` ist
    exakt 1,0); die Kurzschluesse in `DspProgramm` verlassen sich darauf
    nicht, sondern pruefen den dB-Wert selbst gegen 0,0. */
inline double dbInLinear (double db) noexcept { return std::pow (10.0, db / 20.0); }

} // namespace nakama::dsp
