# NAK-312 — Validierung der Matrixprüfung 2 an der Quelle

| Feld | Wert |
|---|---|
| Ticket | NAK-312, Planschritt S25k, Etappe 1 „Spezifikation vor Code" |
| Auftrag | `docs/beweise/roh/NAK-312-matrixpruefung-2-validierung-auftrag.txt` (Dirigent, 21.09.2026) |
| Gegenstand | Prüfbericht `docs/beweise/roh/NAK-312-matrixpruefung-2-urteil.md` (URTEIL NEEDS_WORK: D-A, D-B, D-C, L-A, H-A) |
| Prüfform | frischer, lesender Validierer; nur Lesewerkzeuge und lesende Git-Kommandos; kein Bau, kein Test, kein Kanon, kein Skript, kein FL Studio, kein Python |
| HEAD zu Beginn | `a4494fad0a65a56eb1c1eb09a6a3a6596414ed17` (gemessen 03:00 Uhr) |
| HEAD vor dem Schreiben | `a4494fad0a65a56eb1c1eb09a6a3a6596414ed17` (gemessen 03:10 Uhr, gleich) |
| `git status --short` | zu Beginn und vor dem Schreiben identisch: `?? briefing-hub/`, `?? nimbalyst-local/` — beide fremd, nicht angefasst |
| Datum, Uhrzeit | 21.09.2026, 03:00 bis 03:1x Uhr (`date`, PC-Lokalzeit) |
| Modell | Opus 5 (`claude-opus-5`), lesender Hintergrundlauf |
| Arbeitsbaum | `C:\Users\phili\Projekte\Nakama`, Branch `master`, sichtbarer Checkout |
| Geschrieben | genau diese Datei, LF-Zeilenenden, untracked; nichts gestaget, nichts committet, nichts gepusht |

## 0. Wie die Zeilenangaben gemessen sind

Der Bericht nennt „ZIEL Zeile n" für `d5e3e33a`. Gemessen:
`git diff -U2 d5e3e33a HEAD -- docs/beweise/NAK-312.md` hat genau zwei Hunks,
`@@ -6,5 +6,5 @@` (Ersatz in derselben Zeile, Zeile 9) und
`@@ -2226,2 +2226,64 @@` (Anhang §13). Die Zeilen 1 bis 2226 sind deshalb am
ZIEL und am HEAD zeichengleich nummeriert; alle Zitate unten sind an der
Arbeitsbaumdatei gelesen und gelten damit auch am ZIEL.

Die Mitgliedschaft im Prüfbereich ist gerechnet aus
`git diff -U0 abb64275...d5e3e33a -- docs/beweise/NAK-312.md`, Hunkköpfe der
ZIEL-Seite. Berührte ZIEL-Bereiche, soweit hier gebraucht: 235–242, 382–404,
511–532, 614, 618–620, 657–660, 680, 706, 715–758, 761–764, 767–769, 771–774,
784–799, 802–804, 810–811, 818–820, 824, 848, 850–851, 853, 857–858, 869–870,
877–899, 1019–1033, 1096–1115, 1177–1197, 1232–1356, 1447–1459, 1471–1481,
1487–1559, 1607–1665, 1675, 1677, 1680–1690, 1697–1703, 1731–1757, 1802–1809,
1819–1820, 1824–1825, 1827, 1830. **Nicht** berührt sind unter anderem 380,
616–617, 697, 699, 701, 765, 766, 770, 817, 821, 834, 836.

---

# Teil 1 — Befund für Befund

## D-A — Namenskollision der neuen Zeilen M-81 und M-84

### a) Der beanstandete Satz, wörtlich

Der Bericht beanstandet zwei in der Runde geschriebene Stellen:

1. `docs/beweise/NAK-312.md:706`, Zeile **M-78**, Quellspalte:
   „`R-312-11; CLAUDE.md:51-52` (sinngemäß für den Hostregler, wie M-16); **M-84**"
2. `docs/beweise/NAK-312.md:1195-1196`, §7.3:
   „**M-84** verlangt weiter den geladenen Stand, M-11 misst den Gegenfall
   ‚neuer Gestus nach dem Recall', und M-78 misst den Fall ‚während'."

und acht weitere Stellen als „mitgerissen": 697 (zweimal), 699, 701 (zweimal),
765 (zweimal), 766, 770, 834 (zweimal), 380.

### b) Nachmessung an der Quelle

Die beiden fremden Zeilen, selbst gelesen:

- `docs/beweise/SONDE-015.md:1159`: „`| M-81 | Host-Automation läuft auf einem
  Parameter | Sie wirkt im flüchtigen **`AutomationOverlay`** mit
  `automation_epoch` und zählt **keine** Revision … Die Epoche wechselt einmal
  zu Beginn und einmal am Ende einer Hostgeste beziehungsweise nach einer
  definierten Ruhegrenze. |`"
- `docs/beweise/SONDE-015.md:1162`: „`| M-84 | Ein Projekt wird neu geladen |
  Der Reload rekonstruiert **denselben `state_hash`** und innerhalb numerischer
  Toleranz denselben Audioausgang. Reihenfolge: State lesen → validieren →
  Committed setzen → Hostparameter synchronisieren → erster Block. |`"

Die beiden neuen eigenen Zeilen, selbst gelesen:

- `docs/beweise/NAK-312.md:773`: „`| M-81 | T3-01-05 Teil b · E-312-5 ·
  Grenzzeile boolescher Parameter | …`" — Zusage: „**Auch der boolesche
  Parameter bleibt taktgebunden: er steht nicht in der Abdeckungstabelle
  (M-79) … Keine Gleichheitszusage für die elf booleschen Hostparameter**".
- `docs/beweise/NAK-312.md:851`: „`| M-84 | T3-07-02 · L3 · E-312-8 · Szenario
  S2 „weg von Main" | …`".

**Die Reproduktion stimmt.** Beide Nummern sind am HEAD doppelt belegt, und die
beiden beanstandeten Stellen sind so nicht haltbar:

- `:706`: M-78 ist eine Etappe-3-Zeile über den Ladestart; die eigene Zeile
  M-84 handelt von Rollenwechsel und Hörmarkierung in Etappe 6. Die Quellspalte
  zeigt damit auf eine Zusage, die über den Ladestart nichts sagt.
- `:1195-1196`: „M-84 verlangt weiter den geladenen Stand" ist als geschriebener
  Satz falsch, wenn M-84 nach der Regel von §6 die eigene Zeile meint. Die
  eigene Zeile M-84 (`:851`) verlangt keinen geladenen Stand.

**Drei Präzisierungen zur Reproduktion:**

1. Der Satz, den der Bericht als „in dieser Runde geändert" zitiert, ist es nur
   halb. Gemessen am Diff: `:616-617` („**Fremde Matrixzeilen (SONDE-015 M-nn,
   SONDE-012 U-nn, SONDE-013 M-nn, NAK-246 M-nn) stehen nur in der Quellspalte,
   nie als eigene Zeile**") sind **Kontextzeilen**, unverändert. Geändert hat
   die Runde die zweite Hälfte bei `:618`: aus „Zeile hier heißt immer M-01 bis
   **M-77** dieses Manifests" wurde „… M-01 bis **M-86** …" (Diffhunk
   `@@ -599,9 +654,10 @@` beziehungsweise ZIEL 618–620).
2. Von den acht „mitgerissenen" Stellen liegt **keine** im Diff der Runde
   (gerechnet oben, §0). Im Diff liegen allein die neuen Zeilen `:773` und
   `:851` sowie die beiden selbst geschriebenen Stellen `:706` und `:1195-1196`.
3. Die Zählung „sechs bestehende Matrixzellen" deckt sich nicht mit der eigenen
   Liste des Berichts (M-10, M-12, M-14 zweimal, M-29, M-51 = sechs Zellen für
   M-84; dazu M-24 zweimal und M-25 für M-81). Zählungen sind nach dem
   Prüfauftrag kein Befund; hier nur als Lesehilfe genannt.

### c) Welcher Satz bricht wirklich

- `docs/beweise/NAK-312.md:618` (in der Runde geändert), wörtlich: „eine Zeile
  hier heißt immer M-01 bis M-86 dieses Manifests." Dieser Satz weist die
  Nummern 81 und 84 den eigenen Zeilen zu. Erst dadurch lesen sich die bloßen
  `M-81` und `M-84` anderswo im Manifest als eigene Zeilen.
- `docs/beweise/NAK-312.md:616-617` (Kontext, nicht geändert): „**Fremde
  Matrixzeilen … stehen nur in der Quellspalte, nie als eigene Zeile**."
  Die Befundspalte von M-14 („T3-05-01 · **Hashhälfte M-84**", `:701`) und die
  von M-24 („T3-01-05 · **M-81** · aktivieren↔abklingen", `:765`) tragen eine
  fremde Zeile außerhalb der Quellspalte — beide liegen aber außerhalb des Diffs
  der Runde.
- Der Gegenbeweis des Berichts hält: `docs/beweise/NAK-312.md:1318`, in
  derselben Runde geschrieben, lautet „SONDE-015 M-81 wäre gebrochen. M-28 wacht
  darüber." Die Präfixform war der Runde bekannt.

### d) Urteil und Einordnung

**PRÄZISIERT · DEFEKT**, aber enger als beschrieben.

Ausschlussprüfung nach dem Prüfauftrag:

- „alles außerhalb des Diffs der Runde": schließt die acht mitgerissenen Stellen
  als eigene Befunde aus. Sie bleiben als **Wirkung** zulässig, weil die
  Defektklasse ausdrücklich trägt: „ein geänderter oder **neuer Satz bricht
  einen anderen Satz der Matrix**". Die neue Zeile `:851` macht acht bestehende
  Sätze zweideutig; das ist die Wirkung eines Satzes im Diff.
- „Prosa, Zeilenverweise, Zählungen, Abschnittsstände und Historie in docs/**
  außerhalb der inhaltlichen Zusagen von §5, §6, §7": greift **nicht** für
  `:706` (Quellspalte einer Matrixzeile) und **nicht** für `:1195-1196` (Satz
  des Bauplans §7.3 über die geltende Zusage). Beide liegen innerhalb der
  inhaltlichen Zusagen.
- „Kosmetik, Stil": greift nicht, siehe Wirksamkeitsprüfung.

**Wirksamkeit für einen Bauer, der nur das Manifest liest** — Stelle für Stelle
geprüft, im Diff der Runde liegen nur die ersten beiden:

| Stelle | im Diff | liest er eine falsche Zusage? |
|---|---|---|
| `:1195-1196` (§7.3) | **ja** | **ja.** Der Satz behauptet über die eigene Zeile M-84 etwas, das sie nicht sagt. Ein Bauer, der M-84 aufschlägt, findet Szenario S2 „weg von Main" und muss den Ladestartsatz für falsch halten |
| `:706` (M-78 Quelle) | **ja** | teilweise. Die Quelle zeigt ins Leere; die Zusage von M-78 selbst steht vollständig in der Zeile und bleibt richtig |
| `:765` (M-24 Befund und Quelle) | nein | **ja.** Die eigene Zeile M-81 sagt über Automationsepoche und Ruhegrenze nichts — sie sagt „**Keine** Gleichheitszusage für die elf booleschen Hostparameter" |
| `:766` (M-25 Quelle) | nein | **ja**, gleiche Lage wie `:765` |
| `:701` (M-14 Befund und Quelle) | nein | **ja.** „Hashhälfte M-84" führt in die Irre, weil die eigene Zeile M-84 (`:851`) selbst „**zwei Hälften**" trägt, aber keine Hashhälfte |
| `:697` (M-10 Test und Quelle) | nein | nein. Der Testverweis nennt `TransactionTestMain.cpp:2180-2200` und ist damit eindeutig; die eigene Zeile M-84 liegt in `MarkierungTestMain.cpp` |
| `:699`, `:770` (Quellen) | nein | nein. Die Zusage der Zeile steht für sich; nur der Zeiger geht fehl |
| `:834` (M-51) | nein | nein. „Reihenfolge M-84: State lesen, validieren, Committed setzen, Hostparameter synchronisieren, erster Block" schreibt die Reihenfolge wörtlich aus (identisch mit `SONDE-015.md:1162`); der Inhalt steht in der Zelle |
| `:380` (§5.4) | nein | nein. Die Stelle ist ein wörtliches Zitat von R-312-10 aus §2; `docs/beweise/NAK-312.md:183` trägt denselben Wortlaut „Ruhegrenze M-81" seit vor der Runde |

Also: an vier Stellen wirksam, an fünf nur Bezeichnerhygiene. Es ist **nicht
nur** Hygiene.

### e) Die schließende Regel

Jede Nennung einer fremden Matrixzeile trägt im ganzen Manifest ihr Präfix
(`SONDE-015 M-nn`, `SONDE-013 M-nn`, `SONDE-012 U-nn`, `NAK-246 M-nn`), und ein
bloßes `M-nn` bezeichnet ausnahmslos eine eigene Zeile M-01 bis M-86.

---

## D-B — M-38 behauptet einen Sperrenzähler, den der Code nicht führt

### a) Der beanstandete Satz, wörtlich

`docs/beweise/NAK-312.md:810`, Zeile **M-38**, Rotbeweisspalte (in dieser Runde
geändert, ZIEL 810–811 im Diff):

„Gebrochen wird deshalb an der Zeile, die die Zusage trägt: den Provider
`v3Status()` aus `processBlock` rufen → er nimmt dort `zustandSchloss`
(`SondeProcessor.cpp:1139`) und seine Zeichenketten (`:1145`, `:1172`)
allokieren im Bereich → **beide Zähler steigen** → rot"

### b) Nachmessung an der Quelle

- `eq-copilot/plugin/sonde/SondeProcessor.cpp:1139`:
  `        const juce::ScopedLock l (zustandSchloss);`
  — eine gewöhnliche JUCE-Sperre, **kein** `RtWache::GemeldeteSperre`.
- `eq-copilot/plugin/dsp/DspRtWache.h:50-53`:
  `    static void meldeSperre() noexcept`
  `    {`
  `        if (imAudiopfad()) sperrenZaehler().fetch_add (1, std::memory_order_relaxed);`
  `    }`
- `eq-copilot/plugin/dsp/DspRtWache.h:82-86`: der einzige Aufrufer ist
  `GemeldeteSperre`:
  `        explicit GemeldeteSperre (Lockable& l) noexcept (noexcept (l.lock())) : sperre (l)`
  `        {`
  `            RtWache::meldeSperre();`
  `            sperre.lock();`
  `        }`
- `eq-copilot/plugin/sonde/SondeProcessor.cpp:1145`:
  `            s.stateHash = nakama::transaktion::alsText (transaktion->hash()).toStdString();`
- `eq-copilot/plugin/sonde/SondeProcessor.cpp:1172`:
  `                s.runtime.label = zustand.common.label.toStdString();`
- `docs/beweise/NAK-312.md:1088-1090` (§7.2): „`setNonRealtime` nimmt
  `zustandSchloss` über `RtWache::GemeldeteSperre<Adapter>` statt über
  `juce::ScopedLock` (`SondeProcessor.cpp:1325`)." — und
  `eq-copilot/plugin/sonde/SondeProcessor.cpp:1325` ist heute
  `    const juce::ScopedLock l (zustandSchloss);`.

**Die Reproduktion stimmt.** `RtWache::sperren()` kann durch die genannte
Mutation nicht steigen.

**Eine Präzisierung und eine Erweiterung:**

1. Auf dem Weg `v3Status()` liegen **drei** Sperrnahmen, nicht eine — alle drei
   als gewöhnlicher `juce::ScopedLock`, alle drei für `RtWache::sperren()`
   unsichtbar: `:1139` (`zustandSchloss`), `:1184` (`getCallbackLock()`),
   `:1192` (`hostKontextSchloss`). Von `zustandSchloss` selbst liegt genau eine
   Nahme auf dem Weg, nämlich `:1139`.
2. Die **Allokationshälfte trifft heute einen anderen Zähler als den, den die
   Zusage nennt.** M-38 sagt zu: „`RtWache::sperren()` = 0 und
   **`RtWache::allokationen()`** = 0". Gemessen:
   - `eq-copilot/plugin/tests/TransactionTestMain.cpp:76-91` zählt in einen
     **eigenen** thread-lokalen Zähler (`:72-73`
     `thread_local bool zaehleAllokationen = false;` /
     `thread_local std::uint64_t allokationen = 0;`, Inkrement bei `:78`
     `if (zaehleAllokationen) ++allokationen;`) und ruft
     `RtWache::meldeAllokation()` **nicht**.
   - Der einzige Ort im Baum, der `RtWache::meldeAllokation()` verdrahtet, ist
     B6: `eq-copilot/plugin/tests/DspGoldenTestMain.cpp:100` und `:109`
     (`if (zaehleAllokationen) { ++allokationen; RtWache::meldeAllokation(); }`).
   - B7 liest `RtWache::allokationen()` heute nirgends; der vorhandene Fall
     `eq-copilot/plugin/tests/TransactionTestMain.cpp:3368` liest allein
     `const auto sperren = dsp::RtWache::sperren();` und prüft bei `:3374-3376`
     `imCallback == 0 && sperren == 0` mit dem eigenen Zähler.

### c) Welcher Satz bricht wirklich

- `docs/beweise/NAK-312.md:680`, Zeile **M-04**, wörtlich: „Etappen 2 bis 4:
  **heute nicht messbar** (heute `juce::ScopedLock` bei
  `SondeProcessor.cpp:1325`, für den Zähler unsichtbar)". M-38 behauptet für
  dieselbe Sperre das Gegenteil.
- `docs/beweise/NAK-312.md:1088-1090` (§7.2, Bauplansatz): der gemeldete Adapter
  geht an **genau eine** Stelle, `:1325`. Keine andere Nahme von
  `zustandSchloss` wird gemeldet.
- `docs/beweise/NAK-312.md:644-645` (Regel der Matrix): „Ein Rotbeweis fällt an
  der Zeile, die die Zusage trägt, nie an einem Nebeneffekt."
- Satz des Prüfauftrags zu H3: „der Rotbeweis fällt an der Zeile, die die Zusage
  trägt".
- Zusätzlich brechen, an derselben Zählerfrage, die Zusagen von M-01 (`:677`),
  M-02 (`:678`), M-26 (`:767`) und M-53 (`:836`), die „`RtWache::allokationen()`
  = 0" in B7 zusagen, während weder Manifest noch Code B7s `operator new` an
  `RtWache::meldeAllokation()` bindet. Von diesen liegen `:767` und `:810` im
  Diff der Runde, `:677`, `:678` und `:836` nicht.

### d) Urteil und Einordnung

**PRÄZISIERT · DEFEKT.** Die Sperrenhälfte der Begründung ist an der Quelle
widerlegt; die Zeile liegt im Diff; kein Ausschluss greift (weder Prosa noch
Kosmetik noch Ticketfremdes — es geht um die Rotbeweisspalte einer geänderten
Matrixzeile). Präzisiert wird die Wirkung: der Rotbeweis fällt, aber über B7s
**eigenen** thread-lokalen Allokationszähler, nicht über
`RtWache::allokationen()`, wie die Zusage ihn nennt.

### e) Die schließende Regel

Eine Zeile nennt als Zähler nur, was ihr Bein am ZIEL wirklich liest, und als
Mutation nur eine, die genau diesen Zähler an der Zusagezeile bewegt; eine
Sperre darf ein Rotbeweis nur behaupten, wo sie über `RtWache::GemeldeteSperre`
genommen wird.

---

## D-C — M-81, die Mutationsfolge widerspricht der Schiedsregel derselben Runde

### a) Der beanstandete Satz, wörtlich

`docs/beweise/NAK-312.md:773`, Zeile **M-81** (neu, ZIEL 771–774 im Diff),
Rotbeweisspalte:

„**Regressionswache**; einmal gebrochen: `eq_enabled` in die Abdeckungstabelle
aufnehmen → es gibt kein Rampenziel dafür, der Blockrand quittiert den Zähler
trotzdem und der Wert verschwindet für den Worker → der Schalter wirkt gar nicht
mehr → rot"

### b) Nachmessung an der Quelle

- `docs/beweise/NAK-312.md:1299-1301` (§7.3 Punkt 1, in derselben Runde
  geschrieben): „Der Blockrand liest in **jedem** Block die vier Zähler (vier
  relaxed Loads) und übernimmt einen Wert genau dann, wenn
  `hostEreignis[i] != blockrandStand[i]`; danach `blockrandStand[i] = n`."
- `docs/beweise/NAK-312.md:1314-1316` (§7.3 Punkt 5): „`hostEreignisOffen`
  (`SondeProcessor.h:544`) behält **genau einen** Verbraucher, den Worker
  (`SondeProcessor.cpp:1455`). Der Blockrand liest und tauscht es nicht."
- `docs/beweise/NAK-312.md:1323-1324` (§7.3, Kosten): „im Kern vier
  `std::uint32_t` (**Audiothread-eigen, nicht atomar**) = 16 Byte".
- `eq-copilot/plugin/sonde/SondeProcessor.cpp:1455`:
  `    if (hostEreignisOffen.exchange (false, std::memory_order_relaxed))`
- `:1459`:
  `            const auto n = hostEreignis[(size_t) i].load (std::memory_order_relaxed);`
- `:1460`: `            if (n == hostEreignisGesehen[(size_t) i]) continue;`
- `:1461`: `            hostEreignisGesehen[(size_t) i] = n;`
- `:1462`:
  `            transaktion->automationSchreiben (i, zelleAusHost (i, hostWert[(size_t) i].load (std::memory_order_relaxed)));`
- `eq-copilot/plugin/dsp/DspProgramm.cpp:263`:
  `    aus.eqEngagiert  = w[(size_t) param::kIndexEqEnabled].b;`
- `eq-copilot/plugin/dsp/DspKern.cpp:620-623` und `:626`: genau vier
  `setzeZiel`-Aufrufe für `input`, `output`, `mix`, `width`, dazu `autoGain`.
  Für `eq_enabled` gibt es kein Rampenziel.
- `eq-copilot/plugin/state/NakamaParameter.cpp:79`:
  `    t[i++] = boolean    ("v2.global.eq_enabled", false, false, true, 2);`

**Die Reproduktion stimmt.** Der Blockrand schreibt nach §7.3 allein
`blockrandStand[i]`, ein audiothread-eigenes, nicht atomares Feld. Der Worker
entscheidet über `hostEreignis[i]` gegen `hostEreignisGesehen[i]` (`:1459-1461`)
und wird vom Blockrand nicht berührt. Der Wert verschwindet für ihn also nicht;
`eq_enabled` wirkt weiter über `aus.eqEngagiert` (`DspProgramm.cpp:263`).

### c) Welcher Satz bricht wirklich

- `docs/beweise/NAK-312.md:1299-1301` (§7.3 Punkt 1) und `:1314-1316` (Punkt 5)
  — beide in derselben Runde geschrieben, beide im Diff. Die Kette von M-81
  widerspricht ihnen frontal.
- `docs/beweise/NAK-312.md:644-645`: „Ein Rotbeweis fällt an der Zeile, die die
  Zusage trägt, nie an einem Nebeneffekt."
- Die Kette, die M-81 beschreibt, existiert im Manifest — aber als Mutation
  einer **anderen** Zeile: `docs/beweise/NAK-312.md:769`, M-28, zweite
  Gegenprobe: „den Blockrand `hostEreignisOffen.exchange` rufen lassen → der
  Worker verliert die Meldung, Overlay, Epoche und Ruhegrenze (`:1457-1473`)
  laufen nicht mehr → M-24 und M-25 rot." M-81 hat diese Kette übernommen und
  an eine Mutation gehängt, die sie nicht auslöst.

### d) Urteil und Einordnung

**PRÄZISIERT · DEFEKT.** Zeile und widersprochene Bauplanpunkte liegen
sämtlich im Diff der Runde; kein Ausschluss greift.

Präzisiert wird, wo die Zeile dann fällt: nicht nur an M-79. Die Mengenprüfung
ist **auch die erste Zusagehälfte von M-81 selbst** („er steht nicht in der
Abdeckungstabelle (M-79)", `:773`). Der Rotbeweis fällt damit an einer Zeile,
die die Zusage trägt — er ist kein Nebeneffekt. Falsch ist allein die
geschriebene **Begründung**, und die ist Teil der Rotbeweisspalte.

### e) Die schließende Regel

Die Mutation einer Zeile nennt nur die Folge, die der Bauplan derselben Runde
für sie hergibt; eine Kette, die einer anderen Zeile gehört, wird nicht
übernommen, und eine Grenzzeile ohne eigenen Wirkpfad nennt als Bruch die
Prüfung, die sie selbst trägt.

---

## L-A — der neue irreführende Kommentar ist nirgends zugeordnet

### a) Der beanstandete Satz, wörtlich

`docs/beweise/NAK-312.md:1635-1639` (§7.6 Punkt 2, ZIEL 1607–1665 im Diff):

„2. Damit ein Block, der die neue Klassifikation sieht, **sicher auch** die
Rücknahme sieht, wird das Lesen von `istMainKlassifiziert` im Audiothread von
`relaxed` auf `acquire` gehoben
(`eq-copilot/plugin/src/PluginProcessor.cpp:874`); der Briefkasten überträgt
schon mit `acq_rel` (`HoerMarkierung.h:360-361`). Der Store bleibt `seq_cst` wie
heute."

### b) Nachmessung an der Quelle

- `eq-copilot/plugin/src/prozessor/State.cpp:270-271`:
  `    // eine Frage, die ein Leser jedes Mal neu beantworten muesste. Gelesen`
  `    // wird im processBlock relaxed - dort haengt kein anderer Wert daran.`
  (Der Kommentarblock läuft von `:264` bis `:271`, der Store steht bei `:272`:
  `    istMainKlassifiziert.store (lebenslauf.audioAusnahmeErlaubt());`.)
- `eq-copilot/plugin/src/PluginProcessor.cpp:874`:
  `    const bool erlaubt = istMainKlassifiziert.load (std::memory_order_relaxed)`
- `docs/beweise/NAK-312.md:116-120` (§1) nennt **genau drei** irreführende
  Kommentare: „`eq-copilot/plugin/src/PluginEditor.cpp:731-733` (behaupteter
  SafePointer-Schutz für den Prozessorzugriff), `PluginEditor.cpp:268-269` (drei
  Sicherheitsnetze, die in der Main-Fläche nicht ticken),
  `eq-copilot/plugin/sonde/SondeProcessor.cpp:153` (Taktbezug der Automation)."
- `docs/beweise/NAK-312.md:1561` (§7.5): „**Die drei irreführenden Kommentare**
  gehören in denselben Änderungssatz:".
- `docs/beweise/NAK-312.md:1618` (§7.6): „**Gebaut wird stattdessen:** in
  `spiegleKlassifikation` (`State.cpp:262-273`), **vor** dem Store
  `istMainKlassifiziert.store (…)` (`:272`) …" — die Kommentarzeilen `:270-271`
  liegen in dieser Spanne.
- Gesucht und **nicht gefunden**: ein Satz in §7.6, der eine Berichtigung dieses
  Kommentars verlangt (`git grep -n 'Kommentar'` über 1469–1810 liefert nur
  `:1561`, `:1670` zu `PluginEditor.cpp:268-269` und `:1728`).

**Die Reproduktion stimmt**, mit einer Verfeinerung der Zeilenangabe: der
doppelt falsche Halbsatz steht bei `:270-271`, nicht bei `:264-271`.

### c) Welcher Satz bricht wirklich

Keiner. §1 (`:116`) stellt die Regel „**Irreführende Kommentare** (gehören in
den Änderungssatz, der die Stelle repariert)" nur für die dort aufgezählten drei
auf; §8 Zeile E (`:1829`) hakt ebenfalls nur „die drei irreführenden Kommentare
aus §1" ab. Für einen Kommentar, den der Bau **selbst neu** falsch macht,
schweigen Regeln und Quellen. Es gibt keine Invariante und keinen Gate-Satz über
Kommentarwahrheit.

### d) Urteil und Einordnung

**BESTÄTIGT · LÜCKE.** Genau die Klasse des Prüfauftrags: „Regeln und Quellen
schweigen zu einem Fall, den der Diff neu aufwirft — benennen, keine
Nacharbeit." Der auslösende Satz (§7.6 Punkt 2) liegt im Diff, der Ausschluss
„außerhalb des Diffs" greift nicht.

### e) Die schließende Regel

Ein Änderungssatz, der einen bestehenden Kommentar falsch macht, berichtigt ihn
im selben Änderungssatz — die Regel aus §1 gilt nicht nur für die dort
aufgezählten drei, sondern auch für jeden, den der Bau neu erzeugt.

---

## H-A — die Schleusenhälfte von M-38 hat keine eigene Mutation

### a) Der beanstandete Satz, wörtlich

`docs/beweise/NAK-312.md:810`, Zeile **M-38**, Zusagespalte (unverändert):

„**Weder Schleuse noch Lebendprüfung liegen im Audiopfad: `RtWache::sperren()` =
0 und `RtWache::allokationen()` = 0; der Callbackpfad der Schleuse arbeitet nur
mit Atomics**"

Reihenfolgespalte: „`eq-copilot/plugin/core/ipc/controlclient/Schleuse.h:30-37`:
`warteMutex` gehört ausschließlich dem Schließpfad".

### b) Nachmessung an der Quelle

- `eq-copilot/plugin/core/ipc/controlclient/Schleuse.h:30-37`:
  `// Im Callback-Pfad (`betreten`, `Zug::~Zug`) gibt es KEINE Sperre, nur`
  `// Atomics: die Callbacks laufen unter `sendeMutex` des ControlClients und`
  `// nehmen den Sendezustand des Prozessors - eine Schleusensperre dort waere`
  `// eine neue Verschachtelung, die dieses Projekt nirgends fuehrt. `warteMutex``
  `// und `warte` gehoeren ausschliesslich dem Schliesspfad. …`
- `docs/beweise/NAK-312.md:2074-2075` (§11.3, E-312-10), wörtlich: „`Schleuse.h`
  bekommt keinen Haken; die Mutation „zweites Lesen in `betreten()` entfernen"
  entfällt als Rotbeweis."
- `eq-copilot/plugin/core/ipc/controlclient/Schleuse.h:121-139`, der Pfad, den
  die Zusagehälfte meint, selbst gelesen: `:121`
  `    [[nodiscard]] Zug betreten() noexcept`, und darin ausschließlich Atomics
  — `:123` `        if (geschlossen.load (std::memory_order_seq_cst))`, `:128`
  `        aktivZaehler.fetch_add (1, std::memory_order_seq_cst);`, `:131` das
  zweite Lesen. Keine Sperre, kein Warten. Die Zusage stimmt heute; genau
  deshalb kann sie nur ein eingebauter Haken brechen.
- Die berichtigte Mutation von M-38 (`:810`) greift `v3Status()` an, nicht
  `Schleuse.h`. Sie bewegt die Schleusenhälfte der Zusage nicht.

**Die Reproduktion stimmt.**

### c) Welcher Satz bricht wirklich

Keiner. Die Regel `docs/beweise/NAK-312.md:644-645` („Ein Rotbeweis fällt an der
Zeile, die die Zusage trägt") ist für die Zählerhälfte erfüllt; für die
Schleusenhälfte verlangt keine Zusage eine eigene Mutation, und E-312-10
(`:2074-2075`) schließt den Weg, der eine hergäbe, ausdrücklich aus.

### d) Urteil und Einordnung

**BESTÄTIGT · HÄRTUNG.** Der Prüfauftrag schließt „die Entscheide des Dirigenten
in §11.3 als solche" aus; ein Befund gegen E-312-10 wäre unzulässig. „Von keiner
Zusage verlangt — benennen, keine Nacharbeit" trifft genau zu.

### e) Die schließende Regel

Trägt eine Zeile zwei Zusagehälften, sagt sie je Hälfte, ob ein Rotbeweis sie
trägt oder ob sie ausdrücklich ohne eigene Mutation geführt wird und warum —
nach dem Muster des Etiketts „zwei Hälften" (`docs/beweise/NAK-312.md:639-640`).

---

# Teil 2 — Entscheidungsgrundlagen

## 2.1 Zu D-A: vollständige Liste der bloßen `M-nn` mit fremder Bedeutung in §5 bis §8

Abschnittsgrenzen am HEAD: §5 = 222–613, §6 = 614–900, §7 = 901–1810,
§8 = 1811–1874. Gesucht mit `git grep -n -oE 'M-[0-9]+'` über diesen Bereich,
je Treffer am Kontext entschieden. Eigene Zeilen sind M-01 bis M-86, lückenlos
(Zeilenköpfe `| M-nn |` bei 677–685, 697–707, 761–774, 802–811, 817–825,
831–837, 848–873).

**A. Bloß geschrieben und mit einer eigenen Zeile kollidierend:**

| Manifestzeile | Text (gekürzt) | gemeinte fremde Zeile | eigene Zeile gleicher Nummer | im Diff |
|---|---|---|---|---|
| `:380` | „Ruhegrenze **M-81** und Epochenfolge" (Zitat von R-312-10 aus §2, wortgleich mit `:183`) | `SONDE-015.md:1159` | `:773` M-81 Grenzzeile boolescher Parameter | nein |
| `:677` | M-01, Quelle „R-312-1; **M-47**; `CLAUDE.md:29-33`" | `SONDE-015.md:1100` | `:825` M-47 Zahlenrand Blockgröße 1 | nein |
| `:678` | M-02, Testspalte „B6 `der_zaehler_trennt_worker_und_audiothread (**M-47**)`" — Testname aus `DspGoldenTestMain.cpp:6672-6674` | `SONDE-015.md:1100` | `:825` | nein |
| `:680` | M-04, Quelle „R-312-1; **M-47**; E-312-10" | `SONDE-015.md:1100` | `:825` | **ja** |
| `:697` | M-10, Test „neben dem **M-84**-Block (`TransactionTestMain.cpp:2180-2200`)" und Quelle „**M-84**; R-312-10 zweiter Teil" | `SONDE-015.md:1162` | `:851` M-84 Szenario S2 | nein |
| `:699` | M-12, Quelle „**M-84**; R-312-10 zweiter Teil" | `SONDE-015.md:1162` | `:851` | nein |
| `:701` | M-14, Befund „Hashhälfte **M-84**" und Quelle „**M-84** (Hashhälfte)" | `SONDE-015.md:1162` | `:851` | nein |
| `:706` | M-78, Quelle „… wie M-16); **M-84**" | `SONDE-015.md:1162` | `:851` | **ja** |
| `:765` | M-24, Befund „T3-01-05 · **M-81** · aktivieren↔abklingen" und Quelle „**M-81**" | `SONDE-015.md:1159` | `:773` | nein |
| `:766` | M-25, Quelle „**M-81**; `CLAUDE.md:201-203`" | `SONDE-015.md:1159` | `:773` | nein |
| `:767` | M-26, Quelle „`CLAUDE.md:29-33`; **M-47**; R-312-10 (Satz 3); E-312-5" | `SONDE-015.md:1100` | `:825` | **ja** |
| `:770` | M-29, Quelle „R-312-10 (beide Teile); **M-84**" | `SONDE-015.md:1162` | `:851` | nein |
| `:810` | M-38, Quelle „`CLAUDE.md:29-33`; **M-47**; H3 der Matrixprüfung 1" | `SONDE-015.md:1100` | `:825` | **ja** |
| `:817` | M-40, Quelle „**M-47**; `CLAUDE.md:29-33`; R-312-3" | `SONDE-015.md:1100` | `:825` | nein |
| `:821` | M-43, Quelle „SONDE-015 E-31 (`SONDE-015.md:3425`); **M-55**" (die Rotbeweisspalte derselben Zeile schreibt korrekt „M-55 von SONDE-015") | `SONDE-015.md:1114` | `:848` M-55 Szenario S1 | nein |
| `:834` | M-51, „Reihenfolge **M-84**: State lesen, validieren, …" und Quelle „**M-84**; R-312-4" | `SONDE-015.md:1162` | `:851` | nein |
| `:836` | M-53, Quelle „R-312-4 (letzter Satz); **M-47**; `CLAUDE.md:29-33`" | `SONDE-015.md:1100` | `:825` | nein |
| `:853` | M-59, Test „neben dem **M-36**-Fall (`MarkierungTestMain.cpp:449-547`)" — Fallname nach SONDE-013 | `SONDE-013.md:521` | `:808` M-36 Panel öffnen↔schließen | **ja** |
| `:1195` | §7.3 „**M-84** verlangt weiter den geladenen Stand" | `SONDE-015.md:1162` | `:851` | **ja** |

**Vom Prüfer nicht genannt sind davon:** `:677`, `:678`, `:680`, `:767`,
`:810`, `:817`, `:821`, `:836`, `:853`. Sechs von ihnen kollidieren über M-47,
eine über M-55, eine über M-36 — sämtlich Nummern, die **schon vor der Runde**
eigene Zeilen waren (M-01 bis M-77). Die Runde hat diese Kollisionen also nicht
erzeugt, sondern nur an vier Stellen (`:680`, `:767`, `:810`, `:853`) fortge-
schrieben. Neu erzeugt hat sie allein die Kollisionen über **81** und **84**.

**B. Bloß geschrieben, ohne Kollision (Nummer über 86, eigene Zeilen enden bei
M-86):**

| gemeinte fremde Zeile | Manifestzeilen mit bloßem Bezeichner |
|---|---|
| `SONDE-015.md:1175` (M-92, unbekannter Major) | `:702` (zweimal: „M-92-Block" und Quelle), `:703`, `:704` |
| `SONDE-015.md:1223` (M-119, samplegenaue Automation) | `:370` (im Zitat aus `SondeProcessor.cpp:151-154`), `:1261` |
| `SONDE-015.md:1224` (M-120, Realtime/Offline-Render) | `:377`, `:385`, `:390`, `:458`, `:462`, `:761`, `:762`, `:763`, `:764`, `:768`, `:769`, `:772`, `:817`, `:818`, `:819`, `:820`, `:822`, `:823`, `:825`, `:1384` |

**C. Korrekt präfixiert (zur Abgrenzung, kein Befund):** `:268`, `:327`,
`:331`, `:349`, `:364`, `:402-403`, `:442`, `:444`, `:469`, `:548-549`, `:567`,
`:573`, `:742`, `:1258`, `:1263`, `:1318`, `:1354`, `:1377`, `:1747`. Zwei
Stellen präfixieren nachgestellt und sind damit ebenfalls eindeutig: `:821`
(Rotbeweisspalte „M-55 von SONDE-015 (`SONDE-015.md:1114`)") und `:1603`
(„bricht M-55 von SONDE-015 — M-43 wacht darüber").

**Randnotiz, außerhalb §5 bis §8:** derselbe bloße Gebrauch steht im
Produktcode: `eq-copilot/plugin/sonde/SondeProcessor.cpp:1454`
`    // Hostereignisse -> AutomationOverlay (M-81): keine Revision, kein Undo.`
— gemeint ist `SONDE-015.md:1159`. Kein Prüfgegenstand, aber die Regel aus e)
trifft die Stelle, sobald Etappe 3 sie anfasst.

## 2.2 Zu D-B: Sperrnahmen, Zähler und die wirksame Mutation

**Nahmen von `zustandSchloss` auf dem Weg `v3Status()`** (`SondeProcessor.cpp`,
Funktion `:1134-1210`): **genau eine**, `:1139`
`        const juce::ScopedLock l (zustandSchloss);`. Auf demselben Weg liegen
zwei weitere gewöhnliche Sperren auf anderen Mutexen: `:1184`
`            const juce::ScopedLock l (getCallbackLock());` und `:1192`
`        const juce::ScopedLock l (hostKontextSchloss);`. Keine der drei ist
eine `RtWache::GemeldeteSperre`.

**Was `RtWache` zählt** (`eq-copilot/plugin/dsp/DspRtWache.h`):

- `:36-38` `tiefe()` als thread-lokale Verschachtelungstiefe,
  `imAudiopfad() { return tiefe() > 0; }`.
- `:41-44` `meldeAllokation()` — erhöht nur im Audiopfad; Aufrufer ist allein
  ein Test-`operator new`, der ihn verdrahtet.
- `:50-53` `meldeSperre()` — erhöht nur im Audiopfad; einziger Aufrufer ist
  `GemeldeteSperre` (`:84`).
- `:59-63` `meldeAbleitung()` — getrennt nach Rechenort.

Eine `juce::ScopedLock` ist für `sperren()` unsichtbar; das schreibt M-04
(`:680`) selbst und die Rotbeweiszelle von M-38 (`:810`) im ersten Halbsatz
sogar wörtlich („eine Sperre in `betreten()` bliebe für diesen Zähler
unsichtbar") — bevor sie im zweiten Halbsatz das Gegenteil für `:1139`
behauptet.

**Was §7.2 über den gemeldeten Adapter sagt** (`:1084-1094`): die Sonde bekommt
einen zweizeiligen Adapter, und „`setNonRealtime` nimmt `zustandSchloss` über
`RtWache::GemeldeteSperre<Adapter>` statt über `juce::ScopedLock`
(`SondeProcessor.cpp:1325`). Dieselbe Sperre, dieselbe Sperrenordnung, ein
zusätzlicher thread-lokaler Test." Genau eine Stelle, sonst keine.

**Welche Mutation die Zusage von M-38 wirklich rot macht, und über welchen
Zähler.** Die Zusage trägt zwei Hälften:

1. *„Weder Schleuse noch Lebendprüfung liegen im Audiopfad: `RtWache::sperren()`
   = 0 und `RtWache::allokationen()` = 0."* Die Mutation „`v3Status()` aus
   `processBlock` rufen" bricht sie — aber allein über **Allokationen**, weil
   `:1145` und `:1172` `toStdString()` rufen, und heute allein über B7s
   **eigenen** Zähler (`TransactionTestMain.cpp:72-91`, geprüft bei `:3374`
   `imCallback == 0`). Über `RtWache::allokationen()` fällt sie **nicht**,
   solange B7s `operator new` nicht wie B6s (`DspGoldenTestMain.cpp:100`,
   `:109`) `RtWache::meldeAllokation()` ruft; das verlangt kein Satz des
   Bauplans. Über `RtWache::sperren()` fällt sie nie.
2. *„der Callbackpfad der Schleuse arbeitet nur mit Atomics."* Diese Hälfte
   bricht die Mutation nicht; sie bräuchte einen Haken in `betreten()`
   (`Schleuse.h:121-139`, heute reine Atomics), den E-312-10 (`:2074-2075`)
   ausschließt. Das ist H-A.

**Trägt die Allokationshälfte allein den Rotbeweis an der Zusagezeile?**
Ja — für Hälfte 1 und nur für sie, und nur unter zwei Bedingungen, die der
Bauplan noch nicht schreibt: (a) der `RtWache::Bereich` muss wie in §7.2
(`:1065-1067`) am Eintritt von `SondeProcessor::processBlock` beginnen, sonst
liegt `v3Status()` gar nicht im gemessenen Bereich; (b) für die wörtliche Zusage
`RtWache::allokationen()` = 0 muss B7s `operator new` an
`RtWache::meldeAllokation()` gebunden werden. Ohne (b) misst B7 eine andere
Größe als die Zusage nennt — dieselbe Frage betrifft M-01, M-02, M-26 und M-53.

## 2.3 Zu D-C: die tatsächliche Folge der Mutation

Mutation: `eq_enabled` (`NakamaParameter.cpp:79`) in die Abdeckungstabelle des
Blockrands aufnehmen. Folge nach dem Bauplan §7.3 Punkte 1 bis 5
(`:1299-1318`), Schritt für Schritt:

1. **Wer liest.** Der Blockrand liest je Block `hostEreignis[eq_enabled]` (ein
   relaxed Load) und vergleicht ihn mit `blockrandStand[eq_enabled]` (Punkt 1).
2. **Wer schreibt welchen Stand.** Bei Ungleichheit schreibt der Blockrand
   ausschließlich `blockrandStand[i] = n` — ein `std::uint32_t`, das dem
   Audiothread allein gehört und nicht atomar ist (Punkt 1 und Kosten
   `:1323-1324`; M-82 bei `:774` sagt dasselbe). Er fasst weder
   `hostEreignisGesehen` noch `hostEreignisOffen` an (Punkt 5).
3. **Was er mit dem Wert tut.** Nichts: es gibt für `eq_enabled` kein
   Rampenziel. `DspKern.cpp:620-623` setzt genau vier Ziele (`input`, `output`,
   `mix`, `width`), `:626` `autoGain`; `eq_enabled` wirkt allein über
   `aus.eqEngagiert` im Programm (`DspProgramm.cpp:263`). Punkt 4 („Rampenziel
   nur, wenn `publiziert[i] == blockrandStand[i]`") läuft damit ins Leere.
4. **Was der Worker danach sieht.** Unverändert alles: `parameterValueChanged`
   erhöht `hostEreignis[i]` (`SondeProcessor.cpp:1336`) und setzt
   `hostEreignisOffen` (`:1337`); der Kontrolltakt tauscht `hostEreignisOffen`
   (`:1455`), vergleicht `n` gegen `hostEreignisGesehen[i]` (`:1460`),
   quittiert (`:1461`) und schreibt ins Overlay (`:1462`).
5. **Wodurch `eq_enabled` weiter wirkt.** Über den Worker ins Programm und dort
   über `aus.eqEngagiert` (`DspProgramm.cpp:263`). Der Schalter wirkt also
   weiter; der Satz „der Wert verschwindet für den Worker → der Schalter wirkt
   gar nicht mehr" ist an der Quelle falsch.

**An welcher Zeile der Test dann fällt.** An der Mengenprüfung — und die tragen
**beide** Zeilen:

- M-79 (`:771`) als Kernzusage: „**Die Abdeckung ist genau vier Parameter groß
  und namentlich festgelegt**"; ihr eigener Rotbeweis nennt dieselbe Mutation in
  der Gegenrichtung („umgekehrt `v1.global.mono_bass_hz` aufnehmen → die
  Mengenprüfung fällt → rot").
- M-81 (`:773`) als erste Zusagehälfte: „**er steht nicht in der
  Abdeckungstabelle (M-79)**".

Nicht fällt die **verhaltensbezogene** Hälfte von M-81 („unter der Taktsperre
liegt der Umschaltblock von A hinter dem von B"): ohne Rampenziel ändert die
Tabellenmitgliedschaft am Laufverhalten nichts, A bleibt hinter B. Darin
unterscheidet sich M-81 von M-80 (`:772`), deren Rotbeweis dieselbe Mutation
gerade über die Verhaltenshälfte fallen lässt („die erste Hälfte (ohne Takt
abweichend) fällt") — bei einem Bandwert trägt der Blockrandweg, bei einem
booleschen nicht.

**Welche Mutation die Zusage von M-81 wirklich an M-81 bricht.** Eine, die
`eq_enabled` tatsächlich blockgebunden macht: dem Blockrand einen Schreibweg auf
`aus.eqEngagiert` (`DspProgramm.cpp:263`) geben, statt ihn nur in die Tabelle
einzutragen — also Tabelleneintrag **plus** Ziel. Dann wird A unter der
Taktsperre bitgleich zu B, die Hälfte „mindestens ein abweichendes Sample"
fällt, und mit ihr der Schlusssatz „Keine Gleichheitszusage für die elf
booleschen Hostparameter" samt der Grenze nach NAK-340. Das ist die Mutation,
die an der Zusagezeile selbst angreift und die Grenze von E-312-5 prüft.

---

# Übersicht

| Befund | Urteil | Einordnung | schließende Regel in einem Satz |
|---|---|---|---|
| D-A | PRÄZISIERT | DEFEKT | Jede Nennung einer fremden Matrixzeile trägt ihr Präfix, und ein bloßes `M-nn` bezeichnet ausnahmslos eine eigene Zeile M-01 bis M-86 |
| D-B | PRÄZISIERT | DEFEKT | Eine Zeile nennt als Zähler nur, was ihr Bein wirklich liest, und behauptet eine Sperre nur dort, wo sie über `RtWache::GemeldeteSperre` genommen wird |
| D-C | PRÄZISIERT | DEFEKT | Die Mutation einer Zeile nennt nur die Folge, die der Bauplan derselben Runde für sie hergibt, und eine Grenzzeile ohne eigenen Wirkpfad nennt als Bruch die Prüfung, die sie selbst trägt |
| L-A | BESTÄTIGT | LÜCKE | Ein Änderungssatz berichtigt jeden Kommentar, den er selbst falsch macht, im selben Änderungssatz — nicht nur die drei aus §1 |
| H-A | BESTÄTIGT | HÄRTUNG | Eine Zeile mit zwei Zusagehälften sagt je Hälfte, ob ein Rotbeweis sie trägt oder ob sie bewusst ohne eigene Mutation geführt wird |

Präzisierungen, die der Dirigent für die Nacharbeit 2 kennen muss:

1. **D-A ist enger und zugleich breiter als berichtet.** Enger: von den acht
   „mitgerissenen" Stellen liegt keine im Diff; im Diff liegen `:706`,
   `:1195-1196` und die beiden neuen Zeilen `:773` und `:851`. Breiter: neun vom
   Prüfer nicht genannte Stellen tragen denselben Fehler (`:677`, `:678`,
   `:680`, `:767`, `:810`, `:817`, `:821`, `:836`, `:853`), davon vier im Diff;
   ihre Kollisionen über M-47, M-55 und M-36 bestanden allerdings schon vor der
   Runde. Wirksam falsch liest ein Bauer an vier Stellen (`:1195-1196`, `:765`,
   `:766`, `:701`), an fünf weiteren ist es Bezeichnerhygiene.
2. **D-B hat eine zweite Hälfte, die der Bericht nicht nennt.** Nicht nur der
   Sperrenzähler fehlt — auch `RtWache::allokationen()` kann in B7 heute nicht
   steigen, weil dessen `operator new` `RtWache::meldeAllokation()` nicht ruft.
   Betroffen sind außer M-38 auch M-01, M-02, M-26 und M-53.
3. **D-C trifft die Begründung, nicht den Rotbeweis.** Die Zeile wird durch die
   genannte Mutation rot, und zwar an ihrer eigenen ersten Zusagehälfte; falsch
   ist allein die geschriebene Kette, die aus M-28 (`:769`) stammt.

---

# Nicht geprüft

- Alles außerhalb der fünf Befunde: die elf als geschlossen gemeldeten Punkte
  D1 bis D9, L1, L2, H1 und die Randbeobachtungen habe ich nicht nachgefahren;
  der Auftrag nennt sie nicht.
- Alles außerhalb des Diffs der Runde als eigener Prüfgegenstand; unveränderte
  Matrixzeilen und Bauplanabschnitte sind nur dort gelesen, wo ein Befund sie
  als gebrochenen Satz nennt.
- §0 bis §4, §9 bis §11 als solche; §11.2 und §11.3 sind als Maßstab gelesen,
  nicht beurteilt. Die Entscheide E-312-5 bis E-312-10 sind entschieden.
- §12 als Liste, §13 als Dirigentenarbeit.
- Zählungen des Manifests (§6.8) und die Speicherrechnung in §7.3 habe ich nicht
  nachgerechnet; der Prüfer hat sie geprüft und sie sind ausgeschlossen.
- Nicht gefahren: kein Bau, kein Test, kein Kanon, kein Broker, kein FL Studio,
  kein Python, kein Skript. Ob eine Zeile nach dem Bau wirklich rot wird, ist
  nicht gemessen, sondern am Code am ZIEL gerechnet.
- Nicht angefasst: `briefing-hub/` und `nimbalyst-local/`.

FERTIG Validierung Matrixprüfung 2, 2 bestätigt, 3 präzisiert, 0 widerlegt
