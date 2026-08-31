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

    /** Wie `senden`, aber mit eigenem Inhalt: `f(i)` liefert das Sample fuer
        beide Kanaele.  Die Buchfuehrung (Strom, Projektzeit) ist dieselbe —
        sonst pruefte der Test gegen einen selbstgebauten Zeitfehler. */
    bool sendenMit (const rt::StampedBlock& b,
                    const std::function<float (std::uint32_t)>& f)
    {
        audio.resize ((std::size_t) b.sampleCount * 2u);
        for (std::uint32_t i = 0; i < b.sampleCount; ++i)
        {
            const float v = f (i);
            audio[(std::size_t) i * 2u]      = v;
            audio[(std::size_t) i * 2u + 1u] = v;
        }
        const bool r = engine.nimmBlock (b, audio.data());
        strom   += b.sampleCount;
        if ((b.flags & rt::kFlagZeitGueltig) != 0 && (b.flags & rt::kFlagSpielt) != 0)
            projekt += (std::int64_t) b.sampleCount;
        return r;
    }

    /** Wie `senden`, aber mit FERTIGEM Audio: der Aufrufer liefert den
        interleavten Puffer selbst.

        🔑 DAS IST DIE VORAUSSETZUNG DER ZWILLINGSPROBE (G13).  `senden` und
        `sendenMit` erzeugen ihr Audio je Speiser — zwei Speiser lieferten damit
        nur dann dieselben Samples, wenn ihre Rauschzustaende Zug um Zug
        gleichlaufen.  Ein Bein, das BITGLEICHHEIT zweier Engines behauptet,
        darf diese Gleichheit nicht selbst voraussetzen, sondern muss sie
        herstellen: derselbe Puffer geht in beide. */
    bool sendenRoh (const rt::StampedBlock& b, const std::vector<float>& stereo)
    {
        const bool r = engine.nimmBlock (b, stereo.data());
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
        bisBandakkuGefuellt();
    }

    /** Noch so lange weiter, bis die BANDAKKUS und die fertigen Rahmenzellen
        Inhalt tragen.

        ⚠️ Ohne das haengt der Grenzfall an einer Phasenrechnung: ein Frame
        leert `liveAkku` und `rahmenZellen` (`rahmenLeeren()`), und je nachdem,
        wie viele Bloecke seither liefen, ist beides beim Grenzblock voll oder
        ohnehin leer.  Ein Bein, das gegen einen ohnehin leeren Akku prueft, ist
        genau die Blindheit, die T2-1 durchgelassen hat — deshalb wird der
        Zustand HERGESTELLT und nicht ausgerechnet.

        Der Blockbauer kommt von aussen, weil nicht jeder Fall dieselbe
        Blockart fahren darf: ein `bauen()`-Block mitten in G3 waere selbst eine
        Transportkante und traefe die Fenster vor dem eigentlichen Fall. */
    void bisBandakkuGefuellt (const std::function<rt::StampedBlock()>& bauer,
                              int hoechstens = 400)
    {
        for (int i = 0; i < hoechstens && ! (akkusTragenInhalt() && ! frameStehtAn()); ++i)
            senden (bauer());
    }
    void bisBandakkuGefuellt (int hoechstens = 400)
    {
        bisBandakkuGefuellt ([this] { return bauen(); }, hoechstens);
    }

    /** Die Traeger aus T2-1 tragen Inhalt — die Vorbedingung, ohne die eine
        Pruefung auf "danach leer" nichts aussagt. */
    bool akkusTragenInhalt() const
    {
        return engine.liveAkkuBelegteBaender() > 0
            && engine.evidenzAkkuBelegteBaender() > 0
            && engine.rahmenZellenJetzt() > 0;
    }

    /** Wuerde der NAECHSTE Block einen Frame faellig machen?

        🔑 DAS IST DER UNTERSCHIED ZWISCHEN EINEM SCHARFEN UND EINEM BLINDEN
        GRENZFALL, und er ist beim Vorfuehren aufgefallen: baut der Grenzblock
        selbst einen Frame, laeuft danach `rahmenLeeren()` — und das raeumt die
        Bandakkus HINTERHER weg, egal ob `grenzeZiehen()` sie geraeumt hat.  Die
        Pruefung "danach ist der Akku leer" waere dann auch mit dem Bruch gruen,
        waehrend der eben gebaute Frame den Ton von davor bereits ausgeliefert
        hat.  Der Grenzfall wartet deshalb auf eine Phase, in der der Grenzblock
        NICHT faellig wird. */
    bool frameStehtAn() const
    {
        return (double) (engine.liveSamplesJetzt() + (std::uint64_t) frames) / sr
                 >= FeatureEngine::kLiveIntervallS;
    }

    bool allesOffen() const
    {
        return engine.fuellstandBass() > 0
            && engine.fuellstandHaupt() > 0
            && engine.fuellstandKurzLoudness() > 0
            && engine.flussHatVorgaenger();
    }
};

/** 1 kHz bei -6 dBFS, phasenrichtig aus der Stromposition gerechnet — der Ton,
    mit dem der T2-Pruefer den Befund T2-1 sichtbar gemacht hat. */
float sinus1k (std::uint64_t stromPos, double sr = 48000.0)
{
    constexpr double kZweiPi = 6.283185307179586476925286766559;
    return (float) (0.5 * std::sin (kZweiPi * 1000.0 * (double) stromPos / sr));
}

juce::String fuellstaende (const FeatureEngine& e)
{
    return "Bass=" + juce::String (e.fuellstandBass())
         + " Haupt=" + juce::String (e.fuellstandHaupt())
         + " Zelle=" + juce::String (e.fuellstandLoudnessZelle())
         + " Kurz=" + juce::String (e.fuellstandKurzLoudness())
         + " Fluss=" + juce::String (e.flussHatVorgaenger() ? 1 : 0)
         // T2-1: die vier Traeger, an denen der Bruch unsichtbar war. Sie
         // stehen hier mit in der Diagnosezeile, damit eine rote Zeile
         // gleich sagt, WELCHER Traeger ueberbrueckt hat.
         //
         // ⚠️ ZWEI BERICHTIGUNGEN AN DIESEM VERSPRECHEN (T2R2-4, 24.08.):
         // (1) `Breite` stand mit drei Nachkommastellen da und druckte den
         //     gemessenen Wert 2,6e-04 als `0.000` — vor UND nach der Grenze.
         //     Mutation P2 machte zehn Zeilen rot, und in JEDER sahen alle vier
         //     Traeger unauffaellig aus.  Jetzt steht die Anzahl belegter
         //     Baender davor (eine Anzahl rundet nicht auf null) und die Summe
         //     in Exponentialschreibweise dahinter.
         // (2) Die Zeile zeigt die Traeger MIT AUSKUNFT.  Die zehn
         //     Rahmen-Skalare haben keine und stehen deshalb NICHT hier — sie
         //     sind von aussen nur ueber den FRAME sichtbar, und genau dort
         //     deckt sie G13 (Zwillingsprobe).  Diese Zeile behauptet also
         //     nicht mehr, jeden Traeger zu nennen.
         + " LiveAkku=" + juce::String (e.liveAkkuBelegteBaender())
         + " EvidAkku=" + juce::String (e.evidenzAkkuBelegteBaender())
         + " Breite=" + juce::String (e.liveBreiteAkkuBelegteBaender()) + "B/"
         + juce::String (e.liveBreiteAkkuZustand(), 3, true)
         + " RZellen=" + juce::String ((int) e.rahmenZellenJetzt());
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
/** Die vier Traeger aus T2-1, die keinen Fuellstand haben.

    🔑 SIE SIND DER GRUND, WARUM B5 GRUEN WAR, WAEHREND DER BRUCH DA WAR.
    `keinFensterUeberbrueckt()` fragte bis zum 24.08. fuenf Fuellstaende ab und
    keinen einzigen Akkumulator; die sieben Mutationen dieses Tickets zielten
    ebenfalls auf keinen.  Ein Akku traegt kein Fuellstandsfeld — er traegt den
    Wert selbst, und genau deshalb muss er einzeln gefragt werden. */
bool keineAkkusUeberleben (const FeatureEngine& e)
{
    return e.liveAkkuBelegteBaender() == 0
        && e.evidenzAkkuBelegteBaender() == 0
        && e.liveBreiteAkkuZustand() == 0.0
        && e.rahmenZellenJetzt() == 0
        && e.rahmenAktivZellenJetzt() == 0;
}

bool alleFensterLeer (const FeatureEngine& e)
{
    return e.fuellstandBass() == 0
        && e.fuellstandHaupt() == 0
        && e.fuellstandLoudnessZelle() == 0
        && e.fuellstandKurzLoudness() == 0
        && ! e.flussHatVorgaenger()
        && keineAkkusUeberleben (e);
}

bool keinFensterUeberbrueckt (const FeatureEngine& e, int blockFrames, double sr)
{
    const int zellenSamples = (int) std::llround (FeatureEngine::kZelleSekunden * sr);
    return e.fuellstandBass()  == blockFrames
        && e.fuellstandHaupt() == blockFrames
        && e.fuellstandLoudnessZelle() == blockFrames % zellenSamples
        && e.fuellstandKurzLoudness() == 0        // die 3-s-Historie ist weg
        && ! e.flussHatVorgaenger()               // und der Fluss hat keinen Vorgaenger
        && keineAkkusUeberleben (e);              // T2-1: und kein Akku ueberlebt
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
    const auto verworfenVorher = e.verworfeneBandfenster();

    pruefe (s.allesOffen(), name + ": vor der Grenze sind alle Fenster offen",
            fuellstaende (e));
    pruefe (s.akkusTragenInhalt(),
            name + ": und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts",
            fuellstaende (e));

    ausloesen (s);

    pruefe (keinFensterUeberbrueckt (e, s.frames, s.sr),
            name + ": NACH der Grenze traegt kein Fenster ein Sample von davor",
            fuellstaende (e) + " (erwartet je " + juce::String (s.frames) + ")");
    // Die positive Haelfte: der Akku ist GELEERT worden, nicht ohnehin leer
    // gewesen.  Ohne diesen Zaehler waere "belegte Baender == 0" auch dann
    // gruen, wenn die Leerung nie gelaufen ist.
    pruefe (e.verworfeneBandfenster() > verworfenVorher,
            name + ": und die Bandakkus wurden dabei WIRKLICH verworfen",
            juce::String ((int) (e.verworfeneBandfenster() - verworfenVorher))
            + " Band-Fensterbeitraege gefallen");
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

//==============================================================================
// ── G13: DIE ZWILLINGSPROBE ─────────────────────────────────────────────────
//
// 🔑 DIE ANTWORT AUF T2R2-1, UND SIE IST BEWUSST KEINE LAENGERE FELDLISTE.
//
// Dieses Ticket hat dieselbe Lehre zweimal bezahlt.  T2-1: die drei Bandakkus
// standen nicht auf der Liste in `grenzeZiehen()`, und kein Bein sah es.
// T2R2-1: die Leerung der zehn Rahmen-Skalare liess sich entfernen, ohne dass
// eine einzige Zeile rot wurde (gemessen 24.08.: `155 bestanden, 0 Fehler`).
// Beide Male war der Fix richtig und das Bein blind, und beide Male aus
// demselben Grund: DIE PRUEFFRAGE LAUTETE "welche AUSKUNFT steht auf null?".
// Diese Frage kann nur Felder sehen, die eine Auskunft haben.  Die zehn
// Rahmen-Skalare haben keine — und jedes Feld, das jemand morgen hinzufuegt,
// hat auch keine.  Eine Liste im Bein neben der Liste in `grenzeZiehen()` waere
// die Ursache noch einmal, nicht die Kur: zwei Listen laufen auseinander.
//
// Deshalb wechselt hier die FRAGE, nicht die Liste:
//
//     EINE GRENZE MACHT DIE ENGINE UNUNTERSCHEIDBAR VON EINER,
//     DIE DAS FRUEHERE AUDIO NIE GESEHEN HAT.
//
// Zwei Engines, dieselbe Blockfolge Zug um Zug, GEGENSAETZLICHER Inhalt davor
// (A: zwei laute Toene, B: digitale Stille), dann in beiden dieselbe Grenze und
// danach BITGLEICHES Audio in beide.  Ab da muss jeder Frame in A byteweise
// derselbe sein wie in B.  Ueberlebt IRGENDEIN Traeger die Grenze und erreicht
// den Frame, tragen A und B verschiedene Zahlen — ohne dass dieses Bein den
// Traeger kennen muesste.  Das gilt fuer die zehn Rahmen-Skalare, fuer die drei
// Akkus, fuer den K-Filterzustand, fuer die 3-s-Historie und fuer jedes Feld,
// das es heute noch nicht gibt.
//
// 🔑 WARUM DAS UEBERHAUPT GEHT, steht am ENDE von `verarbeiteSamples()`:
// `liveSamples += n` laeuft dort UNBEDINGT, inhaltsunabhaengig.  (Bewusst ohne
// Zeilennummer — T2R2-3 war genau so ein Verweis, der nach ein paar Edits
// danebenzeigte.)  Die zwei Kadenzzaehler, die
// eine Grenze nach dem Entscheid §10.1 absichtlich UEBERLEBEN, koennen zwischen
// den Zwillingen deshalb gar nicht auseinanderlaufen — die Probe steht dem
// Entscheid nicht im Weg, sie lebt von ihm.  Wer ihn kippen wollte, wird von
// G12 rot gemacht, nicht von hier; jedes Bein misst genau eine Sache.
//
// ⚠️ WAS DIESE PROBE NICHT DECKT, und das gehoert dazu: einen Traeger, der die
// Grenze ueberlebt, aber NIE einen Frame erreicht.  Ein solcher Traeger ist
// vom Vertrag her unsichtbar — der Empfaenger sieht Frames.  Und sie deckt
// nicht die PLAUSIBILITAET der Werte (NAK-68), sondern nur ihre Herkunft.
//
// ⚠️ VERGLICHEN WIRD MIT `operator==`, NICHT MIT `memcmp` — und das ist eine
// GEMESSENE Entscheidung, kein Geschmack.  Die erste Fassung dieses Beins
// verglich Bytes; sie meldete in allen sechs Faellen 18 bis 21 abweichende
// Bytes, ohne einen einzigen Feldunterschied.  Die Offsets (26..29, 46..47,
// 59..61, 89) lagen samt und sonders in FUELLBYTES des Transportstempels, an
// 46/47 standen die oberen Bytes einer `double 1.0` aus einem frueheren
// Objekt.  Fuellbytes sind unbeobachtbar, also darf `/O2` ihr Nullen
// weglassen — auch bei `FeatureFrame f {}`.  `operator== = default` vergleicht
// memberweise, ignoriert Fuellbytes und wird vom COMPILER gepflegt: ein neues
// Feld ist automatisch dabei.  Das ist dieselbe Eigenschaft, die der Byte-
// vergleich haben sollte, nur ohne seinen Fehler.

/** Was an zwei Frames verschieden ist — DIAGNOSE, NICHT DECKUNG.

    ⚠️ Die Deckung ist `a == b` in `zwillingsprobe()` und damit der vom Compiler
    erzeugte memberweise Vergleich.  Diese Aufzaehlung macht eine rote Zeile nur
    LESBAR.  Wer ein Feld zu `FeatureFrame` hinzufuegt und hier vergisst,
    verliert Lesbarkeit — keine Schaerfe; der Vergleich schlaegt trotzdem an,
    und die letzte Zeile dieser Funktion sagt dann ausdruecklich, dass der
    Unterschied in einem nicht gezeigten Feld liegt.  Genau darin liegt der
    Unterschied zu der Liste, die T2-1 und T2R2-1 erzeugt hat. */
juce::String frameUnterschied (const FeatureFrame& a, const FeatureFrame& b)
{
    juce::String s;
    auto zeige = [&s] (const char* was, bool gesetztA, float wertA,
                       bool gesetztB, float wertB)
    {
        if (gesetztA != gesetztB || bitsVon (wertA) != bitsVon (wertB))
            s += " " + juce::String (was) + "=" + (gesetztA ? juce::String (wertA, 3) : juce::String ("-"))
               + "/" + (gesetztB ? juce::String (wertB, 3) : juce::String ("-"));
    };
    zeige ("peakDb",      a.peakGesetzt,        a.peakDb,      b.peakGesetzt,        b.peakDb);
    zeige ("crestDb",     a.crestGesetzt,       a.crestDb,     b.crestGesetzt,       b.crestDb);
    zeige ("psrDb",       a.psrGesetzt,         a.psrDb,       b.psrGesetzt,         b.psrDb);
    zeige ("breite",      a.breiteGesetzt,      a.breite,      b.breiteGesetzt,      b.breite);
    zeige ("korrelation", a.korrelationGesetzt, a.korrelation, b.korrelationGesetzt, b.korrelation);
    zeige ("lufsS",       a.lufsSGesetzt,       a.lufsS,       b.lufsSGesetzt,       b.lufsS);
    zeige ("aktivitaet",  a.aktivitaetGesetzt,  a.aktivitaet,  b.aktivitaetGesetzt,  b.aktivitaet);

    int liveA = 0, liveB = 0, evA = 0, evB = 0;
    for (int i = 0; i < Gitter::liveBaender; ++i)
    {
        if (bitmapLies (a.live.bitmap, i)) ++liveA;
        if (bitmapLies (b.live.bitmap, i)) ++liveB;
    }
    for (int i = 0; i < Gitter::evidenzBaender; ++i)
    {
        if (bitmapLies (a.evidenz.bitmap, i)) ++evA;
        if (bitmapLies (b.evidenz.bitmap, i)) ++evB;
    }
    if (liveA != liveB)
        s += " liveBaender=" + juce::String (liveA) + "/" + juce::String (liveB);
    if (evA != evB)
        s += " evidenzBaender=" + juce::String (evA) + "/" + juce::String (evB);
    if (a.transport.sequence != b.transport.sequence)
        s += " sequence=" + juce::String ((int) a.transport.sequence)
           + "/" + juce::String ((int) b.transport.sequence);
    if (a.transport.transport_epoch != b.transport.transport_epoch)
        s += " epoch=" + juce::String ((int) a.transport.transport_epoch)
           + "/" + juce::String ((int) b.transport.transport_epoch);
    // 🔑 DIE EHRLICHE ZEILE: die Aufzaehlung oben ist Lesbarkeit, nicht Deckung.
    // Findet sie nichts, obwohl `operator==` ungleich meldet, sagt sie das —
    // statt eine leere Diagnose zu drucken, die wie "alles unauffaellig"
    // aussieht.  Das ist derselbe Fehler, den T2R2-4 an `Breite=0.000` gefunden
    // hat: eine Diagnose, die bei echtem Bruch unschuldig aussieht, ist
    // schlimmer als keine.
    if (s.isEmpty())
        s = "der Unterschied liegt in einem Feld, das diese Diagnose nicht "
            "namentlich zeigt - `operator==` deckt mehr als sie";
    return s;
}

/** Ereignisse, die der Ring der JETZIGEN Epoche/Segment zuschreibt.

    Der Ring wird an einer Grenze bewusst NICHT geleert (`rahmenLeeren()`:
    "ein Ereignis gehoert zu seiner Epoche, es traegt sie mit").  Damit dieser
    Entscheid nicht bloss dasteht, zaehlt die Zwillingsprobe die Ereignisse
    NACH Epoche: ein Fluss, der ueber die Grenze gerechnet wuerde, erfaende
    einen Onset in der NEUEN Epoche — und dann traegt A einen, den B nicht hat. */
int ereignisseDieserEpoche (const FeatureEngine& e)
{
    int n = 0;
    for (int i = 0; i < e.ereignisAnzahlJetzt(); ++i)
        if (e.ereignis (i).epoche == e.transportEpocheJetzt()
            && e.ereignis (i).segment == e.segmentJetzt())
            ++n;
    return n;
}

void zwillingsprobe (const juce::String& name, Grenzgrund erwartet,
                     const std::function<rt::StampedBlock (Speiser&)>& grenzblock,
                     const std::function<rt::StampedBlock (Speiser&)>& nachBlock,
                     int vorlauf = 340, int nachlauf = 140)
{
    constexpr double kZweiPi = 6.283185307179586476925286766559;
    const double fs = 48000.0;
    FeatureEngine eA, eB;
    eA.vorbereiten (fs);
    eB.vorbereiten (fs);
    Speiser sA { eA }, sB { eB };
    const int n = sA.frames;

    std::vector<float> laut  ((std::size_t) n * 2u, 0.0f);
    std::vector<float> leise ((std::size_t) n * 2u, 0.0f);
    const std::vector<float> stille ((std::size_t) n * 2u, 0.0f);

    // Das leise Audio NACH der Grenze: eigener, fester Zufall — es geht als
    // DERSELBE Puffer in beide Engines, damit die behauptete Bitgleichheit
    // nicht die Gleichheit zweier Generatoren voraussetzt.  Dekorreliert (L und
    // R aus eigenen Zuegen), damit auch die Stereo-Traeger etwas zu rechnen
    // haben, und mit rund -38 dBFS deutlich ueber dem Aktivitaetsgate (-60 dB),
    // aber weit unter dem Ton davor: ein ueberlebender Peak dominiert dann.
    std::uint32_t lcg = 0x13579bdfu;
    auto leiseFuellen = [&]
    {
        auto zug = [&lcg]
        {
            lcg = lcg * 1664525u + 1013904223u;
            return (float) (0.02 * (((double) ((lcg >> 8) & 0xffffu) / 32768.0) - 1.0));
        };
        for (int k = 0; k < n; ++k)
        {
            leise[(std::size_t) k * 2u]      = zug();
            leise[(std::size_t) k * 2u + 1u] = zug();
        }
    };

    // ── Vorlauf: gleiche Bloecke, gegensaetzlicher Inhalt ───────────────────
    auto vorlaufBlock = [&]
    {
        const auto b = sA.bauen();
        for (int k = 0; k < n; ++k)
        {
            const double t = (double) (sA.strom + (std::uint64_t) k) / fs;
            laut[(std::size_t) k * 2u]      = (float) (0.50 * std::sin (kZweiPi * 1000.0 * t));
            laut[(std::size_t) k * 2u + 1u] = (float) (0.31 * std::sin (kZweiPi * 1487.0 * t + 0.7));
        }
        sA.sendenRoh (b, laut);
        sB.sendenRoh (sB.bauen(), stille);   // gleicher Block, leerer Inhalt
    };
    for (int i = 0; i < vorlauf; ++i)
        vorlaufBlock();

    // ⚠️ DIESELBE PHASENFALLE WIE IN `bisBandakkuGefuellt`, und sie hat beim
    // ersten Lauf zugeschlagen: nach einer festen Blockzahl kann der letzte
    // Frame gerade gefallen sein, dann hat `rahmenLeeren()` die Akkus schon
    // geraeumt und A saehe aus wie B.  Der Zustand wird deshalb HERGESTELLT,
    // nicht ausgerechnet — und weil B nie Inhalt traegt, fuehrt A die
    // Schleife; gesendet wird trotzdem in BEIDE, sonst liefe die Kadenz
    // auseinander und der ganze Vergleich waere hinfaellig.
    for (int i = 0; i < 400 && ! (sA.akkusTragenInhalt() && ! sA.frameStehtAn()); ++i)
        vorlaufBlock();

    // Die Vorbedingung, ohne die der ganze Vergleich nichts sagt: die beiden
    // sind VOR der Grenze wirklich verschieden.  Ohne diese Zeile koennte die
    // Probe zweimal denselben leeren Zustand vergleichen und waere gruen, weil
    // sie nichts misst — genau die Sorte Gegenprobe, die §10.4 (T2-4) als
    // Tautologie entlarvt hat.
    pruefe (eA.liveAkkuBelegteBaender() > 0 && eA.evidenzAkkuBelegteBaender() > 0
              && eA.liveBreiteAkkuBelegteBaender() > 0 && eA.kFilterZustand() > 0.0
              && eA.rahmenAktivZellenJetzt() > 0,
            name + " [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas",
            fuellstaende (eA) + " kFilter=" + juce::String (eA.kFilterZustand(), 3, true)
            + " RAktiv=" + juce::String ((int) eA.rahmenAktivZellenJetzt()));
    pruefe (eB.liveAkkuBelegteBaender() == 0 && eB.evidenzAkkuBelegteBaender() == 0
              && eB.liveBreiteAkkuBelegteBaender() == 0 && eB.kFilterZustand() == 0.0
              && eB.rahmenAktivZellenJetzt() == 0,
            name + " [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden",
            fuellstaende (eB) + " kFilter=" + juce::String (eB.kFilterZustand(), 3, true)
            + " RAktiv=" + juce::String ((int) eB.rahmenAktivZellenJetzt()));

    // ── Die Grenze, in beiden derselbe Block mit demselben Audio ────────────
    const auto grundVorher = eA.grenzenMitGrund (erwartet);
    leiseFuellen();
    const auto gA = grenzblock (sA);
    const auto gB = grenzblock (sB);
    sA.sendenRoh (gA, leise);
    sB.sendenRoh (gB, leise);
    pruefe (eA.grenzenMitGrund (erwartet) == grundVorher + 1
              && eB.grenzenMitGrund (erwartet) == grundVorher + 1,
            name + " [Zwilling]: beide haben die Grenze " + grundName (erwartet) + " gezogen",
            "A " + juce::String ((int) eA.grenzenMitGrund (erwartet))
            + " / B " + juce::String ((int) eB.grenzenMitGrund (erwartet)));

    // ── Nachlauf: bitgleiches Audio in beide, jeder Frame verglichen ────────
    int frames = 0, ungleich = 0, kadenzUngleich = 0, mitBaendern = 0, erstesUngleich = -1;
    juce::String ersteDiagnose;
    for (int i = 0; i < nachlauf; ++i)
    {
        leiseFuellen();
        const bool a = sA.sendenRoh (nachBlock (sA), leise);
        const bool b = sB.sendenRoh (nachBlock (sB), leise);
        if (a != b) { ++kadenzUngleich; continue; }
        if (! a) continue;
        ++frames;

        const auto& fa = eA.frame();
        const auto& fb = eB.frame();
        if (! (fa == fb))
        {
            ++ungleich;
            if (erstesUngleich < 0)
            {
                erstesUngleich = i;
                ersteDiagnose = frameUnterschied (fa, fb);
            }
        }
        for (int bd = 0; bd < Gitter::liveBaender; ++bd)
            if (bitmapLies (fa.live.bitmap, bd)) { ++mitBaendern; break; }
    }

    pruefe (kadenzUngleich == 0,
            name + " [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr "
            "haengt nicht am Inhalt (Entscheid §10.1)",
            juce::String (kadenzUngleich) + " Bloecke auseinander");
    pruefe (frames >= 8,
            name + " [Zwilling]: es wurden ueberhaupt genug Frames verglichen",
            juce::String (frames) + " Frames");
    // Zweite Vorbedingung: die verglichenen Frames tragen wirklich Messwerte.
    // Zwei leere Frames waeren auch dann gleich, wenn alles ueberlebte.
    // Hoechstens der ERSTE darf leer sein: das 4096-Punkte-Fenster der
    // Hauptstufe braucht nach der Grenze acht Bloecke, bis es wieder ein
    // Spektrum liefert, und der erste Frame faellt schon nach 9,375 — je nach
    // Phase liegt er davor.  Verglichen wird er trotzdem, er ist der
    // interessanteste.
    pruefe (frames > 0 && mitBaendern >= frames - 1,
            name + " [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille",
            juce::String (mitBaendern) + " von " + juce::String (frames) + " mit Live-Baendern");
    pruefe (ungleich == 0,
            name + " [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu "
            "unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)",
            ungleich == 0 ? juce::String (frames) + " Frames feldgleich"
                          : juce::String (ungleich) + " von " + juce::String (frames)
                            + " ungleich, erster bei Block " + juce::String (erstesUngleich)
                            + "; " + ersteDiagnose);
    pruefe (ereignisseDieserEpoche (eA) == ereignisseDieserEpoche (eB),
            name + " [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche",
            "A " + juce::String (ereignisseDieserEpoche (eA))
            + " / B " + juce::String (ereignisseDieserEpoche (eB)));
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

        // Ein Frame integriert mehrere Hostbloecke. Sein Start und seine
        // Laenge muessen genau dieses Fenster beschreiben, nicht nur den Block,
        // der die 100-ms-Kadenz zuletzt ueberschritten hat.
        {
            FeatureEngine extent;
            extent.vorbereiten (48000.0);
            Speiser sx { extent };
            int bloeckeImFrame = 0;
            while (! sx.senden (sx.bauen()) && bloeckeImFrame < 20)
                ++bloeckeImFrame;
            ++bloeckeImFrame;
            const auto& fx = extent.frame();
            pruefe (fx.transport.project_sample_start_gesetzt
                    && fx.transport.project_sample_start == 0
                    && fx.transport.sample_count == (std::uint32_t) (bloeckeImFrame * sx.frames),
                    "Frame-Stempel spannt alle integrierten Hostbloecke auf",
                    juce::String ((juce::int64) fx.transport.project_sample_start) + " + "
                    + juce::String ((int) fx.transport.sample_count));
        }

        // Das zweite Live-Intervall enthaelt wegen 50-%-FFT-Ueberlappung ein
        // Fenster, das VOR seinem ersten Skalarblock beginnt. Der Stempel muss
        // diesen echten Support nennen statt die alte 100-ms-Schablone.
        {
            FeatureEngine extent;
            extent.vorbereiten (48000.0);
            Speiser sx { extent };
            bool erster = false;
            for (int i = 0; i < 20 && ! erster; ++i)
            {
                auto b = sx.bauen (rt::kFlagContinuousGueltig);
                b.continuousTimeSamples = b.projectSampleStart;
                erster = sx.senden (b);
            }
            bool zweiter = false;
            for (int i = 0; i < 20 && ! zweiter; ++i)
            {
                auto b = sx.bauen (rt::kFlagContinuousGueltig);
                b.continuousTimeSamples = b.projectSampleStart;
                zweiter = sx.senden (b);
            }
            const auto& fx = extent.frame();
            pruefe (erster && zweiter
                    && fx.transport.project_sample_start_gesetzt
                    && fx.transport.project_sample_start == 2048
                    && fx.transport.sample_count == 8192,
                    "Frame-Stempel umfasst den ueberlappenden FFT-Support",
                    juce::String ((juce::int64) fx.transport.project_sample_start) + " + "
                    + juce::String ((int) fx.transport.sample_count));
            pruefe ((fx.transport.gueltigkeit & kGContinuousTime) != 0
                    && fx.transport.continuous_time_samples_gesetzt
                    && fx.transport.continuous_time_samples
                         == fx.transport.project_sample_start,
                    "ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck");
        }

        {
            FeatureEngine rand;
            rand.vorbereiten (48000.0);
            Speiser sx { rand };
            bool erster = false;
            for (int i = 0; i < 20 && ! erster; ++i)
                erster = sx.senden (sx.bauen());
            bool zweiter = false;
            for (int i = 0; i < 20 && ! zweiter; ++i)
            {
                auto b = sx.bauen (rt::kFlagContinuousGueltig);
                b.continuousTimeSamples = std::numeric_limits<std::int64_t>::min();
                zweiter = sx.senden (b);
            }
            const auto& fx = rand.frame();
            pruefe (erster && zweiter
                    && (fx.transport.gueltigkeit & kGContinuousTime) == 0
                    && ! fx.transport.continuous_time_samples_gesetzt,
                    "Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen");
        }

        {
            FeatureEngine luecke;
            luecke.vorbereiten (48000.0);
            Speiser sx { luecke };
            bool erster = false;
            for (int i = 0; i < 20 && ! erster; ++i)
                erster = sx.senden (sx.bauen()); // Host meldet die Uhr noch nicht
            bool zweiter = false;
            for (int i = 0; i < 20 && ! zweiter; ++i)
            {
                auto b = sx.bauen (rt::kFlagContinuousGueltig);
                b.continuousTimeSamples = b.projectSampleStart;
                zweiter = sx.senden (b);
            }
            const auto& fx = luecke.frame();
            pruefe (erster && zweiter
                    && (fx.transport.gueltigkeit & kGContinuousTime) == 0,
                    "fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden");
        }

        // Der zweite faellige Evidenzsnapshot vereinigt auch den aelteren
        // Basssupport. Dieses Bein deckt beide nichttrivialen Union-Zweige;
        // ein reiner Live-/Hauptstufentest koennte sie nicht rot machen.
        {
            FeatureEngine extent;
            extent.vorbereiten (48000.0);
            Speiser sx { extent };
            int frischeEvidenzen = 0;
            FeatureFrame letzter {};
            for (int i = 0; i < 80 && frischeEvidenzen < 2; ++i)
            {
                if (sx.senden (sx.bauen()) && extent.frame().evidenzFrisch)
                {
                    letzter = extent.frame();
                    ++frischeEvidenzen;
                }
            }
            int evidenzBaender = 0;
            for (int b = 0; b < Gitter::evidenzBaender; ++b)
                if (bitmapLies (letzter.evidenz.bitmap, b))
                    ++evidenzBaender;
            pruefe (frischeEvidenzen == 2
                    && letzter.transport.project_sample_start_gesetzt
                    && letzter.transport.project_sample_start == 0
                    && letzter.transport.sample_count == 30720
                    && evidenzBaender > 0,
                    "Evidenzframe vereinigt Haupt- und aelteren Basssupport",
                    juce::String ((juce::int64) letzter.transport.project_sample_start)
                    + " + " + juce::String ((int) letzter.transport.sample_count)
                    + ", " + juce::String (evidenzBaender) + " Baender");
        }

        // Ein band-inaktiver Abschnitt darf eine fehlende Continuous-Uhr
        // nicht verstecken. Bei 192 kHz ist das Evidenzintervall lang genug,
        // um aktive Fenster vor und nach einer voll ausgespuellten stillen
        // Luecke in denselben Snapshot zu legen.
        {
            FeatureEngine luecke;
            luecke.vorbereiten (192000.0);
            Speiser sx { luecke };
            sx.sr = 192000.0;
            auto aktiv = [&]
            {
                auto b = sx.bauen (rt::kFlagContinuousGueltig);
                b.continuousTimeSamples = b.projectSampleStart;
                return sx.senden (b);
            };
            auto stille = [&] (bool continuous)
            {
                auto b = sx.bauen (continuous ? rt::kFlagContinuousGueltig : 0u);
                b.continuousTimeSamples = b.projectSampleStart;
                return sx.sendenMit (b, [] (std::uint32_t) { return 0.0f; });
            };

            bool ersteEvidenz = false;
            for (int i = 0; i < 114; ++i)
                if (aktiv() && luecke.frame().evidenzFrisch)
                    ersteEvidenz = true;
            for (int i = 0; i < 10; ++i) aktiv();
            for (int i = 0; i < 32; ++i) stille (true);
            for (int i = 0; i < 10; ++i) stille (false);
            for (int i = 0; i < 32; ++i) stille (true);
            bool zweiteEvidenz = false;
            for (int i = 0; i < 30; ++i)
                if (aktiv() && luecke.frame().evidenzFrisch)
                    zweiteEvidenz = true;

            const auto& fx = luecke.frame();
            pruefe (ersteEvidenz && zweiteEvidenz && fx.evidenzFrisch
                    && (fx.transport.gueltigkeit & kGContinuousTime) == 0,
                    "stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen");
        }

        // FL darf stehende 1-Sample-Teilstuecke lokal zusammenhalten (G10),
        // aber daraus entsteht keine fortlaufende Projektachse.
        {
            FeatureEngine stehend;
            stehend.vorbereiten (48000.0);
            Speiser sx { stehend };
            sx.frames = 1;
            bool gebaut = false;
            for (int i = 0; i < 5000 && ! gebaut; ++i)
            {
                gebaut = sx.senden (sx.bauen());
                sx.projekt = 0;
            }
            const auto& fx = stehend.frame();
            pruefe (gebaut && fx.transport.zeitbasis == Zeitbasis::local_monotonic
                    && ! fx.transport.project_sample_start_gesetzt
                    && (fx.transport.gueltigkeit & kGProjectTime) == 0,
                    "stehende FL-Teilstuecke erfinden kein Projektintervall");
        }

        // Auch ein einzelner Hostblock nahe INT64_MAX darf nicht mit einer
        // ueberlaufenden Projektspanne publiziert werden. Lokal bleibt er
        // verwendbar; nur die unbewiesene Zeitangabe faellt.
        {
            FeatureEngine rand;
            rand.vorbereiten (48000.0);
            Speiser sx { rand };
            sx.frames = 4800;
            auto b = sx.bauen();
            b.projectSampleStart = std::numeric_limits<std::int64_t>::max() - 100;
            sx.audio.assign ((std::size_t) b.sampleCount * 2u, 0.1f);
            const bool gebaut = rand.nimmBlock (b, sx.audio.data());
            const auto& fx = rand.frame();
            pruefe (gebaut && fx.transport.zeitbasis == Zeitbasis::local_monotonic
                    && ! fx.transport.project_sample_start_gesetzt
                    && fx.transport.sample_count == b.sampleCount,
                    "ueberlaufende Host-Projektspanne wird konservativ lokal publiziert");
        }

        // Die Kadenzuhr bleibt an einer Grenze bewusst stehen. Dann darf ein
        // unmittelbar faelliger Frame trotzdem NUR Nachgrenzen-Audio nennen.
        {
            FeatureEngine extent;
            extent.vorbereiten (48000.0);
            Speiser sx { extent };
            for (int i = 0; i < 9; ++i)
                sx.senden (sx.bauen());
            sx.projekt += 100000;
            const auto ersterNachGrenze = sx.projekt;
            const bool gebaut = sx.senden (sx.bauen());
            const auto& fx = extent.frame();
            pruefe (gebaut && fx.transport.project_sample_start == ersterNachGrenze
                    && fx.transport.sample_count == (std::uint32_t) sx.frames,
                    "Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block",
                    juce::String ((juce::int64) fx.transport.project_sample_start) + " + "
                    + juce::String ((int) fx.transport.sample_count));
        }

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
            b.continuousTimeSamples = b.projectSampleStart + 12345;
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
        pruefe (f3.transport.continuous_time_samples
                    == f3.transport.project_sample_start + 12345,
                "continuous_time_samples kommt mit demselben Frameanker durch");

        auto pruefeKaputteCycleGrenze = [&] (double start, double ende,
                                             const juce::String& fall)
        {
            FeatureEngine kaputt;
            kaputt.vorbereiten (48000.0);
            Speiser sx { kaputt };
            bool gebaut = false;
            for (int i = 0; i < 20 && ! gebaut; ++i)
            {
                auto b = sx.bauen (rt::kFlagCycleGrenzenGueltig);
                b.cycleStartPpq = start;
                b.cycleEndePpq = ende;
                gebaut = sx.senden (b);
            }
            const auto& fx = kaputt.frame();
            pruefe (gebaut && ! fx.transport.cycle_bounds_valid
                    && ! fx.transport.cycle_start_ppq_gesetzt
                    && ! fx.transport.cycle_end_ppq_gesetzt
                    && (fx.transport.gueltigkeit & kGCycleBounds) == 0,
                    "unbrauchbare Cycle-Bounds werden nicht publiziert: " + fall);
        };
        pruefeKaputteCycleGrenze (std::numeric_limits<double>::quiet_NaN(), 8.0,
                                  "NaN");
        pruefeKaputteCycleGrenze (4.0, std::numeric_limits<double>::infinity(),
                                  "Inf");
        pruefeKaputteCycleGrenze (8.0, 4.0, "Ende vor Start");

        // valid -> invalid ist der wichtige Gegenpfad: ein blosses `return`
        // liesse die alte Rate und Freigabe aktiv.
        FeatureEngine rate;
        rate.vorbereiten (48000.0);
        Speiser sr { rate };
        sr.laufen (10);
        rate.vorbereiten (std::numeric_limits<double>::quiet_NaN());
        auto blockNachNaN = sr.bauen();
        sr.audio.assign ((std::size_t) blockNachNaN.sampleCount * 2u, 0.1f);
        const bool liefMitAlterRate = rate.nimmBlock (blockNachNaN, sr.audio.data());
        rate.vorbereiten (48000.0);
        bool nachReprepare = false;
        for (int i = 0; i < 20 && ! nachReprepare; ++i)
            nachReprepare = sr.senden (sr.bauen());
        pruefe (! liefMitAlterRate && nachReprepare,
                "ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich");
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
        // Weiter mit DERSELBEN Blockart, bis die Akkus tragen — ein
        // `bauen()`-Block waere hier selbst eine Transportkante.
        s.bisBandakkuGefuellt ([&s] { return s.bauen (0, false); });
        pruefe (s.allesOffen(), "G3: bei gestopptem Transport laufen die Fenster normal weiter",
                fuellstaende (e));
        pruefe (s.akkusTragenInhalt(), "G3: und die Bandakkus tragen Inhalt",
                fuellstaende (e));
        pruefe (e.epochenwechsel() == 0,
                "G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)",
                juce::String ((int) e.epochenwechsel()) + " Epochenwechsel");

        const auto vorher = e.grenzenMitGrund (Grenzgrund::zeitSprung);
        const auto verworfenVorher = e.verworfeneBandfenster();
        s.projekt += 480000;                 // der User zieht den Playhead
        auto b = s.bauen (0, false);
        s.senden (b);
        pruefe (keinFensterUeberbrueckt (e, s.frames, s.sr),
                "G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster", fuellstaende (e));
        // T2R2-2 (24.08.): die positive Haelfte gab es hier NICHT — G3, G4 und
        // G5 bauen sich selbst auf statt ueber `grenzfall()`, und die Zusage
        // "jeder der neun Grenzfaelle prueft sie" war damit fuer drei falsch.
        pruefe (e.verworfeneBandfenster() > verworfenVorher,
                "G3: und die Bandakkus wurden dabei WIRKLICH verworfen",
                juce::String ((int) (e.verworfeneBandfenster() - verworfenVorher))
                + " Band-Fensterbeitraege gefallen");
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
        s.bisBandakkuGefuellt ([&]
        {
            auto bb = s.bauen (loop);
            bb.tempo = bpm; bb.ppqPosition = ppq;
            bb.cycleStartPpq = 96.0; bb.cycleEndePpq = 1000.0;
            ppq += ppqProBlock;
            return bb;
        });
        pruefe (s.allesOffen(), "G4: aktive Schleife allein trennt nichts", fuellstaende (e));
        pruefe (s.akkusTragenInhalt(), "G4: und die Bandakkus tragen Inhalt", fuellstaende (e));

        const auto vorher = e.grenzenMitGrund (Grenzgrund::loopWrap);
        const auto verworfenVorher = e.verworfeneBandfenster();
        s.projekt -= 480000;                 // zurueck an den Schleifenanfang
        auto b = s.bauen (loop);
        b.tempo = bpm; b.ppqPosition = 96.0;
        b.cycleStartPpq = 96.0; b.cycleEndePpq = 1000.0;
        s.senden (b);
        pruefe (keinFensterUeberbrueckt (e, s.frames, s.sr),
                "G4 Loop-Wrap: jedes Fenster getrennt", fuellstaende (e));
        pruefe (e.verworfeneBandfenster() > verworfenVorher,   // T2R2-2
                "G4: und die Bandakkus wurden dabei WIRKLICH verworfen",
                juce::String ((int) (e.verworfeneBandfenster() - verworfenVorher))
                + " Band-Fensterbeitraege gefallen");
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
        s.bisBandakkuGefuellt ([&]
        {
            auto bb = s.bauen (loop);
            bb.tempo = bpm; bb.ppqPosition = ppq;
            bb.cycleStartPpq = 0.0; bb.cycleEndePpq = 1e9;
            ppq += ppqProBlock;
            return bb;
        });
        pruefe (s.allesOffen(), "G5: Schleife ohne erreichbare Grenze trennt nichts",
                fuellstaende (e));
        pruefe (s.akkusTragenInhalt(), "G5: und die Bandakkus tragen Inhalt", fuellstaende (e));

        const auto vorher = e.straddleVerworfen();
        const auto verworfenVorher = e.verworfeneBandfenster();
        auto b = s.bauen (loop);
        b.tempo = bpm; b.ppqPosition = ppq;
        b.cycleStartPpq = 0.0;
        b.cycleEndePpq = ppq + 0.5 * ppqProBlock;          // mitten im Block
        s.senden (b);
        pruefe (alleFensterLeer (e),
                "G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, "
                "weil der Straddle-Block auch kein neues beginnt", fuellstaende (e));
        pruefe (e.verworfeneBandfenster() > verworfenVorher,   // T2R2-2
                "G5: und die Bandakkus wurden dabei WIRKLICH verworfen",
                juce::String ((int) (e.verworfeneBandfenster() - verworfenVorher))
                + " Band-Fensterbeitraege gefallen");
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

    // G12 - T2-1 ALS STEHENDES BEIN: nicht der Fuellstand, sondern der FRAME.
    //
    // 🔑 DAS IST DIE LEHRE DIESER NACHARBEIT.  G1..G11 messen, was NACH der
    // Grenze in den Fenstern steht — und das war die ganze Zeit richtig.
    // Ungemessen blieb, was im veroeffentlichten FRAME steht, und dort meldete
    // die Engine unter dem Stempel der NEUEN Epoche den Ton von VOR der Grenze
    // (23 Live-Baender, staerkstes bei 1029 Hz mit -23,7 dB).  Gemessen wird
    // deshalb hier wie beim Pruefer: nach der Grenze laeuft AUSSCHLIESSLICH
    // digitale Stille, jeder Wert im Frame kann also nur von davor stammen.
    {
        const double fs = 48000.0;
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        auto ton    = [&s] (std::uint32_t i) { return sinus1k (s.strom + i); };
        auto stille = [] (std::uint32_t) { return 0.0f; };

        // 29 Bloecke Ton: Frames fallen bei Block 10 und 20 (je 4800 Samples),
        // der Evidenzsatz sammelt ueber alle 29 weiter (er faellt erst bei
        // 12000).  Block 30 traegt dann BEIDE Kadenzen zugleich — nur so ist
        // auch die Evidenzseite wirklich geprueft und nicht bloss deshalb leer,
        // weil sie gar nicht faellig war.
        for (int i = 0; i < 29; ++i)
            s.sendenMit (s.bauen(), ton);

        const int liveVorher    = e.liveAkkuBelegteBaender();
        const int evidenzVorher = e.evidenzAkkuBelegteBaender();
        pruefe (liveVorher > 0 && evidenzVorher > 0,
                "G12: vor der Grenze tragen BEIDE Bandakkus den Ton",
                "Live " + juce::String (liveVorher) + " / Evidenz "
                + juce::String (evidenzVorher) + " Baender");

        s.projekt += 480000;                          // Seek um 10 s = Grenze
        const bool gebaut = s.sendenMit (s.bauen(), stille);
        const auto& f = e.frame();

        pruefe (gebaut,
                "G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber "
                "die Grenze weiter (Entscheid §10.1)",
                "liveSamples=" + juce::String ((int) e.liveSamplesJetzt()));
        pruefe (f.transport.transport_epoch == 1,
                "G12: und der Frame traegt die NEUE Epoche",
                juce::String ((int) f.transport.transport_epoch));
        pruefe (f.evidenzFrisch,
                "G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird "
                "also wirklich geprueft");

        int liveGesetzt = 0, evidenzGesetzt = 0;
        for (int b = 0; b < Gitter::liveBaender; ++b)
            if (bitmapLies (f.live.bitmap, b)) ++liveGesetzt;
        for (int b = 0; b < Gitter::evidenzBaender; ++b)
            if (bitmapLies (f.evidenz.bitmap, b)) ++evidenzGesetzt;

        pruefe (liveGesetzt == 0,
                "G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)",
                juce::String (liveGesetzt) + " gesetzte Baender");
        pruefe (evidenzGesetzt == 0,
                "G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter",
                juce::String (evidenzGesetzt) + " gesetzte Baender");
        pruefe (! (f.aktivitaetGesetzt && f.aktivitaet > 0.0f),
                "G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit",
                f.aktivitaetGesetzt ? juce::String (f.aktivitaet, 3) : juce::String ("nicht gesetzt"));
        pruefe (e.verworfeneBandfenster() > 0,
                "G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden",
                juce::String ((int) e.verworfeneBandfenster()) + " Band-Fensterbeitraege");

        // Kein Einzelfall: derselbe Sweep, den der Pruefer gefahren hat.
        int mitBaendern = 0, mitAktivitaet = 0, mitEvidenz = 0, gemessen = 0;
        for (int vorlauf = 1; vorlauf <= 120; ++vorlauf)
        {
            FeatureEngine ee;
            ee.vorbereiten (fs);
            Speiser ss { ee };
            auto tonS    = [&ss] (std::uint32_t i) { return sinus1k (ss.strom + i); };
            auto stilleS = [] (std::uint32_t) { return 0.0f; };
            for (int i = 0; i < vorlauf; ++i)
                ss.sendenMit (ss.bauen(), tonS);

            ss.projekt += 480000;                     // Grenze, danach nur Stille
            bool fertig = false;
            for (int i = 0; i < 40 && ! fertig; ++i)
                fertig = ss.sendenMit (ss.bauen(), stilleS);
            if (! fertig)
                continue;                             // kein Frame faellig geworden
            ++gemessen;

            const auto& fr = ee.frame();
            for (int b = 0; b < Gitter::liveBaender; ++b)
                if (bitmapLies (fr.live.bitmap, b)) { ++mitBaendern; break; }
            for (int b = 0; b < Gitter::evidenzBaender; ++b)
                if (bitmapLies (fr.evidenz.bitmap, b)) { ++mitEvidenz; break; }
            if (fr.aktivitaetGesetzt && fr.aktivitaet > 0.0f) ++mitAktivitaet;
        }
        pruefe (gemessen >= 100,
                "G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren",
                juce::String (gemessen) + " von 120");
        pruefe (mitBaendern == 0 && mitEvidenz == 0 && mitAktivitaet == 0,
                "G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor "
                "(Pruefer-Messung: 80 / 40 / 43)",
                juce::String (mitBaendern) + " mit Live-Baendern, "
                + juce::String (mitEvidenz) + " mit Evidenz, "
                + juce::String (mitAktivitaet) + " mit Aktivitaet");
    }

    //==========================================================================
    std::cout << std::endl
              << "== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) =="
              << std::endl;
    // G13.0 - KANN DER VERGLEICH ueberhaupt ungleich melden?  Dieselbe Frage,
    // die §10.4 (T2-4) an der alten L2-Gegenprobe gestellt hat, nur diesmal
    // VORHER: der ganze Abschnitt haengt an `FeatureFrame::operator==`, und ein
    // Vergleich, der immer `true` sagt, machte jede Zeile darunter zur
    // Tautologie.  Geprueft wird in beide Richtungen und ueber die drei
    // Feldsorten (Skalar, Bitmap, Bandwert), damit nicht bloss EIN Member den
    // Vergleich traegt.
    {
        FeatureFrame x {}, y {};
        pruefe (x == y, "G13.0: zwei frische Frames sind gleich - der Vergleich sagt "
                        "nicht pauschal ungleich");
        y.peakDb = 1.0f;
        pruefe (! (x == y), "G13.0: ein einziger geaenderter Skalar macht sie ungleich");
        y = x; y.live.werte[17] = 42;
        pruefe (! (x == y), "G13.0: ein einziger geaenderter Bandwert ebenso");
        y = x; y.evidenz.bitmap[3] = 0x08;
        pruefe (! (x == y), "G13.0: und ein einziges Bitmapbit ebenso");
        y = x; y.transport.sequence = 5;
        pruefe (! (x == y), "G13.0: und ein Feld im verschachtelten Transportstempel - "
                            "der Vergleich reicht bis dorthin");
        y = x;
        pruefe (x == y, "G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen "
                        "vorgefuehrt");
    }
    // Sechs der neun Grenzarten laufen hier als Zwillingspaar.  Nicht dabei
    // sind `loopWrap` und `moeglicherStraddle` (sie brauchen eine PPQ-Fuehrung
    // in JEDEM Block, auch im Nachlauf) und der gestoppte `zeitSprung` aus G3.
    // Das ist eine Grenze der VORFUEHRUNG, nicht der Deckung: alle neun Arten
    // laufen durch dieselbe `grenzeZiehen()`, und die ist es, die hier geprueft
    // wird — die Ausloesepfade selbst deckt G1..G12 auf Traegerebene ab.
    zwillingsprobe ("G13a lokaleLuecke", Grenzgrund::lokaleLuecke,
                    [] (Speiser& s)
                    {
                        ++s.segment;
                        auto b = s.bauen (rt::kFlagLueckeDavor);
                        s.strom += 4096;                 // die verlorene Zeit
                        b.stromVon = s.strom;
                        return b;
                    },
                    [] (Speiser& s) { return s.bauen(); });

    zwillingsprobe ("G13b zeitSprung", Grenzgrund::zeitSprung,
                    [] (Speiser& s) { s.projekt += 480000; return s.bauen(); },
                    [] (Speiser& s) { return s.bauen(); });

    zwillingsprobe ("G13c transportKante", Grenzgrund::transportKante,
                    [] (Speiser& s) { return s.bauen (0, /*spielt*/ false); },
                    [] (Speiser& s) { return s.bauen (0, false); });

    zwillingsprobe ("G13d sampleratewechsel", Grenzgrund::sampleratewechsel,
                    [] (Speiser& s) { s.sr = 44100.0; return s.bauen(); },
                    [] (Speiser& s) { return s.bauen(); });

    zwillingsprobe ("G13e neuanlauf", Grenzgrund::neuanlauf,
                    [] (Speiser& s) { ++s.startFolge; return s.bauen(); },
                    [] (Speiser& s) { return s.bauen(); });

    zwillingsprobe ("G13f beweislageWechsel", Grenzgrund::beweislageWechsel,
                    [] (Speiser& s)
                    {
                        auto b = s.bauen (0, true, /*zeitGueltig*/ false);
                        b.flags &= ~rt::kFlagKontextAnwesend;
                        return b;
                    },
                    [] (Speiser& s)
                    {
                        auto b = s.bauen (0, true, false);
                        b.flags &= ~rt::kFlagKontextAnwesend;
                        return b;
                    });

    //==========================================================================
    std::cout << std::endl << "== H - NAK-29: bedingte Feldpflichten des Transportstempels ==" << std::endl;
    {
        Transportstempel gut;
        gut.zeitbasis = Zeitbasis::project_samples;
        gut.project_sample_start_gesetzt = true;
        gut.gueltigkeit = kGProjectTime;
        gut.sample_count = 1;
        pruefe (nak29Verstoss (gut) == 0, "ein sauberer Stempel kommt durch");

        auto t1 = gut; t1.project_sample_start_gesetzt = false;
        pruefe (nak29Verstoss (t1) == 1,
                "Fall 1: project_samples + Bit, aber KEIN project_sample_start");
        auto t1Rueckrichtung = gut; t1Rueckrichtung.gueltigkeit &= ~kGProjectTime;
        pruefe (nak29Verstoss (t1Rueckrichtung) == 1,
                "Fall 1: project_samples + project_sample_start, aber KEIN project_time-Bit");

        auto t2 = gut; t2.zeitbasis = Zeitbasis::local_monotonic;
        pruefe (nak29Verstoss (t2) == 2,
                "Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu §32.3)");
        auto t2Rueckrichtung = t2; t2Rueckrichtung.gueltigkeit &= ~kGProjectTime;
        pruefe (nak29Verstoss (t2Rueckrichtung) == 2,
                "Fall 2: local_monotonic MIT project_sample_start (Widerspruch zu §32.3)");

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

        auto t7 = gut;
        t7.project_sample_start = std::numeric_limits<std::int64_t>::max();
        pruefe (nak29Verstoss (t7) == 7,
                "Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus");
        auto t7b = gut; t7b.sample_count = 1048577u;
        pruefe (nak29Verstoss (t7b) == 7,
                "Fall 7: sample_count bleibt unter der Vertragsobergrenze");
        auto t7c = gut; t7c.sample_count = 0;
        pruefe (nak29Verstoss (t7c) == 7,
                "Fall 7: der Feature-Erzeuger publiziert keinen Leerframe");

        // Und der Riegel sitzt im ERZEUGER, nicht nur als Funktion daneben.
        //
        // Der normale Pfad bleibt ablehnungsfrei; der unmittelbar folgende
        // Oversize-Fall befährt bewusst den echten Gegenpfad.
        FeatureEngine e;
        e.vorbereiten (48000.0);
        Speiser s { e };
        s.laufen (60);
        pruefe (e.nak29Abgelehnt() == 0,
                "der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel "
                "entsteht im normalen, gedeckelten Betrieb",
                juce::String ((int) e.nak29Abgelehnt()));

        // Ein einmaliger, zu grosser Erzeugerversuch darf den laenger laufenden
        // Evidenzakku nicht festklemmen. Ohne dessen Konsum wiederholt jeder
        // Folgeframe denselben >1-Mi-Sample-Stempel und die Telemetrie kommt
        // nie mehr hoch.
        FeatureEngine erholung;
        erholung.vorbereiten (48000.0);
        Speiser gross { erholung };
        gross.frames = 1048577;
        auto zuGross = gross.bauen();
        gross.audio.assign ((std::size_t) zuGross.sampleCount * 2u, 0.1f);
        const bool abgelehnt = gross.sendenRoh (zuGross, gross.audio);
        gross.frames = 512;
        bool wiederDa = false;
        for (int i = 0; i < 30 && ! wiederDa; ++i)
            wiederDa = gross.senden (gross.bauen());
        pruefe (! abgelehnt && erholung.nak29Abgelehnt() == 1 && wiederDa,
                "abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich",
                juce::String ((int) erholung.nak29Abgelehnt()));
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

        // ── Der Deckel, WIRKLICH gefahren ───────────────────────────────────
        //
        // ⚠️ T2-5: bis zum 24.08. stand hier nur `1 <= 64`.  Der Lauf oben
        // erzeugt EIN Ereignis, `ereignisseVerworfen()` wurde nirgends geprueft
        // und keine der sieben Mutationen zielte auf den Ring — "Ring fest
        // gedeckelt" war damit eine Behauptung ueber einen Fall, den das Bein
        // nie erreicht hat.
        //
        // Der Reiz liegt im Signal: ein Onset feuert nur, wenn der Fluss
        // Median+3·MAD ueber die letzten 16 Fenster ueberschreitet, und stille
        // Fenster gehen gar nicht erst in die Flussrechnung (das Aktivitaetsgate
        // greift davor).  Ein strenger Wechsel laut/leise feuert deshalb NIE
        // (bei acht hohen und acht tiefen Werten liegt die Schwelle bei 2H-L).
        // Was feuert, ist ein RUHIGER Boden mit seltenen Ausschlaegen: ein
        // 1-kHz-Sinus bei -26 dBFS haelt jedes Fenster aktiv und den Fluss nahe
        // null, ein breitbandiger Ausschlag alle vier Hops hebt ihn weit
        // darueber.
        FeatureEngine e5;
        e5.vorbereiten (48000.0);
        Speiser s5 { e5 };
        constexpr double kZweiPi = 6.283185307179586476925286766559;
        std::uint64_t aeltestesBeiVoll = 0;
        bool warVoll = false;
        int bloecke = 0;
        for (; bloecke < 4000; ++bloecke)
        {
            const bool ausschlag = (bloecke % 16) == 0;
            s5.sendenMit (s5.bauen(), [&] (std::uint32_t i)
            {
                if (ausschlag)
                    return s5.rausch() * 3.2f;        // breitbandig, ~-2 dBFS
                return (float) (0.05 * std::sin (kZweiPi * 1000.0
                                * (double) (s5.strom + i) / 48000.0));
            });
            if (! warVoll && e5.ereignisAnzahlJetzt() == FeatureEngine::kEreignisPlaetze
                && e5.ereignisseVerworfen() == 0)
            {
                warVoll = true;
                aeltestesBeiVoll = e5.ereignis (0).stromSample;
            }
            if (e5.ereignisseVerworfen() > 0)
                break;
        }
        pruefe (warVoll && e5.ereignisseVerworfen() > 0,
                "der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, "
                "nicht behauptet",
                juce::String (e5.ereignisAnzahlJetzt()) + " im Ring, "
                + juce::String ((int) e5.ereignisseVerworfen()) + " verworfen, nach "
                + juce::String (bloecke) + " Bloecken");
        pruefe (e5.ereignisAnzahlJetzt() == FeatureEngine::kEreignisPlaetze,
                "und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (§48.1: ein "
                "Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)",
                juce::String (e5.ereignisAnzahlJetzt()) + " / "
                + juce::String (FeatureEngine::kEreignisPlaetze));
        pruefe (warVoll && e5.ereignis (0).stromSample > aeltestesBeiVoll,
                "drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste",
                "aeltestes vorher " + juce::String ((int) aeltestesBeiVoll)
                + ", jetzt " + juce::String ((int) e5.ereignis (0).stromSample));
        bool aufsteigend = true;
        bool fensterstartGenau = true;
        for (int i = 1; i < e5.ereignisAnzahlJetzt(); ++i)
            if (e5.ereignis (i).stromSample <= e5.ereignis (i - 1).stromSample)
                aufsteigend = false;
        for (int i = 0; i < e5.ereignisAnzahlJetzt(); ++i)
        {
            const auto& ereignis = e5.ereignis (i);
            if ((ereignis.stromSample % 2048u) != 0u
                || ! ereignis.projektzeitGesetzt
                || ereignis.projektSample != (std::int64_t) ereignis.stromSample)
                fensterstartGenau = false;
        }
        pruefe (aufsteigend,
                "und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf");
        pruefe (fensterstartGenau,
                "Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock");

        // Gegenpfad zur normalen Eventzeit: derselbe lokale Signalverlauf bei
        // einer ueber alle Hostbloecke STEHENDEN Projektzeit. Ein Event darf
        // lokal weiter existieren, seine Projektzeit ist aber unbewiesen.
        FeatureEngine e6;
        e6.vorbereiten (48000.0);
        Speiser s6 { e6 };
        int stehendBloecke = 0;
        for (; stehendBloecke < 1500 && e6.ereignisAnzahlJetzt() == 0; ++stehendBloecke)
        {
            const bool ausschlag = (stehendBloecke % 16) == 0;
            s6.sendenMit (s6.bauen(), [&] (std::uint32_t i)
            {
                if (ausschlag)
                    return s6.rausch() * 3.2f;
                return (float) (0.05 * std::sin (kZweiPi * 1000.0
                                * (double) (s6.strom + i) / 48000.0));
            });
            s6.projekt = 0;
        }
        bool alleEventsLokal = e6.ereignisAnzahlJetzt() > 0;
        for (int i = 0; i < e6.ereignisAnzahlJetzt(); ++i)
            if (e6.ereignis (i).projektzeitGesetzt)
                alleEventsLokal = false;
        pruefe (alleEventsLokal,
                "stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit",
                juce::String (e6.ereignisAnzahlJetzt()) + " Ereignis(se) nach "
                + juce::String (stehendBloecke) + " Bloecken");
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

        const auto frame = p.merkmalFrame();
        const auto& t = frame.transport;
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
        const auto frame2 = p2.merkmalFrame();
        const auto& t2 = frame2.transport;
        pruefe (p2.merkmaleFrames() > 0 && t2.gueltigkeit != kGAlleSieben,
                "Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits",
                "0x" + juce::String::toHexString ((int) t2.gueltigkeit));
        pruefe ((t2.gueltigkeit & kGRecordState) == 0,
                "insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`");

        // Projektfenster am int64-Rand: kein signed overflow im Audiocallback,
        // und ein nicht darstellbares Blockende wird nicht als echte Zeit
        // publiziert.
        auto randBlock = [&] (EqCopilotProcessor& prozessor, std::int64_t start)
        {
            eqcop::hostbruecke::Blockbefund rand;
            rand.kontext.leeren();
            rand.kontext.processContextPresent = true;
            rand.kontext.projectTimeSamples.setze (start);
            rand.kontext.playing.setze (true);
            rand.kontext.sampleRate.setze (fs);
            rand.blockGroesse = (std::uint32_t) bs;
            prozessor.nakamaBlockEmpfangen (rand);
            puffer.clear();
            prozessor.processBlock (puffer, midi);
        };
        EqCopilotProcessor p3;
        p3.setPlayConfigDetails (2, 2, fs, bs);
        p3.prepareToPlay (fs, bs);
        randBlock (p3, std::numeric_limits<std::int64_t>::max() - 100);
        const auto randUeberlauf = p3.messKompakt();
        pruefe (! randUeberlauf.fensterGueltig && randUeberlauf.fensterSpruenge == 1,
                "Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen");

        EqCopilotProcessor p4;
        p4.setPlayConfigDetails (2, 2, fs, bs);
        p4.prepareToPlay (fs, bs);
        randBlock (p4, std::numeric_limits<std::int64_t>::min());
        randBlock (p4, std::numeric_limits<std::int64_t>::max() - bs);
        const auto randAbstand = p4.messKompakt();
        pruefe (randAbstand.fensterGueltig && randAbstand.fensterSpruenge == 1,
                "Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt");

        FeatureEngine ungueltigeRate;
        ungueltigeRate.vorbereiten (std::numeric_limits<double>::infinity());
        ungueltigeRate.vorbereiten (std::numeric_limits<double>::quiet_NaN());
        pruefe (ungueltigeRate.samplerate() == 0.0,
                "nichtendliche Sampleraten initialisieren keine Analyseengine");
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

        // L2 - Der Gittervergleich aus Abschnitt A, gegen eine um EIN Bit
        // veraenderte Fixture-Zeile gefahren.
        //
        // ⚠️ DIE ERSTE FASSUNG WAR EINE TAUTOLOGIE (T2-4): `echt != (echt ^ 1)`
        // ist fuer jedes uint64 wahr und ruft keine Produktionszeile auf — eine
        // Gegenprobe, die nicht scheitern kann, ausgerechnet im Abschnitt "kann
        // dieses Bein ueberhaupt rot werden?".  Jetzt faehrt sie den ECHTEN
        // Vergleich: dieselbe Kette `bitsVon(Gitter::…) != hexBits(text)` wie in
        // A, einmal gegen den Sollwert und einmal gegen eine gekippte Zeile.
        auto hexZeile = [] (std::uint64_t b)
        {
            return "0x" + juce::String::toHexString ((juce::int64) b).paddedLeft ('0', 16);
        };
        const int gekippteZeile = 26;
        int abwEcht = 0, abwGekippt = 0;
        for (int i = 0; i <= Gitter::evidenzBaender; ++i)
        {
            const auto soll = bitsVon (Gitter::evidenzKante (i));
            if (bitsVon (Gitter::evidenzKante (i)) != hexBits (hexZeile (soll)))
                ++abwEcht;
            const auto zeile = (i == gekippteZeile) ? (soll ^ 1ull) : soll;
            if (bitsVon (Gitter::evidenzKante (i)) != hexBits (hexZeile (zeile)))
                ++abwGekippt;
        }
        pruefe (abwEcht == 0,
                "L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen",
                juce::String (abwEcht));
        pruefe (abwGekippt == 1,
                "L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, "
                "und zwar nur dort",
                juce::String (abwGekippt) + " Abweichung(en), Zeile "
                + juce::String (gekippteZeile));

        // L3 - Der NAK-29-Riegel unterscheidet die Faelle, statt nur 'nein' zu
        // sagen: sieben verschiedene Verletzungen, sieben verschiedene Nummern.
        Transportstempel g;
        g.zeitbasis = Zeitbasis::project_samples;
        g.project_sample_start_gesetzt = true;
        g.gueltigkeit = kGProjectTime;
        g.sample_count = 1;
        bool alleVerschieden = true;
        int gesehen[8] = {};
        auto zaehle = [&] (Transportstempel t) { const int v = nak29Verstoss (t); if (v >= 1 && v <= 7) ++gesehen[v]; };
        { auto t = g; t.project_sample_start_gesetzt = false; zaehle (t); }
        { auto t = g; t.zeitbasis = Zeitbasis::local_monotonic; zaehle (t); }
        { auto t = g; t.cycle_bounds_valid = true; zaehle (t); }
        { auto t = g; t.cycle_derivation = Herleitung::validated_block_mapping; zaehle (t); }
        { auto t = g; t.gueltigkeit |= kGCycleBounds; zaehle (t); }
        { auto t = g; t.gueltigkeit |= kGContinuousTime; zaehle (t); }
        { auto t = g; t.project_sample_start = std::numeric_limits<std::int64_t>::max(); zaehle (t); }
        for (int i = 1; i <= 7; ++i) if (gesehen[i] != 1) alleVerschieden = false;
        pruefe (alleVerschieden,
                "L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'");

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

        // L5 - Der Meldeweg des NAK-29-Riegels: ein abgelehnter Frameversuch
        // VERBRAUCHT seine Sequenznummer, und die Luecke ist die Nachricht.
        //
        // ⚠️ Bis zum 24.08. stimmte das nicht (T2-2): `++sequenz` stand hinter
        // dem Ablehnungszweig, der Empfaenger saehe `1, 2, 3, …` ohne Luecke,
        // und §4.5 beschrieb ein Verhalten, das der Code nicht hatte.
        // Hier wird der Entscheid zusaetzlich EINZELN gefahren, damit exakt die
        // Sequenzluecke 1,3 sichtbar wird; H befährt den Produktionszweig.
        {
            Transportstempel gut;
            gut.zeitbasis = Zeitbasis::project_samples;
            gut.project_sample_start_gesetzt = true;
            gut.gueltigkeit = kGProjectTime;
            gut.sample_count = 1;
            auto kaputt = gut; kaputt.project_sample_start_gesetzt = false;

            std::uint64_t zaehler = 0;
            std::vector<std::uint64_t> veroeffentlicht;
            for (const auto& t : { gut, kaputt, gut })
            {
                const auto v = frameversuch (zaehler, t);
                if (v.verstoss == 0)
                    veroeffentlicht.push_back (v.sequence);
            }
            pruefe (zaehler == 3,
                    "L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte",
                    juce::String ((int) zaehler));
            pruefe (veroeffentlicht.size() == 2
                    && veroeffentlicht[0] == 1 && veroeffentlicht[1] == 3,
                    "L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung",
                    veroeffentlicht.size() == 2
                        ? juce::String ((int) veroeffentlicht[0]) + ", "
                          + juce::String ((int) veroeffentlicht[1])
                        : juce::String ((int) veroeffentlicht.size()) + " Frames");
            pruefe (frameversuch (zaehler, kaputt).verstoss == 1,
                    "L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'");
        }

        // L6 - Die Ursachen-Auskunft liest nicht ueber ihr Array hinaus.
        //
        // ⚠️ T2-3: `grundZaehler` hat exakt `anzahl` Elemente, und
        // `Grenzgrund::anzahl` ist ein oeffentlich sichtbarer Enumwert - der
        // Aufruf damit las ein Element HINTER dem Ende, hinter dem letzten
        // Member der Klasse.  Der Selbstaudit-Fix `48fcd9c` hat den Schreib-
        // ueberlauf geschlossen und diesen Lese-Ueberlauf erst aufgemacht.
        {
            FeatureEngine e6;
            e6.vorbereiten (48000.0);
            Speiser s6 { e6 };
            s6.bisAllesOffen();
            s6.projekt += 480000;
            s6.senden (s6.bauen());               // eine echte Grenze
            pruefe (e6.grenzenMitGrund (Grenzgrund::zeitSprung) == 1,
                    "L6: ein echter Grund wird gezaehlt",
                    juce::String ((int) e6.grenzenMitGrund (Grenzgrund::zeitSprung)));
            pruefe (e6.grenzenMitGrund (Grenzgrund::anzahl) == 0,
                    "L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen",
                    "gelesen: " + juce::String ((juce::int64) e6.grenzenMitGrund (Grenzgrund::anzahl)));
            pruefe (e6.grenzenMitGrund (Grenzgrund::keine) == 0,
                    "L6: und `keine` ebenso - getrennt wird nie ohne Grund");
        }
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
