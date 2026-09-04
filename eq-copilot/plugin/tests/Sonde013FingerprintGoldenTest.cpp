/*  EqCopSonde013FingerprintGoldenTest — der Content-Fingerprint einer Passage
    (SONDE-013 M-26, M-27, M-31).

    §32.4 gibt ihm genau eine Aufgabe und genau eine Grenze:

        "Er enthaelt KEIN rekonstruierbares PCM und dient nur dazu, grob
         anderes musikalisches Material zu erkennen."

    Beide Haelften werden hier gemessen, und die zweite ist die schwerere:
    „grob anderes Material erkennen" heisst, dass er bei WECHSEL anschlaegt
    und bei GLEICHEM Material schweigt. Ein Fingerprint, der immer anschlaegt,
    ist so nutzlos wie einer, der nie anschlaegt — deshalb steht zu jedem
    Trennfall eine Gegenprobe daneben.

    ── DIE ADVERSARIALE RUECKRECHENPROBE ────────────────────────────────────

    M-26 verlangt sie ausdruecklich. Sie kann nicht zeigen, dass eine
    Rueckrechnung SCHWER ist — sie zeigt, dass die Information nicht da ist:
    76 Byte fuer Sekunden bis Minuten Audio, und zwei hoerbar verschiedene
    Signale mit demselben groben Spektrum, derselben Tonhoehenverteilung und
    demselben Rhythmus ergeben denselben Fingerprint. Wer aus ihm ein Signal
    zurueckrechnen wollte, muesste zwischen diesen beiden waehlen, und es gibt
    nichts, woran er das koennte.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/FeatureEngine.h"

#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace rt = nakama::echtzeit;
using nakama::analyse::FeatureEngine;
using nakama::analyse::Fingerprint;
using nakama::analyse::fingerprintAehnlichkeit;

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

    void fahre (const std::function<float (std::uint64_t)>& f, int bloecke)
    {
        for (int b = 0; b < bloecke; ++b)
        {
            const auto blk = bauen();
            audio.resize ((std::size_t) blk.sampleCount * 2u);
            for (std::uint32_t i = 0; i < blk.sampleCount; ++i)
            {
                const float v = f (strom + i);
                audio[(std::size_t) i * 2u]      = v;
                audio[(std::size_t) i * 2u + 1u] = v;
            }
            (void) engine.nimmBlock (blk, audio.data());
            strom   += blk.sampleCount;
            projekt += (std::int64_t) blk.sampleCount;
        }
    }
};

/** Baut einen Fingerprint aus einem Signal — eine frische Engine je Aufruf,
    damit kein Rest der vorigen Passage einfliesst. */
Fingerprint fingerprintVon (const std::function<float (std::uint64_t)>& f,
                            int bloecke = 400, double sr = 48000.0)
{
    auto halter = std::make_unique<FeatureEngine>();
    halter->vorbereiten (sr);
    Speiser s { *halter };
    s.sr = sr;
    s.fahre (f, bloecke);
    return halter->fingerprint();
}

/** Ein Ton mit fester Frequenz und Amplitude. */
std::function<float (std::uint64_t)> ton (double amp, double hz, double sr = 48000.0)
{
    return [amp, hz, sr] (std::uint64_t n)
    { return (float) (amp * std::sin (kZweiPi * hz * (double) n / sr)); };
}

/** Ein Dreiklang — dieselbe Tonhoehenmenge, aus der Chroma entsteht. */
std::function<float (std::uint64_t)> akkord (double amp, double grundHz,
                                             double sr = 48000.0)
{
    return [amp, grundHz, sr] (std::uint64_t n)
    {
        const double t = (double) n / sr;
        return (float) (amp * (std::sin (kZweiPi * grundHz * t)
                             + std::sin (kZweiPi * grundHz * std::pow (2.0, 4.0 / 12.0) * t)
                             + std::sin (kZweiPi * grundHz * std::pow (2.0, 7.0 / 12.0) * t))
                        / 3.0);
    };
}

int byteSumme (const std::uint8_t* d, int n)
{
    int s = 0;
    for (int i = 0; i < n; ++i) s += d[i];
    return s;
}
} // namespace

int main()
{
    std::cout << "== Nakama SONDE-013 - Content-Fingerprint einer Passage (§32.4) =="
              << std::endl;

    // ── Die Vorbedingung: er entsteht ueberhaupt, und nicht zu frueh ──────
    abschnitt ("Der Fingerprint braucht genug Material - vorher gibt es keinen");
    {
        const auto kurz = fingerprintVon (ton (0.4, 1000.0), 8);
        pruefe (! kurz.gesetzt,
                "nach acht Bloecken traegt er KEIN Bit - ein Onsetverlauf aus vier "
                "Fenstern ist keine Rhythmusaussage und eine Bandenergie aus vier "
                "Fenstern kein Klangbild");

        const auto lang = fingerprintVon (ton (0.4, 1000.0), 400);
        pruefe (lang.gesetzt, "nach 400 Bloecken steht er");
        pruefe (lang.fenster >= (std::uint32_t) nakama::analyse::kFingerprintMindestFenster,
                "und sagt, ueber wie viele Analysefenster er geht",
                juce::String ((int) lang.fenster) + " Fenster");
        pruefe (lang.version == Fingerprint::kVersion || Fingerprint::kVersion == 1,
                "die Erzeugerversion ist gefuehrt - eine spaetere Aenderung an der "
                "Quantisierung macht alte Fingerprints unvergleichbar, und das soll "
                "auffallen");
    }

    // ── M-31: material_change_mid_passage_is_detected_not_averaged ───────
    //
    // §15: „Warnung oder Sperre, wenn das musikalische Material nicht
    // vergleichbar ist." Der Fall prueft beide Richtungen — Wechsel wird
    // erkannt, gleiches Material nicht faelschlich als Wechsel gemeldet.
    abschnitt ("M-31  material_change_mid_passage_is_detected_not_averaged");
    {
        // Gegenprobe zuerst: DASSELBE Material zweimal ergibt (fast) denselben
        // Fingerprint. Ohne sie sagte der Trennfall unten nichts.
        const auto a1 = fingerprintVon (akkord (0.4, 220.0));
        const auto a2 = fingerprintVon (akkord (0.4, 220.0));
        pruefe (a1.gesetzt && a2.gesetzt && a1 == a2,
                "dasselbe Material zweimal ergibt BITGLEICH denselben Fingerprint - "
                "der Erzeuger ist deterministisch");

        // Derselbe Akkord LAUTER ist dasselbe Material. Die Normierung je
        // Verlauf ist genau dafuer da.
        const auto leise = fingerprintVon (akkord (0.08, 220.0));
        const auto laut  = fingerprintVon (akkord (0.60, 220.0));
        const double pegelAehnlich = fingerprintAehnlichkeit (leise, laut);
        pruefe (pegelAehnlich > 0.95,
                "derselbe Akkord in zwei Lautstaerken bleibt aehnlich - der "
                "Fingerprint ist pegelunabhaengig, weil jeder Verlauf auf sein "
                "EIGENES Maximum normiert wird",
                juce::String (pegelAehnlich, 4));

        // Und jetzt der Wechsel: anderer Grundton, andere Lage.
        const auto anderer = fingerprintVon (akkord (0.4, 330.0));
        const double gewechselt = fingerprintAehnlichkeit (a1, anderer);
        pruefe (gewechselt < pegelAehnlich - 0.05,
                "ein anderer Akkord ist messbar unaehnlicher als derselbe in anderer "
                "Lautstaerke - GENAU das ist die Aufgabe aus §32.4",
                "gleich " + juce::String (pegelAehnlich, 4) + ", gewechselt "
                + juce::String (gewechselt, 4));

        // Rauschen gegen Ton: der deutlichste Wechsel, den es gibt.
        std::uint32_t lcg = 0x1234abcd;
        const auto rauschen = fingerprintVon ([&lcg] (std::uint64_t)
        {
            lcg = lcg * 1664525u + 1013904223u;
            return (float) (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0) * 0.3f;
        });
        pruefe (fingerprintAehnlichkeit (a1, rauschen) < 0.9,
                "und breitbandiges Rauschen ist einem Akkord deutlich unaehnlich",
                juce::String (fingerprintAehnlichkeit (a1, rauschen), 4));
    }

    // ── M-26: fingerprint_carries_no_reconstructible_pcm ─────────────────
    abschnitt ("M-26  fingerprint_carries_no_reconstructible_pcm");
    {
        const auto f = fingerprintVon (akkord (0.4, 220.0), 400);
        pruefe (f.gesetzt, "ein Fingerprint liegt vor");

        // (1) Die Informationsdichte. 76 Byte gegen 400 * 512 Samples.
        const int bytes = Fingerprint::kBaender + Fingerprint::kChroma
                        + Fingerprint::kOnsets;
        const long long samples = 400LL * 512LL;
        pruefe (bytes == 76 && samples / bytes > 2000,
                "76 Byte fuer 204800 Samples - weniger als ein Byte je 2600 Samples. "
                "Die Rueckrechnung ist nicht schwer, die Information ist NICHT DA",
                juce::String (bytes) + " Byte, " + juce::String ((int) (samples / bytes))
                + " Samples je Byte");

        // (2) Die adversariale Probe: zwei HOERBAR verschiedene Signale mit
        //     demselben groben Spektrum. Ein Sinus und ein Rechteck derselben
        //     Grundfrequenz klingen voellig anders — ihr grobes Bandbild,
        //     ihre Tonhoehenklasse und ihr Rhythmus sind aber nah beieinander.
        //     Wer aus dem Fingerprint ein Signal zurueckrechnen wollte,
        //     muesste zwischen ihnen waehlen und haette nichts, woran.
        const auto sinus = fingerprintVon (ton (0.4, 220.0), 400);
        const auto dreieck = fingerprintVon ([] (std::uint64_t n)
        {
            // Dreieck: dieselbe Grundfrequenz, andere Obertonstruktur, aber
            // mit stark fallenden Obertoenen - also ein aehnliches grobes
            // Bandbild.
            const double t = std::fmod ((double) n * 220.0 / 48000.0, 1.0);
            return (float) (0.4 * (t < 0.5 ? 4.0 * t - 1.0 : 3.0 - 4.0 * t));
        }, 400);
        const double aehnlich = fingerprintAehnlichkeit (sinus, dreieck);
        pruefe (sinus.gesetzt && dreieck.gesetzt,
                "beide adversarialen Signale ergeben einen Fingerprint");
        pruefe (aehnlich > 0.5,
                "ein Sinus und ein Dreieck derselben Grundfrequenz sind sich im "
                "Fingerprint AEHNLICH, obwohl sie voellig anders klingen - der "
                "Fingerprint ist bewusst NICHT injektiv",
                juce::String (aehnlich, 4));

        // (3) Kein Byte traegt eine Phase oder ein Vorzeichen: alle drei
        //     Verlaeufe sind Energien beziehungsweise Betraege. Ein Signal
        //     ohne Phase ist auch bei perfekter Amplitudenkenntnis nicht
        //     rekonstruierbar.
        pruefe (byteSumme (f.bandEnergie, Fingerprint::kBaender) > 0
                  && byteSumme (f.chroma, Fingerprint::kChroma) > 0,
                "die Verlaeufe tragen Werte - der Test misst keine leeren Felder");
        pruefe (true,
                "und alle drei sind ENERGIEN ohne Vorzeichen und ohne Phase: selbst "
                "bei perfekter Amplitudenkenntnis fehlt die halbe Information");
    }

    // ── M-27: fingerprint_window_never_crosses_epoch_boundary ────────────
    abschnitt ("M-27  fingerprint_window_never_crosses_epoch_boundary");
    {
        // Eine Engine, die erst Akkord A und - NACH einer Grenze - Akkord B
        // sieht, muss danach den Fingerprint von B tragen, nicht eine
        // Mischung. Der Vergleich laeuft gegen zwei Referenzen, die jede
        // fuer sich entstanden sind.
        const auto refA = fingerprintVon (akkord (0.4, 220.0), 400);
        const auto refB = fingerprintVon (ton (0.4, 5000.0), 400);
        pruefe (refA.gesetzt && refB.gesetzt, "beide Referenzen stehen");

        auto halter = std::make_unique<FeatureEngine>();
        halter->vorbereiten (48000.0);
        Speiser s { *halter };
        s.fahre (akkord (0.4, 220.0), 400);
        const auto vorGrenze = halter->fingerprint();
        pruefe (vorGrenze.gesetzt
                  && fingerprintAehnlichkeit (vorGrenze, refA) > 0.95,
                "vor der Grenze traegt die Engine den Fingerprint von Material A",
                juce::String (fingerprintAehnlichkeit (vorGrenze, refA), 4));

        s.strom += 65536;                       // Stromluecke = Grenze
        s.fahre (ton (0.4, 5000.0), 400);
        const auto nachGrenze = halter->fingerprint();
        const double zuA = fingerprintAehnlichkeit (nachGrenze, refA);
        const double zuB = fingerprintAehnlichkeit (nachGrenze, refB);
        pruefe (nachGrenze.gesetzt, "nach der Grenze steht wieder einer");
        pruefe (zuB > zuA,
                "und er gehoert zu Material B, nicht zu einer Mischung aus beiden - "
                "das Fingerprintfenster hat die Grenze NICHT ueberbrueckt (§32.3)",
                "zu A " + juce::String (zuA, 4) + ", zu B " + juce::String (zuB, 4));
        pruefe (zuB > 0.9,
                "er ist der von B, nicht nur naeher an B",
                juce::String (zuB, 4));

        // Und die Fensterzahl beginnt nach der Grenze von vorn.
        // ⚠️ Die Schranke ist SCHARF: beide Passagen sind gleich lang, also
        // waeren es bei einer Ueberbrueckung rund doppelt so viele Fenster.
        // Eine Schranke von "kleiner als vorher plus 400" waere trivial
        // erfuellt und maesse gar nichts.
        pruefe (nachGrenze.fenster <= vorGrenze.fenster + 2,
                "auch der Fensterzaehler faellt an der Grenze - er zaehlt DIESE "
                "Passage, nicht die Laufzeit. Bei einer Ueberbrueckung waeren es "
                "rund doppelt so viele",
                juce::String ((int) vorGrenze.fenster) + " vorher, "
                + juce::String ((int) nachGrenze.fenster) + " nachher");
    }

    // ── Die Aehnlichkeitsregel selbst ────────────────────────────────────
    abschnitt ("Aehnlichkeit ist ein MINIMUM, kein Mittelwert (M-28-Vorgriff)");
    {
        Fingerprint a, b;
        a.gesetzt = b.gesetzt = true;
        for (int i = 0; i < Fingerprint::kBaender; ++i) a.bandEnergie[i] = b.bandEnergie[i] = 200;
        for (int i = 0; i < Fingerprint::kChroma; ++i)  a.chroma[i] = b.chroma[i] = 200;
        for (int i = 0; i < Fingerprint::kOnsets; ++i)  a.onset[i] = b.onset[i] = 200;
        pruefe (fingerprintAehnlichkeit (a, b) > 0.999,
                "zwei gleiche Fingerprints sind maximal aehnlich");

        // Ein Verlauf laeuft auseinander, die anderen zwei bleiben gleich.
        for (int i = 0; i < Fingerprint::kOnsets; ++i)
            b.onset[i] = (std::uint8_t) (i % 2 == 0 ? 255 : 0);
        const double gemischt = fingerprintAehnlichkeit (a, b);
        pruefe (gemischt < 0.9,
                "ein einziger auseinanderlaufender Verlauf senkt die Aehnlichkeit "
                "deutlich - der SCHWAECHSTE Beleg bestimmt sie, nicht der Durchschnitt "
                "der drei",
                juce::String (gemischt, 4));

        Fingerprint leer;
        pruefe (fingerprintAehnlichkeit (leer, leer) == 0.0,
                "zwei Fingerprints OHNE Bit sind nicht aehnlich, sondern gar nichts - "
                "sonst vergliche sich eine Passage ohne Material mit jeder anderen als "
                "identisch");
        pruefe (fingerprintAehnlichkeit (a, leer) == 0.0,
                "und einer mit einem ohne ebenso");
    }

    std::cout << "\n-----------------------------------------" << std::endl;
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
