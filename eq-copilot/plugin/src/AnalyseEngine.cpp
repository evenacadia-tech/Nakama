// M1-Messkern — Implementierung. Maßstab der Zahlen: tools/analyze-track.py
// (eingefroren in fixtures/golden-referenz.json); Produktregeln: Plan §5.10.
#include "AnalyseEngine.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <limits>

namespace eqcop
{

// ── Startkalibration kMetricsVersion (Änderung NUR über neue Version) ───────
static constexpr double kAktivSchwelleDb     = -60.0;  // §5.10.2 Kurzzeit-RMS-Boden.
                                                       // Der adaptive Teil (Teppich+12)
                                                       // ist V1-ausgesetzt — Kalibration M3.
static constexpr double kBodenDb             = -90.0;  // absoluter Bandteppich (analyze-Skala)
static constexpr double kMindestAktivS       = 15.0;   // §5.10.2 Mindestmaterial
static constexpr double kExcessSchwelleDb    = 6.0;    // §5.10.3 Kandidat
static constexpr double kPersistenzDauerhaft = 0.50;
static constexpr double kPersistenzZeitweise = 0.15;
static constexpr double kAbdeckungBelastbar  = 0.60;   // §5.10.2 Klassen
static constexpr double kAbdeckungMindestens = 0.25;
static constexpr double kBreiteMaxOktaven    = 1.0 / 6.0 + 1e-9;  // §5.10.3
static constexpr int    kBasisMedianBaender  = 6;      // ±6 ⇒ 13 Bänder ≈ 1/2 Okt Median-Hüllkurve (m4)
static constexpr int    kPeakMindestAbstand  = 6;      // 1/4 Okt (wie analyze bpo//4)
static constexpr int    kMaxKandidaten       = 6;
static constexpr double kKonvergenzFensterS  = 10.0;   // „Kurve steht": Vergleichsfenster (AKTIVzeit)
static constexpr int    kZonenTickZellen     = 10;     // 10 × 100 ms = 1 s Aktivzeit pro Zonen-Tick
// m4.1 (Kalibrier-Runde 1): eine Zonen-Region ohne echten Pegel liefert keine
// Zeitverlaufs-Evidenz — unter Teppich+10 dB ist die Schulterlinien-Geometrie
// Rauschmathematik (Testtrack: „Haerte 50 %" bei 0,24 % Energie ueber 2 kHz).
static constexpr double kZonenTickMindestDb  = kBodenDb + 10.0;

static double sicheresDb (double v) { return 20.0 * std::log10 (std::max (v, 1e-12)); }

// m4 (Benchmark-Studie): robuste Basislinie als MEDIAN über ±kBasisMedianBaender
// Bänder ('nearest'-Klemmen an den Bereichsrändern, nur endliche Werte).
// Das frühere arithmetische Mittel wurde vom Peak selbst hochgezogen und
// unterschätzte die Prominenz — der Median ignoriert bis zu 6 der 13 Werte
// (die maximal zulässige Resonanzbreite §5.10.3 füllt höchstens ~5).
static double medianUmBand (const double* dbWerte, int b, int von, int bis)
{
    static_assert (kBasisMedianBaender == 6, "Fenstergröße unten mitziehen");
    double werte[2 * kBasisMedianBaender + 1];
    int n = 0;
    for (int j = -kBasisMedianBaender; j <= kBasisMedianBaender; ++j)
    {
        const double v = dbWerte[juce::jlimit (von, bis - 1, b + j)];
        if (std::isfinite (v))
            werte[n++] = v;
    }
    if (n < 5)
        return std::numeric_limits<double>::quiet_NaN();
    std::nth_element (werte, werte + n / 2, werte + n);
    double med = werte[n / 2];
    if ((n & 1) == 0)
    {
        std::nth_element (werte, werte + n / 2 - 1, werte + n / 2);
        med = 0.5 * (med + werte[n / 2 - 1]);
    }
    return med;
}

// ─────────────────────────────────────────────────────────────── WelchStufe ──
void AnalyseEngine::WelchStufe::init (int ordnung, double samplerate,
                                      const std::array<double, kLtasBaender>& kantenLo,
                                      const std::array<double, kLtasBaender>& kantenHi)
{
    n = 1 << ordnung;
    hop = n / 2;
    fs = samplerate;
    gefuellt = 0;
    aktiveSegmente = 0;
    ringL.assign ((size_t) n, 0.0f);
    ringR.assign ((size_t) n, 0.0f);
    fftDaten.assign ((size_t) n * 2, 0.0f);
    summePsd.assign ((size_t) n / 2 + 1, 0.0);
    fft = std::make_unique<juce::dsp::FFT> (ordnung);

    fenster.resize ((size_t) n);
    double summeW2 = 0.0;
    for (int i = 0; i < n; ++i)
    {
        // Hann PERIODISCH — wie scipy.signal.welch (sym=False).
        const double w = 0.5 - 0.5 * std::cos (2.0 * juce::MathConstants<double>::pi * i / n);
        fenster[(size_t) i] = (float) w;
        summeW2 += w * w;
    }
    dichteFaktor = 2.0 / (fs * summeW2);

    // Bin-Fenster je 1/24-Band: [lo,hi) über rfft-Frequenzen k·fs/n.
    for (int b = 0; b < kLtasBaender; ++b)
    {
        const int von = (int) std::ceil (kantenLo[(size_t) b] * n / fs);
        int bis = (int) std::ceil (kantenHi[(size_t) b] * n / fs);
        bis = std::min (bis, n / 2 + 1);
        bandBinVon[(size_t) b] = von;
        bandBinBis[(size_t) b] = std::max (von, bis);
    }
}

bool AnalyseEngine::WelchStufe::schiebe (float l, float r, bool stereo,
                                         std::vector<double>& einzelPsd, bool& warAktiv)
{
    ringL[(size_t) gefuellt] = l;
    ringR[(size_t) gefuellt] = r;
    if (++gefuellt < n)
        return false;

    // Aktiv-Gate über die mittlere KANALenergie — Side-Material zählt mit
    // (der alte Mono-Mix 0.5·(L+R) löschte Antiphase komplett aus, Paket B).
    double energie = 0.0;
    for (int i = 0; i < n; ++i)
        energie += (double) ringL[(size_t) i] * ringL[(size_t) i];
    if (stereo)
    {
        for (int i = 0; i < n; ++i)
            energie += (double) ringR[(size_t) i] * ringR[(size_t) i];
        energie *= 0.5;
    }
    const double rmsDb = 10.0 * std::log10 (energie / n + 1e-30);
    warAktiv = rmsDb > kAktivSchwelleDb;

    // PSD je Kanal, dann Leistungsmittel (PSD_L+PSD_R)/2 — Auslöschung im
    // Summensignal kann so keine Energie mehr verstecken. Bei L==R ist das
    // bitidentisch zum alten Mid-Spektrum (Kreuzvalidierungs-Anker).
    einzelPsd.resize ((size_t) n / 2 + 1);
    auto kanalPsd = [this] (const std::vector<float>& ring, auto&& uebernimm)
    {
        for (int i = 0; i < n; ++i)
            fftDaten[(size_t) i] = ring[(size_t) i] * fenster[(size_t) i];
        std::fill (fftDaten.begin() + n, fftDaten.end(), 0.0f);
        fft->performRealOnlyForwardTransform (fftDaten.data(), true);
        for (int k = 0; k <= n / 2; ++k)
        {
            const double re = fftDaten[(size_t) k * 2];
            const double im = fftDaten[(size_t) k * 2 + 1];
            double p = (re * re + im * im) * dichteFaktor;
            if (k == 0 || k == n / 2)
                p *= 0.5;                  // einseitiges Spektrum: Ränder ohne Faktor 2
            uebernimm (k, p);
        }
    };
    kanalPsd (ringL, [&] (int k, double p) { einzelPsd[(size_t) k] = p; });
    if (stereo)
        kanalPsd (ringR, [&] (int k, double p)
                  { einzelPsd[(size_t) k] = 0.5 * (einzelPsd[(size_t) k] + p); });

    if (warAktiv)
    {
        ++aktiveSegmente;
        for (size_t k = 0; k < einzelPsd.size(); ++k)
            summePsd[k] += einzelPsd[k];
    }

    std::memmove (ringL.data(), ringL.data() + hop, (size_t) (n - hop) * sizeof (float));
    std::memmove (ringR.data(), ringR.data() + hop, (size_t) (n - hop) * sizeof (float));
    gefuellt = n - hop;
    return true;
}

// ───────────────────────────────────────────────────────────── AnalyseEngine ──
AnalyseEngine::AnalyseEngine()
{
    pegelHistogramm.assign ((size_t) kLtasBaender * kHistStufen, 0);
    teppichInaktivDb.fill (1e9);
}

void AnalyseEngine::vorbereiten (double samplerate)
{
    if (! std::isfinite (samplerate) || samplerate <= 0.0 || samplerate > 768000.0
        || std::abs (samplerate - sr) < 0.5)
        return;    // gleiche Rate ⇒ Messung NICHT wegwerfen (FL ruft prepare mehrfach)
    sr = samplerate;

    // 1/24-Okt-Raster — identisch zu analyze-track _log_spectrum(): Kanten
    // 30·2^(k/24), Zentren geometrisch. fmax bleibt 18 kHz (< 0.95·fs/2 ab 38 kHz).
    for (int b = 0; b < kLtasBaender; ++b)
    {
        kantenLoHz[(size_t) b] = kLtasFmin * std::pow (2.0, b / (double) kLtasBpo);
        kantenHiHz[(size_t) b] = kLtasFmin * std::pow (2.0, (b + 1) / (double) kLtasBpo);
        zentrenHz[(size_t) b] = std::sqrt (kantenLoHz[(size_t) b] * kantenHiHz[(size_t) b]);
    }
    auto ersterAb = [this] (double f)
    {
        for (int b = 0; b < kLtasBaender; ++b)
            if (zentrenHz[(size_t) b] >= f)
                return b;
        return kLtasBaender;
    };
    idxZust200  = ersterAb (200.0);
    idxZust2000 = ersterAb (2000.0);
    idxNaht160  = ersterAb (160.0);
    idxNaht250  = ersterAb (250.0);
    idxNaht1600 = ersterAb (1600.0);
    idxNaht2500 = ersterAb (2500.0);

    // Messbarer LTAS-Bereich (Übergabe-Paket D): oberhalb min(18 kHz,
    // 0.95·Nyquist) gibt es keine Bins — dort bleibt die Kurve NaN, statt den
    // letzten Randwert als erfundene Evidenz fortzuschreiben. Bei fs ≥ 44,1 kHz
    // liegt die Grenze über 18 kHz ⇒ alle 221 Bänder bleiben messbar.
    {
        const double grenzeHz = std::min (kLtasFmax, 0.95 * sr / 2.0);
        ltasBisBand = 0;
        for (int b = 0; b < kLtasBaender; ++b)
            if (zentrenHz[(size_t) b] <= grenzeHz)
                ltasBisBand = b + 1;
    }

    bass.init (14, sr, kantenLoHz, kantenHiHz);       // 16384 — §5.10.1 Bassfenster
    referenz.init (13, sr, kantenLoHz, kantenHiHz);   // 8192 — analyze-track-Achse
    mitten.init (12, sr, kantenLoHz, kantenHiHz);     // 4096
    hoehen.init (11, sr, kantenLoHz, kantenHiHz);     // 2048

    // K-Weighting wie pyloudnorm (RBJ, KEINE DeMan-Variante): High-Shelf
    // G=+4 dB, Q=1/√2, fc=1500 · Hochpass G=0, Q=0.5, fc=38.
    //
    // SONDE-009: die zwei Filterentwürfe standen bis S12-13 als Lambdas hier.
    // Sie liegen jetzt in `core/analysis/KGewichtung.h`, weil die FeatureEngine
    // v2 dieselbe Kette braucht — Ausdruck für Ausdruck übernommen, damit die
    // Gleitkomma-Reihenfolge dieselbe bleibt. Der Beweis dafür ist der
    // GoldenTest: er misst die Lautheit gegen die `analyze-track`-Referenz und
    // fiele bei einem einzigen abweichenden Bit.
    shelfL = nakama::analyse::rbjHighShelf (sr, nakama::analyse::kShelfGainDb,
                                            nakama::analyse::shelfQ(),
                                            nakama::analyse::kShelfFreqHz);
    shelfR = shelfL;
    hpL = nakama::analyse::rbjHighpass (sr, nakama::analyse::kHochpassQ,
                                        nakama::analyse::kHochpassFreqHz);
    hpR = hpL;

    // True-Peak-FIR: 8×, Kaiser β=5.0, 161 Taps — scipy resample_poly-gleich
    // (firwin cutoff 1/8, Σh=1-Normierung, dann ·8).
    {
        auto besselI0 = [] (double x)
        {
            const double t = x * x / 4.0;
            double summe = 1.0, term = 1.0;
            for (int k = 1; k < 40; ++k)
            {
                term *= t / (k * (double) k);
                summe += term;
                if (term < 1e-18 * summe)
                    break;
            }
            return summe;
        };
        const double beta = 5.0;
        const double i0b = besselI0 (beta);
        const int mitte = (kTpTaps - 1) / 2;                       // 80
        double summeH = 0.0;
        for (int i = 0; i < kTpTaps; ++i)
        {
            const int m = i - mitte;
            const double ideal = m == 0
                ? 1.0 / kTpFaktor
                : std::sin (juce::MathConstants<double>::pi * m / kTpFaktor)
                    / (juce::MathConstants<double>::pi * m);
            const double r = (double) m / mitte;
            const double w = besselI0 (beta * std::sqrt (std::max (0.0, 1.0 - r * r))) / i0b;
            tpFir[(size_t) i] = ideal * w;
            summeH += tpFir[(size_t) i];
        }
        for (auto& h : tpFir)
            h = h / summeH * kTpFaktor;
    }
    tpVerlaufL.assign (64, 0.0f);
    tpVerlaufR.assign (64, 0.0f);

    zellenSamples = (int) std::lround (0.1 * sr);
    // SONDE-008: der einzige Ort, an dem der Loudness-Akku Speicher holt.
    // Danach ist sein Bedarf konstant (LoudnessAccumulator::speicherBytes()).
    loudness.vorbereiten (sr);
    zuruecksetzen();
}

void AnalyseEngine::zuruecksetzen()
{
    for (auto* s : { &bass, &referenz, &mitten, &hoehen })
    {
        s->gefuellt = 0;
        s->aktiveSegmente = 0;
        std::fill (s->summePsd.begin(), s->summePsd.end(), 0.0);
    }
    for (auto* a : { &bassAkku, &mittenAkku, &hoehenAkku, &referenzAkku })
    {
        a->summe.fill (0.0);
        a->segmente.fill (0);
        a->excessSegmente.fill (0);
    }
    std::fill (pegelHistogramm.begin(), pegelHistogramm.end(), 0u);
    teppichInaktivDb.fill (1e9);
    inaktiveSegmente = 0;
    liveEmaLinear.fill (std::numeric_limits<double>::quiet_NaN());

    // M3a-Zustand: Ticks + Konvergenz-Referenz. revisionZaehler bleibt
    // bewusst monoton (der Editor erkennt den Reset an der Wertänderung).
    tickLetzteAktiveZellen = 0;
    zonenTicksGesamt.fill (0);
    zonenTicksJenseits.fill (0);
    konvRefKurveDb.fill (std::numeric_limits<double>::quiet_NaN());
    konvRefAktivS = -1.0;
    konvLetzterMeanDb = std::numeric_limits<double>::quiet_NaN();
    konvLetzterMaxDb  = std::numeric_limits<double>::quiet_NaN();
    konvLetzterGueltig = false;

    shelfL.z1 = shelfL.z2 = shelfR.z1 = shelfR.z2 = 0.0;
    hpL.z1 = hpL.z2 = hpR.z1 = hpR.z2 = 0.0;
    zellenStand = 0;
    zelleKEnergie = zelleAktivEnergie = 0.0;
    loudness.zuruecksetzen();     // SONDE-008: leert Histogramm und Ring, ohne Allokation
    aktiveZellen = 0;

    std::fill (tpVerlaufL.begin(), tpVerlaufL.end(), 0.0f);
    std::fill (tpVerlaufR.begin(), tpVerlaufR.end(), 0.0f);
    truePeakLinear = 0.0;

    peakAbs = summeMid2 = summeSide2 = 0.0;
    summeL = summeR = summeL2 = summeR2 = summeLR = 0.0;
    samplesGesamt = 0;
    istStereo = false;
    nanErsetzt = 0;

    {
        std::lock_guard<std::mutex> l (snapMutex);
        fertig = MessSnapshot {};
        fertig.samplerate = sr;
        for (int b = 0; b < kLtasBaender; ++b)
        {
            fertig.ltasZentrenHz[(size_t) b] = zentrenHz[(size_t) b];
            fertig.ltasKompositDb[(size_t) b] = std::numeric_limits<double>::quiet_NaN();
            fertig.ltasReferenzDb[(size_t) b] = std::numeric_limits<double>::quiet_NaN();
            fertig.ltasLiveDb[(size_t) b] = std::numeric_limits<double>::quiet_NaN();
        }
    }
}

void AnalyseEngine::verarbeite (const float* interleaved, int frames, int kanaele)
{
    if (sr <= 0.0 || frames <= 0)
        return;
    const bool stereo = kanaele > 1;
    istStereo = istStereo || stereo;

    juce::uint64 nanLokal = 0;
    for (int i = 0; i < frames; ++i)
    {
        // NaN/Inf-Riegel (Paket C §8.2): die ANALYSE ersetzt nicht-endliche
        // Samples durch Stille und zählt sie — Akkus, Biquads und EMA bleiben
        // damit beweisbar endlich. Das Audio fasst die Engine ohnehin nie an.
        double l = interleaved[(size_t) i * 2];
        if (! std::isfinite (l)) { l = 0.0; ++nanLokal; }
        double r;
        if (stereo)
        {
            r = interleaved[(size_t) i * 2 + 1];
            if (! std::isfinite (r)) { r = 0.0; ++nanLokal; }
        }
        else
            r = l;   // Mono: R ist per FIFO-Vertrag das L-Duplikat

        const double mid = 0.5 * (l + r);
        const double side = 0.5 * (l - r);

        peakAbs = std::max ({ peakAbs, std::abs (l), std::abs (r) });
        summeMid2 += mid * mid;
        summeSide2 += side * side;
        summeL += l;   summeR += r;
        summeL2 += l * l; summeR2 += r * r; summeLR += l * r;
        ++samplesGesamt;

        // K-Kette (Shelf → Hochpass, pyloudnorm-Reihenfolge) — Mono nutzt nur L.
        {
            const double kl = hpL.tick (shelfL.tick (l));
            zelleKEnergie += kl * kl;
            if (stereo)
            {
                const double kr = hpR.tick (shelfR.tick (r));
                zelleKEnergie += kr * kr;
            }
        }
        // Aktivität §5.10.2 über die mittlere KANALenergie — der alte Mid-Mix
        // meldete Antiphase-Material als „still" (Paket B).
        zelleAktivEnergie += stereo ? 0.5 * (l * l + r * r) : l * l;
        if (++zellenStand >= zellenSamples)
        {
            loudness.zelle (zelleKEnergie);     // SONDE-008: fester Speicher statt push_back
            if (10.0 * std::log10 (zelleAktivEnergie / zellenSamples + 1e-30) > kAktivSchwelleDb)
                ++aktiveZellen;
            zelleKEnergie = zelleAktivEnergie = 0.0;
            zellenStand = 0;

            // M3a: Zonen-Zeitverlauf — ein Tick je 1 s AKTIVER Zeit. In
            // Pausen vergeht keine Tick-Zeit (die EMA fröre sonst den letzten
            // Stand ein und zählte ihn mehrfach).
            if (aktiveZellen - tickLetzteAktiveZellen >= (juce::uint64) kZonenTickZellen)
            {
                tickLetzteAktiveZellen = aktiveZellen;
                zonenTick();
            }
        }

        // True Peak: 8×-Polyphase je Kanal.
        {
            const size_t pos = (size_t) (samplesGesamt - 1) & 63;
            tpVerlaufL[pos] = (float) l;
            tpVerlaufR[pos] = (float) r;
            for (int phase = 0; phase < kTpFaktor; ++phase)
            {
                double accL = 0.0, accR = 0.0;
                for (int k = 0; phase + kTpFaktor * k < kTpTaps; ++k)
                {
                    const double h = tpFir[(size_t) (phase + kTpFaktor * k)];
                    const size_t idx = (pos - (size_t) k) & 63;
                    accL += h * tpVerlaufL[idx];
                    accR += h * tpVerlaufR[idx];
                }
                truePeakLinear = std::max ({ truePeakLinear, std::abs (accL), std::abs (accR) });
            }
        }

        // Welch-Stufen — je Kanal, Leistungsmittel entsteht in schiebe().
        const float fl = (float) l;
        const float fr = (float) r;
        bool aktiv = false;
        if (bass.schiebe (fl, fr, stereo, scratchPsd, aktiv))
            segmentInBaender (bass, scratchPsd, aktiv, bassAkku, 0, idxNaht250, 0, idxZust200);
        if (referenz.schiebe (fl, fr, stereo, scratchPsd, aktiv))
            segmentInBaender (referenz, scratchPsd, aktiv, referenzAkku, 0, kLtasBaender, -1, -1);
        if (mitten.schiebe (fl, fr, stereo, scratchPsd, aktiv))
            segmentInBaender (mitten, scratchPsd, aktiv, mittenAkku,
                              idxNaht160, idxNaht2500, idxZust200, idxZust2000);
        if (hoehen.schiebe (fl, fr, stereo, scratchPsd, aktiv))
            segmentInBaender (hoehen, scratchPsd, aktiv, hoehenAkku,
                              idxNaht1600, kLtasBaender, idxZust2000, kLtasBaender);
    }
    nanErsetzt += nanLokal;
}

void AnalyseEngine::segmentInBaender (const WelchStufe& stufe, const std::vector<double>& einzelPsd,
                                      bool aktiv, BandAkku& akku,
                                      int akkuVon, int akkuBis, int zustVon, int zustBis)
{
    // Band-Mitteldichten dieses Segments (NaN = keine Bins im Band).
    std::array<double, kLtasBaender> dichte {};
    std::array<double, kLtasBaender> dbWert {};
    dbWert.fill (std::numeric_limits<double>::quiet_NaN());   // m4: NaN-ehrlich für den Median
    const double skala = 1.5 * stufe.fs / 8192.0;   // ENBW der Referenzachse ⇒ analyze-dB
    for (int b = akkuVon; b < akkuBis; ++b)
    {
        const int von = stufe.bandBinVon[(size_t) b];
        const int bis = stufe.bandBinBis[(size_t) b];
        if (bis <= von)
        {
            dichte[(size_t) b] = std::numeric_limits<double>::quiet_NaN();
            continue;
        }
        double s = 0.0;
        for (int k = von; k < bis; ++k)
            s += einzelPsd[(size_t) k];
        dichte[(size_t) b] = s / (bis - von);
        dbWert[(size_t) b] = 10.0 * std::log10 (dichte[(size_t) b] * skala + 1e-30);
    }

    if (! aktiv)
    {
        // Nur Teppich-Tracking (§5.10.2) im Zuständigkeitsbereich.
        if (zustVon >= 0)
            for (int b = zustVon; b < zustBis; ++b)
                if (! std::isnan (dichte[(size_t) b]))
                    teppichInaktivDb[(size_t) b] = std::min (teppichInaktivDb[(size_t) b], dbWert[(size_t) b]);
        if (&stufe == &mitten)
            ++inaktiveSegmente;
        return;
    }

    for (int b = akkuVon; b < akkuBis; ++b)
    {
        if (std::isnan (dichte[(size_t) b]))
            continue;
        akku.summe[(size_t) b] += dichte[(size_t) b];
        ++akku.segmente[(size_t) b];
    }

    if (zustVon < 0)
        return;   // Referenzachse: nur Kurve, keine Produktregeln

    // Persistenz §5.10.3: excess gegen die MEDIAN-Hüllkurve DIESES Segments
    // (m4 — gleiche Basislinie wie der Kandidaten-Detektor, sonst liefen
    // Persistenz und Kandidat auf verschiedenen Maßstäben), Fenster über den
    // Akku-Bereich (Naht liefert den Randkontext), 'nearest'.
    for (int b = zustVon; b < zustBis; ++b)
    {
        if (std::isnan (dichte[(size_t) b]))
            continue;
        const double basis = medianUmBand (dbWert.data(), b, akkuVon, akkuBis);
        if (std::isfinite (basis) && dbWert[(size_t) b] - basis >= kExcessSchwelleDb)
            ++akku.excessSegmente[(size_t) b];

        // Histogramm der Segment-Pegel (Abdeckung wird erst final ausgezählt).
        const int bin = juce::jlimit (0, kHistStufen - 1,
                                      (int) std::lround (dbWert[(size_t) b]) - kHistMinDb);
        ++pegelHistogramm[(size_t) b * kHistStufen + (size_t) bin];

        // Live-Hüllkurve §5.10.1: 3-s-EMA über die LEISTUNG (nie über dB —
        // ein dB-Mittel ist ein geometrisches Mittel und fällt bei Musik mit
        // Pausen um zig dB unter die Welch-Kurve; in der FL-Probefahrt lag
        // die Live-Linie dadurch ~40 dB zu tief).
        const double alpha = 1.0 - std::exp (-((double) stufe.hop / stufe.fs) / 3.0);
        double& ema = liveEmaLinear[(size_t) b];
        ema = std::isnan (ema) ? dichte[(size_t) b]
                               : ema + alpha * (dichte[(size_t) b] - ema);
    }
}

// ── M3a: Zonen-Zeitverlauf-Tick (läuft in verarbeite, 1×/s Aktivzeit) ───────
void AnalyseEngine::zonenTick()
{
    if (sr <= 0.0)
        return;
    // dB-Sicht der Live-EMA — einmal pro Tick, gleiche Skala wie die Anzeige.
    const double skala = 1.5 * sr / 8192.0;
    std::array<double, kLtasBaender> live;
    live.fill (std::numeric_limits<double>::quiet_NaN());
    for (int b = 0; b < ltasBisBand; ++b)
    {
        const double v = liveEmaLinear[(size_t) b];
        if (! std::isnan (v))
            live[(size_t) b] = 10.0 * std::log10 (v * skala + 1e-30);
    }

    struct Region { double mittel = 0.0, logfMitte = 0.0; int belegt = 0, gesamt = 0; };
    auto region = [&] (double fLo, double fHi)
    {
        Region r;
        r.logfMitte = 0.5 * (std::log2 (fLo) + std::log2 (fHi));
        double summe = 0.0;
        for (int b = 0; b < kLtasBaender; ++b)
        {
            const double f = zentrenHz[(size_t) b];
            if (f < fLo || f >= fHi)
                continue;
            ++r.gesamt;
            const double v = live[(size_t) b];
            if (std::isfinite (v))
            {
                summe += v;
                ++r.belegt;
            }
        }
        if (r.belegt > 0)
            r.mittel = summe / r.belegt;
        return r;
    };
    auto belegtGenug = [] (const Region& r)
    {
        return r.gesamt > 0 && r.belegt >= 3
               && (double) r.belegt / (double) r.gesamt >= 0.6;
    };

    for (int z = 0; z < kZonenAnzahl; ++z)
    {
        const auto& g = kZonen[z];
        const auto zone = region (g.zoneVon, g.zoneBis);
        const auto lo = region (g.schulterLoVon, g.schulterLoBis);
        const bool mitLinie = g.schulterHiVon > 0.0;
        const auto hi = mitLinie ? region (g.schulterHiVon, g.schulterHiBis) : Region {};
        if (! belegtGenug (zone) || ! belegtGenug (lo) || (mitLinie && ! belegtGenug (hi)))
            continue;   // dieser Tick zählt für die Zone nicht (keine Evidenz)
        // m4.1: alle beteiligten Regionen brauchen ECHTEN Pegel — Teppich-
        // Geometrie erzeugt sonst Scheinevidenz (Kalibrier-Runde 1).
        if (zone.mittel < kZonenTickMindestDb || lo.mittel < kZonenTickMindestDb
            || (mitLinie && hi.mittel < kZonenTickMindestDb))
            continue;

        double delta;
        if (mitLinie)
        {
            // Schulterlinie wie in der Diagnose: log-f-linear zwischen den
            // Schulter-Mitteln, ausgewertet an der Zonen-Mitte.
            const double t = (zone.logfMitte - lo.logfMitte) / (hi.logfMitte - lo.logfMitte);
            delta = zone.mittel - (lo.mittel + t * (hi.mittel - lo.mittel));
        }
        else
            delta = zone.mittel - lo.mittel;   // Höhen-Hype: Air − Präsenz

        ++zonenTicksGesamt[(size_t) z];
        const bool jenseits = g.schwelleDb > 0.0 ? delta >= g.schwelleDb
                                                 : delta <= g.schwelleDb;
        if (jenseits)
            ++zonenTicksJenseits[(size_t) z];
    }
}

// ── Finalisierung (auswerten() im Worker-Takt) ──────────────────────────────
static void interpoliereLuecken (std::array<double, kLtasBaender>& kurve, int bisBand)
{
    // wie np.interp über den Band-INDEX; Ränder klemmen auf ersten/letzten Wert
    // — aber NUR innerhalb [0, bisBand): oberhalb der Nyquist-Kappe bleibt NaN
    // („nicht messbar"), nie ein fortgeschriebener Randwert (Paket D).
    int erster = -1, letzter = -1;
    for (int b = 0; b < bisBand; ++b)
        if (! std::isnan (kurve[(size_t) b]))
        {
            if (erster < 0) erster = b;
            letzter = b;
        }
    if (erster < 0)
        return;
    for (int b = 0; b < erster; ++b)
        kurve[(size_t) b] = kurve[(size_t) erster];
    for (int b = letzter + 1; b < bisBand; ++b)
        kurve[(size_t) b] = kurve[(size_t) letzter];
    int links = erster;
    for (int b = erster + 1; b <= letzter; ++b)
    {
        if (std::isnan (kurve[(size_t) b]))
            continue;
        for (int j = links + 1; j < b; ++j)
        {
            const double t = (double) (j - links) / (b - links);
            kurve[(size_t) j] = kurve[(size_t) links] * (1.0 - t) + kurve[(size_t) b] * t;
        }
        links = b;
    }
}

void AnalyseEngine::finalisiereLtas (MessSnapshot& s) const
{
    const double skala = 1.5 * sr / 8192.0;
    // Lücken (1/24-Bänder ohne FFT-Bin) werden wie in analyze-track in
    // LINEARER Leistung interpoliert, erst danach kommt die dB-Wandlung —
    // dB-Interpolation läge an steilen Ton-Flanken um Größenordnungen daneben.
    auto kurveVon = [&] (const BandAkku& a, int von, int bis)
    {
        std::array<double, kLtasBaender> k;
        k.fill (std::numeric_limits<double>::quiet_NaN());
        // Auch die Befüllung endet an der Nyquist-Kappe: ein Band, dessen
        // ZENTRUM oberhalb liegt, kann unterhalb Nyquist noch Rest-Bins haben
        // — ein Teilband-Pegel als Bandwert wäre trotzdem erfundene Evidenz.
        for (int b = von; b < std::min (bis, ltasBisBand); ++b)
            if (a.segmente[(size_t) b] > 0)
                k[(size_t) b] = a.summe[(size_t) b] / (double) a.segmente[(size_t) b] * skala;
        interpoliereLuecken (k, ltasBisBand);
        for (auto& v : k)
            if (! std::isnan (v))
                v = 10.0 * std::log10 (v + 1e-30);
        return k;
    };

    auto ref = kurveVon (referenzAkku, 0, kLtasBaender);
    bool hatWerte = false;
    for (const double v : ref)
        hatWerte = hatWerte || ! std::isnan (v);
    if (! hatWerte)
    {
        s.ltasGueltig = false;
        return;
    }
    s.ltasReferenzDb = ref;

    auto tief = kurveVon (bassAkku, 0, idxNaht250);      // klemmt außerhalb des
    auto mitte = kurveVon (mittenAkku, idxNaht160, idxNaht2500);  // Stufenbereichs —
    auto hoch = kurveVon (hoehenAkku, idxNaht1600, kLtasBaender); // Blend greift dort nie

    for (int b = 0; b < kLtasBaender; ++b)
    {
        const double f = zentrenHz[(size_t) b];
        double v;
        if (b < idxNaht160)
            v = tief[(size_t) b];
        else if (b < idxNaht250)   // Naht Bass→Mitten 160–250 Hz, log-f-Blend
        {
            const double t = std::log (f / 160.0) / std::log (250.0 / 160.0);
            v = tief[(size_t) b] * (1.0 - t) + mitte[(size_t) b] * t;
        }
        else if (b < idxNaht1600)
            v = mitte[(size_t) b];
        else if (b < idxNaht2500)  // Naht Mitten→Höhen 1.6–2.5 kHz
        {
            const double t = std::log (f / 1600.0) / std::log (2500.0 / 1600.0);
            v = mitte[(size_t) b] * (1.0 - t) + hoch[(size_t) b] * t;
        }
        else
            v = hoch[(size_t) b];
        s.ltasKompositDb[(size_t) b] = v;
        s.ltasZentrenHz[(size_t) b] = f;
    }
    s.ltasGueltig = true;
}

void AnalyseEngine::finalisiereLoudness (MessSnapshot& s) const
{
    // SONDE-008 (Entwurf §48.1): das Gating steckt jetzt im fixed-memory
    // `LoudnessAccumulator`. Bis 23.08. stand hier ein Zweitdurchgang über eine
    // frisch allozierte Blockliste über die GANZE Sessionlänge — 4×/s.
    // Gleiche Kette (Blöcke = 4 Zellen à 100 ms, Hop 1 Zelle, absolutes Gate
    // ">= −70", relatives Gate "> Γ_r und > −70"); der einzige Unterschied ist,
    // dass die AUSWAHL für das relative Gate aus einem Histogramm kommt statt
    // aus der Liste. Wieviel das höchstens ausmacht, sagt
    // `loudness.unsicherheitLu()`, und der EBU-Golden misst es (§49: ±0,1 LU).
    double lufs = 0.0;
    if (loudness.integriert (lufs))
    {
        s.lufsIntegriert = lufs;
        s.lufsGueltig = true;
    }
}

LautheitsTelemetrie AnalyseEngine::lautheitFuerTelemetrie() const noexcept
{
    LautheitsTelemetrie aus;
    double wert = 0.0;
    const bool integriert = loudness.integriert (wert);
    const double unsicherheit = loudness.unsicherheitLu();

    if (integriert && std::isfinite (wert) && std::isfinite (unsicherheit))
    {
        const float wireWert = static_cast<float> (wert);
        const float wireUnsicherheit = static_cast<float> (unsicherheit);
        if (std::isfinite (wireWert) && std::isfinite (wireUnsicherheit))
        {
            aus.paar = true;
            aus.lufsI = wireWert;
            aus.unsicherheitLu = wireUnsicherheit;
            return aus;
        }
    }

    // Ein vollstaendiger 400-ms-Block, der keinen integrierten Wert liefert,
    // ist gegatet (kein Signal ueber der gueltigen Auswahl). Vor dem ersten
    // solchen Block sowie bei einer nicht endlichen Konfidenz wird weiter
    // ehrlich gesammelt; es entsteht weder 0 LUFS noch ein halbes Paar.
    aus.status = (! integriert && loudness.bloeckeGesamt() > 0) ? 2 : 1;
    return aus;
}

// ── Leicht-Teil beider Auswertungen (m4): EINE Quelle für Zustand, Sekunden,
//    Live-Kurve und die schnellen Skalare — der 20-Hz-Pfad kann so nie von
//    der 250-ms-Auswertung abweichen. ───────────────────────────────────────
void AnalyseEngine::fuelleBasis (MessSnapshot& s) const
{
    s.samplerate = sr;
    s.verarbeiteteSamples = samplesGesamt;
    s.nanErsetzt = nanErsetzt;
    s.gesamtSekunden = (double) samplesGesamt / sr;

    const double aktivS = (double) aktiveZellen * 0.1;
    s.aktivSekunden = aktivS;
    s.zustand = samplesGesamt == 0        ? MessZustand::keineDaten
              : aktivS < kMindestAktivS   ? MessZustand::sammelt
                                          : MessZustand::messbereit;

    // Live-Kurve — endet an derselben Nyquist-Kappe wie das Komposit.
    {
        const double skala = 1.5 * sr / 8192.0;
        for (int b = 0; b < kLtasBaender; ++b)
        {
            const double v = liveEmaLinear[(size_t) b];
            s.ltasLiveDb[(size_t) b] = (b >= ltasBisBand || std::isnan (v))
                ? std::numeric_limits<double>::quiet_NaN()
                : 10.0 * std::log10 (v * skala + 1e-30);
        }
    }

    s.truePeakDb = truePeakLinear > 0.0 ? sicheresDb (truePeakLinear) : -240.0;
    if (samplesGesamt > 0)
    {
        // Crest gegen die mittlere KANALenergie, nicht den Mid-Mix: Antiphase
        // trägt hörbare Leistung — ihr Mid-RMS wäre −∞ (Paket B). Bei L==R ist
        // 0.5·(ΣL²+ΣR²) bitidentisch zum alten ΣMid².
        const double rms = std::sqrt (0.5 * (summeL2 + summeR2) / (double) samplesGesamt);
        s.crestDb = sicheresDb (peakAbs) - sicheresDb (rms);
        s.crestGueltig = true;
    }
    // SONDE-008: derselbe Wert aus dem 30-Zellen-Ring des Akkus — dieselbe
    // Reihenfolge (ältest → jüngst), dieselbe Assoziativität, also BITGLEICH
    // zur bisherigen Schleife über `kZellen[size-30 … size-1]`.
    double kurzLufs = 0.0;
    if (loudness.kurz (kurzLufs))
    {
        s.lufsShort = kurzLufs;
        s.lufsShortGueltig = true;
    }
}

void AnalyseEngine::auswertenLeicht()
{
    if (sr <= 0.0)
        return;
    std::lock_guard<std::mutex> l (snapMutex);
    // Ohne neue Samples keine Publikation: die Revision steht, der Editor
    // malt im Leerlauf nichts (FPS-Fix Teil 2 — Ruhe kostet nichts).
    if (fertig.verarbeiteteSamples == samplesGesamt)
        return;
    fuelleBasis (fertig);
    fertig.revision = ++revisionZaehler;
}

// ── M3a: Band-Perzentile aus dem Pegelhistogramm (1-dB-Quantisierung) ───────
void AnalyseEngine::berechnePerzentile (MessSnapshot& s) const
{
    s.perzentilP10.fill (std::numeric_limits<double>::quiet_NaN());
    s.perzentilP50.fill (std::numeric_limits<double>::quiet_NaN());
    s.perzentilP95.fill (std::numeric_limits<double>::quiet_NaN());
    bool irgendeins = false;
    for (int b = 0; b < kLtasBaender; ++b)
    {
        const auto* hist = pegelHistogramm.data() + (size_t) b * kHistStufen;
        juce::uint64 gesamt = 0;
        for (int bin = 0; bin < kHistStufen; ++bin)
            gesamt += hist[bin];
        if (gesamt == 0)
            continue;
        irgendeins = true;
        const juce::uint64 ziel10 = (juce::uint64) std::ceil (0.10 * (double) gesamt);
        const juce::uint64 ziel50 = (juce::uint64) std::ceil (0.50 * (double) gesamt);
        const juce::uint64 ziel95 = (juce::uint64) std::ceil (0.95 * (double) gesamt);
        juce::uint64 cum = 0;
        for (int bin = 0; bin < kHistStufen; ++bin)
        {
            cum += hist[bin];
            const double wert = (double) (kHistMinDb + bin);
            if (std::isnan (s.perzentilP10[(size_t) b]) && cum >= ziel10)
                s.perzentilP10[(size_t) b] = wert;
            if (std::isnan (s.perzentilP50[(size_t) b]) && cum >= ziel50)
                s.perzentilP50[(size_t) b] = wert;
            if (cum >= ziel95)
            {
                s.perzentilP95[(size_t) b] = wert;
                break;
            }
        }
    }
    s.perzentileGueltig = irgendeins;
}

void AnalyseEngine::finalisiereSkalar (MessSnapshot& s) const
{
    if (referenz.aktiveSegmente == 0)
        return;
    const int nBins = referenz.n / 2 + 1;
    std::vector<double> mittel ((size_t) nBins);
    for (int k = 0; k < nBins; ++k)
        mittel[(size_t) k] = referenz.summePsd[(size_t) k] / (double) referenz.aktiveSegmente;
    // analyze-track-Äquivalenz: dessen Skalar-Achse zählt ALLE |X|²-Bins gleich,
    // ohne die einseitige Rand-Halbierung. Pink trägt ~46 % seiner Energie unter
    // 20 Hz (1/f bis 0,05 Hz) — ohne diese Angleichung kippen die
    // Energie-VERHÄLTNISSE (low_frac/band_pct/rolloff) messbar.
    mittel[0] *= 2.0;
    mittel[(size_t) (nBins - 1)] *= 2.0;

    double gesamt = 0.0, magSumme = 0.0, fMagSumme = 0.0;
    for (int k = 0; k < nBins; ++k)
    {
        const double f = k * sr / referenz.n;
        const double mag = std::sqrt (mittel[(size_t) k]);
        gesamt += mittel[(size_t) k];
        magSumme += mag;
        fMagSumme += f * mag;
    }
    if (gesamt <= 0.0)
        return;
    s.centroidMagHz = fMagSumme / (magSumme + 1e-30);

    double cum = 0.0;
    const double ziel = 0.85 * gesamt;
    s.rolloffHz = (nBins - 1) * sr / referenz.n;
    for (int k = 0; k < nBins; ++k)
    {
        cum += mittel[(size_t) k];
        if (cum >= ziel)
        {
            s.rolloffHz = k * sr / referenz.n;
            break;
        }
    }

    double unter250 = 0.0;
    static constexpr double kBand8[9] = { 20, 60, 120, 250, 500, 2000, 6000, 12000, 20000 };
    std::array<double, 8> band {};
    for (int k = 0; k < nBins; ++k)
    {
        const double f = k * sr / referenz.n;
        if (f < 250.0)
            unter250 += mittel[(size_t) k];
        for (int b = 0; b < 8; ++b)
            if (f >= kBand8[b] && f < kBand8[b + 1])
            {
                band[(size_t) b] += mittel[(size_t) k];
                break;
            }
    }
    s.lowFrac = unter250 / gesamt;
    for (int b = 0; b < 8; ++b)
        s.band8Prozent[(size_t) b] = 100.0 * band[(size_t) b] / gesamt;

    // Flatness: geometrisches/arithmetisches Mittel pro Segment wäre exakt —
    // V1 rechnet sie auf der GEMITTELTEN Referenz-PSD. Eigene 48-kHz-Achse,
    // bewusst NICHT librosa-vergleichbar (22050-Achse) — nur Größenordnung.
    {
        double logSumme = 0.0, arith = 0.0;
        int nW = 0;
        for (int k = 1; k < nBins; ++k)
        {
            const double p = std::max (mittel[(size_t) k], 1e-30);
            logSumme += std::log (p);
            arith += p;
            ++nW;
        }
        if (nW > 0 && arith > 0.0)
            s.flatness = std::exp (logSumme / nW) / (arith / nW);
    }
    s.spektralGueltig = true;

    if (samplesGesamt > 0)
    {
        const double n = (double) samplesGesamt;
        s.width = (summeSide2 / n) / (summeMid2 / n + 1e-12);
        const double kovarianz = summeLR / n - (summeL / n) * (summeR / n);
        const double varL = summeL2 / n - (summeL / n) * (summeL / n);
        const double varR = summeR2 / n - (summeR / n) * (summeR / n);
        const double nenner = std::sqrt (std::max (varL, 0.0) * std::max (varR, 0.0));
        s.corr = nenner > 1e-20 ? juce::jlimit (-1.0, 1.0, kovarianz / nenner) : 1.0;
        s.stereoGueltig = istStereo;
    }
}

void AnalyseEngine::berechneAbdeckung (MessSnapshot& s) const
{
    s.abdeckung.clear();
    for (int start = 0; start < kLtasBaender; start += 8)   // 8·(1/24) = 1/3 Okt
    {
        const int ende = std::min (start + 8, kLtasBaender);
        double teppich = kBodenDb;
        double minInaktiv = 1e9;
        for (int b = start; b < ende; ++b)
            minInaktiv = std::min (minInaktiv, teppichInaktivDb[(size_t) b]);
        if (minInaktiv < 1e8)
            teppich = std::max (kBodenDb, minInaktiv);

        juce::uint64 aktivGesamt = 0, ueber = 0;
        const int schwellBin = juce::jlimit (0, kHistStufen - 1,
                                             (int) std::lround (teppich + kExcessSchwelleDb) - kHistMinDb);
        for (int b = start; b < ende; ++b)
            for (int bin = 0; bin < kHistStufen; ++bin)
            {
                const auto h = pegelHistogramm[(size_t) b * kHistStufen + (size_t) bin];
                aktivGesamt += h;
                if (bin > schwellBin)
                    ueber += h;
            }

        DrittelOktavAbdeckung a;
        a.zentrumHz = std::sqrt (kantenLoHz[(size_t) start] * kantenHiHz[(size_t) (ende - 1)]);
        a.anteil = aktivGesamt > 0 ? (double) ueber / (double) aktivGesamt : 0.0;
        a.klasse = aktivGesamt == 0                       ? AbdeckungsKlasse::nichtMessbar
                 : a.anteil >= kAbdeckungBelastbar        ? AbdeckungsKlasse::belastbar
                 : a.anteil >= kAbdeckungMindestens       ? AbdeckungsKlasse::eingeschraenkt
                                                          : AbdeckungsKlasse::nichtMessbar;
        s.abdeckung.push_back (a);
    }
}

void AnalyseEngine::findeResonanzen (MessSnapshot& s) const
{
    s.resonanzen.clear();
    if (! s.ltasGueltig || s.zustand != MessZustand::messbereit)
        return;   // §5.3: unter Mindestmaterial NIE ein Befund

    const auto& kurve = s.ltasKompositDb;
    // m4: MEDIAN-Hüllkurve statt Mittel — der Peak zog das Mittel hoch und
    // fraß sich so einen Teil der eigenen Prominenz weg (Benchmark-Studie,
    // RESO-Prinzip). NaN-fest: der Median nimmt nur endliche Nachbarn.
    std::array<double, kLtasBaender> glatt {};
    for (int b = 0; b < kLtasBaender; ++b)
        glatt[(size_t) b] = medianUmBand (kurve.data(), b, 0, kLtasBaender);
    std::array<double, kLtasBaender> excess {};
    for (int b = 0; b < kLtasBaender; ++b)
        // Nicht messbare Bänder → −∞-Ersatz: nie Kandidat, blockieren aber
        // auch kein echtes Maximum am Rand des messbaren Bereichs.
        excess[(size_t) b] = (std::isfinite (kurve[(size_t) b]) && std::isfinite (glatt[(size_t) b]))
            ? kurve[(size_t) b] - glatt[(size_t) b]
            : -1e18;

    struct Roh { int band; double excess; };
    std::vector<Roh> kandidaten;
    for (int b = 1; b < kLtasBaender - 1; ++b)
        if (excess[(size_t) b] >= kExcessSchwelleDb
            && excess[(size_t) b] >= excess[(size_t) b - 1]
            && excess[(size_t) b] >= excess[(size_t) b + 1])
            kandidaten.push_back ({ b, excess[(size_t) b] });
    std::sort (kandidaten.begin(), kandidaten.end(),
               [] (const Roh& a, const Roh& b) { return a.excess > b.excess; });

    std::vector<int> genommen;
    for (const auto& roh : kandidaten)
    {
        bool zuNah = false;
        for (const int g : genommen)
            zuNah = zuNah || std::abs (g - roh.band) < kPeakMindestAbstand;
        if (zuNah)
            continue;

        // Bandbreite: zusammenhängender Bereich mit excess ≥ Spitze − 6 dB.
        int von = roh.band, bis = roh.band;
        while (von > 0 && excess[(size_t) (von - 1)] >= roh.excess - 6.0)
            --von;
        while (bis < kLtasBaender - 1 && excess[(size_t) (bis + 1)] >= roh.excess - 6.0)
            ++bis;
        const double breite = (bis - von + 1) / (double) kLtasBpo;
        if (breite > kBreiteMaxOktaven)
            continue;   // zu breit für "Resonanz" — Balance-Thema, nicht §5.10.3

        // Persistenz aus der ZUSTÄNDIGEN Stufe (±1 Band gegen Raster-Wackeln).
        const BandAkku& akku = roh.band < idxZust200 ? bassAkku
                             : roh.band < idxZust2000 ? mittenAkku : hoehenAkku;
        double persistenz = 0.0;
        for (int b = std::max (0, roh.band - 1); b <= std::min (kLtasBaender - 1, roh.band + 1); ++b)
            if (akku.segmente[(size_t) b] > 0)
                persistenz = std::max (persistenz,
                                       (double) akku.excessSegmente[(size_t) b]
                                           / (double) akku.segmente[(size_t) b]);
        if (persistenz < kPersistenzZeitweise)
            continue;

        // Abdeckungs-Gate: das 1/3-Okt-Band der Spitze muss belastbar sein —
        // filtert Leakage-"Funde" in unbespielten Bändern (Sinus-Landmine).
        const int gruppe = roh.band / 8;
        if (gruppe >= (int) s.abdeckung.size()
            || s.abdeckung[(size_t) gruppe].klasse != AbdeckungsKlasse::belastbar)
            continue;

        ResonanzKandidat k;
        k.freqHz = zentrenHz[(size_t) roh.band];   // Band-Center — nie feiner als das Raster
        k.dbOver = roh.excess;
        k.persistenzAnteil = persistenz;
        k.breiteOktaven = breite;
        k.klasse = persistenz >= kPersistenzDauerhaft ? ResonanzKlasse::dauerhaft
                                                      : ResonanzKlasse::zeitweise;
        s.resonanzen.push_back (k);
        genommen.push_back (roh.band);
        if ((int) s.resonanzen.size() >= kMaxKandidaten)
            break;
    }
}

void AnalyseEngine::auswerten()
{
    if (sr <= 0.0)
        return;
    MessSnapshot s;
    for (int b = 0; b < kLtasBaender; ++b)
    {
        s.ltasZentrenHz[(size_t) b] = zentrenHz[(size_t) b];
        s.ltasKompositDb[(size_t) b] = std::numeric_limits<double>::quiet_NaN();
        s.ltasReferenzDb[(size_t) b] = std::numeric_limits<double>::quiet_NaN();
    }
    fuelleBasis (s);   // Zustand, Sekunden, Live-Kurve, TP/Crest/Kurz-LUFS

    finalisiereLoudness (s);
    finalisiereSkalar (s);
    finalisiereLtas (s);
    berechneAbdeckung (s);
    berechnePerzentile (s);
    findeResonanzen (s);

    // M3a: Zonen-Zeitverlauf in den Snapshot (Zähler laufen in verarbeite()).
    for (int z = 0; z < kZonenAnzahl; ++z)
    {
        const auto ticks = zonenTicksGesamt[(size_t) z];
        auto& ziel = s.zonenZeit[(size_t) z];
        ziel.ticks = ticks;
        ziel.gueltig = ticks > 0;
        ziel.anteil = ticks > 0 ? (double) zonenTicksJenseits[(size_t) z] / (double) ticks : 0.0;
    }

    // M3a: Konvergenz „Kurve steht" — Komposit gegen den Stand von vor
    // ~kKonvergenzFensterS AKTIVzeit (rein informativ, §5.10-Startwerte).
    if (s.ltasGueltig)
    {
        if (konvRefAktivS < 0.0)
        {
            konvRefKurveDb = s.ltasKompositDb;
            konvRefAktivS = s.aktivSekunden;
        }
        else if (s.aktivSekunden - konvRefAktivS >= kKonvergenzFensterS)
        {
            double summe = 0.0, maxD = 0.0;
            int n = 0;
            for (int b = 0; b < kLtasBaender; ++b)
            {
                const double a = s.ltasKompositDb[(size_t) b];
                const double r = konvRefKurveDb[(size_t) b];
                if (! std::isfinite (a) || ! std::isfinite (r))
                    continue;
                const double d = std::abs (a - r);
                summe += d;
                maxD = std::max (maxD, d);
                ++n;
            }
            if (n >= 20)
            {
                konvLetzterMeanDb = summe / n;
                konvLetzterMaxDb = maxD;
                konvLetzterGueltig = true;
            }
            konvRefKurveDb = s.ltasKompositDb;
            konvRefAktivS = s.aktivSekunden;
        }
    }
    s.konvergenzMeanDb = konvLetzterMeanDb;
    s.konvergenzMaxDb = konvLetzterMaxDb;
    s.konvergenzGueltig = konvLetzterGueltig;

    s.revision = ++revisionZaehler;
    {
        std::lock_guard<std::mutex> l (snapMutex);
        fertig = std::move (s);
    }
}

MessSnapshot AnalyseEngine::snapshot() const
{
    std::lock_guard<std::mutex> l (snapMutex);
    return fertig;
}

} // namespace eqcop
