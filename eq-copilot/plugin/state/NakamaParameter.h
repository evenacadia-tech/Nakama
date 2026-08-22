#pragma once

/*  Fester Parameterbestand der aktiven Sonde (SONDE-006, Entwurf §53.8).

    Die Wahrheit ist eq-copilot/schemas/state/nakama-parameter-v1.json; diese
    Tabelle MUSS ihr deckungsgleich sein - EqCopStateMigrationTest misst das
    (Anzahl, IDs, Typen, Grenzen, Defaults, Enumwoerter). Warum die Tabelle
    trotzdem in C++ steht: der Audiothread und die APVTS-Konstruktion
    (SONDE-015) brauchen sie zur Bauzeit, nicht als Datei zur Laufzeit.

    Heute (Eqcp = Gen/Legacy) gibt es KEINE Hostparameter - §53.8: "der
    bestehende Eqcp-Eintrag aendert seine Parameterliste nicht". Die Tabelle
    beschreibt das Layout, das Probeeq (NkAc) ab SONDE-007b/015 traegt, und
    sie ist heute schon der Vertrag fuer `Parameters schema=1` im State und
    fuer das DSP-DTO samt `state_hash`.

    Keine `JucePlugin_*`-Konstante, nur juce_core - S8-tauglich.
*/

#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>

#include <array>

namespace nakama::parameter
{

constexpr int kSlots          = 8;
constexpr int kGlobal         = 5;
constexpr int kJeSlot         = 13;
constexpr int kAnzahl         = kGlobal + kSlots * kJeSlot;   // 109
constexpr int kDspSchemaVersion = 1;
static_assert (kAnzahl == 109, "§53.8: 5 globale + 8 x 13 Slot-Parameter");

enum class Typ     { boolean, gleitkomma, aufzaehlung };
enum class Wechsel { rampe, blockrand };

struct Beschreibung
{
    juce::String id;            ///< z. B. "v1.band.3.freq_hz"
    Typ          typ;
    double       min = 0.0, max = 0.0, standardZahl = 0.0;   ///< gleitkomma
    bool         standardBool = false;                        ///< boolean
    juce::StringArray werte;                                  ///< aufzaehlung (Reihenfolge = Vertrag)
    int          standardIndex = 0;                           ///< aufzaehlung
    Wechsel      wechsel = Wechsel::rampe;
    bool         topologisch = false;
};

/** Die 109 Beschreibungen in Vertragsreihenfolge (global, dann Slot 0..7). */
const std::array<Beschreibung, kAnzahl>& tabelle();

/** Index 0..108 oder -1. */
int indexVonId (const juce::String& id);

/** Ein Parameterwert - genau EIN Feld ist je nach Typ gueltig. */
struct Zelle
{
    bool   b = false;
    double zahl = 0.0;
    int    enumIndex = 0;
};

/** Alle 109 Werte, Index = Vertragsreihenfolge. */
using Satz = std::array<Zelle, kAnzahl>;

/** Der neutrale Zustand: jeder Parameter auf seinem Default. */
Satz standardSatz();

/** Prueft Endlichkeit, Bereich und Enumindex. `grund` ist ein Wort aus
    fixtures/state/MANIFEST.json "gruende" (nichtendlich | bereich | enum). */
bool validiere (const Satz& s, juce::String& grund, juce::String& woId);

/** Liest ein DSP-DTO aus UTF-8-Text - die volle Stufenfolge aus
    nakama-state-v2.md §4: v3-Textriegel (Bytes, Text) → doppelte Schluessel →
    Parser → Struktur → dsp_schema_version → unbekannt → fehlend → Typ →
    nichtendlich → Bereich/Enum. `grund` traegt bei false das Manifestwort
    des ERSTEN Fehlers. */
bool ausDtoText (const void* utf8, size_t laenge, Satz& aus, juce::String& grund, juce::String& detail);

/** Validiert und kanonisiert (RFC 8785) das DTO `{"dsp_schema_version":1,"parameters":{...}}`. */
bool dtoKanon (const Satz& s, juce::MemoryBlock& utf8, juce::String& grund);

/** `state_hash`: SHA-256-Hex des kanonischen DTO (§53.8). */
bool stateHash (const Satz& s, juce::String& hex, juce::String& grund);

/** Schreibt die 109 Werte als Eigenschaften (bool / double / Enumwort) in
    Vertragsreihenfolge; bestehende Eigenschaften werden ueberschrieben,
    fremde bleiben. */
void schreibeInBaum (const Satz& s, juce::ValueTree& parameters);

/** Liest die 109 Eigenschaften; fehlt eine oder hat sie den falschen Typ,
    ist das KEIN Teilstate (false + grund). Zusaetzliche Eigenschaften werden
    ignoriert (additiv). */
bool leseAusBaum (const juce::ValueTree& parameters, Satz& aus, juce::String& grund);

} // namespace nakama::parameter
