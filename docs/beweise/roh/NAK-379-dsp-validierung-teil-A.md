# NAK-379 — Quellvalidierung der DSP-Fachprüfung, Teil A (DSP-01 bis DSP-08)

**Datum:** 23.09.2026 · **Validierer:** Opus 5.5, Effort max, frisch und lesend ·
**Auftrag:** `docs/beweise/roh/NAK-379-dsp-validierung-teil-A-auftrag.txt` ·
**Prüfgegenstand:** `docs/audits/2026-09-23-dsp-fachpruefung/BEFUNDE.md` §2, Zeilen 81-88.

**HEAD vorher:** `3da624e0a313fea92304c7ef70f1cd71e2604258` · **HEAD nachher:** `3da624e0a313fea92304c7ef70f1cd71e2604258` (unverändert)

**Arbeitsmodus.** Nur Lesewerkzeuge und lesende git-Befehle; die Audit-Skripte
unter `skripte/` gelesen, nicht ausgeführt. Eigene Nachrechnung mit `py -3.13`
(numpy 2.4.4, scipy 1.18.0) in drei eigenen Skripten im Job-Scratchpad außerhalb
des Repos (`C:\Users\phili\.claude\jobs\2f962554\tmp\val_a1.py`, `val_a2.py`,
`val_a3.py`; flüchtig, Rechenweg je Befund unten beschrieben). Zwei Marktangaben
und ein Literaturzitat per Websuche an der Herstellerhilfe beziehungsweise der
AES-E-Library geprüft. Kein Kanon, kein Bau, kein FL. Geschrieben wurde nur
diese Datei.

**Stand des Prüfgegenstands.** Der Bericht rechnet an `4a831da3`.
`git diff --stat 4a831da3 3da624e0 -- eq-copilot/ broker/ docs/FL-Nakama-Sonden-Design-Entwurf.md docs/beweise/SONDE-015.md design/abnahmen/ docs/plan/`
ist leer bis auf `docs/offene-punkte.md` (+3/−1, NAK-378 und NAK-379). Alle
Code-, Vertrags- und Manifestzeilen des Berichts gelten damit am HEAD unverändert.

**Erreichbarkeit heute (gilt für alle Befunde außer DSP-03).** Der Prozessor
legt jeden Host-Parameter des Vertrags an (`eq-copilot/plugin/sonde/SondeProcessor.cpp:94-125`),
darunter `v2.global.eq_enabled`, `v2.global.mix`, `v1.global.mono_bass_hz` und
alle Bandwerte samt `dynamic_enabled` (`nakama-parameter-v2.json`, je
`"host_parameter": true`). Der EQ-Kern ist damit heute über FL-Automation und
die generische Parameteransicht bedienbar, auch ohne die geparkte EQ-Oberfläche.

---

## DSP-01 — Koeffizientenrampe statischer Bänder überschwingt

**1. Quelle.** Alle Stellen existieren und sagen das Behauptete:
`DspKern.cpp:39-52` (lineare Mischung der fünf Koeffizienten; Kommentar „Stabil
ist sie, weil die Menge stabiler Nennerpaare … konvex ist"), `:360-366`
(`bleibt` mit den drei Wertekriterien), `:586-603` (Kennungsvergleich,
`nurRampen`, Zustandsübernahme `bankNeu.baender = baenke.bank (alt).baender`),
`DspProgramm.h:150-212` (2,0 / 4,0 / 20 dB, strikt größer reißt),
`SONDE-015.md:3412` (E-19 mit der Konvexitätsbegründung). Laroche 2007 geprüft:
JAES 55(6):460-471, „On the Stability of Time-Varying Recursive Filters".
Die Phase-15-Fälle laufen heute als Crossfade und sind als `311/M-96` bis
`311/M-98` im Golden (`DspGoldenTestMain.cpp:935-947`).
**Bekannt · Ort:** NAK-336 (d) existiert („die drei Sprungkriterien
(2,0 / 4,0 / 20 dB) am Maß nachschärfen, der Rampenweg hält es bei der
Frequenz bis 8,0") und deckt den Befund **teilweise**: die Richtung stimmt, die
Begründung nicht. Die Aussage „hält es bis 8,0", wiederholt in NAK-337 und in
der Messreihe `docs/beweise/NAK-311-verlauf.md:9145-9190` (§57.3: „das Maß
allein trüge bei der Frequenz bis 8,0, bei Güte und Gain über die ganze Reihe"),
beruht auf **einem** Prüfling (Low-Shelf +6 dB Q 0,707, Ton 1 kHz). DSP-01
widerlegt ihre Verallgemeinerung. S25p (NAK-340) existiert; Etappe 0 nennt
„Kriteriensprünge" ausdrücklich — tragfähiger Ort, inhaltlich deckt NAK-340 den
Befund nicht (es geht dort um Bandautomation im Export).

**2. Formel und Zahl.** `gegenpruefung-eq-1/v5_rampe.py` ist gegen den Code
getreu: DF2T-Tick wie `DspFilter.h:90-96`; Mischung aller fünf Koeffizienten
wie `DspKern.cpp:43-52` (Aufruf `:902`); t = (j+1)/256 wie `:793-795` mit
`z.rest = kRampeSamples` (`:633`); Zustand wandert mit (dort analytischer
Dauerzustand von A, gleichwertig); Kriterienfilter strikt größer; Maß wie
R-311-13. Die Referenz „Sweep-Physik" (TPT-SVF, Parameter je Sample log f,
log Q, dB) ist eine Wahl des Audits und überschwingt bei Güte-Absenkung am Bell
selbst (Mechanismus DSP-02); der „Verfahrensanteil" E ist darum
referenzabhängig, belastbar ist die Produktspitze P.
**Eigene Rechnung** (`val_a1.py`: Zeitbereichsvorlauf 4 s mit Programm A statt
analytischem Zustand, 64 Umschaltphasen, 48 kHz): High-Shelf 80→40 Hz,
Q 24→6, +10→−10 dB, Ton 68 Hz — P max **+22,77 dB** (Bericht +22,8), SVF-Sweep
max +9,17 dB, E max **+17,03 dB** (Bericht „bis +17"). Kontrolle am
Phase-15-Fall mit erzwungener Rampe: +17,41 dB (`DspProgramm.h:129-132`: +17,41).
Stationär am Ton: A −6,27 dB, B −15,98 dB; die Spitze liegt damit bei +16,5 dB
re Eingang, unter der eigenen Resonanzspitze von A (+34,3 dB bei Q 24).
**Dauer:** gleitendes Periodenmaximum über stationär +6 dB bis 131 ms, über
+3 dB bis 151 ms. Die Anteile 31 % / 9,7 % / 2,8 % stehen so in
`v5b_verfahrensanteil.txt:1`; sie beziehen sich auf ein Raster aus Grenzfällen,
nicht auf die Häufigkeit im Gebrauch. Mit Q ≤ 6 an beiden Enden: E max +8,43 dB
(`:32`). Gegen die höchste stationäre Spitze beider Einstellungen über alle
Frequenzen übersteigt die DF2T-Rampe im Raster `nachrechnung/r5b_ausgabe.txt:4-9`
höchstens +6,0 dB (Bell) beziehungsweise +5,5 dB (Shelves).

**3. Zusagenlage.** Unterhalb des Kriteriums **schweigen alle**. R-311-13
(`NAK-311-verlauf.md:4513-4524`) bindet das Maß an den Crossfade und an einen
Prüfling: „Maß: die Spitze im Fenster von 0,5 s nach dem Wechsel liegt höchstens
1 dB über dem größeren der beiden stationären Pegel (Prüfling der
Quellvalidierung Teil B §1.5, heute +17,41 dB). Eine Änderung unter dem
Kriterium und dieselbe Strecke in kleinen Stufen bleiben auf dem Rampenweg,
bitgleich zum Basisstand." U44 (`design/abnahmen/2026-09-19-grosser-eq-sprung-ueberblenden-u44.md:26-31`):
„Kleine Änderungen und langsame Reglerfahrten laufen weiter über die heutige
Glättung und klingen wie bisher." — die Grenze ist ausdrücklich „Technik".
Entwurf `:2990-2991` „Stetige Parameter werden geglättet"; `:3109`
„Automations-Zipperresiduen bleiben im definierten Ramp-Test unter −100 dBFS" —
der definierte Test vergleicht gegen dieselbe Koeffizientenrampe
(`DspGoldenTestMain.cpp:3985-4050`, vgl. DSP-10). Einordnung „Lücke" trägt.

**4. Urteil: PRÄZISIERT** — Klassenvorschlag **LÜCKE (mittel)** plus **DOKU**
(„hält bis 8,0" in §57.3, NAK-336 (d), NAK-337 an einem Prüfling
verallgemeinert). **Wirkung:** Größe am betroffenen Ton stimmt, „für
Millisekunden" nicht — die Spitze steht bis etwa 0,13-0,15 s. 10-20 dB treten
im Raster nur mit Q 24 an mindestens einem Ende auf (Shelves und Glocken im
Bass, hochgütige Shelves auch bei 1-8 kHz); mit Q ≤ 6 an beiden Enden bis etwa
8-10 dB. **Heute erreichbar:** ja.
**Kleinster Fix / Ort:** NAK-336 (d) erweitern, in S25p (Etappe 0
„Kriteriensprünge") lösen; das Maß an einem Prüflingsraster statt an einem
Prüfling festlegen. Dabei beachten: „je Ton ≤ 1 dB über dem größeren
stationären Pegel" hält bei hochgütigen Shelves auch der Crossfade nicht
(eigene Rechnung: High-Shelf 80 Hz Q 6 −10 dB → 40 Hz Q 24 +10 dB, Ton 35,6 Hz:
+9,23 dB je Ton, aber −22,4 dB unter der höchsten stationären Spitze); der
DSP-01-Höchstfall selbst liefe als Crossfade mit 0,00 dB.

---

## DSP-02 — SVF-Bell überschwingt bei schneller Güte-Absenkung

**1. Quelle.** `DspSvf.h:102-112` (k = 1/(q·A), m0 = 1, m1 = k(A²−1), m2 = 0;
Tick `:73-81` gibt m0·x + m1·v1 + m2·v2) und `DspKern.cpp:871-895` (Neuentwurf
bei `schrittRest <= 0`, `q = lerp (qb->q, q, t)`, `svfVon = svfNach`, Gewicht
`tSchritt = 1 − schrittRest/8`) stimmen. Güte 24→6 bleibt auf dem Rampenweg:
`sprungImVerhaeltnis (24, 6, 4.0)` prüft 24 > 24, falsch (`DspProgramm.h:190-199`).
Filterzustand samt `svfVon`, `svfNach`, `schrittRest` wandert mit
(`DspBankPool.h:58-86`, `DspKern.cpp:601`). **Bekannt · Ort:** „neu" stimmt —
keine Registerzeile zu SVF oder Güte-Rampe; der einzige Rampengolden
(`DspGoldenTestMain.cpp:3985-4050`) prüft statische Bells (Gain 3→9, Frequenz
1000→1400, Güte 1→3), kein SVF-Band.

**2. Formel und Zahl.** `nachrechnung/r5e_qsprung.py` nutzt `lauf` aus
`r5b_kriterienscan.py:57-100`: Entwurf am Steuersample d mit
t = (d − n0 + 1)/256, Gewicht (n − d)/8, also 0 am Entwurfssample — getreu,
zwei Steuerphasen. `eigen/svf_qrampe.py` weicht leicht ab: Gewicht
(i mod 8 + 1)/8 statt 0 bis 7/8 und Güte-Index (i − iw + 8)/256 statt
(j + 1)/256 — je wenige Samples Versatz, für die Spitze unerheblich.
**Eigene Rechnung** (`val_a1.py`: Produktschema mit `schrittRest` wörtlich,
8 Steuerphasen × 4 Tonphasen, Vorlauf 3 s), +12 dB, Q 24→6, Ton auf f0:
**+9,94 dB** bei 50 Hz (Bericht +9,7; r5e +9,8), **+8,06 dB** bei 1 kHz
(Bericht +7,9). Obergrenze bei eingefrorenem v1:
20·log10((1 + (A²−1)·4)/A²) = +10,23 dB. Derselbe Wechsel als DF2T-Rampe
+0,00/+0,02 dB; Q 6→24 −0,00 dB. **Ergänzend** (`val_a3.py`): Q 24→6 als
lineare Fahrt über 50 / 200 / 1000 ms ergibt bei 50 Hz +8,83 / +6,31 / +1,73 dB,
bei 1 kHz +1,73 / +0,39 / +0,08 dB. Absenkung −12 dB, 50 Hz, Q 24→6 in einer
Rampe: **+16,63 dB über dem eingestellten Pegel**, +4,6 dB über dem
unbearbeiteten Eingang (1 kHz: +2,4 dB); −6 dB, 50 Hz: +4,6 dB.

**3. Zusagenlage.** **Schweigt.** Entwurf §44.2 `:2982-2983`
„topology-preserving State-Variable-Filter beziehungsweise robuste Biquads für
dynamische Bänder" ist Bauabsicht ohne Maß; R8 (`SONDE-015.md:1724-1726`)
„kontinuierliche Werte über Rampen; Zipperresiduen < −100 dBFS (§44.5)" bindet
an den definierten Ramp-Test; U44 betrifft große Sprünge. Einordnung „Lücke"
trägt.

**4. Urteil: PRÄZISIERT** (Zahlen bestätigt, Wirkung zu eng) — **LÜCKE**.
**Wirkung:** Anhebungen bis etwa 10 dB über dem eingestellten Pegel stimmen;
bei dynamischen Absenkungen im Bass, dem üblichen Gebrauch, bis etwa 17 dB über
dem eingestellten Pegel und kurz lauter als unbearbeitet; im Bass genügt schon
eine Reglerfahrt von 0,2 s für rund +6 dB. **Heute erreichbar:** ja — der
SVF-Weg läuft für jedes dynamische Bell oder Shelf, auch bei Range 0 oder
Sidechain `none` (`DspProgramm.cpp:307-314`). **Ort:** neue Registerzeile
(Lücke) unter NAK-379; Bau mit DSP-01 in S25p oder als eigene Nacharbeit;
Golden „SVF-Bell Q 24→6, ±12 dB, 50 Hz und 1 kHz, Maß R-311-13".

---

## DSP-03 — Delta-Hören mit festem +12-dB-Ausgleich ohne Schutz

**1. Quelle.** `DspKern.cpp:1436-1448` (`makeup = dbInLinear (kDeltaMakeupDb)`,
`l = (pl − dryL[i]) · makeup`) und `DspProgramm.h:217` (`kDeltaMakeupDb = 12.0`)
stimmen; danach folgen nur Verengung auf float mit Endlichkeitsriegel
(`:1551-1567`) und die Hostbypass-Stufe (`:1569-1591`) — kein Deckel, kein
Limiter. `SONDE-015.md:1506` (R1-R15 sind Technikentscheide des Dirigenten vom
09.09.2026) und `:1773` (R10 Feinheit 1, „ohne ein starkes Delta zu
übersteuern") stimmen. Entwurf `:2701` (DELTA = „Preview − pegelangepasster
gespeicherter Zustand") und `:2730-2736` (±6 dB, gemeinsames Absenken) stimmen.
**„Ruhend" am Code belegt:** `setzeHoermatrix` hat außerhalb der Tests genau
zwei Aufrufer, beide mit `Hoermatrix::processed` (`SondeProcessor.cpp:1338`
nach dem Laden, `:1614` beim Offline-Wechsel); der öffentliche Durchreicher
`SondeProcessor.h:298` hat keinen Produktaufrufer; kein Editor-, Pipe- oder
Vertragsweg (`git grep` über `eq-copilot/plugin` ohne `tests/`, `broker/src`,
`eq-copilot/schemas`). **Bekannt · Ort:** NAK-370 betrifft nur die
A2-Bedingung des Vorher/Nachher-Vergleichs — „neu" stimmt; S29-31 (aktiver
Vergleich) ist der Ort der Verdrahtung.

**2. Formel und Zahl.** `gegenpruefung-eq-1/v1_delta.py` ist getreu (Makeup
10^(12/20); `pl` = ganzer Committed-Pfad samt Output-Trim, `DspKern.cpp:968-1052`).
**Eigene Rechnung** (`val_a2.py`, geschlossen): Bell +12 dB bei f0:
(A² − 1)·3,981 → **+21,49 dB** über Dry (Bericht +21,5); Output-Trim +24 dB bei
flachen Bändern: (15,85 − 1)·3,981 → **+35,43 dB** (Bericht +35). Rosa Rauschen
mit eigenem FFT-Generator (1/f bis 0,1 Hz), Spitze −6 dBFS, Low-Shelf +12 dB
Q 0,707 bei 200 Hz: Delta-Spitze **+11,0 dBFS**, über fünf weitere
Realisierungen +10,7 bis +12,5 dBFS (Bericht +9,1 mit Kellett-Filter). Die Zahl
hängt am Tiefbassanteil des Rauschens; „deutlich über 0 dBFS" steht.

**3. Zusagenlage.** **Trägt:** Entwurf §41.2 `:2730` „Der automatische
Ausgleich ist produktseitig auf ±6 dB begrenzt" und `:2734-2736` „Ein eigener
Limiter wird nicht heimlich eingeschaltet. Reicht der Headroom nicht, werden
beide Varianten gemeinsam sicher abgesenkt und dieser Offset angezeigt." §41.2
gilt ausdrücklich auch zwischen Dry und Processed (`:2731-2732`), und SONDE-015
nennt die +12 dB selbst „festen Lautheitsabgleich" (`:1773`). §30.1 `:1700`
(„lokalem Dry/Processed/Delta-Hörpfad mit festem Lautheitsabgleich") und R10
(`SONDE-015.md:1766-1767`: „Delta = Processed − Dry mit festem
Lautheitsabgleich (§30.1)") nennen keine Zahl — die +12 dB stammen aus der
Feinheit des Manifests. Der Widerspruch R10-Feinheit 1 ↔ Entwurf §41.2 ist
nicht registriert, obwohl CLAUDE.md das verlangt („Widersprüche werden als Zeile
in `docs/offene-punkte.md` geführt, nie kaschiert"). „Defekt, ruhend" trägt:
§41.2 bricht in dem Moment, in dem Delta erreichbar wird.

**4. Urteil: BESTÄTIGT** — **DEFEKT (ruhend)** plus **DOKU** (Begründung in
`SONDE-015.md:1773` rechnerisch falsch). **Wirkung** stimmt: heute keine;
nach der Verdrahtung beim Umschalten nach kräftigem Boost oder Cut ein Sprung
um 10-20 dB und Pegel über 0 dBFS. **Heute erreichbar:** nein. **Ort:**
Nacharbeit vor der Verdrahtung der Hörmatrix in S29-31 (Delta gegen einen
pegelangepassten Bezug wie §41.1, Ausgleich höchstens ±6 dB, gemeinsame
Headroom-Absenkung mit Anzeige); Registerzeile für den Widerspruch; Berichtigung
von `SONDE-015.md:1773`.

---

## DSP-04 — Mono-Bass ohne Phasengleichheit

**1. Quelle.** `DspKern.cpp:998-1009` (m = (L+R)/2, s = (L−R)/2, nur s durch
den Hochpass, s·w, L = m+s, R = m−s), `DspProgramm.cpp:278-279`
(`entwurfLowCut (samplerate, monoBassHz, 1/√2)`), Vertrag
`nakama-parameter-v2.json:86` („Unterhalb dieser Frequenz wird die Seite
entfernt; 0 = aus.") und Entwurf `:2681-2682` („Der aktive Kern darf nur
minimumphasige, latenzfreie M/S-EQ- oder begrenzte Width-Operationen anbieten;
keine heimliche Allpass-/Delay-Korrektur.") stimmen. **Bekannt · Ort:** „neu"
stimmt (keine Registerzeile, keine Karte). S26-28 ist nicht abgenommen
(`docs/PLAN-STAND.md`: P6-P7 0 von 6) — „Karte vor Abnahme S26-28" ist ein
gültiger Ort.

**2. Formel und Zahl.** `gegenpruefung-eq-2/v1_monobass.py` ist getreu (Teil b
fährt die Samplefolge des Kerns). **Von Hand** (analoger Butterworth-Hochpass,
H = −Ω²/(1 − Ω² + j√2Ω); hart links heißt L′ = x(1+H)/2, R′ = x(1−H)/2) und
**digital** (`val_a2.py`, RBJ bei 48 kHz, fc 120 Hz): fc/2 Rest-Seite
−12,30 dB bei 136,7°, R′/L′ **+3,01 dB**; 70 Hz +3,32 dB; 2·fc −8,02 dB,
4·fc −14,79 dB, 10·fc −22,98 dB; LR4 plus Allpass −30,37 / −54,21 / −86,09 dB;
Allpass-Gruppenlaufzeit nahe DC 3,75 ms = 2√2/(2π·120 Hz). Alle Berichtszahlen
exakt.

**3. Zusagenlage.** **Schweigt** zu Phase und Übersprechen: der Vertrag sagt
nur, dass unterhalb der Grenze die Seite entfernt wird (im Sinn eines
12-dB/Okt-Hochpasses erfüllt: −3 dB bei fc, −12,3 dB bei fc/2), über fc nichts.
Den naheliegenden Ausweg sperrt der Entwurf `:2681-2682` (Allpass auf M ist
nicht minimumphasig). Einordnung „Lücke + Produktfrage" trägt.

**4. Urteil: BESTÄTIGT** — **LÜCKE + PRODUKTFRAGE**. Musikerfrage: „Soll
Mono-Bass den Bass unter der Grenze sauber in die Mitte legen, auch wenn die
Mitte im Bass dafür um einige Millisekunden verzögert wird — oder soll er
verzögerungsfrei bleiben und seitlich liegende tiefe Mitten leicht auf die
andere Seite übersprechen lassen?" **Wirkung** stimmt. **Heute erreichbar:**
ja. **Ort:** Karte vor Abnahme S26-28; bis zur Antwort Registerzeile (Lücke).

---

## DSP-05 — Attack-Boden durch das feste 10-ms-Pegelfenster

**1. Quelle.** `DspFilter.h:289` (`kPegelFensterMs = 10.0`), `:363-368`
(symmetrischer Ein-Pol) und `DspKern.cpp:866-867` (Pegel vor Hüllkurve) stimmen.
Durchschnitt ist unbedingt gesetzt, Spitze über den Vertrag unerreichbar
(`DspProgramm.cpp:339`). U45 stimmt: „Attack, Hold und Release bleiben in beiden
Fällen einstellbar wie heute" ist Fragetext des Dirigenten (`:14-15`), das
User-Wort ist nur „beides einzeln auswählbar" (`:20`) und „1 und 1" (`:34`);
„Die Länge des Messfensters für den Durchschnitt und die Bauart der
Spitzenmessung sind Technik" (`:41-42`). **Bekannt · Ort:** M-127
(`NAK-311-verlauf.md:4983`) misst den Mechanismus bei Attack 20 ms (63-%-Punkt
31,7 statt 20 ms); der Boden bei kurzer Attack ist dort nicht beziffert —
„Mechanismus bekannt, Boden neu" stimmt. NAK-331 existiert (Spitze je Band,
S26-28, S28b, S31b).

**2. Formel und Zahl.** `gegenpruefung-eq-1/v4_attack.py` ist getreu
(Bandpass, Stereo-Leistung, Pegel-Pol, Hüllkurve mit Hold-Reihenfolge, dB,
Kennlinie); die 8-Sample-Steuerrate fehlt und ist im Skript benannt.
**Geschlossen:** zwei Pole in Reihe mit Attack-Pol ≈ 0 — die Leistung
1 − e^(−t/10 ms) erreicht −1 dB bei 10·ln(1/(1 − 10^(−0,1))) = **15,81 ms**.
**Eigene Simulation** (`val_a2.py`), Bell 1 kHz Q 1, 3 dB über der Schwelle:
Attack 0,1 / 1 / 10 ms → **16,38 / 17,33 / 30,04 ms** (mit 8er-Takt
16,50 / 17,33 / 30,17), ohne Fenster 0,79 ms; 18 dB über der Schwelle 2,79 ms.
Alle Berichtszahlen exakt.

**3. Zusagenlage.** **Schweigt:** U45 legt das Fenster in die Technik;
`DspProgramm.h:234-235` „Eine Zusage zur Reaktionszeit gibt es nicht"; der
Vertrag `attack_ms` 0,1-500 ms (`nakama-parameter-v2.json:246-255`) sagt nichts
über die Gesamtreaktion. „Produktinformation + Lücke bis NAK-331" trägt.

**4. Urteil: BESTÄTIGT** — **LÜCKE** (Information, geplant in NAK-331).
**Wirkung** stimmt in der Lesart „reagiert in etwa 15 ms"; als
Attack-Einstellung ohne Fenster entspricht das rund 10 ms (16,29 ms bis −1 dB).
**Heute erreichbar:** ja. **Ort:** Zahl in NAK-331 nachtragen (Boden knapp über
der Schwelle ≈ 16 ms; der Spitze-Golden verlangt eine Reaktion in der Größe
der Attack); kein neues Ticket.

---

## DSP-06 — Detektor an die Band-Güte gekoppelt

**1. Quelle.** `DspProgramm.cpp:347-349` (`entwurfBandpass (samplerate,
b.freqHzWirksam, b.q)`) und `SONDE-015.md:1711` (R7 Feinheit 3: „RBJ-Bandpass
mit konstanter Spitzenverstärkung auf `freq_hz` und `q` desselben Slots")
stimmen. **Bekannt · Ort:** „neu" stimmt. Marktangabe geprüft: FabFilter
Pro-Q 4, Hilfe „Dynamic EQ" — Triggering „Band" als Standard, „Free" mit
eigenem Low- und High-Cut für den Detektor; stimmt für Pro-Q 4.

**2. Formel und Zahl.** `gegenpruefung-eq-2/v2_detektor.py` mit
`v2lib.detektor_kette` ist getreu (Bandpass `DspFilter.h:224-238`,
Pegelstufe, Hüllkurve). **Eigene Rechnung** (`val_a2.py`): 100 Hz Q 24, e_db bis
−1 dB nach **180,2 / 190,7 ms** bei Attack 0,1 / 10 ms. Rauschen: exakter
Leistungsgewinn (1 − a2)/2 des 0-dB-Bandpasses bei 1 kHz: Q 0,707 −10,73 dB,
Q 24 −25,67 dB, Differenz **14,94 dB** (100 Hz 15,27 dB); rosa Rauschen
13,8 dB; durch die ganze Kette (Attack 10, Release 100) 13,76 dB (Bericht
14,04) — „um 14 dB" stimmt (14-15 dB). Low-Shelf 200 Hz Q 0,707: Detektor bei
50 und 800 Hz je **−9,05 dB**, Shelf-Wirkung −5,97 / −0,03 dB. **Ergänzend**
(`val_a3.py`): die volle Wirkung (g_dyn innerhalb 1 dB) kommt bei 100 Hz Q 24
bis 12 dB über der Schwelle nach 180-190 ms, bei 18 dB darüber nach 55-64 ms,
bei 30 dB darüber nach 16-22 ms.

**3. Zusagenlage.** **Schweigt:** R7 Feinheit 3 ist selbst die Spezifikation;
R7 wörtlich (`SONDE-015.md:1692-1693`: „interner Detektor = das bandgefilterte
Eingangssignal des Bandes vor dem Band") lässt offen, was „bandgefiltert" bei
Shelves heißt; Entwurf und Blueprint (`design/docs/funktions-und-bedien-blueprint.md:366-367`,
`:712-720`) kennen keinen Detektorbereich. Doku-Nebenpunkt: `DspFilter.h:220-223`
begründet die 0-dB-Spitze mit „ein Detektor mit Q-abhängiger Spitze machte die
Kennlinie Q-abhängig" — für breitbandiges Material bleibt sie um 14-15 dB
Q-abhängig. „Spezlücke + Produktfrage" trägt.

**4. Urteil: BESTÄTIGT** — **LÜCKE (Spezifikation) + PRODUKTFRAGE**, dazu
DOKU am Kommentar `DspFilter.h:220-223`. Musikerfrage: „Soll ein dynamisches
Band immer auf genau den Bereich hören, den es bearbeitet — schmale Bänder
reagieren dann träge, ein Bass-Shelf hört auf seine Eckfrequenz statt auf den
Tiefbass —, oder soll man den Hörbereich wie bei Pro-Q frei einstellen können?"
**Wirkung** stimmt mit Pegelvorbehalt (die 0,2 s gelten bis etwa 12 dB über der
Schwelle). **Heute erreichbar:** ja. **Ort:** Karte vor Abnahme S26-28.

---

## DSP-07 — Güte-Semantik und Flanken

**1. Quelle.** Vertrag `:166-177` (q 0,15-24, Default 1,0, ein Eintrag für alle
sechs Typen `:136-146`) und `DspFilter.h:133-218` (Shelves mit
alpha = sin(w0)/(2q), Cuts ein Biquad) stimmen. **Bekannt · Ort:** O-04/F10
existieren im Tiefenaudit 3 (`docs/audits/2026-09-15-tiefenaudit/roh/phase-15-paket2-dsp-numerik.md:303`,
`roh/phase-16-d-dsp.md:26`, `BEFUNDE.md:677` „Produktentscheid (Q oder Slope),
danach ein B6-Golden für einen Shelf mit Q 24", `:757`); keine Registerzeile,
keine Karte in `docs/plan/fragen.json` — „teilweise, nur Shelves, ohne
Registerzeile" stimmt.

**2. Formel und Zahl.** `gegenpruefung-eq-2/v3_guete.py` ist getreu.
**Geschlossen und digital** (`val_a2.py`): Low-Cut Q 1, Spitze
Q/√(1 − 1/(4Q²)) = 1,1547 → **+1,25 dB** bei √2·fc; Q 24 **+27,61 dB**; Q 0,15
an der Ecke 20·log10(0,15) = **−16,48 dB**; Low-Shelf ±12 dB mit Q 1 / 2 / 24:
**±0,78 / ±4,58 / ±25,10 dB**; Flanke 12,04 dB/Okt. Alle Berichtszahlen exakt.
Marktangabe geprüft: Pro-Q 4 bietet Flanken bis 96 dB/Okt, für Cuts zusätzlich
Brickwall.

**3. Zusagenlage.** **Schweigt** als Einzelzusage. Die Richtung setzt der User
am 23.08.2026: „es ist ein vollwertiger eq für den master wie andere eq auf dem
markt" (`design/abnahmen/2026-08-23-gen-eq-zentrale.md:36-37`) und „smooth
operator pro , oder pro-q sind die 2 besten. dafür kannst du alles studieren wie
sie das handhaben und das beste kopieren" (`2026-08-23-interview-struktur.md:45-46`).
Flankensteilheit und Shelf-Semantik sind nicht entschieden. „Produktfrage"
trägt.

**4. Urteil: BESTÄTIGT** — **PRODUKTFRAGE**. Musikerfrage: „Sollen Low- und
High-Cut wie bei Pro-Q eine wählbare Steilheit bekommen und Shelves eine
Steilheit statt einer Resonanz — oder bleibt die Güte bei allen Filtertypen die
Resonanz wie heute, mit Buckeln bei hohen Werten?" **Wirkung** stimmt.
**Heute erreichbar:** ja. **Ort:** Karte vor Abnahme S26-28, zusammen mit dem
Tiefenaudit-3-Prüffeld O-04/F10.

---

## DSP-08 — Mix mit phasendrehenden Filtern

**1. Quelle.** `DspKern.cpp:1027-1049` (Auto-Gain → Mix → Output-Trim,
`l = mix·l + (1 − mix)·dL[i]`) stimmt. `SONDE-015.md:1590-1594` stimmt
inhaltlich; das Anführungszitat „Mix 0,5 = halbe Kurve, Dry und Wet
hochkorreliert" ist eine Verdichtung — wörtlich stehen `:1592` „Steht Mix auf
0,5, wirkt die Kurve zur Hälfte" und `:1594` „Bei hochkorrelierten Zweigen — und
das sind Dry und Wet eines EQ — …". Vertrag `:109` („Linear ueberblendet, VOR
Auto-Gain und Output-Trim") widerspricht `DspKern.cpp:1027-1028` und R3
Feinheit 1 (`SONDE-015.md:1590`: „… → Auto-Gain → Mix → Output-Trim"); der Text
ist seit Anlage falsch (`git blame`: `94a8a336`, 10.09.2026). **Bekannt · Ort:**
„neu" stimmt.

**2. Formel und Zahl.** `gegenpruefung-eq-2/v4_mix.py` ist getreu
(H_aus = mix·ag·H + (1 − mix)). **Eigene Rechnung** (`val_a2.py`): Low-Cut
100 Hz Q 4, Mix 0,5: **−15,80 dB bei 69,2 Hz** (Wet dort −1,17 dB, Phase
+161,6°); Mix 0,75: −27,77 dB bei 49,7 Hz; Q 0,707, Mix 0,5: −7,59 dB bei
55,0 Hz. Zusatz: auch am Bell ist Mix 0,5 in dB keine halbe Kurve — +12 dB Q 1
ergibt +7,93 dB statt +6 dB; „zur Hälfte" gilt nur für die lineare Abweichung.

**3. Zusagenlage.** **Schweigt** zur Summenantwort: R3 wörtlich
(`SONDE-015.md:1581-1586`) legt Bereich, Rampe und die Lage von Dry und Wet
fest; die Feinheiten 2 und 4 sind Begründungen. Das User-Wort vom 23.08.2026
(`2026-08-23-interview-struktur.md:73-74`: „und natürlich mix also wieviel sie
auf den mix anwendet des eq") beschreibt Mix als Anteil des EQ; ein Loch, das
weder Dry noch Wet hat, passt nicht zu dieser Lesart — das ist Produktfrage,
keine gebrochene Zusage. „Planfehler in der Begründung + Produktfrage" trägt;
der Vertragstext `:109` ist DOKU.

**4. Urteil: BESTÄTIGT** — **PRODUKTFRAGE + DOKU**. Musikerfrage: „Soll der
Mix-Regler trockenes und bearbeitetes Signal parallel mischen wie ein
Dry/Wet-Regler — mit Löchern an steilen Cut-Kanten —, oder soll er die Stärke
der EQ-Kurve verringern, sodass Mix 50 % wie die halbe Kurve klingt?"
**Wirkung** stimmt. **Heute erreichbar:** ja. **Ort:** Karte vor Abnahme
S26-28; Vertragstext `:109` und `SONDE-015.md:1592`, `:1594` berichtigen.

---

## Übersicht

| Befund | Urteil | Klassenvorschlag | tragender Satz oder „schweigt" | heute erreichbar | Planort |
|---|---|---|---|---|---|
| DSP-01 | PRÄZISIERT | LÜCKE (mittel) + DOKU | schweigt: R-311-13-Maß nur über dem Kriterium und an einen Prüfling gebunden; U44 „Kleine Änderungen … klingen wie bisher" | ja | NAK-336 (d) erweitern; Bau S25p (NAK-340, Etappe 0 „Kriteriensprünge"); „bis 8,0" in §57.3, NAK-336 (d), NAK-337 berichtigen |
| DSP-02 | PRÄZISIERT | LÜCKE | schweigt: Entwurf :2982-2983 „robuste … für dynamische Bänder" ohne Maß | ja | neue Registerzeile; S25p oder eigene Nacharbeit mit SVF-Golden |
| DSP-03 | BESTÄTIGT | DEFEKT (ruhend) + DOKU | Entwurf :2730 „auf ±6 dB begrenzt"; :2735-2736 „Reicht der Headroom nicht, werden beide Varianten gemeinsam sicher abgesenkt" | nein | Nacharbeit vor S29-31; Registerzeile R10-Feinheit 1 gegen §41.2; SONDE-015:1773 berichtigen |
| DSP-04 | BESTÄTIGT | LÜCKE + PRODUKTFRAGE | schweigt: Vertrag :86 nur „unterhalb … entfernt"; Entwurf :2681-2682 sperrt den Allpass | ja | Karte vor Abnahme S26-28 |
| DSP-05 | BESTÄTIGT | LÜCKE (Information) | schweigt: U45 „sind Technik"; DspProgramm.h:234 „Eine Zusage zur Reaktionszeit gibt es nicht" | ja | Zahl in NAK-331 nachtragen |
| DSP-06 | BESTÄTIGT | LÜCKE (Spez.) + PRODUKTFRAGE + DOKU | schweigt: R7 Feinheit 3 ist die Spezifikation | ja | Karte vor Abnahme S26-28; Kommentar DspFilter.h:220-223 |
| DSP-07 | BESTÄTIGT | PRODUKTFRAGE | schweigt; Richtung User 23.08. „vollwertiger eq … wie andere eq auf dem markt" | ja | Karte vor Abnahme S26-28 mit Tiefenaudit-3 O-04/F10 |
| DSP-08 | BESTÄTIGT | PRODUKTFRAGE + DOKU | schweigt zur Summe (R3); Vertrag :109 widerspricht Kette und Code | ja | Karte vor Abnahme S26-28; Vertrag :109, SONDE-015 R3 Feinheit 2 und 4 |

## Querbezüge

**Rampen- und Goldens-Komplex (DSP-01, DSP-02, dazu DSP-10 und DSP-11 aus
Teil B).** Der einzige Rampengolden (`DspGoldenTestMain.cpp:3985-4050`)
vergleicht gegen dieselbe Koeffizientenrampe und fährt nur kleine Schritte an
einem statischen Bell; er sieht weder DSP-01 noch DSP-02. Das Maß aus R-311-13
ist an einem Prüfling gemessen (§57.3); dieselbe Verallgemeinerung trägt
NAK-337 (Sprungsumme über verdrängte Publikationen), dessen Risiko damit
unterschätzt ist, weil schon ein Einzelschritt am Kriterium im Bass mit hoher
Güte überschwingt. Die Referenz „Sweep-Physik" in DSP-01 unterliegt selbst dem
Mechanismus aus DSP-02. Die feste Rampenlänge 256 Samples (DSP-11) verkürzt die
Rampe bei 96/192 kHz; das Raster `r5b_ausgabe.txt` zeigt bei 192 kHz dieselben
Größen (SVF +9,9 dB) — die Spitze liegt schon bei 48 kHz nahe dem Grenzfall
eines sofortigen Sprungs. Ein Maß, das auch der Crossfade hält, muss neu gefasst
werden (Kaltstart-Einschwingen hochgütiger Shelves). **Dynamik-Komplex (DSP-02,
DSP-05, DSP-06, dazu DSP-12 aus Teil B, NAK-331, NAK-336 (e)).** Pegelfenster,
Detektor-Güte, Kennlinie und SVF-Ausgang bilden die eine Kette „wie ein
dynamisches Band hört und reagiert"; eine gemeinsame Karte kann DSP-05, DSP-06
und DSP-12 bündeln. **Hör- und Vergleichskomplex (DSP-03, dazu DSP-11 — Blende
256 Samples statt 5-20 ms —, DSP-18 und DSP-19 aus Teil C, NAK-370, §4 des
Berichts zur Delta-Ausrichtung).** Alles davon gehört vor die Verdrahtung von
Hörmatrix und Vorher/Nachher in S29-31. **Phasenkomplex (DSP-04, DSP-08).**
Dieselbe Physik — die Phasendrehung des minimumphasigen Hochpasses 2. Ordnung
nahe der Ecke (90° bei fc, 137° bei fc/2) — erzeugt Mono-Bass-Übersprechen und
Mix-Löcher; beim Mono-Bass stößt eine Allpass-Kompensation an „nur
minimumphasige, latenzfreie M/S-EQ" (Entwurf `:2681-2682`), beim Mix ist die
Alternative eine Kurvenskalierung statt Dry/Wet (Produktfrage DSP-08). DSP-16
(Teil B) liegt an derselben M/S-Stufe
`DspKern.cpp:998-999`. **Filterform-Komplex (DSP-07, dazu DSP-09 mit NAK-332
und DSP-15 aus Teil B).** Shelf-Semantik, Höhenverformung und das
Auto-Gain-Gitter bei hoher Güte hängen an denselben RBJ-Entwürfen; steilere Cuts
würden die Mix-Löcher aus DSP-08 vertiefen. Mit dem Statistik- und
Einheitenkomplex DSP-17, DSP-19, DSP-22 (Teil C) hat Teil A keinen direkten
Bezug außer über DSP-03 und den Vorher/Nachher-Vergleich.

## Quellen der Websuche

- FabFilter Pro-Q 4 Help, Dynamic EQ: <https://www.fabfilter.com/help/pro-q/using/dynamic-eq>
- FabFilter Pro-Q 4 Help, Band controls: <https://www.fabfilter.com/help/pro-q/using/bandcontrols>
- J. Laroche, „On the Stability of Time-Varying Recursive Filters", JAES 55(6):460-471, 2007: <http://www.aes.org/e-lib/browse.cfm?elib=14168>

FERTIG Validierung DSP-Fachprüfung Teil A, 6 bestätigt, 2 präzisiert, 0 widerlegt.
