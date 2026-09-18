// NAK-309 (R-309-5, R-309-3): die Regeln des Probewerkzeugs eqcop-pipe-probe.exe
// (probe/PipeProbeMain.cpp) als reine Funktionen. Das Werkzeug faehrt kein
// Bein, der Kanon baut es nicht (NAK-289 NB-1); Bein A4b
// (tests/PipeClientLifecycleTestMain.cpp) misst diese Funktionen, und
// Quelltextwachen dort halten ihre Verdrahtung in main fest.
//
// Pipename (R-309-5, CLAUDE.md "Probe-Pipe ist nie Produktions-Pipe"): ohne
// erstes Argument gilt der v2-Probename, nie kPipeName. Zugelassen wird ueber
// eine ERLAUBNIS, keine Sperrliste (dieselbe Wahl wie istProbePipename in
// PipeToken.h): der v2-Probename, er mit einem Suffix aus Punkt und
// [A-Za-z0-9_-], und jeder Name, den das unveraenderte istProbePipename annimmt
// (v3-Probenamensraum). Alles andere ist leer, produktion oder fremd. Den
// Produktionsnamen vergleicht die Regel ohne Gross- und Kleinschreibung, weil
// Windows Pipenamen so behandelt (MS Learn "Pipe Names": "Pipe names are not
// case-sensitive."). Ein Trenner (\ oder /) hinter \\.\pipe\ macht jeden Namen
// fremd: Win32 loest `..` in \\.\-Pfaden auf, und
// `\\.\pipe\<probe>\..\<produktion>` oeffnete sonst die Produktion - auch ueber
// den v3-Zweig, dessen Praefixpruefung den Rest nicht ansieht.
//
// Konfliktende (R-309-3): es gilt nur, wenn A nach dem Stopp von B ein NEUES
// ACK derselben Verbindung ohne Konflikt bestaetigt bekommt. Ein
// Verbindungsabbruch (Status getrennt) und ein Neuaufbau (verbindungsVersuche
// plus eins, ACK-Zaehler neu ab 0) loeschen das Flag ebenfalls
// (PipeClient.cpp, eineVerbindung) und sind kein Konfliktende.
#pragma once

#include "EqCopilotIds.h"
#include "PipeClient.h"
#include "PipeToken.h"

#include <juce_core/juce_core.h>

namespace eqcop::probe
{

/// Der v2-Probename. eqcop-broker-probe.exe lauscht ohne Pipeargument hier
/// (broker/src/bin/eqcop-broker-probe.rs); eq-copilot/README.md nennt ihn.
inline constexpr const char* kV2ProbePipeName = "\\\\.\\pipe\\evenacadia.eq-copilot.m2probe";

enum class PipeUrteil { zugelassen, leer, produktion, fremd };

inline const char* urteilWort (PipeUrteil urteil) noexcept
{
    switch (urteil)
    {
        case PipeUrteil::zugelassen: return "zugelassen";
        case PipeUrteil::leer:       return "leer";
        case PipeUrteil::produktion: return "produktion";
        case PipeUrteil::fremd:      return "fremd";
    }
    return "fremd";
}

/// Der Name, den das Werkzeug oeffnen will: das erste Argument, wie es kommt -
/// auch leer, das entscheidet probePipeUrteil -, ohne Argument der
/// v2-Probename. Nie kPipeName.
inline juce::String probePipeName (int argc, const char* const* argv)
{
    return argc > 1 ? juce::String (argv[1]) : juce::String (kV2ProbePipeName);
}

/// Darf das Probewerkzeug diesen Namen oeffnen? Erlaubnis statt Sperrliste.
inline PipeUrteil probePipeUrteil (const juce::String& name)
{
    if (name.isEmpty())
        return PipeUrteil::leer;
    if (name.equalsIgnoreCase (juce::String (juce::CharPointer_UTF16 (kPipeName))))
        return PipeUrteil::produktion;

    const juce::String praefix ("\\\\.\\pipe\\");
    if (! name.startsWith (praefix) || name.substring (praefix.length()).containsAnyOf ("\\/"))
        return PipeUrteil::fremd;

    const juce::String v2 (kV2ProbePipeName);
    if (name == v2)
        return PipeUrteil::zugelassen;
    if (name.startsWith (v2 + "."))
    {
        const auto suffix = name.substring (v2.length() + 1);
        if (suffix.isNotEmpty()
            && suffix.containsOnly ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-"))
            return PipeUrteil::zugelassen;
    }
    return nakama::ipc::istProbePipename (name.toStdString()) ? PipeUrteil::zugelassen
                                                              : PipeUrteil::fremd;
}

/// R-309-3: `vorher` ist der Snapshot von A direkt nach b->stop(). Die gleiche
/// Versuchszahl schliesst den Neuaufbau aus, auch wenn dessen neu ab 0
/// gezaehlter ACK-Zaehler den alten Wert ueberholt.
inline bool konfliktEndeBestaetigt (const PipeClient::Snapshot& vorher,
                                    const PipeClient::Snapshot& jetzt) noexcept
{
    return jetzt.status == PipeClient::Status::verbunden
        && jetzt.protokollVersion == 2
        && jetzt.verbindungsVersuche == vorher.verbindungsVersuche
        && jetzt.heartbeatsBestaetigt > vorher.heartbeatsBestaetigt
        && ! jetzt.konflikt;
}

} // namespace eqcop::probe
