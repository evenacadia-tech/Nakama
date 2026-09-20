# 2026-09-21 — Der Markierungsauftrag bleibt bestehen, wenn FL die Verarbeitung anhält (Karte U56)

**Frage (Dirigent, 21.09.2026, 01:0x Uhr, in der Dirigentensession über
`AskUserQuestion`; der User hat die Fragen mit „stell mir die fragen" selbst
angefordert):** In Gen lässt sich eine Passage hörbar markieren. Hält FL mitten
in dieser Markierung die Verarbeitung des Plugins an (zum Beispiel beim Anhalten
der Audioausgabe oder beim Wechsel der Audioeinstellungen), passiert heute
nichts Bestimmtes: die Markierung bleibt intern stehen und klingt beim
Weiterlaufen weiter, und der Zeitraum, den Nakama als „durch die Markierung
eingefärbt" führt, wird nie geschlossen (Tiefenaudit 3, Befund T3-01-10; an der
Quelle bestätigt in `../../docs/beweise/roh/NAK-312-quellvalidierung-teil-A.md`,
Abschnitt 8). Dazu gesagt: wann genau FL diesen Anhaltemoment auslöst, ist nicht
gemessen; dass die Markierung in diesem Moment sauber verstummt und der
eingefärbte Zeitraum geschlossen wird, wird in jedem Fall gebaut und war nicht
Teil der Frage. Angeboten: „Auftrag bleibt bestehen (Empfohlen)" — läuft FL
weiter, beginnt die Markierung von vorn; gilt auch ohne Entscheidung — oder
„Anhalten beendet den Auftrag" — danach ist nichts mehr markiert, bis neu
ausgelöst wird.

**User-Wort, 21.09.2026, 01:02 Uhr** (gewählte Antwort, wörtlich):

> „Auftrag bleibt bestehen (Empfohlen)"

## Was damit festliegt

- **Weg 1 gilt:** hält FL die Verarbeitung an, verstummt die Markierung sauber
  und der eingefärbte Zeitraum wird geschlossen; der Auftrag bleibt bestehen,
  und die Markierung beginnt beim Weiterlaufen von vorn.
- Das ist der Zustand, den die Regel R-312-5 im Ticket NAK-312 ohnehin baut
  (`../../docs/beweise/NAK-312.md` §2); ein Nachzug für den anderen Weg
  entfällt.
- Karte U56 in `../../docs/plan/fragen.json` ist beantwortet; Register NAK-312
  in `../../docs/offene-punkte.md` trägt den Entscheid.
