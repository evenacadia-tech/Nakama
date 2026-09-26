# NAK-380 Abschlussprüfung T2 — Urteil (Opus 5.5, Effort xhigh, lesender Ersatzprüfer; Codex-Woche 95 %)

Worker `d97c0d70` (`nakama-nak380-3e5d366-prueft2`, `dontAsk`, Werkzeuge Read, Grep, Glob, Bash), 26.09.2026, gestartet 12:50 Uhr (Kanon-Start 12:49:40, Workerstart unmittelbar danach), beendet 13:15 Uhr (Beobachter „beendet“ 13:15:29); HEAD vor und nach dem Lauf `3e5d3660`, Arbeitsbaum außer dem bewegten Broker-Pin des parallelen Kanons und den fremden Einträgen `briefing-hub/`, `nimbalyst-local/` unverändert. Zeigerauftrag auf `docs/beweise/roh/NAK-380-abschlusspruefung-t2-auftrag.txt` (Vorlage A, unverändert; Prüfbereich `c58a2714...3e5d3660` über die 57 Pfade des Ticketdiffs). Ersatzprüfer statt Codex `gpt-6-astra`, weil die Codex-Woche bei 95 % stand (Skill §3.4, §5). Wortlaut des Prüfers unverändert (aus dem Sitzungsprotokoll `d97c0d70-7ef6-45e3-a33f-1701e6ac5cd2.jsonl`, letzter Assistententext):

---
URTEIL: PASS — kein DEFEKT (Lücken und Härtungen dürfen benannt sein).

**Geprüft.** Ich habe den Drei-Punkte-Diff `c58a2714…HEAD` über die 57 Pfade gelesen. Vollständig gelesen: Broker (`hypothese/*`, `maskierung.rs`, `vergleichbarkeit.rs`, `invalidierung*.rs`, `experiment*.rs`, `telemetrie.rs`, `prepost.rs`), Plugin-Kern (`FeatureEngine.h`, `Frame.h`, `Spektrum.h`, `Stereo.h`, `Lautheit.h`, `Vertrag.h`, `Zeit.h`), `AnalyseEngine.*`, `Diagnose.*`, `DiagnoseAntwort.cpp`, `PluginEditor.cpp`, `Ipc.cpp`, `CMakeLists.txt`, die Vertragsdateien, beide generierten Dateien und `metriken-v1.json`. Bei `beweise.ps1` ändern sich nur Behauptungstexte, `pruefe_v3_vertrag.py` ist rein additiv. Die Tests habe ich je Prüffrage gezielt gelesen: Läufer, Schranken, Vorbedingungen, Laufwahl ohne Filter. Alle entfernten Testzeilen habe ich auf Lockerung geprüft. Ergebnis: nur Umhüllung in Filterabschnitte und die M-113-Schranke laut Matrix. Stichprobe Rotbeweise: M-09, M-21, M-24, M-50, M-73, M-98, M-106, M-122. Gelesen habe ich Matrix §6 und die Fassungen §28.1, §31.2, §36.2–3, §38.2, §39.2, §41.2, §47.2–3, §49.2, §50.2, §55.2 und §58.2. Selbst nachgerechnet: das A/B-Beispiel mit Gewicht 1 (≈ 0,0066), Detektor-K = 1530 bei 48 kHz, die Sperrzeit bei 44,1/48/88,2/96 kHz, die Livegruppengrenze gegen `trennIndex` (Band 65 = Anfang Gruppe 19, keine Gruppe überspannt die Stufengrenze), die Querliterale der Bandbreitentafel und den Kommentarabgleich `.fbs` gegen die generierten Dateien. Verwendet habe ich nur lesende git-Kommandos und Lesewerkzeuge. Im Repo habe ich nichts angelegt oder geändert. Außerhalb des Repos habe ich eine Lesekopie des Matrixabschnitts im Job-Temp-Ordner angelegt (`C:\Users\phili\.claude\jobs\d97c0d70\tmp\matrix6.txt`).

**Nicht geprüft.** Ich habe nichts gebaut oder gefahren: keinen Test, keinen Kanon, keinen A9-Lauf mit `flatc`. Grün-Aussagen zu Beinen stammen aus dem Kanon an `f893c57e` und Manifest §59.5, nicht von mir. Seit `f893c57e` ändern sich außerhalb `docs/` nur Tests, ein Kommentar und der Broker-Pin (gemessen). Nicht jede der 122 Matrixzeilen habe ich einzeln gegen ihren Test gelesen. Der Schwerpunkt lag auf den Gate-Sätzen und den Bezügen zwischen den Etappen. Die Normzitate (EBU Tech 3342 §3.1, SuperFlux) habe ich aus Gate und Matrix übernommen und nicht selbst nachgeschlagen. `codex-bau.ps1` und `gesundheit.py` habe ich nur als Dirigentenwerkzeug angesehen. Abschlusskanon und Laufzeit-Arm gehören dem Dirigenten.

**(1) DSP-17.**
- **`bandpassung`** integriert `10^(P50/10)·breite(index)`: `broker/src/coordinator/hypothese/zusammenhang.rs:201`, Funktion `:193-215`. Die Breiten kommen aus dem eingefrorenen Gitter per `include_str!` und `OnceLock`: `bandbreite.rs:11-39`.
- **Screening** erbt die Rechnung über `bandpassung`: `screening.rs:148`.
- **Confidence:** `befund.rs:321` (`score = rang.rang()`), `bandpassung` ist Rangkomponente (`zusammenhang.rs:176`).
- **Maskierungspegel:** `maskierung.rs:142`.
- **Keine weiteren Leistungsleser:** Außer diesen Stellen und dem Verhältnis je Band in `prepost_verdrahtung.rs:234` wandelt kein Broker-Leser Feinbanddichten in Leistung.
- **Livegruppen:** Σ Dichte·(Kante(b+1)−Kante(b)) in `Frame.h:356-359`, Ergänzung leerer Feinbänder `:376` (M-03), Gruppe ohne Beleg ohne Bit `:368`.
- **Vertrag:** Die Größe ist benannt in `nakama_telemetry_v1.fbs:203-210`, `:253`, `:263` (wortgleich in beiden generierten Dateien), `eq-ipc-v3.schema.json:433` (dBFS/Hz), `:512` (Bandleistung dBFS), `schemas/v3/README.md:12-13` und `flatbuffers/README.md:155`. Kein Feld und keine Version wurden geändert.
- **Versionsschritt:** `FeatureEngine.h:91`, `vergleichbarkeit.rs:160` und `prepost.rs:206` stehen auf 20260929, ebenso `metriken-v1.json:21` (`aktuell`, Fassungen 20260925 bis 20260929). A5 bindet die Werte in `pruefe_v3_vertrag.py:2316-2317`, `:2873-2875` und `:2968-2980`.
- **A/B-Beispiel in A4:** `zusammenhang.rs:483-498` prüft A = 0,4, B = 0,3 (±1·10⁻⁴) und A vor B (`:498`). Der Rotbeweis `NAK-380-rot-M-09.txt` (Gewicht → 1,0 ergibt A = 0,00670906) fällt an dieser Prüfung.
- **Weitere Tests:** M-10 `screening.rs:378`; M-11 bitgleich in Rust (`bandbreite.rs:53-55`) und C++ (`AnalysisGoldenTestMain.cpp:1482-1484`); Livegruppen B5 `AnalysisGoldenTestMain.cpp:1406-1464`.

**(2) DSP-22.**
- **Konstanten:** `FeatureEngine.h:239` (P0 −100 dBFS), `:258` (125 Cent), `:263` (H = 32), `:266` (κ = 3), `:269` (ρ = 1), `:283` (T_min 0,10 dB je Bin), `:288` (Sperrzeit 50 ms), `:292` (Klammer 1000).
- **Detektor:** Binfluss mit P0 und positiven Log-Deltas gegen den Maximumfilter des Vorframes: `Spektrum.h:712-756`, Filter O(K) `:759-788`.
  - Absolute Mindestschwelle: T_eff = max(med+κ·MAD, (1+ρ)·med, T_min), `:806`.
  - Echte MAD (Median der Absolutabweichungen): `:902-918`.
  - Spitzenwahl mit SF(n) ≥ SF(n−1) und Sperrzeit in ms·fs: `:851-856`.
  - Vorframe läuft auch über inaktive Frames: `:487`. Rückweg: `:921`. Stärke κ(SF−med)/(T_eff−med), höchstens 1000: `:877`.
- **§39.1:** Der Entwurf (`docs/FL-Nakama-Sonden-Design-Entwurf.md:2638-2640`) ist Satz für Satz erfüllt, nicht berichtigt.
- **Nullkorpus B5:** `AnalysisGoldenTestMain.cpp:1655-1666` prüft die Schranke ≤ 1, den exakten Golden, die gespeiste Samplezahl und Verlust 0 als eigene Prüfungen. Aufrufe `:2014-2030`: W1/W2/W3 (σ 0,1/0,01/0,00316), P1, S1, S2, V1, jeder mit Golden 0.
  - Gemessene Fehlalarmrate: 0 Ereignisse je 30 s, unter dem Matrixmaß 1. Der Rotbeweis M-50 zeigt am Basisstand 18 Ereignisse, am Endstand 0 (gelesen).
  - Impulskorpus `:1684-1736` und `:2039-2043`; hohe Raten M-115 `:2373-2385`; B18 M-42 bis M-49 `Sonde013DynamicsTest.cpp:996-1293`.

**(3) DSP-23 und DSP-25.**
- **Kohärenz je Bin:** Sxx, Syy und Sxy je Bin gehen in Ringslot und Kurzfenster, mit NaN-Riegel je Band: `Stereo.h:161-260`. Die MSC entsteht je Bin über die belegten Slots und wird erst danach im Band gemittelt (`:454`). Voraussetzung sind Ring-Freiheitsgrade ≥ 8 (`:427`).
  - Die Phase wird am Bin der Bandmitte gelesen (`:435`), die Laufzeit als Steigung der kleinsten Quadrate (`:502-505`).
- **Stereofenster über mehrere Evidenzfenster:** W_H = 3, W_B = 7 (`FeatureEngine.h:321`, `:330`). `evidenzLeeren` schiebt den Ring nur weiter (`Frame.h:167`, `Stereo.h:599-620`); Grenze und Rücksetzen leeren ihn ganz (`Stereo.h:625-632`).
- **B19:** alle Abschnitte ohne Filter aktiv (`Sonde013StereoGoldenTest.cpp:268-271`, `:1802-1817`).
  - M-73: `:992-1034`. M-74 über 18 Blockgrößen: `:1036-1046`.
  - M-75 bis M-79 bei 44,1/88,2/96/176,4/192 kHz, Blöcke 64/512/16 384: `:1048-1057`. Kohärenz ≥ 0,95 an jedem Snapshot ab dem siebten, Laufzeit innerhalb T_B nach R-380-13 (iii)/(v) (`:717-738`).
  - M-80 inklusive ±0,02 ms bei K ≥ 20: `:1066-1092`. Bass nach NAK-177 (M-83): `:1190-1232`.
- **LRA:** `kLraHopZellen = 1` (`FeatureEngine.h:600`), Kommentar `Lautheit.h:65`.
  - B18 `Sonde013DynamicsTest.cpp`: 721 Werte in 75 s `:429-444`; 60-s-Regel `:446-477`; Aliasing gegen §5-Referenz `:479-528`; NaN sperrt 30 Werte `:530-549`; der nachgezogene Golden gegen die §5-Referenz ±0,2 LU `:551-594`.
  - B9 Tabelle-1-Fälle 1 bis 4 (Norm ±1 LU und unabhängige §5-Referenz ±0,1 LU): `LoudnessGoldenTestMain.cpp:272-334`, im Standardlauf `:371-374`. Die Referenz ist unabhängig vom Produkt: `Nak380Pruefsignale.h:142-175`.

**(4) DSP-20, DSP-21 und GRENZE.**
- **Fensterdauer:** 2^round(log2(T·fs)) mit Kappe 65 536 in `FeatureEngine.h:164-176`, angewendet `:660-661`. Dieselbe Regel gilt für die M1: `AnalyseEngine.cpp:220-235`.
- **„Nicht gesucht":** Suchgrenze 17,30·fs/N_Bass, NaN ohne Rate: `AnalyseEngine.cpp:823-827`; Unterdrückung darunter `:1083`. Der Satz entsteht in `Diagnose.cpp:525-534`, steht in jeder Resonanzkarte (`:220-221`) und im vorhandenen Leertext (`PluginEditor.cpp:465-476`, nur Text). Der Testbeobachter ist nur in Testzielen übersetzt.
- **Validity:** Nur die Schreiber setzen `null` bzw. NaN (`DiagnoseAntwort.cpp:124-127`, `Ipc.cpp:1178`); die Werte im Prozess bleiben (`AnalyseEngine.h:163`).
- **M1-Wellen-Text:** Die Perzentile kommen aus linear gemittelten Teilblöcken von 8 aktiven Segmenten (`AnalyseEngine.cpp:30`, `:535-547`) und lesen nur das Teilblockhistogramm (`:898`).
- **Tests:**
  - A2 `GoldenTestMain.cpp`: M-103 bis M-105 `:1184-1219`, M-106 `:1244-1310`, M-107 `:1320-1339`.
  - B15 `ShotTestMain.cpp:2543-2670`: K4 bei 48 und 44,1 kHz am echten Editor, Zählprüfungen.
  - B5 M-98 bis M-120: `AnalysisGoldenTestMain.cpp:2195-2611`.
- **Vertragsgrenze:** Die Vertragsdateien ändern nur Beschreibungen, keine neue v3-Version. Die neuen Herabstufungsgründe (`vergleichbarkeit.rs:80-88`) reisen nur im Store; die Sitzungssicht trägt nur die Klasse. `metrics_version` am gespeicherten Fingerprint steht nur in der Store-Projektion (`experiment_verdrahtung.rs:699-710`), der Leser `:1672` fällt nicht auf die laufende Fassung zurück. `$defs/fingerprint` auf der Leitung bleibt unberührt.
- **Altstände:** Kein Broker-Leser vergleicht gegen `METRICS_VERSION` (per `git grep` geprüft). `null` im v2-Heartbeat nimmt der Broker an (`protokoll.rs:157`, `:191`).
- **Fixtures:** Einzige geänderte Fixture ist `festhalten-referenz.json` (LF, `-text`, über den Erzeuger neu); der v3-Korpus ist unverändert.
- **Audio-Thread:** Keine Datei des Audiopfads im Diff. Beide Engines laufen im Worker (`Analyse.cpp:326`, `SondeProcessor.cpp:863`); `messKompakt` läuft im PipeClient (`PluginProcessor.cpp:174`). Speicher entsteht nur in `vorbereiten` (`FeatureEngine.h:707`, `:729`; `AnalyseEngine.cpp:235`), begrenzt und benannt.
- **NaN-Ehrlichkeit:** nicht-endliche Binleistung wird als 0 verriegelt (`Spektrum.h:712-756`), Laufzeit und Kohärenz nur endlich, ohne Grenze kein Satz.
- **Produktverhalten seit BASIS** (gelesen, nicht gefahren): Identität unberührt, `CMakeLists.txt` ändert nur einen Testlink (`:1010`), Plugin-State unberührt, der Broker-Store ist nur additiv erweitert.

**Lücken und Härtungen**
- **L1 (LÜCKE):** Die Livegruppen-Ergänzung (`Frame.h:368-376`) füllt eine Gruppe, die die Nyquist-Kappe min(18 kHz, 0,95·fs/2) schneidet, über die volle Gruppenbreite auf, also auch jenseits der Kappe. Das betrifft nur Raten unter etwa 37,9 kHz; ab 44,1 kHz liegt die Kappe über der Gitteroberkante 17 959,39 Hz. `vorbereiten` nimmt jede Rate in (0, 768 kHz] an (`FeatureEngine.h:642`). Gate und Matrix schweigen zu Raten unter 44,1 kHz.
- **L2 (LÜCKE):** „Nicht gesucht" steht nur im Leertext und in Resonanzkarten (`Diagnose.cpp:220`, `PluginEditor.cpp:465`). Eine Befundliste nur mit Zonenkarten nennt die Suchgrenze nirgends. M-106 benennt genau diese zwei Orte; zu diesem Fall sagen Gate und Matrix nichts.
- **H1 (HÄRTUNG):** Die Ergänzungsdichte ist das arithmetische Mittel der belegten Feinbänder (`Frame.h:376`), nicht das breitengewichtete des Bauplans T-380-1 ((Σ D·w/Σ w)·W_g). Keine Matrixzeile unterscheidet die beiden: M-02 nutzt gleiche Dichten, M-03 ein belegtes Feinband. Die Abweichung beträgt höchstens wenige Prozent der ergänzten Dichte, nur in den untersten Bassgruppen.
- **H2 (HÄRTUNG):** M-18 (`telemetrie.rs:1229`) prüft die Fassungen 20260904, 20260925 und 20260926, nicht die End-Fassung 20260929 (auch nicht 20260927/28). Der Leser prüft den Wert nicht, die Regressionswache hält trotzdem.

HEAD zu Beginn: `3e5d36605ef6a0aaed463e8923ddaf660c85ee53`; HEAD am Ende: `3e5d36605ef6a0aaed463e8923ddaf660c85ee53` (identisch).

FERTIG Abschlussprüfung T2 NAK-380, PASS, 0 Defekte, 2 Lücken, 2 Härtungen
