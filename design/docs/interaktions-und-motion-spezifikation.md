# Interaktions- und Motion-Spezifikation

> **Status:** Arbeitsvorgabe, festgehalten am 23.08.2026 auf ausdrücklichen Auftrag.  
> **Geltung:** Ergänzt die bestehenden Figma-Stände. Sie verändert weder deren Layout noch deren visuelle Sprache, sondern legt fest, wie die sichtbaren Elemente reagieren müssen.  
> **Grundlage:** [Oberflächen-Spezifikation](oberflaechen-spezifikation.md), [Arbeitsplan](arbeitsplan.md), [Figma-Depot](../assets/figma/LIES-MICH.md) und der technische Entwurf [FL-Nakama-Sonden-Design](../../docs/FL-Nakama-Sonden-Design-Entwurf.md).

## 1. Grundsatz

Die drei aktuellen Figma-Ansichten bleiben die visuelle Referenz:

- [Gen](../assets/figma/2026-08-22-gen.png)
- [Probeeq](../assets/figma/2026-08-22-probeeq.png)
- [Suna](../assets/figma/2026-08-22-suna.png)

Figma zeigt Material, Anordnung und Zustände. Im fertigen Plugin werden der EQ-Graph, die Bänder, die Auswahl, Statusanzeigen, Messwerte und alle Bewegungen aus dem tatsächlichen Zustand gezeichnet. Eine exportierte, statische SVG-Kurve darf nicht als scheinbar lebendiger EQ-Graph verwendet werden.

Jede sichtbare Bewegung muss einen echten Grund haben:

- Direkte Mausbewegung folgt ohne Verzögerung. Kein Nachziehen, kein Feder-Effekt, kein Springen.
- Wechsel, die nicht direkt von der Maus kommen – etwa Entwurf übernehmen, verwerfen, Rückgängig oder Verbindung verlieren – dürfen kurz und ruhig überblenden.
- Für die Figma-Präsentation gilt: etwa 140 ms, sanftes Auslaufen, keine Feder und kein Bounce.
- Diese 140 ms sind nur die visuelle Vorgabe. Die Audio-Rampe und ein notwendiger DSP-Crossfade bleiben an die technische, messbare Parameter-Rampe gebunden und werden nicht durch Figma-Timing ersetzt.
- Ein pulsierender Punkt, ein Fortschritt oder ein Pegel darf nur laufen, wenn wirklich gemessen oder ein echter Pegel geliefert wird. Kein dekoratives Dauer-Pulsieren.

Die Zustände müssen immer wahr sein: Messen, unvollständig, frisch, veraltet, getrennt, Entwurf, Vorschau, übernommen und rückgängig sind unterschiedliche Zustände und werden nicht optisch vermischt.

## 2. Gemeinsamer Ablauf

Die Bedienung folgt für alle drei Plugins derselben Kette:

1. **Observe:** Gen oder Suna liefert echte Mess- und Verbindungsdaten.
2. **Advise:** Gen zeigt genau einen nächsten Befund mit Ursache, kleinstem Test und Höraufgabe.
3. **Audition:** Der Nutzer hält die Vorschau bewusst gedrückt. Beim Loslassen endet sie selbsttätig.
4. **Draft:** Gen sendet nur einen sichtbaren, noch nicht wirksamen Entwurf an das Ziel-Probeeq.
5. **Apply:** Erst eine ausdrückliche Übernahme macht daraus einen bestätigten EQ-Zustand.
6. **Compare:** Danach wird dieselbe Stelle erneut gemessen und der Nutzer entscheidet Behalten, Zurücknehmen oder Unklar.

Ein Entwurf oder eine gehaltene Vorschau ist nie Teil des bestätigten Zustands und erscheint nicht als Undo-Schritt. Erst Apply erzeugt eine Revision und darf über Undo zurückgenommen werden.

Bei Verbindungsverlust gilt: ein bestätigter lokaler EQ bleibt hörbar und bedienbar; eine unbestätigte Vorschau läuft sauber aus und verschwindet; neue Fernentwürfe werden nicht vorgetäuscht.

## 3. Probeeq – Bedienung des EQ-Graphen

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
- Der Sidechain-Source-Parameter bleibt offen. Er wird erst sichtbar platziert, wenn NAK-33 entschieden ist; diese Spezifikation erfindet keinen Platz dafür.
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

### 3.5 Benötigte Figma-Zustände

Für die Übergabe reichen keine einzelnen Screens. Probeeq benötigt Komponenten-Varianten für:

- Band: ausgewählt, nicht ausgewählt, aus, geschützt, Draft-Ziel und dynamisch aktiv.
- Verbindung: verbunden, misst, veraltet, getrennt und gepaart/nicht gepaart.
- Draft: keiner, bereit, wird gehalten, übernommen, verworfen und durch Verbindung verloren.
- Global Panel, Bypass und Undo-Verlauf.

Die Varianten demonstrieren die Reaktion. Der veröffentlichte Plugin-Code übernimmt die echte flüssige Bewegung; Figma Smart Animate oder Motion ist dafür die Vorführung, nicht die technische Implementierung.

## 4. Gen – Messung, Befund und Übergabe

Gen bleibt die Oberfläche zum Verstehen und Entscheiden, nicht das Plugin, das heimlich Audio verändert.

| Bereich | Erforderliche Interaktion |
| --- | --- |
| Observe | Ein Quelleneintrag wählt die Quelle und aktualisiert Detail, Heatmap und Berater-Kontext. Der Messzustand zeigt frisch, misst, unvollständig, veraltet, getrennt oder nicht verfügbar. |
| Sources | Öffnet die zentrale Quellenverwaltung: Name, Rolle, Messposition, Verbindung und Frische. Suna führt genau hierhin. |
| Advise | Zeigt eine Priorität nach der anderen, mit Ursache, kleinstem Test, Höraufgabe und Alternativen. Es gibt eine sichtbare Befundposition, etwa „FINDING 1 OF 3“. |
| Unsicherheit | „Mehr Daten nötig“ und „keine Änderung empfohlen“ sind gültige Ergebnisse, keine leeren Fehlerzustände. |
| Audition | Hold to audition ist eine Halte-Geste: gedrückt hören, losgelassen zurück. Quelle und betroffener Bereich werden währenddessen markiert. |
| Apply | Sendet den nicht aktiven Draft an das gewählte Probeeq. Gen zeigt anschließend **DRAFT SENT / WAITING FOR PROBEEQ**, keine vorgetäuschte Klangänderung. |
| Compare | Nach der Übernahme startet die Messung derselben Passage. Gen bietet Behalten, Zurücknehmen und Unklar; erst danach kommt der nächste Befund. |
| Width unavailable | Das Element erklärt bei Fokus oder Klick verständlich, warum die Fähigkeit fehlt und dass daraus keine musikalische Schlussfolgerung gezogen wird. |
| Undo | Öffnet echte bestätigte Revisionen; es gibt keinen Undo-Schritt für eine gehaltene Vorschau. |

Der sichtbare Messfortschritt, zum Beispiel 68 %, wird nur angezeigt, wenn die Messung ihn tatsächlich liefert. Während einer echten Messung darf der Statuspunkt langsam und zurückhaltend pulsieren; bei einer statischen oder veralteten Messung bleibt er ruhig.

## 5. Suna – kompakte Status-Karte

Suna bleibt eine kleine Status-Karte und wird nicht zu einem zweiten Editor.

- Bus, Rolle und Messposition werden auf der Karte angezeigt, aber nicht in der engen Karte bearbeitet.
- Ein Klick auf die Karte oder den eindeutig ausgewählten Bereich führt in Gen zu **SOURCES**, dem einzigen Ort für Benennung, Rolle, Position und Verbindung.
- CONNECTED und MEASURING bleiben getrennt. Ein Puls ist nur bei echter Messung zulässig.
- Zusätzlich braucht die Karte klare Varianten für frisch, unvollständig mit Grund, veraltet mit tatsächlichem Alter, getrennt und doppelt/konflikthaft gekoppelt.
- Suna besitzt keinen erfundenen dB-Meter. Ein echter Pegelmesser gehört nur in eine spätere Detailansicht, wenn dafür eine technische Anforderung und Datenquelle vorliegen.

Damit löst Suna die Zustandswahrheit vor Ort, während die eigentliche Bedienung übersichtlich in Gen stattfindet.

## 6. Umsetzungsabnahme

Die vorhandenen Figma-Bilder decken bereits Anordnung, Hierarchie und die zentralen sichtbaren Elemente ab. Sie decken die hier festgelegten Gesten und Zustandswechsel noch nicht vollständig ab.

Vor einer Entwicklungsübergabe werden auf einer zusätzlichen Figma-Seite „Motion / Interactions“ genau diese vier Sequenzen als Varianten oder Motion-Demo gezeigt:

1. Probeeq: Band wählen, horizontal/vertikal ziehen und einen überlappenden Griff über BAND auswählen.
2. Probeeq: Draft → halten → loslassen → Apply → Undo sowie Verbindungsausfall während Preview.
3. Gen: Quelle wählen → Befund → Hold to audition → Draft senden → Vergleich.
4. Suna: verbunden/misst, frisch, unvollständig, veraltet und getrennt; Übergang zu Gen Sources.

Erst wenn jede Sequenz von einem sichtbaren Ausgangs- zu einem eindeutigen Endzustand führt, ist die Bewegung des Designs vollständig beschrieben. Die Audio-Rampe, Schutzgrenzen, maximale Fernänderung und die Validierung bleiben dabei die technischen Verträge des Projekts.

## 7. Bewusst offen

- Die sichtbare Platzierung des technischen Sidechain-Source-Parameters wartet auf die Entscheidung zu NAK-33.
- Die exakten, gemessenen DSP-Crossfade- und Rampenzeiten werden technisch kalibriert und versioniert; die oben genannte Dauer beschreibt nur die Darstellung in Figma.
- Weitere dB-Meter werden nicht aus Dekoration ergänzt. Sie brauchen eine eigene, reale Datenquelle und eine klare Lesefrage.
