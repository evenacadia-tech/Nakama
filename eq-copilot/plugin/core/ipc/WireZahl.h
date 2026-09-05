#pragma once

/*  Die Zahl auf dem Draht (NAK-181 R4, G4-Befund V05).

    ZWEI EINSTIEGE, EIN ALGORITHMUS

    `es6Zahl`  — ECMA-262 `Number::toString (x, 10)`, ungedeckelt. Das ist die
                 Zahlenform von RFC 8785 §3.2.2.3 und damit die des
                 `state_hash`; `nakama::kanon::zahlAlsEs6` ruft genau sie.
    `wireZahl` — dieselbe Ziffernfolge, aber mit den Grenzen des v3-Textriegels
                 (`broker/src/vertrag.rs`, A5 `pruefe_v3_vertrag.py`). Alles,
                 was der Riegel ablehnen wuerde, liefert `false`.

    WARUM ES DIESEN KOPF GIBT

    `NakamaEvidenz.cpp` und `ControlClient.cpp` bauten ihre JSON-Zahlen bis
    NAK-181 mit `std::to_string(double)`. Das ist unter MSVC `sprintf("%f")`:
    LC_NUMERIC-abhaengig und auf sechs Nachkommastellen fixiert. Ein fremdes
    Modul im FL-Prozess mit `setlocale(LC_ALL, "")` — unter /MD teilen alle
    Module dieselbe UCRT — macht daraus `1,5` statt `1.5`, und ein Bandsatz aus
    221 Werten wird zu 442 Arrayelementen. Kein Angreifer, ein bekannter
    Audio-Plugin-Fallstrick.

    Beide Einheiten sind JUCE-frei und liegen mit `state/NakamaKanon.cpp` in
    derselben Bibliothek `NakamaKern`; deshalb steht der Algorithmus hier und
    nicht dreimal.

    DIE VIER REGELN DES TEXTRIEGELS (lexikalisch, nie ueber `parse<f64>`)

      1. Endlichkeit.
      2. Betrag: `dez > -308` UND `dez < 308`, wobei
         `dez = (ganz.len() - fuehrende - 1) + exp` am fertigen Text.
      3. Exakt ganzzahliger Wert in JEDER Schreibweise: Betrag <= 2^53 - 1.
      4. Nichtganzzahliger Wert: hoechstens 15 signifikante Dezimalziffern.

    Regel 4 ist der Grund fuer den Deckel: die kuerzeste round-trip-exakte Form
    eines `double` braucht bis zu 17 Stellen, und `1.0/3.0` faellt damit am
    Riegel. Regel 3 ist der Grund, warum der Deckel NUR nichtganzzahlige Werte
    trifft: gedeckelt wuerde `9007199254740991` zu `9.00719925474099e15` — ein
    anderer Wert, den der Riegel zwar annimmt, der aber nicht der gemessene
    ist.

    Wirksam ist damit ein engerer Bereich, als Regel 2 allein sagt: jeder
    `double` mit |x| >= 2^53 ist ohne Nachkommaanteil, faellt also an Regel 3,
    egal wie er geschrieben wird. Nach unten traegt der Draht Betraege ab
    `1e-307`. Die Grenze gilt dem TEXT: `std::nextafter (1e-307, 0.0)` liegt
    darunter, sein gedeckelter Text lautet `1e-307` und wird angenommen.

    Nicht fuer den Audiothread: `wireZahl` allokiert (der `std::string` des
    Aufrufers) und wandelt Zeichenketten.
*/

#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <system_error>

namespace nakama::wire
{

/// Groesste ganze Zahl, die binary64 exakt traegt: 2^53 - 1. Spiegel von
/// `SICHERE_GANZZAHL` in `broker/src/vertrag.rs`.
inline constexpr double kSichereGanzzahl = 9007199254740991.0;

/// Betragsgrenze des Vertrags als Dezimalexponent. Spiegel von `DEZ_GRENZE`.
inline constexpr int kDezGrenze = 308;

/// Signifikante Dezimalziffern, die der Riegel einem nichtganzzahligen Wert
/// zugesteht.
inline constexpr int kMaxSignifikant = 15;

namespace detail
{

/** Die ECMA-262-Form aus Ziffernfolge und Dezimalexponent.

    `ziffern` traegt nur Ziffern ohne fuehrende Nullen, `n` ist der Exponent
    nach Schritt 5 der Norm (`s * 10^(n-k)` mit `k = ziffern.size()`).
    Identisch mit den Schritten 6 bis 10 in `state/NakamaKanon.cpp`. */
inline std::string es6Zusammensetzen (bool negativ, const std::string& ziffern, int n)
{
    const int k = (int) ziffern.size();
    std::string text;
    if (n >= -5 && n <= 21)
    {
        if (n >= k)
            text = ziffern + std::string ((std::size_t) (n - k), '0');
        else if (n > 0)
            text = ziffern.substr (0, (std::size_t) n) + "." + ziffern.substr ((std::size_t) n);
        else
            text = "0." + std::string ((std::size_t) (-n), '0') + ziffern;
    }
    else
    {
        const char vorzeichen = (n < 0) ? '-' : '+';
        const int e = std::abs (n - 1);
        if (k == 1)
            text = ziffern + "e" + vorzeichen + std::to_string (e);
        else
            text = ziffern.substr (0, 1) + "." + ziffern.substr (1) + "e" + vorzeichen
                 + std::to_string (e);
    }
    return (negativ ? "-" : "") + text;
}

/** Zerlegt `betrag` in Ziffernfolge und Exponent `n`.

    `stellen == 0` heisst „kuerzeste round-trip-exakte Form" (Ryu ueber
    `std::to_chars` ohne Genauigkeit), sonst wird auf genau `stellen`
    signifikante Ziffern gerundet. In beiden Faellen fallen Nullen am Ende der
    Mantisse weg — sie tragen keine Information und wuerden die Stellenzahl
    des Riegels unnoetig hochtreiben. */
inline bool zerlegen (double betrag, int stellen, std::string& ziffern, int& n)
{
    char puffer[64];
    const auto ergebnis = stellen > 0
        ? std::to_chars (puffer, puffer + sizeof (puffer), betrag,
                         std::chars_format::scientific, stellen - 1)
        : std::to_chars (puffer, puffer + sizeof (puffer), betrag,
                         std::chars_format::scientific);
    if (ergebnis.ec != std::errc())
        return false;

    const std::string wissenschaftlich (puffer, ergebnis.ptr);
    const auto ePos = wissenschaftlich.find ('e');
    if (ePos == std::string::npos)
        return false;

    ziffern.clear();
    for (std::size_t i = 0; i < ePos; ++i)
        if (wissenschaftlich[i] != '.')
            ziffern += wissenschaftlich[i];
    while (ziffern.size() > 1 && ziffern.back() == '0')
        ziffern.pop_back();

    const int exponent10 = std::atoi (wissenschaftlich.c_str() + ePos + 1);
    n = exponent10 + 1;   // d.ddd * 10^e == s * 10^(n-k) mit n = e + 1
    return true;
}

} // namespace detail

/** ECMA-262 `Number::toString (x, 10)` — ungedeckelt.

    @returns `false` fuer NaN und +/-Infinity (in JSON nicht darstellbar).
             `-0` und `+0` liefern beide `"0"` (Schritt 2 der Norm). */
inline bool es6Zahl (double x, std::string& aus)
{
    if (! std::isfinite (x))
        return false;
    if (x == 0.0)
    {
        aus = "0";
        return true;
    }
    std::string ziffern;
    int n = 0;
    if (! detail::zerlegen (std::fabs (x), 0, ziffern, n))
        return false;
    aus = detail::es6Zusammensetzen (x < 0.0, ziffern, n);
    return true;
}

/** Die Zahl fuer den v3-Draht: ES6-Form mit den Grenzen des Textriegels.

    @returns `false`, wenn der Riegel den Wert nicht traegt — nicht endlich,
             Betrag ausserhalb, oder ganzzahliger Betrag ueber `2^53 - 1`.
             Der Aufrufer nimmt dann seinen bestehenden Ersatzweg (0 ohne
             Praesenzbit, Ersatzzahl, entfallendes Objekt oder gar keine
             Nachricht); eine Saettigung an der Grenze waere die Behauptung,
             der Grenzwert sei gemessen worden. */
inline bool wireZahl (double x, std::string& aus)
{
    if (! std::isfinite (x))
        return false;
    if (x == 0.0)
    {
        aus = "0";                       // deckt +0 und -0 ab (Regel 2 greift nicht)
        return true;
    }

    const double betrag = std::fabs (x);

    // Regel 3, erste Haelfte: ein exakt ganzzahliger Wert reist in seiner
    // exakten Form — aber nur bis zur sicheren Grenze. Darueber ist JEDER
    // `double` ganzzahlig, und genau die faengt der Riegel.
    const bool ganzzahlig = (betrag == std::floor (betrag));
    if (ganzzahlig && betrag > kSichereGanzzahl)
        return false;

    std::string ziffern;
    int n = 0;
    // Regel 4: gedeckelt wird NUR, was einen Nachkommaanteil hat.
    if (! detail::zerlegen (betrag, ganzzahlig ? 0 : kMaxSignifikant, ziffern, n))
        return false;

    // Regel 2 am FERTIGEN Text: `dez` ist der Exponent der normalisierten
    // Mantisse, also `n - 1`. Die Pruefung steht NACH der Rundung, weil der
    // Riegel den Text beurteilt und nicht den `double` — sonst faellt ein
    // Wert, dessen gedeckelter Text die Grenze haelt.
    const int dez = n - 1;
    if (dez >= kDezGrenze || dez <= -kDezGrenze)
        return false;

    // Regel 3, zweite Haelfte: die Rundung kann einen nichtganzzahligen Wert
    // auf eine Ganzzahl heben (etwa `9007199254740991.4`). Der Riegel beurteilt
    // den Text, also wird hier der Text geprueft.
    if (! ganzzahlig && n >= (int) ziffern.size())
    {
        const double gerundet = std::strtod (
            detail::es6Zusammensetzen (false, ziffern, n).c_str(), nullptr);
        if (gerundet > kSichereGanzzahl)
            return false;
    }

    aus = detail::es6Zusammensetzen (x < 0.0, ziffern, n);
    return true;
}

} // namespace nakama::wire
