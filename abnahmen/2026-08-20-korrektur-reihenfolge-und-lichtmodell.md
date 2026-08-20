# 2026-08-20 — Zwei Korrekturen: Reihenfolge und Lichtmodell

## Das wörtliche Urteil

> „das ist wieder nicht richtig, es ist nie entweder oder. die Beleuchtung kann
> zum teil von UI elementen, passiv, aktiv kommen. sie ist abhängig vom
> material matt, metallisch, plastisch oder glänzend. alles spielt da mit rein.
> warum sind wir schon wieder bei punkt wenn wir nichtmal mit 2 richtig
> angefangen haben"

Dazu das Bild von Stufe 2 aus `assets/GUI-Tips.pdf`.

## Korrektur 1 — die Reihenfolge

`GUI-Tips.pdf` Stufe 2 „UX LAYOUT" hat drei Schritte, von denen keiner
stattgefunden hat:

1. **Wireframe UX** — Anordnung, Bedienelemente, Schriftwahl. Keine Optik.
2. **Early Visual Layer** — Kontrast, Gruppierung, erste Stilhinweise.
   Ausdrücklich: *„without yet finalizing the full look"*.
3. **UI Design Stage** — Stil und Feinproportionen.

Der Arbeitsplan dieses Projekts sagt dasselbe: **Phase 1 = Wireframes in
Graustufen, Main zuerst** — „dort steckt die eigentliche
Informationsarchitektur; der teuerste Fehler wird hier billig verhindert".

Nach der abgenommenen Welt (Stufe 1, „THE FEELING") bin ich direkt zu
Material und Licht gesprungen — das ist Stufe 5 und 6. Zwischen beiden fehlt
die ganze Stufe 2.

**Was damit festliegt:** Es wird kein Licht-, Material- oder Farbwert mehr
entschieden, bevor die Anordnung von Main abgenommen ist. `werkzeug/licht.html`
ist bis dahin stillgelegt.

## Korrektur 2 — das Lichtmodell

Mein Werkzeug hat die Herkunft des Lichts als vier sich ausschließende
Möglichkeiten gebaut. Das ist falsch. Richtig ist:

1. **Licht ist additiv.** Mehrere Quellen wirken gleichzeitig; die Frage ist
   nie „woher", sondern immer „wieviel von jeder".
2. **UI-Elemente sind selbst Lichtquellen** — passiv (sie reflektieren und
   werfen Licht zurück in den Raum) und aktiv (sie leuchten selbst, etwa ein
   Pegel, ein Leuchtring, eine Statusanzeige).
3. **Material entscheidet die Antwort auf Licht.** Matt, metallisch, plastisch
   oder glänzend reagieren völlig verschieden auf dieselbe Beleuchtung —
   Streuung, Glanzlichtform, Kantenverhalten. Material und Licht sind nicht
   zwei Fragen, sondern eine.

**Was damit festliegt:** Wenn Stufe 5 an der Reihe ist, wird Licht als Satz
gleichzeitig wirkender Anteile gebaut (Grundlicht, aktive Elemente, passive
Rückwürfe) und jede Fläche bekommt eine Materialantwort, die diese Anteile
verschieden verarbeitet. Keine Auswahlknöpfe mehr.

## Was ausdrücklich offen bleibt

- Die gesamte Stufe 2, beginnend mit dem Wireframe von **Main**.
- Alle Licht- und Materialwerte.
