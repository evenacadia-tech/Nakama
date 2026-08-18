# M1-Befund — Einzelinstanz-Messung, headless bewiesen

Stand 2026-08-13, Anschluss an M0 (Plugin beim User installiert). Prüfmaßstab:
Plan §11 M1 + §5.10. Alles hier Genannte wurde **ausgeführt und gesehen**.

## Was gebaut wurde

- **`AnalyseEngine`** (`plugin/src/AnalyseEngine.{h,cpp}`) — deterministische
  Messschicht, exklusiv im Worker-Thread (Single-Writer-Kontrakt; UI/Host
  stellen Wünsche nur über Atomics):
  - **Vier Welch-Stufen** auf dem Mono-Mix: 16384 Bass (< 200 Hz,
    Bassfenster-Pflicht §5.10.1, 341 ms) · **8192 Referenzachse** (exakt die
    Achse von `tools/analyze-track.py` — macht die Kreuzvalidierung scharf) ·
    4096 Mitten · 2048 Höhen. Komposition in Leistungs-DICHTE mit
    Übergabebändern 160–250 Hz und 1,6–2,5 kHz (dB-Blend über log-f).
  - **LTAS** 221 Bänder à 1/24 Okt ab 30 Hz, Lücken in **linearer Leistung**
    interpoliert (wie `np.interp` im Analyzer — dB-Interpolation läge an
    Ton-Flanken ~7 dB daneben, gemessen an Band 44/108,5 Hz des 116-Hz-Falls).
  - **LUFS** BS.1770 pyloudnorm-gleich (RBJ-Shelf +4 dB/1500 Hz/Q 0,7071 +
    Hochpass 38 Hz/Q 0,5 · 400-ms-Blöcke, 75 % Overlap · Gating −70 abs, −10
    rel mit deren >=/>-Feinheiten) + Short-Term 3 s.
  - **True Peak** 8×-Polyphase, Kaiser β 5,0, 161 Taps (scipy
    `resample_poly`-gleich konstruiert).
  - **Aktivität/Abdeckung §5.10.2**: 100-ms-Zellen, Gate −60 dBFS;
    1/3-Okt-Abdeckung über ein 1-dB-Pegelhistogramm pro Band (löst das
    Henne-Ei „Teppich steht erst am Ende fest" exakt statt per Näherung);
    Klassen belastbar ≥ 60 % / eingeschränkt ≥ 25 %.
  - **Resonanzkandidaten §5.10.3**: ≥ 6 dB über der 1/3-Okt-Hüllkurve,
    −6-dB-Breite ≤ 1/6 Okt, Persistenz über aktive Segmente (dauerhaft ≥ 50 %,
    zeitweise ≥ 15 %), **Abdeckungs-Gate: nur belastbare Bänder**. Frequenz =
    Band-Center — nie feiner als das zuständige Fenster.
  - **Live-Hüllkurve** 3-s-EMA (§5.10.1-Anzeigekurve), nur aus der zuständigen
    Stufe (kein Naht-Flackern), nur aktive Segmente.
- **Editor**: LTAS-Graph im Leitstand-Look (Hauptlinie Komposit, Live-EMA
  leise, Vergleichslinie), Anzeige-Glättung 1/3–1/24 (Default 1/6),
  Roh-/Neigungsansicht (+3 dB/Okt um 1 kHz, Pink = eben), Resonanz-Marker
  (gefüllt = dauerhaft), Abdeckungs-Schattierung (nicht messbare Zonen werden
  still), Zustandshinweis nach §5.3, Statuszeile mit LUFS I/S · TP · Crest.
  Snapshot-Knopf = Messstand festhalten (Datei + Vergleichslinie), Neu messen.
  **Alle UI-Zustände leben nur im Editor und wirken auf die Snapshot-KOPIE —
  die Engine hat keine Anzeige-Parameter (Abnahme „UI beeinflusst DSP nicht"
  ist Architektur, kein Versprechen).** Kurven werden linear über die
  Stützstellen gezeichnet (monoton, erfindet keine Spitzen).
- **Snapshot-Datei** nach `%LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\`
  (Vertrag: `schemas/eq-snapshot.schema.json`; NaN→null; `raw_audio` immer
  null). `keineDaten` schreibt nichts.
- **Golden-Test** `EqCopGoldenTest` (viertes CMake-Target): SHA-256-Riegel auf
  den deterministischen WAVs, füttert die Engine blockweise und vergleicht
  gegen `fixtures/golden-referenz.json`.

## Headless bewiesen

| Prüfung | Werkzeug | Ergebnis |
|---|---|---|
| Kreuzvalidierung gegen analyze-track (4 Signale, 88 Prüfungen: LUFS ±0,3 · TP ±0,3/0,4 · Crest ±0,3 · Centroid ±3 % · Rolloff ±1/6 Okt · low_frac ±0,02 · Band-% ±1 Pp · Width/Corr ±0,02 · LTAS-Referenzachse mean ≤ 0,5/max ≤ 2 dB · Komposit mean ≤ 1,2/max ≤ 3,5 dB) | `EqCopGoldenTest` | **GOLDEN OK 88/88** |
| 116-Hz-Fall: **genau ein** Kandidat, dauerhaft, Persistenz ≥ 50 %, ±1/6 Okt (Referenz-Orakel fand ihn nur auf 118,3 Hz — das Bassfenster liefert die Aussage) | `EqCopGoldenTest` | ok |
| Sinus: genau 1 Kandidat bei 1 kHz; die **6 Leakage-„Resonanzen" der Referenz** (3–17 kHz, ~130 dB unter Träger) werden vom Abdeckungs-Gate geschluckt; ≤ 4 belastbare 1/3-Okt-Bänder | `EqCopGoldenTest` | ok |
| Stille-mit-Tick: nie messbereit, **0 Befunde** (Referenz listet 6 Kleinstpegel-Funde), LUFS null, TP/Crest treffen (−58,9 dBTP/43,0 dB) | `EqCopGoldenTest` | ok |
| Passthrough weiter bitgleich · Latenz 0 · Tail 0 · NaN gezählt, nie gefiltert · Mono ja/5.1 nein · State stabil | `EqCopNullTest` | **NULLTEST OK** |
| VST3-Lebenszyklus, Editor, Bus-Layouts, State-Recall, Parameter-Fuzz | `pluginval --strictness-level 8` | **SUCCESS** |
| Broker-Verträge unverändert grün | `cargo test eq_copilot` | 12/12 ok |
| Ende-zu-Ende C++↔Rust über echte User-ACL-Pipe (M1-Binary) | `eqcop-broker-probe` + `EqCopPipeProbe` | **PROBE OK** |

## Entscheidungen der Startkalibration (metrics `m1-2026-08-13`)

Änderung nur über neue Metrikversion, nie still (§5.10):

- **Randbin-Angleichung der Skalar-Achse:** analyze-track zählt alle
  |X|²-Bins gleich; die einseitige DC/Nyquist-Halbierung der Dichte-PSD wird
  für die Skalare rückgängig gemacht. Ohne das kippen die
  Energie-VERHÄLTNISSE messbar — das Pink-Fixture trägt ~46 % seiner Energie
  unter 20 Hz (1/f-Formung bis 0,05 Hz), gemessen als rolloff 3867 statt 2760.
- **Aktivitäts-Gate fix −60 dBFS** (Segment-RMS bzw. 100-ms-Zellen-RMS). Der
  adaptive Teil aus §5.10.2 (Teppich + 12 dB) ist **V1-ausgesetzt** —
  Kalibrationskandidat für M3 mit echtem Material.
- **Komposit-Kurvenvergleich nur für Breitband:** Schmalband-Bandpegel sind
  physikalisch auflösungsabhängig (ENBW ±3 dB pro Fensterverdopplung,
  Hauptkeulenbreite). Beim Sinus prüft der Test strukturell (Peak bei 1 kHz),
  beim 116er ist ±1/6 Okt um den Ton ausgenommen. §5.10.1 wörtlich: nie mehr
  Präzision behaupten, als das Fenster hergibt.
- **Flatness** läuft auf der eigenen 48-kHz-Achse (Referenz: librosa @ 22050
  mit eigenem amin-Boden) — im Test nur Größenordnung, im Snapshot als eigene
  Achse dokumentiert.
- **LTAS-Anzeige-dB = 'spectrum'@8192-Äquivalent** (Dichte + ENBW-Offset
  1,5·fs/8192): die Zahlen im Plugin decken sich mit den
  analyze-track-Tafeln des Users.

## Bekannte Grenzen (ehrlich, nicht heimlich)

- Messung akkumuliert bis „Neu messen" — Passagen-/Transportfenster kommen in
  M2 (Loop-Erkennung, zeitausgerichtete Snapshots). `kZellen` wächst mit
  ~10 Werten/s (8 h ≈ 2,3 MB) — unkritisch, Reset vorhanden.
- `stille → sammelt`: der Zustand `keineDaten` gilt nur vor dem ersten Sample.
- Editor-Statuszeile ist bei < ~700 px gedrängt; R3.1-Kompaktlayout des
  Mockups (aggregierter Statusknopf) kommt mit der Befund-UI in M3/M4.
- Kein Heartbeat-Transport der Messwerte — Protokoll v1 bleibt bis M2 stabil.

## Nur in FL prüfbar (User)

Installierte Kopie in `Program Files\Common Files\VST3` durch
`eq-copilot/install/EQ-Copilot.vst3` **ersetzen** (UAC) — sonst lädt FL das
M0-Binary ohne Graph. Danach: FL-TESTANLEITUNG §M1 (Graph bei Wiedergabe,
116-Hz-Gegenprobe mit einem Sinus-Generator, Snapshot-Datei erscheint,
16-Instanzen-Last, Offline-Render).
