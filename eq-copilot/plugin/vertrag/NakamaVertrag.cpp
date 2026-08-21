#include "NakamaVertrag.h"

#include <cmath>
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

/*  Geschlossene Mustertabelle. Regex ist zwischen Python, C++ und Rust nicht
    in jeder Ecke gleich; deshalb gibt es hier keine Regex-Auswertung, sondern
    benannte Muster. Ein unbekanntes Muster bricht den Ladevorgang.
    -1 = Muster unbekannt, 0 = passt nicht, 1 = passt. */
int musterPasst (const juce::String& muster, const juce::String& wert)
{
    if (muster == "^[0-9a-f]{32}$")
    {
        if (wert.length() != 32)
            return 0;
        for (auto z = wert.getCharPointer(); ! z.isEmpty(); ++z)
        {
            const auto c = *z;
            const bool ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
            if (! ok)
                return 0;
        }
        return 1;
    }
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

// ------------------------------------------------------------------ Ladelauf

namespace
{

void pruefeTeilschema (const juce::var& knoten, const juce::String& pfad,
                       juce::StringArray& fehler)
{
    auto* props = eigenschaften (knoten);
    if (props == nullptr)
        return;

    for (int i = 0; i < props->size(); ++i)
    {
        const auto name = props->getName (i).toString();
        if (! istSchluesselwort (name) && ! istAnmerkung (name))
            fehler.add ("unbekanntes Schluesselwort " + pfad + "/" + name);
    }

    if (hatEigenschaft (knoten, "pattern"))
        if (musterPasst (hole (knoten, "pattern").toString(), juce::String()) < 0)
            fehler.add ("unbekanntes Muster " + pfad + ": "
                        + hole (knoten, "pattern").toString());

    if (hatEigenschaft (knoten, "oneOf") && ! hatEigenschaft (knoten, "x-nakama-discriminator"))
        fehler.add ("oneOf ohne x-nakama-discriminator bei " + pfad);

    if (hatEigenschaft (knoten, "$ref"))
        if (! hole (knoten, "$ref").toString().startsWith ("#/$defs/"))
            fehler.add ("nicht-lokale Referenz bei " + pfad + ": "
                        + hole (knoten, "$ref").toString());

    if (hatEigenschaft (knoten, "additionalProperties"))
    {
        const auto a = hole (knoten, "additionalProperties");
        if (! a.isBool())
            fehler.add ("additionalProperties bei " + pfad + " ist kein bool");
        else if (static_cast<bool> (a) && ! hatEigenschaft (knoten, "maxProperties"))
            fehler.add ("additives Objekt " + pfad + " ohne maxProperties");
    }

    for (const auto* zweig : { "properties", "$defs" })
        if (auto* kinder = eigenschaften (hole (knoten, zweig)))
            for (int i = 0; i < kinder->size(); ++i)
                pruefeTeilschema (kinder->getValueAt (i),
                                  pfad + "/" + zweig + "/" + kinder->getName (i).toString(),
                                  fehler);

    if (hatEigenschaft (knoten, "items"))
        pruefeTeilschema (hole (knoten, "items"), pfad + "/items", fehler);

    if (auto* zweige = hole (knoten, "oneOf").getArray())
        for (int i = 0; i < zweige->size(); ++i)
            pruefeTeilschema ((*zweige)[i], pfad + "/oneOf/" + juce::String (i), fehler);
}

} // namespace

bool Schema::laden (const juce::var& wurzel, Schema& ziel, juce::String& fehler)
{
    juce::StringArray f;
    pruefeTeilschema (wurzel, "#", f);
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
bool diskriminatorwert (const juce::var& wurzel, const juce::var& zweig,
                        const juce::String& disc, juce::String& ergebnis)
{
    const auto props = hole (zweig, "properties");
    if (hatEigenschaft (props, disc))
    {
        const auto feld = hole (props, disc);
        if (hatEigenschaft (feld, "const"))
        {
            const auto k = hole (feld, "const");
            if (k.isString())
            {
                ergebnis = k.toString();
                return true;
            }
        }
    }

    auto* unter = hole (zweig, "oneOf").getArray();
    if (unter == nullptr)
        return false;

    bool erster = true;
    juce::String gemeinsam;
    for (int i = 0; i < unter->size(); ++i)
    {
        juce::var ziel; juce::String zpfad;
        aufloesen (wurzel, (*unter)[i], "#", ziel, zpfad);
        juce::String w;
        if (! diskriminatorwert (wurzel, ziel, disc, w))
            return false;
        if (erster) { gemeinsam = w; erster = false; }
        else if (gemeinsam != w) return false;
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

        juce::String wert;
        const bool hatWert = istObjekt (daten) && hatEigenschaft (daten, disc)
                             && hole (daten, disc).isString();
        if (hatWert)
            wert = hole (daten, disc).toString();

        if (hatWert)
        {
            for (int i = 0; i < zweige->size(); ++i)
            {
                juce::var ziel; juce::String zpfad;
                aufloesen (wurzel, (*zweige)[i], sp + "/oneOf/" + juce::String (i), ziel, zpfad);
                juce::String k;
                if (diskriminatorwert (wurzel, ziel, disc, k) && k == wert)
                {
                    pruefeWert (wurzel, ziel, zpfad, daten, instanz, out);
                    return;
                }
            }
        }

        // Ist die Instanz gar kein Objekt, gibt es keine Eigenschaft, auf die
        // der Pfad zeigen koennte - dann zeigt er auf die Instanz selbst.
        out.push_back ({ istObjekt (daten) ? zeigerTeil (instanz, disc) : instanz,
                         sp + "/oneOf", "oneOf" });
        return;
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
