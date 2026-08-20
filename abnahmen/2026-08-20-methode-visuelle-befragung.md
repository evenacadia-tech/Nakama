# 2026-08-20 — Befragung ist visuell, und zwar auf Weltebene

## Das wörtliche Urteil

> „wir machen es anders, du musst mich visuell befragen. bau ein tool ,
> technisch absolut minimal, dort stellst du mich immer visuell vor die wahl.
> 3 bilder zu auswahl, 3 animationen, ganz egal was dir etwas bringt um zu
> verstehen welchen stil ich ganz exakt möchte."

> „du hast nicht verstanden was ich mit der visuellen befragung bezwecken
> wolltest und hast mir stattdessen ein zu 90% fertiges design hingestellt und
> über die letzten 10 % konnte ich dann zwischen runden oder eckigen Kanten
> wählen."

Aus dem ersten Werkzeug, Runde „Sicherheitsstufe", Feld „alle falsch":

> „viel zu viel text generell. mehr symbolik"

## Was damit festliegt

1. **Gefragt wird visuell, nicht verbal.** Keine Interviewfragen nach Gefühlen
   oder Adjektiven mehr — Bilder zur Wahl. Erkennen ist präzise, Beschreiben
   nicht.
2. **Zuerst die Welt, dann die Details.** Die erste Frage ist nie ein Parameter
   innerhalb einer Welt, die Claude schon gewählt hat, sondern die Welt selbst:
   Entity Type, Material, Licht, Temperatur — die Achsen aus `GUI-Tips.pdf`
   (Voger Design, „Quality GUI Checklist", Stufe 1 „THE FEELING") in der Truhe.
   Erst wenn die Welt steht, sind Radius, Dichte und Farbwert überhaupt
   sinnvolle Fragen.
3. **Weniger Text, mehr Symbolik.** Gilt für alle drei Oberflächen, nicht nur
   für die Befundkarte, an der es aufgefallen ist. Zustand über Licht, Bogen,
   Form und Zahl — nicht über Sätze.

## Die Truhe und was sie sagt

Der User hat am 2026-08-20 abgelegt:

- **`301.zip`** — „Neumorphic Dark Kobs", 13 Reglervarianten (Figma). Fast
  schwarze Fläche mit feinem Korn, weiches Licht von links oben, Bedienteile
  treten aus der Fläche heraus, LED-Punktkränze in Giftgrün, gepunktete Skalen.
  Die Kornstruktur liegt jetzt als nahtlose Kachel in `werkzeug/textur/korn.png`
  und wird verwendet, nicht nachgebaut.
- **`110.zip`** — ein Nest-artiger Ring (Sketch + PSD): EIN schweres Objekt auf
  hellem Grund, das in den Raum leuchtet; der ganze Zustand ist ein farbiger
  Bogen und eine große dünne Zahl. Beschriftung fast keine.
- **`297.zip`** — „Campfire", dunkles Social-UI-Kit (Figma, Roboto).
- **`GUI-Tips.pdf`** — Voger Design, „Quality GUI Checklist": Feeling → UX-Layout
  → Modulares Raster → Typografie → Licht (Soft/Hard/Inverted/Colourful) →
  Material (Plastik/Metall/Glas/Papier, „IT NEED TO BE TOUCHABLE").
- **`nakama-stilprofil.json`** — das Ergebnis des ersten Werkzeugs.

## Vorläufig, weil in der falschen Welt erhoben

Die Antworten aus `nakama-stilprofil.json` sind **nicht** bindend: sie wurden
innerhalb einer Welt gegeben, die Claude vorentschieden hatte. Zwei Signale
haben immerhin die Gegenprobe stabil überstanden und passen zur Truhe —
**Material mit Tiefe statt flach** und **kleine Bedeutungspalette statt
farbcodierter Bereiche**. Ebenfalls notiert, aber weltabhängig: technische
DIN-Schrift, kühl-blauer Akzent (H 205 / S 44 / L 53), Kantenradius 7,
Flächenkurve, gedämpfter Meterlauf, federnde Bewegung (320 ms mit Überschwingen).

## Was ausdrücklich offen bleibt

- **Die Welt selbst.** Sechs Kandidaten liegen in `werkzeug/weltenwahl.html`
  nebeneinander: dunkles Gerät · helles Objekt · Metall · Glas und Licht ·
  Papier und Tinte · kein Gehäuse. Noch nichts entschieden.
- Alles unterhalb der Weltebene: Anordnung, Maße, Farbwerte, Typo, Bewegung.
- Ob eine der sechs überhaupt trifft, oder ob eine siebte fehlt.
