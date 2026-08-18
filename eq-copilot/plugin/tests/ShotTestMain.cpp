// Editor-Sichtprüfung ohne Host (Übergabe §14.2/10): baut den echten
// Processor, speist ~20 s Rauschen + 116-Hz-Ton durch processBlock (der
// Worker misst wie im Betrieb) und rendert den Material-Editor offscreen als
// PNG — Kurve, Marker, Messleisten und Statuszellen sind dann echt belegt.
//
// Aufruf:  EqCopShot <ziel.png> [breitePx]
// Läuft die Hub-App, verbindet sich der echte PipeClient (LED grün; der
// Sensor erscheint kurz in der Übersicht und meldet sich sauber ab) — ohne
// App zeigt die LED ehrlich „App aus" (rot). Beides ist Betriebsrealität.
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <cstdio>

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File ziel = juce::File::getCurrentWorkingDirectory()
        .getChildFile (argc > 1 ? juce::String (juce::CharPointer_UTF8 (argv[1]))
                                : juce::String ("eqcop-shot.png"));
    const int breite = argc > 2 ? juce::jlimit (600, 1950, juce::String (argv[2]).getIntValue())
                                : 1200;

    eqcop::EqCopilotProcessor proz;
    proz.prepareToPlay (48000.0, 512);

    // 20 s Material: Rauschbett −24 dBFS + 116-Hz-Ton — liefert messbereit,
    // eine lebende Kurve und den Resonanz-Marker aus der echten Engine.
    juce::AudioBuffer<float> block (2, 512);
    juce::MidiBuffer midi;
    juce::Random rnd (20260814);
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
            juce::Thread::sleep (60);   // Worker-Takt 50 ms: FIFO leeren lassen
    }
    juce::Thread::sleep (500);          // letzte auswerten()-Runde (~250 ms)

    std::unique_ptr<juce::AudioProcessorEditor> editor (proz.createEditor());
    editor->setSize (breite, juce::roundToInt ((float) breite * 520.0f / 750.0f));
    // FPS-Fix (m4): paint() liest die Timer-Kopie des Editors — headless
    // feuern Timer nur über callPendingTimersSynchronously (JUCE 8 hat
    // runDispatchLoopUntil hinter JUCE_MODAL_LOOPS_PERMITTED verriegelt).
    for (int i = 0; i < 10; ++i)
    {
        juce::Thread::sleep (35);
        juce::Timer::callPendingTimersSynchronously();
    }
    const auto bild = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
    editor.reset();

    ziel.deleteFile();
    juce::FileOutputStream strom (ziel);
    juce::PNGImageFormat png;
    if (! strom.openedOk() || ! png.writeImageToStream (bild, strom))
    {
        std::printf ("SHOT FEHLGESCHLAGEN — %s nicht schreibbar\n",
                     ziel.getFullPathName().toRawUTF8());
        return 1;
    }
    strom.flush();
    std::printf ("SHOT OK %s (%dx%d)\n", ziel.getFullPathName().toRawUTF8(),
                 bild.getWidth(), bild.getHeight());
    return 0;
}
