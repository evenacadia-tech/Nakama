# EQ-Copilot Kalibrier-Protokoll — Teststück 01

Zweck (User-Protokoll 2026-08-15): dasselbe Notenmaterial (testsong-01,
613 Noten, deterministisch, `verify_testsong.py` 7/7 grün) wird in
wechselnde Instrumente geladen und ohne (später: mit) Effekten gerendert.
Jede Runde vergleicht **drei Sichten auf dieselbe Datei**:

1. **Offline-Analyse:** `py -3.13 tools/analyze-track.py <wav>` — exakt die
   Referenzachse, gegen die die Plugin-Engine kreuzvalidiert ist.
2. **Plugin-Befund:** Snapshot-Datei aus dem Festhalten-Knopf
   (`%LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\`) — trägt seit v3
   Befundkarten, Perzentile, Zonen-Zeitverlauf und Konvergenz.
3. **Ohren des Users** — das letzte Wort.

Abweichungen zwischen 1 und 2 sind Mess-Bugs (sofort fixen). Abweichungen
zwischen 1+2 und 3 sind **Kalibrier-Futter**: Schwellen/Regeln der Diagnose
(versionierte Startwerte) gegen echtes Material nachziehen. Das ist kein
ML-„Training" — es ist Schwellen-Kalibration mit Beleg.

## Feste Erwartungen (VOR Runde 1 festgelegt — Prüfstein für beide Seiten)

- **E1 Tonika-Ballung um A#3/A#4 (117/233 Hz):** Klavier-LH, Gitarren-Ostinato
  und Bass-Obertöne treffen gehäuft das A#-Band (unser Golden-Fixture-Band).
  Interessant: meldet der Copilot dort eine Resonanz — und wenn ja, wie
  ordnen wir „Tonika ist erwartbar laut" ein? (Kandidat für die
  Tonart-Einordnung aus der Benchmark-Studie.)
- **E2 Zonen-Zeitverlauf:** Schlagzeug/Gitarre/E-Pad spielen nur T9–16
  (⅓ der Zeit). Härte-/Höhen-Befunde (falls überhaupt) müssen als
  „~30–40 % der Musikzeit" erscheinen, nie als „durchgehend".
- **E3 Lautheitssprung T8→T9** (alles setzt ein + Crash): LUFS-Short springt;
  Integrated liegt dazwischen; Crest bleibt hoch (keine Kompression).
- **E4 Ohne Effekte kein Höhen-Hype:** Air-über-Präsenz-Karte darf bei rohen
  akustischen Instrumenten praktisch nie erscheinen.
- **E5 Chor doppelt Klavier-Voicings (bewusst):** 230–700 Hz dürfte sich
  füllen/stauen — ob das als Mulm-Karte kommt, hängt von den gewählten
  Instrumenten ab. Das ist die spätere Masking-Testfläche (M3c).
- **E6 „KURVE STEHT"** muss bei 2 Durchläufen (~96 s) erscheinen.

## Die Soundreferenz (USER-VORGABE 2026-08-15)

**Gareth Coker — „Light of Nibel (feat. Aeralie Brighton)"** (Ori and the
Blind Forest). Datei lokal (gitignoriert), Messwerte in
`referenz-light-of-nibel.json`. Ab jetzt der Maßstab für Korridor- und
Matching-Arbeit (`py -3.13 tools/match-reference.py <mix> "<referenz>"`);
weitere Referenzen können den Korridor später verbreitern (Plan §5.4).

Kern-Profil: LUFS −12,0 · **LRA 16,2 LU** (Short-term −25,3…−7,0 — echte
Filmmusik-Dynamik trotz lautem Master) · TP −0,1 dBTP (Soundtrack-typisch
über dem −1-Ceiling — NICHT nachahmen, −1 dBTP bleibt unsere Grenze) ·
**Mitten 500–2 k = 40,6 % der Energie** — die Referenz bestätigt Hebel #1
(alle sechs eigenen Master scoopten genau da) · Helligkeit 1758 Hz, Neigung
−12,8 dB/Okt, über 2 kHz nur 2,2 %, Air ≈ 0 — **warm, kein Höhen-Hype** ·
mud 0,9 / harsh 1,2 dB (sauber) · width 0,44 / corr +0,39 (breit, mono-fest)
· Tiefton-Einsatz erst bei 28 % des Tracks (dieselbe Dramaturgie wie ffff).

## Runden-Log

### Runde 01 — Roh, ohne Effekte — ABGESCHLOSSEN 15.08.2026

| Feld | Wert |
|---|---|
| Datum | 2026-08-15, nachmittags (Plugin-Stand m4/C9AD6ADF) |
| Instrumente | 01=Claire · 02=Noire · 03=Celestial Strings · 04=Angelic Choir · 05=FFOSSO Liminal Winds · 06=Absynth 6 Acoustic Bass · 07=SE Percussion Field Drum + Bassdrum (User-Split, GM-Pattern passte nicht) · 08=FFOSSO Western Guitar |
| Render | `kalibration/Testtrack.wav` (44,1 kHz, **124 s — davon 77,7 s Stille!**, versehentlich 130 BPM — für Spektral-Kalibration egal) + `Testtrack.flp` |
| Snapshots | `snapshot-20260815-150636` (rolle sensor) · `…-150812-Master` (rolle hub) — 2 Messpunkte, praktisch identische Werte |
| Offline | LUFS −22,41 · TP −7,87 · Crest 20,8 · Centroid 829 Hz · Rolloff 318 Hz · low_frac 0,78 · width 0,33 · corr 0,50 · Top-Resonanzen 546,6/919,3/354,4/459,6/229,8/688,7 Hz |
| Plugin | LUFS −22,45/−22,48 · TP −7,75/−6,47 · Centroid 876/879 · width 0,328 · corr 0,507 · Resonanzen 688,7 (+8,7) / 459,6 (+7,8) / 919,3 (+6,5) — 2 Karten (F6 mittel · A#5 hoch), Zonen-Karten still, Konvergenz 0,51–0,55 |

**Mess-Integrität: BESTANDEN.** LUFS Δ≤0,07 LU · TP Δ0,12 dB (Sensor) ·
width/corr Δ<0,01 · low_frac Δ0,02 · die Resonanz-Bänder decken sich
FREQUENZGENAU (identische 1/24-Bänder). Die scheinbare LTAS-Differenz
(+6 dB konstant, 208/208 Bänder Plugin>offline) ist vollständig erklärt und
KEIN Fehler: **−4,3 dB Stille-Verdünnung** (analyze mittelt die 77,7 s Stille
der Datei mit; Engine gated) + **+1,2…+2,5 dB Mid-Mix↔Kanalenergie** (analyze
misst 0,5·(L+R), Engine (L²+R²)/2 — echtes Stereo). Crest-Differenz (20,8 vs
16,4/17,1) = dieselben zwei Definitions-Unterschiede.

**Erwartungen:** E1 ✓ (Tonika-Ballung real, aber A#5/A#6 statt A#3/A#4 —
Melodie+Pads dominieren, nicht die linke Hand; offline fand auch 229,8=A#4) ·
E2 → als K1-Fund umgeleitet · E3 ✓ (LRA 5,7) · E4 ✓✓ (kein Höhen-Hype, Track
extrem dunkel: 0,24 % Energie über 2 kHz — Instrumentwahl) · E5 ✓✓ (ALLE
Resonanzen liegen in der Chor/Klavier-Ballung 230–700 Hz) · E6 ✗ knapp
(Konvergenz 0,51/0,55 > 0,35 — Stück kurz + dreiteilig; Startwert beobachten,
noch nicht ändern).

**Kalibrier-Funde → umgesetzt als m4.1 (beide GOLDEN-bewiesen 239/239):**
- **K1 Tick-Mindestpegel:** `zonen_zeit` meldete „Härte 50 %" auf einem Track
  mit 0,24 % Energie über 2 kHz (Schulterlinien-Geometrie auf Rauschteppich;
  die KARTEN schwiegen korrekt dank Abdeckungs-Gate). Fix: Zeitverlaufs-Tick
  zählt nur, wenn jede Region ≥ Teppich+10 dB (−80 dB) echten Pegel hat.
- **K2 Wellen-Werkzeugwahl:** Die A#5-Karte (=die MELODIE, Tonika!) sagte
  „dauerhaft + kommt in Wellen (~15 dB)" und empfahl trotzdem einen festen
  Cut. Fix: bei Spanne > 10 dB wird die Erstidee dynamisch (Smooth Operator),
  auch wenn dauerhaft — Warum-Zeile nennt ausdrücklich „oft ist so ein
  wogender Dauerton die Melodie".
- **K3 Vergleichs-Protokoll (Werkzeugseite):** Offline↔Plugin-Kurvenvergleiche
  müssen aktivitäts-gegated + kanalenergie-basiert rechnen — gilt auch für
  den M3b-Korridor-Kalibrator. analyze-track bleibt unangetastet (seine Achse
  ist der Skalar-Maßstab; die Kurven-Differenz ist Definitions-Physik).
- Vormerkung: Tonart-Einordnung („A#5 = Tonika = erwartbar laut") braucht ein
  Tonart-Feld — kommt mit M3b-Profil. · Für Runde 2+: Render am Songende
  schneiden (Stille kostet nur Vergleichs-Klarheit, keine Messrichtigkeit).

_(Folge-Runden: gleiche Tabelle. Geplante Achsen: andere Instrumente ·
gezielt „schlechte" Instrumentwahl (Mulm-Kandidaten) · mit einzelnen
Effekten (EQ-Eingriff → PRE/POST-Paar, M3c) · mit Master-Kette. Nächster
Install-Stand m4.1 = `B87AB70E…` gestaged, wartet auf FL-Schließen + UAC.)_

### Runde 02a — Feldvergleich „chor, streicher, piano" (2026-08-16, ungeplant)

Kein Blindtest (der bleibt als Runde 2 offen), sondern ein User-Spontanvergleich —
wertvoll, weil er die **Protokoll-Falle Material-Identität** aufgedeckt hat.

| Feld | Wert |
|---|---|
| Ablauf | 02:02 Snapshot (hub, 209,3 s aktiv) → 02:03:49 Render `Downloads\test.wav` (8 s) → Messung neu gestartet, ~37 s andere Passage gespielt → 02:10 Snapshot (36,6 s aktiv) |
| Offline (test.wav) | LUFS −28,4 · TP −18,0 · B♭ Moll · Resonanzen 229,8 (+15,5) / 473,1 (+14,7) / 2526,1 (+14,4) / 1417,8 (+11,0) · mud 2,7 dB · >6 kHz 0,0 % |
| Plugin (02:10) | LUFS −22,2 · TP −11,3 · Kandidaten 2526,1 (+15,8) / 459,6 (+14,2) / 1686,0 (+12,6) / 102,4 / 136,7 / 1417,8 — 2 Karten (D#8 hoch · A#5 hoch), Zonen still, haerte/hype-Gates zu |

**Befund: Plugin rechnet RICHTIG — es hörte nur unterhalb ~400 Hz anderes
Material.** Ein Python-Nachbau von `findeResonanzen` (Median ±6, Breite ≤1/6 Okt,
Abstand 6, Deckel 6) reproduziert die 6 Snapshot-Kandidaten **auf 0,01 dB**.
Oberhalb ~400 Hz sind Plugin-Komposit und test.wav-LTAS frequenzgenau deckungs-
gleich (460/1418/1686/2124/2526 = identische 1/24-Bänder, normierte Deltas
≤ ±2,7 dB). Unterhalb 400 Hz Deltas bis 12 dB im Zickzack (230 −7 / 300 +8) =
andere Akkorde im Livepass (G#/C#-Bässe statt A#-Grundton), kein EQ-/Messmuster.

Die „fehlende" 230-Hz-Resonanz: in der 02:10-Kurve real nur +9,21 dB = Platz 9
von 15 Peaks ≥6 dB — **der Kandidaten-Deckel (kMaxKandidaten=6, Platz 6 =
+9,94) schnitt sie ab.** Auf test.wav-Material stünde sie bei +14,05 dB =
Kandidat #3. Kein Fix nötig; Karten (Top 2) unberührt.

- **K5 Protokollregel Material-Identität:** Für jeden Vergleich MUSS die App
  exakt das gerenderte Audio hören — **den Render selbst in FL loopen**
  (≥60 s aktiv), Messung nach dem Render neu starten. Projekt-Livespiel
  vergleicht Passagen, nicht Messungen.
- **Beobachtung Kandidaten-Deckel (offen, nicht ändern):** dicht-tonales
  Material (hier 15 Peaks ≥6 dB in 80–5000 Hz) füllt den 6er-Deckel; starke
  echte Resonanzen können knapp rausfallen, Zwillingspeaks (1686/1892, 1/6 Okt)
  kippen die Rangfolge per Abstandsregel. Erst nach mehr Runden entscheiden,
  ob der Graph mehr Ticks verdient.
