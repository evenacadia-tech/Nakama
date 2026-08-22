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
`--gegenprobe` baut vier künstlich kaputte Blätter — Syntaxfehler, leeres
Blatt, fehlendes Standband, **verdecktes** Standband — und verlangt, dass alle
vier gemeldet werden.

Der vierte kam am 2026-08-21 dazu, zusammen mit dem Sichtbarkeits-Riegel: bis
dahin prüfte `pruefen.mjs` nur, ob das Standband **existiert**. In
`formfaktor.html` lag es vollständig unter dem fixen Warnband und dem
Messschirm — 920 Stichpunkte, **0 sichtbar** — und das Blatt galt als heil.
Ein Riegel ohne eigenen Gegenprobe-Fall ist ein unbewiesener Riegel.

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

Fünfzehn Proben über `sonde-messung.html` und `formfaktor.html`:

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
| `grenzfall` | **Zwei** Grenzfälle, die in entgegengesetzte Richtungen zeigen: ist der PLATZ-Grenzfall die Vorgabe (acht Slots, Dynamik an, Schutzbereich getroffen, Ehrlichkeitsschalter neutral)? Und greift der EHRLICHKEITS-Grenzfall wirklich (bei Bypass steht kein Bandparameter mehr da)? Bis 2026-08-21 war das Tor im Blatt ein hart geschriebenes `z(true, …)` |
| `zustaende` | Alle **sechzehn** Kombinationen der Ehrlichkeitsschalter durchgeschaltet: ist bei Bypass und ohne Main wirklich jeder Griff tot, und zeigt jeder wirkungslose Wert „—“? Mit Gegenprobe nach unten — im Schönfall muss etwas leben, sonst prüft die Schleife ins Leere |
| `formfaktor` | Zeigt das Active-Probe-Fenster den echten Editor-Inhalt? |
| `export` | Trägt das exportierte JSON die **Messung** — Überlauf, Richtung, Kurven-Zeichenraum — oder nur Name und Maße? Verglichen gegen den lebenden DOM-Zustand, nicht gegen die Felder, aus denen der Export gebaut ist |
| `annahmen` | Ist jede frei gesetzte Zahl (Q 24, −60 dB, 300/500/3000 ms, 15,4 px) im Blatt **sichtbar** als Testannahme benannt, und wird die eine Zahl aus der Spezifikation genannt? Mit Gegenprobe |
| `maschine` | Ändert **jeder lebende Griff** etwas Sichtbares? Fährt offen→verworfen→offen→angewandt→offen, neutralisiert und nimmt zurück, klickt den Verlauf leer und hält Audition mit echtem `pointerdown`/`pointerup` |
| `kachel` | Steht die Passive-Kachel im Grenzfall — und in **welcher Richtung** läuft sie über? Verlangt vertikal > 1 px und horizontal ≤ 1 px, weil zwei Kommentare im Code das Gegenteil behaupteten |

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
