// SONDE-009 (S12-13) — Bandvertrag: die zwei eingefrorenen Gitter und die eine
// Quantisierungsregel.  Entwurf §33.1, §33.2, §53.9.
//
// WOZU DIESE DATEI EXISTIERT.  Der v3-Vertrag (SONDE-005a/005b) beschreibt, wie
// ein Bandsatz auf der Leitung AUSSIEHT: `Bandwerte` mit Gitter, Kodierung,
// Gueltigkeitsbitmap und Saettigungsbit.  Was fehlte, war die Seite, die diese
// Felder FUELLT.  Hier steht sie - JUCE-frei, damit sie headless pruefbar ist,
// und ohne jede Kenntnis von FFT, Fenstern oder Transport: dieses Blatt kennt
// nur Zahlen.
//
// ⚠️ DAS GITTER HIER IST NICHT DAS GITTER DER `AnalyseEngine`.  Beide tragen
// 221 Baender à 1/24 Oktave, und sie sind trotzdem verschieden:
//   * `AnalyseEngine` (M1) verankert bei 30 Hz - `edges = 30*2^(k/24)`, exakt
//     die Achse von `tools/analyze-track.py`.  Sie ist der Massstab der
//     Golden-Kreuzvalidierung und darf sich nie bewegen.
//   * Der v3-Vertrag verankert nach IEC 61260-1 bei 1000 Hz.
// Gemessen liegen sie 1,2 % auseinander, also rund ein Fuenftel Bandbreite.
// Wer die eine Achse fuer die andere haelt, verschiebt jede Bandaussage um
// diesen Betrag - still, weil beide Achsen plausibel aussehen.  Deshalb ist die
// FeatureEngine v2 eine SCHICHT NEBEN M1 und kein Umbau von M1.
//
// Die Zahlen selbst stehen in `BandGridZahlen.h` (erzeugt aus den Fixtures,
// gemessen von Kanon-Bein B5).  Hier steht nur, was man mit ihnen tut.
#pragma once

#include "BandGridZahlen.h"

#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace nakama::analyse
{

//==============================================================================
/** Zugriff auf die eingefrorenen Gitter.

    Die Konstanten liegen als Bitmuster; `std::bit_cast` ist die eine Stelle,
    an der daraus wieder `double` wird.  Kein `strtod`, kein Dezimalliteral -
    beide gingen durch einen Parser, und ein Parser ist eine Fehlerquelle
    zwischen Vertrag und Code. */
struct Gitter
{
    static constexpr int evidenzBaender = gitter::kEvidenzBaender;   // 221
    static constexpr int liveBaender    = gitter::kLiveBaender;      //  64

    static constexpr double evidenzKante (int i) noexcept
    {
        return std::bit_cast<double> (gitter::kEvidenzKantenBits[(std::size_t) i]);
    }
    static constexpr double evidenzMitte (int i) noexcept
    {
        return std::bit_cast<double> (gitter::kEvidenzMittenBits[(std::size_t) i]);
    }
    static constexpr double liveKante (int i) noexcept
    {
        return std::bit_cast<double> (gitter::kLiveKantenBits[(std::size_t) i]);
    }
    static constexpr double liveMitte (int i) noexcept
    {
        return std::bit_cast<double> (gitter::kLiveMittenBits[(std::size_t) i]);
    }
    /** Feinbandbereich der Livegruppe `j` — halboffen [von, bisExkl). */
    static constexpr int liveVon (int j) noexcept
    {
        return (int) gitter::kLiveGruppeVon[(std::size_t) j];
    }
    static constexpr int liveBisExkl (int j) noexcept
    {
        return (int) gitter::kLiveGruppeBisExkl[(std::size_t) j];
    }
};

//==============================================================================
/** Kodierungen aus `nakama_telemetry_v1.fbs` (enum `BandEncoding`).  Die Zahlen
    sind die Wire-Werte; `unbekannt = 0` wird von beiden Lesern abgelehnt und
    darf hier deshalb nie herauskommen. */
enum class BandEncoding : std::uint8_t
{
    unbekannt      = 0,
    q_db_0p1_i16   = 1,
    q_db_0p01_i16  = 2,
    float32        = 3,
};

/** Gitterkennung aus demselben `.fbs` (enum `Bandgitter`). */
enum class GitterId : std::uint8_t
{
    unbekannt                  = 0,
    nakama_1_24_oct_30_18k_v1  = 1,
    nakama_log64_v1            = 2,
};

//==============================================================================
// Die Quantisierungsregeln stehen woertlich in
// `eq-copilot/schemas/v3/quantisierung-v1.json`.  Hier ist ihre einzige
// Umsetzung; Kanon-Bein B5 misst sie gegen alle 61 Vektoren der Datei.
//
// 🔑 DREI FALLEN, DIE DIE FIXTURE AUSDRUECKLICH BENENNT:
//
//  1. RUNDUNG VON NULL WEG (`std::round`), NICHT zur geraden Zahl.  Pythons
//     eingebautes `round()` rundet zur geraden Zahl; wer den Erzeuger in Python
//     nachbaut und das uebersieht, erzeugt fuer 0,05 dB eine 0 statt einer 1.
//
//  2. `std::floor(std::abs(x) + 0.5)` ist NICHT dieselbe Regel.  Fuer
//     x = 0.49999999999999994 liefert sie 1 statt 0, weil die Addition selbst
//     schon auf 1.0 rundet.  Die Fixture traegt genau diesen Wert als Vektor.
//
//  3. NICHTENDLICHES WIRD NIE SANITISIERT.  NaN und ±inf ergeben Wert 0 mit
//     `gueltig = false` — die Gueltigkeitsbitmap ist die Wahrheit, nicht der
//     Wert.  Sie werden auch nie saturiert: eine Unendlichkeit ist kein
//     "sehr grosser Messwert", sondern gar keiner.
//
//  4. SAETTIGUNG BLEIBT GUELTIG.  Ein saturierter Wert ist gemessen, nur nicht
//     mehr aufloesbar — er setzt zusaetzlich das Saettigungsbit.  Das ist der
//     Unterschied zu Punkt 3 und der Grund, warum es zwei Bits braucht.

struct Quant16
{
    std::int16_t wert      { 0 };
    bool         gueltig   { false };
    bool         saturiert { false };
};

struct QuantF32
{
    float wert      { 0.0f };
    bool  gueltig   { false };
    bool  saturiert { false };
};

/** Skalierung je i16-Kodierung (Feld `skalierung` der Fixture). */
inline constexpr double skalierungVon (BandEncoding e) noexcept
{
    switch (e)
    {
        case BandEncoding::q_db_0p1_i16:  return 10.0;
        case BandEncoding::q_db_0p01_i16: return 100.0;
        case BandEncoding::float32:       return 1.0;
        case BandEncoding::unbekannt:     break;
    }
    return 0.0;
}

/** dB-Wert auf einen i16-Traeger. `skalierung` ist 10 oder 100. */
inline Quant16 quantisiere16 (double db, double skalierung) noexcept
{
    Quant16 q;
    if (! std::isfinite (db))
        return q;                                  // Wert 0, ungueltig, nicht saturiert

    q.gueltig = true;
    const double roh = std::round (db * skalierung);   // halbe Werte von null weg

    constexpr double kMin = (double) std::numeric_limits<std::int16_t>::min();   // -32768
    constexpr double kMax = (double) std::numeric_limits<std::int16_t>::max();   // +32767
    if (roh > kMax)      { q.wert = (std::int16_t) 32767;  q.saturiert = true; }
    else if (roh < kMin) { q.wert = (std::int16_t) -32768; q.saturiert = true; }
    else                 { q.wert = (std::int16_t) roh; }
    return q;
}

inline Quant16 quantisiere16 (double db, BandEncoding e) noexcept
{
    return quantisiere16 (db, skalierungVon (e));
}

/** binary64 auf binary32 verengen (Kodierung `float32`).

    ⚠️ Die Saettigungsfrage wird VOR der Verengung entschieden, nicht danach.
    Ein `(float) 3.5e38` ist bereits `+inf` - danach liesse sich "ueberlaufen"
    nicht mehr von "der Sender hat inf geschickt" unterscheiden, und genau diese
    Unterscheidung ist der Zweck der zwei Bits. */
inline QuantF32 quantisiereF32 (double x) noexcept
{
    QuantF32 q;
    if (! std::isfinite (x))
        return q;

    q.gueltig = true;
    constexpr double kFltMax = (double) std::numeric_limits<float>::max();
    if (x > kFltMax)       { q.wert =  std::numeric_limits<float>::max(); q.saturiert = true; }
    else if (x < -kFltMax) { q.wert = -std::numeric_limits<float>::max(); q.saturiert = true; }
    else                   { q.wert = (float) x; }   // round-to-nearest-even, IEEE-754
    return q;
}

//==============================================================================
/** Gueltigkeitsbitmap, LSB-first ueber ceil(n/8) Bytes: Band i sitzt in Byte
    i/8, Bit i%8 (Fixture-Regel `bitmap`, `.fbs`-Kommentar zu `gueltig_bitmap`).

    ⚠️ Die Fuellbits des letzten Bytes MUESSEN 0 bleiben.  Sonst erzeugen zwei
    Sender fuer denselben Messwert zwei verschiedene Puffer, und ein
    Bytevergleich waere keine Aussage mehr.  `nullen()` ist deshalb kein
    Komfort, sondern Vertragspflicht - und die einzige Stelle, an der Fuellbits
    ueberhaupt entstehen koennen. */
constexpr std::size_t bitmapBytes (int baender) noexcept
{
    return (std::size_t) ((baender + 7) / 8);
}

inline void bitmapNullen (std::uint8_t* bitmap, int baender) noexcept
{
    for (std::size_t i = 0; i < bitmapBytes (baender); ++i)
        bitmap[i] = 0;
}

inline void bitmapSetze (std::uint8_t* bitmap, int i, bool gueltig) noexcept
{
    const std::uint8_t maske = (std::uint8_t) (1u << (unsigned) (i % 8));
    if (gueltig) bitmap[(std::size_t) (i / 8)] |= maske;
    else         bitmap[(std::size_t) (i / 8)] = (std::uint8_t) (bitmap[(std::size_t) (i / 8)] & ~maske);
}

inline bool bitmapLies (const std::uint8_t* bitmap, int i) noexcept
{
    return (bitmap[(std::size_t) (i / 8)] & (std::uint8_t) (1u << (unsigned) (i % 8))) != 0;
}

//==============================================================================
/** Plausibler Bereich eines Bandwertes in dB (Fixture `plausibler_bereich_db`).

    ⚠️ NICHT zu verwechseln mit `bereich_db`: das ist der TRAEGERumfang - was ein
    i16 mit dieser Skalierung darstellen KANN.  Diese Grenze hier ist, was ein
    Bandwert bedeuten DARF.  -144 dB ist die Untergrenze der NaN-Ehrlichkeit:
    darunter ist nichts mehr Signal, sondern Rechenrauschen.

    Der ERZEUGER nutzt sie anders als der Leser: der Leser lehnt ab, der
    Erzeuger sendet erst gar nicht.  Ein Band unter der Grenze wird ungueltig
    gestellt, statt Rauschen als Messwert auszugeben. */
inline constexpr double kPlausibelMinDb = -144.0;
inline constexpr double kPlausibelMaxDb =   24.0;

inline constexpr bool plausibel (double db) noexcept
{
    return db >= kPlausibelMinDb && db <= kPlausibelMaxDb;
}

//==============================================================================
/** Lineare Energie in dB — die eine Umrechnung dieses Blattes.

    §33.1 woertlich: "Energie wird linear integriert und erst danach in dB
    quantisiert."  Wer in dB mittelt, misst etwas anderes: das geometrische
    statt des arithmetischen Mittels.  Diese Funktion ist deshalb bewusst der
    LETZTE Schritt und nimmt eine bereits fertig summierte Energie entgegen.

    Energie <= 0 ist kein Fehler, sondern Stille: sie ergibt -inf und damit
    ueber `quantisiere16` einen ungueltigen Bandwert.  Das ist die richtige
    Aussage - "hier war nichts" ist nicht "hier waren -300 dB". */
inline double energieAlsDb (double energieLinear) noexcept
{
    if (! (energieLinear > 0.0))            // faengt auch NaN: (NaN > 0) ist false
        return -std::numeric_limits<double>::infinity();
    return 10.0 * std::log10 (energieLinear);
}

} // namespace nakama::analyse
