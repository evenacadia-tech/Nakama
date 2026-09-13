/*  EqCopSonde013TruePeakGoldenTest — der True-Peak-Pfad (SONDE-013 M-02).

    Matrixzeile M-02 verlangt eine MESSBARE Entscheidung, keine Behauptung:

        "Der vorhandene 8-fach-Pfad bleibt NUR, wenn er das offizielle
         EBU-Testset bei 48 kHz und generierte Mehrsampleraten-Goldens
         besteht; sonst wird der BS.1770-Polyphase-Referenzpfad verwendet.
         Toleranz +/-0,1 dB (§49.3). Das Bein muss die Entscheidung 8-fach
         gegen Polyphase MESSBAR treffen, nicht behaupten."

    ── WOGEGEN HIER GEMESSEN WIRD, UND WARUM DAS BESSER IST ALS EIN TESTSET ──

    Die True-Peak-Faelle aus EBU Tech 3341 (Testfaelle 15 bis 19) sind KEINE
    aufgenommenen Dateien, sondern analytisch definierte Sinus: eine Frequenz
    als Bruchteil der Samplerate, eine Amplitude in FFS und eine Phase. Der
    wahre Scheitel eines Sinus IST seine Amplitude — die Referenz ist damit
    exakt, nicht selbst eine Messung.

    Das hat drei Folgen, die dieses Bein tragen:

    1. Der Sampleraten-Sweep aus M-02 entsteht von selbst. Die Frequenzen sind
       fs/4, fs/6 und fs/8; bei jeder Samplerate ist die NORMIERTE Frequenz
       dieselbe, und dieselbe Erwartung gilt. Ein heruntergeladenes 48-kHz-Set
       koennte diese Frage gar nicht stellen.
    2. Es gibt keine zweite Implementierung, gegen die gerechnet wird. Ein
       Golden gegen einen zweiten eigenen Detektor waere eine Tautologie —
       genau der Fehler, den `docs/beweise/SONDE-008.md` bei der Loudness
       ausdruecklich vermieden hat.
    3. Die 8-fach-Entscheidung wird nachgerechnet statt geglaubt. Der
       Restfehler der Ueberabtastung hat eine geschlossene Form, und
       `TruePeakDetektor::interpolationsfehlerDb` traegt sie. Dieses Bein
       misst, dass die WIRKLICHE Abweichung diese Schranke einhaelt — und
       dass 4-fach sie bei fs/4 reissen wuerde. Das ist die Entscheidung
       zwischen 8-fach und Polyphase, als Zahl.

    Die EBU-Toleranz ist +0,2/-0,4 dB. Nakama misst gegen die schaerfere aus
    §49.3: +/-0,1 dB. Wer die weichere braeuchte, haette den Pfad nicht.

    ── DIE TRANSIENTEN FAELLE 20 BIS 23 (NAK-283 N01, 13.09.2026) ──

    EBU Tech 3341 v4.0 §2.6 verlangt die Signale 15 bis 23; bis NAK-283 fuhr
    dieses Bein nur 15 bis 19. Die Faelle 20 bis 23 sind transient - ein
    fs/6-Sinus mit einer einzelnen fs/4-Periode, bei 4*fs synthetisiert,
    tiefpassgefiltert und mit Versatz 0 bis 3 auf fs heruntergetastet - und
    damit NICHT analytisch geschlossen: der Scheitel des gefilterten Signals ist
    nicht die Amplitude. Abschnitt 7 rechnet deshalb eine Referenz im Test
    selbst (64-fache Ueberabtastung mit langem Kaiser-Sinc) und beweist sie
    VOR dem Einsatz: an 15 bis 19 auf +/-0,01 dB und am transienten Material
    selbst, wo sie an den 4*fs-Stellen dem synthetisierten Signal gleichen
    muss. Erst dann misst jeder der vier Faelle zweimal: +/-0,1 dB gegen diese
    Referenz (§49.3 gilt fuer 15 bis 23) UND die Normtoleranz 0,0 +0,2/-0,4 dBTP.
    Filter und Referenz leben hier; der 8-fach-Pfad bleibt, wie er ist.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/TruePeak.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

using nakama::analyse::TruePeakDetektor;

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

/** Misst den groessten True Peak eines Sinus, linear.

    `teiler` ist der Nenner von fs: 4 heisst fs/4. Die Phase kommt in Grad,
    weil EBU Tech 3341 sie so angibt.

    Der Vorlauf von `kEinschwingen` Samples wird verworfen: der Interpolator
    ist ein FIR mit 24 Taps je Phase, und solange sein Verlauf noch Nullen
    enthaelt, misst er einen Ausschnitt, nicht den Sinus. Das ist keine
    Nachsicht gegenueber dem Detektor — im Betrieb steht der Verlauf nach
    24 Samples, also nach einer halben Millisekunde. */
double sinusTruePeak (double amplitude, int teiler, double phaseGrad,
                      int samples = 20000)
{
    constexpr int kEinschwingen = TruePeakDetektor::kTapsJePhase + 8;
    TruePeakDetektor d;
    d.vorbereiten (48000.0);
    const double phase = phaseGrad * kZweiPi / 360.0;
    double groesster = 0.0;
    for (int i = 0; i < samples; ++i)
    {
        const double x = amplitude * std::sin (kZweiPi * (double) i / (double) teiler + phase);
        const double tp = d.tick (x, x);
        if (i >= kEinschwingen)
            groesster = std::max (groesster, tp);
    }
    return groesster;
}

double db (double linear) { return linear > 0.0 ? 20.0 * std::log10 (linear) : -400.0; }

/** Der groesste Abtastwert desselben Signals — die Zahl, die ein Detektor
    OHNE Ueberabtastung melden wuerde. Sie ist der Gegenbeleg: ohne sie
    saehe man nicht, wie viel die Interpolation ueberhaupt findet. */
double sinusSamplePeak (double amplitude, int teiler, double phaseGrad,
                        int samples = 20000)
{
    const double phase = phaseGrad * kZweiPi / 360.0;
    double groesster = 0.0;
    for (int i = 0; i < samples; ++i)
        groesster = std::max (groesster,
            std::abs (amplitude * std::sin (kZweiPi * (double) i / (double) teiler + phase)));
    return groesster;
}

// ── Die transienten EBU-Faelle 20 bis 23 (NAK-283 N01, M-65) ──────────────
//
// EBU Tech 3341 v4.0 (2023), Table 1, Fall 20 (zitiert nach
// docs/beweise/NAK-283.md §4 N01): "Stereo sine wave with frequency fs/6 Hz,
// amplitude 0.50 FFS, containing a single period of a sine wave with frequency
// fs/4, amplitude 1.00 ... synthesized at 4*fs ... and then lowpass
// (anti-aliasing) filtered and downsampled to fs with a 0 samples offset.
// ... Max. true-peak level = 0.0 +0.2/-0.4 dBTP"; 21/22/23 "As #20, but
// downsampled with a 1 / 2 / 3 samples offset (at the 4*fs rate)".

constexpr int    kEbuLaenge     = 9600;    // Samples bei fs = 48 kHz, 0,2 s
constexpr int    kEbuBurstStart = 19200;   // 4*fs-Index, an dem die fs/4-Periode beginnt
constexpr int    kAaTaps        = 257;     // Anti-Aliasing-Tiefpass bei 4*fs
constexpr double kAaGrenze      = 0.1125;  // 6-dB-Punkt in Zyklen je Sample bei 4*fs = 0,45*fs
constexpr double kAaBeta        = 10.0;    // Kaiser: Uebergang 0,4 bis 0,5*fs, rund 100 dB Sperrdaempfung
constexpr int    kRefFaktor     = 64;      // Ueberabtastung der Referenz
constexpr int    kRefHalbe      = 64;      // Kernhalbbreite in fs-Samples: 128 Taps je Phase
constexpr double kRefBeta       = 12.0;

// Die fs/4-Periode beginnt auf einem Nulldurchgang des fs/6-Sinus (Periode 24
// Samples bei 4*fs) und auf dem Raster von fs (4 Samples): so ist das Signal an
// beiden Seiten phasen- UND wertstetig, und bei Versatz 0 liegt der Scheitel
// auf einem Abtastwert. Die Signalmitte liegt 4 800 fs-Samples von jedem Rand -
// weit ausserhalb der 32 fs-Samples Laufzeit des Tiefpasses und der 64 Samples
// Kernhalbbreite der Referenz.
static_assert (kEbuBurstStart % 24 == 0 && kEbuBurstStart % 4 == 0,
               "die Einzelperiode beginnt auf einem Nulldurchgang und auf dem fs-Raster");

double kaiserI0 (double x)
{
    double summe = 1.0, glied = 1.0;
    const double halbes = 0.5 * x;
    for (int k = 1; k < 80; ++k)
    {
        glied *= (halbes / (double) k) * (halbes / (double) k);
        summe += glied;
        if (glied < 1e-18 * summe)
            break;
    }
    return summe;
}

/** Kaiser-Fenster bei relativem Abstand zur Mitte (-1 .. 1), ausserhalb 0. */
double kaiserFenster (double relativ, double beta)
{
    const double arg = 1.0 - relativ * relativ;
    return arg < 0.0 ? 0.0 : kaiserI0 (beta * std::sqrt (arg)) / kaiserI0 (beta);
}

/** Fall 20 bei 4*fs. Die Phase wird je Abschnitt geschlossen gerechnet statt
    aufsummiert - kein Rundungsdrift ueber 38 400 Samples. */
std::vector<double> ebuTransientBei4fs()
{
    const int n4 = 4 * kEbuLaenge;
    std::vector<double> x ((size_t) n4);
    for (int n = 0; n < n4; ++n)
    {
        if (n < kEbuBurstStart)
            x[(size_t) n] = 0.5 * std::sin (kZweiPi * (double) (n % 24) / 24.0);
        else if (n < kEbuBurstStart + 16)
            x[(size_t) n] = 1.0 * std::sin (kZweiPi * (double) (n - kEbuBurstStart) / 16.0);
        else
            x[(size_t) n] = 0.5 * std::sin (kZweiPi * (double) ((n - kEbuBurstStart - 16) % 24) / 24.0);
    }
    return x;
}

/** Der Anti-Aliasing-Tiefpass: Kaiser-gefensterter Sinc bei 4*fs, auf DC 1 normiert. */
std::vector<double> aaKoeffizienten()
{
    std::vector<double> h ((size_t) kAaTaps);
    const int mitte = (kAaTaps - 1) / 2;
    double summe = 0.0;
    for (int i = 0; i < kAaTaps; ++i)
    {
        const double m = (double) (i - mitte);
        const double arg = kZweiPi * kAaGrenze * m;
        const double sinc = (i == mitte) ? 1.0 : std::sin (arg) / arg;
        h[(size_t) i] = 2.0 * kAaGrenze * sinc * kaiserFenster (m / (double) mitte, kAaBeta);
        summe += h[(size_t) i];
    }
    for (auto& v : h)
        v /= summe;
    return h;
}

/** Betragsfrequenzgang des Tiefpasses in dB bei f (relativ zu fs; er laeuft bei 4*fs). */
double aaGangDb (const std::vector<double>& h, double fRelFs)
{
    const int mitte = (kAaTaps - 1) / 2;
    const double w = kZweiPi * fRelFs / 4.0;
    double summe = 0.0;
    for (int i = 0; i < kAaTaps; ++i)
        summe += h[(size_t) i] * std::cos (w * (double) (i - mitte));
    return db (std::abs (summe));
}

/** Tiefpass ohne Laufzeit: y[n] = sum_k h[k] * x[n + mitte - k], ausserhalb 0. */
std::vector<double> aaGefiltert (const std::vector<double>& x, const std::vector<double>& h)
{
    const int n4 = (int) x.size();
    const int mitte = (kAaTaps - 1) / 2;
    std::vector<double> y ((size_t) n4, 0.0);
    for (int n = 0; n < n4; ++n)
    {
        double acc = 0.0;
        for (int k = 0; k < kAaTaps; ++k)
        {
            const int j = n + mitte - k;
            if (j >= 0 && j < n4)
                acc += h[(size_t) k] * x[(size_t) j];
        }
        y[(size_t) n] = acc;
    }
    return y;
}

/** Heruntertasten auf fs mit Versatz in 4*fs-Samples: y[m] = x[versatz + 4*m]. */
std::vector<double> dezimiert (const std::vector<double>& x4fs, int versatz)
{
    std::vector<double> y ((size_t) kEbuLaenge);
    for (int m = 0; m < kEbuLaenge; ++m)
        y[(size_t) m] = x4fs[(size_t) (versatz + 4 * m)];
    return y;
}

/** Die 64 Phasen der Referenz: Kaiser-Sinc ueber 128 fs-Samples, jede Phase auf
    Summe 1 - wie im Produktpfad, damit ein Gleichanteil er selbst bleibt. */
std::vector<std::vector<double>> referenzKerne()
{
    std::vector<std::vector<double>> kerne ((size_t) kRefFaktor,
                                            std::vector<double> ((size_t) (2 * kRefHalbe)));
    for (int j = 0; j < kRefFaktor; ++j)
    {
        double summe = 0.0;
        for (int i = 0; i < 2 * kRefHalbe; ++i)
        {
            const double tau = (double) (i - (kRefHalbe - 1)) + (double) j / (double) kRefFaktor;
            const double sinc = (tau == 0.0) ? 1.0 : std::sin (0.5 * kZweiPi * tau) / (0.5 * kZweiPi * tau);
            kerne[(size_t) j][(size_t) i] = sinc * kaiserFenster (tau / (double) kRefHalbe, kRefBeta);
            summe += kerne[(size_t) j][(size_t) i];
        }
        for (auto& v : kerne[(size_t) j])
            v /= summe;
    }
    return kerne;
}

struct ReferenzErgebnis
{
    double maximum = 0.0;         // groesster Betrag der Rekonstruktion, linear
    double abweichung4fs = -1.0;  // groesste Abweichung zum 4*fs-Signal, -1 = nicht verglichen
};

/** Die gerechnete Referenz: Rekonstruktion bei 64-facher Ueberabtastung im
    Inneren [kRefHalbe, N - kRefHalbe) - dort sieht der Kern nur Signal. Mit
    `signal4fs` vergleicht sie an den Phasen 0, 16, 32, 48 (dem 4*fs-Raster)
    gegen das gefilterte 4*fs-Signal, aus dem `x` heruntergetastet wurde. */
ReferenzErgebnis referenzTruePeak (const std::vector<double>& x,
                                   const std::vector<std::vector<double>>& kerne,
                                   const std::vector<double>* signal4fs = nullptr, int versatz = 0)
{
    ReferenzErgebnis e;
    if (signal4fs != nullptr)
        e.abweichung4fs = 0.0;
    const int n = (int) x.size();
    const int schritt4fs = kRefFaktor / 4;
    for (int j = 0; j < kRefFaktor; ++j)
    {
        const auto& kern = kerne[(size_t) j];
        for (int t = kRefHalbe; t < n - kRefHalbe; ++t)
        {
            double acc = 0.0;
            for (int i = 0; i < 2 * kRefHalbe; ++i)
                acc += kern[(size_t) i] * x[(size_t) (t - i + kRefHalbe - 1)];
            e.maximum = std::max (e.maximum, std::abs (acc));
            if (signal4fs != nullptr && j % schritt4fs == 0)
            {
                const double soll = (*signal4fs)[(size_t) (versatz + 4 * t + j / schritt4fs)];
                e.abweichung4fs = std::max (e.abweichung4fs, std::abs (acc - soll));
            }
        }
    }
    return e;
}

/** Der Pruefling: der 8-fach-Pfad ueber das ganze Signal, beide Kanaele gleich
    ("stereo sine wave"), mit Einschwingen wie in Abschnitt 1 und Nachlauf. */
double detektorTruePeak (const std::vector<double>& x)
{
    constexpr int kEinschwingen = TruePeakDetektor::kTapsJePhase + 8;
    TruePeakDetektor d;
    d.vorbereiten (48000.0);
    double groesster = 0.0;
    for (int i = 0; i < (int) x.size(); ++i)
    {
        const double tp = d.tick (x[(size_t) i], x[(size_t) i]);
        if (i >= kEinschwingen)
            groesster = std::max (groesster, tp);
    }
    return std::max (groesster, d.nachlauf());
}
} // namespace

int main()
{
    std::cout << "== Nakama SONDE-013 M-02 - True Peak nach ITU-R BS.1770-5 ==" << std::endl;

    // ── 1. Die fuenf True-Peak-Faelle aus EBU Tech 3341 ───────────────────
    //
    // Sie stehen hier mit ihrer Nummer, damit ein Leser sie in der Norm
    // wiederfindet. Die Erwartung ist NICHT die Tabellenzahl der Norm,
    // sondern 20*log10(Amplitude) — dieselbe Zahl, aber ausgerechnet statt
    // abgelesen, und damit gegen einen Tippfehler in dieser Datei gesichert.
    abschnitt ("true_peak_matches_ebu_test_set - EBU Tech 3341, Testfaelle 15 bis 19 (Toleranz aus §49.3: +/-0,1 dB)");
    {
        const struct { const char* name; double amplitude; int teiler; double phase; } faelle[] = {
            { "15  fs/4, 0,50 FFS, 0 Grad",    0.50, 4, 0.0 },
            { "16  fs/4, 0,50 FFS, 45 Grad",   0.50, 4, 45.0 },
            { "17  fs/6, 0,50 FFS, 60 Grad",   0.50, 6, 60.0 },
            { "18  fs/8, 0,50 FFS, 67,5 Grad", 0.50, 8, 67.5 },
            { "19  fs/4, 1,41 FFS, 45 Grad",   1.41, 4, 45.0 },
        };
        for (const auto& f : faelle)
        {
            const double gemessen = db (sinusTruePeak (f.amplitude, f.teiler, f.phase));
            const double erwartet = db (f.amplitude);
            const double abw = std::abs (gemessen - erwartet);
            pruefe (abw <= 0.1,
                    juce::String ("Testfall ") + f.name + ": True Peak trifft die Amplitude",
                    "gemessen " + juce::String (gemessen, 3) + " dBTP, erwartet "
                    + juce::String (erwartet, 3) + ", Abweichung "
                    + juce::String (abw, 4) + " dB");
        }
    }

    // ── 2. Der Gegenbeleg: OHNE Ueberabtastung waeren es 3 dB weniger ─────
    //
    // Ohne diesen Abschnitt koennte der ganze Test gruen sein, weil der
    // Detektor einfach den Samplemaximalwert zurueckgibt. Testfall 16 ist
    // genau dafuer gebaut: seine Abtastpunkte liegen bei 45, 135, 225 und
    // 315 Grad, also alle bei +/-A/Wurzel(2). Der Sample-Peak liegt 3,01 dB
    // unter dem wahren Scheitel — und wer ihn meldet, ist um drei Dezibel zu
    // optimistisch.
    abschnitt ("Gegenbeleg: der Sample-Peak liegt bei Testfall 16 um 3 dB darunter");
    {
        const double samplePeak = db (sinusSamplePeak (0.50, 4, 45.0));
        const double truePeak   = db (sinusTruePeak (0.50, 4, 45.0));
        pruefe (std::abs (samplePeak - (-9.03)) < 0.05,
                "Sample-Peak von Testfall 16 liegt bei -9,03 dBFS",
                juce::String (samplePeak, 3) + " dBFS");
        pruefe (truePeak - samplePeak > 2.9,
                "der True Peak findet die drei Dezibel, die zwischen den Samples liegen",
                "Unterschied " + juce::String (truePeak - samplePeak, 3) + " dB");
    }

    // ── 3. Der Sampleraten-Sweep aus M-02 ────────────────────────────────
    //
    // Die EBU-Frequenzen sind an fs gebunden, also ist die normierte
    // Frequenz bei jeder Samplerate dieselbe und dieselbe Erwartung gilt.
    // Der Sweep prueft damit genau das, was M-02 "aequivalente generierte
    // Mehrsampleraten-Goldens" nennt: dass der Pfad nicht zufaellig bei
    // 48 kHz stimmt.
    abschnitt ("Sampleraten-Sweep: dieselbe Zusage bei 44,1 / 48 / 88,2 / 96 / 192 kHz");
    {
        const double raten[] = { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 };
        for (const double fs : raten)
        {
            double schlimmste = 0.0;
            for (const int teiler : { 4, 6, 8 })
            {
                TruePeakDetektor d;
                d.vorbereiten (fs);
                double groesster = 0.0;
                for (int i = 0; i < 20000; ++i)
                {
                    const double x = 0.5 * std::sin (kZweiPi * (double) i / (double) teiler + 0.7);
                    const double tp = d.tick (x, x);
                    if (i >= TruePeakDetektor::kTapsJePhase + 8)
                        groesster = std::max (groesster, tp);
                }
                schlimmste = std::max (schlimmste, std::abs (db (groesster) - db (0.5)));
            }
            pruefe (schlimmste <= 0.1,
                    "bei " + juce::String (fs / 1000.0, 1) + " kHz halten fs/4, fs/6 und fs/8 "
                    "die Toleranz",
                    "groesste Abweichung " + juce::String (schlimmste, 4) + " dB");
        }
    }

    // ── 4. DIE ENTSCHEIDUNG 8-FACH GEGEN 4-FACH, ALS ZAHL ────────────────
    //
    // M-02 verlangt, dass dieses Bein die Entscheidung MESSBAR trifft. Der
    // Restfehler der Ueberabtastung hat eine geschlossene Form: der wahre
    // Scheitel liegt hoechstens eine HALBE Stuetzstelle neben der naechsten,
    // also
    //
    //     Fehler(f) = 20*log10( cos( 2*pi * f/fs * 1/(2*Faktor) ) ).
    //
    // Bei 8-fach und fs/4 sind das -0,042 dB, bei 4-fach -0,17 dB. Die
    // erste Zahl haelt die +/-0,1 dB aus §49.3, die zweite nicht. GENAU DAS
    // ist die Begruendung fuer 8-fach, und sie steht als `constexpr` im
    // Produktpfad, nicht als Kommentar.
    abschnitt ("eight_times_oversampling_error_bound - Die 8-fach-Entscheidung: die Schranke, die 4-fach reissen wuerde");
    {
        const double schranke8 = TruePeakDetektor::interpolationsfehlerDb (0.25);
        pruefe (std::abs (schranke8) <= 0.1,
                "8-fach: die ausgerechnete Schranke bei fs/4 haelt die Toleranz aus §49.3",
                juce::String (schranke8, 4) + " dB");

        // Dieselbe Formel mit Faktor 4 — von Hand, weil der Produktpfad den
        // Faktor 4 gar nicht kennt. Sie ist der Gegenbeleg: ohne sie waere
        // "8-fach ist genug" eine Behauptung ueber eine Alternative, die nie
        // gerechnet wurde.
        const double schranke4 = 20.0 * std::log10 (std::cos (kZweiPi * 0.25 / 8.0));
        pruefe (std::abs (schranke4) > 0.1,
                "4-fach wuerde sie bei derselben Frequenz REISSEN - deshalb 8-fach",
                juce::String (schranke4, 4) + " dB");

        // Und die wirkliche Messung bleibt innerhalb der ausgerechneten
        // Schranke. Liefe sie darueber, waere entweder die Formel falsch
        // oder der Filter — und beides waere ein Befund.
        const double wirklich = db (sinusTruePeak (0.5, 4, 45.0)) - db (0.5);
        pruefe (wirklich >= schranke8 - 0.02,
                "die WIRKLICHE Abweichung bleibt innerhalb der ausgerechneten Schranke",
                "gemessen " + juce::String (wirklich, 4) + " dB, Schranke "
                + juce::String (schranke8, 4) + " dB");
    }

    // ── 5. Fail-closed und Zahlenraender ─────────────────────────────────
    abschnitt ("Raender: Stille, NaN, Gleichanteil und der ungeruestete Detektor");
    {
        TruePeakDetektor d;
        d.vorbereiten (48000.0);
        double groesster = 0.0;
        for (int i = 0; i < 1000; ++i)
            groesster = std::max (groesster, d.tick (0.0, 0.0));
        pruefe (groesster == 0.0, "Stille ergibt exakt 0, nicht einen Filterrest",
                juce::String (groesster, 12));

        // Ein NaN im Eingang darf weder durchkommen noch im Filterzustand
        // bleiben: der Riegel liegt VOR der Rechnung, wie in der Engine.
        d.zuruecksetzen();
        for (int i = 0; i < 40; ++i)
            (void) d.tick (0.5, 0.5);
        const double mitNan = d.tick (std::numeric_limits<double>::quiet_NaN(), 0.5);
        pruefe (std::isfinite (mitNan), "ein NaN im Eingang erzeugt keinen NaN im Ausgang",
                juce::String (mitNan, 6));
        double nachNan = 0.0;
        for (int i = 0; i < 200; ++i)
            nachNan = std::max (nachNan, d.tick (0.5, 0.5));
        pruefe (std::isfinite (nachNan) && nachNan > 0.4,
                "und er bleibt nicht im Filterzustand haengen - danach misst er wieder",
                juce::String (nachNan, 6));

        // Ein Gleichanteil ist der Test der PHASENWEISEN Normierung. Bei
        // Normierung nur der Gesamtsumme wuerde jede Phase leicht anders
        // verstaerken, aus der Konstanten wuerde eine mit 8*fs modulierte
        // Welle, und ihr Maximum laege ueber dem Gleichanteil.
        d.zuruecksetzen();
        double dc = 0.0;
        for (int i = 0; i < 500; ++i)
        {
            const double tp = d.tick (0.5, 0.5);
            if (i >= TruePeakDetektor::kTapsJePhase + 8)
                dc = std::max (dc, tp);
        }
        pruefe (std::abs (dc - 0.5) < 1e-9,
                "ein Gleichanteil bleibt exakt er selbst - jede Phase hat DC-Verstaerkung 1",
                "gemessen " + juce::String (dc, 12) + ", Abweichung "
                + juce::String (std::abs (dc - 0.5), 12));

        // Fail-closed: ohne `vorbereiten()` gibt es keinen Verlauf, und der
        // Detektor liefert 0 statt in einen leeren Vektor zu greifen.
        TruePeakDetektor roh;
        pruefe (! roh.bereit() && roh.tick (0.9, 0.9) == 0.0,
                "ohne vorbereiten() misst er nicht, statt zu raten oder zu stuerzen");
    }

    // ── 6. Der Vertrag der Ueberabtastung selbst ─────────────────────────
    abschnitt ("Der Interpolator: Faktor, Taps und die Zusage ueber den Nulldurchgang");
    {
        pruefe (TruePeakDetektor::kFaktor == 8,
                "Ueberabtastfaktor ist 8 (BS.1770-4 Annex 2 verlangt mindestens 4)");
        pruefe (TruePeakDetektor::kTaps == TruePeakDetektor::kFaktor
                                         * TruePeakDetektor::kTapsJePhase + 1,
                "die Filterlaenge ist ungerade und zerlegt sich glatt in die Phasen",
                juce::String (TruePeakDetektor::kTaps) + " Taps, "
                + juce::String (TruePeakDetektor::kTapsJePhase) + " je Phase");

        // Jede Phase summiert sich auf 1 — die Zusage, die den Gleichanteil
        // oben traegt. Sie wird hier an den Koeffizienten selbst gemessen,
        // damit ein Fehler in der Normierung nicht erst am Signal auffaellt.
        const double* h = TruePeakDetektor::koeffizienten();
        double groessteAbweichung = 0.0;
        for (int phase = 0; phase < TruePeakDetektor::kFaktor; ++phase)
        {
            double summe = 0.0;
            for (int k = 0; phase + TruePeakDetektor::kFaktor * k < TruePeakDetektor::kTaps; ++k)
                summe += h[phase + TruePeakDetektor::kFaktor * k];
            groessteAbweichung = std::max (groessteAbweichung, std::abs (summe - 1.0));
        }
        pruefe (groessteAbweichung < 1e-12,
                "JEDE der acht Phasen summiert sich auf 1 - nicht nur die Gesamtsumme auf 8",
                "groesste Abweichung " + juce::String (groessteAbweichung, 15));
    }

    // ── 7. Die transienten EBU-Faelle 20 bis 23 (NAK-283 N01, M-65) ──────
    //
    // Reihenfolge: erst beweist sich die Referenz (7a), dann ist der
    // Tiefpass gemessen (7b), dann misst jeder Fall zweimal (7c). Eine
    // Referenz, die 7a nicht besteht, traegt in 7c keinen Vergleich - der
    // Referenzvergleich faellt dann, statt gegen eine ungepruefte Zahl zu
    // bestehen.
    abschnitt ("true_peak_matches_ebu_transient_set - EBU Tech 3341, Testfaelle 20 bis 23 (transient, Versatz 0 bis 3): +/-0,1 dB gegen die validierte 64-fach-Referenz UND 0,0 +0,2/-0,4 dBTP");
    {
        const auto kerne = referenzKerne();

        // 7a. Die Referenz an den analytisch bekannten Faellen 15 bis 19.
        bool referenzValidiert = true;
        {
            const struct { const char* name; double amplitude; int teiler; double phase; } faelle[] = {
                { "15", 0.50, 4, 0.0 },
                { "16", 0.50, 4, 45.0 },
                { "17", 0.50, 6, 60.0 },
                { "18", 0.50, 8, 67.5 },
                { "19", 1.41, 4, 45.0 },
            };
            for (const auto& f : faelle)
            {
                std::vector<double> x (4096);
                for (size_t i = 0; i < x.size(); ++i)
                    x[i] = f.amplitude * std::sin (kZweiPi * (double) i / (double) f.teiler
                                                   + f.phase * kZweiPi / 360.0);
                const double referenz = db (referenzTruePeak (x, kerne).maximum);
                const double abstand = referenz - db (f.amplitude);
                const bool ok = std::abs (abstand) <= 0.01;
                referenzValidiert = referenzValidiert && ok;
                pruefe (ok,
                        juce::String ("Referenz an Testfall ") + f.name
                            + ": die 64-fach-Referenz trifft die Amplitude innerhalb +/-0,01 dB",
                        "Referenz " + juce::String (referenz, 6) + " dBTP, erwartet "
                            + juce::String (db (f.amplitude), 6) + ", Abstand "
                            + juce::String (abstand, 6) + " dB");
            }
        }

        // 7b. Der Anti-Aliasing-Tiefpass, gemessen statt behauptet: flacher
        // Durchlass bis 0,4*fs, Sperrbereich ab 0,5*fs bis 2*fs (die Nyquist-
        // frequenz von 4*fs) - sonst faltete die Heruntertastung zurueck.
        const auto h = aaKoeffizienten();
        double durchlass = 0.0;
        for (int s = 0; s <= 400; ++s)
            durchlass = std::max (durchlass, std::abs (aaGangDb (h, 0.4 * (double) s / 400.0)));
        double sperre = -400.0;
        for (int s = 0; s <= 1500; ++s)
            sperre = std::max (sperre, aaGangDb (h, 0.5 + 1.5 * (double) s / 1500.0));
        pruefe (durchlass <= 0.001,
                "Anti-Aliasing-Tiefpass (257 Taps, Kaiser 10, 6 dB bei 0,45*fs): Durchlass bis 0,4*fs innerhalb 0,001 dB",
                "groesste Abweichung " + juce::String (durchlass, 6) + " dB");
        pruefe (sperre <= -90.0,
                "Anti-Aliasing-Tiefpass: Sperrbereich 0,5*fs bis 2*fs unter -90 dB",
                "hoechster Wert " + juce::String (sperre, 2) + " dB");

        // 7c. Die vier Faelle.
        const auto gefiltert = aaGefiltert (ebuTransientBei4fs(), h);
        pruefe (3 + 4 * (kEbuLaenge - 1) == (int) gefiltert.size() - 1,
                "Versatz 3 greift beim letzten fs-Sample genau den letzten 4*fs-Index, nicht darueber",
                juce::String (3 + 4 * (kEbuLaenge - 1)) + " von " + juce::String ((int) gefiltert.size() - 1));
        for (int versatz = 0; versatz < 4; ++versatz)
        {
            const auto y = dezimiert (gefiltert, versatz);
            const auto referenz = referenzTruePeak (y, kerne, &gefiltert, versatz);
            const double referenzDb = db (referenz.maximum);
            const double gemessen = db (detektorTruePeak (y));
            double samplePeak = 0.0;
            for (const double v : y)
                samplePeak = std::max (samplePeak, std::abs (v));
            const double abstand = gemessen - referenzDb;
            const juce::String fall = "Testfall " + juce::String (20 + versatz)
                                      + " (Versatz " + juce::String (versatz) + ")";

            pruefe (referenz.abweichung4fs >= 0.0 && referenz.abweichung4fs <= 1e-4,
                    fall + ": die Referenz gleicht an den 4*fs-Stellen dem synthetisierten Signal",
                    "groesste Abweichung " + juce::String (referenz.abweichung4fs, 9));
            pruefe (referenzValidiert && std::abs (abstand) <= 0.1,
                    "true_peak_matches_ebu_transient_set " + fall
                        + ": True Peak trifft die validierte Referenz innerhalb +/-0,1 dB",
                    "gemessen " + juce::String (gemessen, 5) + " dBTP, Referenz "
                        + juce::String (referenzDb, 5) + " dBTP, Abstand " + juce::String (abstand, 5)
                        + " dB, Sample-Peak " + juce::String (db (samplePeak), 4) + " dBFS"
                        + (referenzValidiert ? "" : ", REFERENZ NICHT VALIDIERT"));
            pruefe (gemessen >= -0.4 && gemessen <= 0.2,
                    "true_peak_matches_ebu_transient_set " + fall
                        + ": True Peak liegt in der Normtoleranz 0,0 +0,2/-0,4 dBTP",
                    "gemessen " + juce::String (gemessen, 5) + " dBTP, Normabstand "
                        + juce::String (gemessen - 0.0, 5) + " dB");
        }
    }

    std::cout << "\n-----------------------------------------" << std::endl;
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
