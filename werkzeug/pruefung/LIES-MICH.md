# Prüfung — die Blattprüfung, die nicht mehr verlorengeht

`werkzeug/PRUEFLISTE.md` hielt am 2026-08-20 fest:

> „Die Messungen wurden mit Playwright-Skripten ausgeführt; die Skripte waren
> **sitzungslokal und sind weg**, die Regeln nicht."

Das war ein benannter Verlust. Hier steht er nicht noch einmal an.

## Aufruf

```
node werkzeug/pruefung/pruefen.mjs               alle neun Blätter
node werkzeug/pruefung/pruefen.mjs zustaende     nur eines
node werkzeug/pruefung/pruefen.mjs --gegenprobe  beweist, dass sie scheitern kann
```

Exit 1, sobald ein Blatt tot ist. Screenshots landen in `bilder/` (nicht
versioniert — sie sind Beleg für den Moment, nicht Projektwissen).

## Was sie prüft

| | Regel |
|---|---|
| **JS-Fehler** — `pageerror` und `console.error` | PRUEFLISTE 4.5: ein Syntaxfehler macht das Blatt beim Doppelklick tonlos tot |
| **Leer / stumm** — zu wenige Knoten, zu wenig Text | Ein Blatt, das nichts zeichnet, belegt nichts |
| **Standband** — sagt das Blatt, was es ist? | PRUEFLISTE 5.1: entschieden / eingefroren / Arbeitsannahme / stillgelegt |

Sie misst **nach** dem zweiten Frame, weil auf dem ersten die Schriftmaße noch
nicht stehen (PRUEFLISTE 2.4).

## Warum die Gegenprobe dazugehört

PRUEFLISTE 7.1: **ein Gate, das nicht scheitern kann, beweist nichts.**
`--gegenprobe` baut drei künstlich kaputte Blätter — Syntaxfehler, leeres
Blatt, fehlendes Standband — und verlangt, dass alle drei gemeldet werden.

Das war kein Ritual: beim ersten Lauf hat die Gegenprobe **einen echten Fehler
in der Prüfung selbst** gefunden. Der dritte Fall wurde per Textmuster mit `\n`
am Ende erzeugt, die Blätter haben aber CRLF-Zeilenenden — das Muster griff ins
Leere, der Testfall entstand nie, und die Prüfung meldete zufrieden „kein
Fehler". Genau die Blindheit, die sie finden soll.

## Was sie braucht

`playwright-core` und eine Chromium-Installation. Beides wird gesucht, nicht
behauptet: erst im Projekt, dann in den üblichen `ms-playwright`-Ordnern, zuletzt
in einer System-Chrome-Installation. Findet sie nichts, sagt sie, wo sie gesucht
hat. **Kein Browser-Download, keine Build-Kette** — das Projekt bleibt
doppelklickbar.

## Was sie NICHT prüft

Alles, was Inhalt ist: ehrliche Zähler, Überlauf im Grenzfall, der
Dauerhaft-Vertrag, die Verdeckung durch Overlays. Diese Messungen gehören ins
jeweilige Blatt, weil sie dort bei jeder Neuzeichnung mitlaufen müssen — nicht
in ein Skript, das jemand vergessen kann. Die **Verdeckungsprüfung** aus
PRUEFLISTE 2.3 existiert bis heute in keinem Blatt; siehe `docs/sondenplan.md`.
