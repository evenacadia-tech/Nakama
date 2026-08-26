---
id: w-2026-08-26-zugaengliche-audio-controls
typ: wissenseintrag
status: aktiv
datum: 2026-08-26
aktualisiert: 2026-08-26
hauptbereich: ux
weitere_bereiche: [ui-motion, engineering]
pruefstand: mehrfach-belegt
---

# Zugängliche Audio-Controls müssen auch benutzbar sein

## Menschliches Warum

Ein blinder oder sehbehinderter Musiker will nicht nur Pluginparameter
irgendwie erreichen, sondern denselben kreativen Arbeitsablauf ausführen:
Signalweg verstehen, Ziel wählen, Wert ändern, Ergebnis hören und Fehler
rückgängig machen. Ein technisch vorgelesener Wert hilft wenig, wenn dafür
tausende Tastendrücke, eine unverständliche Fokusreihenfolge oder pausenlose
Meteransagen nötig sind.

Die Kausalkette lautet:

> Weil Menschen je nach Sehvermögen andere Wahrnehmungs- und Eingabewege
> benötigen, muss jedes eigene Audio-Control Bedeutung, Zustand, Wert und
> Handlung gleichwertig zugänglich machen, damit aus formaler Erreichbarkeit
> ein selbstständiger musikalischer Arbeitsweg wird.

## Belegtes Wissen

Der Vortrag unterscheidet visuelle von nichtvisueller Zugänglichkeit.
Sichtbarer Tastaturfokus, Vergrößerung, Text und Kontrast helfen anderen
Situationen als Screenreader, Braille, Audio- oder haptische Rückmeldung.
Anpassbare Farben, Text- und Fokusdarstellung können individuelle Barrieren
abbauen.

WCAG und WAI-ARIA liefern dafür Prinzipien, nicht automatisch eine gute
Audio-Bedienung:

- Farbe darf nicht der einzige Informationsträger sein. Auch Fokus,
  Auswahlzustand und grafische Controls brauchen erkennbaren Nicht-Text-
  Kontrast.
- Eigene Controls benötigen einen aussagekräftigen Namen, eine passende Rolle,
  ihren Wert und Zustand sowie die tatsächlich verfügbaren Aktionen.
- Dynamischer Werttext kann mehrdimensionale Controls oder musikalische
  Bedeutung besser ausdrücken als rohe Zahlen allein.
- Die Fokusreihenfolge soll der verständlichen Struktur der Aufgabe folgen.
  Lange lineare Wege belasten Absicht und Arbeitsgedächtnis.
- Accessibility ist nicht gleich Usability: Ein Frequenzregler von 20 bis
  20.000 Hz bleibt trotz vorgelesenem Wert unbrauchbar, wenn die Tastatur nur
  in Ein-Hertz-Schritten arbeitet. Grob-/Feinschritt, Standardwert und sichere
  Grenzsprünge machen den Arbeitsweg praktisch.
- Live-Ansagen brauchen Priorität, wählbare Kadenz und Entprellung. Meter,
  Spektrum oder Wellenform dürfen nicht ununterbrochen sprechen; der Mensch
  muss relevante Messgröße und Abfragezeitpunkt steuern können.
- Ungültige Eingaben sollen abgewiesen und mit gültigem Bereich erklärt
  werden, statt einen gefährlichen Wert kurz anzuwenden.

## Konsequenz für Entwurf und Bedienung

- Jedes eigene EQ-Band, jeder Griff, Slider und Graph braucht zugänglichen
  Namen, Rolle, Wert, Zustand und Aktionen. Ein zugänglicher Container ersetzt
  nicht die Semantik seiner bedienbaren Teile.
- Quellen- und Kurvenfarben erhalten zusätzlich Text, Auswahlmarker, Form oder
  Struktur. „Die rote Kurve“ darf weder in Bedienung noch Dokumentation der
  einzige Identifikator sein.
- Tastaturbedienung muss in musikalisch sinnvollen Schritten arbeiten und
  Grob-/Feinänderung, Rückkehr zum Standard sowie sichere Grenzen anbieten,
  wo diese Handlungen sinnvoll sind.
- Statusmeldungen werden nach Dringlichkeit geordnet. Verbindungsverlust oder
  sicherer Preview-Abbruch sind wichtiger als laufende Messwerte.
- Ein nichtvisueller Messweg liefert eine gezielte Zusammenfassung auf Abruf;
  er übersetzt nicht jeden Spektrum-Frame in Sprache.

## Visuelle Belege

- [09:12 – Farbe als Informationsträger](https://www.youtube.com/watch?v=O5xX9a7P-SU&t=552s):
  zeigt die WCAG-Frage, welche Bedeutung ohne Farbe verloren geht.
- [25:07 – Name, Rolle und Wert](https://www.youtube.com/watch?v=O5xX9a7P-SU&t=1507s):
  zerlegt ein eigenes Control in die Informationen, die Hilfstechnik braucht.
- [30:25 – zugängliche Synthesizer-Oberfläche](https://www.youtube.com/watch?v=O5xX9a7P-SU&t=1825s):
  demonstriert Tastatur- und Screenreader-Bedienung an komplexen Audio-
  Controls.
- [35:45 – nichtvisuelles Super-Meter](https://www.youtube.com/watch?v=O5xX9a7P-SU&t=2145s):
  lässt Messgröße und Ansageintervall auswählen, statt jede Änderung
  ungefiltert auszugeben.

## Bedeutung für Nakama

Nakama bleibt eine native JUCE-VST3-Oberfläche. ARIA wird daher nicht als
Web-Technik in das Plugin übernommen; seine semantischen Prinzipien werden
über JUCEs `AccessibilityHandler` und die zugehörigen Wert-, Zustands- und
Aktionsschnittstellen nativ abgebildet.

Für den EQ Center bedeutet das: Bandnummer, Filtertyp, Frequenz, Gain, Q,
Aktivzustand, Draft/Bestätigung und Schutzkollision müssen nichtvisuell
unterscheidbar sein. Der bereits vorgesehene Alternativweg über `BAND`,
Tastatur und Zahleneingabe wird auf gleiches Ergebnis, sinnvolle Schrittweite
und sicheren Fehlerweg geprüft. Status- und Messansagen brauchen
Entprellung und eine vom Nutzer auslösbare beziehungsweise einstellbare
Kadenz.

Die aktuelle UX-Analyse weist noch keinen belastbaren Accessibility-,
Keyboard- oder Tooltip-Pfad für die grafische Kurve nach. Das Wissen hier
begründet den Prüfbedarf, behauptet aber nicht, dass der native Pfad schon
implementiert ist. Als Nakama-Prüfhypothese sollte die spätere Abnahme in FL
Studio mindestens NVDA, Windows Narrator und einen vollständigen Weg ohne Maus
umfassen.

## Grenzen, Widersprüche und offene Fragen

Viele Beispiele des Vortrags sind Web- und ARIA-basiert. Die Übertragung auf
JUCE ist semantisch möglich, aber konkrete Fokus- und Screenreader-Ergebnisse
hängen von JUCE, Windows, Hilfstechnik und Host ab. Die aktuelle W3C-
Dokumentation stützt Farbe, Nicht-Text-Kontrast und Name/Rolle/Wert; sie beweist
keine vollständige VST3-Bedienbarkeit. Exakte Kontrastwerte und
Tastaturschritte müssen an Nakamas tatsächlichen Controls geprüft werden. Das
Transkript war automatisch erzeugt; der Name David Shervill wurde am
sichtbaren Namensschild verifiziert.

## Quellen

- Titel: *Building Inclusive Audio Tools – Accessibility with ARIA, WCAG, and Real-World Projects*
- Autoren: Samuel John Prowse und David Shervill
- Jahr: 2025
- Video: https://www.youtube.com/watch?v=O5xX9a7P-SU
- W3C, Use of Color: https://www.w3.org/WAI/WCAG22/Understanding/use-of-color
- W3C, Non-text Contrast: https://www.w3.org/WAI/WCAG22/Understanding/non-text-contrast
- W3C, Name, Role, Value: https://www.w3.org/WAI/WCAG22/Understanding/name-role-value
- WAI-ARIA 1.2: https://www.w3.org/TR/wai-aria/
- JUCE `AccessibilityHandler`: https://docs.juce.com/master/classjuce_1_1AccessibilityHandler.html
- JUCE `Component`: https://docs.juce.com/master/classjuce_1_1Component.html
- Eingang: `youtube-transkript` und `web-recherche`
- geprüft: vollständiges Auto-Transkript, ausgewählte Demonstrationen,
  aktuelle W3C-Erläuterungen und JUCE-Referenz

## Verknüpfungen

- [`../../UX-CONTRACT.md`](../../UX-CONTRACT.md)
- [`../../DESIGN.md`](../../DESIGN.md)
- [`../../design/reviews/2026-08-25-audio-plugin-ux-analyse.md`](../../design/reviews/2026-08-25-audio-plugin-ux-analyse.md)
- [`../../design/docs/interaktions-und-motion-spezifikation.md`](../../design/docs/interaktions-und-motion-spezifikation.md)
- [`2026-08-26-audio-interaktion-kontext-statt-annahmen.md`](2026-08-26-audio-interaktion-kontext-statt-annahmen.md)
- [`../engineering/2026-08-26-audiodevcon-videoauswahl.md`](../engineering/2026-08-26-audiodevcon-videoauswahl.md)
