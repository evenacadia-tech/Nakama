# 2026-08-20 — Bedienebenen: der Streifen führt Buch, das Blatt zeigt das Vorhören

**Richtungsabnahme.** Beurteilt wurden drei bedienbare Blätter auf 760×430,
mit echtem Halten und Loslassen. Die Kombination selbst war zu diesem
Zeitpunkt noch nicht gebaut — sie ist die Übersetzung des Urteils und wurde
danach vorgelegt.

## Das wörtliche Urteil

> „es ist 1 und 2 eine kombination daraus."

Dazu, zu einem Ausschnitt der Karte:

> „die übergroße zeile wirkt nichti richtig"

Und auf Nachfrage, weil ich die erste Aussage falsch übersetzt hatte:

> „ich wollte die markierte zeile + die app umrandung"

## ⚠ Meine erste Übersetzung war falsch

Ich hatte „1 und 2" als *Streifen + Blattrahmen* gelesen und genau das
gebaut. Gemeint waren die **markierte Zeile** (aus „der Zustand klebt am
Ort") und die **Umrandung der App** (aus „das Blatt ist der Zustand").
Korrigiert; die falsche Fassung steht nur noch in der Git-Geschichte.

## Was damit festliegt

Das Zeichen für **Vorhören** besteht aus zwei Teilen, und beide beantworten
verschiedene Fragen:

| Frage | Ort | Zeichen |
|---|---|---|
| *Läuft gerade etwas?* | die **ganze App** | Umrandung, solange gehalten |
| *Wo wirkt es?* | die **betroffene Zeile** in der Karte | Zeile markiert |

Die Karte wird dabei **nicht** gedämpft — die markierte Zeile muss lesbar
bleiben, und ein Dämpfen würde gegen die Markierung arbeiten.

## Zwei Entscheidungen, die ich beim Bauen getroffen habe

Beide sind mechanisch, nicht gestalterisch, und stehen hier, damit sie
nachprüfbar sind:

1. **Nichts wird gedämpft.** Die reine Blatt-Variante dämpfte Streifen und
   Karte. Mit einer markierten Zeile *in* der Karte wäre das falsch: das
   Dämpfen arbeitet gegen die Markierung, und der Streifen trägt die
   Auskunft, die man in dem Moment braucht. Es bleibt bei Umrandung plus
   Markierung.

   **Offen und von mir nicht entschieden:** was der Streifen währenddessen
   tut. Die Spezifikation verlangt „vier Bedienebenen, sichtbar getrennt" und
   „Zustände, die immer ablesbar sein müssen" — der Streifen bleibt also
   stehen und führt weiter Buch. Ob sein `AUDITION`-Reiter dabei zusätzlich
   aufleuchten soll oder ob Umrandung und Markierung genügen, ist die eine
   offene Frage dieser Runde.
2. **Das Blatt spricht ohne Worte.** Das Band „AUDITIONING — RELEASE TO STOP"
   ist entfallen. Gemessen: es verdeckte 6 px des Assistenten-Kastens — eine
   Verdeckung, die keine Überlaufmessung sieht, weil nichts überläuft,
   sondern nur etwas darüberliegt. Und es war in der Kombination das
   **fünfte** Zeichen für denselben Zustand (Reiter, Chip, Rahmen, gedämpfte
   Karte, Band). Von Restorff: ein Akzent, nicht fünf. Die Worte stehen im
   Streifen, das Blatt trägt das Zeichen.

## Der Fehler mit der übergroßen Zeile

Kein Entwurf, sondern ein Fehler im Werkzeug. In `heatmap()` hieß **zweierlei
gleich**:

- `fokus` als **Disclosure-Mechanik** (eine Zeile wird vierfach hoch, der Rest
  schrumpft auf 5-px-Streifen — das ist Focus+Context),
- `fokus` als **Zustand** („welche Quelle geht die Änderung an").

In der Ruhelage griff die erste Bedeutung, obwohl die zweite gemeint war.
Getrennt in `opt.fokus` und `opt.betroffen`.

## Was ausdrücklich offen bleibt

- Ob die Kombination trägt — sie wurde erst nach diesem Urteil gebaut und
  liegt zur Bestätigung vor.
- Die **Zustände** der Spezifikation: frisch / veraltet / getrennt, Messung
  läuft / unvollständig, degradierte Funktion. Sie stehen im Streifen als
  Text, sind aber nicht durchschaltbar und nicht gestaltet.
- Was beim **Anwenden mehrerer** Änderungen passiert — „1 change applied" ist
  ein Zähler ohne Verlauf.
- Alles Visuelle.

## Bezug

- [karte-alle-quellen](2026-08-20-karte-alle-quellen.md)
- [mechanik-main-overview-detail](2026-08-20-mechanik-main-overview-detail.md)
- `werkzeug/LIES-MICH.md`, Abschnitt `bedienebenen.html`
