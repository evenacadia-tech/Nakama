# NAK-313 — Wiederprüfung 1 der Teiletappe 5b, Rohurteil (Codex gpt-6-astra, Effort xhigh, lesend; Vorlage B)

**Thread:** 01a0d4cd-98fd-7b63-bae9-733154651fc5 · **HEAD vor und nach dem Lauf:** 89a2c2184e10c827094c7524e574b8ad5e3d9f44 (seit ebf36da3 nur der Prüfauftrag unter docs/**) · **Lauf:** 24.09.2026, 21:03:58 bis 21:08:21 (+02:00, aus der Startdatei des Laufs `tools/dirigent/codex-lauf.ps1`, Kennung `nak313-e5bw1`) · **Prüfbereich:** git diff ccdc157d...ebf36da3 auf eq-copilot/plugin/tests/Sonde014IntentTest.cpp, eq-copilot/plugin/state/NakamaState.cpp, tools/beweise.ps1 (Fixdiff der Nacharbeit 1; der Befund der Erstprüfung 5b wörtlich) · **Auftrag:** docs/beweise/roh/NAK-313-etappe-5b-wiederpruefung-1-auftrag.txt · **Ablage:** letzte Nachricht des Prüfers (read-only-Sandbox), unverändert vom Dirigenten abgelegt. Der Lauf führte 18 lesende Kommandos aus.

---

URTEIL: PASS — alle Befunde geschlossen, nichts gebrochen.

Frage 1: Ja — am ZIEL ebf36da3cad124c97014106740f92f8a34a838b0 lädt m81() den schreibbaren Eintrag mit Revision 2^53−2 bei Bestandsrevision 1, ändert fuehrt zu traegt und prüft true, Eintragsrevision 9007199254740991 mit neuer Rolle, Bestandsrevision 2, genau ein Host-Dirty und den schreibbaren Reload; beide Prozessoren liegen auf dem Heap, die Eingaben und Assertions sind zeitunabhängig (eq-copilot/plugin/tests/Sonde014IntentTest.cpp:1883-1908), womit der Eintragsteil von tools/beweise.ps1:773 gedeckt ist; W81 dokumentiert die verengte Schranke, genau den gefallenen Positivfall, beide grünen Randprüfungen und den grünen Gegenlauf (docs/beweise/roh/NAK-313-e5b-mutation-W81.txt:16-36,44-49).

Frage 2: Nein — ausschließlich 40 zusätzliche Testzeilen, leere Diffs für NakamaState.cpp und tools/beweise.ps1, keine Mutationsreste im geprüften Baum, M-80 und der bisherige M-81-Randblock identisch (eq-copilot/plugin/tests/Sonde014IntentTest.cpp:1815-1872,1918-1957); kein erkennbarer Bruch von Test, Matrix, Gate oder CLAUDE-Invariante; der Kanonbeleg meldet GRUEN 70/70 auf 56fbfcd5, dessen Code- und Teststand nachweislich dem ZIEL entspricht, einschließlich grüner M-80/M-81-Prüfungen und B27 mit 277 erfolgreichen Assertions (docs/beweise/roh/NAK-313-56fbfcd-dirty.md:3,14-15,9583-9595).

Geprüft: ausschließlich lesender Fixdiff-, Quelltext-, Blob- und Belegabgleich am ZIEL

Nicht geprüft: keine eigenen Builds, Tests, Mutationsläufe oder FL-Läufe; übriger Ticketbereich und docs/** nicht als Prüfgegenstand; fremde Dateien unangetastet

HEAD Beginn: 89a2c2184e10c827094c7524e574b8ad5e3d9f44
HEAD Ende: 89a2c2184e10c827094c7524e574b8ad5e3d9f44

FERTIG Wiederprüfung 1 Teiletappe 5b, PASS, 0 offene Befunde, 0 Brüche
