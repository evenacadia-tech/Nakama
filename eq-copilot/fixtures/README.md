# Fixtures — Golden-Audio und eingefrorene Referenz (M1-Grundlage)

Erzeuger: `py -3.13 tools/eq-copilot/erzeuge_fixtures.py` — **deterministisch**
(feste Seeds/Phasen; SHA-256 der WAVs steht in `golden-referenz.json`). Die
WAVs sind deshalb nicht eingecheckt; eingecheckt ist nur die Referenz.

| Signal (20 s · 48 kHz · Stereo · 24 bit) | Zweck |
|---|---|
| `sinus-1k-minus12` | Frequenz-/Pegelanker: LUFS ≈ −12,03 · Centroid ≈ 1000,1 Hz |
| `pink-minus20` | Breitband-Anker für LTAS-Neigung/Bandenergie · keine Resonanzen |
| `resonanz-116hz` | der §7.9-Fall: Pink-Bett + 116-Hz-Dauerton; Offline-Orakel findet ihn bei 118,3 Hz (+21,7 dB) — die Abweichung zeigt die Rasterauflösung, M1 braucht das Bassfenster §5.10.1 |
| `stille-mit-tick` | Gating-Pflichtfall §5.3: LUFS −inf (im JSON `null`), Analyzer meldet Kleinstpegel-„Resonanzen" — M1 muss hier `noch nicht messbar` liefern, nie einen Befund |

`golden-referenz.json` ist der **eingefrorene Vertrag** für die
M1-Kreuzvalidierung (LUFS, True Peak, Crest, Centroid magnitudengewichtet,
Rolloff, Flatness, Width/Corr, Band-%, LTAS-Kurve, Resonanzliste). ±inf/NaN
sind als `null` kodiert (striktes JSON — Rust-Parser). Neu einfrieren NUR mit
neuer `metrics_version` und begründetem Commit.
