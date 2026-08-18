// Paint-Benchmark (FPS-Baustelle 2026-08-15): misst die reinen paint()-Kosten
// des Editors offscreen — ohne Host, ohne Message-Loop-Rauschen.
//
// Warum das die richtige Messung ist: Der User-Befund „extrem wenig FPS" hat
// zwei mögliche Quellen — (a) paint() ist teuer, (b) die DATEN ändern sich
// selten (Snapshot-Kadenz). Dieser Bench isoliert (a) reproduzierbar; (b)
// steht im Code (Worker-Takt) und im Befund-Doc. Vorher/Nachher-Zahlen
// gehören in eq-copilot/docs (FPS-Kapitel).
//
// Aufruf:  EqCopPaintBench [breitePx=1200] [frames=120]
// Ausgabe: avg/min/max ms pro paint + hochgerechnete Obergrenze in FPS.
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI init;
    const int breite = argc > 1 ? juce::jlimit (600, 1950, juce::String (argv[1]).getIntValue())
                                : 1200;
    const int frames = argc > 2 ? juce::jlimit (10, 2000, juce::String (argv[2]).getIntValue())
                                : 120;

    eqcop::EqCopilotProcessor proz;
    proz.prepareToPlay (48000.0, 512);

    // Wie EqCopShot: ~20 s Rauschbett + 116-Hz-Ton, damit Kurve, Marker,
    // Meter und Statuszellen echte Arbeit leisten (leerer Graph wäre ein
    // geschöntes Ergebnis).
    juce::AudioBuffer<float> block (2, 512);
    juce::MidiBuffer midi;
    juce::Random rnd (20260815);
    double phase = 0.0;
    const double dphi = juce::MathConstants<double>::twoPi * 116.0 / 48000.0;
    for (int b = 0; b < 1875; ++b)
    {
        for (int i = 0; i < 512; ++i)
        {
            const float rausch = (rnd.nextFloat() * 2.0f - 1.0f) * 0.06f;
            const float ton = 0.12f * (float) std::sin (phase);
            phase += dphi;
            block.setSample (0, i, rausch + ton);
            block.setSample (1, i, rausch + ton);
        }
        proz.processBlock (block, midi);
        if ((b + 1) % 48 == 0)
            juce::Thread::sleep (60);
    }
    juce::Thread::sleep (500);

    std::unique_ptr<juce::AudioProcessorEditor> editor (proz.createEditor());
    const int hoehe = juce::roundToInt ((float) breite * 520.0f / 750.0f);
    editor->setSize (breite, hoehe);
    // Timer-Kopie füllen (paint liest `anzeige`) — headless feuern Timer nur
    // über callPendingTimersSynchronously.
    for (int i = 0; i < 10; ++i)
    {
        juce::Thread::sleep (35);
        juce::Timer::callPendingTimersSynchronously();
    }

    juce::Image ziel (juce::Image::ARGB, breite, hoehe, true);

    // Software-Renderer wie im ungünstigsten Host-Fall; 5 Frames Aufwärmen
    // (Font-/Bild-Caches), dann Messreihe.
    auto einFrame = [&]
    {
        juce::Graphics g (ziel);
        editor->paintEntireComponent (g, true);
    };
    for (int i = 0; i < 5; ++i)
        einFrame();

    std::vector<double> ms;
    ms.reserve ((size_t) frames);
    for (int i = 0; i < frames; ++i)
    {
        const auto t0 = juce::Time::getHighResolutionTicks();
        einFrame();
        const auto t1 = juce::Time::getHighResolutionTicks();
        ms.push_back (juce::Time::highResolutionTicksToSeconds (t1 - t0) * 1000.0);
    }
    editor.reset();

    double summe = 0.0, minMs = 1e18, maxMs = 0.0;
    for (const double v : ms)
    {
        summe += v;
        minMs = std::min (minMs, v);
        maxMs = std::max (maxMs, v);
    }
    std::sort (ms.begin(), ms.end());
    const double avg = summe / (double) frames;
    const double p95 = ms[(size_t) juce::jlimit (0, frames - 1, (int) std::lround (frames * 0.95) - 1)];
    std::printf ("PAINTBENCH %dx%d  frames %d\n", breite, hoehe, frames);
    std::printf ("  avg %.2f ms   min %.2f   p95 %.2f   max %.2f\n", avg, minMs, p95, maxMs);
    std::printf ("  Obergrenze aus paint(): %.0f FPS\n", avg > 0.0 ? 1000.0 / avg : 0.0);
    return 0;
}
