# briefing-hub — stillgelegt am 23.08.2026

Dieser Ordner ist **Verlauf, kein Bauteil.** Nicht deployen, nicht füttern,
nicht als Quelle lesen.

Er enthält die Briefing-Seite `nakama-briefing.philipld.chatgpt.site`, die vom
22. bis 23.08.2026 die Sicht des Projektleiters auf Plan, offene Punkte und
Figma-Stand war. Der User hat sie abgeschafft:

> „wir brauchen eine andere möglichkeit als den hub. eie gibt keine
> automatische aktualisation , das heißt er wird driften und somit drifted plan
> stand auch."
> → **„saeite ganz weg, alles im repo + Skill bauen, der automatisch die
> offenen fragen nach und nach an mich stellt und gleich einarbeitet in den
> plan"**

## Warum sie gefallen ist

Nicht wegen ihrer Gestalt, sondern wegen ihrer **Kette**. Jedes Glied war
Handarbeit: `hub.json` fortschreiben → `hub_sync.py senden` → Seite. Vergaß es
eine Session, blieb der Stand still falsch, und nichts schlug an. Gemessen am
22.08.: Seite 9/34, lokal 11/34, real 12/34 bei 13 Commits Drift. Am 23.08.
meldete eine Automatik „kein Diff" — richtig gemessen und trotzdem irreführend,
weil sie Blatt gegen `hub.json` verglich statt gegen die Wirklichkeit.

## Was an ihre Stelle getreten ist

| Was die Seite konnte | Jetzt |
|---|---|
| Plan anzeigen | `docs/PLAN-STAND.md` — **gerechnet** aus Manifesten und Urteilsmarken, vom Hook frisch gehalten |
| Fragen asynchron beantworten | Skill `/fragen` — stellt die Karten aus `docs/plan/fragen.json` einzeln im Chat, hält die Antwort wörtlich fest |
| Bilder zeigen | `docs/plan/bilder/`, direkt im Editor sichtbar |

Einstieg: `docs/plan/LIES-MICH.md`. Der letzte Stand der alten Datenquelle
liegt vollständig unter `docs/archiv/hub-2026-08-23.json`.

## Wenn jemand den Ordner wiederbeleben will

Zwei Dinge vorher lesen: den Registereintrag vom 23.08. in `CLAUDE.md` (die
Abschaffung ist User-Wort, kein Vorschlag) und NAK-61 in
`docs/offene-punkte.md` (ein Typfehler in `app/briefing-app.tsx:963`, der schon
vor der Stilllegung dastand und nie behoben wurde).
