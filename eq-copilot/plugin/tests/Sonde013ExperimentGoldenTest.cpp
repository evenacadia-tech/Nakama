/*  EqCopSonde013ExperimentGoldenTest — Unsicherheit und Mehrfachtestung
    (SONDE-013 M-45; §43.2).

    ── DIE ZWEI FRAGEN UEBER JEDEM METRIKDELTA ──────────────────────────────

    „Ist der Unterschied groesser als das Rauschen?" — dafuer der
    Block-Bootstrap. Und „habe ich ihn nur gefunden, weil ich 221 Baender
    gleichzeitig angeschaut habe?" — dafuer FDR und Cluster.

    Beide Fragen sind unbequem, weil ihre ehrliche Antwort fast immer weniger
    Aussage bedeutet als die naive Rechnung. Genau deshalb stehen sie in der
    Matrix: eine Zahl ohne Intervall und ein Bandscan ohne Korrektur sind
    keine schwaechere Evidenz, sondern gar keine.

    ── SPRACHUEBERGREIFEND ──────────────────────────────────────────────────

    `broker/src/coordinator/experiment.rs` haelt dieselben drei Rechnungen mit
    demselben Generator. Die festen Zahlen unten prueft `sonde013_experiment.rs`
    ebenfalls; wer eine Seite aendert, faellt auf der anderen.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/Statistik.h"

#include <cmath>
#include <limits>
#include <iostream>
#include <vector>

using nakama::analyse::blockBootstrap;
using nakama::analyse::cluster;
using nakama::analyse::fdrSignifikant;
using nakama::analyse::Intervall;

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
    ok ? ++bestanden : ++fehler;
}

void abschnitt (const char* name)
{
    std::cout << std::endl << "== " << name << " ==" << std::endl;
}

/** Die Golden-Eingabe: 64 Deltas. Dieselbe Folge steht in
    `sonde013_experiment.rs`.

    ⚠️ Zwei ungleiche Perioden (5 und 7), und beide teilerfremd zur
    Blocklaenge 8. Der erste Entwurf hatte Periode 8 - dann hat JEDER Block
    von acht aufeinanderfolgenden Werten denselben Mittelwert, jede Ziehung
    liefert dieselbe Zahl, und das Intervall ist trivial null. Der Golden war
    gruen und mass nichts. Nur reine Arithmetik, keine transzendenten
    Funktionen: `sin` kann sich zwischen zwei libm-Implementierungen im
    letzten Bit unterscheiden, und dieser Golden wird sprachuebergreifend
    geprueft. */
std::vector<double> goldenDeltas()
{
    std::vector<double> v;
    v.reserve (64);
    for (int i = 0; i < 64; ++i)
        v.push_back (1.0 + 0.1 * ((double) (i % 5) - 2.0) + 0.05 * ((double) (i % 7) - 3.0));
    return v;
}

/** Eine Folge mit BLOECKEN: acht Werte hoch, acht Werte tief. Genau die
    Korrelationsstruktur, die ein Einzelwert-Bootstrap ignoriert. */
std::vector<double> bloeckigeDeltas()
{
    std::vector<double> v;
    v.reserve (64);
    for (int i = 0; i < 64; ++i)
        v.push_back ((i / 8) % 2 == 0 ? 2.0 : -2.0);
    return v;
}

double mittel (const std::vector<double>& v)
{
    double s = 0.0;
    for (double x : v)
        s += x;
    return v.empty() ? 0.0 : s / (double) v.size();
}

// ═════════════════════════════════════════════════════════════════════════
// M-45 · block_bootstrap_interval
// ═════════════════════════════════════════════════════════════════════════
void blockBootstrapInterval()
{
    abschnitt ("M-45  block_bootstrap_interval");

    const auto deltas = goldenDeltas();
    const auto a = blockBootstrap (deltas, 8, 400, 0.05, 42);
    const auto b = blockBootstrap (deltas, 8, 400, 0.05, 42);
    pruefe (a.gesetzt && b.gesetzt && a.unten == b.unten && a.oben == b.oben,
            "dieselbe Saat ergibt dasselbe Intervall - ein Golden ueber einen "
            "Zufallsprozess ist sonst keiner",
            juce::String (a.unten, 6) + " .. " + juce::String (a.oben, 6));

    const double m = mittel (deltas);
    pruefe (a.gesetzt && a.unten <= m && m <= a.oben,
            "und es enthaelt den Mittelwert", juce::String (m, 6));

    // ⚠️ DIESELBEN ZAHLEN wie `sonde013_experiment.rs` auf der Rust-Seite.
    // Gen zeigt die Statistik live, der Broker schreibt sie in den Store;
    // ein Golden, der nur Eigenschaften prueft, liesse die beiden Fassungen
    // auseinanderdriften, ohne dass es auffiele.
    pruefe (std::abs (a.unten - 0.979688) < 1e-6 && std::abs (a.oben - 1.016406) < 1e-6,
            "und es ist BITGLEICH dem der Rust-Fassung - derselbe Generator, "
            "dieselbe Ziehungsreihenfolge",
            juce::String (a.unten, 6) + " .. " + juce::String (a.oben, 6));

    // ⚠️ Die eigentliche Zusage: das Blockintervall ist BREITER als eines aus
    // Einzelwerten. Benachbarte Deltas sind korreliert; ein Bootstrap ueber
    // Einzelwerte nimmt Unabhaengigkeit an, die es nicht gibt, und macht aus
    // Rauschen eine belastbare Aenderung.
    const auto bloeckig = bloeckigeDeltas();
    const auto block = blockBootstrap (bloeckig, 8, 400, 0.05, 7);
    const auto einzeln = blockBootstrap (bloeckig, 1, 400, 0.05, 7);
    const double breiteBlock = block.oben - block.unten;
    const double breiteEinzeln = einzeln.oben - einzeln.unten;
    pruefe (block.gesetzt && einzeln.gesetzt && breiteBlock > breiteEinzeln,
            "bei bloeckigem Material ist das Blockintervall breiter als das "
            "Einzelwertintervall - genau dafuer gibt es den Block-Bootstrap",
            juce::String (breiteBlock, 4) + " gegen " + juce::String (breiteEinzeln, 4));

    // Leere und unmoegliche Eingaben ergeben KEIN Intervall statt eines um
    // null: „nie gerechnet" ist etwas anderes als „Intervall der Breite 0".
    pruefe (! blockBootstrap ({}, 8, 100, 0.05, 1).gesetzt, "ohne Daten kein Intervall");
    pruefe (! blockBootstrap (deltas, 0, 100, 0.05, 1).gesetzt, "ohne Blocklaenge keins");
    pruefe (! blockBootstrap (deltas, 8, 0, 0.05, 1).gesetzt, "ohne Ziehungen keins");

    // Eine Blocklaenge groesser als die Folge klemmt, statt zu lesen, wo
    // nichts ist.
    const auto lang = blockBootstrap (deltas, 1000, 100, 0.05, 3);
    pruefe (lang.gesetzt && std::abs (lang.unten - m) < 1e-9 && std::abs (lang.oben - m) < 1e-9,
            "eine Blocklaenge ueber der Folgenlaenge klemmt auf sie - das Intervall "
            "wird dann zum Mittelwert selbst",
            juce::String (lang.unten, 6));

    // Ein groesseres alpha ergibt ein engeres Intervall.
    const auto eng = blockBootstrap (deltas, 8, 400, 0.5, 42);
    pruefe (eng.gesetzt && (eng.oben - eng.unten) < (a.oben - a.unten),
            "ein groesseres alpha ergibt ein engeres Intervall",
            juce::String (eng.oben - eng.unten, 5) + " gegen "
                + juce::String (a.oben - a.unten, 5));

    // ── Nacharbeit 1 (2026-09-04, Befund B34): FAIL-CLOSED an NaN/Inf ────
    //
    // Der Spiegel dieser Funktion im Broker akkumulierte einen
    // nicht-endlichen Delta, sortierte danach eine nicht geordnete Menge und
    // lieferte ein GESETZTES NaN-Intervall. Dieselbe Rechnung steht hier;
    // beide tragen jetzt denselben Riegel, und beide werden gemessen.
    for (const double gift : { std::numeric_limits<double>::quiet_NaN(),
                               std::numeric_limits<double>::infinity(),
                               -std::numeric_limits<double>::infinity() })
    {
        pruefe (! blockBootstrap ({ gift }, 1, 10, 0.05, 1).gesetzt,
                "B34: ein einzelner nichtendlicher Delta ergibt KEIN Intervall");
        auto gemischt = deltas;
        gemischt[7] = gift;
        const auto u = blockBootstrap (gemischt, 8, 400, 0.05, 42);
        pruefe (! u.gesetzt,
                "B34: auch EIN nichtendlicher Wert unter sauberen sperrt - ein "
                "Intervall ueber teilweise unbekannte Daten waere keine Aussage");
    }
    pruefe (! blockBootstrap (deltas, 8, 400,
                              std::numeric_limits<double>::quiet_NaN(), 42).gesetzt,
            "B34: und ein nichtendliches alpha ebenso");
}

// ═════════════════════════════════════════════════════════════════════════
// M-45 · band_scan_uses_cluster_or_fdr
// ═════════════════════════════════════════════════════════════════════════
void bandScanUsesClusterOrFdr()
{
    abschnitt ("M-45  band_scan_uses_cluster_or_fdr");

    // 221 Baender, gleichverteilte p-Werte: der Fall, in dem sich NICHTS
    // geaendert hat. Ohne Korrektur waeren bei alpha 0,05 rund elf Baender
    // „signifikant" - und der User lernte, die Meldung zu ignorieren.
    std::vector<double> rein;
    rein.reserve (221);
    for (int i = 0; i < 221; ++i)
        rein.push_back (((double) i + 0.5) / 221.0);

    int naiv = 0;
    for (double p : rein)
        if (p <= 0.05)
            ++naiv;
    pruefe (naiv >= 10,
            "ohne Korrektur meldete reines Rauschen zweistellig viele Baender",
            juce::String (naiv) + " von 221");

    const auto sig = fdrSignifikant (rein, 0.05);
    int nachFdr = 0;
    for (bool s : sig)
        if (s)
            ++nachFdr;
    pruefe (nachFdr == 0,
            "mit FDR-Korrektur kein einziges - das ist der ganze Punkt",
            juce::String (nachFdr));

    // Und ein echter Effekt kommt durch, sonst waere die Korrektur eine
    // Abschaltung.
    auto echt = rein;
    for (int i = 0; i < 20; ++i)
        echt[(std::size_t) i] = 0.0001;
    const auto sig2 = fdrSignifikant (echt, 0.05);
    int durch = 0;
    for (bool s : sig2)
        if (s)
            ++durch;
    pruefe (durch >= 20, "20 klare Treffer kommen durch", juce::String (durch));

    // Die Grenze faellt an ihrem Wert: p = k/m * q ist noch signifikant.
    std::vector<double> genau { 0.01, 0.02, 0.03, 0.04 };   // m = 4, q = 0,04
    const auto sigG = fdrSignifikant (genau, 0.04);
    pruefe (sigG[0] && sigG[1] && sigG[2] && sigG[3],
            "p_(k) = k/m * q gilt noch als signifikant - die Grenze ist inklusiv");
    std::vector<double> knapp { 0.011, 0.021, 0.031, 0.041 };
    const auto sigK = fdrSignifikant (knapp, 0.04);
    int knappDurch = 0;
    for (bool s : sigK)
        if (s)
            ++knappDurch;
    pruefe (knappDurch == 0, "und knapp darueber nicht mehr", juce::String (knappDurch));

    // ── Cluster ───────────────────────────────────────────────────────────
    // Drei verstreute Einzelbaender sind eher ein Artefakt als eine
    // Klangaenderung; ein EQ-Eingriff wirkt auf benachbarte Baender.
    const std::vector<bool> verstreut { true, false, true, false, true, false };
    pruefe (cluster (verstreut, 3).empty(),
            "drei verstreute Einzelbaender ergeben keinen Cluster");

    const std::vector<bool> zusammen { false, true, true, true, true, false };
    const auto c = cluster (zusammen, 3);
    pruefe (c.size() == 1 && c[0].first == 1 && c[0].second == 5,
            "vier benachbarte schon", juce::String ((int) c.size()) + " Cluster");

    // Auch am rechten Rand - ein Cluster, der bis zum letzten Band reicht,
    // wird sonst still verschluckt.
    const std::vector<bool> amRand { false, false, true, true, true };
    const auto cr = cluster (amRand, 3);
    pruefe (cr.size() == 1 && cr[0].first == 2 && cr[0].second == 5,
            "und einer am rechten Rand ebenso");

    // Und am linken.
    const std::vector<bool> linksRand { true, true, true, false, false };
    const auto cl = cluster (linksRand, 3);
    pruefe (cl.size() == 1 && cl[0].first == 0 && cl[0].second == 3,
            "und einer am linken");

    // Genau die Mindestlaenge zaehlt, eine darunter nicht.
    const std::vector<bool> genauDrei { false, true, true, true, false };
    const std::vector<bool> nurZwei { false, true, true, false, false };
    pruefe (cluster (genauDrei, 3).size() == 1 && cluster (nurZwei, 3).empty(),
            "die Mindestlaenge faellt an ihrem Wert");

    // Ein leerer Bandscan ergibt keinen Cluster und keine Signifikanz.
    pruefe (cluster ({}, 3).empty() && fdrSignifikant ({}, 0.05).empty(),
            "ein leerer Bandscan ergibt nichts, statt zu paniken");
}

} // namespace

int main()
{
    std::cout << "== Nakama SONDE-013 - Unsicherheit und Mehrfachtestung (§43.2) =="
              << std::endl;
    blockBootstrapInterval();
    bandScanUsesClusterOrFdr();
    std::cout << std::endl << bestanden << " bestanden, " << fehler << " gescheitert"
              << std::endl;
    return fehler == 0 ? 0 : 1;
}
