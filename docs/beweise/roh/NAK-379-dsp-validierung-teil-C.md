# NAK-379 — Quellvalidierung DSP-Fachprüfung Teil C (DSP-17 bis DSP-25)

**HEAD vorher:** `3da624e0a313fea92304c7ef70f1cd71e2604258` (23.09.2026, 18:14:04 +0200, `git rev-parse HEAD`)
**HEAD nachher:** `3da624e0a313fea92304c7ef70f1cd71e2604258` (23.09.2026, 18:42:31 +0200, nach dem Schreiben dieser Datei; unverändert)
**Arbeitsmodus:** frischer, lesender Validierer, Opus 5.5, Effort max, Zweig `master`, sichtbarer Checkout.
Git nur lesend; kein Bau, kein Kanon, kein Test, kein FL Studio, keine Pipe. Die Skripte unter
`docs/audits/2026-09-23-dsp-fachpruefung/skripte/` und ihre `.out`-Dateien gelesen, **nicht ausgeführt**.
Eigene Nachrechnungen mit `py -3.13` in eigenen Skripten im Scratchpad der Session (außerhalb des Repos,
flüchtig; Rechenweg je Befund unten so angegeben, dass er nachvollziehbar ist). Eine externe Primärquelle
abgerufen: EBU Tech 3342 V4 (tech.ebu.ch, 23.09.2026) für DSP-25.
**Prüfgegenstand:** `docs/audits/2026-09-23-dsp-fachpruefung/BEFUNDE.md:102-110` (§3 Befunde am Messkern).
**Stand der Zitate:** Der Bericht rechnete am HEAD `4a831da3`. Zwischen `4a831da3` und `3da624e0` hat sich
nur `docs/offene-punkte.md` geändert (Zeilen NAK-310, NAK-378, NAK-379; `git diff --stat`); alle zitierten
Code-, Entwurfs-, Manifest- und Abnahmezeilen sind am HEAD dieselben und wurden dort selbst nachgelesen.

Klassen wie im Auftrag: DEFEKT · LÜCKE · HÄRTUNG · PRODUKTFRAGE · DOKU. „Ruhend" = heute für den
Musiker nicht erreichbar, am Code gezeigt.

---

## DSP-17 — Leistungsdichten als Bandleistungen verrechnet

**1. Quelle.**
- `eq-copilot/plugin/core/analysis/featureengine/Spektrum.h:426-429`: `norm = 2/(fs·fensterEnergie)`,
  `psd[k] = (|M|²+|S|²)·norm` — einseitige Dichte. Stimmt. `:488-495`: Bandwert = **Mittel** der
  Bin-Dichten (`energie /= bis − von`). Stimmt. Der Code sagt es selbst (`:443-445`): „Die Integration
  steht am GATE, nicht an `s.psd`: Bandakkumulation, Fluss und Stereo lesen die Dichte unveraendert
  weiter."
- `broker/src/coordinator/hypothese/zusammenhang.rs:185-212`: Doku „Der Anteil seiner LINEAREN Leistung
  im Bandintervall an seiner Gesamtleistung" (`:187-188`), gerechnet wird Σ 10^(P50/10) über alle
  Fenster und gültigen Feinbänder, ohne Bandbreite. Stimmt.
- `broker/src/coordinator/hypothese/screening.rs:144-168`: `gruppenenergie: bandpassung(...)` (`:148`),
  Quantisierung `:161-168`. Stimmt.
- „Confidence summiert Dichten" ist verkürzt: `confidence.score = rang.rang()` (`befund.rs:321`) ist das
  Mittel der sechs quantisierten Rangkomponenten (`zusammenhang.rs:64-70`); die Dichte wirkt über die
  Komponente `bandpassung`.
- `eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs:184-206`: `Bandwerte` ohne physikalische
  Größe; der JSON-Zwilling `eq-ipc-v3.schema.json:423-509` nennt nur „dB" und die Kodierung. „Der
  Vertrag nennt keine Einheit" stimmt.
- Livegruppe (im Befund nicht zitiert, nur im Skriptkopf): `featureengine/Frame.h:337-351` summiert die
  Feinband-Mittel der belegten Feinbänder.
- Weitere Leser derselben Dichtewerte, vom Bericht nicht genannt, hier nicht bewertet:
  `hypothese/rechnung.rs:361-375` (`masteranomalie`, Befundband als größte Abweichung vom Median der
  P50-dB) und `maskierung.rs:133-152`.
- **Bekannt · Ort „neu"** stimmt: kein Registerpunkt zu Dichte gegen Bandleistung (gesucht in
  `docs/offene-punkte.md` nach „Dichte", „dBFS/Hz").

**2. Formel und Zahl.**
- `skripte/gegenpruefung-pegel/b3_b6_baender.py` ist eine getreue Abschrift von `Zeit.h:120-145`
  (ceil-Kanten, Kappe min(18 kHz, 0,95·fs/2), Trennung über Bandmitte ≥ 200 Hz, 16384/4096), des
  Bandmittels, der Livegruppensumme und von `bandpassung`. Abweichung: Teil B3b rechnet über alle
  221 Bänder, obwohl bei 48 kHz 25 Bänder keinen Bin haben und in `gesamt` fehlen; die Verzerrung des
  P50 (Median über Einzelfenster) bei Bändern mit wenigen Bins ist nicht modelliert.
- Eigene Rechnung (Gitter aus `schemas/v3/bandgitter/*.json`, eigene Zuordnung nach `Zeit.h:120-145`):
  rosa Rauschen, Anteil 8-16 kHz über 221 Bänder **0,190 % statt 10,86 %** (Bericht 0,19/10,9 %:
  bestätigt); nur über die bei 48 kHz messbaren Bänder 0,30 % statt 12,2 % (Faktor 40 statt 57,
  Richtung und Größenordnung gleich). Rangumkehr A/B: **0,0067 / 0,1383** (bestätigt). Weißrauschen,
  Livegruppen: Spanne **6,02 dB** bei 48, 96 und 192 kHz (Gruppen mit 1 gegen 4 belegten Feinbändern;
  bestätigt). Maskierung: im Befundintervall (3 bis 4 Feinbänder, `befund.rs:343` übergibt es)
  ≤ 0,05 dB bei 6 dB/Okt Neigungsunterschied, über eine Oktave 0,34 dB, über zwei 1,3 dB —
  „kaum betroffen" gilt für das tatsächlich übergebene schmale Intervall.

**3. Zusagenlage.** Tragend ist der Entwurf, nicht nur die Code-Doku:
Entwurf §35.1 `:2273` „64 Live-Bandenergien und bei Bedarf 221 Evidenzbänder" und `:2285-2286`
„Leistungen werden erst linear integriert und danach logarithmiert; `dBFS/Hz` und integrierte
`dBFS`-Bandleistung bleiben unterschiedliche Einheiten." Dazu §36.2 `:2382` „Quellenergie",
SONDE-014 M-18 (`docs/beweise/SONDE-014.md:737`) „Quellenergie", `docs/beweise/NAK-213.md:99`
„`gruppenenergie` ist ein Leistungsanteil in [0, 1]". Die Einordnung „Defekt (Verbraucher) + Lücke
(Vertrag)" ist richtig; der Bericht nennt den tragenden Entwurfssatz nicht.
„Ruhend" behauptet der Bericht nicht, das Register (NAK-379) zählt DSP-17 aber zu den „ruhenden
Defekten". Am Code: Die Rechnung läuft in jeder Gen-Sitzung mit Probeeq-Evidenz (Intent-Vollbestand
nach Verbindungsaufbau `eq-copilot/plugin/src/prozessor/Ipc.cpp:1059`, Sperre nur ohne Marke
`hypothese_verdrahtung.rs:218-230`). Jeder Überlebende erhält einen eigenen Befund
(`rechnung.rs:259-280`), sichtbar ist nur „Findings: N open" je Quelle
(`SourcesModel.cpp:1781-1799`, `PluginEditor.cpp:1529-1531`); Reihenfolge und Klasse „hoch" zeigt
keine Fläche, Vorschläge dekodiert das Plugin nicht (DSP-14). Sichtbar wird DSP-17 heute nur über den
Screening-Deckel ab sechs Kandidaten (`screening_ueberboten`).

**4. Urteil: PRÄZISIERT.** Klassenvorschlag **DEFEKT** (Entwurf `:2285-2286`) **+ LÜCKE** (Vertrag ohne
Größe). **Die Spalte „Wirkung" ist in der Richtung vertauscht:** Die Dichte gewichtet Leistung mit
1/Bandbreite, also etwa mit 1/f; im eigenen Beispiel des Berichts verliert gerade die basslastige
Quelle A (40 % im Höhenband, 0,0067) gegen B (30 %, 0,138). Richtig: „Bei Höhenbefunden rutscht eine
Spur, die neben Höhen auch Bass hat, hinter eine basslose Spur mit weniger Höhenenergie; bei
Bassbefunden erreicht fast jede Spur mit Bassanteil eine Bandpassung nahe 1." Bandpassung ist eine von
sechs gemittelten Komponenten. Heute erreichbar: Rechnung ja, für den Musiker nur mittelbar (Zähler ab
sechs Kandidaten). Kleinster Fix: in `bandpassung` jeden Summanden mit der Bandbreite aus dem
eingefrorenen Gitter gewichten (Broker, ohne Vertragsänderung; das Screening erbt es); die
Livegruppe `Frame.h:343-347` auf Σ Dichte·Bandbreite; die Größe im Vertrag benennen (mit Versionsschritt
`metrics_version`). Planort: neues Ticket in der Messkette des Ursachenfinders (Nähe S25o), vor jeder
sichtbaren Ursachenanzeige.

---

## DSP-18 — Der Vergleichspegel ist kein Lautheitsabgleich

**1. Quelle.**
- `eq-copilot/plugin/core/analysis/Vergleichspegel.h:205-235`: `friereEin` rechnet
  `20·log10(rmsB/rmsA)`, ungewichtet, ohne Deckel. `:371-412`: `nimmFrames` summiert x² und y²
  ungewichtet. Stimmt.
- `eq-copilot/plugin/src/PluginProcessor.cpp:943-972`: `versuchTrocken` ist der Puffer vor
  `markierung.verarbeite` (`:944-950`), `nass` derselbe Puffer danach (`:970`); der Kommentar
  `:885-888` sagt es ausdrücklich. Stimmt.
- „Ohne aktive Markierung exakt 0 dB": `src/HoerMarkierung.h:541-561` lässt den Puffer unangetastet
  (bit-transparent) → Summe A = Summe B → 20·log10(1) = 0. Stimmt.
- „hebt die Sperre auf": `core/analysis/Blindvergleich.h:91-98` übernimmt jeden eingefrorenen, gesetzten
  Gain; `:124-133` sperrt `ohneLautheitsabgleich` nur ohne Gain. `src/prozessor/Analyse.cpp:833-863`
  friert beim Versuchsbeginn ein, `:724-727` schickt den Wert als `match_gain_db`. Stimmt.
- „nie auf Audio angewandt": einzige Leser `Analyse.cpp:725`, `:1071`, `:1083`, `:1086` (JSON, Abfrage).
  Stimmt.
- Entwurf `:1117` und `:2719-2736` stimmen (Wortlaut unten). Abnahme
  `design/abnahmen/2026-08-24-auto-gain-schalter.md:56` stimmt: Zeile der Tabelle „3. Was daraus
  folgt", Claude-Text („Das Vorhören gleicht die Lautheit schon heute eigenständig ab (Match-Gain §41,
  eingefrorener Vergleichspegel)"), kein User-Zitat, also kein Entscheid.
- „Ruhend" belegt: `beginneVersuch`, `erfasseKandidat`, `urteileVersuch` und
  `assistentVersuchStarten` haben außer `tests/Sonde013PassageStateTest.cpp` und
  `tests/Sonde014AssistentTest.cpp` keinen Aufrufer (`git grep` über `eq-copilot`, `broker`, `tools`).
- **Bekannt · Ort „neu"** stimmt: die Registerpunkte zum Vergleichspegel (NAK-159, NAK-246 D7,
  `offene-punkte.md:185`) betreffen Takt und Nebenläufigkeit, nicht die gemessene Größe.

**2. Formel und Zahl.** `skripte/gegenpruefung-pegel/b9_abgleich.py`: Teil C ist die Produktformel
(trocken gegen Hörmarkierung), Teil A/B die hypothetische Anwendung eines ungewichteten Abgleichs auf
Vorher/Nachher; ITU-Koeffizienten (Tab. 1/2, 48 kHz) und RBJ-Low-Shelf korrekt. Eigene Rechnung
(Frequenzbereich 20 Hz bis 20 kHz, K-Gewichtung Tab. 1/2, RBJ-Low-Shelf S = 1): Spektrum −4,5 dB/Okt,
Low-Shelf −6 dB bei 120 Hz: ungewichtet −2,79 dB, K-gewichtet −1,23 LU → Kandidat nach
RMS-Abgleich **+1,56 LU** (bestätigt). Die Zahl hängt an Spektrum und Eckfrequenz, die der Befund
nicht nennt: rosa +0,72 LU, braun +1,69 LU; Low-Shelf +4 dB bei 80 Hz −0,80 / −1,31 / −1,15 LU.

**3. Zusagenlage.** Entwurf §15 `:1117` „Lautheitsabgleich vor einer Klangwertung"; §41.2
`:2721-2730` „Ab 10 s verwendet sie gated LUFS-I, zwischen 7 und 10 s den robusten Median von
mindestens fünf gültigen LUFS-S-Fenstern … Der automatische Ausgleich ist produktseitig auf ±6 dB
begrenzt"; §43.1 `:2857` „eingefrorener Match-Gain". SONDE-013 M-43 (`SONDE-013.md:533`) „Eine
Klangwertung ohne vorherigen Lautheitsabgleich ist unzulässig" legt weder die verglichenen Signale noch
Gewichtung oder Deckel fest → Matrixlücke, wie berichtet. §41.2 steht unter „Hörbarer Beweis", ist
aber die einzige Definition des Match-Gain; §15 trägt allein: ein Wert, der ohne Markierung immer 0 dB
ist und nie Vorher gegen Nachher misst, ist kein Lautheitsabgleich der zu wertenden Varianten.

**4. Urteil: BESTÄTIGT** (die 1,56 LU gelten für −4,5 dB/Okt und 120 Hz). Klassenvorschlag **DEFEKT
(ruhend)**. Wirkung stimmt. Heute nicht erreichbar. Kleinster Fix: die Sperre
`ohneLautheitsabgleich` nur durch einen Match-Gain aus der Lautheit von Vorher und Nachher derselben
Passage lösen (K-gewichtet nach §41.2, Deckel ±6 dB, sonst sichtbar „nicht pegelangepasst"); den
heutigen Wert trocken gegen Hörmarkierung, falls gebraucht, als eigene Größe führen. Doku: Tabellenzeile
`:56` als überholten Claude-Text im Register vermerken, das Abnahmeblatt nicht umschreiben. Planort:
neues Ticket vor der ersten UI-Verdrahtung von `beginneVersuch`/`assistentVersuchStarten`,
spätestens mit S29–31 („aktiver Vergleich").

---

## DSP-19 — Die Experiment-Statistik ist zu optimistisch

**1. Quelle.**
- `broker/src/coordinator/experiment_verdrahtung.rs:1009-1034`: die Baseline geht nur als Bandmittel
  ein (`mittel_je_band`, `:974-1007`), die Reihe ist Resultatfenster minus dieses feste Mittel
  (`:1028`). Stimmt.
- `broker/src/coordinator/experiment.rs:378-416`: Fenstermittel → `block_bootstrap(…, 4, 400, 0.05, 42)`,
  je Band `bootstrap_p`. `:1530-1531`: `bl = min(4, n)`, `bloecke = n − bl + 1`. `:1572-1607`:
  zweiseitiger p mit +1/+1, p_min = 2/401. Achse Zielmetrik `:481-495`: Änderung bei ≥ 1 FDR-Band und
  Intervall ohne 0. Stimmt.
- `eq-copilot/plugin/tests/Sonde013ExperimentGoldenTest.cpp:144-146`: „eine Blocklaenge ueber der
  Folgenlaenge klemmt auf sie - das Intervall wird dann zum Mittelwert selbst" als Soll. Stimmt. Der
  Golden prüft den C++-Spiegel `core/analysis/Statistik.h:70` (im Produkt ohne Aufrufer), der laut `:117`
  bitgleich zur Rust-Fassung rechnet.
- „Keine Mindestfensterzahl": `GATE_MINDEST_FENSTER = 8` (`vergleichbarkeit.rs:196`) wirkt nur im
  Hypothesenpfad (`befund.rs:244`, `screening.rs:308`); `vergleichbarkeit::beurteile` (`:316-378`)
  prüft keine Fensterzahl. Stimmt.
- „Kein UI-Aufrufer": stimmt (siehe DSP-18).
- „Das Urteil steht wegen der Guardrails auf vergleich_nicht_gueltig" gilt **nur bedingt.** Loudness
  und Peak sind immer „nicht gemessen" (`experiment_verdrahtung.rs:1112-1119`), die Achse ist aber nur
  `NichtBeurteilbar`, wenn **keine** gemessene Guardrail reißt (`experiment.rs:531-542`). Reißt
  Transient, Breite, Abdeckung, Klasse oder „geschützt" (größtes |Δ| außerhalb des geratenen Ziels über
  rund 195 Bänder gegen 1,0 dB; `:580`, `experiment_verdrahtung.rs:1152-1184`), steht die Achse auf
  `Verschlechtert`, und das Urteil wird „messbar anders" oder „Ziel verbessert, aber geschützt
  schlechter" (`experiment.rs:1454-1465`). Eigene Abschätzung (unabhängige Bänder, n_b = n_r = 8):
  „geschützt" reißt unter der Nullhypothese bei einer Streuung je Fenster und Band von 0,5 dB in 1,2 %,
  von 0,75 dB in 78 %, ab 1 dB in rund 100 %. Die Unsichtbarkeit trägt heute allein der fehlende
  UI-Aufrufer.
- **Bekannt · Ort** stimmt: T3-06-03 (`docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md:425`, `:532`) ist
  die Gegenseite (Auflösung verfehlt schmale starke Änderungen); NAK-316 (`offene-punkte.md:320`) und
  S25o (`plan.json`, Karte „Erkennungs- und Fehlalarmgrenze") existieren; die Fehlalarmseite ist dort
  nicht beschrieben.

**2. Formel und Zahl.** `skripte/gegenpruefung-statistik/b1_kern.py` ist zeilengetreu: LCG
(`Wuerfel::neu` = Saat·M + 1, `naechste` = ·M + INC, `>> 33` mod Blöcke), Blockziehung, Perzentilränge
10 und 390, p-Wert, BH-FDR, Achse. Die Zahlen 49/28/22 % stammen aus `b1_intervall.out`
(E, n_b = n_r: 49,4/27,8/21,6 %), nicht aus `b1_null.out` (47,7/25,3/23,0 %). Eigene Rechnung (eigene
Umsetzung der Ziehung, 40 000 Läufe je n): **48,9 / 28,1 / 21,6 %** (bestätigt); Grenzwert
2·(1 − Φ(1,96/√2)) = **16,6 %** (bestätigt); bis n = 4 genau **ein** Ziehungsmuster (bestätigt;
zum Vergleich n = 5: 4 Muster, n = 8: 15). Zusätzlich: selbst mit exakt bekannter Baseline verfehlt das Intervall die 0 in
36,2/14,9/9,5 % — der Perzentil-Blockbootstrap ist bei kleinem n schon für sich antikonservativ. „Auf
Produktebene bei n = 8 etwa 48 %" ist `b1_null.out` Spalte „Alarm" (47,7 %) und meint die Achse
Zielmetrik (Intervall plus FDR), nicht das Endurteil.

**3. Zusagenlage.** Entwurf §43.2 `:2893-2895` „Metrikdeltas erhalten per Block-Bootstrap ein
Unsicherheitsintervall. Für 221 gleichzeitig gescannte Bänder werden zusammenhängende Cluster
beziehungsweise FDR-Korrektur verwendet. Bei anderem Material oder unzureichender Abdeckung gibt es kein
Siegerlabel." SONDE-013 M-45 (`SONDE-013.md:535`) wortgleich. Ein Intervall des Deltas ohne die
Unsicherheit der Baseline und eine FDR-Korrektur, die bis n = 4 jedes Band (p = 2/401) und bei n ≤ 12
in 100 % der Nullläufe mindestens ein Band signifikant macht (`b1_null.out`), erfüllen die Sätze nur
dem Wortlaut nach; `experiment.rs:1667-1672` nennt die Fehlalarmkontrolle selbst als Zweck. Einordnung
„Defekt, ruhend" richtig.

**4. Urteil: PRÄZISIERT** (Zahlenquelle; „Produktebene" = Achse Zielmetrik; der Guardrail-Schutz hält
nur, solange keine gemessene Guardrail reißt). Klassenvorschlag **DEFEKT (ruhend)**. Wirkung stimmt
(die Zeiten passen zu Evidenzfenstern von rund 0,32 s). Heute nicht erreichbar. Kleinster Fix:
Unsicherheit beider Seiten (Resampling von Baseline- und Resultatfenstern oder Zweistichproben-
Bootstrap), Mindestfensterzahl je Seite (mindestens zwei Blöcke), Golden `:144-146` umkehren
(degenerierter Fall = nicht beurteilbar), Nullkorpus mit Fehlalarmrate als Beweis. Planort: S25o /
NAK-316, dort liegen T3-06-03 und die Karte „Erkennungs- und Fehlalarmgrenze".

---

## DSP-20 — Auflösung fest statt samplerateabhängig

**1. Quelle.** `core/analysis/FeatureEngine.h:376-383` (16384/4096 fest, Trennung 200 Hz) stimmt. Für
M1 tragend, im Befund nicht zitiert: `src/AnalyseEngine.cpp:210-213` (16384/8192/4096/2048 fest).
`:596-644` stimmt: Lücken ohne Bin werden in Leistung linear interpoliert, Ränder geklemmt, und gehen
so in die Kompositkurve (`:662-688`). `:1005-1013` stimmt: Breite = zusammenhängender Bereich
≥ Spitze − 6 dB, über 1/6 Oktave verworfen (`kBreiteMaxOktaven`, `:23`). „Die Oberfläche sagt nicht,
dass dort nicht gesucht wird": Die Abdeckungsklasse (`:929-962`) misst den Pegelanteil über dem
Teppich je Drittel-Oktave, nicht die Suchfähigkeit. Stimmt. **Bekannt · Ort „neu"** stimmt.

**2. Formel und Zahl.** `skripte/gegenpruefung-pegel/b6_m1_resonanz.py`/`b6_m1_voll.py` sind getreu:
Kanten 30·2^(b/24) wie `AnalyseEngine.cpp:180-182`, Bin-Fenster `:91-99`, Komposit mit Nähten
`:662-688`, Median ±6 mit ≥ 5 endlichen Werten `:41-62`, Persistenz aus der zuständigen Stufe. Nicht
modelliert: Abdeckungs-Gate `:1027-1032` und Persistenzanteil (für einen stationären Sinus
unerheblich). Eigene Rechnung (Sinus im Hann-Fenster numerisch per FFT statt geschlossener Form, plus
rosa Erwartungsdichte, +15 dB; feines Frequenzraster):
- FeatureEngine, Bänder ohne Bin: 44,1 kHz 22, 48 kHz 25, **96 kHz 60, 192 kHz 94** (bestätigt).
- M1 lückenlos erst ab: 44,1 kHz ≈ 43 Hz, **48 kHz ≈ 47 Hz** (bestätigt „≈ 45 Hz"), **96 kHz
  ≈ 93 Hz** (bestätigt „≈ 100 Hz"), **zusätzlich bei 96 kHz eine Lücke 203-374 Hz** (Mittenstufe
  4096: 23,4 Hz Raster, −6-dB-Breite des Hann-Hauptlappens von 2 Bins liegt bis ≈ 400 Hz über 1/6 Okt),
  im Bericht nicht genannt; **192 kHz erst ab ≈ 746 Hz** (Bericht „≈ 500 Hz"; Überschlag: 2 Bins
  = 93,8 Hz entsprechen 1/6 Okt bei ≈ 810 Hz).

**3. Zusagenlage.** Die geltenden Texte schweigen zur Skalierung: Entwurf §35.1 `:2278` „Die
vorhandenen FFT-Größen 16.384/8.192/4.096/2.048 bleiben sinnvoll" (in Samples). Der nicht bindende
M1-Vorläufer `docs/archiv/FL-EQ-Copilot-Recherche.md:591-599` (Verlauf, keine Vorgabe) definierte das
Bassfenster in Zeit („≥ 170 ms") und verlangte „die UI zeigt nie mehr Präzision an, als das Fenster
hergibt" — Hinweis für den Fix. LÜCKE ist richtig.

**4. Urteil: PRÄZISIERT** (96 kHz mit zweiter Lücke 200-375 Hz, 192 kHz Grenze ≈ 750 Hz).
Klassenvorschlag **LÜCKE**. Wirkung: stimmt für 48 kHz; bei 96 kHz fehlen zusätzlich tiefe Mitten
200-375 Hz, bei 192 kHz alles unter ≈ 750 Hz. Heute erreichbar: ja (Resonanzkarten der lokalen
Diagnose, `src/Diagnose.cpp:156-257`, `src/PluginEditor.cpp:333-339`), bei 96/192-kHz-Projekten und bei
jeder Rate unter ≈ 45 Hz. Kleinster Fix: FFT-Längen an fs koppeln (feste Fensterdauer, z. B. Bassstufe
≈ 341 ms) oder den nicht durchsuchten Bereich ehrlich als „nicht gesucht" melden und interpolierte
LTAS-Bänder kennzeichnen. Planort: neue Registerzeile oder Ticket Messkern.

---

## DSP-21 — Welch-Streuung als musikalische Schwankung (M1)

**1. Quelle.** `src/AnalyseEngine.cpp:500-503` (je Segment ein auf 1 dB gerundeter Bandpegel ins
Histogramm), `:801-836` (P10/P50/P95, Rang ceil(p·N)), `src/Diagnose.cpp:205-214` (P95 − P50 ≤ 6 →
„Der Pegel dort steht ruhig", sonst „kommt in Wellen"), `:223` (über 10 dB wechselt das Werkzeug). Alles
stimmt. **Bekannt · Ort „neu"** stimmt.

**2. Formel und Zahl.** `skripte/gegenpruefung-statistik/b5_welch.py` bildet Rundung, Rang und
Schwellen getreu nach; Einzelperiodogramm von Mono-Rauschen = χ²₂ je Bin. Eigene Rechnung
(geschlossen): P95 − P50 von χ²₂ = 10·log10(ln 20 / ln 2) = **6,357 dB**; nach 1-dB-Rundung beider
Quantile ist die ganzzahlige Differenz 7 für einen Anteil 0,357 der Pegellagen → **35,7 %** ohne
Stichprobenstreuung, Bericht 38,8 % (`b5_welch.out`): bestätigt. Übertreibung echter Bewegung: wahr
1,8/3,6/5,4 dB → gelesen 7/8/9 dB (`b5_modulation.out`), also 3,6 bis 5,2 dB: „etwa 5 dB" bestätigt.
1-Bin-Bänder bei 48 kHz 30-100 Hz und 200-400 Hz, 1 bis 2 Bins bis 800 Hz (`b5_m1bins.out`); unter
≈ 45 Hz entsteht keine Karte (DSP-20), daher „etwa 70-800 Hz" plausibel.

**3. Zusagenlage.** Kein geltender Satz zur Schätzstreuung der M1-Perzentile; die Regel „keine toten
UI-Elemente" (User 24.08.2026) betrifft tote Elemente, nicht Schätzfehler. LÜCKE richtig.

**4. Urteil: BESTÄTIGT.** Klassenvorschlag **LÜCKE** (kleine Textwirkung). Wirkung stimmt; die
Werkzeugempfehlung kippt nicht (10 dB werden nie erreicht). Heute erreichbar: ja. Kleinster Fix: die
Spanne über Welch-gemittelte Teilblöcke (z. B. je 8 Segmente) bilden oder die erwartete χ²-Spanne der
Binzahl abziehen. Planort: neue Registerzeile (Kalibrierung der M1-Texte).

---

## DSP-22 — Ereignisdetektor ohne absolute Schwelle

**1. Quelle.** `featureengine/Spektrum.h:585-608` (log10 der Bandmittel gegen den Vorframe, positive
Deltas summiert), `:635-642` (Median + 3 · mittlere Absolutabweichung um den Median), `:679-708`
(`fluss > schwelle && mad > 0`, keine absolute Schwelle, kein Maximumfilter, keine Spitzenwahl). Alles
stimmt; der Kommentar `:572-573` („SuperFlux-artiger spektraler Fluss") trifft nicht zu. Entwurf
`:2618-2620` stimmt. Guardrail: `coordinator/evidenz.rs:305-315` (onset = Σ `staerke_mad` der Ereignisse
je Evidenzfenster), Schwelle 2,0 (`experiment.rs:575`). **Bekannt · Ort „neu"** stimmt.

**2. Formel und Zahl.** `skripte/gegenpruefung-statistik/b11_fluss.py` bildet Hauptstufe, PSD-Norm,
Aktivgate, Log mit −30 bei 0, Historie 16, Median (s[7]+s[8])/2 und MAD als Mittel getreu nach; der
Peakpfad fehlt (bei stationären Signalen inaktiv, `:672-678`). Belege der Zahlen: Rauschen 13-24
(`b11_fluss.out`), pegelunabhängig 19/19/19 bei −20/−40/−50 dBFS (`b11_ursache.out`); „Synth-Ton 73-91"
= float32-Sägezahn 110 Hz 73 und float32-Sinus 440 Hz 91 (`b11_float32.out`); „7-27 %" = Guardrail bei
**4+4** Fenstern (`b11_guardrail.out`; 8+8: 1,1/11,8 %; 16+16: 0,1/2,0 %). Eigene Rechnung (eigene
Umsetzung, 30 s): Weißrauschen **13-20** (−20 dBFS), **16-24** (−40), **18-23** (−50) — bestätigt;
float32-Sinus 440 Hz **109** Auslösungen bei Amplitude 0,1/0,5/0,9 (Größenordnung bestätigt,
Einzelwert weicht ab). Die Varianten in `b11_fluss.out` zeigen, dass ein Maximumfilter allein die
Fehlauslösungen nicht senkt (Rauschen 18 → 23, Vibrato ±50 Cent 6 → 66): Hauptursache ist die rein
relative Schwelle, wie berichtet.

**3. Zusagenlage.** Entwurf §39.1 `:2618-2620` „Für den spektralen Fluss wird ein SuperFlux-artiger
Maximumfilter über Nachbarbins mit positivem Log-Magnitude-Delta und adaptiver Median/MAD-Schwelle
genutzt. Das reduziert Vibrato-Fehltrigger". Maximumfilter fehlt, gerechnet wird auf Bändern statt
Bins, MAD ist die mittlere statt der medianen Abweichung → Bruch am Wortlaut. Eine absolute Schwelle
verlangt der Entwurf nicht (seine Median/MAD-Schwelle ist selbst relativ) → die Hauptursache ist Lücke.
Einordnung „Abweichung + Schwellenlücke" richtig.

**4. Urteil: PRÄZISIERT** (Guardrail-Anteil gilt für 4+4 Fenster; der Weg „Guardrail reißt → DSP-19
sichtbar" führt auch und stärker über die Geschützt-Guardrail, siehe DSP-19; Sinus-Einzelzahl).
Klassenvorschlag **DEFEKT am Wortlaut §39.1 (ruhend) + LÜCKE** (absolute Schwelle). Heute erreichbar:
Ereignisse reisen in jeder Evidenz und speisen im Broker die Onset-Koinzidenz des Ursachenfinders
(`zusammenhang.rs:227-230`): Rechnung ja, Anzeige nein (wie DSP-17); die Guardrail wirkt nur im
Experiment (kein UI-Weg). Wirkung stimmt. Kleinster Fix: absolute Mindestschwelle für den Fluss
(Pegel- oder Rauschbezug), echte MAD, Spitzenwahl mit Sperrzeit; danach entscheiden, ob der
Maximumfilter kommt, und den Entwurfssatz erfüllen oder berichtigen. Planort: neue Registerzeile, Nähe
S25o, vor der ersten Anzeige von Ereignissen oder Experimenten.

---

## DSP-23 — Kohärenz über die Bins eines Bandes summiert

**1. Quelle.** `featureengine/Stereo.h:65-91` (Kreuz- und Autospektren über alle Bins eines Bandes
summiert, dann über Frames), `:225-244` (|ΣSxy|²/(ΣSxx·ΣSyy), Phase erst über 0,8). Stimmt. Mindestens
8 Frames `FeatureEngine.h:150-161`; Evidenzintervall standardmäßig 0,25 s (`:1344`), verdoppelt nur bei
Last (`sonde/SondeProcessor.cpp:997`, `:1096`). „Heute ohne Verbraucher" belegt: der Broker prüft nur
das Schema (`coordinator/schema.rs:140`), kein Leser in `broker/src/coordinator`, keiner in
`eq-copilot/plugin/src`. NAK-177 (`offene-punkte.md:203`) betrifft nur Bänder unter 200 Hz: **Bekannt ·
Ort** stimmt.

**2. Formel und Zahl.** `skripte/gegenpruefung-statistik/b10_kohaerenz.py` bildet `Stereo.h:65-91` und
`:229-230` getreu nach (M/S-FFT, L = M + S, R = M − S), dazu echte MSC je Bin und die Analytik. Eigene
Rechnung (geschlossen, |sin(Kθ/2)/(K·sin(θ/2))|² mit θ = 2π·Δf·τ, Δf = 11,72 Hz, τ = 1 ms): Band
15,77 kHz (39 Bins) **0,476** (Bericht 0,48), Band 8,85 kHz (22 Bins) **0,7996 < 0,8** (Bericht „ab
etwa 8,9 kHz"): bestätigt. Frames: `b10_frames.out` 44,1 kHz, 0,25 s, Blöcke 64-512: 6 bis 7 Frames,
nie 8 (bei 48 kHz erreichen 3 bis 50 % der Fenster 8 Frames): bestätigt für das Standardintervall.

**3. Zusagenlage.** Entwurf §40.1 `:2653` „Magnitude-Squared Coherence und Interchannel-Phase für
kohärente Signalanteile", `:2663-2666`; §40.3 `:2686-2687` „Goldens für Mono, identisches Stereo,
Polaritätsinvertierung, bekannte Laufzeit und unkorrelierte Kanäle klassifizieren korrekt über alle
unterstützten Blockgrößen." SONDE-013 M-11 (`SONDE-013.md:480`) prüfte die Phase nur an Trägertönen.
Eine bekannte Laufzeit von 1 ms fällt über ≈ 8,9 kHz unter die Phasenschwelle 0,8 (keine Phase, keine
Laufzeitaussage), und bei 44,1 kHz mit Blöcken ≤ 512 entsteht in der Hauptstufe gar keine Kohärenz →
§40.3 bricht. Einordnung „Abweichung (ruhend)" richtig.

**4. Urteil: BESTÄTIGT.** Klassenvorschlag **DEFEKT (ruhend)**. Wirkung stimmt. Heute nicht
erreichbar. Kleinster Fix: MSC je Bin über die Frames bilden und im Band mitteln (Phase je Bin bzw. als
Gruppenlaufzeit), Stereoakkus über mehr als ein Evidenzfenster führen oder ein eigenes Stereofenster
(gemeinsam mit NAK-177), Golden „bekannte Laufzeit" breitbandig. Planort: NAK-177 erweitern.

---

## DSP-24 — K-Gewichtung um −0,04 LU versetzt

**1. Quelle.** `core/analysis/KGewichtung.h:80-94`: RBJ-Hochpass mit Zähler (1+c)/2/a0, also b0 =
0,995044 bei 48 kHz. Stimmt. `plugin/tests/GoldenTestMain.cpp:433`: LUFS gegen `ref["lufs"]` aus
`fixtures/golden-referenz.json`, Toleranz 0,3 LU; die Referenz rechnet `analyze-track.py` mit pyloudnorm
(`docs/beweise/NAK-286.md:436`), und der Code sagt „wie pyloudnorm (RBJ, KEINE DeMan-Variante)"
(`src/AnalyseEngine.cpp:215-216`, `KGewichtung.h:56-59`) → zirkulär, stimmt. NAK-199
(`offene-punkte.md:219`) deckt die fehlende externe LUFS-Referenz, nicht den Versatz: „teilweise"
stimmt.

**2. Formel und Zahl.** `skripte/gegenpruefung-pegel/b7_kgewicht.py` schreibt `KGewichtung.h:63-94` und
Tab. 1/2 getreu ab und vergleicht andere Raten mit dem ITU-48-kHz-Gang in Hz. Eigene Rechnung
(Biquad-Frequenzgang direkt): b0 = **0,995044 (−0,0432 dB)**; 0-dBFS-Sinus 997 Hz in einem Kanal: ITU
**−3,010**, RBJ **−3,052 LKFS**; Versatz bei 997 Hz −0,042 (44,1 kHz), −0,041 (48), −0,039 (96),
−0,038 LU (192). Bestätigt.

**3. Zusagenlage.** Entwurf §39.1 `:2593` „Die Grundlage bleibt ITU-R BS.1770-5"; §39.3 `:2636-2637`
„Loudness und True Peak stimmen auf Standard-Testmaterial mit einer validierten Referenz innerhalb
±0,1 LU beziehungsweise ±0,1 dB überein" (auch `:3605`; SONDE-013 M-81 `:623`). −0,04 LU liegt
innerhalb: keine gebrochene Zusage. HÄRTUNG richtig.

**4. Urteil: BESTÄTIGT.** Klassenvorschlag **HÄRTUNG**. Wirkung stimmt. Heute erreichbar: ja (jede
LUFS-Zahl), unhörbar. Kleinster Fix: Hochpass mit Zähler [1, −2, 1] wie BS.1770 bzw. Ableitung je Rate
nach De Man/libebur128; externes Testset (EBU Tech 3341) als Referenz. Planort: NAK-199.

---

## DSP-25 — LRA mit 1 Hz statt mindestens 10 Hz

**1. Quelle.** `core/analysis/FeatureEngine.h:410-411`: `kLraHopZellen = 10`, Kommentar „EBU Tech 3342
§2.2". `featureengine/Lautheit.h:74-87`: ein Kurzzeitwert je 10 Zellen = 1 Hz. Stimmt. Primärquelle
EBU Tech 3342 V4 (November 2023; abgerufen 23.09.2026): Inhalt 1, 2, 3, 3.1, 4-7, **kein §2.2**;
§3.1 wörtlich „A minimum block overlap of 2.9 s between consecutive analysis windows (i.e. ≥10 Hz
sampling of the loudness level) is required; the exact amount of overlap is implementation-dependent.";
Dokumenthistorie „January 2016 · V3; Minimum calculation frequency at least 10 Hz". Bestätigt.
„Heute ohne Anzeige": `lra_lu` reist nur in der Telemetrie und wird im Broker auf Vorzeichen geprüft
(`broker/src/telemetrie.rs:608`, `:700-704`), kein Leser in `eq-copilot/plugin/src`. **Bekannt · Ort
„neu"** stimmt.

**2. Formel und Zahl.** `skripte/gegenpruefung-pegel/b8_lra.py`: Tech-3342-§5-MATLAB wörtlich,
Produktweg mit 0,1-LU-Histogramm, Bin-Mitten und 60-s-Regel getreu (`Lautheit.h:255-314`). Eigene
Rechnung: Tab. 1 Fall 4 bei 10 Hz und 1 Hz je **15,00 LU** (Histogrammbins exakt: floor 350 und 500).
Die Fälle 1 bis 3 dauern 40 s; der Produktweg gibt dafür erst nach Wiederholung einen Wert (60-s-Regel,
`Lautheit.h:259-261`; die Norm erlaubt Wiederholung). „p95 der größten Abweichung ≈ 0,9 LU" ist
modellabhängig: mein Modell (Pegelstufen 2 bis 10 s, Sprünge σ 6 dB, Zellrauschen 1,5 dB, 300 Läufe)
liefert p95 **2,5 LU** bei 60 s, 1,6 LU bei 120 s, 1,0 LU bei 300 s (Median 0,50/0,32/0,24 LU).

**3. Zusagenlage.** Entwurf §39.1 `:2594-2598` „Fensterdefinitionen … folgen EBU Tech 3341,
LRA-Algorithmus und Mindesttests EBU Tech 3342"; Tech 3342 §3.1 „≥10 Hz … is required". Der Entwurf
bindet den LRA-Algorithmus an eine Norm, deren Pflichtsatz der Code nicht erfüllt; M-04
(`SONDE-013.md:473`) schweigt zur Rate. Der Bericht ordnet „Härtung" ein — am Wortlaut ist es ein
ruhender Defekt mit kleiner Wirkung.

**4. Urteil: PRÄZISIERT** (Klasse; Abweichung modellabhängig und bei kurzen, sprunghaften Passagen
auch über 1 LU). Klassenvorschlag **DEFEKT (ruhend, geringe Wirkung)**. Wirkung stimmt. Heute nicht
erreichbar. Kleinster Fix: `kLraHopZellen = 1` (10 Hz; die 60-s-Regel zieht über `sekundenJeWert`
mit, `Lautheit.h:259`), Kommentar auf §3.1, Golden B18 `lra_below_sixty_seconds_is_not_a_number`
(„nach 75 s 18,9 LU") nachziehen, `kFeatureMetricsVersion` anheben. Planort: neue Registerzeile, kann in
S25o mitlaufen.

---

## Übersicht

| Befund | Urteil | Klassenvorschlag | tragender Satz oder „schweigt" | heute erreichbar | Planort |
|---|---|---|---|---|---|
| DSP-17 | PRÄZISIERT (Wirkung vertauscht) | DEFEKT + LÜCKE (Vertrag) | Entwurf :2285-2286 „`dBFS/Hz` und integrierte `dBFS`-Bandleistung bleiben unterschiedliche Einheiten"; :2273 „64 Live-Bandenergien" | Rechnung ja; sichtbar nur Findings-Zähler ab 6 Kandidaten | neues Ticket, Nähe S25o |
| DSP-18 | BESTÄTIGT | DEFEKT (ruhend) | Entwurf :1117 „Lautheitsabgleich vor einer Klangwertung"; :2730 „auf ±6 dB begrenzt" | nein | neues Ticket vor UI-Weg zu `beginneVersuch`, spätestens S29–31 |
| DSP-19 | PRÄZISIERT | DEFEKT (ruhend) | Entwurf :2893-2894 „Unsicherheitsintervall … FDR-Korrektur"; M-45 | nein | S25o / NAK-316 |
| DSP-20 | PRÄZISIERT (96 und 192 kHz) | LÜCKE | schweigt (:2278 FFT-Größen in Samples) | ja (96/192 kHz; unter 45 Hz immer) | neue Registerzeile |
| DSP-21 | BESTÄTIGT | LÜCKE | schweigt | ja | neue Registerzeile |
| DSP-22 | PRÄZISIERT | DEFEKT am Wortlaut (ruhend) + LÜCKE | Entwurf :2618-2619 „SuperFlux-artiger Maximumfilter … Median/MAD-Schwelle" | Rechnung ja (Koinzidenz), Anzeige nein | neue Registerzeile, Nähe S25o |
| DSP-23 | BESTÄTIGT | DEFEKT (ruhend) | Entwurf :2686-2687 „bekannte Laufzeit … klassifizieren korrekt über alle unterstützten Blockgrößen" | nein | NAK-177 erweitern |
| DSP-24 | BESTÄTIGT | HÄRTUNG | Entwurf :2636-2637 „±0,1 LU" (eingehalten) | ja, unhörbar | NAK-199 |
| DSP-25 | PRÄZISIERT (Klasse) | DEFEKT (ruhend, gering) | Entwurf :2597-2598 „LRA-Algorithmus … EBU Tech 3342"; Tech 3342 §3.1 „≥10 Hz … is required" | nein | neue Registerzeile |

## Querbezüge

- **Statistik- und Einheitenkomplex DSP-17/19/22** speist dieselbe Kette: DSP-17 verzerrt Bandpassung
  und Screening des Ursachenfinders, DSP-22 liefert die Onsetreihen der Koinzidenz und die
  Transient-Guardrail, DSP-19 die Unsicherheit der Experimente. Dieselbe Konstruktion wie in DSP-19
  (Reihe minus festes Vergleichsmittel, Block 4, `bootstrap_p_positiv`) steckt in der
  `wiederholbarkeit` des Ursachenfinders (`hypothese/zusammenhang.rs:329-352`); ein Fix an DSP-19
  sollte sie mitnehmen. Gemeinsamer Beweis: ein Nullkorpus mit gemessener Fehlalarmrate und ein Korpus
  mit bekannter Ursache. Der Schutz „Guardrails halten das Urteil auf vergleich_nicht_gueltig" hängt an
  DSP-22 und an der Geschützt-Guardrail (DSP-19, Punkt 1).
- **Zeitkonstanten in Samples statt Zeit:** DSP-20 (feste FFT-Längen), DSP-23 (8 Frames bei 44,1 kHz
  und 0,25 s nicht erreichbar; NAK-177 im Bass) und DSP-11 aus Teil B (Übergänge 256 Samples) gehören
  zur selben Klasse; DSP-20 erklärt zugleich die 6-dB-Streuung der Livegruppen in DSP-17 (leere
  Feinbänder) und die 1-Bin-Bänder, aus denen DSP-21 entsteht.
- **Testsemantik:** Der Golden zu DSP-19 (`:144-146`) schreibt den degenerierten Bootstrap als Soll fest,
  M-11 prüfte die Phase nur mit Tönen (DSP-23), die LUFS-Validierung ist zirkulär (DSP-24, NAK-199) —
  dieselbe Aussage wie DSP-10 aus Teil B: Goldens belegen Treue zur eigenen Formel, nicht die
  Messqualität.
- **Lautheit:** DSP-18 braucht für einen echten Abgleich die K-Gewichtung; der Versatz aus DSP-24 ist
  ab 100 Hz fast konstant (eigene Rechnung, 48 kHz: −0,038 bis −0,043 dB; bei 20 Hz +0,001 dB) und
  kürzt sich im Verhältnis Vorher/Nachher und in der LRA (DSP-25) bis auf Hundertstel heraus, betrifft
  also im Wesentlichen Absolutwerte.
- **Ruhend durch fehlende Anzeige:** DSP-17 und DSP-22 rechnen heute im Broker, bleiben aber für den
  Musiker unsichtbar, weil Befundreihenfolge, Klasse und Vorschläge keine Fläche haben (vgl. DSP-14
  aus Teil B). Das Register (NAK-379) nennt DSP-17 „ruhend"; am Code ist es „rechnet, nur mittelbar
  sichtbar".
- **M/S-Konvention:** FeatureEngine und Stereo rechnen (L±R)/2 (`Stereo.h:41-46`, `Spektrum.h:215-216`),
  wie DSP-16 aus Teil B für den EQ-Kern festhält; für DSP-17 und DSP-23 ohne Folge.

FERTIG Validierung DSP-Fachprüfung Teil C, 4 bestätigt, 5 präzisiert, 0 widerlegt.
