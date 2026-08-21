#pragma once

/*  C++-Bein des v3-Vertrags (SONDE-005a).

    Gegenstueck zu broker/src/vertrag.rs. Beide implementieren GENAU die
    Teilmenge aus eq-copilot/schemas/v3/README.md und muessen jedes Fixture
    des Korpus gleich klassifizieren - Urteil UND Verletzungsmenge.

    Warum nicht handgeschriebene Parser je Nachricht: v3 hat 17 Familien und
    41 Definitionen. Handarbeit waere rund tausend Zeilen gleichfoermiger
    Code, der bei jeder Schemaaenderung nachgezogen werden muesste - und
    genau dort entsteht Drift, die niemand bemerkt. Eine Engine, die das
    committete Schema LIEST, kann nicht hinter ihm zurueckbleiben.

    Ab SONDE-010 ist das die Eingangspruefung des C++-IPC-Clients. Bis dahin
    haelt sie den Vertrag ehrlich.
*/

#include <juce_core/juce_core.h>

namespace nakama::vertrag
{

/** Eine einzelne Vertragsverletzung.

    `schema` ist der AUFGELOESTE Pfad - ueber `$ref` hinweg. Sonst haetten die
    beiden Engines fuer denselben Fehler zwei verschiedene Texte, je nachdem
    ob sie die Referenz vor oder nach dem Bilden des Pfades aufloesen.
*/
struct Verletzung
{
    juce::String instanz;      ///< JSON-Pointer in die Daten; "" ist die Wurzel
    juce::String schema;       ///< aufgeloester Schemapfad
    juce::String schluessel;   ///< das verletzte Schluesselwort

    bool operator== (const Verletzung& a) const noexcept;
    /** Kanonische Ordnung (instanz, schema, schluessel) - BYTEWEISE ueber
        UTF-8, damit sie mit Rusts `String`-Ordnung und Pythons
        Codepunkt-Ordnung zusammenfaellt. */
    bool operator<  (const Verletzung& a) const noexcept;
};

class Schema
{
public:
    /** Laedt das Schema und prueft es gegen die Engine-Teilmenge.

        Ein Schluesselwort oder Muster ausserhalb der Teilmenge ist ein
        FEHLER, kein zu ignorierender Zusatz. JSON Schema uebergeht
        Unbekanntes absichtlich - genau dadurch verschwaende eine spaeter
        ergaenzte Einschraenkung hier still, waehrend das Referenzbein sie
        weiter durchsetzt.

        @returns false und setzt `fehler`, wenn das Schema die Teilmenge
                 verlaesst.
    */
    static bool laden (const juce::var& wurzel, Schema& ziel, juce::String& fehler);

    /** Alle Verletzungen des gewaehlten Zweiges, kanonisch sortiert und
        doppelfrei. */
    juce::Array<Verletzung> pruefe (const juce::var& daten) const;

    bool gueltig (const juce::var& daten) const  { return pruefe (daten).isEmpty(); }

private:
    juce::var wurzel;
};

} // namespace nakama::vertrag
