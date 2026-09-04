/*  EqCopSonde013DynamicsTest — Loudnessfenster, Headroom und Dynamik
    (SONDE-013 M-01, M-03, M-04, M-07, M-09, M-75, M-77).

    §39.1 zaehlt die normgerechte Basis abschliessend auf: Momentary (400 ms),
    Short-term (3 s), Integrated, BS.1770-konformer True Peak, LRA erst nach
    rund 60 s geeignetem Material, PLR und PSR, Crest in MEHREREN Fenstern.
    Vor diesem Ticket trug der Frame davon drei Dinge, und eines davon war
    die schwaechere Zahl unter dem richtigen Namen.

    ── WAS DIESES BEIN MISST, UND WAS NICHT ─────────────────────────────────

    Es misst NICHT, dass die Felder vorhanden sind. Ein Bein, das nur
    Anwesenheit prueft, ist gruen, sobald irgendein Erzeuger irgendetwas
    hineinschreibt. Jeder Fall hier stellt statt dessen zwei Signale
    gegenueber, die sich in EINER Eigenschaft unterscheiden, und misst, dass
    genau das betroffene Fenster mitgeht und die anderen nicht:

    - Momentary gegen Short-term: ein Pegelsprung bewegt das 400-ms-Fenster
      sofort und das 3-s-Fenster erst spaeter. Waeren beide dieselbe Zahl mit
      zwei Namen, faellt dieser Fall.
    - Crest im Rahmen gegen Crest ueber 3 s: dichte kleine Spitzen und eine
      einzelne grosse sehen im 100-ms-Rahmen gleich aus und ueber drei
      Sekunden verschieden.
    - Headroom als Verteilung gegen Headroom als Spitze: ein Signal mit einer
      einzigen lauten Stelle hebt P95 und laesst P50 stehen.
    - LRA: es entsteht ueberhaupt erst nach der 60-s-Schwelle, und der Weg
      dorthin zaehlt GEGATETE Werte, nicht Wanduhr.

    Die zweite Haelfte ist fail-closed. Kein Fenster liefert eine Zahl,
    solange es nicht steht; keine Zahl ueberlebt eine Transportgrenze; und
    ein Ueberlaufblock verwirft die ANALYSE, nie das Audio.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/FeatureEngine.h"

#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <vector>

namespace rt = nakama::echtzeit;
using nakama::analyse::FeatureEngine;
using nakama::analyse::FeatureFrame;

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

/** Speist Bloecke und fuehrt Strom und Projektzeit selbst.  Wortgleich mit
    dem Speiser in `Sonde013EventWireTest.cpp` — ein Test, der die
    Buchfuehrung dem Pruefling ueberliesse, pruefte gegen einen selbstgebauten
    Zeitfehler. */
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

    bool sende (const std::function<float (std::uint64_t)>& f)
    {
        const auto b = bauen();
        audio.resize ((std::size_t) b.sampleCount * 2u);
        for (std::uint32_t i = 0; i < b.sampleCount; ++i)
        {
            const float v = f (strom + i);
            audio[(std::size_t) i * 2u]      = v;
            audio[(std::size_t) i * 2u + 1u] = v;
        }
        const bool r = engine.nimmBlock (b, audio.data());
        strom   += b.sampleCount;
        projekt += (std::int64_t) b.sampleCount;
        return r;
    }

    /** Faehrt `bloecke` Bloecke und gibt den zuletzt entstandenen Frame
        zurueck; `gab` sagt, ob ueberhaupt einer faellig wurde. */
    FeatureFrame fahre (const std::function<float (std::uint64_t)>& f,
                        int bloecke, bool* gab = nullptr)
    {
        bool irgendeiner = false;
        FeatureFrame letzter {};
        for (int i = 0; i < bloecke; ++i)
            if (sende (f))
            {
                irgendeiner = true;
                letzter = engine.frame();
            }
        if (gab != nullptr) *gab = irgendeiner;
        return letzter;
    }
};

std::function<float (std::uint64_t)> sinus (double amplitude, double hz, double sr)
{
    return [amplitude, hz, sr] (std::uint64_t n)
    { return (float) (amplitude * std::sin (kZweiPi * hz * (double) n / sr)); };
}

/** Wie viele Bloecke a `frames` Samples eine Dauer in Sekunden ergibt. */
int bloeckeFuer (double sekunden, double sr, int frames)
{ return (int) std::ceil (sekunden * sr / (double) frames); }
} // namespace

int main()
{
    constexpr double fs = 48000.0;
    std::cout << "== Nakama SONDE-013 - Loudnessfenster, Headroom, Dynamik ==" << std::endl;

    // ── M-01: drei Fenster, nicht ein Fenster mit drei Namen ──────────────
    //
    // Der Fall faehrt erst zwoelf Sekunden leise, dann laut. Kurz nach dem
    // Sprung MUSS Momentary (400 ms) schon oben sein und Short-term (3 s)
    // noch nicht — genau das ist der Unterschied zwischen den beiden, und
    // eine Umsetzung, die den einen aus dem anderen ableitet, kann ihn nicht
    // herstellen.
    abschnitt ("M-01  momentary_short_integrated_are_separate_windows");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };

        const auto leise = sinus (0.02, 1000.0, fs);
        const auto laut  = sinus (0.50, 1000.0, fs);

        const auto vorher = s.fahre (leise, bloeckeFuer (12.0, fs, s.frames));
        pruefe (vorher.lufsMGesetzt && vorher.lufsSGesetzt,
                "nach zwoelf Sekunden stehen beide Fenster",
                "M " + juce::String (vorher.lufsM, 2) + " LUFS, S "
                + juce::String (vorher.lufsS, 2) + " LUFS");
        pruefe (std::abs (vorher.lufsM - vorher.lufsS) < 0.5,
                "bei stehendem Pegel sind sie fast gleich - der Unterschied ist der "
                "VERLAUF, nicht die Rechnung",
                "Abstand " + juce::String (std::abs (vorher.lufsM - vorher.lufsS), 3) + " LU");

        // 0,8 s laut: mehr als zwei Momentary-Fenster, weniger als ein
        // Drittel des Short-term-Fensters.
        const auto nachSprung = s.fahre (laut, bloeckeFuer (0.8, fs, s.frames));
        pruefe (nachSprung.lufsMGesetzt && nachSprung.lufsSGesetzt,
                "0,8 s nach dem Sprung tragen beide weiterhin einen Wert");
        pruefe (nachSprung.lufsM - nachSprung.lufsS > 5.0,
                "Momentary ist dem Sprung gefolgt, Short-term noch nicht - die zwei "
                "Fenster sind wirklich verschieden lang",
                "M " + juce::String (nachSprung.lufsM, 2) + " gegen S "
                + juce::String (nachSprung.lufsS, 2) + " LUFS, Abstand "
                + juce::String (nachSprung.lufsM - nachSprung.lufsS, 2) + " LU");

        // Und nach genug Zeit treffen sie sich wieder. Ohne diese Gegenprobe
        // koennte der Fall oben auch von einem konstanten Versatz kommen.
        const auto spaeter = s.fahre (laut, bloeckeFuer (5.0, fs, s.frames));
        pruefe (std::abs (spaeter.lufsM - spaeter.lufsS) < 0.5,
                "fuenf Sekunden spaeter sind sie wieder beieinander - der Abstand oben "
                "war der Sprung, kein Versatz",
                "Abstand " + juce::String (std::abs (spaeter.lufsM - spaeter.lufsS), 3) + " LU");
    }

    // ── M-01: Momentary steht frueher als Short-term ──────────────────────
    abschnitt ("M-01  jedes Fenster braucht seine eigene Fuellung");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto ton = sinus (0.3, 1000.0, fs);

        // 0,6 s: mehr als 400 ms, weit unter 3 s.
        const auto frueh = s.fahre (ton, bloeckeFuer (0.6, fs, s.frames));
        pruefe (frueh.lufsMGesetzt && ! frueh.lufsSGesetzt,
                "nach 0,6 s traegt der Frame Momentary, aber KEIN Short-term - eine "
                "Kurzzeitlautheit ueber 0,6 s waere eine andere Zahl mit demselben Namen",
                juce::String ("M ") + (frueh.lufsMGesetzt ? "ja" : "nein")
                + ", S " + (frueh.lufsSGesetzt ? "ja" : "nein"));

        const auto spaet = s.fahre (ton, bloeckeFuer (3.0, fs, s.frames));
        pruefe (spaet.lufsSGesetzt,
                "nach weiteren drei Sekunden steht auch Short-term");
    }

    // ── M-09: integration_samples, der Kern von NAK-68 ────────────────────
    abschnitt ("M-09  thin_frame_after_boundary_is_recognisable");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto ton = sinus (0.3, 1000.0, fs);

        const auto voll = s.fahre (ton, 60);
        pruefe (voll.integrationGesetzt && voll.integrationSamples > 0,
                "ein Rahmen im Dauerbetrieb sagt, ueber wie viel Audio er gerechnet wurde",
                juce::String ((int) voll.integrationSamples) + " Samples je Kanal");
        pruefe (voll.integrationSamples >= 4800 && voll.integrationSamples <= 4800 + 512,
                "und die Zahl passt zur Livekadenz von 10 Hz plus hoechstens einem Block",
                juce::String ((int) voll.integrationSamples) + " gegen 4800..5312");

        // Eine Grenze MITTEN im Rahmen ziehen und den ersten Frame danach
        // nehmen: er traegt weniger integriertes Audio. Genau diesen
        // Unterschied konnte ein Empfaenger vor NAK-68 nicht sehen.
        //
        // ⚠️ Die Stelle ist tragend. `liveSamples` ueberlebt eine Grenze
        // bewusst ("die Grenze schneidet den Inhalt, nicht die Uhr", SONDE-009
        // §10.1), also bleibt die KADENZ stehen, waehrend `rahmenSamples`
        // genullt wird. Liegt die Grenze unmittelbar NACH einem Frame, faellt
        // der naechste wieder nach vollen 4800 Samples und ist gar nicht
        // duenn — der Fall haette dann nichts gemessen. Er stellt den Zustand
        // deshalb her, statt ihn auszurechnen: erst bis zum naechsten Frame
        // fahren, dann ein Stueck weiter, dann die Luecke.
        while (! s.sende (ton)) {}
        for (int i = 0; i < 5; ++i)
            (void) s.sende (ton);
        s.strom += 4096;                       // Luecke im Strom = Grenze
        FeatureFrame duenn {};
        bool gab = false;
        for (int i = 0; i < 40 && ! gab; ++i)
            if (s.sende (ton)) { duenn = e.frame(); gab = true; }
        pruefe (gab && duenn.integrationGesetzt,
                "auch der erste Rahmen nach einer Grenze traegt das Feld");
        pruefe (gab && duenn.integrationSamples < voll.integrationSamples,
                "und er sagt ehrlich, dass er ueber WENIGER Audio gerechnet wurde",
                juce::String ((int) duenn.integrationSamples) + " gegen "
                + juce::String ((int) voll.integrationSamples) + " Samples");
    }

    // ── M-03: PSR rechnet gegen das 3-s-True-Peak-Maximum ─────────────────
    abschnitt ("M-03  plr_psr_definitions_and_labels");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto ton = sinus (0.5, 1000.0, fs);
        const auto f = s.fahre (ton, bloeckeFuer (6.0, fs, s.frames));

        pruefe (f.truePeakGesetzt && f.truePeakPassageGesetzt,
                "der Frame traegt True Peak des Rahmens UND der Passage",
                "Rahmen " + juce::String (f.truePeakDb, 3) + " dBTP, Passage "
                + juce::String (f.truePeakPassageDb, 3) + " dBTP");
        pruefe (f.truePeakDb >= f.peakDb - 0.01f,
                "der True Peak liegt nie unter dem Sample-Peak desselben Rahmens",
                "TP " + juce::String (f.truePeakDb, 3) + " gegen SP "
                + juce::String (f.peakDb, 3) + " dB");
        pruefe (f.truePeakPassageDb >= f.truePeakDb - 0.01f,
                "und das Passagenmaximum liegt nie unter dem Rahmenmaximum");

        pruefe (f.psrGesetzt, "PSR steht, sobald das 3-s-Fenster steht");

        // ⚠️ Der eigentliche Fall. Bei einem stationaeren Sinus sind
        // Rahmen-Sample-Peak, Rahmen-True-Peak und 3-s-True-Peak-Maximum fast
        // dieselbe Zahl — dort kann kein Test die drei auseinanderhalten. Der
        // Rotbeweis `SONDE-013-rot-M-03-psr.txt` hat genau das gezeigt: die
        // erste Fassung dieses Falls blieb gruen, als PSR wieder gegen den
        // Sample-Peak des 100-ms-Rahmens rechnete.
        //
        // Das Signal hier trennt sie: ein lauter Impuls, danach zwei Sekunden
        // leise. Der AKTUELLE Rahmen ist leise (sein Peak liegt tief), das
        // 3-s-Fenster enthaelt den Impuls noch. Wer gegen den Rahmen rechnet,
        // bekommt eine um mehr als 20 dB zu kleine Zahl.
        FeatureEngine e2;
        e2.vorbereiten (fs);
        Speiser s2 { e2 };
        const auto leiseTon = sinus (0.02, 1000.0, fs);
        s2.fahre (leiseTon, bloeckeFuer (4.0, fs, s2.frames));
        s2.fahre (sinus (0.9, 1000.0, fs), bloeckeFuer (0.15, fs, s2.frames));
        const auto nachImpuls = s2.fahre (leiseTon, bloeckeFuer (2.0, fs, s2.frames));

        pruefe (nachImpuls.psrGesetzt && nachImpuls.truePeakGesetzt,
                "im Rahmen NACH dem Impuls stehen PSR und der Rahmen-True-Peak");
        pruefe (nachImpuls.truePeakDb < -25.0f,
                "der aktuelle Rahmen ist leise - sein True Peak liegt tief",
                juce::String (nachImpuls.truePeakDb, 2) + " dBTP");
        const double gegenRahmen = (double) nachImpuls.truePeakDb - (double) nachImpuls.lufsS;
        pruefe ((double) nachImpuls.psrDb - gegenRahmen > 20.0,
                "PSR rechnet trotzdem gegen das True-Peak-Maximum DESSELBEN 3-s-Fensters "
                "- der Impuls ist darin noch enthalten",
                "PSR " + juce::String (nachImpuls.psrDb, 2) + " dB; gegen den Rahmen "
                "waeren es " + juce::String (gegenRahmen, 2) + " dB");
        pruefe (std::abs ((double) nachImpuls.psrDb
                          - (-0.92 - (double) nachImpuls.lufsS)) < 1.0,
                "und die Zahl passt zum Impulspegel: 20*log10(0,9) minus LUFS-S",
                "gemessen " + juce::String (nachImpuls.psrDb, 2) + " dB, erwartet "
                + juce::String (-0.92 - (double) nachImpuls.lufsS, 2) + " dB");

        // PLR entsteht erst im Sondenprozessor (er haelt LUFS-I). Die Engine
        // liefert nur die Zutat - und behauptet deshalb kein PLR.
        pruefe (! f.plrGesetzt,
                "die Engine selbst setzt KEIN PLR - LUFS-I kommt aus dem "
                "Loudness-Akkumulator und wird erst im Sondenprozessor zugemischt");
    }

    // ── M-02: der Peak am Rahmenende (Selbstauditfund dieser Etappe) ─────
    //
    // Der Interpolatorkern ist um seine halbe Laenge zentriert und sieht ein
    // Sample deshalb erst zwoelf Samples spaeter. Liegt der EINZIGE
    // Spitzenwert eines Rahmens in dessen letzten Samples, haette der reine
    // Interpolatorwert ihn noch nicht — der Frame truege einen True Peak
    // UNTER seinem eigenen Sample-Peak, und beide Leser lehnten ihn als
    // `true_peak_unter_sample_peak` ab.
    //
    // ⚠️ Zwei Fallen liegen in diesem Fall, und beide sind in der ersten
    // Fassung zugeschlagen (der Rotbeweis kam zweimal gruen zurueck):
    //
    // 1. Der Interpolatorverlauf ueberlebt Blockgrenzen. Ein Impuls in JEDEM
    //    Block wird im naechsten gesehen — also traegt der Rahmen den Peak
    //    trotzdem, nur von einem frueheren Impuls. Es muss GENAU EIN Impuls
    //    im Rahmen sein, und der ganz am Ende.
    // 2. Wo der Rahmen endet, haengt an der Kadenzarithmetik. Der Fall
    //    RECHNET sie nicht aus, sondern MISST sie zuerst — eine Wette auf
    //    4800 gegen 5120 Samples waere genau die Sorte Annahme, an der die
    //    Zwillingsprobe in §10.2 schon einmal vorbeigelaufen ist.
    abschnitt ("M-02  impuls_am_rahmenende");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto leise = sinus (0.02, 1000.0, fs);

        // Schritt 1: messen, nach wie vielen Bloecken ein Rahmen faellt.
        s.fahre (leise, 30);                    // einschwingen
        int bisFrame = 1;
        while (! s.sende (leise))
            ++bisFrame;

        // Schritt 2: den Rahmen leise fuellen und NUR im letzten Block eine
        // Spitze in die letzten vier Samples legen.
        const int blockLaenge = s.frames;
        const auto mitSpitzeAmEnde = [blockLaenge] (std::uint64_t n)
        {
            const std::uint64_t imBlock = n % (std::uint64_t) blockLaenge;
            if (imBlock >= (std::uint64_t) (blockLaenge - 4))
                return 0.95f;
            return (float) (0.02 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
        };

        int gesehen = 0, verletzt = 0;
        float schlimmsteLuecke = 0.0f;
        for (int runde = 0; runde < 8; ++runde)
        {
            for (int i = 0; i < bisFrame - 1; ++i)
                (void) s.sende (leise);
            if (! s.sende (mitSpitzeAmEnde))
            {
                // Die Kadenz ist verrutscht (etwa durch einen Rundungsrest):
                // bis zum naechsten Frame weiterfahren und die Runde
                // verwerfen, statt einen halben Rahmen zu messen.
                while (! s.sende (leise)) {}
                continue;
            }
            const auto& f = e.frame();
            if (! (f.truePeakGesetzt && f.peakGesetzt) || f.peakDb < -3.0f)
                continue;                       // die Spitze lag nicht in diesem Rahmen
            ++gesehen;
            if (f.truePeakDb < f.peakDb)
            {
                ++verletzt;
                schlimmsteLuecke = std::max (schlimmsteLuecke, f.peakDb - f.truePeakDb);
            }
        }

        pruefe (gesehen > 0,
                "der Zustand ist wirklich hergestellt: Rahmen, deren einzige Spitze in "
                "den letzten vier Samples liegt",
                juce::String (gesehen) + " von 8 Runden, Rahmen alle "
                + juce::String (bisFrame) + " Bloecke");
        pruefe (verletzt == 0,
                "in KEINEM davon liegt der True Peak unter dem Sample-Peak - der "
                "Abtastpunkt geht in den True Peak ein, weil er zur rekonstruierten "
                "Wellenform gehoert",
                verletzt == 0 ? juce::String ("0 Verletzungen")
                              : juce::String (verletzt) + " Verletzungen, groesste Luecke "
                                + juce::String (schlimmsteLuecke, 3) + " dB");
    }

    // ── M-03: Headroom ist eine Verteilung, keine Spitze ──────────────────
    abschnitt ("M-03  headroom_is_distribution_not_single_peak");
    {
        // Zwei Signale mit DEMSELBEN Maximum: eines durchgehend laut, eines
        // ueberwiegend leise mit einer einzelnen lauten Stelle. Ein
        // Einzelwert kann sie nicht unterscheiden, drei Perzentile schon.
        FeatureEngine eDurch, eSpitze;
        eDurch.vorbereiten (fs);
        eSpitze.vorbereiten (fs);
        Speiser sD { eDurch }, sS { eSpitze };

        const auto laut  = sinus (0.5,  1000.0, fs);
        const auto leise = sinus (0.02, 1000.0, fs);

        const auto durch = sD.fahre (laut, bloeckeFuer (5.0, fs, sD.frames));
        // Bei der Spitzenvariante: lange leise, kurz laut, wieder leise.
        sS.fahre (leise, bloeckeFuer (3.0, fs, sS.frames));
        sS.fahre (laut,  bloeckeFuer (0.3, fs, sS.frames));
        const auto spitze = sS.fahre (leise, bloeckeFuer (1.7, fs, sS.frames));

        pruefe (durch.headroomGesetzt && spitze.headroomGesetzt,
                "beide tragen eine Headroomverteilung",
                "Fenster " + juce::String ((int) durch.headroomFenster) + " / "
                + juce::String ((int) spitze.headroomFenster) + " Rahmen");
        pruefe (durch.headroomP10Db <= durch.headroomP50Db
                  && durch.headroomP50Db <= durch.headroomP95Db,
                "die drei Punkte sind geordnet - P10 <= P50 <= P95");
        pruefe (std::abs (durch.headroomP95Db - spitze.headroomP95Db) < 1.0,
                "beide haben nahezu dasselbe P95 - die laute Stelle ist in beiden drin",
                "durchgehend " + juce::String (durch.headroomP95Db, 2) + " dBTP, "
                "Spitze " + juce::String (spitze.headroomP95Db, 2) + " dBTP");
        pruefe (durch.headroomP50Db - spitze.headroomP50Db > 10.0,
                "aber ihr P50 liegt weit auseinander - GENAU das kann ein einzelner "
                "Maximalwert nicht ausdruecken",
                "durchgehend " + juce::String (durch.headroomP50Db, 2) + " dBTP, "
                "Spitze " + juce::String (spitze.headroomP50Db, 2) + " dBTP");
        pruefe (spitze.headroomP95Db - spitze.headroomP10Db > 10.0,
                "und die Spanne der Spitzenvariante ist gross - der Headroom SCHWANKT",
                "Spanne " + juce::String (spitze.headroomP95Db - spitze.headroomP10Db, 2)
                + " dB");
    }

    // ── M-04: Crest in mehreren Fenstern ──────────────────────────────────
    abschnitt ("M-04  crest_multiple_windows");
    {
        // Ein Signal, das im 100-ms-Rahmen ruhig aussieht und ueber drei
        // Sekunden nicht: leiser Dauerton mit einem kurzen lauten Impuls
        // etwa alle zwei Sekunden.
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const double impulsPeriode = 2.0 * fs;
        const auto stossig = [impulsPeriode] (std::uint64_t n)
        {
            const double phase = std::fmod ((double) n, impulsPeriode);
            const double grund = 0.03 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0);
            const double impuls = (phase < 64.0)
                ? 0.7 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0) : 0.0;
            return (float) (grund + impuls);
        };

        // Ein Frame, der GERADE KEINEN Impuls enthaelt: sein Rahmencrest ist
        // klein, sein 3-s-Crest gross.
        FeatureFrame ruhig {};
        bool gefunden = false;
        for (int i = 0; i < 600 && ! gefunden; ++i)
            if (s.sende (stossig))
            {
                const auto& f = e.frame();
                if (f.crestGesetzt && f.crestKurzGesetzt && f.crestDb < 6.0f)
                {
                    ruhig = f;
                    gefunden = true;
                }
            }
        pruefe (gefunden,
                "es gibt Rahmen ohne Impuls, in denen BEIDE Crestfenster stehen");
        pruefe (gefunden && ruhig.crestKurzDb - ruhig.crestDb > 10.0f,
                "und dort liegt der 3-s-Crest weit ueber dem des 100-ms-Rahmens - das "
                "ist der Unterschied, den ein einzelnes Fenster nicht sehen kann",
                "Rahmen " + juce::String (ruhig.crestDb, 2) + " dB, 3 s "
                + juce::String (ruhig.crestKurzDb, 2) + " dB");
    }

    // ── M-04: LRA wird nicht vorgetaeuscht ────────────────────────────────
    abschnitt ("M-04  lra_below_sixty_seconds_is_not_a_number");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        // Ein Signal mit echter Dynamik: der Pegel wandert langsam ueber
        // rund 20 LU. Ohne Dynamik waere LRA nahe null, und ein Test, der
        // nur "> 0" fordert, saehe keinen Unterschied zwischen "gerechnet"
        // und "vergessen".
        const auto wandernd = [] (std::uint64_t n)
        {
            const double t = (double) n / 48000.0;
            const double huellkurve = 0.30 * std::pow (10.0,
                (-10.0 + 10.0 * std::sin (kZweiPi * t / 25.0)) / 20.0);
            return (float) (huellkurve * std::sin (kZweiPi * 1000.0 * t));
        };

        const auto bei30 = s.fahre (wandernd, bloeckeFuer (30.0, fs, s.frames));
        pruefe (! bei30.lraGesetzt,
                "nach 30 s gibt es KEINEN LRA-Wert - `nicht belastbar` heisst hier "
                "Abwesenheit, nicht eine Null");

        const auto bei75 = s.fahre (wandernd, bloeckeFuer (45.0, fs, s.frames));
        pruefe (bei75.lraGesetzt,
                "nach insgesamt 75 s steht er");
        pruefe (bei75.lraGesetzt && bei75.lraLu > 0.5f && bei75.lraLu < 40.0f,
                "und traegt eine Dynamikspanne, die zum wandernden Pegel passt",
                juce::String (bei75.lraLu, 2) + " LU");

        // Gegenprobe: ein Signal OHNE Dynamik ergibt eine kleine Spanne. Ohne
        // sie waere oben nur gezeigt, dass irgendeine Zahl entsteht.
        FeatureEngine flach;
        flach.vorbereiten (fs);
        Speiser sf { flach };
        const auto stetig = sinus (0.2, 1000.0, fs);
        const auto lang = sf.fahre (stetig, bloeckeFuer (75.0, fs, sf.frames));
        pruefe (lang.lraGesetzt && lang.lraLu < 1.0f,
                "ein Signal ohne Dynamik ergibt eine LRA nahe null - der Wert misst "
                "wirklich die Spanne und nicht die Laufzeit",
                juce::String (lang.lraLu, 3) + " LU");
    }

    // ── M-04: die 60-s-Regel zaehlt GEEIGNETES Material ───────────────────
    abschnitt ("M-04  die 60-s-Schwelle zaehlt gegatetes Material, keine Wanduhr");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto stille = [] (std::uint64_t) { return 0.0f; };
        const auto lang = s.fahre (stille, bloeckeFuer (90.0, fs, s.frames));
        pruefe (! lang.lraGesetzt,
                "90 s Stille ergeben KEIN LRA - eine Minute Stille ist keine Minute "
                "Material (§39.1: 'rund 60 s GEEIGNETEN Materials')");
    }

    // ── M-07: nichts Nichtendliches erreicht eine neue Metrik ─────────────
    abschnitt ("M-07  non_finite_never_reaches_wire_for_new_metrics");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto giftig = [] (std::uint64_t n)
        {
            if (n % 997u == 0u) return std::numeric_limits<float>::quiet_NaN();
            if (n % 991u == 0u) return std::numeric_limits<float>::infinity();
            return (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0));
        };
        const auto f = s.fahre (giftig, bloeckeFuer (8.0, fs, s.frames));

        const struct { const char* name; bool bit; float wert; } neue[] = {
            { "lufsM", f.lufsMGesetzt, f.lufsM },
            { "truePeakDb", f.truePeakGesetzt, f.truePeakDb },
            { "truePeakPassageDb", f.truePeakPassageGesetzt, f.truePeakPassageDb },
            { "psrDb", f.psrGesetzt, f.psrDb },
            { "crestKurzDb", f.crestKurzGesetzt, f.crestKurzDb },
            { "lraLu", f.lraGesetzt, f.lraLu },
            { "headroomP10Db", f.headroomGesetzt, f.headroomP10Db },
            { "headroomP50Db", f.headroomGesetzt, f.headroomP50Db },
            { "headroomP95Db", f.headroomGesetzt, f.headroomP95Db },
        };
        bool allesEndlich = true;
        juce::String schuldig;
        for (const auto& n : neue)
            if (n.bit && ! std::isfinite (n.wert))
            {
                allesEndlich = false;
                schuldig = n.name;
            }
        pruefe (allesEndlich,
                "NaN und Inf im Eingang erzeugen in KEINER neuen Metrik einen "
                "nichtendlichen Wert - der Riegel liegt beim Erzeugen, nicht auf der "
                "Leitung",
                allesEndlich ? juce::String ("alle 9 Traeger endlich") : schuldig);
        // Nacharbeit 1 (2026-09-04, Befund B07): die alte Zusage an dieser
        // Stelle lautete "und die Metriken entstehen ueberhaupt". Genau die war
        // der Fehler. M-07 verlangt beim Erzeugen "Wert 0 mit `gueltig=false`"
        // UND einen Zaehler; ein Rahmen, der ueber stillgelegte Samples
        // rechnet und seine Skalare trotzdem als gesetzt meldet, sieht aus wie
        // eine saubere Messung. Der Riegel ist deshalb ein LATCH DES RAHMENS,
        // kein Dauerschweigen: der naechste saubere Rahmen traegt wieder alles.
        pruefe (f.nichtEndlichRahmen > 0,
                "der Rahmen ZAEHLT seine nicht-endlichen Eingangssamples",
                juce::String ((int) f.nichtEndlichRahmen));
        pruefe (! f.truePeakGesetzt && ! f.lufsMGesetzt && ! f.crestKurzGesetzt,
                "und traegt seine sampleabhaengigen Skalare NICHT - 'nicht gemessen' "
                "statt 'gemessen und sauber' (M-07)");

        // Die Gegenprobe: derselbe Aufbau mit sauberem Material traegt sie.
        FeatureEngine rein;
        rein.vorbereiten (fs);
        Speiser s2 { rein };
        const auto sauber = s2.fahre (
            [] (std::uint64_t n)
            { return (float) (0.4 * std::sin (kZweiPi * 1000.0 * (double) n / 48000.0)); },
            bloeckeFuer (8.0, fs, s2.frames));
        pruefe (sauber.nichtEndlichRahmen == 0 && sauber.truePeakGesetzt,
                "ohne NaN traegt derselbe Aufbau seine Skalare - der Riegel ist ein "
                "Latch des Rahmens, kein Dauerschweigen");
    }

    // ── M-75 / M-77: der verworfene Block schliesst auch die neuen Fenster ─
    //
    // ⚠️ Was hier NICHT steht, und warum. M-77 beschreibt einen Hostblock
    // ueber der vorallokierten SLOTKAPAZITAET. Die Kapazitaet gehoert der
    // `StampedAudioQueue`, nicht dieser Engine — die Engine bekommt nur
    // ganze, bereits angenommene Bloecke, und ihr einen Block mit
    // `sampleCount` jenseits des uebergebenen Puffers zu reichen, waere
    // schlicht ein Lesefehler im Test, kein Vertragsfall (gemessen: er
    // stuerzt, und zwar zu Recht). Der Oversizepfad selbst ist B4.
    //
    // Was die Engine SIEHT, ist die Folge: der Block fehlt, im Strom klafft
    // eine Luecke, und daraus wird ein neues `continuity_segment`. Genau das
    // misst dieser Abschnitt — und zwar fuer die Fenster, die es beim
    // Schreiben von §48.1 noch nicht gab.
    abschnitt ("M-75/M-77  der verworfene Analyseblock schliesst auch die neuen Fenster");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto ton = sinus (0.4, 1000.0, fs);
        s.fahre (ton, bloeckeFuer (6.0, fs, s.frames));

        const auto vorher = e.frame();
        pruefe (vorher.truePeakGesetzt && vorher.crestKurzGesetzt && vorher.psrGesetzt,
                "vor dem verworfenen Block stehen die neuen Fenster");

        const auto segmenteVorher = e.segmentwechsel();
        s.strom += 65536;                      // ein verworfener Block: Stromluecke
        // ⚠️ NACH der Luecke laeuft ein DEUTLICH leiseres Signal. Mit
        // demselben Ton wie davor koennte dieser Abschnitt gar nichts zeigen:
        // das Passagenmaximum waere danach wieder derselbe Wert, ob es die
        // Grenze nun ueberbrueckt hat oder neu entstanden ist. Genau die
        // Sorte Gegenprobe, die nichts misst.
        const auto leise = sinus (0.004, 1000.0, fs);
        FeatureFrame danach {};
        bool gab = false;
        for (int i = 0; i < 30 && ! gab; ++i)
            if (s.sende (leise)) { danach = e.frame(); gab = true; }

        pruefe (e.segmentwechsel() > segmenteVorher,
                "die Luecke zieht eine Segmentgrenze - der Verlust ist sichtbar, nicht "
                "interpoliert",
                "Segmentwechsel " + juce::String ((int) e.segmentwechsel()));
        pruefe (gab && ! danach.crestKurzGesetzt && ! danach.psrGesetzt,
                "danach tragen die 3-s-Fenster KEINEN Wert mehr - sie sind geschlossen, "
                "nicht fortgeschrieben");
        pruefe (gab && danach.truePeakPassageDb < vorher.truePeakPassageDb - 20.0f,
                "und das Passagenmaximum hat die Grenze nicht ueberbrueckt - es ist das "
                "der neuen, leisen Passage",
                "vorher " + juce::String (vorher.truePeakPassageDb, 2) + " dBTP, danach "
                + juce::String (danach.truePeakPassageDb, 2) + " dBTP");
    }

    // ── Grenzen: kein neues Fenster ueberbrueckt eine Transportgrenze ─────
    //
    // G13 in B5 misst dasselbe strukturell ueber ALLE Felder. Hier steht die
    // Zusage noch einmal namentlich fuer die neuen Traeger, damit ein
    // Regressionsfall lokalisierbar ist statt nur "irgendein Feld".
    abschnitt ("M-84  jede Grenze trennt auch die neuen Fenster, namentlich");
    {
        FeatureEngine e;
        e.vorbereiten (fs);
        Speiser s { e };
        const auto laut = sinus (0.6, 1000.0, fs);
        const auto leise = sinus (0.01, 1000.0, fs);

        const auto vorher = s.fahre (laut, bloeckeFuer (6.0, fs, s.frames));
        pruefe (vorher.truePeakPassageGesetzt && vorher.headroomGesetzt,
                "vor der Grenze stehen Passagenmaximum und Headroomverteilung",
                "Passage " + juce::String (vorher.truePeakPassageDb, 2) + " dBTP");

        s.strom += 8192;                       // Stromluecke = Grenze
        const auto nachher = s.fahre (leise, bloeckeFuer (2.0, fs, s.frames));
        pruefe (nachher.truePeakPassageGesetzt
                  && nachher.truePeakPassageDb < vorher.truePeakPassageDb - 20.0f,
                "nach der Grenze ist das Passagenmaximum das der NEUEN Passage - der "
                "laute Peak von vorher ist weg",
                "vorher " + juce::String (vorher.truePeakPassageDb, 2) + " dBTP, nachher "
                + juce::String (nachher.truePeakPassageDb, 2) + " dBTP");
        pruefe (! nachher.headroomGesetzt
                  || nachher.headroomP95Db < vorher.headroomP95Db - 20.0f,
                "und die Headroomverteilung ist entweder leer oder ganz aus dem neuen "
                "Material - sie mischt die zwei Passagen nicht");
        pruefe (! nachher.crestKurzGesetzt && ! nachher.psrGesetzt,
                "die 3-s-Fenster sind nach zwei Sekunden noch nicht wieder voll - sie "
                "wurden wirklich geleert, nicht fortgeschrieben");
    }

    std::cout << "\n-----------------------------------------" << std::endl;
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
