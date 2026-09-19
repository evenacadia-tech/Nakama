# 2026-09-19 — Der AUTO-Ausgleich bekommt eine Obergrenze (Karte U54)

**Frage (Dirigent, 19.09.2026, 11:1x Uhr, in der Dirigentensession als Text;
`AskUserQuestion` ist im dontAsk-Modus gesperrt):** Der AUTO-Ausgleich rechnet
aus, wie viel lauter oder leiser die EQ-Kurve das Signal macht, und zieht das
am Ausgang wieder ab. Schneidet eine Einstellung sehr viel weg — mehrere steile
Tiefpässe übereinander, beim Suchen oder in einer geladenen Voreinstellung —,
kommt die Rechnung auf „hier fehlt fast alles" und dreht entsprechend weit
auf, heute ohne Obergrenze (Tiefenaudit 3, Befund T3-15-09, im Audit über
+40 dB; an der Quelle bestätigt in
`../../docs/beweise/roh/NAK-311-quellvalidierung-teil-B.md`, Abschnitt 2). Zu
hören ist dann nicht die Musik, sondern tiefes Rumpeln unter 20 Hz und
Gleichanteil, die die Rechnung nicht erfasst; das kann Boxen und Kopfhörer
gefährden. Angeboten: „1. Obergrenze (empfohlen)" — der Ausgleich geht
höchstens bis zu einem festen Betrag, und Nakama meldet ehrlich, dass gedeckelt
wurde; bei allen normalen Einstellungen ändert sich nichts — oder „2. Keine
Obergrenze wie heute" — volle Kompensation auch in extremen Fällen; gilt auch
ohne Entscheidung.

**User-Wort, 19.09.2026, 11:17 Uhr** (wörtlich):

> „1"

## Was damit festliegt

- **Weg 1 gilt:** der AUTO-Ausgleich bekommt eine Obergrenze. Greift sie,
  meldet Nakama das ehrlich als Zustand. Bei normalen Einstellungen ändert sich
  nichts.
- Die Höhe der Obergrenze ist Technik und bleibt beim Dirigenten; sie wird nach
  Messung in der Verhaltensmatrix festgelegt. Wie die Meldung aussieht, gehört
  zum geparkten Designteil; bis dahin steht der Zustand im Bericht des Kerns.
  Braucht die Meldung ein neues Vertragsfeld, wird es versioniert.
- Gebaut wird das im Audio-Kern-Ticket NAK-311 (Planschritt S25j) als Etappe 5,
  mit eigenem Matrixnachtrag und Rotbeweis im Bein B6. Die Berichtsklemmung auf
  die Vertragsgrenze (Teil a des Befunds, Regel R-311-5) läuft unabhängig davon
  in Etappe 4.
- Karte U54 in `../../docs/plan/fragen.json` ist beantwortet; Register NAK-311
  in `../../docs/offene-punkte.md` trägt den Entscheid.
