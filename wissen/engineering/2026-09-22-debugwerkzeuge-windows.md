---
id: w-2026-09-22-debugwerkzeuge-windows
typ: wissenseintrag
status: aktiv
datum: 2026-09-22
aktualisiert: 2026-09-22
hauptbereich: engineering
weitere_bereiche: [fl-studio]
pruefstand: quellen-geprueft
---

# Debugwerkzeuge für Nakama unter Windows: Fehlerorte zeigen statt erschließen

## Menschliches Warum

- Mensch und Situation: Der Projektleiter ist Musiker, kein Programmierer.
  Gebaut, gemessen und geprüft wird von KI-Agenten über die Kommandozeile.
- Problem: Fehler zeigen sich als stille Hänger (33 bis 45 Minuten bis zum
  Zeitlimit, NAK-241, NAK-290), als Abstürze, deren Speicherabbilder niemand
  liest (NAK-353), und als Nebenläufigkeitsfehler, die nur beim Codelesen
  auffallen (NAK-184, NAK-246 D1 und D2, NAK-159). Jede Ursache wird aus Code
  erschlossen; das kostet Prüfrunden und lässt Klassen durch, die kein Test
  trifft.
- Bedarf: Werkzeuge, die den Ort eines Fehlers direkt zeigen und die ein
  Agent ohne Bildschirm bedienen kann.
- Wirkung: weniger Abstürze und Aussetzer beim Musiker, weniger
  Nacharbeitsrunden im Bau.

> Weil ein Musiker in einer langen Session weder einen Absturz noch einen
> Aussetzer hinnimmt und der Bau ohne menschlichen Programmierer läuft,
> braucht Nakama Werkzeuge, die Absturzort, Warteort, Datenrennen und
> Echtzeitverstoß maschinenlesbar zeigen, damit Agenten Ursachen messen statt
> vermuten.

## Belegtes Wissen

Stand 22.09.2026. Web-Recherche in sechs Strängen an Primärquellen, dazu
lokale Versuche in einem Arbeitsordner außerhalb des Repos (clang-tidy 19.1.5
und cl 19.44 der Build Tools 2022; kein Produktcode geändert).

### Ausgangslage (am Repo gemessen)

- Kein Debugger installiert (weder WinDbg noch cdb noch ProcDump oder TTD);
  kein Sanitizer im Bau; das Release-Plugin und die C++-Tests entstehen ohne
  Symboldateien (CMake-Release: `/O2 /Ob2 /DNDEBUG`, kein `/Zi`). Rust legt
  auch im Release eine PDB an (rustc übergibt `/DEBUG` unter windows-msvc).
- clang-tidy 19.1.5 aus den Build Tools läuft als Ratsche (Bein A33).
- Die Echtzeitwache `RtWache` (`eq-copilot/plugin/dsp/DspRtWache.h`) zählt
  Allokationen nur über ein vom Test ersetztes `operator new` und Sperren nur
  über den eigenen Sperrwrapper; `malloc` in JUCE und Sperren in JUCE, CRT
  und Windows sieht sie nicht (NAK-361 H-1).
- pluginval lief zuletzt Ende August 2026 (Manifest SONDE-007c), mehrfach
  mit `--skip-gui-tests` (NAK-26).
- Installiert und ungenutzt: cargo-nextest 0.9.133, cargo-fuzz, `appverif.exe`,
  `wpr.exe`. Der WSL-Dienst ist deaktiviert.

### Absturz- und Hängerdiagnose

- WinDbg bringt seit Version 1.2506.12002.0 die Konsolendebugger mit
  (`cdbX64.exe`, `kdX64.exe`, `ntsdX64.exe`); Installation
  `winget install --id Microsoft.WinDbg -e`. Das klassische `cdb.exe` gibt es
  weiter im Windows-SDK-Feature „Debugging Tools for Windows".
- Einzeiler, die ein Agent ohne Sitzung fahren kann:
  - Dump lesen: `cdbX64 -z <dmp> -y "<pdb-ordner>;srv*C:\Symbole*https://msdl.microsoft.com/download/symbols" -lines -c "!analyze -v; .ecxr; kn; ~*kn; q"`
  - Stapel eines hängenden Prozesses, ohne ihn zu beenden:
    `cdbX64 -pv -p <pid> -c "~*kn; qd"` (`-pv` hält die Threads nur an und
    liest; `qd` löst, `q` würde beenden).
- ProcDump v12.01 (Sysinternals, 09.07.2026; enthalten in
  `Microsoft.Sysinternals.Suite`): `-e` Dump bei unbehandelter Ausnahme,
  `-e 1` auch bei erster Chance, `-t` Dump beim Prozessende, `-h` bei
  hängendem Fenster, `-ma` voller Dump, `-w` auf Programmstart warten. Für das
  Selbstbeenden von FL (NAK-352): `procdump -accepteula -ma -e -t -w FL64.exe <ordner>`.
  `procdump -i` als nachträglicher Debugger schaltet die lokalen WER-Dumps ab.
- WER LocalDumps ist auf diesem Rechner aktiv; Vorgabe: Minidump, höchstens
  zehn Dateien je Ordner, der älteste wird ersetzt. Eigene Ablage je Programm
  unter `HKLM\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\<exe>`
  (Administratorrechte).
- `0xC0000409` mit Parameter 7 ist `__fastfail(FAST_FAIL_FATAL_APP_EXIT)`,
  der Weg von `std::process::abort` in Rust unter windows-msvc. Rust-Panics
  laufen unter MSVC als C++-Ausnahme (`_CxxThrowException`).
- Symboldateien für Release-C++: `/Zi` ändert die Optimierung nicht; der
  Linker braucht `/DEBUG` und ausdrücklich `/OPT:REF /OPT:ICF /INCREMENTAL:NO`,
  weil `/DEBUG` sonst deren Vorgaben umschaltet. Nicht auf `RelWithDebInfo`
  wechseln (dort `/Ob1` und inkrementelles Linken). Archiv je installiertem
  Bau: `symstore add /r /f <ausgabe>\*.* /s <ablage> /t Nakama /v <sha>`.
- MCP-Brücke: `svnscha/mcp-windbg` v1.3.0 (09.09.2026, MIT, aktiv gepflegt)
  öffnet Dumps und verbindet sich mit einem laufenden `cdb -server`; kein
  Anhängen über die Prozess-ID, keine TTD-Unterstützung, eine blockierende
  Anweisung ist nicht unterbrechbar (Issue #109). Es gibt keinen offiziellen
  WinDbg-MCP-Server von Microsoft.
- LLDB (MCP seit LLDB 21, `lldb-mcp` seit 22.1.0) und DAP-Brücken
  (mcp-debugger, Microsofts DebugMCP) lesen MSVC-PDBs nur teilweise;
  Microsofts eigener Debugger `vsdbg` ist lizenzrechtlich an Visual Studio
  und VS Code gebunden.

### Hängende und wackelnde Rust-Tests

- cargo-nextest (aktuell 0.9.146 vom 21.09.2026) führt jeden Test in einem
  eigenen Prozess aus. `slow-timeout = { period = "60s", terminate-after = 2 }`
  beendet einen hängenden Test nach 120 s; unter Windows über ein Job-Objekt
  samt allen Nachkommen, ohne Gnadenfrist. `retries` mit `flaky-result`
  kennzeichnet wackelnde Tests; JUnit-Ausgabe ist eingebaut. Exitcode 100
  heißt Testfehler einschließlich Zeitlimit.
- Anpassung im Broker: `broker/tests/briefkasten.rs` serialisiert seine Tests
  über den prozessinternen Mutex `NACHEINANDER`; unter nextest braucht das
  eine Testgruppe. `NEXTEST_RUN_ID` kann Probe-Pipenamen je Lauf eindeutig
  machen (Kollisionsursache von NAK-290).

### Statische Prüfungen

- Clang Thread Safety Analysis läuft in clang-tidy 19.1.5 über
  `--extra-arg=-Wthread-safety` beziehungsweise `ExtraArgs` im `.clang-tidy`,
  Check `clang-diagnostic-thread-safety-analysis`; im Versuch gemeldet:
  „writing variable 'generation_' requires holding mutex 'mu_' exclusively".
  Die MSVC-STL trägt keine Annotationen; nötig ist ein annotierter Wrapper um
  `std::mutex` mit Scoped-Lock. Das Muster der verlorenen Weckung wird auch
  bei einem atomaren Zähler gefangen, wenn er als `GUARDED_BY` markiert ist.
  Auf drei unannotierten Produktdateien 0 Meldungen.
- `[[clang::nonblocking]]` mit `-Wfunction-effects` prüft ab Clang 20 den
  Audiopfad statisch: Allokation, Sperre, Ausnahme, statische und
  thread-lokale Variablen, Aufrufe ohne sichtbare Definition. Clang 19 kennt
  das Attribut, prüft aber keinen Rumpf. LLVM 23.1.2 erschien am 22.09.2026
  (Windows-Installer und portables Archiv). Die MSVC-STL 14.44 nimmt Clang ab
  19; ob ein neueres clang-tidy über JUCE sauber läuft, ist ungeprüft.
  `RtWache` selbst nutzt statische und thread-lokale Zähler und bräuchte den
  dokumentierten Ausnahme-Makro.
- MSVC-Codeanalyse C6262 (`cl /analyze:only /analyze:stacksize N`) meldet
  große Stapelrahmen je Funktion; im Probelauf 71 s über alle 86 Einheiten mit
  acht Prozessen. Die Werte sind Schätzungen. MSVC-ConcurrencyCheck versteht
  `std::mutex` ohne Wrapper, meldet ohne SAL-Annotationen aber vor allem
  Fehlalarme. `-Wframe-larger-than` wirkt nur bei echter Codeerzeugung, nicht
  in clang-tidy; `unix.BlockInCriticalSection` sieht unter Windows fast nichts.

### Speicherfehler und Fuzzing

- MSVC AddressSanitizer: die Laufzeit liegt mit den Build Tools 17.14 schon
  vor (`clang_rt.asan_dynamic-x86_64.dll` im Toolset-Ordner). Schalter
  `/fsanitize=address /Zi`, Linker `/DEBUG /INCREMENTAL:NO`; unverträglich mit
  `/RTC*`, `/ZI` und inkrementellem Linken; alle Einheiten gleich bauen, sonst
  `LNK2038 annotate_vector`. `ASAN_OPTIONS=continue_on_error=1` samt
  `COE_LOG_FILE` sammelt alle Fehler statt beim ersten zu enden;
  `ASAN_SAVE_DUMPS=<name>.dmp` schreibt einen Dump. Richtwert doppelte
  Laufzeit.
- Ein ASan-Plugin in einem nicht instrumentierten Host (FL64.exe,
  pluginval.exe) ist laut Microsoft der häufigste Fall teilweiser
  Instrumentierung: es läuft, erkennt aber nicht alles; seit 17.7 auch für
  `/MT`-DLLs. Bedingungen: der Host muss die Laufzeit-DLL finden (sie liegt
  nie in System32), das Modul darf vor Prozessende nicht entladen werden,
  Ausgabe über `COE_LOG_FILE`. pluginval lässt sich mit
  `-DWITH_ADDRESS_SANITIZER=ON` selbst instrumentieren.
- Vorbehalt für Nakamas Tests: mehrere Testprogramme ersetzen `operator new`
  für die Allokationszähler. Eine eigene Überschreibung hat Vorrang vor der
  von ASan; ASan übersieht dann Fehler wie `heap-use-after-free` oder
  `double-free` an so angelegtem Speicher. Im ASan-Bau gehört der Zähler
  abgeschaltet oder auf die ASan-Schnittstelle umgelenkt.
- Mit cdb unter ASan erste-Chance-Zugriffsverletzungen nicht anhalten
  (`sxd av`): ASan legt seinen Schattenspeicher über solche Seitenfehler an.
- libFuzzer unter MSVC (`/fsanitize=fuzzer`) ist offiziell experimentell.
  cargo-fuzz läuft unter windows-msvc laut Nutzerbericht (Issue #450) mit
  Nightly und immer eingeschaltetem ASan; die README nennt Windows weiterhin
  als nicht unterstützt.

### Datenrennen

- Unter Windows gibt es 2026 keinen dynamischen Race-Detektor für natives
  C++: ThreadSanitizer unterstützt Windows nicht (LLVM 23.1.2), MSVC kennt
  kein `/fsanitize=thread`, Intel Inspector endete mit 2024.2, DRace ist
  archiviert.
- ThreadSanitizer unter Linux ist der Weg für portablen Code:
  `eq-copilot/plugin/core/ipc/IpcQueues.h` nutzt nur Standardköpfe,
  `HoerMarkierung.h` nur `juce_audio_basics`. JUCE 8.0.9 baut solche
  Konsolentests unter Linux ohne Bildschirm, braucht zum Konfigurieren aber
  die Entwicklungspakete der Grafikmodule (juceaide). Wege: GitHub-Runner
  `ubuntu-26.04` mit Clang 20 bis 22 (kein Administratorschritt, verbraucht
  Actions-Minuten des Kontos) oder WSL nach einmaliger Aktivierung durch einen
  Administrator.
- ThreadSanitizer meldet keine verlorene Weckung, wenn das Flag atomar ist:
  das ist kein Datenrennen. Diese Klasse fängt die Thread Safety Analysis oder
  ein Modellprüfer (Relacy, gepflegt, MSVC ungetestet).

### Echtzeit

- RealtimeSanitizer unterstützt Windows nicht („does not yet support
  Windows"); rtcheck und die rtcheck-Prüfung in pluginval laufen nur unter
  macOS beziehungsweise Linux. pluginvals Allokationstest (Stufe 9) ersetzt
  `operator new` nur in pluginval.exe; Allokationen in der Plugin-DLL sieht er
  unter MSVC nicht (Schluss aus der statischen Bindung, nicht gemessen).
- Eine Windows-eigene Falle ist mit Microsoft Detours (MIT, Commits bis
  August 2026) baubar: Haken auf `RtlAllocateHeap`, kritische Abschnitte,
  SRW-Sperren, Wartefunktionen und Datei-I/O prüfen das bestehende
  thread-lokale Flag der `RtWache`. Fertig verfügbar ist nichts; Umfang
  geschätzt 400 bis 600 Zeilen (ungeprüft). JUCEs VST3-Wrapper nimmt in FL
  selbst eine Sperre um `process()` (JUCE-Commit 13d2798), die Falle braucht
  deshalb eine Ausnahmeliste.

### Hostverhalten

- JUCEs VST3-Wrapper ruft `reset()` aus `setProcessing(false)` und
  `releaseResources()` aus `terminate()` ohne Sperre; die FL-eigene Sperre
  deckt nur `setActive`, `activateBus`, `setBusArrangements` und `process`.
  Die VST3-Schnittstelle erlaubt `setProcessing` aus dem Audio-Thread.
- Time Travel Debugging: `winget install --id Microsoft.TimeTravelDebugging -e`
  (1.11.611.0, proprietär); Aufnahme braucht Administratorrechte, verlangsamt
  5- bis 20-fach; `-module <plugin>` begrenzt die Aufnahme auf das Plugin,
  `-ring` und `-maxFile` die Größe. Abfrage aller Aufrufe mit Thread und
  Reihenfolge: `dx @$cursession.TTD.Calls("<modul>!*JuceVST3Component::setProcessing")`;
  ohne Symboldateien erscheinen die Funktionen nicht.
- Ohne Aufnahme: cdb-Haltepunkte mit Ausgabe und Weiterlauf
  (`bp <symbol> ".printf \"%x\\n\", @$tid; gc"`), nur für seltene Aufrufe.
- FL Studio 26.1.4.5589 ist installiert; FL schreibt Absturzprotokolle nach
  `Dokumente\Image-Line\FL Studio\Support\Logs\Crash` (jüngstes vom
  05.08.2026) und je Start ein Startprotokoll.
- pluginval v1.0.4 (04.12.2024) ist die letzte Veröffentlichung; `--skip-gui-tests`
  lässt sechs Tests aus, darunter alle Editor-Nebenläufigkeitstests;
  `--block-sizes 1,…` bildet FLs Einzelsample-Blöcke nach. Der
  Steinberg-Validator (VST3 SDK 3.8.1, 11.08.2026, MIT) prüft die rohen
  VST3-Aufrufe, unter anderem `setProcessing` und `process` auf einem
  eigenen Thread und Blockgrößen von 0 bis Maximum; JUCEs CMake kann ihn
  nicht fahren, er entsteht aus dem SDK.

### Zeitleisten und Beobachtung

- ETW TraceLogging (C++ `TraceLoggingProvider.h`, Rust-Crate `tracelogging`
  1.2.4) verwirft bei vollen Puffern statt zu blockieren, kostet aber je
  Ereignis einen Kernelübergang; auf dem Audio-Thread nur über den
  vorhandenen sperrfreien Ring, gesendet von einem Nicht-Echtzeit-Thread.
  Aufnahme mit dem eingebauten `wpr.exe`; eine maschinenlesbare Auswertung
  braucht ein Zusatzwerkzeug (.NET TraceProcessor oder Windows Performance
  Toolkit). Perfetto liest keine `.etl`-Dateien.
- Tracy 0.14.1 zeichnet je Aufnahme nur einen Prozess auf; das Zusammenführen
  verliert Kontextwechsel und Sperrereignisse.
- Process Monitor zeigt seit 4.1 (19.08.2026) Named-Pipe-Ereignisse.
- loom und shuttle modellieren weder Pipes noch Zeitlimits; tokio-console
  setzt Tokio voraus. Application Verifier verlangt für eine DLL den ganzen
  Host (FL prüft dann jedes Fremdplugin mit) und sieht Rusts Sperren nicht.

## Konsequenz für die Prüfspur

Vorschläge nach Nutzen und Aufwand; keine Entscheidung.

1. **Debugger für Agenten:** WinDbg (mit cdb) und ProcDump installieren,
   Release-C++ mit Symboldateien bauen und sie je installiertem Bau
   archivieren. Der Runner nimmt vor dem Beenden eines Beins nach dem
   Zeitlimit die Stapel aller Threads des Prozessbaums auf
   (`tools/beweise.ps1`, `Warte-MitZeitlimit`). Der Laufzeit-Arm lässt
   ProcDump mit `-e -t` auf FL64.exe laufen. mcp-windbg ist optional.
2. **nextest für A4 und A4-SI:** benannte Zeitlimits statt 60 Minuten Stille,
   Kennzeichnung wackelnder Tests.
3. **Compilerprüfungen:** C6262 als eigenes billiges Bein einschließlich
   Tests; Thread Safety Analysis im Bein A33 mit annotiertem Mutex-Wrapper,
   zuerst an den Klassen mit Befunden (ControlClient, IPC, Lebenslauf);
   `nonblocking` als Pilot mit einem neueren clang-tidy nur auf dem DSP-Kern.
4. **ASan-Bau der C++-Tests** in einem eigenen Baubaum, nächtlich oder bei
   Tickets an Lebenslauf und IPC; später Plugin in pluginval oder FL.
   Fuzzing der Parser auf derselben Grundlage.
5. **Linux-Bein** mit ThreadSanitizer und RealtimeSanitizer für die
   portablen sperrfreien Teile und den DSP-Kern.
6. **Hostmessung:** TTD-Aufnahme oder cdb-Haltepunkte für Thread und
   Reihenfolge der Lebenszyklusaufrufe in FL (NAK-361 L-1).
7. **pluginval und Steinberg-Validator** wieder als Beine, mit Editor-Tests
   und Blockgröße 1.

## Visuelle Belege

Keine.

## Bedeutung für Nakama

- Nebenbefund NAK-353, gemessen: alle zehn Dumps stammen aus dem gewollten
  Abbruch des Tests `revert_fehlschlag_beendet_testbrokerprozess_fail_fast`
  (Bein A4); jeder Kanon verdrängt dabei einen älteren Dump aus dem
  gemeinsamen WER-Ordner. Beleg `docs/beweise/roh/NAK-353-dumpanalyse.txt`.
- Nebenbefund NAK-175-Klasse, gemessen: `main` von EqCopAnalysisGoldenTest
  belegt 1 001 008 Bytes Stapel (95,5 % der 1-MiB-Reserve), EqCopQueueStressTest
  930 816 Bytes (88,8 %). Beleg `docs/beweise/roh/NAK-175-stapelmessung.txt`.
- Einhängepunkte: Prüfgang und Tore in `docs/gesundheit/KONZEPT.md` §4.1
  (Echtzeit-Hygiene, Vakuum-Wachen), §4.3 Echtzeit-Audit (für die offene
  Frage „Präzision des Aufrufgraphen ohne clang" aus §11 bietet
  `-Wfunction-effects` einen clang-gestützten Weg), §4.6 Laufzeit-Arm;
  Abdeckungskarte K1, K2, K5, K6.

## Grenzen, Widersprüche und offene Fragen

- Ungeprüft: wie FL64.exe Plugin-DLLs lädt (entscheidet, wo die
  ASan-Laufzeit liegen muss); ob clang-tidy 20 oder neuer über MSVC-STL 14.44
  und JUCE 8.0.9 sauber läuft; der Umfang einer Detours-Falle; ob `cdbX64 -z`
  eine TTD-Aufnahme ohne Fenster öffnet; wie FL Audio unter TTD-Last
  bedient; ob ein ProcDump-Dump bei `-t` den Thread zeigt, der FL beenden
  ließ, oder nur dessen Rest.
- Die Stapelwerte stammen aus Assemblerlistings mit den Schaltern des
  clang-tidy-Baums, nicht aus dem gelinkten Produktbinary.
- Versionen und Daten gelten für den 22.09.2026 und veralten.

## Quellen

- Titel: Debugwerkzeuge für ein JUCE-/Rust-Projekt unter Windows (eigene
  Recherche); Autor: Claude (Session nakama-1f); Jahr: 2026
- Eingang: `web-recherche`
- URLs (Auswahl, alle am 22.09.2026 gelesen):
  - https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/windbg-release-notes
  - https://learn.microsoft.com/en-us/windows-hardware/drivers/debugger/cdb-command-line-options
  - https://learn.microsoft.com/en-us/sysinternals/downloads/procdump
  - https://learn.microsoft.com/en-us/windows/win32/wer/collecting-user-mode-dumps
  - https://learn.microsoft.com/en-us/cpp/intrinsics/fastfail
  - https://learn.microsoft.com/en-us/cpp/build/reference/debug-generate-debug-info
  - https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/time-travel-debugging-ttd-exe-command-line-util
  - https://learn.microsoft.com/en-us/windows-hardware/drivers/debuggercmds/time-travel-debugging-calls-objects
  - https://github.com/svnscha/mcp-windbg
  - https://nexte.st/docs/features/slow-tests/
  - https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
  - https://clang.llvm.org/docs/FunctionEffectAnalysis.html
  - https://learn.microsoft.com/en-us/cpp/code-quality/c6262
  - https://learn.microsoft.com/en-us/cpp/sanitizers/asan
  - https://learn.microsoft.com/en-us/cpp/sanitizers/asan-known-issues
  - https://devblogs.microsoft.com/cppblog/msvc-address-sanitizer-one-dll-for-all-runtime-configurations/
  - https://clang.llvm.org/docs/ThreadSanitizer.html
  - https://www.intel.com/content/www/us/en/developer/articles/release-notes/inspector-release-notes.html
  - https://github.com/realtime-sanitizer/rtsan
  - https://github.com/microsoft/Detours
  - https://github.com/Tracktion/pluginval
  - https://steinbergmedia.github.io/vst3_dev_portal/pages/Versions/Version+3.8.1.html
  - https://github.com/actions/runner-images
  - https://learn.microsoft.com/en-us/windows/win32/api/traceloggingprovider/nf-traceloggingprovider-traceloggingwrite

## Verknüpfungen

- bestehende Verträge: [`CLAUDE.md`](../../CLAUDE.md) (Audio echtzeitfest,
  Beweise), [`docs/gesundheit/KONZEPT.md`](../../docs/gesundheit/KONZEPT.md),
  [`docs/gesundheit/abdeckungskarte.md`](../../docs/gesundheit/abdeckungskarte.md),
  [`docs/offene-punkte.md`](../../docs/offene-punkte.md) (NAK-26, NAK-175,
  NAK-184, NAK-241, NAK-290, NAK-352, NAK-353, NAK-361)
- verwandte Wissenseinträge:
  [Realtime-Qualität misst den seltenen Spike](2026-08-27-realtime-worst-case-statt-durchschnitt.md),
  [Windows-Pipes sind Sicherheits- und Lebenszyklusverträge](2026-08-27-windows-pipes-sicherheit-und-lifecycle.md),
  [Beweise müssen aus versionierten Quellen neu entstehen](2026-08-27-beweise-muessen-reproduzierbar-sein.md)
