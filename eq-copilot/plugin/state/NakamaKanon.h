#pragma once

/*  RFC-8785-Kanonisierung (JCS) und SHA-256 - der `state_hash` des Entwurfs
    (§53.8: "state_hash umfasst ausschliesslich das validierte DSP-DTO in
    RFC-8785-kanonischem JSON").

    Warum eine eigene Wertklasse statt juce::var: der Kanon braucht drei
    Dinge, die `var` nicht garantiert - eine Zahl ist IMMER ein double
    (I-JSON), ein Objekt darf keinen doppelten Schluessel tragen, und die
    Serialisierung ist die aus ECMA-262 `Number::toString`, nicht die von
    juce::JSON. Die Klasse ist absichtlich klein; sie traegt nur, was ein
    DSP-DTO und die Cross-Language-Fixtures brauchen.

    Drei Beine messen gegen dieselben Fixtures (eq-copilot/fixtures/state/):
    dieses hier, `rfc8785` (Python) und `serde_json_canonicalizer` (Rust).

    Nicht fuer den Audiothread: allokiert und wandelt Zeichenketten.
*/

#include <juce_core/juce_core.h>
#include <utility>
#include <vector>

namespace nakama::kanon
{

/** Ein JSON-Wert mit I-JSON-Semantik (Zahl = binary64). */
struct Wert
{
    enum class Art { null, boolean, zahl, text, liste, objekt };

    Art art = Art::null;
    bool b = false;
    double zahl = 0.0;
    juce::String text;
    std::vector<Wert> liste;
    /** Objekt als zwei parallele Vektoren (ein `std::pair<String, Wert>` waere
        ein Paar mit noch unvollstaendigem Typ). Einfuegereihenfolge; der Kanon
        sortiert selbst. Doppelte Schluessel sind hier unzulaessig -
        `objektSetze` weist sie ab. */
    std::vector<juce::String> objektSchluessel;
    std::vector<Wert>         objektWerte;

    static Wert null()                          { return {}; }
    static Wert boolean (bool w)                { Wert x; x.art = Art::boolean; x.b = w; return x; }
    static Wert nummer (double w)               { Wert x; x.art = Art::zahl; x.zahl = w; return x; }
    static Wert zeichen (const juce::String& t) { Wert x; x.art = Art::text; x.text = t; return x; }
    static Wert leereListe()                    { Wert x; x.art = Art::liste; return x; }
    static Wert leeresObjekt()                  { Wert x; x.art = Art::objekt; return x; }

    /** Haengt ein Paar an; false, wenn der Schluessel schon da ist. */
    bool objektSetze (const juce::String& schluessel, Wert wert);
};

/** ECMA-262 6.1.6.1.20 `Number::toString (x, 10)` - die Zahlenform von RFC 8785
    §3.2.2.3. Kuerzeste round-trip-exakte Ziffernfolge (std::to_chars), dann
    die Positions-/Exponentregeln der Norm: n in [-5, 21] ⇒ Positionsschreibweise,
    sonst `d.ddde+X`; ±0 ⇒ "0".
    @returns false fuer NaN und ±Infinity (in JSON nicht darstellbar). */
bool zahlAlsEs6 (double x, juce::String& aus);

/** Kanonisiert nach RFC 8785 in UTF-8: Schluessel nach UTF-16-Code-Units
    sortiert (rekursiv, Arrays behalten ihre Ordnung), Strings mit den
    kurzen Escapes `\b \t \n \f \r \" \\`, sonst `\u00xx` (klein) nur fuer
    U+0000..U+001F, kein Whitespace.
    @returns false (mit `fehler`) bei nichtendlicher Zahl. */
bool kanonisiere (const Wert& w, juce::MemoryBlock& utf8, juce::String& fehler);

/** SHA-256 als 64 Hex-Kleinbuchstaben - das Format von v3 `state_report.state_hash`. */
juce::String sha256Hex (const void* daten, size_t laenge);

/** Liest JSON-Text (RFC 8259) in einen Wert - der EIGENE Leser des Kanons.

    Warum nicht juce::JSON::parse: gemessen am Fixture-Korpus (22.08.2026)
    liest JUCEs `readDoubleValue` `5e-324` und `0.000000000000000000000000001`
    als 0 (Subnormale werden geflusht, Nachkomma-Nullen verrechnet) und
    verweigert den leeren Objektschluessel `""`, den RFC 8785 erlaubt. Fuer
    einen Hash, den Rust (serde_json) und Python auf demselben Text bilden,
    darf der Zahlenleser nicht "ungefaehr" sein. Zahlen kommen hier aus
    std::from_chars (korrekt gerundet, Subnormale inklusive), ein doppelter
    Schluessel ist ein eigener Fehler ("doppelter Schluessel: <k>"), ein
    einsames Surrogat und ein rohes Steuerzeichen in einer Zeichenkette sind
    Fehler (RFC 8259 §7), die Verschachtelungstiefe ist auf 64 begrenzt.
    Ein Zahlenliteral, das binary64 nicht tragen kann (ueber-/unterlaeuft),
    ist ein Fehler "Zahl nicht darstellbar" - kein stilles 0 oder inf. */
bool lies (const juce::String& jsonText, Wert& aus, juce::String& fehler);

} // namespace nakama::kanon
