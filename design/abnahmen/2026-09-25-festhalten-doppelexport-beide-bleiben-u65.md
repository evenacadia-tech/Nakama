# 2026-09-25 — Zweimal „Festhalten“ in derselben Sekunde: beide Messstände bleiben erhalten (Karte U65)

**Frage (Dirigent, 25.09.2026, in der Dirigentensession `e6e40a0d` über
`AskUserQuestion`, gestellt im Zeitfenster nach Skill `/fragen U65`):**
Im Fenster einer Gen-Instanz, die nicht als Hauptprogramm läuft, sichert der
Knopf „Festhalten“ den aktuellen Messstand als Datei in einen gemeinsamen
Ordner; der Dateiname besteht nur aus Datum, Uhrzeit auf die Sekunde und dem
Namen der Instanz. Entsteht in derselben Sekunde ein zweiter Export mit
demselben Namen (schneller Doppelklick, zwei Instanzen mit gleichem Namen,
zwei ohne Namen), ersetzt die zweite Datei heute die erste ohne Hinweis.
Angeboten: „Beide bleiben erhalten (Empfohlen)" — die zweite Datei bekommt
einen kleinen Zusatz im Namen, die Meldung nennt genau die Datei, die
entstanden ist, nichts wird ersetzt oder gelöscht; „Zweite ersetzt die erste,
mit Hinweis" — die Meldung sagt ausdrücklich „ersetzt den Export von
19:15:07", bei zwei Instanzen erfährt es nur die zweite; „Zweiter Export wird
abgelehnt" — Meldung „In dieser Sekunde gibt es schon einen Export mit diesem
Namen – bitte gleich noch einmal.", die vorhandene Datei bleibt, der zweite
Klick zählt nicht. Ohne Antwort wäre nach NAK-313 Etappe 7 der zweite Export
abgelehnt worden und die erste Datei geblieben.

**User-Wort, 25.09.2026, 11:26 Uhr** (gewählte Antwort, wörtlich; Zeitpunkt
mit `Get-Date` unmittelbar nach dem Eingang gemessen):

> „Beide bleiben erhalten (Empfohlen)"

## Was damit festliegt

- **Weg 1 gilt als Produktentscheid:** Bei einer Namenskollision des
  Messstand-Exports (gleiche Sekunde, gleicher bereinigter Instanzname)
  bleiben beide Dateien erhalten; die zweite bekommt einen eindeutigen Zusatz
  im Namen, und die Erfolgsmeldung nennt genau die Datei, die entstanden ist.
  Nichts wird ersetzt, nichts automatisch gelöscht. So legt Nakamas
  Hintergrunddienst seine eigenen Berichte im selben Ordner schon heute ab.
- Wie der Zusatz gebildet wird (Zähler, Millisekunden oder Kurzkennung), wie
  die Kollision erkannt wird (exklusives Anlegen, Umbenennen ohne Ersetzen
  nach Regel R-313-10) und wie der Fall getestet wird, ist Technik und bleibt
  beim Dirigenten: Etappe 8 des Tickets NAK-313 (Planschritt S25l) mit
  Matrixnachtrag, Rotbeweisen, Kanon und Erstprüfung 8
  (`../../docs/beweise/NAK-313.md` §3 Karte U65, §4 Etappenplan, §36
  Übergabe, §37 Antwort).
- Bis Etappe 8 gebaut ist, gilt das Verhalten nach Etappe 7: der zweite
  Export wird abgelehnt, die erste Datei bleibt (R-313-10, `SnapshotExport`
  `abgelehnt`); ein abgebrochenes Schreiben wird als Fehler gemeldet, nie als
  Erfolg.
- Karte U65 in `../../docs/plan/fragen.json` ist beantwortet; Register
  NAK-313 in `../../docs/offene-punkte.md` trägt den Entscheid;
  `../../docs/ZIELBILD.md` führt ihn unter „Im Detail festgelegt“.
