// Nur das Plugin-Target definiert die JUCE-Einstiegsfabrik — Probe und
// Nulltest kompilieren die übrigen Quellen ohne diese Datei.
#include "PluginProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new eqcop::EqCopilotProcessor();
}
