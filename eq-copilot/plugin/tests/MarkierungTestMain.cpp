// Headless-Beweise der Hör-Markierung (Konzept v2 §8, T2–T10).
// T1 (Markierung aus ⇒ bitgleich) beweist weiterhin der bestehende Nulltest.
// Exit 0 nur bei "MARKIERUNGSTEST OK".
//
// Zwei Beweisebenen:
//  · DSP-Verhalten isoliert: testForciereEchtzeit() umgeht das Lebenszeichen
//    (und die Editor-Pflicht), damit Filter/Fades ohne Wanduhr-Taktung
//    deterministisch prüfbar sind.
//  · Das Lebenszeichen selbst (T3/T10): ECHTE Taktung — Freilauf-Blöcke
//    bleiben bitgleich (Render-Beweis), wanduhr-getaktete Blöcke schalten
//    nach ~1 s frei, Transport-Stopp und setNonRealtime schneiden hart.
#include "PluginProcessor.h"
#include "HoerMarkierung.h"

#include <pluginterfaces/vst/ivstprocesscontext.h>
#include <chrono>
#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>

using namespace eqcop;
using VstKontext = Steinberg::Vst::ProcessContext;

static juce::uint32 lcg = 0x2545f491u;
static float zufall()
{
    lcg = lcg * 1664525u + 1013904223u;
    return ((float) (lcg >> 8) / (float) 0x00ffffff) * 2.0f - 1.0f;
}

struct TestPlayHead : juce::AudioPlayHead
{
    bool spielt = false;
    juce::int64 pos = 0;
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo p;
        p.setIsPlaying (spielt);
        p.setTimeInSamples (pos);
        return p;
    }
};

// ⚠️ S10-11/SONDE-008 — WARUM ab hier JEDER Abschnitt einen Playhead braucht.
//
// Bis 23.08. lautete der Transport-Term der Verriegelung `(spielt ∨
// ¬hatTransport)`: ohne gemeldeten Transport faerbte die Markierung. Genau
// dieses fail-open hat der User am 22.08. abgewaehlt (Hub `U10`: „Nein, nur mit
// Signal"); der Code verlangt jetzt ein GUELTIGES „spielt". Diese Abschnitte
// liefen vorher ganz ohne Playhead und lebten damit vom fail-open - sie
// pruefen ab jetzt denselben Weg, den ein Host geht.
//
// Der Playhead ist NICHT durch `testForciereEchtzeit` zu ersetzen: der Schalter
// umgeht, was an der Wanduhr haengt (Lebenszeichen, Editor). Transport haengt an
// nichts dergleichen - waere er mit umgangen, pruefte der Test einen Pfad, den
// das Produkt nicht hat (dieselbe Begruendung wie beim §53.5-Term).
//
// `pos` wandert je Block mit: eine stehende Projektzeit ist zwar kein
// Kontinuitaetsbruch (FL zerteilt Puffer bis 1 Sample), aber ein laufender
// Transport bewegt sie, und der Test soll den Normalfall fahren.
// ⚠️ S20-22/SONDE-013 M-33 — WARUM ab hier JEDER Abschnitt die HOSTBRUECKE
// braucht und nicht mehr nur einen Playhead.
//
// §58 verlangt fuer den Hoermarker vier fail-closed-Terme: `playing=true`,
// `recording=false`, Realtime und Editor offen. Der dritte fehlte bis
// SONDE-013 im `erlaubt`-Term, obwohl der Prozessor ihn kennt.
//
// Der Aufnahmezustand kommt AUSSCHLIESSLICH ueber die Hostbruecke: JUCEs
// oeffentlicher Playhead-Rueckfallweg traegt ihn nicht (und soll ihn auch
// nicht tragen — B5 misst ausdruecklich, dass ueber ihn nur zwei
// Gueltigkeitsbits durchkommen). Ohne Bruecke ist der Zustand also
// UNBEKANNT, und ein unbekannter blockiert wie ein aktiver.
//
// Das ist keine Testkosmetik, sondern die Produktwirkung: in einem Host ohne
// gepatchten Wrapper faerbt die Markierung seit M-33 nicht mehr. Diese
// Klasse faehrt deshalb denselben Weg wie das Produkt.
struct LaufenderTransport
{
    TestPlayHead kopf;
    eqcop::hostbruecke::Bruecke bruecke;

    explicit LaufenderTransport (EqCopilotProcessor& p, bool nimmtAuf = false)
        : prozessor (p), aufnahme (nimmtAuf)
    {
        kopf.spielt = true;
        prozessor.setPlayHead (&kopf);
        bruecke.verbinde (&prozessor);
    }
    ~LaufenderTransport()
    {
        bruecke.verbinde (nullptr);
        prozessor.setPlayHead (nullptr);
    }

    /** Ein Blockvorlauf ueber die Bruecke, genau wie der gepatchte Wrapper:
        `beginneBlock` → `kontextAus` → `uebergib`, unmittelbar vor
        `processBlock`. */
    void vorBlock (int samples)
    {
        VstKontext c {};
        // ⚠️ Der Prozessor bevorzugt den Brueckenstand VOR dem oeffentlichen
        // Playhead. Der Transportzustand muss deshalb HIER stehen und nicht
        // nur im `kopf` — sonst zeigt ein Test, der den Playhead stoppt,
        // trotzdem einen laufenden Transport.
        c.state = VstKontext::kContTimeValid;
        if (kopf.spielt)
            c.state |= VstKontext::kPlaying;
        if (aufnahme)
            c.state |= VstKontext::kRecording;
        c.projectTimeSamples = kopf.pos;
        c.continousTimeSamples = kopf.pos;
        c.sampleRate = 48000.0;
        bruecke.beginneBlock ((std::uint32_t) samples);
        bruecke.kontextAus (c);
        bruecke.uebergib();
    }

    /** Wie `vorBlock`, aber OHNE Aufnahmebit im Kontext — der Fall
        "Aufnahmezustand unbekannt". Er blockiert wie eine laufende Aufnahme
        (M-33, fail-closed). */
    void vorBlockOhneKontext (int samples)
    {
        bruecke.beginneBlock ((std::uint32_t) samples);
        bruecke.kontextFehlt();
        bruecke.uebergib();
    }

    void weiter (int samples) { kopf.pos += samples; }

private:
    EqCopilotProcessor& prozessor;
    bool aufnahme;
};

// S9/SONDE-007b Abschnitt 3: Seit der Lifecycle-Klassifikation (§53.5) faerbt
// NUR ein positiv klassifiziertes Main - `unclassified` und `legacy` sind
// audio-neutral. Der Test geht dafuer denselben Weg wie der User: Editor auf,
// Rolle "hub" waehlen. Absichtlich NICHT ueber testForciereEchtzeit(): dieser
// Schalter umgeht das Lebenszeichen und die Editor-Pflicht, weil beide an der
// Wanduhr haengen. Die Klassifikation haengt an nichts dergleichen - waere sie
// mit umgangen, pruefte dieser Test einen Pfad, den das Produkt nicht hat.
static bool alsMainKlassifizieren (EqCopilotProcessor& p)
{
    p.setzeEditorOffen (true);
    p.setzeBindung ("hub", {}, {});
    return p.holeKlassifikation() == nakama::state::Klassifikation::main;
}

static bool blockBitgleich (const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b)
{
    for (int k = 0; k < a.getNumChannels(); ++k)
        if (std::memcmp (a.getReadPointer (k), b.getReadPointer (k),
                         (size_t) a.getNumSamples() * sizeof (float)) != 0)
            return false;
    return true;
}

//==============================================================================
// ⚠️ S20-22/SONDE-013 — WARUM DIESE DREI ABSCHNITTE FUNKTIONEN SIND UND KEINE
// BLOECKE IN `main`.
//
// MSVC reserviert den Stackframe einer Funktion beim Eintritt und addiert
// dabei die Locals ALLER Bloecke. `main` haelt hier schon ein Dutzend
// `EqCopilotProcessor` (jeder traegt FeatureEngine, AnalyseEngine und mehrere
// Ringe), und die drei neuen Abschnitte haben den 1-MiB-Stack gesprengt,
// BEVOR die erste Zeile Ausgabe kam — der Frame passte nicht mehr in den
// Prolog. Als eigene Funktionen bekommt jeder seinen Frame und gibt ihn
// wieder her.
//
// Derselbe Befund steht fuer `AnalysisGoldenTestMain.cpp` im Manifest §10.3
// als Nebenbefund; hier ist er einmal mehr aufgetreten und lokal geloest.

/** Traegt das `pruefe`-Lambda aus `main` in die drei Funktionen — mit dem
    DEFAULT fuer den Zusatz, den `std::function` allein nicht kann. */
struct Pruefer
{
    std::function<void (bool, const juce::String&, const juce::String&)> fn;
    void operator() (bool ok, const juce::String& name,
                     const juce::String& zusatz = {}) const
    { fn (ok, name, zusatz); }
};

static void sonde013M33 (const Pruefer& pruefe, double fs, int bs)
{
    // ── M-33: der vierte fail-closed-Term ────────────────────────────
    //
    // §58 verlangt fuer den Hoermarker `playing=true`, `recording=false`,
    // Realtime und Editor offen. Der dritte fehlte bis SONDE-013 im
    // `erlaubt`-Term, obwohl der Prozessor den Aufnahmezustand kennt und ihn
    // im `state_report` bereits mitschickt.
    //
    // Der Fall hat DREI Zweige, und der dritte ist der wichtige: eine
    // laufende Aufnahme blockiert, ein UNBEKANNTER Zustand blockiert
    // ebenfalls, und nur ein nachgewiesenes `recording=false` faerbt. Ein
    // unbekannter Zustand, der durchliesse, waere fail-open — und eine
    // Faerbung, die in eine Aufnahme laeuft, steht danach in der Datei.
    {
        MarkierungsWunsch w;
        w.modus = MarkierungsModus::solo;
        w.istResonanz = false;
        w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0;
        w.fs = fs;
        MarkierungsAuftrag auftrag;
        const bool gebaut = baueMarkierungsAuftrag (auftrag, w);
        pruefe (gebaut, "M-33: Auftrag baut", {});

        juce::MidiBuffer midi;
        juce::AudioBuffer<float> puffer (2, bs), kopie (2, bs);
        auto fuelleSinus = [&puffer, bs]
        {
            for (int k = 0; k < 2; ++k)
                for (int i = 0; i < bs; ++i)
                    puffer.setSample (k, i, 0.4f * std::sin (2.0f * 3.14159265f
                                                             * 200.0f * (float) i / 48000.0f));
        };

        // Ein Lauf, der `anzahlBloecke` Bloecke faehrt und sagt, ob der
        // Ausgang je vom Eingang abwich. `nimmtAuf` und `ohneKontext`
        // stellen die drei Zweige her.
        auto faerbt = [&, fs, bs] (bool nimmtAuf, bool ohneKontext, int anzahlBloecke = 40)
        {
            // ⚠️ HEAP, nicht Stack. `EqCopilotProcessor` traegt FeatureEngine,
            // AnalyseEngine und mehrere Ringe; diese Funktion haelt schon ein
            // Dutzend davon, und die vier neuen Abschnitte haben den 1-MiB-Stack
            // sofort gesprengt (gemessen beim Bau der Etappe D).
            auto halter = std::make_unique<EqCopilotProcessor>();
            auto& p = *halter;
            p.setPlayConfigDetails (2, 2, fs, bs);
            p.prepareToPlay (fs, bs);
            p.testForciereEchtzeit (true);
            LaufenderTransport transport (p, nimmtAuf);
            if (! alsMainKlassifizieren (p))
                return false;
            p.markierungEinreichen (auftrag);
            bool abweichung = false;
            for (int block = 0; block < anzahlBloecke; ++block)
            {
                fuelleSinus();
                kopie.makeCopyOf (puffer);
                if (ohneKontext) transport.vorBlockOhneKontext (bs);
                else             transport.vorBlock (bs);
                p.processBlock (puffer, midi);
                transport.weiter (bs);
                if (! blockBitgleich (puffer, kopie))
                    abweichung = true;
            }
            return abweichung;
        };

        pruefe (faerbt (false, false),
                "M-33: bei nachgewiesenem recording=false faerbt die Markierung "
                "(die Gegenprobe, ohne die der Rest nichts sagt)", {});
        pruefe (! faerbt (true, false),
                "M-33: recording_true blockiert - eine Faerbung, die in eine Aufnahme "
                "laeuft, steht danach in der Datei");
        pruefe (! faerbt (false, true),
                "M-33: recording_unknown blockiert EBENSO - fail-closed, nicht "
                "fail-open. Ohne Hostbruecke weiss niemand, ob gerade aufgenommen wird");
    }

}

static void sonde013M34 (const Pruefer& pruefe, double fs, int bs)
{
    // ── M-34/M-35: NAK-47, Ausfade statt Schnitt ─────────────────────
    //
    // NAK-47 woertlich: "`hartAus()` setzt `fade = 0.0` sofort. Faellt
    // `erlaubt` weg, waehrend die Markierung hoerbar ist, springt das Signal
    // im naechsten Block abrupt vom gefilterten auf den Originalpfad —
    // hoerbarer Klick." Kein Bruch von Gate 1 (das Audio wird ja gerade NICHT
    // mehr veraendert), aber ein Bruch des Startbudgets §49.3.
    //
    // Der Fall misst die KANTE: die groesste Sampledifferenz zwischen dem
    // letzten Sample vor und dem ersten nach dem Erlaubnisverlust, gegen
    // dieselbe Klickschwelle, die T5 daneben benutzt.
    {
        MarkierungsWunsch w;
        w.modus = MarkierungsModus::solo;
        w.istResonanz = false;
        w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0;
        w.fs = fs;
        MarkierungsAuftrag auftrag;
        baueMarkierungsAuftrag (auftrag, w);

        auto halter = std::make_unique<EqCopilotProcessor>();   // Heap, s. o.
        auto& p = *halter;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        LaufenderTransport transport (p);
        pruefe (alsMainKlassifizieren (p), "M-34: als Main klassifiziert", {});
        p.markierungEinreichen (auftrag);

        juce::MidiBuffer midi;
        juce::AudioBuffer<float> puffer (2, bs), kopie (2, bs);
        // ⚠️ Der Sinus laeuft ueber die Bloecke FORT. Ein Signal, das in jedem
        // Block bei Phase 0 neu beginnt, springt an jeder Blockgrenze — und
        // der Kantentest unten maesse dann die Sprungstelle des TESTSIGNALS
        // statt die der Markierung (gemessen beim Bau: 0,29 statt 0,01).
        std::int64_t phasePos = 0;
        auto fuelleSinus = [&puffer, &phasePos, bs]
        {
            for (int i = 0; i < bs; ++i)
            {
                const float v = 0.4f * std::sin (2.0f * 3.14159265f * 200.0f
                                                 * (float) (phasePos + i) / 48000.0f);
                for (int k = 0; k < 2; ++k)
                    puffer.setSample (k, i, v);
            }
            phasePos += bs;
        };

        // Einschwingen, bis die Markierung wirklich klingt.
        for (int block = 0; block < 40; ++block)
        {
            fuelleSinus();
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
        }
        pruefe (p.markierungHoerbar(), "M-34: die Markierung klingt vor dem Entzug", {});

        // Der Entzug: der Editor schliesst. `testForciereEchtzeit` bleibt an,
        // damit GENAU dieser Term faellt und nicht drei auf einmal.
        p.testForciereEchtzeit (false);
        p.setzeEditorOffen (false);

        // ⚠️ Der Bezugswert kommt aus dem letzten GEFILTERTEN Block der
        // Einschwingphase. Ein ungefilterter Vorlaufblock erzeugte hier eine
        // Kante zwischen trocken und gefiltert — also den Filter, nicht den
        // Klick (gemessen beim Bau: 0,28 statt 0,01).
        float groessteKante = 0.0f;
        float letztesVorher = puffer.getSample (0, bs - 1);
        int bloeckeMitAenderung = 0;
        for (int block = 0; block < 40; ++block)
        {
            fuelleSinus();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            if (! blockBitgleich (puffer, kopie))
                ++bloeckeMitAenderung;
            groessteKante = std::max (groessteKante,
                                      std::abs (puffer.getSample (0, 0) - letztesVorher));
            for (int i = 1; i < bs; ++i)
                groessteKante = std::max (groessteKante,
                                          std::abs (puffer.getSample (0, i)
                                                    - puffer.getSample (0, i - 1)));
            letztesVorher = puffer.getSample (0, bs - 1);
        }
        pruefe (bloeckeMitAenderung > 0,
                "M-34: permission_loss_fades_instead_of_cutting - nach dem Entzug wird "
                "der Puffer noch fuer die Rampe angefasst, statt sofort zu springen",
                juce::String (bloeckeMitAenderung) + " Bloecke mit Rampe");
        pruefe (groessteKante < 0.04f,
                "M-34: und die groesste Sampledifferenz bleibt unter der Klickschwelle "
                "(dieselbe wie in T5)",
                juce::String (groessteKante, 5));
        pruefe (! p.markierungHoerbar(),
                "M-34: danach meldet sie sich als still", {});

        // M-35: der Neutralpfad ist danach BIT-identisch, ueber mehrere
        // Blockgroessen. §44.2: "ein rechnerischer Identity-Filter reicht
        // nicht als Nullvertrag."
        bool alleBitgleich = true;
        for (const int groesse : { 32, 64, 128, 512, 1024 })
        {
            p.prepareToPlay (fs, groesse);
            juce::AudioBuffer<float> b1 (2, groesse), b2 (2, groesse);
            for (int block = 0; block < 8; ++block)
            {
                for (int k = 0; k < 2; ++k)
                    for (int i = 0; i < groesse; ++i)
                        b1.setSample (k, i, zufall() * 0.5f);
                b2.makeCopyOf (b1);
                transport.vorBlock (groesse);
                p.processBlock (b1, midi);
                transport.weiter (groesse);
                if (! blockBitgleich (b1, b2))
                    alleBitgleich = false;
            }
        }
        pruefe (alleBitgleich,
                "M-35: after_fade_the_neutral_path_is_bit_identical - ueber fuenf "
                "Blockgroessen, mit Zufallsaudio und BITvergleich");
        p.prepareToPlay (fs, bs);
    }

}

static void sonde013M36 (const Pruefer& pruefe, double fs, int bs)
{
    // ── M-36 (§7.1 E-01): der Oversize-Pfad ──────────────────────────
    //
    // Ein Hostblock ueber `wetKapazitaet` konnte der Wet-Pfad noch nie
    // vollstaendig rechnen; frueher hiess das `hartAus()` — derselbe Klick
    // wie bei M-34, nur an einer zweiten Stelle. E-01 entscheidet:
    // erzwungener Ausfade INNERHALB der Kapazitaet, danach Riegel bis
    // `prepareToPlay`.
    //
    // Der Riegel ist der Teil, den man leicht vergisst: ohne ihn blendete die
    // Markierung nach jedem Oversizeblock wieder ein und beim naechsten
    // wieder aus — ein Flattern, das schlimmer ist als der Schnitt.
    {
        MarkierungsWunsch w;
        w.modus = MarkierungsModus::solo;
        w.istResonanz = false;
        w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0;
        w.fs = fs;
        MarkierungsAuftrag auftrag;
        baueMarkierungsAuftrag (auftrag, w);

        auto halter = std::make_unique<EqCopilotProcessor>();   // Heap, s. o.
        auto& p = *halter;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        LaufenderTransport transport (p);
        pruefe (alsMainKlassifizieren (p), "M-36: als Main klassifiziert", {});
        p.markierungEinreichen (auftrag);

        juce::MidiBuffer midi;
        juce::AudioBuffer<float> puffer (2, bs), kopie (2, bs);
        auto fuelleSinus = [] (juce::AudioBuffer<float>& b)
        {
            for (int k = 0; k < b.getNumChannels(); ++k)
                for (int i = 0; i < b.getNumSamples(); ++i)
                    b.setSample (k, i, 0.4f * std::sin (2.0f * 3.14159265f
                                                        * 200.0f * (float) i / 48000.0f));
        };

        for (int block = 0; block < 40; ++block)
        {
            fuelleSinus (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
        }
        pruefe (p.markierungHoerbar(), "M-36: die Markierung klingt vor dem Oversizeblock", {});

        // Der Oversizeblock: doppelte Kapazitaet. Der Host darf das (JUCE:
        // `maximumExpectedSamplesPerBlock` ist ein Hinweis, keine Zusage).
        const int gross = bs * 2;
        juce::AudioBuffer<float> grossPuffer (2, gross), grossKopie (2, gross);
        fuelleSinus (grossPuffer);
        grossKopie.makeCopyOf (grossPuffer);
        transport.vorBlock (gross);
        p.processBlock (grossPuffer, midi);
        transport.weiter (gross);

        // Der HINTERE Teil des Blocks - ab `wetKapazitaet` - ist woertlich
        // der Eingang. Das ist die Zusage aus E-01.
        bool hintenBitgleich = true;
        for (int k = 0; k < 2; ++k)
            for (int i = bs; i < gross; ++i)
                if (grossPuffer.getSample (k, i) != grossKopie.getSample (k, i))
                    hintenBitgleich = false;
        pruefe (hintenBitgleich,
                "M-36: der Teil des Oversizeblocks jenseits der Kapazitaet ist "
                "woertlich der Eingang - kein halber Wet-Pfad, keine Extrapolation");

        // Und danach: der RIEGEL. Ein erneutes `erlaubt` blendet in dieser
        // Laufzeit NICHT wieder ein.
        //
        // ⚠️ Erst die RAMPE auslaufen lassen. Der Oversizeblock setzt das Ziel
        // auf 0, aber die Rampe braucht ihre 80 ms — und wer sofort danach
        // misst, sieht sie und haelt sie fuer ein Wiedereinblenden.
        for (int block = 0; block < 20; ++block)
        {
            fuelleSinus (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
        }
        bool wiederGefaerbt = false;
        for (int block = 0; block < 60; ++block)
        {
            fuelleSinus (puffer);
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            if (! blockBitgleich (puffer, kopie))
                wiederGefaerbt = true;
        }
        pruefe (! wiederGefaerbt,
                "M-36: oversize_block_fades_within_capacity_then_latches - danach "
                "blendet sie NICHT wieder ein, auch nicht nach 60 gueltigen Bloecken");
        pruefe (! p.markierungHoerbar(), "M-36: und meldet sich als still", {});

        // Erst `prepareToPlay` loest den Riegel - die Blockgroesse ist neu
        // ausgehandelt, also ist der Grund fuer die Verriegelung entfallen.
        p.prepareToPlay (fs, bs);
        p.markierungEinreichen (auftrag);
        bool wiederMoeglich = false;
        for (int block = 0; block < 60; ++block)
        {
            fuelleSinus (puffer);
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            if (! blockBitgleich (puffer, kopie))
                wiederMoeglich = true;
        }
        pruefe (wiederMoeglich,
                "M-36: nach prepareToPlay ist sie wieder moeglich - der Riegel ist "
                "eine Verriegelung, keine Abschaltung");
    }
}

// === Nacharbeit 1 nach der Erstpruefung 1 (2026-09-04) ===================
//
// Drei Befunde an DERSELBEN Stelle des RT-Pfades, und alle drei haben
// dieselbe Wurzel: der Ausfade wurde nicht bis zur Null zu Ende gerechnet
// und sein Ende nicht dort gemeldet, wo es lag.
//
//  B01  Der Oversizeblock rechnete mit der VOLLEN Rampenlaenge, obwohl er nur
//       `wetKapazitaet` Samples rechnen kann. Bei 48 kHz, 80 ms Fade und
//       512 Samples Kapazitaet blieben rund 0,867 Wet uebrig - die
//       Folgebloecke waren weiter gefaerbt, entgegen 7.1 E-01.
//  B02  `endete` fiel schon bei `fade <= 0,001`, also waehrend noch gemischt
//       wurde. Der Taint war frei, bevor das Audio sauber war (M-38).
//  B03  `project_sample_end` trug den BLOCKANFANG. Bis zu ein ganzer Hostblock
//       gefaerbtes Audio lag damit ausserhalb der Invalidierung (M-38/M-52).
static void sonde013Nacharbeit1 (const Pruefer& pruefe, double fs, int bs)
{
    MarkierungsWunsch w;
    w.modus = MarkierungsModus::solo;
    w.istResonanz = false;
    w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0;
    w.fs = fs;
    MarkierungsAuftrag auftrag;
    if (! baueMarkierungsAuftrag (auftrag, w))
    {
        pruefe (false, "Nacharbeit 1: der Auftrag baut", {});
        return;
    }

    auto fuelleSinus = [fs] (juce::AudioBuffer<float>& b)
    {
        for (int k = 0; k < b.getNumChannels(); ++k)
            for (int i = 0; i < b.getNumSamples(); ++i)
                b.setSample (k, i, 0.4f * std::sin (2.0f * 3.14159265f
                                                    * 200.0f * (float) i / (float) fs));
    };

    // -- B01: der Oversizeblock ist mit dem Ausfade FERTIG ----------------
    //
    // Die Zusage aus E-01 lautet "Rest des Blocks UND JEDER FOLGEBLOCK sind
    // bitidentischer Neutralpfad". Der alte Bau erfuellte nur die erste
    // Haelfte; gemessen wird deshalb der Block UNMITTELBAR danach, nicht
    // einer nach zwanzig Rampenbloecken.
    {
        HoerMarkierungDsp dsp;
        dsp.setzeSamplerate (fs);
        dsp.vorbereiten (bs);
        dsp.reicheEin (auftrag);

        juce::AudioBuffer<float> puffer (2, bs);
        for (int block = 0; block < 40; ++block)
        {
            fuelleSinus (puffer);
            dsp.verarbeite (puffer, 2, true);
        }
        pruefe (dsp.hoerbar(), "B01: die Markierung klingt vor dem Oversizeblock", {});

        const int gross = bs * 2;
        juce::AudioBuffer<float> grossPuffer (2, gross), grossKopie (2, gross);
        fuelleSinus (grossPuffer);
        grossKopie.makeCopyOf (grossPuffer);
        dsp.verarbeite (grossPuffer, 2, true);

        bool hintenBitgleich = true;
        for (int k = 0; k < 2 && hintenBitgleich; ++k)
            for (int i = bs; i < gross; ++i)
                if (grossPuffer.getSample (k, i) != grossKopie.getSample (k, i))
                { hintenBitgleich = false; break; }
        pruefe (hintenBitgleich,
                "B01: der Teil jenseits der Kapazitaet bleibt woertlich der Eingang");

        // DAS ist die neue Zusage: der NAECHSTE Block ist schon bitidentisch.
        juce::AudioBuffer<float> nach (2, bs), nachKopie (2, bs);
        fuelleSinus (nach);
        nachKopie.makeCopyOf (nach);
        dsp.verarbeite (nach, 2, true);
        pruefe (blockBitgleich (nach, nachKopie),
                "B01: oversize_fade_endet_in_der_kapazitaet - der Block UNMITTELBAR "
                "nach dem Oversizeblock ist bitidentisch, nicht erst nach 20 "
                "Rampenbloecken (7.1 E-01)");
        pruefe (! dsp.hoerbar(),
                "B01: und die Markierung meldet sich sofort als still", {});
    }

    // -- B02: das Ende faellt bei fade == 0 EXAKT ------------------------
    //
    // Die alte Schwelle war `fade > 0,001`. Sie faellt nur dann auf, wenn ein
    // Block GENAU in dem schmalen Band zwischen 0,001 und 0 endet - bei 80 ms
    // Fade sind das knapp vier Samples von 3840. Mit 512er Bloecken trifft man
    // sie fast nie; mit EINsample-Bloecken trifft man sie zwangslaeufig, weil
    // dann jeder Fadewert einmal am Blockende steht.
    //
    // Gemessen wird deshalb der Fadewert selbst: `phase()` ist im Solobetrieb
    // genau `fade`. Faellt das Ende, muss er 0 sein - nicht "fast 0".
    {
        HoerMarkierungDsp dsp;
        dsp.setzeSamplerate (fs);
        dsp.vorbereiten (bs);
        dsp.reicheEin (auftrag);

        juce::AudioBuffer<float> gross (2, bs);
        for (int block = 0; block < 40; ++block)
        {
            fuelleSinus (gross);
            dsp.verarbeite (gross, 2, true);
        }

        juce::AudioBuffer<float> eins (2, 1);
        float phaseBeimEnde = -1.0f;
        int schritte = 0;
        for (; schritte < 8000; ++schritte)
        {
            eins.setSample (0, 0, 0.4f);
            eins.setSample (1, 0, 0.4f);
            const auto schritt = dsp.verarbeite (eins, 2, false);
            if (schritt.endete)
            {
                phaseBeimEnde = dsp.phase();
                break;
            }
        }
        pruefe (phaseBeimEnde >= 0.0f, "B02: der Ausfade meldet ein Ende",
                juce::String (schritte) + " Samples");
        pruefe (phaseBeimEnde == 0.0f,
                "B02: ende_erst_bei_fade_null - beim gemeldeten Ende ist der "
                "Wet-Anteil EXAKT 0, nicht 'unter 0,001' (M-38)",
                juce::String (phaseBeimEnde, 6));
    }

    // -- B03: das Ende traegt den Offset des letzten gefaerbten Samples ---
    {
        HoerMarkierungDsp dsp;
        dsp.setzeSamplerate (fs);
        dsp.vorbereiten (bs);
        dsp.reicheEin (auftrag);

        juce::AudioBuffer<float> puffer (2, bs), kopie (2, bs);
        for (int block = 0; block < 40; ++block)
        {
            fuelleSinus (puffer);
            dsp.verarbeite (puffer, 2, true);
        }

        // Erlaubnis weg: die Rampe laeuft zu Ende. Der Block, in dem `endete`
        // faellt, ist der einzige interessante.
        int endeBlock = -1, endeOffset = -1;
        bool schwanzBitgleich = false, kopfGefaerbt = false, folgeBitgleich = true;
        for (int block = 0; block < 40; ++block)
        {
            fuelleSinus (puffer);
            kopie.makeCopyOf (puffer);
            const auto schritt = dsp.verarbeite (puffer, 2, false);
            if (schritt.endete && endeBlock < 0)
            {
                endeBlock = block;
                endeOffset = schritt.endeOffsetSamples;
                // Hinter dem gemeldeten Offset ist der Puffer woertlich der
                // Eingang - sonst zeigte der Offset vor das Ende des Eingriffs.
                schwanzBitgleich = true;
                for (int k = 0; k < 2 && schwanzBitgleich; ++k)
                    for (int i = endeOffset + 1; i < bs; ++i)
                        if (puffer.getSample (k, i) != kopie.getSample (k, i))
                        { schwanzBitgleich = false; break; }
                // Und der Offset liegt NICHT vor der gefaerbten Zone: das
                // letzte Sample, das sich vom Eingang unterscheidet, liegt bei
                // oder vor ihm. (Genau AM Offset kann die Differenz unter der
                // float-Aufloesung liegen — dort ist `f` bereits ein
                // Zehntausendstel, und dann ist Bitgleichheit die richtige
                // Rechnung, nicht ein Fehler.)
                int letzteDifferenz = -1;
                for (int k = 0; k < 2; ++k)
                    for (int i = 0; i < bs; ++i)
                        if (puffer.getSample (k, i) != kopie.getSample (k, i))
                            letzteDifferenz = std::max (letzteDifferenz, i);
                kopfGefaerbt = letzteDifferenz >= 0 && letzteDifferenz <= endeOffset;
            }
            else if (endeBlock >= 0 && ! blockBitgleich (puffer, kopie))
                folgeBitgleich = false;
        }

        pruefe (endeBlock >= 0, "B03: der Ausfade meldet ein Ende",
                juce::String (endeBlock));
        pruefe (folgeBitgleich,
                "B03: nach dem gemeldeten Ende mischt KEIN Folgeblock mehr Wet");
        pruefe (endeOffset >= 0 && endeOffset < bs,
                "B03: der Uebergang traegt einen Offset INNERHALB des Blocks",
                juce::String (endeOffset));
        pruefe (schwanzBitgleich,
                "B03: ende_offset_zeigt_auf_das_letzte_gefaerbte_sample - hinter dem "
                "Offset ist der Block woertlich der Eingang");
        pruefe (kopfGefaerbt,
                "B03: der Offset deckt die gefaerbte Zone VOLLSTAENDIG ab - das "
                "letzte vom Eingang abweichende Sample liegt bei oder vor ihm");
    }

    // -- B04: das Sticky-Bit des Rings erreicht `v3Status()` -------------
    //
    // 34.2: "Ein verlorenes Begin oder End darf niemals eine scheinbar
    // saubere Baseline erzeugen." Fiel das LETZTE Ereignis, folgte keine
    // Sequenzluecke mehr - der Empfaenger blieb ohne Meldung. Der Heartbeat
    // traegt das Bit deshalb aktiv.
    {
        auto halter = std::make_unique<EqCopilotProcessor>();
        auto& p = *halter;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        LaufenderTransport transport (p);
        pruefe (alsMainKlassifizieren (p), "B04: als Main klassifiziert", {});
        pruefe (! p.v3StatusFuerTest().interventionStateUnknown,
                "B04: frisch ist der Interventionszustand BEKANNT - ein Bit, das "
                "immer stuende, sagte nichts");

        // Der Ring laeuft ueber. Das Sticky-Bit setzt der RING (in `schreibe`),
        // nicht der Testaufruf - gemessen wird der Weg von dort in `v3Status()`.
        const int platz = p.interventionsRingFuellenFuerTest();
        pruefe (platz > 0, "B04: der Ring nimmt seine Kapazitaet auf",
                juce::String (platz));
        const auto status = p.v3StatusFuerTest();
        pruefe (status.interventionStateUnknown,
                "B04: ring_overflow_reist_im_heartbeat - der Ueberlauf des "
                "RT-Control-Rings erreicht `v3Status()` (M-39)");

        nakama::ipc::Adresse a;
        a.logonSid = "S-1-5-21-1";
        a.projectBindingId = std::string (32, 'a');
        a.sessionEpoch     = std::string (32, 'b');
        a.instanceId       = std::string (32, 'c');
        a.runtimeNonce     = std::string (32, 'd');
        const auto mit = nakama::ipc::heartbeatAlsJson (a, 1, status);
        pruefe (mit.find ("\"intervention_state_unknown\":true") != std::string::npos,
                "B04: und steht im Heartbeat-JSON");
        nakama::ipc::ControlStatus sauber;
        const auto ohne = nakama::ipc::heartbeatAlsJson (a, 1, sauber);
        pruefe (ohne.find ("intervention_state_unknown") == std::string::npos,
                "B04: ein sauberer Zustand schreibt das Feld GAR NICHT - ein "
                "`false` in jedem Takt waere die Behauptung 'Zustand bekannt'");
    }
}

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr double fs = 48000.0;
    constexpr int bs = 512;
    int fehler = 0;
    auto pruefe = [&fehler] (bool ok, const juce::String& name,
                             const juce::String& zusatz = {})
    {
        std::cout << (ok ? "  ok      " : "  FEHLER  ") << name.toRawUTF8();
        if (zusatz.isNotEmpty())
            std::cout << "  [" << zusatz.toRawUTF8() << "]";
        std::cout << std::endl;
        if (! ok) ++fehler;
    };

    // ── T7a: Auftrag-Bau — Kanten und Stabilität (reine Funktionen) ────────
    {
        MarkierungsAuftrag a;
        MarkierungsWunsch w;
        w.modus = MarkierungsModus::solo;
        w.fVon = std::numeric_limits<double>::quiet_NaN();
        w.fBis = 300.0; w.fSchwerpunkt = 200.0; w.fs = fs;
        pruefe (! baueMarkierungsAuftrag (a, w), "T7: NaN-Feld verhindert Engage");

        w.fVon = 300.0; w.fBis = 120.0;
        pruefe (! baueMarkierungsAuftrag (a, w), "T7: fVon>fBis verhindert Engage");

        w.fVon = 8000.0; w.fBis = 30000.0; w.fSchwerpunkt = 10000.0;   // > Nyquist
        pruefe (baueMarkierungsAuftrag (a, w) && a.sektionen == 4,
                "T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)");

        w.modus = MarkierungsModus::puls;
        w.istResonanz = true;
        w.fSchwerpunkt = 689.0;
        w.fVon = 689.0 * std::pow (2.0, -1.0 / 24.0);
        w.fBis = 689.0 * std::pow (2.0, 1.0 / 24.0);
        w.breiteOktaven = 1.0 / 12.0;
        w.pulsTiefeDb = 4.5;
        const bool gebaut = baueMarkierungsAuftrag (a, w);
        bool identitaet = gebaut;
        if (gebaut)
        {
            // Ruhephasen-Identität: Stufe 0 muss b == a tragen (bit-exakt).
            const auto& k = a.puls[0];
            identitaet = k.b0 == 1.0 && k.b1 == k.a1 && k.b2 == k.a2;
        }
        pruefe (gebaut, "T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)");
        pruefe (identitaet, "T7: Puls-Stufe 0 ist exakte Identitaet (b == a)");
    }

    // ── T2/T5: Solo — hörbar, klickfrei, sauber wieder transparent ─────────
    {
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        LaufenderTransport transport (p);              // SONDE-008 / Hub U10
        pruefe (alsMainKlassifizieren (p), "T2: als Main klassifiziert (§53.5)");

        MarkierungsWunsch w;
        w.modus = MarkierungsModus::solo;
        w.istResonanz = false;
        w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0;
        w.fs = fs;
        MarkierungsAuftrag auftrag;
        const bool ok = baueMarkierungsAuftrag (auftrag, w);
        pruefe (ok, "T2: Zonen-Solo-Auftrag (Mulm 120-300) baut");
        p.markierungEinreichen (auftrag);

        juce::AudioBuffer<float> puffer (2, bs), kopie (2, bs);
        juce::MidiBuffer midi;
        double phase = 0.0;
        const double dPhase = 2.0 * juce::MathConstants<double>::pi * 200.0 / fs;
        float letzter[2] = { 0.0f, 0.0f };
        float maxSprung = 0.0f;
        bool alleEndlich = true;
        int ersteDifferenz = -1;

        auto fuelleSinus = [&]
        {
            for (int i = 0; i < bs; ++i)
            {
                const float v = 0.5f * (float) std::sin (phase);
                phase += dPhase;
                puffer.setSample (0, i, v);
                puffer.setSample (1, i, v);
            }
        };
        auto beobachte = [&] (int block)
        {
            for (int k = 0; k < 2; ++k)
            {
                const float* d = puffer.getReadPointer (k);
                for (int i = 0; i < bs; ++i)
                {
                    if (! std::isfinite (d[i])) alleEndlich = false;
                    maxSprung = std::max (maxSprung, std::abs (d[i] - letzter[k]));
                    letzter[k] = d[i];
                }
            }
            if (ersteDifferenz < 0 && ! blockBitgleich (puffer, kopie))
                ersteDifferenz = block;
        };

        for (int block = 0; block < 60; ++block)
        {
            fuelleSinus();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            beobachte (block);
        }
        pruefe (ersteDifferenz >= 0, "T2: Solo greift (Ausgang weicht ab)");
        pruefe (p.markierungHoerbar(), "T2: markierungHoerbar meldet aktiv");

        p.markierungAus();
        int letzteDifferenz = -1;
        for (int block = 0; block < 40; ++block)
        {
            fuelleSinus();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            beobachte (100 + block);
            if (! blockBitgleich (puffer, kopie))
                letzteDifferenz = block;
        }
        // 80-ms-Fade = 3840 Samples = 7,5 Bloecke; ab Block 10 muss Ruhe sein.
        pruefe (letzteDifferenz >= 0 && letzteDifferenz <= 9,
                "T2: Fade-out endet im erwarteten Fenster");
        bool danachBitgleich = true;
        for (int block = 0; block < 20; ++block)
        {
            fuelleSinus();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            if (! blockBitgleich (puffer, kopie))
                danachBitgleich = false;
        }
        pruefe (danachBitgleich, "T2: nach dem Fade wieder bitgleich transparent");
        pruefe (! p.markierungHoerbar(), "T2: markierungHoerbar meldet aus");
        // Klickgrenze: Sinus-Steigung 0,0131/Sample; Fades linear ⇒ deutlich darunter.
        pruefe (alleEndlich, "T5: Ausgang bleibt endlich");
        pruefe (maxSprung < 0.04f, "T5: kein Klick an Engage-/Disengage-Kanten");
    }

    // ── S20-22/SONDE-013 M-33 bis M-36: eigene Funktionen, s. Kopf ────────
    const Pruefer pruefer { pruefe };
    sonde013M33 (pruefer, fs, bs);
    sonde013M34 (pruefer, fs, bs);
    sonde013M36 (pruefer, fs, bs);
    sonde013Nacharbeit1 (pruefer, fs, bs);

    // ── T9: Puls — Ruhephase praktisch identisch, Schwellphase hörbar ──────
    {
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        LaufenderTransport transport (p);              // SONDE-008 / Hub U10
        pruefe (alsMainKlassifizieren (p), "T9: als Main klassifiziert (§53.5)");

        MarkierungsWunsch w;
        w.modus = MarkierungsModus::puls;
        w.istResonanz = true;
        w.fSchwerpunkt = 689.0;
        w.fVon = 689.0 * std::pow (2.0, -1.0 / 24.0);
        w.fBis = 689.0 * std::pow (2.0, 1.0 / 24.0);
        w.breiteOktaven = 1.0 / 12.0;
        w.pulsTiefeDb = 4.5;
        w.fs = fs;
        MarkierungsAuftrag auftrag;
        pruefe (baueMarkierungsAuftrag (auftrag, w), "T9: Puls-Auftrag 689 Hz baut");
        p.markierungEinreichen (auftrag);

        juce::AudioBuffer<float> puffer (2, bs), kopie (2, bs);
        juce::MidiBuffer midi;
        double phase = 0.0;
        const double dPhase = 2.0 * juce::MathConstants<double>::pi * 689.0 / fs;
        const int anstieg = auftrag.pulsAnstiegSamples;    // 12000
        const int periode = anstieg + auftrag.pulsRuheSamples;   // 30000
        double maxRuheDelta = 0.0, maxSchwellDelta = 0.0;
        juce::int64 sampleIndex = 0;

        for (int block = 0; block < 480; ++block)
        {
            for (int i = 0; i < bs; ++i)
            {
                const float v = 0.5f * (float) std::sin (phase);
                phase += dPhase;
                puffer.setSample (0, i, v);
                puffer.setSample (1, i, v);
            }
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            for (int i = 0; i < bs; ++i, ++sampleIndex)
            {
                const double delta = std::abs ((double) puffer.getSample (0, i)
                                               - (double) kopie.getSample (0, i));
                const juce::int64 inPeriode = sampleIndex % periode;
                if (sampleIndex > 2 * periode)
                {
                    // Ruhefenster mit Abkling-Marge (4000 Samples ≈ 83 ms ≫ τ).
                    if (inPeriode >= anstieg + 4000 && inPeriode < periode - 200)
                        maxRuheDelta = std::max (maxRuheDelta, delta);
                    if (inPeriode >= 2000 && inPeriode < anstieg - 2000)
                        maxSchwellDelta = std::max (maxSchwellDelta, delta);
                }
            }
        }
        pruefe (maxSchwellDelta > 1.0e-3, "T9: Schwellphase ist deutlich hoerbar");
        pruefe (maxRuheDelta <= 2.0e-6, "T9: Ruhephase <= -120 dBFS Rest (Identitaet)");
    }

    // ── T6: State-Roundtrip — Markierung wird NIE persistiert ──────────────
    {
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        // Erst klassifizieren, DANN speichern: sonst waere `q` unten schon
        // wegen §53.5 stumm und der Test bewiese nicht mehr, was er behauptet
        // ("kein Markierungszustand im State"), sondern nur, dass ein legacy-
        // Stand nicht faerbt. `q` erbt die Klassifikation ueber den Restore -
        // der Stand sagt `main`.
        pruefe (alsMainKlassifizieren (p), "T6: als Main klassifiziert (§53.5)");
        MarkierungsWunsch w;
        w.modus = MarkierungsModus::solo;
        w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0; w.fs = fs;
        MarkierungsAuftrag auftrag;
        baueMarkierungsAuftrag (auftrag, w);
        p.markierungEinreichen (auftrag);
        juce::MemoryBlock state;
        p.getStateInformation (state);

        EqCopilotProcessor q;
        q.setStateInformation (state.getData(), (int) state.getSize());
        pruefe (q.holeKlassifikation() == nakama::state::Klassifikation::main,
                "T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)");
        q.setPlayConfigDetails (2, 2, fs, bs);
        q.prepareToPlay (fs, bs);
        q.testForciereEchtzeit (true);
        LaufenderTransport transportQ (q);         // SONDE-008 / Hub U10
        juce::AudioBuffer<float> puffer (2, bs), kopie (2, bs);
        juce::MidiBuffer midi;
        bool bitgleich = true;
        for (int block = 0; block < 20 && bitgleich; ++block)
        {
            for (int k = 0; k < 2; ++k)
            {
                float* d = puffer.getWritePointer (k);
                for (int i = 0; i < bs; ++i) d[i] = zufall();
            }
            kopie.makeCopyOf (puffer);
            transportQ.vorBlock (bs);
            q.processBlock (puffer, midi);
            bitgleich = blockBitgleich (puffer, kopie);
        }
        pruefe (bitgleich && ! q.markierungHoerbar(),
                "T6: Save/Load laedt IMMER neutral (kein Markierungszustand)");
    }

    // ── T7b: Mono-Pfad ─────────────────────────────────────────────────────
    {
        EqCopilotProcessor p;
        p.setPlayConfigDetails (1, 1, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        LaufenderTransport transport (p);              // SONDE-008 / Hub U10
        pruefe (alsMainKlassifizieren (p), "T7: als Main klassifiziert (§53.5)");
        MarkierungsWunsch w;
        w.modus = MarkierungsModus::solo;
        w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0; w.fs = fs;
        MarkierungsAuftrag auftrag;
        baueMarkierungsAuftrag (auftrag, w);
        p.markierungEinreichen (auftrag);
        juce::AudioBuffer<float> puffer (1, bs), kopie (1, bs);
        juce::MidiBuffer midi;
        bool differenz = false, endlich = true;
        double phase = 0.0;
        for (int block = 0; block < 30; ++block)
        {
            for (int i = 0; i < bs; ++i)
            {
                puffer.setSample (0, i, 0.5f * (float) std::sin (phase));
                phase += 2.0 * juce::MathConstants<double>::pi * 200.0 / fs;
            }
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            if (! blockBitgleich (puffer, kopie)) differenz = true;
            for (int i = 0; i < bs; ++i)
                if (! std::isfinite (puffer.getSample (0, i))) endlich = false;
        }
        pruefe (differenz && endlich, "T7: Mono-Solo laeuft und bleibt endlich");
    }

    // ── T3/T10: Lebenszeichen — der Render-Beweis (echte Taktung) ──────────
    {
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.setzeEditorOffen (true);          // Editor-Pflicht erfüllt, KEIN Echtzeit-Zwang
        pruefe (alsMainKlassifizieren (p), "T3: als Main klassifiziert (§53.5)");
        // SONDE-013 M-33: auch dieser Abschnitt braucht die Hostbruecke — ohne
        // sie ist der Aufnahmezustand unbekannt, und ein unbekannter blockiert.
        LaufenderTransport transport (p);

        MarkierungsWunsch w;
        w.modus = MarkierungsModus::puls;
        w.istResonanz = false;
        w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0;
        w.pulsTiefeDb = 4.5; w.fs = fs;
        MarkierungsAuftrag auftrag;
        baueMarkierungsAuftrag (auftrag, w);
        p.markierungEinreichen (auftrag);   // Latch gesetzt — VOR jedem Beweis

        juce::AudioBuffer<float> puffer (2, bs), kopie (2, bs);
        juce::MidiBuffer midi;
        auto fuelle = [&]
        {
            for (int k = 0; k < 2; ++k)
            {
                float* d = puffer.getWritePointer (k);
                for (int i = 0; i < bs; ++i) d[i] = zufall();
            }
        };

        // (a) Freilauf (Render-Simulation): Blöcke ohne Wanduhr-Takt.
        bool freilaufBitgleich = true;
        for (int block = 0; block < 300; ++block)
        {
            fuelle();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            if (! blockBitgleich (puffer, kopie))
                freilaufBitgleich = false;
        }
        pruefe (freilaufBitgleich,
                "T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)");
        pruefe (p.markierungKillGemeldet(),
                "T3: Freilauf meldet den Latch-Fall an den Editor");

        // (b) Echtzeit-Taktung: Markierung darf erst nach dem Beweis greifen.
        p.markierungEinreichen (auftrag);   // Latch erneut setzen (Kill war nur Meldung)
        int ersteDifferenz = -1;
        for (int block = 0; block < 300; ++block)
        {
            fuelle();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            if (ersteDifferenz < 0 && ! blockBitgleich (puffer, kopie))
            {
                ersteDifferenz = block;
                break;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (10));
        }
        pruefe (ersteDifferenz >= 40,
                "T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)");
        pruefe (ersteDifferenz > 0 && ersteDifferenz < 300,
                "T3: Echtzeit-Taktung schaltet die Markierung frei");

        // (c) setNonRealtime: sofortiger harter Schnitt im selben Block.
        // ⚠️ SONDE-013 M-34 hat diese Zusage GEAENDERT, und zwar bewusst.
        // Bis hierher hiess sie "schneidet sofort auf neutral" — genau der
        // Sofortschnitt aus NAK-47, der einen hoerbaren Klick erzeugt. Jetzt
        // laeuft eine Rampe; neutral wird der Pfad NACH ihr, nicht im selben
        // Block. Gemessen wird deshalb beides: dass die Rampe endlich ist,
        // und dass danach jeder Block bitgleich bleibt.
        p.setNonRealtime (true);
        int letzterMitAenderung = -1;
        for (int block = 0; block < 40; ++block)
        {
            fuelle();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            if (! blockBitgleich (puffer, kopie))
                letzterMitAenderung = block;
        }
        // 80-ms-Fade = 3840 Samples = 7,5 Bloecke bei 512.
        pruefe (letzterMitAenderung <= 9,
                "T3/M-34: isNonRealtime BLENDET AUS statt zu schneiden - und die "
                "Rampe ist nach spaetestens zehn Bloecken zu Ende",
                juce::String (letzterMitAenderung));
        bool offlineBitgleich = true;
        for (int block = 0; block < 10; ++block)
        {
            fuelle();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            if (! blockBitgleich (puffer, kopie))
                offlineBitgleich = false;
        }
        pruefe (offlineBitgleich,
                "T3/M-35: und danach ist der Pfad wieder bitgleich neutral");
        p.setNonRealtime (false);

        // (d = T10) Transport-Stopp: harter Schnitt im selben Block.
        int wiederAktiv = -1;
        for (int block = 0; block < 300; ++block)
        {
            fuelle();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            transport.weiter (bs);
            if (! blockBitgleich (puffer, kopie))
            {
                wiederAktiv = block;
                break;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (10));
        }
        pruefe (wiederAktiv >= 0, "T10: Markierung nach Offline-Ende wieder beweisbar");
        // Dieselbe Aenderung wie oben, aus demselben Grund (M-34).
        transport.kopf.spielt = false;
        int letzterStopp = -1;
        for (int block = 0; block < 40; ++block)
        {
            fuelle();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            if (! blockBitgleich (puffer, kopie))
                letzterStopp = block;
        }
        pruefe (letzterStopp >= 0 && letzterStopp <= 30,
                "T10/M-34: Transport-Stopp BLENDET AUS statt zu schneiden - und die "
                "Rampe ist endlich (der Pulsbetrieb traegt ein laengeres Fadefenster "
                "als der Solobetrieb)",
                juce::String (letzterStopp));
        bool stoppBitgleich = true;
        for (int block = 0; block < 10; ++block)
        {
            fuelle();
            kopie.makeCopyOf (puffer);
            transport.vorBlock (bs);
            p.processBlock (puffer, midi);
            if (! blockBitgleich (puffer, kopie))
                stoppBitgleich = false;
        }
        pruefe (stoppBitgleich,
                "T10/M-35: und danach ist der Pfad wieder bitgleich neutral");
        p.setPlayHead (nullptr);
    }

    // ── T4: Abgriff-Beweis — die Messung sieht NIE das gefärbte Signal ─────
    {
        EqCopilotProcessor markiert, sauber;
        bool beideMain = true;
        for (auto* p : { &markiert, &sauber })
        {
            p->setPlayConfigDetails (2, 2, fs, bs);
            p->prepareToPlay (fs, bs);
            p->testForciereEchtzeit (true);
            // BEIDE klassifizieren, obwohl nur `markiert` einen Auftrag
            // bekommt: die zwei Instanzen sollen sich in genau EINER Sache
            // unterscheiden. Ein unterschiedlicher Lebenslauf waere eine
            // zweite Variable im Vergleich.
            beideMain = alsMainKlassifizieren (*p) && beideMain;
        }
        pruefe (beideMain, "T4: beide Instanzen als Main klassifiziert (§53.5)");
        LaufenderTransport transportM (markiert), transportS (sauber);   // SONDE-008 / Hub U10
        MarkierungsWunsch w;
        w.modus = MarkierungsModus::solo;
        w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0; w.fs = fs;
        MarkierungsAuftrag auftrag;
        baueMarkierungsAuftrag (auftrag, w);
        markiert.markierungEinreichen (auftrag);

        juce::AudioBuffer<float> a (2, bs), b (2, bs);
        juce::MidiBuffer midi;
        bool hoerbarWaehrendMessung = false;
        const int bloecke = (int) (19.0 * fs / bs);          // ~19 s Audio
        for (int block = 0; block < bloecke; ++block)
        {
            for (int k = 0; k < 2; ++k)
            {
                float* da = a.getWritePointer (k);
                for (int i = 0; i < bs; ++i) da[i] = 0.5f * zufall();
                std::memcpy (b.getWritePointer (k), da, (size_t) bs * sizeof (float));
            }
            transportM.vorBlock (bs);
            markiert.processBlock (a, midi);
            transportS.vorBlock (bs);
            sauber.processBlock (b, midi);
            transportM.weiter (bs);
            transportS.weiter (bs);
            if (markiert.markierungHoerbar())
                hoerbarWaehrendMessung = true;
            // FIFO-Drain-Takt des Workers respektieren — sonst zaehlen Drops.
            std::this_thread::sleep_for (std::chrono::milliseconds (2));
        }
        pruefe (hoerbarWaehrendMessung, "T4: Markierung war waehrend der Messung hoerbar");
        pruefe (markiert.statsSnapshot().framesDropped == 0
                && sauber.statsSnapshot().framesDropped == 0,
                "T4: keine Analyse-Drops (Vergleich belastbar)");

        // Beide Worker zu Ende rechnen lassen.
        MessSnapshot ma, mb;
        for (int versuch = 0; versuch < 100; ++versuch)
        {
            ma = markiert.messSnapshot();
            mb = sauber.messSnapshot();
            if (ma.zustand == MessZustand::messbereit && mb.zustand == MessZustand::messbereit
                && ma.aktivSekunden >= 17.5 && mb.aktivSekunden >= 17.5
                && ma.ltasGueltig && mb.ltasGueltig)
                break;
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
        }
        pruefe (ma.zustand == MessZustand::messbereit && mb.zustand == MessZustand::messbereit,
                "T4: beide Messungen messbereit");
        double maxDelta = 0.0;
        int verglichen = 0;
        if (ma.ltasGueltig && mb.ltasGueltig)
            for (int band = 0; band < kLtasBaender; ++band)
            {
                const double va = ma.ltasKompositDb[(size_t) band];
                const double vb = mb.ltasKompositDb[(size_t) band];
                if (std::isfinite (va) && std::isfinite (vb))
                {
                    maxDelta = std::max (maxDelta, std::abs (va - vb));
                    ++verglichen;
                }
            }
        pruefe (verglichen > 100, "T4: genug Baender fuer den Vergleich");
        pruefe (maxDelta <= 0.75, "T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)");
    }

    // ── T11: DER U10-WAECHTER (Nacharbeit T2-2, 23.08.2026) ────────────────
    //
    // WOZU ES DIESEN ABSCHNITT GIBT. Der User hat am 22.08. im Hub (`U10`)
    // entschieden: „Nein, nur mit Signal" - die Hoer-Markierung faerbt nur mit
    // GUELTIGEM „spielt", das fail-open `(spielt ∨ ¬hatTransport)` ist
    // gefallen. S10-11 hat das umgesetzt, und der T2-Pruefer hat es als
    // wirksam gemessen. Er hat aber auch das Gegenteil gemessen: das fail-open
    // probeweise zurueckgebaut - und ALLE VIER Audiobeine blieben gruen
    // (NullTest, MarkierungTest, LebenslaufTest, QueueStressTest, je Exit 0).
    // Ein Entscheid des Users, den kein Bein deckt, laesst sich spaeter
    // unbemerkt zuruecknehmen.
    //
    // WARUM KEIN ANDERER ABSCHNITT DAS LEISTET, und das ist der Kern: alle
    // uebrigen wurden dem neuen Term ANGEPASST (ueberall ein laufender
    // Playhead, s. `LaufenderTransport` oben) statt gegen seine RUECKNAHME
    // gehaertet. Mit laufendem Playhead ist `hatTransport` wahr, der
    // fail-open-Zweig also wieder tot - beide Fassungen verhalten sich dann
    // identisch. Genau die Eigenschaft, die den Term in FL harmlos macht,
    // macht ihn untestbar, solange kein Bein einen Prozessor OHNE Playhead
    // faehrt. T11 faehrt genau das.
    {
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        pruefe (alsMainKlassifizieren (p), "T11: als Main klassifiziert (§53.5)");

        MarkierungsWunsch w;
        w.modus = MarkierungsModus::solo;
        w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0; w.fs = fs;
        MarkierungsAuftrag auftrag;
        const bool gebaut = baueMarkierungsAuftrag (auftrag, w);
        pruefe (gebaut, "T11: Auftrag gebaut");

        juce::AudioBuffer<float> puffer (2, bs), kopie (2, bs);
        juce::MidiBuffer midi;
        // SONDE-013 M-33: mit Hostbruecke, sonst ist der Aufnahmezustand
        // unbekannt und blockiert. Sie steht VOR `fahre`, weil das Lambda sie
        // braucht.
        //
        // ⚠️ Fuer Fall (a) wird der Playhead gleich wieder ABGEZOGEN: „OHNE
        // Playhead" ist genau die Bedingung dieses Falls, und ein Transport,
        // der ihn setzt, machte ihn wertlos. Die Bruecke bleibt — sie liefert
        // den Aufnahmezustand, nicht den Transport.
        LaufenderTransport transport (p);
        transport.kopf.spielt = false;
        p.setPlayHead (nullptr);
        auto fahre = [&] (int bloecke)
        {
            bool gefaerbt = false;
            for (int block = 0; block < bloecke; ++block)
            {
                for (int k = 0; k < 2; ++k)
                {
                    float* d = puffer.getWritePointer (k);
                    for (int i = 0; i < bs; ++i) d[i] = 0.5f * zufall();
                }
                kopie.makeCopyOf (puffer);
                transport.vorBlock (bs);
                p.processBlock (puffer, midi);
                if (! blockBitgleich (puffer, kopie))
                    gefaerbt = true;
            }
            return gefaerbt;
        };

        // (a) DIE Pruefung. Volle Erlaubnis auf jeder anderen Achse - Main
        //     klassifiziert, Editor offen (ueber alsMainKlassifizieren),
        //     testForciereEchtzeit, aktiver Auftrag - und KEIN Playhead. Damit
        //     ist „spielt" ungueltig, und der U10-Entscheid verlangt Stille.
        //     Baut jemand das fail-open zurueck, faerbt hier sofort etwas.
        p.setPlayHead (nullptr);
        p.markierungEinreichen (auftrag);
        pruefe (! fahre (120),
                "T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)");
        pruefe (! p.markierungHoerbar(),
                "T11: und die Markierung meldet sich nicht hoerbar");

        // (b) Der bekannt GESTOPPTE Transport ist derselbe Fall aus der anderen
        //     Richtung: „spielt" ist gueltig und sagt nein.
        // (b) braucht den Playhead wieder — „bekannt gestoppt" ist etwas
        // anderes als „gar kein Transport".
        p.setPlayHead (&transport.kopf);
        transport.kopf.spielt = false;
        p.markierungEinreichen (auftrag);
        pruefe (! fahre (120), "T11: mit gestopptem Transport ebenfalls kein Sample");

        // (c) GEGENPROBE, und ohne sie waere (a) wertlos: derselbe Aufbau mit
        //     laufendem Playhead MUSS faerben. Sonst koennte T11 gruen sein,
        //     weil die Markierung generell stumm ist - der Fehler, vor dem der
        //     Kopf von `LaufenderTransport` warnt.
        transport.kopf.spielt = true;
        p.markierungEinreichen (auftrag);
        bool gefaerbt = false;
        for (int block = 0; block < 200 && ! gefaerbt; ++block)
        {
            gefaerbt = fahre (1);
            transport.weiter (bs);
        }
        pruefe (gefaerbt,
                "T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau");
        p.setPlayHead (nullptr);
    }

    std::cout << (fehler == 0 ? "MARKIERUNGSTEST OK" : "MARKIERUNGSTEST FEHLGESCHLAGEN") << std::endl;
    return fehler == 0 ? 0 : 1;
}
