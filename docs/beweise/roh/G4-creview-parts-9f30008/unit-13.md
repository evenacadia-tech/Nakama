# unit-13 — state/NakamaParameter.cpp, state/NakamaParameter.h, state/NakamaState.cpp, state/NakamaState.h, vertrag/NakamaEvidenz.cpp, vertrag/NakamaEvidenz.h, vertrag/NakamaTelemetrie.cpp (Teil)
Gelesen: 74 von 74 Einheiten vollständig · Zeilen: 2471
Repo `C:\Users\phili\Projekte\Nakama`, HEAD `9f3000881df632f4969d78551fb0a516b61288cb`, strikt lesend.

## Befunde

- **[MEDIUM]** `vertrag/NakamaEvidenz.cpp:78` · konvertierung/logik-protokoll · **`zahlJson` formatiert jede nicht-ganzzahlige Zahl des `evidence_snapshot` mit `std::to_string(double)` — und das ist auf MSVC gebietsschema-abhängig, also kann der Dezimaltrenner zum Komma werden.** · Auslöser: irgendein anderes Modul im FL-Studio-Prozess (fremdes Plugin, Host-DLL, eine Bibliothek in deren Startup) ruft `setlocale(LC_NUMERIC, …)` bzw. `setlocale(LC_ALL, "")` mit einem Komma-Gebietsschema. Der Bau setzt keine `MSVC_RUNTIME_LIBRARY`, also gilt CMakes Vorgabe `MultiThreaded…DLL` (/MD) und das C-Gebietsschema liegt gemeinsam in `ucrtbase.dll` — die Änderung wirkt sofort auch in unserem Modul. Belegt an der Quelle: MSVC 14.44 `include/string:487-492` implementiert `to_string(double)` als `sprintf_s(…, "%f", _Val)`; die ganzzahligen Überladungen (`:463-485`, `_Integral_to_string`) sind davon nicht betroffen, der ganzzahlige Zweig `NakamaEvidenz.cpp:77` ist also sicher. · Wirkung: zwei verschiedene Schäden aus derselben Ursache. (a) In Objektposition — `"abdeckung":0,87`, `"konvergenz":…`, `"staerke_mad":…`, `"band_zentrum_hz":…`, `"mono_folddown_db":…`, `"lr_balance_db":…` — wird der ganze Snapshot ungültiges JSON, der Broker verwirft ihn, die Evidenz dieser Sonde verschwindet (gezählt nur als `evidenzNichtGesendet`). (b) In Arrayposition — `"werte":[…]` der elf Stereobandsätze zu je 221 Werten (`:245-260`) und `"fenster_dauer_ms"` (`:286-295`) — bleibt der Text **gültiges** JSON, aber aus 221 Werten werden bis zu 442: aus `0,87` werden die zwei Elemente `0` und `87`. Das ist stille Messwertverfälschung, kein Parserfehler. · Erreichbar: ja. `SondeProcessor::evidenzSnapshotSenden` (`sonde/SondeProcessor.cpp:448`, Analyseworker) → `:557 evidenceSnapshotAlsJson` → `:508/510 zahlJson (frame.abdeckung/konvergenz)` bzw. → `:305-348 stereoJson` → `:259 zahlJson`. Das Bedrohungsmodell nennt „fremdes Plugin im selben Host" ausdrücklich als Fehlerquelle. · Beleg: `NakamaEvidenz.cpp:71-79`; Gegenprobe im selben Repo: `src/PluginProcessor.cpp:1752-1760` löst genau dasselbe Problem bereits richtig (`std::ostringstream` + `imbue(std::locale::classic())`), im Evidenzpfad fehlt dieser Riegel. · Register: neu (kein Treffer für `locale`/`to_string`/`NakamaEvidenz` in `docs/offene-punkte.md`).

- **[LOW]** `state/NakamaParameter.cpp:213` · integer/bounds · **`juce::String::fromUTF8 ((const char*) utf8, (int) laenge)` schneidet die `size_t`-Länge ungeprüft auf `int` zu; ab 2 GiB wird sie negativ und JUCE liest dann bis zum ersten NUL — über das Pufferende hinaus.** · Auslöser: ein DTO-Text ≥ 2^31 Byte an `ausDtoText (const void*, size_t, …)`. Der vorgeschaltete `vertrag::textriegelBytes` (`:197`) prüft die volle Länge und verbietet rohe NUL-Bytes, weshalb der bis-NUL-Lauf hinter dem Puffer weiterläuft statt vorher zu stoppen. · Wirkung: Lesen jenseits des Puffers (Heap-Overread, im schlechtesten Fall Absturz im Hostprozess); bei `laenge` zwischen 2^31 und 2^32 ohne Vorzeichenwechsel stattdessen stille Teilauswertung. · Erreichbar: **nein im heutigen Produkt** — `ausDtoText` hat außer `tests/StateMigrationTestMain.cpp:559/594` keinen Aufrufer; der Host-State-Weg (`NakamaState::lade`) ist bei 16 MiB gedeckelt. Es ist die Vertragslücke der Funktion für den Tag, an dem SONDE-015 sie an die v3-Grenze hängt. · Beleg: `:186-296`, insbesondere `:197` und `:213`. · Register: **NAK-78** (Punkt 3, 08-24 Gate G1: „`vertrag/NakamaVertrag.cpp:305` und `state/NakamaParameter.cpp:209`: unbewachtes `static_cast<int>(laenge)` im Textriegel" — dieselbe Anweisung, seither um vier Zeilen verschoben).

- **[LOW]** `state/NakamaState.cpp:838` · initialisation · **`leseSchema2` legt den Parametersatz mit `parameter::Satz satz {}` an und schreibt ihn auch dann nach `aus.parameters` (`:851`), wenn die Klasse kein `Parameters`-Kind führt — das sind lauter Nullen, nicht der neutrale Satz, den `NakamaState.h:133-144` an dieser Stelle ausdrücklich verlangt.** · Auslöser: jedes Laden eines Standes mit `plugin_kind` ≠ `active_probe` (also jeder Eqcp- und jeder Probeeq-Passivstand) über `LadeErgebnis::geladen`/`migriert`. · Wirkung: `Zustand::parameters` trägt danach `band.N.freq_hz = 0`, was außerhalb des Bereichs `20…20000` liegt. Solange `hatParameters` (`:850`) mitläuft, wird der Satz nie geschrieben; wer ihn ohne Nachfüllen setzt, erzeugt einen Stand, den `leseAusBaum`+`validiere` beim nächsten Laden als read-only zurückweisen — genau der Gegenpfad speichern↔laden, den der Header als am 23.08. gemessenen Fehler beschreibt. · Erreichbar: **nein heute** — kein Bundle führt gleichzeitig `active_probe` und eine andere Klasse (`NakamaState.h:118-120`), und `hatParameters` wird außerhalb von `lade()` nur im Konstruktor gesetzt (`sonde/SondeProcessor.cpp:97`). Die Zusicherung bricht in dem Moment, in dem ein Bundle zwei Klassen führt oder die Klasse zur Laufzeit wechselt. Härtung, kein heutiger Defekt. · Beleg: `:838`, `:850-851` gegen `state/NakamaState.h:133-144`. · Register: neu.

## Gate-5- und Invariantenbeleg

1. **Audio-Thread ohne Sperren/Allokationen/IO** — *hält*. Keine meiner Einheiten liegt im Audiopfad. `NakamaEvidenz` läuft im Analyseworker (`sonde/SondeProcessor.cpp:427` „Laeuft im Analyseworker unter `analyseSchloss` — nie im Audiothread", Aufruf `:408`), allokiert dort bewusst (`NakamaEvidenz.cpp:394 text.reserve(64*1024)`). `NakamaState::lade/speichere` laufen auf dem Nachrichten-/Hostthread; `NakamaParameter::stateHash` hängt am IPC-Callback `v3Status()` (`sonde/SondeProcessor.cpp:79/684`, `src/PluginProcessor.cpp:108/1115`) unter `zustandSchloss`, das laut `sonde/SondeProcessor.h:250` „nur Nachrichten-/Hostthread, nie processBlock" genommen wird — nachgeprüft: alle vier Nutzungen (`SondeProcessor.cpp:626/645/662/690`) liegen außerhalb von `processBlock` (`:155`). Überlast verwirft hier nur Analysefähiges: `NakamaEvidenz.cpp:456-478` zählt aussortierte Ereignisse in `verloren` hoch, statt sie zu verschweigen.
2. **Passthrough bitidentisch / Hörmarker** — *nicht berührt*. Keine meiner Einheiten fasst einen Hostpuffer an.
3. **Gate 5 (kein Frame steuert Gain / hörbares PRE-POST-Delta)** — *nicht berührt*. Meine Einheiten sind Serialisierer und State-Leser; der einzige Weg zu DSP-Werten (`parameter::Satz`) endet heute im `state_hash` und im Baum, nicht an einem Gain (`SONDE-015` existiert nicht).
4. **NaN-Ehrlichkeit** — *hält*. `NakamaParameter.cpp:121` verriegelt nicht-endliche Parameter vor Bereich und Enum; `dtoKanon:166` prüft vor jedem Hash. `NakamaEvidenz.cpp:253-257` übersetzt einen nicht-endlichen Bandwert in „0 ohne Bit" statt in eine gemeldete 0, `:294` und `:344-348` tun dasselbe für Fensterdauer und Skalare, `:451-455` wirft nicht-endliche Ereigniswerte **einzeln** heraus und zählt sie, `:529-530` trägt `samples_nicht_endlich` immer mit. `zahlJson:73-74` fängt NaN/±Inf **vor** der `long long`-Wandlung ab — der einzige verbleibende float→int-Pfad `:488 std::llround(dauerMs * sample_rate / 1000.0)` ist doppelt begrenzt (`dauerMs` endlich ≥ 0 aus `:455` und laut `core/analysis/FeatureEngine.h:2776` nur ein Fensterhop, `sample_rate` endlich in `(0, 768000]` aus `:162-164`).
5. **State bleibt verlustfrei** — *hält*. `ValueTreeByteRiegel` (`NakamaState.cpp:64-269`) prüft die Hostbytes vor `juce::ValueTree::readFromData` auf genau einen vollständigen Baum (16 MiB, Tiefe 64, 65536 Einträge je Sammlung, 262144 gesamt, `pos != ende` = ungültig) und meldet den nicht verlustfrei lesbaren Variantenmarker 9 getrennt (`:227-236`, `:80-82`). Unbekannte Root-/Kind-Majors und verletzte Matrizen fallen über `nurLesen()` (`:915-926`) auf die Originalbytes zurück, `speichere:985-989` gibt sie unverändert heraus; `nurLesen` wird außerhalb von `lade()` nirgends im Produktcode zurückgesetzt (geprüft repoweit). Gegenprobe zum Bytegate selbst: `pos <= ende` ist auf allen Pfaden gehalten (jeder Vorschub steht hinter `hat()` oder hinter einem geprüften `variantenEnde`), das Array-Substrom-`ende` wird in `:211/219` symmetrisch gesetzt und zurückgenommen, und `komprimierteZahl:114-122` schließt Wrap in beide Richtungen aus.

## Ledger

- `state\NakamaParameter.cpp:file-scope-1` · clean
- `state\NakamaParameter.cpp:14-20` · clean
- `state\NakamaParameter.cpp:22-28` · clean
- `state\NakamaParameter.cpp:30-36` · clean
- `state\NakamaParameter.cpp:38-72` · clean
- `state\NakamaParameter.cpp:76-80` · clean
- `state\NakamaParameter.cpp:82-89` · clean
- `state\NakamaParameter.cpp:91-107` · clean
- `state\NakamaParameter.cpp:109-131` · clean
- `state\NakamaParameter.cpp:138-159` · clean
- `state\NakamaParameter.cpp:163-175` · clean
- `state\NakamaParameter.cpp:177-184` · clean
- `state\NakamaParameter.cpp:186-296` · Befund 2
- `state\NakamaParameter.cpp:300-320` · clean
- `state\NakamaParameter.cpp:322-361` · clean
- `state\NakamaParameter.h:file-scope-1` · clean
- `state\NakamaParameter.h:36-97` · clean
- `state\NakamaState.cpp:file-scope-1` · clean
- `state\NakamaState.cpp:67-71` · clean
- `state\NakamaState.cpp:73-83` · clean
- `state\NakamaState.cpp:86-89` · clean
- `state\NakamaState.cpp:91-125` · clean
- `state\NakamaState.cpp:127-151` · clean
- `state\NakamaState.cpp:153-163` · clean
- `state\NakamaState.cpp:165-240` · clean
- `state\NakamaState.cpp:242-261` · clean
- `state\NakamaState.cpp:271-276` · clean
- `state\NakamaState.cpp:278-286` · clean
- `state\NakamaState.cpp:288-291` · clean
- `state\NakamaState.cpp:293-296` · clean
- `state\NakamaState.cpp:298-302` · clean
- `state\NakamaState.cpp:308-318` · clean
- `state\NakamaState.cpp:320-330` · clean
- `state\NakamaState.cpp:332-339` · clean
- `state\NakamaState.cpp:341-348` · clean
- `state\NakamaState.cpp:395-424` · clean
- `state\NakamaState.cpp:426-430` · clean
- `state\NakamaState.cpp:432-438` · clean
- `state\NakamaState.cpp:445-458` · clean
- `state\NakamaState.cpp:file-scope-2` · clean
- `state\NakamaState.cpp:463-564` · clean
- `state\NakamaState.cpp:574-653` · clean
- `state\NakamaState.cpp:656-788` · clean
- `state\NakamaState.cpp:789-857` · Befund 3
- `state\NakamaState.cpp:861-872` · clean
- `state\NakamaState.cpp:874-911` · clean
- `state\NakamaState.cpp:913-980` · clean
- `state\NakamaState.cpp:982-993` · clean
- `state\NakamaState.cpp:995-1005` · clean
- `state\NakamaState.cpp:1007-1014` · clean
- `state\NakamaState.cpp:1016-1021` · clean
- `state\NakamaState.h:file-scope-1` · clean
- `state\NakamaState.h:37-179` · clean
- `vertrag\NakamaEvidenz.cpp:file-scope-1` · clean
- `vertrag\NakamaEvidenz.cpp:28-60` · clean
- `vertrag\NakamaEvidenz.cpp:62-62` · clean
- `vertrag\NakamaEvidenz.cpp:71-79` · Befund 1
- `vertrag\NakamaEvidenz.cpp:81-90` · clean
- `vertrag\NakamaEvidenz.cpp:92-102` · clean
- `vertrag\NakamaEvidenz.cpp:107-130` · clean
- `vertrag\NakamaEvidenz.cpp:132-147` · clean
- `vertrag\NakamaEvidenz.cpp:157-215` · clean
- `vertrag\NakamaEvidenz.cpp:217-220` · clean
- `vertrag\NakamaEvidenz.cpp:235-265` · clean
- `vertrag\NakamaEvidenz.cpp:278-351` · clean
- `vertrag\NakamaEvidenz.cpp:355-434` · clean
- `vertrag\NakamaEvidenz.cpp:435-535` · clean
- `vertrag\NakamaEvidenz.h:file-scope-1` · clean
- `vertrag\NakamaEvidenz.h:36-112` · clean
- `vertrag\NakamaTelemetrie.cpp:file-scope-1` · clean
- `vertrag\NakamaTelemetrie.cpp:15-18` · clean
- `vertrag\NakamaTelemetrie.cpp:20-27` · clean
- `vertrag\NakamaTelemetrie.cpp:58-110` · clean
- `vertrag\NakamaTelemetrie.cpp:118-131` · clean

## Nicht gelesen

keine.

## Außerhalb meiner Einheiten

- `eq-copilot/plugin/core/ipc/ControlClient.cpp:374` — dieselbe gebietsschema-abhängige `std::to_string(double)`-Zeile wie Befund 1, hier im Zahlenformatierer des Controllinks; repoweit sind es genau diese zwei Stellen.
- `eq-copilot/plugin/CMakeLists.txt` (und `eq-copilot/CMakeLists.txt`) — keine `MSVC_RUNTIME_LIBRARY`-Vorgabe, es gilt CMakes `MultiThreaded$<$<CONFIG:Debug>:Debug>DLL`; damit teilen Plugin, Host und Fremdplugins den C-Gebietsschema-Zustand in `ucrtbase.dll`. Das ist die Voraussetzung für Befund 1.
- `eq-copilot/plugin/core/analysis/FeatureEngine.h:3704-3716` — `cycle_start_ppq_gesetzt` wird nur zusammen mit `isfinite` gesetzt; die zusätzliche `isfinite`-Prüfung in `NakamaEvidenz.cpp:192-193` ist dadurch heute unerreichbar (Notiz, damit sie beim nächsten Umbau nicht für die tragende Hälfte gehalten wird).

## Belege aus der Sitzung

- `git status --short` → leer.
- `git rev-parse HEAD` → `9f3000881df632f4969d78551fb0a516b61288cb`.
- Externe Verifikation für Befund 1 an der Primärquelle statt aus dem Gedächtnis: `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.44.35207\include\string:487-492`.
