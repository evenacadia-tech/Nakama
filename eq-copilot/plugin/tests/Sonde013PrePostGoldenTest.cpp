/*  EqCopSonde013PrePostGoldenTest — die Audioseite des PRE/POST-Joins
    (SONDE-013 M-18, M-20, M-24; §38.3, §38.4).

    ── DIE THEORETISCHE GRUNDLAGE, UND SIE IST DER GANZE TEST ───────────────

    Die Magnitude-Squared Coherence zwischen Eingang und Ausgang einer Kette
    ist **1 für jede lineare zeitinvariante Kette** — unabhängig davon, wie
    stark sie den Klang verändert. Ein Gain von +20 dB, ein steiler Tiefpass,
    eine Allpasskette: alle bleiben bei 1. Sie fällt genau dann, wenn die
    Kette nichtlinear oder zeitvariabel ist.

    Genau das ist die Zusage aus M-18: „Inkonsistente Pegel- und
    Spektralrelationen werden markiert. Danach darf Nakama keinen festen
    Übertragungsfrequenzgang behaupten." Und §38.3 nennt die Zahl: ein
    Transferwert ist nur ab 0,8 Kohärenz zulässig.

    Der Test schickt deshalb PRE nach links und POST nach rechts durch EINE
    FeatureEngine. Die bandweise Interchannel-Kohärenz aus §40.1 IST dann die
    PRE/POST-Kohärenz — es braucht dafür keine zweite Engine und keine neue
    Zahl, sondern nur die richtige Frage an eine vorhandene.

    ── WARUM DIE GEGENPROBE HIER WICHTIGER IST ALS DIE PROBE ────────────────

    Ein Test, der nur zeigt, dass vier böse Ketten die Kohärenz senken, wäre
    auch dann grün, wenn die Kohärenz IMMER niedrig ist. Deshalb laufen zwei
    lineare Ketten mit: ein reiner Gain und ein IIR-Tiefpass. Bleiben die
    nicht oben, misst der Rest nichts.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/FeatureEngine.h"
#include "../core/analysis/Vergleichspegel.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <cstdint>
#include <limits>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace rt = nakama::echtzeit;
using nakama::analyse::Deltasperre;
using nakama::analyse::FeatureEngine;
using nakama::analyse::Gitter;
using nakama::analyse::Vergleichspegel;
using nakama::analyse::deltaZustand;
using nakama::analyse::sperrName;

namespace
{
int bestanden = 0;
int fehler = 0;
constexpr double kZweiPi = 6.283185307179586476925286766559;

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

struct Speiser
{
    FeatureEngine& engine;
    double sr { 48000.0 };
    int    frames { 512 };
    std::uint64_t strom { 0 };
    std::int64_t  projekt { 0 };
    std::vector<float> audio;

    explicit Speiser (FeatureEngine& e) : engine (e) {}

    rt::StampedBlock bauen() const
    {
        rt::StampedBlock b;
        b.stromVon = strom;
        b.sampleCount = (std::uint32_t) frames;
        b.segment = 0;
        b.startFolge = 0;
        b.kanaele = 2;
        b.tapMaske = 1;
        b.projectSampleStart = projekt;
        b.sampleRate = sr;
        b.flags = rt::kFlagKontextAnwesend | rt::kFlagSpieltGueltig
                | rt::kFlagSampleRateGueltig | rt::kFlagSpielt | rt::kFlagZeitGueltig;
        return b;
    }

    bool sende (const std::function<void (std::uint64_t, float&, float&)>& f)
    {
        const auto b = bauen();
        audio.resize ((std::size_t) b.sampleCount * 2u);
        for (std::uint32_t i = 0; i < b.sampleCount; ++i)
        {
            float l = 0.0f, r = 0.0f;
            f (strom + i, l, r);
            audio[(std::size_t) i * 2u]      = l;
            audio[(std::size_t) i * 2u + 1u] = r;
        }
        const bool ok = engine.nimmBlock (b, audio.data());
        strom   += b.sampleCount;
        projekt += (std::int64_t) b.sampleCount;
        return ok;
    }

    bool bisEvidenz (const std::function<void (std::uint64_t, float&, float&)>& f,
                     int maxBloecke = 900)
    {
        for (int i = 0; i < maxBloecke; ++i)
            if (sende (f) && engine.frame().evidenzFrisch)
                return true;
        return false;
    }
};

/** Ein breitbandiges, reproduzierbares Musiksurrogat: mehrere Teiltöne plus
    ein deterministisches Rauschen, damit möglichst viele Bänder überhaupt
    Energie tragen.

    ⚠️ Und mit einer langsamen Pegelhüllkurve — das ist keine Verzierung. Ein
    Kompressor verhält sich bei STATIONÄREM Material wie ein linearer Gain:
    seine Verstärkung steht still, und dann ist er auch messtechnisch linear.
    Die Zusage „Kompression erzeugt keine falsche statische EQ-Behauptung"
    lässt sich also nur an dynamischem Material prüfen. Beim Bau von Etappe F
    gemessen: mit konstanter Amplitude wanderte die PRE/POST-Relation eines
    6:1-Kompressors um 0,06 dB — nicht unterscheidbar von einem Kabel.

    Die Hüllkurve wirkt auf PRE und POST gleich; sie ist Teil der QUELLE, nicht
    der Kette, und lässt die Kohärenz einer linearen Kette deshalb bei 1. */
double quelle (std::uint64_t n, double fs)
{
    const double t = (double) n / fs;
    double x = 0.0;
    for (int k = 1; k <= 9; ++k)
        x += std::sin (kZweiPi * 110.0 * (double) k * t) / (double) k;
    // Deterministisches Rauschen mit derselben LCG wie anderswo im Ticket.
    std::uint64_t s = n * 6364136223846793005ull + 1442695040888963407ull;
    s ^= s >> 33;
    const double rausch = ((double) ((s >> 33) & 0x7fffffffu) / 2147483648.0) - 0.5;
    // Pegelhuellkurve mit rund 2,7 s Periode: 20 dB Spanne, also genau der
    // Bereich, in dem ein Kompressor arbeitet.
    const double h = 0.1 + 0.9 * (0.5 + 0.5 * std::sin (kZweiPi * t / 2.7));
    return h * (0.22 * x + 0.10 * rausch);
}

/** Wie viele Bänder mit gesetzter Kohärenz über der Transferschwelle liegen,
    und wie viele überhaupt eine tragen. */
struct Kohaerenzbild
{
    int gesetzt { 0 };
    int ueberSchwelle { 0 };
    double mittel { 0.0 };
    double anteilUeberSchwelle() const
    {
        return gesetzt > 0 ? (double) ueberSchwelle / (double) gesetzt : 0.0;
    }
};

Kohaerenzbild kohaerenzbild (const FeatureEngine& e, float schwelle)
{
    Kohaerenzbild k;
    double summe = 0.0;
    for (int b = 0; b < Gitter::evidenzBaender; ++b)
    {
        const auto& w = e.stereoBand (b);
        if (! w.kohaerenzGesetzt)
            continue;
        ++k.gesetzt;
        summe += (double) w.kohaerenz;
        if (w.kohaerenz >= schwelle)
            ++k.ueberSchwelle;
    }
    k.mittel = k.gesetzt > 0 ? summe / (double) k.gesetzt : 0.0;
    return k;
}

/** Das Bild einer Kette an BEIDEN Wachen.

    Die Kohärenz ist die erste. Die zweite ist die Streuung der
    PRE/POST-Relation über die Zeit — bei `l = PRE, r = POST` ist
    `seitenanteilDb` (Side gegen Mid) genau diese Relation: bei identischen
    Kanälen liegt sie am Boden, bei `POST = k · PRE` ist sie konstant, und
    bei einer nichtlinearen Kette wandert sie von Fenster zu Fenster. Das ist
    dieselbe Grösse, die `prepost::kettenbefund` im Broker prüft — hier an
    echtem Audio statt an synthetischen Hüllkurven. */
struct Kettenbild
{
    Kohaerenzbild kohaerenz;
    /// Standardabweichung des mittleren Seitenanteils über die Fenster, in dB.
    double seitenStreuungDb { 0.0 };
    int fenster { 0 };
};

/** Fährt eine Kette über mehrere Evidenzfenster. `kette(n, x)` bildet aus dem
    PRE-Sample das POST-Sample; sie darf Zustand halten. */
Kettenbild fahreKette (double fs, const std::function<double (std::uint64_t, double)>& kette,
                       int fensterZahl = 8)
{
    auto halter = std::make_unique<FeatureEngine>();
    auto& e = *halter;
    e.vorbereiten (fs);
    // ⚠️ 1 s Evidenzfenster. Die Kohärenz braucht mindestens acht Welch-
    // Frames (§40.1); im Vorgabefenster liegen zu wenige, und der Test wäre
    // grün, ohne je eine Kohärenz gesehen zu haben.
    e.evidenzIntervallSetzen (1.0);
    Speiser s { e };
    s.sr = fs;
    auto material = [&] (std::uint64_t n, float& l, float& r)
    {
        const double x = quelle (n, fs);
        l = (float) x;
        r = (float) kette (n, x);
    };

    Kettenbild bild;
    // ⚠️ JE BAND eine Zeitreihe, nicht ein Mittel über die Bänder. Eine
    // nichtlineare Kette wirkt bandabhängig verschieden; der Mittelwert über
    // 149 Bänder löscht genau das aus. Beim Bau gemessen: gemittelt wanderte
    // ein 6:1-Kompressor um 0,06 dB, je Band um ein Vielfaches davon.
    std::vector<std::vector<double>> proBand ((std::size_t) Gitter::evidenzBaender);
    for (int f = 0; f < fensterZahl; ++f)
    {
        if (! s.bisEvidenz (material))
            break;
        ++bild.fenster;
        bild.kohaerenz = kohaerenzbild (e, 0.8f);
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
        {
            const auto& w = e.stereoBand (b);
            if (w.basisGesetzt && std::isfinite (w.seitenanteilDb))
                proBand[(std::size_t) b].push_back ((double) w.seitenanteilDb);
        }
    }

    // Der MEDIAN der bandweisen Streuungen. Das Maximum hinge an einem
    // einzelnen unruhigen Band, der Mittelwert an den vielen stillen.
    std::vector<double> streuungen;
    for (const auto& reihe : proBand)
    {
        if (reihe.size() < 3)
            continue;
        const double mittel = std::accumulate (reihe.begin(), reihe.end(), 0.0)
                            / (double) reihe.size();
        double q = 0.0;
        for (double v : reihe)
            q += (v - mittel) * (v - mittel);
        streuungen.push_back (std::sqrt (q / (double) reihe.size()));
    }
    if (! streuungen.empty())
    {
        std::sort (streuungen.begin(), streuungen.end());
        bild.seitenStreuungDb = streuungen[streuungen.size() / 2];
    }
    return bild;
}

// ═════════════════════════════════════════════════════════════════════════
// M-18 · nonlinear_chain_never_yields_static_eq_claim
// ═════════════════════════════════════════════════════════════════════════
void m18()
{
    abschnitt ("M-18  nonlinear_chain_never_yields_static_eq_claim");
    constexpr double fs = 48000.0;

    // ── Die Gegenproben zuerst: ohne sie sagt kein Fall darunter etwas ────

    const auto identisch = fahreKette (fs, [] (std::uint64_t, double x) { return x; });
    pruefe (identisch.kohaerenz.gesetzt > 20 && identisch.fenster >= 5,
            "genug Baender und Fenster - sonst misst der Rest nichts",
            juce::String (identisch.kohaerenz.gesetzt) + " Baender, "
                + juce::String (identisch.fenster) + " Fenster");
    pruefe (identisch.kohaerenz.anteilUeberSchwelle() > 0.95,
            "identische Kette: die Kohaerenz liegt fast ueberall ueber 0,8",
            juce::String (identisch.kohaerenz.mittel, 4));

    const auto gain = fahreKette (fs, [] (std::uint64_t, double x) { return x * 5.6234; });
    pruefe (gain.kohaerenz.anteilUeberSchwelle() > 0.95,
            "reiner Gain (+15 dB): unveraendert kohaerent - die Kohaerenz ist "
            "pegelunabhaengig, und GENAU deshalb ist ein Transferwert hier zulaessig",
            juce::String (gain.kohaerenz.mittel, 4));

    // Ein linearer Tiefpass: veraendert den Klang massiv, bleibt aber LTI.
    double z1 = 0.0;
    const auto tiefpass = fahreKette (fs, [&z1] (std::uint64_t, double x)
    {
        constexpr double a = 0.08;   // Einpol-Tiefpass, rund 600 Hz
        z1 += a * (x - z1);
        return z1;
    });
    pruefe (tiefpass.kohaerenz.anteilUeberSchwelle() > 0.90,
            "linearer Tiefpass: ebenfalls kohaerent - eine LTI-Kette DARF einen "
            "Frequenzgang tragen, so stark sie den Klang auch aendert",
            juce::String (tiefpass.kohaerenz.mittel, 4));

    const double lineareStreuung = std::max (std::max (identisch.seitenStreuungDb,
                                                       gain.seitenStreuungDb),
                                             tiefpass.seitenStreuungDb);
    pruefe (lineareStreuung < 1.5,
            "und ihre PRE/POST-Relation steht still - eine LTI-Kette haelt sie "
            "ueber alle Fenster konstant",
            juce::String (lineareStreuung, 3) + " dB Streuung");

    // ── Und jetzt die vier adversarialen Ketten aus §38.5 ─────────────────

    // (1) Kompressor: schneller Huellkurvenfolger, 6:1 ueber der Schwelle.
    double huelle = 0.0;
    const auto kompressor = fahreKette (fs, [&huelle] (std::uint64_t, double x)
    {
        const double a = std::abs (x);
        huelle += (a > huelle ? 0.02 : 0.0008) * (a - huelle);
        const double schwelle = 0.15;
        double g = 1.0;
        if (huelle > schwelle)
            g = (schwelle + (huelle - schwelle) / 6.0) / huelle;
        return x * g;
    });

    // (2) Tremolo: Amplitudenmodulation mit 7 Hz.
    const auto tremolo = fahreKette (fs, [fs] (std::uint64_t n, double x)
    {
        const double m = 0.5 + 0.5 * std::sin (kZweiPi * 7.0 * (double) n / fs);
        return x * m;
    });

    // (3) Saturation: harter tanh.
    const auto saettigung = fahreKette (fs, [] (std::uint64_t, double x)
    {
        return std::tanh (6.0 * x) / 6.0;
    });

    // (4) Wechselnde Latenz: der Versatz springt zwischen 0 und 64 Samples.
    std::vector<double> ring ((std::size_t) 256, 0.0);
    std::size_t schreib = 0;
    const auto latenz = fahreKette (fs, [&ring, &schreib, fs] (std::uint64_t n, double x)
    {
        ring[schreib] = x;
        const bool spaet = ((double) n / fs * 3.0) - std::floor ((double) n / fs * 3.0) > 0.5;
        const std::size_t versatz = spaet ? 64u : 0u;
        const std::size_t lese = (schreib + ring.size() - versatz) % ring.size();
        schreib = (schreib + 1) % ring.size();
        return ring[lese];
    });

    // ── Der Befund, und er ist der Grund fuer ZWEI Wachen ─────────────────
    //
    // Drei der vier fallen schon an der Kohaerenz durch. Die Kompression
    // NICHT: sie haelt 0,86 im Mittel und laesst rund vier Fuenftel der
    // Baender ueber der 0,8-Schwelle aus §38.3. Wer M-18 allein an der
    // Kohaerenz aufhaengte, liesse also ausgerechnet das haeufigste Geraet
    // der Kette durch und baute aus ihm einen Frequenzgang.
    //
    // Deshalb existiert die zweite Wache: die Streuung der PRE/POST-Relation
    // ueber die Fenster (`prepost::kettenbefund`, gemessen in
    // `sonde013_prepost.rs`). Der Test misst hier BEIDE Groessen und die
    // Zusage lautet: jede der vier faellt an MINDESTENS einer durch, und
    // keine lineare Kette an einer von beiden.
    struct Fall { const char* name; const Kettenbild& bild; };
    const Fall boese[] = {
        { "Kompression", kompressor }, { "Modulation", tremolo },
        { "Saturation", saettigung },  { "wechselnde Latenz", latenz }
    };
    for (const auto& f : boese)
    {
        const bool kohaerenzFaellt = f.bild.kohaerenz.mittel < identisch.kohaerenz.mittel - 0.05;
        const bool relationWandert = f.bild.seitenStreuungDb > lineareStreuung * 2.0
                                  && f.bild.seitenStreuungDb > 1.5;
        pruefe (kohaerenzFaellt || relationWandert,
                juce::String (f.name) + ": faellt an mindestens einer Wache durch",
                "Kohaerenz " + juce::String (f.bild.kohaerenz.mittel, 3) + ", Streuung "
                    + juce::String (f.bild.seitenStreuungDb, 3) + " dB");
    }

    pruefe (kompressor.kohaerenz.anteilUeberSchwelle() > 0.5
                && kompressor.seitenStreuungDb > 1.5,
            "und der Befund selbst: die Kompression haelt die Kohaerenzschwelle, "
            "wandert aber in der Relation - eine Wache allein genuegt nicht",
            juce::String ((int) (kompressor.kohaerenz.anteilUeberSchwelle() * 100.0))
                + " % ueber 0,8, Streuung " + juce::String (kompressor.seitenStreuungDb, 2) + " dB");

    // Die zusammenfassende Aussage: KEINE der vier erreicht das
    // Kohaerenzbild einer linearen Kette, und keine lineare wandert.
    const double bestesBoeses = std::max (std::max (kompressor.kohaerenz.anteilUeberSchwelle(),
                                                    tremolo.kohaerenz.anteilUeberSchwelle()),
                                          std::max (saettigung.kohaerenz.anteilUeberSchwelle(),
                                                    latenz.kohaerenz.anteilUeberSchwelle()));
    const double schlechtestesGutes = std::min (std::min (identisch.kohaerenz.anteilUeberSchwelle(),
                                                          gain.kohaerenz.anteilUeberSchwelle()),
                                                tiefpass.kohaerenz.anteilUeberSchwelle());
    pruefe (bestesBoeses < schlechtestesGutes,
            "die beiden Gruppen ueberlappen an der Kohaerenz nicht",
            juce::String (bestesBoeses, 3) + " gegen " + juce::String (schlechtestesGutes, 3));
}

// ═════════════════════════════════════════════════════════════════════════
// M-20 · match_gain_is_frozen_before_ab_not_tracked
// ═════════════════════════════════════════════════════════════════════════
void m20()
{
    abschnitt ("M-20  match_gain_is_frozen_before_ab_not_tracked");
    constexpr double fs = 48000.0;
    constexpr int block = 512;

    std::vector<float> a ((std::size_t) block), b ((std::size_t) block);
    auto fuelle = [&] (double faktor, std::uint64_t start)
    {
        for (int i = 0; i < block; ++i)
        {
            const double x = quelle (start + (std::uint64_t) i, fs);
            a[(std::size_t) i] = (float) x;
            b[(std::size_t) i] = (float) (x * faktor);
        }
    };

    Vergleichspegel p;
    p.vorbereiten (fs);
    pruefe (! p.bereit() && ! p.gainGesetzt(),
            "ohne Material ist der Pegel NICHT gesetzt - eine gemeldete 0 dB waere "
            "die Behauptung 'gleich laut'");
    pruefe (! p.friereEin(),
            "und laesst sich nicht einfrieren - ein eingefrorener Wert ohne Messung "
            "waere eine Luege");

    // 0,5 s bei Faktor 2 (+6,02 dB).
    std::uint64_t n = 0;
    for (int i = 0; i < 48; ++i, n += (std::uint64_t) block)
    {
        fuelle (2.0, n);
        p.speise (a.data(), b.data(), block);
    }
    pruefe (p.bereit(), "nach 0,5 s ist genug Material da");
    pruefe (p.friereEin() && p.eingefroren(), "der Pegel friert ein");
    const double eingefroren = p.gainDb();
    pruefe (std::abs (eingefroren - 6.0206) < 0.05,
            "und trifft den bekannten Gain", juce::String (eingefroren, 4) + " dB");

    // ── Die eigentliche Zusage: waehrend des A/B aendert er sich NICHT ────
    for (int i = 0; i < 600; ++i, n += (std::uint64_t) block)
    {
        fuelle (8.0, n);                    // +18 dB, sechseinhalb Sekunden lang
        p.speise (a.data(), b.data(), block);
    }
    pruefe (p.gainDb() == eingefroren,
            "sechs Sekunden mit +18 dB aendern ihn um kein Tausendstel - eine "
            "mitlaufende Regelung wuerde selbst zum hoerbaren Prozessor",
            juce::String (p.gainDb(), 6) + " dB");

    // Stille ebenso: sie ist der Fall, bei dem eine Nachregelung am
    // schlimmsten irrt.
    for (auto& v : a) v = 0.0f;
    for (auto& v : b) v = 0.0f;
    for (int i = 0; i < 200; ++i)
        p.speise (a.data(), b.data(), block);
    pruefe (p.gainDb() == eingefroren, "und zwei Sekunden Stille auch nicht");

    // Ein Passagenwechsel loescht ihn bewusst - ein Pegel gehoert zu GENAU
    // einer Passage.
    p.loeschen();
    pruefe (! p.eingefroren() && ! p.gainGesetzt(),
            "erst ein ausdruecklicher Passagenwechsel loescht ihn");

    // Nicht-endliche Werte werden verriegelt, nie gezaehlt.
    Vergleichspegel q;
    q.vorbereiten (fs);
    for (int i = 0; i < 48; ++i, n += (std::uint64_t) block)
    {
        fuelle (2.0, n);
        a[0] = std::numeric_limits<float>::quiet_NaN();
        b[1] = std::numeric_limits<float>::infinity();
        q.speise (a.data(), b.data(), block);
    }
    // ⚠️ `friereEin()` steht bewusst VOR dem `pruefe`-Aufruf. Es hat einen
    // Seiteneffekt, und C++ legt die Auswertungsreihenfolge von
    // Funktionsargumenten nicht fest: im selben Aufruf las der Zusatztext
    // `gainDb()` vor dem Einfrieren und meldete 0,0000 dB neben einem gruenen
    // Haken. Gemessen beim Bau von Etappe F.
    const bool gefroren = q.friereEin();
    pruefe (gefroren && std::abs (q.gainDb() - 6.0206) < 0.1,
            "NaN und Inf im Material verriegeln, statt den Pegel zu vergiften",
            juce::String (q.gainDb(), 4) + " dB");
}

// ═════════════════════════════════════════════════════════════════════════
// M-24 · audible_delta_locked_without_compare_routing
// ═════════════════════════════════════════════════════════════════════════
void m24()
{
    abschnitt ("M-24  audible_delta_locked_without_compare_routing");

    // Drei binaere Eingaben, also acht Faelle - der Raum wird VOLLSTAENDIG
    // abgefahren statt an drei handverlesenen Zeilen geprueft.
    int frei = 0, gesperrt = 0;
    bool ohneRoutingImmerGesperrt = true;
    for (int m = 0; m < 8; ++m)
    {
        const bool routing   = (m & 1) != 0;
        const bool alignment = (m & 2) != 0;
        const bool lag       = (m & 4) != 0;
        const auto z = deltaZustand (routing, alignment, lag);
        if (z == Deltasperre::frei) ++frei; else ++gesperrt;
        if (! routing && z != Deltasperre::keinComparerouting)
            ohneRoutingImmerGesperrt = false;
    }
    pruefe (ohneRoutingImmerGesperrt,
            "ohne nachgewiesenes Compare-Routing ist Delta in ALLEN vier "
            "Kombinationen gesperrt - und zwar mit genau diesem Grund");
    pruefe (frei == 1 && gesperrt == 7,
            "genau EINE der acht Kombinationen ist frei: alle drei Nachweise zusammen",
            juce::String (frei) + " frei, " + juce::String (gesperrt) + " gesperrt");

    // Die Reihenfolge der Gruende ist die der Haerte. Wer sie umdrehte,
    // meldete bei fehlendem Routing einen Lag-Grund und legte nahe, ein
    // laengerer Lag wuerde helfen.
    pruefe (deltaZustand (false, false, false) == Deltasperre::keinComparerouting
                && deltaZustand (true, false, false) == Deltasperre::alignmentUnzureichend
                && deltaZustand (true, true, false) == Deltasperre::lagTraegtGruppenlaufzeitNicht
                && deltaZustand (true, true, true) == Deltasperre::frei,
            "die Gruende kommen in der Reihenfolge ihrer Haerte");

    // Und in DIESER Phase gibt es kein Compare-Routing. Die Konstante steht
    // an genau einer Stelle, damit die spaetere Freischaltung ein sichtbarer
    // Schritt ist statt einer verstreuten Bedingung.
    pruefe (! nakama::analyse::kCompareroutingInDieserPhase,
            "in P4 gibt es kein Compare-Routing (§38.4)");
    pruefe (deltaZustand (nakama::analyse::kCompareroutingInDieserPhase, true, true)
                == Deltasperre::keinComparerouting,
            "also ist hoerbares Delta gesperrt, egal wie gut Alignment und Lag sind",
            sperrName (deltaZustand (nakama::analyse::kCompareroutingInDieserPhase, true, true)));

    // Jeder Grund traegt ein Wort - ein gesperrter Zustand ohne Grund waere
    // ein totes Element.
    for (auto z : { Deltasperre::keinComparerouting, Deltasperre::alignmentUnzureichend,
                    Deltasperre::lagTraegtGruppenlaufzeitNicht, Deltasperre::frei })
        pruefe (juce::String (sperrName (z)).isNotEmpty() && juce::String (sperrName (z)) != "unbekannt",
                juce::String ("der Zustand traegt ein Wort: ") + sperrName (z));
}

} // namespace

int main()
{
    std::cout << "== Nakama SONDE-013 - PRE/POST auf der Audioseite (§38.3, §38.4) =="
              << std::endl;
    m18();
    m20();
    m24();
    std::cout << std::endl << bestanden << " bestanden, " << fehler << " gescheitert"
              << std::endl;
    return fehler == 0 ? 0 : 1;
}
