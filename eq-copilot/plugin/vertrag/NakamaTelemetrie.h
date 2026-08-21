#pragma once

/*  C++-Bein des FlatBuffers-Telemetrievertrags (SONDE-005b).

    Gegenstueck zu broker/src/telemetrie.rs. Beide lesen denselben
    Binaerkorpus und muessen ihn IDENTISCH klassifizieren - Urteil UND
    vollstaendige Verstossmenge -, gemessen gegen dasselbe handgeschriebene
    eq-copilot/fixtures/v3/flatbuffers/MANIFEST.json.

    WARUM EIN LESER NOETIG IST, OBWOHL ES EIN SCHEMA GIBT

    FlatBuffers prueft beim Verifizieren die STRUKTUR: Offsets zeigen in den
    Puffer, `required`-Felder sind da, Vektorlaengen passen zum Puffer. Es
    prueft NICHT:

      * ob ein Enumwert im deklarierten Bereich liegt (ein `ubyte` 99 kommt
        unbeanstandet durch Verify und erst der Leser sieht es),
      * ob ein Bitflagfeld nur bekannte Bits traegt,
      * irgendeine Beziehung zwischen zwei Feldern.

    Genau dort liegen aber die Regeln aus Entwurf §33.1: hoechstens ein Frame
    je Quelle, Encoding passt zur Nutzlast, Bandzahl folgt aus dem Gitter,
    Bitmap ist ceil(n/8) Bytes. Ein Vertrag, der sie nur in seinen
    Kommentaren traegt, ist keiner.
*/

#include <juce_core/juce_core.h>

#include <cstddef>
#include <cstdint>

namespace nakama::telemetrie
{

/** Ein einzelner Vertragsverstoss.

    `pfad` ist ein JSON-Pointer in den Batch ("" ist der Batch selbst),
    `regel` einer der Namen aus der geschlossenen Liste im README. Beide Beine
    bilden denselben Text; die Menge wird kanonisch sortiert, damit der
    Vergleich nicht von der Auswertungsreihenfolge abhaengt.
*/
struct Verstoss
{
    juce::String pfad;
    juce::String regel;

    bool operator== (const Verstoss& a) const noexcept;
    /** Kanonische Ordnung (pfad, regel) - BYTEWEISE ueber UTF-8, damit sie mit
        Rusts `String`-Ordnung zusammenfaellt. */
    bool operator<  (const Verstoss& a) const noexcept;
};

/** Obergrenze der Eintraege je Batch (§53.9: „bis zu 32 Sonden"). */
inline constexpr size_t maxEintraege = 32;

/** Bandzahlen der beiden eingefrorenen Gitter (schemas/v3/bandgitter/). */
inline constexpr size_t baenderFein = 221;
inline constexpr size_t baenderGrob = 64;

/** Prueft einen rohen FeatureBatch-Puffer.

    @returns die kanonisch sortierte, doppelfreie Verstossmenge; leer heisst
             gueltig.
*/
juce::Array<Verstoss> pruefe (const uint8_t* puffer, size_t laenge);

inline bool gueltig (const uint8_t* puffer, size_t laenge)
{
    return pruefe (puffer, laenge).isEmpty();
}

} // namespace nakama::telemetrie
