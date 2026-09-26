# NAK-380 Wiederprüfung 1 der Etappe 6 — Urteil (Opus 5.5, Effort xhigh, lesender Ersatzprüfer; Codex-Woche 95 %)

Worker `4c45309f` (`nakama-nak380-9185b2c-pruefw1e6`, `dontAsk`, Werkzeuge Read, Grep, Glob, Bash), 26.09.2026, gestartet zwischen 12:27:15 und 12:29:26 Uhr (Messungen des Dirigenten), beendet 12:34 Uhr (Sitzungsdauer 5 min 48 s laut `claude logs`); HEAD vor und nach dem Lauf `9185b2c8`, `git status --short` beide Male nur die fremden Einträge `briefing-hub/`, `nimbalyst-local/`. Zeigerauftrag auf `docs/beweise/roh/NAK-380-wiederpruefung-e6-1-auftrag.txt` (Vorlage B, unverändert). Ersatzprüfer statt Codex `gpt-6-astra`, weil die Codex-Woche um 12:27 Uhr bei 95 % stand (Skill §3.4, §5; Statuszeilen-Cache `CodexSevenUsed` 95). Wortlaut des Prüfers unverändert (aus dem Sitzungsprotokoll `4c45309f-6701-407b-81bc-1f052b2ed29f.jsonl`, letzter Assistententext):

---
URTEIL: PASS — alle Befunde geschlossen, nichts gebrochen.

**Geprüft.** VORHER ist `e744f40a0adf7dfa016146947856f28e01419033`; die Merge-Basis mit HEAD ist derselbe Stand. ZIEL ist HEAD.
- **Fixdiff:** `git diff e744f40a...HEAD` über die fünf Pfade (+360/−47) habe ich vollständig gelesen. `git show --stat cda00953` ohne `docs` enthält genau diese fünf Pfade.
- **Produktumfang laut Stat-Kommando:** Es zeigt `FeatureEngine.h` (3 Zeilen, nur `//`) und `telemetrie.rs` (nur `mod tests`). Dazu kommt, anders als erwartet, `tools/plan/gesundheit.py` (+7). Das ist die Pflegeticket-Zuordnung NAK-413 aus dem Dirigenten-Commit `9ccfdf84`, nicht aus dem Nacharbeits-Commit. Die Datei ist kein Produktcode und ändert kein Plugin- oder Brokerverhalten.
- **Ohne Diff:** `Spektrum.h`, `eq-copilot/plugin/src` (darin `AnalyseEngine.cpp`, `Diagnose.cpp`), `vergleichbarkeit.rs`, `broker/src/coordinator`.
- **Gegen den Code gelesen:**
  - `FeatureEngine::vorbereiten` (FeatureEngine.h:640-747; bei gleicher Rate ohne Wirkung, :653-654), `zuruecksetzen` (:752-845), `Stufe::vorbereiten` und `leeren` (:1352-1400).
  - `schiebeStufe` (Spektrum.h:331-412; der Fensteranfang wandert je Fenster um einen Hop) und `detektorLeeren` (Spektrum.h:921-934).
  - `AnalyseEngine::vorbereiten`, `zuruecksetzen`, `auswertenLeicht` (AnalyseEngine.cpp:174-362, :876-887). `auswertenLeicht` schreibt nur den Snapshot, `fuelleBasis` ist const. `AnalyseEngine` hat keinen Verlust- oder Dropzähler (git grep leer).
  - `beurteile_mit_messfassung` (vergleichbarkeit.rs:363-441, None-Zweig :381), `kSignal` und `kSelbstpruefung` (Nak380Pruefsignale.h:916-996), die Dropzähler des Prozessors (PluginProcessor.h:210-215, :237).
- **Zählgrößen nachgerechnet:**
  - FeatureEngine 96 kHz: 702 Fenster, Rest 4 608 (Haupt) und 174, Rest 29 184 (Bass); mit alter Länge 1 405 und 350.
  - B15: 1 440 000 = 2 812·512 + 256 und 1 323 000 = 2 583·512 + 504, also 2 813 und 2 584 Blöcke, verarbeitet 1 439 744 und 1 322 496.
  - M-110: 960 000 = 1 875·512, verarbeitet 959 488.
- **Belege:** Alle 24 Mutationsprotokolle e6n1 gelesen (Stelle, Wortlaut, gefallene Prüfzeile, Rücknahme-SHA). Zeilenenden: vier Dateien i/lf w/crlf, `ShotTestMain.cpp` i/lf w/lf.
- **Frage 2, nichts gebrochen:**
  - Übrige Aufrufer von `nak380Korpuslauf` (M-50 bis M-62, Beobachtung, M-115): Die Vorgabe `weiter = nullptr` ergibt wie bisher eine frische Engine (AnalysisGoldenTestMain.cpp:1565).
  - `nak380KFahren` (M-103 bis M-105, M-107) ist unverändert. Der Testzugang wurde nur erweitert, unter demselben Define.
  - Die geteilte M1-Zeile verliert keine Prüfung, das Histogramm steht jetzt in einer eigenen Zeile.
  - Die Referenz `e` bleibt nach `std::move (lauf->engine)` gültig und wird danach nicht mehr benutzt.
  - Gate-Sätze, CLAUDE.md-Invarianten und die Zusagen aus §54.7 und §56.5 berührt der Diff nicht, weil außer einem Kommentar kein Produktcode drin ist.

**Nicht geprüft.** Nach Auftrag habe ich nur gelesen: kein Bau, kein Testlauf, kein cargo, kein FL Studio. Die Grünläufe B5 384/0, A2 405/0, B15 12/0, A4 850/0, A1, A16, B1, B4 und `sizeof (FeatureEngine)` = 16 248 B stammen aus §59.5 und sind nicht nachgefahren. Dass `sizeof` gleich bleibt, folgt hier nur daraus, dass der Diff an `FeatureEngine.h` ein reiner Kommentar ist. Die Mutationsläufe habe ich als Protokolle gelesen, nicht wiederholt. Nicht Prüfgegenstand waren der übrige Bereich der Etappe (Erstprüfung 6) und `docs/**` außer den Rohbelegen.

**D1 geschlossen:**
- **Signal:** AnalysisGoldenTestMain.cpp:2467-2469 erzeugt P2 bei 96 kHz mit 2 880 000 Samples und prüft es mit `rosaSelbstpruefung`.
- **FeatureEngine:** Dieselbe Engine läuft 30 s weiter (:2533 über `weiter`, :1555/:1565), mit Zähl- und Verlustprüfung (:2534-2536). Die Fensterprüfung :2539-2563 rechnet ihr Soll im Test: W = floor((S−N)/Hop)+1, Rest, Fensteranfang + Rest = S, neue Länge und Hop, verarbeitet = 2 880 000.
- **M1:** 30 s Speisung (:2653), Zählprüfung (:2654), Band 64 mit im Test gerechnetem Soll 174 (:2657-2664).
- **Rotbeweise:** `m120-fe-weiter` (FeatureEngine.h:660, 1 405/350 Fenster) und `m120-m1-weiter` (AnalyseEngine.cpp:228, 350 Segmente) fallen je an ihrer Weiterbetriebszeile.

**D2 geschlossen:** ShotTestMain.cpp:2547-2558 erzeugt K4 mit `kSignal (45, fs, 30·fs)` und prüft es mit `kSelbstpruefung` bei 48 und 44,1 kHz. Der Erzeuger nutzt die P2-Saat, −20 dBFS und +15 dB (Nak380Pruefsignale.h:916-938). „Keine Karte“ ist eine eigene Zusagezeile (:2656-2660), der Leertext mit 51 bzw. 47 Hz ebenso (:2661-2663). `m106-ton60` fällt bei beiden Raten an :2656 (Knopf NEIN, 0 Meldungen), `m106-satz` bei beiden Raten an :2661.

**D3 geschlossen:** Der Speiser gibt den Rest über `std::min` als kurzen letzten Block (ShotTestMain.cpp:2565-2580, Sicht auf dieselben Kanalpuffer :2575). Das Soll kommt aus der Signallänge (:2606-2608). Drei eigene Prüfungen folgen: gespeist, Blockzahl und letzter Block (:2609-2614), Drops (:2615-2621), verarbeitet = gespeist − letzter Block (:2622-2626). `m106-rest` fällt je Rate an gespeist und verarbeitet, `m106-drops` je Rate an der Dropzeile.

**D4 geschlossen:** Die Herleitung mit Ein-Block-Quarantäne und der Toleranz von `verbraucht` steht in AnalysisGoldenTestMain.cpp:2674-2689 und :2738. Gezählt wird an :2736, das Soll wird an :2749-2751 gerechnet. Eigene Prüfungen: gespeist 960 000 in 1 875 Blöcken (:2752-2756), Drops 0 (:2757-2765), verarbeitet 959 488 (:2766-2771). `m110-letzter-block` fällt an gespeist und verarbeitet, während die NaN-Zusage grün bleibt; `m110-drops` fällt an der Dropzeile.

**D5 geschlossen:** telemetrie.rs:1364-1378 ruft `beurteile_mit_messfassung` direkt mit `None` auf Seite a, Seite b und beiden Seiten auf. Je Seite gibt es eine Prüfung für Klasse und Grund und eine eigene für NaN. `m122-unbekannt-nan` (vergleichbarkeit.rs:381, NaN → 0,0) fällt nur an den drei NaN-Zeilen (:1375). Das ist schlüssig, weil der Grund `MessfassungUnbekannt` über :410-411 und :432-440 unverändert Unvergleichbar ergibt.

**D6 geschlossen:** GoldenTestMain.cpp:1270-1274 prüft gespeist = verarbeitet = 1 440 000 und 61 Snapshots. `a2-kfahren-verarbeitet` und `a2-kfahren-rest` fallen an dieser Zeile.

**D7 geschlossen:** GoldenTestMain.cpp:1289-1293 prüft vor der Kartenprüfung dieselbe Bedingung wie die M-107-Gegenprobe (:1332). Dieselben zwei Mutationen fallen an dieser Zeile.

**D8 geschlossen:**
- **Verlust je FeatureEngine-Lauf:** AnalysisGoldenTestMain.cpp:2474 und :2536.
- **M1-Zählprüfung vor und nach dem Wechsel:** :2607-2617 und :2654. Einen Dropzähler der M1 gibt es nicht; verarbeitet = gespeist ist die vollständige Verlustaussage.
- **Vorframe:** :2512-2520, 1 530 Werte, jeder auf P0, mit Vorbedingung „vorher belegt“ (:2480).
- **LRA-Histogramm:** :2523-2530, Bins und Zähler, Vorbedingung :2481-2482.
- **M1-Histogramme:** eigene Zeile :2642-2646.
- **Rotbeweise:** Die Doppelmutationen `m120-vorframe-doppel` (FeatureEngine.h:729, :735, :830) und `m120-lra-doppel` (:680, :808) fallen an ihren Zeilen, ihre je zwei Gegenproben bleiben grün. `m120-lra-zaehler` (:809) fällt am Zähler. `m120-verlust` fällt als Doppelmutation, die Gegenprobe `m120-verlust-allein` bleibt grün. `m120-m1-hist` (AnalyseEngine.cpp:313) und `m120-speisung` fallen je an ihrer Zeile.

**D9 geschlossen:** FeatureEngine.h:725-728 nennt für 96 kHz jetzt 1532, 115, 232 mit R-380-14 (i). Der Diff enthält nur `//`-Zeilen, kein Verhalten.

HEAD zu Beginn: `9185b2c81632ad321872cfcd886deaa64bfe454f`
HEAD am Ende: `9185b2c81632ad321872cfcd886deaa64bfe454f` (identisch; `git status --short` unverändert, nur die fremden Ordner `briefing-hub/` und `nimbalyst-local/`)

FERTIG Wiederprüfung 1 Etappe 6, PASS, 0 offene Befunde, 0 Brüche
