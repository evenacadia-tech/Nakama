// SONDE-009 (S12-13) — Radix-2-FFT, JUCE-frei.
//
// WARUM NICHT `juce::dsp::FFT`.  Der Realtime-/Analysis-Core (`plugin/core/`)
// ist bewusst bibliotheksfrei: `StampedAudioQueue.h`, `LoudnessAccumulator.h`
// und `NakamaHostBridge.h` tragen alle dieselbe Begruendung — so liegt die
// riskante Logik in UNSEREM Repo und ist headless pruefbar, ohne Host und ohne
// Modulbaum.  Eine FFT ist dafuer der billigste Baustein, den es gibt:
// Cooley-Tukey ist keine Erfindung, sondern Standardware, und der Golden misst
// sie gegen ein analytisch bekanntes Ergebnis (Parseval und ein reiner Sinus
// auf einer Binmitte).
//
// 🔑 Ein Nebeneffekt, der hier wichtiger ist als die Ersparnis: `AnalyseEngine`
// (M1) und `FeatureEngine` (v2) rechnen damit auf VERSCHIEDENEN FFTs.  Das ist
// gewollt.  Beide messen dieselbe Physik auf verschiedenen Achsen (siehe
// BandGrid.h, Kopf); waeren sie an derselben Implementierung, sae he ein
// Vergleich ihrer Ergebnisse besser aus, als er ist.
//
// Der gesamte Speicher entsteht in `vorbereiten()`.  `transformiere()` selbst
// alloziert nichts — sie laeuft zwar im Worker und nicht im Audiothread, aber
// eine Allokation je Fenster bei 10 Hz waere trotzdem Muell in einem Blatt,
// das "fester Speicher" verspricht.
#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace nakama::analyse
{

/** Reelle Radix-2-FFT ueber `n` Punkte (n Zweierpotenz).

    Rechnet komplex und liefert die Leistungen der Bins 0..n/2. Das ist fuer
    unsere Groessen (4096) verschwenderisch um Faktor zwei gegenueber einer
    echten Real-FFT — und es ist die richtige Wahl: die Real-FFT-Faltung ist
    genau die Stelle, an der eine handgeschriebene FFT still falsch wird, und
    hier zaehlt Nachvollziehbarkeit mehr als der Faktor. */
class Fft
{
public:
    /** Legt Twiddles und Bitumkehrtabelle an. Idempotent bei gleicher Groesse. */
    void vorbereiten (int n)
    {
        if (n == punkte && ! cosTab.empty())
            return;
        punkte = n;

        stufen = 0;
        while ((1 << stufen) < n)
            ++stufen;

        cosTab.assign ((std::size_t) (n / 2), 0.0);
        sinTab.assign ((std::size_t) (n / 2), 0.0);
        for (int i = 0; i < n / 2; ++i)
        {
            const double w = -2.0 * 3.14159265358979323846 * (double) i / (double) n;
            cosTab[(std::size_t) i] = std::cos (w);
            sinTab[(std::size_t) i] = std::sin (w);
        }

        umkehr.assign ((std::size_t) n, 0u);
        for (int i = 0; i < n; ++i)
        {
            unsigned r = 0;
            for (int b = 0; b < stufen; ++b)
                if ((i & (1 << b)) != 0)
                    r |= (unsigned) 1u << (stufen - 1 - b);
            umkehr[(std::size_t) i] = r;
        }

        re.assign ((std::size_t) n, 0.0);
        im.assign ((std::size_t) n, 0.0);
    }

    int groesse() const noexcept { return punkte; }

    /** `eingang` traegt `n` reelle Werte. Danach liefert `leistung(k)` fuer
        k = 0..n/2 den Betragsquadrat-Wert des Bins. */
    void transformiere (const double* eingang) noexcept
    {
        const int n = punkte;
        for (int i = 0; i < n; ++i)
        {
            const std::size_t z = (std::size_t) umkehr[(std::size_t) i];
            re[z] = eingang[i];
            im[z] = 0.0;
        }

        for (int laenge = 2; laenge <= n; laenge <<= 1)
        {
            const int halb = laenge / 2;
            const int schritt = n / laenge;
            for (int i = 0; i < n; i += laenge)
            {
                for (int j = 0; j < halb; ++j)
                {
                    const std::size_t t = (std::size_t) (j * schritt);
                    const double wr = cosTab[t];
                    const double wi = sinTab[t];
                    const std::size_t a = (std::size_t) (i + j);
                    const std::size_t b = (std::size_t) (i + j + halb);
                    const double tr = re[b] * wr - im[b] * wi;
                    const double ti = re[b] * wi + im[b] * wr;
                    re[b] = re[a] - tr;
                    im[b] = im[a] - ti;
                    re[a] += tr;
                    im[a] += ti;
                }
            }
        }
    }

    double leistung (int k) const noexcept
    {
        const double r = re[(std::size_t) k];
        const double i = im[(std::size_t) k];
        return r * r + i * i;
    }

private:
    int punkte { 0 };
    int stufen { 0 };
    std::vector<double>   cosTab, sinTab, re, im;
    std::vector<unsigned> umkehr;
};

} // namespace nakama::analyse
