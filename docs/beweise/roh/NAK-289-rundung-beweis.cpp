// NAK-289 Etappe 1, Fundklasse e): bugprone-incorrect-roundings in
// eq-copilot/plugin/dsp/DspKern.cpp, mische (HuellkurveKoeffizienten).
//
// Frage: liefert
//     neu = (std::int64_t) std::llround (A + (B - A) * t)
// fuer JEDE Eingabe, die der Aufrufer liefern kann, dasselbe wie
//     alt = (std::int64_t) (A + (B - A) * t + 0.5) ?
//
// Wertebereich am Basisstand 432f209c (aus der Quelle gelesen):
//   A, B    = HuellkurveKoeffizienten::holdSamples
//           = (int64) llround (hold_ms * 0.001 * fs)          dsp/DspFilter.h huellkurveEntwurf
//   hold_ms in [0, 500], endlich                               state/NakamaParameter.cpp (Tabelle; validiere)
//   fs      in {0} oder (0, 768000]                            sonde/SondeProcessor.cpp prepareToPlay (sichereRate)
//   => A, B in [0, 384000]
//   t       = 1 - j / 256, j = rampeRest - i - 1 in [1, 255]  dsp/DspKern.cpp verarbeiteBand (mische nur bei t < 1)
//
// Die Ausdruecke in alt() und neu() sind zeichengleich zur Quelle vor und nach
// dem Umbau (Klammerung und Casts). Uebersetzt mit denselben Gleitkomma-
// Schaltern wie der Kern (Release: /O2, /fp:precise als Voreinstellung, x64 SSE2).
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <random>
#include <vector>

namespace
{
constexpr std::int64_t kHoldMax = 384000;   // llround (500 * 0.001 * 768000)
constexpr int kRampeSamples = 256;          // dsp/DspProgramm.h

std::int64_t alt (std::int64_t a, std::int64_t b, double t)
{
    return (std::int64_t) ((double) a
                           + ((double) b - (double) a) * t + 0.5);
}

std::int64_t neu (std::int64_t a, std::int64_t b, double t)
{
    return (std::int64_t) std::llround ((double) a
                                        + ((double) b - (double) a) * t);
}

// wie verarbeiteBand: t = 1.0 - (double) (rampeRest - i - 1) / (double) kRampeSamples
double rampenT (int j)
{
    return 1.0 - (double) j / (double) kRampeSamples;
}

struct Zaehler
{
    std::uint64_t n = 0, abweichend = 0, unexakt = 0, tNichtUnterEins = 0;
};

void pruefeTripel (std::int64_t a, std::int64_t b, int j, Zaehler& z)
{
    const double t = rampenT (j);
    // exakter Mischwert mal 256: a + (b - a) * (256 - j) / 256
    const std::int64_t x256 = a * kRampeSamples + (b - a) * (kRampeSamples - j);
    const double exakt = (double) x256 / (double) kRampeSamples;
    const double gerechnet = (double) a + ((double) b - (double) a) * t;
    ++z.n;
    if (! (t < 1.0)) ++z.tNichtUnterEins;
    if (gerechnet != exakt) ++z.unexakt;
    if (alt (a, b, t) != neu (a, b, t)) ++z.abweichend;
}

void druckeZaehler (const char* name, const Zaehler& z)
{
    std::printf ("%s: %llu Tripel, %llu abweichend, %llu nicht exakt (Mischwert != m/256), %llu mit t >= 1\n",
                 name, (unsigned long long) z.n, (unsigned long long) z.abweichend,
                 (unsigned long long) z.unexakt, (unsigned long long) z.tNichtUnterEins);
}
} // namespace

int main()
{
    int rc = 0;

    // 0) Obergrenze der Haltezeit in Samples, gerechnet wie huellkurveEntwurf.
    const std::int64_t amax = (std::int64_t) std::llround (500.0 * 0.001 * 768000.0);
    std::printf ("0) holdSamples (hold_ms 500, fs 768000) = %lld; Annahme kHoldMax = %lld\n",
                 (long long) amax, (long long) kHoldMax);
    if (amax != kHoldMax)
        rc = 1;

    // 1) Jeder exakte Mischwert x = m / 256 mit m = 0 .. kHoldMax * 256.
    {
        std::uint64_t n = 0, abw = 0;
        for (std::int64_t m = 0; m <= kHoldMax * kRampeSamples; ++m)
        {
            const double x = (double) m / (double) kRampeSamples;
            ++n;
            if ((std::int64_t) (x + 0.5) != (std::int64_t) std::llround (x))
                ++abw;
        }
        std::printf ("1) x = m/256 fuer m in [0, %lld]: %llu Werte, %llu abweichend\n",
                     (long long) (kHoldMax * kRampeSamples), (unsigned long long) n, (unsigned long long) abw);
        if (abw != 0)
            rc = 1;
    }

    // 2) Randtripel: Grenzen des Bereichs, Zweierpotenzen und ihre Nachbarn, alle j.
    {
        std::vector<std::int64_t> raender = { 0, 1, 2, 127, 128, 129, 255, 256, 257,
                                              191999, 192000, 192001, 383743, 383744, 383999, kHoldMax };
        for (int k = 1; k <= 18; ++k)
        {
            const std::int64_t p = (std::int64_t) 1 << k;
            for (const std::int64_t d : { (std::int64_t) -1, (std::int64_t) 0, (std::int64_t) 1 })
                if (p + d >= 0 && p + d <= kHoldMax)
                    raender.push_back (p + d);
        }
        Zaehler z;
        for (const auto a : raender)
            for (const auto b : raender)
                for (int j = 1; j < kRampeSamples; ++j)
                    pruefeTripel (a, b, j, z);
        druckeZaehler ("2) Randtripel", z);
        if (z.abweichend != 0 || z.unexakt != 0 || z.tNichtUnterEins != 0)
            rc = 1;
    }

    // 2b) Einzelne Randwerte zum Nachlesen: Halbwerte, kleinste und groesste Rampenposition.
    {
        struct Rand { std::int64_t a, b; int j; const char* was; };
        const Rand faelle[] = {
            { 0, 1, 128, "x = 0,5 (Halbwert, steigend)" },
            { 1, 0, 128, "x = 0,5 (Halbwert, fallend)" },
            { 1, 2, 128, "x = 1,5" },
            { 383999, kHoldMax, 128, "x = 383999,5 (Halbwert am oberen Rand)" },
            { 0, 1, 255, "x = 1/256 (erstes Rampensample)" },
            { 0, 1, 1, "x = 255/256 (letztes Rampensample vor dem Ziel)" },
            { kHoldMax, 0, 1, "x = 384000 * 1/256 (fallend, letztes Rampensample)" },
            { 0, kHoldMax, 255, "x = 384000 * 1/256 (steigend, erstes Rampensample)" },
            { 19200, 4800, 127, "Faelle aus B6 W-2 (400 ms -> 100 ms bei 48 kHz)" },
        };
        for (const auto& f : faelle)
        {
            const double t = rampenT (f.j);
            const double x = (double) f.a + ((double) f.b - (double) f.a) * t;
            std::printf ("2b) A=%lld B=%lld j=%d t=%.17g x=%.17g: alt %lld, neu %lld  [%s]\n",
                         (long long) f.a, (long long) f.b, f.j, t, x,
                         (long long) alt (f.a, f.b, t), (long long) neu (f.a, f.b, t), f.was);
            if (alt (f.a, f.b, t) != neu (f.a, f.b, t))
                rc = 1;
        }
    }

    // 3) Zufallstripel ueber den ganzen Bereich, fester Seed.
    {
        std::mt19937_64 gen (0x4E414B323839ull);
        std::uniform_int_distribution<std::int64_t> hold (0, kHoldMax);
        std::uniform_int_distribution<int> jv (1, kRampeSamples - 1);
        Zaehler z;
        for (std::uint64_t i = 0; i < 200000000ull; ++i)
        {
            const std::int64_t a = hold (gen);
            const std::int64_t b = hold (gen);
            pruefeTripel (a, b, jv (gen), z);
        }
        druckeZaehler ("3) Zufallstripel (Seed 0x4E414B323839)", z);
        if (z.abweichend != 0 || z.unexakt != 0 || z.tNichtUnterEins != 0)
            rc = 1;
    }

    // 4) Trennschaerfe AUSSERHALB des Wertebereichs: dort unterscheiden sich
    //    die beiden Rundungen wirklich. Kein Aufrufer liefert diese Werte.
    {
        const double faelle[] = { std::nextafter (0.5, 0.0), -0.5, -1.25, 4503599627370497.0 };
        for (const double x : faelle)
            std::printf ("4) ausserhalb: x = %.17g: alt %lld, neu %lld\n",
                         x, (long long) (std::int64_t) (x + 0.5), (long long) std::llround (x));
    }

    std::printf ("URTEIL: %s\n", rc == 0 ? "gleich fuer den ganzen Wertebereich des Aufrufers"
                                         : "ABWEICHUNG IM WERTEBEREICH");
    return rc;
}
