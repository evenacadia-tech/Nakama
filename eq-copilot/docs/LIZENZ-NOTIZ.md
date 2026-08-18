# Lizenznotiz EQ-Copilot — M0-Gate (Plan §10.1/§11)

Stand 2026-08-13. Kontext: **privater Eigengebrauch auf dieser Maschine, keine
Distribution geplant.** Vor jeder Weitergabe des Plugins ist diese Notiz neu zu
bewerten.

## JUCE 8 (gepinnt 8.0.9)

Quelle: `LICENSE.md` im gepinnten Checkout (lokal gelesen, 2026-08-13).
JUCE 8 ist dual-lizenziert: **AGPLv3** oder kommerzielle JUCE-8-Lizenz.
Entscheidung V1: **AGPLv3-Zweig.** Für privaten Eigengebrauch ohne Verteilung
entstehen daraus keine Offenlegungspflichten gegenüber Dritten; der Quellcode
liegt ohnehin im eigenen Repo. `JUCE_DISPLAY_SPLASH_SCREEN=0` ist unter AGPL
zulässig. **Bei Distribution:** AGPL-Pflichten (Quellcode-Angebot) oder
kommerzielle Lizenz — dann neu entscheiden.

## Steinberg VST3 SDK (von JUCE gebündelt)

Quelle: `modules/juce_audio_processors/format_types/VST3_SDK/LICENSE.txt`
(lokal gelesen): wahlweise **Steinberg VST3 License** oder **GPLv3**.
Entscheidung V1: **GPLv3-Zweig** (konsistent mit AGPL-Weg, keine
Steinberg-Registrierungspflicht für private Nutzung ohne Distribution).
**Bei Distribution:** Steinberg-Lizenzvertrag prüfen (Markennutzung „VST").

## Futura PT Book

Die Schrift ist beim User lizenziert für eigene Gestaltung; eine
**Distributionslizenz zum Einbetten in ein verteiltes Plugin ist NICHT
geklärt** (Plan §7.6). Entscheidung M0: das Plugin rendert mit der
**Systemschrift (Segoe UI)**; Futura bleibt Web-/Mockup-Ebene. Erst nach
schriftlicher Klärung darf die Plugin-GUI die Schrift bündeln.

## pluginval (nur Werkzeug)

Wird nur als lokales Prüfwerkzeug benutzt, nicht verlinkt oder verteilt —
keine Lizenzwirkung auf das Produkt.
