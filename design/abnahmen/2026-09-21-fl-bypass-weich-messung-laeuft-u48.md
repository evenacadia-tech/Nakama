# 2026-09-21 — FLs eigener Bypass-Knopf blendet weich, die Messung läuft weiter (Karte U48)

**Frage (Dirigent, 21.09.2026, 01:0x Uhr, in der Dirigentensession über
`AskUserQuestion`; der User hat die Fragen mit „stell mir die fragen" selbst
angefordert):** FL hat an jedem Mixer-Slot einen eigenen Bypass-Knopf, getrennt
von Nakamas eigenem EQ-Schalter. Heute schaltet FLs Knopf hart um: der EQ ist
von einem Sample zum nächsten weg, und Probeeq hört dabei auch auf zu messen —
in Gen veraltet diese Spur, solange der Knopf gedrückt ist. Nakamas eigener
Schalter blendet dagegen weich über (etwa 5 ms). Dass die Filter nach dem
Zurückschalten neu anfangen, ist bei beiden Schaltern gleich und schon
entschieden; das war nicht Teil der Frage (Tiefenaudit 3, Befund T3-01-09; an
der Quelle bestätigt in
`../../docs/beweise/roh/NAK-312-quellvalidierung-teil-A.md`, Abschnitt 7).
Angeboten: „Weich + Messung läuft (Empfohlen)" — FLs Knopf blendet genauso
weich über wie Nakamas eigener Schalter, und Probeeq misst währenddessen
weiter; „Hart, aber Messung läuft"; „Alles wie heute" — hart umschalten, und
die Messung pausiert; gilt auch ohne Entscheidung.

**User-Wort, 21.09.2026, 01:01 Uhr** (gewählte Antwort, wörtlich):

> „Weich + Messung läuft (Empfohlen)"

## Was damit festliegt

- **Weg 1 gilt:** FLs Slot-Bypass verhält sich wie Nakamas eigener Schalter —
  weiches Überblenden statt hartem Schnitt —, und Probeeq misst im Hostbypass
  weiter; die Spur veraltet in Gen nicht.
- Der Kaltstart der Filter nach dem Zurückschalten bleibt, wie er entschieden
  ist (E-8 in `../../docs/beweise/SONDE-015.md`).
- Wie der Eintritt des Hostbypass gebaut wird und was Probeeq währenddessen
  genau misst, ist Technik und bleibt beim Dirigenten; wie Gen den Zustand „EQ
  aus" zeigt, gehört zum geparkten Designteil.
- Gebaut wird das im Ticket NAK-312 (Planschritt S25k) als Etappe 7, mit eigenem
  Matrixnachtrag und Rotbeweis im Bein A16.
- Karte U48 in `../../docs/plan/fragen.json` ist beantwortet; Register NAK-312
  in `../../docs/offene-punkte.md` trägt den Entscheid.
