URTEIL: PASS — alle Befunde geschlossen, nichts gebrochen.

# NAK-311 Abschluss, Wiederprüfung 1 (Planschritt S25j), Runde 1

Frische, lesende Prüfsession vom 20.09.2026. Gebundener Auftrag:
`docs/beweise/roh/NAK-311-abschluss-wiederpruefung-1-auftrag.txt`.

- HEAD zu Beginn: `8a907fd8942404a97f6cc50f007c151914bb27b9`
- ZIEL: `502eb03244a48f3bfdd8b0b5e443274e5b0e91e8`
- Prüfbereich: ausschließlich der Fixdiff `c4725f00...502eb032` über
  `eq-copilot/plugin/tests/DspGoldenTestMain.cpp`,
  `eq-copilot/plugin/tests/TransactionTestMain.cpp` und `tools/beweise.ps1`.
- Nichts gebaut, nichts gestagt, nichts committet, nichts gepusht. Ein offenes
  FL Studio (FL64, PID 48152) wurde nicht angefasst.

HEAD trägt über dem ZIEL nur Dokumente (`git diff 502eb032..8a907fd8` berührt
`docs/beweise/NAK-311.md` und die Auftragsdatei). Die drei Prüfdateien im
Worktree sind deshalb byteweise der ZIEL-Stand; alle Zeilenangaben unten
gelten für beide.

## Frage 1 — ist D-1 am Zielstand geschlossen?

**Ja.** Die im vorigen Urteil als falsch benannte Aussage steht am ZIEL nicht
mehr, und jeder Satzteil der drei Träger steht entweder in einer Bedingung,
ist als Eingrenzung der Messung formuliert (G3) oder nennt eine am Code
nachlesbare Stelle ohne Messbehauptung.

Die gemessene Bedingung von 311/M-129 ist unverändert
(`DspGoldenTestMain.cpp:2260`):
`! rumpf.empty() && rumpf.size() > 2000 && treffer.empty() && stufen == 1`.
Ich habe sie an der Quelle nachgerechnet: `rumpf` ist der Ausschnitt von
`"void DspKern::verarbeiteBand ("` bis `"\nvoid DspKern::verarbeiteBank"`, also
`DspKern.cpp` Zeile 661-849, 9742 Bytes; die sieben Zeichenketten aus
`verboten[]` (`:2250-2251`) kommen darin je 0-mal vor; `z.pegel.tick` genau
1-mal.

### Träger 1 — Behauptung B6, `tools/beweise.ps1:723`, letzter Satz

| Satzteil | Einordnung | Beleg |
|---|---|---|
| „Im Callback bleiben die Zaehler mit acht dynamischen Baendern bei 0 Allokationen und 0 Sperren" | steht in einer Bedingung | `DspGoldenTestMain.cpp:6605` `testAllokationen == 0 && RtWache::allokationen() == 0` und `:6611` `RtWache::sperren() == 0`; acht dynamische Bänder = `achtDynamisch`, alle `param::kSlots` = 8 Slots (`NakamaParameter.h:38`) über `machDynamisch` (`:6552-6557`), im Zyklus bei `blk % 400 == 315` (`:6576`) |
| „im Quelltext von verarbeiteBand steht der Aufruf der Pegelstufe genau einmal" | steht in einer Bedingung | `stufen == 1`, gezählt als Vorkommen von `z.pegel.tick` im Rumpf (`:2256-2258`); nachgemessen 1 |
| „und huellkurvePol, huellkurveEntwurf, std::exp, std::pow, std::log10, new und malloc stehen dort buchstaeblich nicht" | steht in einer Bedingung | `treffer.empty()` über genau diese sieben Einträge (`:2250-2251`); nachgemessen je 0 im Rumpf |
| „der Pol entsteht im Worker (baueProgramm)" | nachlesbare Stelle, keine Messbehauptung | `huellkurveEntwurf` (`DspFilter.h:317`, setzt `pegelPol` über `huellkurvePol`, `:324`) wird allein in `baueProgramm` gerufen (`DspProgramm.cpp:350`, Funktion ab `:244`); `baueProgramm` läuft am Worker-Weg `baueVor` (`DspKern.cpp:378`), nie im Audiothread (`DspKern.h:575`) |
| „Die Wortsuche liest nur den Text dieser einen Funktion und folgt keinem Aufruf" | Eingrenzung (G3), am Code nachprüfbar | Ausschnittsgrenzen `:2243-2248`; in `DspKern.cpp` beginnt `verarbeiteBand` bei 661 und die nächstfolgende Definition `verarbeiteBank` bei 850 — der Ausschnitt 661-849 ist genau eine Funktion |
| „ueber den Steuerratenschritt, der alle kDynamikSchritt Samples Kennlinie und SVF-Satz neu rechnet (leistungInDb mit log10, svfEntwurf mit pow), sagt sie nichts" | Eingrenzung (G3) plus nachlesbare Stelle | `DspKern.cpp:773-793` (`if (z.schrittRest <= 0)`, `:792` `z.schrittRest = kDynamikSchritt`, Wert 8 in `DspProgramm.h:236`); `:788` `dynamischeKennlinie (leistungInDb (…))` mit `std::log10` in `DspFilter.h:420`; `:791` `svfEntwurf (…)` nach `svfBell`/`svfLowShelf`/`svfHighShelf` mit `std::pow` (`DspSvf.h:104`, `:116`, `:129`) |

Die Aussage des Befundes D-1 — „der Rumpf von verarbeiteBand traegt keinen
Entwurf und keine transzendente Funktion" — steht am ZIEL nicht mehr in B6.
Der satzweise Vergleich der Behauptungszeile zeigt sie als einzige gestrichene
Aussage dieses Satzes.

Die drei Bestimmungen der schließenden Regel (`docs/beweise/NAK-311.md` §70
Nr. 1) sind damit getroffen: G1 im zweiten Satzteil, G2 im dritten, G3 im
fünften und sechsten. Etikett und Bedingung sind unverändert.

### Träger 2 — Detailausgabe von 311/M-129, `DspGoldenTestMain.cpp:2262-2267`

Selbst gefahren, Ausgabe wörtlich:
`Rumpf von verarbeiteBand 9742 Zeichen, Aufrufe der Pegelstufe 1, keine der
sieben verbotenen Zeichenketten im Text dieser Funktion; der Pol entsteht in
baueProgramm (Worker), die Wortsuche folgt keinem Aufruf`

- „9742 Zeichen" und „Aufrufe der Pegelstufe 1" sind gemessene Zahlen
  (`rumpf.size()`, `stufen`), in der Bedingung nur als `> 2000` und `== 1`
  gebunden — das ist der eingeordnete Punkt B-1, kein Befund. Die Zahl ist die
  **Byte**zahl; der Rumpf trägt 2 Nicht-ASCII-Bytes, als Zeichen gezählt sind
  es 9741. Dieselbe Messung, nur eine andere Einheit; die Bedingung berührt es
  nicht.
- „keine der sieben verbotenen Zeichenketten im Text dieser Funktion" deckt
  sich Wort für Wort mit `treffer.empty()` über ein Feld von genau sieben
  Einträgen und steht am `treffer.empty()`-Ast des Ternärs, also genau dort,
  wo die Bedingung hält.
- „der Pol entsteht in baueProgramm (Worker)" nennt eine nachlesbare Stelle
  (Beleg wie Träger 1), „die Wortsuche folgt keinem Aufruf" grenzt die Messung
  ein (G3).

### Träger 3 — Kommentar davor, `DspGoldenTestMain.cpp:2226-2232`

Deckt sich Satzteil für Satzteil mit den Trägern 1 und 2. Zusätzlich benennt
er die Art der Messung („am Quelltext, fail-closed nach dem Muster B-13") — das
ist die Bedingung `:2260`, die bei leerem oder zu kurzem Rumpf rot wird. Keine
Aussage dieses Kommentars geht über die Bedingung hinaus.

### Weiterer berichtigter Satz 1 — B6, Etappe 5 Satz C (§71.3)

Neu: „Im Callback bleibt der Zaehler dabei bei 0 Allokationen und 0 Sperren,
auch wenn eine Publikation ueber dem Kriterium im selben Zyklus liegt."

Beide Zähler stehen in Bedingungen (`:6605`, `:6611`). „Publikation über dem
Kriterium im selben Zyklus" ist `grosserSprung` (`:6543-6545`: `freq_hz` von
Slot 0 mal 100 gegen `kSprungFrequenzVerhaeltnis` = 2,0, `DspProgramm.h:150`),
gefahren bei `blk % 400 == 215` (`:6574`). Die gestrichene Aussage „die drei
Vergleiche laufen ausschliesslich im Worker" stand in keiner Bedingung dieses
Laufs; sie ist ersetzt durch den Fall, den der Lauf tatsächlich fährt.

Der zugehörige Kommentar (`:6540-6542`) spricht die Eingrenzung jetzt aus
(„diese Zeile misst das nicht, sie zaehlt Allokationen und Sperren"). Die
Ortsangabe „Die drei Vergleiche liegen im Worker" ist am Code nachlesbar und
zutreffend: die drei Vergleiche stehen in `DspKern::vergebeKennungen`
(`DspKern.cpp:354-356` über `sprungImVerhaeltnis` und `sprungInDb`,
`DspProgramm.h:190` und `:204`), gerufen aus `publiziereVorbau`
(`DspKern.cpp:416`), also am Publikationsweg, nicht im Callback. Der Nachsatz
„und `blockrand` bleibt unveraendert" ist Wortlaut aus der Vorfassung, eine
Aussage über den Code ohne Messbehauptung dieser Zeile; der Fixdiff ändert
`DspKern::blockrand` nicht (siehe Frage 2).

### Weiterer berichtigter Satz 2 — B7, `tools/beweise.ps1:728`, letzter Satz

Neu: „Zwei Sekunden eingeschwungenes Audio bewegen den Pegelzustand
nachweislich auf einen endlichen Wert ueber 0, der Slot faehrt dabei den
Begriff durchschnitt, und die Statebytes bleiben bytegleich: der Pegelbegriff
ist Laufzeit wie die Kanalzahl und steht in keinem Statefeld."

| Satzteil | Einordnung | Beleg |
|---|---|---|
| „auf einen endlichen Wert ueber 0" | steht in einer Bedingung | `TransactionTestMain.cpp:3129-3130` `pegel > 0.0 && std::isfinite (pegel)`, über `pegelBewegt` in die Bedingung `:3138` gezogen; gefahren 0,124999998 |
| „der Slot faehrt dabei den Begriff durchschnitt" | steht in derselben Bedingung | `begriff == dsp::Pegelbegriff::durchschnitt`, gelesen an Band 0 der gefahrenen Committed-Bank (`:3122-3128`); gefahren „durchschnitt" |
| „die Statebytes bleiben bytegleich" | steht in einer Bedingung | `:3138` `vorher == nachher`; gefahren 4708 Bytes vorher und nachher, gleich |
| „der Pegelbegriff ist Laufzeit wie die Kanalzahl und steht in keinem Statefeld" | nachlesbare Stelle, keine Messbehauptung über den Lauf | unter `eq-copilot/plugin/state` und `eq-copilot/schemas/state` kommt „pegel" 0-mal vor; `baueProgramm` setzt den Begriff unbedingt ohne Eingabe aus dem Vertrag (`DspProgramm.h:59`, gemessen in 311/M-125) |

Die gestrichenen Teile standen in keiner Bedingung: der Lauf vergleicht den
Pegelzustand mit keiner Sollleistung (also nicht „auf die Leistung des
Quadraturtons"), ein Drahtfeld prüft er nicht, und `a->stateRevision()` steht
nur in der Detailausgabe (`:3135`), nicht in der Bedingung. Der begleitende
Kommentar (`:3116-3121`) trägt die Eingrenzung jetzt ausdrücklich und verweist
für den Vertrag auf B3c.

### Neue Allaussage ohne Bedingung im Fixdiff?

Keine. Der Fixdiff führt zwei Allaussagen: die Liste der sieben Zeichenketten
— sie trägt `treffer.empty()` — und „steht in keinem Statefeld". Letztere ist
nicht neu, sondern der schmalste Rest der Vorfassung („kein Statefeld, kein
Drahtfeld, keine Revision"); sie behauptet keine Messung des Laufs, ist am Code
nachlesbar und am ZIEL zutreffend. Alle übrigen neuen Sätze sind entweder
Bedingungstext, Eingrenzung (G3) oder Ortsangabe.

Eine Feinheit, ausdrücklich **kein** Befund: die Bedingung sucht `"new "` mit
nachfolgendem Leerzeichen, die Behauptung nennt „new". Am ZIEL steht auch das
bloße „new" 0-mal im Rumpf, die Behauptung ist dort also nicht stärker als der
Befund; die Schreibweise ist zudem die der schließenden Regel §70 Nr. 1 (G2).
Ein `new(`-Platzierungsausdruck, den die Wortsuche verfehlte, ist ein
Sabotage-Szenario gegen das Prüfwerkzeug und nach Auftrag als Härtung
eingeordnet.

## Frage 2 — hat der Fixdiff etwas gebrochen?

**Nein.**

**Die zwei Testdateien berühren nur Kommentar- und Zeichenkettenzeilen.**
Zeilenweise klassifiziert: 24 Kommentarzeilen und 5 Zeichenkettenzeilen, 0
andere. Keine Bedingung, keine Zahl, kein Bezeichner ist geändert. Die fünf
Zeichenkettenzeilen liegen sämtlich im Detailtext-Argument von `pruefe` in
`f129`; Ternär und Bezeichner `treffer` stehen unverändert, und die
Bedingungszeile `:2260` erscheint im Diff nur als Kontext. Die Etiketten von
311/M-129 (`:2261`), 311/M-111 und 311/M-79 (`:3140-3142`) sind unverändert.

**`tools/beweise.ps1` ändert nur Wortlaut in B6 und B7.** Datei vorher und
nachher je 1586 Zeilen; verschieden sind genau die Zeilen 723 (B6) und 728
(B7). In beiden sind Kopf (`Kuerzel`, `Name`, `Art`, `Argumente`, `AbPhase`)
und Abschluss zeichengleich, verschieden ist allein das Feld `Behauptung`. In
B6 sind zwei Sätze durch drei ersetzt (der lange Satz ist in zwei geteilt), in
B7 ein Satz durch einen; kein weiterer Satz beider Zeilen ist angefasst.

**Der Fixdiff ist außerhalb der drei Dateien leer.**
`git diff c4725f00...502eb032` über `eq-copilot/plugin/dsp`,
`eq-copilot/plugin/sonde`, `eq-copilot/plugin/state`, `eq-copilot/schemas`,
`eq-copilot/fixtures`, `eq-copilot/identity`, `eq-copilot/install` und
`broker` liefert nichts. Der gesamte Fixdiff berührt sechs Dateien: die drei
Prüfdateien und drei Dokumente (`docs/beweise/NAK-311.md`,
`docs/beweise/roh/NAK-311-etappe5-b6-zeilenvergleich.txt`,
`docs/beweise/roh/NAK-311-etappe5-b7-zeilenvergleich.txt`).

**Die zwei Beine laufen grün.** Vor dem Fahren geprüft: kein Bau- und kein
Testprozess aktiv. Binaries jünger als ihre Quellen (`EqCopDspGoldenTest.exe`
22:09:15, `EqCopTransactionTest.exe` 22:09:22 gegen beide Testquellen
22:07:40).

| Bein | Binary | Exit | Ergebnis |
|---|---|---|---|
| B6 | `eq-copilot/build/plugin/EqCopDspGoldenTest_artefacts/Release/EqCopDspGoldenTest.exe` | 0 | 389 geprueft, 0 Fehler; DSP-GOLDEN OK |
| B7 | `eq-copilot/build/plugin/EqCopTransactionTest_artefacts/Release/EqCopTransactionTest.exe` | 0 | 192 geprueft, 0 Fehler; TRANSAKTION OK |

Das deckt sich mit dem, was der Dirigent am ZIEL nachgefahren hat. Die drei
betroffenen Zeilen sind im eigenen Lauf grün: 311/M-129 ok,
`null_allokationen_im_callback_samt_programmwechseln` ok (Testzähler 0,
Kernzähler 0), `null_sperren_im_callback` ok (Sperrenzähler 0), 311/M-79 ok.

**Matrixzeile, Gate-Text, Invariante.** Die Etiketten und Bedingungen der drei
Matrixzeilen sind unverändert, die Erwartung von 311/M-129 bleibt auf die Stufe
begrenzt. Der Gate-Text von S25j (`docs/plan/plan.json`, Schritt S25j) verlangt
unter BEWEIS Matrix, Rotbeweise an B6, A16, A35/R1 und den DSP-Goldens,
bitidentischen Nulltest, vollen Kanon und Laufzeit-Arm; der Fixdiff ändert
keine Bedingung und keinen Messwert und lässt damit jede dieser Zusagen
unberührt — insbesondere bleibt der Rotbeweis an B6 gültig, weil die Bedingung
der Zeile dieselbe ist. Die Invarianten aus `CLAUDE.md` sind nicht berührt:
kein Schema, kein State, kein DSP- und kein Brokercode ist geändert, Identität
und Fixtures bleiben unangetastet. Die Arbeitsregel, die D-1 gerissen hatte
(„kein Text behauptet mehr, als gemessen ist"), ist durch diesen Fixdiff
wiederhergestellt.

## Geprüft

Selbst an der Quelle nachgemessen: die Ausschnittsgrenzen und die volle
Bedingung von 311/M-129 (`rumpf` = `DspKern.cpp` 661-849, 9742 Bytes, je 0
Treffer der sieben Zeichenketten, `z.pegel.tick` genau 1-mal) und daneben das
bloße Wort „new" mit 0 Treffern; dass die nächstfolgende Definition nach
`verarbeiteBand` tatsächlich `verarbeiteBank` ist (Funktionsliste von
`DspKern.cpp`); der Steuerratenschritt `DspKern.cpp:773-793` samt
`leistungInDb` mit `std::log10` (`DspFilter.h:420`) und `svfEntwurf` nach
`svfBell`/`svfLowShelf`/`svfHighShelf` mit `std::pow` (`DspSvf.h:104`, `:116`,
`:129`); der Ort des Pols (`huellkurveEntwurf` ab `DspFilter.h:317`, gerufen in
`baueProgramm` `DspProgramm.cpp:350`, Worker-Weg `baueVor` `DspKern.cpp:378`);
der Ort der drei Vergleiche (`vergebeKennungen` `DspKern.cpp:354-356`, gerufen
aus `publiziereVorbau` `:416`); die zwei Zählerbedingungen
(`DspGoldenTestMain.cpp:6605` und `:6611`) samt dem achtfach dynamischen Satz
(`:6552-6557`, `param::kSlots` = 8 in `NakamaParameter.h:38`) und der
Publikation über dem Kriterium (`:6543-6545` gegen
`kSprungFrequenzVerhaeltnis` = 2,0, `DspProgramm.h:150`); die Bedingung von
311/M-79 (`TransactionTestMain.cpp:3129-3130` und `:3138`); dass „pegel" unter
`eq-copilot/plugin/state` und `eq-copilot/schemas/state` 0-mal vorkommt; die
zeilenweise Klassifikation des gesamten Fixdiffs und der satzweise Vergleich
der zwei Behauptungszeilen; die Leere des Fixdiffs über die acht
ausgeschlossenen Bereiche; die Zeitstempel beider Binaries gegen ihre Quellen.
Selbst gefahren: `EqCopDspGoldenTest.exe` und `EqCopTransactionTest.exe`,
nacheinander, beide Exit 0.

## Nicht geprüft

Nur gelesen, nicht selbst nachgemessen: der vorige Urteilstext
(`docs/beweise/roh/NAK-311-abschlusspruefung-urteil.md`) und die schließenden
Regeln in `docs/beweise/NAK-311.md` §70 und §71 — sie sind Auftragsgrundlage,
nicht Prüfgegenstand. Nicht gefahren: der volle Kanon, der Laufzeit-Arm, der
Nulltest im Host und jedes andere Bein außer B6 und B7; nichts gebaut, deshalb
auch keine Aussage darüber, ob ein Neubau dieselben Binaries ergibt. Außerhalb
des Prüfbereichs und deshalb nicht bewertet: der übrige Ticketbereich (geprüft
in der Abschlussprüfung T2), alle `docs/**`-Dateien als Prüfgegenstand (nur als
Beleg gelesen) und jeder unveränderte Text neben dem Fixdiff — darunter die
Etikettzeile von 311/M-79 (`TransactionTestMain.cpp:3140-3142`) und der
Kommentar zum achtfach dynamischen Satz (`DspGoldenTestMain.cpp:6546-6551`),
die der Fixdiff nicht anfasst. Ein offenes FL Studio wurde nicht angefasst,
also auch nichts installiert oder im Host gemessen.

## HEAD

- zu Beginn des Laufs: `8a907fd8942404a97f6cc50f007c151914bb27b9`
- am Ende des Laufs: `8a907fd8942404a97f6cc50f007c151914bb27b9`

Der Worktree trägt außer dieser Urteilsdatei nur die schon zu Beginn
vorhandenen unversionierten Verzeichnisse `briefing-hub/` und
`nimbalyst-local/`; nichts ist gestagt, committet oder gepusht.
