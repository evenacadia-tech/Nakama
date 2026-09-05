# unit-09 — src\Diagnose.h · src\EqCopilotAssetKit.h · src\EqCopilotIds.h · src\HoerMarkierung.h · src\LeitstandTokens.h · src\PipeClient.cpp · src\PipeClient.h · src\PluginEditor.cpp
Gelesen: 71 von 71 Einheiten vollständig · Zeilen: 2542

## Befunde

- [HIGH] `src/HoerMarkierung.h:548-560` · konvertierung-ub / dsp-invariante-4 · **Ein einziges nicht-endliches Eingangssample vergiftet den Biquad-Zustand der Hör-Markierung dauerhaft; der Ausgang bleibt NaN, solange die Markierung eingeschaltet ist.** · Auslöser: ein Sample mit `±Inf` oder `NaN` im Hostpuffer (feindliches oder defektes Vorgänger-Plugin im selben FL-Kanal — im Bedrohungsmodell ausdrücklich als Eingang geführt), während SOLO oder PULS klingt. In `tdf2Lauf` gilt dann `y = b0·x + s1 = ±Inf` und in derselben Iteration `s1 = b1·x − a1·y + s2 = Inf − Inf = NaN` (`:554-556`); ab da liefert jede weitere Iteration NaN, unabhängig vom Eingang. · Wirkung: die Mischzeile `aus[i] = aus[i] + f·(w[i] − aus[i])` (`:468`) schreibt NaN in den **Hostpuffer** — das einzige Audio-Schreibrecht des Produkts. Der Zustand wird nur von `resetZustaende()` geräumt, und das läuft ausschließlich bei `fade <= 0 && ! zielAn` (`:518-522`) bzw. `hartAus()` — solange der User die Markierung engagiert lässt, bleibt der Masterweg tot. Weder verriegelt noch gezählt: `nanSeen` (`PluginProcessor.cpp:255-265`) zählt nur den **Eingang**, der Wet-Pfad hat keinen eigenen Riegel. · Erreichbar: ja — `EqCopilotProcessor::processBlock` → `PluginProcessor.cpp:545` `markierung.verarbeite (buffer, kanaele, erlaubt)` → `HoerMarkierung.h:404-436` (`tdf2Lauf` je Kanal/Sektion) → Mischschleife `:456-474`. Voraussetzung ist die useraktivierte Markierung plus die vier fail-closed Terme; keine Speicherunsicherheit, die Wirkung ist anhaltende Audiokorruption. · Beleg: `:412`, `:429`, `:468`, `:518-522`, `:548-560`; kein Testbein deckt es (`tests/MarkierungTestMain.cpp:834-855` speist ausschließlich einen endlichen Sinus und prüft `isfinite` nur am Ausgang). · Register: neu (kein Treffer für `tdf2Lauf`, `Filterzustand`, `nicht-endlich` in `docs/offene-punkte.md`)

- [MEDIUM] `src/HoerMarkierung.h:252-264` (und gleichlautend `:533-539`) · logik-protokoll / zustandsautomat · **`vorbereiten()` und `setzeSamplerate()` setzen `warHoerbar` zurück, ohne das fällige `endete` zu melden — ein Interventionsintervall bleibt beim Broker dauerhaft offen.** · Auslöser: ein `prepareToPlay` des Hosts (Blockgrößen- oder Sampleratewechsel in den FL-Audioeinstellungen, auch ein Render-Vorlauf), während die Markierung hörbar ist. `prepareToPlay` ruft beide Vorbereiter (`PluginProcessor.cpp:206-207`); `vorbereiten` setzt über `hartAus()` `fade = 0` und danach `warHoerbar = false; hoerbareSamples = 0` (`:262-263`), `setzeSamplerate` dasselbe (`:537-538`). Der Übergang `! istHoerbar && warHoerbar` (`:500-515`), der `Schritt::endete` erzeugt, kann danach nicht mehr feuern. · Wirkung: das zum vorangegangenen `begann` gehörende `intervention_begin` bekommt nie sein `intervention_end`. Der Broker hält den Eintrag in `taint.interventionen` (`broker/src/coordinator/intervention.rs:146-154`, Ende nur über `:221`); die Sitzung bleibt getaintet, und nach 64 solcher Wechsel (`MAX_AKTIVE_INTERVENTIONEN`, `broker/src/coordinator/mod.rs:76`) kippt sie auf `taint.unknown = true`, das laut `:214-219` nur `neutral_resync` wieder löst. Richtung ist fail-closed (der Bereich bleibt in Quarantäne), der Endzustand aber ein dauerhaft blockiertes Messaggregat. · Erreichbar: ja — Host → `EqCopilotProcessor::prepareToPlay` (`PluginProcessor.cpp:180`, `:206-207`) → `HoerMarkierungDsp::setzeSamplerate`/`vorbereiten`; die Gegenseite ist `PluginProcessor.cpp:563-615` (`interventionsRing.schreibe`). · Beleg: `:262-263`, `:500-515`, `:537-538`. · Register: neu (`warHoerbar` und `prepareToPlay`+`endete` haben keinen Eintrag in `docs/offene-punkte.md`)

- [LOW] `src/HoerMarkierung.h:252-256` · return-values / alloc-lifetime · **`wet.calloc()` wird ungeprüft aufgerufen, `wetKapazitaet` steht aber schon auf dem Sollwert — bei fehlgeschlagener Allokation behauptet das Objekt eine Kapazität, die es nicht hat.** · Auslöser: Speicherdruck (ein lokaler, nicht privilegierter Prozess, der Commit erschöpft) genau während `prepareToPlay`. `juce::HeapBlock<float>` hat `throwOnFailure = false` (`build/_deps/juce-src/modules/juce_core/memory/juce_HeapBlock.h:99`, `:348`), liefert also `nullptr` statt zu werfen; `wetKapazitaet = std::max (maxBlock, 16)` steht in Zeile `:254` **vor** der Allokation und wird nicht zurückgenommen. · Wirkung: `oversize = n > wetKapazitaet` bleibt falsch, und `std::memcpy (wetK[k], puffer.getReadPointer (k), nutzbar·4)` (`:405`) schreibt nach `nullptr` (bzw. `nullptr + wetKapazitaet` für Kanal 1, schon die Zeigerarithmetik ist UB) — Absturz des Hostprozesses im Audiothread. Kein Überschreiben fremder Daten. · Erreichbar: ja, aber nur zusammen mit engagierter Markierung (`zielAn` oder `fade > 0`, sonst kehrt `verarbeite` vor `:405` zurück): `prepareToPlay` (`PluginProcessor.cpp:207`) → `vorbereiten` → später `processBlock` → `verarbeite:405`. · Beleg: `:254-255`, `:339`, `:405`. · Register: neu

- [LOW] `src/PipeClient.cpp:249-261` · logik-protokoll · **`stop()` während der Öffnungsschleife lässt den Snapshot auf `Status::verbindet` stehen** — an der Quelle bestätigt: `eineVerbindung` kehrt bei `sollAbbrechen` in `:373-374` zurück, ohne `zustand.status` zu setzen, während `:346` ihn zuvor auf `verbindet` gestellt hat. Auslöser: `stop()`/`~PipeClient()` im Fenster zwischen `:346` und dem ersten erfolgreichen `CreateFileW`. Wirkung: der Editor zeigt dauerhaft „verbindet" für einen gestoppten Client. Erreichbar: ja (`PluginProcessor.cpp:170` `pipe.stop()` im Prozessordestruktor). Beleg: `:249-261`, `:342-358`, `:371-374`. · Register: **NAK-144** (bereits datiert geführt, bewusst nicht in R1 gefixt) — kein neuer Punkt.

- [LOW] `src/PipeClient.cpp:432` · logik-protokoll · **`++zustand.serverPruefungen` steht positionsgesteuert hinter dem erfolgreichen `CreateFileW`**, während die v3-Pfade statusgesteuert zählen; derselbe Feldname trägt damit zwei Bedeutungen. An der Quelle bestätigt (Zeilendrift gegenüber dem Registereintrag: dort `:418`, am HEAD `:432`). Auslöser: `ACCESS_DENIED` beim Öffnen. Wirkung: Diagnosezahl, kein Sicherheitsurteil. Erreichbar: ja. · Register: **NAK-143** — kein neuer Punkt.

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren/Allokation/IO/Logging — hält.** `HoerMarkierungDsp::verarbeite` (`HoerMarkierung.h:320-528`) nimmt keine Sperre, allokiert nicht (`wet` liegt aus `vorbereiten:255` vor), macht keine Datei-/Pipe-/Netzzugriffe und loggt nicht; die einzigen teuren Operationen sind `std::memcpy` (`:405`) und die POD-Kopie `lokal = ring[...]` (`:365`, ~4,8 KB, nur bei neuer Publikation). Der PipeClient berührt den Audiothread nicht — alle I/O läuft in `threadLauf` (`PipeClient.cpp:288-336`).
2. **Passthrough bitidentisch / kein Hostpuffer angefasst — hält.** Drei Rückkehrpunkte vor jedem Schreibzugriff: `n <= 0 || kanaele <= 0` (`:324-325`), Oversize mit `fade <= 0` (`:340-349`) und `! zielAn && fade <= 0` (`:379-399`). `kanaele` ist beim Aufrufer auf `jmin (buffer.getNumChannels(), 2)` gedeckelt (`PluginProcessor.cpp:237`), `ch = std::min (kanaele, 2)` (`:402`) bleibt damit innerhalb der Kanalzahl und innerhalb von `zust[2][4]`/`pulsZust[2]`. `nutzbar <= wetKapazitaet` und `nutzbar <= n` — kein Überlauf in `memcpy` (`:405`) oder in der Mischschleife (`:462-470`).
3. **Gate 5 — hält.** In meinen Einheiten steuert kein Telemetrie- oder Steuerframe Gain. Die einzigen Eingänge von `verarbeite` sind der Publikationsring (Message-Thread, ausgelöst durch einen Klick des Users auf einer Befundkarte, `PluginEditor.cpp:763-800`) und `erlaubt`. `PipeClient` schreibt ausschließlich in `zustand` (`PipeClient.cpp:342-635`) und hat keinen Pfad zu Audio.
4. **NaN-Ehrlichkeit — bricht.** Siehe Befund 1: der Wet-Pfad hat keinen Riegel und keinen Zähler für nicht-endliche Zwischenwerte (`HoerMarkierung.h:548-560`). Die Nyquist-Kappe selbst hält: `nyKappe = 0,475·fs` (`:169`) deckelt `bis` (`:171`) und `f0` (`:188`, `:209`); die Koeffizienten laufen zusätzlich durch `biquadStabil` (`:59-65`, angewandt `:237-243`), sodass ein nicht-endlicher **Auftrag** nie eingereicht wird. Der Bruch liegt allein im laufenden Zustand.
5. **State verlustfrei — nicht berührt.** Keine meiner Einheiten liest oder schreibt persistenten Zustand; `MarkierungsAuftrag` wird nicht persistiert (einziger Erzeuger `baueMarkierungsAuftrag`, einziger Einreicher `PluginProcessor.h:405`).

Zwei Vertragsbeobachtungen ohne heutigen Eingabepfad (deshalb bewusst **kein** Befund):
- `verarbeite` prüft `lokal.sektionen > 0` (`:376`), aber keine Obergrenze; `lokal.statisch[s]` und `zust[k][s]` (`:412`) sind `std::array` mit ungeprüftem `operator[]` über 4 Plätze. Heute unerreichbar: einziger Erzeuger ist `baueMarkierungsAuftrag` (Werte 1/2/4), und der einzige Aufrufer prüft den Rückgabewert (`PluginEditor.cpp:793`).
- `baueMarkierungsAuftrag` schreibt `ziel` auf den beiden frühen `return false` (`:167`, `:173`) **nicht**; auf den späteren (`:221`, `:239`, `:243`) bleibt ein halbfertiger Auftrag stehen. Der einzige Aufrufer default-konstruiert (alle Felder haben NSDMIs) und wertet den Rückgabewert aus, also liest niemand Uninitialisiertes.

## Ledger

- `src\Diagnose.h:48-83` · clean
- `src\EqCopilotAssetKit.h:file-scope-1` · clean
- `src\EqCopilotAssetKit.h:18-147` · clean
- `src\EqCopilotAssetKit.h:148-297` · clean
- `src\EqCopilotAssetKit.h:298-417` · clean
- `src\EqCopilotAssetKit.h:418-475` · clean
- `src\EqCopilotIds.h:file-scope-1` · clean
- `src\EqCopilotIds.h:6-26` · clean
- `src\HoerMarkierung.h:file-scope-1` · Befund 3 (`wet.calloc` in `vorbereiten`, Bereich [246,266])
- `src\HoerMarkierung.h:53-57` · clean
- `src\HoerMarkierung.h:59-65` · clean
- `src\HoerMarkierung.h:67-83` · clean
- `src\HoerMarkierung.h:85-99` · clean
- `src\HoerMarkierung.h:101-114` · clean
- `src\HoerMarkierung.h:116-129` · clean
- `src\HoerMarkierung.h:157-245` · clean
- `src\HoerMarkierung.h:267-274` · clean
- `src\HoerMarkierung.h:275-278` · clean
- `src\HoerMarkierung.h:289-303` · clean
- `src\HoerMarkierung.h:320-455` · Befund 1 (Mischzeile `:468` liegt in Teil 2, Ursache `tdf2Lauf`)
- `src\HoerMarkierung.h:456-528` · Befund 1, Befund 2 (fehlendes `endete`, Gegenstelle `:500-515`)
- `src\HoerMarkierung.h:533-539` · Befund 2
- `src\HoerMarkierung.h:548-560` · Befund 1
- `src\HoerMarkierung.h:file-scope-2` · clean
- `src\HoerMarkierung.h:586-586` · clean
- `src\LeitstandTokens.h:file-scope-1` · clean
- `src\LeitstandTokens.h:7-101` · clean
- `src\PipeClient.cpp:file-scope-1` · clean
- `src\PipeClient.cpp:22-22` · clean
- `src\PipeClient.cpp:23-23` · clean
- `src\PipeClient.cpp:24-24` · clean
- `src\PipeClient.cpp:25-25` · clean
- `src\PipeClient.cpp:26-26` · clean
- `src\PipeClient.cpp:31-40` · clean
- `src\PipeClient.cpp:42-70` · clean
- `src\PipeClient.cpp:72-96` · clean
- `src\PipeClient.cpp:98-103` · clean
- `src\PipeClient.cpp:105-113` · clean
- `src\PipeClient.cpp:115-120` · clean
- `src\PipeClient.cpp:124-147` · clean
- `src\PipeClient.cpp:149-209` · clean
- `src\PipeClient.cpp:212-225` · clean
- `src\PipeClient.cpp:227-227` · clean
- `src\PipeClient.cpp:229-247` · clean
- `src\PipeClient.cpp:249-261` · Befund 4 (NAK-144)
- `src\PipeClient.cpp:263-280` · clean
- `src\PipeClient.cpp:282-286` · clean
- `src\PipeClient.cpp:288-336` · clean
- `src\PipeClient.cpp:340-467` · Befund 5 (NAK-143, `:432`)
- `src\PipeClient.cpp:468-617` · clean
- `src\PipeClient.cpp:618-637` · clean
- `src\PipeClient.cpp:640-656` · clean
- `src\PipeClient.cpp:658-698` · clean
- `src\PipeClient.cpp:700-810` · clean
- `src\PipeClient.cpp:812-815` · clean
- `src\PipeClient.cpp:817-822` · clean
- `src\PipeClient.h:file-scope-1` · clean
- `src\PipeClient.h:file-scope-2` · clean
- `src\PluginEditor.cpp:file-scope-1` · clean
- `src\PluginEditor.cpp:12-17` · clean
- `src\PluginEditor.cpp:19-19` · clean
- `src\PluginEditor.cpp:22-28` · clean
- `src\PluginEditor.cpp:29-29` · clean
- `src\PluginEditor.cpp:30-30` · clean
- `src\PluginEditor.cpp:31-31` · clean
- `src\PluginEditor.cpp:33-43` · clean
- `src\PluginEditor.cpp:45-61` · clean
- `src\PluginEditor.cpp:63-67` · clean
- `src\PluginEditor.cpp:69-76` · clean
- `src\PluginEditor.cpp:78-85` · clean
- `src\PluginEditor.cpp:87-231` · clean

Geprüfte Negativbefunde, die ich ausdrücklich nicht melde (jeweils an der Quelle entkräftet):
- `PipeClient.cpp:673` `HeapBlock<char> puffer (len + 1)` — kein Überlauf, `len` ist zuvor auf `kMaxFrameBytes = 262144` gedeckelt (`:667`, `EqCopilotIds.h:21`); NUL-, BOM- und UTF-8-Prüfung liegen vor der Terminierung (`:677-696`).
- `PipeClient.cpp:42-70` `ganzzahl64` — der `double`→`int64`-Cast ist vollständig verriegelt (`isfinite`, `trunc(d)==d`, `>= (double) INT64_MIN` exakt darstellbar, `< 2^63`).
- `PipeClient.cpp:700-810` `ioGenau` — keine Handle-Lecks und kein Cancel auf einem recycelten HANDLE: Submission, `CancelIoEx` und `CloseHandle` teilen `handleMutex` (`:730`, `:619-623`, `:819`); jeder Pfad wartet vor der Rückkehr die Completion ab (`:770-773`). Sperrreihenfolge ist überall `zustandMutex → handleMutex`, nie umgekehrt.
- `PipeClient.cpp:340-637` — jeder Ausstieg aus dem `do{}while(false)` ist ein `break`; `h` wird genau einmal geschlossen (`:461` oder `:622`).
- `HoerMarkierung.h:203/217/218/234` `std::lround` (auf MSVC 32-bit `long`) — kein Überlauf: `fs` ist beim Aufrufer auf `(0, 768000]` geklemmt (`PluginProcessor.cpp:182-184`), und `chunks > kPulsMaxChunks` fängt alles über 262 kHz ab (`:219-221`).
- `EqCopilotAssetKit.h:375` `roundToInt (jlimit (…, normalisiert))` mit NaN — nicht erreichbar: der Aufrufer setzt `gueltig = false`, sobald der Wert nicht endlich ist (`PluginEditor.cpp:1370-1379`), und `aktiv` nimmt dann den 0-Zweig.
- `PluginEditor.cpp:233-240` — `setLookAndFeel (nullptr)`, `markierungAus()` und `setzeEditorOffen (false)` schließen die im Konstruktor (`:90`, `:216`) geöffneten Paare.

## Nicht gelesen

keine

## Außerhalb meiner Einheiten

- `eq-copilot/plugin/src/PluginProcessor.cpp:511` — `(std::size_t) (buffer.getNumSamples() * kanaele)` rechnet das Produkt in `int`, bevor es nach `size_t` geht; unbewertet.
- `eq-copilot/plugin/src/PluginProcessor.cpp:210` — `versuchTrocken.assign ((std::size_t) std::max (1, maxBlock) * 2u, 0.0f)` wirft bei sehr großem `maxBlock` aus `prepareToPlay` heraus in den Host; unbewertet.
- `eq-copilot/plugin/src/PluginProcessor.cpp:255-265` — der NaN-Befund des Eingangs wird gezählt (`nanSeen`), der Puffer aber nicht bereinigt; unbewertet (Kontext zu Befund 1).
- `eq-copilot/plugin/src/PluginProcessor.cpp:577-583` — `interventionsSequenz`/`interventionsNummer` als `relaxed` `fetch_add`, Paarbildung Begin/End über `load() - 1`; unbewertet (Kontext zu Befund 2).
