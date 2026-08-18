# Hör-Markierung („Einfärben") — Konzept v2 (nach Technik-Begehung)

**Datum:** v1 2026-08-16 · **v2 gleicher Tag nach Technik-Begehung** (User-Auftrag:
„das Bestmögliche unter der Haube").
**Idee (User):** Gefundene Problembereiche gezielt **akustisch einfärben**, damit man
beim Beheben hört, **wo** die Fehler sind.

---

## ✅ GEBAUT 2026-08-16 („in einem Rutsch", User-Auftrag)

Umsetzung: `plugin/src/HoerMarkierung.h` (DSP + Auftrag + Ring-Publikation) ·
`PluginProcessor` (Lebenszeichen §4, Färbung nach dem Abgriff §5, Heartbeat-Feld) ·
`PluginEditor` (Karten-Chips [Solo]/[Puls], Latch-Regeln N2, Feld-Tönung
puls-phasensynchron, Not-Aus in der Statuszeile) · Schema `hoermarkierung` ·
Tests `plugin/tests/MarkierungTestMain.cpp`.
**Beweise:** MARKIERUNGSTEST **30/30** (T2–T10 inkl. Render-Beweis: Freilauf mit
gesetztem Latch ⇒ JEDES Sample bitgleich; Abgriff-Beweis: LTAS mit/ohne
Markierung gleich) · NULLTEST 10/10 · GOLDEN 239/239 · pluginval 8: s. CLAUDE.md.
**Bewusste Interims-Abweichungen im alten Editor** (NAKAMA §7.6 ersetzt sie):
Chips sind **Toggle-only** (Halten = momentan kommt mit der Marker-Glyphe im
Spectral Field); Aktivzustand = Statuszeilen-Knopf „Markierung aus" + Tönung
statt Marker-Ring; Karten-Chip-Zustand kann bei driftendem Befund-Schwerpunkt
unsynchron wirken — Tönung + Statuszeile tragen die Wahrheit (Engage-Parameter
sind eingefroren). **Broker-Seite der Messpause** (§5): Feld wird gesendet;
Auswertung im Broker folgt NACH dem laufenden Harness-Lauf (one-writer,
`src-tauri` ist dessen Baustelle).

---

## 0. Begehungs-Protokoll v1 → v2 (was die Begehung geändert hat)

| # | v1 | v2 | Grund |
|---|---|---|---|
| B1 | Puls über Linkwitz-Riley-4-Bandsplit + AM | **Parallel modulierter RBJ-Peaking-Bell** | Bei 0 dB Gain gilt bei RBJ-Peaking exakt b₀=a₀, b₁=a₁, b₂=a₂ ⇒ Übertragung = 1: die Ruhephase ist **mathematische Identität**, keine Allpass-Phasenrotation. Weniger Filter, stärkere Ehrlichkeit. |
| B2 | Puls = Dauersinus 1,6 Hz | **Tastverhältnis 40/60**: 250 ms Kosinus-Schwellen, dann 375 ms echte Ruhe auf 0 dB | Das Ohr braucht die Referenzphase — das ist der manuelle A/B-Handgriff der Mischpraxis, automatisiert. Frei laufend, **bewusst nicht tempo-synchron**: ein taktfremder Puls fällt auf, statt sich im Groove zu verstecken. |
| B3 | Puls fest +4,5 dB | **Headroom-bewusst 2,0–4,5 dB** aus eigener Messung (`MessSnapshot.truePeakDb`) | Wir MESSEN True Peak bereits — verfügbarer Headroom = −0,3 dBTP − truePeakDb; Tiefe = clamp(min(4,5, verfügbar), 2,0, 4,5). Unter 2 dB Rest sagt die Karte ehrlich: „Puls gedeckelt — dein Master ist heiß." |
| B4 | Solo einheitlich HP+LP, min 1/3 Okt | **Zweigleisig:** Zonen = HP+LP Butterworth 4. O.; Resonanzen = **2× kaskadierter RBJ-Bandpass** mit Kaskaden-Korrektur | Resonanz-Befunde können 1/12 Oktave schmal sein (`Diagnose.cpp: max(breiteOktaven, 1/12)`) ⇒ Q≈17, klingelt als HP+LP-Paar zudem mit Passband-Einbruch. Echter Bandpass mit Q aus `breiteOktaven` (aufgeweitet auf ≥ 1/3 Okt) ist das richtige Werkzeug. |
| B5 | Equal-Power-Crossfade 30 ms | **Linearer Crossfade** (Solo 80 ms, Puls-Engage 30 ms) | **v1-Fehler:** Equal-Power gilt für unkorrelierte Signale; markiert/neutral sind hochkorreliert ⇒ +3 dB Buckel in der Fade-Mitte. Korrelierte Signale ⇒ konstante Spannung (linear). |
| B6 | Renderschutz: an, bis Freilauf erkannt (Reaktion ≤ 1,5 s) | **Invertiert: neutral, bis Echtzeit BEWIESEN** („Echtzeit-Lebenszeichen") | v1 hätte die erste ~1 s eines Exports gefärbt. v2: nach jedem Transportstart/jeder Verarbeitungslücke bleibt die Markierung neutral, bis das Verhältnis Audiozeit/Echtzeit ~1 s lang ≈ 1 ist. Ein Offline-Export besteht den Beweis nie ⇒ **null gefärbte Render-Samples**, host-unabhängig. |
| B7 | Koeffizienten rechnet der Audiothread bei Engage | **Editor rechnet, Audiothread kopiert nur** | Engage-Tabelle (Gain-Treppe → fertige Koeffizientensätze) entsteht auf dem Message-Thread; im Hot-Path keine einzige Transzendente. |
| B8 | — | **Filterzustands-Hygiene:** Disengage setzt Zustände zurück; Ruhephasen-Rest < −120 dBFS ist Testschwelle | b=a heißt Übertragung 1, aber FP-Zustände tragen Historie — ehrliche ε-Schwelle statt Bit-Behauptung. |
| B9 | — | **Anzeige-Semantik fixiert:** Graph zeigt weiter den ECHTEN Mix | Abgriff liegt vor der Färbung; die Kurve ist Messinstrument, nicht Abhör-Monitor. Was hörbar ist, zeigt allein die Feld-Tönung + der Marker. Ohne diese Festlegung würde die UI lügen. |

Geblieben aus v1: zwei Modi (Solo/Puls), Halten/Latch, eingefrorene Parameter,
mittenLoch nur Solo, IIR-only/0 Latenz, nie persistiert, Broker-Messpause,
Bauplan-Deltas als Übergabeliste (Codex baut gerade die Hauptansicht).

---

## 1. Prüfergebnis (unverändert)

**UMSETZBAR — und produktlogisch ein Volltreffer.** Die Karten sprechen seit M3
Tu/Warum/**Hören**; die Hör-Markierung macht aus dem Hören-Feld eine Taste. Sitzt
der Sammler hinter dem User-EQ (Normalfall Master/Bus, letzter Insert), wird die
Markierung beim Ziehen des Reglers leiser — **Markierung = Erfolgskontrolle**.
Präzedenz fürs Prinzip „Abhören ist keine Bearbeitung": ADPTR Metric AB (reines
Referenz-Plugin mit Band-Solo-Filterbank). Der pulsierende, UI-synchrone
Problemband-Marker ist kein Marktstandard — eigene Kombination, ehrlich so geführt.

---

## 2. Die zwei Modi (V1)

| Modus | Was das Ohr hört | Wofür |
|---|---|---|
| **Solo** | Nur der Problembereich spielt; Rest verschwindet. | Charakter isoliert lernen: „DAS ist das Dröhnen." |
| **Puls** | Voller Mix; der Problembereich **schwillt im Takt an** (2,0–4,5 dB, ~1,6 Hz, 40/60-Tastverhältnis). | Die Stelle **im Zusammenhang** orten und beim Beheben verfolgen — das eigentliche Einfärben. |

Bedienung: **Halten = momentan**, **Klick = Latch** (Maus darf zum eigenen EQ).
Parameter beim Engage **eingefroren** — kein Nachwandern, während der User hört.

**Zuordnung je Befundklasse** (Default zuerst): `resonanz` → **Puls** · Solo ·
`mulm`/`haerte`/`hoehenHype` → **Solo** · Puls · `mittenLoch` → **nur Solo**
(ein Loch kann nicht pulsieren; das Solo zeigt ehrlich, wie leer 500–2000 Hz ist).
Zonen-Solo unter ~150 Hz: `hoeren`-Text empfiehlt Kopfhörer (kleine Boxen tragen
die Zone kaum).

Immer genau **ein** Befund markiert; Kartenwechsel = linearer Crossfade auf das
neue Band.

---

## 3. DSP-Spezifikation v2

Alles IIR, alles vorallokiert, **gemeldete Latenz bleibt 0 — für immer** (nie
FIR/Linearphase: Latenzwechsel beim Einschalten würde FLs PDC umwerfen).
Alle Markierungsfilter: **TDF2-Biquads mit double-Zuständen und
double-Koeffizienten** (Tieffrequenz-Robustheit; Mulm-Kante 120 Hz bei 44,1 kHz).
Mono-Layout (Bus erlaubt mono): ein Kanalzug, gleiche Pfade.

### 3.1 Puls — parallel modulierter Peaking-Bell

- **Ein RBJ-Peaking-Biquad pro Kanal** bei `fSchwerpunkt`; Bandbreite in Oktaven
  direkt aus dem Befund (`fBis/fVon` ⇒ `breiteOktaven`; RBJ-Cookbook-Parametrik
  mit Bandbreite-in-Oktaven, nicht Q — wir HABEN die Oktavbreite). Zonen nutzen
  die Zonenbreite (Mulm 1,32 Okt · Mitte 2 Okt · Härte 1 Okt · Höhen 0,81 Okt) um
  den geometrischen Mittelpunkt.
- **Identität in der Ruhephase:** RBJ-Peaking bei A=1 ⇒ b₀=1+α=a₀, b₁=−2cos ω₀=a₁,
  b₂=1−α=a₂ ⇒ H(z)=1 exakt. (Deshalb RBJ-Form statt SVF/LR4 — die Ruhephase des
  Pulses ist konstruktiv neutral, Rest-ε nur aus FP-Zustandshistorie, s. Test T9.)
- **Hüllkurve:** raised-cosine 0 → Tiefe → 0 über 250 ms, dann 375 ms exakt 0 dB
  (Periode 625 ms ≈ 1,6 Hz, Tastverhältnis 40/60). Frei laufend, nicht
  tempo-synchron (Absicht, s. B2).
- **Tiefe headroom-bewusst (B3):** Editor liest beim Engage `truePeakDb` aus dem
  Snapshot: `verfuegbar = −0,3 − truePeakDb`; `tiefe = clamp(min(4,5, verfuegbar),
  2,0, 4,5)` dB. Unter 2 dB Rest: Tiefe 2,0 + Kartennotiz („Puls gedeckelt — dein
  Master ist heiß; am Wandler kann es kurz übersteuern"). FL rechnet intern float —
  intern clippt nichts.
- **Boost, nie Cut:** eine Markierung darf das Problem nie verdecken.
- **Hot-Path ohne Transzendente (B7):** Der Editor rechnet beim Engage eine
  Tabelle der Hüllkurve als **fertige Koeffizientensätze** (eine Stufe je
  32 Samples ≈ 0,7 ms; Schrittweite unhörbar bei 1,6-Hz-Modulation — dieselbe
  Klasse Gain-Modulation fahren dynamische EQs mit Millisekunden-Attacks). Der
  Audiothread schaltet nur Tabellenzeilen um — kein Interpolieren von
  Koeffizienten (interpolierte Biquad-Koeffizienten können die Pol-Trajektorie
  verlassen; Tabellenzeilen sind alle konstruktiv stabil), kein `pow`, kein `sin`.

### 3.2 Solo — zweigleisig (B4)

- **Zonen** (`mulm`, `mittenLoch`, `haerte`, `hoehenHype`): Butterworth-HP
  4. Ordnung @ `fVon` + Butterworth-LP 4. Ordnung @ `fBis` (je 2 Biquads/Kanal).
  Kappe `fBis ≤ 0,95·Nyquist` (LTAS-Lehre M1; betrifft nur exotische Raten —
  Höhen-Zone endet bei 14 kHz).
- **Resonanzen:** **2 kaskadierte RBJ-Bandpässe** (constant-skirt) bei
  `fSchwerpunkt`. Zielbreite = `max(breiteOktaven, 1/3)` Oktave (Klingel-Grenze);
  **Kaskaden-Korrektur:** zwei identische 2.-Ordnung-BPs verengen die −3-dB-Breite
  um den Faktor √(√2−1) ≈ 0,6436 ⇒ Stufenbreite = Zielbreite / 0,6436.
- Solo senkt nur ab, nie Makeup-Gain — kein Überraschungspegel möglich.

### 3.3 Übergänge

- **Linearer Crossfade** (B5 — korrelierte Signale): Puls-Engage/-Disengage 30 ms,
  Solo 80 ms (der Pegelsprung ins Solo ist groß; 80 ms nimmt den Schreck).
  Moduswechsel/Kartenwechsel: direkter linearer Crossfade markiert→markiert.
- **Disengage setzt Filterzustände zurück** (B8) — kein Altenergie-Plopp beim
  Re-Engage; der Crossfade beginnt immer aus sauberem Zustand.
- NaN-Riegel (CS1-Muster): Befundfelder vor der Tabellenrechnung auf
  endlich/positiv/geordnet prüfen; sonst kein Engage.
- Stereo: beide Kanäle identisch (Kanalenergie-Lehre; Antiphase-Chöre bleiben
  hörbar). `ScopedNoDenormals` deckt die neuen Filter mit ab.
- CPU aktiv: Puls 1 Biquad/Kanal · Solo ≤ 4 Biquads/Kanal + eine Fade-Rampe.
  Inaktiv: exakt der heutige transparente Pfad.

---

## 4. Renderschutz v2 — „neutral, bis Echtzeit bewiesen" (B6)

Host-Flags sind nachweislich unzuverlässig (JUCE-Forumslage: Ableton setzt
kOffline für VST3 nicht, Reaper nie, Cubase zu spät; FLs Verhalten ist nirgends
belastbar dokumentiert). Deshalb trägt der Schutz sich selbst:

- **Grundzustand jeder Wiedergabe: neutral.** Nach jedem Transportstart
  (`transportSpielt` false→true — Atomic existiert bereits) und nach jeder
  Verarbeitungslücke > 250 ms (Smart-Disable-Resume, Bypass-Rückkehr) beginnt das
  **Echtzeit-Lebenszeichen**: zwei aufeinanderfolgende 500-ms-Fenster müssen
  `verarbeitete Audiozeit ÷ Echtzeit` ∈ [0,5 … 1,2] zeigen (steady_clock, ein
  Zeitstempel pro Block — QPC ist realtime-sicher). Erst dann blendet eine
  aktive Markierung ein; der Markerring zeigt die Wartephase gedimmt.
- **Ein Offline-Export besteht den Beweis nie** (Verhältnis ≫ 1,2) ⇒ **null
  gefärbte Samples im Render**, unabhängig vom Host-Flag. Realtime-Wiedergabe
  kann dauerhaft nicht über 1 liegen (die Ausgabe liefe der Soundkarte davon) —
  das Kriterium ist definitorisch, kein Tuning.
- **Laufende Wiedergabe:** Loop-/Seek-Sprünge (FL springt ständig) ändern das
  Verhältnis nicht und lösen KEINE Neuprüfung aus — nur Start und Lücke tun das.
  Kippt das Verhältnis mitten in der Wiedergabe über 1,5 (zwei Fenster), fällt
  die Markierung sofort neutral und der Latch löst sich.
- **Transport steht ⇒ zwangsneutral** (es spielt ohnehin nichts); Latch bleibt
  gesetzt und wartet auf das nächste Lebenszeichen. Hosts ohne Transportinfo
  (`hatTransport == false`, z. B. Test-Harness): nur das Verhältnis-Netz gilt.
- **Sekundärnetz:** `isNonRealtime() == true` ⇒ sofort neutral.
- **Latch endet hart bei:** Editor-Fenster schließt · Karte/Marker abgewählt ·
  Samplerate-Wechsel · 10-min-Totmann ohne UI-Interaktion (Fade + Kartennotiz).
- **Nie persistiert:** `getStateInformation` schreibt keinen Markierungszustand;
  Reload = neutral.
- **Ehrliche Grenze:** Ein **Echtzeit**-Mitschnitt (Edison auf dem Master) ist
  prinzipiell nicht detektierbar — Gegenmaßnahme sind sichtbarer Aktivzustand,
  Kartenabwahl-Regel und Totmann. Steht so in der FL-TESTANLEITUNG.

---

## 5. Mess- und Anzeige-Integrität

- **Abgriff vor der Färbung:** Der FIFO-Push in `processBlock` liest künftig den
  Eingang VOR der Markierungsstufe (heute liegt er am Blockende; Reihenfolge
  wird: FIFO-Push → Markierung in-place). Diese Instanz misst also weiter den
  echten Mix — Beweis Test T4.
- **Anzeige lügt nicht (B9):** Der Graph zeigt weiterhin die Messung (= echter
  Mix), auch während Solo nur ein Band hörbar ist. Die Kurve ist Instrument,
  nicht Monitor. Was gerade hörbar ist, zeigen Feld-Tönung fVon–fBis und der
  Marker. `rmsL/rmsR` bleiben Eingangsgrößen.
- **Nachgelagerte Sensoren hören gefärbtes Signal:** `MessKompakt` bekommt das
  additive Feld `hoermarkierung: bool` (Gueltig-Flag-Muster der Struktur, v1-Broker
  bleibt kompatibel). Der Broker markiert den Messstand („Hör-Markierung aktiv an
  ‹Label›") und pausiert die Aggregat-Aufnahme der ANDEREN Sensoren im Zeitfenster
  — man kalibriert nicht, während die Lupe im Strahlengang steht.

---

## 6. UI im Spectral Field (Anschluss an den NAKAMA-Bauplan)

Kein neues Statuspill, kein Textetikett im Feld — Aktivzustand im vorhandenen
Symbolvokabular:

- **§7.5-Erweiterung:** Ausgewählter Marker trägt eine **Ohr-Glyphe** im Kreis.
  Halten = momentan, Klick = Latch. Bei **Puls** pulsiert der Markerring
  **phasengleich** mit dem Hörbaren (Editor-Timer liest ein Hüllkurven-Atomic;
  20-Hz-Leichtpfad reicht für 1,6 Hz locker). **Wartephase** (Echtzeit-Beweis
  läuft): Ring gedimmt/hohl. Bei **Solo**: Ohr-Glyphe ruhig gefüllt.
- **§8.2-Erweiterung:** Geöffnete Karte bekommt eine Hören-Zeile mit Chips
  **[Solo] [Puls]** (bei `mittenLoch` nur [Solo]); kein Fließtext; Tooltips nur
  über den globalen Schalter.
- **§10.4 Reduzierte Bewegung:** Ring pulsiert nicht — die Ohr-Glyphe wechselt
  zwischen zwei statischen Stufen.
- Feld-Tönung fVon–fBis während aktiver Markierung: Tonwert, nicht Farbe als
  Träger (§2.3.5 bleibt gewahrt).
- `hoeren`-Kartentext verweist auf die Taste, z. B.: *„Drück Puls — das Dröhnen
  schwillt im Takt an. Zieh deinen Regler, bis das Schwellen nichts mehr zu
  fassen hat."*

---

## 7. Threading-Vertrag

- Editor → Audio: POD-Wunsch (Modus, Koeffiziententabelle, Fade-Zeiten,
  Engage-Nonce) über Seq-Lock/Doppelpuffer — exakt das bestehende Wunsch-Muster
  (`messResetWunsch`). Audiothread übernimmt am Blockanfang bei Nonce-Wechsel.
- Audio → Editor: Hüllkurven-Phase + Lebenszeichen-Status als Atomics (relaxed).
- Kein Heap, keine Sperre, keine Transzendente im process-Pfad; Tabellen und
  Filterbänke liegen fest im Prozessor. Hüllkurve auf **64 Gain-Stufen**
  quantisiert (≤ 0,08 dB Stufung — weit unter der Hörschwelle für
  Bandpegel-Sprünge); Tabelle 64 Zeilen × 5 double ≈ 2,6 KB. Der 32-Sample-Takt
  wählt pro Chunk die Zeile zur vorberechneten Hüllkurvenposition.

---

## 8. Testplan v2 (GOLDEN/NULLTEST-Erweiterungen; pluginval bleibt Pflicht)

- **T1** Markierung aus ⇒ bit-transparent (bestehender NULLTEST).
- **T2** Engage Solo/Puls ⇒ Ausgang ≠ Eingang; nach Disengage + Fade-Ende ⇒
  wieder bit-transparent.
- **T3 Render-Beweis (Kernstück):** Fixture speist Blöcke ohne Echtzeit-Takt
  (Freilauf) bei GESETZTEM Latch nach simuliertem Transportstart ⇒ **kein
  einziges Ausgangssample weicht vom Eingang ab** (default-neutral, Lebenszeichen
  schlägt nie an). Zweites Fixture: Echtzeit-Simulation (Blöcke im
  Wanduhr-Takt) ⇒ Markierung blendet nach ~1 s ein. Drittes:
  `setNonRealtime(true)` ⇒ sofort neutral.
- **T4** Abgriff-Beweis: Sinus-Sweep, Solo aktiv ⇒ `MessSnapshot`-LTAS gleich der
  Referenz ohne Solo (ε) — die Messung sieht den Eingang.
- **T5** Klickgrenze: max. Sample-Delta über Engage-/Disengage-/Kartenwechsel-
  Kanten unter Schwelle (linearer Fade wirksam).
- **T6** State-Roundtrip: Speichern mit Latch an → neue Instanz lädt neutral.
- **T7** Kanten: `fBis` an der Nyquist-Kappe · nicht endliche Befundfelder ⇒ kein
  Engage, kein NaN im Ausgang · Mono-Layout · 1/12-Okt-Resonanz (engste Karte).
- **T8** Falsch-Positiv-Riegel unberührt: Pink/Stille ⇒ weiter null Karten;
  Markierung ist kartengebunden, ohne Befund nicht aktivierbar.
- **T9 Ruhephasen-ε:** Während der 0-dB-Phase des Pulses Rest ≤ −120 dBFS
  gegen den Eingang (b=a-Identität + FP-Historie).
- **T10** Transport-Stopp bei Latch ⇒ zwangsneutral; Wiederstart ⇒ erst
  Lebenszeichen, dann Fade-in.

---

## 9. Vertrags-Deltas (Übergabeliste — Codex baut gerade die Hauptansicht)

1. **Bauplan §2.3.12** → „Nakama berät nur. Es setzt keine EQ-Parameter, schreibt
   keine Automation und verändert das Audiosignal **im Renderweg nie und live nur
   während einer vom User gehaltenen Hör-Markierung** (§7.6); die startet erst
   nach bestätigter Echtzeit-Wiedergabe und endet hart bei Fensterschluss und
   Kartenabwahl."
2. **Bauplan neu §7.6 „Hör-Markierung"** → Inhalt aus §2–§6 dieses Konzepts.
3. **Bauplan §8.2** → Hören-Zeile der geöffneten Karte ([Solo] [Puls]).
4. **Bauplan §12/§14** → DSP-Budget-Notiz + Pflichtbeweise T1–T10.
5. **Recherche §9.1** → Ausnahme-Klausel am Punkt „Eingang unverändert an
   Ausgang" mit Verweis hierher.

---

## 10. Erwogen und verworfen (Begehungs-Protokoll)

- **LR4-Bandsplit für den Puls** — Allpass-Phasenrotation auch in der Ruhephase;
  der parallele Bell ist dort exakte Identität (B1).
- **Equal-Power-Crossfade** — falsch für korrelierte Signale (B5).
- **Delta + Korrektur-Vorschau** (soothe-Familie) — Korrektur-Semantik, zu nah am
  verbotenen „Anwenden" (Bauplan §8.2). V2-Kandidat mit eigener Schutz-Diskussion,
  erst wenn die reine Markierung im Alltag sitzt.
- **Mehrfach-Markierung** (mehrere Bänder, verschiedene Pulsraten) —
  Wahrnehmungs-Matsch statt Orientierung.
- **Zusatztöne/Sonification** — maskiert genau das Material, das man hören will.
- **Vorhören über die Tauri-App** (anderer Prozess, Renderweg unberührbar) —
  außerhalb des Mix-Kontexts und asynchron zur Wiedergabe; der Kern-Loop „Regler
  ziehen und Markierung schrumpfen hören" ginge verloren. (Geräte-Frage je nach
  Treiber obendrein: Hardware-ASIO ist oft exklusiv; FL Studio ASIO wäre teilbar
  — ändert am Kontext-Verlust nichts.)
- **Tempo-synchroner Puls** — würde sich im Groove verstecken; taktfremd fällt
  auf (B2).
- **SVF/TPT-Topologie** — für zeitvariable Filter generell erste Wahl, hier aber
  unterlegen: nur die RBJ-Peaking-Form hat die exakte b=a-Identität bei 0 dB, und
  die Modulation läuft ohnehin über konstruktiv stabile Tabellenzeilen (B1/B7).

---

## 11. Aufwand + Reihenfolge (AI-realistisch)

- Prozessor/DSP inkl. Lebenszeichen-Logik + Tests T1–T10: **~75–100 min** —
  unabhängig vom Design-Umbau, kann vorgezogen werden (Bauplan §11.1 lässt den
  Prozessor unangetastet).
- Editor-/Feld-Anbindung: **~30–45 min** — **nach** dem Codex-NAKAMA-Merge
  (one-writer; Einbau in den alten Editor wäre Wegwerfarbeit).
- Heartbeat-Feld + Broker-Messpause + Doku-Deltas: **~30 min**.

## Quellen (Verifikation 2026-08-16)

- Host-Flags unzuverlässig: [Ableton setzt isNonRealtime für VST3 nicht](https://forum.juce.com/t/ableton-vst3-isnonrealtime-not-set/46912) ·
  [Trouble with isNonRealtime](https://forum.juce.com/t/trouble-with-isnonrealtime-for-offline-render/18564) ·
  [kOffline-Verdrahtung in JUCE](https://forum.juce.com/t/vst3-setnonrealtime-in-jucevst3component-setupprocessing/26432) ·
  FL-spezifisches kOffline-Verhalten nirgends belastbar dokumentiert ([Image-Line-Forum zu Offline-Render](https://forum.image-line.com/viewtopic.php?t=215437)) ⇒ Design host-agnostisch.
- Präzedenz Abhör-Plugin mit Band-Solo: [ADPTR Metric AB](https://adptraudio.com/product/metric-ab/) ·
  [SonicScoop-Review](https://sonicscoop.com/new-software-review-metricab-by-adptr-audio/)
- Delta-Abhören etabliert: [soothe2-Manual](https://oeksound.com/manuals/soothe2/)
- Band-Solo als Jagdtechnik: [Gearspace-Übersicht](https://gearspace.com/board/music-computers/1001718-eq-plugins-let-you-solo-bands.html)
- RBJ-Peaking-Identität bei A=1 und Kaskaden-Korrektur √(√2−1): im Dokument
  hergeleitet (Cookbook-Koeffizienten; 2 identische BP-Stufen).
