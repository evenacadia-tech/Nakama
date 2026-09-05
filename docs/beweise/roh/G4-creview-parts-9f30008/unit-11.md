# unit-11 — src\PluginProcessor.cpp (883–2551), src\PluginProcessor.h (1–765), src\SourcesModel.cpp (1–89 + Dateiebene)
Gelesen: 91 von 91 Einheiten vollständig · Zeilen: 4382

Basis: HEAD `9f3000881df632f4969d78551fb0a516b61288cb`, strikt lesend.
Nur als Kontext mitgelesen (kein Befundort): `core/StampedAudioQueue.h`,
`core/analysis/Vergleichspegel.h`, `core/analysis/Blindvergleich.h`,
`core/ipc/InterventionsRing.h`, `core/ipc/ControlClient.cpp`,
`core/ipc/PipeToken.cpp`, `src/AnalyseEngine.{h,cpp}`, `state/NakamaState.cpp`,
`state/NakamaLebenslauf.{h,cpp}`, die Leserpfade in `src/SourcesModel.cpp` und
`src/PluginProcessor.cpp:81–860` (Konstruktor, prepareToPlay, processBlock).

## Befunde

- [HIGH] src/PluginProcessor.cpp:2090 (mit :1937 und :1946) · Logik/Protokoll (Evidenzintegrität, „NaN-Ehrlichkeit") · `erfasseKandidat` baut die Referenz eines LAUFENDEN Versuchs aus dem LIVE-`Vergleichspegel` statt aus dem eingefrorenen Bezug und ohne den Riegel `gainGesetzt()`, den `beginneVersuch` an derselben Stelle hat — ein zwischenzeitlich geleerter Pegel reist deshalb als `"match_gain_db":0` und `"nicht_endliche_samples":0` auf die Leitung. · Auslöser: irgendein `vergleichspegel.vorbereiten()` zwischen `experiment_begin` und `experiment_candidate`; zwei Produktpfade lösen es aus — (a) ein Host-Callback `prepareToPlay` (Sampleraten- oder Blockgrößenwechsel, Resume nach Suspend) über `PluginProcessor.cpp:211`, (b) `merkeManuellePassage` für eine ANDERE Passage über `bindePassagenfensterMitEpoche` (`:1817`). Beide laufen auf `leerenIntern()` hinaus (`gehalten=0.0`, `gehaltenGesetzt=false`, `nichtEndlich=0`), und keiner der beiden schließt den offenen Versuch. · Wirkung: `versuchReferenzJson` schreibt `match_gain_db` mit `zahl (vergleichspegel.gainDb())` unbedingt (`:1937`) und `nicht_endliche_samples` mit dem frisch genullten Zähler (`:1946`). 0 dB ist laut `core/analysis/Vergleichspegel.h:29-31` genau die verbotene Behauptung „die beiden sind gleich laut", und 0 nichtendliche Samples heißt laut `:134-135` „nachweislich keines" statt „nicht gemessen". Der Broker erhält damit einen formal gültigen `experiment_candidate` mit einem Lautheitsabgleich, den es nie gab — §15/§38.3 („für die Dauer des Versuchs eingefroren") und M-07 brechen still. Der Urteilspfad ist NICHT betroffen: `Blindvergleich::uebernimmVergleichspegel` kopiert `gainDb` als Wert (`Blindvergleich.h:95`); genau diese Kopie liest `erfasseKandidat` aber nicht. · Erreichbar: ja. Kette (a) Host → `EqCopilotProcessor::prepareToPlay` (`:180`) → `vergleichspegel.vorbereiten` (`:211`); danach Editor → `erfasseKandidat` (`:2075`) → `versuchReferenzJson` (`:2090`) → `sendeVersuchP0` (`:2103`) → `controlV3.sendePersistenzP0`. Kette (b) Editor → `merkeManuellePassage` (`:1648`) → `bindePassagenfenster` (`:1680`) → `bindePassagenfensterMitEpoche` (`:1817`). Sampleratewechsel ist im Bedrohungsmodell ausdrücklich als Eingabequelle geführt. · Beleg: `:1901-1951` (kein `gainGesetzt`-Riegel, `:1937`, `:1946`), `:2026` (der Riegel, den `beginneVersuch` hat), `:2075-2104`, `:211`, `:1817`, `core/analysis/Vergleichspegel.h:66-72`, `:222`, `:323-332`. · Register: neu

- [MEDIUM] src/PluginProcessor.cpp:2022-2027 · Rückgabewerte/Fehlerpfade (aktivieren↔abklingen) · `beginneVersuch` schaltet die Pegel-Speisung ab (`versuchspegelSpeist=false`), bevor es einfriert, und schaltet sie auf dem Fehlerweg „zu wenig Material" nie wieder ein — die betroffene Passage kann danach nie mehr Material sammeln, und der Handgriff scheitert dauerhaft stumm. · Auslöser: `beginneVersuch(passageId)` für die gebundene Passage, bevor 400 ms Material im Fenster liegen (`Vergleichspegel::kMindestSekunden`; `bereitIntern()` scheitert an `gesehen < mindestSamples`) — der Normalfall, wenn der User markiert und sofort startet oder bei stehendem Transport markiert. · Wirkung: `friereEin()` friert in diesem Zweig ABSICHTLICH nicht ein (`Vergleichspegel.h:203-208`, „ein gesperrter Zustand ohne Wert ist ehrlich"), ist also für einen Retry ausgelegt. Der einzige Setzer von `versuchspegelSpeist=true` ist `bindePassagenfensterMitEpoche:1819`; `beginneVersuch` erreicht ihn nur über den Zweig „Passage NICHT gebunden" (`:2012-2016`). Ist die Passage gebunden, ist jeder weitere Aufruf dieselbe Absage ohne Ausweg. Rückweg nur über den Umweg `vergissManuellePassage` + `merkeManuellePassage` oder über das Binden einer fremden Passage und Zurückwechseln. Das ist derselbe Fehler, den der Code an `:2152-2159` selbst als unzulässig benennt („Ein Handgriff, der beim zweiten Mal stumm nicht mehr geht, ist derselbe Fehler wie ein totes Element"). Der Fall `nichtEndlich > 0` ist davon zu trennen — dort ist der Endzustand gewollt. · Erreichbar: ja, reine Bedienfolge; Editor → `beginneVersuch` (`:1975`). · Beleg: `:2022` (`store(false)`), `:2023`, `:2026-2027` (Rückkehr ohne Wiederanschalten), `:1819` (einziger Gegenpfad), `core/analysis/Vergleichspegel.h:187-215`, `:314-321`. · Register: neu

- [MEDIUM] src/PluginProcessor.cpp:1395-1446 · Rückgabewerte/Fehlerpfade (Save↔Load) · `setStateInformation` löscht die ausstehenden und bestätigten Sources-Kommandos, lässt aber den gesamten Experiment- und Passagenzustand stehen; ein unter Projekt A begonnener Versuch meldet danach unter der Bindung von Projekt B. · Auslöser: der Host ruft `setStateInformation` (Projektwechsel, Preset-Recall, Copy/Paste einer Instanz), während `versuchIdAktiv` gesetzt ist oder eine Passage das Fenster führt. · Wirkung: `versuchIdAktiv`, `versuchPassageId`, `gebundenePassageId`/`gebundenerStart`/`gebundenesEnde`/`gebundeneEpoche`, `passagenfensterWunsch`/`…Start`/`…Ende`, `pegelFensterAktiv`, `versuchspegelSpeist`, `vergleichspegel` und `blindvergleich` bleiben unverändert. Der nächste `urteileVersuch`/`erfasseKandidat`/`brichVersuchAb` baut seinen Kopf mit `versuchKopfJson` aus dem FRISCHEN `v3Hello()` (`:1955-1960`) und schickt die alte `experiment_id` unter der neuen `project_binding_id`/`instance_id`. Parallel misst der Audiothread den Vergleichspegel weiter über das Sample-Fenster des alten Projekts, weil weder das Engine-Fenster noch `pegelFensterAktiv` zurückgesetzt werden, und `passagenfensterFuehrt(id)` behauptet eine Passage, die im geladenen State nicht mehr existiert. Die Asymmetrie ist belegt, nicht vermutet: für genau diesen Reload-Fall hat `wendeBestaetigteSourcesCommandsAn` einen expliziten Riegel (`:2291-2294`, Vergleich `projectBindingId` und `v3SessionEpoch`), der Experimentpfad hat keinen. · Erreichbar: ja; Host → `setStateInformation` (`:1395`), danach Editor → `urteileVersuch` (`:2106`) bzw. `erfasseKandidat` (`:2075`). · Beleg: `:1404-1408` (nur Sources-Kommandos werden geleert), `:1410-1446`, `:1955-1963`, `:2291-2294`, `:2069-2072`, `:1795-1806`. · Register: neu (NAK-169 betrifft den ACK-Zeitpunkt, nicht die Reload-Grenze)

- [LOW] src/PluginProcessor.cpp:2521-2525 (Allokation bei :2348) · alloc-lifetime · `schreibeSnapshotDatei` legt den Wurzel-`DynamicObject` roh an und übergibt ihn erst ganz am Ende an einen `juce::var`; auf dem Fehlerweg „Ordner nicht anlegbar" kehrt die Funktion ohne Eigentümer zurück und leckt den ganzen Snapshotbaum. · Auslöser: `ordner.createDirectory()` scheitert für `%LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots` — etwa weil dort bereits eine Datei gleichen Namens liegt, das Verzeichnis schreibgeschützt ist oder `LOCALAPPDATA` auf einen nicht anlegbaren Pfad zeigt. · Wirkung: der bis dahin gebaute Baum (Wurzel plus alle Kindobjekte, u. a. 3 × 221 Perzentilwerte, 3 × 221 LTAS-Werte und alle Befundkarten) wird nie freigegeben; jeder weitere Klick auf „Snapshot schreiben" leckt erneut. Der JUCE-Leakdetector schlägt nur im Debugbau an. Keine Korruption, kein Angreiferprimitiv. Erfolgs- und Schreibfehlerpfad sind sauber: `juce::var (wurzel)` in `:2537` übernimmt und gibt frei. · Erreichbar: ja, aber nur über Dateisystemzustand im Benutzerprofil; ein Same-user-Angreifer mit Schreibrecht im Profil ist nach Entwurf §48.4 ausgeschlossen, ein defekter oder umgeleiteter Pfad nicht. Editor → `schreibeSnapshotDatei` (`:2335`). · Beleg: `:2348` (`new juce::DynamicObject()`), `:2521-2525` (`return false` ohne `var`-Übernahme), `:2537` (die einzige Übernahme). · Register: neu

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren, Allokationen, Datei-/Pipe-/Netzzugriff und Logging; Überlast verwirft Analyseframes, nie Audio** — **hält.** Meine Einheiten liegen vollständig auf Worker-, Nachrichten-, Host- und IPC-Threads. Der Workerzug `:883-943` läuft unter `analyseSteuerMutex`, nie im Audiothread; der Pipe- und JSON-Sender `interventionenSenden()` wird bewusst AUSSERHALB dieser Sperre gerufen (`:976`, der Sperrblock endet `:969`). Der Zug ist auf 8 Blöcke begrenzt (`:881-883`) und bricht bei wartendem Steuerer ab (`:887`), sodass Prepare/Reset/Snapshot nicht verhungern. Verworfen werden ausschließlich Analyseblöcke (`:893-899` veraltete Anläufe, `:919-920` fail-closed ohne gültige Rate, `:935` Straddle) — nie Audio. Die Telemetriezähler der Quarantäne (`PluginProcessor.h:147-149`) sind Atomics (`core/StampedAudioQueue.h:821-823`), also kein Datenrennen gegen den mutierenden Worker; `merkmale`/`engine` werden außerhalb des Workers ausnahmslos unter `externerAnalyseSteuerZug()` bzw. über das intern gesperrte `engine.snapshot()` gelesen.
2. **Passthrough bitidentisch, kein Hostpuffer** — **nicht berührt.** Keine meiner Einheiten fasst einen `juce::AudioBuffer` an. Die einzige zugelassene Ausnahme (`markierung`) wird in meinen Einheiten nur GELESEN (`:1199` Resync-Riegel, `:1357` Heartbeat-Bit). `versuchTrocken` (`PluginProcessor.h:697`) wird ausschließlich in `prepareToPlay` dimensioniert; der Audiopfad prüft die Größe fail-closed (`:511`) — außerhalb meiner Einheiten, ohne Befund.
3. **Gate 5: kein Telemetrie- oder Steuerframe steuert samplegenauen Gain oder erzeugt ein hörbares PRE/POST-Delta** — **hält.** `v3Frame` (`:1273-1279`) reicht die FlatBuffers-Nutzlast nur an `SourcesModel::uebernehmeP2` (Anzeigemodell); `v3Antwort` (`:1233-1271`) wirkt nur auf `ausstehendeSourcesCommands`, `SourcesModel` und die Diagnose; `wendeBestaetigteSourcesCommandsAn` (`:2278-2321`) schreibt ausschließlich die Mitgliederliste im State. Kein Pfad von einem Pipe-Frame erreicht `markierung`, `versuchTrocken`, `pegelFenster*` oder einen Gain. Die einzigen Schreiber von `pegelFensterAktiv`/`versuchspegelSpeist` sind User-Handgriffe und der Analyseworker (`:870/874`, `:1816/1819`, `:1865/1866`, `:2022`), und sie steuern die MESSUNG, nicht das Audio.
4. **NaN-Ehrlichkeit** — **bricht an einer Stelle** (HIGH-Befund): `versuchReferenzJson:1946` kann `nicht_endliche_samples: 0` als „nachweislich keines" senden, obwohl der Zähler zwischenzeitlich genullt wurde, und `:1937` schickt dazu `match_gain_db: 0`. Sonst hält sie: `messKompakt:1341-1345` bildet nichtendliche LTAS-Werte ausdrücklich auf `quiet_NaN` (⇒ JSON `null`) ab, `truePeakGueltig:1329` fällt bei NaN korrekt auf `false`, `zahl():1752-1760` und `schreibeSnapshotDatei:2344-2347` verriegeln nichtendliche Werte, und `nichtnegativeGanzzahl` (`SourcesModel.cpp:47-64`) weist Doubles sowie Werte über 2^53−1 ab.
5. **State bleibt verlustfrei** — **hält.** `setStateInformation` kehrt bei `ignoriert` unverändert um (`:1402-1403`), hält `nurLesen` read-only und meldet dafür kein Host-Dirty (`:1410-1429`, `:1445`); `getStateInformation:1389-1393` gibt über `nakama::state::speichere` bei `nurLesen` die Originalbytes bytegleich zurück (`state/NakamaState.cpp:982-989`). Alle Schreibkanten (`setzeBindung:1573`, `neueSensorId:1373`, `merkeManuellePassage:1681`, `vergissManuellePassage:1711`, `benenneSourcesHauptziel:1606`, `entferneSourcesHauptziel:1641`, `wendeBestaetigteSourcesCommandsAn:2318`) melden Dirty und prüfen `zustand.nurLesen` zuerst. Schreib- und Lesegrenzen sind symmetrisch (Label ≤ 120, pairId ≤ 60, Passagen `0 <= start < end`, hex32-IDs, Deckel `maxManuellePassagen`/`maxMainProjectMitglieder`) gegen `state/NakamaState.cpp:712-720`, `:759-772`, `:803-834`. Der Befund oben zu `setStateInformation` betrifft NICHT die Bytes, sondern den nicht zurückgesetzten Laufzeitzustand daneben.

## Ledger
src\PluginProcessor.cpp:883-988 · clean
src\PluginProcessor.cpp:1001-1043 · clean
src\PluginProcessor.cpp:1045-1089 · clean
src\PluginProcessor.cpp:1091-1113 · clean
src\PluginProcessor.cpp:1115-1149 · clean
src\PluginProcessor.cpp:1151-1159 · clean
src\PluginProcessor.cpp:1161-1170 · clean
src\PluginProcessor.cpp:1172-1231 · clean
src\PluginProcessor.cpp:1233-1271 · clean
src\PluginProcessor.cpp:1273-1279 · clean
src\PluginProcessor.cpp:1281-1288 · clean
src\PluginProcessor.cpp:1290-1293 · clean
src\PluginProcessor.cpp:1295-1310 · clean
src\PluginProcessor.cpp:1316-1359 · clean
src\PluginProcessor.cpp:1365-1378 · clean
src\PluginProcessor.cpp:1380-1383 · clean
src\PluginProcessor.cpp:1389-1393 · clean
src\PluginProcessor.cpp:1395-1446 · Befund 3
src\PluginProcessor.cpp:1448-1448 · clean
src\PluginProcessor.cpp:1449-1449 · clean
src\PluginProcessor.cpp:1450-1450 · clean
src\PluginProcessor.cpp:1451-1451 · clean
src\PluginProcessor.cpp:1453-1457 · clean
src\PluginProcessor.cpp:1459-1463 · clean
src\PluginProcessor.cpp:1465-1469 · clean
src\PluginProcessor.cpp:1471-1475 · clean
src\PluginProcessor.cpp:1477-1481 · clean
src\PluginProcessor.cpp:1485-1496 · clean
src\PluginProcessor.cpp:1498-1502 · clean
src\PluginProcessor.cpp:1504-1508 · clean
src\PluginProcessor.cpp:1510-1520 · clean
src\PluginProcessor.cpp:1522-1578 · clean
src\PluginProcessor.cpp:1580-1583 · clean
src\PluginProcessor.cpp:1585-1609 · clean
src\PluginProcessor.cpp:1611-1646 · clean
src\PluginProcessor.cpp:1648-1684 · clean
src\PluginProcessor.cpp:1686-1714 · clean
src\PluginProcessor.cpp:1731-1749 · clean
src\PluginProcessor.cpp:1752-1760 · clean
src\PluginProcessor.cpp:1762-1765 · clean
src\PluginProcessor.cpp:1768-1787 · clean
src\PluginProcessor.cpp:1789-1822 · clean
src\PluginProcessor.cpp:1824-1831 · clean
src\PluginProcessor.cpp:1833-1845 · clean
src\PluginProcessor.cpp:1847-1850 · clean
src\PluginProcessor.cpp:1852-1869 · clean
src\PluginProcessor.cpp:1871-1886 · clean
src\PluginProcessor.cpp:1888-1899 · clean
src\PluginProcessor.cpp:1901-1951 · Befund 1
src\PluginProcessor.cpp:1953-1964 · clean
src\PluginProcessor.cpp:1966-1973 · clean
src\PluginProcessor.cpp:1975-2073 · Befund 2
src\PluginProcessor.cpp:2075-2104 · Befund 1
src\PluginProcessor.cpp:2106-2162 · clean
src\PluginProcessor.cpp:2164-2187 · clean
src\PluginProcessor.cpp:2189-2193 · clean
src\PluginProcessor.cpp:2195-2198 · clean
src\PluginProcessor.cpp:2200-2203 · clean
src\PluginProcessor.cpp:2205-2208 · clean
src\PluginProcessor.cpp:2210-2214 · clean
src\PluginProcessor.cpp:file-scope-2 · clean
src\PluginProcessor.cpp:2216-2276 · clean
src\PluginProcessor.cpp:2278-2321 · clean
src\PluginProcessor.cpp:2324-2329 · clean
src\PluginProcessor.cpp:2335-2484 · Befund 4
src\PluginProcessor.cpp:2485-2544 · Befund 4
src\PluginProcessor.cpp:2546-2549 · clean
src\PluginProcessor.h:file-scope-1 · clean
src\PluginProcessor.h:47-196 · clean
src\PluginProcessor.h:197-316 · clean
src\PluginProcessor.h:317-318 · clean
src\PluginProcessor.h:319-321 · clean
src\PluginProcessor.h:327-334 · clean
src\PluginProcessor.h:342-349 · clean
src\PluginProcessor.h:389-400 · clean
src\PluginProcessor.h:402-406 · clean
src\PluginProcessor.h:411-411 · clean
src\PluginProcessor.h:420-420 · clean
src\PluginProcessor.h:file-scope-2 · clean
src\PluginProcessor.h:556-556 · clean
src\PluginProcessor.h:558-558 · clean
src\PluginProcessor.h:562-568 · clean
src\PluginProcessor.h:file-scope-3 · clean
src\PluginProcessor.h:673-674 · clean
src\PluginProcessor.h:file-scope-4 · clean
src\SourcesModel.cpp:file-scope-1 · clean
src\SourcesModel.cpp:18-21 · clean
src\SourcesModel.cpp:23-40 · clean
src\SourcesModel.cpp:42-45 · clean
src\SourcesModel.cpp:47-64 · clean
src\SourcesModel.cpp:66-89 · clean

## Nicht gelesen
keine

## Außerhalb meiner Einheiten
- `eq-copilot/plugin/core/ipc/InterventionsRing.h:94-98`: der Kommentar sagt, der Sender trage `dauerSamples` und `tailSamples` beide, „damit ein Empfaenger die Konservativitaet PRUEFEN kann statt sie zu glauben". Auf der Leitung steht nur `tail_samples`; `audible_intervention_end` in `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:1179-1195` hat `additionalProperties: false` und kein Dauerfeld, und der einzige Leser von `dauerSamples` ist `eq-copilot/plugin/tests/Sonde013InterventionRingTest.cpp:150`.
- `eq-copilot/plugin/src/SourcesModel.cpp:201` und `:228`: das brokergelieferte Mitglieds-`label` wird nur längenbegrenzt (≤ 120) übernommen, ohne die Zeichenprüfung, die `hostNameGueltig` (`:125-133`) für `host_bus_name` fährt; es reist über `sendeSourcesCommand:2254` und `wendeBestaetigteSourcesCommandsAn:2303` in den persistenten FL-Projektstate. Der State-Leser (`state/NakamaState.cpp:759-772`) prüft ebenfalls nur die Länge, Save↔Load bleibt also symmetrisch.
- `eq-copilot/plugin/src/PluginProcessor.cpp:1273-1279` (`v3Frame`): `uebernehmeP2` füllt bei ungültigem oder zu neuem FeatureBatch die Ausgabe `fehler`, die hier nie gelesen wird; der Schwesterpfad `v3Antwort` setzt für denselben Fall `Diagnose::incompatible` (`:1268-1270`).
- `eq-copilot/plugin/src/PluginProcessor.cpp:1069-1073` und `PluginProcessor.h:374-378`: `logon_sid` wird ohne JSON-Escaping konkateniert, während `adresseAlsJson` (`core/ipc/ControlClient.cpp:476-483`) dafür `jsonString` benutzt. Heute folgenlos, weil `aktuelleLogonSid()` (`core/ipc/PipeToken.cpp:147-182`) nur die ASCII-SID-Textform liefert.
- `eq-copilot/plugin/src/PluginProcessor.cpp:1143-1145`: der Kommentar nennt `interventionsRing.zuruecksetzen()` als zweiten Löschpfad des Sticky-Bits; im Produkt hat diese Methode keinen Aufrufer, ebenso wenig `verworfeneEreignisse()`.
