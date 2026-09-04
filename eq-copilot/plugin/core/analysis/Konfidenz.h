// SONDE-013 (S20-22) — Konfidenzklasse einer Messaussage (M-06, Entwurf §34.3).
//
// Der Satz, um den es geht, steht dort woertlich:
//
//     "Eine Gesamtklasse wird nicht aus einem schoenen Mittelwert gerettet:
//      ein harter Mangel bei Session, Passage, Coverage oder Alignment
//      begrenzt die Gesamtaussage."
//
// ─────────────────────────────────────────────────────────────────────────────
// WARUM DAS EINE EIGENE DATEI IST, UND KEIN `float score`.
//
// Ein Score ist eine Zahl, die man mitteln kann — und genau das verbietet
// §34.3. Vier Quellen entscheiden ueber die Gesamtklasse, und zwar NICHT
// gleichberechtigt: drei von ihnen koennen sie DECKELN, egal wie gut die
// uebrigen aussehen. Ein Deckel laesst sich in einem Mittelwert nicht
// ausdruecken; er ist eine Minimumbildung. Deshalb ist `klasse` im Vertrag
// ein eigenes `enum`-Feld und kein gerundeter Score
// (`eq-ipc-v3.schema.json`, `$defs/konfidenz`).
//
// Die zweite Haelfte des Satzes ist genauso tragend: WAS DER ERZEUGER NICHT
// WEISS, DECKELT AUCH. Eine Sonde kennt von den vier Quellen genau eine —
// ihre eigene Coverage. Session, Passage und Alignment entstehen im Broker
// (§35.1, §38.2, §43.2). Eine Sonde, die `stark` meldete, behauptete etwas
// ueber drei Dinge, die sie nicht sieht. Das ist der Grund, warum Etappe B
// die Sondenklasse hart auf `mittel` gedeckelt hat — diese Datei macht die
// Regel dahinter allgemein und pruefbar, statt sie an einer Stelle als
// Literal zu wiederholen.
//
// ⚠️ Die Schwellen hier sind an `kFeatureMetricsVersion` gebunden und im
// Register `eq-copilot/schemas/v3/metriken-v1.json` gefuehrt. Sie zu
// aendern, ohne die Version zu heben, faellt an **A5**
// (`metrics_version_bindet_schwellen`) — genau das Risiko 5 aus §5.3.

#pragma once

#include <cstdint>

namespace nakama::analyse
{

//==============================================================================
/** Die vier Klassen aus §34.3, geordnet.

    Die Reihenfolge ist tragend: `min` zweier Klassen ist die Deckelung, um
    die es in §34.3 geht. Ein `enum class` mit expliziten Werten macht die
    Ordnung zur Zusage statt zum Zufall der Deklarationsreihenfolge. */
enum class Konfidenzklasse : std::uint8_t
{
    unbrauchbar = 0,
    schwach     = 1,
    mittel      = 2,
    stark       = 3
};

inline const char* klasseName (Konfidenzklasse k) noexcept
{
    switch (k)
    {
        case Konfidenzklasse::stark:       return "stark";
        case Konfidenzklasse::mittel:      return "mittel";
        case Konfidenzklasse::schwach:     return "schwach";
        case Konfidenzklasse::unbrauchbar: return "unbrauchbar";
    }
    return "unbrauchbar";
}

/** Was ein Erzeuger ueber seine eigene Aussage weiss.

    Je Quelle ZWEI Bits, und der Unterschied ist der Kern dieser Datei:

    - `…Bekannt` heisst „ich kann diese Quelle ueberhaupt beurteilen".
    - `…Hart`    heisst „ich beurteile sie, und sie ist schlecht".

    Ein Erzeuger, der eine Quelle nicht kennt, ist nicht dasselbe wie einer,
    bei dem sie in Ordnung ist. Genau diese zwei Faelle zusammenzulegen waere
    der „schoene Mittelwert" aus §34.3: die Sonde saehe drei ihrer vier
    Quellen als „kein Mangel" und meldete `stark`. */
struct Konfidenzlage
{
    bool sessionBekannt   { false };  bool sessionHart   { false };
    bool passageBekannt   { false };  bool passageHart   { false };
    bool coverageBekannt  { false };  bool coverageHart  { false };
    bool alignmentBekannt { false };  bool alignmentHart { false };

    /** Gemessene Signalabdeckung in [0, 1] — die Groesse, aus der die
        BASISklasse entsteht, bevor die Deckel greifen. Ohne Bit gibt es
        keine Basis: „nie gemessen" ist nicht „null Abdeckung". */
    bool  abdeckungGesetzt { false };  float abdeckung { 0.0f };

    /** Auf wie viele Analysefenster sich der schwaechste Verteilungspunkt
        stuetzt (`konfidenz.verteilung_fenster`). Eine Verteilung ueber drei
        Fenster traegt keine starke Aussage, auch bei voller Abdeckung. */
    std::uint32_t verteilungFenster { 0 };

    /** SONDE-013 M-07: nicht-endliche EINGANGSsamples im Fenster, aus dem
        dieser Beleg entstand.

        Das ist kein Mangel, den man gegen gute Nachbarwerte verrechnet, und
        deshalb auch kein fuenfter Eintrag in der Zaehlung der harten Maengel:
        ein NaN im Eingang heisst, dass die Engine an dieser Stelle STILLE
        gerechnet hat. Jede Zahl darueber ist eine Aussage ueber Audio, das es
        so nie gab. Der Beleg ist damit `unbrauchbar` — nicht `schwach`. */
    bool          sampleFehlerBekannt { false };
    std::uint32_t sampleFehler        { 0 };
};

//==============================================================================
// Die Schwellen. Sie gehoeren zu `kFeatureMetricsVersion` und stehen im
// Register `metriken-v1.json`; A5 haelt beide gegeneinander.

/** Ab welcher Abdeckung eine Aussage `stark` sein KANN (vor den Deckeln). */
inline constexpr double kKonfidenzAbdeckungStark = 0.80;
/** Ab welcher Abdeckung sie `mittel` sein kann. */
inline constexpr double kKonfidenzAbdeckungMittel = 0.50;
/** Wie viele Analysefenster hinter der Verteilung stehen muessen, damit eine
    Aussage `stark` sein kann. Acht ist dieselbe Zahl wie
    `kWelchMindestFrames`, und aus demselben Grund: unter acht ist eine
    Verteilung ein Zufall, kein Verlauf. */
inline constexpr std::uint32_t kKonfidenzFensterStark = 8;

//==============================================================================
/** Das Minimum zweier Klassen — die Deckelung aus §34.3 in einer Zeile.

    Sie steht als eigene Funktion und nicht als Lambda, weil `gesamtklasse`
    `constexpr` ist: ein Lambda darin ist erst ab C++23 zulaessig, und
    dieser Kern ist C++20. */
constexpr Konfidenzklasse deckle (Konfidenzklasse a, Konfidenzklasse b) noexcept
{
    return a < b ? a : b;
}

/** Die Gesamtklasse nach §34.3.

    Reihenfolge, und jeder Schritt kann allein fallen:

    1. **Basis aus der Abdeckung.** Ohne Abdeckungsbit gibt es keine Aussage
       (`unbrauchbar`) — nicht, weil nichts da waere, sondern weil niemand
       weiss, ob etwas da war.
    2. **Deckel „nicht beurteilbar".** Jede der vier Quellen, die der
       Erzeuger nicht kennt, deckelt auf `mittel`. Eine Sonde kennt genau
       eine, also ist ihre Obergrenze `mittel` — ohne dass das an einer
       Stelle als Literal steht.
    3. **Deckel „harter Mangel".** EIN harter Mangel deckelt auf `schwach`,
       ZWEI oder mehr auf `unbrauchbar`. Das ist der Satz aus §34.3 in
       seiner scharfen Form: der Mangel wird nicht gegen gute Nachbarwerte
       verrechnet.
    4. **Deckel „duenne Verteilung".** Unter `kKonfidenzFensterStark`
       Fenstern ist keine starke Aussage moeglich.

    Die Funktion ist bewusst `constexpr` und ohne Zustand: sie ist eine
    Regel, kein Objekt, und ein Test kann sie ueber die ganze Faelleflaeche
    fahren, ohne eine Engine zu bauen. */
constexpr Konfidenzklasse gesamtklasse (const Konfidenzlage& l) noexcept
{
    // 0. Verriegelung (SONDE-013 M-07). Sie steht VOR der Basis, weil sie
    //    keine Deckelung ist: ein nicht-endliches Eingangssample macht die
    //    Messung nicht schwaecher, sondern zu einer Aussage ueber Stille.
    if (l.sampleFehlerBekannt && l.sampleFehler > 0)
        return Konfidenzklasse::unbrauchbar;

    // 1. Basis.
    if (! l.abdeckungGesetzt)
        return Konfidenzklasse::unbrauchbar;
    Konfidenzklasse k = Konfidenzklasse::unbrauchbar;
    if ((double) l.abdeckung >= kKonfidenzAbdeckungStark)
        k = Konfidenzklasse::stark;
    else if ((double) l.abdeckung >= kKonfidenzAbdeckungMittel)
        k = Konfidenzklasse::mittel;
    else if (l.abdeckung > 0.0f)
        k = Konfidenzklasse::schwach;

    // 2. Was der Erzeuger nicht beurteilen kann, deckelt.
    if (! (l.sessionBekannt && l.passageBekannt
           && l.coverageBekannt && l.alignmentBekannt))
        k = deckle (k, Konfidenzklasse::mittel);

    // 3. Harte Maengel.
    const int harte = (l.sessionBekannt   && l.sessionHart   ? 1 : 0)
                    + (l.passageBekannt   && l.passageHart   ? 1 : 0)
                    + (l.coverageBekannt  && l.coverageHart  ? 1 : 0)
                    + (l.alignmentBekannt && l.alignmentHart ? 1 : 0);
    if (harte >= 2)
        return Konfidenzklasse::unbrauchbar;
    if (harte == 1)
        k = deckle (k, Konfidenzklasse::schwach);

    // 4. Duenne Verteilung.
    if (l.verteilungFenster < kKonfidenzFensterStark)
        k = deckle (k, Konfidenzklasse::mittel);

    return k;
}

} // namespace nakama::analyse
