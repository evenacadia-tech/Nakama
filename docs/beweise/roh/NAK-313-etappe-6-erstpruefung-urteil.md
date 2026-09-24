# NAK-313 — Erstprüfung Etappe 6, Rohurteil (Codex gpt-6-astra, Effort xhigh, lesend; Vorlage A)

**Thread:** 01a0d569-a30e-7c52-a55d-f95db29b4651 · **HEAD vor und nach dem Lauf:** 9dac1bd6679f28b697b618d3b4ce14bd2f45ebbf (Code gleich 1ffac1e5; seit 16c72529 nur der Prüfauftrag unter docs/**) · **Lauf:** 24.09.2026, 23:54:24 bis 25.09.2026, 00:01:58 (+02:00, aus der Startdatei des Laufs `tools/dirigent/codex-lauf.ps1`, Kennung `nak313-e6p1`) · **Prüfbereich:** git diff 00e3bea6...16c72529 auf die 25 Ticketpfade der Etappe 6 (Prüffragen (a) bis (h)) · **Auftrag:** docs/beweise/roh/NAK-313-etappe-6-erstpruefung-auftrag.txt · **Ablage:** letzte Nachricht des Prüfers (read-only-Sandbox), unverändert vom Dirigenten abgelegt. Der Lauf führte 58 lesende Kommandos aus; Astra hat die Etappe am Tor nicht reguliert, ein Sol-Lauf war nicht nötig.

---

URTEIL: PASS — kein DEFEKT

T2-Leseprüfung des gebundenen Diffs BASIS 00e3bea66a1b673d5ea3574927ccd610be161dd0 gegen ZIEL 16c725299e4b74844853d5bc9010a1f2c345516f:

(a) Geprüft: Torreihenfolge, gehaltenes Textriegelurteil, ein strenger Lauf, v2-Ausgang vor v3-Regeln, Protokollwahl, Schema und Übernahme aus demselben Wert in broker/src/transport/bootstrap.rs:283-391; dieselbe Schemaquelle und Engine, kanonischer erster Fehler in :232-267; host-, Zahlenrand-, Laufzähler- und v2-Wachen in :937-1074, Binärframe-Wache in :708; Signatur, Fehlervarianten und Kopplung unverändert;

(b) Geprüft: UTF-8 vor dem Lesen, vollständige Escape-Dekodierung einschließlich Surrogatpaar, Ablehnung ungültiger Formen, dekodierter Namensvergleich und acht Fehlergründe in eq-copilot/plugin/core/ipc/IpcVerbindung.cpp:442-690; Grund-Ausgang nur bei Fehler, Weitergabe durch die beiden Welcome-Aufrufer in controlclient/Verbindung.cpp:652-656 und TelemetryClient.cpp:672-676; ACK-Aufruf weiterhin ohne Grund-Ausgang in controlclient/Intern.h:194-211; B10-Nachweise einschließlich Registerwirkung in IpcTestMain.cpp:4018-4185;

(c) Geprüft: Codepunktgrenzen 64/500 mit separater Leerprüfung der broker_version in eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:218-239; B10-Ränder, leere Texte und Telemetriegegenstücke in IpcTestMain.cpp:3993-4043, :4113-4120 und :4188-4215; Escape-Fall auf bA umgestellt, Duplikat und Nachspann erhalten in :6685-6694; vorhandene gültige ACK-Wertfälle weiterhin mit angewandt-Prüfung in :3528-3537;

(d) Geprüft: beide Slotriegel direkt am VTable-Eintrag, ab Slot 14, einmaliger Verstoß und zulässiger Nullslot in eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp:185-203 und broker/src/telemetrie.rs:571-588; Ablehnung ohne Lautheitsreparatur über fuer_broker in :408-424 und Senke in broker/src/coordinator/senke.rs:134-141; Beobachtungsprüfungen für ausbleibende Frames, p2_reject und unverändertes Gen-Modell in broker/tests/sonde012_sources_slice.rs:560-597 und Sonde012SourcesModelTest.cpp:718-746;

(e) Geprüft: temporäre Schemavariante, gemeinsamer flatc-Aufruf, gültige VTable-Bytemutation und Manifestaufnahme in tools/eq-copilot/erzeuge_fb_fixtures.py:1198-1265, :1277-1350; bestehende Binärfixtures laut Git-Diff unverändert, ausschließlich zwei neue Binärdateien; .fbs und generierte Dateien ausschließlich um denselben Kommentar ergänzt, Feld-IDs unverändert; Zeilenenden selbst mit git ls-files --eol kontrolliert; A9-Neugenerierung und A10-Bytegleichheit nur anhand der vorhandenen Laufbelege geprüft;

(f) Geprüft: Betragsuntergrenze in eq-copilot/schemas/v3/README.md:67-71, vier zusätzliche Fälle im Erzeuger :595-599 und unveränderte ausführbare Zahlenriegel; Tabellenverbrauch durch Rust in broker/tests/contract_cross_language.rs:157-181, C++ in SchemaTestMain.cpp:1071-1107 und Python in pruefe_v3_vertrag.py:458-477; Mutationsbelege U121c/U121r/U121p gelesen, nicht erneut ausgeführt;

(g) Geprüft und selbst nachgezählt: 197 Produkteingänge, davon 170 v3 und 27 v2, sämtliche Eingangs- und Nachrichtenzähler passend, 58 Stufenabweichungen und eine Urteilsabweichung; Kopf in eq-copilot/fixtures/v3/PRODUKTEINGAENGE-FAELLE.json:378, Erzeugerprüfung in tools/eq-copilot/erzeuge_v3_fixtures.py:4751-4903; Produktvergleich samt Zählprüfung in bootstrap.rs:859-921 und IpcTestMain.cpp:3480-3694, Referenzvergleich in pruefe_v3_vertrag.py:2044-2082; JSON-Schemas und bestehende Nachrichtenfixtures laut Diff bytegleich;

(h) Geprüft: Rotbelege M-98 bis M-122 mit Mutationsstellen, Zusageketten und Gegenläufen abgeglichen; Runner-Ergänzungen in tools/beweise.ps1:518, :544-562, :702, :738 und :750 durch die gelesenen Fälle gedeckt; keine Änderung an Audio-, State-, Identitäts- oder NAK-310-Pfaden im abgegrenzten Produktdiff, keine erkennbare Regression gültiger Produktnachrichten oder Batches; vorhandener Kanonbeleg GRUEN 70/70 einschließlich A1/A16/B1 nur gelesen, keine eigene Laufzeitbestätigung.

Compiler, Tests, cargo, flatc, Python und FL Studio wurden auftragsgemäß nicht gestartet; keine Datei wurde angelegt oder verändert.

HEAD Beginn: 9dac1bd6679f28b697b618d3b4ce14bd2f45ebbf
HEAD Ende: 9dac1bd6679f28b697b618d3b4ce14bd2f45ebbf
Status unverändert: .claude/settings.local.json, briefing-hub/ und nimbalyst-local/ untracked; Arbeitsdiff leer

FERTIG Erstprüfung Etappe 6, PASS, 0 Defekte, 0 Lücken, 0 Härtungen
