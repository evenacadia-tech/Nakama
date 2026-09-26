#pragma once

#include "../core/analysis/Fft.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <system_error>
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

/** M-47 (a) (NAK-380 Nacharbeit 1, Befund D6): dasselbe Gauss-Weissrauschen
    wie `weissMono (saat, 1.0, ...)`, aber je Abschnitt [grenzen[i],
    grenzen[i+1]) EMPIRISCH auf die feste Rahmenenergie E_soll = 10^(sollDb[i]/10)
    normiert: die Stichprobenenergie E_ist = Summe x^2 / n der sigma-1-Werte
    wird in double gemessen und der Abschnitt mit sqrt(E_soll/E_ist)
    skaliert, danach als float32 gespeichert. Die Normierung stellt die
    Groesse selbst her; `sigma` waere nur die theoretische
    Standardabweichung, und die endliche Stichprobe wiche davon um rund
    sqrt(2/n) ab (240 000 Werte: 0,29 %, 0,013 dB). Gemessen wird die
    hergestellte Energie im Test. Falsche Grenzen (nicht aufsteigend, nicht
    eine mehr als Pegel, erste nicht 0) liefern einen leeren Puffer. */
inline std::vector<float> weissAbschnitteNormiert (std::uint64_t saat,
                                                   const std::vector<std::uint64_t>& grenzen,
                                                   const std::vector<double>& sollDb)
{
    if (grenzen.size() != sollDb.size() + 1u || grenzen.size() < 2u || grenzen.front() != 0u)
        return {};
    for (std::size_t a = 1; a < grenzen.size(); ++a)
        if (grenzen[a] <= grenzen[a - 1])
            return {};
    GaussRauschen g { saat };
    std::vector<double> roh ((std::size_t) grenzen.back());
    for (auto& v : roh)
        v = g.naechstes();
    std::vector<float> aus ((std::size_t) grenzen.back());
    for (std::size_t a = 0; a + 1 < grenzen.size(); ++a)
    {
        double summe = 0.0;
        for (auto i = grenzen[a]; i < grenzen[a + 1]; ++i)
            summe += roh[(std::size_t) i] * roh[(std::size_t) i];
        const double eIst = summe / (double) (grenzen[a + 1] - grenzen[a]);
        const double faktor = std::sqrt (std::pow (10.0, sollDb[a] / 10.0) / eIst);
        for (auto i = grenzen[a]; i < grenzen[a + 1]; ++i)
            aus[(std::size_t) i] = (float) (faktor * roh[(std::size_t) i]);
    }
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

namespace detail
{
/** Festkommatext ohne Locale (std::to_chars), fuer Meldungen aus dem Kopf. */
inline std::string festkomma (double v, int stellen)
{
    char puffer[64];
    const auto r = std::to_chars (puffer, puffer + sizeof (puffer), v, std::chars_format::fixed, stellen);
    return r.ec == std::errc() ? std::string (puffer, r.ptr) : std::string ("?");
}
} // namespace detail

/** E-380-13 als Kopffunktion des Erzeugers (NAK-380 Nacharbeit 1, Befund
    D4; Erzeugerbindung): jedes Programm ruft sie vor JEDEM Rosa-Nutzer am
    unveraenderten Puffer auf (vor Stille, Klicks oder Pegelstufen) und
    meldet das Ergebnis als eigenen Prueffall `380/<Fall>
    rosa_selbstpruefung_E-380-13`. Zwei Bedingungen:
    - Oktavbandleistung konstant: benachbarte Oktaven 31,25 Hz bis 16 kHz
      weichen hoechstens 1,0 dB ab (§7.1, der E-380-13-Wert selbst).
    - Pegel = Soll-RMS auf 0,5 dB. Herleitung: die Stichprobenleistung
      P = Summe x^2 / N eines Gaussprozesses mit Autokorrelation rho(k) hat
      die relative Varianz (2/N) * Summe_k rho(k)^2. Fuer das Kellet-Filter
      ist Summe_k rho(k)^2 = 96,04 (aus der Impulsantwort, alle Verschiebungen
      beider Richtungen; die Annahme "alles am langsamsten Pol" gaebe
      (1 + a^2)/(1 - a^2) = 877 und waere nur eine Schranke). Kuerzester
      Nutzer 240 000 Samples: sqrt(2*96,04/240 000) = 2,83 % = 0,121 dB, 0,5 dB
      sind 4,1 sigma; 1 440 000 Samples: 0,050 dB, 10 sigma. Die Normierung
      des Erzeugers ist analytisch (`leistungsverstaerkung`), der Erwartungswert
      also der Soll-Pegel.
    Rueckgabe wie bisher im B5-Helfer: Urteil und Meldung mit groesster
    Nachbardifferenz, den zehn Oktavpegeln und dem RMS. Nicht endliche oder
    leere Eingaben sind nie gruen (Vergleiche mit NaN sind falsch). */
struct RosaSelbstpruefung
{
    bool ok { false };
    double groessteDifferenzDb { 0.0 };
    double rmsIst { 0.0 };
    double abweichungDb { 0.0 };
    std::vector<double> oktavenDb;
    std::string meldung;
};

inline RosaSelbstpruefung rosaSelbstpruefung (const std::vector<float>& x, double rms,
                                              double fs = kSamplerate)
{
    RosaSelbstpruefung e;
    e.groessteDifferenzDb = rosaGroessteOktavdifferenzDb (x, fs, &e.oktavenDb);
    double summe = 0.0;
    for (const float v : x)
        summe += (double) v * (double) v;
    e.rmsIst = x.empty() ? 0.0 : std::sqrt (summe / (double) x.size());
    e.abweichungDb = 20.0 * std::log10 (e.rmsIst / rms);
    e.ok = e.groessteDifferenzDb <= 1.0 && std::abs (e.abweichungDb) <= 0.5;
    std::string baender;
    for (std::size_t i = 0; i < e.oktavenDb.size(); ++i)
        baender += (i ? "/" : "") + detail::festkomma (e.oktavenDb[i], 2);
    e.meldung = "groesste Nachbardifferenz " + detail::festkomma (e.groessteDifferenzDb, 3)
              + " dB (Soll <= 1,0), Oktaven " + baender + " dB; RMS " + detail::festkomma (e.rmsIst, 5)
              + " gegen " + detail::festkomma (rms, 5) + " (" + detail::festkomma (e.abweichungDb, 3)
              + " dB), " + std::to_string (x.size()) + " Samples";
    return e;
}

/** M-63 (NAK-380 Nacharbeit 1, Befund D3): unabhaengige Referenz der
    Flussstaerke eines Ereignisses aus seinem Fluss SF und der Historie der
    32 aktiven Frames VOR dem ausloesenden Frame. Median und echte MAD
    (Median der Absolutabweichungen; gerade Anzahl: Mittel der zwei mittleren
    Werte) werden hier gerechnet, nicht vom Produkt geliehen; kappa, rho und
    T_min kommen als Zahlen der Matrix (§6.3, Fassung §38.2: kappa = 3,
    rho = 1, T_min = 0,10 dB * K), nicht aus den Produktkonstanten.
    T_eff = max(med + kappa*MAD, (1 + rho)*med, T_min); Staerke =
    min(obergrenze, kappa*(SF - med)/(T_eff - med)). `gueltig` nur bei
    gerader, nicht leerer Historie und Nenner > 0. */
struct StaerkeReferenz
{
    bool gueltig { false };
    double med { 0.0 }, mad { 0.0 }, tEff { 0.0 }, staerke { 0.0 };
};

inline StaerkeReferenz staerkeReferenz (double sf, std::vector<double> historie, double kappa,
                                        double rho, double tMin, double obergrenze = 1000.0)
{
    StaerkeReferenz r;
    const std::size_t n = historie.size();
    if (n < 2u || n % 2u != 0u)
        return r;
    std::sort (historie.begin(), historie.end());
    r.med = 0.5 * (historie[n / 2 - 1] + historie[n / 2]);
    for (auto& v : historie)
        v = std::abs (v - r.med);
    std::sort (historie.begin(), historie.end());
    r.mad = 0.5 * (historie[n / 2 - 1] + historie[n / 2]);
    r.tEff = std::max (std::max (r.med + kappa * r.mad, (1.0 + rho) * r.med), tMin);
    const double nenner = r.tEff - r.med;
    if (! (nenner > 0.0))
        return r;
    r.staerke = std::min (obergrenze, kappa * (sf - r.med) / nenner);
    r.gueltig = std::isfinite (r.staerke);
    return r;
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

//==============================================================================
// NAK-380 Etappe 5 (§7.2 D1 bis D7): Laufzeit- und Binphasenpaare. L ist
// gleichverteiltes splitmix64-Rauschen mit Amplitude 0,35 (wie der Sweep in
// B19), R(n) = L(n - d). d = 0 ist Mono (L = R) fuer M-82 bis M-84 und M-88.

inline constexpr std::uint64_t kD1D6Saat = 0x0380000Full;
inline constexpr std::uint64_t kD7Saat   = 0x03800010ull;
inline constexpr double kLaufzeitAmplitude = 0.35;

/** Der Rauschkern des B19-Sweeps (`Sonde013StereoGoldenTest.cpp`, Lambda
    `rausch`, Muster §6.4) als reine Funktion des vorzeichenbehafteten
    Sampleindex m und der Saat: splitmix64-Durchmischung von m*phi + saat,
    gleichverteilt in [-1, 1). Die Saat geht VOR der Durchmischung in den
    Zustand (Lehre des Sweeps). Negative m sind derselbe Strom (Umlauf auf
    uint64 ist gewollt): so gilt R(n) = L(n - d) auch fuer n < d exakt. */
inline double laufzeitRauschen (std::int64_t m, std::uint64_t saat) noexcept
{
    std::uint64_t x = (std::uint64_t) m * 0x9E3779B97F4A7C15ull + saat;
    x ^= x >> 30; x *= 0xBF58476D1CE4E5B9ull;
    x ^= x >> 27; x *= 0x94D049BB133111EBull;
    x ^= x >> 31;
    return (double) (x >> 11) / 4503599627370495.5 - 1.0;
}

/** d = round(0,001 * fs) Samples (§7.2): 44, 48, 88, 96, 176, 192. */
inline int laufzeitSamples (double fs) noexcept
{
    return (int) std::llround (0.001 * fs);
}

/** Ein Stereopaar als float32: L(n) = 0,35 * u(n), R(n) = L(n - d). */
struct Rauschpaar
{
    std::vector<float> l, r;
    double fs { 0.0 };
    int d { 0 };
    std::uint64_t saat { 0 };
};

inline Rauschpaar rauschpaar (double fs, std::uint64_t samples, int d, std::uint64_t saat)
{
    Rauschpaar p;
    p.fs = fs;
    p.d = d;
    p.saat = saat;
    p.l.resize ((std::size_t) samples);
    p.r.resize ((std::size_t) samples);
    for (std::uint64_t n = 0; n < samples; ++n)
    {
        p.l[(std::size_t) n] = (float) (kLaufzeitAmplitude * laufzeitRauschen ((std::int64_t) n, saat));
        p.r[(std::size_t) n] = (float) (kLaufzeitAmplitude
                                        * laufzeitRauschen ((std::int64_t) n - (std::int64_t) d, saat));
    }
    return p;
}

/** Kopffunktion des Erzeugers (Lehre D4 aus §39.1: Selbstpruefung vor JEDEM
    Nutzer), eigener Prueffall `380/<Fall> rauschpaar_selbstpruefung`:
    - Saat: Kennwerte des Kerns fuer beide Saaten gegen Literale (am
      26.09.2026 unabhaengig in Python mit uint64-Arithmetik nachgerechnet)
      und die Saat des Paares ist die geforderte.
    - Amplitude: max|L| <= 0,35 und RMS gegen 0,35/sqrt(3) innerhalb 5 sigma
      der Stichprobenleistung. Herleitung: fuer u gleichverteilt ist
      Var(u^2)/E[u^2]^2 = (1/5 - 1/9)/(1/9) = 0,8, die relative
      Standardabweichung der Stichprobenleistung also sigma = sqrt(0,8/n);
      Toleranz -10*log10(1 - 5 sigma) dB: beim kuerzesten Nutzer (M-82,
      n = 44 100) 0,094 dB, bei M-95 (96 000) 0,063 dB, bei n = 1 920 000
      0,014 dB. Ein falscher Amplitudenfaktor (etwa 2, 6 dB) faellt sicher.
    - Verzoegerung: d = round(0,001*fs) wie gefordert, und R[n] ist bitgleich
      L[n - d] fuer jedes n >= d (exakte Sampleverschiebung); fuer n < d ist R[n]
      der Strom an n - d.
    - Laenge: beide Kanaele tragen genau `samplesSoll` Werte.
    Der Verlustzaehler des Ereignisrings ist eine eigene Pruefung jedes
    Laeufers. Leere oder nicht endliche Eingaben sind nie gruen. */
struct RauschpaarSelbstpruefung
{
    bool ok { false };
    double rmsDb { 0.0 }, rmsToleranzDb { 0.0 }, spitze { 0.0 };
    std::uint64_t verschiebungFehler { 0 };
    std::string meldung;
};

inline RauschpaarSelbstpruefung rauschpaarSelbstpruefung (const Rauschpaar& p, double fsSoll, int dSoll,
                                                          std::uint64_t saatSoll, std::uint64_t samplesSoll)
{
    RauschpaarSelbstpruefung e;
    const bool kennwerte = laufzeitRauschen (0, kD1D6Saat) == -0.15400736421841
                        && laufzeitRauschen (1, kD1D6Saat) == -0.16194761881861341
                        && laufzeitRauschen (-48, kD1D6Saat) == 0.6346681360614221
                        && laufzeitRauschen (0, kD7Saat) == 0.35656415952447196
                        && laufzeitRauschen (-48, kD7Saat) == 0.2740274454877165;
    const bool saat = p.saat == saatSoll && p.fs == fsSoll;
    const bool dOk = p.d == dSoll && dSoll >= 0;
    const bool laenge = p.l.size() == (std::size_t) samplesSoll && p.r.size() == (std::size_t) samplesSoll
                     && samplesSoll > (std::uint64_t) dSoll;
    double summe = 0.0;
    for (std::size_t n = 0; n < p.l.size(); ++n)
    {
        const double v = (double) p.l[n];
        summe += v * v;
        e.spitze = std::max (e.spitze, std::abs (v));
        if (! std::isfinite (v))
            e.spitze = std::numeric_limits<double>::infinity();
        const bool gleich = n >= (std::size_t) std::max (0, p.d)
            ? p.r[n] == p.l[n - (std::size_t) p.d]
            : p.r[n] == (float) (kLaufzeitAmplitude
                                 * laufzeitRauschen ((std::int64_t) n - (std::int64_t) p.d, p.saat));
        if (! gleich)
            ++e.verschiebungFehler;
    }
    const double rms = p.l.empty() ? 0.0 : std::sqrt (summe / (double) p.l.size());
    e.rmsDb = 20.0 * std::log10 (rms / (kLaufzeitAmplitude / std::sqrt (3.0)));
    const double sigma = p.l.empty() ? 1.0 : std::sqrt (0.8 / (double) p.l.size());
    e.rmsToleranzDb = 5.0 * sigma < 1.0 ? -10.0 * std::log10 (1.0 - 5.0 * sigma) : 0.0;
    e.ok = kennwerte && saat && dOk && laenge && e.verschiebungFehler == 0
        && e.spitze <= kLaufzeitAmplitude && std::abs (e.rmsDb) <= e.rmsToleranzDb;
    e.meldung = std::string ("Kennwerte ") + (kennwerte ? "ja" : "NEIN") + ", Saat "
              + std::to_string (p.saat) + (saat ? "" : " FALSCH")
              + ", d " + std::to_string (p.d) + (dOk ? "" : " FALSCH") + ", "
              + std::to_string (p.l.size()) + " Samples" + (laenge ? "" : " FALSCH")
              + ", Spitze " + detail::festkomma (e.spitze, 6) + ", RMS "
              + detail::festkomma (e.rmsDb, 4) + " dB gegen 0,35/sqrt(3) (Toleranz "
              + detail::festkomma (e.rmsToleranzDb, 4) + " dB), Verschiebungsfehler "
              + std::to_string (e.verschiebungFehler);
    return e;
}

} // namespace nakama::test::nak380
