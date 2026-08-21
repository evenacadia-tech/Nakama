// Nur das VST3-Target definiert die JUCE-Einstiegsfabrik; der Selbsttest
// kompiliert dieselben Quellen ohne diese Datei.
#include "HostProbeProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new eqcop::hostprobe::HostProbeProcessor();
}
