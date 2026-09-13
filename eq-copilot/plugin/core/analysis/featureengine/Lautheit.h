// FeatureEngine — Lautheit und Pegel: LUFS, LRA, True-Peak, Crest.
//
// NAK-225/S25d (09.09.2026), Definitionen herausgeloest aus der Klasse in
// core/analysis/FeatureEngine.h. Inhalt:
//
//   zelleSchliessen      Eine 100-ms-Zelle abschliessen und in die Verlaeufe
//                        legen.
//   kurzLufs, momentanLufs
//                        Die beiden Fenster nach BS.1770.
//   kurzTruePeak, crestKurz
//                        Spitze und Crest ueber demselben Kurzfenster.
//   lraBin, lraBinMitte, lraPerzentil, lraLu
//                        Der LRA-Weg ueber das Histogramm samt Gates.
//
// Ein Wert entsteht hier nur, wenn genug endliche Zellen dahinterstehen; sonst
// gibt es keine Aussage statt einer erfundenen Zahl (NaN-Ehrlichkeit,
// CLAUDE.md). Die Rueckgabe ist deshalb durchweg `bool` mit Ausgabeparameter.

#ifndef NAKAMA_FEATUREENGINE_TEIL
#error "Teilkopf von FeatureEngine.h - nur ueber FeatureEngine.h einbinden."
#endif

#pragma once

namespace nakama::analyse
{

//== Loudness =============================================================

inline void FeatureEngine::zelleSchliessen() noexcept
{
    // `zelleKEnergie` ist bereits die Kanal-SUMME (s. o.); geteilt wird nur
    // durch die Samplezahl, damit aus Σ y² das mean(y²) der Norm wird.
    const double mittel = zelleKEnergie / (double) zellenSamples;
    kurzZellen[(std::size_t) kurzStand] = mittel;
    // 🔑 NAK-283 F10 (R-283-5, §8.1 Feinheit 8): GUELTIGKEIT JE ZELLE, NICHT
    // JE ZAEHLER.
    //
    // Die Zelle, die hier schliesst, umfasst genau die letzten `zellenSamples`
    // verarbeiteten Samples: nach jedem Neuanfang (Grenze, Passagenanfang,
    // `zuruecksetzen`) zaehlt `zelleStand` von 0, eine Zelle beginnt also nie
    // vor ihm. Liegt das juengste ersetzte Sample darin, traegt die Zelle das
    // Merkmal "enthaelt ersetztes Material" - und jede Metrik ueber ein
    // Fenster, das sie enthaelt, bleibt ohne Wert (`zellenFensterSauber`).
    //
    // Das Merkmal haengt an der SAMPLESTELLE, nicht an `rahmenNichtEndlich`
    // oder `evidenzNichtEndlich`: die zwei Zaehler fallen mit Rahmen und
    // Evidenzfenster und saettigen bei 0xFFFFFFFF (M-59) - beides ist eine
    // andere Lebensdauer als die der Zelle, und genau daran hing der Befund.
    const std::uint64_t zellenBeginn =
        verarbeiteteSamples >= (std::uint64_t) zellenSamples
            ? verarbeiteteSamples - (std::uint64_t) zellenSamples : 0u;
    const bool ersetzt = letztesErsetztesSample > zellenBeginn;
    kurzZellenErsetzt[(std::size_t) kurzStand] = ersetzt ? 1u : 0u;
    // SONDE-013 M-03/M-04: die zwei Nachbarringe wandern MIT demselben
    // Stand, damit ein Kurzzeitfenster ueber alle drei dieselben Zellen
    // sieht.  Zwei Ringe mit eigenem Stand waeren zwei Fenster mit
    // demselben Namen — genau die Vermischung, die §39.1 bei PSR und
    // Crest ausschliesst.
    kurzTpZellen[(std::size_t) kurzStand]  = zelleTruePeak;
    kurzRmsZellen[(std::size_t) kurzStand] = zelleRmsEnergie / (double) zellenSamples;
    kurzStand = (kurzStand + 1) % kKurzZellen;
    if (kurzGefuellt < kKurzZellen) ++kurzGefuellt;

    // LRA (M-04, EBU Tech 3342): alle 10 Zellen — also 1 s Hop — geht der
    // aktuelle Kurzzeitwert ins Histogramm, sofern das VOLLE 3-s-Fenster
    // steht und der Wert das absolute Gate nimmt.  Nur diese gezaehlten
    // Werte tragen die 60-s-Regel; Stille laesst den Zaehler stehen.
    // M-04/M-25: nur eine Zelle, die VOLLSTAENDIG in der markierten Passage
    // lag, geht ins LRA-Histogramm. Ohne Fenster zaehlt jede Zelle, weil
    // dann keine Passage markiert ist.
    const bool zelleImFenster = ! passagenfenster.gesetzt
                             || (int) zelleImFensterSamples >= zellenSamples;
    if (++lraZellenSeitHop >= kLraHopZellen)
    {
        lraZellenSeitHop = 0;
        double kurzJetzt = 0.0;
        if (zelleImFenster && kurzLufs (kurzJetzt) && kurzJetzt >= kLraAbsGateLufs)
        {
            const int bin = lraBin (kurzJetzt);
            if (bin >= 0)
            {
                ++lraHistogramm[(std::size_t) bin];
                ++lraGezaehlt;
            }
        }
    }

    const double aktivMittel = zelleAktivEnergie / (double) zellenSamples;
    if (aktivMittel > 0.0 && 10.0 * std::log10 (aktivMittel) > kAktivGateDb)
        ++rahmenAktivZellen;
    ++rahmenZellen;
}

/** Binindex eines Kurzzeitwertes im LRA-Histogramm, oder -1, wenn er
    ausserhalb des Rasters liegt.  Ein Wert ueber +30 LUFS ist jenseits
    jeder Musik und faellt lieber heraus, als das Raster zu sprengen. */
inline int FeatureEngine::lraBin (double lufs) noexcept
{
    if (! std::isfinite (lufs))
        return -1;
    const int b = (int) std::floor ((lufs - kLraBinUnten) / kLraBinBreite);
    return (b >= 0 && b < kLraBins) ? b : -1;
}

/** LUFS-S = −0,691 + 10·log10(Σ Zellenergie / n) ueber die letzten 3 s.
    Nur bei VOLLER Historie — eine Kurzzeitlautheit ueber 0,4 s ist keine
    Kurzzeitlautheit, sie ist eine andere Zahl mit demselben Namen. */
inline bool FeatureEngine::kurzLufs (double& heraus) const noexcept
{
    if (kurzGefuellt < kKurzZellen)
        return false;
    // NAK-283 F10 (R-283-5): kein Wert, solange eine der 30 Zellen ersetztes
    // Material traegt - die Verriegelung lebt genau so lange wie das Fenster.
    if (! zellenFensterSauber (kKurzZellen))
        return false;
    double su = 0.0;
    for (int i = 0; i < kKurzZellen; ++i)
        su += kurzZellen[(std::size_t) i];
    const double mittel = su / (double) kKurzZellen;
    if (! (mittel > 0.0))
        return false;
    heraus = -0.691 + 10.0 * std::log10 (mittel);
    return std::isfinite (heraus);
}

/** LUFS-M = dieselbe Formel ueber die letzten 400 ms (M-01, §39.1).

    Nur bei mindestens `kMomentZellen` gefuellten Zellen — aus demselben
    Grund wie bei `kurzLufs`: eine Momentanlautheit ueber 200 ms ist keine
    Momentanlautheit, sie ist eine andere Zahl mit demselben Namen. */
inline bool FeatureEngine::momentanLufs (double& heraus) const noexcept
{
    if (kurzGefuellt < kMomentZellen)
        return false;
    // NAK-283 F10: dieselbe Regel ueber das EIGENE Fenster - die letzten vier
    // Zellen. Momentary kommt deshalb nach 400 ms wieder, Short-term erst nach
    // 3 s (M-57); ein gemeinsamer Warnzaehler koennte die zwei Fristen nicht
    // trennen.
    if (! zellenFensterSauber (kMomentZellen))
        return false;
    double summe = 0.0;
    for (int i = 0; i < kMomentZellen; ++i)
    {
        const int idx = (kurzStand - 1 - i + kKurzZellen * 2) % kKurzZellen;
        summe += kurzZellen[(std::size_t) idx];
    }
    const double mittel = summe / (double) kMomentZellen;
    if (! (mittel > 0.0))
        return false;
    heraus = -0.691 + 10.0 * std::log10 (mittel);
    return std::isfinite (heraus);
}

/** NAK-283 F10 (R-283-5, §8.1 Feinheit 8): traegt KEINE der letzten
    `anzahl` geschlossenen Zellen das Merkmal "enthaelt ersetztes Material"?

    Die Gueltigkeit einer Fenstermetrik haengt an der tatsaechlich
    verwendeten Historie: `kurzLufs`, `kurzTruePeak` (und damit PSR und Crest)
    fragen alle 30 Zellen, `momentanLufs` die letzten vier. Die Aufrufer haben
    vorher geprueft, dass mindestens `anzahl` Zellen gefuellt sind. */
inline bool FeatureEngine::zellenFensterSauber (int anzahl) const noexcept
{
    for (int i = 0; i < anzahl; ++i)
    {
        const int idx = (kurzStand - 1 - i + kKurzZellen * 2) % kKurzZellen;
        if (kurzZellenErsetzt[(std::size_t) idx] != 0u)
            return false;
    }
    return true;
}

/** Das Kurzzeitfenster beginnt neu: die laufende Zelle, die drei 3-s-Ringe,
    ihre Zellenmarken und der LRA-Hop.

    SONDE-013 Nacharbeit 2 (Befund R04) liess das beim Binden einer Passage
    geschehen. Seit NAK-283 F07 laeuft es am STARTSAMPLE der Passage
    (`verarbeiteSamples`) - und beim Binden nur noch, wenn der Anfang schon
    zurueckliegt und kein Startmerker ihn mehr sieht (§8.1 Feinheit 17). Die
    K-Filter bleiben unberuehrt, wie bisher: ihre Vorgeschichte an einer
    Passage ist ausdruecklich kein Befund dieses Tickets (Uebergabe §5). */
inline void FeatureEngine::kurzfensterNeuBeginnen() noexcept
{
    zelleStand = 0;
    zelleKEnergie = 0.0;
    zelleAktivEnergie = 0.0;
    zelleTruePeak = 0.0;
    zelleRmsEnergie = 0.0;
    zelleImFensterSamples = 0;
    kurzStand = 0;
    kurzGefuellt = 0;
    lraZellenSeitHop = 0;
    for (auto& z : kurzZellen)        z = 0.0;
    for (auto& z : kurzTpZellen)      z = 0.0;
    for (auto& z : kurzRmsZellen)     z = 0.0;
    for (auto& z : kurzZellenErsetzt) z = 0u;
}

/** True-Peak-Maximum DESSELBEN 3-s-Fensters, gegen das `kurzLufs` rechnet
    (M-03).  Linear, wie der Detektor selbst liefert. */
inline bool FeatureEngine::kurzTruePeak (double& heraus) const noexcept
{
    if (kurzGefuellt < kKurzZellen)
        return false;
    // NAK-283 F10: ein Maximum ueber ersetzte Samples ist keine Aussage ueber
    // das Material - PSR und Crest ueber 3 s schweigen mit `lufsS`.
    if (! zellenFensterSauber (kKurzZellen))
        return false;
    double groesster = 0.0;
    for (int i = 0; i < kKurzZellen; ++i)
        groesster = std::max (groesster, kurzTpZellen[(std::size_t) i]);
    heraus = groesster;
    return groesster > 0.0;
}

/** Crest ueber das 3-s-Fenster: True-Peak-Maximum gegen die ungewichtete
    RMS desselben Fensters (M-04, "Crest-Faktor in MEHREREN Fenstern
    statt nur als globales Maximum").

    Das zweite Fenster neben dem 100-ms-Rahmen in `fuelleSkalare`.  Beide
    reisen, weil ein Signal mit dichten kleinen Spitzen im Rahmen einen
    hohen und ueber 3 s einen niedrigen Crest hat — und umgekehrt eine
    einzelne Spitze in ruhigem Material genau andersherum. */
inline bool FeatureEngine::crestKurz (double& heraus) const noexcept
{
    double tpMax = 0.0;
    if (! kurzTruePeak (tpMax) || tpMax <= 0.0)
        return false;
    double summe = 0.0;
    for (int i = 0; i < kKurzZellen; ++i)
        summe += kurzRmsZellen[(std::size_t) i];
    const double rms = std::sqrt (summe / (double) kKurzZellen);
    if (! (rms > 0.0))
        return false;
    heraus = 20.0 * std::log10 (tpMax) - 20.0 * std::log10 (rms);
    return std::isfinite (heraus);
}

/** LRA nach EBU Tech 3342 (M-04).

    Reihenfolge, und jeder Schritt ist eine eigene Bedingung, die allein
    fallen kann:
    1. mindestens `kLraMindestSekunden` GEGATETE Kurzzeitwerte — sonst
       `false`, also KEIN Wert.  §39.1 woertlich: "vorher `nicht
       belastbar`".  Eine 0 waere hier eine Dynamikaussage, die niemand
       gemessen hat;
    2. gegatetes Mittel ueber die absolut gegatete Verteilung bilden;
    3. relatives Gate bei -20 LU darunter anlegen;
    4. LRA = P95 - P10 der so gegateten Restverteilung.

    ⚠️ Das relative Gate ist -20 LU, NICHT die -10 LU der integrierten
    Lautheit.  Die zwei Zahlen gehoeren zu zwei verschiedenen Normen
    (BS.1770 gegen Tech 3342); sie zu vertauschen ergibt eine Zahl, die
    plausibel aussieht und um mehrere LU falsch ist. */
inline bool FeatureEngine::lraLu (double& heraus) const noexcept
{
    // Ein gegateter Kurzzeitwert je `kLraHopZellen` Zellen, also je
    // `kLraHopZellen * kZelleSekunden` Sekunden.
    const double sekundenJeWert = kZelleSekunden * (double) kLraHopZellen;
    if ((double) lraGezaehlt * sekundenJeWert < kLraMindestSekunden)
        return false;
    if (lraHistogramm.size() != (std::size_t) kLraBins)
        return false;

    // Schritt 2: energetisches Mittel ueber die absolut gegateten Werte.
    double summeZ = 0.0;
    std::uint64_t n = 0;
    for (int b = 0; b < kLraBins; ++b)
    {
        const auto c = lraHistogramm[(std::size_t) b];
        if (c == 0u) continue;
        summeZ += (double) c * std::pow (10.0, (lraBinMitte (b) + 0.691) / 10.0);
        n += c;
    }
    if (n == 0u || ! (summeZ > 0.0))
        return false;
    const double mittelLufs = -0.691 + 10.0 * std::log10 (summeZ / (double) n);
    const double relGate = mittelLufs + kLraRelGateLu;

    // Schritt 3 und 4: Restverteilung und ihre zwei Perzentile.
    std::uint64_t gesamt = 0;
    for (int b = 0; b < kLraBins; ++b)
        if (lraBinMitte (b) >= relGate)
            gesamt += lraHistogramm[(std::size_t) b];
    if (gesamt == 0u)
        return false;

    const double p10 = lraPerzentil (relGate, gesamt, 0.10);
    const double p95 = lraPerzentil (relGate, gesamt, 0.95);
    heraus = p95 - p10;
    return std::isfinite (heraus) && heraus >= 0.0;
}

constexpr double FeatureEngine::lraBinMitte (int b) noexcept
{
    return kLraBinUnten + ((double) b + 0.5) * kLraBinBreite;
}

/** Perzentil der relativ gegateten LRA-Verteilung.  Kumulativ ueber die
    Bins von unten; der erste Bin, der den Anteil erreicht, gewinnt. */
inline double FeatureEngine::lraPerzentil (double relGate, std::uint64_t gesamt, double anteil) const noexcept
{
    const double ziel = anteil * (double) gesamt;
    double lauf = 0.0;
    for (int b = 0; b < kLraBins; ++b)
    {
        const double mitte = lraBinMitte (b);
        if (mitte < relGate) continue;
        lauf += (double) lraHistogramm[(std::size_t) b];
        if (lauf >= ziel)
            return mitte;
    }
    return lraBinMitte (kLraBins - 1);
}

} // namespace nakama::analyse
