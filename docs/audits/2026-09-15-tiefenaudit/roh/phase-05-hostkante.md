URTEIL: DEFEKTE 3 · LÜCKEN 0 · HÄRTUNGEN 0 — Die Hostkante enthält zwei ungesicherte Übergänge zwischen State und Automation sowie eine inkonsistente Broker-Bindung im Bau- und Installationsablauf.

# Tiefenaudit 3 — Phase 05: Hostkante und Nahtstellen

- **Auftrag:** gebundener Prüfauftrag des Dirigenten, 15.09.2026, 22:55.
- **Datum:** 15.09.2026, Europe/Berlin.
- **Branch / HEAD vor und nach dem Lauf:** master / `441db29a7aa7c2d9460771f52cdb9a22b65b2dea`.
- **Modell / Effort:** `gpt-6-astra` / `max`, aus dem Startlog `%TEMP%/nakama-tiefenaudit-p05-441db29a7aa7c2d9460771f52cdb9a22b65b2dea-start.log`.
- **Start laut Startlog:** 22:55:42 CEST.
- **Dauer:** 31 Minuten 2 Sekunden bis zur Abschlussprüfung um 23:26:44 CEST (Startlog 22:55:42; einschließlich Bericht und Datei-/HEAD-Prüfung).
- **Beweisart:** ausschließlich Quellenprüfung, Dateiinventare, vorhandene Messdaten und lesender Hashvergleich. Keine Builds, Tests, Hostläufe, Installationen oder Commits. Vergangene Testberichte dienen als Zusagen und Gegenbelege; sie werden nicht als heutige Testergebnisse ausgegeben.
- **Arbeitsweise:** `daw-compatibility-guide` als lesende Fachhilfe; die konkreten Urteile folgen dem lokalen JUCE-8.0.9-Code und den Nakama-Verträgen. Skeptikerpass durch denselben Prüfer, keine Delegation.

Der Worktree war beim Einstieg fremd verändert: `tools/pruefung/tiefenaudit_phase.py`; untracked waren `.claude/settings.local.json`, `briefing-hub/`, der Phase-05-Auftragstext und `nimbalyst-local/`. Diese Pfade wurden nicht verändert. Das Startskript wurde nur zur Lokalisierung des Startlogs gelesen. Schreibmenge dieser Phase: dieser Bericht und genau ein angehängter Abschnitt in `BEFUNDE.md`.

## Befundtabelle

| ID | Schwere | Klasse | Kategorie | Ort | Befund | Beleg | Bricht | Vorschlag | Status |
|---|---|---|---|---|---|---|---|---|---|
| T3-05-01 | high | K4 | DEFEKT | eq-copilot/plugin/sonde/SondeProcessor.cpp:1047; eq-copilot/plugin/sonde/SondeProcessor.cpp:1296; eq-copilot/plugin/sonde/SondeProcessor.cpp:1388; eq-copilot/plugin/sonde/SondeProcessor.cpp:1421 | Ein vor dem State-Reload eingetroffener, noch nicht abgeholter Hostwert überschreibt nach dem Laden den wirksamen DSP-Zustand. Der Ladestart leert das AutomationOverlay, lässt aber die 112 Mailboxwerte und Ereigniszähler stehen; der Hostparameter-Abgleich unterdrückt seinen eigenen Listener und ersetzt den alten Mailboxwert deshalb nicht. Hostanzeige und gespeicherter Hash können zum geladenen Stand passen, während Audio wieder dem alten Automationswert folgt. | Quellherleitung: vorbereitete Sonde mit eq_enabled=true und output_trim_db=+3 speichern; Host schreibt -9, Kontrolltakt steht noch aus; dieselben gespeicherten Bytes in dieselbe Instanz laden; nächster dspKontrollTakt übernimmt -9 aus hostWert. Ladestart löscht nur das Overlay (NakamaTransaktion.cpp:234); M-84-Test verwendet eine neue Instanz (TransactionTestMain.cpp:2183). Kein Lauf ausgeführt. | „Der Reload rekonstruiert **denselben `state_hash`** und innerhalb numerischer Toleranz denselben Audioausgang.“ (SONDE-015 M-84, docs/beweise/SONDE-015.md:1162). | Die Hostmailbox mit dem Ladestart generationstreu abgrenzen und den Recall in derselben Instanz mit einem vor dem Laden ausstehenden Hostwert prüfen. | offen |
| T3-05-02 | high | K4 | DEFEKT | eq-copilot/plugin/sonde/SondeProcessor.cpp:1047; eq-copilot/plugin/sonde/SondeProcessor.cpp:1202; eq-copilot/plugin/sonde/SondeProcessor.cpp:1421; eq-copilot/plugin/state/NakamaTransaktion.cpp:345; eq-copilot/plugin/dsp/DspProgramm.cpp:173 | Ein unbekanntes State-Major bleibt zwar mit Originalbytes schreibgeschützt, lässt sich danach über Hostautomation hörbar aktivieren. Die Transaktions-API prüft nurLesen; der Hostmailbox-Konsum und die wirksame DSP-Publikation prüfen es nicht. eq_enabled und output_trim_db sind nicht topologisch und gelangen auch bei deaktivierter samplegenauer Automation ins Programm. | Quellherleitung: fremdes-major-3.bin laden; über die vorhandenen Hostparameter eq_enabled=1 und output_trim_db=+6 schreiben; Kontrolltakt konsumiert beide Werte und veröffentlicht einen rechnenden DSP-Satz. DspProgramm.cpp:176 aktiviert EQ, :185 berechnet den Trimfaktor; DspKern.cpp:636 sperrt nur bei deaktiviertem EQ/Bypass. M-92-Test prüft neutrale Committed-Werte und abgelehnte Transaktionen, keine nachfolgende Hostautomation (TransactionTestMain.cpp:2008). Kein Lauf ausgeführt. | „Er bleibt **read-only mit Originalbytes** wie heute: audio-neutral, keine Bindung, `getStateInformation` liefert die Bytes unverändert zurück.“ (SONDE-015 M-92, docs/beweise/SONDE-015.md:1175). | Bei read-only jede wirksame Hostautomation neutral halten und das Laden des Fremd-Majors mit anschließendem Enable-/Trim-Automationsversuch samt Audio- und Originalbytevergleich prüfen. | offen |
| T3-05-03 | high | K4 | DEFEKT | eq-copilot/plugin/CMakeLists.txt:56; tools/beweise.ps1:886; tools/beweise.ps1:895; tools/fl/laufzeit.ps1:408; tools/eq-copilot/pruefe_installer_manifest.py:1150; eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:309 | bekannt NAK-127: Neu ist die veraltete einkompilierte Broker-Bindung nach dem Nachziehen der Manifest-Hashes. Der Bau erstellt die Plugins vor dem Release-Broker; der Installationsarm friert anschließend mit --hashen die neuen Artefakte ein und installiert sie ohne erneuten Pluginbau. Bei veränderten Brokerbytes enthalten beide Plugins weiterhin den alten erwarteten SHA-256, obwohl Installation und Dateivergleich „aktuell“ melden. Gen verweigert den Spawn; auch die Serverauthentisierung benutzt den alten Pin. | Lesende Kettenherleitung: Manifest/Broker H0 → CMake bindet H0 in BrokerInstallBinding.h → legitimer Broker-Neubau ergibt H1 → --hashen schreibt H1 und die vorhandenen Bundlehashes → Installation kopiert passende Dateien → Plugin vergleicht H0 mit H1 und erhält hashFalsch. --hashen verlässt _lauf vor [4c] (pruefe_installer_manifest.py:1650); CMAKE_CONFIGURE_DEPENDS wirkt erst beim nächsten Bau. Aktuell sind lokaler Header und Brokerdatei gleich CA857330…D2B306; behauptet wird der nächste Änderungsfall, kein aktuell gemessener Installationsausfall. | „generierter Produktheader entspricht Pfad, Hash und Thumbprint des Manifests“ (bestehende A17-Prüfung, tools/eq-copilot/pruefe_installer_manifest.py:1382). | Broker bauen und dessen Pin einfrieren, danach beide Plugins bauen, erst dann Bundlehashes einfrieren und die finale Startbindung vor Installation prüfen. | offen |

## Herleitung und Skeptikerpass der überlebenden Befunde

### T3-05-01 — Der Reload trennt Committed-State und ausstehende Hostwerte nicht gemeinsam

Die überprüfte Population sind alle **112** Hostparameter: derselbe Listener, dieselben vier Mailboxbestandteile und derselbe Konsumweg. Es handelt sich nicht um einen nur für Band-Gain geltenden Sonderfall.

1. `parameterValueChanged` speichert normierten Wert, Ereigniszähler und Offen-Flag (`SondeProcessor.cpp:1296`).
2. `setStateInformation` setzt den neuen State und ruft `Transaktionskern::ladestart`; dieser leert Preview und AutomationOverlay (`NakamaTransaktion.cpp:234`). Die Prozessor-Mailbox aus `SondeProcessor.h:541` bleibt bestehen.
3. `hostParameterAbgleichen` setzt den geladenen Hostwert mit `setValueNotifyingHost`. Die lokale Herkunftswache `abgleichTiefe` unterdrückt dabei den Listener (`SondeProcessor.cpp:76`, `:1299`, `:1390`). Gerade deshalb ersetzt diese Synchronisation den ausstehenden alten Mailboxwert nicht.
4. `dspKontrollTakt` erkennt dessen ungelesenen Ereigniszähler, schreibt das Overlay und veröffentlicht es (`:1425`, `:1442`). Committed-State, Revision und Hash bleiben dabei unverändert.

Es genügt die erlaubte Reihenfolge **Hostwert → State-Recall → Kontrolltakt**. Ein gleichzeitiger zweiter Hostaufruf ist nicht erforderlich. Der Produktworker arbeitet zeitversetzt (`:541`); der vorhandene Testzugang `kontrollTaktFuerTest` (`SondeProcessor.h:263`) erlaubt später eine deterministische Wache ohne Schlafheuristik. Nach der Bankübernahme ist der wirksame Output-Trim -9 dB, obwohl der geladene und zum Host synchronisierte Wert +3 dB lautet.

**Gesuchte Gegenbelege:** M-84/B7 prüft tatsächlich Audio, lädt aber in eine frisch erzeugte Instanz (`TransactionTestMain.cpp:2171` bis `:2189`). Die Prozessor-Automationsfälle (`:2030`) treiben Werte und Kontrolltakt, kombinieren sie nicht mit einem dazwischenliegenden Recall. Die Absicht, Hostwerte bei `reset()` zu erhalten (`SondeProcessor.cpp:301`), betrifft das Ende der Audiohistorie; sie rechtfertigt keine Automation aus dem Zustand vor einem **State-Ladestart**. NAK-291(a) betrifft den Reset-Thread. T3-01-05 betrifft die allgemeine Wanduhrkadenz, nicht diese Überschreitung der Ladegrenze.

### T3-05-02 — Read-only schützt die Transaktion, aber nicht den hörbaren Automationspfad

Der State-Lader setzt bei einem fremden Major einen neutralen DSP-Satz und hält die Originalbytes. Das ist an `SondeProcessor.cpp:1047` korrekt. Auch die Transaktions-API verweigert Änderungen ausdrücklich mit `schreibgeschuetzt` (`:1202`).

Die Hostparameter bleiben jedoch erreichbar. `parameterValueChanged` nimmt die Werte an; der Kontrolltakt kennt kein `nurLesen`. Die Parametertabelle markiert Output-Trim und EQ-Enable als nicht topologisch (`NakamaParameter.cpp:51`, `:79`). Deshalb übernimmt `Transaktionskern::wirksam` beide trotz `samplegenau=false` (`NakamaTransaktion.cpp:345`). Der Programmbauer setzt `eqEngagiert` und den Trimfaktor (`DspProgramm.cpp:173`, `:176`, `:185`). Nach dem vorgesehenen Übergang verändert +6 dB einen gewöhnlichen endlichen Eingang mit einem Faktor von ungefähr 1,995.

**Gesuchte Gegenbelege:** Der M-92-Prozessortest (`TransactionTestMain.cpp:2008`) prüft neutralen Committed-State, Transaktionsverweigerung und bytegleichen Save. Er schreibt danach keine Hostparameter. Die Read-only-Prüfung in `v3Status` sperrt nur berichtete Bindung und State-Hash (`SondeProcessor.cpp:1110`), nicht den DSP. Die Originalbytes werden weiterhin korrekt erhalten (`:1012`); der Befund behauptet daher keinen Byteverlust. Eine Host-Enable-Geste hebt laut M-92 die Neutralitätszusage für einen unbekannten State nicht auf. Im Register fand sich kein entsprechender Automation-nach-Fremd-Major-Fall.

### T3-05-03 — Drei einzeln korrekte Hashvergleiche ergeben noch keine zusammenpassende Auslieferung

Die konkrete Ablaufkette besteht aus **zwei Plugin-Artefakten plus einem Broker**:

1. CMake liest den Broker-Pin aus dem aktuellen Manifest und erzeugt `BrokerInstallBinding.h` (`plugin/CMakeLists.txt:39` bis `:89`). Gen benutzt ihn zum Spawn und zum Connect; Probeeq zum Connect (`PluginProcessor.cpp:79`, `:223`; `SondeProcessor.cpp:63`).
2. Der kanonische Bau erstellt die C++-Ziele bei `tools/beweise.ps1:886`, den Release-Broker erst bei `:895`.
3. Der Laufzeitarm ruft danach `--hashen` und unmittelbar die erhöhten Installations-/Prüfaufgaben auf (`tools/fl/laufzeit.ps1:402`). Der Hashzweig ersetzt alle Artefakthashes und beendet sich, bevor die Bindungsprüfung [4c] läuft (`pruefe_installer_manifest.py:1150`, `:1163`, `:1665`).
4. Der Installer prüft und kopiert passende Dateien. Die Plugins enthalten trotzdem den Pin aus Schritt 1. Der Laufzeitvergleich lehnt H1 gegen H0 ab (`BrokerLifecycle.cpp:309`); der Launcher geht in Cooldown (`:890`). Derselbe erwartete Hash schützt die Authentisierung eines schon laufenden Servers.

**Gesuchte Gegenbelege:** `CMAKE_CONFIGURE_DEPENDS` ist vorhanden und korrekt. Es hilft nur, wenn nach der Manifeständerung erneut gebaut wird. [4c] vergleicht den generierten Header mit dem Manifest zum Zeitpunkt seines Aufrufs; es schließt die spätere Änderung durch `--hashen` nicht ab. A18 arbeitet mit synthetischen Bundle-/Brokerbytes und prüft den Installer-Gegenpfad, keine ausgeführte Plugin-Startbindung. NAK-93, NAK-94 und NAK-127 wurden gegengelesen: Der Broker wird inzwischen gebaut; weiche Hashabweichungen im normalen Kanon sind absichtlich. Die neue Aussage ist, dass **Nachhashen allein eine zuvor passende Auslieferung auseinanderziehen kann**.

**Heutiger lesender Kontrollpunkt:** generierter Header und `broker/target/release/eqcop-broker.exe` nennen beziehungsweise ergeben beide SHA-256 `CA85733027ACCEF5E7D98EA4FBCFFD12E7BCFECAE3DA3E2284834D759AD2B306`. Dieser Audit behauptet keine aktuell falschen installierten Bytes. Voraussetzung des Befunds ist ein legitimer Neubau, der Brokerbytes verändert; keine Sabotage eines Prüfers oder Manifests.

## Population: Host-Annahmen und gemessene Fähigkeiten

### Alle zehn Capability-Einträge

Quelle: `eq-copilot/identity/host-capabilities-fl-v1.json:24`; ausgesendete Zehnerliste: `plugin/core/ipc/controlclient/Intern.h:388`. Die Datei wird nicht als Befundfläche bewertet.

| Eigenschaft | Messung und Grenze | Nutzung / Umgang im Code |
|---|---|---|
| host_context_presence | supported; Termin B meldet Kontext in 259.298/259.298 Blöcken. | Gen und Sonde kopieren weiterhin Anwesenheit und einzelne Gültigkeitsbits; fehlender Kontext wird nicht zu Zeit 0 umgedeutet. `PluginProcessor.cpp:626`; `SondeProcessor.cpp:463`. |
| project_time_samples | supported; Sprünge vor/zurück, Loop und Stop/Play gemessen. | Zeitstempel und Quarantäne behandeln die gelieferten Werte; Gen besitzt zusätzlich überlaufsichere Intervallgrenzen. `PluginProcessor.cpp:758`; `SondeProcessor.cpp:470`. |
| sample_accurate_automation | unsupported; 83.303 Punkte, höchstens einer pro Block, Offset 0; Blockgrößen 1–4.096. | Sonde setzt ausdrücklich false (`:151`); 32 topologische Hostfelder werden nicht ins wirksame Overlay übernommen (`NakamaTransaktion.cpp:345`). Die verspätete Wanduhrübernahme bleibt bekannt T3-01-05. |
| presentation_latency | unsupported; 3.924/4.410 Samples gemeldet, ein Wertwechsel verworfen; kein Impulsgolden. | Brücke transportiert Buswerte, Sonde stempelt Ein-/Ausgang 0 (`:488`). Das ist keine bewiesene PDC-Kompensation. NAK-43 bleibt bekannt. |
| aux_compare_pre | unsupported; Termin A hat zwei aktive Stereo-Nebenbusse, aber keine unterscheidbare L/R-Belegung und keine PDC-Last. | Im Produkt kein zusätzlicher Eingangsbus; der separate AuxSpike hat den Messbus. `SondeProcessor.cpp:132`; `AuxSpikeProcessor.cpp:105`. NAK-44 bleibt bekannt. |
| aux_priority_sidechain | unsupported aus derselben Messgrenze. | Im Produkt kein Host-Aux-Eingang; Parameterwahl ist vorhanden, aber kein Beweis eines PDC-synchronen externen Sidechains. Dieselben Buskonstruktoren; Topologieautomation ist gesperrt. |
| contribution_aux | unsupported, von keinem der vier Rohberichte belegt. | Gen deklariert nur den Hauptbus (`PluginProcessor.cpp:129`); Sonde erzeugt für post_fader_contribution keinen gültigen Runtime-Messpunkt (`:1132`). |
| float64_processing | unsupported; 259.298 Float-, null Double-Blöcke. | Gen/Sonde/AuxSpike implementieren nur Float. Nur das Messgerät HostProbe deklariert Double und besitzt einen Double-Callback (`HostProbeProcessor.h:181`, `.cpp:181`). |
| binary_telemetry | unsupported; kein Hostbit. | Bekannte Inkonsistenz zwischen Capability und vorhandenem P2-Pfad: T3-03-10. Keine zweite Befund-ID. |
| remote_control | unsupported; kein Hostbit. | Wird unsupported gesendet; die lokalen Sonde-Hostparameter sind davon getrennt. Kein Termin-A/B-Beleg für Pairing/HMAC/Lease behauptet. |

Zusätzlich geprüft: **host_channel_context** außerhalb dieser zehn Wire-Capabilities (`host-capabilities-fl-v1.json:17`). `SondeProcessor::updateTrackProperties` akzeptiert einen brauchbaren Hostnamen nur vom Message-Thread (`:497`); ohne Namen bleibt der Zustand ausdrücklich ungemeldet. Die Messdateien belegen keine Kanal-ID, aus der Produktcode eine stabile Bindung ableiten dürfte.

Die beiden Termin-B-Berichte sind **kumulativ**, nicht zwei addierbare Messreihen: 75 beziehungsweise 98 gespeicherte Ereignisse, jeweils über alle **22 Ereignisarten** inventarisiert; 7 Gültigkeitsfelder. Maßgeblich ist der spätere Bericht (`host-probe-20260822-132644.json:7`, `:50`, `:63`, `:104`). Die beiden Termin-A-Berichte umfassen je **3 Busse**, zusammen **6 Busdatensätze**; beide bei 44,1 kHz und 2.048 vorbereiteten Samples.

**Unbewiesene Hostannahmen, ohne sie zu neuen Defekten umzubenennen:**

- `reset()` außerhalb des Audiothreads: NAK-291(a), Wrapper `setProcessing(false)` ruft direkt `reset()` (`juce_audio_plugin_client_VST3.cpp:3612`), Sonde sperrt zwei Schlösser (`:306`). Termin A/B erfasst keine Thread-ID.
- Automationszustellung und State-Recall werden nicht gemeinsam gemessen. Die Quelle setzt eine vollständig abgegrenzte Mailbox voraus; T3-05-01 widerlegt diese Abgrenzung.
- Neutralität nach unbekanntem Major ist bisher nur ohne nachfolgende Hostautomation geprüft; T3-05-02 zeigt die fehlende Sperre.
- Smart Disable bei geschlossenem Editor, reale FL-Sampleratenwechsel, erneutes Prepare ohne Release und reale Hostbypass-Übergänge sind in A/B nicht nachgewiesen. Ein gewöhnlicher VST3-Vertrag oder ein C++-Test ist kein FL-Messwert.
- Render ist gemessen (2.587 Offline-Blöcke), aber das misst weder DSP-Gleichheit unter Automation noch den Thread von `setNonRealtime`. Die betreffenden Defekte stehen bereits als T3-01-03/04/05.

## Population: Prozessoren, Busse, Parameter und Identität

### Vollständige Menge der angefragten Host-Eintritte

Gezählt wurden Deklarationen je Prozessor, nicht zusätzlich ihre Definitionen oder Kommentarvorkommen. Vier konkrete Prozessoren: Gen, Probeeq/Sonde, HostProbe, AuxSpike.

| Muster | Population und Fundstellen |
|---|---|
| prepareToPlay / releaseResources | Je **4**. Gen: `src/prozessor/Hostbruecke.cpp:34`, `src/PluginProcessor.h:112`; Sonde: `sonde/SondeProcessor.cpp:232`, `:271`; HostProbe: `hostprobe/HostProbeProcessor.cpp:97`, `.h:176`; AuxSpike: `spike/AuxSpikeProcessor.cpp:117`, `.h:89`. |
| reset | **1 eigener Override**, Sonde `.cpp:287`; die anderen drei erben JUCE. Gen ist bereits T3-01-10. |
| isBusesLayoutSupported | **4**: Gen `Hostbruecke.cpp:149`, Sonde `:311`, HostProbe `:108`, AuxSpike `:124`. Alle erlauben gleiches Mono/Stereo für Main; vier Kanäle und ungleiche Mainlayouts werden abgelehnt. |
| Buskonstruktoren | **4**: Gen `PluginProcessor.cpp:129`, Sonde `:132`, HostProbe `:85`, AuxSpike `:105`. **4 Main-Eingänge + 4 Main-Ausgänge + 2 optionale Aux-Eingänge**; nur der Messspike besitzt Aux. |
| processBlock | **4 Float + 1 Double**: Gen `:609`, Sonde `:335`, HostProbe `:174`/`:181`, AuxSpike `:149`. Aux liest über `getBusBuffer` jeden Kanal des jeweiligen Busses; kein gemeinsamer Zweikanal-Index über drei Busse. |
| getLatencySamples / setLatencySamples | Kein eigener Getter; **2 Setteraufrufe**, beide HostProbe mit **0** (`:94`, `:99`). Sonde prüft 0 beim Prepare (`:236`). Kein dynamischer Latenzsetter im Audio-Callback gefunden. |
| getTailLengthSeconds | **4 Overrides, alle 0**: Gen `.h:122`, Sonde `.h:174`, HostProbe `.h:183`, AuxSpike `.h:100`. |
| processBlockBypassed / getBypassParameter | **0 eigene Overrides** in allen vier Klassen. JUCE nimmt den separaten Standardbypass (`juce_audio_plugin_client_VST3.cpp:3904`); fehlende Übergangssemantik ist schon T3-01-09. |
| acceptsMidi / producesMidi | Je **4**, alle false: Gen `.h:120`, Sonde `.h:171`, HostProbe `.h:184`, AuxSpike `.h:97`. CMake setzt für alle vier VST3-Ziele NEEDS_MIDI_INPUT/OUTPUT false. |
| Programme | **4 × 5 = 20 Methoden**: ein Programm, Index 0, Setzen/Umbenennen ohne Wirkung. Namen Standard / leer / Messung / leer. Gen `.h:124`, Sonde `.h:182`, HostProbe `.h:191`, AuxSpike `.h:102`. Kein zusätzlicher Bank-/Preset-Dateilader in diesen Schalen. |
| getStateInformation / setStateInformation | Je **4**: Gen `prozessor/State.cpp:69`/`:140`; Sonde `.cpp:990`/`:1017`; Messgeräte je zwei leere Overrides (`HostProbeProcessor.h:197`, `AuxSpikeProcessor.h:108`). |
| getName / getUniqueId | **4 eigene getName**, **0 eigene getUniqueId**: Gen `.h:119`, Sonde `.h:167`, HostProbe `.h:173`, AuxSpike `.h:96`. VST3-Identität kommt aus Zieldefinition/Factory, nicht aus einem eigenen UniqueId-Callback. |
| Brückenempfänger / Offline / TrackProperties | **3 Senken**: Gen, Sonde, HostProbe. **1 setNonRealtime-Override** (Sonde `:1287`), **1 updateTrackProperties-Override** (Sonde `:497`). |

**Grenzfälle:** Gen und Sonde begrenzen Prepare-Raten auf endlich, positiv und höchstens 768.000 Hz. Prepare fordert einen neuen Analyseanlauf an; eine gültige erneute Sonde-Vorbereitung baut DSP-Puffer/Publikation neu. Sonde-Release gibt DSP-Ressourcen frei; Reset beendet nur Audiohistorie. Ein ungültiges Prepare nach gültigem Prepare ohne Release wurde nicht als neuer Fehler behauptet: Die reale FL-Aufrufsequenz und ihr Verhalten sind ungemessen. Der lokale Wrapper trennt Setup (`:3592`), Aktivieren/Deaktivieren (`:2833`) und Reset (`:3612`).

Nullblöcke überspringen bei Gen/Sonde die Audiorechnung; die Brücke kann trotzdem Parameter-Flushes beobachten. Bei übergroßen Blöcken ist die DSP-Verarbeitung vom Analyse-Tap getrennt. Ein ungültiger Tap bei rechnendem Kern führt zu `verwirfOhneAudio(samples)`, nicht zur Analyse des bearbeiteten Ausgangs unter falschem Tapnamen (`SondeProcessor.cpp:403` bis `:453`). Die Zeitbuchführung läuft weiter. Das fehlende Wire-Zählerfeld bleibt NAK-291(b).

### Alle Parameter

Gen: **0 registrierte AudioProcessor-Hostparameter**; persistente Gen-Bedienänderungen gehen über seine State-Funktionen und `meldeHostDirty` (`Hostbruecke.cpp:160`). Probeeq: **112 Hostparameter**, erzeugt aus genau einer Tabelle (`SondeProcessor.cpp:86`). HostProbe: **1** Messparameter `testwert`, Bereich 0–1, Default 0,5 (`HostProbeProcessor.cpp:91`). AuxSpike: **0**. Standardparameter, die der JUCE-Wrapper selbst ergänzt, sind nicht als eigene Pluginparameter mitgezählt; sein Bypass ist oben erfasst.

| Parametermuster | Anzahl | Bereich / Typ / Default | Quelle |
|---|---:|---|---|
| v1.global.bypass | 1 | Bool, false | `NakamaParameter.cpp:49` |
| v1.global.input_trim_db / output_trim_db | 2 | -24…24 dB, 0 | `:50` |
| v1.global.width | 1 | 0…2, 1 | `:52` |
| v1.global.mono_bass_hz | 1 | 0…500, 0 | `:53` |
| v1.band.[0–7].enabled | 8 | Bool, false | `:62` |
| v1.band.[0–7].type | 8 | 6 Choices, bell; topologisch | `:63` |
| v1.band.[0–7].freq_hz | 8 | 20…20.000, 1.000 | `:64` |
| v1.band.[0–7].q | 8 | 0,15…24, 1 | `:65` |
| v1.band.[0–7].gain_db | 8 | -12…12 dB, 0 | `:66` |
| v1.band.[0–7].channel_mode | 8 | 5 Choices, stereo; topologisch | `:67` |
| v1.band.[0–7].dynamic_enabled | 8 | Bool, false; topologisch | `:68` |
| v1.band.[0–7].dynamic_range_db | 8 | -12…12 dB, 0 | `:69` |
| v1.band.[0–7].threshold_db | 8 | -60…0 dB, -20 | `:70` |
| v1.band.[0–7].attack_ms | 8 | 0,1…500 ms, 10 | `:71` |
| v1.band.[0–7].hold_ms | 8 | 0…500 ms, 0 | `:72` |
| v1.band.[0–7].release_ms | 8 | 5…5.000 ms, 100 | `:73` |
| v1.band.[0–7].sidechain_source | 8 | 3 Choices, none; topologisch | `:74` |
| v2.global.eq_enabled / mix / auto_gain | 3 | Bool false / 0…1 Default 1 / Bool false | `:79` |
| v2.band.[0–7].occupied | 8 zusätzlich | Bool false, persistent, **kein Hostparameter** | `:87` |

Summe: 5 + 8×13 + 3 = **112 exponierte**, dazu 8 interne Belegungsfelder = **120 Tabellenfelder**. Bool-/Choice-/Float-Normalisierung liegt in `SondeProcessor.cpp:95`; vier Muster benutzen Skew um den Vertragsdefault: freq_hz, q, attack_ms, release_ms (`:105`). Die Werte werden über `zelleAusHost` zurückgeführt; nichtendliche Hostwerte sind bereits im bestehenden E4-10-Test behandelt (`TransactionTestMain.cpp:2146`). Ein Hostwert ohne Geste ist absichtlich Overlay ohne Revision/Undo (M-81), ein Commit meldet Dirty außerhalb des Zustandsschlosses (`SondeProcessor.cpp:1221`). Laden meldet kein Dirty. Diese Trennung widerlegt keinen der beiden Reload-/Read-only-Befunde.

### Identität und Bundlegrenzen

- **3 reservierte Produktidentitäten, davon 2 aktiv und 1 stillgelegt:** main/Eqcp, active-probe/NkAc, passive-probe/NkPr. Alle **6 Component-/Controller-CIDs** bleiben reserviert; Hersteller Evna/evenacadia. Quelle: `identity/plugin-identities-v1.json`.
- **4 konkrete VST3-Ziele:** EqCopilot, NakamaProbeeq sowie die zwei Messziele EqCopAuxSpike/NkSp und EqCopHostProbe/NkHp. Sonde entsteht ausschließlich über `nakama_sonde_ziel` (`plugin/CMakeLists.txt:320`); das passive Suna-Ziel ist stillgelegt (`:368`). Messgeräte zählen nicht zur Installationspopulation.
- **4 Replace-V2-Definitionen, alle 0:** `CMakeLists.txt:152`, `:353`, `:737`, `:1205`. Hersteller-/Plugincodes der Produktziele kommen aus `NakamaIdentitaet.cmake`; dessen Viercodeprüfung steht bei `:219`, Export bei `:246`. Die vier Factorydateien erzeugen jeweils ihren Prozessor.
- Der im Auftrag genannte Name **„Nakama Studio“ ist am gebundenen HEAD kein Produkt-Bundlename**. Installiert werden die eingefrorenen `EQ-Copilot.vst3` und `Nakama Probeeq.vst3`. Das ist kein Umbenennungsbefund; NAK-30 bleibt die Grenze.
- **A14 misst den Kern, B1 die Zielidentitäten:** A14 sucht alle Manifestnadeln in ASCII/UTF-16, Viercode- und beiden CID-Byteordnungen (`pruefe_kern_identitaetsfrei.py:375`, `:409`), mit Gegenprobe im Gen-Bundle und Objektmengenprüfung (`:495`). B1 liest alle Produktziele, prüft CMake, moduleinfo und rechnet die CIDs mit JUCE nach (`IdentityTestMain.cpp:313`, `:419`, `:586`, `:701`).
- **Explizite Messgrenze:** Eine versehentliche Änderung nur des Gen-`getName()`-Rückgabewerts in der dünnen Prozessorschale würde weder eine Identitätsnadel im gemeinsamen Kern erzeugen noch zwingend CMake/moduleinfo ändern. Diese beiden Riegel sind kein dynamischer Test sämtlicher Host-Namensabfragen. Der heutige Rückgabewert stimmt mit der eingefrorenen Identität überein; kein neuer Defekt und kein Sabotageszenario.

## Population: Installer, Brokerstart und FL-Laufzeitarm

### Installer vollständig nach Eingangs- und Gegenpfaden

**29 Installerfunktionen** wurden inventarisiert und entlang Installieren/Prüfen/Rückweg gelesen:

- Pfade/Reparse: `Kanonischer-Pfad:103`, `Ist-GleichOderUnter:108`, `Fordere-GleichOderUnter:115`, `Fordere-Keine-Reparsepunkte:121`, `Loese-Pfad:136`.
- Journal/Abbruch: `Schreibe-JsonAtomar:143`, `Schreibe-Ergebnis:182`, `Abbruch:194`.
- Identitätszuordnung/Altlast: `Ident-Von:280`, `Bekannt-Von:285`, `Melde-StillgelegteAltlasten:389`.
- Ziele/Hash: `Ziel-Pfad:425`, `Innen-Pfad:454`, `Artefakt-Name:461`, `Hash-Von:476`, `Datei-Hash:480`.
- Mutation/Gegenakt: `Erzeuge-Ordnerkette:488`, `Entferne-Stand:499`, `Kopiere-Stand:510`, `Stelle-Gepruefte-SicherungWiederHer:520`.
- Ruhe/Form: `Fl-Laeuft:542`, `Ist-Sha256:546`, `Ist-TransaktionsId:550`.
- Sicherungen/Sperre: `Sicherungsordner-Fuer:554`, `Loese-GeschuetzteSicherung:566`, `Entferne-TransaktionsSicherungen:579`, `Bereinige-VerwaisteSicherungen:588`, `Entferne-LeereTransaktionsWurzeln:598`, `Sperre-GeschuetztenInstaller:609`.

Alle Zeilen dieses Verzeichnisses beziehen sich auf `eq-copilot/install/Install-Nakama.ps1`. Dazu kommen genau **3 Hashfunktionen** in `NakamaOrdnerHash.ps1:47`, `:58`, `:108`: ASCII-Pfadregel, ordinal sortierter rekursiver Bundlehash einschließlich versteckter Dateien, Broker-Dateihash. Reparsepunkte werden abgelehnt.

**3 Installationsartefakte** (2 Bundleordner, 1 Brokerdatei), **3 erhöhte Aufgaben** (installieren/pruefen/rueckweg; `install-aufgaben-registrieren.ps1:47`) und **7 Journalzustände** (VORBEREITET, OK, KOMPENSATION, ERROR_TEILSTAND, RUECKWEG_AKTIV, ERROR_RUECKGEROLLT, RUECKWEG) wurden verfolgt. Aufgaben umgehen weder Hash-, FL-Ruhe- noch NAK-41-Riegel.

Vorwärts werden Quellhashes geprüft (`:910`), FL-Prozesse einschließlich fensterloser FL64/FL/FL Studio verweigert (`:542`, `:966`), Sicherungen und geschützter Transaktionsanker vor Zielmutation erstellt. Der Rückweg prüft Manifestbindung und Sicherung vor dem ersten Gegenakt. Partielle Vorwärts- und Rückwärtskopien bleiben über die gesetzten Mutationsmarker rekonstruierbar. A18 besitzt dafür konkrete Gegenfälle (`pruefe_installer_gegenpfad.py:541`, `:575`, `:600`); sie wurden hier **gelesen, nicht ausgeführt**. Der neue T3-05-03 liegt zwischen Bau und Installation, nicht in einer erneuten Behauptung des bereits geschlossenen Transaktionsfehlers.

Das Brokerziel lautet `C:/Program Files/evenacadia/Nakama/eqcop-broker.exe`; der lokale generierte Header stimmt damit überein. Nur Gen darf über den Lebenslauf-Gatewert einen Broker starten. Es gibt keinen zweiten Spawnpfad in Sonde; deren Verbindung und Gen verwenden die manifestgebundene Serverprüfung. Reconnect und Start sind Workerarbeit; die untersuchten `processBlock`-Funktionen enthalten keinen Brokerstart.

### Was tools/fl misst — und was daraus nicht folgt

Die **5 Dateien** umfassen 55 Top-Level-Funktionen in `laufzeit.ps1`, 107 in `szenario.py`, 29 in `nulltest.py`, 1 in `selbsttest.py` und die LIES-MICH-Datei. Alle 192 Funktionsdeklarationen wurden inventarisiert; die Host-/Installationspfade, Messdefinitionen und zugehörigen Gegenfälle wurden gezielt gelesen. Das ist kein Vollreview aller Szenario-Interpreterfunktionen.

- Der Laufzeitarm misst installierte Artefaktstände, Startbereitschaft, Diagnoseantworten, Szenarioerwartungen und Renderdateien. A35 führt seine drei Selbsttestteile gegen Attrappen aus (`selbsttest.py:28`), nicht gegen einen realen FL-Host.
- `nulltest.py:194` trennt samplegleichen Vergleich, Versatz, Kettenpegel und echte Abweichung. Ein Rendervergleich ist keine Messung des Callback-Threads, der Plugin-Dirty-Meldung oder eines live erfolgten Bypass-Übergangs.
- **Boot-Marke / Initialisierungsfenster:** `laufzeit.ps1:650` verlangt eine neue Controller-Boot-Marke und danach 8 Sekunden Schonfrist; bei fehlender Marke gilt ein gesonderter Rückfallweg. Das betrifft das Messwerkzeug, nicht `prepareToPlay` im Plugin.
- **Fensterlose Restprozesse:** Laufzeitarm `:338`/`:361` unterscheidet Hauptfenster über TFruityLoopsMainForm; der Installer benutzt Prozessnamen und verweigert auch ohne Fenster. Keine Produkt-Audioabhängigkeit.
- **MIDI 60 = C5:** betrifft FL-Bedienung/Fixtures. Alle vier Prozessoren lehnen MIDI-Eingang und -Ausgang ab; im untersuchten Audiopfad existiert keine MIDI-Notennamenumrechnung.
- **Paint-FPS / Demo:** Die Host- und Telemetriemessung wurde nicht aus einem Editor-Timer oder Demoanzeigewert abgeleitet. HostProbe-Timer wurde ausschließlich als Aufrufer seines Messstands gelesen.

## Widerlegte Verdachte

Hier bedeutet „bekannt“: Die **Neuheit** wurde widerlegt, nicht der bestehende Befund. Diese Liste enthält **12** verworfene neue Befundkandidaten.

1. **Sonde akzeptiert weiterhin vier Mainkanäle.** Widerlegt durch `SondeProcessor.cpp:323` und die vollständige Buslayout-Gegenmatrix `SondeNullTestMain.cpp:794`; Gen, HostProbe und AuxSpike besitzen dieselbe Mono-/Stereo-Grenze. NAK-283 F04 ist an dieser Stelle umgesetzt.
2. **AuxSpike liest Aux-Kanäle über den zweikanaligen Mainindex.** Widerlegt: `AuxSpikeProcessor.cpp:149` benutzt Busbuffer und iteriert deren eigene Kanalzahl. Seine zwei Nebenbusse sind keine vierkanalige Mainverarbeitung.
3. **Ungültiger Tap friert die Sonde-Zeit ein oder wird still als richtiger Tap ausgegeben.** Widerlegt durch `SondeProcessor.cpp:434` bis `:453`: Passthrough-Rückfall nur für ruhenden Committed-Passthrough, sonst gezählter Verlust ohne Audio. Das nicht mitgesendete Ursachenfeld ist bereits NAK-291(b).
4. **Host-Reset löscht in der Sonde das Programm oder lässt die alte DSP-Historie laufen.** Widerlegt durch `SondeProcessor.cpp:287` und `SondeNullTestMain.cpp:235`. Der ungemessene Reset-Thread ist bekannt NAK-291(a); der fehlende Gen-Reset bleibt T3-01-10.
5. **Ein neuer allgemeiner Bypass-/Offline-/Automationskadenzbefund.** Neuheit widerlegt: JUCE-Standardbypass, Offline-Sperre, Preview-Ausklang und Wanduhrkonsum sind T3-01-03/04/05/09. T3-05-01 ergänzt ausschließlich die State-Ladegrenze.
6. **Gemeldete Buslatenzen oder zwei Auxberichte seien bereits PDC-/Kanalreihenfolgenbeweise.** Widerlegt durch die Rohdaten und ihre ausdrücklichen Grenzen: keine PDC-Last, identische L/R-Impulse; unsupported bleibt im Wirebericht erhalten (`controlclient/Intern.h:396`). Bekannt NAK-43/44.
7. **Die Produkte behaupten gemessene Double-Verarbeitung.** Widerlegt: nur HostProbe hat den Double-Override; Gen/Sonde/AuxSpike nicht. Das Capabilitybit bleibt unsupported.
8. **Jeder Automationspunkt müsse Dirty, Undo und persistente Revision erhöhen.** Widerlegt durch die ausdrückliche Overlay-Regel M-81 und `TransactionTestMain.cpp:2099`. Persistente Transaktionen melden Dirty separat (`SondeProcessor.cpp:1228`).
9. **Legacy-Bundlenamen oder das reservierte NkPr seien eine Identitätsregression.** Widerlegt durch die eingefrorene Identitätsquelle, die Stilllegungsregel und B1 (`IdentityTestMain.cpp:351`). Kein Produkt-Bundlename „Nakama Studio“ am geprüften HEAD; kein Umbenennungsauftrag.
10. **Eine halb kopierte Installation oder fehlende Sicherung werde als erfolgreicher Rückweg behandelt.** Widerlegt durch den geschützten Journalplan und Vorprüfungen; A18 deckt partielle Vorwärts-/Rückwärtskopien mit bytegleichem Restore ab (`:575`, `:600`). Eine andere Manifestgeneration wird absichtlich verweigert (`Install-Nakama.ps1:686`); kein stiller Erfolg.
11. **Exit 0 von -Pruefen genüge dem Laufzeitarm trotz abweichender Dateien.** Widerlegt: `laufzeit.ps1:419` wertet die Artefaktzeilen aus und verwirft ABWEICHEND/nicht installiert. Das ist getrennt vom unentdeckten kompilierten Pin aus T3-05-03.
12. **HostProbe-Messstand und Ereigniszähler lieferten neue Nebenläufigkeitsbefunde.** Neuheit widerlegt über **NAK-189**, konkret `docs/beweise/roh/G4-creview-parts-9f30008/unit-07.md:6` und `:8`: vorzeichenbehafteter Ringzähler und nichtatomarer Seqlock-Messstand stehen bereits dort. `HostProbeTestMain.cpp:588` prüft nur Plausibilität; `:641` erklärt selbst, dass damit keine Tearing-Freiheit bewiesen ist. Hier kein zusätzlicher Delta-Befund.

## Produktfragen

**Keine neue Produktfrage.** Für die drei Befunde existieren bereits konkrete Zusagen. Die offenen Hostmessungen und bekannten Produktgrenzen wurden nicht durch neue Sollentscheidungen ersetzt.

## Nicht geprüft

- Keine heutige Laufzeitmessung in FL Studio; keine Aussage über aktuell hörbares Verhalten oder aktuell installierte Bundles. Keine Ausführung von A14/A17/A18/A35, B1/B7 oder sonstigen Tests.
- Keine Betriebssystem-Aufgaben abgefragt/gestartet, keine reale Installation oder Wiederherstellung, kein Broker auf irgendeiner Pipe gestartet. Authenticode wurde an der Quelle betrachtet, nicht neu verifiziert.
- Keine vollständige erneute Prüfung des Rust-Coordinators, seines Stores oder aller P0/P1/P2-Akzeptanzwege. Sonde-Join/Session/Revision wurden an den lokalen Hello-/Status-Produzenten und bestehenden Zusagen verfolgt. Die übrigen Vertrags- und Lebenslauffehler stehen in Phase 02–04.
- Ein State-Recall innerhalb einer laufenden Sonde fordert anders als Prepare keine neue Analysegeneration an (`SondeProcessor.cpp:1017`; einziger `neustartAnfordern` bei `:242`). Der mögliche Transport alter Analysefenster unter einer neuen Bindung wurde **nicht bis zur brokerseitigen Annahme nachgewiesen** und deshalb nicht als vierter Befund gezählt. Ein vollständiger Interleaving-Beleg dieser Analyse-/Reconnect-Grenze bleibt aus.
- Keine Design-/Optikprüfung, keine UI-Abnahmen. Aus Editorcode wurde nur der technische Messstand-Aufruf gelesen.
- Die Termin-B-Ereignisarrays wurden vollständig geparst und nach Art gezählt, aber nicht jeder einzelne Ereignisverlauf erneut gegen die handschriftliche Uhr rekonstruiert. Die beiden Berichte sind keine unabhängigen Stichproben.
- `py -3.13 -c` für ein rein lesendes Inventar konnte den lokalen Store-Python nicht starten; das Inventar wurde mit PowerShell/`rg` erstellt. Kein notwendiger Quellpfad blieb deshalb ungelesen. Fehlversuche mit Shell-Wildcards beziehungsweise geratenen Dateinamen wurden durch explizite Pfade ersetzt.
- Keine allgemeine Fehlerfreiheitsgarantie: Die vollständige Musterpopulation ersetzt weder eine Hostmessung noch einen ausgeführten Regressionstest.

## Scope-Beweis

### Primärpopulation — exakt 35 getrackte Dateien

Ermittelt mit `git ls-files --` über die im Auftrag genannten Pfade, einschließlich des tatsächlichen Pfads `third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch`. Buildreste und ungetrackte Altinstaller wurden nicht in die Quellpopulation gemischt.

| Gelesener / durchsuchter Pfad | Dateien der Population | Art und Grenze der Lektüre |
|---|---:|---|
| `eq-copilot/plugin/src/PluginProcessor.cpp`, `.h`, `src/prozessor/{Analyse.cpp,Hostbruecke.cpp,Intern.h,Ipc.cpp,State.cpp}` | 7 | Alle angefragten Hostmethoden und ihre Aufrufstellen inventarisiert; Audio-, Restore-, Dirty-, Start- und Adresspfade quergelesen. Nicht sämtliche Assistenten-/Diagnosefunktionen vollständig gelesen. |
| `eq-copilot/plugin/sonde/{SondeProcessor.cpp,SondeProcessor.h,SondeFactory.cpp}` | 3 | Host-, State-, Parameter-, DSP-Kontroll-, Analyse- und IPC-Nähte; Testzugänge gezielt. |
| `eq-copilot/plugin/hostbridge/{NakamaHostBridge.h,NakamaBrueckeStand.h.in}` | 2 | Brückenfelder, Grenzen, Gültigkeit, Parameterpunkte, Buslatenzen und Standtemplate. |
| `eq-copilot/plugin/hostprobe/{HostProbeProcessor.cpp,HostProbeProcessor.h,HostProbeFactory.cpp,HostProbeEditor.cpp}` | 4 von 5 | Prozessor-/Messpopulation; Editor nur Timer-/Messstand-Aufruf, keine Optik. |
| `eq-copilot/plugin/spike/{AuxSpikeProcessor.cpp,AuxSpikeProcessor.h,AuxSpikeFactory.cpp}` | 3 von 5 | Alle Bus-, Impuls-, Reset- und Reportpfade. |
| `eq-copilot/identity/{plugin-identities-v1.json,host-capabilities-fl-v1.json}` | 2 | Vollständige Eintragsmengen als Kontext, keine Befundfläche. |
| `eq-copilot/install/{Install-Nakama.ps1,NakamaOrdnerHash.ps1,nakama-installer-v1.json}` | 3 | Alle 29+3 Funktionsdeklarationen und Installations-/Prüf-/Rückwegzweige; Manifest als Kontext. |
| `eq-copilot/plugin/CMakeLists.txt` | 1 | Alle vier VST3-Ziele, Produktmengen, Defines und Broker-Bindung; übrige Testzielblöcke nur Suche. |
| `eq-copilot/cmake/NakamaBruecke.cmake`; `third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch` | 2 | Patch-Gate, Quellhashbindung und alle Patch-Hunks. |
| `tools/fl/{laufzeit.ps1,szenario.py,nulltest.py,selbsttest.py,LIES-MICH.md}` | 5 | Vollständiger Funktionsindex; Host-/Installationsarm, Beweisdefinitionen und Gegenfälle gezielt. |

**32/35 Dateien mit Inhaltslektüre oder gezielter Quellensuche; 3/35 bewusst nicht gelesen:** `hostprobe/HostProbeEditor.h`, `spike/AuxSpikeEditor.cpp`, `spike/AuxSpikeEditor.h` (Optik/Editorlayout außerhalb des Auftrags). Die angefragten Prozessor-/Parameter-/Bus-/Installationsmuster besitzen in diesen drei Dateien keine zusätzliche Implementierung; Factory- und Prozessordeklarationen wurden gelesen.

### Direkte Abhängigkeiten und Skeptikerquellen — 25 weitere Quell-/Testdateien

- **1 CMake-Datei:** `eq-copilot/cmake/NakamaIdentitaet.cmake`.
- **7 IPC-Dateien:** `plugin/core/ipc/{BrokerLifecycle.cpp,BrokerLifecycle.h,BrokerInstallBinding.h.in,ControlClient.cpp,ControlClient.h,TelemetryClient.cpp,controlclient/Intern.h}`. Brokerprüfung/Startzustände und Telemetrieherkunft gezielt; ControlClient nur adress-/capabilitybezogene Suche.
- **1 Factory:** `plugin/src/PluginFactory.cpp`.
- **3 State-Dateien:** `plugin/state/{NakamaParameter.cpp,NakamaParameter.h,NakamaTransaktion.cpp}`, Parameterpopulation, Ladestart, Overlay und wirksamer Zustand.
- **2 DSP-Dateien:** `plugin/dsp/{DspProgramm.cpp,DspKern.cpp}`, Enable/Trim, Passthrough und Endlichkeitsübergänge.
- **6 Testdateien:** `plugin/tests/{TransactionTestMain.cpp,IdentityTestMain.cpp,SondeNullTestMain.cpp,HostProbeTestMain.cpp,DspGoldenTestMain.cpp,Sonde012ProjectReloadTest.cpp}`, relevante Fälle und Suchmengen, keine Ausführung.
- **5 Werkzeuge:** `tools/eq-copilot/{pruefe_kern_identitaetsfrei.py,pruefe_installer_manifest.py,pruefe_installer_gegenpfad.py}`, `tools/dirigent/install-aufgaben-registrieren.ps1`, `tools/beweise.ps1`, jeweils der benannte Beweis-/Bau-/Installationszweig. Keine Befunde aus absichtlich sabotierten Prüfquellen.

Die `plugin/…`-Kürzel in dieser Liste liegen unter `eq-copilot/`.

### Kontext und vorhandene Artefakte

- **22 Kontextdateien gezielt gelesen/abgeglichen:** `CLAUDE.md` vollständig; `docs/plugin-wissen.md`; `docs/gesundheit/{KONZEPT.md,abdeckungskarte.md,lebenslauf-audit.md}`; `docs/offene-punkte.md`; `docs/FL-Nakama-Sonden-Design-Entwurf.md`; relevante Matrizen in `docs/beweise/{SONDE-003.md,SONDE-011.md,SONDE-012.md,SONDE-015.md}`; Vorgängerübergabe; **alle 7 Dateien** in `docs/beweise/termin-a/` und `termin-b/`; G4-`unit-06.md`/`unit-07.md` für NAK-189; vorhandenes `BEFUNDE.md`. Register, Manifeste und Prosa ausschließlich als Kontext.
- **3 vorhandene Buildartefakte lesend:** lokaler gepatchter JUCE-VST3-Wrapper (Hostgrenzen), generierter `BrokerInstallBinding.h` (vollständig), `broker/target/release/eqcop-broker.exe` (nur SHA-256, kein Start).
- **2 Metadatenquellen:** Startlog außerhalb des Repos (nur gelesen) und `tools/pruefung/tiefenaudit_phase.py` zur Loglokalisierung. **1 Fachskill:** `C:/Users/phili/.codex/skills/daw-compatibility-guide/SKILL.md`, nur gelesen.

### Nicht gelesene / fehlende Pfade und Abschlussanker

Nicht gelesen wurden die drei oben ausdrücklich ausgeschlossenen Editordateien, übrige Produkt-UI, geparkte Studien, Archivdokumente und nicht benannte Repositoryteile. Breite Symbolsuche in Brokerquellen ist kein Rust-Vollreview.

**Fehlende notwendige Pfade: 0.** Der zunächst geratene Name `ProbeeqNullTestMain.cpp` existiert nicht; das tatsächliche Testziel benutzt `SondeNullTestMain.cpp`. Geratene Coordinator-Einzeldateien waren ebenfalls keine fehlenden Produktdateien; der Rust-Gesamtpfad blieb wie oben angegeben außerhalb des abgeschlossenen Prüfbeweises.

**Abschlussprüfung:** HEAD vor/nach identisch mit dem gebundenen SHA; Branch master. Nur die beiden zugelassenen Audit-Schreibziele wurden bearbeitet. Das unveränderte Bytepräfix von `BEFUNDE.md` umfasst **41.298 Bytes**, SHA-256 **4C65F81B1967FA58D8417183D1F9E18E7596CC55451CEAE2B61D761F9E2583B9**. Genau ein Phase-05-Abschnitt wurde angehängt; die drei Tabellenzeilen sind in Bericht und Anhang identisch. `git diff --check` blieb ohne Befund; der getrackte Anhang enthält 12 hinzugefügte und 0 entfernte Zeilen. Keine Stage-, Commit- oder Pushoperation.
