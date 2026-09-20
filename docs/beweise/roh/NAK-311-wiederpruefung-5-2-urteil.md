URTEIL: PASS — alle Befunde geschlossen, nichts gebrochen.

# NAK-311 Etappe 5 (Planschritt S25j) — Wiederprüfung 2

- **Auftrag:** `docs/beweise/roh/NAK-311-wiederpruefung-5-2-auftrag.txt`
- **Prüfbereich:** `git diff bb210b5e...f14daf9d -- eq-copilot/plugin/tests/DspGoldenTestMain.cpp tools/beweise.ps1`
- **HEAD zu Beginn:** `de5e27144fef01acb0cb330b20ddae5d15b9c729`
- **HEAD am Ende:** `de5e27144fef01acb0cb330b20ddae5d15b9c729` (identisch; nichts gestagt, nichts committet, nichts gepusht, nichts gebaut)
- **Rolle:** frischer, lesender Prüfer. Ich habe das vorhandene Binary B6 einmal gefahren, sonst nur gelesen, verglichen und gerechnet.
- **Zielstand im Arbeitsbaum messbar:** `git diff f14daf9d -- eq-copilot/plugin/tests/DspGoldenTestMain.cpp tools/beweise.ps1` ist leer; die beiden geprüften Dateien im Arbeitsbaum sind bytegleich zum ZIEL. `git diff --stat f14daf9d..de5e2714` trägt nur `docs/beweise/NAK-311.md` und die Auftragsdatei dieser Runde. Alle Zeilenangaben unten gelten für diesen Stand.

---

## Frage 1 — ist der offene Rest von D-2 am Zielstand geschlossen?

**Ja, geschlossen.** Der Schlussteil hängt den Bereich „1 bis 8 / 8 bis 15 Samples" an keiner der drei Stellen mehr am Bezugspunkt, am Entwurf oder am Toneinsatz auf. Er ist an allen drei Stellen ausdrücklich als Rasterzählung über die vier gefahrenen Phasen gekennzeichnet und auf das Sample `basis - dSoll` verankert — genau die Regel F3. Die vier Tatsachen F1 bis F4 stehen vollständig und ohne Zusatzzusage.

### Prüfetikett `DspGoldenTestMain.cpp:4698-4705`, Satzteil für Satzteil

| Satzteil des Etiketts | Deckende Bedingung | Urteil |
|---|---|---|
| „der Tap bleibt bis einschliesslich des Bezugspunkts bitgleich zum Referenzkern" | `erste` ist der erste Index mit `memcmp (&tapD[i], &tapR[i], sizeof (double)) != 0` (`:4626-4629`); `traegt` fordert `erste == basis + 1` (`:4670-4671`), also Bitgleichheit für alle i bis einschließlich `basis` | steht in einer Bedingung (F2) |
| „der Bezugspunkt ist der erste Steuerschritt mit einer Auslenkung ungleich 0" | Definition von `basis` `:4646-4649`: erstes i mit `rest[i] == kDynamikSchritt - 1` UND `auslD[i] != 0.0`; `rest` aus `z.schrittRest` `:4621`, `auslD` aus `fahreStereoTon` `:4616` | steht in einer Bedingung (F1) |
| „und liegt im Raster auf der Phase d = (8 - p) mod 8 zum Toneinsatz" | `dSoll = (kDynamikSchritt - p) % kDynamikSchritt` `:4592`; `traegt` fordert `basis % kDynamikSchritt == dSoll` `:4670`; `rasterStimmt` `:4653-4657` prüft zusätzlich für ALLE i, dass `rest[i] == 7` genau dann gilt, wenn `i % 8 == dSoll`. i zählt ab dem ersten Tonsample (`vorlauf` läuft mit Amplitude 0,0 `:4607-4609`, die Messschleife mit `amp` `:4614-4617`) | steht in zwei Bedingungen (F1) |
| „die erste Abweichung liegt genau 1 Sample nach ihm" | `erste == basis + 1` `:4671` | steht in einer Bedingung (F2) |
| „und genau 8 Samples nach ihm liegt wieder ein Steuerschritt, an dem der dort entworfene Satz mit Gewicht 1 wirkt" | `voll = basis + kDynamikSchritt` `:4663`; `volleWirkung` `:4664-4668` fordert `voll < messen`, `rest[voll] == kDynamikSchritt - 1` und `memcmp (&svfVon[voll], &svfNach[basis], sizeof (SvfKoeffizienten)) == 0` | steht in einer Bedingung (F2) |
| „in jeder der vier gefahrenen Steuerphasen" | Schleife `pi < 4` über `phasen[] = { 0, 1, 3, 7 }` `:4570`, `:4589-4591`; `allesTraegt` sammelt alle vier `:4672` und steht in der `pruefe`-Bedingung `:4696` | steht in einer Bedingung (F2) |
| „1 bis 8 und 8 bis 15 Samples sind die Rasterzaehlung 1 + d und 8 + d ueber diese Phasen, gezaehlt ab dem Sample Bezugspunkt - d" | `ersteRaster = erste - basis + dSoll` und `vollRaster = voll - basis + dSoll` `:4676-4677`, also der Abstand vom Sample `basis - dSoll`; mit `erste == basis + 1` und `voll == basis + 8` ist das 1 + d und 8 + d. `minErste`/`maxErste`/`minVoll`/`maxVoll` `:4678-4681` sind Minimum und Maximum über die vier Phasen, geprüft in `:4696-4697` gegen 1, `kDynamikSchritt`, `kDynamikSchritt` und `2 * kDynamikSchritt - 1` | als Rechnung über die vier Phasen gekennzeichnet und zugleich in der `pruefe`-Bedingung verriegelt (F3) |

**Hängt noch ein Satzteil den Bereich am Bezugspunkt, am Entwurf oder am Toneinsatz?** Nein. Der einzige Anker des Bereichs ist „ab dem Sample Bezugspunkt - d". Der Bezugspunkt selbst kommt im Satz nur noch mit den beiden konstanten Abständen 1 und 8 vor; der Toneinsatz nur noch als Bezug der Phase d, nicht des Bereichs; der Entwurf nur noch als Endpunkt („an dem der dort entworfene Satz mit Gewicht 1 wirkt"), nicht als Anfangspunkt.

### Die zwei mitgezogenen Kommentare

- **Dateikopf `:25-29`** (im Blockkommentar `:1-60`): „im Raster ueber die vier gefahrenen Steuerphasen sind es 1 bis 8 Samples bis zur ersten und 8 bis 15 bis zur vollen Wirkung des ersten Entwurfs, gezaehlt ab dem Sample Bezugspunkt - d". Rasterzählung genannt, Phasenzahl genannt, Anker genannt — deckt sich mit `:4676-4681`. Vorher stand dort „der erste Entwurf wirkt 1 bis 8 Samples anteilig und 8 bis 15 Samples voll", ohne jeden Anker.
- **Kommentar zum Bezugspunkt `:4631-4645`:** „dieser Rasterschritt ist das Einschwingen des Pegelbegriffs und steckt NICHT in den Zahlen unten" (F4) und „erste Wirkung 1 bis 8, volle 8 bis 15 Samples sind die RASTERZAEHLUNG 1 + d und 8 + d ueber die vier Phasen (`ersteRaster` und `vollRaster` weiter unten), gezaehlt ab dem Sample `basis - dSoll`; der Abstand ZUM Bezugspunkt ist konstant 1 und 8" (F3 und F2). Die Verweise auf `ersteRaster`, `vollRaster` und `basis - dSoll` treffen `:4676-4677` wörtlich.

### Der Satz zu T3-15-11 in Behauptung B6 (`tools/beweise.ps1:723`)

Dieselbe Prüfung, Satzteil für Satzteil, ergibt dasselbe Bild. Die drei Stellen, an denen der Bezug vorher brach, sind berichtigt:

| Stelle | vorher (bb210b5e) | jetzt (f14daf9d) | Deckung |
|---|---|---|---|
| erste Abweichung | „weicht ab dem Sample danach ab" | „weicht genau 1 Sample danach ab" | `erste == basis + 1` `:4671` |
| Spanne | „Die erste Wirkung liegt damit je nach Steuerphase 1 bis 8, die volle Wirkung des ersten Entwurfs 8 bis 15 Samples nach dem Bezugspunkt" | „ist dagegen die RASTERZAEHLUNG 1 + d und 8 + d ueber die vier gefahrenen Steuerphasen, gezaehlt ab dem Sample Bezugspunkt - d und nicht als Abstand zum Bezugspunkt" | `:4676-4681` gegen `:4696-4697`; der falsche Anker ist ausdrücklich verneint |
| Rasterschritt | „liegt der Bezugspunkt bei manchen Steuerphasen einen Rasterschritt nach dem Toneinsatz" | „einen ganzen Rasterschritt nach dem Toneinsatz; dieser Rasterschritt ist das Einschwingen des Pegelbegriffs und steckt nicht in den Zahlen 1 bis 8 und 8 bis 15" | F4, im eigenen Lauf nachgemessen (unten) |

Neu und ebenfalls gedeckt: „und er liegt im Raster auf der Phase d = (8 - p) mod 8 zum Toneinsatz" (`basis % kDynamikSchritt == dSoll` und `rasterStimmt`) sowie „in jeder gefahrenen Steuerphase und bei jeder Rate" (vier Phasen je Fall, drei Fälle `:4576-4580` mit 48 000, 44 100 und 96 000 Hz, jeder Fall mit eigener `pruefe`-Bedingung). Unverändert und weiter gedeckt: „im Test aus schrittRest und der Auslenkung gelesen statt aus dem Toneinsatz" (`:4646-4649`) und „dort traegt svfVon genau den am Bezugspunkt entworfenen Satz" (`:4667-4668`).

### ms-Werte gegen die Samplezahlen

Die Detailzeile rechnet `1000 * n / rate` `:4689-4694`; B6 nennt die Zeitwerte jetzt ausdrücklich als die der 8 bis 15 Samples der vollen Wirkung. Selbst nachgerechnet und gegen meinen eigenen Lauf gehalten:

| Rate | 8 Samples | 15 Samples | B6-Wortlaut | Urteil |
|---|---|---|---|---|
| 48 000 Hz | 0,166666667 ms | 0,3125 ms | „0,167 bis 0,3125 ms" | stimmt |
| 44 100 Hz | 0,181405896 ms | 0,340136054 ms | „0,181 bis 0,340 ms" | stimmt |
| 96 000 Hz | 0,083333333 ms | 0,15625 ms | „0,083 bis 0,156 ms" | stimmt |

Die Aussage „also bei 48 und 44,1 kHz GROEBER als die kuerzeste Attack von 0,1 ms" trifft zu (0,167 und 0,181 über 0,1; 96 kHz mit 0,083 ist ausgenommen und wird nicht genannt).

### F4 gegen die Detailzeile meines eigenen Laufs

`EqCopDspGoldenTest.exe`, Exit 0, `389 geprueft, 0 Fehler`, `DSP-GOLDEN OK`. Detailzeilen wörtlich, Anker `basis - d` von mir gerechnet:

| Fall | Phase | d | Bezugspunkt | basis - d | erste Abweichung | volle Wirkung | ersteRaster | vollRaster |
|---|---|---|---|---|---|---|---|---|
| 311/M-73 (48 kHz) | p=0 | 0 | 8 | **8** | 9 | 16 | 1 | 8 |
| 311/M-73 | p=1 | 7 | 7 | 0 | 8 | 15 | 8 | 15 |
| 311/M-73 | p=3 | 5 | 5 | 0 | 6 | 13 | 6 | 13 |
| 311/M-73 | p=7 | 1 | 1 | 0 | 2 | 9 | 2 | 9 |
| 311/M-74 (44,1 kHz) | p=0 | 0 | 8 | **8** | 9 | 16 | 1 | 8 |
| 311/M-75 (96 kHz) | p=0 | 0 | 8 | **8** | 9 | 16 | 1 | 8 |
| 311/M-75 | p=7 | 1 | 9 | **8** | 10 | 17 | 2 | 9 |

F4 deckt sich damit genau: bei p=0 (alle drei Raten) und zusätzlich bei p=7 in 96 kHz liegt der Bezugspunkt einen GANZEN Rasterschritt (8 = `kDynamikSchritt`) nach dem Toneinsatz, bei den übrigen Phasen auf dem Toneinsatz-Raster selbst — also „bei manchen Steuerphasen". Und dieser Schritt steckt nicht in den Zahlen: p=0 mit dem um 8 verschobenen Bezugspunkt liefert `ersteRaster` 1 und `vollRaster` 8, dieselben Werte wie ohne Verschiebung. Die Minima und Maxima über die vier Phasen sind in allen drei Fällen 1 bis 8 und 8 bis 15; die ausgegebenen Spannen der Detailzeile lauten „erste Wirkung 1 bis 8 Samples", „volle Wirkung 8 bis 15 Samples". In jeder Zeile gilt „erste Abweichung − Bezugspunkt = 1" und „volle Wirkung − Bezugspunkt = 8".

---

## Frage 2 — hat der Fixdiff etwas gebrochen?

**Nein.**

### `DspGoldenTestMain.cpp` berührt nur Zeichenketten- und Kommentarzeilen

Selbst gemessen, nicht nur gelesen: Ich habe beide Fassungen der Datei (`git show bb210b5e:` und `git show f14daf9d:`) durch einen Zerteiler geschickt, der Zeilen- und Blockkommentare entfernt, jeden Zeichenketten- und Zeichenrumpf durch eine Marke ersetzt und den Weißraum normiert.

- Erster Durchgang: einziger Unterschied ist die ANZAHL benachbarter Zeichenkettenmarken im `pruefe`-Aufruf (3 gegen 7) — die Längendifferenz beträgt exakt 24 Zeichen, also die vier zusätzlichen Marken. Alles davor und danach zeichengleich.
- Zweiter Durchgang mit zusammengefassten Nachbarliteralen: **beide Fassungen sind zeichengleich, 263 103 gegen 263 103 Zeichen.**

Damit ist bewiesen: keine Bedingung, keine Zahl im Code, kein Bezeichner und keine Detailzeile geändert. Die Änderung besteht aus drei Stücken — Kopfkommentar `:25-29` (im Block `:1-60`), acht `//`-Kommentarzeilen `:4637-4645` und sieben Zeichenkettenzeilen des `pruefe`-Etiketts `:4699-4705`. Die `pruefe`-Bedingung `:4696-4697`, `std::string (f.zeile) + " " + f.was`, der Aufbau der Detailzeile `:4683-4694` und `d.str());` stehen im Diff unverändert als Kontext. Die einzige geänderte Zeile mit Code-Anteil trägt den Verkettungsoperator `+`, der selbst unverändert bleibt; geändert ist nur der Inhalt des Literals dahinter.

### `tools/beweise.ps1`: nur der eine Satz der Behauptung B6

`git diff --numstat` meldet 1 hinzugefügte und 1 entfernte Zeile. Zeilenweiser Vergleich beider Fassungen: von 1 586 Zeilen unterscheidet sich **genau Zeile 723**. Der Präfix bis einschließlich `Behauptung='` ist zeichengleich (`Kuerzel='B6'; Name='EqCopDspGoldenTest'; Art='plugin'; Argumente=@(); AbPhase='jetzt'`), der Schluss `urf und keine transzendente Funktion.' }` ebenfalls, und die Zahl der Apostrophe ist vorher wie nachher 10 — die Behauptung bleibt eine einzelne einfach quotierte Zeichenkette, die Feldstruktur ist unangetastet. Innerhalb der Behauptung ist nur der T3-15-11-Satz geändert; alle übrigen Sätze (Etappe 2, 3, 4a, Satz B, C, D) stehen wörtlich gleich.

### Ausgeschlossene Bereiche leer

`git diff --stat bb210b5e...f14daf9d -- eq-copilot/plugin/dsp eq-copilot/plugin/sonde eq-copilot/plugin/state eq-copilot/plugin/tests/SondeNullTestMain.cpp eq-copilot/schemas eq-copilot/fixtures eq-copilot/identity broker` gibt **keine Zeile** aus. Die vollständige Dateiliste des Fixdiffs (`--name-status`) lautet: `docs/beweise/NAK-311.md`, `docs/beweise/SONDE-015.md`, `docs/beweise/roh/NAK-311-etappe5-b6-zeilenvergleich.txt`, `eq-copilot/plugin/tests/DspGoldenTestMain.cpp`, `tools/beweise.ps1` — kein Produktcode, kein Vertrag, keine Fixture, keine Identität, kein Broker. Die drei `docs/`-Dateien sind nach Auftrag ausgeschlossen.

### Test, Matrixzeile, Gate-Text, Invarianten

- **Test:** eigener Lauf des vorhandenen Binaries, Exit 0, `389 geprueft, 0 Fehler`, `DSP-GOLDEN OK` — deckt sich mit der Angabe des Dirigenten (389/0). 311/M-73 bis 311/M-75 alle `ok`. Binary `2026-09-20 20:15:50` jünger als die Quelle `2026-09-20 20:14:05` — deckt sich mit der Angabe des Dirigenten. Vor dem Lauf lief kein Bau- oder Testprozess (weder cmake, MSBuild, cl, link, lld-link, cargo, rustc, ninja, devenv, FL64 noch ein `EqCop*`-, `Nakama*`- oder `eqcop*`-Prozess; NAK-290). Einziger Fremdprozess war `vctip.exe` (PID 97796, MSVC-Telemetrie aus dem Bau des Bauers, NAK-300) — er baut und testet nichts und stand dem Lauf nicht im Weg.
- **Matrixzeile:** die Zusage von 311/M-73 bis M-75 ist in ihren Bedingungen unverändert (bewiesen durch die Zeichengleichheit des entkommentierten Codes) und läuft grün. Der Wortlaut der Matrix in `docs/beweise/SONDE-015.md` ist nach Auftrag ausgeschlossen.
- **Gate-Text S25j:** kein Satz berührt. INHALT (die 11 Befunde) unverändert; GRENZE („nichts Ungefragtes", ausgeschaltet bitidentisch, Passthrough sampleidentisch, keine RT-Allokationen, RBJ-Goldens bleiben Referenz, keine neue Stateversion ohne Vertrag) unberührt, weil kein Produktcode und kein Schema im Diff liegt; BEWEIS unberührt, weil B6 weiter grün ist und die Behauptung schärfer statt schwächer wurde.
- **Invarianten aus CLAUDE.md:** „Schemas sind Verträge", „Engine kennt keine Optik", „Audio bleibt echtzeitfest", „Zeit ist aktive Musikzeit", „NaN-Ehrlichkeit", „State bleibt verlustfrei", „Identität bleibt eingefroren" — alle unberührt; der Diff ändert ausschließlich Kommentar- und Zeichenkettentext in einem Testtreiber und einem Behauptungsfeld des Beweis-Runners. Die Regel „Begriffe nicht vermischen" wird durch den Fix gestärkt: Rasterzählung und Abstand zum Bezugspunkt sind jetzt getrennt benannt.

---

## Zur Transparenz, nicht als Befund gewertet

- **„des ersten Entwurfs".** B6 und der Kopfkommentar sprechen weiter von der „vollen Wirkung des ersten Entwurfs". Gemessen wird der am BEZUGSPUNKT entworfene Satz, also der erste WIRKSAME Entwurf — bei einer um einen Rasterschritt verschobenen Phase nicht der erste Entwurf im Raster überhaupt. Der Auftrag stellt fest, dass der Testname `erster_entwurf_wirkt_voll_nach_8_bis_15_samples` Bezeichner ist und bleibt; der unveränderte Kommentar `:4636` schreibt „der erste wirksame Entwurf". Der Ausdruck benennt hier den Endpunkt der Spanne, nicht ihren Anker, und die vier Tatsachen verlangen an dieser Stelle nichts weiter.
- **„dieser Rasterschritt ist das Einschwingen des Pegelbegriffs".** Die Ursachenzuschreibung geht über den reinen Wortlaut von F4 hinaus, ist aber nicht neu (der unveränderte Kommentar `:4634-4637` sagt dasselbe, und B6 trug schon vorher „Seit dem festgelegten Pegelbegriff") und deckt sich mit meiner Messung: verschoben sind genau die Phasen mit den wenigsten integrierten Tonsamples vor dem ersten Steuerschritt — d=0 bei allen drei Raten (null Samples) und zusätzlich d=1 bei 96 kHz (ein Sample, bei der höheren Rate die kürzeste Zeit). Kein Widerspruch zu einer Bedingung.

---

## Geprüft

Selbst an der Quelle nachgemessen: den vollständigen Fixdiff beider Dateien; den Bedingungsblock `DspGoldenTestMain.cpp:4569-4707` samt `phasen[]` `:4570`, `dSoll` `:4592`, `vorlauf` `:4607`, `basis` `:4646-4649`, `rasterStimmt` `:4653-4657`, `voll` und `volleWirkung` `:4663-4668`, `traegt` `:4670-4671`, `ersteRaster` und `vollRaster` `:4676-4677`, Minima und Maxima `:4678-4681`, Detailzeile `:4683-4694` und `pruefe`-Bedingung `:4696-4697`; das Prüfetikett `:4698-4705` und die drei Fälle `:4576-4580` Satzteil für Satzteil gegen diese Bedingungen; beide mitgezogenen Kommentare `:25-29` und `:4631-4645` und die Lage des Kopfkommentars im Block `:1-60`; den T3-15-11-Satz der Behauptung B6 `tools/beweise.ps1:723` gegen die Fassung in `bb210b5e` und gegen dieselben Bedingungen; den zeilenweisen Vergleich beider Fassungen von `beweise.ps1` (1 von 1 586 Zeilen unterschiedlich, Feldpräfix, Zeilenschluss und Apostrophzahl gleich); den doppelten Vergleich beider Fassungen von `DspGoldenTestMain.cpp` nach Entfernen von Kommentaren und Zeichenkettenrümpfen (zeichengleich); die Leere des Fixdiffs über dsp, sonde, state, `SondeNullTestMain.cpp`, schemas, fixtures, identity und broker; die vollständige Dateiliste des Fixdiffs; den Abstand ZIEL zu HEAD und die Bytegleichheit der beiden Prüfdateien im Arbeitsbaum mit dem ZIEL; die ms-Werte 1000·n/Rate für n = 1, 8, 15 bei 48 / 44,1 / 96 kHz gegen Detailzeile und B6-Wortlaut; die Abwesenheit fremder Bau- und Testprozesse; den Zeitstempel des Binaries gegen seine Quelle; und den eigenen Lauf `EqCopDspGoldenTest.exe` (Exit 0, 389/0, `DSP-GOLDEN OK`) mit den vollständigen Detailzeilen von 311/M-73 bis 311/M-75.

## Nicht geprüft

Nicht gebaut — der Auftrag schließt das aus; gefahren wurde nur das vorhandene Binary. Nur gelesen, nicht nachgemessen: der vorige Urteilstext `docs/beweise/roh/NAK-311-wiederpruefung-5-1-urteil.md` als Herkunft des Befundes und die dort zitierten Detailzeilen (durch meinen eigenen Lauf ersetzt und bestätigt). Nicht Prüfgegenstand nach Auftrag und daher nicht beurteilt: `docs/**` einschließlich `docs/beweise/NAK-311.md`, `docs/beweise/SONDE-015.md` und der Wortlaut aller Rohdateien; `docs/beweise/roh/NAK-311-etappe5-b6-zeilenvergleich.txt`; der übrige Ticketbereich der Etappe 5 und der bereits geschlossene Befund D-1; `eq-copilot/plugin/dsp/DspProgramm.h`; die übrigen Kanonbeine, der volle Kanon und der Laufzeit-Arm; `tools/dirigent/pruefliste.md`; die Härtungen NAK-81, NAK-269, NAK-286, NAK-306, NAK-331, NAK-332 und die in §58, §61, §62 und §65 für das Register vorgemerkten Punkte. Vom Fixdiff nicht berührt und deshalb außerhalb der Runde: der dritte Kommentar `:4659-4662` zur vollen Wirkung. Neue Befunde außerhalb der beiden Fragen habe ich nicht erhoben.
