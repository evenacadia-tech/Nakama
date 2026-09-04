// SONDE-013 (S20-22) — True-Peak-Detektor nach ITU-R BS.1770-5.
//
// Matrixzeile M-02:  "True Peak ist BS.1770-konform.  Der vorhandene
// 8-fach-Pfad bleibt NUR, wenn er das offizielle EBU-Testset bei 48 kHz und
// generierte Mehrsampleraten-Goldens besteht; sonst wird der
// BS.1770-Polyphase-Referenzpfad verwendet.  Toleranz +/-0,1 dB (§49.3).
// Das Bein muss die Entscheidung 8-fach gegen Polyphase MESSBAR treffen,
// nicht behaupten."
//
// ─────────────────────────────────────────────────────────────────────────────
// WARUM DIESE DATEI UEBERHAUPT EXISTIERT.
//
// Ein True Peak ist NICHT der groesste Samplewert.  Zwischen zwei Samples
// liegt eine Wellenform, und ihr Scheitel kann hoeher sein als jeder Abtast-
// punkt — bei einem Sinus mit fs/4 und 45 Grad Phase um genau 3,01 dB.  Genau
// deshalb misst EBU Tech 3341 diesen Fall (Testfall 16): ein Detektor, der die
// Samples einfach maximiert, meldet dort -9,03 dBFS statt -6,02 dBTP und ist
// um drei Dezibel zu optimistisch.  Ein Master, der auf so eine Zahl gebaut
// wird, clippt im Encoder.
//
// Die Rechnung dagegen ist ueberabtasten und DANN maximieren.  BS.1770-4
// Annex 2 verlangt mindestens 4-fach fuer 48 kHz; Nakama fuehrt 8-fach, weil
// die Restunsicherheit mit dem Faktor faellt und ihr WERT AUSRECHENBAR ist:
// der wahre Scheitel liegt hoechstens eine halbe Stuetzstelle daneben, also
//
//     Fehler(f) = 20*log10( cos( 2*pi * f/fs * 1/(2*kFaktor) ) )
//
// Bei 8-fach und fs/4 sind das -0,042 dB, bei fs/6 -0,019 dB, bei fs/8
// -0,010 dB — alle drei innerhalb der +/-0,1 dB aus §49.3.  Bei 4-fach waere
// derselbe fs/4-Fall schon -0,17 dB und risse die Toleranz.  DAS ist die
// Begruendung fuer 8-fach, und sie steht hier als Formel, damit der Golden sie
// nachrechnen kann statt sie zu glauben (`EqCopSonde013TruePeakGoldenTest`,
// Fall `interpolationsfehler_folgt_der_formel`).
//
// ─────────────────────────────────────────────────────────────────────────────
// WARUM PHASENWEISE NORMIERT WIRD, UND NICHT UEBER DIE GANZE SUMME.
//
// Der Interpolator ist ein Polyphasenfilter: die 8 Teilfilter erzeugen je eine
// Zwischenstuetzstelle.  Damit ein Gleichanteil nach der Interpolation wieder
// ein Gleichanteil ist, muss JEDE Phase fuer sich die Summe 1 haben.  Eine
// Normierung der GESAMTSUMME auf 8 laesst den Phasen ihre Einzelabweichung —
// aus einer Konstanten wird dann eine mit 8*fs modulierte Welle, und ihr
// Maximum ist zu hoch.  Der Unterschied ist klein, aber er ist genau die
// Groessenordnung, um die es bei +/-0,1 dB geht.
//
// ⚠️ Echtzeit: `vorbereiten()` rechnet die Koeffizienten und ist der EINZIGE
// Ort mit Rechenaufwand ueber O(1).  Er laeuft auf dem Nachrichtenthread.
// `tick()` allokiert nicht, sperrt nicht und hat eine feste Schleifenlaenge.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace nakama::analyse
{

//==============================================================================
/** 8-fach-Polyphaseninterpolator mit Kaiser-gefenstertem Sinc.

    Der Detektor haelt KEIN Ergebnis ueber die Zeit: er liefert je Sample den
    groessten interpolierten Betrag.  Ueber welchen Zeitraum daraus ein Maximum
    wird — Rahmen, 3-s-Fenster oder Passage — entscheidet der Aufrufer, weil
    §39.1 fuer PLR und PSR ausdruecklich VERSCHIEDENE Zeitraeume verlangt.
*/
class TruePeakDetektor
{
public:
    /** Ueberabtastfaktor.  Siehe Kopf: 8 ist nicht Geschmack, sondern die
        kleinste Zweierpotenz, deren Restfehler bei fs/4 unter 0,1 dB bleibt. */
    static constexpr int kFaktor = 8;
    /** Taps je Phase.  24 ergibt 193 Gesamttaps (ungerade, symmetrisch um die
        Mitte) und haelt das Passband bis 0,4*fs innerhalb 0,01 dB. */
    static constexpr int kTapsJePhase = 24;
    static constexpr int kTaps = kFaktor * kTapsJePhase + 1;   // 193
    /** Ringlaenge: die naechste Zweierpotenz ueber `kTapsJePhase`, damit die
        Indexrechnung eine Maske statt eines Modulo ist. */
    static constexpr int kRing = 32;
    static_assert (kRing >= kTapsJePhase + 1, "Ring muss die Taps einer Phase fassen");

    /** Kaiser-Beta.  8,0 ergibt rund -80 dB Sperrdaempfung; die Passbandwelligkeit
        liegt damit bei 24 Taps je Phase unter 0,01 dB — eine Groessenordnung
        unter der Toleranz, die dieser Pfad einhalten muss. */
    static constexpr double kKaiserBeta = 8.0;

    /** Rechnet die Koeffizienten.  NICHT im Audiothread aufrufen.

        Die Koeffizienten haengen NICHT von der Samplerate ab: der Interpolator
        arbeitet in normierter Zeit.  Der Parameter existiert trotzdem, weil ein
        Sampleratewechsel nach §32.3 eine Epochengrenze ist und die Historie
        dabei verfallen muss — sonst reichte ein Filterzustand aus der alten
        Rate in die neue hinein. */
    void vorbereiten (double /*samplerate*/) noexcept
    {
        // Zwingt die Tabelle JETZT, auf dem Nachrichtenthread, in ihre
        // Initialisierung.  Danach ist `koeffizienten()` im Audiothread ein
        // reiner Zeigerzugriff — der Compiler prueft das Init-Flag noch, aber
        // er nimmt keine Sperre mehr.
        (void) koeffizienten();
        // Der Verlauf liegt im HEAP, nicht als Feld. 64 `double` je Instanz
        // sind einzeln nichts, aber der Kern haelt in B5 zwanzig Engines
        // gleichzeitig auf dem Stack, und genau an dieser Summe ist in
        // Etappe B schon einmal der 1-MiB-Stack gerissen (Manifest §10.2,
        // Befund 1). Angelegt wird er hier, auf dem Nachrichtenthread;
        // `tick()` allokiert weiterhin nie.
        verlaufL.assign ((std::size_t) kRing, 0.0);
        verlaufR.assign ((std::size_t) kRing, 0.0);
        zuruecksetzen();
    }

    /** Die 193 Filterkoeffizienten.  EINMAL fuer den ganzen Prozess.

        ⚠️ Sie sind bewusst KEIN Instanzfeld.  Der Kern haelt in einem Test
        zwanzig `FeatureEngine`-Objekte gleichzeitig auf dem Stack (die
        Zwillingsprobe G13 in B5 zwei davon nebeneinander), und 193 `double`
        je Instanz sind 1,5 KiB, die sich dort aufsummieren.  Genau daran ist
        in Etappe B schon einmal der 1-MiB-Stack gerissen
        (`STATUS_STACK_OVERFLOW`, Manifest §10.2, Befund 1).  Die Tabelle
        haengt an nichts Instanzabhaengigem — insbesondere NICHT an der
        Samplerate, weil der Interpolator in normierter Zeit arbeitet —, also
        gibt es keinen Grund, sie je Objekt zu halten. */
    static const double* koeffizienten() noexcept
    {
        static const std::array<double, (std::size_t) kTaps> tabelle = berechneFir();
        return tabelle.data();
    }

    /** Leert die Historie.  Aufzurufen an JEDER Epochen- und Segmentgrenze:
        ein Filterzustand ueber eine Grenze hinweg mischt zwei Stellen der Musik
        zu einem Zwischenwert, und dessen Scheitel gehoert zu keiner von beiden
        (§32.3, dieselbe Regel wie fuer den K-Filter in `KGewichtung.h`). */
    void zuruecksetzen() noexcept
    {
        for (auto& v : verlaufL) v = 0.0;
        for (auto& v : verlaufR) v = 0.0;
        stand = 0;
    }

    /** Ob `vorbereiten()` gelaufen ist.  Ohne den Verlauf gibt es keinen
        True Peak, und `tick()` liefert dann 0 statt in den leeren Vektor zu
        greifen — dieselbe fail-closed-Haltung wie `FeatureEngine::vorbereitet`. */
    bool bereit() const noexcept
    { return verlaufL.size() == (std::size_t) kRing && verlaufR.size() == (std::size_t) kRing; }

    /** Ein Samplepaar hinein, der groesste interpolierte Betrag heraus.

        Der Rueckgabewert ist LINEAR, nicht in dB: der Aufrufer bildet daraus
        sein Maximum, und ein Maximum ueber Linearwerte ist dasselbe wie eines
        ueber dB — nur ohne 8 Logarithmen je Sample im Audiothread.

        Nichtendliche Eingaben werden zu 0.  Das ist dieselbe Regel wie in
        `FeatureEngine::verarbeiteSamples`: der NaN-Riegel liegt VOR der
        Rechnung, damit kein NaN in den Filterzustand kommt und dort bleibt. */
    double tick (double l, double r) noexcept
    {
        if (! bereit())
            return 0.0;
        if (! std::isfinite (l)) l = 0.0;
        if (! std::isfinite (r)) r = 0.0;

        verlaufL[(std::size_t) stand] = l;
        verlaufR[(std::size_t) stand] = r;

        const double* const h0 = koeffizienten();
        double groesster = 0.0;
        for (int phase = 0; phase < kFaktor; ++phase)
        {
            double accL = 0.0, accR = 0.0;
            for (int k = 0; phase + kFaktor * k < kTaps; ++k)
            {
                const double h = h0[(std::size_t) (phase + kFaktor * k)];
                const int idx = (stand - k) & (kRing - 1);
                accL += h * verlaufL[(std::size_t) idx];
                accR += h * verlaufR[(std::size_t) idx];
            }
            groesster = std::max (groesster, std::max (std::abs (accL), std::abs (accR)));
        }

        stand = (stand + 1) & (kRing - 1);
        return std::isfinite (groesster) ? groesster : 0.0;
    }

    /** Der ausgerechnete Interpolationsfehler dieses Faktors bei einer
        normierten Frequenz `fRelFs` = f/fs, in dB (negativ oder 0).

        Er steht hier und nicht im Test, damit die Zusage des Kopfes und die
        Messung dieselbe Formel benutzen — eine im Test nachgebaute Formel
        koennte am Produktpfad vorbeidriften, ohne dass etwas faellt. */
    static constexpr double interpolationsfehlerDb (double fRelFs) noexcept
    {
        const double halbeStufe = 1.0 / (2.0 * (double) kFaktor);
        const double c = cosKonst (2.0 * kPi * fRelFs * halbeStufe);
        return c > 0.0 ? 20.0 * log10Konst (c) : -400.0;
    }

private:
    static constexpr double kPi = 3.14159265358979323846;

    /** Rechnet die Koeffizienten einmalig.  Kaiser-gefensterter Sinc, danach
        PHASENWEISE auf Summe 1 normiert — siehe Kopf: eine Phase ohne
        DC-Eins moduliert einen Gleichanteil mit kFaktor*fs, und ihr Maximum
        waere ein Artefakt des Filters statt einer Messung. */
    static std::array<double, (std::size_t) kTaps> berechneFir() noexcept
    {
        std::array<double, (std::size_t) kTaps> h {};
        const int mitte = (kTaps - 1) / 2;                    // 96
        const double i0Beta = besselI0 (kKaiserBeta);
        for (int i = 0; i < kTaps; ++i)
        {
            const double m = (double) (i - mitte);
            // Idealer Interpolationskern: sinc(m / kFaktor).  Bei m == 0 ist
            // er 1 — der Grenzwert, nicht 0/0.
            const double ideal = (m == 0.0)
                ? 1.0
                : std::sin (kPi * m / (double) kFaktor) / (kPi * m / (double) kFaktor);
            const double x = (double) (i - mitte) / (double) mitte;
            const double arg = 1.0 - x * x;
            const double w = besselI0 (kKaiserBeta * std::sqrt (std::max (arg, 0.0))) / i0Beta;
            h[(std::size_t) i] = ideal * w;
        }
        for (int phase = 0; phase < kFaktor; ++phase)
        {
            double summe = 0.0;
            for (int k = 0; phase + kFaktor * k < kTaps; ++k)
                summe += h[(std::size_t) (phase + kFaktor * k)];
            if (summe != 0.0)
                for (int k = 0; phase + kFaktor * k < kTaps; ++k)
                    h[(std::size_t) (phase + kFaktor * k)] /= summe;
        }
        return h;
    }

    /** Modifizierte Besselfunktion erster Art, Ordnung 0 — die Kaiser-Basis.
        Reihenentwicklung; bei beta <= 12 konvergiert sie in unter 30 Gliedern
        auf `double`-Genauigkeit. */
    static double besselI0 (double x) noexcept
    {
        double summe = 1.0, glied = 1.0;
        const double halbes = 0.5 * x;
        for (int k = 1; k < 40; ++k)
        {
            glied *= (halbes / (double) k) * (halbes / (double) k);
            summe += glied;
            if (glied < 1e-18 * summe)
                break;
        }
        return summe;
    }

    // ── constexpr-Hilfen fuer `interpolationsfehlerDb` ─────────────────────
    // std::cos und std::log10 sind erst ab C++26 constexpr; diese zwei
    // Reihen sind es heute und liefern im gebrauchten Bereich (|x| < 1,
    // c in (0,1]) mehr als genug Stellen fuer eine Toleranzaussage.
    static constexpr double cosKonst (double x) noexcept
    {
        double glied = 1.0, summe = 1.0;
        for (int k = 1; k < 16; ++k)
        {
            glied *= -x * x / (double) ((2 * k - 1) * (2 * k));
            summe += glied;
        }
        return summe;
    }
    static constexpr double log10Konst (double x) noexcept
    {
        // ln via atanh-Reihe um 1: ln(x) = 2*atanh((x-1)/(x+1)).
        const double t = (x - 1.0) / (x + 1.0);
        double potenz = t, summe = 0.0;
        for (int k = 0; k < 32; ++k)
        {
            summe += potenz / (double) (2 * k + 1);
            potenz *= t * t;
        }
        return (2.0 * summe) / 2.30258509299404568402;
    }

    std::vector<double> verlaufL, verlaufR;
    int    stand { 0 };
};

} // namespace nakama::analyse
