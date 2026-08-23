// SONDE-009 (S12-13) — K-Gewichtung nach ITU-R BS.1770, als geteiltes Blatt.
//
// WOZU DIESE DATEI ENTSTEHT.  Die FeatureEngine v2 braucht dieselbe
// K-Gewichtung, die `AnalyseEngine` seit M1 rechnet: derselbe RBJ-High-Shelf
// (+4 dB, 1500 Hz, Q=1/√2) und derselbe Hochpass (38 Hz, Q=0,5).  Es gab zwei
// Wege, das zu haben - die Formel ein zweites Mal hinschreiben, oder sie an
// EINEN Ort ziehen und beide Verbraucher daran binden.
//
// Der zweite Weg ist gewaehlt, und die Begruendung ist nicht Aesthetik: zwei
// Kopien derselben Filterrechnung laufen genau so lange gleich, bis jemand
// eine davon anfasst.  Danach messen zwei Teile desselben Produkts dieselbe
// Lautheit verschieden, und zwar still - beide Zahlen sehen plausibel aus.
// Genau diese Klasse von Fehler hat dieses Projekt bei `framesDropped` schon
// einmal bezahlt (SONDE-008 §4: "EINE Wahrheit. Der Zaehler lebt in der Queue,
// die den Verlust verursacht; ein zweiter Atomic daneben koennte nur
// auseinanderlaufen").
//
// 🔑 DER BEWEIS, DASS ES DIESELBE RECHNUNG GEBLIEBEN IST, EXISTIERT SCHON.
// `AnalyseEngine` hat den `EqCopGoldenTest` gegen die Referenzachse von
// `tools/analyze-track.py`, und die Lautheit ist Teil davon.  Waere die
// verschobene Formel auch nur im letzten Bit anders, faellt dieser Test.  Der
// Umzug ist deshalb kein Vertrauensakt, sondern ein gemessener.
//
// JUCE-frei wie der Rest von `core/` — headless pruefbar, ohne Host.
#pragma once

#include <cmath>

namespace nakama::analyse
{

//==============================================================================
/** Direct-Form-II-transponierter Biquad, `double`.

    Identisch zu `AnalyseEngine::Biquad` — bewusst dieselbe Anordnung der
    Rechenschritte, weil Gleitkommaaddition nicht assoziativ ist: eine andere
    Klammerung waere eine andere Zahl, und die Bitgleichheit des GoldenTests
    haengt daran. */
struct Biquad
{
    double b0 { 1.0 }, b1 { 0.0 }, b2 { 0.0 }, a1 { 0.0 }, a2 { 0.0 };
    double z1 { 0.0 }, z2 { 0.0 };

    double tick (double x) noexcept
    {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void zustandNullen() noexcept { z1 = z2 = 0.0; }
};

//==============================================================================
// Die zwei Filterentwuerfe.  RBJ Audio EQ Cookbook, wie `pyloudnorm` sie
// verwendet — ausdruecklich NICHT die DeMan-Variante.  Welche der beiden ein
// Projekt nimmt, ist eine Wahl, keine Ableitung; hier ist sie seit M1
// getroffen und durch die Kreuzvalidierung gegen `analyze-track.py` gebunden.

inline constexpr double kPi = 3.14159265358979323846;

inline Biquad rbjHighShelf (double samplerate, double G, double Q, double fc) noexcept
{
    const double A     = std::pow (10.0, G / 40.0);
    const double w0    = 2.0 * kPi * fc / samplerate;
    const double alpha = std::sin (w0) / (2.0 * Q);
    const double c     = std::cos (w0);
    const double a0    = (A + 1) - (A - 1) * c + 2 * std::sqrt (A) * alpha;

    Biquad f;
    f.b0 = A * ((A + 1) + (A - 1) * c + 2 * std::sqrt (A) * alpha) / a0;
    f.b1 = -2 * A * ((A - 1) + (A + 1) * c) / a0;
    f.b2 = A * ((A + 1) + (A - 1) * c - 2 * std::sqrt (A) * alpha) / a0;
    f.a1 = 2 * ((A - 1) - (A + 1) * c) / a0;
    f.a2 = ((A + 1) - (A - 1) * c - 2 * std::sqrt (A) * alpha) / a0;
    return f;
}

inline Biquad rbjHighpass (double samplerate, double Q, double fc) noexcept
{
    const double w0    = 2.0 * kPi * fc / samplerate;
    const double alpha = std::sin (w0) / (2.0 * Q);
    const double c     = std::cos (w0);
    const double a0    = 1 + alpha;

    Biquad f;
    f.b0 = (1 + c) / 2 / a0;
    f.b1 = -(1 + c) / a0;
    f.b2 = (1 + c) / 2 / a0;
    f.a1 = -2 * c / a0;
    f.a2 = (1 - alpha) / a0;
    return f;
}

//==============================================================================
// Die BS.1770-Parameter.  Sie stehen hier als benannte Konstanten und nicht als
// Zahlen im Aufruf, damit ein Leser sie findet, ohne die Formel zu lesen.
inline constexpr double kShelfGainDb = 4.0;
inline constexpr double kShelfFreqHz = 1500.0;
inline constexpr double kHochpassFreqHz = 38.0;
inline constexpr double kHochpassQ = 0.5;

inline double shelfQ() noexcept { return 1.0 / std::sqrt (2.0); }

/** Die vollstaendige K-Kette EINES Kanals: Shelf, dann Hochpass. */
struct KKette
{
    Biquad shelf;
    Biquad hochpass;

    void entwerfen (double samplerate) noexcept
    {
        shelf    = rbjHighShelf (samplerate, kShelfGainDb, shelfQ(), kShelfFreqHz);
        hochpass = rbjHighpass (samplerate, kHochpassQ, kHochpassFreqHz);
    }

    void zustandNullen() noexcept
    {
        shelf.zustandNullen();
        hochpass.zustandNullen();
    }

    double tick (double x) noexcept { return hochpass.tick (shelf.tick (x)); }
};

} // namespace nakama::analyse
