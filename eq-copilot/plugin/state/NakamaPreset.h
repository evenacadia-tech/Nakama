#pragma once

/*  Preset der aktiven Sonde (SONDE-015 R12; Vertrag:
    eq-copilot/schemas/state/nakama-preset-v1.json).

    Ein Preset ist der KLANGINHALT einer Sonde als eigene, transportierbare
    Datei - nicht ihr Projektzustand. Es traegt, was gehoert wird, und nichts,
    was die Sonde verdrahtet.

    Drei Regeln, die hier Code sind:

      1. Zwei Versionen, nie vermischt. `preset_schema_version` sagt, wie das
         Preset gebaut ist; die mitgefuehrte `dsp_schema_version` sagt, welches
         Parameterlayout sein Inhalt spricht.
      2. Fehlende Felder statt leerer Felder. Sechs Namen - Zieladresse,
         `instance_id`, Bus-Name, Label, `eq_enabled`, Undo-Ring - haben KEIN
         Feld, und ein Preset, das einen davon traegt, wird abgewiesen. Ein
         optionales, leeres Identitaetsfeld waere eine offene Tuer.
      3. Unbekanntes Top-Level-Feld: ignorieren. Unbekannter Major: ablehnen.
         Der Unterschied ist Absicht - ein zusaetzliches Feld ist eine additive
         Erweiterung, ein anderer Major eine andere Sprache. Die
         read-only-Haltung des Projektzustands gilt hier NICHT: ein Preset ist
         keine Nutzerarbeit, die verlustfrei bewahrt werden muesste, sondern
         eine Datei, die man neu laden kann.

    Datei-Dialoge und Browser sind S31b; dieses Ticket liefert den Weg, nicht
    die Hand daran.

    Keine `JucePlugin_*`-Konstante, nur juce_core - S8-tauglich. Nicht fuer den
    Audiothread.
*/

#include <juce_core/juce_core.h>

#include "NakamaParameter.h"

namespace nakama::preset
{

constexpr int kPresetSchemaVersion = 1;

/** Die sechs verbotenen Top-Level-Namen aus `nakama-preset-v1.json`. Sie
    stehen an EINER Stelle im C++-Code; `EqCopStateMigrationTest` haelt die
    Menge gegen die Vertragsdatei, damit ein siebter Name im Vertrag den Test
    faellt und nicht das Produkt. */
extern const juce::StringArray& verboteneFelder();

/** Die Kennungen, die ein Preset traegt: alle 120 des Layouts v2 AUSSER
    `v2.global.eq_enabled`. Abgeleitet aus `parameter::tabelle()`, nie als
    zweite Liste gepflegt - zwei Listen driften. */
bool gehoertInsPreset (int index) noexcept;

/** Zahl der Kennungen im Preset: 119. */
constexpr int kAnzahl = parameter::kAnzahl - 1;

/** Schreibt ein Preset als RFC-8785-kanonisches UTF-8-JSON.

    Kanonisch, nicht huebsch: derselbe Klanginhalt ergibt damit immer dieselben
    Bytes, und ein Writer-Fixture ist bytegleich reproduzierbar.
    `v2.global.eq_enabled` wird nicht geschrieben - es hat kein Feld. */
bool schreibe (const parameter::DspSatz& dto, juce::MemoryBlock& utf8, juce::String& grund);

/** Liest ein Preset in `ausInOut`.

    `ausInOut` kommt mit dem AKTUELLEN Zustand herein: die 119 Presetwerte und
    die Zonen werden ersetzt, `v2.global.eq_enabled` bleibt UNBERUEHRT. Genau
    das ist die Zusage „ein Preset schaltet die Rolle nie ein".

    `grund` traegt bei false ein Wort aus `ablehnungsgruende` der
    Vertragsdatei, `detail` die Fundstelle. */
bool lies (const void* utf8, size_t laenge, parameter::DspSatz& ausInOut,
           juce::String& grund, juce::String& detail);

} // namespace nakama::preset
