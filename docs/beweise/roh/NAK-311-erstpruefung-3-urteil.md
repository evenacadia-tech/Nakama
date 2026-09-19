URTEIL: PASS — kein DEFEKT

# NAK-311 Erstprüfung 3 — Etappe 3 (W01 Pfadrampen in der Ruhe, W03 Slot-Lebenszyklus)

Prüfer: frischer, lesender Prüfer, 20.09.2026. Prüfstufe T2.
HEAD zu Beginn: `f11c3742e8a24af91a845ec4c4ae7f950eb5c27b`.
Prüfbereich: `git diff 024d686f...b68d677e` über `eq-copilot/plugin/dsp/DspKern.cpp`,
`DspKern.h`, `DspProgramm.h`, `eq-copilot/plugin/sonde/SondeProcessor.cpp`,
`eq-copilot/plugin/tests/DspGoldenTestMain.cpp`, `TransactionTestMain.cpp`,
`tools/beweise.ps1` (7 Dateien, 1826 Einfügungen, 15 Löschungen). Der Code am
Arbeitsbaum ist gleich ZIEL: `git diff --stat b68d677e f11c3742` nennt nur
`docs/PLAN-STAND.md`, `docs/beweise/NAK-311.md` und den Prüfauftrag. Die beiden
Etappencommits `ae5cce53` und `125c83ea` berühren außerhalb von `docs/` genau
diese sieben Dateien — keine Identität, kein Schema, kein State.

## (a) W01 — steht ein Pfad in der Ruhe auf den Ruhewerten 1,0?

Ja, an allen drei erreichbaren Stellen und ohne Lücke. `z.aktiv` wird im ganzen
Kern an genau drei Stellen geschrieben: `PfadZustand::ruhe()`
(`eq-copilot/plugin/dsp/DspKern.cpp:87`, setzt die Rampen selbst über
`rampen.setzeSofort (1.0)` an `:91`), der ENDE-Zweig von `blockrand` (`:470`,
`z.aktiv = -1`) und die Übernahme einer neuen Bank (`:539`, `z.aktiv = neu`).
In die Ruhe führt also nur `ruhe()` — aufgerufen aus `bereiteVor` (`:110`) und
`freigeben` (`:138`) — oder `blockrand:470`. Hinter `:470` setzt `:480` genau
zwei Übergänge: `crossfade` oder `hoerHalt`. Der Crossfade endet in
`verarbeitePfad` (`:945`) und weist dort bei `z.aktiv < 0` die Ruhewerte zu
(`:957`); der Hörhalt endet ausschließlich in `beendeHoerHalt` (`:425`, Zuweisung
an `:441`); beendet `beendeAudiohistorie` den Übergang vorzeitig (`:174-180`),
greift dort der ruhende Zweig (`:187-188`). Ein vierter Weg existiert nicht.
`verarbeitePfad` wird für den Committed-Pfad unbedingt gerufen
(`verarbeiteStueck:1091`, auch wenn `committedRuht` gilt), für den Candidate
solange `pfadRechnet` (`:387-391`) wahr ist — und das ist es, solange
`quelle >= 0` oder ein Übergang läuft, also über den ganzen Ausblendfade. Auch
ein Block über `maxBlock` läuft in Stücken durch `verarbeiteStueck` (`:983-999`),
der Übergang endet dort ebenso.

Das Ausblenden bleibt bei den Gains der ausblendenden Bank: der ENDE-Zweig
(`:462-482`) setzt keine Rampe, die Zuweisung liegt erst am Fadeende. M-06
(`bypasswechsel_ist_klickfrei`, `bypasswechsel_endet_bitgleich`) und
`311/M-30 trimwechsel_im_hard_bypass` sind in meinem eigenen B6-Lauf grün; der
Rotbeweis M-30 bricht das Rampenticken des nicht rechnenden Pfades und wird rot.
Der ruhende Zweig in `beendeAudiohistorie` trennt richtig: ein Pfad mit aktiver
Bank (auch im Hard-Bypass, wo eine Bank aktiv bleibt) geht auf sein Ziel, nur
der ruhende auf 1,0.

Die Candidate-Rampen bleiben während des Hörhalts unberührt im Sinn der Matrix:
`blockrand` setzt beim Nehmen der ENDE-Marke keine Rampe, und die gehaltene Bank
rechnet in `verarbeitePfad:902-907` als `klingend` mit ihren eigenen Rampen
weiter. `311/M-31 … Hoerhalt` misst das (32 Samples nach dem Candidate-Ende
gehalten, Abweichung post_candidate zu post_committed 0.000000000000 bei +6 dB);
der Rotbeweis setzt die Rampen schon an der ENDE-Marke auf 1,0 und wird rot.

Die Ruhezuweisung in `beendeHoerHalt` ist unbedingt richtig. `Uebergang::hoerHalt`
entsteht ausschließlich an `:480` und liegt dort hinter `z.aktiv = -1` (`:470`).
Solange ein Übergang läuft, nimmt `blockrand` keine Publikation (`:454`
`if (z.uebergang != Uebergang::keiner) return;`), `z.aktiv` kann also nicht
wieder `>= 0` werden; `beendeAudiohistorie` löscht den Übergang auf `keiner`,
womit die Wache in `beendeHoerHalt` (`:428`) vorher zurückkehrt. Passiert der
Rumpf die Wache, ruht der Pfad immer — eine aktive Bank kann dort keine Rampen
verlieren.

## (b) W03 — Vergaberegel, Blockrand, Übertragung

`vergebeKennungen` (`:255-286`) vergibt nach der Regel der Matrix. Die
Pfadkennung hält genau bei gleichen globalen Feldern von `rampenKompatibel`
(`:267-268`: `eqEngagiert`, `hardBypass`, `samplerate`, Mono-Bass-Stufe an oder
aus — dieselben Felder wie `DspProgramm.cpp:144-146`), sonst kommt eine neue aus
`naechsteKennung`. Je Slot vergleicht `:280-281` genau die Slotfelder von
`rampenKompatibel` (`DspProgramm.cpp:152-156`: `aktiv`, `typ`, `modus`,
`dynamisch`, `nutztSvf`, `quelle`); `:282` gibt einem inaktiven Slot 0, einem
gleich gebliebenen seine alte Kennung, jedem anderen aktiven eine neue.
`naechsteKennung` (`:222-230`) ist streng monoton und überspringt die 0, ein
aktiver Slot trägt also nie 0 und ein publiziertes Bankprogramm nie die
Pfadkennung 0. Verglichen wird gegen den Merkzettel (`:263`), den `merke`
(`:232-253`) bei **jeder** Publikation fortschreibt — also gegen die zuletzt
publizierte, nicht gegen die vom Audiothread genommene Belegung. Der Rotbeweis
M-41 ersetzt genau das (Mutation C schreibt den Merkzettel vor dem Vergleich aus
der gefahrenen Bank) und wird rot; `311/M-41` misst in meinem Lauf Übernahmen
`+1` und Meldung 0.000000 dB.

`blockrand` verlangt für den Rampenweg zusätzlich gleiche Kennungen
(`:494-501`): `pfadGleich` und, über alle acht Slots, gleiche
`lebenszyklus`-Kennung jedes im **neuen** Programm aktiven Slots; ein im alten
aktiver und im neuen inaktiver Slot fällt bereits über `rampenKompatibel`
(`aktiv != aktiv`). Beide Kurzschlüsse sind zeigersicher: ist `alt < 0`, ist
`pfadGleich` falsch, die Schleifenbedingung `i < kSlots && kennungenGleich`
bricht sofort ab und `rampenKompatibel (*pAlt, …)` wird wegen `&&` nie
ausgewertet.

Der Crossfade überträgt (`:512-535`) genau den `BandZustand` der Slots, die in
beiden Programmen aktiv sind und dieselbe Kennung tragen, und nur bei gleicher
Pfadkennung; `monoBassZustand` zusätzlich bei beiderseits aktiver Stufe. Aus der
Ruhe wird nichts übertragen, weil `pfadGleich` `pAlt != nullptr` verlangt
(M-07); in einen laufenden Übergang kommt keine Publikation (E-17, `:454`);
zwischen den Pfaden ist eine Übertragung baulich unmöglich, weil `alt` und `neu`
beide Bänke desselben Pfades sind und der Zähler ohnehin global eindeutig ist
(M-53). Gleiche Kennung impliziert gleiche Topologie über die Merkzettelkette,
auch über verdrängte Zwischenstände.

Eine neue Belegung nach Remove und Neubelegung erbt nichts: das Remove schreibt
im Merkzettel `aktiv = false` und Kennung 0, die Bedingung `bleibt` verlangt
`s.aktiv` und scheitert, die Neubelegung bekommt eine neue Kennung — auch wenn
die Zwischenpublikation verdrängt wurde und der Audiothread sie nie sieht
(`311/M-40` am Prozessor grün: Auslenkung 0 dB ab dem ersten Block, Übergang ist
ein Crossfade, Stille ab Sample 256 exakt 0; `311/M-41` am Kern grün).
Scheitert eine Publikation, bleibt der Merkzettel unberührt: `baueVor`
(`:297-307`) rührt ihn nicht an, `publiziereVorbau` kehrt bei
`reserviere() < 0` vor `vergebeKennungen` zurück (`:332`), und die Transaktion
ruft S8 nur hinter ihrem Commit-Punkt (`NakamaTransaktion.cpp:697`).

## (c) Nebenläufigkeit und Echtzeit

`merkzettel` und `kennungsZaehler` werden an genau acht Stellen berührt, alle im
Worker: `naechsteKennung` (`:228-229`), `merke` (`:234-245`), `vergebeKennungen`
(`:263`, `:269`, `:282`), die ENDE-Marke (`:320`) und `beendeCandidate`
(`:366`). Der Audiothread liest ausschließlich `pAlt->pfadKennung`,
`pAlt->baender[i].lebenszyklus` und `pNeu.…` an `:494`, `:498` und `:530` — in
Programmen zweier Bänke, die ihm in diesem Moment gehören (die alte ist aktiv,
die neue gerade über `uebernehmeBereiten` genommen). Die Aufrufer sind
durchgängig unter demselben Schloss: jeder Weg zu `publiziereVorbau`,
`uebernehmeZustand` und `beendeCandidate` läuft über
`DspKernAusfuehrung`/`Transaktionskern` und dort über `zustandSchloss` des
Prozessors (`SondeProcessor.cpp:255`, `:281`, `:313`, `:1039`, `:1247`, `:1297`,
`:1421`); `prepareToPlay` und `reset()` nehmen zusätzlich das Callback-Schloss.
Im Stresslauf M-122 publiziert genau ein Thread (`DspGoldenTestMain.cpp:4802-4816`),
während der Audiothread nur `verarbeite` fährt.

Die Kennungen stehen in der Bank, bevor sie übergeben wird: `bank.programm = prog`
(`:335`), `generation` (`:336`), `vergebeKennungen` (`:341`), erst dann
`baenke.publiziere` (`:354`) — Zuordnung und Operation auf derselben Seite der
Release-/Acquire-Übergabe (M-52). Echtzeitfest: am Blockrand höchstens neun
64-Bit-Vergleiche und höchstens acht Kopien fester Größe
(`sizeof (BandZustand)` = 256 B, ein POD aus `DspBankPool.h:58-70`, kein
Heapglied) plus eine `monoBassZustand`-Kopie; feste Obergrenze `kSlots`, keine
Allokation, keine Sperre, kein Systemaufruf, kein neues Atomic. Die
Ruhezuweisung sind fünf Skalarzuweisungen am Übergangsende, keine Arbeit je
Sample; `Rampe::setzeSofort` (`DspKern.h:350`) setzt `schritt = 0`, `rest = 0`,
das `tick()` im ruhenden Pfad bleibt damit wirkungslos. Gemessen:
`null_allokationen_im_callback_samt_programmwechseln` grün (4000 Blöcke,
91 Blockrand-Übernahmen, Testzähler 0, Kernzähler 0) und
`bankpool_parallel_ohne_rennen (M-122)` grün (4880 Blöcke, 3000 Publikationen,
0 Invariantenbrüche).

## (d) Rotbeweise je Matrixzeile

Für M-23 bis M-56, M-94, M-95 liegt je eine Rohdatei unter
`docs/beweise/roh/` (M-23 bis M-56 lückenlos, dazu M-82, M-83, M-94, M-95). Ich
habe den Mutationswortlaut jeder Stelle gegen den Code am ZIEL gehalten; jede
`vorher:`-Zeile existiert dort wörtlich und trägt die Zusage ihrer Zeile, nicht
einen Nebeneffekt: W01 bricht `if (z.aktiv < 0) z.rampen.setzeSofort (1.0);`
(`DspKern.cpp:957`, M-23 bis M-27, M-35 bis M-38), den ruhenden Zweig
`if (z.aktiv < 0) … else …` (`:187-191`, M-32, M-33, M-34), die Zuweisung in
`beendeHoerHalt` (`:441`, M-31), die E-17-Wartebedingung (`:454`, M-29, M-51),
das Rampenticken des nicht rechnenden Pfades (`:913`, M-30) und eine Allokation
an der Ruhezuweisung (M-39). W03 bricht `else if (pfadGleich)` (`:512`, Mutation
A für M-43, M-44, M-46, M-47, M-49, M-53, M-54, M-55 und Mutation E für M-45),
die Kennungsprüfung am Blockrand (M-40), den Merkzettelvergleich (`:263`,
Mutation C für M-41), die Slotkennung in der Übertragung (`:530`, Mutation F für
M-46, M-47, M-49), die Mono-Bass-Bedingung der Pfadkennung (`:268`, M-48), die
Kennungsvergabe (`:282`, M-50), die Richtung der Übertragung (`:531`, M-51), das
mitkopierte `BandProgramm` (M-94), die `monoBassZustand`-Übertragung (`:534`,
M-95) und eine Allokation an `:525` (M-56). M-52 bleibt beim
SONDE-015-Rotbeweis M-122 an `DspBankPool.cpp`. Die Maße, die der Bauer
übertragen hat, tragen die Zusage: M-25 misst am Sinus den Sprung zusätzlich zum
Eigensprung des Signals, R-311-10 erscheint in den Ausgaben von M-51 und M-94
als benannte Toleranz am größten Betrag des Laufs (M-51 Umschaltsample 0.000000000
gegen 0.001362526 + 0.000000238; M-94 (a) 0.000000012 gegen 0.001939267 +
0.000000119). Die Schranken von R-311-8 sind nirgends gerissen: M-94 (2) Spitze
0.995268 gegen 1.054241 beziehungsweise 0.250594 gegen 0.265442, (3) höchstens
0.000001 dB neben dem Referenzkern. Die Mutationsläufe selbst habe ich nicht
nachgefahren — ich baue nichts; ich habe die Rohdateien als Beleg gelesen und
die Mutationsstelle am ZIEL nachgemessen.

## (e) Grenzen

Die Fälle der Etappe 2 bleiben grün. Eigener Lauf A16 `EqCopProbeeqNullTest`:
Exit 0, 110 Prüfungen, 0 Fehler, darin `311/M-01` (166 Zeilen), `311/M-10`,
`311/M-11`, `311/M-19`, `311/M-20`, `311/M-92`, `311/M-93`; eigener Lauf B6:
`311/M-12`, `311/M-13` (9), `311/M-14`, `311/M-18` (5), `311/M-90`, `311/M-91`
grün. RBJ- und Stereo-Goldens: `docs/beweise/roh/NAK-311-etappe3-b6-zeilenvergleich.txt`
urteilt für Abschnitt C 78 von 78, G 13 von 13, H 12 von 12 Basiszeilen
bytegleich gegen den Basisstand `024d686f`, ohne Ausnahmezeile; die Abschnitte
sind in meinem Lauf grün. Die Kennungen erreichen weder State noch Draht: eine
Suche nach `lebenszyklus`, `pfadKennung`, `merkzettel`, `kennungsZaehler`,
`vergebeKennungen`, `naechsteKennung` über `eq-copilot/` und `broker/` trifft
außerhalb von `DspKern.cpp`, `DspKern.h`, `DspProgramm.h` und den beiden
Testdateien nur einen Kommentar in `PluginProcessor.cpp:7` („Brokerlebenszyklus").
`BandProgramm` und `DspProgramm` sind Laufzeitstrukturen; persistiert und
gehasht wird `param::DspSatz`. Keine neue Stateversion, keine v3-Vertragsänderung,
kein Schema- und kein Identitätspfad im Änderungssatz.

Das Leeren des Merkzettels an einer verdrängten ENDE-Marke bricht keine Zusage,
die weiter gilt. „Aus und gleich wieder Ein ohne Audio dazwischen" läuft damit
als Crossfade statt als Rampe mit Zustandsübernahme; E-31 hält dabei baulich,
weil das erste Fadesample die Quellbank allein trägt (`:925-931`), und M-07
(„die Sonde beginnt neu") stützt den kalten Start ausdrücklich. Kein bestehender
Fall misst diesen Weg, und B6, B7 und A16 sind grün. Ebenso ohne Wirkung ist,
dass `bereiteVor` und `freigeben` den Merkzettel stehen lassen (§28.9 Punkt 7):
beide setzen die Pfade über `ruhe()` in die Ruhe, die erste Publikation danach
wird mit `alt < 0` genommen und überträgt nichts, jede spätere Übertragung
vergleicht nur Kennungen aus der Zeit danach. `311/M-48 (b)` misst den Fall mit
Ratenwechsel und ist grün.

## Befunde

**DEFEKT:** keiner.

**LÜCKE 1 — Mono-Bass-Frequenzwechsel zusammen mit einem Fremdslotwechsel.**
Die Pfadkennung trägt nach F-8 nur die Mono-Bass-Stufe *an oder aus*
(`DspKern.cpp:268`), nicht die Eckfrequenz; `rampenKompatibel` hält es ebenso
(`DspProgramm.cpp:146`). Wechselt `mono_bass_hz` von 120 auf 500 Hz und ändert
in derselben Publikation ein anderer Slot seine Topologie, läuft ein Crossfade
und `:533-534` überträgt den `monoBassZustand` des alten Hochpasses in den neuen
mit anderen Koeffizienten. Gate, Matrix (M-95, R-311-9, F-8) und Entwurf sagen zu
diesem Fall nichts. Er bricht keine Zusage: der Rampenweg überträgt den Zustand
bei reinem Frequenzwechsel schon am Basisstand (`:510`, unverändert), und
R-311-8 hat für Slots mit gleichzeitiger Wertänderung genau diese Regel
entschieden (übertragen, mit den neuen Koeffizienten weiterrechnen, über den
Crossfade einblenden). Benannt, keine Nacharbeit.

**LÜCKE 2 — beiderseits hard-bypasstes Programmpaar.** `vergebeKennungen`
vergleicht `hardBypass` auf *Gleichheit* (`:267`), `rampenKompatibel` verlangt
auf beiden Seiten *falsch* (`DspProgramm.cpp:144`). Zwischen zwei nacheinander
publizierten, beide hard-bypassten Programmen ist deshalb `pfadGleich` wahr,
`nurRampen` falsch, und der Übertragungszweig `:512-535` läuft. Wirkung hat das
keine: ein Pfad im Hard-Bypass ist `istPassthrough` und rechnet keine Bänder
(`:903`), die Zustände beider Bänke stehen seit `publiziereVorbau`
(`bank.zustaendeNullen()`, `:347`) auf null, kopiert werden Nullen. Der Übergang
vom oder in den Bypass wechselt die Pfadkennung und überträgt nichts. Die Matrix
nennt für dieses Paar keine Zeile. Benannt, keine Nacharbeit.

**HÄRTUNG:** keine.

## Geprüft

Selbst an der Quelle und an laufenden Binaries gemessen: HEAD vor und nach dem
Lauf (`f11c3742e8a24af91a845ec4c4ae7f950eb5c27b`, unverändert, Worktree
unverändert bis auf die untracked Ordner `briefing-hub/` und `nimbalyst-local/`,
die schon zu Beginn dastanden); der vollständige Diff des Prüfbereichs; der
Quelltext von `DspKern.cpp`, `DspKern.h`, `DspProgramm.h`, `DspProgramm.cpp`,
`SondeProcessor.cpp` an den betroffenen Stellen sowie die Aufruferkette über
`NakamaTransaktion.cpp` und `SondeProcessor.cpp` bis zu den Schlössern; die
Abschnitte P und Q von `DspGoldenTestMain.cpp` samt Orakel `frischerKern`,
`ausUndEin` und `pruefeWieM23`; die Suche nach jedem Zugriff auf `z.aktiv`,
`z.uebergang`, `setzeSofort` und die vier Kennungsbezeichner über den Kern und
über `eq-copilot/` und `broker/`; die Dateien der beiden Etappencommits
(`git show --name-only`). Eigene Läufe der gebauten Binaries unter
`eq-copilot/build/plugin/*_artefacts/Release/` (alle vom 20.09.2026 00:24, jünger
als ihre Quellen vom 20.09.2026 00:10 und 00:18): B6 `EqCopDspGoldenTest`
Exit 0, „339 geprueft, 0 Fehler"; B7 `EqCopTransactionTest` Exit 0,
„179 geprueft, 0 Fehler", darin `311/M-34` und `311/M-40`; A16
`EqCopProbeeqNullTest` Exit 0, „110 Pruefungen ok, 0 Fehler".

## Nicht geprüft, nur gelesen

Die Mutationsläufe der Rotbeweise selbst (ich baue nichts): gelesen wurden die
Rohdateien `NAK-311-rot-M-23.txt` bis `-M-56.txt`, `-M-82.txt`, `-M-83.txt`,
`-M-94.txt`, `-M-95.txt` als Beleg; nachgemessen habe ich nur, dass jede
Mutationsstelle am Code des ZIEL wörtlich existiert und die Zusage ihrer Zeile
trägt. Ebenfalls nur gelesen: der B6-Zeilenvergleich gegen den Basisstand
(M-82, M-83), die Kostendatei `NAK-311-etappe3-kosten.txt`, der jüngste
Kanonlauf („GRUEN - 69/69 Kanon-Laeufe bestanden") aus
`NAK-311-125c83e-dirty.md`, die Messungen der Phase 16 und die Abschnitte §2,
§5.3 bis §5.5, §6.2, §6.3, §6.5, §7.0, §7.1, §7.3, §9.1 F-5 bis F-9, §12.2 und
§28.9 des Manifests. Nicht Prüfgegenstand und nicht angefasst: der Nulltest im
Host, der Laufzeit-Arm, der Abschlusskanon, die Etappen 4 und 5, `docs/**` als
Anforderungsquelle.

HEAD am Ende: `f11c3742e8a24af91a845ec4c4ae7f950eb5c27b`.
