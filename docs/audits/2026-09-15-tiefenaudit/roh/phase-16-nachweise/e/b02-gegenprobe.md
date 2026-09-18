# Unabhängiger Gegenangriff auf B-02 / F15

18.09.2026, Prüfer Paket E, unabhängig vom Finder B. Produktstand `aff2d8188f33a9525aec1869449773bfdbe305a6`. Geprüft: unveränderte Quellen in der isolierten Kopie sowie [B-Nachweiscode](../b/wire.rs) und [dessen tatsächliche Ausgabe](../b/wire-run.txt). Der B-Lauf wurde von E **nicht nochmals ausgeführt**; E prüft Einstieg, Assertions, Gegenwege, Vertrag und Abgrenzung. Keine Sicherheitsprüfung.

**Urteil: eigenständige LÜCKE, kein bereits eindeutig spezifizierter DEFEKT und kein Duplikat zu NAK-95.** Der Verlust nach erfolgreichem Pipe-Write ist tatsächlich belegt. Ein pauschales „alle P1-Ereignisse sind unter jedem Linkende garantiert zugestellt“ wäre jedoch eine weitergehende Produktregel, die aus der vorhandenen Matrix nicht folgt.

## Angriff auf die Beweiskraft

`wire.rs:148-173` verwendet den wirklichen v3-Listener und Coordinator. Der eigene `Senke`-Adapter ruft zuerst den echten Coordinator auf und hält **danach** einmal den P1-Verbraucher an (`:102-103`). Die erste Evidenz ist folglich wirklich angenommen. Drei weitere vollständig geschriebene Frames sind durch den Ingress-Höchststand3 auf der Serverseite belegt; anschließend wird der Clienthandle geschlossen. Nach Wiederfreigabe bleibt der echte Annahmezähler1. Ein neuer normaler Link derselben Adresse kann neue Evidenz empfangen, die drei früheren IDs fehlen weiterhin. `ingress_overflow=0` unterscheidet den Fall vom Überlaufpfad.

**Beweisgrenze:** Die sendende Seite ist ein eigener Rust-Drahtclient. Damit misst dieser Lauf nicht selbst das endgültige Ausreihen im produktiven C++-Client. Diese Hälfte ist als Quellenherleitung geschlossen: `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:895` entnimmt P1, `:939-943` legt beim gescheiterten Write zurück, `:1003` bestätigt nach erfolgreichem Write lokal. Ein Empfänger-ACK für die Evidenz-ID wird dabei nicht abgewartet. Der Transport-InFlight-Mechanismus für persistenzpflichtige P0-Befehle ist kein P1-Evidenzregister (`controlclient/Nachrichten.cpp:179-209`, `:258-281`).

## Stärkster Gegenweg: Verwerfen beim Linkende ist ausdrücklich gebaut

`broker/src/transport/server_v3/queues.rs:35-41` erklärt, warum nach dem Schließen keine Restnachrichten mehr an bereits abgemeldete Sitzungen gehen dürfen. `:55-59` prüft das Schließflag vor jeder Entnahme. Das entspricht der geschlossenen NAK-92-Nr.4 (`docs/offene-punkte.md:459`): das frühere Nachlaufen von P0/P1-Callbacks nach der Abmeldung war seinerseits fehlerhaft. Eine Reparatur „Restqueue trotz Abmeldung vollständig leeren“ wäre deshalb kein zulässiger einfacher Gegenfix.

Der Kontrast trägt die Lücke: der Sendepfad betrachtet einen erfolgreichen Write als ausreichend, während der Empfangspfad den schon gelesenen Restbestand beim normalen Ende ausdrücklich verwerfen muss. Es fehlt die gemeinsame Regel, ab welcher Empfangs-/Persistenzbestätigung der Sender seine unverwechselbare Evidenz aufgeben darf, oder welche gezählte Verlustsemantik stattdessen gilt.

## Warum vorhandene Zusagen keine uneingeschränkte Zustellgarantie ergeben

- `docs/beweise/SONDE-010.md:159-162` (`A-P1-09/10/11/12`) behandelt reservierte beziehungsweise noch im Client liegende Einträge. Ein bereits erfolgreich geschriebener und lokal bestätigter Eintrag gehört nicht mehr dazu.
- `docs/beweise/SONDE-010.md:191` (`A-IN-04`) legt für **Überlauf** das sichtbare Trennen fest und verschiebt die Ende-zu-Ende-Wiederholung ausdrücklich an Outbox/Coordinator in SONDE-011. Der hier gemessene Ingress ist nicht voll.
- `docs/FL-Nakama-Sonden-Design-Entwurf.md:4132` fordert Wiederholung nicht koaleszierbarer Events ausdrücklich **bei Überlauf**. Die anschließende Outboxzusage (`:4165-4167`) beweist Commit-/Outboxgrenzen; sie ist kein belegter Receiver-ACK-Vertrag für jede vorausgehende P1-Writegrenze.

**Keine harmlose Snapshotkoaleszierung:** `SondeProcessor.cpp:841-853` vergibt für jede Evidenz einen leeren Koaleszenzschlüssel, weil zwei `evidence_id` ausdrücklich zwei verschiedene Belege sind. `:879-894` leert die darin enthaltenen Ereignisse nach erfolgreicher Queueübergabe. Der nächste Snapshot ersetzt daher nicht einfach dieselben Evidenz-IDs und bereits entnommenen Transientenereignisse. Eine neue vollständige Intentmeldung beim Linkaufbau ist für diese Historie kein Wiederherstellungsweg.

## Abgrenzung und kleinster kohärenter Folgeauftrag

NAK-95-Nr.1/2/3 (`docs/offene-punkte.md:453`) betreffen den Abfluss des Clientwiederholpuffers, den erhaltenen Koaleszenzschlüssel und eine Verdrängung bei voller Rust-P1-Queue. Keiner behandelt den hier gemessenen Zwischenraum nach erfolgreichem Write und vor fachlicher Annahme. T3-03-01 verwirft aufgrund schemawidriger Serialisierung; T3-08-02 betrifft bereits fachlich angenommene und später falsch persistierte Invalidierungen; T3-14-03 betrifft eine sitzungsweite Wiederfreigabe. Diese Ursachen werden nicht ein zweites Mal gezählt.

Für Fable: Annahmegrenze und zulässigen Verlust zuerst festlegen; dann Senderhaltung/Wiederholung, Empfänger-Deduplikation, Generation und Persistenz in einem zusammenhängenden Änderungssatz behandeln. Ein Regressionstest muss echte C++-Producer/ControlClient gegen echten Listener+Coordinator verbinden, nach erfolgreichem Write vor dem P1-Callback trennen und die ursprünglichen Evidenz-IDs nach Wiederaufbau verfolgen. Ein zweiter Fall muss beweisen, dass nach Linkabmeldung kein Callback gegen eine veraltete Sitzung ausgeführt wird. Überlauf und gewöhnliches Linkende sowie neue volle Intentmeldung und unverwechselbare Evidenz getrennt prüfen. Keine automatische Ausweitung auf Sicherheitsarbeit oder neue Produktfunktionen.
