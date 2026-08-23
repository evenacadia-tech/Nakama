// SONDE-008 (S10-11) — fixed-memory Loudness, Entwurf §48.1, §39.1, §53.4.
//
// WOZU. Bis zu diesem Ticket hielt `AnalyseEngine` die K-gewichtete Energie in
// `std::vector<double> kZellen` und schob dort alle 100 ms einen Wert hinein -
// unbegrenzt. Der Kopfkommentar nannte das "10 Werte/s - unproblematisch ueber
// Stunden"; der Entwurf sagt etwas anderes (§48.1): "Die heutige unbeschraenkt
// wachsende `AnalyseEngine::kZellen`-Liste und ihre LUFS-I-Vektorkopien werden
// durch einen fixed-memory `LoudnessAccumulator` aus festen Energie-/
// Loudness-Histogrammbins, kompensierten Summen und einer begrenzten
// Passage-Blockstatistik ersetzt." Der teurere Teil war ohnehin nicht der
// Speicher: `finalisiereLoudness` legte VIERMAL JE SEKUNDE einen frischen
// `std::vector` ueber die volle Sessionlaenge an und summierte ihn - der
// Aufwand wuchs quadratisch mit der Spieldauer.
//
// WARUM DAS NICHT TRIVIAL IST. BS.1770-5 / EBU R128 gaten zweistufig:
//   1. absolutes Gate: Bloecke mit l_j >= -70 LUFS,
//   2. relatives Gate: Gamma_r = Loudness(Mittel der absolut-gegateten z_j)
//      minus 10 LU, danach die Bloecke mit l_j > Gamma_r.
// Schritt 2 braucht die Schwelle, die erst Schritt 1 liefert - ein exakter
// zweiter Durchgang in KONSTANTEM Speicher ist unmoeglich (es ist im Kern eine
// Quantil-Summen-Abfrage ueber die Blockverteilung).
//
// DIE AUFTEILUNG, und sie ist der ganze Trick:
//   * Das ABSOLUTE Gate laeuft als kompensierte Laufsumme (Neumaier) plus
//     Zaehler - dafuer braucht es keine Historie, also ist Gamma_r EXAKT.
//   * Nur die AUSWAHL fuer das relative Gate kommt aus einem Histogramm ueber
//     die Blocklautheit. Dort - und nur dort - sitzt eine Quantisierung: die
//     Bloecke im Bin, den Gamma_r durchschneidet, sind nicht einzeln
//     unterscheidbar. Die Bin-SUMMEN sind exakt; unsicher ist allein die
//     Zugehoerigkeit eines einzigen Bins.
//   * §48.1 verlangt genau deshalb: "Der EBU-Korpus prueft, dass Quantisierung
//     und Gating innerhalb der Toleranz aus Abschnitt 49 bleiben" (§49: ±0,1 LU).
//     `unsicherheitLu()` macht die Schranke auslesbar, statt sie zu behaupten -
//     der Golden misst sie mit.
//
// WAS BITGLEICH BLEIBT. Der Kurzzeit-LUFS (3 s) liest die letzten 30 Zellen in
// derselben Reihenfolge wie die alte Vektorschleife und summiert sie mit
// derselben Assoziativitaet; die Blockbildung (4 Zellen, Hop 1) ebenso. Beide
// Werte sind damit BITGLEICH zum bisherigen Code, nicht nur "innerhalb einer
// Toleranz" - die einzige Abweichung des Tickets ist die Quantisierung im
// relativen Gate der INTEGRIERTEN Lautheit.
//
// THREADVERTRAG. Dieselbe Klasse wie die AnalyseEngine, die sie besitzt: NUR
// der Workerthread ruft `zelle()`, `vorbereiten()`, `zuruecksetzen()`. Der
// Audiothread sieht sie nie. `vorbereiten()` alloziert EINMAL; danach ist der
// Speicherbedarf konstant und aus `speicherBytes()` ablesbar.
#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace nakama::analyse
{

/** Neumaier-Summe: haelt den Fehler auch dann, wenn der neue Summand GROESSER
    ist als die bisherige Summe (Kahan verliert genau dort). Ueber eine Stunde
    kommen ~36000 Bloecke zusammen, deren Energien zehn Groessenordnungen
    auseinanderliegen koennen - das ist der Fall, fuer den es gebaut ist. */
struct KompSumme
{
    double summe     { 0.0 };
    double korrektur { 0.0 };

    void addiere (double x) noexcept
    {
        const double t = summe + x;
        if (std::abs (summe) >= std::abs (x))
            korrektur += (summe - t) + x;
        else
            korrektur += (x - t) + summe;
        summe = t;
    }

    double wert() const noexcept { return summe + korrektur; }
    void   leeren() noexcept     { summe = 0.0; korrektur = 0.0; }
};

//==============================================================================
class LoudnessAccumulator
{
public:
    /** 100-ms-Zellen je Block (400 ms, BS.1770) und Hop 1 Zelle (75 % Overlap). */
    static constexpr int kZellenJeBlock = 4;
    /** Kurzzeit-LUFS = 3 s = 30 Zellen (EBU Tech 3341). Zugleich die Groesse des
        Zellenrings - die "begrenzte Passage-Blockstatistik" aus §48.1. */
    static constexpr int kZellenRing = 30;

    /** Histogramm der BLOCKLAUTHEIT. Untergrenze ist das absolute Gate: was
        darunter liegt, kann kein Gate passieren und braucht keinen Bin.
        Obergrenze mit Reserve - Vollaussteuerung ergibt z <= 2, also l_j <= 2,3
        LUFS; K-Gewichtung hebt hoehenreiches Material um ~4 dB, und selbst eine
        entgleiste Kette vor uns bleibt weit unter +30.

        ⚠️ Was OBERHALB liegt, wird NICHT in den obersten Bin geklemmt. Die
        erste Fassung tat das mit der Begruendung, ein so lauter Block liege
        "ueber JEDER Schwelle" - das war falsch, und der Golden hat es
        widerlegt: bei durchweg ueberlautem Material liegt Gamma_r selbst
        oberhalb der Gittermitte, der geklemmte Bin fiel damit durch das
        relative Gate, und `integriert()` lieferte GAR KEINEN Wert. Solche
        Bloecke stehen deshalb in einem eigenen, exakten Eimer, dessen
        Mittelwert-Lautheit bekannt ist - dieselbe Bin-Mitten-Regel, nur mit
        einer Mitte, die nicht geschaetzt werden muss. */
    static constexpr double kBinUnten  = -70.0;
    static constexpr double kBinOben   =  30.0;
    static constexpr double kBinBreite =   0.01;   // LU
    static constexpr int    kBins      = (int) ((kBinOben - kBinUnten) / kBinBreite) + 1;   // 10001

    /** BS.1770-Blocklautheit. 1e-30 wie bisher: haelt log10 von der Null fern,
        ohne je einen Wert zu erfinden (der Beitrag liegt 300 dB unter allem,
        was ein Gate sieht). */
    static double lautheit (double z) noexcept
    {
        return -0.691 + 10.0 * std::log10 (z + 1e-30);
    }

    //== Einrichtung (Workerthread) ===========================================

    /** Alloziert das Histogramm EINMAL. Ein zweiter Aufruf mit derselben
        Samplerate ist ein No-op; eine andere Samplerate setzt zurueck. */
    void vorbereiten (double samplerate)
    {
        if (binSumme.size() != (std::size_t) kBins)
        {
            binSumme.assign ((std::size_t) kBins, 0.0);
            binAnzahl.assign ((std::size_t) kBins, 0u);
        }
        if (sr != samplerate)
        {
            sr = samplerate;
            zuruecksetzen();
        }
    }

    /** Setzt alles zurueck - ohne eine einzige Allokation. */
    void zuruecksetzen() noexcept
    {
        for (auto& v : binSumme)  v = 0.0;
        for (auto& v : binAnzahl) v = 0u;
        summeAbs.leeren();
        anzahlAbs = 0;
        summeGenauGate.leeren();
        anzahlGenauGate = 0;
        summeUeberGitter.leeren();
        anzahlUeberGitter = 0;
        anzahlUnterGate = 0;
        anzahlNichtEndlich = 0;
        for (auto& v : ring) v = 0.0;
        ringKopf = 0;
        zellen = 0;
        bloecke = 0;
    }

    //== Zufuhr (Workerthread) ================================================

    /** Eine fertige 100-ms-Zelle: Sigma ueber Kanaele der K-gewichteten
        Quadratsumme (genau der Wert, den `AnalyseEngine::verarbeite` bisher in
        `kZellen` schob). */
    void zelle (double kEnergie) noexcept
    {
        ring[(std::size_t) ringKopf] = kEnergie;
        ringKopf = (ringKopf + 1) % kZellenRing;
        ++zellen;

        if (zellen < (std::uint64_t) kZellenJeBlock || sr <= 0.0)
            return;

        // Blockleistung z_j aus den letzten VIER Zellen, in derselben
        // Reihenfolge und mit derselben Assoziativitaet wie bisher
        // ((((0+a)+b)+c)+d entspricht ((a+b)+c)+d) - deshalb bitgleich.
        double e = 0.0;
        for (int k = 0; k < kZellenJeBlock; ++k)
            e += ring[(std::size_t) ((ringKopf + kZellenRing - kZellenJeBlock + k) % kZellenRing)];
        const double z = e / (0.4 * sr);
        ++bloecke;

        const double lj = lautheit (z);
        if (! std::isfinite (lj) || ! std::isfinite (z))
        {
            // NaN-Ehrlichkeit: gezaehlt, nie stillschweigend als 0 verbucht.
            ++anzahlNichtEndlich;
            return;
        }

        if (lj < -70.0)                       // absolutes Gate ist ">=" (pyloudnorm)
        {
            ++anzahlUnterGate;
            return;
        }

        summeAbs.addiere (z);
        ++anzahlAbs;

        if (lj == -70.0)
        {
            // Genau auf der Kante. Das absolute Gate nimmt sie (">="), das
            // relative verwirft sie (">" -70, pyloudnorm) - im Histogramm koennte
            // sie nicht beides sein, also steht sie getrennt und exakt.
            summeGenauGate.addiere (z);
            ++anzahlGenauGate;
            return;
        }

        int bin = (int) std::floor ((lj - kBinUnten) / kBinBreite);
        if (bin >= kBins)
        {
            // Ueber dem Gitter: eigener exakter Eimer statt Klemmen (s. o.).
            summeUeberGitter.addiere (z);
            ++anzahlUeberGitter;
            return;
        }
        if (bin < 0) { bin = 0; }             // nur bei lj knapp ueber -70 durch Rundung
        binSumme[(std::size_t) bin] += z;
        ++binAnzahl[(std::size_t) bin];
    }

    //== Ergebnis (Workerthread) ==============================================

    /** Integrierte Lautheit (LUFS-I). `false` = kein Block ueber dem absoluten
        Gate; die Referenz kodiert das als `null`, nie als Zahl. */
    bool integriert (double& heraus) const noexcept
    {
        if (anzahlAbs == 0)
            return false;
        const double gammaR = lautheit (summeAbs.wert() / (double) anzahlAbs) - 10.0;

        KompSumme s2;
        std::uint64_t n2 = 0;
        for (int b = 0; b < kBins; ++b)
        {
            if (binAnzahl[(std::size_t) b] == 0)
                continue;
            if (binMitte (b) > gammaR)
            {
                s2.addiere (binSumme[(std::size_t) b]);
                n2 += binAnzahl[(std::size_t) b];
            }
        }
        // Die Bloecke ueber dem Gitter, entschieden an ihrer EXAKT bekannten
        // Mittelwert-Lautheit statt an einer geschaetzten Bin-Mitte.
        if (anzahlUeberGitter > 0
            && lautheit (summeUeberGitter.wert() / (double) anzahlUeberGitter) > gammaR)
        {
            s2.addiere (summeUeberGitter.wert());
            n2 += anzahlUeberGitter;
        }
        if (n2 == 0)
            return false;
        heraus = lautheit (s2.wert() / (double) n2);
        return true;
    }

    /** Wieviel LU das Ergebnis hoechstens verschiebt, wenn man den EINEN Bin,
        den Gamma_r durchschneidet, anders zuordnet. Das ist die vollstaendige
        Schranke der Quantisierung aus §48.1 - alles andere ist exakt. 0 heisst:
        kein Block liegt im Grenzbin, das Ergebnis ist unquantisiert. */
    double unsicherheitLu() const noexcept
    {
        if (anzahlAbs == 0)
            return 0.0;
        const double gammaR = lautheit (summeAbs.wert() / (double) anzahlAbs) - 10.0;
        const int grenze = (int) std::floor ((gammaR - kBinUnten) / kBinBreite);
        if (grenze < 0 || grenze >= kBins || binAnzahl[(std::size_t) grenze] == 0)
            return 0.0;

        KompSumme sicher;
        std::uint64_t nSicher = 0;
        for (int b = 0; b < kBins; ++b)
        {
            if (b == grenze || binAnzahl[(std::size_t) b] == 0)
                continue;
            if (binMitte (b) > gammaR)
            {
                sicher.addiere (binSumme[(std::size_t) b]);
                nSicher += binAnzahl[(std::size_t) b];
            }
        }
        // Der Ueber-Gitter-Eimer traegt keine Unsicherheit: seine Mitte ist
        // exakt bekannt, seine Zugehoerigkeit damit entschieden.
        if (anzahlUeberGitter > 0
            && lautheit (summeUeberGitter.wert() / (double) anzahlUeberGitter) > gammaR)
        {
            sicher.addiere (summeUeberGitter.wert());
            nSicher += anzahlUeberGitter;
        }
        const double sMit = sicher.wert() + binSumme[(std::size_t) grenze];
        const std::uint64_t nMit = nSicher + binAnzahl[(std::size_t) grenze];
        if (nSicher == 0 || nMit == 0)
            return 0.0;
        return std::abs (lautheit (sMit / (double) nMit)
                         - lautheit (sicher.wert() / (double) nSicher));
    }

    /** Kurzzeit-LUFS ueber die letzten 3 s. Bitgleich zur bisherigen Schleife
        ueber `kZellen[size-30 .. size-1]`. */
    bool kurz (double& heraus) const noexcept
    {
        if (zellen < (std::uint64_t) kZellenRing || sr <= 0.0)
            return false;
        double e = 0.0;
        for (int i = 0; i < kZellenRing; ++i)
            e += ring[(std::size_t) ((ringKopf + i) % kZellenRing)];   // aeltest -> juengst
        heraus = -0.691 + 10.0 * std::log10 (e / (3.0 * sr) + 1e-30);
        return true;
    }

    //== Telemetrie ===========================================================

    std::uint64_t zellenGesamt() const noexcept      { return zellen; }
    std::uint64_t bloeckeGesamt() const noexcept     { return bloecke; }
    std::uint64_t bloeckeAbsGate() const noexcept    { return anzahlAbs; }
    std::uint64_t bloeckeUnterGate() const noexcept  { return anzahlUnterGate; }
    std::uint64_t bloeckeUeberGitter() const noexcept { return anzahlUeberGitter; }
    std::uint64_t bloeckeNichtEndlich() const noexcept { return anzahlNichtEndlich; }

    /** Der Beweis, dass "fixed-memory" keine Behauptung ist: der Bedarf haengt
        an keiner Laufzeitgroesse. */
    static constexpr std::size_t speicherBytes() noexcept
    {
        return (std::size_t) kBins * (sizeof (double) + sizeof (std::uint32_t))
             + (std::size_t) kZellenRing * sizeof (double);
    }

private:
    static double binMitte (int b) noexcept
    {
        return kBinUnten + ((double) b + 0.5) * kBinBreite;
    }

    double sr { 0.0 };

    // Feste Energie-/Loudness-Histogrammbins (§48.1).
    std::vector<double>        binSumme;    // exakte Summe der z_j je Bin
    std::vector<std::uint32_t> binAnzahl;   // Bloecke je Bin

    // Kompensierte Summen (§48.1) - das absolute Gate braucht keine Historie.
    KompSumme     summeAbs;
    std::uint64_t anzahlAbs { 0 };
    KompSumme     summeGenauGate;           // l_j == -70,0 exakt: im abs. Gate, nie im relativen
    std::uint64_t anzahlGenauGate { 0 };
    /** Bloecke ueber `kBinOben`: exakt gefuehrt, nicht geklemmt (s. kBinOben). */
    KompSumme     summeUeberGitter;
    std::uint64_t anzahlUeberGitter { 0 };

    std::uint64_t anzahlUnterGate    { 0 };
    std::uint64_t anzahlNichtEndlich { 0 };

    // Begrenzte Passage-Blockstatistik (§48.1): die letzten 3 s.
    double        ring[kZellenRing] {};
    int           ringKopf { 0 };
    std::uint64_t zellen   { 0 };
    std::uint64_t bloecke  { 0 };
};

} // namespace nakama::analyse
