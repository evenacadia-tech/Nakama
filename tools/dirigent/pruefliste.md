# Prüfliste für Worker und Prüfer — aus den Befunden vom 23.–29.08.2026

Gewonnen aus rund 40 bestätigten Prüferbefunden der Tickets S8, S9, S9b und
S14–15 (Manifeste `docs/beweise/SONDE-007a/b/c.md`, `SONDE-010.md`; Register
NAK-84 bis NAK-95). Jede Zeile ist eine Klasse, die mehrfach vorkam. Der Worker
hakt sie vor dem Commit ab und nennt im Manifest, wo er sie gemessen hat. Für
den Prüfer ist sie Kontext, **keine Anforderungsquelle**: Anforderungen sind
allein Gate-Text, Verhaltensmatrix, Entwurf und die Invarianten aus
`CLAUDE.md` (Dirigent §3.4, 30.08.2026). Manifestprosa unter `docs/**` ist
kein Prüfgegenstand.

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
- Positionen im **lebenden Kopf** eines Manifests stehen als Symbol/Anker;
  im Verlauf gilt eine Zeilennummer zum Stand des Abschnitts, der sie trägt.
  Kein Prüfer erhebt darüber Befunde (30.08.2026).
- Der lebende Kopf eines Manifests (Urteilsmarken, Gate-Text, Riegelkarte
  oder Matrix, Kurztabelle der Runden, jüngster Kanon) wird beim Abschluss
  nachgezogen; alles darunter ist append-only Verlauf und wird nie
  umgeschrieben.
- Jede neue Prüfung wurde einmal absichtlich gebrochen; Rohausgabe des Rots
  liegt bei. Eine Probe, die auch ohne den Fix rot ist, heißt Regressionswache,
  nicht Beleg.

- **Geänderte Zusage — drei Stellen, kein Inventar (30.08.2026):** wer eine
  Zusage ändert, zieht Runnerkopf (`tools/beweise.ps1`), Skriptkopf und den
  lebenden Manifestkopf nach; ein `git grep` nach dem Kernbegriff genügt als
  Nachweis im Rundenabschnitt. Die Regel „Aussagen-Inventar" vom 30.08.
  (S8 Runde 10) samt Lebend/Historisch-Klassifizierer ist **zurückgenommen**:
  sie erzeugte in S8 Runde 10–19 ausschließlich Befunde über sich selbst
  (Trefferzahlen, Abschnittsstände, Klassifizierer-Logik) und kein einziges
  Produktverhalten.
- **Writer-Fixtures statt Handschrift (30.08.2026, NAK-94 Nacharbeit 5):** ein
  Probe-Datensatz, der „in der Form des Writers" sein soll, wird vom Writer
  erzeugt (Sandbox, Rohausgabe, Stand, Befehl) und eingefroren; synthetische
  Fälle sind deklarierte Mutanten genau eines Writer-Fixtures mit benannter
  Abweichung. Handgeschriebene „Writer-Formen" fielen in drei Runden an je
  einem weiteren Feld.

## F. Änderungssatz

- speichern↔laden, starten↔stoppen, öffnen↔schließen, verbinden↔trennen,
  aktivieren↔abklingen, installieren↔Rückweg im selben Commit.
- Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher eines
  Vertrags im selben Änderungssatz.
