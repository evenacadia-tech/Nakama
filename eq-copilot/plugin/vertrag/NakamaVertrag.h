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

/** Groesste ganze Zahl, die binary64 noch exakt traegt: 2^53 - 1. */
constexpr juce::int64 sichereGanzzahl = 9007199254740991LL;

/** Prueft den ROHTEXT eines v3-Dokuments, BEVOR ihn ein Parser sieht.

    Warum vor dem Parser und nicht als Schemaregel: T2-Runde 1 hat gemessen,
    dass JUCEs `parseNumber` `intValue * 10 + digit` in einem `int64` OHNE
    Bereichspruefung akkumuliert (juce_JSON.cpp). `18446744073709552016`
    kommt hier als **400** an. Ein `maximum: 400` im Schema wuerde also
    anstandslos passieren, waehrend dieselbe Datei auf der Rust-Seite faellt -
    der Wert ist beim Ankommen bereits verfaelscht. Der einzige Ort, an dem
    alle drei Beine dasselbe sehen koennen, ist der Text.

    Sechs Regeln, jede gegen eine GEMESSENE Abweichung zwischen den Beinen:

      1. keine fuehrende Null (`091`) - JUCE liest 91, RFC 8259 verbietet es;
      2. Ganzzahlen nur innerhalb +/-(2^53-1);
      3. Gleitkommaliterale muessen endlich sein (`1e400` ist es nicht);
      4. kein NUL-Escape in einer Zeichenkette - juce::String ist
         nullterminiert und bricht dort ab, serde_json und Python nehmen an;
      5. keine einsamen Surrogate - hier lehnen beide eigenen Engines ab und
         nur das Referenzbein nimmt an;
      6. kein leerer Objektschluessel - JUCE lehnt ihn ab, in einem additiven
         Objekt haette serde_json ihn akzeptiert.

    Gezaehlt wird in CODEPUNKTEN, damit die Positionsangabe in allen drei
    Beinen dieselbe ist.

    @returns true, wenn der Text sauber ist; sonst false mit gesetztem `fehler`.
*/
bool textriegel (const juce::String& text, juce::String& fehler);

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
