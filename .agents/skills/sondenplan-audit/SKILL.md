---
name: sondenplan-audit
description: "Auditiert die Sondenplan-Implementierungen im Nakama-Repository auf Codequalität, Echtzeit-Sicherheit, Robustheit und KI-typische Qualitätsmängel (Slop, Scheintests, Lücken, Heuristiken)."
---

# Sondenplan KI-Qualitätsaudit

Dieser Skill dient dem methodischen, unnachgiebigen und tiefgehenden Auditieren aller bisherigen Implementierungen des Nakama-Sondenplans.

> [!IMPORTANT]
> **Striktes Schreibverbot für Produktcode:**
> Dieser Skill führt ausschließlich statische Analysen, semantische Code-Prüfungen, Trace-Analysen und Test-/Beweisprüfungen durch. Es wird **niemals** Produktcode eigenmächtig modifiziert.

---

## 1. Audit-Dimensionen: „KI-Codequalität auf dem Prüfstand“

Bei der Behauptung „KI produziert minderwertigen Code“ stehen typische Fehlermuster im Fokus:

1. **Echtzeit-Verletzungen (Realtime Safety):**
   - Versteckte Heap-Allokationen im Audio-Thread (`std::vector` resize, `std::string`, dynamic casting, JUCE-Objekterstellung).
   - Mutexe, Locks, atomare Spurious-Loops oder blockierende I/O im Audio-Pfad.
   - Tail-Latenzen, Queue-Überläufe und unbedachtes Locking bei IPC-Nachrichten.

2. **Schein-Korrektheit & Scheintests (Mocking/Tautologien):**
   - Tests, die nur Mocks oder getrennte Dummy-Pfade prüfen statt den echten Produktions-DSP-Pfad.
   - Ungeprüfte Annahmen, fehlende Assertions, Tests die immer grün sind („Green-washing“).
   - Nicht abgedeckte Grenzfälle (Blockgröße 0, 1, 64, 2048, Samplerate-Wechsel, NaN/Inf).

3. **Zustands-, Vertrags- und Datenintegrität:**
   - Asymmetrie zwischen Serialisierung und Deserialisierung (Save/Load-Drift).
   - FlatBuffers-v3-Vertragsbrüche, Endianness, Alignment, uninitialisierte Struct-Felder.
   - Stale State bei Verbindungsabbrüchen oder Disconnects.

4. **Architektur- & Abstraktionsqualität (C++20 / Rust):**
   - Unnötige Kopien (`std::span` vs. By-Value), dangling References in Closures/Lambdas.
   - Resource Leaks, fehlendes RAII, Race Conditions bei atomaren Operationen (`memory_order`).
   - Exception-Unsicherheit und unvollständiges Error-Handling.

---

## 2. Ablauf des Audits

1. **Bestandsaufnahme:**
   - Lesen von `docs/PLAN-STAND.md`, `docs/FL-Nakama-Sonden-Design-Entwurf.md` und den Beweis-Manifesten in `docs/beweise/`.
   - Identifizieren aller implementierten Schritte (z.B. S1–S9, P0, P1, P2) und der zugehörigen Quellcode-Dateien in `eq-copilot/` und `broker/`.

2. **Systematisches Modul-Auditing:**
   - Zeile für Zeile den Produktionscode, die Audio-Callbacks, Lock-Free Ringbuffer, FlatBuffers-Reader/Writer und den Rust-Broker prüfen.
   - Aufrufhierarchien und Thread-Grenzen (UI-Thread ↔ Worker-Thread ↔ Audio-Thread) nachverfolgen.

3. **Adversarial Edge-Case Prüfung:**
   - Grenzwerte: Was passiert bei SampleRate = 0 / 192 kHz?
   - Was passiert bei NaN/Inf/Subnormals im Audio-Puffer?
   - Was passiert bei vollen Queues / Broker-Crash / Pipe-Disconnects?

4. **Wiederholung & Lückenlosigkeit:**
   - Den Prozess iterativ wiederholen, bis alle Module und Phasen ohne blinde Flecken geprüft sind.

5. **Ergebnisbericht:**
   - Erstellung eines detaillierten Auditberichts als Markdown-Datei mit klickbaren Zeilenlinks, Schweregrad-Einstufung, Belegen und konkreten Handlungsempfehlungen.
