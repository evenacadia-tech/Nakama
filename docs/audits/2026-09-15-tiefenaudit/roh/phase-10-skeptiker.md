URTEIL: BESTÄTIGT 56 · HERABGESTUFT 3 · WIDERLEGT 0 · DUPLIKAT 0 · UNENTSCHIEDEN 0

# Phase 10 — Skeptiker

Phase 10 · Auftrag 2026-09-16 03:27 MESZ · Branch master  
HEAD vor dem Lauf und bei der Abschlusskontrolle: `2c3a4b0a806d74aeb22b7f01ac2927266bf38c2f`.  
Modell **gpt-6-astra**, Effort **max**, laut Startlog; Start **2026-09-16 03:27:15,632 MESZ**, Berichtstand **2026-09-16 04:01:21 MESZ**, Dauer bis Berichtstand **34 Minuten 5 Sekunden**.  
Prüferrolle: Quellenlesung; kein Bau, Testlauf, Kanon, Hostlauf, Pipe-Start, Installation oder Git-Schreibbefehl.

Die Quellen tragen die Mechanismen aller 59 Befundzeilen; drei ursprüngliche Einordnungen verlangen eine engere Aussage. Das ist kein Freigabeurteil über die Codebase und kein Laufzeitbeweis.

## Population und Wertungsregel

Die vollständige Population umfasst **59 eindeutige Befund-IDs** aus BEFUNDE.md und den zehn zugehörigen Rohberichten. Je ID wurde der betroffene Produkt- oder Werkzeugpfad neu gelesen, anschließend nach entgegenstehenden Tests, Riegeln, Absichtskommentaren, Registereinträgen und Zusagen gesucht. Suchtreffer und frühere grüne Läufe wurden nicht als neuer Ausführungsbeleg gewertet.

| Herkunft | Befundzeilen | Hier entschieden |
| --- | ---: | ---: |
| Phase 01 | 10 | 10 |
| Phase 02 | 7 | 7 |
| Phase 03 | 11 | 11 |
| Phase 04 | 4 | 4 |
| Phase 05 | 3 | 3 |
| Phase 06 | 3 | 3 |
| Phase 07 | 6 | 6 |
| Phase 08 | 5 | 5 |
| Phase 09 | 5 | 5 |
| Phase 12 | 5 | 5 |
| **Summe** | **59** | **59** |

**BESTÄTIGT** bestätigt die in der letzten Spalte ausgewiesene Kategorie: eine bestätigte LÜCKE/HÄRTUNG wird dadurch kein DEFEKT. Bei DEFEKT steht der gebrochene Satz im Grund. Die ursprünglichen K-Klassen bleiben bestehen. **Phase 12 wurde ausschließlich auf Quellenherleitung und Duplikate geprüft; Schwere und Kategorie werden dort unverändert übernommen und nicht neu begründet.** Ihre ursprünglichen Zusagenzitate werden zur Zuordnung mitgeführt. Alle Quellenstellen beziehen sich auf den gebundenen Stand beziehungsweise dessen unten ausgewiesene gepinnte Abhängigkeiten.

## Urteile je Befund

| ID | Urteil | Grund | Beleg | Schwere neu | Kategorie neu |
| --- | --- | --- | --- | --- | --- |
| T3-01-01 | BESTÄTIGT | Neutral bedeutet hier weiter Float→Double→Float. Die Sonde aktiviert DAZ/FTZ; ein endliches subnormales Eingangssample wird dadurch Null. Der Unity-Kurzschluss vermeidet Multiplikation, nicht die Konversion. Die M-02-Probe speist nur normale Zufallswerte; NAK-81 schließt den Rand nicht. Bricht: „Auch **ohne** Hard-Bypass ist der engagierte, aber neutrale Kern bitidentisch: 0 dB Trim läuft **nicht** als Multiplikation durch die Samples, sondern über einen Unity-Kurzschluss — einschliesslich der Rampe, die auf dem Weg zu 1,0 durch `0.99999994f` laufen kann.“ (SONDE-015 M-02, docs/beweise/SONDE-015.md:1035) | eq-copilot/plugin/sonde/SondeProcessor.cpp:337; eq-copilot/plugin/dsp/DspKern.cpp:893; eq-copilot/plugin/dsp/DspKern.cpp:1077; eq-copilot/build/_deps/juce-src/modules/juce_audio_basics/buffers/juce_FloatVectorOperations.cpp:1558; eq-copilot/plugin/tests/SondeNullTestMain.cpp:356 | critical | DEFEKT |
| T3-01-02 | BESTÄTIGT | Erreichbar über gewöhnliche VST3-Parameterautomation im process-Aufruf: Benachrichtigung nimmt listenerLock und danach den APVTS-Listener-Mutex. Die atomaren Sonde-Listener machen diese äußeren Sperren nicht rückgängig. NAK-48 betrifft eine andere Wrapper-Sperre. Bricht: „Audio-Thread: keine Sperren, Allokationen, Datei-, Pipe- oder Netzzugriffe, kein Logging; Überlast verwirft Analyseframes, nie Audio.“ (CLAUDE.md:32) | eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3757; eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessorParameter.cpp:113; eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/utilities/juce_AudioProcessorValueTreeState.cpp:185; eq-copilot/plugin/sonde/SondeProcessor.cpp:157 | high | DEFEKT |
| T3-01-03 | BESTÄTIGT | Der Wrapper setzt den NonRealtime-Modus je Offline-Block. Der override nimmt dabei zustandSchloss, das Save und Steuerworker ebenfalls nehmen; kein einmaliger vorbereitender Aufruf. Die Audiothread-Invariante enthält keine Offline-Ausnahme. Bricht: „Audio-Thread: keine Sperren, Allokationen, Datei-, Pipe- oder Netzzugriffe, kein Logging; Überlast verwirft Analyseframes, nie Audio.“ (CLAUDE.md:32) | eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3892; eq-copilot/plugin/sonde/SondeProcessor.cpp:1287; eq-copilot/plugin/sonde/SondeProcessor.cpp:992; eq-copilot/plugin/sonde/SondeProcessor.cpp:1415 | high | DEFEKT |
| T3-01-04 | BESTÄTIGT | Preview-Ende setzt den Zielzustand zurück, erhält aber die Candidate-Bank für die Überblendung; bei t=0 liefert der erste Offline-Sample noch die Vorschau. B7 verwirft vor seinem Vergleich acht Blöcke und widerlegt den Übergang nicht. Eigene Wirkung gegenüber 01-03: Audioinhalt statt Sperre. Bricht: „Offline-Render läuft mit dem bestätigten Zustand, nie mit einer Vorschau.“ (SONDE-015 M-120, docs/beweise/SONDE-015.md:1224) | eq-copilot/plugin/sonde/SondeProcessor.cpp:1292; eq-copilot/plugin/dsp/DspKern.cpp:375; eq-copilot/plugin/dsp/DspKern.cpp:980; eq-copilot/plugin/dsp/DspKern.cpp:1062; eq-copilot/plugin/tests/TransactionTestMain.cpp:2219 | high | DEFEKT |
| T3-01-05 | BESTÄTIGT | Mehrere blockweise Hostwerte können vor dem nächsten 5-ms-Workerzug auf einen letzten Wert zusammenfallen. Realtime und schneller Offline-Lauf erreichen deshalb trotz gleicher Event-/Blockfolge verschiedene DSP-Ziele. Die samplegetaktete Rampe heilt verlorene Ziele nicht; B6 synchronisiert seine Workerzüge ausdrücklich. Bricht: „Beide erzeugen **denselben Parameterverlauf** und damit innerhalb numerischer Toleranz denselben Ausgang, bei gleichem Event- und Blockverlauf.“ (SONDE-015 M-120, docs/beweise/SONDE-015.md:1224) | eq-copilot/plugin/sonde/SondeProcessor.cpp:529; eq-copilot/plugin/sonde/SondeProcessor.cpp:1296; eq-copilot/plugin/sonde/SondeProcessor.cpp:1421; eq-copilot/plugin/tests/DspGoldenTestMain.cpp:4410 | high | DEFEKT |
| T3-01-06 | BESTÄTIGT | Ein endlicher Float von etwa 2e19 überläuft bei v*v; zusätzlich kann die Float-Summe vieler einzeln endlicher Quadrate überlaufen. Die spätere Nullersetzung zählt das Rechenergebnis nicht, nanSeen prüft nur Eingangssamples. Der Gegenwert ist ein zulässiger endlicher Hostsample; ein NaN-Eingangsriegel deckt ihn nicht. Bricht: „Nicht-endliche Werte werden verriegelt und gezählt;“ (CLAUDE.md:193) | eq-copilot/plugin/src/PluginProcessor.cpp:627; eq-copilot/plugin/src/PluginProcessor.cpp:631; eq-copilot/plugin/src/PluginProcessor.cpp:633; eq-copilot/plugin/src/PluginProcessor.cpp:636; eq-copilot/plugin/src/PluginProcessor.cpp:641; eq-copilot/plugin/src/prozessor/Ipc.cpp:1134 | medium | DEFEKT |
| T3-01-07 | BESTÄTIGT | Die JUCE-FFT rechnet intern mit Float. Große endliche Fenster können darin Inf erzeugen; die anschließende Double-Umwandlung rettet es nicht. summePsd übernimmt das Ergebnis, während der gezählte Riegel ausschließlich nichtendliche Eingangssamples sieht. Andere Fehlerstelle als 01-06. Bricht: „Nicht-endliche Werte werden verriegelt und gezählt;“ (CLAUDE.md:193) | eq-copilot/plugin/src/AnalyseEngine.cpp:120; eq-copilot/plugin/src/AnalyseEngine.cpp:133; eq-copilot/plugin/src/AnalyseEngine.cpp:153; eq-copilot/plugin/src/AnalyseEngine.cpp:346; eq-copilot/build/_deps/juce-src/modules/juce_dsp/frequency/juce_FFT.cpp:375 | medium | DEFEKT |
| T3-01-08 | BESTÄTIGT | abs(±Inf) besteht die Impulsschwelle und setzt ersten Impuls, Peak und Lautzähler. Der Kommentar über ungültiges Material ist kein ausführbarer Riegel. Damit kann die vorhandene Host-Messsonde einen Versatz aus einem ungültigen Sample ableiten; keine Aussage über aktuellen Hostinput. Bricht: „Nicht-endliche Werte werden verriegelt und gezählt;“ (CLAUDE.md:193) | eq-copilot/plugin/spike/AuxSpikeProcessor.cpp:195; eq-copilot/plugin/spike/AuxSpikeProcessor.cpp:206; eq-copilot/plugin/spike/AuxSpikeProcessor.cpp:278 | medium | DEFEKT |
| T3-01-09 | BESTÄTIGT | Host-Bypass ruft den geerbten processBlockBypassed auf und umgeht Sonde/DspKern samt deren Übergangs- und Verlaufspfad. Die internen Bypass-Goldens prüfen einen anderen Einstieg. Keine Zusage für den Wiederanschluss nach Host-Bypass gefunden; daher weiterhin LÜCKE, kein behaupteter gemessener Klick. Keine Zusage. | eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3906; eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessor.cpp:592; eq-copilot/plugin/sonde/SondeProcessor.h:147 | medium | LÜCKE |
| T3-01-10 | BESTÄTIGT | Der Gen-Prozessor überschreibt reset nicht; der Wrapper ruft bei Stop die leere Basismethode. Die Hörmarkierung besitzt eigene Filterhistorie und explizite Rücksetzwege. Die für Probeeq gebaute Reset-Wache gilt nicht für Gen. Fehlende Regel betrifft das Fortleben dieser Historie am Hostreset. Keine Zusage. | eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:3615; eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessor.cpp:589; eq-copilot/plugin/src/HoerMarkierung.h:731; eq-copilot/plugin/src/PluginProcessor.h:1051 | medium | LÜCKE |
| T3-02-01 | BESTÄTIGT | Ein geladener offener AssistantStep darf INT64_MAX tragen. Der erfolgreiche Prozessorwrapper startet den Versuch und erhöht danach ungeprüft; die drei bewachten State-Bibliotheksmethoden werden dabei umgangen. Neu gegenüber NAK-283/F11 ist dieser zusätzliche Einstieg, nicht erneut der reparierte Bibliotheksweg. Bricht: „Kein Überlauf, kein unlesbarer Stand.“ (NAK-283, M-10); „State bleibt verlustfrei.“ (CLAUDE.md:198) | eq-copilot/plugin/state/NakamaState.cpp:1905; eq-copilot/plugin/src/prozessor/Analyse.cpp:1291; eq-copilot/plugin/src/prozessor/Analyse.cpp:1315; eq-copilot/plugin/tests/Sonde014IntentTest.cpp:1488 | high | DEFEKT |
| T3-02-02 | BESTÄTIGT | Bekannt NAK-252, jetzt mit vollständigem zulässigem Gegenbaum: Common mit vier Pflichtfeldern und 65.532 verschiedenen Zusatzfeldern wird angenommen. Save ergänzt label zu 65.537 Feldern, der nächste Reader verwirft. Byte-Headroom bleibt unter 1 MiB und prüft die Sammlungsgrenze nicht. Das übersteigt den registrierten unentschiedenen Verdacht; kein Lauf behauptet. Bricht: „Der eigene Reader nimmt jeden selbst erzeugten Stand an, und ein abgewiesener Handgriff verändert die gespeicherten Bytes nicht.“ (NAK-283, M-11); „State bleibt verlustfrei.“ (CLAUDE.md:198) | eq-copilot/plugin/state/NakamaState.cpp:184; eq-copilot/plugin/state/NakamaState.cpp:821; eq-copilot/plugin/state/NakamaState.cpp:1116; eq-copilot/plugin/state/NakamaState.cpp:1503; eq-copilot/plugin/state/NakamaState.cpp:2152; docs/audits/2026-09-15-tiefenaudit/roh/phase-02-zustand.md:44; docs/offene-punkte.md:262 | critical | DEFEKT |
| T3-02-03 | BESTÄTIGT | Die persistenten Int64-Revisionen werden oberhalb 2^53−1 geladen, während JSON-Vertrag und Broker sichere Integer verlangen; Sender geben die großen Werte unverändert aus oder kappen einen Statuswert. Ein zulässiger Reload kann damit die geforderte vollständige Intentmeldung verhindern. Kein Duplikat der lexikalischen Zahlendarstellung in 03-02. Bricht: „Main meldet seinen vollständigen Intent-Bestand — auch einen leeren — mit einer Vollständigkeitsmarke samt Revision, bevor der Broker eine Hypothese oder ein Proposal rechnet.“ (SONDE-014, M-86; Markdown-Hervorhebung entfernt) | eq-copilot/plugin/state/NakamaState.cpp:1696; eq-copilot/plugin/state/NakamaState.cpp:1905; eq-copilot/plugin/src/prozessor/Ipc.cpp:586; eq-copilot/plugin/src/prozessor/Ipc.cpp:674; eq-copilot/plugin/core/ipc/controlclient/Intern.h:443; broker/src/vertrag.rs:244 | high | DEFEKT |
| T3-02-04 | BESTÄTIGT | Reader akzeptiert längere Texte; der Panel-Abschluss kürzt Label/Paar-ID ohne bewusste Bearbeitung und ruft setzeBindung auch bei unveränderten Feldern. Bei leerer Main-Bindung erzeugt dieser Weg zusätzlich eine UUID vor dem Gleichheitsvergleich. Der API-No-op-Test öffnet das Panel nicht. Bricht: „State bleibt verlustfrei.“ (CLAUDE.md:198) | eq-copilot/plugin/src/PluginEditor.cpp:683; eq-copilot/plugin/src/PluginEditor.cpp:688; eq-copilot/plugin/src/PluginEditor.cpp:705; eq-copilot/plugin/src/prozessor/State.cpp:315; eq-copilot/plugin/state/NakamaState.cpp:1503; eq-copilot/plugin/tests/StateMigrationTestMain.cpp:2276 | critical | DEFEKT |
| T3-02-05 | BESTÄTIGT | Der Byte-Vorleser prüft Namen einzeln, nicht deren Eindeutigkeit; JUCE setzt gleichnamige Properties erneut und ersetzt ihren früheren Wert. Der eigene Writer erzeugt keine Duplikate, ein externer Stateblob erreicht den Pfad aber. Eine eindeutige Annahme-/Ablehnungsregel für solche Bytes fehlt. Keine Zusage. | eq-copilot/plugin/state/NakamaState.cpp:281; eq-copilot/build/_deps/juce-src/modules/juce_data_structures/values/juce_ValueTree.cpp:1069; eq-copilot/build/_deps/juce-src/modules/juce_core/containers/juce_NamedValueSet.cpp:187 | medium | LÜCKE |
| T3-02-06 | BESTÄTIGT | Der Klassenwechsel leert Mitglieder, lässt weitere Main-Modelle im laufenden Zustand; Save entfernt den MainProject-Unterbaum für Legacy. Zurückwechseln live und nach Reload ergibt deshalb verschiedene Bestände. Die Kind-Matrix erklärt das Wire-/Stateformat, aber keine Wiederherstellungsregel dieser Bedienfolge. Keine Zusage. | eq-copilot/plugin/src/prozessor/State.cpp:321; eq-copilot/plugin/state/NakamaState.cpp:1009; eq-copilot/plugin/state/NakamaState.cpp:2018 | medium | LÜCKE |
| T3-02-07 | BESTÄTIGT | Der Generator enthält kein eingefrorenes Common.project_binding_id-Beispiel. Roundtrips und Missing-field-Migration existieren und werden nicht als fehlend ausgegeben. NAK-267/A-3 nennt drei andere Main-Felder; der zusätzliche persistente Schlüssel ist ein enger neuer historischer Kompatibilitätscheck, weiterhin nur HÄRTUNG. Keine Zusage. | tools/eq-copilot/erzeuge_state_fixtures.py:1084; eq-copilot/plugin/tests/StateMigrationTestMain.cpp:1003; eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:2639; docs/offene-punkte.md:277 | low | HÄRTUNG |
| T3-03-01 | BESTÄTIGT | Der lokale Evidenzwriter schreibt project_sample_start:null. Der lokale Branch lässt null zu, die zugleich wirksame gemeinsame Property fordert integer; allOf hebt diese Bedingung nicht auf. Der fehlende lokale Projektstempel ist regulärer Rückfall, keine unerreichbare Eingabe. Bricht: „Schemas sind Verträge.“ (CLAUDE.md:182); transportstempel.project_sample_start: „type": "integer“. | eq-copilot/plugin/vertrag/NakamaEvidenz.cpp:194; eq-copilot/schemas/v3/eq-ipc-v3.schema.json:307; eq-copilot/schemas/v3/eq-ipc-v3.schema.json:360; eq-copilot/plugin/core/analysis/featureengine/Frame.h:270 | high | DEFEKT |
| T3-03-02 | BESTÄTIGT | 1.0 und 1e0 sind mathematische Integer; das vorhandene Schema-Golden akzeptiert 1.0 ausdrücklich. Produktleser prüfen dagegen JUCE-Var-Tags, Zifferntext oder serde as_u64 und lehnen ab beziehungsweise ersetzen durch 0. Das Schema-Golden ist Gegenbeleg für den Produktleser, nicht dessen Ausführung. Bricht: „Schemas sind Verträge.“ (CLAUDE.md:182); bestehender Test: „integer akzeptiert 1.0 (draft 2020-12)“ (SchemaTestMain.cpp:1115). | eq-copilot/plugin/tests/SchemaTestMain.cpp:1114; eq-copilot/plugin/src/SourcesModel.cpp:101; eq-copilot/plugin/core/ipc/controlclient/Intern.h:81; broker/src/transport/bootstrap.rs:242; broker/src/coordinator/evidenz.rs:267 | high | DEFEKT |
| T3-03-03 | BESTÄTIGT | state_revision oberhalb der sicheren JSON-Grenze wird akzeptiert; bei stoull-Überlauf entsteht sogar Revision 0 statt Ablehnung. inFlightAck läuft vor jedem Produktcallback und löscht den Auftrag. Der vorhandene ACK-Test prüft andere ungültige Formen. Neu gegenüber NAK-267/A-2 ist die konkrete falsche Freigabe. Bricht: „nur_schemafestes_command_ack_gibt_inflight_frei“ (IpcTestMain.cpp:5897); „Schemas sind Verträge.“ (CLAUDE.md:182). | eq-copilot/plugin/core/ipc/controlclient/Intern.h:206; eq-copilot/plugin/core/ipc/controlclient/Intern.h:244; eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp:363; eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:834; eq-copilot/plugin/tests/IpcTestMain.cpp:5897 | high | DEFEKT |
| T3-03-04 | BESTÄTIGT | Der Textriegel ist kein vollständiger JSON-Parser. JUCE akzeptiert hier Schlusskomma, unbekannte Escapeform beziehungsweise Rest nach dem Objekt; Sources übernimmt das Ergebnis. Rusts serde-Parser verwirft dieselben nicht-JSON-Nachrichten. Kein vorgeschalteter strenger Produktparser gefunden. Bricht: „Schemas sind Verträge.“ (CLAUDE.md:182); JSON-Kodierung des v3-Vertrags. | eq-copilot/plugin/src/SourcesModel.cpp:640; eq-copilot/plugin/vertrag/NakamaVertrag.cpp:563; eq-copilot/build/_deps/juce-src/modules/juce_core/json/juce_JSON.cpp:189; eq-copilot/build/_deps/juce-src/modules/juce_core/json/juce_JSON.cpp:303; eq-copilot/build/_deps/juce-src/modules/juce_core/json/juce_JSON.cpp:576; broker/src/coordinator/schema.rs:424 | high | DEFEKT |
| T3-03-05 | BESTÄTIGT | Bootstrap führt serde direkt aus: Option-Felder machen explizites null zu Abwesenheit; Versions-/Hosttexte werden nur nach Zeichenlänge geprüft. Die Authentisierung davor ersetzt keine Schema-/Textriegelprüfung. Ein gültiger Rahmen mit host:null oder NUL im Versionstext erreicht diesen Zweig. Bricht: „Schemas sind Verträge.“ (CLAUDE.md:182); hello_control.host verweist auf object, host_angabe.name/version verlangen string; v3-Textriegel verbietet NUL-Escapes. | broker/src/transport/server_v3/verbindung.rs:253; broker/src/transport/bootstrap.rs:95; broker/src/transport/bootstrap.rs:175; broker/src/transport/bootstrap.rs:190; broker/src/transport/bootstrap.rs:254; eq-copilot/schemas/v3/eq-ipc-v3.schema.json:859; eq-copilot/schemas/v3/eq-ipc-v3.schema.json:887 | high | DEFEKT |
| T3-03-06 | BESTÄTIGT | Handshake vergleicht UTF-8-Bytezahl mit den Zeichenobergrenzen und der Flachparser lehnt auch gültige Escapes ab. Dadurch werden vertragsgültige Welcome-/Reject-Texte abgewiesen. Der bestehende Escape-Test schreibt gerade die engere Implementierungsregel fest; er ändert den Schema-Vertrag nicht. Bricht: „Schemas sind Verträge.“ (CLAUDE.md:182); welcome.broker_version maxLength 64 und reject.reason maxLength 500. | eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:198; eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:228; eq-copilot/plugin/core/ipc/IpcVerbindung.cpp:439; eq-copilot/plugin/tests/IpcTestMain.cpp:5652 | medium | DEFEKT |
| T3-03-07 | BESTÄTIGT | Eine zusätzliche belegte Adresse-Vtableposition passiert den generierten Verifier und beide semantischen Leser, die nur fünf bekannte Felder prüfen. Die übliche FlatBuffers-Additivität widerlegt den Befund gerade nicht: Die Zieladresse ist ausdrücklich geschlossen. Bricht: „Für v3 sind Discriminator, Zieladresse, Revision und Capability nicht additiv erweiterbar.“ (CLAUDE.md:184–185). | eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp:159; broker/src/telemetrie.rs:247; eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h:252; broker/src/generiert/nakama_telemetry_v1_generated.rs:515; eq-copilot/schemas/v3/README.md:263 | high | DEFEKT |
| T3-03-08 | BESTÄTIGT | V2 ruft JUCE-JSON vor einer begrenzenden Zahlenprüfung. Schon die lexikalische Akkumulation eines ausreichend großen positiven Integer überläuft signed int64; spätere Enum-/seq-Prüfungen kommen zu spät. Die UTF-8- und Framegrößenriegel begrenzen die Zahl nicht. Bricht: „Schemas sind Verträge.“ (CLAUDE.md:182); v2 welcome.protocol_version enum [1,2], heartbeat_ack.seq mit int64-Obergrenze. | eq-copilot/plugin/src/PipeClient.cpp:493; eq-copilot/plugin/src/PipeClient.cpp:589; eq-copilot/plugin/src/PipeClient.cpp:682; eq-copilot/build/_deps/juce-src/modules/juce_core/json/juce_JSON.cpp:263 | high | DEFEKT |
| T3-03-09 | BESTÄTIGT | Gleiche JSON-Schlüssel werden je nach Eingang unterschiedlich behandelt: JUCE/serde Value ersetzen, der flache C++-Parser und typisierte Rust-Structs lehnen ab. Striktes Feldinventar allein verbietet keinen zweiten gleichnamigen Schlüssel. Weiterhin fehlende gemeinsame Regel, kein behaupteter normativer Defekt. Keine Zusage. | eq-copilot/build/_deps/juce-src/modules/juce_core/json/juce_JSON.cpp:315; eq-copilot/plugin/core/ipc/IpcVerbindung.cpp:515; broker/src/coordinator/schema.rs:424; broker/src/transport/bootstrap.rs:254 | medium | LÜCKE |
| T3-03-10 | BESTÄTIGT | Das Bit meldet unsupported und P2 läuft tatsächlich ohne Bitprüfung. §53.6 nennt bereits einen JSON-Rückfall; deshalb ist keine neue allgemeine Fallback-Erfindung nötig. Offen bleibt die operative Zuordnung des übernommenen historischen Hostnachweises zur inzwischen produktiven P2-Fähigkeit. LÜCKE enger als ein unbelegtes P2-Verbot. Keine Zusage. | eq-copilot/plugin/core/ipc/controlclient/Intern.h:401; eq-copilot/plugin/core/ipc/TelemetryClient.cpp:780; broker/src/coordinator/senke.rs:125; eq-copilot/identity/host-capabilities-fl-v1.json:97; docs/FL-Nakama-Sonden-Design-Entwurf.md:3994 | medium | LÜCKE |
| T3-03-11 | BESTÄTIGT | Beide Welcome-Eingänge prüfen Familie/Inhalt, aber keine obere Envelope-Minorfassung; spätere Control-/FeatureFrame-Leser tun dies. Keine allgemeine Welcome-Minor-Zusage gefunden. NAK-154 nennt den anderen, späteren Frame-Callback; eine Konsistenzwache am Verbindungsaufbau bleibt zusätzliche HÄRTUNG. Keine Zusage. | eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:638; eq-copilot/plugin/core/ipc/TelemetryClient.cpp:653; eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:804; eq-copilot/plugin/src/SourcesModel.cpp:1531; docs/offene-punkte.md:182 | low | HÄRTUNG |
| T3-04-01 | BESTÄTIGT | Die drei Sonden-Provider fangen this roh; nach 2-s-Stoppfrist kann deren Thread abgelöst weiterlaufen. Halten vor dem ersten Zustandsschloss lässt den Analyseworker regulär enden, dann zerstört der Destruktor die Provider-Ziele. Gen-Schleuse und ihre B14-Tests besitzen nur Gen, nicht diese Sonde. Bricht: „Kein Produkt-Callback greift nach Beginn der Prozessorzerstörung auf Prozessorzustand zu.“ (NAK-246, R-D2, docs/beweise/NAK-246.md:1008). | eq-copilot/plugin/sonde/SondeProcessor.cpp:139; eq-copilot/plugin/sonde/SondeProcessor.cpp:216; eq-copilot/plugin/sonde/SondeProcessor.cpp:1105; eq-copilot/plugin/core/ipc/ControlClient.cpp:133; eq-copilot/plugin/core/ipc/TelemetryClient.cpp:328; eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:440 | critical | DEFEKT |
| T3-04-02 | BESTÄTIGT | Der erste Modelllock prüft Bindung, der zweite mutiert nach beliebig langer Parsephase ohne Reload-Generation. Ein dazwischenliegender Reload kann bereits geleerte Rücknahmefelder wieder mit dem alten Grund füllen. Lebensdauerschleuse und sequenzieller Reload-Test verhindern dieses Interleaving nicht. Bricht: „Die Sicht ist sofort frei von alten Sitzungsobjekten.“ (NAK-246 M-29, docs/beweise/NAK-246.md:651; dort ausdrücklich evidenzRuecknahmen == 0 sowie leerer Grund und Umfang direkt nach setStateInformation). | eq-copilot/plugin/src/SourcesModel.cpp:1375; eq-copilot/plugin/src/SourcesModel.cpp:1504; eq-copilot/plugin/src/SourcesModel.cpp:1519; eq-copilot/plugin/src/prozessor/State.cpp:197; eq-copilot/plugin/src/prozessor/Ipc.cpp:1063; eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:2423 | medium | DEFEKT |
| T3-04-03 | BESTÄTIGT | JUCE besitzt das Panel im Modalcallback und zerstört ihn erst im Async-Update. Zulässige Reihenfolge Editor abbauen → Prozessor abbauen → Nachrichtenschleife lässt dessen Destruktor auf proz zugreifen. Der SafePointer schützt ausschließlich den Editor, nicht den davor aufgerufenen Prozessor. Keine explizite Panel-Zusage gefunden; LÜCKE mit möglichem Hostabsturz bleibt critical. Keine Zusage. | eq-copilot/plugin/src/PluginEditor.cpp:688; eq-copilot/plugin/src/PluginEditor.cpp:707; eq-copilot/build/_deps/juce-src/modules/juce_gui_basics/windows/juce_CallOutBox.cpp:83; eq-copilot/build/_deps/juce-src/modules/juce_gui_basics/components/juce_ModalComponentManager.cpp:88; eq-copilot/build/_deps/juce-src/modules/juce_gui_basics/components/juce_ModalComponentManager.cpp:204; eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp:2326 | critical | LÜCKE |
| T3-04-04 | HERABGESTUFT | Der Nachlauf ist real, aber der Absichtskommentar erlaubt nach Fristablauf ausdrücklich einen Restthread mit geteiltem Speicher und Stoppprüfung vor jedem Takt. M-45 prüft neu ausgelöste Takte nach Stopp, keinen abgebrochenen laufenden Schreibschritt. Kein UAF, Fristbruch oder weiterer Takt belegt. Nützlich bleibt eine Wache, die den bereits erlaubten verzögerten Abschluss misst; HÄRTUNG/low statt Funktionslücke. Keine Zusage. | broker/src/briefkasten.rs:570; broker/src/briefkasten.rs:743; broker/src/briefkasten.rs:887; broker/src/briefkasten.rs:1791; docs/beweise/NAK-286.md:575 | low | HÄRTUNG |
| T3-05-01 | BESTÄTIGT | Reload leert das aktuelle Overlay, nicht hostWert/hostEreignisseOffen. Der anschließende Hostabgleich läuft mit Herkunftssperre und überschreibt diese Mailbox nicht; ein späterer Workerzug wendet deshalb vor dem Reload gespeicherte Werte erneut an. B7s Reload auf neuer Instanz besitzt keine alte Mailbox. Bricht: „Der Reload rekonstruiert **denselben `state_hash`** und innerhalb numerischer Toleranz denselben Audioausgang.“ (SONDE-015 M-84, docs/beweise/SONDE-015.md:1162). | eq-copilot/plugin/sonde/SondeProcessor.cpp:1047; eq-copilot/plugin/sonde/SondeProcessor.cpp:1299; eq-copilot/plugin/sonde/SondeProcessor.cpp:1388; eq-copilot/plugin/sonde/SondeProcessor.cpp:1421; eq-copilot/plugin/tests/TransactionTestMain.cpp:2183 | high | DEFEKT |
| T3-05-02 | BESTÄTIGT | Read-only lädt zunächst neutral, blockiert aber nicht den späteren Hostparameterweg. eq_enabled und Trim können über das nichttopologische Hostoverlay wieder Audioverarbeitung aktivieren. Die bewachte Transaktions-API ist kein Gegenpfad für Hostautomation; Originalbytes können trotzdem erhalten bleiben. Bricht: „Er bleibt **read-only mit Originalbytes** wie heute: audio-neutral, keine Bindung, `getStateInformation` liefert die Bytes unverändert zurück.“ (SONDE-015 M-92, docs/beweise/SONDE-015.md:1175). | eq-copilot/plugin/sonde/SondeProcessor.cpp:1047; eq-copilot/plugin/sonde/SondeProcessor.cpp:1202; eq-copilot/plugin/sonde/SondeProcessor.cpp:1421; eq-copilot/plugin/state/NakamaTransaktion.cpp:345; eq-copilot/plugin/dsp/DspProgramm.cpp:176; eq-copilot/plugin/tests/TransactionTestMain.cpp:2008 | high | DEFEKT |
| T3-05-03 | BESTÄTIGT | Legitime Folge: Plugin gegen Manifest-Pin A bauen, Broker neu bauen, --hashen schreibt B, Installation ohne erneuten Pluginbau. Configure-Dependency wirkt erst beim nächsten Build; --hashen kehrt vor der Headerprüfung zurück. Das installierte Plugin verwirft B. Neu gegenüber NAK-127 ist der im Plugin verbliebene Pin trotz aktueller Manifest-/Artefakthashes. Bricht: „generierter Produktheader entspricht Pfad, Hash und Thumbprint des Manifests“ (bestehende A17-Prüfung, tools/eq-copilot/pruefe_installer_manifest.py:1382). | eq-copilot/plugin/CMakeLists.txt:41; eq-copilot/plugin/CMakeLists.txt:56; tools/beweise.ps1:886; tools/beweise.ps1:895; tools/fl/laufzeit.ps1:408; tools/eq-copilot/pruefe_installer_manifest.py:1665; eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:309 | high | DEFEKT |
| T3-06-01 | BESTÄTIGT | Die einseitige PSD verdoppelt auch DC und Nyquist. Für periodisch gefenstertes konstantes Stereo ist die spektrale Summe 5A²/3 statt A²; bei −61 dBFS überschreitet nur der Spektrumpfad die −60-dBFS-Schranke. Das FFT-Golden behandelt Randbins korrekt, führt diesen FeatureEngine-Gatevergleich aber nicht aus. NAK-283/F06 reparierte die Binbreite, nicht diese Gewichte. Bricht: „Sie liefert kohärente Mess-Snapshots; der Editor hält Anzeigezustand und rendert nur neue Snapshot-Revisionen.“ (CLAUDE.md:186; NAK-283 M-46 nennt dieselbe Invariante.) | eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:426; eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:449; eq-copilot/plugin/core/analysis/Fft.h:114; eq-copilot/plugin/core/analysis/FeatureEngine.h:1118; eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:716; eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp:981 | medium | DEFEKT |
| T3-06-02 | BESTÄTIGT | NaN-PCM wird in der Analyse durch Null ersetzt und gezählt; der separate I-Integrator erhält dennoch endliche Zellen. Danach setzt die Sonde dessen endliches Ergebnis in den bereits invalidierten Frame zurück und setzt die Präsenzbits. B18 prüft andere Rahmenskalare, nicht diesen nachgelagerten I-Overlay. Damit überlebt kein Gegenbeleg aus den vorhandenen NaN-Tests. Bricht: „Bei den Rahmenskalaren fehlt statt dessen das Präsenzbit. Der Zähler wird ausgewertet, nicht ignoriert.“ (SONDE-013 M-07, docs/beweise/SONDE-013.md:476). | eq-copilot/plugin/src/AnalyseEngine.cpp:352; eq-copilot/plugin/src/AnalyseEngine.cpp:387; eq-copilot/plugin/src/AnalyseEngine.cpp:710; eq-copilot/plugin/core/analysis/featureengine/Frame.h:590; eq-copilot/plugin/sonde/SondeProcessor.cpp:638; eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:620; eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp:625; eq-copilot/plugin/tests/LoudnessGoldenTestMain.cpp:458 | medium | DEFEKT |
| T3-06-03 | BESTÄTIGT | 400 Bootstrap-Züge begrenzen den kleinsten zweiseitigen p-Wert auf 2/401. Bei BH-q=0,05 brauchen 221 endliche Bänder daher mindestens 23 gleichzeitig kleine p-Werte, 64 Bänder mindestens sieben. Eine schmale stabile Änderung kann trotz gerichteter Intervalle unverändert heißen. Die Verdrahtung erzeugt diese Familie; Goldens mit p=0,0001 umgehen die produktive Auflösung. Keine zugesagte Mindestempfindlichkeit gefunden. Keine Zusage. | broker/src/coordinator/experiment.rs:417; broker/src/coordinator/experiment.rs:493; broker/src/coordinator/experiment.rs:585; broker/src/coordinator/experiment.rs:1606; broker/src/coordinator/experiment.rs:1675; broker/src/coordinator/experiment_verdrahtung.rs:974; eq-copilot/plugin/tests/Sonde013ExperimentGoldenTest.cpp:217; broker/tests/sonde013_experiment.rs:506 | medium | LÜCKE |
| T3-07-01 | BESTÄTIGT | Ein globaler Userintent und ein abgeleiteter Passagenintent dürfen nebeneinander bestehen. Beide Resolver wählen die Passage vor dem Herkunftsvergleich; dadurch verschwindet auch das globale Verschmelzungs-Veto. Der korrekt getestete reine Prioritätsvergleich wird im Resolver nicht benutzt; der Resolvertest vergleicht zweimal user. Der Gegenfall ist regulär erreichbar. Bricht: „Die Konfliktauflösung folgt **genau der Reihenfolge aus §37.2**: 1. Schutz-/Sicherheitsgrenze, 2. expliziter Userintent, 3. passagespezifischer vor globalem Intent, 4. bestätigte Vorlage, 5. abgeleitete Vermutung.“ (SONDE-014 M-05) | eq-copilot/plugin/state/NakamaState.cpp:2236; eq-copilot/plugin/state/NakamaState.cpp:2256; eq-copilot/plugin/state/NakamaState.cpp:2316; eq-copilot/plugin/state/NakamaState.cpp:2497; broker/src/coordinator/intent.rs:155; eq-copilot/plugin/tests/Sonde014IntentTest.cpp:517 | high | DEFEKT |
| T3-07-02 | BESTÄTIGT | Bekannt NAK-179, mit zusätzlichem Fehler unabhängig von der Hostbrücke: Legacy zeigt Solo/Puls und meldet Engagement, DSP verlangt aber Main. Beim Wechsel zu hub wird der Marker zulässig; zugleich verschwinden Aus-Knopf und die späteren Timer-Rücknahmen. Ein funktionsfähiger Bridge-/Transportpfad macht diese Kombination erreichbar. Der Markierungstest klassifiziert direkt Main und bedient diese Oberfläche nicht. Bricht: „jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich einen Zustand.“ (CLAUDE.md:51) | eq-copilot/plugin/src/PluginEditor.cpp:245; eq-copilot/plugin/src/PluginEditor.cpp:438; eq-copilot/plugin/src/PluginEditor.cpp:800; eq-copilot/plugin/src/PluginEditor.cpp:840; eq-copilot/plugin/src/PluginProcessor.cpp:848; eq-copilot/plugin/src/prozessor/State.cpp:315; eq-copilot/plugin/tests/MarkierungTestMain.cpp:179 | high | DEFEKT |
| T3-07-03 | HERABGESTUFT | invalid→stale ohne neuen Frame ist real. Gegen die behauptete Unehrlichkeit spricht jedoch die sichtbare Kennzeichnung „stale“ samt Alter; L14/A06 erlauben ausdrücklich eine alte gültige Messung mit Alter. Sie regeln nicht eindeutig die rückwirkende Evidenzrücknahme gegenüber dem separaten Lautheitspaar. Deshalb fehlt eine Regel für die dauerhafte Sperre, kein belegter current-Schwindel. R28/V24 prüfen nur unmittelbar nach Rücknahme. Keine Zusage. | eq-copilot/plugin/src/SourcesModel.cpp:1501; eq-copilot/plugin/src/SourcesModel.cpp:1832; eq-copilot/plugin/src/PluginEditor.cpp:1270; eq-copilot/plugin/src/PluginEditor.cpp:1299; docs/beweise/SONDE-012.md:154; docs/beweise/SONDE-012.md:224; eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp:883; eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp:970 | medium | LÜCKE |
| T3-07-04 | BESTÄTIGT | Eine persistente, inzwischen getrennte Quelle behält die gültige alte Nonce. Remove source nimmt dadurch den Brokerpfad; ohne lebendes Ziel kommt unknown_target zurück und der Handler verwirft den Auftrag ohne lokale Entfernung oder Fehleranzeige. Der lokale Pfad für nie verbundene Mitglieder und die erfolgreichen ACK-Tests decken diesen erreichbaren Zustand nicht. Bricht: „Gen führt die Quelle nicht länger als bestätigtes Mitglied; bleibt sie flüchtig entdeckt, ist sie ausdrücklich ungebunden/unclassified statt joined.“ (SONDE-012 L18) | eq-copilot/plugin/src/SourcesModel.cpp:1319; eq-copilot/plugin/src/prozessor/Ipc.cpp:1257; eq-copilot/plugin/src/prozessor/Ipc.cpp:1286; broker/src/coordinator/befehl.rs:549; eq-copilot/plugin/src/prozessor/Ipc.cpp:1063; eq-copilot/plugin/src/prozessor/Ipc.cpp:1073; docs/beweise/SONDE-012.md:158 | high | DEFEKT |
| T3-07-05 | HERABGESTUFT | Die feste Fläche erzeugt höchstens 20 Zeilen; ab Quelle 21 fehlen Auswahl und Handgriffe. Die zitierte Zusage ist aber mit dem ausdrücklich begrenzten Sichtumfang zu lesen: Entwurf §P3 fordert 1 Main + 16 sichtbare Probes, Last bis 32. Die 64 Plätze des Modells sind keine Sichtzusage; der Shot deckt 16 ab. Offen bleibt die Regel für akzeptierte Quellen oberhalb der sichtbaren Kapazität. Daher LÜCKE, nicht belegter Gatebruch. Keine Zusage. | eq-copilot/plugin/src/PluginEditor.cpp:858; eq-copilot/plugin/src/PluginEditor.cpp:951; eq-copilot/plugin/src/PluginEditor.cpp:974; eq-copilot/plugin/tests/ShotTestMain.cpp:114; docs/FL-Nakama-Sonden-Design-Entwurf.md:1590; docs/FL-Nakama-Sonden-Design-Entwurf.md:4277; docs/plan/plan.json:218 | medium | LÜCKE |
| T3-07-06 | BESTÄTIGT | setStateInformation auf einer bestehenden Gen-Instanz leert den Vergleichsversuch, nicht den Analyseverlauf oder die eingefrorene Editorlinie. Ohne zusätzliches prepareToPlay/neue Samples bleibt die alte Messung neben der neuen Identität. Der separate Neustartpfad widerlegt diesen API-Ablauf nicht. Eine Zuordnungsregel fehlt; weder FL-Verhalten noch ein ausgeführter Host-Repro werden behauptet. Andere Daten als 02-06 und 05-01. Keine Zusage. | eq-copilot/plugin/src/prozessor/State.cpp:167; eq-copilot/plugin/src/prozessor/Analyse.cpp:576; eq-copilot/plugin/src/PluginEditor.cpp:287; eq-copilot/plugin/src/PluginProcessor.h:1051; eq-copilot/plugin/src/PluginProcessor.h:1054 | medium | LÜCKE |
| T3-08-01 | BESTÄTIGT | Ein append-Auftrag wird an der 64er-Grenze auf zwei Transaktionen verteilt. Hinter 63 Ereignissen kann das Command vor seinem Domain-Effekt committen; nach Abbruch findet der Retry das Command und führt den Effekt nicht aus. Der Crashmatrix-Test erwartet diese Teilung ausdrücklich; B14 prüft nur die fertig geschriebenen Nachbarn. NAK-157 fehlte der Crashtest, hier ist zusätzlich der produktive Gegenablauf belegt. Bricht: „B14 — eine Preview committet ihre Ruecknahme MIT sich, oder gar nicht.“ (broker/tests/sonde013_verdrahtung.rs:3126; angenommene Regel SONDE-013 §13.4/B14) | broker/src/store/writer.rs:278; broker/src/store/writer.rs:325; broker/src/coordinator/befehl.rs:189; broker/src/coordinator/befehl.rs:409; broker/src/coordinator/befehl.rs:437; broker/tests/store_crash_matrix.rs:1701; broker/tests/sonde013_verdrahtung.rs:3126; docs/offene-punkte.md:185 | critical | DEFEKT |
| T3-08-02 | BESTÄTIGT | Der neue Frame liegt schon im Cache, wenn Transportwechsel die Rücknahme auslösen, aber noch nicht im Store. Die Projektion markiert nur vorhandene Zeilen; das spätere Append persistiert den neuen Frame ohne seinen Cache-Ausschluss. Auch der sequentielle Rücksprung mit neuer Epoche erreicht dies. NAK-167 beschreibt Append-Fehlschlag; hier genügt ein erfolgreicher Append. Die Verlustfolge entsteht beim Restore. Bricht: „Bereits eingegangene Evidenz wird per ID oder Bereich invalidiert.“ (SONDE-013 M-52, docs/beweise/SONDE-013.md:547); „jede Zeile der Sitzung traegt den Grund - haltbar, nicht nur fluechtig“ (broker/tests/sonde013_verdrahtung.rs:1484) | broker/src/coordinator/evidenz.rs:186; broker/src/coordinator/evidenz.rs:197; broker/src/coordinator/evidenz.rs:211; broker/src/coordinator/evidenz.rs:450; broker/src/coordinator/invalidierung_verdrahtung.rs:179; broker/src/store/writer.rs:656; broker/src/coordinator/experiment_verdrahtung.rs:1441; docs/offene-punkte.md:194 | high | DEFEKT |
| T3-08-03 | BESTÄTIGT | Zwischen dauerhaftem Commit und push_p1 fehlt eine gemeinsame Ordnungssperre. Invalidierung N kann dort warten, während Vollsnapshot N+1 bereits zugestellt wird; die Queue prüft Revisionen nur je Schlüssel und ordnet nach Ankunft. Die reparierte Deque ordnet nur bereits eingereihte Nachrichten. Neu gegenüber NAK-269/R-283-2 ist diese vorgelagerte Übergabelücke, nicht der alte Verbraucherfilter. Bricht: „Die Koaleszierung erhält die Entstehungsreihenfolge über Schlüssel: eine Ersetzung wandert ans Ende der Deque (Reihenfolge ist die Zeit der jüngsten Marke), sodass Vollsnapshot und Rücknahme desselben Objekts in ihrer Entstehungsreihenfolge zugestellt werden.“ (R-283-2, docs/beweise/NAK-283.md:112) | broker/src/coordinator/invalidierung_verdrahtung.rs:100; broker/src/coordinator/invalidierung_verdrahtung.rs:110; broker/src/coordinator/invalidierung_verdrahtung.rs:405; broker/src/coordinator/flush.rs:58; broker/src/coordinator/flush.rs:191; broker/src/coordinator/flush.rs:213; broker/src/transport/server_v3/queues.rs:228; broker/src/transport/server_v3/queues.rs:258; docs/beweise/NAK-283.md:112 | high | DEFEKT |
| T3-08-04 | BESTÄTIGT | Der produktive Runtime-Descriptor bildet keine pair_id ab; der Paarjoin überspringt solche Kandidaten. Die Gegenprobe B25 setzt pair_id über descriptor_setzen unmittelbar in den Coordinator. Vollständige Suche der Descriptor-Zuweisungen und Setter-Aufrufe ergibt keinen produktiven Ersatzpfad. Der Testhelper belegt den Algorithmus, nicht die versprochene Verbindung des Senders zum Join. Bricht: „Reihenfolge: Descriptor validieren → Paarkandidat bilden → Vollständigkeit prüfen → erst dann eine Paaraussage erzeugen.“ (SONDE-013 M-13, docs/beweise/SONDE-013.md:488); „B25: der PRE/POST-Join liefert ein Urteil je Paar (M-13/M-14)“ (broker/tests/sonde013_verdrahtung.rs:1154) | broker/src/coordinator/liveness.rs:429; broker/src/coordinator/liveness.rs:472; broker/src/coordinator/liveness.rs:741; broker/src/coordinator/liveness.rs:804; broker/src/coordinator/prepost_verdrahtung.rs:90; eq-copilot/plugin/sonde/SondeProcessor.cpp:1100; broker/tests/sonde013_verdrahtung.rs:1130; broker/tests/sonde013_verdrahtung.rs:1154 | high | DEFEKT |
| T3-08-05 | BESTÄTIGT | Der produktive Transportpfad interpretiert jede höhere Epoche als Epochwechsel und nimmt den alten Bereich zurück. Ein regulärer Loop-Wrap erhöht genau diese Epoche. Der reine Invalidierungshelfer besitzt die korrekte Grenzfallregel, hat dafür aber nur Testaufrufer. NAK-296 betrifft lokalen Analyseverlauf, nicht diese Brokerentscheidung; eine Reparatur von 08-02 behebt sie ebenfalls nicht. Bricht: „Ein Loop-Wrap **an** der Passagengrenze (Loop deckt die Passage vollständig) beendet einen Durchlauf und beginnt den nächsten — das ist **keine** Invalidierung und **keine** Herabstufung (Abgrenzung zu M-23).“ (SONDE-013 M-53, docs/beweise/SONDE-013.md:548) | broker/src/coordinator/invalidierung_verdrahtung.rs:628; broker/src/coordinator/invalidierung_verdrahtung.rs:648; broker/src/coordinator/invalidierung.rs:146; eq-copilot/plugin/core/analysis/featureengine/Zeit.h:222; eq-copilot/plugin/core/analysis/featureengine/Zeit.h:516; eq-copilot/plugin/core/analysis/featureengine/Frame.h:221; broker/tests/sonde013_taint.rs:127 | medium | DEFEKT |
| T3-09-01 | BESTÄTIGT | Der Freshness-Baum lässt plugin/dsp aus, obwohl dortige Quellen in NakamaKern und die Prüfprogramme eingehen. Eine gewöhnliche Änderung nur dort kann deshalb beim Lauf ohne -Bauen alte B6/B7-Binaries beglaubigen. A13 baut nur die Bibliothek und erzwingt kein Relinken dieser Programme. Dafür ist keine Sabotage eines Prüfers nötig; NAK-25 und NAK-133 betreffen andere Frischelücken. Bricht: „Der Runner baut, fährt die deklarierten Beine, legt Rohausgaben unter `docs/beweise/roh/` ab (Manifest trägt Kopf, Urteilszeile, Übersicht) und verweigert die Beglaubigung, wenn Prüfbinaries älter sind als ihre Quellen.“ (CLAUDE.md:212) | tools/beweise.ps1:969; tools/beweise.ps1:1016; eq-copilot/plugin/CMakeLists.txt:199; tools/eq-copilot/pruefe_kern_identitaetsfrei.py:289 | high | DEFEKT |
| T3-09-02 | BESTÄTIGT | Der R1-Nulltest vergleicht Zahlenwerte und meldet bei null Differenzen BITIDENTISCH. +0 und −0 sind numerisch gleich, haben aber verschiedene Bits; die berechneten Hashes ändern das Urteil nicht. Ein ansonsten identisches gültiges Float-WAV-Paar genügt. R2s UInt32-Vergleich und die LSB-Selbstprobe prüfen andere Wege. Bestätigt wird die falsche Detektoraussage, kein beobachteter Plugin-Nulltestbruch. Bricht: „Ausgeschaltet ist der Pfad im Nulltest bitidentisch; sonst Passthrough sampleidentisch, ohne Latenz oder Tail.“ (CLAUDE.md:30) | tools/fl/nulltest.py:206; tools/fl/nulltest.py:212; tools/fl/nulltest.py:225; tools/fl/nulltest.py:252; tools/fl/nulltest.py:419; tools/fl/nulltest.py:616 | medium | DEFEKT |
| T3-09-03 | BESTÄTIGT | P4-Konfidenzen stammen aus Literalwerten im Generator; der Prüfer koppelt sie nur an vorhandene Quellpfade/Testnamen. Sie werden nicht aus den behaupteten Produkturteilen gelesen. Gegen eine Defekt-Einordnung sprechen der ausdrückliche Registercharakter und die getrennten Goldens. Es fehlt die Regel, wann solche Annotationen als gemessene Produktkalibrierung gelten dürfen. NAK-200 und NAK-294 lösen diese Zuordnung nicht. Keine Zusage. | tools/eq-copilot/erzeuge_p4_korpus.py:3; tools/eq-copilot/erzeuge_p4_korpus.py:51; tools/eq-copilot/erzeuge_p4_korpus.py:90; tools/eq-copilot/pruefe_p4_korpus.py:160; tools/eq-copilot/pruefe_p4_korpus.py:219; tools/eq-copilot/erzeuge_p5_korpus.py:5 | medium | LÜCKE |
| T3-09-04 | BESTÄTIGT | Fehlende Junction-Voraussetzungen lassen beide betroffenen Rust-Tests nach einer Skip-Ausgabe erfolgreich zurückkehren. A4/A4SI nutzen keinen Schalter, der Ausgabe erfolgreicher Tests zeigt; Exit 0 wird beglaubigt. Das ist eine reguläre Maschinenbedingung. Ohne verbindliche Skip-Regel bleibt es eine konkrete Beweislücke, kein bewiesener Fehler des geschützten Storepfads. Keine Zusage. | broker/tests/store_crash_matrix.rs:3238; broker/tests/store_crash_matrix.rs:3242; broker/tests/store_crash_matrix.rs:3364; tools/beweise.ps1:493; tools/beweise.ps1:501; tools/beweise.ps1:1164 | medium | LÜCKE |
| T3-09-05 | BESTÄTIGT | Die ausdrücklich so bezeichnete Phaseninvarianzprobe endet in pruefe(true); die vorherigen Assertions prüfen Größe und Ähnlichkeit anderer Ausgaben. Ein tatsächlicher Vergleich nach Vorzeichenumkehr durch denselben Featurepfad wäre eine kleine unabhängige Wache. Daraus folgt weder ein aktueller Phasenleckbefund noch eine Zusage universeller Phaseninvarianz: weiterhin HÄRTUNG/low. Keine Zusage. | eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp:233; eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp:257; eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp:263; eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp:270 | low | HÄRTUNG |
| T3-12-01 | BESTÄTIGT | Die registrierte Highest-Aufgabe startet Skript und Helfer im beschreibbaren Checkout und konsumiert dort Manifest/Artefakte. Ein unabhängiger Anker für die ausgeführte Aktion fehlt in dieser Kette. NAK-285 erlaubt die Aufgaben; NAK-119 beschreibt den leeren Thumbprint, nicht die zusätzliche Skript-Privileggrenze. Bestätigung gilt der Quellkonfiguration, nicht einer ausgelesenen Live-Task-ACL. Schwere gemäß Auftrag unverändert. Ursprünglicher Zusagenbezug (hier nicht neu eingestuft): „Code, der bereits unter demselben User in FL injiziert wurde, liegt außerhalb des IPC-Threat-Models; dafür bleiben Signaturprüfung, Installationspfad und Betriebssystemschutz maßgeblich.“ (Gate NAK-123, docs/beweise/NAK-123.md:15) | tools/dirigent/install-aufgaben-registrieren.ps1:57; tools/dirigent/install-aufgaben-registrieren.ps1:66; tools/dirigent/install-aufgaben-registrieren.ps1:74; eq-copilot/install/Install-Nakama.ps1:87; eq-copilot/install/Install-Nakama.ps1:101; eq-copilot/install/Install-Nakama.ps1:908; eq-copilot/install/Install-Nakama.ps1:1147 | critical | DEFEKT |
| T3-12-02 | BESTÄTIGT | Windows-Auth liefert nur bool; der Coordinator übernimmt anschließend die syntaktisch geprüfte Draht-SID/Hostidentität. Security-Vektoren erreichen Welcome mit erfundener Draht-SID. V2 gibt außerdem seinen Sitzungstoken bereits mit Welcome aus. NAK-152 zur Kanonisierung beseitigt diese fehlende Bindung nicht. Keine Umgehung der realen Windows-Userprüfung behauptet; Schwere gemäß Auftrag unverändert. Ursprünglicher Zusagenbezug (hier nicht neu eingestuft): „Das schützt vor fremden Windows-Usern, Verwechslung und zufälligen lokalen Clients.“ (Gate NAK-123, docs/beweise/NAK-123.md:15) | broker/src/transport/server_v3/auth.rs:45; broker/src/transport/server_v3/auth.rs:103; broker/src/transport/server_v3/auth.rs:168; broker/src/transport/bootstrap.rs:149; broker/src/transport/server_v3/verbindung.rs:253; broker/src/coordinator/link.rs:80; broker/src/coordinator/zustand.rs:18; broker/src/protokoll.rs:307; broker/tests/security_vectors.rs:99; broker/tests/security_vectors.rs:418 | critical | DEFEKT |
| T3-12-03 | BESTÄTIGT | Der erste v2-Listener auf dem festen globalen Namen wird mit ? vor dem SID-gebundenen v3-Listener gestartet. Eine First-Instance-Kollision beendet deshalb den ganzen Start, auch bei freiem v3-Namen. Die isolierten v3-Gegenbesetzungstests widerlegen die vorgelagerte Kopplung nicht. Eigenständige Verfügbarkeitsschwäche gegenüber NAK-123; keine Pipe geöffnet. Schwere gemäß Auftrag unverändert. Ursprünglicher Zusagenbezug (hier nicht neu eingestuft): „Die erste RAII-besessene Instanz entsteht mit aktueller User-DACL, `PIPE_REJECT_REMOTE_CLIENTS` und `FILE_FLAG_FIRST_PIPE_INSTANCE`; eine zweite entsteht ohne First-Flag.“ (Matrix NAK-123 A-01, docs/beweise/NAK-123.md:37) | broker/src/lebenslauf.rs:41; broker/src/lebenslauf.rs:152; broker/src/lebenslauf.rs:161; broker/src/server.rs:502; broker/src/server.rs:514; broker/src/main.rs:50 | high | DEFEKT |
| T3-12-04 | BESTÄTIGT | Der feste Diagnosepfad akzeptiert syntaktische 32-Hex-Anfragen ohne Zulassung; Antworten enthalten Produktmessungen und der Broker leitet Diagnose-IDs aus dem vollständigen v2-Token ab. NAK-286 belegt diese Inhalte, regelt aber den Anforderer nicht. Der Token bringt gegenüber 12-02 keine weitere v2-Berechtigung; der eigenständige Messdaten-/Diagnosezugang verhindert ein vollständiges Duplikat. Schwere gemäß Auftrag unverändert. Keine Zusage. | eq-copilot/plugin/core/diagnose/Briefkasten.h:3; eq-copilot/plugin/core/diagnose/Briefkasten.h:76; eq-copilot/plugin/core/diagnose/Briefkasten.cpp:620; broker/src/briefkasten.rs:663; broker/src/briefkasten.rs:710; broker/src/briefkasten.rs:787; broker/src/briefkasten.rs:821; eq-copilot/plugin/src/prozessor/Analyse.cpp:1344; eq-copilot/plugin/sonde/SondeProcessor.cpp:921 | medium | LÜCKE |
| T3-12-05 | BESTÄTIGT | Die v2-Worker lesen Bootstrap und Folgerahmen blockierend ohne Frist/Rate; 64 belegte Plätze stoppen Annahmen. Nur halbe Hellos können noch vom Broker-Leerlaufende begrenzt werden, ein registrierter schweigender Client verhindert dieses Ende. Die Framegrößengrenze schützt nicht vor Dauer/Ratenlast; v3 hat eigene Wachen. NAK-241 nennt den Testhänger, nicht die Produktionsfolge. Schwere gemäß Auftrag unverändert. Keine Zusage. | broker/src/server.rs:48; broker/src/server.rs:549; broker/src/server.rs:673; broker/src/server.rs:782; broker/src/framing.rs:34; broker/src/framing.rs:59; broker/src/lebenslauf.rs:316 | medium | LÜCKE |

Zähler: bestätigt 56 · herabgestuft 3 · widerlegt 0 · Duplikat 0 · unentschieden 0; geprüfte Befundzeilen 59/59; Produktfragen 7.

Nach der Einordnung: **40 DEFEKTE · 15 LÜCKEN · 4 HÄRTUNGEN**; **critical 8 · high 24 · medium 23 · low 4**. Darin enthalten sind die fünf unverändert übernommenen Phase-12-Einordnungen. Es werden keine neuen T3-10-Befunde angelegt.

## Widerlegte Verdachte

**Keine vollständige Befundzeile ist WIDERLEGT.** Folgende Teilbehauptungen tragen ihre ursprüngliche Einordnung nicht; sie sind oben genau einmal als HERABGESTUFT gezählt:

- **T3-04-04:** Die Restarbeit nach verfehlter Join-Frist ist im Code ausdrücklich vorgesehen (`broker/src/briefkasten.rs:887`). M-45 und der Test verlangen null neu ausgelöste Takte nach Stopp, keinen Abbruch eines schon laufenden Dateischritts (`docs/beweise/NAK-286.md:575`, `broker/src/briefkasten.rs:1791`). Übrig bleibt eine gezielte Wache für diesen erlaubten Nachlauf: **low/HÄRTUNG**.
- **T3-07-03:** Der wieder sichtbare Zahlenwert behauptet keine aktuelle Messung: `PluginEditor.cpp:1270` zeigt stale mit Alter. SONDE-012 L14/A06 lassen ausdrücklich einen alten gültigen Wert mit Alter zu (`docs/beweise/SONDE-012.md:154`, `:224`). Diese Regel deckt nicht eindeutig die rückwirkende Evidenzrücknahme; deshalb **medium/LÜCKE**, keine bewiesene aktuelle Falschanzeige.
- **T3-07-05:** Aus 64 Modellplätzen folgt keine Zusage für 64 sichtbare Quellen. Der Entwurf begrenzt den P3-Sichtumfang ausdrücklich auf 16 Probes (`docs/FL-Nakama-Sonden-Design-Entwurf.md:4277`); Shot prüft diese Zahl (`eq-copilot/plugin/tests/ShotTestMain.cpp:114`). Die fehlende Regel oberhalb der sichtbaren Kapazität bleibt **medium/LÜCKE**.

Der Skeptikerpass hat außerdem mehrere scheinbare Gegenbeweise verworfen: B6/B7 synchronisieren oder überspringen gerade den strittigen Übergang (T3-01-04/05); der Gen-Callbackschutz besitzt nicht die Sonde (T3-04-01); korrekte reine Prioritäts- und Invalidierungshelfer sind nicht ihre produktiven Resolver (T3-07-01, T3-08-05); B25 setzt den Descriptor über einen Testeingang (T3-08-04). Ihre Belegstellen stehen in den jeweiligen Tabellenzeilen. Diese Gegenargumente erhöhen den WIDERLEGT-Zähler nicht.

## Abgrenzung bekannter Punkte und Überschneidungen

Kein Befund ist vollständig als DUPLIKAT auszuscheiden. Insbesondere:

- **NAK-252 / T3-02-02:** Das Register nennt einen unentschiedenen Grenzverdacht. Der zusätzliche vollständige, zulässige Gegenbaum mit 65.536 Properties plus nachgetragenem Label macht die Reader/Writer-Asymmetrie an der Quelle entscheidbar; kein ausgeführtes Grenzfixture wird behauptet.
- **NAK-267 / T3-02-07 und T3-03-03:** Der historische Fixture-Schlüssel `Common.project_binding_id` gehört nicht zu den drei registrierten Main-Feldern. Der ACK-Befund liefert zusätzlich eine konkrete falsche Inflight-Freigabe; er verlangt nicht pauschal die bisher unzugesagte Verdrahtung des Generalprüfers.
- **NAK-157 / T3-08-01:** Über den fehlenden Crashtest hinaus ist die Aufteilung eines fachlichen Append-Auftrags im produktiven Writer nachgewiesen (`broker/tests/store_crash_matrix.rs:1701`).
- **NAK-167 / T3-08-02:** Der Registerfall ist der fehlgeschlagene Snapshot-Append nach sachlich richtiger Rücknahme. Der neue Ablauf verliert den Ausschlussgrund trotz erfolgreichem Append beim späteren Restore.
- **NAK-269, NAK-246 D5/D9 und NAK-283 F03 / T3-08-03:** Die bereits reparierte Queue-Ordnung erfasst Nachrichten nach dem Einreihen. Die neue Lücke liegt zwischen Commit und Einreihen; sie setzt keinen zusätzlichen Consumer-Altersvertrag voraus.
- **NAK-200/294 / T3-09-03:** Klassen-Brier und Quellverweis-Hygiene ersetzen keine Verbindung deklarierter Konfidenzen mit gemessenen Produkturteilen.
- **Phase 12:** NAK-285/119, NAK-152, NAK-123, NAK-286 und NAK-241 erklären die bekannten Anteile. Die zusätzlichen Mechanismen sind in den fünf Urteilszeilen abgegrenzt. Der in T3-12-04 sichtbare Token erweitert wegen T3-12-02 keine Berechtigung; der eigenständige Diagnose-/Messdatenzugang bleibt.

T3-01-05, T3-05-01 und T3-05-02 teilen den Hostparameterkanal, aber nicht den verletzten Vertrag: deterministischer Verlauf, Reload-Grenze und read-only-Neutralität benötigen je eine eigene Wache. T3-04-01 und T3-04-03 besitzen verschiedene Lebensdauern. T3-08-02 und T3-08-05 sind Persistenzverlust und falsche Invalidierungsentscheidung; die Reparatur des einen beseitigt den anderen nicht.

## Produktfragen

Die folgenden sieben Punkte werden dem Dirigenten als fehlende Regeln übergeben; in diesem Lauf wird keine Produktentscheidung angefragt oder vorweggenommen.

- **P10-01 · Host-Lebenslauf:** Welche volatile Filter-, Mess- und Vergleichshistorie soll Host-Bypass, Host-reset und State-Reload überleben? Es fehlt eine Regel für die jeweiligen Wiederanschlüsse, nicht eine pauschale Forderung nach Reset (T3-01-09, T3-01-10, T3-07-06).
- **P10-02 · Klassenwechsel:** Soll Main → Legacy → Main persistente Main-Bestände erhalten, bewusst löschen oder einen ausdrücklichen Löschhandgriff verlangen? Live-Rückwechsel und Save/Reload müssen an derselben Entscheidung gemessen werden (T3-02-06).
- **P10-03 · Doppelte Schlüssel:** Sind doppelte Propertynamen beziehungsweise JSON-Schlüssel ungültig oder gilt eine eindeutige gemeinsame Auswahlregel? Die Entscheidung betrifft Annahme und Erhalt fremder Daten (T3-02-05, T3-03-09).
- **P10-04 · Capability:** Welche Aussage macht `binary_transport`, wenn der übernommene Hostnachweis unsupported meldet, der produktive P2-Pfad aber läuft? Der JSON-Rückfall ist bereits in §53.6 benannt; offen ist die operative Bedeutung des Bits, nicht die Erfindung einer Rückfalloption (T3-03-10).
- **P10-05 · Empfindlichkeit:** Welche kleinste schmale Änderung soll bei welcher gültigen Bandfamilie noch eine gerichtete Aussage erlauben? Erst diese Regel entscheidet über Bootstrap-Auflösung und FDR-Zusammenspiel (T3-06-03).
- **P10-06 · Rücknahme:** Entwertet evidence_invalidate auch ein zuvor gültiges separates LUFS-I-Paar dauerhaft, bis neue gültige Daten kommen, oder darf es ausdrücklich alt mit Alter stehen? Die vorhandenen Bad-frame-Regeln beantworten diese rückwirkende Zuordnung nicht eindeutig (T3-07-03).
- **P10-07 · Kapazität:** Welche technische Bedienregel gilt für akzeptierte Quellen oberhalb der zugesagten 16 sichtbaren Probes beziehungsweise der tatsächlichen 20 Zeilen? Zu entscheiden sind Erreichbarkeit oder Annahmegrenze; keine neue Designrichtung (T3-07-05).

## Nicht geprüft

- Keine Befunde außerhalb der Abschnitte 01–09 und 12 neu auditiert; keine Gesamtfreigabe, keine Garantie von Fehlerfreiheit.
- Keine Builds, Tests, Mutationen, Kanon-, Crash-, Sanitizer-, Stress-, Host- oder Installationsläufe. Alle positiven Urteile sind Quellenherleitungen; Häufigkeit, Zeit bis zum Fehler und Auswirkungen im konkreten FL-Lauf wurden nicht gemessen. Kein Fall musste für seine hier begrenzte Aussage UNENTSCHIEDEN bleiben.
- Kein Auslesen oder Ändern tatsächlicher Task-/Datei-ACLs, kein Sicherheitsexperiment, keine Verbindung zu Produktions- oder Probe-Pipes. T3-12-01 gilt für die implementierte Aufgabenregistrierung; die tatsächliche Maschineninstallation ist damit nicht neu vermessen.
- Manifeste, Register, Plan und Verhaltensmatrizen wurden nur als Zusage-, Absichts- und Duplikatkontext gelesen. Keine Dokumentationsbefunde, Design-/Optikbefunde, Legacy-Umbenennungen oder Forderungen gegen geparkte Studien.
- Keine Änderung fremder uncommitteter Pfade. Zu Beginn vorhanden: `.claude/settings.local.json`, `briefing-hub/`, `docs/audits/2026-09-15-tiefenaudit/roh/phase-10-auftrag.txt`, `nimbalyst-local/`.
- Der anfängliche Python-3.13-Leseversuch scheiterte an der lokalen Startumgebung; die Quellenlesung wurde mit PowerShell/rg durchgeführt. Kein benötigter Quellpfad blieb deswegen ungelesen.

## Scope-Beweis

**59/59 Befundzeilen, zehn von zehn erforderlichen Rohberichten, 152 explizit gelesene Repository-/Abhängigkeitsdateien.** Die folgende Liste zählt jede Datei einmal. CLAUDE.md und der angeforderte Einstiegskontext wurden gelesen; bei großen Fach- und Testdateien wurden die Befundstellen samt Aufruf-, Gegenpfad- und Testkontext gelesen, nicht jede Zeile des gesamten Baums. Die Dateizahl behauptet deshalb keine vollständige Zeilenabdeckung. Die Rohberichte wurden als Befundpopulation und Begründungskontext gelesen; ihre damaligen Scope-Listen sind kein Beleg für zusätzliche eigene Quellenlesung.

Die Quellen sind bei `git status --short` frei von Produktänderungen. `git rev-parse HEAD` vor und nach der Arbeit verlangt denselben SHA oben. Einzige eigene Schreibziele sind dieser Bericht und der angehängte Phase-10-Abschnitt in BEFUNDE.md. Der bisherige BEFUNDE-Inhalt umfasst **81.273 Bytes**, SHA-256 **F40B7D41ADC22F523499489BB4B3DF09D00A409ADBC2EB53DAE120E783039CC6**; die Abschlusskontrolle vergleicht genau diesen Bytepräfix und beide Urteilstabellen auf Identität und 59 eindeutige IDs.

**Abhängigkeitsstand:** JUCE ist lokal als **8.0.9** vorhanden und durch `eq-copilot/CMakeLists.txt:32` gebunden. Der gelesene gepatchte VST3-Wrapper hat nach LF-Normalisierung SHA-256 `6e5d4660d960836a875e4b2207f5bb4372b5266776e00c4bb0fdef1ee87a01bc`, passend zum Patch-Pin `eq-copilot/cmake/NakamaBruecke.cmake:22`. Die weiteren gelesenen JUCE-Dateien sind gegenüber diesem Abhängigkeitsstand unverändert. FlatBuffers liegt sauber auf `7e163021e59cca4f8e1e35a7c828b5c6b7915953` / v25.12.19, passend zum Werkzeugpin. Der Root-HEAD allein würde ungeprüfte Build-Abhängigkeiten nicht beglaubigen.

### Repository-Einstieg — 2 Dateien

```text
AGENTS.md
CLAUDE.md
```

### Broker — 34 Dateien

```text
broker/src/briefkasten.rs
broker/src/coordinator/befehl.rs
broker/src/coordinator/evidenz.rs
broker/src/coordinator/experiment.rs
broker/src/coordinator/experiment_verdrahtung.rs
broker/src/coordinator/flush.rs
broker/src/coordinator/intent.rs
broker/src/coordinator/invalidierung.rs
broker/src/coordinator/invalidierung_verdrahtung.rs
broker/src/coordinator/link.rs
broker/src/coordinator/liveness.rs
broker/src/coordinator/mod.rs
broker/src/coordinator/prepost_verdrahtung.rs
broker/src/coordinator/schema.rs
broker/src/coordinator/senke.rs
broker/src/coordinator/zustand.rs
broker/src/framing.rs
broker/src/generiert/nakama_telemetry_v1_generated.rs
broker/src/lebenslauf.rs
broker/src/main.rs
broker/src/protokoll.rs
broker/src/server.rs
broker/src/store/writer.rs
broker/src/telemetrie.rs
broker/src/transport/bootstrap.rs
broker/src/transport/server_v3/auth.rs
broker/src/transport/server_v3/queues.rs
broker/src/transport/server_v3/verbindung.rs
broker/src/vertrag.rs
broker/tests/security_vectors.rs
broker/tests/sonde013_experiment.rs
broker/tests/sonde013_taint.rs
broker/tests/sonde013_verdrahtung.rs
broker/tests/store_crash_matrix.rs
```

### Anforderungs- und Auditkontext — 28 Dateien

```text
docs/FL-Nakama-Sonden-Design-Entwurf.md
docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md
docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md
docs/audits/2026-09-15-tiefenaudit/roh/phase-01-echtzeit.md
docs/audits/2026-09-15-tiefenaudit/roh/phase-02-zustand.md
docs/audits/2026-09-15-tiefenaudit/roh/phase-03-vertrag.md
docs/audits/2026-09-15-tiefenaudit/roh/phase-04-lebenslauf.md
docs/audits/2026-09-15-tiefenaudit/roh/phase-05-hostkante.md
docs/audits/2026-09-15-tiefenaudit/roh/phase-06-messkern.md
docs/audits/2026-09-15-tiefenaudit/roh/phase-07-modell.md
docs/audits/2026-09-15-tiefenaudit/roh/phase-08-broker.md
docs/audits/2026-09-15-tiefenaudit/roh/phase-09-beweisluecken.md
docs/audits/2026-09-15-tiefenaudit/roh/phase-12-pipesicherheit.md
docs/beweise/NAK-123.md
docs/beweise/NAK-246.md
docs/beweise/NAK-283.md
docs/beweise/NAK-286.md
docs/beweise/SONDE-010.md
docs/beweise/SONDE-012.md
docs/beweise/SONDE-013.md
docs/beweise/SONDE-014.md
docs/beweise/SONDE-015.md
docs/gesundheit/KONZEPT.md
docs/gesundheit/abdeckungskarte.md
docs/gesundheit/lebenslauf-audit.md
docs/offene-punkte.md
docs/plan/plan.json
docs/plugin-wissen.md
```

### Verträge, Identität und Baukonfiguration — 7 Dateien

```text
eq-copilot/CMakeLists.txt
eq-copilot/cmake/NakamaBruecke.cmake
eq-copilot/identity/host-capabilities-fl-v1.json
eq-copilot/install/Install-Nakama.ps1
eq-copilot/schemas/v3/README.md
eq-copilot/schemas/v3/eq-ipc-v3.schema.json
eq-copilot/schemas/v3/flatbuffers/WERKZEUG.json
```

### Gepinnte Abhängigkeiten — 13 Dateien

```text
eq-copilot/build/_deps/flatbuffers-src/include/flatbuffers/verifier.h
eq-copilot/build/_deps/juce-src/modules/juce_audio_basics/buffers/juce_FloatVectorOperations.cpp
eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp
eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessor.cpp
eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessorParameter.cpp
eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/utilities/juce_AudioProcessorValueTreeState.cpp
eq-copilot/build/_deps/juce-src/modules/juce_core/containers/juce_NamedValueSet.cpp
eq-copilot/build/_deps/juce-src/modules/juce_core/json/juce_JSON.cpp
eq-copilot/build/_deps/juce-src/modules/juce_data_structures/values/juce_ValueTree.cpp
eq-copilot/build/_deps/juce-src/modules/juce_dsp/frequency/juce_FFT.cpp
eq-copilot/build/_deps/juce-src/modules/juce_gui_basics/components/juce_Component.cpp
eq-copilot/build/_deps/juce-src/modules/juce_gui_basics/components/juce_ModalComponentManager.cpp
eq-copilot/build/_deps/juce-src/modules/juce_gui_basics/windows/juce_CallOutBox.cpp
```

### Plugin, State und DSP — 40 Dateien

```text
eq-copilot/plugin/CMakeLists.txt
eq-copilot/plugin/core/analysis/FeatureEngine.h
eq-copilot/plugin/core/analysis/Fft.h
eq-copilot/plugin/core/analysis/featureengine/Frame.h
eq-copilot/plugin/core/analysis/featureengine/Lautheit.h
eq-copilot/plugin/core/analysis/featureengine/Spektrum.h
eq-copilot/plugin/core/analysis/featureengine/Vertrag.h
eq-copilot/plugin/core/analysis/featureengine/Zeit.h
eq-copilot/plugin/core/diagnose/Briefkasten.cpp
eq-copilot/plugin/core/diagnose/Briefkasten.h
eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp
eq-copilot/plugin/core/ipc/ControlClient.cpp
eq-copilot/plugin/core/ipc/IpcVerbindung.cpp
eq-copilot/plugin/core/ipc/TelemetryClient.cpp
eq-copilot/plugin/core/ipc/controlclient/Intern.h
eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp
eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp
eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp
eq-copilot/plugin/dsp/DspKern.cpp
eq-copilot/plugin/dsp/DspProgramm.cpp
eq-copilot/plugin/sonde/SondeProcessor.cpp
eq-copilot/plugin/sonde/SondeProcessor.h
eq-copilot/plugin/spike/AuxSpikeProcessor.cpp
eq-copilot/plugin/src/AnalyseEngine.cpp
eq-copilot/plugin/src/HoerMarkierung.h
eq-copilot/plugin/src/PipeClient.cpp
eq-copilot/plugin/src/PluginEditor.cpp
eq-copilot/plugin/src/PluginProcessor.cpp
eq-copilot/plugin/src/PluginProcessor.h
eq-copilot/plugin/src/SourcesModel.cpp
eq-copilot/plugin/src/SourcesModel.h
eq-copilot/plugin/src/prozessor/Analyse.cpp
eq-copilot/plugin/src/prozessor/Ipc.cpp
eq-copilot/plugin/src/prozessor/State.cpp
eq-copilot/plugin/state/NakamaState.cpp
eq-copilot/plugin/state/NakamaTransaktion.cpp
eq-copilot/plugin/vertrag/NakamaEvidenz.cpp
eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp
eq-copilot/plugin/vertrag/NakamaVertrag.cpp
eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h
```

### C++-Prüfquellen — 17 Dateien

```text
eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp
eq-copilot/plugin/tests/DspGoldenTestMain.cpp
eq-copilot/plugin/tests/IpcTestMain.cpp
eq-copilot/plugin/tests/LoudnessGoldenTestMain.cpp
eq-copilot/plugin/tests/MarkierungTestMain.cpp
eq-copilot/plugin/tests/SchemaTestMain.cpp
eq-copilot/plugin/tests/ShotTestMain.cpp
eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp
eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp
eq-copilot/plugin/tests/Sonde013DynamicsTest.cpp
eq-copilot/plugin/tests/Sonde013EventWireTest.cpp
eq-copilot/plugin/tests/Sonde013ExperimentGoldenTest.cpp
eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp
eq-copilot/plugin/tests/Sonde014IntentTest.cpp
eq-copilot/plugin/tests/SondeNullTestMain.cpp
eq-copilot/plugin/tests/StateMigrationTestMain.cpp
eq-copilot/plugin/tests/TransactionTestMain.cpp
```

### Werkzeugquellen — 11 Dateien

```text
tools/beweise.ps1
tools/dirigent/install-aufgaben-registrieren.ps1
tools/eq-copilot/erzeuge_p4_korpus.py
tools/eq-copilot/erzeuge_p5_korpus.py
tools/eq-copilot/erzeuge_state_fixtures.py
tools/eq-copilot/pruefe_installer_manifest.py
tools/eq-copilot/pruefe_kern_identitaetsfrei.py
tools/eq-copilot/pruefe_p4_korpus.py
tools/fl/laufzeit.ps1
tools/fl/nulltest.py
tools/pruefung/codex-audit-lauf.ps1
```

### Ergänzende lesende Inventare

Die Suche `rg -a -l project_binding_id eq-copilot/fixtures/state/schema2` umfasst **acht Binärfixtures**, ohne Treffer; keine Behauptung eines deserialisierten Fixture-Laufs. Der zugehörige Generator und die Migrationstests sind oben als gelesene Quellen aufgeführt.

```text
eq-copilot/fixtures/state/schema2/aus-schema1-hub.bin
eq-copilot/fixtures/state/schema2/aus-schema1-post.bin
eq-copilot/fixtures/state/schema2/aus-schema1-pre.bin
eq-copilot/fixtures/state/schema2/aus-schema1-sensor.bin
eq-copilot/fixtures/state/schema2/dsp-v2-voll.bin
eq-copilot/fixtures/state/schema2/fremdes-major-3.bin
eq-copilot/fixtures/state/schema2/layout-v1.bin
eq-copilot/fixtures/state/schema2/main-intent-v1.bin
```

Aufruf-/Duplikatsuchen liefen mit `rg -n` über die betroffenen Produkt- und Testbäume; unter anderem vollständig über die Descriptor-Zuweisungen/`descriptor_setzen`-Aufrufer in `broker/src/coordinator/` und die betreffenden Verdrahtungstests. Ein Suchlauf wird nicht als vollständige Lektüre jeder durchsuchten Datei gezählt.

Zusätzlich gelesen: **vier Hilfsdateien**, ohne Schreibzugriff und ohne zusätzliche Produktanforderungen:

```text
C:/Users/phili/Projekte/Nakama/.agents/skills/nakama-plugin-quality/SKILL.md
C:/Users/phili/.codex/skills/unsafe-checker/SKILL.md
C:/Users/phili/.codex/skills/loop/SKILL.md
C:/Users/phili/AppData/Local/Temp/nakama-tiefenaudit-p10-2c3a4b0a806d74aeb22b7f01ac2927266bf38c2f-start.log
```

Der Qualitäts- und Unsafe-Leitfaden wurden innerhalb des lesenden Auftrags verwendet; die offene Loop-Arbeitsweise wurde nicht aktiviert.

### Nicht gelesene und fehlende Pfade

**Nicht gelesen:** übrige Dateien außerhalb der expliziten Liste und ihrer gezielten Suchkontexte, insbesondere `briefing-hub/`, `nimbalyst-local/`, fremde lokale Einstellungen, FL-Projektinhalte, ausführbare Artefakte, vollständige übrige Dependencybäume, geparkte Studien und Design-/Figma-Bestände. `docs/archiv/` und alte Laufprotokolle wurden nicht zur aktuellen Beglaubigung herangezogen. Der Startlog dient ausschließlich Modell-/Zeitprovenienz.

**Fehlende erforderliche Pfade: 0.** Alle zehn Populationsberichte und alle 59 urteilstragenden Quellenketten waren zugänglich. Kein ausgelassener Befund und kein fehlender Scope-Teil wird als geprüft ausgegeben.
