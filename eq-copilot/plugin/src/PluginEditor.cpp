#include "PluginEditor.h"
#include "LeitstandTokens.h"
#include "EqCopilotIds.h"
#include <cmath>
#include <limits>

namespace eqcop
{
using leitstand::farbe;

static juce::Font uiFont (float groesse)
{
    // Systemschrift, bis die Futura-Distributionslizenz geklärt ist
    // (docs/LIZENZ-NOTIZ.md). Kein synthetisches Fett — Gewicht bleibt plain.
    return skin::font (groesse);
}

static juce::String u8 (const char* text) { return juce::String (juce::CharPointer_UTF8 (text)); }

// ── CopilotLookAndFeel ─────────────────────────────────────────────────────
CopilotLookAndFeel::CopilotLookAndFeel()
{
    setColour (juce::PopupMenu::backgroundColourId, farbe (leitstand::copilot_overlay));
    setColour (juce::PopupMenu::textColourId, farbe (leitstand::copilot_text_light));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, farbe (leitstand::copilot_control_active));
    setColour (juce::PopupMenu::highlightedTextColourId, farbe (leitstand::copilot_text_light));
}
juce::Font CopilotLookAndFeel::getComboBoxFont (juce::ComboBox&)          { return uiFont (10.0f * faktor); }
juce::Font CopilotLookAndFeel::getTextButtonFont (juce::TextButton&, int) { return uiFont (9.4f * faktor); }
juce::Font CopilotLookAndFeel::getPopupMenuFont()                         { return uiFont (13.0f * faktor); }

void CopilotLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b,
                                                const juce::Colour& grund,
                                                bool hervorgehoben, bool gedrueckt)
{
    g.saveState();
    if (! b.isEnabled())
        g.setOpacity (0.52f);
    skin::taste (g, b.getLocalBounds().toFloat(), grund, hervorgehoben,
                 gedrueckt, b.getToggleState(), faktor);
    g.restoreState();
}

void CopilotLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b,
                                         bool, bool gedrueckt)
{
    g.saveState();
    if (! b.isEnabled())
        g.setOpacity (0.52f);
    skin::tasteText (g, b, gedrueckt, faktor);
    g.restoreState();
    // setOpacity greift nicht: der Skin setzt eigene Farben (EqCopilotAssetKit
    // taste/tasteText). Ein gesperrter Knopf sah deshalb aktiv aus (T2-Befund
    // SONDE-006). Der Schleier liegt NACH dem Text ueber dem ganzen Knopf.
    if (! b.isEnabled())
    {
        g.setColour (juce::Colours::black.withAlpha (0.42f));
        g.fillRoundedRectangle (b.getLocalBounds().toFloat(), 4.0f * faktor);
    }
}

void CopilotLookAndFeel::drawComboBox (juce::Graphics& g, int breite, int hoehe,
                                       bool gedrueckt, int, int, int, int, juce::ComboBox&)
{
    skin::combo (g, { 0.0f, 0.0f, (float) breite, (float) hoehe }, gedrueckt, faktor);
}

void CopilotLookAndFeel::positionComboBoxText (juce::ComboBox&, juce::Label& label)
{
    label.setBounds (juce::roundToInt (8.0f * faktor), 1,
                     label.getParentWidth() - juce::roundToInt (26.0f * faktor),
                     label.getParentHeight() - 2);
    label.setFont (uiFont (10.2f * faktor));
    label.setJustificationType (juce::Justification::centredLeft);
}

void CopilotLookAndFeel::drawCallOutBoxBackground (juce::CallOutBox&, juce::Graphics& g,
                                                   const juce::Path& pfad, juce::Image&)
{
    g.setColour (farbe (leitstand::copilot_overlay));
    g.fillPath (pfad);
    g.setColour (farbe (leitstand::copilot_control_border));
    g.strokePath (pfad, juce::PathStrokeType (1.2f));
}

EqCopilotEditor::EqCopilotEditor (EqCopilotProcessor& p)
    : juce::AudioProcessorEditor (p), processor (p)
{
    setLookAndFeel (&lnf);
    const auto tinte = farbe (leitstand::copilot_text_light);

    auto initCombo = [&] (juce::ComboBox& c)
    {
        c.setColour (juce::ComboBox::backgroundColourId, farbe (leitstand::copilot_control));
        c.setColour (juce::ComboBox::textColourId, tinte);
        c.setColour (juce::ComboBox::outlineColourId, farbe (leitstand::copilot_control_border));
        c.setColour (juce::ComboBox::arrowColourId, farbe (leitstand::copilot_accent_soft));
        c.addListener (this);
        addAndMakeVisible (c);
    };

    // Werkzeugzeile — Glättung ist reine ANZEIGE (gemessen wird immer 1/24).
    glaettungWahl.addItem ("Kurve glatt", 1);
    glaettungWahl.addItem ("Kurve mittel", 2);
    glaettungWahl.addItem ("Kurve fein", 3);
    glaettungWahl.addItem ("Kurve ganz fein", 4);
    glaettungWahl.setSelectedId (2, juce::dontSendNotification);
    initCombo (glaettungWahl);

    auto initKnopf = [&] (juce::TextButton& k, const juce::String& text, std::function<void()> tun)
    {
        k.setButtonText (text);
        k.setColour (juce::TextButton::buttonColourId, farbe (leitstand::copilot_control));
        k.setColour (juce::TextButton::buttonOnColourId, farbe (leitstand::copilot_control_active));
        k.setColour (juce::TextButton::textColourOffId, tinte);
        k.setColour (juce::TextButton::textColourOnId, tinte);
        k.onClick = std::move (tun);
        addAndMakeVisible (k);
    };
    initKnopf (ansichtKnopf, u8 ("Ansicht: natürlich"), [this]
    {
        // „begradigt": +3 dB/Okt um 1 kHz — gleichmäßiges Rauschen liegt dann
        // waagrecht, Abweichungen springen ins Auge.
        neigungsAnsicht = ! neigungsAnsicht;
        ansichtKnopf.setToggleState (neigungsAnsicht, juce::dontSendNotification);
        ansichtKnopf.setButtonText (neigungsAnsicht ? "Ansicht: begradigt"
                                                    : u8 ("Ansicht: natürlich"));
        yInitialisiert = false;
        uiDirty = true;
    });
    initKnopf (festhaltenKnopf, "Festhalten", [this]
    {
        juce::String meldung;
        const auto m = processor.messSnapshot();
        if (processor.schreibeSnapshotDatei (meldung))
        {
            vergleichRohDb = m.ltasKompositDb;   // Roh gemerkt — Glättung wirkt
            vergleichAktiv = m.ltasGueltig;      // beim Zeichnen auf BEIDE gleich
            statusMeldung = "Festgehalten (Vergleichslinie + Datei): " + meldung;
        }
        else
            statusMeldung = u8 ("Festhalten nicht möglich — ") + meldung;
        statusMeldungBisMs = juce::Time::getMillisecondCounter() + 8000;
        uiDirty = true;
    });
    initKnopf (vergleichWegKnopf, "Vergleich aus", [this]
    {
        vergleichAktiv = false;
        uiDirty = true;
    });
    vergleichWegKnopf.setButtonText ("Vergleich leer");
    vergleichWegKnopf.setEnabled (false);
    initKnopf (resetKnopf, "Neu messen", [this]
    {
        processor.fordereMessResetAn();
        statusMeldung = "Messung neu gestartet";
        statusMeldungBisMs = juce::Time::getMillisecondCounter() + 4000;
        yInitialisiert = false;
        uiDirty = true;
    });
    initKnopf (hinweisKnopf, "keine Hinweise", [this] { zeigeHinweise(); });
    // Messpunkt-Bindung (Aufgabe/Name/Paar) — OPTIONAL, deshalb im Popover
    // statt im Dauer-Kopf: das Plugin misst ohne jedes Setup.
    initKnopf (messpunktKnopf, u8 ("Messpunkt…"), [this] { zeigeMesspunkt(); });
    initKnopf (konfliktKnopf, "Kennung doppelt!", [this] { zeigeKonflikt(); });
    konfliktKnopf.setColour (juce::TextButton::buttonColourId, farbe (leitstand::copilot_fault));
    konfliktKnopf.setColour (juce::TextButton::textColourOffId, farbe (leitstand::copilot_text_light));
    konfliktKnopf.setVisible (false);

    // Hör-Markierung: Not-Aus in der Statuszeile — der ständig sichtbare
    // Aktivzustand ist Teil des Sicherheitsnetzes (Konzept v2 §4, N4).
    initKnopf (markierungAusKnopf, u8 ("Markierung aus"), [this]
    { markierungBeenden (u8 ("Markierung aus.")); });
    markierungAusKnopf.setColour (juce::TextButton::buttonColourId, farbe (leitstand::copilot_warning_bg));
    markierungAusKnopf.setColour (juce::TextButton::textColourOffId, farbe (leitstand::copilot_accent_soft));
    markierungAusKnopf.setVisible (false);

    processor.setzeEditorOffen (true);
    letzteInteraktionMs = juce::Time::getMillisecondCounter();
    addMouseListener (this, true);   // Totmannschalter: jede Bedienung zählt

    // Materialfront: 750:520 hält alle eigenständigen Vektorbauteile und ihre
    // optischen Radien proportional.
    setResizable (true, true);
    setResizeLimits (600, 416, 1950, 1352);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) skin::kEinheitB / (double) skin::kEinheitH);
    // 1200 px ist die neue Qualitäts-/Lesebasis; auf kleineren Displays kann
    // der Host weiterhin bis 600 px verkleinern.
    setSize (1200, 832);
    startTimerHz (30);
}

EqCopilotEditor::~EqCopilotEditor()
{
    // Fensterschluss beendet die Markierung (Konzept v2 N2); editorOffen=false
    // lässt die Audioseite zusätzlich hart auf den Originalpfad schneiden.
    processor.markierungAus();
    processor.setzeEditorOffen (false);
    setLookAndFeel (nullptr);
}

void EqCopilotEditor::timerCallback()
{
    vergleichWegKnopf.setEnabled (vergleichAktiv);
    vergleichWegKnopf.setToggleState (vergleichAktiv, juce::dontSendNotification);
    vergleichWegKnopf.setButtonText (vergleichAktiv ? "Vergleich aus" : "Vergleich leer");
    // Kennungs-Konflikt (M2): sichtbar rot, solange der Broker ihn meldet —
    // er verschwindet erst, wenn eine der Instanzen eine neue Kennung bekommt
    // oder geschlossen wird. Nie still (Plan §8.4).
    konfliktKnopf.setVisible (processor.konfliktGemeldet());

    // Hör-Markierung: Sicherheitsnetz-Ticks (Konzept v2 §4/N2) — Freilauf-
    // Meldung löst den Latch sichtbar, Samplerate-Wechsel und Totmann ebenso.
    if (processor.markierungKillGemeldet() && markModus != MarkierungsModus::aus)
        markierungBeenden (u8 ("Markierung beendet — Offline-Render/Freilauf erkannt."));
    if (markModus != MarkierungsModus::aus)
    {
        if (processor.holeSamplerate() != markEngageSr)
            markierungBeenden (u8 ("Markierung beendet — Samplerate gewechselt."));
        else if (juce::Time::getMillisecondCounter() - letzteInteraktionMs > 10u * 60u * 1000u)
            markierungBeenden (u8 ("Markierung nach 10 Minuten ohne Bedienung beendet."));
    }
    markierungAusKnopf.setVisible (markModus != MarkierungsModus::aus);
    if (markModus != MarkierungsModus::aus)
        uiDirty = true;               // Feld-Tönung folgt der Puls-Phase

    // FPS-Fix (m4): EIN Snapshot-Zug pro Tick; gemalt wird nur, wenn sich
    // wirklich etwas geändert hat (Daten-Revision, Werkzeuge, Pipe-Status,
    // Statusmeldungs-Fenster). Der Graph folgt damit der 20-Hz-Publikation
    // des Workers statt dem alten 4-Hz-Schwertakt — und ruht im Leerlauf.
    auto frisch = processor.messSnapshot();
    const bool datenNeu = ! anzeigeInitial || frisch.revision != anzeigeRevision;
    if (datenNeu)
    {
        anzeigeRevision = frisch.revision;
        anzeige = std::move (frisch);
        anzeigeInitial = true;
        statsAnzeige = processor.statsSnapshot();
    }
    const auto pipeFrisch = processor.pipeSnapshot();
    const bool pipeNeu = (int) pipeFrisch.status != letzterPipeStatus;
    if (pipeNeu || datenNeu)
    {
        letzterPipeStatus = (int) pipeFrisch.status;
        pipeAnzeige = pipeFrisch;
    }
    // read-only-State: die Anzeige darf keinen Zustand vortäuschen, den es
    // nicht gibt — Rolle/Name sind dann nicht bedienbar, die Pipe ist aus.
    const bool nurLesenFrisch = processor.stateNurLesen();
    if (nurLesenFrisch != stateNurLesenAnzeige)
    {
        stateNurLesenAnzeige = nurLesenFrisch;
        stateGrundAnzeige = processor.holeStateGrund();
        messpunktKnopf.setEnabled (! nurLesenFrisch);
        uiDirty = true;
    }

    // M3-Diagnose: 1×/s auf der Anzeige-Kopie — der Hinweis-Knopf trägt das
    // Ergebnis der Kernfunktion, nicht mehr nur die Resonanzliste.
    const auto jetztMs = juce::Time::getMillisecondCounter();
    if (jetztMs - befundeStandMs >= 1000)
    {
        befundeStandMs = jetztMs;
        befunde = diagnose (anzeige, processor.holeRolle());
    }
    const int n = (int) befunde.size();
    hinweisKnopf.setButtonText (diagnoseKnopfText (befunde));
    hinweisKnopf.setColour (juce::TextButton::buttonColourId,
                            n == 0 ? farbe (leitstand::copilot_control)
                                   : farbe (leitstand::copilot_warning_bg));
    hinweisKnopf.setColour (juce::TextButton::textColourOffId,
                            n == 0 ? farbe (leitstand::copilot_text_light)
                                   : farbe (leitstand::copilot_accent_soft));

    const bool meldungAktiv = statusMeldung.isNotEmpty() && jetztMs < statusMeldungBisMs;
    const bool meldungWechsel = meldungAktiv != meldungWarSichtbar;
    meldungWarSichtbar = meldungAktiv;

    if (datenNeu || uiDirty || pipeNeu || meldungWechsel)
    {
        uiDirty = false;
        repaint();
    }
}

// ── Befundkarten (§7.3): der Kern des Plugins — was ist schlecht, was tun ──
namespace
{
struct BefundListe : juce::Component
{
    struct Abschnitt
    {
        juce::TextLayout layout;
        float hoehe = 0.0f;
        float abstandDanach = 0.0f;
        bool trennerDanach = false;
    };
    std::vector<Abschnitt> abschnitte;
    float gesamtHoehe = 8.0f;
    float trennerHoehe = 9.0f;

    // Hör-Markierung (Konzept v2 §6): [Solo]/[Puls] je Karte. Die Knöpfe
    // leben als echte Kinder an den im Textfluss reservierten Positionen.
    struct HoerKnopf
    {
        std::unique_ptr<juce::TextButton> knopf;
        Befund befund;
        MarkierungsModus modus = MarkierungsModus::aus;
    };
    std::vector<HoerKnopf> hoerKnoepfe;
    std::function<void (const Befund&, MarkierungsModus)> schalte;
    std::function<bool (const Befund&, MarkierungsModus)> istAktiv;

    void zeigeZustand()
    {
        for (auto& h : hoerKnoepfe)
            h.knopf->setToggleState (istAktiv && istAktiv (h.befund, h.modus),
                                     juce::dontSendNotification);
    }

    BefundListe (const std::vector<Befund>& befunde, const MessSnapshot& m,
                 float s, float breite,
                 std::function<void (const Befund&, MarkierungsModus)> schalteFn = {},
                 std::function<bool (const Befund&, MarkierungsModus)> istAktivFn = {})
        : schalte (std::move (schalteFn)), istAktiv (std::move (istAktivFn))
    {
        trennerHoehe = 9.0f * s;
        const float innen = breite - 16.0f;

        auto zeile = [&] (const juce::String& prefix, const juce::String& text,
                          float groesse, juce::Colour farbeText,
                          float abstand, bool trenner = false)
        {
            juce::AttributedString a;
            if (prefix.isNotEmpty())
                a.append (prefix + "  ", uiFont (groesse), farbe (leitstand::copilot_text_muted));
            a.append (text, uiFont (groesse), farbeText);
            a.setLineSpacing (2.0f * s);
            Abschnitt ab;
            ab.layout.createLayout (a, innen);
            ab.hoehe = ab.layout.getHeight();
            ab.abstandDanach = abstand;
            ab.trennerDanach = trenner;
            gesamtHoehe += ab.hoehe + abstand + (trenner ? trennerHoehe : 0.0f);
            abschnitte.push_back (std::move (ab));
        };

        if (m.zustand != MessZustand::messbereit)
        {
            zeile ({}, u8 ("Noch keine Aussage — erst ab 15 Sekunden Musik sagt die Messung etwas."),
                   13.0f * s, farbe (leitstand::copilot_text_light), 4.0f * s);
        }
        else if (befunde.empty())
        {
            zeile ({}, u8 ("Keine Auffälligkeit gefunden."), 14.0f * s,
                   farbe (leitstand::copilot_text_light), 2.0f * s);
            zeile ({}, u8 ("Kein Ton sticht heraus, keine Zone staut sich oder fehlt gegenüber "
                           "ihren Nachbarn. Das heißt nicht „perfekt“ — nur: die Kurve gibt "
                           "gerade keinen konkreten Handgriff her. Die Ohren behalten das "
                           "letzte Wort."),
                   12.0f * s, farbe (leitstand::copilot_text_soft), 4.0f * s);
        }
        else
        {
            bool beobachtenKopf = false;
            for (const auto& b : befunde)
            {
                if (! b.topRang && ! beobachtenKopf)
                {
                    beobachtenKopf = true;
                    zeile ({}, u8 ("BEOBACHTEN — noch nicht belastbar genug für die Top-Liste"),
                           10.5f * s, farbe (leitstand::copilot_text_muted), 4.0f * s);
                }
                zeile ({}, b.titel, 14.5f * s, farbe (leitstand::copilot_text_light), 3.0f * s);
                zeile (u8 ("Gemessen:"), b.gemessen, 12.0f * s, farbe (leitstand::copilot_text_soft), 3.0f * s);
                zeile (u8 ("Wirkung:"), b.wirkung, 12.0f * s, farbe (leitstand::copilot_text_soft), 5.0f * s);
                zeile (u8 ("Tu:"), b.tu, 12.5f * s, farbe (leitstand::copilot_text_light), 3.0f * s);
                zeile (u8 ("Warum:"), b.warum, 12.0f * s, farbe (leitstand::copilot_text_soft), 3.0f * s);
                zeile (u8 ("Hören:"), b.hoeren, 12.0f * s, farbe (leitstand::copilot_text_soft), 3.0f * s);
                if (schalte)
                {
                    // Einfärben-Zeile: Solo für alle Klassen, Puls nicht fürs
                    // Mitten-Loch (ein Loch kann nicht pulsieren — Konzept §2).
                    const float knopfH = 24.0f * s, knopfB = 76.0f * s, luecke = 6.0f * s;
                    const float knopfY = gesamtHoehe;
                    auto macheKnopf = [&] (const juce::String& text, MarkierungsModus modus, float x)
                    {
                        HoerKnopf h;
                        h.befund = b;
                        h.modus = modus;
                        h.knopf = std::make_unique<juce::TextButton> (text);
                        h.knopf->setColour (juce::TextButton::buttonColourId, farbe (leitstand::copilot_control));
                        h.knopf->setColour (juce::TextButton::buttonOnColourId, farbe (leitstand::copilot_control_active));
                        h.knopf->setColour (juce::TextButton::textColourOffId, farbe (leitstand::copilot_text_light));
                        h.knopf->setColour (juce::TextButton::textColourOnId, farbe (leitstand::copilot_accent_soft));
                        h.knopf->setBounds ((int) x, (int) knopfY, (int) knopfB, (int) (knopfH - 2.0f));
                        const size_t index = hoerKnoepfe.size();
                        h.knopf->onClick = [this, index]
                        {
                            if (index < hoerKnoepfe.size() && schalte)
                                schalte (hoerKnoepfe[index].befund, hoerKnoepfe[index].modus);
                            zeigeZustand();
                        };
                        addAndMakeVisible (*h.knopf);
                        hoerKnoepfe.push_back (std::move (h));
                    };
                    macheKnopf (u8 ("Solo"), MarkierungsModus::solo, 8.0f);
                    if (b.klasse != BefundKlasse::mittenLoch)
                        macheKnopf (u8 ("Puls"), MarkierungsModus::puls, 8.0f + knopfB + luecke);
                    abschnitte.back().abstandDanach += knopfH + 6.0f * s;
                    gesamtHoehe += knopfH + 6.0f * s;
                }
                zeile (u8 ("Sicherheit:"), b.konfidenzGrund, 10.5f * s,
                       farbe (leitstand::copilot_text_muted), 4.0f * s, true);
            }
        }

        setSize ((int) breite, (int) std::ceil (gesamtHoehe) + 8);
        zeigeZustand();
    }

    void paint (juce::Graphics& g) override
    {
        float y = 8.0f;
        for (const auto& ab : abschnitte)
        {
            ab.layout.draw (g, { 8.0f, y, (float) getWidth() - 16.0f, ab.hoehe });
            y += ab.hoehe + ab.abstandDanach;
            if (ab.trennerDanach)
            {
                g.setColour (farbe (leitstand::copilot_control_border));
                g.fillRect (8.0f, y + trennerHoehe * 0.4f, (float) getWidth() - 16.0f, 1.0f);
                y += trennerHoehe;
            }
        }
    }
};
} // namespace

void EqCopilotEditor::zeigeHinweise()
{
    const float s = ui();
    // Frisch rechnen auf der Anzeige-Kopie — Karten und Graph zeigen damit
    // garantiert denselben Stand (die Kopie ist höchstens einen Tick alt).
    const auto& m = anzeige;
    befunde = diagnose (m, processor.holeRolle());
    befundeStandMs = juce::Time::getMillisecondCounter();

    // SafePointer: die CallOutBox kann den Editor überleben (FL schließt das
    // Fenster mit offenem Popup) — Markierungs-Schaltung darf dann ins Leere.
    const auto safe = juce::Component::SafePointer<EqCopilotEditor> (this);
    auto liste = std::make_unique<BefundListe> (
        befunde, m, s, 480.0f * s,
        [safe] (const Befund& b, MarkierungsModus modus)
        {
            if (auto* editor = safe.getComponent())
                editor->schalteMarkierung (b, modus);
        },
        [safe] (const Befund& b, MarkierungsModus modus)
        {
            auto* editor = safe.getComponent();
            return editor != nullptr && editor->markierungIstAktiv (b, modus);
        });
    const int maxHoehe = (int) (560.0f * s);
    if (liste->getHeight() <= maxHoehe)
    {
        juce::CallOutBox::launchAsynchronously (std::move (liste),
                                                hinweisKnopf.getBounds(), this);
        return;
    }
    auto viewport = std::make_unique<juce::Viewport>();
    viewport->setScrollBarsShown (true, false);
    viewport->setViewedComponent (liste.release(), true);
    viewport->setSize ((int) (480.0f * s) + viewport->getScrollBarThickness(), maxHoehe);
    juce::CallOutBox::launchAsynchronously (std::move (viewport),
                                            hinweisKnopf.getBounds(), this);
}

// M2, Plan §8.4: die sichtbare Entscheidung beim Kennungs-Konflikt. Zwei
// Instanzen melden dieselbe Sensor-ID (typisch: Plugin in FL dupliziert) —
// der User gibt DIESER Instanz eine neue Kennung, die andere behält ihre.
void EqCopilotEditor::zeigeKonflikt()
{
    const float s = ui();

    struct KonfliktPanel : juce::Component
    {
        juce::Label text;
        juce::TextButton neuKnopf;
        std::function<void()> tun;

        KonfliktPanel (const juce::String& t, float skala, std::function<void()> aktion)
            : tun (std::move (aktion))
        {
            text.setText (t, juce::dontSendNotification);
            text.setFont (uiFont (13.0f * skala));
            text.setColour (juce::Label::textColourId, farbe (leitstand::copilot_text_light));
            text.setJustificationType (juce::Justification::topLeft);
            addAndMakeVisible (text);

            neuKnopf.setButtonText (u8 ("Dieser Instanz eine neue Kennung geben"));
            neuKnopf.setColour (juce::TextButton::buttonColourId, farbe (leitstand::copilot_fault));
            neuKnopf.setColour (juce::TextButton::textColourOffId, farbe (leitstand::copilot_text_light));
            neuKnopf.onClick = [this]
            {
                if (tun)
                    tun();
                if (auto* box = findParentComponentOfClass<juce::CallOutBox>())
                    box->dismiss();
            };
            addAndMakeVisible (neuKnopf);
            setSize ((int) (460 * skala), (int) (190 * skala));
        }

        void resized() override
        {
            auto r = getLocalBounds().reduced (8);
            neuKnopf.setBounds (r.removeFromBottom (juce::jmax (28, getHeight() / 6)));
            r.removeFromBottom (8);
            text.setBounds (r);
        }
    };

    auto inhalt = std::make_unique<KonfliktPanel> (
        u8 ("Zwei Messpunkte melden dieselbe Kennung.\n\n"
            "Das passiert, wenn das Plugin in FL dupliziert wurde — beide Kopien "
            "tragen dann dieselbe innere Kennung, und die App kann sie nicht "
            "auseinanderhalten.\n\n"
            "Wenn DIESE Instanz ein eigener Messpunkt sein soll: gib ihr unten "
            "eine neue Kennung. Die andere Instanz behält ihre — dort musst du "
            "nichts tun."),
        s,
        [this]
        {
            statusMeldung = processor.neueSensorId()
                ? u8 ("Neue Kennung vergeben — dieser Messpunkt meldet sich frisch an.")
                : juce::String ("State read-only: " + processor.holeStateGrund() + ". No new identity assigned.");
            statusMeldungBisMs = juce::Time::getMillisecondCounter() + 6000;
        });

    juce::CallOutBox::launchAsynchronously (std::move (inhalt),
                                            konfliktKnopf.getBounds(), this);
}

void EqCopilotEditor::comboBoxChanged (juce::ComboBox* c)
{
    if (c == &glaettungWahl)
    {
        const int id = glaettungWahl.getSelectedId();
        glaettFenster = id == 1 ? 9 : id == 2 ? 5 : id == 3 ? 3 : 1;
        uiDirty = true;
    }
}

// ── Messpunkt-Popover: die OPTIONALE Bindung (Aufgabe/Name/Paar) — bewusst
// nicht im Dauer-Kopf, damit kein Setup-Gefühl entsteht (USER-VORGABE:
// laden → Musik spielen → Befund; alles andere ist Beiwerk). ───────────────
void EqCopilotEditor::zeigeMesspunkt()
{
    if (processor.stateNurLesen())
    {
        statusMeldung = "State read-only: " + processor.holeStateGrund() + ". Role and name cannot be changed.";
        statusMeldungBisMs = juce::Time::getMillisecondCounter() + 6000;
        uiDirty = true;
        return;
    }
    const float s = ui();

    struct MesspunktPanel : juce::Component, juce::ComboBox::Listener, juce::TextEditor::Listener
    {
        EqCopilotProcessor& proz;
        std::function<void()> geaendert;   // Editor neu malen (Kopf zeigt Rolle/Name)
        juce::Label hinweis, rolleEtikett, labelEtikett, paarEtikett;
        juce::ComboBox rolleWahl;
        juce::TextEditor labelFeld, paarFeld;
        float s;

        MesspunktPanel (EqCopilotProcessor& p, float skala, std::function<void()> beiAenderung)
            : proz (p), geaendert (std::move (beiAenderung)), s (skala)
        {
            const auto tinte = farbe (leitstand::copilot_text_light);
            auto initEtikett = [&] (juce::Label& l, const juce::String& text)
            {
                l.setText (text, juce::dontSendNotification);
                l.setFont (uiFont (10.5f * s));
                l.setColour (juce::Label::textColourId, farbe (leitstand::copilot_text_muted));
                addAndMakeVisible (l);
            };
            hinweis.setText (juce::String (juce::CharPointer_UTF8 (
                "Alles hier ist OPTIONAL — gemessen wird immer. Aufgabe/Name helfen "
                "nur, wenn mehrere Messpunkte zusammenarbeiten (App-Übersicht, "
                "Vorher/Nachher-Paar).")), juce::dontSendNotification);
            hinweis.setFont (uiFont (11.0f * s));
            hinweis.setColour (juce::Label::textColourId, farbe (leitstand::copilot_text_soft));
            addAndMakeVisible (hinweis);

            initEtikett (rolleEtikett, "AUFGABE DIESES MESSPUNKTS");
            initEtikett (labelEtikett, juce::String (juce::CharPointer_UTF8 ("NAME (z. B. „Klavier-Bus“)")));
            initEtikett (paarEtikett,  juce::String (juce::CharPointer_UTF8 ("PAAR-NAME (verbindet VOR ↔ NACH)")));

            rolleWahl.addItem ("Misst diese Spur", 1);
            rolleWahl.addItem ("Sammelpunkt (Master)", 2);
            rolleWahl.addItem ("VOR dem EQ (Paar)", 3);
            rolleWahl.addItem ("NACH dem EQ (Paar)", 4);
            const auto r = proz.holeRolle();
            rolleWahl.setSelectedId (r == "hub" ? 2 : r == "pre" ? 3 : r == "post" ? 4 : 1,
                                     juce::dontSendNotification);
            rolleWahl.setColour (juce::ComboBox::backgroundColourId, farbe (leitstand::copilot_control));
            rolleWahl.setColour (juce::ComboBox::textColourId, tinte);
            rolleWahl.setColour (juce::ComboBox::outlineColourId, farbe (leitstand::copilot_control_border));
            rolleWahl.setColour (juce::ComboBox::arrowColourId, farbe (leitstand::copilot_text_muted));
            rolleWahl.addListener (this);
            addAndMakeVisible (rolleWahl);

            auto initFeld = [&] (juce::TextEditor& f, const juce::String& inhalt)
            {
                f.setText (inhalt, juce::dontSendNotification);
                f.applyFontToAllText (uiFont (14.0f * s));
                f.setColour (juce::TextEditor::backgroundColourId, farbe (leitstand::copilot_field));
                f.setColour (juce::TextEditor::textColourId, tinte);
                f.setColour (juce::CaretComponent::caretColourId, tinte);
                f.setColour (juce::TextEditor::outlineColourId, farbe (leitstand::copilot_control_border));
                f.setColour (juce::TextEditor::focusedOutlineColourId, farbe (leitstand::copilot_accent_soft));
                f.addListener (this);
                addAndMakeVisible (f);
            };
            initFeld (labelFeld, proz.holeLabel());
            initFeld (paarFeld, proz.holePaarId());
            paarSichtbarkeit();
            setSize ((int) (400 * s), (int) (238 * s));
        }
        ~MesspunktPanel() override { uebernehmen(); }

        void paarSichtbarkeit()
        {
            const auto id = rolleWahl.getSelectedId();
            const bool paar = (id == 3 || id == 4);
            paarFeld.setEnabled (paar);
            paarFeld.setAlpha (paar ? 1.0f : 0.5f);
        }
        // Übernimmt NUR bei echter Änderung — setzeBindung löst einen
        // Reconnect aus, und der wäre bei jedem Öffnen/Schließen oder
        // Fokuswechsel ohne Änderung reines Verbindungs-Geflacker.
        void uebernehmen()
        {
            const auto id = rolleWahl.getSelectedId();
            const juce::String rolle = id == 2 ? "hub" : id == 3 ? "pre" : id == 4 ? "post" : "sensor";
            const bool paar = (id == 3 || id == 4);
            const auto label = labelFeld.getText().substring (0, 120);
            const auto paarId = paar ? paarFeld.getText().substring (0, 60) : juce::String();
            if (rolle == proz.holeRolle() && label == proz.holeLabel() && paarId == proz.holePaarId())
                return;
            if (proz.setzeBindung (rolle, label, paarId) && geaendert)
                geaendert();   // Kopfzeile (Rolle/Name) sofort nachziehen
        }
        void comboBoxChanged (juce::ComboBox*) override { paarSichtbarkeit(); uebernehmen(); }
        void textEditorFocusLost (juce::TextEditor&) override { uebernehmen(); }
        void textEditorReturnKeyPressed (juce::TextEditor& f) override { f.giveAwayKeyboardFocus(); }

        void resized() override
        {
            auto r = getLocalBounds().reduced ((int) (10 * s));
            hinweis.setBounds (r.removeFromTop ((int) (44 * s)));
            r.removeFromTop ((int) (6 * s));
            const int etikettH = (int) (15 * s), feldH = (int) (28 * s), luecke = (int) (8 * s);
            rolleEtikett.setBounds (r.removeFromTop (etikettH));
            rolleWahl.setBounds (r.removeFromTop (feldH));
            r.removeFromTop (luecke);
            labelEtikett.setBounds (r.removeFromTop (etikettH));
            labelFeld.setBounds (r.removeFromTop (feldH));
            r.removeFromTop (luecke);
            paarEtikett.setBounds (r.removeFromTop (etikettH));
            paarFeld.setBounds (r.removeFromTop (feldH));
        }
    };

    // SafePointer statt rohem this: der Panel-Destruktor ruft uebernehmen()
    // auch dann, wenn FL das Editorfenster MIT offenem Popover schließt —
    // der Editor ist dann schon im Abbau (Component nullt SafePointer sofort).
    juce::CallOutBox::launchAsynchronously (
        std::make_unique<MesspunktPanel> (
            processor, s,
            [safe = juce::Component::SafePointer<EqCopilotEditor> (this)]
            {
                if (auto* editor = safe.getComponent())
                    editor->uiDirty = true;
            }),
        messpunktKnopf.getBounds(), this);
}

// ── Hör-Markierung: Engage/Aus (Konzept v2 §2/§4) ──────────────────────────
// Parameter werden beim Engage EINGEFROREN (kein Nachwandern, während der
// User hört); Identität eines Latches = Klasse + Schwerpunkt.
bool EqCopilotEditor::markierungIstAktiv (const Befund& b, MarkierungsModus modus) const
{
    return markModus == modus && markKlasse == b.klasse
        && std::abs (markSchwerpunkt - b.fSchwerpunkt) < 0.5;
}

void EqCopilotEditor::schalteMarkierung (const Befund& b, MarkierungsModus modus)
{
    letzteInteraktionMs = juce::Time::getMillisecondCounter();
    if (markierungIstAktiv (b, modus))
    {
        markierungBeenden (u8 ("Markierung aus."));
        return;
    }

    MarkierungsWunsch w;
    w.modus = modus;
    w.istResonanz = b.klasse == BefundKlasse::resonanz;
    w.fVon = b.fVon;
    w.fBis = b.fBis;
    w.fSchwerpunkt = b.fSchwerpunkt;
    w.breiteOktaven = w.istResonanz && b.fVon > 0.0
                          ? std::log2 (std::max (b.fBis, b.fVon + 0.1) / b.fVon)
                          : 0.0;
    w.fs = processor.holeSamplerate();

    // Puls-Tiefe headroom-bewusst aus der eigenen True-Peak-Messung (B3).
    juce::String deckelNotiz;
    if (modus == MarkierungsModus::puls)
    {
        w.pulsTiefeDb = 4.5;
        const auto m = processor.messSnapshot();
        if (std::isfinite (m.truePeakDb) && m.truePeakDb > -200.0)
        {
            const double frei = -0.3 - m.truePeakDb;
            w.pulsTiefeDb = juce::jlimit (2.0, 4.5, frei);
            if (frei < 2.0)
                deckelNotiz = u8 (" Puls gedeckelt (+2,0 dB) — dein Master ist heiß, am Wandler kann es kurz übersteuern.");
            else if (frei < 4.5)
                deckelNotiz = u8 (" Puls angepasst (+") + juce::String (w.pulsTiefeDb, 1)
                              + u8 (" dB) — wenig Headroom.");
        }
    }

    MarkierungsAuftrag auftrag;
    if (! baueMarkierungsAuftrag (auftrag, w))
    {
        statusMeldung = u8 ("Markierung nicht möglich — Befundwerte ungültig.");
        statusMeldungBisMs = letzteInteraktionMs + 6000;
        uiDirty = true;
        return;
    }
    processor.markierungEinreichen (auftrag);
    markModus = modus;
    markKlasse = b.klasse;
    markVon = w.fVon;
    markBis = w.fBis;
    markSchwerpunkt = w.fSchwerpunkt;
    markEngageSr = w.fs;
    statusMeldung = (modus == MarkierungsModus::puls
                         ? u8 ("Einfärben: PULS — der Bereich schwillt im Takt an.")
                         : u8 ("Einfärben: SOLO — nur dieser Bereich spielt."))
                    + deckelNotiz
                    + (processor.markierungEchtzeitOk() ? juce::String()
                                                        : u8 (" Startet mit der Wiedergabe."));
    statusMeldungBisMs = letzteInteraktionMs + 8000;
    uiDirty = true;
}

void EqCopilotEditor::markierungBeenden (const juce::String& grund)
{
    if (markModus == MarkierungsModus::aus)
        return;
    processor.markierungAus();
    markModus = MarkierungsModus::aus;
    statusMeldung = grund;
    statusMeldungBisMs = juce::Time::getMillisecondCounter() + 5000;
    uiDirty = true;
}

void EqCopilotEditor::mouseMove (const juce::MouseEvent&)
{
    letzteInteraktionMs = juce::Time::getMillisecondCounter();
}

void EqCopilotEditor::mouseDown (const juce::MouseEvent&)
{
    letzteInteraktionMs = juce::Time::getMillisecondCounter();
}

juce::Rectangle<int> EqCopilotEditor::graphFlaeche() const
{
    return { E (skin::kGraphX), E (skin::kGraphY),
             E (skin::kGraphB), E (skin::kGraphH) };
}

void EqCopilotEditor::resized()
{
    lnf.faktor = ui();   // ComboBox-/Knopf-/Popup-Schriften wachsen mit
    front.stelleSicher (getWidth(), getHeight());

    // Alle ECHTEN Werkzeuge sitzen als frei verschiebbare Komponenten im
    // gemeinsamen Bett. Keine dekorativen Regler, keine Fake-Funktionen.
    const int luecke = E (5.0f);
    auto zeile = juce::Rectangle<int> (E (skin::kWerkzeugX + 8.0f),
                                       E (skin::kWerkzeugY + 8.0f),
                                       E (skin::kWerkzeugB - 16.0f),
                                       E (skin::kWerkzeugH - 16.0f));
    glaettungWahl.setBounds (zeile.removeFromLeft (E (88.0f)));
    zeile.removeFromLeft (luecke);
    ansichtKnopf.setBounds (zeile.removeFromLeft (E (94.0f)));
    zeile.removeFromLeft (luecke);
    festhaltenKnopf.setBounds (zeile.removeFromLeft (E (72.0f)));
    zeile.removeFromLeft (luecke);
    vergleichWegKnopf.setBounds (zeile.removeFromLeft (E (76.0f)));
    resetKnopf.setBounds (zeile.removeFromRight (E (68.0f)));
    zeile.removeFromRight (luecke);
    messpunktKnopf.setBounds (zeile.removeFromRight (E (82.0f)));
    zeile.removeFromRight (luecke);
    hinweisKnopf.setBounds (zeile.removeFromRight (E (96.0f)));

    // Konflikt überdeckt nur die rechten Statuszellen, niemals Werkzeuge.
    konfliktKnopf.setBounds (juce::Rectangle<int> (E (skin::kStatusX + skin::kStatusB - 158.0f),
                                                   E (skin::kStatusY + 8.0f),
                                                   E (150.0f), E (skin::kStatusH - 16.0f)));
    // Markierungs-Not-Aus links im Statusbereich — kollidiert nie mit dem
    // Konflikt-Knopf rechts.
    markierungAusKnopf.setBounds (juce::Rectangle<int> (E (skin::kStatusX + 8.0f),
                                                        E (skin::kStatusY + 8.0f),
                                                        E (150.0f), E (skin::kStatusH - 16.0f)));
}

// Anzeige-Glättung — arbeitet auf der Kopie. Die Neigungsansicht rechnet der
// Aufrufer (braucht die Zentren).
std::array<double, kLtasBaender> EqCopilotEditor::anzeigeKurve (const std::array<double, kLtasBaender>& roh) const
{
    std::array<double, kLtasBaender> k;
    const int halb = glaettFenster / 2;
    for (int b = 0; b < kLtasBaender; ++b)
    {
        double summe = 0.0; int nWerte = 0;
        for (int j = -halb; j <= halb; ++j)
        {
            const int idx = juce::jlimit (0, kLtasBaender - 1, b + j);
            if (! std::isnan (roh[(size_t) idx]))
            {
                summe += roh[(size_t) idx];
                ++nWerte;
            }
        }
        k[(size_t) b] = nWerte > 0 ? summe / nWerte : std::numeric_limits<double>::quiet_NaN();
    }
    return k;
}

void EqCopilotEditor::zeichneKurve (juce::Graphics& g, const MessSnapshot& m,
                                    const std::array<double, kLtasBaender>& kurve,
                                    juce::Colour farbeLinie, float staerke,
                                    float xLinks, float xRechts, float yOben, float yUnten) const
{
    const double dbOben = yObenDb, dbUnten = yObenDb - 80.0;
    juce::Path pfad;
    bool offen = false;
    for (int b = 0; b < kLtasBaender; ++b)
    {
        const double v = kurve[(size_t) b];
        if (std::isnan (v))
        {
            offen = false;
            continue;
        }
        const double f = m.ltasZentrenHz[(size_t) b];
        const float x = xLinks + (float) (std::log10 (f / 20.0) / std::log10 (1000.0)) * (xRechts - xLinks);
        const float y = yOben + (float) ((dbOben - v) / (dbOben - dbUnten)) * (yUnten - yOben);
        const float yGeklemmt = juce::jlimit (yOben, yUnten, y);
        if (! offen)
        {
            pfad.startNewSubPath (x, yGeklemmt);
            offen = true;
        }
        else
            pfad.lineTo (x, yGeklemmt);   // linear = monoton, erfindet keine Spitzen
    }
    g.setColour (farbeLinie);
    g.strokePath (pfad, juce::PathStrokeType (staerke, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
}

void EqCopilotEditor::paint (juce::Graphics& g)
{
    const float s = ui();          // Schriftskala (Lesegröße wie bisher)
    const float gs = geraet();     // Geräteeinheiten → Pixel

    // Statische Gerätefront aus dem Cache — paint zeichnet nur noch Dynamik.
    front.stelleSicher (getWidth(), getHeight());
    g.drawImageAt (front.bild, 0, 0);

    // FPS-Fix (m4): paint liest die Timer-Kopie — kein zweiter Snapshot-Zug
    // pro Frame, und Diagnose + Anzeige arbeiten garantiert auf demselben Stand.
    const auto& m = anzeige;

    // ── Dynamisches Markenmodul im Kopf: echte Rolle + echter Name ──
    const auto rolleCode = processor.holeRolle();
    juce::String rolle = rolleCode == "hub"  ? "SAMMELPUNKT"
                         : rolleCode == "pre"  ? "VOR EQ"
                         : rolleCode == "post" ? "NACH EQ"
                                               : "SPUR";
    auto messpunktName = processor.holeLabel().trim();
    if (messpunktName.isEmpty())
        messpunktName = "UNBENANNTER MESSPUNKT";
    if (stateNurLesenAnzeige)
    {
        // Keine Rolle, kein Name: der State ist fuer diese Version nicht lesbar.
        rolle = "READ-ONLY";
        messpunktName = "STATE NOT READABLE";
    }
    skin::kopfAnzeigeText (g, skin::kopfAnzeige (gs), rolle, messpunktName, gs);

    // Zwei reine ANZEIGEN statt vorgetäuschter Regler: links momentane
    // Lautheit, rechts True Peak. Beide Werte stammen aus demselben Snapshot
    // wie Graph und Statuszellen.
    const bool shortGueltig = m.lufsShortGueltig && std::isfinite (m.lufsShort);
    const bool peakGueltig = m.truePeakDb > -200.0 && std::isfinite (m.truePeakDb);
    const auto kurzWert = shortGueltig ? juce::String (m.lufsShort, 1) : u8 ("—");
    const auto peakWert = peakGueltig ? juce::String (m.truePeakDb, 1) : u8 ("—");
    skin::meter (g, { skin::kMeterLinksX * gs, skin::kMeterY * gs,
                      skin::kMeterB * gs, skin::kMeterH * gs },
                 (float) ((m.lufsShort + 60.0) / 60.0), kurzWert, "LUFS S", shortGueltig, gs);
    skin::meter (g, { skin::kMeterRechtsX * gs, skin::kMeterY * gs,
                      skin::kMeterB * gs, skin::kMeterH * gs },
                 (float) ((m.truePeakDb + 30.0) / 30.0), peakWert, "dBTP", peakGueltig, gs);

    const auto graph = graphFlaeche().toFloat();
    const auto innen = graph;      // das Display ist bereits der Innenraum
    const float achseLinks = innen.getX() + 38.0f * gs;
    const float zonenHoehe = 9.0f * gs;
    const float achseUnten = innen.getBottom() - (20.0f + 9.0f) * gs;   // Labels + Zonenleiste
    const float achseOben  = innen.getY() + 24.0f * gs;
    const float achseRechts = innen.getRight() - 10.0f * gs;

    // Display-Mikroleiste: Messraster, Ansicht und optionale Vergleichslegende.
    g.setColour (farbe (leitstand::copilot_text_muted));
    g.setFont (uiFont (9.2f * s));
    g.drawText ("LTAS  /  1/24 OKT", (int) (innen.getX() + 7.0f * gs), (int) (innen.getY() + 4.0f * gs),
                (int) (150.0f * gs), (int) (12.0f * gs), juce::Justification::centredLeft);
    // M3a-Konvergenz (smart:EQ-/Pro-Q-Erbe): „KURVE STEHT" erscheint, sobald
    // sich das Komposit über ~10 s Aktivzeit um < 0,35 dB im Mittel bewegt —
    // ab da ist die Kurve belastbar lesbar. Nur ein positives Signal, kein
    // Dauer-Gegenstück (Ruhe zuerst, §7.1 R3).
    juce::String ansichtText = neigungsAnsicht ? "BEGRADIGT" : u8 ("NATÜRLICH");
    if (m.zustand == MessZustand::messbereit && m.konvergenzGueltig
        && m.konvergenzMeanDb < 0.35)
        ansichtText = "KURVE STEHT  /  " + ansichtText;
    g.drawText (ansichtText + "  /  " + glaettungWahl.getText().toUpperCase(),
                (int) (innen.getRight() - 300.0f * gs), (int) (innen.getY() + 4.0f * gs),
                (int) (292.0f * gs), (int) (12.0f * gs), juce::Justification::centredRight);
    if (vergleichAktiv)
    {
        g.setColour (farbe (leitstand::copilot_curve_compare));
        g.fillEllipse (innen.getRight() - 309.0f * gs, innen.getY() + 8.0f * gs,
                       3.0f * gs, 3.0f * gs);
    }

    const auto fx = [&] (double f)
    {
        return achseLinks + (float) (std::log10 (f / 20.0) / std::log10 (1000.0)) * (achseRechts - achseLinks);
    };

    auto transformiere = [&] (const std::array<double, kLtasBaender>& roh)
    {
        auto k = anzeigeKurve (roh);
        if (neigungsAnsicht)
            for (int b = 0; b < kLtasBaender; ++b)
                if (! std::isnan (k[(size_t) b]))
                    k[(size_t) b] += 3.0 * std::log2 (m.ltasZentrenHz[(size_t) b] / 1000.0);
        return k;
    };
    const auto haupt = transformiere (m.ltasKompositDb);
    const auto live  = transformiere (m.ltasLiveDb);
    const auto vergleich = vergleichAktiv ? transformiere (vergleichRohDb)
                                          : std::array<double, kLtasBaender> {};

    // Stabiler Y-Bereich: an der Hauptkurve ankern, danach nur noch wachsen.
    if (m.ltasGueltig)
    {
        double maxDb = -1e9;
        for (const double v : haupt)
            if (! std::isnan (v))
                maxDb = std::max (maxDb, v);
        if (maxDb > -1e8)
        {
            const double ziel = std::ceil ((maxDb + 4.0) / 10.0) * 10.0;
            yObenDb = yInitialisiert ? std::max (yObenDb, ziel) : ziel;
            yInitialisiert = true;
        }
    }

    // ── Achsen im Display: helles Gitter auf Glas ──
    g.setFont (uiFont (12.0f * s));
    const double marken[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    for (const double f : marken)
    {
        const float x = fx (f);
        g.setColour (farbe (leitstand::copilot_grid).withAlpha (0.74f));
        g.drawVerticalLine ((int) x, achseOben, achseUnten);
        g.setColour (farbe (leitstand::copilot_axis_text));
        const auto text = f >= 1000.0 ? juce::String (f / 1000.0, 0) + "k" : juce::String ((int) f);
        g.drawText (text, (int) (x - 24 * s), (int) (achseUnten + zonenHoehe + 3 * gs),
                    (int) (48 * s), (int) (15 * s), juce::Justification::centred);
    }
    for (int schritt = 0; schritt <= 8; ++schritt)
    {
        const double dbWert = yObenDb - 10.0 * schritt;
        const float y = achseOben + (float) schritt / 8.0f * (achseUnten - achseOben);
        g.setColour (farbe (leitstand::copilot_grid).withAlpha (0.64f));
        g.drawHorizontalLine ((int) y, achseLinks, achseRechts);
        g.setColour (farbe (leitstand::copilot_axis_text));
        g.drawText (juce::String ((int) dbWert), (int) innen.getX(), (int) (y - 8 * s),
                    (int) (36 * gs), (int) (16 * s), juce::Justification::centredRight);
    }

    // ── Abdeckungs-Zonenleiste: „wo kann die Messung nichts sagen" ──
    // Auf dem dunklen Display heben helle Schleier die blinden Zonen an
    // (bernstein-getönt = eingeschränkt, neutral = nicht messbar).
    for (const auto& a : m.abdeckung)
    {
        if (a.klasse == AbdeckungsKlasse::belastbar)
            continue;
        const double lo = a.zentrumHz / std::pow (2.0, 1.0 / 6.0);
        const double hi = a.zentrumHz * std::pow (2.0, 1.0 / 6.0);
        const float x1 = juce::jlimit (achseLinks, achseRechts, fx (lo));
        const float x2 = juce::jlimit (achseLinks, achseRechts, fx (hi));
        g.setColour (a.klasse == AbdeckungsKlasse::eingeschraenkt
                         ? farbe (leitstand::copilot_marker).withAlpha (0.22f)
                         : juce::Colours::white.withAlpha (0.055f));
        g.fillRect (x1, achseUnten + 2.0f * gs, x2 - x1, zonenHoehe - 2.0f * gs);
    }
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawHorizontalLine ((int) (achseUnten + 1.0f * gs), achseLinks, achseRechts);

    // ── Hör-Markierung: Feld-Tönung des markierten Bereichs (Konzept v2 §6) ──
    // Tonwert trägt die Bedeutung; der Puls pulsiert phasengleich mit dem
    // Hörbaren (dieselbe Hüllkurve). Wartet der Echtzeit-Beweis, steht die
    // Tönung still und die Mikroleiste sagt es ehrlich.
    if (markModus != MarkierungsModus::aus)
    {
        const float x1 = juce::jlimit (achseLinks, achseRechts, fx (markVon));
        const float x2 = juce::jlimit (achseLinks, achseRechts, fx (markBis));
        const bool hoerbar = processor.markierungHoerbar();
        const float phase = processor.markierungPhase();
        const float alpha = markModus == MarkierungsModus::puls
                                ? (hoerbar ? 0.05f + 0.20f * phase : 0.05f)
                                : (hoerbar ? 0.15f : 0.05f);
        g.setColour (farbe (leitstand::copilot_accent_soft).withAlpha (alpha));
        g.fillRect (x1, achseOben, std::max (1.0f, x2 - x1), achseUnten - achseOben);
        g.setColour (farbe (leitstand::copilot_text_muted));
        g.setFont (uiFont (9.2f * s));
        const juce::String modusText = markModus == MarkierungsModus::puls ? "PULS" : "SOLO";
        const juce::String bereich = juce::String ((int) std::lround (markVon)) + "-"
                                   + juce::String ((int) std::lround (markBis)) + " Hz";
        g.drawText (u8 ("EINFÄRBEN: ") + modusText + "  " + bereich
                        + (hoerbar ? juce::String() : u8 ("  —  wartet auf Wiedergabe")),
                    (int) (innen.getX() + 7.0f * gs), (int) (innen.getY() + 16.0f * gs),
                    (int) (330.0f * gs), (int) (12.0f * gs), juce::Justification::centredLeft);
    }

    // ── Kurven: Live leise hinter der Hauptlinie; Vergleich als dritte Ebene ──
    if (m.ltasGueltig || m.zustand != MessZustand::keineDaten)
    {
        zeichneKurve (g, m, live, farbe (leitstand::copilot_curve_live).withAlpha (0.65f), 1.2f * gs,
                      achseLinks, achseRechts, achseOben, achseUnten);
        if (vergleichAktiv)
            zeichneKurve (g, m, vergleich, farbe (leitstand::copilot_curve_compare), 1.4f * gs,
                          achseLinks, achseRechts, achseOben, achseUnten);
        if (m.ltasGueltig)
        {
            zeichneKurve (g, m, haupt, juce::Colours::black.withAlpha (0.72f), 4.4f * gs,
                          achseLinks, achseRechts, achseOben, achseUnten);
            zeichneKurve (g, m, haupt, farbe (leitstand::copilot_curve_main), 2.0f * gs,
                          achseLinks, achseRechts, achseOben, achseUnten);
        }
    }

    // ── Resonanz-Marker: nur Dreiecke — die Liste wohnt im Hinweis-Knopf ──
    {
        const double dbOben = yObenDb, dbUnten = yObenDb - 80.0;
        for (const auto& kandidat : m.resonanzen)
        {
            int naechst = 0;
            double bester = 1e18;
            for (int b = 0; b < kLtasBaender; ++b)
            {
                const double abstand = std::abs (m.ltasZentrenHz[(size_t) b] - kandidat.freqHz);
                if (abstand < bester) { bester = abstand; naechst = b; }
            }
            const double anzeigeDb = haupt[(size_t) naechst];
            if (std::isnan (anzeigeDb))
                continue;
            const float x = fx (kandidat.freqHz);
            const float y = achseOben + (float) ((dbOben - anzeigeDb) / (dbOben - dbUnten))
                                          * (achseUnten - achseOben);
            skin::resonanzMarker (g, { x, y - 12.0f * gs },
                                  kandidat.klasse == ResonanzKlasse::dauerhaft, gs);
        }
    }

    // ── Zustandshinweis im Display, solange Befunde gesperrt sind (§5.3) ──
    if (m.zustand != MessZustand::messbereit)
    {
        g.setColour (farbe (leitstand::copilot_text_soft).withAlpha (0.9f));
        g.setFont (uiFont (14.0f * s));
        const auto text = m.zustand == MessZustand::keineDaten
            ? u8 ("noch keine Daten — Wiedergabe in FL starten")
            : u8 ("hört zu … noch ")
                  + juce::String ((int) std::ceil (juce::jmax (0.0, 15.0 - m.aktivSekunden)))
                  + u8 (" s Musik, dann sagt die Messung etwas");
        g.drawText (text, graph.toNearestInt(), juce::Justification::centred);
    }

    // ── Status als getrennte, schnell lesbare Instrumentenzellen ──
    const auto& pipe = pipeAnzeige;
    const auto& stats = statsAnzeige;

    juce::String wort; juce::Colour punktFarbe;
    switch (pipe.status)
    {
        case PipeClient::Status::verbunden:
            wort = "APP VERBUNDEN";
            punktFarbe = farbe (leitstand::copilot_led_green); break;
        case PipeClient::Status::verbindet:
            wort = "VERBINDET " + u8 ("…");
            punktFarbe = farbe (leitstand::copilot_led_amber); break;
        default:
            wort = "APP AUS / LOKAL";
            punktFarbe = farbe (leitstand::copilot_led_red); break;
    }
    if (stateNurLesenAnzeige)
    {
        wort = "STATE READ-ONLY";
        punktFarbe = farbe (leitstand::copilot_led_red);
    }

    auto fmt = [] (double v, bool gueltig, int stellen = 1)
    {
        return gueltig ? juce::String (v, stellen) : u8 ("—");
    };

    const juce::String messWort = m.zustand == MessZustand::messbereit
                                      ? "BEREIT"
                                      : m.zustand == MessZustand::sammelt
                                            ? u8 ("HÖRT ZU") : "WARTET";
    const auto messWert = messWort + " / " + juce::String (m.aktivSekunden, 0) + " s";

    auto zelle = [gs] (float x, float b)
    {
        return juce::Rectangle<float> (x * gs, skin::kStatusY * gs, b * gs, skin::kStatusH * gs);
    };

    // Die LED sitzt als eigenes Asset vor der ersten Textzelle.
    skin::led (g, { 39.0f * gs, (skin::kStatusY + skin::kStatusH * 0.5f) * gs }, punktFarbe, gs);
    skin::statusZelle (g, zelle (43.0f, 135.0f), "VERBINDUNG", wort, gs, {}, false);
    skin::statusZelle (g, zelle (178.0f, 122.0f), "MESSUNG", messWert, gs);
    skin::statusZelle (g, zelle (300.0f, 82.0f), "LUFS I",
                       fmt (m.lufsIntegriert, m.lufsGueltig), gs);
    skin::statusZelle (g, zelle (382.0f, 82.0f), "LUFS S",
                       fmt (m.lufsShort, m.lufsShortGueltig), gs);
    skin::statusZelle (g, zelle (464.0f, 84.0f), "SPITZE",
                       fmt (m.truePeakDb, peakGueltig) + " dB", gs);
    skin::statusZelle (g, zelle (548.0f, 84.0f), "DYNAMIK",
                       fmt (m.crestDb, m.crestGueltig) + " dB", gs);

    juce::String signalWert = "OK";
    auto signalFarbe = farbe (leitstand::copilot_led_green);
    if (stats.nanSeen)
    {
        signalWert = "NaN / INF";
        signalFarbe = farbe (leitstand::copilot_led_red);
    }
    else if (stats.framesDropped > 0)
    {
        signalWert = "DROPS " + juce::String ((juce::int64) stats.framesDropped);
        signalFarbe = farbe (leitstand::copilot_led_amber);
    }
    skin::statusZelle (g, zelle (632.0f, 86.0f), "DATEN", signalWert, gs, signalFarbe);

    // Meldung (Festhalten-Pfad, Reset) nur im Zeitfenster — danach gehört die
    // Fläche wieder den Messzellen. Die Meldung überlagert nur die Mitte.
    // Ausnahme: ein read-only-State bleibt DAUERHAFT sichtbar (Anzeige-Pflicht).
    juce::String meldungText = statusMeldung;
    bool meldungZeigen = ! statusMeldung.isEmpty() && juce::Time::getMillisecondCounter() < statusMeldungBisMs;
    if (stateNurLesenAnzeige && ! meldungZeigen)
    {
        // Der Grund kommt aus der State-Bibliothek (englisch): "NakamaState
        // schema 3 is unknown to this version (it reads schema 2)" oder eine
        // verletzte Kind-Matrix - nie die Behauptung "neuere Version", wenn es
        // keine ist.
        meldungText = "State read-only: " + stateGrundAnzeige
                      + ". Audio passes through, nothing is written over it.";
        meldungZeigen = true;
    }
    if (meldungZeigen)
    {
        auto box = juce::Rectangle<float> (178.0f * gs, (skin::kStatusY + 8.0f) * gs,
                                           (konfliktKnopf.isVisible() ? 382.0f : 532.0f) * gs,
                                           (skin::kStatusH - 16.0f) * gs);
        g.setColour (farbe (leitstand::copilot_field));
        g.fillRoundedRectangle (box, 4.5f * gs);
        g.setColour (farbe (leitstand::copilot_accent).withAlpha (0.72f));
        g.drawRoundedRectangle (box.reduced (0.6f * gs), 4.0f * gs, 0.8f * gs);
        g.setColour (farbe (leitstand::copilot_text_light));
        g.setFont (uiFont (10.0f * s));
        g.drawFittedText (meldungText, box.reduced (8.0f * gs).toNearestInt(),
                          juce::Justification::centred, 2, 0.75f);
    }
}

} // namespace eqcop
