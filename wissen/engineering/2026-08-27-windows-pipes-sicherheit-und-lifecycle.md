---
id: w-2026-08-27-windows-pipes-sicherheit-und-lifecycle
typ: wissenseintrag
status: aktiv
datum: 2026-08-27
aktualisiert: 2026-08-27
hauptbereich: engineering
weitere_bereiche: [fl-studio]
pruefstand: mehrfach-belegt
---

# Windows-Pipes sind Sicherheits- und Lebenszyklusverträge

## Menschliches Warum

Ein Plugin muss sich schließen, neu laden und wieder verbinden lassen, ohne die
DAW festzuhalten. Gleichzeitig darf ein lokaler Gegenprozess weder die
Identität des Hosts missbrauchen noch Befehle oder Messwerte vortäuschen. Für
den Produzenten zeigt sich beides als dieselbe Qualitätsfrage: Ist die Session
verlässlich und endet sie wirklich?

Die Kausalkette lautet:

> Weil ein Produzent Plugins in einem privilegierten, langlebigen Host öffnet
> und schließt, braucht jede Pipe begrenzte Rechte, Fristen und eindeutig
> besessene Lebenszyklen, damit weder ein lokaler Angreifer noch ein hängender
> Peer die DAW-Session kapert oder blockiert.

## Belegtes Wissen

Microsoft dokumentiert zwei oft übersehene Eigenschaften von Named Pipes:

- Ein Server ohne eigene Sicherheitsbeschreibung erhält eine Default-DACL.
  Diese gewährt nicht nur System, Administratoren und Ersteller Rechte,
  sondern auch Everyone und Anonymous bestimmte Leserechte. Der gewünschte
  lokale Benutzerkreis muss daher ausdrücklich festgelegt und geprüft werden.
- Ein Pipe-Server darf einen Client impersonieren. Ohne gesetzte SQOS-Flags
  erhält er standardmäßig `SecurityImpersonation`; ein Client kann dies mit
  `SECURITY_SQOS_PRESENT` und etwa `SECURITY_IDENTIFICATION` begrenzen. Nach
  jeder Impersonation muss der Server auf jedem Pfad `RevertToSelf` erreichen.

Synchrones Pipe-I/O kann unbegrenzt blockieren. `CancelSynchronousIo` markiert
ausstehendes I/O zur Abbrechung, wartet aber nicht auf dessen Abschluss und
einzelne Operationen können weiterlaufen. Auch `FlushFileBuffers` ist kein
harmloser Abschluss: Auf der Serverseite kehrt der Aufruf erst zurück, wenn der
Client alle gepufferten Bytes gelesen hat.

## Fehlerlexikon und Gegenbeweis

| Fehlerquelle | Beobachtbare Folge | Erforderlicher Gegenbeweis |
|---|---|---|
| Default-DACL oder globaler Pipename | fremder Prozess liest, belegt oder imitiert einen Endpunkt | ACL- und fremder-Benutzer-Test am echten Namen |
| fehlende Client-SQOS | Server darf Hostidentität stärker impersonieren als nötig | Handle-Flags prüfen und Identifikationsstufe testen |
| Impersonation ohne garantierten Rückweg | Server arbeitet nach Fehler weiter als Client | Fehler-Injektion vor jedem `RevertToSelf`-Pfad |
| nur Timeout pro Teil-Lesevorgang | Drip-Feed hält Kopf oder Body beliebig lange offen | eine absolute Deadline über den ganzen Frame |
| Cancel als Abschluss behandelt | Handle/Thread wird wiederverwendet, während I/O noch läuft | Cancel, Join und Handle-Lebensdauer unter Rennen testen |
| `FlushFileBuffers` beim Trennen/Ablehnen | nicht lesender Peer hält Serverthread fest | kein Flush im normalen Disconnect; bösartiger Peer-Test |
| unbegrenzte Verbindungen oder Tombstones | lokaler Slowloris erschöpft Threads und Speicher | harte Caps plus Bootstrap- und Idle-Deadline |
| zwei Pipes halb verbunden | Control gilt, Telemetry fehlt oder stammt aus alter Session | gemeinsamer Session-Epoch und Half-open-State-Test |
| P2-Flut vor P0/P1 | Telemetrie verdrängt Steuerung und Sicherheitsantworten | Prioritäts- und Sättigungstest mit garantierter P0-Liveness |
| Stop/Reconnect ohne Generation | verspäteter ACK gehört scheinbar zur neuen Session | Generations-ID, Session-ID und verspätete-Paket-Fixture |

## Bedeutung für Nakama

Die Debugging-Session reparierte nicht nur einzelne Pipe-Aufrufe, sondern den
gesamten Besitzvertrag: Client-Handles und Brokerthreads werden bis zum Join
besessen, Stop und Reconnect invalidieren die Generation, Welcome/ACK/Session
und Sequenz werden exakt geprüft und der Broker begrenzt aktive Verbindungen
und Sensor-IDs. Die Client-SQOS wurde auf Identifikation begrenzt. Ein
besonders wertvoller Gegenbeweis zeigte, dass fünf Sekunden je Teiloperation
keine fünf Sekunden je Frame sind; die Frist läuft nun absolut über Kopf und
Body.

Für SONDE-010 und die spätere Paarung bleiben zwei Fehlerquellen ausdrücklich
offen: Ein gleichberechtigter lokaler Peer kann ohne Bootstrap-/Idle-Frist
weiterhin die begrenzten Slots langsam belegen, und eine DACL ersetzt keine
kryptografische Serverauthentisierung. Die geplante HMAC-Schicht muss deshalb
mit Pipe-Namensbesitz, Session-Epoch, Replay-Schutz und Lebenszyklus gemeinsam
geprüft werden.

## Grenzen, Widersprüche und offene Fragen

Named Pipes sind lokal, aber nicht automatisch vertrauenswürdig. Welche DACL
und Identität Nakama genau bindet, ist eine Produkt- und Installationsfrage des
aktuellen Bauplans. Die hier genannten Tests sind Fehlerhypothesen aus den
Windows-Semantiken und der Debugging-Session; sie behaupten noch keine fertige
v3-Implementierung.

## Quellen

- Titel: *Named Pipe Security and Access Rights*
- Autor: Microsoft
- URL: https://learn.microsoft.com/en-us/windows/win32/ipc/named-pipe-security-and-access-rights
- Titel: *Impersonating a Named Pipe Client*
- Autor: Microsoft
- URL: https://learn.microsoft.com/en-us/windows/win32/ipc/impersonating-a-named-pipe-client
- Titel: *CancelSynchronousIo function*
- Autor: Microsoft
- URL: https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-cancelsynchronousio
- Titel: *FlushFileBuffers function*
- Autor: Microsoft
- URL: https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-flushfilebuffers
- Eingang: `web-recherche`
- geprüft: Microsoft-Win32-Referenz sowie Nakamas aktuelle Pipe-Implementierung,
  Regressionstests und Debugging-Beweise

## Verknüpfungen

- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/04-pipe-transport.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/04-pipe-transport.md)
- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/06-pipe-deadline.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/06-pipe-deadline.md)
- [`../../docs/FL-Nakama-Sonden-Design-Entwurf.md`](../../docs/FL-Nakama-Sonden-Design-Entwurf.md)
- [`2026-08-27-json-vertraege-vor-dem-parser.md`](2026-08-27-json-vertraege-vor-dem-parser.md)
