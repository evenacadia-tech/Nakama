// Oberflaeche des Termin-B-Messgeraets.
//
// ZWEI REGELN, beide teuer bezahlt:
//
// 1. Sie zeigt AUSSCHLIESSLICH Gemessenes. Jede Zeile ohne Messwert sagt
//    ausdruecklich "noch nicht gesehen" statt eine 0 zu zeigen. Eine Anzeige,
//    die "0" fuer "unbekannt" schreibt, waere genau die Luege, gegen die
//    dieses Geraet gebaut ist.
//
// 2. Die Fensterhoehe wird aus dem Inhalt BERECHNET, nicht geraten. Die erste
//    Fassung war 660x520 und schnitt 49 px ab - ausgerechnet die
//    Automationszeilen, also genau die Messwerte, fuer die der Termin
//    existiert; die Knoepfe lagen zusaetzlich auf ihnen (T2-Befund 21.08.2026).
//    Deshalb liefert `zeilen()` die Zeilenliste, `benoetigteHoehe()` rechnet
//    daraus die Fensterhoehe, und der Selbsttest prueft beides gegeneinander.
//    Eine rein visuelle Flaeche ohne Bildbeweis wird nicht noch einmal
//    abgenommen.
#pragma once

#include "HostProbeProcessor.h"

namespace eqcop::hostprobe
{

/** Eine Zeile der Anzeige. `istKopf` bekommt zusaetzlichen Abstand darueber. */
struct Anzeigezeile
{
    juce::String  name;
    juce::String  wert;
    juce::Colour  farbe;
    bool          istKopf { false };
    bool          eingerueckt { false };
    /** Freitext-Hinweis ohne Beschriftung. Genau EINE solche Zeile ist erlaubt,
        und der Selbsttest zaehlt sie - sonst waere "jede Wertzeile hat eine
        Beschriftung" eine Regel mit stiller Hintertuer (T2-Runde 2). */
    bool          istHinweis { false };
};

/** Layoutmasse an EINER Stelle - sonst rechnen paint(), resized() und der
    Test jeweils ihre eigene Wahrheit aus. */
namespace masse
{
    inline constexpr int fensterBreite   = 680;
    inline constexpr int kopfHoehe       = 56;   // Titel + Untertitel
    inline constexpr int zeile           = 21;
    inline constexpr int kopfAbstand     = 8;    // zusaetzlich ueber einer Kopfzeile
    inline constexpr int meldungHoehe    = 34;   // Band fuer den Dateipfad
    inline constexpr int knopfStreifen   = 56;
    inline constexpr int rand            = 16;
}

class HostProbeEditor final : public juce::AudioProcessorEditor,
                              private juce::Timer
{
public:
    explicit HostProbeEditor (HostProbeProcessor&);
    ~HostProbeEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Die Zeilen, die die Anzeige gerade zeigt. Oeffentlich, damit der
        Selbsttest sie zaehlen und ihre Hoehe pruefen kann. */
    static juce::Array<Anzeigezeile> zeilen (const Messstand&, bool brueckeLiefert);

    /** Fensterhoehe, die diese Zeilenliste braucht - inklusive Meldungsband
        und Knopfstreifen. */
    static int benoetigteHoehe (const juce::Array<Anzeigezeile>&);

    /** Unterkante der letzten Textzeile. Muss ueber dem Knopfstreifen liegen. */
    static int inhaltsUnterkante (const juce::Array<Anzeigezeile>&);

private:
    void timerCallback() override;
    static juce::String signatur (const Messstand&);

    HostProbeProcessor& prozessor;
    juce::TextButton berichtKnopf { "Bericht sichern" };
    juce::TextButton resetKnopf   { "Messung zuruecksetzen" };
    juce::String     letzteMeldung;
    juce::String     letzteSignatur;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HostProbeEditor)
};

} // namespace eqcop::hostprobe
