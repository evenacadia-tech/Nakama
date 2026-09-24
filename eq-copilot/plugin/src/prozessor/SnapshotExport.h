// NAK-313 Etappe 7 (R-313-10): das Ergebnis des Festhalten-Exports und seine
// injizierbaren Fassaden (docs/beweise/NAK-313.md §8.7).
//
// EqCopilotProcessor::schreibeSnapshotDatei nimmt den Messsnapshot des
// Aufrufers, liest die Uhr genau einmal und veroeffentlicht nach dem Muster
// des Diagnose-Briefkastens: Temp-Datei exklusiv anlegen, schreiben und
// spuelen, Groesse pruefen, ohne Ersetzen umbenennen. Eine Kollision ersetzt
// nie; bis zur Antwort auf Karte U65 endet sie als abgelehnt.
//
// Die Fassaden sind die Typen des Briefkastens (core/diagnose), unveraendert
// benutzt: die Uhr liefert die Zeit, die Ordnerfassade die Wurzel
// %LOCALAPPDATA%, unter der der Export die drei Ebenen evenacadia,
// EQ-Copilot und snapshots anlegt, die Dateisystemfassade jeden Zugriff. Im
// Testbau setzt ein Bein sie ueber testzugang; ohne gesetzten Testordner
// verweigert der Testbau den Export, bevor er das Dateisystem beruehrt.
//
// Der Kopf liegt ausserhalb von PluginProcessor.h, weil die Datei an ihrer
// Zeilengrenze steht (A32).
#pragma once

#include <juce_core/juce_core.h>
#include "diagnose/Briefkasten.h"

#include <memory>

namespace eqcop
{

/** Was ein Export ergeben hat. neu: die Datei ist entstanden; ersetzt: sie hat
    eine vorhandene ersetzt (erzeugt vor der Antwort auf U65 kein Weg);
    abgelehnt: der Name war schon vergeben, nichts gespeichert; fehler: nichts
    veroeffentlicht. */
struct SnapshotExport
{
    enum class Art { neu, ersetzt, abgelehnt, fehler };

    Art          art = Art::fehler;
    juce::String datei;      ///< neu, ersetzt: die entstandene Datei
    juce::String vorhanden;  ///< abgelehnt: die Datei, die den Namen schon traegt
    juce::String rest;       ///< eine eigene Temp-Datei, die nicht entfernt werden konnte
    juce::String grund;      ///< fehler, abgelehnt: der Grund; ersetzt: der Hinweis
};

/** Der Zielordner des Produkts: %LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots,
    aus derselben Wurzelfassade und denselben Ebenen wie der Export. Nur der
    Pfad, kein Dateizugriff (M-143). */
juce::File snapshotOrdnerVorgabe();

/** Die Uhr des Produkts: die Systemuhr (M-143). */
std::shared_ptr<nakama::diagnose::UhrFassade> snapshotUhrVorgabe();

#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
namespace testzugang
{
/** NAK-313 Etappe 7 (M-133, M-143): die Fassaden des Exports, nur im Testbau
    und prozessweit, weil zwei Instanzen denselben Ordner teilen (M-135).
    Ohne gesetzte Ordnerfassade verweigert der Export; ohne Uhr oder
    Dateisystem gelten die echten. */
std::shared_ptr<nakama::diagnose::WurzelFassade>&      snapshotOrdnerFuerTest();
std::shared_ptr<nakama::diagnose::UhrFassade>&         snapshotUhrFuerTest();
std::shared_ptr<nakama::diagnose::DateisystemFassade>& snapshotDateisystemFuerTest();
} // namespace testzugang
#endif

} // namespace eqcop
