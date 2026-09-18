# Phase 16 · Paket C — Persistenz, Reload und Rückwege

18.09.2026, Hauptsession als Finder; unabhängige Prüfung durch Paket D in [phase-16-f-gegenpruefung-c.md](phase-16-f-gegenpruefung-c.md). Geprüft: `aff2d8188f33a9525aec1869449773bfdbe305a6`, unveränderte Produktquellen in `C:\na-audit16-aff2d818`. Keine neue T3-ID aus diesem Paket. Bekannte Ursachen bekommen aktuelle Nachweise und engere Aussagen.

## Ausgeführter Weg

[state_probe.cpp](phase-16-nachweise/c/state_probe.cpp) linkt die wirklichen `SondeProcessor.cpp`, `PluginProcessor.cpp`, deren State-/IPC-/Analyse-Dateien sowie `NakamaKern`, JUCE und FlatBuffers. [CMakeLists.txt](phase-16-nachweise/c/CMakeLists.txt) und [run.ps1](phase-16-nachweise/c/run.ps1) beschreiben den Bau. Release, MSVC x64; finaler Ergebnisnachweis [ergebnisse-reviewed.txt](phase-16-nachweise/c/ergebnisse-reviewed.txt): **33 geprüfte Bedingungen, 0 unerwartete Abweichungen, Exit 0**. Darunter sind ausdrücklich Assertions, die vorhandene Fehler reproduzieren; daraus folgt kein Produkt-PASS. Dieser letzte Lauf enthält auch die nach der unabhängigen Prüfung verschärften Assertions für Revision, tatsächlichen Undo-Zwischenstand und Wiederholungszähler.

Die produktiven v3-Threads sind durch den vorhandenen Testbauschalter deaktiviert. Die beiden Replay-Fälle starten den echten Gen-ControlClient ausdrücklich gegen einen PID-eindeutigen Probe-Server. Gens v2-Client startet dagegen bereits im Konstruktor. [PipeClientIsoliert.cpp](phase-16-nachweise/c/PipeClientIsoliert.cpp) inkludiert die unveränderte `PipeClient.cpp` und ersetzt ausschließlich deren `CreateFileW`-/`WaitNamedPipeW`-Ziel durch einen eigenen Probe-Namen. Damit ist dies ein Prozessor-/Komponentenbeweis mit isolierter Betriebssystemnaht, kein unveränderter Vollproduktstart oder echter VST3-/FL-Nachweis. Es wurde weder ein Produktbroker gestartet noch installiert.

Der P0-Gegenpart ist der vorhandene `V3TestServer.h`, **kein Broker-Coordinator**. Die bestätigungsfähige Quellensicht wird mit dem vorhandenen Testsetter hergestellt. Der geprüfte Abschnitt beginnt danach an `bindeSourcesHauptziel`, benutzt echten JSON-Sender, Queue, Pipe, ACK-Empfang, Callback und Save. Weder Brokerwirkung noch Sichtaufbau aus einem echten Subscriber werden hier behauptet. Paket B prüft die Empfängerseite getrennt; die beiden Ergebnisse sind kein durchgängiger Ende-zu-Ende-Lauf.

## T3-05-01 — vor dem Recall ausstehender Hostwert

**Tatsächlich ausgeführt:** Sonde vorbereiten; EQ ein, Output-Trim +3 dB über echte Transaktion committen; Audio laufen lassen; speichern; über den wirklichen `RangedAudioParameter::setValueNotifyingHost` −9 dB schreiben; dieselben Bytes sofort in dieselbe Instanz laden; echten `dspKontrollTakt` ausführen; Ausgangsaudio messen. Ergebnis: bestätigter Trim **+3 dB**, wirksamer Trim **−9 dB**, gemessener stabiler Ausgang **−8,99999944679 dB**. Die Assertion verlangt alle drei Werte gleichzeitig.

Aufrufkette: `SondeProcessor.cpp:1296` nimmt das Hostereignis an; `:1047` lädt den Transaktionsstand, `:1061` publiziert ihn; `:1388` gleicht die Hostanzeige unter Herkunftssperre ab; `:1421-1430` konsumiert später den alten Mailboxwert. `NakamaTransaktion.cpp:234` leert den Overlay, nicht diese Prozessor-Mailbox. Das bestehende M-84-Golden (`TransactionTestMain.cpp:2170-2191`) lädt eine neue Instanz ohne alte Mailbox und deckt diesen Eingang nicht.

**Gegenargument:** Der echte Worker kann das Ereignis vor dem Reload konsumieren. Das ist richtig und kann diesen Harnisch bei anderer Zeitplanung falschnegativ machen; es widerlegt die protokollierte konkrete Reihenfolge mit altem ausstehendem Wert nicht. Keine statistische Häufigkeit oder FL-Reproduktion behauptet. Eine dauerhafte Regression benötigt zusätzlich eine kontrollierte Workerbarriere, damit beide zulässigen Reihenfolgen deterministisch gemessen werden.

Erreichbarkeit **1, Hostparameter-/State-Eingang**. Keine neue ID; kleinster Korrekturumfang bleibt die Generation des Ladestarts gemeinsam mit Hostmailbox, Overlay und Publikation. M-84 und „State bleibt verlustfrei“ sind die bisherigen Zusagen. Gegenregression: frisches Ereignis *nach* dem Reload muss weiterhin wirken.

## T3-05-02 — read-only erhält Bytes, verliert Audioneutralität

Die vorhandene Fixture `fremdes-major-3.bin` wird geladen. Vor der Automation misst der echte Ausgang **0,00000012943 dB** (Float-Rundungsrest). Danach Hostparameter `eq_enabled=1` und Output-Trim +6 dB; Kontrolltakt; Audio. Ergebnis **+6,00000017258 dB**, während `nurLesen=true` bleibt und `getStateInformation` die **bytegleichen Originaldaten** zurückgibt.

Die Assertion misst Neutralität vor, aktive Verstärkung nach dem Hosteingang sowie Originalbytes. Die Sperre in `fuehreTransaktionAus` (`SondeProcessor.cpp:1195-1215`) ist ein anderer Eingang und widerlegt den Automationspfad `:1421-1445` nicht. Der vorhandene M-92-Fall (`TransactionTestMain.cpp:2008`) prüft Laden und direkte Transaktionsablehnung, keine anschließende Hostautomation. Erreichbarkeit **1**. Der hier gemessene Defekt ist die verletzte Audioneutralitätszusage M-92 (`docs/beweise/SONDE-015.md:1175`), kein Verlust der Originalbytes. Korrektur und Regression gehören zu T3-05-02, nicht zu einer neuen ID.

## F02 — Wiederholung nach Same-Instance-Reload: NAK-278

Zwei vollständige Gegenläufe fahren denselben Sources-Join:

| Folge | Echte P0-Writes | Lokale Mitglieder nach Save | Host-Dirty | Ergebnis |
| --- | ---: | ---: | ---: | --- |
| Join → erfolgreicher ACK → Save | 1 | 1 | 1 | Normaler positiver Rückweg funktioniert |
| Join → erster Write ohne ACK → alter State in dieselbe Instanz → Reconnect → erfolgreicher ACK → Save | 2 mit **derselben command_id** | 0 | 0 | InFlight wiederholt und quittiert, lokale Zuordnung ist bereits gelöscht |

Vor Reload fordert die Assertion `inFlight==1`; danach `inFlight==0`, Wiederholungszähler 1 und gleiche ID beider beobachteter P0-Texte. Ein bloßer Queue-Erfolg könnte diese Behauptung nicht erfüllen. Die Antwort im zweiten Lauf ist die normale positive Testserver-Antwort; ob ein konkreter echter Broker sie positiv oder wegen des ersten Heartbeats negativ beantworten würde, hängt zusätzlich an T3-15-14. Dies wurde hier bewusst getrennt.

`State.cpp:123-145` leert die Sources-Listen und erhöht die Reloadgeneration; `:155-223` fordert Reconnect, hält jedoch den ControlClient am Leben. `controlclient/Nachrichten.cpp:258-281` spielt dessen offenes Register wieder ab. `Ipc.cpp:1062-1081` findet die gelöschte Produktzuordnung nicht mehr und erzeugt keinen lokalen Abschluss. Das ist bereits **NAK-278** (`docs/offene-punkte.md:288`): brokerseitig angewandter, nach Reload lokal verworfener Auftrag und die fehlende Entscheidung, wie geladenes Projekt und extern bestätigte Wirkung abgeglichen werden.

**Status F02: durch bestehenden Befund abgedeckt.** Kein Duplikat als neue T3-ID. Erreichbarkeit **1** bei sichtbarem Join/Remove und Host-Reload; der vollständige Hostablauf wurde nicht ausgeführt. Abzugrenzen ist T3-15-01: dort verbraucht ein alter Callback das ACK vor seiner Generationswache; hier fehlt die Zuordnung bereits durch den expliziten Reload. Fable muss Abbruch-/Abgleichregel für vor dem Reload ausgeführte und erst danach bestätigte Befehle entscheiden, statt den alten Auftrag ungefragt in den geladenen Stand zurückzuschreiben.

## T3-15-03 und A-01 — tatsächliche Gen-Eingänge

`setzeQuellenrolle` setzt über den gebauten Prozessor-API-Eingang zuerst die Rolle `traegt` (Bestandsrevision 1), nach Save `fuehrt` (Revision 2). Reload der alten Bytes liefert wieder Revision 1 bei **gleicher Session-Epoche und gleicher Runtime-Nonce**. Der wirkliche `v3IntentUpdateJson` erzeugt `vollstaendig=true`, `bestand_revision=1` und die ältere Rolle. Damit ist der Senderfall aus T3-15-03 ausgeführt; der Brokervergleich kleinerer Revisionen ist in Paket A/Phase 15 separat hergeleitet. Eine heutige Intent-Bedienfläche existiert nicht; **Stufe 2**, kein gemessener FL-Presetfall. Die ältere-Recall-Autorität bleibt eine Produktentscheidung, kein Anlass für eine zusätzliche ID.

`v3StatusFuerTest` ruft den echten Gen-Statusprovider. Seine Runtime meldet **gemeldet=false**. Das stützt unabhängig den in A-01 gefundenen fehlenden Descriptor; die fehlende Gen-Evidence-Sendefunktion ergibt sich aus der von A und Hauptsession gelesenen vollständigen Senderkette, nicht allein aus diesem einen Boolean. Siehe [Paket A](phase-16-a-verdrahtung.md).

## Positive Rückwege und F19

Ausgeführt mit echten Prozessoren und Transaktionen:

- Commit plus Retry gleicher TID erzeugen genau eine Revision und genau ein Host-Dirty. Ein neuer Auftrag mit alter Basisrevision wird ohne weiteres Dirty abgelehnt.
- Clone/Save/Load bewahren DSP-Hash, Undo-Ring und persistente Instanz-ID; eine neue Prozessorinstanz besitzt eine andere Runtime-Nonce. Das ist die vorgesehene Trennung aus `nakama-state-v2.md:262-269`, kein eigenständiger Klonfehler. Die fehlende Konfliktauflösung gehört zu **T3-15-15**.
- Undo und Redo funktionieren nach Reload des gespeicherten Rings. Der Ladestart löscht transienten Transaktionscache/Preview, nicht die gespeicherte Undo-Geschichte (`NakamaTransaktion.cpp:234-278`).
- Same-Instance-Reload beendet Preview, bewahrt Runtime-Nonce und meldet kein Dirty.
- Ein eigenes Nakama-DSP-Preset übernimmt tatsächlich den neuen Trim −6 dB, bewahrt aber Empfänger-ID, Label und dessen eingeschaltetes `eq_enabled`. Das Preset enthält gemäß `NakamaPreset.h:9-29` bewusst keine Identität. Dieser gebaute API-Weg besitzt noch keinen Presetbrowser.

**Status F19: durch T3-15-15 abgedeckt für normale Zustandsklone; eigener Presetweg als vorgesehene Regel ausgeführt.** Das tatsächliche FL-Presetformat und seine Reihenfolge `setStateInformation/prepareToPlay` sind **nicht gelaufen**. Konkreter verbleibender Hostbeweis: isoliertes Testprojekt, Probeeq-State als FL-Preset speichern, in zweite Instanz laden, beide persistenten IDs/Nonces und Brokerkonflikt protokollieren, anschließend dokumentierten Auflösungsweg prüfen. Keine laufende Nutzerdatei wurde verwendet.

F06 und F12 werden in [Abdeckung](phase-16-abdeckung.md), [Paket D](phase-16-d-dsp.md) und dessen [C-Gegenprüfung](phase-16-f-gegenpruefung-c.md) eingeordnet. Warme Audiostate-Historie, Undo-Ring, Preview und Hostmailbox sind verschiedene Lebenszyklen.

## Harnischfehler und Grenze

Der erste Linkversuch des Prüfprogramms scheiterte an fehlender `juce_cryptography`-Verknüpfung des neuen Audit-CMake. Der erste Replaylauf scheiterte an einer unvollständigen **Sichtfixture** ohne führende Main-ID; deshalb wurde überhaupt kein Join gesendet. Die Ausgangslogs bleiben als `build-initial.log` und `ergebnisse-initial.txt` erhalten. Nach Korrektur ausschließlich dieser Auditdateien und Ergänzung der Preset-Zielwertassertion entstand der erfolgreiche Zwischenlauf `ergebnisse.txt`. Nach unabhängiger Assertionsprüfung wurde erneut gebaut und ausgeführt: [Build](phase-16-nachweise/c/build-reviewed.log), [finaler Lauf](phase-16-nachweise/c/ergebnisse-reviewed.txt). Diese Harnischfehler werden nicht dem Produkt zugeschrieben.

Kein voller Kanon, kein echter Host, keine Installation, keine Aussage über Audio-Thread-Sperren oberhalb des Prozessors. Der wirkliche Worker lief; die positiven schnellen Gegenproben sind keine vollständige Nebenläufigkeitsprüfung. Quellstand und Artefaktintegrität werden zentral geprüft.
