# Beweismanifest — SONDE-007a «Gemeinsamer Kern ohne `JucePlugin_*`-Konstanten»
<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-23 nachgearbeitet -->
<!-- NAKAMA-URTEIL: T3 NEEDS_WORK 2026-08-24 offen -->
<!-- NAKAMA-URTEIL: T3 NEEDS_WORK 2026-08-24 nachgearbeitet -->

> **T3 / Gate G1, 24.08.2026 — NEEDS_WORK.** Der Riegel **K2b**
> (`cmake/NakamaKern.cmake:218 ff.`, tragende Schleife `:234`) laeuft nur ueber
> die Referenz und fragt „fehlt das im Kern?" — nie umgekehrt. Ein kern-eigenes
> Zusatz-Define ist damit unsichtbar, obwohl der Kopf desselben Riegels (`:202`)
> zusagt: „dieser Riegel haelt die beiden Mengen zusammen" — eine Aussage ueber
> MENGENGLEICHHEIT, die einseitige Enthaltung nicht leistet. Ein Riegel, der
> weniger kann als er behauptet, beweist mit seinem Schweigen nichts.
> Vollstaendig: `docs/beweise/G1.md` §4.4, §7.1.


> **Die eine harte Regel** (`docs/bauaufteilung-sonden.md` §2): *Eine Behauptung
> ohne eingefügte Rohausgabe ist ein **gescheitertes** Ticket, kein fast
> fertiges.* Unten stehen stdout/stderr wortgleich.

| Feld | Wert |
|---|---|
| Ticket | `SONDE-007a` (Teil 1 von `SONDE-007`; Teil 2 = `SONDE-007b` / S9) |
| Phase / Session | P1 / S8 |
| Gate-Text (Quelle) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §53.4 „Static-Lib-Randbedingung" — **wörtlich:** „Der Wechsel auf gemeinsame statische Bibliotheken ist für drei Ziele gerechtfertigt, aber nur unter einer harten Regel: Der gemeinsame Kern sieht **keine** `JucePlugin_*`-Konstanten; Identität kommt ausschließlich aus `plugin-identities-v1.json` über die dünnen Target-Schichten. Andernfalls erhalten zwei der drei Bundles die Identitätskonstanten des dritten — genau die Werte, die §44.1 einfriert. `SONDE-007` enthält dafür einen expliziten Prüfschritt." · `docs/bauaufteilung-sonden.md` S8-Zeile + Fließtext: „Wenn der geteilte Kern auch nur **eine** `JucePlugin_*`-Konstante sieht, bekommen zwei Bundles die Identität des dritten — und genau das Identitäts-Golden aus S2 fällt." |
| Datum | 2026-08-22 |
| Prüfstufen | T1 ☑ (§3) · T2 ☑ gefahren 23.08. — **NEEDS_WORK**, fünf Befunde, Urteil und Belege in [§5](#5-t2--frischkontext-prüfer), Nacharbeit in [§6](#6-nacharbeit-zu-den-t2-befunden) · T3 ☐ (erst am Gate G1) |

**Was gebaut wurde (Karte, keine Behauptung — Behauptungen stehen in §1):**

- `eq-copilot/cmake/NakamaKern.cmake` — Linkhüllen-Läufer (einmal geschrieben,
  zweimal benutzt), **Kopf-Fassade** `nakama_kern_juce_fassade()`, der
  Konfigurier-Riegel **K2** `nakama_kern_riegel_pruefen()` und der
  Konfigurationsgleichheits-Riegel **K2b** `nakama_kern_konfig_pruefen()`.
- `eq-copilot/plugin/state/NakamaKernRiegel.h` — Kompilier-Riegel **K1**, 46
  `JucePlugin_*`-Makros namentlich, scharf nur unter `NAKAMA_KERN_UEBERSETZUNG`.
- `eq-copilot/plugin/CMakeLists.txt` — `NakamaKern` als `add_library(… STATIC)`
  mit den vier geteilten Quellen (`state/NakamaKanon.cpp`,
  `state/NakamaParameter.cpp`, `state/NakamaState.cpp`,
  `vertrag/NakamaVertrag.cpp`); `nakama_state_anbinden()` → `nakama_kern_anbinden()`
  bei acht Zielen. Der Kopfkommentar, der bis heute die Static-Lib begründet
  ablehnte, trägt jetzt den abgelösten Satz samt Grund der Ablösung.
- `tools/eq-copilot/pruefe_kern_identitaetsfrei.py` — Artefakt-Riegel **K3**
  mit eigener Gegenprobe und COFF-Archivleser.
- `tools/beweise.ps1` — Kanon-Bein **A14**; `NakamaKern` als *gemessenes Ziel*
  (wird gebaut, läuft aber nicht selbst).

**Arbeitsteilung der fünf Riegel** — der Punkt des Tickets, nicht Redundanz.
Diese Übersicht beschreibt den heutigen Arbeitsbaum; ältere Rohausgaben weiter
unten bleiben datierte Belege ihres damaligen Quellstands:

| | misst | sieht | sieht **nicht** |
|---|---|---|---|
| **K1** Präprozessor | Quelltext | 46 bekannte Makros namentlich, am Anfang **und Ende** jeder der fünf Kern-Übersetzungseinheiten; damit auch bis zum TU-Ende definierte Makros aus später eingebundenen eigenen/generierten Headern | Makronamen außerhalb der Liste und vor dem TU-Ende wieder entfernte Makros (der Präprozessor kann kein Präfix aufzählen; resultierende Identitätsbytes misst K3) |
| **K2** CMake-Konfigurierzeit | Kernziel plus dessen compilerwirksame Usage-Requirements-Hülle; Verbraucher nur bei einer echten fehlerhaften Rückkante; Ausführung verzögert bis zum Ende von `plugin/` nach allen Zieländerungen | jedes compilerwirksame `JucePlugin_` aus eigenen und transitiven `*_COMPILE_DEFINITIONS` sowie `-D`/`/D` in `*_COMPILE_OPTIONS`; direkte Zielnamen, `debug`/`optimized`/`general`-Kanten und die unten inventarisierten bedingten bzw. zielbezogenen Generatorausdrücke | Makros, die erst im C++-Quelltext entstehen (dafür K1/K3); String-Transformationen in Linkkanten und `MAP_IMPORTED_CONFIG_*` werden nicht ausgewertet, sondern ausdrücklich **ROT** gemeldet |
| **K3/A14** Artefakt + Frische | gebaute `.lib`, `.vcxproj`, `.tlog` und echte Kern-Includehülle | jeden eingefrorenen Text als ASCII/UTF-16LE, Viercodes zusätzlich als 4-Byte-Integer in **beiden** Byteordnungen, CIDs roh/COM-vertauscht; rekursive lokale Includes aus den tatsächlichen `NAKAMA_KERN_QUELLEN`; heutige Definemenge exakt in beide Richtungen gegen **jede** gebaute Kern-TU | Baubeschreibung ohne resultierende Artefaktbytes (dafür K1/K2/K2b/K2c) |
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

## 4. Kanon-Lauf (roh, vom Runner erzeugt)

**Lauf:** 2026-08-23 00:09 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 19/19 Kanon-Laeufe bestanden | 4 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-23 00:09:12 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | f96c95a Hub: S8 gebaut heisst nicht abgenommen - T2 steht aus, wie bei S5 und S6 |
| Commit (voll) | f96c95a4cfc9b7106e1e38342803a342dda257e0 |
| Arbeitsbaum | 12 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M briefing-hub/app/briefing-app.tsx
 M briefing-hub/app/globals.css
 M briefing-hub/data/friendly-copy.ts
 M briefing-hub/data/hub.json
 M docs/NEXT-SESSION.md
 M docs/beweise/SONDE-007a.md
 D eq-copilot/design/ASSET-KIT.md
 D eq-copilot/design/eq-copilot-material-preview.png
 M nimbalyst-local/automations/planstand-nakama.md
 M tools/eq-copilot/pruefe_kern_identitaetsfrei.py
?? "Untitled Workspace/"
?? nimbalyst-local/automations/planstand-nakama/
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-22 23:53:16 | `CFA457D6923EF2E8` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-22 23:53:18 | `EDC21105792F0BDB` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-22 23:53:19 | `F32C08C4419EAA37` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-22 23:53:19 | `BF1FB959A3CA0876` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-22 23:53:21 | `06972C99F49CBC0B` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-22 23:36:33**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,23 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 19,86 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,57 s | [↓ A4](#a4) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 0,60 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,17 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,23 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,70 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,27 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,23 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,18 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine vier eigenen Objekte, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,18 s | [↓ A14](#a14) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,11 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,13 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,09 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,16 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

stdout:

```text
  ok      Latenz ist 0 Samples
  ok      Tail ist 0 s
  ok      1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)
  ok      kein falscher NaN-Alarm bei endlichem Signal
  ok      NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)
  ok      NaN/Inf wurde als Mangel registriert
  ok      Mono-Layout wird angenommen
  ok      5.1 wird klar abgelehnt (kein stilles Downmixen)
  ok      Save/Load: Sensor-ID bleibt stabil
  ok      Save/Load: Rolle und Paar-ID bleiben
NULLTEST OK
```

stderr:

_(leer)_

<a id="a2"></a>
#### A2 | EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,23 s

stdout:

```text
sinus-1k-minus12     32 ok, 0 Fehler
pink-minus20         39 ok, 0 Fehler
resonanz-116hz       41 ok, 0 Fehler
stille-mit-tick       7 ok, 0 Fehler
diag-scoop-mitte     10 ok, 0 Fehler
diag-mulm            12 ok, 0 Fehler
diag-haerte           8 ok, 0 Fehler
diag-mulm-halb       10 ok, 0 Fehler
diag-wander-ton      25 ok, 0 Fehler
stereo-antiphase     10 ok, 0 Fehler
stereo-nur-links      8 ok, 0 Fehler
stereo-nur-rechts     5 ok, 0 Fehler
stereo-dekorr         7 ok, 0 Fehler
nan-recovery          9 ok, 0 Fehler
nyquist-32k           4 ok, 0 Fehler
leicht-publikation    8 ok, 0 Fehler
konvergenz            4 ok, 0 Fehler

GOLDEN OK — 239 Pruefungen ok, 0 Fehler (metrics m4.1-2026-08-15 · diagnose m4.1-2026-08-15)
```

stderr:

_(leer)_

<a id="a3"></a>
#### A3 | EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 19,86 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,57 s

stdout:

```text

running 51 tests
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::stale_wird_sichtbar_aber_nie_entfernt ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_ltas_array_wird_gezaehlt_verworfen ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 51 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 7 tests
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test quantisierung_stimmt_mit_den_testvektoren ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 7 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.09s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.06s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 0,60 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
47 Definitionen, 17 Nachrichtenfamilien, 153 Fixtures
Pruefungen: 177 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 38  
  const                     4 / 4    HART
  enum                     14 / 14   HART
  exclusiveMinimum          2 / 2   
  items                     1 / 4   
  maxItems                  3 / 4   
  maxLength                 2 / 15  
  maxProperties             2 / 3   
  maximum                   4 / 14  
  minItems                  2 / 4   
  minLength                 4 / 9   
  minimum                   6 / 40  
  oneOf                     4 / 4   
  pattern                   1 / 1   
  required                 38 / 169 
  type                      8 / 134 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:610: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,17 s

stdout:

```text
36 gueltige, 117 ungueltige Fixtures
  bytegleich: 155 Dateien, MANIFEST sha256=bc72c521906af48592413ffa28064bdeb8feddee2955ec50093fbdda4a421b2c
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,70 s

stdout:

```text
9 gueltige, 42 ungueltige Fixtures
  bytegleich: 52 Dateien, MANIFEST sha256=15bcde347cfa559c0b2992c5f9f5f6e7583f64e7b4ba33347b4c46f808942a35
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,27 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (758764 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (6941696 Byte)
Nadeln    : 13 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die vier Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

21 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: PLUGIN_CODE unveraendert  [Eqcp]
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE unveraendert  [Evna]
  ok      CMake-Quelle: PRODUCT_NAME unveraendert
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [23 Aug 2026 12:09:18am vs 22 Aug 2026 11:36:33pm]
  ok      moduleinfo.json ist nach dem Kommaputz parsebar
  ok      moduleinfo: Produktname wie im Manifest  [EQ-Copilot]
  ok      moduleinfo: Vendor wie im Manifest  [evenacadia]
  ok      moduleinfo: genau zwei Klassen (Component + Controller)  [2]
  ok      moduleinfo: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      moduleinfo: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      moduleinfo: Unterkategorien wie im Manifest
  ok      moduleinfo: keine der vier reservierten Sonden-CIDs im heutigen Bundle
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      CID nachgerechnet: active-probe Component (NkAc)  [ABCDEF019182FAEB45766E614E6B4163]
  ok      CID nachgerechnet: active-probe Controller (NkAc)  [ABCDEF011234ABCD45766E614E6B4163]
  ok      VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus  [5653544571637065712D636F70696C6F]
  ok      die echte CID stammt aus dem jucePluginId-Pfad
  ok      Golden-Ordner vorhanden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\identity]
  ok      Rolle 'sensor' geladen  [sensor]
  ok      Rolle 'sensor': Sensor-ID unveraendert uebernommen
  ok      Rolle 'sensor': Label uebernommen  [Klavier A]
  ok      Rolle 'sensor': Paar-ID uebernommen
  ok      Golden 'sensor' gelesen  [state-schema1-sensor.bin]
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 67 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      17 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [17]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 17 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [b53c16cf1ede4673b0c8027311553e25]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [b23693f19a6b42508ab16b0308d66a49]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 130 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
== A · Quellhash-Gate (JUCE 8.0.9) ==
  ok      JUCE-Wrapper liegt am gepinnten Ort: C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp
  ok      Patchdatei liegt im Repo: juce-8.0.9-nakama-vst3-bridge.patch
  ok      die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)
  ok      Wrapper liess sich lesen
  ok      Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: 6e5d4660d960836a…)
  ok      Wrapper ist NICHT der unberuehrte Auslieferungsstand
  ok      Patch: Bruecken-Header eingebunden
  ok      Patch: IAudioPresentationLatency als Basisklasse
  ok      Patch: Interface auch im queryInterface
  ok      Patch: Buslatenz-Methode umgesetzt
  ok      Patch: Blockbeginn verdrahtet
  ok      Patch: Context-Anwesenheit verdrahtet
  ok      Patch: fehlender Context wird ausdruecklich gemeldet
  ok      Patch: alle Parameterpunkte werden beobachtet
  ok      Patch: Uebergabe an die Senke verdrahtet
  ok      Patch: Bruecke als Wrapper-Member
  ok      Patch: Senke wird EINMAL beim Erzeugen aufgeloest
  ok      Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert
  ok      ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)
  ok      Patchdatei ist ein echter Unified Diff
  ok      Patch zielt auf den VST3-Wrapper
== B · Context-Anwesenheit und Gueltigkeitsbits ==
  ok      Senke wird genau einmal je Block gerufen
  ok      fehlender Context: processContextPresent == false
  ok      fehlender Context: Projektzeit ist UNGUELTIG, nicht 0
  ok      fehlender Context: Tempo und PPQ ungueltig
  ok      genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)
  ok      genullter Context: Projektzeit gueltig und 0 â eine Aussage, keine Vermutung
  ok      genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)
  ok      genullter Context: Tempo bleibt ungueltig
  ok      genullter Context: playing ist gueltig false â nicht 'unbekannt'
  ok      voller Context: anwesend
  ok      voller Context: projectTimeSamples 123456
  ok      voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)
  ok      voller Context: playing true
  ok      voller Context: recording true
  ok      voller Context: Tempo 128
  ok      voller Context: PPQ 8.25
  ok      voller Context: Taktbeginn 8.0 PPQ
  ok      voller Context: Cycle 4.0â12.0 PPQ, aktiv (PPQ, nicht Samples)
  ok      Teil-Context: playing true
  ok      Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)
  ok      Teil-Context: Cycle nicht aktiv
  ok      beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)
== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==
  ok      drei Punkte angekommen
  ok      Punkte sind nach Sample-Offset sortiert
  ok      sauberer Block: sampleAccurateAutomation == true
  ok      Blockgroesse wird mitgefuehrt
  ok      kleinerer Offset zuerst, auch wenn spaeter geliefert
  ok      gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)
  ok      Punkte aus mehreren Queues sind global aufsteigend
  ok      Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte
  ok      Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt
  ok      Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ueberlauf des Ereignisrings verwirft KEINEN Letztwert
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      NaN nach Ueberlauf: Zusicherung faellt
  ok      NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Letztwert-Tabelle: genau kMaxLetztwerte Parameter werden gefuehrt
  ok      Letztwert-Tabelle: die 3 ueberzaehligen Parameter sind gezaehlt
  ok      Letztwert-Ueberlauf laesst die Zusicherung ebenfalls fallen
  ok      Letztwert-Ueberlauf: der ueberzaehlige Parameter meldet ehrlich 'nichts'
  ok      ohne Ueberlauf: Rueckfallwert deckt sich mit dem letzten Ringeintrag (0.30)
  ok      ohne Ueberlauf: der Ring traegt denselben Wert am Ende
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      der Buszaehler steht im Blockbefund, ist also fuer die Senke sichtbar
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (7880 Byte)

HOSTKONTEXT OK — 91/91 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3b"></a>
#### B3b | EqCopHostProbeTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,09 s

stdout:

```text
== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==
  ok      meldet 0 Samples Latenz
  ok      meldet 0 s Tail
  ok      200 Bloecke float: Ausgang bitgleich zum Eingang
  ok      double-Block: Ausgang bitgleich zum Eingang
  ok      double-Praezision wird dem Host angeboten
  ok      5.1 wird klar abgelehnt, nicht still gemischt
== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==
  ok      dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch
  ok      vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT
  ok      nach dem ersten Bruecken-Block meldet es: Bruecke liefert
== C - Context-Anwesenheit und Gueltigkeiten ==
  ok      acht Bloecke mit Context gezaehlt
  ok      der eine Block OHNE Context ist getrennt gezaehlt
  ok      neun verarbeitete Bloecke
  ok      neun Senke-Aufrufe - keine Asymmetrie ohne Grund
  ok      der Kontextverlust ist als Ereignis vermerkt
  ok      Tempo war in jedem Context gueltig
  ok      continousTimeSamples war NIE gueltig - und wird nicht erfunden
== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==
  ok      fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)
  ok      Seek vorwaerts wird als Sprung erkannt
  ok      die Sprungweite wird mitgemessen
  ok      Seek rueckwaerts wird als Sprung erkannt
  ok      nach Stop/Play wird der Positionswechsel NICHT als laufender Sprung gemeldet
  ok      er verschwindet aber auch nicht: eigener Zaehler 'ueber Stop/Play' (T2-Befund 21.08.)
  ok      und ein eigenes Ereignis, damit eine 0 bei den Spruengen nicht mehrdeutig ist
  ok      Stop und Play sind als Transportereignisse vermerkt
  ok      Loop-Wechsel erscheint als Rueckwaertssprung
  ok      Schleife-an ist genau einmal vermerkt
  ok      der Schleifenzustand wird gefuehrt
  ok      Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)
== E - Samplegenaue Automation: der Kernbeweis von Termin B ==
  ok      ohne Automation wird nichts behauptet
  ok      ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben
  ok      alle Punkte werden aufsummiert
  ok      der dichteste Block wird gemerkt
  ok      Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit
  ok      kleinster und groesster Offset werden gefuehrt
  ok      der erste Mehrpunkt-Block ist als Ereignis vermerkt
  ok      Mehrpunkt-Block OHNE Zusicherung zaehlt NICHT als samplegenau
  ok      er wird stattdessen getrennt gezaehlt - kein Befund verschwindet
  ok      Unplausibilitaeten und Ueberlaeufe der Bruecke werden AUFSUMMIERT uebernommen, nicht geglaettet
  ok      beide Bloecke ohne Zusicherung sind gezaehlt (der mit Offset- und der mit Wertfehler)
== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==
  ok      ohne Hostmeldung: 'nie gemeldet' - und KEIN Eintrag wird erfunden
  ok      Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten
  ok      die Gegenrichtung bleibt ungemeldet - es wird kein Wert erfunden
  ok      Ausgang Bus 0 = 1024 wird NEBEN dem Eingang gefuehrt, nicht verschluckt
  ok      auch ein dritter Bus kommt an
  ok      ein nicht gemeldeter Bus dazwischen bleibt ungemeldet
  ok      der zuerst gemeldete Latenzwert rastet ein
  ok      die spaetere Aenderung wird gezaehlt statt still verworfen
  ok      50 weitere Bloecke mit DEMSELBEN geaenderten Wert zaehlen NICHT weiter (Uebergaenge, nicht Bloecke)
  ok      ein dritter, wieder anderer Wert zaehlt als zweiter Uebergang
  ok      der Bericht listet genau die drei gemeldeten Eintraege - keinen erfundenen
  ok      und der Wertwechsel-Zaehler steht im Bericht - mit der Einheit im Schluesselnamen
== G - Senke ohne processBlock (Flush/Hostriegel) ==
  ok      Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt
  ok      der Block ohne Verarbeitung ist ausdruecklich vermerkt
== H - Gegenpfad: Bericht schreiben und wieder einlesen ==
  ok      der Bericht ist nicht leer
  ok      der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)
  ok      Schemaname steht im Bericht
  ok      der Bericht traegt die Kernaussage 'samplegenau belegt'
  ok      die Blockzahl im Bericht deckt sich mit der Messung
  ok      das Ereignisprotokoll liegt im Bericht
  ok      die Gueltigkeitsmaske steht im Bericht
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260823-000953.json
  ok      die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung
== I - Zuruecksetzen und Ringueberlauf ==
  ok      Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung
  ok      nach dem Zuruecksetzen wird die laufende Aufnahme wieder als aufnahme_an gemeldet
  ok      mehr Ereignisse als Ringplaetze wurden erzeugt
  ok      die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still
  ok      der Ring gibt genau seine Kapazitaet heraus
  ok      die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)
== I2 - Offline-Uebergang und negative Projektzeit ==
  ok      Offline- und Echtzeitbloecke getrennt gezaehlt
  ok      der Wechsel IN den Render ist vermerkt
  ok      der Wechsel ZURUECK in Echtzeit ist ebenfalls vermerkt - nicht nur der erste Block ueberhaupt
  ok      negative Projektzeit wird GEZAEHLT, nicht still uebersprungen
  ok      der erste negative Wert ist als Ereignis vermerkt
  ok      im negativen Bereich wird kein Scheinsprung erfunden
== J - Audiothread: keine Allokation ==
  ok      500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen
== J2 - Nebenlaeufig lesen, waehrend der Audiothread schreibt ==
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (34426 Bloecke)
  ok      200 nebenlaeufige Lesevorgaenge liefern durchweg plausible Eintraege und Zaehler
  ok      der letzte Lesevorgang hat Eintraege geliefert
  hinweis   Grenze: Rauchtest, kein Beweis der Tearing-Freiheit (der Wiederholpfad wird selten bis nie betreten)
== K - Anzeige: passt der Inhalt ueberhaupt ins Fenster? ==
  ok      der Editor laesst sich erzeugen
  ok      Fensterhoehe 850 deckt den Leerzustand (850 noetig)
  ok      Fensterhoehe deckt auch den Messzustand (850 noetig)
  ok      keine Textzeile liegt unter den Knoepfen (Text bis 760, Knopfstreifen ab 794)
  ok      keine Wertzeile ohne Beschriftung - und 27 Zeilen wurden dabei wirklich geprueft
  ok      hoechstens EINE unbeschriftete Hinweiszeile (gefunden: 1)
  ok      und der Riegel faellt: eine untergeschobene Wertzeile ohne Beschriftung wird erkannt

HOSTPROBE OK - 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3c"></a>
#### B3c | EqCopSchemaTest

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,16 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [59 Faelle]
[ok]   Falltabelle hat Substanz  [59 Faelle]
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (51 Fixtures)
[ok]   Binaerkorpus hat Substanz  [51 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (153 Fixtures)
[ok]   Korpus hat Substanz  [153 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
56 bestanden, 0 gescheitert
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 6,89 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostProbeTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_rc_lib.dir\Release\EqCopHostProbeTest_rc_lib.lib
  EqCopHostProbeTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSchemaTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_rc_lib.dir\Release\EqCopSchemaTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
```

</details>

---

## 5. T2 — Frischkontext-Prüfer

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

## Kanon-Lauf - SONDE-007a T2-Nacharbeit

**Lauf:** 2026-08-23 00:32 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 19/19 Kanon-Laeufe bestanden | 4 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-23 00:32:32 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 75afae2 Hub: naechster Schritt ist S8s Nachpruefung, nicht S9 |
| Commit (voll) | 75afae29c21b7c77045eba103b2e5e41631c4d09 |
| Arbeitsbaum | 16 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M briefing-hub/app/briefing-app.tsx
 M briefing-hub/app/globals.css
 M briefing-hub/data/friendly-copy.ts
 M briefing-hub/data/hub.json
 M design/docs/arbeitsplan.md
 M design/docs/oberflaechen-spezifikation.md
 M docs/beweise/SONDE-007a.md
 M eq-copilot/cmake/NakamaKern.cmake
 D eq-copilot/design/ASSET-KIT.md
 D eq-copilot/design/eq-copilot-material-preview.png
 M eq-copilot/plugin/CMakeLists.txt
 M nimbalyst-local/automations/planstand-nakama.md
 M tools/eq-copilot/pruefe_kern_identitaetsfrei.py
?? "Untitled Workspace/"
?? design/docs/interaktions-und-motion-spezifikation.md
?? nimbalyst-local/automations/planstand-nakama/
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-23 00:32:40 | `2DE23955049AC2AE` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-23 00:32:42 | `E88C24C69ECCC8C6` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-23 00:32:43 | `860A3F2341D18C0A` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-23 00:32:43 | `00BAD31020E212F6` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-23 00:32:46 | `50EBD38724945CCC` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-23 00:32:10**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,28 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 20,84 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 1,02 s | [↓ A4](#a4) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 0,69 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,14 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,18 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,26 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,71 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,26 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,24 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine vier eigenen Objekte, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,20 s | [↓ A14](#a14) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,20 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,23 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,12 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,19 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,23 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,10 s

stdout:

```text
  ok      Latenz ist 0 Samples
  ok      Tail ist 0 s
  ok      1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)
  ok      kein falscher NaN-Alarm bei endlichem Signal
  ok      NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)
  ok      NaN/Inf wurde als Mangel registriert
  ok      Mono-Layout wird angenommen
  ok      5.1 wird klar abgelehnt (kein stilles Downmixen)
  ok      Save/Load: Sensor-ID bleibt stabil
  ok      Save/Load: Rolle und Paar-ID bleiben
NULLTEST OK
```

stderr:

_(leer)_

<a id="a2"></a>
#### A2 | EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,28 s

stdout:

```text
sinus-1k-minus12     32 ok, 0 Fehler
pink-minus20         39 ok, 0 Fehler
resonanz-116hz       41 ok, 0 Fehler
stille-mit-tick       7 ok, 0 Fehler
diag-scoop-mitte     10 ok, 0 Fehler
diag-mulm            12 ok, 0 Fehler
diag-haerte           8 ok, 0 Fehler
diag-mulm-halb       10 ok, 0 Fehler
diag-wander-ton      25 ok, 0 Fehler
stereo-antiphase     10 ok, 0 Fehler
stereo-nur-links      8 ok, 0 Fehler
stereo-nur-rechts     5 ok, 0 Fehler
stereo-dekorr         7 ok, 0 Fehler
nan-recovery          9 ok, 0 Fehler
nyquist-32k           4 ok, 0 Fehler
leicht-publikation    8 ok, 0 Fehler
konvergenz            4 ok, 0 Fehler

GOLDEN OK — 239 Pruefungen ok, 0 Fehler (metrics m4.1-2026-08-15 · diagnose m4.1-2026-08-15)
```

stderr:

_(leer)_

<a id="a3"></a>
#### A3 | EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 20,84 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 1,02 s

stdout:

```text

running 51 tests
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::stale_wird_sichtbar_aber_nie_entfernt ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_ltas_array_wird_gezaehlt_verworfen ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 51 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.35s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 7 tests
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test quantisierung_stimmt_mit_den_testvektoren ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 7 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.12s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.16s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 0,69 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
47 Definitionen, 17 Nachrichtenfamilien, 153 Fixtures
Pruefungen: 177 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 38  
  const                     4 / 4    HART
  enum                     14 / 14   HART
  exclusiveMinimum          2 / 2   
  items                     1 / 4   
  maxItems                  3 / 4   
  maxLength                 2 / 15  
  maxProperties             2 / 3   
  maximum                   4 / 14  
  minItems                  2 / 4   
  minLength                 4 / 9   
  minimum                   6 / 40  
  oneOf                     4 / 4   
  pattern                   1 / 1   
  required                 38 / 169 
  type                      8 / 134 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:610: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
36 gueltige, 117 ungueltige Fixtures
  bytegleich: 155 Dateien, MANIFEST sha256=bc72c521906af48592413ffa28064bdeb8feddee2955ec50093fbdda4a421b2c
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,26 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,71 s

stdout:

```text
9 gueltige, 42 ungueltige Fixtures
  bytegleich: 52 Dateien, MANIFEST sha256=15bcde347cfa559c0b2992c5f9f5f6e7583f64e7b4ba33347b4c46f808942a35
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,26 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,24 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,20 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (700188 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7045120 Byte)
Nadeln    : 13 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als Kernquellen und Baubeschreibung

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die vier Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

21 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,20 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: PLUGIN_CODE unveraendert  [Eqcp]
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE unveraendert  [Evna]
  ok      CMake-Quelle: PRODUCT_NAME unveraendert
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [23 Aug 2026 12:33:06am vs 23 Aug 2026 12:32:10am]
  ok      moduleinfo.json ist nach dem Kommaputz parsebar
  ok      moduleinfo: Produktname wie im Manifest  [EQ-Copilot]
  ok      moduleinfo: Vendor wie im Manifest  [evenacadia]
  ok      moduleinfo: genau zwei Klassen (Component + Controller)  [2]
  ok      moduleinfo: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      moduleinfo: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      moduleinfo: Unterkategorien wie im Manifest
  ok      moduleinfo: keine der vier reservierten Sonden-CIDs im heutigen Bundle
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      CID nachgerechnet: active-probe Component (NkAc)  [ABCDEF019182FAEB45766E614E6B4163]
  ok      CID nachgerechnet: active-probe Controller (NkAc)  [ABCDEF011234ABCD45766E614E6B4163]
  ok      VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus  [5653544571637065712D636F70696C6F]
  ok      die echte CID stammt aus dem jucePluginId-Pfad
  ok      Golden-Ordner vorhanden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\identity]
  ok      Rolle 'sensor' geladen  [sensor]
  ok      Rolle 'sensor': Sensor-ID unveraendert uebernommen
  ok      Rolle 'sensor': Label uebernommen  [Klavier A]
  ok      Rolle 'sensor': Paar-ID uebernommen
  ok      Golden 'sensor' gelesen  [state-schema1-sensor.bin]
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 67 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      17 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [17]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 17 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [bc0a5810418f4b56a73287e25d6364fd]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [51867aac30384ba7bdf2714900012f8f]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 130 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,12 s

stdout:

```text
== A · Quellhash-Gate (JUCE 8.0.9) ==
  ok      JUCE-Wrapper liegt am gepinnten Ort: C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp
  ok      Patchdatei liegt im Repo: juce-8.0.9-nakama-vst3-bridge.patch
  ok      die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)
  ok      Wrapper liess sich lesen
  ok      Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: 6e5d4660d960836a…)
  ok      Wrapper ist NICHT der unberuehrte Auslieferungsstand
  ok      Patch: Bruecken-Header eingebunden
  ok      Patch: IAudioPresentationLatency als Basisklasse
  ok      Patch: Interface auch im queryInterface
  ok      Patch: Buslatenz-Methode umgesetzt
  ok      Patch: Blockbeginn verdrahtet
  ok      Patch: Context-Anwesenheit verdrahtet
  ok      Patch: fehlender Context wird ausdruecklich gemeldet
  ok      Patch: alle Parameterpunkte werden beobachtet
  ok      Patch: Uebergabe an die Senke verdrahtet
  ok      Patch: Bruecke als Wrapper-Member
  ok      Patch: Senke wird EINMAL beim Erzeugen aufgeloest
  ok      Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert
  ok      ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)
  ok      Patchdatei ist ein echter Unified Diff
  ok      Patch zielt auf den VST3-Wrapper
== B · Context-Anwesenheit und Gueltigkeitsbits ==
  ok      Senke wird genau einmal je Block gerufen
  ok      fehlender Context: processContextPresent == false
  ok      fehlender Context: Projektzeit ist UNGUELTIG, nicht 0
  ok      fehlender Context: Tempo und PPQ ungueltig
  ok      genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)
  ok      genullter Context: Projektzeit gueltig und 0 â eine Aussage, keine Vermutung
  ok      genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)
  ok      genullter Context: Tempo bleibt ungueltig
  ok      genullter Context: playing ist gueltig false â nicht 'unbekannt'
  ok      voller Context: anwesend
  ok      voller Context: projectTimeSamples 123456
  ok      voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)
  ok      voller Context: playing true
  ok      voller Context: recording true
  ok      voller Context: Tempo 128
  ok      voller Context: PPQ 8.25
  ok      voller Context: Taktbeginn 8.0 PPQ
  ok      voller Context: Cycle 4.0â12.0 PPQ, aktiv (PPQ, nicht Samples)
  ok      Teil-Context: playing true
  ok      Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)
  ok      Teil-Context: Cycle nicht aktiv
  ok      beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)
== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==
  ok      drei Punkte angekommen
  ok      Punkte sind nach Sample-Offset sortiert
  ok      sauberer Block: sampleAccurateAutomation == true
  ok      Blockgroesse wird mitgefuehrt
  ok      kleinerer Offset zuerst, auch wenn spaeter geliefert
  ok      gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)
  ok      Punkte aus mehreren Queues sind global aufsteigend
  ok      Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte
  ok      Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt
  ok      Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ueberlauf des Ereignisrings verwirft KEINEN Letztwert
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      NaN nach Ueberlauf: Zusicherung faellt
  ok      NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Letztwert-Tabelle: genau kMaxLetztwerte Parameter werden gefuehrt
  ok      Letztwert-Tabelle: die 3 ueberzaehligen Parameter sind gezaehlt
  ok      Letztwert-Ueberlauf laesst die Zusicherung ebenfalls fallen
  ok      Letztwert-Ueberlauf: der ueberzaehlige Parameter meldet ehrlich 'nichts'
  ok      ohne Ueberlauf: Rueckfallwert deckt sich mit dem letzten Ringeintrag (0.30)
  ok      ohne Ueberlauf: der Ring traegt denselben Wert am Ende
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      der Buszaehler steht im Blockbefund, ist also fuer die Senke sichtbar
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (7880 Byte)

HOSTKONTEXT OK — 91/91 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3b"></a>
#### B3b | EqCopHostProbeTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,19 s

stdout:

```text
== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==
  ok      meldet 0 Samples Latenz
  ok      meldet 0 s Tail
  ok      200 Bloecke float: Ausgang bitgleich zum Eingang
  ok      double-Block: Ausgang bitgleich zum Eingang
  ok      double-Praezision wird dem Host angeboten
  ok      5.1 wird klar abgelehnt, nicht still gemischt
== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==
  ok      dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch
  ok      vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT
  ok      nach dem ersten Bruecken-Block meldet es: Bruecke liefert
== C - Context-Anwesenheit und Gueltigkeiten ==
  ok      acht Bloecke mit Context gezaehlt
  ok      der eine Block OHNE Context ist getrennt gezaehlt
  ok      neun verarbeitete Bloecke
  ok      neun Senke-Aufrufe - keine Asymmetrie ohne Grund
  ok      der Kontextverlust ist als Ereignis vermerkt
  ok      Tempo war in jedem Context gueltig
  ok      continousTimeSamples war NIE gueltig - und wird nicht erfunden
== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==
  ok      fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)
  ok      Seek vorwaerts wird als Sprung erkannt
  ok      die Sprungweite wird mitgemessen
  ok      Seek rueckwaerts wird als Sprung erkannt
  ok      nach Stop/Play wird der Positionswechsel NICHT als laufender Sprung gemeldet
  ok      er verschwindet aber auch nicht: eigener Zaehler 'ueber Stop/Play' (T2-Befund 21.08.)
  ok      und ein eigenes Ereignis, damit eine 0 bei den Spruengen nicht mehrdeutig ist
  ok      Stop und Play sind als Transportereignisse vermerkt
  ok      Loop-Wechsel erscheint als Rueckwaertssprung
  ok      Schleife-an ist genau einmal vermerkt
  ok      der Schleifenzustand wird gefuehrt
  ok      Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)
== E - Samplegenaue Automation: der Kernbeweis von Termin B ==
  ok      ohne Automation wird nichts behauptet
  ok      ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben
  ok      alle Punkte werden aufsummiert
  ok      der dichteste Block wird gemerkt
  ok      Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit
  ok      kleinster und groesster Offset werden gefuehrt
  ok      der erste Mehrpunkt-Block ist als Ereignis vermerkt
  ok      Mehrpunkt-Block OHNE Zusicherung zaehlt NICHT als samplegenau
  ok      er wird stattdessen getrennt gezaehlt - kein Befund verschwindet
  ok      Unplausibilitaeten und Ueberlaeufe der Bruecke werden AUFSUMMIERT uebernommen, nicht geglaettet
  ok      beide Bloecke ohne Zusicherung sind gezaehlt (der mit Offset- und der mit Wertfehler)
== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==
  ok      ohne Hostmeldung: 'nie gemeldet' - und KEIN Eintrag wird erfunden
  ok      Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten
  ok      die Gegenrichtung bleibt ungemeldet - es wird kein Wert erfunden
  ok      Ausgang Bus 0 = 1024 wird NEBEN dem Eingang gefuehrt, nicht verschluckt
  ok      auch ein dritter Bus kommt an
  ok      ein nicht gemeldeter Bus dazwischen bleibt ungemeldet
  ok      der zuerst gemeldete Latenzwert rastet ein
  ok      die spaetere Aenderung wird gezaehlt statt still verworfen
  ok      50 weitere Bloecke mit DEMSELBEN geaenderten Wert zaehlen NICHT weiter (Uebergaenge, nicht Bloecke)
  ok      ein dritter, wieder anderer Wert zaehlt als zweiter Uebergang
  ok      der Bericht listet genau die drei gemeldeten Eintraege - keinen erfundenen
  ok      und der Wertwechsel-Zaehler steht im Bericht - mit der Einheit im Schluesselnamen
== G - Senke ohne processBlock (Flush/Hostriegel) ==
  ok      Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt
  ok      der Block ohne Verarbeitung ist ausdruecklich vermerkt
== H - Gegenpfad: Bericht schreiben und wieder einlesen ==
  ok      der Bericht ist nicht leer
  ok      der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)
  ok      Schemaname steht im Bericht
  ok      der Bericht traegt die Kernaussage 'samplegenau belegt'
  ok      die Blockzahl im Bericht deckt sich mit der Messung
  ok      das Ereignisprotokoll liegt im Bericht
  ok      die Gueltigkeitsmaske steht im Bericht
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260823-003343.json
  ok      die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung
== I - Zuruecksetzen und Ringueberlauf ==
  ok      Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung
  ok      nach dem Zuruecksetzen wird die laufende Aufnahme wieder als aufnahme_an gemeldet
  ok      mehr Ereignisse als Ringplaetze wurden erzeugt
  ok      die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still
  ok      der Ring gibt genau seine Kapazitaet heraus
  ok      die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)
== I2 - Offline-Uebergang und negative Projektzeit ==
  ok      Offline- und Echtzeitbloecke getrennt gezaehlt
  ok      der Wechsel IN den Render ist vermerkt
  ok      der Wechsel ZURUECK in Echtzeit ist ebenfalls vermerkt - nicht nur der erste Block ueberhaupt
  ok      negative Projektzeit wird GEZAEHLT, nicht still uebersprungen
  ok      der erste negative Wert ist als Ereignis vermerkt
  ok      im negativen Bereich wird kein Scheinsprung erfunden
== J - Audiothread: keine Allokation ==
  ok      500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen
== J2 - Nebenlaeufig lesen, waehrend der Audiothread schreibt ==
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (52796 Bloecke)
  ok      200 nebenlaeufige Lesevorgaenge liefern durchweg plausible Eintraege und Zaehler
  ok      der letzte Lesevorgang hat Eintraege geliefert
  hinweis   Grenze: Rauchtest, kein Beweis der Tearing-Freiheit (der Wiederholpfad wird selten bis nie betreten)
== K - Anzeige: passt der Inhalt ueberhaupt ins Fenster? ==
  ok      der Editor laesst sich erzeugen
  ok      Fensterhoehe 850 deckt den Leerzustand (850 noetig)
  ok      Fensterhoehe deckt auch den Messzustand (850 noetig)
  ok      keine Textzeile liegt unter den Knoepfen (Text bis 760, Knopfstreifen ab 794)
  ok      keine Wertzeile ohne Beschriftung - und 27 Zeilen wurden dabei wirklich geprueft
  ok      hoechstens EINE unbeschriftete Hinweiszeile (gefunden: 1)
  ok      und der Riegel faellt: eine untergeschobene Wertzeile ohne Beschriftung wird erkannt

HOSTPROBE OK - 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3c"></a>
#### B3c | EqCopSchemaTest

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [59 Faelle]
[ok]   Falltabelle hat Substanz  [59 Faelle]
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (51 Fixtures)
[ok]   Binaerkorpus hat Substanz  [51 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (153 Fixtures)
[ok]   Korpus hat Substanz  [153 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
56 bestanden, 0 gescheitert
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 34,69 s

<details><summary>Rohe Ausgabe</summary>

```text
CMake is re-running because C:/Users/phili/Projekte/Nakama/eq-copilot/build/CMakeFiles/generate.stamp is out-of-date.
  the file 'C:/Users/phili/Projekte/Nakama/eq-copilot/build/CMakeFiles/cmake.verify_globs'
  is newer than 'C:/Users/phili/Projekte/Nakama/eq-copilot/build/CMakeFiles/generate.stamp.depend'
  result='-1'
-- Selecting Windows SDK version 10.0.26100.0 to target Windows 10.0.26200.
-- Configuring juceaide
-- Building juceaide
-- Exporting juceaide
-- Testing juceaide
-- Finished setting up juceaide
-- Nakama-Bruecke: JUCE-Wrapper ist bereits gepatcht (6e5d4660d960836a875e4b2207f5bb4372b5266776e00c4bb0fdef1ee87a01bc).
-- Proceeding with version: 25.12.19.0
-- CMAKE_CXX_FLAGS: /DWIN32 /D_WINDOWS /EHsc
-- Nakama-FlatBuffers: Quellstand 25.12.19 bestaetigt
-- Nakama-FlatBuffers: flatc-Zeiger -> C:/Users/phili/Projekte/Nakama/eq-copilot/build/nakama-flatc-pfad-<CONFIG>.txt
-- Nakama-Kern: JUCE-Kopffassade 'NakamaKernJuce' aus 5 Modulzielen abgeleitet.
-- Nakama-Kern: K2 gruen — 9 Ziele in der Linkhuelle von 'NakamaKern', keine JucePlugin_-Konstante.
-- Nakama-Kern: K2b gruen — JUCE-Konfiguration von 'NakamaKern' deckt 'EqCopilot'.
-- Nakama-Kern: K2c gruen — 2 Empfehlungsschalter von 'EqCopilot' auch am Kern (juce_recommended_config_flags, juce_recommended_warning_flags); ausgenommen: juce_recommended_lto_flags (/GL ohne -LTCG im Verbraucher).
-- Configuring done (3.6s)
-- Generating done (1.1s)
-- Build files have been written to: C:/Users/phili/Projekte/Nakama/eq-copilot/build
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKanon.cpp
  NakamaParameter.cpp
  NakamaState.cpp
  NakamaVertrag.cpp
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostProbeTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_rc_lib.dir\Release\EqCopHostProbeTest_rc_lib.lib
  EqCopHostProbeTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSchemaTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_rc_lib.dir\Release\EqCopSchemaTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
     Bibliothek "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.lib" und Objekt "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.exp" werden erstellt.
  Code wird generiert.
  Codegenerierung ist abgeschlossen.
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
```

</details>


---

## Nacharbeit zum Gate G1 — 24.08.2026

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

## Kanon-Lauf - S8 Dirigent 2026-08-28 - Kanon auf 3353fb6 vor T3-Urteil

**Lauf:** 2026-08-28 21:01 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 29/29 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-28 21:01:56 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 3353fb6 S7 SONDE-006: T3 PASS 2026-08-28 - zwei frische Codex-Pruefungen, eine Nacharbeitsrunde, Planstand |
| Commit (voll) | 3353fb6f39ae3ae1901253eedaef32b7f2589359 |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-28 16:37:15 | `A8000FEF90307EEE` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-27 03:22:19 | `C89836581A6FFD48` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-28 16:37:19 | `08BFDD5F08117D89` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-27 02:25:05 | `ECA970E1DE7BA8CC` | frisch (Bau bestaetigt) |
| `EqCopSunaNullTest` | 2026-08-28 16:37:30 | `2EB04A41F6432C5F` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-28 16:37:32 | `C393C82F65A024A2` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-28 16:37:34 | `F12EA9D0EBF94F07` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-28 20:27:36 | `2421B8E6DE7490EB` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-28 18:51:57 | `69413AB1A66C7B5E` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-28 16:37:45 | `FBD7BBAAD558392D` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-27 01:11:05 | `F56913243873A4FD` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-28 16:37:49 | `91F2C529928CB0FB` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-28 16:37:51 | `ED6BD5E4CAB0D9F5` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-28 20:25:40**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,07 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 10,66 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 21,60 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,96 s | [↓ A4](#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,06 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,47 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,18 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,25 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,91 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,24 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,23 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,19 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,38 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,79 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 25,89 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [↓ A19](#a19) |
| A15 | Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | [OK] Exit 0 | 0,04 s | [↓ A15](#a15) |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,05 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,31 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,19 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,91 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,03 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
  ok      Latenz ist 0 Samples
  ok      Tail ist 0 s
  ok      1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)
  ok      kein falscher NaN-Alarm bei endlichem Signal
  ok      NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)
  ok      NaN/Inf wurde als Mangel registriert
  ok      Mono-Layout wird angenommen
  ok      5.1 wird klar abgelehnt (kein stilles Downmixen)
  ok      Save/Load: Sensor-ID bleibt stabil
  ok      Save/Load: Rolle und Paar-ID bleiben
NULLTEST OK
```

stderr:

_(leer)_

<a id="a2"></a>
#### A2 | EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 10,66 s

stdout:

```text
sinus-1k-minus12     32 ok, 0 Fehler
pink-minus20         39 ok, 0 Fehler
resonanz-116hz       41 ok, 0 Fehler
stille-mit-tick       7 ok, 0 Fehler
diag-scoop-mitte     10 ok, 0 Fehler
diag-mulm            12 ok, 0 Fehler
diag-haerte           8 ok, 0 Fehler
diag-mulm-halb       10 ok, 0 Fehler
diag-wander-ton      25 ok, 0 Fehler
stereo-antiphase     10 ok, 0 Fehler
stereo-nur-links      8 ok, 0 Fehler
stereo-nur-rechts     5 ok, 0 Fehler
stereo-dekorr         7 ok, 0 Fehler
nan-recovery          9 ok, 0 Fehler
nyquist-32k           4 ok, 0 Fehler
leicht-publikation    8 ok, 0 Fehler
konvergenz            4 ok, 0 Fehler

GOLDEN OK — 239 Pruefungen ok, 0 Fehler (metrics m4.1-2026-08-15 · diagnose m4.1-2026-08-15)
```

stderr:

_(leer)_

<a id="a3"></a>
#### A3 | EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 21,60 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: als Main klassifiziert (§53.5)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: als Main klassifiziert (§53.5)
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: als Main klassifiziert (§53.5)
  ok      T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: als Main klassifiziert (§53.5)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: als Main klassifiziert (§53.5)
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: beide Instanzen als Main klassifiziert (§53.5)
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
  ok      T11: als Main klassifiziert (§53.5)
  ok      T11: Auftrag gebaut
  ok      T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)
  ok      T11: und die Markierung meldet sich nicht hoerbar
  ok      T11: mit gestopptem Transport ebenfalls kein Sample
  ok      T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,96 s

stdout:

```text

running 90 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test dto::tests::as_f64_liefert_fuer_jede_number_einen_wert ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test dto::tests::bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege ... ok
test aggregat::tests::schreiben_erzeugt_haltbare_datei ... ok
test dto::tests::doppelter_schluessel_wird_nicht_still_ueberschrieben ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test dto::tests::geschachtelter_doppelter_schluessel_faellt_ebenfalls ... ok
test aggregat::tests::alte_temp_restdatei_blockiert_snapshot_nicht ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test bindung::tests::fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow ... ok
test aggregat::tests::gleicher_zeitstempel_ueberschreibt_keinen_snapshot ... ok
test protokoll::tests::gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt ... ok
test protokoll::tests::heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::hello_enum_und_audiofelder_werden_semantisch_begrenzt ... ok
test protokoll::tests::hello_textfelder_haben_codepunktgenaue_harte_grenzen ... ok
test protokoll::tests::messstand_guard_prueft_minima_endlichkeit_und_textcap ... ok
test protokoll::tests::option_felder_akzeptieren_explizites_null_wie_das_schema ... ok
test bindung::tests::wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand ... ok
test protokoll::tests::unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset ... ok
test register_tests::owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test bindung::tests::parallele_schreiber_hinterlassen_genau_einen_intakten_stand ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen ... ok
test aggregat::tests::parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id ... ok
test register_tests::stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge ... ok
test register_tests::unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::fremdes_session_token_beendet_established_statt_acklos_zu_haengen ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::ungelesenes_welcome_blockiert_connection_ende_nicht ... ok
test server::tests::vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v1_heartbeat_mit_measurement_ist_protokollbruch ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test vertrag::tests::textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python ... ok
test server::tests::sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung ... ok
test server::tests::sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten ... ok
test server::tests::zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen ... ok
test server::tests::zweites_hello_kann_die_established_bindung_nicht_umschreiben ... ok
test server::tests::bye_braucht_exakt_das_ausgegebene_session_token ... ok
test server::tests::ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz ... ok
test server::tests::connection_und_pipe_instanzzahl_bleiben_hart_begrenzt ... ok
test server::tests::ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist ... ok
test server::tests::acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup ... ok

test result: ok. 90 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.46s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 8 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test dto_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 8 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.13s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.07s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a4b"></a>
#### A4b | EqCopPipeClientTest

**Befehl:** `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | **Exitcode:** 0 | **Dauer:** 1,06 s

stdout:

```text
PIPECLIENT-LIFECYCLE-TEST
  ok      Fake-Pipe fuer Welcome-Stopp angelegt
  ok      stop() bricht blockiertes Welcome-Lesen begrenzt ab  [0 ms]
  ok      SQOS begrenzt den Peer auf SecurityIdentification  [1]
  ok      Fake-Pipe fuer ACK-Stopp angelegt
  ok      stop() bricht blockiertes ACK-Lesen begrenzt ab  [0 ms]
  ok      Reconnect-Fake-Pipe angelegt
  ok      Reconnect waehrend Welcome verliert keine Konfigurationsgeneration
  ok      ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt
  ok      Welcome mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ACK mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers  [328 ms]
  ok      u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze
  ok      200 konkurrierende start/stop-Paare ohne joinable-Leak
PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler
```

stderr:

_(leer)_

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 1,47 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
52 Definitionen, 17 Nachrichtenfamilien, 188 Fixtures
Pruefungen: 284 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 49  
  const                     8 / 8    HART
  enum                     10 / 10   HART
  exclusiveMinimum          2 / 2   
  items                     3 / 8   
  maxItems                  3 / 8   
  maxLength                 4 / 20  
  maxProperties             2 / 3   
  maximum                   6 / 18  
  minItems                  2 / 8   
  minLength                 5 / 14  
  minimum                   8 / 48  
  oneOf                     7 / 8   
  pattern                   4 / 9   
  required                 40 / 229 
  type                     11 / 165 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:933: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
42 gueltige, 146 ungueltige Fixtures
  bytegleich: 190 Dateien, MANIFEST sha256=bedc885d7e2ca10aebadee1d41b04187ed61b2792649804d04600d5dbb96cbee
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  flatc-Beleg: Commit 7e163021e59cca4f8e1e35a7c828b5c6b7915953, sha256=c4cf81a4f940f91a29c0d9f627654635bed88b5307f4e0f640ca26a63d9f3018  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\nakama-flatc-commit-Release.txt)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,91 s

stdout:

```text
9 gueltige, 53 ungueltige Fixtures
  bytegleich: 63 Dateien, MANIFEST sha256=b6be5564a60ba6e524ea174807c253c7ad7ee0e810bef2260cc6eba52f1e8282
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,24 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,19 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,38 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (737942 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7105024 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      die Lib wurde mit der heutigen Definemenge gebaut (15 aus der Projektdatei)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii]
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
  ok      Archivmitglieder sind genau die 5 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

26 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 0,79 s

stdout:

```text
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag  [3 vs 3; identity=ok]
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID im Installer-Manifest
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest
  ok      faellt am verdorbenen Manifest: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt am verdorbenen Manifest: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt am verdorbenen Manifest: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt am verdorbenen Manifest: Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt am verdorbenen Manifest: kein Viercode, keine Class-ID im Installer-Manifest
  ok      faellt am verdorbenen Manifest: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt am verdorbenen Manifest: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt am verdorbenen Manifest: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt am verdorbenen Manifest: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt am verdorbenen Manifest: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt am verdorbenen Manifest: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt am verdorbenen Manifest: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[4] Auslieferungsstand
  hinweis nicht ausgeliefert - 4 Artefakt(e) ohne Hash: main, passive-probe, active-probe, eqcop-broker.exe
          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

38 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 25,89 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
  ok      kanonische VST3-Produktpolicy steht unveraendert im Original
  ok      kanonische Broker-Produktpolicy steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle
      - $erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
      + $erlaubteVst3Basis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-1db5_m_3\ziel\VST3'
      - $erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
      + $erlaubteBrokerBasis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-1db5_m_3\ziel\programme\evenacadia\Nakama'

[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
  ok      Bundle ohne erwartete innere Payload wird fail-closed verweigert  [Exit 1]
  ok      Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0: phili\AppData\Local\Temp\nakama-gegenpfad-1db5_m_3\ziel\VST3\EQ-Copilot.vst3]
  ok      Nakama Suna installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-1db5_m_3\ziel\VST3\Nakama Suna.vst3]
  ok      Nakama Probeeq installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-1db5_m_3\ziel\VST3\Nakama Probeeq.vst3]
  ok      eqcop-broker.exe installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-1db5_m_3\ziel\programme\evenacadia\Nakama\eqcop-broker.exe]
INSTALLATION OK
]
  ok      geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg
  ok      jede Sicherung ist relativ an backups/<transaktions-id> gebunden
  ok      Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet
  ok      beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM
  ok      wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      passive-probe: Bundle traegt moduleinfo.json  [Nakama Suna.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[2b] Aktiver Recovery-Anker wird nicht still superseded
  ok      geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie  [Exit 1]
  ok      VORBEREITET verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      VORBEREITET: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      KOMPENSATION verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      KOMPENSATION: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      ERROR_TEILSTAND verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      ERROR_TEILSTAND: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      RUECKWEG_AKTIV verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      RUECKWEG_AKTIV: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded  [Exit 0]
  ok      Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren
  ok      zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle vier Artefakte melden `aktuell`  [4x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben
  ok      frische Transaktion friert den nun bekannten historischen Stand ein
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht
  ok      passive-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Suna.vst3]
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      das im Journal belegte Broker-Ziel ist weg
  ok      unbelegte Elternverzeichnisse bleiben unangetastet
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert
  ok      spaeter Installationsfehler endet laut und nicht als Teilerfolg  [Exit 1]
  ok      Journal bestaetigt die vollstaendige Kompensation  [ERROR_RUECKGEROLLT]
  ok      Main ist nach der Kompensation bytegleich zum Vorzustand
  ok      passive-probe: kein Teilstand nach spaetem Fehler
  ok      active-probe: kein Teilstand nach spaetem Fehler
  ok      der fremde Blocker wurde nicht angetastet
  ok      vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei
  ok      ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden  [Exit 0]
  ok      nach Fehler-Supersede bleibt nur die aktive Backup-Generation

[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig
  ok      Ausgangsinstallation fuer den Hard-Crash-Zwischenstand
  ok      VORBEREITET + mutation_begonnen restauriert statt Drift zu melden  [Exit 0]
  ok      der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt
  ok      Ausgangsinstallation fuer einen unterbrochenen inversen Pfad
  ok      RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort  [Exit 0]
  ok      auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand

[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen
  ok      Ausgangsinstallation fuer Journal-Missbrauchstests
  ok      manipulierte Repo-Diagnose ist keine Rueckwegautoritaet  [Exit 0]
  ok      fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben  [Exit 1]
  ok      fremder Loeschpfad im Journal wird nicht entfernt  [Exit 1]
  ok      Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen  [Exit 1]

[11] Geschuetzte Autoritaet, Backup- und Zieldrift
  ok      oeffentliches Journal allein autorisiert keinen Rueckweg  [Exit 1]
  ok      Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash  [Exit 1]
  ok      frische Ausgangsinstallation fuer Zieldrift
  ok      veraendertes Installationsziel wird nicht ueberschrieben  [Exit 1]
  ok      Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet

[12] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2
  ok      Runtime verwirft eine Bundle-Zielkollision  [Exit 1]
  ok      Runtime bindet Identity typstreng an State-Schema 2  [Exit 1]
  ok      Runtime verwirft kanonisches Broker-Zieltraversal  [Exit 1]

80 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a19"></a>
#### A19 | erzeuge_bandgitter_header.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen (204 Zeilen).
    Evidenz nakama_1_24_oct_30_18k_v1: 221 Baender, 222 Kanten
    Live    nakama_log64_v1: 64 Gruppen, lueckenlose Partition der 221
```

stderr:

_(leer)_

<a id="a15"></a>
#### A15 | EqCopSunaNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse passive_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [passive_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [172 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [passive_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [172 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse active_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [active_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [3439 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [3439 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes Ziel im Manifest hat hier eine Zeile  [3 vs 3]
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest
  ok      CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle: COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle: der Viercode steht nicht literal im Zielblock  [Eqcp]
  ok      CMake-Quelle: der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      CMake-Quelle: der Produktname steht nicht literal im Zielblock
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden
  ok      CMake: Defineblock der Sondenfunktion gefunden
  ok      CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      passive-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkPr]
  ok      passive-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Suna]
  ok      passive-probe: NakamaSuna entsteht ueber nakama_sonde_ziel()
  ok      passive-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaSuna    passive-probe NAKAMA_SONDE_PASSIV)]
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 9:02:23pm vs 27 Aug 2026 2:27:21am]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      passive-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\Resources\moduleinfo.json]
  ok      passive-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 9:02:26pm vs 27 Aug 2026 2:27:21am]
  ok      passive-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      passive-probe: Produktname wie im Manifest  [Nakama Suna]
  ok      passive-probe: Vendor wie im Manifest  [evenacadia]
  ok      passive-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      passive-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: Unterkategorien wie im Manifest
  ok      passive-probe: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 9:02:28pm vs 27 Aug 2026 2:27:21am]
  ok      active-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      active-probe: Produktname wie im Manifest  [Nakama Probeeq]
  ok      active-probe: Vendor wie im Manifest  [evenacadia]
  ok      active-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      active-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B4163]
  ok      active-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B4163]
  ok      active-probe: Unterkategorien wie im Manifest
  ok      active-probe: keine fremde Ziel-CID im Bundle
  ok      Viercode ist vier Zeichen lang: main.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      Viercode ist vier Zeichen lang: passive-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      Viercode ist vier Zeichen lang: active-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: active-probe Component (NkAc)  [ABCDEF019182FAEB45766E614E6B4163]
  ok      CID nachgerechnet: active-probe Controller (NkAc)  [ABCDEF011234ABCD45766E614E6B4163]
  ok      VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus  [5653544571637065712D636F70696C6F]
  ok      die echte CID stammt aus dem jucePluginId-Pfad
  ok      Golden-Ordner vorhanden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\identity]
  ok      Rolle 'sensor' geladen  [sensor]
  ok      Rolle 'sensor': Sensor-ID unveraendert uebernommen
  ok      Rolle 'sensor': Label uebernommen  [Klavier A]
  ok      Rolle 'sensor': Paar-ID uebernommen
  ok      Golden 'sensor' gelesen  [state-schema1-sensor.bin]
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 114 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,31 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      Matrix Klasse=main, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=main, Position=pre: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position pre is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=legacy, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for legacy', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=passive_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for passive_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=active_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for active_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      16 Kombinationen aus Klasse x Messposition wie §2.2 - post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)  [16]
  ok      == Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle
  ok      21 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [21]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser
  ok      gueltiger Praefix mit Suffix wird vollstaendig ignoriert
  ok      abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen
  ok      deklarierte Riesenvariante wird vor JUCE-Allokation verworfen
  ok      zu tiefer ValueTree wird begrenzt und ignoriert
  ok      zu tief geschachteltes Variantenarray wird begrenzt und ignoriert
  ok      State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen
  ok      ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel
  ok      63 verschachtelte Arrays plus Blatt passieren und laden
  ok      65.536 Eintraege in einer Sammlung passieren den Byte-Riegel
  ok      65.537 Eintraege in einer Sammlung werden verworfen
  ok      262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel
  ok      262.145 Eintraege ueber mehrere Sammlungen werden verworfen
  ok      Teststate trifft die schreibbare Bytegrenze exakt  [16773120]
  ok      Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773848]
  ok      State ohne konkreten Writer-Headroom bleibt read-only bytegleich
  ok      vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [9]
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [10]
  ok      additive Binaer- und Array-Properties bleiben bytegleich lesbar
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [6621dc5e6f2f407a9f711092a0d4532a]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [34c231fb859049bbaa7995a22cd7d26e]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 169 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
== A · Quellhash-Gate (JUCE 8.0.9) ==
  ok      JUCE-Wrapper liegt am gepinnten Ort: C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp
  ok      Patchdatei liegt im Repo: juce-8.0.9-nakama-vst3-bridge.patch
  ok      die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)
  ok      Wrapper liess sich lesen
  ok      Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: 6e5d4660d960836a…)
  ok      Wrapper ist NICHT der unberuehrte Auslieferungsstand
  ok      Patch: Bruecken-Header eingebunden
  ok      Patch: IAudioPresentationLatency als Basisklasse
  ok      Patch: Interface auch im queryInterface
  ok      Patch: Buslatenz-Methode umgesetzt
  ok      Patch: Blockbeginn verdrahtet
  ok      Patch: Context-Anwesenheit verdrahtet
  ok      Patch: fehlender Context wird ausdruecklich gemeldet
  ok      Patch: alle Parameterpunkte werden beobachtet
  ok      Patch: Uebergabe an die Senke verdrahtet
  ok      Patch: Bruecke als Wrapper-Member
  ok      Patch: Senke wird EINMAL beim Erzeugen aufgeloest
  ok      Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert
  ok      ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)
  ok      Patchdatei ist ein echter Unified Diff
  ok      Patch zielt auf den VST3-Wrapper
== B · Context-Anwesenheit und Gueltigkeitsbits ==
  ok      Senke wird genau einmal je Block gerufen
  ok      fehlender Context: processContextPresent == false
  ok      fehlender Context: Projektzeit ist UNGUELTIG, nicht 0
  ok      fehlender Context: Tempo und PPQ ungueltig
  ok      genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)
  ok      genullter Context: Projektzeit gueltig und 0 â eine Aussage, keine Vermutung
  ok      genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)
  ok      genullter Context: Tempo bleibt ungueltig
  ok      genullter Context: playing ist gueltig false â nicht 'unbekannt'
  ok      voller Context: anwesend
  ok      voller Context: projectTimeSamples 123456
  ok      voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)
  ok      voller Context: playing true
  ok      voller Context: recording true
  ok      voller Context: Tempo 128
  ok      voller Context: PPQ 8.25
  ok      voller Context: Taktbeginn 8.0 PPQ
  ok      voller Context: Cycle 4.0â12.0 PPQ, aktiv (PPQ, nicht Samples)
  ok      Teil-Context: playing true
  ok      Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)
  ok      Teil-Context: Cycle nicht aktiv
  ok      beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)
== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==
  ok      drei Punkte angekommen
  ok      Punkte sind nach Sample-Offset sortiert
  ok      sauberer Block: sampleAccurateAutomation == true
  ok      Blockgroesse wird mitgefuehrt
  ok      kleinerer Offset zuerst, auch wenn spaeter geliefert
  ok      gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)
  ok      Punkte aus mehreren Queues sind global aufsteigend
  ok      Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte
  ok      Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt
  ok      Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ueberlauf des Ereignisrings verwirft KEINEN Letztwert
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      NaN nach Ueberlauf: Zusicherung faellt
  ok      NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Letztwert-Tabelle: genau kMaxLetztwerte Parameter werden gefuehrt
  ok      Letztwert-Tabelle: die 3 ueberzaehligen Parameter sind gezaehlt
  ok      Letztwert-Ueberlauf laesst die Zusicherung ebenfalls fallen
  ok      Letztwert-Ueberlauf: der ueberzaehlige Parameter meldet ehrlich 'nichts'
  ok      ohne Ueberlauf: Rueckfallwert deckt sich mit dem letzten Ringeintrag (0.30)
  ok      ohne Ueberlauf: der Ring traegt denselben Wert am Ende
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      der Buszaehler steht im Blockbefund, ist also fuer die Senke sichtbar
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (7880 Byte)

HOSTKONTEXT OK — 91/91 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3b"></a>
#### B3b | EqCopHostProbeTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==
  ok      meldet 0 Samples Latenz
  ok      meldet 0 s Tail
  ok      200 Bloecke float: Ausgang bitgleich zum Eingang
  ok      double-Block: Ausgang bitgleich zum Eingang
  ok      double-Praezision wird dem Host angeboten
  ok      5.1 wird klar abgelehnt, nicht still gemischt
== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==
  ok      dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch
  ok      vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT
  ok      nach dem ersten Bruecken-Block meldet es: Bruecke liefert
== C - Context-Anwesenheit und Gueltigkeiten ==
  ok      acht Bloecke mit Context gezaehlt
  ok      der eine Block OHNE Context ist getrennt gezaehlt
  ok      neun verarbeitete Bloecke
  ok      neun Senke-Aufrufe - keine Asymmetrie ohne Grund
  ok      der Kontextverlust ist als Ereignis vermerkt
  ok      Tempo war in jedem Context gueltig
  ok      continousTimeSamples war NIE gueltig - und wird nicht erfunden
== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==
  ok      fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)
  ok      Seek vorwaerts wird als Sprung erkannt
  ok      die Sprungweite wird mitgemessen
  ok      Seek rueckwaerts wird als Sprung erkannt
  ok      nach Stop/Play wird der Positionswechsel NICHT als laufender Sprung gemeldet
  ok      er verschwindet aber auch nicht: eigener Zaehler 'ueber Stop/Play' (T2-Befund 21.08.)
  ok      und ein eigenes Ereignis, damit eine 0 bei den Spruengen nicht mehrdeutig ist
  ok      Stop und Play sind als Transportereignisse vermerkt
  ok      Loop-Wechsel erscheint als Rueckwaertssprung
  ok      Schleife-an ist genau einmal vermerkt
  ok      der Schleifenzustand wird gefuehrt
  ok      Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)
== E - Samplegenaue Automation: der Kernbeweis von Termin B ==
  ok      ohne Automation wird nichts behauptet
  ok      ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben
  ok      alle Punkte werden aufsummiert
  ok      der dichteste Block wird gemerkt
  ok      Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit
  ok      kleinster und groesster Offset werden gefuehrt
  ok      der erste Mehrpunkt-Block ist als Ereignis vermerkt
  ok      Mehrpunkt-Block OHNE Zusicherung zaehlt NICHT als samplegenau
  ok      er wird stattdessen getrennt gezaehlt - kein Befund verschwindet
  ok      Unplausibilitaeten und Ueberlaeufe der Bruecke werden AUFSUMMIERT uebernommen, nicht geglaettet
  ok      beide Bloecke ohne Zusicherung sind gezaehlt (der mit Offset- und der mit Wertfehler)
== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==
  ok      ohne Hostmeldung: 'nie gemeldet' - und KEIN Eintrag wird erfunden
  ok      Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten
  ok      die Gegenrichtung bleibt ungemeldet - es wird kein Wert erfunden
  ok      Ausgang Bus 0 = 1024 wird NEBEN dem Eingang gefuehrt, nicht verschluckt
  ok      auch ein dritter Bus kommt an
  ok      ein nicht gemeldeter Bus dazwischen bleibt ungemeldet
  ok      der zuerst gemeldete Latenzwert rastet ein
  ok      die spaetere Aenderung wird gezaehlt statt still verworfen
  ok      50 weitere Bloecke mit DEMSELBEN geaenderten Wert zaehlen NICHT weiter (Uebergaenge, nicht Bloecke)
  ok      ein dritter, wieder anderer Wert zaehlt als zweiter Uebergang
  ok      der Bericht listet genau die drei gemeldeten Eintraege - keinen erfundenen
  ok      und der Wertwechsel-Zaehler steht im Bericht - mit der Einheit im Schluesselnamen
== G - Senke ohne processBlock (Flush/Hostriegel) ==
  ok      Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt
  ok      der Block ohne Verarbeitung ist ausdruecklich vermerkt
== H - Gegenpfad: Bericht schreiben und wieder einlesen ==
  ok      der Bericht ist nicht leer
  ok      der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)
  ok      Schemaname steht im Bericht
  ok      der Bericht traegt die Kernaussage 'samplegenau belegt'
  ok      die Blockzahl im Bericht deckt sich mit der Messung
  ok      das Ereignisprotokoll liegt im Bericht
  ok      die Gueltigkeitsmaske steht im Bericht
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260828-210335.json
  ok      die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung
== I - Zuruecksetzen und Ringueberlauf ==
  ok      Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung
  ok      nach dem Zuruecksetzen wird die laufende Aufnahme wieder als aufnahme_an gemeldet
  ok      mehr Ereignisse als Ringplaetze wurden erzeugt
  ok      die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still
  ok      der Ring gibt genau seine Kapazitaet heraus
  ok      die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)
== I2 - Offline-Uebergang und negative Projektzeit ==
  ok      Offline- und Echtzeitbloecke getrennt gezaehlt
  ok      der Wechsel IN den Render ist vermerkt
  ok      der Wechsel ZURUECK in Echtzeit ist ebenfalls vermerkt - nicht nur der erste Block ueberhaupt
  ok      negative Projektzeit wird GEZAEHLT, nicht still uebersprungen
  ok      der erste negative Wert ist als Ereignis vermerkt
  ok      im negativen Bereich wird kein Scheinsprung erfunden
== J - Audiothread: keine Allokation ==
  ok      500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen
== J2 - Nebenlaeufig lesen, waehrend der Audiothread schreibt ==
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (35098 Bloecke)
  ok      200 nebenlaeufige Lesevorgaenge liefern durchweg plausible Eintraege und Zaehler
  ok      der letzte Lesevorgang hat Eintraege geliefert
  hinweis   Grenze: Rauchtest, kein Beweis der Tearing-Freiheit (der Wiederholpfad wird selten bis nie betreten)
== K - Anzeige: passt der Inhalt ueberhaupt ins Fenster? ==
  ok      der Editor laesst sich erzeugen
  ok      Fensterhoehe 850 deckt den Leerzustand (850 noetig)
  ok      Fensterhoehe deckt auch den Messzustand (850 noetig)
  ok      keine Textzeile liegt unter den Knoepfen (Text bis 760, Knopfstreifen ab 794)
  ok      keine Wertzeile ohne Beschriftung - und 27 Zeilen wurden dabei wirklich geprueft
  ok      hoechstens EINE unbeschriftete Hinweiszeile (gefunden: 1)
  ok      und der Riegel faellt: eine untergeschobene Wertzeile ohne Beschriftung wird erkannt

HOSTPROBE OK - 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3c"></a>
#### B3c | EqCopSchemaTest

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,19 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [102 Faelle]
[ok]   Falltabelle hat Substanz  [102 Faelle]
[ok]   Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (62 Fixtures)
[ok]   Binaerkorpus hat Substanz  [62 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-auf-2hoch53.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740992.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-nahe-2hoch53-ab.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740991.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-unter-2hoch53-auf.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 4503599627370495.9]
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-kleiner-bruch-rundet-auf-eins.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 1.00000000000000001]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nan-token.json  [unbekanntes Literal NaN an Position 1090]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (188 Fixtures)
[ok]   Korpus hat Substanz  [188 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
63 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 3,91 s

stdout:

```text
== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==
Produktionsstrom GenStrom: 1 Tap(s), 131072 Frames Ring, 2048 Deskriptoren, max 16384 Frames je Block.

== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==
  ok      drei Bloecke, drei Deskriptoren  [3]
  ok      jeder Frame kommt bitgleich und an derselben Stelle heraus
  ok      Stempel: lokaler Strom und Projektzeit stimmen je Block
  ok      nach dem Leeren ist die Queue leer
  ok      kein Verlust bei Normallast
== B - Wrap: der Ring laeuft mehrfach um ==
  ok      der Ring ist wirklich mehrfach umgelaufen  [11 Umlaeufe]
  ok      ueber jeden Wrap hinweg bitgleicher Inhalt
  ok      kein Frame verloren, keiner doppelt  [12000/12000]
== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==
  ok      vier Bloecke passen, danach ist Schluss  [4]
  ok      beide abgewiesenen Bloecke sind gezaehlt  [2]
  ok      verlorene Frames = 2 x 256 (keine Teilmenge)  [512]
  ok      ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock
  ok      der erste Block nach dem Verlust traegt kFlagLueckeDavor
  ok      und beginnt ein neues continuity_segment  [1]
== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==
  ok      genau `deskriptorPlaetze` Bloecke passen  [8]
  ok      die vier ueberzaehligen sind gezaehlt  [4]
  ok      und genau so viele stehen zum Lesen bereit  [8]
== E - Oversize: ein Block ueber der Slotkapazitaet ==
  ok      ein Block ueber maxBlockFrames wird als GANZES abgewiesen
  ok      und als Oversize gezaehlt, nicht als Ueberlauf  [1/0]
  ok      der Ueberlaufzaehler bleibt sauber
  ok      nichts Halbes im Ring
  ok      die verlorene Zeit ist vollstaendig gemeldet  [257]
  ok      genau maxBlockFrames passt noch (Kante, nicht Ausschluss)
== F - Flush (numSamples == 0) ist kein Verlust ==
  ok      ein Flush-Block wird nicht eingestellt
  ok      und zaehlt NICHT als Verlust - er traegt keine Audiozeit
  ok      der Folgeblock geht normal
  ok      und traegt keine Luecke
== G - Mono und Mehrfach-Tap ==
  ok      Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)
  ok      und der Deskriptor sagt kanaele == 1
  ok      Drei-Tap-Layout: Block angenommen
  ok      das Bitset meldet genau die belegten Taps  [3]
  ok      jeder Tap liegt getrennt und richtig; der inaktive liefert false
  ok      eine falsche Tapzahl wird abgewiesen statt halb kopiert
  ok      ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor
== H - Ein-Block-Quarantaene ==
  ok      5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene  [4]
  ok      und die Quarantaene haelt ihn wirklich
  ok      kein Bruch bei lueckenlosem Strom
  ok      der Seek ist als Bruch erkannt  [1]
  ok      genau EIN Block faellt an der Grenze - der Strom laeuft weiter
  ok      der Block NACH der Grenze ist schon wieder in Quarantaene
  ok      Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)
  ok      stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      Vorlauf ohne Bruch
  ok      Transportkante play->stop ist ein Bruch  [1]
  ok      ohne Hostkontext traegt der lokale monotone Strom (§53.6)
  ok      und versiegelt normal  [3]
  ok      Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung
  ok      negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)
== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==
  20 000 Bloecke, Groessen 1..288: angenommen 17809, Ueberlauf-Drops 0, Oversize-Drops 2191, groesster Block 256, versiegelt 15872, Brueche 1936
  ok      jeder gelesene Frame steht an der Stelle, die sein Stempel nennt
  ok      es gab nie einen Block mit sampleCount 0
  ok      die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt  [2191]
  ok      kein angenommener Block war je groesser als die Slotkapazitaet  [256]
  ok      es wurde wirklich gelesen  [2288127 Frames]
== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==
  ok      18 Blockgroessen von 1 bis 16 384: kein Sample veraendert
  ok      0 Samples Latenz  [0]
  ok      kein Tail
  angenommen 18, groesster Block 16384, Ueberlauf 0, Oversize 0
== K - verdrahtet: Oversize aendert das Audio nicht ==
  ok      ein Block ueber der Slotkapazitaet laeuft unveraendert durch
  ok      und ist als Oversize-Drop der ANALYSE gezaehlt  [1]
  ok      der abgewiesene Oversize-Block erscheint nicht im Projektfenster
  ok      der naechste angenommene Lueckenblock beginnt das Fenster exakt neu  [16385..16449]
  ok      der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben  [0]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3848 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
== M - verdrahtet: die Messung bekommt den Strom wirklich ==
  eingespeist 287744, analysiert 287232, Drops 0, versiegelt 561, Quarantaene-Verwuerfe 0, Brueche 0
  ok      bei Normallast kein einziger Drop
  ok      und kein Kontinuitaetsbruch (lueckenloser Transport)
  ok      die Engine sieht den ganzen Strom bis auf den Block in Quarantaene  [512 Frames offen]
  ok      Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)  [-8.684240]
  ok      framesDropped im Stats-Snapshot kommt aus derselben Quelle
== N - Gegenpfad: prepareToPlay mitten im Betrieb ==
  ok      auch der erste Block nach prepareToPlay alloziert nicht  [0]
  veraltete Bloecke (alter Anlauf, nicht analysiert): 40
  ok      ein Neuanlauf ist kein Oversize
== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==
  ok      der Anlauf steigt sofort, nicht erst beim naechsten Audioblock  [0 -> 1]
  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt 3, als aktuell durchgelassen 0
  ok      alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar  [3/3]
  ok      der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt
  ok      Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell
  ok      und er traegt die Luecke, die der Neuanlauf gerissen hat
  ok      der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut
  ok      zwei Neuanlaeufe hintereinander gehen beide nicht verloren  [1 -> 3]
  ok      zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration
  ok      und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste
  ok      Nullframe konsumiert die wartende Generation nicht
  ok      der naechste echte Block uebernimmt sie weiterhin
== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==
  ok      Seek um 10 s bei gestopptem Transport ist EIN Bruch  [1]
  ok      Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)  [0]
  ok      und bei laufendem Transport ist der Seek weiterhin ein Bruch  [1]
  ok      bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet  [0]
== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==
  ok      nach Start ist nur die leichte Publikation sofort faellig
  ok      10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus
  ok      vor 250 ms bleibt Gating/Kandidaten gesperrt
  ok      bei 250 ms ist genau die schwere Runde faellig
  ok      dieselbe Deadline kann nicht zweimal verbraucht werden
  ok      nach Pause genau eine Runde, kein Catch-up-Sturm
  ok      Reset/Generation startet die 250-ms-Deadline neu
== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==
  ok      Test hat echten Queue-Rueckstau hergestellt  [257 Bloecke bis zum ersten Drop]
  ok      unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms  [1 Runden in 600 ms]
  ok      neue Generation erbt keinen Batch-beschleunigten Schwertakt  [0 Runden in den ersten 100 ms]
  ok      Frame-Leser wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Reset-Steuerer wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Stop/Join bleibt auch mit Queue-Rest begrenzt  [0 ms]
== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==
  ok      Gegenprobe: erste Generation hat ihr eigenes Projektfenster
  ok      Testbarriere liegt deterministisch im laufenden Audiocallback
  ok      der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster  [10000..10064]
  ok      die Generationskante ist kein Seek innerhalb des alten Fensters  [0]
== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==
  ok      Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine
  ok      nichtendliche Hostrate wird fail-closed auf 0 gespiegelt
  ok      M1 verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      FeatureEngine verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung
  ok      eine folgende gueltige Generation aktiviert beide Engines wieder

114 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Feingitter: 10001 Bins a 0.01 LU ueber [-70.00, 30.01) LUFS.
Oberband:   3070 Bins a 1.00 LU ueber [30.01, 3100.01) LUFS - lautheit(DBL_MAX) = 3081,86.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: kein Block faellt durch das Raster  [0]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: kein Block faellt durch das Raster  [0]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): kein Block faellt durch das Raster  [0]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: kein Block faellt durch das Raster  [0]
  ok      Musik + echte Stille: LUFS-I innerhalb ±0,1 LU  [akku=-8.281644 ref=-8.281644 d=0.000000000]
  ok      Musik + echte Stille: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Musik + echte Stille: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000027]
  ok      Musik + echte Stille: Kurz-LUFS Gueltigkeit gleich
  ok      Musik + echte Stille: Kurz-LUFS BITGLEICH  [akku=-12.451912590513381 ref=-12.451912590513381]
== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==
  Sweep: 201 Laeufe, groesste Abweichung 2.781360 LU bei Faktor 1.000000, groesste gemeldete Schranke 2.781412 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen (Eingang aendert sich um 1e-5): 2.781403 LU
  ok      Sweep vollstaendig gefahren  [201/201]
  ok      Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)  [sonst pruefte D nichts; schranke=2.781412]
  ok      die selbstgemeldete Schranke deckt JEDEN Lauf  [201/201]
  ok      wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU  [76/76]
  ok      die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)  [maxD=2.781360 refSprung=2.781403]
  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)
== E - fixed memory: eine Million Zellen ==
  Akku:     0 Allokationen, LUFS-I -8.386510, Schranke 0.000000000 LU
  Referenz: 36 Allokationen, LUFS-I -8.386510
  ok      1 000 000 Zellen: 0 Allokationen im Akku  [0]
  ok      Gegenprobe: die alte Rechnung alloziert sehr wohl  [36]
  ok      1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU  [d=0.000000000]
  Fester Bedarf laut speicherBytes(): 157122 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      Kurz-LUFS meldet einen nichtendlichen Ring nie als gueltig
  ok      ueber dem Gitter: EIN Pegel bleibt wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      Gegenprobe: der Korpus liegt wirklich im Oberband  [197]
  ok      und kein Block faellt durch das Raster
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer
== G - Oberband: zwei Pegel ueber dem Feingitter ==
  G1: akku=49.302611 ref=49.302611 schranke=0.000000000 ueberGitter=1997
  ok      G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)  [1997]
  ok      zwei Pegel ueber dem Gitter (T2-1): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      zwei Pegel ueber dem Gitter (T2-1): kein Block faellt durch das Raster  [0]
  ok      zwei Pegel ueber dem Gitter (T2-1): LUFS-I innerhalb ±0,1 LU  [akku=49.302611 ref=49.302611 d=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS Gueltigkeit gleich
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS BITGLEICH  [akku=32.319299956639810 ref=32.319299956639810]
  ok      Naht, Gamma_r im Oberband: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Oberband: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Oberband: LUFS-I innerhalb ±0,1 LU  [akku=49.302481 ref=49.302481 d=0.000000000]
  ok      Naht, Gamma_r im Oberband: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Oberband: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  ok      Naht, Gamma_r im Feingitter: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Feingitter: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Feingitter: LUFS-I innerhalb ±0,1 LU  [akku=49.243363 ref=49.243363 d=0.000000000]
  ok      Naht, Gamma_r im Feingitter: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Feingitter: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  G3-Sweep: 201 Laeufe, groesste Abweichung 2.781577 LU bei Faktor 1.001000, groesste gemeldete Schranke 2.781577 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: 2.781401 LU
  ok      G3: Sweep vollstaendig gefahren  [201/201]
  ok      G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)  [1997]
  ok      G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)  [sonst pruefte G3 nichts; schranke=2.781577]
  ok      G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband  [201/201]
  ok      G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz  [maxD=2.781577 refSprung=2.781401]
  ok      G3: kein Block faellt durch das Raster  [0]
  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - 1000 Bloecke kippen gemeinsam die Gateseite.)
  G4: LUFS-I 3038.851, Schranke 0.000000000, ueberGitter 197, ueberOberband 0
  ok      G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert  [3038.851]
  ok      G4: nichts wird faelschlich als nicht-endlich gezaehlt  [0]
  ok      G4: auch am aeussersten Rand faellt kein Block durch das Raster  [0]
  ok      G4: und die Schranke bleibt endlich  [0.000000000]

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
109 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b5"></a>
#### B5 | EqCopAnalysisGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 2,03 s

stdout:

```text
== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==
Gate: "Drop/Seek/Loop trennt jedes offene Fenster."
Stufen: Bass 16384 (Hop 8192), Haupt 4096 (Hop 2048), Trennung bei 200 Hz.

== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==
[ok]   Evidenzgitter: band_anzahl == kEvidenzBaender  [221]
[ok]   Livegitter: band_anzahl == kLiveBaender  [64]
[ok]   Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)  [0 Abweichungen]
[ok]   Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)  [0 Abweichungen]
[ok]   Live-Kanten BITGLEICH zur Fixture (65 Werte)  [0 Abweichungen]
[ok]   Live-Mitten BITGLEICH zur Fixture (64 Werte)  [0 Abweichungen]
[ok]   Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal  [0 Abweichungen, Ende bei 221]
[ok]   Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank  [0 Abweichungen]

== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==
[ok]   q_db_0p1_i16: Skalierung stimmt mit der Fixture ueberein  [10]
[ok]   q_db_0p1_i16: alle 23 Vektoren treffen
[ok]   q_db_0p01_i16: Skalierung stimmt mit der Fixture ueberein  [100]
[ok]   q_db_0p01_i16: alle 23 Vektoren treffen
[ok]   float32: alle 15 Vektoren treffen bitgenau
[ok]   Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0
[ok]   halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl
[ok]   und negativ genauso (-0,05 -> -1)

== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==
[ok]   Band i sitzt in Byte i/8, Bit i%8 (LSB-first)  [Byte0=81 Byte1=1]
[ok]   ein Bit loeschen laesst die Nachbarn stehen
[ok]   Lesen und Schreiben sind dieselbe Adressierung
[ok]   221 Baender brauchen 28 Bitmapbytes  [28]
[ok]   die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)  [letztes Byte = 1f]

== D - FFT: gegen analytisch bekannte Ergebnisse ==
[ok]   Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin  [100.0000 %]
[ok]   Parseval haelt (Zeit- und Frequenzenergie gleich)  [rel. Fehler 0.0000000000000002]
[ok]   Gleichanteil sitzt vollstaendig in Bin 0  [Bin0 = 1048576.0, Rest = 0.00000000000000000000]

== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==
[ok]   RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander  [max 0.04312 dB bei 20000.0 Hz]
       (gemessene Groesstabweichung, zur Ablesung: 0.043120 dB)
[ok]   Hochpass sperrt den Gleichanteil vollstaendig  [|H(0)|Â² = 0.000000000000000000000000]

== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==
[ok]   die Sequenz zaehlt ab 1 aufwaerts  [6]
[ok]   mit gueltiger Projektzeit ist die Zeitbasis project_samples
[ok]   und project_sample_start ist gesetzt
[ok]   Gueltigkeitsbit project_time steht
[ok]   der Frame traegt seine Metrikversion  [20260823]
[ok]   und die Samplerate
[ok]   Frame-Stempel spannt alle integrierten Hostbloecke auf  [0 + 5120]
[ok]   Frame-Stempel umfasst den ueberlappenden FFT-Support  [2048 + 8192]
[ok]   ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck
[ok]   Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen
[ok]   fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden
[ok]   Evidenzframe vereinigt Haupt- und aelteren Basssupport  [0 + 30720, 196 Baender]
[ok]   stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen
[ok]   stehende FL-Teilstuecke erfinden kein Projektintervall
[ok]   ueberlaufende Host-Projektspanne wird konservativ lokal publiziert
[ok]   Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block  [104608 + 512]
[ok]   ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (Â§32.3)
[ok]   und dann steht WEDER das Bit NOCH ein Startwert da
[ok]   alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet  [0x7f]
[ok]   eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (Â§32.3)
[ok]   Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer
[ok]   continuous_time_samples kommt mit demselben Frameanker durch
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: NaN
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Inf
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Ende vor Start
[ok]   ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich

== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==
[ok]   G1 Drop (kFlagLueckeDavor): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G1 Drop (kFlagLueckeDavor): Ursache ist lokaleLuecke  [Zaehler 1]
[ok]   G1 Drop (kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche (§32.3)  [Epochen 0 Segmente 1]
[ok]   G2 Seek waehrend Wiedergabe: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G2 Seek waehrend Wiedergabe: Ursache ist zeitSprung  [Zaehler 1]
[ok]   G2 Seek waehrend Wiedergabe: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G3: bei gestopptem Transport laufen die Fenster normal weiter  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)  [0 Epochenwechsel]
[ok]   G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G3: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G3: und sie heisst zeitSprung, nicht lokaleLuecke
[ok]   G4: aktive Schleife allein trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4 Loop-Wrap: jedes Fenster getrennt  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G4: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G4: und die Ursache heisst loopWrap, nicht zeitSprung
[ok]   G5: Schleife ohne erreichbare Grenze trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, weil der Straddle-Block auch kein neues beginnt  [Bass=0 Haupt=0 Zelle=0 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G5: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet
[ok]   G5: mit eigener Ursache moeglicherStraddle
[ok]   G6 Transportkante (Play -> Stop): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G6 Transportkante (Play -> Stop): Ursache ist transportKante  [Zaehler 1]
[ok]   G6 Transportkante (Play -> Stop): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G7 Sampleratewechsel: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G7 Sampleratewechsel: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G7 Sampleratewechsel: Ursache ist sampleratewechsel  [Zaehler 1]
[ok]   G7 Sampleratewechsel: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G8 Neuanlauf (prepareToPlay): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G8 Neuanlauf (prepareToPlay): Ursache ist neuanlauf  [Zaehler 1]
[ok]   G8 Neuanlauf (prepareToPlay): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): Ursache ist beweislageWechsel  [Zaehler 1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G11: im Betrieb traegt die K-Kette einen Filterzustand  [1.392229]
[ok]   G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame  [290 / 290]
[ok]   G11: beide Laeufe liefern eine Kurzzeitlautheit
[ok]   G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - der Filternachklang ueberbrueckt nicht  [A=-60.034744263 B=-60.034744263]
[ok]   G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)  [0 Trennungen]
[ok]   G10: die Fenster laufen dabei weiter  [Bass=9224 Haupt=3080 Zelle=264 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G12: vor der Grenze tragen BEIDE Bandakkus den Ton  [Live 149 / Evidenz 149 Baender]
[ok]   G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber die Grenze weiter (Entscheid Â§10.1)  [liveSamples=0]
[ok]   G12: und der Frame traegt die NEUE Epoche  [1]
[ok]   G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird also wirklich geprueft
[ok]   G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)  [0 gesetzte Baender]
[ok]   G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter  [0 gesetzte Baender]
[ok]   G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit  [nicht gesetzt]
[ok]   G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden  [1192 Band-Fensterbeitraege]
[ok]   G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren  [120 von 120]
[ok]   G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor (Pruefer-Messung: 80 / 40 / 43)  [0 mit Live-Baendern, 0 mit Evidenz, 0 mit Aktivitaet]

== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) ==
[ok]   G13.0: zwei frische Frames sind gleich - der Vergleich sagt nicht pauschal ungleich
[ok]   G13.0: ein einziger geaenderter Skalar macht sie ungleich
[ok]   G13.0: ein einziger geaenderter Bandwert ebenso
[ok]   G13.0: und ein einziges Bitmapbit ebenso
[ok]   G13.0: und ein Feld im verschachtelten Transportstempel - der Vergleich reicht bis dorthin
[ok]   G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen vorgefuehrt
[ok]   G13a lokaleLuecke [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13a lokaleLuecke [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13a lokaleLuecke [Zwilling]: beide haben die Grenze lokaleLuecke gezogen  [A 1 / B 1]
[ok]   G13a lokaleLuecke [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13a lokaleLuecke [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13a lokaleLuecke [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13a lokaleLuecke [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13a lokaleLuecke [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13b zeitSprung [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13b zeitSprung [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13b zeitSprung [Zwilling]: beide haben die Grenze zeitSprung gezogen  [A 1 / B 1]
[ok]   G13b zeitSprung [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13b zeitSprung [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13b zeitSprung [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13b zeitSprung [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13b zeitSprung [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13c transportKante [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13c transportKante [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13c transportKante [Zwilling]: beide haben die Grenze transportKante gezogen  [A 1 / B 1]
[ok]   G13c transportKante [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13c transportKante [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13c transportKante [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13c transportKante [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13c transportKante [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13d sampleratewechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13d sampleratewechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13d sampleratewechsel [Zwilling]: beide haben die Grenze sampleratewechsel gezogen  [A 1 / B 1]
[ok]   G13d sampleratewechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13d sampleratewechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13d sampleratewechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13d sampleratewechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13d sampleratewechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13e neuanlauf [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13e neuanlauf [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13e neuanlauf [Zwilling]: beide haben die Grenze neuanlauf gezogen  [A 1 / B 1]
[ok]   G13e neuanlauf [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13e neuanlauf [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13e neuanlauf [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13e neuanlauf [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13e neuanlauf [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13f beweislageWechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13f beweislageWechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13f beweislageWechsel [Zwilling]: beide haben die Grenze beweislageWechsel gezogen  [A 1 / B 1]
[ok]   G13f beweislageWechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13f beweislageWechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13f beweislageWechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13f beweislageWechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13f beweislageWechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]

== H - NAK-29: bedingte Feldpflichten des Transportstempels ==
[ok]   ein sauberer Stempel kommt durch
[ok]   Fall 1: project_samples + Bit, aber KEIN project_sample_start
[ok]   Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu Â§32.3)
[ok]   Fall 3: bounds_valid ohne start_ppq/end_ppq
[ok]   Fall 4: derivation=validated_block_mapping ohne bounds_valid
[ok]   Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht
[ok]   Fall 6: continuous_time-Bit ohne continuous_time_samples
[ok]   Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus
[ok]   Fall 7: sample_count bleibt unter der Vertragsobergrenze
[ok]   Fall 7: der Feature-Erzeuger publiziert keinen Leerframe
[ok]   der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel entsteht im normalen, gedeckelten Betrieb  [0]
[ok]   abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich  [1]

== I - Ereignisse: kein Fluss ueber eine Grenze ==
[ok]   der spektrale Fluss hat im Betrieb einen Vorgaenger
[ok]   nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss einen Onset, den es nie gab
[ok]   jedes Ereignis traegt die Epoche, in der es gesehen wurde  [1 Ereignis(se)]
[ok]   der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, nicht behauptet  [64 im Ring, 1 verworfen, nach 1107 Bloecken]
[ok]   und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (Â§48.1: ein Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)  [64 / 64]
[ok]   drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste  [aeltestes vorher 38912, jetzt 47104]
[ok]   und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf
[ok]   Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock
[ok]   stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit  [1 Ereignis(se) nach 84 Bloecken]

== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==
[ok]   LUFS-S ist nach 4 s gesetzt
[ok]   LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (Â§39.3)  [gemessen -6.0620 erwartet -6.0620 d=0.00000]
[ok]   Peak trifft die Amplitude  [-6.021 dBFS]
[ok]   Crest eines Sinus ist 3,01 dB  [3.0091 dB]
[ok]   L==R ergibt Korrelation 1  [1.000000]
[ok]   und Breite 0 (kein Seitenanteil)  [0.000000000000]
[ok]   das Live-Band um 997 Hz ist gueltig  [Band 35]
[ok]   und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal

== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==
[ok]   die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen  [33 Bloecke]
[ok]   und mindestens einen Frame gebaut  [3 Frames]
[ok]   process_context_present kommt aus der BRUECKE durch (Â§32.3)
[ok]   recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen
[ok]   continuous_time_samples kommt durch
[ok]   die Schleifengrenzen kommen durch, samt `active`
[ok]   beide Presentation-Latencies kommen durch  [3924 / 4410]
[ok]   ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits  [0x7f]
[ok]   Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits  [0x3]
[ok]   insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`
[ok]   Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen
[ok]   Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt
[ok]   nichtendliche Sampleraten initialisieren keine Analyseengine

== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==
[ok]   L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt
[ok]   L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen  [0]
[ok]   L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, und zwar nur dort  [1 Abweichung(en), Zeile 26]
[ok]   L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'
[ok]   L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts  [0 gesetzte Baender]
[ok]   L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte  [3]
[ok]   L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung  [1, 3]
[ok]   L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'
[ok]   L6: ein echter Grund wird gezaehlt  [1]
[ok]   L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen  [gelesen: 0]
[ok]   L6: und `keine` ebenso - getrennt wird nie ohne Grund

== M - Auskunft ueber die Grenzen der Messung ==
[ok]   die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da  [erstes messbares Band 2 (32.6 Hz)]
[ok]   bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz  [erstes Band ueber der Kappe 221]
[ok]   bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab  [erstes Band ueber der Kappe 202 (10525.6 Hz)]
[ok]   und sie sitzt genau am ersten Band, dessen Oberkante 0,95Â·Nyquist ueberschreitet  [10374.7 .. 10678.7 Hz gegen 10473.8 Hz]
[ok]   kein Live-Band vollstaendig ueber der Kappe traegt einen Wert  [0 Baender]
[ok]   Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler

Ergebnis: 235 bestanden, 0 Fehler.
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  ok      frisch: unclassified  [unclassified]
  ok      frisch: neutral
  ok      frisch: keine Audio-Ausnahme (audio-neutral)
  ok      frisch: kein Brokerstart
  ok      Schema-1 'sensor' -> legacy  [legacy]
  ok      Schema-1 'sensor': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'pre' -> legacy  [legacy]
  ok      Schema-1 'pre': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'post' -> legacy  [legacy]
  ok      Schema-1 'post': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'hub' -> main  [main]
  ok      Schema-1 'hub': Audio-Ausnahme moeglich
  ok      Schema-2 main (Kind-Matrix bestaetigt) -> main
  ok      Schema-2 legacy -> legacy
  ok      Vorbereitung: erst main
  ok      unbekanntes Major danach -> zurueck auf unclassified
  ok      Muell (ignoriert) aendert nichts - main bleibt main
  ok      Muell auf einen frischen Automaten klassifiziert nicht
  ok      explizite Initialisierung OHNE Editor: abgelehnt
  ok      ... und der Automat bleibt neutral
  ok      der geoeffnete Editor allein klassifiziert NICHT
  ok      Editor offen + explizite Initialisierung: angenommen
  ok      ... -> main
  ok      main + offener Editor darf den Broker starten
  ok      Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)
  ok      main OHNE offenen Editor darf NICHT starten
  ok      Rueckweg: derselbe Akt fuehrt von main zurueck
  ok      ... -> legacy
  ok      legacy darf nie starten - auch mit Editor nicht
  ok      legacy ist wieder audio-neutral
  ok      Sonde (NkPr) frisch: neutral trotz fester Produktklasse
  ok      Sonde (NkPr): gueltiger eigener Stand -> passive_probe
  ok      Sonde (NkPr): kein Brokerstart
  ok      Sonde (NkPr): auch mit offenem Editor kein Brokerstart
  ok      Sonde (NkPr): kein Akt macht sie zum Main
  ok      Sonde (NkAc): gueltiger eigener Stand -> active_probe
  ok      Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main

[2] Verdrahtet: EqCopilotProcessor (Gen)
  ok      Scannerlauf: klassifiziert NICHT (53.5)  [unclassified]
  ok      Scannerlauf: kein Brokerstart
  ok      Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)
  ok      Scannerlauf: Markierung meldet sich nicht hoerbar
  ok      Altprojekt 'sensor' -> legacy  [legacy]
  ok      legacy mit offenem Editor: kein Brokerstart
  ok      legacy ist 'immer passiv' - kein Sample gefaerbt
  ok      Altprojekt 'hub' -> main  [main]
  ok      main, aber Editor zu: noch kein Brokerstart
  ok      main + offener Editor: Brokerstart erlaubt
  ok      main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)
  ok      Vorbereitung: Rollenwahl aendert den Stand
  ok      Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)  [unclassified]
  ok      der geoeffnete Editor allein holt es auch nicht nach
  ok      Vorbereitung: der User waehlt die Rolle im offenen Editor
  ok      frische Instanz: Editor offen + Rollenwahl 'hub' -> main
  ok      frische Instanz als main: faerbt
  ok      Rueckweg: 'sensor' klassifiziert zurueck auf legacy
  ok      Rueckweg: der Schnitt auf neutral greift sofort
  ok      Vorbereitung: main
  ok      der fremde Stand kommt read-only zurueck
  ok      read-only -> zurueck auf unclassified
  ok      read-only: kein Brokerstart
  ok      read-only: wieder audio-neutral

LEBENSLAUF-TEST OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 31,99 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopPipeClientTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_rc_lib.dir\Release\EqCopPipeClientTest_rc_lib.lib
  EqCopPipeClientTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSunaNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_rc_lib.dir\Release\EqCopSunaNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopSunaNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostProbeTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_rc_lib.dir\Release\EqCopHostProbeTest_rc_lib.lib
  EqCopHostProbeTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSchemaTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_rc_lib.dir\Release\EqCopSchemaTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopAnalysisGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_rc_lib.dir\Release\EqCopAnalysisGoldenTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopAnalysisGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaSuna_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_rc_lib.dir\Release\NakamaSuna_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaSuna.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\Nakama Suna_SharedCode.lib
  NakamaSuna_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaSuna_vst3_helper.exe
  NakamaSuna_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\x86_64-win\Nakama Suna.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaSuna_artefacts/Release/VST3/Nakama Suna.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>


---

## Kanon-Lauf - S8 Dirigent 2026-08-28 - Kanon auf 374eea7 nach Nacharbeit

**Lauf:** 2026-08-28 23:09 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 29/29 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-28 23:09:32 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 374eea7 S8 Nacharbeit T3: Riegel K1/K2/K2b/K2c und A14 messen, was sie zusagen |
| Commit (voll) | 374eea7c8d7cef48fa5b83554dda2ad6d8d4ffb4 |
| Arbeitsbaum | 1 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M docs/PLAN-STAND.md
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-28 22:50:07 | `5AF83DC8A66EC081` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-27 03:22:19 | `C89836581A6FFD48` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-28 22:50:11 | `620AD1FFB7ABB49D` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-27 02:25:05 | `ECA970E1DE7BA8CC` | frisch (Bau bestaetigt) |
| `EqCopSunaNullTest` | 2026-08-28 22:50:15 | `F3F9879805698617` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-28 22:50:17 | `3FC3E48E58A9CA95` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-28 22:50:19 | `EA1E7E62C46B71C2` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-28 22:50:21 | `F6FE9313DBED1CB4` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-28 22:50:26 | `53D6108D902A9ADF` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-28 22:50:28 | `ABBB29D7F552D8C9` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-27 01:11:05 | `F56913243873A4FD` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-28 22:50:32 | `75C84EA6AC1BAAAD` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-28 22:50:34 | `5D0C5CE7D2E3A2AF` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-28 22:47:31**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 10,30 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 18,87 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,95 s | [↓ A4](#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,07 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,49 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,18 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,25 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,85 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,26 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,21 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,42 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,82 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 26,08 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [↓ A19](#a19) |
| A15 | Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | [OK] Exit 0 | 0,04 s | [↓ A15](#a15) |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,05 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,31 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,19 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,62 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,15 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,21 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

stdout:

```text
  ok      Latenz ist 0 Samples
  ok      Tail ist 0 s
  ok      1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)
  ok      kein falscher NaN-Alarm bei endlichem Signal
  ok      NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)
  ok      NaN/Inf wurde als Mangel registriert
  ok      Mono-Layout wird angenommen
  ok      5.1 wird klar abgelehnt (kein stilles Downmixen)
  ok      Save/Load: Sensor-ID bleibt stabil
  ok      Save/Load: Rolle und Paar-ID bleiben
NULLTEST OK
```

stderr:

_(leer)_

<a id="a2"></a>
#### A2 | EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 10,30 s

stdout:

```text
sinus-1k-minus12     32 ok, 0 Fehler
pink-minus20         39 ok, 0 Fehler
resonanz-116hz       41 ok, 0 Fehler
stille-mit-tick       7 ok, 0 Fehler
diag-scoop-mitte     10 ok, 0 Fehler
diag-mulm            12 ok, 0 Fehler
diag-haerte           8 ok, 0 Fehler
diag-mulm-halb       10 ok, 0 Fehler
diag-wander-ton      25 ok, 0 Fehler
stereo-antiphase     10 ok, 0 Fehler
stereo-nur-links      8 ok, 0 Fehler
stereo-nur-rechts     5 ok, 0 Fehler
stereo-dekorr         7 ok, 0 Fehler
nan-recovery          9 ok, 0 Fehler
nyquist-32k           4 ok, 0 Fehler
leicht-publikation    8 ok, 0 Fehler
konvergenz            4 ok, 0 Fehler

GOLDEN OK — 239 Pruefungen ok, 0 Fehler (metrics m4.1-2026-08-15 · diagnose m4.1-2026-08-15)
```

stderr:

_(leer)_

<a id="a3"></a>
#### A3 | EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 18,87 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: als Main klassifiziert (§53.5)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: als Main klassifiziert (§53.5)
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: als Main klassifiziert (§53.5)
  ok      T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: als Main klassifiziert (§53.5)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: als Main klassifiziert (§53.5)
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: beide Instanzen als Main klassifiziert (§53.5)
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
  ok      T11: als Main klassifiziert (§53.5)
  ok      T11: Auftrag gebaut
  ok      T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)
  ok      T11: und die Markierung meldet sich nicht hoerbar
  ok      T11: mit gestopptem Transport ebenfalls kein Sample
  ok      T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,95 s

stdout:

```text

running 90 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test dto::tests::as_f64_liefert_fuer_jede_number_einen_wert ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test dto::tests::bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege ... ok
test aggregat::tests::schreiben_erzeugt_haltbare_datei ... ok
test dto::tests::doppelter_schluessel_wird_nicht_still_ueberschrieben ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test dto::tests::geschachtelter_doppelter_schluessel_faellt_ebenfalls ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test bindung::tests::fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben ... ok
test aggregat::tests::alte_temp_restdatei_blockiert_snapshot_nicht ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test aggregat::tests::gleicher_zeitstempel_ueberschreibt_keinen_snapshot ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false ... ok
test protokoll::tests::gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow ... ok
test protokoll::tests::gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt ... ok
test protokoll::tests::heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::hello_enum_und_audiofelder_werden_semantisch_begrenzt ... ok
test protokoll::tests::hello_textfelder_haben_codepunktgenaue_harte_grenzen ... ok
test bindung::tests::wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand ... ok
test protokoll::tests::messstand_guard_prueft_minima_endlichkeit_und_textcap ... ok
test protokoll::tests::option_felder_akzeptieren_explizites_null_wie_das_schema ... ok
test protokoll::tests::unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset ... ok
test bindung::tests::parallele_schreiber_hinterlassen_genau_einen_intakten_stand ... ok
test register_tests::owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id ... ok
test register_tests::stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge ... ok
test aggregat::tests::parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test register_tests::unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::fremdes_session_token_beendet_established_statt_acklos_zu_haengen ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::ungelesenes_welcome_blockiert_connection_ende_nicht ... ok
test server::tests::vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test vertrag::tests::textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python ... ok
test server::tests::sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung ... ok
test server::tests::sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten ... ok
test server::tests::v1_heartbeat_mit_measurement_ist_protokollbruch ... ok
test server::tests::zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen ... ok
test server::tests::zweites_hello_kann_die_established_bindung_nicht_umschreiben ... ok
test server::tests::bye_braucht_exakt_das_ausgegebene_session_token ... ok
test server::tests::ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz ... ok
test server::tests::connection_und_pipe_instanzzahl_bleiben_hart_begrenzt ... ok
test server::tests::ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist ... ok
test server::tests::acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup ... ok

test result: ok. 90 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.48s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 8 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test dto_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 8 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.13s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a4b"></a>
#### A4b | EqCopPipeClientTest

**Befehl:** `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | **Exitcode:** 0 | **Dauer:** 1,07 s

stdout:

```text
PIPECLIENT-LIFECYCLE-TEST
  ok      Fake-Pipe fuer Welcome-Stopp angelegt
  ok      stop() bricht blockiertes Welcome-Lesen begrenzt ab  [0 ms]
  ok      SQOS begrenzt den Peer auf SecurityIdentification  [1]
  ok      Fake-Pipe fuer ACK-Stopp angelegt
  ok      stop() bricht blockiertes ACK-Lesen begrenzt ab  [0 ms]
  ok      Reconnect-Fake-Pipe angelegt
  ok      Reconnect waehrend Welcome verliert keine Konfigurationsgeneration
  ok      ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt
  ok      Welcome mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ACK mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers  [321 ms]
  ok      u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze
  ok      200 konkurrierende start/stop-Paare ohne joinable-Leak
PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler
```

stderr:

_(leer)_

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 1,49 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
52 Definitionen, 17 Nachrichtenfamilien, 188 Fixtures
Pruefungen: 284 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 49  
  const                     8 / 8    HART
  enum                     10 / 10   HART
  exclusiveMinimum          2 / 2   
  items                     3 / 8   
  maxItems                  3 / 8   
  maxLength                 4 / 20  
  maxProperties             2 / 3   
  maximum                   6 / 18  
  minItems                  2 / 8   
  minLength                 5 / 14  
  minimum                   8 / 48  
  oneOf                     7 / 8   
  pattern                   4 / 9   
  required                 40 / 229 
  type                     11 / 165 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:933: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
42 gueltige, 146 ungueltige Fixtures
  bytegleich: 190 Dateien, MANIFEST sha256=bedc885d7e2ca10aebadee1d41b04187ed61b2792649804d04600d5dbb96cbee
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  flatc-Beleg: Commit 7e163021e59cca4f8e1e35a7c828b5c6b7915953, sha256=c4cf81a4f940f91a29c0d9f627654635bed88b5307f4e0f640ca26a63d9f3018  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\nakama-flatc-commit-Release.txt)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,85 s

stdout:

```text
9 gueltige, 53 ungueltige Fixtures
  bytegleich: 63 Dateien, MANIFEST sha256=b6be5564a60ba6e524ea174807c253c7ad7ee0e810bef2260cc6eba52f1e8282
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,26 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,21 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,42 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (737942 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7105024 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      jede gebaute TU hat exakt die heutige Definemenge (5 TUs, 16 Defines)

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
  ok      Archivmitglieder sind genau die 5 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

26 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 0,82 s

stdout:

```text
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag  [3 vs 3; identity=ok]
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID im Installer-Manifest
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest
  ok      faellt am verdorbenen Manifest: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt am verdorbenen Manifest: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt am verdorbenen Manifest: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt am verdorbenen Manifest: Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt am verdorbenen Manifest: kein Viercode, keine Class-ID im Installer-Manifest
  ok      faellt am verdorbenen Manifest: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt am verdorbenen Manifest: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt am verdorbenen Manifest: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt am verdorbenen Manifest: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt am verdorbenen Manifest: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt am verdorbenen Manifest: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt am verdorbenen Manifest: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[4] Auslieferungsstand
  hinweis nicht ausgeliefert - 4 Artefakt(e) ohne Hash: main, passive-probe, active-probe, eqcop-broker.exe
          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

38 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 26,08 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
  ok      kanonische VST3-Produktpolicy steht unveraendert im Original
  ok      kanonische Broker-Produktpolicy steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle
      - $erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
      + $erlaubteVst3Basis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-3jq6wa_3\ziel\VST3'
      - $erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
      + $erlaubteBrokerBasis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-3jq6wa_3\ziel\programme\evenacadia\Nakama'

[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
  ok      Bundle ohne erwartete innere Payload wird fail-closed verweigert  [Exit 1]
  ok      Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0: phili\AppData\Local\Temp\nakama-gegenpfad-3jq6wa_3\ziel\VST3\EQ-Copilot.vst3]
  ok      Nakama Suna installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-3jq6wa_3\ziel\VST3\Nakama Suna.vst3]
  ok      Nakama Probeeq installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-3jq6wa_3\ziel\VST3\Nakama Probeeq.vst3]
  ok      eqcop-broker.exe installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-3jq6wa_3\ziel\programme\evenacadia\Nakama\eqcop-broker.exe]
INSTALLATION OK
]
  ok      geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg
  ok      jede Sicherung ist relativ an backups/<transaktions-id> gebunden
  ok      Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet
  ok      beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM
  ok      wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      passive-probe: Bundle traegt moduleinfo.json  [Nakama Suna.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[2b] Aktiver Recovery-Anker wird nicht still superseded
  ok      geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie  [Exit 1]
  ok      VORBEREITET verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      VORBEREITET: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      KOMPENSATION verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      KOMPENSATION: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      ERROR_TEILSTAND verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      ERROR_TEILSTAND: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      RUECKWEG_AKTIV verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      RUECKWEG_AKTIV: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded  [Exit 0]
  ok      Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren
  ok      zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle vier Artefakte melden `aktuell`  [4x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben
  ok      frische Transaktion friert den nun bekannten historischen Stand ein
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht
  ok      passive-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Suna.vst3]
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      das im Journal belegte Broker-Ziel ist weg
  ok      unbelegte Elternverzeichnisse bleiben unangetastet
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert
  ok      spaeter Installationsfehler endet laut und nicht als Teilerfolg  [Exit 1]
  ok      Journal bestaetigt die vollstaendige Kompensation  [ERROR_RUECKGEROLLT]
  ok      Main ist nach der Kompensation bytegleich zum Vorzustand
  ok      passive-probe: kein Teilstand nach spaetem Fehler
  ok      active-probe: kein Teilstand nach spaetem Fehler
  ok      der fremde Blocker wurde nicht angetastet
  ok      vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei
  ok      ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden  [Exit 0]
  ok      nach Fehler-Supersede bleibt nur die aktive Backup-Generation

[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig
  ok      Ausgangsinstallation fuer den Hard-Crash-Zwischenstand
  ok      VORBEREITET + mutation_begonnen restauriert statt Drift zu melden  [Exit 0]
  ok      der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt
  ok      Ausgangsinstallation fuer einen unterbrochenen inversen Pfad
  ok      RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort  [Exit 0]
  ok      auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand

[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen
  ok      Ausgangsinstallation fuer Journal-Missbrauchstests
  ok      manipulierte Repo-Diagnose ist keine Rueckwegautoritaet  [Exit 0]
  ok      fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben  [Exit 1]
  ok      fremder Loeschpfad im Journal wird nicht entfernt  [Exit 1]
  ok      Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen  [Exit 1]

[11] Geschuetzte Autoritaet, Backup- und Zieldrift
  ok      oeffentliches Journal allein autorisiert keinen Rueckweg  [Exit 1]
  ok      Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash  [Exit 1]
  ok      frische Ausgangsinstallation fuer Zieldrift
  ok      veraendertes Installationsziel wird nicht ueberschrieben  [Exit 1]
  ok      Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet

[12] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2
  ok      Runtime verwirft eine Bundle-Zielkollision  [Exit 1]
  ok      Runtime bindet Identity typstreng an State-Schema 2  [Exit 1]
  ok      Runtime verwirft kanonisches Broker-Zieltraversal  [Exit 1]

80 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a19"></a>
#### A19 | erzeuge_bandgitter_header.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen (204 Zeilen).
    Evidenz nakama_1_24_oct_30_18k_v1: 221 Baender, 222 Kanten
    Live    nakama_log64_v1: 64 Gruppen, lueckenlose Partition der 221
```

stderr:

_(leer)_

<a id="a15"></a>
#### A15 | EqCopSunaNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse passive_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [passive_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [172 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [passive_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [172 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse active_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [active_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [3439 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [3439 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes Ziel im Manifest hat hier eine Zeile  [3 vs 3]
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest
  ok      CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle: COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle: der Viercode steht nicht literal im Zielblock  [Eqcp]
  ok      CMake-Quelle: der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      CMake-Quelle: der Produktname steht nicht literal im Zielblock
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden
  ok      CMake: Defineblock der Sondenfunktion gefunden
  ok      CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      passive-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkPr]
  ok      passive-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Suna]
  ok      passive-probe: NakamaSuna entsteht ueber nakama_sonde_ziel()
  ok      passive-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaSuna    passive-probe NAKAMA_SONDE_PASSIV)]
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 11:10:00pm vs 28 Aug 2026 9:57:32pm]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      passive-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\Resources\moduleinfo.json]
  ok      passive-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 11:10:03pm vs 28 Aug 2026 9:57:32pm]
  ok      passive-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      passive-probe: Produktname wie im Manifest  [Nakama Suna]
  ok      passive-probe: Vendor wie im Manifest  [evenacadia]
  ok      passive-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      passive-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: Unterkategorien wie im Manifest
  ok      passive-probe: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 11:10:05pm vs 28 Aug 2026 9:57:32pm]
  ok      active-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      active-probe: Produktname wie im Manifest  [Nakama Probeeq]
  ok      active-probe: Vendor wie im Manifest  [evenacadia]
  ok      active-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      active-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B4163]
  ok      active-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B4163]
  ok      active-probe: Unterkategorien wie im Manifest
  ok      active-probe: keine fremde Ziel-CID im Bundle
  ok      Viercode ist vier Zeichen lang: main.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      Viercode ist vier Zeichen lang: passive-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      Viercode ist vier Zeichen lang: active-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: active-probe Component (NkAc)  [ABCDEF019182FAEB45766E614E6B4163]
  ok      CID nachgerechnet: active-probe Controller (NkAc)  [ABCDEF011234ABCD45766E614E6B4163]
  ok      VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus  [5653544571637065712D636F70696C6F]
  ok      die echte CID stammt aus dem jucePluginId-Pfad
  ok      Golden-Ordner vorhanden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\identity]
  ok      Rolle 'sensor' geladen  [sensor]
  ok      Rolle 'sensor': Sensor-ID unveraendert uebernommen
  ok      Rolle 'sensor': Label uebernommen  [Klavier A]
  ok      Rolle 'sensor': Paar-ID uebernommen
  ok      Golden 'sensor' gelesen  [state-schema1-sensor.bin]
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 114 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,31 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      Matrix Klasse=main, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=main, Position=pre: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position pre is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=main, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for main', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=legacy, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=legacy, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for legacy', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=passive_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=passive_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for passive_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      Matrix Klasse=active_probe, Position=insert: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=pre: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post: erwartet LadeErgebnis::geladen, tatsaechlich LadeErgebnis::geladen  [nurLesen=false, Grund='', Rueckweg=nicht gefordert, Host-read-only=nicht gefordert, Host-Dirty=nicht gefordert]
  ok      Matrix Klasse=active_probe, Position=post_fader_contribution: erwartet LadeErgebnis::nurLesen, tatsaechlich LadeErgebnis::nurLesen  [nurLesen=true, Grund='measurement_position post_fader_contribution is not allowed for active_probe', Rueckweg=bytegleich, Host-read-only=true, Host-Dirty=nein]
  ok      16 Kombinationen aus Klasse x Messposition wie §2.2 - post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)  [16]
  ok      == Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle
  ok      21 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [21]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser
  ok      gueltiger Praefix mit Suffix wird vollstaendig ignoriert
  ok      abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen
  ok      deklarierte Riesenvariante wird vor JUCE-Allokation verworfen
  ok      zu tiefer ValueTree wird begrenzt und ignoriert
  ok      zu tief geschachteltes Variantenarray wird begrenzt und ignoriert
  ok      State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen
  ok      ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel
  ok      63 verschachtelte Arrays plus Blatt passieren und laden
  ok      65.536 Eintraege in einer Sammlung passieren den Byte-Riegel
  ok      65.537 Eintraege in einer Sammlung werden verworfen
  ok      262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel
  ok      262.145 Eintraege ueber mehrere Sammlungen werden verworfen
  ok      Teststate trifft die schreibbare Bytegrenze exakt  [16773120]
  ok      Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773848]
  ok      State ohne konkreten Writer-Headroom bleibt read-only bytegleich
  ok      vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [9]
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [10]
  ok      additive Binaer- und Array-Properties bleiben bytegleich lesbar
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [c9755bff8f4b481b9ba44460ca042fa2]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [5346f1492171480b9ee616b5421add74]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 169 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
== A · Quellhash-Gate (JUCE 8.0.9) ==
  ok      JUCE-Wrapper liegt am gepinnten Ort: C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp
  ok      Patchdatei liegt im Repo: juce-8.0.9-nakama-vst3-bridge.patch
  ok      die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)
  ok      Wrapper liess sich lesen
  ok      Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: 6e5d4660d960836a…)
  ok      Wrapper ist NICHT der unberuehrte Auslieferungsstand
  ok      Patch: Bruecken-Header eingebunden
  ok      Patch: IAudioPresentationLatency als Basisklasse
  ok      Patch: Interface auch im queryInterface
  ok      Patch: Buslatenz-Methode umgesetzt
  ok      Patch: Blockbeginn verdrahtet
  ok      Patch: Context-Anwesenheit verdrahtet
  ok      Patch: fehlender Context wird ausdruecklich gemeldet
  ok      Patch: alle Parameterpunkte werden beobachtet
  ok      Patch: Uebergabe an die Senke verdrahtet
  ok      Patch: Bruecke als Wrapper-Member
  ok      Patch: Senke wird EINMAL beim Erzeugen aufgeloest
  ok      Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert
  ok      ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)
  ok      Patchdatei ist ein echter Unified Diff
  ok      Patch zielt auf den VST3-Wrapper
== B · Context-Anwesenheit und Gueltigkeitsbits ==
  ok      Senke wird genau einmal je Block gerufen
  ok      fehlender Context: processContextPresent == false
  ok      fehlender Context: Projektzeit ist UNGUELTIG, nicht 0
  ok      fehlender Context: Tempo und PPQ ungueltig
  ok      genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)
  ok      genullter Context: Projektzeit gueltig und 0 â eine Aussage, keine Vermutung
  ok      genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)
  ok      genullter Context: Tempo bleibt ungueltig
  ok      genullter Context: playing ist gueltig false â nicht 'unbekannt'
  ok      voller Context: anwesend
  ok      voller Context: projectTimeSamples 123456
  ok      voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)
  ok      voller Context: playing true
  ok      voller Context: recording true
  ok      voller Context: Tempo 128
  ok      voller Context: PPQ 8.25
  ok      voller Context: Taktbeginn 8.0 PPQ
  ok      voller Context: Cycle 4.0â12.0 PPQ, aktiv (PPQ, nicht Samples)
  ok      Teil-Context: playing true
  ok      Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)
  ok      Teil-Context: Cycle nicht aktiv
  ok      beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)
== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==
  ok      drei Punkte angekommen
  ok      Punkte sind nach Sample-Offset sortiert
  ok      sauberer Block: sampleAccurateAutomation == true
  ok      Blockgroesse wird mitgefuehrt
  ok      kleinerer Offset zuerst, auch wenn spaeter geliefert
  ok      gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)
  ok      Punkte aus mehreren Queues sind global aufsteigend
  ok      Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte
  ok      Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt
  ok      Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ueberlauf des Ereignisrings verwirft KEINEN Letztwert
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      NaN nach Ueberlauf: Zusicherung faellt
  ok      NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Letztwert-Tabelle: genau kMaxLetztwerte Parameter werden gefuehrt
  ok      Letztwert-Tabelle: die 3 ueberzaehligen Parameter sind gezaehlt
  ok      Letztwert-Ueberlauf laesst die Zusicherung ebenfalls fallen
  ok      Letztwert-Ueberlauf: der ueberzaehlige Parameter meldet ehrlich 'nichts'
  ok      ohne Ueberlauf: Rueckfallwert deckt sich mit dem letzten Ringeintrag (0.30)
  ok      ohne Ueberlauf: der Ring traegt denselben Wert am Ende
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      der Buszaehler steht im Blockbefund, ist also fuer die Senke sichtbar
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (7880 Byte)

HOSTKONTEXT OK — 91/91 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3b"></a>
#### B3b | EqCopHostProbeTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

stdout:

```text
== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==
  ok      meldet 0 Samples Latenz
  ok      meldet 0 s Tail
  ok      200 Bloecke float: Ausgang bitgleich zum Eingang
  ok      double-Block: Ausgang bitgleich zum Eingang
  ok      double-Praezision wird dem Host angeboten
  ok      5.1 wird klar abgelehnt, nicht still gemischt
== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==
  ok      dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch
  ok      vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT
  ok      nach dem ersten Bruecken-Block meldet es: Bruecke liefert
== C - Context-Anwesenheit und Gueltigkeiten ==
  ok      acht Bloecke mit Context gezaehlt
  ok      der eine Block OHNE Context ist getrennt gezaehlt
  ok      neun verarbeitete Bloecke
  ok      neun Senke-Aufrufe - keine Asymmetrie ohne Grund
  ok      der Kontextverlust ist als Ereignis vermerkt
  ok      Tempo war in jedem Context gueltig
  ok      continousTimeSamples war NIE gueltig - und wird nicht erfunden
== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==
  ok      fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)
  ok      Seek vorwaerts wird als Sprung erkannt
  ok      die Sprungweite wird mitgemessen
  ok      Seek rueckwaerts wird als Sprung erkannt
  ok      nach Stop/Play wird der Positionswechsel NICHT als laufender Sprung gemeldet
  ok      er verschwindet aber auch nicht: eigener Zaehler 'ueber Stop/Play' (T2-Befund 21.08.)
  ok      und ein eigenes Ereignis, damit eine 0 bei den Spruengen nicht mehrdeutig ist
  ok      Stop und Play sind als Transportereignisse vermerkt
  ok      Loop-Wechsel erscheint als Rueckwaertssprung
  ok      Schleife-an ist genau einmal vermerkt
  ok      der Schleifenzustand wird gefuehrt
  ok      Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)
== E - Samplegenaue Automation: der Kernbeweis von Termin B ==
  ok      ohne Automation wird nichts behauptet
  ok      ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben
  ok      alle Punkte werden aufsummiert
  ok      der dichteste Block wird gemerkt
  ok      Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit
  ok      kleinster und groesster Offset werden gefuehrt
  ok      der erste Mehrpunkt-Block ist als Ereignis vermerkt
  ok      Mehrpunkt-Block OHNE Zusicherung zaehlt NICHT als samplegenau
  ok      er wird stattdessen getrennt gezaehlt - kein Befund verschwindet
  ok      Unplausibilitaeten und Ueberlaeufe der Bruecke werden AUFSUMMIERT uebernommen, nicht geglaettet
  ok      beide Bloecke ohne Zusicherung sind gezaehlt (der mit Offset- und der mit Wertfehler)
== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==
  ok      ohne Hostmeldung: 'nie gemeldet' - und KEIN Eintrag wird erfunden
  ok      Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten
  ok      die Gegenrichtung bleibt ungemeldet - es wird kein Wert erfunden
  ok      Ausgang Bus 0 = 1024 wird NEBEN dem Eingang gefuehrt, nicht verschluckt
  ok      auch ein dritter Bus kommt an
  ok      ein nicht gemeldeter Bus dazwischen bleibt ungemeldet
  ok      der zuerst gemeldete Latenzwert rastet ein
  ok      die spaetere Aenderung wird gezaehlt statt still verworfen
  ok      50 weitere Bloecke mit DEMSELBEN geaenderten Wert zaehlen NICHT weiter (Uebergaenge, nicht Bloecke)
  ok      ein dritter, wieder anderer Wert zaehlt als zweiter Uebergang
  ok      der Bericht listet genau die drei gemeldeten Eintraege - keinen erfundenen
  ok      und der Wertwechsel-Zaehler steht im Bericht - mit der Einheit im Schluesselnamen
== G - Senke ohne processBlock (Flush/Hostriegel) ==
  ok      Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt
  ok      der Block ohne Verarbeitung ist ausdruecklich vermerkt
== H - Gegenpfad: Bericht schreiben und wieder einlesen ==
  ok      der Bericht ist nicht leer
  ok      der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)
  ok      Schemaname steht im Bericht
  ok      der Bericht traegt die Kernaussage 'samplegenau belegt'
  ok      die Blockzahl im Bericht deckt sich mit der Messung
  ok      das Ereignisprotokoll liegt im Bericht
  ok      die Gueltigkeitsmaske steht im Bericht
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260828-231109.json
  ok      die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung
== I - Zuruecksetzen und Ringueberlauf ==
  ok      Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung
  ok      nach dem Zuruecksetzen wird die laufende Aufnahme wieder als aufnahme_an gemeldet
  ok      mehr Ereignisse als Ringplaetze wurden erzeugt
  ok      die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still
  ok      der Ring gibt genau seine Kapazitaet heraus
  ok      die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)
== I2 - Offline-Uebergang und negative Projektzeit ==
  ok      Offline- und Echtzeitbloecke getrennt gezaehlt
  ok      der Wechsel IN den Render ist vermerkt
  ok      der Wechsel ZURUECK in Echtzeit ist ebenfalls vermerkt - nicht nur der erste Block ueberhaupt
  ok      negative Projektzeit wird GEZAEHLT, nicht still uebersprungen
  ok      der erste negative Wert ist als Ereignis vermerkt
  ok      im negativen Bereich wird kein Scheinsprung erfunden
== J - Audiothread: keine Allokation ==
  ok      500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen
== J2 - Nebenlaeufig lesen, waehrend der Audiothread schreibt ==
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (37015 Bloecke)
  ok      200 nebenlaeufige Lesevorgaenge liefern durchweg plausible Eintraege und Zaehler
  ok      der letzte Lesevorgang hat Eintraege geliefert
  hinweis   Grenze: Rauchtest, kein Beweis der Tearing-Freiheit (der Wiederholpfad wird selten bis nie betreten)
== K - Anzeige: passt der Inhalt ueberhaupt ins Fenster? ==
  ok      der Editor laesst sich erzeugen
  ok      Fensterhoehe 850 deckt den Leerzustand (850 noetig)
  ok      Fensterhoehe deckt auch den Messzustand (850 noetig)
  ok      keine Textzeile liegt unter den Knoepfen (Text bis 760, Knopfstreifen ab 794)
  ok      keine Wertzeile ohne Beschriftung - und 27 Zeilen wurden dabei wirklich geprueft
  ok      hoechstens EINE unbeschriftete Hinweiszeile (gefunden: 1)
  ok      und der Riegel faellt: eine untergeschobene Wertzeile ohne Beschriftung wird erkannt

HOSTPROBE OK - 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3c"></a>
#### B3c | EqCopSchemaTest

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,19 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [102 Faelle]
[ok]   Falltabelle hat Substanz  [102 Faelle]
[ok]   Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (62 Fixtures)
[ok]   Binaerkorpus hat Substanz  [62 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-auf-2hoch53.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740992.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-nahe-2hoch53-ab.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740991.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-unter-2hoch53-auf.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 4503599627370495.9]
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-kleiner-bruch-rundet-auf-eins.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 1.00000000000000001]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nan-token.json  [unbekanntes Literal NaN an Position 1090]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (188 Fixtures)
[ok]   Korpus hat Substanz  [188 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
63 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 3,62 s

stdout:

```text
== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==
Produktionsstrom GenStrom: 1 Tap(s), 131072 Frames Ring, 2048 Deskriptoren, max 16384 Frames je Block.

== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==
  ok      drei Bloecke, drei Deskriptoren  [3]
  ok      jeder Frame kommt bitgleich und an derselben Stelle heraus
  ok      Stempel: lokaler Strom und Projektzeit stimmen je Block
  ok      nach dem Leeren ist die Queue leer
  ok      kein Verlust bei Normallast
== B - Wrap: der Ring laeuft mehrfach um ==
  ok      der Ring ist wirklich mehrfach umgelaufen  [11 Umlaeufe]
  ok      ueber jeden Wrap hinweg bitgleicher Inhalt
  ok      kein Frame verloren, keiner doppelt  [12000/12000]
== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==
  ok      vier Bloecke passen, danach ist Schluss  [4]
  ok      beide abgewiesenen Bloecke sind gezaehlt  [2]
  ok      verlorene Frames = 2 x 256 (keine Teilmenge)  [512]
  ok      ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock
  ok      der erste Block nach dem Verlust traegt kFlagLueckeDavor
  ok      und beginnt ein neues continuity_segment  [1]
== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==
  ok      genau `deskriptorPlaetze` Bloecke passen  [8]
  ok      die vier ueberzaehligen sind gezaehlt  [4]
  ok      und genau so viele stehen zum Lesen bereit  [8]
== E - Oversize: ein Block ueber der Slotkapazitaet ==
  ok      ein Block ueber maxBlockFrames wird als GANZES abgewiesen
  ok      und als Oversize gezaehlt, nicht als Ueberlauf  [1/0]
  ok      der Ueberlaufzaehler bleibt sauber
  ok      nichts Halbes im Ring
  ok      die verlorene Zeit ist vollstaendig gemeldet  [257]
  ok      genau maxBlockFrames passt noch (Kante, nicht Ausschluss)
== F - Flush (numSamples == 0) ist kein Verlust ==
  ok      ein Flush-Block wird nicht eingestellt
  ok      und zaehlt NICHT als Verlust - er traegt keine Audiozeit
  ok      der Folgeblock geht normal
  ok      und traegt keine Luecke
== G - Mono und Mehrfach-Tap ==
  ok      Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)
  ok      und der Deskriptor sagt kanaele == 1
  ok      Drei-Tap-Layout: Block angenommen
  ok      das Bitset meldet genau die belegten Taps  [3]
  ok      jeder Tap liegt getrennt und richtig; der inaktive liefert false
  ok      eine falsche Tapzahl wird abgewiesen statt halb kopiert
  ok      ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor
== H - Ein-Block-Quarantaene ==
  ok      5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene  [4]
  ok      und die Quarantaene haelt ihn wirklich
  ok      kein Bruch bei lueckenlosem Strom
  ok      der Seek ist als Bruch erkannt  [1]
  ok      genau EIN Block faellt an der Grenze - der Strom laeuft weiter
  ok      der Block NACH der Grenze ist schon wieder in Quarantaene
  ok      Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)
  ok      stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      Vorlauf ohne Bruch
  ok      Transportkante play->stop ist ein Bruch  [1]
  ok      ohne Hostkontext traegt der lokale monotone Strom (§53.6)
  ok      und versiegelt normal  [3]
  ok      Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung
  ok      negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)
== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==
  20 000 Bloecke, Groessen 1..288: angenommen 17809, Ueberlauf-Drops 0, Oversize-Drops 2191, groesster Block 256, versiegelt 15872, Brueche 1936
  ok      jeder gelesene Frame steht an der Stelle, die sein Stempel nennt
  ok      es gab nie einen Block mit sampleCount 0
  ok      die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt  [2191]
  ok      kein angenommener Block war je groesser als die Slotkapazitaet  [256]
  ok      es wurde wirklich gelesen  [2288127 Frames]
== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==
  ok      18 Blockgroessen von 1 bis 16 384: kein Sample veraendert
  ok      0 Samples Latenz  [0]
  ok      kein Tail
  angenommen 18, groesster Block 16384, Ueberlauf 0, Oversize 0
== K - verdrahtet: Oversize aendert das Audio nicht ==
  ok      ein Block ueber der Slotkapazitaet laeuft unveraendert durch
  ok      und ist als Oversize-Drop der ANALYSE gezaehlt  [1]
  ok      der abgewiesene Oversize-Block erscheint nicht im Projektfenster
  ok      der naechste angenommene Lueckenblock beginnt das Fenster exakt neu  [16385..16449]
  ok      der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben  [0]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3844 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
== M - verdrahtet: die Messung bekommt den Strom wirklich ==
  eingespeist 287744, analysiert 287232, Drops 0, versiegelt 561, Quarantaene-Verwuerfe 0, Brueche 0
  ok      bei Normallast kein einziger Drop
  ok      und kein Kontinuitaetsbruch (lueckenloser Transport)
  ok      die Engine sieht den ganzen Strom bis auf den Block in Quarantaene  [512 Frames offen]
  ok      Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)  [-8.684240]
  ok      framesDropped im Stats-Snapshot kommt aus derselben Quelle
== N - Gegenpfad: prepareToPlay mitten im Betrieb ==
  ok      auch der erste Block nach prepareToPlay alloziert nicht  [0]
  veraltete Bloecke (alter Anlauf, nicht analysiert): 40
  ok      ein Neuanlauf ist kein Oversize
== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==
  ok      der Anlauf steigt sofort, nicht erst beim naechsten Audioblock  [0 -> 1]
  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt 3, als aktuell durchgelassen 0
  ok      alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar  [3/3]
  ok      der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt
  ok      Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell
  ok      und er traegt die Luecke, die der Neuanlauf gerissen hat
  ok      der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut
  ok      zwei Neuanlaeufe hintereinander gehen beide nicht verloren  [1 -> 3]
  ok      zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration
  ok      und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste
  ok      Nullframe konsumiert die wartende Generation nicht
  ok      der naechste echte Block uebernimmt sie weiterhin
== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==
  ok      Seek um 10 s bei gestopptem Transport ist EIN Bruch  [1]
  ok      Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)  [0]
  ok      und bei laufendem Transport ist der Seek weiterhin ein Bruch  [1]
  ok      bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet  [0]
== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==
  ok      nach Start ist nur die leichte Publikation sofort faellig
  ok      10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus
  ok      vor 250 ms bleibt Gating/Kandidaten gesperrt
  ok      bei 250 ms ist genau die schwere Runde faellig
  ok      dieselbe Deadline kann nicht zweimal verbraucht werden
  ok      nach Pause genau eine Runde, kein Catch-up-Sturm
  ok      Reset/Generation startet die 250-ms-Deadline neu
== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==
  ok      Test hat echten Queue-Rueckstau hergestellt  [257 Bloecke bis zum ersten Drop]
  ok      unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms  [1 Runden in 600 ms]
  ok      neue Generation erbt keinen Batch-beschleunigten Schwertakt  [0 Runden in den ersten 100 ms]
  ok      Frame-Leser wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Reset-Steuerer wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Stop/Join bleibt auch mit Queue-Rest begrenzt  [0 ms]
== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==
  ok      Gegenprobe: erste Generation hat ihr eigenes Projektfenster
  ok      Testbarriere liegt deterministisch im laufenden Audiocallback
  ok      der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster  [10000..10064]
  ok      die Generationskante ist kein Seek innerhalb des alten Fensters  [0]
== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==
  ok      Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine
  ok      nichtendliche Hostrate wird fail-closed auf 0 gespiegelt
  ok      M1 verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      FeatureEngine verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung
  ok      eine folgende gueltige Generation aktiviert beide Engines wieder

114 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Feingitter: 10001 Bins a 0.01 LU ueber [-70.00, 30.01) LUFS.
Oberband:   3070 Bins a 1.00 LU ueber [30.01, 3100.01) LUFS - lautheit(DBL_MAX) = 3081,86.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: kein Block faellt durch das Raster  [0]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: kein Block faellt durch das Raster  [0]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): kein Block faellt durch das Raster  [0]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: kein Block faellt durch das Raster  [0]
  ok      Musik + echte Stille: LUFS-I innerhalb ±0,1 LU  [akku=-8.281644 ref=-8.281644 d=0.000000000]
  ok      Musik + echte Stille: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Musik + echte Stille: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000027]
  ok      Musik + echte Stille: Kurz-LUFS Gueltigkeit gleich
  ok      Musik + echte Stille: Kurz-LUFS BITGLEICH  [akku=-12.451912590513381 ref=-12.451912590513381]
== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==
  Sweep: 201 Laeufe, groesste Abweichung 2.781360 LU bei Faktor 1.000000, groesste gemeldete Schranke 2.781412 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen (Eingang aendert sich um 1e-5): 2.781403 LU
  ok      Sweep vollstaendig gefahren  [201/201]
  ok      Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)  [sonst pruefte D nichts; schranke=2.781412]
  ok      die selbstgemeldete Schranke deckt JEDEN Lauf  [201/201]
  ok      wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU  [76/76]
  ok      die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)  [maxD=2.781360 refSprung=2.781403]
  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)
== E - fixed memory: eine Million Zellen ==
  Akku:     0 Allokationen, LUFS-I -8.386510, Schranke 0.000000000 LU
  Referenz: 36 Allokationen, LUFS-I -8.386510
  ok      1 000 000 Zellen: 0 Allokationen im Akku  [0]
  ok      Gegenprobe: die alte Rechnung alloziert sehr wohl  [36]
  ok      1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU  [d=0.000000000]
  Fester Bedarf laut speicherBytes(): 157122 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      Kurz-LUFS meldet einen nichtendlichen Ring nie als gueltig
  ok      ueber dem Gitter: EIN Pegel bleibt wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      Gegenprobe: der Korpus liegt wirklich im Oberband  [197]
  ok      und kein Block faellt durch das Raster
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer
== G - Oberband: zwei Pegel ueber dem Feingitter ==
  G1: akku=49.302611 ref=49.302611 schranke=0.000000000 ueberGitter=1997
  ok      G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)  [1997]
  ok      zwei Pegel ueber dem Gitter (T2-1): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      zwei Pegel ueber dem Gitter (T2-1): kein Block faellt durch das Raster  [0]
  ok      zwei Pegel ueber dem Gitter (T2-1): LUFS-I innerhalb ±0,1 LU  [akku=49.302611 ref=49.302611 d=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS Gueltigkeit gleich
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS BITGLEICH  [akku=32.319299956639810 ref=32.319299956639810]
  ok      Naht, Gamma_r im Oberband: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Oberband: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Oberband: LUFS-I innerhalb ±0,1 LU  [akku=49.302481 ref=49.302481 d=0.000000000]
  ok      Naht, Gamma_r im Oberband: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Oberband: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  ok      Naht, Gamma_r im Feingitter: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Feingitter: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Feingitter: LUFS-I innerhalb ±0,1 LU  [akku=49.243363 ref=49.243363 d=0.000000000]
  ok      Naht, Gamma_r im Feingitter: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Feingitter: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  G3-Sweep: 201 Laeufe, groesste Abweichung 2.781577 LU bei Faktor 1.001000, groesste gemeldete Schranke 2.781577 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: 2.781401 LU
  ok      G3: Sweep vollstaendig gefahren  [201/201]
  ok      G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)  [1997]
  ok      G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)  [sonst pruefte G3 nichts; schranke=2.781577]
  ok      G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband  [201/201]
  ok      G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz  [maxD=2.781577 refSprung=2.781401]
  ok      G3: kein Block faellt durch das Raster  [0]
  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - 1000 Bloecke kippen gemeinsam die Gateseite.)
  G4: LUFS-I 3038.851, Schranke 0.000000000, ueberGitter 197, ueberOberband 0
  ok      G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert  [3038.851]
  ok      G4: nichts wird faelschlich als nicht-endlich gezaehlt  [0]
  ok      G4: auch am aeussersten Rand faellt kein Block durch das Raster  [0]
  ok      G4: und die Schranke bleibt endlich  [0.000000000]

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
109 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b5"></a>
#### B5 | EqCopAnalysisGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 2,21 s

stdout:

```text
== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==
Gate: "Drop/Seek/Loop trennt jedes offene Fenster."
Stufen: Bass 16384 (Hop 8192), Haupt 4096 (Hop 2048), Trennung bei 200 Hz.

== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==
[ok]   Evidenzgitter: band_anzahl == kEvidenzBaender  [221]
[ok]   Livegitter: band_anzahl == kLiveBaender  [64]
[ok]   Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)  [0 Abweichungen]
[ok]   Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)  [0 Abweichungen]
[ok]   Live-Kanten BITGLEICH zur Fixture (65 Werte)  [0 Abweichungen]
[ok]   Live-Mitten BITGLEICH zur Fixture (64 Werte)  [0 Abweichungen]
[ok]   Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal  [0 Abweichungen, Ende bei 221]
[ok]   Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank  [0 Abweichungen]

== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==
[ok]   q_db_0p1_i16: Skalierung stimmt mit der Fixture ueberein  [10]
[ok]   q_db_0p1_i16: alle 23 Vektoren treffen
[ok]   q_db_0p01_i16: Skalierung stimmt mit der Fixture ueberein  [100]
[ok]   q_db_0p01_i16: alle 23 Vektoren treffen
[ok]   float32: alle 15 Vektoren treffen bitgenau
[ok]   Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0
[ok]   halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl
[ok]   und negativ genauso (-0,05 -> -1)

== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==
[ok]   Band i sitzt in Byte i/8, Bit i%8 (LSB-first)  [Byte0=81 Byte1=1]
[ok]   ein Bit loeschen laesst die Nachbarn stehen
[ok]   Lesen und Schreiben sind dieselbe Adressierung
[ok]   221 Baender brauchen 28 Bitmapbytes  [28]
[ok]   die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)  [letztes Byte = 1f]

== D - FFT: gegen analytisch bekannte Ergebnisse ==
[ok]   Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin  [100.0000 %]
[ok]   Parseval haelt (Zeit- und Frequenzenergie gleich)  [rel. Fehler 0.0000000000000002]
[ok]   Gleichanteil sitzt vollstaendig in Bin 0  [Bin0 = 1048576.0, Rest = 0.00000000000000000000]

== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==
[ok]   RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander  [max 0.04312 dB bei 20000.0 Hz]
       (gemessene Groesstabweichung, zur Ablesung: 0.043120 dB)
[ok]   Hochpass sperrt den Gleichanteil vollstaendig  [|H(0)|Â² = 0.000000000000000000000000]

== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==
[ok]   die Sequenz zaehlt ab 1 aufwaerts  [6]
[ok]   mit gueltiger Projektzeit ist die Zeitbasis project_samples
[ok]   und project_sample_start ist gesetzt
[ok]   Gueltigkeitsbit project_time steht
[ok]   der Frame traegt seine Metrikversion  [20260823]
[ok]   und die Samplerate
[ok]   Frame-Stempel spannt alle integrierten Hostbloecke auf  [0 + 5120]
[ok]   Frame-Stempel umfasst den ueberlappenden FFT-Support  [2048 + 8192]
[ok]   ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck
[ok]   Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen
[ok]   fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden
[ok]   Evidenzframe vereinigt Haupt- und aelteren Basssupport  [0 + 30720, 196 Baender]
[ok]   stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen
[ok]   stehende FL-Teilstuecke erfinden kein Projektintervall
[ok]   ueberlaufende Host-Projektspanne wird konservativ lokal publiziert
[ok]   Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block  [104608 + 512]
[ok]   ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (Â§32.3)
[ok]   und dann steht WEDER das Bit NOCH ein Startwert da
[ok]   alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet  [0x7f]
[ok]   eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (Â§32.3)
[ok]   Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer
[ok]   continuous_time_samples kommt mit demselben Frameanker durch
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: NaN
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Inf
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Ende vor Start
[ok]   ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich

== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==
[ok]   G1 Drop (kFlagLueckeDavor): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G1 Drop (kFlagLueckeDavor): Ursache ist lokaleLuecke  [Zaehler 1]
[ok]   G1 Drop (kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche (§32.3)  [Epochen 0 Segmente 1]
[ok]   G2 Seek waehrend Wiedergabe: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G2 Seek waehrend Wiedergabe: Ursache ist zeitSprung  [Zaehler 1]
[ok]   G2 Seek waehrend Wiedergabe: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G3: bei gestopptem Transport laufen die Fenster normal weiter  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)  [0 Epochenwechsel]
[ok]   G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G3: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G3: und sie heisst zeitSprung, nicht lokaleLuecke
[ok]   G4: aktive Schleife allein trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4 Loop-Wrap: jedes Fenster getrennt  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G4: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G4: und die Ursache heisst loopWrap, nicht zeitSprung
[ok]   G5: Schleife ohne erreichbare Grenze trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, weil der Straddle-Block auch kein neues beginnt  [Bass=0 Haupt=0 Zelle=0 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G5: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet
[ok]   G5: mit eigener Ursache moeglicherStraddle
[ok]   G6 Transportkante (Play -> Stop): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G6 Transportkante (Play -> Stop): Ursache ist transportKante  [Zaehler 1]
[ok]   G6 Transportkante (Play -> Stop): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G7 Sampleratewechsel: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G7 Sampleratewechsel: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G7 Sampleratewechsel: Ursache ist sampleratewechsel  [Zaehler 1]
[ok]   G7 Sampleratewechsel: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G8 Neuanlauf (prepareToPlay): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G8 Neuanlauf (prepareToPlay): Ursache ist neuanlauf  [Zaehler 1]
[ok]   G8 Neuanlauf (prepareToPlay): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): Ursache ist beweislageWechsel  [Zaehler 1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G11: im Betrieb traegt die K-Kette einen Filterzustand  [1.392229]
[ok]   G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame  [290 / 290]
[ok]   G11: beide Laeufe liefern eine Kurzzeitlautheit
[ok]   G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - der Filternachklang ueberbrueckt nicht  [A=-60.034744263 B=-60.034744263]
[ok]   G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)  [0 Trennungen]
[ok]   G10: die Fenster laufen dabei weiter  [Bass=9224 Haupt=3080 Zelle=264 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G12: vor der Grenze tragen BEIDE Bandakkus den Ton  [Live 149 / Evidenz 149 Baender]
[ok]   G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber die Grenze weiter (Entscheid Â§10.1)  [liveSamples=0]
[ok]   G12: und der Frame traegt die NEUE Epoche  [1]
[ok]   G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird also wirklich geprueft
[ok]   G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)  [0 gesetzte Baender]
[ok]   G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter  [0 gesetzte Baender]
[ok]   G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit  [nicht gesetzt]
[ok]   G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden  [1192 Band-Fensterbeitraege]
[ok]   G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren  [120 von 120]
[ok]   G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor (Pruefer-Messung: 80 / 40 / 43)  [0 mit Live-Baendern, 0 mit Evidenz, 0 mit Aktivitaet]

== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) ==
[ok]   G13.0: zwei frische Frames sind gleich - der Vergleich sagt nicht pauschal ungleich
[ok]   G13.0: ein einziger geaenderter Skalar macht sie ungleich
[ok]   G13.0: ein einziger geaenderter Bandwert ebenso
[ok]   G13.0: und ein einziges Bitmapbit ebenso
[ok]   G13.0: und ein Feld im verschachtelten Transportstempel - der Vergleich reicht bis dorthin
[ok]   G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen vorgefuehrt
[ok]   G13a lokaleLuecke [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13a lokaleLuecke [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13a lokaleLuecke [Zwilling]: beide haben die Grenze lokaleLuecke gezogen  [A 1 / B 1]
[ok]   G13a lokaleLuecke [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13a lokaleLuecke [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13a lokaleLuecke [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13a lokaleLuecke [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13a lokaleLuecke [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13b zeitSprung [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13b zeitSprung [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13b zeitSprung [Zwilling]: beide haben die Grenze zeitSprung gezogen  [A 1 / B 1]
[ok]   G13b zeitSprung [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13b zeitSprung [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13b zeitSprung [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13b zeitSprung [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13b zeitSprung [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13c transportKante [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13c transportKante [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13c transportKante [Zwilling]: beide haben die Grenze transportKante gezogen  [A 1 / B 1]
[ok]   G13c transportKante [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13c transportKante [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13c transportKante [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13c transportKante [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13c transportKante [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13d sampleratewechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13d sampleratewechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13d sampleratewechsel [Zwilling]: beide haben die Grenze sampleratewechsel gezogen  [A 1 / B 1]
[ok]   G13d sampleratewechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13d sampleratewechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13d sampleratewechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13d sampleratewechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13d sampleratewechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13e neuanlauf [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13e neuanlauf [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13e neuanlauf [Zwilling]: beide haben die Grenze neuanlauf gezogen  [A 1 / B 1]
[ok]   G13e neuanlauf [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13e neuanlauf [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13e neuanlauf [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13e neuanlauf [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13e neuanlauf [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13f beweislageWechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13f beweislageWechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13f beweislageWechsel [Zwilling]: beide haben die Grenze beweislageWechsel gezogen  [A 1 / B 1]
[ok]   G13f beweislageWechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13f beweislageWechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13f beweislageWechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13f beweislageWechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13f beweislageWechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]

== H - NAK-29: bedingte Feldpflichten des Transportstempels ==
[ok]   ein sauberer Stempel kommt durch
[ok]   Fall 1: project_samples + Bit, aber KEIN project_sample_start
[ok]   Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu Â§32.3)
[ok]   Fall 3: bounds_valid ohne start_ppq/end_ppq
[ok]   Fall 4: derivation=validated_block_mapping ohne bounds_valid
[ok]   Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht
[ok]   Fall 6: continuous_time-Bit ohne continuous_time_samples
[ok]   Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus
[ok]   Fall 7: sample_count bleibt unter der Vertragsobergrenze
[ok]   Fall 7: der Feature-Erzeuger publiziert keinen Leerframe
[ok]   der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel entsteht im normalen, gedeckelten Betrieb  [0]
[ok]   abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich  [1]

== I - Ereignisse: kein Fluss ueber eine Grenze ==
[ok]   der spektrale Fluss hat im Betrieb einen Vorgaenger
[ok]   nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss einen Onset, den es nie gab
[ok]   jedes Ereignis traegt die Epoche, in der es gesehen wurde  [1 Ereignis(se)]
[ok]   der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, nicht behauptet  [64 im Ring, 1 verworfen, nach 1107 Bloecken]
[ok]   und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (Â§48.1: ein Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)  [64 / 64]
[ok]   drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste  [aeltestes vorher 38912, jetzt 47104]
[ok]   und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf
[ok]   Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock
[ok]   stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit  [1 Ereignis(se) nach 84 Bloecken]

== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==
[ok]   LUFS-S ist nach 4 s gesetzt
[ok]   LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (Â§39.3)  [gemessen -6.0620 erwartet -6.0620 d=0.00000]
[ok]   Peak trifft die Amplitude  [-6.021 dBFS]
[ok]   Crest eines Sinus ist 3,01 dB  [3.0091 dB]
[ok]   L==R ergibt Korrelation 1  [1.000000]
[ok]   und Breite 0 (kein Seitenanteil)  [0.000000000000]
[ok]   das Live-Band um 997 Hz ist gueltig  [Band 35]
[ok]   und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal

== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==
[ok]   die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen  [40 Bloecke]
[ok]   und mindestens einen Frame gebaut  [4 Frames]
[ok]   process_context_present kommt aus der BRUECKE durch (Â§32.3)
[ok]   recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen
[ok]   continuous_time_samples kommt durch
[ok]   die Schleifengrenzen kommen durch, samt `active`
[ok]   beide Presentation-Latencies kommen durch  [3924 / 4410]
[ok]   ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits  [0x7f]
[ok]   Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits  [0x3]
[ok]   insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`
[ok]   Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen
[ok]   Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt
[ok]   nichtendliche Sampleraten initialisieren keine Analyseengine

== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==
[ok]   L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt
[ok]   L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen  [0]
[ok]   L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, und zwar nur dort  [1 Abweichung(en), Zeile 26]
[ok]   L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'
[ok]   L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts  [0 gesetzte Baender]
[ok]   L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte  [3]
[ok]   L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung  [1, 3]
[ok]   L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'
[ok]   L6: ein echter Grund wird gezaehlt  [1]
[ok]   L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen  [gelesen: 0]
[ok]   L6: und `keine` ebenso - getrennt wird nie ohne Grund

== M - Auskunft ueber die Grenzen der Messung ==
[ok]   die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da  [erstes messbares Band 2 (32.6 Hz)]
[ok]   bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz  [erstes Band ueber der Kappe 221]
[ok]   bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab  [erstes Band ueber der Kappe 202 (10525.6 Hz)]
[ok]   und sie sitzt genau am ersten Band, dessen Oberkante 0,95Â·Nyquist ueberschreitet  [10374.7 .. 10678.7 Hz gegen 10473.8 Hz]
[ok]   kein Live-Band vollstaendig ueber der Kappe traegt einen Wert  [0 Baender]
[ok]   Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler

Ergebnis: 235 bestanden, 0 Fehler.
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  ok      frisch: unclassified  [unclassified]
  ok      frisch: neutral
  ok      frisch: keine Audio-Ausnahme (audio-neutral)
  ok      frisch: kein Brokerstart
  ok      Schema-1 'sensor' -> legacy  [legacy]
  ok      Schema-1 'sensor': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'pre' -> legacy  [legacy]
  ok      Schema-1 'pre': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'post' -> legacy  [legacy]
  ok      Schema-1 'post': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'hub' -> main  [main]
  ok      Schema-1 'hub': Audio-Ausnahme moeglich
  ok      Schema-2 main (Kind-Matrix bestaetigt) -> main
  ok      Schema-2 legacy -> legacy
  ok      Vorbereitung: erst main
  ok      unbekanntes Major danach -> zurueck auf unclassified
  ok      Muell (ignoriert) aendert nichts - main bleibt main
  ok      Muell auf einen frischen Automaten klassifiziert nicht
  ok      explizite Initialisierung OHNE Editor: abgelehnt
  ok      ... und der Automat bleibt neutral
  ok      der geoeffnete Editor allein klassifiziert NICHT
  ok      Editor offen + explizite Initialisierung: angenommen
  ok      ... -> main
  ok      main + offener Editor darf den Broker starten
  ok      Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)
  ok      main OHNE offenen Editor darf NICHT starten
  ok      Rueckweg: derselbe Akt fuehrt von main zurueck
  ok      ... -> legacy
  ok      legacy darf nie starten - auch mit Editor nicht
  ok      legacy ist wieder audio-neutral
  ok      Sonde (NkPr) frisch: neutral trotz fester Produktklasse
  ok      Sonde (NkPr): gueltiger eigener Stand -> passive_probe
  ok      Sonde (NkPr): kein Brokerstart
  ok      Sonde (NkPr): auch mit offenem Editor kein Brokerstart
  ok      Sonde (NkPr): kein Akt macht sie zum Main
  ok      Sonde (NkAc): gueltiger eigener Stand -> active_probe
  ok      Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main

[2] Verdrahtet: EqCopilotProcessor (Gen)
  ok      Scannerlauf: klassifiziert NICHT (53.5)  [unclassified]
  ok      Scannerlauf: kein Brokerstart
  ok      Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)
  ok      Scannerlauf: Markierung meldet sich nicht hoerbar
  ok      Altprojekt 'sensor' -> legacy  [legacy]
  ok      legacy mit offenem Editor: kein Brokerstart
  ok      legacy ist 'immer passiv' - kein Sample gefaerbt
  ok      Altprojekt 'hub' -> main  [main]
  ok      main, aber Editor zu: noch kein Brokerstart
  ok      main + offener Editor: Brokerstart erlaubt
  ok      main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)
  ok      Vorbereitung: Rollenwahl aendert den Stand
  ok      Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)  [unclassified]
  ok      der geoeffnete Editor allein holt es auch nicht nach
  ok      Vorbereitung: der User waehlt die Rolle im offenen Editor
  ok      frische Instanz: Editor offen + Rollenwahl 'hub' -> main
  ok      frische Instanz als main: faerbt
  ok      Rueckweg: 'sensor' klassifiziert zurueck auf legacy
  ok      Rueckweg: der Schnitt auf neutral greift sofort
  ok      Vorbereitung: main
  ok      der fremde Stand kommt read-only zurueck
  ok      read-only -> zurueck auf unclassified
  ok      read-only: kein Brokerstart
  ok      read-only: wieder audio-neutral

LEBENSLAUF-TEST OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 32,27 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopPipeClientTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_rc_lib.dir\Release\EqCopPipeClientTest_rc_lib.lib
  EqCopPipeClientTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSunaNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_rc_lib.dir\Release\EqCopSunaNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopSunaNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostProbeTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_rc_lib.dir\Release\EqCopHostProbeTest_rc_lib.lib
  EqCopHostProbeTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSchemaTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_rc_lib.dir\Release\EqCopSchemaTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopAnalysisGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_rc_lib.dir\Release\EqCopAnalysisGoldenTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopAnalysisGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaSuna_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_rc_lib.dir\Release\NakamaSuna_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaSuna.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\Nakama Suna_SharedCode.lib
  NakamaSuna_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaSuna_vst3_helper.exe
  NakamaSuna_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\x86_64-win\Nakama Suna.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaSuna_artefacts/Release/VST3/Nakama Suna.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>

