# 2026-09-19 — Die Schwelle eines dynamischen Bands ist je Band zwischen Spitze und Durchschnitt umschaltbar (Karte U45)

**Frage (Dirigent, 19.09.2026, 11:1x Uhr, in der Dirigentensession als Text;
`AskUserQuestion` ist im dontAsk-Modus gesperrt):** Ein dynamisches EQ-Band
greift, sobald der Pegel in seinem Frequenzbereich eine Schwelle (Threshold)
überschreitet. Heute ist nicht festgelegt, was „Pegel" dabei heißt: je nach
Hold- und Attack-Einstellung folgt die Messung mal dem Augenblickswert, mal
einem Mittel, derselbe Threshold wirkt dadurch unterschiedlich (Tiefenaudit 3,
Befund T3-15-06; an der Quelle bestätigt in
`../../docs/beweise/roh/NAK-311-quellvalidierung-teil-B.md`, Abschnitt 4).
Angeboten: „1. Durchschnittspegel über einen kurzen Zeitraum, RMS (empfohlen)"
— greift gleichmäßig, reagiert weniger nervös auf einzelne Schläge — oder
„2. Spitzenpegel, Peak" — greift sofort auf Transienten wie Snare-Schläge, kann
bei perkussivem Material flattern. Attack, Hold und Release bleiben in beiden
Fällen einstellbar wie heute; ohne Entscheidung bleibt es beim heutigen,
uneinheitlichen Verhalten.

**User-Wort, 19.09.2026, 11:2x Uhr** (wörtlich):

> „beides einzeln auswählbar"

**Nachfrage (Dirigent, gleiche Sitzung):** „einzeln" kann zwei Dinge heißen.
Teil 1, wo der Umschalter sitzt: „1. Je Band" — jedes dynamische Band hat
seinen eigenen Umschalter zwischen Spitze und Durchschnitt — oder „2. Einmal
für den ganzen EQ". Teil 2, was für ein neues Band gilt, bevor umgeschaltet
wird: „1. Durchschnitt (empfohlen)" oder „2. Spitze". Dazu die Folge: ein
Umschalter ist eine neue gespeicherte Einstellung, braucht ein versioniertes
Projektformat und einen Bedienpunkt in Gen; im Audio-Kern-Ticket entstehen
zunächst beide Messarten im Kern mit der Ausgangseinstellung aus Teil 2, der
sichtbare Umschalter kommt als eigener Schritt mit dem EQ in Gen.

**User-Wort, 19.09.2026, 11:21 Uhr** (wörtlich):

> „1 und 1"

## Was damit festliegt

- **Beide Messarten gibt es, je Band umschaltbar:** jedes dynamische Band hat
  seinen eigenen Umschalter zwischen Spitze und Durchschnitt.
- **Ein neues dynamisches Band startet auf Durchschnitt.**
- Die Länge des Messfensters für den Durchschnitt und die Bauart der
  Spitzenmessung sind Technik und bleiben beim Dirigenten.
- Im Audio-Kern-Ticket NAK-311 (Planschritt S25j, Etappe 5) bekommt der Kern
  beide Messarten mit festgelegtem Pegelbegriff; aktiv ist Durchschnitt. Kein
  neues gespeichertes Feld in diesem Ticket.
- Der Umschalter je Band ist ein neuer gespeicherter Wert: zuerst versioniert
  (gespeicherter Zustand, Parameter, Nachricht zwischen den Programmteilen),
  Altstände ohne den Wert laden mit Durchschnitt. Register NAK-331 in
  `../../docs/offene-punkte.md`; zugeordnet den Planschritten S26–28 (Zustand
  und Vertrag), S28b (Gen-Master-EQ) und S31b (Bedienung, Designteil geparkt).
- Karte U45 in `../../docs/plan/fragen.json` ist beantwortet.
