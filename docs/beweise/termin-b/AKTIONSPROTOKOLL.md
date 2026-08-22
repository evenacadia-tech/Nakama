# Termin B — Aktionsprotokoll (Claude über FL-MCP), 22.08.2026

Uhrzeiten = Wanduhr des Rechners (Get-Date), Reihenfolge ist die Wahrheit.
Aufbau (per MCP verifiziert): Channel 1 „Grand Piano" → Mixer-Kanal 1, dort Slot 0 = `EqCop-Host-Probe`.
Automationsclip „EqCop-Host-Probe - Testwert (nur Messung)" im Playlist, dichte Zickzack-Kurve (Screenshot des Users).
**Berichtigt nach T2 (22.08.):** die Dauern/Takte in der ersten Fassung waren aus den
unzuverlässigen MCP-Positionsanzeigen geschätzt. Aus den Ereignissen des Berichts:
Tempo 140 bpm (1,71 s je Takt), Song nach Verlängerung 7,14 Takte = 539 634 Samples
= 12,24 s (Loop-Wrap alle ~3 003 Blöcke); nach dem Export 509 119 Samples = 11,54 s.
Die Uhrzeiten unten sind die Wahrheit; die Klammerangaben dazu sind die berichtigten Werte.

## Vorlauf (VOR der Verlängerung des Songs — steht ebenfalls im Probe-Log)
- ≈12:53–12:56: Song-Modus, Position 0, Play (Ereignisse 6–10: 2 Loop-Wraps à 179 747 Samples = 4,08 s + 84 922 Samples ≈ 10 s, dann Pause bei 84 922 — Ursache ungeklärt; Song war 2,4 Takte); erneut Play (Ereignisse 12–29: Wiedereinstieg 69 Samples VOR der Pauseposition, 14 Wraps ≈ 57 s), Stop.

## Messungen (nach Verlängerung)
| Zeit | Aktion | Klicklisten-Schritt |
|---|---|---|
| ≈12:57:40 | Song-Modus, Position 0, Play | 3 Live (bis zum ersten Seek 5 Song-Wraps + 300 037 Samples ≈ 68 s, Ereignisse 31–37) |
| 12:58:50 | Seek mit `mode=2` und Wert 10 — **das sind 10 Ticks** (1 969 Samples), also ein RÜCKsprung (Ereignis 38, −298 131) | 4 (rückwärts, nicht wie beabsichtigt vorwärts) |
| 12:59:04 | Seek `mode=2`, Wert 2 = 2 Ticks (394 Samples), Rücksprung −18 996 (Ereignis 40) | 4 (rückwärts) |
| 12:59:16 | Seek `mode=2`, Wert 8 = 8 Ticks (1 575 Samples), Rücksprung −533 347 (Ereignis 41) | 4 (rückwärts) |
| 12:59:41 | Stop | 4b |
| ≈12:59:55 | Position → 5 s (im Stop) | 4b |
| 13:00:01 | Play | 4b (Positionswechsel über Stop/Play) |
| 13:00:18 | Umschalten Pattern-Modus (läuft weiter) | 5 Schleife (Pattern-Wrap 173 250 Samples = 9,2 Beats ≈ 2,3 Takte, 8 Wraps: Ereignisse 51–58) |
| 13:00:42 | Ende Schleifenfenster | 5 |
| ≈13:00:50 | Stop; zurück in Song-Modus | — |
| (User) | Smart disable im Wrapper-Menü eingeschaltet („hab es an gemacht") | 6b |
| 13:02:45 | Piano-Channel STUMM (fl_mute_channel), Position 0, Play | 6b Stille (bis 13:03:07 = 22 s; Ereignisse 61–66, 3 Wraps à 3 003 Blöcke, kein Vorwärtssprung) |
| 13:03:07 | Piano wieder laut | 6b Ton (8 s) |
| 13:03:15 | Stop | — |
| (User) | Export → WAV, Fenster wieder öffnen, „Bericht sichern" | 7 + Abschnitt 4 |

## Nachmessung (13:10–13:12, mit `mode=1`)

`fl_set_song_position(mode=1)` ist laut FL-API **Sekunden** (`SONGLENGTH_S = 1`), nicht
Millisekunden, wie die MCP-Beschreibung behauptet; die Werte 9000/2000/10000 wurden
modulo der Songlänge 509 119 Samples (11,545 s) eingeordnet — deshalb landeten sie
„zufällig" brauchbar: 296 494 / 122 456 / 103 163 Samples (Ereignisse 79, 84, 82).

| Zeit | Aktion | Ereignis |
|---|---|---|
| 13:10:22 | Play ab 0 | Abschnitt 75–86 |
| 13:10:27 | Seek `mode=1` 9000 → 296 494 Samples, ohne Stop | 79 `zeitsprung_vor` +49 901 |
| 13:10:42 | Seek `mode=1` 2000 → 122 456 Samples, ohne Stop | 84 `zeitsprung_zurueck` −38 559 (nach Wrap 83) |
| 13:10:57 | Seek `mode=1` 10000 → 103 163 Samples, ohne Stop | 82 `zeitsprung_vor` +65 960 |
| 13:11:12 | Stop | 86 |
| 13:11:35 | Piano STUMM, Position 0, Play (Smart disable an) | Abschnitt 88–96: 5 Wraps à 2 833 Blöcke, kein Vorwärtssprung |
| 13:12:14 | Piano wieder laut (Stille damit 39 s) | — |
| 13:12:22 | Stop | 96 |
| 13:26:44 | User: „Bericht sichern" (zweiter Bericht) | — |

Lesart für die Auswertung: meine Seeks sind die Sprünge 38/40/41 (rückwärts, Ticks) und
79/82 (vorwärts), 84 (rückwärts); alle übrigen `zeitsprung_zurueck` sind Loop-Wraps mit
exakt Song- bzw. Pattern-Länge; `zeitsprung_ueber_stop` 5 = Ereignisse 13/46/69/76/89
(jeder Play nach Stop/Pause mit Positionswechsel); Smart-Disable-Fenster 61–66 und 88–96
ohne Vorwärtssprung und mit regelmäßiger Wrap-Kadenz ⇒ keine Lücke (bei offenem Fenster).

**FL-MCP-Positionsmodi (an der Quelle verifiziert, `transport.setSongPos(position, mode)`
reicht `mode` durch; FL-Konstanten `SONGLENGTH_MS = 0`, `SONGLENGTH_S = 1`,
`SONGLENGTH_ABSTICKS = 2`):** 0 = Millisekunden, 1 = Sekunden, 2 = absolute Ticks
(96 je Beat) — die Werkzeugbeschreibung (0 = Prozent, 1 = ms, 2 = s) ist falsch.
