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

    /** Fasst eine Richtung der Latenztabelle in EINE Zeile. Nennt nur
        tatsaechlich gemeldete Busse - es wird kein Bus -1 erfunden. */
    juce::String latenzZeile (const Messstand::LatenzEintrag* eintraege)
    {
        juce::StringArray teile;
        int weitere = 0;
        for (int bus = 0; bus < eqcop::hostbruecke::kMaxBusse; ++bus)
        {
            if (! eintraege[bus].gemeldet) continue;
            if (teile.size() < 3) teile.add ("Bus " + juce::String (bus) + " = "
                                             + juce::String ((int) eintraege[bus].samples));
            else ++weitere;
        }
        if (teile.isEmpty()) return "nicht gemeldet";
        auto text = teile.joinIntoString (" · ");
        if (weitere > 0) text << " · +" << weitere << " weitere";
        return text;
    }
}

//==============================================================================
juce::Array<Anzeigezeile> HostProbeEditor::zeilen (const Messstand& s, bool brueckeLiefert)
{
    juce::Array<Anzeigezeile> z;
    auto fuege = [&z] (juce::String name, juce::String wert, juce::Colour farbe,
                       bool kopf = false, bool ein = false)
    {
        z.add (Anzeigezeile { std::move (name), std::move (wert), farbe, kopf, ein });
    };

    // Der wichtigste Zustand zuerst.
    fuege ("Hostbruecke liefert", brueckeLiefert ? "JA" : "NEIN - Bau ohne Bridge-Patch?",
           brueckeLiefert ? kJa : kNein);
    fuege ("Bloecke verarbeitet", juce::String (s.verarbeiteteBloecke), kText);
    fuege ("Senke-Aufrufe", juce::String (s.senkeAufrufe)
               + (s.senkeAufrufe != s.verarbeiteteBloecke ? "  (weicht ab - siehe Bericht)" : ""),
           s.senkeAufrufe == s.verarbeiteteBloecke ? kText : kOffen);
    fuege ("Bloecke MIT ProcessContext", juce::String (s.bloeckeMitKontext),
           s.bloeckeMitKontext > 0 ? kJa : kOffen);
    fuege ("Bloecke OHNE ProcessContext", juce::String (s.bloeckeOhneKontext),
           s.bloeckeOhneKontext > 0 ? kOffen : kMatt);

    fuege ("Was FL im Context ausfuellt", "", kMatt, true);
    auto flag = [&] (const juce::String& name, bool immer, bool jeMal)
    {
        const auto wert = s.bloeckeMitKontext == 0 ? juce::String ("noch kein Context")
                        : immer ? juce::String ("immer")
                        : jeMal ? juce::String ("manchmal")
                                : juce::String ("nie");
        fuege (name, wert,
               s.bloeckeMitKontext == 0 ? kMatt : immer ? kJa : jeMal ? kOffen : kNein,
               false, true);
    };
    flag ("projectTimeSamples", s.immerGueltig.projektZeit, s.jeGueltig.projektZeit);
    flag ("continousTimeSamples", s.immerGueltig.fortlaufend, s.jeGueltig.fortlaufend);
    flag ("Tempo", s.immerGueltig.tempo, s.jeGueltig.tempo);
    flag ("PPQ-Position", s.immerGueltig.ppq, s.jeGueltig.ppq);
    flag ("Taktbeginn", s.immerGueltig.taktbeginn, s.jeGueltig.taktbeginn);
    flag ("Cycle-Grenzen", s.immerGueltig.schleife, s.jeGueltig.schleife);

    fuege ("Zeit und Transport", "", kMatt, true);
    fuege ("Zeitspruenge vorwaerts (Seek ODER Smart Disable)",
           zahlOderOffen (s.zeitspruengeVor, s.bloeckeMitKontext),
           s.zeitspruengeVor > 0 ? kOffen : kMatt, false, true);
    fuege ("Zeitspruenge rueckwaerts (Seek ODER Schleife)",
           zahlOderOffen (s.zeitspruengeZurueck, s.bloeckeMitKontext),
           s.zeitspruengeZurueck > 0 ? kOffen : kMatt, false, true);
    fuege ("Positionswechsel ueber Stop/Play",
           zahlOderOffen (s.spruengeUeberStop, s.bloeckeMitKontext),
           s.spruengeUeberStop > 0 ? kOffen : kMatt, false, true);
    fuege ("Projektzeit negativ gesehen",
           zahlOderOffen (s.projektzeitNegativ, s.bloeckeMitKontext),
           s.projektzeitNegativ > 0 ? kOffen : kMatt, false, true);
    fuege ("Transport / Schleife",
           juce::String (s.spieltGerade ? "spielt" : "steht") + " / "
               + (s.schleifeAktiv ? "Schleife an" : "Schleife aus"), kText, false, true);
    fuege ("Offline-Bloecke (Render)", juce::String (s.bloeckeOffline),
           s.bloeckeOffline > 0 ? kJa : kMatt, false, true);
    fuege ("float / double", juce::String (s.bloeckeFloat) + " / " + juce::String (s.bloeckeDouble),
           s.bloeckeDouble > 0 ? kJa : kText, false, true);

    fuege ("Samplegenaue Automation - dafuer wurde die Bruecke gebaut", "", kText, true);
    fuege ("Automationspunkte", zahlOderOffen (s.automationPunkte, s.automationPunkte),
           s.automationPunkte > 0 ? kJa : kOffen, false, true);
    fuege ("Punkte im dichtesten Block", zahlOderOffen (s.maxPunkteProBlock, s.automationPunkte),
           s.maxPunkteProBlock > 1 ? kJa : kOffen, false, true);
    fuege ("Mehr als EIN Punkt je Block?",
           s.automationPunkte == 0 ? juce::String ("noch keine Automation gesehen")
                                   : (s.bloeckeMitMehrpunkt > 0
                                          ? "JA - in " + juce::String (s.bloeckeMitMehrpunkt) + " Bloecken"
                                          : "bisher nie"),
           s.automationPunkte == 0 ? kMatt : (s.bloeckeMitMehrpunkt > 0 ? kJa : kOffen), false, true);
    fuege ("Offsets (kleinster/groesster)",
           s.automationPunkte == 0 ? juce::String ("noch keine Automation gesehen")
                                   : juce::String (s.kleinsterOffset) + " / " + juce::String (s.groesterOffset),
           s.automationPunkte == 0 ? kMatt : kText, false, true);
    fuege ("Bloecke, denen die Bruecke die Zusicherung entzog",
           juce::String (s.mehrpunktOhneZusicherung) + " von " + juce::String (s.bloeckeOhneZusicherung),
           s.mehrpunktOhneZusicherung > 0 ? kOffen : kMatt, false, true);

    fuege ("Presentation-Latency je Bus", "", kText, true);
    fuege ("Eingang", latenzZeile (s.latenzEingang),
           s.latenzJeGemeldet ? kJa : kOffen, false, true);
    fuege ("Ausgang", latenzZeile (s.latenzAusgang),
           s.latenzJeGemeldet ? kJa : kOffen, false, true);
    fuege ("Meldungen mit unbekanntem Bus verworfen", juce::String (s.verworfeneBusmeldungen),
           s.verworfeneBusmeldungen > 0 ? kOffen : kMatt, false, true);
    if (! s.latenzJeGemeldet)
        fuege ("", "bisher NIE gemeldet - das ist ein gueltiger Befund", kOffen, false, true);

    return z;
}

int HostProbeEditor::inhaltsUnterkante (const juce::Array<Anzeigezeile>& z)
{
    int y = masse::kopfHoehe;
    for (const auto& zeile : z)
    {
        if (zeile.istKopf) y += masse::kopfAbstand;
        y += masse::zeile;
    }
    return y;
}

int HostProbeEditor::benoetigteHoehe (const juce::Array<Anzeigezeile>& z)
{
    return inhaltsUnterkante (z) + masse::meldungHoehe + masse::knopfStreifen;
}

//==============================================================================
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

    // Hoehe aus dem Inhalt rechnen, nicht raten. Der Leerzustand hat die meisten
    // Zeilen (die Latenz-Hinweiszeile faellt weg, sobald gemeldet wurde), also
    // ist er das Mass.
    const auto leer = zeilen (Messstand {}, false);
    setSize (masse::fensterBreite, benoetigteHoehe (leer));
    startTimerHz (6);
}

void HostProbeEditor::resized()
{
    auto unten = getLocalBounds().removeFromBottom (masse::knopfStreifen).reduced (masse::rand, 12);
    berichtKnopf.setBounds (unten.removeFromLeft (200).withHeight (30));
    unten.removeFromLeft (12);
    resetKnopf.setBounds (unten.removeFromLeft (220).withHeight (30));
}

juce::String HostProbeEditor::signatur (const Messstand& s)
{
    juce::String sig;
    sig << s.bloecke << "|" << s.senkeAufrufe << "|" << s.bloeckeMitKontext << "|"
        << s.bloeckeOhneKontext << "|" << s.zeitspruengeVor << "|" << s.zeitspruengeZurueck << "|"
        << s.spruengeUeberStop << "|" << s.automationPunkte << "|" << s.maxPunkteProBlock << "|"
        << s.bloeckeMitMehrpunkt << "|" << s.mehrpunktOhneZusicherung << "|"
        << (int) s.latenzJeGemeldet << "|" << s.bloeckeOffline << "|" << s.bloeckeDouble << "|"
        << (int) s.spieltGerade << "|" << (int) s.schleifeAktiv << "|" << s.projektzeitNegativ;
    return sig;
}

void HostProbeEditor::timerCallback()
{
    // Nur neu malen, wenn sich wirklich etwas geaendert hat - im Leerlauf
    // exakt nichts tun.
    const auto sig = signatur (prozessor.messstand());
    if (sig == letzteSignatur) return;
    letzteSignatur = sig;
    repaint();
}

void HostProbeEditor::paint (juce::Graphics& g)
{
    const auto s = prozessor.messstand();
    const auto liste = zeilen (s, prozessor.brueckeLiefert());

    g.fillAll (kGrund);
    g.setColour (kText);
    g.setFont (juce::FontOptions (17.0f, juce::Font::bold));
    g.drawText ("Nakama Host-Probe - Termin B", masse::rand, 12,
                getWidth() - 2 * masse::rand, 22, juce::Justification::centredLeft);

    g.setFont (juce::FontOptions (12.0f));
    g.setColour (kMatt);
    g.drawText ("Misst, was FL wirklich liefert. Greift nicht ins Audio ein.",
                masse::rand, 34, getWidth() - 2 * masse::rand, 18, juce::Justification::centredLeft);

    const int spalte = 340;
    int y = masse::kopfHoehe;

    for (const auto& z : liste)
    {
        if (z.istKopf)
        {
            y += masse::kopfAbstand;
            g.setColour (z.farbe);
            g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
            g.drawText (z.name, masse::rand, y, getWidth() - 2 * masse::rand, masse::zeile,
                        juce::Justification::centredLeft);
            y += masse::zeile;
            continue;
        }

        const int x = masse::rand + (z.eingerueckt ? 14 : 0);
        g.setColour (kMatt);
        g.setFont (juce::FontOptions (13.0f));
        g.drawText (z.name, x, y, spalte - x - 8, masse::zeile, juce::Justification::centredLeft);
        g.setColour (z.farbe);
        g.drawText (z.wert, spalte, y, getWidth() - spalte - masse::rand, masse::zeile,
                    juce::Justification::centredLeft);
        y += masse::zeile;
    }

    if (letzteMeldung.isNotEmpty())
    {
        g.setColour (kMatt);
        g.setFont (juce::FontOptions (11.0f));
        g.drawFittedText (letzteMeldung, masse::rand, y + 4,
                          getWidth() - 2 * masse::rand, masse::meldungHoehe - 8,
                          juce::Justification::topLeft, 2);
    }
}

} // namespace eqcop::hostprobe
