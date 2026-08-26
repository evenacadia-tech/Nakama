---
name: fragen
description: Stellt dem User die offenen Fragen aus docs/plan/fragen.json — eine nach der anderen, mit Bild wo es um Sichtbares geht — hält seine Antwort WÖRTLICH mit Datum fest und arbeitet die Folge sofort in Plan, Register und Tickets ein. Aufruf ohne Argument (nächste offene Frage) oder mit Kennung (z. B. /fragen U9).
---

# /fragen — die offenen Punkte abarbeiten, einer nach dem anderen

Bis zum 23.08.2026 lagen die Fragen an den User auf einer Briefing-Seite, die
er besuchen musste und die niemand automatisch aktualisierte. Er hat sie
abgeschafft: **„saeite ganz weg, alles im repo + Skill bauen, der automatisch
die offenen fragen nach und nach an mich stellt und gleich einarbeitet in den
plan"**. Das bist du.

Zwei Wörter aus diesem Satz sind der ganze Auftrag:

- **„nach und nach"** — du legst ihm nicht zehn Karten auf einmal hin. **Eine
  Frage je Runde.** Erst wenn sie eingearbeitet und committet ist, kommt die
  nächste. Er darf jederzeit abbrechen; alles Beantwortete ist dann schon fest.
- **„gleich einarbeitet"** — eine Antwort ist erst erledigt, wenn sie an ihrem
  Zielort steht und committet ist. Nicht am Ende der Session, nicht „merke ich
  mir": **im selben Zug**.

## 1. Was du liest, bevor du fragst

| Datei | wozu |
|---|---|
| `docs/plan/fragen.json` | `offen[]` = die Karten · `beantwortet{}` = 35 frühere Antworten im Wortlaut |
| `docs/plan/bilder/` | die Bilder zu den Karten (`bilder[].datei`) |
| `CLAUDE.md` (Register) | was schon entschieden ist — **frag nie etwas, das dort steht** |
| `docs/PLAN-STAND.md` | wo der Plan steht (gerechnet, nie von Hand) |

🔑 **Prüfe vor jeder Frage, ob sie noch offen IST.** Zwei Karten (`U5`, `U10`)
tragen `frueher_beantwortet` — zu ihrer Kennung liegt schon ein Wortlaut vor.
`U10` ist im Register vom 22.08. entschieden *und* mit SONDE-008 gebaut. Eine
Frage zu stellen, die der User längst beantwortet hat, kostet Vertrauen. Zeig
ihm in dem Fall den alten Wortlaut und frag nur, ob die Karte damit zu ist.

## 2. Welche Frage als Nächstes

In dieser Reihenfolge:

1. Hat der User eine Kennung genannt (`/fragen U9`) — die.
2. `dringlichkeit: "jetzt"` vor `"wenn du dazu kommst"` vor `"später"`.
3. Bei Gleichstand: die, die **etwas freischaltet**. Eine Karte, an der ein
   Ticket hängt (`U9` blockiert G3, `U11` ist Termin A2), geht vor einer, die
   nur aufräumt (`U12`).
4. Sag ihm in einem Satz, warum gerade diese — und wie viele noch offen sind.

## 3. Wie du fragst

**Zeigen, nicht beschreiben.** Trägt die Karte `bilder`, zeig sie **zuerst**
direkt in der Sitzung: `Read` auf den absoluten Pfad
`docs/plan/bilder/<datei>` — Bilder erscheinen nativ im Claude-Kanal, auch in
Remote Control — und nenne den `text` der Karte dazu. Der User ist Gestalter —
er entscheidet an Bildern, nicht an Beschreibungen. (Nimbalyst ist seit
26.08.2026 vollständig deinstalliert; es gibt keine `mcp__nimbalyst__*`-
Werkzeuge und keinen Ersatzbus. Gefragt wird direkt hier.)

Dann die Frage, wo verfügbar über das native `AskUserQuestion`, sonst als
klar abgesetzte Einzelfrage im Text:

- **Ein Entscheid mit 2–4 klaren Wegen** → `AskUserQuestion`. Deine Empfehlung
  steht an erster Stelle und trägt „(Empfohlen)".
- **Mehrere Folgefragen einer Karte** (`U2`, `U6`) → ein
  `AskUserQuestion`-Aufruf mit bis zu vier Fragen; sind es mehr, in derselben
  Runde aufteilen. Das ist die eine erlaubte Ausnahme von „eine Frage je
  Runde": es ist **eine Karte**.
- **Ein Handgriff** (`U7`, `U11`, `U12`, `U13`) → keine Meinungsfrage. Sag,
  was zu tun ist, was du vorbereitet hast, und frag mit zwei klaren Wegen
  (erledigt / bleibt liegen), wie es steht.

Regeln für den Text der Frage:

- Alltagssprache. Der User ist Projektleiter, kein Programmierer.
- Kein Fachwort ohne Übersetzung, keine Ticketnummer ohne Klartext daneben.
- **Keine technische Entscheidung vorlegen.** Bauform, Dateipfad, Bibliothek,
  Schwellwert = deine Sache. Ihm gehört die Richtung: Aussehen, Verhalten,
  Reihenfolge, was ihm wichtig ist.
- Sag dazu, **was passiert, wenn er sich nicht entscheidet** — oft ist das die
  eigentliche Information.

## 4. Wie du die Antwort festhältst

Die Antwort ist **User-Wort**. Sie wird **wörtlich** übernommen, nie
zusammengefasst, nie geglättet, auch nicht bei Tippfehlern.

An **drei** Orten, in dieser Reihenfolge:

1. **`docs/plan/fragen.json`** — die Karte wandert von `offen` nach
   `beantwortet` mit `{"wahl": …, "text": <Wortlaut>, "datum": "JJJJ-MM-TT HH:MM",
   "status": "eingearbeitet", "ergebnis": <was daraus folgt, ein Satz>}`.
2. **Das Register in `CLAUDE.md`** — nur bei einem **Entscheid**: eine Zeile
   `| TT.MM. | <was gilt> | „<Wortlaut>" |`. Ohne Zitat kein Registereintrag;
   ohne Registereintrag gilt nichts als entschieden.
3. **`design/abnahmen/JJJJ-MM-TT-<thema>.md`** — wenn es um Aussehen oder
   Verhalten der drei Apps geht (`U2`, `U5`, `U6`, `U9`). Dort steht der volle
   Wortlaut mit der Frage, die er beantwortet hat.

Widerspricht die Antwort einem älteren Registereintrag: den alten **nicht
löschen**, sondern als überholt markieren (`~~…~~` + „überholt TT.MM.") und den
neuen darunter setzen. Die Historie ist Teil der Wahrheit.

## 5. Wie du sie einarbeitest

„Gleich einarbeitet in den plan" heißt: die Folge der Antwort landet dort, wo
gearbeitet wird — **im selben Änderungssatz**:

| Die Antwort … | … landet in |
|---|---|
| ändert, **was** gebaut wird | `docs/plan/plan.json` (Text des Schrittes) + `docs/bauaufteilung-sonden.md` |
| ändert, **wie** etwas aussieht/sich verhält | `design/abnahmen/` + der betroffenen Spezifikation unter `design/docs/` |
| erledigt einen Handgriff | die Stelle, die ihn führte (`docs/offene-punkte.md` mit NAK-ID, `docs/NEXT-SESSION.md`) |
| macht einen offenen Punkt gegenstandslos | `docs/offene-punkte.md` — schließen **mit ID + Commit-SHA**, nie still löschen |

Der Planstand selbst wird **nicht** angefasst: `docs/PLAN-STAND.md` ist
gerechnet (`tools/plan/planstand.py`), und der Hook zieht ihn selbst nach.

## 6. Committen

Ein Commit je Karte, mit **explizitem Pathspec** (parallele Sessions!):

```
git add docs/plan/fragen.json CLAUDE.md <weitere betroffene Pfade>
git commit -m "Antwort U9 eingearbeitet: <was jetzt gilt> — User-Wort 23.08."
```

Nie `git add -A`, nie `--amend`. Der Auto-Push-Hook schickt es raus.

## 7. Wann du aufhörst

- Der User sagt „gut", „fertig", „reicht" → **sofort** aufhören, nichts mehr
  fragen, den Stand melden (wie viele beantwortet, wie viele bleiben).
- Alle Karten sind zu → sagen, dass nichts mehr offen ist.
- Eine Antwort wirft eine neue Frage auf → als **neue Karte** in `offen`
  anlegen (mit `seit`-Datum), nicht sofort nachhaken. „Nach und nach."

## 8. Was du nie tust

- Eine Antwort zusammenfassen statt zitieren.
- Eine Frage stellen, die im Register schon beantwortet ist.
- Aus einer Antwort mehr ableiten, als dasteht — im Zweifel nachfragen, welche
  Lesart gilt, statt eine zu wählen.
- Eine technische Entscheidung als Frage verkleiden.
- Eine Karte selbst schließen, weil sie dir erledigt vorkommt.
