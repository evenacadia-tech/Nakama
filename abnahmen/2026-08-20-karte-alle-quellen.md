# 2026-08-20 — Die Karte trägt jede Quelle, immer

**Abnahme.** Beurteilt auf 760×430, in echten Pixeln, mit ehrlichen Blättern
und durchschaltbaren Arbeitsschritten. Der Preis stand während der Wahl auf
dem Blatt (`3/5`, `1/3`, „2 more not shown") und in der These.

## Das wörtliche Urteil

```json
{
 "werkzeug": "nakama-wireframe-main-karte", "version": 1,
 "zeitpunkt": "2026-08-20T18:04:06.495Z",
 "stufe": "2 / Main / Overview + Detail — die dauerhafte Karte",
 "engereWahl": ["alle", "rollen"],
 "sieger": "alle",
 "mechanik": "Alle 16, gleich hoch", "name": "Der ganze Mix, immer",
 "ablehnungen": [],
 "sekunden": { "schritt1": "68.8", "schritt2": "21.0" }
}
```

## Was damit festliegt

**Die dauerhafte Karte trägt jede Quelle, und jede bekommt dieselbe Zeile.**
Keine Gruppierung, keine Gewichtung, keine Auswahl. Der Grund steht in der
These: gleiche Fläche heißt gleiche Sache — nur so ist die Karte ein
Vergleichsinstrument.

Zusammen mit der Aufteilung (siehe unten) ist damit der **Dauerhaft-Vertrag
von Main vollständig**:

| | Baustein | Umfang |
|---|---|---|
| dauerhaft | Bedienebenen + Status | 28 px |
| dauerhaft | Übersichtskarte, eine Zeile je Quelle | **16 von 16** |
| einen Schritt entfernt | Befunde · Assistent · Detail · A/B-Urteil | |

## Die Aufteilung, mit entschieden

Vorausgegangen ist eine **Einzeichnung des Users** auf einem Screenshot:
magenta die Grenze zwischen Karte und Arbeitsfläche in zwei Entwürfen, gelb
die gewünschte Lage. Ausgemessen (Maßstab über die bekannte Blattbreite):
201 px / 282 px / **gelb ≈ 250 px**.

> Die Regel, nicht die Zahl: **Karte und Arbeitsfläche bekommen erst ihre
> natürliche Höhe, der Überschuss wird geteilt.** Nicht „jede die Hälfte" —
> das wären 229 px gewesen. Gemessen liegt die Grenze jetzt bei 245 px.

Die Karte darf wachsen, aber nicht schrumpfen — sie ist das dauerhafte Ding.

## Was das kostet, und was daraus folgt

Bei 16 Quellen braucht die Karte **212** der 370 px. Gemessen bleibt der
Arbeitsfläche:

| Schritt | Arbeitsfläche | Folge im Blatt |
|---|---|---|
| Mix lesen | 138 px | Befunde **1 von 3** |
| Befund verstehen | 155 px | Assistent **3 von 5** |
| Urteilen | 138 px | A/B-Urteil passt |

**Das ist kein hingenommener Mangel, sondern die nächste Aufgabe.** Die
Fünf-Teile-Form des Assistenten steht als Pflicht in
`docs/oberflaechen-spezifikation.md`; ein Blatt, das nur drei Teile zeigt,
zeigt einen Zustand, den die Spezifikation nicht kennt — und verstößt gegen
harte Regel 6 („Ehrliche Blätter"). Die Arbeitsfläche muss ihren
Pflichtinhalt in der Höhe liefern, die die Karte übriglässt. Das ist
handwerklich und wird nicht erneut zur Wahl gestellt.

## Nachtrag am selben Tag: der Preis ist eingelöst

Die oben genannte Klemme ist **gelöst**, ohne die Wahl anzutasten. Gemessen
auf 760×430 mit 16 Quellen liefert der gewählte Entwurf jetzt in **allen drei
Arbeitsschritten** seinen vollen Pflichtinhalt:

| Schritt | Karte | Arbeitsfläche |
|---|---|---|
| Mix lesen | 16/16 | Befunde **3/3** |
| Befund verstehen | 16/16 | Assistent **5/5** |
| Urteilen | 16/16 | A/B + Detail, vollständig |

Drei Eingriffe, alle gemessen:

1. **Die Ratsche in der Aufteilung beseitigt.** Die Karte wuchs auf 230 px,
   obwohl sie für ihre 16 Zeilen nur 212 braucht — weil die Arbeitsfläche
   ihre Listen kürzte, dadurch schrumpfte deren Basis und die Karte wuchs
   nach. Jetzt gilt: **Überschuss teilen, Mangel nicht.** Die Karte hält bei
   Platzmangel ihre natürliche Höhe. Das gab der Arbeitsfläche 156 statt 138.
2. **Die Befunde bekamen echte Disclosure**: einer offen, die anderen als
   benannte Zeile mit Ort und Sicherheit. Alle drei bleiben sichtbar, nur die
   Tiefe ist einen Griff entfernt — was die Spezifikation mit „genau EINEN
   Schritt zur Zeit" ohnehin verlangt.
3. **Der Assistent wurde dichter, nicht kleiner.** Die fehlenden 16 px kamen
   aus den Abständen (Zeilenabstand 5→2, Knopfreihe 6→4, Kopfzeile 6/5→5/4),
   nicht aus Inhalt und nicht aus der Schriftgröße. Er misst jetzt 154 px bei
   156 verfügbaren — knapp, und das ist eine Tatsache über diese Fläche, kein
   Versehen.

**Achtung für spätere Sessions:** das Werkzeug zeigt heute `5/5` und `3/3`,
zum Zeitpunkt des Urteils zeigte es `3/5` und `1/3`. Die Wahl fiel also mit
sichtbarem Preis; der Preis wurde danach eingelöst. Das Urteil selbst bleibt
davon unberührt.

## Was aus dem Verlierer mitzunehmen ist

„Karte nach Rollen" war der Finalist und der einzige Entwurf, in dem alles
bequem passte (Assistent 5/5, Befunde 3/3). Er ist unterlegen, weil er die
einzelne Quelle im Mittel ihrer Gruppe verschwinden lässt. **Die Rollen
bleiben als Ordnungsmittel interessant** — die Spezifikation kennt sie
ohnehin („musikalische Prioritäten", „Fokusgruppen"). Sie könnten die
Reihenfolge oder Gruppierung *innerhalb* der Karte tragen, ohne Zeilen
zusammenzufassen.

## Was ausdrücklich offen bleibt

- **Wie die Arbeitsfläche ihren Pflichtinhalt unterbringt** — Aufgabe, keine
  Frage.
- Die Reihenfolge der Quellen in der Karte (alphabetisch, nach Rolle, nach
  Energie, vom User gesetzt).
- Die vier Bedienebenen: „sichtbar getrennt" ist bisher ein Streifen mit vier
  Wörtern.
- Zustände: frisch / veraltet / getrennt, Messung läuft / unvollständig,
  degradierte Funktion — als Schalter noch nicht gebaut.
- Alles Visuelle.

## Kein Prüfpunkt ausgelöst

Welt und Fassung bleiben **Arbeitsannahmen**. Ihr Prüfpunkt ist „sobald das
Layout von Main steht"; die Arbeitsfläche ist noch nicht gelöst und die
Zustände fehlen.

## Bezug

- [mechanik-main-overview-detail](2026-08-20-mechanik-main-overview-detail.md)
- `werkzeug/LIES-MICH.md`, Abschnitt `wireframe-main-karte.html`
