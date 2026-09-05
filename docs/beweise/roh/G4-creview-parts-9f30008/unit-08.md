# unit-08 — spike/AuxSpikeProcessor.cpp, spike/AuxSpikeProcessor.h, src/AnalyseEngine.cpp, src/AnalyseEngine.h, src/Diagnose.cpp, src/Diagnose.h
Gelesen: 58 von 58 Einheiten vollständig · Zeilen: 2476 (die sechs Dateien umfassen 2560 Zeilen; die Differenz sind die Leerzeilen zwischen den Einheiten, ebenfalls gelesen)

HEAD beim Lesen: `9f3000881df632f4969d78551fb0a516b61288cb`. Bedrohungsmodell LOCAL_UNPRIVILEGED.

## Befunde

- [LOW] `src/AnalyseEngine.cpp:270` · integer/division · **`zellenSamples` wird 0, sobald der Host eine Samplerate ≤ 4 Hz meldet; danach schließt die 100-ms-Zelle bei JEDEM Sample und `zelleAktivEnergie / zellenSamples` teilt durch 0.** · Auslöser: `prepareToPlay(sr, …)` mit `0 < sr ≤ 4`. Die einzigen Sanitizer (`src/PluginProcessor.cpp:182-184`, `sonde/SondeProcessor.cpp:127-128`) prüfen nur `isfinite`, `> 0.0`, `≤ 768000.0`; `AnalyseEngine::vorbereiten:171-173` prüft dieselben drei Bedingungen und übernimmt die Rate. `zellenSamples = (int) std::lround (0.1 * sr)` ist dann 0. · Wirkung: `src/AnalyseEngine.cpp:385` (`++zellenStand >= 0`) ist ab dem ersten Sample wahr; `src/AnalyseEngine.cpp:388` rechnet `zelleAktivEnergie / 0` → bei Signal `+inf`, bei Stille `NaN` (IEEE-Division, **kein** UB, keine Speicherverletzung). Bei `+inf` wächst `aktiveZellen` um 1 **pro Sample**, also `s.aktivSekunden` (`:748`) um 0,1 s pro Sample — der Zustand springt nach 150 Samples auf `messbereit` und behauptet Minuten aktiver Musikzeit, die es nicht gibt (verletzt „Messzeit"-Konfidenz in `src/Diagnose.cpp:252/332/470`). Zusätzlich feuert `zonenTick()` (`:396`, 221 Bänder × 4 Zonen × 3 Regionen) alle 10 Audiosamples auf dem Analyseworker — bei realer 48-kHz-Blockzustellung rund 4800 Ticks/s statt 1/s. · Erreichbar: **nein** über FL Studio (Kette `prepareToPlay` → `samplerateAtomic`/`v3Samplerate` → Worker → `engine.vorbereiten`, `src/PluginProcessor.cpp:190`+`788-792`, `sonde/SondeProcessor.cpp:131`+`303-306`); die Rate stammt ausschließlich vom Host, **nicht** aus Pipe, State oder Datei — geprüft per Grep über alle Schreiber von `samplerateAtomic`/`v3Samplerate`. Ein Wrapper oder Testhost, der `sr ≤ 4` meldet, erreicht ihn. · Beleg: `:171-173`, `:270`, `:385-401`, `:748-752`. · Register: neu.

- [LOW] `spike/AuxSpikeProcessor.cpp:37` · bounds/macro-contract · **`Schnappschuss::versatz` sichert `busse[busIndex]` und `busse[0]` allein mit `jassert`, das im Release-Build zu nichts expandiert; dort ist der Zugriff ungeprüft.** · Auslöser: ein Aufruf von `versatz(i)` mit `i >= busse.size()` oder auf einem leeren `busse`. · Wirkung: OOB-Lesen eines `BusBefund` (`juce::String`-Member ⇒ Dereferenzierung eines Fremdzeigers, nicht nur ein Zahlenfehler). · Erreichbar: **nein** im heutigen Stand — alle Aufrufer prüfen zuerst `hatVersatz()`, das über `versatzGrund` (`:16-33`) sowohl `busIndex == 0` als auch `busIndex >= busse.size()` abfängt: `spike/AuxSpikeProcessor.cpp:80-82`, `spike/AuxSpikeEditor.cpp:132-134`, `plugin/tests/AuxSpikeTestMain.cpp:165-168/220`. Der Befund ist der fehlende Riegel, nicht ein erreichbarer Pfad: `hatVersatz` ist eine Bitte an den Aufrufer, kein Vertrag der Funktion. · Beleg: `:35-39` gegen `:16-33`. · Register: neu.

- [LOW] `spike/AuxSpikeProcessor.cpp:43,57` · alloc-lifetime · **`wurzel` und `eintrag` sind rohe `new juce::DynamicObject`, die erst am Funktionsende (`:96`, `:100`) an ein `juce::var` übergeben werden; wirft eine der `setProperty`-/`Array::add`-Zeilen dazwischen, leckt der Block.** · Auslöser: `std::bad_alloc` in `setProperty`/`add` bei erschöpftem Speicher. · Wirkung: Speicherleck von einigen hundert Byte je Bericht; kein Doppel-Free, kein UAF — die Refcount-Übergabe an `juce::var` selbst ist korrekt (Refcount 0 → 1, Freigabe mit dem temporären `var`). · Erreichbar: nur über Speichererschöpfung; Nachrichtenthread, kein Audiopfad. · Beleg: `:41-101`. · Register: neu.

Keine weiteren Befunde. Insbesondere geprüft und **sauber**:

- **Ringgrenzen der Welch-Stufen** (`src/AnalyseEngine.cpp:105-108,156-158`): `gefuellt` steht beim Schreiben immer `< n`, nach jedem Segment auf `n - hop = n/2`; `memmove` kopiert `(n-hop)*sizeof(float)` aus `[hop,n)` nach `[0,n/2)` — Länge, Ziel und `sizeof` des Pointees stimmen.
- **`bandBinVon/Bis`** (`:92-99`): `bis` wird auf `n/2+1` geklemmt, `bandBinBis = max(von,bis)`; ein `von` jenseits von `n/2+1` erzeugt ein **leeres** Intervall, das `segmentInBaender:453` zusätzlich abfängt. `einzelPsd` hat in jedem Aufruf genau `n/2+1` Elemente derselben Stufe (`:127`), auch bei geteiltem `scratchPsd` über vier Stufen mit verschiedenem `n`.
- **Zuständigkeits- gegen Akkubereiche** (`:427-435` gegen `:449,469,477,492`): `[zustVon,zustBis) ⊆ [akkuVon,akkuBis)` gilt für alle vier Aufrufe (200 ≤ 250, 160 ≤ 200, 2000 ≤ 2500, 1600 ≤ 2000); die drei Zuständigkeitsbereiche sind untereinander disjunkt, `pegelHistogramm`, `teppichInaktivDb` und `liveEmaLinear` werden deshalb nie doppelt gezählt. Die Referenzachse (`zustVon = -1`) wird bei `:468` und `:485` sauber abgezweigt.
- **`medianUmBand`** (`:41-62`): `jlimit(von, bis-1, b+j)` mit `von < bis` in allen drei Aufrufstellen (`:496`, `:977`), 13 Werte in einem 13er-Feld, `n < 5` liefert ehrlich NaN.
- **`lround` auf möglicherweise nicht-endlichen Werten**: `:502` und `:944` sind die einzigen Stellen; beide Ergebnisse laufen unmittelbar durch `juce::jlimit(0, kHistStufen-1, …)`, ein implementierungsdefiniertes `lround`-Ergebnis kann den Index also nicht aus `pegelHistogramm` heraustragen. In `src/Diagnose.cpp:16,27,73,180,210` ist der Eingang jeweils vorher endlich geprüft (`:25` fängt NaN durch `! (hz > 0.0)`, `:207` durch `isfinite`).
- **Sortier-Komparatoren** (`:993`, `src/Diagnose.cpp:355,493`): kein NaN kann in die sortierte Menge gelangen (Kandidaten brauchen `excess >= 6.0`, Zonen-Deltas entstehen aus Mitteln endlicher Werte mit von Null verschiedenem Nenner), die strikt schwache Ordnung bleibt gültig.
- **Aufrufervertrag von `verarbeite`** (`src/AnalyseEngine.h:188-190`): beide Produktaufrufer (`src/PluginProcessor.cpp:937`, `sonde/SondeProcessor.cpp:359`) liefern `frei.audio` aus `Blockquarantaene::puffer` mit `maxBlockFrames * 2 = 32768` floats und `frames = sampleCount ≤ maxBlockFrames`; `2·frames` Werte sind damit garantiert, auch im Mono-Fall (`StampedAudioQueue.h:263-268`, L-Duplikat auf R). `(int) sampleCount` kann nicht negativ werden (Layout-Cap 16384), ein negativer Wert würde bei `:340` ohnehin abgewiesen.
- **Single-Writer-Vertrag** (`src/AnalyseEngine.h:181-184`): `vorbereiten`/`zuruecksetzen`/`verarbeite`/`auswerten`/`auswertenLeicht`/`lautheitFuerTelemetrie` werden ausschließlich aus den beiden Analyseworkern gerufen (Grep über alle Aufrufstellen); `snapshot()` ist der einzige fremde Einstieg und läuft unter `snapMutex` (`:1123-1127`).
- **`Biquad`** (`core/analysis/KGewichtung.h:41-42`) ist vollständig defaultinitialisiert; `zuruecksetzen()` vor dem ersten `vorbereiten()` arbeitet auf leeren Vektoren ohne Zugriff (`:314-315`).

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren, Allokationen, I/O, Logging — HÄLT.**
   `spike/AuxSpikeProcessor.cpp:149-245` ist der einzige `processBlock` meiner Einheiten: `ScopedNoDenormals`, ausschließlich Atomics (`exchange`/`load`/`store`/`fetch_add`), keine Allokation, kein Lock, kein Dateizugriff, kein Logging; der Reset kommt als Flagge herein und wird im Audiothread abgeräumt (`:155-166`), der Gegenpfad `messungZuruecksetzen()` (`:285-288`) setzt nur die Flagge.
   `AnalyseEngine` berührt den Audiothread nie: `verarbeite` (`src/AnalyseEngine.cpp:338`, alloziert über `scratchPsd.resize`), `auswerten`/`auswertenLeicht`/`finalisiereSkalar` (`:843`, `std::vector`) und `snapshot()` (`:1123`, Rückgabe mit zwei Vektoren + `std::mutex`) laufen nachweislich nur auf Worker- bzw. Nachrichtenthread (`src/PluginProcessor.cpp:788-968`, `sonde/SondeProcessor.cpp:298-366`, `src/PluginEditor.cpp:135/287/779`, `src/PluginProcessor.cpp:1318/2337`).
2. **Passthrough bitidentisch, 0 Latenz, kein Tail — HÄLT.**
   `spike/AuxSpikeProcessor.cpp:149-245` liest nur (`sicht.getReadPointer`, `:190`) und schreibt nie in `buffer`; der Kommentar `:243-244` ist durch die Quelle gedeckt. `getTailLengthSeconds() == 0.0` (`spike/AuxSpikeProcessor.h:100`), keine `setLatencySamples`-Zeile in der Datei. `AnalyseEngine` bekommt nur eine Kopie aus der Quarantäne und hat keinen Schreibpfad zum Host (`const float*`, `src/AnalyseEngine.h:190`).
3. **Gate 5 (kein Telemetrie-/Steuerframe steuert Gain oder erzeugt PRE/POST-Delta) — HÄLT.**
   In meinen Einheiten gibt es keinen Eingang aus Pipe oder Modell. Der einzige Berührungspunkt ist `lautheitFuerTelemetrie()` (`src/AnalyseEngine.cpp:710-736`) und der geht **hinaus** (Engine → Telemetrieframe, `sonde/SondeProcessor.cpp:366-370`), nie hinein. `AuxSpikeProcessor::setStateInformation` ist leer (`spike/AuxSpikeProcessor.h:109`).
4. **NaN-Ehrlichkeit — HÄLT.**
   Nicht-endliche Eingangssamples werden vor jeder Rechnung durch Stille ersetzt und gezählt (`src/AnalyseEngine.cpp:348-357`, `nanErsetzt` → `:437`, `:745`). Nyquist wird gekappt: `ltasBisBand = min(18 kHz, 0.95·fs/2)` (`:202-208`), angewandt in `finalisiereLtas:641`, `interpoliereLuecken:644` und `fuelleBasis:760` — oberhalb bleibt NaN statt fortgeschriebener Randevidenz. Ohne genügend endliche Nachbarn gibt es keine Basislinie (`medianUmBand:52-53`, `n < 5` ⇒ NaN; `excess`-Ersatz `-1e18` bei nicht endlicher Kurve, `:982-984`). Ungültige Messungen werden als ungültig gemeldet statt als 0 (`lufsGueltig`, `spektralGueltig`, `perzentileGueltig`, `konvergenzGueltig`, `protokollOk`/`lautAnteil = -1` im Spike). Beim Impulsdetektor ist das NaN-Verhalten ausdrücklich richtig herum: jeder Vergleich mit NaN ist falsch, also löst NaN nie einen Impuls aus (`spike/AuxSpikeProcessor.cpp:197-206`).
   Eine ehrliche Einschränkung, kein Befund: `fftDaten` ist `float` (`src/AnalyseEngine.h:216`). Endliche, aber astronomisch große Eingangssamples (≳ 2·10³⁴) könnten die FFT-Summe zu `±inf` überlaufen lassen; das `inf` wandert dann in `akku.summe` und in die LTAS-dB-Werte. Speichersicher bleibt es (`jlimit` bei `:501`, `isfinite`-Riegel bei `:982`, `:1095`), und mit Hostaudio ist der Wert unerreichbar — deshalb hier vermerkt statt als Befund geführt.
5. **State verlustfrei — NICHT BERÜHRT.**
   Meine Einheiten halten keinen persistenten Zustand. `AuxSpikeProcessor::getStateInformation`/`setStateInformation` sind bewusst leer (`spike/AuxSpikeProcessor.h:108-109`, Wegwerf-Messgerät ohne Produktidentität, `:8-10`); `MessSnapshot` ist ein reiner Laufzeitwert.

## Ledger

spike/AuxSpikeProcessor.cpp:35-39 · Befund 1 (LOW, jassert-only bounds)
spike/AuxSpikeProcessor.cpp:41-101 · Befund 1 (LOW, alloc-lifetime bei bad_alloc)
spike/AuxSpikeProcessor.cpp:105-115 · clean
spike/AuxSpikeProcessor.cpp:117-122 · clean
spike/AuxSpikeProcessor.cpp:124-147 · clean
spike/AuxSpikeProcessor.cpp:149-245 · clean
spike/AuxSpikeProcessor.cpp:247-250 · clean
spike/AuxSpikeProcessor.cpp:252-283 · clean
spike/AuxSpikeProcessor.cpp:285-288 · clean
spike/AuxSpikeProcessor.cpp:290-296 · clean
spike/AuxSpikeProcessor.cpp:298-311 · clean
spike/AuxSpikeProcessor.h:file-scope-1 · clean
spike/AuxSpikeProcessor.h:17-146 · clean
src/AnalyseEngine.cpp:file-scope-1 · clean
src/AnalyseEngine.cpp:34-34 · clean
src/AnalyseEngine.cpp:41-62 · clean
src/AnalyseEngine.cpp:65-100 · clean
src/AnalyseEngine.cpp:102-160 · clean
src/AnalyseEngine.cpp:163-167 · clean
src/AnalyseEngine.cpp:169-275 · Befund 1 (LOW, zellenSamples == 0 bei sr ≤ 4)
src/AnalyseEngine.cpp:277-336 · clean
src/AnalyseEngine.cpp:338-438 · Befund 1 (LOW, Division durch zellenSamples == 0; Folgestelle desselben Befunds)
src/AnalyseEngine.cpp:440-514 · clean
src/AnalyseEngine.cpp:517-593 · clean
src/AnalyseEngine.cpp:596-626 · clean
src/AnalyseEngine.cpp:628-690 · clean
src/AnalyseEngine.cpp:692-708 · clean
src/AnalyseEngine.cpp:710-736 · clean
src/AnalyseEngine.cpp:741-785 · clean
src/AnalyseEngine.cpp:787-798 · clean
src/AnalyseEngine.cpp:801-836 · clean
src/AnalyseEngine.cpp:838-927 · clean
src/AnalyseEngine.cpp:929-963 · clean
src/AnalyseEngine.cpp:965-1046 · clean
src/AnalyseEngine.cpp:1048-1121 · clean
src/AnalyseEngine.cpp:1123-1127 · clean
src/AnalyseEngine.h:file-scope-1 · clean
src/AnalyseEngine.h:file-scope-2 · clean
src/AnalyseEngine.h:172-174 · clean
src/AnalyseEngine.h:file-scope-3 · clean
src/AnalyseEngine.h:file-scope-4 · clean
src/Diagnose.cpp:file-scope-1 · clean
src/Diagnose.cpp:10-10 · clean
src/Diagnose.cpp:13-18 · clean
src/Diagnose.cpp:23-33 · clean
src/Diagnose.cpp:36-47 · clean
src/Diagnose.cpp:51-61 · clean
src/Diagnose.cpp:65-75 · clean
src/Diagnose.cpp:87-108 · clean
src/Diagnose.cpp:111-123 · clean
src/Diagnose.cpp:131-152 · clean
src/Diagnose.cpp:156-257 · clean
src/Diagnose.cpp:269-339 · clean
src/Diagnose.cpp:341-344 · clean
src/Diagnose.cpp:346-492 · clean
src/Diagnose.cpp:493-508 · clean
src/Diagnose.cpp:510-518 · clean
src/Diagnose.h:file-scope-1 · clean

## Nicht gelesen

keine

## Außerhalb meiner Einheiten

(unbewertet, nur notiert)

- `eq-copilot/plugin/core/StampedAudioQueue.h:508-526` — `lies()` kopiert `2 · block.sampleCount` floats in ein `float* ziel`, dessen Größe die Funktion nicht kennt; die Zusage `sampleCount ≤ Layout::maxBlockFrames` liegt allein beim Produzenten.
- `eq-copilot/plugin/src/PluginProcessor.cpp:182-184` und `eq-copilot/plugin/sonde/SondeProcessor.cpp:127-128` — der Samplerate-Sanitizer akzeptiert jeden Wert in `(0, 768000]`, auch Raten unter 5 Hz (Eingang des ersten Befunds oben).
- `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h` — Verhalten von `vorbereiten(sr)` und `zelle()` bei sehr kleinen Samplerates nicht untersucht.
- `eq-copilot/plugin/spike/AuxSpikeEditor.cpp:132-147` — Anzeigepfad des Spike-Schnappschusses; prüft `hatVersatz` vor `versatz` (nur als Beleg des Aufrufervertrags gelesen).
- `eq-copilot/plugin/src/AnalyseEngine.cpp:794` (in meiner Einheit, aber ein Produkt- und kein Sicherheitspunkt) — `auswertenLeicht()` frischt nur die leichten Felder in `fertig` auf; unmittelbar nach dem Überschreiten von 15 s Aktivzeit kann ein Snapshot `zustand == messbereit` melden, während `resonanzen`/`abdeckung` noch aus dem letzten schweren Lauf mit `zustand == sammelt` stammen (leer). Selbstheilend nach ≤ 250 ms, `diagnose()` prüft beide Flaggen.
