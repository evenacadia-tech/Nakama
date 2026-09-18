# Phase 16, Paket E: aktive Prozessorlast und Lebenszyklen

18.09.2026. Unabhängiger Prüfer Paket E. Unveränderter Produktstand `aff2d8188f33a9525aec1869449773bfdbe305a6` in `C:\na-audit16-aff2d818`. Im gemeinsamen Checkout ausschließlich dieser Rohbericht und Nachweisartefakte. Keine Produktreparatur, Installation, FL-Sitzung oder Sicherheitsprüfung. Die zentralen IDs und die Fable-Reihenfolge stehen in [../BEFUNDE.md](../BEFUNDE.md).

**Ergebnis:** tatsächliche aktive Mehrinstanzlast, Analyse, Hostparameter und die zusammengehörigen Prozessor-Lebenszyklen wurden ausgeführt. Der finale Lastlauf beendet sich mit Exit0 und **17 eng begrenzten erfüllten Assertions**. Das ist keine Host-, Release- oder allgemeine Echtzeitfreigabe. Kurze Audioblöcke zeigen gemessene Dienstzeitüberschreitungen; die Ressourcenmessung verlangt die unten getrennte Einordnung. Aus Paket E wird kein neuer Produktdefekt allein aufgrund einer Maschinenzeit oder eines Working-Set-Anstiegs abgeleitet.

## Nachweisartefakte und Ausführungsgrenze

- [Echter Prozessorharness](phase-16-nachweise/e/last_probe.cpp), [gemeinsames CMake-Projekt](phase-16-nachweise/c/CMakeLists.txt), [isolierter v2-OS-Einstieg](phase-16-nachweise/c/PipeClientIsoliert.cpp), [Start-/Manifestskript](phase-16-nachweise/e/run.ps1).
- [Vollständige finale Ausgabe](phase-16-nachweise/e/ergebnisse.txt), [Manifest samt 156 Produktquellen-Hashes](phase-16-nachweise/e/manifest.json), [erfolgreicher finaler Bau](phase-16-nachweise/e/build-with-binding.log).
- Sämtliche Callbackmessungen: [Stereo48k/64](phase-16-nachweise/e/stereo48k64.csv), [Stereo96k/64](phase-16-nachweise/e/stereo96k64.csv), [gemischte Blocks](phase-16-nachweise/e/stereo48k_mixed.csv), [Mono48k/64](phase-16-nachweise/e/mono48k64.csv), [Offline48k/333](phase-16-nachweise/e/offline48k333.csv). [Maschinell nach Blockgröße ausgewertet](phase-16-nachweise/e/summary.json) durch [summarize.py](phase-16-nachweise/e/summarize.py); dieses Skript berechnet ausschließlich Statistiken bereits gemessener Werte.
- [Ressourcenmessungen des Hauptlaufs](phase-16-nachweise/e/resources.csv), [separater erweiterter Ressourcenharness](phase-16-nachweise/e/resources_probe.cpp), [36-Zyklen-Ausgabe](phase-16-nachweise/e/resources-run.txt), [vollständige CSV](phase-16-nachweise/e/resources-extended.csv), [Ressourcenmanifest](phase-16-nachweise/e/resources-manifest.json) und [dessen Startskript](phase-16-nachweise/e/resources-run.ps1).

Der Harness linkt die echten `EqCopilotProcessor`-/`SondeProcessor`-Quellen, deren Analyseworker, Transaktionskern und DSP-Kern. Er schreibt keine Filter-, Rampen-, Detektor- oder Queueformeln nach. Ein Thread bedient den Gen-Prozessor und zehn Probeeq-Prozessoren nacheinander; gleichzeitig laufen deren wirkliche Hintergrundworker. Die Signalpuffer, Zeitmessungsvektoren und MIDI-Puffer sind vor dem Messlauf reserviert. Signalerzeugung, Automationsaufrufe und Messauswertung liegen außerhalb der gemessenen Audiokette; `nakamaBlockEmpfangen` und `processBlock` liegen darin. Die Zeiterfassung je Einzelprozessor ist als zusätzlicher Harnessaufwand enthalten.

**Isolation:** `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` verhindert den Start der eigenen v3-Clients. Gens trotzdem startender v2-Client verwendet den unveränderten Produktquelltext; nur `CreateFileW`/`WaitNamedPipeW` werden auf `\\.\pipe\evenacadia.eq-copilot.probe.audit16.v2.<PID>` umgeleitet. Auf dieser Pipe läuft kein Server. Keine Produktionspipe wird geöffnet. Die Probeeq-Projektbindung entsteht über den wirklichen serialisierten Stateeingang; keine Frameflags oder Publikationszähler werden von Hand gesetzt.

**Nicht ausgeführt:** VST3-Wrapper, FL Studio, ASIO-Gerät, installiertes Plugin, echter Brokerverkehr während der Audiofristenmessung, Brokerneustart, vollständiger Kanon, Allokationsinstrumentierung auf dem Audiothread oder ein Hörtest. Die Gen-Bundleklasse ist echt; im Lastlauf wird keine angebundene Gen-Main-Sitzung behauptet. Der Quellen-/Komponentennachweis von Paket B ergänzt Verbindungsfehler, ersetzt aber keinen gemeinsamen Audio-/Brokerlastlauf.

## Aktive Last statt EQ-aus-Soak

Je Probeeq sind acht Slots belegt und eingeschaltet. Vier benutzen dynamischen SVF mit tatsächlich aktivem internem Sidechain, Threshold−35dB, Range−6dB und Attack/Hold/Release5/10/120ms. Die Kanalmodi durchlaufen Stereo/Left/Right/Mid/Side. Die Hälfte der Instanzen benutzt Auto-Gain. Zwei echte Hostparameter jeder Probe werden etwa alle20ms über `RangedAudioParameter::setValueNotifyingHost` geändert; die wirklichen Worker übernehmen diese Werte asynchron. Es wird kein synchroner Test-Kontrolltakt als Abkürzung gerufen.

Der finale Lauf misst für **jede** der zehn Instanzen einen von Dry verschiedenen Ausgang, eine dynamische Auslenkung größer0, tatsächliche Feature-/P2-Produktion und endliches Audio. Je Szenario entstehen bei allen zehn Probes28–29 Featureframes und erfolgreiche Producerpublikationen; auch Gen meldet28–29 Featureframes. Keine Audio-NaN/Inf, keine Analyse-Queueüberläufe oder Oversize-Drops in den regulären Lastszenarien. `state_revision` bleibt1, während `automation_epoch`1 erreicht: die Hostautomation wurde als flüchtiger Overlay aufgenommen. Die Zahl beweist keine samplegenaue Automation; diese ist im Produkt ausdrücklich nicht gebaut (`SondeProcessor.cpp:149-155`, `:1296-1304`, `:1414-1444`).

Die wirklichen Evidenzproducer jeder Probe bauen dabei fünf P1-Snapshots; zwei weitere Versuche werden bereits vor dem Bauen gezählt zurückgehalten, weil der absichtlich unverbundene P1-Client nicht abfließt. Das entspricht der Grenze4 in `SondeProcessor.h:494` und dem vorgeschalteten Rückstauschutz in `SondeProcessor.cpp:758-774`. Ein voller256-Einträge-Client oder ein Broker-Ingressüberlauf wird dadurch nicht behauptet. Die vorhandenen Producer-/Kadenzschutzpfade liefen tatsächlich, während Audio weiter verarbeitet wurde.

Prüfmaschine: AMD Ryzen5 7500F,6 Kerne/12 logische Prozessoren, Windows, MSVC x64 Release. Normale Prozesspriorität, keine Kernbindung oder Echtzeitpriorität. Root bestätigte vor dem Lauf das Ende der C-Build-/Testarbeit; kein absichtlich konkurrierender Auditcompiler oder Lastlauf. Andere Betriebssystem- und Benutzerlast ist nicht ausgeschlossen.

| Szenario, je rund3s Audio | Kettenblöcke | p99 | Maximum | Vergleichsbudget n/fs | Dienstzeitüberschreitungen |
| --- | ---: | ---: | ---: | ---: | ---: |
| Stereo48k, Block64 | 2250 | 163,9µs | 271,2µs | 1333,33µs | 0 |
| Stereo96k, Block64 | 4500 | 177,9µs | 491,2µs | 666,67µs | 0 |
| Mono48k, Block64 | 2250 | 193,9µs | 605,6µs | 1333,33µs | 0 |
| Offline48k, Block333 | 433 | 618,7µs | 1725,6µs | 6937,5µs, nur rechnerischer Vergleich | 0 |
| Gemischt48k, Anteil Block1 | 356 | 49,7µs | 115,1µs | 20,83µs | 92 |
| Gemischt48k, Anteil Block7 | 356 | 103,3µs | 207,3µs | 145,83µs | 1 |
| Gemischt48k, Anteil Block64 | 356 | 128,0µs | 153,0µs | 1333,33µs | 0 |
| Gemischt48k, Anteil Block333 | 356 | 608,6µs | 673,7µs | 6937,5µs | 0 |

Gemischt bedeutet wiederholt1/7/64/333 bei fortlaufenden Hoststempeln. **Die93 Überschreitungen sind gemessene Dienstzeiten gegen das jeweils angelegte n/fs-Budget, keine93 FL-Aussetzer.** Der Scheduler des normalen Konsolenprozesses startet einzelne Blöcke bis2,44ms nach dem Soll; diese Verspätung steht ausdrücklich separat im CSV. Werden kleine Aufrufe in einem wirklichen Host als Unterblöcke eines größeren Gerätebuffers verarbeitet, gilt dessen Gesamtbudget. Die Messung rechtfertigt deshalb weder eine pauschale Echtzeitfreigabe noch einen Produktdefekt allein aus dem1-Sample-Fall. Das Offline-Szenario verarbeitet3,00394s Audio in0,21441s Wandzeit; es prüft keine Audio-Gerätefrist.

**Bisheriges Lastbein und sein präziser Rand:** `SessionSoakMain.cpp:494-522` verwendet für Main separate `ControlClient`/`TelemetryClient` plus `SourcesModel`, keinen Gen-Prozessor. `:598-608` lädt für die tatsächlichen Sonden einen frischen aktiven Probe-State, dessen EQ-Default aus ist. `:952-1017` bedient diese Sonden und zählt Blöcke/Segmente; dort werden keine Audio-Callbackfristen gemessen. Sein Brokerneustart-/IPC-/Ressourcenbeweis ist damit nicht widerlegt; er deckt die hier gefahrene aktive DSP-/Gen-Kette nicht ab.

## Gekoppelte Hosteingänge und Rückwege

| Tatsächlich ausgeführte Folge | Assertion und Ergebnis | Gegenweg / Grenze |
| --- | --- | --- |
| Acht aktive Bänder einschwingen → `reset()` → Stille | Ausgang exakt0, Statebytes unverändert | `SondeProcessor.cpp:287-306`; Programm bleibt, nur Audiohistorie endet. Kein echter Wrapper-Reset gemessen. |
| Aktiver Zustand → `releaseResources()` → Block ohne neue Vorbereitung | Ausgabe bitgleich zum Eingang | `:271-285`; unvorbereiteter Rückfall ist beabsichtigt, keine verlorene Persistenz. |
| Release → Vorbereitung96k/Mono → aktive Blocks | Mono wird tatsächlich angenommen, alle Samples endlich | `:232-267`, `:311-333`; kein Monobus-Handgriff in FL behauptet. Kanal-/Auto-Gain-Regel selbst liegt bei F07/Paket D. |
| Vorschau/Candidate → `setNonRealtime(true)` | Preview endet und Hoermatrix wird Processed | `:1284-1292`; hier Status-/Lebenszyklusassertion, kein zusätzlicher statischer/dynamischer Audiovergleich gegen eine zweite Instanz. B7 M-120 prüft diesen gesondert. |
| `processBlockBypassed()` bei aktivem Zustand | Bitgleicher Puffer | Die geerbte JUCE-Bypassfunktion führt keinen Probe-DSP-Callback aus; dies ist ein Komponentenbeleg zum Pfad von T3-01-09, keine Auflösung seiner noch fehlenden Übergangs-/Historienregel. Keine vom echten Host gemessene Bypass-/Reset-Reihenfolge. |
| Vorbereitet64 → tatsächlicher Block333 | Audio endlich, `analyseDropsOhneAudio` steigt | `:396-452`: der übergroße aktive Tap wird verworfen, Analyseverlust gezählt. Diese Probe ist getrennt von den regulär bis333 vorbereiteten Lastfällen. |
| Vorstehende Rückwege, State-Reload, Offline-/Bypasswechsel | Bestätigte Statebytes bleiben bytegleich | Kein Undo-, Revisions- oder Recallverlust in dieser begrenzten Folge. Paket C prüft ausstehende Befehle und ältere Projektstände. |

EQ-aus/ein mit alten Rampen ist bereits T3-15-05 und wird von D unabhängig vertieft; E führt dafür keine neue ID. Gens aktive Hörmarkierung über den geerbten Hostreset bleibt T3-01-10; der hier geprüfte Sonde-Reset ist kein Gegenbeweis zu dieser anderen Klasse. Dynamik beim Same-Instance-Reload ist F12/Paket C+D. F18 Konfliktriegel braucht tatsächlichen Coordinator-/Storezustand (Paket B), keine Ressourcenfolgerung aus einem Prozessorharness. F19 bewahrt die Grenze: Stateklonen am echten Prozessor ist geprüft bei C; ein tatsächliches FL-Preset wurde weder geladen noch verändert.

## Ressourcen und Editorobjekte

Im Hauptlauf entstehen nach der Lastmessung12 Zyklen mit je1 Gen +2 aktiven Probeeq. Jeder Zyklus erzeugt und zerstört drei Gen-Editorobjekte über `createEditorIfNeeded`/`editorBeingDeleted`, lädt dreimal die eigenen Statebytes zurück und fährt `reset`/`release`/`prepare` sowie weitere aktive Blocks. Die Editorobjekte bleiben ohne Desktopfenster; eine sichtbare Hostfenster-/DPI-/Paintprüfung wurde nicht durchgeführt. Probeeq hat produktiv keinen Editor (`SondeProcessor.h:158-159`).

Nach dem jeweiligen Abbau werden **Handles, Threads, PrivateBytes und Working Set** über echte Windows-APIs gemessen. APIfehler brechen den Harness ab; sie werden nicht zu Messwert0. Die Workerzerstörung wird am echten Destruktor ausgeführt (`SondeProcessor.cpp:212-229` stoppt und joint; Gen in `PluginProcessor.cpp:457-483`). Die Lastreihen enden jeweils mit4 verbleibenden Prozessthreads. Ihre `*_destroyed`-Speicherzeile enthält noch die lokalen Messvektoren des Harness; die anschließend bei `cycle_before` gemessene Ausgangslage ist deshalb der bessere Bezug für den Editorlauf.

Der Hauptlauf zeigt nach dem ersten Editorzyklus251 Handles und16 Threads; die Handles bleiben über sämtliche12 Zyklen251, Threads erreichen19. PrivateBytes steigen von66.580.480 auf100.970.496 und bleiben nach2s auf diesem Wert. Das ist ein tatsächlich beobachteter Anstieg, **kein Leakfreiheitsbeweis**. Er trennt allein noch keine prozessweite GUIinitialisierung/Caches von verlorenen Instanzressourcen.

Deshalb lief anschließend der **separate36-Zyklen-Zusatz im selben Prozess**, ohne vorgelagerte Audiotimingmessungen: drei Serien derselben unveränderten Zyklusfunktion, je12Zyklen und2s Nachlauf. Insgesamt36 Gen-/72 Probeeq-Instanzen und108 Editorobjekte wurden erzeugt und zerstört. Exit0, `RESOURCE_RUN_COMPLETE=36`. Das zusätzliche Bauziel wurde erst nach der Timingmessung ergänzt; deren Programm, Binary und Rohdaten blieben unverändert.

| Nach kumulierten Zyklen und2s Nachlauf | Handles | Threads | PrivateBytes | Working Set |
| --- | ---: | ---: | ---: | ---: |
| 12 | 251 | 18 | 78.209.024 | 101.052.416 |
| 24 | 251 | 18 | 102.891.520 | 126.152.704 |
| 36 | 251 | 18 | 102.539.264 | 125.837.312 |

Die dritte Serie erhöht die Speicherobergrenze nicht weiter. PrivateBytes schwanken dort zwischen80.183.296 und103.043.072; zwischen zweiter und dritter Endmessung sinken sie um352.256Bytes. Die Handles bleiben ab dem ersten Editorzyklus251, die Threads ab dem vierten Zyklus18. **Ein linearer Ressourcenverlust pro Wiederholung wurde in dieser36er-Matrix nicht beobachtet.** Das erreichte Plateau ist mit prozessweiter Aufbewahrung beziehungsweise einem Allocator-/GUIcache vereinbar, identifiziert aber keinen Besitzer und garantiert keinen begrenzten Langzeitverbrauch. Die präzise verbleibende Messung wäre ein allokationszugeordneter Vergleich vor dem ersten Editor und nach warmen Serien, getrennt Gen ohne Editor / Gen mit Editor / Probeeq; kein pauschaler RSS-Sollwert und keine Behauptung, alle18 Threads gehörten dem Produkt.

## Unabhängiger Gegenangriff B-02/F15

[Dauerhafte E-Gegenprüfung](phase-16-nachweise/e/b02-gegenprobe.md). **Urteil LÜCKE:** der echte B-Listenerlauf belegt verlorene bereits geschriebene P1-Evidenz nach normalem Linkende. Der stärkste Gegenweg ist das ausdrücklich notwendige Verbot von Callbacks nach Linkabmeldung; es darf nicht durch blindes Leeren umgangen werden. Die vorhandenen Wiederholungsregeln betreffen Clientreserve und Überlauf, keine eindeutige Empfangsbestätigung jeder erfolgreichen Evidenz-Writegrenze. NAK-95 betrifft andere Queuezustände. Die Notiz hält Einstieg, Assertion, Source-/Laufgrenze und den kleinsten zusammenhängenden Folgeauftrag fest. E hat B dabei an Quelle und Harness unabhängig geprüft, den B-Lauf nicht als eigene zweite Ausführung ausgegeben.

## Harnessfehler und offen begrenzte Nachweise

Der erste Kompilierungsversuch scheiterte ausschließlich am Windows-`min/max`-Makro im neuen Harness ([erhaltenes Log](phase-16-nachweise/e/build-initial.log)); `NOMINMAX` beseitigt diesen Werkzeugfehler. Der erste ausführbare Lauf ([erhaltene Ausgabe](phase-16-nachweise/e/initial-run/ergebnisse.txt)) hatte alle Lifecycle-/Audiokriterien erfüllt, aber die Prüfung tatsächlicher P2-Publication fiel fünfmal: ungebundene frische Probeeq bauten zwar Featureframes, konnten ohne gültige Projektadresse keine P2-Nachricht bilden. Der finale Harness lädt deshalb über den echten Stateeingang eine gültige Bindung; danach werden P2-/Evidenzproducer tatsächlich ausgeführt. Es wurde keine Assertion auf bloßes Bestehen abgeschwächt.

Für einen folgenden konkreten Lastnachweis: dieselben aktiven Programme in einer isolierten Hostumgebung mit explizitem Gerätebuffer, Echtzeit-/Offlinewechsel, Gen-Main und echten an Probe-Pipes gebundenen v3-Clients fahren; dabei Brokerneustart, Reconnect und Queueüberlast mit Audiofristen und P0-/P1-/P2-Annahmezählern gemeinsam messen. Ein Langzeitlauf mit Allokationsbesitzern kann die im36-Zyklen-Lauf erreichte Aufbewahrung genauer zuordnen. Dieser Bericht behauptet weder diesen Lauf noch eine vollständige Neuvalidierung aller älteren Last-/Lebenszyklusbefunde.
