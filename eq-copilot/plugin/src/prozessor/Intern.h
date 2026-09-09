// Interner Kopf des Prozessors — der EINE Helfer, den mehrere Teile brauchen.
//
// NAK-225/S25d (09.09.2026): Beim Aufteilen von PluginProcessor.cpp nach
// Fachbereich wurde gemessen, welcher Helfer des anonymen Namensraums von mehr
// als einem Teil gebraucht wird. Es ist genau einer: `uuidHex32`. Er entsteht
// im Konstruktor (Session-Epoche), in der IPC-Anbindung (Command-IDs), im
// State (Bindungswechsel) und im Analyse-Anstoss (Versuchs- und
// Passagen-IDs) — vier Uebersetzungseinheiten, ein Helfer.
//
// Ein anonymer Namensraum in einem Kopf haette in jeder einbindenden
// Uebersetzungseinheit eine EIGENE Kopie erzeugt: keine ODR-Verletzung, aber
// vier Funktionen mit vier Adressen dort, wo eine gemeint ist. Deshalb steht
// er hier `inline` in einem eigenen, benannten Namensraum: eine Entitaet, eine
// Adresse, und der Name sagt, dass er niemandem ausserhalb des Prozessors
// gehoert. Wer ihn braucht, holt ihn mit einer using-Deklaration einzeln in
// `eqcop` — nicht mit `using namespace`, damit die Kopfzeile jeder Teildatei
// zeigt, was sie von hier nimmt.
//
// Alle uebrigen Helfer der drei anonymen Namensraeume sind bei ihrem EINZIGEN
// Nutzer geblieben und stehen dort weiterhin anonym: `projektEnde`,
// `projektAbstandGroesserAls64`, `brokerStartMutexName` und
// `brokerServerErwartung` in der Wurzel, `alsHex32` und `wireZahl` in Ipc.cpp,
// `fingerprintJson`, `zahl` und `jsonText` in Analyse.cpp.
//
// Dieser Kopf gehoert zu keinem Audio-Pfad und wird von ihm nur im
// Konstruktor beruehrt (einmalig, vor dem ersten Block).

#pragma once

#include <juce_core/juce_core.h>

#include "PipeToken.h"

namespace eqcop::prozessor_intern
{

inline std::string uuidHex32()
{
    std::string roh = juce::Uuid().toString().toStdString();
    std::string aus;
    aus.reserve (32);
    for (char c : roh)
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            aus.push_back (c);
    return aus.size() == 32 ? aus
                            : nakama::ipc::instanceAdresseAusState ("runtime:" + roh);
}

} // namespace eqcop::prozessor_intern
