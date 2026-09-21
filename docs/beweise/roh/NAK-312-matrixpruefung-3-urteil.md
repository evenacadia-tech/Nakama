URTEIL: PASS — jeder Befund der Liste (D-A, D-B, D-C, L-A) ist geschlossen, und kein geänderter Satz bricht einen anderen Satz der Matrix, des Bauplans, der Regeln, des Gates oder den Code am ZIEL. Benannt: eine Lücke (L-B) und eine Härtung (H-B), beide ohne Nacharbeitsauftrag.

# Matrixprüfung 3 — NAK-312, Planschritt S25k (Wiederprüfung der Matrixnacharbeit 2)

Frischer, lesender Prüfer, 21.09.2026. Gefahren wurden ausschließlich lesende
Kommandos (`git rev-parse`, `git status`, `git diff`, `git show`, `git grep`,
`git ls-files --eol`, `grep`, `sed`, `awk`, `wc`) und die Lesewerkzeuge. Kein
Bau, kein Test, kein Kanon, kein Broker, kein FL Studio, kein Python. Geschrieben
wurde genau diese Datei.

- **HEAD zu Beginn:** `75b66066e6762067c1b6fa6697337f94faaf6659`
- **HEAD am Ende:** `75b66066e6762067c1b6fa6697337f94faaf6659`
- `git status --short` vor und nach dem Lauf: nur die zwei fremden untracked
  Ordner `briefing-hub/` und `nimbalyst-local/` (vor dem Schreiben dieser Datei).
- **Prüfbereich gemessen:**
  `git diff --stat 801cb75a...b5e60e2c -- docs/beweise/NAK-312.md` ergibt
  **1 Datei, +280/−65** — deckungsgleich mit dem Auftrag.
- **Produktcode unverändert:** `git diff --stat 090d0fa1 HEAD -- eq-copilot
  broker tools` ist **leer** (selbst gemessen). Alle Zeilenangaben dieses
  Berichts sind deshalb am Arbeitsbaum = ZIEL nachgelesen.

## 1. Urteil je Befund

| Befund | geschlossen | Beleg (selbst am ZIEL gemessen) |
|---|---|---|
| **D-A** (Namenskollision, R-312-13) | **ja** | §6-Kopf trägt die Regel im Wortlaut. In §5–§8 zählt `grep -oE 'M-[0-9]+'` **626** Nennungen, davon **68** mit Präfix (vor der Runde 17). Jede Nummer über 86 (SONDE-015 M-92, M-119, M-120) trägt ein Präfix — 0 bloße Treffer. Alle Kollisionsnummern 35/36/47/55/81/84/92 einzeln am Kontext entschieden: jede fremde Bedeutung ist präfixiert oder steht in der erlaubten nachgestellten Form. `SONDE-012` erscheint nur als `U02`/`U03` mit Präfix, `NAK-246` nur als `R-D2`. Der Satz „stehen nur in der Quellspalte" ist in §5–§8 restlos entfernt (0 Treffer auf `Quellspalte`). |
| **D-B** (Rotbeweis M-38, E-312-11) | **ja** | (1) `TransactionTestMain.cpp:72-73` eigener thread-lokaler Zähler, `:76-91` beide Operatoren, Inkremente `:78` und `:87`, **kein** `meldeAllokation()`; `grep -rn meldeAllokation` über `eq-copilot` trifft nur `DspRtWache.h:13/:41` und `DspGoldenTestMain.cpp:55/:100/:109` — die Ursache ist exakt beschrieben, das B6-Muster in §7.2 ist zeichengleich zitiert. §7.0 trägt den Pfad mit Zweck; M-01, M-02, M-26, M-38, M-53 sagen alle, ab welcher Etappe der Zähler messbar ist. (2) `:1145` und `:1172` sind beide `toStdString()` (Allokation); `:1139`, `:1184`, `:1192` sind alle drei `const juce::ScopedLock` — `meldeSperre` hat **genau einen** Aufrufer, `GemeldeteSperre` (`DspRtWache.h:84`). Kein Sperrenanstieg wird mehr behauptet. (3) „zwei Hälften" gesetzt, Schleusenhälfte ausdrücklich mutationsfrei. |
| **D-C** (Mutationskette M-81, E-312-12) | **ja** | M-81 trägt „zwei Hälften". Mengenhälfte mit eigener Behauptung über die Abdeckungstabelle; M-79 belegt, dass diese Tabelle über einen Testzugang lesbar ist und eine eigene Mengenprüfung trägt — die Mutation „`eq_enabled` aufnehmen" fällt damit an der **eigenen** Behauptung. Die wahre Folge in der Zeile deckt sich Wort für Wort mit §7.3 Punkt 1 (Blockrand schreibt nur `blockrandStand[i]`) und Punkt 5 (`hostEreignisOffen` behält den Worker als einzigen Verbraucher) und mit dem Code: `:1455` Worker-`exchange`, `:1460` Gleichheitsvergleich, `DspProgramm.cpp:263` `aus.eqEngagiert`. Verhaltenshälfte messend, ohne eigene Mutation. |
| **L-A** (Kommentar, R-312-14) | **ja** | §7.6 Punkt 2 verlangt die Berichtigung im selben Änderungssatz und zitiert `State.cpp:270-271` wörtlich richtig; §8 Zeile E („geänderte Zusage an drei Stellen") führt die Regel. Am Code: `:270-271` trägt genau diesen Satz, `:264-269` den übrigen Block, `:272` den `seq_cst`-Store; `PluginProcessor.cpp:874` ist der `memory_order_relaxed`-Load, den §7.6 Punkt 2 auf `acquire` hebt. Beide Halbsätze des Kommentars werden dadurch falsch — die Zuordnung stimmt. |
| **L-B** (neu, Lücke) | — | siehe §3 |
| **H-B** (neu, Härtung) | — | siehe §4 |

## 2. Gate und Invarianten

Der Gate-Text steht unverändert im Kopf von `docs/beweise/NAK-312.md`, Zeile
„Gate" (die Runde fasst ihn nicht an).

- **„Nichts Ungefragtes":** keine geänderte Zeile verlangt im Audio-Thread eine
  Sperre, Allokation, ein Warten, einen Datei-, Pipe- oder Netzzugriff oder
  Logging. Im Gegenteil: die Änderungen an M-01, M-02, M-26, M-38 und M-53
  verschärfen die Messung, weil `RtWache::meldeAllokation()` nur im Audiopfad
  zählt (`DspRtWache.h:41-44`, gelesen) und die Bindung allein im Testbein
  liegt. E-312-2 („kein Warten, auch nicht offline") ist nicht berührt.
- **Rotbeweis an der Zusagezeile:** der Nahtpunkt hält. Der einzige
  `RtWache::Bereich` des Produkts steht in `DspKern.cpp:1059`; M-01 legt ihn an
  den Eintritt von `processBlock` (`SondeProcessor.cpp:371` ist die erste
  Anweisung des Rumpfes, Signatur `:369`), `dspKern->verarbeite` steht bei
  `:432`. Ein `v3Status()` aus `processBlock` läge damit im gemessenen Bereich,
  und die beiden `toStdString()` erhöhten `RtWache::allokationen()`. Die
  Gegenprobe von M-38 fällt also wirklich an der Zusagezeile.
- **Keine Wandzeit, kein Zufall** in einer geänderten Zeile; M-02 stützt sich auf
  `kontrollTaktFuerTest`, M-81 auf die Taktsperre.
- **Etiketten stimmen am Code:** M-38 „zwei Hälften" deckt sich mit E-312-10
  („`Schleuse.h` bekommt keinen Haken; die Mutation ‚zweites Lesen in
  `betreten()` entfernen' entfällt als Rotbeweis") und mit dem Muster von M-04.
  `betreten()` ist exakt `Schleuse.h:121-139` und sperrenfrei; `:30-37` trägt
  wörtlich „`warteMutex` und `warte` gehoeren ausschliesslich dem
  Schliesspfad". M-81 „zwei Hälften" folgt demselben Muster.
- **State, Identität, tote Elemente:** die Runde ändert keine Zusage zu State,
  Vertrag, Identität oder Oberfläche.
- **Struktur:** alle **86** Matrixzeilen in §6 tragen exakt elf Trenner, kein
  ungeschützter senkrechter Strich, keine doppelte ID; vor der Runde ebenfalls
  86 — es wurde keine Zeile angelegt oder entfernt, §6.8 sagt das richtig.

## 3. LÜCKE L-B — M-80 trägt die Mutationsform, die E-312-12 für M-81 verwirft

M-81 begründet seine mutationsfreie Verhaltenshälfte neu und in der Sache
richtig: „eine Mutation, die zusätzlich einen Schreibweg auf `aus.eqEngagiert`
gäbe, baute im Rotlauf genau den Blockrandweg, den E-312-5 nach NAK-340
verschiebt, und ist deshalb verworfen."

M-80, die Schwesterzeile für Bandwerte, trägt ihre Mutation unverändert:
„`gain_db` in die Abdeckungstabelle von M-79 aufnehmen → die erste Hälfte (ohne
Takt abweichend) fällt, **und der Kern schriebe Bandwerte ohne die eigene
Besitzregel, die NAK-340 erst baut** → rot." Der hervorgehobene Halbsatz ist
formgleich mit dem Grund, aus dem E-312-12 die M-81-Mutation verwirft, steht dort
aber als Bestandteil des Rotlaufs statt als Verwerfungsgrund. §15.4 hält M-80
ausdrücklich für geprüft, misst sie aber nur gegen §7.3 Punkt 1 bis 5 — nicht
gegen den neuen Grund aus E-312-12. E-312-12 selbst ist auf M-81 formuliert und
schweigt dazu, ob sein letzter Satz M-80 erreicht.

**Klasse LÜCKE, nicht DEFEKT:** M-80 steht nicht in der Befundliste, und ihr
Rotbeweis liegt außerhalb des Diffs dieser Runde (berührt ist dort nur die
Quellspalte). Hinzu kommt eine echte Lesart, unter der nichts offen ist: liest
man „die erste Hälfte" als die Mengenaussage „er steht NICHT in der
Abdeckungstabelle", fällt auch M-80s Mutation an der eigenen Behauptung, und der
Halbsatz über den Kern ist eine Warnung, keine Mutationsstufe. Genau diese
Zweideutigkeit — das Klammerwort „(ohne Takt abweichend)" benennt die zweite,
nicht die erste Zusagehälfte — ist der offene Punkt. Benannt, keine Nacharbeit.

## 4. HÄRTUNG H-B — die Präfixregel kennt drei Schreibweisen, aber keine Prüfform

R-312-13 erlaubt nach dieser Runde drei gültige Formen für eine fremde Zeile:
`SONDE-015 M-47` (einfach), `[SONDE-015] M-47` (in Zitaten, Codezitaten und
Testnamen) und `M-55 von SONDE-015` (nachgestellt, vom §6-Kopf ausdrücklich
bestätigt). Der §6-Kopf nennt alle drei, aber keine Stelle nennt ein Muster, mit
dem eine spätere Runde die Regel mechanisch nachprüfen könnte; §15.3 hat dafür
ad hoc `grep -oE '(\[)?SONDE-01[235](\])? M-[0-9]+'` benutzt, das die
nachgestellte Form nicht trifft. Eine Zeile in §8 mit dem verbindlichen
Suchmuster machte R-312-13 so prüfbar wie die übrigen Prüflistenzeilen. Von
keiner Zusage verlangt — benannt, keine Nacharbeit.

## 5. Was ich selbst am ZIEL nachgemessen habe

**Quellen, Zeile für Zeile am Arbeitsbaum gelesen** (Produktcode seit `090d0fa1`
unverändert, selbst gemessen):

- `eq-copilot/plugin/tests/TransactionTestMain.cpp:70-95` (Zähler `:72-73`,
  `operator new` `:76-82` mit Inkrement `:78`, `operator new[]` `:85-91` mit
  `:87`, kein `meldeAllokation`) und `:3365-3380` (`dsp::RtWache::sperren()` bei
  `:3368`, `imCallback == 0` in der Prüfung `:3374-3376`).
- `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:95-115` (`:100` und `:109`
  zeichengleich mit dem Zitat in §7.2) und `:6670-6675` — dort prüft B6
  `allokationen > 0 && RtWache::allokationen() == 0`, was M-02s Satz „der
  Grundsatz selbst hält heute schon, gemessen in B6" unabhängig bestätigt.
- `eq-copilot/plugin/dsp/DspRtWache.h:30-95`: `:36-38` thread-lokale Tiefe,
  `:41-44` `meldeAllokation` nur im Audiopfad, `:50-53` `meldeSperre`,
  `:78-93` `GemeldeteSperre` mit `:84` als einzigem Aufrufer (per
  `grep -rn meldeSperre` über `eq-copilot` bestätigt), `:94` `allokationen()`.
- `eq-copilot/plugin/sonde/SondeProcessor.cpp`: `:136-146` Provider-Lambdas,
  `:151-154` M-119-Kommentar, `:369-374` Kopf von `processBlock`, `:432`
  `dspKern->verarbeite`, `:1139`, `:1145`, `:1172`, `:1184`, `:1192`, `:1325`,
  `:1455`, `:1460`.
- `eq-copilot/plugin/sonde/SondeProcessor.h:175-180`,
  `eq-copilot/plugin/src/HoerMarkierung.h:419-430`,
  `eq-copilot/plugin/core/ipc/controlclient/Schleuse.h:28-40` und `:119-141`.
- `eq-copilot/plugin/dsp/DspProgramm.cpp:261-265`,
  `eq-copilot/plugin/dsp/DspKern.cpp:529-533`, `:618-628`, `:1057-1061`,
  `eq-copilot/plugin/state/NakamaParameter.cpp:79`,
  `eq-copilot/plugin/src/prozessor/State.cpp:262-274`,
  `eq-copilot/plugin/src/PluginProcessor.cpp:874`,
  `eq-copilot/plugin/tests/MarkierungTestMain.cpp:529-546`.
- Fremde Manifeste als Präfixstichprobe: `docs/beweise/SONDE-015.md:1087`
  (M-39), `:1100` (M-47), `:1114` (M-55), `:1159` (M-81), `:1162` (M-84),
  `:1175` (M-92), `:1223` (M-119), `:1224` (M-120) und
  `docs/beweise/SONDE-013.md:519-521` (M-34, M-35, M-36). **Jedes gesetzte
  Präfix trifft das richtige Manifest und die richtige Zeile**, inhaltlich
  abgeglichen, nicht nur der Nummer nach.

**Zitattreue, gegen die Quelle geprüft** — in allen fünf Fällen ist der Wortlaut
unverändert und nur der Klammerzusatz hinzugekommen: der M-119-Kommentar
(`SondeProcessor.cpp:151-154`, die Auslassung „…" bestand schon vor der Runde),
der M-120-Kopfkommentar (`SondeProcessor.h:176-179`), der NAK-47-Kommentar
(`HoerMarkierung.h:420-429`, dessen eigene Schreibweise „NAK-47 (SONDE-013
M-34)" das Präfix `[SONDE-013] M-35` bestätigt), der B6-Testname
(`DspGoldenTestMain.cpp:6672`) und der A3-Testname
(`MarkierungTestMain.cpp:546`, Präfix vor dem öffnenden Anführungszeichen, das
Zitat selbst unberührt).

**Eigene Zählungen:** 626 `M-nn` in §5–§8, davon 68 präfixiert (vorher 17 —
deckt sich mit §15.3); 0 bloße Nennungen mit Nummer über 86; 86 Matrixzeilen vor
und nach der Runde, alle mit elf Trennern, keine doppelte ID; 0 Treffer auf
`Quellspalte` in §5–§8. `git ls-files --eol docs/beweise/NAK-312.md` meldet
`i/lf w/lf` — die Runde hat die Zeilenenden nicht gedreht.

**Maßstab gelesen:** §14 vollständig (§14.3 und §14.4 als verbindliche Grundlage),
§5 bis §8 am ZIEL, der Diff der Runde vollständig, §6-Kopf mit den vier
Etiketten, §6.8, §7.0, §7.1-Verweise, §7.2, §7.3 (Punkte 1 bis 5 und die
Abdeckungsabsätze), §7.6 Punkte 1 bis 3, §8 Zeile E in allen drei Fassungen,
§11.3 E-312-10, §15 vollständig.

## 6. Was ich nur gelesen und nicht nachgemessen habe

- **Alles außerhalb des Diffs dieser Runde.** Matrixzeilen und Bauplansätze, die
  der Diff nicht berührt, habe ich nur so weit gelesen, wie eine geänderte Zeile
  auf sie zeigt (M-04, M-19, M-28, M-79, M-80, M-82, §7.1, §7.5). Ihre eigenen
  Zusagen sind Gegenstand der Matrixprüfungen 1 und 2 gewesen.
- **§15 als Liste.** Ich habe sie gelesen und ihre Zahlen zufällig bestätigt
  (51 Stellen, 17 → 68, Zählung 86), aber nicht als Maßstab benutzt; maßgeblich
  war durchgehend der Diff selbst.
- **Kein Lauf.** Ob B7 nach der Bindung tatsächlich `RtWache::allokationen()`
  zählt, ob `312/M-79` seine Tabelle wirklich lesen kann und ob M-38s Mutation
  im gebauten Bein rot wird, sind Aussagen über Etappen, die noch nicht gebaut
  sind. Geprüft habe ich, dass sie am heutigen Code widerspruchsfrei und am
  richtigen Mechanismus aufgehängt sind — nicht, dass sie laufen.
- **Nicht Gegenstand, entsprechend nicht bewertet:** die Entscheide des
  Dirigenten in §11.3 und §14.4 als solche (E-312-2 bis E-312-12, R-312-13,
  R-312-14), H-A als eigene Mutation, H2/NAK-342, Etappe 7, NAK-340, die Karten
  U48/U49/U51/U56, der JUCE-VST3-Wrapperpatch, T3-12-*, `pruefliste.md`,
  Prosa, Zeilenverweise, Abschnittsstände und Historie in `docs/**`.
- **Fremde Dateien** (`briefing-hub/`, `nimbalyst-local/`) habe ich weder
  gelesen noch angefasst.

FERTIG Matrixprüfung 3, PASS, 0 Defekte, 1 Lücke, 1 Härtung.
