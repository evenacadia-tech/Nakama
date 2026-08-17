#!/bin/bash
# SessionStart hook: Baseline-Disposition für jede Nakama-Session.
# Nakama-Zuschnitt des Sonifold-Depth-Primers: Beweis-Kanon, Gegenpfade,
# Realtime-Selbstaudit. Feuert auch nach Compaction (Fade-Resilienz).

cat <<'PRIMER'
=== Depth-Primer (Baseline-Disposition) ===

## Code ist die einzige Wahrheit
Datei öffnen, bevor auf Doku-, Memory- oder Audit-Behauptungen gehandelt
wird. Befund-Docs beschreiben den Stand ihres Datums; das Musterblatt und
die Engine entwickeln sich schneller als jede Zusammenfassung. Bei
Widerspruch gewinnt die Quelle — immer.

## Verifizieren statt annehmen
Vor Code, der von Library-/API-Fakten abhängt (JUCE 8, CMake, three.js,
Playwright, VST3): searx auf localhost:8888 via WebFetch befragen.
Eine Query ist trivial; eine falsche Annahme kompoundiert.

## Karte vor Vorschlag
Vor UI-/Design-Vorschlägen ZUERST lesen: docs/design-stand.md (hier) +
NAKAMA-SPECTRAL-FIELD-BAUPLAN.md (dort) — insbesondere die VERWORFENEN
Richtungen. Einen verworfenen Weg neu vorzuschlagen kostet Vertrauen.
Vor Engine-/Editor-Arbeit: docs/plugin-wissen.md + die betroffene Quelle.

## Gegenpfade — beide Hälften sind das Deliverable (Bauplan §11.5)
In jedem Änderungssatz gemeinsam prüfen: Messung starten ↔ stoppen/neu ·
Archiv öffnen ↔ schließen · speichern ↔ laden/ungültig zurücksetzen ·
Schnappschuss speichern ↔ laden/entfernen · Befund aktivieren ↔
abklingen/archivieren · installieren ↔ Rollback. Eine Einweg-Änderung
ist unvollständig.

## Beweise vor "fertig"
Der Standard der Befund-Docs gilt: "ausgeführt und gesehen". Betroffene
Beweise laufen lassen (GoldenTest · NullTest · pluginval 8 · Shot ·
PaintBench · PipeProbe · cargo test eq_copilot), Ergebnis nennen. Ein
grüner Pixel-Beweis misst ÄNDERUNG, nie Schönheit — Optik nimmt der User
am lebenden Blatt ab.

## Selbstaudit vor "fertig"
Das Geschriebene mit adversarialen Augen erneut lesen:
- Realtime: Allokation/Sperre/Log im processBlock? Denormals? NaN-Pfade
  (NaN-Riegel zählt mit)? Nyquist-Kanten?
- JUCE-Lebenszyklus: Message-Thread-Disziplin; Component::SafePointer
  statt rohem this in Popover-/Async-Callbacks (FL darf den Editor
  jederzeit schließen); Timer in Headless-Tests brauchen
  callPendingTimersSynchronously().
- Schema-Verträge: neues Feld ⇒ erst Version; alte Snapshots müssen ohne
  das Feld laden; unbekannte Felder dürfen alte Consumer nicht brechen.
- Querbezüge: existiert die Datei, stimmt die Signatur, stimmen Zeilen?
Selbst gefundene Fehler kosten 10 Minuten; vom User gefundene kosten
Vertrauen.

## Zwischenstands-Digests
Nach 5+ Tool-Calls 3–5 Sätze im Antworttext: was steht, was fehlt,
welche Drift vom Auftrag. Unsichtbare Checkpoints können nicht korrigieren.

## Out-of-scope-Funde nach docs/offene-punkte.md
Bugs/Politur außerhalb des aktuellen Auftrags: dort als datierte Zeile mit
ID erfassen. Offene Einträge nie beim Auffrischen löschen; Schließen nur
mit ID + Commit-SHA. NEXT-SESSION-Briefs sind Einweg-Konsumgut, die
offene-punkte.md ist das durable Gedächtnis.

## Keine Abkürzungen
- Nicht erzählen statt bauen ("ich würde X" → X bauen)
- Kein "gut genug für jetzt", wenn der richtige Weg länger dauert
- Synthese (Pläne, Audits, Design-Urteil) nicht an Subagenten delegieren
- Nach einer Empfehlung kein Menü — festlegen und ausführen

## Nicht-destruktiv = keine Erlaubnis nötig
Lesen/Editieren/Testen/searx/Committen frei (Commit-Autonomie). Bestätigen
NUR bei irreversiblem: force-push, hard-reset, Löschen, externem Versand.
UAC-Installation macht immer der User (der Klick gehört ihm).
PRIMER
