URTEIL: NEEDS_WORK — zwei DEFEKTE (D-1 an der Matrixzeile 311/M-133, D-2 an den Zusagetexten von 311/M-73 bis 311/M-75), dazu zwei Lücken und fünf Härtungen.

# NAK-311 Erstprüfung 5 (Planschritt S25j, Etappe 5) — Rohbericht

Prüfer: frischer, lesender Opus-Thread, Effort `max`, `dontAsk`.
Prüfbereich: `git diff 5ee8318c...1183ad9f` über die 18 im Auftrag genannten
Pfade. Prüfstufe T2. Datum 20.09.2026.

HEAD zu Beginn: `4ffdc70a4460ae5e3c151589291379575e77c7ca`.

Gefahren (nur lesend, nacheinander, kein Bau; vorher geprüft, dass kein Bau-
oder Testprozess läuft — fünf `pwsh`-Prozesse ohne Kindprozess, kein `cmake`,
kein `MSBuild`, kein `vctip`):

- `EqCopDspGoldenTest.exe` — Exit 0, 389 geprüft, 0 Fehler, `DSP-GOLDEN OK`
- `EqCopTransactionTest.exe` — Exit 0, 192 geprüft, 0 Fehler, `TRANSAKTION OK`
- `EqCopSchemaTest.exe` — Exit 0, 162 bestanden, 0 gescheitert
- `EqCopProbeeqNullTest.exe` — Exit 0, 131 Prüfungen ok, 0 Fehler

Dieselben Zahlen wie im Auftrag und in §60.10 / §61. Binary-Zeitstempel B6
16:19:50, B7 16:40:17, B3c 16:20:24, A16 16:39:43 liegen nach der jüngsten
Quelle (`DspKern.cpp` 16:19:48); die vier Läufe enthalten die Prüfnamen, die
erst dieser Änderungssatz anlegt (311/M-96 bis 311/M-144), das Binary
entspricht also dem ZIEL-Quellstand.

---

## (a) Satz A — R-311-16 und R-311-20, unter 44,1 kHz neutral

**Das Prädikat steht genau einmal.** `samplerateUnterstuetzt`
(`eq-copilot/plugin/dsp/DspProgramm.h:113-119`) ist die einzige Stelle mit der
Vergleichsrichtung; `kMinSamplerateHz = 44100.0` steht in `:101`. Eine
Volltextsuche über `eq-copilot/plugin`, `eq-copilot/schemas` und `broker`
findet außerhalb der Tests keine zweite Zahl 44100 im DSP-, Sonden- oder
State-Pfad (`NakamaState.cpp:1575` nennt 44100,5 in einem Kommentar über
Projektsamples, `NakamaKernRiegel.h:18` gar nicht). Die drei geforderten Leser
lesen es: `DspKern::bereiteVor` (`DspKern.cpp:108`),
`Transaktionskern::setzeSamplerate` (`NakamaTransaktion.cpp:360`) und
`SondeProcessor::prepareToPlay` (`SondeProcessor.cpp:282`). Gemessen in
311/M-134: 44 100 minus 1 ULP und 44 099 werden abgelehnt, 44 100 und der
nächste double darüber angenommen, NaN und ±Inf abgelehnt — am Prädikat und am
echten Prozessor.

**Der Pfad ist sampleidentischer Passthrough auf jedem Weg.** `bereiteVor`
prüft die Rate als Erstes, vor jeder Pufferzuweisung, nimmt den Weg von
`freigeben` (`DspKern.cpp:108-129`) und kehrt zurück. `freigeben`
(`:164-195`) leert `dryL` und alle übrigen Puffer, setzt `maxBlockGroesse = 0`
und `abtastrate = 0.0`. `verarbeiteStueck` kehrt daraufhin in seiner ersten
Zeile zurück (`:1102`, `if ((size_t) numSamples > dryL.size()) { letzteKanaele
= numKanaele; return; }`) — vor dem ersten Sample, ohne float-nach-double-
Umweg. Der Stückel-Zweig von `verarbeite` (`:1078`) ist durch
`maxBlockGroesse > 0` geschützt, `zaehlerVerworfen` steigt also nicht.
Gemessen in 311/M-132 über 460 000 Samples je Fall bei 32 und 22,05 kHz, in
Stereo und Mono: Rauschen 0 abweichend, Bitmuster 0 von 13 352 (Mono 6 676),
6 nicht endliche Werte unverändert, drei Wachmarken bytegleich, Zähler 0/0/0;
in 311/M-139 zusätzlich Blockgrößen 1, 255, 256, 257, 2048 und 4096 (über
maxBlock) bytegleich, Taplänge 0, Latenz 0, Tail 0,0 s, im Prozessor 0
Allokationen im Callback über 200 Blöcke und 0 gemeldete Sperren.

**Melder, Zähler und Deckelzustand sind genullt.** `DspKern.cpp:119-122` nullt
`autoGainBericht`, `autoGainRohBericht`, `autoGainGedeckeltBericht` und ruft
`zaehlerZuruecksetzen()`. Gemessen in 311/M-143: im 32-kHz-Fenster Kern +0,0
(Vorzeichenbit 0) und Bericht +0,0, `klemmungen` leer, Übernahmen 1 auf 0,
`busKanaele` 2; nach dem Rückweg tragen beide wieder bitgleich 15,688900344 dB.

**Kein erreichbarer Weg mit veraltetem Zustand.** Ich habe alle
Publikationswege gelesen: `prepareToPlay` publiziert nur bei unterstützter Rate
(`SondeProcessor.cpp:282-291`), der Ladestart nur bei
`dspKern->samplerate() > 0.0` (`:1096`), der Workertakt ebenso (`:1476`),
`DspKernAusfuehrung::publiziereWirksam` (`NakamaTransaktion.cpp:768`) und
`publizierePreview` (`:755`) haben denselben Riegel, und die Zweihälftenform
des Commits (`baueVor` / `publiziereVorbau`, `:730-731` und `:740`) setzt
`vorbauGebaut = kern.samplerate() > 0.0`. Im abgelehnten Fenster ist
`abtastrate` gleich 0,0; damit erreicht **keine** Publikation den Kern, und
`blockrand` findet nichts vor. `freigeben` nimmt `abgelehnteRate` zurück
(`DspKern.cpp:181`), der unterstützte Zweig ebenso (`:131`). Gemessen:
311/M-135 (48 → 32 → 48 kHz, Ausgang im Fenster bytegleich, danach bitgleich
zu einem nie abgelehnten Kern, eq_enabled, Statebytes 4704, Revision und
Host-Dirty unverändert), 311/M-132 Teilfälle (zweimal dieselbe Rate, andere
Rate, maxBlock 0, `releaseResources`), 311/M-136 (verriegeltes Fenster: 0,
NaN, ±Inf, 1e300 und maxBlock 0 lassen die Vorbereitung stehen und melden
keine Ablehnung), 311/M-137 (v3Samplerate, v3BlockSize, v3Channels tragen
weiter die Hostwerte).

**44,1 kHz und der Weg auf den Draht.** 311/M-133 misst bei 44,1 / 48 / 96 /
192 kHz, dass der Kern vorbereitet, das Band hörbar und `abgelehnteSamplerateHz`
exakt +0,0 ist. Der **zweite Teil** der Zeile — „der Ausgang ist bitgleich zum
Lauf am Basis-SHA der Etappe (memcmp)" — wird von keinem Riegel gemessen:
siehe **DEFEKT D-1**. Die abgelehnte Rate erreicht weder State noch Draht noch
`state_hash`: `abgelehnteRate` ist ein privates Feld neben `fs`
(`NakamaTransaktion.h:334`), `baueBericht` setzt `aus.jcs` und den Hash vor dem
Programmzweig (`NakamaTransaktion.cpp:816-829`), und 311/M-138 (b) misst, dass
`$defs/dsp_bericht` mit `additionalProperties: false` ein Dokument **mit** dem
Feld abweist (1 Verletzung an `/dsp/abgelehnte_samplerate_hz`) und ohne es
gültig ist. Ein Sender für `state_report.dsp` existiert im Plugin noch nicht
(`SondeProcessor.h:220-221`, „der Sender ist 4b").

## (b) Satz B — R-311-14 und R-311-19, Obergrenze des AUTO-Ausgleichs

**Einseitig, an beiden Rückgabezeilen.** `gedeckelterAutoGainDb`
(`DspProgramm.h:463-466`) ist `roh > kAutoGainDeckelDb ? kAutoGainDeckelDb :
roh` — strikt größer, keine Multiplikation, kein Runden, kein `std::clamp`.
Sie steht an der Monozeile (`DspProgramm.cpp:192`) und an der Zweikanalzeile
(`:203`); die fünf Kurzschluss- und Wachwege geben 0,0 zurück und sind damit
unter dem Deckel. `leiteAutoGainAb` belegt `ungedeckeltAus` auf **jedem**
Rückweg (Vorbelegung `:180`, die zwei rechnenden Wege `:191` und `:202`).
Gemessen: 311/M-115 (Mono- und Zweikanalzweig decken an ihrer eigenen Zeile),
311/M-112 (acht High-Cuts 20 Hz Q 0,15, abgeleitet +150,464333526 dB,
angewandt bitgenau 24,0, Tapfaktor 15,848931925 statt 3,3e7).

**Programm, Kern und Bericht lesen dieselbe Zahl.** `baueProgramm` schreibt
den angewandten Wert nach `autoGainDb`, den ungedeckelten nach `autoGainRohDb`
und leitet `autoGainLin` vom angewandten ab (`DspProgramm.cpp:254-257`).
`DspKern::meldeProgramm` belegt alle drei Melder am selben Ort, vom selben
Schreiber, mit derselben Speicherordnung, aus **einem** Programm
(`DspKern.cpp:263-270`); die Bedingung steht einmal in
`DspProgramm::autoGainGedeckelt()` (`DspProgramm.h:531-534`) und wird von Kern
und `baueBericht` (`NakamaTransaktion.cpp:850`) gelesen. Gemessen in 311/M-117
über B3c und über `SondeProcessor::dspBericht` (B7): abgeleitet
150,464333526 dB, Kern angewandt 24,000000000000000 dB, Bericht
24,000000000000000 dB, `autoGainLin` 15,848931924611, Zustand in beiden
gesetzt.

**Absenkungsseite, Zustandslogik, Rückfall, Draht.** 311/M-114: acht
Low-Shelves 1 kHz +12 dB Q 8 bleiben mit −199,767783231 dB bitgleich, der
Bericht klemmt wie seit R-311-5 auf exakt −120, Zustand in beiden falsch,
`klemmungen` leer. 311/M-118: mit ausgeschaltetem Auto-Gain ist der Zustand in
Kern und Bericht falsch. 311/M-116: Q 0,15 gesetzt, Q 0,5 gesetzt, Q 1,0
(19,082282924 dB) falsch und bitgleich; der angewandte Faktor läuft über
`kRampeSamples` (erstes Sample 15,822167895, nach 300 Samples 8,997340287),
springt also nicht. 311/M-117 misst zusätzlich, dass derselbe Bericht **mit**
dem Feld im dsp-Dokument abgewiesen wird (1 Verletzung an
`/dsp/auto_gain_gedeckelt`). `baueBericht` setzt `aus.autoGainGedeckelt =
false` vor dem Programmzweig (`NakamaTransaktion.cpp:821`), der Zustand fällt
also mit dem Wert zurück.

**Zahlenränder.** 311/M-113 (a): −1 ULP 23,99999999999999645 kommt bitgleich
zurück, genau 24,0 bitgleich, +1 ULP 24,00000000000000355 auf die Grenze; −0,0,
+0,0 und das kleinste Subnormal bitgleich. (b): ohne aktives Band bleibt der
Kurzschluss, beide Vorzeichenbits 0. (c): die Ableitung überspringt die Grenze
(0 exakte Treffer in 800 benachbarten doubles), Prädikat an den drei Kanten
001 mit und 000 ohne Auto-Gain. (d): acht High-Cuts beidseits der Grenze bei
Q 0,931076192540312. NaN und ±Inf erreichen die Deckelfunktion nicht — der
Kurzschluss und die zwei Wachen von `leiteAutoGainAb` (`DspProgramm.cpp:187`,
`:198`) stehen davor.

**Verträge und Fixtures.** `git diff --stat 5ee8318c...1183ad9f` zeigt über
`eq-copilot/schemas` genau eine geänderte Zeile — den `$comment` von
`auto_gain_db` in `eq-copilot/schemas/v3/eq-ipc-v3.schema.json` (1 Einfügung,
1 Löschung) — und über `eq-copilot/fixtures`, `eq-copilot/schemas/state`,
`eq-copilot/identity` und `broker/` **nichts**. `minimum: -120`,
`maximum: 120`, Discriminator, Zieladresse, Revision und Capability sind
unberührt.

## (c) Satz C — R-311-13, große Sprünge werden überblendet

**Die drei Wertekriterien in der bleibt-Bedingung.** `DspKern.cpp:351-356`:
die fünf Topologiefelder, dazu `s.pegelbegriff == b.pegelbegriff` und die drei
Vergleiche `! sprungImVerhaeltnis (s.freqHzWirksam, b.freqHzWirksam,
kSprungFrequenzVerhaeltnis)`, `… (s.q, b.q, kSprungGueteVerhaeltnis)` und
`! sprungInDb (s.gainDb, b.gainDb, kSprungGainDb)`. Verglichen wird gegen den
Merkzettel **desselben** Pfades (`merkzettel[(size_t) p]`, `:324`), der die
drei Werte in `merke` mitführt (`:310-312`). `freqHzWirksam` ist der Wert nach
der Nyquistkappung (`DspProgramm.cpp:302`, `kappeNyquist`), also derselbe, den
`entwurf` und `entwurfBandpass` benutzen. Strikt größer:
`sprungImVerhaeltnis` rechnet `gross > grenze * klein` mit zwei exakten
Zweierpotenzen (`DspProgramm.h:405-414`), `sprungInDb` `|alt − neu| > grenze`
(`:419-427`). Die NaN-Falle ist positiv geschlossen: `brauchbar` verlangt
`> 0.0` und `< inf`, und `if (! brauchbar) return true` heißt „gerissen" —
der sichere Ausgang ist der kalte Start, nicht die Übertragung. Gemessen:
311/M-101 mit 18 Kanten (je Kriterium −1 ULP und genau auf der Grenze →
Rampe, +1 ULP → Crossfade, in beiden Richtungen), 0 gegen die Erwartung;
311/M-109 (freq NaN, q NaN/+Inf/0/negativ, gain NaN/±Inf erzwingen je eine
neue Kennung).

**Nur der gesprungene Slot.** `b.lebenszyklus = ! b.aktiv ? 0 : (bleibt ?
s.kennung : naechsteKennung())` (`:357`) arbeitet je Slot. Am Blockrand
übernimmt der Crossfade den `BandZustand` genau der Slots mit gleicher Kennung
(`:601-607`), alles andere bleibt kalt. Gemessen: 311/M-104 (Slot 3 3 → 4,
alle anderen unverändert, Slot 0 warm mit Perioden-RMS 0,004402 dB gegen
0,5 dB), 311/M-108 (acht Slots zugleich, genau ein Crossfade, Übernahmen 1,
Übergangsspitze −0,2196 dB gegen 1 dB).

**`blockrand` ist textgleich.** Der Diff über `DspKern.cpp` berührt die
Funktion (`:520-627`) nicht; ich habe sie ganz gelesen. Die Kennungen werden
dort nur gelesen, es gibt keinen zweiten Wertevergleich.

**Unter und genau auf dem Kriterium bleibt alles bitgleich.** 311/M-100:
zwanzig Stufen zu höchstens 1,258925 (Kriterium 2,0), Übernahmen 20,
Crossfade-Blöcke 0, Kennung Slot 0 unverändert, **Fingerabdruck des Ausgangs
über 40 960 Samples 13210420415127086859 gegen Basisstand
13210420415127086859** (hier ist der Bitvergleich als Konstante im Test — das
Muster, das 311/M-133 fehlt). Die zwei 18,0-dB-Fälle liegen mit 2,0 dB Abstand
unter `kSprungGainDb`; sie werden über die unveränderten Bestandsprüfungen
`bandwert_gain_db_rampt_ohne_zustandsreset` und
`bandwert_q_rampt_ohne_zustandsreset` (Matrixzeilen 311/M-142 a und b) geführt
— dort ohne eingebauten Basisvergleich, siehe **L-1**.

**Verdrängte Zwischenpublikation, Candidate, Hörhalt, zweiter Sprung.**
311/M-106 (Automationstreppe, Vergleich gegen die zuletzt **publizierte**
Belegung statt gegen die genommene), 311/M-107 (Ruhe, Hard-Bypass, Candidate;
ENDE-Marke leert den Merkzettel, `DspKern.cpp:395` und `:441`), 311/M-105
(zweiter Sprung im laufenden Crossfade: Übernahmen genau 2, die zweite 256
Samples nach der ersten, Übergangsspitze −0,0096 dB), 311/M-110
(Umschaltsample und Fadedauer bei Blockgröße 1, 255, 256, 257, 4096).
Nach `bereiteVor` und `freigeben` setzen `baenke.zuruecksetzen()` und
`pfade[].ruhe()` den Audiozustand zurück; der Merkzettel überlebt, aber
`pfadGleich` reißt über `m.samplerate == prog.samplerate` (`:328-330`), sodass
jeder Slot kalt startet.

**`vergebeKennungen` läuft nie im Audio-Thread.** Einziger Aufrufer ist
`publiziereVorbau` (`DspKern.cpp:416`), dessen einzige Aufrufer
`uebernehmeZustand` (`:368`) und `DspKernAusfuehrung::publiziereVorbau`
(`NakamaTransaktion.cpp:741`) sind — beide außerhalb des Callbacks, unter dem
Zustandsschloss. Gemessen: 311/M-111 (Rotbeweis mit den drei Vergleichen
zusätzlich am Blockrand, Diffprüfung JA) und
`null_allokationen_im_callback_samt_programmwechseln` (4000 Blöcke, 121
Blockrand-Übernahmen, Testzähler 0, Kernzähler 0).

## (d) Satz D — R-311-15, festgelegter Pegelbegriff

**Lage und Tickbedingung.** `DspKern.cpp:768-769`: `const double pegel =
z.pegel.tick (b.huelle.pegelPol, leistungEin); z.huelle.tick (h, pegel);` —
genau zwischen Detektor und Hüllkurve, und ausschließlich im Zweig
`if (detektorAktiv)` (`:733-770`). Der Pol kommt aus `b.huelle`, nicht aus dem
gemischten `h`; das ist die Konsequenz der Topologiezusage und nicht eine
Abkürzung, weil `rampenKompatibel` auf beiden Seiten denselben Begriff **und**
dieselbe Samplerate verlangt.

**Spitze schließt kurz und ist bitgleich zum Basisstand.**
`PegelZustand::tick` (`DspFilter.h:168-173`) kehrt bei `! (pol > 0.0)`
unverändert zurück und schreibt den Zustand nicht; das fängt Fenster 0,
negative und nicht endliche Pole. `huellkurvePol (0.0, fs)` ergibt 0,0
(`:306-311`). Zur Herkunft des Vergleichsstands: `NAK-311-etappe5-m123-basis.txt`
weist den Fingerabdruck 4420950042542505610 als Ausgabe des Basisstands
`cded8a20` aus, erzeugt **vor** der ersten Änderung am Produktcode; die Datei
nennt Prüfstand, Befehl und Folge. Ich habe den Anspruch zusätzlich aus der
Quelle geprüft: unter `spitze` ist die Kette mathematisch identisch zum
Vorzustand (der Tick gibt den Eingang zurück und schreibt nichts), und die
übrigen Änderungen des Satzes D an `BandZustand` sind neutral, weil
`istDenormalKlein(0,0)` falsch ist (`DspFilter.h:66-69`) und `pegel.leistung`
unter Spitze bitgenau +0,0 bleibt. Der Lauf bestätigt es: 311/M-123
4420950042542505610 gegen 4420950042542505610, Kurzschluss auch bei NaN, ±Inf
und denormalem Zustand.

**Pol im Worker, nie im Callback.** `huellkurveEntwurf` (`DspFilter.h:317-325`)
wird nur von `baueProgramm` gerufen (`DspProgramm.cpp:352-354`), also über
`baueVor` im Worker. Gemessen in 311/M-129: Rumpf von `verarbeiteBand` 9742
Zeichen, genau ein Aufruf der Pegelstufe, kein Entwurf und keine transzendente
Funktion; ein Mult und ein Add je Sample.

**Zustand, Nullen, Heilung, Wanderung.** `PegelZustand pegel {}` liegt im
`BandZustand` (`DspBankPool.h:60`); `nullen` (`DspBankPool.cpp:22`),
`istEndlich` (`:33`) und `riegleDenormale` (`:41`) fassen ihn wie die
Hüllkurve. Die E-29-Stelle nullt ihn mit Hüllkurve und Detektor
(`DspKern.cpp:829-834`). Er wandert mit, weil `blockrand` den ganzen
`BandZustand` kopiert — im Rampenweg das Array (`:584`), im Crossfade je Slot
(`:606`); `heileZustaende` (`:629-658`) liest `istEndlich` und
`riegleDenormale` am Blockrand. Gemessen: 311/M-130 (NaN geheilt +1, denormal
geriegelt +1, Ausgang endlich; bei Stille bitgenau +0,0; 63-%-Punkt 10,344 bis
10,363 ms gegen `kPegelFensterMs` 10,0 ± 1 ms bei allen vier Raten),
311/M-131 (c) (Pegel wandert über den Fremdslotwechsel, 0,124999997615 →
0,124999997624), 311/M-141 (nach der ersten Rampe bitgenau +0,0, zweite Rampe
gegen die ausgeschriebene Referenz des frischen Detektors, Residuum
0,000000000000 dB).

**Topologie an beiden Stellen, genau ein Gleichheitsvergleich.**
`rampenKompatibel` bekommt `|| a.pegelbegriff != n.pegelbegriff`
(`DspProgramm.cpp:237`), die bleibt-Bedingung `&& s.pegelbegriff ==
b.pegelbegriff` (`DspKern.cpp:353`). Die drei Wertvergleiche des Satzes C sind
textgleich geblieben (Diff gelesen). Gemessen in 311/M-131 (a) und (b).

**Vertragsweg ergibt immer Durchschnitt.** `b.pegelbegriff =
Pegelbegriff::durchschnitt;` steht unbedingt und ohne Eingabe aus dem `DspSatz`
(`DspProgramm.cpp:339`); eine Volltextsuche über den Produktcode findet
`Pegelbegriff::spitze` nur in `pegelFensterMs` (`DspProgramm.h:73`). Gemessen
in 311/M-125 über alle acht Slots und jede Kombination aus `dynamic_enabled`
und `sidechain_source`.

**Nichts davon erreicht State, Draht, Bericht oder `state_hash`.**
`pegelbegriff` kommt im Produktcode nur in `BandProgramm`, `SlotMerkmal`,
`rampenKompatibel` und `vergebeKennungen` vor; `DspBericht` trägt ihn nicht.
Gemessen: 311/M-126 (Parameterbestand und `state_hash` über B2), 311/M-79
(zwei Sekunden eingeschwungenes Audio bewegen den Pegelzustand auf
0,124999998, Statebytes 4708 → 4708, Revision 1, Slot fährt `durchschnitt`).

**Verschobene Bestandsprüfungen — Referenz und Schranken.** Ich habe jede der
im Auftrag genannten Stellen im Diff gelesen:

- `attack_hold_release_als_sprungantwort_bei_vier_raten`: die Erwartung kommt
  aus `refZeiten` (`DspGoldenTestMain.cpp:4232-4278`) — eigener `RefBandpass`,
  eigener `refPol`, eigene Rekursion über Pegel, Hüllkurve, Holdzähler und
  Steuerraster, kein Kernaufruf. Toleranz `toleranzMs = 1.0` **unverändert**,
  die Spannenprüfung `hi - lo > toleranzMs` unverändert. Lauf: Soll und
  Messung stimmen je Rate auf 0,00 ms (32,11 / 32,17 / 32,08 / 32,04 ms).
- Die drei `huellkurvenwert_…_rampt_ohne_zustandsreset`: die Idealrampe
  bekommt den Pegelpol als vorgeschaltete Stufe; der Startzustand wird — wie
  `lRef` schon am Basisstand — am Kern abgelesen, die Rekursion ist
  ausgeschrieben. Schranken `maxResL < 1e-6 && maxResG < 1e-4 && maxResHold
  <= 1.0` **unverändert** (Kontextzeilen im Diff).
- `range_0_nach_minus_12_rampt_ueber_die_volle_rampe`: Erwartung jetzt aus
  `rampenReferenz` (`:1743-1789`, ganze Kette ausgeschrieben, kein Kernlauf),
  Schranke `maxRes < 1e-6` **unverändert**; gemessen 0,000000000000 dB. Die
  Behandlung wie erste Liste ist in Ausschluss 2 als angenommen benannt.
- `detektor_ist_bandbegrenzt (M-21)`: geprüfte Ungleichung unverändert, siehe
  Ausschluss 2.
- 311/M-73 bis 311/M-75: Schranke `minErste == 1 && maxErste ==
  kDynamikSchritt && minVoll == kDynamikSchritt && maxVoll == 2 *
  kDynamikSchritt - 1` **unverändert**, zusätzlich neu `basis %
  kDynamikSchritt == dSoll`. Die gemessene **Größe** hat sich aber verschoben,
  und die Zusagetexte sind ihr nicht gefolgt: siehe **DEFEKT D-2**.
- 311/M-77 und 311/M-78 (B7): der t_E-Term rechnet `5*(tau_a +
  kPegelFensterMs)*fs` statt `5*tau_a*fs`, die Toleranz 0,1 dB ist
  unverändert; gemessen höchstens 0,027 dB. Die Behauptung in
  `tools/beweise.ps1` ist hier **mitgezogen** worden (Diffzeile B7) — das ist
  der Maßstab, an dem D-2 fehlt.

**Die zweiseitige Schranke von 311/M-124, unabhängig nachgerechnet.** Mit
r = 1/(4·pi·f0·tau_m) und tau_m = 0,010 s ergibt sich 4·pi·0,01 =
0,12566370614 und damit r = 0,00795775 (1 kHz), 0,023337 (341 Hz), 0,0795775
(100 Hz), 0,39788736 (20 Hz). Daraus 10·log10((1 + r)/(1 − r)) =
0,0691 / 0,2028 / 0,6926 / 3,6579 dB und max(0,1; …) = 0,1000 / 0,2028 /
0,6926 / 3,6579. Der Test misst gegen 0,1000 / 0,2027 / 0,6927 / 3,6579 —
Übereinstimmung auf die letzte ausgegebene Stelle. Die einseitige Form
10·log10(1 + r) ergibt 0,0344 / 0,1002 / 0,3325 / 1,4547 gegen die gemessenen
0,1000 / 0,1002 / 0,3325 / 1,4547. §61 Nr. 1 hält also rechnerisch. Gemessene
Spannen 0,0507 / 0,1484 / 0,4975 / 2,2005 dB liegen darunter; der Rest von
2,20 dB bei 20 Hz ist im Register vorgemerkt (Ausschluss 2). Die
Fensterkonstante ist im Test eigenständig ausgeschrieben
(`kPegelFensterRefMs = 10.0`, `:1686`), nicht aus dem Produkt übernommen.

## (e) Rotbeweise je Matrixzeile und die Grenzen

**Rotbeweise.** Für alle 47 Zeilen (M-96 bis M-120, M-123 bis M-144) liegt eine
Rohdatei vor; M-97 und M-98 verweisen ausdrücklich auf die gemeinsame Mutation
in `NAK-311-rot-M-96.txt`, M-111 urteilt über eine Diffprüfung, M-144 über den
Auswertungslauf C-0, M-140 über den Kanon-Diffbefehl. Ich habe je Datei die
Mutationsstelle gelesen und gegen den ZIEL-Code gehalten; jede genannte Zeile
existiert dort wörtlich — Beispiele: M-96/M-102/M-103 an den drei Vergleichen
(`DspKern.cpp:354-356`), M-101 an `gross > grenze * klein`
(`DspProgramm.h:413`), M-105 an `if (z.uebergang != Uebergang::keiner) return;`
(`DspKern.cpp:529`), M-108 an der Kennungsvergabe (`:357`), M-109 an
`if (! brauchbar) return true;` (`DspProgramm.h:410`, `:425`), M-112 an den zwei
Rückgabezeilen (`DspProgramm.cpp:192`, `:203`), M-113 an beiden
Strikt-größer-Stellen (`DspProgramm.h:465`, `:533`), M-116 an `meldeProgramm`,
M-120/M-124 an der Pegelzeile (`DspKern.cpp:768`), M-123 an
`PegelZustand::tick`, M-125 an der einen Setzstelle (`DspProgramm.cpp:339`),
M-129 an `verarbeiteBand`, M-130 an `istEndlich` und `riegleDenormale`
(`DspBankPool.cpp`), M-131 an `rampenKompatibel` **und** `vergebeKennungen`,
M-132/M-135/M-139/M-143 an `bereiteVor` und `freigeben`, M-133 an
`kMinSamplerateHz`, M-134 an der Vergleichsrichtung, M-141 an der
E-29-Stelle, M-142 a/b an den zwei Konstanten. Zeilen mit zwei Hälften tragen
zwei Läufe (M-112, M-130, M-131, M-132, M-138, M-143). Die sechs benannten
NEIN-Fälle aus Ausschluss 2 habe ich an der tragenden Zeile geprüft: M-107
(beide Versuche) fällt an `311/M-53 candidate_slot_behaelt_historie` — diese
Zeile trägt die Zusage „eigener Merkzettel je Pfad" tatsächlich, weil unter der
Mutation das Maß am Tap `post_candidate` auf 8,002865 dB gegen 0,5 dB springt;
M-128 fällt an `311/M-49 slotzahl_null_und_acht` (zweite Liste, Zahlwert
verschoben) mit der gemessenen und plausiblen Begründung, warum
`kennlinie_im_knie_wirkt_im_audiopfad` unter der Mutation grün bleibt;
M-124 Spalte 2a, M-126 an B2, M-129 an der Quelltextwache und M-106 (b) sind
wie benannt. **Ausnahme: M-133.** Die zweite Hälfte der Zeile hat keinen
Riegel und deshalb auch keinen Rotbeweis — die Rohdatei weist den Bitvergleich
als **Ablesen von Hand** aus („Er ist am Zwischenstand VOR der Reparatur und am
Endstand NACH der Reparatur gleich", vier Fingerabdrücke in einer Tabelle). Das
ist D-1.

**Grenzen.** Die Etappen 2 bis 4 sind grün (die vier Läufe oben enthalten
311/M-01, 311/M-10 bis 311/M-80 und 311/M-90 bis 311/M-95 ohne Fehler); als
Zahlwerte verschoben sind nur die in §60.7 geführten, mit unveränderten
Schranken. RBJ-Goldens (M-82), Stereo-Goldens (M-83) und die vierzehn
Hexgoldens von 311/M-61 sind bitgleich — 14 von 14 per `memcmp`, dazu
311/M-119 (alle zwischen −5,997027825804 und +0,230273950213 dB, keiner über
dem Deckel, keiner nicht bitgleich, keiner mit gesetztem Zustand). Keine neue
Stateversion (`eq-copilot/schemas/state` und `eq-copilot/plugin/state` ohne
Versionsänderung im Diff, `eq-copilot/fixtures` unberührt), keine
v3-Vertragsänderung außer der einen `$comment`-Zeile, Identität eingefroren
(`eq-copilot/identity` unberührt). Keine Allokation, Sperre, Ableitung oder
transzendente Funktion neu im Audiopfad: `null_allokationen_im_callback…`
meldet Testzähler 0 und Kernzähler 0 über 4000 Blöcke und 4 179 358 Samples
mit acht dynamischen Bändern, 311/M-129 misst den Rumpf am Quelltext,
311/M-139 misst 0 Allokationen und 0 Sperren im abgelehnten Fenster.
Ausgeschaltet bitidentisch: A16 131/0.

---

## Befunde

### DEFEKT

**D-1 — 311/M-133 hat für seine zweite Hälfte keinen Riegel; die Riegelspalte
der Matrix behauptet einen.**

- Datei/Zeile: `eq-copilot/plugin/tests/SondeNullTestMain.cpp:1669` (der
  Fingerabdruck wird berechnet) gegen `:1676-1678` (die `pruefe`-Bedingung).
- Der Satz, der bricht: Matrixzeile M-133 (§39.4), Zusagespalte — „der Ausgang
  ist bitgleich zum Lauf am Basis-SHA der Etappe (**memcmp**)" — und
  Riegelspalte — „A16 bestehend …, **NEU** um den Bitvergleich gegen den
  Basisstand ergänzt". Mitbetroffen: die Behauptung von A16 in
  `tools/beweise.ps1:688` („der Ausgang trägt bei 44,1 / 48 / 96 / 192 kHz
  denselben Fingerabdruck wie am Basisstand des Änderungssatzes") und das
  Prüfetikett `:1681-1682` („und der Ausgang trägt den Fingerabdruck des
  Basisstands").
- Befund: `const auto abdruck = fnvAusgang (*p, 40, 512, (int) rate + 133);`
  fließt ausschließlich in die Detailzeichenkette. Die geprüfte Bedingung ist
  `e.ausgang == tx::Ausgang::commit && hoerbar && kern.samplerate() == rate &&
  abgelehnt == 0.0 && ! std::signbit (abgelehnt)` — ohne jeden Vergleich des
  Fingerabdrucks. Es gibt keine Goldenkonstante: eine Suche nach den vier
  ausgegebenen Werten `cb14d6eee50e7d7c`, `bd787728731ba929`,
  `5ece3bcf185baff5`, `33f35b42b6693e06` über den ganzen Baum trifft nur fünf
  Dokumente (`docs/beweise/NAK-311.md`, die zwei Kanon-Rohausgaben, der
  A16-Zeilenvergleich, `NAK-311-rot-M-133.txt`) und **keine** Quelldatei.
  `fnvAusgang` wird in der Datei genau einmal gerufen (`:180` Definition,
  `:1669` Aufruf).
- Reproduktion:
  `grep -n "fnvAusgang" eq-copilot/plugin/tests/SondeNullTestMain.cpp` liefert
  Definition und den einen Aufruf;
  `sed -n '1661,1688p' eq-copilot/plugin/tests/SondeNullTestMain.cpp` zeigt,
  dass `abdruck` nicht in der `pruefe`-Bedingung steht;
  `grep -ril "cb14d6eee50e7d7c" .` liefert nur Dokumente.
  Folge: eine Regression im hörbaren 44,1-kHz-Pfad ließe 311/M-133 grün.
- Einordnung: das Muster, das hier fehlt, steht im selben Änderungssatz zweimal
  richtig — `311/M-100` vergleicht `13210420415127086859` gegen den Basisstand,
  `311/M-123` vergleicht gegen die Konstante `kM123Golden`. Die praktische
  Aussetzung ist klein, weil B6 (RBJ-Goldens, Hexgoldens, 311/M-61) und die
  A16-Nulltests denselben Kern anderweitig festhalten; die Zusage der Zeile ist
  trotzdem ungemessen, und ihr Etikett ist „Regressionswache".
- Nacharbeit (Vorschlag, Entscheid beim Dirigenten): die vier Fingerabdrücke
  als Konstanten in den Test nehmen und in die Bedingung aufnehmen, wie
  `kM123Golden`.

**D-2 — der Code des Satzes D falsifiziert den Zusagetext von 311/M-73 bis
311/M-75; zwei Dateien im Prüfbereich tragen ihn unverändert weiter.**

- Datei/Zeile: `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4691-4693` (das
  Prüfetikett) und `tools/beweise.ps1:723` (Behauptung von B6, der
  Etappe-4b-Satz, im Diff unverändert übernommen).
- Der Satz, der bricht: „gegen einen Referenzkern mit Range 0 … bleibt der Tap
  `post_committed` nach dem Einsatz eines Quadraturtons **bis einschließlich
  Sample d = (8 − p) mod 8** bitgleich und **weicht ab d + 1 ab**; der Entwurf
  fällt auf Sample d und **wirkt ab d + 8** mit Gewicht 1 — dort trägt `svfVon`
  genau den bei d entworfenen Satz." Wortgleich in der Matrixzeile M-73 (§9).
- Gemessen am ZIEL (mein Lauf, B6 Zeile 184 bis 186):
  `311/M-73`, `p=0 d=0 Bezugspunkt 8 erste Abweichung 9 … volle Wirkung 16`;
  `311/M-74` (44,1 kHz) ebenso;
  `311/M-75` (96 kHz), `p=7 d=1 Bezugspunkt 9 erste Abweichung 10 … volle
  Wirkung 17`.
  Für p = 0 ist d = 0: der Tap bleibt also **bis Sample 8** bitgleich, nicht
  bis 0, weicht ab 9 ab, nicht ab 1, und die volle Wirkung liegt bei 16, nicht
  bei 8. Das ist eine Produkttatsache, keine Messfrage — die kalt startende
  Pegelstufe schiebt den ersten **wirksamen** Entwurf um einen Rasterschritt
  nach hinten.
- Reproduktion:
  `eq-copilot/build/plugin/EqCopDspGoldenTest_artefacts/Release/EqCopDspGoldenTest.exe`
  und die Zeilen `311/M-73` bis `311/M-75` lesen.
- Einordnung: die **Substanz** ist unversehrt. §40.4 Risiko 3 hat den neuen
  Bezugspunkt vorhergesagt und den Weg vorgeschrieben („der Bezugspunkt wird im
  Test aus `schrittRest` und der Auslenkung gelesen … die geprüften
  Samplezahlen bleiben dieselben"), §60.7 hat die Verschiebung Zahl für Zahl
  geführt, und die Schranke (1 bis 8, 8 bis 15 Samples, jetzt relativ zum
  Bezugspunkt) hält mit einer zusätzlichen Bedingung. Der Befund ist, dass die
  **Zusagetexte** an zwei Stellen des Prüfbereichs nicht mitgezogen wurden,
  während derselbe Änderungssatz sie für 311/M-77 und 311/M-78 (t_E) in
  `tools/beweise.ps1` sehr wohl mitgezogen hat. Ein grüner Kanon beglaubigt
  damit einen Satz, den dieser Änderungssatz widerlegt hat. Ausschluss 2 nennt
  genau diesen Fall als Befund („wohl aber ist ein Befund, wenn der Code
  dadurch eine Matrixzeile … bricht"). Die Matrixzelle in `docs/**` selbst ist
  nach Ausschluss 1 kein Prüfgegenstand und hier nur als Fundstelle genannt.
- Nacharbeit (Vorschlag): die drei Sätze in `DspGoldenTestMain.cpp:4691-4693`
  und in `tools/beweise.ps1:723` auf den Bezugspunkt umformulieren („ab dem
  ersten wirksamen Steuerschritt"), und die Matrixzelle M-73 im Manifest
  append-only berichtigen.

### LÜCKE

**L-1 — 311/M-142 (a) und (b) messen ihre Bitgleichheit zum Basisstand nicht
im Riegel.** Die Zusagespalte verlangt „Der Ausgang ist über den ganzen Lauf
bitgleich zum Lauf am Basisstand"; die Riegelspalte nennt die zwei bestehenden
Prüfungen `bandwert_gain_db_rampt_ohne_zustandsreset` und
`bandwert_q_rampt_ohne_zustandsreset` ausdrücklich als „unverändert". Diese
prüfen das Residuum gegen die ausgeschriebene Koeffizientenrampe, nicht den
Basisstand; der Basisvergleich läuft über den einmaligen Zeilenvergleich
(`NAK-311-etappe5-b6-zeilenvergleich.txt`, 0 abweichende Zeilen für diese
zwei). Anders als bei D-1 behauptet die Riegelspalte hier keinen eingebauten
Bitvergleich — deshalb Lücke, nicht Defekt. Benannt, keine Nacharbeit.

**L-2 — keine Prüfung deckt eine Publikation im abgelehnten Fenster.** Ich habe
alle fünf Publikationswege gelesen und jeden als geriegelt befunden (siehe (a));
der Fall ist damit durch Konstruktion ausgeschlossen und in keiner Matrixzeile
benannt. Fiele einer der Riegel weg, bliebe der Ausgang trotzdem bitgleich,
weil `verarbeiteStueck` vor dem ersten Sample zurückkehrt — es gibt also keine
Aussetzung, nur keine Messung. Benannt, keine Nacharbeit.

### HÄRTUNG

**H-1 — die Herleitung der abgelehnten Rate steht zweimal.** Das Prädikat steht
einmal, die Zeile `samplerate > 0.0 && samplerate < kMinSamplerateHz` aber in
`DspKern.cpp:127` und `NakamaTransaktion.cpp:361`. Der Kommentar in
`DspProgramm.h:109-112` erklärt das ausdrücklich („den Unterschied macht der
Aufrufer"); ein dritter Verbraucher müsste sie erneut abschreiben. Eine zweite
`constexpr`-Funktion neben dem Prädikat würde das schließen.

**H-2 — `m123Lauf (false)` ist am ZIEL nicht erreichbar.**
`DspGoldenTestMain.cpp:2039` nimmt den Parameter `mitSpitze`, `f123` ruft nur
`m123Lauf (true)` (`:2099`). Der Zweig existiert, weil der Bauer damit am
Basisstand den Golden erzeugt hat; heute ist er toter Schaltweg im Test.

**H-3 — `PegelZustand::tick` hat keine Obergrenze für den Pol.** Bei
`pol >= 1.0` friert der Zustand ein. Über den Vertragsweg entsteht das nicht
(`huellkurvePol` liegt für jede gültige Rate in [0, 1)), und der Fall gehört zu
der in §58 für das Register vorgemerkten Härtung „Ausgangsriegel bei nicht
endlichen Koeffizienten, die über den Vertragsweg nicht erreichbar sind".

**H-4 — `eq-copilot/install/nakama-installer-v1.json` trägt einen neuen
Broker-Hash ohne Brokeränderung.** `git diff BASIS...ZIEL -- broker/` ist leer;
der Hash wechselt von `F29F4AC4…` auf `FBE41339…`, also durch einen Neubau des
Rust-Brokers. Das ist der vorgesehene `--hashen`-Weg, bindet das Manifest aber
an ein Binary, das nur im Bautree dieses Rechners liegt; auf dem zweiten
Rechner verweigert der Installer bis zum eigenen Neubau. Kein Ticketbruch,
aber ein Punkt für das Abschlussfenster.

**H-5 — 311/M-133 trägt einen zweiten Fingerabdruck, den niemand liest.** Neben
dem fehlenden Vergleich (D-1) gibt die Zeile die vier Werte nur aus; ein
Zeilenvergleich fängt sie erst, wenn ihn jemand fährt. Die zwei Muster im
selben Änderungssatz (`311/M-100`, `311/M-123`) zeigen den kürzeren Weg.

---

## Geprüft — selbst an der Quelle nachgemessen

Ich habe den vollständigen Diff über die 18 Prüfpfade gelesen (Produktcode
`DspBankPool.cpp/.h`, `DspFilter.h`, `DspKern.cpp/.h`, `DspProgramm.cpp/.h`,
`SondeProcessor.cpp`, `NakamaTransaktion.cpp/.h`, dazu Schema, Installer und
Runner; die vier Testdateien in den geänderten Abschnitten und in jeder
Bestandsprüfung, deren Erwartung sich verschoben hat). Darüber hinaus habe ich
am ZIEL-Quellstand vollständig gelesen: `DspKern::bereiteVor`, `freigeben`,
`zaehlerZuruecksetzen`, `meldeProgramm`, `merke`, `vergebeKennungen`,
`uebernehmeZustand`, `baueVor`, `publiziereVorbau`, `beendeCandidate`,
`blockrand`, `heileZustaende`, `verarbeiteBand`, `verarbeite`,
`verarbeiteStueck`; `DspProgramm::leiteAutoGainAb`, `rampenKompatibel`,
`baueProgramm`, `gedeckelterAutoGainDb`, `sprungImVerhaeltnis`, `sprungInDb`,
`samplerateUnterstuetzt`, `pegelFensterMs`, `autoGainGedeckelt`;
`huellkurvePol`, `huellkurveEntwurf`, `PegelZustand`, `istDenormalKlein`;
`SondeProcessor::prepareToPlay`, `releaseResources` und die zwei
Publikationsstellen `:1096` und `:1476`; `Transaktionskern::setzeSamplerate`,
`baueBericht`, `DspKernAusfuehrung::baueVor`, `publiziereVorbau`,
`publiziereWirksam`, `publizierePreview`.

Selbst gefahren und gegen die Quelle gehalten: die vier Beine B6, B7, B3c und
A16 (Zahlen oben), mit Auswertung jeder Matrixzeile M-96 bis M-120 und M-123
bis M-144 in der Ausgabe. Selbst nachgerechnet: die einseitige und die
zweiseitige Schranke von 311/M-120 und 311/M-124 an allen vier Bandmitten
(Ergebnis in (d), Übereinstimmung mit dem Test auf die ausgegebene Stelle);
`istDenormalKlein(0,0)` als falsch und damit die Neutralität des neuen
Denormalriegels unter Spitze. Selbst gemessen über `git`: der Änderungssatz
berührt außerhalb `docs/**` nur die 18 Prüfpfade; `eq-copilot/fixtures`,
`eq-copilot/schemas/state`, `eq-copilot/identity` und `broker/` sind leer im
Diff; `eq-copilot/schemas` zeigt genau eine geänderte Zeile. Selbst geprüft:
die 47 Rotbeweisdateien auf Urteil, Mutationsstelle und Rücknahme, jede
Mutationsstelle gegen den ZIEL-Code; die Abwesenheit einer Goldenkonstante für
311/M-133 über eine Volltextsuche des ganzen Baums. Selbst gemessen:
Binary-Zeitstempel gegen die jüngsten Quellen; keine laufenden Bau- oder
Testprozesse vor den Läufen.

## Nicht geprüft — nur gelesen oder ausdrücklich ausgeschlossen

**Nur gelesen, nicht nachgefahren:** der volle Kanon (`GRUEN 69/69` auf
`a5815dfc`, Rohausgabe `NAK-311-a5815df-dirty.md`) und die Hinweisbeine A32 und
A33 — ich baue nichts und fahre den Runner nicht. Die Beine A1, B1, B2, A5, A8,
A9 und A14 habe ich nicht gefahren; ihre Aussagen (Identität, Parameterbestand,
Korpus) habe ich stattdessen am Diff geprüft, der die zugehörigen Dateien nicht
berührt. Die Messläufe in `NAK-311-etappe5-wertspruenge.txt`,
`-wertekriterien.txt`, `-bankpool-wiederholungen.txt` und `-pegelbegriff.txt`
sowie die vier Zeilenvergleiche habe ich als Beleg gelesen, nicht wiederholt —
sie brauchen Läufe am Basisstand und damit einen Bau. Der M-123-Basisstand
(`cded8a20`) ist über die Rohdatei und über die Rechnung aus der Quelle
geprüft, nicht durch einen eigenen Lauf des alten Kerns.

**Ausdrücklich ausgeschlossen und deshalb nicht bewertet:** `docs/**` samt
Manifest, Register, Planstand, Kommentarwortlauten und den SONDE-015-Nachträgen
(die Matrixzellen M-73 und M-133 sind oben nur als Fundstelle genannt, nicht
als Befund); die im Register datierten oder für den Ticketabschluss
vorgemerkten Härtungen (NAK-81, NAK-269, NAK-286, NAK-306, NAK-331, NAK-332,
die Punkte aus §58 und §61); die Härtungen und Lücken der Matrixprüfungen 1 bis
6 und der Erstprüfungen 2 bis 4 samt R-311-11 und R-311-12; die in §51.7,
§53.7, §57.8 und §60.9 benannten und vom Dirigenten angenommenen Abweichungen
der vier Bauer, darunter die sechs NEIN-Rotbeweise, die zweiseitige Schranke
von 311/M-124, `DspBankPool.cpp` als Ticketpfad, die Zahlenübergabe an
`huellkurveEntwurf`, die Behandlung von `range_0_nach_minus_12…` und
`detektor_ist_bandbegrenzt` wie die erste Liste, die abweichende Zeile „die
dynamische Auslenkung bewegte sich im Lauf", `bankpool_parallel_ohne_rennen`
als nichtdeterministischer Bestandsfall, die Quelltextwachen und `setzeSpitze`;
`tools/dirigent/pruefliste.md`; die Gate-Sätze der Etappen 2 bis 4, „Nulltest
im Host bitidentisch", der Laufzeit-Arm und die Abschlussprüfung T2;
Sicherheitsbefunde T3-12-*; Legacy-Bezeichner, Design, Kosmetik, Stil,
optionale Härtung und theoretische Randfälle ohne Zusage.

---

HEAD am Ende: `4ffdc70a4460ae5e3c151589291379575e77c7ca` — unverändert.
`git status --short` zeigt nur die zwei fremden, untracked Ordner
`briefing-hub/` und `nimbalyst-local/` sowie diese Datei. Nichts gestagt,
nichts committet, nichts gepusht, nichts gebaut.
