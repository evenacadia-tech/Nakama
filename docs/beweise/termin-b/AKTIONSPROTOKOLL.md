# Termin B — Aktionsprotokoll (Claude über FL-MCP), 22.08.2026

Uhrzeiten = Wanduhr des Rechners (Get-Date), Reihenfolge ist die Wahrheit.
Aufbau (per MCP verifiziert): Channel 1 „Grand Piano" → Mixer-Kanal 1, dort Slot 0 = `EqCop-Host-Probe`.
Automationsclip „EqCop-Host-Probe - Testwert (nur Messung)" im Playlist, dichte Zickzack-Kurve (Screenshot des Users).
Song-Länge nach Verlängerung ≈ 4 Takte, ≈ 3,5 s je Takt → Song-Loop alle ≈ 14 s im Song-Modus.

## Vorlauf (VOR der Verlängerung des Songs — steht ebenfalls im Probe-Log)
- ≈12:53–12:56: Song-Modus, Position 0, Play ≈20 s (Song war nur ≈2 Takte, FL loopte im Sekundentakt), dann stand „is_playing=false" bei 2:02:23 (Ende/Stop unklar); erneut Play ≈10 s (Positionen 2:02 → 2:13 → 2:09 = Loop), Stop.

## Messungen (nach Verlängerung)
| Zeit | Aktion | Klicklisten-Schritt |
|---|---|---|
| ≈12:58:22 | Song-Modus, Position 0, Play | 3 Live (≈20 s; enthält ≥1 Song-Loop-Rücksprung bei ≈14 s) |
| 12:58:50 | Seek → 10 s, OHNE Stop | 4 (vorwärts) |
| 12:59:04 | Seek → 2 s, OHNE Stop | 4 (rückwärts) |
| 12:59:16 | Seek → 8 s, OHNE Stop | 4 (vorwärts) |
| 12:59:41 | Stop | 4b |
| ≈12:59:55 | Position → 5 s (im Stop) | 4b |
| 13:00:01 | Play | 4b (Positionswechsel über Stop/Play) |
| 13:00:18 | Umschalten Pattern-Modus (läuft weiter) | 5 Schleife (Pattern = 1 Takt, ≈3 Durchläufe) |
| 13:00:42 | Ende Schleifenfenster | 5 |
| ≈13:00:50 | Stop; zurück in Song-Modus | — |
| (User) | Smart disable im Wrapper-Menü eingeschaltet („hab es an gemacht") | 6b |
| 13:02:45 | Piano-Channel STUMM (fl_mute_channel), Position 0, Play | 6b Stille 12 s |
| 13:03:07 | Piano wieder laut | 6b Ton 8 s |
| 13:03:15 | Stop | — |
| (User) | Export → WAV, Fenster wieder öffnen, „Bericht sichern" | 7 + Abschnitt 4 |

Erwartung für die Auswertung: Seeks ohne Stop = 3 Sprungereignisse (12:58:50 vor, 12:59:04 zurück, 12:59:16 vor) plus periodische Loop-Rücksprünge (Song ≈14 s, Pattern ≈3,5 s); genau EIN Positionswechsel über Stop/Play (13:00:01); Smart-Disable-Sprung(e) vorwärts nur im Fenster 13:02:45–13:03:15; Automationspunkte während aller Song-Modus-Abschnitte.
