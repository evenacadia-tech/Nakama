# NAK-309 — Quellvalidierung Teil A (lesender Opus-Agent, HEAD 4c1c7f3c)

**Ticket:** NAK-309 · Planschritt S25h · Etappe 0 „Quellvalidierung", **Teil A**
**HEAD:** `4c1c7f3c8cb5145de0e545aebe6305e450f00f7c` (Zweig `master`)
**Datum:** 18.09.2026 · **Modell:** Opus (lesend)
**IDs (7):** T3-09-01, T3-05-03, T3-02-07, T3-03-11, T3-04-04, T3-09-04, T3-09-05
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (Phasenzeilen,
Skeptikerzeilen Phase 10, Arbeitsübersicht V0/W32/W34) und die Rohberichte unter
`docs/audits/2026-09-15-tiefenaudit/roh/`.
**Muster:** `docs/beweise/roh/NAK-246-quellvalidierung.md`,
`docs/beweise/roh/NAK-283-quellvalidierung.md`.

**Prüfform:** ausschließlich Quellenlektüre am HEAD. Kein Bau, kein Testlauf, kein Kanon,
kein FL Studio, kein Codex, keine Fehlerinjektion. Git nur lesend. Jede Zeilennummer unten
ist am HEAD `4c1c7f3c` nachgelesen. Sicherheitsbefunde (T3-12-\*) sind nicht Gegenstand
dieses Teils; Legacy-Bezeichner (`EQ-Copilot`, `EqCop*`, `Eqcp`) sind kein Befund.

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn des Laufs:* nur die zwei
fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/`, sonst nichts; HEAD
`4c1c7f3c`. *Beim Schreiben dieser Datei:* zusätzlich ` M docs/PLAN-STAND.md` und
`?? docs/beweise/NAK-309.md` — **beides fremde, nicht von diesem Agenten erzeugte Änderungen
einer parallel laufenden Session**; sie wurden nicht angefasst. Gleichzeitig ist der HEAD auf
`3b4e8bd0` gewandert (ein Commit: „Karten U44 bis U52 …"). **Basisabgleich:**
`git diff --stat 4c1c7f3c 3b4e8bd0 -- eq-copilot/ broker/ tools/` ist **leer**; der einzige
Unterschied liegt in `docs/plan/fragen.json`. Keine der unten zitierten Dateien ist betroffen,
alle Zeilennummern gelten unverändert. Diese Datei ist die einzige Schreibaktion des Agenten.

**Ergebnis in einer Zeile je ID**

| ID | Urteil | Kategorievorschlag | Schwere | Rotbeweis fällt an |
|---|---|---|---|---|
| T3-09-01 | BESTÄTIGT (Reichweite breiter als die Befundzeile) | DEFEKT | high | `tools/beweise.ps1:1016` (+ Urteil `:1208-1210`), neues Selbsttest-Bein nach Muster A34/A35 |
| T3-05-03 | BESTÄTIGT | DEFEKT | high | `tools/eq-copilot/pruefe_installer_manifest.py:1382` im Bein A17, erreicht über den `--hashen`-Zweig `:1650-1665` |
| T3-02-07 | PRÄZISIERT (es gibt eine Zusage: die gezählte Abdeckungszahl) | HÄRTUNG + Zahlkorrektur | low | Bein B2 `StateMigrationTestMain.cpp` an der Feldpräsenz des neuen Fixtures; A12-Registerzeile `erzeuge_state_fixtures.py:1084` |
| T3-03-11 | BESTÄTIGT | HÄRTUNG | low | Bein B10 `EqCopIpcTest` an der neuen Minorwache in `Verbindung.cpp:640ff` / `TelemetryClient.cpp:662ff` |
| T3-04-04 | BESTÄTIGT als Härtung (Herabstufung aus Phase 10 trägt) | HÄRTUNG | low | Bein A4 an `broker/src/briefkasten.rs:560-562` (Mutante: Stoppflag entfernen) |
| T3-09-04 | PRÄZISIERT (es gibt eine Zusage: zwei Matrixzeilen NAK-121) | DEFEKT (hilfsweise LÜCKE) | medium | Beine A4 (`tools/beweise.ps1:493`) und A4-SI (`:501`) an `store_crash_matrix.rs:3239` / `:3364` |
| T3-09-05 | PRÄZISIERT (es gibt eine Zusage: die Behauptungszeile des Beins B22) | DEFEKT | low | Bein B22 an `Sonde013FingerprintGoldenTest.cpp:270`, Behauptung `tools/beweise.ps1:725` |

---

## T3-09-01 — Frischeriegel kennt die DSP-Quellen nicht (V0, Bündel B01)

*Befundzeile: `BEFUNDE.md:466` · Skeptiker: `:544` · Arbeitsübersicht: `:802` (V0) ·
Herleitung: `roh/phase-09-beweisluecken.md:28-33`*

### 1. Quellkette, zitiert

**Station 1 — die Frischepopulation des Runners.** `tools/beweise.ps1:956-957` (der
Absichtskommentar, der die Vollständigkeit behauptet) und `:969-984` (die tatsächliche
Liste):

```
# Vollstaendig halten: JEDE Quelle, aus der eine Pruefbinaerdatei entsteht.
# Fehlt ein Ort, beglaubigt der Runner eine veraltete Messung als frisch -
```

```
$quellOrte = @(
    (Join-Path $Wurzel 'eq-copilot\plugin\src'),
    (Join-Path $Wurzel 'eq-copilot\plugin\core'),
    (Join-Path $Wurzel 'eq-copilot\plugin\state'),
    (Join-Path $Wurzel 'eq-copilot\plugin\sonde'),
    (Join-Path $Wurzel 'eq-copilot\plugin\tests'),
    (Join-Path $Wurzel 'eq-copilot\plugin\hostbridge'),
    (Join-Path $Wurzel 'eq-copilot\plugin\vertrag'),
    (Join-Path $Wurzel 'eq-copilot\plugin\hostprobe'),
    (Join-Path $Wurzel 'eq-copilot\plugin\spike'),
    (Join-Path $Wurzel 'eq-copilot\plugin\probe'),
```

`eq-copilot\plugin\dsp` steht nicht darin; die Ordnerliste des Plugins ist
`core dsp hostbridge hostprobe probe sonde spike src state tests vertrag` — genau ein Ordner
mit Produktquellen fehlt.

**Station 2 — die DSP-Quellen sind Kernquellen.** `eq-copilot/plugin/CMakeLists.txt:198-208`:

```
set(NAKAMA_KERN_QUELLEN
    dsp/DspProgramm.cpp
    dsp/DspBankPool.cpp
    dsp/DspKern.cpp
    state/NakamaKanon.cpp
```

`:240` bindet sie zur Static-Lib: `add_library(NakamaKern STATIC ${NAKAMA_KERN_QUELLEN})`.
Dazu die reinen Kopfdateien `dsp/DspFilter.h`, `dsp/DspSvf.h`, `dsp/DspRtWache.h`,
`dsp/DspKern.h`, `dsp/DspBankPool.h`, `dsp/DspProgramm.h` — ebenfalls unerfasst.

**Station 3 — die Frischeentscheidung.** `tools/beweise.ps1:1016-1018`:

```
    $istVeraltet = (-not $bauBestaetigt) `
                   -and ($null -ne $neuesteQuelle -and $datei.LastWriteTime -lt $neuesteQuelle)
    if ($istVeraltet) { $veraltet = $true }
```

`$neuesteQuelle` ist ausschließlich das größte `LastWriteTime` über `$quellOrte`
(`:985-996`). Eine reine DSP-Änderung bewegt diesen Wert nicht.

**Station 4 — das Urteil.** `tools/beweise.ps1:1208-1211`:

```
elseif ($veraltet) {
    $exitcode = 4
    $urteil = "NICHT BEGLAUBIGT - $($gruen.Count)/$($gelaufen.Count) gruen, aber Pruefbinaries sind aelter als die Quellen$nachsatz"
}
```

Bleibt `$veraltet` falsch, steht dort `GRUEN` (`:1212-1214`).

**Auslöser (wer ruft wann).** Vollständiger frischer Bau; danach eine Änderung
ausschließlich unter `eq-copilot/plugin/dsp/`; danach der ausdrücklich erlaubte Lauf **ohne**
`-Bauen` (`tools/beweise.ps1` kennt `-Bauen` als Schalter, nicht als Pflicht). Der Baustand
wird bei `:1005-1025` **vor** dem Kanon (`:1027ff`) gerechnet — A14 läuft also später und
kann das Urteil nicht mehr beeinflussen.

**Erreichbarkeit: Stufe 1.** Der Runner ist das heutige Kanonwerkzeug; beide Läufe (mit und
ohne `-Bauen`) sind freigegeben, und der Fall braucht weder Sabotage noch eine ungewöhnliche
Bedienung.

**Reichweite — enger gefasst in der Befundzeile, als sie ist.** Die Befundzeile nennt
B6/B7. Tatsächlich bindet `nakama_kern_anbinden(...)` (`eq-copilot/plugin/CMakeLists.txt:277`,
41 Aufrufstellen) **jedes** Plugin-Ziel an `NakamaKern` — darunter A1 `EqCopNullTest`
(`:641`), A3 `EqCopMarkierungTest` (`:675`), A16 (über die Zielschleife `:365`/`:412`/`:444`),
B1 `EqCopIdentityTest` (`:775`), B2 `EqCopStateMigrationTest` (`:796`), B4 `EqCopQueueStressTest`
(`:956`), B10 `EqCopIpcTest` (`:975`), B22 (`:1033`), B5 (`:1135`), B6 (`:1154`), B7 (`:1176`),
B3c (`:1271`) und das Produktziel `EqCopilot` (`:291`). Betroffen ist damit die gesamte
`Art='plugin'`-Familie, nicht nur die zwei DSP-nahen Beine.

### 2. Zusage, wörtlich

`CLAUDE.md:212-214`:

> „Der Runner baut, fährt die deklarierten Beine, legt Rohausgaben unter `docs/beweise/roh/`
> ab (Manifest trägt Kopf, Urteilszeile, Übersicht) und **verweigert die Beglaubigung, wenn
> Prüfbinaries älter sind als ihre Quellen**."

Zweite, engere Zusage im Werkzeug selbst — `tools/beweise.ps1:956`:

> „Vollstaendig halten: JEDE Quelle, aus der eine Pruefbinaerdatei entsteht."

Beide brechen: `plugin/dsp/` **ist** eine solche Quelle, und die Beglaubigung wird nicht
verweigert. Der Kommentar `:962-968` hält denselben Fehler bereits zweimal in der Geschichte
fest („DREI Orte fehlten … genau der Fehler, gegen den er errichtet wurde") — die Regel ist
also bekannt und wurde erneut gerissen.

### 3. Heutige Abdeckung

- **Kein Gegenbeweis, sondern Ehrlichkeit ohne Wirkung:** `tools/beweise.ps1:1358-1362`
  druckt die Ortsliste aus `$quellOrte` selbst ins Manifest, statt sie danebenzuschreiben.
  Das Manifest lügt also nicht darüber, *welche* Orte gescannt wurden — es macht die Lücke
  nur nicht zu einem Urteil.
- **`-Bauen` heilt den Lauf, nicht den Riegel:** `:1003` (`$bauBestaetigt`) schaltet den
  Zeitstempelvergleich ab, weil CMake die Abhängigkeiten selbst geprüft hat. Der zweite
  freigegebene Weg bleibt ungeschützt.
- **A14 ist kein Gegenbeweis:** `tools/eq-copilot/pruefe_kern_identitaetsfrei.py:289`
  `… "--target", "NakamaKern"` baut ausschließlich die Bibliothek neu; die bereits gelinkten
  Test-EXE werden nicht neu gelinkt.
- **A32 ist ein anderer Riegel:** `gesundheit.py` inventarisiert `eq-copilot/plugin` rekursiv
  (Behauptungszeile `tools/beweise.ps1:756`) — das ist die Codebase-Messung, nicht die
  Beweisfrische.
- **Register:** keine Zeile deckt diese Auslassung. NAK-25 (zu strenge globale mtimes) und
  NAK-133 (nicht erfasste Python-Helfer) betreffen andere Frischelücken; NAK-249 ist der
  bereits reparierte A32-Umfang.
- **Absichtskommentar, der ausdrücklich erlaubt:** keiner.

### 4. Urteil

**BESTÄTIGT**, mit einer Ausweitung: die Befundzeile ist zu eng, weil alle Plugin-Beine
betroffen sind, nicht nur B6/B7.
**Kategorievorschlag: DEFEKT** (gebrochene Zusage aus `CLAUDE.md:212-214` und aus dem
Werkzeugkommentar `beweise.ps1:956`), **Schwere high** — der Befund entwertet potenziell
jede grüne Meldung ohne `-Bauen`, weshalb die Arbeitsübersicht ihn zu Recht als V0 („vor
Bewertung jeder nachfolgenden grünen Meldung", `BEFUNDE.md:802`) führt. Klasse K5.

### 5. Kleinster Fix und Ort des Rotbeweises

**Fix.** `$quellOrte` nicht um einen weiteren handgeschriebenen Eintrag ergänzen, sondern
nach dem Muster des A32-Inventarriegels ableitbar machen: der Runner liest die tatsächlich
übersetzten Projektquellen (`NAKAMA_KERN_QUELLEN` plus die je Ziel gelisteten `target_sources`
aus `eq-copilot/plugin/CMakeLists.txt`) und meldet jede Quelldatei, die weder gescannt noch
mit Grund ausgenommen ist, als **Werkzeugfehler** statt still zu übergehen. `plugin/dsp/`
fällt damit automatisch hinein.

**Rotbeweis.** Die Zusage trägt `tools/beweise.ps1:1016` (`$istVeraltet`) und, davon
abhängig, die Urteilszeile `:1208-1210` (Exit 4). Da kein Bein den Runner selbst misst, gehört
der Rotbeweis in ein **neues Selbsttest-Bein nach dem Muster A34/A35** (`tools/beweise.ps1:616`
bzw. `:618`, beide „ohne Repo-Fixture, jede Erwartung mit ihrem Gegenteil"): es baut im
Speicher einen Baustand, in dem genau eine DSP-Quelle jünger ist als ein Prüfbinary, und
verlangt Exit 4. **Vor dem Fix** meldet dieser Fall `GRUEN` → das Selbsttest-Bein ist rot.
**Nach dem Fix** meldet er `NICHT BEGLAUBIGT` an `:1209` → grün. Die Gegenprobe (Quelle
älter als das Binary) muss weiterhin grün bleiben, sonst wäre der Riegel nur überstreng
(NAK-25).

---

## T3-05-03 — Broker-Pin über Bau und Installation (W32, Bündel B03)

*Befundzeile: `BEFUNDE.md:413` · Skeptiker: `:529` · Arbeitsübersicht: `:835` (W32) ·
Herleitung: `roh/phase-05-hostkante.md:47-57`*

### 1. Quellkette, zitiert

**Station 1 — CMake bindet den Broker-Pin aus dem Manifest in einen Produktheader.**
`eq-copilot/plugin/CMakeLists.txt:39-43` und `:56-57`:

```
set(NAKAMA_INSTALLER_MANIFEST
    "${CMAKE_CURRENT_SOURCE_DIR}/../install/nakama-installer-v1.json")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${NAKAMA_INSTALLER_MANIFEST}")
file(READ "${NAKAMA_INSTALLER_MANIFEST}" NAKAMA_INSTALLER_JSON)
```

```
        string(JSON NAKAMA_BROKER_SHA256 GET "${NAKAMA_INSTALLER_JSON}"
               artefakte ${NAKAMA_ARTEFAKT_INDEX} sha256)
```

`:86-89` erzeugt daraus den Header:

```
set(NAKAMA_BROKER_BINDING_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/nakama")
file(MAKE_DIRECTORY "${NAKAMA_BROKER_BINDING_DIR}")
configure_file(core/ipc/BrokerInstallBinding.h.in
               "${NAKAMA_BROKER_BINDING_DIR}/BrokerInstallBinding.h" @ONLY)
```

Vorlage `eq-copilot/plugin/core/ipc/BrokerInstallBinding.h.in:8-10`:

```
inline constexpr const wchar_t* brokerPfad = L"@NAKAMA_BROKER_INSTALL_PFAD@";
inline constexpr const char* brokerSha256 = "@NAKAMA_BROKER_SHA256@";
inline constexpr const char* authenticodeThumbprint = "@NAKAMA_BROKER_THUMBPRINT@";
```

Verbraucher sind **beide** Apps: `eq-copilot/plugin/src/PluginProcessor.cpp:82` und `:226`,
`eq-copilot/plugin/sonde/SondeProcessor.cpp:66`.

**Station 2 — der Kanon baut die Plugins vor dem Release-Broker.**
`tools/beweise.ps1:886` (C++-Ziele) und `:895-899` (Broker):

```
    $b = Fuehre-Aus -Datei $cmakeBefehl -Argumente (@('--build', 'eq-copilot/build', '--config', 'Release', '--target') + $zuBauen) …
```

```
    $cargoRelease = Fuehre-Aus -Datei 'cargo' -Argumente @(
        'build', '--release', '--manifest-path', 'broker/Cargo.toml',
        '--bin', 'eqcop-broker-v3probe', '--bin', 'eqcop-broker-sonde012-probe',
        '--bin', 'eqcop-broker',
```

**Station 3 — der Laufzeit-Arm friert danach die neuen Bytes ein und installiert.**
`tools/fl/laufzeit.ps1:407-412`:

```
    Log 'Manifest-Hashes nachziehen (--hashen)'
    $h = & $script:U.Py @((Join-Path $script:K.Repo 'tools\eq-copilot\pruefe_installer_manifest.py'), '--hashen')
    foreach ($z in @($h.Zeilen)) { Log "  hashen: $z" }
    if ($h.Exit -ne 0) { return @{ Ok = $false; Ueberspringen = $false; Grund = "--hashen Exit $($h.Exit)" } }
    Log 'Aufgabe \Nakama\installieren starten'
    $r = Aufgabe 'installieren'
```

**Station 4 — `--hashen` schreibt neue Hashes und kehrt vor der Bindungsprüfung zurück.**
`tools/eq-copilot/pruefe_installer_manifest.py:1650` und `:1665`:

```
    if args.hashen:
```
```
        return hashen(manifest)
```

`hashen()` (`:1117`, Schreibpunkt `:1149` / `:1162`) ersetzt `a["sha256"]` jedes Artefakts
durch den Hash des **gebauten** Standes. Die Bindungsprüfung `[4c]` liegt ab `:1359` und wird
in diesem Zweig nie erreicht.

**Station 5 — das installierte Plugin verwirft den neuen Broker.**
`eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:308-313`:

```
    bericht.hashGeprueft = true;
    if (gross (erwarteterSha256) != bericht.dateiSha256)
    {
        bericht.fehler = BrokerPruefFehler::hashFalsch;
        return bericht;
    }
```

**Auslöser (wer ruft wann).** Ein legitimer Lauf, in dem sich Brokerbytes ändern: Kanon mit
`-Bauen` → Plugins tragen Pin H0 aus dem damaligen Manifest, `cargo build --release` erzeugt
Broker H1 → Laufzeit-Arm ruft `--hashen` → Manifest trägt jetzt H1 und die zu H0 gebauten
Bundlehashes → Installation kopiert passende Dateien und meldet für jedes Artefakt „aktuell"
(`laufzeit.ps1:419-424` prüft genau diese Zeilen) → zur Laufzeit vergleicht das Plugin H0
gegen die Datei H1 und erhält `hashFalsch`. `CMAKE_CONFIGURE_DEPENDS` (`:41-42`) wirkt erst
beim **nächsten** Bau; innerhalb desselben Laufs schließt es nichts.

**Erreichbarkeit: Stufe 1.** Bau und Installation über die erhöhten Aufgaben sind der heute
vorgeschriebene Weg (`CLAUDE.md`, Abschnitt „Bauen und beweisen": Laufzeit-Arm nach grünem
Kanon, Installation über `\Nakama\installieren`, „nach grünem Kanon `--hashen`").

### 2. Zusage, wörtlich

`tools/eq-copilot/pruefe_installer_manifest.py:1381-1383` (Block `[4c]`, gefahren im
Kanonbein **A17**, `tools/beweise.ps1:584`):

```
    pruefe(ist == soll,
           "generierter Produktheader entspricht Pfad, Hash und Thumbprint des Manifests",
           f"ist={ist!r} soll={soll!r}")
```

Der Docstring desselben Blocks (`:1355-1358`) formuliert die Absicht noch schärfer:

> „Der Vergleich ist absichtlich NACH dem CMake-Lauf: eine bloss richtige Template-Datei
> beweist nicht, dass ein geaenderter Manifesthash den Build erreicht hat.
> `CMAKE_CONFIGURE_DEPENDS` schliesst genau dieses Stalefenster."

**Was genau bricht:** die Zusage ist nicht falsch formuliert, sie wird **zum falschen
Zeitpunkt gemessen**. `--hashen` verändert genau die Größe, gegen die `[4c]` vergleicht, und
kehrt vor `[4c]` zurück; zwischen der letzten Messung und der Installation liegt eine
Mutation des Manifests, die kein Bein sieht. Das „Stalefenster", das der Docstring als
geschlossen bezeichnet, ist auf dem Installationsweg offen.

### 3. Heutige Abdeckung

- **A17 `[4c]`** misst korrekt — aber nur im Kanon, vor `--hashen`
  (`tools/beweise.ps1:584`, Behauptung nennt `[4]`, `[4b]`, `[5]`, `[6]`, nicht die
  Installationsreihenfolge).
- **A17 `[4]`** vergleicht Artefakthashes im Kanon absichtlich **weich** und erst mit
  `--release` hart (Behauptungszeile `tools/beweise.ps1:584`; Registerzeile NAK-127,
  `docs/offene-punkte.md:164`). Die weiche Stelle ist gewollt und deckt den Fall nicht.
- **A18** (`pruefe_installer_gegenpfad.py`, `tools/beweise.ps1:593`) misst den Gegenpfad
  installieren↔Rückweg mit synthetischen Bytes, nie eine ausgeführte Plugin-Startbindung.
- **`laufzeit.ps1:419-424`** verlangt für jedes Artefakt „aktuell" — das ist ein
  Dateivergleich gegen das **soeben nachgehashte** Manifest und meldet daher konsistent
  „aktuell", obwohl die Bindung im Plugin veraltet ist.
- **Register:** NAK-127 (`docs/offene-punkte.md:164`) ist die nächste Zeile („Vor der
  nächsten Installation müssen Broker-Release-Bau und Manifest-Hash gemeinsam nachgezogen
  werden"). Sie beschreibt die Pflicht, nennt aber **nicht** den hier neuen Punkt: dass das
  Nachziehen selbst eine zuvor passende Auslieferung auseinanderziehen kann, weil der Pin
  einkompiliert ist.
- **Kein Absichtskommentar erlaubt den Fall**; `CMakeLists.txt:34-38` begründet den
  einkompilierten Pin ausdrücklich als Sicherheitsgewinn („unvollstaendige oder mehrdeutige
  Brokerangaben sind ein Konfigurationsfehler statt ein spaeterer unsicherer Vorgabewert").

### 4. Urteil

**BESTÄTIGT.** Die Kette ist am HEAD lückenlos nachlesbar; der Audit behauptet zu Recht
keinen aktuell falschen installierten Stand, sondern den nächsten Änderungsfall.
**Kategorievorschlag: DEFEKT** (gebrochene Zusage `[4c]`, gemessen zum falschen Zeitpunkt),
**Schwere high** — die Folge ist ein installierter Stand, in dem Gen den Broker-Spawn
verweigert und auch die Serverauthentisierung den alten Pin benutzt, während Installer und
Dateivergleich „aktuell" melden. Klasse K4/K5 (Vertrag zwischen Bau, Manifest und
Installation).

### 5. Kleinster Fix und Ort des Rotbeweises

**Fix (zwei Sätze, ein Änderungssatz).** (a) In `tools/beweise.ps1` den Release-Broker vor
den C++-Zielen bauen, damit der Pin, den `configure_file` einbrennt, schon aus dem Manifest
der endgültigen Brokergeneration stammt; (b) `--hashen` nicht vor `[4c]` verlassen, sondern
die Bindungsprüfung **gegen das frisch geschriebene Manifest** laufen lassen, sodass ein
Plugin-Pin, der nicht mehr zum eingefrorenen Broker passt, die Installation blockiert statt
sie zu passieren.

**Rotbeweis.** Die Zusage trägt `tools/eq-copilot/pruefe_installer_manifest.py:1382`; im
Kanon hängt sie am Bein **A17**. **Vor dem Fix:** ein Manifest, dessen Broker-SHA-256 vom
`brokerSha256`-Literal des generierten Headers abweicht, verlässt `--hashen` mit Exit 0
(`:1665`), und `laufzeit.ps1:410` lässt den Lauf weiter — der Gegenfall ist also grün, obwohl
er rot sein müsste. **Nach dem Fix** fällt genau `pruefe(...)` in `:1381-1383` mit
`ist != soll`, `--hashen` endet ungleich 0, der Laufzeit-Arm bricht mit „--hashen Exit …"
ab; mit korrekter Baureihenfolge (Broker → Manifest → Plugins) ist dieselbe Zeile grün. Der
Rotbeweis fällt damit an der Zusagezeile, nicht an einem Nebeneffekt wie dem Exitcode des
Installers.

---

## T3-02-07 — Eingefrorener `Common.project_binding_id`-Vektor fehlt (W34, Bündel B33)

*Befundzeile: `BEFUNDE.md:368` · Skeptiker: `:511` · Arbeitsübersicht: `:837` (W34) ·
Herleitung: `roh/phase-02-zustand.md:117-125`*

### 1. Quellkette, zitiert

**Station 1 — das Feld ist persistent und wird vom Produktwriter geschrieben.**
`eq-copilot/plugin/state/NakamaState.cpp:33` und `:823-824`:

```
const juce::Identifier kBinding     ("project_binding_id");
```
```
    if (z.common.projectBindingId.isNotEmpty()) common.setProperty (kBinding, z.common.projectBindingId, nullptr);
    else                                        common.removeProperty (kBinding, nullptr);
```

Der Leser validiert es hart, `:1515`:

```
        grund = "Common.project_binding_id must be lowercase hex32"; return false;
```

**Station 2 — die Migration erfindet es bewusst nicht.** `:2079`:

```
    // project_binding_id wird NICHT erfunden (§32.2).
```

**Station 3 — die eingefrorene Goldenliste kennt es nicht.**
`tools/eq-copilot/erzeuge_state_fixtures.py:1084-1085`:

```
    for datei in ("aus-schema1-sensor", "aus-schema1-hub", "aus-schema1-pre", "aus-schema1-post",
                  "fremdes-major-3", "main-intent-v1", "dsp-v2-voll", "layout-v1"):
```

Der Bezeichner `project_binding_id` kommt in der ganzen Generatordatei nicht vor
(`grep` über `tools/eq-copilot/erzeuge_state_fixtures.py`: 0 Treffer). **Eigene
Nachmessung an den Bytes:** alle acht Dateien unter `eq-copilot/fixtures/state/schema2/`
enthalten die Zeichenkette `project_binding_id` **null**mal.

**Station 4 — die zwei Tests, die es heute berühren.**
`eq-copilot/plugin/tests/StateMigrationTestMain.cpp:1013` (Negativseite, gewollt):

```
            pruefe (z.common.projectBindingId.isEmpty(), juce::String ("'") + r.name + "': project_binding_id NICHT erfunden");
```

`eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:2639-2640` (Positivseite, aber als
Prozessor-Roundtrip, nicht als eingefrorene historische Eingabe):

```
    const bool stateWahr = z.common.klasse == nakama::state::Klasse::main
        && z.common.projectBindingId.isNotEmpty()
```

**Auslöser.** Kein Produktpfad — der Befund betrifft die Beweisfläche: kein eingefrorener
Altstand trägt eine gesetzte Projektbindung, also misst kein Test, dass ein **historisch
geschriebenes** Byte-Bild mit gesetztem Feld heute noch korrekt gelesen wird.

**Erreichbarkeit: Stufe 1** für die Beweislücke (A12 und B2 laufen in jedem Kanon,
`tools/beweise.ps1:552` bzw. B2 ab Phase P1); Stufe 4 für den Schaden — das Regressionsbild
entsteht erst, wenn jemand die Serialisierung des Feldes ändert.

### 2. Zusage, wörtlich

Die Befundzeile sagt „keine Zusage". **Das ist zu kurz.** Es gibt eine gezählte,
veröffentlichte Abdeckungszusage — `docs/gesundheit/abdeckungskarte.md:135-136`:

> „**Population:** State v2 mit 136 persistenten Feldern (Common 7, MainProject 8,
> Parameters 114, Dsp 6, Wurzel 1), **133 mit eingefrorenem Fixture**, 19 mit
> Migrationszweig"

Dieselbe Zahl steht als Zusammenfassung in `docs/gesundheit/KONZEPT.md:141`
(„StateMigrationTest (133 von 136 Feldern mit eingefrorenem Fixture …)"), und `KONZEPT.md:149-151`
erklärt die Karte ausdrücklich zur Wahrheit dieser Tabelle. Die Karte rechnet 136 − 3
(die drei NAK-267/A-3-Felder) = 133. Ist `project_binding_id` ein vierter fehlender Ort,
lautet die richtige Zahl **132**. Die Zusage ist also keine Verhaltenszusage, sondern eine
**Messzahl, die um eins zu hoch steht** — ein K5-Punkt („Zahlen gemessen, nicht
abgeschrieben") an einem K4-Gegenstand.

*Einschränkung meiner eigenen Messung:* ich habe die Population von 136 benannten
Property-Orten **nicht** nachgezählt und stütze die Zahl auf `roh/phase-02-zustand.md:127-132`
(Zählregel 1 Root + 7 Common + 8 MainProject + 114 Parameters + 6 Dsp). Unabhängig davon
nachgemessen und belegt ist: **kein** eingefrorenes schema2-Fixture trägt das Feld.

Eine Verhaltenszusage bricht **nicht**: `CLAUDE.md` („State bleibt verlustfrei", „Save und
Load gemeinsam testen") ist durch den Prozessor-Roundtrip erfüllt; A12 verspricht
Bytegleichheit des Korpus, keine Feldabdeckung (Behauptungszeile `tools/beweise.ps1:552`).

### 3. Heutige Abdeckung

- **Positiver Roundtrip:** `Sonde012ProjectReloadTest.cpp:2639-2640` (Bein B14,
  `tools/beweise.ps1:718`) — echter Prozessor, gesetzte Bindung überlebt Save/Load.
- **Negativfall:** `StateMigrationTestMain.cpp:1013` (Bein B2) — Migration erfindet nichts.
- **Absichtsquelle:** `eq-copilot/schemas/state/nakama-state-v2.md` §32.2, gespiegelt im
  Code `NakamaState.cpp:2079`.
- **Register:** **NAK-267** (`docs/offene-punkte.md:277`, A-3) führt genau dieses Muster
  bereits für `confirmed_members_v1`, `manual_passages_v1`, `assistant_step_v1` als Härtung
  („Kanon-Roundtrips, aber kein eingefrorenes Fixture und keinen Feldmengen-Test") und
  ordnet sie einem Pflegeschritt zu. `project_binding_id` gehört sachlich in dieselbe Zeile.
- **Bekannte Klassenlücke, ausdrücklich benannt:** `abdeckungskarte.md:146-150` — „Es gibt
  keinen Test, der die Feldmenge eines Zustandsobjekts abgleicht … Ein neues
  MainProject-Feld ohne Fixture liefe durch."

### 4. Urteil

**PRÄZISIERT.** Der Sachverhalt ist bestätigt (kein eingefrorenes Fixture trägt das Feld;
der Roundtrip ersetzt keine historische Eingabe), aber die Befundzeile „keine Zusage" ist
falsch: die Abdeckungskarte trägt eine konkrete Zahl, die um eins zu hoch steht.
**Kategorievorschlag: HÄRTUNG, Schwere low** für das fehlende Fixture — plus die
**Zahlkorrektur** in `docs/gesundheit/abdeckungskarte.md:136` und `docs/gesundheit/KONZEPT.md:141`
als Pflichtteil desselben Änderungssatzes (sonst bleibt eine gemessene Doku-Zahl falsch, und
das ist nach K5 ein Befund für sich). Enger als die Befundzeile gilt außerdem: der Fall
gehört sachlich unter NAK-267/A-3 und braucht keine eigene neue Regel, nur einen vierten
Eintrag.

### 5. Kleinster Fix und Ort des Rotbeweises

**Fix.** Ein vom **Produktwriter** erzeugtes viertes Writer-Golden (Muster `main-intent-v1`)
mit gesetzter `project_binding_id` einfrieren, in die Goldenliste
`tools/eq-copilot/erzeuge_state_fixtures.py:1084-1085` und in
`eq-copilot/fixtures/state/MANIFEST.json` aufnehmen und in `StateMigrationTestMain.cpp` neben
dem bestehenden Negativfall (`:1013`) die **Feldpräsenz** aus diesem eingefrorenen Byte-Bild
prüfen; die Zahl in der Abdeckungskarte im selben Satz auf den gemessenen Wert ziehen.

**Rotbeweis.** Die Zusage trägt zwei Zeilen: die Registrierung
`erzeuge_state_fixtures.py:1084` (A12, `tools/beweise.ps1:552`, „Korpus samt MANIFEST
bytegleich zur Neuerzeugung, keine verwaiste Datei") und die neue Präsenzprüfung in
`StateMigrationTestMain.cpp` (B2). **Vor dem Fix** existiert die Fixture-Datei nicht; die
neue Prüfung läuft ins Leere → B2 rot. **Nach dem Fix** liest sie das eingefrorene Byte-Bild
und findet die gesetzte Bindung → grün, und A12 bestätigt zusätzlich, dass das Golden
bytegleich neu erzeugt wird. Gegenprobe gegen eine Tautologie: eine Mutante, die
`NakamaState.cpp:823` zu einem bedingungslosen `removeProperty` ändert, muss genau diese
neue Zeile brechen — der vorhandene Roundtrip allein reicht dafür nicht, weil er auf
demselben Writer steht.

---

## T3-03-11 — Welcome-Minor-Grenze in beiden C++-Handschlägen (W34, Bündel B34)

*Befundzeile: `BEFUNDE.md:388` · Skeptiker: `:522` · Arbeitsübersicht: `:837` (W34) ·
Herleitung: `roh/phase-03-vertrag.md` (Abschnitt T3-03-11)*

### 1. Quellkette, zitiert

**Station 1 — der Envelope-Leser prüft Major, nicht Minor.**
`eq-copilot/plugin/core/ipc/WireEnvelope.cpp:133-146`:

```
    const std::uint8_t schemaMajor  = kopfBytes[2];
    const std::uint8_t schemaMinor  = kopfBytes[3];
…
    if (schemaMajor != kSchemaMajor)
        menge |= alsBit (Verstoss::schemaMajorUnbekannt);
```

Der Minor wird nur durchgereicht (`:187`: `u.kopf.schemaMinor = schemaMinor;`). Das ist
vertragsgemäß — Minor ist die vorwärtskompatible Achse, der Deckel gehört zum Verbraucher.

**Station 2 — der Control-Handschlag prüft Familie und Inhalt, nicht den Minor.**
`eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:637-646` und `:664-671`:

```
        if (e.art == StromLeser::Art::frame)
        {
            if (e.kopf.familie != Familie::p0)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "welcome kam nicht als P0";
                break;
            }
```
```
            if (! welcomeHaeltVertrag (felder, linkId, challenge, brokerEpoch, brokerVersion))
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "unerwartete Antwort auf hello";
                break;
            }
            welcomeKam = true;
```

**Station 3 — der Telemetrie-Handschlag spiegelbildlich.**
`eq-copilot/plugin/core/ipc/TelemetryClient.cpp:662-668` (Familie), `:690`
(`welcomeHaeltVertrag`). Kein Minorvergleich in beiden Schleifen.

**Station 4 — die spätere Strenge, die den Handschlägen fehlt.**
`Verbindung.cpp:804-811` (Control-Eingang **nach** dem Welcome):

```
            if (e.kopf.schemaMinor > kJsonSchemaMinor)
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler =
                    "Envelope schema_minor ist neuer als der JSON-Leser — wird geschlossen";
                ++zustand.envelopeAbweisungen;
                return false;
            }
```

`eq-copilot/plugin/src/SourcesModel.cpp:1531-1535` (P2-Verbraucher):

```
    if (schemaMinor > nakama::ipc::kFeatureBatchSchemaMinor)
    {
        fehler = "FeatureBatch schema_minor is newer than this Main reader";
        return false;
    }
```

**Auslöser.** Ein Peer, der ein inhaltlich gültiges Welcome mit einem Envelope-Minor über
`kJsonSchemaMinor = 5` (`WireEnvelope.h:81`) sendet. Der eigene Broker tut das nie:
`broker/src/transport/server_v3/mod.rs:204-206` pinnt `P0_SCHEMA_MINOR = 5`,
`P1_SCHEMA_MINOR = 5`, `P2_SCHEMA_MINOR = 2`.

**Erreichbarkeit: Stufe 2** — gebauter API-Vertrag ohne heutige Bedienung: der Pfad ist
gebaut und erreichbar, aber kein heutiger Produktpeer erzeugt die Eingabe; sie entsteht erst
mit einem neueren oder fremden Broker.

### 2. Zusage, wörtlich

**Keine Zusage gefunden.** Weder `eq-copilot/schemas/v3/README.md` noch der Gate-Text in
`docs/plan/plan.json` noch eine Beinbehauptung formuliert eine allgemeine
Welcome-Minor-Politik. Die nächstliegenden Beine sagen etwas anderes: **A5**
(`tools/beweise.ps1:520`) misst den Fassungsschritt am Schema, **B10** (`:711`) misst
„Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken" — keine Welcome-Minorgrenze.

Statt einer Zusage gibt es eine **Registerzeile, die den Fall ausdrücklich als Härtung
einordnet** — `docs/offene-punkte.md:182` (NAK-154):

> „(2) `eq-copilot/plugin/core/ipc/TelemetryClient.cpp:508` reicht `e.kopf.schemaMinor`
> ungeprüft an den Frame-Callback; ein Cap auf den bekannten Minor fehlt. **Beide sind
> Härtung ohne gebrochene Produktzusage.**"

**Welche Regel fehlen würde:** „Beide C++-Handschläge lehnen ein Welcome ab, dessen
Envelope-`schema_minor` über der höchsten vom jeweiligen Leser beherrschten Fassung liegt,
bevor der Zustand `verbunden` erreicht wird."

### 3. Heutige Abdeckung

- **Inhaltlich vollständig geprüft** ist das Welcome selbst:
  `controlclient/Vertrag.cpp:198-215` (`welcomeHaeltVertrag`) verlangt die Feldmenge exakt
  (`feldmengeGenau`, `additionalProperties:false`), jeden Typ und jede Länge. Eine unbekannte
  Nutzlast wird also nicht verarbeitet — nur der Versionsdeckel fehlt.
- **Downstream geschützt:** `Verbindung.cpp:804` (Control) und `SourcesModel.cpp:1531` (P2).
  Der von NAK-154 genannte ungedeckelte Durchreichepunkt `TelemetryClient.cpp:531`
  (`beiFrame (e.payload, e.payloadLaenge, e.kopf.schemaMinor);`) besteht am HEAD weiterhin,
  ist aber am Verbraucher abgefangen.
- **Fixture, das nicht einspringt:** `p0-schema-minor-unbekannt.bin` ist absichtlich gültig
  und ersetzt die Produktprüfung nicht (`BEFUNDE.md:388`).
- **Register:** NAK-154 (`docs/offene-punkte.md:182`) deckt den Geschwisterfall und stuft
  die Familie als Härtung ein.

### 4. Urteil

**BESTÄTIGT.** Beide Handschläge tragen keinen Minordeckel; die Strenge existiert nur
später.
**Kategorievorschlag: HÄRTUNG, Schwere low.** Begründung gegen eine höhere Stufe: kein
heutiger Peer erzeugt die Eingabe (Broker-Minor gepinnt), und das Welcome ist inhaltlich
feldgenau geprüft — es gibt also keinen Pfad, auf dem eine unbekannte Fassung heute etwas
Zerstörendes bewirkt. Enger als die Befundzeile gilt: der Mangel ist eine
**Konsistenzlücke zwischen zwei Verbraucherklassen** derselben Verbindung, kein
Vertragsbruch. Klasse K3.

### 5. Kleinster Fix und Ort des Rotbeweises

**Fix.** In beiden Welcome-Schleifen vor `welcomeKam = true` denselben Deckel setzen, den
der Control-Eingang schon führt (`e.kopf.schemaMinor > kJsonSchemaMinor` → Verbindung
schließen, `envelopeAbweisungen` zählen), und die Gegenprobe „höchster erlaubter Minor wird
angenommen" mitschreiben, damit der Deckel nicht zur Fassungssperre wird.

**Rotbeweis.** Die Zusage trägt nach dem Fix die neue Bedingungszeile in
`Verbindung.cpp` (Einfügepunkt zwischen `:640` und `:664`) bzw. `TelemetryClient.cpp`
(zwischen `:662` und `:690`); im Kanon hängt sie am Bein **B10** `EqCopIpcTest`
(`tools/beweise.ps1:711`), das bereits einen selbst gehosteten Gegenserver fährt.
**Vor dem Fix** erreicht ein Welcome mit `schema_minor = kJsonSchemaMinor + 1` den Zustand
`verbunden` — der neue Testfall, der `getrennt` erwartet, ist rot. **Nach dem Fix** ist er
grün, während der Fall `schema_minor = kJsonSchemaMinor` weiterhin verbindet. Beide Hälften
(Control und Telemetrie) brauchen denselben Gegenfall, sonst bliebe die Asymmetrie ungemessen.

---

## T3-04-04 — Diagnoseabschluss nach Stop im Broker-Briefkasten (W34, Bündel B35)

*Befundzeile: `BEFUNDE.md:401` (ursprünglich medium/LÜCKE) · **Skeptiker Phase 10:
`:526` HERABGESTUFT auf low/HÄRTUNG** · Arbeitsübersicht: `:837` (W34) ·
Herleitung: `roh/phase-04-lebenslauf.md` (Abschnitt T3-04-04)*

### 1. Quellkette, zitiert

**Station 1 — das Stoppflag wird vor jedem Takt geprüft, nicht innerhalb eines Takts.**
`broker/src/briefkasten.rs:558-571`:

```
        loop {
            // Das Stoppflag vor jedem Takt und nach jedem Wecken (M-45).
            if weck.stopp {
                break;
            }
            let jetzt = Instant::now();
            if weck.ausloesungen > 0 || jetzt >= frist_ende {
…
                drop(weck);
                self.takt();
```

**Station 2 — der Schreibschritt innerhalb des Takts.** `takt()` `:594-605` ruft `rumpf()`
`:607`; der Antwortschreiber `schreibe()` `:727-751`, Schreib- und Abschlussschritte
`:743` und `:750`:

```
        // (3) schreiben, spuelen, schliessen.
        let geschrieben = dateisystem.schreibe_und_spuele(&mut datei, umschlag);
        drop(datei);
        // (4) Groesse: groesser 0 und gleich der geschriebenen Bytes.
        if !geschrieben || dateisystem.groesse(&temp) != Some(umschlag.len() as u64) {
            return false;
        }
        // (5) ohne Ersetzen in den Antwortnamen. (6) kein Aufraeumer.
        dateisystem.benenne_um_ohne_ersetzen(&temp, &ziel)
```

Zwischen `:743` und `:750` steht keine weitere Stoppprüfung.

**Station 3 — `stoppen()` gibt den Thread nach zwei Sekunden frei.**
`:874-891`, Frist `JOIN_FRIST = Duration::from_secs(2)` (`:63`):

```
        let beginn = Instant::now();
        let mut weck = sperre(&kern.weck);
        while !weck.beendet && beginn.elapsed() < JOIN_FRIST {
…
        } else {
            // Frist verstrichen: der Thread bleibt sich selbst ueberlassen. Er haelt nur
            // geteilten Speicher und prueft das Stoppflag vor jedem Takt.
            zaehle(&kern.zaehlwerk.join_frist_verfehlt);
            drop(thread);
        }
```

**Auslöser.** Eine gültige Anfrage läuft bis `schreibe_und_spuele` (`:743`) und hängt dort
länger als zwei Sekunden (langsames oder blockiertes Dateisystem); `stoppen()` setzt das Flag
(`:868`), wartet die Frist ab, zählt `join_frist_verfehlt` und lässt den Thread los; der
Schreibschritt läuft danach durch `:746-750` und veröffentlicht die fertige Antwort **nach**
Rückkehr von `stoppen()`. Aufgerufen wird `stoppen()` aus `Drop for BriefkastenGriff`
(`:894-897`) und aus `lebenslauf::geordnet_stoppen`.

**Erreichbarkeit: Stufe 1** für den Pfad (jedes Brokerende), Stufe 3 für die Beobachtung —
sie verlangt eine Dateisystemfassade, die den Schreibschritt über die Frist hält.

### 2. Zusage, wörtlich

**Keine Zusage gefunden**, und der Code **erlaubt den Nachlauf ausdrücklich** —
`broker/src/briefkasten.rs:887-888`:

```
            // Frist verstrichen: der Thread bleibt sich selbst ueberlassen. Er haelt nur
            // geteilten Speicher und prueft das Stoppflag vor jedem Takt.
```

Damit trägt der Absichtskommentar genau die Herabstufung aus Phase 10 (`BEFUNDE.md:526`):
Der Nachlauf ist real, aber gewollt; es gibt keinen belegten Use-after-free, keinen
Fristbruch und keinen weiteren Takt. `Arc` schützt den geteilten Besitz.

**Welche Regel fehlen würde:** „Nach Ablauf der Join-Frist darf ein bereits begonnener
Briefkastentakt seinen Schreibschritt genau einmal zu Ende führen; er beginnt keinen neuen
Takt, und der zulässige Restabschluss ist mit einer eigenen Wache gemessen."

### 3. Heutige Abdeckung

- **M-45 (2)** `nach_stopp_keine_lieferung` (`broker/src/briefkasten.rs:1791-1806`) misst den
  benachbarten, aber anderen Fall: nach `stoppen()` werden **zehn neue** Auslösungen
  angestoßen, und der Test verlangt 0 Existenzprüfungen, 0 Antworten und einen unberührten
  Kanarienwert. Ein **laufender** Schreibschritt wird nicht gehalten.
- **M-45 (1)** `stopp_weckt_ohne_wartedurchlauf` (`:1776-1789`) misst, dass der Stoppweg
  weckt und die Join-Frist eingehalten wird.
- **M-45 (3)** `stoppreihenfolge_briefkasten_zuerst` (`:1817ff`) misst die Reihenfolge des
  geordneten Stopps.
- **Bein:** A4 (`tools/beweise.ps1:490`), dessen Behauptung den Briefkasten-Lebenslauf nicht
  ausdrücklich nennt.
- **Register:** NAK-286 ist die Herkunft (Laufzeit-Arm und Briefkasten); keine Zeile
  beschreibt den Abschlusszeitpunkt.
- **Arbeitsübersicht, ausdrücklich abgegrenzt:** `BEFUNDE.md:812` (W09) warnt, den Fall
  **nicht** mit dem Provider-/Panel-Besitzproblem T3-04-01/T3-04-03 gleichzusetzen; `:837`
  (W34) hält fest, der erlaubte Nachlauf brauche **keine** erneut offene Produktfrage.

### 4. Urteil

**BESTÄTIGT als Härtung** — die Herabstufung aus Phase 10 trägt am HEAD: der
Absichtskommentar `:887-888` erlaubt den Restthread, der Stoppcheck vor jedem Takt (`:560-562`)
verhindert neue Takte, und `Arc` verhindert einen Zugriff auf freigegebene Besitzer.
**Kategorievorschlag: HÄRTUNG, Schwere low.** Präzisierung gegenüber der ursprünglichen
Befundzeile (medium/LÜCKE): es fehlt **keine Regel für das Verhalten**, sondern nur die
**Messung des bereits erlaubten Verhaltens** — Abschluss genau einmal, kein neuer Takt,
keine unbegrenzte Restarbeit. Klasse K2.

### 5. Kleinster Fix und Ort des Rotbeweises

**Fix.** Einen Lebenslauftest ergänzen, der über die Dateisystemfassade den Schritt
`schreibe_und_spuele` (`:743`) über die Join-Frist hält, dann `stoppen()` ruft und nach der
Freigabe verlangt: genau eine fertige Antwort, `takte_fertig` unverändert, `join_frist_verfehlt == 1`
und keine weitere Existenzprüfung. Zusätzlich die erlaubte Restarbeit als Satz in den
Kommentar bei `:887` schreiben, damit Verhalten und Zusage denselben Umfang haben.

**Rotbeweis.** Die Zusage „kein neuer Takt nach Stop" trägt `briefkasten.rs:560-562`; die
neue Zusage „der begonnene Schreibschritt endet genau einmal" trägt die neue Assertion.
Im Kanon hängt beides am Bein **A4** (`tools/beweise.ps1:490`). **Vor dem Fix** existiert für
den gehaltenen Schreibschritt keine Messung; eine Mutante, die `if weck.stopp { break; }`
(`:560-562`) entfernt, lässt heute `nach_stopp_keine_lieferung` fallen, aber **nicht** den
Abschlussfall — dieser bleibt ungemessen. **Nach dem Fix** bricht dieselbe Mutante zusätzlich
die neue Assertion, und eine zweite Mutante (Stoppprüfung zwischen `:746` und `:750`
einfügen, also den erlaubten Abschluss abwürgen) muss sie ebenfalls brechen. Erst diese zwei
Richtungen machen den Fall zu einer Messung statt zu einer Beschreibung.

---

## T3-09-04 — Junction-Voraussetzung erscheint als bestandener Test (W34, Bündel B32)

*Befundzeile: `BEFUNDE.md:469` · Skeptiker: `:547` · Arbeitsübersicht: `:837` (W34) ·
Herleitung: `roh/phase-09-beweisluecken.md:52-56`*

### 1. Quellkette, zitiert

**Station 1 — erste Probe, stiller Rückweg.**
`broker/tests/store_crash_matrix.rs:3216-3244`:

```
#[cfg(windows)]
#[test]
fn store_weist_reparse_punkt_im_pfad_ab() {
…
    let junction_da = status.map(|s| s.success()).unwrap_or(false) && verweis.exists();
    if !junction_da {
        // Ohne Junction misst der Test nichts - dann sagt er das, statt gruen
        // zu schweigen.
        eprintln!("mklink /J nicht verfuegbar; Reparse-Fall uebersprungen");
        return;
    }
```

Der Rückweg liegt **vor** `store_pfad_ist_remote` (`:3246-3248`).

**Station 2 — zweite Probe, gleicher Rückweg, hinter `#[ignore]`.**
`:3343-3367`:

```
#[cfg(windows)]
#[test]
#[ignore = "A4-SI: Junction-Umhaengen unter einer offenen SQLite-Datenbank"]
fn volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen() {
…
    if !junction_legen(&verweis, &ziel_a) {
        eprintln!("mklink /J nicht verfuegbar; TOCTOU-Fall uebersprungen");
        return;
    }
```

Der Rückweg liegt vor `Connection::open_with_flags` (`:3370`) und vor
`volume_am_sqlite_handle` (`:3385-3386`). Helfer `junction_legen` `:3437-3450`, `false` bei
jedem Fehlschlag.

**Station 3 — die Kanonaufrufe geben erfolgreiche Testausgaben nicht frei.**
`tools/beweise.ps1:490-493` (A4) und `:498-501` (A4-SI):

```
        Kuerzel    = 'A4'
        Name       = 'broker'
        Art        = 'cargo'
        Argumente  = @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never')
```
```
        Kuerzel    = 'A4-SI'
        Name       = 'subscription_server_integration'
        Art        = 'cargo'
        Argumente  = @('test', '--manifest-path', 'broker/Cargo.toml', '--color', 'never', '--test', 'store_crash_matrix', '--', '--ignored', '--test-threads=1')
```

Weder `--nocapture` noch `--show-output`. Der `eprintln!` bleibt im zurückgehaltenen
Testausgabepuffer.

**Station 4 — der Runner wertet nur den Exitcode.** `tools/beweise.ps1:1163-1167`:

```
    if ($lauf.ExitCode -eq 0) {
        $zeile.Symbol = '[OK]'
        $zeile.Status = 'Exit 0'
```

**Auslöser.** Eine Maschinenbedingung: `cmd` nicht auffindbar, `mklink /J` scheitert
(Richtlinie, Dateisystem ohne Reparse-Punkte, belegter Zielname). Kein Eingriff in
Repoquellen nötig. Beide Tests sind auf Windows normalerweise erreichbar — der Kommentar bei
`:3226` nennt die Junction ausdrücklich als „praxisnahen Fall … braucht keine Adminrechte".

**Erreichbarkeit: Stufe 1** — beide Beine laufen in jedem Kanon (`AbPhase='jetzt'`).

### 2. Zusage, wörtlich

Die Befundzeile sagt „keine Zusage". **Das ist zu kurz.** Beide Tests sind namentlich als
Rotbeweis zweier **Matrixzeilen** im Manifest `docs/beweise/NAK-121.md` eingetragen:

`docs/beweise/NAK-121.md:146` (Zeile `G2-TOCTOU-002`), Zusagespalte:

> „Klassifiziert wird das geoeffnete Objekt, nicht ein Vorfahre: … eine vorhandene
> Komponente wird mit Backup-Semantik und Reparse-Punkt-Flag geoeffnet und bei gesetztem
> Reparse-Attribut abgewiesen; die Volumenklassifikation laeuft danach auf dem Handle der
> geoeffneten Datenbank" — Beweisspalte: „neu in `broker/tests/store_crash_matrix.rs`:
> `store_weist_reparse_punkt_im_pfad_ab`"

`docs/beweise/NAK-121.md:1552` (Zeile `R2-3` / D10 / `G2-TOCTOU-002`), Beweisspalte:

> „`volumenentscheidung_haengt_am_sqlite_handle_nicht_am_namen` (neu, A4-SI): Datenbank ueber
> eine Verzeichnis-Junction geoeffnet (`cmd /c mklink /J`, ohne Adminrechte), Junction danach
> auf ein zweites Verzeichnis mit gleichnamiger Datei umgehaengt"

Dazu der **Absichtskommentar im Test selbst**, `store_crash_matrix.rs:3240-3241`:

> „Ohne Junction misst der Test nichts - dann sagt er das, **statt gruen zu schweigen**."

Genau das wird im kanonischen Aufruf nicht eingelöst: der Hinweis erreicht das Manifest
nicht, und der Kanon zählt den Fall als bestanden.

### 3. Heutige Abdeckung

- **A4-SI** (`tools/beweise.ps1:498-501`) behebt die `#[ignore]`-Hälfte, nicht das vorzeitige
  `return`.
- **Keine der beiden Beinbehauptungen** (`:495`, `:503`) nennt die Junction-Fälle, so dass
  auch die Manifestzeile nichts vermisst.
- **Gegenmuster im selben Baum:** Der Fingerprint-/Golden-Stil benutzt bei ähnlich
  formulierten Skips einen Fehlerzähler statt eines stillen `return` (`roh/phase-09-beweisluecken.md:56`).
- **Runner-Vokabular ist vorhanden:** `tools/beweise.ps1:1204-1207` kennt bereits
  `UNVOLLSTAENDIG` mit Exit 3 für „Voraussetzung(en) fehlen"; `Ist-Hinweisexit` (`:1168ff`)
  bietet den nicht blockierenden Weg. Es fehlt nur die Meldung aus dem Rust-Test heraus.
- **Register:** keine Zeile beschreibt diese zwei grünen Voraussetzungsrückwege.

### 4. Urteil

**PRÄZISIERT.** Der Sachverhalt ist bestätigt. Die Einordnung „keine Zusage" gilt jedoch nur
gegen Gate-Texte und Beinbehauptungen — gegen **zwei Matrixzeilen in `docs/beweise/NAK-121.md`**
(`:146`, `:1552`) und gegen den Absichtskommentar `store_crash_matrix.rs:3240-3241` besteht
sehr wohl eine Zusage, und jeder grüne Kanon rezertifiziert diese Zeilen implizit.
**Kategorievorschlag: DEFEKT, Schwere medium** — eine abgenommene Matrixzeile, deren
benannter Rotbeweis ohne Spur ausfallen kann, ist ein K5-Bruch („Runner bezeugt nur, was er
baut … grüner Lauf auf altem Binary"). *Hilfsweise* trägt auch die ursprüngliche Einordnung
**LÜCKE/medium**, wenn der Dirigent die strenge Lesart wählt, dass eine Matrixzeile nur ihren
historischen Rotbeweis bindet und nicht jeden späteren Lauf. Nicht belegt ist ein Fehler des
geschützten Storepfads selbst — dafür gibt es keinen Hinweis.

### 5. Kleinster Fix und Ort des Rotbeweises

**Fix.** Beide Rückwege von `return` auf einen gemeinsamen Helfer umstellen, der die fehlende
Voraussetzung **als Testfehlschlag mit eindeutigem Text** meldet (oder, falls die
Maschinenbedingung toleriert werden soll, über eine eigene Ergebnisdatei/Marke, die der
Runner als „Voraussetzung fehlt" mit Exit 3 in Manifest und Urteilszeile trägt). Ein still
bestandener Gegenfall darf in keinem Fall als grüner Testprozess gezählt werden.

**Rotbeweis.** Die Zusage trägt im Test `store_crash_matrix.rs:3239` (`if !junction_da`) bzw.
`:3364` (`if !junction_legen(...)`); im Kanon hängt sie an **A4** (`tools/beweise.ps1:493`)
und **A4-SI** (`:501`). **Vor dem Fix:** ein Lauf, in dem die Junction-Anlage scheitert
(herstellbar über einen Testschalter, der den `cmd`-Aufruf umlenkt), endet in beiden Beinen
mit Exit 0 und im Manifest mit `[OK]` — der Gegenfall ist grün, obwohl nichts gemessen wurde.
**Nach dem Fix** endet derselbe Lauf mit einem Fehlschlag bzw. Exit 3 und einer sichtbaren
Zeile „Voraussetzung fehlt", während der normale Lauf mit funktionierender Junction die
beiden Proben wirklich fährt und grün bleibt.

---

## T3-09-05 — Fingerprint-Phaseninvarianz endet in `pruefe(true)` (W34, Bündel B36)

*Befundzeile: `BEFUNDE.md:470` · Skeptiker: `:548` · Arbeitsübersicht: `:837` (W34) ·
Herleitung: `roh/phase-09-beweisluecken.md:58-62`*

### 1. Quellkette, zitiert

**Station 1 — die messenden Assertions davor.**
`eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp:233` (Informationsdichte),
`:256-261` (adversariale Ähnlichkeit) und `:267-269` (Nichtleere):

```
        pruefe (byteSumme (f.bandEnergie, Fingerprint::kBaender) > 0
                  && byteSumme (f.chroma, Fingerprint::kChroma) > 0,
                "die Verlaeufe tragen Werte - der Test misst keine leeren Felder");
```

**Station 2 — die Aussage, die nichts misst.** `:270-272`:

```
        pruefe (true,
                "und alle drei sind ENERGIEN ohne Vorzeichen und ohne Phase: selbst "
                "bei perfekter Amplitudenkenntnis fehlt die halbe Information");
```

Der Kopfkommentar `:263-266` kündigt genau diese Eigenschaft an („Kein Byte traegt eine Phase
oder ein Vorzeichen"). Die Bedingung `true` ist vom Fingerprintresultat unabhängig.

**Station 3 — was die Eigenschaft heute strukturell hält.**
`eq-copilot/plugin/core/analysis/featureengine/Vertrag.h:344-347`:

```
    std::uint8_t bandEnergie[kBaender] {};
    std::uint8_t chroma[kChroma] {};
    std::uint8_t onset[kOnsets] {};
```

Vorzeichenfreiheit folgt heute aus dem Typ; **Phasenfreiheit** ist eine semantische Aussage
über die Befüllung und folgt aus keinem Typ.

**Auslöser.** Das Bein **B22** (`tools/beweise.ps1:725`, `AbPhase='P4'`) fährt diesen
Abschnitt in jedem Kanon ab Phase P4. Eine versehentliche Vorzeichen- oder Phasenkodierung
in einem der drei Verläufe ließe genau diese Assertion weiter bestehen.

**Erreichbarkeit: Stufe 1** — die Zeile läuft im heutigen Kanon und geht in jedes Manifest
ein, das B22 enthält.

### 2. Zusage, wörtlich

Die Befundzeile sagt „keine Zusage". **Das ist falsch.** Die Behauptungszeile des Beins
**B22** trägt die Aussage wörtlich — `tools/beweise.ps1:725`:

> „… ein Sinus und ein Dreieck derselben Grundfrequenz sind sich AEHNLICH, obwohl sie
> voellig anders klingen - der Fingerprint ist bewusst nicht injektiv, **und alle drei
> Verlaeufe sind Energien ohne Phase**."

Diese Zeile wird vom Runner in die Übersichtstabelle jedes Manifests geschrieben
(`tools/beweise.ps1:1377-1379`, Spalte „Behauptung"). Damit behauptet jedes Beweismanifest
mit B22 eine Eigenschaft, die der Test nicht misst — genau der Bruch, den K5 benennt:
**„Behauptung ≤ Messung … jede Prüfung einmal gebrochen"** (`docs/gesundheit/KONZEPT.md:142`).

### 3. Heutige Abdeckung

- **Echte Gegenbeweise im selben Abschnitt:** `:233` (76 Byte gegen 204 800 Samples),
  `:254-261` (Sinus vs. Dreieck ähnlich → nicht injektiv), `:267-269` (Felder nicht leer).
  Diese sind nicht wertlos, messen aber jeweils eine andere Eigenschaft.
- **Kein Test** im Repo schickt ein Signal und seine Vorzeichenumkehr durch denselben
  Featurepfad und vergleicht die Ausgabe.
- **Register:** keine Zeile.
- **Absichtskommentar, der ausdrücklich erlaubt:** keiner; der Kommentar `:263-266`
  behauptet die Eigenschaft, er erlaubt nicht, sie ungemessen zu lassen.
- **Kein Privacy-Befund:** aus dem Punkt folgt weder Rekonstruierbarkeit noch ein
  gegenwärtiges Leck; die Feldtypen sind vorzeichenlos.

### 4. Urteil

**PRÄZISIERT.** Der Sachverhalt ist bestätigt; die Einordnung „keine Zusage / HÄRTUNG" hält
nicht, weil die Beinbehauptung `tools/beweise.ps1:725` die Aussage in jedes Manifest trägt.
**Kategorievorschlag: DEFEKT, Schwere low** — eine überdehnte Beinbehauptung ist ein
K5-Bruch, aber der behauptete Sachverhalt ist heute strukturell wahr (uint8-Verläufe), also
keine falsche Produktaussage, sondern eine ungedeckte Beweisaussage. Ebenfalls präzisiert:
eine **allgemeine Phaseninvarianz** wird hier nicht verlangt — eine beliebige zeitliche
Verschiebung ist für Onset-/Fenstermerkmale keine gültige Invariante. Gemessen werden soll
nur der Teil, der messbar ist: Vorzeichenumkehr.

### 5. Kleinster Fix und Ort des Rotbeweises

**Fix.** `pruefe (true, …)` durch eine echte Messung ersetzen: dasselbe Signal und sein
vorzeicheninvertiertes Gegenstück durch `fingerprintVon` schicken und `bandEnergie`,
`chroma` und `onset` byteweise gleich verlangen; die Behauptungszeile des Beins
(`tools/beweise.ps1:725`) im selben Änderungssatz auf den tatsächlich gemessenen Umfang
verengen (Vorzeichenumkehr statt „ohne Phase").

**Rotbeweis.** Die Zusage trägt `Sonde013FingerprintGoldenTest.cpp:270`; im Kanon hängt sie
am Bein **B22** (`tools/beweise.ps1:725`). **Vor dem Fix:** eine Mutante, die in
`featureengine` das Vorzeichen eines Eingangsblocks in ein Chroma-Byte kodiert, lässt
`:270` unverändert grün — die Zeile kann per Konstruktion nicht rot werden.
**Nach dem Fix** bricht dieselbe Mutante genau an dieser Zeile, und der unveränderte Kern
hält sie grün. Der Rotbeweis fällt damit an der Zeile, die die Zusage trägt, und nicht an
`:233`, `:256` oder `:267`, die andere Eigenschaften messen.

---

## Nebenbefunde (nicht Teil der IDs, keine neuen IDs)

Beim Lesen der Quellketten fiel Folgendes auf. Es ist **kein** neuer Befund, keine ID und
kein Auftrag — nur der Vollständigkeit halber notiert, damit der Dirigent entscheiden kann,
ob es in den jeweiligen Änderungssatz gehört oder nach `docs/offene-punkte.md`.

1. **Zahl in der Abdeckungskarte** (`docs/gesundheit/abdeckungskarte.md:136`,
   `docs/gesundheit/KONZEPT.md:141`): „133 von 136 Feldern mit eingefrorenem Fixture" steht
   um eins zu hoch, wenn die Zählung aus `roh/phase-02-zustand.md:127-132` gilt. Gehört in
   denselben Änderungssatz wie T3-02-07; deshalb dort unter „Fix" mit aufgeführt und hier
   nicht doppelt gezählt.
2. **NAK-154, Punkt 2 besteht am HEAD fort** (`docs/offene-punkte.md:182`):
   `eq-copilot/plugin/core/ipc/TelemetryClient.cpp:531` reicht `e.kopf.schemaMinor`
   weiterhin ungedeckelt an `beiFrame`. Das Produkt ist am Verbraucher geschützt
   (`SourcesModel.cpp:1531`). Die Registerzeile ist also weiterhin zutreffend offen; die
   Reparatur von T3-03-11 löst sie sachlich mit, wenn sie beide Stellen anfasst.
3. **Kein Bein misst den Runner selbst.** `tools/beweise.ps1` hat Selbsttest-Beine für das
   Soak-Orakel (A34, `:616`), den Laufzeit-Arm (A35, `:618`), P4/P5 (A27/A30) und die
   Gesundheitsmessung (A32, `:756`) — aber keines für den Beweisrunner. Das ist der Grund,
   warum der Rotbeweis zu T3-09-01 ein neues Bein braucht statt einer Zeile in einem
   vorhandenen.

## Scope-Beweis

**Gelesen** (Datei · Zeilenzahl am HEAD; bei großen Dateien wurden die genannten Abschnitte
gelesen, nicht die ganze Datei):

| Datei | Zeilen | gelesen |
|---|---|---|
| `CLAUDE.md` | 307 | Wahrheitskern, Invarianten, „Bauen und beweisen" (`:205-220`), Maschinen-Landminen |
| `docs/gesundheit/KONZEPT.md` | 760 | §3 Klassen K1–K8 (`:127-166`), Gliederung |
| `docs/gesundheit/abdeckungskarte.md` | 237 | K3-Schluss und K4 (`:125-155`) |
| `docs/offene-punkte.md` | 514 | NAK-309 (`:316`), NAK-308 (`:514`), NAK-127 (`:164`), NAK-154 (`:182`), NAK-267 (`:277`), NAK-259 (`:269`), NAK-282 (`:292`), Zählabsatz `:416` |
| `docs/plan/plan.json` | 510 | S0 (`:5-16`), S25h (`:333-341`), S25i (`:342-345`) |
| `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` | 840 | Bündelzeilen `:71`, `:73`, `:102-106`; Befundzeilen `:368`, `:388`, `:401`, `:413`, `:466`, `:469`, `:470`; Skeptiker `:511`, `:522`, `:526`, `:529`, `:544`, `:547`, `:548`; Übersicht `:802`, `:812`, `:835`, `:837`; Kopf `:53`, `:255` |
| `.../roh/phase-09-beweisluecken.md` | — | `:18`, `:28-62`, `:80`, `:177-182`, `:291`, `:308-351` |
| `.../roh/phase-05-hostkante.md` | — | `:22`, `:47-57`, `:172`, `:201` |
| `.../roh/phase-02-zustand.md` | — | `:26`, `:110-132` |
| `.../roh/phase-03-vertrag.md` | 263 KB | gezielt über die T3-03-11-Zeile |
| `.../roh/phase-04-lebenslauf.md` | — | gezielt über die T3-04-04-Zeile |
| `docs/beweise/roh/NAK-246-quellvalidierung.md` | — | Kopf und D1/D2 (Form und Tiefe) |
| `docs/beweise/roh/NAK-283-quellvalidierung.md` | — | Kopf und F01 (Form und Tiefe) |
| `docs/beweise/NAK-121.md` | 2238 | `:8`, `:17`, `:146`, `:419`, `:1552` |
| `tools/beweise.ps1` | 1447 | `:485-503`, `:520`, `:552`, `:560`, `:570`, `:584`, `:593`, `:615-621`, `:696`, `:701`, `:711`, `:718`, `:725`, `:753-756`, `:870-905`, `:930-1049`, `:1155-1175`, `:1200-1215`, `:1300-1379` |
| `tools/fl/laufzeit.ps1` | 1613 | `:395-425` |
| `tools/eq-copilot/pruefe_installer_manifest.py` | 1710 | `:30-66`, `:1117-1165`, `:1200`, `:1249`, `:1280`, `:1337-1395`, `:1566`, `:1637-1700` |
| `tools/eq-copilot/pruefe_kern_identitaetsfrei.py` | 779 | `:265-309` |
| `tools/eq-copilot/erzeuge_state_fixtures.py` | 1169 | `:1075-1100`, Volltextsuche `project_binding_id` (0 Treffer) |
| `eq-copilot/plugin/CMakeLists.txt` | 1386 | `:16-100`, `:167`, `:185-291`, `:1130-1190`, `:1256-1271`, `:1351-1386`, alle `nakama_kern_anbinden`-Stellen |
| `eq-copilot/plugin/state/NakamaState.cpp` | 2911 | `:33`, `:810-840`, `:1515`, `:2070-2095` |
| `eq-copilot/plugin/tests/StateMigrationTestMain.cpp` | 2460 | `:995-1015` |
| `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` | 2790 | `:2630-2650` |
| `eq-copilot/plugin/tests/Sonde013FingerprintGoldenTest.cpp` | 356 | `:215-285` |
| `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp` | 1080 | `:620-700`, `:790-815`, `:835-841`, `:905` |
| `eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp` | 325 | `:185-215` |
| `eq-copilot/plugin/core/ipc/TelemetryClient.cpp` | 812 | `:495-531`, `:635-700` |
| `eq-copilot/plugin/core/ipc/WireEnvelope.cpp` | 303 | `:120-195` |
| `eq-copilot/plugin/core/ipc/WireEnvelope.h` | 252 | `:60-100`, `:141`, `:174-200` |
| `eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp` | 951 | `:295-325` |
| `eq-copilot/plugin/core/ipc/BrokerInstallBinding.h.in` | 11 | vollständig |
| `eq-copilot/plugin/src/SourcesModel.cpp` | 1947 | `:1520-1545` |
| `eq-copilot/plugin/src/PluginProcessor.cpp` | — | `:38`, `:82`, `:226` (Einbindungsstellen) |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp` | — | `:4`, `:66` (Einbindungsstellen) |
| `eq-copilot/plugin/core/analysis/featureengine/Vertrag.h` | 773 | `:325-360` |
| `broker/src/briefkasten.rs` | 1995 | `:63`, `:555-620`, `:715-760`, `:853-900`, `:1776-1830` |
| `broker/tests/store_crash_matrix.rs` | 3981 | `:3210-3260`, `:3335-3392`, `:3437-3455` |
| `broker/src/transport/server_v3/mod.rs` | — | `:204-212` |
| `eq-copilot/fixtures/state/schema2/*.bin` | 8 Dateien | byteweise Suche nach `project_binding_id` (0 Treffer je Datei) |

**Nicht gelesen** (bewusst außerhalb des Auftrags):

- `docs/audits/2026-09-15-tiefenaudit/roh/phase-01`, `phase-06` bis `phase-08`,
  `phase-10` (außer den zitierten Skeptikerzeilen in `BEFUNDE.md`), `phase-11`,
  `phase-12` (Sicherheit, ausdrücklich ausgenommen), `phase-13` bis `phase-16` (außer den
  Übersichtszeilen V0/W09/W32/W34 in `BEFUNDE.md`).
- `eq-copilot/plugin/dsp/**` inhaltlich (für T3-09-01 zählt nur die Existenz und die
  CMake-Bindung, nicht das DSP-Verhalten — das gehört zu S25k/NAK-311).
- `design/**` (geparkt), `wissen/**`, `eq-copilot/install/nakama-installer-v1.json`
  (nur über die lesenden Werkzeuge betrachtet, nicht als Datei geöffnet).
- `broker/src/store/**`, `broker/src/coordinator/**` (nur über die genannten Testzeilen
  berührt).
- Die fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` — nicht angefasst.

**Keine Codeänderung, kein Bau, kein Testlauf, keine Reparaturvorschläge außerhalb dieser
sieben IDs.**
