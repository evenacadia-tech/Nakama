# NAK-311 — Abschlussprüfung T2 (Planschritt S25j), 20.09.2026

URTEIL: NEEDS_WORK — ein DEFEKT (D-1: die Behauptung B6 in `tools/beweise.ps1:723` behauptet über den Rumpf von `DspKern::verarbeiteBand` mehr, als gemessen ist, und die Aussage ist am ZIEL-Code falsch), dazu zwei Lücken und vier Härtungen.

BASIS `7463c3ebe78c2637bec9da42e23d7f6f2bd9af38`, ZIEL `3ff8430bf1dbeeff2394e75a7eba193de967c52b`.
HEAD zu Beginn: `4be1d193fd58b80d031d5fa94a6f384a2055246e`. Im Prüfbereich ist HEAD
zeichengleich zu ZIEL (`git diff 3ff8430b..4be1d193 -- eq-copilot/plugin eq-copilot/schemas
eq-copilot/fixtures eq-copilot/install tools/fl/nulltest.py tools/beweise.ps1
tools/eq-copilot/erzeuge_v3_fixtures.py` ist leer; der einzige Unterschied HEAD ↔ ZIEL ist
`docs/beweise/roh/NAK-311-abschlusspruefung-auftrag.txt`).

---

## (a) Zusammenspiel der Etappen im Kern

Die sechs Etappenzustände treffen sich widerspruchsfrei. Der **Merkzettel** (W03/W07/W35,
`DspKern.h:479-506`) führt genau die sieben Topologiefelder, die `rampenKompatibel`
(`DspProgramm.cpp:218-241`) je Slot nennt — `typ`, `modus`, `dynamisch`, `nutztSvf`,
`quelle`, `pegelbegriff` plus `aktiv` —, dazu die vier globalen Felder (`eqEngagiert`,
`hardBypass`, `samplerate`, `monoBassAn`) und die drei Werte aus W07. Beide Listen decken
sich Feld für Feld; ein siebtes Topologiefeld ohne Gegenstück in `rampenKompatibel` oder
umgekehrt gibt es nicht. `vergebeKennungen` läuft ausschließlich in `publiziereVorbau`
(`DspKern.cpp:416`), also im Worker, nach `bank.programm = prog` und vor
`baenke.publiziere` — Zuordnung und Übergabe liegen auf derselben Seite der
Release-/Acquire-Kante. Die beiden Lesestellen am Blockrand (`DspKern.cpp:569-610`)
verlangen für den Rampenweg zusätzlich gleiche Pfad- und Slotkennung; übertragen wird im
Crossfade nur der Slot, dessen Kennung gleich blieb.

Die durchgespielten Folgen tragen: **Ausschalten und wieder ein** — die ENDE-Marke leert
den Merkzettel (`:395`), der Pfad endet über `verarbeitePfad` in der Ruhe und setzt dort
`rampen.setzeSofort (1.0)` (`:1052`), ebenso `beendeHoerHalt` (`:516`) und
`beendeAudiohistorie` (`:234-238`); ein späteres Programm schaltet damit wie ein frischer
Kern ein. **Remove und Neubelegung** — eine verdrängte Zwischenpublikation bleibt im
Merkzettel stehen, weil er die zuletzt PUBLIZIERTE und nicht die genommene Belegung führt;
311/M-41 misst genau das (in dieser Session grün, Slot-0-Meldung exakt 0,0). **Großer
Sprung im laufenden Crossfade** — `blockrand` kehrt bei laufendem Übergang früh zurück
(`:529`), die zweite Publikation wird erst am Blockrand danach genommen (311/M-105 grün).
**Ratenwechsel 48 → 32 → 48 kHz mit dynamischem Band** — `bereiteVor` nimmt bei nicht
unterstützter Rate den Weg von `freigeben`, setzt Pool und beide Pfade in die Ruhe und
nullt die drei Auto-Gain-Melder und die Zähler; der Merkzettel bleibt zwar stehen, wirkt
aber nicht: beim nächsten Blockrand ist `alt < 0`, also ist `pAlt == nullptr`, `pfadGleich`
falsch, und weder der Rampenweg noch die Zustandsübernahme des Crossfades ist erreichbar
(Härtung H-3). **Ladestart** — beide Publikationswege außerhalb von `prepareToPlay` sind
mit `dspKern->samplerate() > 0.0` geriegelt (`SondeProcessor.cpp:1096`, `:1476`), im
abgelehnten Fenster ist die Rate 0, und der Ladestand wird beim nächsten unterstützten
`prepareToPlay` nach `vergissLetztePublikation` mit `erzwingen = true` vollständig
publiziert. **Candidate- und Hörhaltpfad** — im Hörhalt ist `z.aktiv == -1` und
`uebergang != keiner`, `blockrand` nimmt dort nichts; `beendeCandidate` leert den eigenen
Merkzettel des Pfades (`:441`). **Monobus** — die Kanalzahl wird in `prepareToPlay` EINMAL
gemessen (`SondeProcessor.cpp:245`) und an Kern und Transaktionskern gereicht; gelesen wird
sie nur von `leiteAutoGainAb`. **Pegelzustand und Hüllkurve** — `z.pegel` liegt im
`BandZustand`, wandert also über dieselbe Blockrandübernahme wie Filter- und
Detektorzustand, wird mit dem abgeschalteten Detektor gliedweise genullt
(`DspKern.cpp:829-834`) und am Blockrand geheilt und denormalgeriegelt
(`DspBankPool.cpp:13`, `:26-28`, `:37`). Der Pol kommt aus `b.huelle.pegelPol`, nie aus dem
gemischten Satz; weil `rampenKompatibel` gleichen `pegelbegriff` UND gleiche Samplerate
verlangt, tragen beide Seiten einer Rampe bitgleich denselben Pol. **autoGainBericht,
Rohwert und Deckelzustand** werden in `meldeProgramm` (`:261-274`) am selben Ort, vom
selben Schreiber und aus demselben Programm belegt und können deshalb nicht auseinander
laufen. Ein Fall, in dem ein Zustand einer Etappe einen der anderen veraltet oder falsch
übernimmt, ist mir nicht begegnet; ein nicht gedeckter Randfall steht als Lücke L-1.

## (b) Grundgesetz am Endstand

Ausgeschaltet und hard-bypassed wird kein Sample geschrieben: `verarbeiteStueck` kehrt bei
`committedRuht` vor der Schreibschleife zurück (`DspKern.cpp:1220-1229`), und die
Schreibschleife ist der einzige Ort, der in `kanaele[..]` schreibt. In dieser Session
gemessen: A16 311/M-01 gibt unter `juce::ScopedNoDenormals` mit MXCSR `0x00009FE0`
(FTZ 1, DAZ 1) über 18 Kombinationen (Zustände a/b/c × Stereo/Mono × 44,1/48/96 kHz) für
das Bitmuster aus Subnormals, ±0 und kleinsten Normalen **a 0, b 0, c 0** veränderte
Muster aus. Engagiert-neutral und Mix 0 tragen dieselbe Zusage über die Neutralprüfung
(`:1310-1337`): sie fragt `Rampe::ruhtBei` statt eines Zielvergleichs, schneidet das
Schreibende auf `max (cUebergangsEnde, hoerFadeRest)` und greift nur, wenn die Hörmatrix
Processed oder Dry hört — bei Delta und Candidate schreibt der Kern weiter, was 311/M-11
und 311/M-49 messen. Bei `mixNullRuht` liefert `verarbeiteBank` bitgenau `dryL/dryR`
(`:948`), Output-Trim ist im Kurzschluss; der Rückweg über `float` wäre ohnehin exakt, aber
es wird gar nicht geschrieben. Bei nicht unterstützter Rate ist `dryL` leer, und
`verarbeiteStueck` kehrt in der ersten Anweisung zurück (`:1102`) — kein Sample gelesen
oder geschrieben, kein Tap gefüllt, `maxBlockGroesse` 0, also auch kein Stückeln und kein
Zähler; Latenz und Tail misst B7 (in dieser Session grün).

Neu im Audiopfad sind eine Multiplikation, eine Addition und ein `double`-Zustand je Sample
und laufendem Detektor (`PegelZustand::tick`, `DspFilter.h:363-368`), am Blockrand eine
Schleife über acht Slotkennungen und je Stück eine Handvoll Vergleiche der
Neutralprüfung. Keine Allokation, keine Sperre, kein Logging: gemessen über
`null_allokationen_im_callback_samt_programmwechseln` (4000 Blöcke, 4 179 358 Samples,
Testzähler 0, Kernzähler 0) und `null_sperren_im_callback` (0). `leiteAutoGainAb` meldet
seinen Rechenort selbst; `auto_gain_wird_nicht_im_audiothread_gerechnet` misst ≥ 4
Ableitungen außerhalb und 0 im Audiopfad. `vergebeKennungen` und jeder Programmbau hängen
ausschließlich am Publikationsweg: die einzigen Aufrufer von `uebernehmeZustand`,
`baueVor` und `publiziereVorbau` im Produktcode sind `DspKernAusfuehrung`
(`NakamaTransaktion.cpp:710`, `:741`, `:758`, `:772`), also der Worker unter dem
Zustandsschloss. Eine Einschränkung zur Zusage über den Rumpf von `verarbeiteBand` steht in
D-1.

## (c) Vertrag, State, Identität

Gemessen mit `git diff --stat BASIS...ZIEL`: über `eq-copilot/schemas` **genau eine Zeile**
— der `$comment` von `auto_gain_db` in `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2325`
(1 +, 1 −); `minimum` −120, `maximum` 120 und `type` stehen Wort für Wort. Über
`eq-copilot/fixtures` **genau eine Zeile** — das Feld `warum` des Eintrags
`ungueltig/dsp-auto-gain-ausserhalb.json` in `MANIFEST.json`, dazu die spiegelnde Zeile in
`tools/eq-copilot/erzeuge_v3_fixtures.py`; kein Fixture-Byte, kein Hash. Über
`eq-copilot/schemas/state` und `eq-copilot/identity` ist der Diff **leer**. Die
Installer-Datei trägt nur die drei nachgezogenen SHA-256-Zeilen. `git grep` über
`eq-copilot/schemas`, `eq-copilot/fixtures`, `eq-copilot/plugin/state` und `broker/src`
nach `pegelbegriff`, `pegelzustand`, `lebenszyklus`, `autoGainGedeckelt`, `autoGainRoh`,
`abgelehnteSamplerate` und `kanalzahl` findet keinen Treffer, der aus diesem Ticket stammt
(die Treffer auf „Kennung" gehören zu Diagnose-, Installer- und Parametertexten des
Bestands). Die zwei neuen Berichtsfelder `DspBericht::abgelehnteSamplerateHz` und
`::autoGainGedeckelt` sind C++-lokal; B3c misst beide Richtungen (Dokument mit dem Feld
wird von `additionalProperties: false` abgewiesen, ohne das Feld ist es gültig).
Discriminator, Zieladresse, Revision und Capability sind unberührt; `dsp_schema_version`
steigt nicht.

## (d) Je Gate-Befund ein messender Test mit Rotbeweis

Für jeden im Gate genannten Befund liegt mindestens ein Test an der tragenden Zeile plus
Rotbeweisdatei vor: T3-01-01 und T3-09-02 → 311/M-02 bis M-22 (`SondeNullTestMain.cpp`,
`DspGoldenTestMain.cpp`, `tools/fl/nulltest.py`); T3-15-05 → M-23 bis M-39; T3-14-02 und
T3-15-08 → M-40 bis M-56; T3-16-04 → M-57 bis M-66; T3-15-09 Teil a → M-67 bis M-71,
Teil b → M-112 bis M-119; T3-15-11 → M-72 bis M-79; T3-15-07 → M-96 bis M-111, M-142,
M-144; T3-15-06 → M-120 bis M-132, M-141; F08 → M-133 bis M-140; F12 → M-76 bis M-80.
T3-15-10 ist nach Karte U46 vertagt und nicht gebaut. Unter `docs/beweise/roh/` liegen
**139** Dateien `NAK-311-rot-M-*.txt`; lückenlos fehlen nur M-01 und M-81 — M-01 ist
ausdrücklich eine Messung ohne Zusage („prüft nur, dass alle 18 Kombinationen liefen"), und
M-81 ist die Abschlusszeile selbst, deren Diffbefehl in dieser Prüfung läuft. Goldens: der
Diff über `DspGoldenTestMain.cpp` entfernt oder ändert **kein** Hex- oder Zahlengolden der
Abschnitte C, G und H; die einzigen gelöschten Zeilen mit Zahlen gehören zur
Sprungantwort-Referenz (M-127, in der Matrix mit der Zahl 31,7 ms begründet), zum
Vorgängerblock von 311/M-141 und zu zwei Prüfungen, die **verschärft** statt entfernt
wurden (`ableitungenAusserhalb() >= 2` → `>= 4`, Prüfname von
`null_allokationen_im_callback…` um 311/M-111 und 311/M-129 ergänzt). Die Wachmarke wurde
auf `ausBitmuster` umgestellt (§53.7 Nr. 1, angenommene Abweichung). Neue Goldens
(`kM61Bits`, `kM100Golden`, `kM133Goldens`, `kM123Golden`) tragen ihre Herkunft im Kommentar
und haben eine Rohdatei im Repo.

## (e) Behauptungen und Prüfetiketten

Stichprobe von zehn Fällen quer über die Etappen, je Etikett gegen die Bedingung gelesen:
**311/M-01** (`SondeNullTestMain.cpp:1426`) — Etikett sagt ausdrücklich „alle 18
Kombinationen liefen", Bedingung ist `kombinationen == 18`; die Behauptung A16 sagt
denselben Satz („prueft nur, dass alle 18 Kombinationen liefen"). **311/M-23**
(`DspGoldenTestMain.cpp:8588`) — Bedingung prüft Bytegleichheit zum frischen Kern,
Spitzenverhältnis, Abweichung von 1,0 und E-31 am Umschaltsample; deckt das Etikett.
**311/M-41** (`:7821`) — `vorherA[0] == -12.0 && uebernahmen == 1 && slot0Max == 0.0 &&
spitzeNachFade == 0.0`; misst den Kaltstart nach verdrängter Zwischenpublikation.
**311/M-61** (`:5374`) — `m61Gleich == kM61Zahl`, 14 × `memcmp`. **311/M-71**
(`SchemaTestMain.cpp:1559`) — Kern und `autoGainLin` bleiben beim ungeklemmten Wert,
`klemmungen` leer. **311/M-100** (`DspGoldenTestMain.cpp:1057`) — keine Crossfade-Blöcke,
Kennung unverändert, 20 Übernahmen, Fingerabdruck gleich dem Golden. **311/M-116**
(`:6193`) — Zustand gesetzt/gesetzt/falsch, zweimal bitgenau der Deckel, einmal bitgleich
zum Rohwert, dazu die Rampe statt eines Sprungs. **311/M-124** (`:2010`) — `spanne <=
pegelSchrankeSpanne (f0)`, und die Behauptung B6 nennt die gemessenen Spannen samt dem
Satz, dass die Restwelligkeit mit fallender Bandmitte wächst und als Zahl in der Schranke
steht. **311/M-133** (`SondeNullTestMain.cpp:1690`) — der Fingerabdruck steht jetzt IN der
Bedingung, dazu `abgelehnt == 0.0` mit Vorzeichenprüfung. **311/M-141** (`:2540`) —
Pegelzustand bitgenau +0,0, Residuum und erster Steuerschritt gegen die ausgeschriebene
Referenz. Neun der zehn tragen; **311/M-129 trägt sein eigenes Etikett, aber nicht den
Satz, den die Behauptung B6 daraus macht — das ist D-1.**

`tools/fl/nulltest.py`: BITIDENTISCH verlangt seit dem Ticket `v == 0 && nullvorzeichen ==
0 && sha_gleich` (`:255`), also **gleiche Bytes** des Bereichs; die SHA-256 wird über
dieselbe `<f4`-Darstellung gebildet wie der Bitvergleich (`:224`, `:232-233`).
NULLVORZEICHEN ist ein eigenes Urteil mit Szenario-Exit 5 (`:254`, `:287`), zählt getrennt
(`nullvorzeichen_stellen = bitverschieden & ~ungleich`, `:225`) und weicht dem
Wertunterschied: `abweichungen != 0` verlässt den Block vor der Regelliste, und ein
Bytunterschied ohne Wert- und Nullvorzeichenunterschied endet fail-closed als ABWEICHUNG.
NaN ist nie wertgleich, fällt also in `abweichungen`, nicht in `nullvorzeichen`; der
Selbsttest misst das in 311/M-07. Die Rangfolge im Kopf der Datei (`:33-39`) beschreibt
genau diese Regeln.

---

## Befunde

### DEFEKT

**D-1 — die Behauptung B6 sagt über den Rumpf von `verarbeiteBand` mehr, als gemessen ist,
und die Aussage ist am ZIEL falsch.**

| Ort | Inhalt |
|---|---|
| `tools/beweise.ps1:723`, letzter Satz der Behauptung B6 | „Im Callback bleiben die Zaehler mit acht dynamischen Baendern bei 0 Allokationen und 0 Sperren, **und der Rumpf von verarbeiteBand traegt keinen Entwurf und keine transzendente Funktion**." |
| `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:2260` | derselbe Wortlaut in der Ausgabezeile der Prüfung: `", kein Entwurf und keine transzendente Funktion"` |
| gemessene Bedingung, `DspGoldenTestMain.cpp:2246-2256` | der Quelltext des Rumpfes enthält keine der sieben Zeichenketten `std::exp`, `std::pow`, `std::log10`, `huellkurvePol`, `huellkurveEntwurf`, `new `, `malloc` und genau ein `z.pegel.tick` |
| Code am ZIEL, `eq-copilot/plugin/dsp/DspKern.cpp:791` | `z.svfNach = svfEntwurf (b.typ, grundG, q, gainDb + gDyn);` — ein Entwurf IM Rumpf; der Kommentar zwei Zeilen darüber (`:772`) nennt ihn selbst „Neuentwurf" |
| `eq-copilot/plugin/dsp/DspKern.cpp:788` | `dynamischeKennlinie (leistungInDb (z.huelle.leistung), …)` → `eq-copilot/plugin/dsp/DspFilter.h:420` `std::log10` |
| `eq-copilot/plugin/dsp/DspSvf.h:104`, `:116`, `:129` | `svfBell`, `svfLowShelf`, `svfHighShelf` rechnen je `std::pow (10.0, gainDb / 40.0)` |
| `eq-copilot/plugin/dsp/DspProgramm.h:222-223` | die Quelle widerspricht der Behauptung wörtlich: „nur `log10` und `pow` haengen an diesem Schritt, und genau ihre Kosten sind der Grund" |

**Reproduktion (in dieser Session gefahren).**
`eq-copilot/build/plugin/EqCopDspGoldenTest_artefacts/Release/EqCopDspGoldenTest.exe`
→ Exit 0, 389 geprüft, 0 Fehler; Zeile 196 der Ausgabe lautet
`ok 311/M-129 pegelstufe_rechnet_im_callback_nur_mult_und_add … [Rumpf von verarbeiteBand
9742 Zeichen, Aufrufe der Pegelstufe 1, kein Entwurf und keine transzendente Funktion; …]`.
Danach `grep -n "svfEntwurf\|leistungInDb" eq-copilot/plugin/dsp/DspKern.cpp` → `:788` und
`:791`, beide innerhalb des Rumpfes (`:661` bis `:847`). Ein grüner Kanon beglaubigt damit
einen Satz, den der Code derselben Etappe widerlegt.

**Was bricht.** Die Arbeitsregel, die der Dirigent in §62 zweimal als tragend benennt
(„gegen die Arbeitsregel, dass kein Text mehr behauptet, als gemessen ist"), und die
Matrixzeile M-129 in ihrem Wortlaut: deren Erwartung ist auf **die Stufe** begrenzt („Die
Stufe kostet je Sample und laufendem Detektor eine Multiplikation, eine Addition und einen
`double` Zustand; kein `log10`, kein `pow`"), die Behauptung B6 dehnt sie auf den ganzen
Rumpf aus. Das ist derselbe Befundtyp wie D-2 der Erstprüfung 5 („ein grüner Kanon
beglaubigt in der Behauptung B6 einen Satz, den der Satz D widerlegt hat"). Das
Prüfetikett selbst (`311/M-129 pegelstufe_rechnet_im_callback_nur_mult_und_add`) und die
Matrixzeile sind richtig; falsch sind der Behauptungssatz in `tools/beweise.ps1:723` und
die Ausgabezeile `DspGoldenTestMain.cpp:2260`. Produktverhalten ist nicht betroffen.

### LÜCKE

**L-1 — ein Wertsprung, der auf zwei verdrängte Publikationen verteilt ist, bleibt auf dem
Rampenweg.** `vergebeKennungen` vergleicht gegen die zuletzt PUBLIZIERTE Belegung
(`DspKern.cpp:324`, `:351-356`); eine verdrängte Zwischenpublikation schreibt den
Merkzettel fort (`:360`), und der Pool gibt die verdrängte Bank sofort frei
(`DspBankPool.cpp:154`). Publiziert der Worker ohne Audioblock dazwischen zweimal, je unter
dem Kriterium (zum Beispiel `freq_hz` 5000 → 2500 → 1250 Hz, Verhältnis je Schritt 2,0),
behält der Slot seine Kennung, und der Blockrand rampt von der gefahrenen Bank auf das
Endprogramm — mit einem Gesamtverhältnis von 4,0 über dem Kriterium. M-100 deckt die
Treppe mit Audio dazwischen, M-106 die Treppe über dem Kriterium; der Fall „Summe
verdrängter Schritte unter dem Kriterium" steht in keiner Zeile. Kein Bruch des
Grundgesetzes: der Ausschlag bleibt weit unter dem gemessenen Ausgangsfall (Verhältnis 100,
+17,41 dB), und die Grenze selbst ist nach F-20 ausdrücklich als „genau auf der Grenze
bleibt der Rampenweg" entschieden.

**L-2 — nach `releaseResources` tragen Kern und Bericht verschiedene abgelehnte Raten.**
`SondeProcessor::releaseResources` ruft `dspKern->freigeben()` (setzt `abgelehnteRate` auf
+0,0, `DspKern.cpp:181`), aber kein `transaktion->setzeSamplerate(...)`; `setzeSamplerate`
hat im Produktcode nur einen Aufrufer (`SondeProcessor.cpp:270`, `prepareToPlay`). Nach der
Folge `prepareToPlay (32000)` → `releaseResources` meldet `DspKern::abgelehnteSamplerateHz()`
also +0,0, `DspBericht::abgelehnteSamplerateHz` weiter 32 000. Der Header schränkt die
Zusage selbst ein („Dieselbe Zahl … solange beide dieselbe Rate gereicht bekommen",
`NakamaTransaktion.h:298-303`), und das neue Feld hat damit genau dieselbe Lebensdauer wie
das bestehende `fs`, das `releaseResources` ebenfalls nicht zurücknimmt. M-135, M-136,
M-143 und der Teilfall 311/M-132 messen jeweils nur den Weg über `prepareToPlay`
beziehungsweise nur den Kern; die Matrix sagt zum Paar nach `releaseResources` nichts.

### HÄRTUNG

**H-1 — `PegelZustand::tick` fängt keinen Pol ≥ 1.** `DspFilter.h:365` prüft nur
`! (pol > 0.0)`; ein Pol ≥ 1 machte das Mittel instabil. Über den Vertragsweg entsteht
keiner (`huellkurvePol` liegt für jede gültige Rate in [0, 1)), und die Heilung am
Blockrand fängt den Ausreißer, bevor ein Sample ihn liest.

**H-2 — die Rechenortwache von 311/M-129 folgt keiner Indirektion.**
`DspGoldenTestMain.cpp:2246` prüft sieben Zeichenketten im Rumpftext; ein Aufruf über einen
Helfer (`svfEntwurf`, `leistungInDb`) passiert sie. Das ist genau die Lücke, die D-1
sichtbar macht: die Wache ist für den Pegelbegriff richtig gebaut, trägt aber keine Aussage
über den ganzen Rumpf.

**H-3 — `bereiteVor` und `freigeben` nehmen den Merkzettel nicht zurück.** Heute ist das
folgenlos, weil dieselben Funktionen jeden Pfad in die Ruhe setzen und der Blockrand mit
`alt < 0` weder den Rampenweg noch die Zustandsübernahme betreten kann. Die Kopplung steht
damit an zwei Orten statt an einem; ein ausdrückliches `merkzettel[p].gueltig = false` in
beiden Funktionen machte sie lokal.

**H-4 — `setzeSamplerate` hat kein Gegenstück in `releaseResources`** (Gegenstück zu L-2).
`bereiteVor` ↔ `freigeben` sind im Kern als Paar gebaut; der Transaktionskern kennt nur die
Setzrichtung.

---

## Geprüft — selbst an der Quelle nachgemessen

Ich habe den vollständigen Diff des Prüfbereichs gelesen und zusätzlich die ZIEL-Fassungen
von `DspKern.h`, `DspKern.cpp`, `DspProgramm.h`, `DspProgramm.cpp`, `DspFilter.h`
(Pegelteil), `DspBankPool.cpp`, `SondeProcessor.cpp` (prepareToPlay, releaseResources,
reset, processBlock, Ladestart, Workerschleife), `NakamaTransaktion.h`/`.cpp` (geänderte
Teile) und `tools/fl/nulltest.py` (Vergleichs- und Urteilsteil, Selbsttestfälle) ganz
gelesen. Selbst gefahren, nacheinander und bei nachweislich ruhendem Rechner (kein cmake-,
MSBuild-, cl.exe- oder Testprozess; offen waren nur Dirigent, Cockpit, FL64 des
Laufzeit-Arms und die MCP-Server, die ich nicht angefasst habe):
`EqCopDspGoldenTest.exe` **Exit 0, 389 geprüft, 0 Fehler**; `EqCopProbeeqNullTest.exe`
**Exit 0, 131 Prüfungen ok, 0 Fehler**; `EqCopSchemaTest.exe` **Exit 0, 162 bestanden, 0
gescheitert**; `EqCopTransactionTest.exe` **Exit 0, 192 geprüft, 0 Fehler**. Vorher geprüft,
dass jedes Binary jünger ist als seine Quellen (20:15:50 / 19:43:56 / 20:35:54 / 20:36:05
gegen 20:14:05 / 19:43:48 / 12:05:53 / 16:06:13 und die dsp-Quellen 19:29:33 / 19:35:35).
Aus dem A16-Lauf dieser Session stammt die M-01-Messung (MXCSR `0x00009FE0`, FTZ 1, DAZ 1;
Summe veränderter Muster a 0, b 0, c 0). Selbst gemessen habe ich außerdem: den Diffumfang
und die Leerheit über `eq-copilot/schemas/state`, `eq-copilot/identity` und `eq-copilot`
außerhalb der Ticketpfade; die eine Schemazeile und die eine Fixturezeile; die Abwesenheit
der Ticketbegriffe in Schemas, Fixtures, `plugin/state` und `broker/src`; die Aufrufer von
`bereiteVor`, `setzeSamplerate`, `baueVor`, `publiziereVorbau`, `uebernehmeZustand`,
`publiziereWirksam` und `leiteAutoGainAb`; die Zahl und die Namen der Rotbeweisdateien
(139); und den Widerspruch von D-1 an den Zeilen `DspKern.cpp:788`, `:791`,
`DspFilter.h:420`, `DspSvf.h:104/116/129` und `DspProgramm.h:222-223`. HEAD habe ich vor
und nach dem Lauf gelesen.

## Nicht geprüft — nur gelesen oder ausgelassen

Nur gelesen, nicht nachgefahren: die Verhaltensmatrix (§6 M-01 bis M-83, §13/§16 M-90 bis
M-95, §39 in der Fassung nach §44 und §47 für M-96 bis M-120 und M-123 bis M-144) samt den
Rotbeweistabellen der Sätze A bis D, die Einordnung §62 und die Regeln R-311-1 bis
R-311-27; die Rohdateien habe ich als Beleg gelesen, nicht nachgerechnet. Die Herkunft der
Goldens (`kM61Bits` vom Basisstand `df14a648`, `kM133Goldens` von `5ee8318c`,
`kM100Golden` von `a93f3c79`) habe ich an den Kommentaren und den Rohdateien gelesen, aber
nicht selbst an diesen Ständen erzeugt — dazu hätte ich bauen müssen. Nicht gefahren habe
ich den vollen Kanon (69 Beine), die Hinweisbeine A32 und A33, die Korpusbeine A4, A5, A8
und A9, A1, B1, B2, B7s Prozessorteil über FL, den Laufzeit-Arm und `tools/fl/nulltest.py`
samt seinem Selbsttest A35 — dessen Logik habe ich am Quelltext gelesen und gegen die
Behauptung geprüft, aber nicht ausgeführt (er braucht Renderdateien). Gebaut, gestaged,
committet und gepusht habe ich nichts; FL64 des Laufzeit-Arms blieb unangetastet.
Geschrieben habe ich genau diese Datei.

HEAD am Ende: `4be1d193fd58b80d031d5fa94a6f384a2055246e` — unverändert.
