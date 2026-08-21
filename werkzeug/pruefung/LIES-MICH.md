# Prüfung — die Blattprüfung, die nicht mehr verlorengeht

`werkzeug/PRUEFLISTE.md` hielt am 2026-08-20 fest:

> „Die Messungen wurden mit Playwright-Skripten ausgeführt; die Skripte waren
> **sitzungslokal und sind weg**, die Regeln nicht."

Das war ein benannter Verlust. Hier steht er nicht noch einmal an.

## Aufruf

```
node werkzeug/pruefung/pruefen.mjs               alle zehn Blätter
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

## Was sie NICHT prüft — und wer es tut

`pruefen.mjs` fragt nur: **lebt** das Blatt? Alles, was Inhalt ist — ehrliche
Zähler, Überlauf im Grenzfall, Verdeckung durch Overlays, der Beleg je
Baustein — gehört ins jeweilige Blatt, weil es dort bei jeder Neuzeichnung
mitlaufen muss und nicht in einem Skript, das jemand vergessen kann.

**`sondenprobe.mjs` prüft, dass das Blatt diese Messungen wirklich macht.**

```
node werkzeug/pruefung/sondenprobe.mjs alles
node werkzeug/pruefung/sondenprobe.mjs zahlen
```

Elf Proben über `sonde-messung.html` und `formfaktor.html`:

| Probe | Frage |
|---|---|
| `zahlen` | Ist jede gezeigte Zahl gemessen? Drei Beweisschritte: gegengerechnet · ändert sie sich, wenn der Inhalt sich ändert · bleibt sie gleich, wenn der **Maßstab** sich ändert |
| `waage` | Meldet jeder Baustein Höhe **und** Breite? |
| `anordnungen` | Melden alle fünf Messfälle je fünf Zahlen? |
| `gegenprobe-ueberlauf` | Wird ein künstlich eingebauter Überlauf gemeldet — und verschwindet die Warnung wieder? |
| `gegenprobe-verdeckung` | Werden **beide** Sorten (Randfarbe · Auflage) künstlich erzeugt gemeldet? |
| `ratsche` | Drei Durchgänge, dieselben Zahlen? (Ohne `listenZuruecksetzen` wandern sie.) |
| `deckel` | Ist keine Zeile flacher gedeckelt als ihr Inhalt? |
| `beleg` | Nennt jeder Baustein Handgriff und Spezifikationsstelle — und ist kein Main-Baustein übriggeblieben? |
| `grenzfall` | Ist der Grenzfall die **Vorgabe**, nicht der Sonderfall? |
| `formfaktor` | Zeigt das Active-Probe-Fenster den echten Editor-Inhalt? |
| `kachel` | Steht die Passive-Kachel im Grenzfall, nicht im Schönfall? |

**Die drei Fehler, die sie im ersten Lauf gefunden hat** — jeder in echtem
Code, keiner beim Ansehen aufgefallen:

1. `data-mess` bezeichnete zwei Dinge; die Sonde las „+12 dB" als Höhe.
2. `getBoundingClientRect()` unter `transform:scale(.52)`: alle
   Anordnungszahlen waren um 48 % zu klein. **Und die erste Fassung der Sonde
   hatte denselben Fehler** — sie verglich zwei falsche Werte und meldete
   „0 Abweichungen". Deshalb prüft sie jetzt eigens auf
   Maßstabsunabhängigkeit.
3. `.anordnung .nr` traf auch die Slot-Nummern im Spezimen: acht Nummern
   absolut gesetzt übereinander in einer Ecke.
