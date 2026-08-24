# Bausteine — Nakama Control Library

Die Übersetzung der vom User bereitgestellten UI-Assets (2026-08-24,
„diese assets sind gold") in **bedienbare, skalierbare Bausteine** für die
Plugin-Oberflächen. Quelle in der Truhe:
`../assets/figma/2026-08-24-ui-assets/` (dort LIES-MICH mit Herkunft und
Hashes). Kein Build, per Doppelklick auf `index.html` lauffähig.

**Was das ist — und was nicht.** Die Vorlagen sind Raster-Renderings; die
Plugins brauchen Formen mit Zuständen (hover, drag, deaktiviert) und
Skalierung. Darum ist jeder Baustein als CSS/SVG-Rezept nachgebaut, die
Vorlage bleibt daneben als Maßstab abrufbar („Ausschnitte zeigen").
Das Blatt entscheidet **keine** Produktfragen: welcher Baustein wohin
kommt, entscheidet der User am Figma-Stand (die drei Familien A/B/C liegen
in derselben Figma-Datei, Section `108:15`).

## Inventar (Baustein → Quelle)

| Baustein | Quelle im Asset |
|---|---|
| Knob **Hero** (Leuchtbogen + Punktkranz + Wertpunkt auf der Kappe; S/M/L) | Delay-UI, rechter Knopf (`presentation-1.png`) |
| Knob **Orbit** (Kugel-Griff läuft auf der Kreisbahn) | Delay-UI, linker Knopf |
| Knob **Pointer** (gebürstetes Pedal-Metall, Zeigerstrich; Variante mit fester Wort-Skala) | Guitar-Pedal (`guitar-pedal.jpg`), TIME/BALANCE |
| **Slider** (Punkt-Ticks, Fasen-Griff mit Fingerrille, leuchtender Wertweg) | Delay-UI, GAIN/MIX |
| **Range** (Doppelgriff LPASS/HPASS im eingelassenen Rahmen) | Delay-UI, Filterleiste |
| **Pille** rastend (gedrückt = einwärts + Akzentpunkt) | Delay-UI, LINK/DUCK/SYNC |
| **Segment**-Schalter (aktive Hälfte eingedrückt) | Delay-UI, PING/PONG |
| **Kippschalter** 3 Stellungen (Gewindering, Chromhebel) | Guitar-Pedal, SYNC-OFF-FREE u. a. |
| **LED** (Bezel + Glut, akzentfarbig) | Guitar-Pedal, TAP/BYPASS-Punkte |
| **Fußschalter** (Schraubring + Tastfläche, schaltet seine LED) | Guitar-Pedal, Footswitches |
| **Dock** (runde Soft-Tasten in Pillenwanne, genau eine aktiv) | Figue.io-Toggle-Nav (`beleg-toggle-nav-viewport.png`) |
| **Power**, **Stepper** ‹ › mit Preset-Anzeige | Delay-UI, Kopfzeile |
| **Gehäuse** (Schale mit großem Radius) + **Einlass**-Gruppe | Delay-UI, Korpus + Filterkasten |

## Regeln des Blatts

- **Keine toten Elemente:** jeder Baustein ist bedienbar (ziehen, klicken,
  Tastatur: Pfeile/Shift, Doppelklick = Ausgangswert) und zeigt seinen
  echten lokalen Zustand. Werte sind Demo — Standband sagt es.
- **Bedien-Panel** (rechts, deutsch): Thema dunkel/hell · Akzentfarbe
  (Nakama-Rot Voreinstellung, Referenz-Neutral, Pedal-Grün, frei) ·
  Grundton/Radius/Typo/Dichte live · Zustand „Deaktiviert" (ehrlich stumpf,
  Werte „—") · Vorlagen-Ausschnitte · Werte mischen.
- **Produktsprache englisch** auf den Spezimen, Chrome deutsch.
- Beurteilt wird bei **1:1**; die Größenreihe zeigt echte Pixelgrößen
  nebeneinander statt zu skalieren.
- Schrift: Geist / Geist Mono aus `../prototyp/assets/fonts/` (dieselben
  Dateien wie der Simulator).

## Offene Punkte (für den User, nicht hier entscheiden)

1. **Akzent je App:** Die Gen-Familien A/B/C nutzen Rot; das Pedal Grün,
   die Delay-Referenz Neutral. Voreinstellung hier: Nakama-Rot `#b04038`
   (aus den Gen-Ständen gegriffen, nicht abgenommen).
2. **Fußschalter im Plugin:** als große Bypass-Geste denkbar — gehört er in
   die Nakama-Formensprache oder bleibt er Pedal-Vokabular?
3. **Dock als Seiten-/Sondenwechsler in Gen** (Overview · Routing · Advisor
   · EQ Center): Vorschlag, kein Entscheid.

## Stand

Angelegt 2026-08-24 auf den User-Auftrag im Chat (Wortlaut in der
Truhen-LIES-MICH). Noch ohne Abnahme — das Blatt ist die Vorlage für das
Gespräch, nicht sein Ergebnis.
