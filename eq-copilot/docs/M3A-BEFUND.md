# M3a-Befund — FPS-Fix + Benchmark-Mechaniken (Nacht 15.08.2026)

Auslöser: **User-GO** auf die Benchmark-Studie
(`BENCHMARK-STUDIE-RESO-SMARTEQ-PROQ.md`) plus User-Meldung: *„der Autopilot
rendert den EQ-Graphen aktuell mit extrem wenig FPS — die visuelle Darstellung
ist genauso wichtig wie die Usability."* Alles hier Genannte wurde
**ausgeführt und gesehen**. Versionen: metrics **m4-2026-08-15** · diagnose
**m4-2026-08-15** · Snapshot-Datei **v3**.

## 1 · FPS-Diagnose: gemessen, nicht geraten

Eigenes Messwerkzeug gebaut (`EqCopPaintBench`, bleibt im Baukasten): rendert
den echten Editor mit echter 20-s-Messung offscreen und misst reine
paint()-Kosten.

| Messung | Wert |
|---|---|
| paint() @ 1200×832 (VORHER, Material-Kit-Stand) | **avg 2,42 ms** · p95 2,65 · Obergrenze ~413 FPS |
| paint() @ 1950×1352 (VORHER) | avg 3,85 ms · ~260 FPS |
| paint() @ 1200×832 (NACHHER, m4) | avg 2,90 ms · ~345 FPS |

**Befund: paint() war unschuldig.** Der wahrgenommene Ruckel kam aus der
**Datenkadenz**: der Worker leerte den FIFO alle 50 ms, rief `auswerten()`
aber nur jeden 5. Tick — der veröffentlichte Snapshot (und damit Live-Kurve,
Meter, Statuszellen) änderte sich mit **4 Hz**, während der Editor 30×/s
dasselbe Bild neu malte. Sichtbares Ergebnis: Viertelsekunden-Stakkato =
„extrem wenig FPS".

## 2 · Der Fix (zwei Hälften)

1. **`auswertenLeicht()`** (Engine, neuer 20-Hz-Pfad): publiziert jeden
   Worker-Tick Live-Kurve, Kurz-LUFS, True Peak, Crest und Zustand über den
   gemeinsamen Baustein `fuelleBasis()` — EINE Quelle, der Leichtpfad kann
   nie von der 250-ms-Schwerauswertung abweichen. Ohne neue Samples
   publiziert er nichts (Revision steht).
2. **Repaint-Disziplin** (Editor): EIN Snapshot-Zug pro Timer-Tick in die
   Anzeige-Kopie; gemalt wird nur bei neuer **Revision**, Werkzeug-/
   Ansicht-Änderung, Pipe-Statuswechsel oder Statusmeldungs-Fenster.
   Leerlauf-Riegel auch am Schwertakt (keine identischen 4-Hz-Snapshots im
   Stillstand). **Im Leerlauf malt der Editor exakt nichts** — FL bleibt ruhig.

Bewusst NICHT gebaut: Gitter-/Ebenen-Bitmap-Caches und Regionen-Buchhaltung —
bei 2,4 ms paint wäre das Komplexität ohne Nutzen gewesen (die Messung hat
die Entscheidung getroffen, nicht das Bauchgefühl).

Sichtbare Datenrate: **4 Hz → ~20 Hz** (Worker-Takt), Editor-Malrate folgt
den Daten (~20 FPS) statt blind 30 Hz.

## 3 · M3a — die drei Benchmark-Mechaniken im Kern

### 3.1 Median-Basislinie (RESO-Prinzip)

Resonanz-Prominenz wird jetzt gegen den **Median ±6 Bänder (~1/2 Okt)**
gemessen statt gegen das Mittel ±4 — das Mittel wurde vom Peak selbst
hochgezogen und unterschätzte den Überstand. Gilt identisch im
Kandidaten-Detektor UND in der Segment-Persistenz (`excessSegmente`), damit
beide auf demselben Maßstab laufen. NaN-ehrlich (Nyquist-Kappe), 'nearest'
an Bereichsrändern, < 5 endliche Nachbarn ⇒ keine Basislinie.

### 3.2 Zeitverlauf (EXPOSE-/RESO-Erbe): Zonen-Persistenz + Perzentile

- **Zonen-Ticks:** je 1 s AKTIVER Zeit ein Tick in `verarbeite()` (nicht am
  Auswerte-Takt — deterministisch, GoldenTest-beweisbar; in Pausen vergeht
  keine Tick-Zeit). Pro Diagnose-Zone (neu: geteiltes `ZonenRegeln.h` —
  Engine und Diagnose können nicht mehr auseinanderlaufen) wird die Live-EMA
  gegen die Schulterlinie geprüft; der Snapshot trägt Anteil + Tick-Zahl.
- **Diagnose:** `kt.persistenz` der Zonen-Befunde kommt jetzt aus dem echten
  Zeitverlauf (ab 15 Ticks) statt pauschal 0,5 — **Zonen-Befunde können
  ehrlich „hoch" erreichen**; Karten sagen „Das war in NN % der Musikzeit
  so." (Lernsprache, nur bei belegtem Verlauf).
- **Perzentile P10/P50/P95** je Band aus dem VORHANDENEN 1-dB-Pegelhistogramm
  (nur Auslesen, keine neue Akkumulation). P95−P50 steuert die Charakter-
  Zeile der Resonanzkarten: „Pegel steht ruhig" (fester Absenker sicher) vs.
  „kommt in Wellen" (dynamisches Werkzeug).

### 3.3 Lernfenster-Konvergenz (smart:EQ-/Pro-Q-Erbe): „KURVE STEHT"

Alle ~10 s Aktivzeit wird das Komposit gegen den Stand von vor einem Fenster
verglichen (mittlere + maximale Bewegung über beidseitig endliche Bänder).
Steht die Kurve (< 0,35 dB im Mittel — Startwert), zeigt die Display-
Mikroleiste **„KURVE STEHT"**. Bewusst **rein informativ** (UI + Snapshot):
ungetestete Schwellen gehören nicht über Nacht in die Konfidenz-Formel —
Kopplung erst nach Alltagsbeobachtung.

### 3.4 Notennamen in FL-Zählung (RESO-Erbe + Landmine beachtet)

Resonanzkarten tragen die Note im Titel und im Gemessen: **116 Hz = A#3**
(FL zeigt MIDI 60 als C5 ⇒ Oktave = MIDI div 12 — die tools/midi-ideen-
Landmine). Der 116-Hz-Trick der Testanleitung (§8: 3x Osc auf A#3) und die
Karte nennen jetzt dieselbe Note.

## 4 · Beweise (alles ausgeführt und gesehen)

| Prüfung | Werkzeug | Ergebnis |
|---|---|---|
| Kern-Referenzen unverändert (Median bricht keine Kreuzvalidierung; 116er: db_over im Fenster, GENAU 1 Kandidat) | `EqCopGoldenTest` | ok |
| **Halbzeit-Mulm** (neues Fixture `diag-mulm-halb`, +10-dB-Glocke nur in 2. Hälfte — +7 hätte die Halbzeit-Leistungsmittelung unter die 4-dB-Schwelle gedrückt): Karte erscheint, Zeitverlauf-Anteil in [0,30 · 0,70], Karte nennt „% der Musikzeit" | `EqCopGoldenTest` | ok |
| **Wandernder Ton** (neues Fixture `diag-wander-ton`, 4×5 s auf Pink-Bett): ≥ 2 Kandidaten, ALLE zeitweise (Persistenz ~25 %), nie dauerhaft, dynamische Erstidee | `EqCopGoldenTest` | ok |
| Pink: alle 4 Zonen-Zeitverläufe belegt mit Anteil ≤ 0,10 (Falsch-Positiv-Riegel des Zeitverlaufs) · Perzentile belegt + P10≤P50≤P95 überall | `EqCopGoldenTest` | ok |
| 116-Hz-Titel nennt **A#3** | `EqCopGoldenTest` | ok |
| **Leichtpfad**: publiziert ohne Schwerlauf (Kurz-LUFS, TP, Live-Kurve, Revision), Komposit bleibt Schwerlauf-Sache, **Leerlauf publiziert nicht** | `EqCopGoldenTest` | ok |
| **Konvergenz**: nach 1. Fenster kein Wert, nach 2. gültig; stationäres Pink mittlere Bewegung < 1 dB | `EqCopGoldenTest` | ok |
| Gesamt | `EqCopGoldenTest` | **GOLDEN OK 239/239** |
| Passthrough bitgleich, State stabil | `EqCopNullTest` | **NULLTEST OK** |
| VST3-Lebenszyklus, Editor, Fuzz | `pluginval --strictness-level 8` | **SUCCESS** |
| Editor-Sichtprüfung mit echter Messung (Kurve, Marker, Meter, „1 Auffälligkeit", Broker-LED grün) | `EqCopShot` → `scratchpad/eqcop-m3a-shot.png` | **SHOT OK** |
| Referenz-JSON-Diff | git | rein additiv (nur 2 neue WAV-SHAs — Kern bitidentisch, Determinismus belegt) |

Headless-Lehre dieser Nacht: `Thread::sleep` pumpt keine Message-Loop, und
JUCE 8 verriegelt `runDispatchLoopUntil` hinter `JUCE_MODAL_LOOPS_PERMITTED`
— der offizielle Weg für Editor-Timer in Konsolen-Tests ist
**`juce::Timer::callPendingTimersSynchronously()`** (ShotTest/PaintBench
brauchen ihn jetzt, weil paint() die Timer-Kopie liest).

## 5 · Grenzen / bewusste Entscheidungen

- Konvergenz fließt NICHT in die Konfidenz (nur UI/Snapshot) — erst nach
  Alltagsbeobachtung der realen Werte auf even-Material.
- „KURVE STEHT" braucht ≥ ~21 s durchgehende Aktivzeit (zwei Fenster) — im
  20-s-Shot fehlt es korrekt.
- Zonen-Ticks nutzen die 3-s-EMA: Zustandswechsel schmieren ~3 s nach
  (Halbzeit-Fixture misst deshalb 0,3–0,7 statt exakt 0,5 — dokumentierte
  Physik, kein Fehler).
- Charakter-Zeile (P95−P50) misst Band-Dynamik (inkl. Musik-Dynamik), nicht
  isolierte Resonanz-Dynamik — Formulierung entsprechend vorsichtig.
- M3b (Korridor aus even-Bounces) + M3c (PRE/POST-Karte, Hover-Inspekt,
  Passagen, Masking) stehen als nächste Stufen in der Benchmark-Studie.

## 6 · Install-Stand (wartet auf den User — UAC braucht den Klick)

Gestaged: `eq-copilot\install\EQ-Copilot.vst3` — SHA
`C9AD6ADF189F1DD740900BBFD72CAAACBC94329C487EBAC1755C73FB8C9D558F`
(inkl. Selbst-Audit-Fix: `Component::SafePointer` statt rohem `this` im
Messpunkt-Popover-Callback — FL darf das Editorfenster mit offenem Popover
schließen, ohne dass der Panel-Destruktor in den abgebauten Editor schreibt).
Rollback gesichert: `install\rollback-8FB5EE05-before-m3a.vst3` (= aktuell
installierter Material-Kit-Stand `8FB5EE05…`).
**Morgens einmal:** FL beenden → `eq-copilot\install\Install-EQ-Copilot.ps1`
als Administrator ausführen (Rechtsklick → „Mit PowerShell ausführen" +
UAC bestätigen) → `install-result.txt` muss `STATUS=OK` + den neuen Hash
zeigen. Danach FL-TESTANLEITUNG **§17** (~5 min).
