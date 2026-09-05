# unit-03 — core\analysis\{FeatureEngine.h (3552–4313), Fft.h, KGewichtung.h, Konfidenz.h, LoudnessAccumulator.h, Statistik.h, TruePeak.h, Vergleichspegel.h (1–268 + Dateirumpf)}
Gelesen: 53 von 53 Einheiten vollständig · Zeilen: 2485

Kontext mitgelesen (nicht Befundfläche): `BandGrid.h` (Quantisierung, Bitmaps, Gittergrenzen),
`BandGridZahlen.h` (Live-Gruppentabelle), `FeatureEngine.h` 500–900/1540–1690/2900–3040,
`src/AnalyseEngine.cpp` 150–400/690–790, `src/PluginProcessor.cpp` 180–215/490–560/780–800/1800–1830/2020–2030,
`tests/Sonde013ExperimentGoldenTest.cpp` 90–280.

## Befunde

- [LOW] `core/analysis/Fft.h:45` · return-values/Fehlerpfad (halber Zustand) + bounds · **Der Idempotenz-Riegel prüft nur `cosTab`; wirft eine der vier folgenden `assign`-Allokationen, kehrt ein Wiederholungsaufruf früh zurück und lässt `sinTab`/`umkehr`/`re`/`im` leer.** Auslöser: `std::bad_alloc`/`length_error` in `sinTab.assign` (Z. 54), `umkehr.assign` (Z. 62), `re.assign` (Z. 72) oder `im.assign` (Z. 73) — bei `kBassPunkte = 16384` sind das 128-KiB-Blöcke, die ein lokaler Prozess unter Speicherdruck zum Scheitern bringen kann, nachdem `cosTab` (Z. 53) schon stand. Wirkung: `punkte` bleibt auf `n` gesetzt (Z. 47, vor allen Allokationen), der nächste `vorbereiten(n)` nimmt Z. 45 (`n == punkte && ! cosTab.empty()`) und meldet „fertig"; das anschließende `transformiere()` ist `noexcept` und schreibt `re[z] = eingang[i]` (Z. 86) über `umkehr[i]` (Z. 85) in leere Vektoren — Heap-Lese- UND -Schreibzugriff außerhalb. Erreichbar: **heute nein.** Aufrufkette `workerLauf` (PluginProcessor.cpp:796) → `FeatureEngine::vorbereiten` (Z. 986) → `Stufe::vorbereiten` (Z. 1585) → `Fft::vorbereiten`; auf diesem Pfad fängt niemand (`grep catch` über `plugin/src`, `plugin/sonde`: nur `PipeClient.cpp:242`), die Ausnahme verlässt den `std::thread`-Rumpf und beendet den Prozess vor dem zweiten Aufruf. Der Riegel wäre trotzdem falsch, sobald ein Aufrufer die Ausnahme je einfängt — und `merkmale.vorbereiten` läuft je Workerrunde in einer Schleife, der zweite Aufruf käme sofort. Beleg: Fft.h:45–47, 53–54, 62, 72–73, 85–86. Register: neu.
- [LOW] `core/analysis/LoudnessAccumulator.h:169` · return-values/Fehlerpfad (halber Zustand) + bounds · **Dieselbe Klasse Fehler eine Datei weiter: der Bereitschaftstest prüft nur `binSumme.size()`, alloziert aber zwei Felder.** Auslöser: Allokationsfehler in `binAnzahl.assign` (Z. 172, 13071 × `uint32_t` ≈ 52 KiB), nachdem `binSumme.assign` (Z. 171) stand. Wirkung: jeder spätere `vorbereiten()` überspringt beide Zeilen, `sr` wird gesetzt (Z. 176) und `zelle()` schreibt `++binAnzahl[(std::size_t) bin]` (Z. 276) mit `bin` bis 13070 in einen leeren Vektor. Erreichbar: **heute nein**, gleiche Begründung wie oben — `AnalyseEngine::vorbereiten` (AnalyseEngine.cpp:273) liegt in `prepareToPlay`, ohne `catch`. Beleg: LoudnessAccumulator.h:169–178, 275–276. Register: neu.
- [LOW] `core/analysis/Statistik.h:132` · Konvertierung/UB (Vergleichsordnung) · **`fdrSignifikant` sortiert mit `pWerte[a] < pWerte[b]`; enthält `pWerte` ein NaN, ist der Vergleich keine strikte schwache Ordnung und `std::sort` ist undefiniert** (MSVC-Introsort kann beim Median-Partitionieren über das Ende laufen). Auslöser: ein einzelner nicht-endlicher p-Wert im Eingangsvektor. Wirkung: im besten Fall unsinnige Signifikanzbits, im schlechten ein Schreibzugriff außerhalb von `rang` (Z. 129, Länge `m`). Erreichbar: **heute nein** — einziger Aufrufer ist `tests/Sonde013ExperimentGoldenTest.cpp:204/218/227/231/270` mit sauberen Werten; im Produktcode ruft niemand die Funktion. Der Punkt ist trotzdem eine Vertragslücke: die Schwesterfunktion `blockBootstrap` hat genau diesen Riegel am 04.09.2026 als Nacharbeit 1 / Befund B34 bekommen (Statistik.h:83–87, Test Z. 157–178), `fdrSignifikant` und `cluster` blieben ohne. Beleg: Statistik.h:122–133 gegen Statistik.h:79–87. Register: neu (B34 ist nur für `blockBootstrap` geschlossen).
- [LOW] `core/analysis/Vergleichspegel.h:70` · Konvertierung/UB (float→integer) · **`mindestSamples = (std::uint64_t) (kMindestSekunden * fs)` prüft nur `> 0.0`, nicht Endlichkeit oder Bereich.** Auslöser: eine Hostrate `+inf` oder `> 1,8e19`; `NaN` und `<= 0` fängt der Ternär in Z. 69 ab, `+inf` nicht (`inf > 0.0` ist wahr). Wirkung: die Konvertierung eines außerhalb von `uint64_t` liegenden `double` ist UB (MSVC x64 liefert 0x8000…0), `mindestSamples` wird unerreichbar groß, `bereitIntern()` (Z. 319–320) nie wahr, der Vergleichspegel friert nie mit Wert ein — fail-closed, keine Speicherunsicherheit. Erreichbar: über `PluginProcessor.cpp:1817`, das die **ungeprüfte** `getSampleRate()` übergibt, während `PluginProcessor.cpp:211` dieselbe Methode mit der geprüften `sichereSamplerate` (isfinite, 0 < sr ≤ 768000, PluginProcessor.cpp:182–183) speist. Realistisch liefert FL keine solche Rate; der belastbare Teil des Befunds ist die Asymmetrie der zwei Aufrufstellen. Beleg: Vergleichspegel.h:66–72, 319–320. Register: neu.

Ausdrücklich **keine** Befunde (geprüft und gehalten), damit die Abwesenheit nachvollziehbar ist:
`Fft::transformiere` — Butterfly-Indizes `j*schritt < n/2` und `i+j+halb < n` sind für Zweierpotenzen dicht an der Grenze, aber innerhalb; die einzige Produktquelle von `n` sind die Konstanten `kBassPunkte = 16384` / `kHauptPunkte = 4096` (FeatureEngine.h:894–896), also nie ungerade oder negativ.
`TruePeakDetektor::tick` — `(stand - k) & (kRing - 1)` ist seit C++20 mit Zweierkomplement definiert, `k ≤ 24 < kRing = 32` (static_assert Z. 82), `h0`-Index max. 192 < 193.
`LoudnessAccumulator::zelle`/`binIndex` — jede endliche Blocklautheit landet dank Oberband in `[0, kBinsGesamt)`, der `bin >= kBinsGesamt`-Zweig ist geklemmt UND gezählt.
`Konfidenz::gesamtklasse` — NaN-Abdeckung fällt durch alle Vergleiche auf `unbrauchbar`, `klasseName` hat einen Rückfall für Fremdwerte.
`FeatureEngine::fuelleLive/fuelleEvidenz/fuelleVerteilung` — `Gitter::liveBisExkl` endet exakt bei 221 (BandGridZahlen.h:157–162) = Länge von `liveAkku`/`liveBreiteAkku`; `ringInZeitfolge` schreibt höchstens `kVerteilungPlaetze = 64` in Stapelpuffer derselben Länge, weil `VerteilungsRing::schiebe` (Z. 878–883) `gefuellt` deckelt.
`baueStempel` — die `sample_count`-Rechnung prüft den `uint64`-Überlauf vor der Addition (Z. 3649–3657), die Aggregatinitialisierung Z. 3634 trifft die Feldreihenfolge von `Support` (Z. 1619–1626).
`KGewichtung` — `entwerfen` wird nur hinter dem Ratenriegel FeatureEngine.h:971 aufgerufen, also kein `w0 = 2π·fc/0` und kein dauerhaft NaN-verriegelter Filterzustand.

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren/Allokationen/IO** — *hält*. Einziger Audiothread-Pfad in meinen Einheiten ist `Vergleichspegel::speise` (Vergleichspegel.h:92–126): ein `compare_exchange_strong` (Z. 245–247), Aufgeben statt Warten bei Misserfolg (Z. 96–97), keine Allokation, kein Log; das Warten liegt ausschließlich im `Steuerzug` des Nachrichtenthreads (Z. 259–267). `TruePeakDetektor::tick` (TruePeak.h:157–184) alloziert nicht und hat feste Schleifenlänge; der Speicher entsteht in `vorbereiten` (Z. 109–110) auf dem Nachrichtenthread, `koeffizienten()` wird dort erzwungen (Z. 102). `Fft::transformiere`, `LoudnessAccumulator::zelle` und alle `FeatureEngine::fuelle*` laufen im Worker (PluginProcessor.cpp:796) bzw. auf der Nachrichtenseite.
2. **Passthrough bitidentisch, kein Hostpuffer angefasst** — *hält*. Keine Einheit dieser Zuweisung schreibt in einen Hostpuffer; `speise` nimmt zwei `const float*` (Vergleichspegel.h:92), alle übrigen Einheiten lesen Akkumulatoren oder schreiben in eigene Frames.
3. **Gate 5 (kein Frame steuert Gain / erzeugt PRE-POST-Delta)** — *nicht berührt*. In meinen Einheiten wird kein Gain angewendet; `Vergleichspegel::gainDb()` (Z. 222) ist ein reiner Messwert hinter `gainGesetzt()`, und die einzige Anwendung liegt außerhalb (Blindvergleich/PluginProcessor).
4. **NaN-Ehrlichkeit (verriegelt UND gezählt)** — *hält, mit einer Ausnahme*. Belege: `LoudnessAccumulator.h:208–210` (ungültige Zelle wird als 0 gemerkt, aber als ungültig markiert), `:224–228` und `:235–240` (Block verworfen und gezählt), `TruePeak.h:161–162` und `:183`, `Vergleichspegel.h:114–118` (gezählt) mit `:192–202`/`:319` (verriegelt) und `:212`, `FeatureEngine.h:3985–3988` (Rahmenverriegelung M-07), `:3769`, `:3959`, `:4101`, `Konfidenz.h:158–159` (Sampleschaden ⇒ `unbrauchbar`), `BandGrid.h:148–152`/`236–239` (nicht-endlicher dB-Wert wird nie quantisiert). Ausnahme: `Statistik.h:122` ohne den B34-Riegel der Schwesterfunktion — Befund 3.
5. **State bleibt verlustfrei (unbekannte Major-Version read-only)** — *nicht berührt*: in meinen Einheiten liegt kein Persistenz- oder Migrationspfad.

## Ledger

core\analysis\FeatureEngine.h:3552-3701 · clean
core\analysis\FeatureEngine.h:3702-3851 · clean
core\analysis\FeatureEngine.h:3852-3911 · clean
core\analysis\FeatureEngine.h:3912-3972 · clean
core\analysis\FeatureEngine.h:3973-4112 · clean
core\analysis\FeatureEngine.h:4113-4113 · clean
core\analysis\FeatureEngine.h:4122-4122 · clean
core\analysis\FeatureEngine.h:4123-4123 · clean
core\analysis\FeatureEngine.h:4126-4126 · clean
core\analysis\FeatureEngine.h:4127-4127 · clean
core\analysis\FeatureEngine.h:4128-4128 · clean
core\analysis\FeatureEngine.h:file-scope-5 · clean
core\analysis\FeatureEngine.h:4141-4142 · clean
core\analysis\FeatureEngine.h:4152-4152 · clean
core\analysis\FeatureEngine.h:4156-4156 · clean
core\analysis\FeatureEngine.h:4207-4227 · clean
core\analysis\FeatureEngine.h:4228-4228 · clean
core\analysis\FeatureEngine.h:4244-4244 · clean
core\analysis\FeatureEngine.h:4275-4275 · clean
core\analysis\FeatureEngine.h:4279-4279 · clean
core\analysis\FeatureEngine.h:4280-4280 · clean
core\analysis\FeatureEngine.h:4301-4301 · clean
core\analysis\FeatureEngine.h:file-scope-6 · clean
core\analysis\Fft.h:file-scope-1 · clean
core\analysis\Fft.h:29-143 · Befund 1 (Fft.h:45)
core\analysis\KGewichtung.h:file-scope-1 · clean
core\analysis\KGewichtung.h:29-53 · clean
core\analysis\KGewichtung.h:63-78 · clean
core\analysis\KGewichtung.h:80-94 · clean
core\analysis\KGewichtung.h:104-104 · clean
core\analysis\KGewichtung.h:107-122 · clean
core\analysis\KGewichtung.h:124-124 · clean
core\analysis\Konfidenz.h:file-scope-1 · clean
core\analysis\Konfidenz.h:55-65 · clean
core\analysis\Konfidenz.h:88-194 · clean
core\analysis\LoudnessAccumulator.h:file-scope-1 · clean
core\analysis\LoudnessAccumulator.h:89-103 · clean
core\analysis\LoudnessAccumulator.h:158-161 · clean
core\analysis\LoudnessAccumulator.h:167-179 · Befund 2 (LoudnessAccumulator.h:169)
core\analysis\LoudnessAccumulator.h:182-331 · clean
core\analysis\LoudnessAccumulator.h:332-460 · clean
core\analysis\Statistik.h:file-scope-1 · clean
core\analysis\Statistik.h:28-52 · clean
core\analysis\Statistik.h:58-185 · Befund 3 (Statistik.h:132)
core\analysis\TruePeak.h:file-scope-1 · clean
core\analysis\TruePeak.h:58-68 · clean
core\analysis\TruePeak.h:69-218 · clean
core\analysis\TruePeak.h:219-258 · clean
core\analysis\TruePeak.h:259-307 · clean
core\analysis\Vergleichspegel.h:file-scope-1 · clean
core\analysis\Vergleichspegel.h:11-31 · clean
core\analysis\Vergleichspegel.h:32-181 · Befund 4 (Vergleichspegel.h:70)
core\analysis\Vergleichspegel.h:182-268 · clean

## Nicht gelesen

keine.

## Außerhalb meiner Einheiten

- `eq-copilot/plugin/src/PluginProcessor.cpp:1817` — `vergleichspegel.vorbereiten (getSampleRate())` übergibt die ungeprüfte Hostrate, während `:211` dieselbe Methode mit der in `:182–183` geprüften `sichereSamplerate` speist (zwei Aufrufstellen, zwei Prüfstände).
- `eq-copilot/plugin/src/PluginProcessor.cpp:792-796` — `engine.vorbereiten`/`merkmale.vorbereiten` laufen je Workerrunde in einer Schleife; eine Allokationsausnahme daraus verlässt den `std::thread`-Rumpf, es gibt auf dem Pfad kein `catch`.
- `eq-copilot/plugin/src/PluginProcessor.cpp:548-553` — `speise` wird je Kanal einzeln gerufen, das CAS-Tor fällt zwischen den Kanälen (bereits als NAK-159 geführt).
- `eq-copilot/plugin/core/analysis/FeatureEngine.h:1585-1586` — einzige Produktquelle der FFT-Größe; die Konstanten liegen in `:894-896`.
- `eq-copilot/plugin/core/analysis/FeatureEngine.h:2521,2566,2856-2857` — alle `leistung()`/`realTeil()`/`imagTeil()`-Aufrufe des Produkts, jeweils mit `k <= n/2`.
- `eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:129-130` — Kopfkommentar nennt `kBins = 10001`; ob `(int)((30.0 - -70.0)/0.01)` auf dem Compiler 10000 oder 10001 ergibt, entscheidet die Gleitkommadivision, nicht die Kommentarzeile (kein Sicherheitsbelang, `kFeinObersteKante` wird aus `kBins` gerechnet).
