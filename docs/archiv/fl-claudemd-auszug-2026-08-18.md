> **ARCHIV (21.08.2026). Auszug der FL-Studio-CLAUDE.md vom Umzugstag; sein Präsens („AKTUELLER STAND TIEFENFELD“, „NÄCHSTE SESSION: Design-Freeze“) ist historisch. Keine Vorgabe mehr. Gültige Produktdefinition: CLAUDE.md (Wahrheitskern + Entscheide-Register); technischer Entwurf: docs/FL-Nakama-Sonden-Design-Entwurf.md.**

# FL-CLAUDE.md-Auszug — EQ-Copilot/Nakama (wortgleich gesichert beim Umzug 18.08.2026)

> Diese Blöcke standen bis zum Workspace-Umzug in C:\Users\phili\FL-Studio\CLAUDE.md.
> Code + Docs leben seit dem Umzug in C:\Users\phili\Projekte\Nakama\.

**🎚️ EQ-Copilot — beratendes Mess-VST3 (Stand 2026-08-15 · m4.1 installiert; Meilenstein-
Vollprotokolle M0–m4.1 wortgleich im Archiv 2026-08-15, Beweise je Meilenstein in
`eq-copilot/docs/M*-BEFUND.md` + `CS1-BEFUND.md`):** Architektur: transparentes
Sammler-VST3 (JUCE 8, `eq-copilot/`) misst insert-genau Master/Bus/Instrument +
PRE/POST-Paare (Protokoll v2 mit Heartbeat-Messstand, Konflikt-UI); Tauri-Broker in
`plugin-hub-app/src-tauri/src/eq_copilot/` (Named-Pipe nur-User-ACL, nimmt v1 weiter an,
Sensorübersicht in der Hub-App); Verträge `eq-copilot/schemas/` (Snapshot v3 trägt
`befunde`). **User-Entscheidungen absolut:** rein beratend — setzt NIE Parameter, kein
„späterer Rückweg"; Claude erhielte nur strukturierte Messwerte, keine Werkzeugrechte, erfindet keine Zahlen ·
**„Kernfunktion vor Verwaltung"** (Graph-Diagnose mit konkreten Empfehlungen IST das
Produkt; kein Pflicht-Setup) · Lernsprache Tu/Warum/Hören · Kompaktmodus = vollwertige
Alltagsansicht und Auslegungsmaßstab (R3.1). Vollbild: `FL-EQ-Copilot-Recherche.md`
(§5.10 Regeln, §7 UI); Mockup `FL-EQ-Copilot-Mockup.html`.
**Gebaut + headless bewiesen** (je Stand GOLDEN · NULLTEST · pluginval 8 · cargo grün;
aktuell **GOLDEN 239/239 · cargo 36/36**): M0 Gerüst+Pipe+Lizenz (JUCE-AGPL, Futura
nicht im Plugin) · M1 `AnalyseEngine` (4 Welch-Stufen, Referenz 8192 = exakt die
analyze-track-Achse; BS.1770-LUFS pyloudnorm-exakt; 8×-True-Peak; Abdeckungs-Histogramm;
Resonanzkandidaten mit Gate) · M1.1 Live-Abnahme („passt so") · M2 Protokoll v2 +
PRE/POST-Herabstufung + Profilbindung + Aggregat-Snapshot · M3-Kern `Diagnose.{h,cpp}`:
5 eigenkurven-relative Befundklassen (Resonanz · Mitten-Loch 500–2k [User-Hebel #1] ·
Mulm 120–300 · Härte 2,5–5k · Höhen-Hype) als Befundkarten in Lernsprache mit
Startwerten (Werkzeuge PEQ2 + Smooth Operator Pro); Falsch-Positiv-Riegel bewiesen
(Pink+Stille karten-still); Tilt/„Air fehlt" bewusst weggelassen · CS1 (Fremd-Review-
Härtung): Kanalenergie (PSD_L+PSD_R)/2 statt Mid-Mix [Antiphase/Chöre!], NaN-Riegel,
LTAS-Nyquist-Kappe 0.95, Rust-FFI-Härtung · Benchmark-Studie RESO/smart:EQ 4/Pro-Q 4 →
`eq-copilot/docs/BENCHMARK-STUDIE-RESO-SMARTEQ-PROQ.md` (+ Roadmap M3a/b/c) · M3a +
FPS-Fix (m4): Täter war die 4-Hz-Snapshot-Kadenz, nicht paint() → 20-Hz-Leichtpfad +
Leerlauf-Ruhe; Median-Basislinie ±1/2 Okt; Zonen-Zeitverlauf („NN % der Musikzeit",
Zonen-Konfidenz kann ehrlich „hoch"); Perzentile P10/50/95; „KURVE STEHT"
(<0,35 dB/10 s); Notennamen in FL-Zählung (116 Hz = A#3).
**🧪 Kalibrier-Protokoll (USER-Design, LÄUFT):** Teststück 01 = 8 Instrument-MIDIs
(even-Gerüst B♭m/120, 613 Noten) in `eq-copilot/kalibration/testsong-01/`; Erzeuger +
Verifikator `tools/eq-copilot/` (7/7 grün). Je Runde: User rendert effektfrei →
`analyze-track.py` ↔ Plugin-Snapshot ↔ Ohren; Schwellen-Kalibration, KEIN ML;
Erwartungen vorab in `eq-copilot/kalibration/KALIBRIER-PROTOKOLL.md`. **Runde 1
BESTANDEN** (LUFS Δ0,05, Resonanzbänder frequenzgenau 689/460/919 Hz; die scheinbare
+6-dB-LTAS-Differenz war Stille-Verdünnung + Mid↔Kanalenergie — KEIN Bug) → **m4.1**:
K1 Zonen-Tick-Mindestpegel (Teppich+10) · K2 Wellen-Werkzeugwahl (P95−P50 > 10 dB ⇒
dynamische Erstidee — der Fall war die eigene Melodie A#5). E6 beobachten: „KURVE
STEHT" bei kurzem Stück 0,51 > Schwelle 0,35. **Runde 2 offen:** helle Instrumente
ODER gezielter EQ-Fehler als Blindtest.
**🎨 Hör-Markierung („Einfärben") — GEBAUT 16.08., Version 0.3.0** (User-Idee → Konzept
v2 nach Technik-Begehung → Bau in einem Rutsch, alles gleicher Tag; Spez + Bauplan-Deltas:
`eq-copilot/docs/HOER-MARKIERUNG-KONZEPT.md`): Solo/Puls je Befundkarte (mittenLoch nur
Solo), Puls = parallel modulierter RBJ-Bell (b=a-Identität in Ruhephase, 40/60-Takt
~1,6 Hz, Tiefe 2,0–4,5 dB headroom-bewusst aus eigener TP-Messung), Solo zweigleisig
(Zonen HP+LP Butterworth 4. O. · Resonanzen 2× RBJ-BP mit Kaskaden-Korrektur 0,6436),
linearer Crossfade (equal-power wäre +3-dB-Buckel). **Renderschutz „neutral, bis Echtzeit
bewiesen"**: Markierung erst nach 2×500-ms-Fenstern mit Audiozeit≈Echtzeit, Transportkante/
Lücke/prepareToPlay reset, Freilauf killt Latch, isNonRealtime hart aus — Host-kOffline
bleibt unzuverlässig, darum host-agnostisch. Abgriff VOR der Färbung (Messung sieht nie
gefärbtes Signal), Heartbeat-Feld `hoermarkierung` (additiv, Schema nachgezogen), Latch
endet bei Fensterschluss/SR-Wechsel/10-min-Totmann, nie persistiert. **Beweise:
MARKIERUNGSTEST 30/30 (T2–T10, inkl. Freilauf-Latch = jedes Sample bitgleich) · NULLTEST
10/10 · GOLDEN 239/239 · pluginval 8 SUCCESS.** Interims-UI im alten Editor (Toggle-Chips,
Statuszeilen-Not-Aus, Feld-Tönung phasensynchron); NAKAMA-§7.6-Marker-UI nach Codex-Merge,
Broker-Messpause nach Harness-Lauf. Edison-Echtzeit-Mitschnitt bleibt ehrliche Grenze
(TESTANLEITUNG §18).
**Installiert: 0.3.0 SHA `74D86BD5…`** (16.08. 11:06, UAC-Helfer STATUS=OK, Hash von
außen bestätigt; Rollback m4.1 `B87AB70E…` gesichert in `eq-copilot/install/`).
**Offene User-Prüfungen:** FL-TESTANLEITUNG §11–17 (M2-Roundtrip ·
M3-Karten · flüssiger Graph/„KURVE STEHT"/A#3/%-Musikzeit) + **§18 Hör-Markierung**
(Puls/Solo hören · Render-Beweis · „wartet auf Wiedergabe"); 16-Instanzen/PDC/
Offline-Render = Alltags-Beobachtung. **Nächste Hebel:** M3b Korridor-Kalibration aus
even-Bounces · M3c PRE/POST-Karte, Hover-Inspekt, Passagen, Masking · Dauerbetrieb
(Loudness inkrementell, Pipe-Deadlines/CancelIoEx, Broker-is_finite) · V1-ausgesetzt:
adaptiver Teppich, Passagen-Fenster · Claude-Anbindung: `--bare` scheitert an OAuth
(M0-Spike) → Agent-SDK/API-Adapter.
🔑 **Teuer bezahlte DSP-/Bau-Lehren (bleiben hier):** LTAS-Lücken in LINEARER Leistung
interpolieren, nie dB (7-dB-Fehler an Ton-Flanken) · Live-EMA in Leistung mitteln, nie
dB (Linie lag ~40 dB zu tief) · Schmalband-Pegel sind auflösungsabhängig (ENBW ±3 dB je
Fensterverdopplung) — Komposit-Kurvenvergleich nur Breitband · Pink-Fixture trägt ~46 %
Energie < 20 Hz (Randbin-Falle einseitiger PSD) · Offline-Kurvenvergleiche IMMER
aktiv-gegated + kanalenergie-basiert (Stille-Verdünnung, K3) · Pipe-Namens-Squatting:
zwei Broker auf einem Namen stehlen sich STILL Clients → FILE_FLAG_FIRST_PIPE_INSTANCE
+ eigener Probe-Pipename · Samplerate-Wechsel muss das Projektfenster mit-resetten ·
Halbzeit-Störungen brauchen +10 dB im Fixture (Leistungs-Halbzeitmittelung) · headless
feuern JUCE-8-Editor-Timer nur per `callPendingTimersSynchronously()` · UAC-Install-
Muster: Start-Process -Verb RunAs-Helfer, User klickt, Hash-Beweis von außen.

---

**🖤 NAKAMA = der Plugin-Look (14.08., USER-Entscheid; Wortlaut im Archiv 2026-08-15):**
Der EQ-Copilot-Editor trägt die dunkle NAKAMA-Gerätefront aus dem User-Figma-Entwurf —
`NakamaGehaeuse.h` (Kontur/Kerben/Noppen prozedural als juce::Path; die Figma-Noppe war
Bitmap-Fill und fehlt in jedem CSS/SVG-Export), Graph = Glossy-Display, **Fenster hält
750:520**. Schriftzug NAKAMA, VST3-Kennung bleibt „EQ-Copilot" (Projekte!). Studie:
`NAKAMA-Design-Studie.html`. Sichtprüfung ohne FL: `EqCopShot.exe` (Offscreen-PNG mit
echter 20-s-Messung). Hub-App bleibt hell (Leitstand).
⚠️ Das ist der Look des INSTALLIERTEN Plugins (0.3.0) — die Neuausrichtung darunter
ersetzt ihn nach Design-Freeze.

**🎨 NAKAMA-Neuausrichtung „Papier & Tusche" (16.08., USER-Entscheide, Design läuft):**
App wird sichtbar zu **Nakama** (仲間 „Weggefährte") umbenannt; innere VST3-Kennung
bleibt EQ-Copilot (Projekte!). Identität = **japanische Mentalität: der Graph ist eine
Tuschezeichnung** — die vier Befunde leben im Blatt: Bass-Stau · Grundleben ·
Mittenloch · Dauerresonanz (ursprünglich als PAPIER-Schäden gedacht; seit dem
Shader-Verwurf 16.08. abends erlebt die **TUSCHE** die Befunde — s.u.).
Interview-Antworten (bindend): hell (frisches Washi) UND
dunkel (getränktes Blatt) · Kurve = Handzeichnung (Pinselbreite=Energie, Kasure) · alle
4 Ereignisse beispielhaft bauen, User entscheidet am Bild · aus 3 User-Referenzbildern
2 Stil-Varianten: **„Tusche & Gold"** (Creme/Tiefschwarz/Goldadern/Siegelrot) und
**„Nebel & Nacht"** (Sumi-Nebel hell · Petrolnacht+Stroh dunkel), Siegelrot = die eine
Signalfarbe · ALLES auf dem Papier geschrieben (Archiv als Rollbild, Symbole als
Wachssiegel/Zeichnung) · Symbolalphabet bauen User+Codex (v2 mit Nakama-Phase-Schrift
übernommen). **Arbeitsteilung: Codex besitzt das Vorentwurf-Original; Claude baut
parallel auf KOPIE `eq-copilot/design/nakama-spectral-field-claude.html`
(= das Musterblatt).** Verlauf: Runde 1 (Graph als CanvasTexture auf wehendem
3D-Blatt) → User: „Blatt gut, Effekte nicht"; Runde 2 „Echte 3D-Effekte" E0–E5
(Composer/Bloom/onBeforeCompile, Regler-Beweis 18/18) → ⛔ **USER 16.08. abends:
„das shader model wird komplett verworfen — das sieht sehr schlecht aus."**
Befund am Bild: Riss/Brand = Blasen-Schablonen, Papierkorn = Beton, Dunkelwelt =
Schmelzen. 🔑 **Teure Lehre: der Regler-Pixel-Beweis maß nur ÄNDERUNG, nie
Schönheit — prozedurale Material-Simulation (Noise+Glow+Verformung) erreicht die
Referenz-Qualität nicht, zweimal bewiesen.** Verworfen-Vermerk im Plan-Doc
(`eq-copilot/docs/NAKAMA-PAPIER-3D-EFFEKTE-PLAN.md`); E0–E5 nur noch Git-Historie.
Runde 3 (Tusche-Einzelmarken, `452e7fd`) ebenfalls VERWORFEN — **USER 17.08.:
Bruch-Borsten = „zerissenes Stroh", Siegel = „Kochhut", Hintergrund zu
statisch; FabFilter nutzt TIEFE als Mittel; „wir müssen den Graphen nutzen —
ein Ereignis muss visuell Info zeigen; kein verschiedenes, EIN gutes."**
→ **AKTUELLER STAND „TIEFENFELD" (17.08., Commit `0e4f01e`): EIN Mechanismus —
die Abweichung vom Bleistift-Plan wird Relief, der Graph selbst trägt alles.**
Jedes Sichtwerkzeug trägt GENAU EINE Info (3. Fassung 17.08.: **„Weiß
fliegt raus — erkennt man nichts" → NUR NOCH DUNKLE WELTEN** [tusche-dunkel
= :root-Standard, nebel-nacht; Hell-Paletten gelöscht], und auf dunklem
Blatt erzählt LICHT statt Schatten): Strich = Ist (hungert im Tal) ·
Bleistift = Soll · Fahne (Lamellen 3 Dichten + Masse zwischen Plan und
Strich) = die Abweichung · **Befund-NEBEL = WO** (Fog verdichtet sich über
dem Frequenzband aktiver Befunde, Dicke = Schwere, wogt; Ganzflächen-
Spektrumnebel raus) · **LICHT von oben = WIE TIEF** (`tiefenLicht`:
Lichtsäule fällt unter den Plan, so tief wie der Befund 1:1 px — vom
Feld-Regler entkoppelt; `talLicht`-Kante unter der Plan-Lippe;
`strichSchein` = Lichthof statt Schlagschatten; Lichtzug vertikal) ·
Pool = Gedächtnis · Lichtkamm ENTFERNT (färbte Tusche weiß) ·
Deko-Anmerkungen GELÖSCHT; Marker-Siegel verschwinden mit abgeschaltetem
Befund · t3-Werkzeug-Serie = je Sichtwerkzeug ein Solo-Render ·
Chips = BEFUND-DATEN
(Resonanz/Mulm/Loch/Härte, `?aus=resonanz,…`) · Kali 10 Regler
(`nebel`/`feld`/`schatten` ERSETZEN riss/brand/saugen) + **Farbwahl je
Graph-Element** (10 Picker, PRO WELT in localStorage `nakama-farben`;
Kopier-JSON = Transfer-Vertrag inkl. `farben`) · `?kali=schluessel:wert`
setzt Regler per URL (Beweis t2-schatten-0/200) · Washi statisch (Wolken +
Einzelfasern, NIE Hochfrequenz-Korn) · `?zeit=` friert · Renders
`node eq-copilot/design/render-blatt.mjs` → `vorentwurf-renders/t2-*.png`.
**NÄCHSTE SESSION:** User urteilt am LEBENDEN Blatt (der Nebel atmet nur
live), Welt + Drastik über Stift-Regler, „Werte kopieren"-JSON in den Chat =
Design-Freeze (wird JUCE-Default); danach Rollbild-Archiv/Panels papieren;
JUCE-Umsetzung = eigener Meilenstein nach Design-Freeze.
