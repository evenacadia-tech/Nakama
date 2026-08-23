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
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <thread>

using namespace eqcop;

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

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr double fs = 48000.0;
    constexpr int bs = 512;
    int fehler = 0;
    auto pruefe = [&fehler] (bool ok, const char* name)
    {
        std::cout << (ok ? "  ok      " : "  FEHLER  ") << name << std::endl;
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
            p.processBlock (puffer, midi);
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
            p.processBlock (puffer, midi);
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
            p.processBlock (puffer, midi);
            if (! blockBitgleich (puffer, kopie))
                danachBitgleich = false;
        }
        pruefe (danachBitgleich, "T2: nach dem Fade wieder bitgleich transparent");
        pruefe (! p.markierungHoerbar(), "T2: markierungHoerbar meldet aus");
        // Klickgrenze: Sinus-Steigung 0,0131/Sample; Fades linear ⇒ deutlich darunter.
        pruefe (alleEndlich, "T5: Ausgang bleibt endlich");
        pruefe (maxSprung < 0.04f, "T5: kein Klick an Engage-/Disengage-Kanten");
    }

    // ── T9: Puls — Ruhephase praktisch identisch, Schwellphase hörbar ──────
    {
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
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
            p.processBlock (puffer, midi);
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
            p.processBlock (puffer, midi);
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
        TestPlayHead kopf;
        kopf.spielt = true;
        p.setPlayHead (&kopf);

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
            p.processBlock (puffer, midi);
            kopf.pos += bs;
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
            p.processBlock (puffer, midi);
            kopf.pos += bs;
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
        p.setNonRealtime (true);
        bool offlineBitgleich = true;
        for (int block = 0; block < 5; ++block)
        {
            fuelle();
            kopie.makeCopyOf (puffer);
            p.processBlock (puffer, midi);
            kopf.pos += bs;
            if (! blockBitgleich (puffer, kopie))
                offlineBitgleich = false;
        }
        pruefe (offlineBitgleich, "T3: isNonRealtime schneidet sofort auf neutral");
        p.setNonRealtime (false);

        // (d = T10) Transport-Stopp: harter Schnitt im selben Block.
        int wiederAktiv = -1;
        for (int block = 0; block < 300; ++block)
        {
            fuelle();
            kopie.makeCopyOf (puffer);
            p.processBlock (puffer, midi);
            kopf.pos += bs;
            if (! blockBitgleich (puffer, kopie))
            {
                wiederAktiv = block;
                break;
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (10));
        }
        pruefe (wiederAktiv >= 0, "T10: Markierung nach Offline-Ende wieder beweisbar");
        kopf.spielt = false;
        bool stoppBitgleich = true;
        for (int block = 0; block < 5; ++block)
        {
            fuelle();
            kopie.makeCopyOf (puffer);
            p.processBlock (puffer, midi);
            if (! blockBitgleich (puffer, kopie))
                stoppBitgleich = false;
        }
        pruefe (stoppBitgleich, "T10: Transport-Stopp schneidet sofort auf neutral");
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
            markiert.processBlock (a, midi);
            sauber.processBlock (b, midi);
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

    std::cout << (fehler == 0 ? "MARKIERUNGSTEST OK" : "MARKIERUNGSTEST FEHLGESCHLAGEN") << std::endl;
    return fehler == 0 ? 0 : 1;
}
