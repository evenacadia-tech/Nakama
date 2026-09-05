# unit-04 — core/analysis/Vergleichspegel.h (269–404), core/ipc/BrokerLifecycle.cpp, core/ipc/BrokerLifecycle.h, core/ipc/ControlClient.cpp (1–1317)
Gelesen: 107 von 107 Einheiten vollständig · Zeilen: 2518

Alle vier Dateien wurden im Ganzen gelesen (die Teile von Vergleichspegel.h und
ControlClient.cpp ausserhalb meiner Einheiten nur als Kontext). Gegenseiten, die ich zur
Erreichbarkeit mitgelesen habe: `core/ipc/IpcVerbindung.{h,cpp}`,
`core/ipc/WireEnvelope.{h,cpp}`, `src/PluginProcessor.cpp`, `sonde/SondeProcessor.cpp`,
`vertrag/NakamaVertrag.cpp`, `broker/src/vertrag.rs`,
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json`, `docs/offene-punkte.md`.

## Befunde

- **[MEDIUM]** `core/ipc/ControlClient.cpp:1023-1027` · Nebenläufigkeit (verlorene
  Weckung / Hänger) · **Die Prädikatsvariablen des unbefristeten `warte.wait` werden
  ohne `wartemutex` geändert; eine Weckung zwischen Prädikatsprüfung und Parken geht
  verloren, und der Clientthread blockiert für immer.** `stop()` setzt
  `laeuft.store(false)` (`:707`), erhöht die Generation (`:709`) und ruft
  `warte.notify_all()` (`:711`) — alles OHNE `wartemutex`; dasselbe in `reconnect()`
  (`:759`, `:766`). Der Clientthread hält `wartemutex` beim Auswerten des Prädikats
  (`:1024-1027`), ist dabei aber noch nicht als Waiter registriert; trifft die Weckung
  genau dieses Fenster, ist sie nach C++-Regel wirkungslos. Anders als der zweite
  Wartepunkt (`:1034`, `wait_for(backoffMs)`) hat dieser KEINE Frist und heilt deshalb
  nicht. · **Auslöser:** ein lokaler, nicht privilegierter Prozess belegt den
  v3-Pipenamen (Präfix plus Logon-SID) oder antwortet darauf und fällt durch die
  PID-/SID-/Datei-/Hashprüfung; `IpcVerbindung.cpp:98-115` stuft `ERROR_ACCESS_DENIED`
  und jeden unbekannten Code als `belegtAberUnverifiziert` ein, `ControlClient.cpp:1015`
  führt den Lauf damit in JEDER Runde in genau diesen Zweig. Danach genügt ein
  Schliessen der Plugininstanz (`~EqCopilotProcessor` → `controlV3.stop()`), das in das
  Mikrofenster fällt. · **Wirkung:** der Clientthread wacht nie wieder auf (nach `stop()`
  ändert niemand mehr `laeuft` oder die Generation); `stop()` verbraucht die volle Frist
  `kStopFristMs = 2000` (`:729-742`), zählt `stopFristUeberschritten` und **detacht** den
  Thread — der bleibt bis zum Prozessende stehen und hält über seinen eigenen
  `shared_ptr` die ganze `Laufzeit` (Queues, In-Flight-Vektor, Callbacks) am Leben. Pro
  betroffener Instanz ein dauerhaft hängender Thread und ein 2-Sekunden-Stillstand des
  Nachrichtenthreads beim Schliessen. · **Erreichbar:** ja für den Zustand
  (`belegtAberUnverifiziert` ist fremdauslösbar — genau das NAK-123-Szenario, seither
  fail-closed); das Interleaving-Fenster selbst ist schmal (zwischen dem atomaren Lesen
  im Prädikat und dem Parken) und nicht gezielt steuerbar. · **Beleg:**
  `ControlClient.cpp:707-711`, `:759-766`, `:1012-1030`, `:1034`, `:729-742`;
  `IpcVerbindung.cpp:98-115`. · **Register:** neu.

- **[MEDIUM]** `core/ipc/ControlClient.cpp:727-742` · Objektlebenszyklus
  (Use-after-free) · **Der nach der Frist abgelöste Thread berührt zwar den
  `ControlClient` nicht mehr, wohl aber dessen Besitzer: die gespeicherten
  `std::function` fangen den Prozessor, und `stop()` wird aus dessen Destruktor
  gerufen.** Der Kommentar `:724-726` sagt, der abgelöste Thread halte die Laufzeit über
  seinen eigenen `shared_ptr` am Leben und berühre den Client nie — das stimmt für das
  `ControlClient`-Objekt, nicht für `helloProvider`, `statusProvider`, `beiAntwort`,
  `beiLinkStatus` und `beiVersionierterAntwort`, die in `src/PluginProcessor.cpp:106-111`
  bzw. in `sonde/SondeProcessor.cpp` sämtlich `this` des Prozessors fangen. Läuft ein
  solcher Callback noch, wenn die Frist abläuft (`thread.detach()`, `:738`), wird
  unmittelbar danach der Prozessor zu Ende zerstört; der abgelöste Thread arbeitet ab da
  in freigegebenem Speicher weiter. Die zweite `stop()`-Runde aus `~ControlClient()`
  wartet nicht mehr (`thread.joinable() == false`, `:716-722`). · **Auslöser:** ein
  Produktcallback, der beim Schliessen länger als `kStopFristMs = 2000` steht — etwa
  `v3Status()` am `getCallbackLock()` (`sonde/SondeProcessor.cpp:735`) oder am
  `hostKontextSchloss` (`:743`), oder `helloProvider()` an derselben Stelle
  (`ControlClient.cpp:1072`). · **Wirkung:** Use-after-free auf dem Prozessorobjekt im
  Hostprozess. · **Erreichbar:** nicht durch den Angreifer des Bedrohungsmodells — es
  braucht einen hostseitigen Callback-Stillstand über 2 s genau beim Schliessen. Die
  Frist selbst ist der gewollte NAK-95(6)/NAK-104(3)-Fix; was fehlt, ist die Gegenseite
  (die Callbacks über eine Wache entwerten, die der Prozessor VOR seinem Destruktor
  schliesst). · **Beleg:** `ControlClient.cpp:724-742`, `:716-722`, `:1072`;
  `src/PluginProcessor.cpp:106-111`, `:165-178`; `sonde/SondeProcessor.cpp:112-120`,
  `:735-746`. · **Register:** neu (NAK-95 (6) und NAK-104 (3) betreffen die Frist, nicht
  die Lebensdauer des Callback-Ziels).

- **[LOW]** `core/ipc/ControlClient.cpp:988-996` · Logik/Protokoll
  (Reihenfolge-Inversion) · **`meldeLinkStatus` entscheidet atomar, ruft den Callback
  aber ausserhalb des `exchange`; zwei Threads können die Callbacks in umgekehrter
  Reihenfolge zustellen und hinterlassen dem Produkt einen Link, der als verbunden gilt,
  obwohl er abgebaut ist.** Der Clientthread meldet `true` (`:1282`), Nachrichten- bzw.
  Lifecyclethread melden `false` (`stop()` `:708`, `reconnect()` `:767`, gerufen aus
  `BrokerLifecycle::tick` über `hooks.reconnect`). Fällt das `exchange(false)` zwischen
  `exchange(true)` und dessen Callback, wird zuerst `false`, dann `true` zugestellt; das
  Flag steht danach auf `false`, sodass das abschliessende `meldeLinkStatus(false)` am
  Verbindungsende (`:1559`) UNTERDRÜCKT wird. · **Auslöser:** ein Reconnect des
  Brokerlifecycles im selben Moment, in dem der Clientthread eine neu aufgebaute
  Verbindung meldet (flappender Broker, etwa weil ein Fremdprozess den Pipenamen
  abwechselnd belegt). · **Wirkung:** `sourcesModel.beginneSubscription(...)` und
  `interventionsSequenz.store(0)` laufen für einen bereits abgebauten Link
  (`src/PluginProcessor.cpp:1199-1218`); das Produkt zeigt Quellen bis zum nächsten
  erfolgreichen Connect als lebend an, obwohl kein Controllink besteht — genau die
  unehrliche Anzeige, die „keine toten Elemente" ausschliesst. Kein Speicherfehler.
  · **Erreichbar:** ja, aber nur über ein enges Interleaving; heilt beim nächsten
  erfolgreichen Connect. · **Beleg:** `ControlClient.cpp:988-996`, `:1282`, `:708`,
  `:767`, `:1550-1560`; `src/PluginProcessor.cpp:1172-1230`. · **Register:** neu.

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren, Allokationen, IO und Logging — hält.** Keine meiner
   Einheiten liegt auf dem Audiopfad. `operationZaehlen()` (`BrokerLifecycle.cpp:29-33`)
   ist der A-07-Messhaken; alle öffentlichen Einstiege des Lifecycles rufen ihn
   (`:643`, `:651`, `:667`, `:674`, `:697`, `:715`), und `BrokerLifecycle.h:28` hält
   `AUDIO_THREAD_BROKER_OPERATIONEN_MAX = 0` fest. `ControlClient::sendeP0` / `sendeP1`
   (`:799`, `:951`) nehmen Mutexe und allozieren, werden im Produkt aber vom
   Analyseworker (`src/PluginProcessor.cpp:1081`, in `workerLauf`) beziehungsweise vom
   Nachrichtenthread (`:1968`, `:2271`) gerufen. Die Einheiten aus `Vergleichspegel.h`
   (`Steuerzug`, `Abzug`, `uebernehmenIntern`, `leerenIntern`) gehören sämtlich dem
   Steuerthread; der einzige wartende Zug liegt in `Steuerzug` (`:257-268`), und der
   Audiozug (`:242-248`, nicht meine Einheit) gibt bei Misserfolg auf.
2. **Passthrough im Nulltest, 0 Samples Latenz — nicht berührt.** Keine meiner Einheiten
   fasst einen Hostpuffer an.
3. **Gate 5 (kein Telemetrie- oder Steuerframe steuert Gain oder erzeugt ein hörbares
   PRE/POST-Delta) — hält.** Der einzige Wirkweg eines empfangenen Frames in meinen
   Einheiten ist `inFlightAck` (`ControlClient.cpp:919-949`): er löscht Registereinträge
   und zählt, berührt keinen Audioparameter. `welcome` und `reject` (`:422-453`) setzen
   nur Verbindungszustand. Die Deltasperre selbst ist in meinen Einheiten total:
   `deltaZustand` (`Vergleichspegel.h:374-385`) deckt alle acht Eingabekombinationen ab,
   `kCompareroutingInDieserPhase = false` (`:390`) hält P4 gesperrt, und `sperrName`
   (`:392-402`) hat für jeden Enumerator einen Fall plus Rückfall.
4. **NaN-Ehrlichkeit — hält.** `zahl()` (`ControlClient.cpp:367-375`) prüft `isfinite`
   UND den Wertebereich VOR der `long long`-Wandlung (T2-Befund 9); `audioGueltig`
   (`:455-464`) weist eine nicht-endliche Samplerate ab, bevor serialisiert wird;
   `stateHashJson` (`:318-323`) sendet `null` statt eines kaputten Hashes. Die Zählfelder
   `nichtEndlich`, `gesehen` und `bloeckeAufgenommen` (`Vergleichspegel.h:334-345`) sind
   als verriegelnde Zähler deklariert; `leerenIntern` (`:323-332`) setzt sie gemeinsam
   zurück und lässt `fs` und `mindestSamples` bewusst stehen (die setzt `vorbereiten`).
5. **State bleibt verlustfrei — hält.** `commandAckArtLesen`
   (`ControlClient.cpp:152-210`) verlangt bei Erfolg zwingend einen hex64-`state_hash`,
   verbietet `event_uuid`, erzwingt die exakte Feldmenge und akzeptiert `code` nur aus
   der geschlossenen Vertragsliste; `welcomeHaeltVertrag` (`:422-442`) prüft Feldmenge,
   Typ und Länge (`feldmengeGenau` ist exakt, weil `flachesJsonObjekt` doppelte
   Schlüssel bereits ablehnt, `IpcVerbindung.cpp:493-497`).

**Härtungsnotizen ohne heutigen Eingabepfad (deshalb kein Befund):** `runtimeJson`
(`ControlClient.cpp:341-342`) schickt `host_bus_name` ohne eigene Längen- oder
Codepointprüfung auf den Draht, während `label` daneben `utf8CodepointsBis(..., 120)`
durchläuft (`:337`). Das Schema verlangt ausdrücklich, dass mehr als 120
Unicode-Codepoints „nicht auf die Leitung" dürfen
(`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:719-724`). Heute folgenlos, weil der
einzige Produzent vorher filtert (`sonde/SondeProcessor.cpp:41-55`, `:274-282`) und Gen
das Feld gar nicht setzt; der Riegel liegt aber in einer anderen Übersetzungseinheit als
die Sendekante. — `authenticodePruefen` fährt mit `WTD_REVOKE_NONE`
(`BrokerLifecycle.cpp:162`, `:240`): ein widerrufenes Zertifikat käme durch. Heute ohne
Wirkung, weil `authenticode_thumbprint` null ist und der SHA-256-Pin exakt greift;
NAK-119 hält den Punkt für S34–35 fest. — Der PID-Weg in
`namedPipeServerAuthentisieren` (`:478-491`) ist trotz theoretischer PID-Wiederverwendung
nicht täuschbar: Fremd-SID scheitert an `EqualSid` (`:527-529`), gleiche SID ist per
Modell (Entwurf §48.4) ausgeschlossen, und Dateiidentität plus SHA-256 des laufenden
Bildes (`:563-596`) verlangen ohnehin das echte Brokerbinary.

## Ledger

- `core\analysis\Vergleichspegel.h:269-273` · clean
- `core\analysis\Vergleichspegel.h:286-287` · clean
- `core\analysis\Vergleichspegel.h:296-308` · clean
- `core\analysis\Vergleichspegel.h:312-312` · clean
- `core\analysis\Vergleichspegel.h:323-332` · clean
- `core\analysis\Vergleichspegel.h:345-346` · clean
- `core\analysis\Vergleichspegel.h:349-404` · clean
- `core\ipc\BrokerLifecycle.cpp:file-scope-1` · clean
- `core\ipc\BrokerLifecycle.cpp:29-33` · clean
- `core\ipc\BrokerLifecycle.cpp:35-44` · clean
- `core\ipc\BrokerLifecycle.cpp:46-52` · clean
- `core\ipc\BrokerLifecycle.cpp:54-65` · clean
- `core\ipc\BrokerLifecycle.cpp:67-121` · clean
- `core\ipc\BrokerLifecycle.cpp:123-149` · clean
- `core\ipc\BrokerLifecycle.cpp:151-257` · clean
- `core\ipc\BrokerLifecycle.cpp:259-264` · clean
- `core\ipc\BrokerLifecycle.cpp:267-270` · clean
- `core\ipc\BrokerLifecycle.cpp:272-275` · clean
- `core\ipc\BrokerLifecycle.cpp:277-280` · clean
- `core\ipc\BrokerLifecycle.cpp:282-286` · clean
- `core\ipc\BrokerLifecycle.cpp:288-338` · clean
- `core\ipc\BrokerLifecycle.cpp:345-345` · clean
- `core\ipc\BrokerLifecycle.cpp:346-350` · clean
- `core\ipc\BrokerLifecycle.cpp:351-351` · clean
- `core\ipc\BrokerLifecycle.cpp:352-352` · clean
- `core\ipc\BrokerLifecycle.cpp:355-379` · clean
- `core\ipc\BrokerLifecycle.cpp:381-389` · clean
- `core\ipc\BrokerLifecycle.cpp:391-400` · clean
- `core\ipc\BrokerLifecycle.cpp:403-416` · clean
- `core\ipc\BrokerLifecycle.cpp:418-430` · clean
- `core\ipc\BrokerLifecycle.cpp:432-461` · clean
- `core\ipc\BrokerLifecycle.cpp:463-602` · clean
- `core\ipc\BrokerLifecycle.cpp:604-629` · clean
- `core\ipc\BrokerLifecycle.cpp:631-634` · clean
- `core\ipc\BrokerLifecycle.cpp:636-639` · clean
- `core\ipc\BrokerLifecycle.cpp:641-647` · clean
- `core\ipc\BrokerLifecycle.cpp:649-663` · clean
- `core\ipc\BrokerLifecycle.cpp:665-670` · clean
- `core\ipc\BrokerLifecycle.cpp:672-676` · clean
- `core\ipc\BrokerLifecycle.cpp:678-691` · clean
- `core\ipc\BrokerLifecycle.cpp:693-709` · clean
- `core\ipc\BrokerLifecycle.cpp:711-720` · clean
- `core\ipc\BrokerLifecycle.cpp:722-871` · clean
- `core\ipc\BrokerLifecycle.cpp:872-939` · clean
- `core\ipc\BrokerLifecycle.cpp:941-941` · clean
- `core\ipc\BrokerLifecycle.cpp:942-942` · clean
- `core\ipc\BrokerLifecycle.cpp:943-943` · clean
- `core\ipc\BrokerLifecycle.cpp:944-947` · clean
- `core\ipc\BrokerLifecycle.h:file-scope-1` · clean
- `core\ipc\BrokerLifecycle.h:109-155` · clean
- `core\ipc\ControlClient.cpp:file-scope-1` · clean
- `core\ipc\ControlClient.cpp:47-55` · clean
- `core\ipc\ControlClient.cpp:57-67` · clean
- `core\ipc\ControlClient.cpp:69-77` · clean
- `core\ipc\ControlClient.cpp:83-112` · clean
- `core\ipc\ControlClient.cpp:114-150` · clean
- `core\ipc\ControlClient.cpp:152-210` · clean
- `core\ipc\ControlClient.cpp:212-224` · clean
- `core\ipc\ControlClient.cpp:226-258` · clean
- `core\ipc\ControlClient.cpp:260-292` · clean
- `core\ipc\ControlClient.cpp:294-297` · clean
- `core\ipc\ControlClient.cpp:299-299` · clean
- `core\ipc\ControlClient.cpp:301-316` · clean
- `core\ipc\ControlClient.cpp:318-323` · clean
- `core\ipc\ControlClient.cpp:325-348` · clean
- `core\ipc\ControlClient.cpp:350-360` · clean
- `core\ipc\ControlClient.cpp:367-375` · clean
- `core\ipc\ControlClient.cpp:378-389` · clean
- `core\ipc\ControlClient.cpp:391-412` · clean
- `core\ipc\ControlClient.cpp:422-442` · clean
- `core\ipc\ControlClient.cpp:446-453` · clean
- `core\ipc\ControlClient.cpp:455-464` · clean
- `core\ipc\ControlClient.cpp:466-474` · clean
- `core\ipc\ControlClient.cpp:476-483` · clean
- `core\ipc\ControlClient.cpp:485-490` · clean
- `core\ipc\ControlClient.cpp:492-509` · clean
- `core\ipc\ControlClient.cpp:511-515` · clean
- `core\ipc\ControlClient.cpp:517-523` · clean
- `core\ipc\ControlClient.cpp:532-542` · clean
- `core\ipc\ControlClient.cpp:552-553` · clean
- `core\ipc\ControlClient.cpp:593-597` · clean
- `core\ipc\ControlClient.cpp:599-605` · clean
- `core\ipc\ControlClient.cpp:file-scope-2` · clean
- `core\ipc\ControlClient.cpp:655-671` · clean
- `core\ipc\ControlClient.cpp:673-676` · clean
- `core\ipc\ControlClient.cpp:678-693` · clean
- `core\ipc\ControlClient.cpp:695-750` · Befund 2
- `core\ipc\ControlClient.cpp:752-768` · clean
- `core\ipc\ControlClient.cpp:770-770` · clean
- `core\ipc\ControlClient.cpp:772-775` · clean
- `core\ipc\ControlClient.cpp:777-777` · clean
- `core\ipc\ControlClient.cpp:779-782` · clean
- `core\ipc\ControlClient.cpp:784-787` · clean
- `core\ipc\ControlClient.cpp:789-792` · clean
- `core\ipc\ControlClient.cpp:794-797` · clean
- `core\ipc\ControlClient.cpp:799-833` · clean
- `core\ipc\ControlClient.cpp:835-879` · clean
- `core\ipc\ControlClient.cpp:881-904` · clean
- `core\ipc\ControlClient.cpp:906-917` · clean
- `core\ipc\ControlClient.cpp:919-949` · clean
- `core\ipc\ControlClient.cpp:951-970` · clean
- `core\ipc\ControlClient.cpp:972-976` · clean
- `core\ipc\ControlClient.cpp:978-986` · clean
- `core\ipc\ControlClient.cpp:988-996` · Befund 3
- `core\ipc\ControlClient.cpp:998-1051` · Befund 1
- `core\ipc\ControlClient.cpp:1053-1194` · Befund 2 (Aufrufstelle der Fremdcallbacks)
- `core\ipc\ControlClient.cpp:1195-1317` · clean

## Nicht gelesen

keine

## Ausserhalb meiner Einheiten

- `core/ipc/TelemetryClient.cpp:439` - derselbe unbefristete `warte.wait` wie in Befund 1,
  mit denselben Weckungen ohne `wartemutex` (`:298`, `:355`); unbewertet.
- `core/ipc/IpcVerbindung.cpp:475-497` - der flache JSON-Leser bricht bei Verschachtelung
  und bei doppelten Schluesseln ab; darauf ruht die Exaktheit meiner Vertragspruefer;
  unbewertet.
- `core/ipc/WireEnvelope.cpp:221-236` - `StromLeser::fuettern` hat keine eigene
  Obergrenze; begrenzt wird der Puffer erst dadurch, dass `envelopePruefen` ein
  `frame_len` ueber `kMaxFrameBytes` als Verstoss meldet (`:103-104`); unbewertet.
- `src/PluginProcessor.cpp:1199-1209` - `interventionsSequenz.store(0)` im Link-Callback
  ist die Produktwirkung, an der Befund 3 sichtbar wird; unbewertet.
