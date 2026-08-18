# Benchmark-Studie — RESO/EXPOSE · smart:EQ 4 · Pro-Q 4 als Mechanik-Spender

> **UI-Vorrang 2026-08-16:** Die DSP-Mechaniken und Messbegründungen dieser
> Studie bleiben gültig. Sichtbare Layer, Controls und Interaktionen werden
> dagegen abschließend durch `NAKAMA-SPECTRAL-FIELD-BAUPLAN.md` festgelegt.
> Insbesondere ersetzt der bewusste Schnappschuss einen zusätzlichen
> Freeze-/Max-Hold-Knopf. Das Hauptfenster arbeitet mit textfreien
> Werkzeugkreisen, überlagerbaren Problemsymbolen und einem ausschließlich
> manuell geöffneten Befundarchiv.

Stand 2026-08-15. Auftrag: die Analyse-/Feedback-Mechanik der drei
Industrie-Benchmarks dekonstruieren und daraus einen technischen Blueprint
ableiten. **Einordnung:** Das Zielprodukt („beratender EQ-Analyzer, der beim
Hören mitmisst und konkrete Hinweise gibt, ohne selbst einzugreifen") **ist der
gebaute EQ-Copilot** (kanonischer Plan `FL-EQ-Copilot-Recherche.md`, Kern-Stand
`M3-KERN-BEFUND.md`). Diese Studie ist deshalb kein Neuentwurf, sondern die
Vermessung der Benchmarks gegen unseren Kern — mit dem Ergebnis als
**M3a/M3b/M3c-Roadmap** auf der bestehenden Architektur (VST3-Sammler +
AnalyseEngine + Diagnose + Tauri-Broker).

Quellen-Disziplin: pro Benchmark ist getrennt, was **dokumentiert** ist
(Hersteller-Handbuch/-Blog, zitiert) und was **Engineering-Ableitung** aus dem
beobachtbaren Verhalten ist. Trainingswissen allein wurde nirgends als Fakt
verwendet (Recherche 2026-08-15, Quellenliste am Ende).

---

## Teil 1 — Wie die Benchmarks messen

### 1.1 Mastering The Mix RESO — dynamische Resonanzerkennung

**Dokumentiert (RESO-Manual):**

- Kern-Satz der Erkennung: *„RESO samples the input spectrum multiple times per
  second and creates a record of maximum frequency magnitudes that are
  statistically significantly higher than their neighbouring frequencies."*
  → Frame-weise Spektral-Abtastung (mehrmals pro Sekunde), pro Frame
  Peak-Prominenz **gegen die Nachbarschaft**, akkumuliert als **Statistik über
  die Hörzeit** — nicht ein Einzelbild.
- „Calculate Targets" erzeugt daraus Ziel-Knoten, die zeigen, *wie viel*
  Absenkung nötig ist, um die Resonanz „transparent" aufzulösen (Formel nicht
  offengelegt).
- **Q automatisch frequenzabhängig:** tiefe Frequenzen bekommen enge
  Q-Spannen (~5–7), hohe Frequenzen breite (~25–40). Manuell übersteuerbar.
- Eingriff **dynamisch mit Soft-Knee**: reduziert nur, wenn die Resonanz
  gerade über der Schwelle liegt — kein statischer Cut.
- Knoten tragen Frequenz **und Notennamen** (musikalische Verortung).

**Engineering-Ableitung:** Die „statistisch signifikant über den Nachbarn"-
Formulierung impliziert eine **robuste Basislinie** (Nachbarschafts-Statistik,
nicht bloßes Mittel — ein Mittel würde vom Peak selbst hochgezogen) plus eine
**Persistenz-Buchhaltung** (record over time). Der vorgeschlagene Gain dürfte
eine gedeckelte Funktion des gemessenen Überstands sein — exakt das Muster
unseres `cutBis = clamp(2..6, 0.6·dbOver)`.

**Was RESO NICHT ist:** rein beratend. Es greift ein (dynamischer Suppressor).
Unser Produkt übernimmt die *Mess*-Mechanik, nie den Eingriff.

### 1.2 Mastering The Mix EXPOSE 2 — Datei-QC mit Befundliste

**Dokumentiert (EXPOSE-Manual):** Fünf Prüf-Sektionen auf der fertigen Datei:

1. **Loudness** — Integrated + Short-term LUFS gegen Plattform-Ziele
   (Spotify ≈ −14 LUFS int; jenseits −6 LUFS int/st gilt als überkomprimiert).
2. **Peak** — True Peak (dBTP) + Sample Peak.
3. **Stereo** — L/R-Balance- und Phasenkorrelations-**Heatmap über die Zeit**
   (Mono-Kompatibilität pro Stelle, nicht nur global).
4. **Dynamik** — DR (Punch) + Loudness Range (Empfehlung: LRA > 5 LU für
   dynamisches Material).
5. **Tonale Balance** — Durchschnittsspektrum **gegen importierte Referenz**;
   Toleranzsprache: **±3 dB = „sehr ähnlich", > ±6 dB = „deutlich anders"**.

Präsentation: Sektions-Icon wird **rot**, wenn außerhalb des akzeptierten
Bereichs; Klick aufs Icon **isoliert die Problemstellen auf der Waveform**
(zeitliche Verortung!); dazu konkrete Handlungsansagen („Limiter-Input um
X dB senken").

**Engineering-Ableitung:** EXPOSE ist das Vorbild fürs *Befund-Format*
(Ampel → Klick → Ort → konkreter Griff), nicht für die Messung — unsere
BS.1770/True-Peak/DR-Kette (pyloudnorm-exakt, 8×-Polyphase) ist bereits auf
oder über EXPOSE-Niveau. Was uns fehlt, ist die **zeitliche Verortung**
(„WO in der Passage") — EXPOSE kann als Datei-Tool trivially über die ganze
Timeline zeigen; wir messen live und brauchen dafür eine Zeitreihe (→ M3c).

### 1.3 sonible smart:EQ 4 — Lernfenster + Profil

**Dokumentiert (sonible-Blog „How we do Spectral Processing" + Produktseite/
Reviews):**

- Zerlegung per FFT in linear verteilte Bins, dann **Gruppierung in
  perzeptiv gewichtete Bänder** (Bark/Mel-artig): tiefe/mittlere Frequenzen
  bekommen mehr Auflösung, ausdrücklich betont für **100–500 Hz**
  (Grundton-Bereich von Stimme und vielen Instrumenten).
- smart:EQ arbeitet mit **> 100 Bändern** für Analyse/Vergleich.
- **Lern-Vorgang:** Profil wählen (Genre/Instrument oder „Universal"),
  Abspielen, Lern-Klick — nach **~6 s Hörzeit (Default, verlängerbar)** steht
  die Analyse; daraus wird die Ausgleichskurve generiert. Die Entscheidungs-
  Logik („was stört, was fehlt") beruht laut Hersteller auf Signalstatistik +
  **psychoakustischen Modellen** + kuratierter Mixing-Erfahrung.
- Eigene Profile lassen sich **aus Referenztracks** erzeugen (Statistik eines
  Wunschklangs als Ziel).

**Engineering-Ableitung:** Das „Profil" ist eine **statistische Zielverteilung
pro Band** (mindestens Mittel, praktisch sicher auch Streuung), das Lernen ist
eine **kurzzeitige LTAS-Akkumulation mit Konvergenz-Annahme** (6 s reichen,
weil >100 perzeptive Bänder schnell stabil werden; feine 1/24-Okt-Raster wie
unseres brauchen länger). Übersetzung für uns: **Zielkorridor §5.4** = Profil;
„Lernfenster" = unsere laufende Messung — was fehlt, ist ein **sichtbares
Konvergenz-Signal** („Kurve steht") statt bloßer Messzeit-Anzeige.

### 1.4 FabFilter Pro-Q 4 — Analyzer, Spectrum Grab, EQ Match, Collisions

**Dokumentiert (Pro-Q-Hilfe):**

- **Analyzer:** Auflösung Low/Medium/High/Maximum = **1024/2048/4096/8192
  Punkte**; einstellbare Release-Geschwindigkeit (Averaging); Anzeige-**Tilt
  um 1 kHz, Default 4,5 dB/Okt** („entspricht am besten der wahrgenommenen
  Lautheit"); Range 60/90/120 dB; **Freeze** = Max-Hold, das sich über die
  Zeit aufbaut; Pre/Post/External (Sidechain) getrennt schaltbar.
- **Spectrum Grab:** Fahre über den Analyzer → weiße Kontur + **erkannte
  Peaks als greifbare Punkte mit Frequenz-Label**; Peak greifen und
  herunterziehen erzeugt ein EQ-Band. Empfohlen auf dem Post-Spektrum („was
  du wirklich hörst").
- **EQ Match:** Referenz aus Sidechain, anderer Pro-Q-Instanz oder
  **Audiodatei**; beide Seiten mitteln, bis sich das Durchschnittsspektrum
  **„nach normalerweise ≤ 30 s" nicht mehr nennenswert ändert**; dann dicke
  weiße **Differenzkurve**; daraus automatisch **so wenige Bänder wie nötig**
  („sufficient to match the most important characteristics"), per
  Detail-Slider zwischen „nur Grundform" und „jede Kleinigkeit".
- **Show Collisions:** rotes Glühen, wo sich das eigene Spektrum mit dem
  einer anderen Instanz/Sidechain **gleichzeitig energetisch überlappt** —
  Masking-Verdachtsanzeige zwischen Kanälen.

**Engineering-Ableitung:** Drei übertragbare Ideen. (a) Der Analyzer trennt
strikt **Messung** (schnell, roh) von **Anzeige** (Tilt, Averaging, Max-Hold)
— unsere Architektur tut das bereits per Snapshot-Kopie. (b) **Peak-Griff als
Interaktionsmodell**: der Analyzer kennt seine Peaks und macht sie zu
Objekten mit Label — für uns die Vorlage für „Kandidat anfassen → Karte", ohne
je einen Filter zu setzen. (c) **EQ Match als Übersetzer**: eine
Differenzkurve wird auf **wenige benennbare Griffe** reduziert — exakt die
Philosophie unserer Befundkarten, als Algorithmus formuliert (stückweise
Approximation, Detail-Regler = Anzahl zugelassener Griffe).

---

## Teil 2 — Die drei Kernmechaniken als DSP-Rezepte

### 2.1 Resonanz-Tracking (RESO-Mechanik)

Formal, auf log-f-Raster (unser 1/24-Okt, 221 Bänder):

1. **Frame-Spektren** X_t(b) in dB (bei uns: Welch-Segmente der zuständigen
   Stufe — Bass 16384 / Mitten 4096 / Höhen 2048; „mehrmals pro Sekunde" ist
   durch die Hops gegeben).
2. **Robuste Basislinie** B_t(b) = Statistik der Nachbarschaft ±W (W ≈ 1/3
   Okt). *Kritisch:* **Median statt Mittel** — das Mittel wird vom Peak
   selbst hochgezogen und unterschätzt die Prominenz systematisch; „statistisch
   signifikant über den Nachbarn" (RESO) verlangt eine Basislinie, die der
   Ausreißer nicht kontaminiert.
3. **Prominenz** P_t(b) = X_t(b) − B_t(b). Kandidat, wenn P ≥ θ_P (unser
   Startwert 6 dB) **und** lokales Maximum.
4. **Persistenz-Buchhaltung** pro Band: Anteil aktiver Frames mit P ≥ θ_P
   („record over time"). Klassen: **zeitweise** (≥ 15 %) vs. **dauerhaft**
   (≥ 50 %) — bereits gebaut (`excessSegmente/segmente`).
5. **Breiten-Gate:** −6-dB-Breite der Prominenz ≤ 1/6 Okt ⇒ Resonanz
   (Q ≥ ~8,7); breiter ⇒ Balance-Thema, anderer Regelweg. (Q↔Breite:
   Q ≈ 1/(2^(B/2) − 2^(−B/2)); RESO-Q-Spannen 5–40 sind damit konsistent.)
6. **Empfehlungs-Gain** = gedeckelte Funktion des Überstands (unser
   0,6·P, 2–6 dB) — nie „auf null bügeln": transparent auflösen heißt, die
   Spitze in die Nachbarschafts-Statistik zurückholen.
7. **Musikalische Verortung:** f → nächster Halbton
   (n = round(69 + 12·log2(f/440)), Abweichung in Cent). ⚠️ In jedem
   User-Text FL-Oktavzählung verwenden (MIDI 60 = **C5**, also Oktave =
   n div 12 — Landmine aus `tools/midi-ideen`). Beispiel-Fixture: 116 Hz →
   MIDI 46 = **A#3/B♭3 (FL-Zählung)** — in einem B♭-Moll-Song die Tonika-
   Oktave: *das* ist beratbare Information, die kein Benchmark liefert.

### 2.2 Lernfenster / Profil (smart:EQ-Mechanik)

1. **Akkumulation:** laufende LTAS als Leistungs-Mittel aktiver Segmente
   (gebaut, inkl. Aktivitäts-Gate −60 dBFS). Perzeptive Gewichtung steckt bei
   uns im log-f-Raster; zusätzliches Bark-Mapping ist unnötig, weil wir
   **nicht** automatisch eingreifen — wir brauchen Ortsauflösung, keine
   Lautheitsentscheidung pro Band.
2. **Konvergenz statt Stoppuhr:** Lernfenster ist fertig, wenn sich die
   Kurve nicht mehr bewegt (Pro-Q: „≤ 30 s, bis sich nichts mehr ändert";
   smart:EQ: 6 s auf groben Bändern). Kriterium: Δ = max über belastbare
   Bänder von |LTAS_t − LTAS_{t−T}| (T ≈ 10 s Aktivzeit); **konvergiert wenn
   Δ < 0,5 dB**. Das ersetzt die reine Messzeit-Komponente der Konfidenz
   durch einen *gemessenen* Stabilitätsbeleg.
3. **Profil = Verteilung, nicht Linie:** pro Band Mittel + Streuung über den
   Referenz-**Korpus** (mehrere eigene Bounces, passagen-gematcht — Plan
   §5.4 „Zielkorridor statt universeller Optimalkurve"). Toleranzsprache von
   EXPOSE übernehmen: **±3 dB unauffällig, ab ±6 dB deutlich** — als
   Startwerte der Korridor-Halbbreite, wo der Korpus zu dünn für echte
   Streuung ist.
4. **Percentile aus dem Pegelhistogramm:** unser 1-dB-Histogramm pro Band
   (existiert für Abdeckung §5.10.2) liefert ohne neue Akkumulation
   **P10/P50/P95 pro Band** — also nicht nur „wo liegt das Mittel", sondern
   „wie verteilt sich das Band über die Zeit". P95−P50 groß = Band lebt
   dynamisch (Momente), P50-Überstand = statischer Zustand. Das ist die
   Frame-Statistik, die RESO „record" nennt — wir haben sie schon auf Platte.

### 2.3 Peak-Griff + Vergleich (Pro-Q-Mechanik)

1. **Anzeige-Schichten:** Messkurve (Komposit), Live (3-s-EMA), Max-Hold
   („Freeze" als aufbauendes Maximum — trivial aus vorhandenen Segmenten),
   Vergleichslinie (gebaut). Tilt/Neigungsansicht: gebaut.
2. **Hover-Inspekt:** Cursor → nächstliegender Kandidat/Bandmaximum
   (parabolische Interpolation über die 3 Nachbar-Bänder für die
   Feinfrequenz), Label = Frequenz + Note + Überstand; Klick öffnet die
   zugehörige Befundkarte. **Kein** Band-Erzeugen — der Griff bleibt beim
   User im EQ seiner Wahl (Produkt-Grundsatz „rein beratend").
3. **Differenz → wenige Griffe (EQ-Match-Übersetzer):** D(b) = eigene Kurve −
   Referenz (Korridor-Mitte, PRE/POST-Partner oder geladene Vergleichskurve),
   geglättet auf 1/3 Okt. Greedy: größte |D|-Region nehmen, als Glocke
   beschreiben (Center = Extremum, Breite = Überschreitungs-Region der halben
   Spitze, Gain = mittleres D dort), aus D abziehen, wiederholen bis Residuum
   < 1,5 dB oder **3 Griffe** erreicht. Detail-Regler à la Pro-Q brauchen wir
   nicht — die Karten-Philosophie deckelt bewusst auf Top-3 (§5.9).
4. **Collisions → Masking-Hypothese (§5.6):** zwei Sensoren (M2-Instanzen)
   gelten als kollidierend in Band b, wenn beide **im selben Projektzeit-
   Fenster aktiv** sind (Heartbeat v2 trägt {von, bis, Sprünge}) und beide
   P50(b) im jeweiligen Top-Energie-Drittel liegen. Ausgabe ist eine
   **Hypothesen-Karte** („Klavier und Chor konkurrieren um 200–400 Hz —
   prüfe, wer dort führen soll"), nie ein Fehleretikett.

---

## Teil 3 — Schwellen-Mathematik: „normaler Peak" vs. „Problem"

Die Differenzierung steht auf **fünf orthogonalen Achsen** — ein Befund
braucht alle relevanten, nicht eine:

| Achse | Maß | Startschwellen (versioniert, `kDiagnoseVersion`) |
|---|---|---|
| **Prominenz** | dB über robuster Nachbarschafts-Basislinie (Median ±1/3 Okt) | Kandidat ≥ 6 dB; Karten-Empfehlung ≈ 0,6·Überstand, 2–6 dB |
| **Persistenz** | Anteil aktiver Frames/Ticks über Schwelle | Resonanz: ≥ 15 % zeitweise, ≥ 50 % dauerhaft · Zonen: dito auf 1-s-Ticks (M3a) |
| **Breite** | −6-dB-Breite in Oktaven | ≤ 1/6 Okt = Resonanz · breiter = Zonen-Regelweg (Mulm/Loch/Härte mit eigenen ±3/4-dB-Schwellen gegen die Schulterlinie) |
| **Abdeckung** | Anteil belastbarer 1/3-Okt-Bänder (Pegelhistogramm über Teppich+6 dB) | Gate ≥ 60 % — ohne Signal keine Aussage (Falsch-Positiv-Riegel, Pink/Stille ⇒ null Karten) |
| **Charakter** | P95−P50 aus dem Band-Histogramm | groß (> ~8 dB) ⇒ Moment-Problem → dynamische Erstidee · klein ⇒ Zustands-Problem → statischer Griff |

Dazu die **Kontext-Achse**, die kein Benchmark hat: Stil-Priorisierung
(Mitten-Loch = User-Hebel #1 vor Mulm vor Härte; Air-Hype ist in diesem Stil
immer verdächtig) und **Tonart-Bezug** (Resonanz auf der Tonika ist erwartbar
und wird anders formuliert als eine leiterfremde — Erklärung statt Alarm).

Warum „Mud" nicht einfach „Energie bei 300 Hz" ist: die Zonen-Regeln messen
**relativ zur eigenen Schulterlinie** (log-f-Linie der Nachbarzonen) mit
60-%-Bandkriterium gegen Einzel-Peak-Verzerrung — ein voller, gewollter
Tiefmitten-Bereich (tiefe Klavier-Akkorde!) triggert nichts, solange er im
Verhältnis zu seinen Schultern bleibt. Das ist der entscheidende Unterschied
zu absoluten Schwellen („−12 dB bei 300 Hz = Mud") — die wären bei
Ballade↔Rock-Atmosphäre beide falsch.

---

## Teil 4 — Ist-Stand EQ-Copilot vs. Benchmarks (ehrlich)

**Bereits auf/über Benchmark-Niveau (gebaut, GOLDEN 178/178):**

- Mess-Substanz: 4-stufige Welch-LTAS (16384…2048) mit Kanalenergie-PSD,
  BS.1770 pyloudnorm-exakt, True Peak 8×/Kaiser — EXPOSE-Messumfang live.
- Resonanz-Frame-Persistenz mit Abdeckungs-Gate + Breiten-Gate (RESO-Kern,
  konservativer abgesichert als das Vorbild dokumentiert).
- Befundkarten Gemessen/Wirkung/Tu/Warum/Hören/Sicherheit in Lernsprache —
  über EXPOSE hinaus (das rote Icons + Zahlen zeigt, aber nicht erklärt).
- Konfidenz-Modell (4 Komponenten, min+Mittel) — hat kein Benchmark.
- Architektur-Trennung Messung/Anzeige (Snapshot-Kopie) = Pro-Q-Disziplin.

**Lücken, die die Benchmarks aufzeigen (→ Roadmap):**

| Lücke | Vorbild | Träger im Bestand |
|---|---|---|
| Basislinie = Mittel (peak-kontaminiert) | RESO „statistically significant" | `findeResonanzen`-Glättung → Median |
| Keine Notennamen/Tonart in Karten | RESO-Knoten | `resonanzKarte` + FL-Zählungs-Regel |
| Zonen-Persistenz fix 0,5 (Zeitmittel) | RESO-Record / EXPOSE-Timeline | 1-s-Tick-Ring auf `liveEmaLinear` |
| Kein Konvergenz-Signal („Kurve steht") | Pro-Q EQ Match ≤ 30 s-Stabilität | LTAS-Delta über 10 s Aktivzeit |
| Kein kalibrierter Korridor | smart:EQ-Profile / EXPOSE ±3/±6 dB | §5.4 + `tools/` (analyze-track-Achse = identisches Raster!) |
| Max-Hold/Percentile unsichtbar | Pro-Q Freeze | `pegelHistogramm` → P10/50/95 |
| Keine zeitliche Verortung („wo im Song") | EXPOSE Waveform-Isolation | Tick-Ring + M2-Projektzeit-Fenster |
| Masking nur geplant | Pro-Q Show Collisions | M2-Heartbeat v2 (Fenster + LTAS liegen im Broker) |
| PRE/POST-Differenz ungenutzt für Karten | Pro-Q EQ Match Differenzkurve | M2-Paar-Logik + Aggregat-Snapshot |

---

## Teil 5 — Blueprint: Architektur-Deltas

Keine neue Schicht, keine neue App. Alle Deltas landen in bestehenden
Bausteinen; Audio-Thread bleibt unberührt (alles im Worker/auf Snapshots):

1. **AnalyseEngine (Worker):**
   - `findeResonanzen`: Basislinie auf **Median** (±1/2 Okt, Zentrum ±1 Band
     ausgenommen) umstellen — neue `kMetricsVersion`, GOLDEN-Referenz wird
     regeneriert und Wert-für-Wert begründet (Erwartung: Excess-Werte steigen
     leicht; 116-Hz-Fall bleibt GENAU 1 Kandidat).
   - `auswerten()`-Tick (1/s): **Zonen-Ring** — pro Diagnose-Zone (5 Zonen +
     Schultern) Mittel aus `liveEmaLinear` + Schulterlinien-Delta + Short-LUFS
     in einen Ring (900 Einträge ≈ 15 min). Daraus: echte **Zonen-Persistenz**
     (Anteil Ticks jenseits Schwelle) und später Passagen-Cluster.
   - **Percentile** P10/P50/P95 pro Band aus `pegelHistogramm` in den
     Snapshot (billig: Histogramm existiert, nur Auslesen).
   - **Konvergenz:** LTAS-Kopie bei t−10 s Aktivzeit, Δ_max über belastbare
     Bänder in den Snapshot.
2. **Diagnose (pur, Snapshot):**
   - Konfidenz: `persistenz` der Zonen-Befunde aus dem Ring statt 0,5;
     `messzeit` aus Konvergenz statt Stoppuhr. Zonen können damit ehrlich
     „hoch" erreichen — der bekannte Deckel fällt beweisbar, nicht still.
   - Resonanzkarten: Notenname + Cent + Tonart-Einordnung (Profil kennt die
     Tonart nicht — Formulierung bleibt bedingt: „= A#3, in einem B♭-Moll-
     Stück die Tonika"), Charakter-Zeile aus P95−P50.
   - Neue Karte **Korridor-Abweichung** erst, wenn Korpus-Kalibration
     vorliegt (§5.10.4: keine Orientierungs-Aussage ohne Griff).
   - Neue Karte **PRE/POST**: Differenzkurve → max. 3 Griffe (Übersetzer aus
     2.3) — formuliert als „das hat dein EQ getan / das wäre noch offen".
   - **Masking-Hypothese** (nur Broker-Seite sichtbar, §5.6-Sprache).
3. **Editor (Anzeige):** Max-Hold-Schicht (Freeze), Hover-Inspekt mit
   Peak-Snap + Notenlabel, Konvergenz-Anzeige im Statusfeld („Kurve steht
   seit 12 s"), Percentile optional als Band (P10–P95-Schattierung) hinter
   der Kompositkurve — Ebenenbudget aus §7.2 beachten (R3: Ruhe zuerst).
4. **Broker (Rust):** Kollisions-Auswertung auf vorhandenen v2-Heartbeats
   (Fenster-Schnitt + Band-Energie-Drittel), Ausgabe in Sensorübersicht +
   Aggregat-Snapshot. Korridor-Datei pro Projektprofil (Bindung existiert).
5. **tools/ (Offline-Kalibration):** `tools/eq-copilot/kalibriere_korridor.py`
   — läuft mit `py -3.13` über benannte eigene Bounces (+ ffff.wav als
   Stil-Anker), exakt auf der analyze-track-Achse (= Plugin-Referenzachse,
   Kreuzvalidierung bleibt der Maßstab), schreibt Korridor-JSON
   (Mittel + Streuung + Passagen-Metadaten) nach
   `%LOCALAPPDATA%\evenacadia\EQ-Copilot\korridore\`. Schema versioniert
   neben `eq-snapshot.schema.json`.

**Bewusst NICHT übernommen:** Auto-Nodes/Auto-Kurve (RESO/smart:EQ wenden an
— verletzt den Produktkern „User-Entscheidung absolut"), Bark-Band-Reduktion
(wir beraten ortsgenau, statt pro Lautheitsband zu regeln), EXPOSE-
Plattform-Presets als Pflicht (unser Ziel ist der eigene Stil-Korridor, nicht
eine Streaming-Norm; LUFS/TP-Zahlen stehen ja bereits im Snapshot).

---

## Teil 6 — Roadmap (3 Phasen auf dem lebenden Kern)

**M3a — Resonanz v2 + ehrliche Zonen-Zeit (~2–3 h, reine Kern-Vertiefung):**
Median-Basislinie · Notennamen (FL-Zählung!) + Charakter aus P95−P50 ·
Zonen-Tick-Ring → echte Zonen-Persistenz → Konfidenz ohne 0,5-Deckel ·
Konvergenz-Signal in Snapshot + Status. **Beweis:** GOLDEN erweitert um
(a) wanderndes Sinus-Fixture (Ton springt je 2 s um 1/2 Okt ⇒ MUSS
„zeitweise" klassifizieren, nie „dauerhaft"), (b) Zwei-Zustands-Fixture
(Mulm nur in „Refrain"-Hälfte ⇒ Zonen-Persistenz ≈ 0,5, Karte nennt
„zeitweise"), (c) Median-vs-Mittel-Regression Wert für Wert begründet;
NULLTEST · pluginval 8 unverändert.

**M3b — Lernfenster sichtbar + Korridor (~3–4 h):** Kalibrations-Skript über
even-Bounces + Korridor-JSON + Profil-Bindung im Broker · Korridor-Karte
(±3 dB-Sprache, nur bei belastbarer Abdeckung + Konvergenz) · damit fällt
auch die M3-Aussetzung „Tilt/Air nur qualitativ" — gegen Korridor wird sie
ein konkreter Griff. **Beweis:** GOLDEN-Fixtures gegen synthetischen
Mini-Korpus (3 gefärbte Pink-Varianten als „Bounces"), Kalibrator-Selbsttest
(Korridor aus identischen Dateien = Streuung 0), Schema-Validierung.

**M3c — Vergleich + Ort (~3–4 h):** PRE/POST-Differenzkarte (Übersetzer,
max 3 Griffe) · Hover-Inspekt/Max-Hold im Editor · Passagen-Cluster auf dem
Zonen-Ring (2–3 Zustände über Short-LUFS+Zonenvektor; Karten sagen „im
lauten Teil…, im leisen nicht") · Masking-Hypothese im Broker auf v2-Fenstern.
**Beweis:** GOLDEN-Paar-Fixture (bekannter EQ-Eingriff zwischen PRE/POST ⇒
Übersetzer findet GENAU die gesetzte Glocke), Broker-Test mit konstruierten
Heartbeats (Kollision erkannt/nicht erkannt), PROBE-Roundtrip.

Reihenfolge ist verbindlich (Kern vor Verwaltung, §0.3): M3a vertieft die
Diagnose, die der User täglich sieht; M3b braucht Korpus-Entscheid (welche
Bounces zählen — einzige Nicht-Technik-Frage, Empfehlung: die lauf5x-Familie
+ ffff.wav); M3c braucht M2-Instanzpraxis im Alltag.

---

## Quellen (abgerufen 2026-08-15)

- RESO-Manual: https://www.masteringthemix.com/pages/reso-manual · Produkt: https://www.masteringthemix.com/products/reso
- EXPOSE-Manual: https://www.masteringthemix.com/pages/expose-manual · EXPOSE 2: https://www.masteringthemix.com/products/expose-2
- Pro-Q-Hilfe Analyzer/Spectrum Grab/EQ Match: https://www.fabfilter.com/help/pro-q/using/analyzer · …/spectrumgrab · …/eqmatch
- sonible „How we do Spectral Processing": https://www.sonible.com/blog/spectral-processing/ · smart:EQ 4: https://www.sonible.com/smarteq4/
- smart:EQ-4-Lernfenster (6 s Default, Review): https://digitalfilms.wordpress.com/2024/09/27/sonible-smarteq-4/
- Sekundär (Reviews, nur zur Triangulation): mixonline.com (RESO), audiopluginguy.com (RESO/EXPOSE), musictech.com (smart:EQ)
