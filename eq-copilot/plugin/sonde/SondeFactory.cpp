// Der JUCE-Einstiegspunkt beider Sonden-Bundles. Eine Datei genuegt fuer
// zwei Ziele: welche Produktklasse entsteht, entscheidet der Define aus der
// duennen Target-Schicht (NAKAMA_SONDE_PASSIV / NAKAMA_SONDE_AKTIV), nicht
// dieser Code. Nur die Plugin-Ziele uebersetzen diese Datei - Tests binden
// SondeProcessor.cpp ohne sie.
#include "SondeProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new nakama::sonde::SondeProcessor();
}
