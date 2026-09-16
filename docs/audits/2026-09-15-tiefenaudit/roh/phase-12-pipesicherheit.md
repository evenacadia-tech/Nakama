URTEIL: DEFEKTE 3 · LÜCKEN 2 · HÄRTUNGEN 0 — Die v3-Kernmechanik sperrt Fremdbenutzer und begrenzt Frames, doch fünf Vertrauenslücken lassen erhöhte Installation, behauptete Clientidentität, Legacy-Verfügbarkeit und Diagnosedaten lokal angreifen.

# Tiefenaudit 3 — Phase 12: Pipe-Sicherheit

| Feld | Wert |
| --- | --- |
| Phase | 12 — Pipe-Sicherheit (Broker und Plugin) |
| Datum | 2026-09-16 |
| HEAD | `d20b8c156a21b52e93ad31a58bcda706e0439ecb` |
| Modell | `gpt-5.6-sol`, Effort `max` |
| Dauer | 00:43 h (02:43–03:26 Uhr, Europe/Berlin) |
| Arbeitsart | Statischer, lesender Sicherheitslauf; kein Bau, kein Test, kein Kanon, keine Installation |

## Befundtabelle

| ID | Schwere | Klasse | Kategorie | Ort | Befund | Beleg | Bricht | Vorschlag | Status |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| T3-12-01 | critical | K-neu: Privileggrenze | DEFEKT | tools/dirigent/install-aufgaben-registrieren.ps1:57; tools/dirigent/install-aufgaben-registrieren.ps1:66; tools/dirigent/install-aufgaben-registrieren.ps1:74; eq-copilot/install/Install-Nakama.ps1:87; eq-copilot/install/Install-Nakama.ps1:101; eq-copilot/install/Install-Nakama.ps1:215; eq-copilot/install/Install-Nakama.ps1:896; eq-copilot/install/Install-Nakama.ps1:908; eq-copilot/install/Install-Nakama.ps1:1147 | bekannt NAK-285: Neu ist die offene Privileggrenze: Die mit höchsten Rechten laufende Aufgabe führt direkt `Install-Nakama.ps1` aus dem für denselben Benutzer beschreibbaren Checkout aus. Dort werden auch Helfer, Manifest und Artefakte aus demselben Checkout geladen; Hashen gegen ein gemeinsam änderbares Manifest und der derzeit leere Signer-Thumbprint bilden keinen unabhängigen Vertrauensanker. Ein nicht erhöhter Prozess desselben Benutzers kann dadurch die erhöhte Aktion umlenken. | Quellkette: Aufgabenaktion und Arbeitsverzeichnis zeigen in den Checkout; `Install-Nakama.ps1` dot-sourct `NakamaOrdnerHash.ps1`, liest Manifest und Identität, prüft Artefakte gegen dieses Manifest und kopiert danach nach Program Files. `tools/eq-copilot/pruefe_installer_manifest.py:1117-1163,1650-1665` schreibt mit `--hashen` genau dieses Manifest; `tools/fl/laufzeit.ps1:402-416` zieht es vor dem Aufgabenstart nach. NAK-119 grenzt nur den bekannten `null`-Thumbprint ab; keine unabhängige Integritätsprüfung der Aufgabenaktion wurde gefunden. | „Code, der bereits unter demselben User in FL injiziert wurde, liegt außerhalb des IPC-Threat-Models; dafür bleiben Signaturprüfung, Installationspfad und Betriebssystemschutz maßgeblich.“ (Gate NAK-123, docs/beweise/NAK-123.md:15) | Die Aufgabe darf nur einen gepinnten oder signierten Helfer aus einem admin-geschützten Installationsort ausführen und nur ein unabhängig signiertes oder gepinntes Paket samt Manifest konsumieren, nie Skript oder Vertrauensanker aus dem Checkout. | offen |
| T3-12-02 | critical | K4 | DEFEKT | broker/src/transport/server_v3/auth.rs:45; broker/src/transport/server_v3/auth.rs:103; broker/src/transport/server_v3/auth.rs:168; broker/src/transport/bootstrap.rs:149; broker/src/transport/server_v3/verbindung.rs:253; broker/src/transport/server_v3/verbindung.rs:300; broker/src/coordinator/zustand.rs:10; broker/src/coordinator/link.rs:80; broker/src/coordinator/mod.rs:457; broker/src/lebenslauf.rs:142; broker/src/server.rs:691; broker/src/server.rs:786 | bekannt NAK-152: Neu ist, dass die erfolgreiche Windows-Authentisierung nur ein `bool` liefert und nicht an die im Hello behauptete Logon-SID, Sensor-ID, Pluginart oder Host-PID gebunden wird. Jeder Prozess desselben Benutzers kann deshalb eine fremde Drahtidentität anmelden; im v2-Pfad erhält jedes syntaktisch gültige Hello zusätzlich denselben Sitzungstoken, der erst bei späteren Heartbeat-/Bye-Nachrichten geprüft wird. | `authentifiziere_client` vergleicht den Pipe-Token mit der erwarteten User-SID, verwirft die kanonische SID aber vor dem Bootstrap-Callback. Der Bootstrap prüft die behaupteten Identitätsfelder nur syntaktisch; Coordinator und Register übernehmen sie. `broker/tests/security_vectors.rs:99-105,418-440,603-632` benutzt bewusst eine erfundene Draht-SID unter dem echten aktuellen Windows-Benutzer und erreicht Welcome sowie Control/Telemetry. `rg -n 'GetNamedPipeClientProcessId' broker/src` ergab keinen Treffer; ein Prozesspfad allein wäre ohnehin kein Identitätsersatz. | „Das schützt vor fremden Windows-Usern, Verwechslung und zufälligen lokalen Clients.“ (Gate NAK-123, docs/beweise/NAK-123.md:15) | Die authentisierte kanonische SID muss das behauptete SID-Feld ersetzen oder eine Abweichung ablehnen; vor dem Coordinator-Callback braucht jeder Client eine versionierte, pro Instanz ausgegebene Zulassung, und v2 darf keinen gemeinsamen Bearer an jedes Hello ausgeben. | offen |
| T3-12-03 | high | K2 | DEFEKT | broker/src/lebenslauf.rs:38; broker/src/lebenslauf.rs:152; broker/src/lebenslauf.rs:159; broker/src/server.rs:502; broker/src/server.rs:505; broker/src/main.rs:50 | bekannt NAK-123: Neu ist die Legacy-Kopplung vor dem gesicherten Endpunkt: Der Prozess startet zuerst die maschinenweit feste v2-Pipe `evenacadia.eq-copilot.v1` und bricht bei einem fremd belegten Namen ab; die SID-gebundene v3-Pipe wird dann trotz freien v3-Namens nie angelegt. Ein lokaler Prozess eines anderen oder desselben Benutzers kann so den produktiven v3-Brokerstart dauerhaft unterdrücken. | Ablaufherleitung: `BrokerLaufzeit::starten` ruft den v2-Start mit `?` vor Ableitung und Start des v3-Tokens auf. Der v2-Start setzt `FILE_FLAG_FIRST_PIPE_INSTANCE` und reicht die Kollision als Fehler bis `main`, das beendet. Die v2-DACL verhindert fremdes Verbinden mit einer echten Instanz, verhindert aber nicht, dass ein anderer Server den globalen Namen zuerst besitzt. Die NAK-123-Tests decken den v3-Namen isoliert, nicht diese vorgelagerte v2-Kollision. | „Die erste RAII-besessene Instanz entsteht mit aktueller User-DACL, `PIPE_REJECT_REMOTE_CLIENTS` und `FILE_FLAG_FIRST_PIPE_INSTANCE`; eine zweite entsteht ohne First-Flag.“ (Matrix NAK-123 A-01, docs/beweise/NAK-123.md:37) | Den SID-gebundenen v3-Server unabhängig und zuerst starten; eine Kollision des festen Legacy-Listeners muss für v3 nichtfatal sein oder der Legacy-Name muss benutzergebunden beziehungsweise entfernt werden. | offen |
| T3-12-04 | medium | K-neu: Diagnose-Vertrauensgrenze | LÜCKE | eq-copilot/plugin/core/diagnose/Briefkasten.h:3; eq-copilot/plugin/core/diagnose/Briefkasten.h:76; eq-copilot/plugin/core/diagnose/Briefkasten.cpp:620; broker/src/briefkasten.rs:663; broker/src/briefkasten.rs:710; broker/src/briefkasten.rs:787; broker/src/briefkasten.rs:823; eq-copilot/plugin/src/prozessor/Analyse.cpp:1344; eq-copilot/plugin/sonde/SondeProcessor.cpp:921 | bekannt NAK-286: Neu ist die fehlende Vertrauensregel am Diagnose-Briefkasten. Eine Anfrage im Benutzerprofil enthält nur Format und eine frei wählbare 32-Hex-ID; jeder Prozess desselben Benutzers kann sie anlegen und die Antwort lesen. Das Plugin antwortet mit Snapshot/Frame, der Broker mit Aggregat und dem vollständigen v2-Sitzungstoken. | Beide Implementierungen akzeptieren jede syntaktisch gültige Anfrage am festen `%LOCALAPPDATA%`-Pfad ohne Capability oder Benutzerhandlung. Die Produktpfade bauen die Mess-Snapshots beziehungsweise Frames; der Broker leitet `instanz_id` und `laufzeit_id` aus dem vollständigen Sitzungstoken ab. Matrix NAK-286 F-3/M-42 und die Briefkastentests belegen, dass diese Inhalte absichtlich geschrieben werden, enthalten aber keine Requester-Authentisierung. Der Token erweitert wegen T3-12-02 die heutige v2-Berechtigung nicht zusätzlich, ist aber trotzdem als Geheimnis im Diagnoseformat ungeeignet. | keine Zusage | Nie den Sitzungstoken serialisieren, sondern unabhängige nicht geheime Diagnose-IDs verwenden; Messinhalte nur nach explizitem Diagnose-Opt-in oder mit einer pro Lauf ausgegebenen Capability beantworten. | offen |
| T3-12-05 | medium | K1 | LÜCKE | broker/src/lebenslauf.rs:152; broker/src/server.rs:45; broker/src/server.rs:549; broker/src/server.rs:673; broker/src/server.rs:782; broker/src/framing.rs:34; broker/src/framing.rs:65; broker/src/lebenslauf.rs:316 | bekannt NAK-241: Neu ist die Produktionsfolge hinter dem dort bekannten Test-Hänger: Der produktive v2-Listener besitzt für Bootstrap und laufende Clients weder absolute Lese-/Inaktivitätsfrist noch Frame- oder Byte-Ratengrenze. Bis zu 64 halbe oder nach gültigem Hello schweigende Verbindungen binden alle Worker; eine Folge gültiger Kleinstframes kann außerdem unbegrenzt Parser-, Register- und ACK-Arbeit auslösen. | Der v2-Acceptor hört bei `MAX_VERBINDUNGEN = 64` auf anzunehmen. Jeder Worker ruft das blockierende `frame_lesen` zunächst für Hello und danach in der Sitzungsschleife auf; `framing.rs` deckelt einen einzelnen Frame auf 262144 Bytes, besitzt aber keine Zeit- oder Ratenlogik. Die 60-s-Leerlaufentscheidung zählt registrierte Sensoren und beendet einen Broker mit etabliertem schweigendem Client nicht. Gegenprobe v3: `BOOTSTRAP_FRIST = 5 s`, `RATE_PRO_SEKUNDE = 4000`, Ingress-Cap 256 Rahmen und 4 MiB je Verbindung. | keine Zusage | Für v2 absolute Bootstrap- und Heartbeat-Inaktivitätsfristen sowie pro Verbindung und global begrenzte Frame-/Byte-Token-Buckets einführen und bei Überschreitung schließen; Cap-Erschöpfung als Produktprüfung festhalten. | offen |

## Herleitung und Schwere

- **T3-12-01 ist critical**, weil ein nicht erhöhter Prozess desselben lokalen Benutzerkontos die dauerhaft registrierte höchste Aufgabe gegen benutzerbeschreibbare Eingaben auslösen kann; der mögliche Übergang endet in Codeausführung mit dem erhöhten Token und Schreibzugriff auf Program Files.
- **T3-12-02 ist critical**, weil ein nicht erhöhter Prozess desselben Benutzers ohne FL-Injektion als beliebige Sonde auftreten und Mess- oder Steuerdaten unter deren Identität einspeisen kann. Die Grenze „bereits in FL injiziert“ ist dafür nicht erforderlich.
- **T3-12-03 ist high**, weil die Schwäche den Normalstart des abgesicherten v3-Endpunkts verhindert, aber weder Audio- noch Statebytes unmittelbar verändert.
- **T3-12-04 und T3-12-05 sind medium**, weil sie erreichbare Diagnose- beziehungsweise Legacy-Sonderpfade betreffen, für die keine bestehende Zusage Authentisierung, Vertraulichkeit oder Fristen festlegt.

## Widerlegte Verdachte

| Nr. | Verdacht | Widerlegung |
| --- | --- | --- |
| V-01 | Die Produktpipes hätten eine offene `Everyone`-ACL oder ließen Remote-Clients zu. | `broker/src/server.rs:59-79,230-247,505-506,566-569` baut `D:P(A;;GA;;;<SID>)` aus der aktuellen binären User-SID und setzt `PIPE_REJECT_REMOTE_CLIENTS`; v3 benutzt denselben Deskriptor in `server_v3/win_handles.rs:52-74`. Fremdbenutzer-Datenzugriff über eine echte Instanz überlebte den Skeptikerpass nicht. |
| V-02 | Ein Produktclient erlaube dem Server Delegation oder volle Impersonation. | Beide Öffnungsprimitive setzen `SECURITY_SQOS_PRESENT \| SECURITY_IDENTIFICATION`: `eq-copilot/plugin/src/PipeClient.cpp:379-380` und `eq-copilot/plugin/core/ipc/IpcVerbindung.cpp:108-109`. |
| V-03 | v3 verliere den Pipenamen routinemäßig zwischen Accept und Ersatzlistener. | Zwei Listener werden bewaffnet; der Ersatz entsteht vor Workerübergabe, und nur die erste Instanz trägt `FILE_FLAG_FIRST_PIPE_INSTANCE` (`server_v3/listener.rs:288-429,600-694`; `server_v3/win_handles.rs:52-74`). Die überlebende Kollision liegt ausschließlich in der vorgelagerten festen v2-Pipe, T3-12-03. |
| V-04 | Der Broker-Spawn aus dem Plugin benutze `PATH`, Shellsuche oder einen relativen CWD-Pfad. | `BrokerLifecycle.cpp:423-445,480-560,600-627` bindet den absoluten installierten Pfad, prüft den tatsächlich verbundenen Serverprozess beziehungsweise vor Spawn Hash und optional Signer, übergibt den absoluten Pfad als `lpApplicationName`, setzt das Arbeitsverzeichnis auf dessen Elternpfad und startet ohne Shell. Das widerlegt nicht die getrennte erhöhte Aufgabenaktion T3-12-01. |
| V-05 | Ein halbes v3-Hello könne einen Worker unbegrenzt festhalten. | `server_v3/mod.rs:154` setzt fünf Sekunden, `server_v3/verbindung.rs:195-241` registriert und begrenzt den Bootstrap-Puffer, der Wachhund bricht die I/O ab. Die fristlose Kante blieb nur in v2, T3-12-05. |
| V-06 | v3 vertraue Längenfeld, Familie oder CRC erst nach Inhaltsverarbeitung. | `transport/v3.rs:228-289` prüft u64-sicher Gesamtlänge, Kopf, Payloadlänge, Familie/Encoding und CRC vor der Übergabe; Überlänge endet am festen 262144-Byte-Cap. Ein kaputter Strom wird geschlossen, nicht resynchronisiert. |
| V-07 | Millionen kleiner v3-Frames ließen Speicher oder Arbeit ungebremst wachsen. | `server_v3/mod.rs:145-158`, `server_v3/verbindung.rs:637-660,749-756` und `warteschlange.rs:24-52` liefern Worker-Cap, 4000 Frames/s je Verbindung, 256 Ingress-Slots und 4 MiB Bytebudget; P0/P1-Überlauf trennt, P2 wird begrenzt verworfen. |
| V-08 | Anfrageinhalt des Briefkastens werde als Pfad benutzt oder Reparse-Ziele würden still überschrieben beziehungsweise gelöscht. | C++ und Rust leiten Namen nur aus validierter 32-Hex-ID ab, begrenzen die Anfrage auf 1024 Bytes, prüfen Reparse-Punkte, schreiben exklusiv über temporäre Dateien ohne Ersetzen und löschen keine fremden Dateien (`Briefkasten.cpp:119-238,620-684`; `broker/src/briefkasten.rs:167-318,663-688`). Übrig bleibt die Authentisierungs-/Inhaltslücke T3-12-04. |
| V-09 | Der Installer kopiere ohne Ruhe-, Reparse-, Zielhash- oder Rückwegprüfung. | `Install-Nakama.ps1:721-840,893-944,1047-1160` sperrt laufendes FL, prüft Pfadkomponenten auf Reparse, sichert in den geschützten Zielbaum, verifiziert nach Kopie und Rückweg erneut gegen das Manifest. Das widerlegt Betriebsfehler innerhalb einer vertrauenswürdigen Ausführung, nicht den veränderbaren Vertrauensanker T3-12-01. |
| V-10 | Eingefrorene Class-IDs oder Codes würden aus Namen neu abgeleitet. | `eq-copilot/identity/plugin-identities-v1.json` und `host-capabilities-fl-v1.json` enthalten die IDs und Codes literal; Installer und Plugin lesen beziehungsweise spiegeln diese Werte. Im geprüften Pfad wurde keine Namensableitung gefunden; NAK-30 bleibt unberührt. |

## Bekannter Vorlauf, nicht erneut gezählt

- NAK-118: Traversal im Probe-Pipe-Präfix; keine neue Reproduktionsform über die Registerzeile hinaus.
- NAK-119: leerer Authenticode-Thumbprint; T3-12-01 betrifft zusätzlich den veränderbaren erhöhten Aktions- und Manifestpfad.
- NAK-123: v3-Namensbesitz und beidseitige Windows-Identitätsprüfung; T3-12-03 betrifft neu die davor liegende feste v2-Pipe.
- NAK-127 beziehungsweise T3-05-03: Frische des erzeugten Broker-Pins; nicht erneut bewertet.
- NAK-135: nur zwei bewaffnete v3-Listener; keine neue Folge jenseits der bekannten Zeile.
- NAK-152: SID-Kanonisierung im Store; T3-12-02 betrifft neu die fehlende Bindung von Windows- an Drahtidentität.
- NAK-241: blockierende v2-Testthreads beim Stoppen; T3-12-05 ergänzt die erreichbare Produktionserschöpfung.
- NAK-285: drei erhöhte Aufgaben; T3-12-01 ergänzt die konkrete Privileggrenze ihrer Checkout-Aktion.
- NAK-286: Diagnose-Briefkasten; T3-12-04 ergänzt die fehlende Requester-Authentisierung und Token-/Messdatenoffenlegung.

## Produktfragen

Keine.

## Nicht geprüft

- Kein dynamischer Zwei-Benutzer-Versuch, keine Pipe-ACL-Abfrage am laufenden Broker und kein Start-/Kollisionsversuch; die Aussagen sind Quell- und Aufrufkettenherleitungen.
- Keine Abfrage oder Ausführung der drei geplanten Aufgaben, keine Änderung von Manifest, Artefakten, Zertifikatsstore, Program Files oder `%LOCALAPPDATA%`.
- Kein Build, kein Test, kein Kanon, keine Installation, kein FL-Studio-Lauf und kein Exploitcode.
- Windows-Kernel- und Task-Scheduler-Implementierung wurden nicht extern untersucht; geprüft wurde die vom Produkt gewählte API- und Besitzlogik.
- `broker/src/store/**`, DSP, UI, State/Migration und geparkte Designflächen lagen außerhalb dieser Phase; nur ihre unmittelbar aufgerufenen Grenzsymbole wurden verfolgt.

## Scope-Beweis

### Gelesene Pfade

Die beauftragte Quellpopulation wurde vollständig inventarisiert und an ihren Vertrauenskanten gelesen: **61 von 61 Dateien, 27.951 physische Zeilen; davon 25.732 nichtleer** (`Get-Content | Measure-Object -Line`).

| Teilpopulation | Dateien |
| --- | ---: |
| `broker/src/transport/**` | 21 |
| `broker/src/bin/**` | 4 |
| `broker/src/{briefkasten,lebenslauf,lib,main,persistenz,register,server}.rs` | 7 |
| `eq-copilot/plugin/core/diagnose/**`, die benannten IPC-Dateien samt ausgelagertem `controlclient/**`, `src/PipeClient.*` | 23 |
| `eq-copilot/install/**` | 3 |
| `eq-copilot/identity/**` | 2 |
| `tools/dirigent/install-aufgaben-registrieren.ps1` | 1 |

Der Populationslauf zählte zusätzlich **681 Rust-`fn`-Treffer und 32 PowerShell-Funktionen**; C++-Deklarationen und -Definitionen sowie alle Treffer an Pipe-, Handle-, Spawn-, Token-, Pfad-, Reparse-, Hash-, Signatur- und Task-Vertrauenskanten wurden einzeln den Aufrufketten zugeordnet. Produktion besitzt drei syntaktische `CreateNamedPipeW`-Stellen (v2-Erst-/Folgeinstanz und v3-Helfer), zwei produktive `CreateFileW`-Primitive, eine Broker-Impersonationskette, einen `CreateProcessW`-Spawn, eine Aufgabenaktion in drei Modi, zwei Briefkastenimplementierungen und zwei Identitätsmanifeste.

Als unmittelbare Grenz- und Skeptikerquellen wurden außerdem **12 Dateien** gelesen: `broker/src/coordinator/{zustand,link,mod,liveness}.rs`, `broker/src/{protokoll,framing}.rs`, `broker/tests/security_vectors.rs`, `tools/eq-copilot/pruefe_installer_manifest.py`, `tools/fl/laufzeit.ps1`, `eq-copilot/plugin/src/prozessor/Analyse.cpp`, `eq-copilot/plugin/sonde/SondeProcessor.cpp` und `eq-copilot/plugin/tests/BriefkastenTestMain.cpp`.

Kontext und Zusagen wurden in `CLAUDE.md`, `docs/plugin-wissen.md`, `docs/gesundheit/{KONZEPT,abdeckungskarte,lebenslauf-audit}.md`, `docs/offene-punkte.md`, `docs/FL-Nakama-Sonden-Design-Entwurf.md`, `docs/plan/plan.json`, `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md`, den gezielten Matrizen `docs/beweise/{NAK-123,NAK-286,SONDE-011}.md` und per ID-/Zusage-Suche in `docs/beweise/*.md` geprüft. Dokumente waren nur Anforderungs- und Skeptikerquelle, nie Befundfläche.

### Populations- und Skeptikerkommandos

- `rg --files` über jede oben genannte Teilpopulation, danach Zeilenzählung jedes Eintrags mit `Get-Content`; Summe 61 Dateien, 27.951 physische beziehungsweise 25.732 nichtleere Zeilen.
- `rg -n` über alle Funktionen und die Vertrauenskanten `CreateNamedPipeW`, `CreateFileW`, `ConnectNamedPipe`, `ImpersonateNamedPipeClient`, `RevertToSelf`, SID/Token/Hello/Welcome, Längen/CRC/Rate/Budget, `CreateProcessW`, Taskaktion, Hash/Signatur, Reparse und Briefkasten.
- Für jeden Überlebenden Suche in Tests, Matrizen, Absichtskommentaren und `docs/offene-punkte.md`; bekannte Nachbarschaften stehen in jeder Befundzeile und im Vorlauf oben.

### Nicht gelesene Pfade

Nicht gelesen wurden die übrigen DSP-, UI-, Editor-, State-, Store- und Schemaimplementierungen, externe Abhängigkeiten, Buildausgaben, Archive und geparkte Studien. Aus angrenzenden Verzeichnissen wurden nur die oben explizit genannten Grenzdateien geöffnet.

### Fehlende Pfade

**Kein beauftragter Populationspfad fehlte.** Ein zunächst als möglicher Ticketbeleg gesuchtes `docs/beweise/NAK-285.md` existiert nicht; NAK-285 ist als Register-/Planmechanik in `docs/offene-punkte.md`, `docs/plan/plan.json` und `docs/beweise/NAK-286.md` dokumentiert. Das beeinträchtigte die Quellprüfung der Aufgabenaktion nicht.
