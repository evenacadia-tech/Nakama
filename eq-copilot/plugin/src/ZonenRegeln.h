// M3a — geteilte Zonen-Geometrie zwischen AnalyseEngine (Zeitverlauf-Ticks)
// und Diagnose (Befundregeln §5.10.4).
//
// EINE Quelle, damit die Engine-Persistenz („in wie viel % der Musikzeit lag
// die Zone jenseits ihrer Schwelle?") und die Diagnose-Schwelle nie
// auseinanderlaufen. Werte sind versionierte Startwerte — Änderung nur
// zusammen mit kMetricsVersion UND kDiagnoseVersion, nie still.
#pragma once

namespace eqcop
{

// Slot-Reihenfolge = Index in MessSnapshot::zonenZeit.
inline constexpr int kZonenAnzahl = 4;
inline constexpr int kZonenSlotMittenLoch = 0;
inline constexpr int kZonenSlotMulm       = 1;
inline constexpr int kZonenSlotHaerte     = 2;
inline constexpr int kZonenSlotHoehenHype = 3;

struct ZonenGeometrie
{
    double schulterLoVon, schulterLoBis;
    double zoneVon, zoneBis;
    // schulterHiVon == 0 ⇒ keine Schulterlinie: delta = Zone − Lo-Schulter
    // (Höhen-Hype: Air gegen Präsenz, §5.10.4).
    double schulterHiVon, schulterHiBis;
    double schwelleDb;   // >0: Zone ÜBER der Linie ist der Befund; <0: darunter
};

inline constexpr ZonenGeometrie kZonen[kZonenAnzahl] = {
    { 150.0,  400.0,   500.0,  2000.0,  2500.0,  5000.0, -3.0 },  // Mitten-Loch
    {  50.0,  100.0,   120.0,   300.0,   400.0,   800.0,  4.0 },  // Mulm
    { 1000.0, 2000.0,  2500.0,  5000.0,  6000.0, 10000.0, 4.0 },  // Härte
    { 2000.0, 6000.0,  8000.0, 14000.0,     0.0,     0.0,  1.0 }, // Höhen-Hype
};

} // namespace eqcop
