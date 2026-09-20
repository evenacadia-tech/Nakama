# 2026-09-21 — Die Annahme von Probeeq-Instanzen wird auf 20 begrenzt (Karte U51)

**Frage (Dirigent, 21.09.2026, 01:0x Uhr, in der Dirigentensession über
`AskUserQuestion`; der User hat die Fragen mit „stell mir die fragen" selbst
angefordert):** Gens Quellenliste ist für 16 Zeilen zugesagt, heute passen 20;
intern nimmt Nakama bis zu 64 Quellen an. Ab der 21. ist eine Quelle zwar
angenommen, taucht in der Liste aber gar nicht mehr auf — man sieht nicht, dass
sie fehlt (Tiefenaudit 3, Befund T3-07-05; an der Quelle bestätigt in
`../../docs/beweise/roh/NAK-312-quellvalidierung-teil-B.md`, Abschnitt 6 und 8).
Dazu gesagt: der Auditentwurf bevorzugte Weg 2; die Empfehlung für Weg 1 folgt
daraus, dass Weg 2 eine blätterbare Liste braucht und erst nach der geparkten
Designarbeit baubar ist; bereits gespeicherte Quellen werden in keinem Fall
gelöscht. Angeboten: „Auf 20 begrenzen (Empfohlen)" — eine 21. Probeeq meldet
ehrlich „Liste voll" und misst nicht mit, bis Platz ist — oder „Bis 64
erreichbar halten" — bis zur Designarbeit bliebe die 21. Quelle unsichtbar wie
heute; gilt auch ohne Entscheidung.

**User-Wort, 21.09.2026, 01:02 Uhr** (gewählte Antwort, wörtlich):

> „Auf 20 begrenzen (Empfohlen)"

## Was damit festliegt

- **Weg 1 gilt:** Gen nimmt höchstens 20 Probeeq-Instanzen an. Eine weitere
  meldet ehrlich, dass die Liste voll ist, und misst nicht mit, bis Platz ist.
- Bereits gespeicherte Quellen werden nie gelöscht, um sie an diese Grenze
  anzupassen.
- Wo die Grenze sitzt, wie die Meldung technisch transportiert wird und wie ein
  Altprojekt mit mehr als 20 gespeicherten Quellen behandelt wird, ist Technik
  und bleibt beim Dirigenten; das Aussehen der Meldung gehört zum geparkten
  Designteil, bis dahin ist sie eine ehrliche Zustandsmeldung.
- Unabhängig davon und schon vorher gebaut (Regel R-312-6): kein scharfer Knopf
  steht auf einer ungezeichneten Quelle.
- Gebaut wird die Grenze im Ticket NAK-312 (Planschritt S25k) als Etappe 7, mit
  eigenem Matrixnachtrag und Rotbeweis im Bein B15.
- Karte U51 in `../../docs/plan/fragen.json` ist beantwortet; Register NAK-312
  in `../../docs/offene-punkte.md` trägt den Entscheid.
