# NAK-246 — Quellvalidierung der Wiederprüfung 1 (P1)

Lesender Agent, HEAD `05c04622`, nichts geändert, nichts gebaut, nichts gefahren.
Alle Zeilenangaben gelten zu diesem Stand (die Zeilen im Prüfauftrag und in
§13.2/§13.4 stammen von älteren Ständen und sind hier umgerechnet).

Kurzbelegte Stellen:

| Symbol | Stelle |
|---|---|
| Swap `bestaetigteSourcesCommands` | `Ipc.cpp:1394` unter `sourcesCommandMutex` `:1393` (Funktion `:1389-1396`) |
| ACK-Push | `Ipc.cpp:1074` unter `sourcesCommandMutex` `:1069` |
| Rahmen | `Ipc.cpp:1473-1521`: `sourcesDrainMutex` `:1484`, Abholen `:1485`, Generationslesung `:1493`, Haken `:1498-1499`, `bindungMutex` `:1509`, Vergleich `:1510`, Anwendung `:1515-1517`, Nachführung `:1520` |
| Speicher-Drain | `State.cpp:82` / `:83` / `:90` / `:91` / `:92` / `speichere :100` / Nachführung `:102-103` |
| Reload | `State.cpp:106-185`: Leeren `:115-119`, `vergleichszustandLeeren :131`, read-only-Zweig `:133-164` (`pipe.stop :139`, `bindungMutex :142`, Tausch `:143`, `fetch_add :151`, `projektReload :160`), Vollrestore (`bindungMutex :168`, Tausch `:169`, `fetch_add :172`, `projektReload :179`) |
| Nachführung | `Ipc.cpp:1430-1471`: `bindungMutex :1449`, früher Ausstieg `:1450-1454`, Kopie `:1455`, Haken `:1459-1460`, Publikation `:1461`, Dirty/Revision `:1469-1470` |
| Modell | `SourcesModel.cpp:360` (mutex) / `:368` (Generationsvergleich) / `:370-371` (Idempotenz); `projektReload :412-429`, mutex `:423`, Generation `:428` |

---

## (a) Ist das beschriebene Fenster an der Quelle vorhanden? — **JA**

Schritt für Schritt (Thread in Klammern):

1. (Worker, `Analyse.cpp:347`) betritt den Rahmen `Ipc.cpp:1473`, nimmt
   `sourcesDrainMutex` `:1484`.
2. (Worker) `bestaetigteSourcesCommandsAbholen()` `:1485` nimmt
   `sourcesCommandMutex` `:1393`, **swappt** den Batch `:1394`, gibt die Sperre
   mit `:1396` wieder frei. Ab hier liegt der bestätigte `unbind_probe(A)` nur
   noch lokal auf dem Workerstack.
3. (Worker) wird zwischen `:1396` und `:1493` unterbrochen. In diesem Fenster
   hält er **nur** `sourcesDrainMutex` — und `setStateInformation` nimmt diese
   Sperre nie (`State.cpp:106-185` enthält keinen `sourcesDrainMutex`;
   `git grep -n sourcesDrainMutex` listet in `State.cpp` nur `:72` (Kommentar)
   und `:82` (Speicher-Drain)).
4. (Message) `setStateInformation` läuft **vollständig** durch: Leeren beider
   Listen `:115-119` (erreicht den lokalen Batch nicht),
   `vergleichszustandLeeren :131`, `bindungMutex :168`, `zustand = geladen`
   `:169`, `reloadGeneration` G→G+1 `:172`, Freigabe `:178`,
   `sourcesModel.projektReload (geladen.mainProjectMitglieder, G+1)` `:179`.
5. (Worker) setzt fort: `generationBeimAbholen = reloadGeneration.load()`
   `:1493` liefert **G+1** — die NEUE Generation.
6. (Worker) Haken `:1498-1499` (im Produkt leer), `bindungMutex :1509`,
   Vergleich `:1510`: `G+1 == G+1` → **passiert**.
7. (Worker) `:1515-1517` wendet den alten Batch an. Der innere Riegel
   `wendeSourcesCommandAnUnterBindung` `:1398-1413` prüft `nurLesen`, Klasse,
   `projectBindingId`, `v3SessionEpoch` — bei denselben Bytes und derselben
   Instanz passieren alle vier (`v3SessionEpoch` ist über die Prozessorlebenszeit
   konstant; der Kommentar `:1409-1412` sagt genau das). `A` wird aus
   `zustand.mainProjectMitglieder` entfernt `:1417-1418`, `geaendert = 1`.
8. (Worker) `:1520` → `meldeSourcesMitgliederNachBefehl (G+1)`: Vergleich
   `:1450` passiert (`G+1 == G+1`), Kopie ohne A `:1455`, Publikation `:1461`
   gegen das Modell — und Dirty `:1469` und Revision `:1470` steigen,
   **unmittelbar nach dem Laden**. `sourcesBatchNachReloadVerworfen` bleibt 0.

Damit ist der Reviewer-Satz an der Quelle bestätigt: die Generation wird
**nach** dem Swap gelesen (`:1394` → `:1493`) und die einzige Sperre, die beide
Punkte klammert (`sourcesDrainMutex`), ist gegen `setStateInformation` wirkungslos.
Die Zusage, gegen die das fällt: `Sonde012ProjectReloadTest.cpp:1368-1372`
(„das geladene Mitglied BLEIBT Mitglied des `zustand` UND des Modells"),
`Ipc.cpp:1409-1412`, `PluginProcessor.h:1135-1143`.

Und der Reviewer hat auch darin recht, dass M-38 das Fenster **nicht** misst:
der Haken sitzt bei `:1498-1499`, also **hinter** der Generationslesung `:1493`;
der Test kann den Reload nur zwischen `:1493` und `:1510` schieben, und genau
den Fall fängt der Vergleich `:1510`.

## (b) Der zweite Satz des Reviewers — **JA, nachvollziehbar**

Dieselbe Vorgeschichte bis Schritt 7 (a), aber der Worker läuft nicht bis
`State.cpp:179` durch, sondern überholt es: die Nachführung
`Ipc.cpp:1461` erreicht `SourcesModel::setzePersistenteMitglieder`
(`SourcesModel.cpp:349-410`) **vor** `projektReload` (`State.cpp:179`).

- `SourcesModel.cpp:360` nimmt den Modell-`mutex`, `:368` vergleicht gegen
  `reloadGeneration` des **Modells**. Das Modell steht zu diesem Zeitpunkt noch
  auf **G** (gesetzt wird es erst in `projektReload`, `SourcesModel.cpp:428`).
  Die Publikation trägt G+1 ⇒ `G+1 != G` ⇒ `return false` `:369`.
- `Ipc.cpp:1461-1467`: die Publikation unterbleibt, Zähler
  `sourcesNachfuehrungNachReloadUnterblieben` +1, **kein** Dirty, **keine**
  Revision.
- Danach `State.cpp:179` → `SourcesModel.cpp:423-441` setzt
  `reloadGeneration = G+1` und baut `persistenteMitglieder`/`eintraege` aus den
  **geladenen** Mitgliedern neu — also wieder **mit A** (`:431-441`).
- `zustand` dagegen hat A in Schritt 7 verloren (`Ipc.cpp:1417-1418`).

Ergebnis: Modell trägt A, `zustand` trägt A nicht. Exakt der Satz des
Reviewers. Die Gegenreihenfolge (Publikation nach `projektReload`) ergibt den
Fall aus (a): Modell und `zustand` sind beide ohne A, obwohl der geladene State
A trug, plus Dirty und Revision. Beide Ausgänge haben dieselbe Ursache
(Schritt 3/5), es ist kein zweiter Befund — die Einordnung des Reviewers stimmt.

## (c) Sperrenordnung, vollständig gemessen

Alle Stellen aus `git grep -n sourcesCommandMutex eq-copilot/plugin/src`
(sieben in `Ipc.cpp`, eine in `State.cpp`):

| # | Stelle | Thread | beim Nehmen bereits gehalten | innerhalb der Sperre danach genommen |
|---|---|---|---|---|
| 1 | `Ipc.cpp:1069` (ACK-Push `:1074`) | Control-Reader (`Verbindung.cpp:838-839`) | **keine** — `zustandMutex`-Blöcke `Verbindung.cpp:806`, `:821`, `:828-831` sind geschlossen; `CallbackSchleuse::betreten` `Schleuse.h:121-139` nimmt keine Sperre (nur Atomics) | keine; `return :1077` liegt im Block, der Modellaufruf `:1088` dahinter |
| 2 | `Ipc.cpp:1333` (`emplace :1341`) | Message | keine — `bindungMutex`-Block `:1300-1311` ist geschlossen | keine; `sendePersistenzP0 :1350` (Client-Sperren) liegt hinter `:1342` |
| 3 | `Ipc.cpp:1351` (`erase :1352`) | Message | keine | keine |
| 4 | `Ipc.cpp:1393` (Swap `:1394`) | Worker (`Analyse.cpp:347`), Message (`Ipc.cpp:1113`, `State.cpp:83`) | **`sourcesDrainMutex`** (`Ipc.cpp:1484` bzw. `State.cpp:82`) | keine — Sperre endet `:1396`; `bindungMutex` erst `Ipc.cpp:1509` bzw. `State.cpp:91` |
| 5 | `Ipc.cpp:1527`, `:1534` (Testleser) | Test | keine | keine |
| 6 | `Ipc.cpp:1567` (`merkeSourcesCommandFuerTest`) | Test | keine — `bindungMutex`-Block `:1556-1561` geschlossen | keine |
| 7 | `State.cpp:116` (Leeren `:117-118`) | Message | **keine** — `bindungMutex` wird erst `:142` / `:168` genommen | keine |

Antworten auf die vier Fragen:

| Frage | Heute | Beleg |
|---|---|---|
| Pfad hält `sourcesCommandMutex` und nimmt dann `bindungMutex`? | **nein** | Zeilen 1-7 oben: jede Stelle ist ein Blattblock |
| Pfad hält `bindungMutex` und nimmt dann `sourcesCommandMutex`? | **nein** | `State.cpp:116` vor beiden `bindungMutex`-Blöcken; `Ipc.cpp:1333`/`:1567` hinter geschlossenen Blöcken |
| `sourcesDrainMutex` → `sourcesCommandMutex`? | **ja**, zweimal | `Ipc.cpp:1484`→`:1393`; `State.cpp:82`→`:83` |
| `sourcesCommandMutex` → `sourcesDrainMutex`? | **nein** | keine der sieben Stellen nimmt ihn |

Vollständigkeit halber, weil (d) darauf aufsetzt:

| Kante | Vorhanden | Beleg |
|---|---|---|
| `sourcesDrainMutex` → `bindungMutex` | ja | `Ipc.cpp:1484`→`:1509`; `State.cpp:82`→`:91` (dort zusätzlich über `speichere :100`) |
| `bindungMutex` → `sourcesDrainMutex` | nein | kein Aufrufer |
| `bindungMutex` → `SourcesModel::mutex` | **ja, einmal, lesend** | `Ipc.cpp:1214` hält `bindungMutex`, `:1216` ruft `sourcesModel.istAktuellesHauptziel` (`SourcesModel.cpp:1635`, mutex `:1637`). Das ist die in §13.5 (e) „Abweichungen Punkt 1" benannte Stelle |
| `SourcesModel::mutex` → irgendeine Prozessorsperre | **nein** | `SourcesModel.h`/`.cpp` enthalten keinen `std::function`-Rückruf und keine Referenz auf den Prozessor (`grep -n "std::function" SourcesModel.h` leer; `grep -n "Processor\|prozessor" SourcesModel.h SourcesModel.cpp` leer) — unter dem Modell-mutex läuft kein Rückruf in den Prozessor |
| `bindungMutex` → `versuchMutex` | nein | `vergleichszustandLeeren` (`Analyse.cpp:523`, `versuchMutex`) wird `State.cpp:131` **außerhalb** beider `bindungMutex`-Blöcke gerufen |

## (d) Regel R-A1 Punkt 2' — **schließt, mit zwei Auflagen**

Die vorgesehene Regel: (1) `setStateInformation` erhöht `reloadGeneration`
innerhalb des `sourcesCommandMutex`-Blocks, der die beiden Listen leert, und
dieser Block liegt innerhalb des `bindungMutex`-Blocks, der `zustand` tauscht
(in beiden Zweigen); (2) `bestaetigteSourcesCommandsAbholen` liest die
Generation unter `sourcesCommandMutex` zusammen mit dem Swap und gibt beides
zurück; (3) sonst nichts.

**Der Beweis.** Drei Operationen fassen die geteilten Listen an, und alle drei
laufen unter `sourcesCommandMutex`: der ACK-Push (`Ipc.cpp:1069-1076`), das
Leeren + Erhöhen (neu, in `State.cpp`), der Swap + die Lesung (neu, in
`Ipc.cpp:1389-1396`). Ein einzelner Mutex ordnet sie **total**. Damit gilt für
jeden Batch B mit gelesener Generation g:

- g wird im selben kritischen Abschnitt wie der Swap gelesen ⇒ g ist der Wert
  **zum Zeitpunkt des Swaps**, nicht danach. Das ist genau die Eigenschaft, die
  heute fehlt.
- Läuft der Reload-Block **nach** dem Swap-Block, dann ist g = G und die
  Generation danach G+1 ⇒ Vergleich `Ipc.cpp:1510` (bzw. `State.cpp:92`)
  scheitert ⇒ **ganzer Batch verworfen**, gezählt, kein Dirty, keine Revision.
- Läuft der Reload-Block **vor** dem Swap-Block, war die Liste beim Verlassen
  des Reload-Blocks leer; B kann nur Befehle enthalten, die **danach** gepusht
  wurden, und g = G+1 ⇒ der Vergleich passiert zu Recht. Zusätzlich kann ein
  solcher Push nur ein Befehl des **neuen** Laufs sein, weil `:117` auch
  `ausstehendeSourcesCommands` leert und `Ipc.cpp:1070-1071` ohne Zuordnung
  nichts pusht.

Die vier Reihenfolgen, ausgeschrieben (P = ACK-Push, L = Leeren+Erhöhen,
S = Swap+Lesung):

| Reihenfolge | Inhalt von B | g | Vergleich | Ergebnis |
|---|---|---|---|---|
| P, L, S | leer oder nur Post-Reload-Pushes | G+1 | passiert | richtig angewandt |
| P, S, L | enthält den Vor-Reload-Befehl | G | scheitert | ganz verworfen — der Fall aus (a)/(b) ist geschlossen |
| L, P, S | nur Post-Reload-Pushes | G+1 | passiert | richtig angewandt |
| S, P, L | Vor-Reload-Befehl (der Push landet in der bereits geswappten, jetzt leeren Liste und wird erst vom nächsten Drain geholt) | G | scheitert | ganz verworfen |

Damit gilt beides, was die Frage verlangt: enthält ein Batch einen Befehl, der
vor dem Leeren in der Liste stand, ist seine Generation kleiner als die nach
dem Reload; und ein Batch mit der neuen Generation enthält nur Befehle, die
nach dem Leeren kamen.

**Auflage 1 — die Erhöhung muss im selben `bindungMutex`-Block wie der Tausch
bleiben.** Das ist heute schon so (`State.cpp:169`→`:172`, `:143`→`:151`) und
muss es bleiben: nur dann ist „Generation alt" gleichbedeutend mit „`zustand`
noch alt". Läge die Erhöhung außerhalb von `bindungMutex` **hinter** dem Tausch,
entstünde das Fenster von (a) neu — ein Drain mit alter Generation würde den
bereits getauschten `zustand` mutieren. Die geforderte Verschachtelung ist also
`bindungMutex { zustand = geladen; { sourcesCommandMutex: clear; fetch_add } … }`.

**Auflage 2 — der Speicher-Drain muss mitziehen.** `State.cpp:90` liest die
Generation heute **nach** dem Abholen `:83`. Trivial richtig ist das nur, weil
Speichern und Laden derselbe Message-Thread sind (`State.cpp:84-89` sagt das);
mit (2) verschwindet die Sonderbegründung von selbst, weil `:83` die Generation
mitliefert. `:90` entfällt dann ersatzlos.

**Zyklenfreiheit.** Die neue Kante ist `bindungMutex → sourcesCommandMutex`.
Aus (c): heute existiert **keine** Kante `sourcesCommandMutex → X` (jede der
sieben Stellen ist ein Blattblock), insbesondere keine
`sourcesCommandMutex → bindungMutex`. Die Gesamtordnung wird
`sourcesDrainMutex < bindungMutex < sourcesCommandMutex`, daneben
`bindungMutex < SourcesModel::mutex`. Der Drain nimmt `sourcesCommandMutex`
zwar zeitlich **vor** `bindungMutex` (`:1393` vor `:1509`), gibt sie aber mit
`:1396` frei — das ist keine Kante. **Kein Zyklus.**
Zwei Randfälle geprüft: (i) `getStateInformation` und `setStateInformation`
laufen beide auf dem Message-Thread, können also nie gegeneinander blockieren;
(ii) ein Hostrückruf `meldeHostDirty → updateHostDisplay → getState` kann
`setStateInformation` nicht treffen, weil unter `bindungMutex` kein Hostaufruf
steht (siehe unten).

**M-12 („`sourcesDrainMutex` ist die einzige Klammer über Swap UND
Anwendung") bleibt wörtlich wahr.** Die neue Kante klammert in
`setStateInformation` das **Leeren**, nicht den Swap eines Drains; der Swap
`Ipc.cpp:1394` liegt weiterhin nur unter `sourcesCommandMutex`, die Anwendung
`:1515-1517` nur unter `bindungMutex`, und die einzige Sperre über beide bleibt
`sourcesDrainMutex` `:1484-1518`. Der Rotbeweis von M-12
(`Sonde012ProjectReloadTest.cpp:994`, `:1029`) fällt unverändert.

**§10.2 Punkt 1 bleibt gewahrt.** Die neue Sperre unter `bindungMutex` ist
`sourcesCommandMutex` — ein Blatt: unter ihr laufen nur `clear()` und ein
`fetch_add`, kein Hostaufruf, kein Modellaufruf, keine Allokation von Belang.
`projektReload` bleibt `State.cpp:160`/`:179` außerhalb, `meldeHostDirty` bleibt
`Ipc.cpp:1469` außerhalb, die Publikationen `Ipc.cpp:1233`, `:1275`,
`:1461`, `State.cpp:309` bleiben außerhalb. Die eine bestehende Ausnahme
(lesender Modellaufruf `Ipc.cpp:1216` unter `bindungMutex`) wird nicht berührt.

**Read-only-Zweig: ja, er muss umgebaut werden — minimal so.** Heute steht das
Leeren bei `State.cpp:115-119`, also **vor** `vergleichszustandLeeren :131`,
vor `pipe.stop :139` und vor **beiden** `bindungMutex`-Blöcken. Minimaler Umbau:

1. `State.cpp:115-119` ersatzlos entfernen.
2. Eine private Hilfe (Aufrufer hält `bindungMutex`), die genau das tut, was
   heute `:116-118` plus `:151`/`:172` tun:
   `sourcesCommandMutex` nehmen → beide Listen leeren → `reloadGeneration.fetch_add(1) + 1`
   zurückgeben.
3. Im read-only-Zweig `State.cpp:151` und im Vollrestore `State.cpp:172` den
   `fetch_add` durch den Aufruf dieser Hilfe ersetzen — je **eine** geänderte
   Zeile, die Blockstruktur `:142-156` bzw. `:168-178` bleibt wie sie ist.

Das ist alles; `vergleichszustandLeeren :131` und `pipe.stop :139` bleiben an
ihrem Platz (sie nehmen `versuchMutex` bzw. Pipe-eigene Sperren, keine der
drei hier betroffenen). Verhalten in der verschobenen Strecke `:115` → `:142`:
ein ACK, der dort einläuft, wird gleich darauf mit geleert (kein Verlust
gegenüber heute); ein Drain, der dort swappt, liest die **alte** Generation und
fällt am Vergleich — also genau richtig. Der `ignoriert`-Rückweg `:113-114`
leert wie heute nichts.

**Rest-Einschränkung, ehrlich benannt:** Punkt 2' ordnet Abholen und
Generationszuordnung; es ordnet **nicht** die Publikation. Dafür bleibt R-A1
Punkt 4' (`SourcesModel.cpp:368` unter demselben `mutex` wie `:428`)
zuständig — das ist bereits gebaut und vom Reviewer als tragend bestätigt
(Urteilszeile 1). Beide zusammen decken den Fall aus (b): der Batch wird schon
gar nicht mehr angewandt, und selbst wenn eine Kopie unterwegs wäre, fiele sie
im Modell.

**Nebenbefund für die Doku:** Manifest §5.3 Feinheit 2 behauptet
(`docs/beweise/NAK-246.md:1142-1145`) „Heute nistet kein Pfad
`sourcesCommandMutex` in `bindungMutex` oder umgekehrt". Punkt 2' (1) macht
diesen Satz bewusst unwahr. §5.3 ist append-only; die Richtigstellung gehört
in den neuen Abschnitt (wie §13.2 (7) es für §10.2 Punkt 1 schon vorgemacht
hat), zusammen mit `PluginProcessor.h:1209-1214` (Sperrenordnungsblock) und
`Ipc.cpp:1378-1387` (Kopfkommentar, der heute „`setStateInformation` … nimmt
`sourcesDrainMutex` nie. Diese Lücke schliesst die `reloadGeneration`" sagt —
mit 2' schließt sie die **gemeinsame Sperre über Swap und Lesung**).

## (e) Die verworfene Alternative (Generation nur VOR dem Swap lesen)

Aufbau: im Rahmen `Ipc.cpp` die Lesung vor `:1485` ziehen, `setStateInformation`
unangetastet lassen.

**Sicher gegen P1, aber verlustbehaftet.** Weil die Lesung dann nie neuer sein
kann als der Swap, gilt immer g ≤ g(Swap); der Vergleich `:1510` kann nur
**häufiger** scheitern, nie seltener — P1 und der Fall (b) sind damit ebenfalls
zu. Das bleibende Fenster ist das umgekehrte:

1. (Worker) `sourcesDrainMutex :1484`, Lesung g = G.
2. (Worker) wird unterbrochen — er hält nur `sourcesDrainMutex`, das
   `setStateInformation` nicht nimmt.
3. (Message) Reload läuft ganz durch, `reloadGeneration` = G+1
   (`State.cpp:172`), Listen geleert `:115-119`, Modell neu `:179`.
4. (Message) Der User bindet im **neuen** Projekt eine Sonde:
   `sendeSourcesCommand` `Ipc.cpp:1287`, Eintrag `:1341`.
5. (Control-Reader) Der Broker bestätigt, `Ipc.cpp:1074` pusht den Befehl in
   `bestaetigteSourcesCommands`.
6. (Worker) setzt fort: `:1485` swappt **diesen legitimen Post-Reload-Befehl**
   mit g = G. Vergleich `:1510`: `G+1 != G` ⇒ `sourcesBatchNachReloadVerworfen`
   +1, `return :1513`.

Der Befehl ist damit **endgültig weg**: der Swap `:1394` hat ihn aus der
geteilten Liste genommen, kein späterer Drain sieht ihn wieder, und der
Broker hat die Mitgliedschaft angewandt. Das ist wörtlich der Defekt, für den
D3 überhaupt geöffnet wurde (§1.6, Entwurf §57 „Save/Reload … erhalten richtige
Mitgliedschaft"; `CLAUDE.md` „State bleibt verlustfrei"), nur mit vertauschtem
Vorzeichen — und er ist still: der Zähler sagt „verworfen wegen Reload", was
hier eine Lüge wäre.

Punkt 2' hat dieses Fenster **nicht**: dort ist ein Post-Reload-Push per
Konstruktion in einem Batch mit der neuen Generation (Reihenfolgen „L, P, S"
und „P, L, S" in (d)). Die Verwerfung des Dirigenten ist also begründet:
2' ist nicht nur ebenso sicher, sondern zusätzlich verlustfrei.

## (f) Rotbeweis für die geschärfte Zusage M-38

**Der Haken INNERHALB von `sourcesCommandMutex` ⇒ DEADLOCK, ja.** Begründung am
bestehenden M-38-Muster: der Haken hält die Schranke auf dem **Worker**
(`Sonde012ProjectReloadTest.cpp:1321-1325`, gesetzt `:1321`, Halten `:1324`);
der **Testthread** wartet erst auf das Erreichen (`:1341`), ruft dann
`setStateInformation` (`:1349`) und gibt die Schranke erst danach frei
(`:1358`). Läge der Haken unter `sourcesCommandMutex`, hielte der Worker diese
Sperre, während `setStateInformation` sie nimmt — heute `State.cpp:116`, nach
Punkt 2' im verschachtelten Block. Der Testthread blockiert bei `:1349`
**ohne Frist**, die Freigabe `:1358` wird nie erreicht: harter Hänger, kein
Timeout-Fehlschlag. Genau deshalb liegt der bestehende Haken ausdrücklich vor
`bindungMutex` (`Ipc.cpp:1494-1497`, `PluginProcessor.h:832-836`) — dasselbe
Argument, eine Sperre weiter.

**Damit fällt die vorgeschlagene Konstruktion in der Form „Rotbau = heutiger
Stand plus Haken zwischen Swap und Lesung" nicht ganz sauber aus**, denn der
zusätzliche Haken existierte nur im Rotbau (zwei deklarierte Abweichungen statt
einer), und im Grünstand wäre seine Stelle unerreichbar — der Test bekäme zwei
verschiedene Testkörper.

**Sauberer, mit genau EINER deklarierten Abweichung und ohne Teständerung:**
Der Haken bleibt, wo er ist (`Ipc.cpp:1498-1499`, außerhalb
`sourcesCommandMutex`, vor `bindungMutex`). Der **Rotbau verschiebt die
Generationslesung aus dem Abholblock hinter den Haken** — eine Zeile, genau die
Zeile, die die geschärfte Zusage trägt („Abholen und Generationszuordnung sind
gegen den Reload geordnet"):

| Stand | Reihenfolge im Rahmen | Verhalten im M-38-Fall |
|---|---|---|
| Grün (2') | Swap + Lesung atomar unter `sourcesCommandMutex` → Haken → `bindungMutex` → Vergleich | g = G, nach dem Reload G+1 ⇒ Vergleich `:1510` scheitert ⇒ Batch ganz verworfen, `A` bleibt |
| Rot | Swap → Haken → Lesung → `bindungMutex` → Vergleich | g = G+1 (der Reload lief am Haken) ⇒ Vergleich passiert ⇒ `unbind(A)` wird angewandt |

Der Rotbau fällt damit an der Zusagezeile `Sonde012ProjectReloadTest.cpp:1368-1372`
(„das geladene Mitglied BLEIBT Mitglied des `zustand` UND des Modells"),
zusätzlich an `:1373-1375` (Verwerfzähler 0 statt 1) und `:1376-1381`
(Dirty/Revision steigen). Der Testkörper von
`abgeholter_batch_ueberlebt_den_reload_nicht` (`:1306-1386`) bleibt **unverändert** —
er hält den Drain bereits am richtigen Haken und fährt den Reload aus dem
Testthread (`:1341`, `:1349`, `:1358`), also über Ereignisse, nicht über Zeit.
Das Interleaving ist deterministisch erzwungen, weil der Haken im Rotbau
zwischen Swap und Lesung liegt und die Schranke erst nach `setStateInformation`
freigegeben wird.

Ergänzend: derselbe Testkörper misst am Grünstand **auch** das heutige Fenster
nicht mehr, weil es keines mehr gibt — die Zusage wird dadurch stärker, nicht
schwächer, und M-38s bisherige Grünzeilen bleiben gültig.

## (g) Betroffene Pfade einer Nacharbeit 2 und Ticketgrenze

| Pfad | Was | In der Ticketgrenze? |
|---|---|---|
| `eq-copilot/plugin/src/prozessor/State.cpp` | `:115-119` entfernen; Hilfe (Leeren + `fetch_add`) in beiden `bindungMutex`-Blöcken `:151`/`:172`; `:90` entfällt | **ja** (Kopf Zeile 16, wörtlich genannt) |
| `eq-copilot/plugin/src/prozessor/Ipc.cpp` | `bestaetigteSourcesCommandsAbholen :1389-1396` liefert Batch + Generation; `:1493` entfällt; Kopfkommentar `:1378-1387`; Kommentar `:1488-1492` | **ja** |
| `eq-copilot/plugin/src/PluginProcessor.h` | Signatur `:1134`, Doc-Block `:1119-1134`, Sperrenordnung `:1202-1214`, `reloadGeneration`-Block `:1169-1182`, ggf. Deklaration der neuen Hilfe | **ja** |
| `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` | Testkörper unverändert; nur Kopfkommentar `:1275-1298` und die Zusagetexte `:1300-1305` schärfen | **ja** |
| `docs/beweise/NAK-246.md` | neuer Abschnitt (§13.6), M-38-Zeile geschärft, Richtigstellung zu §5.3 Feinheit 2 (`:1142-1145`) und §10.2 Punkt 1 | **ja** |
| `docs/beweise/roh/NAK-246-rot-M-38.txt` u. a. | Rotbeweis ersetzen, Beine einzeln | **ja** (`docs/beweise/roh/NAK-246-*`) |

**Nicht** betroffen und damit auch nicht anzufassen: `SourcesModel.h/.cpp`
(Punkt 4' bleibt wie gebaut), `Analyse.cpp` (nur Aufrufer, `:347` unverändert),
`Sonde012SourcesModelTest.cpp`, Schemas und Fixtures (`eq-copilot/schemas/`,
`eq-copilot/fixtures/` stehen ausdrücklich in „Nicht angefasst", Kopf Zeile 17)
— die Änderung berührt keinen Wire-, Schema- oder State-Wert. Betroffene Beine
nach bisherigem Muster: B14 (`--nur m38`, `--nur m39`, `--nur m39ro`, voll),
B10, B13, B2, B8, B23, B29, A12, A1, A16, Bau.

**Alle Pfade einer Nacharbeit 2 liegen innerhalb der Ticketgrenze.**
