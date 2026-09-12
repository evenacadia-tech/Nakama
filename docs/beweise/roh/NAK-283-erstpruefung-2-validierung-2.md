# NAK-283 Erstprüfung 2 — Validierung von Befund 2 (lesend)

| Feld | Wert |
|---|---|
| Ticket | NAK-283, Etappe 2 (Plugin-Publikation und State, F01 + F11) |
| Befund | Erstprüfung 2 (Codex Astra max, Thread `01a0965b`), Befund 2 `[P2]` „Synchronisiere den Aufbau vor dem Scharfschalten" |
| Geprüfter Stand | `988dc44d` (Etappenstand), Basis `3cfa4176`, HEAD `92386c37` |
| Art der Prüfung | rein lesend — kein Bau, kein Lauf, kein Werkzeug; Git nur `git show` / `git grep` |
| Urteil | **DEFEKT** (siehe §4) |

---

## 1. Quellzitate je Teilbehauptung

Der Befund zerfällt in sieben Teilbehauptungen. Alle Zitate stammen aus
`988dc44d`.

### T1 — „Der Mitgliederstand bestätigt nur die Mutation, nicht den abgeschlossenen Publikations-/Dirty-Nachlauf"

Die Wartebedingung der drei Fälle liest **den Prozessorstate**, nicht das
Modell und nicht die Zähler.

`eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:774-777`:

```cpp
std::vector<nakama::state::MainProjectMitglied> mitglieder (const eqcop::EqCopilotProcessor& p)
{
    return p.holeZustandKopie().mainProjectMitglieder;
}
```

`eq-copilot/plugin/src/prozessor/State.cpp:254-258`:

```cpp
nakama::state::Zustand EqCopilotProcessor::holeZustandKopie() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand;
}
```

M-01, `Sonde012ProjectReloadTest.cpp:1696-1703`:

```cpp
    s.bestaetige (Art::confirmJoin, b);
    pruefe (warteAuf (5000, [&] { return genau (mitglieder (*s.p), { b }); })
                && modellTraegt (*s.p, { b }),
            "M-01 Aufbau: B ist Mitglied in State UND Modell", modellBestand (*s.p));

    const auto dirtyVor = dirty.nonParam.load();
    const auto revisionVor = s.p->v3StateRevisionFuerTest();
    const auto ueberholtVor = s.p->sourcesPublikationUeberholtFuerTest();
```

M-02, `:1774-1778`:

```cpp
    s.bestaetige (Art::confirmJoin, a);
    pruefe (warteAuf (5000, [&] { return genau (mitglieder (*s.p), { a }); })
                && modellTraegt (*s.p, { a }),
            "M-02 Aufbau: A ist Mitglied", modellBestand (*s.p));
    const auto ueberholtVor = s.p->sourcesPublikationUeberholtFuerTest();
```

M-72 Phase B, `:2104-2111` — **hier fehlt der Modellvergleich ganz**:

```cpp
        s.bestaetige (Art::confirmJoin, b);
        pruefe (warteAuf (5000, [&] { return genau (mitglieder (*s.p), { b }); }),
                "M-72 Phase B Aufbau: B ist Mitglied");
        const auto dirtyVor = dirty.nonParam.load();
        const auto revisionVor = s.p->v3StateRevisionFuerTest();

        s.scharf.store (true);
        s.bestaetige (Art::confirmJoin, a);
```

### T2 — „`Ipc.cpp`, Zeilen 1576-1587": Anwendung unter Sperre, Publikation danach

`eq-copilot/plugin/src/prozessor/Ipc.cpp:1576-1588`:

```cpp
        std::lock_guard<std::mutex> l (bindungMutex);
        if (reloadGeneration.load() != generationBeimAbholen)
        {
            sourcesBatchNachReloadVerworfen.fetch_add (1);
            return;  // kein Dirty, keine Revision: der geladene State ist die Wahrheit.
        }
        for (const auto& befehl : befehle)
            if (wendeSourcesCommandAnUnterBindung (befehl))
                ++geaendert;
    }
    for (std::size_t i = 0; i < geaendert; ++i)
        meldeSourcesMitgliederNachBefehl (generationBeimAbholen);
}
```

Die Nachführung selbst, `Ipc.cpp:1518-1536`:

```cpp
    std::vector<nakama::state::MainProjectMitglied> kopie;
    std::uint64_t folge = 0;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (reloadGeneration.load() != generationBeimAbholen)
        {
            sourcesNachfuehrungNachReloadUnterblieben.fetch_add (1);
            return;
        }
        kopie = zustand.mainProjectMitglieder;
        folge = naechsteSourcesFolgeUnterBindung();
    }
    // NAK-246 M-39: der Testhaken zwischen Kopie und Publikation, ohne
    // gehaltene Sperre. Im Produkt leer.
    if (sourcesPublikationHakenFuerTest)
        sourcesPublikationHakenFuerTest (kopie.size());
    werteSourcesPublikationAus (
        sourcesModel.setzePersistenteMitglieder (kopie, generationBeimAbholen, folge));
```

Und der Dirty-/Revisions-Nachlauf **nach** der Modellübernahme,
`Ipc.cpp:1462-1490` (Auszug `:1466-1489`):

```cpp
        case SourcesModel::Publikation::uebernommen:
            break;
        ...
        case SourcesModel::Publikation::ueberholt:
            ...
            sourcesPublikationUeberholt.fetch_add (1);
            break;
        ...
        case SourcesModel::Publikation::reloadAbgewiesen:
            ...
            sourcesNachfuehrungNachReloadUnterblieben.fetch_add (1);
            return;
    }
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
```

`eq-copilot/plugin/src/prozessor/Hostbruecke.cpp:160-163` — `meldeHostDirty`
meldet synchron im rufenden Faden:

```cpp
void EqCopilotProcessor::meldeHostDirty()
{
    updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged (true));
}
```

Die Modellübernahme liegt **innerhalb** von `setzePersistenteMitglieder`,
`eq-copilot/plugin/src/SourcesModel.cpp:361`, `:395-398`:

```cpp
    std::lock_guard<std::mutex> l (mutex);
    ...
    zuletztUebernommeneFolge = folge;
    if (neu == persistenteMitglieder)
        return Publikation::uebernommen;  // Publikation fand statt, ohne Aenderung.
    persistenteMitglieder = std::move (neu);
```

Damit ist die Reihenfolge im Quelltext belegt:
**Anwendung (unter `bindungMutex`, `Ipc.cpp:1582-1584`) → Freigabe beider
Sperren (`:1585`) → Kopie und Folgenummer (`:1521-1529`) → Haken (`:1532-1533`)
→ Publikation ins Modell (`SourcesModel.cpp:361-398`) → `meldeHostDirty()` und
`v3StateRevision` (`Ipc.cpp:1488-1489`).**

### T3 — „werden hier die Baselines zu früh gelesen"

`holeZustandKopie` nimmt `bindungMutex` (`State.cpp:256`), die Anwendung hält
ihn (`Ipc.cpp:1576`). Der Zustand wird für den Test also frühestens sichtbar,
wenn die Klammer bei `Ipc.cpp:1585` endet — und dann steht der Workerzug am
Anfang der Publikationsschleife `:1586-1587`. Die Wartebedingung von M-01
(`:1697`), M-02 (`:1775`) und M-72 Phase B (`:2105`) ist damit **genau in dem
Augenblick erfüllt, in dem Publikation, Dirty und Revision des Aufbaus noch
ausstehen.** Die Baselines `dirtyVor` / `revisionVor` werden unmittelbar danach
gelesen (`:1701-1702`, `:2107-2108`).

`warteAuf` prüft in 5-ms-Schritten und gibt beim ersten wahren Befund zurück
(`:57-67`); es gibt keine zweite Bedingung, die den Nachlauf abwartet.

### T4 — „der Haken kann B statt A festhalten"

Der Haken der Bühne, `Sonde012ProjectReloadTest.cpp:1650-1654`:

```cpp
        p->setzeSourcesPublikationHakenFuerTest ([this] (std::size_t)
        {
            if (scharf.load() && hakenRufe.fetch_add (1) == 0)
                gate.halten();
        });
```

Er hält **den ersten scharfen Ruf**, gleich aus welchem Batch. `scharf` wird
in M-01 bei `:1708`, in M-02 bei `:1782` und in M-72 Phase B bei `:2110`
gesetzt — also nach einer Wartebedingung, die den Aufbau-Ruf des Hakens
(`Ipc.cpp:1532`) noch **vor sich** hat. Der Kommentar der Bühne behauptet die
Gegenrichtung ohne Riegel, `:1633-1635`:

```
/// `scharf` wird erst nach dem Aufbau gesetzt, damit die Vorbelegung
/// durchlaeuft.
```

### T5 / T6 — Dirty-Bilanz in der korrekten Fassung und in MUT-B

Die Zusagezeilen, `Sonde012ProjectReloadTest.cpp:1749-1755` (M-01) und
`:2131-2137` (M-72 Phase B):

```cpp
    pruefe (dirty.nonParam == dirtyVor + 2, ...
    pruefe (s.p->v3StateRevisionFuerTest() == revisionVor + 2, ...
```

```cpp
        pruefe (dirty.nonParam == dirtyVor + 2
                    && s.p->v3StateRevisionFuerTest() == revisionVor + 2, ...
```

MUT-B ist in `docs/beweise/NAK-283.md:2731` und `:2745` als
„`Ipc.cpp:1477-1478`: Überholt-Ausstieg auf den Reload-Zweig" beschrieben; die
Rohdatei nennt sie wörtlich (`docs/beweise/roh/NAK-283-rot-M-01b-etappe-2.txt:3`):

```
Mutation:   Ipc: den Ueberholt-Ausstieg auf den Reload-Zweig legen (Ausstieg VOR meldeHostDirty und v3StateRevision)
```

Der Zähler `sourcesPublikationUeberholt.fetch_add (1)` (`Ipc.cpp:1477`) bleibt
in MUT-B stehen; nur `meldeHostDirty()` und `v3StateRevision` (`:1488-1489`)
entfallen für den Überholtfall. Genau das zeigen die gemessenen Läufe:

`docs/beweise/roh/NAK-283-rot-M-01b-etappe-2.txt:19-20` (mutiert):

```
  FEHLER  M-01: Dirty-Zaehler == Zahl der State-aendernden Befehle (2), nicht 1 - ...  [1]
  FEHLER  M-01: Revision-Delta == 2  [1]
```

`docs/beweise/roh/NAK-283-rot-M-72-etappe-2.txt:26` (mutiert):

```
  FEHLER  M-72 Phase B: reloadablehnung_und_ueberholung_sind_unterscheidbar - ...  [1 Dirty, Revision-Delta 1]
```

Beide Läufe zeigen die Bilanz **des glücklichen Zeitplans**: `dirtyVor` war
dort bereits nach dem Aufbau-Dirty gelesen, deshalb 1 statt 2.

### T7 — „ein einmaliger Modellvergleich außerhalb der Wartebedingung … genügt ebenfalls nicht"

In M-01 (`:1697-1698`) und M-02 (`:1775-1776`) steht `modellTraegt (…)` hinter
dem `&&` **außerhalb** des Lambdas von `warteAuf`; es wird genau einmal
ausgewertet, nachdem `warteAuf` zurückgekehrt ist. Fällt der Vergleich negativ
aus, setzt `pruefe` nur einen Fehler und der Lauf geht weiter
(`:48-55`, kein `return`, kein `abort`). Und selbst ein **wahrer**
Modellvergleich schließt das Fenster nicht: er beweist nur, dass
`SourcesModel::setzePersistenteMitglieder` zurück ist — `meldeHostDirty()` und
`v3StateRevision.fetch_add (1)` (`Ipc.cpp:1488-1489`) stehen danach.

---

## 2. Ist jede Teilbehauptung am Stand `988dc44d` wahr?

**Ja, alle sieben.** Im Einzelnen zu den gestellten Fragen:

### 2.1 Worauf wartet der Test vor Baseline und Scharfschalten?

Ausschließlich auf **den Mitgliederbestand im Prozessorstate**
(`holeZustandKopie().mainProjectMitglieder`, `:774-777`, `:1697`, `:1775`,
`:2105`). Nicht gewartet wird auf:

- den Modellbestand (M-72 Phase B prüft ihn gar nicht; M-01 und M-02 prüfen ihn
  einmalig außerhalb der Wartebedingung),
- den Dirty-Zähler,
- die Revision,
- die **Ruhe des Workerzugs**. Das dafür vorhandene, deterministische Signal
  `ruheAbwarten()` (`:1669-1673`) wird in allen drei Fällen erst **nach** der
  Hakenfreigabe benutzt (`:1730`, `:1792`, `:2120`), nie im Aufbau.

### 2.2 Gibt es ein Fenster „Join B angewandt, Publikation und Dirty noch nicht gelaufen", in dem der Test schon Baselines liest?

**Ja.** Es beginnt mit der Freigabe von `bindungMutex` und `sourcesDrainMutex`
bei `Ipc.cpp:1585` — vorher kann `holeZustandKopie` die Änderung gar nicht
sehen, weil es dieselbe Sperre nimmt (`State.cpp:256`) — und endet erst mit
`v3StateRevision.fetch_add (1)` bei `Ipc.cpp:1489`. Dazwischen liegen die
Kopie samt Folgenummer (`:1521-1529`), der **Hakenruf** (`:1532-1533`), die
Modellübernahme (`SourcesModel.cpp:361-398`) sowie `meldeHostDirty()`
(`:1488`). Jede dieser Kanten ist ein Präemptionspunkt; keine davon ist gegen
den Testfaden verriegelt.

Zwei Ausprägungen desselben Fensters:

- **Fenster klein (M-01, M-02):** der Modellvergleich ist wahr, aber
  `meldeHostDirty` / `v3StateRevision` stehen noch aus. `dirtyVor` und
  `revisionVor` sind dann um je 1 zu niedrig.
- **Fenster groß (M-72 Phase B):** dort wird das Modell gar nicht abgefragt
  (`:2105`); das Fenster reicht vom Lock-Release bis zur Revision und schließt
  den Hakenruf mit ein.

### 2.3 Kann der Haken dann die Kopie `[B]` statt `[B, A]` festhalten?

**Ja.** Erfüllt sich die Wartebedingung im Fenster aus 2.2, so hat der
Workerzug den Hakenruf des Aufbau-Batches (`Ipc.cpp:1532`) noch vor sich. Setzt
der Testfaden in diesem Augenblick `scharf.store (true)` (`:1708`, `:1782`,
`:2110`), so ist die Bedingung `scharf.load() && hakenRufe.fetch_add (1) == 0`
(`:1652`) beim **Aufbau**-Ruf wahr, und `gate.halten()` friert den Workerzug mit
der Kopie `[B]` (M-01/M-72) bzw. `[A]` mit altem Label (M-02) ein — vor dem
Ereignis, das die Zeile messen will. `warteBisErreicht (2000)` (`:1710`,
`:1784`, `:2112`) kann diesen Ruf nicht von dem erwarteten unterscheiden: die
Schranke trägt nur ein `bool erreicht` (`:365-384`), keine Kennung des Standes.

Der anschließend eingeschleuste `confirm_join (A)` (`:1709`, `:2111`) wird vom
festgehaltenen Workerzug nie abgeholt; ihn drainiert dann der `sourcesTick`
des Kontrollfadens (`:1721-1723`, `:2116-2118`) gemeinsam mit dem
`unbind_probe (B)` — `sourcesTick` ruft denselben Rahmen
(`Ipc.cpp:1111-1113`).

### 2.4 Folge für „Dirty-Zähler == 2, Revisionsdelta 2"

Bilanz ab `dirtyVor`, wenn das Fenster getroffen wird (Aufbau-Dirty fällt
**nach** der Baseline):

| Lauf | Aufbau (Join B) | angehaltene Publikation | Tick (Join A + Unbind B) | Summe | Zusage `+2` |
|---|---|---|---|---|---|
| korrekte Fassung, Fenster klein (M-01) | +1 | +1 (überholt, zählt) | +1 | **+3** | **FEHLER** (falsches Rot) |
| MUT-B, Fenster klein (M-01) | +1 | +0 (Ausstieg vor Dirty) | +1 | **+2** | **ok** (falsches Grün) |
| korrekte Fassung, Fenster groß (M-72 B) | +1 (verzögert, überholt) | — | +2 | **+3** | **FEHLER** |
| MUT-B, Fenster groß (M-72 B) | +0 (überholt, kein Dirty) | — | +2 | **+2** | **ok** (falsches Grün) |

Die übrigen Zusagen von M-72 Phase B halten in beiden MUT-B-Fällen: der
Überholt-Zähler steigt weiter auf 1 (`Ipc.cpp:1477` bleibt in MUT-B stehen,
`:2122-2123`), der Reloadzähler bleibt 0 (`:2127-2128`). **Damit wird der
gesamte Fall M-72 unter MUT-B grün** — der einzige Rotbeweis dieser Zeile
(`docs/beweise/NAK-283.md:2745`: „M-72 … wie M-01 (b)") wäre verloren. Für
M-01 gilt dasselbe: MUT-B verliert Dirty **und** Revision zugleich, und beide
Erwartungen kippen gemeinsam von `+3`-rot auf `+2`-grün.

Das Vorzeichen der Prüfung dreht sich also: **die korrekte Fassung wird rot,
die Mutante grün.** Das ist strenger als „flaky" — es ist eine mögliche
Invertierung des Rotbeweises.

### 2.5 Warum die gemessenen Rohläufe trotzdem grün/rot sind

`NAK-283-rot-M-01b-etappe-2.txt` und `-M-72-etappe-2.txt` zeigen die Bilanz des
Zeitplans, in dem der Aufbau-Nachlauf rechtzeitig fertig war (Dirty-Delta 1
mutiert / 2 zurückgespielt). Ein einzelner grüner Lauf beweist hier nichts über
die Erzwingung — genau die Unterscheidung, die §6.1 Schritt 2 verlangt.

---

## 3. Welcher Satz bricht?

**Es brechen drei zugesagte Sätze:**

1. **§6.1 Schritt 2** (`docs/beweise/NAK-283.md:800-803`), wörtlich:

   > „Bei Nebenläufigkeitszeilen (M-01 bis M-03, M-05, M-13, M-17, M-20 Hälfte
   > 2, M-21, M-25, M-26, M-72) wird zusätzlich das Interleaving über den
   > bestehenden bzw. den neuen Testhaken **erzwungen**; eine Zeile, die nur
   > „manchmal" rot wird, ist kein Rotbeweis."

   Der Haken erzwingt hier nur, **dass** irgendein erster Ruf hält, nicht
   **welcher**. Das Scharfschalten ist zeitlich, nicht ereignisbasiert an den
   Abschluss des Aufbaus gekoppelt. M-01 und M-72 können damit „manchmal" rot
   und „manchmal" grün werden — unter MUT-B in die falsche Richtung.

2. **M-01, Spalte „Zustand und Vorbedingung"** (`:597`), wörtlich:

   > „B ist bestätigtes Mitglied; der Analyseworker (`Analyse.cpp:347`, Zug
   > ≤ 50 ms) hat `confirm_join (A)` gedrain't, unter `bindungMutex` angewandt
   > (`zustand = [B, A]`), den `sourcesDrainMutex` verlassen und **seine Kopie
   > `[B, A]` gezogen**; der bestehende Publikationshaken … hält ihn zwischen
   > Kopie und Publikation".

   Die Vorbedingung verlangt, dass der Haken die Kopie `[B, A]` hält. Der Test
   stellt das nicht sicher; im Fenster hält er `[B]`. Derselbe Bruch trifft
   M-02 (`:598`: „der Worker hält seine Kopie am Haken wie in M-01") und über
   die Bühnenreferenz M-72 (`:610`: „(b) … **derselben** Generation (Bühne
   M-01)").

3. **M-01, Spalte „Zusage"** (`:597`) und **M-72, Spalte „Zusage"** (`:610`),
   wörtlich:

   > „**Dirty-Zähler und Revisionsdelta zählen beide State-ändernden Befehle**
   > (Join und Unbind), nicht nur den jüngeren"

   > „**Nur der Reloadfall unterdrückt Dirty und Revision**; der Überholtfall
   > meldet beide, weil seine Änderung angewandt bleibt"

   Gemessen wird nicht „beide Befehle", sondern „zwei Meldungen seit einem
   Zeitpunkt, der selbst im Nachlauf eines dritten Befehls liegen kann". Die
   Zusage und ihre Messung sind nicht deckungsgleich.

Zusätzlich widerspricht der Befund dem Selbstaudit §20.5
(`docs/beweise/NAK-283.md:2841`, Zeile „Stale Closures"): dort ist nur die
**Lebensdauer** des gefangenen `this` geprüft („Der Haken der Testbühne fängt
`this`; die Struktur überlebt den Prozessor"). Die **Scharfschaltordnung** des
gefangenen `scharf`-Flags — wann es gegenüber dem laufenden Drain gültig wird —
ist dort nicht geprüft; die Zeile „Lebenszyklus" (`:2832`) behandelt ebenfalls
nur Zerstörungsreihenfolge. Das Selbstaudit hat die Lücke also nicht abgedeckt,
sondern danebengeprüft.

**Nicht gebrochen** sind: die Sperrenordnung (`:2842`, Folgenummer nur unter
`bindungMutex` — im Produkt korrekt), die Zeile M-04 (Reload-Riegel bleibt
erste Entscheidung, `SourcesModel.cpp:373-377`), und der Produktpfad selbst.
**Der Befund trifft die Bühne, nicht das Produkt.**

---

## 4. Einordnung: **DEFEKT**

Der Bauplan verlangt für Nebenläufigkeitszeilen ein **erzwungenes**
Interleaving und erklärt eine Zeile, die nur manchmal rot wird, ausdrücklich
zum Nicht-Rotbeweis (§6.1 Schritt 2); hier hängt sowohl der Haltepunkt des
Hakens als auch die Gültigkeit der Dirty-/Revisions-Baseline an der Frage, ob
der Workerzug seinen Aufbau-Nachlauf zwischen `Ipc.cpp:1585` und `:1489` schon
hinter sich hat — eine reine Zeitfrage ohne Riegel. Die Folge ist nicht bloß
Flakiness, sondern eine mögliche **Invertierung**: die korrekte Fassung zählt
drei statt zwei Änderungen und wird rot, während MUT-B durch den ausgelassenen
Dirty-/Revisions-Pfad genau die erwarteten zwei liefert und grün wird — womit
der einzige Rotbeweis von M-72 und die zweite Hälfte von M-01
(`NAK-283-rot-M-01b-etappe-2.txt`) ihren Beweiswert verlieren. Das
deterministische Signal für den Abschluss des Aufbaus existiert bereits in
derselben Bühne (`ruheAbwarten()`, `:1669-1673`) und wird nur an dieser Stelle
nicht benutzt, weshalb es weder um eine Baulücke noch um eine bloße Härtung
geht, sondern um einen behebbaren Defekt der Messung.

---

## 5. Die schließende Regel

**Bedingung, die der Aufbau messbar erreicht haben muss:** Der Drain-Aufruf,
der den Aufbau-Befehl angewandt hat, muss **vollständig zurückgekehrt** sein —
also samt Kopie, Hakenruf, Modellübernahme, `meldeHostDirty()` und
`v3StateRevision` (`Ipc.cpp:1586-1587` → `:1532-1535` → `:1488-1489`). Erst
danach dürfen `dirtyVor`, `revisionVor` und `ueberholtVor` gelesen und
`scharf` gesetzt werden.

**Das deterministische Signal ist die Rückkehr des Workerzugs, nicht der
Zustand und nicht der Haken.** Gemeint ist der Eintrittszähler des
Drain-Rahmens, dessen Vertrag genau das zusagt
(`eq-copilot/plugin/src/PluginProcessor.h:844-852`):

> „wie oft ein Drain den RAHMEN `wendeBestaetigteSourcesCommandsAn` betreten
> hat - gezählt beim Eintritt, VOR `sourcesDrainMutex` … Ein Bein weist damit
> Ereignisse nach, statt sie abzuwarten: … **der nächste Eintritt des
> Workerzugs heisst, sein voriger Aufruf ist samt Nachführung zurück**."

Der Zähler wird bei `Ipc.cpp:1546` vor jedem Riegel erhöht, auch bei leerem
Batch (`:1558-1559`). Weil der Workerzug den Rahmen sequenziell aufruft
(`Analyse.cpp:346-347`), beweist ein Zählerstand größer als der beim Eintreten
der Zustandsbedingung gelesene, dass der Aufruf, der den Aufbau angewandt hat,
zurück ist — einschließlich Publikation und Dirty. Die Bühne besitzt diesen
Riegel bereits als `ruheAbwarten()` (`:1669-1673`); er ist im Aufbau zu fahren,
vor dem Lesen der Baselines und vor `scharf.store (true)`.

**Warum die drei naheliegenden Alternativen nicht genügen:**

- *Zustandsbestand* (`mitglieder (…)`): sichtbar ab `Ipc.cpp:1585`, also **vor**
  Haken, Publikation und Dirty — genau die Lücke.
- *Einmaliger Modellvergleich* (`modellTraegt`, außerhalb der
  Wartebedingung): beweist nur die Rückkehr von
  `setzePersistenteMitglieder`; `meldeHostDirty` und `v3StateRevision` stehen
  danach (`Ipc.cpp:1488-1489`). Auch als Teil der Wartebedingung bliebe dieses
  Reststück offen.
- *Der Haken selbst* (`hakenRufe`, `gate`): er ist das zu schützende Objekt,
  nicht der Riegel — solange er unscharf ist, meldet er nichts, und sobald er
  scharf ist, hat er den Aufbau-Ruf möglicherweise schon gefangen. Eine
  Schranke ohne Standkennung (`:365-393`) kann den falschen Ruf nicht
  ausweisen.

**Zusätzliche Absicherung, gleicher Gedanke, ohne Codevorschlag:** die Zeile
sollte nach dem Halt auch belegen, **welchen** Stand der Haken hält (M-01 tut
das bei `:1713-1715`, M-72 Phase B bei `:2112-2114` nicht). Ohne diesen Beleg
bleibt das gemessene Interleaving eine Annahme; mit ihm fällt ein falsch
gefangener Aufbau-Ruf sofort auf, statt sich erst in der Dirty-Bilanz zu
zeigen.

---

## 6. Scope-Beweis — gelesene Dateien

Alle Codequellen über `git show 988dc44d:<pfad>`; Manifest und Rohdateien im
Arbeitsbaum (HEAD `92386c37`, die genannten Dateien seit `988dc44d`
unverändert bzw. mit `92386c37` hinzugefügt).

| Datei | Gelesene Zeilen |
|---|---|
| `docs/beweise/NAK-283.md` | 8, 10, 105, 496-500, 593-610, 617, 676, 693-709, 770-838, 793-822, 838-936, 1770-1870, 1940-2013, 2041-2071, 2119-2207, 2634-2900 (Überschriftenraster ganz) |
| `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` | 44-100, 360-400, 770-790, 1570-1830, 2040-2170 |
| `eq-copilot/plugin/src/prozessor/Ipc.cpp` | 1100-1120, 1440-1610 |
| `eq-copilot/plugin/src/prozessor/Analyse.cpp` | 85-140, 335-355 |
| `eq-copilot/plugin/src/prozessor/State.cpp` | 254-258 |
| `eq-copilot/plugin/src/prozessor/Hostbruecke.cpp` | 150-185 |
| `eq-copilot/plugin/src/SourcesModel.cpp` | 350-400 |
| `eq-copilot/plugin/src/PluginProcessor.h` | 838-856 |
| `docs/beweise/roh/NAK-283-rot-M-01b-etappe-2.txt` | 1-60 (ganze Datei bis Grünlauf) |
| `docs/beweise/roh/NAK-283-rot-M-72-etappe-2.txt` | 1-40 |
| `CLAUDE.md` | 180-200 |

Nicht angefasst: kein Bau, kein Testlauf, kein Python, keine Änderung an Code,
Manifest oder Plandokumenten. Geschriebene Datei: **diese**
(`docs/beweise/roh/NAK-283-erstpruefung-2-validierung-2.md`).
