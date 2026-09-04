// SONDE-013 M-45: Unsicherheit und Mehrfachtestung (§43.2). JUCE-frei und
// ohne Allokation im heissen Pfad — derselbe Kernvertrag wie `Konfidenz.h`.
//
// ── WARUM DIESE DREI FUNKTIONEN ZUSAMMENGEHOEREN ─────────────────────────
//
// Sie beantworten die zwei Fragen, die ueber jedem Metrikdelta stehen:
// „ist der Unterschied groesser als das Rauschen" (Bootstrap) und „habe ich
// ihn nur gefunden, weil ich 221 Baender gleichzeitig angeschaut habe"
// (FDR und Cluster). Ohne die erste ist jede Zahl eine Behauptung, ohne die
// zweite meldet Nakama bei jedem Vergleich irgendein „veraendertes" Band —
// und der User lernt, die Meldung zu ignorieren.
//
// ── DIESELBEN ZAHLEN WIE IM BROKER ───────────────────────────────────────
//
// `broker/src/coordinator/experiment.rs` haelt dieselben drei Rechnungen:
// Gen zeigt sie live, der Broker schreibt sie in den Store. Beide benutzen
// denselben LCG mit denselben Konstanten und dieselbe Ziehungsreihenfolge,
// und der Golden in `Sonde013ExperimentGoldenTest` prueft feste Zahlen, die
// `sonde013_experiment.rs` ebenfalls prueft. Wer eine Seite aendert, faellt
// auf der anderen.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace nakama::analyse
{

/** Derselbe deterministische Generator wie auf der Rust-Seite.

    Bootstrap braucht Ziehungen, ein Golden braucht Reproduzierbarkeit —
    beides zugleich geht nur mit gesetzter Saat. Die Konstanten sind die des
    `PCG`-Multiplikators; sie stehen hier und in `experiment.rs` identisch. */
class Wuerfel
{
public:
    explicit Wuerfel (std::uint64_t saat) noexcept
        : zustand (saat * 6364136223846793005ull + 1ull) {}

    std::size_t naechste (std::size_t grenze) noexcept
    {
        zustand = zustand * 6364136223846793005ull + 1442695040888963407ull;
        if (grenze == 0)
            return 0;
        return (std::size_t) ((zustand >> 33) % (std::uint64_t) grenze);
    }

private:
    std::uint64_t zustand;
};

/** Das Ergebnis eines Bootstraps: ein Intervall, oder gar keins. */
struct Intervall
{
    bool   gesetzt { false };
    double unten { 0.0 };
    double oben { 0.0 };
};

/** Block-Bootstrap fuer ein Konfidenzintervall des Mittelwerts (M-45).

    ⚠️ Warum BLOCK-Bootstrap und nicht der gewoehnliche: die Deltas
    benachbarter Analysefenster sind korreliert — dasselbe Material, dieselbe
    Kette, ueberlappende Fenster. Ein Bootstrap, der einzelne Werte zieht,
    nimmt Unabhaengigkeit an, die es nicht gibt, und liefert ein VIEL zu enges
    Intervall. Genau daraus entstuende eine „belastbare" Aenderung, die keine
    ist. */
inline Intervall blockBootstrap (const std::vector<double>& deltas,
                                 std::size_t blocklaenge,
                                 std::size_t ziehungen,
                                 double alpha,
                                 std::uint64_t saat)
{
    const std::size_t n = deltas.size();
    if (n == 0 || blocklaenge == 0 || ziehungen == 0)
        return {};
    // 🔑 SONDE-013 Nacharbeit 1 (Befund B34), derselbe Riegel wie im Broker:
    // ein nicht-endlicher Delta wandert sonst durch die Summe, macht die
    // Sortierung bedeutungslos und ergibt ein GESETZTES NaN-Intervall. M-07
    // und M-45 verlangen hier "ungueltig", nicht "ein Intervall".
    for (const double d : deltas)
        if (! std::isfinite (d))
            return {};
    if (! std::isfinite (alpha))
        return {};
    const std::size_t bl = std::min (blocklaenge, n);
    const std::size_t bloecke = n - bl + 1;

    Wuerfel w (saat);
    std::vector<double> mittel;
    mittel.reserve (ziehungen);
    for (std::size_t z = 0; z < ziehungen; ++z)
    {
        double summe = 0.0;
        std::size_t gezogen = 0;
        while (gezogen < n)
        {
            const std::size_t start = w.naechste (bloecke);
            for (std::size_t k = 0; k < bl && gezogen < n; ++k, ++gezogen)
                summe += deltas[start + k];
        }
        mittel.push_back (summe / (double) n);
    }
    std::sort (mittel.begin(), mittel.end());
    const std::size_t u = (std::size_t) std::floor ((alpha / 2.0) * (double) ziehungen);
    const std::size_t o = std::min ((std::size_t) std::ceil ((1.0 - alpha / 2.0) * (double) ziehungen),
                                    ziehungen - 1);
    const double unten = mittel[std::min (u, ziehungen - 1)];
    const double oben  = mittel[o];
    // Auch am Ausgang: eine Summe endlicher Werte kann ueberlaufen.
    if (! std::isfinite (unten) || ! std::isfinite (oben))
        return {};
    return { true, unten, oben };
}

/** Benjamini-Hochberg-Korrektur fuer viele gleichzeitige Tests (M-45).

    Bei 221 gleichzeitig gescannten Baendern sind bei alpha = 0,05 rund elf
    Fehlalarme zu erwarten, wenn sich NICHTS geaendert hat. */
inline std::vector<bool> fdrSignifikant (const std::vector<double>& pWerte, double q)
{
    const std::size_t m = pWerte.size();
    std::vector<bool> aus (m, false);
    if (m == 0)
        return aus;

    std::vector<std::size_t> rang (m);
    for (std::size_t i = 0; i < m; ++i)
        rang[i] = i;
    std::sort (rang.begin(), rang.end(),
               [&] (std::size_t a, std::size_t b) { return pWerte[a] < pWerte[b]; });

    // Der GROESSTE Rang k mit p_(k) <= k/m * q; alle Raenge bis dahin gelten.
    // Das ist der Kern von Benjamini-Hochberg: es wird nicht der erste
    // Ueberschreiter gesucht, sondern der letzte Unterschreiter.
    bool haben = false;
    std::size_t grenze = 0;
    for (std::size_t r = 0; r < m; ++r)
    {
        const double schwelle = ((double) (r + 1) / (double) m) * q;
        if (pWerte[rang[r]] <= schwelle)
        {
            grenze = r;
            haben = true;
        }
    }
    if (haben)
        for (std::size_t r = 0; r <= grenze; ++r)
            aus[rang[r]] = true;
    return aus;
}

/** Zusammenhaengende Cluster signifikanter Baender (M-45).

    Die Alternative zur FDR-Korrektur, und sie beantwortet eine andere Frage:
    FDR sagt „welche EINZELNEN Baender halten der Mehrfachtestung stand",
    Cluster sagt „wo liegt ein zusammenhaengender Bereich". Ein EQ-Eingriff
    wirkt auf benachbarte Baender; drei verstreute Einzelbaender sind eher ein
    Artefakt als eine Klangaenderung. */
inline std::vector<std::pair<std::size_t, std::size_t>>
cluster (const std::vector<bool>& signifikant, std::size_t mindestlaenge)
{
    std::vector<std::pair<std::size_t, std::size_t>> aus;
    bool offen = false;
    std::size_t start = 0;
    for (std::size_t i = 0; i < signifikant.size(); ++i)
    {
        if (signifikant[i] && ! offen)
        {
            offen = true;
            start = i;
        }
        else if (! signifikant[i] && offen)
        {
            if (i - start >= mindestlaenge)
                aus.emplace_back (start, i);
            offen = false;
        }
    }
    if (offen && signifikant.size() - start >= mindestlaenge)
        aus.emplace_back (start, signifikant.size());
    return aus;
}

} // namespace nakama::analyse
