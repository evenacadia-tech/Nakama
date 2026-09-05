# unit-01 — core/StampedAudioQueue.h, core/analysis/BandGrid.h, core/analysis/BandGridZahlen.h, core/analysis/Blindvergleich.h, core/analysis/FeatureEngine.h (Teil)

Gelesen: 53 von 53 Einheiten vollständig · Zeilen: 2481
HEAD 9f30008 · Bedrohungsmodell LOCAL_UNPRIVILEGED · strikt lesend

Abgedeckte Quellzeilen: `core/StampedAudioQueue.h` 1–826 (ganz),
`core/analysis/BandGrid.h` 1–259 (ganz), `core/analysis/BandGridZahlen.h` 1–203
(ganz), `core/analysis/Blindvergleich.h` 1–182 (ganz),
`core/analysis/FeatureEngine.h` 1–1001 und 4114–4125.

## Befunde

- **[LOW]** `core/analysis/BandGrid.h:135-168` · logik / caller-contract · **Eine
  Bandkodierung `unbekannt` erzeugt einen GÜLTIGEN Bandwert 0 dB statt „keine
  Aussage".** Auslöser: `quantisiere16 (db, BandEncoding::unbekannt)` — über
  `skalierungVon` (`:135-145`) wird die Skalierung `0.0`, `roh = round(db*0.0)`
  ist `0.0`, und `q.gueltig` ist zu diesem Zeitpunkt bereits auf `true` gesetzt
  (`:154`), bevor die Skalierung überhaupt angesehen wird. Wirkung: der Aufrufer
  setzt daraufhin das Bitmap-Bit (`FeatureEngine.h:3782-3783`, `:3803-3804`) und
  meldet für jedes plausible Band exakt 0,0 dB — genau der Fall, den der
  Dateikopf ausschließt („`unbekannt = 0` wird von beiden Lesern abgelehnt und
  darf hier deshalb nie herauskommen", `:77-79`) und ein Bruch der
  NaN-Ehrlichkeit, weil ein nicht kodierbarer Wert als Zahl herauskommt.
  Verschärfend: `Bandsatz::leeren()` (`FeatureEngine.h:534-539`) setzt `encoding`
  bewusst NICHT zurück, der Startwert eines `Bandsatz` ist `unbekannt`
  (`FeatureEngine.h:527`), und die Kodierung wird nur an drei Stellen unmittelbar
  vor dem Füllen gesetzt. Erreichbar: **nein am HEAD** — alle drei Aufrufer
  (`FeatureEngine.h:3515` vor `:3516`, `:3519` vor `:3522`, `:3856` vor `:3887`)
  setzen sie direkt davor; der Riegel liegt damit ausschließlich in der
  Aufrufreihenfolge, nicht im Typ. Beleg: `BandGrid.h:135-145`, `:148-163`;
  `FeatureEngine.h:527`, `:534-539`, `:3779-3784`. Register: neu.

- **[LOW]** `core/analysis/FeatureEngine.h:399-424` · logik / protokoll · **`fingerprintAehnlichkeit`
  vergleicht die Erzeugerversion der zwei Fingerprints nicht.** Auslöser: zwei
  `Fingerprint` mit `gesetzt == true` und verschiedenem `version`. Wirkung: die
  Funktion liefert eine Zahl in [0,1], die wie eine Materialaussage aussieht,
  obwohl die Datei selbst schreibt, eine spätere Änderung an Quantisierung oder
  Bandgruppierung mache „alle alten Fingerprints unvergleichbar — und das soll
  auffallen" (`:354-358`). §15 verlangt „Warnung oder Sperre, wenn das
  musikalische Material nicht vergleichbar ist"; die Version reist genau dafür
  mit und liegt auch auf der Leitung (`src/PluginProcessor.cpp:1743-1744`), wird
  hier aber nie gelesen. Der schon vorhandene Fail-closed-Zweig `! a.gesetzt ||
  ! b.gesetzt` (`:402-403`) zeigt, wo die Prüfung hingehörte. Erreichbar: **nein
  am HEAD** — es gibt keinen Produktaufrufer, nur
  `tests/Sonde013FingerprintGoldenTest.cpp`. Beleg: `:399-424`, `:352-378`.
  Register: neu.

- **[LOW]** `core/analysis/Blindvergleich.h:95` (Feld `:166`) · logik /
  return-values · **Der eingefrorene Vergleichspegel wird kopiert, aber von
  niemandem gelesen — die dokumentierte Zusage ist unenforced.** Auslöser: jeder
  Ablauf `uebernimmVergleichspegel()` → `urteile()` → Ereignis. Wirkung: die
  Klasse begründet die Kopie damit, „ein später gelöschter oder neu gemessener
  Pegel darf ein bereits gefälltes Urteil nicht rückläufig entwerten"
  (`:77-90`), aber `gainDb` hat keinen Getter, keine `friend`-Deklaration und
  keinen Leser im ganzen Repo; das Ereignis nimmt den Pegel stattdessen aus dem
  LEBENDEN `Vergleichspegel` (`src/PluginProcessor.cpp:1937`), der von
  `prepareToPlay` über `vergleichspegel.vorbereiten()` jederzeit geleert wird
  (`src/PluginProcessor.cpp:211` → `Vergleichspegel.h:66-72` →
  `leerenIntern()`: `gehalten = 0.0`, `gehaltenGesetzt = false`). Die Kopie, die
  genau diesen Fall abfangen sollte, liegt ungenutzt daneben. Erreichbar: ja für
  die Wirkung „Zusage ohne Träger"; die daraus folgende Wire-Inkonsistenz liegt
  außerhalb meiner Einheiten (siehe unten). Beleg: `:91-98`, `:160-168`.
  Register: neu.

- **[LOW]** `core/StampedAudioQueue.h:614` · nebenläufigkeit / DSP-Gate 1 · **Der
  Lock-free-Riegel des Audiothreads deckt nur eine der drei benutzten
  Atomic-Breiten.** Auslöser: eine Toolchain/Zielarchitektur, auf der
  `std::atomic<bool>` oder `std::atomic<std::uint32_t>` nicht lock-frei ist.
  Wirkung: im Produzentenpfad `veroeffentliche()` liegen `neustartWunsch`
  (`:596`, gelesen `:386`), `aktuelleStartFolge` (`:597`, gelesen `:388`) und
  `groesstenBlock` (`:608`, geschrieben `:485-486`) — ein versteckter Mutex im
  Audiothread, genau das, was der `static_assert` ausschließen soll. Erreichbar:
  nein auf dem gebauten x64-/MSVC-Ziel. Beleg: `:596-597`, `:608`, `:614`.
  Register: **NAK-154 (1)** — dort wortgleich schon geführt.

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren, Allokationen, I/O, Logging; Überlast verwirft
   Analyseframes, nie Audio** — **hält**. Der gesamte Backing-Store entsteht in
   `StampedAudioQueue::vorbereiten()` (`core/StampedAudioQueue.h:281-300`), und
   beide Produktaufrufer rufen ihn im Konstruktor vor dem Workerstart
   (`src/PluginProcessor.cpp:153`, `sonde/SondeProcessor.cpp:99`); `prepareToPlay`
   fasst nur `neustartAnfordern()` an (`:332-338`). `veroeffentliche()`
   (`:371-491`) und `kopiereTap()` (`:559-582`) enthalten keine Allokation, kein
   Lock, kein I/O und kein Logging; `deskriptoren`/`ring` werden nur über
   `operator[]`/`data()` auf bereits fester Länge benutzt. Jeder Engpass endet in
   `verlust(...)` und verwirft einen GANZEN Analyseblock (`:417-418`, `:421-422`,
   `:426-427`, `:429-431`, `:441-442`) — der Hostpuffer bleibt unberührt.
   Einschränkung: der Lock-free-Nachweis `:614` deckt nur `uint64_t` (Befund 4 /
   NAK-154). Kapazitätsrechnung geprüft: `belegt = ringSchreib - lesePos`
   (`:425`) kann unter dem Produzenteninvariant nie über `budgetFrames` steigen,
   der Unsigned-Vergleich `:426` wrappt daher nicht.
   *Registerhinweis (kein Befund, `docs/**` ausgenommen):* **NAK-71 Punkt (b) und
   (c) beschreiben die Quelle bei HEAD nicht mehr.** Seit `2f3ee24` (27.08.2026)
   sind `neustartWunsch.store(..., release)` (`:337`) und
   `neustartWunsch.exchange(false, acquire)` (`:386`) ein echtes
   Release/Acquire-Paar; es ordnet den vorausgehenden
   `aktuelleStartFolge.fetch_add(relaxed)` (`:334`) vor den späteren
   `load(relaxed)` (`:388`). Der Registertext sagt weiterhin „Beide Zugriffe sind
   relaxed … das C++-Speichermodell ordnet sie nicht" und leitet daraus einen
   Dauerschaden ab. Punkt (a) (Mutation M7 = Zeilentausch fällt durch kein Bein)
   gilt unverändert.
2. **Passthrough bitidentisch, 0 Samples Latenz, kein Tail** — **hält**. Die
   Queue sieht den Hostpuffer ausschließlich lesend: `TapQuelle` trägt zwei
   `const float*` (`:265-269`), `kopiereTap` liest nur `l[...]`/`r[...]`
   (`:575-576`) und schreibt nur in den eigenen Ring. In keiner meiner Einheiten
   gibt es einen Schreibzugriff auf einen Hostpuffer; die Hörmarkierung wird von
   hier nicht berührt.
3. **Gate 5 (kein Telemetrie-/Steuerframe steuert Gain oder erzeugt PRE/POST-Delta)**
   — **nicht berührt**. Keine meiner Einheiten liest ein Pipe-, FlatBuffer- oder
   Modellframe, und keine schreibt einen Gain. `Blindvergleich` hält Userdaten
   und den (ungenutzten) Match-Gain; er wird nirgends auf Audio angewandt.
4. **NaN-Ehrlichkeit** — **hält** in den geprüften Zeilen.
   `quantisiere16` gibt für nicht-endliches `db` Wert 0 mit `gueltig=false` und
   ohne Saturationsbit (`BandGrid.h:151-152`), `quantisiereF32` ebenso
   (`:179-181`); Sättigung wird VOR der Verengung entschieden (`:183-186`).
   `energieAlsDb` fängt NaN und Nullenergie über `! (x > 0.0)` (`:254-256`),
   `plausibel` (`:236-239`) verwirft NaN durch beide Vergleiche.
   `FeatureEngine::vorbereiten` lehnt nicht-endliche, nicht-positive und über
   768 kHz liegende Raten ab und lässt dabei einen zuvor gültigen Zustand
   ausdrücklich verfallen (`FeatureEngine.h:971-981`).
   `fingerprintAehnlichkeit` prüft Nenner und `isfinite` (`:413-417`).
   `schliesstAn` prüft den int64-Überlauf der Projektzeit explizit
   (`StampedAudioQueue.h:780-785`). Die Nyquist-Kappe selbst
   (`kObergrenzeHz`/`kNyquistAnteil`, `FeatureEngine.h:905-906`) wird außerhalb
   meiner Einheiten gerechnet (`:1757`).
5. **State verlustfrei (unbekannte Major-Version read-only mit Originalbytes)** —
   **nicht berührt**. Keine meiner Einheiten liest oder schreibt einen
   persistierten Zustand.

Zusätzlich geprüft und ohne Befund:
`BandGridZahlen.h` ist gegen die deklarierten Feldgrößen und gegen sich selbst
konsistent — 222/221 Evidenzwerte, 65/64 Livewerte, `kLiveGruppeVon`/`BisExkl`
streng monoton, lückenlos aneinandergrenzend und exakt [0, 221) abdeckend
(maschinell nachgezählt). Die Ringalgebra der Queue (`:425-431`, `:483`,
`:516-524`, `:537-538`, `:565-581`) ist bis an die Grenze durchgerechnet: Ziel-
und Quellindizes bleiben in jedem Zweig innerhalb `taps*budgetFrames*2`
beziehungsweise `2*maxBlockFrames`, weil `GenStrom`
(`StreamLayout<1,131072,2048,16384>`, `:240`) `maxBlockFrames <= budgetFrames`
per `static_assert` erzwingt (`:222-223`) und der Produzent `frames >
maxBlockFrames` verwirft (`:421-422`). Reihenfolge im Worker geprüft: `lies()`
kopiert vor `freigeben()` (`src/PluginProcessor.cpp:902-903`,
`sonde/SondeProcessor.cpp:335`), der Deskriptorzeiger aus `spitze()` wird nach
`freigeben()` nicht mehr benutzt.

## Ledger

- `core\StampedAudioQueue.h:file-scope-1` · clean
- `core\StampedAudioQueue.h:126-149` · clean
- `core\StampedAudioQueue.h:file-scope-2` · clean
- `core\StampedAudioQueue.h:199-205` · clean
- `core\StampedAudioQueue.h:281-300` · clean
- `core\StampedAudioQueue.h:file-scope-3` · clean
- `core\StampedAudioQueue.h:332-338` · clean
- `core\StampedAudioQueue.h:file-scope-4` · clean
- `core\StampedAudioQueue.h:file-scope-5` · clean
- `core\StampedAudioQueue.h:file-scope-6` · clean
- `core\StampedAudioQueue.h:497-503` · clean
- `core\StampedAudioQueue.h:file-scope-7` · Befund 4
- `core\StampedAudioQueue.h:508-526` · clean
- `core\StampedAudioQueue.h:529-539` · clean
- `core\StampedAudioQueue.h:555-555` · clean
- `core\StampedAudioQueue.h:556-556` · clean
- `core\StampedAudioQueue.h:678-684` · clean
- `core\StampedAudioQueue.h:688-724` · clean
- `core\StampedAudioQueue.h:727-798` · clean
- `core\StampedAudioQueue.h:file-scope-8` · clean
- `core\StampedAudioQueue.h:817-817` · clean
- `core\StampedAudioQueue.h:818-818` · clean
- `core\StampedAudioQueue.h:819-824` · clean
- `core\analysis\BandGrid.h:file-scope-1` · clean
- `core\analysis\BandGrid.h:34-74` · clean
- `core\analysis\BandGrid.h:124-125` · clean
- `core\analysis\BandGrid.h:130-259` · Befund 1
- `core\analysis\BandGridZahlen.h:file-scope-1` · clean
- `core\analysis\BandGridZahlen.h:19-163` · clean
- `core\analysis\BandGridZahlen.h:164-203` · clean
- `core\analysis\Blindvergleich.h:file-scope-1` · clean
- `core\analysis\Blindvergleich.h:91-98` · Befund 3
- `core\analysis\Blindvergleich.h:103-122` · clean
- `core\analysis\Blindvergleich.h:133-139` · clean
- `core\analysis\Blindvergleich.h:150-158` · clean
- `core\analysis\Blindvergleich.h:166-182` · Befund 3
- `core\analysis\FeatureEngine.h:file-scope-1` · clean
- `core\analysis\FeatureEngine.h:file-scope-2` · clean
- `core\analysis\FeatureEngine.h:178-273` · clean
- `core\analysis\FeatureEngine.h:file-scope-3` · clean
- `core\analysis\FeatureEngine.h:275-279` · clean
- `core\analysis\FeatureEngine.h:307-424` · Befund 2
- `core\analysis\FeatureEngine.h:501-503` · clean
- `core\analysis\FeatureEngine.h:515-516` · clean
- `core\analysis\FeatureEngine.h:530-543` · clean
- `core\analysis\FeatureEngine.h:557-557` · clean
- `core\analysis\FeatureEngine.h:558-558` · clean
- `core\analysis\FeatureEngine.h:file-scope-4` · clean
- `core\analysis\FeatureEngine.h:570-570` · clean
- `core\analysis\FeatureEngine.h:683-683` · clean
- `core\analysis\FeatureEngine.h:701-701` · clean
- `core\analysis\FeatureEngine.h:702-851` · clean
- `core\analysis\FeatureEngine.h:852-1001` · clean

## Nicht gelesen

keine.

## Außerhalb meiner Einheiten

- `src/PluginProcessor.cpp:1937` — `versuchReferenzJson` schreibt
  `match_gain_db` aus dem LEBENDEN `vergleichspegel.gainDb()` und prüft
  `gainGesetzt()` nicht; ruft der Host zwischen `experiment_begin` (`:2039`) und
  `experiment_candidate` (`:2090`) ein `prepareToPlay`, leert
  `vergleichspegel.vorbereiten()` (`:211`, `Vergleichspegel.h:66-72`) den
  eingefrorenen Wert auf `0.0`, und die zwei Ereignisse desselben
  `experiment_id` tragen verschiedene Match-Gains.
- `src/PluginProcessor.cpp:2087` — der Rückgabewert von
  `blindvergleich.bindeReihenfolge(...)` wird verworfen; bei einem zweiten
  `erfasseKandidat(...)` mit der anderen Reihenfolge sendet
  `experiment_candidate` (`:2100-2102`) die NEUE Reihenfolge, während
  `Blindvergleich` die erste behält und `experiment_manual_result` (`:2140-2141`)
  sie später so aufdeckt.
- `core/analysis/FeatureEngine.h:212-251` — `nak29Verstoss` prüft
  `sample_rate` nicht auf Endlichkeit oder Positivität, obwohl der Riegel auch
  vor der Serialisierung steht (`core/ipc/TelemetryClient.cpp:51`); heute kommt
  der Wert immer aus dem in `vorbereiten()` geprüften `sr` (`:3658`).
- `core/analysis/BandGrid.h` → Aufrufseite: `Gitter::evidenzKante/liveVon/...`
  (`:49-73`) und `bitmapSetze/bitmapLies` (`:210-220`) nehmen den Index
  ungeprüft; alle heutigen Aufrufer (`FeatureEngine.h:1704`, `:1724-1725`,
  `:1757`, `:2677`, `:3140`, `:3751`, `vertrag/NakamaEvidenz.cpp:258`) bleiben
  nachweislich in den Grenzen.
