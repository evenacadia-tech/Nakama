# 2026-08-23 — Interview „struktur": Arbeitsfluss der EQ-Zentrale (Runde 1)

Phase `struktur` des Interview-Skills, im Anschluss an den Entscheid
`2026-08-23-gen-eq-zentrale.md`. Vier Fragen, Antworten wörtlich; die
Zusammenfassung wurde dem User vorgelesen und bestätigt.

## Frage 1 — der häufigste Handgriff auf Seite 2

> „zwischen den sonden durchschalten, verschiedene eq filter setzen, zurück
> auf den master eq schalten. das durchschalten muss smooth sein, kein
> menü , am besten scrollen mit dem mausrad wenn nichts anderes markiert
> ist per klick. jede EQ ansicht braucht direkt zugriff auf die
> vorgeschlagenen veränderungen die GEN anbietet um sie anzuwenden. jede
> funktion muss da sein wo man sie braucht. die frontseite bleibt trotzdem
> erstmal wie gehabt um nicht alles umzuwerfen. design kann danach noch
> angepasst werden."

## Frage 2 — der Weg vom Befund zum Eingriff

> „man kann die veränderung direkt anwenden indem man den button klickt
> dafür. der clou : man kann den button gedrückt halten und hört die
> veränderung, lässt man ihn los ist sie weg. klickt man einmal ist sie
> angewendet man muss nochmal klicken dass sie permanent ist. ansonsten
> schaltet sie nach 10 sekunden zurück. ansonsten button daneben springt
> direkt zum eq der sonde. dort befindet sich ei ui element mit welchem
> man das selbe machen kann. also halten, klicken klicken."

## Frage 3 — die zwei Spuren im Graph

> „egal auf welchem band man ist , kann man über die ein wählbares band
> hinzufügen. standardmäßig ist es immer nur eine spur. das ist rein
> optional. hat man 2 bänder aktiv kann man auswählen welches band man
> bearbeiten will."

Nachfrage zur Lesart („Band" = Spur/EQ-Kurve, nicht Filter-Band) —
Antwort:

> „ja ich meine den graph, eq kurve."

## Frage 4 — was die Hände schon können

> „smooth operator pro , oder pro-q sind die 2 besten. dafür kannst du
> alles studieren wie sie das handhaben und das beste kopieren. wir nehmen
> das probeeq als working design für jetzt. die GEN 1 und seite 2 werden
> nochmal komplett überarbeitet um den technischen neuen funktionen
> gerecht zu werden."

## Nachtrag bei der Bestätigung (Arbeitsweise + Auftrag)

> „ich werde parallel zum implement die beiden designs nach und nach
> designen, das ist ja ein prozess kein ‚mal eben schnell machen' ich
> brauche hierfür aber nochmal eine eigenständige spec was die UI
> technisch braucht, also die spielregeln die das design erfüllen muss um
> umsetzbar zu sein."

→ Die Spec liegt seit heute als `design/docs/ui-spielregeln-eq-zentrale.md`.

---

## Festgelegt (User-Wort, 23.08.2026)

1. Kern-Handgriff Seite 2: Sonden durchschalten → Filter setzen → zurück
   zum Master. Durchschalten ohne Menü; Mausrad wechselt die Sonde, wenn
   nichts per Klick markiert ist.
2. Die Advisor-Vorschläge von Gen sind in JEDER EQ-Ansicht direkt
   anwendbar — „jede funktion muss da sein wo man sie braucht".
3. Anwenden-Geste in drei Stufen: **Halten** = hören, loslassen = weg ·
   **1. Klick** = angewendet, fällt nach 10 Sekunden von selbst zurück ·
   **2. Klick** = permanent. Am Befund zusätzlich ein Button, der direkt
   in den EQ der betroffenen Sonde springt; dort dasselbe Element.
4. Graph: standardmäßig EINE Spur; optional eine frei wählbare zweite;
   bei zwei aktiven Spuren wählt der User das Bearbeitungsziel.
5. Referenzen für das Handling: **Smooth Operator Pro** und **Pro-Q** —
   Claude darf ihr Handling studieren und das Beste übernehmen.
6. Probeeq-Figma = Working Design für jetzt; Seite 1 bleibt vorerst wie
   gehabt; später werden Gen Seite 1 UND Seite 2 komplett überarbeitet.
   Das Design entsteht parallel zur Implementierung, als Prozess.

## Angenommen (unbestätigt — Claude-Lesart, risikoarm)

- Mausrad-Konvention: ein markiertes Element besitzt das Rad (Band-Gain/Q
  wie in Pro-Q üblich); nur im „nichts markiert"-Zustand schaltet es Sonden.
- „das beste kopieren" meint Bedienlogik und Handling, nicht Optik — die
  visuelle Identität bleibt dein Figma (Designvertrag 21.08.).
- Die 10 Sekunden sind ein Startwert (änderbar wie alle Produktzahlen).
- Beim Speichern und Rendern zählt nur der bestätigte Stand; gehaltene
  Previews und nicht bestätigte Erst-Klicks werden nie persistiert.

## Offen (nächste Runden / Kennungen)

- Rückfallfläche der Sonde: Mindestinhalt ohne Gen (Runde 2).
- Master-EQ: was „vollwertig wie andere am Markt" an Umfang heißt (Runde 2).
- Überladen-Grenze auf Seite 2 (Runde 2).
- Reihenfolge der Sonden beim Durchschalten (Runde 2).
- U14 (±3-dB-Cap) · U5/NAK-33 (Sidechain sichtbar) · NAK-65 (Größe
  Seite 2, Figma-Flächen) · Undo-Form (U2.8-Folgefrage).
