VALIDIERUNG: D-1 PRÄZISIERT · D-2 BESTÄTIGT · D-3 BESTÄTIGT · D-4 BESTÄTIGT · D-5 BESTÄTIGT

# NAK-311 — Validierung der Matrixprüfung 4 (Etappe 5, Planschritt S25j)

Frischer, lesender Opus-Thread, 20.09.2026. Gelesen und gerechnet, nicht
gebaut: kein Compiler, kein Produktbinary, kein Repo-Skript, keine Repo-Datei
geändert, nichts gestagt oder committet. Geschrieben wurde genau diese Datei.
Zeilennummern in `docs/beweise/NAK-311.md` und im Code gelten zu HEAD
`0eab7d1c`; der Code ist seit `34953955` unverändert (§38, Kopf).

---

## D-1 — M-103 (§39.1) und §40.3: der genannte „größte Bestandsfall auf dem Rampenweg"

### 1. Die betroffenen Sätze, wörtlich

`docs/beweise/NAK-311.md:6267` (Zusagespalte von M-103):

> „Der größte Bestandsfall auf dem Rampenweg ist `gain_db` −12 → 0 dB
> (Differenz 12,0 dB,
> `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:6145-6147`) und bleibt mit
> Abstand 1,5 unter der Grenze"

Rotbeweisspalte derselben Zeile (`:6267`):

> „**heute nicht messbar**; Rotbeweis: `kSprungGainDb` auf 10,0 gesetzt → der
> Bestandsfall −12 → 0 dB verlässt den Rampenweg → `311/M-100` und der
> M-37-Spiegel (`:6145-6160`) weichen vom Basisstand ab → rot"

`:6691-6693` (§40.3, Herleitung von `kSprungGainDb`):

> „Der größte bestehende Rampenfall ist `gain_db` −12 → 0 dB, also genau
> 12,0 dB (der M-37-Spiegel, `DspGoldenTestMain.cpp:6145-6147`); er behält
> Abstand 1,5."

`:6752-6753` (§40.3, Golden- und Fixture-Regel):

> „die drei bekannten Rampenfälle (1,4; 3; 6,0 dB) und der M-37-Spiegel
> (12,0 dB) liegen unter den Kriterien"

Die verletzte Matrixregel steht in `:530-531`:

> „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt, nie an einem
> Nebeneffekt."

### 2. Die Codebehauptungen, nachgemessen

Die Fundstelle `DspGoldenTestMain.cpp:6145-6147` stimmt für die zwei
Gainwerte, nicht für die Wegbehauptung. Am HEAD steht dort der Fall
`311/M-27` des Abschnitts P (`:6142-6164`), Kommentar `:6139-6141`
(„Spiegel des M-37-Pruefllings aus H"):

```cpp
6145	            belege (ag, 0, Filtertyp::highShelf, 20.0, 1.0, -12.0);
6146	            auto flach = ag;
6147	            flach.werte[(size_t) param::indexBandV1 (0, param::kGainDb)].zahl = 0.0;
…
6154	            const auto w = ausUndEin (fs, bs, mb, ag, { ohneEq (flach) }, flach, dc, 1024);
```

Der Helfer `ausUndEin` (`:5940-5966`) legt zwischen die zwei Gainwerte die
Ruhe:

```cpp
5947	            k->uebernehmeZustand (vorher);
5948	            fahre (*k, quelle, n, 1024, blockGroesse);
5950	            k->uebernehmeZustand (ohneEq (vorher));
5951	            auto lauf = fahre (*k, quelle, n, kFadeSamples + 512, blockGroesse);
5955	                k->uebernehmeZustand (s);
5961	            k->uebernehmeZustand (ziel);
```

Beide Sperren des Rampenwegs greifen. `rampenKompatibel`
(`eq-copilot/plugin/dsp/DspProgramm.cpp:212`):

```cpp
212	    if (! alt.eqEngagiert || ! neu.eqEngagiert || alt.hardBypass || neu.hardBypass) return false;
```

und der Blockrand (`eq-copilot/plugin/dsp/DspKern.cpp:492`, `:500-507`):

```cpp
492	    const DspProgramm* pAlt = alt >= 0 ? &baenke.bank (alt).programm : nullptr;
500	    const bool pfadGleich = pAlt != nullptr && pAlt->pfadKennung == pNeu.pfadKennung;
507	    const bool nurRampen = kennungenGleich && rampenKompatibel (*pAlt, pNeu);
```

Aus der Ruhe ist `alt < 0`, also `pAlt == nullptr`, `pfadGleich` falsch,
`nurRampen` falsch — Crossfade, kein Zustand übertragen. Zusätzlich wechselt
schon beim Gang in die Ruhe die Pfadkennung (`DspKern.cpp:273-275`), weil
`m.eqEngagiert == prog.eqEngagiert` reißt; jeder Slot bekommt dort ohnehin
eine neue Kennung. Der Fall nimmt nie den Rampenweg. Der Prüfer hat recht.

### 3. Eigene Herleitung und Rechnung

**(a) Nimmt der Fall je den Rampenweg?** Nein (Schritt 2). Zwischen den zwei
Gainwerten liegen drei Publikationen mit `eq_enabled = false`.

**(b) Welche Bestandsfälle nehmen am HEAD wirklich den Rampenweg mit einem
Gainwechsel?** Nachgezählt über jede Zuweisung an `kGainDb` und jeden
`belege`-Aufruf in den vier Testdateien:

- **Abschnitt E** (`:1937-2020`, M-43 bis M-45, M-54, M-55): kein Gainwechsel;
  geändert wird `v1.band.5.enabled` beziehungsweise der Typ von Slot 5.
  Bestätigt.
- **Abschnitt E, Rampenblock** (`:1873-1878`, `bandwert_<feld>_rampt_ohne_
  zustandsreset (M-17, R8, B-4)`, Prüfname `:1932`): `gain_db` 3,0 → 9,0 =
  **6,0 dB**, dazu `freq_hz` 1000 → 1400 (1,4) und `q` 1 → 3 (3,0).
- **Abschnitt O** (`:5670`, `:5686`): `basis` Bell 250 Hz Q 1,2 **+5,0 dB** →
  `rampe` **−2,0 dB** = **7,0 dB**, Kommentar `:5682` „nur kontinuierliche
  Werte: Rampenuebergang und globale Rampen". Die Fundstellen des Prüfers
  (`:5677`, `:5686`) sind zur Hälfte falsch: `:5677` ist `machDynamisch` an
  Slot 5, der +5 dB steht an `:5670`.
- **Abschnitt P**: jeder Fall läuft über `ausUndEin` und damit über die Ruhe —
  kein Rampenfall. Bestätigt.
- **Abschnitt Q** (`:6825`, `:6831`, M-94 (a)): der **Referenzkern** publiziert
  `nurWert` mit `gain_db` +12 → +6 = **6,0 dB** auf dem Rampenweg.
- **Abschnitt J** (`:4196-4228`, `uebernahme_nur_am_aeusseren_blockrand
  (M-25, R9, B-11)`): `ein` Slot 0 Low-Shelf 8000 Hz Q 0,707 **+9,0 dB**
  (`:4199`) → `anders` derselbe Slot, Typ, Modus, Frequenz und Güte,
  **−9,0 dB** (`:4205`) = **18,0 dB**. Topologie identisch, `eqEngagiert`
  beiderseits wahr, keine Dynamik: der Fall nimmt heute den Rampenweg.

**Der größte Rampenfall mit Gainwechsel in B6 ist damit 18,0 dB, nicht 7,0 dB
und nicht 12,0 dB.** Er liegt **genau auf** `kSprungGainDb` = 18,0; nur der
strikte Vergleich aus M-101/M-103 („Strikt größer entscheidet") hält ihn auf
dem Rampenweg. 9,0 und 18,0 sind in `double` exakt, der Abstand ist exakt 0.
Außerhalb von B6 reichen zwei Bestandsfolgen noch höher:
`TransactionTestMain.cpp:577` und `:1396` publizieren `gain_db` als
`(i % 24) − 12 + 0,25` beziehungsweise `+ 0,5`; am Umlauf i = 23 → 24
entsteht ein Sprung von **23,0 dB** über dem Kriterium. Der Kern ist dort
vorbereitet (`Stand`, `:136-143`), aber keine dieser Prüfungen vergleicht
Audio — sie prüfen Ring, Hash, Revision und die Wachen I1/I2/I4 (`:196-201`).
`SondeNullTestMain.cpp:203`/`:662` erreicht 15,0 dB, prüft aber Statebytes.

**(c) Bleibt unter der Mutation „`kSprungGainDb` auf 10,0" irgendetwas rot?**
Nein. Geprüft:

- `311/M-100` ändert nur `freq_hz` (`:6264`, „dieselbe Strecke 5000 → 50 Hz in
  20 gleichen Stufen") — von `kSprungGainDb` unabhängig.
- Der M-37-Spiegel läuft über die Ruhe (Schritt 2) — der Weg ändert sich nicht.
- Abschnitt O (7,0 dB), M-17 (6,0 dB), M-94 (a) (6,0 dB) und Abschnitt J,
  Auto-Gain-Zyklus (`:4013-4014`, 4,0 dB) liegen unter 10,0.
- Der 18,0-dB-Fall aus Abschnitt J verlässt bei 10,0 zwar den Rampenweg,
  **fällt aber nicht**: `abweichung` misst den ERSTEN Aufruf, in dem die
  Publikation noch nicht wirkt (`:4215-4218`), `uebernahmen()` steigt in
  beiden Wegen um 1, und `naechster.back()` ist Sample 1023 — Crossfade wie
  Rampe sind nach 256 Samples eingeschwungen, die Schranke ist 1e−4
  (`:4223-4224`).
- Der eigene Test von M-103 bildet die drei Kanten symbolisch aus der
  Konstanten (`:6267`: „`gain_db` springt auf −12 + d dB mit d =
  `kSprungGainDb` − 1 ULP, genau `kSprungGainDb` und + 1 ULP") und bleibt bei
  jedem Wert der Konstanten grün.
- Keine geplante Zeile M-96 bis M-111 trägt einen Gainwechsel: M-96 bis M-98,
  M-100, M-101, M-104 bis M-108 und M-110 fahren Frequenzsprünge, M-102 die
  Güte, M-109 nicht endliche Werte, M-111 misst Allokationszähler und den
  Diff über `blockrand`.

**Das Gainkriterium hat damit keinen fallenden Rotbeweis.**

**(d) Dasselbe für M-101 und M-102.** Beide fallen, weil ihre Mutation
**strukturell** ist, nicht ein Zahlenwert:

- M-101 (`:6265`): „der Vergleich auf `>=` statt `>` gesetzt → der Fall genau
  auf der Grenze verlässt den Rampenweg → rot". Der eigene Test fährt die
  Kante „genau auf der Grenze" und verlangt dort den Rampenweg — er fällt,
  gleich welchen Wert `kSprungFrequenzVerhaeltnis` trägt. Fällt an der Zusage.
- M-102 (`:6266`): „das Gütekriterium aus `bleibt` entfernt → der Sprung
  1 → 24 bleibt auf dem Rampenweg → rot". Der eigene Test fährt die Kante
  `+ 1 ULP` über der Grenze und verlangt dort die neue Kennung; unter der
  Mutation bleibt sie aus → rot. Fällt an der Zusage. *Einzelheit:* der in der
  Rotbeweisspalte genannte Prüfling „1 → 24" steht nicht in der
  Ereignisspalte von M-102 (dort `q` = 1,0 mal v mit v = 4,0 ± 1 ULP,
  abwärts von `q` = 4,0); gefallen wäre die Zeile über ihre eigene
  `+ 1 ULP`-Kante.
- Der **Wert** der Frequenzkonstanten ist zusätzlich durch M-100 gewacht
  (Mutation auf 1,2 unter dem Stufenverhältnis 1,2589 → Bitvergleich fällt).
  Für Güte und Gain gibt es keine entsprechende Wertwache in der Matrix.

### 4. Gegenargument gesucht

Drei Wege geprüft, keiner trägt:

1. *Fängt ein Satz des Nachtrags den Fall schon auf?* §40.3 „Risiken und
   Grenze", Risiko 1 (`:6757-6760`) nennt „ein Bestandsfall … verlässt still
   den Rampenweg" und wehrt es mit Zeilenvergleich und `git grep` ab. Das ist
   eine Abwehr gegen unbemerkte Wegwechsel **beim gebauten Wert**, kein
   Rotbeweis für das Kriterium.
2. *Macht die Mutation doch irgendwo rot?* Vollständig durchgezählt in
   Schritt 3 (c) — nein, auch nicht über den 18,0-dB-Fall.
3. *Trägt die Zahl trotzdem?* Die Aussage „alle Bestandsfälle liegen unter
   18,0 dB" bleibt wahr, aber nur mit Abstand **0** statt 1,5 — und der
   18,0-dB-Fall ist in M-103 und §40.3 nicht genannt.

### 5. Urteil

**PRÄZISIERT.** Der Defekt hält in beiden Teilen: der zitierte Fall ist kein
Rampenfall, und der Rotbeweis von M-103 fällt an nichts. Anders ist die
Einzelheit des Nebenbefunds: der größte wirkliche Rampenfall mit Gainwechsel
ist nicht Abschnitt O mit 7,0 dB, sondern Abschnitt J mit **18,0 dB**
(`DspGoldenTestMain.cpp:4199`, `:4205`) — er liegt exakt auf der
vorgeschlagenen Grenze. Das verschärft den Befund.

### 6. Gebrochene Sätze und kleinste Korrektur

Gebrochen: `:6267` (Zusage von M-103), `:6691-6693` und `:6752-6753` (§40.3)
sowie die Matrixregel `:530-531`.

**(i) Zahlen und Fundstellen berichtigen.** In M-103 und in §40.3 wird „Der
größte Bestandsfall auf dem Rampenweg ist `gain_db` −12 → 0 dB (Differenz
12,0 dB, `…:6145-6147`) und bleibt mit Abstand 1,5 unter der Grenze" ersetzt
durch: „Der größte Bestandsfall auf dem Rampenweg ist `gain_db` +9 → −9 dB
(Differenz 18,0 dB, `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4199`,
`:4205`, `uebernahme_nur_am_aeusseren_blockrand (M-25, R9, B-11)`); er liegt
GENAU auf der Grenze und bleibt nur durch den strikten Vergleich auf dem
Rampenweg." In der Golden-Regel `:6752-6753` tritt dieser Fall neben die drei
Rampenfälle; der M-37-Spiegel entfällt dort, weil er über die Ruhe läuft.
Begründung an der Quelle: Schritt 2 und 3 (b).

**(ii) Folge für die Zahl 18,0 (Entscheid des Dirigenten, §42.2).** Mit
Abstand 0 zu einem Bestandsfall trägt `kSprungGainDb` = 18,0 keinen Rand. Zwei
Wege stehen offen, beide sind Vorschlag: die Zahl anheben (etwa auf 20,0, dann
bleibt der 18,0-dB-Fall mit Abstand 1,11 darunter und der Vertragsrand 24 dB
bleibt darüber), oder sie bei 18,0 belassen und den Bestandsfall in M-103 und
§40.3 als Kante ausdrücklich benennen. Die Messreihe aus §42.2 („der reine
Gainsprung −12 → +12 dB") entscheidet es.

**(iii) Ein Prüfling und eine Mutation je Kriterium, die an der eigenen Zusage
fallen.** Vorschlag, in der Form, die M-101 und M-102 schon tragen:

- *Frequenz (M-101):* unverändert — Mutation `>=` statt `>`, fällt an der
  Kante „genau auf der Grenze" des eigenen Tests. Zusätzlich wacht M-100 über
  den Wert.
- *Güte (M-102):* unverändert — Mutation „Gütekriterium aus `bleibt`
  entfernt", fällt an der eigenen `+ 1 ULP`-Kante. Der Prüfling „1 → 24" in
  der Rotbeweisspalte wird auf die Kante der eigenen Ereignisspalte
  umgeschrieben.
- *Gain (M-103), neue Rotbeweisspalte:* „das Gainkriterium aus der
  `bleibt`-Bedingung in `vergebeKennungen`
  (`eq-copilot/plugin/dsp/DspKern.cpp:286-288`) entfernt → der Sprung
  −12 → −12 + `kSprungGainDb` + 1 ULP behält seine Kennung, `nurRampen` bleibt
  wahr und der Crossfade läuft nicht → `311/M-101` Teilfall `311/M-103` rot".
  Deterministisch und unabhängig vom Wert der Konstanten.
- *Wert des Gainkriteriums, zusätzliche Wache* (Muster M-100, als Teilfall von
  `311/M-100` oder als eigene Zeile): Prüfling ist der bestehende Rampenfall
  `gain_db` 3 → 9 dB (`DspGoldenTestMain.cpp:1875`), Mutation
  „`kSprungGainDb` auf 5,0 gesetzt" → der Fall verlässt den Rampenweg →
  `bandwert_gain_db_rampt_ohne_zustandsreset (M-17, R8, B-4)` (`:1931-1933`)
  vergleicht seinen Tap gegen die im Test ausgeschriebene lineare
  Koeffizientenrampe und reißt seine Schranke 1e−5 → rot. Das ist die Zeile,
  die die Zusage „Eine Änderung unter dem Kriterium … bleibt auf dem
  Rampenweg, bitgleich zum Basisstand" (R-311-13) trägt. Dieselbe Wache für
  die Güte: Mutation `kSprungGueteVerhaeltnis` auf 2,0 → `bandwert_q_…`
  (Verhältnis 3,0) fällt.

---

## D-2 — §40.4 und §39.3: der neue Pegelzustand wird beim Abschalten des Detektors nicht zurückgesetzt

### 1. Die betroffenen Sätze, wörtlich

`docs/beweise/NAK-311.md:6782-6784` (§40.4, Ticketpfade):

> „- `eq-copilot/plugin/dsp/DspKern.h` und `DspKern.cpp`: das Feld im
> `SlotMerkmal`, `merke`, `vergebeKennungen` und `verarbeiteBand` (`:664-687`,
> eine Zeile vor `z.huelle.tick`)."

`:6780-6781` (§40.4, Ticketpfade):

> „- `eq-copilot/plugin/dsp/DspBankPool.h`: der Pegelzustand im `BandZustand`
> samt `nullen`, `istEndlich` und `riegleDenormale`."

`:6881` (§40.4, Änderungen je Datei):

> „`verarbeiteBand` schiebt genau eine Zeile zwischen Detektor und Hüllkurve."

Die weiter geltende Zusage, `docs/beweise/SONDE-015.md:3423` (E-29):

> „Ein Detektor, der am Ende eines Fensters nicht läuft, wird genullt." …
> „Das Nullen macht „danach ist der Detektor aus" zu einem Zustand statt zu
> einem eingefrorenen Pegel: eine spätere Rampe von 0 weg beginnt wie ein
> frischer Detektor, nicht bei einem Pegel von damals."

W-3 (`:3741`) führt dieselbe Zusage als geschlossenen Befund.

### 2. Die Codestelle, nachgemessen

`eq-copilot/plugin/dsp/DspKern.cpp:737-745`:

```cpp
737	    // W-3 (E-29): ein Detektor, der nicht mehr laeuft, haelt keinen Zustand.
738	    // Nach einer Rampe der Range auf 0 ist er damit wirklich aus, und eine
739	    // spaetere Rampe von 0 weg beginnt wie ein frischer Detektor bei 0 statt
740	    // bei einem Pegel von damals.
741	    if (b.nutztSvf && ! detektorZuletzt)
742	    {
743	        z.huelle.nullen();
744	        for (auto& d : z.detektor) d.nullen();
745	    }
```

Genullt wird **gliedweise**: `HuellkurveZustand::nullen()` (`DspFilter.h:295`)
und die zwei Detektor-Biquads. `BandZustand::nullen()`
(`DspBankPool.h:72`) wird an dieser Stelle nicht gerufen. Der Pegelzustand
läge nach §40.4 als eigenes Glied im `BandZustand` neben `huelle`
(`DspBankPool.h:58-78`) und bliebe damit stehen. Die Fundstellenangabe des
Prüfers ist richtig.

Die Ticketpfade nennen `:664-687` — den Detektorblock (`:664` `if
(detektorAktiv)` bis `:687` schließende Klammer). `:741-745` liegt außerhalb
und kommt in keinem Satz des Bauplans vor. `:6881` schließt eine zweite
Änderung sogar ausdrücklich aus („genau eine Zeile").

### 3. Eigene Herleitung

Die Stufe sitzt nach F-17 (`:6971-6985`) zwischen Detektor und Hüllkurve:
Bandpass → Pegelbegriff → Hüllkurve → Kennlinie. Wird der Bauplan wörtlich
ausgeführt, gilt nach einer Rampe der Range auf 0:

1. `detektorZuletzt` ist falsch, `z.huelle.leistung` und die zwei
   Detektorzustände werden 0 (`:741-745`).
2. Der Pegelzustand behält den Wert aus dem letzten Fenster und klingt mit
   `kPegelFensterMs` nicht ab, weil er gar nicht mehr getickt wird — die
   Stufe läuft nur im `detektorAktiv`-Zweig.
3. Bei der nächsten Rampe von 0 weg ist `detektorAktiv` wieder wahr. Die
   Detektor-Biquads beginnen bei 0, ihr Ausgang wächst von 0 an, die
   Pegelstufe gibt aber im ersten Sample `pol · alterPegel + (1 − pol) · ~0`
   aus — praktisch den alten Pegel. `HuellkurveZustand::tick`
   (`DspFilter.h:312-318`) sieht `leistungEin > leistung` (0) und nimmt den
   Attack-Zweig auf diesen alten Pegel zu.

Genau der Zustand, den E-29 ausschließt: „eine spätere Rampe von 0 weg beginnt
… bei einem Pegel von damals."

**Misst eine Zeile M-120, M-123 bis M-131 den Fall „Range auf 0 gerampt, danach
wieder von 0 weg"?** Nein:

- M-120 (`:6318`), M-124 (`:6320`): eingeschwungener Betrieb, keine
  Range-Rampe.
- M-123 (`:6319`): Pegelbegriff `spitze`, Stufe kurzgeschlossen — der Zustand
  wird nie beschrieben.
- M-125 (`:6321`), M-126 (`:6322`), M-129 (`:6325`): Programmbau, Statebytes,
  Echtzeit.
- M-127 (`:6323`): Sprungantwort aus der Stille, keine Range-Rampe.
- M-128 (`:6324`): bestehende Prüfungen, unverändert.
- M-130 (`:6326`): (a) nicht endlich und denormal — Heilung am Blockrand
  (`DspKern.cpp:560-578`) und `riegleDenormale` (`DspFilter.h:299-303`); ein
  **endlicher, stehengebliebener** Pegel wird davon nicht berührt. (b) ±0, (c)
  Raten, (d) Thresholdränder.
- M-131 (`:6327`): topologische Übertragung.

Auch der Bestandstest fällt nicht. `range_minus_12_nach_0_rampt_ueber_die_
volle_rampe (M-17, R8, B-4, W-3)` (`DspGoldenTestMain.cpp:2534-2578`) liest
ausschließlich die Hüllkurvenleistung:

```cpp
2564	            const double leistungNachRampe = k->pool().bank (cA).baender[0].huelle.leistung;
2569	            const double leistungSpaeter = bn.baender[0].huelle.leistung;
```

Beide werden von `:743` auf 0 gesetzt. Der Test fährt außerdem je Lauf nur
**eine** Richtung (`richtung` 0 oder 1, `:2534-2536`) — die Folge „auf 0, dann
wieder von 0 weg" kommt in keinem Bestandsfall vor.

### 4. Gegenargument gesucht

- *Fängt §40.4 es über `DspBankPool.h` auf?* Nein: `:6780-6781` gibt dem
  `BandZustand` zwar `nullen`, aber `:741-745` ruft `BandZustand::nullen()`
  nicht.
- *Fängt M-130 (a) es auf?* Nein: der stehengebliebene Wert ist endlich und
  nicht denormal.
- *Nennt §38.3 E-29 als berührte Zusage?* Nein. §38.3 (`:6070-6119`) führt
  Entwurf `:2984`, R7 Feinheit 3, M-18, M-19, M-26, E-25, M-27, E-6, NAK-331
  und B6 — E-29 kommt nicht vor.
- *Bleibt die Wirkung hörbar?* Ja: nach dem Attack-Sprung liest
  `dynamischeKennlinie` (`DspKern.cpp:705`) einen zu hohen Pegel und lenkt das
  Band aus, bis die Hüllkurve nachgeführt hat.

### 5. Urteil

**BESTÄTIGT.**

### 6. Gebrochener Satz und kleinste Korrektur

Gebrochen ist E-29 / W-3 (`SONDE-015.md:3423`, `:3741`), die der Nachtrag
nicht als verschoben benennt; verletzt ist damit auch R-311-15 letzter Satz
(„Jede Bestandszeile …, dessen Zahl sich dadurch verschiebt, wird im Nachtrag
einzeln benannt").

**(i) Bauplan.** In §40.4 wird die Zeile `:6782-6784` ergänzt um
`verarbeiteBand` (`eq-copilot/plugin/dsp/DspKern.cpp:741-745`): der
Pegelzustand wird dort zusammen mit `z.huelle` und den Detektor-Biquads
genullt. Der Satz `:6881` wird von „schiebt genau eine Zeile zwischen Detektor
und Hüllkurve" zu „schiebt eine Zeile zwischen Detektor und Hüllkurve und
nullt den Pegelzustand in `:741-745` mit `z.huelle` und dem Detektor (E-29)".

**(ii) Matrixzeile mit Rotbeweis.** M-130 bekommt einen Teilfall, oder — weil
die Beziehung aktivieren↔abklingen eine eigene Zeile verdient (§6,
`:544-548`) — es entsteht eine neue Zeile im Abschnitt 39.3:

- *Zustand:* der Prüfstand von M-120, ein dynamisches Bell, Range −12 dB,
  Material L = R-Sinus weit über Threshold plus Knie.
- *Ereignis:* Range −12 → 0 rampen, `kRampeSamples` + 4096 Samples Stille im
  Detektorfenster, danach Range 0 → −12 rampen.
- *Zusage:* im ersten Steuerschritt nach der zweiten Rampe ist der Pegel exakt
  +0,0 und die Auslenkung folgt derselben ausgeschriebenen Idealrampe wie im
  Bestandsfall `range_0_nach_minus_12_rampt_ueber_die_volle_rampe`; der
  Pegelzustand des Bandes ist nach der ersten Rampe bitgenau +0,0 (gelesen wie
  `:2564` über `pool().bank(cA).baender[0]`).
- *Rotbeweis (heute nicht messbar, der Zustand entsteht im Bau):* den
  Pegelzustand aus dem Nullen in `DspKern.cpp:741-745` herausgelassen → die
  frisch genullte Hüllkurve bekommt als ersten Eingang den alten Pegel, springt
  mit der Attack darauf und reißt die Idealrampe im ersten Steuerschritt → rot.

Beides berührt keine Zahl eines Bestandsgoldens: der Pegel ist nach dem
Nullen 0, und `range_minus_12_nach_0_…` misst genau das schon für die
Hüllkurve.

---

## D-3 — M-127 (§39.3) und §40.4: die Absolutzahl der Sprungantwort

### 1. Die betroffenen Sätze, wörtlich

`docs/beweise/NAK-311.md:6323` (Zusagespalte von M-127):

> „Die Absolutzahlen wachsen: bei Attack 20 ms und `kPegelFensterMs` 10 ms
> liegt der 63-%-Punkt der Reihe bei rund 27 ms statt 20 ms. Hold und Release
> messen vom Plateau aus und wachsen um weniger als 1 ms"

`:6852` (§40.4, Verschiebungstabelle, Zelle „Richtung"):

> „**größer**: zwei Pole in Reihe; bei Attack 20 ms und Fenster 10 ms liegt
> der 63-%-Punkt bei rund 27 ms"

### 2. Der Prüfstand, nachgemessen

`eq-copilot/plugin/tests/DspGoldenTestMain.cpp:2375-2435`, tragende Zeilen:

```cpp
2376	            const double attackMs = 20.0, holdMs = 30.0, releaseMs = 50.0, toleranzMs = 1.0;
2377	            const double thresh = -30.0;
2378	            const double pss = std::pow (10.0, (thresh + 12.0) / 10.0);
2380	            const double attackSchwelle  = -(12.0 + 10.0 * std::log10 (1.0 - std::exp (-1.0)));
2405	                    if (aus[i] <= attackSchwelle) { nAttack = i; break; }
2419	            const double soll[3] = { attackMs, holdMs, releaseMs };
2426	                    if (std::abs (zeiten[ri][st] - soll[st]) > toleranzMs) innerhalb = false;
```

`attackSchwelle` = −10,00800 dB entspricht in der Kennlinie genau
`leistung = (1 − 1/e) · P_ss` — der 63-%-Punkt der Reihe. Die Rekursionen:
`huellkurvePol` (`DspFilter.h:273-278`, `exp(−1/(fs·τ))`) und
`HuellkurveZustand::tick` (`:312-328`).

### 3. Eigene Rechnung

Nachgefahren mit denselben Rekursionen, Quadraturton (konstante
Detektorleistung `P_ss`), Steuerraster `kDynamikSchritt` = 8, je einmal ohne
und einmal mit vorgeschaltetem Ein-Pol von 10 ms:

| Rate | heute A / H / R (ms) | mit 10-ms-Pegelpol A / H / R (ms) | Δ A |
|---|---|---|---|
| 44,1 kHz | 20,14 / 30,20 / 49,89 | **31,75** / 30,20 / 50,43 | +11,61 |
| 48 kHz | 20,00 / 30,17 / 49,83 | **31,83** / 30,17 / 50,50 | +11,83 |
| 96 kHz | 20,00 / 30,17 / 49,83 | **31,75** / 30,17 / 50,50 | +11,75 |
| 192 kHz | 20,00 / 30,12 / 49,88 | **31,71** / 30,12 / 50,50 | +11,71 |

Analytisch für die Kaskade zweier Pole: 1 − 2e^(−t/τ_a) + e^(−t/τ_m) = 1 − 1/e
mit τ_a = 20 ms, τ_m = 10 ms ergibt u = 1 − √(1 − 1/e) = 0,20494 und
t = −20·ln(u) = **31,70 ms**. Ohne die Rasterung auf `kDynamikSchritt` liefert
die Simulation 31,69 ms.

Der 63-%-Punkt liegt bei **31,7 ms**, nicht bei rund 27 ms; der Zuwachs ist
**11,8 ms**, nicht etwa 7 ms. Die Zahlen des Prüfers sind reproduziert.

Der zweite Satz derselben Zusage hält: Hold +0,00 ms, Release +0,54 bis
+0,67 ms über alle vier Raten, beides unter 1 ms. Die Spanne der vier
Attackwerte bleibt mit 0,12 ms unter der 1-ms-Schranke von
`dieselbe_ms_angabe_ergibt_bei_jeder_rate_dieselbe_zeit`.

**Folge für die Neubelegung von E-25 (§40.4) und für t_E.** §40.4 schreibt
richtig, dass die im Test ausgeschriebene Referenz den Pegelpol als zweite
Stufe bekommt und die Toleranz 1 ms je Stufe bleibt. Mit der Zahl „rund 27 ms"
als Referenz läge die Messung 4,8 ms daneben — fast das Fünffache der eigenen
Toleranz; der Test fiele nach dem Bau aus dem falschen Grund. Die Neubelegung
muss also 31,7 ms (analytisch) beziehungsweise die je Rate gemessenen 31,71
bis 31,83 ms tragen. Für **t_E** in `311/M-77` und `311/M-78` (§40.4,
`:6858`: „größer um höchstens 5·`kPegelFensterMs`·fs Samples") folgt nichts
Gegenteiliges: 5·τ_m = 50 ms bleibt eine gültige obere Schranke für die
Verschiebung des Einschwingpunkts, und die hier gemessene Verschiebung des
63-%-Punkts (11,8 ms) liegt weit darunter. Der Fehler von D-3 steckt allein in
der Absolutzahl der Sprungantwort.

### 4. Gegenargument gesucht

- *Steht „rund 27 ms" unter einem anderen Kriterium?* Nein: beide Stellen
  nennen ausdrücklich den 63-%-Punkt bei Attack 20 ms und Fenster 10 ms.
- *Trägt der Bandpass-Einschwinger die Differenz?* Nein, er wirkt in die
  andere Richtung und ist klein: τ ≈ Q/(π·f0) ≈ 0,225 ms bei 1 kHz, Q 0,707.
- *Rettet die Toleranz die Zahl?* Nein: 31,7 − 27 = 4,7 ms gegen 1 ms
  Toleranz.
- *Ist die Stelle als Schranke statt als Ergebnis gemeint?* §39 verlangt
  (`:6242-6244`) für erst zu messende Zahlen ausdrücklich die Form „Schranke,
  nie Ergebnis"; „liegt bei rund 27 ms" ist eine Ergebnisaussage.

### 5. Urteil

**BESTÄTIGT.**

### 6. Gebrochener Satz und kleinste Korrektur

Gebrochen ist der Satz „bei Attack 20 ms und `kPegelFensterMs` 10 ms liegt der
63-%-Punkt der Reihe bei rund 27 ms statt 20 ms" (`:6323`) und seine
Wiederholung in `:6852`.

Kleinste Korrektur, an beiden Stellen dieselbe Zahl: „bei Attack 20 ms und
`kPegelFensterMs` 10 ms liegt der 63-%-Punkt der Reihe bei **rund 31,7 ms**
statt 20 ms (analytisch 1 − 2e^(−t/τ_a) + e^(−t/τ_m) = 1 − 1/e mit τ_a = 20 ms
und τ_m = 10 ms; diskret an den Rekursionen von `DspFilter.h:273-278` und
`:312-328` 31,71 bis 31,83 ms über 44,1 / 48 / 96 / 192 kHz)". Der zweite Satz
(„Hold und Release … wachsen um weniger als 1 ms") bleibt wörtlich; gemessen
sind +0,00 und höchstens +0,67 ms.

---

## D-4 — §40.2 Nr. 3: die Abstandsrechnung zur Deckelhöhe

### 1. Der betroffene Satz, wörtlich

`docs/beweise/NAK-311.md:6576-6578`:

> „**Nakama hebt automatisch nie weiter an, als der User selbst aufdrehen
> kann.** Das liegt mit Faktor 3,4 über der +7,0 dB aus (1) und mit Faktor 15
> unter der kleinsten gemessenen Gefahr aus (2)."

Die Bezugszahl steht in `:6568-6572` (§40.2 Nr. 2): „Die gemessenen Extreme
liegen bei +42,99 dB … und +150,46 dB (Q 0,15)".

### 2. Die Codebehauptung, nachgemessen

`eq-copilot/schemas/state/nakama-parameter-v2.json:52-58`:

```json
      "id": "v1.global.output_trim_db",
      "typ": "float",
      "einheit": "dB",
      "min": -24.0,
      "max": 24.0,
```

Die Fundstelle und die Zahl +24,0 stimmen.

### 3. Eigene Rechnung

Abstand zwischen +24 dB und +42,99 dB in jeder sinnvollen Lesart:

| Lesart | Wert |
|---|---|
| Verhältnis der dB-Zahlen | 42,99 / 24 = **1,791** |
| Abstand in dB | 42,99 − 24 = **18,99 dB** |
| linearer Amplitudenfaktor | 10^(18,99/20) = **8,902** |
| Leistungsfaktor | 10^(18,99/10) = **79,25** |

Keine Lesart ergibt 15. Die einzige 15 in der Nähe ist der lineare Faktor des
Deckels selbst, 10^(24/20) = **15,849** — in M-112 (`:6291`) korrekt als
„10^(24/20) = 15,849-mal" benannt.

Die übrigen Zahlen der Herleitung habe ich unabhängig nachgerechnet und
bestätigt: Low-Cut 80 Hz entfernt log2(4)/log2(1000) = 20,07 % → +0,9728 dB;
Low-Cut 500 Hz 46,60 % → +2,7244 dB; High-Cut 2 kHz 33,33 % → +1,7609 dB;
beide zusammen lassen 20,07 % stehen → +6,9748 dB; 24 / 6,9748 = **3,441**,
also „Faktor 3,4" richtig.

**Hält das Maß von R-311-14 mit der berichtigten Zahl?** Ja. R-311-14 verlangt
(`:5803-5804`): „Die Höhe liegt über jeder Anhebung normaler Arbeit und unter
dem, was Wiedergabegeräte gefährdet." 24 dB liegt 17,0 dB über der +7,0 dB aus
(1) und 18,99 dB unter der kleinsten gemessenen Gefahr aus (2) — beide
Richtungen mit Rand. Nur die Zahl der Herleitung trägt nicht.

### 4. Gegenargument gesucht

- *Meint „Faktor 15" den Abstand zum zweiten Extrem +150,46 dB?* Auch nicht:
  150,46 / 24 = 6,27 als dB-Verhältnis, 126,46 dB Abstand, 10^(126,46/20) =
  2,1·10^6 linear.
- *Meint er den Faktor des Deckels selbst?* Der ist 15,849, aber er beschreibt
  den Deckel, nicht den Abstand zur Gefahr — und der Satz sagt ausdrücklich
  „unter der kleinsten gemessenen Gefahr aus (2)".
- *Ist „Faktor" hier auf Gitterenergie bezogen?* Dann wäre es 79,25.

### 5. Urteil

**BESTÄTIGT.**

### 6. Gebrochener Satz und kleinste Korrektur

Gebrochen: „und mit Faktor 15 unter der kleinsten gemessenen Gefahr aus (2)"
(`:6577-6578`).

Kleinste Korrektur: „und mit **18,99 dB** (linear Faktor 8,9, in der Leistung
79,3) unter der kleinsten gemessenen Gefahr aus (2)." Wenn die Form „Faktor …"
erhalten bleiben soll: „mit Faktor **8,9** (linear) unter der kleinsten
gemessenen Gefahr aus (2)". Das Maß der Regel hält mit beiden Fassungen; die
Zahl 24,0 selbst bleibt unberührt.

---

## D-5 — M-99 und M-97 (§39.1): das Etikett „heute nicht messbar"

### 1. Die betroffenen Sätze, wörtlich

Die Etikettendefinition, `docs/beweise/NAK-311.md:515-517`:

> „**heute nicht messbar**: der Mechanismus oder das Urteil entsteht erst im
> Bau; die Zeile nennt die Mutation nach dem Bau."

Zum Vergleich `:513-515`:

> „**heute rot**: am Basis-SHA widerspricht das Verhalten der Zusage
> nachweislich; die Zeile nennt die Gegenprobe (Datei:Zeile oder Rohbeleg) und
> die Mutation, die nach dem Bau an der neuen Zusagezeile rot macht."

M-97 (`:6261`), Rotbeweisspalte:

> „**heute nicht messbar** — das Urteil für diese Richtung ist am Basisstand
> nicht erhoben: Phase 15 hat nur die zwei Abwärtsfälle gemessen (+17,41 und
> +22,20 dB), die Arbeitsübersicht W07 führt die Rückrichtung als Prüfweg ohne
> Zahl (`docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md:810`). Der Fall läuft
> nach §7.1 vor der Reparatur einmal gegen den unveränderten Kern, und die
> Rohdatei hält fest, ob er dort fällt."

M-99 (`:6263`), Rotbeweisspalte:

> „**heute nicht messbar** — der Fall ist am Basisstand nicht gemessen (die
> Arbeitsübersicht führt ihn als Gegenweg, `BEFUNDE.md:810`); der SVF-Weg
> interpoliert `grundG` und `q` je Steuerschritt linear
> (`eq-copilot/plugin/dsp/DspKern.cpp:690-710`) und kann denselben oder einen
> größeren Ausschlag tragen."

§39.6 (`:6383-6385`): „heute rot 7 … heute nicht messbar 19: M-97, M-99,
M-101 bis M-104, M-108, M-109, M-113, M-115 bis M-118, M-125, M-127, M-130,
M-131, M-134, M-139."

### 2. Die Codebehauptungen, nachgemessen

`eq-copilot/plugin/dsp/DspProgramm.cpp:318`:

```cpp
318	        b.detektorLaeuft = b.nutztSvf && detektorGewuenscht && b.rangeDb != 0.0;
```

Bei Range 0 ist `detektorLaeuft` falsch. `nutztSvf` (`:295`) ist
`dynAn && typHatGain (b.typ)`; `typHatGain` (`eq-copilot/plugin/dsp/DspSvf.h:
149-152`) liefert für `lowShelf` **wahr**:

```cpp
151	    return t == Filtertyp::bell || t == Filtertyp::lowShelf || t == Filtertyp::highShelf;
```

Der Prüfling von §39.1 (`:6249-6250`) ist ein Low-Shelf +6 dB Q 0,707 — M-99
ist damit über den SVF-Weg erreichbar, genau wie die Zeile es beschreibt. Der
Auditwortlaut `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md:810` lautet in
der Spalte Prüfwege wörtlich: „Shelf 5 kHz→50 Hz und Rückrichtung, 0,5 s
Ausgang; dynamisch Range 0 und 20 kleine Schritte als Gegenwege." `:633`
(T3-15-07) fordert dieselben zwei Gegenfälle.

### 3. Eigene Herleitung

**Sind M-97 und M-99 am HEAD mit vorhandenen Mitteln messbar?** Ja, mit
demselben Aufbau, aus dem M-96 und M-98 ihre +17,41 und +22,20 dB haben:

- Der Prüfling steht vollständig (48 kHz, Blockgröße 64, Slot 0, 1-kHz-Sinus
  0,5, eine Publikation mit `freq_hz`-Sprung); die Richtung ist ein Vorzeichen.
- `machDynamisch` mit Range 0 ist im Bein B6 vorhanden (Muster
  `DspGoldenTestMain.cpp:5677`, `:6821`).
- Die Übergangsspitze über 0,5 s ist die Messgröße aus §39 (`:6212-6217`) und
  wird am Ausgang abgelesen.
- Weder der Mechanismus noch das Urteil entsteht im Bau: beide Fälle laufen
  heute über `rampenKompatibel` als Rampe, und das Maß (Spitze gegen die
  größere stationäre Spitze) ist heute bildbar.

Beide Zeilen sind **ungemessen**, nicht unmessbar. M-97 widerlegt das Etikett
im eigenen Text, indem es den §7.1-Lauf vor der Reparatur ankündigt — das ist
genau das Verfahren, das §7.1 für „heute rot" vorschreibt.

**Welche weiteren der neunzehn Zeilen sind nach derselben Definition messbar?**
Jede der übrigen siebzehn durchgeprüft — keine:

- M-101 bis M-103: die drei Konstanten und die Vergleiche entstehen im Bau.
- M-104: „nur der gesprungene Slot startet kalt" entsteht im Bau (heute
  behalten alle Slots ihren Zustand).
- M-108: acht neue Kennungen entstehen im Bau.
- M-109: die drei Vergleiche und ihre Endlichkeitswache entstehen im Bau.
- M-113, M-115 bis M-118: Deckelfunktion, zweiter Zweig, Zustand, Getter und
  Berichtsfeld entstehen im Bau.
- M-125, M-127, M-130, M-131: Pegelbegriff, Reihenreferenz, Pegelzustand und
  seine Topologie entstehen im Bau.
- M-134: `kMinSamplerateHz` und das Prädikat entstehen im Bau.
- M-139: der frühe Rückweg wird heute bei 32 kHz nie genommen.

Betroffen sind also genau M-97 und M-99, wie der Prüfer sagt.

### 4. Gegenargument gesucht

Das stärkste Gegenargument: die Etappe 5 verwendet nur vier der sieben
Etiketten (`:6204-6208`), und „heute rot" verlangt ein **nachweisliches**
Widersprechen. Ohne Messung darf der Bauer „heute rot" nicht behaupten — es
bleibt ihm kein passendes Etikett für „messbar, aber ungemessen". Das erklärt
die Wahl, rechtfertigt sie aber nicht: §6 bindet „heute nicht messbar" auf
„der Mechanismus oder das Urteil entsteht erst im Bau", und beides ist hier
falsch. Der Weg aus der Lage steht in §7.1 selbst („der Test wird vor der
Reparatur geschrieben und einmal rot gefahren") und in M-97s eigenem Text.

Ein zweites Gegenargument: M-97 ist Teilfall von `311/M-96`, das „heute rot"
trägt — das Etikett der Mutterzeile deckt den Teilfall aber nicht, denn M-97
hat eine eigene Etikettenzelle und wird in §39.6 eigens gezählt.

### 5. Urteil

**BESTÄTIGT.**

### 6. Gebrochene Sätze und kleinste Korrektur

Gebrochen ist die Etikettendefinition `:515-517` und damit die Zählung in
§39.6 (`:6383-6389`).

**(i) Etiketten.** M-97 und M-99 bekommen **heute rot**, mit derselben
Gegenprobe wie M-96: „Gegenprobe: `rampenKompatibel` kennt keine Wertegrenze
(`eq-copilot/plugin/dsp/DspProgramm.cpp:207-227`) und `vergebeKennungen`
vergleicht nur Topologiefelder (`DspKern.cpp:286-288`); der Sprung bleibt
heute auf dem Rampenweg. Die Zahl ist am Basisstand nicht erhoben und wird
nach §7.1 vor der Reparatur gemessen; die Rohdatei
`docs/beweise/roh/NAK-311-rot-M-97.txt` beziehungsweise `…-M-99.txt` hält sie
fest. Nach dem Bau: Mutation wie M-96 beziehungsweise Kriterienprüfung auf
`! b.dynamisch` eingeschränkt → rot." M-99 bekommt dabei den Satz über den
§7.1-Lauf, den M-97 schon trägt.

**(ii) Ablauf „vor der Reparatur einmal rot".** §40.3, Absatz „Rotbeweise nach
§7.1" (`:6741-6744`): „„Heute rot" sind M-96 und M-98" wird zu „„Heute rot"
sind M-96, M-97, M-98 und M-99"; für M-96 und M-98 bleiben +17,41 und
+22,20 dB die Erwartung aus Phase 15, für M-97 und M-99 ist die Erwartung die
Schranke selbst (Spitze höchstens 1 dB über der größeren stationären Spitze),
und die gemessene Zahl geht in die Rohdatei. Fällt einer der zwei Fälle dort
**nicht**, ist das ein Befund an den Dirigenten: die Zeile wird dann
Regressionswache, nicht stillschweigend umetikettiert.

**(iii) Zählung §39.6.** „heute rot **9**: M-96, M-97, M-98, M-99, M-112,
M-120, M-124, M-132, M-135. heute nicht messbar **17**: M-101 bis M-104,
M-108, M-109, M-113, M-115 bis M-118, M-125, M-127, M-130, M-131, M-134,
M-139." Regressionswache 15 und zwei Hälften 2 bleiben, Summe 43 bleibt. Der
Schlusssatz „Am Basis-SHA stehen damit 7 Zeilen ganz rot" wird zu „9 Zeilen".

---

## Fakten zu den Lücken

### L-1 — Bandmitten von M-124 und der Rest des 10-ms-Fensters

M-124 (`:6320`) misst ausschließlich bei **f0 = 1 kHz Q 0,707**, L = R-Sinus,
Threshold im Knie, neun Kombinationen aus Attack 0,1 / 10 / 500 ms und Hold
0 / 30 / 500 ms. Eine zweite Bandmitte kommt in der Zeile nicht vor; M-120
(`:6318`) fährt als einzige Zeile auch f0 = 20 Hz.

Restwelligkeit r = 1/(4π·f0·τ_m) mit τ_m = 10 ms und die Schranke des Versatzes
10·log10(1 + r), selbst nachgerechnet:

| f0 | r | Schranke |
|---|---|---|
| 20 Hz | 0,397887 | **1,4547 dB** |
| 100 Hz | 0,079577 | **0,3325 dB** |
| 341 Hz | 0,023337 | **0,1002 dB** |
| 1 kHz | 0,007958 | **0,0344 dB** |

M-124 verlangt „höchstens 0,1 dB auseinander". Bei 1 kHz lässt das Fenster
0,034 dB gleichrichtbaren Versatz zu, die Schranke klemmt dort also nicht; bei
20 Hz wären es 1,45 dB gegen heute 3,01 dB (10·log10 2, der
Spitzen-/Effektivwertabstand eines Sinus). Ein Fenster, das auch bei 20 Hz
unter 0,1 dB bliebe, müsste 170,8 ms lang sein — die Zahl aus §40.4 Nr. 4 ist
nachgerechnet richtig.

### L-2 — Material der zwei in M-128 genannten Prüfungen

`detektor_ist_bandbegrenzt (M-21)` (`DspGoldenTestMain.cpp:2225-2227`) und
`detektor_hoert_vor_dem_band (M-21)` (`:2238-2241`) fahren beide einen
**L = R-Sinus**, kein Quadraturpaar (`:2216-2218`: `a[i] = b[i] = (float) x`),
einmal bei 1000 Hz, einmal bei 60 Hz neben der Bandmitte 1 kHz Q 4.

Ihre Zahlen und ihr Urteil verschieben sich trotzdem nicht: beide messen im
**Plateau** beziehungsweise gegen eine lose Verhältnisschranke. Bei Amplitude
0,9 liegt die Detektorleistung −3,9 dB, Threshold −40 dB, Kniebreite 12 dB →
`min(1, …)` klemmt auf die volle Range (`innen` = −9,0 dB exakt,
`mitAbsenkung` = −12,0 dB exakt); der 60-Hz-Ton liegt mit rund −77 dB weit
unter Threshold → `aussen` = 0,0 exakt. Der Pegelbegriff ändert daran nichts.
Die **Begründung** von M-128 („deren Material ein Quadraturton auf der
Bandmitte im eingeschwungenen Zustand ist") trägt sie also nicht.

Dasselbe gilt für zwei weitere, in keiner der beiden Listen genannten
Prüfungen des Abschnitts F: `dynamische_kennlinie_golden_plateau (M-18)`
(`:2149-2151`) und `auslenkung_bleibt_innerhalb_range (M-18)` (`:2152-2153`)
fahren ebenfalls einen L = R-Sinus (`:2142-2143`) und halten nur, weil die
Kennlinie im Plateau klemmt. In keiner der beiden Listen stehen außerdem
`range_null_rechnet_keinen_detektor (M-20)` (`:2067`),
`huellkurvenwert_<feld>_holdzaehler_rundet_exakt (NAK-289)` (`:2521-2523`) und
`range_…_rampt_ueber_die_volle_rampe (M-17, R8, B-4, W-3)` (`:2572-2578`).
M-128 nennt neun Prüfungen, davon zwei nur als Umschreibung („die
Plateauprüfung, die Selektivitätsprüfung"); §40.4 führt sechs Tabellenzeilen
aus B6 als verschoben. Für die zwei Umschreibungen kommen fünf Prüfungen des
Abschnitts F in Frage (`dynamische_kennlinie_golden_plateau`,
`auslenkung_bleibt_innerhalb_range`,
`plateau_wirkt_zusaetzlich_zu_gain_db_im_audiopfad`,
`detektor_ist_bandbegrenzt`, `detektor_hoert_vor_dem_band`) — unter jeder
Lesart bleiben mindestens drei Prüfungen des Abschnitts F in beiden Listen
ungenannt.

### L-3 — `$comment` von `auto_gain_db` und der Wert des Drahtfelds

`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:2325`, wörtlich:

> „ABGELEITET, nie persistent und nie im state_hash (R4). Er wird immer
> gerechnet, angewandt aber nur bei v2.global.auto_gain = true. Die Grenze ist
> eine Berichtsgrenze: der Bericht klemmt auf +/-120 (NAK-311 R-311-5); die
> Ableitung selbst kann darueber liegen (acht Low-Shelves 1 kHz +12 dB Q 8:
> -199.77 dB)."

Nach dem Bau trüge das Drahtfeld auf der **Anhebungsseite** den gedeckelten
Wert: der Deckel sitzt nach F-23 (`:7026-7029`) an den zwei Rückgabezeilen von
`leiteAutoGainAb` (`DspProgramm.cpp:163`, `:203`), `baueBericht` baut sein
eigenes Programm mit derselben Funktion
(`eq-copilot/plugin/state/NakamaTransaktion.cpp:817`) und gibt es durch
`berichtsAutoGainDb` (`:822`), das jeden Wert innerhalb ±120 bitgleich
durchlässt (`:783-788`). Für den Prüfling aus M-112 stünde im Feld also
**+24,0** statt +150,46 — auf der Absenkungsseite bleibt alles wie heute
(−199,77 → −120, M-114 und M-67). Der `$comment` beschreibt dann nur noch die
Absenkungsseite richtig. §38.2 (`:6011-6014`) führt diese Zeilen als
„unverändert", und M-140 (`:6358`) verlangt einen leeren Diff über
`eq-copilot/schemas`.

### L-4 — Kanalzahl und Ausgleichsmeldung im dritten Fenster

`freigeben` (`eq-copilot/plugin/dsp/DspKern.cpp:127-154`) setzt zusätzlich
`kanalzahl = 2` (`:140`) und setzt `autoGainBericht` **nicht** zurück;
`bereiteVor` (`:97-125`) setzt `kanalzahl = kanaele` (`:100`), nullt
`autoGainBericht` (`:120`) und ruft `zaehlerZuruecksetzen()` (`:124`).
`freigeben` ruft es nicht. `DspKern::autoGainDb()` liest genau dieses Feld
(`DspKern.h:312`: `return autoGainBericht[0].load (…)`).

§40.1 Nr. 2 (`:6492-6497`) zählt für den neuen Weg auf: „Puffer leeren,
`abtastrate` auf 0, Bänke zurücksetzen, Pfade in Ruhe, Zähler zurück" — die
Kanalzahl und `autoGainBericht` kommen darin nicht vor, und „Zähler zurück"
gehört zu `bereiteVor`, nicht zu `freigeben`. M-132 (`:6345`) verlangt
„`DspKern::autoGainDb()` ist exakt +0,0": nach einem Wechsel 48 → 32 kHz mit
zuvor gefahrenem Programm hinge das daran, ob der neue Weg `:120` mitnimmt.

§40.1 Nr. 3 (`:6498-6501`): „Die Kanalzahl bleibt in beiden Fällen unverändert
übernommen (R-311-3, R-311-12)." `Transaktionskern::setzeSamplerate`
(`NakamaTransaktion.cpp:350-357`) übernimmt sie heute unbedingt (`:356`).
R-311-12 (`:5737-5741`) sagt dagegen: „Die Kanalzahl folgt auf jedem Weg der
Samplerate: … bei `maxBlock == 0` oder verriegelter Samplerate gibt
`prepareToPlay` weder Rate noch Kanalzahl weiter." Im neuen, dritten Fenster
reicht `prepareToPlay` beides durch (`sichereRate > 0` und `maxBlock > 0`
bleiben nach §40.1 Nr. 5 unverändert), `fs` wird 0 und die Kanalzahl wird
genommen — dem **Wortlaut** „auf jedem Weg" widerspricht das, der Begründung
von R-311-12 (zwei benannte Fenster, kein klingender Block) nicht.

---

## Beifang

- `DspGoldenTestMain.cpp:4196-4228` ist ein Bestandsfall auf dem Rampenweg mit
  **18,0 dB** Gainwechsel und liegt damit exakt auf `kSprungGainDb` = 18,0
  (siehe D-1 Schritt 3 b). Das berührt unmittelbar die geprüfte Zeile M-103
  und die Zahlenwahl aus §42.2.
- `TransactionTestMain.cpp:577` und `:1396` publizieren Gainsprünge von
  **23,0 dB** am Umlauf `i % 24`; sie liegen nach dem Bau über dem Kriterium.
  §40.3 Risiko 1 und der dort zugesagte `git grep` fangen sie ab, eine Zeile
  benennt sie nicht.

---

## Selbst nachgemessen

Am HEAD gelesen und geprüft: `eq-copilot/plugin/dsp/DspKern.cpp`
(`:97-125` `bereiteVor`, `:127-154` `freigeben`, `:220-226` `meldeProgramm`,
`:238-259` `merke`, `:261-292` `vergebeKennungen` mit `:273-275` und
`:286-288`, `:450-558` `blockrand` mit `:460`, `:491-507`, `:509-541`,
`:543-547`, `:560-583` `heileZustaende`, `:655-745` `verarbeiteBand` mit
`:660`, `:686`, `:690-710`, `:737-745`, `:747-754`); `DspKern.h` (`:312`,
`:315`, `:541`); `DspProgramm.cpp` (`:76-83`, `:85-204` `leiteAutoGainAb` mit
`:96`, `:115-164`, `:200-204`, `:207-227` `rampenKompatibel`, `:230-340`
`baueProgramm` mit `:240`, `:286-300`, `:318`, `:326-333`, `:339-340`);
`DspFilter.h` (`:260-288`, `:290-329`, `:331-341`); `DspSvf.h` (`:145-152`);
`DspBankPool.h` (`:51-90`); `NakamaTransaktion.cpp` (`:350-357`, `:777-789`,
`:792-836`); `eq-ipc-v3.schema.json` (`:2312-2345`, insbesondere `:2315-2316`,
`:2324-2329`, `:2331-2336`); `nakama-parameter-v2.json` (`:48-60`).
In den Testdateien: `DspGoldenTestMain.cpp` (`:171-179` `belege`, `:447-478`
`fahreStereoTon`, Abschnitt E `:1792-2021` mit `:1873-1935`, Abschnitt F
`:2022-2732` vollständig auf Prüfnamen und Material, Abschnitt J `:3987-4267`
mit `:3991-4031` und `:4196-4228`, Abschnitt L `:4506-5306` an `:4864-4930`,
Abschnitt N `:5430-5651` an `:5470-5493`, Abschnitt O `:5652-5820` an
`:5664-5729`, Abschnitt P `:5821-6459` an `:5930-5975` und `:6130-6180`,
Abschnitt Q `:6460-6948` an `:6810-6860`); `TransactionTestMain.cpp`
(`:128-144`, `:196-230`, `:568-594`, `:888-919`, `:1140-1185`, `:1387-1408`,
`:2160-2235`); `SondeNullTestMain.cpp` (`:195-205`, `:650-690`); dazu ein
`grep` über jede Zuweisung an `kGainDb` und jeden `belege`-Aufruf in den vier
Testdateien.

Selbst gerechnet (Python nur für Formeln und Rekursionen, keine Repo-Skripte):
die Sprungantwort A/H/R einmal analytisch über die Kaskade zweier Pole und
einmal als diskrete Simulation von `huellkurvePol` und
`HuellkurveZustand::tick` bei 44,1 / 48 / 96 / 192 kHz, mit und ohne
10-ms-Pegelpol, einschließlich der Kennlinie, des Steuerrasters
`kDynamikSchritt` = 8 und der zwei Schwellen aus `:2380-2381`; die
Restwelligkeit r und 10·log10(1 + r) bei 20 / 100 / 341 / 1000 Hz; die
171-ms-Alternative; die Rauschbandbreite B_n = 2221,8 Hz, σ = 0,150 und
10·log10(1 + 3σ) = 1,614 dB; der Spitzen-/Effektivwertabstand 3,0103 dB; die
vier Lesarten des Abstands +24 dB zu +42,99 dB; die vollständige
Deckelherleitung aus §40.2 Nr. 1 und Nr. 3; die Wertverhältnisse und
-differenzen aller gefundenen Bestandsfälle auf dem Rampenweg.

Gelesen als Bezug: `docs/beweise/NAK-311.md` §6 (`:491-552`), §36 (`:5713-5773`),
§37 (`:5774-5835`), §38 (`:5836-6191`), §39 (`:6192-6411`), §40 (`:6412-6924`),
§41.2 (`:6966-7071`), §42 (`:7208-7267`), §7.1; `docs/beweise/SONDE-015.md`
(`:3413`, `:3423`, `:3741`, `:3765`);
`docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (`:632`, `:633`, `:810`);
`docs/beweise/roh/NAK-311-matrixpruefung-4-urteil.md` ganz.

## Nur gelesen / nicht geprüft

Nichts gebaut und nichts gefahren: jede Aussage über „rot" und „grün" ist aus
der Quelle und aus eigener Rechnung geschlossen, nicht aus einem Lauf. Die
Zahlen aus Phase 15 (+17,41 / +22,20 / +150,46 / +42,99 / +2,12 / +3,01 /
+8,66 dB) habe ich nicht neu erhoben, sondern nur ihre Verwendung geprüft;
`docs/beweise/roh/NAK-311-quellvalidierung-teil-B.md`,
`roh/phase-16-d-dsp.md`, `roh/NAK-311-etappe4-autogain-hex.txt` und die vierzehn
Hexwerte habe ich nicht gelesen — sie tragen keine der fünf Behauptungen. Die
Abschnitte A bis D und G bis I von B6 habe ich nur auf Gainwechsel
durchgesehen, nicht auf ihre übrigen Zusagen; `SchemaTestMain.cpp` nur an
`:1437`. Die Härtungen H-1 bis H-4, die Zahlen der Regeln als solche (§42.2),
Prosa und Zählungen außerhalb der berührten Zusagen, T3-12-*, Legacy-Bezeichner
und Design sind nach Auftrag ausgeschlossen und nicht bewertet. Die Einordnung
Defekt, Lücke oder Härtung trifft der Dirigent; alle Korrekturen in Schritt 6
und alle Vorschläge zu den Lücken sind Vorschlag, kein Entscheid.

---

HEAD zu Beginn: `0eab7d1c9b556d33e853ae32552622ca3a21bb0c`.
HEAD am Ende: `0eab7d1c9b556d33e853ae32552622ca3a21bb0c`.
