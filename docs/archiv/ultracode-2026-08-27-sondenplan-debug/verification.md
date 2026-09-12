# Verifikation

Stand: 27.08.2026, Abschluss auf dem integrierten Arbeitsbaum.

## Kanonischer Gesamtbeweis

- Befehl: `pwsh -File tools/beweise.ps1 -Bauen -Titel 'Sondenplan-Debug Abschluss 2026-08-27'`
- Endmanifest: `docs/beweise/lauf-2026-08-27-0326.md`
- Urteil: **GRÜN — 29/29 Kanon-Läufe, Exit 0**.
- Der Runner baute 19 Releaseziele einschließlich aller drei aktuellen
  VST3-Bundles, `NakamaKern` und des neuen `EqCopPipeClientTest` frisch.
- Zwei Tests bleiben korrekt als erst ab P6 geplant und ungebaut markiert:
  `EqCopDspGoldenTest` (B6) und `EqCopTransactionTest` (B7).

Der unmittelbar vorherige Lauf
`docs/beweise/lauf-2026-08-27-0322.md` war ehrlich rot (28/29): Nur A9 fand
Zeilenendendrift in der generierten Rust-FlatBuffers-Datei. Neugenerierung aus
dem unveränderten `.fbs` mit dem gepinnten `flatc 25.12.19` ergab denselben
Text in CRLF und danach **Drift 0**. Das rote Manifest bleibt als Gegenbeleg
erhalten; es wird nicht als Abschlussbeweis verwendet.

## Wichtige frische Teilbelege

| Prüfung | Ergebnis |
|---|---:|
| Rust-Broker Unit / Cross-Language | 90/90 + 8/8 |
| `cargo clippy --all-targets -- -D warnings` | Exit 0 |
| Python-v3-Referenz | 216/216, 169 JSON-Fixtures |
| C++ v3-Schema | 61/61, 90 Textriegelfälle, 169 JSON- und 55 FlatBuffers-Fixtures |
| State/Migration/DTO/JCS | 153/153; State-Generator 36 Dateien bytegleich |
| Queue/Worker | 114/114; zusätzlich fünf Wiederholungsläufe 114/114 |
| FeatureEngine | 235/235 |
| Loudness | 109/109 |
| PipeClient-Lifecycle | Exit 0; absolute 320-ms-Framefrist und Lebenszyklen grün |
| Installer-Manifest / Sandbox-Gegenpfad | 38/38 + 80/80 |
| Suna / Probeeq Nulltest | je 62/62 |
| Identität / Lifecycle | 114/114 + 61/61 |

Alle Broker-E2E- und PipeClient-Tests verwendeten eindeutige Probe-Pipenamen.
A18 lief nur unter `%TEMP%`; keine Installation oder Deinstallation wurde
ausgeführt.

## Abschluss-Hygiene

- `git diff --check` über die eigene Implementierungsfläche (`broker`,
  `eq-copilot`, `tools`, Planstand und offenen Punkt) meldet keinen Fehler;
  die Konsole zeigt nur die bestehende LF/CRLF-Konvertierungswarnung.
- Der vollständige Worktree-Check findet ausschließlich zwei bereits fremd
  vorhandene Markdown-Hardbreaks mit abschließenden Leerzeichen in
  `design/docs/interaktions-und-motion-spezifikation.md`. Sie wurden gemäß
  Parallelbetriebsregel nicht verändert.
- Die generierte Rust-FlatBuffers-Datei ist nach Neugenerierung inhaltlich
  diff-frei und besteht den bytegenauen A9-Riegel.
- Parallel vorhandene Änderungen in `CLAUDE.md`, `design/**`,
  `docs/FL-Nakama-Sonden-Design-Entwurf.md` und `docs/plugin-wissen.md` wurden
  weder umformatiert noch zurückgesetzt.

## Nicht durch diesen Lauf behauptet

Der Beweis ersetzt keinen echten Hostscan, keinen Release-/Signaturbeleg,
keine Admininstallation, keinen Power-Loss-Test und keine Sanitizer- oder
plattformübergreifende Schedulergarantie. Die fachliche Restfrage NAK-83 und
die weiteren Grenzen stehen in `integration.md`.
