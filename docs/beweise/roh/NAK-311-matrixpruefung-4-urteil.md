URTEIL: NEEDS_WORK

# NAK-311 — Matrixprüfung 4 (Etappe 5, Matrixnachtrag und Bauplan)

Frischer, lesender Opus-Thread, Effort `max`, 20.09.2026. BASIS
`8408d78274355f0244f2d95e9526f8178a118b80`, ZIEL
`9a98989a2ecbda7644d13879d11d0421051e95da`. Prüfbereich: §38 bis §41 des
Diffs (1 374 Zeilen, 43 Matrixzeilen M-96 bis M-120 und M-123 bis M-140);
Gate-Bezug Kopf, §37 und §42. Gelesen, nicht gebaut: kein Compiler, kein
Produktbinary, keine Repo-Datei geändert, nichts gestagt oder committet.
Geschrieben wurde genau diese Datei.

**Gate-Text.** Der Kopf zitiert den Text des Schritts S25j aus
`docs/plan/plan.json` ab „INHALT:" wörtlich; die GRENZE-Passage in §38.0 ist
mit dem Feld `text` zeichengleich. Der Kopf lässt den Vorsatz „ANGELEGT
18.09.2026 (Register NAK-311; Arbeitsübersicht Phase 16 W01, W03, W04, W07,
W08, W35)." ohne Auslassungszeichen weg; inhaltlich fehlt nichts.

---

## Prüffrage 3 (a) — R-311-13 (T3-15-07, U44)

**Die Stelle trägt die Regel in beiden Richtungen.** `vergebeKennungen`
(`eq-copilot/plugin/dsp/DspKern.cpp:261-292`) vergleicht heute in der
`bleibt`-Bedingung (`:286-288`) ausschließlich `typ`, `modus`, `dynamisch`,
`nutztSvf`, `quelle` und `aktiv`; `freq_hz`, `q` und `gain_db` kommen darin
nicht vor, und `rampenKompatibel` (`DspProgramm.cpp:207-227`) kennt ebenfalls
keine Wertegrenze. Drei weitere Vergleiche in `bleibt` erzeugen genau das von
R-311-13 verlangte Verhalten: der gesprungene Slot bekommt über
`naechsteKennung()` eine neue Kennung, im Blockrand fällt damit
`kennungenGleich` (`:495-507`) und `nurRampen`, `pfadGleich` bleibt aber wahr,
sodass der `else if (pfadGleich)`-Zweig (`:518-541`) genau die Slots mit
gleicher Kennung überträgt und der Übergang mit `z.rest = kFadeSamples`
(`:547`) als Crossfade läuft. Ein Sprung unter dem Kriterium und die Stufenfahrt
behalten ihre Kennung und bleiben bitgleich auf dem Rampenweg. **F-22 hält:**
`blockrand` bleibt unberührt, der Audiothread liest weiter nur zwei 64-Bit-
Kennungen je Slot; die drei `double` liegen im `SlotMerkmal`
(`DspKern.h:409-418`) des Workers. **F-21 ist mit der NaN-Ehrlichkeit
vereinbar:** ein Vergleich `verhaeltnis > grenze` ist mit NaN falsch und
übertrüge den Zustand in eine Bank mit nicht endlichen Koeffizienten; der kalte
Start ist der sichere Ausgang, und der Eingangsriegel (`DspKern.cpp:1087-1093`)
sowie die Heilung am Blockrand (`:560-578`) bleiben, wie sie sind.
**Automationstreppe und zweiter Sprung:** `blockrand` kehrt bei laufendem
Übergang früh zurück (`:460`), der Worker verdrängt Zwischenpublikationen, und
`vergebeKennungen` vergleicht ausdrücklich gegen die zuletzt **publizierte**
Belegung (`:263-268`) — M-105 und M-106 (a) treffen genau diese zwei Stellen.
**Die Bestandsfälle liegen unter den Kriterien.** Nachgerechnet am ZIEL:
M-17 `freq_hz` 1000 → 1400 (Verhältnis 1,4 gegen 2,0), `q` 1 → 3 (3,0 gegen
4,0), `gain_db` 3 → 9 (6,0 dB gegen 18,0 dB), alle drei in
`DspGoldenTestMain.cpp:1873-1878`. M-94 (a) ändert `gain_db` +12 → +6
(6,0 dB, `:6828-6831`), M-94 (b) nur `dynamic_range_db` −12 → −6, und Range
ist nach F-20 kein Kriterium — §38.1 hält hier wörtlich. Abschnitt E (M-43 bis
M-45, M-54, M-55) ändert nur `v1.band.5.enabled` beziehungsweise den Typ von
Slot 5; Abschnitt Q (M-46 bis M-51, M-53, M-94, M-95) ändert Typ, Remove,
`dynamic_enabled`, Mono-Bass, Samplerate, Output-Trim −3 dB und die zwei
Wertfälle von M-94 — kein Slotwert reißt ein Kriterium. Abschnitt P
(`ausUndEin`, `:5940-5964`) fährt jeden Fall über die Ruhe, nicht über den
Rampenweg. **Daraus folgt D-1:** die in M-103 und §40.3 als „größter
Bestandsfall auf dem Rampenweg" genannte Zeile ist kein Rampenfall, und der
darauf gestützte Rotbeweis fällt an nichts.

## Prüffrage 3 (b) — R-311-14 (T3-15-09 Teil b, U54)

**Die Stelle ist richtig gewählt.** `leiteAutoGainAb` hat vier Ausgänge, die
0,0 liefern (`DspProgramm.cpp:96`, `:161-162`, `:201-202`) und zwei, die
rechnen (`:163` Monozweig, `:203` Zweikanalzweig); nur die zwei letzten können
die Obergrenze reißen. `baueProgramm` legt den Rückgabewert in
`aus.autoGainDb` und leitet `autoGainLin` daraus ab (`:339-340`),
`meldeProgramm` veröffentlicht ihn (`DspKern.cpp:220-226`), das Rampenziel
liest ihn (`:557`), und `baueBericht` baut sein eigenes Programm mit derselben
Funktion und derselben Kanalzahl (`NakamaTransaktion.cpp:817-822`). Programm,
Kern und Bericht lesen damit wirklich denselben Wert; die zweite Stelle aus
§34.13 Nr. 1 (Rampenziel) täte das nicht. Der Monozweig endet mit `return`
(`:163-164`), ein Deckel hinter der Verzweigung träfe ihn nicht — M-115 trifft
das genau. **Die vierzehn Hexwerte halten.** Aus
`docs/beweise/roh/NAK-311-etappe4-autogain-hex.txt` je Wert geprüft:
0,000000000000 (dreimal), −5,997027825804 (dreimal), −5,860616750098
(zweimal), −4,312390025805, −2,670806897046 (zweimal), −2,270777818370
(zweimal), +0,230273950213. Vierzehn Werte, alle unter +24,0 dB und sämtlich
auf der ungedeckelten Seite; M-119 hält wörtlich, und sein Rotbeweis (Deckel
0,1 → `M47_gemischtes_programm` wandert) ist deterministisch. **M-61, M-65 (b),
M-67, M-68 und M-71 bleiben gültig**: `berichtsAutoGainDb`
(`NakamaTransaktion.cpp:777-789`) klemmt weiter auf ±120 und lässt alles
darunter bitgleich durch, `klemmungen` bekommt nur R7-Einträge (`:823-832`).
**Der Zustand fällt zurück**, weil `meldeProgramm` ihn je publizierter Bank neu
setzt (`DspKern.cpp:220-226`) — M-116 hängt ihn richtig am gefahrenen
Programm. **F-24 ist mit U54 vereinbar:** U54 verlangt die ehrliche Meldung,
wenn die Obergrenze greift; greift sie bei ausgeschaltetem `v2.global.auto_gain`,
hört niemand einen Ausgleich, und `CLAUDE.md:51-52` verlangt genau dann keine
Meldung. Offen bleibt die Bedeutung des Drahtfelds (L-3).

## Prüffrage 3 (c) — R-311-15 (T3-15-06, U45)

**Die Reihe schließt den Befund am Kern des Falls.** Der Fixpunkt ist
belastbar: `pol · x + (1 − pol) · x == x` habe ich in IEEE-754-Doppelgenauigkeit
für 10 ms bei 44,1 / 48 / 192 kHz und vier Eingangswerte geprüft — bitgenau
wahr. Die Iteration aus 0 bleibt rund 3·10⁻¹⁴ relativ darunter stehen, also
etwa 10⁻¹³ dB (H-3). Die Restwelligkeit r = 1/(4π·f0·τ_m) mit τ_m = 10 ms habe
ich nachgerechnet: 0,39789 bei 20 Hz (1,4547 dB), 0,023337 bei 341 Hz
(0,1002 dB), 0,0079577 bei 1 kHz (0,0344 dB), dazu 0,0796 bei 100 Hz
(0,3325 dB) — alle Zahlen aus §40.4 und M-120 stimmen, ebenso die 171 ms für
0,1 dB bei 20 Hz, σ = 0,150 mit B_n = 2221,8 Hz und 10·log10(1+3σ) = 1,614 dB
sowie die 3,01 dB Spitzen-/Effektivwertabstand. **Attack, Hold und Release
bleiben wörtlich einstellbar:** `HuellkurveZustand::tick`
(`DspFilter.h:312-328`) wird nicht angefasst, `huellkurveEntwurf` (`:280-288`)
behält seine drei Parameter — U45 ist erfüllt. **Der Befund wird am
eingeschwungenen Zustand geschlossen, aber nur bei hoher Bandmitte gemessen**
(L-1): M-124 fährt die neun Hold-/Attack-Einstellungen ausschließlich bei
f0 = 1 kHz, wo die Restwelligkeit höchstens 0,034 dB Versatz zulässt; bei 20 Hz
bliebe ein gleichrichtbarer Rest von 1,45 dB gegen heute 3,01 dB, und keine
Zeile misst dort. **Die Liste der verschobenen Bestandszeilen** habe ich gegen
jeden Fall mit `machDynamisch` beziehungsweise `dynamic_enabled` in den vier
Testdateien abgeglichen (über vierzig Fundstellen). Kein Test außerhalb der
Liste verschiebt seine Zahl: die drei in keiner der beiden Listen genannten
F-Prüfungen `range_null_rechnet_keinen_detektor (M-20)`,
`huellkurvenwert_<feld>_holdzaehler_rundet_exakt (NAK-289)` und
`range_..._rampt_ueber_die_volle_rampe (M-17, R8, B-4, W-3)` messen entweder
gar keinen Detektor, oder ihren Holdzähler bei durchweg steigendem Eingang,
oder das Plateau — in allen drei Fällen ohne Wirkung des Pegelpols. Die
Begründung von M-128 greift jedoch bei zwei genannten Prüfungen nicht (L-2),
und **die Absolutzahl der Sprungantwort ist falsch** (D-3). **F-19 ist mit W03
vereinbar:** der Begriff ist topologisch, `baueProgramm` setzt ihn unbedingt
auf Durchschnitt, zwei Produktprogramme unterscheiden sich darin nie, die
Bedingung in `rampenKompatibel` (`:216-225`) und in `bleibt` ist damit im
Produkt immer wahr, und M-43 bis M-56, M-94 und M-95 messen unverändert
dasselbe. **Nicht getragen ist der Rückweg des neuen Zustands** (D-2).

## Prüffrage 3 (d) — R-311-16 (F08, U47)

**Der Weg ist der des ausgeschalteten EQ.** `verarbeiteStueck` kehrt bei
`(size_t) numSamples > dryL.size()` vor dem ersten Sample zurück
(`DspKern.cpp:1013`); nach `freigeben` (`:127-154`) ist `dryL` leer, also
schreibt der Kern keinen Sample, es gibt keinen `float → double → float`-
Rücklauf und keinen Tap. F-26 begründet die Wahl gegen die Neutralprüfung der
Etappe 2 richtig: die deckt nur M-02 und M-33. Auch die Verwerfung des
Vorschlags aus der Quellvalidierung Teil B §7.5 ist am Code belegt — läge die
Schranke an `sichereRate` (`SondeProcessor.cpp:237-238`), liefe der ganze
Block `if (sichereRate > 0.0 && maxBlock > 0)` (`:258`) nicht mehr, der Kern
bliebe beim Wechsel 48 → 32 kHz auf seiner alten Vorbereitung stehen und
rechnete weiter. **Die Schranke steht an einem Ort** (F-25): Konstante und
Prädikat einmal in `DspProgramm.h` neben `kFadeSamples` (`:62`) und
`kRampeSamples` (`:66`), drei Leser. **44 100 Hz bleibt unterstützt:** bei
44,1 kHz liegt keine der 121 Gitterstellen über Nyquist (selbst nachgerechnet),
bei 32 kHz sind es vier und bei 22,05 kHz elf — genau die Zahlen aus
`roh/phase-16-d-dsp.md:24` und `:57`, deren −0,397113 dB gegen −0,234376 dB
ich dort wörtlich wiedergefunden habe. Das Gitter selbst
(`autoGainGitterHz`, `DspProgramm.cpp:76-83`) hat keine Nyquistkappung; die
Fundstellenangabe der Gegenprobe zeigt allerdings am ZIEL auf einen Kommentar
(H-1). **`eq_enabled` bleibt unberührt:** nichts Persistentes ändert sich, und
`baueBericht` betritt den Programmzweig nur bei `fs > 0`
(`NakamaTransaktion.cpp:806-833`), sodass `autoGainDb` +0,0 und `klemmungen`
leer bleiben wie heute bei Rate 0. **Der Zustand fällt mit dem nächsten
`prepareToPlay` zurück,** weil `vergissLetztePublikation` und
`publiziereWirksam` (`SondeProcessor.cpp:271-272`) den bestätigten Zustand
erneut publizieren. **Mit R-311-12 verträgt sich der Weg im Ergebnis, nicht im
Wortlaut** (L-4): R-311-12 sagt „Die Kanalzahl folgt auf jedem Weg der
Samplerate", während §40.1 Nr. 3 für das neue, dritte Fenster die Kanalzahl
ausdrücklich übernimmt und R-311-12 dafür zitiert. **Die Analyse bleibt
unberührt:** `v3Samplerate`, `v3BlockSize` und `v3Channels` stehen vor dem
Wächter (`:245-251`) — M-137 ist am Code korrekt.

## Prüffrage 3 (e) — F-27 am Schema

Am ZIEL gelesen: `dsp_bericht` (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2312-2345`)
trägt `"required": ["jcs", "auto_gain_db", "klemmungen", "verletzte_baender"]`
und `"additionalProperties": false` (`:2315-2316`) — ein neues Feld ist damit
eine Vertragsänderung. `klemmungen` (`:2330-2336`) hat `"minItems": 0`,
`"maxItems": 8` (`:2334`) und `"items": { "$ref": "#/$defs/dsp_klemmung" }`;
`baueBericht` kann diese acht Plätze bereits selbst belegen, weil es je aktivem
Slot mit `priority_sidechain` einen Eintrag schreibt
(`NakamaTransaktion.cpp:823-832`). Die Beschreibung von `dsp_klemmung`
(`:2300-2310`) bindet den Eintrag auf „EIN geklemmter Parameter des
bestaetigten Programms". **F-27 liest das Schema richtig:** der bestehende
v3-Vertrag trägt die zwei Zustände nicht ohne Änderung, und die Folge aus §37
(C++-Bericht plus Getter, Drahtfeld als Registerpunkt für S26–28) ist
zwingend. Ein Befund ist F-27 nicht.

---

## Befunde

### DEFEKT

**D-1 — M-103 (§39.1) und §40.3: der genannte „größte Bestandsfall auf dem
Rampenweg" ist keiner, und der Rotbeweis fällt an nichts.**
Reproduktion: M-103 sagt in der Zusage „Der größte Bestandsfall auf dem
Rampenweg ist `gain_db` −12 → 0 dB (Differenz 12,0 dB,
`eq-copilot/plugin/tests/DspGoldenTestMain.cpp:6145-6147`)"; §40.3 wiederholt
es („Der größte bestehende Rampenfall ist `gain_db` −12 → 0 dB … der
M-37-Spiegel"). Am ZIEL ist `:6142-6165` der Fall
`311/M-27 wiedereinschalten_nach_ruhe_wie_frischer_kern_auto_gain` des
Abschnitts P. Er läuft über den Helfer `ausUndEin` (`:5940-5964`): `ag`
(gain −12) wird publiziert, danach `ohneEq (ag)` — der Pfad geht in die Ruhe —,
danach die ENDE-Marke `ohneEq (flach)`, erst danach `flach` (gain 0). Zwischen
den zwei Gainwerten liegt die Ruhe: `rampenKompatibel` ist schon wegen
`eqEngagiert` falsch (`DspProgramm.cpp:212`), und am Blockrand ist `alt < 0`,
sodass `pAlt == nullptr` und nichts übertragen wird
(`DspKern.cpp:491-507`, `:518`). Der Fall nimmt nie den Rampenweg.
Folge: der Rotbeweis von M-103 („`kSprungGainDb` auf 10,0 gesetzt → der
Bestandsfall −12 → 0 dB verlässt den Rampenweg → `311/M-100` und der
M-37-Spiegel … weichen vom Basisstand ab → rot") wird nicht rot —
`311/M-100` ändert nur `freq_hz` und ist von `kSprungGainDb` unabhängig, und
der M-37-Spiegel vergleicht Prüf- und Frischkern, die beide über die Ruhe
laufen. Das Gainkriterium hat damit keinen fallenden Rotbeweis; M-103s eigener
Test bildet die drei Kanten symbolisch aus der Konstanten und bleibt bei jeder
Zahl grün. Gebrochen wird §39 („Der Rotbeweis fällt an der Zeile, die die
Zusage trägt, nie an einem Nebeneffekt") und §6 zur Etikettenpflicht.
Nebenbefund derselben Quelle: der größte wirkliche Rampenfall mit Gainwechsel
am ZIEL ist Abschnitt O, `basis` Bell 250 Hz Q 1,2 +5 dB → `rampe` −2 dB, also
**7,0 dB** (`DspGoldenTestMain.cpp:5677`, `:5686`, Kommentar „nur
kontinuierliche Werte: Rampenuebergang"); er fehlt in der Aufzählung „die drei
bekannten Rampenfälle (1,4; 3; 6,0 dB)". Alle liegen unter 18,0 dB, das
Verhalten bricht also nicht — der Beweis tut es.

**D-2 — §40.4 und §39.3: der neue Pegelzustand wird beim Abschalten des
Detektors nicht zurückgesetzt (E-29 / W-3).**
Reproduktion: `DspKern.cpp:737-745` trägt die gebaute Zusage „ein Detektor,
der nicht mehr läuft, hält keinen Zustand. Nach einer Rampe der Range auf 0
ist er damit wirklich aus, und eine spätere Rampe von 0 weg beginnt wie ein
frischer Detektor bei 0 statt bei einem Pegel von damals" und führt sie über
`z.huelle.nullen()` und `for (auto& d : z.detektor) d.nullen();` aus —
gliedweise, nicht über `BandZustand::nullen()`. §40.4 nennt als Änderung an
`DspKern.cpp` „das Feld im `SlotMerkmal`, `merke`, `vergebeKennungen` und
`verarbeiteBand` (`:664-687`, eine Zeile vor `z.huelle.tick`)" und für
`DspBankPool.h` „der Pegelzustand im `BandZustand` samt `nullen`, `istEndlich`
und `riegleDenormale`" (`DspBankPool.h:58-78`). Die Stelle `:741-745` kommt in
keinem Satz vor, und `BandZustand::nullen()` wird dort nicht gerufen. Wird der
Bauplan wörtlich ausgeführt, behält der Pegelpol nach einer Range-Rampe auf 0
seinen alten Wert; bei der nächsten Rampe von 0 weg bekommt die frisch
genullte Hüllkurve als ersten Eingang den Pegel „von damals" und springt mit
der Attack darauf — genau das, was E-29 ausschließt. Keine Matrixzeile misst
es: M-130 (a) deckt nur Heilung und Denormalriegel am Blockrand
(`DspKern.cpp:560-578`, `DspFilter.h:299-303`), M-131 nur die topologische
Übertragung, M-123 nur die Bitgleichheit des Begriffs `spitze`. Der bestehende
Test `range_minus_12_nach_0_rampt_ueber_die_volle_rampe` liest ausschließlich
`huelle.leistung` und fällt deshalb nicht. Gebrochen wird eine weiter geltende
Zusage (E-29 / W-3), die der Nachtrag nicht als verschoben benennt.

**D-3 — M-127 (§39.3) und §40.4: die Absolutzahl der Sprungantwort ist falsch.**
Reproduktion: M-127 sagt „Die Absolutzahlen wachsen: bei Attack 20 ms und
`kPegelFensterMs` 10 ms liegt der 63-%-Punkt der Reihe bei rund 27 ms statt
20 ms"; die Verschiebungstabelle in §40.4 wiederholt „bei Attack 20 ms und
Fenster 10 ms liegt der 63-%-Punkt bei rund 27 ms". Ich habe den Prüfstand
`attack_hold_release_als_sprungantwort_bei_vier_raten`
(`DspGoldenTestMain.cpp:2375-2435`: Attack 20 ms, Hold 30 ms, Release 50 ms,
Threshold −30 dB, `attackSchwelle` aus 1 − 1/e) mit den Rekursionen des Codes
(`huellkurvePol`, `DspFilter.h:273-278`; `HuellkurveZustand::tick`, `:312-328`)
bei 48 kHz nachgefahren: heute A 20,00 / H 30,17 / R 49,83 ms, mit dem
10-ms-Pegelpol davor A **31,83** / H 30,17 / R 50,50 ms. Der 63-%-Punkt der
Reihe liegt bei 31,8 ms, nicht bei rund 27 ms; der Zuwachs ist 11,8 ms statt
etwa 7 ms. Dieselbe Rechnung analytisch: 31,7 ms. Der Satz derselben Zusage
„Hold und Release messen vom Plateau aus und wachsen um weniger als 1 ms" ist
dagegen richtig (+0,00 und +0,67 ms). Gebrochen wird die Herleitungspflicht
aus dem Prüfauftrag („Ein Befund ist es, wenn die Herleitung rechnerisch
falsch ist") und die Zusagenspalte von M-127, die die Größe der Verschiebung
als Zahl behauptet.

**D-4 — §40.2 Nr. 3: die Abstandsrechnung zur Deckelhöhe stimmt nicht.**
Reproduktion: „24 dB … liegt mit Faktor 3,4 über der +7,0 dB aus (1) und mit
Faktor 15 unter der kleinsten gemessenen Gefahr aus (2)." Der erste Faktor
stimmt (24 / 6,975 = 3,44). Die kleinste gemessene Gefahr aus (2) ist
+42,99 dB. Nachgerechnet: als dB-Verhältnis 42,99 / 24 = **1,79**, als linearer
Abstand 10^(42,99/20) / 10^(24/20) = 141,1 / 15,849 = **8,90**, als
Leistungsverhältnis 79,3. Keine Lesart ergibt 15; die einzige 15 in der Nähe
ist der lineare Faktor des Deckels selbst (15,849, in M-112 korrekt als
„10^(24/20) = 15,849-mal" genannt). Die übrigen Zahlen der Herleitung habe ich
bestätigt: Low-Cut 80 Hz entfernt 20,1 % → +0,973 dB; Low-Cut 500 Hz 46,6 % →
+2,724 dB; High-Cut 2 kHz 33,3 % → +1,761 dB; beide zusammen lassen 20,07 %
stehen → +6,975 dB; `v1.global.output_trim_db` reicht bis +24,0 dB
(`eq-copilot/schemas/state/nakama-parameter-v2.json:52-58`). Das Maß der Regel
(„über jeder Anhebung normaler Arbeit und unter dem, was Wiedergabegeräte
gefährdet") hält mit dem berichtigten Faktor; die Zahl der Herleitung tut es
nicht.

**D-5 — M-99 und M-97 (§39.1): das Etikett „heute nicht messbar" trifft am
ZIEL nicht zu.**
Reproduktion: §6 bindet das Etikett auf „der Mechanismus oder das Urteil
entsteht erst im Bau". M-99 (Gegenfall dynamisches Band mit Range 0, derselbe
Sprung 5000 → 50 Hz) ist am ZIEL vollständig messbar: der SVF-Weg besteht,
`detektorLaeuft` ist bei Range 0 falsch (`DspProgramm.cpp:318`), `nutztSvf`
wahr, und die Übergangsspitze über 0,5 s lässt sich mit demselben Aufbau
erheben, aus dem M-96 und M-98 ihre +17,41 und +22,20 dB haben. Die Zeile
sagt selbst, der Fall „kann denselben oder einen größeren Ausschlag tragen",
sieht aber keine Messung am Basisstand vor — obwohl der Auditwortlaut ihn
ausdrücklich als Gegenweg fordert
(`docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md:810`: „dynamisch Range 0 und
20 kleine Schritte als Gegenwege"). M-97 (Gegenrichtung 50 → 5000 Hz) trägt
dasselbe Etikett und widerlegt es im eigenen Text: „Der Fall läuft nach §7.1
vor der Reparatur einmal gegen den unveränderten Kern, und die Rohdatei hält
fest, ob er dort fällt." Beide Zeilen sind nicht unmessbar, sondern
ungemessen. Gebrochen wird die Etikettendefinition in §6 und damit die
Zählung in §39.6 („heute nicht messbar 19").

### LÜCKE

**L-1 — R-311-15 ist nur bei hoher Bandmitte gemessen.** Der erste Satz der
Regel („derselbe Threshold wirkt bei jeder Hold- und Attack-Einstellung nach
demselben Begriff") trägt keine Frequenzeinschränkung; M-124 misst die neun
Hold-/Attack-Einstellungen nur bei f0 = 1 kHz, wo die Restwelligkeit höchstens
0,034 dB gleichrichtbaren Versatz zulässt und die Schranke 0,1 dB damit nicht
klemmt. Bei 20 Hz bleibt ein Rest von 1,45 dB — dieselbe Größenordnung wie der
heutige Befund (3,01 dB). F-18 schränkt ausdrücklich nur das **Materialmaß**
von M-120 je Bandfrequenz ein, nicht das Hold-/Attack-Maß von M-124; §41.5
Nr. 2 ist vom Dirigenten als Registerzeile [Härtung] eingeordnet. Der Dirigent
entscheidet, ob M-124 wie M-120 eine frequenzabhängige Schranke braucht.

**L-2 — M-128 nennt zwei Prüfungen unter einer Begründung, die für sie nicht
gilt.** M-128 zählt „die Plateauprüfung, die Selektivitätsprüfung" unter
„alle bestehenden Prüfungen des Abschnitts F von B6, deren Material ein
Quadraturton auf der Bandmitte im eingeschwungenen Zustand ist". Am ZIEL
fahren `detektor_ist_bandbegrenzt (M-21)` und `detektor_hoert_vor_dem_band
(M-21)` einen **L = R-Sinus** (`DspGoldenTestMain.cpp:2210-2221`), einer davon
bei 60 Hz weit neben der Bandmitte — genau das Material, dessen gemessener
Pegel sich verschiebt. Ihre Zahlen verschieben sich trotzdem nicht, weil beide
im Plateau beziehungsweise gegen eine lose Verhältnisschranke messen; die
Begründung trägt sie aber nicht. Ebenfalls in keiner der beiden Listen:
`range_null_rechnet_keinen_detektor (M-20)`,
`huellkurvenwert_<feld>_holdzaehler_rundet_exakt (NAK-289)` und
`range_..._rampt_ueber_die_volle_rampe (M-17, R8, B-4, W-3)`; auch sie
verschieben nichts (kein Detektor, durchweg steigender Eingang, Plateau).

**L-3 — die Bedeutung des Drahtfelds `auto_gain_db` ändert sich, ohne dass
eine Zeile es sagt.** Der Deckel sitzt in `leiteAutoGainAb`, also vor
`baueBericht` (`NakamaTransaktion.cpp:817-822`); M-117 verlangt ausdrücklich,
dass der Bericht `kAutoGainDeckelDb` trägt. Damit führt das v3-Feld
`auto_gain_db` auf der Anhebungsseite künftig den **gedeckelten** statt des
abgeleiteten Werts, während sein `$comment`
(`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2324-2329`) sagt „ABGELEITET …
die Ableitung selbst kann darueber liegen" und nur die Berichtsgrenze ±120
nennt. §38.2 führt diese Zeilen als „unverändert", und M-140 verlangt einen
leeren Diff über `eq-copilot/schemas`. Die Etappe 4 hat genau eine
`$comment`-Zeile derselben Stelle geändert (Kopf, Ticketpfade), eine
`$comment`-Änderung ist also keine Vertragsänderung. Der Dirigent entscheidet,
ob der `$comment` den einseitigen Deckel nennen muss und ob M-140 dafür eine
Ausnahme bekommt.

**L-4 — was im neuen, dritten Fenster mit Kanalzahl und Ausgleichsmeldung
geschieht, ist nicht entschieden.** §40.1 Nr. 2 lässt `bereiteVor` bei nicht
unterstützter Rate „denselben Weg wie `freigeben`" nehmen und zählt auf
„Puffer leeren, `abtastrate` auf 0, Bänke zurücksetzen, Pfade in Ruhe, Zähler
zurück". `freigeben` (`DspKern.cpp:127-154`) setzt zusätzlich `kanalzahl = 2`
und setzt `autoGainBericht` **nicht** zurück, während `bereiteVor` es heute bei
`:120` tut und M-132 „`DspKern::autoGainDb()` ist exakt +0,0" verlangt; die
Aufzählung nennt beides nicht. §40.1 Nr. 3 übernimmt zugleich die Kanalzahl im
Transaktionskern und beruft sich dafür auf R-311-12, dessen Satz „Die
Kanalzahl folgt auf jedem Weg der Samplerate" das Gegenteil sagt. Hörbar wird
nichts, weil in diesem Fenster kein Block klingt; eine Zeile misst es nicht.

### HÄRTUNG

**H-1 — M-132 zeigt für die Gitterauswertung auf eine Kommentarzeile.** Die
Gegenprobe sagt „`eq-copilot/plugin/dsp/DspProgramm.cpp:99-102` wertet das
Gitter ohne Nyquistprüfung aus". Am ZIEL sind `:99-102` vier Kommentarzeilen
des Monozweigs; das Gitter wird bei `:118-120` (Mono) und `:166-171`
(Zweikanal) gelaufen, `autoGainGitterHz` steht bei `:76-83`. Die Angabe stammt
unverändert aus der Quellkette des Audits, deren Stand vor dem Monozweig lag.
Die Aussage selbst ist richtig — es gibt keine Nyquistprüfung —, und §38 sagt
zu, dass alle Zeilenangaben am Stand `8408d782` gelten.

**H-2 — §40.2 lässt `311/M-116` in „Tests und Beine" aus.** §39.2 nennt für
M-116 „B6 **NEU** `311/M-116 deckelzustand_faellt_zurueck`"; die Aufzählung des
Änderungssatzes B nennt nur `311/M-112`, `311/M-113`, `311/M-115` und die
Teilfälle zu `311/M-114`, `311/M-118`, `311/M-119`. Der Rotbeweisabsatz
desselben Satzes führt M-116 wieder. Das Bein (B6) stimmt in beiden Fassungen.

**H-3 — der Zeilenvergleich des Abschnitts F kann in der letzten Stelle
wandern.** M-128 sagt „Kein Zahlwert dieser Prüfungen verschiebt sich" und
belegt es mit einem Zeilenvergleich vorher/nachher nach dem Muster von M-82
und M-83, also einem bitgenauen Vergleich. Der Fixpunkt ist bitgenau, die
**Iteration** dorthin bleibt in IEEE-754 rund 3·10⁻¹⁴ relativ darunter stehen
(selbst gemessen für 10 ms bei 44,1 / 48 / 192 kHz), also etwa 10⁻¹³ dB. Für
jede Schranke des Abschnitts F ist das bedeutungslos; für einen bitgenauen
Zeilenvergleich ausgeschriebener Zahlen ist es das nicht. Eine Rundungsangabe
für diesen Vergleich würde die Zusage tragfähig machen.

**H-4 — M-110 ist als Regressionswache etikettiert, obwohl sein Reiz heute
keinen Crossfade erzeugt.** Der Prüfling ist „der Sprung aus M-96", der am ZIEL
den Rampenweg nimmt; die gewachte Eigenschaft (`z.rest` in Samples,
`DspKern.cpp:547`) und die Mutation greifen erst nach dem Bau. §40.3 ordnet
M-110 folgerichtig unter die Regressionswachen, deren Mutation nach dem Bau
gefahren wird — das Etikett ist vertretbar, seine Bedeutung aber eine andere
als bei M-100 oder M-105, die heute mit ihrem eigenen Reiz grün laufen.

---

## Geprüft

Selbst am ZIEL nachgemessen (lesend beziehungsweise nachgerechnet): die
Kennungsvergabe und der Blockrand in `eq-copilot/plugin/dsp/DspKern.cpp`
(`:97-125` `bereiteVor`, `:127-154` `freigeben`, `:220-226` `meldeProgramm`,
`:238-259` `merke`, `:261-292` `vergebeKennungen` mit `:273-274` und
`:286-288`, `:347` in `publiziereVorbau`, `:365-374` `beendeCandidate`,
`:451-557` `blockrand` mit `:460`, `:495-507`, `:518-541`, `:547`, `:560-578`
`heileZustaende`, `:664-687` und `:690-710` und `:737-745` in `verarbeiteBand`,
`:1011-1013` `verarbeiteStueck`, `:1087-1093` Eingangsriegel); `DspKern.h`
(`SlotMerkmal` und `Merkzettel`); `DspProgramm.cpp` (`:76-83`
`autoGainGitterHz`, `:85-204` `leiteAutoGainAb` mit `:96`, `:161-163`,
`:201-203`, `:207-227` `rampenKompatibel`, `:230-340` `baueProgramm` mit
`:237-240`, `:286-288`, `:290-300`, `:318`, `:326-333`, `:339-340`);
`DspProgramm.h` (`:62`, `:66`, `:90`, `:95-97`); `DspFilter.h` (`:266-271`,
`:273-278`, `:280-288`, `:290-329`, `:334-341`, `:355-361`); `DspBankPool.h`
(`:58-78`); `NakamaTransaktion.cpp` (`:777-789`, `:792-842`) und `.h`
(`:273-275`, `:387-396`, `:398-427`); `SondeProcessor.cpp` (`:232-280`,
`:282-296`); `eq-ipc-v3.schema.json` (`:2300-2345`);
`nakama-parameter-v2.json` (`:52-58`, `:155-191`, `:233-282`);
`tools/beweise.ps1` (`:688`, `:702`, `:723`, `:728`, einschließlich der
H-4-Stelle „zwei bis acht Baender"). In den Testdateien: `DspGoldenTestMain.cpp`
Abschnitt E (`:1937-2020`), Abschnitt F (`:2020-2620` vollständig auf
Prüfnamen und Material), Abschnitt J (`:3990-4030`), Abschnitt O
(`:5650-5820`), Abschnitt P (`:5920-6200`), Abschnitt Q (`:6460-6940`) und
`:1873-1878`; `SondeNullTestMain.cpp` (`:195-210`, `:600-700`, `:1125-1145`);
`TransactionTestMain.cpp` (`:565-590`, `:885-915`, `:1004-1020`, `:1385-1400`,
`:2240-2275`) sowie ein `grep` über alle vier Dateien nach jeder Zuweisung an
`kFreqHz`, `kQ` und `kGainDb` und nach jedem `machDynamisch` /
`dynamic_enabled`. Selbst gerechnet: Restwelligkeit und Versatz bei 20 / 100 /
341 / 1000 Hz, Rauschbandbreite und σ, die 171-ms-Alternative, der
IEEE-754-Fixpunkt des Ein-Pol-Mittels, die Gitterstellen über Nyquist bei
32 000 / 22 050 / 44 100 Hz, die vollständige Deckelherleitung aus §40.2, die
vierzehn Hexwerte gegen +24 dB, die Wertverhältnisse aller Bestandsfälle aus
E, P, Q, M-94 und dem M-37-Spiegel sowie die Sprungantwort A/H/R einmal
analytisch und einmal als diskrete Simulation der Coderekursionen bei 48 kHz.
Gelesen als Gate-Bezug: Kopf, §37, §42, §6 (Etiketten und Regeln), §7.0, §7.1,
§36 (R-311-12, H-4), die fünf Abnahmen `design/abnahmen/2026-09-19-*.md`
wörtlich, `docs/plan/plan.json` (Feld `text` zu S25j),
`docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (`:632`, `:633`, `:810`),
`roh/phase-16-d-dsp.md` (`:24`, `:57`, Quellkette),
`docs/beweise/roh/NAK-311-quellvalidierung-teil-B.md` (§1.1, §1.4, §1.5, §4.3,
§4.4, §4.5, §7.3, §7.4, §7.5), `docs/beweise/roh/NAK-311-etappe4-autogain-hex.txt`
und `docs/beweise/SONDE-015.md` an allen achtzehn zitierten Zeilen (`:1048`,
`:1051`, `:1061`, `:1062`, `:1069`, `:1070`, `:1213`, `:1603`, `:1620`,
`:1696`, `:1711`, `:2951`, `:3397`, `:3400`, `:3410`, `:3412`, `:3419`,
`:3425`) — jede Fundstelle trägt den zitierten Satz.

## Nicht geprüft

Nichts gebaut und nichts gefahren: jede Aussage über „rot" und „grün" ist aus
der Quelle geschlossen, nicht aus einem Lauf; die sieben „heute rot"-Zeilen
habe ich am Code des ZIEL auf ihren Mechanismus geprüft, nicht gemessen.
`docs/audits/2026-09-15-tiefenaudit/roh/phase-16-f-gegenpruefung-d.md` und
`roh/phase-16-abdeckung.md` habe ich nicht gelesen; die Zahlen daraus habe ich
über `BEFUNDE.md` und `phase-16-d-dsp.md` gegengeprüft. Von der
Quellvalidierung Teil B habe ich die Abschnitte 2 und 5 nur über ihre in
`BEFUNDE.md` (Zeile T3-15-09) wiederholten Zahlen abgeglichen, nicht im
Volltext. Die Abgleichung der Bestandsfälle gegen die drei Wertekriterien habe
ich für die vom Auftrag genannten Bereiche vollständig geführt (M-40 bis M-56,
M-94, M-95, Abschnitte E, P, Q) und darüber hinaus über den `grep` nach
`kFreqHz`/`kQ`/`kGainDb` in allen vier Testdateien; die Treffer in
`TransactionTestMain.cpp` (`:577`, `:896`, `:1009`, `:1013`, `:1313-1316`,
`:1396`, `:1881`, `:2247`) habe ich nur bis zu ihrer Prüfart verfolgt
(Transaktions- und Zustandsfälle ohne Audiovergleich, dazu `freq_hz`
1000 → 500 mit dem Verhältnis genau 2,0, das nach M-101 auf dem Rampenweg
bleibt), nicht bis zu jeder einzelnen Behauptung — §40.3 fängt diesen Rest mit
Risiko 1, dem Zeilenvergleich und dem `git grep` ab. Ausgeschlossen und
deshalb nicht bewertet: §41.3, §41.4, §41.5, die Einordnungen in §42, die
Wahl der fünf Zahlen als solche, Prosa und Zählstände außerhalb von §38 bis
§41, `tools/dirigent/pruefliste.md`, die im Register datierten Härtungen und
die Sicherheitsbefunde T3-12-*.

---

HEAD zu Beginn: `c600c3c2911843516634dee19b6db8736ba8d01b`.
HEAD am Ende: `c600c3c2911843516634dee19b6db8736ba8d01b`.
