# unit-05 — core/ipc/ControlClient.cpp (1318-1562), ControlClient.h, InterventionsRing.h, IpcQueues.h, IpcVerbindung.cpp, IpcVerbindung.h, PipeToken.cpp (1-126)
Gelesen: 45 von 45 Einheiten vollständig · Zeilen: 2474

HEAD 9f3000881df632f4969d78551fb0a516b61288cb, strikt lesend.
Hinweis zur Schreibweise: die Ledger-Kennungen unten benutzen Schrägstriche
statt der Backslashes aus `unit-05.json`; die Zuordnung ist 1:1
(`core/ipc/X` ⇔ `core\ipc\X`).

## Befunde

- **[HIGH]** `core/ipc/IpcVerbindung.cpp:27` · banned-api / caller-contract ·
  **`namedPipeErreichbar` blockiert unbegrenzt lange, weil `WaitNamedPipeW`
  mit Zeitlimit 0 die Frist vom SERVER nimmt, nicht von uns.** ·
  Auslöser: ein lokaler, nicht privilegierter Prozess legt den Pipenamen an
  (`\\.\pipe\`-Namen darf jedes Konto erzeugen; der Name ist
  `kPipePraefixV3 + base32(sha256(domäne+LogonSID))`, `PipeToken.cpp:128-145`,
  und eine fremde Logon-SID ist kein Geheimnis) — mit `nMaxInstances = 1`,
  `nDefaultTimeOut = 0xFFFFFFFE` — und hält die eine Instanz mit einem eigenen
  Client belegt. `nTimeOut = 0` ist `NMPWAIT_USE_DEFAULT_WAIT`: „the time-out
  interval is the default value specified by the server process in the
  CreateNamedPipe function"; unser eigener Broker setzt dort 0 und bekommt
  damit 50 ms (`broker/src/transport/server_v3/win_handles.rs:76`,
  `broker/src/server.rs:511`) — aber genau diese Funktion läuft, solange noch
  NICHT feststeht, wem der Name gehört. ·
  Wirkung: der Aufruf steht bis zu ~49,7 Tagen. Der Thread hält dabei bereits
  das Win32-Startmutex (`BrokerLifecycle.cpp:865`), also zählt jede weitere
  Plugininstanz `mutexVerloren` und startet nie einen Broker; und
  `BrokerLifecycle::stop()` joint den Thread OHNE Frist
  (`BrokerLifecycle.cpp:660-661`) und ist die ERSTE Anweisung von
  `~EqCopilotProcessor()` (`src/PluginProcessor.cpp:167`) — Plugin entfernen,
  Projekt schliessen oder FL beenden hängt den Message-Thread des Hosts für
  die vom Angreifer gewählte Dauer. Es gibt keinen Gegenweg: `WaitNamedPipeW`
  ist nicht über `CancelIoEx` oder das `abbruch`-Flag abbrechbar. ·
  Erreichbar: ja. Aufruferkette
  `BrokerLifecycle::threadLauf` (`BrokerLifecycle.cpp:678-686`, 25-ms-Takt) →
  `tick` → `vorhandenePipeUebernehmen` (`:783-785`, gerufen `:880` und `:911`).
  Der Zweig wird erreicht, wenn `brokerFehlt` gilt (`:858-862`, d. h. der
  ControlClient hat zuletzt `ServerPruefStatus::nichtDa` gemeldet) und
  `hooks.darfStarten()` true ist. Das Rennen liegt beim Angreifer: er legt die
  belegte Pipe an, NACHDEM das Plugin `nichtDa` gemeldet hat; der ControlClient
  kann diesen Status frühestens nach seinem Backoff (500–8000 ms,
  `IpcVerbindung.h:34-35`) aktualisieren, der Lebenslauf tickt alle 25 ms. ·
  Beleg: `IpcVerbindung.cpp:19-31` (ganze Funktion), `IpcVerbindung.h:147-150`
  (die Kopfzeile nimmt „Nullzeitlimit" fälschlich als „nicht warten" an),
  `BrokerLifecycle.cpp:783-785/858-881/649-663`, `PluginProcessor.cpp:165-167`.
  Gegenprobe: die zweite `WaitNamedPipeW`-Stelle im selben File
  (`IpcVerbindung.cpp:96`) übergibt explizit 200 ms und ist auf 20 Runden
  gedeckelt — dort tritt der Fall nicht auf. ·
  Register: **neu**. NAK-134 betrifft genau die andere Stelle
  (`WaitNamedPipeW(…, 200)` in `oeffnen`) und die PIPE_BUSY-Klassifikation;
  NAK-123 hat die *Adoption* eines fremden Servers geschlossen
  (`namedPipeServerAuthentisieren`), nicht das *Blockieren* vor jeder
  Authentisierung. `namedPipeErreichbar` steht in keiner Registerzeile.

- **[MEDIUM]** `core/ipc/ControlClient.cpp:1412-1414` · return-values ·
  **Der State-Report-Sender wirft das Ergebnis von `sendeP1` weg und merkt
  sich die Nachricht trotzdem als gesendet.** ·
  Auslöser: `sendeP1` liefert `P1Ergebnis::abgewiesen` (Hauptqueue 128 UND
  Wiederholpuffer 128 voll, `IpcQueues.h:220-226`) oder `zuGross`
  (`ControlClient.cpp:954-959`). Erreichbar über eine längere Trennung, in der
  der Evidenzsender weiter einreiht (`sonde/SondeProcessor.cpp:563`, leerer
  Koaleszenzschlüssel ⇒ Ereignisse, die NIE koaleszieren) — z. B. weil ein
  Namensbesetzer oder ein toter Broker den Connect minutenlang scheitern lässt;
  beim ersten Heartbeat-Takt der neuen Verbindung ist die Queue voll und der
  State-Report findet keinen Koaleszenzpartner. ·
  Wirkung: `letzterStateReport = report` (`:1413`) wird UNBEDINGT gesetzt, also
  wird derselbe Report nie wieder gesendet, solange sich der Produktstatus
  nicht ändert — der Broker hält für den Rest dieser Verbindung einen veralteten
  `state_report` (`state_revision`, `state_hash`, `recording`,
  `interventionStateUnknown`). Still ist es ausserdem:
  `P1Warteschlange::abgewiesene()` (`IpcQueues.h:319`) existiert, wird aber
  nirgends in `Snapshot` gehoben, und der Rückgabewert ist der einzige weitere
  Träger. Genau dagegen ist `p1Uebergeben` geschrieben worden
  (`IpcQueues.h:136-153`: „der Evidenzsender leerte seinen Ereignisring
  bedingungslos … ein stiller Verlust ohne Zähler"); der Evidenzsender hält die
  Regel (`SondeProcessor.cpp:567-578`), dieser Sender als einziger nicht. ·
  Erreichbar: ja, Aufruferkette `Laufzeit::threadLauf` → `eineVerbindung`
  (Client-Thread) → `:1403-1414`. Ehrliche Schranke: `letzterStateReport` ist
  eine Lokale von `eineVerbindung` (`:1311`), der Verlust heilt also beim
  nächsten Reconnect, und das sticky `intervention_state_unknown` reist
  zusätzlich im P0-Heartbeat (`ControlClient.h:150-155`). ·
  Beleg: `ControlClient.cpp:1409-1414`, `IpcQueues.h:148-153`, `:220-226`,
  `:319`, `SondeProcessor.cpp:563-578`. · Register: **neu**.

- **[LOW]** `core/ipc/IpcVerbindung.cpp:77-78` · initialisation ·
  **Der Abbruchausgang von `oeffnen` schreibt den Ausgabeparameter `fehler`
  nicht, der Aufrufer liest ihn trotzdem und löscht damit den sichtbaren
  Grund.** ·
  Auslöser: `abbruch` wird nicht nur von `stop()`/`reconnect()` gesetzt (die
  zusätzlich die Generation erhöhen und damit `veralteteGeneration` auslösen),
  sondern auch von `sendeP0`/`sendePersistenzP0` bei P0-Überlauf
  (`ControlClient.cpp:829`, `:875` → `ioAbbrechen()`), und dieser Weg erhöht
  die Generation NICHT. Fällt er in die Öffnungsschleife, kehrt `oeffnen` bei
  `:77-78` zurück, ohne `fehler` zu berühren; `bericht` ist durch `:59` korrekt
  auf `{}` gesetzt. ·
  Wirkung: der Aufrufer schreibt unbedingt `zustand.letzterFehler = fehler;`
  (`ControlClient.cpp:1142`, dieselbe Form `TelemetryClient.cpp:577 ff.`), also
  steht im Snapshot „getrennt" mit LEERER Begründung, und eine vorher
  gemeldete, richtige Ursache ist überschrieben. Das ist der Bruch von „jedes
  sichtbare Element … meldet ehrlich einen Zustand". ·
  Erreichbar: ja, aber nur als Sichtbarkeits-/Ehrlichkeitsfehler; kein
  Speicher- oder Zustandsschaden. Aufruferkette `eineVerbindung:1123` →
  `oeffnen:77`. · Beleg: `IpcVerbindung.cpp:59`, `:77-78`,
  `ControlClient.cpp:1135-1143`, `:829`. · Register: **neu**.

- **[LOW]** `core/ipc/ControlClient.cpp:1470` · caller-contract ·
  **`Snapshot::p1Wiederholungen` trägt einen Füllstand, obwohl Name und
  Nachbarschaft einen kumulativen Zähler versprechen.** ·
  Auslöser: jede Zuweisung. Beide Schreibstellen setzen das Feld auf
  `p1.wiederholungen()`, also `wiederholung.size()` (`IpcQueues.h:315`) — die
  eine in meiner Einheit (`:1470`), die zweite ausserhalb (`:966`). In der
  Zeile darauf steht derselbe Wert nochmals in `p1WiederholTiefe` (`:1472`,
  `:968`). ·
  Wirkung: das Feld steht in `ControlClient.h:195-199` zwischen lauter
  monoton wachsenden Zählern (`p0Gesendet`, `p1Gesendet`, `empfangen`,
  `p0Ueberlaeufe`) und ist dort das einzige, das wieder auf 0 fällt. Ein
  Auswerter, der es als „wie oft musste wiederholt werden" liest, bekommt eine
  Momentanaufnahme; die echte Zahl der Wiederholungen misst heute niemand.
  Doppelt geführt ist es ausserdem. ·
  Erreichbar: kein Angreiferpfad — reine Metrik-Ehrlichkeit, kein
  Speicherfehler; im Produkt liest das Feld heute niemand
  (`grep p1Wiederholungen` findet nur die zwei Zuweisungen und die
  Deklaration). · Beleg: `ControlClient.h:195-212`,
  `ControlClient.cpp:1467-1472`, `IpcQueues.h:312-319`. · Register: **neu**.

## Gate-5- und Invariantenbeleg

1. **Audiothread ohne Sperren, Allokationen, Datei-/Pipe-/Netzzugriff und
   Logging — HÄLT.** Der einzige Audiothread-Pfad in meinen Einheiten ist
   `InterventionsRing::schreibe` (`InterventionsRing.h:123-137`): fester
   `std::array<Interventionsereignis,32>`, zwei Atomics, POD-Zuweisung, kein
   Lock, keine Allokation, kein Logging; der Überlauf verwirft ein
   ANALYSE-/Meldeereignis und meldet es sticky (`:129-132`), nie Audio.
   Kapazität ist Zweierpotenz, der Index ist eine Maske (`:134`, `:146`), also
   auch ohne Division. `P2Schleuse::veroeffentlichen` (`IpcQueues.h:455-580`)
   mit seiner CAS-Schleife läuft NICHT im Audiothread, sondern im
   Analyseworker (`sonde/SondeProcessor.cpp:396`, dort wird allokiert und
   `v3Hello()` gerufen); `P0-/P1Warteschlange` und beide IPC-Dateien laufen
   ausschliesslich auf Client-, Lebenslauf- oder Nachrichtenthread.
2. **Passthrough bitidentisch / 0 Samples Latenz — NICHT BERÜHRT.** Keine
   meiner Einheiten fasst einen Hostpuffer, ein Gain oder einen Samplezähler an;
   `HoerMarkierung.h` liegt ausserhalb.
3. **Gate 5 (kein Telemetrie-/Steuerframe steuert samplegenauen Gain) — HÄLT.**
   Der Empfangspfad `empfangenes` (`ControlClient.cpp:1318-1397`) übergibt die
   Payload ausschliesslich als `std::string` an `beiVersionierterAntwort` bzw.
   `beiAntwort` auf dem Client-Thread (`:1385-1395`), nach Familien- und
   Kodierungssperre (`:1348-1356`, nur P0/P1 und nur JSON), Schema-Minor-Sperre
   (`:1357-1364`) und Ratengrenze VOR dem Callback (`:1369-1379`). Kein Pfad in
   meinen Einheiten schreibt Audio oder Gain. Der `InterventionsRing` läuft
   ausdrücklich nur in EINE Richtung (RT → Control, `InterventionsRing.h:36-41`).
4. **NaN-Ehrlichkeit — NICHT BERÜHRT.** In meinen Einheiten gibt es keine
   einzige Fliesskomma-Rechnung und keine float→int-Wandlung. Die Verriegelung
   der Audiofelder passiert bei `ControlClient.cpp:1098` (`audioGueltig`) VOR
   der Serialisierung und liegt ausserhalb meiner Zeilen.
5. **State bleibt verlustfrei — NICHT BERÜHRT.** Kein State-Blob-, Migrations-
   oder Matrixcode in meinen Einheiten.

Zusätzlich geprüft und tragend (nicht Teil der fünf, aber Kern dieser Einheiten):

- **P0-Zusage „nichts verwerfen" hält über den Weg Queue↔Pipe.** Die
  Reservierung (`IpcQueues.h:61-101`) und die Aufrufstelle passen zusammen: nach
  `entnehmen` folgt auf JEDEM Pfad genau ein `bestaetigen()`
  (`ControlClient.cpp:1499`) oder ein `zuruecklegen()` (`:1462`), dazwischen
  liegt kein Rücksprung. `nachricht` ist nach dem `std::move` in `zuruecklegen`
  nicht mehr gelesen (`:1462-1492` geprüft); auf dem Erfolgspfad ist sie
  unversehrt und wird bei `:1505` noch gebraucht.
- **Sperrordnung ist durchgehend `sendeMutex` → `zustandMutex`** (`:1460/:1469`,
  `:851/:869`, `:886/:900`, `:929/:942`, `:961/:965`); die Gegenrichtung kommt
  nirgends vor, also kein Deadlock.
- **Kein `schliessen()` läuft neben laufender I/O.** `stop()` schliesst erst
  NACH `thread.join()` (`ControlClient.cpp:743-744`) bzw. wenn der Thread nie
  lief (`:716-718`); nach dem Ablösen (`:738`) schliesst der abgelöste Thread
  seine EIGENE Verbindung (`:1549`). Damit ist die naheliegende
  Handle-nach-CloseHandle-Wiederverwendung in `lesen`/`schreibenGenau`
  (`IpcVerbindung.cpp:285`, `:367`) nicht erreichbar. Von aussen kommt nur
  `ioAbbrechen()` (`:710`, `:765`, `:829`, `:875`), und das schliesst kein
  Handle.
- **`InterventionsRing::zuruecksetzen()`** (`InterventionsRing.h:177-183`)
  würde `kopf`/`schwanz` unter dem Audiothread-Produzenten wegziehen und den
  Ring dauerhaft in den Überlauf treiben. Im Produkt ruft es niemand — nur
  `resync()` (`PluginProcessor.cpp:1201`), das ausschliesslich das Sticky-Bit
  löscht. Nicht erreichbar, deshalb kein Befund; beim nächsten Anfassen des
  Rings ist es die Zeile, die man nicht ins Produkt ziehen darf.
- **SHA-256/Base32 in `PipeToken.cpp` sind korrekt und grenzenfest.**
  `restLaenge ∈ [0,63]` durch die Schleifenbedingung (`:77-81`), `rest[128]`
  nullinitialisiert, `rest[restLaenge] = 0x80` also immer im Feld, `gesamt`
  wählt 64 nur für `restLaenge ≤ 55`, Längenfeld und 0x80 kollidieren nie
  (`:84-89`); `block` liest genau 64 Byte und bekommt sie an allen drei
  Aufrufstellen (`:78`, `:90`, `:92`). `rotr` (`:33-36`) hätte bei `n == 0`
  UB — alle zehn Aufrufstellen liegen in `[2,25]`, und die Funktion ist im
  anonymen Namensraum, also gibt es keinen weiteren Aufrufer. In `base32`
  bleibt `bits ∈ [0,4]` nach der Schleife, `5 - bits ∈ [1,4]`
  (`:113-123`); `kAlphabet` hat 32 Einträge, der Index ist maskiert.
- **Der JSON-Leser terminiert und rät nicht.** `flachesJsonObjekt`
  (`IpcVerbindung.cpp:440-515`) prüft jeden Zugriff gegen `text.size()`, lehnt
  Verschachtelung, Escapes und Steuerzeichen ab (`:478-481`, `:427-429`),
  weist doppelte Schlüssel zurück (`:493-496`) und macht je Runde mindestens
  ein Zeichen Fortschritt. `feldmengeGenau` (`:548-559`) ist dank dieser
  Doppelschlüsselsperre exakt.
- **Der Envelope-Empfang läuft nicht endlos.** `StromLeser::naechster`
  verbraucht bei einem gültigen Frame `drahtlaenge > 0`
  (`WireEnvelope.cpp:249`), die `for(;;)`-Schleife bei
  `ControlClient.cpp:1331-1396` endet also spätestens bei `unvollstaendig`; die
  Ratengrenze deckelt zusätzlich auf 4000 Frames/s.
- **`P2Schleuse` ist frei von Datenrennen.** Beide Seiten nehmen den Slot per
  `compare_exchange_strong(acq_rel)` und geben ihn mit `store(release)` frei
  (`IpcQueues.h:530`, `:577`, `:602`, `:620`); `bytes`/`laenge` werden nur unter
  Besitz berührt, `laenge > SlotBytes` wird an der Tür abgewiesen (`:457-461`),
  `laenge <= zielGroesse` vor dem Kopieren geprüft (`:617`), und das Ziel des
  einzigen Produktverbrauchers ist genau `slotGroesse()`
  (`TelemetryClient.cpp:743`), also greift die Prüfung nie stillschweigend.
  `p - c` unterläuft nicht: `schreib` bewegt allein der Erzeuger, `verbraucht`
  und `boden` bleiben `<= p`.

## Ledger

- core/ipc/ControlClient.cpp:1318-1399 · clean
- core/ipc/ControlClient.cpp:1400-1549 · Befund 2 (MEDIUM, :1412), Befund 4 (LOW, :1470)
- core/ipc/ControlClient.cpp:1550-1562 · clean
- core/ipc/ControlClient.h:file-scope-1 · clean
- core/ipc/ControlClient.h:33-174 · clean
- core/ipc/ControlClient.h:175-319 · clean (Feldbefund 4 ist an der Schreibstelle geführt)
- core/ipc/InterventionsRing.h:file-scope-1 · clean
- core/ipc/InterventionsRing.h:50-193 · clean
- core/ipc/IpcQueues.h:file-scope-1 · clean
- core/ipc/IpcQueues.h:31-116 · clean
- core/ipc/IpcQueues.h:117-266 · clean
- core/ipc/IpcQueues.h:267-369 · clean
- core/ipc/IpcQueues.h:370-450 · clean
- core/ipc/IpcQueues.h:455-580 · clean
- core/ipc/IpcQueues.h:671-678 · clean
- core/ipc/IpcQueues.h:680-684 · clean
- core/ipc/IpcQueues.h:705-708 · clean
- core/ipc/IpcQueues.h:file-scope-2 · clean
- core/ipc/IpcQueues.h:733-733 · clean
- core/ipc/IpcVerbindung.cpp:file-scope-1 · clean
- core/ipc/IpcVerbindung.cpp:19-31 · Befund 1 (HIGH, :27)
- core/ipc/IpcVerbindung.cpp:33-36 · clean
- core/ipc/IpcVerbindung.cpp:38-42 · clean
- core/ipc/IpcVerbindung.cpp:44-176 · Befund 3 (LOW, :77-78)
- core/ipc/IpcVerbindung.cpp:178-195 · clean
- core/ipc/IpcVerbindung.cpp:197-203 · clean
- core/ipc/IpcVerbindung.cpp:205-211 · clean
- core/ipc/IpcVerbindung.cpp:213-309 · clean
- core/ipc/IpcVerbindung.cpp:311-392 · clean
- core/ipc/IpcVerbindung.cpp:394-407 · clean
- core/ipc/IpcVerbindung.cpp:411-415 · clean
- core/ipc/IpcVerbindung.cpp:419-437 · clean
- core/ipc/IpcVerbindung.cpp:440-515 · clean
- core/ipc/IpcVerbindung.cpp:519-525 · clean
- core/ipc/IpcVerbindung.cpp:528-536 · clean
- core/ipc/IpcVerbindung.cpp:538-546 · clean
- core/ipc/IpcVerbindung.cpp:548-559 · clean
- core/ipc/IpcVerbindung.h:file-scope-1 · clean
- core/ipc/IpcVerbindung.h:file-scope-2 · clean
- core/ipc/IpcVerbindung.h:161-275 · clean
- core/ipc/PipeToken.cpp:file-scope-1 · clean
- core/ipc/PipeToken.cpp:33-36 · clean
- core/ipc/PipeToken.cpp:38-67 · clean
- core/ipc/PipeToken.cpp:72-101 · clean
- core/ipc/PipeToken.cpp:103-126 · clean

## Nicht gelesen

keine

## Außerhalb meiner Einheiten

- `core/ipc/PipeToken.cpp:176` — `sid.push_back (static_cast<char> (*p & 0x7F))`
  maskiert jedes Zeichen der `ConvertSidToStringSidW`-Ausgabe auf 7 Bit; zwei
  verschiedene Wide-Zeichen können damit auf dasselbe ASCII-Zeichen fallen.
- `core/ipc/ControlClient.cpp:966-968` — dieselbe Doppelbelegung von
  `p1Wiederholungen`/`p1WiederholTiefe` wie Befund 4, an der zweiten
  Schreibstelle.
- `core/ipc/TelemetryClient.cpp:577` ff. — der Telemetriepfad liest den
  `fehler`-Ausgabeparameter von `oeffnen` in derselben Form wie
  `ControlClient.cpp:1142` (Befund 3).
- `core/ipc/BrokerLifecycle.cpp:660-661` und `src/PluginProcessor.cpp:167` —
  `stop()` joint den Lebenslaufthread ohne Frist und steht als erste Anweisung
  im Destruktor des Prozessors (Wirkungspfad von Befund 1).
- `core/ipc/WireEnvelope.cpp:228-235` — `StromLeser::fuettern` verdichtet den
  Puffer erst ab 65536 verbrauchten Bytes.
