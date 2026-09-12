# FL-Diagnoseprojekt für den Laufzeit-Arm

`Nakama-Diagnose.flp` ist das Projekt, das der User am 12.09.2026 für den
Laufzeit-Arm (Register NAK-284, KONZEPT §4.6) in FL Studio 2026 (26.1.4.5589)
angelegt hat. Es wird nur gelesen, nie aus der Automatik heraus gespeichert.
Archivkopie hier; Arbeitskopie beim User unter
`C:\Users\phili\OneDrive\Desktop\Skripte\Project_1\Project_1.flp`.

## Aufbau (gemessen 12.09.2026 über die FL-Scripting-API, nicht aus der Datei geraten)

| Element | Stand |
|---|---|
| Channel 1 `Testtrack` | Audioclip `Samples\Testtrack.wav` (44,1 kHz, Stereo, 124 s; byteidentisch mit `eq-copilot/kalibration/Testtrack.wav`), Ziel Mixerspur 1 |
| Channel 0 `Sampler` | leerer Standardkanal, Ziel Master; ohne Funktion |
| Mixerspur 1 `Insert 1`, Slot 1 | **Nakama Probeeq** (`plugin_kind` `active_probe` im gespeicherten `NakamaState`) |
| Master, Slot 1 | **EQ-Copilot** (Gen; `plugin_kind` `legacy` im gespeicherten `NakamaState`) |
| Routing | Insert 1 → Master, Sendpegel 0,8 (FL-Standard) |
| Songlänge | 27 Takte, 46 s; Loop-Modus Song |
| Automationsclip | keiner (optional, noch nicht angelegt) |
| Plugin-Fenster, Smart disable | aus der Datei nicht prüfbar; Vorgabe: Fenster offen, Smart disable aus |

## Samplepfad

FL speichert den Samplepfad absolut (`…\Project_1\Samples\Testtrack.wav`).
Wer das Projekt von hier aus startet, legt vorher denselben Pfad oder einen
Ordner `Samples\Testtrack.wav` neben der Projektdatei an (Quelle:
`eq-copilot/kalibration/Testtrack.wav`); die WAV liegt bewusst nicht doppelt
im Repo. Ein fehlendes Sample öffnet in FL einen Dialog und blockiert die
Automatik.

## Bekannte Falle beim Lesen der Datei

`pyflp` (2.x) liest das FL-2026-Format nicht (Kopf ja, Kanäle nein); die
Rohdatei ist größtenteils ein Container-Ereignis. Struktur deshalb immer
live über die Scripting-API messen (`transport.getLength`, `channels.getAll`,
`plugins.isValid`/`getName` je Spur und Slot, `mixer.getRouting`).
Die Längen-Modi im Controller-Skript des MCP sind vertauscht: FL liefert
`getSongLength(1)` in Sekunden, `(2)` in absoluten Ticks, `(3)` in Takten.
