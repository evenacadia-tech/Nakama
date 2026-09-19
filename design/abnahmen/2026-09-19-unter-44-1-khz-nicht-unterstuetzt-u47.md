# 2026-09-19 — Projekte unter 44,1 kHz werden nicht unterstützt, der EQ bleibt dort neutral (Karte U47)

**Frage (Dirigent, 19.09.2026, 11:2x Uhr, in der Dirigentensession als Text;
`AskUserQuestion` ist im dontAsk-Modus gesperrt):** FL-Projekte laufen fast
immer mit 44,1 oder 48 kHz oder mehr. Nakamas Regeln nennen 44,1 kHz als
kleinste unterstützte Abtastrate; trotzdem nimmt der EQ heute auch 22,05 oder
32 kHz an, und dann liegen einige Stützpunkte des AUTO-Ausgleichs oberhalb des
Bereichs, den es bei dieser Rate überhaupt gibt (Tiefenaudit 3, Abdeckungsfeld
F08, in Phase 16 mit 32 und 22,05 kHz gemessen: 4 von 11 Stützpunkten oberhalb
der Grenze; an der Quelle bestätigt in
`../../docs/beweise/roh/NAK-311-quellvalidierung-teil-B.md`, Abschnitt 7). Ob
FL Studio solche Raten überhaupt anbietet, ist nicht belegt. Angeboten:
„1. Nicht unterstützen (empfohlen)" — der EQ bleibt neutral, das Signal läuft
unverändert durch, und Nakama meldet den Grund als ehrlichen Zustand; wie die
Anzeige das zeigt, gehört zum geparkten Designteil — oder „2. Unterstützen" —
der EQ arbeitet auch bei niedrigen Raten, mit einer daran angepassten Rechnung.
Ohne Antwort bleibt es beim heutigen Verhalten.

**User-Wort, 19.09.2026, 11:26 Uhr** (wörtlich):

> „1"

## Was damit festliegt

- **Weg 1 gilt:** unter 44,1 kHz ist der EQ nicht unterstützt. Er bleibt dort
  neutral, das Signal läuft unverändert durch (Grundgesetz „nichts
  Ungefragtes"), und Nakama meldet den Grund als ehrlichen Zustand.
- Wie der Zustand in der Oberfläche aussieht, gehört zum geparkten Designteil;
  bis dahin steht er im Bericht des Kerns. Die Messung in Probeeq und Gen ist
  von diesem Entscheid nicht berührt.
- Gebaut wird das im Audio-Kern-Ticket NAK-311 (Planschritt S25j) als Etappe 5,
  mit eigenem Matrixnachtrag und Rotbeweis im Bein A16 (Fall 32 kHz); genau
  44,1 kHz bleibt unterstützt.
- Karte U47 in `../../docs/plan/fragen.json` ist beantwortet; Register NAK-311
  in `../../docs/offene-punkte.md` trägt den Entscheid.
