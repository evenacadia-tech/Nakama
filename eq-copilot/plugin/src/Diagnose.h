// M3-KERN — deterministische Graph-Diagnose mit konkreten Umsetzungsempfehlungen.
//
// USER-VORGABE 2026-08-14: DAS ist die Kernfunktion des Plugins — „sag mir,
// was an meinem Graphen schlecht ist, mit konkreten Umsetzungsempfehlungen".
// Kein Setup nötig: laden → Musik spielen → Befundkarten. Alles Weitere
// (Profile, Paare, Broker) ist optionale Infrastruktur.
//
// Regeln (metrics m3): Plan §5.10.3/§5.10.4/§5.10.6/§5.10.7, §5.8 („nicht
// jedes Problem braucht EQ"), §7.3 (Karten-Struktur: Gemessen / Wirkung /
// Versuch / Gegenhören / Konfidenz getrennt), §7.4 (Lernsprache Tu/Warum/
// Hören, Zahlen als Startwert-Bereiche in Klammern).
//
// Alle Zonen-Befunde sind EIGENKURVEN-relativ (Zone gegen die log-f-Linie
// ihrer eigenen Schultern) — sie brauchen keinen kalibrierten Zielkorridor
// und behaupten keinen. Abdeckungs-Gates (§5.10.2) verhindern Aussagen über
// unbespielte Bereiche; der Falsch-Positiv-Riegel ist Teil des Golden-Tests
// (Pink Noise ⇒ null Karten).
//
// Reine Funktionen ohne Zustand: laufen auf der Snapshot-KOPIE (Editor,
// Snapshot-Datei, Golden-Test) — nie im Audiothread, nie in der Engine.
#pragma once

#include "AnalyseEngine.h"
#include <vector>

namespace eqcop
{

// m4-2026-08-15 (M3a): Notennamen in FL-Zählung (MIDI 60 = C5) + Charakter
// aus P95−P50 in Resonanzkarten; Zonen-Persistenz aus dem echten Zeitverlauf
// (ZonenRegeln.h) statt der pauschalen 0,5-Deckelung — Zonen-Befunde können
// jetzt ehrlich „hoch" erreichen, wenn die Evidenz da ist.
// m4.1 (Kalibrier-Runde 1, Testtrack): bei WELLIGEM Pegel (P95−P50 > 10 dB)
// wird die Erstidee auch fuer dauerhafte Resonanzen DYNAMISCH — der Fall war
// die eigene Melodie (A#5-Tonika): „dauerhaft" traf zu, aber ein fester Cut
// haette die Melodie gedaempft; das Wogen stand schon auf der Karte.
inline constexpr const char* kDiagnoseVersion = "m4.1-2026-08-15";

enum class BefundKlasse
{
    resonanz,       // schmaler Dauerklang/zeitweiser Klang sticht heraus (§5.10.3)
    mittenLoch,     // 500–2000 Hz unter der Schulterlinie („Smile-EQ"-Scoop)
    mulm,           // 120–300 Hz staut sich über der Schulterlinie
    haerte,         // 2,5–5 kHz beißt über der Schulterlinie
    hoehenHype      // Luft/Air glitzert unnatürlich über den Präsenzen
};

enum class Konfidenz { niedrig, mittel, hoch };

// Eine Befundkarte nach §7.3 — Messsatz, Hypothese und Vorschlag sind
// getrennte Felder und dürfen nie in einem Satz verschmelzen.
struct Befund
{
    BefundKlasse klasse = BefundKlasse::resonanz;
    // Nur für klasse==resonanz relevant: trennt statische von dynamischer
    // Erstidee und hebt die Priorität (§5.10.6).
    bool dauerhaft = false;
    double fVon = 0.0, fBis = 0.0;      // betroffener Bereich (Hz)
    double fSchwerpunkt = 0.0;          // stärkste Stelle (Band-Center, nie feiner)
    double staerkeDb = 0.0;             // gemessener Über-/Fehlbetrag in dB

    juce::String titel;                 // Lernsprache, mit Frequenzangabe
    juce::String gemessen;              // nur Zahlen/Fakten der Messung
    juce::String wirkung;               // mögliche Wirkung — Hypothese, markiert
    juce::String tu;                    // konkreter Versuch mit Startwerten (Bereich)
    juce::String warum;                 // der musikalische Grund, ein Satz
    juce::String hoeren;                // erwartetes Hörergebnis + Stop-Bedingung

    Konfidenz konfidenz = Konfidenz::niedrig;
    juce::String konfidenzGrund;        // welche Komponente begrenzt (§5.10.7)
    bool topRang = false;               // Top-Drei (§5.9); Rest = „Beobachten"
};

// Diagnose auf einem Mess-Snapshot. `rolle` ist der Sensor-Kontext
// ("sensor"/"hub"/"pre"/"post") — aktuell fließt er nur in Formulierungen
// ein; alle Regeln sind eigenkurven-relativ und damit überall wohldefiniert.
// Liefert eine leere Liste, solange der Zustand nicht messbereit ist.
std::vector<Befund> diagnose (const MessSnapshot& m, const juce::String& rolle);

// Kompakter Einzeiler für den Hinweis-Knopf: „2 Auffälligkeiten" o. ä.
juce::String diagnoseKnopfText (const std::vector<Befund>& befunde);

} // namespace eqcop
