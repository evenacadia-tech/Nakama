# unit-12 — src/SourcesModel.cpp, src/SourcesModel.h, src/WorkerCadence.h, src/ZonenRegeln.h, state/NakamaKanon.cpp, state/NakamaKanon.h, state/NakamaKernRiegel.h, state/NakamaLebenslauf.cpp, state/NakamaLebenslauf.h
Gelesen: 76 von 76 Einheiten vollständig · Zeilen: 2498 (alle neun Dateien ganz gelesen: 2641 Quellzeilen, 2498 davon in Einheiten)

Pfade sind repo-relativ ab `eq-copilot/plugin/`. Die Einheitenkennungen der
Zuweisung tragen Backslashes; hier stehen überall Schrägstriche, sonst
unverändert. HEAD beim Lesen: `9f30008`, Worktree unverändert.

## Befunde

- **[MEDIUM] F1 · src/SourcesModel.cpp:530-574, 575-612 · logik/protokoll (fehlende Vertragsprüfung)** ·
  `experimente` und `paare` werden nur auf `isString()` geprüft; die geschlossenen
  Enum-Mengen und die Längengrenze aus `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`
  (`$defs/session_experiment`, `$defs/session_paar`, `$defs/pair_id` mit `maxLength: 64`)
  werden nicht durchgesetzt. ·
  **Auslöser:** ein sonst gültiger `session_snapshot` mit
  `"experimente":[{"experiment_id":"<32 hex>","ereignis":"quatsch","offen":true,"vergleichbarkeit":"stark","urteil":"objektiv besser"}]`
  bzw. `"paare":[{"pair_id":"<250 KiB Text>","klasse":"erfunden","kettenbefund":"stationaer"}]`. ·
  **Wirkung:** der Snapshot wird `uebernommen` statt `ungueltig`. Das Modell hält ein
  `urteil` außerhalb der fünf Aussagen aus M-46 — genau den Wert, den der
  Schemakommentar ausdrücklich ausschließt („`objektiv besser` ist keine davon") —,
  eine `vergleichbarkeit`, die nicht gerechnet wurde, und einen `kettenbefund`
  außerhalb der fünf zulässigen. Damit ist die Zusage in `src/SourcesModel.h:93`
  („Eine der fuenf zulaessigen Aussagen aus M-46, oder leer") heute unwahr. Ein
  Anzeigekonsument existiert noch nicht (`Sicht::experimente`/`Sicht::paare` werden
  nur in `sicht()` gefüllt, `src/SourcesModel.cpp:1064-1065`; `src/PluginEditor.cpp`
  liest sie nicht) — der Defekt ist der Vertragsriegel selbst, nicht ein Pixel, und
  er kippt in dem Moment zur Falschaussage auf dem Schirm, in dem die Fläche gebaut wird. ·
  **Erreichbar:** ja. Kette: Broker/Control-Pipe → `nakama::ipc::ControlClient` →
  `EqCopilotProcessor::v3Antwort` (`src/PluginProcessor.cpp:1266`) →
  `SourcesModel::uebernehmeSessionSnapshot`. ·
  **Beleg:** `:550-573` und `:594-609` gegen die Nachbarfelder derselben Funktion, die
  jeweils eine geschlossene Menge erzwingen: `plugin_kind` `:652-654`,
  `aussageklasse` `:200`, `betrieb` `:206-210`, `p2_reject.grund` `:247-258`,
  `measurement_position` `:135-141`. `liesDescriptor` prüft `pair_id` bei `:217-224`
  auf `<= 64` — im selben Snapshot, für dasselbe Feld. ·
  **Register:** neu.

- **[MEDIUM] F2 · src/SourcesModel.cpp:607-609 · logik/protokoll (fail-open Semantik)** ·
  Ein `ausschluss`, der vorhanden aber kein String ist, wird still verworfen; leer
  bedeutet laut `src/SourcesModel.h:103` „das Paar traegt eine Aussage". ·
  **Auslöser:** `"paare":[{"pair_id":"p1","klasse":"audio_aligned","kettenbefund":"nicht_beurteilbar","ausschluss":0}]`
  — `exakteFelder` (`:588-589`) lässt `ausschluss` als optionales Feld durch, `:608`
  prüft nur `isString()`. ·
  **Wirkung:** aus „dieses Paar trägt KEINE Aussage" wird „es trägt eine". Das ist die
  Umkehrung der fail-closed Richtung, in derselben Funktion, die jedes andere
  Typvergehen mit `SnapshotErgebnis::ungueltig` quittiert. ·
  **Erreichbar:** ja, dieselbe Kette wie F1. ·
  **Beleg:** `:607-609` gegen `:597-602` (dieselbe Schleife lehnt einen nicht-String
  `pair_id`/`klasse`/`kettenbefund` ab) und gegen `src/SourcesModel.h:103-104`. ·
  **Register:** neu (Sonderfall von F1 mit eigener, entgegengesetzter Wirkung).

- **[MEDIUM] F3 · src/SourcesModel.cpp:429-431 · logik/zustandsautomat** ·
  `Diagnose::incompatible` wird vom nächsten Transport-Tick gelöscht, obwohl der
  Gegenpfad `controlEnde()` genau diesen Wert ausdrücklich schützt. ·
  **Auslöser:** (1) ein gültiger `session_snapshot` setzt `subscriptionAktiv = true`
  (`:787`); (2) der Broker sendet einen vertragswidrigen `session_snapshot`, z. B. mit
  einem unbekannten Wurzelfeld → `:488-492` liefert `ungueltig`, und
  `EqCopilotProcessor::v3Antwort` setzt `setzeDiagnoseFuerSichtbeweis(incompatible, false)`
  (`src/PluginProcessor.cpp:1268-1270`); `subscriptionAktiv` bleibt dabei stehen;
  (3) der nächste `sourcesTick()` aus `EqCopilotEditor::timerCallback`
  (`src/PluginEditor.cpp:230` `startTimerHz (30)`, `:247`) ruft
  `setzeControlTransport` mit `status == verbunden`. Der dritte Zweig `:429-431`
  schließt nur `confirmationRequired` und `storeDegraded` aus und setzt
  `Diagnose::keine`. ·
  **Wirkung:** die einzige ehrliche Meldung „Incompatible broker or protocol" lebt
  höchstens ~33 ms. `src/PluginEditor.cpp:1200-1214` zeichnet das Diagnoseband nur bei
  `diagnose != keine`, also verschwindet es im nächsten Frame; die Quellzeilen zeigen
  weiter den letzten gültigen, jetzt veralteten Stand, ohne dass der User erfährt, dass
  die Gegenseite vertragswidrige Rahmen schickt. Bei wiederholt ungültigen Rahmen
  flackert nichts Sichtbares — es bleibt dauerhaft still. ·
  **Erreichbar:** ja, deterministisch. `controlEnde()` (`:396-402`) bewahrt
  `incompatible`, `storeDegraded` und `serverUnverified` ausdrücklich — die beiden
  Pfade widersprechen sich, und der Widerspruch ist ungetestet:
  `tests/Sonde012SourcesModelTest.cpp:417-432` erzeugt `incompatible` ausschließlich
  über `status == getrennt` und prüft nie den Übergang zurück auf `verbunden` bei
  aktiver Subscription. ·
  **Register:** neu.

- **[LOW] F4 · state/NakamaKanon.cpp:232-241 · initialisation** ·
  `kanonisiere` gibt im Fehlerfall N Bytes **uninitialisierten Heap** in `utf8` zurück,
  obwohl es dafür `utf8.reset()` aufruft — die Rücknahme wird vom Destruktor des
  `MemoryOutputStream` wieder aufgehoben. ·
  **Auslöser:** ein `kanon::Wert` mit einer nicht-endlichen Zahl **unterhalb** der
  Wurzel, z. B. `{"a": NaN}`: `schreibeWert` schreibt `{"a":` (5 Bytes) und scheitert
  dann in `zahlAlsEs6` (`:32-33`, `:172-176`). ·
  **Wirkung/Mechanismus:** `aus.flush()` trimmt den Block auf N = 5; `utf8.reset()`
  (`:239`) gibt ihn frei (`data == nullptr`, `size == 0`); danach — nach `return ok` —
  läuft `~MemoryOutputStream` → `trimExternalBlockSize()` →
  `MemoryBlock::setSize(5, false)` → `HeapBlock::allocate(5, false)`, also `malloc`
  ohne Nullen (JUCE 8, `eq-copilot/build/_deps/juce-src/modules/juce_core/streams/juce_MemoryOutputStream.cpp:58-71`
  und `.../memory/juce_MemoryBlock.cpp` `setSize`). `utf8` trägt danach 5 nicht
  initialisierte Bytes statt leer zu sein. ·
  **Erreichbar:** heute **nein** über Pipe oder State. Der einzige Produktionsaufrufer
  `nakama::state::param::dtoKanon` (`state/NakamaParameter.cpp:169-174`) gibt bei
  `false` zurück, ohne `utf8` zu lesen, und `validiere` (`:165`) verriegelt
  nicht-endliche Parameter vorher, macht den Fehlerfall dort also unerreichbar. Der
  defensive `reset()` ist trotzdem wirkungslos, und die Absicherung hängt allein am
  Verhalten zweier Aufrufer statt an der Funktion, die sie zusagt. ·
  **Beleg:** `:232-241` (Reihenfolge `reset()` vor Destruktor) gegen
  `state/NakamaKanon.h:66-67` („@returns false (mit `fehler`) bei nichtendlicher Zahl"). ·
  **Register:** neu.

- **[LOW] F5 · src/SourcesModel.cpp:984-985 · konvertierung/NaN-Ehrlichkeit** ·
  `fensterDauerMs` kann `+inf` werden und wird ungefiltert gespeichert und angezeigt;
  die Bedingung `frame.sampleRate > 0.0` ist exakt dieselbe Schwelle, die der
  Telemetrieriegel schon geprüft hat, und schützt deshalb nichts. ·
  **Auslöser:** ein P2-`FeatureBatch` mit `transport.sample_rate = 1e-300` und
  `sample_count = 1048576`. Beides passiert `vertrag/NakamaTelemetrie.cpp:196-203`
  (`sample_count <= 1048576`, `isfinite(sr) && sr > 0.0 && sr <= 768000.0`). ·
  **Wirkung:** `1048576 * 1000.0 / 1e-300` überläuft binary64 → `+inf` landet in
  `Zeile::fensterDauerMs` und wird in `src/PluginEditor.cpp:1273` als
  „window inf ms" gezeichnet. Die Stale-Frist ist **nicht** betroffen:
  `messStaleFristMs` (`:1241-1242`) verriegelt nicht-endlich korrekt und fällt auf
  `messungMindestensStaleNachMs` zurück — genau dieser Riegel fehlt an der
  Speicherstelle, obwohl Invariante 4 „verriegelt und gezählt" verlangt. ·
  **Erreichbar:** ja, über die P2-Telemetriepipe;
  `EqCopilotProcessor::v3Frame` (`src/PluginProcessor.cpp:1277`) → `uebernehmeP2`. ·
  **Register:** neu.

- **[bereits registriert] src/SourcesModel.cpp:196-203 · logik/protokoll** ·
  Die Klasse↔Position-Matrix in `liesDescriptor` ist nur einseitig redundant
  (`pre`/`post` + `main` explizit, alles Weitere allein an `aussageklasse != "beobachtend"`).
  An der Quelle nachgemessen, unverändert. · **Register: NAK-141** (09-03, Gate-Lauf G3)
  — hier nicht als neuer Befund geführt.

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren/Allokationen/IO/Logging — hält.**
   `SourcesModel` hat keinen Aufrufer aus dem Audiocallback: alle 24 Fundstellen in
   `src/PluginProcessor.cpp` liegen in `v3Antwort`/`v3Frame` (IPC-Thread),
   `sourcesTick`/`sicht` (Message-Thread) und den State-Pfaden; keine in
   `processBlock`. `WorkerCadence.h:45-63` wird ausschließlich aus
   `EqCopilotProcessor::workerLauf` gefahren (`src/PluginProcessor.cpp:751`, `:954`),
   also aus dem Analyse-Worker, und tut dort nur `steady_clock`-Vergleiche — keine
   Sperre, keine Allokation. Der Kadenz-Automat holt bewusst keine verpassten Takte
   nach (`WorkerCadence.h:49-52`), Überlast verwirft damit Analysearbeit, nie Audio.
   `SourcesModel::mutex` ist ein reiner Message-/IPC-Thread-Mutex; jede öffentliche
   Methode nimmt ihn genau einmal und ruft die privaten Helfer nur unter ihm
   (`:284`, `:336`, `:367`, `:387`, `:409`, `:684`, `:807/:936`, `:966`, `:1031`,
   `:1051`, `:1092`, `:1102`, `:1108`, `:1115`, `:1127`, `:1136`) — keine
   Wiedereintrittskante, kein Nesting.
2. **Passthrough / Hörmarker fail-closed — hält.**
   `state/NakamaLebenslauf.h:137` `audioAusnahmeErlaubt()` ist einer der vier Terme.
   Er wird nie im Audiothread befragt, sondern über `spiegleKlassifikation()`
   (`src/PluginProcessor.cpp:1489-1496`) in eine Atomic gespiegelt, und zwar nach
   **jeder** Zustandsänderung des Automaten (`:1422`, `:1437`, `:1569`, jeweils unter
   `bindungMutex`). Der Automat selbst ist fail-closed: `ignoriert` ändert nichts
   (`state/NakamaLebenslauf.cpp:52-53`), `nurLesen`/`z.nurLesen` fällt auch aus einer
   früheren positiven Klassifikation auf `unclassified` zurück (`:60-64`), eine feste
   Sondenklasse kann sich nicht zum Main erklären (`:37-38`, `:66-74`), und
   `expliziteInitialisierung` verlangt fehlenden `festeKlasse`, offenen Editor und
   einen interpretierbaren Stand (`:93-108`).
3. **Gate 5 (kein Telemetrie-/Steuerframe steuert Gain oder erzeugt PRE/POST-Delta) — nicht berührt.**
   Keine meiner Einheiten schreibt einen Hostpuffer, einen Gain oder eine
   Latenzangabe. `SourcesModel` schreibt ausschließlich Anzeigezustand
   (`Zeile`/`Sicht`), `NakamaKanon` Text und Hash, `NakamaLebenslauf` eine
   Klassifikation, `ZonenRegeln.h` nur konstante Geometrie.
4. **NaN-Ehrlichkeit — hält bis auf F5.**
   Korrekt verriegelt: `messStaleFristMs` (`src/SourcesModel.cpp:1241-1247`, inkl.
   Sättigung gegen `uint64`-Überlauf), `zahlAlsEs6` (`state/NakamaKanon.cpp:32-33`:
   NaN/±Inf sind kein JSON), `Leser::zahl` (`state/NakamaKanon.cpp:373`: `from_chars`
   plus `isfinite`, kein stilles 0 oder inf), `controlAlterMs` mit sättigender
   Addition (`src/SourcesModel.cpp:1202-1203`) und negativ-geklemmtem Delta
   (`:1198-1201`, `:1209-1213`). **Bricht** an `:984-985` (F5): das Fensterergebnis
   wird weder auf Endlichkeit geprüft noch gezählt.
5. **State bleibt verlustfrei — hält.**
   `state/NakamaLebenslauf.cpp:60-64` hält einen `nurLesen`-Stand read-only und
   entzieht ihm jedes Recht, statt ihn zu interpretieren; `:66-74` lässt ein
   Sondenbundle mit fremder Klasse neutral, statt zu raten; `:52-53` lässt einen
   ignorierten Baum den gehaltenen Zustand unberührt.
   `state/NakamaKanon.cpp` erfindet keine Werte: doppelter Schlüssel ist ein Fehler
   (`:18-25`, `:410-411`), einsames Surrogat und rohes Steuerzeichen sind Fehler
   (`:293`, `:313-326`), eingebettetes NUL wird abgewiesen (`:327`), die Tiefe ist auf
   64 begrenzt (`:396`, `:424`), und `lies` verlangt, dass hinter dem Dokument nichts
   mehr steht (`:465-466`).

## Ledger

- `src/SourcesModel.cpp:91-101` (11 Z.) - clean
- `src/SourcesModel.cpp:103-123` (21 Z.) - clean
- `src/SourcesModel.cpp:125-133` (9 Z.) - clean
- `src/SourcesModel.cpp:135-141` (7 Z.) - clean
- `src/SourcesModel.cpp:143-149` (7 Z.) - clean
- `src/SourcesModel.cpp:167-243` (77 Z.) - clean (bereits registriert: NAK-141)
- `src/SourcesModel.cpp:245-262` (18 Z.) - clean
- `src/SourcesModel.cpp:264-270` (7 Z.) - clean
- `src/SourcesModel.cpp:274-324` (51 Z.) - clean
- `src/SourcesModel.cpp:326-362` (37 Z.) - clean
- `src/SourcesModel.cpp:364-383` (20 Z.) - clean
- `src/SourcesModel.cpp:385-404` (20 Z.) - clean
- `src/SourcesModel.cpp:406-438` (33 Z.) - Befund F3
- `src/SourcesModel.cpp:440-575` (136 Z.) - Befund F1
- `src/SourcesModel.cpp:576-697` (122 Z.) - Befund F1, F2
- `src/SourcesModel.cpp:698-796` (99 Z.) - clean
- `src/SourcesModel.cpp:798-945` (148 Z.) - clean
- `src/SourcesModel.cpp:947-1027` (81 Z.) - Befund F5
- `src/SourcesModel.cpp:1029-1047` (19 Z.) - clean
- `src/SourcesModel.cpp:1049-1088` (40 Z.) - clean
- `src/SourcesModel.cpp:1090-1098` (9 Z.) - clean
- `src/SourcesModel.cpp:1100-1104` (5 Z.) - clean
- `src/SourcesModel.cpp:1106-1111` (6 Z.) - clean
- `src/SourcesModel.cpp:1113-1122` (10 Z.) - clean
- `src/SourcesModel.cpp:1124-1132` (9 Z.) - clean
- `src/SourcesModel.cpp:1134-1140` (7 Z.) - clean
- `src/SourcesModel.cpp:1143-1164` (22 Z.) - clean
- `src/SourcesModel.cpp:1167-1172` (6 Z.) - clean
- `src/SourcesModel.cpp:1174-1191` (18 Z.) - clean
- `src/SourcesModel.cpp:1193-1237` (45 Z.) - clean
- `src/SourcesModel.cpp:1239-1248` (10 Z.) - clean
- `src/SourcesModel.cpp:1250-1253` (4 Z.) - clean
- `src/SourcesModel.cpp:1255-1278` (24 Z.) - clean
- `src/SourcesModel.cpp:1280-1288` (9 Z.) - clean
- `src/SourcesModel.cpp:1289-1297` (9 Z.) - clean
- `src/SourcesModel.cpp:1298-1305` (8 Z.) - clean
- `src/SourcesModel.cpp:1306-1314` (9 Z.) - clean
- `src/SourcesModel.cpp:1315-1321` (7 Z.) - clean
- `src/SourcesModel.cpp:1322-1329` (8 Z.) - clean
- `src/SourcesModel.cpp:1330-1340` (11 Z.) - clean
- `src/SourcesModel.h:file-scope-1` (127 Z.) - clean
- `src/SourcesModel.h:128-241` (114 Z.) - clean
- `src/WorkerCadence.h:file-scope-1` (14 Z.) - clean
- `src/WorkerCadence.h:13-68` (56 Z.) - clean
- `src/ZonenRegeln.h:file-scope-1` (9 Z.) - clean
- `src/ZonenRegeln.h:10-37` (28 Z.) - clean
- `state/NakamaKanon.cpp:file-scope-1` (60 Z.) - clean
- `state/NakamaKanon.cpp:18-26` (9 Z.) - clean
- `state/NakamaKanon.cpp:30-95` (66 Z.) - clean
- `state/NakamaKanon.cpp:102-109` (8 Z.) - clean
- `state/NakamaKanon.cpp:111-141` (31 Z.) - clean
- `state/NakamaKanon.cpp:143-161` (19 Z.) - clean
- `state/NakamaKanon.cpp:163-228` (66 Z.) - clean
- `state/NakamaKanon.cpp:232-241` (10 Z.) - Befund F4
- `state/NakamaKanon.cpp:243-246` (4 Z.) - clean
- `state/NakamaKanon.cpp:259-259` (1 Z.) - clean
- `state/NakamaKanon.cpp:260-260` (1 Z.) - clean
- `state/NakamaKanon.cpp:261-261` (1 Z.) - clean
- `state/NakamaKanon.cpp:263-269` (7 Z.) - clean
- `state/NakamaKanon.cpp:271-282` (12 Z.) - clean
- `state/NakamaKanon.cpp:285-342` (58 Z.) - clean
- `state/NakamaKanon.cpp:344-377` (34 Z.) - clean
- `state/NakamaKanon.cpp:379-386` (8 Z.) - clean
- `state/NakamaKanon.cpp:388-455` (68 Z.) - clean
- `state/NakamaKanon.cpp:460-470` (11 Z.) - clean
- `state/NakamaKanon.h:file-scope-1` (58 Z.) - clean
- `state/NakamaKanon.h:24-53` (30 Z.) - clean
- `state/NakamaKernRiegel.h:file-scope-1` (122 Z.) - clean
- `state/NakamaLebenslauf.cpp:file-scope-1` (14 Z.) - clean
- `state/NakamaLebenslauf.cpp:7-18` (12 Z.) - clean
- `state/NakamaLebenslauf.cpp:20-31` (12 Z.) - clean
- `state/NakamaLebenslauf.cpp:33-43` (11 Z.) - clean
- `state/NakamaLebenslauf.cpp:45-87` (43 Z.) - clean
- `state/NakamaLebenslauf.cpp:89-115` (27 Z.) - clean
- `state/NakamaLebenslauf.h:file-scope-1` (50 Z.) - clean
- `state/NakamaLebenslauf.h:51-148` (98 Z.) - clean

76 von 76 Einheiten, jede genau einmal.

## Nicht gelesen

keine.

## Geprüft und ausdrücklich KEIN Befund (damit die nächste Runde nicht zweimal sucht)

- **Keine baumelnden `juce::var`-Zeiger.** `DynamicObject::getProperty` liefert
  `const var&` auf den gespeicherten Wert
  (`_deps/juce-src/modules/juce_core/containers/juce_DynamicObject.cpp:51-54`), und
  ein Array ist ein `RefCountedArray : ReferenceCountedObject`
  (`.../juce_Variant.cpp:416-421`), das beim Kopieren die Referenz teilt. Die Zeiger
  aus `:533`, `:578`, `:615`, `:862`, `:879` bleiben gültig, solange `root` lebt —
  das tut es über die ganze Funktion.
- **Sortierprädikat in `sicht()` (`:1071-1084`) ist eine strikte schwache Ordnung**
  (Index-vorhanden trennt zwei geschlossene Gruppen, innerhalb jeder Gruppe eine
  totale Ordnung über `(index, Sortierschlüssel, Name, instanceId)`).
- **`zahlAlsEs6` folgt ECMA-262 6.1.6.1.20 korrekt.** Der Positionsbereich
  `-5 <= n <= 21` deckt genau die Schritte 8-10; das Exponentvorzeichen wird bei
  `n < 0` gesetzt statt bei `n - 1 < 0`, aber die beiden Fälle, in denen sich das
  unterschiede (`n == 0`, `n == 1`), erreichen den Exponentzweig nie. Puffer
  `char[64]` reicht für jedes `to_chars`-Ergebnis eines `double` (max. 24 Zeichen).
- **Die 8-Byte-UTF-8-Puffer (`:104`, `:334`) laufen nicht über.**
  `CharPointer_UTF8::write` schreibt höchstens 4 Bytes, auch für Werte über
  U+10FFFF (`_deps/juce-src/modules/juce_core/text/juce_CharPointer_UTF8.h`), plus
  `writeNull` also höchstens 5.
- **`setzeFixtureFuerTest` (`:1143-1164`) ist keine Produktionsnaht.**
  `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` wird in `eq-copilot/plugin/CMakeLists.txt` nur
  an Konsolen-Testziele vergeben, an kein `juce_add_plugin`-Bundle.
- **Längen-DoS über den Snapshot ist gedeckelt.** `textriegelBytes` weist über
  16 MiB ab (`vertrag/NakamaVertrag.cpp:443-447`, vor dem `static_cast<int>` bei
  `src/SourcesModel.cpp:466`), und der Wire-Envelope deckelt schon bei 256 KiB
  (`core/ipc/WireEnvelope.h:38-44`). Die fehlende Längengrenze aus F1 kostet damit
  höchstens eine 250-KiB-Zeichenkette pro Feld, keine unbegrenzte Allokation.

## Außerhalb meiner Einheiten

- `eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp:202-203` — `sample_rate` wird nur
  gegen `isfinite`, `> 0.0` und `<= 768000.0` geprüft; ein subnormaler oder
  extrem kleiner Positivwert passiert und erzeugt beim Verbraucher einen Überlauf
  (Ursprung von F5).
- `eq-copilot/plugin/src/PluginProcessor.cpp:1256-1271` — ein `ungueltig` aus
  `uebernehmeSessionSnapshot` oder `uebernehmeEvidenzruecknahme` trennt den
  Control-Link nicht und lässt `subscriptionAktiv` stehen (Vorbedingung von F3).
- `eq-copilot/plugin/state/NakamaParameter.cpp:224-226` — indiziert `objektWerte[i]`
  über `objektSchluessel.size()`; die Gleichlänge der beiden Vektoren garantiert nur
  `kanon::Wert::objektSetze`, nicht der Typ.
- `eq-copilot/plugin/tests/StateMigrationTestMain.cpp:567` — `param::dtoKanon` wird
  ohne Rückgabeprüfung aufgerufen (Kontext zu F4; Tests sind nicht Teil der Partition).
