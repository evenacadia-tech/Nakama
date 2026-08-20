# Werkzeug — Messinstrumente, keine Entwürfe

Hier liegen Hilfsmittel, mit denen die Vorstellung des Users **gemessen** wird.
Nichts davon ist ein Gestaltungsvorschlag. Layoutentscheidungen gehören nach
`prototyp/` und in Phase 1 des Arbeitsplans.

## Reihenfolge — die ist der eigentliche Punkt

1. **`weltenwahl.html`** — zuerst. Welche Welt? *(entschieden: Glas und Licht)*
2. **`glaswelt.html`** — dann. Welche Verwirklichung? *(entschieden: dunkles
   Glas, ein Lichtleiter)*
3. **`licht.html`** — dann. Woher kommt das zusätzliche Licht, und wieviel?
   Kein Quiz mehr, sondern ein Drehinstrument: vier Herkünfte, vier Regler,
   alles wirkt sofort.
4. **`stilbefragung.html`** — zuletzt. Welche Werte *innerhalb* der Fassung?
   Die Spezimen darin zeigen noch die alte, verworfene Welt und müssen vor dem
   nächsten Einsatz umgestellt werden.

Diese Reihenfolge war der Fehler der ersten Runde: Stilbefragung wurde gebaut,
bevor die Welt entschieden war, also fragte sie nach Radien innerhalb einer
Welt, die Claude sich selbst ausgesucht hatte. Siehe
`abnahmen/2026-08-20-methode-visuelle-befragung.md`.

## weltenwahl.html

Sechs Formsprachen nebeneinander, **gleiche Information** (Quelle,
Frequenzkurve, ein Wert, ein Zustand), **alles andere verschieden**: Material,
Licht, ob es überhaupt ein Gehäuse gibt, wie viel Text, wie bedient wird.

Die Achsen stammen aus `assets/GUI-Tips.pdf` (Voger Design, „Quality GUI
Checklist", Stufe 1) — Entity Type, Material, Licht, Temperatur — nicht aus
Claudes Geschmack. Zwei Welten sind direkte Übersetzungen von Referenzen aus
der Truhe: „Dunkles Gerät" aus `301.zip`, „Helles Objekt" aus `110.zip`.

Ablauf: sechs → zwei in die engere Wahl → Zweikampf groß → Ergebnis als JSON.

## glaswelt.html

Fünf Verwirklichungen der abgenommenen Welt, gleiche Information an gleicher
Stelle: Milchglas vor kaltem Licht · die Messung ist die Lichtquelle ·
geschichtete Scheiben · dunkles Glas mit einem Lichtleiter · Glas mit Griff
(nimmt Korn und Leuchtring aus dem unterlegenen „dunklen Gerät" mit).

Der Befund erscheint hier erstmals **ohne Text**: ein Ring sagt, wie sicher die
Aussage ist, ein weiches Band im Kurvenfeld sagt, wo sie gilt.

Die Thesen zu den Varianten sind standardmäßig ausgeblendet — wer sie vorher
liest, wählt den besseren Satz statt das bessere Bild.

## licht.html

Die Fassung steht, offen ist das Licht. Vier **Herkünfte** (nur der Grund
angehoben · der Lichtleiter strahlt in den Raum · ein Schein hinter dem Glas ·
die Glaskanten fangen Licht) und vier Regler (Grundhelligkeit, Lichtmenge,
Farbanteil, Streuung).

Zwei Dinge stecken im Code, die leicht zu übersehen wären:

- Alles läuft über CSS-Variablen, nichts wird neu gebaut — der Regler wirkt
  ohne Sprung, und das Auge kann kleinen Unterschieden folgen.
- **Regler, die in der gewählten Herkunft nichts bewirken, sind abgeschaltet
  und zeigen „—".** Ein Regler, der sich bewegen lässt und nichts tut, ist eine
  Lüge in der Oberfläche.

„Original halten" zeigt gedrückt die unveränderte Ausgangsfassung — Vergleich
ohne Umschalten, damit sich das Auge zwischendurch nicht anpasst.

## stilbefragung.html

18 Runden Zwangswahl auf echtem Nakama-Inhalt, pro Runde genau ein Token
verschieden. Erst sinnvoll, wenn die Welt steht — dann müssen ihre Spezimen
auf diese Welt umgestellt werden.

## Prinzipien, die im Code stehen und nicht verhandelbar sind

1. **Eine Variable pro Runde** — aber nur unterhalb der Weltebene. Auf
   Weltebene ändert sich absichtlich alles gleichzeitig, weil eine Welt kein
   Parameter ist.
2. **Kein Variantenname während der Wahl.** Ein Label wie „technisch" nimmt die
   Antwort vorweg. Die Kacheln heißen 1 bis 6; die Namen kommen erst im
   Ergebnis, als Vokabular für das Gespräch danach.
3. **Neutrales Mittelgrau als Umgebung.** Ein dunkles Werkzeug-Chrome ließe
   dunkle Kacheln gewinnen, ein helles die hellen.
4. **Die Großansicht skaliert dieselbe Kachel**, sie baut sie nicht neu — sonst
   verglichest du in Stufe 2 etwas anderes als das, was du ausgewählt hast.
5. **Kornstärke ist pro Welt verschieden.** Die Welt, deren Aussage „kein
   Material" ist, bekommt keine Struktur — sonst widerlegt die Kachel ihre
   eigene These.
6. **Demo-Kennzeichnung gehört zum Werkzeug, nicht zum Spezimen.** Sonst wäre
   sie Teil dessen, was beurteilt wird.
7. **Ablehnung wird erfasst.** Bei Abneigung sind Menschen präziser als bei
   Zuneigung, und der Designvertrag braucht Antireferenzen ohnehin.

## textur/

`korn.png` — 512er-Kachel, aus `assets/301.zip` herauskopiert (Mittelwert
exakt 128, also für `mix-blend-mode: overlay` gebaut), Kanten überblendet damit
sie nahtlos kachelt. Die Truhe selbst wurde nicht verändert.
