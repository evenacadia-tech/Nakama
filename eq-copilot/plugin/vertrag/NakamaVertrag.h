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

#include <cstddef>

namespace nakama::vertrag
{

/** Gemeinsame Obergrenze des sprachuebergreifenden JSON-/DTO-Textriegels.
    Der aktuelle Pipe-Framer ist mit 256 KiB enger; diese Grenze gilt auch fuer
    direkte Datei-/DTO-Aufrufer und verhindert size_t->int-Drift in C++. */
inline constexpr size_t kMaxDokumentBytes = 16u * 1024u * 1024u;

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

/** Betragsgrenze der Gleitkommazahlen des Vertrags: |x| < 1e308. */
constexpr int dezGrenze = 308;

/** Prueft den ROHTEXT eines v3-Dokuments, BEVOR ihn ein Parser sieht.

    Warum vor dem Parser und nicht als Schemaregel: T2-Runde 1 hat gemessen,
    dass JUCEs `parseNumber` `intValue * 10 + digit` in einem `int64` OHNE
    Bereichspruefung akkumuliert (juce_JSON.cpp). `18446744073709552016`
    kommt hier als **400** an. Ein `maximum: 400` im Schema wuerde also
    anstandslos passieren, waehrend dieselbe Datei auf der Rust-Seite faellt -
    der Wert ist beim Ankommen bereits verfaelscht. Der einzige Ort, an dem
    alle drei Beine dasselbe sehen koennen, ist der Text.

    ACHT Regeln, jede gegen eine GEMESSENE Abweichung zwischen den Beinen:

      1. keine fuehrende Null (`091`) - JUCE liest 91, RFC 8259 verbietet es;
      2. mathematische Ganzzahlen nur innerhalb +/-(2^53-1), echte Brueche
         mit hoechstens 15 signifikanten Dezimalziffern;
      3. Zahlen betragsmaessig unter 1e308, und ein `e` braucht Ziffern -
         beides AUS DEM LITERAL gerechnet, nie ueber getDoubleValue();
      4. genau vier ASCII-Hexziffern in einem u-Escape;
      5. kein NUL-Escape in einer Zeichenkette - juce::String ist
         nullterminiert und bricht dort ab, serde_json und Python nehmen an;
      6. keine einsamen Surrogate - hier lehnen beide eigenen Engines ab und
         nur das Referenzbein nimmt an;
      7. kein leerer Objektschluessel - JUCE lehnt ihn ab, in einem additiven
         Objekt haette serde_json ihn akzeptiert;
      8. auf Byteebene (textriegelBytes): hoechstens 16 MiB, kein BOM,
         kein rohes NUL und strikt gueltiges UTF-8.

    ZU REGEL 3, teuer bezahlt in T2-Runde 2: die erste Fassung fragte hier
    `lit.getDoubleValue()` - also GENAU den Leser, gegen dessen Ueberlauf der
    Riegel schuetzen soll. `juce_CharacterFunctions.h` akkumuliert den
    Exponenten in einem `int` ohne Schranke und prueft erst DANACH gegen
    max_exponent10; `1e4294967296` kam hier als 1.0 an, waehrend Rust und
    Python `inf` lasen. Ein Riegel darf nie die Bibliothek befragen, gegen
    deren Verhalten er schuetzt.

    Die Faelle stehen NICHT hier, sondern in
    eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json - drei handgepflegte Kopien
    waren in T2-Runde 2 auf 31, 32 und 33 Faelle auseinandergelaufen.

    Gezaehlt wird in CODEPUNKTEN, damit die Positionsangabe in allen drei
    Beinen dieselbe ist.

    @returns true, wenn der Text sauber ist; sonst false mit gesetztem `fehler`.
*/
bool textriegel (const juce::String& text, juce::String& fehler,
                 bool schemaGanzzahlSichern = true);

/** Derselbe Riegel auf BYTE-Ebene - so, wie ein Dokument wirklich ankommt.

    Vier Regeln lassen sich nur hier ausdruecken: maximale Dokumentgroesse,
    kein BOM, kein rohes NUL und gueltiges UTF-8.

      * BOM. RFC 8259 §8.1: serde_json und Pythons json lehnen ein BOM ab,
        JUCEs loadFileAsString streift es und parst weiter.
      * Kaputtes UTF-8. Gemessen liefen die drei Beine hier voellig
         auseinander: das Python-Bein warf eine ungefangene Ausnahme, das
         Rust-Bein panickte beim Lesen, und JUCE ersetzte das Byte STILL.
      * Rohes NUL. JUCE beendet seine UTF-8-Pruefung dort, die anderen Beine
        beurteilen die volle Bytefolge.

    @returns true, wenn der Puffer sauber ist; sonst false mit gesetztem `fehler`.
*/
bool textriegelBytes (const void* daten, size_t laenge, juce::String& fehler,
                      bool schemaGanzzahlSichern = true);

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
