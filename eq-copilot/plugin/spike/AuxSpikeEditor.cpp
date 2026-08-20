#include "AuxSpikeEditor.h"

namespace eqcop::spike
{

namespace
{
    const juce::Colour kGrund   { 0xff14171b };
    const juce::Colour kText    { 0xffe6e9ee };
    const juce::Colour kMatt    { 0xff8a929c };
    const juce::Colour kJa      { 0xff5fd48a };
    const juce::Colour kNein    { 0xffd4715f };
    const juce::Colour kOffen   { 0xffd8b24a };
}

AuxSpikeEditor::AuxSpikeEditor (AuxSpikeProcessor& p)
    : juce::AudioProcessorEditor (&p), prozessor (p)
{
    berichtKnopf.onClick = [this]
    {
        const auto datei = prozessor.berichtSchreiben();
        letzteMeldung = datei.existsAsFile()
                          ? "Bericht: " + datei.getFullPathName()
                          : "Bericht KONNTE NICHT geschrieben werden: " + AuxSpikeProcessor::berichtsOrdner().getFullPathName();
        repaint();
    };

    resetKnopf.onClick = [this]
    {
        prozessor.messungZuruecksetzen();
        letzteMeldung = "Messung zurueckgesetzt - Impulse neu abspielen";
        repaint();
    };

    addAndMakeVisible (berichtKnopf);
    addAndMakeVisible (resetKnopf);

    setSize (620, 430);
    startTimerHz (8);
}

void AuxSpikeEditor::resized()
{
    auto unten = getLocalBounds().removeFromBottom (72).reduced (16, 12);
    berichtKnopf.setBounds (unten.removeFromLeft (200).withHeight (30));
    unten.removeFromLeft (12);
    resetKnopf.setBounds (unten.removeFromLeft (200).withHeight (30));
}

juce::String AuxSpikeEditor::signatur (const Schnappschuss& s)
{
    juce::String sig;
    sig << s.samplerate << "|" << s.blockgroesse << "|" << s.samplesVerarbeitet;
    for (const auto& b : s.busse)
        sig << "|" << (int) b.aktiv << "," << b.kanaele << "," << b.impulsSample
            << "," << juce::String (b.spitzeGesamt, 3) << "," << (int) b.protokollOk;
    return sig;
}

void AuxSpikeEditor::timerCallback()
{
    // Nur malen, wenn sich etwas geaendert hat - im Leerlauf exakt nichts.
    const auto sig = signatur (prozessor.schnappschuss());
    if (sig == letzteSignatur)
        return;
    letzteSignatur = sig;
    repaint();
}

void AuxSpikeEditor::paint (juce::Graphics& g)
{
    const auto s = prozessor.schnappschuss();

    g.fillAll (kGrund);
    g.setColour (kText);
    g.setFont (juce::FontOptions (17.0f).withStyle ("Bold"));
    g.drawText ("AUX-SPIKE - Messgeraet, kein Produkt", 16, 12, getWidth() - 32, 22,
                juce::Justification::centredLeft);

    g.setFont (juce::FontOptions (12.0f));
    g.setColour (kMatt);
    g.drawText (s.host + "  |  " + s.wrapper
                    + "  |  " + juce::String (s.samplerate, 0) + " Hz"
                    + "  |  Block " + juce::String (s.blockgroesse),
                16, 36, getWidth() - 32, 18, juce::Justification::centredLeft);

    // Laeuft ueberhaupt Audio? Ohne diese Zeile haelt man eine tote Instanz
    // fuer einen negativen Messbefund.
    const bool laeuft = s.samplesVerarbeitet > 0;
    g.setColour (laeuft ? kJa : kOffen);
    g.drawText (laeuft ? ("Audio laeuft - " + juce::String (s.samplesVerarbeitet) + " Samples verarbeitet")
                       : "Noch kein Audio verarbeitet - Transport starten",
                16, 56, getWidth() - 32, 18, juce::Justification::centredLeft);

    int y = 88;
    for (size_t i = 0; i < s.busse.size(); ++i)
    {
        const auto& b = s.busse[i];
        const bool istMain = (i == 0);

        g.setColour (kText);
        g.setFont (juce::FontOptions (14.0f).withStyle ("Bold"));
        g.drawText ((istMain ? "MAIN  " : "AUX " + juce::String ((int) i) + "  ") + b.name,
                    16, y, 300, 18, juce::Justification::centredLeft);

        g.setFont (juce::FontOptions (12.0f));
        g.setColour (b.aktiv ? kJa : kNein);
        g.drawText (b.aktiv ? (juce::String (b.kanaele) + " Kanaele aktiv")
                            : "INAKTIV - der Host hat diesen Bus nicht zugeschaltet",
                    330, y, getWidth() - 346, 18, juce::Justification::centredLeft);
        y += 20;

        g.setColour (kMatt);
        if (! b.aktiv)
        {
            g.drawText ("Signal: -   Impuls: -   Versatz: -", 32, y, getWidth() - 48, 18,
                        juce::Justification::centredLeft);
            y += 26;
            continue;
        }

        const juce::String signal = b.spitzeGesamt > 0.0f
                                      ? "Signal: ja (Spitze " + juce::String (b.spitzeGesamt, 3) + ")"
                                      : "Signal: nein - hier kommt nichts an";
        const juce::String impuls = b.impulsSample >= 0
                                      ? "Impuls: Sample " + juce::String (b.impulsSample)
                                      : "Impuls: - noch keiner";

        juce::String versatz;
        if (istMain)
            versatz = "Versatz: Bezugspunkt";
        else if (s.hatVersatz (i))
        {
            const auto v = s.versatz (i);
            versatz = "Versatz: " + juce::String (v > 0 ? "+" : "") + juce::String (v) + " Samples";
            if (s.samplerate > 0.0)
                versatz << " (" << juce::String ((double) v * 1000.0 / s.samplerate, 2) << " ms)";
        }
        else
            versatz = "Versatz: - (" + s.versatzGrund (i) + ")";

        g.setColour (b.spitzeGesamt > 0.0f ? kText : kMatt);
        g.drawText (signal, 32, y, 250, 18, juce::Justification::centredLeft);
        g.setColour (b.impulsSample >= 0 ? kText : kMatt);
        g.drawText (impuls, 282, y, 190, 18, juce::Justification::centredLeft);
        y += 18;
        g.setColour ((istMain || s.hatVersatz (i)) ? kText : kMatt);
        g.drawText (versatz, 32, y, getWidth() - 48, 18, juce::Justification::centredLeft);
        y += 18;

        // Das Messprotokoll ist Stille + EIN Impuls. Laeuft stattdessen Musik,
        // muss das hier stehen - sonst haelt man Zufall fuer eine Messung.
        if (! b.protokollOk)
        {
            g.setColour (kNein);
            g.drawText ("Dauersignal auf diesem Bus ("
                            + juce::String (b.lautAnteil * 100.0, 1)
                            + " % laut) - erst alles stumm schalten, dann EINEN Impuls",
                        32, y, getWidth() - 48, 18, juce::Justification::centredLeft);
            y += 18;
        }
        y += 8;
    }

    // Der eine FL-Fakt, ohne den der Termin scheitert.
    g.setColour (kOffen);
    g.setFont (juce::FontOptions (11.5f));
    g.drawText ("In FL: Sidechain im Mixer zuweisen, dann im Wrapper (VFX-Pins) auf die",
                16, getHeight() - 118, getWidth() - 32, 16, juce::Justification::centredLeft);
    g.drawText ("Plugin-Eingaenge 3/4 legen - Aux 2 auf 5/6. Nummern sind projektweit.",
                16, getHeight() - 102, getWidth() - 32, 16, juce::Justification::centredLeft);

    g.setColour (kMatt);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText (letzteMeldung, 16, getHeight() - 80, getWidth() - 32, 16,
                juce::Justification::centredLeft);
}

} // namespace eqcop::spike
