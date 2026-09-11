# Auftragsvorlage `/lebenslauf-audit` (Prüfsystem, Befundklassen K1 und K2)

Nach `docs/gesundheit/KONZEPT.md` §4.3 und §5. Der Dirigent (oder eine
Konzeptsession) füllt die Platzhalter und startet die Prüfsession; der
Auftrag wird nie frei formuliert. Erste Ausführung 12.09.2026 in `nakama-d8`
(Bericht `docs/gesundheit/lebenslauf-audit.md`).

## Frage

Würde die nächste Queue, der nächste Thread, der nächste Callback still
gegen Prüfliste A oder B verstoßen? Gemessen wird die **Population**, nicht
eine Stichprobe.

## Platzhalter

- `{{BASIS}}`: Git-Stand, auf dem gelesen wird (Arbeitskopie, nie der
  laufende Worktree).
- `{{AUSSCHLUESSE}}`: im Register datierte Härtungen zu K1/K2 (Kennungen).
- `{{VORLAUF}}`: Pfad des vorigen Berichts, gegen den gedifft wird.

## Population (Leser, mechanisch, Sonnet oder Opus)

Leser Broker: `broker/src/transport/`, `broker/src/transport/server_v3/`,
`broker/src/store/`, `broker/src/coordinator/`, `broker/src/server.rs`,
`broker/src/lebenslauf.rs`. Leser Plugin: `eq-copilot/plugin/core/ipc/`,
`eq-copilot/plugin/core/StampedAudioQueue.h`, `eq-copilot/plugin/src/PipeClient.cpp`,
`eq-copilot/plugin/src/PluginProcessor.cpp`. Tests: `broker/tests/`,
Inline-Tests `tests_*.rs`, `eq-copilot/plugin/tests/`.

Je Objekt eine Zeile der Matrix:

| Spalte | Zusage (Prüfliste) | Zulässige Werte |
|---|---|---|
| Politik bei voll | A: je Klasse ausdrücklich abweisen, koaleszieren, ältesten ersetzen | abweisen · koaleszieren · ersetzen · unklar |
| Abfluss ohne Reconnect | A: ein Puffer, der später wiederholt, hat einen Abflussweg ohne Reconnect | ja · nein · unklar |
| Schlüssel überleben Puffer | A: Snapshot-/Objektidentität überlebt jeden Zwischenpuffer | ja · nein · n. a. |
| Join mit Frist | B: jeder Join hat eine Frist | Frist ja · fristlos begründet · fristlos unbegründet · kein Join |
| Close-Flag vor Inhalt | B: nach dem Schließen nichts mehr liefern, Flag vor Inhalt | ja · nein · n. a. |
| Callback-Besitz | B: Callbacks nie auf dem gejointen Thread, oder Stop von innen erlaubt und getestet | geklärt (Ort) · Self-Join möglich · unklar |
| Stopp-Fenster | B: Registrierung nach begonnenem Stop ist ein Test | Test (Ort) · keiner |
| Test | A/B: dieselbe Regel in allen Sprachen, beidseitig gemessen | Datei:Zeile · keiner |

Jeder Leser endet mit einem Scope-Beweis (gelesene Pfade, Dateizahl,
fehlende Pfade). Ohne Scope-Beweis gilt der Bericht als nicht gelaufen.

## Skeptiker (je Zelle „unbegründet", „nein", „unklar", „keiner")

Auftrag: die Zelle widerlegen. Finde den Test, den Absichtskommentar, die
Garantie (Stop-Flag vor Join, begrenzte Laufzeit) oder die Registerzeile.
Urteil je Zelle: widerlegt · bleibt · teilweise, mit `Datei:Zeile`.

## Urteil (Hauptlauf der Prüfsession, nie delegiert)

- **Defekt**: die Zelle bricht eine Zeile der Verhaltensmatrix eines
  abgenommenen Tickets, einen Gate-Text oder eine Invariante aus
  `CLAUDE.md`. Wird ein Ticket nach dem Muster NAK-246.
- **Härtung**: Prüfliste verlangt es, keine Zusage tut es. Registerzeile
  [Härtung/Struktur] oder [Härtung · Test] mit K-Kennung.
- **Lücke**: weder Matrix noch Entwurf sagen etwas; benennen, keine
  Nacharbeit.

## Bericht

`docs/gesundheit/lebenslauf-audit.md`, kanonisch, überschrieben. Kopf:
Urteil, Befundliste mit K-Kennung und Klasse, Zähler (Objekte, mit Test,
Zellen offen). Darunter die Matrix mit Evidenz und die Skeptikerurteile.
Der Dirigent liest nur den Kopf.

## Ausschlüsse

`docs/**` ist keine Befundfläche. `tools/dirigent/pruefliste.md` ist
Kontext, keine Anforderungsquelle. Befunde gegen Prüfwerkzeuge, die
absichtliche Sabotage voraussetzen, sind Härtung. `{{AUSSCHLUESSE}}`.
