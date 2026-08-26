---
id: w-2026-08-26-nutzerzentriertes-audio-design
typ: wissenseintrag
status: aktiv
datum: 2026-08-26
aktualisiert: 2026-08-26
hauptbereich: ux
weitere_bereiche: [produkt, ui-motion, engineering]
pruefstand: quellen-geprueft
---

# Nutzerzentriertes Design für Audio-Produkte

## Das Wichtigste

Ein Audio-Designsystem ist nicht nur eine Sammlung visueller Komponenten. Es
legt über eine Produktfamilie hinweg dieselbe Interaktionssprache fest:
Bedienhandlungen, Rückmeldungen, Begriffe, Licht- und Klangsignale sowie die
Zuordnung von Funktionen zu Controls. Konsistenz senkt den Lernaufwand und
verhindert, dass getrennte Produktteams dieselbe Handlung unterschiedlich
lösen.

Der vorgestellte Prozess trennt zwei Fragen:

1. **Das Richtige bauen:** Nutzer, Kontext, Ziele und Probleme beobachten und
   daraus Anforderungen formulieren. Anforderungen beschreiben zunächst das
   benötigte Ergebnis, nicht vorschnell Slider, Knopf oder Menü.
2. **Es richtig bauen:** Systemverhalten, Controls, Parameterabbildung,
   kontinuierliche oder diskrete Werte, Presets, Defaults und Rückmeldung
   gemeinsam mit der Technik entwerfen, prototypisieren und testen.

Für Audio-Produkte reicht ein Papier- oder Klickprototyp nicht als letzte
Prüfstufe. Ein funktionaler Prototyp muss das tatsächliche Hören, die
Reaktionszeit und technische Grenzen erfahrbar machen. Die Quelle nennt
unterschiedliche Latenzen ausdrücklich als Beispiel für etwas, das nur im
funktionalen Erlebnis sinnvoll beurteilt werden kann.

## Bedeutung für Nakama

- Der Web-Simulator ist richtig eingeordnet: Er muss Zustände, Gesten und
  hörbare Reaktion funktional prüfbar machen. Ein Figma-Klickpfad allein wäre
  für Hold-to-Audition, Preview, Apply und Recovery zu schwach.
- Gen, Probeeq und Suna brauchen eine gemeinsame Interaktionssprache, nicht nur
  dieselben Materialien. Gleiche Aktionen und Zustände müssen über alle drei
  Produkte gleich benannt und rückgemeldet werden.
- Der bestehende UI-Vertrag übernimmt die Rolle einer Interaction Library:
  Aktion, Zustand und sichtbare Rückmeldung werden gemeinsam geführt.
- Technische Grenzen wie Transport, Verbindungsverlust und Latenz gehören in
  die UX-Tests, weil sie das erlebte Produkt direkt verändern.
- Die Quelle bestätigt die Arbeitsrichtung, ändert aber keinen bestehenden
  Nakama-Vertrag und keine User-Entscheidung.

## Visuelle Belege

- [09:35 – Interface-Prinzipien und skalierbare Grundform](https://www.youtube.com/watch?v=UEyGN9DG7Ns&t=575s)
- [15:36 – Prozess von Nutzerforschung zu funktionalem Prototyp](https://www.youtube.com/watch?v=UEyGN9DG7Ns&t=936s)
- [26:56 – Interaktionsbibliothek über mehrere Produkte](https://www.youtube.com/watch?v=UEyGN9DG7Ns&t=1616s)

## Grenzen, Widersprüche und offene Fragen

Der Vortrag ist ein Praxisbericht einer Designagentur, keine kontrollierte
wissenschaftliche Studie. Beispiele stammen überwiegend aus Hardware und
Consumer-Audio; die Übertragung auf ein professionelles VST3 muss deshalb an
Nakama-Aufgaben getestet werden. Das gelesene Transkript war automatisch
erzeugt; Begriffe und Eigennamen können darin fehlerhaft sein.

## Quellen

- Titel: *Enabling Unique Sound Experiences Through User-Centred Design*
- Vortragende: Alexander Häberlin und Vanessa Barrera
- Jahr: unbekannt
- URL: https://www.youtube.com/watch?v=UEyGN9DG7Ns
- Eingang: `youtube-transkript`
- geprüft: vollständiges englisches Auto-Transkript und Videobeschreibung

## Verknüpfungen

- [`../../UX-CONTRACT.md`](../../UX-CONTRACT.md)
- [`../../DESIGN.md`](../../DESIGN.md)
- [`../../design/docs/interaktions-und-motion-spezifikation.md`](../../design/docs/interaktions-und-motion-spezifikation.md)
