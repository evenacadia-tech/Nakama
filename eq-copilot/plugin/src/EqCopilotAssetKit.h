// EQ-Copilot Material Kit
//
// Eigenständiger, auf den Sammler zugeschnittener JUCE-Vektorbaukasten.
// Die visuelle Referenz war nur die Qualitätslatte (Materialtiefe, klare
// Hierarchie, warme Metallfläche, dunkles Glas, Ember-Akzent) — Geometrie,
// Marke und Bedienelemente sind eigens für den EQ-Copiloten entworfen.
//
// Alle Bauteile sind unabhängig renderbar und nehmen ihre Zielgeometrie als
// Rectangle entgegen. Dadurch bleiben Display, Messleisten, Tasten, LEDs,
// Statuszellen und Markenmodul einzeln verschiebbar und auf jeder Plugin-
// Größe scharf. Es gibt bewusst keine dekorativen Potis: Der Copilot besitzt
// keine Audio-Parameter und darf keine vortäuschen.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LeitstandTokens.h"

namespace eqcop::skin
{
using leitstand::farbe;

inline constexpr float kEinheitB = 750.0f;
inline constexpr float kEinheitH = 520.0f;

// Gerätekörper
inline constexpr float kAussenX = 8.0f;
inline constexpr float kAussenY = 8.0f;
inline constexpr float kAussenB = 734.0f;
inline constexpr float kAussenH = 504.0f;
inline constexpr float kKopfY   = 12.0f;
inline constexpr float kKopfH   = 58.0f;

// Funktionsflächen. Diese Konstanten sind der Layoutvertrag zwischen dem
// statischen Rahmen und PluginEditor::resized()/paint().
inline constexpr float kDisplayX = 64.0f;
inline constexpr float kDisplayY = 104.0f;
inline constexpr float kDisplayB = 622.0f;
inline constexpr float kDisplayH = 270.0f;
inline constexpr float kGraphX   = 73.0f;
inline constexpr float kGraphY   = 113.0f;
inline constexpr float kGraphB   = 604.0f;
inline constexpr float kGraphH   = 252.0f;

inline constexpr float kMeterLinksX  = 22.0f;
inline constexpr float kMeterRechtsX = 702.0f;
inline constexpr float kMeterY       = 108.0f;
inline constexpr float kMeterB       = 26.0f;
inline constexpr float kMeterH       = 256.0f;

inline constexpr float kWerkzeugX = 50.0f;
inline constexpr float kWerkzeugY = 386.0f;
inline constexpr float kWerkzeugB = 650.0f;
inline constexpr float kWerkzeugH = 48.0f;

inline constexpr float kStatusX = 24.0f;
inline constexpr float kStatusY = 446.0f;
inline constexpr float kStatusB = 702.0f;
inline constexpr float kStatusH = 50.0f;

inline juce::Font font (float groesse)
{
    return juce::Font (juce::FontOptions ("Segoe UI", groesse, juce::Font::plain));
}

inline juce::Colour gradientMischen (juce::Colour a, juce::Colour b, float t)
{
    return a.interpolatedWith (b, juce::jlimit (0.0f, 1.0f, t));
}

inline void weicherSchatten (juce::Graphics& g, juce::Rectangle<float> r,
                             float radius, float staerke = 0.32f, float versatzY = 3.0f)
{
    // Mehrere sehr leichte Konturen ergeben auch ohne Bitmap-Blur eine
    // stabile, skalierbare weiche Kante.
    for (int i = 7; i >= 1; --i)
    {
        const float e = (float) i * 0.75f;
        g.setColour (juce::Colours::black.withAlpha (staerke * (8.0f - (float) i) / 34.0f));
        g.fillRoundedRectangle (r.expanded (e * 0.45f).translated (0.0f, versatzY + e * 0.22f),
                                radius + e * 0.45f);
    }
}

inline void metallFlaeche (juce::Graphics& g, juce::Rectangle<float> r, float radius)
{
    const auto oben  = farbe (leitstand::copilot_panel_top);
    const auto unten = farbe (leitstand::copilot_panel_bottom);
    juce::ColourGradient verlauf (oben, r.getX(), r.getY(), unten, r.getX(), r.getBottom(), false);
    verlauf.addColour (0.38, gradientMischen (oben, unten, 0.32f));
    verlauf.addColour (0.72, gradientMischen (oben, unten, 0.63f));
    g.setGradientFill (verlauf);
    g.fillRoundedRectangle (r, radius);

    // Sehr zurückhaltende gebürstete Horizontalstruktur. Sie sitzt im
    // gecachten Rahmen und kostet deshalb nichts im 30-Hz-Paintpfad.
    g.saveState();
    g.reduceClipRegion (r.toNearestInt());
    for (int y = (int) r.getY() + 3; y < (int) r.getBottom(); y += 5)
    {
        const float alpha = ((y / 5) % 2 == 0) ? 0.022f : 0.013f;
        g.setColour (juce::Colours::white.withAlpha (alpha));
        g.drawHorizontalLine (y, r.getX() + 8.0f, r.getRight() - 8.0f);
    }
    g.restoreState();

    g.setColour (farbe (leitstand::copilot_panel_light).withAlpha (0.72f));
    g.drawRoundedRectangle (r.reduced (0.8f), radius - 0.5f, 1.0f);
    g.setColour (farbe (leitstand::copilot_panel_edge).withAlpha (0.86f));
    g.drawRoundedRectangle (r, radius, 1.2f);
}

inline void schraube (juce::Graphics& g, juce::Point<float> p, float s)
{
    const float radius = 4.1f * s;
    g.setColour (juce::Colours::black.withAlpha (0.32f));
    g.fillEllipse (p.x - radius, p.y - radius + 1.0f * s, radius * 2.0f, radius * 2.0f);
    juce::ColourGradient metall (farbe (leitstand::copilot_panel_light), p.x, p.y - radius,
                                 farbe (leitstand::copilot_panel_edge), p.x, p.y + radius, false);
    g.setGradientFill (metall);
    g.fillEllipse (p.x - radius, p.y - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.48f));
    g.drawLine (p.x - 2.1f * s, p.y + 0.8f * s, p.x + 2.1f * s, p.y - 0.8f * s, 0.8f * s);
    g.setColour (juce::Colours::white.withAlpha (0.24f));
    g.drawEllipse (p.x - radius + 0.7f * s, p.y - radius + 0.4f * s,
                   radius * 2.0f - 1.4f * s, radius * 2.0f - 1.4f * s, 0.7f * s);
}

inline void marke (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
{
    // Eigenes Spektrum-/Dialogzeichen: drei Frequenzbögen enden in einem
    // Beratungs-Punkt. Kein Bestandteil der Referenzmarke.
    const float d = juce::jmin (r.getWidth(), r.getHeight());
    const float x = r.getX() + (r.getWidth() - d) * 0.5f;
    const float y = r.getY() + (r.getHeight() - d) * 0.5f;
    juce::Path p;
    p.startNewSubPath (x + d * 0.08f, y + d * 0.66f);
    p.cubicTo (x + d * 0.24f, y + d * 0.63f, x + d * 0.27f, y + d * 0.30f,
               x + d * 0.45f, y + d * 0.30f);
    p.cubicTo (x + d * 0.64f, y + d * 0.30f, x + d * 0.62f, y + d * 0.76f,
               x + d * 0.80f, y + d * 0.56f);
    g.setColour (c);
    g.strokePath (p, juce::PathStrokeType (d * 0.085f, juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded));
    g.setColour (farbe (leitstand::copilot_accent));
    g.fillEllipse (x + d * 0.76f, y + d * 0.50f, d * 0.17f, d * 0.17f);
}

inline void kopfleiste (juce::Graphics& g, juce::Rectangle<float> r, float s)
{
    juce::ColourGradient kopf (farbe (leitstand::copilot_header_top), r.getX(), r.getY(),
                               farbe (leitstand::copilot_header_bottom), r.getX(), r.getBottom(), false);
    g.setGradientFill (kopf);
    g.fillRoundedRectangle (r, 10.0f * s);
    // Die Unterkante überdeckt die unteren Rundungen und ergibt einen klaren
    // Übergang zur Metallplatte.
    g.fillRect (r.getX(), r.getBottom() - 10.0f * s, r.getWidth(), 10.0f * s);
    g.setColour (juce::Colours::white.withAlpha (0.055f));
    g.drawHorizontalLine ((int) (r.getY() + 1.0f * s), r.getX() + 11.0f * s, r.getRight() - 11.0f * s);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawHorizontalLine ((int) r.getBottom(), r.getX(), r.getRight());

    marke (g, { r.getX() + 16.0f * s, r.getY() + 14.0f * s, 28.0f * s, 28.0f * s },
           farbe (leitstand::copilot_text_light));
    g.setColour (farbe (leitstand::copilot_text_light));
    g.setFont (font (10.5f * s));
    g.drawText ("EQ COPILOT", (int) (r.getX() + 50.0f * s), (int) (r.getY() + 13.0f * s),
                (int) (112.0f * s), (int) (16.0f * s), juce::Justification::centredLeft);
    g.setColour (farbe (leitstand::copilot_text_muted));
    g.setFont (font (6.7f * s));
    g.drawText ("SPECTRAL ADVISOR", (int) (r.getX() + 50.0f * s), (int) (r.getY() + 30.0f * s),
                (int) (112.0f * s), (int) (11.0f * s), juce::Justification::centredLeft);

    // Wahrer Produktzustand statt eines dekorativen Fake-Power-Schalters.
    g.setColour (farbe (leitstand::copilot_text_muted));
    g.setFont (font (7.2f * s));
    g.drawText ("TRANSPARENT AUDIO", (int) (r.getRight() - 142.0f * s), (int) (r.getY() + 15.0f * s),
                (int) (126.0f * s), (int) (12.0f * s), juce::Justification::centredRight);
    g.setColour (farbe (leitstand::copilot_accent_soft));
    g.drawText ("0 ms  /  0 tail", (int) (r.getRight() - 142.0f * s), (int) (r.getY() + 29.0f * s),
                (int) (126.0f * s), (int) (12.0f * s), juce::Justification::centredRight);
}

inline juce::Rectangle<float> kopfAnzeige (float s)
{
    return { 205.0f * s, 24.0f * s, 340.0f * s, 29.0f * s };
}

inline void kopfAnzeigeRahmen (juce::Graphics& g, juce::Rectangle<float> r, float s)
{
    g.setColour (juce::Colours::black.withAlpha (0.66f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.2f * s), r.getHeight() * 0.5f);
    juce::ColourGradient glas (farbe (leitstand::copilot_field), r.getX(), r.getY(),
                               farbe (leitstand::copilot_control), r.getX(), r.getBottom(), false);
    g.setGradientFill (glas);
    g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
    g.setColour (juce::Colours::white.withAlpha (0.16f));
    g.drawRoundedRectangle (r.reduced (0.6f), r.getHeight() * 0.5f, 0.8f * s);
    g.setColour (farbe (leitstand::copilot_accent));
    g.fillEllipse (r.getX() + 10.0f * s, r.getCentreY() - 2.0f * s, 4.0f * s, 4.0f * s);
}

inline void kopfAnzeigeText (juce::Graphics& g, juce::Rectangle<float> r,
                             const juce::String& rolle, const juce::String& name, float s)
{
    g.setColour (farbe (leitstand::copilot_text_muted));
    g.setFont (font (6.6f * s));
    g.drawText (rolle.toUpperCase(), (int) (r.getX() + 20.0f * s), (int) (r.getY() + 3.0f * s),
                (int) (88.0f * s), (int) (9.0f * s), juce::Justification::centredLeft);
    g.setColour (farbe (leitstand::copilot_text_light));
    g.setFont (font (9.8f * s));
    g.drawFittedText (name.toUpperCase(), (int) (r.getX() + 20.0f * s), (int) (r.getY() + 12.0f * s),
                      (int) (r.getWidth() - 34.0f * s), (int) (13.0f * s),
                      juce::Justification::centredLeft, 1, 0.86f);
}

inline void display (juce::Graphics& g, juce::Rectangle<float> r, float s)
{
    weicherSchatten (g, r, 13.0f * s, 0.54f, 4.0f * s);
    juce::ColourGradient rand (farbe (leitstand::copilot_display_rim_top), r.getX(), r.getY(),
                               farbe (leitstand::copilot_display_rim_bottom), r.getX(), r.getBottom(), false);
    g.setGradientFill (rand);
    g.fillRoundedRectangle (r, 13.0f * s);
    g.setColour (juce::Colours::black.withAlpha (0.90f));
    g.fillRoundedRectangle (r.reduced (4.0f * s), 10.0f * s);

    const auto glas = r.reduced (8.5f * s);
    juce::ColourGradient innen (farbe (leitstand::copilot_display_top), glas.getX(), glas.getY(),
                                farbe (leitstand::copilot_display_bottom), glas.getX(), glas.getBottom(), false);
    g.setGradientFill (innen);
    g.fillRoundedRectangle (glas, 7.0f * s);

    // Weicher Glanz nur im oberen Viertel, damit die Kurve nicht ausgewaschen wird.
    juce::Path reflex;
    reflex.startNewSubPath (glas.getX(), glas.getY());
    reflex.lineTo (glas.getRight(), glas.getY());
    reflex.lineTo (glas.getRight(), glas.getY() + 42.0f * s);
    reflex.cubicTo (glas.getX() + glas.getWidth() * 0.66f, glas.getY() + 62.0f * s,
                    glas.getX() + glas.getWidth() * 0.32f, glas.getY() + 48.0f * s,
                    glas.getX(), glas.getY() + 58.0f * s);
    reflex.closeSubPath();
    g.setColour (juce::Colours::white.withAlpha (0.035f));
    g.fillPath (reflex);

    g.setColour (juce::Colours::white.withAlpha (0.09f));
    g.drawRoundedRectangle (glas.reduced (0.6f), 6.5f * s, 0.7f * s);
    g.setColour (juce::Colours::black.withAlpha (0.82f));
    g.drawRoundedRectangle (r.reduced (3.0f * s), 10.5f * s, 1.1f * s);
}

inline void eingelasseneFlaeche (juce::Graphics& g, juce::Rectangle<float> r, float radius, float s)
{
    g.setColour (juce::Colours::black.withAlpha (0.29f));
    g.fillRoundedRectangle (r, radius);
    g.setColour (juce::Colours::white.withAlpha (0.22f));
    g.drawRoundedRectangle (r.translated (0.0f, 0.8f * s).reduced (0.7f * s), radius, 0.8f * s);
    g.setColour (juce::Colours::black.withAlpha (0.42f));
    g.drawRoundedRectangle (r.reduced (0.4f * s), radius, 0.9f * s);
}

inline void werkzeugBett (juce::Graphics& g, juce::Rectangle<float> r, float s)
{
    weicherSchatten (g, r, 9.0f * s, 0.20f, 2.0f * s);
    juce::ColourGradient v (farbe (leitstand::copilot_well_top), r.getX(), r.getY(),
                            farbe (leitstand::copilot_well_bottom), r.getX(), r.getBottom(), false);
    g.setGradientFill (v);
    g.fillRoundedRectangle (r, 8.5f * s);
    g.setColour (juce::Colours::white.withAlpha (0.28f));
    g.drawRoundedRectangle (r.reduced (0.7f * s), 8.0f * s, 0.8f * s);
    g.setColour (juce::Colours::black.withAlpha (0.48f));
    g.drawRoundedRectangle (r, 8.5f * s, 1.0f * s);
}

inline void statusBett (juce::Graphics& g, juce::Rectangle<float> r, float s)
{
    eingelasseneFlaeche (g, r, 8.0f * s, s);
    const auto innen = r.reduced (2.0f * s);
    juce::ColourGradient v (farbe (leitstand::copilot_status_top), innen.getX(), innen.getY(),
                            farbe (leitstand::copilot_status_bottom), innen.getX(), innen.getBottom(), false);
    g.setGradientFill (v);
    g.fillRoundedRectangle (innen, 6.6f * s);
}

inline void taste (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour basis,
                   bool hover, bool down, bool aktiv, float s)
{
    if (down)
        r = r.translated (0.0f, 1.0f * s);

    const auto normal = basis.isTransparent() ? farbe (leitstand::copilot_control) : basis;
    auto oben = normal.brighter (hover ? 0.18f : 0.09f);
    auto unten = normal.darker (down ? 0.25f : 0.12f);

    if (! down)
    {
        g.setColour (juce::Colours::black.withAlpha (0.34f));
        g.fillRoundedRectangle (r.translated (0.0f, 2.0f * s), 5.0f * s);
    }
    juce::ColourGradient v (oben, r.getX(), r.getY(), unten, r.getX(), r.getBottom(), false);
    g.setGradientFill (v);
    g.fillRoundedRectangle (r, 5.0f * s);
    g.setColour (aktiv ? farbe (leitstand::copilot_accent)
                       : farbe (leitstand::copilot_control_border));
    g.drawRoundedRectangle (r.reduced (0.45f * s), 4.7f * s, (aktiv ? 1.25f : 0.8f) * s);
    g.setColour (juce::Colours::white.withAlpha (hover ? 0.13f : 0.075f));
    g.drawLine (r.getX() + 5.0f * s, r.getY() + 1.2f * s,
                r.getRight() - 5.0f * s, r.getY() + 1.2f * s, 0.7f * s);
    if (aktiv)
    {
        g.setColour (farbe (leitstand::copilot_accent));
        g.fillRoundedRectangle (r.getX() + 7.0f * s, r.getY() + 2.0f * s,
                                r.getWidth() - 14.0f * s, 1.5f * s, 0.75f * s);
    }
}

inline void tasteText (juce::Graphics& g, const juce::TextButton& b,
                       bool down, float s)
{
    auto r = b.getLocalBounds().toFloat().reduced (4.0f * s, 1.0f * s);
    if (down)
        r.translate (0.0f, 1.0f * s);
    const auto farbeText = b.findColour (b.getToggleState()
                                             ? juce::TextButton::textColourOnId
                                             : juce::TextButton::textColourOffId);
    g.setColour (farbeText);
    g.setFont (font (10.4f * s));
    g.drawFittedText (b.getButtonText().toUpperCase(), r.toNearestInt(),
                      juce::Justification::centred, 1, 0.76f);
}

inline void combo (juce::Graphics& g, juce::Rectangle<float> r, bool down, float s)
{
    taste (g, r, farbe (leitstand::copilot_control), false, down, false, s);
    const auto p = juce::Point<float> (r.getRight() - 11.0f * s, r.getCentreY());
    juce::Path pfad;
    pfad.startNewSubPath (p.x - 3.0f * s, p.y - 1.5f * s);
    pfad.lineTo (p.x, p.y + 1.6f * s);
    pfad.lineTo (p.x + 3.0f * s, p.y - 1.5f * s);
    g.setColour (farbe (leitstand::copilot_accent_soft));
    g.strokePath (pfad, juce::PathStrokeType (1.2f * s, juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
}

inline void led (juce::Graphics& g, juce::Point<float> p, juce::Colour c, float s)
{
    const float r = 4.0f * s;
    g.setColour (c.withAlpha (0.13f));
    g.fillEllipse (p.x - r * 2.0f, p.y - r * 2.0f, r * 4.0f, r * 4.0f);
    g.setColour (juce::Colours::black.withAlpha (0.72f));
    g.fillEllipse (p.x - r * 1.25f, p.y - r * 1.25f, r * 2.5f, r * 2.5f);
    g.setColour (c.darker (0.18f));
    g.fillEllipse (p.x - r, p.y - r, r * 2.0f, r * 2.0f);
    g.setColour (c.brighter (0.45f).withAlpha (0.78f));
    g.fillEllipse (p.x - r * 0.48f, p.y - r * 0.60f, r * 0.75f, r * 0.65f);
}

inline void meter (juce::Graphics& g, juce::Rectangle<float> r, float normalisiert,
                   const juce::String& wert, const juce::String& label, bool gueltig, float s)
{
    const auto track = juce::Rectangle<float> (r.getX() + 6.0f * s, r.getY() + 27.0f * s,
                                                r.getWidth() - 12.0f * s, r.getHeight() - 57.0f * s);
    g.setColour (farbe (leitstand::copilot_ink));
    g.setFont (font (8.2f * s));
    g.drawFittedText (wert, (int) (r.getX() - 8.0f * s), (int) r.getY(),
                      (int) (r.getWidth() + 16.0f * s), (int) (16.0f * s),
                      juce::Justification::centred, 1, 0.78f);

    g.setColour (juce::Colours::black.withAlpha (0.42f));
    g.fillRoundedRectangle (track.expanded (2.0f * s), 3.0f * s);
    g.setColour (farbe (leitstand::copilot_field));
    g.fillRoundedRectangle (track, 1.8f * s);

    constexpr int segmente = 26;
    const float luecke = 1.4f * s;
    const float segmentH = (track.getHeight() - (segmente - 1) * luecke) / (float) segmente;
    const int aktiv = gueltig ? juce::roundToInt (juce::jlimit (0.0f, 1.0f, normalisiert) * segmente) : 0;
    for (int i = 0; i < segmente; ++i)
    {
        const float y = track.getBottom() - (float) (i + 1) * segmentH - (float) i * luecke;
        const bool an = i < aktiv;
        auto c = an ? farbe (leitstand::copilot_accent) : farbe (leitstand::copilot_meter_off);
        if (an && i >= segmente - 3)
            c = farbe (leitstand::copilot_led_red);
        else if (an && i >= segmente - 7)
            c = farbe (leitstand::copilot_accent_soft);
        g.setColour (c.withAlpha (an ? 1.0f : 0.58f));
        g.fillRoundedRectangle (track.getX() + 1.0f * s, y,
                                track.getWidth() - 2.0f * s, segmentH, 0.6f * s);
    }

    g.setColour (farbe (leitstand::copilot_ink_soft));
    g.setFont (font (7.2f * s));
    g.drawFittedText (label.toUpperCase(), (int) (r.getX() - 8.0f * s),
                      (int) (r.getBottom() - 18.0f * s), (int) (r.getWidth() + 16.0f * s),
                      (int) (12.0f * s), juce::Justification::centred, 1, 0.72f);
}

inline void statusZelle (juce::Graphics& g, juce::Rectangle<float> r,
                         const juce::String& titel, const juce::String& wert,
                         float s, juce::Colour wertFarbe = {}, bool trenner = true)
{
    if (trenner)
    {
        g.setColour (juce::Colours::white.withAlpha (0.065f));
        g.drawVerticalLine ((int) r.getX(), r.getY() + 8.0f * s, r.getBottom() - 8.0f * s);
    }
    g.setColour (farbe (leitstand::copilot_text_muted));
    g.setFont (font (7.0f * s));
    g.drawFittedText (titel.toUpperCase(), (int) (r.getX() + 8.0f * s), (int) (r.getY() + 7.0f * s),
                      (int) (r.getWidth() - 13.0f * s), (int) (9.0f * s),
                      juce::Justification::centredLeft, 1, 0.78f);
    g.setColour (wertFarbe.isTransparent() ? farbe (leitstand::copilot_text_light) : wertFarbe);
    g.setFont (font (9.2f * s));
    g.drawFittedText (wert, (int) (r.getX() + 8.0f * s), (int) (r.getY() + 19.0f * s),
                      (int) (r.getWidth() - 13.0f * s), (int) (17.0f * s),
                      juce::Justification::centredLeft, 1, 0.70f);
}

inline void resonanzMarker (juce::Graphics& g, juce::Point<float> spitze,
                            bool dauerhaft, float s)
{
    juce::Path p;
    p.addTriangle (spitze.x, spitze.y, spitze.x - 5.5f * s, spitze.y - 9.5f * s,
                   spitze.x + 5.5f * s, spitze.y - 9.5f * s);
    g.setColour (farbe (leitstand::copilot_marker));
    if (dauerhaft)
        g.fillPath (p);
    else
        g.strokePath (p, juce::PathStrokeType (1.25f * s));
}

// Statische, bei Größenwechsel neu gerenderte Gerätefront.
struct Frame
{
    juce::Image bild;

    void stelleSicher (int breitePx, int hoehePx)
    {
        if (bild.isValid() && bild.getWidth() == breitePx && bild.getHeight() == hoehePx)
            return;
        bild = juce::Image (juce::Image::ARGB, juce::jmax (1, breitePx),
                            juce::jmax (1, hoehePx), true);
        juce::Graphics g (bild);
        male (g, (float) breitePx / kEinheitB);
    }

private:
    static void male (juce::Graphics& g, float s)
    {
        g.fillAll (farbe (leitstand::copilot_stage));
        const auto aussen = juce::Rectangle<float> (kAussenX * s, kAussenY * s,
                                                     kAussenB * s, kAussenH * s);
        weicherSchatten (g, aussen, 14.0f * s, 0.72f, 4.0f * s);
        metallFlaeche (g, aussen, 13.0f * s);

        kopfleiste (g, { 12.0f * s, kKopfY * s, 726.0f * s, kKopfH * s }, s);
        kopfAnzeigeRahmen (g, kopfAnzeige (s), s);

        g.setColour (farbe (leitstand::copilot_ink_soft));
        g.setFont (font (8.2f * s));
        g.drawText ("ANALYSE / BERATUNG", (int) (274.0f * s), (int) (79.0f * s),
                    (int) (202.0f * s), (int) (13.0f * s), juce::Justification::centred);
        g.setColour (farbe (leitstand::copilot_accent));
        g.fillEllipse (269.0f * s, 84.0f * s, 2.5f * s, 2.5f * s);
        g.fillEllipse (478.5f * s, 84.0f * s, 2.5f * s, 2.5f * s);

        display (g, { kDisplayX * s, kDisplayY * s, kDisplayB * s, kDisplayH * s }, s);
        werkzeugBett (g, { kWerkzeugX * s, kWerkzeugY * s, kWerkzeugB * s, kWerkzeugH * s }, s);
        statusBett (g, { kStatusX * s, kStatusY * s, kStatusB * s, kStatusH * s }, s);

        schraube (g, { 19.0f * s, 22.0f * s }, s);
        schraube (g, { 731.0f * s, 22.0f * s }, s);
        schraube (g, { 19.0f * s, 499.0f * s }, s);
        schraube (g, { 731.0f * s, 499.0f * s }, s);
    }
};

} // namespace eqcop::skin
