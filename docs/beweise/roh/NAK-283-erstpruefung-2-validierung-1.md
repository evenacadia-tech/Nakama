# NAK-283 Erstprüfung 2 — Validierung Befund 1 (M-06-Rotbeweis)

**Stand:** Branch `master`, HEAD `92386c37`; Etappenstand `988dc44d`, Basis
`3cfa4176`. `git diff 988dc44d HEAD` berührt ausschließlich
`docs/beweise/NAK-283.md` (16+/1-); alle hier gelesenen Quell- und Testdateien
sind an beiden Ständen bytegleich, ebenso im Arbeitsbaum (`git diff HEAD --
eq-copilot/` leer). Die Validierung gilt damit unverändert für `988dc44d`.

**Befund 1 (Codex Astra, Thread `01a0965b`), wörtlich:**

> [P2] Lasse den M-06-Rotbeweis am Bestandsvergleich scheitern —
> eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:1944-1949. DEFEKT gegen
> M-06 und den geforderten Rotbeweis an der Zusage: Nach Freigabe der alten
> Kopie publizieren Benennung und lokaler Unbind erneut den aktuellen State und
> reparieren dadurch den zu messenden Fehler. Nachvollziehbar mit
> `git show 988dc44d:docs/beweise/roh/NAK-283-rot-M-06-etappe-2.txt`: Trotz
> entfernter Folgenummernprüfung bleibt der Bestandsvergleich grün;
> ausschließlich der Überholt-Zähler fällt. Die alte Kopie muss zuletzt
> freigegeben werden, ohne anschließenden heilenden Handgriff, damit der Test
> die zugesagte Modellkohärenz tatsächlich falsifiziert (vgl.
> CLAUDE.md:183-184).

---

## 1. Quellzitate je Teilbehauptung

### 1.1 Was M-06 zusagt (Spezifikation)

`docs/beweise/NAK-283.md:602` (Zeile M-06, Spalte **Zustand und
Vorbedingung**):

> Schließende Zusage. Fünf Publikationswege (`Ipc.cpp:1233`, `:1275`, `:1475`,
> `State.cpp:347`, Testpfad) und beide Threads sind in einem Lauf aktiv; nach
> dem Lauf ist Ruhe (kein weiterer Befehl, kein Tick)

Spalte **Zusage**:

> **Nach Ruhe zeigen Prozessorstate und Modell denselben Mitgliederbestand.**
> die Schlüsselmenge aus `holeZustandKopie().mainProjectMitglieder` und die
> Schlüsselmenge von `sourcesModel.persistenteMitglieder` (`std::map`,
> `SourcesModel.cpp:372`) sind gleich, und je Schlüssel ist das Label gleich

Spalte **Reihenfolge und Frist**:

> Es gibt keinen Heilungstakt: `wendeBestaetigteSourcesCommandsAn` kehrt bei
> leerem Batch zurück (`Ipc.cpp:1503-1504`), ein `session_snapshot` schreibt
> `eintraege`, nicht `persistenteMitglieder`. **Die Zusage muss ohne weiteren
> Befehl halten**

Spalte **Rotbeweis**:

> Mutation wie M-01 → State trägt `[A]`, Modell `[B, A]`. **ROT am Basis-SHA**

Spalte **Quelle**: „`CLAUDE.md` „Engine kennt keine Optik"; §4 F01".

Vergleichszeile M-01, `docs/beweise/NAK-283.md:597`, Spalte **Zusage**
(Auszug):

> **Eine ältere Publikation ersetzt keine jüngere — und ihre bereits angewandte
> Änderung bleibt gezählt.** Nach Freigabe und Ruhe trägt `sourcesModel` genau
> `[A]`; die ältere Publikation wird als **überholt** abgewiesen und in einem
> eigenen Zähler geführt.

M-01 Spalte **Reihenfolge und Frist** (Auszug): „Frist: die Zusage gilt nach
Freigabe des Hakens ohne weiteren Befehl (kein Heilungstakt)". M-01 Spalte
**Rotbeweis** (a): „An `SourcesModel.cpp:360-372`: die Folgenummernprüfung
entfernen (nur `generation != reloadGeneration` bleibt) → das Modell übernimmt
`[B, A]`."

**Der Überholt-Zähler ist die Zusage von M-01/M-71/M-72, nicht von M-06.**
M-01 (`:597`) führt ihn wörtlich („in einem eigenen Zähler geführt"), M-71
(`:609`) und M-72 (`:610`) messen ihn als eigene Zusage. In der Zusagespalte
von M-06 (`:602`) kommt er nicht vor.

### 1.2 Rotbeweis-Regel

`docs/beweise/NAK-283.md:799-803` (§6.1 Punkt 2):

> **Mutation** genau an der Zeile, die die Zusage trägt — nie an einem
> Nebeneffekt. Bei Nebenläufigkeitszeilen (M-01 bis M-03, M-05, M-13, M-17,
> M-20 Hälfte 2, M-21, M-25, M-26, M-72) wird zusätzlich das Interleaving über
> den bestehenden bzw. den neuen Testhaken **erzwungen**; eine Zeile, die nur
> „manchmal" rot wird, ist kein Rotbeweis.

`docs/beweise/NAK-283.md:8` (Kopfzeile **Gate**, Auszug):

> … gemessen durch die Matrixzeilen M-01 bis M-75 (§5) mit Rotbeweis an der
> Zusagezeile …

### 1.3 Feinheiten 18 bis 20

- **Feinheit 18** (`:1798 ff.`) betrifft R-283-1 / F02 (Rust,
  `hypothese_verdrahtung.rs`) — für M-06 ohne Belang.
- **Feinheit 19** (`:1845-1856`): „`setzePersistenteMitglieder` meldet künftig
  **übernommen**, **überholt** oder **Reload abgewiesen** statt `true`/`false`."
  Begründet die drei Ausgänge und die Dirty-/Revisionsbehandlung — das ist der
  Gegenstand von M-01 (b) und M-72, nicht von M-06.
- **Feinheit 20** (`:1858-1869`): „Die Frischemarke wird von **jeder**
  angenommenen Publikation fortgeschrieben, auch von der inhaltsgleichen" —
  Gegenstand von M-05.

Keine der drei Feinheiten verschiebt die Zusage von M-06 oder erlaubt, sie über
einen Zähler zu messen.

### 1.4 Bauabschnitt des Bauers

`docs/beweise/NAK-283.md:2736` (§20.2):

> \| M-06 \| `:1925` `state_und_modell_sind_nach_ruhe_gleich` \| B14 \| wie
> M-01 (a) \| `NAK-283-rot-M-06-etappe-2.txt` \| Exit 1 / Exit 0 \|

`docs/beweise/NAK-283.md:2919-2924` (§20.7 Punkt 2) erklärt, dass der fünfte
Publikationsweg (Testpfad) in B13 liegt und der Fall „die **vier Produktwege**
in einem Lauf" misst. Diese Abweichung ist erklärt und ist **nicht** Gegenstand
von Befund 1.

### 1.5 Der gebaute Testfall (Ablauf Schritt für Schritt)

`eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:1925-1984`. Reihenfolge
der Handgriffe, mit Zeilen:

1. `:1930` `Buehne s;` — Weg 4: `setzeBindung` in `mainAnlegen` hat bereits
   publiziert.
2. `:1931` `s.p->setzeSourcesFixtureFuerTest (lebendeQuelle (a));`
3. `:1936-1938` `s.scharf.store (true); s.bestaetige (Art::confirmJoin, a);` →
   der Workerzug hält am Publikationshaken mit der Kopie `[A]`
   (Haken: `:1650-1654`, `gate.halten()`).
4. `:1939` `s.bestaetige (Art::confirmJoin, b);`
5. `:1940-1943` `std::thread tick (… s.p->sourcesTick() …); tick.join();` — der
   Message-Thread-Drain wendet `confirm_join (B)` an und publiziert `[A, B]`
   (Weg 3, `Ipc.cpp:1492-1536`).
6. **`:1944` `s.gate.freigeben();`** — **hier** wird die alte Worker-Kopie
   `[A]` freigegeben; sie publiziert danach.
7. `:1945-1946` `pruefe (tickDurch && s.ruheAbwarten(), …)` — Ruhe als
   Ereignis (`:1669-1673`: der nächste Eintritt des Workerzugs).
8. **`:1949` `pruefe (s.p->benenneSourcesHauptziel (a, "Fluegel"), …)`** —
   Weg 1, **nach** der Freigabe.
9. `:1954-1957` Fixture auf `b` ohne gültige Runtime-Nonce.
10. **`:1958` `pruefe (s.p->entferneSourcesHauptziel (b), …)`** — Weg 2
    (lokaler Unbind), **nach** der Freigabe.
11. `:1962` `pruefe (s.ruheAbwarten(), "M-06: nach dem letzten Handgriff ist
    Ruhe");`
12. `:1964-1979` Bestandsvergleich State ↔ Modell.
13. `:1980-1983` Prüfzeile auf `sourcesPublikationUeberholtFuerTest() == 1`.

Zum Vergleich: die Schwesterfälle geben die alte Kopie **zuletzt** frei und
messen sofort — M-01 `:1729-1730`, M-02 `:1791-1792`, M-03 `:1840-1841`,
M-05 `:1894-1895` (je `gate.freigeben(); ruheAbwarten();` ohne weiteren
Handgriff). M-06 ist der einzige Fall der Gruppe mit nachgelagerten
Handgriffen.

### 1.6 Die Produktpfade nach der Freigabe publizieren den aktuellen State

`eq-copilot/plugin/src/prozessor/Ipc.cpp:1205-1243`
(`benenneSourcesHauptziel`), Kernzeilen:

```
:1215            std::lock_guard<std::mutex> l (bindungMutex);
:1224            gefunden->label = label;
:1225            kopie = zustand.mainProjectMitglieder;
:1228            generation = reloadGeneration.load();
:1232            folge = naechsteSourcesFolgeUnterBindung();
:1240        werteSourcesPublikationAus (
:1241            sourcesModel.setzePersistenteMitglieder (kopie, generation, folge));
```

`eq-copilot/plugin/src/prozessor/Ipc.cpp:1245-1287`
(`entferneSourcesHauptziel`, Zweig ohne gültige Runtime-Nonce):

```
:1263            std::lock_guard<std::mutex> l (bindungMutex);
:1273            zustand.mainProjectMitglieder.erase (gefunden);
:1274            kopie = zustand.mainProjectMitglieder;
:1275            generation = reloadGeneration.load();   // R-A1 Punkt 4' (b)
:1276            folge = naechsteSourcesFolgeUnterBindung();   // NAK-283 F01 (M-03)
:1282        werteSourcesPublikationAus (
:1283            sourcesModel.setzePersistenteMitglieder (kopie, generation, folge));
```

Beide Handgriffe ziehen ihre Kopie **frisch aus `zustand.mainProjectMitglieder`
unter `bindungMutex`** und publizieren sie. Jede solche Publikation trägt die
jüngste Folgenummer; ohne Folgenummernriegel (Mutation a) wird sie im Modell
bedingungslos übernommen:

`eq-copilot/plugin/src/SourcesModel.cpp:349-436`, mutierte Stelle
`:383-387`:

```
:383    if (folge <= zuletztUebernommeneFolge)
:384    {
:385        ++ueberholtZaehler;
:386        return Publikation::ueberholt;
:387    }
```

und `:396-398`:

```
:396    if (neu == persistenteMitglieder)
:397        return Publikation::uebernommen;  // Publikation fand statt, ohne Aenderung.
:398    persistenteMitglieder = std::move (neu);
```

`eq-copilot/plugin/src/prozessor/Ipc.cpp:1462-1490`
(`werteSourcesPublikationAus`) zeigt, dass nur der Ausgang `ueberholt` den
Zähler `sourcesPublikationUeberholt` erhöht (`:1477`).

### 1.7 Der Rotbeweis selbst

`git show 988dc44d:docs/beweise/roh/NAK-283-rot-M-06-etappe-2.txt`,
mutierter Lauf (Mutation: „SourcesModel: die Folgenummernprüfung entfernen -
nur der Generationsvergleich bleibt, wie am Basis-SHA"):

```
  ok      M-06: nach dem letzten Handgriff ist Ruhe
  ok      M-06: state_und_modell_sind_nach_ruhe_gleich - dieselbe Schluesselmenge,
          je Schluessel dasselbe Label  [State: aaaa=Fluegel | Modell: aaaa=Fluegel]
  FEHLER  M-06: genau die eine angehaltene Kopie wurde ueberholt - die drei
          seriellen Wege nicht  [0]
SONDE-012 ProjectReload (nur 283m06): 7/8 gruen
```

Zum Kontrast derselbe Mutantenlauf bei M-01,
`docs/beweise/roh/NAK-283-rot-M-01a-etappe-2.txt`:

```
  FEHLER  M-01: aeltere_mitgliederpublikation_ersetzt_keine_juengere - nach Ruhe
          traegt das Modell GENAU [A], nicht die aeltere Kopie [B, A]
          [aaaa=Eingeschleust bbbb=Eingeschleust]
  FEHLER  M-01: die aeltere Publikation ist als UEBERHOLT gezaehlt - im Prozessor
          und im Modell  [0 / 0]
```

Bei M-01 fällt **der Bestandsvergleich** mit; bei M-06 fällt er nicht.

### 1.8 CLAUDE.md

`CLAUDE.md:183-184`:

> - **Engine kennt keine Optik.** Sie liefert kohärente Mess-Snapshots; der
>   Editor hält Anzeigezustand und rendert nur neue Snapshot-Revisionen.

`CLAUDE.md:195-197`:

> - **State bleibt verlustfrei.** Unbekannte Major-Versionen oder unzulässige
>   Matrizen werden read-only mit Originalbytes gehalten; jede persistente
>   Änderung meldet dem Host Dirty-State.

---

## 2. Ist jede Teilbehauptung am Stand `988dc44d` wahr?

| Teilbehauptung | Urteil | Beleg |
|---|---|---|
| Fundstelle `Sonde012ProjectReloadTest.cpp:1944-1949` | **wahr** | `:1944` `s.gate.freigeben();`, `:1949` `pruefe (s.p->benenneSourcesHauptziel (a, "Fluegel"), …)` |
| Nach Freigabe der alten Kopie laufen noch Handgriffe | **wahr** | zwei: Benennung `:1949`, lokaler Unbind `:1958`; beide nach `:1944` |
| Diese Handgriffe publizieren erneut den aktuellen State | **wahr** | `Ipc.cpp:1225`/`:1240-1241` und `Ipc.cpp:1274`/`:1282-1283` — Kopie frisch aus `zustand.mainProjectMitglieder` unter `bindungMutex`, dann `setzePersistenteMitglieder` |
| Sie reparieren dadurch den zu messenden Fehler | **wahr** | ohne Folgenummernriegel (`SourcesModel.cpp:383-387` entfernt) wird jede dieser Publikationen übernommen (`:398`); die letzte gewinnt, und sie trägt genau den State — der Rücksprung durch die alte Kopie ist überschrieben |
| Trotz entfernter Folgenummernprüfung bleibt der Bestandsvergleich grün | **wahr** | Rotdatei: „ok  M-06: state_und_modell_sind_nach_ruhe_gleich … [State: aaaa=Fluegel \| Modell: aaaa=Fluegel]" |
| Ausschließlich der Überholt-Zähler fällt | **wahr** | Rotdatei: eine einzige `FEHLER`-Zeile, „genau die eine angehaltene Kopie wurde ueberholt … [0]"; 7/8 grün |
| Der fallende Zähler ist ein Nebeneffekt, nicht die Zusage von M-06 | **wahr** | Zusagespalte M-06 (`:602`) nennt nur den Bestandsvergleich; der Überholt-Zähler ist die Zusage von M-01 (`:597`), M-71 (`:609`), M-72 (`:610`) |
| Bezug `CLAUDE.md:183-184` | **wahr, aber nur mittelbar** | M-06 führt „Engine kennt keine Optik" in der Quellenspalte; verletzt ist nicht die Invariante selbst, sondern die Regel, mit der sie gemessen wird (§6.1, Gate) |

**Direkte Antwort auf die beiden Kernfragen:**

1. **Läuft nach der Freigabe der alten Worker-Kopie noch ein heilender
   Handgriff?** Ja — **zwei**: `benenneSourcesHauptziel` (`:1949`) und
   `entferneSourcesHauptziel` (`:1958`). Beide publizieren den State, wie er
   unter `bindungMutex` gerade ist. Der letzte davon setzt das Modell
   deterministisch auf den State und schließt jede vorher entstandene
   Abweichung. Es handelt sich um Handgriffe, nicht um einen Heilungstakt des
   Produkts — M-06 verbietet aber genau das: „Die Zusage muss ohne weiteren
   Befehl halten" (`:602`).
2. **Fällt im mutierten Lauf der Bestandsvergleich oder nur der Zähler?**
   **Nur der Zähler.** Die Bestandsvergleichszeile `:1976-1979` steht im
   Mutantenlauf ausdrücklich auf `ok` mit `State: aaaa=Fluegel | Modell:
   aaaa=Fluegel`; rot ist einzig `:1980-1983`
   (`sourcesPublikationUeberholtFuerTest() == 1`, gemessen `0`).

---

## 3. Welcher Satz bricht

Drei Sätze brechen, alle wörtlich:

1. **§6.1 Punkt 2** (`docs/beweise/NAK-283.md:799-800`): „**Mutation** genau an
   der Zeile, die die Zusage trägt — **nie an einem Nebeneffekt**." Die
   Mutation sitzt zwar an der richtigen Produktzeile
   (`SourcesModel.cpp:383-387`), aber der Fall fällt nur an einem Nebeneffekt
   (Überholt-Zähler). Gemessen wird damit nicht M-06, sondern ein Stück von
   M-01/M-71/M-72, das dort bereits eigene Rotbeweise hat.

2. **Kopfzeile Gate** (`docs/beweise/NAK-283.md:8`): „… gemessen durch die
   Matrixzeilen M-01 bis M-75 (§5) **mit Rotbeweis an der Zusagezeile**." Die
   Zusagezeile von M-06 ist der Bestandsvergleich; sie ist im Mutantenlauf
   grün.

3. **M-06, Spalte Reihenfolge und Frist** (`docs/beweise/NAK-283.md:602`): „Es
   gibt keinen Heilungstakt … **Die Zusage muss ohne weiteren Befehl halten**."
   Der gebaute Fall erreicht die Ruhe erst nach zwei weiteren Befehlen, die
   genau das leisten, was der Satz ausschließt. Zusätzlich trifft die
   Rotbeweisspalte derselben Zeile („Mutation wie M-01 → State trägt `[A]`,
   Modell `[B, A]`") nicht zu: der Mutantenlauf zeigt State und Modell gleich.

`CLAUDE.md:183-184` selbst bricht **nicht** — die Invariante „Engine kennt
keine Optik" wird vom Produktcode gewahrt; unbelegt ist nur ihre Messung.

---

## 4. Einordnung

**DEFEKT.**

Die Matrix ist Spezifikation und verlangt für M-06 einen Rotbeweis an der
Zusagezeile; der gebaute Fall liefert ihn an einem Nebeneffekt, und der Bruch
ist an drei wörtlichen Sätzen belegbar (§6.1 Punkt 2, Gate-Kopfzeile, M-06
Frist-Spalte), nicht bloß aus einer Lücke abgeleitet. Die Rotbeweisspalte von
M-06 sagt die Divergenz „State `[A]`, Modell `[B, A]`" ausdrücklich zu — die
Rohdatei widerlegt sie am eigenen Lauf, also fehlt der Zeile ihr Beleg
vollständig und sie zählt heute faktisch als Regressionswache (§6.1 letzter
Absatz: „Eine Probe, die auch ohne den Fix rot ist, heißt Regressionswache und
ist kein Beleg" — hier sogar schwächer: sie ist **mit** dem Fehler grün). Der
Punkt ist nicht durch §20.6 oder §20.7 gedeckt: dort steht nur die erklärte
Abweichung zum fünften Publikationsweg, nichts zur Reihenfolge der Handgriffe.

---

## 5. Die schließende Regel

Ohne Codevorschlag; messbar zu erfüllen ist:

1. **Reihenfolge der Handgriffe.** Alle Publikationswege, die den Lauf
   ausmachen (Weg 4 Bindung, Weg 1 Benennung, Weg 2 lokaler Unbind, Weg 3
   Drain-Nachführung aus beiden Threads), laufen **vor** der Freigabe der
   angehaltenen Worker-Kopie. Die angehaltene Kopie muss zum Zeitpunkt der
   Freigabe **älter** sein als der dann geltende State, also mindestens einen
   State-ändernden Handgriff hinter sich haben.

2. **Letzter Schritt vor der Ruhe.** Der letzte Schritt des Falls ist die
   Freigabe der alten Kopie plus das Abwarten ihrer Nachführung
   (`ruheAbwarten`). Danach darf **kein** Handgriff, kein Tick, kein
   Fixture-Setzen und keine weitere Publikation mehr folgen — genau wie in
   M-01 (`:1729-1730`), M-02 (`:1791-1792`), M-03 (`:1840-1841`) und M-05
   (`:1894-1895`). Erst dann werden `holeZustandKopie().mainProjectMitglieder`
   und `sourcesPersistenteMitgliederFuerTest()` gelesen.

3. **Welche Prüfzeile im Rotbeweis fallen muss.** Unter der Mutation
   „Folgenummernprüfung entfernt" (`SourcesModel.cpp:383-387`) muss die
   Prüfzeile **„M-06: state\_und\_modell\_sind\_nach\_ruhe\_gleich — dieselbe
   Schlüsselmenge, je Schlüssel dasselbe Label"** (heute `:1976-1979`) auf
   `FEHLER` gehen, mit einer Wertspalte, die die Divergenz zeigt (State und
   Modell tragen verschiedene Schlüsselmengen oder verschiedene Labels). Fällt
   im Mutantenlauf ausschließlich die Zählerzeile, gilt der Rotbeweis als
   **nicht gemessen**. Die Zählerzeile darf bleiben; sie ist dann zusätzlicher
   Befund, nicht der Beleg.

4. **Unverändert gültig bleibt**, dass der Fall die vier Produktwege in einem
   Lauf hält (§20.7 Punkt 2 erklärt den fünften) und dass die Bühne das
   Interleaving über den bestehenden Publikationshaken **erzwingt** (§6.1
   Punkt 2: „eine Zeile, die nur ‚manchmal' rot wird, ist kein Rotbeweis").

5. **Rücknahme.** Der neue Rotbeweis läuft nach §6.1 vollständig: SHA-256 vor
   der Mutation, Zeitstempel setzen, Bein rot, bytegleiche Rücknahme,
   Zeitstempel, Bein grün; Rohdatei ersetzt
   `docs/beweise/roh/NAK-283-rot-M-06-etappe-2.txt`, §20.2 Zeile M-06
   (`:2736`) wird mit derselben Änderung nachgezogen.

---

## 6. Scope-Beweis: gelesene Dateien

Nur gelesen; keine Änderung an Code oder Dokumenten außer dieser Datei.

- `docs/beweise/NAK-283.md` — `:8` (Gate), `:595-615` (§5.1 Kopf, M-01 bis
  M-12, M-71, M-72), `:786-830` (§6.0-Rest, §6.1, §6.2-Anfang), `:838-935`
  (§6.3), `:1845-1900` (§8.1 Feinheit 19 und 20, §8.2, §8.3-Anfang), Trefferzeilen
  zu Feinheit 18 (`:617`, `:1798`, `:1993`, `:2188`), `:2720-2760` (§20.2),
  `:2845-2935` (§20.6, §20.7)
- `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` — `:1633-1695`
  (`Buehne`, M-01-Kopf), `:1900-2050` (M-05-Ende, M-06 vollständig
  `:1917-1984`, M-71, M-72-Kopf); zusätzlich Trefferzeilen zu
  `gate.freigeben` / `ruheAbwarten` / `scharf` über die ganze Datei
- `eq-copilot/plugin/src/prozessor/Ipc.cpp` — `:1195-1300`
  (`benenneSourcesHauptziel` `:1205-1243`, `entferneSourcesHauptziel`
  `:1245-1287`, `sendeSourcesCommand`-Anfang ab `:1289`), `:1440-1545`
  (`naechsteSourcesFolgeUnterBindung`,
  `werteSourcesPublikationAus`, `meldeSourcesMitgliederNachBefehl`,
  `wendeBestaetigteSourcesCommandsAn`-Anfang)
- `eq-copilot/plugin/src/SourcesModel.cpp` — `:345-440`
  (`setzePersistenteMitglieder` vollständig)
- `docs/beweise/roh/NAK-283-rot-M-06-etappe-2.txt` — vollständig (aus
  `git show 988dc44d:…`)
- `docs/beweise/roh/NAK-283-rot-M-01a-etappe-2.txt` — vollständig (aus
  `git show 988dc44d:…`)
- `CLAUDE.md` — `:176-202` (Tragende technische Invarianten)
- `.gitattributes` — `:1-30` (Zeilenenden-Regel für die neue Datei)
- Git nur lesend: `git log --oneline -1`, `git status --short`,
  `git diff --stat 988dc44d HEAD -- …`, `git diff --stat HEAD -- …`,
  `git show 988dc44d:<datei>`, `git ls-files --eol docs/beweise/roh/`
