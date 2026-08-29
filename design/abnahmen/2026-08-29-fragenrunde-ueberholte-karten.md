# 2026-08-29 — Fragenrunde: überholte Karten geschlossen, U10 bestätigt, U12 freigegeben, U11 vertagt

Gestellt in der Dirigenten-Sitzung (User-Wort davor: „NE STELLE Mir die fragen
jetz"). Vier Fragen in einer Runde, Antworten als Auswahl wörtlich.

## U2 · U6 · U9 — überholt durch den Rework-Entscheid vom 25.08.

**Frage:** U2, U6 und U9 (Gen-Studie 04, Suna-Kachel, Figma-gegen-Entwurf):
Ihre offenen Folgefragen betreffen die alten Figma-Exporte. Dein Entscheid vom
25.08. („drei Bilder sind die aktuelle Wahrheit, alle früheren Lesarten sind
Verlauf") und die Suna-Stilllegung vom 28.08. haben sie überholt; technische
Fragen entscheidet ohnehin der Entwurf. Karten damit schließen?

**Antwort (Auswahl):** „Ja, schließen (Empfohlen)"

**Folge:** Die zwölf Folgefragen aus `2026-08-22-hub-antworten-35.md`
Abschnitt 6 (U2.1, U2.4, U2.6, U2.8, U2.9, U6.4, U6.5, U6.6, U6.8,
U9.1-Folge, U9.3, U9.4) sind gegenstandslos. Sichtbare Zustände, Wortlaute,
Rollenwörter und Arbeitsschritte werden beim Rework aus den drei Bildern unter
`design/assets/rework-basis-2026-08-25/` gelesen (Leseregel in
`2026-08-25-rework-referenz-drei-designs.md`); technische Mengen (Rollenliste,
Messzustände der Engine) aus dem Entwurf (Rangfolge U9.1, 22.08.). Was die
Bilder nicht zeigen, wird beim Rework als konkrete Lücke einzeln vorgelegt —
keine Sammelkarte mehr. NAK-38 (UI-Rückfluss-Liste) wird entsprechend
nachgetragen.

## U10 — Hör-Markierung nur mit Transport-Signal

**Frage:** U10 (Hör-Markierung ohne Transport-Signal): Du hast am 22.08. schon
geantwortet — „Nein, nur mit Signal". Ist die Karte damit zu?

**Antwort (Auswahl):** „Ja, zu (Empfohlen)"

**Folge:** Es gilt das Wort vom 22.08. Die Umsetzung im Code (das heutige
fail-open bei fehlender Transport-Information) ist Technik und wird vom
Dirigenten gegen `PluginProcessor.cpp` geprüft; Registerpunkte NAK-35/NAK-24.

## U12 — alter Design-Ordner und altes GitHub-Repo

**Frage:** U12: Der alte Ordner Projekte\Nakama-Design und das GitHub-Repo
evenacadia-tech/Nakama-Design sind seit 22.08. vollständig im Nakama-Repo
enthalten. Soll ich jetzt aufräumen?

**Antwort (Auswahl):** „Ordner löschen, Repo archivieren (Empfohlen)"

**Folge:** Lokaler Ordner gelöscht (vorher gemessen: Arbeitsbaum sauber, kein
ungepushter Commit, letzter Commit `5c2c4f6` „ZUSAMMENGEFÜHRT nach
Nakama/design/"); GitHub-Repo archiviert, nicht gelöscht. Ausführung und
Ergebnis stehen in `docs/plan/fragen.json` (U12, `ergebnis`).

## U11 — FL-Termin A2

**Frage:** U11 (FL-Termin A2, ca. 20 Minuten in FL): Nebenwege einmal mit
einem verzögernden Plugin messen — schaltet später das Entmaskieren und den
Vorher/Nachher-Vergleich frei (erst ab Phase P6/P8 nötig). FL ist gerade
offen. Jetzt machen?

**Antwort (Auswahl):** „Später (Empfohlen)"

**Folge:** Karte bleibt offen, vertagt am 29.08.; wird wieder vorgelegt, wenn
P6 näher rückt (NAK-43/NAK-44 unverändert).

## Nicht gestellt

- **U13** (FL-Kurztermin C): heute nicht ablesbar — der Zähler für
  Kontinuitätsbrüche hat noch keine Anzeige (NAK-57). Das ist eine technische
  Lücke des Dirigenten, keine Frage an den User; die Karte trägt jetzt den
  Blocker.
- **U8** wurde heute früher beantwortet (Installation, FL-Termin).
