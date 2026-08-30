# Beweismanifest — SONDE-007a «Gemeinsamer Kern ohne `JucePlugin_*`-Konstanten»
<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-23 nachgearbeitet -->
<!-- NAKAMA-URTEIL: T3 NEEDS_WORK 2026-08-24 offen -->
<!-- NAKAMA-URTEIL: T3 NEEDS_WORK 2026-08-24 nachgearbeitet -->
<!-- NAKAMA-URTEIL: T3 NEEDS_WORK 2026-08-28 nachgearbeitet -->
<!-- NAKAMA-URTEIL: T3 PASS 2026-08-30 -->

Der folgende Prueferblock ist **woertlich** und bezieht sich auf den Stand
`@ 3353fb6` (24.08.2026); seine Zeilenangaben gelten fuer jenen Stand, nicht
fuer den heutigen. Die heutigen Symbolverweise stehen unter dem Zitat.

> **T3 / Gate G1, 24.08.2026 — NEEDS_WORK.** Der Riegel **K2b**
> (`cmake/NakamaKern.cmake:218 ff.`, tragende Schleife `:234`) laeuft nur ueber
> die Referenz und fragt „fehlt das im Kern?" — nie umgekehrt. Ein kern-eigenes
> Zusatz-Define ist damit unsichtbar, obwohl der Kopf desselben Riegels (`:202`)
> zusagt: „dieser Riegel haelt die beiden Mengen zusammen" — eine Aussage ueber
> MENGENGLEICHHEIT, die einseitige Enthaltung nicht leistet. Ein Riegel, der
> weniger kann als er behauptet, beweist mit seinem Schweigen nichts.
> Vollstaendig: `docs/beweise/G1.md` §4.4, §7.1.

Heute (ausserhalb des Zitats, gemessen am Stand dieses Abschnitts): der
Riegel steht als `nakama_kern_konfig_pruefen()` in
`eq-copilot/cmake/NakamaKern.cmake`; die tragende Schleife ist die Schleife
ueber die Konfigurationen in derselben Funktion, und die zitierte Kopfzusage
ist der Kopfkommentar desselben Riegels. Die drei Zeilennummern des Zitats
zeigen am heutigen Stand alle drei auf den Generatorausdruck-Parser
(`_nakama_kern_genex_ende` bzw. `_nakama_kern_genex_kopf`) — genau darum
traegt der einleitende Satz den Stand.


> **Die eine harte Regel** (`docs/bauaufteilung-sonden.md` §2): *Eine Behauptung
> ohne eingefügte Rohausgabe ist ein **gescheitertes** Ticket, kein fast
> fertiges.* Unten stehen stdout/stderr wortgleich.

| Feld | Wert |
|---|---|
| Ticket | `SONDE-007a` (Teil 1 von `SONDE-007`; Teil 2 = `SONDE-007b` / S9) |
| Phase / Session | P1 / S8 |
| Gate-Text (Quelle) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §53.4 „Static-Lib-Randbedingung" — **wörtlich:** „Der Wechsel auf gemeinsame statische Bibliotheken ist für drei Ziele gerechtfertigt, aber nur unter einer harten Regel: Der gemeinsame Kern sieht **keine** `JucePlugin_*`-Konstanten; Identität kommt ausschließlich aus `plugin-identities-v1.json` über die dünnen Target-Schichten. Andernfalls erhalten zwei der drei Bundles die Identitätskonstanten des dritten — genau die Werte, die §44.1 einfriert. `SONDE-007` enthält dafür einen expliziten Prüfschritt." · `docs/bauaufteilung-sonden.md` S8-Zeile + Fließtext: „Wenn der geteilte Kern auch nur **eine** `JucePlugin_*`-Konstante sieht, bekommen zwei Bundles die Identität des dritten — und genau das Identitäts-Golden aus S2 fällt." |
| Datum | 2026-08-22 |
| Prüfstufen | T1 ☑ (§3) · T2 ☑ gefahren 23.08. — **NEEDS_WORK**, fünf Befunde, Urteil und Belege in [§5](#5-t2--frischkontext-prüfer), Nacharbeit in [§6](#6-nacharbeit-zu-den-t2-befunden) · T3 ☑ gefahren seit 24.08. (G1), NEEDS_WORK in 20 Prüfrunden nachgearbeitet — **PASS 30.08.** durch den gebundenen Abschlussprüfer (Thread `01a0523e`, Stand `7a87b7d`; Abschnitt „Dirigentenstand — 2026-08-30 12:48") |

**Was gebaut wurde (Karte, keine Behauptung — Behauptungen stehen in §1):**

**Stand dieser Karte:** `f68cd9a` — an
diesem Stand sind ihre Anker geprüft (Runde 10: Zeile **K3/A14** trägt den Ausgang F13/F14/F15, Zeile
**Tlog-Riegel** nennt keine Anzahl mehr; Runde 16: Zeile **K1b** nennt den
K1-Fehlerkranz als einzige Quelle der Makroliste und führt die Kurzform
`#ifdef`/`#ifndef` nicht mehr als erlaubten Kontext; Runde 17: dieselbe Zeile
**K1b** nennt die Vorstufen — fail-closed Kodierung mit BOM-Erkennung und
Präprozessor-Phase-2-Faltung vor Kommentarentfernung und Tokenprüfung;
Runde 18: dieselbe Zeile **K1b** nennt zusätzlich die **Präprozessor-Phase 1**
davor und die Folge, dass die Zeilenendform nicht mehr über das Urteil
entscheidet; Runde 19: an dieser Karte hat sich **nichts** geändert — die zwei
nachgezogenen Stellen sind die Prosa-Köpfe in `tools/beweise.ps1` und
`tools/eq-copilot/pruefe_kern_identitaetsfrei.py`, nicht die Kartenzeilen; die
übrigen Anker sind an diesem Commit einzeln nachgeschlagen).

**Was der Stand dieser Zeile bedeutet** (Befund des achtzehnten Prüfers): der
genannte Commit ist der **letzte Commit der Runde vor dem Kanon-Abschluss**,
und sein Manifest trägt genau diese Kartenzeilen. Bis Runde 17 nannte sie
jeweils den Stand der *vorigen* Runde: `git show 75466c0:docs/beweise/SONDE-007a.md`
enthielt die Runde-17-Anker nicht, obwohl die Zeile sie dort als geprüft
auswies. Der Kartenstand wird deshalb im Abschluss-Commit nachgezogen, und die
Anker werden an genau diesem Stand nachgeschlagen — ein Stand, dessen Manifest
die Kartenzeilen noch nicht enthält, ist ein Befund und kein Formfehler.
Sie nennt bewusst **keine** Anzahlen (Befund B4, Runde 6): Quellen, Verbraucher
und Makros stehen als Quellenanker da, weil jede abgeschriebene Zahl still
falsch wird, sobald jemand eine Datei oder ein Ziel ergänzt.

- `eq-copilot/cmake/NakamaKern.cmake` — Linkhüllen-Läufer (einmal geschrieben,
  zweimal benutzt), **Kopf-Fassade** `nakama_kern_juce_fassade()`, der
  Konfigurier-Riegel **K2** `nakama_kern_riegel_pruefen()` und der
  Konfigurationsgleichheits-Riegel **K2b** `nakama_kern_konfig_pruefen()`.
- `eq-copilot/plugin/state/NakamaKernRiegel.h` — Kompilier-Riegel **K1**, die
  dort namentlich geführten `JucePlugin_*`-Makros, scharf nur unter
  `NAKAMA_KERN_UEBERSETZUNG`. Die aktuelle Anzahl gibt der Messlauf aus.
- `eq-copilot/plugin/CMakeLists.txt` — `NakamaKern` als `add_library(… STATIC)`
  über die Liste `NAKAMA_KERN_QUELLEN` ebendort (sie ist die Quelle, nicht diese
  Karte; `pruefe_kern_identitaetsfrei.py` liest sie mit `_kernquellen_aus_cmake()`);
  `nakama_state_anbinden()` → `nakama_kern_anbinden()` bei jedem Ziel, das die
  Funktion in derselben Datei ruft. Der Kopfkommentar, der bis heute die
  Static-Lib begründet ablehnte, trägt jetzt den abgelösten Satz samt Grund
  der Ablösung.
- `tools/eq-copilot/pruefe_kern_identitaetsfrei.py` — Artefakt-Riegel **K3**
  mit eigener Gegenprobe und COFF-Archivleser.
- `tools/beweise.ps1` — Kanon-Bein **A14**; `NakamaKern` als *gemessenes Ziel*
  (wird gebaut, läuft aber nicht selbst).

**Arbeitsteilung der acht Riegel** — der Punkt des Tickets, nicht Redundanz.
Diese Übersicht beschreibt den heutigen Arbeitsbaum; ältere Rohausgaben weiter
unten bleiben datierte Belege ihres damaligen Quellstands:

| | misst | sieht | sieht **nicht** |
|---|---|---|---|
| **K1** Präprozessor | Quelltext | die in `eq-copilot/plugin/state/NakamaKernRiegel.h` namentlich geführten `JucePlugin_*`-Makros — die Liste dort ist die Quelle, nicht diese Karte —, am Anfang **und Ende** jeder Kern-Übersetzungseinheit (gemessen aus `NAKAMA_KERN_QUELLEN`; die aktuelle Anzahl gibt der Messlauf aus); damit auch bis zum TU-Ende definierte Makros aus später eingebundenen eigenen/generierten Headern | Makronamen außerhalb der Liste und vor dem TU-Ende wieder entfernte Makros (der Präprozessor kann kein Präfix aufzählen; resultierende Identitätsbytes misst K3, den Quelltext selbst **K1b**) |
| **K1b** Quelltext-Token | die tatsächlichen Compiler-Eingaben: jede Datei aus dem frisch geschriebenen `CL.read.1.tlog`, die **nicht** aus den JUCE-Modulen und nicht aus den Toolchain-/SDK-Wurzeln stammt — also `plugin/**` **und alles Übrige**, `/FI` und vorkompilierte Köpfe eingeschlossen —, plus die literale Include-Hülle als Gegenprobe; die namentlich erlaubten Systemdateien werden dabei roh in ASCII und UTF-16LE durchsucht statt als C++ geparst. Jede andere Eingabe geht durch dieselben Vorstufen wie im Übersetzer, in dieser Reihenfolge: **fail-closed gelesen** — eine BOM entscheidet die Kodierung (UTF-8, UTF-16LE, UTF-16BE), ohne BOM gilt strikt UTF-8, und eine nicht sicher dekodierbare Eingabe ist eine **namentliche Klage** statt eines still ersetzten Zeichens —, danach **Präprozessor-Phase 1 normalisiert** (CRLF und einzelnes CR werden LF, ohne dass sich die Zeilenzahl ändert) und erst dann **Präprozessor-Phase 2 gefaltet**: Backslash + Zeilenende (auch CRLF, auch mit Leerraum davor) verschwindet | jedes `JucePlugin_`-Token im Quelltext, unabhängig von `#define`/`#undef`; Kommentare werden vorher entfernt, Stringliterale nicht — und Normalisierung und Faltung laufen ihrerseits **vor** der Kommentarentfernung, wie im Übersetzer, so dass ein über ein Zeilenende geteiltes `JucePlugin_Name`, ein UTF-16-Kopf mit BOM und ein hinter `//` in einer CR-only-Datei verstecktes `#define`/`#if`/`#undef` ROT sind statt unsichtbar; die Zeilenendform entscheidet damit nicht mehr über das Urteil; einzige Ausnahme ist `NakamaKernRiegel.h`, gemessen und namentlich — und das heißt seit Runde 15 ein **Abgleich**, keine Freistellung: jedes ihrer Token muss in der Makroliste stehen, die K1 in **derselben** Datei abfragt (eine Quelle, zwei Verbraucher — die Liste dort ist die Quelle, nicht diese Karte), und in einem Riegelkontext. Diese Liste ist seit Runde 16 **ausschließlich** der eine `#if defined (…)`-Kranz, der in den K1-`#error` mündet — nicht mehr jede `#if`/`#elif`-Zeile der Datei; erlaubt sind nur `defined (…)` **in** diesem Kranz und das blosse Präfix in **seiner** `#error`-Meldung. Die Kurzform `#ifdef`/`#ifndef` ist damit kein erlaubter Kontext mehr: sie ist stets eine eigene Direktive und liegt außerhalb des Kranzes. Jeder andere Name, jedes Token außerhalb des Kranzes — auch ein **bekannter** Name in einem eigenen `#if defined (…)`-Block — und jedes `#undef` sind ROT und werden beim Namen genannt; ohne eindeutig ableitbare Makroliste ist die Ausnahme selbst ROT: kein Kranz mit `#error`, mehr als einer und eine unpaarige Bedingungsstruktur sind je für sich ROT. Die Zahlen (Token, abgeglichen, Makros) gibt der Messlauf aus | den Inhalt der JUCE-Module (dafür der JUCE-Baum-Riegel) und der Toolchain-/SDK-Header außerhalb des Repos (benannte Nichtzusage, kein Fingerprint); Makronamen, die erst durch Tokenverkettung entstehen. Lässt sich eine der drei Ausschlusswurzeln nicht ableiten, bildet K1b **keine** Menge, sondern klagt |
| **K2** CMake-Konfigurierzeit | Kernziel plus dessen compilerwirksame Usage-Requirements-Hülle; Verbraucher nur bei einer echten fehlerhaften Rückkante; Ausführung verzögert bis zum Ende von `plugin/` nach allen Zieländerungen | jedes compilerwirksame `JucePlugin_` aus eigenen und transitiven `*_COMPILE_DEFINITIONS` sowie `-D`/`/D` in `*_COMPILE_OPTIONS`; direkte Zielnamen, `debug`/`optimized`/`general`-Kanten und die unten inventarisierten bedingten bzw. zielbezogenen Generatorausdrücke | Makros, die erst im C++-Quelltext entstehen (dafür K1/K3); String-Transformationen in Linkkanten und `MAP_IMPORTED_CONFIG_*` werden nicht ausgewertet, sondern ausdrücklich **ROT** gemeldet |
| **K3/A14** Artefakt + Neubau | die `.lib`, die das Bein im selben Lauf selbst hat neu erzeugen lassen, dazu `.vcxproj`, `.tlog` und `lastbuildstate`; fehlt eine Voraussetzung, endet der Lauf über `voraussetzung_exit()` — ohne registrierten Befund **3**, mit registriertem Befund **2**, **nie 0**, und das an jedem Ausgang, den unmöglichen oder fehlgeschlagenen Bau eingeschlossen (Matrix F13/F14/F15, Runde 8/9; Proben `R8-1` und `P9-F13`) | jeden eingefrorenen Text als ASCII/UTF-16LE, Viercodes zusätzlich als 4-Byte-Integer in **beiden** Byteordnungen, CIDs roh/COM-vertauscht; dass Objekte, Tlogs und Lib vor der Messung gelöscht und vollständig neu erzeugt wurden (Zeitanker, Bauausgabe, Objektzahl) — damit ist „veraltetes Artefakt" keine Frage mehr; die früheren Frischewachen bleiben als Diagnose „womit wurde gebaut" | Baubeschreibung ohne resultierende Artefaktbytes (dafür K1/K1b/K2/K2b/K2c); ein Compilerwechsel innerhalb derselben `lastbuildstate`-Kennung (benannte Nichtzusage) |
| **Tlog-Riegel** Leseorte | das frisch geschriebene `CL.read.1.tlog` des Kerns | aus welchen Orten der Compiler wirklich gelesen hat: erlaubt sind `plugin/**`, `juce-src/modules/**` ohne `juce_audio_plugin_client`, die **aus dem Bau abgeleiteten** MSVC- und Windows-SDK-Wurzeln und — **namentlich, nicht über ihr Verzeichnis** — die gemessenen Systemdateien unter `%SystemRoot%` (Liste `SYSTEMDATEIEN` im Skript — sie ist die Quelle, nicht diese Karte; ihre Namen stehen seit Runde 8 als eingefügte `CL.read.1.tlog`-Rohausgabe im Abschnitt „Nacharbeit Runde 8", Probe `P8-SYS`, ihre Anzahl gibt der Messlauf aus — vorher war die Liste eine Behauptung ohne Rohausgabe). Es gibt weder eine Endungsausnahme noch eine Ortserlaubnis für `%SystemRoot%`: jede andere Datei darunter, `<ziel>_artefacts/JuceLibraryCode/**` und alles Unbekannte sind ROT und werden namentlich genannt. Fehlt eine heutige Kernquelle als Marker, ist auch das ROT | den **Inhalt** der gelesenen Dateien (dafür K1b und der JUCE-Baum-Riegel) |
| **JUCE-Baum-Riegel** Herkunft | der ganze FetchContent-Baum `build/_deps/juce-src` gegen `git status` und den Nakama-Patch | dass juce-src der gepinnte Tag plus **genau** `third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch` ist, gemessen in drei Zeilen: (i) `HEAD` **ist** der Commit des Tags (`8.0.9^{commit}`, verglichen statt nur beschrieben); (ii) `git status --porcelain --ignored -uall` — die geänderte Menge ist genau die Patchdateimenge, **ignorierte** Fremddateien eingeschlossen; (iii) die Patchdateien tragen genau den Patch, gemessen als Inhalt gegen den in einem temporären Index gerechneten Sollzustand „Tag + Patch" (`read-tree` → `apply --cached` → `diff-files`). `git apply --check --reverse` ist dafür **ersetzt**, nicht ergänzt: es prüft nur rückwärts passende Hunks und schwieg zu einer zusätzlichen Zeile | Löschungen außerhalb `modules/**` (gezählt und benannt, nicht ROT — eine gelöschte Datei kann keine Compiler-Eingabe werden); Toolchain- und SDK-Header außerhalb des Repos (benannte Nichtzusage, kein Fingerprint) |
| **K2b** CMake-Konfigurierzeit | Kern und je ein registrierter Verbraucher als **getrennte** Wurzeln mit ihrer jeweiligen compilerwirksamen Usage-Requirements-Hülle | Mengengleichheit und Wertwidersprüche der `JUCE_`-Defines beider Zielmengen, je Konfiguration, rekursiv und inklusive `-D`/`/D`; `JucePlugin_*` des Verbrauchers gehört nicht zur Vergleichsmenge | bewusst ausgenommene Hüllendefines (`JUCE_MODULE_AVAILABLE_*` als Familie; exakt die Makronamen `JUCE_SHARED_CODE`, `JUCE_STANDALONE_APPLICATION`, `JUCE_VST3_CAN_REPLACE_VST2`, jeweils ohne Wert oder mit `=…`) · Nicht-Define-Schalter (dafür K2c) |
| **K2c** CMake-Konfigurierzeit | volle Linkhülle des Kerns und volle Linkhülle je eines registrierten Verbrauchers, **getrennt** je Konfiguration | ob jedes transitiv und bedingt erreichbare `juce_recommended_*`-Ziel der Referenz in derselben Konfiguration auch am Kern hängt — Quelle der Schalter, nicht einzelne Flags | `lto_flags` (begründet ausgenommen: `/GL` ohne `-LTCG` im Verbraucher) · alles, was kein Empfehlungsziel ist; String-Transformationen und importierte Konfigurationsabbildungen sind nicht unterstützt und deshalb ROT |

K2b ist im Selbstaudit nach dem ersten Commit dazugekommen (§2 B8), **K2c** aus
dem T2-Lauf am 23.08. (§5 T2-1/T2-3, Nacharbeit §6) — bis dahin sagte die
K2b-Zeile hier „mit derselben Konfiguration" und griff damit weiter, als der
Riegel misst. Beide beantworten eine andere Frage als K1–K3: nicht „trägt der
Kern eine fremde Identität?", sondern „bedeutet derselbe JUCE-Header im Kern
dasselbe wie im Verbraucher, und wird er gleich scharf übersetzt?". Die
Rohausgaben in §2 stammen von vor K2c: dort steht die Linkhülle noch mit
**7** statt 9 Zielen.

### Riegel-Nacharbeit S8 vom 28.08.2026 — acht übergebene P1

**Stand dieses Abschnitts:** `374eea7` — Positionen ohne eigene Angabe sind an diesen Commit gebunden.

Alle acht Fälle werden gemessen; keine Zusage wurde auf eine unbenannte Lücke
verengt. Die kontrollierten roten Fälle bleiben als manuelle Repro-Blöcke
erhalten. Jeweils **eine** Probe an der angegebenen Stelle in
`plugin/CMakeLists.txt` einsetzen, mit dem angegebenen frischen Bauverzeichnis
konfigurieren, den roten Configure-Ausgang sichern und die Probe wieder entfernen:

1. **Konfigurationsabhängiger Wertwiderspruch, nur Release (K2b):**

   ```cmake
   target_compile_definitions(NakamaKernJuce INTERFACE
       "$<$<CONFIG:Release>:JUCE_USE_CURL=1>")
   ```

   ```powershell
   cmake -S eq-copilot -B eq-copilot/build-riegelprobe-config -G "Visual Studio 17 2022" -A x64
   ```

   Erwartet: K2b bleibt für Debug widerspruchsfrei und fällt für Release auf
   `JUCE_USE_CURL=0` plus `JUCE_USE_CURL=1`.

2. **Transitives Interface-Define über `mid -> leaf` (K2b):**

   ```cmake
   add_library(NakamaKernProbeLeaf INTERFACE)
   add_library(NakamaKernProbeMid INTERFACE)
   target_compile_definitions(NakamaKernProbeLeaf INTERFACE JUCE_NUR_AM_KERN=1)
   target_link_libraries(NakamaKernProbeMid INTERFACE NakamaKernProbeLeaf)
   target_link_libraries(NakamaKern PRIVATE NakamaKernProbeMid)
   ```

   ```powershell
   cmake -S eq-copilot -B eq-copilot/build-riegelprobe-transitiv -G "Visual Studio 17 2022" -A x64
   ```

   Erwartet: K2b nennt `JUCE_NUR_AM_KERN=1` als nur am Kern vorhanden.

3. **Identitätsdefine als `/D`-Compile-Option (K2):**

   ```cmake
   target_compile_options(NakamaKern PRIVATE /DJucePlugin_NeueIdentitaet=0x45716370)
   ```

   ```powershell
   cmake -S eq-copilot -B eq-copilot/build-riegelprobe-option -G "Visual Studio 17 2022" -A x64
   ```

   Erwartet: K2 fällt auf `JucePlugin_NeueIdentitaet=0x45716370`.

4. **Bedingte, verschachtelte Release-Linkkante (K2; derselbe Läufer trägt K2c):**

   ```cmake
   add_library(NakamaKernIdentitaetsIface INTERFACE)
   target_compile_definitions(NakamaKernIdentitaetsIface INTERFACE
       JucePlugin_NeueIdentitaet=0x45716370)
   target_link_libraries(NakamaKern PRIVATE
       "$<$<CONFIG:Release>:NakamaKernIdentitaetsIface>")
   ```

   ```powershell
   cmake -S eq-copilot -B eq-copilot/build-riegelprobe-linkkante -G "Visual Studio 17 2022" -A x64
   ```

   Erwartet: K2 fällt in Release; die Debug-Hülle enthält das Interface-Ziel
   nicht. K2c wird mit derselben Kantenform separat vorgeführt (ebenfalls vor
   dem K2-Aufruf einfügen):

   ```cmake
   add_library(juce_recommended_k2c_probe INTERFACE)
   target_link_libraries(EqCopilot PUBLIC
       "$<$<CONFIG:Release>:juce_recommended_k2c_probe>")
   ```

   ```powershell
   cmake -S eq-copilot -B eq-copilot/build-riegelprobe-k2c-linkkante -G "Visual Studio 17 2022" -A x64
   ```

   Erwartet: K2c fällt nur für Release, weil der Verbraucher dort die bedingte
   Empfehlungsquelle trägt und der Kern nicht.

5. **K1 nach einem späteren Kern-Header:** Als temporär letzte Zeile von
   `plugin/state/NakamaState.h` einsetzen:

   ```cpp
   #define JucePlugin_PluginCode 0x45716370
   ```

   ```powershell
   cmake -S eq-copilot -B eq-copilot/build-riegelprobe-k1 -G "Visual Studio 17 2022" -A x64
   cmake --build eq-copilot/build-riegelprobe-k1 --config Release --target NakamaKern
   ```

   Erwartet: Der Configure bleibt grün (isolierte K1-Probe), der Bau fällt in
   der Endprüfung von `NakamaState.cpp`. Die Anfangsprüfung allein liegt vor
   `NakamaState.h` und würde diese Probe nicht sehen.

Die baulose A14-Vorführung ist direkt ausführbar und benutzt als Testobjekt
exakt die little-endian Immediate-Bytes `70 63 71 45` von `0x45716370`:

```powershell
py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py --selbsttest
```

Erwartet: `fourcc-int-le`, `fourcc-int-be`, `roh16`, beide Define-Differenz-
richtungen und `NakamaUtf8.h` in der Includehülle, Exitcode 0. Der vollständige
A14-Lauf bleibt ein Nach-Bau-Bein; vor dem Scannen muss seine
rekursiv abgeleitete Includehülle (einschließlich `vertrag/NakamaUtf8.h`) älter
als die Lib sein und die Define-Mengen aus `.vcxproj` und `.tlog` müssen exakt
gleich sein.

### Letzte Nacharbeit S8 vom 28.08.2026 — vier frische Prüferfälle

**Stand dieses Abschnitts:** `068c9ce` — Positionen ohne eigene Angabe sind an diesen Commit gebunden.

Die ersten beiden Fälle erweitern die Messung. Die letzten beiden werten die
jeweilige CMake-Semantik bewusst **nicht** nach, sondern fassen die Zusage enger
und machen die nicht unterstützte Form fail-closed ROT.

1. **Gemessen — K2 läuft erst nach allen Änderungen am Kernziel.** Direkt nach
   der heutigen verzögerten Registrierung einsetzen:

   ```cmake
   cmake_language(DEFER DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
       CALL nakama_kern_riegel_pruefen NakamaKern)
   target_compile_options(NakamaKern PRIVATE /DJucePlugin_NeueIdentitaet=17)
   ```

   ```powershell
   cmake -S eq-copilot -B eq-copilot/build-riegelprobe-k2-reihenfolge -G "Visual Studio 17 2022" -A x64
   ```

   Erwartet: Erst nachdem die restliche `plugin/CMakeLists.txt` verarbeitet ist,
   fällt der verzögerte K2 auf `JucePlugin_NeueIdentitaet=17`. Ein früher
   Snapshot am alten Aufrufort würde diese Probe übersehen.

2. **Gemessen — die drei K2b-Einzelausnahmen sind vollständige Makronamen.**
   Unmittelbar vor dem abschließenden K2b/K2c-Lauf einsetzen:

   ```cmake
   target_compile_definitions(NakamaKern PRIVATE JUCE_SHARED_CODE_EXTRA=1)
   ```

   ```powershell
   cmake -S eq-copilot -B eq-copilot/build-riegelprobe-k2b-ausnahme -G "Visual Studio 17 2022" -A x64
   ```

   Erwartet: K2b nennt `JUCE_SHARED_CODE_EXTRA=1` als nur am Kern vorhanden.
   Nur `JUCE_SHARED_CODE` beziehungsweise `JUCE_SHARED_CODE=…` ist ausgenommen;
   dasselbe gilt exakt für die beiden anderen Einzelmakros. Der baulose
   Genex-Selbsttest prüft die drei Namen jeweils mit/ohne Wert und ihre
   `*_EXTRA`-Gegenstücke.

3. **Enger gefasst — String-Transformationen in Linkkanten.** Linkkanten werden
   bis zu direkten Zielnamen, `debug`/`optimized`/`general` und den inventarisierten
   Bedingungen/Zielreferenzen (`IF`, Kurzform, `CONFIG`, `TARGET_*`,
   `LINK_ONLY`, `BUILD_INTERFACE`, `COMPILE_ONLY`) aufgelöst.
   String-transformierende Generatorausdrücke (`LOWER_CASE`, `UPPER_CASE`,
   `MAKE_C_IDENTIFIER`, `JOIN`, `REMOVE_DUPLICATES`, `LIST`, `PATH`,
   `SHELL_PATH`) werden in Linkkanten nicht aufgelöst; ihr Auftreten ist ROT:

   ```cmake
   add_library(nakamaidentitaetsiface INTERFACE)
   target_compile_definitions(nakamaidentitaetsiface INTERFACE
       JucePlugin_NeueIdentitaet=17)
   target_link_libraries(NakamaKern PRIVATE
       "$<LOWER_CASE:NakamaIdentitaetsIface>")
   ```

   Erwartet: Configure fällt mit `String-transformierender Generatorausdruck
   LOWER_CASE in Linkkante wird nicht aufgeloest`, bevor die Kante still leer
   werden kann. **Benannte Lücke: `NAK-xx` (neue Nummer ausstehend) —
   String-Transformationen in Linkkanten werden nicht semantisch ausgewertet.**

4. **Enger gefasst — importierte Konfigurationsabbildung.**
   `MAP_IMPORTED_CONFIG_<CONFIG>` wird nicht berücksichtigt. Sobald ein
   importiertes Ziel in einer K2/K2b/K2c-Linkhülle für eine aktive
   Projektkonfiguration eine solche Property gesetzt hat, ist die Hülle ROT:

   ```cmake
   add_library(NakamaImportiert INTERFACE IMPORTED)
   set_target_properties(NakamaImportiert PROPERTIES
       IMPORTED_CONFIGURATIONS Hidden
       MAP_IMPORTED_CONFIG_DEBUG Hidden
       MAP_IMPORTED_CONFIG_RELEASE Hidden
       MAP_IMPORTED_CONFIG_RELWITHDEBINFO Hidden
       MAP_IMPORTED_CONFIG_MINSIZEREL Hidden
       INTERFACE_COMPILE_DEFINITIONS
           "$<$<CONFIG:Hidden>:JucePlugin_NeueIdentitaet=17>")
   target_link_libraries(NakamaKern PRIVATE NakamaImportiert)
   ```

   Erwartet: Configure fällt mit Ziel und erster gesetzter
   `MAP_IMPORTED_CONFIG_*`-Property; es behauptet nicht, die `Hidden`-Abbildung
   selbst ausgewertet zu haben. **Benannte Lücke: `NAK-xx` (neue Nummer
   ausstehend) — importierte Konfigurationsabbildungen werden nicht semantisch
   ausgewertet.**

### Runde 2/3 — JUCE-Generatorausdrücke und Listen-Eigenschaften

**Stand dieses Abschnitts:** `374eea7` — Positionen ohne eigene Angabe sind an diesen Commit gebunden.

Die Inventur vom 28.08.2026 umfasst alle `$<`-Vorkommen in
`extras/Build/CMake/*.cmake`, `eq-copilot/cmake/*.cmake` und den beiden
Projekt-`CMakeLists.txt`. Für die von K2/K2b/K2c gelesenen Definitions-,
Options- und Linkeigenschaften ergibt sich:

| Fundstelle | dort vorkommende Formen | Behandlung im Riegelauswerter |
|---|---|---|
| `JUCEHelperTargets.cmake:41-44,78-84,109,123,137,144-165` | `CONFIG`, `OR`, `COMPILE_LANGUAGE`, `IF`, `STREQUAL` und die bedingte Kurzform | je Konfiguration verschachtelt ausgewertet; die Compile-Sprache ist für die ausschließlich aus C++-Quellen bestehenden Riegelzielmengen `CXX` |
| `JUCEModuleSupport.cmake:108-109,535,594` | `IF`, `OR`, `CONFIG`, `PLATFORM_ID`, `TARGET_EXISTS` und die bedingte Kurzform | verschachtelt ausgewertet; fehlende explizit referenzierte Ziele bleiben ROT |
| `JUCEUtils.cmake:309-312,841-849,1058-1062,1521-1582,2111` | `TARGET_GENEX_EVAL`, zweistelliges `TARGET_PROPERTY`, `FILTER`, `BOOL`, `TARGET_EXISTS` und die bedingte Kurzform | Ziel/Alias zuerst aufgelöst; skalare Eigenschaften direkt, Listeneigenschaften Element für Element rekursiv; `FILTER` unterstützt `INCLUDE`/`EXCLUDE`; die RC-Quellproperty in 841-849 gehört nicht zum Kern, benutzt aber dieselben unterstützten Formen |
| `plugin/CMakeLists.txt` | `CXX_COMPILER_ID` | gegen `CMAKE_CXX_COMPILER_ID` ausgewertet |
| `NakamaKern.cmake` | zweistelliges `TARGET_PROPERTY` für `INTERFACE_INCLUDE_DIRECTORIES`, `INTERFACE_COMPILE_DEFINITIONS`, `INTERFACE_COMPILE_OPTIONS` | Listenexpansion mit Besuchtmenge `Ziel::Eigenschaft`; Aliase werden aufgelöst, importierte CMake-Ziele ohne Konfigurationsabbildung wie normale Ziele gelesen, Zyklen/fehlende Ziele und gesetztes `MAP_IMPORTED_CONFIG_*` sind ROT |

Zusätzlich kennt der Auswerter die in den Riegelproben beziehungsweise von
CMake in Usage Requirements benötigten Formen `AND`, `NOT`, `EQUAL`,
`GENEX_EVAL`, `$<CONFIG>`, `LINK_ONLY`, `COMPILE_ONLY`, `BUILD_INTERFACE`,
`INSTALL_INTERFACE`, `TARGET_NAME` und `TARGET_NAME_IF_EXISTS`. Linkkanten mit
`LOWER_CASE`, `UPPER_CASE`, `MAKE_C_IDENTIFIER`, `JOIN`, `REMOVE_DUPLICATES`,
`LIST`, `PATH` oder `SHELL_PATH` werden nicht transformiert, sondern ROT
gemeldet. Die übrigen
gefundenen JUCE-Formen `TARGET_FILE`, `TARGET_BUNDLE_DIR` und
`TARGET_BUNDLE_CONTENT_DIR` stehen nur in Custom-Command-, Ausgabe- und
Bundlepfaden (`JUCEUtils.cmake:212-221,748,962-965,1184-1192,1251-1362`),
nicht in einer von K2/K2b/K2c gelesenen Eigenschaft; dasselbe gilt für
`CONFIG`/`TARGET_FILE` in den FlatBuffers-Ausgabe- und Custom-Command-Pfaden
(`NakamaFlatBuffers.cmake:145-224`). Ebenso liegen
`EQUAL`/`GENEX_EVAL` mit der einstelligen, zielkontextabhängigen
`TARGET_PROPERTY` nur in `INTERFACE_LINK_DIRECTORIES`
(`JUCEModuleSupport.cmake:434-444`). Diese Pfadformen werden daher nicht
behauptet; tauchen ein unbekannter Ausdruck oder eine Zielreferenz doch in
einer Riegelzielmenge auf, wird nicht still verworfen, sondern ROT gemeldet.

Die Zielmengen bleiben getrennt: K2 liest `NakamaKern` plus dessen
compilerwirksame Usage-Requirements-Hülle. K2b berechnet Kern und registrierten
Verbraucher als zwei eigene Wurzeln und vergleicht nur ihre `JUCE_`-Defines;
die `JucePlugin_*`-Defines des Verbrauchers werden dabei zwar vollständig
ausgewertet, gehören aber nicht zur Vergleichsmenge. K2c vergleicht die volle
Linkhülle beider Wurzeln je Konfiguration. `EqCopilot`, `NakamaSuna` und
`NakamaProbeeq` sind Verbraucher und werden nur durch eine echte Rückkante zu
Quellen des Kerns.

Der baulose Regressionstest prüft 26 Ausdrücke, darunter genau
`JucePlugin_IsSynth=$<BOOL:$<TARGET_PROPERTY:EqCopilot,JUCE_IS_SYNTH>>`, die
reale `juce_core`-Liste aus `INTERFACE_COMPILE_DEFINITIONS`, eine bedingte
`INTERFACE_LINK_LIBRARIES`-Liste über einen Alias und alle oben als unterstützt
benannten Inventurformen. Sensitivitätsproben prüfen zusätzlich Property-Zyklus,
fehlendes Ziel, unbekannte Eigenschaft, die exakten K2b-Ausnahmen und die Regel:
Eine Zielreferenz muss auch ohne sichtbares `JUCE_`-/`JucePlugin_`-Präfix
expandieren. Eine eigene Unterprozessprobe hält die `LOWER_CASE`-Linkkante ROT.
Nur ein Ausdruck ohne beide Präfixe **und ohne Zielreferenz** darf irrelevant bleiben:

```powershell
$cmake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
& $cmake -DNAKAMA_TEST_CONFIG=Debug -DNAKAMA_TEST_IS_SYNTH=FALSE -P eq-copilot/cmake/pruefe_nakama_kern_genex.cmake
& $cmake -DNAKAMA_TEST_CONFIG=Release -DNAKAMA_TEST_IS_SYNTH=TRUE -P eq-copilot/cmake/pruefe_nakama_kern_genex.cmake
```

Erwartet: beide Läufe enden mit
`Nakama-Kern-Genex-Selbsttest: 26/26 Ausdruecke korrekt.` und Exitcode 0.
Der fail-closed Pfad ist mit demselben Skript kontrolliert rot ausführbar:

```powershell
& $cmake -DNAKAMA_TEST_CONFIG=Debug -DNAKAMA_TEST_IS_SYNTH=FALSE -DNAKAMA_TEST_UNBEKANNT_ROT=ON -P eq-copilot/cmake/pruefe_nakama_kern_genex.cmake
```

Erwartet: Exitcode ungleich 0 und die Meldung
`JUCE_RIEGEL_UNBEKANNT=$<TARGET_PROPERTY:EqCopilot,NAKAMA_UNBEKANNT>` sei
„nicht aufloesbar; stilles Verwerfen waere falsch gruen“.

**Kontrollierter Bruch für eine unbekannte relevante Eigenschaft:** Unmittelbar
vor der abschließenden `foreach(verbraucher IN LISTS
NAKAMA_KERN_VERBRAUCHER)`-Schleife einsetzen:

```cmake
target_compile_definitions(EqCopilot PRIVATE
    "JUCE_RIEGEL_UNBEKANNT=$<TARGET_PROPERTY:EqCopilot,NAKAMA_UNBEKANNT>")
```

Dann aus der Repo-Wurzel:

```powershell
$cmake = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
& $cmake -S eq-copilot -B eq-copilot/build-riegelprobe-unbekannt -G "Visual Studio 17 2022" -A x64
```

Erwartet: K2b fällt beim Configure mit
`JUCE_RIEGEL_UNBEKANNT=$<TARGET_PROPERTY:EqCopilot,NAKAMA_UNBEKANNT>` als
„nicht aufloesbar; stilles Verwerfen waere falsch gruen“. Auch ein anders
benanntes Define wie
`NAKAMA_TEST=$<TARGET_PROPERTY:EqCopilot,NAKAMA_UNBEKANNT>` bleibt ROT, weil
die Zielreferenz expandiert werden muss und dabei Defines tragen könnte.

---

## 1. Ticket-Behauptungen

**Stand dieses Abschnitts:** `5d0e9fd` — Positionen ohne eigene Angabe sind an diesen Commit gebunden.

| # | Behauptung | Befehl | Ergebnis | Rohausgabe | Datum |
|---|---|---|---|---|---|
| 1 | **Der gemeinsame Kern ist eine echte Static-Lib**: `NakamaKern.lib` entsteht aus genau vier Übersetzungseinheiten. Vorher übersetzten **sieben** Ziele alle vier Quellen selbst (`nakama_state_anbinden`) und ein achtes (`EqCopSchemaTest`) eine davon — 29 Übersetzungen derselben vier Dateien, jetzt 4 | `cmake --build … --target NakamaKern` | ☑ | [↓ B1](#s8b1) | 2026-08-22 |
| 2 | **K1 fällt**: eine `JucePlugin_*`-Konstante im Kern bricht die Übersetzung — in **jeder** der vier Einheiten, mit benannter Ursache. K2 blieb dabei grün (der Define wurde bewusst nach dessen Lauf gesetzt), die Probe war also isoliert | Probe `target_compile_definitions(NakamaKern PRIVATE JucePlugin_PluginCode=…)` nach dem K2-Aufruf | ☑ **gefallen** | [↓ B2](#s8b2) | 2026-08-22 |
| 3 | **K2 fällt** bei der *realistischen* Regression — einer Linkkante vom Kern zu einem `juce_add_plugin`-Ziel — und benennt Ziel, Eigenschaft und jedes einzelne geerbte Makro | Probe `target_link_libraries(NakamaKern PRIVATE EqCopilot)` vor dem K2-Aufruf | ☑ **gefallen** | [↓ B3](#s8b3) | 2026-08-22 |
| 4 | **K3 fällt bei genau der Lücke, die K1 und K2 strukturell nicht sehen**: ein Stringliteral `"EQ-Copilot"` im Kern. Bau grün, K2 grün — K3 rot | Probe `const char* = "EQ-Copilot";` in `NakamaState.cpp` | ☑ **gefallen** | [↓ B4](#s8b4) | 2026-08-22 |
| 5 | **K3s Gegenprobe trägt K3**: derselbe Scanner findet dieselben Nadeln im gebauten Bundle. Sie hat in der ersten Fassung einen echten Fehler *dieses Beins* gefunden — CIDs liegen im Binärbild nicht als Hextext, sondern als 16 rohe Bytes in COM-vertauschter Ordnung; ohne Gegenprobe hätte K3 dauerhaft falsch grün gemeldet | `py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py` | ☑ | [↓ B5](#s8b5) · [↓ A14](#a14--pruefe_kern_identitaetsfreipy) | 2026-08-22 |
| 6 | **Der Kern trägt keinen einzigen eingefrorenen Identitätswert** — 13 Nadeln aus `plugin-identities-v1.json` (Herstellername/-code, drei Viercodes, Produktname, Bundle, sechs CIDs), gesucht als ASCII, UTF-16LE, roh und COM-vertauscht | A14, Abschnitt [2] | ☑ | [↓ A14](#a14--pruefe_kern_identitaetsfreipy) | 2026-08-22 |
| 7 | **Die Kopf-Fassade hält**: `NakamaKern.lib` enthält genau die vier eigenen Objekte und **kein** JUCE-Modulobjekt — die JUCE-Modulquellen werden nicht in den Kern hineinkompiliert | A14, Abschnitt [3] | ☑ | [↓ A14](#a14--pruefe_kern_identitaetsfreipy) | 2026-08-22 |
| 8 | **Die Identität ist unberührt** (SONDE-001/002): das gebaute `moduleinfo.json`, beide Class-IDs, `JUCE_VST3_CAN_REPLACE_VST2=0` und die CMake-Quelle messen unverändert | `EqCopIdentityTest.exe` | ☑ | [↓ B1 im Kanon](#b1--eqcopidentitytest) | 2026-08-22 |
| 9 | **Verhalten unverändert**: Passthrough bitgleich, 0 Samples Latenz, AnalyseEngine deckungsgleich zur Referenz, Hör-Markierung weiter verriegelt — der Umbau ist ein Bau-Umbau, kein Verhaltens-Umbau | `EqCopNullTest` · `EqCopGoldenTest` · `EqCopMarkierungTest` | ☑ | [↓ §4 A1–A3](#4-kanon-lauf-roh-vom-runner-erzeugt) | 2026-08-22 |
| 10 | **Fremder Host-Harness**: `pluginval --strictness-level 8` SUCCESS am neu gelinkten Bundle (die Linkstruktur des ausgelieferten Artefakts hat sich geändert, also wird sie fremd nachgemessen) | `pluginval.exe --strictness-level 8 --validate-in-process --validate EQ-Copilot.vst3` | ☑ | [↓ B6](#s8b6) | 2026-08-22 |
| 11 | **Die Architektur ist gemessen, nicht geraten**: drei Wegwerf-Experimente entscheiden zwischen den drei möglichen Bauformen — `$<COMPILE_ONLY:>` streift `INTERFACE_SOURCES` **nicht** ab; zwei Kopien derselben Modulquelle linken zwar sauber, halten aber nur solange beide deckungsgleich übersetzt werden; die Fassade übersetzt die Modulquelle genau einmal | Wegwerfprojekt, VS 17 2022 / CMake 3.31.6-msvc6 | ☑ | [↓ B7](#s8b7) | 2026-08-22 |
| 12 | **Kanon-Lauf** mit A14 als Pflichtbein — 18 → **19** Beine | `pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-007a.md -Anhaengen -Titel 'SONDE-007a'` | ☑ 19/19 | [↓ §4](#4-kanon-lauf-roh-vom-runner-erzeugt) | 2026-08-22 |
| 14 | **A14 misst nie ein veraltetes Artefakt.** Aus dem zweiten Selbstaudit: der Baustand-Riegel des Runners (`tools/beweise.ps1:484`) läuft nur über die `.exe`-Beine (`Art -eq 'plugin'`). `NakamaKern.lib` ist eine neue **Art** von gemessenem Artefakt und fiel durch dieses Raster — ohne `-Bauen` hätte A14 eine veraltete Lib messen und grün melden können. Das Bein bewacht seine Frische jetzt selbst | `py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py`, Abschnitt [0] | ☑ **gefallen** | [↓ B9](#s8b9) | 2026-08-22 |
| 13 | **K2b — die JUCE-Konfiguration von Kern und Verbraucher läuft nicht auseinander.** Aus dem Selbstaudit: der Kern übersetzt dieselben JUCE-Kopfdateien mit einer *anderen* Definemenge (12 statt 74). Gemessen weicht genau eine Konfigurationsschraube ab, folgenlos — aber die Divergenz selbst ist der Fehler. Riegel gebaut und gefallen | `cmake -S eq-copilot -B eq-copilot/build` (läuft bei jedem Configure) | ☑ **gefallen** | [↓ B8](#s8b8) | 2026-08-22 |

**Was dieses Ticket ausdrücklich NICHT behauptet:**

- Es ist **kein** Umzug hinter die §53.4-Verzeichnisgrenzen (`plugin/core/…`).
  Der Entwurf verlangt das ausdrücklich inkrementell: „Die vorhandenen Dateien
  werden inkrementell hinter diese Grenzen verschoben. P0 und P1 sind kein
  Freibrief für einen Big-Bang-Rename."
- Es baut **keine** zweite oder dritte App. Dass der Kern identitätsfrei ist,
  ist die *Voraussetzung* dafür; die drei Ziele sind `SONDE-007b` (S9).
- `vertrag/NakamaTelemetrie.cpp` bleibt außerhalb des Kerns (braucht die
  FlatBuffers-Laufzeit, liest heute nur `EqCopSchemaTest`) — sein Umzug gehört
  zu `SONDE-010`.
- Die Identität steht weiterhin als Literal in `plugin/CMakeLists.txt`, nicht
  aus `plugin-identities-v1.json` gelesen. §53.4 verlangt das für die **dünnen
  Target-Schichten**, und die entstehen mit S9; `EqCopIdentityTest` friert die
  CMake-Quelle heute genau in dieser Form ein. Notiert als NAK-52.

---

## 2. Rohe Ausgaben

**Stand dieses Abschnitts:** `5d0e9fd` — Positionen ohne eigene Angabe sind an diesen Commit gebunden.

<a id="s8b1"></a>
### B1 · Der Kern baut aus vier Einheiten (Behauptung 1)

**Befehl:** `cmake --build eq-copilot/build --config Release --target NakamaKern`

```text
  NakamaKanon.cpp
  NakamaParameter.cpp
  NakamaState.cpp
  NakamaVertrag.cpp
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
EXITCODE=0
```

Konfigurierzeit, derselbe Lauf:

```text
-- Nakama-Kern: JUCE-Kopffassade 'NakamaKernJuce' aus 5 Modulzielen abgeleitet.
-- Nakama-Kern: K2 gruen — 7 Ziele in der Linkhuelle von 'NakamaKern', keine JucePlugin_-Konstante.
-- Configuring done (5.0s)
-- Generating done (1.6s)
EXITCODE=0
```

Die Fassade wurde über **vier** Module benannt (`juce_core`, `juce_events`,
`juce_data_structures`, `juce_cryptography`) und leitet aus **fünf** ab: der
Hüllen-Läufer hat `juce_atomic_wrapper` transitiv mitgenommen. Genau dafür
läuft er die Hülle ab, statt die vier Namen abzuschreiben.

<a id="s8b2"></a>
### B2 · K1 fällt (Behauptung 2)

**Probe** (nach `nakama_kern_riegel_pruefen(NakamaKern)` eingesetzt, damit K2
sie nicht vorher abfängt und die Probe wirklich K1 misst):

```cmake
target_compile_definitions(NakamaKern PRIVATE JucePlugin_PluginCode=0x45716370)  # RIEGELPROBE K1
```

```text
-- Nakama-Kern: K2 gruen — 7 Ziele in der Linkhuelle von 'NakamaKern', keine JucePlugin_-Konstante.
C:\Users\phili\Projekte\Nakama\eq-copilot\plugin\state\NakamaKernRiegel.h(95,1): error C1189: #error:  "S8/SONDE-007a:
Der gemeinsame Kern sieht eine JucePlugin_*-Konstante. Damit traegt sein Objektcode die Identitaet EINES Bundles, und
alle drei Apps (Gen, Probeeq, Suna) erben sie - der Identitaets-Golden aus S2 faellt. Ursache ist fast immer eine
Linkkante von NakamaKern zu einem juce_add_plugin-Ziel: dessen JucePlugin_*-Defines sind PUBLIC
(JUCEUtils.cmake:1543). Identitaet gehoert in die duenne Target-Schicht, nicht in den Kern (Entwurf §53.4)."
[C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaKern.vcxproj]
EXITCODE=1
```

Der Fehler steht **viermal**, einmal je Übersetzungseinheit (hier eine Fassung;
die drei weiteren sind wortgleich, nur mit anderer Quelldatei im Kontext). Dass
K2 in derselben Ausgabe grün meldet, ist der Beweis, dass die Probe isoliert
war — sie hat K1 zum Fallen gebracht, nicht K2.

<a id="s8b3"></a>
### B3 · K2 fällt (Behauptung 3)

**Probe** — kein künstlicher Define, sondern die Regression, die im Alltag
passiert: jemand linkt den Kern gegen das Plugin-Ziel.

```cmake
target_link_libraries(NakamaKern PRIVATE EqCopilot)  # RIEGELPROBE K2
```

```text
CMake Error at cmake/NakamaKern.cmake:142 (message):
  S8/SONDE-007a K2: Der gemeinsame Kern 'NakamaKern' sieht
  JucePlugin_-Konstanten.

    EqCopilot [COMPILE_DEFINITIONS] JucePlugin_Build_AU=0
      EqCopilot [COMPILE_DEFINITIONS] JucePlugin_Build_AUv3=0
      EqCopilot [COMPILE_DEFINITIONS] JucePlugin_Build_AAX=0
      EqCopilot [COMPILE_DEFINITIONS] JucePlugin_Build_LV2=0
      EqCopilot [COMPILE_DEFINITIONS] JucePlugin_Build_Standalone=0
      EqCopilot [COMPILE_DEFINITIONS] JucePlugin_Build_Unity=0
      EqCopilot [COMPILE_DEFINITIONS] JucePlugin_Build_VST=0
      EqCopilot [COMPILE_DEFINITIONS] JucePlugin_Build_VST3=1
      EqCopilot [COMPILE_DEFINITIONS] JUCE_STANDALONE_APPLICATION=JucePlugin_Build_Standalone
```

Der Riegel bricht **vor** der ersten Übersetzungseinheit ab und nennt Ziel,
Eigenschaft und jedes geerbte Makro.

<a id="s8b4"></a>
### B4 · K3 fällt, wo K1 und K2 nicht greifen können (Behauptung 4)

**Probe** — ein Stringliteral, kein Makro. Weder der Präprozessor noch die
CMake-Eigenschaften können das sehen:

```cpp
namespace { const char* riegelprobeK3 = "EQ-Copilot"; }  // RIEGELPROBE K3
```

```text
-- Nakama-Kern: K2 gruen — 7 Ziele in der Linkhuelle von 'NakamaKern', keine JucePlugin_-Konstante.
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  FEHLER  NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT  [ascii]
19 ok, 1 Fehler
FEHLGESCHLAGEN:
  - NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT  [ascii]
EXITCODE=2
```

Konfigurieren grün, Bau grün, K1 grün, K2 grün — **nur** K3 rot. Das ist der
Grund, warum drei Riegel und nicht einer.

<a id="s8b5"></a>
### B5 · Die Gegenprobe widerlegt die erste Fassung von K3 (Behauptung 5)

Erster Lauf des Beins, **vor** der Berichtigung. Der Scanner suchte die
Class-IDs als Hextext:

```text
[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii]
  FEHLER  Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [NICHT GEFUNDEN]
  FEHLER  Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [NICHT GEFUNDEN]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT

18 ok, 2 Fehler
EXITCODE=2
```

Die beiden `ok` in Abschnitt [2] waren **falsch grün**: der Kern trug die CIDs
tatsächlich nicht, aber der Scanner hätte sie auch dann nicht gefunden, wenn er
sie getragen hätte. Berichtigt: CIDs werden als 16 rohe Bytes gesucht, in
Original- **und** COM-vertauschter Ordnung — die Ordnung, vor der
`plugin-identities-v1.json` in `hinweis_zur_byteordnung` selbst warnt
(„der TUID im Speicher ist unter COM_COMPATIBLE (Windows) in den ersten acht
Bytes vertauscht"). Nach der Berichtigung findet die Gegenprobe sie als
`roh16-com`; die volle Ausgabe steht im Kanon unter
[A14](#a14--pruefe_kern_identitaetsfreipy).

🔑 **Lehre für den Baum:** Ein Riegel, der etwas NICHT findet, sagt nichts, bis
gezeigt ist, dass er überhaupt etwas finden kann. Die Gegenprobe gehört in das
Bein hinein, nicht in ein Protokoll daneben.

<a id="s8b6"></a>
### B6 · `pluginval` Strenge 8 am neu gelinkten Bundle (Behauptung 10)

**Befehl:** `%TEMP%\pluginval.exe --strictness-level 8 --validate-in-process --timeout-ms 300000 --validate eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3` · **Exitcode:** 0

```text
Starting tests in: pluginval / Disabling non-main busses...
Completed tests in pluginval / Disabling non-main busses
-----------------------------------------------------------------
Starting tests in: pluginval / Restoring default layout...
Main bus num input channels: 2
Main bus num output channels: 2
Completed tests in pluginval / Restoring default layout
-----------------------------------------------------------------
Starting tests in: pluginval / Fuzz parameters...
Completed tests in pluginval / Fuzz parameters
SUCCESS
EXITCODE=0
```

<a id="s8b7"></a>
### B7 · Die drei Messungen, auf denen die Bauform beruht (Behauptung 11)

Wegwerfprojekt, `Visual Studio 17 2022` / `cmake version 3.31.6-msvc6`. Eine
INTERFACE-Bibliothek `ModulIface` mit `modul.cpp` als `INTERFACE_SOURCES` bildet
nach, was JUCE mit seinen Modulen tut
(`JUCEModuleSupport.cmake:505` `_juce_add_interface_library`).

**Messung 1 — `$<COMPILE_ONLY:>` streift `INTERFACE_SOURCES` nicht ab.** Die
CMake-Dokumentation sagt zu dieser Frage nichts („provide compilation usage
requirements without any linking requirements"), also gemessen:

```cmake
target_link_libraries(Kern PRIVATE $<COMPILE_ONLY:ModulIface>)
```
```text
  kern.cpp
  modul.cpp
  Kern.vcxproj -> …\Release\Kern.lib
```

`modul.cpp` landet trotzdem in `Kern`. Der Weg ist damit tot.

**Messung 2 — zwei Kopien derselben Modulquelle linken sauber.** Kern und
Verbraucher übersetzen `modul.cpp` beide:

```text
  modul.cpp
  modul.cpp
  Verbraucher.vcxproj -> …\Release\Verbraucher.exe
EXITCODE=0
```

Kein `LNK2005`: MSVC zieht ein `.lib`-Mitglied nur, wenn sonst ein Symbol offen
bliebe, und die eigene `modul.obj` des Verbrauchers deckt alles ab. **Geht
heute** — aber es hält nur, solange beide Kopien deckungsgleich übersetzt
werden. Für einen Kern, den drei Bundles mit unterschiedlichen Modulmengen
linken, ist das zu wenig.

**Messung 3 — die Fassade.** Includes und Defines aus dem Modul-Target
abgeleitet, Quellen nicht geerbt; ein `#error`-Gegentest im Kern prüft, dass die
Defines wirklich ankommen:

```cmake
target_include_directories(ModulKopf INTERFACE $<TARGET_PROPERTY:ModulIface,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_definitions(ModulKopf INTERFACE $<TARGET_PROPERTY:ModulIface,INTERFACE_COMPILE_DEFINITIONS>)
```
```text
  kern.cpp
  modul.cpp
  Verbraucher.vcxproj -> …\Release\Verbraucher.exe
EXITCODE=0
```

`kern.cpp` übersetzt (der `#error` schwieg, die Defines waren also da),
`modul.cpp` **genau einmal**, Link sauber. Das ist die gebaute Form.

<a id="s8b8"></a>
### B8 · K2b aus dem Selbstaudit (Behauptung 13)

Der Selbstaudit nach dem ersten Commit stellte die Frage, die die drei Riegel
nicht beantworten: der Kern übersetzt dieselben JUCE-Kopfdateien wie seine
Verbraucher — aber mit welcher Konfiguration? Gemessen an den erzeugten
`.vcxproj`:

```text
KERN: 12 Defines | PLUGIN: 74 Defines

--- nur im PLUGIN (Kern sieht sie NICHT), gekuerzt ---
JUCE_DISPLAY_SPLASH_SCREEN=0
JUCE_MODULE_AVAILABLE_juce_audio_basics=1        (… 11 Modulzeilen …)
JUCE_SHARED_CODE=1
JUCE_STANDALONE_APPLICATION=JucePlugin_Build_Standalone
JUCE_USE_CURL=0
JUCE_VST3_CAN_REPLACE_VST2=0
JUCE_WEB_BROWSER=0
JucePlugin_ManufacturerCode=0x45766e61           (… 46 JucePlugin_-Zeilen …)
JucePlugin_Name="EQ-Copilot"
NAKAMA_HOST_BRIDGE=1

--- nur im KERN ---
NAKAMA_KERN_UEBERSETZUNG=1
```

Die 46 `JucePlugin_`-Zeilen fehlen dem Kern — das ist der Zweck des Tickets.
Die JUCE-**Konfigurations**schrauben fehlten ihm aber ebenfalls, und das ist
kein Zweck, sondern ein Loch. Folgenabschätzung, gemessen statt geschätzt —
Grep über **alle** Kopfdateien der vier Module, die der Kern einbindet:

```text
=== juce_core ===
juce_core/juce_core.h
=== juce_events ===
=== juce_data_structures ===
=== juce_cryptography ===

juce_core/juce_core.h:145:/** Config: JUCE_USE_CURL
juce_core/juce_core.h:151:#ifndef JUCE_USE_CURL
juce_core/juce_core.h:152: #define JUCE_USE_CURL 1
```

Ein einziger Treffer, und der ist der Konfigblock selbst: kein Header
verzweigt auf eine der fehlenden Schrauben, und der Kern übersetzt ohnehin
keine `juce_core`-Quelle. **Heute folgenlos** — aber der Kern sähe `1`, wo das
Projekt `0` sagt, und ein künftiger JUCE-Stand kann daraus jederzeit eine
ODR-Verletzung machen. Also geschlossen statt notiert: die drei Schrauben
stehen an der Kopf-Fassade, und **K2b** hält beide Mengen zusammen.

Riegel gefallen (Probe: `JUCE_USE_CURL=0` an der Fassade entfernt):

```text
-- Nakama-Kern: K2 gruen — 7 Ziele in der Linkhuelle von 'NakamaKern', keine JucePlugin_-Konstante.
CMake Error at cmake/NakamaKern.cmake:242 (message):
  S8/SONDE-007a K2b: Der Kern 'NakamaKern' uebersetzt die JUCE-Kopfdateien
      JUCE_USE_CURL=0
  Derselbe Header kann dort dann etwas anderes bedeuten als hier.  Trag die
  Schraube an der Kopf-Fassade nach (plugin/CMakeLists.txt, NakamaKernJuce).
```

Grün nach dem Nachtragen:

```text
-- Nakama-Kern: JUCE-Kopffassade 'NakamaKernJuce' aus 5 Modulzielen abgeleitet.
-- Nakama-Kern: K2 gruen — 7 Ziele in der Linkhuelle von 'NakamaKern', keine JucePlugin_-Konstante.
-- Nakama-Kern: K2b gruen — JUCE-Konfiguration von 'NakamaKern' deckt 'EqCopilot'.
EXITCODE=0
```

**Trägt der Ausschlusssatz?** K2b vergleicht nicht alles — vier Define-Familien
sind ausgenommen, weil sie legitim verschieden sind. Ein Ausschluss ist aber
nur so gut wie sein Beleg, also nachgemessen an denselben vier Modulen:

```text
=== JUCE_MODULE_AVAILABLE_ ===
juce_core/native/juce_BasicNativeHeaders.h:43:  #if JUCE_MODULE_AVAILABLE_juce_opengl
juce_core/native/juce_BasicNativeHeaders.h:56:  #if JUCE_MODULE_AVAILABLE_juce_opengl
=== JUCE_SHARED_CODE ===
=== JUCE_STANDALONE_APPLICATION ===
=== JUCE_VST3_CAN_REPLACE_VST2 ===
```

Drei der vier kommen in **keinem** Header der Kernmodule vor. Der vierte
trifft zweimal, beide auf `juce_opengl` gegated — ein Modul, das weder der Kern
noch `EqCopilot` linkt (die Definemenge des Plugins oben zählt elf
`JUCE_MODULE_AVAILABLE_`-Zeilen, `juce_opengl` ist keine davon). Beide Seiten
sehen es gleich undefiniert. Der Ausschlusssatz lässt also nichts durch, das
die Übersetzung der Kernmodul-Header verändern könnte.

⚠️ **Für S9 offen:** K2b nimmt genau **eine** Referenz. Mit Gen, Probeeq und
Suna ist zu entscheiden, ob der Kern gegen alle drei geprüft wird oder die drei
untereinander gleich konfiguriert sein müssen. Heute gibt es einen
Verbrauchertyp, die Frage stellt sich noch nicht.

Der Kanon-Lauf in §4 ist **nach** dieser Änderung gefahren; die Übersetzung des
Kerns hat sich dadurch geändert, also wäre der frühere Lauf kein Beleg mehr
gewesen.

<a id="s8b9"></a>
### B9 · Der Frische-Riegel, zweiter Selbstaudit (Behauptung 14)

Ein Bein, das ein Artefakt misst, ist nur so viel wert wie die Frische dieses
Artefakts. Der Runner hat dafür einen Riegel — er misst Quell-mtime gegen
Binär-mtime und verweigert mit Exitcode 4 die Beglaubigung. Gemessen:

```text
tools/beweise.ps1:484:foreach ($eintrag in ($kanon | Where-Object { $_.Art -eq 'plugin' })) {
```

Er läuft **nur** über die `.exe`-Beine. `NakamaKern.lib` ist die erste
Bibliothek, die der Kanon misst, und fiel durch dieses Raster: ohne `-Bauen`
hätte A14 eine veraltete Lib messen und grün melden können — genau der
Fehlertyp, gegen den der Runner-Riegel am 20.08. gebaut wurde.

Geschlossen im Bein selbst (Abschnitt `[0]`), statt den Runner umzubauen: A14
vergleicht die mtime der Lib gegen `plugin/state/*` und
`plugin/vertrag/NakamaVertrag.*` — dieselbe ehrliche Heuristik, nur am
richtigen Artefakt. Beide Zustände vorgeführt:

```text
[0] Frische - misst dieses Bein den aktuellen Quellstand?
21 ok, 0 Fehler

--- jetzt Riegel fallen lassen: Quelle beruehren ---
[0] Frische - misst dieses Bein den aktuellen Quellstand?
20 ok, 1 Fehler
FEHLGESCHLAGEN:
EXITCODE=2
```

🔑 **Lehre:** Wer dem Kanon eine neue ART von Artefakt hinzufügt, erbt seine
Frischeprüfung nicht mit. Der Runner-Riegel ist auf `.exe` geschnitten — jedes
Bein, das etwas anderes misst, muss seine Frische selbst belegen.

---

## 3. T1 — Selbstprüfung

**Stand dieses Abschnitts:** `5d0e9fd` — Positionen ohne eigene Angabe sind an diesen Commit gebunden.

| Frage | Antwort | Beleg |
|---|---|---|
| Läuft jede Behauptung als Befehl, nicht von Hand? | ja — A14 ist Kanon-Bein; K1/K2 laufen bei **jedem** Bau bzw. Configure mit | §4 A14 |
| Ist jeder Riegel beim Fallen gesehen worden? | ja, alle **fünf**, je mit der Ursache, gegen die er gerichtet ist | B2, B3, B4, B8, B9 |
| Kann A14 auf einem veralteten Artefakt grün melden? | nein mehr — der Runner-Riegel deckt nur `.exe`, das Bein bewacht seine Lib jetzt selbst | B9 |
| Übersetzt der Kern die JUCE-Kopfdateien wie seine Verbraucher? | jetzt ja — im Selbstaudit wich eine Konfigurationsschraube ab; geschlossen, nicht notiert · ⚠️ **T2-3 hat diese Zeile eingeschränkt:** sie galt nur für **Defines**. Die Übersetzungsschalter wichen ab (`/W1` statt `/W4`); erst K2c macht den Satz wahr — §5/§6 | B8 · §6 |
| Kann ein Riegel falsch grün melden? | K3 konnte es — gefunden und berichtigt; die Gegenprobe ist jetzt Teil des Beins | B5 |
| Beruht die Bauform auf Messung oder auf Annahme? | Messung, drei Experimente | B7 |
| Ist die eingefrorene Identität berührt? | nein | §4 B1 `EqCopIdentityTest` |
| Ist Audio-/Analyseverhalten berührt? | nein | §4 A1–A3 |
| Hält ein fremder Host-Harness das neu gelinkte Bundle? | ja, Strenge 8 SUCCESS | B6 |
| Gegenpfad vorhanden? | ja — `nakama_kern_anbinden()` ist die eine Stelle, die den Kern anbindet; sie löst `nakama_state_anbinden()` vollständig ab, es gibt keine zweite Anbindungsform | `plugin/CMakeLists.txt` |
| Was ist **nicht** bewiesen? | **T2 ist offen** — kein Frischkontext-Prüferurteil. Außerdem: dass die Fassade auch für die Modulmengen von Probeeq und Suna trägt, ist erst mit S9 messbar; heute gibt es genau einen Verbrauchertyp. | — |

---

---


---

## 5. T2 — Frischkontext-Prüfer

**Stand dieses Abschnitts:** `75afae2` - der Stand, der dem Prüfer am
23.08.2026 vorgelegt wurde (Zeile `Vorgelegt` unten). Der Abschnitt hält
ein abgeschlossenes Prüfurteil fest und wird nicht nachgezogen; seine
Zahlen und Zeilennummern gehören zu jenem Stand (S8 Runde 14,
30.08.2026 - die erweiterten Inventar-Muster haben hier `13 Nadeln` als
lebend gezählt; heute sind es 17).

| Feld | Wert |
|---|---|
| Prüfer | eigene Session, frischer Kontext — sah keine Zusammenfassung des Baus, nur `git diff dafa5a5..HEAD` (18 Dateien), dieses Manifest, den Gate-Text und die Quelldateien |
| Vorgelegt | 1. `git diff dafa5a5..HEAD` · 2. dieses Manifest (Stand `75afae2`) · 3. Gate-Text §53.4 wörtlich (Kopftabelle) · 4. `docs/bauaufteilung-sonden.md` §2 (T3-Regel: Befund für Befund gegen die **Quelldatei**) |
| Bruchaufträge | **A** Kern trägt doch Identität · **B** ein Riegel meldet falsch grün · **C** Zahl im Manifest ≠ Zahl im Code · **D** Fassade zieht JUCE-Modulquellen mit · **E** Kern und Verbraucher übersetzen JUCE verschieden · **F** Umbau ändert Verhalten/Identität · **G** Manifest behauptet mehr, als es misst · **H** Gegenpfad unvollständig (alte Anbindung lebt weiter) |
| Urteil | ☐ PASS ☑ **NEEDS_WORK** — **A/B/D/F/H: kein Pfad gefunden**, die zentrale Behauptung hält unter eigener Messung. **C: sauber** (alle acht nachgezählten Zahlen stimmen). **E und G: fünf Befunde**, davon einer eine echte, mit diesem Änderungssatz eingeführte Regression (T2-1). Keiner falsifiziert das Ticket; alle sind klein und lokal. |
| Datum | 2026-08-23 |

### Was der Prüfer selbst neu gemessen hat (nicht aus dem Manifest übernommen)

| Probe | Ergebnis |
|---|---|
| Definemengen aus den **erzeugten** `.vcxproj` neu ausgezählt (`NakamaKern` vs. `EqCopilot`, Release\|x64) | 15 vs. 74. Im Kern **keine einzige** `JucePlugin_`-Zeile. Die einzigen `JUCE_`-Defines, die ihm fehlen, sind exakt die vier von K2b ausgenommenen Familien (`JUCE_MODULE_AVAILABLE_*` ×10, `JUCE_SHARED_CODE`, `JUCE_STANDALONE_APPLICATION`, `JUCE_VST3_CAN_REPLACE_VST2`) plus `NAKAMA_HOST_BRIDGE`. Der Ausschlusssatz von K2b lässt heute nachweislich nichts durch |
| A14 selbst gefahren | `21 ok, 0 Fehler`, EXITCODE=0 |
| Frische-Riegel `[0]` selbst zum Fallen gebracht (mtime der Lib auf 2020-01-01, danach zurückgesetzt) | `FEHLER  NakamaKern.lib ist nicht aelter als die Kernquellen  [NakamaKanon.cpp, NakamaKanon.h, NakamaKernRiegel.h, NakamaParameter.cpp, NakamaParameter.h, NakamaState.cpp, NakamaState.h, NakamaVertrag.cpp, NakamaVertrag.h]` — Behauptung 14 trägt |
| Zahlen nachgezählt statt geglaubt | 29 → 4 Übersetzungen ☑ · 8 Verbraucher (`nakama_kern_anbinden`) ☑ · 13 Nadeln ☑ · 46 Makros in K1 ☑ · 7 Ziele in der K2-Hülle (2 + 5 abgeleitete) ☑ · 5 Modulziele in der Fassade ☑ · 19 Kanon-Beine (23 Einträge − 4 geplante) ☑ |
| Gegenpfad: lebt die alte Anbindung irgendwo weiter? | nein — `grep -rn nakama_state_anbinden` über `*.txt *.cmake *.md *.ps1` außerhalb der Manifeste: **0 Treffer** |
| ABI-relevante Schalter Kern vs. Verbraucher | identisch: `RuntimeLibrary` MultiThreadedDLL · `LanguageStandard` stdcpp20 · `ExceptionHandling` Sync. Ein CRT-/Standard-Bruch, der beim Linken still durchginge, liegt **nicht** vor |
| Archivmitglieder der `.lib` | genau `NakamaKanon.obj NakamaParameter.obj NakamaState.obj NakamaVertrag.obj`, kein `juce*`-Objekt — die Kopf-Fassade hält (Bruchauftrag D leer) |

### Befunde

**T2-1 · `/W4` fällt für genau die Quellen, die alle drei Apps teilen werden** (Bruchauftrag E, **echte Regression dieses Änderungssatzes**)

`juce::juce_recommended_warning_flags` (auf MSVC exakt `/W4`,
`JUCEHelperTargets.cmake:50`) hängt **PUBLIC** an jedem Verbraucher-Ziel. Vor S8
lagen die vier Quellen per `target_sources()` **in** diesen Zielen und wurden
damit unter `/W4` übersetzt. `NakamaKern` linkt nur `NakamaKernJuce` — die
Kopf-Fassade leitet aus den **Modul**zielen ab, und die Helfer-Ziele
`juce_recommended_{config,warning}_flags` sind keine Module. Gemessen an den
erzeugten Projektdateien:

```text
NakamaKern              : 0 Zeilen <WarningLevel>
EqCopilot               : 4 Zeilen <WarningLevel>Level4
EqCopNullTest           : 4 Zeilen <WarningLevel>Level4
EqCopSchemaTest         : 4 Zeilen <WarningLevel>Level4
EqCopStateMigrationTest : 4 Zeilen <WarningLevel>Level4

CMAKE_CXX_FLAGS:STRING=/DWIN32 /D_WINDOWS /EHsc      (kein /W — cl-Vorgabe ist /W1)
```

Ebenso fehlen `/Ox` und `/MP` aus `juce_recommended_config_flags`
(`<Optimization>MaxSpeed` statt `Full`). **Wie schlimm heute:** nicht schlimm —
der Prüfer hat alle vier Quellen mit der Definemenge des Kerns einmal per Hand
unter `/W1` und unter `/W4` übersetzt:

```text
=== /W1 ===                              === /W4 ===
  state\NakamaKanon.cpp    : 0 Warnung(en)  state\NakamaKanon.cpp    : 0 Warnung(en)
  state\NakamaParameter.cpp: 0 Warnung(en)  state\NakamaParameter.cpp: 0 Warnung(en)
  state\NakamaState.cpp    : 0 Warnung(en)  state\NakamaState.cpp    : 0 Warnung(en)
  vertrag\NakamaVertrag.cpp: 0 Warnung(en)  vertrag\NakamaVertrag.cpp: 0 Warnung(en)
```

Es wird also **nichts verschwiegen** — verloren ist der Wächter, nicht ein
Befund. Aber er ist ausgerechnet dort verloren, wo ab S9 der geteilte Boden
aller drei Apps liegt und wo Code hinzukommt. Kein `juce_recommended_lto_flags`
an den Kern: `/GL`-Objekte gingen in Konsolenziele ohne `-LTCG`.

**T2-2 · Der Ausschlusssatz von K2b nennt einen Riegel, den es nicht gibt** (Bruchauftrag G)

`cmake/NakamaKern.cmake:206-208` begründet den Ausschluss von
`JUCE_SHARED_CODE` / `JUCE_STANDALONE_APPLICATION` / `JUCE_VST3_CAN_REPLACE_VST2`
mit „K1 verbietet die **ersten beiden** im Kern ausdruecklich". K1
(`plugin/state/NakamaKernRiegel.h:43-89`) führt `JUCE_SHARED_CODE` — aber
`JUCE_STANDALONE_APPLICATION` steht auf keiner der 46 Zeilen. Der Ausschluss ist
trotzdem sicher, nur durch einen **anderen** Riegel: der Wert lautet
`JUCE_STANDALONE_APPLICATION=JucePlugin_Build_Standalone`, und K2s Regex greift
in den Wert hinein — die eigene Rohausgabe B3 zeigt genau diese Zeile unter den
K2-Funden. Die Begründung zeigt auf den falschen Wächter.

**T2-3 · „dieselbe Konfiguration" ist weiter behauptet als gemessen** (Bruchauftrag G)

Drei Stellen sagen es ohne Einschränkung — §1 Arbeitsteilung („die JUCE-**Kopfdateien**
mit derselben Konfiguration übersetzen"), §3 T1 („Übersetzt der Kern die
JUCE-Kopfdateien wie seine Verbraucher? — jetzt ja") und `CLAUDE.md` („K2b Kern
und Verbraucher übersetzen JUCE gleich konfiguriert"). `nakama_kern_konfig_pruefen()`
liest ausschließlich `COMPILE_DEFINITIONS`; **Übersetzungsschalter** liegen
außerhalb seiner Reichweite, und sie weichen heute ab (T2-1). Für die Defines
ist die Aussage nachgemessen richtig; als Satz greift sie zu weit.

**T2-4 · A14 wählt bei zwei Konfigurationen still die alphabetisch erste** (Bruchauftrag B, latent)

`pruefe_kern_identitaetsfrei.py:187` nimmt `sorted(bau.glob("plugin/**/NakamaKern.lib"))[0]`,
`:175` verfährt für das Gegenprobe-Bundle genauso. Existiert neben `Release/`
auch ein `Debug/`-Stand, sortiert `Debug` **vor** `Release` — A14 misst dann die
Debug-Lib, während der Runner Release baut und beglaubigt, und der Frische-Riegel
`[0]` bewacht ebenfalls die falsche Datei. Heute liegt nur `Release/` im
Baumverzeichnis (nachgesehen), der Fehler ist also latent, nicht aktiv. Ein Bein,
das das Artefakt misst, darf sich das Artefakt nicht per Zufall der
Sortierreihenfolge aussuchen.

**T2-5 · Der Frische-Riegel deckt die Baubeschreibung nicht** (Bruchauftrag B, klein)

`[0]` vergleicht die mtime der Lib gegen `plugin/state/*` und
`plugin/vertrag/NakamaVertrag.*` — nicht gegen `plugin/CMakeLists.txt` und
`cmake/NakamaKern.cmake`. Dass gerade die Baubeschreibung die Übersetzung des
Kerns ändert, sagt B8 selbst: „Der Kanon-Lauf in §4 ist **nach** dieser Änderung
gefahren; die Übersetzung des Kerns hat sich dadurch geändert, also wäre der
frühere Lauf kein Beleg mehr gewesen." Genau dieser Fall — Fassaden-Defines
geändert, keine `.cpp` berührt — lässt A14 heute grün auf einer veralteten Lib.

### Was der Prüfer **nicht** geprüft hat

- Den Kanon-Lauf in §4 nicht vollständig nachgefahren; A14 einzeln neu gefahren,
  die übrigen 18 Beine aus dem Manifest übernommen.
- `pluginval` für das Urteil nicht wiederholt (B6 übernommen) — in der
  Nacharbeit dann doch gefahren, weil T2-1 den Kern neu übersetzt (§6).
- Die drei Wegwerf-Experimente aus B7 nicht nachgebaut; ihre **Folge** ist
  stattdessen am Artefakt gegengeprüft (Archivmitglieder, Bruchauftrag D).
- Ob die Fassade für die Modulmengen von Probeeq und Suna trägt — mit einem
  Verbrauchertyp nicht messbar, wie das Manifest selbst sagt (S9).

---

## 6. Nacharbeit zu den T2-Befunden

**Stand dieses Abschnitts:** `26b94c5` — Positionen ohne eigene Angabe sind an diesen Commit gebunden.

Alle fünf am selben Tag geschlossen, jeder gegen die **Quelldatei** verifiziert
(T3-Regel, `docs/bauaufteilung-sonden.md` §2). Rohausgabe des Kanon-Laufs danach
unten unter „Kanon-Lauf - SONDE-007a T2-Nacharbeit".

| Befund | Gegen die Quelldatei verifiziert? | Ausgang |
|---|---|---|
| **T2-1** `/W4` und `/Ox` fehlen dem Kern | ☑ ja (`NakamaKern.vcxproj`: 0 Zeilen `<WarningLevel>` · `JUCEHelperTargets.cmake:50`, `:122-123`) | **gefixt:** `target_link_libraries(NakamaKern PRIVATE juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)`. Kein `lto_flags` — `/GL`-Objekte gingen in Konsolenziele ohne `-LTCG`. Nachgemessen: Kern und `EqCopilot` tragen jetzt beide `<WarningLevel>Level4` ×4 und `<Optimization>Full` ×3 + `Disabled` ×1; `WholeProgramOptimization` steht weiterhin **nur** am Plugin. Der Bau unter `/W4` bringt **0 Warnungen** |
| **T2-2** Ausschlusssatz nennt K1 statt K2 | ☑ ja (`NakamaKernRiegel.h:43-89` — `JUCE_STANDALONE_APPLICATION` steht auf keiner Zeile; `SONDE-007a.md` B3 zeigt sie unter den K2-Funden) | **gefixt (Text):** der Kommentar in `cmake/NakamaKern.cmake` nennt jetzt den Riegel, der den Ausschluss wirklich trägt, und sagt warum (der **Wert** trägt den Präfix) |
| **T2-3** „dieselbe Konfiguration" weiter behauptet als gemessen | ☑ ja (`nakama_kern_konfig_pruefen()` liest ausschließlich `COMPILE_DEFINITIONS`) | **gefixt, nicht abgeschwächt:** neuer Riegel **K2c** `nakama_kern_schalter_pruefen()`. Er vergleicht nicht einzelne Schalter (versionsabhängig, in Generatorausdrücken versteckt), sondern deren **Quelle**: jedes `juce_recommended_*`-Ziel in der Hülle der Referenz muss auch in der Hülle des Kerns liegen. Damit ist die Aussage gemessen statt behauptet |
| **T2-4** A14 nimmt `sorted(...)[0]` | ☑ ja (`pruefe_kern_identitaetsfrei.py:187`, `:175`) | **gefixt:** `waehle_release()` bevorzugt die Release-Fassung für Lib **und** Gegenprobe-Bundle und bricht mit Exit 3 ab, wenn mehrere Kandidaten ohne Release-Fassung dastehen — statt still den alphabetisch ersten zu nehmen |
| **T2-5** Frische-Riegel sieht die Baubeschreibung nicht | ☑ ja (`pruefe_kern_identitaetsfrei.py:218-221`) | **gefixt im dritten Anlauf** — die ersten beiden waren falsch, siehe unten |

### K2c beim Fallen (Probe: `warning_flags` am Kern entfernt)

```text
-- Nakama-Kern: K2 gruen — 8 Ziele in der Linkhuelle von 'NakamaKern', keine JucePlugin_-Konstante.
-- Nakama-Kern: K2b gruen — JUCE-Konfiguration von 'NakamaKern' deckt 'EqCopilot'.
CMake Error at cmake/NakamaKern.cmake:304 (message):
  S8/SONDE-007a K2c: Der Kern 'NakamaKern' uebersetzt ohne Empfehlungsschalter, die
  'EqCopilot' traegt.  Fehlend am Kern:
      juce_recommended_warning_flags
EXITCODE=1
```

⚠️ **Reihenfolge ist hier tragend:** K2c liest die Linkhülle der **Referenz**.
`EqCopilot` hängt seine Empfehlungsschalter erst weiter unten in
`plugin/CMakeLists.txt` an sich — am Ort von K2/K2b gerufen hätte K2c eine leere
Menge gesehen und wäre **still grün** gewesen. Der Aufruf steht deshalb nach
`target_link_libraries(EqCopilot …)`, mit einer Zeile an der alten Stelle, die
das sagt.

### T2-5: drei Anläufe, zwei davon gemessen falsch

Der Befund war richtig, die ersten beiden Reparaturen nicht. Beide fielen im
Alltag desselben Tages auf, nicht im Nachdenken — deshalb stehen sie hier.

**Anlauf 1 — mtime auf die handgeschriebenen Dateien** (`plugin/CMakeLists.txt`,
`cmake/NakamaKern.cmake`). Fiel korrekt, als die Baubeschreibung sich änderte:

```text
[0] Frische - misst dieses Bein den aktuellen Quellstand?
  FEHLER  NakamaKern.lib ist nicht aelter als Kernquellen und Baubeschreibung  [CMakeLists.txt, NakamaKern.cmake]
```

Und fiel dann **falsch**: die erste S9-Änderung berührte den Identitätsblock von
`EqCopilot` — den Kern geht das nichts an — und färbte A14 im Kanon rot
(`ROT - 1 von 19`). Schlimmer: es blieb rot, weil ein Bau die Lib nicht neu
linkte.

**Anlauf 2 — mtime auf die erzeugte `NakamaKern.vcxproj`.** Die Idee war besser
begründet (gemessen: ein zweites Configure ohne Edit lässt ihren Zeitstempel
stehen, CMake schreibt sie nur bei Inhaltsänderung neu). Trotzdem falsch, und
das zeigte erst die Messung: **MSBuild entscheidet über `.tlog`-Dateien, nicht
über den Zeitstempel der Projektdatei.** Ein Bau nach der Änderung linkte die
Lib nicht neu, der Riegel hing genauso.

🔑 **Lehre:** Eine mtime-Wache taugt nur an einer Datei, die der Bau auch
**verbraucht**. Keiner der beiden Stellvertreter war das — beide bleiben für
immer neuer als das Artefakt, bis dieses aus einem anderen Grund entsteht.

**Anlauf 3 — die Kommandozeile, mit der die Lib wirklich entstand.** MSBuild
schreibt sie vollständig nach
`NakamaKern.dir/Release/NakamaKern.tlog/CL.command.1.tlog` (UTF-16LE), mit jedem
`/D`. Dagegen hält das Bein die Definemenge, die die heutige Projektdatei
vorschreibt. Kein Stellvertreter mehr, und selbstheilend, weil ein Bau die
`.tlog` neu schreibt.

Gefallen (Probe: ein Define an der Kopf-Fassade ergänzt, **nicht** gebaut):

```text
[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  FEHLER  die Lib wurde mit der heutigen Definemenge gebaut (16 aus der Projektdatei)
          [nicht in der gebauten Kommandozeile: NAKAMA_RIEGELPROBE_FASSADE=1]
```

Grün nach dem Zurücknehmen, und **kein** Fehlalarm mehr bei der S9-Änderung am
Identitätsblock:

```text
[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      die Lib wurde mit der heutigen Definemenge gebaut (15 aus der Projektdatei)
```

### Grün danach

```text
-- Nakama-Kern: JUCE-Kopffassade 'NakamaKernJuce' aus 5 Modulzielen abgeleitet.
-- Nakama-Kern: K2 gruen — 9 Ziele in der Linkhuelle von 'NakamaKern', keine JucePlugin_-Konstante.
-- Nakama-Kern: K2b gruen — JUCE-Konfiguration von 'NakamaKern' deckt 'EqCopilot'.
-- Nakama-Kern: K2c gruen — 2 Empfehlungsschalter von 'EqCopilot' auch am Kern
   (juce_recommended_config_flags, juce_recommended_warning_flags);
   ausgenommen: juce_recommended_lto_flags (/GL ohne -LTCG im Verbraucher).
-- Configuring done (4.0s)
EXITCODE=0
```

`pluginval --strictness-level 8` am erneut gelinkten Bundle: **SUCCESS**,
Exitcode 0 (Behauptung 10 gilt damit für den heutigen Stand, nicht nur für den
von gestern Nacht).

**Was sich an den Zahlen älterer Abschnitte verschiebt** — die Rohausgaben in §2
bleiben stehen, wie sie gemessen wurden, und sind damit an zwei Stellen nicht
mehr der heutige Stand:

- K2s Linkhülle wuchs von **7** auf **9** Ziele (die beiden Empfehlungsziele
  tragen nur Optionen, keine Defines — K2 bleibt grün).
- Die Riegel sind **fünf**, nicht vier: K1 · K2 · K2b · **K2c** · K3/A14.

**Kanon nach der Nacharbeit: 19/19 grün** (Lauf unten). **T2-Stand: die fünf
Befunde sind geschlossen; ein zweites Prüferurteil auf diesen Stand steht aus —
kein PASS eingetragen.**

---

## Nacharbeit zum Gate G1 — 24.08.2026

**Stand dieses Abschnitts:** `906e932` — Positionen ohne eigene Angabe sind an diesen Commit gebunden.

**Marke:** `T3 NEEDS_WORK 2026-08-24 nachgearbeitet`. Der G1-Befund §4.4 ist
geschlossen (`fac67f4`); das Urteil bleibt NEEDS_WORK, weil wer repariert, sich
nicht selbst freispricht.

K2b misst jetzt in **beide** Richtungen und zusätzlich auf **Wertwidersprüche**
(ein Name, zwei Werte — für den alten Test unsichtbar, weil der erwartete Wert
ja vorhanden war, nur nicht allein). Voraussetzung war, beide Seiten *gleich*
zu rechnen; zwei verschieden gerechnete Mengen lassen sich nur in eine Richtung
vergleichen, und genau in eine wurde verglichen.

🚨 **Der Befund war größer als beschrieben.** Die Kopf-Fassade trägt ihre
Defines als **Generatorausdruck**, den `get_target_property` zur
Konfigurierzeit nicht auflöst. Damit fällt eine Aussage vom 22.08., die als
gemessener Befund im Riegelkopf stand — *„heute weicht genau eine ab
(JUCE_USE_CURL; der Kern sähe den Vorgabewert 1)"* — als **falsch** auf: die
Abweichung war nie eine Eigenschaft des Baus, sondern des **Blicks**. Beim
Übersetzen expandiert derselbe Ausdruck sehr wohl, der Kern hatte die `0`
immer. Aufgelöst wird jetzt über dieselbe Herkunftsspur
`NAKAMA_KERN_ABGELEITET_VON`, die K2 aus genau diesem Grund schon benutzt — die
Datei kannte die Falle, der Riegel nutzte das Wissen nicht.

Der Riegel **nennt jetzt die verglichene Menge** in der Konfigurierausgabe
(vier Defines), damit sein Umfang nicht wieder unsichtbar wird. Grün gegen alle
**14** Verbraucher. Vorgeführt an zwei Mutationen (nur-am-Kern,
Wertwiderspruch), beide vorher unsichtbar; Rückstellung byteweise, `git status`
leer. Roh in `G1.md` §10.4.

⚠️ **Dabei aufgefallen, offen als NAK-80:** K2b/K2c leben ausschließlich im
Configure-Schritt und tauchen in keiner der 28 Kanonzeilen auf. Heute folgenlos,
aber das ist eine Annahme über das Buildsystem, keine Messung — dieselbe Klasse
wie T2-2 in NAK-58.

---

## Prüf- und Nacharbeitsrunden — Kurztabelle (lebend, nachgezogen am 30.08.2026)

Der vollständige Verlauf jeder Runde (Befunde wörtlich, Reproduktionen, Bruch und
Rücknahme, Inventare) steht unverändert in `docs/beweise/SONDE-007a-verlauf.md`.
Klasse: **Riegel** = Produkt-/CMake-Riegel K1/K2/K2b/K2c, **Prüfskript** =
`pruefe_kern_identitaetsfrei.py` / Runnerbein A14, **Text** = Manifest-, Kommentar-
oder Behauptungswortlaut ohne Verhaltensänderung.

| Runde | Datum | Prüfer (Codex `gpt-5.6-sol`) | Urteil | Klasse der Befunde | Nacharbeit (Commits) | Kanon |
|---|---|---|---|---|---|---|
| T2 | 23.08. | Frischkontext-Prüfer (§5) | NEEDS_WORK (5) | Riegel (K2c, /W4-Regression) | §6, K2c | 19/19 |
| T3 / G1 | 24.08. | Gate G1 | NEEDS_WORK (K2b einseitig) | Riegel | „Nacharbeit zum Gate G1" | 28/28 |
| 1–2 | 28.08. | xhigh, 2 Threads | NEEDS_WORK | Riegel + Prüfskript | `374eea7`, `068c9ce` (K2 per DEFER, K2b-Ausnahmen, Genex fail-closed) | 29/29 |
| 3 | 29.08. | — (Restbefunde NAK-84/85) | — | Prüfskript | `a728fba`, `d3c741c` | 32/32 `b6003c1` |
| 4 | 29.08. | Prüfer 4 `01a04dff` @ `facea2d` | NEEDS_WORK (3) | Prüfskript (Schalterklassen, TU-Mengen, Linkfrische) | `5acf7f7` | 32/32 `0ea62e4` |
| 5 | 29.08. | Prüfer 5 `01a04e39` @ `5538fb0` | NEEDS_WORK (5) | Prüfskript (Wegwechsel W1: Neubau vor Messung; K1b, Tlog-Ortsriegel, JUCE-Baum-Riegel) | `a3bce3c`, `cd346e1`, `3353301` | ROT nur A17 → NAK-94 vorgezogen `2ed7caa` |
| 6 | 29.08. | Prüfer 6 `01a04ecb` @ `25b57ec` | NEEDS_WORK (4) | Prüfskript + Text | `ccb98cd`, `370e513` | 32/32 `370e513` |
| 7 | 29.08. | Prüfer 7 `01a04f0a` @ `b80fdce` | NEEDS_WORK (3) | Prüfskript + Text (Systemdateien namentlich, K1b über alle Compiler-Eingaben, Exit 3) | `69b4d20`, `60717c5` | 32/32 `5df7497` |
| 8 | 29.08. | Prüfer 8 `01a04f48` @ `a94c33e` | NEEDS_WORK (3) | Prüfskript + Text (Voraussetzungs-Ausgang F13/F14/F15) | `3a20064`, `93e8a7c` | 32/32 `93e8a7c` |
| 9 | 30.08. | Prüfer 9 `01a04f8b` @ `401d036` | NEEDS_WORK (2) | Text | `f808ad0` | 32/32 `d4f7ed3` |
| 10 | 30.08. | Prüfer 10 `01a04fb9` @ `32d86d9` | NEEDS_WORK (3) | Text (Wegwechsel W2: Aussagen-Inventar) | `308947d`, `0856dc0` | 32/32 `00d2796` |
| 11 | 30.08. | Prüfer 11 `01a04ff0` @ `e9ea54b` | NEEDS_WORK (1) | Text | `b9f7ee1` | 32/32 `9602d6c` |
| 12 | 30.08. | Prüfer 12 `01a05029` @ `4a379bb` | NEEDS_WORK (1) | Text (Klassifizierer lebend/historisch) | `d11be90` | 32/32 `196f97e` |
| 13 | 30.08. | Prüfer 13 `01a0505f` @ `d084296` | NEEDS_WORK (3) | Text | `88255d8` | 32/32 `f423527` |
| 14 | 30.08. | Prüfer 14 `01a05093` @ `e63a53f` | NEEDS_WORK (1) | Text | `4287839` | 32/32 `1991ff8` |
| 15 | 30.08. | Prüfer 15 `01a050ca` @ `713f0ae` | NEEDS_WORK (1, P1) | Prüfskript (K1b-Ausnahme als Abgleich) | `c212280`, `a87ab1d` | 32/32 `12fcdab` |
| 16 | 30.08. | Prüfer 16 `01a05115` @ `4a2b8da` | NEEDS_WORK (4, 1 P1) | Prüfskript (Makrokranz) + Text ×3 | `3de3a13`, `b6a79d2` | 32/32 `219424f` |
| 17 | 30.08. | Prüfer 17 `01a0516a` @ `27865ca` | NEEDS_WORK (4, 2 P1) | Prüfskript (Zeilenfortsetzung, UTF-16 fail-closed) + Text ×2 | `65e5b77`, `6112130` | 32/32 `3ef3efa` |
| 18 | 30.08. | Prüfer 18 `01a051bf` @ `eb84bec` | NEEDS_WORK (4, 1 P1) | Prüfskript (CR-only) + Text ×3 | `d4900ce`, `ae32ea4` | 32/32 `0e7a60e` |
| 19 | 30.08. | Prüfer 19 `01a051f2` @ `6cd244d` | NEEDS_WORK (2, Text) | Text | `c117e40`, `f68cd9a` | 32/32 `f68cd9a` |
| 20 | 30.08. | Abschlussprüfer, gebundener Auftrag (Skill §3.4 seit 30.08.), Thread `01a0523e` @ `7a87b7d` | **PASS** | — | Konvergenzentscheid `7a87b7d` (NAK-103, Regeln) | 32/32 `f68cd9a` |

Bilanz der Runden 7–19 (`fbbe9bf..f68cd9a`, 29.08. 21:57 → 30.08. 11:47): 79 Commits;
Produktcode-Diff 17 Kommentarzeilen in `NakamaKernRiegel.h`; Prüfskript +1 650 Zeilen,
Installer-Prüfskript +1 600 Zeilen (NAK-94), Manifest +8 000 Zeilen. Kein Befund seit
Runde 8 betraf das Produkt; die Befunde der Runden 15–18 betrafen Umgehungen des
Textscanners K1b, die absichtliche Sabotage in repo-eigenen Quellen voraussetzen
(→ NAK-103, Härtung), die übrigen den Manifestwortlaut. Daraus die Regeln in
`.claude/skills/dirigent/SKILL.md` §3.4 vom 30.08.2026.

## Kanon-Lauf - SONDE-007a Runde 19 - Abschluss

**Stand dieses Abschnitts:** `f68cd9a` — Momentaufnahme dieses Laufs (Kopf-Tabelle des Laufs); Positionen ohne eigene Angabe sind an diesen Commit gebunden.

**Lauf:** 2026-08-30 11:47 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 32/32 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-007a-f68cd9a.md](roh/SONDE-007a-f68cd9a.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-30 11:47:44 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | f68cd9a SONDE-007a Runde 19: Runde-17-Abschnitt wiederhergestellt, Klassifizierer in Runde 18, Manifestabschnitt |
| Commit (voll) | f68cd9afb19c8fb451f7b622a7a4dfff6a5e83b9 |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [A1](roh/SONDE-007a-f68cd9a.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,84 s | [A2](roh/SONDE-007a-f68cd9a.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,39 s | [A3](roh/SONDE-007a-f68cd9a.md#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 3,14 s | [A4](roh/SONDE-007a-f68cd9a.md#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,08 s | [A4b](roh/SONDE-007a-f68cd9a.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,57 s | [A5](roh/SONDE-007a-f68cd9a.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,14 s | [A6](roh/SONDE-007a-f68cd9a.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [A7](roh/SONDE-007a-f68cd9a.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,21 s | [A8](roh/SONDE-007a-f68cd9a.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,34 s | [A9](roh/SONDE-007a-f68cd9a.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,35 s | [A10](roh/SONDE-007a-f68cd9a.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [A11](roh/SONDE-007a-f68cd9a.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,25 s | [A12](roh/SONDE-007a-f68cd9a.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,22 s | [A13](roh/SONDE-007a-f68cd9a.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet im gebauten EQ-Copilot-Bundle die Teilmenge der Nadeln, die dort stehen MUSS - hersteller.name, main.produktname, main.plugin_code, main.component_cid und main.controller_cid -, waehrend die Werte der Sonden-Ziele (Suna/Probeeq) dort ausdruecklich NICHT erwartet werden und hersteller.code sowie main.bundle nicht zur Pflichtmenge gehoeren; gegen den KERN laeuft dagegen jede Nadel der Identitaetsdatei, ihre Anzahl gibt der Lauf aus. Gemessen wird kein vorhandenes Artefakt: das Bein loescht vor jeder Messung Objekte, Tlogs und Lib des Kernverzeichnisses und laesst NakamaKern vollstaendig neu uebersetzen und linken (dabei laeuft ueber ZERO_CHECK auch das Configure samt K2/K2b/K2c mit); ob eine fruehere Lib veraltet war, ist damit keine Frage mehr. Ist der Neubau nicht moeglich oder schlaegt er fehl, ist das ein Voraussetzungs-Ausgang ueber voraussetzung_exit() und kein Urteil; dasselbe gilt, wenn kein schreibbares temporaeres Verzeichnis fuer den Sollindex des JUCE-Baums da ist; ohne Neubau (--nur-messen) gibt es kein gruenes Frische-Urteil. An JEDEM dieser Ausgaenge gewinnt ein bereits registrierter Befund: war vorher etwas rot, endet der Lauf mit 2 statt 3, und die VORAUSSETZUNG-Zeile bleibt zusaetzlich stehen - der unmoegliche oder fehlgeschlagene Bau eingeschlossen (Matrix F13/F14/F15, Runde 8/9). Zusaetzlich gemessen: keine der vom Compiler tatsaechlich gelesenen Dateien ausserhalb der JUCE-Module und der Toolchain-/SDK-Wurzeln - erzwungene Includes und vorkompilierte Koepfe eingeschlossen -, traegt das Token JucePlugin_ ausserhalb von NakamaKernRiegel.h, wobei die namentlich erlaubten Systemdateien roh in ASCII und UTF-16LE durchsucht werden; jede andere Eingabe geht dabei durch dieselben Vorstufen wie im Uebersetzer, und zwar in dieser Reihenfolge - fail-closed gelesen (eine BOM entscheidet die Kodierung: UTF-8, UTF-16LE oder UTF-16BE; ohne BOM gilt strikt UTF-8; eine nicht sicher dekodierbare Compiler-Eingabe ist eine NAMENTLICHE Klage, nie mehr ein still ersetztes Zeichen), danach Praeprozessor-Phase 1 normalisiert (CRLF und einzelnes CR werden LF, ohne dass sich die Zeilenzahl aendert) und erst dann Praeprozessor-Phase 2 gefaltet (Backslash plus Zeilenende verschwindet, auch als CRLF und auch mit Leerraum davor) VOR Kommentarentfernung und Tokenpruefung, so dass ein ueber ein Zeilenende geteiltes JucePlugin_Name, ein UTF-16-Kopf mit BOM und ein hinter // in einer CR-only-Datei verstecktes #define/#if/#undef ROT sind statt unsichtbar - die Zeilenendform entscheidet nicht mehr ueber das Urteil; und NakamaKernRiegel.h ist dabei keine Freistellung, sondern ein ABGLEICH - jedes ihrer JucePlugin_-Token wird namentlich gegen die Makroliste geprueft, die der Praeprozessor in DERSELBEN Datei abfragt, und muss in einem Riegelkontext stehen; diese Liste ist seit Runde 16 AUSSCHLIESSLICH der eine #if-Kranz, der in den K1-#error muendet - dieselbe Quelle, aus der K1 seine Wirkung bezieht -, und erlaubt sind nur defined(...) IN diesem Kranz sowie das blosse Praefix in SEINER #error-Meldung; jeder andere Name, jedes Token ausserhalb des Kranzes - auch ein bekannter Name in einem eigenen #if defined(...)-Block, auch die Kurzform #ifdef/#ifndef - und jedes #undef sind ROT und werden beim Namen genannt, und ohne eindeutig ableitbare Makroliste ist die Ausnahme selbst ROT: kein Kranz mit #error, mehr als einer und eine unpaarige Bedingungsstruktur sind je fuer sich ROT (Runde 15/16); die Zahl der abgeglichenen Token und der Makros gibt der Lauf aus; JEDE vom Compiler gelesene Datei - ohne Endungsausnahme - stammt aus einem erlaubten, aus dem Bau abgeleiteten Ort oder ist eine der NAMENTLICH gefuehrten Systemdateien unter %SystemRoot% - ihre Namen stammen aus einer im Manifest eingefuegten CL.read.1.tlog-Rohausgabe (Probe P8-SYS, Runde 8), ihre Anzahl gibt der Messlauf aus, wobei juce_audio_plugin_client, generierte JuceLibraryCode-Header, jede sonstige Datei unter %SystemRoot% und alles Unbekannte ROT sind; und der JUCE-Baum ist der gepinnte Tag plus genau der benannte Nakama-VST3-Patch, gemessen in drei Zeilen: HEAD IST der Commit des Tags 8.0.9 (verglichen, nicht nur beschrieben), ausserhalb der Patchdateien ist nichts geaendert, unverfolgt oder IGNORIERT, und die Patchdateien tragen genau den Patch - Inhalt gegen den in einem temporaeren Index gerechneten Sollzustand "Tag + Patch", nicht bloss rueckwaerts passende Hunks. Die eigenen Wachen des Beins (Configure-Stamps, vier Schalterklassen beidseitig, TU-Mengen, Linkfrische, lastbuildstate) belegen nur noch, WOMIT gebaut wurde; die AdditionalOptions-Klasse prueft dabei ausdruecklich nur Enthaltensein. Ausdruecklich nicht behauptet: der Inhalt der Toolchain- und SDK-Header ausserhalb des Repos (nur ihre Herkunft aus den abgeleiteten Wurzeln wird geprueft, kein Fingerprint), ein Compilerwechsel innerhalb derselben lastbuildstate-Kennung, der Inhalt im JUCE-Baum ausserhalb modules/** GELOESCHTER Dateien (sie werden benannt - eine geloeschte Datei kann keine Compiler-Eingabe werden), und die uebrigen ClCompile-Elemente der Projektdatei (Warnstufe, Optimierung, Laufzeitbibliothek und die anderen) - sie sind durch den Neubau gegenstandslos, aber nicht einzeln nachgebildet. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 6,69 s | [A14](roh/SONDE-007a-f68cd9a.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. Zum Auslieferungsstand sagt dieser Kanon-Lauf NICHT, dass der Bau dem festgeschriebenen Paket gleicht (nach einem Relink tut er das erwartungsgemaess nicht) - hart gemessen wird hier nur, dass JEDES festgeschriebene Artefakt UEBERHAUPT vorliegt und sein Ordner-Hash bildbar ist, und zwar unabhaengig davon, ob sein sha256 gesetzt ist; eine Hash-Abweichung erscheint als Hinweis mit beiden Kurz-Hashes, ein fehlender Hash als Hinweis ohne Vergleich. [3b] laesst die Kanten einmal fallen: ein fehlendes Artefakt bleibt auch neben einem sha256=null ROT, [4b] verwandelt eine unbrauchbare Kennung in einen Hinweis statt in einen Abbruch, und jeder Journalstatus aus Install-Nakama.ps1 wird einzeln gefahren. [4b] berichtet ohne Urteil und ohne Abbruch, ob der installierte Stand aus install-ergebnis.json dem Manifest entspricht; ein `ok` gibt es dabei NUR bei Journalstatus OK und einem abgeschlossenen, nicht zurueckgerollten Eintrag - VORBEREITET, KOMPENSATION, ERROR_TEILSTAND, ERROR_RUECKGEROLLT, RUECKWEG_AKTIV, RUECKWEG, ein unbekannter und ein fehlender Status melden "installierter Stand unbekannt" OHNE Hashvergleich, weil der gespeicherte Hash dann nur den Stand vor dem Gegenakt beweist (NAK-94 Nacharbeit 2). Die Reihenfolge ist dabei STATUS VOR LISTE: die Statussperre steht vor jeder Verwendung von `eintraege`, weil ein regulaer abgeschlossener Rueckweg gar keine Eintragsliste schreibt; verlangt wird sie nur noch im Status-OK-Pfad. [3b] faehrt dafuer zwei Sorten Probe-Journal und nennt die Sorte in jeder Zeile: WRITER-FIXTUREN - von Install-Nakama.ps1 selbst in der A18-Sandbox erzeugt und byteweise eingefroren (eq-copilot/fixtures/installer/journale/ mit MANIFEST.json aus Fall, Stand, Installer-Befehl und SHA-256; Erzeuger tools/eq-copilot/erzeuge_installer_journale.py) - und daraus abgeleitete deklarierte MUTANTEN mit benannter Abweichung (mutant_von, abweichung), die im Skript aus genau einer Fixtur entstehen. Als Writer-Fixturen laufen OK nach Erstinstallation, OK nach Tausch, RUECKWEG nach dem Gegenpfad (sieben Felder, ohne eintraege) und ERROR_RUECKGEROLLT nach voll kompensiertem Abbruch; als Mutanten die Durchgangsstaende, die ein abgeschlossener Lauf nicht hinterlaesst (VORBEREITET, KOMPENSATION, ERROR_TEILSTAND, RUECKWEG_AKTIV), ein erfundener und ein fehlender Status sowie ein OK-Journal ohne eintraege - das ist ausdruecklich keine Writer-Form, denn der OK-Pfad entfernt die Liste nie. Vor der Benutzung rechnet [3b] jede Fixtur gegen ihren SHA-256 nach und verlangt JEDEN in MANIFEST.json gefuehrten Fall - es gibt keine handgepflegte Teilmenge und keinen optionalen Ueberspringer mehr; dazu muss die Statusachse OK / RUECKWEG / ERROR_RUECKGEROLLT im Korpus vertreten sein, und sie steht AUSSERHALB des Korpus, damit ein Fall nicht zusammen mit seiner MANIFEST-Zeile still verschwinden kann. Eine von Hand angefasste, fehlende oder verwaiste Fixtur ist ROT, nicht still eine andere Probe. Dabei bricht Z1 an einem GEAENDERTEN BYTE, waehrend eine FEHLENDE Pflichtdatei den Block abbricht: liegt die Datei vor, ist lesbar und traegt die von Z2..Z7 gelesene Writer-Struktur, weicht aber ihr SHA-256 vom MANIFEST ab, faerbt das genau die eine Zusagenzeile Z1, und Z2..Z7 laufen auf dem vollstaendigen Korpus gruen weiter; fehlt sie, ist sie unlesbar oder kein Journalobjekt, liegt eine verwaiste daneben oder ist eine Statusklasse verschwunden, haelt [3b] fail-closed an, weil die uebrigen Zusagen dann nichts Vollstaendiges mehr messen koennten (NAK-94 Nacharbeit 7). Nach rotem Z1 laufen Z2..Z7 nur auf strukturell gueltigem Objekt weiter; sonst Abbruch - eine einzelne Byteaenderung kann eintraege zu xntraege machen, ohne dass der Kopf aufhoert, ein JSON-Objekt zu sein, und Z3 stuerbe dann mit einem KeyError. [3b] prueft deshalb vor jeder Benutzung die von Z2..Z7 gelesene Writer-Struktur - schema, status und transaktions_id fuer jeden Fall, bei MANIFEST-Status OK zusaetzlich eine nicht leere Eintragsliste aus Objekten mit Kennung, sha256, mutation_abgeschlossen und rollback_abgeschlossen -, prueft dabei SCHLUESSEL und Grobform statt Werte, und endet sonst mit Klartext und Exit ungleich 0, nie mit einem Traceback (NAK-94 Nacharbeit 8). JEDE von diesem Bein gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; Verstoss = kontrollierter Abbruch mit Klartext und Exit ungleich 0, nie ein Traceback - das Installer-Manifest (Wurzel Objekt, `artefakte` nicht leere Liste von Objekten mit Zeichenkette `quelle`, `ziele` Objekt, `rueckweg.bekannte_staende` Liste), die Identitaetsdatei (Wurzel Objekt, `ziele` nicht leere Liste von Objekten mit `id`, je AKTIVEM Ziel `produktname` und `bundle`, `hersteller.code` Zeichenkette) und das Korpusmanifest journale/MANIFEST.json (Wurzel Objekt, `stand` Zeichenkette, `faelle` nicht leere Liste, je Fall `datei`, `status`, `fall` und `befehl` als nicht leere Zeichenketten und `sha256` als HEX64 in Grossbuchstaben) eingeschlossen; am Installer-Manifest gehoeren dazu seit NAK-94 Nacharbeit 10 auch `ziel_id` UND `cmake_ziel` als Zeichenketten an jedem VST3-Artefakt, weil [3] beide hart liest - den zweiten hat der Byte-Kipp-Fuzz gefunden, nicht ein Pruefer. Was den Strukturpruefungen dennoch entgeht, faengt seit derselben Runde ein ZENTRALER FAENGER in main() ab: jede Ausnahme, die kein Strukturhalt ist, endet als eine Klartextzeile mit Ausnahmetyp, Meldung, Datei und Zeile des Ausloesers, Exit 2 und OHNE Traceback auf stdout oder stderr - den gibt es nur mit --debug. Seit NAK-94 Nacharbeit 11 ist dieser Faenger EINE Funktion (_geschuetzt), die main() um den ganzen Lauf und [3c] um JEDEN einzelnen Fuzz-Fall legt; der Fuzz hat keine zweite Ausnahmebehandlung mehr, also macht jeder Eingriff an dieser Funktion den Fuzz rot statt ihn unberuehrt zu lassen. Beides ist nicht behauptet, sondern GEMESSEN: der Byte-Kipp-Fuzz [3c] kippt jedes Byte jeder gelesenen JSON-Datei einzeln auf 0xFF (ungueltiges UTF-8) und auf 0x20 und faehrt die Datei in-process durch Lesen, Strukturpruefung und jeden verbrauchenden Block - seit Nacharbeit 11 auch den mutierenden Zweig --hashen, mit Schreibziel unter %TEMP% statt im Repo, sodass [3c/0] ohne Handausnahme auskommt, und seit Nacharbeit 12 zusaetzlich den ZWEITEN Leser desselben Korpusmanifests - pruefen() aus erzeuge_installer_journale.py, das dieselbe Strukturpruefung IMPORTIERT statt sie zu kopieren und dessen main() im selben zentralen Faenger liegt; beide Verbraucher laufen je Fall unabhaengig voneinander, weil ein an den ersten angehaengter Aufruf fuer genau die Mutationen unerreichbar waere, um die es beim zweiten geht; seit Nacharbeit 13 haengt diese Zusage nicht mehr an der Deckungsrechnung, die sie gar nicht sehen kann - eine Differenz ueber _lauf() vermisst keinen Verbraucher, der in _lauf() nicht vorkommt -, sondern an einer eigenen, fallenden PFLICHTMENGE DER FUZZ-VERBRAUCHER: A17-Kanonpfad ueber _fuzz_verbraucher/_lies_geprueft, --hashen ueber _fuzz_verbraucher/hashen und Erzeuger-pruefen() ueber _fuzz_erzeuger/korpus_nachrechnen, jede Zusage namentlich. Fehlt ein Verbraucher in FUZZ_VERBRAUCHER oder ruft er seinen Pflichtaufruf nicht mehr, ist [3c/0b] ROT MIT NAMEN; wurde er im Lauf kein einziges Mal wirklich gefahren, ist es [3c/2], das dafuer den Laufzaehler statt des Quelltexts liest. [3c/1] haelt zusaetzlich den sha256 des Manifests im Repo vor und nach dem Fuzz dagegen; zugesagt und gezaehlt ist: KEINE Ausnahme ausser Strukturhalt und dem eigenen, ebenfalls kontrollierten Ausgang Gegenprobe unmoeglich - der Lauf gibt Dateien, gekippte Stellen, Laeufe und die Zaehler je Klasse aus, im Kanon als deterministisches Sample (jedes n-te Byte, n steht in der Ausgabe), vollstaendig mit --fuzz-voll, dessen Lauf als eigene Roh-Datei unter docs/beweise/roh/ liegt statt nur behauptet zu werden (NAK-94 Nacharbeit 11). Geprueft werden dabei Schluessel und Grobform, nie Werte: ein geaenderter sha256-WERT im Korpusmanifest laesst die Struktur gueltig und faerbt weiterhin genau Z1. Die einzige Ausnahme vom Abbruch ist der urteilsfreie Berichtsblock [4b], wo derselbe Verstoss ein kontrollierter Hinweis mit Rueckkehr ist, weil dieser Block per Zusage nie urteilt und nie abbricht (NAK-94 Nacharbeit 9). Die Transaktions-IDs der Fixturen bestehen die Ist-TransaktionsId-Regex des Skripts (das Muster wird live aus Install-Nakama.ps1 gelesen). Volatile Werte - zeit, transaktions_id, Pfade, Hashes - bleiben, wie der Writer sie schrieb; verglichen werden Struktur und Status, jede erwartete Zeile fuer JEDES Artefakt. Von Hand geschriebene Writer-Formen gibt es nicht mehr. [3b] ist nach ZUSAGEN gegliedert (Z1..Z7), und je Zusage gibt es genau EINEN diskriminierenden Bruch, der nur ihre Zeilen rot macht (B6-Z1..B6-Z7 aus NAK-94 Nacharbeit 6, fuer Z1 abgeloest durch B7-Z1 aus Nacharbeit 7 - dazu die eigene Probe Pflichtmenge fuer den fail-closed Abbruch; die frueheren Einzelbrueche stehen in Nacharbeit 3/4/5). Eine ueber mehrere Werte parametrisierte Zusage - das OK-Urteil ueber jede OK-Fixtur, die Statussperre ueber jeden Nicht-OK-Status - ist EINE Zusage mit EINEM Bruch; ihre Werte zaehlt die Laufausgabe. Hart verglichen wird der Hash nur mit --release (Auslieferungsschritt, hier nicht aufgerufen) und in Install-Nakama.ps1 Riegel 2 (NAK-94). | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 52,94 s | [A17](roh/SONDE-007a-f68cd9a.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 36,76 s | [A18](roh/SONDE-007a-f68cd9a.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [A19](roh/SONDE-007a-f68cd9a.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,32 s | [A20](roh/SONDE-007a-f68cd9a.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, >Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len loesen keine Allokation aus; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,32 s | [A21](roh/SONDE-007a-f68cd9a.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,36 s | [A22](roh/SONDE-007a-f68cd9a.md#a22) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Nachtrag Runde 2 (Prueferbefund P1): dass Riegel 1 KLASSENUNABHAENGIG sperrt, misst dasselbe Bein zusaetzlich direkt an der oeffentlichen positionErlaubt fuer alle vier Klassen - ohne diese vier Zeilen bliebe die urspruengliche passive_probe-Regression hier unbemerkt, weil kein Ziel mehr NAKAMA_SONDE_PASSIV baut (S9b/SONDE-007c). Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,11 s | [A16](roh/SONDE-007a-f68cd9a.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,11 s | [B1](roh/SONDE-007a-f68cd9a.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,35 s | [B2](roh/SONDE-007a-f68cd9a.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [B3](roh/SONDE-007a-f68cd9a.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/SONDE-007a-f68cd9a.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,23 s | [B3c](roh/SONDE-007a-f68cd9a.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,32 s | [B4](roh/SONDE-007a-f68cd9a.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [B9](roh/SONDE-007a-f68cd9a.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,12 s | [B5](roh/SONDE-007a-f68cd9a.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | v3-Envelope in C++ klassifiziert den Envelope-Korpus wie das Manifest (Urteil UND Verstossmenge, alle 14 Regeln mit Negativfixture); CRC32C trifft die RFC-3720-Vektoren, P0/P1 tragen CRC exakt 0, P2 die Pflichtsumme ueber genau die Payloadbytes; 40 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und 7671 angenommene EINBIT-Mutanten gueltiger Frames halten jede Kopfregel (reiner Zufall wird praktisch immer abgewiesen - die Invariante braucht deshalb die Mutanten, sonst spraeche sie ueber eine leere Menge), 3000 gekippte P2-Bits fallen einzeln, byteweise Zustellung liefert dieselben 40 Frames und ein kaputter Frame beendet den Strom; Pipetoken trifft das Golden aus §48.3 samt SHA-256- und RFC-4648-Vektoren; P0 verwirft nichts und meldet den 65. Eintrag, P1 koalesziert an der Position und haelt Ereignisse fuer den Reconnect vor, die P2-Schleuse ersetzt den aeltesten ungesendeten Frame, uebergibt 100 000 Frames mit 0 Allokationen (mit Gegenprobe am selben Zaehler) und liefert unter Flut keinen zerrissenen Frame; verdrahtet: Control koppelt Telemetry ueber link_id + challenge, ein ungekoppelter Telemetry-Connect wird geschlossen, der Client verbindet nach Serverneustart von selbst wieder, ein kaputter Envelope vom Server schliesst die Verbindung, und ein P0-Ueberlauf WAEHREND einer stehenden Verbindung schliesst sie ebenfalls statt still zu kuerzen. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 31,96 s | [B10](roh/SONDE-007a-f68cd9a.md#b10) |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,12 s | [B8](roh/SONDE-007a-f68cd9a.md#b8) |


## Dirigentenstand — 2026-08-30 12:48 (Sitzung 9f06075a): Prüfer 20b **PASS** — S8 abgenommen

**Stand dieses Abschnitts:** `7a87b7d`

**Marke:** `T3 PASS 2026-08-30` (oben im Kopf). Ticketbasis `dafa5a5`; Zielstand `7a87b7d` (= origin/master); Kanon GRÜN 32/32 auf `f68cd9a` (Roh-Datei `docs/beweise/roh/SONDE-007a-f68cd9a.md`); zwischen `f68cd9a` und `7a87b7d` änderte sich im Prüfbereich nur eine Kommentarzeile in `tools/beweise.ps1` (Prüfer-20-Befund P2, Sekundenangabe entfernt).

**Konvergenzentscheid (Skill §3.4, seit 30.08.2026):** Nach Prüfer 20 (NEEDS_WORK, P1 Raw-String-Literale im Kommentar-Entferner von K1b, P2 volatile Sekundenangabe) hat der Dirigent die Runde nicht als Runde 20 weitergeführt, sondern die offenen Befunde am wörtlichen Gate-Text §53.4 eingeordnet: P1 ist ein weiterer Umgehungsweg des Textscanners K1b, der absichtliche Sabotage in repo-eigenen Quellen voraussetzt — **Härtung**, datiert als NAK-103 (K1b eingefroren); P2 ist ein Textfix und wurde im Abschluss-Commit `7a87b7d` behoben. Danach eine **gebundene Abschlussprüfung** über den S8-Änderungssatz allein.

| Schritt | Worker / Prüfer | Stand | Ergebnis |
|---|---|---|---|
| Konvergenzentscheid | Fable (Dirigent), Commit `7a87b7d` | `fd5aff1`…`7a87b7d` | Skill §3.4/§3.5/§6, Prüfliste E, Codex-Skill `sondenplan-audit` §3, Register NAK-103, Runnerkopf ohne Sekundenangabe |
| Prüfer 20b (Abschluss) | Codex xhigh `01a0523e-12d7-7a61-afbb-10c5fc682e63`, lesend über `git diff dafa5a5...7a87b7d -- eq-copilot/plugin/state/NakamaKernRiegel.h eq-copilot/cmake/NakamaKern.cmake eq-copilot/cmake/pruefe_nakama_kern_genex.cmake eq-copilot/plugin/CMakeLists.txt tools/eq-copilot/pruefe_kern_identitaetsfrei.py tools/beweise.ps1`, Gate-Text §53.4 wörtlich, `docs/**` und NAK-103 als erklärter Ausschluss, HEAD vor/nach identisch, 334 s | `7a87b7d` | **PASS** — unten wörtlich |

**Urteil des Abschlussprüfers, wörtlich (`@ 7a87b7d`):**

> URTEIL: PASS — Geprüft: Scoped Diff, CMake-Genex-Selbsttest 26/26, alle generierten Konfigurationen, Release-Tlog sowie A14-Scan mit 17 kernfreien Nadeln, erfolgreicher Bundle-Gegenprobe und erwarteten Archivmitgliedern; HEAD blieb unverändert.
>
> Nicht geprüft: Kein Neubau und kein vollständiger 32/32-Kanonlauf wegen Schreibverbot; der Python-Selbsttest brach wegen fehlendem beschreibbarem Temp-Verzeichnis nach seinen ersten erfolgreichen Prüfungen ab.

**Was der Prüfer nicht fahren konnte, liegt als Rohbeleg vor:** Neubau und Kanon 32/32 auf `f68cd9a` (`docs/beweise/roh/SONDE-007a-f68cd9a.md`, Bein A14 darin mit Neubau, Gegenprobe und Kern-Scan).

**Urteil des Dirigenten:** S8 `SONDE-007a` ist **abgenommen** — das Gate §53.4 („der gemeinsame Kern sieht keine `JucePlugin_*`-Konstanten") ist am Artefakt gemessen (A14: Neubau, Binärscan gegen jede Nadel, Gegenprobe am Bundle, Archivmitglieder) und am Bau bewacht (K1 `#error`, K2/K2b/K2c). Was darüber hinaus in den Runden 4–19 gebaut wurde (Frischewachen, K1b-Präprozessor-Nachbau, JUCE-Baum-Riegel, ihre Selbsttests), misst nicht das Gate und wird unter **NAK-100** (umgewidmet: Rückbau auf das Gate-Maß statt Aufteilung in Pakete) zurückgebaut — mit eigenem, gebundenem Prüfer.

**Offen außerhalb der Grenze:** NAK-89, NAK-93, NAK-98, NAK-99, NAK-100 (Rückbau), NAK-103 (Härtung, geparkt).

## NAK-100 — Rückbau von A14 auf das Gate-Maß (30.08.2026)

**Stand dieses Abschnitts:** `e2551a9`

**Warum:** Prüfer 4–20 bauten `pruefe_kern_identitaetsfrei.py` von rund 900 auf 4 197 Zeilen aus — Frischewachen (seit Runde 5 gegenstandslos), K1b-Präprozessor-Nachbau, Tlog-Ortsriegel, JUCE-Baum-Riegel und ~1 300 Zeilen Selbsttests dieser Wachen. Am Gate §53.4 gemessen trugen ~800 Zeilen die Abnahme. User-Wort 30.08.: „macht der prüfskript überhaupt sinn? codex ist bekannt maximal zu übertreiben was tests angeht" — Register NAK-100 (umgewidmet), NAK-103 (K1b als Härtung).

**Was A14 seit `e2551a9` misst (746 Zeilen):** [0] Neubau der `NakamaKern.lib` vor der Messung mit drei Belegen, [1] Gegenprobe der Pflichtnadeln am Gen-Bundle, [2] Binärscan gegen jede Nadel der Identitätsdatei in allen Byteformen, [3] Archivmitglieder = erwartete Kernobjekte, kein JUCE-Objekt. Exit-Semantik unverändert (0/2/3, registrierter Befund gewinnt). Entfernt: K1b, Ortsriegel, JUCE-Baum-Riegel (S3-Quellhash-Gate B3 bewacht den Baum), Frischewachen. Runner-Behauptung auf das Gate-Maß gekürzt; `NakamaKernRiegel.h` nur im Kommentar geändert (Makroliste und `#error` unverändert, `git diff` zeigt keine Direktive); `NakamaKern.cmake` K3-Kommentarzeile.

### Selbsttest — jeder verbleibende Riegel einmal beim Fallen gesehen (`--selbsttest` @ `e2551a9`)

```text
A14-Selbsttest: jeder Riegel einmal beim Fallen gesehen (baulos)

[S1] Nadelformen
  ok      little-endian Immediate 0x45716370 wird gefunden
  ok      big-endian Integerfolge 0x45716370 wird gefunden
  ok      16-Byte-CID roh wird gefunden
  ok      16-Byte-CID COM-vertauscht wird gefunden
  ok      UTF-16LE-Text wird gefunden
  ok      17 Nadeln aus der Identitaetsdatei, main.plugin_code dabei

[S2] Gegenprobe - faellt, sobald eine Pflichtnadel fehlt
  ok      Bundle mit allen Pflichtnadeln: Gegenprobe gruen
  ok      Bundle ohne CIDs: genau die zwei CID-Zeilen fallen  [Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle; Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle]

[S3] Kernscan - faellt bei jeder Nadelform
  ok      sauberer Kern: alle Nadeln NICHT gefunden
  ok      Kern mit eingepflanzter Nadel (ascii): Scan faellt und nennt die Form  [ascii]
  ok      Kern mit eingepflanzter Nadel (fourcc-int-le): Scan faellt und nennt die Form  [fourcc-int-le]
  ok      Kern mit eingepflanzter Nadel (roh16-com): Scan faellt und nennt die Form  [ascii,fourcc-int-be; roh16-com; ascii,fourcc-int-be]

[S4] Bauform - Archivleser und Objektliste
  ok      synthetisches Archiv mit Kurz- und Langnamen wird vollstaendig gelesen
  ok      genau die Kernobjekte: Bauform gruen
  ok      juce_core.obj im Archiv: beide Bauform-Zeilen fallen und nennen es
  ok      kein Archiv: Bauform faellt
  ok      Nicht-Archiv wird als None erkannt

[S5] Neubau-Beleg - faellt bei altem Objekt, fehlender Lib, fehlender TU
  ok      konsistenter Neubau: keine Klage
  ok      Objekte und Lib aelter als der Zeitanker: beide Klagen  [Objekt aelter als der Neubau (nicht neu uebersetzt): A.obj, B.obj | NakamaKern.lib ist aelter als der Neubau - nicht neu gelinkt]
  ok      Bauausgabe ohne B.cpp: Klage nennt B.cpp  [Bauausgabe nennt diese Uebersetzungseinheiten nicht: B.cpp]
  ok      unbekannte Datei im Kernverzeichnis wird benannt  [Datei im Kernverzeichnis, die weder Bauausgabe noch bekannte Eingabe ist: x.bin]
  ok      fehlende Lib nach Neubau: Klage  [NakamaKern.lib fehlt nach dem Neubau]
  ok      Objektzahl ungleich Quellzahl: Klage  [1 Objekte, aber 2 Kernquellen | NakamaKern.lib fehlt nach dem Neubau]

[S6] Kernquellen aus CMakeLists, Release-Wahl, Voraussetzungs-Ausgang
  ok      NAKAMA_KERN_QUELLEN (9) und ERWARTETE_OBJEKTE decken sich  [ControlClient, IpcVerbindung, NakamaKanon, NakamaLebenslauf, NakamaParameter, NakamaState, NakamaVertrag, TelemetryClient, WireEnvelope]
  ok      set(...)-Block wird ohne Kommentare und Anfuehrungszeichen gelesen
  ok      fehlendes NAKAMA_KERN_QUELLEN ist ein Fehler, kein leeres Ergebnis
  ok      Release gewinnt gegen Debug, unabhaengig von der Sortierung
  ok      Voraussetzung fehlt: Exit 3 ohne, Exit 2 mit registriertem Befund

28 ok, 0 Fehler
selbsttest exit 0
```

### Messlauf mit Neubau (`py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py` @ `e2551a9`)

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (1218518 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7105024 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - der Kern wurde fuer diese Messung neu gebaut
  ok      Kernartefakte geloescht und in 30.1s neu erzeugt (22 Dateien entfernt, 9 Uebersetzungseinheiten, 9 Objekte, Lib neu gelinkt)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii,fourcc-int-be]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.bundle = 'Nakama Probeeq.vst3' NICHT
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt active-probe.produktname = 'Nakama Probeeq' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.bundle = 'Nakama Suna.vst3' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT
  ok      NakamaKern.lib traegt passive-probe.produktname = 'Nakama Suna' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die 9 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

25 ok, 0 Fehler
messlauf exit 0
```

### Bruchproben am echten Artefakt — und Rücknahme

Drei Brüche gegen die frisch gebaute Lib und das gebaute Bundle über die Modulfunktionen; danach dieselben Funktionen auf den unveränderten Artefakten:

```text
Lib: eq-copilot/build/plugin/Release/NakamaKern.lib 1218518 Byte; Archivmitglieder: ['ControlClient.obj', 'IpcVerbindung.obj', 'NakamaKanon.obj', 'NakamaLebenslauf.obj', 'NakamaParameter.obj', 'NakamaState.obj', 'NakamaVertrag.obj', 'TelemetryClient.obj', 'WireEnvelope.obj']
Bruch B1 - 'Nakama Probeeq' als UTF-16LE an die echte Lib angehaengt: 1 rot -> ["NakamaKern.lib traegt active-probe.produktname = 'Nakama Probeeq' NICHT [utf-16le]"]
Bruch B2 - Gegenprobe gegen die Kern-Lib statt des Bundles: 5 von 5 rot -> ["Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle", "Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle", "Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle", "Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle", "Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle"]
Bruch B3 - juce_core.obj zu den echten Mitgliedern gelegt: 2 rot -> ['Archivmitglieder sind genau die 9 Kernobjekte [ControlClient.obj, IpcVerbindung.obj, NakamaKanon.obj, NakamaLebenslauf.obj, NakamaParameter.obj, NakamaState.obj, NakamaVertrag.obj, TelemetryClient.obj, WireEnvelope.obj, juce_core.obj]', 'kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt) [juce_core.obj]']
Ruecknahme - echte Lib, echtes Bundle: 0 rot im Kernscan, 0 rot in der Bauform
```

**Kanon:** folgt als Abschlusslauf nach dem Rückbau von A17 (NAK-100, zweiter Teil) — ein Lauf für beide.

