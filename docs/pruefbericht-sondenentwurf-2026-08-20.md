> **Historischer Prüfbericht (20.08.2026) zur Fassung 0.3 des Entwurfs.** Er prüfte Korrektheit, nicht Produktentscheide. Pfade darin meinen den Stand vor dem Umzug vom 21.08. (`docs/visuelles-zielbild-hoerkompass.md` → `eq-copilot/design/prisma-studie/docs/`). Gültige Produktdefinition: `CLAUDE.md`.

# Prüfbericht — Sondenentwurf 0.3 (Codex)

- **Stand:** 2026-08-20
- **Geprüftes Dokument:** `docs/FL-Nakama-Sonden-Design-Entwurf.md`, Fassung 0.3
  (Commit `39859b8`, 2026-08-19 14:31)
- **Prüfer:** Claude (Opus 5), unabhängige Session
- **Methode:** jede Ist-Stand-Behauptung gegen den Quellcode; jede
  Plattform-Behauptung gegen den gevendorten JUCE-8.0.9-Quelltext; jede
  Formel numerisch nachgerechnet; Normen und Produktstände gegen
  Primärquellen. Nichts aus dem Gedächtnis.
- **Ich habe das geprüfte Dokument NICHT verändert** (Codex-Besitz,
  One-Writer).

---

## 0. Gesamturteil

Der Entwurf hält der Prüfung stand. **Keine einzige Ist-Stand-Behauptung
über den eigenen Code war falsch** — und das sind über zwanzig, teilweise
sehr spezifische (Class-IDs auf das Byte, Feldnamen, Gate-Ausdrücke,
Framegrenzen). Zwei schwer prüfbare JUCE-Wrapper-Aussagen, auf denen ein
ganzer Phasenschritt ruht, sind am Quelltext **wörtlich bestätigt**. Ein
Base32-Golden im Dokument stimmt **bitgenau** — es wurde also gerechnet,
nicht behauptet. Fünf DSP-Formeln reproduzieren im Nachbau exakt die
angegebenen Werte.

Das ist eine andere Qualitätsklasse als übliche AI-Audits (Erfahrungswert
dieses Projekts: ~25 % Fehlbefunde). Die Befunde unten sind deshalb
**keine Widerlegung**, sondern Nachschärfungen — mit einer wichtigen
Ausnahme (Befund A), die vor einer Bauentscheidung geklärt sein muss.

**Empfehlung:** Der technische Teil ist baureif. Vor `SONDE-001` fehlt
genau eine Klärung — die Versöhnung von Kernfunktion 1 mit dem
Hörkompass-Zielvertrag. Diese Klärung ist eine Produktentscheidung des
Users, keine technische.

---

## 1. Was bestätigt wurde

### 1.1 Code-Wahrheit — 20/20 korrekt

| Behauptung im Entwurf | Beleg | Ergebnis |
|---|---|---|
| Nur `processBlock(AudioBuffer<float>&)`, kein Double-Callback | `PluginProcessor.h:36` | ✅ |
| Hörmarker-Gate ist heute `playing \|\| !hasTransport` (fail-open) | `PluginProcessor.cpp:199` | ✅ wörtlich |
| `hatTransport` wird bei irgendeiner `PositionInfo` gesetzt, nie gelöscht | `PluginProcessor.cpp:135`, kein Reset | ✅ |
| `projektZeitSamples` wird nicht in jedem Block invalidiert | `PluginProcessor.cpp:139` | ✅ |
| Float-FIFO schreibt Teilblöcke, verliert Blockzeit | `PluginProcessor.cpp:171–184` (`schreibbar < n` ⇒ nur Zähler) | ✅ |
| Analyse-Abgriff liegt VOR der Färbung | `PluginProcessor.cpp:169` vs. `:200` | ✅ |
| C++ sendet `hoermarkierung`, Rust-`MessStand` kennt es nicht | `PipeClient.cpp:66` + `protokoll.rs:88–115` | ✅ (= NAK-10) |
| `AnalyseEngine::kZellen` wächst unbeschränkt | `AnalyseEngine.h:320`, nur `clear()`/`push_back()` | ✅ |
| `PLUGIN_MANUFACTURER_CODE=Evna`, `PLUGIN_CODE=Eqcp` | `plugin/CMakeLists.txt:15–16` | ✅ |
| Component-CID `ABCDEF019182FAEB45766E6145716370` | `moduleinfo.json:16` | ✅ bytegleich |
| Controller-CID `ABCDEF011234ABCD45766E6145716370` | `moduleinfo.json:32` | ✅ bytegleich |
| JUCE 8.0.9 gepinnt | `eq-copilot/CMakeLists.txt:13` | ✅ |
| State schema=1 mit `sensor_id`/`role`/`label`/`pair_id` | `PluginProcessor.cpp:405–413` | ✅ |
| `instance_nonce` war nie Projekt-State | fehlt in `getStateInformation` | ✅ |
| `role`-Enum `sensor\|hub\|pre\|post` | `eq-ipc.schema.json:36` | ✅ |
| Framegrenze 262 144 Bytes, u32-Präfix, 1-Hz-Heartbeat | `EqCopilotIds.h:20,23` | ✅ |
| Pipename `\\.\pipe\evenacadia.eq-copilot.v1` | `EqCopilotIds.h:17` | ✅ |
| v2-Schemas sind `additionalProperties:false` | alle fünf Schemas | ✅ |
| `pipe.start()` im Processor-Konstruktor | `PluginProcessor.cpp:37` | ✅ |
| `paare_auswerten()` existiert im Broker | `broker/src/lib.rs:309` | ✅ |
| Installer kopiert nur die VST3-DLL | `Install-EQ-Copilot.ps1:29` | ✅ |
| Broker-Frische über `SystemTime`, nicht `Instant` | `broker/src/lib.rs:32` | ✅ |
| Pipe-DACL `D:P(A;;GA;;;<SID>)`, keine Impersonation heute | `broker/src/server.rs:41,162` | ✅ |
| 221 LTAS-Bänder, FFT 16384/8192/4096/2048 | `AnalyseEngine.h:64,221–224` | ✅ |
| 8×-True-Peak-Pfad | `AnalyseEngine.h:325` (`kTpFaktor = 8`, 161 Taps) | ✅ |
| Passive Instanz hat null Hostparameter | keine APVTS/`addParameter` im Baum | ✅ |

### 1.2 JUCE-8.0.9-Plattformaussagen — am Quelltext bestätigt

Beide sind tragend (§44.3 / §32.3 hängen vollständig daran) und beide
stimmen wörtlich:

**(a) Parameterqueue — nur der letzte Punkt, Offset verworfen.**
`juce_audio_plugin_client_VST3.cpp:3637`:

```cpp
if (const auto change = getPointFromQueue (paramQueue, numPoints - 1))
    if (auto* param = comPluginInstance->getParamForVSTParamID (vstParamID))
        setValueAndNotifyIfChanged (*param, (float) change->value);
```

`offsetSamples` wird nur im `JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS`-
Zweig weitergereicht — und dort in den MIDI-Puffer, nicht an den Parameter.
Für normale Pluginparameter geht der Sampleoffset **vollständig verloren**.
Die geforderte Wrapper-Bridge ist damit sachlich zwingend, nicht optional.

**(b) Fehlender ProcessContext ist nicht unterscheidbar.**
`:3676–3688` — bei `data.processContext == nullptr` läuft `zerostruct
(processContext)`. `getPosition()` (`:3102`) gibt danach **immer** ein
belegtes `Optional` zurück und setzt `timeInSamples` **bedingungslos**:

```cpp
info.setTimeInSamples (jmax ((Steinberg::int64) 0, processContext.projectTimeSamples));
info.setTimeInSeconds (static_cast<double> (*info.getTimeInSamples()) / processContext.sampleRate);
```

**Zusatzbefund, den der Entwurf nicht nennt:** In diesem Pfad ist
`processContext.sampleRate` durch `zerostruct` ebenfalls 0 ⇒
`getTimeInSeconds()` rechnet `0.0 / 0.0` = **NaN**. Ein Host, der nie
einen ProcessContext liefert, produziert also eine NaN-Quelle im
JUCE-Wrapper selbst. Für den NaN-Riegel des Projekts relevant: `HostBlockContext`
muss `timeInSeconds` verwerfen, nicht sanitisieren.

**Praktische Folge für den fail-open-Marker:** Weil `getPosition()` im
VST3-Pfad **nie** `nullopt` liefert, wird `hatTransport` im ersten Block
jedes VST3-Hosts `true`. Der `!hatTransport`-Zweig ist damit im
ausgelieferten Format ein **toter Zweig** — das Gate wirkt praktisch
fail-closed. Der eigentliche Mangel ist deshalb nicht „Marker läuft bei
unbekanntem Transport", sondern: **`hatTransport` ist eine Tautologie und
kann „Transport unbekannt" gar nicht ausdrücken.** Der Entwurf beschreibt
das Symptom richtig, die Ursache ist eine Ebene tiefer — und seine Lösung
(`process_context_present` + eigene Validity-Bits) ist genau die richtige.

### 1.3 Rechnerisch nachgeprüft

| Angabe | Nachrechnung | Ergebnis |
|---|---|---|
| Base32-Golden `S-1-5-21-111111111-222222222-333333333-1001` → `BNSM62JZZCCXIDV3PJZAEHMZPA` | SHA-256 der UTF-8-Bytes, erste 128 bit, RFC-4648-Base32 ohne Padding | ✅ **bitgenau** |
| `Pmono=(PLL+PRR+2·Re{PLR})/4` gegen `Pstereo=(PLL+PRR)/2` | identisch 0,000 dB · unkorreliert −3,037 dB · gegenphasig −∞; deckt sich exakt mit dem direkt gefalteten Puffer | ✅ |
| `M=(L+R)/√2`, `S=(L−R)/√2` energieerhaltend | E(M)+E(S) = E(L)+E(R) auf 4 Nachkommastellen; mit `/2` nur halbe Energie | ✅ |
| `φᵢ = Re{Xᵢ·conj(Y)}`, `Σφᵢ = |Y|²` | relativer Fehler 3·10⁻¹⁶; negative Beiträge treten auf | ✅ |
| 16384 @ 48 kHz ≈ 341 ms | 341,33 ms | ✅ |
| Linear statt Equal-Power bei korreliertem Dry/Processed | Equal-Power erzeugt bei t=0,5 einen **+2,9 dB**-Buckel, linear bleibt flach; bei unkorreliert genau umgekehrt (linear −3,0 dB) | ✅ Regel quantitativ belegt |
| `min(20 kHz, 0,45·fₛ)` | 44,1 k → 19 845 Hz; 48 k → 20 kHz | ✅ sinnvoll |

### 1.4 Normen und Marktstand

| Angabe | Prüfung | Ergebnis |
|---|---|---|
| ITU-R BS.1770-5, 11/2023 | ITU-Rec-Seite, genehmigt 22.11.2023 | ✅ |
| EBU R 128 v5 + offizielles Testset v5.0 | tech.ebu.ch/loudness | ✅ |
| LRA erst nach ~60 s belastbar, vorher „unstable" | EBU Tech 3341/3342 | ✅ wörtlich |
| Neutron 5.2.0 · 04.02.2026 | iZotope Release Notes | ✅ Datum exakt |
| Ozone 12.1.0 · 01.12.2025 | iZotope Release Notes | ✅ Datum exakt |
| FabFilter Pro-Q 4.13 · 30.06.2026 | FabFilter News, verlinkte URL stimmt | ✅ Datum exakt |
| sonible smart:EQ 4 · 1.1.1 | Produktseite | ✅ |
| sonible pure:unmask · 1.0.1, braucht Sidechain | Produktseite | ✅ |
| FabFilter dokumentiert FL-Fehlreihenfolge bei Latenz | Pro-Q-4-Hilfe, Known Issues: *„FL Studio: Plug-ins … possibly won't re-order instances correctly when some of them introduce latency."* | ✅ wörtlich |
| arXiv:1803.09960 seit 2021 aus IP-Gründen zurückgezogen | arXiv v3, 05.01.2021, *„Need to resolve ownership of intellectual property"* | ✅ wörtlich |
| SQLite: `wal_autocheckpoint=0` ⇒ kein Autocheckpoint; `synchronous=FULL` ⇒ Sync je Commit; WAL nicht über Netzlaufwerk | sqlite.org/wal.html | ✅ alle drei |
| `maximumExpectedSamplesPerBlock` ist nur ein Hinweis | JUCE-Doku, „program defensively in case a buggy host exceeds this value" | ✅ |
| FL: Sidechain = ungehörter Send, Plugin sieht ihn als **zusätzliche Eingänge** | Image-Line-Handbuch Mixer | ✅ |

---

## 2. Befunde

### A · TRAGEND — Kernfunktion 1 kollidiert mit dem Hörkompass-Zielvertrag

**Sachlage.** `docs/visuelles-zielbild-hoerkompass.md` wurde am
**19.08.2026 um 09:45** committet (`ed678ea`) und ist laut `CLAUDE.md`
der **verbindliche visuelle Nordstern**, vom User wörtlich bestätigt.
Fassung 0.3 des Sondenentwurfs entstand **4¾ Stunden später** (14:31) und
**referenziert ihn mit keinem einzigen Wort** (Volltextsuche nach
„Hörkompass", „Prisma", „visuelles-zielbild", „design-stand",
„geschmacksprofil": **0 Treffer**).

**Der Widerspruch ist wörtlich.** Hörkompass §2.3/§2.4/§7:

> „Gesund = leeres Glas: Kein permanenter Energiewasserfall, kein
> dauerhaft volles Prüffeld und kein Ambient-Visualizer."
> „Befund statt Musik: Nur evidenzbasierte, priorisierte Befunde werden
> sichtbar. Rohenergie ist kein Dauerinhalt."
> Ausdrücklich **nicht** das Ziel: „**zwanzig gleichzeitig farbcodierte
> Instrumentenspuren im Glas**"; „ein Wasserfall aus Musikenergie".

Sondenentwurf §35.1 / §33.2 / §49.3:

> „Main zeigt drei Informationsdichten: kompakte Quellenliste,
> vergleichende **Heatmap** und Detailansicht."
> Live-Telemetrie **10 Hz**, 64 Bänder, **16 bis 32 Sonden**.
> Budget: „Livekarte, 16 Sonden < 300 ms p95 → sichtbarer Main-State."

16–32 Quellen × 64 Bänder × 10 Hz als Dauerbild **ist** der
Energiewasserfall, der am 18.08. schon einmal als „Wasserfall aus Pixeln"
verworfen wurde, und **ist** „zwanzig gleichzeitig farbcodierte
Instrumentenspuren".

**Es ist kein Schreibfehler, sondern strukturell.** §6 macht die Landkarte
zum Fundament aller elf anderen Funktionen („Die Landkarte liefert
Kontext"). Und §57 liefert sie als **erstes user-sichtbares Release
überhaupt** (`R1 · Passive Alpha`, P3) — also **bevor** in P5 die erste
Befundlogik existiert. R1 ist damit per Konstruktion ein Dauer-Visualizer
ohne einen einzigen Befund: exakt der verbotene Zustand.

**Erschwerend:** `offene-punkte.md` NAK-14 hält fest, dass die
Befund-Verkörperung noch **nicht einmal für 3–4 gleichzeitige Befunde**
lesbarkeitsgeprüft ist. Der Entwurf springt auf 16–32 Dauerquellen.

**Die Versöhnung existiert bereits und ist billig.** Hörkompass §5 erlaubt
ausdrücklich: *„Stufe 3 darf eine präzise, ruhige UI **neben** dem Objekt
verwenden."* Die Landkarte ist damit legitim — **als Fokusansicht nach
Klick**, nicht als Ruhezustand. Konkret nötig:

1. Der Sessiongraph bleibt technisch unverändert (er ist richtig gebaut).
2. Die **Ruhedarstellung** ist das leere Prisma, nicht die Heatmap.
3. `R1` liefert die Landkarte als **Diagnose-/Setup-Ansicht** (Sonde
   benennen, Join, Frische, Fehlerdiagnose) — nicht als Dauerarbeitsfläche.
   Das deckt sich mit §57s eigener Formulierung „minimale tägliche UX für
   Benennen, Join, Entfernen und Fehlerdiagnose".
4. §35.1s Satz zu den „drei Informationsdichten" braucht einen
   Vorbehalt: die Dichtestufen sind Fokus-Ebenen, keine Dauerzustände.

**Das ist eine Produktentscheidung des Users, keine technische.** Sie
muss vor `SONDE-001` fallen, weil sie das Exit-Gate von P3 definiert.

---

### B · KONKRETER FEHLER — Impersonation-Reihenfolge in §48.4

**Entwurf §48.4:**

> „Server impersoniert den Pipe-Client **unmittelbar nach Connect und vor
> einem akzeptierten Hello**, vergleicht dessen Token-User-SID mit der
> erwarteten SID …"

**Microsoft-Doku zu `ImpersonateNamedPipeClient`, Remarks:**

> „When this function is called, the named-pipe file system changes the
> thread of the calling process to start impersonating the security
> context **of the last message read from the pipe**."

Es gibt vor dem ersten Read **keine** „last message". „Unmittelbar nach
Connect" — also vor jedem Lesen — impersoniert damit nichts oder das
Falsche. Und genau dieser Fall ist der gefährliche, denn dieselbe
MSDN-Seite warnt:

> „If the **ImpersonateNamedPipeClient** function fails, the client is not
> impersonated, and all subsequent client requests are made in the
> security context of the process that called the function."

**Korrekte Reihenfolge:** `ConnectNamedPipe` → **Bootstrap-Hello lesen** →
`ImpersonateNamedPipeClient` → `GetTokenInformation`/SID vergleichen →
`RevertToSelf` (in jedem Pfad, auch im Fehlerpfad) → Hello **annehmen oder
ablehnen**. Der Rückgabewert muss geprüft werden; bei `FALSE` wird die
Verbindung geschlossen, nicht weitergearbeitet.

Die Formulierung „vor einem *akzeptierten* Hello" ist möglicherweise genau
so gemeint (lesen, dann impersonieren, dann akzeptieren) — dann ist nur
„unmittelbar nach Connect" irreführend. Weil hier ein Sicherheitsgate
hängt, muss die Reihenfolge im Text eindeutig sein. `security_vectors`
(§66.2) braucht zusätzlich einen Negativtest „Impersonation schlägt fehl
⇒ Verbindung geschlossen, kein Fallback in den Serverkontext".

Nebenbestätigung: Der Broker läuft unprivilegiert (kein
`SeImpersonatePrivilege`), aber die MSDN-Bedingung „The authenticated
identity is same as the caller" greift — Impersonation ist zulässig. ✅

---

### C · UNGENANNTE ABHÄNGIGKEIT — der CID-Freeze hängt an einem Define

Der Entwurf verlangt (§44.1): *„Ein Rename oder neues JUCE-Target darf
diese Werte nicht neu generieren."* Er nennt aber nicht, **wovon** das
abhängt. Am Quelltext (`juce_VST3ModuleInfo.h:56–61`):

```cpp
#if JUCE_VST3_CAN_REPLACE_VST2
    return VST3Interface::vst2PluginId (JucePlugin_VSTUniqueID, JucePlugin_Name, interfaceType);
#endif
    return VST3Interface::jucePluginId (JucePlugin_ManufacturerCode, JucePlugin_PluginCode, interfaceType);
```

`vst2PluginId` **hasht den Pluginnamen** (`juce_VST3Interface.h:104`:
`iid[index] = tolower(*pluginName)`). `jucePluginId` tut das nicht — die
CID ist dort ausschließlich `(manufacturerCode, pluginCode)` plus zwei
Interface-Konstanten.

Das Projekt setzt heute `JUCE_VST3_CAN_REPLACE_VST2=0`
(`plugin/CMakeLists.txt:37`). **Nur deshalb** ist der Wunsch des Entwurfs
erfüllbar, das Produkt intern „Nakama Main" nennen zu dürfen. Ein Flip auf
`1` würde jedes bestehende Projekt beim nächsten Laden verwaisen lassen.

⇒ `SONDE-001` muss **`JUCE_VST3_CAN_REPLACE_VST2=0` als Teil der
eingefrorenen Identität** aufnehmen, nicht nur die resultierende Hexfolge.

**Geschenk aus derselben Stelle:** Weil die CID rein abgeleitet ist,
müssen die neuen IDs in P0 gar nicht „erzeugt" werden — sie stehen fest.
Die Ableitung reproduziert die bestehende CID exakt
(`Evna`=`45766E61`, `Eqcp`=`45716370`), also gilt:

| Ziel | Component-CID | Controller-CID |
|---|---|---|
| `NkPr` Passive Probe | `ABCDEF019182FAEB45766E614E6B5072` | `ABCDEF011234ABCD45766E614E6B5072` |
| `NkAc` Active Probe | `ABCDEF019182FAEB45766E614E6B4163` | `ABCDEF011234ABCD45766E614E6B4163` |

`SONDE-001` reduziert sich damit auf *verifizieren statt würfeln*.

---

### D · SPEZIFIKATIONS-WIDERSPRUCH — Frame-Bündelung

Drei Stellen sagen Unvereinbares:

| Stelle | Aussage |
|---|---|
| §33.1 | „**Vier bis fünf Liveframes** dürfen pro Write gebündelt werden." |
| §49.3 | „höchstens **ein** aktueller Liveframe je Probe; alte Frames werden verworfen" |
| §53.9 | P2-Queue **Startcap 2**, Politik „ältesten ungesendeten Frame **ersetzen**" |

Bei Cap 2 mit *replace-oldest* können nie 4–5 Frames **einer** Sonde
auflaufen. Die Bündelung kann also nur den **Broker→Main**-Weg meinen
(dort liegen 16–32 Sonden vor) — §33.1 nennt „Broker→Main-Liveupdates"
tatsächlich als P2-Inhalt. Der Satz steht aber im Absatz über die
Quantisierung der Sondenframes und liest sich als Sonde→Broker.

Das ist nicht kosmetisch: Es entscheidet die **Form des
FlatBuffers-`FeatureBatch`** — trägt ein Batch *N Frames einer Quelle*
oder *je 1 Frame von N Quellen*? Beides zugleich geht nur mit einem
Wrapper-Level mehr. Das Schema ist P1-Lieferumfang (`SONDE-005`), also
muss die Frage **vor** P1-Abschluss entschieden sein, sonst wird sie
später zu einer Schema-Major-Änderung.

---

### E · ÜBERSCHRIEBENE ENGINEERING-ENTSCHEIDUNG — die gemeinsame Static-Lib

§53.4 und P1 verlangen „drei VST3-Ziele aus gemeinsamen **statischen
Bibliotheken**". Das aktuelle `plugin/CMakeLists.txt` sagt in seinem
Kopfkommentar das genaue Gegenteil — **mit Begründung**:

> „Probe und Tests kompilieren die geteilten Quellen erneut, statt eine
> static-lib mit fremden JUCE-Moduldefinitionen zu teilen — bewusst simpel."

`juce_add_plugin` erzeugt pro Target eigene Moduldefinitionen
(`JucePlugin_*`, `JUCE_MODULE_AVAILABLE_*`). Eine über drei Plugin-Targets
geteilte statische Bibliothek muss gegen **genau eine** dieser
Konfigurationen kompiliert werden — sonst bekommen zwei der drei Bundles
die Identitätskonstanten des dritten. Das trifft ausgerechnet die Werte,
die §44.1 einfrieren will.

Der Entwurf begründet den Bruch nicht und nennt das Risiko nicht. Kein
Blocker, aber `SONDE-007` braucht dafür einen expliziten Schritt: der
gemeinsame Kern darf **keine** `JucePlugin_*`-Konstanten sehen; Identität
kommt ausschließlich aus `plugin-identities-v1.json` über die dünnen
Target-Schichten. Andernfalls fällt genau das Identitäts-Golden, das die
Phase absichern soll.

---

### F · KLEINE LÜCKE — kein Abbruchweg für ein Experiment

§33.3 listet `experiment_begin`, `experiment_result`, `user_verdict` —
aber **kein** `experiment_abort`. §46.1 bietet nur *Rekonstruktion* eines
abgebrochenen Schritts, nicht sein Verwerfen. Damit fehlt dem
append-only-Store der Gegenpfad für den häufigsten Realfall: Der User
startet einen Versuch und misst die Passage nie erneut.

Das verletzt die eigene Regel (§67 Frage 2: „Wie werden Unbind,
Unsubscribe, Revert, Timeout, Remove oder Migration vollständig
ausgeführt?"). Die Liste ist mit „mindestens" offen — der Punkt gehört
trotzdem ins Schema, weil `experiments` sonst unbegrenzt offene Zeilen
sammelt und die Retention-Regel aus Roadmap 15 („benannte Experimente
bleiben, bis der User sie löscht") sie nie erfasst.

---

### G · ZITAT-ÜBERDEHNUNG — smart:EQ „Front/Middle/Back"

§37.1: *„das bei smart:EQ **offiziell als Front/Middle/Back bezeichnete**
Gruppenmuster"*. sonibles Produktseite formuliert:

> „bringing some elements to the forefront of the sonic stage, leaving
> some in the middle, and pushing some into the background."

Das Konzept ist real und dokumentiert; die Begriffe sind
**forefront/middle/background**, nicht „Front/Middle/Back" als offizielle
Bezeichnung. Nach dem eigenen Maßstab des Entwurfs („Übernommen werden nur
dokumentierte Fähigkeiten") gehört das entschärft. Auf die Technik hat es
keine Wirkung.

---

### H · DATUMS-NIT

§29.2 nennt ADPTR Metric AB `1.5.0 · 29.07.2026`; Plugin Alliance nennt
den **30.07.2026**. Ein Tag, ohne technische Folge. Es ist die **einzige**
Datumsabweichung in der ganzen Benchmarktabelle.

---

## 3. Risiko-Rangfolge der Spikes (§51)

Der Entwurf gewichtet die sieben Spikes nicht. Nach der Recherche ist die
Reihenfolge nicht gleichverteilt:

| Rang | Spike | Einschätzung |
|---:|---|---|
| **1** | **Aux-/Sidechainlayout** | **Höchstes Risiko.** FLs Sidechain ist mixer-, nicht pluginbasiert; Community-Befunde melden für Drittanbieter-VST3 *einen* Sidechain-Eingang zur Zeit, Mixer-Input 1 → Plugin-Eingänge 3/4. Der Entwurf braucht für die Active Probe **zwei getrennte** Aux-Busse (`priority_sidechain` + `compare_pre`) **gleichzeitig** und für Main **mehrere diskrete** Contribution-Busse. Fällt der Spike, sterben auf einen Schlag: Kernfunktion 17 komplett, das hörbare Delta in 5 und 12, und die exakte Attribution in 1. |
| 2 | Automation/Undo | Am Quelltext bereits als *fehlend* bewiesen (Befund 1.2a). Der Spike prüft nur noch, ob der Wrapper-Patch trägt — kein Erkenntnis-, sondern ein Umsetzungsrisiko. |
| 3 | FL-Zeit/PDC | Loop-Straddle und Presentation-Latency sind in FL erfahrungsgemäß unzuverlässig; der Fallback (rohe Projektzeit, herabgestufte Aussage) ist aber tragfähig. |
| 4 | Maskingkalibrierung | Kein Ja/Nein, sondern ein Korpusaufwand. Der Fallback ist echt. |
| 5 | Active-DSP · IPC v3 · Brokerstart · Plugin-IDs | Reine Ingenieurarbeit ohne Fremdabhängigkeit. Plugin-IDs sind durch Befund C sogar vorab gelöst. |

**Konsequenz:** `SONDE-004` (Aux/PDC) gehört an die **allererste** Stelle
der Umsetzungswelle, nicht an die vierte. Ein negatives Ergebnis ändert
den Produktumfang mehr als jeder andere Spike — und es ist billig zu
messen (eine `.flp`, zwei Impulse).

**Ehrliche Konsequenz im Releasemodell:** §53.2 verlangt für den Wegfall
von P8 eine *sichtbare Produktentscheidung*. Für den Wegfall von
`aux_compare_pre` verlangt sie das **nicht** — dabei entfällt damit der
Hörmodus `DELTA` (einer von vier in §41.1) und das hörbare PRE/POST-Delta
aus §38.4. Beides sollte demselben Sichtbarkeitsvorbehalt unterliegen.

---

## 4. Kleinere Beobachtungen ohne Handlungsdruck

- **§35.2 Σφᵢ-Mathematik.** Formal einwandfrei (nachgerechnet, 3·10⁻¹⁶),
  aber sie setzt voraus, dass **alle** `Xᵢ` und `Y` im **selben lokalen
  Audiocallback** vorliegen. Wenn Spike 1 (oben) fällt, ist dieser ganze
  Abschnitt totes Gewicht im Dokument. Das sollte er dann auch sagen.
- **`synchronous=FULL` + append-only `event_log`** heißt ein fsync pro
  Ereignis. Bei Evidenz-Kadenz 1–4 Hz × bis zu 32 Sonden ist Batching
  nötig; §53.9 nennt nur „kurze Transaktionen".
- **`wal_autocheckpoint=0` + PASSIVE-Checkpoint nur im Broker-Idle:** In
  einer langen Mischsession gibt es kaum Idle. Das P9-Gate („keine
  WAL-Datei wächst im Soak unbegrenzt") fängt es ab — aber die
  Checkpoint-Politik braucht ein Größen-Trigger, nicht nur ein
  Idle-Trigger.
- **§29.2 Titel der v2-Schemas.** `eq-ipc.schema.json` heißt noch
  „Plugin ↔ **Tauri**-Broker". Der Entwurf weist Tauri-Verweise
  korrekt als überholt aus; der Schematitel selbst ist noch nicht
  nachgezogen (out of scope des Entwurfs, gehört in die Doku-Hygiene).
- **Bassframe-Budget.** §49.3 nennt 750 ms p95 „erstes Fenstersample →
  UI" für die 16 384er Auflösung. Davon sind 341 ms reine Fensterdauer,
  bleiben ~409 ms für Rechnen, Transport und Zeichnen — gegenüber 300 ms
  im Kleinframe-Fall. Konsistent, **sofern** die Bassbänder auf dem
  10-Hz-Livepfad reisen. Reisen sie auf dem Evidenzpfad (1–4 Hz), reicht
  das Budget bei 1 Hz nicht. Der Entwurf legt das nicht fest.

---

## 5. Was der Entwurf besser macht als das Projekt bisher

Drei Punkte verdienen ausdrückliche Übernahme, unabhängig von der
Bauentscheidung:

1. **Der `hoermarkierung`-Befund ist eine unabhängige Wiederentdeckung von
   NAK-10.** Zwei getrennte Analysen, gleiches Ergebnis — das erhöht das
   Vertrauen in beide.
2. **§48.3 ist die konkrete Antwort auf NAK-13** (fehlender
   Broker-Autostart) und zwar eine bessere als „Autostart-Eintrag":
   klassifikationsgebundener On-demand-Start, Scanner startet nie,
   per-User-Mutex, Idle-Ende. Das ist umsetzbar, auch ohne den Rest des
   Sondenplans.
3. **§32.2 trennt vier Achsen** (`plugin_kind`, `measurement_position`,
   `pair_id`, `SourceIntent`), die heute in dem einen Feld `role`
   vermischt sind. Diese Trennung ist auch für das rein passive Produkt
   richtig und sollte nicht auf die Sonden warten.

---

## 6. Empfohlene Reihenfolge

1. **Befund A klären** — Produktentscheidung des Users: Ist die
   Mix-Landkarte Ruhezustand oder Fokusansicht? Ohne diese Antwort ist das
   P3-Exit-Gate nicht definiert.
2. **Befund C** in `SONDE-001` einarbeiten (Define einfrieren, die vier
   abgeleiteten CIDs eintragen statt erzeugen).
3. **`SONDE-004` (Aux/PDC) vorziehen** an Position 1 der Welle.
4. **Befund B** (Impersonation-Reihenfolge) im Text korrigieren und als
   Negativtest in `security_vectors` aufnehmen.
5. **Befund D** vor P1-Abschluss entscheiden (FeatureBatch-Form).
6. **Befunde E, F, G, H** als redaktionelle Nachträge in eine Fassung 0.4.

---

_Erstellt 2026-08-20. Alle Codezeilen gegen den Stand `5de1e3b` geprüft,
alle Onlinequellen am 20.08.2026 abgerufen._
