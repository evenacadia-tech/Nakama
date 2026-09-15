URTEIL: DEFEKTE 2 · LÜCKEN 2 · HÄRTUNGEN 0 — Die Transportkerne besitzen ausgearbeitete Stopp- und Rückstauregeln, doch Sonde-Callbacks, Reload-Publikation und zwei verzögerte Abschlusswege bleiben unvollständig abgesichert.

# Tiefenaudit 3 — Phase 04: Lebenslauf und Nebenläufigkeit

| Laufmerkmal | Wert |
| --- | --- |
| Auftrag | Gebundener Prüfauftrag, 2026-09-15 21:42; Kurzname lebenslauf |
| Datum / Zeitzone | 2026-09-15, Europe/Berlin |
| Branch / HEAD vor dem Lauf | master / 93c7a3cefa0568c311ea2652cb622f70f40aedb4 |
| HEAD nach dem Lauf | 93c7a3cefa0568c311ea2652cb622f70f40aedb4 |
| Prüfer | Codex; Modell gpt-6-astra; Effort max, aus dem Startlog gelesen |
| Beginn | 2026-09-15 21:42:56.9613194 +02:00 |
| Ende / Dauer | 2026-09-15, gegen 22:23 +02:00; rund 40 Minuten einschließlich Bericht und Abschlusskontrolle |
| Verfahren | Lesende Quellenprüfung, Aufrufketten, Interleavings, Register- und Testskeptikerpass; keine Ausführung von Produkt oder Tests |
| Schreibumfang | Dieser Bericht und ausschließlich der neue Phase-04-Abschnitt in BEFUNDE.md |
| Ausgangsworktree | Untracked: .claude/settings.local.json, briefing-hub/, nimbalyst-local/, roh/phase-04-auftrag.txt im Auditordner; sämtlich unberührt |

Startlog: `C:/Users/phili/AppData/Local/Temp/nakama-tiefenaudit-p04-93c7a3cefa0568c311ea2652cb622f70f40aedb4-start.log`, vorhandene Datei nur gelesen. Keine Unteragenten. Die Prüfdisziplin aus dem Skill `.agents/skills/nakama-plugin-quality/SKILL.md` wurde auf den ausdrücklich lesenden Auftrag begrenzt; für die vorhandenen Rust-Handle-Kanten wurde außerdem der lokale Skill `unsafe-checker` gelesen. Kein Build-, Test-, Installations- oder Commitauftrag wurde daraus abgeleitet.

## Befundtabelle

| ID | Schwere | Klasse | Kategorie | Ort | Befund | Beleg | Bricht | Vorschlag | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| T3-04-01 | critical | K2 | DEFEKT | eq-copilot/plugin/sonde/SondeProcessor.cpp:139; eq-copilot/plugin/sonde/SondeProcessor.cpp:228; eq-copilot/plugin/core/ipc/ControlClient.cpp:133; eq-copilot/plugin/core/ipc/TelemetryClient.cpp:328 | bekannt NAK-246: Die drei v3-Provider der Sonde fangen weiterhin rohes this; anders als die zehn Gen-Callbacks besitzt keiner eine Besitzschleuse. Nach Ablauf der Client-Stoppfrist kann ein bereits laufender Provider den zerstörten SondeProcessor berühren. Neu ist die bisher nicht abgesicherte Sonde-Hälfte von D2. | Quellherleitung: Provider vor dem ersten Besitzerzugriff anhalten; Sonde-Destruktor joint den Analyseworker, stoppt beide Clients und kehrt nach deren Detach zurück; Provider fortsetzen, etwa v3Status mit Zugriff auf transaktion. Die gemeinsame Client-Laufzeit hält nur ihre eigenen Daten. B14 testet diesen Fall ausschließlich mit EqCopilotProcessor (Sonde012ProjectReloadTest.cpp:440, :466). | „Kein Produkt-Callback greift nach Beginn der Prozessorzerstörung auf Prozessorzustand zu.“ (NAK-246, R-D2, docs/beweise/NAK-246.md:1008). | Alle drei Sonde-Provider durch eine gemeinsame Besitzschleuse führen, diese vor der Mitgliederzerstörung schließen und den Ablauf jenseits der Client-Stoppfrist am SondeProcessor absichern. | offen |
| T3-04-02 | medium | K2 | DEFEKT | eq-copilot/plugin/src/SourcesModel.cpp:1375; eq-copilot/plugin/src/SourcesModel.cpp:1504; eq-copilot/plugin/src/SourcesModel.cpp:479; eq-copilot/plugin/src/prozessor/Ipc.cpp:1087 | bekannt NAK-246: Eine vor dem Reload begonnene evidence_invalidate-Übernahme kann nach projektReload erneut Rücknahmezähler, Grund und Umfang in den geleerten Sitzungsstand schreiben. Neu gegenüber D6 ist das Interleaving zwischen Eingangsprüfung und Übernahme; die sequentielle Leerung funktioniert. | Quellherleitung: gültige Rücknahme passiert die Bindungsprüfung bei :1377; read-only-State laden und projektReload abschließen; denselben Callback bei :1504 fortsetzen. Er prüft weder Reload-Generation noch Bindung erneut und setzt evidenzRuecknahmen von 0 auf 1. M-29 im Test :2423 lädt erst nach vollständig übernommener Rücknahme. | „Die Sicht ist sofort frei von alten Sitzungsobjekten.“ (NAK-246 M-29, docs/beweise/NAK-246.md:651; dort ausdrücklich evidenzRuecknahmen == 0 sowie leerer Grund und Umfang direkt nach setStateInformation). | Beim Eintritt die Reload-Generation erfassen und unter derselben Modellsperre wie projektReload vor jeder Rücknahme-Publikation erneut vergleichen; den gehaltenen Parse über den Reload prüfen. | offen |
| T3-04-03 | critical | K2 | LÜCKE | eq-copilot/plugin/src/PluginEditor.cpp:627; eq-copilot/plugin/src/PluginEditor.cpp:688; eq-copilot/plugin/src/PluginEditor.cpp:707; eq-copilot/plugin/src/PluginEditor.cpp:734 | Das asynchron gehaltene MesspunktPanel kann den Editor und anschließend den Prozessor überleben; sein Destruktor ruft ungeprüft proz.setzeBindung auf. Der SafePointer schützt erst das nachfolgende Neuzeichnen. Für diesen ausgelagerten Panel-Abbau fehlt eine Besitzerregel. | Quellherleitung: Messpunkt-Popover einer Legacy-Instanz öffnen; Editor und danach Prozessor im selben Message-Thread-Zug zerstören; anschließend Modal-Aufräumnachricht verarbeiten. JUCE CallOutBoxCallback besitzt das Panel (:83), Parent-Abbau meldet nur cancel/triggerAsyncUpdate (ModalComponentManager.cpp:74, :88); erst handleAsyncUpdate (:204) zerstört es. VST3-Wrapper :2326 meldet den Editor ab, leert diese CallOutBox nicht synchron. | keine Zusage | Die gesamte Übernahme einschließlich des Destruktorpfads an einen geprüften lebenden Besitzer binden und Editor-/Prozessorabbau vor der verzögerten Modal-Freigabe absichern. | offen |
| T3-04-04 | medium | K2 | LÜCKE | broker/src/briefkasten.rs:570; broker/src/briefkasten.rs:743; broker/src/briefkasten.rs:750; broker/src/briefkasten.rs:876; broker/src/briefkasten.rs:890 | bekannt NAK-286: Ein bereits begonnener Broker-Briefkastentakt darf nach der zweisekündigen Join-Frist weiterlaufen und seine Antwort noch nach Rückkehr von stoppen veröffentlichen. Der Fall laufender Dateiarbeit ist von M-45s zehn neu ausgelösten Takten nach Stopp nicht entschieden oder geprüft. | Quellherleitung: gültige Anfrage bis schreibe_und_spuele laufen lassen, dort länger als JOIN_FRIST halten; stoppen setzt das Flag und verwirft den JoinHandle; Schreibschritt freigeben. Ohne weitere Stoppprüfung folgen Größenprüfung und Umbenennung zur fertigen Antwort. Arc verhindert dabei einen Speicherzugriff auf freigegebene Besitzer; der Befund betrifft den Abschlusszeitpunkt. | keine Zusage | Die erlaubte Restarbeit nach der Join-Frist festlegen und den Abschluss einer gehaltenen Antwort über Stopp und anschließende Freigabe mit der Dateifassade prüfen. | offen |

Zähler: **critical 2 · high 0 · medium 2 · low 0**; **10 widerlegte Verdachte**; **1 Produktfrage**. Kategorien: **2 DEFEKTE · 2 LÜCKEN · 0 HÄRTUNGEN**. Die beiden NAK-246-Zeilen bezeichnen zusätzliche erreichbare Fälle, nicht die bereits behobenen Originalbefunde. NAK-286 bezeichnet beim vierten Befund die bereits registrierte Funktion; neu ist der Ablauf einer vor Stopp begonnenen Antwort.

## Herleitung und Skeptikerpass der Überlebenden

### T3-04-01 — Sonde-Provider außerhalb der Besitzschleuse

**Population:** genau drei Sonde-Lambdas: Control-Hello und Status in `SondeProcessor.cpp:139` und `:140`, Telemetrie-Hello in `:141`. Die Clients starten im Produktzweig bei `:195` und `:196`; auch ohne gültige Kopplung wird der jeweilige Hello-Provider aufgerufen. Die Getter berühren den Besitzer: `v3Hello` bei `:1078`, `v3Status` bei `:1105`/Transaktionskern `:1109`, `v3TelemetryHello` bei `:1181`/Control-Kopplung `:1183`.

**Erreichbarer Ablauf ohne kaputte Nachricht:**

1. Ein Client hat seinen Provider betreten und wird vor dem nächsten Besitzerzugriff verzögert.
2. Der Host zerstört die Sonde. Der Briefkasten endet, Parameterlistener werden abgemeldet und der Analyseworker endet (`:216`, `:222`, `:224`).
3. Der betroffene Client überschreitet die 2-s-Stoppfrist und löst seinen Thread ab. Die Sonde wartet auf keine eigene Callback-Schleuse.
4. Der Destruktor kehrt zurück; später setzt der Client seinen Provider fort. Die Mitglieder `transaktion`, `dspAusfuehrung`, `dspKern` sind dann zerstört (`SondeProcessor.h:535` bis `:537`).

Der Halt darf **nicht** so gewählt werden, dass der Provider bereits das Zustandsschloss hält und damit zuvor den Analyseworker blockiert. Ein Halt vor dem ersten Besitzerzugriff genügt; beim Status existiert außerdem ein Abschnitt nach Freigabe des Zustandsschlosses. Es wird kein beobachteter Absturz behauptet: nachgewiesen ist die fehlende Lebensdauerbeziehung im Quellablauf.

**Gegenbelege geprüft:** `ControlClient::Laufzeit` und die Telemetrie-Laufzeit besitzen ihre Verbindung über `shared_ptr`, nicht den SondeProcessor. Ihre Stopp-/Laufgenerationen werden nach dem Provider geprüft und können einen schon laufenden Besitzerzugriff nicht rückgängig machen (`TelemetryClient.cpp:397`, `:403`). Der Briefkasten hat eine eigene Schleuse; diese umfasst keine IPC-Provider. Die zehn Gen-Callbacks sind dagegen geschützt (`PluginProcessor.cpp:163`, `:171`, `:177`, `:182`, `:195`, `:202`, `:263`, `:313`, `:330`, `:417`; Schließen `:474`).

**Testabgleich:** B14s `mainAnlegen` liefert ausdrücklich `EqCopilotProcessor*` (`Sonde012ProjectReloadTest.cpp:440`). M-07 hält dessen Antwortcallback über die Stoppfrist (`:460`); M-06 prüft spätes Betreten. Die generischen Schleusentests in `IpcTestMain.cpp:2581` und `:2777` installieren die Schleuse selbst. Keine dieser Konstruktionen schützt oder prüft die drei Sonde-Provider. NAK-272 E3-3 betrifft die nicht ablösenden v2-/BrokerLifecycle-Callbacks, nicht diese drei ablösbaren Provider.

### T3-04-02 — Rücknahme nach der Reload-Grenze

`uebernehmeEvidenzruecknahme` hat zwei getrennte Modell-Sperrabschnitte. Der erste prüft nur, ob Bindung und Sitzung überhaupt gesetzt sind (`:1375`). Dazwischen liegen Textriegel, JSON-Parse und Umfangsprüfung. Der zweite Abschnitt (`:1504`) publiziert ohne erneute Zustandsprüfung.

Ein gültiges `invalidate-ganze-sitzung` des alten Laufs kann deshalb zuerst die Eingangsprüfung passieren. Danach lädt `State.cpp:175` einen read-only-State und ruft `sourcesModel.projektReload` bei `:197` auf. Dieses erhöht/übernimmt die Reload-Generation, löscht die erwartete Bindung und leert die Rücknahmefelder (`SourcesModel.cpp:479`, `:493`, `:510`, `:534`). Wenn anschließend derselbe alte Callback fertigparst, erhöht er den gerade geleerten Zähler und trägt alten Grund/Umfang erneut ein (`:1519`). Er kann vor Rückkehr von `setStateInformation` oder danach fortsetzen; beides ist mit den getrennten Sperrabschnitten vereinbar.

**Skeptiker:** Der Callback besitzt den Prozessor dank Gen-Schleuse weiterhin sicher; diese schützt Lebensdauer, nicht Projektzugehörigkeit. `v3Antwort` (`Ipc.cpp:1063`) trägt keine äußere Reload-Sperre. `controlV3.reconnect()` beendet keine bereits begonnene Übernahme synchron. Der Snapshot-Leser hat immerhin eine erneute Bindungs-/Sitzungsprüfung bei `SourcesModel.cpp:1235`; genau diese fehlt im Rücknahmeleser. Die neue Mitglieder-Publikation hat einen Vergleich der Reload-Generation unter derselben Sperre (`:361`), ist aber ein anderer Eingang.

**Testabgleich:** `Sonde012ProjectReloadTest.cpp:2423` sowie `Sonde012SourcesModelTest.cpp:1182` prüfen vollständig aufgebauten Altstand → Reload → leeren Stand. Keine der gelesenen Wachen hält die Rücknahme zwischen Eingangsprüfung und Publikation. Der Vorgängeraudit nennt diese Richtung bei `UEBERGABE-AN-CLAUDE.md:285` noch als unbestätigten Kandidaten; sie gehört nicht zu den über NAK-283 geschlossenen 14 Gruppen. Hier ist der konkrete Ablauf samt Publikationsstelle hergeleitet. Kein persistenter Datenverlust wird behauptet.

### T3-04-03 — Panel-Abschluss nach Besitzerende

Dies ist eine Speicherlebensdauerprüfung der vorhandenen Oberfläche, keine Designarbeit.

Der Editor übergibt das Panel an `CallOutBox::launchAsynchronously`, ohne dessen Lebensdauer selbst zu besitzen. Die lokale JUCE-Implementierung legt einen `CallOutBoxCallback` an; dessen `unique_ptr<Component> content` hält das Panel (`juce_CallOutBox.cpp:62`, `:83`, `:93`). Der Elternzeiger des Popovers ist kein Eigentumszeiger.

Beim Editorabbau meldet JUCE die Löschung des Eltern-Components und entfernt die Kinder (`juce_Component.cpp:271`, `:278`). Der Modal-Manager setzt `isActive=false` und plant eine asynchrone Aufräumnachricht (`juce_ModalComponentManager.cpp:70`, `:81`). Erst deren Verarbeitung zerstört die gespeicherten Callbacks und damit das Panel (`:196`, `:204`). Wird der Prozessor nach dem Editor, aber vor dieser Nachricht zerstört, läuft `~MesspunktPanel` später mit hängender Prozessorreferenz.

**Skeptiker:** Der SafePointer bei `PluginEditor.cpp:737` verhindert nur `editor->uiDirty`. Die linke Seite von `if (proz.setzeBindung(...) && geaendert)` bei `:707` läuft vorher und nimmt bereits den Prozessormutex (`State.cpp:304`). Auch unveränderte Eingaben umgehen diesen Zugriff nicht. Der VST3-Wrapper meldet den Editor bei `juce_audio_plugin_client_VST3.cpp:2327` korrekt ab und schließt PopupMenu-Menüs; das ist kein synchroner Abbau der CallOutBox-Inhalte. Die gültige Reihenfolge Editor → Prozessor wird hier eingehalten; es wird kein Host verlangt, der einen noch offenen Editor nach dem Prozessor zerstört.

**Einordnung:** LÜCKE, weil kein gelesener Gate-/Matrix-Satz die Lebensdauer dieses ausgelagerten Panels ausdrücklich regelt. R-D2s konkrete Client-Schleuse wird nicht stillschweigend auf jeden JUCE-Dialog ausgedehnt. Der technische Befund ist trotzdem ein herleitbarer Zugriff auf freigegebenen Speicher; deshalb critical. Ein entsprechender Modal-Abbautest wurde in den durchsuchten Plugintests nicht gefunden. T3-02-04 betrifft am selben Panel die Kürzung von gespeicherten Strings und ist davon unabhängig.

### T3-04-04 — Broker-Antwort nach Rückkehr des Stopps

Der Broker prüft das Stoppflag am Anfang der Warteschleife, gibt deren Sperre vor `takt()` frei und prüft es erst nach dessen Ende wieder (`briefkasten.rs:560`, `:570`, `:575`). Die laufende Antwort kann während eines normalen Dateischritts verzögert werden. Der Stoppweg setzt das Flag unter der richtigen Sperre und weckt korrekt; nach zwei Sekunden ohne Threadende wird nur der JoinHandle fallengelassen (`:868`, `:876`, `:890`).

Bei anschließend erfolgreicher Rückkehr aus `schreibe_und_spuele` folgen Größenprüfung und Umbenennung ohne Stoppprüfung (`:743`, `:746`, `:750`). Die fertige Antwort kann somit nach `stoppen()` und nach Beginn des Abbaus der anderen Brokerteile erscheinen. Das Register sowie die Fassaden bleiben durch Arc gültig; **kein** Use-after-free und **kein** Überschreiten der zugesagten Join-Frist werden behauptet.

**Skeptiker:** F-12 und M-45 legen die Frist und Reihenfolge fest. M-45s Teil „nach dem Stopp keine Lieferung“ konkretisiert jedoch zehn **neu ausgelöste** Takte, nachdem der Thread zuvor in seiner Wartebedingung stand. Genau das prüft `nach_stopp_keine_lieferung` (`:1791`). `stoppreihenfolge_briefkasten_zuerst` (`:1817`) prüft den Reihenfolgepfad, keine zurückgehaltene Dateiarbeit. Die Behandlung einer bereits begonnenen Antwort nach Fristablauf ist nicht festgelegt. Deshalb LÜCKE und Produktfrage P04-1; kein erneuter Befund gegen die ausdrücklich ohne Zeit-Rotbeweis dokumentierte Frist.

## Population und Kanteninventar

### Erfassung

Die vorgegebene Population enthält **64 vorhandene Dateien: 46 Plugin-Dateien und 18 Broker-Dateien**. Davon sind sechs eigenständige Broker-Testmodule; die übrigen 58 Dateien enthalten Implementierung, Header oder Fabriken. Alle Pfade wurden mit `rg --files` erfasst und nach Lebenslauf-/Besitzstellen durchsucht. Deren Aufrufketten wurden anhand der Quellen gelesen, einschließlich der Gegenpfade.

Verwendete Suchfamilien: `std::thread`, `.spawn(`, `join`, `detach`, `start/stop`, `startTimer/stopTimer`, `std::function`, `add/removeListener`, `onClick/onReturnKey/onFocusLost`, `launchAsynchronously/callAsync`, `Queue/Ring/Puffer`, `VecDeque/HashMap`, `reloadGeneration`, `beginneSubscription/controlEnde`. Testhaken wurden von Produktregistrierungen getrennt; ein Suchtreffer allein wird nicht als Laufzeitbeweis behandelt. Reine Parser-, DSP- und Zeichenfunktionen wurden nur soweit gelesen, wie sie Besitz oder Übergabe berühren.

### Threads: 19 Erzeugungsstellen im Produktpfad

Sechs C++-Erzeugungsstellen ergeben beim Paar Gen/Probeeq acht Worker: fünf bei Gen, drei bei Probeeq. Der Spike erzeugt keinen eigenen Worker. Auf der untersuchten Broker-Seite liegen 13 Erzeugungsstellen; die Zahl gleichzeitig lebender Instanzen ist verbindungsabhängig. Testthreads sind darin nicht enthalten.

| Nr. | Thread / Start | Stopp, Besitz und Join |
| --- | --- | --- |
| 1 | Gen-Analyse: PluginProcessor.cpp:436 | workerLaeuft=false, notify, join bei :476–482; vor Mitgliederabbau; kein Produktcallback zerstört den Besitzer von diesem Worker aus. |
| 2 | Sonde-Analyse: SondeProcessor.cpp:189 | Flag :224, notify :225, join :227; Listener vorher abgemeldet; betrifft nicht die später abgelösten Provider aus T3-04-01. |
| 3 | v2 PipeClient: PipeClient.cpp:240 | Lebenslaufmutex, Abbruch der Pipe, Weckruf, fristloser Join :249–260; kein Detach, Provider leben bis Join; Self-Stop bleibt bekannt NAK-268 B-7. |
| 4 | ControlClient: ControlClient.cpp:85 | Flag und Link-ab vor IO-Abbruch :102–106; threadId-Prüfung :95; 2-s-Frist, Detach :133; Laufzeit und Verbindung je Lauf mit Besitz. |
| 5 | TelemetryClient: TelemetryClient.cpp:280 | Flag :295, Generation, IO-Abbruch, threadId-Prüfung :290; 2-s-Frist, Detach :328; eigenes Verbindungsobjekt je Lauf. |
| 6 | BrokerLifecycle: BrokerLifecycle.cpp:646 | Stop/notify/join :649–662; eigene Hooks nur von diesem Thread; Frist/Self-Stop unverändert bekannt NAK-192 und NAK-268 B-6. |
| 7 | Broker-Supervisor: lebenslauf.rs:180 | Stopflag vor Join in BrokerSupervisor::drop :75–80; Store-Wartekante und fehlender Fristbeweis bleiben NAK-268 B-1/B-2. |
| 8 | Broker-Briefkasten: briefkasten.rs:836 | Flag unter Weckmutex :868, notify, Join mit 2-s-Frist; Arc-Kern bleibt bei Ablösung erhalten; T3-04-04. |
| 9 | v2-Acceptor: server.rs:532 | Griff setzt Stopp, CancelSynchronousIo wird bis Threadende wiederholt; Join über :283. |
| 10 | v2-Verbindung: server.rs:623 | Register der JoinHandles, Ernte fertiger Threads :314; geordneter Stopp benutzt denselben Abbruch-/Join-Helfer. |
| 11 | v2-Flush: server.rs:941 | Flush besitzt sein Pipe-Handle; Fristpfad bricht den synchronen IO-Thread ab und wartet auf sein Ende. |
| 12 | v3-Wachhund: listener.rs:371 | gemeinsames Stopflag, periodischer Tick; V3Griff joint :330; auch bei Startfehler nach Flagsetzung gejoint (:419, :681). |
| 13 | v3-Acceptor: listener.rs:440 | Stopflag plus acceptor_stop-Event :320–321 in griff.rs; Registrierung vor Übergabe, Stopfenster durch tests_lebenszyklus.rs:158 abgedeckt. |
| 14 | v3-Verbindung/Leser: listener.rs:614 | V3Griff hält JoinHandle; wiederholter IO-Abbruch bis Ende :342; Verbindungsroutine beendet Queues und koppelt vor ihren inneren Joins ab. |
| 15 | Senkenruf: verbindung.rs:63 | Rückgabe über sync_channel(1), join_mit_frist :72; nach Frist bleibt nur besitzender Thread; keine geliehene Senkenreferenz. |
| 16 | v3-Writer: verbindung.rs:497 | Ende-/Ausgangsflag und IO-Abbruch; Join mit SENKE_FRIST bei :849; gehaltene Handle-/Queue-Besitzer. |
| 17 | v3-P0-Verbraucher: verbindung.rs:593 | Eingang schließen vor Join :844; getrennt vom Restverbraucher, damit hängendes P1 kein P0 blockiert. |
| 18 | v3-Restverbraucher: verbindung.rs:622 | gleicher Stopp-/Fristpfad; blockierende Senke besitzt ihre Arc-Kopie bis Rückkehr. |
| 19 | v3-Flush: win_handles.rs:438 | eigenes Handle in besitzendem Helfer; IO-Abbruch-/Fristpfad :462; kein Borrow auf Stack des beendeten Aufrufers. |

Fristlose äußere Joins sind kein pauschaler neuer Defekt: Die inneren IO-/Senkenpfade haben Abbruch- und Fristregeln, und die gesonderten, bereits registrierten Lücken bleiben benannt. Eine reale obere Laufzeitgrenze wurde in diesem lesenden Auftrag nicht gemessen.

### Timer, Listener und entkommende Callbacks

**Drei eigene Timer-Startstellen:** Editor 30 Hz (`PluginEditor.cpp:230`), Spike-Editor 8 Hz (`AuxSpikeEditor.cpp:39`), Diagnosekern 1 000 ms (`Briefkasten.cpp:1003`, derselbe Typ in beiden Prozessoren). SourcesModel besitzt keinen eigenen Timer und keinen Listener; seine Pflege erfolgt über Worker bzw. Editor-Aufrufe.

| Population | Sämtliche Registrierungsstellen / Übergabe | Lebensdauer und Thread |
| --- | --- | --- |
| Gen: 10 v3-Callbacks | PluginProcessor.cpp:163, :171, :177, :182, :195, :202, :263, :313, :330, :417 | alle mit CallbackSchleuse; Provider/Antwort/Frame auf Clientthreads; negativer Linkstatus auch synchron im stop-Aufrufer; Replay/Verwerfungs-Hooks teilweise unter sendeMutex. |
| Sonde: 3 v3-Provider | SondeProcessor.cpp:139, :140, :141 | Clientthreads, rohe Besitzerreferenz; T3-04-01. |
| Gen: 3 v2-Provider | PluginProcessor.cpp:135, :150, :151; Speicherung PipeClient.h:151 | Pipe-Thread; durch unbedingten Join vor Besitzerabbau beendet. |
| BrokerLifecycle: 7 Hook-Slots | BrokerLifecycle.h:71–77; Belegung PluginProcessor.cpp:210–234 | verbunden, connectFehlgeschlagen, serverPruefstatus, darfStarten, reconnect, pruefen, spawn; Lebenslaufthread, keine nachträgliche Umverdrahtung im Produkt. |
| ControlClient: 2 im Produkt unbenutzte Slots zusätzlich zu den belegten Pfaden | Laufzeit.h:122 und :88 | alter beiAntwort-Slot und hookAuftragAbgeschlossen; letzterer bekannt NAK-272 E3-1, kein neuer Zugriff. |
| Briefkasten: 2 Prozessor-Quellen | PluginProcessor.cpp:432; SondeProcessor.cpp:186 | Message-Thread, hinter eigener Besitzschleuse; Kern-Freigabe per callAsync :820 oder bewusst bis Prozessende gehalten :845. |
| Editor: 10 feste Knopfaktionen | PluginEditor.cpp:121, :132, :147, :154, :162, :165, :166, :173, :179, :193 | Belegung über initKnopf :118; Kind-Komponenten gehören dem Editor, Aktionen laufen auf Message-Thread. |
| Editor: 2 Textaktionen | PluginEditor.cpp:209, :210 | onReturnKey/onFocusLost des Sources-Labels; direkte Besitzeraktion auf Message-Thread. |
| Hinweise: 2 Callback-Übergabestellen | PluginEditor.cpp:454 und :510 | Knopf je Hinweis und SafePointer auf Editor; zwei alternative CallOutBox-Starts :523/:531. |
| Konfliktpanel: 2 Callback-Übergabestellen | PluginEditor.cpp:560 und :589 | eigener Knopf und Aktion an Editor; asynchroner Panelstart :597; kein Prozessorzugriff aus Paneldestruktor. |
| Messpunktpanel: 1 externer Änderungs-Callback | PluginEditor.cpp:737 | SafePointer für Neuzeichnen; direkter Prozessorzugriff davor ist T3-04-03. |
| Spike: 2 Knopfaktionen | AuxSpikeEditor.cpp:19, :28 | Message-Thread, Prozessor lebt nach JUCE-Vertrag länger als Editor. |
| Listener: 5 Registrierungsstellen, 117 Registrierungen bei einer Sonde und einem Gen-Editor mit offenem Panel | PluginEditor.cpp:99 (1 Combo), :218 (1 Maus), :668 (1 Panel-Combo), :680 (2 Panel-Texte); SondeProcessor.cpp:163 (112 Parameter) | Editor-/Panel-Listener mit ihren Kindern; Sonde entfernt alle 112 vor Worker-Join :222. kHostParameter==112 ist in state/NakamaParameter.h:51 statisch geprüft. |
| Broker-Senke: 12 Methoden | senke.rs:45, :52–55, :57–59, :63, :66, :69, :74 | fünf Lebenslaufmethoden, je drei Basis-/Minor-Nachrichtenmethoden und Abweisung; blockierbare Senkenrufe isoliert, control_schliesst synchron (bekannt B-4). |

Zusammen sind dies **19 UI-Callback-Übergabestellen** (10 + 2 + 2 + 2 + 1 + 2) und **vier asynchrone Popover-Startstellen**. Lokale, sofort ausgeführte Hilfslambdas zum Aufbau von Layout/JSON wurden nicht als entkommende Callbacks gezählt. Testhaken sind in den Client-/Briefkasten-Laufzeiten separat erkennbar; sie erzeugen keine zusätzliche Produktregistrierung.

Editorrevisionen: Sources wird bei `PluginEditor.cpp:249` nur bei neuer Revision oder UI-Änderung übernommen; Messsnapshot bei `:288`. Der Timer ist damit kein Versprechen einer 30-Hz-Datenquelle. Der Spike vergleicht eine Signatur (`AuxSpikeEditor.cpp:67`). UI-Abbau auf dem Message-Thread wird durch JUCE-Timer-/Komponentenbesitz getragen; der besondere verzögerte Besitzer des Messpunktpanels ist davon ausgenommen.

### Queues, Ringe, Zwischenpuffer und Registraturen

Die folgende Liste zählt **28 Speichergruppen**; verbundene Felder eines Protokolls stehen gemeinsam mit ihrer Feldanzahl. Sie erfasst die dauerhaften Übergabe-/Rückstau-/IO-Zustände der Population, keine kurzlebigen JSON-Strings oder DSP-Arbeitsarrays ohne Übergabe.

| Nr. | Speicher / Feldpopulation | Quelle und Politik / Abfluss |
| --- | --- | --- |
| 1 | Control-P0: Queue plus reservierter Schreibplatz | IpcQueues.h:125; Laufzeit.h:199; Cap 64 einschließlich Reservation; voll wird abgewiesen/gezählt, kein stilles Ersetzen; Wire-Commit bestätigt, fehlgeschlagener Write gibt zurück. |
| 2 | Control-P1: Hauptqueue, Retryqueue, Reservation | IpcQueues.h:345; je 128 Haupt-/Retryplätze; gleicher Schlüssel wird ersetzt, Ereignisse bleiben FIFO; erfolgreicher Commit ruft :419 abfliessen auf, kein Reconnect nötig. |
| 3 | Control-Persistenz: inFlight-Vektor | Laufzeit.h:214; Cap kCapP0, command_id bleibt mit Inhalt erhalten; dreiwertige Annahme, ACK/Fehler räumt; Reconnect spielt weiter; bekannte P0-Aufbaulivelock-Kante NAK-270. |
| 4 | Telemetrie-P2: drei besessene Slots bei zwei wartenden Frames | TelemetryClient.cpp:234; IpcQueues.h:637; Slotbesitz verhindert Überschreiben eines Lesers; Überlast ersetzt/verwirft Analyseframes. |
| 5 | Gen-Interventionsring | PluginProcessor.h:1677; InterventionsRing.h; SPSC, 32 Ereignisplätze, Sticky-Überlauf; Audio schreibt, Worker leert; unbenutztes zuruecksetzen bleibt B-8. |
| 6 | Gen-Audioqueue: Sample- und Deskriptorring | PluginProcessor.h:1420; StampedAudioQueue.h:619/:620; Ganzblock oder Verwerfen, Generation beim Produzenten, kein Zurücksetzen beider Enden bei laufendem Leser. |
| 7 | Sonde-Audioqueue: Sample- und Deskriptorring | SondeProcessor.h:454; derselbe Queue-Typ; Worker-Verbrauch :529 in SondeProcessor.cpp, Neustart via Generation. |
| 8 | Gen-Blockquarantäne: zwei Arbeitsblöcke | StampedAudioQueue.h:852, Gen-Worker Analyse.cpp:133; verzögerte Übergabe prüft Kontinuität/Nachbarn, nur Worker besitzt die Blöcke. |
| 9 | Sonde-Blockquarantäne: zwei Arbeitsblöcke | SondeProcessor.h:455; identischer Besitztyp, benutzt durch Sonde-Worker. |
| 10 | Sources-Aufträge: ausstehende Map und bestätigter Vektor | PluginProcessor.h:1332/:1333; command_id bleibt bis ACK erhalten; sourcesCommandMutex, Worker-/Save-Drain, Reload leert beides unter Generation. |
| 11 | Enden: ausstehendeEnden | PluginProcessor.h:1783; Cap 256; bei vollem Deque ältestes Ende über nachlaufFristSetzen konservativ abschließen, dann ersetzen (Ipc.cpp:273); ACK entfernt passend zur Marke. |
| 12 | Zugestellter Mitschnitt: gesendeteInterventionen | PluginProcessor.h:1835; Cap 256, ältester Diagnoseeintrag fällt; Sendezustandsmutex, kein Steuerauftrag. |
| 13 | Noch nicht bestätigter Mitschnitt: ausstehendeMitschnitte | PluginProcessor.h:1845; Cap 256, ältester Mitschnitt fällt; Marke ordnet Write-ACK zu (PluginProcessor.cpp:580). |
| 14 | Sonde-Hostmailbox: hostWert, hostEreignis, hostEreignisGesehen | SondeProcessor.h:541–543; je 112 Felder; letzte Werte koaleszieren, Ereigniszähler/Offenflag wecken Worker; Geste separat unter Zustandsschloss. |
| 15 | Plugin-Briefkasten: Kennungsring, Merker, offener Auftrag | Briefkasten.cpp:533/:542/:551; 256 FIFO-Kennungen, erst nach erfolgreichem Umbenennen merken; maximal drei Schreibversuche; kein Löschen im Antwortweg. |
| 16 | Broker-Briefkasten: Kennungsring, Merker, offener Auftrag | briefkasten.rs:499/:504/:607; gleiche 256er-Politik, Mutex schützt Taktzustand; Ausgabe nach Stopp gesondert T3-04-04. |
| 17 | Wire-Stromleser: Bytepuffer und Leseposition | WireEnvelope.h:230; WireEnvelope.cpp:227; Payload zeigt nur bis nächstem Füttern in eigenen Puffer; Aufrufer verarbeitet/kopiert synchron vor nächstem Read. |
| 18 | Wire-Ratenzähler: fester Zeitstempelring | WireEnvelope.h:248; WireEnvelope.cpp:269; Verbindungsthread allein, kein Übergang in Audio; Grenzwert führt zum Abbruch. |
| 19 | v2-Client: empfangene Teilzeile | PipeClient.cpp:493; Eigentum beim einen Pipe-Thread, Größen-/Zeilenendegrenze; Stopp bricht IO ab, kein zweiter Leser. |
| 20 | Broker-v3: Bootstrap-/Frame-Rohpuffer | verbindung.rs:203; ein Leser je Verbindung; nach vollständigem Frame Übergabe als eigene Bytes an Eingang, keine geliehene Senken-Payload nach Stackende. |
| 21 | Broker-Eingang: eine Ingressqueue mit P0- und Restentnahme | queues.rs:11; transport/warteschlange.rs:356; 256 Slots und 4 MiB Bytebudget; zuerst P2 verdrängen, sonst P0/P1 trennen und P2 verwerfen; Close-Flag vor Entnahme. |
| 22 | Broker-Ausgang: Writerdeque plus Schlüssel-Hochwassermarken | queues.rs:108; Cap 256; P0 priorisiert, Snapshots je Objekt ans Ende ersetzt, P2 je Quelle ersetzt; Close leert unter derselben Sperre; Hochwassermarken bleiben bis Verbindungsende (bekannt NAK-274). |
| 23 | Zwei Familien von Einzelantwort-Kanälen | verbindung.rs:60 und queues.rs:191; sync_channel(1) für Senkenresultat und Write-Ergebnis; Empfänger hat Frist, spätes Senden scheitert bei wegem Empfänger ohne Borrow. |
| 24 | Ausgangsregistrierung: zwei Maps | griff.rs:60/:61; Control-/Telemetrie-Link → Arc<Ausgang>; Abmeldung vor inneren Joins in verbindung.rs:800 ff.; keine temporär geliehenen Writer. |
| 25 | Trennregister: Map mit Melder je Control-Link | trennung.rs:48; erwartete und gemeldete Telemetrietrennung unter Mutex; maximal SENKE_FRIST warten, Status bei abgelöster Kopplung angepasst. |
| 26 | v3-Besitzregister: Restlistener, Verbindungsjoins, Bootstrapfristen, Handles | listener.rs:283/:286/:288; griff.rs:22–26; Grenze 96 aktive Worker, 98 Pipeinstanzen; Registrierung/Übergabe und Ernte haben Gegenpfade, Restlistener zuletzt geschlossen. |
| 27 | Sources-Modell und Editor-Snapshots | SourcesModel.h:391/:392/:394/:421/:422; Modellmutex, eigenständige Sichtkopie; Mitglieder-Publikation mit Folge/Generation, drei Wire-Übernahmen bei SourcesModel.cpp:621/:1366/:1526; Rücknahme ohne Reload-Abgleich ist T3-04-02. |
| 28 | Broker-v2: unvollständige Eingangszeile und Antwort-IO | server.rs:663 ff.; ein Verbindungsthread besitzt den Empfangspuffer bis zur vollständigen Nachricht, Flush-Helfer besitzt den separaten Antwortweg; Abbruch und Join beenden die Nutzung vor Freigabe. |

Zusätzliche nur threadlokale Puffer (etwa Analyse-L/R, FlatBuffer-Ausgabe, v2-/v3-Lesechunks, temporäre Envelope-Ausgabe) wurden am Aufrufort auf Besitz bis zum synchronen Verbraucher geprüft; sie sind keine weiteren geteilten Warteschlangen. Die drei generischen Rust-P0/P1/P2-Queue-Typen in `transport/warteschlange.rs:84`, `:165`, `:295` wurden beim Nachverfolgen des Eingangs mitgelesen; dessen produktiver Speicher ist die Ingressqueue aus Zeile 21.

### Verbinden, Trennen und Beziehungspaare

| Paar / Kante | Ergebnis der Quell- und Testprüfung |
| --- | --- |
| Control anmelden → Callback → Welcome | verbindung.rs:300–345 wartet den Senkenruf ab, bevor Welcome geschrieben wird; eigener Test tests_kopplung.rs:112. Ein abgelöster Anmelder ist ausdrücklich Sonderpfad, kein vorgezogenes Welcome. |
| Telemetrie koppeln → Control-Ende | TrennMelder hält die erwartete Meldung; Tests bei tests_kopplung.rs:192, :228, :284, :361, :448 decken beide Endreihenfolgen und abgelöste Rückrufe ab. |
| Verbindung registrieren ↔ stoppen/ernten | Listener registriert vor Bedienung; tests_lebenszyklus.rs:127 und :158 prüfen Ernte und Stoppfenster. Griff bricht IO wiederholt ab und räumt Listener zuletzt. |
| Reconnect ↔ alte Verbindung schließen | beide C++-Clients besitzen pro Lauf eine eigene IpcVerbindung; alte Laufgeneration darf die neue Verbindung nicht schließen. Backoff-/Kopplungswartefälle in IpcTestMain.cpp:1430 und :1868, v2 in PipeClientLifecycleTestMain.cpp:964. |
| Broker stirbt / voller oder halber Frame ↔ Client-Stopp | IpcVerbindung arbeitet mit absoluter Frist, Overlapped-Abbruch und eigenem Verbindungsobjekt; PipeClientLifecycleTestMain.cpp:490 prüft eine Deadline für den ganzen Frame; Tests nur gelesen. |
| P1 voll ↔ späterer Abfluss | IpcTestMain.cpp:3804 sowie :3831/:3850 prüfen Retry/Reservierung/Schlüssel; gleichwertiger Rust-Abfluss bei warteschlange.rs:268. Kein Wiederanlauf als einzige Entleerung. |
| ACK ↔ persistente Übernahme | Ipc.cpp:1402/:1538 nimmt unter Drain-/Command-/Bindungssperren geordnet ab; State.cpp:97 drainiert vor Save; kein Editorzwang mehr. Gegenfall Reload durch Generation abgefangen, Rücknahmeleser hiervon getrennt. |
| Hostautomation ↔ DSP-Übernahme | Sonde parameterValueChanged :1296 schreibt Mailbox; Worker :1413 übernimmt; Preview/Transaktion außerhalb Audio unter Zustandsschloss; keine neue direkte Pipe-Mutation von Audiozustand in dieser Population gefunden. Vollständiger DSP-Beweis bleibt anderer Phase vorbehalten. |
| Editor öffnen ↔ schließen | Hostbruecke.cpp:165 ff. und PluginEditor.cpp:216/:233; Marker wird beim Schließen beendet, Editorflag zurückgesetzt; ausgelagerter Messpunktpanelabschluss T3-04-03. |
| Briefkasten starten ↔ stoppen | beide Plugin-Konstruktoren und Destruktoren paarig; M-34-Tests bei BriefkastenTestMain.cpp:2957, :3033, :3054; Broker startet nach Servern und stoppt zuerst, lebenslauf.rs:171/:242. Offener Fall laufender Broker-Dateiarbeit T3-04-04. |
| Kennung neu ↔ genau einmal beantworten | Plugin-Tests :1002/:1044/:1415, Broker-Tests :1398/:1469/:1917; erste Antwort, Wiederholung und Verdrängung nach 256 Kennungen vorhanden. „Genau einmal“ gilt im festgelegten Ringfenster, nicht als ewiges Archiv. |
| aktivieren ↔ abklingen | Interventionsring, Ende-Marken, ausstehendeEnden, Replay und Tail-Frist gemeinsam gelesen; bestehende NAK-155/NAK-158 werden nicht erneut aufgemacht. Vollständige hörbare DSP-Abklingmessung nicht ausgeführt. |
| installieren ↔ Rückweg | außerhalb dieser Phase; weder Installer noch erhöhte Tasks aufgerufen. |

Testpopulation der sechs `server_v3/tests_*.rs`: **38 #[test]-Funktionen**, vollständig nach Namen erfasst: Rückstau 4, Lebenszyklus 7, Kopplung 7, Fristen 6, Abonnement 14, Hilfe 0. Relevante Gegenfälle wurden im Körper gelesen; dies ist kein Anspruch, alle 38 Tests ausgeführt oder jede Assertion geprüft zu haben.

## Abgleich mit dem Vorlauf

Vergleichsbasis: Lebenslaufaudit vom 12.09.2026, Commit `32528917`. Der lesende `git diff 32528917..93c7a3ce --stat -- <Phasenpfade>` und gezielte Quelldiffs bestätigen die hinzugekommenen Briefkästen, Broker-Stoppintegration, Publikationsfolgen und Queue-/noexcept-Nacharbeiten. Der breitere src-Diff umfasst auch Dateien außerhalb der gebundenen Population; diese wurden nicht allein wegen eines Diff-Treffers zum Prüfgegenstand gemacht.

| Vorlauf / Änderung | Stand an HEAD / neue Aussage |
| --- | --- |
| B-1/B-2, NAK-268 | Supervisor-/Store-Stopp bleibt bekannte Frist-/Messlücke. Der neue Briefkasten-Reihenfolgetest ist kein Test für Terminierung des echten Store-Writers. Kein neuer Tabellenbefund. |
| B-3, NAK-268 | Ausgang liest formal Inhalt vor Flag, aber schließen leert Inhalt unter derselben Sperre. Kein gefundenes Nach-Stopp-Liefern durch diese Queue; Spiegeltest bleibt bekannte Härtung. |
| B-4, NAK-268 | control_schliesst weiterhin synchron; Produktimplementierung räumt unter Coordinator-Sperre auf. Keine neue Behauptung, fremde Senken könnten hier beliebig blockieren. |
| B-5, NAK-268 | Hook-/Probe-Setter während Stopp und Start nach internem Stop weiterhin nicht als Produktbenutzung gefunden; keine erneute Meldung derselben API-Härtung. |
| B-6/B-7, NAK-268 | BrokerLifecycle-/PipeClient-Self-Stop weiterhin nicht im Produkt aufgerufen; der Sonde-Detach aus T3-04-01 ist ein anderer erreichbarer Besitzerfehler. |
| B-8, NAK-268 | Queue vorbereiten im Konstruktor; Neustart via Generation; InterventionsRing::zuruecksetzen ohne Produktaufrufer. Bekannter Strukturrest. |
| Abdeckungskarte K1 | P0/P1/Ingress/Writer getrennt verfolgt; Schlüssel und Ordinal bleiben bis Writer erhalten. A-1 (session_command_reihenfolge) liegt im hier nicht vollständig auditierten Coordinator-Befehlsweg. Bereits registrierter P0-Aufbau-Livelock NAK-270 und Hochwasserkartenrest NAK-274 nicht erneut gezählt. |
| Abdeckungskarte K2 | Client-Detach ist nur mit Besitzerregel sicher; diese gilt bei Gen, fehlt bei der Sonde. UI-Modal-Abbau wurde zusätzlich gegen JUCE-Eigentum nachverfolgt. |
| NAK-283 Etappen 2–6 | Quellen zeigen Folge+Generation am Mitglieder-Commit, Snapshot-Ersetzung ans Queue-Ende, zusätzliche Host-/Analyse-Grenzen; keine dieser Regeln schützt den Rücknahmeleser über projektReload. Kein Wiederaufmachen der 14 geschlossenen Gruppen. |
| NAK-286 | Neue eigene Timer-Kern-Lebensdauer im Plugin, Stop zuerst in beiden Prozessoren, FIFO-Kennungsring und Broker-Stoppweg geprüft; abgelehnte Message-Übergabe bewusst mit Halteplatz. Neuer Rest: laufender Broker-Dateitakt nach Join-Frist. |
| NAK-289 Etappe 1 | IpcQueues.h:419 und Vertrag.cpp:245 machen bisheriges terminate bei noexcept-Ausnahme ausdrücklich; kein neu eingeführter Queue-Abflussverlust. Die Ausnahmegrenze wurde nicht als neue Audio-Allokation umgedeutet. |

## Widerlegte Verdachte

1. **„Gen-Callbacks laufen nach freigegebenem Prozessor weiter.“** Für die zehn tatsächlich belegten v3-Pfade widerlegt: gemeinsame Schleuse, richtige Deklarationsreihenfolge vor Clients (`PluginProcessor.h:1924`), Schließen bei `PluginProcessor.cpp:474`; Test `Sonde012ProjectReloadTest.cpp:460`. Das widerlegt nicht T3-04-01 am SondeProcessor.
2. **„Ein abgelöster alter Client schließt beim Wiederanlauf die neue Pipe.“** Widerlegt durch eigene Verbindungsobjekte je Lauf (`ControlClient.cpp:84`, `Laufzeit.h:137`; `TelemetryClient.cpp:280`) und Lebenslauf-Generation. Kein geteiltes rohes Pipe-Objekt zwischen den Läufen.
3. **„P1-Retry bleibt ohne Reconnect liegen oder verliert beim Reservieren den Schlüssel.“** Widerlegt durch Bestätigung/Abfluss und Reservation/Rückgabe in `IpcQueues.h:414`; passende Gegenfälle `IpcTestMain.cpp:3804`, `:3831`, `:3850`. NAK-289 ändert die erfolgreiche Abflusssemantik nicht.
4. **„Ausgang liefert wegen Inhalt-vor-Close nach geschlossenem Ausgang weiter.“** Kein entsprechendes Fenster: `queues.rs:331` leert unter derselben Sperre, unter der `:305` entnimmt. Der fehlende Spiegeltest ist schon B-3/NAK-268, kein neuer Defekt.
5. **„Snapshots/Invalidierungen überschreiben einander im Writer oder ein älterer Stand überholt einen neueren.“** Für die untersuchte Queuekante widerlegt: getrennte Schlüssel, Hochwassermarke, Snapshot-Ersetzung ans Ende (`queues.rs:189`, `:251`); Gegenfälle `tests_abonnement.rs:873`, `:946`, `:1039`, `:1108` und `queues.rs:663`.
6. **„Welcome geht vor dem verbunden-Callback hinaus / Control wartet ewig auf abgelöste Telemetrie.“** Widerlegt an `verbindung.rs:337` und Trennmelderpfad; Tests `tests_kopplung.rs:112`, `:228`, `:448`. Die Zeitgrenzen wurden hier nicht neu gemessen.
7. **„Verspätete Control-Anmeldung hinterlässt zwingend einen permanenten Coordinator-Eintrag.“** Nicht bestätigt: der Fristpfad kann eine späte Anmeldung überleben, aber `coordinator/liveness.rs:211` bis `:273` behandelt veraltete Mitgliedschaft/Tombstone auch danach. Der hängende Callback ist als Sonderfall dokumentiert und `tests_lebenszyklus.rs:318` geprüft. Ein permanenter Leak folgt daraus nicht.
8. **„Der Plugin-Briefkasten zerstört den Timer-Träger vor einem bereits begonnenen Rückruf.“** Widerlegt durch Kernbesitz und kontrollierte letzte Freigabe (`Briefkasten.cpp:820`, `:845`, `:875`); Tests `BriefkastenTestMain.cpp:2778`, `:2898`, `:2957`. Der Halteplatz bei abgelehnter callAsync-Übergabe ist ausdrücklich gewollt, kein unentdeckter Leak.
9. **„Die beiden Editor-Timer brauchen allein wegen fehlenden expliziten stopTimer einen neuen Defekt.“** Kein solcher Defekt am regulären Message-Thread-Abbau: `juce_Timer.cpp:359` stoppt; gleichzeitiger Message-Thread-Rückruf ist dort ausgeschlossen. Prozessorzerstörung bei noch aktivem Editor verletzt bereits den JUCE-Hostvertrag (`juce_AudioProcessor.cpp:60`). T3-04-03 hält dagegen die gültige Editor-vor-Prozessor-Reihenfolge ein.
10. **„Ein voller P2-Sendestrom verhungert im Produkt seinen gleichzeitig benötigten Rückkanal.“** Der untersuchte Sendepfad liest tatsächlich beim leeren Sendepuffer (`TelemetryClient.cpp:742` ff.); Produktbelegung trennt aber Rollen: Sonde produziert P2 und hat keinen Frame-Callback (`SondeProcessor.cpp:141`), Gen konsumiert P2 über `PluginProcessor.cpp:202`. Kein erreichbarer bidirektionaler Produktfall gefunden; keine allgemeine Fairnessbehauptung über die API.

## Produktfragen

- **P04-1 / T3-04-04:** Soll eine vor Stopp begonnene Broker-Diagnoseantwort nach Ablauf der Join-Frist noch als fertige Datei erscheinen dürfen, oder muss der Stopp ihren späteren Abschluss unterdrücken? Die derzeitige Zusage entscheidet neu ausgelöste Takte nach Stopp; eine Regel für den schon laufenden Schreibweg fehlt. Diese Frage wird nur registriert, nicht an den User zur Beantwortung gestellt.

## Nicht geprüft

- Keine Builds, Tests, Stressläufe, Nulltests, Kanonläufe, Host-/FL-/EqCopShot-Ausführung oder Installation. Sämtliche erwähnten Tests wurden als Quell-Gegenbeleg gelesen; ältere PASS-Protokolle sind keine Messung dieses Laufs.
- Kein tatsächlicher UAF-Crash und keine Fristüberschreitung provoziert; die vier Befunde beruhen auf konkreten Quellabläufen. Scheduling-Häufigkeit und Hostreproduktion bleiben Nacharbeit.
- Keine vollständige fachliche Prüfung von DSP, Parsergrammatik, Stateformaten oder Sicherheitsauthentisierung; nur deren Lebensdauer-/Übergabekanten in der Population. Schema-/Zahlenbefunde aus Phase 03 nicht dupliziert.
- Kein erneuter Store-Worker-Audit, keine fremden Threadmodelle in UI-Bibliotheken, kein Prozessende während OS-Shutdown. JUCE wurde nur für konkret benötigte Besitzer-/Timer-/VST3-Kanten gelesen.
- Keine Volllektüre sämtlicher Testkörper, Manifestdateien oder Registerzeilen. Suchbegriffe, abgedeckte Fälle und vorhandene Gegenbelege wurden gezielt geprüft.
- Kein Design, keine optische Bewertung, keine Studien, Legacy-Umbenennungen oder Sabotage von Prüfskripten; keine Bewertung von Prosa/Verweiszahlen als Produktbefund.

## Scope-Beweis

### Gebundene Population: 64/64 vorhandene Pfade erfasst und nach Phasenmustern gelesen

„Gelesen“ bedeutet hier vollständige Dateierfassung und pfadgebundene Lebenslaufsuche sowie Quelllektüre der einschlägigen Funktionen/Übergaben; nicht Vollprüfung jeder Zeile jedes Fachalgorithmus. Die letzte Zahl ist die frisch gelesene Dateizeilenzahl zur Identifikation, kein Test- oder Qualitätsmaß.

```text
broker/src/briefkasten.rs 1995
broker/src/lebenslauf.rs 418
broker/src/server.rs 1841
broker/src/transport/server_v3/auth.rs 189
broker/src/transport/server_v3/griff.rs 495
broker/src/transport/server_v3/listener.rs 703
broker/src/transport/server_v3/mod.rs 231
broker/src/transport/server_v3/queues.rs 831
broker/src/transport/server_v3/senke.rs 140
broker/src/transport/server_v3/tests_abonnement.rs 1567
broker/src/transport/server_v3/tests_fristen.rs 179
broker/src/transport/server_v3/tests_hilfe.rs 454
broker/src/transport/server_v3/tests_kopplung.rs 498
broker/src/transport/server_v3/tests_lebenszyklus.rs 422
broker/src/transport/server_v3/tests_rueckstau.rs 240
broker/src/transport/server_v3/trennung.rs 192
broker/src/transport/server_v3/verbindung.rs 890
broker/src/transport/server_v3/win_handles.rs 718
eq-copilot/plugin/core/diagnose/Briefkasten.cpp 1071
eq-copilot/plugin/core/diagnose/Briefkasten.h 349
eq-copilot/plugin/core/ipc/BrokerInstallBinding.h.in 11
eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp 951
eq-copilot/plugin/core/ipc/BrokerLifecycle.h 155
eq-copilot/plugin/core/ipc/ControlClient.cpp 247
eq-copilot/plugin/core/ipc/ControlClient.h 744
eq-copilot/plugin/core/ipc/controlclient/Intern.h 467
eq-copilot/plugin/core/ipc/controlclient/Laufzeit.h 227
eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp 479
eq-copilot/plugin/core/ipc/controlclient/Schleuse.h 204
eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp 1080
eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp 325
eq-copilot/plugin/core/ipc/InterventionsRing.h 232
eq-copilot/plugin/core/ipc/IpcQueues.h 919
eq-copilot/plugin/core/ipc/IpcVerbindung.cpp 587
eq-copilot/plugin/core/ipc/IpcVerbindung.h 282
eq-copilot/plugin/core/ipc/PipeToken.cpp 184
eq-copilot/plugin/core/ipc/PipeToken.h 88
eq-copilot/plugin/core/ipc/TelemetryClient.cpp 812
eq-copilot/plugin/core/ipc/TelemetryClient.h 161
eq-copilot/plugin/core/ipc/WireEnvelope.cpp 303
eq-copilot/plugin/core/ipc/WireEnvelope.h 252
eq-copilot/plugin/core/ipc/WireZahl.h 228
eq-copilot/plugin/core/StampedAudioQueue.h 862
eq-copilot/plugin/sonde/SondeFactory.cpp 11
eq-copilot/plugin/sonde/SondeProcessor.cpp 1446
eq-copilot/plugin/sonde/SondeProcessor.h 562
eq-copilot/plugin/spike/AuxSpikeEditor.cpp 179
eq-copilot/plugin/spike/AuxSpikeEditor.h 36
eq-copilot/plugin/spike/AuxSpikeFactory.cpp 8
eq-copilot/plugin/spike/AuxSpikeProcessor.cpp 313
eq-copilot/plugin/spike/AuxSpikeProcessor.h 146
eq-copilot/plugin/src/PipeClient.cpp 824
eq-copilot/plugin/src/PipeClient.h 174
eq-copilot/plugin/src/PluginEditor.cpp 1665
eq-copilot/plugin/src/PluginEditor.h 183
eq-copilot/plugin/src/PluginProcessor.cpp 1154
eq-copilot/plugin/src/PluginProcessor.h 1932
eq-copilot/plugin/src/prozessor/Analyse.cpp 1438
eq-copilot/plugin/src/prozessor/Hostbruecke.cpp 187
eq-copilot/plugin/src/prozessor/Intern.h 49
eq-copilot/plugin/src/prozessor/Ipc.cpp 1663
eq-copilot/plugin/src/prozessor/State.cpp 697
eq-copilot/plugin/src/SourcesModel.cpp 1947
eq-copilot/plugin/src/SourcesModel.h 458
```

Gruppensummen: **core/ipc 22**, **core/diagnose 2**, **StampedAudioQueue 1**, **src/prozessor 5**, **acht src-Dateien 8**, **sonde 3**, **spike 5**, **broker server_v3 15**, **drei Broker-Wurzeldateien 3** = **64**. Keine Datei dieser gebundenen Population fehlt.

### Ergänzende Quellpfade

**12 zusätzliche Dateien**, jeweils nur die für die Kante nötigen Ausschnitte oder Symbolstellen:

- `broker/src/transport/warteschlange.rs`: Ingress-Speicher, Limits, Entnahme; zusammengehörige Queue-Typen mitgelesen.
- `broker/src/coordinator/senke.rs`, `link.rs`, `liveness.rs`: Anmeldung, Trennung, späte Anmeldung und Bereinigung.
- `eq-copilot/plugin/state/NakamaParameter.h`: ausschließlich die statische Anzahl der Hostparameter.
- Lokales JUCE unter `eq-copilot/build/_deps/juce-src/modules/`: `juce_gui_basics/windows/juce_CallOutBox.cpp`, `juce_gui_basics/components/juce_ModalComponentManager.cpp`, `juce_gui_basics/components/juce_Component.cpp`, `juce_audio_processors/processors/juce_AudioProcessor.cpp`, `juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp`, `juce_events/timers/juce_Timer.h`, `juce_events/timers/juce_Timer.cpp`.

**Fünf zusätzliche Testdateien gezielt gelesen/gesucht:** `eq-copilot/plugin/tests/IpcTestMain.cpp`, `PipeClientLifecycleTestMain.cpp`, `Sonde012ProjectReloadTest.cpp`, `Sonde012SourcesModelTest.cpp`, `BriefkastenTestMain.cpp`. Weitere repositoryweite Testsuchen dienten nur dem Auffinden von Gegenbelegen und sind kein Vollaudit jener Testpfade. Die sechs Broker-Testmodule stehen schon in den 64 Dateien, eingebettete Broker-Tests sind dort ebenfalls enthalten.

### Kontextpfade, keine Befundflächen

Gelesen: `CLAUDE.md` vollständig, `AGENTS.md`, `docs/plugin-wissen.md` in den Phasenabschnitten, `docs/gesundheit/KONZEPT.md` §3/§4.3, `docs/gesundheit/abdeckungskarte.md` K1/K2/A-Fälle, `docs/gesundheit/lebenslauf-audit.md`, `docs/offene-punkte.md` per gezieltem Registerabgleich, relevante Abschnitte von `docs/FL-Nakama-Sonden-Design-Entwurf.md`, `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md`, `docs/beweise/NAK-246.md`, `docs/beweise/NAK-286.md` und bisheriges `BEFUNDE.md` dieses Audits. Dazu die zwei genannten SKILL.md, das Quality-Referenzblatt und das vorhandene Startlog. Keine dieser Dateien wurde als Befundfläche verwendet.

### Nicht gelesene und fehlende Pfade

Nicht vollständig gelesen: reine DSP-/Analyse-/Zeichenkörper innerhalb großer Populationsdateien; die nicht einschlägigen Körper der 38 Broker-Tests; der übrige `broker/src/coordinator/**`-, `broker/src/store/**`-, Plugin-DSP-/State-/Schema-/Installationsbaum; übriges JUCE und historische Doku. Nicht gefundene Suchhypothesen `plugin/tests/BriefkastenTest.cpp` und `plugin/parameter/` wurden auf die vorhandenen `BriefkastenTestMain.cpp` bzw. `state/NakamaParameter.h` aufgelöst; **kein fehlender Pflichtpfad**.

### Abschlusskontrolle

HEAD vor und nach dem Lauf: **93c7a3cefa0568c311ea2652cb622f70f40aedb4**. Die Statusprüfung zeigt ausschließlich die zwei autorisierten Auditdateien als eigene Änderungen; die vier vorbestehenden untracked Einträge bleiben bestehen. Keine Staging-, Commit- oder Push-Aktion.

Der bisherige Inhalt von BEFUNDE.md ist bytegleich erhalten: die ersten **35 749 Bytes** haben weiterhin SHA-256 **98B660505DA1021C93CCF40CF9EED97EEBAAC9DC429A82CFD6519EFEB31550CC**. Git-Diff für diese Datei: **13 hinzugefügte, 0 entfernte Zeilen**. Der Phase-04-Abschnitt existiert genau einmal. Die sechs Zeilen der Befundtabelle (Kopf, Trenner, vier Befunde) stimmen zwischen Bericht und Anhang wortgleich überein; 64 Pfade stehen im Scope-Inventar. Dies sind Dateiintegritätsprüfungen dieses Laufs, keine ausgeführten Produkttests.
