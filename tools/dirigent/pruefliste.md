# Prüfliste für Worker und Prüfer — aus den Befunden vom 23.–29.08.2026

Gewonnen aus rund 40 bestätigten Prüferbefunden der Tickets S8, S9, S9b und
S14–15 (Manifeste `docs/beweise/SONDE-007a/b/c.md`, `SONDE-010.md`; Register
NAK-84 bis NAK-95). Jede Zeile ist eine Klasse, die mehrfach vorkam. Der Worker
hakt sie vor dem Commit ab und nennt im Manifest, wo er sie gemessen hat; der
Prüfer prüft zuerst gegen diese Liste und die Verhaltensmatrix des Tickets,
dann erst frei.

## A. Rückstau und Prioritätsklassen (IPC, Queues)

- Je Klasse (P0/P1/P2) steht die Politik bei *voll* ausdrücklich: abweisen,
  koaleszieren, ältesten ersetzen — nie stillschweigend Akzeptiertes löschen.
- Ein Puffer, der „später wiederholt", hat einen Abflussweg **ohne** Reconnect;
  „nur beim Verbindungsaufbau leeren" ist ein Befund.
- Schlüssel (Snapshot-/Objekt-Identität) überleben jeden Zwischenpuffer;
  Koaleszierung gilt auch dort.
- Solange eine niedrigere Klasse blockiert (Write, Rückstau), wird die höhere
  weiter gelesen und beantwortet — Lesepfad nie hinter „erst alles senden"
  verstecken.
- Rückgabewerte und Zähler einer Politik werden vom Lastbein ausgewertet, nicht
  ignoriert; ein Zähler, der strukturell 0 sein muss, ist eine Wache mit Test.
- Dieselbe Regel in **allen** Sprachen (C++ und Rust) — der Test, der sie
  misst, existiert beidseitig und ist einmal gebrochen worden.

## B. Lebenszyklus (verbinden/trennen, starten/stoppen)

- Reihenfolge beim Verbinden ist festgelegt und gemessen: anmelden →
  Callback „verbunden" abgeschlossen → erst dann Welcome/Freigabe nach außen.
- Beim Trennen zuerst abmelden/entkoppeln, dann Fristen und Joins; nichts
  bleibt registriert, während auf einen Verbraucher gewartet wird.
- Jeder Join hat eine Frist; Callbacks laufen nie auf dem Thread, den `stop()`
  joint, oder `stop()` ist von innen erlaubt und getestet (Self-Join = Befund).
- Nach dem Schließen einer Queue wird nichts mehr geliefert; das Schließflag
  wird **vor** dem Inhalt geprüft.
- Stopp-Fenster: Registrierung nach begonnenem Stop ist ein Test, kein Zufall.

## C. Verträge und Längen

- Jedes Textfeld mit Schema-Länge wird im Leser gegen genau diese Länge
  geprüft (Zeichen vs. Bytes benannt); Negativfixture je Feld.
- Objekte werden mit exakter Feldmenge geprüft (`additionalProperties:false`
  hat eine C++- und eine Rust-Hälfte).
- Discriminator, Familie und Typ werden vor dem Inhalt geprüft; ein Frame der
  falschen Familie wird abgewiesen, in beiden Richtungen.
- NaN/Inf/Grenzwerte an jeder Zahl, die über den Draht geht.

## D. Bau- und Prüfriegel (CMake, Runner)

- Ein Riegel ist fail-closed ohne Rohtextheuristik: Unbekanntes ist ROT.
- Ein Bein, das „misst nie ein veraltetes Artefakt" behauptet, prüft die
  Frische seiner Eingaben (Stamp/mtime) und meldet Voraussetzung-fehlt (Exit 3)
  statt grün.
- Nach einem Relink ändern sich Bundlebytes; festgeschriebene Hashes gelten
  für eine Auslieferung, nicht für den Kanon (NAK-94).
- Was der Kanon nicht baut, darf er nicht als frisch bezeugen (NAK-93).

## E. Behauptung ≤ Messung

- Jede Behauptung im Runner, Manifest oder Kommentar sagt nicht mehr, als der
  Test misst („keine Allokation" braucht einen Allokationszähler; „isoliert
  gemessen" braucht die isolierte Probe).
- Zahlen im Manifest (Beinanzahl, Verbraucher, Fixtures) sind gemessen, nicht
  aus einer anderen Datei abgeschrieben.
- Positionen im Repo stehen im Manifest als Symbol/Anker oder als Zahl mit
  Commit (`Datei:Zeile @ sha7`); eine nackte Zeilennummer altert mit dem
  nächsten Edit und ist ein Befund (NAK-96 Nacharbeit 2, 29.08.2026). Die
  Regel gilt für das **ganze** Manifest, nicht nur für den neuen Abschnitt.
  Ausgenommen sind allein wörtlich zitierte Rohausgaben (Codeblöcke) und
  wörtliche Zitate (`>`-Blöcke): dort bleibt die Zahl, wie das Werkzeug oder
  die Quelle sie schrieb, und der **einleitende Satz** davor trägt den Commit
  (NAK-96 Nacharbeit 3, 29.08.2026).
- Ein Abschnitt, der in seinem Kopf seinen Stand (Commit) nennt, ist eine
  Momentaufnahme @ diesem Stand; nackte Zeilennummern darunter gelten als
  daran gebunden. Abschnitte ohne Standangabe binden jede Position einzeln
  (`@ sha7` oder Symbol). Wer ein altes Manifest nachzieht, setzt deshalb
  eine Zeile **Stand dieses Abschnitts:** `<sha7>` unter jede
  Überschrift ohne Standangabe, statt hunderte Einzelpositionen zu
  annotieren — und prüft, dass die genannten Positionen **an diesem
  Commit** stimmen (NAK-85 Runde 4, 29.08.2026).
- Jede neue Prüfung wurde einmal absichtlich gebrochen; Rohausgabe des Rots
  liegt bei. Eine Probe, die auch ohne den Fix rot ist, heißt Regressionswache,
  nicht Beleg.

## F. Änderungssatz

- speichern↔laden, starten↔stoppen, öffnen↔schließen, verbinden↔trennen,
  aktivieren↔abklingen, installieren↔Rückweg im selben Commit.
- Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher eines
  Vertrags im selben Änderungssatz.
