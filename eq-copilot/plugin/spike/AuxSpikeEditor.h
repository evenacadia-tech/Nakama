// Arbeitsflaeche des Wegwerf-Messgeraets. Bewusst schmucklos: sie muss in FL
// in zwei Sekunden ABLESBAR sein und darf keinen Zustand behaupten, den es
// nicht gibt (Entwurf Abschnitt 0.4 - fehlender Messwert wird als fehlend gezeigt,
// nie als 0). Keine Designarbeit, kein Kandidat fuer die Produkt-UI.
#pragma once

#include "AuxSpikeProcessor.h"

namespace eqcop::spike
{

class AuxSpikeEditor final : public juce::AudioProcessorEditor,
                             private juce::Timer
{
public:
    explicit AuxSpikeEditor (AuxSpikeProcessor&);
    ~AuxSpikeEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    /** Kurzform des Messstands - aendert sie sich nicht, wird nicht gemalt. */
    static juce::String signatur (const Schnappschuss&);

    AuxSpikeProcessor& prozessor;
    juce::TextButton berichtKnopf { "Bericht schreiben" };
    juce::TextButton resetKnopf   { "Messung zuruecksetzen" };
    juce::String letzteMeldung { "noch kein Bericht geschrieben" };
    juce::String letzteSignatur;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AuxSpikeEditor)
};

} // namespace eqcop::spike
