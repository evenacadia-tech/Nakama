> **ARCHIV (21.08.2026). Leitstand-Karte 15.08. (Hub-App-Zeit). Keine Vorgabe mehr. Gültige Produktdefinition: CLAUDE.md (Wahrheitskern + Entscheide-Register); technischer Entwurf: docs/FL-Nakama-Sonden-Design-Entwurf.md.**

# Subsystemkarte Nakama (Legacy-Scope `eq-copilot`)

<!-- LEITSTAND-KARTE · Quelle: .leitstand/maps/index.json · beim Scope-Start von Hand
     angelegt (im Repo liegt kein Generator); beim nächsten Generator-Lauf ersetzen. -->

## Zweck und Grenze
Transparentes Sammler-VST3 (JUCE 8/CMake) plus Verträge und Design-Tokens von
**Nakama**. Der sichtbare Produktname wurde am 2026-08-15 festgelegt; interne
`EqCop*`-/Pipe-/Schemanamen sind kompatibilitätsrelevantes Legacy. Kanonischer
Produktplan: `FL-EQ-Copilot-Recherche.md`. Verbindlicher Zielvertrag der neuen
VST3-Hauptansicht: `eq-copilot/docs/NAKAMA-SPECTRAL-FIELD-BAUPLAN.md`. Der
Broker lebt NICHT hier, sondern in `plugin-hub-app/src-tauri/src/eq_copilot/`
(Scope src-tauri).

## Besitzer
`cpp-cmake` — Builds laufen über die VS-2022-BuildTools-CMake, nie über einen
zweiten, handgepflegten Projektbaum.

## Invarianten
- **beratung-statt-bearbeitung:** kein Parameter-, Automations- oder Audio-Schreibweg; Passthrough sampleidentisch, 0 Latenz, kein Tail.
- **audiothread-rein:** im `process`-Pfad keine Allokation, Sperre, Datei/Netz/Log; Überlast verwirft Analyseframes, nie Audio (Plan §9.1).
- **vertrag-vor-code:** IPC/Messung/Report nur über `schemas/*.schema.json`; Protokolländerungen erhöhen die Version, nie still.
- **tokens-generiert:** `plugin/src/LeitstandTokens.h` entsteht nur aus `design/tokens.json` via `design/gen-tokens.mjs`.
- **assets-code-native:** die aktive Front entsteht aus den einzeln verschiebbaren JUCE-Vektorbauteilen in `plugin/src/EqCopilotAssetKit.h`; keine Gesamtfront-Bitmap und keine scheinbaren Audio-Parameter.
- **engine-single-writer:** die `AnalyseEngine` gehört exklusiv dem Worker-Thread; UI/Host stellen Wünsche über Atomics, nur `snapshot()` ist threadsicher.
- **ui-nie-dsp:** Anzeige-Zustände (Glättung, Ansicht, Vergleich) leben nur im Editor auf der Snapshot-Kopie — die Engine kennt keine Anzeige-Parameter.
- **metrics-versioniert:** Schwellen/Toleranzen aus Plan §5.10 ändern sich nur über eine neue `kMetricsVersion`, nie still (Code-Stand: Metrics und Diagnose jeweils `m4.1-2026-08-15`).

## Verpflichtende Prüfungen
- Konfigurieren/Bauen: siehe `eq-copilot/README.md` (CMake VS17, x64)
- `EqCopNullTest.exe` — Bitgleichheits-/NaN-/Latenz-Nulltest, muss `NULLTEST OK` melden
- `EqCopGoldenTest.exe eq-copilot\fixtures` — M1-Kreuzvalidierung gegen golden-referenz.json, muss `GOLDEN OK` melden (WAVs vorher: `py -3.13 tools/eq-copilot/erzeuge_fixtures.py --nur-wav`)
- `EqCopPipeProbe.exe` — Ende-zu-Ende-Handshake gegen den laufenden Broker
- Broker-Seite: `cargo test --manifest-path plugin-hub-app/src-tauri/Cargo.toml eq_copilot`

## Gegenpfade
Install↔Uninstall (VST3-Datei kopieren/entfernen), Connect↔Disconnect
(Hello/Welcome ↔ Bye/Timeout, Reconnect mit Backoff), Save↔Load (Plugin-State:
Sensor-ID, Rolle, Label, Paar-ID) stets gemeinsam prüfen.
