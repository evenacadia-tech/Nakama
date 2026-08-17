#!/bin/bash
# SessionStart hook: Baseline-Disposition für jede Nakama-Session.
# Fable-5-kalibriert (platform.claude.com …/prompting-claude-fable-5):
# kurze Steuersätze statt Verhaltens-Enumeration; nur was projektspezifisch
# ist — generisches Kommunikations-/Pausen-Coaching liefert der Harness
# selbst. Feuert auch nach Compaction (Fade-Resilienz).

cat <<'PRIMER'
=== Depth-Primer (Baseline-Disposition) ===

## Code ist die einzige Wahrheit
Datei öffnen, bevor auf Doku-, Memory- oder Audit-Behauptungen gehandelt
wird. Befund-Docs beschreiben den Stand ihres Datums; Musterblatt und
Engine entwickeln sich schneller als jede Zusammenfassung.

## Verifizieren statt annehmen — dann handeln
Library-/API-Fakten (JUCE 8, CMake, three.js, Playwright, VST3) vor dem
Bauen per searx (localhost:8888, WebFetch) prüfen. Sobald genug Information
zum Handeln da ist: handeln — Feststehendes nicht erneut herleiten.

## Karte vor Vorschlag
Vor Design-/UI-Vorschlägen: docs/design-stand.md + Bauplan lesen —
besonders die VERWORFENEN Richtungen (einen verworfenen Weg neu
vorzuschlagen kostet Vertrauen). Vor Engine-/Editor-Arbeit:
docs/plugin-wissen.md + die betroffene Quelle.

## Im Auftrag bleiben
Nichts über den Auftrag hinaus refactoren oder „modernisieren" —
Legacy-Namen (EqCop*), Material-Kit-Front und Schema-Felder bleiben, bis
ihr Umbau DER Auftrag ist. Beschreibt der User ein Problem oder denkt
laut, ist der Befund das Deliverable — erst fixen, wenn er es sagt.

## Gegenpfade (Bauplan §11.5)
starten↔stoppen · öffnen↔schließen · speichern↔laden ·
aktivieren↔abklingen/archivieren · installieren↔Rollback — beide Hälften
im selben Änderungssatz; eine Einweg-Änderung ist unvollständig.

## Beweise vor „fertig"
Jede Fortschritts- und Fertig-Meldung braucht einen Beleg aus DIESER
Session (Testlauf, Render, Diff) — nicht Verifiziertes explizit als offen
nennen. Betroffene Beweise laufen lassen: GoldenTest · NullTest ·
MarkierungTest · pluginval 8 · Shot · PaintBench · PipeProbe · cargo test
eq_copilot. Pixel-Beweise messen Änderung, nie Schönheit — Optik nimmt
allein der User am lebenden Blatt ab.

## Selbstaudit vor „fertig"
- Realtime: Allokation/Sperre/Log im processBlock? NaN-Pfade? Nyquist-Kante?
- JUCE: Component::SafePointer in Async-/Popover-Callbacks; Headless-Timer
  brauchen callPendingTimersSynchronously().
- Schemas: neues Feld ⇒ erst Version; alte Snapshots laden weiter.
Bei größeren Änderungssätzen einen FRISCHEN Verifikations-Subagenten gegen
Bauplan + Grundgesetz prüfen lassen — frischer Kontext schlägt Selbstkritik.
Breite mechanische Suchen parallel an Explore-Agenten geben und
währenddessen weiterarbeiten; das Design-Urteil bleibt im Hauptlauf.

## Funde außerhalb des Auftrags
→ docs/offene-punkte.md, datierte Zeile mit ID. Offene Einträge nie beim
Auffrischen löschen; Schließen nur mit ID + Commit-SHA.

## Pausen
Nur bei echt Irreversiblem (force-push, Löschen, externer Versand), echter
Scope-Entscheidung oder Input, den nur der User geben kann (UAC-Klick,
Design-Freeze, Optik-Abnahme). Sonst handeln und committen
(Commit-Autonomie; im geteilten FL-Studio-Repo mit explizitem Pathspec).
PRIMER
