# Paket 05 — Adversariale Integration: Realtime und C++-PipeClient

Stand: 27.08.2026, aktueller uncommitteter Worktree. Dieses Paket ist eine
read-only Gegenpruefung; keine Produktdatei wurde durch diesen Pruefer geaendert.
Der native `c-review`-Workflow war in dieser Laufzeit nicht aufrufbar
(`Workflow`/`AskUserQuestion` fehlen), deshalb wurde sein Bugklassenraster
manuell auf den eng vorgegebenen C++-Scope angewendet. Es lief keine
False-Positive-Judge-Pipeline.

## Ergebnis nach Schwere

### HOCH — Der Frame-Stempel behauptet ein nicht bewiesenes Projektintervall

`FeatureEngine::verarbeiteSamples()` merkt sich den ersten Block eines
Live-Rahmens (`FeatureEngine.h:1108-1112`). `baueStempel()` veroeffentlicht
anschliessend dessen `projectSampleStart` zusammen mit der Summe **aller**
Rahmensamples (`FeatureEngine.h:1494-1522`). Dazwischen wird weder bewiesen,
dass die Projektzeit ueber alle beteiligten Bloecke samplegenau fortschreitet,
noch dass `project_sample_start + sample_count` in `int64` darstellbar ist.

Eine vorhandene Regel faengt das nicht ab:

- `grenzeZwischen()` prueft die erwartete Fortsetzung nur, wenn sich der
  gemeldete Projektstart ueberhaupt aendert (`FeatureEngine.h:897-914`).
- Dieselbe Projektzeit ueber mehrere FL-Teilstuecke gilt absichtlich als
  lueckenlos; G10 beweist genau diese Produktregel
  (`AnalysisGoldenTestMain.cpp:1401-1424`).
- Der neue `projektEnde()`-Riegel schuetzt nur das separate M1-Projektfenster
  im Audiocallback (`PluginProcessor.cpp:253-297`). Der unveraenderte Stempel
  wird danach trotzdem in die Analysequeue gestellt (`PluginProcessor.cpp:300-307`).
- `nak29Verstoss()` prueft bedingte Felder, aber keinen Intervallueberlauf
  (`FeatureEngine.h:184-234`).

Damit kann ein Rahmen beispielsweise bei zehn Bloecken mit demselben gueltigen
Projektstart als `[start, start + 5120)` erscheinen, obwohl der Host diese
Abbildung nie geliefert hat. Bei `start = INT64_MAX - 100` ist das publizierte
Intervall sogar arithmetisch unmoeglich. Das ist nicht nur Diagnose: Der Entwurf
erlaubt Cross-Probe-Alignment gerade anhand kompatibler Projektzeitintervalle
(`docs/FL-Nakama-Sonden-Design-Entwurf.md:1687-1701`).

Minimaler Regressionstest:

1. Eine frische `FeatureEngine` mit ausreichend 512er-Bloecken speisen, bei
   denen `kFlagZeitGueltig` gesetzt ist, der Projektstart aber konstant bleibt.
   Der gebaute Mehrblockrahmen darf **kein** `project_samples`-Intervall
   behaupten; lokaler monotoner Stempel bleibt erlaubt.
2. Dasselbe mit `projectSampleStart = INT64_MAX - 100`; kein veroeffentlichter
   Projektstempel darf beim Addieren von `sample_count` ueberlaufen.
3. Gegenprobe mit streng fortlaufenden Starts muss weiterhin den heutigen
   Mehrblockstempel `0 + 5120` liefern.

Kleinste sichere Korrekturrichtung: pro laufendem Rahmen ein eigenes
`projektKontinuierlich`-Bit fuehren und es nur erhalten, wenn jeder Block eine
darstellbare, exakt anschliessende Abbildung belegt. Analyse bei stehender oder
fragmentierter Zeit darf weiterlaufen, aber der Rahmen muss dann auf
`local_monotonic` herabgestuft werden. Vor Publikation zusaetzlich den
`start + sample_count`-Rand pruefen.

### HOCH — FFT-Ereignisse extrapolieren Projektzeit aus dem lokalen Strom

Beim Fuellen einer FFT-Stufe wird der Projektstart nur beim leeren Fenster aus
dem aktuellen Block abgeleitet (`FeatureEngine.h:1170-1181`). Nach jedem
50-Prozent-Hop erhoeht der Code ihn blind um `hop`
(`FeatureEngine.h:1187-1203`); `flussSchritt()` setzt deshalb
`projektzeitGesetzt=true` und publiziert diesen extrapolierten Wert
(`FeatureEngine.h:1337-1351`).

Auch hier greift keine vorgelagerte Regel: stehende Projektzeit ist nach G10
ausdruecklich keine Grenze. Das Fenster darf lokal weiterlaufen, daraus folgt
aber keine samplegenaue Projektabbildung. Der aktuelle Eventtest beweist nur
den Idealfall `projektSample == stromSample` bei streng monotonen 512er-Bloecken
(`AnalysisGoldenTestMain.cpp:1719-1776`); er kombiniert ihn nicht mit G10.

Minimaler Regressionstest: den vorhandenen Eventkorpus mit konstantem
Projektstart beziehungsweise 1-Sample-FL-Teilstuecken fahren. `stromSample`
muss weiter exakt auf dem 2048er-Hop liegen; fuer einen nicht bewiesenen
Fensterstart muss dagegen `projektzeitGesetzt == false` gelten. Eine zweite
Gegenprobe mit streng fortlaufender Projektzeit behaelt den heutigen exakten
Projektstart.

Kleinste sichere Korrekturrichtung: die Projektgueltigkeit je FFT-Stufe
unabhaengig von der lokalen Fenstergueltigkeit fuehren. Jeder neue Block muss
die erwartete Projektfortsetzung bestaetigen; bei stehender/mehrdeutiger Zeit
wird nur das Projektbit des offenen Fensters geloescht, nicht das Audiofenster
und nicht der lokale `stromSample`.

### MITTEL — Der neue 8er-Drain macht die 250-ms-Kadenz lastabhaengig

Der Worker verarbeitet hoechstens acht Queuebloecke unter
`analyseSteuerMutex`, erhoeht danach aber bei **jedem** solchen Zug
`auswertTeiler` (`PluginProcessor.cpp:531-599`). Ist noch Queueinhalt vorhanden,
folgen nach `yield()` sofort weitere Zuege ohne den 50-ms-Wartetakt
(`PluginProcessor.cpp:602-610`). Unter Backlog wird `engine.auswerten()` daher
nicht mehr etwa alle 250 ms, sondern alle 40 Bloecke ausgefuehrt. Gerade FLs
belegte 1-Sample-Fragmentierung kann damit viele schwere Auswertungen in einem
kurzen Burst ausloesen. Das erhoeht CPU- und Allokationsdruck des
Hintergrundthreads und kann den Analysestau, den die Queue begrenzen soll,
selbst verstaerken.

Die Begrenzung auf acht Bloecke begrenzt zwar eine einzelne Mutex-Haltedauer,
beweist aber keine Fairness: `std::this_thread::yield()` plus sofortiges
Wiedererlangen eines `std::mutex` garantiert nicht, dass Prepare, Reset oder
ein Frame-Leser dazwischen zum Zug kommt.

Minimaler Regressionstest: mehrere hundert kleine Bloecke schneller als der
Worker einstellen, bis ein echter Queue-Rueckstand besteht. Dann zugleich

- die Snapshot-Revisionen beziehungsweise einen expliziten Schwerlaufzaehler
  ueber ein festes Wanduhrfenster messen (sie duerfen nicht mit der Anzahl der
  8er-Batches skalieren), und
- `fordereMessResetAn()` sowie `merkmalFrame()` aus einem zweiten Thread mit
  einer konservativen Latenzgrenze abschliessen lassen.

Der Test braucht eine Gegenprobe ohne Backlog, damit er nicht bloss einen
inaktiven Worker misst.

Kleinste sichere Korrekturrichtung: Leicht-/Schwerpublikation an
`steady_clock`-Deadlines koppeln, nicht an Drain-Zuege. Bei Queue-Rest wird nur
weitergedraint; eine Auswertung geschieht erst an ihrer echten 50-/250-ms-
Deadline. Fuer die Steuersperre braucht es zusaetzlich einen expliziten
Warte-/Uebergabemechanismus oder eine Snapshot-Publikation ohne denselben
unfairen Mutex; `yield()` allein ist kein Latenzbeweis.

### MITTEL — Der neue Frame-Extent umfasst nicht den Support seiner FFT-Werte

`rahmenLeeren()` setzt die Live-Akkus und `rahmenStartBlock` zurueck, laesst
aber die 50-Prozent-Overlap-Puffer der FFT-Stufen absichtlich stehen
(`FeatureEngine.h:1475-1491` gegen `FeatureEngine.h:1187-1203`). Bei 48 kHz und
512er Hostbloecken beginnt der zweite 100-ms-Rahmen bei Sample 5120; sein erster
4096er Spektralbeitrag kann jedoch das Fenster ab Sample 2048 enthalten. Der
Kommentar, `sample_count` spanne „exakt diesen Inhalt“ auf
(`FeatureEngine.h:1503-1505`), trifft damit fuer Peak/Loudness zu, aber nicht
fuer die publizierten Bandwerte. Der Entwurf verlangt, FFT-Hops auf das rohe
Projektsampleraster zu legen und nennt Fensterlaenge/Frische je Aufloesung
(`docs/FL-Nakama-Sonden-Design-Entwurf.md:1993-2005`).

Minimaler Regressionstest: vor der ersten 100-ms-Grenze einen klaren Bin-Ton,
danach Stille einspeisen und den zweiten Frame lesen. Wenn sein Stempel erst an
der Grenze beginnt, darf er keinen Bandwert publizieren, dessen FFT-Support vor
diesem Start liegt. Die Gegenprobe mit durchgehendem Ton muss weiterhin
Spektralwerte liefern.

Korrekturrichtung: nicht den Overlap zur kosmetischen Stempelpassung loeschen.
Stattdessen den tatsaechlichen Support je Aufloesung transportieren oder den
gemeinsamen Frame-Extent konservativ bis zum fruehesten beitragenden
Fensterstart erweitern. Welche Form der v3-Vertrag bekommen soll, ist eine
kleine Vertragsentscheidung; der heutige eine Stempel ist fuer gleichzeitig
4096er/16384er Support nicht „exakt“.

### NIEDRIG — Das Pipe-I/O-Zeitlimit ist pro Teiltransfer, nicht pro Frame

`ioGenau()` startet `WaitForSingleObject(..., 5000)` innerhalb der Schleife fuer
jeden partiellen Read/Write neu (`PipeClient.cpp:551-631`). Zudem bekommen
4-Byte-Kopf und Body getrennte `ioGenau()`-Aufrufe
(`PipeClient.cpp:493-548`). Ein Peer kann daher jeweils kurz vor Ablauf ein
weiteres Byte liefern und einen bis zu 16 MiB grossen Frame praktisch
unbegrenzt offenhalten. `stop()` und `reconnect()` bleiben dank `CancelIoEx`
begrenzt; betroffen ist die Liveness einer laufenden, nicht aktiv
abgebrochenen Verbindung.

Minimaler Regressionstest: mit einem testseitig verkleinerten Timeout einen
Fake-Peer Kopf und Body byteweise jeweils knapp vor dem Teiltimeout senden
lassen. Der gesamte Empfang muss an **einer** absoluten Framefrist enden, nicht
nach `bytes * timeout`.

Kleinste sichere Korrekturrichtung: in `empfange()`/`sende()` eine absolute
`steady_clock`-Deadline bilden und fuer Kopf, Body und alle Teiltransfers nur
die Restzeit weiterreichen. `CancelIoEx` und die bestaetigte Completion der
stacklokalen `OVERLAPPED`-Struktur bleiben unveraendert.

## Widerlegte Hypothesen

- **Kein neuer Audiothread-Mutex/Heapzugriff:** `processBlock()` verwendet nur
  Atomics und die vorallokierte SPSC-Queue; `analyseSteuerMutex` wird nur von
  Prepare/Reset/Worker/Gettern genommen. Die neuen Framekopien und FFTs laufen
  im Worker.
- **Kein signed Overflow mehr in der M1-Fensterdistanz:**
  `projektAbstandGroesserAls64()` bildet `int64` monoton auf `uint64` ab
  (`PluginProcessor.cpp:22-30`). Der Randtest `INT64_MIN` gegen `INT64_MAX`
  lief gruen.
- **Kein ungeschuetzter Live-Leser der FeatureEngine mehr:** die sechs Getter
  und `merkmalFrame()` teilen jetzt `analyseSteuerMutex` mit dem Writer
  (`PluginProcessor.h:148-161`). Im aktuellen Callgraph nimmt der Audiothread
  keinen dieser Getter.
- **Kein Pipe-HANDLE-ABA/Stop-Deadlock gefunden:** I/O-Submission, Cancel und
  Close teilen `handleMutex`; nach Cancel wird die Completion abgewartet, bevor
  die stacklokale `OVERLAPPED` endet (`PipeClient.cpp:568-631`, `640-645`). Die
  beiden blockierten Fake-Peer-Faelle beendeten `stop()` in 0 ms.
- **ACK/Welcome sind nicht mehr permissiv:** UTF-8/NUL, exakte Top-Level-Felder,
  Typ, Version, Token, ACK-Sequenz und Booltyp werden vor Statusuebernahme
  geprueft (`PipeClient.cpp:355-461`, `510-548`). Die vorhandenen Gegenproben
  waren gruen.
- **Nichtendliche Loudness-Zellen vergiften den 3-s-Ring nicht mehr:** das
  separate Gueltigkeitsarray wird bei Reset geloescht und von `kurz()`
  vollstaendig geprueft (`LoudnessAccumulator.h:192-229`, `346-365`).

## Ausgefuehrte Belege

- `EqCopAnalysisGoldenTest.exe`: **218 bestanden, 0 Fehler**.
- `EqCopLoudnessGoldenTest.exe`: **109 Pruefungen, 0 Fehler**.
- `EqCopPipeClientTest.exe`: **0 Fehler**; blockiertes Welcome/ACK, SQOS
  Identification, Reconnect-Generation, UTF-8/NUL, falsche ACK-Sequenz,
  schemafremde Welcome-/ACK-Felder und 200 konkurrierende Start/Stop-Paare.

Die Laeufe waren fuer die aktuellen Quellen frisch. Sie widerlegen die oben
genannten positiven Hypothesen, decken aber die vier Zeit-/Backlog-Faelle und
den partiellen Pipe-Drip nicht ab; gruen bedeutet dort deshalb nicht geschlossen.
