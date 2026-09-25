# NAK-380 Wiederprüfung 2 der Etappe 4 — Urteil (Opus 5.5, Effort xhigh, lesender Ersatzprüfer; Codex zweimal 401)

Worker `c130868f` (`nakama-nak380-88e3a63-pruefw2`), 26.09.2026, 00:45:06 bis 00:47 Uhr, HEAD vor und nach dem Lauf `88e3a63e`; Auftrag `docs/beweise/roh/NAK-380-wiederpruefung-e4-2-auftrag.txt` (Vorlage B, unverändert übernommen). Codex-Läufe `nak380-e4-w2` (Thread 01a0dabb, 00:41:55 bis 00:42:28) und `nak380-e4-w2b` (00:44:03 bis 00:44:35) endeten mit „401 Unauthorized: Incorrect API key provided“ gegen das ChatGPT-Backend (auth.json trägt neben den ChatGPT-Tokens einen OPENAI_API_KEY; `codex login status`: Logged in using ChatGPT). Wortlaut des Prüfers unverändert:

---

URTEIL: PASS — alle Befunde geschlossen, nichts gebrochen.

Wiederprüfung 2, Etappe 4, NAK-380 — lesender Ersatzprüfer (Opus 5.5) für den ausgefallenen Codex-Lauf (401), gebundener Auftrag `docs/beweise/roh/NAK-380-wiederpruefung-e4-2-auftrag.txt` unverändert.

HEAD zu Beginn: 88e3a63e5eaa6954886245a01722832daf42a6fa
HEAD am Ende: 88e3a63e5eaa6954886245a01722832daf42a6fa (identisch; `git status --short` beide Male nur die fremden `briefing-hub/` und `nimbalyst-local/`)

## Geprüft

Fixdiff `git diff 46bd211c...HEAD -- eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp` vollständig (24 Zeilen, drei Hunks: Kopfkommentar des Beobachtungsläufers, zwei neue Prüfaufrufe mit Kennung, Moduskommentar in `main`); `git log 46bd211c..HEAD` = genau ein Commit 88e3a63e; `git diff --stat VORHER...HEAD -- eq-copilot/plugin/core eq-copilot/plugin/vertrag eq-copilot/plugin/src eq-copilot/fixtures eq-copilot/schemas broker tools` leer. Am ZIEL gelesen: gemeinsamer Läufer `AnalysisGoldenTestMain.cpp:1324-1381` (Schleife `:1344`, `aus->samples = strom` `:1376`, `aus->verworfen` `:1375`, `engineSamples` `:1378`), Prüfhelfer `:1387-1396` und `:1400-1406`, Beobachtungsläufer `:1514-1648`, Modusausgang `:1917-1930`, `pruefe`/`fehler` `:154-164`, Testzugang `verarbeitet` `:136`, Abstände `Nak380Pruefsignale.h:579-580`, `klicksEinsetzen` `Nak380Pruefsignale.h:597-603` (ändert die Pufferlänge nicht), Mutationsstelle n1-verlust `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:933-940`, Zähler `FeatureEngine.h:999`, `:1145` (Entnehmen setzt nur `ereignisAnzahl` zurück), `:756` (Rücksetzung nur in `zuruecksetzen`), JUCE `operator+ (const char*, const String&)` `juce_String.h:1407`. Anker am VORHER per `git show 46bd211c:…`: R1 `:1589-1590` war reine Ausgabe, R2 `:1914-1915` war der Kommentar „Einzige Pruefung … E-380-13“ mit `return`. Rohbelege m62-rest und n1-verlust als Kontext gelesen, Aussage „fällt an der neuen Prüfung“ an Test und Mutationsstelle nachgerechnet. Zeilenenden `git ls-files --eol`: `i/lf w/crlf` (Regel erfüllt). Messbereiche A32 (`tools/plan/gesundheit.py`) und A33 (`tools/plan/tidy.py`, `AUSGENOMMEN = ("tests/", …)`) nehmen `eq-copilot/plugin/tests/` aus; der Diff kann sie nicht bewegen.

## Nicht geprüft

Kein Bau, kein Testlauf, keine Mutation gefahren (lesender Auftrag); die Exitcodes 0/1 sind aus Code und Mutationswortlaut hergeleitet, die Laufzahlen (B5 312/0, 108/4 bzw. 112, drei Prüfungen je Modus) nur aus den Rohbelegen übernommen. docs/** (Manifest §43/§44, Matrixzeilen, Beobachtungsrohdatei) nur als Kontext; übriger Ticketbereich nach Auftrag nicht Prüfgegenstand.

## Frage 1 — Befunde

**R1 geschlossen: ja.** Der Beobachtungsläufer ruft `nak380SamplesGeprueft ("Beobachtung", kennung…, *lauf, x.size(), 1440000u, "30 s * 48 000, 2812 Bloecke + Rest 256")` als eigene Prüfung (`AnalysisGoldenTestMain.cpp:1559-1560`, Prüfzeile `:1390` verlangt Puffer = Läufer = Engine = 1 440 000; Herleitung 2812·512 + 256 = 1 440 000 stimmt), `pruefe` zählt `fehler` (`:163`), der Modus endet `return fehler == 0 ? 0 : 1;` (`:1929`), Kommentare berichtigt (`:1506-1509`, `:1914-1916`, `:1925-1928`); unter `m62-rest` (`:1344` → `while (strom + block <= samples)`) wird `lauf.samples = engineSamples = 1 439 744`, nur diese Prüfung fällt, Verlust (0) und Rosa (Puffer vor dem Lauf, `:1522`) bleiben grün — Rotbeweis an der neuen Prüfung.

**R2 geschlossen: ja.** Der Beobachtungsläufer ruft `nak380VerlustGeprueft ("Beobachtung", kennung…, lauf->verworfen)` als eigene Prüfung (`AnalysisGoldenTestMain.cpp:1561`, Prüfzeile `:1402` verlangt 0), der Exitcode folgt über `fehler` (`:1929`), keine Zusage zu Ereigniszahlen hinzugekommen (Zählung `:1566-1593` unverändert, nur Ausgabe); unter `n1-verlust` (`Spektrum.h:935-936` → jedes Ereignis `++zEreignisseVerworfen; return;`) steigt `ereignisseVerworfen()` (`FeatureEngine.h:999`, von `ereignisseEntnommen` `:1145` nicht zurückgesetzt) über 0, Samplezahl und Rosa bleiben grün — Rotbeweis an der neuen Prüfung.

## Frage 2 — Brüche

Keine. Kein Produktpfad im Diff (Stat leer); geändert sind nur `nak380BeobachtungKlickpaare` (nur aus `main` `:1920`/`:1922` erreichbar, `#if NAKAMA_FEATUREENGINE_TESTZUGANG`), Kommentare und der Moduskommentar; `nak380Detektor` ruft die Beobachtung nicht, die B5-Zusagefälle (Nullfall `:1426-1427`, Impulsfall `:1493-1494`, M-61, M-62) und beide Prüfhelfer sind unverändert; die neuen Prüfungen lesen nur `lauf`, verändern weder `ev` noch `fenster`, die Beobachtungszahlen bleiben Ausgabe ohne Zusage; `kennung` lebt bis Funktionsende, `toRawUTF8()` wird sofort verbraucht; keine Invariante aus CLAUDE.md berührt (Audio-Thread, Schemas, Identität unberührt; Engine weiter im Heap über den Läufer).

## Befundtabelle

| Befund | Status | Beleg am ZIEL |
|---|---|---|
| R1 Samplezahl im Beobachtungslauf | geschlossen | `AnalysisGoldenTestMain.cpp:1559-1560`, `:1390`, `:1929`; Mutation `:1344` |
| R2 Ringverlust im Beobachtungsmodus | geschlossen | `AnalysisGoldenTestMain.cpp:1561`, `:1402`, `:1929`; Mutation `Spektrum.h:935-936` |

FERTIG Wiederprüfung 2 Etappe 4, PASS, 0 offene Befunde, 0 Brüche
