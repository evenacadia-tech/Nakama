# 2026-09-13 — Kurzzeit-Lautheit in der Sperrfrist nach einer Störung (Karte U42)

**Frage (Dirigent, 13.09.2026, 12:0x Uhr, in der Dirigentensession per
`AskUserQuestion`):** Nach einer Störung im Zulauf (ersetzte, unbrauchbare
Samples) bleibt die Kurzzeit-Lautheit künftig bis zu drei Sekunden ohne
gültigen Wert, bis ihr ganzes Zeitfenster wieder sauber ist (Audit-Befund
F10, NAK-283 Etappe 5; heute zeigt Gen sofort wieder einen Wert, einen
falschen). Was soll Gen in dieser Sperrfrist zeigen? Angeboten: „Nichts
zeigen — Feld leer (Empfohlen)" — wie bei fehlender Messung, kein Wert, kein
Platzhalter — oder „Letzten gültigen Wert mit Unsicherheitsmarke" — der
zuletzt gültige Wert bleibt mit deutlicher Marke „unsicher" stehen (zusätzlicher
Anzeigezustand, weiteres Feld auf dem Weg von der Sonde zu Gen).

**User-Wort, 13.09.2026** (Wahl der Option, wörtlich):

> „Nichts zeigen — Feld leer (Empfohlen)"

## Was damit festliegt

- In der Sperrfrist trägt die Kurzzeit-Lautheit kein Präsenzbit (NAK-283
  M-56 bis M-58, Regel R-283-5: Gültigkeit je Zelle); Momentary hat ihre
  eigene, kürzere Frist (0,4 s), Short-term 3,0 s.
- Gen zeigt das Feld in dieser Zeit leer, wie bei fehlender Messung — kein
  Platzhalter, kein letzter Wert, keine Marke. Kein zusätzlicher
  Anzeigezustand, kein neues Feld auf der Leitung (kein Vertragswechsel).
- Die Messung baut NAK-283 Etappe 5; der Anzeigeteil gehört zur
  Gen-Oberfläche (Planschritt S31b, Designteil geparkt seit 12.09.2026) und
  braucht dafür keine Änderung über das bestehende Verhalten „ohne Präsenzbit
  kein Wert" hinaus.
- Karte U42 in `../../docs/plan/fragen.json` ist damit beantwortet; Folge im
  Bauauftrag der Etappe 5 (`../../docs/beweise/NAK-283.md` §38.4).
