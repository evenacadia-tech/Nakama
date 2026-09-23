# 2026-09-23 — FLs Bypass-Knopf loslassen: die Filter laufen weiter (Karte U58)

**Frage (Dirigent, 23.09.2026, 09:03 Uhr, in der Dirigentensession über
`AskUserQuestion`; gestellt im Zeitfenster über den One-Shot-Cron der
Session nach Skill `/fragen U58`):** FLs Bypass-Knopf am Mixer-Slot
loslassen: laufen die Filter durch oder fangen sie neu an? Der User hatte am
21.09.2026 gewählt, dass FLs eigener Bypass-Knopf weich überblendet und
Probeeq währenddessen weiter misst (Karte U48,
`2026-09-21-fl-bypass-weich-messung-laeuft-u48.md`). Damit die Messung im
Bypass weiterläuft, rechnet der EQ im Hintergrund weiter; hörbar ist nur das
unbearbeitete Signal; beim Loslassen blendet Nakama in etwa 5 ms zurück.
Angeboten: „Filter laufen weiter (Empfohlen)" — kein Einschwingen, kein
hörbarer Knick beim Zurückblenden, Probeeq misst im Bypass weiter den
bestätigten EQ; „Filter fangen kalt neu an" — wie bei Nakamas eigenem
Schalter, bei einem schmalen, tiefen Band als kurzer Einbruch bis etwa 200 ms
hörbar, Probeeq misst im Bypass das unbearbeitete Signal und Gen zeigt es als
Wirkung des EQ. In der Frage vom 21.09. hieß es, die Filter fingen bei beiden
Schaltern neu an; das galt für Nakamas Schalter, nicht für FLs Knopf, der
bisher gar keine Blende hatte. Ohne Antwort wäre dieser Teil nicht gebaut
worden.

**User-Wort, 23.09.2026, 09:05 Uhr** (gewählte Antwort, wörtlich):

> „Filter laufen weiter (Empfohlen)"

## Was damit festliegt

- **Weg 1 gilt (Technik: Weg K-B, E-312-19):** nach dem Lösen von FLs
  Bypass-Knopf laufen die Filter so weiter, wie sie im Hintergrund gerechnet
  haben; kein Einschwingen, kein hörbarer Knick. Probeeq misst im Hostbypass
  weiter den bestätigten EQ, Gens Landkarte und der Advisor rechnen mit dem
  Signal, das der bestätigte EQ erzeugt.
- Der Satz der Frage vom 21.09.2026, die Filter fingen bei beiden Schaltern
  neu an, ist damit für FLs Knopf überholt; für Nakamas eigenen EQ-Schalter
  bleibt der Kaltstart, wie er entschieden ist (E-8 in
  `../../docs/beweise/SONDE-015.md`).
- Wie der Eintritt des Hostbypass und die Blende gebaut werden, ist Technik
  und bleibt beim Dirigenten (Bauplan `../../docs/beweise/NAK-312.md` §47.3,
  Weg E1 mit K-B); wie Gen den Zustand zeigt, gehört zum geparkten Designteil.
- Gebaut wird das im Ticket NAK-312 (Planschritt S25k) als Etappe 7b, Teil 2
  (Satz 2: Matrixzeilen M-94 bis M-109 und M-132), mit eigenem Kanon,
  Laufzeit-Arm und Erstprüfung.
- Karte U58 in `../../docs/plan/fragen.json` ist beantwortet; Register NAK-312
  in `../../docs/offene-punkte.md` trägt den Entscheid.
