# NAK-380 Erstprüfung 6 — Validierung der Befunde an der Quelle

- **Ticket:** NAK-380, Planschritt S25q, Etappe 6 (Fensterdauer statt Samplezahl, DSP-20) samt Nachtrag M-122 und Nacharbeit 0 nach R-380-14.
- **HEAD Beginn:** `f893c57eedf6a48741b9e0b3b470b9405c13112e` (26.09.2026 09:58:02 Uhr). **HEAD Ende:** `f893c57eedf6a48741b9e0b3b470b9405c13112e` (26.09.2026 10:14:51 Uhr, vor dem Schreiben dieser Datei; nach dem Schreiben erneut gemessen, gleich).
- **Datum:** 26.09.2026. **Modell:** Opus 5.5 (`claude-opus-5-5`), Effort xhigh; frischer Validierer, weder Bauer noch Nacharbeiter noch Prüfer.
- **Gegenstand:** Prüfbericht der Erstprüfung 6 (Codex `gpt-6-astra` xhigh, Thread `01a0dca7-2623-7ff3-9ad5-336ff417132f`, Vorlage A, URTEIL NEEDS_WORK, 5 Defekte, 0 Lücken, 0 Härtungen; Wortlaut `docs/beweise/roh/NAK-380-erstpruefung-6-urteil.md`) über `git diff a4ccba93...f893c57e -- broker eq-copilot tools`.
- **Prüfform:** nur lesend (Read, Grep, `git show`, `git grep`, `git diff --stat`, `git status`, `git rev-parse`); kein Bau, kein Test, kein cargo, kein Python, kein FL Studio. Alle Zitate am HEAD; die Arbeitskopie der zitierten Dateien ist gleich HEAD (`git diff --stat HEAD -- eq-copilot/plugin broker docs/beweise/NAK-380.md` und beide Etappenaufträge leer, 10:12:14 Uhr). Fremde Einträge, Bauartefakte, Installer-Pin und Kanon-Anhang nicht angefasst.
- Abkürzungen: **Manifest** = `docs/beweise/NAK-380.md`; **Auftrag E6** = `docs/beweise/roh/NAK-380-etappe-6-auftrag.txt`; **Auftrag N0** = `docs/beweise/roh/NAK-380-etappe-6-nacharbeit-0-auftrag.txt`; **Prüfauftrag** = `docs/beweise/roh/NAK-380-erstpruefung-6-auftrag.txt`; **B5-Datei** = `eq-copilot/plugin/tests/AnalysisGoldenTestMain.cpp`; **A2-Datei** = `eq-copilot/plugin/tests/GoldenTestMain.cpp`; **B15-Datei** = `eq-copilot/plugin/tests/ShotTestMain.cpp`.

Gemeinsamer Maßstab, wörtlich:

- Manifest:75-77 (§2): „je Matrixzeile fällt der Rotbeweis an der Zeile, die die Zusage trägt, nicht an einem Nebeneffekt.“
- Manifest:788-789 (§6-Kopf): „Das Etikett gilt je Satz der Zusage (R-313-12)“.
- Auftrag E6:26: „Vorbedingung hergestellt und verriegelt (…): jeder Test stellt Rate, Blockgröße, Dauer, Saat, Pegel, Tonfrequenz selbst her und verriegelt hergeleitete Zählgrößen mit Formel im Kommentar, nie aus dem Lauf abgelesen (R-380-8): (…) gespeiste Samplezahl = Signallänge und Verlustzähler 0 als eigene Prüfungen in jedem Läufer (Muster `nak380SamplesGeprueft`/`nak380VerlustGeprueft`, M1-Läufer `nak380M1Fahren`)“.
- Auftrag E6:25: „Wiederverwendete Helfer tragen jede Ereignisspalte (Lehre §50.1 D1)“. Auftrag E6:28: „Keine Wache ohne Trennschärfe (…): (…) M-106 prüft den Satz mit der Zahl 51 bei 48 kHz und 47 bei 44,1 kHz“. Auftrag E6:24: „Doppelt getragene Zusagen (…): trägt eine Zusage zwei Riegel, ist der Rotbeweis eine Doppelmutation mit Gegenproben je Einzelmutation“.
- Manifest:1100-1101 (§7.1): „der Test prüft seinen Erzeuger vor jedem Nutzer gegen E-380-13“.

## Teil 1 — je Befund

### D1 — M-120 nach dem Wechsel

**a) Matrixzeile und Präzisierungen.**
Manifest:1041, M-120 wörtlich: Befund „DSP-20 · Ratenwechsel im Lauf (starten↔stoppen)“; Zustand „FeatureEngine und AnalyseEngine: 30 s bei 48 kHz, dann `vorbereiten (96000)`, 30 s“; Ereignis „nach dem Wechsel“; Zusage „neue Längen 32 768/8 192 (M1 32 768/16 384/8 192/4 096), kein Akku, Ring, Vorframe oder Histogramm der alten Rate überlebt (B5-Grenzfall „Sampleratewechsel“ erweitert), Allokation nur im Worker unter der Steuersperre“; Test „C++: B5 bestehend „Sampleratewechsel“, NEU `380/M-120 ratenwechsel_laengen`“; Rotbeweis „zwei Hälften: Leeren Regressionswache (Mutation `zuruecksetzen` aus `vorbereiten` → Akkus überleben → rot); Längen heute rot (Mutation wie M-98)“.
Auftrag E6:23: „vorbereiten↔zuruecksetzen (M-120: kein Akku, Ring, Vorframe, Histogramm oder Vorfenster der alten Rate überlebt; Allokation nur im Worker unter der Steuersperre)“. Auftrag E6:37 (Zahlenränder): „erster Snapshot nach Ratenwechsel trägt nur neue Frames (M-120, §32.3)“. Dazu Auftrag E6:26 und Manifest:1100-1101 (oben).

**b) Nachmessung am HEAD.**
- Signale: B5-Datei:2404 `const auto p48 = sig::rosaMono (sig::kP2Saat, 0.1, 1440000u);` (30 s bei 48 kHz), Selbstprüfung :2405-2406 nur an `p48`. B5-Datei:2407 `const auto p96 = sig::rosaMono (sig::kP2Saat, 0.1, 96000u);` = 96 000 Samples = **1 s** bei 96 kHz; für `p96` steht kein `rosaSelbstpruefung`. Der Erzeugerkopf leitet seine Pegeltoleranz für mindestens 240 000 Samples her (`Nak380Pruefsignale.h:450-451`: „Kuerzester Nutzer 240 000 Samples“); `p96` liegt darunter.
- FeatureEngine: :2410 `auto lauf = nak380Korpuslauf (p48);`, :2411 `nak380SamplesGeprueft ("M-120", …, p48.size(), 1440000u, …)`; kein `nak380VerlustGeprueft` (Muster :1629-1630, :2343-2344). :2415 `e.vorbereiten (96000.0);` — danach speist der Fall `e` nicht mehr: **die FeatureEngine erhält nach dem Wechsel 0 Samples.** Geprüft wird nur der Zustand unmittelbar nach `vorbereiten`: Längen und Hop (:2418-2422), Stufenringe leer (:2423-2432), Band-Akkus, `verarbeitet (e) == 0u` und Detektorhistorie (:2433-2441).
- Der Docstring behauptet mehr, als der Fall prüft: :2394-2396 „Danach traegt die Engine nur neue Samples: nach 1 s bei 96 kHz 96 000 verarbeitete; die M1-Bassstufe hat … 4 Segmente“. Für die FeatureEngine existiert diese Prüfung nicht.
- AnalyseEngine (M1), eigener Thread :2449-2450: Speiselambda :2459-2470 (Rest korrekt über `std::min`, :2464), 30 s bei 48 kHz (:2471), Zustand nach `vorbereiten (96000)` (:2473-2494: Längen, Stufen, Segmente und Histogramme leer), dann `speise (*m1, p96)` = 1 s (:2500) und Band 64 = 4 Segmente (:2501-2510). Eine eigene Samplezahlprüfung hat die M1 weder bei 48 noch bei 96 kHz; das Segmentsoll 4 bindet nur 81 920 ≤ n < 98 304 Samples.
- Snapshots nach dem Wechsel: keine (kein `auswerten` der M1, kein Frame der FeatureEngine).
- Angenommene Abweichungen: §54.11 Punkt 5 (Manifest:3175-3177) „M-120 (B5) auf eigenem Thread für die M1-Hälfte (…) Ablauf unverändert, Engine im Heap“ deckt den Thread; Punkt 12 (:3192-3193) den G7-Block, Punkt 13 (:3194-3196) Band 64. Keiner nennt 1 s oder 0 s; `NAK-380-rot-M-120.txt:10` führt „nach 1 s bei 96 kHz“ als Messung, nicht als Abweichung. Einordnung §55.1 (Manifest:3369) nimmt nur §54.11 Punkte 1 bis 17 an.
- Was die Zeile verlangt: Die Zusagespalte nennt Zustandssätze, die unmittelbar nach dem Wechsel prüfbar sind (Längen, Leeren); die 30 s danach stehen in der Zustandsspalte, der Snapshot nach dem Wechsel in Auftrag E6:37. Die FeatureEngine-Seite des Snapshotsatzes misst an HEAD nur ein anderer Fall für die Stereoevidenz (B19 `380/M-87 ring_leert rate_change`, `Sonde013StereoGoldenTest.cpp:1477-1487`, 1 s bei 96 kHz), nicht M-120.

**c) Was bricht.** Der Zustand von M-120 (Manifest:1041) „dann `vorbereiten (96000)`, 30 s“ zusammen mit Auftrag E6:26 (Dauer selbst hergestellt, Zählgrößen verriegelt) und Auftrag E6:37 („erster Snapshot nach Ratenwechsel trägt nur neue Frames (M-120, §32.3)“); für `p96` zusätzlich Manifest:1100-1101 (E-380-13 vor jedem Nutzer). Die Zusagespalte selbst ist für den Zustand unmittelbar nach dem Wechsel getragen; „Weiterbetrieb“ steht dort nicht wörtlich, die Forderung kommt aus Zustand und Auftrag E6:37.

**d) Urteil.** **PRÄZISIERT** — **DEFEKT**. Die Tatsachen des Prüfers stimmen (1 s, 0 s für die FeatureEngine, 2 880 000 selbstgeprüfte Samples je Engine nötig); präzisiert sind die brechenden Sätze (Zustand und Auftrag E6:37 statt Zusagespalte), die fehlende Selbstprüfung von `p96` und der falsche Docstring. Kein Ausschluss: §54.11 Punkte 5, 12, 13 decken die Dauer nicht; NAK-406 betrifft den Stapel, nicht die Dauer.

**e) Schließende Regel.** M-120 speist nach `vorbereiten (96000)` 30 s = 2 880 000 Samples eines vor dem Nutzer bei 96 kHz selbstgeprüften P2 in beide Engines, verriegelt gespeist = verarbeitet = 2 880 000 (FeatureEngine zusätzlich Verlust 0) als eigene Prüfungen und prüft danach, dass beide nur Frames der neuen Länge tragen (M1 Band 64: floor((2 880 000 − 32 768)/16 384) + 1 = 174 Segmente; FeatureEngine entsprechend aus 32 768/8 192 hergeleitet). Folgestellen: B5-Datei `nak380M120` :2384-2451 und `nak380M120M1` :2453-2511 samt Docstring :2394-2396; `docs/beweise/roh/NAK-380-rot-M-120.txt` (neue Prüfzeile mit Rotbeweis); §54.7-Nachfolgetabelle zu Manifest:3081-3086 (neue Zeile „nach 30 s nur Frames der neuen Rate“).

### D2 — Signal des B15-Leertextfalls

**a) Matrixzeile und Präzisierungen.**
Manifest:1027, M-106 wörtlich: Zustand „AnalyseEngine 48 kHz; nach E-380-13 selbstgeprüftes rosa Rauschen mit Sinus 45 Hz +15 dB (§7 K4)“; Ereignis „`diagnose`, Leertext der Befundliste“; Zusage „keine Karte; der Satz „Unter 51 Hz wurde nicht nach Tönen gesucht – das Messfenster ist dort zu grob.“ (Wortlaut §8.6, Zahl = gerundetes `resonanzSucheAbHz`) steht im Leertext (`PluginEditor.cpp:453-456`) und in `gemessen` jeder Resonanzkarte; bei 44,1 kHz nennt er 47 Hz“; Reihenfolge „Text aus `Diagnose.cpp` (Satzfunktion), vom Editor nur angefügt“; Test „C++: A2 NEU `380/M-106 nicht_gesucht_satz`; B15 NEU `380/M-106 leertext_nennt_suchgrenze`“; Rotbeweis „heute rot: (…); nach dem Bau Mutation Satzfunktion liefert leer → rot“.
Manifest:1146 (§7.2 K1 bis K5): „P2 (−20 dBFS) plus Sinus | Sinusleistung so, dass das LTAS-Band des Tons 15 dB über der Rosa-Banddichte liegt (…); (…) K4 45 Hz bei 48 kHz, K5 48 Hz bei 48 kHz | 0x3800008 | 30 s“. Dazu Auftrag E6:26 (Dauer, Tonfrequenz) und E6:28.

**b) Nachmessung am HEAD.**
- B15-Datei:2531 Raten `{ 48000.0, 44100.0 }`; :2533 `sig::rosaMono (sig::kP2Saat, 0.1, (std::uint64_t) std::llround (20.0 * fs))` — **20 s reines P2, kein Ton**; Selbstprüfung :2534, :2537. Der Kommentar :2510-2513 nennt die Wahl ausdrücklich: „Das Material ist P2 (rosa -20 dBFS, Saat 0x3800008, 20 s, E-380-13 vor dem Nutzer): kein Ton sticht heraus, die Liste ist leer“.
- Prüfungen je Rate: :2586-2590 (messbereit, Knopf „keine Auffälligkeit“, genau ein Leertext), :2591-2593 (`gesehen == soll` mit 51 bzw. 47 Hz, :2579).
- Der Satz ist signalunabhängig: `Diagnose.cpp:525-534` formt ihn nur aus `m.resonanzSucheAbHz`; `PluginEditor.cpp:459-476` hängt ihn an, wenn `befunde.empty()`. Der B15-Fall misst damit, dass der echte Editor den Satz bei leerer Liste anfügt (Rotbeweis `m106-satz` fällt dort, `NAK-380-rot-M-106.txt:42-43`). Mit K4 mäße er zusätzlich, dass ein Ton unter der Suchgrenze am echten Gen-Pfad **keine Karte irgendeiner Klasse** erzeugt (Knopf und Leertext erscheinen nur dann) und der Nutzer genau in diesem Fall „nicht gesucht“ liest (messen↔melden).
- A2-Fall K4: A2-Datei:1260-1276 (K4, 1 440 000 Samples, `kSelbstpruefung` :1263, Prüfung „keine Resonanzkarte an irgendeinem Snapshot“ :1271-1273). `nak380Karten` zählt nur `BefundKlasse::resonanz` (A2-Datei:1168), obwohl der Lauf alle Befunde der Diagnose sammelt (:1116). Zuständigkeit laut §54.7: „keine Karte bei 45 Hz (K4)“ → A2 als „keine Resonanzkarte“ (Manifest:3055); Leertext → B15 (Manifest:3058). **„Keine Karte“ jeder Klasse unter K4 misst an HEAD kein Test.**
- 44,1-kHz-Hälfte: K4 ist in §7.2 nur „bei 48 kHz“ definiert (Manifest:1146); für 44,1 kHz nennt die Matrix kein Signal.

**c) Was bricht.** Für die 48-kHz-Hälfte des B15-Falls: Zustand von M-106 (Manifest:1027, K4) mit §7.2 K4 (Manifest:1146: 45 Hz, +15 dB, 30 s) und Auftrag E6:26 („Dauer, Saat, Pegel, Tonfrequenz selbst her“). Für die 44,1-kHz-Hälfte bricht kein Satz; die Matrix schweigt.

**d) Urteil.** **PRÄZISIERT** — **DEFEKT** (48 kHz), **LÜCKE** (44,1 kHz). Kein Ausschluss: §54.11 Punkt 1 (Manifest:3160-3164) deckt nur den Testhaken „`testzugang::leertextBeobachterFuerTest` (…) nur unter `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3`“; die Signal- und Dauerwahl ist weder in §54.11 noch in §55.1 (Manifest:3369) geführt.

**e) Schließende Regel.** B15 `380/M-106` fährt bei 48 kHz K4 (`kSignal (45, 48 000, 1 440 000)` mit `kSelbstpruefung` vor dem Nutzer, 30 s) und verriegelt am echten Editor „keine Karte“ (Knopf „keine Auffälligkeit“) und den Leertext mit 51 Hz; das Signal der 44,1-kHz-Hälfte entscheidet der Dirigent (naheliegend der K4-Erzeuger bei 44,1 kHz, 45 Hz < 46,57 Hz, 30 s, als Zusatz zu §7.2). Folgestellen: B15-Datei `leertextNenntSuchgrenze` :2527-2600 samt Kommentar :2507-2515; `NAK-380-rot-M-106.txt` (B15-Teil); §54.7-Nachfolge zu Manifest:3058; bei Entscheid §7.2-Zeile K1 bis K5.

### D3 — Restblock im 44,1-kHz-Leertextlauf

**a) Maßstab.** Auftrag E6:26 (oben, „gespeiste Samplezahl = Signallänge und Verlustzähler 0 als eigene Prüfungen in jedem Läufer“); Manifest:1112-1113 (§7.1): „der Speiser zerlegt jedes Signal in Blöcke der genannten Größe mit fortlaufender Projektzeit“.

**b) Nachmessung am HEAD.**
- Signallänge B15-Datei:2533 `llround (20.0 * fs)`: 960 000 bei 48 kHz, 882 000 bei 44,1 kHz.
- B15-Datei:2543 `for (std::size_t i = 0, n = 0; i + 512u <= x.size(); i += 512u, ++n)`: bei 48 kHz 1 875 Blöcke = 960 000 (Rest 0); bei 44,1 kHz 1 722 Blöcke = 881 664, **verworfen 882 000 − 881 664 = 336 Samples**.
- Prüfungen je Rate genau drei (:2537, :2586, :2591), Summe `return fehler == 0 && geprueft == 6;` (:2599) — keine Samplezahl-, keine Verlustprüfung. Die Speisung wartet nicht auf Verbrauch, sie schläft 60 ms je 48 Blöcke (:2551-2552); Drops sind nicht gemessen.
- Zugang vorhanden: `MessSnapshot::verarbeiteteSamples` (`AnalyseEngine.h:198`; der Fall liest `m = proz->messSnapshot()` :2559), Dropzähler `analyseDropsUeberlauf()`, `analyseDropsOversize()`, `analyseQuarantaeneVerworfen()` (`PluginProcessor.h:210-215`). Auf dem Gen-Pfad hält die Ein-Block-Quarantäne den jüngsten Block (`eq-copilot/plugin/core/StampedAudioQueue.h:656-658`: „Worker haelt den juengsten vollstaendigen Block in Ein-Block-Quarantaene. Erst der zeitlich konsistente Folgeblock versiegelt ihn.“); die Engine verarbeitet deshalb höchstens gespeist − letzter Block.
- Muster der anderen Läufer: `nak380Korpuslauf` verarbeitet den Rest über `std::min` (B5-Datei:1549-1550) und liefert Läufer- und Enginezahl (:1578-1582), geprüft von `nak380SamplesGeprueft` (:1590-1599) und `nak380VerlustGeprueft` (:1603-1609); A2 `nak380M1SpeisenSamples` (A2-Datei:446-449, Rest über `std::min`) und `nak380KFahren` (:1125-1132) mit der Prüfung :1194-1197.

**c) Was bricht.** Auftrag E6:26. Die Zusage selbst (Leertext) hängt nicht an den 336 Samples (`Diagnose.cpp:525-534`).

**d) Urteil.** **BESTÄTIGT** — **DEFEKT**. Zahlen exakt nachgerechnet. Kein Ausschluss.

**e) Schließende Regel.** B15 speist den Rest als kurzen letzten Block (336 Samples bei 44,1 kHz) und verriegelt je Rate als eigene Prüfungen gespeist = Signallänge und alle Dropzähler 0, dazu die Enginezahl gegen die hergeleitete Quarantänedifferenz (verarbeitet = gespeist − letzter Block). Folgestellen: B15-Datei :2543-2553 und Zählerstand :2598-2599; `NAK-380-rot-M-106.txt` (B15-Teil); Behauptungszeile B15 in `tools/beweise.ps1`, falls sie den Zählerstand nennt; §54.7-Nachfolge zu Manifest:3058.

### D4 — Verbrauch im Heartbeat-Fall M-110

**a) Matrixzeile und Präzisierung.** Manifest:1031, M-110 wörtlich: Befund „DSP-20 · Validity im v2-Heartbeat“; Zustand „Gen 48 kHz nach einem Schwerlauf“; Ereignis „`messKompakt()` (`Ipc.cpp:1150-1180`)“; Zusage „`ltasKompositDb` trägt NaN (im JSON `null`, `PipeClient.cpp:196-197`) genau für die Bänder aus M-109; der Broker nimmt `null` an (`broker/src/protokoll.rs:157`, `:191`)“; Test „C++: B5 NEU `380/M-110 heartbeat_null_interpoliert` (Muster `AnalysisGoldenTestMain.cpp:2722`)“; Rotbeweis „heute rot; Mutation wie M-109 → rot“. Dazu Auftrag E6:26.

**b) Nachmessung am HEAD.**
- Signal B5-Datei:2525 `sig::rosaMono (sig::kP2Saat, 0.1, 960000u);   // 20 s`, Selbstprüfung :2526-2527. Speisung über den echten Prozessor (`nakamaBlockEmpfangen` und `processBlock`, :2539-2558), Züge zu 40 Blöcken, Bedingung `pos + (std::size_t) block <= x.size()` (:2537, :2539); 960 000 = 1 875 · 512, kein Rest.
- Verbrauch :2559-2562: `while (p->merkmaleBloecke() + 1u < gefuettert …)`, `verbraucht = p->merkmaleBloecke() + 1u >= gefuettert;` — Toleranz ein Block. Das ist die Ein-Block-Quarantäne (`StampedAudioQueue.h:656-658`; derselbe Grund steht in B30 `BriefkastenTestMain.cpp:258-261`: „Die Quarantaene haelt den juengsten Block: verbraucht sind alle bis auf ihn.“); im M-110-Kommentar (:2517-2519) steht nur „folgt der Referenzbuehne von B30“. `merkmaleBloecke()` zählt die Blöcke der FeatureEngine (`PluginProcessor.h:237`); der Heartbeat kommt aus der AnalyseEngine (`Ipc.cpp:1152`, :1170-1181), die im selben Workerschritt gespeist wird, außer bei Straddle-Verwurf (`prozessor/Analyse.cpp:312-333`).
- Prüfungen: :2580-2584 „Vorbedingung 20 s verbraucht, Heartbeat traegt 221 Baender, Maske der Referenz nicht leer“ (`verbraucht && k.ltasKompositDb.size() == 221u && maske.anzahlKomposit > 0`), :2585-2590 NaN genau für die Maskenbänder. Keine Prüfung von `gefuettert == 1875`, keine von `messSnapshot().verarbeiteteSamples` (`PluginProcessor.h:1113`, `AnalyseEngine.h:198`), keine Dropzähler (`PluginProcessor.h:210-215`). Die Maskenreferenz hängt nur an Binzuordnung und Nähten (`Nak380Pruefsignale.h:823-882`); ein fehlender Block änderte die Zusageprüfung nicht.
- Präzisierung zum Soll des Prüfers: Auf dem Gen-Pfad bleibt der letzte Block ohne Folgeblock in der Quarantäne; „die vollständigen 960.000 Samples“ in der Engine sind so nicht erreichbar. Herleitbar sind: gespeist 960 000 (1 875 Blöcke), von der AnalyseEngine verarbeitet 960 000 − 512 = 959 488, Drops 0 — oder ein Folgeblock versiegelt den letzten Block.

**c) Was bricht.** Auftrag E6:26. Die NaN-Zusage ist getragen; der zweite Zusagesatz „der Broker nimmt `null` an“ ist durch bestehende A4-Tests getragen (`broker/src/protokoll.rs:648-660`: `"ltas_komposit_db":[-40.0,null,-42.5]`, `assert_eq!(m.ltas_komposit_db.as_ref().unwrap()[1], None);`).

**d) Urteil.** **PRÄZISIERT** — **DEFEKT**. Die fehlende eigene Zähl- und Verlustprüfung ist bestätigt; der „fehlende Block“ ist die Quarantäne des Produkts, und das Soll ist 959 488 verarbeitete Samples bei 960 000 gespeisten, nicht 960 000. Kein Ausschluss.

**e) Schließende Regel.** M-110 verriegelt als eigene Prüfungen gespeist = 960 000 Samples (1 875 Blöcke), die von der AnalyseEngine verarbeitete Zahl gegen die aus der Quarantäne hergeleitete Differenz (959 488) und alle Dropzähler 0; die Toleranz von `verbraucht` trägt diese Herleitung im Kommentar. Folgestellen: B5-Datei `nak380M110` :2513-2591; `NAK-380-rot-M-110.txt` (neue Prüfzeilen); §54.7-Nachfolge zu Manifest:3065.

### D5 — NaN bei unbekannter Messfassung

**a) Matrixzeile, Regel und Wortlaute.**
Manifest:1048, M-122 Zusage wörtlich: „gleiche `metrics_version`: endliche numerische Ähnlichkeit in [0, 1] wie heute; ungleiche `metrics_version`: Ergebniszustand „nicht vergleichbar“ (fail-closed), kein Zahlenwert und nie „anderes Material““.
Matrixfassung §55.2 (Manifest:3424-3429): „M-122 (§6.5): Zustand ergänzt „ein gespeicherter Fingerprint trägt die Messfassung seines Schreibers; fehlt sie, ist die Fassung unbekannt“; Zusage ergänzt „unbekannte Fassung → nicht vergleichbar“; Test ergänzt „A4 NEU `nak380_m122_gespeicherte_fassung_*`“; Rotbeweis ergänzt „Rückfall des Lesers, Aufrufer mit `METRICS_VERSION`, Schreiber ohne Feld“.“
R-380-14 (ii) (Manifest:3396-3400): „Unbekannte Fassung auf einer Seite ist fail-closed „nicht vergleichbar“ (kein Materialwechsel, nie `MaterialVerschieden`, Grund `MessfassungVerschieden` oder ein eigener benannter Grund „Messfassung unbekannt“ — Technikentscheid des Bauers, im Manifest begründet).“
Die im Validierungsauftrag R-380-14 (ii) zugeschriebene Wendung „kein Zahlenwert (material_cosine NaN)“: **in §55.2 (Manifest:3371-3429) nicht gefunden**; `git grep -n 'Zahlenwert' HEAD -- docs/beweise/NAK-380.md` trifft nur :870 (M-07, fremd), :1048 und :3089 (beide „ungleiche Fassung“). Wörtlich steht sie im Codevertrag `broker/src/coordinator/vergleichbarkeit.rs:339-340` („kein Zahlenwert (`material_cosine` NaN)“) und :357-362 („Beide Male kein Zahlenwert und nie `MaterialVerschieden`.“), im Prüfauftrag Punkt 7 (Prüfauftrag:24: „… nie MaterialVerschieden, kein Zahlenwert (material_cosine NaN), kein Materialwechsel“) und in dessen DEFEKT-Liste (Prüfauftrag:30: „eine Zahl statt NaN bei ungleicher oder unbekannter Fassung“). Auftrag N0:14 und N0:23 (Doppelmutation bei zwei Riegeln).

**b) Nachmessung am HEAD.**
- Test (b) `broker/src/coordinator/experiment_verdrahtung.rs:2307-2341`: Leser liefert `None` (:2314-2325); Vergleichbarkeit `m.vergleichbarkeit.as_deref() == Some("unvergleichbar") && hat(&m, "MessfassungUnbekannt") && !hat(&m, "MaterialVerschieden")` (:2327-2333); Invalidierung `!wirkung && ausschluss.is_none()` (:2335-2339). Kein `material_cosine`: der Fall geht über `resultatmessung`, und `Resultatmessung` hat kein solches Feld (`broker/src/coordinator/experiment.rs:293-355`; `resultatmessung` übernimmt nur Klasse und Gründe, `experiment_verdrahtung.rs:1297-1302`).
- Riegel `vergleichbarkeit.rs:373-382`: `(Some(fa), Some(fb))` → `fingerprint_vergleich`; `_ => (f64::NAN, Some(Herabstufungsgrund::MessfassungUnbekannt)),` (:381).
- Einzige NaN-Prüfung: `broker/src/telemetrie.rs:1349-1351` an `beurteile_versioniert(&beleg(&a), ALT, &beleg(&b), NEU)` (:1343) — zwei bekannte, verschiedene Fassungen, Zweig :377-379.
- Rotbeweise: `NAK-380-e6n0-mutation-m122-riegel-unbekannt-verg.txt` (Mutation :7-12 „None gilt als METRICS_VERSION“) fiel an „(b) Vergleichbarkeit, Altstand ohne Feld …“ mit `Some("schwach") ["MaterialVerschieden"]` (:16), also an Klasse und Grund; `…-riegel-unbekannt-inv.txt` (Mutation :7-14) fiel an „(b) Invalidierung“ (:18).
- Mutation des Prüfers (:381 `f64::NAN` → `0.0`, Grund unverändert), am Code nachgerechnet: `gruende` erhält weiter `MessfassungUnbekannt` (:410-411), frühe Rückgabe `Unvergleichbar` (:432-441) mit `material_cosine` 0.0; Test (b) liest das Vergleichsurteil nicht; der `telemetrie.rs`-Test nimmt den Zweig :375-379; `broker/tests/sonde013_passage.rs` ruft `beurteile` (Some/Some, `vergleichbarkeit.rs:333-335`). **Alle Prüfungen blieben grün** (gelesen, nicht gefahren).
- Produktwirkung: `material_cosine` liest im Broker außer `vergleichbarkeit.rs` nur Testcode (`git grep -n 'material_cosine' HEAD -- broker`: `telemetrie.rs`-Test, `broker/tests/sonde013_passage.rs`). Eine 0.0 wäre im Produkt nicht sichtbar.

**c) Was bricht.** Die M-122-Zusage „Ergebniszustand „nicht vergleichbar“ (fail-closed), kein Zahlenwert“ (Manifest:1048), durch §55.2 auf die unbekannte Fassung ausgedehnt (Manifest:3427 „unbekannte Fassung → nicht vergleichbar“) in der Lesart, dass der Ergebniszustand „nicht vergleichbar“ „kein Zahlenwert“ mitträgt — so implementiert und vertraglich kommentiert (`vergleichbarkeit.rs:357-362`) und so in der DEFEKT-Liste des Prüfauftrags (:30) geführt; mit Manifest:75-77 und :788-789 fehlt diesem Satz Prüfzeile und Rotbeweis. R-380-14 (ii) selbst zählt den Satz nicht auf.

**d) Urteil.** **PRÄZISIERT** — **DEFEKT** (ohne Produktwirkung). Tatsachen und Mutation des Prüfers bestätigt; präzisiert ist die Quelle des Satzes (M-122-Zusage in der Fassung §55.2 und Prüfauftrag:30, nicht der Wortlaut von R-380-14 (ii)). Kein Ausschluss: §56.9 Punkte 1 bis 6 und die Technikentscheide §56.3 (Manifest:3570-3582) betreffen Ort, Namen, Grund und Schreiber, nicht die NaN-Prüfung; NAK-410 und NAK-411 sind nicht berührt.

**e) Schließende Regel.** Für die unbekannte Fassung prüft A4 das Vergleichsurteil von `beurteile_mit_messfassung` direkt (`None` auf je einer Seite und auf beiden) auf `material_cosine` NaN, zusätzlich zu Klasse und Grund; Rotbeweis `vergleichbarkeit.rs:381` NaN → 0.0 bei unverändertem Grund macht genau diese Prüfung rot. Folgestellen: `experiment_verdrahtung.rs` `nak380_m122_tests` Fall (b) :2307-2341 oder der `telemetrie.rs`-Test :1257-1353; `NAK-380-rot-M-122.txt` (Abschnitt e6n0, neue Mutation); neues Protokoll `NAK-380-e6n0-mutation-m122-unbekannt-nan.txt`; §56.5-Zeile (b) Vergleichbarkeit (Manifest:3614); Behauptungszeile A4 in `tools/beweise.ps1` (Zählerstand).

## Teil 2 — Gemeinsame Ursache und Durchsicht

**Wurzeln.** Die drei vermuteten Wurzeln stimmen: (W1) der Test bildet Vorbedingung oder Zusage der Matrixzeile nicht vollständig ab — D1 (Dauer, `p96` ungeprüft), D2 (K4, Dauer); (W2) Läufer ohne eigene Zähl- und Verlustprüfung — D3, D4; (W3) Zusage mit mehreren Sätzen, nur einer geprüft — D5. Gemeinsamer Nenner: Die Präzisierungen Auftrag E6:26 und Manifest:1100-1101 sind am gemeinsamen B5-Läufer (`nak380Korpuslauf` mit den zwei Helfern) und am A2-K-Läufer (`nak380KFahren`) umgesetzt, nicht an Fällen mit eigener Speiseschleife (B15, M-110 über den Prozessor, M-120-M1-Lambda) und nicht dort, wo der geprüfte Satz aus einem signalunabhängigen Zustand folgt (B15-Leertext, M-120-Leeren). Die Tabellen §54.7 und §56.5 führen je Zusagesatz eine Prüfzeile, aber keine Spalte für Vorbedingung und Zählprüfung; genau dort fehlen die Stellen.

**Durchsicht der Läufer der Etappe 6.**

| Läufer | Signal und Vorbedingung gegen §7.2 | Samplezahl eigene Prüfung | Verlust | Restblock | Ergebnis |
|---|---|---|---|---|---|
| B5 `nak380M98`, `M100`, `M102`, `M114`, `M119` | kein Signal (nur `vorbereiten`) | – | – | – | kein Läufer |
| B5 `nak380M115` | W1 `weissMono (kW1Saat, 0.1, 30·fs)` (B5-Datei:2339-2341) = Manifest:1120 | :2343 | :2344 | `std::min` :1549-1550 | in Ordnung |
| B5 `nak380M110` | P2 20 s, Zustand nennt kein Signal | fehlt | fehlt | Signal ohne Rest | D4 |
| B5 `nak380M120` | P2 30 s + 1 s statt 30 s + 30 s | FE :2411, M1 fehlt | FE fehlt | `std::min` | D1, Z3, Z4 |
| B5 NAK-182-Sweep (M-113, angepasst) | kein fest langes Signal (`while (sp.strom < bis)`, :2300) | – | E-b Ringüberlauf :2438-2441 | Blockrundung | bestehender Läufer, nur t0/Laufende/Schranke berührt; keine eigene Wertung |
| A2 `nak380M99`, `M101`, `M108` | kein Signal | – | – | – | kein Läufer |
| A2 `nak380Resonanzfall` (M-103 bis M-105) | K1/K2/K3 mit `kSelbstpruefung` (A2-Datei:1187-1192) = Manifest:1146 | :1194-1197 (Läufer und Engine) | kein Ereignisring | `std::min` :1127 | in Ordnung |
| A2 `nak380M106` K4 | K4 30 s 48 kHz (:1261-1263) | nur `gespeist` (:1268) | – | `std::min` | Z1 |
| A2 `nak380M106` Karte 60 Hz | K-Erzeuger 60 Hz (:1278-1281) | fehlt | – | `std::min` | Z2 |
| A2 `nak380M107` K5 und Gegenprobe | K5 und 60 Hz (:1312-1319) | :1321-1323 | – | `std::min` | in Ordnung |
| A2 `nak380M112` | P2 30 s bei 48/96 kHz, selbstgeprüft (:1383-1387) | :1401-1403 (Engine, mit 300 Zellen) | – | `std::min` :1393 | in Ordnung |
| A2 `nak380M1Fahren` | von Etappe-6-Fällen nicht benutzt | – | – | `std::min` :448-449 | kein Etappe-6-Läufer |
| B30 `nak380M109` | P2 30 s, selbstgeprüft (`BriefkastenTestMain.cpp:3827-3829`) = Manifest:1125 | :3854 (Engine) | – | `std::min` :3835 | in Ordnung |
| B15 `leertextNenntSuchgrenze` | P2 20 s statt K4 30 s | fehlt | fehlt | 336 verworfen | D2, D3 |
| K-Erzeuger `kSignal`, `kSelbstpruefung` | Saat P2 (:922), RMS 0,1 über `rosaSelbstpruefung` (:964), Ton gemessen (:965-978), Pegel 15 dB ± 0,01 (:979-987), Form (:985-986) | – | – | – | in Ordnung |
| `rosaSelbstpruefung` vor jedem Rosa-Nutzer | M-110 :2526, M-112 :1385, M-109 :3828, B15 :2534, M-120 `p48` :2405; `p96` (:2407) ohne | – | – | – | `p96` unter D1 erfasst |

**Zusagen mit mehreren Sätzen.**
- M-122 (ii): „nicht vergleichbar“ (experiment_verdrahtung.rs:2328), Grund (:2329), nie `MaterialVerschieden` (:2330), kein Materialwechsel (:2336) geprüft; „kein Zahlenwert“ nicht → D5.
- M-109: `null` genau für die Maskenbänder, alle anderen Zahl, je Kurve — `BriefkastenTestMain.cpp:3879` (`nullOhne == 0 && zahlTrotz == 0 && nullGenau == sollNull && nullGenau + zahlen == 221`), :3884-3885. In Ordnung.
- M-110: NaN genau für die Maskenbänder, sonst Zahl (B5-Datei:2572-2590); „der Broker nimmt `null` an“ tragen die bestehenden A4-Tests `protokoll.rs:648-660` (in §54.7 Manifest:3065 nicht geführt). Keine weitere Stelle.
- M-119: zwei Größen und Stapelwache je eigene Prüfzeile (B5-Datei:2375-2381; Manifest:3078-3080). In Ordnung.
- M-106: „keine Karte“ nur als Resonanzklasse gemessen → unter D2 erfasst.
- M-120 → Z4. M-115 → Z5.

**Weitere Stellen.**

- **Z1 — A2 M-106 K4, Enginezahl fehlt.** A2-Datei:1268-1270: `p.wahr (l->gespeist == samples && l->messbereit == 61, "380/M-106 K4: Vorbedingung 30 s gespeist und verarbeitet, 61 Snapshots ab messbereit", …)` — `l->verarbeitet` (gesetzt :1142) wird nicht geprüft, obwohl der Prüftext „verarbeitet“ behauptet; die Nachbarfälle prüfen beide (:1194-1197, :1321-1323). Wurzel W2. **DEFEKT** (Auftrag E6:26). Regel: die K4-Vorbedingung verriegelt gespeist = verarbeitet = 1 440 000 wie M-103 bis M-105 und M-107.
- **Z2 — A2 M-106 Karte 60 Hz ohne Vorbedingungsprüfung.** A2-Datei:1277-1298: nach `nak380KFahren (k.x, 48000.0)` (:1282) folgt nur `karten > 0 && mitSatz == karten` (:1294); keine Prüfung von gespeist, verarbeitet oder messbereit. Wurzel W2. **DEFEKT** (Auftrag E6:26). Regel: derselbe Vorbedingungssatz wie die M-107-Gegenprobe (:1321-1323).
- **Z3 — B5 M-120, Zählprüfungen vor dem Wechsel unvollständig.** FeatureEngine: B5-Datei:2410-2411 prüft die Samplezahl, `nak380VerlustGeprueft` fehlt (Muster :1629-1630). M1: das Lambda :2459-2470 zählt `i`, gibt es aber nicht zurück; weder 30 s bei 48 kHz (:2471) noch der Lauf nach dem Wechsel (:2500) hat eine Samplezahlprüfung. Wurzel W2. **DEFEKT** (Auftrag E6:26: „in jedem Läufer“). Regel: der FeatureEngine-Lauf prüft zusätzlich Verlust 0; der M1-Lauf liefert die gespeiste Zahl und prüft sie gegen Signallänge und Enginezahl.
- **Z4 — B5 M-120, Zusage-Element „Vorframe“ ohne Prüfzeile.** Matrix Manifest:1041 „kein Akku, Ring, Vorframe oder Histogramm der alten Rate überlebt“ und Auftrag E6:23 nennen den Vorframe; B5-Datei:2423-2441 prüft Stufenringe, Band-Akkus, `verarbeitet` und Detektorhistorie, nicht den Vorframe, obwohl der Testzugang ihn liest (`vorframeMaxDb`/`vorframeMinDb`, B5-Datei:128-138). §54.7 führt den Satz als „kein Akku, keine Historie, kein Samplezaehler“ (Manifest:3083). Zwei Träger: Neuanlage `d.vorframe.assign ((std::size_t) d.binAnzahl, kFlussP0Db);` (`FeatureEngine.h:734`) und `detektorLeeren()` in `zuruecksetzen` (`FeatureEngine.h:829`). Das LRA-Histogramm der FeatureEngine (`FeatureEngine.h:680`, geleert :807) ist ebenfalls ungeprüft, sofern „Histogramm“ die FeatureEngine einschließt (die M1-Histogramme prüft :2487-2494); der Stereoring ist in B19 `380/M-87 rate_change` mit Doppelmutation getragen (`Sonde013StereoGoldenTest.cpp:1477-1490`). Wurzel W1/W3. **DEFEKT** (Zusageelement ohne Prüfung und Rotbeweis; Manifest:75-77, :788-789). Regel: M-120 prüft nach dem Wechsel den Vorframe (und nach Entscheid des Dirigenten das FeatureEngine-Histogramm) mit Doppelmutation Neuanlage und `zuruecksetzen` samt Gegenproben (Auftrag E6:24).
- **Z5 — B5 M-115, Satz „Sperrzeit und Historie in Zeit“ ohne Prüfzeile.** Matrix Manifest:1036 „Hop 42,67 ms, Sperrzeit und Historie in Zeit wie bei 48 kHz“; B5-Datei:2348-2350 prüft K = 1530 und Hop, nicht Sperrzeit und Historie (§54.7 Manifest:3073). Die Sperrzeit rechnet in ms·fs (`featureengine/Spektrum.h:851-853`, seit Etappe 4 unverändert), die Historie folgt aus 32 Frames mal geprüftem Hop. Wurzel W3. **HÄRTUNG**. Regel: optional eine Prüfung Sperrzeit und Historie in ms je Rate.

Hinweise mit anderer Wurzel (keine Wertung, Kommentarwortlaut nach Prüfauftrag:35): `FeatureEngine.h:725-727` nennt noch „96 kHz 766, 58, 118 (NAK-380 M-43, R-380-12 (i))“; seit R-380-14 (i) gilt 1532/115/232. Der Docstring B5-Datei:2394-2396 ist unter D1 zitiert.

## Übersicht

| Befund | Urteil | Einordnung | schließende Regel in einem Satz | Folgestellen |
|---|---|---|---|---|
| D1 M-120 Dauer nach dem Wechsel | PRÄZISIERT | DEFEKT | Nach `vorbereiten (96000)` 30 s selbstgeprüftes P2 (2 880 000 Samples) in beide Engines mit gespeist = verarbeitet und Verlust 0 als eigene Prüfungen und Prüfung „nur Frames der neuen Länge“. | B5-Datei `nak380M120` :2384-2451, `nak380M120M1` :2453-2511, Docstring :2394-2396; `NAK-380-rot-M-120.txt`; §54.7-Nachfolge zu Manifest:3081-3086 |
| D2 B15 ohne K4 | PRÄZISIERT | DEFEKT (48 kHz), LÜCKE (44,1 kHz) | B15 fährt bei 48 kHz K4 30 s mit `kSelbstpruefung` und verriegelt „keine Karte“ und Leertext; das 44,1-kHz-Signal entscheidet der Dirigent. | B15-Datei :2507-2600; `NAK-380-rot-M-106.txt`; §54.7-Nachfolge zu Manifest:3058; ggf. §7.2 K-Zeile |
| D3 B15 Restblock | BESTÄTIGT | DEFEKT | B15 speist den Restblock und verriegelt gespeist = Signallänge, Drops 0 und Enginezahl = gespeist − Quarantäneblock als eigene Prüfungen. | B15-Datei :2543-2553, :2598-2599; `NAK-380-rot-M-106.txt`; B15-Behauptung in `tools/beweise.ps1` bei Zählerstand |
| D4 M-110 Verbrauch | PRÄZISIERT | DEFEKT | M-110 verriegelt gespeist 960 000, verarbeitet 959 488 (Quarantäne hergeleitet) und alle Dropzähler 0 als eigene Prüfungen. | B5-Datei `nak380M110` :2513-2591; `NAK-380-rot-M-110.txt`; §54.7-Nachfolge zu Manifest:3065 |
| D5 NaN bei unbekannter Fassung | PRÄZISIERT | DEFEKT (ohne Produktwirkung) | A4 prüft `beurteile_mit_messfassung` mit `None` direkt auf `material_cosine` NaN, Rotbeweis NaN → 0.0 an `vergleichbarkeit.rs:381`. | `experiment_verdrahtung.rs` :2307-2341 oder `telemetrie.rs` :1257-1353; `NAK-380-rot-M-122.txt`; neues e6n0-Protokoll; §56.5 Manifest:3614 |
| Z1 A2 M-106 K4 Enginezahl | – | DEFEKT | K4-Vorbedingung prüft gespeist = verarbeitet = 1 440 000. | A2-Datei :1268-1270 |
| Z2 A2 M-106 Karte 60 Hz | – | DEFEKT | Der 60-Hz-Lauf prüft gespeist, verarbeitet und 61 Snapshots wie die M-107-Gegenprobe. | A2-Datei :1277-1298 |
| Z3 B5 M-120 Zählprüfungen | – | DEFEKT | FeatureEngine-Lauf mit Verlust 0, M1-Lauf mit gespeister und verarbeiteter Samplezahl. | B5-Datei :2410-2411, :2459-2471, :2500 |
| Z4 B5 M-120 Vorframe | – | DEFEKT | M-120 prüft den Vorframe (und nach Entscheid das FeatureEngine-Histogramm) mit Doppelmutation Neuanlage und `zuruecksetzen`. | B5-Datei :2423-2441; `NAK-380-rot-M-120.txt`; §54.7-Nachfolge zu Manifest:3083 |
| Z5 B5 M-115 Sperrzeit und Historie | – | HÄRTUNG | Optional Sperrzeit und Historie in ms je Rate prüfen. | B5-Datei :2345-2350 |

## Nicht geprüft

- Kein Lauf: weder Bau noch Test, keine Mutation gefahren; die Mutation des Prüfers zu D5 (NaN → 0.0) ist nur am Code nachgerechnet.
- Ob K4 am echten Gen-Editor eine leere Befundliste ergibt, ist an HEAD nirgends gemessen und hier nicht feststellbar; entsteht dort eine Karte anderer Klasse, widerspricht das Verhalten der Zusage „keine Karte; Leertext“ (dann Entscheid des Dirigenten).
- Ob `rosaSelbstpruefung` auf 96 000 Samples bei 96 kHz bestünde, und ob in B15 oder M-110 tatsächlich Drops auftreten (Queue-Kapazität nicht nachgerechnet).
- B19-Änderungen (`Sonde013StereoGoldenTest.cpp`), A5 (`pruefe_v3_vertrag.py`), B18-Nacharbeit, `tools/beweise.ps1` und die Produktänderungen nur soweit oben zitiert; Matrixzeilen M-98 bis M-122 nur soweit Teil 2 sie verlangt.
- Rohdateien außer `NAK-380-rot-M-106.txt`, `NAK-380-rot-M-120.txt` und den zwei e6n0-Protokollen `m122-riegel-unbekannt-verg` und `-inv` nicht gelesen; §54 und §56 als Selbstbericht gelesen, nicht nachgemessen.
- Kanon-Anhang im Manifest, Installer-Pin und Bauartefakte nicht gelesen und nicht angefasst.

FERTIG Validierung Erstprüfung 6, 1 bestätigt, 4 präzisiert, 0 widerlegt, 5 weitere Stellen
