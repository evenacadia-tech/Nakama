URTEIL: NEEDS_WORK

# NAK-311 — Matrixprüfung 5 (Etappe 5, Wiederprüfung der Matrixnacharbeit 3, Planschritt S25j)

Frischer, lesender Prüfer, Auftrag
`docs/beweise/roh/NAK-311-matrixpruefung-5-auftrag.txt` (Vorlage B).
Prüfbereich: `git diff 137672b4...d4b40aa6 -- docs/beweise/NAK-311.md`
(53 Hunks mit `-U0` nachgemessen, 991 Einfügungen, 174 Löschungen, nur diese
Datei). **HEAD zu Beginn `e1d98973bfbf4e99acdb8775550ce2fdb9b316f5`.**
Nur lesende Kommandos (`git diff`, `git show`, `git grep`, `git ls-files`,
`git rev-parse`, `rg`, `sed`, `awk`) und ein Python-Interpreter zum Nachrechnen
der Formeln und Rekursionen. Kein Bau, kein Test, kein FL Studio, kein
Repo-Skript. Geschrieben ist genau diese Datei.

**Offen (drei Befunde):** D-1 (a) und (d), D-5 (eine neue Fundstelle),
R-311-18. Geschlossen: D-2, D-3, D-4, D-1 (b) und (c), R-311-17, R-311-19,
R-311-20. Frage 2 fällt mit Nein aus: kein Bruch gegen Code, Gate-Text,
Regeln, Entscheide, unberührte Matrixzeilen, SONDE-015-Zusagen oder
CLAUDE.md-Invarianten; §39.6 stimmt am Text.

---

## Frage 1 — je Befund und je Regel

### D-1 — M-103 (§39.1) und §40.3: größter Bestandsfall und Rotbeweis des Gainkriteriums

**NEIN.** (b) und (c) sind geschlossen, (a) und (d) nicht.

**(b) geschlossen.** `kSprungGainDb` = **20,0** steht an jeder Stelle: §38.1
(`:5931`), §39.1 Vorspann (`:6287`), M-103 (`:6300`), §40.3 (`:6822-6842`),
§41.2 F-20 (`:7305`). Ein `awk`-Durchlauf über §38 bis §41 findet keine Stelle
mehr, die 18,0 als **Kriterium** nennt; die verbleibenden „18,0" sind der Wert
des Bestandsfalls, die Historie („mit der früheren Zahl") und der
Candidate-Fall `TransactionTestMain.cpp:2222`. Abstand und Faktor
nachgerechnet: 20,0 − 18,0 = 2,0 dB, 20/18 = 1,1111. Die Vertragsgültigkeit der
drei Kanten stimmt: −12 + 20 + 1 ULP = +8 dB + ε ≤ +12 dB
(`nakama-parameter-v2.json:180-190` gelesen, ±12 dB), größte vertragsgültige
Differenz 24 dB. Messreihe 12 / 18 / 20 / 24 / 36 / 48 dB steht in §40.3
(`:6842`) und §43.2.

**(c) geschlossen.** Je Kriterium ein Prüfling plus eigene Mutation, an der
Quelle nachvollzogen:

| Kriterium | Mutation an der eigenen Zusage | Zeile | Wertwache | an der Quelle geprüft |
|---|---|---|---|---|
| Frequenz | `>=` statt `>` → Kante „genau auf der Grenze" verlässt den Rampenweg | M-101 | M-100 (`kSprungFrequenzVerhaeltnis` auf 1,2) | ja |
| Güte | Gütekriterium aus `bleibt` entfernt → eigene `+ 1 ULP`-Kante behält die Kennung | M-102 | M-142 (b), `kSprungGueteVerhaeltnis` auf 2,0 | ja |
| Gain | Gainkriterium aus derselben Bedingung entfernt → eigene `+ 1 ULP`-Kante behält die Kennung | M-103 | M-142 (a), `kSprungGainDb` auf 5,0 | ja |

Nachvollzogen: die `bleibt`-Bedingung steht an
`eq-copilot/plugin/dsp/DspKern.cpp:286-288` und vergleicht heute genau
`s.typ`, `s.modus`, `s.dynamisch`, `s.nutztSvf`, `s.quelle` — kein Wertfeld.
Fällt ein Wertkriterium aus `bleibt`, behält der Slot bei
`b.lebenszyklus = bleibt ? s.kennung : naechsteKennung()` (`:288`) seine
Kennung, `kennungenGleich` bleibt wahr (`:501-505`), `nurRampen` wird wahr
(`:507`), `z.uebergang = Uebergang::rampe` (`:546`) — der Crossfade läuft
nicht, der eigene Teilfall fällt. Die Mutation ist wertunabhängig, weil M-102
und M-103 ihre Kante symbolisch aus der Konstanten bilden.

Die Wertwache M-142 habe ich am Prüfstand nachgemessen:
`eq-copilot/plugin/tests/DspGoldenTestMain.cpp:1873-1878` trägt die drei
Sprünge (`:1875` `gain_db` 3,0 → 9,0 = **6,0 dB**; `:1876` `freq_hz`
1000 → 1400 = **1,4**; `:1877` `q` 1,0 → 3,0 = **3,0**), je genau ein
geändertes Feld (`:1886`, `:1888`), Wechsel bei Sample 2048 an einem
Blockrand (`:1900`), Vergleich gegen die im Test ausgeschriebene lineare
Koeffizientenrampe über `kRampeSamples` (`:1915-1920`), Schranke
`maxRes < 1e-5` mit Prüfname an `:1931-1933`. Steht die Konstante unter dem
Bestandswert, bekommt Slot 0 eine neue Kennung, `nurRampen` wird falsch,
`pfadGleich` bleibt wahr → der Crossfade-Zweig (`DspKern.cpp:518-541`)
überträgt den `BandZustand` **nicht**, weil `a.lebenszyklus != n.lebenszyklus`
(`:536`); die neue Bank startet kalt, der Tap `Tap::postCommitted` trägt den
geblendeten Ausgang (`:1122`) und weicht von der stetigen Rampe um
Größenordnungen über 1e−5 ab. Der Rotbeweis trägt.

**(a) NICHT geschlossen — DEFEKT.** Die Aufzählung „Die Bestandsfälle auf dem
Rampenweg, **vollständig**" (§40.3, `:6844-6868`) ist nicht vollständig. Ich
habe die vier Testdateien selbst durchgesucht und **zwei** rampenkompatible
Publikationspaare mit Wertwechsel gefunden, die in keiner der zwei Tabellen
stehen:

1. **`eq-copilot/plugin/tests/TransactionTestMain.cpp:2069` → `:2081`** —
   `setzeBand (z, 0, 1000.0, **9.0**)` (Bell 1 kHz, Vertragsdefault-Güte,
   Kanalmodus stereo, `mitEq (true)`), publiziert an `:2070`, 200 Blöcke
   Audio an `:2074`; dann `hostSchreibt (*mit, iBand (0, param::kGainDb),
   **-9.0f**)` an `:2081`, `kontrollTaktFuerTest()` an `:2082`, 50 Blöcke
   Audio an `:2083`. **Differenz 18,0 dB.** Topologie unverändert — die zwei
   topologischen Hostschreiber an `:2071-2072` wirken nachweislich nicht,
   der Test prüft es selbst an `:2078` (`kType`-enumIndex bleibt 0). Beide
   Programme tragen `eqEngagiert`, keinen `hardBypass`, dieselbe Rate
   (48 kHz, `prozessor()` `:1722`) und dieselbe Topologie je Slot, also ist
   `rampenKompatibel` wahr (`DspProgramm.cpp:207-227`), die `bleibt`-Bedingung
   hält, `nurRampen` ist wahr. Dieser Fall **vergleicht Audio**
   (`! bitgleich (c, d)` an `:2085-2086`, Prüfname „Gegenprobe:
   kontinuierliche Automation (gain_db) wirkt blockweise" im Test
   `capability_entscheidet_den_automationspfad (M-119)`) und liegt mit 18,0 dB
   **gleichauf** mit dem in M-103 und §40.3 als „der größte" benannten Fall
   `uebernahme_nur_am_aeusseren_blockrand`.
2. **`eq-copilot/plugin/tests/TransactionTestMain.cpp:1313` → `:1316`/`:1317`**
   — `grundzustand (st, s, 100)` setzt Slot 0 auf `freq_hz` 1000 Hz
   (`:276-281`), dann `raus` mit **500,0 Hz** (`:1313`, committet an `:1315`)
   und `rein` mit **1000,0 Hz** (`:1316`, committet an `:1317`); beide Commits
   sind im Test zugesichert (`:1318-1320`). **Verhältnis genau 2,0 in beiden
   Richtungen**, also **exakt auf** `kSprungFrequenzVerhaeltnis`. `Stand`
   (`:130-144`) hält einen echten `DspKern`, mit `bereiteVor (48000, 512)`
   vorbereitet; `fahre` (`:156-161`) ruft kein `verarbeite`, die Publikation
   läuft aber über `DspKernAusfuehrung` und damit durch `vergebeKennungen`.
   Nach der Definition, mit der §40.3 selbst die drei Fälle „ÜBER einem
   Kriterium" in derselben Datei zählt (Publikationspaar, kein Audio), gehört
   dieser Fall in eine der zwei Tabellen.

**Welche Sätze brechen:**

- §40.3 `:6862-6863`: „Kein weiterer Fall der vier Testdateien publiziert zwei
  rampenkompatible Programme mit geändertem `freq_hz`, `q` oder `gain_db`."
- §40.3 `:6867`: „Alle sieben Fälle liegen **unter** den drei Kriterien" — mit
  Fall 2 liegt ein Fall exakt **auf** dem Frequenzkriterium, mit Abstand 0.
- §40.3 `:6888`: „Weitere Fälle über einem Kriterium gibt es in den vier
  Testdateien nicht" bleibt wahr; der Satz darüber („Alle drei liegen in
  `TransactionTestMain.cpp` … und **keine von ihnen vergleicht Audio**",
  `:6872-6873`) trägt jedoch die Annahme, in dieser Datei laufe kein Audio —
  Fall 1 widerlegt sie (Abschnitte M bis T fahren den echten `Prozessor` mit
  `processBlock`, `fahreAudio` `:1751`).
- §40.3 „Risiken und Grenze", Risiko 1 (`:6977-6985`): „die zwei Aufzählungen
  oben nennen **jeden** Fall der vier Testdateien mit Fundstelle und Wert —
  sieben auf dem Rampenweg unter den Kriterien, drei ohne Audio darüber".
- M-103 (§39.1, `:6300`): „Die vollständige Aufzählung der Bestandsfälle auf
  dem Rampenweg steht in §40.3."
- §38.1 (`:5933-5934`): „die vollständige Aufzählung steht in §40.3."
- §40.3 „Golden- und Fixture-Regel" (`:6969-6971`): „die **sieben** oben
  aufgezählten Rampenfälle … der engste ist
  `uebernahme_nur_am_aeusseren_blockrand` mit 18,0 dB gegen 20,0".

**Ursache.** Die Zählvorschrift in `:6845-6846` („gezählt über jede Zuweisung
an `kGainDb`, `kFreqHz` und `kQ` und jeden `belege`-Aufruf in den vier
Testdateien") erfasst weder die dateieigenen Helfer
`TransactionTestMain.cpp:253-259` (`setzeBand`) und `:276-281`
(`grundzustand`) noch den Host-Automationsweg `hostSchreibt` (`:1795-1799`).
`belege` existiert ausschließlich in `DspGoldenTestMain.cpp` (144 Aufrufe; 0 in
den drei anderen Dateien, nachgezählt). Damit ist die Methode selbst die Lücke,
nicht ein einzelner vergessener Fall. Derselbe `git grep` als Wache
(`:6982-6983`) hätte Fall 1 halb gefunden (`:2081` trägt `kGainDb`), die
Gegenseite `:2069` nicht.

**Was an (a) stimmt.** Die sieben aufgezählten Fälle sind an der Quelle
bestätigt: `DspGoldenTestMain.cpp:1875` / `:1876` / `:1877` (Prüfname
`:1931-1933`), `:4013`/`:4014` (6,0 → 2,0 dB, Prüfname `:4075-4080`),
`:4199`/`:4205` (Low-Shelf 8000 Hz Q 0,707, +9,0 → −9,0 dB, Prüfname `:4225`),
`:5670`/`:5686` (Abschnitt O, +5,0 → −2,0 dB), `:6825`/`:6831` (M-94-
Referenzkern, +12 → +6 dB, publiziert an `:6845`/`:6851`). Der Ausschluss des
M-37-Spiegels stimmt: `:6142-6165` läuft über `ausUndEin` (`:5940-5964`), und
`rampenKompatibel` ist dort schon wegen `eqEngagiert` falsch
(`DspProgramm.cpp:212`). Die Stressläufe `:5397-5401` (Slot rotiert je Runde,
`runde % param::kSlots`, `v = s` jeweils frisch) und `:5479-5481`
(`channel_mode` jedes Bandes wechselt je Publikation) sind richtig als schon
heute Crossfades eingeordnet. Die Candidate-Paare (`:4915`/`:4918`,
`:4864`/`:4871`) sind richtig nicht aufgenommen — sie liegen auf getrennten
Pfaden.

**(d) NICHT geschlossen — dieselbe Ursache.** Die Tabelle „Bestandsfälle ÜBER
einem Kriterium" (`:6879-6883`) ist an ihren drei Zeilen richtig:
`TransactionTestMain.cpp:577` (`(i % 24) − 12 + 0,25`, am Umlauf 23 → 24
**23,0 dB**), `:1396` (`((i % 24) − 12) + 0,5`, ebenso 23,0 dB), `:1009`/`:1013`
(1050 → 500 Hz = **2,1**); `fahre` ruft dort kein `verarbeite`, die Urteile
hängen an Register, Hash, Revision und Ring. Auch die zwei Abgrenzungen
stimmen: `SondeNullTestMain.cpp:202-203` gegen `:662-663` erreicht
12,0 → −3,0 dB = 15,0 dB (Gain) und Güte-Default 1,0 → 0,7071 = Faktor 1,414,
bleibt also unter beiden Kriterien; `TransactionTestMain.cpp:2222` erreicht
+6 → −12 dB = 18,0 dB, liegt aber auf dem Candidate-Pfad (`setzePreview`
`:2224`). Der **Vollständigkeitssatz** `:6888` steht jedoch auf derselben
unvollständigen Suche wie (a), und Fall 2 aus (a) liegt mit Abstand 0 auf einem
Kriterium — genau der Zustand, den §43.2 für den Gain zum Anlass genommen hat,
die Zahl von 18,0 auf 20,0 zu heben („eine Grenze ohne Rand zu einem Golden
trägt die Zusage … nur über die Richtung eines Vergleichs"). Für das
Frequenzkriterium steht dieser Rand-0-Fall unbenannt da.

### D-2 — §40.4 und §39.3: Pegelzustand beim Abschalten des Detektors

**JA, geschlossen.** An der Quelle nachgemessen:

- Die Stelle stimmt. `eq-copilot/plugin/dsp/DspKern.cpp:741-745` lautet
  wörtlich `if (b.nutztSvf && ! detektorZuletzt) { z.huelle.nullen(); for
  (auto& d : z.detektor) d.nullen(); }`, gliedweise und nicht über
  `BandZustand::nullen()`; der Kommentar `:737-740` trägt E-29 / W-3. §40.4
  nennt sie an drei Stellen (`:7006-7012` Ticketpfade, `:7156-7158` Änderungen
  je Datei, `:7200-7205` Risiko 2b).
- §38.3 führt E-29 und W-3 wörtlich als weiter geltende Zusage (`:6124-6141`).
  Beide Zitate stimmen bytegleich mit `docs/beweise/SONDE-015.md:3423`
  („Ein Detektor, der am Ende eines Fensters nicht läuft, wird genullt.", samt
  Begründungssatz) und `:3741` (W-3-Zeile).
- M-141 (`:6362`) misst Range −12 → 0, Stille, Range 0 → −12; der Prüfling
  deckt sich Feld für Feld mit dem Bestandsfall
  `DspGoldenTestMain.cpp:2537-2545` (48 kHz, `neuerKern (fs, 64)`, Bell 1 kHz
  Q 0,707 Grundgain 0, `machDynamisch (sa, 0, rVon, -60.0, 0.1, 0.0, 100.0)`,
  Quadraturton 0,9, 4096 Samples); Pegelzustand bitgenau +0,0, Auslenkung
  gegen dieselbe ausgeschriebene Idealrampe, Residuum unter 1e−6 dB — das ist
  die Schranke des Bestandsfalls (`:2572`, `maxRes < 1e-6`).
- Der Rotbeweis fällt an M-141 und wird nirgends abgefangen. Nachvollzogen:
  nach der ersten Rampe ist `detektorLaeuft` falsch (Range 0,
  `DspProgramm.cpp:318`), damit ist `detektorAktiv` falsch (`DspKern.cpp:660`),
  der Detektorzweig `:664-687` läuft nicht, und `:741-745` nullt je Block. Ein
  nicht mitgenullter Pegelzustand bleibt über die Stille eingefroren (er wird
  gar nicht getickt) und ist beim ersten Sample der zweiten Rampe größer als
  die frisch genullte Hüllkurve → Attack-Zweig (`DspFilter.h:314-318`) →
  Sprung im ersten Steuerschritt. Der Bestandsfall fängt es nicht auf: er
  liest ausschließlich `huelle.leistung` (`DspGoldenTestMain.cpp:2564`,
  `:2569`), die `:743` weiterhin nullt, und fährt je Lauf nur **eine**
  Richtung (`:2534-2536`) — für `richtung == 1` startet er auf einem frischen
  Kern mit Range 0, in dem der Detektor nie lief.
- Die Beziehung aktivieren↔abklingen ist in §39.6 geführt (`:6453-6454`),
  M-141 steht im Änderungssatz D („Tests und Beine", `:7165`).

### D-3 — M-127 (§39.3) und §40.4: Absolutzahl der Sprungantwort

**JA, geschlossen.** Selbst nachgerechnet, analytisch **und** mit den
Rekursionen des Codes für alle vier Raten.

*Analytisch.* Für zwei Ein-Pol-Stufen in Reihe mit τ_m = 10 ms und
τ_a = 20 ms ist τ_a = 2·τ_m, und die Sprungantwort der Kaskade
1 − (τ_a·e^(−t/τ_a) − τ_m·e^(−t/τ_m))/(τ_a − τ_m) fällt exakt mit
1 − 2e^(−t/τ_a) + e^(−t/τ_m) = (1 − e^(−t/τ_a))² zusammen. Aus
(1 − e^(−t/τ_a))² = 1 − 1/e folgt t = −τ_a·ln(1 − √(1 − 1/e)) =
**31,70077 ms**. Die in M-127 (`:6357`) und §41.3 Nr. 3 (`:7415-7416`)
genannte Gleichung ist damit richtig, und ihre Gleichsetzung mit der
Produktform ist kein Zufall, sondern die Folge von τ_a = 2·τ_m.

*Diskret, Rekursionen des Codes.* `huellkurvePol` (`DspFilter.h:273-278`,
`exp(−1/(fs·τ))`) und `HuellkurveZustand::tick` (`:312-328`), Pegelstufe
`p = a_m·p + (1 − a_m)·x`, Steuerrate `kDynamikSchritt` = 8 wie in
`DspKern.cpp:690-714`: Übergang bei Sample 1400 / 1528 / 3048 / 6088, also
**31,746 / 31,833 / 31,750 / 31,708 ms** bei 44,1 / 48 / 96 / 192 kHz. Das sind
genau die vier Zahlen der Nacharbeit (31,75 / 31,83 / 31,75 / 31,71), und
§44.3 etikettiert sie richtig als „63-%-Punkt der **Polreihe**".

*Volle Kette des Prüfstands, zur Gegenprobe.* Habe ich
`DspGoldenTestMain.cpp:2375-2435` ganz nachgefahren (Bandpass
`entwurfBandpass` `DspFilter.h:224-238` mit `Biquad::tick` `:90-96`,
Quadraturton L sin / R cos `:460-461`, `leistungEin = (d0² + d1²)·0,5`,
Pegelpol, Hüllkurve, Kennlinie `DspFilter.h:355-361`, Schwelle
`attackSchwelle = −(12 + 10·log10(1 − 1/e)) = −10,008 dB`, Steuerrate 8,
Blockgröße 1, float-Rundung des Eingangs): **heute** 20,32 / 20,33 / 20,33 /
20,38 ms (Sollwert 20, Toleranz 1 ms — der Test ist heute grün, wie er sein
muss); **mit 10-ms-Pegelpol** 32,11 / 32,17 / 32,08 / 32,04 ms. Der **Zuwachs**
ist damit 11,79 / 11,83 / 11,75 / 11,67 ms — die Angabe „der Zuwachs ist 11,7
bis 11,8 ms" (§40.4 `:7080`, §41.3 `:7411`) stimmt. Hold wächst um 0,000 /
0,000 / 0,000 / 0,042 ms, Release um 0,544 / 0,667 / 0,667 / 0,583 ms: die
Angabe „(gemessen +0,00 ms und höchstens +0,67 ms)" in M-127 stimmt. Der neue
Sollwert 31,7 ms trägt: die volle Kette liegt 0,34 bis 0,47 ms darüber und
bleibt in der unveränderten Toleranz von 1 ms; die Spanne über vier Raten ist
0,13 ms („Spanne 0,12 ms", §40.4 `:7081`). Mit „rund 27 ms" läge die Messung
4,7 bis 5,2 ms daneben.

*Folgezahlen.* Nachgezogen sind §40.4 Zeile 1 und 2 der Verschiebungstabelle
(`:7080`, `:7081`), M-127 selbst (t_E und die vier Absolutzahlen) und §41.3
Nr. 3 (`:7407-7419`, Nachtragsvorschlag zu E-25 mit derselben Gleichung, der
unveränderten Toleranz 1 ms und den Hold-/Release-Zahlen). E-25 zitiert
`SONDE-015.md:3419` wörtlich richtig. Die Kette in §40.4 Zeile 3 (`:7082`,
`huellkurvenwert_<feld>_rampt_ohne_zustandsreset`) und Zeile 7 (`:7086`,
t_E von `311/M-77`/`311/M-78`) tragen keine Absolutzahl aus den 27 ms.

### D-4 — §40.2 Nr. 3: Abstandsrechnung zur Deckelhöhe

**JA, geschlossen.** Jede Zahl des berichtigten Satzes (`:6651-6666`)
nachgerechnet:

| Angabe | Nachgerechnet |
|---|---|
| 42,99 − 24 = Abstand in dB | 18,99 dB |
| 10^(18,99/20) linear | 8,9022 („8,9") |
| 10^(18,99/10) in der Leistung | 79,2501 („79,3") |
| 42,99 / 24 als dB-Verhältnis | 1,7913 („1,79") |
| 10^(24/20), der lineare Faktor des Deckels | 15,8489 („15,849") |
| 24 / 6,9748 | 3,4410 („3,441") |
| 24 − 7,0 | 17,0 dB |

Auch die Herleitung aus Nr. 1, auf die der Satz sich stützt, hält:
log2(80/20)/log2(1000) = 0,20069 (20,07 %) → −10·log10(1 − 0,20069) =
**0,9728 dB**; 500 Hz: 0,46598 (46,60 %) → **2,7243 dB**; High-Cut 2 kHz:
0,33333 → **1,7609 dB**; zwei stehende Oktaven: 2/9,96578 = 0,200686 →
**6,9748 dB**. Die Zahl 24,0 bleibt, das Maß von R-311-14 („über jeder
Anhebung normaler Arbeit und unter dem, was Wiedergabegeräte gefährdet") hält
in beiden Richtungen mit Rand, und der Absatz sagt, was bei einer reißenden
Messung geschieht (`:6667-6672`). Der Vertragsrand +24 dB für
`v1.global.output_trim_db` steht an
`eq-copilot/schemas/state/nakama-parameter-v2.json:52-58`, wie zitiert.

### D-5 — M-97 und M-99 (§39.1): Etikett „heute nicht messbar"

**NEIN — die Umetikettierung trägt, eine neue Fundstelle in M-99 nicht.**

*Was geschlossen ist.* Beide Zeilen tragen jetzt „**heute rot**" mit
Gegenprobe (`DspProgramm.cpp:207-227` und `DspKern.cpp:286-288` — an der
Quelle bestätigt: `rampenKompatibel` nennt kein Wertfeld, `bleibt` vergleicht
nur Topologiefelder), mit dem §7.1-Lauf vor der Reparatur samt Zielpfad
(`docs/beweise/roh/NAK-311-rot-M-97.txt` beziehungsweise `…-M-99.txt`) und mit
der Befundregel („Fällt er dort NICHT, ist das ein Befund an den Dirigenten
und die Zeile wird Regressionswache, nie still umetikettiert"). Das Etikett
stimmt gegen §6 (`:513-516`): am Basisstand ist die Zusage in ihren ersten
zwei Teilen nachweislich verletzt — es läuft kein Crossfade, und Slot 0 meldet
nicht 0,0 —, nur die Zahl fehlt. §40.3 „Rotbeweise nach §7.1" (`:6939-6951`)
nennt jetzt M-96, M-97, M-98 und M-99 als „heute rot" und führt die
Befundregel mit; §39.6 (`:6422-6431`) stimmt damit überein.

Die Aussage „siebzehn halten" (§44.1) habe ich an sieben Zeilen gegen §6 am
Code des HEAD geprüft. Ein `git grep` über `eq-copilot`, `broker` und `tools`
findet **null** Treffer für `kSprungFrequenzVerhaeltnis`,
`kSprungGueteVerhaeltnis`, `kSprungGainDb`, `kAutoGainDeckelDb`,
`kPegelFensterMs`, `kMinSamplerateHz` und `abgelehnteSamplerateHz`. Damit
hält das Etikett für M-101, M-102, M-103 (die drei Konstanten und Vergleiche
entstehen im Bau), M-104 (heute behalten auf dem Rampenweg **alle** Slots
ihren Zustand, `DspKern.cpp:515-516`), M-109 (die drei Vergleiche und ihre
Endlichkeitswache entstehen erst), M-113 und M-115 bis M-118 (Deckel),
M-125/M-127/M-130/M-131 (Pegelbegriff), M-134 (Prädikat) und M-139 (der frühe
Rückweg wird heute bei 32 kHz nie genommen — `SondeProcessor.cpp:258` prüft
nur `sichereRate > 0.0 && maxBlock > 0`).

*Was bricht — DEFEKT.* M-99 (`:6296`) behauptet neu: „`machDynamisch` mit
Range 0 steht im Bein (`DspGoldenTestMain.cpp:5677`, `:6821`)". **Beide
Fundstellen sind falsch:**

- `DspGoldenTestMain.cpp:5677` ist `machDynamisch (basis, 5, **-6.0**, -20.0,
  5.0, 10.0, 80.0)` — Range −6,0 dB auf Slot 5.
- `DspGoldenTestMain.cpp:6821` ist `machDynamisch (vorher, 0, **-12.0**,
  -40.0, 0.1, 0.0, 100.0)` — Range −12,0 dB.

Der einzige Aufruf mit Range 0 in den vier Testdateien steht an
`DspGoldenTestMain.cpp:2057` (`machDynamisch (s, 0, 0.0, -60.0);   // Range
0`); zusätzlich trifft `:2540` Range 0 über die Laufvariable `rVon` in der
Richtung `richtung == 1`. Die Signatur ist `machDynamisch (s, slot, rangeDb,
thresholdDb, …)` (`:183-187`), das dritte Argument ist also die Range. Der
Satz ist neu im Fixdiff (die alte M-99-Zeile trug ihn nicht) und stützt genau
die Aussage, um die es in D-5 geht („der Fall ist am Basisstand mit
vorhandenen Mitteln messbar"). Die Umetikettierung selbst fällt dadurch nicht:
`nutztSvf`, `detektorLaeuft` bei Range 0 (`DspProgramm.cpp:318`), der SVF-Weg
(`DspKern.cpp:690-710`) und das am Ausgang ablesbare Maß tragen sie ohne diese
Fundstelle. Der Satz ist aber falsch und wiederholt die Ursache, die §43.1 für
D-1 und D-5 gemeinsam benennt („am Code behauptet statt nachgemessen").

### R-311-17 — M-124 misst vier Bandmitten mit bezifferter Schranke

**JA.** M-124 (`:6354`) trägt die vier Bandmitten 1 kHz / 341 Hz / 100 Hz /
20 Hz, die Schranke max(0,1 dB; 10·log10(1 + 1/(4π·f0·τ_m))) mit
τ_m = `kPegelFensterMs` und die vier Zahlen. Nachgerechnet mit τ_m = 10 ms:
r = 0,0079577 / 0,0233368 / 0,0795775 / 0,3978874 und 10·log10(1 + r) =
0,03444 / 0,10019 / 0,33254 / 1,45472 dB, also nach der max-Klammer
**0,1 / 0,1002 / 0,3325 / 1,4547 dB** — genau die Zahlen der Regel und der
Zeile. §40.4 Nr. 3 (`:7036-7038`) und §41.2 F-18 (`:7285-7294`) nennen
dieselben Werte. Der Rotbeweis fällt je Bandmitte an der Zeile: „das Mittel
asymmetrisch gemacht (steigend Pol 0, fallend `kPegelFensterMs`) → die
Gleichrichtung kehrt zurück, und die Spanne wächst an JEDER der vier
Bandmitten über ihre Schranke → vier rote Teilfälle". Der Rest bei tiefen
Bandmitten ist als Registerzeile [Härtung] geführt, nicht als Befund
(`:7292-7294`, §42.2).

### R-311-18 — zwei Listen mit Testnamen und Fundstelle

**NEIN — DEFEKT.** Die zwei Listen (§40.4 `:7078-7087` und `:7098-7141`) sind
gegenüber dem Vorstand deutlich vollständiger, erfüllen die Regel aber nicht:

1. **`311/M-76 gleicher_ladestart_bleibt_warm (NAK-311 F12, M-84)`**
   (`eq-copilot/plugin/tests/TransactionTestMain.cpp:3008`, Prüfstand
   `:2976-3012`) steht in **keiner** der zwei Listen. Der Prüfling ist ein
   dynamisches Band: `nak311DynBand` (`:2828-2849`) setzt
   `kDynamicEnabled` = true (`:2841`) und `kDynamicRangeDb` ≠ 0 (`:2842`). Ein
   `grep` über `docs/beweise/NAK-311.md` findet im ganzen §40.4 keinen Treffer
   für `M-76` oder `gleicher_ladestart`.
2. **`311/M-79 audiohistorie_ist_kein_state (NAK-311 F12, R-311-4)`** (`:3103`)
   steht nur in „Tests und Beine" (`:7172`, „B7 `311/M-79` erweitert"), nicht in
   einer der zwei Listen und damit ohne Richtung und ohne eigenen Grund.
3. Die Sammelzeile `:7138-7141` führt fünf **Fundstellen ohne Testnamen**:
   „In `TransactionTestMain.cpp` fassen `:1095-1107`, `:1145-1156`,
   `:1523-1541`, `:1575` und `:2685-2700` ein dynamisches Band an, messen aber
   Register, Hash, Revision und Schemagültigkeit". Genau das schließt R-311-18
   aus („Umschreibungen ohne Testnamen entfallen"). Betroffen ist mindestens
   `dynamicwerte_ueberleben_aus_und_ein_bitgleich (M-24, ueber zwei
   Transaktionen)` (`:1542`), dessen Name im Manifest nicht vorkommt.
4. Zeile `:7086` der ersten Liste nennt „B7 `311/M-77` und `311/M-78` (F12,
   R-311-4)" — Matrixkennungen statt der Testnamen
   (`311/M-77 recall_in_neue_instanz_innerhalb_der_toleranz_pruefling_<n>`,
   `311/M-78 recall_nach_preparetoplay_innerhalb_der_toleranz_pruefling_<n>`,
   gebildet an `:3063-3067`) und **ohne Fundstelle**.

*Was an R-311-18 stimmt.* Keine Prüfung steht doppelt; die 14 Zeilen des
Abschnitts F und die 13 Zeilen außerhalb tragen Testname, Fundstelle und einen
eigenen Grund. Sechs Gründe habe ich am Testmaterial geprüft, alle stimmen:
`unter_threshold_steht_das_band_exakt_still (M-19)` (`:2049`) — Amplitude
1e−6 gegen Threshold −10 dB ergibt 10·log10(1e−12/2) = −123,0 dB, also rund
113 dB darunter, Kennlinie bitgenau 0,0;
`range_null_ergibt_exakt_null_auslenkung (M-20)` (`:2079`) — Range 0 ⇒
`detektorLaeuft` falsch (`DspProgramm.cpp:318`) ⇒ der Detektorzweig und damit
die Pegelstufe laufen nie (`DspKern.cpp:660-687`);
`kennlinie_im_knie_wirkt_im_audiopfad (M-18, B-14)` (`:2187`) — Quadraturton
über `fahreStereoTon` an `:2175-2176`, Detektorleistung konstant a²/2, und der
Fixpunkt eines Ein-Pol-Mittels bei konstantem Eingang ist bitgenau dieser
Eingang; `detektor_ist_bandbegrenzt (M-21)` (`:2226`) — 1 kHz liegt 36 dB über
Threshold −40, also im Plateau (Auslenkung exakt −9,0), 60 Hz kommt mit
|H| ≈ (1/Q)(f/f0) = 0,015 bei rund −77 dB heraus, weit unter Threshold
(Auslenkung exakt 0,0), und die Schranke ist die lose Ungleichung
|außen| < 0,5·|innen|; `detektor_hoert_vor_dem_band (M-21)` (`:2239`) —
Plateau, Schranke 1e−6 gegen −12,0;
`range_…_rampt_ueber_die_volle_rampe` (`:2572-2578`) — gelesen wird
ausschließlich `huelle.leistung` (`:2564`, `:2569`), und je Lauf läuft nur eine
Richtung (`:2534-2536`). `SondeNullTestMain.cpp` und `SchemaTestMain.cpp`
enthalten **null** Treffer für `kDynamicEnabled`/`kDynamicRangeDb`, was den
Satz „A16 fährt kein dynamisches Band im Nulltestmaterial" (`:7136`) belegt.

### R-311-19 — die eine `$comment`-Zeile und ihre Folge

**JA.** An der Quelle nachgemessen:
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2324` ist `"auto_gain_db": {`,
`:2325` der `$comment` (Wortlaut in §38.2 `:6026-6031` bytegleich zitiert),
`:2326` `"type"`, `:2327` `"minimum": -120`, `:2328` `"maximum": 120` — der
Block `:2324-2329` und die Einzelzeile `:2325` stimmen.
`git ls-files --eol` meldet `i/crlf  w/crlf  attr/-text`, und `.gitattributes`
trägt die Regel `eq-copilot/schemas/v3/**  -text` **auf Zeile 35** — beide
Angaben aus `:6716-6718` stimmen wörtlich. Der Wortlautvorschlag
(`:6708-6712`) ist reines ASCII. §38.2 führt die Zeile nicht mehr als
„unverändert", sondern als „**eine Zeile geändert**" (`:6013-6014`). M-140
(`:6394`) trägt die benannte Ausnahme („genau eine Zeile … sonst leer") und
seine Mutation wirkt nach dem Bau: ein zweites Feld in `$defs/dsp_bericht`
erzeugt eine **zweite**, nicht ausgenommene Zeile im Diff und ändert die
Bedeutung von `required`/`additionalProperties` → A5 und B3c melden das
geänderte Schema. §40.2 trägt die Folge vollständig (`:6726-6734`: Broker
bettet das Schema ein, `--broker-pin` schreibt die Brokerzeile in
`eq-copilot/install/nakama-installer-v1.json`; `:6747-6752`: A5, A8, A9, A4 im
Änderungssatz). `minimum`, `maximum`, `type` und jede andere Schemazeile
bleiben; kein Vertragswechsel, `dsp_schema_version` steigt nicht. §41.3 Nr. 4
(`:7421-7432`) führt den Widerspruch als vierten Fall mit demselben
Vorschlag.

### R-311-20 — das dritte Fenster in Kern und Bericht

**JA.** Die Aufzählung in §40.1 Nr. 2 (`:6540-6558`) habe ich Glied für Glied
gegen `eq-copilot/plugin/dsp/DspKern.cpp` gelesen; sie ist vollständig und
exakt:

| Angabe des Nachtrags | Quelle |
|---|---|
| achtzehn Arbeitspuffer und `tapPuffer` geleert | `:129-133` (die Initialisierungsliste trägt genau 18 Vektoren, nachgezählt) |
| `maxBlockGroesse` = 0 | `:134` |
| `abtastrate` = 0,0 | `:135` |
| `kanalzahl` = 2 | `:140` |
| `baenke.zuruecksetzen()` (Ressourcen, nicht der Generationszähler) | `:143` samt Kommentar `:141-142` |
| alle Pfade in Ruhe | `:144` |
| `hoerwunsch`, `hoerwirksam`, `hoerLaufend`, `hoerVorher` auf `processed`, `hoerFadeRest` = 0 | `:145-149` |
| `candidateAktiv`, `dynamikAktiv` falsch | `:150-151` |
| `tapGueltig` = 0, `letzteKanaele` = 0 | `:152-153` |
| **zusätzlich**, was `freigeben` heute NICHT tut: `autoGainBericht` genullt | `:120` (steht nur in `bereiteVor`) |
| **zusätzlich**: `zaehlerZuruecksetzen()` | `:124` (steht nur in `bereiteVor`) |

`DspKern::autoGainDb()` liest `autoGainBericht[0]` (`DspKern.h:312`), ist nach
dem Nullen also exakt +0,0. §40.1 Nr. 3 (`:6559-6574`) beruft sich für das
dritte Fenster ausdrücklich auf R-311-20 und grenzt R-311-12 auf seine zwei
benannten Fenster ab; die zwei Codebehauptungen stimmen:
`Transaktionskern::setzeSamplerate` bekommt Rate und Kanalzahl gemeinsam
(`NakamaTransaktion.cpp:350-357`, `kanalzahl = kanaele` an `:356`), und
`baueBericht` betritt den Programmzweig nur bei `fs > 0.0` (`:806-833`, Wächter
an `:807`), liest die Kanalzahl im dritten Fenster also nie. M-143 (`:6388`)
misst 48 → 32 → 48 kHz in Kern und Bericht, nennt +0,0 für `autoGainDb()` und
für `auto_gain_db`, leere `klemmungen`, Kanalzahl 2 (`:140`) und den Zustand in
beiden; der Rotbeweis fällt an der Zeile (Nullen von `:120` weggelassen → der
48-kHz-Wert bleibt im 32-kHz-Fenster stehen), mit einem zweiten Teilfall für
`:124`. M-138 (`:6386`) trägt die Berichtshälfte, M-132 (`:6380`) die
Kanalzahl. Die Beziehung `bereiteVor` ↔ `freigeben` ist in §39.6 geführt
(`:6456`).

---

## Frage 2 — hat der Fixdiff etwas gebrochen?

**Nein.** Geprüft und nicht gebrochen:

**Code am HEAD.** Jede in einer geänderten oder neuen Zeile genannte
Fundstelle habe ich gelesen. Bestätigt: `DspKern.cpp:120`, `:124`, `:127-154`,
`:140`, `:220-226`, `:286-288`, `:495-507`, `:507`, `:515-516`, `:518-541`,
`:536`, `:546-547`, `:660`, `:664-687`, `:690-714`, `:741-745`, `:1013`,
`:1122`; `DspKern.h:312`; `DspProgramm.cpp:120`, `:169`, `:207-227`, `:212`,
`:288`, `:318`; `DspProgramm.h:95-97`; `DspFilter.h:90-96`, `:224-238`,
`:266-271`, `:273-278`, `:280-288`, `:290-329`, `:312-328`, `:336-341`,
`:355-361`; `NakamaTransaktion.cpp:350-357`, `:356`, `:806-833`;
`SondeProcessor.cpp:237-238`, `:258`; `eq-ipc-v3.schema.json:2324-2329`,
`:2325`; `.gitattributes:35`; `nakama-parameter-v2.json:52-58`, `:180-190`;
`SONDE-015.md:1711`, `:3412`, `:3419`, `:3423`, `:3741`;
`DspGoldenTestMain.cpp:1873-1878`, `:1931-1933`, `:2049`, `:2057`, `:2079`,
`:2175-2192`, `:2226`, `:2239`, `:2375-2435`, `:2531-2578`, `:4013-4014`,
`:4193-4228`, `:5397-5401`, `:5479-5481`, `:5670-5694`, `:6812-6853`;
`SondeNullTestMain.cpp:195-204`, `:379`, `:399`, `:522-540`, `:658-664`;
`TransactionTestMain.cpp:130-161`, `:246-289`, `:577`, `:1009-1021`, `:1396`,
`:1502-1518`, `:2222`, `:2828-2849`. Die zwei im Fixdiff **berichtigten**
Fundstellen stimmen jetzt: `SondeNullTestMain.cpp:522-540` trägt
`eq_an_bypass_aus_alles_neutral_ist_bitidentisch` mit der Ratenschleife
44,1 / 48 / 96 kHz an `:523` (192 kHz an `:379` und `:399`), und das
Auto-Gain-Gitter läuft an `DspProgramm.cpp:120` (Monozweig) und `:169`
(Zweikanalzweig) über `autoGainGitterHz`, Stellenzahl `DspProgramm.h:95-97`.

**Gate-Text und §6.** Der Gate-Satz „Je Matrixzeile fällt der Rotbeweis an der
Zeile, die die Zusage trägt, nicht an einem Nebeneffekt" steht im Kopf der
Datei und in §39 (`:6267-6268`). Die drei neuen Zeilen halten ihn: M-141 fällt
am mitzunullenden Pegelzustand (und der Bestandstest fängt es nachweislich
nicht auf), M-142 an der eigenen Schranke 1e−5 der zwei Bestandsprüfungen,
M-143 am nicht genullten `autoGainBericht`. Die Etiketten der drei neuen
Zeilen stimmen gegen §6 `:513-526`: M-141 und M-143 „heute nicht messbar"
(Pegelzustand beziehungsweise drittes Fenster entstehen im Bau; die Konstanten
fehlen am HEAD), M-142 „Regressionswache" (heute grün, von den Kriterien
unberührt, einmal absichtlich gebrochen).

**Regeln R-311-1 bis R-311-20, Entscheide §42 und §43.** Kein Widerspruch.
R-311-13s dritter Satz hat mit M-142 jetzt eine tragende Wache. Der Entscheid
`kSprungGainDb` = 20,0 mit der Messreihe 12 / 18 / 20 / 24 / 36 / 48 dB ist
umgesetzt. Der Wortlautwiderspruch R-311-12 gegen R-311-20 ist vom Dirigenten
in §45 entschieden und nach Auftrag ausgeschlossen; §40.1 Nr. 3 setzt den
Entscheid richtig um.

**Unberührte Matrixzeilen.** M-100, M-101, M-104, M-112, M-130, M-131 und
M-136 sind vom Fixdiff nicht angefasst und werden von keiner geänderten Zeile
überholt. M-100 bleibt die Wertwache des Frequenzkriteriums, M-142 tritt für
Gain und Güte daneben (kein Doppel: M-142 nennt M-100 ausdrücklich als das
Muster). M-102 verweist für den **Wert** auf M-142 und behält seine eigene
strukturelle Mutation. Die berührten M-128, M-132, M-133, M-138 und M-140
widersprechen keiner unberührten Zeile.

**SONDE-015-Zusagen.** E-25, E-29, W-3, E-17 und E-19 sind wörtlich zitiert
und bytegleich mit der Quelle; drei davon bekommen nach §41.3 einen datierten,
append-only Nachtrag, keiner wird still umgeschrieben.

**CLAUDE.md-Invarianten.** Ausgeschaltet bitidentisch und Passthrough
sampleidentisch stehen in §39 (`:6270-6272`) und §40.1 (`:6526-6534`); der
Audio-Thread bleibt frei von Sperren, Allokationen, Datei-, Pipe- und
Netzzugriffen und Logging (M-111, M-129, M-139; die drei neuen Vergleiche und
die zwei neuen Getter laufen im Worker, `DspKern.cpp:261-292`); NaN-Ehrlichkeit
trägt M-109 und M-130; keine neue Stateversion; die v3-Änderung bleibt auf die
eine `$comment`-Zeile beschränkt und ist in M-140 als benannte Ausnahme
geführt; Identität eingefroren; RBJ- und Stereo-Goldens bitgenau (M-82, M-83).

**Je Matrix-ID genau eine Zeile.** Ja, maschinell nachgezählt: 46 Tabellen-
zeilen in §39.1 bis §39.5, jede ID genau einmal, lückenlos M-96 bis M-120 und
M-123 bis M-143, M-121 und M-122 frei.

**§39.6 stimmt am Text.** Nachgezählt: Gegenstände 17 + 8 + 11 + 9 + 1 = **46**
(M-96 bis M-111 plus M-142 = 17; M-112 bis M-119 = 8; M-120 plus M-123 bis
M-131 plus M-141 = 11; M-132 bis M-139 plus M-143 = 9; M-140 = 1). Etiketten
maschinell aus der Rotbeweisspalte gezogen: **heute rot 9** (M-96, M-97, M-98,
M-99, M-112, M-120, M-124, M-132, M-135), **heute nicht messbar 19** (M-101 bis
M-104, M-108, M-109, M-113, M-115 bis M-118, M-125, M-127, M-130, M-131,
M-134, M-139, M-141, M-143), **Regressionswache 16** (M-100, M-105, M-107,
M-110, M-111, M-114, M-119, M-123, M-126, M-128, M-129, M-133, M-136, M-137,
M-140, M-142), **zwei Hälften 2** (M-106, M-138 — beide tragen „zwei Hälften"
als führendes Etikett und je Hälfte ein zweites). Summe 46.

**§40 nennt jede neue Zeile im richtigen Änderungssatz.** M-141 → Satz D
(`:7165`, B6 Abschnitt F) und in §40.4s Rotbeweisabsatz (`:7182-7183`);
M-142 → Satz C (`:6934-6936`, B6 bestehend
`bandwert_gain_db_…`/`bandwert_q_…`) und in der Wertwachentabelle
(`:6958-6960`); M-143 → Satz A (`:6588`, B7 neben `311/M-135`, B3c als
Teilfall von `311/M-138`) und im Rotbeweisabsatz (`:6604-6607`). Die drei sind
in §39.6 je Regel und je Etikett mitgezählt.

**Kein Änderungssatz braucht etwas, das erst ein späterer liefert.** Die
Reihenfolge A (R-311-16) → B (R-311-14) → C (R-311-13) → D (R-311-15) steht in
§40.0. Geprüft: M-143 liegt in Satz A und nennt in seiner Zustandsspalte
`kAutoGainDeckelDb`, das erst Satz B liefert — die Zusage selbst braucht es
nicht, weil sie ausschließlich selbstbezüglich vergleicht (+0,0 im
32-kHz-Fenster, „denselben abgeleiteten Wert, bitgleich zum Stand vor dem
Wechsel" danach) und der Rotbeweis am Nullen von `:120` hängt. Siehe
HÄRTUNG H-2.

---

## Lücken

**L-1 — `311/M-76` und `311/M-79` ohne Regelbezug für das Verschieben.** Die
zwei Prüfungen des Abschnitts T fehlen in den zwei Listen (siehe R-311-18).
R-311-18 verlangt die Einordnung, sagt aber nicht, in welche Liste eine
Recall-Prüfung gehört, deren Verschiebung erst über den neuen Zustand im
`BandZustand` entstehen könnte. Benannt, keine Nacharbeit.

---

## Härtungen

**H-1 — die volle Kette liegt 0,34 bis 0,47 ms über dem Sollwert 31,7 ms.**
§40.4 und M-127 nennen die Zahlen der **Polreihe** (31,75 / 31,83 / 31,75 /
31,71 ms) und etikettieren sie in §44.3 auch so. Die vom Test wirklich
gemessene Attackzeit trägt zusätzlich den Einschwingvorgang des
Detektor-Bandpasses und die Steuerrate; meine Nachfahrung der ganzen Kette
ergibt 32,11 / 32,17 / 32,08 / 32,04 ms. Mit Sollwert 31,7 ms schrumpft der
Rand in der 1-ms-Toleranz von 1,00 auf 0,53 ms (48 kHz). Von keiner Zusage
verlangt; der Bau kann den Sollwert der Bauetappe aus der Reihenreferenz des
Tests nehmen statt aus der Absolutzahl.

**H-2 — M-143 nennt eine Konstante des nächsten Änderungssatzes.** „+42,99 dB,
gedeckelt auf `kAutoGainDeckelDb`" in der Zustandsspalte ist zur Bauzeit von
Satz A noch nicht wahr (der Deckel kommt in Satz B). Ohne Wirkung auf Zusage
und Rotbeweis, aber eine Stelle, die der Bauer von Satz A nicht wörtlich lesen
darf.

**H-3 — `311/M-76` als Beziehung.** Die Recall-Prüfung
`gleicher_ladestart_bleibt_warm` ist die Gegenseite von M-141
(aktivieren↔abklingen) für den Ladeweg: sie prüft, dass ein eingeschwungener
dynamischer Zustand denselben Instanz-Ladestart warm überlebt. Ein neuer
Zustand im `BandZustand` berührt sie im Prinzip; M-131 deckt die Übertragung
am Blockrand, nicht den Ladestart. Von keiner Zusage verlangt.

---

## Geprüft

- Fixdiff vollständig gelesen, mit `-U0` an den Hunkgrenzen nachgemessen: 53
  Hunks, einer in der Kopfzeile „Etappe", 51 in §38 bis §41 in place, einer für
  den neuen §44. Nur `docs/beweise/NAK-311.md` geändert.
- **Selbst an der Quelle nachgemessen** (nicht nur gelesen): die
  `bleibt`-Bedingung und der Blockrand in `DspKern.cpp`; `rampenKompatibel`;
  das Nullen an `:741-745`; `freigeben` gegen `bereiteVor` Glied für Glied;
  `setzeSamplerate` und der `fs > 0`-Wächter in `baueBericht`; der
  `prepareToPlay`-Wächter; der `$comment`-Block samt `minimum`/`maximum`,
  `git ls-files --eol` und `.gitattributes:35`; der Prüfstand
  `attack_hold_release_als_sprungantwort_bei_vier_raten`; der Rampenblock
  `bandwert_<feld>_rampt_ohne_zustandsreset`; `uebernahme_nur_am_aeusseren_
  blockrand`; `range_…_rampt_ueber_die_volle_rampe`; die sechs geprüften
  Gründe der zweiten R-311-18-Liste; die zwei berichtigten Fundstellen
  (`SondeNullTestMain.cpp:522-540`, `DspProgramm.cpp:120`/`:169`); die
  `machDynamisch`-Aufrufe der vier Testdateien; alle Publikationspaare mit
  Wertwechsel in den vier Testdateien.
- **Selbst nachgerechnet** (Python nur für Formeln und Rekursionen): der
  63-%-Punkt der Polreihe analytisch und diskret für vier Raten, sowohl als
  reine Polreihe wie als volle Kette des Prüfstands, samt Hold und Release und
  samt Basisstand zur Gegenprobe; die Restwelligkeit r und die vier Schranken
  von R-311-17; alle sieben Zahlen der D-4-Rechnung; die Gitteranteile und
  +0,9728 / +2,7243 / +1,7609 / +6,9748 dB; 24/6,9748; 10^(24/20); die Abstände
  und Faktoren zu `kSprungGainDb` = 20,0; die Bandpassdämpfung bei 60 Hz für
  `detektor_ist_bandbegrenzt`.
- **Maschinell nachgezählt am Text:** 46 Matrixzeilen, Eindeutigkeit jeder ID,
  die vier Etikettzahlen, jede Stelle in §38 bis §41 mit „18,0" oder
  `kSprungGainDb`, jedes Vorkommen von `M-76`/`M-79`/`gleicher_ladestart` in
  §40.4, die 18 Vektoren in `freigeben`, `belege`-Aufrufe je Testdatei,
  `machDynamisch`-Aufrufe, `kDynamicEnabled`/`kDynamicRangeDb` je Testdatei.
- Gelesen zur Einordnung: §6 (Etiketten und Matrixregeln), §37, §42, §43,
  §44, §45 der Manifestdatei; `NAK-311-matrixpruefung-4-urteil.md` (Befunde),
  `NAK-311-matrixpruefung-4-validierung.md` (D-1 in Gänze, Präzisierung und
  Beifang), `NAK-311-matrixnacharbeit-3-auftrag.txt` (S-D1 bis S-D5).

## Nicht geprüft

- Der unberührte Bereich von §38 bis §41 (in der Matrixprüfung 4 geprüft),
  §1 bis §37, §42, §43 und §45 als Prüfgegenstand; §44 als Text (nur als
  Wegweiser gelesen, jede gestützte Aussage an der Quelle nachgemessen).
- Alles, was der Auftrag ausschließt: H-1 bis H-4 der Matrixprüfung 4; der in
  §44.7 Nr. 1 benannte Wortlautwiderspruch R-311-12 gegen R-311-20; die
  veraltete Fundstelle `SondeNullTestMain.cpp:356-373` in §1 und §6; die Länge
  des Kopfsatzes; der Wortlaut des neuen `$comment`; die Zahlen der Regeln als
  Vorschläge (nur ihre Herleitung nachgerechnet); Prosa, Zeilenverweise,
  Trefferzahlen und Historie außerhalb der berührten Zusagen; die
  Produktentscheide U44 bis U47 und U54; Sicherheitsbefunde; datierte
  Härtungen des Registers; `tools/dirigent/pruefliste.md`; §41.1, §41.4, §41.5
  und §41.6 als Prüfgegenstand.
- Jede Zahl, die erst die Bauetappe messen kann: die drei Wertekriterien am
  Prüfling, `kPegelFensterMs` gegen die vier Bandmitten, `kAutoGainDeckelDb`
  gegen ein vertragsgültiges Programm gewöhnlicher Arbeit, die
  Übergangsspitzen von M-96 bis M-99 am unveränderten Kern. Kein Bau, kein
  Test, kein FL Studio gefahren.
- Die Vollständigkeit meiner eigenen Suche nach weiteren Rampenfällen: ich habe
  alle direkten Wertzuweisungen und alle `belege`-Aufrufe der vier Testdateien
  durchgesehen sowie die Helfer `setzeBand`, `grundzustand`, `mitGain`,
  `hostSchreibt`, `nak311DynBand` und `resonator` und die Prozessorabschnitte
  M bis T von `TransactionTestMain.cpp`. Die zwei gefundenen Fälle belegen die
  Lücke; dass es keine dritten gibt, behaupte ich nicht.

**HEAD am Ende `e1d98973bfbf4e99acdb8775550ce2fdb9b316f5`** — gleich dem Stand
zu Beginn. Nichts gestaget, nichts committet, nichts gepusht; die zwei fremden
untracked Ordner `briefing-hub/` und `nimbalyst-local/` sind unberührt.
