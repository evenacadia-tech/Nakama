D-1: BESTAETIGT — D-2: PRAEZISIERT

# NAK-311 Erstpruefung 5 — Quellvalidierung der zwei DEFEKTE

Validierer: frischer, lesender Opus-Thread. Nur Quellenlage, keine Einordnung
als Defekt, Luecke oder Haertung. Datum 20.09.2026.

HEAD zu Beginn und am Ende: `4ffdc70a4460ae5e3c151589291379575e77c7ca`.
Nichts gebaut, nichts gestagt, nichts committet, nichts gepusht.

Vorher geprueft: kein `cmake`, kein `MSBuild`, kein `vctip`, kein `EqCop*`
lief (`Get-Process` leer). Danach nacheinander gefahren:

- `eq-copilot/build/plugin/EqCopDspGoldenTest_artefacts/Release/EqCopDspGoldenTest.exe`
  — Exit 0, `389 geprueft, 0 Fehler`, `DSP-GOLDEN OK`
- `eq-copilot/build/plugin/EqCopProbeeqNullTest_artefacts/Release/EqCopProbeeqNullTest.exe`
  — Exit 0, `SONDE-NULLTEST OK - 131 Pruefungen ok, 0 Fehler`

Dieselben Zahlen wie im Urteil der Erstpruefung 5 und in dessen Kopfteil.

---

## D-1 — BESTAETIGT

**Befundkern: 311/M-133 hat fuer seine zweite Haelfte keinen Riegel, waehrend
Matrix, Runner und Pruefetikett einen behaupten.**

Jede Tatsachenbehauptung des Befundes haelt an der Quelle am HEAD. Zusaetzlich
faellt der Befund schaerfer aus als geschrieben (Punkt 6).

### 1. Der Fingerabdruck fliesst nicht in die Bedingung

`eq-copilot/plugin/tests/SondeNullTestMain.cpp:1669`:

```
            const auto abdruck = fnvAusgang (*p, 40, 512, (int) rate + 133);
```

Die geprueften Bedingungen, `:1676-1678`:

```
            pruefe (e.ausgang == tx::Ausgang::commit && hoerbar
                        && kern.samplerate() == rate
                        && abgelehnt == 0.0 && ! std::signbit (abgelehnt),
```

`abdruck` steht nicht darin. Sein einziger weiterer Gebrauch ist die
Detailzeichenkette, `:1683`:

```
                    "Ausgang FNV-1a 0x" + juce::String::toHexString ((juce::int64) abdruck)
```

Beleg der Vollstaendigkeit:
`grep -n "abdruck" eq-copilot/plugin/tests/SondeNullTestMain.cpp` liefert genau
`:175` (Doc-Kommentar), `:1662` (Blockkommentar), `:1669`, `:1681`, `:1683`.
`git grep -n "fnvAusgang" -- eq-copilot/plugin/tests/SondeNullTestMain.cpp`
liefert `:180` (Definition) und `:1669` (einziger Aufruf).

### 2. Das Pruefetikett behauptet die ungemessene Haelfte

`eq-copilot/plugin/tests/SondeNullTestMain.cpp:1681-1682`:

```
                          "abgelehnteSamplerateHz ist exakt +0,0, und der Ausgang traegt den Fingerabdruck "
                          "des Basisstands",
```

Der Lauf gibt diesen Satz als `ok` aus (A16, Ausgabezeilen 303-306).

### 3. Die brechende Zusage, woertlich

`docs/beweise/NAK-311.md:6385`, Matrixzeile M-133 in Abschnitt 39.4,
Zusagespalte:

> **44 100 Hz bleibt unterstützt: der Kern bereitet sich vor, das Band ist
> hörbar, und der Ausgang ist bitgleich zum Lauf am Basis-SHA der Etappe
> (memcmp). `abgelehnteSamplerateHz()` ist exakt +0,0. Dasselbe gilt für 48, 96
> und 192 kHz, die A16 und B6 heute schon fahren**

Riegelspalte derselben Zeile:

> A16 bestehend (die Ratenschleife 44,1 / 48 / 96 kHz in
> `eq-copilot/plugin/tests/SondeNullTestMain.cpp:523`, dazu 192 kHz in `:379`
> und `:399`; Fundstelle berichtigt in der Matrixnacharbeit 3, §44), **NEU** um
> den Bitvergleich gegen den Basisstand ergänzt

`tools/beweise.ps1:688`, Behauptung von A16, woertlich:

> Genau 44 100 Hz und der naechste double darueber bleiben unterstuetzt - das
> Band ist hoerbar, und der Ausgang traegt bei 44,1 / 48 / 96 / 192 kHz
> denselben Fingerabdruck wie am Basisstand des Aenderungssatzes -, waehrend
> 44 100 minus 1 ULP, 44 099, 32 000, 22 050, 8 000, 0, negativ, NaN und +-Inf
> abgelehnt werden.

Gemessen wird davon nur die erste Haelfte. Die Zeichenkette `(memcmp)` der
Zusagespalte trifft ausserdem die Bauart nicht: gemessen wuerde ein FNV-1a-Hash
ueber die Ausgangsbytes (`SondeNullTestMain.cpp:180-205`), kein `memcmp`.

### 4. Keine Goldenkonstante im Baum

Mein Lauf gibt die vier Werte aus (A16, Zeilen 303-306):
`0xcb14d6eee50e7d7c` (44,1 kHz), `0xbd787728731ba929` (48 kHz),
`0x5ece3bcf185baff5` (96 kHz), `0x33f35b42b6693e06` (192 kHz) — identisch zu
den vom Pruefer genannten.

`git grep -l` auf jeden der vier Werte liefert je dieselben fuenf Dateien und
keine Quelldatei:

```
docs/beweise/NAK-311.md
docs/beweise/roh/NAK-311-1166409-dirty.md
docs/beweise/roh/NAK-311-a5815df-dirty.md
docs/beweise/roh/NAK-311-etappe5-a16-zeilenvergleich.txt
docs/beweise/roh/NAK-311-rot-M-133.txt
```

Gegenprobe ohne Git ueber die Arbeitskopie:
`grep -ril "cb14d6eee50e7d7c" eq-copilot tools broker design wissen` — leer.
In `SondeNullTestMain.cpp` gibt es ueberhaupt keine Goldenkonstante:
`grep -n "kM1\|Golden\|golden"` ueber die Datei liefert nichts.

### 5. Keine widerlegende Stelle

- **Kein zweiter Vergleich des Fingerabdrucks.** `fnvAusgang` hat genau einen
  Aufrufer (Punkt 1).
- **Kein anderer Basisvergleich im Nulltest.** Alle `memcmp`-Stellen der Datei
  (`:334`, `:356`, `:468`, `:496`, `:530`, `:1079`, `:1090`, `:1238`, `:1266`,
  `:1268`, `:1582`, `:1639`, `:1718`, `:1767`) vergleichen Ein- gegen Ausgang,
  Tap gegen Sollwert oder zwei Werte desselben Laufs — keiner einen
  Basisstand.
- **Das Muster existiert zweimal richtig, aber in B6, nicht in A16.**
  `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:1003` setzt
  `constexpr std::uint64_t kM100Golden = 0xB754D7603137E70Bull;` (dezimal
  13210420415127086859, nachgerechnet) und `:1055` prueft `fp == kM100Golden`
  in der `pruefe`-Bedingung; `:2097` setzt
  `constexpr std::uint64_t kM123Golden = 4420950042542505610ull;` und `:2125`
  prueft `unterSpitze == kM123Golden`. Beide messen aber andere Konfigurationen
  (Low-Shelf-Treppe am Kern beziehungsweise Pegelbegriff `spitze`), nicht den
  Prozessorausgang von M-133.

### 6. Praezisierung, die den Befund verschaerft

Der Pruefer schreibt, die Rohdatei weise den Bitvergleich als Ablesen von Hand
aus. Das stimmt woertlich — `docs/beweise/roh/NAK-311-rot-M-133.txt`:

> Der BITVERGLEICH gegen den Basisstand laeuft ueber den Fingerabdruck des
> Ausgangs (FNV-1a ueber die Bytes jedes ausgegebenen float, 40 Bloecke zu 512
> Samples, feste Saat). Er ist am Zwischenstand VOR der Reparatur und am
> Endstand NACH der Reparatur gleich:

Verglichen werden dort **Zwischenstand und Endstand**, beide innerhalb des
Aenderungssatzes — nicht der Basisstand `5ee8318c`.

Auch der Zeilenvergleich liefert ihn nicht.
`docs/beweise/roh/NAK-311-etappe5-a16-zeilenvergleich.txt` fuehrt den
Basislauf in Abschnitt 2 (`:37-331`). Die vier Hexwerte und jede M-133-Zeile
stehen dort **nicht**; die Fundstellen sind `:25-28` (Diffhaelfte „nur im
Endstand", Zeichen `>`), `:635-638`, `:964-967`, `:1290-1293`, `:1620-1623` —
allesamt Endstaende der Saetze A bis D. Der Basislauf hat 294 Zeilen, der
Endstand 313; `311/M-133` ist eine der neuen Zeilen.

Damit gilt: die Zusage „bitgleich zum Lauf am Basis-SHA der Etappe" ist weder
im Riegel noch von Hand belegt. Es existiert kein Lauf am Basisstand, gegen den
verglichen werden koennte.

### 7. Der kleinste Fix, und alle Stellen, die denselben Satz tragen

Der Fix zerfaellt in zwei Faelle, weil kein Basislauf existiert (Punkt 6). Die
Wahl zwischen ihnen ist ein Entscheid, den ich nicht treffe.

**Fall A — Zusage halten.** Den Nulltest am Basisstand `5ee8318c` mit der
M-133-Messung fahren, die vier Fingerabdruecke aufschreiben (Muster von
`311/M-100`: Golden aus dem unveraenderten Kern, Herkunft in einer Rohdatei),
als `constexpr`-Konstanten in `SondeNullTestMain.cpp` aufnehmen und in die
`pruefe`-Bedingung `:1676-1678` legen. Braucht einen Bau.

**Fall B — Zusage auf das Gemessene zuruecknehmen.** Die vier heutigen
Fingerabdruecke als Regressionswache **ab diesem Aenderungssatz** einfrieren
und in `:1676-1678` legen; die Woerter „Basisstand", „Basis-SHA" und „(memcmp)"
fallen. Braucht keinen Basislauf, gibt die Aussage ueber den Basisstand auf.

Sachlage dazu: die M-133-Belegung ist ein statisches Bell 14 kHz +12 dB Q 4 mit
Auto-Gain (`SondeNullTestMain.cpp:1534-1545`, `mitBell14k`) ohne
`dynamic_enabled`. Satz B greift erst ueber +24 dB (gemessen -0,76 bis
-1,39 dB), Satz C nur bei einer Wertaenderung, Satz D nur im dynamischen Weg.
Dass die vier Werte denen des Basisstands gleichen, ist damit plausibel —
belegt ist es nicht.

Stellen, die denselben Satz tragen (`git grep`):

| Stelle | Art | Was zu aendern ist |
|---|---|---|
| `eq-copilot/plugin/tests/SondeNullTestMain.cpp:1676-1678` | Bedingung | Vergleich aufnehmen |
| `eq-copilot/plugin/tests/SondeNullTestMain.cpp:1681-1682` | Pruefetikett | Wortlaut an den Fall anpassen |
| `eq-copilot/plugin/tests/SondeNullTestMain.cpp:175-179` | Doc-Kommentar zu `fnvAusgang` | traegt „Bitvergleich gegen den Basisstand" |
| `eq-copilot/plugin/tests/SondeNullTestMain.cpp:1662-1664` | Blockkommentar | traegt denselben Satz |
| `tools/beweise.ps1:688` | Behauptung A16 | Wortlaut |
| `docs/beweise/NAK-311.md:6385` | Matrixzeile M-133, §39.4 | Zusage- und Riegelspalte, append-only |

Nur Verlauf, nicht anzufassen: `docs/beweise/NAK-311.md:10013` und `:11901`
(Kanon-Uebersichten, Abschnitte „Uebersicht" ab `:9974` und `:11862`),
`docs/beweise/roh/NAK-311-1166409-dirty.md`,
`docs/beweise/roh/NAK-311-a5815df-dirty.md`,
`docs/beweise/roh/NAK-311-etappe5-a16-zeilenvergleich.txt`,
`docs/beweise/roh/NAK-311-rot-M-133.txt`.

---

## D-2 — PRAEZISIERT

**Befundkern haelt: der Zusagetext von 311/M-73 bis 311/M-75 ist am ZIEL
falsch, und zwei Stellen des Pruefbereichs tragen ihn unveraendert weiter.**
Vier Angaben des Befundes sind zu berichtigen oder zu ergaenzen.

### 1. Die brechende Zusage, woertlich, und wo sie steht

`tools/beweise.ps1:723`, Behauptung B6 — genau der vom Pruefer zitierte Satz,
woertlich bestaetigt:

> Seit NAK-311 Etappe 4 Teil b (T3-15-11) ist die Steuerrate gemessen statt
> begruendet: gegen einen Referenzkern mit Range 0 (derselbe SVF-Weg) bleibt
> der Tap post_committed nach dem Einsatz eines Quadraturtons bis
> einschliesslich Sample d = (8 - p) mod 8 bitgleich und weicht ab d + 1 ab;
> der Entwurf faellt auf Sample d und wirkt ab d + 8 mit Gewicht 1 - dort
> traegt svfVon genau den bei d entworfenen Satz. Die erste Wirkung liegt damit
> 1 bis 8, die volle Wirkung des ersten Entwurfs 8 bis 15 Samples nach dem
> Einsatz: 0,167 bis 0,3125 ms bei 48 kHz, 0,181 bis 0,340 ms bei 44,1 kHz und
> 0,083 bis 0,156 ms bei 96 kHz […]

`eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4691-4693`, Pruefetikett:

```
                            + " (NAK-311 T3-15-11): der Tap bleibt bis einschliesslich Sample "
                              "d = (8 - p) mod 8 bitgleich zum Referenzkern, der Entwurf faellt auf d "
                              "und wirkt ab d + 8 mit Gewicht 1 - erste Wirkung 1 bis 8, volle 8 bis 15 Samples",
```

**Praezisierung 1 (Fundstelle).** Der Pruefer schreibt „Wortgleich in der
Matrixzeile M-73 (§9)". Die Matrixzeile M-73 steht in **Abschnitt 6.4**
(`docs/beweise/NAK-311.md:685`, Ueberschrift `:659`
„### 6.4 Etappe 4 — Auto-Gain im Monobus, Berichtsgrenze, Steuerrate, Recall").
Abschnitt 9 (`:1387` „## 9. Offene Punkte (Etappe 1, Bauer, 19.09.2026)")
enthaelt unter F-15 (`:1624-1628`) keinen der brechenden Saetze — dort steht
nur, dass `kDynamikSchritt` 8 bleibt und die Begruendung berichtigt wird.

**Praezisierung 2 (Wortlaut).** „Wortgleich" trifft auf `tools/beweise.ps1:723`
zu, nicht auf die zwei anderen Traeger. Die Matrixzeile `:685` sagt sinngleich,
aber anders gebaut:

> **Der Tap `post_committed` (bei Hörmatrix Processed gleich dem Ausgang)
> bleibt bis einschließlich Sample d = (8 − p) mod 8 nach dem Einsatz bitgleich
> zum Referenzkern und weicht ab Sample d + 1 ab; der Entwurf fällt auf Sample
> d (`schrittRest` ist nach Sample d gleich 7, gelesen am Bandzustand) und
> wirkt ab d + 8 mit Gewicht 1. Die erste Wirkung liegt damit 1 bis 8 Samples,
> die volle Wirkung des ersten Entwurfs 8 bis 15 Samples nach dem Einsatz =
> 0,167 bis 0,3125 ms**

Das Pruefetikett `:4691-4693` traegt **weder** „und weicht ab d + 1 ab" **noch**
„dort traegt svfVon genau den bei d entworfenen Satz". Der Pruefer legt ein
Zitat ueber zwei verschieden lautende Texte.

### 2. Die Messung am ZIEL

Mein Lauf, B6-Ausgabezeilen 184 bis 186, vollstaendig:

```
311/M-73 (48 kHz):
  p=0 d=0 Bezugspunkt 8 erste Abweichung 9 (soll 9) volle Wirkung 16
  p=1 d=7 Bezugspunkt 7 erste Abweichung 8 (soll 8) volle Wirkung 15
  p=3 d=5 Bezugspunkt 5 erste Abweichung 6 (soll 6) volle Wirkung 13
  p=7 d=1 Bezugspunkt 1 erste Abweichung 2 (soll 2) volle Wirkung 9
311/M-74 (44,1 kHz): dieselben vier Zeilen
311/M-75 (96 kHz):
  p=0 d=0 Bezugspunkt 8 erste Abweichung 9 (soll 9) volle Wirkung 16
  p=1 d=7 Bezugspunkt 7 erste Abweichung 8 (soll 8) volle Wirkung 15
  p=3 d=5 Bezugspunkt 5 erste Abweichung 6 (soll 6) volle Wirkung 13
  p=7 d=1 Bezugspunkt 9 erste Abweichung 10 (soll 10) volle Wirkung 17
```

Die vom Pruefer zitierten Zahlen stimmen.

Gegenprobe am Basisstand: `docs/beweise/roh/NAK-311-1166409-dirty.md:5419`
(Kanonlauf der Etappe 4) traegt fuer dieselbe Zeile

```
[p=0 d=0 erste Abweichung 1 (soll 1) volle Wirkung 8; p=1 d=7 erste Abweichung 8
 (soll 8) volle Wirkung 15; p=3 d=5 erste Abweichung 6 (soll 6) volle Wirkung 13;
 p=7 d=1 erste Abweichung 2 (soll 2) volle Wirkung 9; …]
```

— kein Feld „Bezugspunkt", und erste Abweichung genau `d + 1`, volle Wirkung
genau `d + 8`. Der Zusagetext war am Basisstand also **richtig** und ist durch
diesen Aenderungssatz falsch geworden. Das belegt den Befund haerter als das
Urteil es tut.

**Praezisierung 3 (Reichweite).** Der Pruefer nennt nur p = 0. Verschoben ist
je Fall **genau eine** der vier Phasen: p = 0 bei 48 und 44,1 kHz, p = 7 bei
96 kHz. Fuer p = 1 und p = 3 (und bei 96 kHz zusaetzlich p = 0) trifft der Text
weiterhin zu. Falsifiziert sind dadurch streng genommen die Teilsaetze „weicht
ab d + 1 ab" und „wirkt ab d + 8 mit Gewicht 1" sowie das „nach dem Einsatz"
der ms-Werte; „bleibt bis einschliesslich Sample d bitgleich" bleibt woertlich
wahr, nur zu schwach. Diese Verteilung ist im Manifest vorhergesagt
(`docs/beweise/NAK-311.md:7207`, §40.4, Ueberschrift `:7113`) und gemessen
bestaetigt (`:11408`, §60.7, Ueberschrift `:11394`):

> bei p = 0 von 0 auf 8, bei 96 kHz und p = 7 von 1 auf 9; p = 1 und p = 3
> unverändert

### 3. Was den Befund nicht widerlegt, aber teilweise entlastet

- **Der Code selbst ist schon umformuliert.**
  `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4629-4637` benennt den neuen
  Bezugspunkt ausdruecklich („Der Pegelbegriff ist eine dritte kalt startende
  Stufe; bei manchen Phasen liegt der erste wirksame Entwurf dadurch einen
  Rasterschritt spaeter") und formuliert die Zusage relativ: „erste Wirkung 1
  bis 8, volle 8 bis 15 Samples NACH dem Entwurf". Nur das Pruefetikett
  `:4691-4693` blieb stehen.
- **Die Schranke ist mitgezogen.** `:4662` prueft `basis % kDynamikSchritt ==
  dSoll && erste == basis + 1`, `:4668-4669` rechnen `ersteRaster` und
  `vollRaster` relativ zum Bezugspunkt, `:4688-4689` halten `minErste == 1 &&
  maxErste == kDynamikSchritt && minVoll == kDynamikSchritt && maxVoll ==
  2 * kDynamikSchritt - 1` unveraendert. Der Test misst also das Richtige; nur
  sein Etikett sagt etwas anderes.
- **§40.4 hat genau diesen Weg vorgeschrieben.** `:7381-7383`, Risiko 3:
  „Abwehr: der Bezugspunkt wird im Test aus `schrittRest` und der Auslenkung
  gelesen, nicht aus dem Tonbeginn; die geprüften Samplezahlen bleiben
  dieselben." `:7207` haelt fest, welche Zusage bleiben soll: „die Zusage
  „erste Wirkung 1 bis 8, volle 8 bis 15 Samples" und ihre ms-Werte bleiben".
  Der „d"-Wortlaut war davon nicht gedeckt.

### 4. Der Vergleichsmassstab des Befundes stimmt

Der Pruefer stellt B6 gegen B7. Nachgemessen ueber den Pruefbereich:

- `git diff --stat 5ee8318c...1183ad9f -- tools/beweise.ps1` → 4 Einfuegungen,
  4 Loeschungen; beruehrt A16, B3c, B6 und B7.
- Der T3-15-11-Satz innerhalb der B6-Behauptung ist vorher und nachher
  **bytegleich** (`git show 5ee8318c:tools/beweise.ps1` gegen den HEAD-Stand,
  Ausschnitt ab „Seit NAK-311 Etappe 4 Teil b (T3-15-11)": `diff` leer).
- Der t_E-Term `5*(tau_a + kPegelFensterMs)*fs` kommt am Basisstand 0-mal und
  am HEAD 1-mal vor — in der B7-Behauptung neu ergaenzt.
- Auch das Pruefetikett `DspGoldenTestMain.cpp:4691-4693` ist bytegleich zum
  Basisstand (`git show 5ee8318c:…:2725 ff.`).

### 5. Der kleinste Fix, und alle Stellen, die denselben Satz tragen

Den Wortlaut auf den Bezugspunkt umstellen — „ab dem ersten Steuerschritt mit
einer Auslenkung ungleich 0" statt „ab Sample d" —, wie der Code-Kommentar
`:4629-4637` und §40.4 `:7207` es bereits fassen. Die Zahlen 1 bis 8 und 8 bis
15 sowie die ms-Werte bleiben.

`git grep -n -- "p) mod 8"` und `git grep -n -- "ab d + 8"` liefern
uebereinstimmend diese lebenden Traeger:

| Stelle | Art | Im Pruefbereich |
|---|---|---|
| `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4691-4693` | Pruefetikett | ja |
| `tools/beweise.ps1:723` | Behauptung B6 | ja |
| `docs/beweise/NAK-311.md:685` | Matrixzeile M-73, §6.4 | nein (docs) |
| `docs/beweise/SONDE-015.md:4457-4460` | §13.3, Nachtrag zu E-6, datiert 20.09.2026 | nein (docs) |

**Praezisierung 4 (Vollstaendigkeit der Nacharbeit).** Der Pruefer nennt nur
die ersten drei. `docs/beweise/SONDE-015.md:4457-4460` traegt denselben Satz
und entsteht in diesem Ticket:

> Gegen einen Referenzkern mit Range 0 bleibt der Tap `post_committed` nach dem
> Einsatz eines Quadraturtons bis einschließlich Sample d = (8 − p) mod 8
> bitgleich und weicht ab d + 1 ab, wobei p die Steuerphase des Einsatzes ist.
> Die erste Wirkung liegt damit **1 bis 8**, die volle Wirkung des ersten
> Entwurfs **8 bis 15 Samples** nach dem Einsatz:

Dazu `docs/beweise/NAK-311.md:5345` (§34.5 „Die gemessenen Zahlen gegen ihre
Schranken", Ueberschrift `:5336`): „p = 0/1/3/7 → d = 0/7/5/1; erste Abweichung
d + 1; volle Wirkung d + 8". Das ist der Messstand der Etappe 4 und war dort
richtig (Punkt 2); ob er als Verlauf stehenbleibt, ist ein Entscheid, kein
Quellenbefund.

Nur Verlauf, nicht anzufassen: `docs/beweise/NAK-311.md:5141`, `:10022`,
`:11910` (Kanon-Uebersichten, Abschnitte „Uebersicht" ab `:5093`, `:9974`,
`:11862`), `docs/beweise/roh/NAK-311-1166409-dirty.md`,
`-3495395-dirty.md`, `-3495395-dirty-2.md`, `-a5815df-dirty.md`,
`docs/beweise/roh/NAK-311-etappe5-b6-zeilenvergleich.txt`,
`docs/beweise/roh/NAK-311-rot-M-73.txt`.

---

## Geprueft

Gelesen am HEAD: `SondeNullTestMain.cpp:170-205` und `:1534-1560`,
`:1620-1720`; `DspGoldenTestMain.cpp:990-1067`, `:2091-2130`, `:4545-4700`;
`tools/beweise.ps1:688`, `:723`, `:728`; `docs/beweise/NAK-311.md` Zeilen 685,
1255, 1624-1632, 5141, 5312-5348, 6385-6386, 7113-7210, 7374-7385, 9155-9170,
10013, 11394-11413, 11901, 11910; `docs/beweise/SONDE-015.md:4445-4472`;
`docs/beweise/roh/NAK-311-rot-M-133.txt` ganz;
`docs/beweise/roh/NAK-311-etappe5-a16-zeilenvergleich.txt` Kopf, Abschnittsmarken
und alle M-133-Fundstellen; `docs/beweise/roh/NAK-311-etappe5-b6-zeilenvergleich.txt:1840-1892`;
`docs/beweise/roh/NAK-311-1166409-dirty.md:5419`.

Selbst gefahren: die zwei Binaries oben, nacheinander, ohne Bau. Selbst
gemessen ueber Git: `git diff --stat` und der Vergleich der zwei Satztexte
gegen `5ee8318c`. Selbst nachgerechnet: `0xB754D7603137E70B` =
13210420415127086859.

Nicht bewertet: ob die zwei Befunde Defekt, Luecke oder Haertung sind; die
uebrigen Befunde der Erstpruefung 5 (L-1, L-2, H-1 bis H-5); alles ausserhalb
der zwei Befunde.
