# Paket 02 — Realtime und Messkern

Stand: 2026-08-27, read-only Codeaudit mit Revalidierung nach den parallel vom
Parent integrierten Korrekturen. Dieser Agent hat ausschließlich dieses
Paketartefakt geändert. Produktcode, Tests, Manifeste und Beweise blieben durch
diesen Agenten unangetastet.

## Endurteil nach Revalidierung

Im ersten sichtbaren Source-Stand waren **neun Kandidaten bestätigt**. Während
des Audits wurden alle neun im gemeinsamen Arbeitsbaum korrigiert. Gegen den
zuletzt sichtbaren Stand gilt:

- **9/9 source-geschlossen**;
- **0/9 weiterhin source-bestätigt offen**;
- **Runtime-Verifikation der Integrationsänderungen noch ausstehend**, weil der
  einzige erlaubte Schreibpfad dieses Agents das Paketartefakt ist und deshalb
  kein neuer Build erzeugt wurde;
- ein während der Revalidierung neu gefundener Starvationspfad im ersten
  Mutex-Fix wurde ebenfalls noch in derselben Parent-Integration source-seitig
  geschlossen (INT-01 unten).

„Source-geschlossen“ ist bewusst nicht gleich „bewiesen bestanden“: Die neuen
Tests liegen als fremde parallele Änderungen im Arbeitsbaum, wurden von diesem
Agenten aber nicht gebaut oder ausgeführt. Die vorhandenen Release-Binaries
stammen vom Vor-Integrationsstand.

| ID | Ursprungsschwere | Ersturteil | aktueller Source-Status | Restbeweis |
|---|---|---|---|---|
| RTM-01 | hoch | bestätigt | **geschlossen** | M1-Ton/Stille über Drop, Seek, Straddle und Same-rate-Prepare deterministisch fahren |
| RTM-02 | hoch | bestätigt | **geschlossen** | Reset gegen nichtleere Queue und Callback-Interleaving mit Acknowledge fahren |
| RTM-03 | hoch | bestätigt | **geschlossen** | beide Rate/Generation-Interleavings mit Barrieren und Bandkanten-Golden fahren |
| RTM-04 | hoch | bestätigt | **geschlossen** | neue Frame-Extent-Tests bauen/fahren; mehrere Blockgrößen ergänzen |
| RTM-05 | mittel-hoch | bestätigt | **geschlossen** | neuen Eventzeit-Test bauen/fahren; Blockgrößen/Raten und FL-Teilstücke ergänzen |
| RTM-06 | mittel | bestätigt | **geschlossen** | TSAN beziehungsweise konkurrierenden Publish/Read-Hammer fahren |
| RTM-07 | mittel | bestätigt | **geschlossen** | Silent-Tail/FP-Modus auf x86 und ARM messen |
| RTM-08 | niedrig | bestätigt | **geschlossen** | neuen Nonfinite-LUFS-S-Gegenpfad bauen/fahren |
| RTM-09 | niedrig | bestätigt | **geschlossen** | neue INT64-Prozessor-Gegenpfade bauen/fahren, idealerweise zusätzlich UBSan |

Nicht in die neun eingerechnet sind die bewusst akzeptierte Vier-Slot-
Publikation der Hörmarkierung, ein bereits in SONDE-008 dokumentierter
portabler Ordering-Verdacht und die absichtlich noch nicht vorhandene
Messverdrahtung der heutigen Suna-/Probeeq-Hüllen. Diese Klassifikationen stehen
weiter unten.

## Scope, Norm und harte Invarianten

Gemeinsam über Producer/Consumer sowie Prepare/Start/Stop verfolgt wurden:

- `eq-copilot/plugin/hostbridge/NakamaHostBridge.h` und die produktive
  EqCopilot-VST3-Verdrahtung in `eq-copilot/plugin/CMakeLists.txt:49-71`;
- `eq-copilot/plugin/src/PluginProcessor.{h,cpp}`, Worker, Queue-Abgriff,
  Hostbefund, Projektfenster und Hörmarkierung;
- `eq-copilot/plugin/core/StampedAudioQueue.h`, einschließlich vollständiger
  Veröffentlichung, Wrap, Drop, Neustart und Ein-Block-Quarantäne;
- `eq-copilot/plugin/src/AnalyseEngine.{h,cpp}` sowie
  `eq-copilot/plugin/core/analysis/{FeatureEngine,LoudnessAccumulator,
  KGewichtung,BandGrid,FFT}.h`;
- `QueueStressTestMain`, `LoudnessGoldenTestMain`, `AnalysisGoldenTestMain`,
  `HostContextTestMain`, Null- und Markierungstests sowie die Nulltests der
  beiden Sondenhüllen;
- die aktuellen Aussagen und offenen Grenzen in `docs/beweise/SONDE-008.md`,
  `docs/beweise/SONDE-009.md` und `docs/plugin-wissen.md`.

Normative Riegel waren Entwurf §32.3
(`docs/FL-Nakama-Sonden-Design-Entwurf.md:1652-1732`), §48.1–48.2 und §49.1–49.2
(`:3098-3146`), Gates 1/2/5, §53.7 (`:3629-3679`), §56
(`:3844-3863`) und §65 SONDE-008/009 (`:4079-4080`). Zusätzlich galt der
JUCE-Echtzeitvertrag aus `CLAUDE.md`: keine Heap-Allokation, Sperre, Datei-/
Pipe-I/O oder Logs im Audiocallback; neutraler Pfad sampleidentisch; keine
Telemetry→Gain-Kopplung.

Die daraus abgeleiteten Invarianten:

1. Der Hauptpuffer bleibt bittransparent, außer eine ausdrücklich erlaubte
   Hörmarkierung ist aktiv; Analyse liest nur und steuert niemals Gain.
2. Die SPSC-Queue publiziert ausschließlich vollständige Deskriptor-/Audio-
   Paare, verwirft bei Überlast einen ganzen Block und blockiert nie.
3. Drop, Seek, Loop, Stop/Start, Hostreset, Neustart und Sampleratewechsel
   schneiden **jeden** offenen FFT-, Loudness-, K-, Korrelations- und
   Ereigniszustand.
4. Startfolge, Samplerate und die daraus gebaute Band-/Filterkonfiguration
   gehören zu derselben Generation.
5. `project_sample_start` und `sample_count` beschreiben das Intervall des
   veröffentlichten Analyseframes; Eventzeit bezeichnet den versprochenen
   FFT-Fensteranfang und darf nicht von der Hostblockpartition abhängen.
6. Nichtendliche Werte und Denormals erzeugen weder gültige Telemetrie noch
   dauerhaft teure rekursive Pfade; alle Speicherströme bleiben gedeckelt.

## Die neun Befunde und ihr aktueller Abschluss

### RTM-01 — Legacy-M1 überbrückte Queue-/Transportgrenzen

**Erstbefund:** bestätigt, hoch. Confidence: sehr hoch.

Vor der Integration erhielt `AnalyseEngine` im Worker nur Audio und Länge,
während ausschließlich `FeatureEngine` den Deskriptor und damit Drop, Seek,
Loop, Startfolge und Straddle sah. Offene Legacy-FFT-, K- und Loudnesszustände
konnten so Vor- und Nachgrenzen-Audio verbinden. Der Same-rate-No-op in
`AnalyseEngine::vorbereiten()` verstärkte den Pfad.

**Aktueller Source-Abschluss:** Der Worker merkt seine Queue-Generation und
setzt M1 bei jedem neuen Anlauf zurück
(`eq-copilot/plugin/src/PluginProcessor.cpp:495-520`). Er vergleicht vor und
nach `Blockquarantaene::schiebe()` den Bruchzähler und schneidet M1 bei einer
lokalen Queuegrenze (`:551-560`). Für semantische Grenzen wird zuerst
`FeatureEngine::nimmBlock()` ausgeführt; ein gestiegener Grenzzähler setzt M1
vor dessen Verarbeitung zurück (`:564-579`). Der Straddle-Pfad ist in derselben
Reihenfolge korrekt: `straddleVerworfen()` steigt, M1 wird zurückgesetzt und
der Block wegen `! blockVerworfen` gerade **nicht** verarbeitet (`:565-579`).
Damit sehen beide Engines dieselbe Grenze, obwohl nur eine den Deskriptor
besitzt. Status: **source-geschlossen**.

**Restbeweis:** Es fehlt weiterhin ein deterministischer echter
Prozessor/Worker-Test mit Vorgrenzen-Ton und Nachgrenzen-Stille für Queue-Drop,
Seek, Loop/Straddle und Same-rate-Prepare. Er muss nicht nur Füllstände, sondern
auch Legacy-Bandenergie, K-Tail und Loudness prüfen.

### RTM-02 — Messreset hatte keinen Schnitt im Queue-Strom

**Erstbefund:** bestätigt, hoch. Confidence: sehr hoch.

Der UI-Wunsch leerte früher nur Engines und Quarantäne. Bereits wartende
Vor-Reset-Deskriptoren konnten danach die frische Messung wieder füllen; traf
der Wunsch während des Drains ein, war der Schnitt in die andere Richtung
verschoben.

**Aktueller Source-Abschluss:** `fordereMessResetAn()` nimmt die neue
Analyse-Steuersperre, erhöht die Queue-Generation und setzt erst dann den
Resetwunsch (`eq-copilot/plugin/src/PluginProcessor.h:173-183`). Der Worker
nimmt dieselbe Sperre, verarbeitet den Generationswechsel vor der Queue und
setzt danach beide Engines und die Quarantäne zurück
(`PluginProcessor.cpp:500-529`). Alte Deskriptoren können die aktuelle
Startfolge nicht mehr bestehen (`:537-548`). Der Reset besitzt damit einen
geordneten Ganzblock-Cut; ein mitten im Callback gesehener Wunsch ordnet den
vollständigen Block auf genau eine Seite ein. Status: **source-geschlossen**.

**Restbeweis:** Barrieren um UI-Reset, Queuepublikation und Workerzug fehlen.
Zusätzlich ist die Consumersemantik zu dokumentieren: `merkmalFrames` bleibt
ein monotoner Lifetime-Zähler und ist kein Reset-Acknowledge. Ein späterer
Telemetryclient darf daraus nicht schließen, dass unmittelbar nach dem Wunsch
schon ein neuer Frame vorliegt.

### RTM-03 — Samplerate und Startfolge waren nicht atomar gepaart

**Erstbefund:** bestätigt, hoch. Confidence: hoch.

`prepareToPlay()` publizierte die Rate früher vor dem Generationswechsel; der
Worker lud die Rate einmal vor einem ungebremsten Drain. Damit waren sowohl
„alte Blöcke/neue Binzuordnung“ als auch „neue Blöcke/alte Binzuordnung“
möglich.

**Aktueller Source-Abschluss:** `prepareToPlay()` koppelt Rate und
`queue.neustartAnfordern()` unter `analyseSteuerMutex`
(`eq-copilot/plugin/src/PluginProcessor.cpp:85-101`). Der Worker nimmt dieselbe
Sperre, richtet beide Engines zuerst auf die geladene Rate aus und prüft danach
Generation/Deskriptoren (`:500-520,537-579`). Ein Prepare zwischen zwei
Workerzügen macht alle bereits wartenden alten Blöcke stale; während eines
Workerzugs kann die Rate nicht wechseln. Der Zug ist inzwischen auf acht
Deskriptoren begrenzt (`:531-585`), sodass ein voller Producer die
Steueroperation nicht mehr hinter einem einzigen unendlichen Drain festhält.
Status: **source-geschlossen**.

**Restbeweis:** Beide ursprünglichen Interleavings mit Hooks fahren: Rate-Store
vor Generationswechsel und Worker-Rate-Load vor neuer Generation. Nicht nur das
Framefeld, sondern ein Ton an einer 44,1/48/96-kHz-Bandkante muss die tatsächlich
verwendete Binzuordnung beweisen. Mutex-Fairness/Steuerlatenz bei dauerhaft
voller Queue bleibt eine Performance-, keine gefundene Zuordnungsfrage.

### RTM-04 — Framezeit beschrieb nur den auslösenden Hostblock

**Erstbefund:** bestätigt, hoch. Confidence: hoch.

Ein Frame integrierte mindestens 100 ms, stempelte aber
`project_sample_start` und `sample_count` des letzten Callbackblocks. Bei
48 kHz/512 Samples enthielten die Werte 5120 Samples ab Position 0, während der
Stempel `4608 + 512` meldete.

**Aktueller Source-Abschluss:** Der erste tatsächlich verarbeitete Block eines
Rahmens wird in `rahmenStartBlock` festgehalten
(`eq-copilot/plugin/core/analysis/FeatureEngine.h:1108-1112`). Reset, Grenze und
Framepublikation leeren Start und Inhaltszähler gemeinsam (`:1077-1081,
:1479-1484`). `baueFrame()` stempelt den ersten Block (`:1427-1431`) und
`sample_count` wird aus den wirklich integrierten `rahmenSamples` gebildet
(`:1500-1506`). Weil `grenzeZiehen()` den Inhaltszähler leert, kann die bewusst
weiterlaufende Kadenzuhr einen unmittelbar fälligen Frame nur mit
Nachgrenzen-Inhalt stempeln. Status: **source-geschlossen**.

**Neues Testbein, noch nicht gefahren:**
`eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp:1043-1080` prüft das ganze
Mehrblockintervall sowie einen unmittelbar nach Seek fälligen Einblockframe.

**Residual:** 64/127/512 Samples und Oversize-nahen Maximalblock fahren. Die
Stützintervalle einzelner Langzeitwerte wie LUFS-S sind weiterhin gesondert
vom 100-ms-Publikationsintervall zu dokumentieren.

### RTM-05 — Eventzeit war der Hostblock- statt FFT-Fensteranfang

**Erstbefund:** bestätigt, mittel-hoch. Confidence: sehr hoch.

Der Eventvertrag nennt `stromSample` den Fensteranfang; gespeichert wurde aber
`block.stromVon`. Beim 4096er Fenster und 512er Hostblock lag jedes Event damit
3584 Samples zu spät und hing von der Blockpartition ab.

**Aktueller Source-Abschluss:** Jede FFT-Stufe führt jetzt lokalen und, falls
ableitbar, Projekt-Fensteranfang (`FeatureEngine.h:746-790`). Beim ersten Sample
eines Fensters wird der exakte Blockoffset addiert; nach jedem 50-%-Hop werden
beide Starts um den Hop fortgeschrieben, mit INT64-Überlaufwache
(`:1170-1203`). Das Event übernimmt diese Stufenposition statt des Hostblocks
(`:1337-1349`). Status: **source-geschlossen**.

**Neues Testbein, noch nicht gefahren:**
`AnalysisGoldenTestMain.cpp:1758-1776` verlangt 2048er Fensterstart-Raster und
identische lokale/Projektposition beim vorhandenen 512er Lauf.

**Residual:** Derselbe annotierte Impulsstrom muss noch mit 64, 127, 512 und
>Hop Samples sowie mehreren Raten laufen. Für FL-Teilstücke, die wiederholt
dieselbe Host-Projektzeit melden, bleibt die echte Wrapperabbildung ungeprüft;
lokale monotone Eventzeit ist nun korrekt, starke Projektzeitaussage braucht
weiter den FL-Beweis.

### RTM-06 — Feature-Getter und Frame enthielten Datenrennen

**Erstbefund:** bestätigt, mittel. Confidence: sehr hoch.

Öffentliche Getter lasen nichtatomare Workerzähler direkt, und
`merkmalFrame()` reichte eine Referenz auf den mutierten Workerbesitz heraus.
Der vorhandene Integrationstest tat dies bereits bei laufendem Worker.

**Aktueller Source-Abschluss:** Sämtliche nichtatomaren Feature-Getter nehmen
`analyseSteuerMutex`; `merkmalFrame()` gibt unter derselben Sperre eine Kopie
zurück (`eq-copilot/plugin/src/PluginProcessor.h:143-161`). Der Worker hält die
Sperre bei allen Feature-Schreibzügen (`PluginProcessor.cpp:498-600`). Der
öffentliche Direktzugriff `merkmalEngine()` ist entfernt. Der Test hält jetzt
die zurückgegebene Kopie statt einer Referenz
(`AnalysisGoldenTestMain.cpp:1903-1904,1950-1951`). Die Sperre liegt niemals im
Audiocallback. Status: **source-geschlossen**.

**Restbeweis:** TSAN oder ein Publish/Read-Hammer muss zeigen, dass Leser nie
Felder zweier Generationen mischen. Stop/Destroy, Resetpublikation und die
maximale Wartezeit eines Getters unter voller Queue sind einzubeziehen.

### RTM-07 — K-Gewichtung lief im Worker ohne Denormal-Schutz

**Erstbefund:** bestätigt, mittel. Confidence: hoch.

Die exakte 48-kHz-K-Rekurrenz erreichte in einem read-only numerischen
Impuls/Silent-Tail nach 142666 Samples (ca. 2,97 s) Subnormalwerte und blieb im
Bereich um `7.46e-322`. `ScopedNoDenormals` existierte nur im Audiocallback und
schützte den separaten Workerthread nicht.

**Aktueller Source-Abschluss:** `workerLauf()` setzt nun einen eigenen, über die
gesamte Workerlebenszeit geltenden `juce::ScopedNoDenormals`
(`eq-copilot/plugin/src/PluginProcessor.cpp:483-487`). Status:
**source-geschlossen**.

**Restbeweis:** Impuls plus langer Silent-Tail bei kontrolliertem FP-Modus auf
x86/ARM und Debug/Release fahren; neben Endlichkeit auch Laufzeit und
Queue-Drops beobachten. Ein Test sollte bestätigen, dass der Worker-FP-Modus
den aufrufenden Thread nicht unbeabsichtigt beeinflusst.

### RTM-08 — `LoudnessAccumulator::kurz()` meldete NaN/Inf als gültig

**Erstbefund:** bestätigt, niedrig. Confidence: sehr hoch.

`zelle()` schrieb NaN/Inf vor der Endlichkeitsprüfung in den 30-Zellen-Ring;
`kurz()` summierte ihn und lieferte bedingungslos `true`. Der bestehende
Nonfinite-Test fragte nur die integrierte Lautheit ab.

**Aktueller Source-Abschluss:** Der Ring führt pro Zelle ein Gültigkeitsbit,
speichert für nichtendliche Eingaben keinen nichtendlichen Rechenwert und lässt
weder 400-ms-Block noch 3-s-Kurzzeitwert über eine ungültige Zelle entstehen
(`eq-copilot/plugin/core/analysis/LoudnessAccumulator.h:194-229,354-367`).
`kurz()` prüft zusätzlich das Endergebnis. Die feste Speicherbilanz wurde um
die Boolmaske ergänzt (`:387-390,452-455`). Status: **source-geschlossen**.

**Neues Testbein, noch nicht gefahren:**
`eq-copilot/plugin/tests/LoudnessGoldenTestMain.cpp:471-477` füllt 30 gültige
Zellen, injiziert NaN und verlangt `kurz()==false`.

**Residual:** Festgelegt ist damit „ungültig, bis die Zelle aus dem 3-s-Ring
gefallen ist“. Ein späterer Wire-Consumer braucht dafür weiterhin ein
Validitätsbit; stilles Ersetzen als gültige Stille wäre falsch.

### RTM-09 — Signed overflow in der Projektfensterrechnung

**Erstbefund:** bestätigt, niedrig. Confidence: sehr hoch; operative
Wahrscheinlichkeit niedrig.

Im Audiocallback wurden `t+n`, `t-fensterErwartet` und `llabs` ohne
Bereichsprüfung verwendet. Nahe `INT64_MIN/MAX` konnte die sichtbare
Fensterbuchhaltung UB auslösen.

**Aktueller Source-Abschluss:** `projektEnde()` prüft die Addition vorab;
`projektAbstandGroesserAls64()` bildet signed `int64` monoton auf `uint64` ab
und berechnet den Abstand ohne signed Subtraktion
(`eq-copilot/plugin/src/PluginProcessor.cpp:12-31`). Ein nicht darstellbares
Blockende deaktiviert das Fenster und zählt eine Grenze; gültige Intervalle
verwenden nur das geprüfte Ende (`:256-293`). Status: **source-geschlossen**.

**Neues Testbein, noch nicht gefahren:**
`AnalysisGoldenTestMain.cpp:1969-2000` treibt den echten Prozessorweg über ein
Blockende jenseits `INT64_MAX` sowie einen Abstand von `INT64_MIN` zu
`INT64_MAX`.

**Residual:** Den neuen Test unter UBSan fahren und zusätzlich `n=0`, maximalen
zulässigen Hostblock sowie exakt `INT64_MAX-n` prüfen. Der Hoststart bleibt bei
einem nicht darstellbaren Ende als empfangener Einzelwert sichtbar; nur das
Messfenster wird zu Recht ungültig.

## Integrationsrevalidierung: neu gefundener Pfad

### INT-01 — Steuer-Mutex lag zunächst über einem ungebremsten Drain

**Zwischenbefund:** bestätigt, mittel; im aktuellen Source-Stand geschlossen.
Confidence: hoch.

Die erste Parallelkorrektur von RTM-01/02/03 hielt `analyseSteuerMutex` über
`while (queue.spitze())`. Bei einem dauerhaft nachliefernden Producer konnte
der Worker den kritischen Abschnitt ohne obere Schranke halten; UI-Reset,
Feature-Snapshot und `prepareToPlay()` konnten verhungern. Das war kein Lock im
Audiocallback, aber ein Lebenszyklus-/Bedienstillstand und hätte gerade unter
dem Denormal-Rückstau aus RTM-07 konkret werden können.

Der aktuelle Worker verarbeitet höchstens acht Deskriptoren je Steuerzug,
merkt vorhandenen Rest, gibt die Sperre frei und setzt nach `yield()` fort
(`PluginProcessor.cpp:531-608`). Damit ist die Einzelhaltezeit durch acht
Blöcke plus eine leichte/schwere Auswertung begrenzt. Status:
**source-geschlossen**. Residual: Ein Stressbein mit dauerhaft voller Queue und
parallel wiederholtem Snapshot/Reset/Prepare sollte maximale Steuerlatenz
messen; `std::mutex` garantiert keine formale Fairness.

## Intentional, Duplikat oder stale — nicht als offener Kandidat gezählt

### Suna/Probeeq ohne Hostbridge und Messkern — intentionaler Zwischenstand

Nur EqCopilot kompiliert den gepatchten Wrapper mit `NAKAMA_HOST_BRIDGE=1` und
bindet das Hostbridge-Include ein (`eq-copilot/plugin/CMakeLists.txt:49-71`).
Die Sondenziele enthalten ausschließlich `SondeProcessor`/Factory und kein
Hostbridge-Define (`:181-223`); ihr `processBlock()` ist absichtlich leerer
Passthrough (`eq-copilot/plugin/sonde/SondeProcessor.cpp:28-64`).
`docs/plugin-wissen.md:16-27` bezeichnet beide ausdrücklich als Hüllen vor dem
Produktumschnitt. Klassifikation: **intentional/Staging**. Die grünen
Hüllen-Nulltests sind kein Nachweis für spätere Probeeq-Hostzeit-, Queue- oder
Ratepfade.

### Vier-Slot-Publikation der Hörmarkierung — intentional akzeptiertes Risiko

Der Message-Thread schreibt ein nichtatomares POD in einen von vier Slots und
publiziert danach den Zähler; der Audiothread kopiert den Slot
(`eq-copilot/plugin/src/HoerMarkierung.h:259-295`). Nach vier Publikationen kann
der Writer denselben Slot während der Audiokopie überschreiben. Der
Kopfkommentar nennt dies „menschlich unerreichbar“ (`:10-18`). Das ist kein
formaler C++-Race-Freibrief, aber ein sichtbar bewusst akzeptierter Pfad und
keine neu entdeckte normale neutrale Audioveränderung: ohne Erlaubnis kehrt der
DSP mit unangetastetem Puffer zurück (`:306-312`). Klassifikation:
**intentional akzeptiertes Restrisiko**. Vor automatisiertem/hochfrequentem
Publisher durch wirklich konfliktfreie wait-free-Publikation ersetzen und mit
konkurrierendem Hammer testen.

### Relaxed Neustartpublikation — Duplikat/portables Restrisiko

`StampedAudioQueue::neustartAnfordern()` erhöht `aktuelleStartFolge` und setzt
danach `neustartWunsch`, beide relaxed; der Producer liest beide ebenfalls
relaxed (`eq-copilot/plugin/core/StampedAudioQueue.h:332-368`). Portabel ist
nicht garantiert, dass ein Producer, der den Wunsch sieht, schon die neue Zahl
sieht. Der identische Punkt steht bereits in
`docs/beweise/SONDE-008.md:1554-1599`. Die aktuelle MSVC-x64-Ausgabe verwendet
für die RMWs volle Maschinenbarrieren; im vorhandenen Ziel wurde kein
Fehlstempel reproduziert. Klassifikation: **Duplikat/portables Restrisiko**.
Bei Portierung oder Änderung der Atomik als ein Release/Acquire-Protokoll
schließen.

### Same-rate-No-op der Legacy-Engine — stale Absicht, jetzt gegengekoppelt

`AnalyseEngine::vorbereiten()` bewahrt bei gleicher Rate die Messung, weil FL
mehrfach prepare aufrufen kann (`AnalyseEngine.cpp:169-173`). Lokal bleibt das
stale zur heutigen Epochenregel; der Worker kompensiert es nun explizit über
`workerAnlauf` und `engine.zuruecksetzen()` (`PluginProcessor.cpp:495-520`).
Klassifikation: **stale lokale Absicht, systemisch geschlossen**.

## Widerlegte Hypothesen und belastbare positive Belege

- **Neutraler Hauptpuffer:** Der Queue-Abgriff liest vor der Hörmarkierung und
  schreibt nie ins Audio (`PluginProcessor.cpp:297-344`). Die Markierung ist
  die einzige Schreibstelle und fail-closed an Klassifikation,
  Echtzeitbeweis, gültiges `playing`, Realtime-Modus und offenen Editor. Null-
  und Markierungstests blieben im vorhandenen Build grün. Hypothese
  „unbeabsichtigte normale Audioveränderung im neutralen Zustand“:
  **widerlegt für die gefahrenen Pfade**, mit dem separat benannten
  Vier-Slot-Race als bewusstem Restrisiko.
- **Allocation/Lock/I/O/Logging im Audiocallback:** Queue- und Marker-Puffer
  werden vorallokiert; `processBlock`, Host-Senke, Queuepublikation und
  Markerpfad enthalten keine Sperre, Datei-/Pipe-I/O oder Logs. Der neue
  `analyseSteuerMutex` liegt ausschließlich in Prepare/UI/Worker. Der
  vorhandene Queue-Stresstest meldete über 4000 Audioblöcke null
  Audio-Thread-Allokationen. Hypothese: **widerlegt**.
- **Partieller/staler SPSC-Deskriptor durch den Kernring:** Audio wird vor dem
  Release-Publish vollständig kopiert, alle wiederverwendeten Felder werden
  überschrieben; Consumer liest Acquire und gibt Samplebereich vor Deskriptor
  frei (`StampedAudioQueue.h:412-505`). Voll-/Oversizefälle droppen ganz.
  Wrap-, Overflow- und Quarantänetests blieben grün. Hypothese:
  **widerlegt**, abgesehen vom separat als Duplikat geführten Restart-Ordering.
- **Unbegrenztes Speicherwachstum:** Queue, FFT-/Bandzustände, Loudnessring und
  Ereignisring sind fest gedeckelt; der Ereignistest fährt den 64er-Ring real
  in `drop-oldest` (`AnalysisGoldenTestMain.cpp:1702-1776`). Hypothese:
  **widerlegt**. Ein einzelner `uint32`-Loudness-Binzähler kann theoretisch nach
  rund 13,6 Jahren ununterbrochener 10-Hz-Belegung desselben Bins umlaufen; das
  bleibt ein extrem fernes Langzeitlimit, kein priorisierter Kandidat.
- **FeatureEngine überbrückt nach der Integration Grenzen:**
  `grenzeZiehen()` leert FFT, Bandakkus, K-Zustände, LUFS-S, Korrelation,
  Frameextent und Flusshistorie (`FeatureEngine.h:1000-1099`). Die
  Legacy-Gegenkopplung steht jetzt vor deren Verarbeitung. Hypothese:
  **widerlegt am aktuellen Sourcepfad**, Runtime-Gegenbein noch ausstehend.
- **FFT/BandGrid-OOB im festen Produktpfad:** Verwendet werden nur vorbereitete
  4096-/16384-Punkte-Größen und feste 64-/221-Bandgitter; analytische FFT-,
  Parseval- und Bandkanten-Goldens waren grün. Eine fehlende generische
  Power-of-two-Prüfung der internen Hilfe hat keinen variablen Produktcaller.
- **Telemetry steuert Audio:** Analyseabgriff liegt vor der Markierung; keine
  Feature-/Loudnessausgabe fließt in Erlaubnis oder Gain. Gate 5 bleibt erfüllt.

## Prüfungen und Beweisstatus

Vor den Parent-Integrationsänderungen wurden aus der vorhandenen
Release-Konfiguration in dieser Session ausgeführt:

| Test | Vor-Integrations-Ergebnis |
|---|---|
| `EqCopQueueStressTest` | 81 Prüfungen, 0 Fehler |
| `EqCopLoudnessGoldenTest` | 108 Prüfungen, 0 Fehler |
| `EqCopAnalysisGoldenTest` | 212 Prüfungen, 0 Fehler |
| `EqCopHostContextTest` | 91 Prüfungen, 0 Fehler |
| `EqCopNullTest` | OK |
| `EqCopMarkierungTest` | OK |
| `EqCopProbeeqNullTest` | 62 Prüfungen, 0 Fehler |
| `EqCopSunaNullTest` | 62 Prüfungen, 0 Fehler |

Diese Läufe belegen Queue-/Null-/Hostgrundlagen des vorhandenen Builds, nicht
die danach geänderten Quellen. Neu im sichtbaren Testcode sind Frameextent,
Post-Seek-Extent, Event-Fensterstart, Nonfinite-LUFS-S und beide
INT64-Prozessorränder; für RTM-01 bis RTM-03 sowie INT-01 fehlen weiterhin
deterministische Integrationsbeine.

## Verbleibende externe Beweisgrenzen und empfohlene Reihenfolge

1. **Neu bauen und die acht gezielten Binaries wiederholen.** Erst danach darf
   aus „source-geschlossen“ ein laufender Beleg werden.
2. **RTM-01 bis RTM-03 plus INT-01 mit Barrieren ergänzen.** Die richtige
   Workerreihenfolge und Mutexkopplung sind sichtbar, aber bisher nicht
   scheduler-deterministisch getestet.
3. **Event-/Framepartition verbreitern.** 64/127/512/>Hop und 44,1/48/96 kHz;
   dabei exakte lokale und Projektpositionen vergleichen.
4. **Sanitizer/FP-Stress:** TSAN für Getter/Worker, UBSan für Projektzeitränder,
   Impuls/Silent-Tail für Denormals.

Nicht gefahren wurden FL Studio, der gepatchte Wrapper als echter
VST3-Callback, ThreadSanitizer, UBSan, ein annotierter Impulskorpus und ein
Mehrfach-Tap-Produktpfad. Diese Grenzen stehen bereits in
`docs/beweise/SONDE-009.md:6930-6970`. Der direkte
`nakamaBlockEmpfangen()`-Test beweist Senke und Abbildung, nicht den echten
Hostaufruf des Wrappers.
