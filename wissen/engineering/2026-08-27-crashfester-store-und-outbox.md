---
id: w-2026-08-27-crashfester-store-und-outbox
typ: wissenseintrag
status: aktiv
datum: 2026-08-27
aktualisiert: 2026-08-27
hauptbereich: engineering
weitere_bereiche: []
pruefstand: mehrfach-belegt
---

# Crashfester Store und Outbox: mindestens einmal, genau einmal wirksam

## Menschliches Warum

Ein Produzent muss nach Absturz oder Stromausfall entweder den alten oder den
neuen Zustand wiederfinden – nicht eine unbemerkte Mischung. Ebenso darf ein
verlorener ACK weder eine Änderung verschlucken noch dieselbe Änderung zweimal
hörbar anwenden.

Die Kausalkette lautet:

> Weil ein Produzent Arbeit und Hörentscheidungen auch bei einem abrupten Ende
> behalten muss, müssen Zustand und ausgehendes Ereignis gemeinsam dauerhaft
> werden und Wiederholungen dieselbe Wirkung behalten, damit Recovery weder
> Verlust noch doppelte Anwendung erzeugt.

## Belegtes Wissen

SQLite erreicht atomaren Commit über eine genau geordnete Journal- und
Synchronisationsfolge. Vor dem Commitpunkt führt ein Absturz zur
Wiederherstellung des alten Zustands; danach gilt die Transaktion als
committet. In WAL-Betrieb können Leser und ein Schreiber weitgehend parallel
arbeiten, aber lange Lesetransaktionen können Checkpoints verhungern lassen.
SQLite erlaubt weiterhin nur einen gleichzeitigen Schreiber und kann auch im
WAL-Modus `SQLITE_BUSY` liefern.

Die Haltbarkeitsstufe ist Teil des Produktvertrags. SQLite dokumentiert für
WAL plus `synchronous=NORMAL`: Die Datenbank bleibt konsistent, eine als
committet gemeldete Transaktion kann nach Strom- oder Betriebssystemausfall
aber zurückrollen. `FULL` synchronisiert zusätzlich nach jedem Commit und ist
im WAL-Modus ACID.

Das Transactional-Outbox-Muster löst den Dual-Write-Fehler, indem
Geschäftszustand und ausgehendes Ereignis in derselben Datenbanktransaktion
geschrieben werden. Der Relay darf nach einem Absturz erneut senden. Deshalb
ist die Lieferung mindestens einmal, während genau-einmal-Wirkung erst durch
stabile Nachrichten-ID, Reihenfolge und idempotenten Verbraucher entsteht.

## Fehlerlexikon und Gegenbeweis

| Fehlerquelle | Verlust- oder Doppelpfad | Erforderlicher Gegenbeweis |
|---|---|---|
| Zustand committen, Nachricht danach erzeugen | Absturz dazwischen verschluckt Nachricht | Zustand und Outbox-Zeile in einer Transaktion |
| Outbox vor ACK als erledigt markieren | Sendung oder ACK geht verloren | erst bestätigte Wirkung abschließen |
| Outbox nach ACK markieren | ACK kommt an, Markierung crasht | erneute Zustellung mit gleicher ID ist wirkungsgleich |
| Reihenfolge nur aus UTC ableiten | gleiche oder rückwärts springende Uhr sortiert falsch | monotone Sequenz pro Vertrag/Session |
| Coalescing aller Ereignisse | Revert, Lease oder Revision wird verschluckt | explizite coalescible- und non-coalescible-Klassen |
| Retention vor Bestätigung | ungesendete Wahrheit wird gelöscht | Löschgrenze an ACK/Projection koppeln |
| langer Leser im WAL | WAL wächst, Checkpoint erreicht Ende nie | langlebigen Leser und manuellen Checkpoint töten/testen |
| `busy_timeout` als Garantie | Schreibkonflikt endet trotzdem in `SQLITE_BUSY` | definierte Retry-/Fail-closed-Regel mit Kill-Test |
| nur Hauptdatei sichern | WAL enthält neuere committete Daten | SQLite-Backupweg oder konsistenter Checkpoint |
| `NORMAL` als volle Dauerhaftigkeit lesen | Stromausfall kann letzten Commit zurückrollen | Haltbarkeitsentscheidung plus Power-loss-Grenze dokumentieren |
| Migration nur am Happy Path | Crash zwischen DDL und Datenprojektion | Killpunkt an jeder Migrationsgrenze und Wiederanlauf |
| mehrere Wahrheitsbesitzer | Store, Speicher und Probe widersprechen sich | ein Schreiber und deterministisch neu aufbaubare Projektionen |

## Bedeutung für Nakama

Die Debugging-Session zeigte dieselbe Problemform bereits beim Installer und
bei Broker-Snapshots: Ein privilegiertes Journal lag zunächst in einem
benutzerbeschreibbaren Pfad, Teilinstallationen hatten keinen dauerhaft
fortsetzbaren Rückweg und temporäre Dateien konnten kollidieren. Repariert
wurden geschützter Transaktionsanker, hashgeprüfte Backups, exklusive Sperre,
`CreateNew`-Temporärdateien, Flush und atomarer Ersatz sowie eine
driftbewusste Wiederherstellung.

SONDE-011 plant SQLite, Ereignisprotokoll und Outbox. Daraus folgt als
Nakama-Hypothese: Der Coordinator muss alleiniger Schreiber bleiben;
Zustandsänderung und Outbox entstehen atomar; eine wiederholte Nachricht
referenziert dieselbe Revision und bleibt wirkungsgleich. Kill-Tests müssen
nicht nur DB-Commit, sondern die Grenzen Store → Relay → Pipe → ACK →
Projection abdecken. WAL-Größe, Reader-Lebensdauer und `SQLITE_BUSY` sind
Qualitätswerte, keine internen Nebensachen.

## Grenzen, Widersprüche und offene Fragen

AWS beschreibt das Muster an verteilten Cloud-Diensten. Der zugrunde liegende
Dual-Write-Fehler gilt auch lokal, konkrete AWS-Dienste sind für Nakama aber
irrelevant. Ob Nakama `FULL` oder eine andere Haltbarkeitsstrategie benötigt,
entscheidet der Bauplan anhand des Verlustversprechens und gemessener Kosten;
dieser Eintrag legt sie nicht vorweg fest.

## Quellen

- Titel: *Atomic Commit In SQLite*
- Autor: SQLite Project
- URL: https://www.sqlite.org/atomiccommit.html
- Titel: *Write-Ahead Logging*
- Autor: SQLite Project
- URL: https://www.sqlite.org/wal.html
- Titel: *PRAGMA Statements – synchronous*
- Autor: SQLite Project
- URL: https://www.sqlite.org/pragma.html#pragma_synchronous
- Titel: *Transactional outbox pattern*
- Autor: AWS Prescriptive Guidance
- URL: https://docs.aws.amazon.com/prescriptive-guidance/latest/cloud-design-patterns/transactional-outbox.html
- Eingang: `web-recherche`
- geprüft: Originaldokumentation sowie Nakamas Installer-, Persistenz- und
  Debugging-Beweise

## Verknüpfungen

- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/03-targets-lifecycle-installer.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/03-targets-lifecycle-installer.md)
- [`../../docs/FL-Nakama-Sonden-Design-Entwurf.md`](../../docs/FL-Nakama-Sonden-Design-Entwurf.md)
- [`../../docs/plugin-wissen.md`](../../docs/plugin-wissen.md)
- [`2026-08-27-windows-pipes-sicherheit-und-lifecycle.md`](2026-08-27-windows-pipes-sicherheit-und-lifecycle.md)
