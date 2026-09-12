# Paket 09 — Adversarialer Abschlussreview der FeatureEngine-Zeitbeweise

Stand: 27.08.2026, final gelesene Fassung

- `FeatureEngine.h`: SHA-256
  `D968770ED285B8D8E7EFC3BBA9A1D4CD1D7A6BB85F9CF4AB810CBDB67E82F598`
- `AnalysisGoldenTestMain.cpp`: SHA-256
  `9021D4CC6B13030A8AC9D449C02FF8297153C9A7FE7268BCAA7E7316BF3CF6CC`
- fuer den Produktions-Callgraph mitgelesen: `PluginProcessor.cpp`, SHA-256
  `82659A269EE3C6D135664E0402FAED91F3E6161E653C24A563C22090AE3466DE`

Dieses Paket ist die verlangte read-only Gegenpruefung. Der Pruefer hat keine
Produktdatei geaendert; neu ist nur dieses Review-Paket. Der installierte
`c-review`-Skill wurde als Bugklassenraster benutzt. Dessen vorgesehene
`Workflow`-/`AskUserQuestion`- und False-Positive-Judge-Pipeline ist in dieser
Laufzeit nicht verfuegbar, daher ist die Einstufung eine manuelle
Reviewerentscheidung und kein unabhaengig adjudiziertes Scanergebnis.

## Abschlussurteil

In der finalen Fassung wurde **kein kritischer oder hoher Fehler** im neuen
Projekt-/Continuous-Zeitbeweis, im 50-%-FFT-Overlap oder im
Reset-/Ablehnungspfad gefunden. Die waehrend des Reviews gefundenen harten
Fehler wurden im laufenden Gegenpruefzyklus geschlossen und mit roten Beinen
versehen. Es bleibt eine mittlere Vertrags-/Ankerfrage: Einige optionale
Host-Snapshots koennen zeitlich zu einem spaeteren Block gehoeren als der
publizierte, durch FFT-Support vorgezogene Frameanfang. Daneben bleiben zwei
niedrige Robustheits-/Testgrenzen.

## Verbleibende Befunde nach Schwere

### MITTEL — Frameanker und optionale Host-Snapshots koennen verschiedene Zeitpunkte bezeichnen

`baueStempel()` zieht den gemeinsamen lokalen und Projekt-/Continuous-Anker
bis zum fruehesten tatsaechlich beitragenden Live- oder Evidenz-FFT-Fenster
zurueck (`FeatureEngine.h:1787-1812`). Das ist fuer die Bandwerte und die beiden
Zeitachsen richtig. `recording`, rohe Cycle-Angaben und beide Presentation-
Latencies stammen dagegen unveraendert aus `b`, also aus dem ersten Block des
aktuellen **skalaren** Live-Rahmens (`FeatureEngine.h:1838-1890`).

Ein Recording-Wechsel ist keine Analysegrenze; die Beweis-/Kantenmaske trennt
Kontext, Projektzeit, Play-Validity, Offline und Play-State, aber nicht
Recording, Cycle oder Latenzen (`FeatureEngine.h:1012-1022`). Dadurch ist etwa
folgende gueltige Folge moeglich:

1. Ein 4096er Fenster beginnt lokal bei 2048 mit `recording=false`.
2. Der zweite Skalarrahmen beginnt bei 5120 mit `recording=true`.
3. Der Frame wird wegen des Overlaps als `project_sample_start=2048,
   sample_count=8192` publiziert, traegt aber `recording=true` vom Block 5120.

Wenn diese Felder laut Vertrag Zustand **am Frameanfang** sind, ist das eine
falsche Zuordnung. Falls sie stattdessen einen Snapshot am Beginn des
Skalarrahmens oder am Publikationszeitpunkt darstellen sollen, fehlt dem
Vertrag ein zweiter Anker beziehungsweise eine ausdrueckliche Semantik. Die
vorhandenen All-Seven- und Bridge-Tests halten diese Felder ueber den gesamten
Lauf konstant (`AnalysisGoldenTestMain.cpp:1284-1313`, `2201-2218`) und koennen
diese Abweichung daher nicht rot machen.

Kleinste sichere Richtung: Semantik zuerst festlegen. Bei
Frameanfangssemantik die Felder am fruehesten Support mitschieben oder ihre
Validity bei vorgezogenem Support ohne Konstanzbeweis weglassen. Bei
Snapshotsemantik einen separaten Snapshot-Anker in Vertrag und Tests
ausdruecken; `project_sample_start` darf nicht stillschweigend beide Zeitpunkte
bedeuten.

### NIEDRIG — Der lokale `uint64`-Strom hat an drei Stellen nur Modulo-, keinen Wrap-Beweis

Die Projekt-/Continuous-Rechnung ist checked; der lokale Strom addiert dagegen
unguarded:

- Anschlusspruefung `alt.stromVon + alt.sampleCount`
  (`FeatureEngine.h:1043-1047`),
- Fenstereintritt `block.stromVon + sampleOffset`
  (`FeatureEngine.h:1381-1386`),
- Fortschritt nach dem 50-%-Hop
  (`FeatureEngine.h:1441-1456`).

Unsigned Overflow ist definiertes Modulo und daher kein C++-UB. Er zerstoert
aber am Wrap die numerische Ordnung, auf der `supportVereinen()` und die
Extent-Subtraktion beruhen. Der Produktionswert kommt aus einer vorallokierten,
auf 16.384 Frames gedeckelten Queue; der Wrap ist bei Audio-Sampleraten
astronomisch entfernt. Darum ist dies kein Releaseblocker, wohl aber eine
offene defensive Randbedingung der oeffentlich direkt testbaren Engine.

Minimal: checked lokale Addition verwenden und einen Wrap wie eine lokale
Kontinuitaetsgrenze behandeln. Ein Direkt-Golden bei
`stromVon=UINT64_MAX-k` sollte beweisen, dass kein modulo-sortierter Support
publiziert wird.

### NIEDRIG — Exakte gueltige `int64`-/NAK-29-Raender sind nicht separat regressionsfest

Der Code prueft den oberen Rand korrekt: `projektVorwaerts()` konvertiert erst
nach `delta <= INT64_MAX`, prueft `start <= INT64_MAX-delta` und addiert erst
dann (`FeatureEngine.h:854-863`); Blockspannen verwenden dieselbe Form
(`FeatureEngine.h:1237-1255`). Negative Starts koennen bei einer reinen
Vorwaertsaddition nicht unterlaufen. Die Tests belegen einen **ungueltigen**
Start nahe `INT64_MAX` (`AnalysisGoldenTestMain.cpp:1233-1249`) und einen
Continuous-Gegenpfad (`1099-1137`), aber nicht separat:

- einen gueltigen Projekt-/Continuous-Rahmen ab `INT64_MIN`,
- den exakt noch gueltigen Fall `start == INT64_MAX - sample_count`,
- `sample_count == 1.048.576` als erlaubte Obergrenze.

Insbesondere setzt der als „Continuous-Time-Unterlauf“ beschriftete Fall zuvor
fehlende Continuous-Werte und danach in jedem Block erneut `INT64_MIN`; er
faellt somit auch wegen fehlendem Overlap beziehungsweise stehender Uhr und ist
kein isolierter Randbeweis. Das ist keine gefundene Fehlrechnung, aber eine
Mutation am `<=`-Rand koennte ohne eigenes rotes Bein durchkommen.

Minimal: drei kleine positive Randgoldens neben den vorhandenen negativen
Faellen ergaenzen.

## Im Review geschlossene Befunde

- **Mehrblock-Projektzeit:** Der skalare Rahmen merkt seinen ersten Block und
  erhaelt Projektzeit nur bei exakt fortlaufenden, darstellbaren Blockspannen
  (`FeatureEngine.h:1281-1312`). Stehende 1-Sample-FL-Teilstuecke fallen ehrlich
  auf `local_monotonic` zurueck (`AnalysisGoldenTestMain.cpp:1213-1230`).
- **FFT-Event- und Bandzeit:** Projekt- und Continuous-Zeit samt Validity laufen
  pro Sample in beiden Ringen. Ein voller Ring beweist jede `+1`-Kante; beim
  50-%-Overlap werden Audio und alle vier Zeitmetadaten gemeinsam kopiert
  (`FeatureEngine.h:1381-1456`). Events verwenden erst danach den bewiesenen
  Fensterstart.
- **Continuous statt Rueckrechnung:** Es gibt keine lokale Rueckextrapolation
  mehr. Ein fehlender Wert im alten Overlap laesst nur das optionale Feld
  fallen (`AnalysisGoldenTestMain.cpp:1063-1137`); der Offsetfall
  `continuous=project+12345` bleibt am gemeinsamen Frameanker erhalten
  (`1284-1313`).
- **Band-inaktive Evidenzluecke:** `evidenzContinuousBelegen()` latcht ueber
  jeden Block des gesamten Evidenzintervalls, nicht nur ueber aktive
  FFT-Beitraege (`FeatureEngine.h:1258-1278`, `1794-1802`). Das 192-kHz-Bein
  spuelt beide FFT-Ringe mit Stille leer, entfernt Continuous in der stillen
  Mitte und verlangt im spaeteren frischen Snapshot ein geloeschtes Bit
  (`AnalysisGoldenTestMain.cpp:1172-1210`). Dieser Test ist nicht tautologisch:
  ohne den Evidenz-Latch bleibt der Anfangs-/Endoffset affine und das Bit wird
  gesetzt.
- **Live-/Evidenz-/Bassvereinigung:** Jede Achse wird unabhaengig vereinigt und
  bleibt nur gueltig, wenn die lokalen Startdeltas dieselbe affine Abbildung
  ergeben (`FeatureEngine.h:868-912`). Der zweite frische Evidenzsnapshot muss
  den alten Basssupport als exakt `0 + 30720` tragen und zugleich echte
  Evidenzbaender enthalten (`AnalysisGoldenTestMain.cpp:1140-1169`). Das Bein
  wird sowohl durch Entfernen des Evidenzzweigs als auch durch Ignorieren der
  Bassstufe rot.
- **Ablehnung verbraucht Evidenz:** Bei Regel-7-Ablehnung werden Live-Rahmen und
  bei faelliger Evidenz auch Akku, Support und Continuous-Latch konsumiert
  (`FeatureEngine.h:1704-1719`, `1745-1752`). Der direkte >1-Mi-Sample-Fall
  erreicht den echten Erzeugerzweig und beweist Erholung
  (`AnalysisGoldenTestMain.cpp:1898-1915`).
- **NaN/Inf/Reset:** Ungueltiges Folge-Prepare deaktiviert eine zuvor gueltige
  Engine; NaN/Inf und umgekehrte Cycle-Bounds werden nicht publiziert. Der
  Worker reicht auch eine ungueltige Rate an die FeatureEngine weiter und
  fuettert bis zur naechsten gueltigen Rate keine alten Analysezustande
  (`PluginProcessor.cpp:532-539`, `602-610`).

## Ausdruecklich widerlegte Kandidaten

- **Kein signed Overflow bei negativen Projektstarts:** Die einzige
  Zeitrichtung ist vorwaerts. `INT64_MAX-d` liegt fuer gueltiges `d` zwischen 0
  und `INT64_MAX`; ein negativer `start` kann deshalb die obere Pruefung
  passieren, ohne dass `start+d` unterlaeuft. `delta > INT64_MAX` wird
  konservativ verworfen, auch wenn einzelne negative Starts mathematisch noch
  darstellbar waeren. Produktionsdeltas liegen durch Frame-/Queuegrenzen weit
  darunter.
- **Startkonsistenz ist fuer die tatsaechlich vereinigten FFT-Fenster
  ausreichend:** Jedes Eingabefenster beweist bereits alle Samples und der
  Vereinigungscode prueft den affinen Abstand beider Starts. Die zuvor offene
  Luecke waren nicht beitragende Zwischenabschnitte; fuer Continuous schliesst
  sie jetzt der separate Evidenz-Latch. Projekt-Beweislagewechsel ziehen eine
  Grenze, stehende Projektzeit kann nur durch einen erkannten Sprung wieder auf
  die alte affine Achse aufholen.
- **Kein NAK-29-/Schemawiderspruch bei `sample_count=0`:** Das allgemeine
  v3-Wire-Schema erlaubt 0 (`eq-ipc-v3.schema.json:208`). Regel 7 ist im Code
  ausdruecklich die strengere Invariante **dieses Feature-Erzeugers**, der nie
  einen Leerframe publiziert (`FeatureEngine.h:234-244`). `rg` fand keinen
  Einsatz als eingehenden allgemeinen Schema-Validator. Bei kuenftiger
  Wiederverwendung muss die producer-lokale Regel getrennt bleiben.
- **Kein ungewollter Heapzugriff pro Block:** Alle neuen Vektoren werden in
  `vorbereiten()` dimensioniert (`FeatureEngine.h:767-813`). Im Blockpfad gibt
  es nur Indexzugriffe, feste Akkus, O(N)-Validity-Scans und `std::sort` auf dem
  bereits angelegten `flussSortiert`. `FeatureEngine::nimmBlock()` wird im
  Produktions-Callgraph ausschliesslich im Worker aufgerufen
  (`PluginProcessor.cpp:494-504`, `610`). Die zusaetzlichen Zeitringe kosten
  Speicher beim Prepare, aber keine Audio- oder Worker-Allokation je Block.

## Externe Beweisgrenzen

- Der eine Transportstempel beschreibt den gemeinsamen Skalar-/FFT-Huellsupport.
  `LUFS-S` verwendet dagegen eine feste 3-s-Historie
  (`FeatureEngine.h:439`, `1669-1679`, `1973-1976`). Ob `sample_count` auch den
  vollen Support jedes Langzeit-Skalars ausdruecken soll oder ob dessen
  Fensterlaenge allein aus `metrics_version` folgt, ist eine noch externe
  Vertragsentscheidung; diese Aenderung loest keine per-Metrik-Supportfelder.
- Die Goldens simulieren FLs stehende/fragmentierte Zeit und Hostflags, ersetzen
  aber keinen aktuellen FL-Studio-Hostlauf. Insbesondere Presentation-Time/PDC
  und PPQ-zu-Sample-Abbildung bleiben laut Entwurf unbewiesen; Cycle-Derivation
  bleibt deshalb korrekt `unproven`.
- Es lief kein ASan/UBSan-Fuzzing, kein Exhaustivmodell der `uint64`-Wrapfolge
  und kein Allocation-Hook. Die Realtime-/Allokationsaussage ist Callgraph- und
  Quellcodebeweis, kein Laufzeitprofil.
- Worker-Wanduhrkadenz, Queue-Fairness und Pipe-Lifecycle gehoeren zu den
  getrennten Paketen 07 beziehungsweise 04/06 und wurden hier nicht erneut als
  FeatureEngine-Beweis ausgegeben.

## Frische Belege auf der finalen Fassung

- Release-Build des Ziels `EqCopAnalysisGoldenTest`: erfolgreich.
- `EqCopAnalysisGoldenTest.exe`: **235 bestanden, 0 Fehler**.
- `git diff --check -- FeatureEngine.h AnalysisGoldenTestMain.cpp
  PluginProcessor.cpp`: keine Whitespace-Fehler; nur die bestehende
  Git-Zeilenende-Warnung LF/CRLF.

Das gruene Golden schliesst die oben als geschlossen genannten, konkret
gefahrenen Pfade. Es entscheidet nicht die verbleibende Snapshot-Ankersemantik
und nicht die aufgefuehrten externen Host-/Vertragsgrenzen.
