# NAK-313 — Erstprüfung Etappe 7, Rohurteil (Codex gpt-6-astra, Effort xhigh, lesend; Vorlage A)

**Thread:** 01a0d609-019b-72e3-a403-c246949e0b81 · **HEAD vor und nach dem Lauf:** 35a8d9a691b7956e0e4c007bf143f5e7411fd273 (Code gleich d64680cf; seit bac9115c nur der Prüfauftrag unter docs/**) · **Lauf:** 25.09.2026, 02:48:28 bis 03:02:39 (+02:00, aus der Startdatei des Laufs `tools/dirigent/codex-lauf.ps1`, Kennung `nak313-e7p1`) · **Prüfbereich:** git diff 4aee3eec...bac9115c auf die 20 Ticketpfade der Etappe 7 (Prüffragen (a) bis (h)) · **Auftrag:** docs/beweise/roh/NAK-313-etappe-7-erstpruefung-auftrag.txt · **Ablage:** letzte Nachricht des Prüfers (read-only-Sandbox), unverändert vom Dirigenten abgelegt. Der Lauf führte 75 lesende Kommandos aus; Astra hat nicht reguliert.

---
URTEIL: PASS — kein DEFEKT

Die lesende Prüfung des gebundenen Änderungssatzes von 4aee3eec835ca4bf51e3a73f1d141ed0658878ee bis bac9115c1f06e2cdacdd583c95eeb88809e51e17 hat keinen behebungsbedürftigen Verstoß ergeben; alle folgenden Belegstellen beziehen sich auf ZIEL.

(a) Geprüft, lesend: zehn Capability-Paare in bisheriger Reihenfolge, binary_telemetry supported, remote_control unsupported und passender Kommentar in eq-copilot/plugin/core/ipc/controlclient/Intern.h:373–389; strenge Schreiberauswertung und schlüsselweiser Berichtsvergleich in eq-copilot/plugin/tests/IpcTestMain.cpp:4236–4370; unveränderte Übernahme beziehungsweise Formprüfung in broker/src/coordinator/liveness.rs:471–478 und eq-copilot/plugin/src/SourcesModel.cpp:179–198; keine Schemaänderung

(b) Geprüft, lesend: Termin-/Kanonbeweisregel, Rohdateiprüfung und getrennte Fehlerlisten der Selbstproben in tools/eq-copilot/pruefe_host_capabilities.py:63–116; kanonischer Build-Beleg und Erratum-Verweis in eq-copilot/identity/host-capabilities-fl-v1.json:97–104; neun andere Bits und Terminrohdaten unverändert; Basis- und Mutationsbelege mit passenden Ablehnungen

(c) Geprüft, lesend: genau drei Wiretext-Ersetzungen in eq-copilot/fixtures/v3/heartbeat-wire-v1.json:47–57; PE-198/PE-199 aus tools/eq-copilot/erzeuge_v3_fixtures.py:4745–4766; Kopfzahlen 199 insgesamt, 23 rust_p0 und 172 v3; unverändertes MANIFEST; tabellengestützte P2-Annahme mit beiden Bitwerten sowie Reconnect-Fenster und erneuerte Deskriptoren in broker/tests/sonde012_sources_slice.rs:600–774; keine neue Empfangsschranke

(d) Geprüft, lesend und durch Diff-/Zeilenzählung: Erratum-Kasten unmittelbar vor der Tabelle in docs/FL-Nakama-Sonden-Design-Entwurf.md:3993–4001, Verlaufseintrag nach (q) auf :365–372, aktualisierte Zähler auf :10 und :57; geforderte Abgrenzung von Build-Tatsache, P2 und P0/P1 einschließlich 3/7-Zählung; 4605 → 4624 Zeilen, übriger Git-Dateiinhalt unverändert und drei Kommentarverweise passend nachgezogen

(e) Geprüft, lesend: einzige Kollisionsreaktion beiKollision, exklusive Temp-Datei, geprüfter Schreib-/Spülausgang, Größenvergleich, Umbenennen ohne Ersetzen sowie Bereinigung beziehungsweise rest in eq-copilot/plugin/src/prozessor/Analyse.cpp:1546–1600; Aufrufersnapshot, frühe Keine-Daten-Ablehnung und eine Uhrablesung auf :1638–1670; Fehler-, Kollisions- und Zeitfälle in eq-copilot/plugin/tests/BriefkastenTestMain.cpp:3652–3800; kein erzeugender ersetzt-Pfad; PluginProcessor.h selbst nachgezählt: 2000 Zeilen

(f) Geprüft, lesend: gemeinsamer Snapshot für Datei und Vergleichslinie, unveränderte Linie bei Ablehnung/Fehler sowie tatsächlicher Dateipfad und Restmeldung in eq-copilot/plugin/src/PluginEditor.cpp:146–170; echte Editorprüfungen über Statuszugang und Messdatenänderungshaken in eq-copilot/plugin/tests/ShotTestMain.cpp:2387–2484; keine neuen sichtbaren Elemente und keine weiteren funktionalen Editoränderungen

(g) Geprüft, lesend: datierte Referenzbühnenanpassung, übergebener Snapshot und unveränderte Bytevergleichsgrundlage in eq-copilot/plugin/tests/BriefkastenTestMain.cpp:135–145, :281–285 und :417–457; temporäre Testablage mit Bereinigung auf :350–379; Nullzugriffsprüfung ohne Testordner auf :3641–3649; Testbauschranke in eq-copilot/plugin/src/prozessor/Analyse.cpp:1447–1466; unveränderte Makrozuordnung für B15/B30 in eq-copilot/plugin/CMakeLists.txt:1331–1337

(h) Geprüft, lesend: Rotbelege M-123 bis M-145 einschließlich Basisstellen, Mutationsketten, Gegenproben und korrigiertem R129-Nachlauf; Behauptungsänderungen in tools/beweise.ps1:509, :518, :544, :584, :738, :752 und :777 durch die gelesenen Fälle gedeckt; große Prozessorstände auf dem Heap; geschützte Audio-, State-, Schema-, Identitäts- und Briefkastenimplementierungen im Diff unverändert; vorhandener Kanon GRÜN 70/70 laut docs/beweise/roh/NAK-313-bb2bfd4-dirty.md:3, darunter Nulltest-, Identitäts- und Exportbelege

Selbst nachgemessen wurden Git-Diff, Änderungsgrenzen, Zeilenzahlen und HEAD; Compiler, Tests, Python und Laufzeitprogramme wurden auftragsgemäß nicht gestartet, Laufzeitergebnisse ausschließlich als vorhandene Rohbelege gelesen.

HEAD Beginn: 35a8d9a691b7956e0e4c007bf143f5e7411fd273
HEAD Ende: 35a8d9a691b7956e0e4c007bf143f5e7411fd273
Arbeitsbaum unverändert: dieselben drei ungetrackten Pfade; keine Dateien angelegt oder verändert

FERTIG Erstprüfung Etappe 7, PASS, 0 Defekte, 0 Lücken, 0 Härtungen
