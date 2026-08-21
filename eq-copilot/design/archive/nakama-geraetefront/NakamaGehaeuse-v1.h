// NAKAMA-GERÄTEFRONT — User-Entscheid 14.08.2026 („Figma = Plugin-Look“), am 15.08. ohne Protokoll durch die Material-Kit-Front ersetzt (Codex 625b97a); der User hat die Material-Kit-Front nie abgenommen (21.08.: „Nie abgenommen – bleibt Provisorium“). Verlauf; die neue UI kommt aus Figma. Siehe ../LIES-MICH.md
// NAKAMA-Gerätefront des EQ-Copilot-Editors (USER-Design 2026-08-14).
//
// Quelle der Geometrie: der Figma-Entwurf des Users („Display Test", 750×520)
// + die abgenommene Studie NAKAMA-Design-Studie.html im Repo-Root. Kontur,
// Kerben, Lochfelder und der NAKAMA-Schriftzug sind 1:1 die Original-Pfade;
// verändert wie in der Studie: Display größer (hier 64 % Höhe — der Graph ist
// das Herzstück, R3.1), Reflexkante oben + weich, Noppe prozedural, die
// Frontplatte trägt zwei gefräste Inseln für die ECHTEN Werkzeuge (keine
// Dummy-Potis — das Plugin hat bewusst keine Parameter, Plan §0.2).
//
// Alles hier ist statisch: die Front wird EINMAL pro Fenstergröße in ein
// Image gerendert (30-Hz-Repaint blittet nur). Farben ausschließlich aus
// LeitstandTokens.h (nakama_*-Gruppe, generiert aus design/tokens.json).
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LeitstandTokens.h"

namespace eqcop::nakama
{
using leitstand::farbe;

// ── Geräteeinheiten: Original-Figma-Raster. Skala s = Fensterbreite / 750;
//    das Fenster hält 750:520 fest, daher skaliert alles verzerrungsfrei. ──
inline constexpr float kEinheitB = 750.0f;
inline constexpr float kEinheitH = 520.0f;

// Display (= Graph): x/Breite wie Original, Höhe 226 → 331 (Studie-Hebel 1,
// im Plugin noch etwas mehr, weil echte Werkzeuge Platz brauchen).
inline constexpr float kDisplayX = 17.748f;
inline constexpr float kDisplayY = 37.0f;
inline constexpr float kDisplayB = 714.5f;
inline constexpr float kDisplayH = 331.0f;

// Gefräste Inseln auf der Frontplatte (Werkzeuge + Status).
inline constexpr float kInselX    = 24.0f;
inline constexpr float kInselB    = 702.0f;
inline constexpr float kWerkzeugY = 377.0f;
inline constexpr float kWerkzeugH = 46.0f;
inline constexpr float kStatusY   = 431.0f;
inline constexpr float kStatusH   = 46.0f;

// Frontplatten-Kontur (Kerben) — Original-Pfad aus dem Figma-SVG.
inline const char* plattenPfad()
{
    return "M280.019 26.6725C281.916 28.79 284.625 30 287.467 30H462.385C465.222 30 467.925 28.7954 "
           "469.821 26.6861L480.047 15.3139C481.943 13.2046 484.646 12 487.483 12H732C737.523 12 742 "
           "16.4772 742 22V506C742 511.523 737.523 516 732 516H632.546C629.721 516 627.028 514.805 "
           "625.132 512.711L610.268 496.289C608.373 494.195 605.679 493 602.854 493H147.566C144.748 "
           "493 142.061 494.189 140.166 496.275L125.216 512.725C123.321 514.811 120.634 516 117.815 "
           "516H18C12.4772 516 8 511.523 8 506V22C8 16.4772 12.4772 12 18 12H262.408C265.25 12 "
           "267.959 13.21 269.856 15.3275L280.019 26.6725Z";
}

// NAKAMA-Schriftzug — Original-Pfad (outlined Text, kein Font nötig).
inline const char* schriftzugPfad()
{
    return "M327.481 16H326.493V8.26562H327.879L332.578 13.5776V8.26562H333.561V16H332.176L327.481 "
           "10.6934V16ZM352.246 12.2993V12.4014C352.239 12.7021 352.207 13.0137 352.149 13.3359C352.096 "
           "13.6582 352.019 13.9805 351.918 14.3027C351.818 14.625 351.698 14.9437 351.559 15.2588C351.419 "
           "15.5739 351.261 15.8747 351.086 16.1611C350.914 16.4512 350.728 16.7215 350.527 16.9722C350.327 "
           "17.2264 350.116 17.4502 349.894 17.6436L349.093 17.1816C349.219 16.9704 349.331 16.7394 349.432 "
           "16.4888C349.535 16.2417 349.629 15.9821 349.711 15.71C349.793 15.4378 349.863 15.1585 349.92 "
           "14.8721C349.981 14.5856 350.031 14.3009 350.071 14.0181C350.11 13.7352 350.139 13.4577 350.157 "
           "13.1855C350.178 12.9098 350.189 12.6484 350.189 12.4014C350.189 12.208 350.187 11.9896 350.184 "
           "11.7461C350.184 11.499 350.169 11.2484 350.141 10.9941C350.116 10.7399 350.073 10.4928 350.012 "
           "10.2529C349.951 10.0094 349.86 9.79281 349.738 9.60303C349.62 9.41325 349.466 9.26107 349.276 "
           "9.14648C349.09 9.0319 348.857 8.97461 348.578 8.97461C348.331 8.97461 348.117 9.01758 347.938 "
           "9.10352C347.763 9.18587 347.613 9.29867 347.487 9.44189C347.366 9.58154 347.267 9.74447 347.192 "
           "9.93066C347.12 10.1169 347.063 10.3138 347.02 10.5215C346.981 10.7256 346.954 10.9333 346.939 "
           "11.1445C346.929 11.3522 346.923 11.5492 346.923 11.7354V13.2339H349.238V14.1201H346.923V16H344.791"
           "V12.0684C344.791 11.5062 344.881 10.9834 345.06 10.5C345.239 10.0166 345.491 9.59766 345.817 "
           "9.24316C346.146 8.88509 346.54 8.60579 346.999 8.40527C347.46 8.20117 347.974 8.09912 348.54 "
           "8.09912C348.923 8.09912 349.281 8.15462 349.614 8.26562C349.951 8.37305 350.257 8.52344 350.533 "
           "8.7168C350.808 8.90658 351.054 9.13395 351.269 9.39893C351.487 9.66032 351.668 9.94678 351.811 "
           "10.2583C351.958 10.5698 352.069 10.8993 352.144 11.2466C352.219 11.5939 352.253 11.9448 352.246 "
           "12.2993ZM365.34 16H363.207V8.26562H365.34V11.9502C365.998 11.4453 366.612 10.8849 367.182 "
           "10.269C367.755 9.65316 368.247 8.98356 368.659 8.26025H370.114C369.735 8.90837 369.3 9.51351 "
           "368.809 10.0757C368.319 10.6379 367.785 11.1589 367.209 11.6387L370.297 16H368.073L365.748 "
           "12.7129C365.68 12.7594 365.612 12.8042 365.544 12.8472C365.479 12.8901 365.411 12.9331 365.34 "
           "12.9761V16ZM388.09 12.2993V12.4014C388.083 12.7021 388.051 13.0137 387.993 13.3359C387.94 "
           "13.6582 387.863 13.9805 387.763 14.3027C387.662 14.625 387.542 14.9437 387.403 15.2588C387.263 "
           "15.5739 387.105 15.8747 386.93 16.1611C386.758 16.4512 386.572 16.7215 386.371 16.9722C386.171 "
           "17.2264 385.96 17.4502 385.738 17.6436L384.937 17.1816C385.063 16.9704 385.175 16.7394 385.276 "
           "16.4888C385.38 16.2417 385.473 15.9821 385.555 15.71C385.637 15.4378 385.707 15.1585 385.764 "
           "14.8721C385.825 14.5856 385.875 14.3009 385.915 14.0181C385.954 13.7352 385.983 13.4577 386.001 "
           "13.1855C386.022 12.9098 386.033 12.6484 386.033 12.4014C386.033 12.208 386.031 11.9896 386.028 "
           "11.7461C386.028 11.499 386.013 11.2484 385.985 10.9941C385.96 10.7399 385.917 10.4928 385.856 "
           "10.2529C385.795 10.0094 385.704 9.79281 385.582 9.60303C385.464 9.41325 385.31 9.26107 385.12 "
           "9.14648C384.934 9.0319 384.701 8.97461 384.422 8.97461C384.175 8.97461 383.962 9.01758 383.783 "
           "9.10352C383.607 9.18587 383.457 9.29867 383.331 9.44189C383.21 9.58154 383.111 9.74447 383.036 "
           "9.93066C382.964 10.1169 382.907 10.3138 382.864 10.5215C382.825 10.7256 382.798 10.9333 382.784 "
           "11.1445C382.773 11.3522 382.767 11.5492 382.767 11.7354V13.2339H385.082V14.1201H382.767V16H380.635"
           "V12.0684C380.635 11.5062 380.725 10.9834 380.904 10.5C381.083 10.0166 381.335 9.59766 381.661 "
           "9.24316C381.99 8.88509 382.384 8.60579 382.843 8.40527C383.305 8.20117 383.818 8.09912 384.384 "
           "8.09912C384.767 8.09912 385.125 8.15462 385.458 8.26562C385.795 8.37305 386.101 8.52344 386.377 "
           "8.7168C386.653 8.90658 386.898 9.13395 387.113 9.39893C387.331 9.66032 387.512 9.94678 387.655 "
           "10.2583C387.802 10.5698 387.913 10.8993 387.988 11.2466C388.063 11.5939 388.097 11.9448 388.09 "
           "12.2993ZM400.485 10.2314L399.884 16H398.568L399.884 8.26562H401.834L403.644 14.1685L405.475 "
           "8.26562H407.425L408.521 16H406.351L405.711 10.3818L404.202 16H402.252L400.485 10.2314ZM426.867 "
           "12.2993V12.4014C426.86 12.7021 426.828 13.0137 426.77 13.3359C426.717 13.6582 426.64 13.9805 "
           "426.539 14.3027C426.439 14.625 426.319 14.9437 426.179 15.2588C426.04 15.5739 425.882 15.8747 "
           "425.707 16.1611C425.535 16.4512 425.349 16.7215 425.148 16.9722C424.948 17.2264 424.736 17.4502 "
           "424.514 17.6436L423.714 17.1816C423.839 16.9704 423.952 16.7394 424.052 16.4888C424.156 16.2417 "
           "424.249 15.9821 424.332 15.71C424.414 15.4378 424.484 15.1585 424.541 14.8721C424.602 14.5856 "
           "424.652 14.3009 424.692 14.0181C424.731 13.7352 424.76 13.4577 424.778 13.1855C424.799 12.9098 "
           "424.81 12.6484 424.81 12.4014C424.81 12.208 424.808 11.9896 424.804 11.7461C424.804 11.499 "
           "424.79 11.2484 424.761 10.9941C424.736 10.7399 424.693 10.4928 424.633 10.2529C424.572 10.0094 "
           "424.48 9.79281 424.359 9.60303C424.24 9.41325 424.086 9.26107 423.897 9.14648C423.71 9.0319 "
           "423.478 8.97461 423.198 8.97461C422.951 8.97461 422.738 9.01758 422.559 9.10352C422.384 9.18587 "
           "422.233 9.29867 422.108 9.44189C421.986 9.58154 421.888 9.74447 421.813 9.93066C421.741 10.1169 "
           "421.684 10.3138 421.641 10.5215C421.601 10.7256 421.575 10.9333 421.56 11.1445C421.55 11.3522 "
           "421.544 11.5492 421.544 11.7354V13.2339H423.859V14.1201H421.544V16H419.412V12.0684C419.412 "
           "11.5062 419.501 10.9834 419.68 10.5C419.859 10.0166 420.112 9.59766 420.438 9.24316C420.767 "
           "8.88509 421.161 8.60579 421.619 8.40527C422.081 8.20117 422.595 8.09912 423.161 8.09912C423.544 "
           "8.09912 423.902 8.15462 424.235 8.26562C424.572 8.37305 424.878 8.52344 425.154 8.7168C425.429 "
           "8.90658 425.675 9.13395 425.889 9.39893C426.108 9.66032 426.289 9.94678 426.432 10.2583C426.579 "
           "10.5698 426.69 10.8993 426.765 11.2466C426.84 11.5939 426.874 11.9448 426.867 12.2993Z";
}

// Die gecachte Gerätefront. stelleSicher() rendert nur bei Größenwechsel neu.
struct Front
{
    juce::Image bild;

    void stelleSicher (int breitePx, int hoehePx)
    {
        if (bild.isValid() && bild.getWidth() == breitePx && bild.getHeight() == hoehePx)
            return;
        bild = juce::Image (juce::Image::ARGB, juce::jmax (1, breitePx), juce::jmax (1, hoehePx), true);
        juce::Graphics g (bild);
        male (g, (float) breitePx / kEinheitB);
    }

private:
    // Noppen-Tile (Studie-Hebel 3): Basiston + gestanztes Loch + Lichtkante.
    // Tilegröße rundet auf ganze Pixel — sonst flimmert das Raster (Moiré).
    static juce::Image noppenTile (float s)
    {
        const int t = juce::jmax (3, juce::roundToInt (3.6f * s));
        juce::Image tile (juce::Image::ARGB, t, t, true);
        juce::Graphics g (tile);
        g.fillAll (farbe (leitstand::nakama_platte));
        const float m = (float) t;
        g.setColour (juce::Colours::black.withAlpha (0.28f));
        g.fillEllipse (m * 0.21f, m * 0.21f, m * 0.58f, m * 0.58f);
        g.setColour (juce::Colours::white.withAlpha (0.065f));
        g.fillEllipse (m * 0.21f, m * 0.17f, m * 0.44f, m * 0.44f);
        g.setColour (juce::Colours::black.withAlpha (0.20f));
        g.fillEllipse (m * 0.33f, m * 0.35f, m * 0.34f, m * 0.34f);
        return tile;
    }

    void male (juce::Graphics& g, float s)
    {
        const auto E = [s] (float v) { return v * s; };

        // 1 · Bühne + Gehäuse: schwarzer Rahmen mit Hauch von Licht oben.
        //     (Der Original-Radial-Sweep ist durch die 93-%-Dämpfung praktisch
        //     unsichtbar — ein vertikaler Hauch trifft denselben Eindruck.)
        g.fillAll (farbe (leitstand::nakama_buehne));
        juce::Path gehaeuse;
        gehaeuse.addRoundedRectangle (0.0f, 0.0f, E (kEinheitB), E (kEinheitH), E (12.0f));
        g.setColour (farbe (leitstand::nakama_gehaeuse));
        g.fillPath (gehaeuse);
        {
            juce::ColourGradient licht (juce::Colours::white.withAlpha (0.05f), 0.0f, 0.0f,
                                        juce::Colours::transparentWhite, 0.0f, E (90.0f), false);
            g.setGradientFill (licht);
            g.fillPath (gehaeuse);
        }

        // 2 · Frontplatte: Original-Kerbenpfad, prozedurale Noppe, Lichtverlauf.
        auto platte = juce::Drawable::parseSVGPath (juce::String (plattenPfad()));
        platte.applyTransform (juce::AffineTransform::scale (s));
        g.setColour (farbe (leitstand::nakama_platte));
        g.fillPath (platte);
        {
            juce::Graphics::ScopedSaveState clip (g);
            g.reduceClipRegion (platte);
            g.setTiledImageFill (noppenTile (s), 0, 0, 1.0f);
            g.fillPath (platte);
            juce::ColourGradient tiefe (juce::Colours::white.withAlpha (0.05f), 0.0f, E (12.0f),
                                        juce::Colours::black.withAlpha (0.15f), 0.0f, E (516.0f), false);
            tiefe.addColour (0.25, juce::Colours::white.withAlpha (0.012f));
            tiefe.addColour (0.80, juce::Colours::black.withAlpha (0.05f));
            g.setGradientFill (tiefe);
            g.fillPath (platte);
        }
        g.setColour (farbe (leitstand::nakama_platte_kontur));
        g.strokePath (platte, juce::PathStrokeType (E (2.0f)));

        // 3 · Lochfelder: Original-Raster (6 Felder à 2×17, 4,5er-Takt).
        //     Der 0,22-Blauschimmer des Originals ist bei Plugin-Größe unter
        //     einem Pixel — bewusst weggelassen, kein stiller Fake.
        {
            juce::Path loecher;
            const float felder[4][2] = { { 182.5f, 19.5f }, { 494.5f, 19.5f },
                                         { 33.0f, 490.5f }, { 644.0f, 490.5f } };
            for (const auto& f : felder)
                for (int reihe = 0; reihe < 2; ++reihe)
                    for (int i = 0; i < 17; ++i)
                        loecher.addEllipse (E (f[0] + (float) i * 4.5f - 1.5f),
                                            E (f[1] + (float) reihe * 5.0f - 1.5f),
                                            E (3.0f), E (3.0f));
            g.setColour (juce::Colours::black);
            g.fillPath (loecher);
        }

        // 4 · NAKAMA-Schriftzug (Original-Pfad, graviert).
        auto zug = juce::Drawable::parseSVGPath (juce::String (schriftzugPfad()));
        zug.applyTransform (juce::AffineTransform::scale (s));
        g.setColour (farbe (leitstand::nakama_schriftzug));
        g.fillPath (zug);

        // 5 · Display: tiefschwarz, oben links minimal offen; Reflexkante im
        //     oberen Drittel mit ECHTER Weichzeichnung (Studie-Hebel 2).
        const juce::Rectangle<float> display (E (kDisplayX), E (kDisplayY), E (kDisplayB), E (kDisplayH));
        g.setColour (farbe (leitstand::nakama_display_grund));
        g.fillRoundedRectangle (display, E (12.0f));
        {
            juce::ColourGradient grund (farbe (leitstand::nakama_display_tief),
                                        display.getX(), display.getY(),
                                        farbe (leitstand::nakama_display_grund),
                                        display.getX() + display.getWidth() * 0.35f,
                                        display.getY() + display.getHeight() * 0.8f, false);
            g.setGradientFill (grund);
            g.fillRoundedRectangle (display, E (12.0f));
        }
        {
            // Reflex hart in ein Zwischenbild, dann Gauß — so bleibt die Feder
            // entlang der GEBOGENEN Kante überall gleich weich.
            const auto db = display.getSmallestIntegerContainer();
            juce::Image reflex (juce::Image::ARGB, juce::jmax (1, db.getWidth()),
                                juce::jmax (1, db.getHeight()), true);
            {
                juce::Graphics rg (reflex);
                juce::Path kante;
                kante.startNewSubPath (0.0f, E (148.0f - kDisplayY));
                kante.cubicTo (E (212.0f), E (166.0f - kDisplayY),
                               E (502.0f), E (138.0f - kDisplayY),
                               E (kDisplayB), E (108.0f - kDisplayY));
                kante.lineTo (E (kDisplayB), 0.0f);
                kante.lineTo (0.0f, 0.0f);
                kante.closeSubPath();
                rg.setColour (juce::Colours::white.withAlpha (0.06f));
                rg.fillPath (kante);
            }
            juce::ImageConvolutionKernel weich (juce::jmax (3, juce::roundToInt (E (4.0f))));
            weich.createGaussianBlur (juce::jmax (1.5f, E (3.2f)));
            const juce::Image quelle = reflex.createCopy();   // Kernel braucht getrennte Quelle
            weich.applyToImage (reflex, quelle, reflex.getBounds());
            juce::Graphics::ScopedSaveState clip (g);
            juce::Path displayClip;
            displayClip.addRoundedRectangle (display, E (12.0f));
            g.reduceClipRegion (displayClip);
            g.drawImageAt (reflex, db.getX(), db.getY());
        }
        // Glas-Einfassung + Lichtlinie an der Unterkante.
        g.setColour (juce::Colours::black.withAlpha (0.85f));
        g.drawRoundedRectangle (display.reduced (E (0.75f)), E (11.4f), E (1.5f));
        g.setColour (juce::Colours::white.withAlpha (0.07f));
        g.drawLine (E (32.0f), display.getBottom() + E (0.8f),
                    E (718.0f), display.getBottom() + E (0.8f), E (1.0f));

        // 6 · Gefräste Inseln (Fräsung = Noppe entfernt; Fugen-Doppelkante,
        //     Licht von oben) — hierauf sitzen die echten Werkzeuge.
        auto insel = [&] (float y, float h)
        {
            const juce::Rectangle<float> r (E (kInselX), E (y), E (kInselB), E (h));
            g.setColour (farbe (leitstand::nakama_platte_insel));
            g.fillRoundedRectangle (r, E (10.0f));
            g.setColour (juce::Colours::black.withAlpha (0.45f));
            g.drawRoundedRectangle (r, E (10.0f), 1.0f);
            g.setColour (juce::Colours::white.withAlpha (0.06f));
            g.drawRoundedRectangle (r.translated (0.0f, 1.2f), E (10.0f), 1.0f);
        };
        insel (kWerkzeugY, kWerkzeugH);
        insel (kStatusY, kStatusH);
    }
};

} // namespace eqcop::nakama
