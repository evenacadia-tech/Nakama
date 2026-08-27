# Plan — ausführliches Debugging des gebauten Sondenplans

## Ziel

Den aktuell gebauten Nakama-Sondenstand bis einschließlich S13 gegen Source,
Ticket-Gates und laufende Tests prüfen, bestätigte Fehler im beauftragten
Bereich mit dem kleinsten kohärenten Änderungssatz beheben und den Endstand
frisch belegen.

## Gemessener Ausgangspunkt

- Worktree vor dem Planstand-Lauf sauber.
- `tools/plan/planstand.py`: 11/36 abgenommen, 6 gebaut, 19 offen.
- Relevanter Codeumfang: P0-Host-/Identitätsbasis, P1 S5–S9/G1 sowie P2
  S10–S13. S14 ff. sind nicht implementiert und nicht Gegenstand von
  vorgezogenen Features.
- `docs/PLAN-STAND.md` wurde vom vorgeschriebenen Rechner nur beim Datum von
  2026-08-26 auf 2026-08-27 aktualisiert.

## Annahmen

- „Code debugging“ autorisiert Diagnose, reproduzierbare Tests und Fixes für
  bestätigte Fehler innerhalb der gebauten Sondenfläche.
- Fremde Änderungen bleiben unangetastet; Agenten prüfen Code zunächst
  lesend und schreiben höchstens ihr eigenes Paketartefakt.
- Produktziel vom 27.08. (Probeeq-Doppelrolle, kein lokaler Voll-Editor) wird
  vom heutigen Vor-Umschnitt-Code getrennt behandelt; kein P6/P7-Vorgriff.

## Hauptrisiken

- Audiothread: Allokation, Locks, inkonsistente SPSC-Publikation,
  Blockgrößen-/Wrap-Ränder, stale Hostzustände.
- Messwahrheit: Epoch-/Segmentgrenzen, Quarantäne, NaN/Inf, Sample-Rate- und
  Integergrenzen, alte fertige Frames nach einem Bruch.
- Verträge/State: unterschiedliche C++-/Rust-/Python-Urteile, unbekannte
  Versionen, Save/Load-Asymmetrie, Verlust unbekannter Felder, falsche
  Aussageklasse.
- Zieltrennung: Identitätsleck aus dem gemeinsamen Kern, falsche
  Klassifikation, Scanner-/Render-Spawn, nicht vollständiger Rollback.
- Beweise: alte Binaries, selbstbestätigende Tests oder Behauptungen ohne
  aktuellen Lauf.

## Arbeitspakete

1. **Verträge und State (S5–S7):** Parser, Schemas, FlatBuffers, JCS,
   Migration, read-only, Host-Dirty und Cross-Language-Gleichheit.
2. **Realtime und Messkern (S3/S10–S13):** Hostbridge, Queue, Quarantäne,
   Loudness, FeatureEngine und Echtzeitregeln.
3. **Targets/Lifecycle/Installer (S8–S9):** CMake-Isolation, Identitäten,
   Bundle-State, Klassifikation, Neutralität, Install/Rollback.
4. **Integration (Parent):** Caller/Callee-Beziehungen, Broker-/Plugin-Glue,
   Beweisrunner, Widersprüche zwischen Paketen und aktuelle Baseline.

## Verifikations-Gates

- Jeder gemeldete Befund wird an der Source bestätigt oder als widerlegt
  protokolliert.
- Für einen Fix existiert möglichst ein vor dem Fix roter und danach grüner
  gezielter Test; andernfalls wird der Beleg enger und ehrlich benannt.
- Gezielt geänderte Ziele werden neu gebaut und gefahren.
- Abschließend läuft der kanonische Beweisrunner vom Workspace-Root mit dem
  aus dem Plan abgeleiteten Manifestpfad; keine Installation.
- Finaler Diff-Review auf NaN/Inf, Zahlenränder, Nebenläufigkeit, stale State,
  Save/Load-Symmetrie, Neutralität und UI-Wahrheit.

