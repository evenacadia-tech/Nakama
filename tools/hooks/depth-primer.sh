#!/bin/bash
# SessionStart hook: Baseline-Disposition für jede Nakama-Session.
# Fable-5-kalibriert (platform.claude.com …/prompting-claude-fable-5):
# kurze Steuersätze statt Verhaltens-Enumeration; nur was projektspezifisch
# ist — generisches Kommunikations-/Pausen-Coaching liefert der Harness
# selbst. Feuert auch nach Compaction (Fade-Resilienz).

# searx wird GEMESSEN, nicht behauptet: die Regel "erst belegen, dann bauen"
# stand hier mit einer festen Adresse, die auf dem Laptop und nach jedem
# Neustart des Dienstes fehlen kann (gemessen 22.08.2026: HTTP 000). Ein
# Primer, der einen toten Dienst als Pflichtweg nennt, erzieht zum Ignorieren.
if curl -s -o /dev/null --connect-timeout 1 --max-time 2 \
     "http://localhost:8888/search?q=x&format=json" 2>/dev/null; then
  SEARX="erreichbar — erster Weg für Library-/API-Fakten"
else
  SEARX="NICHT erreichbar (gerade gemessen) — dann WebSearch/Herstellerdoku, aber belegen bleibt Pflicht"
fi

cat <<'PRIMER'
=== Depth-Primer (Baseline-Disposition) ===

## Code ist die einzige Wahrheit
Datei öffnen, bevor auf Doku-, Memory- oder Audit-Behauptungen gehandelt
wird. Befund-Docs beschreiben den Stand ihres Datums; Musterblatt und
Engine entwickeln sich schneller als jede Zusammenfassung.

## Verifizieren statt annehmen — dann handeln
Library-/API-Fakten (JUCE 8, CMake, three.js, Playwright, VST3) vor dem
Bauen belegen. Sobald genug Information zum Handeln da ist: handeln —
Feststehendes nicht erneut herleiten.
PRIMER
echo "searx (localhost:8888): $SEARX"
cat <<'PRIMER'

## Karte vor Vorschlag
Vor Engine-/Broker-/Vertragsarbeit: docs/plugin-wissen.md, den Sonden-Entwurf
(mit Errata-Block) und die betroffene Quelle lesen. Design: Figma-Stände des
Users sind die Quelle, design/ übersetzt — unter eq-copilot/design/ gibt es
keine Design-Arbeit; Archiv und Prisma-Studie sind Verlauf, nie Vorgabe.
Einen verworfenen Weg neu vorzuschlagen kostet Vertrauen.

## Im Auftrag bleiben
Nichts über den Auftrag hinaus refactoren oder „modernisieren" —
Legacy-Namen (EqCop*), Material-Kit-Front (Provisorium) und Schema-Felder
bleiben, bis ihr Umbau DER Auftrag ist (Umbenennung = NAK-30). Beschreibt der User ein Problem oder denkt
laut, ist der Befund das Deliverable — erst fixen, wenn er es sagt.

## Gegenpfade (Invariante, Herkunft Bauplan 2.0 §11.5 — archiviert, gilt weiter)
starten↔stoppen · öffnen↔schließen · speichern↔laden ·
aktivieren↔abklingen/archivieren · installieren↔Rollback — beide Hälften
im selben Änderungssatz; eine Einweg-Änderung ist unvollständig.

## Beweise vor „fertig"
Jede Fortschritts- und Fertig-Meldung braucht einen Beleg aus DIESER
Session (Testlauf, Render, Diff) — nicht Verifiziertes explizit als offen
nennen. Ein Entscheid des Users existiert nur mit Datum + Zitat (Register in
CLAUDE.md); eigene Vorschläge heißen „Vorschlag", nie „abgenommen". Betroffene Beweise laufen lassen: GoldenTest · NullTest ·
MarkierungTest · Identität · Hostkontext · Schema · pluginval 8 · Shot ·
PaintBench · PipeProbe · cargo test --manifest-path broker/Cargo.toml —
oder alles auf einmal: pwsh -File tools/beweise.ps1. Pixel-Beweise messen Änderung, nie Schönheit — Optik nimmt
allein der User am lebenden Blatt ab.

## Selbstaudit vor „fertig"
- Realtime: Allokation/Sperre/Log im processBlock? NaN-Pfade? Nyquist-Kante?
- JUCE: Component::SafePointer in Async-/Popover-Callbacks; Headless-Timer
  brauchen callPendingTimersSynchronously().
- Schemas: neues Feld ⇒ erst Version; alte Snapshots laden weiter.
Bei größeren Änderungssätzen gegen Sonden-Entwurf (Errata zuerst) +
Wahrheitskern nachprüfen — an der Quelldatei, nicht am Gedächtnis. Wo die
Umgebung Subagenten ohne Rückfrage erlaubt, ist ein FRISCHER
Verifikations-Subagent das schärfere Werkzeug (frischer Kontext schlägt
Selbstkritik) und breite mechanische Suchen laufen parallel; verlangt sie
dafür erst das Wort des Users, gilt das und die Prüfung läuft im Hauptlauf.
Das Design-Urteil bleibt in jedem Fall im Hauptlauf.

## Funde außerhalb des Auftrags
→ docs/offene-punkte.md, datierte Zeile mit ID. Offene Einträge nie beim
Auffrischen löschen; Schließen nur mit ID + Commit-SHA.

## Pausen
Nur bei echt Irreversiblem (force-push, Löschen, externer Versand), echter
Scope-Entscheidung oder Input, den nur der User geben kann (UAC-Klick,
Figma-Stand, Abnahme im Design-Repo). Sonst handeln und committen
(Commit-Autonomie; bei paralleler Codex-Session mit explizitem Pathspec).
PRIMER
