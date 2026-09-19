VALIDIERUNG: D-2 PRÄZISIERT · D-3 BESTÄTIGT · B-1 BESTÄTIGT

# NAK-311 — Validierung der Matrixprüfung 2 (D-2-Rest, D-3-Rest, B-1, Fakten zu H-a), 19.09.2026

Frischer, lesender Validierer; Auftrag `docs/beweise/roh/NAK-311-matrixpruefung-2-validierung-auftrag.txt`. Geprüft wird das Urteil `docs/beweise/roh/NAK-311-matrixpruefung-2-urteil.md` (D-2 `:34-38`, D-3 `:40-44`, B-1 `:48`, H-a `:58`). `:n` ohne Datei meint die im selben Satz oder in derselben Zelle zuletzt genannte Datei, sonst `docs/beweise/NAK-311.md` am HEAD; dort ist das Manifest bis auf Zeile 8 und den angehängten §14 gleich `70332210`, die Arbeitskopie gleich HEAD. Der Code unter `eq-copilot/`, `broker/` und `tools/fl/` ist seit `ac62c734` unverändert. Jede Aussage über Rot und Grün ist aus der Quelle hergeleitet, keine am Produkt gemessen. Die Einschwingzahlen zu D-2 stammen aus einem Nachbau der Coderekursionen (Aufbau und Eichung unter „Selbst nachgemessen"), nicht aus einem Lauf des Produkts.

## D-2 — die Toleranz aus F-12 hält in ihrem eigenen Geltungsbereich nicht

### 1. Manifest am HEAD, wörtlich

- R-311-4 (`:129-133`): „**R-311-4 (F12).** Die numerische Toleranz von M-84 für den Recall eingeschwungener dynamischer Bänder bekommt eine Zahl. Der Bauer der Etappe 1 schlägt sie aus der Hüllkurvenmechanik vor und belegt sie in Etappe 4 mit einer Messung; die Hüllkurvenleistung bleibt Audiohistorie, kein persistenter Wert, keine neue Stateversion."
- F-12, Geltungsbereich (`:1468-1476`): „**Geltungsbereich:** ein dynamisches Bell (Typ `bell`, Kanalmodus `stereo`, Sidechain `internal`) mit Quadraturton auf der Bandmitte f0 (L sin, R cos, E-25) im ganzen vertragsgültigen Raum von `eq-copilot/schemas/state/nakama-parameter-v2.json`: Q 0,15 bis 24, Attack 0,1 bis 500 ms, Grundgain g0 und Range je −12 bis +12 dB, Threshold −60 bis 0 dBFS, Hold und Release beliebig (während des Anstiegs wirkungslos), f0 von 20 Hz bis min(20 kHz, 0,45·fs), fs 44,1 bis 96 kHz. Für Shelves und breitbandiges Material nennt dieses Ticket keine Zahl; dort bleibt M-84 bei „innerhalb numerischer Toleranz"."
- F-12, Toleranz (`:1476-1479`): „**Toleranz:** ab t_E = max(`kFadeSamples`, 5·τ_a·fs + 10·Q·A_max·fs/(π·f0) + 16) Samples mit A_max = 10^(max(0, g0, g0 + Range)/40) weicht der Quadraturbetrag je Sample höchstens 0,1 dB ab"
- F-12, Herleitung (3) (`:1500-1505`): „(3) Detektor und Band starten kalt: der Detektor-Bandpass hat die Zeitkonstante Q/(π·f0), das Bandfilter beim Gesamtgain g die Zeitkonstante Q·A/(π·f0) mit A = 10^(g/40) (Dämpfung k = 1/(Q·A), `DspSvf.h:104-105`), bei Anhebung also länger; A_max nimmt den größten Gesamtgain des Einschwingens, der zwischen g0 (Auslenkung 0) und g0 + g_A liegt. Nach zehn Zeitkonstanten ist der Rest e^−10 und vernachlässigbar."
- F-12, Herleitung (4) (`:1505-1508`): „… 0,1 dB lassen damit über das Dreifache Raum für Rundung und Restglieder."
- M-77 (`:689`), Zustand: „**Prüfling 1** der aus M-76 (Bell 1 kHz Q 2); **Prüfling 2**, bei dem der Einblend-Crossfade überwiegt: dynamisches Bell 1 kHz Q 0,707, Grundgain 0 dB, `stereo`, Range −12 dB, Attack 0,1 ms, Hold 0, Release 100 ms"; Zusage: „Die Abweichung des Quadraturbetrags je Sample, 20·log10(B/A), ist ab t_E = max(`kFadeSamples`, 5·τ_a·fs + 10·Q·A_max·fs/(π·f0) + 16) Samples höchstens 0,1 dB, mit A_max = 10^(max(0, g0, g0 + Range)/40); … (Geltungsbereich und Herleitung §9 F-12). Prüfling 1 bei Attack 0,1, 10 und 500 ms: t_E = 345,6 Samples, 56,7 ms und 2,507 s bei 48 kHz; Prüfling 2 bei Attack 0,1 ms: t_E = max(256, 148) = 256 Samples; je im Knie (Threshold −15 dB bei Detektorleistung −9,03 dB, Auslenkung −5,97 dB) und im Plateau (Threshold −40 dB)".
- M-78 (`:690`): Zusage „wie M-77"; Rotbeweis „… je Hälfte dieselbe Mutation wie in M-77 → rot aus demselben Grund, weil derselbe Weg `bereiteVor` → Ruhe → Einblenden läuft".
- §7.4 (`:1248-1251`): „Der Nachtrag in SONDE-015 nennt bei M-84 die Zahl aus §9 F-12 samt Geltungsbereich — ab t_E = max(`kFadeSamples`, 5·τ_a·fs + 10·Q·A_max·fs/(π·f0) + 16) Samples höchstens 0,1 dB, davor im Betrag höchstens Betrag(g0) + Betrag(Range) + 0,1 dB — und verweist auf NAK-311 M-77."
- Schließregel D-2 (`docs/beweise/roh/NAK-311-matrixnacharbeit-1-auftrag.txt:21`): „Die Zahl gilt im ganzen vertragsgültigen Parameterraum (Q, Attack, Grundgain, Range nach eq-copilot/schemas/state/nakama-parameter-v2.json) oder nennt ihren Geltungsbereich ausdrücklich; …"

### 2. Codebehauptungen des Prüfers, an der Quelle nachgemessen

| Behauptung (Urteil `:36-38`) | Quelle am HEAD | Ergebnis |
|---|---|---|
| Detektor RBJ-Bandpass, α = sin(w0)/(2Q), Bandgüte (`DspFilter.h:224-238`, `DspProgramm.cpp:255`) | `DspFilter.h:227` `const double alpha = std::sin (w0) / (2.0 * q);`, `:232-236` b0 = α/a0, b1 = 0, b2 = −α/a0, a1 = −2c/a0, a2 = (1 − α)/a0; `DspProgramm.cpp:255` `b.detektor = entwurfBandpass (samplerate, b.freqHzWirksam, b.q);` | stimmt |
| Band TPT-SVF, g = tan(π·f0/fs), k = 1/(Q·A) (`DspSvf.h:87-111`) | `DspSvf.h:89` `return std::tan (kPi * freqHz / samplerate);`, `:104-105` `A = std::pow (10.0, gainDb / 40.0)`, `k = 1.0 / (q * A)`, `:97-99` a1 bis a3, `:109` m1 = k·(A·A − 1); Neuentwurf je Steuerschritt `DspKern.cpp:575` | stimmt (`svfBell` reicht bis `:112`) |
| Polradius² = (1 + g² − g·k)/(1 + g² + g·k) | aus `DspSvf.h:73-81` mit `:97-99` hergeleitet (3.2) | stimmt für komplexe Pole (k < 2) |
| „Beide klingen mit 2·Q·A/sin(w0) Samples ab" | exakt 1/artanh(sin(w0)/(2·Q·A)), beim Detektor A = 1 (3.1, 3.2); 2·Q·A/sin(w0) ist die Näherung für großes Q·A (Randfall 96,000 statt 95,997) | stimmt als Näherung für komplexe Pole |
| Faktor w0/sin(w0): 1,003 (1 kHz/48 kHz), 5,24 (20 kHz/48 kHz), 9,15 (0,45·fs) | 1,0029; 5,2360; 9,1498 | stimmt |
| Randfall nach F-12: 5·τ_a·fs = 24, 10·Q·A_max·fs/(π·f0) = 183,3, t_E = max(256; 223,3) = 256 | 24; 183,35; 223,35; 256 | stimmt |
| α = 0,010417, r = 0,98964, Detektor-τ 96,0 Samples (Formel 18,3), 1 − r^256 = 0,9305, Pegel −0,63 dB | 0,0104167; 0,989637; 95,997 (18,335); 0,93052; −0,626 dB | stimmt |
| Knie mit Steigung 1 (`DspFilter.h:355-361`) | `:359` `u = ueber >= kKniebreiteDb ? 1.0 : ueber / kKniebreiteDb`, `:360` `return rangeDb * u;`, `kKniebreiteDb` = 12 (`:347`): Steigung Betrag(Range)/12 = 1 bei Range −12 dB | stimmt |
| „senkt B um mindestens 0,63 dB weniger ab (−5,34 statt −5,97 dB)" | Modell bei n = 256: Hüllkurvenpegel B −9,684 dB gegen A −9,031 dB; letzter Entwurf B −5,316 dB gegen A −5,969 dB | stimmt (0,65 dB) |
| „das Band (τ 70 bis 96 Samples) ist ebenfalls nicht eingeschwungen" | 96,0 Samples bei 0 dB, 70,6 bei −5,34 dB (A bei −5,97 dB: 68,1) | stimmt |
| „20·log10(B/A) ≥ +0,6 dB bei n = t_E" | Modell: +0,567 dB bei n = 256 (+0,604 bei n = 255); der normierte Bandpasszustand k·v1 von B steht bei 1,023 statt 1,0 und senkt um 0,086 dB mehr ab als die Auslenkung −5,316 dB | **anders**: +0,57 dB; das „mindestens" hält nicht, der Bruch der 0,1 dB bleibt |
| „Q·A < 1/2: reelle Pole" | Band reell für Q·A < 1/2 (k > 2), Detektor für Q < 1/2; A geht in den Detektor nicht ein | stimmt fürs Band |
| „Detektor Q 0,15 bei 100 Hz/48 kHz: langsamer Pol 0,99784, τ ≈ 463 Samples gegen 22,9" | aus den RBJ-Koeffizienten a1 = −1,9162202, a2 = 0,9163843: langsamer Pol 0,9979922, τ = 497,6 Samples; Näherung 22,92 | **anders**: 0,99799 und 497,6 Samples |
| Grenzen Q 0,15 bis 24, Gain und Range ±12 dB, Threshold −60 bis 0 dBFS, Attack 0,1 bis 500 ms, Release 5 bis 5000 ms | `nakama-parameter-v2.json:170-171`, `:183-184`, `:223-224`, `:236-237`, `:249-250`, `:275-276` | stimmt |

### 3. Herleitung, selbst nachvollzogen und gerechnet

Bezeichnungen: w0 = 2π·f0/fs, c = cos(w0), g = tan(π·f0/fs) = tan(w0/2); Zeitkonstante τ in Samples = −1/ln(Betrag des langsamsten Pols). Die Näherung aus F-12 (3) ist in Samples Q·A·fs/(π·f0) = 2·Q·A/w0.

**3.1 Detektor** (`DspFilter.h:224-238`, Rekursion `:90-96`). Nennerpolynom z² + a1·z + a2 mit a1 = −2c/(1 + α), a2 = (1 − α)/(1 + α).
- Komplexe Pole genau dann, wenn a1² < 4·a2, also c² < 1 − α², also α < sin(w0), also Q > 1/2. Polradius r = √((1 − α)/(1 + α)); τ = −1/ln r = 1/artanh(α) = 1/artanh(sin(w0)/(2Q)) ≈ 2Q/sin(w0). Verhältnis zur Näherung 2Q/w0: w0/(2Q·artanh(sin(w0)/(2Q))), für großes Q gleich w0/sin(w0).
- Reelle Pole für Q < 1/2: p = (c ± √(α² − sin²(w0)))/(1 + α); langsamer Pol (Betrag(c) + √(α² − sin²(w0)))/(1 + α); τ = −1/ln davon.
- Wegen α = sin(w0)/(2Q) = g·k/(1 + g²) mit k = 1/Q ist r² = (1 + g² − g·k)/(1 + g² + g·k): derselbe Ausdruck wie beim Band (3.2) mit A = 1.

**3.2 Band** (`DspSvf.h:73-81`, `:93-100`, `:102-112`). Aus `tick` mit x = 0: ic1' = (2·a1 − 1)·ic1 − 2·a2·ic2 und ic2' = 2·a2·ic1 + (1 − 2·a3)·ic2, mit a1 = 1/(1 + g·(g + k)), a2 = g·a1, a3 = g·a2. Spur T = 2·(1 − g²)/(1 + g² + g·k); Determinante D = 2·a1 + 2·a3 − 1 = (1 + g² − g·k)/(1 + g² + g·k), weil −4·a1·a3 + 4·a2² = 0.
- Komplexe Pole genau dann, wenn T² < 4·D, also g²·k² < 4·g², also k < 2, also Q·A > 1/2. r² = D (die Formel des Prüfers); τ = −2/ln D = 1/artanh(g·k/(1 + g²)) = 1/artanh(sin(w0)/(2·Q·A)) ≈ 2·Q·A/sin(w0), weil g/(1 + g²) = sin(w0)/2. Verhältnis zur Näherung für großes Q·A: w0/sin(w0).
- Reelle Pole für Q·A < 1/2: p = ((1 − g²) ± g·√(k² − 4))/(1 + g² + g·k); langsamer Pol (Betrag(1 − g²) + g·√(k² − 4))/(1 + g² + g·k); für kleines w0 τ ≈ 2/(w0·(k − √(k² − 4))). Dieses τ wächst mit k = 1/(Q·A), also mit fallendem A: der langsamste Bandpol liegt bei A_min = 10^(min(0, g0, g0 + Range)/40), nicht bei dem A_max aus F-12 (3). Für komplexe Pole fällt τ mit k; über den ganzen A-Bereich ist die längste Bandzeitkonstante deshalb max(τ(1/(Q·A_max)), τ(1/(Q·A_min))).

**3.3 Zahlen** (exakt nach 3.1 und 3.2; „Näherung" nach F-12 (3); 48 kHz, soweit nicht anders genannt):

| Filter, Fall | k | Pole | r bzw. langsamer Pol | τ exakt [Samples] | Näherung [Samples] | Verhältnis |
|---|---|---|---|---|---|---|
| Detektor 20 kHz, Q 24 | 0,041667 | komplex | 0,989637 | 95,997 | 18,335 | 5,236 |
| Band 20 kHz, Q 24, −5,97 dB | 0,058754 | komplex | 0,985418 | 68,075 | 13,002 | 5,236 |
| Band 20 kHz, Q 24, −12 dB | 0,083136 | komplex | 0,979428 | 48,107 | 9,189 | 5,235 |
| Detektor w0 = 0,9·π (0,45·fs, bei 44,1 kHz 19 845 Hz), Q 24 | 0,041667 | komplex | 0,993583 | 155,33 | 16,977 | 9,150 |
| Detektor 1 kHz, Q 2 (Prüfling 1) | 0,5 | komplex | 0,967884 | 30,634 | 30,558 | 1,0025 |
| Detektor 1 kHz, Q 0,707 (Prüfling 2) | 1,414427 | komplex | 0,911582 | 10,802 | 10,802 | 1,0000 |
| Detektor 1 kHz, Q 0,15 | 6,666667 | reell | 0,980074 | 49,684 | 2,292 | 21,68 |
| Band 1 kHz, Q 0,15, −12 dB | 13,301749 | reell | 0,990138 | 100,895 | 1,149 | 87,84 |
| Detektor 100 Hz, Q 0,15 | 6,666667 | reell | 0,997992 | 497,559 | 22,918 | 21,71 |
| Band 100 Hz, Q 0,15, −12 dB | 13,301749 | reell | 0,999011 | 1010,39 | 11,486 | 87,96 |
| Detektor 20 Hz, Q 0,15 | 6,666667 | reell | 0,999598 | 2487,8 | 114,59 | 21,71 |

Gegengerechnet für den Detektor direkt aus den RBJ-Koeffizienten (`DspFilter.h:229-236`): 20 kHz Q 24 r = 0,9896370, 100 Hz Q 0,15 langsamer Pol 0,9979922, 1 kHz Q 0,15 0,9800740 — gleich den Werten der Tabelle.

**3.4 Randfall des Prüfers** (48 kHz, f0 20 kHz, Q 24, g0 0 dB, Range −12 dB, Threshold −15 dB, Attack 0,1 ms, Release 100 ms, Quadraturton 0,5). F-12: t_E = max(256; 24 + 183,35 + 16) = 256. Code: Detektor τ = 96,0 statt 18,3 Samples; bei n = 256 erreicht die kalte Detektoramplitude 0,930, der Hüllkurvenpegel liegt 0,65 dB unter dem von A, die Auslenkung von B ist −5,316 statt −5,969 dB. Das Band wirkt entgegen: sein Bandpasszustand lädt bei 0 dB mit k = 1/Q = 0,0417 auf und schießt, wenn k mit der Absenkung steigt, 2,3 % über das Ziel hinaus. Modell: 20·log10(B/A) = +0,567 dB bei n = 256, über 0,1 dB bis n = 355; im Plateau (Threshold −40 dB) +0,122 dB bei n = 256, über 0,1 dB bis n = 263.

| Fall (48 kHz, Q 24, g0 0 dB, Range −12 dB, Knie, Attack 0,1 ms) | t_E nach F-12 | 20·log10(B/A) bei t_E | über 0,1 dB bis n |
|---|---|---|---|
| 16 kHz | 269,2 | +0,011 dB | — |
| 18 kHz | 256 | +0,091 dB | — |
| 19 kHz | 256 | +0,224 dB | 292 |
| 20 kHz (Randfall) | 256 | +0,567 dB | 355 |
| 44,1 kHz, 19 845 Hz (0,45·fs) | 256 | +2,387 dB | 576 |

**3.5 Unterer Q-Rand, Q 0,15** (g0 0 dB, Range −12 dB, Attack 0,1 ms, Release 100 ms, 48 kHz, soweit nicht anders). Der Detektor hat reelle Pole (Q < 1/2), das Band auch (Q·A zwischen 0,075 und 0,15). F-12 setzt 2,29 Samples (1 kHz) beziehungsweise 22,9 Samples (100 Hz) je Zeitkonstante an; die langsamen Pole haben 49,7 und 497,6 Samples (Detektor), 100,9 und 1010,4 Samples (Band bei −12 dB). Hinzu kommt ein zweiter Mechanismus: die kalt startende Detektorleistung schießt über ihren eingeschwungenen Wert hinaus, die Hüllkurve fängt das mit Attack ein (`DspFilter.h:314-318`) und hält es mit Release (`:323-326`).

| Q | 0,15 | 0,3 | 0,5 | 0,6 | 0,707 | 1 | 2 |
|---|---|---|---|---|---|---|---|
| größtes Überschwingen der kalten Detektorleistung, f0 100 Hz bis 20 kHz | 0,61 bis 1,02 dB | 0,61 bis 0,78 dB | bis 0,37 dB | bis 0,16 dB | bis 0,03 dB | bis 0,003 dB | 0,000 dB |

| Fall | t_E nach F-12 | 20·log10(B/A) bei t_E | größter Betrag ab t_E (bei n) | über 0,1 dB bis n |
|---|---|---|---|---|
| 1 kHz, Knie | 256 | −0,48 dB | 0,55 dB (274) | 8374 (174 ms) |
| 1 kHz, Knie, Release 5 ms | 256 | −0,20 dB | 0,25 dB (273) | 466 |
| 1 kHz, Knie, Release 5000 ms | 256 | −0,50 dB | 0,57 dB (275) | über 29 999 (dort −0,48 dB) |
| 1 kHz, Plateau | 256 | +0,12 dB | 0,155 dB (273) | 301 |
| 100 Hz, Plateau | 269,2 | −0,46 dB | 1,51 dB (356) | 3006 |
| 100 Hz, Knie | 269,2 | −0,39 dB | 1,41 dB (360) | 9584 |

Im Knie ist das Vorzeichen negativ: B hält das Überschwingen und senkt stärker ab als A. Die Abweichung hängt damit am Release, gegen F-12 „Hold und Release beliebig (während des Anstiegs wirkungslos)" (`:1473`). Auch mit Q 1 bricht der reelle Bandpol, sobald Q·A_min unter 1/2 liegt: 100 Hz, Q 1, g0 −12 dB, Range −12 dB (Q·A_min = 0,25), Plateau: t_E = 1567,9, +0,150 dB bei t_E, über 0,1 dB bis n = 1625.

**3.6 Grenzen.** Vertrag: `freq_hz` 20 bis 20 000 Hz (`nakama-parameter-v2.json:157-158`), zur Laufzeit auf 0,45·fs gekappt (`:14`, `:162`; `DspFilter.h:32-34`, `:109-114`; `DspProgramm.cpp:214-215`); `q` 0,15 bis 24 (`:170-171`); `gain_db` und `dynamic_range_db` je −12 bis +12 dB (`:183-184`, `:223-224`); `threshold_db` −60 bis 0 dBFS (`:236-237`); `attack_ms` 0,1 bis 500 (`:249-250`); `hold_ms` 0 bis 500 (`:262-263`); `release_ms` 5 bis 5000 (`:275-276`); die Samplerate steht nicht im Parametervertrag. F-12 (`:1470-1474`) übernimmt Q und f0 unverändert (f0 bis min(20 kHz, 0,45·fs)) und setzt fs 44,1 bis 96 kHz. Im Geltungsbereich liegen damit 20 kHz bei 48 kHz (w0/sin(w0) = 5,24), 19 845 Hz bei 44,1 kHz (9,15) und Q 0,15 bei jeder Frequenz.

### 4. Gegenargument, aktiv gesucht

- F-12 ist „Vorschlag, gemessen in Etappe 4" (`:1464-1465`). Die Messung fährt aber nur die zwei 1-kHz-Prüflinge (w0/sin(w0) = 1,003; Q 2 und 0,707). Das Modell gibt dort ab t_E 0,000 dB (Prüfling 1) und 0,000 beziehungsweise −0,018 dB (Prüfling 2, Plateau und Knie): die Messung bliebe grün und zeigte den Bruch nicht. Die Schließregel verlangt, dass die Zahl in ihrem genannten Geltungsbereich gilt.
- (4) „über das Dreifache Raum": 0,1 dB gegen 0,029 dB decken Rundung und Hüllkurvenrest, keinen Faktor 5,24 oder 9,15 in der Zeitkonstante.
- F-12 (5) kennzeichnet nur die Schranke vor t_E als Näherung; (3) setzt die analoge Zeitkonstante als die des Codes.
- Trägt die Näherung doch im Geltungsbereich? Im Modell hält sie bei Q 24 bis f0 um 18 kHz bei 48 kHz (18 kHz +0,091 dB, 19 kHz +0,224 dB, 3.4), darüber nicht; am unteren Q-Rand bricht sie schon bei 1 kHz (3.5). Der Fall des Prüfers ist kein Einzelpunkt.
- Rettet der Bandeinschwingvorgang den Randfall? Er senkt von rund +0,65 auf +0,57 dB, nicht unter 0,1 dB.

Keines davon fängt den Bruch auf.

### 5. Urteil: PRÄZISIERT

Der Kern ist bestätigt: F-12, M-77, M-78 und der §7.4-Nachtrag setzen die analoge Zeitkonstante an, der Code klingt mit der diskreten ab, und die 0,1 dB halten im genannten Geltungsbereich nicht. Anders als im Urteil:
1. Randfall: +0,57 dB bei t_E (Modell), nicht „≥ +0,6 dB"; der Bandpasszustand wirkt der Detektorlücke entgegen.
2. Unterer Q-Rand: langsamer Detektorpol 0,99799 und τ = 497,6 Samples bei 100 Hz, nicht 0,99784 und 463. Reelle Pole hat der Detektor für Q < 1/2 (ohne A), das Band für Q·A < 1/2. Der langsame Bandpol wächst mit fallendem A, das A_max aus F-12 (3) ist dort das falsche Ende. Dort überwiegt das Detektorüberschwingen, das Release hält; damit bricht auch „Hold und Release beliebig" (`:1473`). Beziffert: 1 kHz −0,48 dB, 100 Hz −0,46 dB (Plateau) und −0,39 dB (Knie) bei t_E.
3. „2·Q·A/sin(w0)" ist die Näherung für komplexe Pole; exakt 1/artanh(sin(w0)/(2·Q·A)), der Detektor mit A = 1.

### 6. Gebrochener Satz und kleinste Korrektur (Vorschlag an den Dirigenten)

**Gebrochen:** F-12 Geltungsbereich und Toleranz (`:1470-1479`, wörtlich unter 1) mit der Herleitung (3) (`:1500-1505`), dieselbe Zahl samt Geltungsbereich in M-77 (`:689`), M-78 (`:690`) und §7.4 (`:1248-1251`). Verletzt sind R-311-4 („bekommt eine Zahl … belegt sie in Etappe 4 mit einer Messung") und die Schließregel D-2 („Die Zahl gilt im ganzen vertragsgültigen Parameterraum … oder nennt ihren Geltungsbereich ausdrücklich"): sie gilt weder im Vertragsraum noch im genannten Geltungsbereich.

**Fassung A — exakte diskrete Formel.** t_E = max(`kFadeSamples`, 5·τ_a·fs + 10·τ_max + 16) Samples mit τ_max = max(τ(1/Q), τ(1/(Q·A_max)), τ(1/(Q·A_min))), A_min = 10^(min(0, g0, g0 + Range)/40), g = tan(π·f0/fs) und
- τ(k) = 1/artanh(k·sin(w0)/2) für k < 2 (komplexe Pole),
- τ(k) = −1/ln((Betrag(1 − g²) + g·√(k² − 4))/(1 + g² + g·k)) für k ≥ 2 (langsamer reeller Pol; bei k = 2 Doppelpol, Rest nach zehn Zeitkonstanten 11·e^−10).

Begründung an der Quelle: 3.1, 3.2; das Maximum über die drei k deckt den ganzen A-Bereich, weil τ(k) für k < 2 fällt und für k > 2 steigt. Wirkung im Modell: der Randfall bekommt t_E = 24 + 960 + 16 = 1000, ab dort 0,000 dB. Nicht geschlossen ist der untere Q-Rand: 1 kHz, Q 0,15, Knie, t_E nach A = 24 + 1009 + 16 = 1049, dort −0,44 dB (Release 100 ms), über 0,1 dB bis n = 8374; mit Release 5000 ms bei n = 29 999 noch −0,48 dB. Für den ganzen Vertragsraum braucht Fassung A zusätzlich einen Releaseterm (bei Release 5000 ms Sekunden) oder eine Q-Untergrenze.

**Fassung B — eingeschränkter Geltungsbereich, Formel unverändert.** F-12 `:1470-1476` neu: „… mit Quadraturton auf der Bandmitte f0 (L sin, R cos, E-25) im Teilraum Q ≥ 1 und Q·A_min ≥ 0,5 mit A_min = 10^(min(0, g0, g0 + Range)/40), f0 von 20 Hz bis min(20 kHz, fs/4), sonst im vertragsgültigen Raum von `eq-copilot/schemas/state/nakama-parameter-v2.json` (Attack 0,1 bis 500 ms, Grundgain g0 und Range je −12 bis +12 dB, Threshold −60 bis 0 dBFS, Hold und Release beliebig), fs 44,1 bis 96 kHz. Für Shelves, breitbandiges Material, f0 über fs/4, Q unter 1 und Q·A_min unter 0,5 nennt dieses Ticket keine Zahl; dort bleibt M-84 bei „innerhalb numerischer Toleranz"." Dazu in (3) nach „Q·A/(π·f0)": „— die analoge Näherung; die Zeitkonstante des Codes ist 1/artanh(sin(w0)/(2·Q·A)) Samples (`DspFilter.h:227-236`, `DspSvf.h:73-81`, `:97-109`), im Teilraum höchstens w0/sin(w0) ≤ π/2-mal so lang: zehn genäherte Zeitkonstanten sind mindestens 6,4 wirkliche, der Rest höchstens e^−6,4 = 0,0017 (0,015 dB). Q ≥ 1 hält das Überschwingen der kalt startenden Detektorleistung bei höchstens 0,003 dB, sodass Hold und Release wirkungslos bleiben; Q·A_min ≥ 0,5 hält die Bandpole komplex (bei Gleichheit kritisch gedämpft), sodass A_max die längste Bandzeitkonstante gibt."
- Begründung an der Quelle: artanh(x) ≥ x, also τ ≤ 2·Q·A/sin(w0) ≤ (π/2)·2·Q·A/w0 für w0 ≤ π/2 (3.1, 3.2); Überschwingtabelle in 3.5; Monotonie von τ(k) in 3.2.
- Folge für M-77: Prüfling 2 (Q 0,707) liegt außerhalb und geht auf Q 1,0. Dann t_E = max(256; 24 + 152,8 + 16 = 192,8) = 256, der Crossfade überwiegt weiter (Modell bei n = 193: Plateau +4,78 dB, Knie +1,89 dB; ab n = 256 höchstens 0,001 dB). Sein Rotbeweis „Einblenden doppelt so lang" (`DspKern.cpp:414`) gilt unverändert. M-77, M-78 und §7.4 übernehmen den Teilraum über ihre Verweise („Geltungsbereich und Herleitung §9 F-12", „samt Geltungsbereich").
- Reserve (Modell, ab t_E): 22 Fälle im Teilraum höchstens 0,002 dB, dazu der Hüllkurvenrest 0,029 dB aus (2) bei Attack 500 ms (Prüfling 1). Reserve mindestens 0,07 dB. Die Fälle: 12 kHz Q 24 (Knie, Plateau, g0/Range +12/+12, +12/−12), 12 kHz Q 1 (0/−12, +12/−12 mit Release 5000 ms), 12 kHz Q 2 −12/−12; 44,1 kHz 11 025 Hz Q 24 (0/−12, +12/+12) und Q 1 mit Release 5000 ms; 96 kHz 20 kHz Q 24; 20 Hz Q 1 (0/−12 Knie und Plateau, −12/+12); 100 Hz Q 2 −12/−12 (Knie, Plateau); 1 kHz Q 1 −6/−6 mit Release 5000 ms (Knie, Plateau); Prüfling 2 mit Q 1 (Knie, Plateau, Knie mit Release 5000 ms); Prüfling 1 (Attack 0,1 und 500 ms).

**Welche Fassung ist die kleinere, prüfbare Zusage:** Fassung B. Sie ändert einen Satz des Geltungsbereichs, eine Ergänzung in (3) und die Güte von Prüfling 2; Formel, Terme und Rotbeweise bleiben. Ihre Grenzen sind an zwei Ecken messbar. Fassung A schließt den Vertragsraum nicht ohne Releaseterm oder Q-Untergrenze, und ihr τ(k) mit Fallunterscheidung wäre selbst ein neuer Prüfgegenstand. Fassung A mit der Q-Untergrenze aus B deckte zusätzlich f0 bis 0,45·fs, bleibt aber größer.

**Dritter Prüfling:** nötig, damit die Messung der Etappe 4 die neue Grenze belegt; die zwei 1-kHz-Prüflinge liegen bei w0/sin(w0) = 1,003 und merken eine falsche Frequenzgrenze nicht. Vorschlag für Fassung B: dynamisches Bell f0 = fs/4 (12 kHz bei 48 kHz), Q 24, g0 0 dB, `stereo`, Range −12 dB, Threshold −15 dB (Knie), Attack 0,1 ms, Hold 0, Release 100 ms; t_E = 24 + 305,6 + 16 = 345,6 Samples, der Q-Term überwiegt; Modell ab t_E −0,001 dB. Rotbeweis nach S-1: die Detektorbandbreite halbiert, `DspFilter.h:227` `alpha = std::sin (w0) / (4.0 * q)` → rot, weil die Detektorzeitkonstante von 48,0 auf 96,0 Samples steigt, während die Spitzenverstärkung des Bandpasses 0 dB bleibt und A unverändert ist; bei t_E hat die kalte Detektoramplitude erst 1 − e^−3,6 = 0,973 erreicht, im Knie mit Steigung 1 Modell +0,195 dB bei n = 346, über 0,1 dB bis n = 409. Deterministisch: fester Ton, kein Zufall, keine Nebenläufigkeit. Die Q-Untergrenze belegt Prüfling 2 mit Q 1,0 (Q·A_min = 0,501) an ihrer Kante. M-77 trüge dann drei Teile mit je einer Mutation; §6.6 (`:739-740`) und die Beine in §7.4 (`:1257-1259`) folgen. Für Fassung A wäre ebenfalls ein dritter Prüfling nötig (20 kHz bei 48 kHz, Q 24, Knie, t_E = 1000).

## D-3 — M-17 bleibt unter der genannten Mutation nach dem Bau grün

### 1. Manifest am HEAD, wörtlich

- M-17 (`:585`), Zustand: „eq an, +12-dB-Bell bei 1 kHz auf Slot 0 (`setzeHoerbaresBand`, `eq-copilot/plugin/tests/SondeNullTestMain.cpp:128-134`, Q-Vorgabe 1,0), sonst Default; Material von `311/M-11`: …"; Zusage: „Die Neutralprüfung greift nicht: der Kern schreibt auch ab Sample 512 — dort kommt mindestens ein Rauschsample verändert heraus, das Band ist hörbar —, und an den sechs nicht endlichen Stellen kommt ein endlicher Wert heraus, nie bytegleich `0x7F800001`: …"; Rotbeweis: „**Regressionswache** (heute so: …); einmal gebrochen: die Bandbedingung des Merkmals „neutral" (jedes aktive Band ein statischer Einheitsbiquad ohne SVF, §7.2 Punkt 3) aus der Neutralprüfung genommen → rot, weil das Programm dann als neutral gilt — Trims, Width, Mix und Auto-Gain stehen auf Default, alle fünf Rampen ruhen auf 1,0 — und der Kern ab dem Schreibende (Sample 256) keinen Sample mehr schreibt: die Wachmarke kommt bytegleich heraus, und ab Sample 512 weicht kein Rauschsample mehr ab".
- §7.2 Punkt 3 (`:933-944`): „`baueProgramm` setzt ein Merkmal „neutral": engagiert und nicht im Hard-Bypass, jedes aktive Band ein statischer Einheitsbiquad ohne SVF (`statischIstEinheit`, `nutztSvf` falsch), M/S-Stufe aus (`msStufeAktiv` falsch), Input-Trim, Output-Trim und Auto-Gain im Kurzschluss bei 0 dB (`DspProgramm.cpp:184-185`, `:267`). `verarbeiteStueck` … bestimmt das Schreibende neu: … Committed-Pfad engagiert, kein Übergang, alle fünf Rampen in Ruhe (`Rampe::ruhtBei`, `DspKern.h:336`), und entweder (M-02-Zustand) Programm neutral mit allen Rampen auf 1,0 oder (M-33-Zustand) …"
- §7.2 Risiken (`:985-988`): „Risiko: das Merkmal „neutral" deckt einen Zustand ab, der doch rechnet, … abgesichert durch die Bedingung „Rampe ruht" statt eines Zielvergleichs und durch M-14 und M-17."
- Schließregel D-3 (`docs/beweise/roh/NAK-311-matrixnacharbeit-1-auftrag.txt:23`): „Die Zusage sagt, was am vorgeschriebenen Material am Basisstand wirklich herauskommt (heute grün), und fällt unter der genannten Mutation deterministisch (S-1)." S-1 (b) und (c) (`:9`): „… der zeigt, dass der Test DIESER Zeile unter DIESER Mutation nach dem im Bauplan §7 beschriebenen Bau deterministisch an der Zusage fällt; (c) ist die Zusage nach dem Bau doppelt gesichert, nennt die Zeile den Aufbau, der die eine Sicherung allein trägt, oder sie entfällt als eigene Rotbeweiszeile mit Verweis auf die Zeile, die die Zusage trägt".

### 2. Codebehauptungen des Prüfers, an der Quelle nachgemessen

| Behauptung (Urteil `:44`) | Quelle am HEAD | Ergebnis |
|---|---|---|
| `:267` ist der Kurzschluss des abgeleiteten Werts | `DspProgramm.cpp:267` `aus.autoGainLin = aus.autoGainDb == 0.0 ? 1.0 : dbInLinear (aus.autoGainDb);` | stimmt |
| unabhängig vom Schalter | `DspProgramm.cpp:263-266` „R4: immer gerechnet, angewandt nur bei eingeschaltetem Schalter (M-35)" mit `aus.autoGainDb  = leiteAutoGainAb (aus);`; `DspProgramm.h:146-150` „`autoGainDb` wird IMMER gerechnet …; angewandt wird es nur bei `autoGainAn`"; Schalter getrennt `DspProgramm.cpp:178` | stimmt |
| +12-dB-Bell: `leiteAutoGainAb` (`DspProgramm.cpp:85-136`) nie 0; Betrag² ≥ 1, um 1 kHz bis 15,85; Mittel über 1 | nachgerechnet über die 121 Stellen (`:76-83`, `:98-135`): kleinstes Betrag² 1,0004 bis 1,0015, Mittel 2,445 / 2,446 / 2,451, `autoGainDb` −3,882 / −3,885 / −3,894 dB bei 44,1 / 48 / 96 kHz | stimmt |
| M-17 meint den Schalter, angewandt über `DspKern.cpp:424` | `DspKern.cpp:424` `z.rampen.autoGain.setzeZiel (pn.autoGainAn ? pn.autoGainLin : 1.0);` — bei Schalter aus Ziel 1,0; angewandt nur über die Rampe (`:709`, `:720`); Default des Schalters false (`nakama-parameter-v2.json:116`) | stimmt |
| Material ist ein statischer Bell | `SondeNullTestMain.cpp:128-134` setzt nur occupied, enabled, 1000 Hz, +12 dB; Typ `bell` (`nakama-parameter-v2.json:146`), Q 1,0 (`:172`), `dynamic_enabled` false (`:212`) → `nutztSvf` falsch (`DspProgramm.cpp:222`) | stimmt |

### 3. Herleitung

- Welche Größe meint das Merkmal? §7.2 Punkt 3 nennt drei Größen „im Kurzschluss bei 0 dB" mit je ihrer Kurzschlusszeile: `inputTrimDb == 0.0` (`DspProgramm.cpp:184`), `outputTrimDb == 0.0` (`:185`), `autoGainDb == 0.0` (`:267`). Der Schalter ist ein bool (`:178`), hat kein „0 dB" und keine der zitierten Zeilen. Nach dem Wortlaut meint das Merkmal den abgeleiteten Wert.
- Wird `autoGainDb` bei ausgeschaltetem Auto-Gain abgeleitet? Ja, immer (`:263-266`). Das Rampenziel setzt der Kern bei Schalter aus auf 1,0 (`DspKern.cpp:424`); die Rampenbedingung der Neutralprüfung (§7.2, `:940-942`) wäre damit erfüllt — sperren würde das Merkmal.
- M-17 unter der Mutation: ohne Bandbedingung prüft das Merkmal weiter `autoGainDb == 0.0`; für den +12-dB-Bell ist `autoGainDb` = −3,88 dB, das Merkmal bleibt falsch, das Programm nicht neutral. Der Kern schreibt wie heute (Riegel 0,0 `DspKern.cpp:921-924`, Bell y = z1 `DspFilter.h:92`, geschrieben `(float) z1` `DspKern.cpp:1077-1079`): die Wachmarke kommt endlich heraus, das Rauschen ab Sample 512 verändert. M-17 bleibt grün. Rot würde es nur, wenn der Bau „Auto-Gain" als Schalter oder angewandte Rampe liest.
- Die zweite Sicherung ist unvollständig: der abgeleitete Term folgt aus der Bandbedingung. Ist jedes aktive Band ein statischer Einheitsbiquad, liefert `bandRuheAntwort` exakt (1, 0) (`DspProgramm.cpp:64`); ohne aktives Band greift `:96`; in beiden Fällen ist `autoGainDb` exakt 0,0 (`:96` oder `:134`). Im ungebrochenen Code entscheidet der Term also nie. Unter der Mutation sperrt er statische Bänder, deren Ruheantwort den Auto-Gain von 0 dB wegbewegt (der Fall von M-17), aber keine dynamischen Bänder mit Grundgain 0 dB. Deren Ruheantwort läuft über den SVF (`DspProgramm.cpp:63`), dessen Ruheentwurf bei 0 dB m1 = k·(A·A − 1) = 0 und m2 = 0 hat (`DspSvf.h:109-110`). `svfAntwort` liefert an jeder Stelle exakt (1, 0) (`DspProgramm.cpp:52`), die Summe ist 121,0, das Mittel 1,0, `:134` gibt 0,0. Dort trägt die Bandbedingung das Merkmal allein.

### 4. Gegenargument, aktiv gesucht

- SONDE-015 M-02, zitiert in §5.2 (`:230-232`), nennt als neutrale Bedingung „`auto_gain` = false", also den Schalter; ein Bau nach dieser Quelle machte die Mutation wirksam. Bindend für den Rotbeweis ist aber der Bau nach §7 (S-1 (b)), und §7.2 Punkt 3 zitiert `:267`. Nach dem beschriebenen Bau fällt M-17 nicht deterministisch.
- §7.2 Risiken (`:985-988`) nennt M-17 als Absicherung genau dieses Merkmals; die doppelte Sicherung nimmt ihr diese Wirkung.
- Kein anderer Weg macht M-17 unter der Mutation rot: `autoGainDb` ist bei allen drei Raten ungleich 0, und der Kern schreibt `(float) z1`.
- Einen Verweis nach S-1 (c) gibt es nicht: keine andere Zeile trägt die Bandbedingung (M-14 die Rampenruhe, M-18 die Hörmatrix, M-11 und M-12 die ganze Neutralprüfung).

### 5. Urteil: BESTÄTIGT

### 6. Gebrochener Satz und kleinste Korrektur (Vorschlag an den Dirigenten)

**Gebrochen:** der Rotbeweis von M-17 (`:585`), wörtlich unter 1 („… → rot, weil das Programm dann als neutral gilt — Trims, Width, Mix und Auto-Gain stehen auf Default …"), gegen die Schließregel D-3 („fällt unter der genannten Mutation deterministisch (S-1)") und S-1 (c).

**Korrektur K1 (kleinste, die D-3 und S-1 (c) schließt; ein Material nach dem Beispiel des Auftrags).** M-17 bekommt einen Teilfall mit dem Aufbau, der die Bandbedingung allein trägt: dynamisches Bell 1 kHz auf Slot 0, Grundgain 0 dB, Q 1, `stereo`, Sidechain `internal`, Range −12 dB, Threshold −60 dB, sonst Default, Material von `311/M-11`.
- Abgeleiteter Auto-Gain exakt 0,0 (`DspProgramm.cpp:63`, `:52`, `:134`; `DspSvf.h:109-110`), Trims 0 dB, M/S aus, Rampen bei Schalter aus auf 1,0 (`DspKern.cpp:424`): nur `nutztSvf` hält das Merkmal falsch.
- Heute grün: der Detektor hört das Rauschen weit über Threshold + 12 dB, die Auslenkung ist −12 dB, der Kern schreibt; an den nicht endlichen Stellen liefert das Band aus dem Riegelwert 0,0 den Wert m0·0 + m1·v1 (`DspSvf.h:80`), endlich.
- Einmal gebrochen: Bandbedingung aus dem Merkmal → rot, weil das Programm dann als neutral gilt und der Kern ab der Ruhe nach dem Übergangsende nicht mehr schreibt (spätestens ab dem nächsten Stück, §9 F-4, `:1395-1398`): die Wachmarke im ersten Block ab Sample 512 kommt bytegleich heraus.
- Der +12-dB-Bell-Fall behält seine Zusage; sein Rotbeweis nennt nach S-1 (c) diesen Teilfall als tragenden Aufbau, weil er selbst doppelt gesichert ist (Bandbedingung und abgeleiteter Auto-Gain −3,88 dB).

Kleiner, aber schwächer:
- **K3**, die gemeinsame Brechung von Band- und Auto-Gain-Bedingung, macht M-17 mit dem +12-dB-Bell deterministisch rot. Sie erfüllt die Schließregel D-3, nennt aber weder Aufbau noch Verweis nach S-1 (c) und ließe einen Verlust der Bandbedingung allein (dynamisches 0-dB-Band) unbemerkt.
- **K2** liegt am Bauplan statt an M-17: in §7.2 Punkt 3 „und Auto-Gain" und „`:267`" streichen. Das ist verhaltensgleich, weil der abgeleitete Term aus der Bandbedingung folgt und den angewandten Auto-Gain die Bedingung „alle fünf Rampen auf 1,0" deckt (`DspKern.cpp:424`); danach wirkt die bestehende Mutation von M-17 unverändert. K2 ist die kleinste Textänderung.

## B-1 — die Bitmusterhälfte von M-20 bei Ausgang (b) ist keine Regressionswache

### 1. Manifest am HEAD, wörtlich

- §7.2 Punkt 2 (`:926-930`, im Fixdiff geändert): „Der Bauer trägt das Ergebnis mit Datum in dieses Manifest ein: Ausgang (a) → T3-01-01 DEFEKT, die Bitmusterhälften von M-10 und M-20 heute rot; Ausgang (b) → LÜCKE, die Bitmusterhälfte von M-20 Regressionswache, die von M-10 ohne eigene Brechung (S-1 (c), Verweis M-11). Die Kopfzeile Klasse zieht der Dirigent nach." (vorher: „Ausgang (b) → LÜCKE, M-10 und M-20 Regressionswachen.")
- M-20 (`:588`, im Fixdiff unverändert), Rotbeweis: „**zwei Hälften:** Wachmarke heute rot (Gegenprobe `eq-copilot/plugin/dsp/DspKern.cpp:726` schreibt `dL[i]`); Bitmuster nach M-01. Rotbeweis: Mix-0-Zweig der Neutralprüfung entfernt → Wachmarke überschrieben → rot".
- M-21 (`:589`): „*Bitmuster je Rate* (M-10, M-20) nach M-01, das alle drei Raten misst: bei Ausgang (a) **heute rot** (…) und nach dem Bau mit derselben Mutation rot, weil `:893` und `:1077` die Muster wieder wandeln; bei Ausgang (b) entfällt diese Hälfte als eigene Brechung (S-1 (c)), die Neutralprüfung je Rate trägt die Wachmarkenhälfte dieser Zeile."
- M-10 (`:578`): „Bei Ausgang (b) entfällt diese Hälfte als eigene Brechung (S-1 (c)): nach dem Bau doppelt gesichert — die Neutralprüfung schreibt nicht, und die Rückwandlung ist nach M-01 (b) für das Bitmuster bittreu —, keine Einzelmutation macht sie rot; die Neutralprüfung allein trägt M-11 mit der Wachmarke, unabhängig von DAZ".
- §6.6 (`:743-745`): „Hälften und Unterfälle ohne eigene Brechung nach S-1 (c), je mit Verweis auf die tragende Zeile: M-10, M-19 und M-21 (Bitmuster bei Ausgang (b) von M-01), M-42 (b), M-49 (a); …" — M-20 fehlt.
- §7.2 Tests und Beine (`:967-969`): „Bei Ausgang (b) von M-01 tragen M-10, M-19 und M-21 für das Bitmuster keine eigene Brechung (S-1 (c)); die Neutralprüfung brechen M-11 und die Wachmarkenhälften von M-19 und M-21." — M-20 fehlt in beiden Aufzählungen.
- §6, Etikett (`:517-519`): „**Regressionswache**: heute grün und hält, was der Bau nicht ändern darf; sie wird einmal absichtlich gebrochen und ist kein Beleg"; Regel (`:530-531`): „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt, nie an einem Nebeneffekt."

### 2. Codebehauptungen des Prüfers, an der Quelle nachgemessen

| Behauptung (Urteil `:48`) | Quelle am HEAD | Ergebnis |
|---|---|---|
| ohne Neutralprüfung schreibt `DspKern.cpp:726` `dL[i]` | `:721-726`: bei nicht ruhender Mix-Rampe und `mix == 0.0` `{ l = dL[i]; rr = dR[i]; }`; `dL` ist `dryL` (`:645`), `dryL[i] = (double) kanaele[0][i]` (`:893`); Output-Rampe ruht bei 0 dB (`:711`, `:730`), Auto-Gain vor dem Mix überschrieben (`:720`, `:726`) | stimmt |
| über `:1077` zurück | `:1077` `float fl = (float) l;`, `:1079` `kanaele[0][i] = fl;` (Kanal 1 `:1082-1084`) | stimmt |
| bei Ausgang (b) nach M-01 bittreu | M-01 (`:569`): Ausgang (b) heißt „alle Muster bytegleich" in den Zuständen (a) M-02 und (b) M-33 unter derselben Maske (`SondeProcessor.cpp:337`); der Rückweg `(float) (double) x` ist damit für das Bitmuster die Identität | stimmt |
| die einzige genannte Brechung fällt an der Wachmarke | M-20 nennt nur „Mix-0-Zweig … entfernt → Wachmarke überschrieben"; die Wandlung nach double macht den signalisierenden NaN ruhig (§6.1 `:558-561`) | stimmt |
| M-21 führt die Hälfte nach S-1 (c); §6.6 und §7.2 nennen M-20 nicht | wörtlich unter 1 | stimmt |

### 3. Herleitung

Nach dem Bau schreibt die Neutralprüfung im M-33-Zustand nicht („Mix-Rampe ruht auf 0,0 und Output-Rampe auf 1,0", §7.2, `:943`). Die Mutation „Mix-0-Zweig entfernt" lässt den Kern `dL[i]` über `DspKern.cpp:1077` zurückschreiben. Bei Ausgang (b) ist das für das Bitmuster bittreu: die Bitmusterhälfte bleibt grün, nur die Wachmarke kommt ruhig heraus. Jede andere Einzelmutation am Schreibweg (`:726`, `:893`, `:1077`, die Maske `SondeProcessor.cpp:337`) bleibt unsichtbar, solange die Neutralprüfung nicht schreibt. Die Hälfte ist doppelt gesichert, keine Einzelmutation macht sie rot — derselbe Aufbau wie bei M-10 (b), für den M-10 und M-21 S-1 (c) anwenden. Bei Ausgang (a) macht dieselbe Mutation auch die Bitmusterhälfte rot, weil `:893` und `:1077` unter DAZ und FTZ die gemessenen Muster verändern; M-20 sagt das nicht, M-21 schon.

### 4. Gegenargument, aktiv gesucht

- Gilt „Regressionswache" in §7.2 Punkt 2 vielleicht der ganzen Zeile? Nein: der Satz nennt ausdrücklich „die Bitmusterhälfte von M-20", und nach dem Etikett „zwei Hälften" (`:525-526`) gilt je Hälfte ihr Etikett.
- Bricht eine Mutation an der Wandlung selbst die Hälfte? Nur zusammen mit einer zweiten, die die Neutralprüfung nimmt.
- M-21 fängt die Hälfte je Rate richtig auf, §7.2 Punkt 2 widerspricht ihm aber für dieselbe Hälfte.

### 5. Urteil: BESTÄTIGT

### 6. Gebrochener Satz und Fassung, die mit M-21, §6.6 und S-1 (c) übereinstimmt (Vorschlag)

**Gebrochen:** §7.2 Punkt 2 (`:928-929`) „die Bitmusterhälfte von M-20 Regressionswache" gegen das Etikett „wird einmal absichtlich gebrochen" (`:518`), die Regel „nie an einem Nebeneffekt" (`:530-531`) und S-1 (c).

- §7.2 Punkt 2 (`:927-930`): „… Ausgang (a) → T3-01-01 DEFEKT, die Bitmusterhälften von M-10 und M-20 heute rot; Ausgang (b) → LÜCKE, die Bitmusterhälften von M-10 und M-20 ohne eigene Brechung (S-1 (c), Verweis M-11 beziehungsweise die Wachmarkenhälfte von M-20). Die Kopfzeile Klasse zieht der Dirigent nach."
- M-20, Rotbeweis (`:588`): „**zwei Hälften.** *Wachmarke*: **heute rot** — Gegenprobe `eq-copilot/plugin/dsp/DspKern.cpp:726` schreibt `dL[i]`, `:1077` wandelt, der signalisierende NaN kommt ruhig heraus; nach dem Bau der Mix-0-Zweig der Neutralprüfung entfernt → rot, weil der Kern dann wieder über `:726` und `:1077` schreibt. *Bitmuster* nach M-01: bei Ausgang (a) **heute rot** (Gegenprobe: M-01-Rohdatei, Zustand (b)) und nach dem Bau mit derselben Mutation rot, weil `:893` und `:1077` die gemessenen Muster wieder wandeln; bei Ausgang (b) entfällt diese Hälfte als eigene Brechung (S-1 (c)): nach dem Bau doppelt gesichert — die Neutralprüfung schreibt im M-33-Zustand nicht, und der Rückweg `:726` → `:1077` ist nach M-01 (b) für das Bitmuster bittreu —; die Wachmarkenhälfte dieser Zeile trägt den Mix-0-Zweig der Neutralprüfung, unabhängig von DAZ."
- §6.6 (`:743-745`): „… je mit Verweis auf die tragende Zeile: M-10 und M-19 bis M-21 (Bitmuster bei Ausgang (b) von M-01), M-42 (b), M-49 (a); …"
- §7.2 Tests und Beine (`:967-969`): „Bei Ausgang (b) von M-01 tragen M-10 und M-19 bis M-21 für das Bitmuster keine eigene Brechung (S-1 (c)); die Neutralprüfung brechen M-11 und die Wachmarkenhälften von M-19 bis M-21."

Begründung an der Quelle: `DspKern.cpp:726`, `:893`, `:1077`; M-01 Ausgang (b) (`:569`); Neutralprüfung im M-33-Zustand (§7.2, `:943`); dieselbe Form wie M-10 (`:578`) und M-21 (`:589`). Die Zählungen in §6.6 ändern sich nicht (M-20 bleibt „zwei Hälften").

## H-a — Fakten zu M-11 und M-17 (keine Einordnung)

- **Kanäle nach dem Wortlaut:** nicht festgelegt. M-11 (`:579`): „wie M-10; im ersten Block nach Fade und Rampen liegen an drei Stellen die Wachmarke, dazu ein ruhiger NaN, +Inf und −Inf" — „Stellen" ohne Kanal, M-10 ist Stereo (`:578`). M-17 (`:585`): „im ersten Block ab Sample 512 dreimal die Wachmarke, ein ruhiger NaN, +Inf und −Inf" — ohne Kanal. §6.1 (`:555-561`) legt das Bitmuster „je Kanal" fest, die Wachmarke ohne Kanal. Nur M-19 (`:587`) nennt einen Kanal („auf Kanal 0", Mono).
- **Was der Riegel zählt** (`DspKern.cpp:919-925`): je Sample `double l = dryL[i], r = dryR[i];`, dann `if (! std::isfinite (l)) { l = 0.0; zaehlerEingaenge.fetch_add (1, …); }` und dasselbe für `r` — eine Zählung je nicht endlicher Komponente. In Stereo ist `dryR` Kanal 1 (`:894`), im Mono gilt `dryR[i] = dryL[i]` (`:894`): dort zählt ein nicht endlicher Wert zweimal. Die übrigen Schreiber desselben Zählers (`DspKern.cpp:1078`, `:1083`; `SondeProcessor.cpp:421`, `:428`) zählen nur endliche doubles, die als float nicht endlich werden; an den sechs Stellen tragen Band und Tap `post_committed` endliche Werte, dort zählt nichts dazu.
- **Richtige Zahl am Basisstand** (Stereo, M-11 und M-17 gleich, weil der Riegel in beiden Zuständen läuft): +6, wenn jeder der sechs Werte auf genau einem Kanal liegt (so das bestehende M-50-Material `SondeNullTestMain.cpp:334-336`, je Wert ein Kanal); +12, wenn jede Stelle den Wert auf beiden Kanälen trägt (so das Vorbild `DspGoldenTestMain.cpp:1020`: `wl[i] = wachmarke; wr[i] = wachmarke;`); dazwischen bei gemischter Lage. M-17 („steigt um 6") stimmt nur für die einkanalige Lage; M-11 („um die Zahl der nicht endlichen Werte") stimmt für beide, wenn „Werte" Kanalwerte zählt. Im Mono (M-19, nicht gefragt) ergäben sechs Werte auf Kanal 0 +12.

## Selbst nachgemessen

- `git rev-parse HEAD` zu Beginn und am Ende (Werte unten); `git status --short` zu Beginn: nur `briefing-hub/` und `nimbalyst-local/`; am Ende zusätzlich von einer parallelen Session geändert `.claude/skills/dirigent/SKILL.md`, `docs/PLAN-STAND.md`, `docs/offene-punkte.md`, `docs/plan/fragen.json` (nicht angefasst, außerhalb der geprüften Quellen) und dieser Bericht; `git diff --stat HEAD` über Manifest, Urteil, Nacharbeitsauftrag, `eq-copilot`, `broker`, `tools/fl` am Ende leer; `git log --oneline -8`.
- `git diff --stat ac62c734 HEAD -- eq-copilot broker tools/fl`: leer. `git diff --stat 7aa89ef2 HEAD -- eq-copilot/plugin/dsp eq-copilot/plugin/tests/SondeNullTestMain.cpp eq-copilot/schemas/state`: leer.
- `git diff -U0 70332210 HEAD -- docs/beweise/NAK-311.md`: nur `@@ -8 +8 @@` und `@@ -2060,0 +2061,34 @@` (Kopfzeile, §14); `git diff --stat HEAD -- docs/beweise/NAK-311.md eq-copilot`: leer; `git ls-files --eol docs/beweise/NAK-311.md`: `i/lf w/crlf`.
- `git diff 928c4c9a 70332210 -- docs/beweise/NAK-311.md`: §7.2 Punkt 2 vorher und nachher (unter B-1 zitiert); die Zeilen M-20 und M-01 im Fixdiff unverändert.
- Gelesen am HEAD: `DspFilter.h`, `DspSvf.h`, `DspProgramm.cpp`, `DspProgramm.h` vollständig; `DspKern.cpp` vollständig; `DspKern.h:60-451`; `DspBankPool.cpp:1-45`; `SondeNullTestMain.cpp:1-200`, `:296-455`; `DspGoldenTestMain.cpp:985-1039`; `SondeProcessor.cpp:330-429`; `nakama-parameter-v2.json:1-300`; Manifest `:101-145`, `:224-255`, `:468-597`, `:659-773`, `:872-993`, `:1133-1272`, `:1365-1564`, `:1710-2094`; Urteil der Matrixprüfung 2 vollständig; Nacharbeitsauftrag vollständig; Auftrag der Matrixprüfung 2 vollständig; Validierung der Matrixprüfung 1 `:210-236` (Form). Gesucht per Grep: „Kurzschluss", „Merkmal", „autoGainAn", `:267` im Manifest; `BandZustand`, `schrittRest`, `zustaendeNullen` im DSP-Code; `isfinite` und `zaehleVerriegelteVerengungen` in `SondeProcessor.cpp`.
- Gerechnet (PowerShell-Ausdrücke, nur Arithmetik, keine Datei): Polradien und Zeitkonstanten aller Fälle in 3.3 nach 3.1 und 3.2 sowie direkt aus den RBJ-Koeffizienten; das Überschwingen der kalten Detektorleistung (Detektor allein, Q 0,15 bis 24, f0 100 Hz bis 20 kHz, bis 60 Zeitkonstanten); `autoGainDb` des +12-dB-Bells (Nachbau von `biquadAntwort` und des Gitters, `DspProgramm.cpp:26-31`, `:76-83`, `:98-135`, `DspFilter.h:116-131`); t_E nach F-12 für jeden Fall.
- Modell der Einschwingzahlen: Nachbau der Rekursionen `DspFilter.h:90-96` (Detektor je Komponente, Koeffizienten `:224-238`), `:312-328` (Hüllkurve, Hold 0), `:336-341`, `:355-361`; `DspSvf.h:73-81`, `:93-112`; `DspKern.cpp:25-37` (Koeffizientenmischung), `:531-584` (Detektorleistung, Steuerschritt je 8 Samples, Mischung, SVF je Komponente), `:762-768` und `:796-813` (Einblenden aus Dry über 256 Samples). Ein dynamisches Stereo-Bell mit Quadraturton 0,5 (L sin, R cos); A eingeschwungen (Vorlauf 20 000 bis 40 000 Samples, bis 2000 Samples vor t = 0 die Detektorleistung als Hüllkurve übernommen, danach die echte Hüllkurve); B kalt ab t = 0 mit Startzustand nach `DspBankPool.cpp:8-18` und `DspKern.cpp:266-271`. Maß je Sample 20·log10(Betrag(B)/Betrag(A)). Die Phase des Tons hat keinen Einfluss (0° und 90° gleich, Quadraturbetrag).
- Eichung des Modells an Zahlen aus Manifest und Messung: Prüfling 2 bei n = 149 Plateau +7,030 dB und Knie +2,996 dB (Manifest `:1520-1521`: „+7,03 dB", „rund +3,0 dB"); A im Knie Pegel −9,031 dB und Auslenkung −5,969 dB (`:689`: −9,03 und −5,97 dB); Prüfling 1 mit Attack 500 ms Mittel über Sekunde 1 bis 2 +0,265 dB gegen die Phase-16-Messung +0,267720 dB (`:1523`), bei t_E = 2,507 s +0,029 dB (F-12 (2): 0,029 dB).
- Dateiende und Zeilenenden dieses Berichts: LF, geprüft nach dem Schreiben.

## Nur gelesen / nicht geprüft

- Kein Bau, kein Test, kein Kanon, kein Prozess des Produkts, kein Python, kein FL. Jede Rot- und Grünaussage ist hergeleitet.
- Das Modell ist ein Nachbau, nicht der Produktcode: es rechnet durchgehend in double (die float-Wandlung am Hostpuffer `DspKern.cpp:893`, `:1077` fehlt, relativ unter 10^−7), mit einem Band, in Stereo, mit festem Quadraturton; Blockgrenzen ändern die Rekursion nicht, `heileZustaende` ist nicht nachgebaut (greift bei endlichen, nicht denormalen Zuständen nicht).
- Fassung B ist für die Zeitkonstanten analytisch begründet und an 23 Fällen im Modell geprüft, nicht über den ganzen Teilraum bewiesen: Threshold nur −15 dB (Knie) und −40 dB (Plateau), Hold 0, Attack 500 ms nur an Prüfling 1, fs 44,1 und 96 kHz nur an Einzelpunkten.
- K1 zu D-3: dass das dynamische 0-dB-Bell das Rauschen hörbar verändert, ist abgeschätzt (Detektorpegel des Rauschens um −17 dB, weit über −48 dB), nicht gerechnet; die Stückgrenze nach F-4 hängt am Blockaufbau des neuen Falls.
- Nicht nachgerechnet: die übrigen Phase-16-Werte; die Zusagekette von M-17 am Basisstand (vom Prüfer als erfüllt geführt).
- Nicht Gegenstand: D-1, D-4 und alles, was die Matrixprüfung 2 als geschlossen führt; §7.3, §7.5; SONDE-015; U44 bis U47, U54, Etappe 5; T3-12-*; Legacy-Bezeichner, Design, Stil.

## Beifang

- F-12 (5), Schranke vor t_E (`:1479-1482`): am unteren Q-Rand fällt B/A unter die untere Schranke — Modell 1 kHz, Q 0,15, g0 0 dB, Range −12 dB, Knie, Attack 0,1 ms, Release 100 ms: −0,44 dB bei n = 255 gegen min(0, −g_A, −(g0 + g_A)) − 0,1 dB = −0,1 dB; Ursache wie D-2 3.5 (Überschwingen, das Release hält). Fassung B (Q ≥ 1) schließt es mit.
- M-01 (`:569`) sagt für Ausgang (b) weiter „M-10 und M-20 sind Regressionswachen"; das widerspricht M-10 (`:578`, S-1 (c)) und der B-1-Fassung; dieselbe Korrektur.

HEAD zu Beginn: `a95960f11a33e613e7eef618af7c9e4a7e7fee2a` · HEAD am Ende: `a95960f11a33e613e7eef618af7c9e4a7e7fee2a`
