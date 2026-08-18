# M0-Befund — Beweis-Spike und Verträge

Stand 2026-08-13, Nacht-Baulauf nach GO. Prüfmaßstab: Plan §11 M0.
Alles hier Genannte wurde **ausgeführt und gesehen**, nicht angenommen.

## Headless bewiesen

| Prüfung | Werkzeug | Ergebnis |
|---|---|---|
| Passthrough bitgleich (1000 Blöcke Rauschen) | `EqCopNullTest.exe` | ok |
| Latenz 0 Samples · Tail 0 s | `EqCopNullTest.exe` | ok |
| NaN/Inf: gezählt, Signal unverändert | `EqCopNullTest.exe` | ok |
| Mono angenommen · 5.1 klar abgelehnt | `EqCopNullTest.exe` | ok |
| State Save→Load: Sensor-ID/Rolle/Paar-ID stabil | `EqCopNullTest.exe` | ok |
| VST3-Lebenszyklus, Editor, Bus-Layouts, State-Recall, Parameter-Fuzz | `pluginval --strictness-level 8` | **SUCCESS** |
| Framing (Grenzen, Abbruch, kein UTF-8, feindliches 4-GiB-Präfix) | `cargo test eq_copilot` | 12/12 ok |
| Handshake/Reject/Heartbeat/Konflikt über echte Named Pipe | `cargo test eq_copilot` | ok |
| **Ende-zu-Ende C++↔Rust**: Plugin-Client → User-ACL-Pipe → Broker → welcome+Token → Heartbeats → bye → Sensor sichtbar getrennt | `eqcop-broker-probe.exe` + `EqCopPipeProbe.exe` | **PROBE OK** |

Artefakt: `build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3`
(JUCE 8.0.9, VS 2022 Build Tools, x64 Release).

## Claude-Spike (Plan §6.4)

`claude --bare -p … --output-format json --json-schema … --disallowedTools "*"`
in leerem Arbeitsverzeichnis, CLI 2.1.229. Ergebnis
(`claude-spike/bare-lauf-2026-08-13.json`): **„Not logged in"** bei 0 Tokens,
0 Turns — `--bare` überspringt Hooks/Plugins/MCP wie gewünscht, nutzt aber die
OAuth-Anmeldung nicht. Damit ist der im Plan antizipierte Konflikt
**bare-Isolation ↔ Abo-Auth real bestätigt**.

**Gate-Empfehlung:** CLI-Weg nur mit `--settings`-basiertem `apiKeyHelper`
weiterverfolgen oder direkt den **Agent-SDK-/API-Adapter** bauen (Plan §6.4
Punkt 8). Kein unisolierter CLI-Aufruf im Produktkern — ohne `--bare` liefen
die globalen Hooks dieses Rechners mit.

## Nur in FL prüfbar (offene M0-Punkte → FL-TESTANLEITUNG.md)

- FL scannt/lädt/speichert das Plugin; Projekt schließen/neu öffnen
- 16 Instanzen ohne Dropout; Bridging/Smart Disable/Offline-Render
- `projectTimeSamples`/Transport bei Play/Stop/Seek/Loop (Anzeige im Broker-Status)
- PDC-Verhalten mit Latenzplugin vor einem Sensor
- GUI bei 100–250 % Skalierung und FL-Wrapper-Resize

## Vorbefund außerhalb dieses Scopes

`scanner::tests::scans_real_db` (plugin-hub-app) ist rot: „3x Osc → Synth"
schlägt fehl, weil `3x Osc.fst` aktuell in **keinem** Kategorie-Ordner der
FL-Favoriten-DB liegt — die parallele Harness-Session baut `plugin-auswahl.json`
gerade um (uncommitted). Kein Zusammenhang mit dem EQ-Copilot-Diff (Test misst
die Platte, one-writer respektiert, nichts angefasst). Die übrigen 710 Tests
plus die 12 neuen laufen grün.
