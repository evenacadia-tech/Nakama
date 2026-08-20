# Beweismanifest — SONDE-003 «JUCE-Bridge-Patch: Context, Parameterpunkte, Buslatenz»

| Feld | Wert |
|---|---|
| Ticket | `SONDE-003` |
| Phase / Session | P0 / S3 |
| Gate-Text (Quelle) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §65, Zeile 3797 — **wörtlich**: *„JUCE-Bridge-Patch für Context-Anwesenheit, Parameterpunkte und Buslatenz \| Quellhashgate plus Wrapper-Unitfixture grün; Fallbackbit geprüft"* |
| Technik-Quelle | Entwurf §31.2 (Wrapper-Lücken), §44.3 (Patch + Fallback), §53.7 (Struktur-Vertrag) |
| Commits | `1e91d54` · `4024bb0` · `359b899` · `7bb2f1e` · `3f2fccd` (+ Kommentar-Nachtrag, s. §6) |
| Datum | 2026-08-21 |
| Prüfstufen | T1 ✅ · **T2 ✅ PASS in Runde 3** (Runden 1+2 = NEEDS_WORK, beide nachgearbeitet — §5/§6) · T3 ☐ (kommt am Gate G0) |

---

## 0. Was hier bewiesen wird — und was ausdrücklich NICHT

Der Entwurf behauptet drei Lücken im gepinnten JUCE-8.0.9-VST3-Wrapper. Alle
drei sind vor dem Bau **an der Quelle nachgemessen** worden, nicht abgeschrieben:

| Behauptung des Entwurfs | An der Quelle gefunden |
|---|---|
| Parameterqueues: nur der letzte Punkt | `juce_audio_plugin_client_VST3.cpp`, `processParameterChanges()`: `if (const auto change = getPointFromQueue (paramQueue, numPoints - 1))` — der Sample-Offset wird für normale Parameter komplett verworfen. |
| Fehlender `processContext` nicht erkennbar | `process()`: `zerostruct (processContext)` im else-Zweig; `getPosition()` liefert danach trotzdem ein gefülltes `PositionInfo`. Im VST3-Pfad ist `getPosition().hasValue()` deshalb eine Tautologie, und `setTimeInSeconds (timeInSamples / processContext.sampleRate)` rechnet dort `0.0/0.0` = **NaN**. |
| Presentation-Latency fehlt | `Vst::IAudioPresentationLatency` kommt im **unberührten** 8.0.9-Wrapper nirgends vor — weder in der Basisklassenliste noch in `queryInterfaceInternal` (dort steht nur `IProcessContextRequirements`). Erst der Patch fügt beides hinzu. |

**Bewusst NICHT in diesem Ticket:** kein Processor implementiert heute
`eqcop::hostbruecke::Senke`. Das ist keine Lücke, sondern die Schnittgrenze —
`EqCopilotProcessor` bleibt in `SONDE-003` **unangetastet**, damit der Patch am
Produkt nachweislich folgenlos ist (`verbinde()` bekommt dann `nullptr`, und
`uebergib()` kehrt sofort zurück). Der Verbraucher entsteht in `SONDE-008/009`
zusammen mit den zeitgestempelten Featureframes; der heutige `hatTransport`-
Tautologiefehler ist als **NAK-24** festgehalten und wird dort behoben, nicht
nebenbei.

**Nicht bewiesen und offen benannt:** dass FL Studio die Bridge-Daten in der
Praxis liefert (welche Flags gesetzt sind, ob Presentation-Latency überhaupt
gemeldet wird, wie sich Loop-Straddle und Offline-Render verhalten). Das ist
kein Headless-Beweis, sondern **User-Termin B** — genau dafür ist er im
Sessionplan vorgesehen. Headless bewiesen ist: der Patch sitzt, greift genau
dort, wo er soll, kompiliert in das Produkt-Bundle hinein, nimmt nichts weg,
und die Abbildungslogik hält alle Verträge.

---

## 1. Ticket-Behauptungen

| # | Behauptung (Gate-Text) | Befehl | Ergebnis | Rohausgabe | Datum |
|---|---|---|---|---|---|
| 1 | „Wrapper-Unitfixture grün" | `EqCopHostContextTest.exe` | ✅ **91/91**, Exit 0 | [↓ B1](#b1) | 2026-08-21 |
| 2 | „Quellhashgate" — fremder Quellstand bricht den Bau ab | `cmake -S eq-copilot -B eq-copilot/build` nach Fremdänderung | ✅ Exit 1, gemessener Hash genannt | [↓ B2](#b2) | 2026-08-21 |
| 3 | „Quellhashgate" — unberührt wird gepatcht, gepatcht bleibt unangetastet (idempotent) | dito, zweimal | ✅ Exit 0 / Exit 0 | [↓ B3](#b3) | 2026-08-21 |
| 4 | Der Patch landet wirklich im Produkt-Wrapper (und der Fallbackweg existiert daneben) | `#error`-Falle + zwei Bauten | ✅ Produkt bricht, Fallback baut | [↓ B4](#b4) | 2026-08-21 |
| 5 | Der Patch ist ein echter, anwendbarer Unified Diff | `git apply --check` + Rundlauf | ✅ Exit 0, Zielhash erreicht | [↓ B5](#b5) | 2026-08-21 |
| 6 | „Fallbackbit geprüft" — ohne Bridge/Senke bleibt alles folgenlos | Teil von B1 (Abschnitt E) + B4 + B6 | ✅ | [↓ B6](#b6) | 2026-08-21 |
| 7 | Der Patch bricht das VST3-Hosting nicht (Zusatzbeweis über den Gate-Text hinaus) | `pluginval --strictness-level 8` gegen das GEBAUTE Bundle | ✅ SUCCESS, Exit 0 | [↓ B7](#b7) | 2026-08-21 |
| 8 | Die Prüfung des Vertrags-Rückfallwerts **unterscheidet** Bug und Fix | Bug wiederhergestellt, Test gefahren, Fix zurückgestellt | ✅ rot → grün | [↓ B8](#b8) | 2026-08-21 |
| 9 | Auch die Prüfungen gegen die **Regression aus Runde 2** unterscheiden | Regression wiederhergestellt, Test gefahren | ✅ **5 rot** → 91/91 grün | [↓ B9](#b9) | 2026-08-21 |

---

## 2. Rohe Ausgaben

<a id="b1"></a>
### B1 · Wrapper-Unitfixture

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` · **Exitcode:** 0 · **Datum:** 2026-08-21 (77 → 85 → **91** Prüfungen; die Zuwächse sind T2-Nacharbeit)

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
EXITCODE=0
```

<a id="b2"></a>
### B2 · Quellhash-Gate, Zweig „fremder Quellstand"

Eine fremde Zeile wird an den gepatchten Wrapper angehängt; das Configure muss
abbrechen und den **gemessenen** Hash nennen, statt blind zu patchen.

**Exitcode:** 1 · **Datum:** 2026-08-21

```text
$ printf "// fremde Zeile" >> juce_audio_plugin_client_VST3.cpp
$ cmake -S eq-copilot -B eq-copilot/build
CMake Error at cmake/NakamaBruecke.cmake:60 (message):
  Nakama-Bruecke: unbekannter JUCE-Quellstand — Bau abgebrochen.

    Datei    : C:/Users/phili/Projekte/Nakama/eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp
    gemessen : cca67428ebb240ab951e2a86ac58e18dcc6a6d5bd00f283e545c2c93ca1f65e9
    erwartet : 1374eb400cb6152dc39698dfdd1d7c89c2188d6720edc6bdcc486872f7da72d0 (unberuehrt 8.0.9)
    oder     : 6e5d4660d960836a875e4b2207f5bb4372b5266776e00c4bb0fdef1ee87a01bc (bereits gepatcht)

  

  Wahrscheinliche Ursache: JUCE wurde aktualisiert.  Dann ist der

  Bridge-Patch NEU zu beweisen (SONDE-003): Patch gegen den neuen

EXITCODE=1
```

<a id="b3"></a>
### B3 · Quellhash-Gate, Zweige „unberührt" und „idempotent"

`git checkout` setzt den Wrapper auf den Auslieferungsstand zurück (das ist
zugleich der Selbstheilungsfall, wenn FetchContent die Datei ersetzt). Der
nächste Configure patcht und **misst nach**; der übernächste tut nichts mehr.

**Exitcode:** 0 / 0 · **Datum:** 2026-08-21

```text
$ git -C .../juce-src checkout -- modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp
$ cmake -S eq-copilot -B eq-copilot/build
-- Nakama-Bruecke: JUCE-Wrapper gepatcht und nachgemessen (6e5d4660d960836a875e4b2207f5bb4372b5266776e00c4bb0fdef1ee87a01bc).
-- Configuring done (2.6s)
-- Generating done (0.8s)
EXITCODE=0 (Configure durchgelaufen)
$ cmake -S eq-copilot -B eq-copilot/build   # zweites Mal, unveraendert
-- Nakama-Bruecke: JUCE-Wrapper ist bereits gepatcht (6e5d4660d960836a875e4b2207f5bb4372b5266776e00c4bb0fdef1ee87a01bc).
-- Configuring done (2.5s)
EXITCODE=0
```

<a id="b4"></a>
### B4 · Die Brücke ist wirklich im Produkt-Bundle — und der Fallbackweg daneben

Ein Riegel, den man nicht fallen sieht, ist keiner. In `NakamaHostBridge.h` wird
vorübergehend ein `#error` gesetzt: `EqCopilot_VST3` (mit `NAKAMA_HOST_BRIDGE=1`)
**muss** daran scheitern, `EqCopAuxSpike_VST3` (ohne das Define) **muss** im
selben Moment weiterbauen — das ist zugleich der Beweis, dass der gepatchte
Wrapper ohne Brücke übersetzbar bleibt.

**Datum:** 2026-08-21

```text
$ # In NakamaHostBridge.h voruebergehend "#error NAKAMA-BRUECKE-WURDE-EINGEBUNDEN" einsetzen
$ cmake --build eq-copilot/build --config Release --target EqCopilot_VST3
C:\Users\phili\Projekte\Nakama\eq-copilot\plugin\hostbridge\NakamaHostBridge.h(39,1): error C1189: #error:  NAKAMA-BRUECKE-WURDE-EINGEBUNDEN [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_VST3.vcxproj]
$ cmake --build eq-copilot/build --config Release --target EqCopAuxSpike_VST3   # ohne NAKAMA_HOST_BRIDGE
  EqCopAuxSpike_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAuxSpike_artefacts\Release\VST3\EqCop-Aux-Spike.vst3\Contents\x86_64-win\EqCop-Aux-Spike.vst3
$ # Falle entfernt, Kontrollbau:
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
```

<a id="b5"></a>
### B5 · Patch-Rundlauf gegen den unberührten Wrapper

**Datum:** 2026-08-21

```text
$ git -C .../juce-src checkout -- <wrapper>   # auf unberuehrt zuruecksetzen
$ git -C .../juce-src apply --check --verbose <patch>
Checking patch modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp...
EXITCODE=0
$ git apply <patch> && sha256(CRLF->LF)
  sha256 normalisiert: 6e5d4660d960836a875e4b2207f5bb4372b5266776e00c4bb0fdef1ee87a01bc
  CRLF erhalten      : True ( 4243 Zeilen )
```

<a id="b6"></a>
### B6 · Erreicht das Define den Wrapper? (Fallbackweg wird in jedem Bau mitkompiliert)

Nicht geglaubt, sondern in den erzeugten Projektdateien nachgesehen.

**Datum:** 2026-08-21

```text
$ grep -c "NAKAMA_HOST_BRIDGE=1" eq-copilot/build/plugin/EqCopilot_VST3.vcxproj
8
$ grep -c "NAKAMA_HOST_BRIDGE"   eq-copilot/build/plugin/EqCopAuxSpike_VST3.vcxproj
0
```


<a id="b7"></a>
### B7 · Echter VST3-Host: `pluginval`, Strenge 8

Der einzige Host-Beweis, der ohne FL zu haben ist. `pluginval` lädt das
**gebaute** Bundle, fährt 30 Samplerate-/Blockgrößen-Kombinationen, automatisiert
und fuzzt Parameter, schaltet Busse an und ab. Damit ist belegt, dass die
zusätzlich angebotene COM-Schnittstelle `IAudioPresentationLatency` und der
zweite Durchlauf durch die Parameterqueue das Hosting nicht brechen.

**Nicht** belegt: dass Daten in einer Senke ankommen — `pluginval` ist keine.
Das bleibt Termin B.

Dieser Lauf ist der **erneuerte** — der erste lief gegen ein Bundle von vor der
T2-Nacharbeit (Notiz des Prüfers aus Runde 2); hier ist das Bundle des Endstands
validiert.

**Befehl:** `pluginval --strictness-level 8 --validate "...\EQ-Copilot.vst3" --skip-gui-tests` · **Exitcode:** 0 · **Datum:** 2026-08-21

```text
Started validating: C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3
Random seed: 0x4f42eb1
Validation started
Strictness level: 8
-----------------------------------------------------------------
Starting tests in: pluginval / Scan for plugins located in: C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3...
[... 30 Samplerate-/Blockgroessen-Kombinationen, Automation, Busse an/aus, Parameter-Fuzz ...]
SUCCESS
EXITCODE=0
```

<a id="b8"></a>
### B8 · Der neue Riegel fällt sichtbar

Eine Prüfung, die unter Bug und Fix gleich ausgeht, ist keine. Deshalb wurde
der T2-Befund im Header **wiederhergestellt** (`merkeLetztwert()` hinter den
Überlauf-Ausstieg verschoben) und der Test erneut gefahren — er muss rot
werden und den falschen Wert benennen.

**Datum:** 2026-08-21

```text
$ # merkeLetztwert() voruebergehend HINTER den Ueberlauf-Ausstieg verschoben (= der T2-Befund)
$ EqCopHostContextTest.exe
  FEHLER  Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.5110)
HOSTKONTEXT FEHLGESCHLAGEN - 1 von 85 Pruefungen rot.
EXITCODE=1
$ # Fix zurueckgestellt, Kontrolllauf:
HOSTKONTEXT OK - 85/85 Pruefungen bestanden.
EXITCODE=0
```

<a id="b9"></a>
### B9 · Auch der zweite Riegel fällt sichtbar

Die Regression aus Runde 2 (Plausibilitätsprüfung hinter dem Überlauf-Ausstieg)
wurde wiederhergestellt und der Test erneut gefahren. **Fünf** Prüfungen gehen
rot — darunter die vier neuen für die Kombination Überlauf + unplausibler Punkt.

**Datum:** 2026-08-21

```text
  FEHLER  Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.5110)
  FEHLER  NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  FEHLER  NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  FEHLER  unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  FEHLER  Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
HOSTKONTEXT FEHLGESCHLAGEN — 5 von 91 Pruefungen rot.
```

---

## 3. Kanon-Lauf

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-003.md -Anhaengen -Titel 'SONDE-003'
```

_(Der angehängte Abschnitt steht am Ende dieser Datei.)_

---

## 4. T1 — Selbstaudit

| # | Punkt | Befund |
|---|---|---|
| 1 | Numerische Ränder | **Geprüft und getestet.** Negativer Sample-Offset → auf 0 geklemmt, gezählt, `sampleAccurateAutomation` fällt. Offset ≥ Blockgröße → auf `blockGroesse-1` geklemmt, gezählt. Blockgröße 0 (Parameter-Flush, VST3 erlaubt `numSamples == 0`) hat einen eigenen Zweig: Offset 0 plausibel, alles andere gemeldet. NaN/Inf als Parameterwert werden **gezählt, aber nicht sanitisiert** — der Messwert bleibt, verworfen wird die Zusicherung. Überlauf ab 512 Punkten lässt den ganzen Block-Anspruch fallen und zählt, WIE VIELE Punkte nicht mehr passten — welche es waren, sagt der Zähler nicht (das kann er nicht). Der vom Vertrag geforderte Rückfallwert überlebt den Überlauf trotzdem, weil er in einer eigenen Letztwert-Tabelle geführt wird; die hat ihre eigene Grenze und ihren eigenen Zähler. `timeInSeconds` wird gar nicht erst gebildet (wäre im genullten Context 0/0 = NaN). Busindex außerhalb `[0, 16)` wird verworfen und gezählt statt zu schreiben. |
| 2 | Gegenpfad vorhanden? | **Ja, beide Richtungen im selben Änderungssatz.** Patch anwenden ↔ `git checkout` + erneutes Configure heilt automatisch nach (B3). Verbinden ↔ Nicht-Verbinden (`istVerbunden()`, ohne Senke folgenlos, B1/E). `beginneBlock` ↔ `uebergib` (Blockzustand wird je Block gelöscht, B1/B letzter Punkt). Kontext vorhanden ↔ ausdrücklich fehlend. Latenz gemeldet ↔ nie gemeldet. Getestet ist auch, dass die Latenz `beginneBlock` **überlebt** (Setup-Zustand, kein Blockwert) — das ist der Fall, den ein reines „alles löschen" still kaputtgemacht hätte. |
| 3 | Behauptungs-Integrität | **Jede Zeile geöffnet, keine zitiert.** Die drei Wrapper-Lücken sind in Abschnitt 0 mit dem tatsächlichen Quelltext belegt. `Vst::BusDirection`/`setAudioPresentationLatencySamples` stammen aus dem mitgelieferten SDK-Header `pluginterfaces/vst/ivstaudioprocessor.h` (Zeile 371–382), nicht aus dem Gedächtnis. Die Feldnamen des `ProcessContext` sind nicht abgeschrieben, sondern werden vom Test gegen den **echten** `Steinberg::Vst::ProcessContext` instanziiert — inklusive der SDK-Schreibweise `continousTimeSamples` (ohne „u"), die ein Nachbau still falsch gehabt hätte. |
| 4 | Lügt der UI-Text? | **Keine UI angefasst.** Das Ticket ist reine Infrastruktur; der Editor wurde nicht berührt. Die einzigen Texte sind Testausgaben und CMake-Meldungen — beide nennen gemessene Hashes statt Urteile. |
| 5 | Anzeige-Pflichten (§0.4) | **Nicht betroffen** (kein UI-Diff). Der Vertrag liefert allerdings genau die Rohdaten, aus denen die späteren Pflichtanzeigen „Capability-Degradation" und „welche Aktion gerade nicht aktiv ist" gespeist werden: `sampleAccurateAutomation`, `processContextPresent` und die drei Zähler. |
| 6 | Audiothread | **Gemessen, nicht behauptet.** 200 Blöcke à 64 Punkte mit überladenem `operator new`: **0 Allokationen** (B1/E). Kein Lock, kein I/O, kein Logging im Blockpfad; die einzige Synchronisation ist ein `std::atomic<uint64_t>` je Bus mit `memory_order_relaxed`, dessen Lock-Freiheit per `static_assert` **und** zur Laufzeit geprüft wird. Der einzige `dynamic_cast` steht im Wrapper-Konstruktor, also auf dem Nachrichtenthread. Die Sortierung ist eine Einfügung in einen vorallokierten Puffer fester Größe. |

---

## 5. T2 — Frischkontext-Prüfer

| Feld | Wert |
|---|---|
| Agent | `evaluator` (eigener Kontext, sieht meine Zusammenfassung **nie**) |
| Vorgelegt | 1. Diff `0ba87cc..HEAD` · 2. dieses Manifest · 3. Gate-Text §65/§44.3/§53.7 im Wortlaut |
| Urteil Runde 1 | ☑ **NEEDS_WORK** (blockierend) |
| Datum Runde 1 | 2026-08-21 |
| Urteil Runde 2 | ☑ **NEEDS_WORK** (eine Regression AUS der Runde-1-Nacharbeit) |
| Datum Runde 2 | 2026-08-21 |
| Urteil Runde 3 | ☑ **PASS** |
| Datum Runde 3 | 2026-08-21 |

**Runde 3 — was der Prüfer getan hat.** Er hat wieder nichts geglaubt: ein
eigenes Programm gegen den jetzigen Header gebaut, den Vertragswert und die drei
Unplausibilitätsfälle jenseits von Punkt 512 nachgemessen — und den Riegel
**selbst** zum Fallen gebracht, ohne das Repo anzufassen (Kopie des Headers,
Überlauf-Ausstieg wieder nach vorn): **5 rot gegen die gebugte Kopie inklusive
des protokollierten `0.5110`, 0 rot gegen HEAD**, während die drei
Kontrollprüfungen in BEIDEN Läufen grün bleiben — der Riegel trifft also den
Fehler und nicht das Umfeld.

Dieselben fünf Punkte, hier mit dem Kanon-Binär auf dem Endstand gemessen
(Auszug aus [B1](#b1), eigener Lauf — kein zitiertes Fremdprotokoll):

```text
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
```

Weiter geprüft und nichts gefunden: `ueberlaeufe > 0` kann nie mit
`sampleAccurateAutomation == true` zusammenfallen (die Konjunktion bindet alle
vier Blockzähler) · kein Wrap durch die vorgezogene Offset-Arithmetik
(`INT_MIN`, Flush mit Offset 2 Milliarden, `blockGroesse == 1` mit Offset 5 —
alle drei landen bei Offset 0 mit einem gezählten Ausreißer) · 0 Allokationen
bei 200 Blöcken à 600 Punkten über 140 Parameter · kein Eingabepfad, der Audio
oder Parameterwerte verändert.

Der Prüfer hat nichts geglaubt: beide gepinnten Hashes eigenständig nachgerechnet
(drei unabhängige Implementierungen — CMake, `juce::SHA256`, sein eigenes Python —
liefern denselben Wert), den Patch-Rundlauf in einer eigenen Sandkiste gefahren,
den Test selbst gestartet und ein **eigenes Programm** gegen den unveränderten
Header gebaut, um den Befund zu messen statt ihn zu argumentieren. Sein Ergebnis:
`anzahl=512 ueberlaeufe=6 sampleAcc=0 hatLetzten=1 wert=0.511` — erwartet wäre
`0.777`.

Ausdrücklich **nicht** beanstandet: Quellhash-Gate in allen drei Zweigen,
Allokations-/Lock-Freiheit, die numerischen Ränder, die Busindex-Grenzen, die
Einhängung von `IAudioPresentationLatency` — und dass es **keinen Eingabepfad
gibt, über den der Patch Audio oder Parameterwerte verändert** (Gate 1 unberührt).

---

## 6. Befunde aus diesem Ticket

| Befund | Quelle | Gegen die **Quelldatei** verifiziert? | Ausgang |
|---|---|---|---|
| `git` hätte den Patch beim Commit kaputtnormalisiert: JUCE speichert den Wrapper **im Objektspeicher** mit CRLF (4165/4165 gemessen), der Patch trägt deshalb in 149 seiner 163 Zeilen CRLF. Unter `core.autocrlf=true` hätte git sie beim `add` entfernt — hier wäre nichts aufgefallen, auf dem Zweitrechner hätte `git apply` versagt. | T1 (beim Stagen aufgefallen) | ✅ ja — Objektspeicher und Index-Blob beide byteweise nachgezählt | **gefixt**: `.gitattributes` → `*.patch -text`; Index-Blob nachgemessen, trägt wieder 149 CRLF |
| `tools/beweise.ps1` prüfte den Baustand nur gegen `plugin/src`, `plugin/tests` und die beiden CMakeLists. `hostbridge/`, `spike/`, `probe/`, `cmake/` und der Patch selbst fehlten — eine Änderung dort hätte veraltete Prüfbinaries als **beglaubigt** durchgehen lassen. Das ist derselbe T2-Befund vom 20.08., nur eine Ebene tiefer. | T1 | ✅ ja — Quellorte-Liste im Runner geöffnet | **gefixt**: fünf Orte ergänzt, Manifest-Text nachgezogen |
| Der Kanon führte `EqCopHostContextTest` als „geplant (ab P2)". Gebaut wird es aber in P0. Eine Zeile, die „geplant" sagt, während das Ziel schon Pflicht ist, schönt das Urteil. | T1 | ✅ ja — Kanon-Tabelle geöffnet | **gefixt**: `AbPhase` auf `P0`, Behauptungstext um das Quellhash-Gate ergänzt |
| Zwei MSVC-C4458-Warnungen (Parameter verdeckt Member) im ersten Wurf des Brücken-Headers. | T1 (Compiler) | ✅ ja | **gefixt**: Parameter umbenannt; Bau jetzt warnungsfrei |
| Die neue COM-Schnittstelle ist ab jetzt für **jeden** Host sichtbar: `queryInterface(IAudioPresentationLatency)` liefert `kResultOk` statt `kNoInterface`. Ein Host kann daraufhin Aufrufe machen, die er vorher unterließ. | T1 (abgewogen, nicht übersehen) | ✅ ja — SDK-Header `ivstaudioprocessor.h:342–382` geöffnet: die Schnittstelle ist rein **informativ** (Host → Plugin, „Inform the plug-in about…"), sie verlangt nichts und verändert keine Verarbeitung. Unsere Umsetzung macht ausschließlich einen atomaren Store. | **kein Fehler, aber benannt**: `pluginval` Strenge 8 gegen das gebaute Bundle ist SUCCESS (B7); das FL-Verhalten misst Termin B |
| `verworfeneLetztwerte` zählt verworfene **Aktualisierungen**, nicht **Parameter** — bei 133 Parametern mit vielen Punkten maß der Prüfer 516. Unter dem Feldnamen ist das exakt richtig, aber der Zeilenkommentar („mehr als `kMaxLetztwerte` Parameter im Block") beschrieb die **Bedingung** und las sich wie die **Einheit**. | **T2 Runde 3** (ausdrücklich NICHT als Mangel gewertet) | ✅ ja — der Prüfer hat es gemessen und belegt, dass keine Verbraucherentscheidung kippt (ungleich null ⇔ verworfen; die verdrängten IDs melden ehrlich „nichts") | **trotzdem gefixt**: die Einheit steht jetzt im Feldkommentar. Ein Zähler, dessen Kommentar sich wie eine andere Einheit liest, fällt unter unseren eigenen Maßstab „lügt der Text?". Das PASS wurde auf `3f2fccd` erteilt; diese Klarstellung ist die einzige spätere Änderung und berührt keine Zeile Code |
| **REGRESSION aus meiner eigenen Runde-1-Nacharbeit:** der Überlauf-Ausstieg sprang über die Plausibilitätsprüfungen hinweg. Ein NaN als Punkt 513 wurde damit stiller Rückfallwert, während `unplausibleWerte` **0** meldete — der Zähler log also genau in dem Fall, für den er da ist. Dasselbe für Inf und für unplausible Offsets. | **T2 Runde 2** | ✅ ja — `NakamaHostBridge.h:330–332` geöffnet; der Prüfer hat es zusätzlich gemessen (`NaN-NACH-UEBERLAUF unplausibleWerte=0 rueckfallwert=nan`) und mit einer Kopie des Vor-Fix-Stands belegt, dass die Lücke NEU war | **gefixt**: `punkt()` misst jetzt in der Reihenfolge Offset → NaN/Inf → Rückfallwert → Ringkapazität. Die Zähler beschreiben damit, was der **Host** geliefert hat, nicht was in den Ring passte. Vier neue Prüfungen; der Riegel wurde zum Fallen gebracht (B9: 5 rot unter der Regression) |
| `verworfeneBusse` ist **kumulativ**, die vier Nachbarzähler setzen pro Block zurück — der Feldkommentar sagte es nicht. | **T2 Runde 2** (NOTE) | ✅ ja | **gefixt**: im Feldkommentar benannt, samt Begründung (die Latenztabelle ist Setup-Zustand und überlebt `beginneBlock` ebenfalls) |
| `pluginval` (B7) lief gegen ein Bundle von **vor** der Nacharbeit. | **T2 Runde 2** (NOTE) | ✅ ja — Zeitstempel verglichen | **gefixt**: gegen den Endstand erneut gefahren, B7 trägt jetzt diesen Lauf |
| **§53.7 war nicht eingelöst — der Überlauf verwarf genau den Wert, den der Vertrag überleben lässt.** `punkt()` stieg bei vollem Ereignisring aus, bevor der Wert gemerkt war; `hatLetztenBlockwert()` las nur den Ring und lieferte deshalb `true` mit **0.511** statt **0.777** — ununterscheidbar von einem gültigen Wert. JUCE setzt den Parameter tatsächlich auf 0.777; ein Verbraucher hätte auf einen Wert gerampt, den das Plugin nie hält. | **T2 Runde 1** | ✅ ja — `NakamaHostBridge.h` geöffnet, Entwurf §53.7 (Zeile 3377–3379) und §44.3 (Zeile 2481) im Wortlaut gegengelesen; der Prüfer hat es zusätzlich selbst gemessen | **gefixt**: eigene Letztwert-Tabelle (`kMaxLetztwerte = 128`), die VOR jedem Überlauf-Ausstieg fortgeschrieben wird und einen eigenen Zähler hat; `hatLetztenBlockwert()` liest jetzt sie statt des Rings |
| **Die einzige Prüfung dieses Vertrags unterschied nicht.** Sie testete nur das Bool von `hatLetztenBlockwert()`; `letzter` wurde geschrieben und nie geprüft — grün bei 0.777 wie bei 0.511. | **T2 Runde 1** | ✅ ja — `HostContextTestMain.cpp:332–333` geöffnet | **gefixt**: Assertion auf den WERT; und der Riegel wurde zum Fallen gebracht (B8: mit Bug rot mit „gemessen: 0.5110", nach Fix 85/85 grün). Dazu drei neue Prüfungen für den eigenen Überlauf der Letztwert-Tabelle und eine für die Deckungsgleichheit ohne Überlauf |
| **`verworfeneBusse` war ein Zähler, den im Produkt niemand lesen kann** — die `Bruecke` ist im gepatchten Wrapper ein privates Member, und `Blockbefund` trug den Zähler nicht. „Zähler statt stiller Korrekturen" galt damit nur für drei von vier. | **T2 Runde 1** | ✅ ja — Header und Patch-Hunk geöffnet | **gefixt**: `verworfeneBusse` und `verworfeneLetztwerte` stehen jetzt im `Blockbefund`; eigene Prüfung dazu |
| Der Arbeitsbaum war beim Prüflauf minimal neuer als die Beweise (zwei unbestätigte Kommentar-Änderungen). | **T2 Runde 1** (NOTE) | ✅ ja — der Prüfer hat den Diff geöffnet und als reine Textpräzisierung bestätigt | **gefixt**: committet als `359b899`; danach Kanon neu gefahren |
| Der VST3-Parameter-Flush (`numSamples == 0`, `data.numInputs == 0`) ruft `processBlock` **nicht**, die Brücke aber schon. Ein Verbraucher, der 1:1-Paarung mit `processBlock` annimmt, liefe aus dem Takt. | T1 | ✅ ja — `process()` geöffnet, der Flush-Zweig steht dort ausdrücklich | **kein Fehler, sondern Vertrag**: der Befund trägt dann `blockGroesse == 0`; im Patch und im Header dokumentiert, im Test belegt (zwei Flush-Prüfungen) |

---

## 7. Nur bei Phasengates — T3

Kommt bei **G0** (nach S4), nicht hier: `/c-review` gegen den Bridge-Patch +
Codex als zweites Modell, Falsifikationsaufträge gegen die harten Gates 1 und 5.

---

## Kanon-Lauf - SONDE-003

**Lauf:** 2026-08-21 00:14 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 6/6 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 00:14:01 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 1e91d54 S3 SONDE-003: JUCE-Bridge-Patch fuer Context, Parameterpunkte und Buslatenz |
| Commit (voll) | 1e91d541b36c0200369a9d7effdd81afe0caee30 |
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
?? docs/beweise/SONDE-003.md
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-18 11:46:33 | `A6F71DAE47C7BE10` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-18 11:47:19 | `3456136DDD939B7F` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-20 21:28:44 | `AFEC07D04C9A80A6` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 00:14:06 | `8F097B1DE00603B4` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 00:11:50**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,10 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,67 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,06 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 1,51 s | [↓ A4](#a4) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,10 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3](#b3) |
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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,67 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,06 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 1,51 s

stdout:

```text

running 36 tests
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::hin_und_zurueck ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
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
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 36 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.82s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-a60741d6aa4f32af.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-b91f868b743e27b1.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-c17ffc214d5eff74.exe)
   Doc-tests eqcop_broker
```

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,10 s

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 12:14:33am vs 21 Aug 2026 12:02:43am]
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
  ok      Schema-1-State 'sensor' ist bytegleich zum Golden  [128 Bytes]
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-State 'hub' ist bytegleich zum Golden  [125 Bytes]
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-State 'pre' ist bytegleich zum Golden  [133 Bytes]
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-State 'post' ist bytegleich zum Golden  [135 Bytes]
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 63 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

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
  ok      Ueberlauf: letzter Blockwert bleibt abrufbar (Rueckfallweg)
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (6840 Byte)

HOSTKONTEXT OK — 77/77 Pruefungen bestanden.
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 31,54 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  HostContextTestMain.cpp
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  juce_audio_plugin_client_VST3.cpp
     Bibliothek "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.lib" und Objekt "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.exp" werden erstellt.
  Code wird generiert.
  Codegenerierung ist abgeschlossen.
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
```

</details>


---

## Kanon-Lauf - SONDE-003 (Endstand)

**Lauf:** 2026-08-21 00:25 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 6/6 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 00:25:38 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 4024bb0 S3 SONDE-003: Beweismanifest, Kanon 6/6, Doku nachgezogen |
| Commit (voll) | 4024bb0433feaab629101b505926d40a5882a85c |
| Arbeitsbaum | 5 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M docs/NEXT-SESSION.md
 M docs/bauaufteilung-sonden.md
 M docs/beweise/SONDE-003.md
 M eq-copilot/cmake/NakamaBruecke.cmake
 M eq-copilot/plugin/hostbridge/NakamaHostBridge.h
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-18 11:46:33 | `A6F71DAE47C7BE10` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-18 11:47:19 | `3456136DDD939B7F` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-20 21:28:44 | `AFEC07D04C9A80A6` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 00:24:41 | `E3F3B87169A7B010` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 00:23:43**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,54 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,00 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,36 s | [↓ A4](#a4) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [↓ B3](#b3) |
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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,54 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,00 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,36 s

stdout:

```text

running 36 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
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
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 36 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-a60741d6aa4f32af.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-b91f868b743e27b1.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-c17ffc214d5eff74.exe)
   Doc-tests eqcop_broker
```

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 12:25:42am vs 21 Aug 2026 12:02:43am]
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
  ok      Schema-1-State 'sensor' ist bytegleich zum Golden  [128 Bytes]
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-State 'hub' ist bytegleich zum Golden  [125 Bytes]
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-State 'pre' ist bytegleich zum Golden  [133 Bytes]
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-State 'post' ist bytegleich zum Golden  [135 Bytes]
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 63 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

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
  ok      Ueberlauf: letzter Blockwert bleibt abrufbar (Rueckfallweg)
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (6840 Byte)

HOSTKONTEXT OK — 77/77 Pruefungen bestanden.
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 4,23 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
```

</details>


---

## Kanon-Lauf - SONDE-003 (nach T2-Nacharbeit)

**Lauf:** 2026-08-21 00:34 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 6/6 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 00:34:50 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 359b899 S3 SONDE-003: Host-Beweis mit pluginval, Zweitrechner-Haertung, Termin B benannt |
| Commit (voll) | 359b899f909af8f9edfd21c87ba82bfc05b5c239 |
| Arbeitsbaum | 3 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M docs/beweise/SONDE-003.md
 M eq-copilot/plugin/hostbridge/NakamaHostBridge.h
 M eq-copilot/plugin/tests/HostContextTestMain.cpp
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-18 11:46:33 | `A6F71DAE47C7BE10` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-18 11:47:19 | `3456136DDD939B7F` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-20 21:28:44 | `AFEC07D04C9A80A6` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 00:33:24 | `C449D8FB6A2F3E9C` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 00:33:21**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,07 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,85 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,03 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,41 s | [↓ A4](#a4) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |

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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,85 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,03 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,41 s

stdout:

```text

running 36 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
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
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 36 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.13s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-a60741d6aa4f32af.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-b91f868b743e27b1.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-c17ffc214d5eff74.exe)
   Doc-tests eqcop_broker
```

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 12:35:23am vs 21 Aug 2026 12:02:43am]
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
  ok      Schema-1-State 'sensor' ist bytegleich zum Golden  [128 Bytes]
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-State 'hub' ist bytegleich zum Golden  [125 Bytes]
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-State 'pre' ist bytegleich zum Golden  [133 Bytes]
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-State 'post' ist bytegleich zum Golden  [135 Bytes]
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 63 Pruefungen ok, 0 Fehler
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

HOSTKONTEXT OK — 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 32,39 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  juce_audio_plugin_client_VST3.cpp
     Bibliothek "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.lib" und Objekt "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.exp" werden erstellt.
  Code wird generiert.
  Codegenerierung ist abgeschlossen.
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
```

</details>


---

## Kanon-Lauf - SONDE-003 (nach T2-Runde 2)

**Lauf:** 2026-08-21 00:45 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 6/6 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 00:45:42 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 7bb2f1e S3 SONDE-003: T2-Nacharbeit - der Rueckfallwert ueberlebt den Ueberlauf |
| Commit (voll) | 7bb2f1e60e23558da5c6863ace309217a2ce98a7 |
| Arbeitsbaum | 3 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M docs/beweise/SONDE-003.md
 M eq-copilot/plugin/hostbridge/NakamaHostBridge.h
 M eq-copilot/plugin/tests/HostContextTestMain.cpp
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-18 11:46:33 | `A6F71DAE47C7BE10` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-18 11:47:19 | `3456136DDD939B7F` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-20 21:28:44 | `AFEC07D04C9A80A6` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 00:44:05 | `B8811711BFA5483E` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 00:44:03**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,49 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 5,99 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,36 s | [↓ A4](#a4) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3](#b3) |
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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,49 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 5,99 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,36 s

stdout:

```text

running 36 tests
test framing::tests::abbruch_mitten_im_frame ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test framing::tests::hin_und_zurueck ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
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
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok

test result: ok. 36 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.09s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-a60741d6aa4f32af.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-b91f868b743e27b1.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-c17ffc214d5eff74.exe)
   Doc-tests eqcop_broker
```

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 12:45:47am vs 21 Aug 2026 12:02:43am]
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
  ok      Schema-1-State 'sensor' ist bytegleich zum Golden  [128 Bytes]
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-State 'hub' ist bytegleich zum Golden  [125 Bytes]
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-State 'pre' ist bytegleich zum Golden  [133 Bytes]
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-State 'post' ist bytegleich zum Golden  [135 Bytes]
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 63 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

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

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 4,22 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
```

</details>


---

## Kanon-Lauf - SONDE-003 (Endstand, T2 PASS)

**Lauf:** 2026-08-21 01:40 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 6/6 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 01:40:52 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 3f2fccd S3 SONDE-003: T2-Runde 2 - Zaehler beschreiben den Host, nicht die Ringgroesse |
| Commit (voll) | 3f2fccdca29be36960660119e2ad52518b83f933 |
| Arbeitsbaum | 5 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M CLAUDE.md
 M docs/NEXT-SESSION.md
 M docs/bauaufteilung-sonden.md
 M docs/beweise/SONDE-003.md
 M eq-copilot/plugin/hostbridge/NakamaHostBridge.h
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-18 11:46:33 | `A6F71DAE47C7BE10` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-18 11:47:19 | `3456136DDD939B7F` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-20 21:28:44 | `AFEC07D04C9A80A6` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 01:38:22**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,53 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 5,83 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,37 s | [↓ A4](#a4) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [↓ B3](#b3) |
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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,53 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 5,83 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,37 s

stdout:

```text

running 36 tests
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::hin_und_zurueck ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
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
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 36 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.11s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-a60741d6aa4f32af.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-b91f868b743e27b1.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-c17ffc214d5eff74.exe)
   Doc-tests eqcop_broker
```

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 1:41:21am vs 21 Aug 2026 12:02:43am]
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
  ok      Schema-1-State 'sensor' ist bytegleich zum Golden  [128 Bytes]
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-State 'hub' ist bytegleich zum Golden  [125 Bytes]
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-State 'pre' ist bytegleich zum Golden  [133 Bytes]
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-State 'post' ist bytegleich zum Golden  [135 Bytes]
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 63 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

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

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 28,93 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  juce_audio_plugin_client_VST3.cpp
     Bibliothek "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.lib" und Objekt "C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.exp" werden erstellt.
  Code wird generiert.
  Codegenerierung ist abgeschlossen.
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
```

</details>

