URTEIL: NEEDS_WORK — 3 Defekte, 0 Lücken, 0 Härtungen

# NAK-312 S25k, Etappe 7b Teil 1 (Sätze 1 und 3) — Erstprüfung T2

- Prüfer: frischer, lesender Codex-Thread `01a0cc61-692c-7c12-aa17-5e19d483cf9b`, `gpt-6-astra`, Effort xhigh, Sandbox read-only, abgekoppelt (Kennung `NAK-312-7b1-erst`, `tools/dirigent/codex-lauf.ps1`), 23.09.2026, 05:48:50 bis 05:59:29 Uhr, `EXIT=0`; weder Bauer noch Prüfer oder Validierer einer früheren Runde.
- Prüfbereich: `git diff e1bd9c6d2de8ddbea9f81ce3a2318ec657a4f184...19bb1803e15d252fd503459d94ff4f3d05a2ee35` über die 18 Pfade des Auftrags `docs/beweise/roh/NAK-312-etappe-7b-teil1-erstpruefung-auftrag.txt` (Vorlage A; Gate-Text wörtlich im Auftrag).
- HEAD zu Beginn: `f636d1a3e6e6bfe2ebd865927cb941090b046776`. HEAD am Ende: `f636d1a3e6e6bfe2ebd865927cb941090b046776` (Startlog und Schlussausgabe).
- Rohdaten: JSONL `nakama-NAK-312-7b1-erst-f636d1a3.jsonl` im Temp-Ordner (940 KB, nicht im Repo). Der Wortlaut der Schlussausgabe steht unverändert unten; die Kennungen D-1 bis D-3 vergibt der Dirigent in der Reihenfolge des Prüfers (D-1 Befundzähler nach dem Nachrücken, D-2 Prüfdeckung M-130, D-3 B15-Behauptungszeile).

## Wortlaut der Schlussausgabe

URTEIL: NEEDS_WORK

Der Änderungssatz enthält einen Produktfehler und zwei Defekte der zugesagten Prüfdeckung.

Geprüft: vollständiger Diff der 18 vereinbarten Pfade, Matrix, Statevertrag, Tests, Behauptungszeilen und relevante Aufrufpfade; selbst nachgemessen wurden neun identische Golden-Blobs, der passende neue Golden-Hash und unveränderte Sonde-, DSP-, Identitäts- und v3-Pfade; Annahmeregel, Stilllegung, Sperrenordnung und Statewege wurden statisch geprüft.

Nicht geprüft: keine Builds, Produktprozesse, Laufzeit- oder Bildtests gestartet; Kanon, Rotbeweise und Vorstandslauf nur gelesen, keine eigenen Laufzeiterfolge behauptet; keine Dateien verändert.

HEAD Beginn: f636d1a3e6e6bfe2ebd865927cb941090b046776
HEAD Ende: f636d1a3e6e6bfe2ebd865927cb941090b046776

FERTIG Erstprüfung Etappe 7b Teil 1, NEEDS_WORK, 3 Defekte, 0 Lücken, 0 Härtungen

Full review comments:

- [P2] Aktualisiere den Befundzähler nach dem Nachrücken — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/src/SourcesModel.cpp:476-477
  DEFEKT gegen die ehrliche Zustandsanzeige aus [CLAUDE.md:44–45](CLAUDE.md#L44-L45): Bei 21 gespeicherten Quellen kann X zunächst warten, während eine angenommene Quelle Y offline ist. Enthält der Snapshot bereits einen offenen Befund für X und wird Y anschließend lokal entfernt, rückt X hier mit `findingsOffen == 0` nach. Wartende Quellen wurden zuvor absichtlich nicht gezählt; nach dem Einfügen fehlt jedoch `zaehleOffeneFindings()`, und auch `tick()` korrigiert den Zähler nicht. Bei Auswahl von X behauptet der Editor deshalb bis zum nächsten Snapshot „no findings yet“, obwohl der Befund vorhanden ist. Nach dem Nachrücken die Zähler erneut ableiten. Lesebeleg für diese reproduzierbare Folge: `git show 19bb180:eq-copilot/plugin/src/SourcesModel.cpp`.

- [P2] Prüfe M-130 bereits nach dem ersten erlaubten Block — C:/Users/phili/Projekte/Nakama/eq-copilot/plugin/tests/MarkierungTestMain.cpp:2298-2300
  DEFEKT in der Prüfdeckung von [M-130](docs/beweise/NAK-312.md#L9154): Nach `prepareToPlay` verarbeitet der Test zunächst 40 erlaubte Blöcke und verlangt anschließend lediglich `hoerbar > 0` sowie ein neues `begin`. Ein versehentlich erst im zweiten oder späteren Block beginnender Auftrag erfüllt sämtliche Bedingungen ebenfalls, obwohl M-130 ausdrücklich den ersten erlaubten Block zusagt. Der vorhandene Rotbeweis prüft nur den Verlust des Auftrags, nicht diesen Zeitpunkt. Den ersten Block separat verarbeiten und dessen Beginn beobachten, bevor die restlichen Blöcke folgen. Lesebeleg: `git show 19bb180:eq-copilot/plugin/tests/MarkierungTestMain.cpp | Select-Object -Skip 2268 -First 39`.

- [P2] Beschränke die B15-Behauptung auf den tatsächlichen Messumfang — C:/Users/phili/Projekte/Nakama/tools/beweise.ps1:752-752
  DEFEKT gegen Prüfauftrag (d): Der neue Nachsatz „die bisherigen Bilder bleiben bytegleich“ wird von B15 nicht gemessen. `schreibeShot` überschreibt die PNGs und prüft lediglich erfolgreiches Schreiben sowie Fenstermaße; die neuen Pixelvergleiche betreffen Diagnosefeld und Sources-Spalte, nicht die bisherigen Bilder gegen eine Referenz. Eine versehentliche Änderung etwa des Titeltexts könnte daher unbemerkt bleiben. Der einmalige Bildvergleich des Bauers ersetzt keinen Vergleich im jeweiligen Kanonlauf. Den Nachsatz entfernen oder einen echten Referenzvergleich anschließen. Lesebeleg: `git show 19bb180:eq-copilot/plugin/tests/ShotTestMain.cpp | Select-Object -Skip 118 -First 180`.
