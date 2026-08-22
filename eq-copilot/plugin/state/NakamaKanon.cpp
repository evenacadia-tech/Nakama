#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 — keine JucePlugin_*-Konstante im Kern
#include "NakamaKanon.h"

#include <juce_cryptography/juce_cryptography.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <system_error>

namespace nakama::kanon
{

bool Wert::objektSetze (const juce::String& schluessel, Wert wert)
{
    for (const auto& bekannt : objektSchluessel)
        if (bekannt == schluessel)
            return false;
    objektSchluessel.push_back (schluessel);
    objektWerte.push_back (std::move (wert));
    return true;
}

// ── Zahlen: ECMA-262 Number::toString (x, 10) ──────────────────────────────

bool zahlAlsEs6 (double x, juce::String& aus)
{
    if (! std::isfinite (x))
        return false;

    // Schritt 2 der Norm: +0 und -0 sind beide "0".
    if (x == 0.0)
    {
        aus = "0";
        return true;
    }

    const bool negativ = x < 0.0;
    const double betrag = std::fabs (x);

    // Schritt 5: k, s, n - die KUERZESTE Ziffernfolge s mit k Ziffern, so dass
    // s * 10^(n-k) exakt x ist. std::to_chars ohne Genauigkeit liefert genau
    // diese kuerzeste round-trip-exakte Form (Ryu), hier als d.ddde±XX.
    char puffer[64];
    const auto ergebnis = std::to_chars (puffer, puffer + sizeof (puffer), betrag,
                                         std::chars_format::scientific);
    if (ergebnis.ec != std::errc())
        return false;

    const std::string wissenschaftlich (puffer, ergebnis.ptr);
    const auto ePos = wissenschaftlich.find ('e');
    if (ePos == std::string::npos)
        return false;

    std::string ziffern;
    for (size_t i = 0; i < ePos; ++i)
        if (wissenschaftlich[i] != '.')
            ziffern += wissenschaftlich[i];
    // d.ddd-Form: Nullen am Ende der Mantisse tragen keine Information.
    while (ziffern.size() > 1 && ziffern.back() == '0')
        ziffern.pop_back();

    const int exponent10 = std::atoi (wissenschaftlich.c_str() + ePos + 1);
    const int k = (int) ziffern.size();
    const int n = exponent10 + 1;   // d.ddd * 10^e  ==  s * 10^(n-k)  mit  n = e + 1

    std::string text;
    if (n >= -5 && n <= 21)
    {
        // Schritt 6: Positionsschreibweise.
        if (n >= k)
            text = ziffern + std::string ((size_t) (n - k), '0');
        else if (n > 0)
            text = ziffern.substr (0, (size_t) n) + "." + ziffern.substr ((size_t) n);
        else
            text = "0." + std::string ((size_t) (-n), '0') + ziffern;
    }
    else
    {
        // Schritte 7-10: Exponentschreibweise mit Vorzeichen, abs (n - 1).
        const char vorzeichen = (n < 0) ? '-' : '+';
        const int e = std::abs (n - 1);
        if (k == 1)
            text = ziffern + "e" + vorzeichen + std::to_string (e);
        else
            text = ziffern.substr (0, 1) + "." + ziffern.substr (1) + "e" + vorzeichen + std::to_string (e);
    }

    aus = juce::String ((negativ ? "-" : "") + text);
    return true;
}

// ── Strings ────────────────────────────────────────────────────────────────

namespace
{

void schreibeCodepunktUtf8 (juce::juce_wchar c, juce::MemoryOutputStream& aus)
{
    char puffer[8] = { 0 };
    juce::CharPointer_UTF8 schreiber (puffer);
    schreiber.write (c);
    schreiber.writeNull();
    aus.write (puffer, std::strlen (puffer));
}

void schreibeText (const juce::String& t, juce::MemoryOutputStream& aus)
{
    aus.writeByte ('"');
    for (auto p = t.getCharPointer(); ! p.isEmpty();)
    {
        const juce::juce_wchar c = p.getAndAdvance();
        switch (c)
        {
            case '"':  aus.write ("\\\"", 2); break;
            case '\\': aus.write ("\\\\", 2); break;
            case 0x08: aus.write ("\\b", 2);  break;
            case 0x09: aus.write ("\\t", 2);  break;
            case 0x0A: aus.write ("\\n", 2);  break;
            case 0x0C: aus.write ("\\f", 2);  break;
            case 0x0D: aus.write ("\\r", 2);  break;
            default:
                if (c < 0x20)
                {
                    char esc[8];
                    std::snprintf (esc, sizeof (esc), "\\u%04x", (unsigned) c);
                    aus.write (esc, 6);
                }
                else
                {
                    schreibeCodepunktUtf8 (c, aus);
                }
                break;
        }
    }
    aus.writeByte ('"');
}

std::vector<juce::uint16> alsUtf16 (const juce::String& s)
{
    std::vector<juce::uint16> aus;
    for (auto p = s.getCharPointer(); ! p.isEmpty();)
    {
        juce::uint32 c = (juce::uint32) p.getAndAdvance();
        if (c >= 0x10000)
        {
            c -= 0x10000;
            aus.push_back ((juce::uint16) (0xD800 + (c >> 10)));
            aus.push_back ((juce::uint16) (0xDC00 + (c & 0x3FF)));
        }
        else
        {
            aus.push_back ((juce::uint16) c);
        }
    }
    return aus;
}

bool schreibeWert (const Wert& w, juce::MemoryOutputStream& aus, juce::String& fehler)
{
    switch (w.art)
    {
        case Wert::Art::null:    aus.write ("null", 4);  return true;
        case Wert::Art::boolean: if (w.b) aus.write ("true", 4); else aus.write ("false", 5); return true;
        case Wert::Art::zahl:
        {
            juce::String t;
            if (! zahlAlsEs6 (w.zahl, t))
            {
                fehler = "nichtendliche Zahl ist in JSON nicht darstellbar";
                return false;
            }
            aus.write (t.toRawUTF8(), t.getNumBytesAsUTF8());
            return true;
        }
        case Wert::Art::text:    schreibeText (w.text, aus); return true;
        case Wert::Art::liste:
        {
            aus.writeByte ('[');
            bool erster = true;
            for (const auto& e : w.liste)
            {
                if (! erster) aus.writeByte (',');
                erster = false;
                if (! schreibeWert (e, aus, fehler))
                    return false;
            }
            aus.writeByte (']');
            return true;
        }
        case Wert::Art::objekt:
        {
            // RFC 8785 §3.2.3: nach UTF-16-Code-Units, rekursiv.
            std::vector<size_t> reihenfolge (w.objektSchluessel.size());
            std::vector<std::vector<juce::uint16>> schluessel;
            schluessel.reserve (w.objektSchluessel.size());
            for (size_t i = 0; i < w.objektSchluessel.size(); ++i)
            {
                reihenfolge[i] = i;
                schluessel.push_back (alsUtf16 (w.objektSchluessel[i]));
            }
            std::stable_sort (reihenfolge.begin(), reihenfolge.end(), [&] (size_t a, size_t b)
            {
                return std::lexicographical_compare (schluessel[a].begin(), schluessel[a].end(),
                                                     schluessel[b].begin(), schluessel[b].end());
            });
            aus.writeByte ('{');
            bool erster = true;
            for (size_t i : reihenfolge)
            {
                if (! erster) aus.writeByte (',');
                erster = false;
                schreibeText (w.objektSchluessel[i], aus);
                aus.writeByte (':');
                if (! schreibeWert (w.objektWerte[i], aus, fehler))
                    return false;
            }
            aus.writeByte ('}');
            return true;
        }
    }
    fehler = "unbekannte Wertart";
    return false;
}

} // namespace

bool kanonisiere (const Wert& w, juce::MemoryBlock& utf8, juce::String& fehler)
{
    utf8.reset();
    juce::MemoryOutputStream aus (utf8, false);
    const bool ok = schreibeWert (w, aus, fehler);
    aus.flush();
    if (! ok)
        utf8.reset();
    return ok;
}

juce::String sha256Hex (const void* daten, size_t laenge)
{
    return juce::SHA256 (daten, laenge).toHexString().toLowerCase();
}

// ── JSON-Leser ─────────────────────────────────────────────────────────────

namespace
{

struct Leser
{
    juce::String::CharPointerType p;
    juce::String fehler;
    int tiefe = 0;

    static bool istWs (juce::juce_wchar c) noexcept { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
    void ws() { while (! p.isEmpty() && istWs (*p)) p.getAndAdvance(); }
    bool fehl (const juce::String& text) { if (fehler.isEmpty()) fehler = text; return false; }

    static bool hexZiffer (juce::juce_wchar c, int& wert)
    {
        if (c >= '0' && c <= '9') { wert = (int) (c - '0'); return true; }
        if (c >= 'a' && c <= 'f') { wert = 10 + (int) (c - 'a'); return true; }
        if (c >= 'A' && c <= 'F') { wert = 10 + (int) (c - 'A'); return true; }
        return false;
    }

    bool hex4 (juce::uint32& aus)
    {
        aus = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (p.isEmpty()) return fehl ("Hex-Escape abgebrochen");
            int z = 0;
            if (! hexZiffer (p.getAndAdvance(), z)) return fehl ("kein 4-stelliges Hex-Escape");
            aus = aus * 16 + (juce::uint32) z;
        }
        return true;
    }

    /** Nach dem oeffnenden `"`. */
    bool text (juce::String& aus)
    {
        juce::MemoryOutputStream puffer;
        for (;;)
        {
            if (p.isEmpty()) return fehl ("unbeendete Zeichenkette");
            const juce::juce_wchar c = p.getAndAdvance();
            if (c == '"') break;
            if (c < 0x20) return fehl ("rohes Steuerzeichen in Zeichenkette");
            juce::juce_wchar zeichen = c;
            if (c == '\\')
            {
                if (p.isEmpty()) return fehl ("Escape abgebrochen");
                const juce::juce_wchar e = p.getAndAdvance();
                switch (e)
                {
                    case '"':  zeichen = '"';  break;
                    case '\\': zeichen = '\\'; break;
                    case '/':  zeichen = '/';  break;
                    case 'b':  zeichen = 0x08; break;
                    case 'f':  zeichen = 0x0C; break;
                    case 'n':  zeichen = 0x0A; break;
                    case 'r':  zeichen = 0x0D; break;
                    case 't':  zeichen = 0x09; break;
                    case 'u':
                    {
                        juce::uint32 u = 0;
                        if (! hex4 (u)) return false;
                        if (u >= 0xD800 && u <= 0xDBFF)
                        {
                            // Hohes Surrogat: das tiefe MUSS folgen.
                            if (p.isEmpty() || p.getAndAdvance() != '\\') return fehl ("hohes Surrogat ohne Paar");
                            if (p.isEmpty() || p.getAndAdvance() != 'u')  return fehl ("hohes Surrogat ohne Paar");
                            juce::uint32 t = 0;
                            if (! hex4 (t)) return false;
                            if (t < 0xDC00 || t > 0xDFFF) return fehl ("hohes Surrogat ohne tiefes");
                            u = 0x10000 + ((u - 0xD800) << 10) + (t - 0xDC00);
                        }
                        else if (u >= 0xDC00 && u <= 0xDFFF)
                        {
                            return fehl ("einsames tiefes Surrogat");
                        }
                        if (u == 0) return fehl ("NUL in Zeichenkette (juce::String ist nullterminiert)");
                        zeichen = (juce::juce_wchar) u;
                        break;
                    }
                    default: return fehl ("unbekanntes Escape");
                }
            }
            char utf8[8] = { 0 };
            juce::CharPointer_UTF8 w (utf8);
            w.write (zeichen);
            w.writeNull();
            puffer.write (utf8, std::strlen (utf8));
        }
        aus = juce::String::fromUTF8 ((const char*) puffer.getData(), (int) puffer.getDataSize());
        return true;
    }

    bool zahl (Wert& aus)
    {
        // Grammatik RFC 8259 §6: -?(0|[1-9][0-9]*)(.[0-9]+)?([eE][+-]?[0-9]+)?
        std::string lit;
        auto istZiffer = [] (juce::juce_wchar c) { return c >= '0' && c <= '9'; };
        if (! p.isEmpty() && *p == '-') { lit += '-'; p.getAndAdvance(); }
        if (p.isEmpty() || ! istZiffer (*p)) return fehl ("Zahl ohne Ziffern");
        if (*p == '0')
        {
            lit += '0'; p.getAndAdvance();
            if (! p.isEmpty() && istZiffer (*p)) return fehl ("fuehrende Null");
        }
        else
            while (! p.isEmpty() && istZiffer (*p)) lit += (char) p.getAndAdvance();
        if (! p.isEmpty() && *p == '.')
        {
            lit += '.'; p.getAndAdvance();
            if (p.isEmpty() || ! istZiffer (*p)) return fehl ("Dezimalpunkt ohne Nachkommaziffern");
            while (! p.isEmpty() && istZiffer (*p)) lit += (char) p.getAndAdvance();
        }
        if (! p.isEmpty() && (*p == 'e' || *p == 'E'))
        {
            lit += 'e'; p.getAndAdvance();
            if (! p.isEmpty() && (*p == '+' || *p == '-')) lit += (char) p.getAndAdvance();
            if (p.isEmpty() || ! istZiffer (*p)) return fehl ("Exponent ohne Ziffern");
            while (! p.isEmpty() && istZiffer (*p)) lit += (char) p.getAndAdvance();
        }
        double d = 0.0;
        const auto r = std::from_chars (lit.data(), lit.data() + lit.size(), d);
        if (r.ec != std::errc() || r.ptr != lit.data() + lit.size() || ! std::isfinite (d))
            return fehl ("Zahl nicht darstellbar: " + juce::String (lit));
        aus = Wert::nummer (d);
        return true;
    }

    bool literal (const char* wort, Wert& aus, Wert w)
    {
        for (const char* q = wort; *q != 0; ++q)
            if (p.isEmpty() || p.getAndAdvance() != (juce::juce_wchar) *q)
                return fehl (juce::String ("Literal erwartet: ") + wort);
        aus = std::move (w);
        return true;
    }

    bool wert (Wert& aus)
    {
        ws();
        if (p.isEmpty()) return fehl ("unerwartetes Ende");
        const juce::juce_wchar c = *p;
        if (c == '{')
        {
            p.getAndAdvance();
            if (++tiefe > 64) return fehl ("zu tief verschachtelt");
            aus = Wert::leeresObjekt();
            ws();
            if (! p.isEmpty() && *p == '}') { p.getAndAdvance(); --tiefe; return true; }
            for (;;)
            {
                ws();
                if (p.isEmpty() || p.getAndAdvance() != '"') return fehl ("Objektschluessel erwartet");
                juce::String schluessel;
                if (! text (schluessel)) return false;
                ws();
                if (p.isEmpty() || p.getAndAdvance() != ':') return fehl ("':' erwartet");
                Wert kind;
                if (! wert (kind)) return false;
                if (! aus.objektSetze (schluessel, std::move (kind)))
                    return fehl ("doppelter Schluessel: " + schluessel);
                ws();
                if (p.isEmpty()) return fehl ("Objekt nicht geschlossen");
                const juce::juce_wchar t = p.getAndAdvance();
                if (t == '}') break;
                if (t != ',') return fehl ("',' oder '}' erwartet");
            }
            --tiefe;
            return true;
        }
        if (c == '[')
        {
            p.getAndAdvance();
            if (++tiefe > 64) return fehl ("zu tief verschachtelt");
            aus = Wert::leereListe();
            ws();
            if (! p.isEmpty() && *p == ']') { p.getAndAdvance(); --tiefe; return true; }
            for (;;)
            {
                Wert kind;
                if (! wert (kind)) return false;
                aus.liste.push_back (std::move (kind));
                ws();
                if (p.isEmpty()) return fehl ("Array nicht geschlossen");
                const juce::juce_wchar t = p.getAndAdvance();
                if (t == ']') break;
                if (t != ',') return fehl ("',' oder ']' erwartet");
            }
            --tiefe;
            return true;
        }
        if (c == '"')
        {
            p.getAndAdvance();
            juce::String t;
            if (! text (t)) return false;
            aus = Wert::zeichen (t);
            return true;
        }
        if (c == 't') return literal ("true", aus, Wert::boolean (true));
        if (c == 'f') return literal ("false", aus, Wert::boolean (false));
        if (c == 'n') return literal ("null", aus, Wert::null());
        if (c == '-' || (c >= '0' && c <= '9')) return zahl (aus);
        return fehl ("unerwartetes Zeichen");
    }
};

} // namespace

bool lies (const juce::String& jsonText, Wert& aus, juce::String& fehler)
{
    Leser l { jsonText.getCharPointer() };   // CharPointer hat keinen Default-Konstruktor
    Wert w;
    if (! l.wert (w)) { fehler = l.fehler; return false; }
    l.ws();
    if (! l.p.isEmpty()) { fehler = "Text nach dem Dokument"; return false; }
    aus = std::move (w);
    fehler.clear();
    return true;
}

} // namespace nakama::kanon
