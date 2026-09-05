# unit-10 — src/PluginEditor.cpp, src/PluginEditor.h, src/PluginFactory.cpp, src/PluginProcessor.cpp
Gelesen: 53 von 53 Einheiten vollständig · Zeilen: 2536 (Summe der Einheitenbereiche; `file-scope`-Einheiten nach `ranges` gerechnet)

Gelesene Bereiche: `PluginEditor.cpp` 233–1663 · `PluginEditor.h` 1–183 · `PluginFactory.cpp` 1–8 ·
`PluginProcessor.cpp` 1–882 plus die verstreuten `file-scope`-Zeilen bis 2209.
Kontext zusätzlich gelesen (nicht meine Einheiten, nur zur Erreichbarkeitsprüfung):
`HoerMarkierung.h` 157–240/320–520, `AnalyseEngine.cpp` 165–200/310–340/595–690/1040–1070,
`SourcesModel.cpp` (Längenriegel), `state/NakamaState.cpp` 595–860, `core/ipc/PipeToken.cpp` 120–184,
`core/ipc/ControlClient.cpp` 478–490, `PluginProcessor.cpp` 1648–1830/1995–2025.

## Befunde

- [MEDIUM] src/PluginProcessor.cpp:611 · Konvertierung/UB + Logik/Protokoll · Der Quarantäne-Tail wird aus JUCEs **unbereinigter** Host-Samplerate gerechnet, während die ganze übrige Funktion auf der in `prepareToPlay:182-184` geprüften `samplerateAtomic` steht. · Auslöser: der Host meldet in `setupProcessing`/`setRateAndBufferSizeDetails` eine nicht-endliche oder über `INT_MAX` liegende Samplerate (Bedrohungsmodell: „Sampleratewechsel"), während eine Hörmarkierung hörbar ist. `prepareToPlay` setzt daraufhin `samplerateAtomic = 0.0` (`:182-190`) und `echtzeitOk = false` (`:212`), `markierung.setzeSamplerate(0.0)` (`:206`) lässt `lokal.fs == fsAktuell` scheitern, der erzwungene Ausfade läuft aus und `schritt.endete` wird wahr (`HoerMarkierung.h:500-515`). · Wirkung: `(int) getSampleRate()` konvertiert einen nicht darstellbaren `double` → UB; auf MSVC/x64 liefert `cvttsd2si` `INT_MIN`, `INT_MIN/10` ist negativ, `std::max (1, …)` ergibt **1**. `e.tailSamples` schrumpft damit von rund 100 ms auf **ein Sample** — genau der Fall, den der Kommentar `:604-609` ausschließt („Zu kurz wäre hier der teure Fehler"); gefärbtes Audio liegt danach außerhalb des invalidierten Bereichs (§34.2). · Erreichbar: ja — `processBlock` (`:233`) → `markierung.verarbeite` (`:545`) → `schritt.endete` (`:563`) → `:610-611`; Aufrufer ist der Audiothread des Hosts. · Beleg: `:610-611` gegen `:182-184`, `:206`, `:211`; einziger weiterer `getSampleRate()`-Leser ist `:1817` (außerhalb meiner Einheiten). · Register: neu

- [LOW] src/PluginProcessor.cpp:257-260 · NaN-Ehrlichkeit / Integer-Überlauf im Gleitkomma-Akku · Der `float`-RMS-Akkumulator kann bei **endlichen**, sehr großen Samples nach `+inf` laufen; der Wert wird verriegelt, aber nicht gezählt. · Auslöser: ein vorgeschaltetes fremdes Plugin im selben FL-Host (im Bedrohungsmodell ausdrücklich enthalten) liefert endliche `float`-Samples mit |v| ≳ 1,3e19; `summe += v*v` (`:257`) überschreitet dann `FLT_MAX`. · Wirkung: `rms` ist nicht endlich, `:260` speichert `0.0f`; `nanSeen` bleibt aber falsch, weil `:255` nur das **rohe** Sample auf `isfinite` prüft. Die Statuszelle „DATEN" (`PluginEditor.cpp:1622-1632`) zeigt weiter „OK", während `rmsL`/`rmsR` still auf 0 liegen. Das ist der einzige Punkt in meinen Einheiten, an dem ein nicht-endlicher Zwischenwert verriegelt, aber nicht gezählt wird. · Erreichbar: ja — Audiothread des Hosts → `processBlock:248-265`; Anzeigepfad `statsSnapshot()` → `PluginProcessor.cpp:1298-1304` → Editor `:1622`. · Beleg: `:251`, `:255`, `:257`, `:259-260`, `:264-265`. · Register: neu

- [LOW] src/PluginProcessor.cpp:863-875 · Nebenläufigkeit · Beim **Wechsel** der gebundenen Passage werden `pegelFensterStart` und `pegelFensterEnde` überschrieben, während `pegelFensterAktiv` aus der vorigen Veröffentlichung durchgehend `true` bleibt; es gibt kein „Bit aus → neue Grenzen → Bit an". · Auslöser: `merkeManuellePassage` für eine zweite Passage während laufender Wiedergabe (`:1648` → `:1789-1806`), während das Fenster der ersten Passage aktiv ist. · Wirkung: der Audiothread lädt das Bit mit `acquire` (`:507`) und danach die beiden Grenzen einzeln mit `relaxed` (`:522-523`); er kann für einen Hostblock das Paar `[neuerStart, altesEnde]` sehen und speist `vergleichspegel` (`:547-553`) mit Material außerhalb der gebundenen Passage — dieselbe Fehlerklasse, die Befund C4 schließen sollte. Die Generationsprüfung `:863` schützt nur den Worker gegen den Nachrichtenthread, nicht den Audiothread gegen den Worker. · Erreichbar: ja, aber nur im Nanosekundenfenster zwischen den zwei `relaxed`-Stores; Folge ist ein falscher `match_gain_db`, kein Speicherfehler. · Beleg: `:866-871` gegen `:506-523`; Kommentar `:855-858` beschreibt die Ordnung nur für den Löschfall. · Register: eigener Fall; Nachbarn sind die bereits datierten NAK-158 (Transportkante löscht `pegelFensterAktiv` nicht) und NAK-159 (halber Stereoblock in `friereEin`), beide in denselben Zeilen `:507` / `:549-553`.

- [LOW] src/PluginProcessor.cpp:155-162 · Objektlebenszyklus / Exception-Sicherheit · Der Analyseworker wird gestartet, **bevor** die vier Client-Starts laufen, die selbst Threads erzeugen und werfen können. · Auslöser: `std::system_error` aus `pipe.start()` (`PipeClient.cpp:229-237`, erzeugt einen Thread), `controlV3.start()` (`ControlClient.cpp:678-688`), `telemetryV3.start()` oder `brokerLifecycle.start()` — praktisch nur unter Thread-/Handle-Erschöpfung des Prozesses. · Wirkung: bei einem Wurf aus dem Konstruktorrumpf läuft `~EqCopilotProcessor` (`:165-178`) **nie**; `workerLaeuft` bleibt `true`, der Worker wird nicht benachrichtigt und nicht gejoint, und die Zerstörung des `joinable`-Members `worker` (`PluginProcessor.h:544`) ruft `std::terminate()`. Statt eines fehlgeschlagenen Plugin-Ladens bricht der ganze Hostprozess ab. · Erreichbar: nur unter Ressourcenerschöpfung; ein lokaler unprivilegierter Prozess kann sie systemweit provozieren, trifft dann aber nicht gezielt diese Instanz. · Beleg: `:155-162` gegen `:165-178` und `PluginProcessor.h:544`. · Register: neu

- [LOW] src/PluginProcessor.cpp:511 · Integer · `(std::size_t) (buffer.getNumSamples() * kanaele)` multipliziert zwei `int`, bevor verbreitert wird. · Auslöser: ein Hostblock mit mehr als `INT_MAX/2` Samples. · Wirkung: die Multiplikation ist vorzeichenbehafteter Überlauf (UB); das gewrappte Ergebnis wird als `size_t` riesig, der Riegel weist den Block also **zufällig** weiterhin ab — die Schranke hält aus dem falschen Grund. Der Riegel selbst ist korrekt dimensioniert (`versuchTrocken` fasst `max(1,maxBlock)*2` Floats, `:210`), und die `memcpy`-Kette `:541-543` / `:550-553` bleibt innerhalb. · Erreichbar: nein, praktisch — ein 4-GiB-Float-Hostpuffer kommt aus FL nicht. · Beleg: `:511` gegen `:210`, `:541-543`. · Register: neu (Härtung: erst verbreitern, dann multiplizieren)

- [LOW] src/PluginEditor.cpp:589-595 · Objektlebenszyklus (Asymmetrie zu den beiden Nachbarn) · `zeigeKonflikt` fängt den **rohen** `this` des Editors in der Aktionslambda des `KonfliktPanel`, während die beiden anderen Popover derselben Datei bewusst `juce::Component::SafePointer` benutzen und im Kommentar begründen, dass die CallOutBox den Editor überleben kann (`:505-507`, `:731-737`). · Auslöser: Klick auf „Dieser Instanz eine neue Kennung geben" (`:560-566`) nach Zerstörung des Editors. · Wirkung: `statusMeldung`/`statusMeldungBisMs` und `processor` würden auf einem zerstörten Editor beschrieben (Use-after-free). · Erreichbar: **nein** im heutigen Stand — die CallOutBox wird mit dem Editor als Elternkomponente gestartet (`:597-598`), verliert beim Editor-Abbau ihren Peer und ist danach nicht mehr klickbar. Erreichbar würde es durch genau eine Änderung: Start ohne Elternkomponente (Desktop-CallOut) oder ein asynchron aufgerufenes `tun`. · Beleg: `:589-595` gegen `:507-519` und `:734-742`. · Register: neu (Härtung)

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren, Allokationen, Datei-/Pipe-/Netzzugriff und Logging; Überlast verwirft Analyseframes, nie Audio — hält.**
   `processBlock` (`PluginProcessor.cpp:233-616`) nimmt keine Sperre, allokiert nicht (`versuchTrocken` ist in `prepareToPlay:210` vorallokiert; der Riegel `:511` überspringt einen zu großen Block, statt zu vergrößern), fasst weder Datei noch Pipe an — die zwei Interventionsereignisse gehen in den vorallokierten `interventionsRing` (`:613`), gesendet wird im Worker. Kein Logging. `lebenszeichen` (`:688-749`) benutzt nur `steady_clock` und Atomics. Überlast: `queue.veroeffentliche` (`:366`) darf `false` liefern, der Block fällt aus der **Analyse**, das Audio bleibt unberührt (`:375-380`).
2. **Passthrough sampleidentisch, nur der Hörmarker als Ausnahme, vier fail-closed Terme — hält.**
   Der Hostpuffer wird an genau einer Stelle geschrieben: `markierung.verarbeite (buffer, kanaele, erlaubt)` (`:545`). Die Trockenkopie `:541-543` **liest** den Puffer und schreibt nur in `versuchTrocken`. `erlaubt` (`:470-479`) trägt alle vier Terme: `spielt` (`:475`, gültiges playing), `aufnahmeAus` (`:470-471`, unbekannter Aufnahmezustand blockiert wie ein aktiver), `echtzeitOk || testEchtzeit` zusammen mit `! isNonRealtime()` (`:473-477`) und `editorOffen` (`:478-479`), zusätzlich `istMainKlassifiziert` (`:472`). Editorseitig setzen `~EqCopilotEditor:237-238` (`markierungAus` + `setzeEditorOffen(false)`) und die Sicherheitsnetz-Ticks `PluginEditor.cpp:270-278` (Freilauf, Samplerate-Wechsel, 10-min-Totmann) den Gegenpfad.
3. **Gate 5: kein Telemetrie- oder Steuerframe erzeugt samplegenauen Gain — hält in diesen Einheiten.**
   Einziger Erzeuger eines `MarkierungsAuftrag` ist `EqCopilotEditor::schalteMarkierung` (`PluginEditor.cpp:754-815`), ausgelöst nur durch Klick auf die Befundkarte (`:454-459`) oder den Not-Aus. `baueMarkierungsAuftrag` (`:793`) weist alles Nicht-Endliche ab. `nakamaBlockEmpfangen` (`PluginProcessor.cpp:629-681`) schreibt ausschließlich Transportmetadaten in `brueckeStand.stempel`, keinen Gain, keine Markierung. Kein Pfad aus Pipe oder Modell erreicht `markierungEinreichen` in meinen Einheiten.
4. **NaN-Ehrlichkeit — hält mit einer Lücke (Befund 2).**
   Verriegelt und gezählt: `:255/:264-265` (nicht-endliches Sample), `:260` (RMS). Überlaufsicher: `projektEnde` (`:23-29`), `projektAbstandGroesserAls64` (`:31-39`, Bias-Flip, definiert auch an INT64_MIN/MAX), Sättigung von `e.projektSample` (`:600-602`). Editorseitig: `anzeigeKurve:1057` und `zeichneKurve:1079` überspringen NaN, `paint:1370-1371` prüft `isfinite` vor der Anzeige, `paintMainFlaeche:1299-1303` gibt ohne endliche Werte **keinen** LUFS-Wert aus. Gegenprobe zur Frage, ob `+inf` in die Zeichenpfade gelangen kann: nein — die Eingangssamples werden in `AnalyseEngine.cpp:352/357` auf endlich gezwungen, und ein Snapshot wird nur bei `sr > 0` veröffentlicht (`:1050`), so dass `ltasZentrenHz` immer echte Bandmitten trägt (`:1055`). Die Nullwerte des editoreigenen Default-`MessSnapshot` sind durch `zustand == keineDaten` (`PluginEditor.cpp:1516`) von `zeichneKurve` und von der Resonanzschleife abgeriegelt. Lücke: der endliche Überlauf des RMS-Akkus wird verriegelt, aber nicht gezählt (Befund 2).
5. **State bleibt verlustfrei — hält, soweit berührt.**
   `stateNurLesen()` verriegelt in meinen Einheiten drei Wege: `istMainFlaeche` (`PluginEditor.cpp:830`) hält die Main-Fläche zu, `zeigeMesspunkt:616-622` bricht vor jeder Bindungsänderung ab, und `paint:1359-1364` / `:1639-1648` zeigen den read-only-Zustand mit dem Grund aus der State-Bibliothek dauerhaft an (Anzeige-Pflicht). `neueSensorId()` wird auf Fehlschlag geprüft und ehrlich gemeldet (`:591-593`). `prepareToPlay` und `processBlock` fassen den State nicht an. Gegenprobe zur untrusted-State-Kante: die Passagengrenzen, die über `bindePassagenfenster` bis in `pegelFensterStart/Ende` und damit in den Audiothread reisen, sind beim Laden auf `0 <= start < end` geprüft (`state/NakamaState.cpp:820-827`), deshalb ist die Differenz `:522-523` unter dem Riegel `blockAnfang >= 0` (`:520`) überlauffrei.

## Ledger

- `src\PluginEditor.cpp:233-240` · clean
- `src\PluginEditor.cpp:242-340` · clean
- `src\PluginEditor.cpp:370-375` · clean
- `src\PluginEditor.cpp:377-476` · clean
- `src\PluginEditor.cpp:478-492` · clean
- `src\PluginEditor.cpp:496-533` · clean
- `src\PluginEditor.cpp:538-599` · Befund 6
- `src\PluginEditor.cpp:601-609` · clean
- `src\PluginEditor.cpp:614-743` · clean
- `src\PluginEditor.cpp:748-752` · clean
- `src\PluginEditor.cpp:754-815` · clean
- `src\PluginEditor.cpp:817-826` · clean
- `src\PluginEditor.cpp:828-831` · clean
- `src\PluginEditor.cpp:833-872` · clean
- `src\PluginEditor.cpp:874-900` · clean
- `src\PluginEditor.cpp:902-944` · clean
- `src\PluginEditor.cpp:946-949` · clean
- `src\PluginEditor.cpp:951-967` · clean
- `src\PluginEditor.cpp:969-972` · clean
- `src\PluginEditor.cpp:974-988` · clean
- `src\PluginEditor.cpp:990-994` · clean
- `src\PluginEditor.cpp:996-1043` · clean
- `src\PluginEditor.cpp:1047-1066` · clean
- `src\PluginEditor.cpp:1068-1099` · clean
- `src\PluginEditor.cpp:1101-1249` · clean
- `src\PluginEditor.cpp:1250-1329` · clean
- `src\PluginEditor.cpp:1331-1472` · clean
- `src\PluginEditor.cpp:1473-1621` · clean
- `src\PluginEditor.cpp:1622-1663` · clean
- `src\PluginEditor.h:file-scope-1` · clean
- `src\PluginEditor.h:25-30` · clean
- `src\PluginEditor.h:31-180` · clean
- `src\PluginEditor.h:181-182` · clean
- `src\PluginEditor.h:183-183` · clean
- `src\PluginFactory.cpp:file-scope-1` · clean
- `src\PluginFactory.cpp:5-8` · clean
- `src\PluginProcessor.cpp:file-scope-1` · clean
- `src\PluginProcessor.cpp:23-29` · clean
- `src\PluginProcessor.cpp:31-39` · clean
- `src\PluginProcessor.cpp:41-51` · clean
- `src\PluginProcessor.cpp:53-64` · clean
- `src\PluginProcessor.cpp:66-71` · clean
- `src\PluginProcessor.cpp:73-78` · clean
- `src\PluginProcessor.cpp:81-163` · Befund 4
- `src\PluginProcessor.cpp:165-178` · clean
- `src\PluginProcessor.cpp:180-220` · clean
- `src\PluginProcessor.cpp:222-231` · clean
- `src\PluginProcessor.cpp:233-382` · Befund 2
- `src\PluginProcessor.cpp:383-505` · clean
- `src\PluginProcessor.cpp:506-616` · Befund 1, Befund 5
- `src\PluginProcessor.cpp:629-681` · clean
- `src\PluginProcessor.cpp:688-749` · clean
- `src\PluginProcessor.cpp:751-882` · Befund 3

## Nicht gelesen

keine

## Außerhalb meiner Einheiten

- `src/PluginProcessor.cpp:1817` — `vergleichspegel.vorbereiten (getSampleRate())` benutzt die rohe Host-Samplerate, während derselbe Aufruf in `prepareToPlay:211` die geprüfte `sichereSamplerate` bekommt. Unbewertet; dieselbe Quelle wie Befund 1.
- `state/NakamaState.cpp:820-827` — der Laderiegel prüft `0 <= start < end` der manuellen Passagen, aber keine Obergrenze; `start = INT64_MAX-2` ist ein zulässiger Ladewert. Unbewertet.
- `core/ipc/PipeToken.cpp:147-182` — `aktuelleLogonSid()` liefert bei fehlgeschlagener Token-Abfrage einen leeren String; `PluginProcessor.cpp:85-86/139` übernimmt ihn ungeprüft in Pipename und Mutexnamen. Unbewertet (der Adressriegel `core/ipc/ControlClient.cpp:487` weist eine leere SID auf dem Control-Link ab).
