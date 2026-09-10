# Audit-Paket State, C++-IPC und Zustandswahrheit

Stand: 10.09.2026. Gepruefter Snapshot: `844b9c15935377e89d124ac33848e9cba1ecf4c2`, Arbeitskopie `C:/na-audit-20260910-844b9c1`.

Rein statischer Audit. Kein Produktcode, kein Test, keine Fixture und kein Git-Stand wurde geaendert. Kein Build, keine Pipe-Verbindung, kein Lasttest und kein Hostlauf wurde ausgefuehrt. Der einzige Schreibpfad dieses Pakets ist diese Datei. Der aktive Fable-Checkout wurde nicht benutzt.

**Evidenzstatus:** Die folgenden fuenf Defekte sind anhand der aktuellen Aufrufketten nachvollzogen und vom Root unabhaengig gegengelesen. Ein aktueller Laufbeweis fuer die beschriebenen Ausloeser ist jeweils **NOT RUN**. Ein vorhandener Test ist kein Laufbeleg. Die Prioritaeten sind Auditvorschlaege, kein Releaseurteil.

## SIPC-01 — P1: Timeout-detach schuetzt den Client, aber nicht den Prozessor seiner Callbacks

**Primaerstelle:** [ControlClient.cpp:119](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/ipc/ControlClient.cpp#L119), insbesondere `thread.detach()` in Zeile 133. Derselbe Weg steht in [TelemetryClient.cpp:314](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/ipc/TelemetryClient.cpp#L314), detach in Zeile 328.

Der detached Thread haelt `ControlClient::Laufzeit` ueber `shared_ptr` am Leben. Darin gespeicherte Produkt-Callbacks halten jedoch nur einen rohen Zeiger auf den Prozessor: [PluginProcessor.cpp:112](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginProcessor.cpp#L112) bis 121 und [SondeProcessor.cpp:78](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/sonde/SondeProcessor.cpp#L78) bis 81. Der Prozessor wird durch den Laufzeitzeiger nicht gehalten.

**Ausloesekette:** Ein Hello-, Status-, Antwort- oder Frame-Callback laeuft bereits und bleibt laenger als die Stoppfrist stehen, etwa an einer Sperre oder durch Thread-Scheduling. Der Host entfernt die Instanz. `EqCopilotProcessor::~EqCopilotProcessor` stoppt die Clients und laesst danach den Prozessor zerstoeren ([PluginProcessor.cpp:324](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginProcessor.cpp#L324)); Sonde macht dasselbe nach seinem Worker-Join ([SondeProcessor.cpp:112](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/sonde/SondeProcessor.cpp#L112)). Nach Ablauf der Frist kehrt `stop()` trotz laufendem Callback zurueck. Wird der Callback danach fortgesetzt, benutzt sein `[this]` bereits freigegebenen Prozessorzustand. Auch dessen Mutexe und Client-Member koennen bereits zerstoert sein.

**Schaden:** Use-after-free im Hostprozess; moeglicher DAW-Absturz beim Entfernen oder Schliessen eines Projekts. Wie oft ein realer Produkt-Callback die Frist erreicht, wurde nicht gemessen. Der Defekt liegt im abgedeckten Timeout-Fall, nicht in der Behauptung, dass jeder normale Shutdown abstuerzt.

**Gegenargument geprueft:** Die Generationspruefungen stoppen Folgearbeit des Clientthreads, koennen einen bereits laufenden fremden Callback aber nicht rueckwirkend beenden. `shared_ptr<Laufzeit>` schuetzt nur die im Client liegenden Daten. Die normale Stoppstrecke joint korrekt; sie beseitigt den Timeout-Fall nicht.

**Testluecke:** [IpcTestMain.cpp:4390](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/IpcTestMain.cpp#L4390) bis 4441 testet explizit einen blockierten Callback nach Zerstoerung des Clients. Sein Callback captured `shared_ptr<atomic<bool>>`; genau diese sichere Ownership unterscheidet ihn vom Produkt. Er prueft weder Prozessorlebensdauer noch das Weiterlaufen eines `[this]`-Callbacks nach Besitzerfreigabe.

**Beweisidee / Behebung:** Einen echten Produkt-Callback ueber eine kontrollierte Schranke festhalten, Besitzer auf dem legitimen Hostpfad abbauen, nach der Frist Callback freigeben und mit AddressSanitizer/PageHeap den Besitzerzugriff pruefen. Laufzeitdaten und Callbacks muessen dieselbe gesicherte Lebensdauer haben; ein nackter Weak-Pointer-Check vor einem laengeren Callback reicht ebenfalls nicht. Die Alternative ist ein Shutdown, der Eigentuemerdaten erst nach sicherem Abschluss aller auf sie zugreifenden Callbacks freigibt.

## SIPC-02 — P1: Ein bestaetigter Join/Unbind wird nur bei offenem Editor gespeichert

**Primaerstelle:** [Ipc.cpp:1063](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Ipc.cpp#L1063) bis 1074 legt erfolgreiche ACKs nur in `bestaetigteSourcesCommands` ab. Die eigentliche State-Aenderung und `meldeHostDirty()` stehen in [Ipc.cpp:1327](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Ipc.cpp#L1327) bis 1368.

Die Suche nach `sourcesTick(` liefert genau einen Produktionsaufrufer: [PluginEditor.cpp:242](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/PluginEditor.cpp#L242) bis 257, nur bei aktiver Main-Flaeche. Der 30-Hz-Timer gehoert dem Editor (Zeile 230). `sourcesTick()` ruft die Uebernahme auf ([Ipc.cpp:1108](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Ipc.cpp#L1108)).

**Ausloesekette:** Der User bestaetigt eine Quelle oder entfernt deren Bindung und schliesst das Pluginfenster vor Eingang des ACKs oder vor dem naechsten Timerlauf. Das ACK trifft im weiterhin laufenden Prozessor ein. Danach wird das Projekt mit geschlossenem Editor gespeichert. [State.cpp:63](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/State.cpp#L63) bis 66 serialisiert nur `zustand`; der Save uebernimmt die bestaetigte Warteliste nicht. Ein Reload loescht sie in [State.cpp:79](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/State.cpp#L79) bis 81.

**Schaden:** Ein bereits Broker-seitig bestaetigter User-Handgriff fehlt in `MainProject.confirmed_members_v1` und meldet dem Host kein Dirty. Der Join geht im Projekt verloren oder die entfernte Quelle kommt beim Reload wieder als bestaetigt zurueck. Die zeitliche Luecke kann bei geschlossenem Editor unbegrenzt dauern.

**Gegenargument geprueft:** Dass der normale Editor alle 33 ms tickt, macht den Besitz der Persistenz nicht korrekt; der User darf das Fenster schliessen. Ein ACK vor dem Save reicht nicht, solange es nur in einer fluechtigen Liste liegt. Kein Worker-, AsyncUpdater-, Save- oder Editor-Destruktorpfad drainiert diese Liste.

**Testluecke:** [Sonde012ProjectReloadTest.cpp:293](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp#L293) bis 320 ruft nach jedem eingespeisten ACK ausdruecklich `sourcesTick()` auf, bevor State oder Dirty geprueft werden. Der Test umgeht damit genau den fehlenden Besitz ausserhalb des Editors.

**Beweisidee / Behebung:** Den vorhandenen Produkt-Test mit erfolgreichem ACK, aber ohne `sourcesTick()`, direkt speichern und neu laden lassen. Ein offener Editor darf fuer die Persistenz bestaetigter Befehle keine Voraussetzung sein. Die Uebernahme muss auf einem Prozessorpfad mit sicherem Thread- und Host-Dirty-Vertrag stattfinden.

## SIPC-03 — P2: P0-Ueberlauf verwirft die ACK-Zuordnung, obwohl der Auftrag spaeter ausgefuehrt wird

**Primaerstelle:** [Ipc.cpp:1320](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/Ipc.cpp#L1320) bis 1324 loescht bei `sendePersistenzP0(...) == false` den Eintrag aus `ausstehendeSourcesCommands`.

Die Rueckgabe bedeutet jedoch nicht durchgehend, dass der logische Auftrag abgelehnt wurde: [Nachrichten.cpp:184](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp#L184) legt ihn zuerst in `inFlight` an; bei voller P0-Queue wird er in Zeile 206 nur als nicht eingereiht markiert, behalten und in Zeile 226 mit `false` gemeldet. [Nachrichten.cpp:231](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp#L231) bis 245 reiht ihn spaeter erneut ein. Der Sendezug ruft diesen Wiederholweg auch nach erfolgreichem Wire-Write erneut auf ([Verbindung.cpp:1023](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp#L1023)).

**Ausloesekette:** Bei gefuellter P0-Queue ruft Gen `confirm_join` oder `unbind_probe`. Der Transport nimmt den logischen Auftrag in seine Wiederholung auf, liefert `false`, und der Prozessor entfernt die einzige Zuordnung zum geplanten Mitgliedschaftswechsel. Nach Entleerung/Neuaufbau wird derselbe Auftrag trotzdem Broker-seitig angewandt. Das erfolgreiche ACK findet in `v3Antwort()` keinen Pending-Eintrag mehr; State-Uebernahme und Host-Dirty entfallen auch bei offenem Editor.

**Schaden:** Brokerzustand und gespeicherte Main-Mitgliedschaft widersprechen sich. Ein als abgewiesen gemeldeter Handgriff kann spaeter trotzdem ausgefuehrt werden; der zugehoerige persistente Rueckweg ist verloren. Erneutes Klicken kann eine weitere Command-ID erzeugen, statt denselben offenen Vorgang weiterzufuehren.

**Gegenargument geprueft:** Retention ist absichtliche Transportsemantik, keine unklare Annahme. [IpcTestMain.cpp:5937](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/IpcTestMain.cpp#L5937) bis 5958 verlangt ausdruecklich `false`, gehaltenes `inFlight` und spaetere erfolgreiche Anwendung. Die Verwurfmeldung enthaelt eine Transportmarke; der registrierte Prozessor-Hook ersetzt die geloeschte Source-Command-Zuordnung nicht.

**Testluecke:** Der Transport-Replay-Test und der Source-Mitgliedschaftstest pruefen jeweils ihre Haelfte; ein voller Transport mit realem Source-Command plus spaetem ACK fehlt. [Sonde014AssistentTest.cpp:921](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/Sonde014AssistentTest.cpp#L921) bis 953 bestaetigt separat die `false`-Rueckmeldung und endet vor Replay; das beweist keine endgueltige Ablehnung.

**Beweisidee / Behebung:** Produkt-Source-Command bei voller P0-Queue absenden, Wiederholung und finales ACK zulassen, dann Main-State und Dirty pruefen. Die API sollte endgueltige Ablehnung von angenommenem, noch nicht eingereihtem Auftrag unterscheiden; die Source-Zuordnung muss bis zum tatsaechlichen Abschluss leben.

## SIPC-04 — P2: Projekt-Reload behaelt Experiment-, Paar- und Befunddaten der alten Sitzung

**Primaerstelle:** [SourcesModel.cpp:401](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/SourcesModel.cpp#L401) bis 436 setzt Quellen, Bindung, Epoche und Subscription zurueck, leert aber weder `experimente`, `paare`, `befunde` noch `evidenzRuecknahmen`, `ruecknahmeGrund` und `ruecknahmeUmfang`.

**Ausloesekette:** Gen hat einen vollstaendigen Snapshot mit Experimenten/Paarurteilen/Befunden oder hat eine Evidenzruecknahme empfangen. `setStateInformation` laedt in derselben Instanz ein anderes Projekt oder einen read-only/ungebundenen State und ruft `projektReload` ([State.cpp:111](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/prozessor/State.cpp#L111), Zeile 126). Die anschliessende Sicht kopiert die alten Listen und Ruecknahmeangaben unveraendert ([SourcesModel.cpp:1560](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/src/SourcesModel.cpp#L1560) bis 1565). Ohne gueltigen neuen Control-Link wird `beginneSubscription`, das diese Felder tatsaechlich leert (Zeilen 450 bis 466), nie erreicht.

**Schaden:** Die Sitzungssicht zeigt fremde oder ueberholte Experimente und Paarurteile; alte Befunde/Ruecknahmegruende bleiben im neuen Projektmodell. Bei einem ungebundenen oder nicht lesbaren State kann das dauerhaft sein. Es wird kein bereits implementierter DSP-Apply behauptet; das ist ein Defekt der heutigen Zustandswahrheit.

**Gegenargument geprueft:** `controlEnde()` entwertet Befunde zu `stale`, loescht aber die Listen und Experiment-/Paarergebnisse nicht. `mainDarfSchreiben` wird durch den Reload false, was Schreibhandlungen begrenzt, aber eine falsche Sitzungssicht nicht heilt. Ein erfolgreicher spaeterer Subscribe heilt den Zustand; er ist keine Garantie bei ungebundenem/read-only State.

**Testluecke:** [Sonde012ProjectReloadTest.cpp:323](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp#L323) bis 331 prueft Quellen/Runtime-Nonce/LUFS, nicht die spaeter ergaenzten Sitzungsliste-Felder. Die Scope-Kommentare im Modell verlangen ausdruecklich, dass diese Listen nur der jeweiligen Sitzung gehoeren.

**Beweisidee / Behebung:** Ein SourcesModel ueber reale Snapshot-/Invalidierungsleser befuellen, `projektReload` aufrufen, danach alle fluechtigen Sichtfelder pruefen. Die gemeinsame Reload-Grenze muss den vollstaendigen transienten Bestand zuruecksetzen; ein spaeterer Connect darf dafuer nicht notwendig sein.

## SIPC-05 — P2: Zonen-ID wird vor der Bereichspruefung von double nach int konvertiert

**Primaerstellen:** [NakamaParameter.cpp:390](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/state/NakamaParameter.cpp#L390) bis 397 sowie [NakamaPreset.cpp:243](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/state/NakamaPreset.cpp#L243) bis 248.

Beide Reader pruefen nur Endlichkeit und Ganzzahligkeit, bevor `(int) id->zahl` ausgefuehrt wird. Die fachliche Pruefung `0 <= id < 8` erfolgt erst am bereits konvertierten Wert in [NakamaParameter.cpp:219](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/state/NakamaParameter.cpp#L219), gerufen nach `leseZonen` beziehungsweise nach Aufbau der Presetzonenliste.

**Ausloesekette:** Ein ansonsten gueltiges DTO/Preset traegt beispielsweise `"id":2147483648` in einer Zone. Der Textriegel akzeptiert diese endliche, exakt darstellbare JSON-Ganzzahl: seine Ganzzahlgrenze ist 2^53-1 ([NakamaVertrag.cpp:324](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/vertrag/NakamaVertrag.cpp#L324)). Der `double` passt nicht in den 32-Bit-`int`, wird aber vor dessen Fachpruefung konvertiert.

**Schaden:** Undefiniertes C++-Verhalten am externen Datenleser statt garantierter fail-closed-Abweisung. Fuer den konkreten aktuellen MSVC-Build ist kein Crash oder falsches Akzeptieren nachgewiesen; eine typische Konvertierung zu INT_MIN mit anschliessendem Reject ist kein Sprach-/Optimierungsbeweis fuer den Reader.

**Gegenargument geprueft:** Der Safe-Integer-Riegel prueft Praezision in binary64, nicht Darstellbarkeit in `int`. Die spaetere Validierung kommt fuer einen ungueltigen Cast zu spaet. Der Rust-Gegenleser wurde vom Broker-Agent geprueft: `broker/src/dto.rs:461` bis 474 nutzt den breiteren i64-Weg und verwirft den Beispielwert danach definiert; ein entsprechendes Rust-UB wurde nicht gefunden.

**Testluecke:** StateMigrationTest prueft fachliche ID-Grenzen wie 8 und typisierte Zonen, aber kein Raw-JSON mit ID an INT_MAX+1/INT_MIN-1 oder der Safe-Integer-Grenze an diesen beiden Casts. Die zwei duplizierten Zonenreader tragen denselben Fehler.

**Beweisidee / Behebung:** Beide realen Reader mit `2147483648`, `-2147483649` und `9007199254740991` bei sonst gueltiger Nutzlast unter `float-cast-overflow`-Sanitizer pruefen. Die erlaubte ID-Menge am double pruefen, bevor nach int konvertiert wird, und den gemeinsamen Parserweg moeglichst nur einmal halten.

## Weitere Kandidaten und Ressourcenrisiken

**K-SIPC-01 — gezielt nachzumessen: Writer-Headroom prueft Bytes, nicht alle Readergrenzen.**

[NakamaState.cpp:1094](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/state/NakamaState.cpp#L1094) baut einen groessten Folgezustand, doch `passt` vergleicht in Zeile 1122 nur die Bytelaenge mit 16 MiB. Der vorgeschaltete ByteRiegel begrenzt dagegen auch Sammlungseintraege auf 65.536 und die Summe auf 262.144 (Zeilen 84 bis 85 und 180 bis 190). Ein gueltiger Common-Knoten mit genau 65.536 Properties, aber ohne optionales `label`, kann beim Laden schreibbar sein; der Save fuegt `label` in `synchronisiert` Zeile 820 hinzu und erzeugt 65.537 Properties. Der eigene Reader duerfte diesen Save als `ignoriert` verwerfen, obwohl er weit unter 16 MiB liegt. Ebenso ist globale Eintragsreserve nicht im Headroombeweis enthalten. Keine aktuelle Reproduktion; daher als Kandidat statt als sechster bestaetigter Defekt. Der existierende Headroomtest in StateMigrationTestMain.cpp:1980 bis 2056 variiert die Bytelaenge grosser Blob-Properties, nicht die Anzahl. Beweis: Raw-ValueTree an der Propertygrenze bauen, laden, ohne Eingriff speichern, erneut laden und den Ergebnisstatus vergleichen.

**R-SIPC-01 — unbeschraenkter logischer In-Flight-Bestand.**

`sendePersistenzP0` legt jede neue gueltige Command-ID in einen `std::vector` ab ([Nachrichten.cpp:175](https://github.com/evenacadia-tech/Nakama/blob/844b9c15935377e89d124ac33848e9cba1ecf4c2/eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp#L175) bis 184); geloescht wird beim finalen ACK, nicht nach Zeit oder Anzahl. Die feste P0-Queuecap begrenzt diesen Bestand ausdruecklich nicht. Der Test IpcTestMain.cpp:6241 bis 6323 verlangt bereits 65 zugleich offene Auftraege als zulaessig. Ohne ACKs kann der Bestand mit jedem neuen Userauftrag wachsen; jede ID-Suche und mehrere Sendebuchungen laufen linear darueber. Kein gemessener Leak oder Budgetbruch; hier fehlt eine Ressourcenpolitik fuer lange Verbindungsstoerungen, etwa begrenzte Annahme neuer Auftraege bei weiter verlustfreier Aufbewahrung bereits angenommener. Ein willkuerliches Verwerfen des aeltesten Auftrags waere kein korrekter Fix.

## Intermodulare Bestaetigungen fuer das Broker-Paket

Diese Punkte sollen im Gesamtbericht bei den jeweiligen Broker-Ursachen stehen und nicht doppelt als C++-Defekte gezaehlt werden.

- **Snapshot-Reihenfolge:** SourcesModel.cpp:575 bis 597 hat kein Snapshot-Sequenzfeld im geschlossenen Vertrag. Zeilen 1152 bis 1163 pruefen nur Bindung/Session und ersetzen Epoche/Empfangszeit; Zeilen 1252 bis 1255 ersetzen alle Listen. `finding.intent_revision` wird in Zeilen 1053 bis 1060 kopiert, nicht monoton verglichen. Ein Broker-seitig spaeter zugestellter aelterer Snapshot derselben Sitzung wird voll angenommen und kann entwertete Befunde wieder als `ready_to_send` anzeigen. Dies bestaetigt die vom Broker-Agent untersuchte Flush-/Enqueue-Umordnung.
- **Invalidierung ist kein ersetzbarer Vollsnapshot:** `uebernehmeEvidenzruecknahme` invalidiert die Messachse in SourcesModel.cpp:1421 bis 1424 und alle Befunde in Zeile 1435. Ein nachfolgender SessionSnapshot uebernimmt bei gleicher Epoch/Nonce alte Messdaten in Zeilen 1171 bis 1174 und ersetzt diese Wirkung nicht. Die vom Broker-Agent gefundene Koaleszierung unterschiedlicher Familien unter demselben Snapshot-Key kann daher eine heutige Wirkung verlieren.
- **Begrenzung beim Invalidierungs-Outbox-Fund:** Heutiges Gen kennt keine Evidenz-ID-Zuordnung und invalidiert bei jeder Ruecknahme konservativ global (SourcesModel.cpp:1411 bis 1424). Werden zwei disjunkte Ruecknahmen zu einer zusammengefasst, reicht die letzte fuer diesen aktuellen Gen-Messzustand. Ein Verlust disjunkter Scopes ist damit nicht ohne Weiteres ein zweiter heutiger Gen-Schaden; der Vertrags-/Outbox-Punkt ist getrennt zu beurteilen.

## Pruefumfang und ehrliche Grenzen

Vollstaendig gelesen: Repository-AGENTS.md, CLAUDE.md, Skills `nakama-plugin-quality` und `sondenplan-audit`; `src/prozessor/State.cpp`; `state/NakamaPreset.cpp`, `state/NakamaKanon.cpp`, `state/NakamaLebenslauf.cpp`; Implementierung von `core/ipc/IpcVerbindung.cpp` und `core/ipc/WireEnvelope.cpp`; C++-FlatBuffers-Pruef-/Lesepfad in `vertrag/NakamaTelemetrie.cpp`. Kommentare wurden bei einigen langen Lesebloecken ausgeblendet, Code dabei mit Zeilennummern gelesen.

Tief gelesen, mit folgenden Grenzen:

- `state/NakamaState.cpp`: ByteRiegel, Matrizen/Versionen/Common, DSP-/Undo-Kind, Save/Load, Writer-Headroom und Migrationspfad; etwa Zeilen 65 bis 344, 683 bis 837, 983 bis 1590, 1911 bis 2217. Die mittleren Intent-/Assistentenlisten und reine Intent-Mutatoren nicht vollstaendig zeilenweise auditiert.
- `state/NakamaParameter.cpp`: Default-/Gleichheits-/Validierungs-, DTO-, Zonen- und ValueTree-Reader/Writerpfad, Zeilen 96 bis 336 und 338 bis 689. Parametertabellenaufbau davor nur als Vertragseinstieg gescannt.
- `core/ipc/ControlClient.cpp`, `controlclient/Laufzeit.h` und `Nachrichten.cpp`: Laufzeit-/Ownership-Aufbau, Stop/Start/Reconnect, P0/P1-Annahme und InFlight-/ACK-Wiederholung; Control-Header und Verbindung.cpp mit Fokus auf Callbackgrenzen/Empfang und Sendezug. Nicht jede Testhilfsmethode erneut semantisch auditiert.
- `core/ipc/TelemetryClient.cpp`: Stop/Start, Hello/Kopplung, Empfangs-Callback und Versandloop; P2-Schleusenalgorithmus in diesem Paket nicht erneut zeilenweise geprueft, DSP-Agent deckt Audiouebergabe ab.
- `core/ipc/BrokerLifecycle.cpp`: Hash/Authenticode, Server-PID/SID-/Dateiidentitaetspruefung, Spawn und Lifecycle-Tick/Stop gelesen. Keine Win32-Systembeweise ausgefuehrt.
- `src/prozessor/Ipc.cpp`: Hello/Status, Link-Grenze, ACK/Frame, Sources-Befehle und Persistenz; Interventionssendepfad nur fuer Ownership/Sperrordnung quergelesen. Nicht der vollstaendige Experiment-/Intent-Wirepfad.
- `src/SourcesModel.cpp`: Subscription/Control/Reload, Snapshot-Commit, Invalidierung, P2-Commit und Sicht-/Frischeableitung tief gelesen; lange Experiment-/Finding-Feldvalidierung nur ausschnittsweise gegen geschlossene Mengen geprueft.
- `src/PluginProcessor.cpp`, `src/PluginEditor.cpp`: Callback-Ownership, Konstruktor/Destruktor und Timer-Aufrufbeziehungen; kein visueller UX-Audit. `sonde/SondeProcessor.cpp`: Konstruktor/Destruktor und State-/V3-Status-Zusammenhang; Audiokern beim DSP-Agent.

Tests gezielt zur Gegenpruefung gelesen: relevante Abschnitte aus `IpcTestMain.cpp` (Stop/Detach, Persistenz-Replay, Ueberlauf, ACKs und >64 InFlight), `Sonde012ProjectReloadTest.cpp`, `Sonde014AssistentTest.cpp`, `StateMigrationTestMain.cpp`. `Sonde012SourcesModelTest.cpp` und `Sonde014BefundTest.cpp` auf Reload-/Monotonieabdeckung gescannt. Kein Testlauf in diesem Paket.

Schemas/Fixtures: State-v2-Vertrag, v3-README/FlatBuffers-README und betroffene Gegenvertraege als Router/Anforderungsquelle gelesen; nicht jede JSON-/Binary-Fixture erneut byteweise geprueft. `NakamaVertrag.cpp` mit Fokus auf Byte-/Zahlenriegel gelesen, nicht der gesamte allgemeine Schema-Interpreter. `NakamaEvidenz.*`, `PipeToken.*` und generierter FlatBuffers-Header nur im Quellinventar/bei Aufrufbeziehungen gescannt.

Keine Behauptung einer vollstaendigen A-bis-Z-Freigabe. Keine Befunde allein aus geplanter, noch nicht abgeschlossener SONDE-015-Produktverdrahtung. Die gefundenen Integrationsdefekte betreffen heute vorhandene Lebenszyklen und Reader; ihre Behebung ist nicht Teil dieses rein lesenden Auftrags.
