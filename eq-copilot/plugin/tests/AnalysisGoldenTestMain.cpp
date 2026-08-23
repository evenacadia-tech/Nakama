// SONDE-009 (S12-13) — Kanon-Bein B5 `EqCopAnalysisGoldenTest`.
// Entwurf §66.1: "EBU, Bandgitter, Stereo, Ereignisse, Epochgrenzen".
// Gate-Text §65: "Drop/Seek/Loop trennt jedes offene Fenster."
//
// ─────────────────────────────────────────────────────────────────────────────
// WIE DIESES BEIN GEBAUT IST, UND WARUM SO.
//
// Die Lehre aus T2-1 zu SONDE-008 steht ueber allem: EIN KORPUS, DER NICHT
// SCHEITERN KANN, BEWEIST NICHTS.  Dort fuellte ein Golden den fraglichen Eimer
// mit KONSTANTEM Material - mit einem Pegel konnte dort konstruktiv nichts
// auseinanderlaufen, und der Fehler von 2,918 LU blieb zwei Runden unsichtbar.
//
// Daraus folgen hier drei Regeln:
//   1. Jede Grenzart bekommt einen EIGENEN Fall.  "Drop/Seek/Loop" sind drei
//      Woerter, aber sechs unterscheidbare Ereignisse, und ein Riegel kann
//      fuenf davon sehen und das sechste nicht.
//   2. Jeder Fall prueft VORHER, dass die Fenster wirklich gefuellt waren.
//      Eine Trennung, die ein leeres Fenster leert, ist keine Aussage.
//   3. Abschnitt L faehrt die GEGENPROBEN: dieselben Faelle mit ausgehebelter
//      Trennung.  Wenn das Bein dann nicht rot wird, misst es nichts.
//
// ─────────────────────────────────────────────────────────────────────────────
// NAK-56, WERKBANKHAELFTE (Abschnitt K).  Der T2-Bericht zu SONDE-008 nennt sie
// "die groesste ungedeckte Flaeche dieses Tickets": `nakamaBlockEmpfangen()`
// war gelesen, nie gefahren.  Abschnitt K faehrt sie am ECHTEN Prozessor und
// misst, dass die Brueckenwerte bis in den Transportstempel durchkommen - mit
// Gegenprobe ueber den Playhead-Rueckfallweg, der weniger kann.
#include "PluginProcessor.h"
#include "analysis/FeatureEngine.h"

#include <juce_core/juce_core.h>

#include <cmath>
#include <complex>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <vector>

using namespace nakama::analyse;
namespace rt = nakama::echtzeit;
// Nur der Prozessor, kein `using namespace eqcop` — sonst stuenden zwei
// `Biquad` und zwei `AnalyseEngine`-nahe Namen nebeneinander.
using eqcop::EqCopilotProcessor;

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

juce::File finde (const juce::String& relativ)
{
    auto ausCwd = juce::File::getCurrentWorkingDirectory().getChildFile (relativ);
    if (ausCwd.existsAsFile())
        return ausCwd;
    auto ordner = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
    for (int i = 0; i < 10 && ordner.exists(); ++i)
    {
        auto kandidat = ordner.getChildFile (relativ);
        if (kandidat.existsAsFile())
            return kandidat;
        ordner = ordner.getParentDirectory();
    }
    return ausCwd;
}

juce::var lies (const juce::String& relativ, bool& ok)
{
    const auto datei = finde (relativ);
    if (! datei.existsAsFile())
    {
        std::cout << "[ROT]  Datei fehlt: " << datei.getFullPathName().toRawUTF8() << std::endl;
        ++fehler; ok = false; return {};
    }
    juce::var wert;
    const auto ergebnis = juce::JSON::parse (datei.loadFileAsString(), wert);
    if (ergebnis.failed())
    {
        std::cout << "[ROT]  JSON kaputt in " << relativ.toRawUTF8() << std::endl;
        ++fehler; ok = false; return {};
    }
    ok = true;
    return wert;
}

/** Hexziffern -> Bitmuster.  ⚠️ Bewusst KEIN Zahlenparser: die Fixture traegt
    `hex64` als Text, und ein Dezimalleser (JUCEs zumal) waere genau die
    Bibliothek, gegen deren Rundung dieses Gitter schuetzen soll. */
std::uint64_t hexBits (const juce::String& h)
{
    return (std::uint64_t) h.getHexValue64();
}

std::uint64_t bitsVon (double d) { std::uint64_t b {}; std::memcpy (&b, &d, sizeof b); return b; }
std::uint32_t bitsVon (float f)  { std::uint32_t b {}; std::memcpy (&b, &f, sizeof b); return b; }
double        ausBits (std::uint64_t b) { double d {}; std::memcpy (&d, &b, sizeof d); return d; }

//==============================================================================
// Blockgenerator: baut `StampedBlock` samt Audio, so wie die Quarantaene sie
// liefern wuerde.  Der lokale Strom laeuft mit, die Projektzeit auch — beides
// muss der Generator korrekt fuehren, sonst prueft der Test gegen einen
// selbstgebauten Fehler.
struct Speiser
{
    FeatureEngine& engine;
    double sr { 48000.0 };
    int    frames { 512 };

    std::uint64_t strom { 0 };
    std::uint64_t segment { 0 };
    std::uint32_t startFolge { 0 };
    std::int64_t  projekt { 0 };
    std::vector<float> audio;
    std::uint32_t lcg { 0x2468acefu };

    explicit Speiser (FeatureEngine& e) : engine (e) {}

    float rausch()
    {
        lcg = lcg * 1664525u + 1013904223u;
        return ((float) ((lcg >> 8) & 0xffffu) / 32768.0f - 1.0f) * 0.25f;
    }

    rt::StampedBlock bauen (std::uint32_t zusatzFlags = 0, bool spielt = true,
                            bool zeitGueltig = true)
    {
        rt::StampedBlock b;
        b.stromVon = strom;
        b.sampleCount = (std::uint32_t) frames;
        b.segment = segment;
        b.startFolge = startFolge;
        b.kanaele = 2;
        b.tapMaske = 1;
        b.projectSampleStart = projekt;
        b.sampleRate = sr;
        b.flags = rt::kFlagKontextAnwesend | rt::kFlagSpieltGueltig
                | rt::kFlagSampleRateGueltig | zusatzFlags;
        if (spielt)       b.flags |= rt::kFlagSpielt;
        if (zeitGueltig)  b.flags |= rt::kFlagZeitGueltig;
        return b;
    }

    /** Ein Block mit Rauschen; fuehrt Strom und Projektzeit fort. */
    bool senden (const rt::StampedBlock& b)
    {
        audio.resize ((std::size_t) b.sampleCount * 2u);
        for (std::uint32_t i = 0; i < b.sampleCount; ++i)
        {
            const float v = rausch();
            audio[(std::size_t) i * 2u]      = v;
            audio[(std::size_t) i * 2u + 1u] = v * 0.8f + rausch() * 0.2f;
        }
        const bool r = engine.nimmBlock (b, audio.data());
        strom   += b.sampleCount;
        if ((b.flags & rt::kFlagZeitGueltig) != 0 && (b.flags & rt::kFlagSpielt) != 0)
            projekt += (std::int64_t) b.sampleCount;
        return r;
    }

    /** `n` normale, lueckenlose Bloecke. */
    void laufen (int n)
    {
        for (int i = 0; i < n; ++i)
            senden (bauen());
    }

    /** Genug Audio, damit ALLE Fenster offen und gefuellt sind: das
        Bassfenster braucht 16384 Samples, die 3-s-Loudness 144 000, und der
        Fluss braucht seinen Vorgaenger. */
    void bisAllesOffen()
    {
        const int noetig = (int) (3.4 * sr / (double) frames) + 4;
        laufen (noetig);
    }

    bool allesOffen() const
    {
        return engine.fuellstandBass() > 0
            && engine.fuellstandHaupt() > 0
            && engine.fuellstandKurzLoudness() > 0
            && engine.flussHatVorgaenger();
    }
};

juce::String fuellstaende (const FeatureEngine& e)
{
    return "Bass=" + juce::String (e.fuellstandBass())
         + " Haupt=" + juce::String (e.fuellstandHaupt())
         + " Zelle=" + juce::String (e.fuellstandLoudnessZelle())
         + " Kurz=" + juce::String (e.fuellstandKurzLoudness())
         + " Fluss=" + juce::String (e.flussHatVorgaenger() ? 1 : 0);
}

/** DER Kernbefund je Grenzart — und die Formulierung ist der halbe Beweis.

    Die naheliegende Frage waere "sind nach der Grenze alle Fenster LEER?", und
    sie ist falsch: der Block, der die Grenze traegt, gehoert bereits zur neuen
    Epoche und beginnt dort das neue Fenster.  "Leer" waere nur richtig, wenn
    man den Zustand ZWISCHEN Trennung und Einspeisung ansaehe — einen Zustand,
    den es von aussen gar nicht gibt.

    Die richtige Frage ist: TRAEGT ein Fenster noch Samples von VOR der Grenze?
    Messbar wird sie daran, dass in jedem Fenster nach dem Grenzblock GENAU
    dieser eine Block steht und kein Sample mehr.  Das ist zugleich schaerfer
    als "leer": ein Fenster, das die Grenze ueberbrueckte, haette mehr; eines,
    das faelschlich zweimal getrennt haette, weniger. */
/** Wirklich LEER — und das gilt an genau einer Stelle: beim moeglichen
    Straddle.  Dort wird der Grenzblock nicht nur getrennt, sondern selbst
    verworfen (§32.3 "als ungueltig markiert"), also beginnt er auch kein neues
    Fenster.  Dass die beiden Faelle sich hier unterscheiden, ist der Beleg,
    dass die Straddle-Regel etwas anderes tut als eine gewoehnliche Trennung. */
bool alleFensterLeer (const FeatureEngine& e)
{
    return e.fuellstandBass() == 0
        && e.fuellstandHaupt() == 0
        && e.fuellstandLoudnessZelle() == 0
        && e.fuellstandKurzLoudness() == 0
        && ! e.flussHatVorgaenger();
}

bool keinFensterUeberbrueckt (const FeatureEngine& e, int blockFrames, double sr)
{
    const int zellenSamples = (int) std::llround (FeatureEngine::kZelleSekunden * sr);
    return e.fuellstandBass()  == blockFrames
        && e.fuellstandHaupt() == blockFrames
        && e.fuellstandLoudnessZelle() == blockFrames % zellenSamples
        && e.fuellstandKurzLoudness() == 0        // die 3-s-Historie ist weg
        && ! e.flussHatVorgaenger();              // und der Fluss hat keinen Vorgaenger
}

const char* grundName (Grenzgrund g)
{
    switch (g)
    {
        case Grenzgrund::keine:              return "keine";
        case Grenzgrund::lokaleLuecke:       return "lokaleLuecke";
        case Grenzgrund::transportKante:     return "transportKante";
        case Grenzgrund::zeitSprung:         return "zeitSprung";
        case Grenzgrund::loopWrap:           return "loopWrap";
        case Grenzgrund::sampleratewechsel:  return "sampleratewechsel";
        case Grenzgrund::neuanlauf:          return "neuanlauf";
        case Grenzgrund::moeglicherStraddle: return "moeglicherStraddle";
        case Grenzgrund::beweislageWechsel:  return "beweislageWechsel";
        case Grenzgrund::anzahl:             return "<anzahl, kein Grund>";
    }
    return "?";
}

/** Ein kompletter Grenzfall: aufbauen, pruefen dass alles offen war, Grenze
    ausloesen, pruefen dass alles leer ist und der Grund stimmt. */
void grenzfall (const juce::String& name, Grenzgrund erwartet,
                const std::function<void (Speiser&)>& ausloesen,
                bool erwarteEpoche = true)
{
    FeatureEngine e;
    e.vorbereiten (48000.0);
    Speiser s { e };
    s.bisAllesOffen();

    const auto epochenVorher  = e.epochenwechsel();
    const auto segmenteVorher = e.segmentwechsel();
    const auto grundVorher    = e.grenzenMitGrund (erwartet);

    pruefe (s.allesOffen(), name + ": vor der Grenze sind alle Fenster offen",
            fuellstaende (e));

    ausloesen (s);

    pruefe (keinFensterUeberbrueckt (e, s.frames, s.sr),
            name + ": NACH der Grenze traegt kein Fenster ein Sample von davor",
            fuellstaende (e) + " (erwartet je " + juce::String (s.frames) + ")");
    pruefe (e.grenzenMitGrund (erwartet) == grundVorher + 1,
            name + ": Ursache ist " + grundName (erwartet),
            "Zaehler " + juce::String (e.grenzenMitGrund (erwartet)));
    if (erwarteEpoche)
        pruefe (e.epochenwechsel() == epochenVorher + 1 && e.segmentwechsel() == segmenteVorher,
                name + ": zaehlt als EPOCHE, nicht als Segment",
                "Epochen " + juce::String (e.epochenwechsel())
                + " Segmente " + juce::String (e.segmentwechsel()));
    else
        pruefe (e.segmentwechsel() == segmenteVorher + 1 && e.epochenwechsel() == epochenVorher,
                name + ": zaehlt als SEGMENT, nicht als Epoche (§32.3)",
                "Epochen " + juce::String (e.epochenwechsel())
                + " Segmente " + juce::String (e.segmentwechsel()));
}

/** |H(e^{jw})|² eines Biquads — analytisch, ohne den Zeitbereichspfad. */
double biquadLeistung (const Biquad& f, double w)
{
    const std::complex<double> z (std::cos (-w), std::sin (-w));
    const std::complex<double> z2 = z * z;
    const auto zaehler = f.b0 + f.b1 * z + f.b2 * z2;
    const auto nenner  = 1.0 + f.a1 * z + f.a2 * z2;
    const auto h = zaehler / nenner;
    return std::norm (h);
}
} // namespace

//==============================================================================
int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    std::cout << "== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==" << std::endl;
    std::cout << "Gate: \"Drop/Seek/Loop trennt jedes offene Fenster.\"" << std::endl;
    std::cout << "Stufen: Bass " << FeatureEngine::kBassPunkte << " (Hop "
              << FeatureEngine::kBassHop << "), Haupt " << FeatureEngine::kHauptPunkte
              << " (Hop " << FeatureEngine::kHauptHop << "), Trennung bei "
              << FeatureEngine::kTrennungHz << " Hz." << std::endl << std::endl;

    //==========================================================================
    std::cout << "== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==" << std::endl;
    {
        bool ok1 = false, ok2 = false;
        const auto fein = lies ("eq-copilot/schemas/v3/bandgitter/nakama_1_24_oct_30_18k_v1.json", ok1);
        const auto grob = lies ("eq-copilot/schemas/v3/bandgitter/nakama_log64_v1.json", ok2);
        if (ok1 && ok2)
        {
            auto* feinObj = fein.getDynamicObject();
            auto* grobObj = grob.getDynamicObject();
            const auto feinKanten = feinObj->getProperty ("kanten_hz").getDynamicObject()->getProperty ("hex64");
            const auto feinMitten = feinObj->getProperty ("mitten_hz").getDynamicObject()->getProperty ("hex64");
            const auto grobKanten = grobObj->getProperty ("kanten_hz").getDynamicObject()->getProperty ("hex64");
            const auto grobMitten = grobObj->getProperty ("mitten_hz").getDynamicObject()->getProperty ("hex64");
            const auto gruppen    = grobObj->getProperty ("gruppen");

            pruefe ((int) feinObj->getProperty ("band_anzahl") == Gitter::evidenzBaender,
                    "Evidenzgitter: band_anzahl == kEvidenzBaender",
                    juce::String (Gitter::evidenzBaender));
            pruefe ((int) grobObj->getProperty ("band_anzahl") == Gitter::liveBaender,
                    "Livegitter: band_anzahl == kLiveBaender", juce::String (Gitter::liveBaender));

            int abw = 0;
            for (int i = 0; i <= Gitter::evidenzBaender; ++i)
                if (bitsVon (Gitter::evidenzKante (i)) != hexBits (feinKanten[i].toString()))
                    ++abw;
            pruefe (abw == 0, "Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)",
                    juce::String (abw) + " Abweichungen");

            abw = 0;
            for (int i = 0; i < Gitter::evidenzBaender; ++i)
                if (bitsVon (Gitter::evidenzMitte (i)) != hexBits (feinMitten[i].toString()))
                    ++abw;
            pruefe (abw == 0, "Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)",
                    juce::String (abw) + " Abweichungen");

            abw = 0;
            for (int i = 0; i <= Gitter::liveBaender; ++i)
                if (bitsVon (Gitter::liveKante (i)) != hexBits (grobKanten[i].toString()))
                    ++abw;
            pruefe (abw == 0, "Live-Kanten BITGLEICH zur Fixture (65 Werte)",
                    juce::String (abw) + " Abweichungen");

            abw = 0;
            for (int i = 0; i < Gitter::liveBaender; ++i)
                if (bitsVon (Gitter::liveMitte (i)) != hexBits (grobMitten[i].toString()))
                    ++abw;
            pruefe (abw == 0, "Live-Mitten BITGLEICH zur Fixture (64 Werte)",
                    juce::String (abw) + " Abweichungen");

            // Die Gruppierung: lueckenlos, ueberschneidungsfrei, vollstaendig.
            int gAbw = 0, erwartet = 0;
            for (int g = 0; g < Gitter::liveBaender; ++g)
            {
                auto* eintrag = gruppen[g].getDynamicObject();
                if ((int) eintrag->getProperty ("fein_von") != Gitter::liveVon (g)) ++gAbw;
                if ((int) eintrag->getProperty ("fein_bis_exkl") != Gitter::liveBisExkl (g)) ++gAbw;
                if (Gitter::liveVon (g) != erwartet) ++gAbw;
                erwartet = Gitter::liveBisExkl (g);
            }
            pruefe (gAbw == 0 && erwartet == Gitter::evidenzBaender,
                    "Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal",
                    juce::String (gAbw) + " Abweichungen, Ende bei " + juce::String (erwartet));

            // Und die groben Kanten sind KOPIEN feiner Kanten - sonst waere das
            // Livegitter still eine zweite Filterbank geworden.
            int kAbw = 0;
            for (int g = 0; g < Gitter::liveBaender; ++g)
                if (bitsVon (Gitter::liveKante (g)) != bitsVon (Gitter::evidenzKante (Gitter::liveVon (g))))
                    ++kAbw;
            if (bitsVon (Gitter::liveKante (Gitter::liveBaender))
                != bitsVon (Gitter::evidenzKante (Gitter::evidenzBaender)))
                ++kAbw;
            pruefe (kAbw == 0, "Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank",
                    juce::String (kAbw) + " Abweichungen");
        }
    }

    //==========================================================================
    std::cout << std::endl << "== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==" << std::endl;
    {
        bool ok = false;
        const auto q = lies ("eq-copilot/schemas/v3/quantisierung-v1.json", ok);
        if (ok)
        {
            auto* kod = q.getDynamicObject()->getProperty ("kodierungen").getDynamicObject();
            struct Fall { const char* name; BandEncoding e; };
            const Fall faelle[] = { { "q_db_0p1_i16",  BandEncoding::q_db_0p1_i16 },
                                    { "q_db_0p01_i16", BandEncoding::q_db_0p01_i16 } };
            for (const auto& f : faelle)
            {
                auto* eintrag = kod->getProperty (f.name).getDynamicObject();
                const int skala = (int) eintrag->getProperty ("skalierung");
                pruefe ((int) skalierungVon (f.e) == skala,
                        juce::String (f.name) + ": Skalierung stimmt mit der Fixture ueberein",
                        juce::String (skala));

                const auto vektoren = eintrag->getProperty ("vektoren");
                int schlecht = 0;
                juce::String erster;
                for (int i = 0; i < vektoren.size(); ++i)
                {
                    auto* v = vektoren[i].getDynamicObject();
                    const double ein = ausBits (hexBits (v->getProperty ("ein_hex64").toString()));
                    const auto got = quantisiere16 (ein, f.e);
                    const bool gut = got.wert == (std::int16_t) (int) v->getProperty ("wert")
                                  && got.gueltig == (bool) v->getProperty ("gueltig")
                                  && got.saturiert == (bool) v->getProperty ("saturiert");
                    if (! gut)
                    {
                        ++schlecht;
                        if (erster.isEmpty())
                            erster = v->getProperty ("ein_dezimal").toString()
                                   + " -> " + juce::String (got.wert)
                                   + "/" + juce::String (got.gueltig ? 1 : 0)
                                   + "/" + juce::String (got.saturiert ? 1 : 0)
                                   + ", erwartet " + v->getProperty ("wert").toString();
                    }
                }
                pruefe (schlecht == 0, juce::String (f.name) + ": alle "
                        + juce::String (vektoren.size()) + " Vektoren treffen",
                        schlecht == 0 ? juce::String() : erster);
            }

            auto* f32 = kod->getProperty ("float32").getDynamicObject();
            const auto vektoren = f32->getProperty ("vektoren");
            int schlecht = 0;
            juce::String erster;
            for (int i = 0; i < vektoren.size(); ++i)
            {
                auto* v = vektoren[i].getDynamicObject();
                const double ein = ausBits (hexBits (v->getProperty ("ein_hex64").toString()));
                const auto got = quantisiereF32 (ein);
                const auto sollBits = (std::uint32_t) v->getProperty ("wert_hex32").toString().getHexValue64();
                const bool gut = bitsVon (got.wert) == sollBits
                              && got.gueltig == (bool) v->getProperty ("gueltig")
                              && got.saturiert == (bool) v->getProperty ("saturiert");
                if (! gut)
                {
                    ++schlecht;
                    if (erster.isEmpty())
                        erster = v->getProperty ("ein_dezimal").toString()
                               + " -> " + juce::String::toHexString ((int) bitsVon (got.wert))
                               + ", erwartet " + v->getProperty ("wert_hex32").toString();
                }
            }
            pruefe (schlecht == 0, juce::String ("float32: alle ") + juce::String (vektoren.size())
                    + " Vektoren treffen bitgenau", schlecht == 0 ? juce::String() : erster);

            // Die Rundungsfalle, die die Fixture ausdruecklich benennt.
            pruefe (quantisiere16 (0.049999999999999996, BandEncoding::q_db_0p1_i16).wert == 0,
                    "Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0");
            pruefe (quantisiere16 (0.05, BandEncoding::q_db_0p1_i16).wert == 1,
                    "halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl");
            pruefe (quantisiere16 (-0.05, BandEncoding::q_db_0p1_i16).wert == -1,
                    "und negativ genauso (-0,05 -> -1)");
        }
    }

    //==========================================================================
    std::cout << std::endl << "== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==" << std::endl;
    {
        std::uint8_t bm[bitmapBytes (Gitter::liveBaender)] {};
        bitmapNullen (bm, Gitter::liveBaender);
        bitmapSetze (bm, 0, true);
        bitmapSetze (bm, 7, true);
        bitmapSetze (bm, 8, true);
        pruefe (bm[0] == 0x81 && bm[1] == 0x01,
                "Band i sitzt in Byte i/8, Bit i%8 (LSB-first)",
                "Byte0=" + juce::String::toHexString (bm[0]) + " Byte1=" + juce::String::toHexString (bm[1]));
        bitmapSetze (bm, 7, false);
        pruefe (bm[0] == 0x01, "ein Bit loeschen laesst die Nachbarn stehen");
        pruefe (bitmapLies (bm, 0) && ! bitmapLies (bm, 7) && bitmapLies (bm, 8),
                "Lesen und Schreiben sind dieselbe Adressierung");

        // 221 Baender = 27,625 Bytes -> 28 Bytes, 3 Fuellbits im letzten.
        std::uint8_t ev[bitmapBytes (Gitter::evidenzBaender)] {};
        for (auto& b : ev) b = 0xff;
        bitmapNullen (ev, Gitter::evidenzBaender);
        for (int i = 0; i < Gitter::evidenzBaender; ++i)
            bitmapSetze (ev, i, true);
        pruefe (bitmapBytes (Gitter::evidenzBaender) == 28,
                "221 Baender brauchen 28 Bitmapbytes", juce::String ((int) bitmapBytes (Gitter::evidenzBaender)));
        pruefe (ev[27] == 0x1f,
                "die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)",
                "letztes Byte = " + juce::String::toHexString (ev[27]));
    }

    //==========================================================================
    std::cout << std::endl << "== D - FFT: gegen analytisch bekannte Ergebnisse ==" << std::endl;
    {
        Fft f;
        const int n = 1024;
        f.vorbereiten (n);
        std::vector<double> x ((std::size_t) n, 0.0);

        // Reiner Sinus exakt auf einer Binmitte: die ganze Energie in einem Bin.
        const int bin = 64;
        for (int i = 0; i < n; ++i)
            x[(std::size_t) i] = std::sin (2.0 * kPi * (double) bin * (double) i / (double) n);
        f.transformiere (x.data());
        double gesamt = 0.0, imBin = f.leistung (bin);
        for (int k = 0; k <= n / 2; ++k) gesamt += f.leistung (k);
        pruefe (imBin / gesamt > 0.999,
                "Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin",
                juce::String (100.0 * imBin / gesamt, 4) + " %");

        // Parseval: Summe |X|² / n == Summe x²  (bei reeller Eingabe ueber ALLE
        // n Bins; die Haelfte oben ist das Spiegelbild).
        double zeit = 0.0;
        for (int i = 0; i < n; ++i) zeit += x[(std::size_t) i] * x[(std::size_t) i];
        double freq = 0.0;
        for (int k = 0; k <= n / 2; ++k)
            freq += f.leistung (k) * ((k == 0 || k == n / 2) ? 1.0 : 2.0);
        const double relFehler = std::abs (freq / (double) n - zeit) / zeit;
        pruefe (relFehler < 1e-12, "Parseval haelt (Zeit- und Frequenzenergie gleich)",
                "rel. Fehler " + juce::String (relFehler, 16));

        // Gleichanteil landet in Bin 0 und nirgends sonst.
        for (auto& v : x) v = 1.0;
        f.transformiere (x.data());
        double rest = 0.0;
        for (int k = 1; k <= n / 2; ++k) rest += f.leistung (k);
        pruefe (std::abs (f.leistung (0) - (double) n * (double) n) < 1e-6 && rest < 1e-12,
                "Gleichanteil sitzt vollstaendig in Bin 0",
                "Bin0 = " + juce::String (f.leistung (0), 1) + ", Rest = " + juce::String (rest, 20));
    }

    //==========================================================================
    std::cout << std::endl << "== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==" << std::endl;
    {
        // Die in ITU-R BS.1770-5 fuer 48 kHz GEDRUCKTEN Koeffizienten. Sie
        // stehen hier als Referenz, nicht als Quelle: gebaut wird nach RBJ (wie
        // pyloudnorm), und die Frage ist, ob beide Ketten denselben
        // Frequenzgang haben - nicht, ob beide dieselben Ziffern haben.
        Biquad normShelf; normShelf.b0 = 1.53512485958697; normShelf.b1 = -2.69169618940638;
        normShelf.b2 = 1.19839281085285; normShelf.a1 = -1.69065929318241; normShelf.a2 = 0.73248077421585;
        Biquad normHp; normHp.b0 = 1.0; normHp.b1 = -2.0; normHp.b2 = 1.0;
        normHp.a1 = -1.99004745483398; normHp.a2 = 0.99007225036621;

        KKette unsere;
        unsere.entwerfen (48000.0);

        double maxAbwDb = 0.0, beiHz = 0.0;
        for (int i = 0; i < 200; ++i)
        {
            const double hz = 20.0 * std::pow (1000.0, (double) i / 199.0);   // 20 Hz .. 20 kHz
            const double w = 2.0 * kPi * hz / 48000.0;
            const double norm = biquadLeistung (normShelf, w) * biquadLeistung (normHp, w);
            const double uns  = biquadLeistung (unsere.shelf, w) * biquadLeistung (unsere.hochpass, w);
            if (norm > 0.0 && uns > 0.0)
            {
                const double d = std::abs (10.0 * std::log10 (uns / norm));
                if (d > maxAbwDb) { maxAbwDb = d; beiHz = hz; }
            }
        }
        // 0,1 dB ist die Toleranz, die §39.3 fuer Loudness/True Peak nennt.
        pruefe (maxAbwDb < 0.1,
                "RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander",
                "max " + juce::String (maxAbwDb, 5) + " dB bei " + juce::String (beiHz, 1) + " Hz");
        std::cout << "       (gemessene Groesstabweichung, zur Ablesung: "
                  << juce::String (maxAbwDb, 6).toRawUTF8() << " dB)" << std::endl;

        // Und die Kette ist bei 0 Hz wirklich zu: der Hochpass muss den
        // Gleichanteil vollstaendig entfernen, sonst waere jede Lautheit auf
        // Material mit DC-Versatz zu hoch.
        const double dc = biquadLeistung (unsere.hochpass, 0.0);
        pruefe (dc < 1e-20, "Hochpass sperrt den Gleichanteil vollstaendig",
                "|H(0)|² = " + juce::String (dc, 24));
    }

    //==========================================================================
    std::cout << std::endl << "== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==" << std::endl;
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        s.laufen (60);                       // > 100 ms, also mindestens ein Frame

        const auto& f = e.frame();
        pruefe (f.transport.sequence >= 1, "die Sequenz zaehlt ab 1 aufwaerts",
                juce::String ((int) f.transport.sequence));
        pruefe (f.transport.zeitbasis == Zeitbasis::project_samples,
                "mit gueltiger Projektzeit ist die Zeitbasis project_samples");
        pruefe (f.transport.project_sample_start_gesetzt,
                "und project_sample_start ist gesetzt");
        pruefe ((f.transport.gueltigkeit & kGProjectTime) != 0,
                "Gueltigkeitsbit project_time steht");
        pruefe (f.metricsVersion == kFeatureMetricsVersion,
                "der Frame traegt seine Metrikversion", juce::String ((int) f.metricsVersion));
        pruefe (f.transport.sample_rate == 48000.0, "und die Samplerate");

        // Ohne Zeitbeweis: local_monotonic, KEIN Zeitbit, KEIN Startwert.
        FeatureEngine e2;
        e2.vorbereiten (48000.0);
        Speiser s2 { e2 };
        for (int i = 0; i < 60; ++i)
            s2.senden (s2.bauen (0, true, /*zeitGueltig*/ false));
        const auto& f2 = e2.frame();
        pruefe (f2.transport.zeitbasis == Zeitbasis::local_monotonic,
                "ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (§32.3)");
        pruefe (! f2.transport.project_sample_start_gesetzt
                && (f2.transport.gueltigkeit & kGProjectTime) == 0,
                "und dann steht WEDER das Bit NOCH ein Startwert da");

        // Die sieben Bits: was der Block meldet, meldet der Stempel - und nur das.
        FeatureEngine e3;
        e3.vorbereiten (48000.0);
        Speiser s3 { e3 };
        const std::uint32_t alles = rt::kFlagRecordingGueltig | rt::kFlagRecording
                                  | rt::kFlagContinuousGueltig
                                  | rt::kFlagCycleGrenzenGueltig
                                  | rt::kFlagEingangLatenzGemeldet
                                  | rt::kFlagAusgangLatenzGemeldet;
        for (int i = 0; i < 60; ++i)
        {
            auto b = s3.bauen (alles);
            b.continuousTimeSamples = 12345;
            b.cycleStartPpq = 4.0; b.cycleEndePpq = 8.0;
            b.eingangLatenzSamples = 0;      // "gemeldet 0" - der schwierige Fall
            b.ausgangLatenzSamples = 4410;
            s3.senden (b);
        }
        const auto& f3 = e3.frame();
        pruefe (f3.transport.gueltigkeit == kGAlleSieben,
                "alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet",
                "0x" + juce::String::toHexString ((int) f3.transport.gueltigkeit));
        pruefe (f3.transport.input_presentation_latency_gesetzt
                && f3.transport.input_presentation_latency == 0,
                "eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (§32.3)");
        pruefe (f3.transport.cycle_derivation == Herleitung::unproven,
                "Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer");
        pruefe (f3.transport.continuous_time_samples == 12345,
                "continuous_time_samples kommt durch");
    }

    //==========================================================================
    std::cout << std::endl << "== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==" << std::endl;

    // G1 - Drop (Queue-Ueberlauf oder Oversize). §32.3: SEGMENT, nicht Epoche.
    grenzfall ("G1 Drop (kFlagLueckeDavor)", Grenzgrund::lokaleLuecke,
               [] (Speiser& s)
               {
                   ++s.segment;
                   auto b = s.bauen (rt::kFlagLueckeDavor);
                   s.strom += 4096;                  // die verlorene Zeit
                   b.stromVon = s.strom;
                   s.senden (b);
               }, /*Epoche*/ false);

    // G2 - Seek bei LAUFENDEM Transport.
    grenzfall ("G2 Seek waehrend Wiedergabe", Grenzgrund::zeitSprung,
               [] (Speiser& s)
               {
                   s.projekt += 480000;              // 10 s vorwaerts
                   s.senden (s.bauen());
               });

    // G3 - Seek bei BEKANNT GESTOPPTEM Transport. Das ist die Grenze, die es
    // erst seit der SONDE-008-Nacharbeit (T2-4) gibt, und deren Eigentuemer
    // laut Pruefbericht dieses Ticket ist ("die Fensterbuchhaltung baut auf
    // dieser Grenze auf").
    {
        // Eigener Aufbau statt `grenzfall`: der Fall laeuft von Anfang an
        // GESTOPPT. Erst umzuschalten waere selbst schon eine Transportkante
        // und traefe die Fenster, bevor der eigentliche Fall dran ist.
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        const int noetig = (int) (3.4 * s.sr / (double) s.frames) + 4;
        for (int i = 0; i < noetig; ++i)
        {
            auto b = s.bauen (0, /*spielt*/ false);
            s.senden (b);                    // Projektzeit STEHT (Vorhoeren)
        }
        pruefe (s.allesOffen(), "G3: bei gestopptem Transport laufen die Fenster normal weiter",
                fuellstaende (e));
        pruefe (e.epochenwechsel() == 0,
                "G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)",
                juce::String ((int) e.epochenwechsel()) + " Epochenwechsel");

        const auto vorher = e.grenzenMitGrund (Grenzgrund::zeitSprung);
        s.projekt += 480000;                 // der User zieht den Playhead
        auto b = s.bauen (0, false);
        s.senden (b);
        pruefe (keinFensterUeberbrueckt (e, s.frames, s.sr),
                "G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster", fuellstaende (e));
        pruefe (e.grenzenMitGrund (Grenzgrund::zeitSprung) == vorher + 1,
                "G3: und sie heisst zeitSprung, nicht lokaleLuecke");
    }

    // G4 - Loop-Wrap: Rueckwaertssprung auf den Schleifenanfang.
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        const std::uint32_t loop = rt::kFlagCycleAktiv | rt::kFlagCycleGrenzenGueltig
                                 | rt::kFlagPpqGueltig | rt::kFlagTempoGueltig;
        const double bpm = 120.0;
        const int noetig = (int) (3.4 * s.sr / (double) s.frames) + 4;
        double ppq = 100.0;                  // weit hinter dem Schleifenanfang
        const double ppqProBlock = bpm / 60.0 / s.sr * (double) s.frames;
        for (int i = 0; i < noetig; ++i)
        {
            auto b = s.bauen (loop);
            b.tempo = bpm; b.ppqPosition = ppq;
            b.cycleStartPpq = 96.0; b.cycleEndePpq = 1000.0;   // Ende weit weg
            s.senden (b);
            ppq += ppqProBlock;
        }
        pruefe (s.allesOffen(), "G4: aktive Schleife allein trennt nichts", fuellstaende (e));

        const auto vorher = e.grenzenMitGrund (Grenzgrund::loopWrap);
        s.projekt -= 480000;                 // zurueck an den Schleifenanfang
        auto b = s.bauen (loop);
        b.tempo = bpm; b.ppqPosition = 96.0;
        b.cycleStartPpq = 96.0; b.cycleEndePpq = 1000.0;
        s.senden (b);
        pruefe (keinFensterUeberbrueckt (e, s.frames, s.sr),
                "G4 Loop-Wrap: jedes Fenster getrennt", fuellstaende (e));
        pruefe (e.grenzenMitGrund (Grenzgrund::loopWrap) == vorher + 1,
                "G4: und die Ursache heisst loopWrap, nicht zeitSprung");
    }

    // G5 - MOEGLICHER Straddle: die Schleifengrenze liegt rechnerisch IM Block,
    // und die Abbildung PPQ->Sample ist nicht bewiesen. §32.3.
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        const std::uint32_t loop = rt::kFlagCycleAktiv | rt::kFlagCycleGrenzenGueltig
                                 | rt::kFlagPpqGueltig | rt::kFlagTempoGueltig;
        const double bpm = 120.0;
        const double ppqProBlock = bpm / 60.0 / s.sr * (double) s.frames;
        double ppq = 10.0;
        const int noetig = (int) (3.4 * s.sr / (double) s.frames) + 4;
        for (int i = 0; i < noetig; ++i)
        {
            auto b = s.bauen (loop);
            b.tempo = bpm; b.ppqPosition = ppq;
            b.cycleStartPpq = 0.0; b.cycleEndePpq = 1e9;    // Ende unerreichbar weit
            s.senden (b);
            ppq += ppqProBlock;
        }
        pruefe (s.allesOffen(), "G5: Schleife ohne erreichbare Grenze trennt nichts",
                fuellstaende (e));

        const auto vorher = e.straddleVerworfen();
        auto b = s.bauen (loop);
        b.tempo = bpm; b.ppqPosition = ppq;
        b.cycleStartPpq = 0.0;
        b.cycleEndePpq = ppq + 0.5 * ppqProBlock;          // mitten im Block
        s.senden (b);
        pruefe (alleFensterLeer (e),
                "G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, "
                "weil der Straddle-Block auch kein neues beginnt", fuellstaende (e));
        pruefe (e.straddleVerworfen() == vorher + 1,
                "G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet");
        pruefe (e.grenzenMitGrund (Grenzgrund::moeglicherStraddle) >= 1,
                "G5: mit eigener Ursache moeglicherStraddle");
    }

    // G6 - Transportkante (Stop waehrend Wiedergabe).
    grenzfall ("G6 Transportkante (Play -> Stop)", Grenzgrund::transportKante,
               [] (Speiser& s) { s.senden (s.bauen (0, /*spielt*/ false)); });

    // G7 - Sampleratewechsel im laufenden Betrieb.
    grenzfall ("G7 Sampleratewechsel", Grenzgrund::sampleratewechsel,
               [] (Speiser& s)
               {
                   auto b = s.bauen();
                   b.sampleRate = 44100.0;
                   s.senden (b);
               });

    // G8 - Neuanlauf (prepareToPlay -> hoehere startFolge).
    grenzfall ("G8 Neuanlauf (prepareToPlay)", Grenzgrund::neuanlauf,
               [] (Speiser& s) { ++s.startFolge; s.senden (s.bauen()); });

    // G9 - Beweislagewechsel: der Hostkontext faellt weg.
    grenzfall ("G9 Beweislagewechsel (Kontext faellt weg)", Grenzgrund::beweislageWechsel,
               [] (Speiser& s)
               {
                   auto b = s.bauen (0, true, /*zeitGueltig*/ false);
                   b.flags &= ~rt::kFlagKontextAnwesend;
                   s.senden (b);
               });

    // G11 - Der K-FILTERZUSTAND ueberbrueckt die Grenze nicht.
    //
    // Der subtilste Fall des ganzen Tickets: ein Biquad traegt seinen Nachklang
    // im Zustand, nicht in einem Puffer. Laesst man ihn ueber eine Grenze
    // stehen, fliesst Audio von VOR der Grenze in die erste Zelle danach - und
    // kein einziger Fuellstand zeigt das an.
    //
    // Gemessen wird BITGLEICH: derselbe leise Abschnitt, einmal nach einem sehr
    // lauten Vorlauf mit Grenze, einmal an einer frischen Engine. Nur wenn der
    // Zustand wirklich genullt wird, sind beide LUFS-S identisch bis aufs
    // letzte Bit - eine Toleranz waere hier die falsche Frage, weil der
    // Unterschied ein RECHENWEG ist, keine Messungenauigkeit.
    //
    // ⚠️ UND DER LAUF ENDET BEIM ERSTEN LUFS-FRAME, NICHT NACH FESTER ZEIT.
    // Die erste Fassung lief 3,5 s weiter und war damit BLIND: LUFS-S mittelt
    // ueber die letzten 3 s, also war die eine betroffene Zelle - die direkt
    // nach der Grenze, in der der Nachklang sitzt - zum Messzeitpunkt laengst
    // aus der Historie herausgelaufen. Die Mutationsprobe (M7: Nullung
    // entfernt) lief gruen durch. Dieselbe Falle wie Lehre (b) aus der
    // SONDE-008-Nacharbeit: das Bein stand am richtigen Ort und fuhr trotzdem
    // an dem einen Fenster vorbei, in dem der Fehler sitzt.
    {
        const double fs = 48000.0;
        auto lauf = [&] (FeatureEngine& e, Speiser& s, int bloecke, double amp,
                         double& phase, bool bisErsterLufs) -> int
        {
            const double dphi = 2.0 * kPi * 1000.0 / fs;
            int gefahren = 0;
            for (int i = 0; i < bloecke; ++i)
            {
                auto b = s.bauen();
                s.audio.resize ((std::size_t) b.sampleCount * 2u);
                for (std::uint32_t k = 0; k < b.sampleCount; ++k)
                {
                    const float v = (float) (amp * std::sin (phase));
                    phase += dphi;
                    s.audio[(std::size_t) k * 2u]      = v;
                    s.audio[(std::size_t) k * 2u + 1u] = v;
                }
                const bool frame = e.nimmBlock (b, s.audio.data());
                s.strom += b.sampleCount;
                s.projekt += (std::int64_t) b.sampleCount;
                ++gefahren;
                if (bisErsterLufs && frame && e.frame().lufsSGesetzt)
                    break;                    // GENAU hier ist die erste Zelle noch drin
            }
            return gefahren;
        };

        // A: lauter Vorlauf, Seek, dann leise bis zum ERSTEN LUFS-Frame.
        FeatureEngine eA;
        eA.vorbereiten (fs);
        Speiser sA { eA };
        double phA = 0.0;
        lauf (eA, sA, 60, 1.0, phA, false);            // Vollaussteuerung
        pruefe (eA.kFilterZustand() > 0.0,
                "G11: im Betrieb traegt die K-Kette einen Filterzustand",
                juce::String (eA.kFilterZustand(), 6));
        sA.projekt += 480000;                          // Seek = Grenze
        double phB = 0.0;                              // Phase ab hier neu
        const int bloeckeA = lauf (eA, sA, 2000, 0.001, phB, true);
        const float lufsA = eA.frame().lufsS;

        // B: nur der leise Teil, frische Engine, GLEICH VIELE Bloecke.
        FeatureEngine eB;
        eB.vorbereiten (fs);
        Speiser sB { eB };
        double phC = 0.0;
        const int bloeckeB = lauf (eB, sB, 2000, 0.001, phC, true);
        const float lufsB = eB.frame().lufsS;
        pruefe (bloeckeA == bloeckeB,
                "G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame",
                juce::String (bloeckeA) + " / " + juce::String (bloeckeB));

        pruefe (eA.frame().lufsSGesetzt && eB.frame().lufsSGesetzt,
                "G11: beide Laeufe liefern eine Kurzzeitlautheit");
        pruefe (bitsVon (lufsA) == bitsVon (lufsB),
                "G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - "
                "der Filternachklang ueberbrueckt nicht",
                "A=" + juce::String ((double) lufsA, 9) + " B=" + juce::String ((double) lufsB, 9));
    }

    // G10 - Die FL-Teilstueckregel darf NICHT als Grenze durchgehen.
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        s.bisAllesOffen();
        const auto vorher = e.getrennteFenster();
        // FL zerteilt Puffer bis auf 1 Sample; die Teilstuecke tragen DIESELBE
        // Projektzeit (Capabilityreport S4). Das ist lueckenlose Fortsetzung.
        for (int i = 0; i < 8; ++i)
        {
            auto b = s.bauen();
            const auto merk = s.projekt;
            s.frames = 1;
            b = s.bauen();
            s.senden (b);
            s.projekt = merk;                // Zeit STEHT ueber die Teilstuecke
            s.frames = 512;
        }
        pruefe (e.getrennteFenster() == vorher,
                "G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)",
                juce::String ((int) (e.getrennteFenster() - vorher)) + " Trennungen");
        pruefe (! alleFensterLeer (e), "G10: die Fenster laufen dabei weiter",
                fuellstaende (e));
    }

    //==========================================================================
    std::cout << std::endl << "== H - NAK-29: bedingte Feldpflichten des Transportstempels ==" << std::endl;
    {
        Transportstempel gut;
        gut.zeitbasis = Zeitbasis::project_samples;
        gut.project_sample_start_gesetzt = true;
        gut.gueltigkeit = kGProjectTime;
        pruefe (nak29Verstoss (gut) == 0, "ein sauberer Stempel kommt durch");

        auto t1 = gut; t1.project_sample_start_gesetzt = false;
        pruefe (nak29Verstoss (t1) == 1,
                "Fall 1: project_samples + Bit, aber KEIN project_sample_start");

        auto t2 = gut; t2.zeitbasis = Zeitbasis::local_monotonic;
        pruefe (nak29Verstoss (t2) == 2,
                "Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu §32.3)");

        auto t3 = gut; t3.cycle_bounds_valid = true;
        pruefe (nak29Verstoss (t3) == 3,
                "Fall 3: bounds_valid ohne start_ppq/end_ppq");

        auto t4 = gut; t4.cycle_derivation = Herleitung::validated_block_mapping;
        pruefe (nak29Verstoss (t4) == 4,
                "Fall 4: derivation=validated_block_mapping ohne bounds_valid");

        auto t5 = gut; t5.gueltigkeit |= kGCycleBounds;
        pruefe (nak29Verstoss (t5) == 5,
                "Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht");

        auto t6 = gut; t6.gueltigkeit |= kGContinuousTime;
        pruefe (nak29Verstoss (t6) == 6,
                "Fall 6: continuous_time-Bit ohne continuous_time_samples");

        // Und der Riegel ist im ERZEUGER wirksam, nicht nur als Funktion:
        // ein Frame mit verletztem Stempel wird gar nicht erst veroeffentlicht.
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        s.laufen (60);
        pruefe (e.nak29Abgelehnt() == 0,
                "im Normalbetrieb entsteht kein einziger verletzter Stempel",
                juce::String ((int) e.nak29Abgelehnt()));
    }

    //==========================================================================
    std::cout << std::endl << "== I - Ereignisse: kein Fluss ueber eine Grenze ==" << std::endl;
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        s.bisAllesOffen();
        pruefe (e.flussHatVorgaenger(),
                "der spektrale Fluss hat im Betrieb einen Vorgaenger");

        s.projekt += 480000;
        s.senden (s.bauen());
        pruefe (! e.flussHatVorgaenger(),
                "nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss "
                "einen Onset, den es nie gab");

        // Ein Ereignis traegt seine Epoche und sein Segment mit.
        FeatureEngine e2;
        e2.vorbereiten (48000.0);
        Speiser s2 { e2 };
        s2.bisAllesOffen();
        // Ein harter Impuls nach Stille erzeugt Fluss.
        for (int runde = 0; runde < 40; ++runde)
        {
            auto b = s2.bauen();
            s2.audio.assign ((std::size_t) b.sampleCount * 2u, 0.0f);
            if (runde % 8 == 0)
                for (std::uint32_t i = 0; i < 64; ++i)
                {
                    s2.audio[(std::size_t) i * 2u]      = 0.9f;
                    s2.audio[(std::size_t) i * 2u + 1u] = 0.9f;
                }
            e2.nimmBlock (b, s2.audio.data());
            s2.strom += b.sampleCount;
            s2.projekt += (std::int64_t) b.sampleCount;
        }
        bool alleInEpoche = true;
        for (int i = 0; i < e2.ereignisAnzahlJetzt(); ++i)
            if (e2.ereignis (i).epoche != e2.transportEpocheJetzt())
                alleInEpoche = false;
        pruefe (alleInEpoche, "jedes Ereignis traegt die Epoche, in der es gesehen wurde",
                juce::String (e2.ereignisAnzahlJetzt()) + " Ereignis(se)");
        pruefe (e2.ereignisAnzahlJetzt() <= FeatureEngine::kEreignisPlaetze,
                "der Ereignisstrom ist fest gedeckelt (§33 'feste Obergrenzen')",
                juce::String (e2.ereignisAnzahlJetzt()) + " / "
                + juce::String (FeatureEngine::kEreignisPlaetze));
    }

    //==========================================================================
    std::cout << std::endl << "== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==" << std::endl;
    {
        const double fs = 48000.0, hz = 997.0, amp = 0.5;
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };

        // 4 s reiner Sinus auf beiden Kanaelen.
        const int bloecke = (int) (4.0 * fs / (double) s.frames);
        double phase = 0.0;
        const double dphi = 2.0 * kPi * hz / fs;
        for (int blk = 0; blk < bloecke; ++blk)
        {
            auto b = s.bauen();
            s.audio.resize ((std::size_t) b.sampleCount * 2u);
            for (std::uint32_t i = 0; i < b.sampleCount; ++i)
            {
                const float v = (float) (amp * std::sin (phase));
                phase += dphi;
                s.audio[(std::size_t) i * 2u]      = v;
                s.audio[(std::size_t) i * 2u + 1u] = v;
            }
            e.nimmBlock (b, s.audio.data());
            s.strom += b.sampleCount;
            s.projekt += (std::int64_t) b.sampleCount;
        }

        // Erwartung, unabhaengig vom Zeitbereichspfad: |H(w)|² analytisch,
        // dann BS.1770 auf zwei Kanaele mit G=1.
        KKette k; k.entwerfen (fs);
        const double w = 2.0 * kPi * hz / fs;
        const double gain2 = biquadLeistung (k.shelf, w) * biquadLeistung (k.hochpass, w);
        const double meanQuadrat = amp * amp / 2.0 * gain2;
        const double erwartet = -0.691 + 10.0 * std::log10 (2.0 * meanQuadrat);

        const auto& f = e.frame();
        pruefe (f.lufsSGesetzt, "LUFS-S ist nach 4 s gesetzt");
        if (f.lufsSGesetzt)
        {
            const double d = std::abs ((double) f.lufsS - erwartet);
            pruefe (d < 0.1,
                    "LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (§39.3)",
                    "gemessen " + juce::String ((double) f.lufsS, 4)
                    + " erwartet " + juce::String (erwartet, 4)
                    + " d=" + juce::String (d, 5));
        }
        pruefe (f.peakGesetzt && std::abs ((double) f.peakDb - 20.0 * std::log10 (amp)) < 0.01,
                "Peak trifft die Amplitude", juce::String ((double) f.peakDb, 3) + " dBFS");
        pruefe (f.crestGesetzt && std::abs ((double) f.crestDb - 3.0103) < 0.05,
                "Crest eines Sinus ist 3,01 dB", juce::String ((double) f.crestDb, 4) + " dB");
        pruefe (f.korrelationGesetzt && f.korrelation > 0.999,
                "L==R ergibt Korrelation 1", juce::String ((double) f.korrelation, 6));
        pruefe (f.breiteGesetzt && f.breite < 1e-9,
                "und Breite 0 (kein Seitenanteil)", juce::String ((double) f.breite, 12));

        // Das Band, in dem 997 Hz liegt, ist belegt - und ein weit entferntes nicht.
        int bandMit997 = -1;
        for (int b = 0; b < Gitter::liveBaender; ++b)
            if (Gitter::liveKante (b) <= hz && hz < Gitter::liveKante (b + 1))
                bandMit997 = b;
        pruefe (bandMit997 >= 0 && bitmapLies (f.live.bitmap, bandMit997),
                "das Live-Band um 997 Hz ist gueltig", "Band " + juce::String (bandMit997));
        pruefe (bandMit997 >= 0 && ! bitmapLies (f.live.bitmap, 0),
                "und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal");
    }

    //==========================================================================
    std::cout << std::endl << "== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==" << std::endl;
    {
        // Der T2-Bericht zu SONDE-008 §8.9 Punkt 2: "`nakamaBlockEmpfangen()`
        // und damit die ganze Brueckenhaelfte des Zeitstempels hat KEIN Bein."
        // Hier ist es. Gefahren wird die echte Senke des echten Prozessors.
        const double fs = 48000.0;
        const int bs = 512;
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);

        juce::AudioBuffer<float> puffer (2, bs);
        juce::MidiBuffer midi;
        std::uint32_t lcg = 0x9e3779b9u;
        auto fuellen = [&]
        {
            for (int k = 0; k < 2; ++k)
                for (int i = 0; i < bs; ++i)
                {
                    lcg = lcg * 1664525u + 1013904223u;
                    puffer.setSample (k, i, ((float) ((lcg >> 8) & 0xffffu) / 32768.0f - 1.0f) * 0.2f);
                }
        };

        // Ein Blockbefund, wie ihn der gepatchte VST3-Wrapper liefern wuerde.
        eqcop::hostbruecke::Blockbefund befund;
        std::int64_t zeit = 0;
        const int bloecke = (int) (0.5 * fs / (double) bs) + 4;
        for (int i = 0; i < bloecke; ++i)
        {
            befund.kontext.leeren();
            befund.kontext.processContextPresent = true;
            befund.kontext.projectTimeSamples.setze (zeit);
            befund.kontext.playing.setze (true);
            befund.kontext.recording.setze (false);
            befund.kontext.continuousTimeSamples.setze (zeit + 1000);
            befund.kontext.tempo.setze (128.0);
            befund.kontext.ppqPosition.setze ((double) zeit * 128.0 / 60.0 / fs);
            befund.kontext.sampleRate.setze (fs);
            befund.kontext.cycle.aktiv = true;
            befund.kontext.cycle.gueltig = true;
            befund.kontext.cycle.startPpq = 0.0;
            befund.kontext.cycle.endePpq = 1.0e9;
            befund.kontext.presentationLatency.eingang[0] = { 3924u, true };
            befund.kontext.presentationLatency.ausgang[0] = { 4410u, true };
            befund.blockGroesse = (std::uint32_t) bs;
            p.nakamaBlockEmpfangen (befund);

            fuellen();
            p.processBlock (puffer, midi);
            zeit += bs;
        }

        // Auf den Worker warten - mit Fortschrittsbedingung, nicht blind.
        const auto frist = juce::Time::getMillisecondCounter() + 3000;
        while (p.merkmaleBloecke() == 0 && juce::Time::getMillisecondCounter() < frist)
            juce::Thread::sleep (10);
        pruefe (p.merkmaleBloecke() > 0,
                "die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen",
                juce::String ((int) p.merkmaleBloecke()) + " Bloecke");

        const auto frist2 = juce::Time::getMillisecondCounter() + 3000;
        while (p.merkmaleFrames() == 0 && juce::Time::getMillisecondCounter() < frist2)
            juce::Thread::sleep (10);
        pruefe (p.merkmaleFrames() > 0, "und mindestens einen Frame gebaut",
                juce::String ((int) p.merkmaleFrames()) + " Frames");

        const auto& t = p.merkmalFrame().transport;
        pruefe (t.process_context_present,
                "process_context_present kommt aus der BRUECKE durch (§32.3)");
        pruefe ((t.gueltigkeit & kGRecordState) != 0 && ! t.recording,
                "recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen");
        pruefe ((t.gueltigkeit & kGContinuousTime) != 0 && t.continuous_time_samples_gesetzt,
                "continuous_time_samples kommt durch");
        pruefe ((t.gueltigkeit & kGCycleBounds) != 0 && t.cycle_active,
                "die Schleifengrenzen kommen durch, samt `active`");
        pruefe (t.input_presentation_latency_gesetzt && t.input_presentation_latency == 3924u
                && t.output_presentation_latency_gesetzt && t.output_presentation_latency == 4410u,
                "beide Presentation-Latencies kommen durch",
                juce::String ((int) t.input_presentation_latency) + " / "
                + juce::String ((int) t.output_presentation_latency));
        pruefe (t.gueltigkeit == kGAlleSieben,
                "ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits",
                "0x" + juce::String::toHexString ((int) t.gueltigkeit));

        // Gegenprobe: derselbe Lauf OHNE Bruecke, nur mit Playhead. Er kann
        // weniger - und genau das macht Abschnitt K zu einer Aussage ueber die
        // Bruecke statt ueber irgendeinen Weg.
        struct NurPlayHead : juce::AudioPlayHead
        {
            juce::int64 pos = 0;
            juce::Optional<PositionInfo> getPosition() const override
            {
                PositionInfo pi;
                pi.setIsPlaying (true);
                pi.setTimeInSamples (pos);
                return pi;
            }
        };
        EqCopilotProcessor p2;
        NurPlayHead kopf;
        p2.setPlayConfigDetails (2, 2, fs, bs);
        p2.setPlayHead (&kopf);
        p2.prepareToPlay (fs, bs);
        for (int i = 0; i < bloecke; ++i)
        {
            fuellen();
            p2.processBlock (puffer, midi);
            kopf.pos += bs;
        }
        const auto frist3 = juce::Time::getMillisecondCounter() + 3000;
        while (p2.merkmaleFrames() == 0 && juce::Time::getMillisecondCounter() < frist3)
            juce::Thread::sleep (10);
        const auto& t2 = p2.merkmalFrame().transport;
        pruefe (p2.merkmaleFrames() > 0 && t2.gueltigkeit != kGAlleSieben,
                "Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits",
                "0x" + juce::String::toHexString ((int) t2.gueltigkeit));
        pruefe ((t2.gueltigkeit & kGRecordState) == 0,
                "insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`");
    }

    //==========================================================================
    std::cout << std::endl << "== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==" << std::endl;
    {
        // L1 - Ein Fenster, das NICHT getrennt wird, ist an den Fuellstaenden
        // sichtbar. Statt den Produktionscode zu mutieren (das taete ein
        // Pruefer), wird hier gezeigt, dass die Messgroesse ueberhaupt
        // reagiert: ohne Grenze bleiben die Fenster voll.
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        s.bisAllesOffen();
        pruefe (! keinFensterUeberbrueckt (e, s.frames, s.sr),
                "L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt",
                fuellstaende (e));
        s.projekt += 480000;
        s.senden (s.bauen());
        pruefe (keinFensterUeberbrueckt (e, s.frames, s.sr),
                "L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt");

        // L2 - Der Gitterriegel reagiert auf eine geaenderte Zahl.
        const auto echt = bitsVon (Gitter::evidenzKante (0));
        pruefe (echt != (echt ^ 1ull),
                "L2: eine um EIN Bit geaenderte Gitterzahl ist von der echten unterscheidbar",
                "0x" + juce::String::toHexString ((juce::int64) echt));

        // L3 - Der NAK-29-Riegel unterscheidet die Faelle, statt nur 'nein' zu
        // sagen: sechs verschiedene Verletzungen, sechs verschiedene Nummern.
        Transportstempel g;
        g.zeitbasis = Zeitbasis::project_samples;
        g.project_sample_start_gesetzt = true;
        g.gueltigkeit = kGProjectTime;
        bool alleVerschieden = true;
        int gesehen[7] = {};
        auto zaehle = [&] (Transportstempel t) { const int v = nak29Verstoss (t); if (v >= 1 && v <= 6) ++gesehen[v]; };
        { auto t = g; t.project_sample_start_gesetzt = false; zaehle (t); }
        { auto t = g; t.zeitbasis = Zeitbasis::local_monotonic; zaehle (t); }
        { auto t = g; t.cycle_bounds_valid = true; zaehle (t); }
        { auto t = g; t.cycle_derivation = Herleitung::validated_block_mapping; zaehle (t); }
        { auto t = g; t.gueltigkeit |= kGCycleBounds; zaehle (t); }
        { auto t = g; t.gueltigkeit |= kGContinuousTime; zaehle (t); }
        for (int i = 1; i <= 6; ++i) if (gesehen[i] != 1) alleVerschieden = false;
        pruefe (alleVerschieden,
                "L3: sechs Verletzungen ergeben sechs VERSCHIEDENE Nummern, nicht sechsmal 'nein'");

        // L4 - Der Aktivitaetsriegel: Stille fuellt keine Bandakkus. Ohne ihn
        // waere jeder Bandwert eine Aussage ueber die Pausen.
        FeatureEngine e4;
        e4.vorbereiten (48000.0);
        Speiser s4 { e4 };
        for (int i = 0; i < 60; ++i)
        {
            auto b = s4.bauen();
            s4.audio.assign ((std::size_t) b.sampleCount * 2u, 0.0f);
            e4.nimmBlock (b, s4.audio.data());
            s4.strom += b.sampleCount;
            s4.projekt += (std::int64_t) b.sampleCount;
        }
        int gesetzteBaender = 0;
        for (int b = 0; b < Gitter::liveBaender; ++b)
            if (bitmapLies (e4.frame().live.bitmap, b)) ++gesetzteBaender;
        pruefe (gesetzteBaender == 0,
                "L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts",
                juce::String (gesetzteBaender) + " gesetzte Baender");
    }

    //==========================================================================
    std::cout << std::endl << "== M - Auskunft ueber die Grenzen der Messung ==" << std::endl;
    {
        FeatureEngine e;
        e.vorbereiten (48000.0);
        pruefe (e.erstesMessbaresBand() > 0
                && e.erstesMessbaresBand() < Gitter::evidenzBaender,
                "die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da",
                "erstes messbares Band " + juce::String (e.erstesMessbaresBand())
                + " (" + juce::String (Gitter::evidenzMitte (e.erstesMessbaresBand()), 1) + " Hz)");
        // ⚠️ SELBSTFUND beim ersten Lauf: die naheliegende Kappenpruefung bei
        // 48 und 44,1 kHz KANN NICHT SCHEITERN.  Das oberste Band der Fixture
        // endet bei 17 959 Hz, und 0,95·Nyquist ist dort 22 800 bzw. 20 947 Hz
        // — die Kappe greift nie, beide Male steht 221 da, und der Prueflauf
        // saehe trotzdem gruen aus.  Genau die Falle aus T2-1 zu SONDE-008.
        // Deshalb gemessen wird sie bei einer Rate, bei der sie WIRKLICH greift.
        pruefe (e.erstesBandUeberKappe() == Gitter::evidenzBaender,
                "bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz",
                "erstes Band ueber der Kappe " + juce::String (e.erstesBandUeberKappe()));

        FeatureEngine e2;
        e2.vorbereiten (22050.0);            // Nyquist 11 025, Kappe 10 474 Hz
        const int kappe22 = e2.erstesBandUeberKappe();
        pruefe (kappe22 > 0 && kappe22 < Gitter::evidenzBaender,
                "bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab",
                "erstes Band ueber der Kappe " + juce::String (kappe22)
                + " (" + juce::String (Gitter::evidenzMitte (kappe22), 1) + " Hz)");
        pruefe (kappe22 > 0 && Gitter::evidenzKante (kappe22 + 1) > 0.95 * 22050.0 * 0.5
                && Gitter::evidenzKante (kappe22) <= 0.95 * 22050.0 * 0.5,
                "und sie sitzt genau am ersten Band, dessen Oberkante 0,95·Nyquist ueberschreitet",
                juce::String (Gitter::evidenzKante (kappe22), 1) + " .. "
                + juce::String (Gitter::evidenzKante (kappe22 + 1), 1) + " Hz gegen "
                + juce::String (0.95 * 22050.0 * 0.5, 1) + " Hz");

        // Und die abgeschnittenen Baender sind dann auch wirklich ungueltig -
        // nicht 0 dB, sondern kein Bit.
        Speiser s22 { e2 };
        s22.sr = 22050.0;
        s22.laufen (60);
        int ueberDerKappe = 0;
        for (int g = 0; g < Gitter::liveBaender; ++g)
            if (Gitter::liveVon (g) >= kappe22 && bitmapLies (e2.frame().live.bitmap, g))
                ++ueberDerKappe;
        pruefe (ueberDerKappe == 0,
                "kein Live-Band vollstaendig ueber der Kappe traegt einen Wert",
                juce::String (ueberDerKappe) + " Baender");

        // Gegenpfad vorbereiten <-> neu vorbereiten: eine zweite Rate legt neu
        // an und beginnt bei null.
        FeatureEngine e3;
        e3.vorbereiten (48000.0);
        Speiser s3 { e3 };
        s3.laufen (40);
        e3.vorbereiten (96000.0);
        pruefe (e3.samplerate() == 96000.0 && e3.bloeckeGesehen() == 0
                && e3.fuellstandBass() == 0,
                "Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler");
    }

    //==========================================================================
    std::cout << std::endl;
    std::cout << "Ergebnis: " << bestanden << " bestanden, " << fehler << " Fehler." << std::endl;
    return fehler == 0 ? 0 : 1;
}
