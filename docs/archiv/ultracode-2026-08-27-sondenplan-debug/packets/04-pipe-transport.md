# Paket 04 — Pipe- und Transport-Lebenszyklus

Stand: 2026-08-27, initialer adversarialer read-only Review. In dieser ersten
Phase hat dieser Agent keine Produktdatei und keinen Test verändert; einzig
dieses Paketartefakt wurde angelegt. Der später ausdrücklich beauftragte
Broker-Folgefix ist im Revalidierungsabschnitt am Ende getrennt dokumentiert.
Die ursprünglichen Zeilenangaben beziehen sich auf den während der ersten
Prüfung sichtbaren Parallelstand.

## Kurzurteil

Der vorhandene v2-Transport hält seine **Framegrenzen** ein und bleibt aus dem
Audiocallback heraus. Die Happy-Path-Architektur ist nachvollziehbar: eigener
Pipe-Thread im Plugin, blockierendes Byte-Pipe-Framing, je Verbindung ein
Broker-Thread, kurze Register-Locks und keine Pipe-I/O unter dem Register-Lock.

Nicht tragfähig sind derzeit die Vertrauens- und Abbruchgrenzen. Bestätigt sind
**elf Befundcluster**: sechs hoch, vier mittel bis mittel-hoch und einer
niedrig-mittel. Besonders dringlich sind Client-SQOS/Peer-Authentisierung, der
unbegrenzte Client-Stop, ungültiges UTF-8 in JUCE-Strings sowie Broker-Stop und
Ressourcenkappen. Diese Befunde liegen überwiegend im bereits bekannten
Legacy-v2-Pfad, der heute trotzdem real aus jedem `EqCopilotProcessor`-Konstruktor
gestartet wird (`PluginProcessor.cpp:33-70`). `SONDE-010` ist laut
`docs/PLAN-STAND.md:97` noch offen; ein zukünftiger v3-Transport ist daher kein
Beleg dafür, dass der aktuell laufende v2-Pfad sicher oder abbrechbar ist.

| ID | Priorität | Kurzurteil |
|---|---:|---|
| PIP-01 | hoch | Der Client übergibt einem beliebigen Pipe-Besitzer ohne Not einen impersonierbaren Host-Threadtoken. |
| PIP-02 | hoch | Der Client authentisiert den Broker nicht und sendet nach einem selbst behaupteten `welcome` dauernd Messdaten. |
| PIP-03 | hoch | `stop()` kann trotz gegenteiliger Header-Zusage dauerhaft hängen; Cancellation und Handle-Lebenszeit sind racy, danach folgt neue blockierende I/O. |
| PIP-04 | mittel-hoch | `reconnect()` garantiert kein frisches Hello: blockierende I/O wird nicht geweckt und eine neue Generation kann am Welcome verloren gehen. |
| PIP-05 | hoch | Ungültige Peer-Bytes werden in Release ohne UTF-8-Riegel zu einem JUCE-String. |
| PIP-06 | mittel | Client und Server erzwingen die Nachrichten-Zustandsmaschine nur teilweise; falsche ACKs, wiederholte Hellos und Sessionfehler bleiben inkonsistent. |
| PIP-07 | hoch | Der Broker-Acceptor hat einen Lost-Wakeup beim Stop; Verbindungs-Threads werden weder verfolgt noch beendet. |
| PIP-08 | hoch | Unbegrenzte Pipe-Instanzen, Threads, Lesedauer und Registereinträge erlauben deterministische lokale Ressourcenerschöpfung. |
| PIP-09 | mittel-hoch | `FlushFileBuffers` kann einen Verbindungsthread absichtlich unbegrenzt an einen nicht lesenden Client binden. |
| PIP-10 | mittel | Liveness/Stale basiert auf verstellbarer Wallclock statt auf einer monotonen Uhr. |
| PIP-11 | niedrig-mittel | Lebenszyklusfehler sind im Status und im heutigen Probe-Bein teilweise unsichtbar beziehungsweise falsch grün. |

## Scope und Norm

Gelesen und über Aufrufbeziehungen verfolgt wurden:

- `eq-copilot/plugin/src/PipeClient.{h,cpp}` sowie Konstanten in
  `EqCopilotIds.h`;
- Konstruktion, State-Restore, neue Sensor-ID und Bindungsänderungen in
  `PluginProcessor.{h,cpp}`;
- `eq-copilot/plugin/probe/PipeProbeMain.cpp` und die Pipe-bezogenen CMake-/
  Beweisrunner-Einträge;
- `broker/src/{server,framing,protokoll,lib}.rs` einschließlich Server- und
  Registertests;
- der aktuelle v2-Vertrag `eq-copilot/schemas/eq-ipc.schema.json`;
- Entwurf §48.3–48.4 (`docs/FL-Nakama-Sonden-Design-Entwurf.md:3148-3201`),
  §53.5 (`:3594-3606`), die IPC-Gates (`:3236`, `:3357-3359`, `:4122-4128`)
  und die Istbeschreibung `docs/plugin-wissen.md:300-310,435-460`.

Für die Win32-Semantik wurden die Primärquellen gegengeprüft:

- [CreateFileW / SQOS](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew),
- [CancelSynchronousIo](https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-cancelsynchronousio),
- [FlushFileBuffers](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-flushfilebuffers),
- [Impersonating a Named Pipe Client](https://learn.microsoft.com/en-us/windows/win32/ipc/impersonating-a-named-pipe-client).

Die Befunde PIP-01, PIP-02, Teile von PIP-03, PIP-05, PIP-07 und PIP-08
überschneiden sich mit den weiterhin offenen Sammelpunkten NAK-49/NAK-78. Sie
werden hier nicht als neu ausgegeben, sondern am heutigen Source-Stand erneut
bestätigt und als zusammenhängender Connect↔Disconnect-Gegenpfad bewertet.

## Bestätigte Befunde

### PIP-01 — Client-Pipe ohne begrenztes SQOS

**Priorität:** hoch · **Confidence:** sehr hoch · **Status:** offen

`PipeClient::eineVerbindung()` öffnet die feste Pipe mit
`dwFlagsAndAttributes = 0`:

- `eq-copilot/plugin/src/PipeClient.cpp:186-188`;
- fester Produktionsname: `eq-copilot/plugin/src/EqCopilotIds.h:18`.

Es fehlen `SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION`. Der Transport
braucht nirgends, dass der Server im Sicherheitskontext des FL-/DAW-Threads
handelt. Trotzdem kann ein Prozess, der den vorhersagbaren Namen zuerst besitzt,
den Client über `ImpersonateNamedPipeClient` mit dem standardmäßig stärkeren
Impersonationsniveau übernehmen. `FILE_FLAG_FIRST_PIPE_INSTANCE` auf der
ehrlichen Serverseite (`broker/src/server.rs:243-247`) verhindert nur eine
zweite ehrliche Serverinstanz; es vertreibt keinen bereits lauschenden
Namensbesetzer.

**Reproduktion/Testidee:** In einem Wegwerfprozess zuerst eine Pipe mit dem
Produktionsformat, aber eindeutigem Testsuffix anlegen. Den echten `PipeClient`
verbinden lassen, auf Serverseite genau ein Byte/Hello lesen, danach
`ImpersonateNamedPipeClient` und `OpenThreadToken` ausführen. Der Negativtest
darf ausschließlich die Impersonationstufe abfragen und muss nach dem Fix mit
`SecurityIdentification` keine nutzbare Impersonation erlauben.

**Korrekturrichtung:** SQOS schon im aktuellen Client minimal setzen. Das ist
unabhängig von der späteren v3-SID-Pipe und reduziert die Rechte auch dann,
wenn PIP-02 noch nicht vollständig gelöst ist.

### PIP-02 — Keine Authentisierung des Server-Peers

**Priorität:** hoch · **Confidence:** sehr hoch · **Status:** offen

Nach `CreateFileW` besteht die gesamte Vertrauensentscheidung aus
`type == welcome` und `protocol_version == 2`
(`PipeClient.cpp:211-239`). Weder Server-PID/-Pfad/-Signatur noch ein
kryptographischer Nachweis werden geprüft. Der vom Peer gelieferte
`session_token` kann ihn nicht authentisieren. Danach sendet der Client im
Sekundentakt Sensor-ID, Nonce, Rolle, Label, Pair-ID, Host-PID, Audioformat,
Pegel, Projektzeit und den kompakten Messstand einschließlich LTAS
(`PipeClient.cpp:39-95,242-267`). Umgekehrt übernimmt er `konflikt` aus dem
Peer-ACK in sichtbaren Zustand (`:257-263`).

Ein beliebiger lokaler Namensbesetzer kann daher private, audioabgeleitete
Telemetrie lesen, einen gesund aussehenden Verbindungsstatus vortäuschen und
über ein gefälschtes Konfliktflag den Identitätswechsel des Users anstoßen.
Die legitime Broker-DACL (`server.rs:164-184,236-252`) hilft dem Client nicht,
weil eine fremde Serverinstanz ihre eigene DACL bestimmt.

**Reproduktion/Testidee:** Fake-Server gewinnt einen eindeutigen Testnamen,
antwortet mit syntaktischem `welcome`, protokolliert genau einen Heartbeat und
antwortet mit `heartbeat_ack/konflikt=true`. Erwartung nach dem Fix: kein
Measurement vor erfolgreicher Peer-Prüfung, Status niemals `verbunden`, das
Konfliktflag bleibt unverändert.

**Korrekturrichtung:** Die im Entwurf §48.3/48.4 vorgesehene SID-gebundene Pipe
und Broker-Authentisierung implementieren. Als zusätzliche lokale Evidenz sind
Server-PID, installierter kanonischer Pfad und Signatur sinnvoll; die eigentliche
Protokollidentität sollte an eine nicht vom Peer selbst gewählte Capability /
Challenge gebunden sein. Fail closed: ohne Prüfung keine Telemetrie.

### PIP-03 — Client-Stop ist nicht begrenzt und kann neue I/O nach Cancellation starten

**Priorität:** hoch · **Confidence:** sehr hoch · **Status:** offen

Der Header verspricht ausdrücklich, dass `stop()` blockierende Pipe-I/O
abbricht und der Destruktor nie am Broker hängt (`PipeClient.h:3-7`). Der Code
erfüllt das nicht:

1. `DuplicateHandle` wird nicht geprüft und erzeugt per
   `DUPLICATE_SAME_ACCESS` einen unnötig weit berechtigten Handle
   (`PipeClient.cpp:148-153`). Scheitert der Aufruf, fehlt die einzige
   Cancellation-Schiene still.
2. Der Worker publiziert den Threadhandle selbst und schließt ihn selbst
   (`:171-173`), während `stop()` ihn per Check-then-use liest (`:129-130`).
   Zwischen Load, `store(nullptr)` und `CloseHandle` kann der Handle ungültig
   oder bereits wiederverwendet sein.
3. `CancelSynchronousIo` wird genau einmal gerufen; Rückgabe und
   `ERROR_NOT_FOUND` werden ignoriert (`:129-132`). Die API markiert nur die zu
   diesem Zeitpunkt bereits ausstehenden synchronen Operationen, wartet aber
   selbst nicht auf deren Ende.
4. Nach jedem Break und auch nach `laeuft=false` erzeugt der Worker
   bedingungslos ein `bye` und startet über `sende()` einen **neuen**
   synchronen `WriteFile` (`:270-274,291-305`). Der einmalige Cancel ist dann
   bereits verbraucht.
5. `aktivesHandle` wird zwar geführt (`PipeClient.h:139`,
   `PipeClient.cpp:198,278`), aber an keiner Abbruchstelle gelesen.

Folge: Plugin-Entfernen, Projekt-Schließen, read-only-State-Restore und
Destruktion können auf dem Host-/Message-Thread dauerhaft in `thread.join()`
hängen (`PluginProcessor.cpp:73-82,722-735`). Ein Pipe-Squatter kann den Zustand
durch ACK-Verweigerung oder einen kleinen, nicht geleerten Inputpuffer gezielt
herstellen.

**Reproduktion/Testidee:** Den echten Client gegen vier deterministische
Fake-Server fahren: (a) Welcome nie vollständig senden, (b) nach Welcome kein
ACK senden, (c) nach Welcome nichts mehr lesen, sodass Heartbeat/Bye schreibt,
(d) Verbindung genau im Übergang Wait→Read stoppen. In jeder Variante `stop()`
10.000-mal mit einem harten, großzügigen Deadline-Orakel ausführen; keine
Variante darf den Hostthread über die Deadline halten. Zusätzlich Application
Verifier „invalid handle“ aktivieren, um den Close/Cancel-Race sichtbar zu
machen.

**Korrekturrichtung:** Bevorzugt Overlapped-I/O plus explizites Stop-Event und
ein einziger Besitzer des Pipehandles. Alternativ den Threadhandle vor Start
unter Lifecycle-Lock erzeugen, erst nach Join schließen, `CancelIoEx` auf dem
aktiven Pipehandle als stabile Abbruchschiene einsetzen und nach Stop/I/O-Fehler
kein Bye mehr senden. „Best effort“ muss zeitlich begrenzt sein.

### PIP-04 — Reconnect verliert Generationen und weckt blockierende I/O nicht

**Priorität:** mittel-hoch · **Confidence:** hoch · **Status:** offen

`reconnect()` setzt nur `neuVerbinden=true` und signalisiert eine Condition
Variable (`PipeClient.cpp:135-140`). Das weckt weder den Welcome-/ACK-`ReadFile`
noch einen blockierenden `WriteFile`. Im Backoff hilft das Signal ebenfalls
nicht, weil dessen Prädikat nur `!laeuft` prüft (`:163-167`).

Zusätzlich baut der Worker das Hello bereits bei `:204`, wartet dann beliebig
lange auf Welcome und löscht bei `:240` **bedingungslos** jede inzwischen
gesetzte Reconnect-Anforderung. Wird in diesem Fenster über
`neueSensorId()`/`setzeBindung()` State geändert
(`PluginProcessor.cpp:680-694,830-862`), kann die Methode Erfolg und
Host-Dirty melden, während die bestehende Verbindung dauerhaft das alte Hello
repräsentiert.

Ein einzelnes Bool kann außerdem zwei schnelle Änderungen nicht als
Generationen unterscheiden.

**Reproduktion/Testidee:** Fake-Server liest Hello A und hält Welcome an einer
Barriere. Dann neue Sensor-ID setzen und `reconnect()` rufen, erst danach
Welcome freigeben. Erwartung: Die nächste akzeptierte Verbindung trägt sicher
ID B; der aktuelle Code löscht die Anforderung an `:240`. Zweiter Fall:
Welcome senden, ACK zurückhalten, Reconnect anfordern; ein frisches Hello muss
innerhalb fester Zeit eintreffen.

**Korrekturrichtung:** Monotonen Reconnect-/Konfigurationszähler statt Bool
verwenden. Generation vor dem Hello erfassen, nach jedem blockierenden Schritt
vergleichen und auf Änderung die aktive I/O abbrechen. Der Backoff wartet auf
Stop **oder** neue Generation und entfällt bei bewusstem Reconnect.

### PIP-05 — Fehlender UTF-8-Riegel im C++-Empfang

**Priorität:** hoch · **Confidence:** sehr hoch · **Status:** offen

`PipeClient::empfange()` begrenzt die Länge korrekt, übernimmt die Peerbytes
aber direkt über `juce::String::fromUTF8(puffer, len)`
(`PipeClient.cpp:324-337`). JUCEs Gültigkeitsprüfung ist im Releasepfad nur ein
`jassert`; ungültige UTF-8-Folgen verletzen danach die String-Invariante. Das
Repository besitzt inzwischen für andere Byteverträge genau den benötigten
Vorab-Riegel (`plugin/vertrag/NakamaVertrag.cpp`), der Pipepfad verwendet ihn
nicht. Betroffen sind Welcome, Reject und jedes ACK; die Strings werden teils
langfristig im Snapshot gehalten.

**Reproduktion/Testidee:** Für jede ungültige UTF-8-Klasse (bare continuation,
truncated multibyte, overlong, surrogate, >U+10FFFF, rohes NUL plus Suffix) ein
formal korrektes Framepräfix vom Fake-Server senden. Release+ASan muss die
Verbindung ohne JUCE-Stringkonstruktion schließen. Ein Mutationsbein entfernt
jeweils den Riegel und muss fallen.

**Korrekturrichtung:** Vor `fromUTF8` bytegenau dieselbe bounded UTF-8-/NUL-
Validierung wie an der bereits gehärteten Vertragsgrenze verwenden; erst danach
JSON parsen.

### PIP-06 — Nachrichten-Zustandsmaschine und Session/ACK werden nur teilweise geprüft

**Priorität:** mittel · **Confidence:** hoch · **Status:** offen

Mehrere unabhängige Inkonsistenzen bilden gemeinsam eine zu permissive
Zustandsmaschine:

- Der Client verlangt bei Welcome nur Typ und per C-Cast gelesene Version
  (`PipeClient.cpp:211-230`). Ein leerer/fehlender `session_token` wird
  akzeptiert. Ein Peerwert wie `1e300` erreicht vor der Ablehnung eine
  undefinierte `double→int`-Konvertierung im JUCE-`var`-Cast.
- Beim ACK wird nur bei exakt erkanntem `heartbeat_ack` Zustand aktualisiert;
  jeder andere syntaktische Frame gilt trotzdem als erfolgreiche Antwort und
  die Schleife läuft weiter (`:252-264`). Bei erkanntem ACK werden weder das
  zitierte `seq` noch Feldtypen/Vollständigkeit geprüft.
- `heartbeatsGesendet` wird vor dem ACK erhöht (`:246-250`). Deshalb beweist
  die Probe-Bedingung `>=2` nicht, dass zwei korrekte ACKs ankamen
  (`probe/PipeProbeMain.cpp:79-85`).
- Der Broker akzeptiert ein zweites `hello` im Heartbeat-Zustand ohne erneuten
  Aufruf von `hello_pruefen`, ohne Reject/Welcome und übernimmt dabei auch eine
  beliebige neue Protokollversion (`broker/src/server.rs:449-460`). Der echte
  Client nutzt diesen Pfad nicht; er reconnectet, sodass der Zweit-Hello-Zweig
  nur zusätzliche Angriffsfläche ist.
- Ein Heartbeat mit fremdem Sessiontoken wird protokolliert, aber die
  Verbindung bleibt ohne ACK offen (`server.rs:410-418`). Der synchrone Client
  wartet dann unbegrenzt. `Bye.session_token` wird deserialisiert, beim Break
  aber gar nicht geprüft (`protokoll.rs:141-145`, `server.rs:448`).
- Sequenzen werden nicht auf Duplikat/Rücklauf geprüft; ein älterer Heartbeat
  darf den jüngeren Messstand überschreiben (`server.rs:430-445`,
  `lib.rs:161-184`).

**Reproduktion/Testidee:** Tabellengetriebene State-Machine-Tests mit
`welcome` ohne Token, Version `1e300`, ACK mit falschem Typ, fehlendem/falschem
Seq, doppeltem/zurücklaufendem Seq, Zweit-Hello Version 99 und falschem
Sessiontoken. Jedes Paket braucht ein festes Urteil: akzeptieren, Reject mit
Grund oder Verbindung schließen; nie still weiterlaufen oder den Gegenpeer
unbegrenzt warten lassen.

**Korrekturrichtung:** Eine explizite Nachrichtenzustandsmaschine verwenden
(`AwaitHello → Established(v, token, expected_seq) → Closing`), Antworten
streng typisieren und Sequenz/Session koppeln. Zweit-Hello entweder entfernen
oder wie ein vollständiges Rehandshake behandeln.

### PIP-07 — Broker-Stop hat Lost-Wakeup; Connection-Threads überleben den Griff

**Priorität:** hoch · **Confidence:** sehr hoch · **Status:** offen

`ServerGriff::stoppen()` setzt ein Atomic, versucht genau einmal einen
Wegwerfclient zu öffnen, verwirft dessen Fehler und joint danach unbegrenzt
(`broker/src/server.rs:193-203`). Der Acceptor verwendet synchrones
`ConnectNamedPipe` ohne Event (`:285-331`). Liegt Stop zwischen Loop-Prüfung und
dem Anlegen/Connect der nächsten Instanz oder schlägt der einzige Open wegen
der momentanen Pipebelegung fehl, entsteht danach ein neuer blockierender
`ConnectNamedPipe`, den niemand mehr weckt.

Der erste Handle verlässt außerdem RAII als rohes `Option<HANDLE>`
(`:267-283`). Sieht der Thread Stop schon vor der ersten Loopiteration, wird
dieser Handle nicht wieder in einen Guard übernommen.

Noch grundsätzlicher werden `eqcop-pipe-conn`-Threads bei `:339-341` detached;
ihre JoinHandles und Pipehandles kennt `ServerGriff` nicht. Stop beendet damit
nur den Acceptor, nicht den Server: bereits akzeptierte Verbindungen können
weiter in `frame_lesen` hängen und den Pipenamen/State halten. Das ist heute in
der kurzlebigen Probe relevant und wird beim geplanten On-demand-/Idle-Lifecycle
aus §48.3 zwingend produktiv.

**Reproduktion/Testidee:** Acceptor mit Testbarrieren jeweils (a) vor der
Loopprüfung, (b) nach ihr, vor Create, (c) vor Connect und (d) im Connect
anhalten; parallel `stoppen()` rufen und Deadline prüfen. Dann mit einem
stillen verbundenen Client stoppen und auf Null Connection-Threads/Handles
prüfen. Sofortstart→Sofortstop→Neustart auf demselben Testnamen 10.000-mal
fahren.

**Korrekturrichtung:** Overlapped `ConnectNamedPipe` mit Stop-Event und
deterministischem Cancel; HandleGuard bis zur echten Übergabe behalten.
Connection-Handles/-Tasks in einer begrenzten Registry besitzen, beim Stop
abbrechen und joinen. Ein Stop-Ergebnis darf Fehler nicht verwerfen.

### PIP-08 — Unbegrenzte Threads, Pipeinstanzen und Registertombstones

**Priorität:** hoch · **Confidence:** sehr hoch · **Status:** offen

Der Broker kombiniert:

- `PIPE_UNLIMITED_INSTANCES` und je Instanz 64 KiB In/Out
  (`server.rs:243-251,291-301`),
- einen neuen OS-Thread je Verbindung, dessen Spawnfehler verworfen wird
  (`:339-341`),
- kein Hello-/Read-Timeout (`:353-369`),
- keinen globalen/je-User Connection-Cap,
- ein dauerhaft wachsendes `HashMap` auf peerbestimmter Sensor-ID
  (`broker/src/lib.rs:93-110`), dessen Einträge beim Disconnect nur markiert,
  nie begrenzt/evicted werden (`:193-207`),
- den unvalidierten Zweit-Hello-Pfad, mit dem **eine** Verbindung beliebig viele
  neue Sensor-IDs hinterlassen kann (`server.rs:449-460`),
- keine Durchsetzung der v2-Schema-Längengrenze für `label` (120) und keine
  sinnvollen Caps für die übrigen gespeicherten Strings.

Ein lokaler Prozess desselben Users kann daher mit stillen Verbindungen Threads,
Stacks, Kernelpuffer und Handles binden oder mit wiederholten Hellos dauerhafte
Registereinträge bis zum Speicherende erzeugen. Status/Snapshot klont später
die gesamte Map unter Mutex (`lib.rs:240-255,523-542`) und verstärkt CPU- und
Lockkosten.

**Reproduktion/Testidee:** Mehr Clients als der festgelegte Systemcap öffnen
und vor dem ersten Präfix schweigen; Thread-/Handlezahl muss begrenzt bleiben
und Ablehnung sichtbar sein. Separat über eine Verbindung 10.000 neue IDs und
maximale Labels senden; Mapgröße muss unter einem dokumentierten Cap bleiben,
alte Tombstones müssen deterministisch evicted/verdichtet werden. Slowloris:
jeweils nur ein Headerbyte kurz vor Deadline senden.

**Korrekturrichtung:** Feste Caps, RAII-Zähler/Semaphore vor Spawn, Deadline
bereits für das Bootstrap-Hello, Rate-/Framebudget je Verbindung und
deterministische stale-first-Tombstone-Eviction wie §48.3 vorgibt. Alle
String-/Enum-/Zahlenkappen vor Eintrag ins Register erzwingen.

### PIP-09 — `FlushFileBuffers` macht Disconnect vom Lesen des Clients abhängig

**Priorität:** mittel-hoch · **Confidence:** sehr hoch · **Status:** offen

Jeder Server-Verbindungspfad endet in `pipe_schliessen()` und ruft dort vor
`DisconnectNamedPipe` synchron `FlushFileBuffers` auf
(`broker/src/server.rs:478-491`). Laut Win32-Vertrag kehrt
`FlushFileBuffers` am **Serverende einer Named Pipe erst zurück, wenn der Client
alle gepufferten Daten gelesen hat**. Genau das kontrolliert der untrusted
Client.

Deterministischer Fall: Client sendet gültiges Hello, liest das Welcome nicht,
sendet im Duplexkanal sofort Bye und hält den Handle offen. Der Server liest
Bye, gelangt zu `FlushFileBuffers` und wartet unbegrenzt auf das Lesen des
Welcome. Gleiches gilt für Reject-/Fehlerpfade. Zusammen mit PIP-08 wird daraus
ein billiger permanenter Connection-Thread-Leak.

**Reproduktion/Testidee:** Rohclient sendet Hello+Bye, liest keine Serverbytes
und bleibt offen. Der Verbindungsthread muss nach fester Frist verschwunden
sein; heute bleibt er in `FlushFileBuffers`.

**Korrekturrichtung:** Am untrusted Disconnect keine unbegrenzte Flush-Garantie
erzwingen. Handle schließen beziehungsweise einen explizit zeitlich begrenzten,
overlapped Closing-State verwenden. Ein „geordnetes Bye“ darf kein Recht des
Peers sein, den Serverthread festzuhalten.

### PIP-10 — Stale/Liveness hängt an verstellbarer Systemzeit

**Priorität:** mittel · **Confidence:** hoch · **Status:** offen

`jetzt_ms()` verwendet `SystemTime::now() - UNIX_EPOCH`
(`broker/src/lib.rs:36-40`). Derselbe Wert setzt `last_seen_ms` und wird für
`stale` mit `saturating_sub` verglichen (`:104-159,240-247`). Ein
Wallclock-Sprung nach vorn macht aktive Sensoren sofort stale; ein Sprung
zurück hält sie wegen Saturation beliebig lange frisch. Das widerspricht der
expliziten Zielinvariante in Entwurf §48.3 (`:3173-3178`), wonach laufende
Frische/Eviction ausschließlich `Instant` nutzt.

**Reproduktion/Testidee:** Uhr als Dependency injizieren. Nach einem Heartbeat
Wallclock ±24 h springen lassen, monotone Zeit aber nur 1 s; Status bleibt
frisch. Dann monotone Zeit >5 s vorrücken, Wallclock unverändert lassen; Status
wird stale. Anzeigezeit darf separat wallclockbasiert bleiben.

**Korrekturrichtung:** Intern `Instant`/monotone Deltas speichern; nur für
Serialisierung/Anzeige zusätzlich UTC erfassen.

### PIP-11 — Fehlerstatus und vorhandenes Probe-Bein beweisen den Gegenpfad nicht

**Priorität:** niedrig-mittel · **Confidence:** hoch · **Status:** offen

I/O-Fehler nach einem Welcome werden nirgends mit Win32-Code in
`letzterFehler` übernommen. Beim Schließen bleiben außerdem
`brokerVersion`, `letztesAck` und der alte Heartbeatzähler erhalten; nur Status,
Token, Version und Konflikt werden zurückgesetzt
(`PipeClient.cpp:278-286`). Das erschwert Feldanalyse gerade bei den oben
beschriebenen Abbruchfehlern.

`EqCopPipeProbe` wertet `heartbeatsGesendet >= 2` als „≥2 quittierte
Heartbeats“ (`PipeProbeMain.cpp:79-85`). Der Zähler steigt jedoch **vor** dem
ACK, und der Client akzeptiert irgendeinen Antwortframe als Fortschritt. Das
Probe-Bein kann daher grün werden, nachdem nur auf Heartbeat 1 irgendein Frame
kam und Heartbeat 2 gerade unbeantwortet blockiert. Der kanonische Lauf A4
führt Rust-Servertests aus; ein adversarialer echter C++-Client-Lifecycle ist
dort nicht enthalten.

Auch Brokerfehler werden teils verschluckt: Connection-Thread-Spawn
(`server.rs:339-341`) und Acceptor-Join (`:201-203`) verwerfen ihr Ergebnis,
obwohl der Funktionsvertrag „nichts scheitert still“ behauptet.

**Reproduktion/Testidee:** Fake-Server beantwortet Heartbeat 1 mit
`{"type":"quatsch"}` und Heartbeat 2 gar nicht; die Probe muss rot werden.
Für jeden I/O-Abbruch muss der Snapshot einen frischen, konkreten Fehler tragen
und alte ACK-/Brokerfelder dürfen nicht als aktueller Zustand erscheinen.

**Korrekturrichtung:** `heartbeatsGesendet` und `heartbeatsBestaetigt` trennen,
ACK erst nach Typ-/Seq-Prüfung zählen, Verbindungszustand atomar als neue
Generation publizieren und alle falliblen Thread-/Win32-Operationen sichtbar
melden.

## Zusätzlicher Lifecycle-Race an der öffentlichen Client-API

Nicht als zwölfter Kernbefund gezählt, weil seine heutige Aufrufer-Parallelität
hostabhängig ist, aber vor einem Fix mitzuprüfen:

`start()` und `stop()` schützen das `std::thread`-Objekt nicht gemeinsam
(`PipeClient.cpp:112-133`). Liegt Stop nach `laeuft.exchange(true)`, aber vor
`thread = std::thread(...)`, sieht Stop noch keinen joinbaren Thread und kehrt
zurück. Danach wird ein joinbarer, sofort auslaufender Thread veröffentlicht,
während `laeuft=false` ist; ein späteres `stop()` kehrt wegen des frühen
`exchange(false)` ebenfalls ohne Join zurück. Der `std::thread`-Destruktor kann
dann terminieren. Umgekehrt kann ein Start während eines noch nicht gejointen
Stop eine joinbare Threadinstanz überschreiben. Ein Lifecycle-Mutex oder eine
einzige Owner-Thread-State-Machine sollte diese API serialisieren. Ein
Barrieretest zwischen Exchange und Threadzuweisung macht den Fall
deterministisch.

## Widerlegte beziehungsweise positive Punkte

Diese Eigenschaften sind am heutigen Source-Stand **nicht** gebrochen:

- Die C++- und Rust-Grenze stimmt auf 262.144 Byte überein
  (`EqCopilotIds.h:21`, `framing.rs:7-9`). Null- und Oversize-Frames werden auf
  beiden Leseseiten abgelehnt.
- Beide Leser sammeln Header/Payload in Schleifen und vertragen fragmentierte
  Reads (`PipeClient.cpp:310-337`, `framing.rs:36-67`). Rust validiert UTF-8
  vor `String`; PIP-05 ist spezifisch die fehlende C++-Gegenseite.
- Der Broker hält den `Register`-Mutex nicht über Pipe-Read/-Write oder
  `FlushFileBuffers`; Heartbeat-Update und Konfliktabfrage sind kurz
  (`server.rs:430-445`). PIP-08 kann die kritische Sektion durch Mapgröße
  trotzdem indirekt teuer machen.
- Der Audiocallback ruft weder `PipeClient` noch Pipe-/Datei-I/O. Stats kommen
  über Atomics; der große Messsnapshot wird nur zwischen Worker und Pipe-/UI-
  Threads unter `AnalyseEngine::snapMutex` kopiert
  (`PluginProcessor.cpp:614-677`, `AnalyseEngine.cpp:1088-1098`). Es wurde kein
  Telemetrie→Gain- oder Pipe→Audiolock-Pfad gefunden.
- `PIPE_REJECT_REMOTE_CLIENTS`, eine explizite Current-User-DACL und
  `FILE_FLAG_FIRST_PIPE_INSTANCE` härten den **ehrlichen Broker** sinnvoll
  (`server.rs:164-184,243-252`). Sie ersetzen PIP-01/PIP-02 auf der
  Clientseite nicht.
- Der v2-Happy-Path `hello → welcome → heartbeat → ack → bye`, feindliche
  Längenpräfixe und doppelte IDs besitzen Rust-Tests
  (`server.rs:549-791`). Die Befunde betreffen gerade die fehlenden
  Abbruch-/Slowloris-/Reconnect-/Peer-Negativpfade.

## Empfohlene Integrationsreihenfolge

1. **Sofortiger kleiner Sicherheitsriegel:** PIP-01 und PIP-05 im heutigen
   Client; beide sind lokal, eng und unabhängig von der v3-Architektur.
2. **Ein gemeinsamer Client-Lifecycle-Fix:** PIP-03, PIP-04 und der zusätzliche
   start/stop-Race als eine State-Machine lösen, nicht als einzelne
   `if`-Pflaster.
3. **Broker-Lifecycle gemeinsam:** PIP-07, PIP-08 und PIP-09 über Overlapped
   Accept/I/O, feste Caps, besessene Connection-Handles und joinbaren Stop.
4. **Protokollgrenze:** PIP-02 und PIP-06 in `SONDE-010` mit echter
   Peer-/Sessionauthentisierung und strikt typisierter State-Machine schließen.
5. **Beweis:** Fake-Peer-Matrix für SQOS, malformed UTF-8/JSON, blockierte
   Reads/Writes, Reconnect-Generationen, Slowloris, Stop/Restart und falsche
   Session/Seq; danach echter Prozess-E2E ausschließlich auf eindeutigem
   Probe-Pipenamen.

## Restbeweis

In diesem read-only Paket wurde kein neuer Build und kein Lauf erzeugt. Der
aktuelle Session-Basislauf meldet die vorhandenen Rust-Unit-/Servertests grün,
aber diese decken die oben genannten Client- und Shutdown-Gegenpfade nicht. Ein
Source-Fix ist daher erst nach den expliziten Deadline-/Fake-Peer-Tests als
geschlossen zu werten.

## Revalidierung nach Broker-Folgefix (2026-08-27)

Der ausdrücklich beauftragte Folgefix änderte ausschließlich
`broker/src/server.rs` und `broker/src/protokoll.rs`; die parallel bearbeiteten
`lib.rs`, `vertrag.rs`, `telemetrie.rs` und generierten Dateien blieben
unangetastet. Die folgenden Statusaussagen ersetzen nur die **Broker-Hälfte**
der ursprünglichen Befunde. PIP-01 bis PIP-05 sowie die clientseitigen Teile
von PIP-06 bleiben davon unberührt.

### PIP-06 — Broker-State-Machine geschlossen, Clienthälfte offen

**Neuer Brokerstatus:** umgesetzt und adversarial getestet.

- `Established` ist jetzt an das erste gültige Hello, genau eine Sensor-ID und
  das ausgegebene Sessiontoken gebunden (`server.rs:545-643`).
- Fremdes Heartbeat-Token, fehlendes/fremdes Bye-Token, Sequenzduplikat oder
  -rücklauf und ein Zweit-Hello werden gezählt, sichtbar protokolliert und
  schließen die Verbindung (`server.rs:643-711`). Ein älteres Paket kann den
  jüngeren Messstand damit nicht mehr überschreiben.
- Tests decken fremdes Token, beide ungültigen Bye-Formen, Rücklauf und
  Duplikat sowie das versuchte Rebinding durch Zweit-Hello ab
  (`server.rs:1101-1205`).

**Verbleibende Grenze:** Die im Ausgangsbefund beschriebenen C++-Prüflücken
für Welcome/ACK/Seq und die ungenaue Probe-Zählung sind nicht Teil dieses
Brokerpakets und bleiben offen. Ein Protokollbruch bekommt bewusst keinen
neuen Fehlerframe; der Broker schließt, damit der synchrone Client sicher aus
seinem Read zurückkehrt.

### PIP-07 — Acceptor und Connection-Threads besessen und abbrechbar

**Neuer Brokerstatus:** für den synchronen v2-Windows-Server umgesetzt und
getestet.

- `ServerGriff` besitzt nun die Connection-JoinHandles. Stop setzt zuerst das
  Flag, bricht die blockierende I/O des Acceptor-Threads über dessen stabiles
  `JoinHandle` mit `CancelSynchronousIo` ab, joint ihn und tut danach dasselbe
  mit allen Connection-Threads (`server.rs:226-329`). Damit kann nach dem
  Acceptor-Join kein neuer Connection-Thread mehr in die Registry gelangen.
- Der erste Pipehandle bleibt bis zur echten Übergabe in RAII; der frühere rohe
  Stop-Leak ist entfernt (`server.rs:421-450`). Acceptor-/Connection-Panics,
  unerwartete Cancel-Fehler sowie Thread-Spawnfehler landen im begrenzten
  Fehlerregister (`server.rs:203-275,510-529`).
- Ein etablierter, danach stiller Client wird beim Stop innerhalb einer
  Zwei-Sekunden-Testdeadline beendet. Zusätzlich laufen hundert unmittelbare
  Start→Stop→Neustart-Runden jeweils auf einem eigenen Testpipenamen
  (`server.rs:1275-1305`). Der alte Wegwerfclient als Wakeup existiert nicht
  mehr.

**Verbleibende Grenze:** Das Design baut auf der dokumentierten
`CancelSynchronousIo`-Garantie für dasselbe Prozess-Threadhandle. Ein
unerwarteter dauerhafter Kernel-/Handlefehler hat absichtlich keinen
`TerminateThread`-Fallback, weil dieser Rust-/Lockzustand korrumpieren könnte.
Der Test ist ein belastbarer Race-/Deadline-Beleg, aber kein 10.000-Runden-
Stresstest und keine fault-injizierte Windows-API-Matrix.

### PIP-08 — Ressourcenwachstum hart begrenzt, Slowloris innerhalb des Caps offen

**Neuer Brokerstatus:** wesentlich entschärft, aber nur **teilweise
geschlossen**.

- Produktion verwendet höchstens 64 aktive Connections/Pipeinstanzen statt
  `PIPE_UNLIMITED_INSTANCES`; die Pipe erhält denselben Win32-Instanzcap
  (`server.rs:47,347-466`). Vor dem Spawn wird atomar reserviert, nach jedem
  Ausgang per RAII freigegeben. Spawnfehler sind sichtbar
  (`server.rs:504-529`).
- Das persistierende Register nimmt höchstens 128 unterschiedliche Sensor-IDs
  an; bekannte IDs dürfen weiterhin reconnecten (`server.rs:52,590-624`). Das
  verhindert serielles Tombstone-Wachstum trotz Connection-Cap.
- Hello validiert vor dem Registereintrag Sensor-ID, Nonce, Pluginversion,
  Label, Pair-ID, Rolle sowie endliche und begrenzte Audiofelder. Die Grenzen
  sind codepunktgenau 64/64/64/120/64 Zeichen, 768 kHz, Blockgröße 65.536 und
  64 Kanäle (`protokoll.rs:13-20,178-244`). Nullwerte der Audiofelder bleiben
  für den vorhandenen pre-prepare-v2-Client kompatibel.
- Tests erzwingen Connection-/Pipe-Cap 2, Sensor-ID-Cap 2 mit erfolgreichem
  Known-ID-Reconnect sowie Unicode-/Grenzwertfälle des Hello
  (`server.rs:1208-1272`, `protokoll.rs:334-391`).

**Verbleibende Grenzen:** Ein Client kann weiterhin bis zu 64 Slots vor dem
Hello oder zwischen Frames unbegrenzt lange belegen; das Wachstum ist jetzt
begrenzt, die Verfügbarkeit aber nicht gegen einen Same-User-Slowloris
geschützt. Es gibt noch keine Bootstrap-/Idle-Deadline, Rategrenze oder
stale-first Eviction. Nach 128 jemals gesehenen IDs werden neue IDs bis zum
Brokerneustart abgelehnt. Die zusätzlichen defensiven v2-Caps sind noch nicht
vollständig in `eq-ipc.schema.json` gespiegelt; die vorhandenen C++-Writerwerte
liegen innerhalb der Caps, eine spätere Vertragsangleichung bleibt nötig.
Thread-Spawnfehler sind im Produktpfad sichtbar, wurden mangels sicherer
deterministischer Spawn-Fault-Injektion aber nur per Codepfad/Clippy, nicht per
Negativtest bewiesen.

### PIP-09 — Kein unbegrenzter Flush mehr

**Neuer Brokerstatus:** umgesetzt und adversarial getestet.

Normale Sessionenden rufen nur noch `DisconnectNamedPipe`; ein ungelesenes
Welcome kann den Connection-Thread nicht mehr festhalten (`server.rs:730-738`).
Ein Reject muss andererseits zuverlässig lesbar bleiben. Nur dafür läuft der
Flush in einem besessenen Hilfsthread, hat eine harte 100-ms-Frist und wird
danach mit `CancelSynchronousIo` abgebrochen und gejoint
(`server.rs:56,740-773`). Spawn-/Cancelfehler werden sichtbar.

Zwei Cap-1-Gegentests halten den ersten Clienthandle offen: einmal bleibt das
Welcome ungelesen, einmal der Reject. In beiden Fällen kann danach wieder eine
zweite Verbindung geöffnet werden (`server.rs:1310-1354`). Damit ist gerade
der zuvor deterministische Thread-Leak reproduzierbar ausgeschlossen.

### Ausgeführter Beweis

Alle Testpipes folgen ausschließlich
`\\.\pipe\evenacadia.eq-copilot.test.<pid>.<eindeutiger-suffix>`; kein Test
öffnet oder startet die Produktionspipe.

- `cargo test --manifest-path broker/Cargo.toml`: **grün**, 70 Rust-Libtests
  und 8 Cross-Language-Integrationstests, außerdem Binär-/Doc-Testziele ohne
  Fehler.
- `cargo clippy --manifest-path broker/Cargo.toml --all-targets -- -D warnings`:
  **grün**, keine Warnung.
- `rustfmt --edition 2021 broker/src/server.rs broker/src/protokoll.rs` wurde
  ausschließlich auf den zwei eigenen Produktdateien ausgeführt.

Nicht gelaufen sind ein externer Mehrprozess-Slowloris, Application Verifier,
Win32-Fault-Injektion und der vollständige kanonische Sonden-Beweislauf. Die
oben genannten Unit-/Integrationstests sind der aktuelle, ehrliche Beleg für
dieses abgegrenzte Brokerpaket.

## Zweite Revalidierung: NAK-10, PIP-10 und v2-Vertragsparität (2026-08-27)

Dieser anschließende, ausdrücklich erweiterte Integrationsauftrag änderte
`broker/src/lib.rs`, `broker/src/protokoll.rs`, `broker/src/server.rs` und
`eq-copilot/schemas/eq-ipc.schema.json`. Er ersetzt die obige Aussage, dass
`lib.rs` unangetastet blieb, **nur für diesen zweiten Änderungssatz**. Parallel
vorhandene Änderungen in `lib.rs` (stabile Sortierung und saturierende
Paarfensterarithmetik) wurden erhalten.

### NAK-10 — Hör-Markierung wird getragen und fremde Evidenz fail-closed

**Status:** umgesetzt und adversarial getestet.

- `MessStand` trägt jetzt das bereits vom C++-Writer gesendete Feld
  `hoermarkierung`; fehlend bleibt es für ältere v2-Sender additiv `false`
  (`protokoll.rs:127-159`). Das Schema beschreibt ausdrücklich den sauberen
  eigenen Tap vor der Färbung, die fremde Kontamination und die unvermeidliche
  Heartbeat-Lücke (`eq-ipc.schema.json:99-107`).
- Der Broker hält den Markierungszustand je Instanz-Nonce. Der eigene
  Messabgriff des Markierers darf weiter angenommen werden; alle anderen
  Sensoren beziehungsweise Duplikat-Nonces werden ab dem beobachteten `true`
  quarantänisiert. Bereits angenommene Stände werden im Snapshot sofort
  verborgen, neue fremde Stände nicht als Evidenz übernommen
  (`lib.rs:265-369,470-486`). Paarstatus und Aggregatschreiben liefern einen
  sichtbaren Sperrgrund statt scheinbar belastbarer Ergebnisse
  (`lib.rs:552-653,917-942`).
- Ein beobachtetes `false` beendet die aktive Markierung, hebt die Quarantäne
  aber nicht rückwirkend auf. Freigabe braucht danach einen beweisbaren Reset:
  explizit `keine_daten` mit Nullzählern oder einen erst nach einer markerfreien
  Basis beobachteten Rücklauf von `gesamt_s` (`lib.rs:336-350`).
- Stale oder Disconnect bei letztem `true` beweist kein Ende und bleibt
  fail-closed sichtbar. Dieselbe danach nicht mehr lebende Nonce darf
  reconnecten und explizit `false` melden; die fremde Messung bleibt dennoch
  bis zu einem neuen, zeitlich einordenbaren Reset gesperrt
  (`lib.rs:401-431,552-567`).
- Unbestätigte, nacheinander wechselnde Marker-Nonces wachsen nicht selbst zur
  neuen Ressourcenlücke: höchstens 64 werden je Sensor gespeichert. Ein
  Überlauf wird sticky und bleibt fail-closed, statt eine nicht mehr
  zuordenbare Quelle still zu vergessen.
- Die Registertests decken Heartbeat-Reihenfolge (zuerst fremde Evidenz, dann
  Marker), eigenen sauberen Tap, Unterdrückung weiterer kumulativer Messung,
  Paar-/Aggregatsperre, beide Resetbeweise sowie stale→disconnect→Same-Nonce-
  reconnect→false ab (`lib.rs:1149-1293`); der Ressourcen-Gegentest beginnt
  bei `lib.rs:1296`.

**Unvermeidliche v2-Grenzen:** Das Flag kommt im Richtwert-1-Hz-Heartbeat.
Zwischen tatsächlichem Einschalten und dem ersten empfangenen `true` kann ein
Aggregat die Markierung daher noch nicht kennen. Weil `measurement` nur einen
kumulativen Stand seit Reset liefert, kann der Broker einen Reset zwischen
dem letzten beobachteten `true` und dem ersten beobachteten `false` nicht
beweisen; er sperrt konservativ weiter. Verschwindet eine wahr meldende
Instanz endgültig und kehrt nicht mit derselben Nonce/`false` zurück, bleibt
das Ende unbestätigt. Das aktuelle Aggregatschreiben sperrt außerdem bewusst
global, auch wenn ein späterer Filter theoretisch nur den sauberen Marker-Tap
auswählen könnte; selektives Herausschneiden wäre ein eigener v3-Vertrag.

### PIP-10 — Liveness intern monoton

**Neuer Status:** umgesetzt und getestet.

`last_seen_ms`, `verbunden_seit_ms` und `getrennt_seit_ms` bleiben UTC-Ausgabe.
Zusätzlich besitzt jede Verbindung einen nicht serialisierten monotonen
Zeitstempel aus `Instant`; ausschließlich dieser entscheidet `stale`
(`lib.rs:43-71,122-138,176-248,470-478`). Private Zeitstempel-/Snapshotpfade
sind für Tests injizierbar. Der Gegentest springt UTC erst stark zurück, dann
bis nahe `u64::MAX` vor: Frische ändert sich nicht; erst monotone
`STALE_MS + 1` setzt stale (`lib.rs:1115-1146`). Damit ist PIP-10 geschlossen,
ohne die öffentliche UTC-Wireform zu ändern.

### Instanzkonflikt-Lifecycle

**Status:** geschlossen für die bestätigten Brokerfälle.

- Handshakeprüfung und Registereintrag laufen unter demselben Lock. Eine
  bereits lebende Kombination `(sensor_id, instance_nonce)` bekommt jetzt
  `reject`; die erste Verbindung bleibt allein lebend, kann weiter heartbeat/
  ACK austauschen und anschließend korrekt trennen (`server.rs:609-640`, Test
  `:1213-1253`). Fehlende Nonce bleibt für v1 kompatibel servergeneriert, ein
  vorhandener leerer String wird dagegen nicht länger still als UUID
  umgedeutet (`protokoll.rs:270-287`, `server.rs:597-604`, Test `:1256-1277`).
- Für unterschiedliche lebende Nonces derselben Sensor-ID speichert das
  Register die vollständigen Hello-Metadaten je Verbindung. Trennt der jüngste
  Owner, werden Rolle, Label, Pair-ID, Plugin-/Protokollversion, Host-PID,
  Audioformat und Zeiten aus dem überlebenden Owner restauriert. Alte
  Stats/Messung werden entwertet, bis dieser Owner selbst wieder schreibt
  (`lib.rs:65-71,216-248,401-426`; Regression `:1059-1113`).

### Ungültige Messung und v1-Versprechen

**Status:** geschlossen.

`MessStand::ist_gueltig` erzwingt neben Zustand/LTAS nun eine nichtleere,
maximal 64 Codepunkte lange Metrics-Version, endliche und nichtnegative
`aktiv_s`/`gesamt_s` sowie endliche optionale Skalare und LTAS-Werte
(`protokoll.rs:122-199`). Ein negativer oder übergroßer Messstand beendet die
Session ohne ACK und ohne Last-Seen-/Heartbeat-Aktualisierung; der vorherige
Messstand des Owners wird atomar entwertet, bevor Disconnect ihn sichtbar
unklar macht (`server.rs:217-228,697-734`, Test `:1112-1181`). Ein
`measurement`-Objekt in einer ausgehandelten v1-Session ist derselbe
Protokollbruch und kann nie im Register landen (`server.rs:700-708`, Test
`:1184-1210`).

### v2-Schema-/Rust-Parität

**Status:** für die geprüfte Rust-Consumer-/Schema-Grenze geschlossen; die
separate C++-Reader-Härtung gehört nicht zu diesem Paket.

Die im ersten Revalidierungsabschnitt noch genannte fehlende Spiegelung der
defensiven v2-Caps im Schema ist mit diesem zweiten Änderungssatz geschlossen.

- Alle Clientnachrichten und ihre verschachtelten Objekte verwenden
  `deny_unknown_fields`; schemawidrige Extras werden nicht mehr still
  ignoriert. Explizites `null` für `instance_nonce`, `pair_id`, `stats` und
  `measurement` stimmt nun zwischen serde und Schema überein. `bye` verlangt
  im Schema wie der Established-Server ein nichtleeres Sessiontoken
  (`protokoll.rs:30-215`, Tests `:468-506`; Schema `:17-48,75-97,148-157`).
- Hello-/Response-/Token-/Metrics-Caps stehen nun auch im Schema. Vorhandene
  leere Sensor-/Instanz-/Pair-IDs werden abgelehnt; fehlende optionale Felder
  bleiben kompatibel (`eq-ipc.schema.json:17-73,75-107,137-157`).
- Gemeinsame Integerbreiten sind explizit: `host_pid` und `spruenge` bis
  `u32::MAX`; `seq`, `frames_dropped`, `project_time_samples`, `von_samples`
  und `bis_samples` im signed-i64-Bereich, die Zähler zusätzlich
  nichtnegativ. Rust verwendet für Wire-Seq/Frames nun `i64`, prüft die
  nichtnegative Semantik und testet jeweils N/N+1 beziehungsweise beide
  i64-Kanten. RMS ist explizit auf endliches `float32`, der Messstand auf
  endliches `float64` begrenzt; Maximum und Overflow werden gegengeprüft
  (`protokoll.rs:67-115,508-632`; Schema `:24-26,82-94,104-145`).

### Frischer Beweis des zweiten Änderungssatzes

Alle Servertests verwenden ausschließlich eindeutige Namen nach
`\\.\pipe\evenacadia.eq-copilot.test.<pid>.<suffix>`; die Produktionspipe
wurde von keinem Test geöffnet.

- `cargo test --manifest-path broker/Cargo.toml`: **grün**, zuletzt 90
  Rust-Libtests und 8 Cross-Language-Integrationstests; Binär- und Doc-Ziele
  ebenfalls ohne Fehler.
- `cargo clippy --manifest-path broker/Cargo.toml --all-targets -- -D warnings`:
  **grün**, keine Warnung.
- `py -3.13 tools/eq-copilot/pruefe_v2_schemas.py`: **grün**, 5/5 v2-Schemas
  syntaktisch und gegen Draft 2020-12 gültig.
- Zusätzliche Standard-`jsonschema`-Grenzmatrix: **grün**, 4 gültige Null-/
  Maximalfälle angenommen und 7 Bye-/Integer-/Float-N+1-Fälle abgelehnt.
- `git diff --check` auf den vier Produktpfaden: **grün** (nur erwartete
  Git-Zeilenendewarnungen). `rustfmt` lief ausschließlich auf den vollständig
  eigenen Dateien `server.rs` und `protokoll.rs`; `lib.rs` wurde wegen
  paralleler fremder Änderungen bewusst nicht kollateral formatiert.

Nicht Teil dieses Belegs bleiben PIP-01 bis PIP-05 und die clientseitige Hälfte
von PIP-06. Ebenfalls nicht gelaufen sind externer Mehrprozess-Slowloris,
Application Verifier, Win32-Fault-Injektion und der vollständige kanonische
Sonden-Beweislauf.
