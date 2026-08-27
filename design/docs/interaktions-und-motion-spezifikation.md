# Interaktions- und Motion-Spezifikation

> **Status:** Arbeitsvorgabe vom 23.08.2026, auf die Produktstruktur vom
> 27.08.2026 berichtigt.  
> **Geltung:** Ergänzt die drei aktuellen Rework-Bilder um Bedienung und
> Zustandswechsel, ohne ihre visuelle Sprache umzudeuten.  
> **Grundlage:** [Oberflächen-Spezifikation](oberflaechen-spezifikation.md),
> [Arbeitsplan](arbeitsplan.md),
> [aktuelle Rework-Referenz](../abnahmen/2026-08-25-rework-referenz-drei-designs.md)
> und der technische Entwurf
> [FL-Nakama-Sonden-Design](../../docs/FL-Nakama-Sonden-Design-Entwurf.md).

## 1. Grundsatz

Die drei aktuellen User-Bilder bleiben die visuelle Referenz:

- [Gen Fläche 1](../assets/rework-basis-2026-08-25/gen-page-1-overview.png)
- [Gen Fläche 2](../assets/rework-basis-2026-08-25/gen-page-2-eq-center.png)
- [Probeeq](../assets/rework-basis-2026-08-25/probeeq.png)

Die Bilder zeigen Material, Anordnung und einzelne Zustände. Im fertigen
Plugin werden EQ-Graph, Bänder, Auswahl, Statusanzeigen, Messwerte und
Bewegungen aus dem tatsächlichen Zustand gezeichnet. Eine exportierte,
statische Kurve darf nicht als scheinbar lebendiger EQ-Graph verwendet werden.

Jede sichtbare Bewegung muss einen echten Grund haben:

- Direkte Mausbewegung folgt ohne Verzögerung. Kein Nachziehen, kein Feder-Effekt, kein Springen.
- Wechsel, die nicht direkt von der Maus kommen – etwa Entwurf übernehmen, verwerfen, Rückgängig oder Verbindung verlieren – dürfen kurz und ruhig überblenden.
- Für die Figma-Präsentation gilt: etwa 140 ms, sanftes Auslaufen, keine Feder und kein Bounce.
- Diese 140 ms sind nur die visuelle Vorgabe. Die Audio-Rampe und ein notwendiger DSP-Crossfade bleiben an die technische, messbare Parameter-Rampe gebunden und werden nicht durch Figma-Timing ersetzt.
- Ein pulsierender Punkt, ein Fortschritt oder ein Pegel darf nur laufen, wenn wirklich gemessen oder ein echter Pegel geliefert wird. Kein dekoratives Dauer-Pulsieren.

Die Zustände müssen immer wahr sein: Messen, unvollständig, frisch, veraltet, getrennt, Entwurf, Vorschau, übernommen und rückgängig sind unterschiedliche Zustände und werden nicht optisch vermischt.

## 2. Gemeinsamer Ablauf

Die Bedienung folgt über die zwei Apps und drei Oberflächen derselben Kette:

1. **Observe:** Probeeq liefert echte Mess- und Verbindungsdaten; Gen macht
   sie vergleichbar und verständlich.
2. **Advise:** Gen zeigt genau einen nächsten Befund mit Ursache, kleinstem Test und Höraufgabe.
3. **Audition:** Der Nutzer hält die Vorschau bewusst gedrückt. Beim Loslassen endet sie selbsttätig.
4. **Draft:** Gen sendet nur einen sichtbaren, noch nicht wirksamen Entwurf an das Ziel-Probeeq.
5. **Apply:** Erst eine ausdrückliche Übernahme macht daraus einen bestätigten EQ-Zustand.
6. **Compare:** Danach wird dieselbe Stelle erneut gemessen und der Nutzer entscheidet Behalten, Zurücknehmen oder Unklar.

Ein Entwurf oder eine gehaltene Vorschau ist nie Teil des bestätigten Zustands und erscheint nicht als Undo-Schritt. Erst Apply erzeugt eine Revision und darf über Undo zurückgenommen werden.

Bei Verbindungsverlust bleibt ein bestätigter Probeeq-EQ auf seinem Bus
hörbar. Die vollständige Band- und Kurvenbedienung liegt jedoch nur in Gen
und ist ohne Verbindung nicht verfügbar; die Probeeq-Rückfallfläche zeigt
nur ihre ausdrücklich vorgesehenen kompakten Handgriffe. Eine unbestätigte
Vorschau läuft sauber aus und verschwindet, neue Fernentwürfe werden nicht
vorgetäuscht.

## 3. Gen Fläche 2 – Bedienung des EQ-Graphen

### 3.1 Entscheidend: Bänder werden gezogen, nicht über ein Knob-Feld gesteuert

Die acht nummerierten Griffe auf der Kurve sind die primäre Bedienung. Sie ersetzen kein großes Knob-Feld.

| Aktion | Sichtbare Reaktion | Technische Bedeutung |
| --- | --- | --- |
| Klick auf einen Band-Griff | Griff und passende Parameterzeile werden ausgewählt; die Nummer bleibt eindeutig sichtbar. | Wählt einen von acht Band-Slots. |
| Griff horizontal ziehen | Frequenz, Zahl und Kurvenform folgen derselben Position auf logarithmischer Frequenzachse. | Ändert die Frequenz kontinuierlich. |
| Griff vertikal ziehen | Gain-Zahl und Kurve folgen sofort. | Ändert Gain kontinuierlich, begrenzt auf den erlaubten Bereich. |
| Ziehen an einer geschützten Grenze | Bewegung endet sichtbar an der Grenze; Schutzkennzeichnung bleibt aktiv. | Kein stilles Überschreiten einer geschützten Zone. |
| Griff eines ausgeschalteten Bands anklicken | Das Band wird ausgewählt, aber nicht unbemerkt eingeschaltet. | Auswahl und Aktivierung sind getrennt. |
| Ein/Aus in der Parameterzeile betätigen | Griff und Kurvenanteil erscheinen oder verschwinden klar als aktiver bzw. inaktiver Zustand. | Ändert den aktivierten Band-Slot. |

Die optisch kleinen Griffe erhalten eine unsichtbar größere Trefferfläche. Bei dicht überlappenden Griffen bleibt der aktuell ausgewählte Griff vorn. Als eindeutiger Ausweg öffnet das bestehende Feld **BAND** in der Parameterzeile eine kleine Auswahl für Band 1–8. Dafür wird keine zweite dauerhafte Kontrollreihe ergänzt.

Während eines direkten Zugs gibt es keine geglättete UI-Animation: Griff, Cursorposition, Zahlenfeld und Kurve lesen denselben sofortigen Zustand. Beim Loslassen endet der Zug, nicht aber der bestätigte Parameterwert.

### 3.2 Parameterzeile

Die vorhandene eine Parameterzeile ist die Präzisionsbedienung; sie wird nicht durch zusätzliche Knob-Reihen ersetzt.

- Kontinuierliche Werte wie Frequenz, Gain, Q, Dynamic Range, Threshold, Attack, Hold und Release lassen sich im Wertfeld vertikal ziehen und per Doppelklick direkt numerisch eingeben.
- Filtertyp, Kanalmodus und Dynamic sind Auswahlfelder mit einer kleinen, verankerten Liste.
- Nicht aktive dynamische Werte zeigen einen klar deaktivierten Zustand statt scheinbar gültiger Zahlen.
- Der Sidechain-Source-Parameter bleibt im ersten Release unsichtbar
  (Entscheid 24.08.) und steht technisch auf Default `none`. Diese
  Spezifikation erzeugt dafür weder Feld noch Untermenü.
- Änderungen an Filtertyp, Kanalmodus, Dynamic oder später Sidechain sind strukturelle Änderungen. Das UI zeigt kurz den Zustand **SWITCHING**; die Audioseite nutzt dafür den vorgesehenen blockgrenzensicheren Crossfade.

### 3.3 Dynamisches Band

Die manuell gesetzte Position eines Griffs bleibt stabil. Eine dynamische Absenkung bewegt nicht den Hauptgriff, weil dies den eingestellten Wert verfälschen würde.

Stattdessen zeigt ein dezentes zweites Signal am Band oder an der Kurve die gerade wirksame Reduktion. Dieses Signal darf sich mit dem Audio bewegen, ist aber klar als temporäre Aktivität erkennbar. Fällt das Steuersignal weg, kehrt nur diese Anzeige sanft zu neutral zurück; der Audiofluss wartet nie darauf.

### 3.4 Draft, Audition, Apply und Undo

| Zustand | Darstellung | Aktion |
| --- | --- | --- |
| Draft angekommen | Gestrichelte Zielkurve, Zielgriff und Draft Card sind sichtbar; die aktive Kurve bleibt unverändert. | Der Nutzer kann prüfen, ohne dass sich der Klang ändert. |
| Hold to audition gedrückt | Der Entwurf wird hörbar; Zielbereich und zugehörige Quelle sind klar hervorgehoben. | Die Vorschau gilt nur, solange gedrückt wird. |
| Hold losgelassen | Klang und Anzeige gehen zum bestätigten Zustand zurück. | Die Vorschau beendet sich selbst. |
| Apply | Der Entwurf wird zur aktiven Kurve, Revision und Undo-Zähler werden aktualisiert; anschließend startet die erneute Messung. | Bestätigte Änderung. |
| Reject oder Entwurf abgelaufen | Gestrichelte Kurve und Draft Card verschwinden. | Kein Undo-Schritt, weil nichts übernommen wurde. |
| Verbindung während Vorschau verloren | Vorschau klingt sicher aus; Entwurf verschwindet und der Status zeigt die Trennung. | Keine falsche Bestätigung. |

**GLOBAL** öffnet ein kleines verankertes Panel für Input Trim, Output Trim, Stereo Width und Mono Bass. Es wird keine zusätzliche permanente Zeile eingeblendet.

**BYPASS** ist ein echter Zustand: die aktive Bearbeitung wird erkennbar als umgangen markiert, nicht lediglich farblich abgeschwächt. **UNDO n** öffnet den Revisionsverlauf mit „letzten Schritt rückgängig“ und der vorhandenen Möglichkeit, auf neutral zurückzugehen.

### 3.5 Benötigte Oberflächenzustände

Für die Übergabe reichen keine einzelnen Screens. Gens zweite Oberfläche
benötigt Zustände für:

- Band: ausgewählt, nicht ausgewählt, aus, geschützt, Draft-Ziel und dynamisch aktiv.
- Verbindung: verbunden, misst, veraltet, getrennt und gepaart/nicht gepaart.
- Draft: keiner, bereit, wird gehalten, übernommen, verworfen und durch Verbindung verloren.
- Global Panel, Bypass und Undo-Verlauf.

Die Varianten demonstrieren die Reaktion. Der veröffentlichte Plugin-Code übernimmt die echte flüssige Bewegung; Figma Smart Animate oder Motion ist dafür die Vorführung, nicht die technische Implementierung.

## 4. Gen Fläche 1 – Messung, Befund und Übergabe

Fläche 1 bleibt der Ort zum Verstehen und Entscheiden. Klangänderung entsteht
nur nach ausdrücklicher Aktion und wird auf Fläche 2 bedient.

| Bereich | Erforderliche Interaktion |
| --- | --- |
| Observe | Ein Quelleneintrag wählt die Quelle und aktualisiert Detail, Heatmap und Berater-Kontext. Der Messzustand zeigt frisch, misst, unvollständig, veraltet, getrennt oder nicht verfügbar. |
| Sources | Öffnet die zentrale Quellenverwaltung: Name, Rolle, Messposition, Verbindung und Frische. Probeeq verweist bei Bedarf genau hierhin. |
| Advise | Zeigt eine Priorität nach der anderen, mit Ursache, kleinstem Test, Höraufgabe und Alternativen. Es gibt eine sichtbare Befundposition, etwa „FINDING 1 OF 3“. |
| Unsicherheit | „Mehr Daten nötig“ und „keine Änderung empfohlen“ sind gültige Ergebnisse, keine leeren Fehlerzustände. |
| Audition | Hold to audition ist eine Halte-Geste: gedrückt hören, losgelassen zurück. Quelle und betroffener Bereich werden währenddessen markiert. |
| Apply | Sendet den nicht aktiven Draft an das gewählte Probeeq. Gen zeigt anschließend **DRAFT SENT / WAITING FOR PROBEEQ**, keine vorgetäuschte Klangänderung. |
| Compare | Nach der Übernahme startet die Messung derselben Passage. Gen bietet Behalten, Zurücknehmen und Unklar; erst danach kommt der nächste Befund. |
| Width unavailable | Das Element erklärt bei Fokus oder Klick verständlich, warum die Fähigkeit fehlt und dass daraus keine musikalische Schlussfolgerung gezogen wird. |
| Undo | Öffnet echte bestätigte Revisionen; es gibt keinen Undo-Schritt für eine gehaltene Vorschau. |

Der sichtbare Messfortschritt, zum Beispiel 68 %, wird nur angezeigt, wenn die Messung ihn tatsächlich liefert. Während einer echten Messung darf der Statuspunkt langsam und zurückhaltend pulsieren; bei einer statischen oder veralteten Messung bleibt er ruhig.

## 5. Probeeq – kompakte Status- und Rückfallfläche

Probeeq bleibt kompakt und wird nicht zu einem zweiten vollständigen
EQ-Editor. Sie misst passiv und führt bei Zuschaltung Gens Fern-EQ auf ihrem
Bus aus.

- Bus, Rolle und Messposition werden angezeigt; Bearbeitung führt an den
  dafür vorgesehenen Ort in Gen.
- Link-/Verbindungszustand, Messfrische, EQ an/aus und geänderte Bänder sind
  ehrlich ablesbar.
- Nur die im kompakten Vertrag vorgesehenen direkten Handgriffe wie Bypass
  und Mix bleiben lokal; Bänder, Kurve und Präzisionswerte werden
  ausschließlich in Gen bedient.
- CONNECTED und MEASURING bleiben getrennt. Ein Puls ist nur bei echter Messung zulässig.
- Die Fläche braucht klare Varianten für frisch, unvollständig mit Grund,
  veraltet mit tatsächlichem Alter, getrennt und konflikthaft gekoppelt.
- Probeeq besitzt keinen erfundenen dB-Meter. Ein Pegelmesser braucht eine
  reale Datenquelle und eine klare Lesefrage.

Damit bleibt die Zustandswahrheit vor Ort, während die vollständige
EQ-Bedienung in Gen stattfindet.

## 6. Umsetzungsabnahme

Die drei aktuellen Rework-Bilder decken Anordnung, Hierarchie und zentrale
sichtbare Elemente ab. Sie beweisen die hier festgelegten Gesten und
Zustandswechsel noch nicht vollständig.

Vor einer Entwicklungsübergabe werden auf einer zusätzlichen Figma-Seite „Motion / Interactions“ genau diese vier Sequenzen als Varianten oder Motion-Demo gezeigt:

1. Gen Fläche 2: Band wählen, horizontal/vertikal ziehen und einen
   überlappenden Griff über BAND auswählen.
2. Gen Fläche 2: Draft → halten → loslassen → Apply → Undo sowie
   Verbindungsausfall während Preview.
3. Gen Fläche 1: Quelle wählen → Befund → Hold to audition → Draft senden →
   Vergleich.
4. Probeeq: verbunden/misst, frisch, unvollständig, veraltet, getrennt und
   EQ aktiv; Übergang zu Gen.

Erst wenn jede Sequenz von einem sichtbaren Ausgangs- zu einem eindeutigen Endzustand führt, ist die Bewegung des Designs vollständig beschrieben. Die Audio-Rampe, Schutzgrenzen, maximale Fernänderung und die Validierung bleiben dabei die technischen Verträge des Projekts.

## 7. Bewusst offen

- Eine spätere sichtbare Sidechain-Bedienung wird erst nach dem technischen
  Sidechain-/PDC-Gate gestaltet; im ersten Release ist sie entschieden
  unsichtbar.
- Die exakten, gemessenen DSP-Crossfade- und Rampenzeiten werden technisch kalibriert und versioniert; die oben genannte Dauer beschreibt nur die Darstellung in Figma.
- Weitere dB-Meter werden nicht aus Dekoration ergänzt. Sie brauchen eine eigene, reale Datenquelle und eine klare Lesefrage.
