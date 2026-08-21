// Oberflaeche des Termin-B-Messgeraets. Sie zeigt AUSSCHLIESSLICH Gemessenes -
// jede Zeile, die noch keinen Messwert hat, sagt das ausdruecklich ("noch
// nicht gesehen") statt eine 0 zu zeigen. Eine Anzeige, die "0" fuer
// "unbekannt" schreibt, waere genau die Luege, gegen die dieses Geraet gebaut ist.
#pragma once

#include "HostProbeProcessor.h"

namespace eqcop::hostprobe
{

class HostProbeEditor final : public juce::AudioProcessorEditor,
                              private juce::Timer
{
public:
    explicit HostProbeEditor (HostProbeProcessor&);
    ~HostProbeEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    static juce::String signatur (const Messstand&);

    HostProbeProcessor& prozessor;
    juce::TextButton berichtKnopf { "Bericht sichern" };
    juce::TextButton resetKnopf   { "Messung zuruecksetzen" };
    juce::String     letzteMeldung;
    juce::String     letzteSignatur;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HostProbeEditor)
};

} // namespace eqcop::hostprobe
