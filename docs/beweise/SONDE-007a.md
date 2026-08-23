# Beweismanifest — SONDE-007a «Gemeinsamer Kern ohne `JucePlugin_*`-Konstanten»
<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-23 nachgearbeitet -->


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

**Arbeitsteilung der drei Riegel** — der Punkt des Tickets, nicht Redundanz:

| | misst | sieht | sieht **nicht** |
|---|---|---|---|
| **K1** Präprozessor | Baubeschreibung | 46 Makros namentlich, in jeder Kern-Übersetzungseinheit | Makros, die nicht auf der Liste stehen (der Präprozessor kann kein Präfix prüfen) |
| **K2** CMake-Konfigurierzeit | Baubeschreibung | **jedes** `JucePlugin_` per Regex über die ganze Linkhülle | Werte, die erst über einen Generatorausdruck entstehen |
| **K3** Artefakt | die gebaute `.lib` | jeden eingefrorenen Identitätswert als Bytes — auch als Stringliteral, das nie ein Makro war | nichts von dem, was oben steht (er läuft nach dem Bau) |
| **K2b** CMake-Konfigurierzeit | Baubeschreibung | ob Kern und Verbraucher dieselben `JUCE_`-**Defines** über den JUCE-Kopfdateien haben | Übersetzungs**schalter** (dafür kam K2c) · Identität (dafür die drei oben) |
| **K2c** CMake-Konfigurierzeit | Baubeschreibung | ob jedes `juce_recommended_*`-Ziel der Referenz auch am Kern hängt — die **Quelle** der Schalter, nicht einzelne Flags | `lto_flags` (ausgenommen: `/GL` ohne `-LTCG` im Verbraucher) · alles, was kein Empfehlungsziel ist |

K2b ist im Selbstaudit nach dem ersten Commit dazugekommen (§2 B8), **K2c** aus
dem T2-Lauf am 23.08. (§5 T2-1/T2-3, Nacharbeit §6) — bis dahin sagte die
K2b-Zeile hier „mit derselben Konfiguration" und griff damit weiter, als der
Riegel misst. Beide beantworten eine andere Frage als K1–K3: nicht „trägt der
Kern eine fremde Identität?", sondern „bedeutet derselbe JUCE-Header im Kern
dasselbe wie im Verbraucher, und wird er gleich scharf übersetzt?". Die
Rohausgaben in §2 stammen von vor K2c: dort steht die Linkhülle noch mit
**7** statt 9 Zielen.

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

