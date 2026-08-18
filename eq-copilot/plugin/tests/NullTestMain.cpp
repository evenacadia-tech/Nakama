// Headless-Nulltest (M0-Abnahme, Plan §11): beweist ohne Host, dass der
// Processor das Signal bitgleich durchreicht, 0 Latenz und 0 Tail meldet und
// NaN/Inf zwar zählt, aber nicht verändert. Exit 0 nur bei "NULLTEST OK".
#include "PluginProcessor.h"
#include <cstring>
#include <iostream>

static juce::uint32 lcg = 0x2545f491u;
static float zufall()
{
    lcg = lcg * 1664525u + 1013904223u;
    return ((float) (lcg >> 8) / (float) 0x00ffffff) * 2.0f - 1.0f;   // −1 … +1
}

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    int fehler = 0;
    auto pruefe = [&fehler] (bool ok, const char* name)
    {
        std::cout << (ok ? "  ok      " : "  FEHLER  ") << name << std::endl;
        if (! ok) ++fehler;
    };

    {
        eqcop::EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, 48000.0, 512);
        p.prepareToPlay (48000.0, 512);

        pruefe (p.getLatencySamples() == 0, "Latenz ist 0 Samples");
        pruefe (p.getTailLengthSeconds() == 0.0, "Tail ist 0 s");

        // 1000 Bloecke deterministisches Rauschen — Ausgang muss bitgleich sein.
        juce::AudioBuffer<float> puffer (2, 512), kopie (2, 512);
        juce::MidiBuffer midi;
        bool identisch = true;
        for (int block = 0; block < 1000 && identisch; ++block)
        {
            for (int k = 0; k < 2; ++k)
            {
                float* d = puffer.getWritePointer (k);
                for (int i = 0; i < 512; ++i) d[i] = zufall();
            }
            kopie.makeCopyOf (puffer);
            p.processBlock (puffer, midi);
            for (int k = 0; k < 2 && identisch; ++k)
                identisch = std::memcmp (puffer.getReadPointer (k), kopie.getReadPointer (k),
                                         512 * sizeof (float)) == 0;
        }
        pruefe (identisch, "1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)");
        pruefe (! p.statsSnapshot().nanSeen, "kein falscher NaN-Alarm bei endlichem Signal");

        // NaN/Inf: wird gezaehlt, Signal bleibt unangetastet (Plan §9.1).
        for (int k = 0; k < 2; ++k)
        {
            float* d = puffer.getWritePointer (k);
            for (int i = 0; i < 512; ++i) d[i] = zufall();
        }
        puffer.getWritePointer (0)[7]   = std::numeric_limits<float>::quiet_NaN();
        puffer.getWritePointer (1)[100] = std::numeric_limits<float>::infinity();
        kopie.makeCopyOf (puffer);
        p.processBlock (puffer, midi);
        bool nanUnveraendert = true;
        for (int k = 0; k < 2 && nanUnveraendert; ++k)
            nanUnveraendert = std::memcmp (puffer.getReadPointer (k), kopie.getReadPointer (k),
                                           512 * sizeof (float)) == 0;
        pruefe (nanUnveraendert, "NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)");
        pruefe (p.statsSnapshot().nanSeen, "NaN/Inf wurde als Mangel registriert");

        // Mono muss ebenfalls gehen; unbekannte Layouts lehnt das Plugin ab.
        juce::AudioProcessor::BusesLayout mono;
        mono.inputBuses.add (juce::AudioChannelSet::mono());
        mono.outputBuses.add (juce::AudioChannelSet::mono());
        pruefe (p.checkBusesLayoutSupported (mono), "Mono-Layout wird angenommen");
        juce::AudioProcessor::BusesLayout surround;
        surround.inputBuses.add (juce::AudioChannelSet::create5point1());
        surround.outputBuses.add (juce::AudioChannelSet::create5point1());
        pruefe (! p.checkBusesLayoutSupported (surround), "5.1 wird klar abgelehnt (kein stilles Downmixen)");

        // State-Rueckweg: Save→Load erhaelt Sensor-ID und Bindung.
        p.setzeBindung ("post", "PIANO-Bus POST", "paar-a");
        juce::MemoryBlock state;
        p.getStateInformation (state);
        const auto idVorher = p.holeSensorId();
        eqcop::EqCopilotProcessor q;
        q.setStateInformation (state.getData(), (int) state.getSize());
        pruefe (q.holeSensorId() == idVorher, "Save/Load: Sensor-ID bleibt stabil");
        pruefe (q.holeRolle() == "post" && q.holePaarId() == "paar-a", "Save/Load: Rolle und Paar-ID bleiben");
    }

    std::cout << (fehler == 0 ? "NULLTEST OK" : "NULLTEST FEHLGESCHLAGEN") << std::endl;
    return fehler == 0 ? 0 : 1;
}
