#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 — keine JucePlugin_*-Konstante im Kern
#include "NakamaVertrag.h"
#include "NakamaUtf8.h"

#include <cmath>
#include <cstring>
#include <set>
#include <string>
#include <vector>

namespace nakama::vertrag
{
namespace
{

/*  Genau die Liste aus schemas/v3/README.md. Wer sie erweitert, erweitert auch
    broker/src/vertrag.rs und tools/eq-copilot/pruefe_v3_vertrag.py - sonst
    faellt eine der drei Pruefungen. */
const char* const kSchluesselwoerter[] = {
    "$ref", "type", "const", "enum", "required", "properties",
    "additionalProperties", "maxProperties", "minimum", "maximum",
    "exclusiveMinimum", "exclusiveMaximum", "minLength", "maxLength",
    "pattern", "items", "minItems", "maxItems", "oneOf",
    "x-nakama-discriminator"
};

const char* const kAnmerkungen[] = {
    "$schema", "$id", "title", "description", "$comment", "$defs"
};

bool enthaelt (const char* const* liste, int anzahl, const juce::String& name)
{
    for (int i = 0; i < anzahl; ++i)
        if (name == liste[i])
            return true;
    return false;
}

bool istSchluesselwort (const juce::String& n)
{
    return enthaelt (kSchluesselwoerter, juce::numElementsInArray (kSchluesselwoerter), n);
}

bool istAnmerkung (const juce::String& n)
{
    return enthaelt (kAnmerkungen, juce::numElementsInArray (kAnmerkungen), n);
}

bool hexKette (const juce::String& wert, int laenge)
{
    if (wert.length() != laenge)
        return false;
    for (auto z = wert.getCharPointer(); ! z.isEmpty(); ++z)
    {
        const auto c = *z;
        const bool ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        if (! ok)
            return false;
    }
    return true;
}

bool istBase64Zeichen (juce::juce_wchar c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
        || (c >= '0' && c <= '9') || c == '+' || c == '/';
}

/*  `alphabet` Zeichen aus dem Base64-Alphabet, dann EIN Zeichen aus `schluss`,
    dann `fuell` Gleichheitszeichen.

    `schluss` ist nicht Kosmetik: das letzte Alphabetzeichen einer gepolsterten
    Base64-Kette traegt weniger als sechs echte Bits, der Rest sind FUELLBITS.
    Ein Restbyte laesst vier frei, zwei Restbytes zwei. Nur die Zeichen, in
    denen diese Bits null sind, sind kanonisch — sonst erzeugen zwei Sender
    fuer dieselbe Bitmap zwei verschiedene Zeichenketten, und ein
    Bytevergleich waere keine Aussage mehr. */
bool base64Kette (const juce::String& wert, int alphabet, const char* schluss, int fuell)
{
    if (wert.length() != alphabet + 1 + fuell)
        return false;
    for (int i = 0; i < alphabet; ++i)
        if (! istBase64Zeichen (wert[i]))
            return false;
    if (juce::String (schluss).indexOfChar (wert[alphabet]) < 0)
        return false;
    for (int i = 0; i < fuell; ++i)
        if (wert[alphabet + 1 + i] != '=')
            return false;
    return true;
}

bool istUnicodeWhitespace (juce::juce_wchar c) noexcept
{
    // Unicode White_Space explizit statt einer Bibliotheksheuristik: Rust,
    // C++ und das jsonschema-Referenzbein beurteilen dieselben Codepoints.
    return (c >= 0x0009 && c <= 0x000d) || c == 0x0020 || c == 0x0085
        || c == 0x00a0 || c == 0x1680 || (c >= 0x2000 && c <= 0x200a)
        || c == 0x2028 || c == 0x2029 || c == 0x202f || c == 0x205f
        || c == 0x3000;
}

bool hostBusNamePasst (const juce::String& wert) noexcept
{
    bool hatNichtWhitespace = false;
    for (auto z = wert.getCharPointer(); ! z.isEmpty(); ++z)
    {
        const auto c = *z;
        if ((c >= 0x0000 && c <= 0x001f) || (c >= 0x007f && c <= 0x009f))
            return false;
        hatNichtWhitespace = hatNichtWhitespace || ! istUnicodeWhitespace (c);
    }
    return hatNichtWhitespace;
}

/*  Geschlossene Mustertabelle. Regex ist zwischen Python, C++ und Rust nicht
    in jeder Ecke gleich; deshalb gibt es hier keine Regex-Auswertung, sondern
    benannte Muster. Ein unbekanntes Muster bricht den Ladevorgang.
    -1 = Muster unbekannt, 0 = passt nicht, 1 = passt.

    🔑 Jedes dieser Muster steht im Schema NEBEN einem festen minLength ==
    maxLength. Das ist keine Doppelung: Pythons `re` laesst `$` auch VOR einem
    abschliessenden Zeilenumbruch passen, diese Handschleifen nicht. Weil die
    Laengenschranke den Umbruch schon faengt, kommen alle drei Beine trotzdem
    zum selben URTEIL — die Schranke ist der Grund, warum die Ankersemantik
    hier folgenlos bleibt (gemessen in der G1-Nacharbeit, Fixture
    `state-hash-mit-umbruch`). */
int musterPasst (const juce::String& muster, const juce::String& wert)
{
    if (muster == "^[0-9a-f]{32}$")                         return hexKette (wert, 32) ? 1 : 0;
    if (muster == "^[0-9a-f]{64}$")                         return hexKette (wert, 64) ? 1 : 0;
    if (muster == "^[A-Za-z0-9+/]{37}[AQgw]==$")            return base64Kette (wert, 37, "AQgw", 2) ? 1 : 0;
    if (muster == "^[A-Za-z0-9+/]{10}[AEIMQUYcgkosw048]=$") return base64Kette (wert, 10, "AEIMQUYcgkosw048", 1) ? 1 : 0;
    if (muster == R"(^(?![\s\S]*[\u0000-\u001F\u007F-\u009F])(?=[\s\S]*\S)[\s\S]+$)")
        return hostBusNamePasst (wert) ? 1 : 0;
    return -1;
}

// ------------------------------------------------------------------ Zugriff

/*  JUCEs JSON-Leser bildet `null` auf ein leeres `var` ab - und ein FEHLENDES
    Property liefert ebenfalls ein leeres `var`. Die beiden duerfen nie
    verwechselt werden: `null` ist ein Typ, "fehlt" ist eine
    required-Verletzung. Deshalb geht jeder Zugriff ueber hatEigenschaft(). */
bool istObjekt (const juce::var& v)          { return v.getDynamicObject() != nullptr; }

const juce::NamedValueSet* eigenschaften (const juce::var& v)
{
    if (auto* o = v.getDynamicObject())
        return &o->getProperties();
    return nullptr;
}

bool hatEigenschaft (const juce::var& v, const juce::String& name)
{
    auto* p = eigenschaften (v);
    return p != nullptr && p->contains (juce::Identifier (name));
}

juce::var hole (const juce::var& v, const juce::String& name)
{
    auto* p = eigenschaften (v);
    return p != nullptr ? (*p) [juce::Identifier (name)] : juce::var();
}

bool istZahl (const juce::var& v)
{
    // isBool() zuerst: `true` waere sonst ueber operator double() eine 1.
    return ! v.isBool() && (v.isInt() || v.isInt64() || v.isDouble());
}

juce::String zeigerTeil (const juce::String& pfad, const juce::String& teil)
{
    return pfad + "/" + teil.replace ("~", "~0").replace ("/", "~1");
}

// ------------------------------------------------------------------ Vergleiche

/*  JSON-Gleichheit mit ZAHLENSEMANTIK: 3 und 3.0 sind derselbe Wert. `var`
    unterscheidet sie strukturell, JSON Schema nicht. */
bool gleich (const juce::var& a, const juce::var& b)
{
    if (istZahl (a) && istZahl (b))
        return static_cast<double> (a) == static_cast<double> (b);
    if (a.isBool() || b.isBool())
        return a.isBool() && b.isBool() && static_cast<bool> (a) == static_cast<bool> (b);
    if (a.isString() || b.isString())
        return a.isString() && b.isString() && a.toString() == b.toString();
    if (a.isVoid() || b.isVoid())
        return a.isVoid() && b.isVoid();
    if (a.isArray() && b.isArray())
    {
        auto* x = a.getArray();
        auto* y = b.getArray();
        if (x->size() != y->size())
            return false;
        for (int i = 0; i < x->size(); ++i)
            if (! gleich ((*x)[i], (*y)[i]))
                return false;
        return true;
    }
    // OBJEKTE. Bis T2-Runde 1 fiel dieser Fall auf `return false` durch,
    // waehrend die Rust-Seite zwei Objekte korrekt verglich - eine latente
    // Divergenz. Heute unerreichbar, weil jeder `const`- und `enum`-Wert im
    // Schema ein Skalar ist; sie zu schliessen kostet zehn Zeilen, sie stehen
    // zu lassen kostet irgendwann einen Nachmittag.
    //
    // Der Vergleich laeuft ueber Schluesselzugriff und ist damit in beiden
    // Sprachen reihenfolgeunabhaengig - juce::NamedValueSet und serde_json::Map
    // ordnen verschieden, und eine positionsweise Pruefung waere genau der
    // Unterschied, den niemand bemerkt.
    if (auto* x = a.getDynamicObject())
    {
        auto* y = b.getDynamicObject();
        if (y == nullptr)
            return false;
        const auto& xs = x->getProperties();
        const auto& ys = y->getProperties();
        if (xs.size() != ys.size())
            return false;
        for (int i = 0; i < xs.size(); ++i)
        {
            const auto name = xs.getName (i);
            if (! ys.contains (name) || ! gleich (xs.getValueAt (i), ys[name]))
                return false;
        }
        return true;
    }
    return false;
}

bool typPasst (const juce::String& name, const juce::var& w)
{
    if (name == "object")  return istObjekt (w);
    if (name == "array")   return w.isArray();
    if (name == "string")  return w.isString();
    if (name == "boolean") return w.isBool();
    if (name == "null")    return w.isVoid();
    if (name == "number")  return istZahl (w);
    if (name == "integer")
    {
        // draft 2020-12: jede Zahl OHNE Nachkommateil ist ein integer, also
        // auch 1.0. Nicht "ist ein int-var" - das waere enger als der Standard.
        if (! istZahl (w))
            return false;
        const auto d = static_cast<double> (w);
        return std::isfinite (d) && std::trunc (d) == d;
    }
    return false;
}

} // namespace

// ------------------------------------------------------------------ Verletzung

bool Verletzung::operator== (const Verletzung& a) const noexcept
{
    return instanz == a.instanz && schema == a.schema && schluessel == a.schluessel;
}

bool Verletzung::operator< (const Verletzung& a) const noexcept
{
    // Byteweise ueber UTF-8: so ordnen Rusts String und - weil UTF-8 die
    // Codepunkt-Ordnung erhaelt - auch Pythons sorted().
    const std::string ai (instanz.toRawUTF8()),    bi (a.instanz.toRawUTF8());
    if (ai != bi) return ai < bi;
    const std::string as (schema.toRawUTF8()),     bs (a.schema.toRawUTF8());
    if (as != bs) return as < bs;
    const std::string ak (schluessel.toRawUTF8()), bk (a.schluessel.toRawUTF8());
    return ak < bk;
}

// ------------------------------------------------------------------ Textriegel

namespace
{

bool istAsciiZiffer (juce::juce_wchar c) noexcept
{
    // NICHT juce::CharacterFunctions::isDigit - die ist auch fuer
    // arabisch-indische Ziffern wahr. T2-Runde 2, Befund BF-4: Pythons
    // str.isdigit() war es ebenfalls, und dieses Bein war es nicht. Drei
    // Ziffernbegriffe waeren drei Grammatiken.
    return c >= '0' && c <= '9';
}

bool istAsciiBuchstabe (juce::juce_wchar c) noexcept
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool istHexziffer (juce::juce_wchar c) noexcept
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

/*  Entscheidet AUS DEM LITERAL, ob eine Zahl im Vertragsbereich liegt.

    T2-Runde 2, Blocker BL-1/BL-2: die erste Fassung fragte hier
    `lit.getDoubleValue()`. Genau das war der Fehler - das ist DERSELBE Leser,
    gegen dessen Ueberlauf der Riegel schuetzen soll.
    `juce_CharacterFunctions.h` akkumuliert den Exponenten in einem `int` OHNE
    Schranke (`exponent = (exponent * 10) + digit`), und der
    `max_exponent10`-Riegel laeuft DANACH; `1e4294967296` kam hier als 1.0 an,
    waehrend Rust und Python `inf` lasen. Der zweite Zweig (`extraExponent`)
    hat gar keinen Riegel, und `writeExponentDigits` schreibt immer genau DREI
    Ziffern - bei 1018 Vorkommastellen entsteht ':00' statt eines Exponenten,
    strtod bricht dort ab, und JUCE liest 1e17.

    Die Lehre, die ueber diesen Fall hinausgeht: EIN RIEGEL DARF NIE DIE
    BIBLIOTHEK BEFRAGEN, GEGEN DEREN VERHALTEN ER SCHUETZT. Die erste
    Ganzzahlregel rechnete zwar aus dem Literal, erfasste aber nur die Form
    ohne Punkt/Exponent; heute werden Wert, Ganzzahligkeit und Praezision fuer
    alle Schreibweisen lexikalisch bestimmt. Die Endlichkeitsregel war
    delegiert und hat ebenfalls nicht gehalten.

    Hier wird deshalb nur mit kleinen ganzen Zahlen gerechnet, ohne jede
    Gleitkommaoperation.
*/
bool zahlPruefen (const juce::String& ganz, const juce::String& bruch,
                  const juce::String& expZiffern, bool expNegativ,
                  const juce::String& lit, juce::String& fehler,
                  bool schemaGanzzahlSichern)
{
    if (bruch.isEmpty() && expZiffern.isEmpty())
    {
        const bool zuGross = ganz.length() > 16
            || (ganz.length() == 16 && ganz.getLargeIntValue() > sichereGanzzahl);
        if (zuGross)
        {
            fehler = "Ganzzahl ausserhalb 2^53-1: " + lit;
            return false;
        }
        return true;
    }

    // Der Exponent selbst: mehr als drei Ziffern liegen schon ausserhalb, und
    // so wird er auch nie gross genug, um irgendwo ueberzulaufen.
    auto ohneNull = expZiffern;
    while (ohneNull.startsWithChar ('0'))
        ohneNull = ohneNull.substring (1);
    if (ohneNull.length() > 3)
    {
        fehler = "Exponent ausserhalb +/-" + juce::String (dezGrenze) + ": " + lit.substring (0, 40);
        return false;
    }
    auto exp = static_cast<int> (ohneNull.getLargeIntValue());
    if (expNegativ)
        exp = -exp;

    const auto alle = ganz + bruch;
    int fuehrende = 0;
    while (fuehrende < alle.length() && alle[fuehrende] == '0')
        ++fuehrende;
    if (fuehrende == alle.length())
        return true;                      // der Wert ist exakt 0

    // Die Endlichkeitsgrenze hat Vorrang vor der engeren Ganzzahlregel: so
    // bleibt ein 1e308-Ueberlauf in allen drei Beinen als Zahlenbereichsfehler
    // klassifiziert, statt zufaellig als Praezisionsfehler.
    const auto dez = (ganz.length() - fuehrende - 1) + exp;
    if (dez >= dezGrenze || dez <= -dezGrenze)
    {
        fehler = "Zahl ausserhalb +/-1e" + juce::String (dezGrenze) + ": " + lit.substring (0, 40);
        return false;
    }

    // JSON Schema meint mit `integer` den mathematischen Wert, nicht nur ein
    // Literal ohne Punkt: auch 5.0 und 5e0 sind ganze Zahlen. Deshalb muss
    // die 2^53-Regel alle exakt ganzzahligen Schreibweisen erfassen, bevor
    // ein binary64-Parser z. B. 9007199254740992.0 still rundet.
    const auto skala = exp - bruch.length(); // alle * 10^skala
    bool istGanzzahl = false;
    bool ganzzahlZuGross = false;
    if (skala >= 0)
    {
        istGanzzahl = true;
        const auto signifikant = alle.substring (fuehrende);
        const auto stellen = signifikant.length() + skala;
        if (stellen > 16)
        {
            ganzzahlZuGross = true;
        }
        else if (stellen == 16)
        {
            auto normalisiert = signifikant;
            for (int i = 0; i < skala; ++i)
                normalisiert += '0';
            ganzzahlZuGross = normalisiert.compare ("9007199254740991") > 0;
        }
    }
    else
    {
        const auto abzuschneiden = -skala;
        istGanzzahl = abzuschneiden <= alle.length();
        for (int i = alle.length() - abzuschneiden;
             istGanzzahl && i < alle.length(); ++i)
            istGanzzahl = alle[i] == '0';

        if (istGanzzahl)
        {
            auto normalisiert = alle.substring (0, alle.length() - abzuschneiden);
            while (normalisiert.startsWithChar ('0'))
                normalisiert = normalisiert.substring (1);
            ganzzahlZuGross = normalisiert.length() > 16
                || (normalisiert.length() == 16
                    && normalisiert.compare ("9007199254740991") > 0);
        }
    }
    if (ganzzahlZuGross)
    {
        fehler = "Ganzzahl ausserhalb 2^53-1: " + lit;
        return false;
    }

    // Nichtganzzahlige Dezimalwerte mit mehr als 15 signifikanten Ziffern
    // koennen schon beim binary64-Lesen auf eine Ganzzahl kippen. Dann wuerde
    // `type: integer` einen im Rohtext gebrochenen Wert akzeptieren. Exakte
    // Ganzzahlen haben oben ihre eigene, weitere 2^53-Grenze.
    auto signifikanteStellen = alle.length() - fuehrende;
    while (signifikanteStellen > 0
           && alle[fuehrende + signifikanteStellen - 1] == '0')
        --signifikanteStellen;
    if (schemaGanzzahlSichern && ! istGanzzahl && signifikanteStellen > 15)
    {
        fehler = "Zahl mit mehr als 15 signifikanten Dezimalziffern: " + lit.substring (0, 40);
        return false;
    }

    return true;
}

} // namespace

bool textriegelBytes (const void* daten, size_t laenge, juce::String& fehler,
                      bool schemaGanzzahlSichern)
{
    const auto* bytes = static_cast<const unsigned char*> (daten);
    if (daten == nullptr)
    {
        fehler = "kein Puffer";
        return false;
    }
    if (laenge > kMaxDokumentBytes)
    {
        fehler = "Dokument zu gross";
        return false;
    }
    if (laenge >= 3 && bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF)
    {
        fehler = "BOM am Dokumentanfang";
        return false;
    }
    // JUCEs Stringaufbau endet am ersten NUL. Ohne diesen Byte-Riegel wuerde
    // C++ nur einen gueltigen JSON-Praefix
    // pruefen und alles dahinter ignorieren, waehrend Rust und Python die
    // gesamte Bytefolge beurteilen. Ein NUL im JSON muss als `\u0000`
    // escaped sein; roh ist es ein ungueltiges Steuerzeichen.
    if (std::memchr (daten, 0, laenge) != nullptr)
    {
        fehler = "rohes NUL im Dokument";
        return false;
    }
    if (! utf8::istGueltig (daten, laenge))
    {
        fehler = "kein gueltiges UTF-8";
        return false;
    }
    return textriegel (juce::String::fromUTF8 (static_cast<const char*> (daten),
                                               static_cast<int> (laenge)), fehler,
                       schemaGanzzahlSichern);
}

bool textriegel (const juce::String& text, juce::String& fehler,
                 bool schemaGanzzahlSichern)
{
    // Codepunkte statt Bytes: die Positionsangabe soll in allen drei Beinen
    // dieselbe sein, und juce::juce_wchar ist UTF-32. Der Namensraum muss
    // ausgeschrieben werden: der Typ lebt in `juce`, diese Funktion in
    // `nakama::vertrag`.
    std::vector<juce::juce_wchar> z;
    z.reserve (static_cast<size_t> (text.length()));
    for (auto p = text.getCharPointer(); ! p.isEmpty(); ++p)
        z.push_back (*p);

    const auto n = z.size();
    const auto teil = [&z] (size_t von, size_t bis)
    {
        juce::String s;
        for (auto k = von; k < bis && k < z.size(); ++k)
            s += juce::String::charToString (z[k]);
        return s;
    };

    size_t i = 0;
    while (i < n)
    {
        const auto c = z[i];

        if (c == '"')
        {
            size_t j = i + 1;
            juce::int64 hoch = -1;     // -1 = kein offenes hohes Surrogat
            size_t ende = 0;
            for (;;)
            {
                if (j >= n) { fehler = "unbeendete Zeichenkette"; return false; }
                const auto d = z[j];

                if (d == '\\')
                {
                    if (j + 5 < n && z[j + 1] == 'u')
                    {
                        // GENAU vier ASCII-Hexziffern (T2-Runde 2, BF-2/BF-3):
                        // Pythons int(roh,16) naehme Vorzeichen, Leerzeichen,
                        // 0x-Praefix, Ziffern-Trenner und arabisch-indische
                        // Ziffern; Rusts from_str_radix das Vorzeichen. Drei
                        // Hex-Grammatiken waeren drei Urteile.
                        juce::int64 cp = 0;
                        bool sauber = true;
                        for (int k = 0; k < 4; ++k)
                        {
                            const auto hz = z[j + 2 + static_cast<size_t> (k)];
                            if (! istHexziffer (hz)) { sauber = false; break; }
                            const int wert = (hz >= '0' && hz <= '9') ? hz - '0'
                                           : (hz >= 'a' && hz <= 'f') ? hz - 'a' + 10
                                                                      : hz - 'A' + 10;
                            cp = cp * 16 + wert;
                        }
                        if (! sauber)
                        {
                            fehler = "kein 4-stelliges Hex-Escape an Position " + juce::String ((int) j);
                            return false;
                        }
                        if (cp == 0)
                        {
                            fehler = "NUL-Escape in Zeichenkette an Position " + juce::String ((int) j);
                            return false;
                        }
                        const bool tief = cp >= 0xDC00 && cp <= 0xDFFF;
                        if (tief && hoch < 0)
                        {
                            fehler = "einsames tiefes Surrogat an Position " + juce::String ((int) j);
                            return false;
                        }
                        if (hoch >= 0 && ! tief)
                        {
                            fehler = "hohes Surrogat ohne Paar an Position " + juce::String ((int) j);
                            return false;
                        }
                        hoch = (cp >= 0xD800 && cp <= 0xDBFF) ? cp : -1;
                        j += 6;
                        continue;
                    }
                    if (hoch >= 0)
                    {
                        fehler = "hohes Surrogat ohne Paar an Position " + juce::String ((int) j);
                        return false;
                    }
                    j += 2;
                    continue;
                }

                if (hoch >= 0)
                {
                    fehler = "hohes Surrogat ohne Paar an Position " + juce::String ((int) j);
                    return false;
                }
                if (d == '"') { ende = j; break; }
                if (d < 0x20)
                {
                    fehler = "rohes Steuerzeichen an Position " + juce::String ((int) j);
                    return false;
                }
                ++j;
            }

            if (hoch >= 0) { fehler = "hohes Surrogat ohne Paar am Zeichenkettenende"; return false; }

            const bool leer = (ende == i + 1);
            size_t k = ende + 1;
            while (k < n && (z[k] == ' ' || z[k] == '\t' || z[k] == '\r' || z[k] == '\n'))
                ++k;
            if (leer && k < n && z[k] == ':')
            {
                fehler = "leerer Objektschluessel an Position " + juce::String ((int) i);
                return false;
            }
            i = ende + 1;
            continue;
        }

        // JSON kennt ausserhalb von Zeichenketten genau drei alphabetische
        // Literale. Python akzeptiert zusaetzlich NaN/Infinity, JUCE und
        // serde_json nicht. Das optionale Minus gehoert zur Position und zum
        // gemeldeten Literal, damit -Infinity in allen drei Beinen gleich
        // klassifiziert wird.
        const bool vorzeichenVorLiteral = c == '-' && i + 1 < n
                                       && istAsciiBuchstabe (z[i + 1]);
        if (istAsciiBuchstabe (c) || vorzeichenVorLiteral)
        {
            size_t j = i + static_cast<size_t> (c == '-');
            while (j < n && istAsciiBuchstabe (z[j])) ++j;
            const auto literal = teil (i, j);
            if (literal != "true" && literal != "false" && literal != "null")
            {
                fehler = "unbekanntes Literal " + literal
                       + " an Position " + juce::String ((int) i);
                return false;
            }
            i = j;
            continue;
        }

        if (c == '-' || istAsciiZiffer (c))
        {
            size_t j = i;
            if (z[j] == '-') ++j;
            const auto anfang = j;
            while (j < n && istAsciiZiffer (z[j])) ++j;
            const auto ganz = teil (anfang, j);
            if (ganz.isEmpty())
            {
                fehler = "Zahl ohne Ziffern an Position " + juce::String ((int) i);
                return false;
            }
            if (ganz.length() > 1 && ganz[0] == '0')
            {
                fehler = "fuehrende Null in \"" + teil (i, j).substring (0, 20)
                       + "\" an Position " + juce::String ((int) i);
                return false;
            }

            juce::String bruch, expZiffern;
            bool expNegativ = false;

            if (j < n && z[j] == '.')
            {
                ++j;
                const auto a = j;
                while (j < n && istAsciiZiffer (z[j])) ++j;
                bruch = teil (a, j);
                if (bruch.isEmpty())
                {
                    fehler = "Dezimalpunkt ohne Nachkommaziffern an Position " + juce::String ((int) i);
                    return false;
                }
            }
            if (j < n && (z[j] == 'e' || z[j] == 'E'))
            {
                ++j;
                if (j < n && (z[j] == '+' || z[j] == '-'))
                {
                    expNegativ = z[j] == '-';
                    ++j;
                }
                const auto a = j;
                while (j < n && istAsciiZiffer (z[j])) ++j;
                expZiffern = teil (a, j);
                if (expZiffern.isEmpty())
                {
                    // BF-1: getDoubleValue("1e") liefert 1.0.
                    fehler = "Exponent ohne Ziffern an Position " + juce::String ((int) i);
                    return false;
                }
            }

            if (! zahlPruefen (ganz, bruch, expZiffern, expNegativ, teil (i, j), fehler,
                               schemaGanzzahlSichern))
                return false;
            i = j;
            continue;
        }

        ++i;
    }

    fehler.clear();
    return true;
}

// ------------------------------------------------------------------ Ladelauf

namespace
{

/*  Welchen Werttyp verlangt ein Schluesselwort?

    Die Ladepruefung sah bis T2-Runde 1 nur NAMEN. Gemessen: `"maxLength": 5.0`
    wurde von Rust still verworfen (`as_u64()` auf einer Float-Zahl ist None)
    und von C++ durchgesetzt - dieselbe Fehlerklasse wie ein unbekanntes
    Schluesselwort, nur eine Ebene tiefer. Ein Werttyp, den die eine Engine
    lesen kann und die andere nicht, laesst eine Einschraenkung auf einer Seite
    verschwinden.
*/
enum class Werttyp { Beliebig, Text, Zahl, GanzzahlNichtNegativ, Objekt,
                     ArrayNichtLeer, TextArray, Bool, TypAngabe };

Werttyp erwarteterTyp (const juce::String& name)
{
    // `type` ist String ODER nicht-leeres Array von Strings. "beliebig" waere
    // hier ein neues Divergenzloch: bei `"type": 5` laese diese Engine ueber
    // toString() die Zeichenkette "5", die Rust-Engine bekaeme von as_str()
    // ein None - zwei Engines, zwei Urteile.
    if (name == "type")                       return Werttyp::TypAngabe;
    // `const` ist bewusst beliebig: es vergleicht gegen einen Wert, und jeder
    // JSON-Wert ist ein zulaessiges Vergleichsziel.
    if (name == "const")                      return Werttyp::Beliebig;
    if (name == "enum")                       return Werttyp::ArrayNichtLeer;
    if (name == "required")                   return Werttyp::TextArray;
    if (name == "properties" || name == "$defs" || name == "items") return Werttyp::Objekt;
    if (name == "additionalProperties")       return Werttyp::Bool;
    if (name == "maxProperties" || name == "minLength" || name == "maxLength"
        || name == "minItems" || name == "maxItems")                return Werttyp::GanzzahlNichtNegativ;
    if (name == "minimum" || name == "maximum"
        || name == "exclusiveMinimum" || name == "exclusiveMaximum") return Werttyp::Zahl;
    if (name == "pattern" || name == "$ref" || name == "x-nakama-discriminator") return Werttyp::Text;
    if (name == "oneOf")                      return Werttyp::ArrayNichtLeer;
    return Werttyp::Beliebig;
}

bool typPasst (Werttyp erwartet, const juce::var& wert)
{
    switch (erwartet)
    {
        case Werttyp::Beliebig:   return true;
        case Werttyp::Text:       return wert.isString();
        case Werttyp::Bool:       return wert.isBool();
        case Werttyp::Objekt:     return wert.getDynamicObject() != nullptr;
        case Werttyp::Zahl:       return wert.isInt() || wert.isInt64() || wert.isDouble();
        case Werttyp::GanzzahlNichtNegativ:
            // `5.0` ist hier bewusst KEINE Ganzzahl: JSON Schema erlaubt sie,
            // aber die beiden Engines lesen sie verschieden. Wir verlangen die
            // Form, ueber die sie sich nicht streiten koennen.
            return (wert.isInt() || wert.isInt64()) && static_cast<juce::int64> (wert) >= 0;
        case Werttyp::ArrayNichtLeer:
            return wert.getArray() != nullptr && ! wert.getArray()->isEmpty();
        case Werttyp::TypAngabe:
            if (wert.isString())
                return true;
            if (auto* a = wert.getArray())
            {
                if (a->isEmpty())
                    return false;
                for (const auto& e : *a)
                    if (! e.isString())
                        return false;
                return true;
            }
            return false;
        case Werttyp::TextArray:
            if (auto* a = wert.getArray())
            {
                for (const auto& e : *a)
                    if (! e.isString())
                        return false;
                return true;
            }
            return false;
    }
    return true;
}

void pruefeTeilschema (const juce::var& wurzel, const juce::var& knoten,
                       const juce::String& pfad, juce::StringArray& fehler)
{
    auto* props = eigenschaften (knoten);
    if (props == nullptr)
        return;

    for (int i = 0; i < props->size(); ++i)
    {
        const auto name = props->getName (i).toString();
        if (! istSchluesselwort (name) && ! istAnmerkung (name))
        {
            fehler.add ("unbekanntes Schluesselwort " + pfad + "/" + name);
            continue;
        }
        if (istSchluesselwort (name) && ! typPasst (erwarteterTyp (name), props->getValueAt (i)))
            fehler.add ("falscher Werttyp fuer " + pfad + "/" + name);
    }

    if (hatEigenschaft (knoten, "pattern"))
        if (musterPasst (hole (knoten, "pattern").toString(), juce::String()) < 0)
            fehler.add ("unbekanntes Muster " + pfad + ": "
                        + hole (knoten, "pattern").toString());

    if (hatEigenschaft (knoten, "oneOf") && ! hatEigenschaft (knoten, "x-nakama-discriminator"))
        fehler.add ("oneOf ohne x-nakama-discriminator bei " + pfad);

    if (hatEigenschaft (knoten, "$ref"))
    {
        const auto r = hole (knoten, "$ref").toString();
        if (! r.startsWith ("#/$defs/"))
        {
            fehler.add ("nicht-lokale Referenz bei " + pfad + ": " + r);
        }
        else
        {
            // T2-Runde 1: bis hierher wurde nur das PRAEFIX geprueft. Ein
            // haengender $ref liess `aufloesen()` auf den Originalknoten
            // zurueckfallen - und damit blieb der ganze Teilbaum still
            // UNGEPRUEFT. Das ist dieselbe Klasse wie ein uebergangenes
            // Schluesselwort, nur schlimmer: es verschwindet nicht eine
            // Einschraenkung, sondern jede des Zieles.
            const auto name = r.substring (juce::String ("#/$defs/").length());
            if (! hatEigenschaft (hole (wurzel, "$defs"), name))
                fehler.add ("haengende Referenz bei " + pfad + ": " + r + " hat kein Ziel");
        }
    }

    if (hatEigenschaft (knoten, "additionalProperties"))
    {
        const auto a = hole (knoten, "additionalProperties");
        if (a.isBool() && static_cast<bool> (a) && ! hatEigenschaft (knoten, "maxProperties"))
            fehler.add ("additives Objekt " + pfad + " ohne maxProperties");
    }

    for (const auto* zweig : { "properties", "$defs" })
        if (auto* kinder = eigenschaften (hole (knoten, zweig)))
            for (int i = 0; i < kinder->size(); ++i)
                pruefeTeilschema (wurzel, kinder->getValueAt (i),
                                  pfad + "/" + zweig + "/" + kinder->getName (i).toString(),
                                  fehler);

    if (hatEigenschaft (knoten, "items"))
        pruefeTeilschema (wurzel, hole (knoten, "items"), pfad + "/items", fehler);

    if (auto* zweige = hole (knoten, "oneOf").getArray())
        for (int i = 0; i < zweige->size(); ++i)
            pruefeTeilschema (wurzel, (*zweige)[i], pfad + "/oneOf/" + juce::String (i), fehler);
}

} // namespace

bool Schema::laden (const juce::var& wurzel, Schema& ziel, juce::String& fehler)
{
    juce::StringArray f;
    pruefeTeilschema (wurzel, wurzel, "#", f);
    if (! f.isEmpty())
    {
        fehler = f.joinIntoString ("; ");
        return false;
    }
    ziel.wurzel = wurzel;
    fehler.clear();
    return true;
}

// ------------------------------------------------------------------ Pruefwerk

namespace
{

/*  Loest `$ref` auf. Ein Knoten mit `$ref` wird VOLLSTAENDIG ersetzt;
    danebenstehende Anmerkungen ($comment) tragen keine Bedeutung. */
void aufloesen (const juce::var& wurzel, const juce::var& knoten, const juce::String& pfad,
                juce::var& zielKnoten, juce::String& zielPfad)
{
    if (! hatEigenschaft (knoten, "$ref"))
    {
        zielKnoten = knoten;
        zielPfad = pfad;
        return;
    }
    const auto name = hole (knoten, "$ref").toString().substring (juce::String ("#/$defs/").length());
    const auto defs = hole (wurzel, "$defs");
    if (hatEigenschaft (defs, name))
    {
        zielKnoten = hole (defs, name);
        zielPfad = "#/$defs/" + name;
        return;
    }
    zielKnoten = knoten;   // nach der Ladepruefung unerreichbar
    zielPfad = pfad;
}

/*  Welchen Wert der Eigenschaft `disc` legt dieser Zweig fest?

    Normalerweise steht er als properties/<disc>/const. Ein Zweig darf aber
    selbst wieder eine diskriminierte Union sein - `hello` ist genau das: es
    haelt keine eigenen properties, sondern zwei Varianten, die BEIDE
    type:"hello" festlegen. Ohne diesen Abstieg fiele der aeussere
    Discriminator ins Leere und die ganze Familie waere unerreichbar.

    Der Wert gilt nur, wenn ALLE Untervarianten denselben festlegen; sonst
    waere die Zuordnung mehrdeutig. */
bool pointerSegment (const juce::String& roh, juce::String& out)
{
    out.clear();
    for (int i = 0; i < roh.length(); ++i)
    {
        if (roh[i] != '~')
        {
            out += roh[i];
            continue;
        }
        if (++i >= roh.length())
            return false;
        if (roh[i] == '0') out += '~';
        else if (roh[i] == '1') out += '/';
        else return false;
    }
    return true;
}

bool diskriminatorSchema (const juce::var& wurzel, const juce::var& zweig,
                          const juce::String& disc, juce::var& ergebnis)
{
    juce::StringArray segmente;
    if (disc.startsWithChar ('/'))
        segmente.addTokens (disc.substring (1), "/", "");
    else
        segmente.add (disc);

    juce::var aktuell = zweig;
    for (const auto& roh : segmente)
    {
        juce::var aufgeloest; juce::String pfad;
        aufloesen (wurzel, aktuell, "#", aufgeloest, pfad);
        juce::String segment;
        if (! pointerSegment (roh, segment))
            return false;
        const auto props = hole (aufgeloest, "properties");
        if (! hatEigenschaft (props, segment))
            return false;
        aktuell = hole (props, segment);
    }
    juce::String pfad;
    aufloesen (wurzel, aktuell, "#", ergebnis, pfad);
    return true;
}

bool diskriminatorwert (const juce::var& wurzel, const juce::var& zweig,
                        const juce::String& disc, juce::var& ergebnis)
{
    juce::var feld;
    if (diskriminatorSchema (wurzel, zweig, disc, feld)
        && hatEigenschaft (feld, "const"))
    {
        const auto k = hole (feld, "const");
        if (k.isString() || k.isBool())
        {
            ergebnis = k;
            return true;
        }
    }

    auto* unter = hole (zweig, "oneOf").getArray();
    if (unter == nullptr)
        return false;

    bool erster = true;
    juce::var gemeinsam;
    for (int i = 0; i < unter->size(); ++i)
    {
        juce::var ziel; juce::String zpfad;
        aufloesen (wurzel, (*unter)[i], "#", ziel, zpfad);
        juce::var w;
        if (! diskriminatorwert (wurzel, ziel, disc, w))
            return false;
        if (erster) { gemeinsam = w; erster = false; }
        else if (! gleich (gemeinsam, w)) return false;
    }
    if (erster)
        return false;
    ergebnis = gemeinsam;
    return true;
}

void pruefeWert (const juce::var& wurzel, const juce::var& knotenRoh,
                 const juce::String& pfadRoh, const juce::var& daten,
                 const juce::String& instanz, std::vector<Verletzung>& out)
{
    juce::var knoten; juce::String sp;
    aufloesen (wurzel, knotenRoh, pfadRoh, knoten, sp);
    if (! istObjekt (knoten))
        return;

    // --- oneOf mit Discriminator ---------------------------------------
    if (auto* zweige = hole (knoten, "oneOf").getArray())
    {
        const auto disc = hatEigenschaft (knoten, "x-nakama-discriminator")
                            ? hole (knoten, "x-nakama-discriminator").toString()
                            : juce::String ("type");

        juce::var wert;
        bool hatWert = false;
        if (istObjekt (daten))
        {
            if (disc.startsWithChar ('/'))
            {
                juce::var aktuell = daten;
                juce::StringArray segmente;
                segmente.addTokens (disc.substring (1), "/", "");
                hatWert = true;
                for (const auto& roh : segmente)
                {
                    juce::String segment;
                    if (! pointerSegment (roh, segment) || ! istObjekt (aktuell)
                        || ! hatEigenschaft (aktuell, segment))
                    {
                        hatWert = false;
                        break;
                    }
                    aktuell = hole (aktuell, segment);
                }
                if (hatWert)
                    wert = aktuell;
            }
            else if (hatEigenschaft (daten, disc))
            {
                wert = hole (daten, disc);
                hatWert = true;
            }
        }
        hatWert = hatWert && (wert.isString() || wert.isBool());

        bool getroffen = false;
        if (hatWert)
        {
            for (int i = 0; i < zweige->size(); ++i)
            {
                juce::var ziel; juce::String zpfad;
                aufloesen (wurzel, (*zweige)[i], sp + "/oneOf/" + juce::String (i), ziel, zpfad);
                juce::var k;
                if (diskriminatorwert (wurzel, ziel, disc, k) && gleich (k, wert))
                {
                    pruefeWert (wurzel, ziel, zpfad, daten, instanz, out);
                    getroffen = true;
                    break;
                }
            }
        }

        if (! getroffen)
        {
            // Ist die Instanz gar kein Objekt, gibt es keine Eigenschaft, auf
            // die der Pfad zeigen koennte - dann zeigt er auf die Instanz.
            const auto ort = ! istObjekt (daten) ? instanz
                             : disc.startsWithChar ('/') ? instanz + disc
                             : zeigerTeil (instanz, disc);
            out.push_back ({ ort, sp + "/oneOf", "oneOf" });
            return;
        }
    }

    // --- type ------------------------------------------------------------
    if (hatEigenschaft (knoten, "type"))
    {
        const auto t = hole (knoten, "type");
        bool ok = false;
        if (t.isString())
            ok = typPasst (t.toString(), daten);
        else if (auto* liste = t.getArray())
            for (int i = 0; i < liste->size() && ! ok; ++i)
                ok = (*liste)[i].isString() && typPasst ((*liste)[i].toString(), daten);

        if (! ok)
        {
            out.push_back ({ instanz, sp + "/type", "type" });
            // KEIN Abstieg nach einem Typfehler: sonst haengt an einem
            // falschen Typ eine Lawine von Folgefehlern, ueber deren
            // Reihenfolge sich beide Engines einigen muessten.
            return;
        }
    }

    // --- const / enum -----------------------------------------------------
    if (hatEigenschaft (knoten, "const"))
        if (! gleich (hole (knoten, "const"), daten))
            out.push_back ({ instanz, sp + "/const", "const" });

    if (auto* werte = hole (knoten, "enum").getArray())
    {
        bool treffer = false;
        for (int i = 0; i < werte->size() && ! treffer; ++i)
            treffer = gleich ((*werte)[i], daten);
        if (! treffer)
            out.push_back ({ instanz, sp + "/enum", "enum" });
    }

    // --- Zahlen ------------------------------------------------------------
    if (istZahl (daten))
    {
        const auto z = static_cast<double> (daten);
        struct { const char* name; bool (*ok) (double, double); } grenzen[] = {
            { "minimum",          [] (double a, double b) { return a >= b; } },
            { "maximum",          [] (double a, double b) { return a <= b; } },
            { "exclusiveMinimum", [] (double a, double b) { return a >  b; } },
            { "exclusiveMaximum", [] (double a, double b) { return a <  b; } }
        };
        for (const auto& g : grenzen)
            if (hatEigenschaft (knoten, g.name))
                if (! g.ok (z, static_cast<double> (hole (knoten, g.name))))
                    out.push_back ({ instanz, sp + "/" + g.name, g.name });
    }

    // --- Strings ------------------------------------------------------------
    if (daten.isString())
    {
        const auto s = daten.toString();
        // JUCEs String::length() zaehlt CODEPUNKTE, nicht Bytes - genau die
        // JSON-Schema-Regel und dieselbe wie Rusts chars().count().
        const auto n = s.length();
        if (hatEigenschaft (knoten, "minLength") && n < static_cast<int> (hole (knoten, "minLength")))
            out.push_back ({ instanz, sp + "/minLength", "minLength" });
        if (hatEigenschaft (knoten, "maxLength") && n > static_cast<int> (hole (knoten, "maxLength")))
            out.push_back ({ instanz, sp + "/maxLength", "maxLength" });
        if (hatEigenschaft (knoten, "pattern"))
            if (musterPasst (hole (knoten, "pattern").toString(), s) == 0)
                out.push_back ({ instanz, sp + "/pattern", "pattern" });
    }

    // --- Objekte -------------------------------------------------------------
    if (istObjekt (daten))
    {
        if (auto* pflicht = hole (knoten, "required").getArray())
            for (int i = 0; i < pflicht->size(); ++i)
            {
                const auto feld = (*pflicht)[i].toString();
                if (! hatEigenschaft (daten, feld))
                    // Ein fehlender Wert hat keinen eigenen Instanzpfad -
                    // deshalb zeigt `instanz` auf das ELTERNobjekt und der
                    // Name steht im Schemapfad.
                    out.push_back ({ instanz, sp + "/required/" + feld, "required" });
            }

        auto* felder = eigenschaften (daten);
        if (hatEigenschaft (knoten, "maxProperties"))
            if (felder->size() > static_cast<int> (hole (knoten, "maxProperties")))
                out.push_back ({ instanz, sp + "/maxProperties", "maxProperties" });

        const auto deklariert = hole (knoten, "properties");
        const bool strikt = hatEigenschaft (knoten, "additionalProperties")
                            && ! static_cast<bool> (hole (knoten, "additionalProperties"));
        if (strikt)
            for (int i = 0; i < felder->size(); ++i)
            {
                const auto name = felder->getName (i).toString();
                if (! hatEigenschaft (deklariert, name))
                    out.push_back ({ zeigerTeil (instanz, name),
                                     sp + "/additionalProperties", "additionalProperties" });
            }

        if (auto* teile = eigenschaften (deklariert))
            for (int i = 0; i < teile->size(); ++i)
            {
                const auto name = teile->getName (i).toString();
                if (hatEigenschaft (daten, name))
                    pruefeWert (wurzel, teile->getValueAt (i), sp + "/properties/" + name,
                                hole (daten, name), zeigerTeil (instanz, name), out);
            }
    }

    // --- Arrays ---------------------------------------------------------------
    if (auto* a = daten.getArray())
    {
        if (hatEigenschaft (knoten, "minItems") && a->size() < static_cast<int> (hole (knoten, "minItems")))
            out.push_back ({ instanz, sp + "/minItems", "minItems" });
        if (hatEigenschaft (knoten, "maxItems") && a->size() > static_cast<int> (hole (knoten, "maxItems")))
            out.push_back ({ instanz, sp + "/maxItems", "maxItems" });

        if (hatEigenschaft (knoten, "items"))
            for (int i = 0; i < a->size(); ++i)
                pruefeWert (wurzel, hole (knoten, "items"), sp + "/items",
                            (*a)[i], instanz + "/" + juce::String (i), out);
    }
}

} // namespace

juce::Array<Verletzung> Schema::pruefe (const juce::var& daten) const
{
    std::vector<Verletzung> roh;
    pruefeWert (wurzel, wurzel, "#", daten, juce::String(), roh);

    std::set<Verletzung> menge (roh.begin(), roh.end());
    juce::Array<Verletzung> out;
    for (const auto& v : menge)
        out.add (v);
    return out;
}

} // namespace nakama::vertrag

#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1-Endpruefung nach allen Kern-Headern
