# NAK-313 — Erstprüfung Teiletappe 5a, Rohurteil (Codex gpt-6-astra, Effort xhigh, lesend; Vorlage A)

**Thread:** 01a0d3c4-146e-71c1-8320-944fe011f43a · **HEAD vor und nach dem Lauf:** ef73236e8f9fea9d74e25f23d4e4aa1e62b6e5f1 (Code gleich 7ea49e2c; seit 1fabd5b4 nur der Prüfauftrag unter docs/**) · **Lauf:** 24.09.2026, 16:13:57 bis 16:28:33 (+02:00, aus der Startdatei des Laufs `tools/dirigent/codex-lauf.ps1`, Kennung `nak313-e5ap1`) · **Prüfbereich:** git diff a9cbbbb7...1fabd5b4 auf die 30 Ticketpfade der Teiletappe 5a (Prüffragen (a) bis (h)) · **Auftrag:** docs/beweise/roh/NAK-313-etappe-5a-erstpruefung-auftrag.txt · **Ablage:** letzte Nachricht des Prüfers (read-only-Sandbox), unverändert vom Dirigenten abgelegt. Der Lauf führte 118 lesende Kommandos aus; die letzte Nachricht trägt je Prüffrage (a) bis (h) einen Absatz mit Beleg am ZIEL, die eigenen Nachmessungen, HEAD-Werte und die FERTIG-Zeile.

---

URTEIL: PASS — kein DEFEKT

Im gebundenen Änderungssatz wurde kein eingeführter Defekt festgestellt. Compiler, Tests, Cargo, Python und FL Studio wurden nicht gestartet; sämtliche Ausführungsergebnisse sind ausschließlich gelesene Fremdbelege.

Prüfgrundlage: BASIS a9cbbbb7e72b6441a250978802ff465b185b7749 → ZIEL 1fabd5b43bd27d23293f909807167a22bc5ab5f4; alle folgenden Quellverweise am ZIEL

(a) Geprüft, lesend — JSON-Grammatik, exponentenabhängige Ganzzahligkeit, Grenzvergleich, -0 und unveränderte Ausgabe bei Fehlschlag in eq-copilot/plugin/core/ipc/WireZahl.h:269–413; gemeinsame Zerlegung in eq-copilot/plugin/vertrag/NakamaVertrag.cpp:333–418; ACK-Bereich 0…9007199254740991 ohne Ersatznull in eq-copilot/plugin/core/ipc/controlclient/Intern.h:196–235 und Welcome-Bereich 3…3 in controlclient/Vertrag.cpp:206–216; B10-Randfälle sowie Freigabe-, Konflikt- und Wiederholungsprüfungen in eq-copilot/plugin/tests/IpcTestMain.cpp:3344–3818

(b) Geprüft, lesend — rekursive Übernahme einschließlich null und korrekter int64-/double-Unterscheidung in eq-copilot/plugin/vertrag/NakamaVertrag.cpp:778–822; beide Quellenmodelleinstiege aus demselben strengen Lauf in eq-copilot/plugin/src/SourcesModel.cpp:750 und :1509; nichtnegativeGanzzahl unverändert; B3c entsprechend in eq-copilot/plugin/tests/SchemaTestMain.cpp:592; Wertprüfungen für controlAlterMs 1500, hostMixerIndex 3 sowie Rücknahmegrenzen in eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp:424–678

(c) Geprüft, lesend — gemeinsamer Rust-Ganzzahlhelfer und optionale Feldauswertung in broker/src/vertrag.rs:607–639; Heartbeat-Sequenz vor Seiteneffekten in broker/src/coordinator/befehl.rs:692–704; ungültige Evidenzwerte ohne Ersatznull in broker/src/coordinator/evidenz.rs:272–350, zugehöriger Restore in experiment_verdrahtung.rs:1439–1444; normalisierter Mixerindex in liveness.rs:781–784; Intervention, Befehlskopf, Experimentziel, Konfliktvergleich, Intent und Assistent ebenfalls über den Helfer; Wirkungstests in broker/tests/sonde013_verdrahtung.rs:6618–7006 und sonde014_verdrahtung.rs:3129–3177

(d) Geprüft, lesend — Protokollwahl und Normalisierung über denselben Helfer in broker/src/transport/bootstrap.rs:205–229 und :281–317; Wertprüfungen für Protokoll 3, Blockgröße 256, zwei Kanäle und pid 1234 sowie Ablehnung von 3.5 in :743–784; bestehender V2-Weg in :609; kein vorgezogenes Textriegel-/Schema-/Adress-Tor

(e) Geprüft, einschließlich eigener struktureller Auszählung — 147 Einträge, davon 122 v3 und 25 v2; 37 Stufenabweichungen und eine Urteilsabweichung, jeweils ausgewiesen; Sollzahlen B10 10/11/11, B13 22/20, A4b 25, A4 21/17/10, A5 122, A11 25 und B16 14; Formen, erwartete Werte, Negativvektoren, Leserstufen und PE-Kennungen gegen Erzeuger und Verbraucher abgeglichen; Erzeugung und Konsistenzprüfung in tools/eq-copilot/erzeuge_v3_fixtures.py:4027–4551; Tabelle nicht im MANIFEST gehasht, aber vollständiger erzeugter Bytevergleich über :5019 und :5051, somit mehr als Selbstprüfung; bestehende Fixtures, MANIFEST und sämtliche Schemadateien laut Git-Diff bytegleich

(f) Geprüft, lesend — v2-Zahlriegel nach UTF-8-Prüfung und vor strengem Parser beziehungsweise JUCE in eq-copilot/plugin/src/PipeClient.cpp:706–724; Fehlerweitergabe als „Zahlriegel: <Grund>“; rein ziffernbasierte Bewertung einschließlich nackter Integer und Dreiziffernschranke in eq-copilot/plugin/vertrag/NakamaVertrag.cpp:333–418 und :677–776; geforderte Grenz-, Überlauf-, Exponenten- und Präzisionsvektoren in eq-copilot/plugin/tests/PipeClientLifecycleTestMain.cpp:462–655; UB-Schalter am Programmeinstieg :2043; kanonischer Aufruf weiterhin ohne Argumente in tools/beweise.ps1:530–533

(g) Geprüft, lesend — Decimal-fähige Typ- und Wertprüfung in tools/eq-copilot/pruefe_v2_schemas.py:64–106; parse_float=decimal.Decimal neben dem Hook, Vergleich ausschließlich mit vertrag einschließlich Stufe und Zählprüfung auf 25 Einträge in :119–142; M-93(c)/PE-116 als gültiger Referenzfall, unabhängig von der absichtlichen produktseitigen Sequenzabweichung

(h) Geprüft, lesend — Rotbelege M-55 bis M-73, M-90, M-91 und M-93 bis M-97 einschließlich zugehöriger Mutationsprotokolle gegen Zusage, Fehlerzeile und Gegenlauf abgeglichen; v2-Rotläufe mit --ohne-ueberlaufvektoren, Überlaufprüfung ohne Übergabe an JUCE; keine neu eingeführten großen Prozessor-/Engine-Stackobjekte; Behauptungserweiterungen in tools/beweise.ps1:518–767 durch entsprechende Fälle gedeckt; WireZahl weiterhin header-only; keine Änderungen an Sendern, State-, Identitäts- oder Audio-Thread-Pfaden; gelesener Endstand-Kanon GRÜN 70/70, einschließlich A1/A16/B1 und vollständiger Korpusbeine, in docs/beweise/roh/NAK-313-7ea49e2-dirty.md; keine eigene Laufzeitbestätigung

Eigene Nachmessungen: Git-Diffs, unveränderte Bestandsdateien, Korpusstruktur und Fallzahlen, identischer Produktcode zwischen 7ea49e2c und ZIEL, HEAD und Arbeitsbaumstatus; keine Dateien angelegt oder verändert; dieselben drei untracked Pfade zu Beginn und Ende

HEAD Beginn: ef73236e8f9fea9d74e25f23d4e4aa1e62b6e5f1
HEAD Ende: ef73236e8f9fea9d74e25f23d4e4aa1e62b6e5f1

FERTIG Erstprüfung Teiletappe 5a, PASS, 0 Defekte, 0 Lücken, 0 Härtungen
