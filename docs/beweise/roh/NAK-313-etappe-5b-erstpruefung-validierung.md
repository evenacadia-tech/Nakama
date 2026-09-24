# NAK-313 — Validierung des Befunds der Erstprüfung 5b (lesender Opus-Thread, Effort max)

- **Auftrag:** `docs/beweise/roh/NAK-313-etappe-5b-erstpruefung-validierung-auftrag.txt` (Dirigent, 24.09.2026).
- **Befund:** Rohurteil `docs/beweise/roh/NAK-313-etappe-5b-erstpruefung-urteil.md:35-36` ([P2] zu `tools/beweise.ps1:773`), dazu Prüfteil (c) `:13` und (h) `:23`.
- **HEAD zu Beginn:** `97872e87c089170f5d38c7e9189d850632e7e651` (24.09.2026, 19:41:35 +02:00, Get-Date).
- **HEAD am Ende:** `97872e87c089170f5d38c7e9189d850632e7e651` (24.09.2026, 20:00:38 +02:00, Get-Date, nach dem Schreiben dieses Berichts; `git status --short` zeigt nur die vorhandenen untracked Pfade und diesen Bericht).
- **Modell:** Claude Opus 5.5 (`claude-opus-5-5`), Effort max.
- **Arbeitsweise:** nur gelesen (Dateien, `git show`, `git diff`, `git log`, Suche). Kein Compiler, kein Test, kein cargo, kein FL Studio, kein Mutationslauf. Nichts gestaged, committet oder gepusht. Die einzige geschriebene Datei ist dieser Bericht.
- **Stand:** Zeilenangaben gelten am HEAD. `git diff --stat b40a1bf0 HEAD` nennt nur `docs/beweise/roh/NAK-313-etappe-5b-erstpruefung-auftrag.txt`; Code und Runner sind gleich dem geprüften ZIEL `b40a1bf0`. Zeilenangaben innerhalb zitierter Matrixzellen gelten zum Basis-SHA `2b7472d2` (`docs/beweise/NAK-313.md:896-897`).

## 1. Wortlaut von B27 und Deckung je Teilsatz

`tools/beweise.ps1:773`, Behauptung wörtlich:

> Der musikalische Intent als Projektintent im MainProjectState (§37.1, U22, M-01 bis M-09/M-11). Die Rollenmenge ist GESCHLOSSEN und hat genau fuenf Werte; ein sechster faellt in der Produkt-API UND im Leser, statt still auf einen bekannten Zweig abgebildet zu werden. Die §37.1-Belegung ist ABGELEITET und damit total und injektiv: alle fuenf Rollen kommen ueber den Roundtrip zurueck, keine zwei teilen sich eine Belegung (alle zwanzig geordneten Paare), und eine fremde Belegung gehoert zu keiner Rolle. Schutzangaben sind ORTHOGONAL zur Rolle - eine Quelle ohne Rolle kann Attack geschuetzt haben, und keiner der fuenf Rollenwechsel loescht einen Schutzbereich. Die Rolle bewusst-verschmolzen ist ein globales Veto gegen Entmaskierung; eine ausdrueckliche gerichtete Beziehung hebt es NUR fuer dieses Paar auf, nicht fuer ein drittes. Die Konfliktregeln sind eine geordnete Liste, kein Score: alle zehn Paarungen der fuenf Stufen fallen in beiden Leserichtungen, hundert Vermutungen schlagen die Schutzgrenze nie, und innerhalb Stufe 2 gewinnt das Spezifischere. Ein Zyklus im fuehrt_vor-Graphen erreicht die Persistenz NIE unmarkiert - die Pruefung laeuft beim Speichern, der Leser weist einen von Hand gebauten Zyklus als read-only ab, und derselbe Schluss als gleichrangig gespeichert laedt und kommt als gleichrangig zurueck. Eine abgeleitete Vermutung ueberschreibt einen Userwert nie, der umgekehrte Weg gilt. Derselbe Bus traegt verschiedene Rollen in zwei Passagen ohne Datenverlust, passagespezifisch vor global. Dazu die fuenf Teile der State-Invariante: Fassung im Namen, Altstand ohne die Felder laedt bytegleich, eine unbekannte Eigenschaft ueberlebt, Save/Load ueber zwei Runden bytegleich, und jede echte Aenderung meldet GENAU einmal Host-Dirty, waehrend No-op, abgewiesener Wert, Laden und read-only schweigen. Jede Grenze faellt von BEIDEN Seiten (17 Leserfaelle samt NaN/Inf-Konfidenz, Revision 0, Bandrand 0/221 und Selbstbeziehung), und die volle 256er-Liste, die die API erzeugt, laedt der eigene Leser. Zuletzt M-11 am Produktpfad: zwei echte Prozessoren mit demselben Strom, einer mit vollem Intent, liefern bitgleiche Evidenzbaender, Perzentile, Livebaender, metrics_version und NaN-Zaehler - mit Gegenprobe, dass die Baender wirklich Werte tragen. Seit NAK-313 Etappe 5b (R-313-4; 313/M-80, M-81, M-89): die Revisionsraender der NAK-283-Faelle M-07 bis M-10 und M-12 stehen bei 2^53-1 (assistentenrevision_laeuft_nicht_ueber misst den neuen Rand); Bestands- und Eintragsrevision gehen bei 2^53-2 genau einen Schritt mit genau einem Host-Dirty und weisen am Rand ab, bevor sie etwas zuweisen (Prozessor ohne Host-Dirty, Bibliothek mit Grund und gleichen Bytes); der Intentwriter schreibt bestand_revision 2^53-1 ab bestand_revision bytegleich zur Handinstanz intent-wire-v1.json.

Teiletappe 5b hat nur den letzten Satz ab „Seit NAK-313 Etappe 5b“ angehängt (Commit `28e9d571`; an `062a98fc` ist die Zeile bis Zeichen 2401 gleich). Nur dieser Satz ist Gegenstand des Befunds. Die SONDE-014-Sätze davor sind nicht geprüft (§6).

| Teilsatz des 5b-Satzes | Fall in `eq-copilot/plugin/tests/Sonde014IntentTest.cpp` (Name, Zeilen) | gedeckt |
|---|---|---|
| T1 „die Revisionsraender der NAK-283-Faelle M-07 bis M-10 und M-12 stehen bei 2^53-1“ | `NAK-283 M-07 bis M-09` 1329-1418 (Rand `kMax` = 2^53−1, 1242, 1334); `M-08/M-09 (Host)` 1440-1492; `M-10` 1498-1560 (1503); `M-12 (Wache)` 1571-1782 (Start 2^53−2, Ziel 2^53−1, 1576) | ja |
| T2 „(assistentenrevision_laeuft_nicht_ueber misst den neuen Rand)“ | `NAK-283 M-10 assistentenrevision_laeuft_nicht_ueber` 1498-1560, Prüftext 1555-1557 | ja |
| T3a Bestandsrevision: „Bestands- und Eintragsrevision gehen bei 2^53-2 genau einen Schritt mit genau einem Host-Dirty“ | `313/M-80 bestandsrevision_haelt_an_der_grenze: bei 2^53-2 genau ein Schritt auf 2^53-1, 1 Host-Dirty, Reload schreibbar` 1819-1839; zusätzlich `M-12 Host (Intent/Schutz/Beziehung)` 1636-1721 | ja |
| T3b Eintragsrevision: derselbe Teilsatz | kein Fall | **ungedeckt** |
| T4a/T5a Bestand: „und weisen am Rand ab, bevor sie etwas zuweisen (Prozessor ohne Host-Dirty“ | `313/M-80 …: bei 2^53-1 Rueckgabe false, Bestand und Revision unveraendert, 0 Host-Dirty` 1841-1852 | ja |
| T6a Bestand: „Bibliothek mit Grund und gleichen Bytes“ | `313/M-80 … (Bibliothek)` 1855-1871 | ja |
| T4b/T5b Eintrag: derselbe Teilsatz wie T4a/T5a | `313/M-81 eintragsrevision_haelt_an_der_grenze: Rueckgabe false vor jeder Zuweisung …, 0 Host-Dirty` 1885-1900 | ja |
| T6b Eintrag: derselbe Teilsatz wie T6a | `313/M-81 … (Bibliothek)` 1903-1916 | ja |
| T7 „der Intentwriter schreibt bestand_revision 2^53-1 … bytegleich zur Handinstanz intent-wire-v1.json“ | `313/M-89 intentwriter_revision_am_rand` 1919-1946 | ja |

**Vergleich der Form** (je der 5b-Satz):

- B7 (`tools/beweise.ps1:728`): „bei 2^53-2 committet genau eine Transaktion auf 2^53-1, die naechste scheitert in S4 mit revision_erschoepft …“. Den positiven Schritt misst `313/M-83 transaktion_haelt_an_2hoch53` (`eq-copilot/plugin/tests/TransactionTestMain.cpp:6631-6638`, Einzelbruch M83b).
- B29 (`:774`): für die Assistentenrevision nur „an der Revision 2^53-1 weisen Weiter, Antwort und ein neuer Lauf ab, ohne etwas zu aendern und ohne Host-Dirty“, also genau M-82.
- B2 (`:693`): nur Leserränder (2^53−1 schreibbar, 2^53 und `int64max` read-only) und NAK-283 M-11.
- B27 ist die einzige der vier Zeilen, die einen positiven Schritt nennt, den kein Fall misst.

Die Beweiszeile übernimmt den Satz: der Kanonabschnitt 5b trägt ihn in `docs/beweise/NAK-313.md:7087` wörtlich mit „[OK] Exit 0“.

## 2. Was m80() und m81() einrichten und messen

`m80()` (`Sonde014IntentTest.cpp:1815-1872`):

| Teil | Einrichtung | Ereignis | gemessen |
|---|---|---|---|
| Prozessor, Schritt | `MainProject.intent_revision_v1` = 2^53−2, **kein** `source_intents_v1` (`baumMitEigenschaft (…, kRand - 1, false)`, 1820); Prozessor auf dem Heap (1821-1822); Dirty-Zähler nach dem Laden (1823-1824); Vorbedingung schreibbar und Bestand 2^53−2 (1825) | `setzeQuellenrolle (kQuelleA, …, fuehrt, …)` legt einen **neuen** Eintrag an (1826-1827) | Rückgabe true, Bestand 2^53−1, 1 Host-Dirty, Reload schreibbar mit Bestand 2^53−1 (1835-1837). Der neue Eintrag trägt Revision 1 (`NakamaState.cpp:2454`); eine Eintragsrevision nahe am Rand kommt nicht vor. |
| Prozessor, Rand | derselbe Prozessor, Bestand 2^53−1 | `setzeQuellenrolle (kQuelleB, …, traegt, …)`, wieder ein neuer Eintrag (1842-1843) | Rückgabe false, Bestand 2^53−1, Einträge unverändert, 0 Host-Dirty (1847-1849) |
| Bibliothek, Rand | `intent_revision_v1` = 2^53−1, kein Eintrag, geladen (1857-1858) | `state::setzeIntent (… kQuelleB …)` (1863-1864) | false, `veraendert` false, Grund „intent revision would overflow“, Bestand 2^53−1, Bytes gleich (1867-1870) |

`m81()` (`Sonde014IntentTest.cpp:1874-1917`):

| Teil | Einrichtung | Ereignis | gemessen |
|---|---|---|---|
| Prozessor, Rand | ein Eintrag `[kQuelleA, "", fuehrt, 2^53−1, user, 1.0]`, Bestand 1 über die Voreinstellung `mitRevision` (143-152; 1879-1880); Heap; Dirty-Zähler nach dem Laden; Vorbedingung schreibbar, ein Eintrag mit 2^53−1, Bestand 1 (1885-1887) | `setzeQuellenrolle (kQuelleA, …, traegt, …)`, geänderte Rolle am bestehenden Eintrag (1888-1889) | Rückgabe false, Eintrag unverändert (Rolle `fuehrt`, Revision 2^53−1), Bestand 1, 0 Host-Dirty (1893-1897) |
| Bibliothek, Rand | derselbe Baum, geladen (1903-1904) | `state::setzeIntent (… kQuelleA … traegt …)` (1909-1910) | false, `veraendert` false, Grund, Bytes gleich (1913-1915) |

Die Eintragsrevision wird nur am Rand 2^53−1 gemessen, als Abweisung. Ein Schritt der Eintragsrevision kommt in keinem der beiden Fälle vor.

**Suche nach einem Fall, der einen Eintrag bei 2^53−2 anlegt und den Schritt auf 2^53−1 mit genau einem Host-Dirty misst: keiner.**

- B27: `kRand - 1` und `kMax - 1` stehen nur in Bestands- (1576, 1820) und Assistenteneinrichtungen (1611, 1724). Jede `source_intents_v1`-Einrichtung trägt Revision 1, 3 oder 2^53−1 (235-237, 1015-1016, 1037-1039, 1879-1880); die Tabelle 945-975 misst Leserfehler.
- B2: `313/M-75` misst nur den Leser bei 2^53−1, 2^53 und `int64max` (`StateMigrationTestMain.cpp:1374-1378`, `:1448-1459`). Der NAK-283-Fall M-11 legt seinen Eintrag normal mit Revision 1 an (`:4003-4005`). `maximalerMain` setzt Einträge direkt auf 2^53−1 (`:822-826`); die M-121-Fälle wechseln nur die Rolle (`setzeBindung`, `:4302-4325`) und ändern keinen Eintrag.
- B7: `TransactionTestMain.cpp` kennt keinen Intent (0 Treffer für `source_intents`, `setzeQuellenrolle`, `setzeIntent`, `intentBestand`).
- B29: `setzeQuellenrolle` nur an einem Normalstand (`Sonde014AssistentTest.cpp:842`); `kRand` nur für die Assistentenrevision (`:239`, `:1384`, `:1401`, `:1428`, `:1507`).

Der Rotlauf MC zeigt den Schritt nur unter der Mutation und über den Rand hinaus („[angenommen, Eintrag 9007199254740992, Bestand 2, 1 Dirty]“, `docs/beweise/roh/NAK-313-rot-M-81.txt:48`). Ein Beleg für den letzten gültigen Schritt am HEAD ist das nicht.

## 3. Was Matrix, Regel und Bauplan verlangen

**Matrixkopf** (`docs/beweise/NAK-313.md`): „*Rotbeweis* das Etikett und die Mutation an der Zeile, die die Zusage trägt; *Quelle* die Zusage aus §5 oder die Regel aus §2“ (`:894-895`); „**Regressionswache**: heute grün und hält, was der Bau nicht ändern darf; sie wird einmal absichtlich gebrochen und ist kein Beleg“ (`:903-905`); „**zwei Hälften**: die Zeile trägt zwei Etiketten und sagt je Hälfte, welches gilt“ (`:906-907`); „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt, nie an einem Nebeneffekt“ (`:911-912`).

**M-80** (`:1142`), Mutator Bestandsrevision:

- Zustand: „Bestand mit `intentBestandRevision` 2^53−2 und 2^53−1“; Ereignis: „`setzeQuellenrolle` (erste und zweite Änderung)“.
- Zusage: „**Bei 2^53−2 genau ein Schritt auf 9007199254740991, 1 Host-Dirty, Reload schreibbar; bei 2^53−1 Rückgabe false, Bestand und Revision unverändert, 0 Host-Dirty (der Grund entsteht im Bibliotheksaufruf `setzeIntent`; der Prozessor gibt nur false zurück, `State.cpp:523-526`) — kein Umklappen, keine Sättigung, keine 0**“.
- Rotbeweis: „**zwei Hälften.** Abweisung bei 2^53−1: **heute rot** — die Schranke liegt bei `int64max` (`:2337`), der zweite Handgriff hebt auf 2^53 (`:2342`), 1 Host-Dirty; am Basis-SHA lädt dieser Stand sogar schreibbar (der Leser hat keine Obergrenze, `:1971-1985`); nach dem Bau: Schranke auf `int64max` zurück → 2^53, Reload `nurLesen` → rot. Schritt auf 2^53−1: **Regressionswache**; einmal gebrochen: Schranke `>= kRevisionMax − 1` → schon der erste Handgriff wird abgewiesen → rot“.
- Quelle: „R-313-4 Satz 3; R-313-12; R-313-15“.

**M-81** (`:1143`), Mutator Eintragsrevision:

- Zustand: „Eintrag mit `revision` 2^53−1, Bestand darunter“; Ereignis: „`setzeQuellenrolle` (→ `setzeIntent`, `NakamaState.cpp:2352`) mit geänderter Rolle“.
- Zusage: „**Rückgabe false vor jeder Zuweisung: Rolle, Eintragsrevision 9007199254740991 und Bestandsrevision unverändert, 0 Host-Dirty (das Dirty entsteht nur am Prozessor, `State.cpp:528-531`)**“.
- Reihenfolge: „Prüfung `NakamaState.cpp:2390-2393` gegen 2^53−1 vor `bestandsrevisionHeben`“.
- Rotbeweis: „**heute rot** — Schranke bei `int64max` (`:2390`), danach `++treffer->revision` auf 2^53 (`:2398`). Nach dem Bau: Schranke zurück → 2^53 → rot“. Ein Etikett, keine zweite Hälfte.
- Quelle: „R-313-4 Satz 3; R-313-15“ (ohne R-313-12).

**M-82** (`:1144`) zum Vergleich: Zusage „**Alle drei Rückgabe false; Schritt, `offen`, Ergebnis und Revision 9007199254740991 unverändert; 0 Host-Dirty (den Grund misst B27 auf Bibliotheksebene, M-79; der Prozessor verwirft ihn, `Analyse.cpp:1145-1149`)**“; Rotbeweis „**heute rot** (Schranke bei `int64max`, `:2807`). Nach dem Bau: Schranke zurück → 2^53 → rot“; Quelle „R-313-4 Satz 3; R-313-15“. Auch M-82 verlangt nur die Abweisung. M-83 (`:1145`) hat wie M-80 zwei Hälften, darunter „Commit auf 2^53−1: **Regressionswache**“.

**Antwort.** M-81 verlangt nur die Abweisung am Rand, nicht den positiven Eintragsschritt, und ist belegt: §31.4 „Abweisung am Rand: heute rot – rot – MC“, „JA (1 von 1 Hälften)“ (`:7311`); `docs/beweise/roh/NAK-313-rot-M-81.txt:10-41` enthält nur diese Hälfte. M-80 trägt den positiven Schritt nur für den Bestand (W80, `:7273`; `NAK-313-rot-M-80.txt:81-110`).

- **R-313-4** (`:135-153`) sagt den Schritt nicht wörtlich. Satz 1: „Alle persistenten Revisionen (Intent-Bestand, Intent-Eintrag, Assistentenschritt, Undo, `Dsp.state_revision`) und ihre Drahtform haben einen Bereich: Untergrenze wie heute (0 beziehungsweise 1), Obergrenze 2^53−1.“ (`:135-138`). Satz 3: „Mutatoren halten an der Grenze (kein Umklappen, keine Sättigung, keine 0) und prüfen vor jeder Nebenwirkung …“ (`:140-143`). „An der Grenze halten“ mit der Obergrenze 2^53−1 schließt den letzten Schritt auf 2^53−1 ein. Die Matrix liest denselben Satz 3 in M-80 als zwei Hälften (mit R-313-12), in M-81 nur als Abweisung.
- **R-313-12** (`:3103-3108`): „Das Etikett … wird je Satz der Zusage am Basis-SHA gemessen, nicht je Zeile vergeben; … und jede Wache hat ihren Einzelbruch dort, wo sie nach dem Bau allein trägt.“ Die Regel gilt je Satz der Zusage; die Zusage von M-81 hat einen Satz.
- **Bauplan §8.5**, „Revisionsbereich“: „Die Mutatoren vergleichen gegen `kRevisionMax` statt `int64max`.“ (`:2306-2307`), ohne Messforderung für den Schritt. „Behauptungen“: „B2, B7, B27, B29 (Revisionsbereich 2^53−1), B27 zusätzlich „der Intentwriter schreibt `bestand_revision` 2^53−1 bytegleich zur Handinstanz `intent-wire-v1.json`““ (`:2429-2431`). Der Satz über einen Schritt „bei 2^53-2“ für Bestand und Eintrag stammt nicht aus dem Bauplan.
- **Wörtlich verlangt** ist der positive Schritt für alle Mutatoren nur in der Präzisierung des Dirigenten zu 5b: „PRÄZISIERUNGEN DES DIRIGENTEN (gehen dem Bauplantext vor):“ (`docs/beweise/roh/NAK-313-etappe-5b-auftrag.txt:14`) und „Mutatoren: bei 2^53−2 genau ein Schritt auf 2^53−1 mit genau einem Host-Dirty; bei 2^53−1 Rückgabe false vor jeder Zuweisung, 0 Host-Dirty, Bestand und Revision unverändert (M-80 bis M-83).“ (`:17`). Der Prüfauftrag fragt es in (c) ausdrücklich für die Eintragsschranke ab: „halten bestandsrevisionHeben, die Eintragsschranke in setzeIntent und assistentenrevisionHeben mit >= kRevisionMax vor jeder Zuweisung und vor jedem Host-Dirty (bei 2^53−2 genau ein Schritt auf 2^53−1 mit genau einem Host-Dirty; …)“ (`docs/beweise/roh/NAK-313-etappe-5b-erstpruefung-auftrag.txt:26`). Der Wortlaut von B27 folgt dieser Präzisierung; die Fälle folgen der Matrixzeile.

**Etappenbericht §31:**

- §31.3: „`tools/beweise.ps1`: Behauptungen B2, B7, B10, B27, B29 und A4 je ein Satz zum Revisionsbereich“ (`:7221-7222`); die Mutatoren „`bestandsrevisionHeben`, die Eintragsschranke in `setzeIntent` und `assistentenrevisionHeben` halten an `kRevisionMax` vor jeder Zuweisung“ (`:7175-7177`).
- §31.4: MB (`:7272`), W80 (`:7273`, nur `bestandsrevisionHeben` um eins zu eng), MC (`:7274`, nur die Abweisung der Eintragsschranke); Zeilentabelle M-80 „2 von 2 Hälften“, M-81 „1 von 1 Hälften“ (`:7310-7311`). Kein Lauf bricht die Eintragsschranke um eins zu eng.
- §31.7 (`:7376-7475`): keine Abweichung zu M-81 oder zur B27-Behauptung.
- §31.8 C: „Zahlenränder je Revision — 2^53−2, 2^53−1, 2^53 und `int64max` an jedem Leser, jedem Mutator, dem Transaktionskern und beiden Sendern“ (`:7485-7487`); E: „die sieben Behauptungssätze nennen nur Gemessenes“ (`:7494`). Für die Eintragsschranke bei 2^53−2 treffen beide Sätze nicht zu (`docs/**`, kein Prüfgegenstand; Kontext für die Einordnung).
- §31.9 enthält den Satz „Behauptungen nicht weiter als die Messung“ nicht. Er steht im Auftrag 5b, Schritt 9 Selbstaudit (`NAK-313-etappe-5b-auftrag.txt:39`). §31.9 nennt für die Mutatoren nur die Abweisung: „`setzeIntent` prüft die Eintragsgrenze vor `bestandsrevisionHeben`; der Prozessor gibt am Rand `false` ohne Host-Dirty zurück (M-80 bis M-82, M-84, M-86: 0 Host-Dirty gemessen)“ (`:7513-7515`). Das ist gedeckt.
- Prüfliste E (`tools/dirigent/pruefliste.md:62-63`): „Jede Behauptung im Runner, Manifest oder Kommentar sagt nicht mehr, als der Test misst“. Für den Prüfer ist die Liste „keine Anforderungsquelle“ (`:7-10`).

## 4. Ist die Eintragsschranke ein Mutator im Sinn von R-313-4?

`eq-copilot/plugin/state/NakamaState.cpp`:

```cpp
2415  bool bestandsrevisionHeben (Zustand& z, juce::String& grund)
2416  {
2417      if (z.intentBestandRevision >= kRevisionMax)
2418      {
2419          grund = "intent revision would overflow";
2420          return false;
2421      }
2422      ++z.intentBestandRevision;
2423      return true;
2424  }
...
2444      auto treffer = std::find_if (z.sourceIntents.begin(), z.sourceIntents.end(),
2445          [&] (const SourceIntent& s) { return s.quelleId == quelleId && s.passageId == passageId; });
...
2467      if (treffer->rolle == rolle && treffer->herkunft == herkunft && treffer->konfidenz == konfidenz)
2468          return true;   // No-op: keine Revision, kein Dirty.
2469
2470      if (treffer->revision >= kRevisionMax)
2471      {
2472          grund = "intent revision would overflow"; return false;
2473      }
2474      if (! bestandsrevisionHeben (z, grund)) return false;
2475      treffer->rolle     = rolle;
2476      treffer->herkunft  = herkunft;
2477      treffer->konfidenz = konfidenz;
2478      ++treffer->revision;
2479      veraendert = true;
2480      return true;
```

Prozessor (`eq-copilot/plugin/src/prozessor/State.cpp:565-571`): `setzeIntent` liefert false → `return false` ohne Meldung; mit `veraendert` folgt genau ein `meldeHostDirty()`.

- Die Schranke `:2470-2473` vergleicht nur. Sie ist aber die Vorprüfung des Inkrements `++treffer->revision` (`:2478`) in derselben Funktion. Die Eintragsrevision liefert kein Aufrufer: `treffer` ist der gespeicherte Eintrag (`:2444-2445`), und `setzeIntent` hebt dessen persistente Revision je angenommener Änderung um genau eins, nach dem Bestandsschritt (`:2474`) und den drei Zuweisungen. `setzeIntent` ist damit der Mutator der persistenten Revision „Intent-Eintrag“ aus R-313-4 Satz 1, und die Schranke ist sein „prüfen vor jeder Nebenwirkung“ aus Satz 3. `assistentenrevisionHeben` (`:2886-2895`) hat für den Assistenten dieselbe Form.
- Der Schritt 2^53−2 → 2^53−1 ist nach Lesen ein wirklicher Produktweg: Bei einem Eintrag mit 2^53−2 und einem Bestand unter 2^53−1 lässt `:2470` ihn durch, `:2474` hebt den Bestand, `:2478` setzt 2^53−1, und der Prozessor meldet genau ein Host-Dirty. Steht der Bestand schon bei 2^53−1, scheitert der Schritt an `:2474`, nicht an der Eintragsschranke. Erreichbar ist ein solcher Eintrag nur über geladene Bytes: neue Einträge beginnen bei 1 (`:2454`), und R-313-4 sagt „kein Produktweg schreibt im Gebrauch Revisionen über 2^53−1; Stände darüber stammen aus fremden Bytes“ (`docs/beweise/NAK-313.md:147-149`). Der Leser nimmt einen Eintrag nahe am Rand unabhängig vom Bestand an (m81: Eintrag 2^53−1 mit Bestand 1 lädt schreibbar, `Sonde014IntentTest.cpp:1886-1887`).
- Daraus folgt: Der Schritt auf 2^53−1 gehört zur Zusage im Sinn von R-313-4 (Obergrenze 2^53−1 als gültiger Wert, Halt an der Grenze und nicht davor) und wörtlich zur Präzisierung `:17`. In der Matrixzeile M-81 steht er nicht. Nach Lesen verhält sich das Produkt wie B27 behauptet; es fehlt die Messung, nicht das Verhalten. Ein Mutant `if (treffer->revision >= kRevisionMax - 1)` an `:2470` überlebt nach Lesen alle Beine: kein Fall ändert einen Eintrag bei 2^53−2, und m81 bleibt grün, weil 2^53−1 weiter abgewiesen wird (nicht gelaufen).

## 5. Einordnung und kleinster Fix

**Vorlage** (`tools/dirigent/pruefauftrag-vorlage.md:33-41`; Prüfauftrag 5b `:14-22`): DEFEKT „verletzt einen Satz des Gate-Textes oben, eine Zeile der Verhaltensmatrix dieser Teiletappe, einen Test oder eine Invariante aus CLAUDE.md“; LÜCKE „Gate, Matrix und Entwurf sagen zu dem Fall nichts“; HÄRTUNG „wünschenswert, von keiner Zusage verlangt“. Ausgeschlossen sind `docs/**` und die Prüfliste als Anforderungsquelle; ein Sabotageszenario gegen den Runner ist HÄRTUNG.

**Was nicht bricht:** die Matrixzeile M-81 (erfüllt, eine Hälfte, MC), M-80 (zwei Hälften, MB und W80), die Tests (Kanon 70/70, `NAK-313.md:7360`) und die Invarianten aus CLAUDE.md. Nach Lesen verhält sich das Produkt wie behauptet. Es gibt also weder einen Produktfehler noch einen Matrixzeilenbruch; das Rohurteil sagt das selbst („kein nachgewiesener Produktfehler“, `…-urteil.md:7`).

**Was bricht:**

- `tools/beweise.ps1:773` ist ein Ticketpfad und liegt nicht unter `docs/**`.
- Prüffrage (h) fragt genau danach: „sagen die Behauptungszeilen B2, B7, B10, B27, B29, A4 und A5 in tools/beweise.ps1 nicht mehr, als die Fälle messen“ (Prüfauftrag `:26`). Für B27 lautet die Antwort nein.
- Der Satz geht als beglaubigte Zeile in den BEWEIS des Gates ein („vollem Kanon GRÜN“, Prüfauftrag `:9`; `NAK-313.md:7087`, „[OK] Exit 0“). Der Beweis bescheinigt damit einen Schritt, den kein Fall misst.
- Der Auftrag 5b verlangte „Behauptungen nicht weiter als die Messung“ (`:39`).

**Keine HÄRTUNG:** Es gibt kein Sabotageszenario (Vorlage `:40`). Eine wahre Behauptung verlangen (h) und Auftrag `:39`; sie ist nicht bloß wünschenswert.

**Keine LÜCKE im eingereichten Sinn:** Die Behauptung existiert und ist in ihrem Eintragsteil falsch. Die fehlende Messung darunter ist, allein an der Matrixzeile M-81 gemessen, eine Lücke dieser Zeile gegenüber R-313-4 Satz 3 (so gelesen in M-80) und gegenüber Präzisierung `:17` und Prüffrage (c).

**Kategorievorschlag: DEFEKT.** Der Defekt liegt im Beleg, nicht im Produkt. Er beruht auf Prüffrage (h) und der BEWEIS-Klausel des Gates, nicht auf einer Matrixzeile.

**Fix (a): Behauptung begrenzen.** In `tools/beweise.ps1:773` den 5b-Satz ersetzen durch (ASCII wie der Runner):

> Seit NAK-313 Etappe 5b (R-313-4; 313/M-80, M-81, M-89): die Revisionsraender der NAK-283-Faelle M-07 bis M-10 und M-12 stehen bei 2^53-1 (assistentenrevision_laeuft_nicht_ueber misst den neuen Rand); die Bestandsrevision geht bei 2^53-2 genau einen Schritt mit genau einem Host-Dirty, und Bestands- wie Eintragsrevision weisen bei 2^53-1 ab, bevor sie etwas zuweisen (Prozessor ohne Host-Dirty, Bibliothek mit Grund und gleichen Bytes); der Intentwriter schreibt bestand_revision 2^53-1 ab bestand_revision bytegleich zur Handinstanz intent-wire-v1.json.

Jeder Teil ist dann durch T1, T2, T3a, T4 bis T6 und T7 aus Abschnitt 1 gedeckt. Es gibt keinen Test und keinen Rotlauf. Der positive Eintragsschritt bleibt ungemessen und wäre als Lücke von M-81 zu benennen.

**Fix (b): positiven Eintragsfall ergänzen.**

- **Ort:** `m81()` (`Sonde014IntentTest.cpp:1874` ff.) vor dem Randblock, unter demselben Testnamen `313/M-81 eintragsrevision_haelt_an_der_grenze`. Der Prüftext folgt M-80: „bei 2^53-2 genau ein Schritt auf 2^53-1, Bestand +1, 1 Host-Dirty, Reload schreibbar“.
- **Einrichtung:** `baumMitEigenschaft ("source_intents_v1", juce::var (liste ({ kQuelleA, juce::String(), "fuehrt", juce::var (kRand - 1), "user", juce::var (1.0) })))` mit der Voreinstellung Bestand 1 (`:143-152`). Prozessor auf dem Heap (NAK-175), Dirty-Zähler nach dem Laden. Vorbedingung: schreibbar, ein Eintrag mit 2^53−2, Bestand 1.
- **Ereignis:** `setzeQuellenrolle (kQuelleA, {}, state::Rolle::traegt, state::IntentHerkunft::user, 1.0)`.
- **Messung:** Rückgabe true; Eintragsrevision 9007199254740991 mit Rolle `traegt`; Bestand 2; genau 1 Host-Dirty; `getStateInformation` in einen neuen Prozessor lädt schreibbar mit Eintragsrevision 9007199254740991. Bestand 1 trennt die Eintragsschranke von der Bestandsschranke: unter W80 und MC bleibt diese Prüfung grün.
- **Etikett:** Regressionswache. Am Basisstand `062a98fc` lag die Schranke bei `int64max`, und der Leser hatte keine Obergrenze (Rotbeweisspalten M-81 und M-80, `:1143`, `:1142`). Schritt und Reload waren dort grün, wie bei W80 für M-80.
- **Rotbeweis W81** an `NakamaState.cpp:2470`: `if (treffer->revision >= kRevisionMax)` → `if (treffer->revision >= kRevisionMax - 1)`. Kette nach R-313-11: Beim Eintrag 2^53−2 weist `setzeIntent` an `:2470-2473` ab, vor `:2474`. Der Prozessor gibt false zurück, bevor `meldeHostDirty` kommt (`State.cpp:565-571`). Ergebnis: Rückgabe false, Eintrag 2^53−2, Bestand 1, 0 Dirty. Die neue Prüfung fällt; die Randprüfungen von m81 bleiben grün.
- **Matrix und Beleg:** datierter Nachtrag zu M-81 (zweite Hälfte „Schritt auf 2^53−1“, Etikett Regressionswache, Rotbeweis W81, Quelle ergänzt um R-313-12); die Rohdatei zu M-81 bekommt die zweite Hälfte, dazu `NAK-313-e5b-mutation-W81.txt`. Der Wortlaut von B27 bleibt und ist danach gedeckt.

In beiden Fällen braucht es einen Kanon auf dem End-Stand, weil erst er die Beweiszeile `NAK-313.md:7087` erneuert. Der Auftrag 5b sagt: „Ein Commit am Code nach dem Kanon verlangt einen erneuten Kanon auf dem End-Stand (§29.7)“ (`:38`).

**Empfehlung: (b).**

- **Regel:** R-313-4 Satz 1 nennt den Intent-Eintrag mit der Obergrenze 2^53−1. Satz 3 („halten an der Grenze“) liest die Matrix in M-80, bei derselben Quelle, als zwei Hälften mit R-313-12. Die Präzisierung `:17` nennt den Schritt für M-80 bis M-83 und geht dem Bauplantext vor (`:14`). Prüffrage (c) fragt ihn für die Eintragsschranke ab. Die Lücke liegt in der Matrixzeile M-81, nicht in der Behauptung.
- **Matrix:** Jeder andere Rand der Teiletappe 5b hat seine Hälfte „letzter gültiger Wert“ mit Einzelbruch: die Leser L74b bis L78b, der Bestand W80, der Transaktionskern M83b und die Sender S88c und S88d (`:7262-7285`). M-81 ist die einzige Schranke ohne diese Hälfte, und nach Lesen überlebt der Einsschritt-Mutant an `:2470` den Kanon.
- **Muster im selben Ticket:** A-E2-1 (`:4161-4165`): Der Bauplan legte einen Weg fest, den die Matrixzeile nicht maß; B15 bekam in derselben Zeile zusätzliche Prüfungen mit Rotbeweis. A-E2-2 (`:4166-4173`): Eine Behauptung wurde gekürzt, wo kein bindender Text sie verlangte. Hier verlangt die Präzisierung `:17` den Schritt; es gilt also das Muster A-E2-1.
- **Wirkung:** Die Behauptung B27 wird so, wie sie dasteht, wahr. §31.8 C und E stimmen danach auch für die Eintragsschranke.

(a) ist der kleinere Textdiff. Er reicht, wenn der Dirigent festlegt, dass die Matrixzeile M-81 (nur Abweisung) das Maß ist und „(M-80 bis M-83)“ in `:17` die Zeilen gemeinsam zusammenfasst. Dann ist der positive Eintragsschritt eine benannte Lücke von M-81.

## 6. Nichtgeprüftes und Unsicheres

- Nichts wurde übersetzt, gefahren oder mutiert. Dass der Einsschritt-Mutant an `NakamaState.cpp:2470` alle Beine überlebt, ist aus der Suche in Abschnitt 2 und der Randprüfung von m81 gelesen, nicht gemessen.
- Nicht geprüft: die SONDE-014-Sätze von B27 vor „Seit NAK-313 Etappe 5b“ (seit `062a98fc` unverändert) und die Rust-Seite (A4) zur Eintragsrevision; B27 ist ein C++-Bein.
- Die Kanon-Rohausgabe `docs/beweise/roh/NAK-313-50cac87-dirty.md` ist nicht geöffnet. Grundlage sind die Kanonzeile `NAK-313.md:7087` und §31.5 und §31.6.
- Unsicher ist die Reichweite der Präzisierung `:17`: ob „(M-80 bis M-83)“ beide Hälften je Zeile verlangt oder die Zeilen gemeinsam zusammenfasst. Das entscheidet zwischen (a) und (b) und ist Sache des Dirigenten.
- Außerhalb des Befunds, ohne Einordnung: Den positiven Schritt der Assistentenrevision bei 2^53−2 misst NAK-283 M-12 in B27 (Bibliothek `1609-1622`, Host `1722-1752`). Keiner der 34 Läufe aus §31.4 bricht `assistentenrevisionHeben` um eins zu eng; an `:2888` läuft nur MA (`NAK-313.md:7271`). Weder B27 noch B29 behauptet diesen Schritt für 5b, eine Überdehnung liegt also nicht vor. Von Belang ist das nur, wenn der Dirigent mit (b) die positiven Hälften symmetrisch vervollständigt.
- Die Quellenangabe des Auftrags „§31.9 Selbstaudit (Satz „Behauptungen nicht weiter als die Messung“)“ trifft am HEAD nicht zu. Der Satz steht im Auftrag 5b `:39`; §31.8 E trägt „nennen nur Gemessenes“.

## URTEIL

URTEIL: PRÄZISIERT

Der Befund stimmt in allen Tatsachen. B27 (`tools/beweise.ps1:773`) behauptet für die Eintragsrevision einen Schritt bei 2^53−2 mit genau einem Host-Dirty, den kein Fall in B27, B2, B7 oder B29 misst. m80 richtet nur die Bestandsrevision knapp unter dem Rand ein; m81 misst die Eintragsrevision nur am Rand.

Präzisiert in vier Punkten:

1. Grundlage ist keine Matrixzeile. M-81 verlangt nur die Abweisung und ist mit MC belegt. Grundlage sind die ausdrücklich beauftragte Prüffrage (h) und die Beweiszeile des Kanons (`docs/beweise/NAK-313.md:7087`, „[OK] Exit 0“).
2. Die Ursache ist eine Lücke der Matrixzeile M-81 gegenüber R-313-4 Satz 3 (in M-80 als zwei Hälften gelesen) und gegenüber der Präzisierung des Dirigenten (Auftrag 5b `:17`, Prüffrage (c)).
3. Nach Lesen überlebt ein Einsschritt-Mutant der Eintragsschranke (`NakamaState.cpp:2470`, `>= kRevisionMax - 1`) den Kanon.
4. Dieselbe Überdehnung steht in §31.8 C und E (`docs/**`, Kontext).

Kein Produktfehler: Nach Lesen verhält sich das Produkt wie behauptet.

**Kategorievorschlag:** DEFEKT, im Beleg, nicht im Produkt.

**Kleinster Fix, empfohlen: (b).** In `m81()` einen positiven Fall mit Eintrag 2^53−2 und Bestand 1 ergänzen: Schritt auf 2^53−1, Bestand 2, genau 1 Host-Dirty, Reload schreibbar; Etikett Regressionswache; Rotbeweis W81 an `NakamaState.cpp:2470` mit `>= kRevisionMax - 1`; datierter Nachtrag zu M-81. B27 bleibt im Wortlaut.

**Alternative (a),** wenn der Dirigent M-81 als Maß festlegt: B27 nach dem Wortlautvorschlag in Abschnitt 5 begrenzen und den positiven Eintragsschritt als Lücke von M-81 benennen.

In beiden Fällen folgt ein Kanon auf dem End-Stand.

FERTIG Validierung 5b, PRÄZISIERT, Kategorievorschlag DEFEKT, Fix b
