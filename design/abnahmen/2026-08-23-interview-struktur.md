# 2026-08-23 — Interview „struktur": Arbeitsfluss der EQ-Zentrale (Runden 1–2)

Phase `struktur` des Interview-Skills, im Anschluss an den Entscheid
`2026-08-23-gen-eq-zentrale.md`. Antworten wörtlich; jede Runde wurde als
Zusammenfassung vorgelesen und vom User bestätigt (R1 mit Korrektur der
Lesart, R2 mit „ok").

## Runde 1 — Arbeitsfluss (Fragen 1–4)

### Frage 1 — der häufigste Handgriff auf Seite 2

> „zwischen den sonden durchschalten, verschiedene eq filter setzen, zurück
> auf den master eq schalten. das durchschalten muss smooth sein, kein
> menü , am besten scrollen mit dem mausrad wenn nichts anderes markiert
> ist per klick. jede EQ ansicht braucht direkt zugriff auf die
> vorgeschlagenen veränderungen die GEN anbietet um sie anzuwenden. jede
> funktion muss da sein wo man sie braucht. die frontseite bleibt trotzdem
> erstmal wie gehabt um nicht alles umzuwerfen. design kann danach noch
> angepasst werden."

### Frage 2 — der Weg vom Befund zum Eingriff

> „man kann die veränderung direkt anwenden indem man den button klickt
> dafür. der clou : man kann den button gedrückt halten und hört die
> veränderung, lässt man ihn los ist sie weg. klickt man einmal ist sie
> angewendet man muss nochmal klicken dass sie permanent ist. ansonsten
> schaltet sie nach 10 sekunden zurück. ansonsten button daneben springt
> direkt zum eq der sonde. dort befindet sich ei ui element mit welchem
> man das selbe machen kann. also halten, klicken klicken."

### Frage 3 — die zwei Spuren im Graph

> „egal auf welchem band man ist , kann man über die ein wählbares band
> hinzufügen. standardmäßig ist es immer nur eine spur. das ist rein
> optional. hat man 2 bänder aktiv kann man auswählen welches band man
> bearbeiten will."

Nachfrage zur Lesart („Band" = Spur/EQ-Kurve, nicht Filter-Band) —
Antwort:

> „ja ich meine den graph, eq kurve."

### Frage 4 — was die Hände schon können

> „smooth operator pro , oder pro-q sind die 2 besten. dafür kannst du
> alles studieren wie sie das handhaben und das beste kopieren. wir nehmen
> das probeeq als working design für jetzt. die GEN 1 und seite 2 werden
> nochmal komplett überarbeitet um den technischen neuen funktionen
> gerecht zu werden."

### Nachtrag bei der Bestätigung (Arbeitsweise + Auftrag)

> „ich werde parallel zum implement die beiden designs nach und nach
> designen, das ist ja ein prozess kein ‚mal eben schnell machen' ich
> brauche hierfür aber nochmal eine eigenständige spec was die UI
> technisch braucht, also die spielregeln die das design erfüllen muss um
> umsetzbar zu sein."

→ Die Spec liegt seit heute als `design/docs/ui-spielregeln-eq-zentrale.md`.

## Runde 2 — Grenzfälle (Fragen 5–8)

### Frage 5 — die Sonde ohne Gen (Rückfallfläche)

> „es geht hier in erster linie um performance, das plugin muss so wenig
> wie möglich auf die leistung schlagen, da sie ja in hoher anzahl
> angewendet wird. wenn GEN abgestürzt ist steht da was der user wissen
> muss : disconnected / Connected . ich kann das nicht ausführlich
> beantworten weil ich nicht im kopf habe was GEN anzeigt bezüglich des
> alten Suna konzeptes. es muss überlegt werden was der user veranlassen
> könnte den probeeq zu öffnen statt gleich Gen? Connected /
> disconnected, EQ Mode grüner oder roter punkt und welche eq werte
> aktuell verändert sind. bypass, und natürlich mix also wieviel sie auf
> den mix anwendet des eq (das gehört aber auch noch in den Gen ganz
> wichtig)"

**Befund dazu (Claude, am Vertrag gemessen):** Der eingefrorene
Parameterbestand `eq-copilot/schemas/state/nakama-parameter-v1.json`
kennt global nur Bypass · Input-Trim · Output-Trim · Width · Mono-Bass —
**einen Mix/Dry-Wet gibt es noch nicht.** Das User-Wort macht ihn zur
Anforderung; Umsetzung als versionierte Erweiterung (neue ID) im
Umschnitt NAK-64, Punkt 4.

### Frage 6 — was „vollwertig" beim Master-EQ heißt

> „das übersteigt mein technik verständnis, er muss das rad nicht neu
> erfinden, aber vorbild ist auf jedenfall besagte eq. dynamisch ist schon
> drin, alle gängigen filter die es gibt highpass und so weiter. der rest
> übersteigt mein verständniss was umsetzbar ist und was nicht. smooth
> operator hat resonanzpeak als fokus zum beispiel."

### Frage 7 — die Überladen-Grenze auf Seite 2

> „das müssen wir gesondert button für button machen. dazu musst du das
> aktuelle probeeq design anschauen, analysieren was fehlt und was da
> ist. dann klären wir das ganz genau. so halb halb bringt das nix."

### Frage 8 — Reihenfolge beim Durchschalten

> „mixer reihenfolge, master erhält ui element"

Zusammenfassung beider Befunde und aller fünf Punkte vorgelesen —
Antwort: **„ok"**.

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
7. **Rückfallfläche der Sonde: Performance zuerst** (hohe Instanzzahl,
   „so wenig wie möglich auf die leistung schlagen"). Inhalt:
   CONNECTED/DISCONNECTED · EQ-Mode-Punkt grün/rot · welche EQ-Werte
   aktuell verändert sind · Bypass · Mix.
8. **Mix (Dry/Wet je Sonde) ist beschlossen** — „und natürlich mix also
   wieviel sie auf den mix anwendet des eq"; er gehört zusätzlich „ganz
   wichtig" in den Gen.
9. **Master-EQ: das Rad nicht neu erfinden.** Vorbild Pro-Q und Smooth
   Operator Pro; dynamische Bänder und alle gängigen Filtertypen; die
   Machbarkeits-Bewertung ist Sache der Technik.
10. **Die Überladen-Grenze wird Button für Button geklärt** — am
    aktuellen Probeeq-Design, „so halb halb bringt das nix".
11. **Durchschalt-Reihenfolge = FL-Mixer-Reihenfolge; der Master ist
    keine Rad-Position, sondern erhält ein eigenes UI-Element.**

## Angenommen (unbestätigt — Claude-Lesart, risikoarm)

- Mausrad-Konvention: ein markiertes Element besitzt das Rad (Band-Gain/Q
  wie in Pro-Q üblich); nur im „nichts markiert"-Zustand schaltet es Sonden.
- „das beste kopieren" meint Bedienlogik und Handling, nicht Optik — die
  visuelle Identität bleibt dein Figma (Designvertrag 21.08.).
- Die 10 Sekunden sind ein Startwert (änderbar wie alle Produktzahlen).
- Beim Speichern und Rendern zählt nur der bestätigte Stand; gehaltene
  Previews und nicht bestätigte Erst-Klicks werden nie persistiert.
- Rückfallfläche: **Bypass und Mix bedienbar**, die veränderten EQ-Werte
  nur **Anzeige** (bearbeitet wird in Gen).
- Mix wird eine neue versionierte Parameter-ID (Technik, NAK-64 Punkt 4).
- Smooth Operators Resonanzpeak-Fokus = spätere Ausbaustufe, nicht der
  erste Wurf.

## Offen (nächste Runden / Kennungen)

- **Runde 3: Button-für-Button-Durchgang** am Probeeq-Working-Design
  (Inventur: was ist da / was fehlt / was ist unklar), daraus die
  Überladen-Grenze für Seite 2.
- Rückfallflächen-Zuschnitt final erst, wenn der User sieht, was Gen je
  Sonde zeigt (sein Vorbehalt: „ich habe nicht im kopf was GEN anzeigt
  bezüglich des alten Suna konzeptes"). Leitfrage dafür: „was könnte den
  User veranlassen den probeeq zu öffnen statt gleich Gen?"
- U14 (±3-dB-Cap) · U5/NAK-33 (Sidechain sichtbar) · NAK-65 (Größe
  Seite 2, Figma-Flächen) · Undo-Form (U2.8-Folgefrage).
