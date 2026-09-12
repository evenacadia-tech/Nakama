# NAK-283 Wiederprüfung 1 — Validierung des Befundes [P2] (lesend, 12.09.2026)

**Auftrag.** Lesende Validierung des einen offenen Befundes der
Codex-Wiederprüfung 1 (Thread `01a0968f`) gegen die Quelle am Stand
`faea3c1a` (Rundenstand der Nacharbeit 1; HEAD `d5205c28`, Stand vor der Runde
`f9750046`). Kein Bau, kein Lauf, kein Repro, keine Änderung an Code oder
Doku.

**Befund, wörtlich (Wiederprüfung 1).**

> [P2] Prüfe den Modellbestand erst nach dem Aufbau-Drain —
> eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:2165-2168. Wird der
> Worker beim Aufbau-Join nach Freigabe von `bindungMutex` (`Ipc.cpp:1585`),
> aber vor der Modellpublikation unterbrochen, liefert `warteAuf(...)` bereits
> `true`, während das anschließend einmalig ausgewertete `modellTraegt(...)`
> noch `false` liefert. `pruefe` zählt dadurch einen Fehler, den das
> nachfolgende `ruheAbwarten()` nicht zurücknehmen kann. Dieser neue Vergleich
> macht M-72 und ebenso den neuen M-06-Aufbau (`:1975-1977`) trotz korrektem
> Produktcode timingabhängig rot; der geforderte Grünlauf nach Rücknahme ist
> damit nicht zuverlässig (§6.1). Verschiebe die Modellprüfung hinter die
> State-Wartebedingung und den bestehenden Drainriegel.

---

## 1. Quellzitate

Alle Zitate aus dem Stand `faea3c1a` (`git show faea3c1a:<datei>`), Zeilen
gezählt in derselben Ausgabe.

### 1.1 Die beanstandeten Fundstellen

`eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:2164-2179` (M-72 Phase B,
Aufbau):

```cpp
        s.bestaetige (Art::confirmJoin, b);
        pruefe (warteAuf (5000, [&] { return genau (mitglieder (*s.p), { b }); })
                    && modellTraegt (*s.p, { b }),
                "M-72 Phase B Aufbau: B ist Mitglied in State UND Modell",
                modellBestand (*s.p));
        // ⚠️ NAK-283 Erstpruefung 2 Befund 2: erst der naechste Eintritt des
        // Drainrahmens beweist, dass der Aufbau-Drain samt Publikation, Dirty
        // und Revision zurueck ist. …
        pruefe (s.ruheAbwarten(),
                "M-72 Phase B Aufbau: der Aufbau-Drain ist samt Nachfuehrung zurueck - "
                "erst danach gelten Baseline und Scharfschalten");
```

`…:1973-1979` (M-06, Aufbau):

```cpp
    s.bestaetige (Art::confirmJoin, a);
    s.bestaetige (Art::confirmJoin, c);
    pruefe (warteAuf (5000, [&] { return genau (mitglieder (*s.p), { a, c }); })
                && modellTraegt (*s.p, { a, c }),
            "M-06 Aufbau: A und C sind Mitglied in State UND Modell", modellBestand (*s.p));
    pruefe (s.ruheAbwarten(),
            "M-06 Aufbau: der Aufbau-Drain ist samt Nachfuehrung zurueck");
```

### 1.2 Dieselbe Form in M-01 und M-02

`…:1696-1713` (M-01):

```cpp
    s.bestaetige (Art::confirmJoin, b);
    pruefe (warteAuf (5000, [&] { return genau (mitglieder (*s.p), { b }); })
                && modellTraegt (*s.p, { b }),
            "M-01 Aufbau: B ist Mitglied in State UND Modell", modellBestand (*s.p));
    …
    pruefe (s.ruheAbwarten(),
            "M-01 Aufbau: der Aufbau-Drain ist samt Publikation, Dirty und Revision "
            "zurueck - erst danach gelten Baseline und Scharfschalten");
```

`…:1788-1797` (M-02):

```cpp
    s.bestaetige (Art::confirmJoin, a);
    pruefe (warteAuf (5000, [&] { return genau (mitglieder (*s.p), { a }); })
                && modellTraegt (*s.p, { a }),
            "M-02 Aufbau: A ist Mitglied", modellBestand (*s.p));
    …
    pruefe (s.ruheAbwarten(),
            "M-02 Aufbau: der Aufbau-Drain ist samt Nachfuehrung zurueck - erst danach "
            "wird scharf geschaltet");
```

### 1.3 Die Hilfen der Testbühne

`…:48-55` (`pruefe` — zählt sofort, kennt keine Rücknahme):

```cpp
void pruefe (bool ok, const char* name, const juce::String& detail = {})
{
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << name;
    if (detail.isNotEmpty())
        std::cout << "  [" << detail.toRawUTF8() << "]";
    std::cout << '\n';
    ok ? ++bestanden : ++fehler;
}
```

`…:57-67` (`warteAuf` — Abtastraster 5 ms, kehrt beim ERSTEN wahren Befund
zurück):

```cpp
template <typename Bedingung>
bool warteAuf (int millisekunden, Bedingung&& bedingung)
{
    for (int i = 0; i < millisekunden / 5; ++i)
    {
        if (bedingung())
            return true;
        std::this_thread::sleep_for (std::chrono::milliseconds (5));
    }
    return bedingung();
}
```

`…:774-777` (`mitglieder` — liest den PROZESSORSTATE):

```cpp
std::vector<nakama::state::MainProjectMitglied> mitglieder (const eqcop::EqCopilotProcessor& p)
{
    return p.holeZustandKopie().mainProjectMitglieder;
}
```

`…:1621-1630` (`modellTraegt` — liest das MODELL, ohne jede Wiederholung):

```cpp
bool modellTraegt (const eqcop::EqCopilotProcessor& p, std::initializer_list<std::string> ids)
{
    const auto bestand = p.sourcesPersistenteMitgliederFuerTest();
    if (bestand.size() != ids.size())
        return false;
    …
}
```

`…:1669-1673` (`ruheAbwarten` — der Riegel über den Eintrittszähler):

```cpp
    bool ruheAbwarten()
    {
        const auto vorher = p->sourcesDrainEintritteFuerTest();
        return warteAuf (5000, [&] { return p->sourcesDrainEintritteFuerTest() > vorher; });
    }
```

`…:2759-2761` (der Exitcode des Beins hängt am Fehlerzähler):

```cpp
    std::cout << "SONDE-012 ProjectReload: " << bestanden << "/"
              << (bestanden + fehler) << " gruen\n";
    return fehler == 0 ? 0 : 1;
```

### 1.4 Der Produktpfad

`eq-copilot/plugin/src/prozessor/Ipc.cpp:1542-1547` (Eintrittszähler VOR jedem
Riegel):

```cpp
#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    // NAK-246 Etappe 4 Nacharbeit 1 (M-12, R-E4-1): der Eintritt in den
    // Rahmen, gezaehlt VOR dem Riegel …
    sourcesDrainEintritteZaehlerFuerTest.fetch_add (1);
#endif
```

`…:1576-1587` (Anwendung unter `bindungMutex`; die Klammer endet auf `:1585`,
die Nachführung folgt DANACH):

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
```

`…:1518-1535` (die Nachführung: eigene, SPÄTERE `bindungMutex`-Klammer, dann
Haken, dann erst die Modellpublikation):

```cpp
    std::vector<nakama::state::MainProjectMitglied> kopie;
    std::uint64_t folge = 0;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        …
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

`eq-copilot/plugin/src/prozessor/State.cpp:254-258` (der State-Leser nimmt
`bindungMutex`):

```cpp
nakama::state::Zustand EqCopilotProcessor::holeZustandKopie() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand;
}
```

`eq-copilot/plugin/src/SourcesModel.cpp:349-361` und `:450-454` (das Modell
steht unter einer ANDEREN Sperre; `persistenteMitglieder` wird erst dort
ersetzt):

```cpp
SourcesModel::Publikation SourcesModel::setzePersistenteMitglieder (…)
{
    std::map<std::string, juce::String> neu;
    for (const auto& m : mitglieder)
        …
    std::lock_guard<std::mutex> l (mutex);
```

```cpp
std::map<std::string, juce::String> SourcesModel::persistenteMitgliederKopie() const
{
    std::lock_guard<std::mutex> l (mutex);
    return persistenteMitglieder;
}
```

`eq-copilot/plugin/src/prozessor/Analyse.cpp:346-347` (der Workerzug ruft den
Rahmen sequenziell aus einem Faden):

```cpp
        if (! workerDrainAusFuerTest.load (std::memory_order_relaxed))
            wendeBestaetigteSourcesCommandsAn();
```

`eq-copilot/plugin/src/PluginProcessor.h:844-852` (die Zusage des Zählers, auf
der `ruheAbwarten` ruht):

> „wie oft ein Drain den RAHMEN `wendeBestaetigteSourcesCommandsAn` betreten
> hat — gezaehlt beim Eintritt, VOR `sourcesDrainMutex` … der naechste Eintritt
> des Workerzugs heisst, sein voriger Aufruf ist samt Nachfuehrung zurueck."

### 1.5 Die Vorgabe, gegen die geprüft wird

`docs/beweise/NAK-283.md:812-815` (§6.1 Punkt 5):

> 5. **Bytegleiche Rücknahme**: Datei zurückspielen, SHA-256 gegen Schritt 1
>    prüfen (**gleich**), Zeitstempel erneut setzen, Bein erneut fahren.
>    Erwartet: **grün**. Beide Hashes und beide Exitcodes stehen im
>    Etappenabschnitt.

`docs/beweise/NAK-283.md:799-803` (§6.1 Punkt 2, Schlusssatz):

> eine Zeile, die nur „manchmal" rot wird, ist kein Rotbeweis.

`docs/beweise/NAK-283.md:2962` (§22, Einordnung des Befundes 2 der
Erstprüfung 2):

> ein Test, der auf Timing hofft, ist laut Prüfauftrag ein Defekt.

`docs/beweise/NAK-283.md:3053` (§23.2, Selbstbericht des Bauers — er nennt die
beanstandete Ergänzung ausdrücklich):

> \| M-72 Phase B \| `:2177` \| dieselbe Wartebedingung; Aufbau prüft jetzt auch
> das Modell (`:2165`); Standbeleg `:2187` (State `[B, A]`, Modell noch `[B]`) \|

---

## 2. Sind die Teilbehauptungen am Stand `faea3c1a` wahr?

### 2.1 Steht in M-72 (`:2165-2168`) und M-06 (`:1975-1977`) ein einmalig ausgewertetes `pruefe(… modellTraegt(…))` zwischen `warteAuf(State-Bedingung)` und `ruheAbwarten()`?

**Ja, wörtlich.** Beide Fundstellen bilden `warteAuf (5000, …genau (mitglieder
(…))…) && modellTraegt (…)`. Der `&&`-Operator wertet kurzschließend aus:
sobald `warteAuf` `true` liefert, wird `modellTraegt` **genau einmal** gerufen —
ohne Wiederholung, ohne Frist. Die `ruheAbwarten()`-Zeile steht in beiden
Fällen erst danach (`:2177`, `:1978`).

**Beide Fundstellen sind vom Fixdiff eingeführt.** `git diff
f9750046..faea3c1a -- eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp`
zeigt für M-72 den Ersatz der alten, reinen State-Zeile

```
-        pruefe (warteAuf (5000, [&] { return genau (mitglieder (*s.p), { b }); }),
-                "M-72 Phase B Aufbau: B ist Mitglied");
+        pruefe (warteAuf (5000, [&] { return genau (mitglieder (*s.p), { b }); })
+                    && modellTraegt (*s.p, { b }),
+                "M-72 Phase B Aufbau: B ist Mitglied in State UND Modell",
+                modellBestand (*s.p));
```

und für M-06 den ganzen Aufbaublock `:1969-1979` als Neuzugang.

### 2.2 Existiert das Fenster am Produktpfad?

**Ja.** Der Weg ist lückenlos belegt:

1. `wendeBestaetigteSourcesCommandsAn` wendet den Batch unter `bindungMutex`
   an; die Klammer endet auf `Ipc.cpp:1585`. Ab diesem Augenblick liefert
   `holeZustandKopie()` (`State.cpp:254-258`, dieselbe Sperre) den neuen
   Mitgliederbestand — `genau (mitglieder (…), …)` wird wahr.
2. Erst **danach** läuft die Schleife `:1586-1587` und ruft je geänderten
   Befehl `meldeSourcesMitgliederNachBefehl`. Dort wird `bindungMutex` **neu**
   genommen (`:1521`), die Kopie und die Folgenummer gezogen (`:1527-1528`),
   die Sperre wieder freigegeben (`:1529`), der Publikationshaken gerufen
   (`:1532-1533`) und erst dann `sourcesModel.setzePersistenteMitglieder`
   (`:1535`).
3. `setzePersistenteMitglieder` baut die Karte `neu` noch **außerhalb** der
   Modellsperre und nimmt `SourcesModel::mutex` erst auf `SourcesModel.cpp:361`;
   `persistenteMitgliederKopie()` (`:450-454`) liest unter derselben Sperre.

State und Modell stehen also unter **zwei verschiedenen Sperren**. Zwischen
`Ipc.cpp:1585` und `SourcesModel.cpp:361` ist der neue State sichtbar, während
das Modell noch den alten Bestand trägt. Ein Leser, der keine der beiden
Sperren hält — genau die Testbühne —, kann diesen Zwischenstand sehen.

Der Haken (`:1532-1533`) hält in dieser Phase **nicht**: `scharf` ist im Aufbau
noch `false` (`Sonde012ProjectReloadTest.cpp:1650-1654`), der Haken kehrt sofort
zurück. Das Fenster ist damit schmal — zwei Sperrübernahmen, ein
`std::function`-Aufruf, der Aufbau einer `std::map` —, aber es ist ein echtes
Fenster mit zwei Vorzugs-Unterbrechungspunkten (den beiden Sperren) und liegt
exakt an dem Augenblick, an dem die Wartebedingung wahr wird.

### 2.3 Kann `pruefe` dort einen Fehler zählen, den `ruheAbwarten` nicht zurücknimmt?

**Ja.** `pruefe` (`:48-55`) druckt sofort `FEHLER` und erhöht `fehler`; es gibt
keinen Weg, einen gezählten Fehler zurückzunehmen. `main` liefert
`return fehler == 0 ? 0 : 1` (`:2761`). Ein Treffer im Fenster macht das ganze
Bein rot — unabhängig davon, dass die folgende `ruheAbwarten()`-Zeile und alle
Zusagezeilen danach grün sind.

Die Richtung des Schadens ist doppelt:

- **Im Rücknahmelauf** (unmutierte Quelle) wird aus dem geforderten Grünlauf
  ein Rotlauf — §6.1 Punkt 5 bricht.
- **Im mutierten Lauf** entsteht eine zusätzliche `FEHLER`-Zeile, die nichts
  mit der Mutation zu tun hat; der Rotbeweis fiele dann (auch) an einer Zeile,
  die die Zusage nicht trägt.

Betroffen ist nicht nur das Rotskript: dieselben Fälle laufen im Bein **B14**
(`EqCopSonde012ProjectReloadTest`, Exit 0 gefordert) und im Kanon.

### 2.4 Haben M-01 (`:1711`) und M-02 (`:1795`) dasselbe Muster?

**Ja — dieselbe Form, aber nicht aus diesem Fixdiff.** M-01 prüft das Modell auf
`:1697-1699`, M-02 auf `:1789-1791`, in beiden Fällen als einmalig
ausgewerteter `&&`-Zweig hinter der State-Wartebedingung und **vor** dem
`ruheAbwarten()` auf `:1711` bzw. `:1795`. Der Fixdiff zeigt diese beiden
Zeilen als unveränderten Kontext: sie bestanden bereits am Stand `f9750046`.
Die Nacharbeit 1 hat dort nur die `ruheAbwarten()`-Zeile **nach** ihnen
ergänzt — und damit die Modellprüfung ungewollt im offenen Fenster stehen
lassen. Das Fehlerbild ist identisch.

Nebenbefund ohne eigene Schwere: die Beschriftung von M-02 (`:1791`, „M-02
Aufbau: A ist Mitglied") nennt die Modellhälfte nicht, obwohl die Bedingung sie
prüft.

### 2.5 Gegenprobe — die korrekte Form steht im selben Bein

M-71 zeigt beide sauberen Muster:

- `:2068` nimmt `modellTraegt` **in** die Wartebedingung (`warteAuf (5000, [&]
  { return modellTraegt (*s.p, { a }); })`) — es wird abgetastet, nicht
  gehofft.
- `:2079` setzt `ruheAbwarten()`, und erst `:2090` prüft `modellTraegt (*s.p,
  { a })` einmalig — **hinter** dem Drainriegel.

Damit ist die schließende Regel in derselben Datei bereits belegt und braucht
keine neue Bauweise.

### 2.6 Was die Rohbelege sagen — und was nicht

`docs/beweise/roh/NAK-283-rot-M-06-etappe-2-nacharbeit-1.txt`: Rotlauf Exit 1
(9/11), Rücknahmelauf Exit 0 (11/11), SHA-256 vorher = nachher. Die
Aufbau-Zeile ist in beiden Läufen `ok` (Zeilen 14 und 29).

`docs/beweise/roh/NAK-283-rot-M-72-etappe-2-nacharbeit-1.txt`: drei Rotläufe
Exit 1 (16/17), Rücknahmelauf Exit 0 (17/17), SHA-256 vorher = nachher. Die
Aufbau-Zeile ist in allen vier Läufen `ok` (Zeilen 22, 43, 64, 85).

**Die Grünläufe nach bytegleicher Rücknahme sind also grün.** Dazu kommen
weitere `ok`-Belege derselben Zeilenform in
`NAK-283-rot-M-01a-…` (2 Läufe), `NAK-283-rot-M-01b-…` (4 Läufe),
`NAK-283-rot-M-02-…` (2 Läufe) und `NAK-283-etappe-2-nacharbeit-1-beine.txt`
(je 1 Lauf). In keinem der aufgezeichneten Läufe hat das Fenster getroffen.

Das ist **kein** Gegenbeweis: ein Fenster von Mikrosekunden gegen ein
5-ms-Abtastraster trifft selten, und eine Verdrängung des Workerfadens
zwischen `Ipc.cpp:1585` und `SourcesModel.cpp:361` weitet es auf ein
Scheduler-Quantum. Rund zehn grüne Läufe sagen nichts über den Rand einer
Zeile, die in jedem künftigen Rotbeweis, in jedem B14-Lauf und in jedem Kanon
erneut gewürfelt wird. Die Zusage von §6.1 Punkt 5 lautet „Erwartet: **grün**",
nicht „meistens grün".

**Ergebnis: jede Teilbehauptung des Befundes ist am Stand `faea3c1a` wahr.**

---

## 3. Welcher Satz bricht?

**§6.1 Punkt 5** (`docs/beweise/NAK-283.md:812-815`), wörtlich:

> **Bytegleiche Rücknahme**: Datei zurückspielen, SHA-256 gegen Schritt 1
> prüfen (**gleich**), Zeitstempel erneut setzen, Bein erneut fahren.
> Erwartet: **grün**.

Die Erwartung „grün" ist eine deterministische Zusage über den unmutierten
Stand. Eine Zusagezeile, deren Ausgang bei unverändertem, korrektem
Produktcode vom Interleaving zweier Threads abhängt, erfüllt sie nicht.

Mitgebrochen, in zweiter Linie:

- **§6.1 Punkt 2**, Schlusssatz (`:803`): „eine Zeile, die nur „manchmal" rot
  wird, ist kein Rotbeweis." Der Satz zielt auf die Rotseite; die
  beanstandete Zeile kann beim Rotlauf eine mutationsfremde `FEHLER`-Zeile
  beisteuern und damit den Rotbeweis an einer Zeile mitfällen lassen, die die
  Zusage nicht trägt.
- **Die Dirigentenregel aus §22** (`:2962`): „ein Test, der auf Timing hofft,
  ist laut Prüfauftrag ein Defekt." Genau diese Regel hat die Nacharbeit 1
  schließen sollen (Defekt 2, §23.2); die neue Zeile öffnet sie an derselben
  Bühne wieder, nur eine Prüfzeile früher.

**Nicht gebrochen:** kein Vertrag, keine Invariante, keine Matrixzeile aus §5.1
und **kein Produktcode**. F01 bleibt im Produkt erfüllt; Folgenummer,
Sperrenordnung und dreiwertige Auswertung sind nicht berührt. Der Produktdiff
der Runde ist leer (§24) und bleibt es auch nach der Schließung dieses
Befundes. Auch die Zusage von §23.2 selbst — „Vor dem Lesen der Baselines und
vor `scharf.store (true)` läuft `ruheAbwarten()`" — steht weiter; sie gilt für
Baselines und Scharfschalten, nur eben nicht für den zusätzlich eingezogenen
Modellvergleich.

---

## 4. Einordnung: **DEFEKT**

Der Vergleich `modellTraegt(...)` steht an beiden neuen Fundstellen
(`:2166`, `:1976`) in einem Fenster, das der Produktpfad zwischen
`Ipc.cpp:1585` und `SourcesModel.cpp:361` nachweislich offen hält, und wird dort
genau einmal ausgewertet — der Ausgang hängt damit bei korrektem Produktcode am
Interleaving, und `pruefe` kann den daraus entstehenden Fehler nicht mehr
zurücknehmen. Weil der Fixdiff diese beiden Vergleiche neu einzieht, ist es ein
vom Fixdiff **eingeführter** Bruch, kein mitgeschleppter: es ist ein Test, der
für den geforderten Grünlauf nach bytegleicher Rücknahme auf Timing hofft, und
das ist nach der Dirigentenregel aus §22 ein Defekt, keine Lücke. Die
gleichartigen, aber schon am Stand `f9750046` vorhandenen Vergleiche in M-01
(`:1698`) und M-02 (`:1790`) sind derselbe Defektfall und gehören unter dieselbe
Regel, auch wenn sie diese Runde nicht eingeführt hat.

---

## 5. Die schließende Regel

**Regel.** Der Modellbestand des Aufbaus wird an keiner der vier Bühnen mehr
zusammen mit der State-Wartebedingung ausgewertet. Er wird aus der
`&&`-Verbindung herausgelöst und als **eigene Prüfzeile hinter den bestehenden
Drainriegel** `s.ruheAbwarten()` gestellt — also hinter den Eintrittszähler des
Drainrahmens (`PluginProcessor.h:844-852`), der als einziger beweist, dass der
Aufruf, der den Aufbau angewandt hat, samt Publikation, Dirty und Revision
zurück ist. Die Reihenfolge je Bühne lautet danach: State-Wartebedingung →
`ruheAbwarten()` → Modellvergleich → Baselines lesen → `scharf.store (true)`.
Wo die Modellhälfte stattdessen in die Wartebedingung selbst gezogen wird
(Muster `:2068`), ersetzt das den Drainriegel **nicht**; er bleibt, weil nur er
Dirty und Revision einschließt.

**Fundstellen (Stand `faea3c1a`):**

| Zeile | Vergleich, der wandert | Ziel: hinter diese Zeile | Herkunft |
|---|---|---|---|
| M-72 Phase B | `:2166` (`&& modellTraegt (*s.p, { b })`) | `:2177-2179` (`ruheAbwarten()`) | neu aus dem Fixdiff |
| M-06 | `:1976` (`&& modellTraegt (*s.p, { a, c })`) | `:1978-1979` (`ruheAbwarten()`) | neu aus dem Fixdiff |
| M-01 | `:1698` (`&& modellTraegt (*s.p, { b })`) | `:1711-1713` (`ruheAbwarten()`) | bestand schon am Stand `f9750046` |
| M-02 | `:1790` (`&& modellTraegt (*s.p, { a })`) | `:1795-1797` (`ruheAbwarten()`) | bestand schon am Stand `f9750046` |

**Was dabei nicht verschoben werden darf:**

- **Die Standbelege** — sie belegen, welchen Stand der Haken festhält, und sind
  deterministisch, weil der Workerzug am Haken steht und das Modell währenddessen
  nicht vorrücken kann: M-01 `:1727`, M-02 `:1808`, M-06 `:1988`, M-72
  `:2187`. Sie bleiben, wo sie stehen — unmittelbar nach
  `s.gate.warteBisErreicht (2000)` und vor dem zweiten Ereignis.
- **Die Baselines** — `dirtyVor` / `revisionVor` / `ueberholtVor` (M-01
  `:1715-1717`, M-02 `:1798`, M-72 `:2180-2181`) bleiben **nach**
  `ruheAbwarten()` und **vor** `s.scharf.store (true)`; genau das ist die
  Schließung von Defekt 2 aus §23.2 und der Grund, warum die Rotläufe `[1]`
  statt `[2]` messen.
- **`s.scharf.store (true)`** bleibt der letzte Schritt des Aufbaus (M-01
  `:1722`, M-02 `:1802`, M-06 `:1984`, M-72 `:2183`).
- **M-71** (`:2068`, `:2079`, `:2090`) ist bereits regelkonform und bleibt
  unverändert; ebenso M-03 und M-05, die nach §23.2 keinen Aufbau-Drain haben.
- **Kein Produktcode.** Die Regel betrifft ausschließlich
  `Sonde012ProjectReloadTest.cpp`; der Produktdiff der Runde bleibt leer, und
  die Quell-Hashes der Rotbeweise bleiben bytegleich mit §20.2.

**Nachweis der Schließung.** Die betroffenen Rotbeweise (M-01a/M-01b, M-02,
M-06, M-72) werden nach §6.1 neu gefahren; der Rücknahmelauf muss grün sein und
die Rotläufe müssen weiterhin an derselben Zusagezeile mit demselben Wert
fallen (M-01b/M-72: `[1]` an Dirty und Revision; M-06: am Bestandsvergleich).
Bein **B14** läuft einzeln, Exit 0.

---

## 6. Scope-Beweis — gelesene Dateien

Nur gelesen (`git show`, `git diff`, `git grep`, `git log`, `git check-attr`,
`git ls-files --eol`). Kein Bau, kein Test, kein Python, kein Repro, keine
Injektion, keine Änderung außer dieser Datei.

| Datei | Stand | Zeilenbereiche |
|---|---|---|
| `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` | `faea3c1a` | 20-80, 770-790, 1600-1700, 1700-1760, 1770-1830, 1950-2035, 2055-2095, 2140-2200, 2690-2762; dazu Trefferliste über die ganze Datei zu `warteAuf` / `modellTraegt` / `ruheAbwarten` / `pruefe` |
| `eq-copilot/plugin/src/prozessor/Ipc.cpp` | `faea3c1a` | 1100-1120, 1470-1620 |
| `eq-copilot/plugin/src/prozessor/State.cpp` | `faea3c1a` | 250-268 |
| `eq-copilot/plugin/src/prozessor/Analyse.cpp` | `faea3c1a` | 335-355 |
| `eq-copilot/plugin/src/PluginProcessor.h` | `faea3c1a` | 836-905; Trefferzeilen 154, 1165, 1852 |
| `eq-copilot/plugin/src/SourcesModel.cpp` | `faea3c1a` | 340-395, 445-462 |
| `eq-copilot/plugin/src/SourcesModel.h` | `faea3c1a` | Trefferzeile 292 |
| `docs/beweise/NAK-283.md` | `faea3c1a` / `d5205c28` | 593-612, 780-830, 2951-2980, 3029-3080, 3240-3252, 3254-3266; Überschriftenliste der ganzen Datei |
| `docs/beweise/roh/NAK-283-rot-M-06-etappe-2-nacharbeit-1.txt` | `faea3c1a` | 1-39 (ganz) |
| `docs/beweise/roh/NAK-283-rot-M-72-etappe-2-nacharbeit-1.txt` | `faea3c1a` | 1-93 (ganz) |
| `docs/beweise/roh/NAK-283-rot-M-01a-etappe-2-nacharbeit-1.txt` | `faea3c1a` | Trefferzeilen 14-15, 30-31 |
| `docs/beweise/roh/NAK-283-rot-M-01b-etappe-2-nacharbeit-1.txt` | `faea3c1a` | Trefferzeilen 14-15, 30-31, 46-47, 62-63 |
| `docs/beweise/roh/NAK-283-rot-M-02-etappe-2-nacharbeit-1.txt` | `faea3c1a` | Trefferzeilen 14-15, 27-28 |
| `docs/beweise/roh/NAK-283-etappe-2-nacharbeit-1-beine.txt` | `faea3c1a` | Trefferzeilen 42-46, 57, 71-72, 115-118, 131-177, 190-191, 209-249 |

Git-Abfragen: `git diff f9750046 faea3c1a --stat`, `git diff f9750046 faea3c1a
-- eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp`, `git check-attr -a`
und `git ls-files --eol` auf `docs/beweise/roh/`.
