# Offene Punkte — durables Offen-Set

> Regeln: Offene Einträge werden beim Auffrischen NIE gelöscht. Schließen
> nur mit ID + Commit-SHA (oder expliziter User-Entscheidung). Neue
> Out-of-scope-Funde kommen als datierte Zeile mit ID dazu.
> Nur Nakama-/eq-copilot-Punkte — FL-Studio-Workspace-Themen (MIDI,
> Vorlagen, Plugin-Hub-GUI) bleiben dort.

## Offen (Stand 2026-08-17)

| ID | Seit | Punkt |
|---|---|---|
| NAK-01 | 08-17 | **Design-Freeze ausstehend:** User kalibriert das Tiefenfeld-Musterblatt im Browser; „Werte kopieren"-JSON in den Chat = Freeze → JUCE-Voreinstellung. |
| NAK-02 | 08-17 | **JUCE-Umbau der Hauptansicht** nach Bauplan 2.0 §13 (12 Schritte) — startet erst NACH NAK-01. |
| NAK-03 | 08-15 | Beim User: FL-TESTANLEITUNG §11–17 (M2-Roundtrip · M3-Karten · Graph/„KURVE STEHT") + §18 Hör-Markierung (Puls/Solo hören, Render-Beweis). Plugin 0.3.0 ist installiert. |
| NAK-04 | 08-15 | 16-Instanzen-/PDC-/Offline-Render-Verhalten = Alltagsbeobachtung, User meldet formlos. |
| NAK-05 | 08-14 | M3b: Korridor-Kalibration aus even-Bounces (Benchmark-Studie, nächste Stufe). |
| NAK-06 | 08-14 | M3c: PRE/POST-Karte, Hover-Inspekt, Passagen, Masking (Benchmark-Studie). |
| NAK-07 | 08-14 | Dauerbetrieb: Loudness inkrementell, Pipe-Deadlines. |
| NAK-08 | 08-14 | Claude-Anbindung: `--bare` scheitert an OAuth → Agent-SDK-/API-Adapter nötig (M4-Thema; Grundgesetz: Aufruf nur auf bewussten Klick). |
| NAK-09 | 08-15 | Kalibrier-Protokoll Runde 2 (helle Instrumente ODER EQ-Fehler-Blindtest), `eq-copilot/kalibration/`. |
| NAK-10 | 08-17 | Broker liest das `hoermarkierung`-Heartbeat-Flag noch nicht (serde verwirft still; `protokoll.rs` MessStand ohne Feld) — Aggregat soll laut `eq-ipc.schema.json` während Markierung pausieren/markieren. Bewusst nach dem Harness-Lauf, s. HOER-MARKIERUNG-KONZEPT. |

## Geschlossen

_(noch leer — Schließungen hier mit ID + SHA eintragen, nicht löschen)_
