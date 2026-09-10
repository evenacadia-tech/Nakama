# Audit-Paket: Audiopfad, Analyse und DSP

Stand: 10.09.2026, eingefrorener Commit `844b9c15935377e89d124ac33848e9cba1ecf4c2` unter `C:\na-audit-20260910-844b9c1`. Profil: statischer Deep-Audit. Produktcode, Tests und Git unverändert. Der aktive Fable-Checkout wurde nicht benutzt. Dieses Paket ist das einzige vom Audio-Reviewer geschriebene Artefakt.

**Ergebnis:** Ein statisch belegtes Datenrennen im heutigen Hörmarker, ein vom Root-Reviewer zusätzlich reproduzierter Zeitfehler im heutigen Vergleichspegel und ein statisch belegter Fehler im beworbenen Vier-Bank-Lastnachweis. Zwei weitere konkrete Performance-Themen brauchen einen eigenen Messlauf. Kein Gesamt-PASS und keine Aussage zur Releasefähigkeit.

## Belegstufen und Grenzen

- Der read-only Qualitäts-Preflight wurde im Snapshot ausgeführt: richtige Revision, anfangs sauberer Worktree, benötigte Werkzeuge vorhanden, keine implizite Änderungsauswahl.
- In diesem Paket wurden keine Produktbinaries gebaut oder ausgeführt, keine Hosts, Pipes oder Lastläufe gestartet. Bestehende Tests wurden an der Quelle gelesen, nicht als heute bestanden ausgegeben.
- Der Root-Reviewer hat unabhängig einen kleinen MSVC-Harness direkt gegen das unveränderte produktive `Vergleichspegel.h` gebaut und ausgeführt. Sein Ergebnis und Rohartefakt sind bei A-DSP-02 angegeben. Das ist kein Host- oder vollständiger Prozessorbeweis.
- Die DSP-Bibliothek aus SONDE-015 wurde als vorhandener, noch nicht in die Prozessoren integrierter Etappenstand gelesen. Das Fehlen der geplanten Verdrahtung ist ausdrücklich kein Defekt dieses Audits.

## A-DSP-01 — P1: Hörmarker überschreibt Slots ohne Leserbesitz

**Status:** statisch belegter Defekt; konkurrierender Lauf/TSan **NOT RUN**.

Die Übergabe von UI-Aufträgen an Audio benutzt vier nichtatomare `MarkierungsAuftrag`-Objekte. [HoerMarkierung.h:314](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/HoerMarkierung.h#L314) erhöht lediglich den Publikationszähler, schreibt `ring[nr % 4] = a` und publiziert den Zähler mit Release. Der Audiozug liest den Zähler mit Acquire und greift danach auf den Slot zu, erst für `modus`, dann für die gesamte Kopie ([HoerMarkierung.h:377](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/HoerMarkierung.h#L377)). Eine Rückmeldung, dass der Leser diesen Slot besitzt oder verlassen hat, gibt es nicht.

**Vollständige Auslösekette:**

1. Audio liest Generation `g` in Zeile 377 und wird vor oder während des Slotzugriffs unterbrochen.
2. UI führt vier normale Publikationen aus. Der vierte Auftrag verwendet wieder Slot `g % 4`.
3. Audio und UI lesen/schreiben dasselbe normale Objekt ohne synchronisierten Besitzwechsel. Bei Fortsetzung kann Audio einen späteren beziehungsweise gemischten Auftrag sehen. Schon das fehlende Happens-before zwischen Wiederverwendung und Lesen ist ein C++-Datenrennen.

Der Produktionsaufrufer ist [PluginEditor.cpp:754](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginEditor.cpp#L754), Einreichen in Zeile 800, sowie Ausschalten in Zeile 821. Beide führen über [PluginProcessor.h:891](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginProcessor.h#L891) unmittelbar zur Markerübergabe. Eine Geschwindigkeitsbegrenzung mit garantierter Reader-Laufzeit existiert dort nicht.

**Schaden:** undefiniertes Verhalten im Audiopfad; unter anderem inkonsistente Filterkoeffizienten oder Filterkonfiguration. Dass unterschiedliche gültige Aufträge einzeln stabil sind, garantiert keine Stabilität eines gerissenen Auftrags. Der Callback kann dadurch hörbar falsch arbeiten; die Nulltest-Zusage ersetzt keinen Besitznachweis beim eingeschalteten Marker.

**Gegenargument geprüft:** Der Kopfkommentar in `HoerMarkierung.h:16-18` nennt vier Publikationen während einer Blockkopie „menschlich unerreichbar“. Der Scheduler garantiert jedoch keine maximale Unterbrechungszeit des Audiothreads. Release/Acquire veröffentlicht die vorhergehenden Schreibzugriffe, verhindert aber keinen späteren Umlauf des Schreibers. Auch ein nachträglicher Zählervergleich würde bereits konkurrierende nichtatomare Lesezugriffe nicht legalisieren.

**Testlücke:** Die gelesenen Markerprüfungen reichen Aufträge synchron vor dem folgenden Audiolauf ein. `MarkierungTestMain.cpp` benutzt keine `std::thread`-Publikationsprobe; die dortigen Thread-Aufrufe sind Wartezeiten. Die vorhandene Queue-Stressprüfung betrifft den getrennten, korrekt besessenen Audio-Analysering, nicht diesen Viererspeicher.

**Kleinster nächster Beweis:** Produktiven Marker unter ThreadSanitizer mit einem UI-Produzenten und Audio-Konsumenten betreiben; alternativ ein gezielter Testhaken genau nach dem Zähler-Acquire mit vier anschließenden Publikationen. Eine Lösung braucht tatsächlichen Slotbesitz oder eine vorallokierte SPSC-/Triple-Buffer-Übergabe mit klarer Vollpolitik; kein Warten, Reclaim oder Heap im Callback.

## A-DSP-02 — P2: 400 ms Vergleichsmaterial werden bei Stereo schon nach 200 ms erreicht

**Status:** **FAIL**, durch Root-Harness am produktiven Header reproduziert; Prozessor-/Hostlauf **NOT RUN**.

[PluginProcessor.cpp:777](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginProcessor.cpp#L777) ruft `vergleichspegel.speise(...)` für jeden der beiden Kanäle separat auf. Der Pegel zählt in [Vergleichspegel.h:119](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/analysis/Vergleichspegel.h#L119) jedes Kanalsample in `gesehen`. Die Mindestmenge entsteht jedoch aus `0.4 * fs` ohne Kanalzahl ([Vergleichspegel.h:66](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/analysis/Vergleichspegel.h#L66)); `bereitIntern()` vergleicht genau diese beiden unterschiedlichen Einheiten ([Vergleichspegel.h:314](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/analysis/Vergleichspegel.h#L314)).

**Vollständige Auslösekette:** Eine Passage wird gebunden, ihr Fenster freigegeben und der Vergleichspegel beginnt mit Stereo-Audio. 9.600 Hostframes bei 48 kHz bedeuten 200 ms Musik, ergeben durch zwei `speise`-Aufrufe aber 19.200 gezählte Kanalsamples. Das erfüllt bereits `mindestSamples = 19.200`. `beginneVersuch` friert den Wert über `friereEin()` ein und kann den unzureichend langen Pegel als gültigen Referenzwert übernehmen ([Analyse.cpp:762](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Analyse.cpp#L762)).

**Schaden:** Die ausdrückliche 400-ms-Schutzschwelle wird im üblichen Stereobetrieb halbiert; der A/B-Abgleich darf bereits kürzeres, transientenlastiges Material verwenden. Mono und Stereo besitzen dadurch unterschiedliche Zeitbedingungen für dieselbe Produktfunktion.

**Gegenargument geprüft:** Für das RMS-Verhältnis dürfen Energien beider Kanäle gemeinsam summiert werden. Daraus folgt jedoch keine doppelte musikalische Dauer. Die reine Pegelformel ist hier nicht der Defekt, die Einheit ihrer Mindestlänge ist es. Auch der zufällige Worker-Vorlauf kann das Problem nur verdecken, nicht die Zählweise korrigieren.

**Frischer Beleg des Root-Reviewers:** MSVC `/O2`, unveränderter produktiver Header, 1/2 Kanäle und 199/200/399/400 ms bei 48 kHz. Mono wird korrekt erst bei 400 ms bereit; Stereo wird bereits bei 200 und 399 ms bereit und einfrierbar, obwohl die Erwartung dort `false` ist. Compiler Exit 0, Harness Exit 1 mit zwei verletzten Erwartungen. Rohbeleg: [repro-vergleichspegel.txt](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/.workflow/ultracode/20260910-audit/evidence/repro-vergleichspegel.txt). Die Produktionsaufrufkette wurde zusätzlich statisch geprüft, nicht im Harness durch einen vollständigen JUCE-Prozessor ausgeführt.

**Testlücke:** [Sonde013PassageStateTest.cpp:3086](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/Sonde013PassageStateTest.cpp#L3086) fährt für die Negativprobe nur zehn Stereo-Hostblöcke à 512 Samples. Diese Menge liegt auch unter der falschen 200-ms-Grenze. Der Helfer `fahreBisPegel` in Zeilen 294–308 bezeichnet pro Kanal gezählte Aufnahmeblöcke als Hostblöcke. Die direkten Pegeltests in `Sonde013ExperimentUiTest.cpp:65-84` speisen nur einen Kanalzug je Zeitblock und können die Integrationslücke deshalb nicht aufdecken.

**Kleinster Gegenbeweis nach einem Fix:** Im echten Prozessor 399 und 400 ms derselben markierten Passage jeweils mono und stereo speisen, Startfreigabe exakt an der Framegrenze prüfen. Zeitframes und Kanalbeiträge müssen getrennte Größen werden; zudem sollte die Aufnahme eines Stereo-Hostblocks ein gemeinsam konsistenter Pegelzug sein.

## A-PERF-01 — P2-Pflegekandidat: Probeeq rechnet ungenutzte Analysepfade

**Status:** ungenutzte Rechenarbeit statisch nachgewiesen; tatsächliche CPU-/Deadline-Auswirkung **NOT RUN**. Kein gemessener Budgetbruch.

[SondeProcessor.cpp:367](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/sonde/SondeProcessor.cpp#L367) verarbeitet jedes freigegebene Audiostück in `FeatureEngine`. Anschließend läuft zusätzlich die vollständige ältere `AnalyseEngine::verarbeite` in Zeile 376. Aus dieser älteren Engine wird im gesamten `sonde/`-Baum ausschließlich `lautheitFuerTelemetrie()` gelesen (Zeile 383). Die übrigen Treffer sind Vorbereitung und Reset. Der Getter benötigt integrierte Lautheit und deren Unsicherheit, nicht die übrigen Analyseergebnisse.

Trotzdem berechnet die alte Engine:

- den eigenen True Peak für beide Kanäle mit insgesamt 161 FIR-Taps je Sample und Kanal, [AnalyseEngine.cpp:404](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/AnalyseEngine.cpp#L404);
- vier Welch-Spektralpfade mit 16.384/8.192/4.096/2.048 Punkten, [AnalyseEngine.cpp:210](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/AnalyseEngine.cpp#L210), Verarbeitung ab Zeile 425;
- zusätzliche LTAS-, Histogramm-, Zonen- und globale Stereoakkus.

Die FeatureEngine berechnet daneben ihren eigenen 193-Tap-True-Peak- und Spektralpfad (`featureengine/Spektrum.h:147-192`, `TruePeak.h:74-78`).

Die zählbare Arbeit des **zusätzlichen alten** True-Peak-Pfads beträgt `161 * 2 = 322` Multiplikationen mit Akkumulation pro Stereo-Frame: bei 48 kHz **15.456.000 pro Sekunde je Probeeq**, für zehn Probeeq **154.560.000 pro Sekunde**. Das sind Quellcode-Operationszahlen, keine CPU-Prozentwerte, Maschineninstruktionszahlen oder Laufzeitmessungen. FFT-, Speicher- und weitere Statistikarbeit kommen hinzu.

**Warum relevant:** Arbeit läuft zwar außerhalb des Audiocallbacks, beansprucht aber dieselben CPU-/Cache-/Scheduling-Ressourcen. Die Kosten multiplizieren sich gerade im angefragten Mehrinstanzfall. Die kleinen Callback-Tests können diese Workerlast nicht widerlegen.

**Empfehlung:** Als eigenes verhaltensneutrales Pflegeticket den tatsächlich benötigten K-gewichteten integrierten Loudness-Pfad isolieren oder die gemeinsame Enginekomponente nutzen. Vorher/nachher vollständige Loudness-Goldens sowie Snapshotgleichheit prüfen und ein Gen plus zehn Probeeq messen. Keine vorschnelle Zusammenlegung der verschieden definierten Spektralgitter oder True-Peak-Verträge.

## A-PERF-02 — Messlücke: Pollintervall passt nicht zum kleinsten dauerhaften Hostblock

**Status:** Kapazitätsverhältnis statisch hergeleitet; wiederholter Produktions-/Hostlauf **NOT RUN**. Als Dimensionierungsrisiko und nötiger Test geführt, nicht als gemessener Performancefehler.

`GenStrom` hat 2.048 Deskriptorslots ([StampedAudioQueue.h:240](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/StampedAudioQueue.h#L240)). Beide Prozessoren benutzen diesen Typ. Bei leerer Queue wartet Gen bis 50 ms ([Analyse.cpp:337](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Analyse.cpp#L337)), Probeeq bis 20 ms ([SondeProcessor.cpp:435](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/sonde/SondeProcessor.cpp#L435)). Der Callback weckt den Worker nicht.

Bei dauerhaft einem Sample je Hostblock reicht der Deskriptorring bei 48 kHz für **42,67 ms**, bei 192 kHz für **10,67 ms**. Damit kann schon die absichtlich eingelegte Pollpause einen Drop verursachen, auch wenn der Worker die eigentliche Rechenarbeit rechtzeitig leisten könnte. Wiederkehrende Drops trennen zusätzlich Analysefenster; das Problem ist nicht nur der Dropzähler.

**Gegenargument geprüft:** Der Ganzblockdrop schützt Audio korrekt, und längere typische Hostblöcke vergrößern die Zeitreserve deutlich. Der Auslöser verlangt eine anhaltend sehr kleinteilige Callbackfolge. Ob FL diesen Fall in konkreten Automationsszenarien anhaltend liefert, ist mit dieser statischen Prüfung nicht bewiesen. Das Risiko verdient deshalb einen getakteten Grenztest, keine Behauptung über heutige FL-Aussetzer.

**Testlücke:** `QueueStressTestMain.cpp:667-700` führt jede der 18 Blockgrößen einmal aus und prüft unverändertes Audio; der ungetaktete 4.000-Block-Lauf in Zeilen 780–794 akzeptiert Analyseverluste ausdrücklich. Keiner dieser gelesenen Abschnitte verlangt fortschreitende Messfenster bei kontinuierlichen 1-/2-Sample-Callbacks mit Echtzeittakt.

**Nächster Beweis:** Getaktete kleinste Hostblöcke bei unterstützten Raten, zunächst eine Instanz, dann ein Gen plus zehn Probeeq. Deskriptorfüllstand, Drops, neue Featureframes und Callback-Tails erfassen. Anschließend Pollstrategie und Ringkapazität am gemessenen zulässigen Fall ausrichten.

## A-PERF-03 — P2: Der Vier-Bank-Messwert misst zwei rechnende Banken

**Status:** statisch belegter Beweisfehler; Testlauf und instrumentierter Lauf **NOT RUN**. Keine Behauptung über einen tatsächlichen CPU-Budgetbruch.

[DspGoldenTestMain.cpp:4065](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/DspGoldenTestMain.cpp#L4065) kündigt den Messwert des Vier-Bank-Falls an. Vor dem Lauf prüft der Test nur `freieSlots() == 0` (Zeile 4098); die Ausgabe behauptet danach „Committed und Candidate, je im Fade“ (Zeile 4119). Der konfigurierte und zeitlich gefahrene Pfad erfüllt diesen Rechenfall jedoch nicht.

**Vollständige Auslösekette im Test:**

1. Committed und Candidate erhalten `voll` und laufen nach der jeweiligen Publikation einen Stilleblock mit 256 Samples. Das ist bereits die gesamte Einblenddauer: `kFadeSamples == 256` und `kRampeSamples == 256` ([DspProgramm.h:53](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/dsp/DspProgramm.h#L53)).
2. `voll2` unterscheidet sich ausschließlich durch `output_trim_db = -2` (Testzeilen 4092–4093). `rampenKompatibel()` prüft gleiche Topologie und lässt diese Trimänderung ausdrücklich kompatibel ([DspProgramm.cpp:139](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/dsp/DspProgramm.cpp#L139)).
3. Der Committed-Wechsel läuft deshalb als `Uebergang::rampe`, nicht als Crossfade ([DspKern.cpp:322](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/dsp/DspKern.cpp#L322)). Der folgende Stilleblock in Testzeile 4095 beendet ihn vollständig; die alte Bank ist vor Messbeginn bereits `ausgedient`.
4. Candidate erhält anschließend denselben kompatiblen Wechsel. Vor Messbeginn liegen damit zwei aktive Banken, eine bereits ausgediente Bank und eine bereite Bank im Pool. `freieSlots()` zählt nur den Zustand `frei` ([DspBankPool.cpp:99](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/dsp/DspBankPool.cpp#L99)); vier nichtfreie Slots sagen nichts über vier laufende Filterberechnungen aus.
5. Im ersten gemessenen 256er-Block läuft die Candidate-Rampe vollständig aus. Der Kern berechnet bei einer Rampe nur die aktive Bank; die Quelle dient als Koeffizienten-/Zustandsreferenz. Einen zweiten Bankdurchlauf gibt es nur bei `crossfade` ([DspKern.cpp:667](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/dsp/DspKern.cpp#L667), aktive Bank in Zeile 695). Das Übergangsende entfernt die Quelle in Zeilen 733–738.
6. Die 4.000-Blöcke-Schleife publiziert keine weiteren Programme. Sie berechnet durchgehend zwei aktive Bankdurchläufe; 3.999 von 4.000 Blöcken laufen sogar ohne Pfadrampe. Vier gleichzeitige Filterbankdurchläufe werden durch diesen Messabschnitt nicht provoziert.

**Schaden:** Der dokumentierte „Vier-Bank“-Wert kann als Worst-Case-Beleg übernommen werden, obwohl der relevante zusätzliche Rechenpfad fehlt. Auch ein erfolgreicher aktueller Testlauf würde diese Lücke nicht widerlegen. Die reine Poolkapazität und ein tatsächlicher vierfacher Filterdurchlauf sind unterschiedliche Eigenschaften.

**Weitere Grenze desselben Messwerts:** Das Zeitfenster enthält zusätzlich die Sinuserzeugung des Testtreibers (Zeilen 4104–4110). Geprüft werden nur positive Dauer und endlicher Gesamtanteil; es gibt keine Einzelblock-Verteilung oder Deadlineprüfung. Diese Einordnung ist unabhängig davon, dass ein willkürlich erfundenes universelles CPU-Limit sinnvollerweise vermieden wird.

**Kleinster Gegenbeweis nach Korrektur des Nachweises:** Zwei tatsächlich inkompatible, jeweils vollständig belegte Programme für beide Pfade vorbereiten und ihre echten Crossfades zeitlich überlappen lassen. Vor und während jeder gewerteten Messregion die aktiven/quellenden Bankidentitäten und den tatsächlichen Bankdurchlaufzähler prüfen. Nur diese Regionen getrennt vom Testsignalaufbau auswerten; genug wiederholte Übergänge für Callback-Tails sammeln. Keine Produktänderung am Bankpool ist aus diesem Befund abgeleitet.

## Bestätigte Schutzmechanismen an der Quelle

Dies sind nachvollzogene Invarianten und vorhandene Prüfwege, keine heute gemessenen PASS-Ergebnisse:

- Die produktiven Float-Callbacks von Gen und Probeeq haben in den gelesenen Pfaden keine Heapallokation, Mutexnahme, Pipe-, Datei- oder Loggingoperation. Sie lesen die Hostpuffer für die Analyse und veröffentlichen begrenzte Ganzblöcke. Gen nimmt Änderungen nur im vorhandenen erlaubnisverriegelten Markerpfad vor; Probeeq ist am Snapshot weiterhin passiv.
- `StampedAudioQueue` reserviert Sample- und Deskriptorkapazität vor der Kopie, veröffentlicht den Deskriptor erst nach den Samples mit Release und gibt Sample-/Deskriptorslots erst nach dem Consumerzug frei. Oversize und volle Queues erzeugen vollständige Analysedrops mit anschließendem Kontinuitätsbruch. Die Producer-Kopierschleife ist durch den angenommenen Maximalblock begrenzt.
- `Blockquarantaene` gibt den Vorgänger nur bei einem passenden Folgestempel frei; Anlauf, Segment, Kanäle, Tapmaske und Zeitkanten werden geprüft. `INT64_MAX` bei Projektzeitaddition hat einen ausdrücklichen Gegenweg. Queue-Neustarts verstellen nicht fremd beide Ringzeiger.
- Gen/Probeeq setzen `ScopedNoDenormals` im Audiocallback und im jeweiligen Analyseworker. Der Pegel-Audiozug benutzt genau einen CAS-Versuch und verwirft die Aufnahme bei konkurrierender Steuerung; seine Warteschleife liegt auf dem Steuerpfad.
- Workerzüge sind auf acht Blöcke begrenzt. Gens schwere Auswertung hängt an monotonen Deadlines und holt nach Pausen keine verpassten Takte nach. Der aktuelle `WorkerCadence`-Test arbeitet mit injizierter Zeit und prüft gerade diesen Gegenfall.
- Der neue `DspBankPool` besitzt im Gegensatz zum Marker tatsächliche Zustands-/Generationswörter. Veraltete Publikationen scheitern am Generation-CAS; aktive Banken werden erst über `verblassend -> ausgedient` und ACK vom Worker wieder frei. Der ACK-Überlauf sperrt die Bank und zählt ihn. Die gelesenen Tests `DspGoldenTestMain.cpp:2667-2691` stellen verdrängte und wiederverwendete Generationen ausdrücklich her.
- `DspKern` reserviert seine Vektoren vor Audio, übernimmt Programme nur am äußeren Blockrand und teilt Oversize-Audio in vorallokierte Stücke. Die Sampleverarbeitung führt keinen Heap-Reclaim aus; Retire passiert über den Bankpool. Die Tests ab `DspGoldenTestMain.cpp:2897` prüfen den realen Kern bei Programmwechseln, Oversize, Stille und wechselnden Blockgrößen mit threadlokalen New-Zählern. Solche Hooks sind hilfreich, aber kein universeller Beleg für direkte `malloc`-/Alignment-Allokationen oder externe Synchronisation.
- Der DSP-Eingangsriegel und das Heilen nichtendlicher Filterzustände werden im Test getrennt provoziert. Der Passthrough umgeht die Sanitierung ausdrücklich; die gehörige Ausnahme ist im Test als Bitgleichheit enthalten. Roh-Dry im bewusst gemischten Dry/Wet-Zweig ist daher kein eigenständiger NaN-Befund dieses Audits.

## Gelesene Abdeckung

**Tief gelesen, jeweils für die angegebenen Pfade:**

- `src/PluginProcessor.cpp`: Float-Callback 463–856, Hoststempel 867–921 und Lebenszeichen 930–991; Konstruktor-/Destruktorbezüge gezielt.
- `src/prozessor/Hostbruecke.cpp`: `prepareToPlay`, Buslayout und Dirty-/Editorgrenze 34–178.
- `sonde/SondeProcessor.cpp`: Konstruktor, Destruktor, Prepare, Float-Callback, Hoststempel, Worker 71–438; Ausgabenutzung der alten Analyseengine im gesamten Sonde-Baum gesucht.
- `src/HoerMarkierung.h`: Auftragserzeugung, Viererspeicher, aktive Verarbeitung, Oversize/Fade, Ereignisübergänge und TDF2-Zustand über die gesamte Datei.
- `core/StampedAudioQueue.h`: Layout, Ringbesitz, Producer-/Consumerpfad, Neustart und Blockquarantäne über die gesamte Datei.
- `core/analysis/Vergleichspegel.h`: gesamter Mess-/Freeze-/CAS-Pfad und Snapshotkopie.
- `src/prozessor/Analyse.cpp`: Worker 91–341, Passagebindung 421–470, Freeze-/Versuchsaufruf 715–856; sonstige Methoden nur gezielt als Aufrufbeziehungen.
- `src/WorkerCadence.h`: vollständig.
- `dsp/DspBankPool.h/.cpp`: Ownership-Protokoll, Generationen, Release/Acquire, Verdrängung, ACK/Reclaim und Reset vollständig.
- `dsp/DspKern.cpp`: Vorbereitung/Freigabe, Programmübergabe, Blockrand, Bank-/Bandverarbeitung, Oversize, Dry/Wet/Hörmatrix und Retire vollständig; Teile der historischen Kommentare nur gescannt.
- `dsp/DspProgramm.cpp`, `DspFilter.h`, `DspSvf.h`, `DspRtWache.h`: Produktionsalgorithmen vollständig; insbesondere Programmentwurf, Rampenkompatibilität, dynamische Steuerrate und RT-Instrumentierung.
- `core/analysis/FeatureEngine.h`: Prepare/Reset/Blockeingang 230–532, Struktur-/Kapazitätsfelder und Einbindung der Teilköpfe; nicht jede öffentliche Auskunftsmethode.
- `core/analysis/featureengine/Zeit.h`: Zeitaddition, Grenzerkennung, Zuordnung und Grenzreset vollständig; `Lautheit.h` vollständig.
- `core/analysis/featureengine/Spektrum.h`: Sampleeingang, Passage-/Peakpfad, Zeitbuchhaltung und FFT-Schieben 31–357.
- `core/analysis/Fft.h`: vollständig; `TruePeak.h`: Koeffizienten-/Ringvertrag und Tick bis 180.
- `src/AnalyseEngine.cpp`: Welch-/Prepare-/Reset-/Samplepfad bis 506, integrierter Lautheitsgetter und Endauswertungsfunktionen gezielt; Kostenkette bis zu tatsächlich verwendeten Outputs verfolgt.

**Tests gezielt gelesen, keine vollständige Testsuite-Abdeckung:**

- `MarkierungTestMain.cpp`: Auftragseinreichung, erlaubte/unerlaubte Verarbeitung, Oversize, NaN-Folgezustand, gültige Samplerate und State-Latch; Thread-/Aufruferstruktur gesucht.
- `QueueStressTestMain.cpp`: Ringgrenzen, Ganzblockdrop, variable Callbackgrößen, threadlokale New-Hooks, Worker-Deadline-Prüfung, Restore/Resetbezüge.
- `Sonde013ExperimentUiTest.cpp`: realer Pegelaufbau, Nicht-Endlich-Gegenprobe und konkurrierender CAS-/Snapshot-Test.
- `Sonde013PassageStateTest.cpp`: `fahreBisPegel`, echte Prozessor-Start-/Retrywege, Fensterfreigabe und Mindestlängen-Negativprobe.
- `DspGoldenTestMain.cpp`: Generation-ABA-Gegenfall, erlaubte Zustandskette, Rückstau/ACK, Echtzeitzähler mit Programmwechseln, Hard-Bypass/Bitgleichheit, getrennte NaN-Eingangs-/Zustandsprüfungen, Dry/Wet-Ausnahme; Vier-Bank-Messabschnitt 4065–4123 einschließlich Helfer `fahreStille` und vollständiger Übergangskette nachgeprüft; übrige Frequenzgang- und Lastabschnitte gescannt.

**Nur gescannt oder offen:** `FeatureEngine`-Teilköpfe `Stereo.h`, `Fingerprint.h`, `Frame.h`, `Vertrag.h` und zweite Hälfte `Spektrum.h`; `LoudnessAccumulator.h`, `KGewichtung.h`, `Statistik.h`, `Konfidenz.h`, `BandGrid*.h`, `Blindvergleich.h`; vollständige DSP-Headerflächen und alle verbliebenen Goldenkorpora. Kein adversarial Komplettbeweis der psychoakustischen/EBU-Referenzmodelle, kein Compiler-/Assemblyaudit, kein TSAN/ASAN, keine echte FL-Matrix, keine Performanceverteilung, kein Soak.

## Empfohlene Reihenfolge

1. Markerübergabe mit echtem Readerbesitz schließen und konkurrierend prüfen.
2. Mindestlänge des Vergleichspegels in Frames ausdrücken und im realen Mono-/Stereo-Prozessor an der exakten Grenze beweisen.
3. Vier-Bank-Nachweis an tatsächliche gleichzeitig rechnende Crossfade-Banken binden; bisherigen Messwert nicht als Worst-Case-Beleg verwenden.
4. Probeeq-Doppelanalyse als eigenes Pflegeticket mit Goldens und Vorher-/Nachher-Lastmessung bearbeiten.
5. Kleine getaktete Hostblöcke und ein Gen plus zehn Probeeq als eigene Performanceprüfung ergänzen. Für Releaseaussagen fehlen weiterhin Callback-Tail-/Deadline-Verteilungen, CPU-/Speicher-/Thread-/Handleverlauf und reale Hostläufe.
