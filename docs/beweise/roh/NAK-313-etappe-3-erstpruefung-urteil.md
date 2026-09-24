# NAK-313 — Erstprüfung Etappe 3, Rohurteil (Codex gpt-6-astra, Effort xhigh, lesend; Vorlage A)

**Thread:** 01a0d209-b458-7091-873c-5b88618b8783 · **HEAD vor und nach dem Lauf:** 53caac1ec253f84aa48ee24c6064e5374f2466c2 (Code gleich 097bb663; seit 9491b3ce nur docs/**) · **Lauf:** 24.09.2026, 08:10:46 bis 08:17:59 (+02:00, aus der Startdatei des Laufs `tools/dirigent/codex-lauf.ps1`, Kennung `nak313-e3p1`) · **Prüfbereich:** git diff f77738a0...9491b3ce -- eq-copilot/plugin/state/NakamaState.cpp eq-copilot/schemas/state/nakama-state-v2.md eq-copilot/plugin/tests/StateMigrationTestMain.cpp tools/eq-copilot/erzeuge_state_fixtures.py eq-copilot/fixtures/state/MANIFEST.json eq-copilot/fixtures/state/schema2/doppelte-eigenschaft-v1.bin tools/beweise.ps1 (Etappe 3; Prüffragen (a) bis (f)) · **Auftrag:** docs/beweise/roh/NAK-313-etappe-3-erstpruefung-auftrag.txt · **Ablage:** letzte Nachricht des Prüfers (read-only-Sandbox), unverändert vom Dirigenten abgelegt. Der Lauf führte 25 lesende Kommandos aus (26 abgeschlossene Einträge im JSONL). Die letzte Nachricht trägt je Prüffrage (a) bis (f) einen Absatz mit Beleg am ZIEL.

---

URTEIL: PASS — kein DEFEKT im gebundenen T2-Prüfbereich.

(a) Geprüft: serialisierter Kandidat durch denselben Riegel, ausschließlich `verlustfrei` akzeptiert; unveränderte Grenzen 16 MiB/64/65 536/262 144; maximale Userfelder und Klassen-/Positionsschleife in `eq-copilot/plugin/state/NakamaState.cpp:1286-1428`; G = 258 412 beziehungsweise 258 413 aus 3 729 zusätzlichen Bestandseinträgen und fehlenden Common-Feldern nachvollzogen; beide Ränder in `eq-copilot/plugin/tests/StateMigrationTestMain.cpp:977-1148`

(b) Geprüft: allokationsfreie Vor-/Nachzählung über dieselben Ebenen, begrenzte Rekursion und fail-closed; bekannte Wurzeln vor Migration read-only mit Duplikatgrund und leerem Baum, fremde ignoriert (`NakamaState.cpp:289-381,2306-2349`); symmetrischer M-34-Rotbeweis D5 mit ausschließlich M-34(a) und Abschnittsurteil gefallen, M-33 unverändert grün

(c) Geprüft: Originalbytes-Rückgabe (`NakamaState.cpp:2293-2303,2352-2362`), unveränderte Erhaltung unbekannter Baumteile, unveränderte State-Version und bestehende Goldens; Negativ-Golden aus erklärtem Writer-Mutanten (`StateMigrationTestMain.cpp:2051-2087`), Registrierung und Verwaistenprüfung (`tools/eq-copilot/erzeuge_state_fixtures.py:1094-1178`); selbst gemessener Golden-SHA passend zum MANIFEST: `bc49816e3739ebbfdc3ad1c1205be49a9fd47bc00c73234d139989ef49b8f4fd`

(d) Geprüft: drei Vertragspräzisierungen mit passenden Zahlen und Duplikatregel (`eq-copilot/schemas/state/nakama-state-v2.md:58-59,286,297-307`); selbst nachgemessen 323 Zeilen an BASIS und ZIEL, CRLF/-text und ausschließlich sieben geänderte Zeilen

(e) Geprüft: deterministische Erwartungen ohne Zeitgrenzwert, Prozessoren auf dem Heap, Rotbeweisketten M-17 bis M-38 gegen Prüfbedingungen abgeglichen; B2-/A12-Behauptungen passend zum Messumfang (`tools/beweise.ps1:576,693`); vorhandener Kanon 70/70, B2 371 Prüfungen und A12 71 Dateien ausschließlich als gelesene Laufbelege

(f) Geprüft: Ausnahmebehandlung durch `liesJuceBaum` verhaltensgleich (`NakamaState.cpp:341-350,2313-2315`); weiterhin ausschließlich die beiden Produkteintritte in `eq-copilot/plugin/src/prozessor/State.cpp:146` und `eq-copilot/plugin/sonde/SondeProcessor.cpp:1259`; Identitäts- und Callback-Dateien unverändert

Selbst nachgemessen: Diff, Quell-/Golden-Hashes, Zeilenzahl, Zeilenenden, Status und HEAD; keine Dateien verändert

Nicht ausgeführt: Compiler, Tests, Python, cargo oder FL Studio gemäß Prüfauftrag; daher keine eigene Laufzeit-, Nulltest- oder Allokationsmessung

HEAD Beginn: `53caac1ec253f84aa48ee24c6064e5374f2466c2`
HEAD Ende: `53caac1ec253f84aa48ee24c6064e5374f2466c2`
Status unverändert: ausschließlich die drei bereits vorhandenen untracked Pfade

FERTIG Erstprüfung Etappe 3, PASS, 0 Defekte, 0 Lücken, 0 Härtungen
