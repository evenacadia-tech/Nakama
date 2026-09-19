# 2026-09-19 — Die Höhen-Korrektur des digitalen EQ (Decramping) ist vertagt (Karte U46)

**Frage (Dirigent, 19.09.2026, 11:2x Uhr, in der Dirigentensession als Text;
`AskUserQuestion` ist im dontAsk-Modus gesperrt):** Digitale EQ-Filter drücken
Glocken und Höhenregler nahe der halben Abtastrate zusammen: bei 44,1 kHz wird
eine Glocke bei 16 kHz zur oberen Grenze hin schmaler und schief, anders als
bei einem analogen EQ. Eine Korrektur (Decramping) gleicht das an und kostet
etwas Rechenzeit je Band (Tiefenaudit 3, Befund T3-15-10: ein alter
Prüfauftrag nennt die Korrektur als zu prüfen, ohne Entscheid und ohne
Matrixzeile; an der Quelle bestätigt in
`../../docs/beweise/roh/NAK-311-quellvalidierung-teil-B.md`, Abschnitt 5).
Angeboten: „1. Vertagen (empfohlen)" — erst den Kern fertigstellen, später ein
eigener Schritt mit Hörprobe; gilt auch ohne Antwort — „2. Jetzt einbauen, im
Audio-Kern-Ticket" — dann muss die gemessene Filter-Referenz, gegen die heute
jeder Filtertest läuft, neu festgelegt und neu abgenommen werden — oder
„3. Verwerfen" — es bleibt dauerhaft beim heutigen Verhalten.

**User-Wort, 19.09.2026, 11:24 Uhr** (wörtlich):

> „1."

## Was damit festliegt

- **Weg 1 gilt:** die Höhen-Korrektur ist vertagt, nicht verworfen. Am Klang
  und an der gemessenen Filter-Referenz (RBJ-Goldens) ändert sich jetzt nichts.
- Sie kommt später als eigener Schritt mit Hörprobe; wann, entscheidet der User
  mit einer neuen Karte, wenn der Kern fertig ist. Register NAK-332 in
  `../../docs/offene-punkte.md` hält den Punkt.
- Im Audio-Kern-Ticket NAK-311 ist der Befund T3-15-10 damit entschieden: kein
  Bau, der offene Prüfauftrag ist durch diesen Entscheid ersetzt.
- Karte U46 in `../../docs/plan/fragen.json` ist beantwortet.
