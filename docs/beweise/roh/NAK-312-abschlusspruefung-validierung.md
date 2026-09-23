# NAK-312 Abschlussprüfung (S25k) — Quellvalidierung des Befunds [P2], 23.09.2026

HEAD vorher `225784b79044f01433c897e9820efbc61cf24abd`, HEAD nachher `225784b79044f01433c897e9820efbc61cf24abd`. Prüfstand `e8c6f079f4331755f4a901c2715ce6df2324420a`; `git diff --stat e8c6f079 HEAD -- eq-copilot` leer, der Arbeitsbaum von `eq-copilot/plugin/tests/TransactionTestMain.cpp` gleich `e8c6f079` (leerer Diff). Frischer, lesender Opus-5.5-Thread, Effort max; nur lesende git- und Lesewerkzeuge, kein Compiler, kein Test, kein FL Studio, kein Python. Zeilen ohne Dateinamen: `TransactionTestMain.cpp` am Prüfstand.

## 1. Quelle

- `:4469` `struct Blockstand { const char* name; param::DspSatz satz; };`
- `:4471` `std::array<Blockstand, 2> blockStaende()` gibt als Wert zurück (prvalue); `:4473-4475` baut `mitEq (true)` und den Stand mit Band (Slot 0, Bell 1 kHz +6 dB) und gibt `{ { { "neutral", mitEq (true) }, { "Band", mitBand } } }` zurück.
- `param::DspSatz` (`eq-copilot/plugin/state/NakamaParameter.h:151-158`): `Satz werte` (`std::array<Zelle, kAnzahl>`, `:121`) und `std::vector<Schutzzone> zonen` (`:154`). Der Destruktor ist nicht trivial, also auch der von `std::array<Blockstand, 2>`.
- `:4655` wörtlich: `const auto& mitBand = blockStaende()[1].satz;`; `:4656` `auto a = blockPruefling (blk, mitBand);`; `:4657` `auto c = blockPruefling (blk, mitBand);`
- `:4442` `std::unique_ptr<Prozessor> blockPruefling (int groesse, const param::DspSatz& z)` nimmt eine **Referenz** und kopiert beim Aufruf nicht; `:4444` `prozessor (48000.0, groesse)`, `:4445` `setze (*p, z);`.
- `:1822` `tx::Ergebnis setze (Prozessor& p, const param::DspSatz& z)`; gelesen wird in `:1828` `a.satz = z;` (Kopierzuweisung von `werte` und `zonen`).
- `fahreFolge` (`:4401-4427`) nimmt den Prozessor, nicht den Satz; `:4661-4662` lesen `mitBand` nicht mehr.
- `:4610` `for (const auto& st : blockStaende())`, der Hauptfall 312/M-20 in `mengeLauf` (`:4606-4623`).
- Herkunft: `git blame` ordnet `:4442-4476`, `:4610` und `:4639-4677` dem Commit `ad18d984` zu („NAK-312 Etappe 3b: Blockbindung der Hostwerte …", 21.09.2026). BASIS `090d0fa1` ist sein Vorfahr, er ist Vorfahr von `e8c6f079`. An BASIS gibt es `blockStaende` nicht (`git grep` leer): die Stelle ist neu im Ticketdiff.
- Lauf: `main` ruft `nak312Blockbindung()` unbedingt (`:6680`). Der Teilfall-Block `:4643-4677` steht außerhalb jedes `#if`; nur `:4658-4660` und `:4667-4671` hängen an `NAK312_GEGENPROBE_OHNE_BLOCKRAND`. Er läuft also im regulären B7-Lauf und ebenso im Gegenprobebau.
- Sprachstand: `eq-copilot/CMakeLists.txt:24` `set(CMAKE_CXX_STANDARD 20)`.

## 2. Ablauf

Die Normen stammen aus C++20 (N4861), gelesen unter timsong-cpp.github.io/cppwp/n4861.

1. `blockStaende()` ist ein prvalue vom Klassentyp. `blockStaende()[1]` ist kein eingebauter Indexzugriff: nach [over.sub] bedeutet `x[y]` bei einem Klassenoperanden `x.operator[](y)`, also einen Funktionsaufruf. Für den Aufruf wird das Temporary materialisiert ([class.temporary]/2, „when performing member access on a class prvalue"). `operator[]` liefert `Blockstand&` auf Element 1 des Temporaries. `.satz` ist ein lvalue-Unterobjekt, und `const auto&` bindet direkt daran.
2. [class.temporary]/6 verlängert die Lebensdauer nur, wenn der glvalue auf einem dieser Wege entsteht: Temporary-Materialisierung, „subscripting ([expr.sub]) of an array operand", „class member access … using the . operator where the left operand is one of these expressions", Casts, Bedingungs- oder Kommaausdruck. Der linke Operand von `.satz` ist hier der Funktionsaufruf `operator[](1)` und damit keiner dieser Wege; `std::array` ist kein Array-Typ nach [expr.sub]. **Es gibt keine Verlängerung.**
3. Temporaries werden „as the last step in evaluating the full-expression" zerstört ([class.temporary]/4), und der volle Ausdruck ist der init-declarator ([intro.execution]/5). Ab `:4656` verweist `mitBand` auf ein Objekt, dessen Lebensdauer mit dem Destruktoraufruf geendet hat ([basic.life]/1.4).
4. Beide Aufrufe `:4656` und `:4657` reichen die Referenz durch `blockPruefling` (`:4442`) an `setze` (`:4445`). Dort kopiert `:1828` aus dem zerstörten Objekt: „the glvalue is used to access the object", also **undefiniertes Verhalten** nach [basic.life]/7.1. Die Kopie in `setze` hilft nicht, denn ihre Quelle ist in beiden Aufrufen schon tot.
5. Der Ablauf des Prüfers ist am Code reproduzierbar, statisch:
   `git show e8c6f079:eq-copilot/plugin/tests/TransactionTestMain.cpp | sed -n '1822,1830p;4442,4448p;4469,4476p;4655,4657p'`.
   Ein dynamischer Nachweis wäre möglich, ist aber nicht gefahren, weil der Auftrag keinen Compiler zulässt: MSVC-AddressSanitizer meldet Zugriffe in abgelaufene Temporaries als `stack-use-after-scope` (Microsoft Learn, „Error: stack-use-after-scope", Example 4 – temporaries, Bau mit `/fsanitize=address /Zi /Od`).
6. Die Messungen widersprechen dem nicht. Im Kanon an `3d4a5a8e` ist der Teilfall grün (`docs/beweise/roh/NAK-312-3d4a5a8-dirty.md:6064`, „Blockrandziele 12"). Der Rotbeweis Teil b fiel ohne die Endlichkeitsregel und war nach der Rücknahme wieder grün (`docs/beweise/roh/NAK-312-rot-M-20.txt:126-133`, `:170-176`). Der MSVC-Release-Bau hat also bisher den gemeinten Satz gelesen, garantiert ist das nicht. Ob der Stapelplatz wiederverwendet wird, hängt von Compiler, Optimierung und Nachbarcode ab. Dann bauen `a` und `c` aus fremden Bytes; im schlimmeren Fall liest `zonen = z.zonen` fremde Zeiger. Der Teilfall kann dann rot werden oder grün bleiben, ohne den Stand mit Band zu fahren. Nicht gemessen.
7. Für `:4610` bestätigt: nach [stmt.ranged]/1 entspricht die Schleife `auto &&__range = blockStaende();`. Der prvalue wird durch Materialisierung direkt an `__range` gebunden und lebt nach [class.temporary]/6 (erster Weg) über die ganze Schleife. Der Hauptfall 312/M-20 ist fehlerfrei.
8. Weitere Stellen: im Ticketdiff `git diff 090d0fa1...e8c6f079 -- eq-copilot/plugin/tests/` (10 Dateien) habe ich die hinzugefügten Zeilen durchsucht. Gesucht waren Referenzbindungen an `f(…)[…]`, `f(…).g(…)` und `*f(…)`, dazu `string_view`, `StringRef` und `std::span` aus Temporaries, Zeiger aus `f(…).c_str()` oder `data()` sowie Range-for über `f(…)[…]` oder `f(…).g(…)`. **Einziger Treffer: `:4655`.**
   - Alle 30 Treffer der Suche nach Referenzdeklarationen mit Initialisierer habe ich einzeln gelesen. Sie binden an lvalues (`q.back()`, `p->dspKernFuerTest()`, `hostParam (…)`, `assistent` usw.) oder direkt an einen prvalue, der dann verlängert wird.
   - Die 25 Range-for-Schleifen mit Aufruf im Bereichsausdruck (14 Formen) sind unbedenklich: `…holeZustandKopie().mainProjectMitglieder` ist Memberzugriff auf einen prvalue und wird verlängert; `juce::StringArray::fromLines (…)` ist ein direkt gebundener prvalue.

## 3. Zusagenlage

- **Gate** (`docs/beweise/NAK-312.md:6`, laut Prüfauftrag gleich `docs/plan/plan.json` S25k): „BEWEIS: docs/beweise/NAK-312.md mit Matrix, Rotbeweisen an B2, B6, B7, B14, B15, RT-Wache vor Parameterverarbeitung, vollem Kanon GRÜN und Laufzeit-Arm."
  - Der Satz trägt den Fall nur allgemein. Der Teilfall ist ein B7-Rotbeweis dieses BEWEISES (§21.6, Zeile M-20: „Endlichkeitsregel entfernt → Teilfall rot (Teil b)").
  - Der Satz nennt den Teilfall nicht, und die übrigen B7-Rotbeweise bleiben unberührt.
- **Matrixzeile M-20** (§6, `:792`): „Für jeden der VIER abgedeckten Parameter ist der Ausgang von A über den ganzen Lauf BITGLEICH zu B (0 abweichende Samples); der wirksame Block hängt am Blockrand, nicht an der Wanduhr. Über andere Hostparameter sagt die Zeile nichts (M-79 bis M-81)".
  - Die Zeile sagt nichts zu NaN oder Inf. Laut §21.8 A-28 hat der Teilfall „ohne eigene Matrixzeile"; er steht nur im Bauerbericht §21.6, der laut Prüfauftrag Kontext ist.
  - Der Test der Zusage (`:4610`) ist fehlerfrei. **M-20 schweigt zum Teilfall.**
- **B7-Behauptung** (`tools/beweise.ps1:728`): „… ohne Ereignis setzt der Blockrand kein Ziel, NaN und +-Inf erreichen ihn als bestaetigter Wert, …".
  - Vollständig misst diesen Halbsatz im Kanon nur der Teilfall `:4643-4677`: alle vier Parameter, je NaN, +Inf und −Inf, 12 Ziele.
  - **R-312-34** (§2, `:203-207`, Wortlaut von §56.3): „Eine Behauptungszeile in `tools/beweise.ps1` trägt nur, was das Bein im Kanonlauf misst." Die Regel betrifft die Reichweite der Behauptung, nicht die Gültigkeit des Tests. Sie trägt den Fall deshalb nur mittelbar: der Halbsatz steht auf einer Messung, deren Ergebnis die Sprache nicht festlegt.
  - Teilweise zweite Wache: 312/M-18 (`:4261-4301`, NaN auf Output-Trim, +Inf auf Input-Trim, −Inf auf Width) fiel unter derselben Mutation Teil b mit (Rohdatei `:131`).
- **Prüfauftrag, DEFEKT** (`docs/beweise/roh/NAK-312-abschlusspruefung-auftrag.txt:15`): „DEFEKT: verletzt einen Satz des Gate-Textes oben, eine Zeile der Verhaltensmatrix (§6, §46), eine Zusage der Abnahmen U48, U49, U51, U56, U58, einen Test oder eine Invariante aus CLAUDE.md (…). Reproduzierbar, mit Kommando; je Defekt: Datei:Zeile und der Satz, der bricht."
  - Der Test `312/M-20 Teilfall nicht_endliche_hostwerte_am_blockrand` liegt im Prüfbereich (Ticketpfad, neu in `ad18d984`) und ist selbst fehlerhaft; so lese ich „einen Test" am tragfähigsten.
  - Der Ausschluss „theoretische Randfälle ohne Zusage" greift nicht: das undefinierte Verhalten tritt in jedem Lauf ein, nicht an einem Randwert.
  - Formlücke des Rohbefunds: er nennt keinen brechenden Satz und kein Kommando („statisch festgestellt"). Beides liefern §2.5 und dieser Abschnitt.
- **CLAUDE.md**: „NaN-Ehrlichkeit. Nicht-endliche Werte werden verriegelt und gezählt; …" ist eine Produktinvariante.
  - Der Befund richtet sich nicht gegen Produktcode; der Test ist ihr Beleg am Blockrand. Die Invariantenliste des Prüfauftrags führt NaN-Ehrlichkeit nicht.
  - „Fortschritt erst nach Beleg" ist eine Arbeitsregel, keine Zusage des Tickets.

Ergebnis: **Die Matrix schweigt.** Getragen wird der Fall vom Defektkriterium „einen Test", zusammen mit dem BEWEIS-Satz des Gates („Rotbeweisen an … B7", ausgeprägt in §21.6). Mittelbar trägt ihn R-312-34 über den B7-Halbsatz „NaN und +-Inf erreichen ihn als bestaetigter Wert".

## 4. Urteil

**BESTÄTIGT.** Jede Tatsachenaussage des Rohbefunds hält an der Quelle:
- `blockStaende()` liefert ein temporäres `std::array` (`:4471`).
- Der überladene `operator[]` ist ein Funktionsaufruf und verlängert nichts.
- Das Array wird am Ende von `:4655` zerstört.
- Beide Aufrufe `:4656-4657` lesen es danach über `setze` (`:1828`).
- Der Teilfall aus `ad18d984` hat damit undefiniertes Verhalten.

Ergänzt, nicht korrigiert: gelesen wird in `setze`; der Hauptfall 312/M-20 (`:4610`) ist nicht betroffen; der Block läuft auch im Gegenprobebau; die gemessenen Läufe verhielten sich wie gewollt; 312/M-18 bewacht die Endlichkeitsregel teilweise mit.

**Klassenvorschlag: DEFEKT.** Ein Test im Prüfbereich hat undefiniertes Verhalten und trägt seinen Rotbeweis und den B7-Halbsatz nicht belastbar. Die Einordnung trifft der Dirigent.

**Kleinster Fix** (eine Zeile, `:4655`): `const auto mitBand = blockStaende()[1].satz;` kopiert statt zu referenzieren (ein `DspSatz` mit 120 Zellen und leerer Zonenliste). Gleichwertig: `const auto staende = blockStaende();` und danach `const auto& mitBand = staende[1].satz;`.
- Beide Wege geben `a` und `c` genau den gemeinten Satz.
- Unverändert bleiben die Erwartung (Ausgang endlich, 0 abweichend, 12 Blockrandziele), der Behauptungstext im Test (`:4672-4676`) und die B7-Behauptung (`tools/beweise.ps1:728`).
- Das Ergebnis bleibt voraussichtlich grün; gemessen ist das nicht.
- Rotbeweis Teil b danach einmal wiederholen, damit er auf definiertem Verhalten steht.

| Befund | Urteil | Klassenvorschlag | tragender Satz |
|---|---|---|---|
| [P2] `TransactionTestMain.cpp:4655-4657`: die Referenz auf `blockStaende()[1].satz` überlebt das Temporary nicht; undefiniertes Verhalten im Teilfall `nicht_endliche_hostwerte_am_blockrand` | BESTÄTIGT | DEFEKT | Prüfauftrag, DEFEKT „verletzt … einen Test"; Gate „BEWEIS: … Rotbeweisen an B2, B6, B7 …" (§21.6 M-20, Teil b); mittelbar R-312-34 über die B7-Behauptung „NaN und +-Inf erreichen ihn als bestaetigter Wert"; Matrix M-20 (§6) schweigt |

FERTIG Validierung Abschlussprüfung NAK-312, 1 bestätigt, 0 präzisiert, 0 widerlegt
