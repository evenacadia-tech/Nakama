# 2026-09-13 — Headroom-Verteilung ohne markierte Passage (Karte U41)

**Frage (Dirigent, 13.09.2026, 12:03 Uhr, in der Dirigentensession per
`AskUserQuestion`):** Gen zeigt die Headroom-Verteilung (wie viel Luft nach
oben das Signal über die Zeit hatte, von leisen bis lauten Momenten) künftig
über die ganze markierte Passage (Audit-Befund F08, NAK-283 Etappe 5). Was
soll sie zeigen, solange keine Passage markiert ist? Angeboten: „Gleitendes
Fenster mit sichtbarer Länge (Empfohlen)" — die Verteilung beschreibt nur die
letzten Sekunden (heute 6,4 s), die Fensterlänge steht sichtbar dabei — oder
„Seit der letzten Transportgrenze" — alles seit dem letzten Start oder Sprung
des Abspielkopfs, beliebig lang.

**User-Wort, 13.09.2026** (Wahl der Option, wörtlich):

> „Gleitendes Fenster mit sichtbarer Länge (Empfohlen)"

## Was damit festliegt

- Ohne markierte Passage beschreibt die Headroom-Verteilung ein gleitendes
  Fenster fester Länge (heute die 64 Rahmen des Verteilungsrings, 6,4 s bei
  10 Hz); sie sammelt nicht seit der letzten Transportgrenze.
- Mit markierter Passage beschreibt sie die ganze Passage (NAK-283 M-53 bis
  M-55, Histogramm über die Passage).
- In beiden Fällen nennt `headroomFenster` die tatsächliche Zahl der
  eingegangenen Rahmen; die Anzeige beschriftet die Fensterlänge sichtbar.
  Der Anzeigeteil gehört zur Gen-Oberfläche (Planschritt S31b, Designteil
  geparkt seit 12.09.2026); die Messung baut NAK-283 Etappe 5.
- Karte U41 in `../../docs/plan/fragen.json` ist damit beantwortet; Folge im
  Bauauftrag der Etappe 5 (`../../docs/beweise/NAK-283.md` §38.4).
