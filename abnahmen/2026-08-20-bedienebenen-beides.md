# 2026-08-20 — Bedienebenen: der Streifen führt Buch, das Blatt zeigt das Vorhören

**Richtungsabnahme.** Beurteilt wurden drei bedienbare Blätter auf 760×430,
mit echtem Halten und Loslassen. Die Kombination selbst war zu diesem
Zeitpunkt noch nicht gebaut — sie ist die Übersetzung des Urteils und wurde
danach vorgelegt.

## Das wörtliche Urteil

> „es ist 1 und 2 eine kombination daraus."

Dazu, zu einem Ausschnitt der Karte:

> „die übergroße zeile wirkt nichti richtig"

## Was damit festliegt

**Zwei Fragen, zwei Orte:**

| Frage | Ort | Zeichen |
|---|---|---|
| *Was ist der Stand?* | der **Streifen** | Ebenen-Reiter leuchtet · „1 change applied" mit Rückweg |
| *Läuft gerade etwas?* | das **Blatt** | Rahmen, Karte tritt zurück — solange gehalten |

Die dritte Antwort — „der Zustand klebt am Ort der Wirkung" — ist damit
ausgeschieden.

## Zwei Entscheidungen, die ich beim Bauen getroffen habe

Beide sind mechanisch, nicht gestalterisch, und stehen hier, damit sie
nachprüfbar sind:

1. **Der Streifen wird beim Halten NICHT mitgedämpft.** In der reinen
   Blatt-Variante dämpfte das Blatt auch den Streifen. In der Kombination
   wäre das falsch: dann verdunkelt das Blatt genau die Auskunft, die man in
   diesem Moment braucht. Gedämpft wird nur die Karte — der Zusammenhang
   tritt zurück, die Sache bleibt.
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
