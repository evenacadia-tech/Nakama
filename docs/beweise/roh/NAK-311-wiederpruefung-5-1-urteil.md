URTEIL: NEEDS_WORK — D-1 ist geschlossen; D-2 nur zur Hälfte: der Zahlenbereich „1 bis 8 / 8 bis 15 Samples" hängt in Prüfetikett und B6-Behauptung jetzt am Bezugspunkt, gemessen wird dort konstant 1 und 8.

# NAK-311 Etappe 5 (Planschritt S25j) — Wiederprüfung 1

- **Auftrag:** `docs/beweise/roh/NAK-311-wiederpruefung-5-1-auftrag.txt`
- **Prüfbereich:** `git diff 1fff97a5...1cf25cba -- eq-copilot/plugin/tests/SondeNullTestMain.cpp eq-copilot/plugin/tests/DspGoldenTestMain.cpp tools/beweise.ps1`
- **HEAD zu Beginn:** `e567e29578de59cb8d1eaca72e54050c101cb374`
- **HEAD am Ende:** `e567e29578de59cb8d1eaca72e54050c101cb374` (identisch; nichts gestagt, nichts committet, nichts gepusht, nichts gebaut)
- **Rolle:** frischer, lesender Prüfer. Ich habe die beiden vorhandenen Binaries gefahren, sonst nur gelesen und gerechnet.

---

## Frage 1 — sind D-1 und D-2 am Zielstand geschlossen?

### D-1 — 311/M-133 ohne Riegel für seine zweite Hälfte: **ja, geschlossen**

| Teilfrage des Auftrags | Befund | Beleg |
|---|---|---|
| Steht der Vergleich in der `pruefe`-Bedingung für alle vier Raten? | ja | `SondeNullTestMain.cpp:1671-1677` legt `kM133Goldens` als `constexpr` mit vier Paaren an, `:1678-1680` läuft über genau dieses Feld, `:1693` trägt `&& abdruck == golden.abdruck` INNERHALB der `pruefe`-Bedingung `:1690-1693`. Kein zweiter M-133-Prüfort im Plugin (`grep -rn "M-133" eq-copilot/plugin`: nur diese Datei). |
| Stimmen die vier Konstanten mit der Herkunftsdatei? | ja, alle vier | `:1673-1676` gegen `NAK-311-etappe5-m133-basisgolden.txt:146-149`: 44 100 `0xcb14d6eee50e7d7c`, 48 000 `0xbd787728731ba929`, 96 000 `0x5ece3bcf185baff5`, 192 000 `0x33f35b42b6693e06` — zeichengleich, Reihenfolge und Raten gleich. |
| Belegt die Herkunftsdatei nachvollziehbar den unveränderten Kern des Basisstands 5ee8318c? | ja | Weg (§1, 11 Dateien auf `git show 5ee8318c:<datei>` gesetzt), temporäre Ergänzung wörtlich als Diff (§2), Bau und Lauf mit Zeitstempelprüfung (§3), bytegleiche Rücknahme mit SHA-256 je Datei (§5). **Eigene Nachmessung:** die 11 geänderten Dateien zwischen `5ee8318c` und `1fff97a5` unter `eq-copilot/plugin` sind genau die zurückgesetzten 11 plus `DspGoldenTestMain.cpp`, `SchemaTestMain.cpp`, `TransactionTestMain.cpp` — und diese drei gehen nicht in `EqCopProbeeqNullTest` ein (`CMakeLists.txt:397-413`: `tests/SondeNullTestMain.cpp`, `sonde/SondeProcessor.cpp`, `src/AnalyseEngine.cpp`, `src/DiagnoseAntwort.cpp`, `core/ipc/PipeToken.cpp`, `NakamaKern`). Der Lauf lief also wirklich am unveränderten Kern. SHA-256 heute gegen §1: zehn der elf `gleich`, nur `SondeNullTestMain.cpp` abweichend — erwartet, weil der Fix danach genau dort die Goldens eingefügt hat. |
| Ist die Messung dieselbe wie im Endstand? | ja | `mitBell14k` im Endstand `:1536-1547` ist feldgleich zur temporären Fassung (`m133-basisgolden.txt:103-114`); Aufrufform `vorbereitet (rate, 2048)` / `fnvAusgang (*p, 40, 512, (int) rate + 133)` identisch (`:1681-1683` gegen `:118-120` der Herkunftsdatei). `fnvAusgang` selbst ist im Diff nur im Kommentar berührt, der Rumpf `:184-205` unverändert. |
| Fällt der Rotbeweis am Fingerabdruckvergleich? | ja (gelesen, nicht nachgefahren) | `NAK-311-rot-M-133-b.txt`: Mutation `alpha`-Nenner `2.0 -> 2.000001` in `DspFilter.h`, Bein Exit 1, genau vier gefallene Prüfungen, in jeder Zeile ist „gleich nein" die einzige gefallene Teilbedingung, die übrigen vier Teilbedingungen bleiben sichtbar wahr; 127 Bestandsprüfungen bleiben grün. Rücknahme bytegleich belegt — **eigene Nachmessung:** `DspFilter.h` trägt heute exakt den dort genannten SHA-256 `E2ABC8FE…AEC3D`, und mein eigener Lauf gibt dieselbe Zählzeile `131 Pruefungen ok, 0 Fehler` wie der Lauf nach der Rücknahme. |
| Sagen Prüfetikett, Kommentare und A16 genau das, was die Bedingung misst? | ja | Etikett `:1694-1697`: „…und der Fingerabdruck des Ausgangs ist gleich dem am Basisstand 5ee8318c erzeugten Golden" — das misst `:1693`. Detailzeile `:1698-1700` zeigt Ist, Soll und „gleich ja/nein". Kommentare `:175-181` und `:1663-1670` nennen Herkunft, Messung und die Golden-Regel §7.1 und behaupten keinen Laufzeitvergleich mehr. `tools/beweise.ps1:688` (A16): „der Fingerabdruck des Ausgangs ist bei 44,1 / 48 / 96 / 192 kHz gleich dem Golden, das dieselbe Messung am unveränderten Kern des Basisstands der Etappe (5ee8318c) erzeugt hat" — deckungsgleich mit der Bedingung. |

**Eigener Lauf:** `EqCopProbeeqNullTest.exe` Exit 0, `SONDE-NULLTEST OK - 131 Pruefungen ok, 0 Fehler`. Alle vier M-133-Zeilen `ok` mit „gleich ja" und den vier Golden-Werten. Binary `2026-09-20T19:43:56` jünger als Quelle `2026-09-20T19:43:48`. Vor dem Lauf lief kein Bau- oder Testprozess (`cmake, MSBuild, cl, link, cargo, FL64, vctip, EqCop*` leer, NAK-290).

### D-2 — Zusagetext von 311/M-73 bis 311/M-75: **nein, nur zur Hälfte geschlossen**

Geschlossen ist der Teil, an dem die Erstprüfung 5 den Defekt festgemacht hatte. Prüfetikett `DspGoldenTestMain.cpp:4691-4692` und B6-Behauptung `tools/beweise.ps1:723` sagen jetzt „bis einschließlich des Bezugspunkts bitgleich … (erster Steuerschritt mit Auslenkung ungleich 0)" und „der dort entworfene Satz wirkt acht Samples später mit Gewicht 1". Beides ist genau das, was `:4662-4663` (`erste == basis + 1`) und `:4655-4660` (`voll == basis + kDynamikSchritt`, `svfVon[voll]` bytegleich `svfNach[basis]`) messen; die Definition von `basis` `:4638-4641` deckt sich wörtlich. Auch die Zusatzzeile in B6 „Seit dem festgelegten Pegelbegriff … liegt der Bezugspunkt bei manchen Steuerphasen einen Rasterschritt nach dem Toneinsatz" ist gemessen (siehe Detailzeilen unten: p=0 bei 48 und 44,1 kHz, zusätzlich p=7 bei 96 kHz). Im Fixdiff von `DspGoldenTestMain.cpp` ist **keine Bedingung und keine Zahl** berührt — die Änderung liegt ausschließlich in der Zeichenkette `:4691-4693`, die Bedingung `:4688-4689` steht unverändert als Kontext im Diff.

**Offen ist der Schlussteil desselben Satzes.**

- **Datei:Zeile:** `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4693` und `tools/beweise.ps1:723`.
- **Welcher Satz bricht:**
  - Prüfetikett `:4691-4693`: „der Tap bleibt bis einschließlich des Bezugspunkts bitgleich … — erste Wirkung 1 bis 8, volle 8 bis 15 Samples **danach**". Der einzige Bezug, den dieser Satz nennt, ist der Bezugspunkt.
  - B6 `:723`: „Die erste Wirkung liegt damit **je nach Steuerphase** 1 bis 8, die volle Wirkung des ersten Entwurfs 8 bis 15 Samples **nach dem Bezugspunkt**."
- **Was die Bedingung misst:** `minErste`/`maxErste`/`minVoll`/`maxVoll` werden `:4668-4673` aus `ersteRaster = erste - basis + dSoll` und `vollRaster = voll - basis + dSoll` gebildet, also im Raster ab dem Toneinsatz gezählt, nicht als Abstand zum Bezugspunkt — der Kommentar `:4665-4667` sagt das selbst („Im Raster gezählt, also unabhängig davon, wie viele Rasterschritte der kalt startende Pegel den Bezugspunkt nach hinten schiebt"). Der Abstand ZUM Bezugspunkt ist durch `:4662-4663` und `:4655` auf genau einen Wert festgenagelt: `erste - basis == 1` und `voll - basis == 8`, in jeder Phase und bei jeder Rate. Die Spanne 1…8 bzw. 8…15 entsteht allein aus `dSoll ∈ {0, 7, 5, 1}` (`:4568`, `:4590`). „Je nach Steuerphase" trifft also auf die Lage des Bezugspunkts zu, nicht auf den Abstand von ihm.
- **Reproduktion:** `eq-copilot/build/plugin/EqCopDspGoldenTest_artefacts/Release/EqCopDspGoldenTest.exe`, Detailzeile von 311/M-73 (48 kHz), wörtlich:

  `p=0 d=0 Bezugspunkt 8 erste Abweichung 9 (soll 9) volle Wirkung 16; p=1 d=7 Bezugspunkt 7 erste Abweichung 8 (soll 8) volle Wirkung 15; p=3 d=5 Bezugspunkt 5 erste Abweichung 6 (soll 6) volle Wirkung 13; p=7 d=1 Bezugspunkt 1 erste Abweichung 2 (soll 2) volle Wirkung 9; erste Wirkung 1 bis 8 Samples … volle Wirkung 8 bis 15 Samples`

  In allen vier Phasen ist `erste Abweichung − Bezugspunkt = 1` und `volle Wirkung − Bezugspunkt = 8`. 311/M-74 (44,1 kHz) zeigt dieselben vier Paare, 311/M-75 (96 kHz) dieselben mit p=7 bei Bezugspunkt 9, erste Abweichung 10, volle Wirkung 17 — wieder 1 und 8.
- **Woher der Bruch kommt:** vor dem Fix trug B6 an dieser Stelle „8 bis 15 Samples nach dem **Einsatz**". Der Fix hat den Bezug des ganzen Satzes auf den Bezugspunkt umgestellt und damit die vorderen Teilsätze berichtigt, den hinteren Zahlenbereich aber mit umgehängt. Die Zahlen selbst (1, 8, 15) und die ms-Werte (0,167 bis 0,3125 ms bei 48 kHz usw.) sind unverändert und weiterhin richtig — sie gehören zur Rasterzählung ab dem Toneinsatz, nicht zum Abstand vom Bezugspunkt. Es bricht also nur der Bezug des Schlussteils, an zwei Stellen dieselbe Formulierung.

Nicht als Defekt gewertet (zur Transparenz): das Etikett nennt die zusätzlich gemessenen Teilbedingungen `rasterStimmt` (`:4645-4649`) und `basis % kDynamikSchritt == dSoll` (`:4662`) nicht. Es behauptet damit weniger, als gemessen wird — das falsifiziert nichts und war nicht Gegenstand von D-2.

---

## Frage 2 — hat der Fixdiff etwas gebrochen?

**Nein** — außer der unter D-2 genannten Textstelle, die zum Fix selbst gehört.

- **Produktpfade leer, selbst gemessen:** `git diff --stat 1fff97a5...1cf25cba -- eq-copilot/plugin/dsp eq-copilot/plugin/sonde eq-copilot/plugin/state eq-copilot/schemas eq-copilot/fixtures eq-copilot/identity broker` gibt keine Zeile aus. Die vollständige Dateiliste des Fixdiffs (`--name-only`) ist: vier Dateien unter `docs/beweise/` bzw. `docs/beweise/roh/`, `docs/beweise/SONDE-015.md`, `eq-copilot/plugin/tests/DspGoldenTestMain.cpp`, `eq-copilot/plugin/tests/SondeNullTestMain.cpp`, `tools/beweise.ps1`. Kein Produktcode.
- **ZIEL zu HEAD:** `git diff --stat 1cf25cba..e567e295` zeigt nur `docs/beweise/NAK-311.md` und die Auftragsdatei dieser Runde. Der Zielstand des Bauers ist im Arbeitsbaum also unverfälscht messbar.
- **Tests, selbst gefahren (kein Bau):**
  - A16 `EqCopProbeeqNullTest.exe`: Exit 0, `131 Pruefungen ok, 0 Fehler` — deckt sich mit der Angabe des Dirigenten (131/0).
  - B6 `EqCopDspGoldenTest.exe`: Exit 0, `DSP-GOLDEN OK`, 311/M-73 bis 311/M-75 alle `ok` — deckt sich mit der Angabe des Dirigenten (389/0; die Zählzeile des Beins führt keine Gesamtzahl in der Abschlusszeile, die Einzelzeilen sind grün).
  - Binaries jünger als ihre Quellen: `EqCopProbeeqNullTest.exe 19:43:56` gegen `SondeNullTestMain.cpp 19:43:48`; `EqCopDspGoldenTest.exe 19:35:51` gegen `DspGoldenTestMain.cpp 19:33:11`.
- **Matrixzeilen:** die Zusage von M-133 (Rate unterstützt, Band hörbar, +0,0, Bitvergleich) trägt jetzt an allen vier Punkten einen Riegel; die Zusage von M-73 bis M-75 (kDynamikSchritt bleibt 8, erste und volle Wirkung im Raster) ist in den Bedingungen unverändert erfüllt.
- **Gate-Text S25j:** kein Satz berührt. Der Fix ändert kein Verhalten, keine Grenze („nichts Ungefragtes", bitidentisch ausgeschaltet, keine RT-Allokationen), keine Stateversion und keinen Vertrag — er fügt vier `constexpr`-Konstanten in einen Test ein und berichtigt Text.
- **Invarianten aus CLAUDE.md:** „Schemas sind Verträge", „Engine kennt keine Optik", „Audio bleibt echtzeitfest", „NaN-Ehrlichkeit", „State bleibt verlustfrei", „Identität eingefroren" — alle unberührt, weil kein Produktcode im Diff liegt. Die Goldens liegen als `constexpr` im Testtreiber, nicht im Produktpfad; `fnvAusgang` läuft außerhalb des Audio-Callbacks im Testrahmen.

---

## Geprüft

Selbst an der Quelle nachgemessen: den vollständigen Fixdiff der drei Dateien; die vier `constexpr`-Werte `SondeNullTestMain.cpp:1673-1676` gegen `m133-basisgolden.txt:146-149`; die Lage des Vergleichs in der `pruefe`-Bedingung `:1690-1693`; `mitBell14k` `:1536-1547` und die Aufrufform `:1681-1683` gegen §2 und §3 der Herkunftsdatei; SHA-256 aller elf Dateien aus §1 der Herkunftsdatei gegen den heutigen Arbeitsbaum; die Liste der zwischen `5ee8318c` und `1fff97a5` geänderten Dateien gegen die Quellenliste von `EqCopProbeeqNullTest` in `eq-copilot/plugin/CMakeLists.txt:397-413` und `nakama_kern_anbinden` `:277-290`; den Bedingungsblock `DspGoldenTestMain.cpp:4638-4689` samt `phasen[]` `:4568` und `dSoll` `:4590`; die Leere des Fixdiffs über dsp, sonde, state, schemas, fixtures, identity und broker; die Dateiliste des Fixdiffs; den Abstand ZIEL zu HEAD; die Zeitstempel beider Binaries gegen ihre Quellen; die Abwesenheit fremder Bau- und Testprozesse; und die beiden Läufe `EqCopProbeeqNullTest.exe` (Exit 0, 131/0) und `EqCopDspGoldenTest.exe` (Exit 0, `DSP-GOLDEN OK`) mit den Detailzeilen von 311/M-133 und 311/M-73 bis 311/M-75.

## Nicht geprüft

Nur gelesen, nicht nachgemessen: der Rotbeweis `NAK-311-rot-M-133-b.txt` — die Mutation nachzufahren hätte einen Bau verlangt, den der Auftrag ausschließt. Indirekt gestützt durch den heutigen SHA-256 von `DspFilter.h` (gleich der dort genannten Rücknahme) und durch meine eigene Zählzeile 131/0, die dem Lauf nach der Rücknahme entspricht. Ebenso nur gelesen: die Abschnitte 2 bis 4 der Herkunftsdatei (der Lauf am Basisstand selbst ist nicht wiederholbar ohne Bau; gestützt durch die Übereinstimmung der vier Werte mit meinem eigenen Lauf am Zielstand). Ausgeschlossen nach Auftrag und daher nicht beurteilt: `docs/**` einschließlich `docs/beweise/NAK-311.md`, `docs/beweise/SONDE-015.md` und der Wortlaut der Rohdateien; die beiden Zeilenvergleichsdateien `NAK-311-etappe5-a16-zeilenvergleich.txt` und `-b6-zeilenvergleich.txt`; `tools/dirigent/pruefliste.md`; der übrige Ticketbereich der Etappe 5, die übrigen Kanonbeine, der Laufzeit-Arm und die für das Register vorgemerkten Härtungen. Neue Befunde außerhalb der beiden Fragen habe ich nicht erhoben.
