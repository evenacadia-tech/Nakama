# 2026-09-19 — Große Sprünge eines EQ-Bands werden überblendet (Karte U44)

**Frage (Dirigent, 19.09.2026, 11:0x Uhr, in der Dirigentensession als Text;
`AskUserQuestion` ist im dontAsk-Modus gesperrt):** Springt ein EQ-Band in
einem Schritt weit — beim Laden einer Voreinstellung, beim Tippen eines Werts,
bei einer Automationsstufe —, kann für den Bruchteil einer Sekunde ein deutlich
lauteres Wummern oder Zischen durchkommen, das weder vorher noch nachher im
Klang steckt (Übergangsspitze; Tiefenaudit 3, Befund T3-15-07, im Audit etwa 17
bis 22 dB über dem Klang gemessen; an der Quelle bestätigt in
`../../docs/beweise/roh/NAK-311-quellvalidierung-teil-B.md`, Abschnitt 1).
Heute gleitet Nakama bei jeder Änderung in rund 5 Millisekunden vom alten zum
neuen Wert; bei kleinen Änderungen ist das sauber, bei großen Sprüngen entsteht
genau dabei die Spitze. Angeboten: „1. Überblenden (empfohlen)" — der alte
Klang blendet in wenigen Millisekunden in den neuen, im Audit praktisch ohne
Spitze; kostet nur im Moment des Sprungs etwas mehr Rechenzeit; langsame
Reglerfahrten klingen exakt wie heute — „2. Lassen wie heute" — die Spitze
bleibt; gilt auch ohne Entscheidung — oder „3. Hart umschalten" — kleinere
Spitze als heute (im Audit etwa 7 dB), dafür ein hörbarer Klick.

**User-Wort, 19.09.2026, 11:10 Uhr** (wörtlich):

> „1"

## Was damit festliegt

- **Weg 1 gilt:** große Sprünge eines EQ-Bands werden kurz überblendet. Kleine
  Änderungen und langsame Reglerfahrten laufen weiter über die heutige Glättung
  und klingen wie bisher.
- Ab wann ein Sprung als groß gilt und wie lang die Überblendung dauert, ist
  Technik und bleibt beim Dirigenten; beides wird in der Verhaltensmatrix mit
  Maß und Messung festgelegt.
- Gebaut wird das im Audio-Kern-Ticket NAK-311 (Planschritt S25j) als Etappe 5,
  nach dem Slot-Lebenszyklus der Etappe 3 (Arbeitspaket W03 ist Vorbedingung
  von W07), mit eigenem Matrixnachtrag und Rotbeweis im Bein B6. Das Grundgesetz
  bleibt: ausgeschaltet bitidentisch, keine Allokation im Audio-Thread.
- Karte U44 in `../../docs/plan/fragen.json` ist beantwortet; Register NAK-311
  in `../../docs/offene-punkte.md` trägt den Entscheid.
