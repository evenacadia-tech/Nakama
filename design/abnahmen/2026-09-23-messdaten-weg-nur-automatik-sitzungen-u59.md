# 2026-09-23 — Messwerte an andere Programme: nur in den Messläufen der Automatik (Karte U59)

**Frage (Dirigent, 23.09.2026, in der Dirigentensession `dfab8765` über
`AskUserQuestion`, gestellt im Zeitfenster nach Skill `/fragen U59`):**
Nakama kann seine Messwerte über eine Datei an Werkzeuge herausgeben; die
Automatik nutzt das, um nach jedem Bau in FL nachzumessen. Heute ist dieser
Weg in jeder FL-Sitzung offen, auch in den eigenen des Users: jedes Programm
unter seinem Windows-Konto kann dort Messwerte des laufenden Projekts abholen
(Lautheit, Spektrum, Stereobild, Namen der Messpunkte). Am Klang ändert das
nichts; dass ein interner Schlüssel des Brokers nicht mehr in diesen
Antworten steht, gilt in beiden Fällen. Angeboten: „Nur Automatik-Sitzungen
(Empfohlen)" — offen nur in FL-Sitzungen, die die Automatik für ihre
Messläufe selbst startet; in eigenen Sitzungen zu; für eine gewünschte
Messung einer eigenen Sitzung startet der User FL über eine eigene
Verknüpfung, die der Dirigent ihm gibt. „Wie heute: jede Sitzung" — offen in
jeder FL-Sitzung. Ohne Antwort hätte der Dirigent Weg 1 gebaut.

**User-Wort, 23.09.2026, 12:12 Uhr** (gewählte Antwort, wörtlich; Zeitpunkt
aus dem Sessiontranskript):

> „Nur Automatik-Sitzungen (Empfohlen)"

## Was damit festliegt

- **Weg 1 gilt als Produktentscheid** (Technik: Regel R-310-2 in
  `../../docs/beweise/NAK-310.md` §2, Etappe 5 des Etappenplans §4): der
  Diagnose-Briefkasten von Nakama ist nur in den FL-Sitzungen offen, die die
  Automatik (Laufzeit-Arm `tools/fl/laufzeit.ps1`) für ihre Messläufe selbst
  startet. In Sitzungen, die der User selbst öffnet, ist er zu.
- Will der User eine eigene Sitzung ausmessen lassen, startet er FL über eine
  eigene Verknüpfung, die der Dirigent ihm gibt; wie diese Marke gesetzt und
  gebunden wird, ist Technik und bleibt beim Dirigenten (Matrix §6 und Bauplan
  §7 des Manifests; die Befunde der Matrixprüfung 1 zur Lebensdauer der Marke
  werden dort eingeordnet).
- Dass die Antworten des Briefkastens keinen internen Schlüssel des Brokers
  mehr tragen, gilt unabhängig von dieser Wahl (Audit-Befund T3-12-04,
  Bündel B30).
- Gebaut wird das im Ticket NAK-310 (Planschritt S25i) als Etappe 5, mit
  Rotbeweisen, vollem Kanon und Laufzeit-Arm; die Matrixzeilen, die bisher
  „nach U59" hießen, sind damit bindend.
- Karte U59 in `../../docs/plan/fragen.json` ist beantwortet; Register NAK-310
  in `../../docs/offene-punkte.md` trägt den Entscheid; `../../docs/ZIELBILD.md`
  führt ihn unter „Im Detail festgelegt".
