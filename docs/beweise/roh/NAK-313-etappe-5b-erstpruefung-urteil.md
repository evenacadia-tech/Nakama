# NAK-313 — Erstprüfung Teiletappe 5b, Rohurteil (Codex gpt-6-astra, Effort xhigh, lesend; Vorlage A)

**Thread:** 01a0d472-b101-7ca3-bbfe-4b2456de1844 · **HEAD vor und nach dem Lauf:** 97872e87c089170f5d38c7e9189d850632e7e651 (Code gleich ce83e67c; seit b40a1bf0 nur der Prüfauftrag unter docs/**) · **Lauf:** 24.09.2026, 19:24:39 bis 19:39:08 (+02:00, aus der Startdatei des Laufs `tools/dirigent/codex-lauf.ps1`, Kennung `nak313-e5bp1`) · **Prüfbereich:** git diff 062a98fc...b40a1bf0 auf die 18 Ticketpfade der Teiletappe 5b (Prüffragen (a) bis (h)) · **Auftrag:** docs/beweise/roh/NAK-313-etappe-5b-erstpruefung-auftrag.txt · **Ablage:** letzte Nachricht des Prüfers (read-only-Sandbox), unverändert vom Dirigenten abgelegt. Der Lauf führte 60 lesende Kommandos aus; die letzte Nachricht trägt je Prüffrage (a) bis (h) einen Absatz mit Beleg am ZIEL, die eigenen Nachmessungen, HEAD-Werte, die FERTIG-Zeile und einen Befund.

---

URTEIL: NEEDS_WORK — ein DEFEKT im ausdrücklich beauftragten Belegabgleich, kein nachgewiesener Produktfehler.

(a) Geprüft am ZIEL: gemeinsame Produktkonstante als juce::int64 in eq-copilot/plugin/state/NakamaState.h:54, abgeleitete Transaktionsgrenze in NakamaTransaktion.h:57; relevante Produktprüfungen verwenden diese Grenze, unabhängige Zahlenliterale der Tests dienen dagegen bewusst als Vertragsorakel;

(b) Geprüft: fünf Leser mit vorgelagerter Typ-/Untergrenzenprüfung in NakamaState.cpp:1482–1488, 1550–1554, 1891–1906, 2048–2066 und 2115–2125; gemeinsamer Main-/Retained-Leseweg, atomare Übernahme und Originalbytes-Rückweg in :2230–2251 und :2308–2379; B2-Grenzvektoren und Bytevergleiche in eq-copilot/plugin/tests/StateMigrationTestMain.cpp:1373–1519, angepasster NAK-283-Rand in :3993–4079; State-Version, Migrationen und Goldens unverändert;

(c) Geprüft: Mutationsschranken vor Zuweisungen in NakamaState.cpp:2415–2423, 2470–2478 und 2886–2894; Headroomkandidat mit begrenzten Revisionen und weiter aktivem Byte-Vorleser in :1272–1428; unveränderte Transaktionsprüfungen in NakamaTransaktion.cpp:247 und :528–530 vor dem Commit-Punkt :656; B7 misst Grenzcommit, erschöpfte Revision und abgewiesenen Ladestart in TransactionTestMain.cpp:6624–6668; B27 misst den letzten Bestandsschritt und die Eintragsabweisung, nicht den behaupteten letzten Eintragsschritt;

(d) Geprüft: Vorprüfung unter bindungMutex vor beginneVersuch sowie Abbruch nach abgewiesener Verknüpfung in eq-copilot/plugin/src/prozessor/Analyse.cpp:1291–1341; öffentliche Bibliotheksfunktion mit Offen-/Kennungsprüfung, No-op und einmaligem geprüften Inkrement in NakamaState.cpp:3072–3097; Testhaken ausschließlich im Testbereich von PluginProcessor.h:1087, keine Aktivierung für Produktziele; eingerichtete Versuchsbühne und M-84 bis M-87 in Sonde014AssistentTest.cpp:315–368 und :1430–1544; beginneVersuch und brichVersuchAb weiterhin außerhalb von bindungMutex;

(e) Geprüft: unveränderte uint64-Ziffernausgabe der Revision in eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:165 und Intern.h:429, jsonSafe für Sequence/Zähler beibehalten; B10 vergleicht beide Sender bei beiden Grenzwerten in IpcTestMain.cpp:3812–3833; bestehende Empfängergrenzen in Intern.h:195–211 und broker/src/coordinator/liveness.rs:559–577 weiterhin wirksam;

(f) Geprüft: deterministische Handinstanz samt Selbstprüfung und Hashbildung in tools/eq-copilot/erzeuge_v3_fixtures.py:3793–3872, Byteprüfung in :5120–5149; C++-Writervergleich in Sonde014IntentTest.cpp:1919–1945 und Rust-P1-Verarbeitung in broker/tests/sonde014_verdrahtung.rs:4948–4970; selbst gemessener Fixture-SHA-256 672111c85015ee449095e88491da0c27306160917b5a4f005f65895fd00ab17e stimmt mit MANIFEST überein; bestehende Fixtures und Schema-Dateien unverändert, Handinstanz mit -text;

(g) Geprüft: Kurzform, Tabelle und datierte Präzisierung in eq-copilot/schemas/state/nakama-state-v2.md:29, :38, :184, :187 und :189–195; CRLF und Änderung von 323 auf 331 Zeilen selbst abgeglichen; Vertragszitate in NakamaState.cpp:2495, :2881 und Sonde014IntentTest.cpp:1223, :1404, :1755 nachgezogen;

(h) Geprüft durch Quellen- und Rohbeleglektüre: 34 Mutationsläufe einschließlich Zusagezeilen, Gegenproben und dokumentierter Ketten M-79/M-86/M-87; große Testobjekte auf dem Heap, keine Änderung an Audioverarbeitung, Identität, State-Goldens oder ausgeschlossenen Anschlussdateien; Runner-Behauptungen mit Ausnahme von tools/beweise.ps1:773 durch die gelesenen Fälle gedeckt; Produktstand ce83e67c bis ZIEL unverändert.

Eigenständig ausgeführt wurden ausschließlich lesende Git-/Datei-/Hashabgleiche; Compiler, Tests, Kanon und Laufzeitprüfungen wurden auftragsgemäß nicht gestartet, deren Ergebnisse ausschließlich als vorhandene Belege gelesen und keine Datei verändert.

HEAD Beginn: 97872e87c089170f5d38c7e9189d850632e7e651
HEAD Ende: 97872e87c089170f5d38c7e9189d850632e7e651
Status unverändert: ausschließlich die bereits vorhandenen untracked Pfade .claude/settings.local.json, briefing-hub/ und nimbalyst-local/

FERTIG Erstprüfung Teiletappe 5b, NEEDS_WORK, 1 Defekte, 0 Lücken, 0 Härtungen

Review comment:

- [P2] Beschränke die B27-Behauptung auf die gemessenen Randfälle — C:/Users/phili/Projekte/Nakama/tools/beweise.ps1:773-773
  DEFEKT im Belegabgleich aus Prüffrage (h): B27 bescheinigt hier den erfolgreichen Schritt von 2^53−2 auf 2^53−1 einschließlich genau eines Host-Dirty für Bestands- **und Eintragsrevision**. Tatsächlich richtet `m80()` nur die Bestandsrevision knapp unterhalb der Grenze ein; `m81()` prüft die Eintragsrevision ausschließlich an der bereits erreichten Grenze. Lesend nachvollziehbar mit `git show b40a1bf0:eq-copilot/plugin/tests/Sonde014IntentTest.cpp` in Zeilen 1815–1917. Ein grünes B27-Ergebnis belegt somit den behaupteten letzten gültigen Eintragsschritt nicht. Entweder diesen positiven Eintragsfall ergänzen oder die Behauptung entsprechend begrenzen.
