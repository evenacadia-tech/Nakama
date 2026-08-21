#include "HostProbeEditor.h"

namespace eqcop::hostprobe
{

namespace
{
    const juce::Colour kGrund { 0xff14171b };
    const juce::Colour kText  { 0xffe6e9ee };
    const juce::Colour kMatt  { 0xff8a929c };
    const juce::Colour kJa    { 0xff5fd48a };
    const juce::Colour kNein  { 0xffd4715f };
    const juce::Colour kOffen { 0xffd8b24a };

    /** „noch nicht gesehen" ist eine eigene Aussage, keine 0. */
    juce::String zahlOderOffen (juce::int64 wert, juce::int64 gesehen)
    {
        return gesehen == 0 ? juce::String ("noch nicht gesehen") : juce::String (wert);
    }
}

HostProbeEditor::HostProbeEditor (HostProbeProcessor& p)
    : juce::AudioProcessorEditor (&p), prozessor (p)
{
    berichtKnopf.onClick = [this]
    {
        const auto datei = prozessor.berichtSchreiben();
        letzteMeldung = datei.existsAsFile()
                          ? "Bericht: " + datei.getFullPathName()
                          : "Bericht KONNTE NICHT geschrieben werden: "
                                + HostProbeProcessor::berichtsOrdner().getFullPathName();
        repaint();
    };

    resetKnopf.onClick = [this]
    {
        prozessor.messungZuruecksetzen();
        letzteMeldung = "Messung zurueckgesetzt - der naechste Block faengt neu an";
        repaint();
    };

    addAndMakeVisible (berichtKnopf);
    addAndMakeVisible (resetKnopf);

    setSize (660, 520);
    startTimerHz (6);
}

void HostProbeEditor::resized()
{
    auto unten = getLocalBounds().removeFromBottom (72).reduced (16, 12);
    berichtKnopf.setBounds (unten.removeFromLeft (200).withHeight (30));
    unten.removeFromLeft (12);
    resetKnopf.setBounds (unten.removeFromLeft (220).withHeight (30));
}

juce::String HostProbeEditor::signatur (const Messstand& s)
{
    juce::String sig;
    sig << s.bloecke << "|" << s.senkeAufrufe << "|" << s.bloeckeMitKontext << "|"
        << s.bloeckeOhneKontext << "|" << s.zeitspruengeVor << "|" << s.zeitspruengeZurueck << "|"
        << s.automationPunkte << "|" << s.maxPunkteProBlock << "|" << s.bloeckeMitMehrpunkt << "|"
        << (int) s.latenzJeGemeldet << "|" << s.bloeckeOffline << "|" << s.bloeckeDouble << "|"
        << (int) s.spieltGerade << "|" << (int) s.schleifeAktiv;
    return sig;
}

void HostProbeEditor::timerCallback()
{
    // Nur neu malen, wenn sich wirklich etwas geaendert hat (Bauplan §1.4:
    // im Leerlauf exakt nichts tun).
    const auto sig = signatur (prozessor.messstand());
    if (sig == letzteSignatur) return;
    letzteSignatur = sig;
    repaint();
}

void HostProbeEditor::paint (juce::Graphics& g)
{
    const auto s = prozessor.messstand();

    g.fillAll (kGrund);
    g.setColour (kText);
    g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
    g.drawText ("Nakama Host-Probe - Termin B", 16, 12, getWidth() - 32, 22,
                juce::Justification::centredLeft);

    g.setFont (juce::FontOptions (12.0f));
    g.setColour (kMatt);
    g.drawText ("Misst, was FL wirklich liefert. Greift nicht ins Audio ein.",
                16, 34, getWidth() - 32, 18, juce::Justification::centredLeft);

    int y = 62;
    const int zeile = 21;
    const int spalte = 300;

    auto schreibe = [&] (const juce::String& name, const juce::String& wert, juce::Colour farbe)
    {
        g.setColour (kMatt);
        g.setFont (juce::FontOptions (13.0f));
        g.drawText (name, 16, y, spalte - 20, zeile, juce::Justification::centredLeft);
        g.setColour (farbe);
        g.drawText (wert, spalte, y, getWidth() - spalte - 16, zeile, juce::Justification::centredLeft);
        y += zeile;
    };

    auto jaNein = [] (bool b) { return b ? kJa : kNein; };

    // Der wichtigste Zustand zuerst: liefert die Bruecke ueberhaupt?
    const bool bruecke = prozessor.brueckeLiefert();
    schreibe ("Hostbruecke liefert", bruecke ? "JA" : "NEIN - Bau ohne Bridge-Patch?", jaNein (bruecke));

    schreibe ("Bloecke verarbeitet", juce::String (s.verarbeiteteBloecke), kText);
    schreibe ("Senke-Aufrufe", juce::String (s.senkeAufrufe)
                  + (s.senkeAufrufe != s.verarbeiteteBloecke ? "  (weicht ab - siehe Bericht)" : ""),
              s.senkeAufrufe == s.verarbeiteteBloecke ? kText : kOffen);

    schreibe ("Bloecke MIT ProcessContext", juce::String (s.bloeckeMitKontext),
              s.bloeckeMitKontext > 0 ? kJa : kOffen);
    schreibe ("Bloecke OHNE ProcessContext", juce::String (s.bloeckeOhneKontext),
              s.bloeckeOhneKontext > 0 ? kOffen : kMatt);

    y += 6;
    g.setColour (kMatt);
    g.drawText ("Was FL im Context ausfuellt (immer / mindestens einmal)", 16, y, getWidth() - 32, zeile,
                juce::Justification::centredLeft);
    y += zeile;

    auto flagZeile = [&] (const juce::String& name, bool immer, bool jeMal)
    {
        const auto wert = s.bloeckeMitKontext == 0 ? juce::String ("noch kein Context")
                        : immer  ? juce::String ("immer")
                        : jeMal  ? juce::String ("manchmal")
                                 : juce::String ("nie");
        schreibe ("  " + name, wert,
                  s.bloeckeMitKontext == 0 ? kMatt : immer ? kJa : jeMal ? kOffen : kNein);
    };

    flagZeile ("projectTimeSamples", s.immerGueltig.projektZeit, s.jeGueltig.projektZeit);
    flagZeile ("continousTimeSamples", s.immerGueltig.fortlaufend, s.jeGueltig.fortlaufend);
    flagZeile ("Tempo", s.immerGueltig.tempo, s.jeGueltig.tempo);
    flagZeile ("PPQ-Position", s.immerGueltig.ppq, s.jeGueltig.ppq);
    flagZeile ("Taktbeginn", s.immerGueltig.taktbeginn, s.jeGueltig.taktbeginn);
    flagZeile ("Cycle-Grenzen", s.immerGueltig.schleife, s.jeGueltig.schleife);

    y += 6;
    schreibe ("Zeitspruenge vorwaerts", zahlOderOffen (s.zeitspruengeVor, s.bloeckeMitKontext),
              s.zeitspruengeVor > 0 ? kOffen : kMatt);
    schreibe ("Zeitspruenge rueckwaerts", zahlOderOffen (s.zeitspruengeZurueck, s.bloeckeMitKontext),
              s.zeitspruengeZurueck > 0 ? kOffen : kMatt);
    schreibe ("Transport / Schleife",
              juce::String (s.spieltGerade ? "spielt" : "steht") + " / "
                  + (s.schleifeAktiv ? "Schleife an" : "Schleife aus"), kText);
    schreibe ("Offline-Bloecke (Render)", zahlOderOffen (s.bloeckeOffline, s.bloecke),
              s.bloeckeOffline > 0 ? kJa : kMatt);
    schreibe ("float / double", juce::String (s.bloeckeFloat) + " / " + juce::String (s.bloeckeDouble),
              s.bloeckeDouble > 0 ? kJa : kText);

    y += 6;
    g.setColour (kText);
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.drawText ("Samplegenaue Automation - dafuer wurde die Bruecke gebaut", 16, y, getWidth() - 32, zeile,
                juce::Justification::centredLeft);
    y += zeile;

    schreibe ("  Automationspunkte", zahlOderOffen (s.automationPunkte, s.automationPunkte),
              s.automationPunkte > 0 ? kJa : kOffen);
    schreibe ("  Punkte im dichtesten Block", zahlOderOffen (s.maxPunkteProBlock, s.automationPunkte),
              s.maxPunkteProBlock > 1 ? kJa : kOffen);
    schreibe ("  Mehr als EIN Punkt je Block?",
              s.automationPunkte == 0 ? juce::String ("noch keine Automation gesehen")
                                      : (s.bloeckeMitMehrpunkt > 0
                                             ? "JA - in " + juce::String (s.bloeckeMitMehrpunkt) + " Bloecken"
                                             : "bisher nie"),
              s.automationPunkte == 0 ? kMatt : (s.bloeckeMitMehrpunkt > 0 ? kJa : kOffen));
    schreibe ("  Offsets (kleinster/groesster)",
              s.automationPunkte == 0 ? juce::String ("noch keine Automation gesehen")
                                      : juce::String (s.kleinsterOffset) + " / " + juce::String (s.groesterOffset),
              s.automationPunkte == 0 ? kMatt : kText);

    y += 6;
    schreibe ("Presentation-Latency gemeldet?",
              s.latenzJeGemeldet
                  ? juce::String ("JA (Eingang Bus " + juce::String (s.latenzEingangBus) + " = "
                                  + juce::String ((int) s.latenzEingangWert) + ", Ausgang Bus "
                                  + juce::String (s.latenzAusgangBus) + " = "
                                  + juce::String ((int) s.latenzAusgangWert) + ")")
                  : juce::String ("bisher NIE - das ist ein gueltiger Befund"),
              s.latenzJeGemeldet ? kJa : kOffen);

    if (letzteMeldung.isNotEmpty())
    {
        g.setColour (kMatt);
        g.setFont (juce::FontOptions (11.0f));
        g.drawFittedText (letzteMeldung, 16, getHeight() - 104, getWidth() - 32, 30,
                          juce::Justification::topLeft, 2);
    }
}

} // namespace eqcop::hostprobe
