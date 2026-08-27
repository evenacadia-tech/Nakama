# Paket 07 — Worker-Kadenz und Steueruebergabe

Stand: 27.08.2026, aktueller gemeinsamer Worktree.

## Geschlossener Befund

Der Analyse-Worker leitete den ungefaehren 250-ms-Schwertakt aus fuenf
Workerzuegen ab. Seit ein Zug bei Rueckstau nur noch acht Bloecke verarbeitet
und danach ohne 50-ms-Wartephase weiterlaeuft, waren diese fuenf Zuege keine
Zeitmessung mehr: Gating/Kandidatensuche konnten auf CPU-Geschwindigkeit
beschleunigen.

`src/WorkerCadence.h` fuehrt deshalb zwei monotone
`std::chrono::steady_clock`-Deadlines:

- leichte Publikation: 50 ms;
- schwere Gating-/Kandidatenrunde: 250 ms.

Eine ueberfaellige Deadline wird genau einmal verbraucht und von der aktuellen
Zeit aus neu geplant. Nach Suspend/Debuggerhalt entsteht somit kein Catch-up-
Sturm. Queue-Generation, expliziter Mess-Reset und erkannte
Kontinuitaetsgrenzen starten die Kadenz neu; die neue Epoche erbt keinen fast
faelligen Schwertakt der alten. `PluginProcessor.cpp` fragt die Deadline nach
jedem begrenzten Drain-Zug ab, unabhaengig davon, ob danach gewartet oder wegen
Queue-Rest sofort weitergearbeitet wird.

`analyseSchwereAuswertungen()` ist ein monotoner Diagnosezaehler fuer echte
`AnalyseEngine::auswerten()`-Aufrufe. Er beeinflusst weder Auswertung noch
Audio und macht den verdrahteten Rueckstautest beobachtbar.

## Mutex-Fairness

Die fruehere Kombination aus `yield()` und sofortigem erneutem
`std::mutex`-Erwerb war keine Fairnessgarantie. Externe Nutzer der
Analysesteuerung melden sich nun atomar an, bevor sie den Mutex anfordern. Der
Worker prueft diese Anmeldung vor und direkt nach seinem Lock-Erwerb, beginnt
dann keinen neuen Zug und beendet einen bereits laufenden Zug nach hoechstens
dem gerade verarbeiteten Block. Eine bereits sichtbare Anfrage geht auch einer
faelligen Leicht-/Schwerpublikation vor. Damit kann der Worker einen wartenden
Prepare-/Reset-/Frame-Leser nicht durch beliebig viele eigene 8er-Zuege
verhungern lassen.

Der Audiothread nimmt weiterhin keinen dieser Locks, alloziert fuer diese
Aenderung nichts und wartet nicht. Der bestehende Ganzblock-Dropweg bleibt
unveraendert.

## Queue-Generation und publizierte Luecke als Projektfensterkanten

Der zwischenzeitliche Gegenpfad mit einem separaten
`fensterResetWunsch`-Atomic war trotz richtiger Schreibreihenfolge nicht
vollstaendig: Ein bereits laufender Callback konnte das Bit schon gelesen
haben, danach die neue Queue-Generation sehen und denselben neuen Block noch
ins alte Projektfenster schreiben.

`StampedAudioQueue::veroeffentliche()` besitzt nun den optionalen Out-Parameter
`neustartUebernommen`. Er ist bei jedem Aufruf definiert und genau in dem
Produzentenzug wahr, der `neustartWunsch` wirklich konsumiert. Das gilt auch,
wenn dieser Block nach der Generationsuebernahme wegen Form oder Kapazitaet
verworfen wird; ein Nullframe konsumiert dagegen weiterhin nichts.

Der Audiothread schreibt das Projektfenster erst nach diesem Queue-Zug fort.
Bei wahrer Rueckmeldung leert er es zuerst und nimmt dann den aktuellen Block
als ersten Block der neuen Generation auf. Das separate Fensterbit wurde
entfernt. Damit gibt es keine Sichtbarkeitsannahme ueber zwei fremde Atomics
mehr und auch ein same-rate-Prepare trennt Messung und Projektfenster an
derselben tatsaechlichen Kante.

Zusaetzlich war die boolesche Rueckgabe des Queue-Zugs zuvor ignoriert worden:
Ein wegen Oversize oder voller Queue verlorener Block erweiterte dadurch das
Projektfenster, obwohl M1 seine Samples nie sehen konnte. Das Fenster waere
damit als Messbereich irrefuehrend gewesen. Der zweite optionale Out-Parameter
`lueckeVorBlock` ist nur an einem wirklich angenommenen Block wahr, der auch
`kFlagLueckeDavor` traegt. Ein verworfener Block oeffnet die Luecke, behauptet
sie aber nicht selbst. `PluginProcessor` nimmt nur angenommene Bloecke bei
gueltiger Analyserate ins Fenster auf und beginnt es am naechsten publizierten
Lueckenblock neu. Damit enthaelt das Fenster weder Oversize-/Ueberlaufzeit noch
Zeit, in der beide Engines fail-closed deaktiviert sind.
Beim Start eines neuen Fensters werden dessen Grenzen vor dem
`fensterAktiv`-Publikationsbit geschrieben; ein paralleler Heartbeat kann die
neue Generation damit nicht schon gueltig mit den alten Grenzen sehen.

## Ungueltige Samplerate fail-closed

Eine auf 0 sanitizierte Samplerate darf keine zuvor gueltig vorbereitete
Engine unter ihrer alten Binzuordnung weiterfuettern. Der Worker reicht daher
jeden Sampleratewunsch, auch 0, an die deaktivierbare FeatureEngine weiter,
verarbeitet bei ungueltiger Rate aber weder einen freigegebenen Block in
FeatureEngine/M1 noch eine leichte oder schwere M1-Auswertung. Queue und
Quarantaene laufen weiter in begrenzten Zuegen leer. Die folgende gueltige
Generation bereitet beide Engines wieder vor.

## Geaenderte Pfade

- `eq-copilot/plugin/src/WorkerCadence.h` — deterministisch testbare monotone
  Deadline-Zustandsmaschine.
- `eq-copilot/plugin/core/StampedAudioQueue.h` — optionale Rueckmeldung der
  tatsaechlich uebernommenen Produzentengeneration und der am angenommenen
  Block publizierten Kontinuitaetsluecke.
- `eq-copilot/plugin/src/PluginProcessor.cpp` — zeitbasierte Kadenz,
  generation-gekoppeltes Projektfenster, ungueltige Rate fail-closed und
  explizite Steueruebergabe.
- `eq-copilot/plugin/src/PluginProcessor.h` — externe Lock-Anmeldung und
  Diagnosezaehler; separates Fensterreset-Atomic entfernt.
- `eq-copilot/plugin/tests/QueueStressTestMain.cpp` — neue Abschnitte Q-T fuer
  Deadlinekanten, Batchsturm, Catch-up, Reset/Generation, echten
  Produktionsring-Rueckstau, Frame-/Reset-Uebergabe, Stop/Join, das
  deterministische In-Flight-Callback-Race, verworfene Projektzeit und
  ungueltige Sampleraten.

## Frische Belege

- Release-Build `EqCopQueueStressTest`: erfolgreich.
- `EqCopQueueStressTest.exe`: **114 Pruefungen, 0 Fehler**,
  `QUEUE-STRESSTEST OK`.
  - 10.000 Zuege bei identischer Wanduhr: 0 Schwerlauf;
  - Kante 249/250 ms und kein doppelter Deadlineverbrauch: gruen;
  - langer Zeitsprung: genau ein Lauf, kein Catch-up;
  - echte volle Produktionsqueue: 1 Schwerlauf in 600 ms;
  - neue Generation: 0 Schwerlaeufe in den ersten 100 ms;
  - Frame-Leser und Reset-Steuerer unter Rueckstau: je 0 ms im Beleglauf;
  - Stop/Join mit Queue-Rest: 0 ms im Beleglauf;
  - im deterministisch blockierten Callback beginnt die uebernommene neue
    Generation das Fenster exakt bei `10000..10064`, Sprungzaehler 0;
  - eine volle Queue meldet die Luecke erst am naechsten angenommenen Block;
    ein verdrahteter Oversize-Block bleibt aus dem Fenster, dessen Folgeblock
    beginnt exakt bei `16385..16449`, Sprungzaehler 0;
  - NaN-Samplerate: M1-Samples 0 und Feature-Bloecke 0; folgende 48-kHz-
    Generation aktiviert beide wieder; das Projektfenster bleibt dabei
    ungueltig.
- Der aktuelle 114er-Stand lief danach fuenfmal hintereinander: jeder Lauf
  **114/0** und `QUEUE-STRESSTEST OK`.
- Release-Build `EqCopNullTest`: erfolgreich; `NULLTEST OK` einschliesslich
  bitgleichem 1.000-Block-Passthrough, NaN/Inf-Gegenpfad, 0 Latenz und 0 Tail.
- Release-Build `EqCopAnalysisGoldenTest`: erfolgreich;
  **235 bestanden, 0 Fehler**, einschliesslich Feature-Grenzen,
  Samplerate-Deaktivierung/-Erholung und verdrahtetem Prozessorweg.
- Gezieltes `git diff --check` auf den fuenf Produkt-/Testpfaden: keine Fehler
  (nur die bestehende Git-Zeilenende-Warnung LF/CRLF).

## Verbleibendes Risiko

Die Uebergabe verhindert Worker-Wiedererwerb, ist aber keine harte
Wanduhr-SLA des Betriebssystems. Trifft eine externe Anfrage erst nach der
letzten Anmeldungskontrolle ein, wartet sie noch auf die aktuell laufende,
endliche kritische Operation (hoechstens ein Block beziehungsweise eine schon
begonnene Auswertung) und auf Windows-Scheduling. Umgekehrt kann ein exakt
zwischen Anmeldung und Mutex-Erwerb deschedulter externer Thread die
Hintergrundanalyse kurz pausieren. Beides betrifft nur Analyse/UI-Steuerung,
nie den Audio-Thread oder das unveraenderte Audio. Der Stresstest belegt den
konkreten heutigen Pfad, ersetzt aber keinen formalen Scheduler-Hardbound.

Das Projektfenster folgt jetzt exakt den vom Produzenten angenommenen
Kontinuitaetssegmenten, nicht jedoch dem spaeteren Versiegelungszeitpunkt der
Ein-Block-Quarantaene: Der juengste angenommene Block kann deshalb noch im
Fenster stehen, waehrend M1 ihn bis zum Nachfolger bewusst zurueckhaelt. Eine
vollstaendige Gleichheit mit den bereits verarbeiteten M1-Samples braeuchte
ein worker-eigenes, separat publiziertes Fenster und waere kein kleiner
Audio-Thread-Fix. Verlorene Bloecke und ihre Zeit werden dagegen nachweislich
nicht mehr als gemessen ausgegeben.

Konsequenz der verlangten tatsaechlichen Produzentenkante: Ein Resetwunsch
allein leert das Fenster nicht vorzeitig. Bleibt danach jeder echte
Audioblock aus, wird die wartende Queue-Generation nicht uebernommen und das
alte Fenster bleibt sichtbar, waehrend der Worker seine Messung bereits
zuruecksetzen kann. Ein sofortiges UI-Leeren waere ein zweiter Zustand mit
anderer Semantik; es kann nicht zugleich die exakte Produzentenkante sein.
