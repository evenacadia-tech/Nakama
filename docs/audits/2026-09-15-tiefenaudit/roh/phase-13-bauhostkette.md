URTEIL: DEFEKTE 3 · LÜCKEN 2 · HÄRTUNGEN 1 — Die beidseitig gelesene Bau- und Hostnachweiskette lässt Produktions-Pipezugriff, fremden Projektbesitz und verspätete MCP-Antworten an ihren bisherigen Nachweisgrenzen vorbei.

# Tiefenaudit 3 — Phase 13: Bau- und Hostnachweiskette

- Datum und Auftrag: 16.09.2026, 04:53 MESZ; Start 04:53:26 MESZ.
- Berichtsstand: 2026-09-16 05:33:00 MESZ; Dauer bis zum Berichtsstand: 39 min 34 s (einschließlich Berichtserstellung; Abschlussvergleich anschließend).
- Nakama: master, HEAD vor und nach dem Lauf `1b2faf9926b8236e5b9ef1cb85f2cac0e4071d9d`.
- Externe Gegenseite: C:/Users/phili/Projekte/fl-studio-mcp, evenacadia-local, HEAD vor und nach dem Lauf `01f72386bfc7eab507a40b9c96ef9e2e2a0265cc`.
- Prüfer: Codex, laut Sitzungsinstruktion GPT-6. Die konkrete Backend-Modellkennung und der Effort sind dieser Prüfersitzung nicht exponiert; hierfür bleibt das Startlog des Dirigenten maßgeblich. Keine erfundene Modell-/Effortangabe.
- Modus: eigener lesender Prüfer mit eigenem Skeptikerpass, keine Delegation. Quellenlesen, rg, lesende Git-Abfragen, Dateimetadaten, SHA-256 und statische Textauswertung. Kein Bau, Test, Kanon, FL-Start, uv-Aufruf, Controller- oder MCP-Server-Aufruf. Kein Stage, Commit oder Push.
- Angewandter Arbeitsrahmen: AGENTS.md und vollständig gelesene CLAUDE.md; nakama-plugin-quality nur für Population, Nahtstellen und Skeptiker. Der gebundene Prüfauftrag schließt dessen sonstige Ausführungs-/Fixschritte aus.
- Belegniveau: Die sechs Befunde sind Quellenherleitungen. Kein neuer Fehler wurde in einem laufenden Host provoziert. Statische Feld-/Hashvergleiche sind ausdrücklich kein grüner Build- oder Laufzeitnachweis.

## Befundtabelle

| ID | Schwere | Klasse | Kategorie | Ort | Befund | Beleg | Bricht | Vorschlag | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| T3-13-01 | high | K3 | DEFEKT | eq-copilot/plugin/probe/PipeProbeMain.cpp:112; eq-copilot/plugin/probe/PipeProbeMain.cpp:55; eq-copilot/plugin/src/EqCopilotIds.h:18; eq-copilot/plugin/src/PipeClient.cpp:221 | PipeProbe verwendet ohne Pipeargument die Produktions-Pipe und meldet dort synthetische Sensorwerte sowie ein Duplikat an. | Lesende Aufrufkette: main ohne argv[1] → kPipeName; serverErwartungFuer ohne Binaryargument → installierter Broker; baueClient (:67) → echter v2-PipeClient. Ein gültiger installierter Broker erfüllt die Authentisierung; eine Probe-Namensprüfung fehlt. | „Probe-Pipe ist nie Produktions-Pipe“ (Auftragsinvariante; CLAUDE.md:195 begrenzt Testbroker auf Probe-Pipenamen). | Ein explizites isoliertes Probe-Pipeargument verlangen und leere Argumente sowie den Produktionsnamen vor jedem Verbindungsversuch abweisen. | offen |
| T3-13-02 | critical | K2 | DEFEKT | tools/fl/laufzeit.ps1:350; tools/fl/laufzeit.ps1:353; tools/fl/laufzeit.ps1:250; tools/fl/laufzeit.ps1:809; tools/fl/laufzeit.ps1:820 | Ein fremdes FL-Projekt mit „Nakama-Diagnose.flp“ im Fenstertitel wird als Diagnoseprojekt eingestuft und kann samt ungespeicherten Änderungen zwangsweise beendet werden. | Quellherleitung: eigenes fremdes D:/Songs/Nakama-Diagnose.flp öffnen; Neustart-/Installationspfad oder -Beenden trifft Ist-Diagnose per Teilstring und ruft Stop-Process -Force für dessen PID auf. Weder Projektpfad noch Besitz des Prozesses werden geprüft; hier nicht ausgeführt. | „kein Beenden eines FL mit fremdem Projekt.“ (Gate S25e, docs/plan/plan.json:313). | Beenden an eine vom Lauf nachweislich gestartete Prozessinstanz und bestätigte Projektidentität binden; unzugeordnete Titelübereinstimmungen als Voraussetzung behandeln. | offen |
| T3-13-03 | high | K1 | LÜCKE | C:/Users/phili/Projekte/fl-studio-mcp/src/fl_studio_mcp/utils/midi_connection.py:215; C:/Users/phili/Projekte/fl-studio-mcp/src/fl_studio_mcp/utils/midi_connection.py:256; C:/Users/phili/Projekte/fl-studio-mcp/fl_controller/device_FLStudioMCP.py:145; tools/fl/szenario.py:615; tools/fl/szenario.py:178 | bekannt NAK-307: Darüber hinaus kann schon ein einzelner Client nach einem Timeout-Retry die verspätete Antwort des vorigen Befehls als erfolgreiche Messung des nächsten Befehls werten. | Quellableitung: A=Peaks Spur 1 läuft in Timeout; Retry A nimmt erste A-Antwort an; zweite A-Ausführung hat den Auftrag bereits gelesen, veröffentlicht aber erst nach Beginn von B=Peaks Master. B akzeptiert jedes gültige JSON; beide Schritte prüfen nur peak_max. Request-ID, Aktions-, Ziel- und Instanzabgleich fehlen. Der konkrete Ablauf ist nicht im Host reproduziert. | keine Zusage | Jeden Auftrag mit einer unverwechselbaren Kennung versehen, diese vom Controller zurückgeben lassen und ausschließlich die passende Antwort annehmen. | offen |
| T3-13-04 | medium | K5 | LÜCKE | eq-copilot/plugin/probe/PipeProbeMain.cpp:187; eq-copilot/plugin/probe/PipeProbeMain.cpp:194; eq-copilot/plugin/src/PipeClient.cpp:626; eq-copilot/plugin/src/PipeClient.cpp:351 | PipeProbe kann einen Verbindungsabbruch von A als erfolgreich gemessenes Konfliktende nach dem Stoppen von B ausgeben. | Nach B.stop genügt !A.konflikt. Der eigene Disconnect-/Neuaufbaupfad von A setzt konflikt=false ohne neues Broker-ACK; dadurch folgen KONFLIKT-ENDE OK und Exit 0 trotz fehlender Konfliktende-Antwort. Anders als Stufe 1 wird kein Verbindungsstatus verlangt. | keine Zusage | Konfliktende nur bei bestehender Verbindung und einem nach B.stop neu bestätigten ACK derselben Sitzung akzeptieren. | offen |
| T3-13-05 | medium | K5 | DEFEKT | tools/fl/szenario.py:241; C:/Users/phili/Projekte/fl-studio-mcp/src/fl_studio_mcp/utils/__init__.py:4; tools/fl/szenario.py:2415 | Der reale Szenarioimport lädt entgegen M-22 indirekt fl_trigger; die lokale Importwache erfasst nur die eigene Quelldatei und einen Attrappenlauf. | Python lädt vor utils.connection das Paket utils; dessen __init__.py importiert fl_trigger ausdrücklich. Der Selbsttest umgeht Umgebung.verbindung und prüft die eigene AST. Kein aktueller Tastendruck daraus hergeleitet; pynput wird im Fremdmodul erst in Methoden importiert. | „kein Import von `fl_studio_mcp.utils.fl_trigger` und `pynput`; jede FL-Aktion geht über `send_command` (MIDI-Trigger und JSON-Datei), keine über einen Tastendruck“ (M-22, docs/beweise/NAK-286.md:535). | Den eager Import von fl_trigger aus utils.__init__ entfernen und die Importgrenze mit dem echten Paket bei ersetztem MIDI-Zugang prüfen. | offen |
| T3-13-06 | low | K5 | HÄRTUNG | tools/fl/laufzeit.ps1:64; tools/fl/laufzeit.ps1:312; tools/fl/laufzeit.ps1:428; tools/fl/laufzeit.ps1:781; tools/fl/laufzeit.ps1:1601 | Der Laufzeit-Arm übernimmt die externe MCP-Arbeitskopie ohne geprüften Commit-Pin und ohne deren Bibliotheksstand im Rohprotokoll zu identifizieren. | rg über tools/fl und eq-copilot/install findet keinen erzwungenen MCP-Revisionseintrag; uv lädt den Arbeitsbaum. Pruefe-Controller protokolliert und gleicht den Controller-SHA ab, übernimmt aber den gerade vorhandenen Fremdstand; Änderungen an midi_connection.py oder fenster.py bleiben durch diesen SHA und den protokollierten Nakama-HEAD unidentifiziert. | keine Zusage | Erlaubte MCP-Revision und Arbeitsbaumfingerprint einschließlich geladener Bibliothek erfassen und vor einer als reproduzierbar verwendeten Hostmessung abgleichen. | offen |

## Herleitungen und Skeptikerpass je Befund

### T3-13-01 — Probe startet auf dem Produkttransport

`PipeProbeMain.cpp:112` setzt ohne erstes Argument `EqCopilotIds.h:18` ein; auch ein leerer Name fällt in `PipeClient.cpp:221` auf diesen Namen zurück. `serverErwartungFuer` (:55) verwendet ohne drittes Argument gerade den installierten Broker. Der Sicherheitsnachweis des Gegenprozesses verhindert diesen Fall deshalb nicht: Der echte Produktionsbroker ist der erwartete Gegenprozess. `probeMessung` (:31) und `baueClient` (:67) liefern künstliche Werte; anschließend meldet sich B mit derselben Sensor-ID an (:162).

Das ist ein v2-Werkzeug, kein v3-PipeToken-Verbraucher: `plugin/CMakeLists.txt:599` bis :609 bindet main, PipeClient und den Kern. Die v3-Probe-Allowlist wird hier nicht gerufen. Ein Aufruf mit isolierter Pipe bleibt möglich, ist aber nicht erzwungen. NAK-290 betrifft zwei Kanons auf geteilten **Probenamen**, nicht diesen Default auf der Produktions-Pipe.

Gegenbeleg gesucht: `PipeClientLifecycleTestMain.cpp:1408` bis :1411 beschreibt ausdrücklich, dass kein Kanon-Bein dieses main ausführt. Die zwei NAK-289-Quelltextwachen (:1431 und :1461) prüfen Ausnahmegrenze und Flush, nicht den Pipenamen. Der neue Befund verlangt weder eine manipulierte Quelle noch einen falschen Testbroker: Der normale Aufruf ohne Argument reicht bei vorhandenem installiertem Broker.

### T3-13-02 — Namensgleichheit ist kein Prozessbesitz

`Ist-Diagnose` (:350) und `Ist-Fremd` (:351) unterscheiden ausschließlich anhand eines Teilstrings im sichtbaren Fenstertitel. Ein Projekt gleichen Dateinamens in einem anderen Ordner oder ein Titel mit diesem Namensbestandteil erfüllt die Diagnosebedingung. `Beende-Diagnose-FL` (:353) durchläuft **alle** so gefundenen Instanzen und benutzt den echten `Stop-Process -Force`-Adapter (:250). Installieren/Controllerwechsel/Rendern können diesen Helfer erreichen; explizit stehen der Neustartpfad bei :809 und der Abschluss mit -Beenden bei :820.

Der SHA-256-Vergleich in `Bereite-Projekt/Pruefe-Projekt` (:485 bis :553) betrifft die vorbereiteten Dateien. Er belegt nicht, dass ein bereits vorhandenes Fenster genau diese Datei geöffnet hat. Bei Wiederverwendung entfällt `Warte-Boot` (:810); `Warte-Ping` (:683) ersetzt eine zuvor gestartete PID durch die erste Titelübereinstimmung. Damit kann dieselbe fehlende Besitzbindung auch die Messung auf die falsche Instanz richten. Der kritische Befund ist das unmittelbar erreichbare erzwungene Beenden samt möglichem Verlust ungespeicherter Arbeit.

Skeptiker: M-03 in `docs/beweise/NAK-286.md:510` setzt für seinen Fremdprojektfall ausdrücklich einen Titel ohne Diagnose-Dateinamen voraus. Der Selbsttest (:1093 bis :1116) benutzt „Mein Song.flp“. Diese engere Matrixzeile wird durch den Kollisionsfall nicht widerlegt; verletzt wird die weiter gefasste Grenze des Gates S25e, wörtlich in der Tabelle. NAK-297 betrifft den leeren MainWindowTitle und dessen Ersatz durch TFruityLoopsMainForm. Hier ist der echte Hauptfenstertitel vorhanden und nicht leer. Es geht daher über NAK-297 hinaus.

### T3-13-03 — Späte Antwort wird zur nächsten Messung

Die externe Bibliothek legt nur `action` und `params` in eine gemeinsame Kommandodatei (:215), entfernt die gemeinsame Antwortdatei (:227) und sendet einen MIDI-Trigger (:237). Der Controller liest den Auftrag (:145), führt ihn aus (:152) und schreibt eine Antwort ohne Korrelationskennung (:166). Der Client nimmt jedes parsebare JSON als die gerade erwartete Antwort an (:256 bis :268).

Eine zulässige zeitliche Reihenfolge mit **einem** Client:

1. Peaks für Spur 1 (A) überschreitet die Frist; `_sende` sendet nach 0,5 s genau einmal erneut.
2. Die erste A-Ausführung veröffentlicht ihre Antwort. Die Wiederholungswartezeit nimmt sie an.
3. Die zweite A-Ausführung hat bereits den A-Auftrag gelesen, veröffentlicht ihre Antwort aber erst später.
4. Der Client beginnt B, Peaks für Spur 0: schreibt B und entfernt eine etwaige alte Antwort.
5. Die zweite A-Ausführung veröffentlicht jetzt ihre Spur-1-Antwort. Die B-Wartezeit nimmt sie als B-Ergebnis an.

Dafür genügt Schedulingverzögerung zwischen Lesen und Schreiben des Controllers. Der Ablauf wird nicht als beobachteter Hostfehler ausgegeben. Er ist aus den getrennten Dateioperationen herleitbar; eine andere Reihenfolge kann korrekt verlaufen. `bereitschaft.json:16` und :17 verlangen in beiden Fällen nur denselben Wertebereich von peak_max. `_pruefe` (:178) gleicht die Antwort nicht automatisch gegen den angeforderten Track ab. Ein von Hand aufmerksamer Leser könnte die falsche Tracknummer in einer Rohantwort bemerken; der automatische Schritt kann dennoch bestehen.

NAK-307 kennt Timeout und mögliche Konkurrenz eines zweiten Clients. Neu ist die falsche **erfolgreiche** Zuordnung ohne zweiten Client. M-20/M-78 sichern Timeoutwahl und genau einen Retry mit einer sofort antwortenden Attrappe, nicht das Eigentum einer Antwort. Eine explizite Regel für Request-Korrelation und Wiederholungsidentität fehlt: deshalb LÜCKE. Für zustandsändernde Befehle ist zusätzlich zu entscheiden, ob ein Retry denselben Auftrag dedupliziert; dies ist eine technische Protokollregel, keine offene Produktentscheidung.

### T3-13-04 — Fehlendes Flag ersetzt das fehlende ACK

Stufe 1 von PipeProbe verlangt bei :139 eine bestehende Verbindung, Protokoll v2 und mindestens zwei bestätigte Heartbeats. Das abschließende Konfliktende verlangt bei :188 nur noch `!konflikt`. Die Aufräumsequenz von PipeClient (:626) setzt den Snapshot getrennt und konflikt=false; ebenso der Neuaufbau (:351). Ein Pipefehler/Brokerabbruch von A nach erfolgreich beobachtetem Duplikat erfüllt damit den Erfolgsausdruck, ohne dass A ein konfliktfreies ACK erhalten hat.

Die erreichbare Sonderlage ist ein normaler Verbindungsabbruch während der abschließenden Wartezeit, keine Manipulation von Tests oder Quellen. Die Quelltextwachen aus NAK-289 sind auch hier kein Verhaltensbeleg. Im gelesenen Gate-/Matrix-/Registerkontext wurde kein expliziter Satz über dieses Abschlussprädikat gefunden. Deshalb LÜCKE mit fehlender Regel „Konfliktende braucht ein frisches, derselben Sitzung zugeordnetes Broker-ACK“.

### T3-13-05 — Importwache endet vor der Paketgrenze

Der echte Import in `Umgebung.verbindung` (:241) initialisiert das Elternpaket utils. Dieses importiert in seiner Zeile 4 fl_trigger. Die transitive Importkette genügt bereits, um die ausdrückliche M-22-Zusage zu verletzen. Der Selbsttest `kein_piano_roll_weg` (:2415) sieht nur die Importknoten der eigenen Datei beziehungsweise seine Attrappenumgebung.

Die Tragweite bleibt begrenzt: `fl_trigger.py:67` und :158 importieren pynput erst in Methoden; der reine Paketimport ruft weder Fokuswechsel noch Tastendruck auf. Kein aktueller Tastendruck und kein aktueller Fokusdiebstahl wird behauptet. Der konkrete Defekt besteht in der gebrochenen Importgrenze einer abgenommenen Matrix und ihrer einseitigen Wache.

### T3-13-06 — Controller-SHA ist kein Stand der Messbibliothek

Der reale MCP-HEAD entsprach zu Beginn dem im Auftrag genannten SHA. Dies ist ein Befund **dieses Audits**, kein vom Laufzeit-Arm erzwungener Pin. Die historischen Fremdcommits im Register und NAK-286-Manifeste dokumentieren frühere Arbeiten; sie werden vom Runner nicht als zulässiger Stand eingelesen.

`Pruefe-Controller` (:428) ist eine echte Wache: Quellskript- und Installations-SHA werden protokolliert (:441), Abweichungen per setup-local angeglichen und nachgeprüft (:450). Beide Dateien hatten beim Lesen SHA-256 `704DECFF3689D618E48CF0B8F175A67E3B73312C0A9BA1DAE6F5CF9206D834EC`. Ein abweichender Controller **auf Platte** wäre somit nicht unsichtbar. Der Bezugspunkt ist aber immer die aktuelle externe Arbeitskopie; der geladene Bibliotheksstand (MIDI-Antwortsemantik, Portwahl, Fenstererfassung) wird dadurch nicht identifiziert. Nakama-HEAD im Rohkopf (:781/:1601) beantwortet diese Frage nicht.

Keine vorhandene Zusage verlangt einen unveränderlichen externen Commit; daher HÄRTUNG. Der nachweisliche Nutzen ist, einen Messwert einer bestimmten Implementierung seiner Gegenseite zuordnen und Abweichungen gezielt reproduzieren zu können. Dies wiederholt nicht B03, das Plugin und installierten Broker unterschiedlicher Baugenerationen betrifft.

## Vollständige Population und Grenzen des Nachweises

### Bau-Pins und Codegen

| Grenze | Vollständig erfasste Population / Quelle | Ergebnis und tatsächlicher Detektor |
| --- | --- | --- |
| Rust-Auflösung | Cargo.toml (82 Zeilen), Cargo.lock (549 Zeilen): 66 Pakete, davon 65 Registry-Pakete mit Checksum und ein lokales Paket | Keine Git-Abhängigkeit ohne rev, kein patch/replace. Ein mehrfacher Paketname: syn 2.0.119 und 3.0.3 in unterschiedlichen Makroketten (Lock :307/:452). Zwei Versionen sind kein Defekt. |
| Rust-Compiler | Cargo.toml:22; Suche nach rust-toolchain, rust-toolchain.toml und build.rs | rust-version=1.85 ist eine Mindestversion, kein Compilerpin; kein eigenes broker/build.rs und keine Rust-Toolchain-Datei gefunden. beweise.ps1:948 protokolliert rustc/cargo. Die Buildaufrufe :493/:895 verwenden kein --locked; bei unverändertem Manifest/Lock ist daraus kein aktueller falscher Build ableitbar. |
| JUCE | CMakeLists.txt:32; NakamaBruecke.cmake:19/:50 | Tag 8.0.9; vorhandener Checkout f72bad64d29715216226685810c5196bd0d79d77. Die Brücke akzeptiert den bekannten Original- oder Patchhash der Wrapperquelle und prüft das Ergebnis. Das pinnt diesen Patchpfad, nicht jede Datei der gesamten Toolchain. |
| VST3 und MSVC | VST3_SDK/pluginterfaces/vst/vsttypes.h:27 im eingebundenen JUCE; CMakeCache.txt:416 | SDK 3.7.12 aus dem JUCE-Baum; kein zweiter separater SDK-Download. VS 17 2022 / x64, kein exakter MSVC-Toolsetpin im Cache. Keine neue, hier belegte Nulltestabweichung; NAK-175 zum Stack ist bekannt. |
| FlatBuffers-Bereitstellung | fünf Funktionen in NakamaFlatBuffers.cmake; Projekthook 11 Zeilen; Belegskript 111 Zeilen | WERKZEUG.json bindet Runtime/flatc-Stand. FetchContent nutzt Commit 7e163021e59cca4f8e1e35a7c828b5c6b7915953. Der Hook hängt den Nachweis nach dem flatc-Bau an; Version und Quellstand werden geprüft. |
| flatc-Zeiger | build/nakama-flatc-pfad-Release.txt, eine Zeile; Zielbinary und Belegdatei | Ziel vorhanden: C:/Users/phili/Projekte/Nakama/eq-copilot/build/_deps/flatbuffers-build/Release/flatc.exe. Beleg nennt denselben Commit und SHA-256 c5e4c8859a1cd37b706c8c010577755475ac67451fb3a2b231b70b3edc6cfd04; lesender Hashvergleich mit der Binary stimmt. Binary nicht gestartet. |
| Schema → Generat | ein .fbs, ein C++-Generat, neun Tabellen, 64 Felder, fünf Enums | Alle 64 IDs statisch gegen die generierten VT-Offsets gelesen/zugeordnet (Anhang). Kein abweichender Offset. Kein behaupteter bytegleicher Neugenerierungslauf. |
| Frischebeweis | pruefe_flatc_drift.py, alle 391 Zeilen, insbesondere :174/:286/:340 | A9 generiert C++ und Rust in temporäre Ausgabe und vergleicht mit den eingecheckten Dateien. Der reguläre Plugin-Bau regeneriert sie absichtlich nicht automatisch. Ein neuer Schemaeintrag ohne manuelles Generat kann einen normalen Build überleben, aber nicht den korrekt ausgeführten A9-Vergleich. |

Der flatc-Beleg ist ein Nachweis zur **Generator-Binary**, kein Zeitstempelnachweis „Generat jünger als Schema“. Beleg vor einem Schemaedit ist deshalb allein kein Fehler. Der eigentliche Schema-/Generatvergleich steht in A9. Die breitere Lücke im Frischeriegel ist bekannt als T3-09-01/B01 und wird hier nicht neu gezählt. NAK-280 bezeichnet einen verschwundenen Zeigerzielpfad; das Ziel war in dieser Sitzung vorhanden. Rust-Generat und Generatorausführung wurden nicht neu gemessen.

### Identitätsneutraler Kern

`plugin/CMakeLists.txt:198` bis :219 bildet die gesamte Quellmenge mit **21 Übersetzungseinheiten**. A14 führt dazu unabhängig 21 erwartete Objekte (:84 bis :127 in pruefe_kern_identitaetsfrei.py). PipeToken.cpp bleibt nach dem expliziten Kommentar in CMakeLists.txt:1306 außerhalb dieses Kerns. K1 ist als NAKAMA_KERN_UEBERSETZUNG geschaltet (:261); K2 wird deferred am Kern und seiner compilerwirksamen Hülle ausgeführt (:268); K2b/K2c laufen für jeden registrierten Verbraucher (:1381).

| CMakeLists.txt-Zeile | Quelle relativ zu eq-copilot/plugin/ |
| --- | --- |
| 199 | dsp/DspProgramm.cpp |
| 200 | dsp/DspBankPool.cpp |
| 201 | dsp/DspKern.cpp |
| 202 | state/NakamaKanon.cpp |
| 203 | state/NakamaLebenslauf.cpp |
| 204 | state/NakamaParameter.cpp |
| 205 | state/NakamaPreset.cpp |
| 206 | state/NakamaState.cpp |
| 207 | state/NakamaTransaktion.cpp |
| 208 | vertrag/NakamaVertrag.cpp |
| 209 | vertrag/NakamaTelemetrie.cpp |
| 210 | vertrag/NakamaEvidenz.cpp |
| 211 | core/ipc/WireEnvelope.cpp |
| 212 | core/ipc/IpcVerbindung.cpp |
| 213 | core/ipc/BrokerLifecycle.cpp |
| 214 | core/ipc/ControlClient.cpp |
| 215 | core/ipc/controlclient/Vertrag.cpp |
| 216 | core/ipc/controlclient/Nachrichten.cpp |
| 217 | core/ipc/controlclient/Verbindung.cpp |
| 218 | core/ipc/TelemetryClient.cpp |
| 219 | core/diagnose/Briefkasten.cpp |

Die **40 Verbraucher** bestehen aus 36 direkten Aufrufen und vier Instanzen aus drei Hilfsfunktionen. Die vollständige Aufrufliste steht im Anhang. Kein aktueller fremder Quellpfad im Kern oder nachweislich falscher Kernverbraucher wurde gefunden. A14 prüft Identitätsfreiheit und Objektpopulation des Kerns, nicht die fachliche Richtigkeit beliebiger Quellen außerhalb seiner Zielmenge.

NakamaKern.cmake enthält **33 Funktionen**. Sein Interpreter hat 22 benannte Operatorfälle: TARGET_PROPERTY (:512), BOOL (:587), IF (:599), STREQUAL (:623), EQUAL (:648), GENEX_EVAL (:675), TARGET_GENEX_EVAL (:679), FILTER (:704), NOT (:742), AND/OR (:754), CONFIG (:784), COMPILE_LANGUAGE (:805), CXX_COMPILER_ID (:824), LINK_ONLY (:846), COMPILE_ONLY/BUILD_INTERFACE (:855), INSTALL_INTERFACE (:860), PLATFORM_ID (:863), TARGET_EXISTS (:880), TARGET_NAME_IF_EXISTS/TARGET_NAME (:892). Hinzu kommen verschachtelte Bedingung (:485) und 0/1-Kurzform (:503). Nicht unterstützte aktive Ausdrücke werden im strengen Pfad nicht einfach als leer angenommen (:455/:473).

pruefe_nakama_kern_genex.cmake enthält **26 Erwartungsaufrufe und drei Rotproben** (vollständige Zeilenliste im Anhang). Es prüft den selbst geschriebenen Interpreter an Testwerten/Testzielen, nicht unmittelbar die von CMake für jeden echten Build erzeugte Kommandozeile. In den durchsuchten CMake-/Runnerquellen wurde kein automatischer Aufruf dieses separaten Skripts gefunden. Daraus allein folgt kein gebrochener Gate-Satz; die tatsächlichen K2/K2b/K2c-Wachen sind an den regulären Zielgraphen gebunden. Eine nur durch absichtliche Änderung von Quellen und unabhängiger A14-Liste erzeugte Umgehung wurde nicht als Befund aufgenommen.

### PipeProbe, PipeToken und Fristen

PipeProbe hat einen realen v2-Clientpfad mit vier Stufen: Verbindung/ACKs, laufende synthetische Analyse, Duplikatkonflikt, Konfliktende. Es misst keine v3-Telemetrie und keinen Audio-/Nulltest. Die Stufen 1/2 haben sekundenweise begrenzte Wartezeit (:114/:137), die Konfliktphasen je 100 Versuche à 100 ms (:164/:188). PipeClient begrenzt belegte Pipeversuche und verwendet Fristen für gerahmtes I/O; unvollständiger Handshake erreicht nicht die zwei verlangten ACKs. Die Ausgabe unterscheidet Stufenfehler (1), fehlende Servererwartung (2) und unbehandelte Ausnahme (70). Keine Ausführung in diesem Audit.

PipeToken.h/.cpp wurden vollständig einschließlich der fünf öffentlichen Funktionsdeklarationen und des Probehelfers gelesen. Der v3-Produktname entsteht aus dem SID-Token; der Token verwendet 26 Zeichen des Großbuchstaben/Base32-Alphabets. Der explizite Probepräfix enthält dagegen „.probe.“ und verlangt einen nicht leeren Rest (PipeToken.h:59). Im regulären Ableitungspfad können beide Namen deshalb nicht zusammenfallen. Diese Trennung deckt den v2-Default von PipeProbe nicht ab.

### Beidseitige FL-Antwortpopulation

Alle fünf JSON-Szenarien wurden vollständig gelesen: bereitschaft (14 Schritte), fenster (5), nulltest-host (9), snapshot-runde01 (13), u40-aktivitaetsgate (4): **45 Schritte, davon 43 Aktionen und zwei Wartezeiten**. Die Aktionsmenge enthält **14 externe Aktionen und fünf lokale Aktionstypen**. Die zusätzlichen Befehle innerhalb lokaler Aktionen (szenario.py:678/:1184/:1472/:1500/:1536/:1551/:2022) gehören zu derselben Transport-/Pingmenge.

Pfade mit Präfix MCP beziehen sich in den folgenden Tabellen auf C:/Users/phili/Projekte/fl-studio-mcp/. Controller ist jeweils fl_controller/device_FLStudioMCP.py.

| Externe Antwort / Controller-Zeile | Was die Rückgabe tatsächlich enthält | Nakama-Verwendung / Grenze |
| --- | --- | --- |
| system.ping :305 | is_playing aus transport; program_title aus ui.getProgTitle; FL-/API-Version aus APIs; Fehler pro Versionsfeld möglich | Bereitschaft und jeder Szenariostart; kein Projektpfad, keine Prozess-ID, keine Boot-/Requestkennung. Programmtitel ist kein nachgewiesener Projektdateiname. |
| transport.start :336 | Aufruf start, danach isPlaying gelesen | is_playing=true wird geprüft; start ist eine Toggle-Operation, ein verspäteter Retry ist daher nicht automatisch idempotent. |
| transport.stop :342 | stopped=true nach stop-Aufruf, kein nachgelesener Playingstatus | Bereitschaft prüft diese Bestätigung; sie ist kein unabhängiger Stillstandsmesswert. |
| transport.getStatus :354 | isPlaying, isRecording, SongPosHint, getLoopMode | Bereitschaft prüft is_playing und loop_mode nach den Stellbefehlen. |
| transport.setPosition :372 | requested_position und mode als Auftragsecho | Bewusst keine Positionsmessung; M-18 verlangt getrennten getPosition-Aufruf. |
| transport.getPosition :386 | getSongPos in ms und Sekunden, Hint separat | Snapshot und Stell-/Umlaufpfade; echte API-Lesung. |
| transport.getLength :396 | getSongLength in ms, s, absoluten Ticks und Takten | Bereitschaft/Rendervergleich/Snapshot; vier Modi explizit. |
| transport.setLoopMode :406 | liest vorherigen Modus, stellt nötigenfalls, gibt angeforderten mode zurück | Echo des Zielmodus; Bereitschaft liest später über getStatus nach. |
| mixer.getTrackInfo :435 | Name, Farbe, volume normalisiert, volume_db direkt aus FL, Pan, Flags, Slotdaten | Pegelkette auf Insert 1 und Master; dB nicht aus Normalwert erraten. |
| mixer.getRouting :587 | getRouteSendActive und getRouteToLevel für gültige Zielspuren | Sendlevel ist normalisiert; keine fiktive dB-Messung. |
| mixer.getPeaks :732 | linke/rechte Spitzen über getTrackPeaks, Maximum daraus; Trackindex mitgegeben | Bereitschaft verlangt peak_max in Bereich; angeforderter und gelieferter Track werden nicht automatisch gepaart (T3-13-03). |
| mixer.focusEditor :749 | Pluginvalidität/-name gelesen, showForm gerufen, focused=true bestätigt | fenster folgt mit separater Fensteraufnahme; focused=true selbst belegt keinen Vordergrundzustand. |
| channels.getInfo :786 | Kanalname, Routing, Normalpegel und dB-Modus von getChannelVolume, Flags | Kanal 0 gruppenlokal; Testtrack-/Routingprüfung und rohe Pegelkette. |
| plugins.getName :1031 | Name via Plugin-API für Index/Slot/use_global | Pluginbesetzung von Insert/Master; keine installierte Binaryhashmessung. |

Die komplette Serverregistrierung (server.py, 178 Zeilen) und alle fünf Toolmodule samt utils wurden gelesen. **Der Nakama-Laufzeitpfad benutzt die Verbindungsbibliothek direkt, nicht die MCP-Toolwrapper.** Default-/Fehlerrückgaben in solchen Wrappern wurden deshalb nicht als tatsächlich benutzte Messwerte des Runners ausgegeben. Die Portwahl ist Teil von midi_connection.py; Mehrdeutigkeit wird abgewiesen und nicht durch Wahl des ersten Ports verdeckt. Die Tests zur Portwahl wurden gelesen, nicht gestartet.

| Lokale Aktion | Eingang / echte Evidenz | Beurteilte Grenze |
| --- | --- | --- |
| lokal.fenster | fenster.py: Window-Enumeration nach Klasse/Titel/PID, PrintWindow-Bild, Abmessungen/Hash/Einfarbigkeit; Szenario-Unterprozess mit 5-s-Frist | Kein SetForegroundWindow nötig; der Bildpfad filtert die übergebene PID korrekt. Deren vorgelagerte Eigentumsbindung fehlt (T3-13-02). |
| lokal.nulltest | Renderstatus und WAV-/Bereichsvergleich in nulltest.py; separate Referenzpfade | Fehlender Render ist Voraussetzung, nicht Messung. NAK-306 zum noch nicht erreichbaren aktiven Hostvergleich bleibt bekannt. |
| lokal.briefkasten | Anfrage-/Antwortdateien, Rollen, Format, Version, Snapshotfelder | Kein Ersatz von Messwerten durch Controller-Echos. Vollständiges Produzenten-Audit des Briefkastens ist nicht diese Phase. |
| lokal.umlauf | Transportsteuerung, eigene getPosition-Aufrufe und Briefkasten-Snapshots | Transportantworten aus obiger Menge; Antwortkorrelation bleibt gemeinsame Grenze. |
| lokal.stellen | definierte Stellen, Transportantworten, Snapshotpaare und Aktivitätsmessung | Dieselbe Verbindungsbibliothek und dieselbe Grenze; keine neue Messung in dieser Sitzung. |

### Boot-, Projekt- und Exitidentität

Der Controller schreibt mcp_boot.json bei OnInit (:106), nicht bei jedem Ping. OnDeInit entfernt sie nicht. Die lesbare Marke unter C:/Users/phili/OneDrive/Dokumente/Image-Line/FL Studio/Settings/Hardware/FLStudioMCP/mcp_boot.json trug script_version=2026-09-15, loaded_at=2026-09-15T18:48:00 und program_title=FL Studio 2026; LastWriteTimeUtc war 2026-09-15T16:48:00.9600949Z. Sie enthält weder PID noch Projektpfad noch Laufkennung. Ihre Existenz wird hier nicht als lebender Hostbeweis gewertet.

Warte-Boot prüft bei frischem Start die Dateizeit gegen FlStart (:659). Nach Controllerinstallation verlangt es zusätzlich die erwartete Scriptversion (:661) und scheitert ohne neue Marke (:667). Sonst folgt nach 60 s eine 30-s-Schonfrist (:668), dann muss Ping antworten. Bei wiederverwendetem Diagnosefenster entfällt der Bootpfad (:810). Ping und Fenster werden bei :680/:683 lediglich nebeneinander gefunden; es gibt keine controllerseitige PID, die beide verbindet. Dies erklärt die Instanzgrenze von T3-13-02/-03, ohne aus einer alten Datei allein einen grünen Lauf herzuleiten.

Die drei binären .flp-Dateien wurden nur über Namen und SHA-256 erfasst, nicht als Projektinhalt geöffnet:

| Fixture relativ zu eq-copilot/fixtures/fl/ | SHA-256 dieser Sitzung |
| --- | --- |
| Nakama-Diagnose.flp | B93AA3FD0DBC134AB5049CEB3AFBEF535FB90704F71BB67E663A88B11E586229 |
| Nakama-Diagnose-Referenz.flp | 4C1A0E85B01F35C60A828363E89B1578F22BE224C04BF366F78A8791DDF3F02D |
| vorlagen/Nakama-Diagnose-Verarbeitung-hostparameter-unwirksam.flp | F35ACA8E47CC89E4CB0E072241302DDD7E75A07DC75815F398CE89974C759F36 |

Der fehlende aktive Referenzname Nakama-Diagnose-Verarbeitung.flp ist mit NAK-306 bekannt; die vorhandene Vorlage ersetzt ihn ausdrücklich nicht. laufzeit.ps1 kopiert/hashprüft Diagnose-/Referenzprojekte und schreibt Renderstatus mit Projekthash; diese Dateibeweise identifizieren keine beliebige bereits laufende FL-Instanz.

| Eingangsfall | Wertung im gelesenen Code |
| --- | --- |
| Kein brauchbarer Port, kein FL/Ping, stummer Controller nach Bereitschaftsfrist | Laufzeit-Exit 3 VORAUSSETZUNG; kein gemessener Erfolg. |
| Fremdprojekt ohne Diagnose-Namensbestandteil | Exit 0 UEBERSPRUNGEN mit Grund. Namenskollision siehe T3-13-02. |
| Erfolgreiche Szenarien | Exit 0 GEMESSEN. Beide Exit-0-Fälle sind erst mit dem Urteilswort unterscheidbar. |
| Falscher Messwert / normaler Schrittfehler | Szenario-/Laufzeit-Exit 4 VERFEHLT. |
| Nur dieser Szenarioschritt nicht messbar, z. B. fehlendes Referenzrender / eingeklappter Wrapper | Szenario-Exit 5; Runner übersetzt bei :828 zu Exit 3 VORAUSSETZUNG. laufzeit.ps1 selbst gibt keinen Exit 5 aus. |
| Ein Szenario 4 und ein anderes 3/5 | Voraussetzung hat in :828 Vorrang vor :829; Einzelcodes stehen im Protokoll. Nicht als neue Fehlklassifikation gezählt. |
| Halbes Antwort-JSON | midi_connection.py:269 gibt success=false sofort zurück; nur Timeout wird wiederholt. Beim Ping Voraussetzung, im normalen Messschritt verfehlt; kein automatischer grüner Default. |
| Späte, vollständige Antwort zum falschen Auftrag | Kann dagegen die Erfolgsprüfungen bestehen (T3-13-03). |

planstand.py wurde vollständig gelesen. Es wertet den vorhandenen Plan-/Beweiszustand und Kanon-/NAKAMA-URTEIL-Marken aus; eine eigene LAUFZEIT-Auswertung für GEMESSEN/UEBERSPRUNGEN/VERFEHLT/VORAUSSETZUNG wurde nicht gefunden. Der Planstand ist daher kein zusätzlicher Detektor dieser Unterschiede. Die Rohprotokolle können sie über Wort und Einzelcode ausdrücken. Keine Zusage einer solchen automatischen Planfortschreibung gefunden, deshalb hierfür kein zusätzlicher Defekt.

### Vier Header und drei Kontextdateien

Die vier Header besitzen **11 direkte Include-Stellen** in Produkt-/Testcode:

| Header | Alle direkten Include-Stellen |
| --- | --- |
| Diagnose.h | src/Diagnose.cpp:3; src/PluginEditor.h:22; src/prozessor/Analyse.cpp:37; tests/GoldenTestMain.cpp:27 |
| EqCopilotAssetKit.h | src/PluginEditor.h:23 |
| EqCopilotIds.h | probe/PipeProbeMain.cpp:22; src/PluginEditor.cpp:3; src/PipeClient.cpp:2; src/prozessor/Ipc.cpp:36 |
| LeitstandTokens.h | src/EqCopilotAssetKit.h:16; src/PluginEditor.cpp:2 |

Die Suche über die gesamte Produkt-/Testfläche ergab **128 benannte constexpr-Variablen**: Diagnose 1, AssetKit 30 (29 Geometriekonstanten plus lokales segmente), Ids 7, Leitstand 90. Jede Definition und ihre namensraumzugeordneten Leser stehen im Anhang. Dazu die acht Enumwerte von Diagnose.h: BefundKlasse resonanz/mittenLoch/mulm/haerte/hoehenHype (:41 bis :45) und Konfidenz niedrig/mittel/hoch (:48). Alle haben Leser; der Suchnachweis steht ebenfalls im Anhang.

Diagnose.h enthält 15 Befundfelder (:54 bis :71) und zwei Funktionsdeklarationen (:78/:81). AssetKit enthält 21 freie Inlinefunktionen (:60 bis :418) und die Frame-Methoden stelleSicher (:436) / male (:447). Der einzige Frame-Datenbesitz ist bild (:434). Die Dateien wurden als technische Referenz-/Verwendungsfläche gelesen; keine Design-/Optikbewertung vorgenommen.

51 Leitstand-Konstanten (:9 bis :59) haben keinen Code-Leser außerhalb ihrer Definition; 39 copilot-Konstanten (:60 bis :98) haben Leser. Das ist kein Funktionsfehler und keine neue Härtungsforderung für den geparkten Designteil. Gleichnamige Frame-Limits/Backoffwerte in WireEnvelope/IpcVerbindung wurden als getrennte Namespacekonstanten geprüft, nicht als weitere Leser von EqCopilotIds gezählt; aus den gegenwärtig gleichen Werten entsteht kein belegter Widerspruch.

Sensoruebersicht.svelte (475 Zeilen) bleibt Referenzfläche: Kein Bau-/Testaufrufer in der durchsuchten CMake-/Cargo-/Werkzeugfläche gefunden. antworten_blatt.py (113 Zeilen, num :27, zelle :36, main :41) erzeugt eine Antwortenübersicht aus dem Bearbeitungsstatus der Fragen; es setzt keinen Abnahme-/Kanonstatus. planstand.py verwendet für offene Fragen und Ticketstand die dafür vorgesehenen JSON-Werte. Verschiedene Zählpopulationen sind hier kein nachgewiesener Widerspruch. auftrag-lebenslauf-audit.md (73 Zeilen) ist ein Auftragsmuster; kein vom Bau oder Test ausgeführtes Programm. Keine Befunde gegen deren Prosa.

## Populationsanhang: jedes Feld, jede Konstante und jeder Kernaufruf

Die Zeilenlisten sind statische Inventare, keine Testausgaben. Die Inhaltsprüfung der vollständig gelesenen Dateien geht über die hier aus Platzgründen knappen Anker hinaus.

### CMake-Funktionen und Kernverbraucher

43 Funktionsdefinitionen in den gelesenen CMake-Helfern (33 davon in NakamaKern):

```text
eq-copilot/cmake/NakamaBruecke.cmake:25:function(nakama_quellhash datei heraus)
eq-copilot/cmake/NakamaBruecke.cmake:33:function(nakama_bruecke_anwenden juce_quelle patch_datei)
eq-copilot/cmake/NakamaFlatBuffers.cmake:28:function(nakama_flatbuffers_steckbrief steckbrief_datei)
eq-copilot/cmake/NakamaFlatBuffers.cmake:55:function(nakama_flatbuffers_version_pruefen quelle major minor revision)
eq-copilot/cmake/NakamaFlatBuffers.cmake:92:function(nakama_flatbuffers_commit_pruefen quelle erwartet ergebnis)
eq-copilot/cmake/NakamaFlatBuffers.cmake:134:function(nakama_flatbuffers_flatc_beleg_anhaengen)
eq-copilot/cmake/NakamaFlatBuffers.cmake:171:function(nakama_flatbuffers_bereitstellen steckbrief_datei)
eq-copilot/cmake/pruefe_nakama_kern_genex.cmake:128:function(_nakama_genex_rotprobe schalter was pass_text)
eq-copilot/cmake/pruefe_nakama_kern_genex.cmake:153:function(_nakama_genex_erwarte name ausdruck erwartung)
eq-copilot/cmake/NakamaIdentitaet.cmake:70:function(nakama_identitaet_lesen ziel_id praefix)
eq-copilot/cmake/NakamaKern.cmake:76:function(_nakama_kern_konfigurationen ausgabe)
eq-copilot/cmake/NakamaKern.cmake:88:function(_nakama_kern_ziel_aufloesen eingabe ausgabe)
eq-copilot/cmake/NakamaKern.cmake:106:function(_nakama_kern_ziel_existiert eingabe ausgabe)
eq-copilot/cmake/NakamaKern.cmake:118:function(_nakama_kern_importierte_konfigabbildung_pruefen ziel kontext)
eq-copilot/cmake/NakamaKern.cmake:153:function(_nakama_kern_konfiguration_passt namen konfiguration ausgabe)
eq-copilot/cmake/NakamaKern.cmake:195:function(_nakama_kern_genex_ende text anfang ausgabe bekannt)
eq-copilot/cmake/NakamaKern.cmake:226:function(_nakama_kern_genex_kopf innen operator rest hat_rest bekannt)
eq-copilot/cmake/NakamaKern.cmake:268:function(_nakama_kern_genex_argumente text ausgabe bekannt)
eq-copilot/cmake/NakamaKern.cmake:318:function(_nakama_kern_genex_text_vollstaendig text ausgabe)
eq-copilot/cmake/NakamaKern.cmake:336:function(_nakama_kern_eigenschaftseintraege_gruppieren wert ausgabe)
eq-copilot/cmake/NakamaKern.cmake:364:function(_nakama_kern_liste_auswerten_mit_stapel
eq-copilot/cmake/NakamaKern.cmake:386:function(_nakama_kern_wahrheitswert wert ausgabe)
eq-copilot/cmake/NakamaKern.cmake:404:function(_nakama_kern_target_property_lesen ziel eigenschaft ausgabe bekannt)
eq-copilot/cmake/NakamaKern.cmake:455:function(_nakama_kern_genex_fehler art operator innen kontext stapel)
eq-copilot/cmake/NakamaKern.cmake:470:function(_nakama_kern_genex_auswerten innen konfiguration art kontext stapel ausgabe bekannt)
eq-copilot/cmake/NakamaKern.cmake:928:function(_nakama_kern_text_auswerten_mit_stapel
eq-copilot/cmake/NakamaKern.cmake:983:function(_nakama_kern_text_auswerten wert konfiguration art kontext ausgabe bekannt)
eq-copilot/cmake/NakamaKern.cmake:991:function(_nakama_kern_wert_relevant wert art ausgabe)
eq-copilot/cmake/NakamaKern.cmake:1039:function(_nakama_kern_art_ist_definemenge art ausgabe)
eq-copilot/cmake/NakamaKern.cmake:1048:function(_nakama_kern_linkkante_stringoperator wert ausgabe)
eq-copilot/cmake/NakamaKern.cmake:1059:function(_nakama_kern_wert_auswerten wert konfiguration art kontext ausgabe)
eq-copilot/cmake/NakamaKern.cmake:1107:function(_nakama_kern_huelle start konfiguration ausgabe)
eq-copilot/cmake/NakamaKern.cmake:1169:function(_nakama_kern_nutzungshuelle start konfiguration ausgabe)
eq-copilot/cmake/NakamaKern.cmake:1227:function(_nakama_kern_option_defines ausgabe)
eq-copilot/cmake/NakamaKern.cmake:1251:function(_nakama_kern_eigenschaft_defines ziel eigenschaft konfiguration fokus ausgabe)
eq-copilot/cmake/NakamaKern.cmake:1297:function(_nakama_kern_wirksame_defines ziel konfiguration fokus ausgabe)
eq-copilot/cmake/NakamaKern.cmake:1339:function(nakama_kern_riegel_pruefen ziel)
eq-copilot/cmake/NakamaKern.cmake:1379:function(nakama_kern_juce_fassade name)
eq-copilot/cmake/NakamaKern.cmake:1475:function(_nakama_kern_juce_define_ist_ausgenommen define aus)
eq-copilot/cmake/NakamaKern.cmake:1489:function(_nakama_kern_juce_defines ziel konfiguration aus)
eq-copilot/cmake/NakamaKern.cmake:1517:function(_nakama_kern_widersprueche menge aus)
eq-copilot/cmake/NakamaKern.cmake:1534:function(nakama_kern_konfig_pruefen kern referenz)
eq-copilot/cmake/NakamaKern.cmake:1626:function(nakama_kern_schalter_pruefen kern referenz)
```

36 direkte Kernanbindungen in eq-copilot/plugin/CMakeLists.txt; nach dem Doppelpunkt steht die Quellzeile:

```text
291:nakama_kern_anbinden(EqCopilot)
473:nakama_kern_anbinden(EqCopBriefkastenTest)
500:nakama_kern_anbinden(EqCopSessionSoak)
514:nakama_kern_anbinden(EqCopSonde012SourcesModelTest)
547:nakama_kern_anbinden(EqCopSonde014AssistentTest)
560:nakama_kern_anbinden(EqCopSonde014BefundTest)
578:nakama_kern_anbinden(EqCopSonde012ProjectReloadTest)
596:nakama_kern_anbinden(EqCopSonde012SourcesLatencyTest)
609:nakama_kern_anbinden(EqCopPipeProbe)
624:nakama_kern_anbinden(EqCopPipeClientTest)
641:nakama_kern_anbinden(EqCopNullTest)
675:nakama_kern_anbinden(EqCopMarkierungTest)
692:nakama_kern_anbinden(EqCopShot)
709:nakama_kern_anbinden(EqCopPaintBench)
775:nakama_kern_anbinden(EqCopIdentityTest)
796:nakama_kern_anbinden(EqCopStateMigrationTest)
810:nakama_kern_anbinden(EqCopSonde013ExperimentGoldenTest)
820:nakama_kern_anbinden(EqCopSonde013ExperimentUiTest)
838:nakama_kern_anbinden(EqCopSonde013PrePostGoldenTest)
872:nakama_kern_anbinden(EqCopSonde013PassageStateTest)
904:nakama_kern_anbinden(EqCopSonde014IntentTest)
930:nakama_kern_anbinden(EqCopLebenslaufTest)
956:nakama_kern_anbinden(EqCopQueueStressTest)
975:nakama_kern_anbinden(EqCopIpcTest)
988:nakama_kern_anbinden(EqCopIpcLast)
1004:nakama_kern_anbinden(EqCopSonde013EventWireTest)
1018:nakama_kern_anbinden(EqCopSonde013DynamicsTest)
1033:nakama_kern_anbinden(EqCopSonde013FingerprintGoldenTest)
1047:nakama_kern_anbinden(EqCopSonde013InterventionRingTest)
1061:nakama_kern_anbinden(EqCopSonde013QualityClassTest)
1077:nakama_kern_anbinden(EqCopSonde013StereoGoldenTest)
1093:nakama_kern_anbinden(EqCopSonde013TruePeakGoldenTest)
1135:nakama_kern_anbinden(EqCopAnalysisGoldenTest)
1154:nakama_kern_anbinden(EqCopDspGoldenTest)
1176:nakama_kern_anbinden(EqCopTransactionTest)
1271:nakama_kern_anbinden(EqCopSchemaTest)
```

Vier weitere konkrete Ziele aus Hilfsfunktionen:

- plugin/CMakeLists.txt:390 → NakamaProbeeq, über nakama_sonde_ziel:320 und Kernanbindung:365
- plugin/CMakeLists.txt:421 → EqCopProbeeqNullTest, über nakama_sonde_nulltest:397 und Kernanbindung:412
- plugin/CMakeLists.txt:447 → EqCopSonde012HostChannelContextTest, über nakama_sonde012_test:427 und Kernanbindung:444
- plugin/CMakeLists.txt:449 → EqCopSonde012LoudnessSourceTest, über nakama_sonde012_test:427 und Kernanbindung:444

26 Erwartungsaufrufe und drei Rotproben in eq-copilot/cmake/pruefe_nakama_kern_genex.cmake:

```text
189:_nakama_genex_erwarte(
193:_nakama_genex_erwarte(
197:_nakama_genex_erwarte(
201:_nakama_genex_erwarte(
205:_nakama_genex_erwarte("05 NOT" "$<NOT:$<BOOL:OFF>>" "1")
206:_nakama_genex_erwarte(
210:_nakama_genex_erwarte(
214:_nakama_genex_erwarte("08 CONFIG-Wert" "$<CONFIG>" "${NAKAMA_TEST_CONFIG}")
215:_nakama_genex_erwarte(
219:_nakama_genex_erwarte(
230:_nakama_genex_erwarte(
234:_nakama_genex_erwarte(
238:_nakama_genex_erwarte(
247:_nakama_genex_erwarte(
251:_nakama_genex_erwarte(
255:_nakama_genex_erwarte(
259:_nakama_genex_erwarte(
263:_nakama_genex_erwarte(
267:_nakama_genex_erwarte(
276:_nakama_genex_erwarte(
287:_nakama_genex_erwarte(
293:_nakama_genex_erwarte(
306:_nakama_genex_erwarte(
310:_nakama_genex_erwarte(
314:_nakama_genex_erwarte(
318:_nakama_genex_erwarte(
501:_nakama_genex_rotprobe(
510:_nakama_genex_rotprobe(
516:_nakama_genex_rotprobe(
```

### 64 Schemafelder gegen die generierten VT-Einträge

`schema` = eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs.
`generat` = eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h.
True bedeutet ausschließlich: generierter VT-Offset = 4 + 2 × deklarierte Feld-ID. Die fünf Enums stehen im Schema bei :60/:66/:73/:79/:104; Root und Identifier bei :365/:39. Typen, Defaults, Builder/Verifier und Enumabschnitte wurden ebenfalls im vollständigen Generat gelesen. Dieser Textvergleich ersetzt keine Ausführung von flatc.

```text
Adresse.logon_sid | schema:122 | id=0 | generat:229 VT=4 | True
Adresse.project_binding_id | schema:123 | id=1 | generat:230 VT=6 | True
Adresse.session_epoch | schema:124 | id=2 | generat:231 VT=8 | True
Adresse.instance_id | schema:125 | id=3 | generat:232 VT=10 | True
Adresse.runtime_nonce | schema:126 | id=4 | generat:233 VT=12 | True
AbgeleiteteGrenzen.start | schema:134 | id=0 | generat:352 VT=4 | True
AbgeleiteteGrenzen.ende | schema:135 | id=1 | generat:353 VT=6 | True
AbgeleiteteGrenzen.herleitung | schema:136 | id=2 | generat:354 VT=8 | True
Schleife.active | schema:143 | id=0 | generat:423 VT=4 | True
Schleife.bounds_valid | schema:144 | id=1 | generat:424 VT=6 | True
Schleife.start_ppq | schema:145 | id=2 | generat:425 VT=8 | True
Schleife.end_ppq | schema:146 | id=3 | generat:426 VT=10 | True
Schleife.abgeleitete_grenzen | schema:147 | id=4 | generat:427 VT=12 | True
Transportstempel.transport_epoch | schema:158 | id=0 | generat:519 VT=4 | True
Transportstempel.continuity_segment | schema:159 | id=1 | generat:520 VT=6 | True
Transportstempel.sequence | schema:160 | id=2 | generat:521 VT=8 | True
Transportstempel.zeitbasis | schema:161 | id=3 | generat:522 VT=10 | True
Transportstempel.project_sample_start | schema:162 | id=4 | generat:523 VT=12 | True
Transportstempel.sample_count | schema:163 | id=5 | generat:524 VT=14 | True
Transportstempel.sample_rate | schema:164 | id=6 | generat:525 VT=16 | True
Transportstempel.playing | schema:165 | id=7 | generat:526 VT=18 | True
Transportstempel.recording | schema:166 | id=8 | generat:527 VT=20 | True
Transportstempel.continuous_time_samples | schema:167 | id=9 | generat:528 VT=22 | True
Transportstempel.schleife | schema:168 | id=10 | generat:529 VT=24 | True
Transportstempel.input_presentation_latency | schema:169 | id=11 | generat:530 VT=26 | True
Transportstempel.output_presentation_latency | schema:170 | id=12 | generat:531 VT=28 | True
Transportstempel.gueltigkeit | schema:171 | id=13 | generat:532 VT=30 | True
Transportstempel.process_context_present | schema:181 | id=14 | generat:533 VT=32 | True
Bandwerte.gitter | schema:200 | id=0 | generat:732 VT=4 | True
Bandwerte.encoding | schema:201 | id=1 | generat:733 VT=6 | True
Bandwerte.werte_i16 | schema:202 | id=2 | generat:734 VT=8 | True
Bandwerte.werte_f32 | schema:203 | id=3 | generat:735 VT=10 | True
Bandwerte.gueltig_bitmap | schema:204 | id=4 | generat:736 VT=12 | True
Bandwerte.saturated | schema:205 | id=5 | generat:737 VT=14 | True
Headroomverteilung.p10_db | schema:225 | id=0 | generat:871 VT=4 | True
Headroomverteilung.p50_db | schema:226 | id=1 | generat:872 VT=6 | True
Headroomverteilung.p95_db | schema:227 | id=2 | generat:873 VT=8 | True
Headroomverteilung.fenster | schema:228 | id=3 | generat:874 VT=10 | True
Frame.transport | schema:240 | id=0 | generat:957 VT=4 | True
Frame.baender | schema:241 | id=1 | generat:958 VT=6 | True
Frame.metrics_version | schema:242 | id=2 | generat:959 VT=8 | True
Frame.aktivitaet | schema:243 | id=3 | generat:960 VT=10 | True
Frame.lufs_s | schema:244 | id=4 | generat:961 VT=12 | True
Frame.peak_db | schema:245 | id=5 | generat:962 VT=14 | True
Frame.crest_db | schema:246 | id=6 | generat:963 VT=16 | True
Frame.psr_db | schema:247 | id=7 | generat:964 VT=18 | True
Frame.breite | schema:248 | id=8 | generat:965 VT=20 | True
Frame.korrelation | schema:249 | id=9 | generat:966 VT=22 | True
Frame.band_stereo | schema:253 | id=10 | generat:967 VT=24 | True
Frame.lufs_i | schema:257 | id=11 | generat:968 VT=26 | True
Frame.lufs_i_unsicherheit_lu | schema:258 | id=12 | generat:969 VT=28 | True
Frame.lufs_i_status | schema:261 | id=13 | generat:970 VT=30 | True
Frame.integration_samples | schema:275 | id=14 | generat:971 VT=32 | True
Frame.lufs_m | schema:281 | id=15 | generat:972 VT=34 | True
Frame.true_peak_db | schema:289 | id=16 | generat:973 VT=36 | True
Frame.true_peak_passage_db | schema:296 | id=17 | generat:974 VT=38 | True
Frame.plr_db | schema:303 | id=18 | generat:975 VT=40 | True
Frame.lra_lu | schema:309 | id=19 | generat:976 VT=42 | True
Frame.crest_kurz_db | schema:316 | id=20 | generat:977 VT=44 | True
Frame.headroom | schema:318 | id=21 | generat:978 VT=46 | True
Frame.band_dynamic_gain_db | schema:338 | id=22 | generat:979 VT=48 | True
QuellenEintrag.quelle | schema:343 | id=0 | generat:1362 VT=4 | True
QuellenEintrag.frame | schema:344 | id=1 | generat:1363 VT=6 | True
FeatureBatch.eintraege | schema:362 | id=0 | generat:1438 VT=4 | True
```

### 128 constexpr-Definitionen und alle zugeordneten Code-Leser

Pfadkürzel in dieser Tabelle sind relativ zu eq-copilot/plugin/. Die Definitionsstelle selbst zählt nicht als Leser. Kommentare und gleichnamige Symbole anderer Namespaces wurden aus den lexicalen Treffern entfernt.

| Definition | Name | Leser im Produkt-/Testcode |
| --- | --- | --- |
| src/Diagnose.h:37 | kDiagnoseVersion | src/prozessor/Analyse.cpp:1339; tests/GoldenTestMain.cpp:881 |
| src/EqCopilotAssetKit.h:22 | kEinheitB | src/EqCopilotAssetKit.h:443; src/PluginEditor.h:106; src/PluginEditor.cpp:225; src/PluginEditor.cpp:868 |
| src/EqCopilotAssetKit.h:23 | kEinheitH | src/PluginEditor.cpp:225; src/PluginEditor.cpp:868 |
| src/EqCopilotAssetKit.h:26 | kAussenX | src/EqCopilotAssetKit.h:450 |
| src/EqCopilotAssetKit.h:27 | kAussenY | src/EqCopilotAssetKit.h:450 |
| src/EqCopilotAssetKit.h:28 | kAussenB | src/EqCopilotAssetKit.h:451 |
| src/EqCopilotAssetKit.h:29 | kAussenH | src/EqCopilotAssetKit.h:451 |
| src/EqCopilotAssetKit.h:30 | kKopfY | src/EqCopilotAssetKit.h:455 |
| src/EqCopilotAssetKit.h:31 | kKopfH | src/EqCopilotAssetKit.h:455 |
| src/EqCopilotAssetKit.h:35 | kDisplayX | src/EqCopilotAssetKit.h:466 |
| src/EqCopilotAssetKit.h:36 | kDisplayY | src/EqCopilotAssetKit.h:466 |
| src/EqCopilotAssetKit.h:37 | kDisplayB | src/EqCopilotAssetKit.h:466 |
| src/EqCopilotAssetKit.h:38 | kDisplayH | src/EqCopilotAssetKit.h:466 |
| src/EqCopilotAssetKit.h:39 | kGraphX | src/PluginEditor.cpp:992 |
| src/EqCopilotAssetKit.h:40 | kGraphY | src/PluginEditor.cpp:992 |
| src/EqCopilotAssetKit.h:41 | kGraphB | src/PluginEditor.cpp:993 |
| src/EqCopilotAssetKit.h:42 | kGraphH | src/PluginEditor.cpp:993 |
| src/EqCopilotAssetKit.h:44 | kMeterLinksX | src/PluginEditor.cpp:1374 |
| src/EqCopilotAssetKit.h:45 | kMeterRechtsX | src/PluginEditor.cpp:1377 |
| src/EqCopilotAssetKit.h:46 | kMeterY | src/PluginEditor.cpp:1374; src/PluginEditor.cpp:1377 |
| src/EqCopilotAssetKit.h:47 | kMeterB | src/PluginEditor.cpp:1375; src/PluginEditor.cpp:1378 |
| src/EqCopilotAssetKit.h:48 | kMeterH | src/PluginEditor.cpp:1375; src/PluginEditor.cpp:1378 |
| src/EqCopilotAssetKit.h:50 | kWerkzeugX | src/EqCopilotAssetKit.h:467; src/PluginEditor.cpp:1017 |
| src/EqCopilotAssetKit.h:51 | kWerkzeugY | src/EqCopilotAssetKit.h:467; src/PluginEditor.cpp:1018 |
| src/EqCopilotAssetKit.h:52 | kWerkzeugB | src/EqCopilotAssetKit.h:467; src/PluginEditor.cpp:1019 |
| src/EqCopilotAssetKit.h:53 | kWerkzeugH | src/EqCopilotAssetKit.h:467; src/PluginEditor.cpp:1020 |
| src/EqCopilotAssetKit.h:55 | kStatusX | src/EqCopilotAssetKit.h:468; src/PluginEditor.cpp:1035; src/PluginEditor.cpp:1040 |
| src/EqCopilotAssetKit.h:56 | kStatusY | src/EqCopilotAssetKit.h:468; src/PluginEditor.cpp:1036; src/PluginEditor.cpp:1041; src/PluginEditor.cpp:1604; src/PluginEditor.cpp:1608; src/PluginEditor.cpp:1651 |
| src/EqCopilotAssetKit.h:57 | kStatusB | src/EqCopilotAssetKit.h:468; src/PluginEditor.cpp:1035 |
| src/EqCopilotAssetKit.h:58 | kStatusH | src/EqCopilotAssetKit.h:468; src/PluginEditor.cpp:1037; src/PluginEditor.cpp:1042; src/PluginEditor.cpp:1604; src/PluginEditor.cpp:1608; src/PluginEditor.cpp:1653 |
| src/EqCopilotAssetKit.h:372 | segmente | src/EqCopilotAssetKit.h:374; src/EqCopilotAssetKit.h:375; src/EqCopilotAssetKit.h:376; src/EqCopilotAssetKit.h:381; src/EqCopilotAssetKit.h:383 |
| src/EqCopilotIds.h:11 | kProtocolVersion | src/PipeClient.cpp:141; src/PipeClient.cpp:526 |
| src/EqCopilotIds.h:14 | kPluginVersion | src/PipeClient.cpp:142; src/prozessor/Ipc.cpp:477 |
| src/EqCopilotIds.h:18 | kPipeName | probe/PipeProbeMain.cpp:113; src/PipeClient.cpp:221 |
| src/EqCopilotIds.h:21 | kMaxFrameBytes | src/PipeClient.cpp:645; src/PipeClient.cpp:667 |
| src/EqCopilotIds.h:23 | kHeartbeatMs | src/PipeClient.cpp:608 |
| src/EqCopilotIds.h:24 | kBackoffStartMs | src/PipeClient.cpp:290; src/PipeClient.cpp:299; src/PipeClient.cpp:303; src/PipeClient.cpp:318; src/PipeClient.cpp:331 |
| src/EqCopilotIds.h:25 | kBackoffMaxMs | src/PipeClient.cpp:334 |
| src/LeitstandTokens.h:9 | flaechen_grund | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:10 | flaechen_station | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:11 | flaechen_bett | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:12 | flaechen_kachel | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:13 | flaechen_kachel_a | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:14 | flaechen_kachel_b | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:15 | flaechen_ueberlage | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:16 | flaechen_kante | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:17 | text_tinte | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:18 | text_tinte_weich | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:19 | text_tinte_leise | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:20 | ampel_bernstein | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:21 | ampel_bernstein_text | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:22 | ampel_bernstein_wasch | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:23 | ampel_gruen | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:24 | ampel_gruen_wasch | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:25 | ampel_rot | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:26 | ampel_rot_text | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:27 | analyse_mess | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:28 | analyse_vergleich | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:29 | analyse_chor | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:30 | analyse_nachmessung | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:31 | analyse_drums | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:32 | analyse_geist | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:33 | nakama_buehne | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:34 | nakama_gehaeuse | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:35 | nakama_platte | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:36 | nakama_platte_insel | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:37 | nakama_platte_kontur | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:38 | nakama_schriftzug | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:39 | nakama_display_grund | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:40 | nakama_display_tief | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:41 | nakama_achse_text | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:42 | nakama_kurve_mess | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:43 | nakama_kurve_live | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:44 | nakama_kurve_vergleich | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:45 | nakama_marker | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:46 | nakama_text_hell | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:47 | nakama_text_weich | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:48 | nakama_text_leise | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:49 | nakama_knopf | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:50 | nakama_knopf_aktiv | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:51 | nakama_kante | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:52 | nakama_feld | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:53 | nakama_ueberlage | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:54 | nakama_hinweis_bg | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:55 | nakama_hinweis_text | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:56 | nakama_led_gruen | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:57 | nakama_led_bernstein | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:58 | nakama_led_rot | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:59 | nakama_konflikt_rot | Kein Leser außerhalb der Definition gefunden. |
| src/LeitstandTokens.h:60 | copilot_stage | src/EqCopilotAssetKit.h:449 |
| src/LeitstandTokens.h:61 | copilot_header_top | src/EqCopilotAssetKit.h:150 |
| src/LeitstandTokens.h:62 | copilot_header_bottom | src/EqCopilotAssetKit.h:151 |
| src/LeitstandTokens.h:63 | copilot_panel_top | src/EqCopilotAssetKit.h:86 |
| src/LeitstandTokens.h:64 | copilot_panel_bottom | src/EqCopilotAssetKit.h:87 |
| src/LeitstandTokens.h:65 | copilot_panel_edge | src/EqCopilotAssetKit.h:108; src/EqCopilotAssetKit.h:118 |
| src/LeitstandTokens.h:66 | copilot_panel_light | src/EqCopilotAssetKit.h:106; src/EqCopilotAssetKit.h:117 |
| src/LeitstandTokens.h:67 | copilot_ink | src/EqCopilotAssetKit.h:361 |
| src/LeitstandTokens.h:68 | copilot_ink_soft | src/EqCopilotAssetKit.h:390; src/EqCopilotAssetKit.h:458 |
| src/LeitstandTokens.h:69 | copilot_text_light | src/EqCopilotAssetKit.h:163; src/EqCopilotAssetKit.h:164; src/EqCopilotAssetKit.h:209; src/EqCopilotAssetKit.h:411; src/PluginEditor.cpp:25; src/PluginEditor.cpp:27; src/PluginEditor.cpp:91; src/PluginEditor.cpp:168; src/PluginEditor.cpp:328; src/PluginEditor.cpp:407; src/PluginEditor.cpp:412; src/PluginEditor.cpp:430; src/PluginEditor.cpp:433; src/PluginEditor.cpp:450; src/PluginEditor.cpp:553; src/PluginEditor.cpp:559; src/PluginEditor.cpp:637; src/PluginEditor.cpp:1658 |
| src/LeitstandTokens.h:70 | copilot_text_soft | src/PluginEditor.cpp:417; src/PluginEditor.cpp:431; src/PluginEditor.cpp:432; src/PluginEditor.cpp:434; src/PluginEditor.cpp:435; src/PluginEditor.cpp:650; src/PluginEditor.cpp:1558 |
| src/LeitstandTokens.h:71 | copilot_text_muted | src/EqCopilotAssetKit.h:168; src/EqCopilotAssetKit.h:174; src/EqCopilotAssetKit.h:205; src/EqCopilotAssetKit.h:406; src/PluginEditor.cpp:392; src/PluginEditor.cpp:428; src/PluginEditor.cpp:470; src/PluginEditor.cpp:642; src/PluginEditor.cpp:667; src/PluginEditor.cpp:1390; src/PluginEditor.cpp:1504 |
| src/LeitstandTokens.h:72 | copilot_accent | src/EqCopilotAssetKit.h:144; src/EqCopilotAssetKit.h:198; src/EqCopilotAssetKit.h:301; src/EqCopilotAssetKit.h:309; src/EqCopilotAssetKit.h:380; src/EqCopilotAssetKit.h:462; src/PluginEditor.cpp:1656 |
| src/LeitstandTokens.h:73 | copilot_accent_soft | src/EqCopilotAssetKit.h:178; src/EqCopilotAssetKit.h:338; src/EqCopilotAssetKit.h:384; src/PluginEditor.cpp:98; src/PluginEditor.cpp:176; src/PluginEditor.cpp:329; src/PluginEditor.cpp:451; src/PluginEditor.cpp:679; src/PluginEditor.cpp:1502 |
| src/LeitstandTokens.h:74 | copilot_display_rim_top | src/EqCopilotAssetKit.h:219 |
| src/LeitstandTokens.h:75 | copilot_display_rim_bottom | src/EqCopilotAssetKit.h:220 |
| src/LeitstandTokens.h:76 | copilot_display_top | src/EqCopilotAssetKit.h:227 |
| src/LeitstandTokens.h:77 | copilot_display_bottom | src/EqCopilotAssetKit.h:228 |
| src/LeitstandTokens.h:78 | copilot_axis_text | src/PluginEditor.cpp:1454; src/PluginEditor.cpp:1465 |
| src/LeitstandTokens.h:79 | copilot_grid | src/PluginEditor.cpp:1452; src/PluginEditor.cpp:1463 |
| src/LeitstandTokens.h:80 | copilot_curve_main | src/PluginEditor.cpp:1527 |
| src/LeitstandTokens.h:81 | copilot_curve_live | src/PluginEditor.cpp:1518 |
| src/LeitstandTokens.h:82 | copilot_curve_compare | src/PluginEditor.cpp:1407; src/PluginEditor.cpp:1521 |
| src/LeitstandTokens.h:83 | copilot_marker | src/EqCopilotAssetKit.h:424; src/PluginEditor.cpp:1482 |
| src/LeitstandTokens.h:84 | copilot_well_top | src/EqCopilotAssetKit.h:263 |
| src/LeitstandTokens.h:85 | copilot_well_bottom | src/EqCopilotAssetKit.h:264 |
| src/LeitstandTokens.h:86 | copilot_status_top | src/EqCopilotAssetKit.h:277 |
| src/LeitstandTokens.h:87 | copilot_status_bottom | src/EqCopilotAssetKit.h:278 |
| src/LeitstandTokens.h:88 | copilot_control | src/EqCopilotAssetKit.h:193; src/EqCopilotAssetKit.h:289; src/EqCopilotAssetKit.h:332; src/PluginEditor.cpp:95; src/PluginEditor.cpp:114; src/PluginEditor.cpp:325; src/PluginEditor.cpp:448; src/PluginEditor.cpp:664 |
| src/LeitstandTokens.h:89 | copilot_control_active | src/PluginEditor.cpp:26; src/PluginEditor.cpp:115; src/PluginEditor.cpp:449 |
| src/LeitstandTokens.h:90 | copilot_control_border | src/EqCopilotAssetKit.h:302; src/PluginEditor.cpp:83; src/PluginEditor.cpp:97; src/PluginEditor.cpp:487; src/PluginEditor.cpp:666; src/PluginEditor.cpp:678 |
| src/LeitstandTokens.h:91 | copilot_field | src/EqCopilotAssetKit.h:192; src/EqCopilotAssetKit.h:369; src/PluginEditor.cpp:675; src/PluginEditor.cpp:1654 |
| src/LeitstandTokens.h:92 | copilot_overlay | src/PluginEditor.cpp:24; src/PluginEditor.cpp:81 |
| src/LeitstandTokens.h:93 | copilot_warning_bg | src/PluginEditor.cpp:175; src/PluginEditor.cpp:326 |
| src/LeitstandTokens.h:94 | copilot_fault | src/PluginEditor.cpp:167; src/PluginEditor.cpp:558 |
| src/LeitstandTokens.h:95 | copilot_meter_off | src/EqCopilotAssetKit.h:380 |
| src/LeitstandTokens.h:96 | copilot_led_green | src/PluginEditor.cpp:1577; src/PluginEditor.cpp:1621 |
| src/LeitstandTokens.h:97 | copilot_led_amber | src/PluginEditor.cpp:1580; src/PluginEditor.cpp:1630 |
| src/LeitstandTokens.h:98 | copilot_led_red | src/EqCopilotAssetKit.h:382; src/PluginEditor.cpp:1583; src/PluginEditor.cpp:1588; src/PluginEditor.cpp:1625 |

### Acht Enumwerte von Diagnose.h

BefundKlasse: resonanz (:41), mittenLoch (:42), mulm (:43), haerte (:44), hoehenHype (:45).
Konfidenz: niedrig, mittel, hoch (alle :48). Vollständige qualifizierte Verwendungsfundstellen in src/ und tests/:

```text
eq-copilot/plugin/tests/GoldenTestMain.cpp:171:        p.wahr (bs.size() == 1 && bs.front().klasse == BefundKlasse::resonanz,
eq-copilot/plugin/tests/GoldenTestMain.cpp:177:        p.wahr (hatKlasse (bs, BefundKlasse::resonanz), "116-Hz-Karte vorhanden", "fehlt");
eq-copilot/plugin/tests/GoldenTestMain.cpp:178:        p.wahr (! hatKlasse (bs, BefundKlasse::mittenLoch)
eq-copilot/plugin/tests/GoldenTestMain.cpp:179:                    && ! hatKlasse (bs, BefundKlasse::haerte),
eq-copilot/plugin/tests/GoldenTestMain.cpp:182:            if (b.klasse == BefundKlasse::resonanz)
eq-copilot/plugin/tests/GoldenTestMain.cpp:197:                p.wahr (b.konfidenz == Konfidenz::hoch,
eq-copilot/plugin/tests/GoldenTestMain.cpp:204:        p.wahr (hatKlasse (bs, BefundKlasse::mittenLoch), "Mitten-Loch gefunden",
eq-copilot/plugin/tests/GoldenTestMain.cpp:206:        p.wahr (! hatKlasse (bs, BefundKlasse::mulm) && ! hatKlasse (bs, BefundKlasse::haerte),
eq-copilot/plugin/tests/GoldenTestMain.cpp:209:            if (b.klasse == BefundKlasse::mittenLoch)
eq-copilot/plugin/tests/GoldenTestMain.cpp:216:                p.wahr (b.konfidenz != Konfidenz::niedrig && b.topRang,
eq-copilot/plugin/tests/GoldenTestMain.cpp:222:        p.wahr (hatKlasse (bs, BefundKlasse::mulm), "Mulm gefunden",
eq-copilot/plugin/tests/GoldenTestMain.cpp:224:        p.wahr (! hatKlasse (bs, BefundKlasse::mittenLoch) && ! hatKlasse (bs, BefundKlasse::haerte),
eq-copilot/plugin/tests/GoldenTestMain.cpp:227:            if (b.klasse == BefundKlasse::mulm)
eq-copilot/plugin/tests/GoldenTestMain.cpp:239:        p.wahr (hatKlasse (bs, BefundKlasse::haerte), "Haerte gefunden",
eq-copilot/plugin/tests/GoldenTestMain.cpp:241:        p.wahr (! hatKlasse (bs, BefundKlasse::mulm), "kein Mulm an der Haerte", "faelschlich gemeldet");
eq-copilot/plugin/tests/GoldenTestMain.cpp:243:            if (b.klasse == BefundKlasse::haerte)
eq-copilot/plugin/tests/GoldenTestMain.cpp:252:        p.wahr (hatKlasse (bs, BefundKlasse::mulm), "Halbzeit-Mulm gefunden",
eq-copilot/plugin/tests/GoldenTestMain.cpp:254:        p.wahr (! hatKlasse (bs, BefundKlasse::mittenLoch) && ! hatKlasse (bs, BefundKlasse::haerte),
eq-copilot/plugin/tests/GoldenTestMain.cpp:257:            if (b.klasse == BefundKlasse::mulm)
eq-copilot/plugin/tests/GoldenTestMain.cpp:263:        p.wahr (hatKlasse (bs, BefundKlasse::resonanz), "Resonanz-Karte vorhanden", "fehlt");
eq-copilot/plugin/tests/GoldenTestMain.cpp:265:            if (b.klasse == BefundKlasse::resonanz)
eq-copilot/plugin/src/Diagnose.h:54:    BefundKlasse klasse = BefundKlasse::resonanz;
eq-copilot/plugin/src/Diagnose.h:69:    Konfidenz konfidenz = Konfidenz::niedrig;
eq-copilot/plugin/src/Diagnose.cpp:40:        case BefundKlasse::mittenLoch: return kZonenSlotMittenLoch;
eq-copilot/plugin/src/Diagnose.cpp:41:        case BefundKlasse::mulm:       return kZonenSlotMulm;
eq-copilot/plugin/src/Diagnose.cpp:42:        case BefundKlasse::haerte:     return kZonenSlotHaerte;
eq-copilot/plugin/src/Diagnose.cpp:43:        case BefundKlasse::hoehenHype: return kZonenSlotHoehenHype;
eq-copilot/plugin/src/Diagnose.cpp:44:        case BefundKlasse::resonanz:   break;
eq-copilot/plugin/src/Diagnose.cpp:142:    b.konfidenz = (minW >= 0.6 && mittel >= 0.75) ? Konfidenz::hoch
eq-copilot/plugin/src/Diagnose.cpp:143:                : (minW >= 0.4 && mittel >= 0.55) ? Konfidenz::mittel
eq-copilot/plugin/src/Diagnose.cpp:144:                                                  : Konfidenz::niedrig;
eq-copilot/plugin/src/Diagnose.cpp:147:    const char* stufe = b.konfidenz == Konfidenz::hoch ? "hoch"
eq-copilot/plugin/src/Diagnose.cpp:148:                      : b.konfidenz == Konfidenz::mittel ? "mittel" : "niedrig";
eq-copilot/plugin/src/Diagnose.cpp:159:    b.klasse = BefundKlasse::resonanz;
eq-copilot/plugin/src/Diagnose.cpp:366:        if (zonenBefund (m, { BefundKlasse::mittenLoch,
eq-copilot/plugin/src/Diagnose.cpp:374:                         + zeitverlaufText (m, BefundKlasse::mittenLoch);
eq-copilot/plugin/src/Diagnose.cpp:393:        if (zonenBefund (m, { BefundKlasse::mulm,
eq-copilot/plugin/src/Diagnose.cpp:401:                         + zeitverlaufText (m, BefundKlasse::mulm);
eq-copilot/plugin/src/Diagnose.cpp:418:        if (zonenBefund (m, { BefundKlasse::haerte,
eq-copilot/plugin/src/Diagnose.cpp:426:                         + zeitverlaufText (m, BefundKlasse::haerte);
eq-copilot/plugin/src/Diagnose.cpp:450:            b.klasse = BefundKlasse::hoehenHype;
eq-copilot/plugin/src/Diagnose.cpp:459:                         + zeitverlaufText (m, BefundKlasse::hoehenHype);
eq-copilot/plugin/src/Diagnose.cpp:471:            kt.persistenz = zonenPersistenz (m, BefundKlasse::hoehenHype);   // M3a
eq-copilot/plugin/src/Diagnose.cpp:485:            case BefundKlasse::resonanz:   return b.dauerhaft ? 0 : 5;
eq-copilot/plugin/src/Diagnose.cpp:486:            case BefundKlasse::mittenLoch: return 1;
eq-copilot/plugin/src/Diagnose.cpp:487:            case BefundKlasse::mulm:       return 2;
eq-copilot/plugin/src/Diagnose.cpp:488:            case BefundKlasse::haerte:     return 3;
eq-copilot/plugin/src/Diagnose.cpp:489:            case BefundKlasse::hoehenHype: return 4;
eq-copilot/plugin/src/Diagnose.cpp:502:        if (b.konfidenz != Konfidenz::niedrig && top < 3)
eq-copilot/plugin/src/PluginEditor.h:157:    BefundKlasse markKlasse = BefundKlasse::resonanz;
eq-copilot/plugin/src/PluginEditor.cpp:464:                    if (b.klasse != BefundKlasse::mittenLoch)
eq-copilot/plugin/src/PluginEditor.cpp:765:    w.istResonanz = b.klasse == BefundKlasse::resonanz;
eq-copilot/plugin/src/prozessor/Analyse.cpp:108:        o->setProperty ("klasse", b.klasse == BefundKlasse::resonanz ? "resonanz"
eq-copilot/plugin/src/prozessor/Analyse.cpp:109:                                : b.klasse == BefundKlasse::mittenLoch ? "mitten_loch"
eq-copilot/plugin/src/prozessor/Analyse.cpp:110:                                : b.klasse == BefundKlasse::mulm ? "mulm"
eq-copilot/plugin/src/prozessor/Analyse.cpp:111:                                : b.klasse == BefundKlasse::haerte ? "haerte"
eq-copilot/plugin/src/prozessor/Analyse.cpp:123:        o->setProperty ("konfidenz", b.konfidenz == Konfidenz::hoch ? "hoch"
eq-copilot/plugin/src/prozessor/Analyse.cpp:124:                                   : b.konfidenz == Konfidenz::mittel ? "mittel" : "niedrig");
```

## Widerlegte Verdachte

Die folgenden **14 Verdachte** wurden nicht als weitere Befunde übernommen:

1. **„Ein Schemaedit muss beim normalen Plugin-Bau automatisch flatc starten.“** Der Vertrag verwendet eingecheckte Generatdateien; A9 in pruefe_flatc_drift.py:174/:340 erzeugt und vergleicht beide Sprachziele. Das Fehlen eines normalen Schema→Header-Custom-Commands ist daher kein eigener Vertragsbruch. B01 bleibt davon getrennt bekannt.
2. **„Der NAK-280-Zeiger ist in dieser Sitzung erneut tot oder sein Beleg beschreibt eine andere flatc-Binary.“** Ziel vorhanden; Belegcommit und gelesener SHA stimmen. Kein erneuter NAK-280-Befund, keine flatc-Ausführung.
3. **„Cargo.lock lässt schon heute eine Git-Abhängigkeit frei treiben oder zwei syn-Versionen sind ein Defekt.“** Ganze 66-Paketpopulation gelesen: 65 Registry-Checksums, kein Git/patch/replace. syn 2/3 liegen in verschiedenen Makroketten; kein inkompatibler gemeinsamer API-Verbraucher hergeleitet.
4. **„Aus einem fehlenden Compilerpin folgt hier bereits eine belegte Bitabweichung.“** Toolchain ist nicht exakt gepinnt; Rust-/Cargo-Versionen werden aber im Kanon protokolliert (beweise.ps1:948). Keine Kompilierung und kein Audiovergleich in dieser Phase, daher keine solche Kausalbehauptung. NAK-175/B03 werden nicht als neue Beobachtung ausgegeben.
5. **„v3-PipeToken kann bei normaler SID-Ableitung denselben Namen wie die Probe liefern.“** Produktalphabet/Base32 gegen expliziten .probe.-Präfix, PipeToken.h:59 und PipeToken.cpp:128/:142, schließen diese Kollision aus. T3-13-01 liegt im anderen v2-Pfad.
6. **„Halber v2-Handshake oder belegte Pipe genügt für Stufe-1-PASS.“** PipeProbe:139 verlangt verbunden/v2/zwei ACKs; PipeClient hat begrenzte Verbindungs- und Framefristen. Kein solcher PASS hergeleitet. Das schwächere Abschlussprädikat bleibt T3-13-04.
7. **„Eine normale neue Kernquelle bleibt allein wegen gemeinsamer Listen von A14 unbemerkt.“** CMake-Quellliste und A14-Objektpopulation sind unabhängig; 21 gegen 21. K2/K2b/K2c hängen am echten Zielgraphen. Absichtliches gleichzeitiges Umbauen aller Wächter ist ausgeschlossenes Sabotagemodell.
8. **„setPosition meldet seine Echoposition als aktuelle FL-Position.“** Controller:383 bezeichnet sie requested_position; getPosition:390 liest getrennt. M-18 verlangt gerade diese Trennung; externe Controller-Tests prüfen beide Funktionen.
9. **„volume_db wird aus einem normierten Pegel geraten.“** mixer.getTrackInfo:442 und channels.getInfo lesen den dB-Modus der FL-API. Routing bleibt explizit normiert (Controller:587); tests/test_controller_probe_handlers.py und test_mixer_routing.py prüfen die jeweilige Semantik.
10. **„Mehrere loopMIDI-Ports werden still auf den ersten reduziert.“** midi_connection.py und tests/test_port_selection.py unterscheiden explizite Portwahl, einzigen passenden Port und Mehrdeutigkeit; letzterer Fall scheitert ehrlich. NAK-307 zur gemeinsamen Antwortdatei bleibt davon unberührt.
11. **„Fenstererfassung benötigt aktuell Fokuswechsel oder der indirekte Import führt bereits einen Tastendruck aus.“** fenster.py verwendet PrintWindow und tests/test_fenster.py überwacht die Fokusgrenze; fl_trigger importiert pynput erst in Methoden (:67/:158). Der reine Importvertrag bricht dennoch (T3-13-05).
12. **„Die 51 unbenutzten Leitstandwerte oder gleichnamige Namespacekonstanten erzeugen einen Produktfehler.“** Komplette Konstanten-/Leserpopulation im Anhang; unbenutzte Farben werden nicht gelesen, gegenwärtige Frame-/Backoffduplikate stimmen. Kein Laufzeitfehler und kein Auftrag zur Designbereinigung.
13. **„Sensoruebersicht.svelte ist eine bisher unbewachte ausgelieferte UI.“** Vollständig gelesen und Referenzsuche über Bau/Test durchgeführt; kein entsprechender Einbau gefunden. Die Datei ist eine Referenzfläche, keine neu zu auditierende Designrichtung.
14. **„antworten_blatt.py setzt einen falschen Ticketfortschritt gegenüber planstand.py.“** Beide Dateien vollständig gelesen: Antwortenbearbeitung und Plan-/Beweisstatus stammen aus verschiedenen dafür benannten JSON-Populationen. Kein hier hergeleiteter falscher Abnahmewert; auftrag-lebenslauf-audit.md ist nur ein Muster.

Zusätzlich ausdrücklich bekannt und nicht neu gezählt: T3-09-01/B01 (Frischeriegel), T3-05-03/B03 (Baugenerationen), NAK-175 (Stack), NAK-280 (flatc-Zeiger), NAK-286 (Laufzeitaufbau), NAK-290 (parallele Kanons), NAK-297 (leerer Fenstertitel), NAK-305 (Parameter-API), NAK-306 (aktive Referenz) und T3-09-02 (Nulltest/Signed-zero-Grenze). NAK-307 erhält in T3-13-03 nur die dort ausdrücklich abgegrenzte neue Ableitung.

## Produktfragen

Keine. Die fehlenden Eigentums-/Korrelationsregeln sind technische Vertragsgrenzen; die vorhandenen Produktzusagen reichen für die genannten Vorschläge aus.

## Nicht geprüft

- Keine Ausführung von Kanon, Tests, CMake, Cargo, flatc, uv, FL, Controller oder MCP-Server; keine Installation und keine Projekt-/Stateänderung.
- Kein dynamischer Reproduktionsnachweis der sechs Befunde und keine Zusicherung, dass ein aktueller Hostlauf betroffen war.
- Keine frische Generierung und kein byteweiser Neugenerierungsvergleich; der statische Vergleich der 64 Felder und der vorhandene Generatorhash sind enger.
- Kein erneutes Audio-/DSP-/Realtime-Audit der vollständigen Kernimplementierungen; hier wurden ihre Bauzuordnung und die benannten Nahtstellen untersucht.
- Kein vollständiges Audit von JUCE, VST3-SDK, Rust-Compiler, MSVC oder installierten Python-Paketen. Versions-/Patch-/Aufrufstellen wurden gelesen.
- Externes uv.lock (413879 Bytes, im externen Git nicht als Datei geführt) nur auf Existenz/Versionierungsstatus geprüft, nicht vollständig inhaltlich gelesen; .venv und Paketcache nicht geprüft.
- FL-Projektbinärinhalte nicht dekodiert; nur Namen/Hashbezüge. Die installierte Controllerkopie wurde gehasht und gegen die vollständig gelesene Quelle verglichen, nicht ausgeführt.
- Keine Bewertung geparkter Designs/Studien, Legacy-Namen, Register-/Manifestprosa oder des Arbeitslisteninhalts.
- Das externe .pytest_cache-Verzeichnis war beim Git-Status nicht lesbar; es ist kein erforderlicher Quellpfad. Ein vollständiger Sauberkeitsnachweis sämtlicher dortiger unversionierter Dateien wird deshalb nicht behauptet.

## Scope-Beweis

**64 Dateien der definierten Prüfpopulation und ihrer direkten Aufruf-/Testpaare vollständig gelesen: 43 in Nakama, 21 im externen MCP-Repo.** Davon sind alle 14 Restdateien aus Phase 11 einzeln unten markiert. Zusätzlich sieben Kontextdateien vollständig gelesen; neun weitere Kontext-/Gate-/Riegeldateien gezielt gelesen. Drei kleine Beleg-/Bootdateien vollständig gelesen; binäre Dateien nur nach den ausdrücklich angegebenen Hash-/Versionsmerkmalen.

Die Volllektüre wurde abschnittsweise mit nummeriertem Get-Content durchgeführt; bei gekürzter Toolausgabe wurden die betreffenden Blöcke nachgelesen. rg-Inventare ergänzen die Volllektüre, ersetzen sie nicht. Die folgenden Dateizahlen bezeichnen gelesene Dateien, nicht eine Behauptung vollständiger Prüfung sämtlicher Abhängigkeiten.

### Alle 14 bislang ungelesenen Dateien aus A

| Pfad | Zeilen | Marke |
| --- | ---: | --- |
| broker/Cargo.lock | 549 | gelesen — vollständig |
| broker/sensoruebersicht-referenz/Sensoruebersicht.svelte | 475 | gelesen — vollständig |
| eq-copilot/cmake/NakamaFlatBuffers.cmake | 227 | gelesen — vollständig |
| eq-copilot/cmake/NakamaFlatBuffersProjektHook.cmake | 11 | gelesen — vollständig |
| eq-copilot/cmake/NakamaFlatcBeleg.cmake | 111 | gelesen — vollständig |
| eq-copilot/cmake/NakamaKern.cmake | 1671 | gelesen — vollständig |
| eq-copilot/cmake/pruefe_nakama_kern_genex.cmake | 583 | gelesen — vollständig |
| eq-copilot/plugin/probe/PipeProbeMain.cpp | 212 | gelesen — vollständig |
| eq-copilot/plugin/src/Diagnose.h | 83 | gelesen — vollständig |
| eq-copilot/plugin/src/EqCopilotAssetKit.h | 477 | gelesen — vollständig |
| eq-copilot/plugin/src/EqCopilotIds.h | 26 | gelesen — vollständig |
| eq-copilot/plugin/src/LeitstandTokens.h | 101 | gelesen — vollständig |
| tools/plan/antworten_blatt.py | 113 | gelesen — vollständig |
| tools/pruefung/auftrag-lebenslauf-audit.md | 73 | gelesen — vollständig |

### Weitere 29 vollständig gelesene Nakama-Dateien

| Pfad | Zeilen | Marke |
| --- | ---: | --- |
| broker/Cargo.toml | 82 | gelesen — vollständig |
| eq-copilot/plugin/core/ipc/PipeToken.h | 88 | gelesen — vollständig |
| eq-copilot/plugin/core/ipc/PipeToken.cpp | 184 | gelesen — vollständig |
| eq-copilot/cmake/NakamaBruecke.cmake | 127 | gelesen — vollständig |
| eq-copilot/cmake/NakamaIdentitaet.cmake | 257 | gelesen — vollständig |
| eq-copilot/CMakeLists.txt | 64 | gelesen — vollständig |
| eq-copilot/plugin/CMakeLists.txt | 1386 | gelesen — vollständig |
| tools/fl/laufzeit.ps1 | 1613 | gelesen — vollständig |
| tools/fl/szenario.py | 3492 | gelesen — vollständig |
| tools/fl/nulltest.py | 897 | gelesen — vollständig |
| tools/fl/selbsttest.py | 82 | gelesen — vollständig |
| tools/fl/LIES-MICH.md | 145 | gelesen — vollständig |
| eq-copilot/fixtures/fl/LIES-MICH.md | 38 | gelesen — vollständig |
| eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs | 365 | gelesen — vollständig |
| eq-copilot/schemas/v3/flatbuffers/WERKZEUG.json | 39 | gelesen — vollständig |
| eq-copilot/schemas/v3/flatbuffers/FELD-IDS.json | 150 | gelesen — vollständig |
| eq-copilot/schemas/v3/flatbuffers/README.md | 194 | gelesen — vollständig |
| eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h | 1547 | gelesen — vollständig |
| tools/eq-copilot/pruefe_flatc_drift.py | 391 | gelesen — vollständig |
| eq-copilot/plugin/src/PipeClient.h | 174 | gelesen — vollständig |
| eq-copilot/plugin/src/PipeClient.cpp | 824 | gelesen — vollständig |
| tools/eq-copilot/pruefe_kern_identitaetsfrei.py | 779 | gelesen — vollständig |
| tools/plan/planstand.py | 467 | gelesen — vollständig |
| docs/gesundheit/szenarien/bereitschaft.json | 21 | gelesen — vollständig |
| docs/gesundheit/szenarien/fenster.json | 12 | gelesen — vollständig |
| docs/gesundheit/szenarien/nulltest-host.json | 16 | gelesen — vollständig |
| docs/gesundheit/szenarien/snapshot-runde01.json | 33 | gelesen — vollständig |
| docs/gesundheit/szenarien/u40-aktivitaetsgate.json | 17 | gelesen — vollständig |
| docs/gesundheit/szenarien/LIES-MICH.md | 28 | gelesen — vollständig |

### Alle 21 vollständig gelesenen Dateien der externen Gegenseite

Wurzel: C:/Users/phili/Projekte/fl-studio-mcp/.
HEAD vor dem Lauf: **01f72386bfc7eab507a40b9c96ef9e2e2a0265cc**.
HEAD nach dem Lauf: **01f72386bfc7eab507a40b9c96ef9e2e2a0265cc**.
Zweig: evenacadia-local.

| Pfad relativ zur externen Wurzel | Zeilen | Marke |
| --- | ---: | --- |
| setup-local.ps1 | 82 | gelesen — vollständig |
| fl_controller/device_FLStudioMCP.py | 1203 | gelesen — vollständig |
| src/fl_studio_mcp/utils/connection.py | 93 | gelesen — vollständig |
| src/fl_studio_mcp/utils/midi_connection.py | 320 | gelesen — vollständig |
| src/fl_studio_mcp/utils/fl_paths.py | 65 | gelesen — vollständig |
| src/fl_studio_mcp/server.py | 178 | gelesen — vollständig |
| src/fl_studio_mcp/utils/fenster.py | 246 | gelesen — vollständig |
| src/fl_studio_mcp/__init__.py | 3 | gelesen — vollständig |
| src/fl_studio_mcp/utils/__init__.py | 13 | gelesen — vollständig |
| src/fl_studio_mcp/utils/fl_trigger.py | 239 | gelesen — vollständig |
| tests/test_controller_probe_handlers.py | 254 | gelesen — vollständig |
| tests/test_port_selection.py | 155 | gelesen — vollständig |
| src/fl_studio_mcp/tools/transport.py | 162 | gelesen — vollständig |
| src/fl_studio_mcp/tools/mixer.py | 438 | gelesen — vollständig |
| src/fl_studio_mcp/tools/channels.py | 408 | gelesen — vollständig |
| src/fl_studio_mcp/tools/plugins.py | 292 | gelesen — vollständig |
| src/fl_studio_mcp/tools/__init__.py | 15 | gelesen — vollständig |
| tests/test_fenster.py | 222 | gelesen — vollständig |
| tests/test_mixer_routing.py | 231 | gelesen — vollständig |
| pyproject.toml | 64 | gelesen — vollständig |
| src/fl_studio_mcp/tools/piano_roll.py | 410 | gelesen — vollständig |

### Kontext und gezielter Skeptikerpass

Sieben zusätzlich vollständig gelesene Kontextdateien:

| Pfad | Zeilen |
| --- | ---: |
| AGENTS.md | 127 |
| CLAUDE.md | 307 |
| .agents/skills/nakama-plugin-quality/SKILL.md | 117 |
| docs/plugin-wissen.md | 1000 |
| docs/gesundheit/abdeckungskarte.md | 237 |
| docs/gesundheit/lebenslauf-audit.md | 107 |
| docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md | 373 |

Neun weitere Dateien wurden gezielt gelesen beziehungsweise anhand einschlägiger Zeilen gesucht, nicht als vollständige Befundpopulation ausgegeben:

| Pfad | Gelesene / gesuchte Fläche |
| --- | --- |
| docs/gesundheit/KONZEPT.md | §3 und §4.3, einschließlich :120 bis :275; K1–K8 und Spezialauditmechanik |
| docs/offene-punkte.md | Alle zu den Kandidaten passenden Registertreffer; insbesondere NAK-175, 280, 284–286, 290, 297, 305–307; Abgleich gegen neue Befunde |
| docs/FL-Nakama-Sonden-Design-Entwurf.md | Phasen-/Nachweiszusagen :3538 bis :3595, :3873 bis :3934, :4165 bis :4195 und gezielte Suchstellen |
| docs/beweise/NAK-286.md | Abnahmematrizen insbesondere M-03, M-09, M-17 bis M-22, M-78; externe Standangaben und Abschluss/Referenzgrenzen |
| docs/plan/plan.json | Gate S25e :310 bis :314, kein Planprogramm ausgeführt |
| docs/audits/2026-09-15-tiefenaudit/roh/phase-11-synthese.md | Restpopulation/Blind Spots und Synthesekopf; keine neue Bewertung der Dokumentprosa |
| docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md | Bestehende Befunde/Bündel, Suchabgleich, Anhangsgrenze; vorhandener Inhalt bleibt bytegleich |
| eq-copilot/plugin/tests/PipeClientLifecycleTestMain.cpp | :1390 bis :1550; PipeProbe-Ausnahme-/Flushwachen samt Absichtskommentar |
| tools/beweise.ps1 | Aufrufbeine, Bauparameter, Versionserfassung; :855 bis :970 und einschlägige rg-Stellen |

Die Konstantsuche berührte außerdem **18 weitere Produkt-/Testdateien nur an Verwendungs- oder Abgrenzungsstellen**; daraus wird keine Volllektüre ihrer Implementierungen abgeleitet:

- eq-copilot/plugin/src/Diagnose.cpp
- eq-copilot/plugin/src/ZonenRegeln.h
- eq-copilot/plugin/src/prozessor/Analyse.cpp
- eq-copilot/plugin/tests/GoldenTestMain.cpp
- eq-copilot/plugin/src/PluginEditor.h
- eq-copilot/plugin/src/PluginEditor.cpp
- eq-copilot/plugin/src/AnalyseEngine.cpp
- eq-copilot/plugin/src/AnalyseEngine.h
- eq-copilot/plugin/vertrag/NakamaVertrag.cpp
- eq-copilot/plugin/src/prozessor/Ipc.cpp
- eq-copilot/plugin/core/ipc/WireEnvelope.h
- eq-copilot/plugin/core/ipc/WireEnvelope.cpp
- eq-copilot/plugin/tests/IpcTestMain.cpp
- eq-copilot/plugin/tests/V3TestServer.h
- eq-copilot/plugin/tests/SessionSoakMain.cpp
- eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp
- eq-copilot/plugin/core/ipc/TelemetryClient.cpp
- eq-copilot/plugin/core/ipc/IpcVerbindung.h

Zusätzliche Lese-/Metadatenflächen, getrennt von den 64 voll gelesenen Quelldateien:

- eq-copilot/build/nakama-flatc-pfad-Release.txt: vollständig, 1 Zeile.
- eq-copilot/build/nakama-flatc-commit-Release.txt: vollständig, 2 Zeilen.
- C:/Users/phili/OneDrive/Dokumente/Image-Line/FL Studio/Settings/Hardware/FLStudioMCP/mcp_boot.json: vollständig, 5 Zeilen.
- flatc.exe: SHA-256, kein Start; drei .flp-Dateien: Namen und SHA-256 gemäß Tabelle; installierte device_FLStudioMCP.py: Hashvergleich.
- eq-copilot/build/CMakeCache.txt: Generator-/Toolsetstellen; JUCE-Checkout: Git-Stand/Tag; JUCE-VST3-SDK/vsttypes.h: Versionsstelle.
- Suchfläche tools/fl/** und eq-copilot/install/** für MCP-Pin/Hashes; Build-/Testdateien für Svelte- und Genex-Skriptaufrufer; keine vollständige Installerprüfung.
- Externes uv.lock: Dateigröße/Existenz und git ls-files, keine inhaltliche Volllektüre.

### Nicht gelesene und fehlende Pfade

Nicht vollständig gelesen: Rust-Telemetrie-Generat, übrige Broker-/Pluginimplementierungen außerhalb dieser Population, übrige Tests/Fixtures, große Abhängigkeitsbäume unter build/_deps, installierte Pythonpakete/.venv/uv.lock, externe Piano-Roll-Skriptimplementierung fl_controller/ComposeWithLLM.pyscript sowie weitere externe Entwicklungs-/Cachedateien. Die serverseitigen Piano-Roll-Toolwrapper wurden dagegen als Teil der gesamten Serverregistrierung gelesen. Geparkte Studien, Designarchive und sonstige Dokumentbäume blieben außerhalb des Auftrags.

**Fehlend, jeweils durch Dateisuche/Existenzprüfung erfasst:** broker/build.rs; Rust-Toolchain-Dateien rust-toolchain und rust-toolchain.toml im betrachteten Repo-Einstieg; eigene CMake-Toolchain-Datei in den beauftragten CMake-Pfaden. Das sind optionale Dateien, keine Lektüreblocker. Nakama-Diagnose-Verarbeitung.flp fehlt wie unter NAK-306 bekannt. Zwei andere probierte Bootmarkenorte unter C:/Users/phili/Documents und C:/Users/phili/OneDrive/Documents waren nicht vorhanden; der oben benannte OneDrive/Dokumente-Pfad war lesbar. Keine fehlende erforderliche Datei der 14er-Population, kein unlesbarer erforderlicher externer Quellpfad.

### Unveränderter Stand und genau zwei Schreibziele

Nakama-HEAD vor/nach: **1b2faf9926b8236e5b9ef1cb85f2cac0e4071d9d**.
Externer HEAD vor/nach: **01f72386bfc7eab507a40b9c96ef9e2e2a0265cc**.
Abfrage extern mit ausschließlich auf diesen Befehl begrenztem safe.directory; keine Git-Konfiguration geändert.

Vorhandene fremde/unzugeordnete Nakama-Pfade laut Anfangsstatus: .claude/settings.local.json, briefing-hub/, nimbalyst-local/ und roh/phase-13-auftrag.txt. Unverändert belassen. Das externe Git meldete keine Änderung, warnte aber vor dem nicht lesbaren .pytest_cache und der globalen Ignoredatei; deshalb keine umfassende Sauberkeitsbehauptung für unversionierte externe Artefakte.

Eigene Schreibziele ausschließlich:

1. docs/audits/2026-09-15-tiefenaudit/roh/phase-13-bauhostkette.md
2. docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md — ausschließlich neuer Abschnitt am Ende

Der vorherige BEFUNDE.md-Inhalt hatte **181400 Bytes** und SHA-256 **53D692F0E8ABAEF4F58144E631B2DD677737E5240469EE1DCCF76E2B913FBC6F**. Der Abschlussvergleich prüft genau diesen unveränderten Bytepräfix und die wortgleiche sechszeilige Befundtabelle beider Ausgaben. Kein anderer Abschnitt, insbesondere kein Phasenindex, wird nachgezogen. Kein Commit, Push oder Stage; kein ausführender Produkt-/Hostnachweis in dieser Sitzung.

**Abschlussbeleg, 16.09.2026 05:34:23 MESZ (40 min 57 s seit Start):** Beide HEAD-Abfragen liefern die oben genannten unveränderten SHAs. SHA-256 des 181400-Byte-Präfixes stimmt; die beiden Befundtabellen sind wortgleich und enthalten genau sechs Befundzeilen. git diff --check ohne Fehler; BEFUNDE.md ausschließlich 18 hinzugefügte Zeilen, keine Entfernung. Der Abschlussstatus zeigt nur die zwei eigenen Auditziele zusätzlich zum unveränderten fremden Anfangsstatus.
