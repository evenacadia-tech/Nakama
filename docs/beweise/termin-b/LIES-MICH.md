# FL-Termin B — Hostzeit/Automation: gemessen am 22.08.2026

Messgerät `EqCop-Host-Probe` (S3b, Wegwerfware `NkHp`), installiert unter
`C:\Program Files\Common Files\VST3\` (FL scannt VST3 nur dort). Klickliste:
`eq-copilot/docs/FL-TERMIN-B-HOSTZEIT.md`. **Arbeitsteilung:** der User legte
Plugin, Piano und Automationsclip an, schaltete Smart disable ein, exportierte
und klickte „Bericht sichern"; Abspielen, Springen, Schleife, Stummschalten
fuhr Claude über den FL-MCP (`fl-studio`) — Uhrzeiten in `AKTIONSPROTOKOLL.md`.

## Aufbau (per MCP verifiziert)

- Channel „Grand Piano" → Mixer-Kanal 1, dort Slot 0 = `EqCop-Host-Probe`,
  Fenster offen, „Hostbrücke liefert: JA".
- Automationsclip auf „Testwert (nur Messung)", dichte Zickzack-Kurve
  (Screenshot des Users 22.08. ~12:45).
- Song ≈ 7 Takte bei 140 bpm (Loop-Länge 539 438 Samples = 12,23 s), 44 100 Hz.

## Ergebnis (zweiter Bericht `host-probe-20260822-132644.json`, enthält den ersten)

| Frage der Klickliste | Messung |
|---|---|
| Hostbrücke / Kontext | **JA**; Kontext in **259 298 von 259 298** Blöcken; alle sieben Felder (`project_time_samples`, `continous_time_samples`, `tempo`, `ppq_position`, `bar_position`, `cycle_bounds`, `sample_rate`) **immer** gültig — auch im Stand (Block 0, Projektzeit 0) |
| Live | 20 s + weitere Abschnitte; `projektzeit_negativ` 0 |
| Springen ohne Stop | **wird gemeldet**: Rücksprünge −298 131 / −18 996 / −533 347 (Durchgang 12:58–12:59), Vorwärtssprünge **+49 901** und **+65 960** sowie Rücksprung −38 559 (Nachmessung 13:10) — `zeitsprung_vor` 2, `zeitsprung_zurueck` 51 (inkl. Loop-Wraps) |
| Springen mit Stop | **eigene Zeile**: `zeitsprung_ueber_stop` 5 (jeder Play nach Stop mit Positionswechsel), Sprung-Zeilen dabei unverändert |
| Schleife | Loop-Wraps als Rücksprünge mit **exakt** der Songlänge (−539 634/635) bzw. Pattern-Länge; `cycle_bounds` immer gültig (`schleife_an` 8×, Ende 28,6 / 27,0 / 9,5 Beats) |
| Automation (wichtigste Messung) | **83 303 Punkte, nie mehr als EIN Punkt je Block, alle bei Offset 0**, `samplegenau_belegt: false`. Blockgrößen **1 … 4 096** Samples: FL zerteilt die Puffer an den Automationspunkten (≈ 180–190 Samples je Block bei 140 bpm ≈ 1 Tick), statt Punkte mit Offsets in die VST3-Queue zu legen |
| Smart Disable | **keine Lücke** — 12 s und 30 s stumme Wiedergabe (Channel gemutet, Smart disable im Wrapper an): `zeitsprung_vor` nur durch meine Seeks, `block_ohne_verarbeitung` 0. Bedingung: Plugin-Fenster war offen |
| Export (Render) | `offline_an`/`offline_aus` je 1, **2 587 Offline-Blöcke**, Projektzeit läuft im Render kontinuierlich (Stop bei 510 300) |
| float / double | **nur float** (259 298 / 0) |
| Presentation-Latency je Bus | **gemeldet**: Eingang Bus 0 = **3 924**, Ausgang Bus 0 = **4 410** Samples (= 100 ms); `verworfene_wertwechsel` 1 — ein späterer anderer Wert wurde von der Probe verworfen, welcher, steht nicht im Bericht (Befund für den Report) |

Nebenbefunde: Play nach Pause setzte 69 Samples **vor** der Pauseposition wieder
ein (Ereignis 13: 84 922 → 84 853); der FL-MCP setzt Positionen nur im Modus 1
(Millisekunden) brauchbar — Modus 2 („Sekunden") landet nahe am Songanfang,
daher war die erste Sprungrunde rein rückwärts.

Capability-Folge (S4, `docs/beweise/SONDE-004.md`): `host_context_presence`
und `project_time_samples` supported · `sample_accurate_automation`
**unsupported** (Fallback Blockrampe — passt zu FLs Puffer-Zerteilung) ·
`presentation_latency` gemeldet · `float64_processing` unsupported.

Rohdaten: die zwei JSON-Dateien in diesem Ordner (unverändert kopiert); der
zweite Bericht ist der vollständige (die Probe sammelt seit dem Laden).
Nicht gemacht: Teil 3 der Klickliste (`nakama-altprojekt.flp`, optional) —
die Legacy-`.flp`-Fixture aus SONDE-002 bleibt offen.
