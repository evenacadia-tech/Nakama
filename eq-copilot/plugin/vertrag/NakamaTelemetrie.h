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
#include <string>
#include <vector>

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

/** Plausibilitaetsgrenzen der Bandwerte, als bereits skalierte Traegerwerte.

    Quelle ist schemas/v3/quantisierung-v1.json, Feld `plausibler_bereich_db` -
    NICHT `bereich_db`, das ist der Traegerumfang (+/-32767). Der Unterschied
    war ein T2-Runde-3-Befund: README und Beweismanifest nannten `bereich_db`
    als Quelle, waehrend hier etwas ganz anderes durchgesetzt wurde.

    EqCopSchemaTest vergleicht diese vier Zahlen bei JEDEM Lauf mit der
    Vertragsdatei; die Rust-Seite tut dasselbe. Damit ist die Quellenangabe
    keine Behauptung mehr, sondern eine Pruefung.
*/
inline constexpr int16_t q0p1Min  = -1440;   // -144.0 dB * 10
inline constexpr int16_t q0p1Max  =   240;   //   24.0 dB * 10
inline constexpr int16_t q0p01Min = -14400;  // -144.00 dB * 100
inline constexpr int16_t q0p01Max =   2400;  //   24.00 dB * 100

/** Prueft einen rohen FeatureBatch-Puffer.

    @returns die kanonisch sortierte, doppelfreie Verstossmenge; leer heisst
             gueltig.
*/
juce::Array<Verstoss> pruefe (const uint8_t* puffer, size_t laenge);

inline bool gueltig (const uint8_t* puffer, size_t laenge)
{
    return pruefe (puffer, laenge).isEmpty();
}

/** Typisierte, eigentuemerhaltende Kopie eines gueltigen Broker->Main-Frames.

    Die FlatBuffers-Zeiger leben nur so lange wie der Eingangspuffer. Das
    Main-Modell arbeitet asynchron zur Pipe und bekommt deshalb ausschliesslich
    diese Kopie; kein generierter Zeiger verlaesst den Leser. */
struct Empfangsframe
{
    std::string projectBindingId, sessionEpoch, instanceId, runtimeNonce;
    std::uint64_t transportEpoch = 0, continuitySegment = 0, sequence = 0;
    std::uint32_t sampleCount = 0;
    double sampleRate = 0.0;
    bool lufsPaar = false;
    float lufsI = 0.0f, lufsIUnsicherheitLu = 0.0f;
    int lufsIStatus = 0;  ///< 0=fehlt, 1=collecting, 2=gated

    /** NAK-68: ueber wie viel Audio dieser Rahmen integriert wurde.

        Das Praesenzbit steht ABSICHTLICH neben dem Wert, statt 0 als
        "fehlt" zu lesen: 0 ist auf der Leitung ein Senderfehler, Abwesenheit
        dagegen erlaubt. Ein Empfaenger, der beides zusammenzoege, koennte
        "leise" weiterhin nicht von "kurz gemessen" unterscheiden - genau der
        Zustand, den NAK-68 beschreibt. */
    bool integrationGesetzt = false;
    std::uint32_t integrationSamples = 0;
};

/** Prueft und kopiert einen FeatureBatch in einem Zug.

    Bei Vertragsverletzung bleibt `aus` leer und `verstoesse` nennt dieselbe
    kanonische Menge wie `pruefe()`. */
bool lese (const uint8_t* puffer, size_t laenge,
           std::vector<Empfangsframe>& aus,
           juce::Array<Verstoss>& verstoesse);

} // namespace nakama::telemetrie
