# 2026-09-21 — Beim Rollenwechsel einer Gen-Instanz bleibt alles erhalten (Karte U49)

**Frage (Dirigent, 21.09.2026, 01:0x Uhr, in der Dirigentensession über
`AskUserQuestion`; der User hat die Fragen mit „stell mir die fragen" selbst
angefordert):** Eine Gen-Instanz kann vom Hauptprogramm zum reinen Messpunkt und
zurück wechseln. Heute ist das Ergebnis uneinheitlich: die Liste der
angemeldeten Messpunkte ist sofort weg, auch ohne Speichern; markierte Passagen,
Rollen, Schutzangaben und der Assistentenstand überstehen den Wechsel nur,
solange nicht gespeichert und neu geladen wird — danach sind auch sie weg
(Tiefenaudit 3, Befund T3-02-06; an der Quelle präzisiert in
`../../docs/beweise/roh/NAK-312-quellvalidierung-teil-B.md`, Abschnitt 3 und 8).
Angeboten: „Alles bleibt erhalten (Empfohlen)" — Messpunktliste, Passagen,
Rollen, Schutzangaben und Assistentenstand überstehen den Wechsel, live und nach
Speichern und Neuladen gleich; gelöscht wird nur durch einen ausdrücklichen
Handgriff — oder „Wechsel löscht alles bewusst" — in beiden Fällen gleich. Ohne
Entscheidung bliebe das uneinheitliche Verhalten.

**User-Wort, 21.09.2026, 01:01 Uhr** (gewählte Antwort, wörtlich):

> „Alles bleibt erhalten (Empfohlen)"

## Was damit festliegt

- **Weg 1 gilt:** die Bestände des Hauptprogramms überstehen den Wechsel zum
  Messpunkt und zurück, live und über Speichern und Neuladen gleich. Gelöscht
  wird nur durch einen ausdrücklichen Handgriff, nie als Nebenwirkung des
  Rollenwechsels.
- Unbekannte gespeicherte Felder bleiben in jedem Fall erhalten (State bleibt
  verlustfrei).
- Die Form der Ablage ist Technik und bleibt beim Dirigenten. Bekannt ist: der
  heutige Zustandsvertrag verbietet die Hauptprogramm-Bestände im
  Messpunkt-Zustand; „erhalten über Speichern hinweg" braucht deshalb ein neues,
  versioniertes Kind im Zustandsbaum — Schema, Leser, Schreiber, Fixtures und
  Migration in einem Änderungssatz, Altstände laden weiter.
- Gebaut wird das im Ticket NAK-312 (Planschritt S25k) als Etappe 7, nach dem
  Ladestart-Änderungssatz (W02), mit eigenem Matrixnachtrag und Rotbeweis im
  Bein B2.
- Karte U49 in `../../docs/plan/fragen.json` ist beantwortet; Register NAK-312
  in `../../docs/offene-punkte.md` trägt den Entscheid.
