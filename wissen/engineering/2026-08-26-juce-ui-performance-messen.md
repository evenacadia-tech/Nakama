---
id: w-2026-08-26-juce-ui-performance-messen
typ: wissenseintrag
status: aktiv
datum: 2026-08-26
aktualisiert: 2026-08-26
hauptbereich: engineering
weitere_bereiche: [ui-motion, ux, fl-studio]
pruefstand: quellen-geprueft
---

# JUCE-UI-Performance messen statt vermuten

## Menschliches Warum

Ein EQ-Griff fühlt sich nur dann direkt an, wenn Cursor, Griff, Zahlenwert und
Kurve als eine Handlung erscheinen. Stockt die Darstellung, wird die hörbare
Änderung schwerer der eigenen Bewegung zugeordnet. Gleichzeitig teilt sich ein
Plugin die Ressourcen mit Host, Audioverarbeitung und weiteren Instanzen. Eine
ruhende Oberfläche darf deshalb nicht unbemerkt weiter Rechenzeit verbrauchen.

Die Kausalkette lautet:

> Weil ein Produzent während einer feinen Hörentscheidung eine unmittelbare
> Ursache-Wirkungs-Beziehung braucht, muss die UI am echten Interaktionspfad
> gemessen und nur bei wirklicher Änderung neu gezeichnet werden, damit
> Bediengefühl und Host-Stabilität nicht durch versteckte Paint-Arbeit leiden.

## Belegtes Wissen

Der Vortrag beginnt bei JUCEs Repaint-Diagnose, weist aber auf ihre Grenze hin:
Sie zeigt unnötig gezeichnete Regionen, nicht automatisch Ursache oder Dauer.
Für eine belastbare Analyse müssen Komponentenbaum, `paint()`,
`paintOverChildren()`, Effekte und Framework-Overhead getrennt profiliert
werden.

Die wichtigsten technischen Befunde sind bedingt, nicht universal:

- `setPaintingIsUnclipped()` ist nur korrekt, wenn eine Komponente garantiert
  nie außerhalb ihrer Grenzen zeichnet. Der mögliche Gewinn ist in aktuellen
  JUCE-Versionen kleiner und muss gemessen werden.
- `setOpaque()` kann darunterliegende Paint-Arbeit vermeiden, besitzt aber
  selbst Prüfkosten. Viele kleine opake Nachbarn können langsamer sein.
- `setBufferedToImage()` puffert eine Komponente samt Kindern. Ein animiertes
  Kind invalidiert deshalb den ganzen Puffer und kann mehr Arbeit erzeugen als
  ungepuffertes Zeichnen.
- Eine günstige Struktur trennt teure statische Hintergründe von bewegten
  Geschwistern. Container ordnen Kinder, zeichnen aber möglichst nicht selbst;
  cachebare Zeichnung liegt in isolierten Blattkomponenten.
- Betriebssystem, Anzeige, Energiezustand, Mausbewegung und Release-/Debug-Bau
  beeinflussen Messungen. Relevant ist der Release-Bau in der Zielumgebung.
- Textformung kann teuer sein. Unveränderter Text kann gepuffert oder als
  vorbereitete Glyphenanordnung wiederverwendet werden; bewegliche Werte
  brauchen eine andere Strategie als statische Beschriftung.

Plattformspezifische Schalter bleiben Risikotausch: Unter Windows kann das
Abschalten bestimmter Direct2D-Bildsicherungen schneller sein, verlangt dann
aber Tests für Geräteverlust und Remote-Sitzungen. Auch die macOS-Option für
mehrere Paint-Aufrufe kann je nach Oberfläche helfen oder schaden.

## Konsequenz für Entwurf und Bedienung

- Performance wird pro echte Handlung gemessen: Band ziehen, Seite wechseln,
  Spektrum aktualisieren, Draft morphen, Skalierungsstufe wechseln und Fenster
  wiederherstellen.
- Statische Schale, Material und unveränderte Beschriftungen erhalten andere
  Repaint-Grenzen als Spektrum, Messdaten und EQ-Kurve.
- Ein Cache ist nur sinnvoll, wenn sein Inhalt länger stabil bleibt als seine
  Erzeugung kostet. Ein bewegtes Kind darf nicht unbemerkt einen großen
  Elternpuffer pro Frame entwerten.
- Durchschnittswerte allein reichen nicht. Ausreißer sind beim Ziehen direkt
  spürbar; deshalb gehören Perzentile und schlechtester beobachteter Frame in
  die Abnahme.
- Optimierungsflags werden dokumentiert und mit Vorher-/Nachhermessung
  abgesichert. Der Name eines Flags ist kein Leistungsbeweis.

## Visuelle Belege

- [08:00 – Paint-Diagnose nach Komponentenebenen](https://www.youtube.com/watch?v=0n9x6R0fheo&t=480s):
  farbige Regionen und Zeitgraph verbinden sichtbare Fläche mit Kosten.
- [21:34 – Komponenten- und Cache-Hierarchie](https://www.youtube.com/watch?v=0n9x6R0fheo&t=1294s):
  zeigt, warum ein animiertes Kind den Puffer seines Elternteils entwertet.
- [28:17 – gemessener Repaint-Vergleich](https://www.youtube.com/watch?v=0n9x6R0fheo&t=1697s):
  stellt Codeänderung und beobachtete Draw-Zeit nebeneinander.
- [37:44 – Zusammenfassung der Messregeln](https://www.youtube.com/watch?v=0n9x6R0fheo&t=2264s):
  ordnet Diagnostik, Caching und plattformspezifische Optionen ein.

## Bedeutung für Nakama

Für die spätere native Oberfläche folgt daraus eine prüfbare
Komponentenaufteilung: statische Schale und Material, unveränderte Texte,
dynamischer Graph, Spektrum und transiente Controls werden nicht in einen
gemeinsamen großen Cache gezwungen. Die bestehende Vorgabe, dass direkte
Manipulation Griff, Wert und Kurve im selben Frame aktualisiert, ist zugleich
ein UX- und ein Performance-Test.

Die Messmatrix sollte den Windows-Release-Bau in FL Studio, alle vier
Skalierungsstufen, mehrere Plugin-Instanzen, Stillstand und Interaktion
umfassen. Nur echte Bewegung oder neue Daten lösen Repaints aus. Falls die
Direct2D-Bildsicherung abgeschaltet wird, gehören Geräteverlust,
Fensterwiederöffnung und Remote-Sitzung zwingend in den Gegenbeweis.

## Grenzen, Widersprüche und offene Fragen

Der Vortrag liefert Diagnosemuster und Fallbeispiele, aber keine universellen
Grenzwerte für Nakama. Messwerte aus der Präsentation lassen sich nicht auf
eine andere Komponentenstruktur oder Grafikhardware übertragen. Das
Transkript wurde automatisch erzeugt; API-Aussagen wurden deshalb an der
aktuellen JUCE-Dokumentation gegengeprüft. Welche Cache-Grenzen für Nakama
gewinnen, entscheidet erst der native Release-Benchmark.

## Quellen

- Titel: *Measuring and Improving UI Performance with the JUCE C++ Framework*
- Autor: Anthony Nicholls
- Jahr: 2025
- Video: https://www.youtube.com/watch?v=0n9x6R0fheo
- JUCE `Component`: https://docs.juce.com/master/classjuce_1_1Component.html
- JUCE `ImagePixelDataBackupExtensions`: https://docs.juce.com/master/classjuce_1_1ImagePixelDataBackupExtensions.html
- JUCE `ComponentPaintDiagnostics`: https://docs.juce.com/develop/structjuce_1_1ComponentPaintDiagnostics.html
- JUCE Breaking Changes: https://github.com/juce-framework/JUCE/blob/master/BREAKING_CHANGES.md
- Eingang: `youtube-transkript` und `web-recherche`
- geprüft: vollständiges Auto-Transkript, ausgewählte Demonstrationen und
  aktuelle JUCE-Referenz

## Verknüpfungen

- [`../../DESIGN.md`](../../DESIGN.md)
- [`../../UX-CONTRACT.md`](../../UX-CONTRACT.md)
- [`../../design/docs/interaktions-und-motion-spezifikation.md`](../../design/docs/interaktions-und-motion-spezifikation.md)
- [`2026-08-26-audiodevcon-videoauswahl.md`](2026-08-26-audiodevcon-videoauswahl.md)
