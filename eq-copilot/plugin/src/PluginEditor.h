// EQ-Copilot-Editor — die Kernfunktion ist die
// Graph-Diagnose mit konkreten Umsetzungsempfehlungen; alles andere ist
// Beiwerk und darf nicht im Weg stehen. Der eigenständige Materialbaukasten
// (EqCopilotAssetKit.h) zerlegt die Front in unabhängig verschiebbare
// Vektorbauteile: Analyseglas, LUFS-/Peak-Leisten, Werkzeugbett, Tasten,
// Statuszellen, LEDs, Marker und Markenmodul. Das Fenster hält 750:520 fest.
//  · KEIN Setup-Gefühl: die Messpunkt-Bindung (Aufgabe/Name/Paar) ist aus dem
//    Dauer-Kopf in ein „Messpunkt…"-Popover gewandert — laden, Musik spielen,
//    Befund bekommen.
//  · Der Hinweis-Knopf trägt die DIAGNOSE (Befundkarten §7.3: Gemessen /
//    Wirkung / Tu / Warum / Hören / Sicherheit — Lernsprache §7.4).
//  · Marker bleiben Dreiecke, Abdeckung bleibt Zonenleiste (M1.1) — jetzt
//    auf dunkler Display-Palette (copilot_*-Tokens).
//
// UNVERHANDELBAR (M1-Abnahme): alles hier ist Anzeige auf der Snapshot-KOPIE.
// Glättung, Ansicht, Vergleich, Y-Bereich, Diagnose existieren nur in dieser
// Klasse — die AnalyseEngine kennt keinen dieser Zustände.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "Diagnose.h"
#include "EqCopilotAssetKit.h"

namespace eqcop
{

// Material-LookAndFeel mit skalierenden Schriften — ohne ihn blieben
// ComboBox/Knöpfe/Popup auf der festen LookAndFeel-Größe (Probefahrt: „viel zu
// klein"); Popups/CallOuts folgen der dunklen Analysepalette.
struct CopilotLookAndFeel : juce::LookAndFeel_V4
{
    CopilotLookAndFeel();
    float faktor = 1.0f;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int) override;
    juce::Font getPopupMenuFont() override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool hervorgehoben, bool gedrueckt) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool hervorgehoben, bool gedrueckt) override;
    void drawComboBox (juce::Graphics&, int breite, int hoehe, bool gedrueckt,
                       int knopfX, int knopfY, int knopfB, int knopfH,
                       juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawCallOutBoxBackground (juce::CallOutBox&, juce::Graphics&,
                                   const juce::Path&, juce::Image&) override;
};

class EqCopilotEditor : public juce::AudioProcessorEditor,
                        private juce::Timer,
                        private juce::ComboBox::Listener
{
public:
    explicit EqCopilotEditor (EqCopilotProcessor&);
    ~EqCopilotEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    bool sourcesLabelAktivFuerTest() const { return sourcesLabelFeld.isEnabled(); }
    juce::String sourcesBedienstatusFuerTest() const { return sourcesBedienstatus; }
    void sourcesLabelSchreibversuchFuerTest (const juce::String& text)
    {
        sourcesLabelFeld.setText (text, juce::dontSendNotification);
        uebernehmeSourcesLabel();
    }
#endif

private:
    void timerCallback() override;
    void comboBoxChanged (juce::ComboBox*) override;
    void zeigeHinweise();
    void zeigeKonflikt();
    void zeigeMesspunkt();
    juce::Rectangle<int> graphFlaeche() const;

    // SONDE-012, Gen Flaeche 1. Diese Ansicht ist absichtlich eine
    // funktionale Instrumentenskizze: keine visuelle Richtung, aber jede
    // sichtbare Information hat eine Quelle und jeder Knopf einen echten
    // Gegenpfad.
    bool istMainFlaeche() const;
    void wechsleFlaecheWennNoetig();
    void aktualisiereSourcesSteuerung();
    void uebernehmeSourcesLabel();
    void paintMainFlaeche (juce::Graphics&);
    juce::Rectangle<int> sourcesSpalte() const;
    std::vector<juce::Rectangle<int>> sourcesZeilen() const;

    // ── Hör-Markierung (Konzept v2 §6): Latch lebt im Editor ──
    // Klick auf [Solo]/[Puls] einer Befundkarte = Latch an/aus; endet hart bei
    // Fensterschluss, Samplerate-Wechsel, Freilauf-Meldung und 10-min-Totmann.
    // Interims-UI im alten Editor (NAKAMA §7.6 bringt Marker-Glyphe + Halten):
    // Aus-Knopf in der Statuszeile + Feld-Tönung im Graph, puls-phasensynchron.
    void schalteMarkierung (const Befund&, MarkierungsModus);
    void markierungBeenden (const juce::String& grund);
    bool markierungIstAktiv (const Befund&, MarkierungsModus) const;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseDown (const juce::MouseEvent&) override;

    // Zwei Skalen, sauber getrennt:
    //  · geraet(): Geräteeinheiten → Pixel (Figma-Raster 750 breit; das
    //    Fenster hält 750:520, alles skaliert verzerrungsfrei mit).
    //  · ui(): Schriftskala, 1.0 an der Basisbreite 900 px (entspricht der
    //    bisherigen Lesegröße), wächst mit dem Fenster.
    float geraet() const { return (float) getWidth() / skin::kEinheitB; }
    float ui() const     { return juce::jlimit (0.85f, 2.2f, (float) getWidth() / 900.0f); }
    // Einheiten-Helfer für Layout/paint.
    int E (float einheiten) const { return juce::roundToInt (einheiten * geraet()); }

    std::array<double, kLtasBaender> anzeigeKurve (const std::array<double, kLtasBaender>& roh) const;
    void zeichneKurve (juce::Graphics&, const MessSnapshot&,
                       const std::array<double, kLtasBaender>& kurve,
                       juce::Colour, float staerke,
                       float xLinks, float xRechts, float yOben, float yUnten) const;

    EqCopilotProcessor& processor;

    // M1-Werkzeugzeile (die Messpunkt-Bindung lebt im „Messpunkt…"-Popover)
    juce::ComboBox glaettungWahl;
    juce::TextButton ansichtKnopf, festhaltenKnopf, vergleichWegKnopf, resetKnopf, hinweisKnopf;
    juce::TextButton messpunktKnopf;
    // M2: erscheint NUR, wenn der Broker per heartbeat_ack einen Kennungs-
    // Konflikt meldet (FL-Duplikation) — rot, nie still (Plan §8.4).
    juce::TextButton konfliktKnopf;

    // Genau ein gemeinsamer Aktionssatz fuer genau das aktuelle Hauptziel.
    // Referenzzeilen tragen nie eigene, spaeter veraltende Knopf-Callbacks.
    juce::TextButton sourcesAktionKnopf, sourcesRecoveryKnopf;
    juce::TextEditor sourcesLabelFeld;
    SourcesModel::Sicht sourcesAnzeige;
    juce::uint64 sourcesRevision = 0;
    std::string sourcesAktionsZiel;
    juce::String sourcesBedienstatus;
    bool sourcesSchreibfehlerAktiv = false;
    bool mainFlaecheAktiv = false;

    // M3: Diagnose-Cache — 1×/s auf der Snapshot-Kopie neu gerechnet
    // (Anzeige-Takt ist 30 Hz; die Regeln brauchen ihn nicht).
    std::vector<Befund> befunde;
    juce::uint32 befundeStandMs = 0;

    // FPS-Fix (m4): EIN Snapshot-Zug pro Timer-Tick in `anzeige`; gemalt wird
    // nur bei neuer Revision oder UI-Änderung — der 30-Hz-Timer ist nur noch
    // der Poll, nicht mehr der Maltakt. Im Leerlauf ruht der Editor komplett.
    MessSnapshot anzeige;
    PipeClient::Snapshot pipeAnzeige;
    StatsSnapshot statsAnzeige;
    juce::uint64 anzeigeRevision = 0;
    bool anzeigeInitial = false;
    bool uiDirty = true;              // Werkzeug-/Ansicht-Änderungen
    int  letzterPipeStatus = -1;
    bool meldungWarSichtbar = false;

    // Hör-Markierung: Latch-Zustand (eingefrorene Engage-Parameter, Konzept §2)
    MarkierungsModus markModus = MarkierungsModus::aus;
    BefundKlasse markKlasse = BefundKlasse::resonanz;
    double markVon = 0.0, markBis = 0.0, markSchwerpunkt = 0.0;
    double markEngageSr = 0.0;
    juce::uint32 letzteInteraktionMs = 0;
    juce::TextButton markierungAusKnopf;

    // Anzeige-Zustand (beeinflusst ausschließlich paint())
    bool neigungsAnsicht = false;
    int  glaettFenster = 5;                 // Bänder: 1/6-Okt-Default („mittel")
    bool vergleichAktiv = false;
    std::array<double, kLtasBaender> vergleichRohDb {};
    juce::String statusMeldung;
    juce::uint32 statusMeldungBisMs = 0;    // Meldung verdrängt Kennzahlen nur kurz
    // read-only-State (SONDE-006, Vertrag nakama-state-v2.md §5): vom Timer
    // gepollt; Anzeige-Pflicht „Capability-Degradation" (Entwurf §0.4).
    bool stateNurLesenAnzeige = false;
    juce::String stateGrundAnzeige;
    double yObenDb = -20.0;                 // stabiler Y-Bereich: wächst nur
    bool   yInitialisiert = false;

    CopilotLookAndFeel lnf;
    skin::Frame front;                      // gecachte Gerätefront (Resize-stabil)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqCopilotEditor)
};

} // namespace eqcop
