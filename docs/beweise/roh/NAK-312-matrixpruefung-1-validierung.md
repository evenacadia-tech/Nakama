# NAK-312 — Validierung der Matrixprüfung 1 an der Quelle

| Merkmal | Wert |
|---|---|
| Ticket | NAK-312, Planschritt S25k, Etappe 1 (Verhaltensmatrix und Bauplan), Validierung der Matrixprüfung 1 |
| Auftrag | `docs/beweise/roh/NAK-312-matrixpruefung-1-validierung-auftrag.txt` (Dirigent, 21.09.2026) |
| Gegenstand | Prüfbericht `docs/beweise/roh/NAK-312-matrixpruefung-1-urteil.md` (URTEIL NEEDS_WORK: D1 bis D9, L1 bis L3, H1 bis H3) über `docs/beweise/NAK-312.md` §5 bis §8 (Stand `2deb5e3d`), Gate-Bezug §0 bis §4 und §9.2 |
| HEAD zu Beginn | `bc32f3c993bdf10e0b03a7610dcd5ade2fb3c471` |
| HEAD vor dem Schreiben | `bc32f3c993bdf10e0b03a7610dcd5ade2fb3c471` (unverändert) |
| Arbeitsbaum | zu Beginn und vor dem Schreiben `git status --short` = `?? briefing-hub/`, `?? nimbalyst-local/` (fremd, nicht angefasst) |
| Datum | 21.09.2026 |
| Modell | Claude Opus 5 (`claude-opus-5`), frischer lesender Validierer; weder Bauer noch Prüfer |
| Prüfform | Quellenlektüre. `git diff --stat 090d0fa1 HEAD -- eq-copilot broker tools` ist leer (gemessen): Produkt-, Test- und Werkzeugcode am HEAD ist der Code am ZIEL. `git diff --unified=0 2deb5e3d HEAD -- docs/beweise/NAK-312.md` zeigt nur `@@ -8,2 +8,2 @@` und einen Anhang ab Zeile 1429: jede Zeilenangabe in §0 bis §8 gilt am HEAD. Kein Bau, kein Testlauf, kein Kanon, kein Skript, kein FL Studio; Git nur lesend. JUCE-Quellen unter `eq-copilot/build/_deps/juce-src/` sind Bauartefakt: gelesen, nicht gegen den gepinnten Stand verifiziert. |
| Geschrieben | nur diese Datei (LF); nichts gestaged, committet oder gepusht |

**Kürzel.** „Manifest" = `docs/beweise/NAK-312.md`; eine Zeilenangabe ohne Pfad in
Manifestzitaten meint diese Datei. „Prüfauftrag" =
`docs/beweise/roh/NAK-312-matrixpruefung-1-auftrag.txt`. „Etappe-1-Auftrag" =
`docs/beweise/roh/NAK-312-etappe-1-auftrag.txt`. „Entwurf" =
`docs/FL-Nakama-Sonden-Design-Entwurf.md`. Codepfade relativ zu `eq-copilot/plugin/`,
wo kein anderer Pfad steht.

**Urteilsbegriffe.** BESTÄTIGT: Reproduktion und Einordnung tragen an der Quelle
(eine Ergänzung ändert daran nichts). PRÄZISIERT: der Kern trägt, eine tragende
Einzelheit oder die Einordnung weicht ab (genannt). WIDERLEGT: trägt an der Quelle
nicht.

---

## Teil 1 — die fünfzehn Befunde

### D1 — Weg 2 deckt die globalen Gains, nicht die Bandwerte

**a) Beanstandeter Satz.** Manifest `:1047-1054` (§7.3, Weg 2): „Der Kern liest am
Blockrand — und nur, wenn `hostEreignisOffen` steht — die vorhandenen Mailboxfelder
(`SondeProcessor.h:541-542`, Atomics) und setzt daraus die Rampenziele derselben
kontinuierlichen globalen Gains, die er heute aus dem Programm setzt (…). Abgedeckt
sind genau die Parameter, für die das Overlay heute schon durchlässt
(`eq-copilot/plugin/state/NakamaTransaktion.cpp:345`: `! topologisch`)"; dazu
`:1066-1070`: „gleicher Event- und Blockverlauf liefert in Echtzeit und offline
dieselbe Ausgabe, und der Entwurfssatz „vom vorigen zum letzten Blockwert" (…) ist
wörtlich erfüllt."

**b) Nachmessung.**
- `state/NakamaTransaktion.cpp:345`: `if (automationOverlay.gesetzt[(size_t) i] && (samplegenau || ! t[(size_t) i].topologisch))`;
  `samplegenau` ist aus: `sonde/SondeProcessor.cpp:155`
  `transaktion->setzeSamplegenaueAutomation (false);`.
- `state/NakamaParameter.cpp:30` (in `gleitkomma`): `b.wechsel = Wechsel::rampe; b.topologisch = false;`.
  Tabelle `:49-81`: vier globale (`:50-53`), je Slot acht (`:64-66`, `:69-73`),
  `v2.global.mix` (`:80`) — 69 kontinuierliche Hostparameter, nachgezählt.
- Ergänzung: auch elf **boolesche** Hostparameter sind nichttopologisch und passieren
  das Overlay — `:49` `boolean ("v1.global.bypass", false, false)`, `:62`
  `boolean (p + "enabled", false, false)` (achtmal), `:79` `v2.global.eq_enabled`,
  `:81` `v2.global.auto_gain` (`boolean` übernimmt das Argument, `:20`). Das Overlay
  lässt 80 Hostparameter durch; topologisch sind nur `type`, `channel_mode`,
  `dynamic_enabled`, `sidechain_source` (`:63`, `:67`, `:68`, `:74`), zusammen 32.
- Blockrand: `dsp/DspKern.cpp:620-626` setzt genau fünf Rampenziele — und nur, wenn
  eine Bank übernommen wurde (`:531-532` `const int neu = baenke.uebernehmeBereiten (p); if (neu == -1) return;`).
  1:1 abbildbar sind vier Hostparameter: `input_trim_db` (`dsp/DspProgramm.cpp:259`,
  `:271`), `output_trim_db` (`:260`, `:272`), `width` (`:261`), `v2.global.mix`
  (`:264`). Das Ziel `autoGain` ist abgeleitet (`:367`
  `aus.autoGainDb = leiteAutoGainAb (aus, &autoGainRoh);`), kein Hostparameter.
- Nur über ein Programm wirken: `gain_db`, `freq_hz`, `q` (`DspProgramm.cpp:297-304`
  `b.statisch = entwurf (…)`, `:312-313` `svfGrundG`/`svfEntwurf`),
  `dynamic_range_db`, `threshold_db` (`:316-317`), `attack/hold/release_ms`
  (`:350-354`), `mono_bass_hz` (`:277-279`), die elf Booleschen (`:258`, `:263`,
  `:265`, `:285-293`). Das Programm baut der Worker unter dem Schloss:
  `SondeProcessor.cpp:1449` und `:1476-1477`.
- Matrix: M-20 (`:660`) „dieselbe Eventfolge auf `v1.global.output_trim_db`"; M-21
  bis M-25, M-27 bis M-29 bauen darauf; M-26 (`:666`) nennt keinen Parameter. Der
  Harnisch kennt Bandparameter: `tests/TransactionTestMain.cpp:2050-2053`.
- Ergebnis: Reproduktion stimmt. Nach Weg 2 bleiben 65 kontinuierliche **und** 11
  boolesche, also 76 der 80 durchgelassenen Hostparameter wanduhrgebunden, dazu das
  abgeleitete Auto-Gain-Ziel, sobald Bänder sich bewegen.

**c) Was bricht.** R-312-10 (Manifest `:177-181`): „Die Übernahme von
Hostparameterwerten ist an den Audioblock gebunden, nicht an die Wanduhr: derselbe
Event- und Blockverlauf liefert offline und in Echtzeit dieselbe Ausgabe innerhalb
der Toleranz von M-120; kontinuierliche Werte rampen vom vorigen zum letzten
Blockwert (Entwurf §44.3, Rückfallsatz)." E-312-1 (`:1444-1447`): „… nur für die
nichttopologischen kontinuierlichen Hostparameter …" — Weg 2 erreicht davon vier.
SONDE-015 M-120 (`docs/beweise/SONDE-015.md:1224`): „Beide erzeugen **denselben
Parameterverlauf** und damit innerhalb numerischer Toleranz denselben Ausgang, bei
gleichem Event- und Blockverlauf." Entwurf `:3084-3086`. Der Bauplan widerspricht
sich außerdem selbst: „Rampenziele derselben kontinuierlichen globalen Gains" (fünf
Ziele) gegen „Abgedeckt sind genau die Parameter, für die das Overlay heute schon
durchlässt" (80).

**d) Urteil.** BESTÄTIGT (Ergänzung: 80 statt 69 durchgelassene Parameter).
Einordnung **DEFEKT**. Kein Ausschluss des Prüfauftrags: Bauplan §7.3; der Befund
verlangt keinen Wrapperpatch (Ausschluss 3).

**e) Schließende Regel.** Matrix und Bauplan nennen die blockgebundene Parametermenge
namentlich und codegleich; was nicht blockgebunden wird, steht mit datierter
Entscheidung außerhalb von R-312-10 und E-312-1, und mindestens ein Bandwert und ein
boolescher Parameter haben je eine messende Zeile.

---

### D2 — Ordnungsvergleich auf dem überlaufenden Zähler

**a) Beanstandeter Satz.** Manifest `:1058-1061`: „eine Publikation setzt ein
Rampenziel nur, wenn ihr Ereigniszählerstand nicht älter ist als der zuletzt am
Blockrand gesehene; verglichen wird derselbe monotone Zähler wie in `:1460`." Dazu
`:1065`: „Kein Speicherzuwachs". Gegensatz: M-19 (`:650`).

**b) Nachmessung.** `sonde/SondeProcessor.h:542`
`std::array<std::atomic<std::uint32_t>, (size_t) nakama::parameter::kHostParameter> hostEreignis {};`,
`:543` `std::array<std::uint32_t, …> hostEreignisGesehen {};   ///< unter Zustandsschloss`;
`SondeProcessor.cpp:1336` `hostEreignis[(size_t) index].fetch_add (1, std::memory_order_relaxed);`
(läuft über), `:1460` `if (n == hostEreignisGesehen[(size_t) i]) continue;`.
Reproduktion stimmt. Ergänzungen:
- Speicher: einen Ereignisstand gibt es weder im Kern noch im Programm; `generation`
  ist der Bankzähler (`dsp/DspBankPool.h:166-171`), kein Hostereigniszähler.
- Zweiter Verbraucher des Flags (nicht im Prüfbericht): `hostEreignisOffen` hat heute
  genau einen Verbraucher, `SondeProcessor.cpp:1455`
  `if (hostEreignisOffen.exchange (false, std::memory_order_relaxed))`. Weg 2 macht den
  Blockrand zum zweiten („nur, wenn `hostEreignisOffen` steht", `:1048`; „im
  schlechtesten Fall ein `exchange`", `:1061-1062`). Wer zuerst tauscht, nimmt dem
  anderen die Meldung: verliert sie der Worker, laufen Overlay, Epoche und Ruhegrenze
  (`:1457-1473`) für diese Ereignisse nicht.

**c) Was bricht.** M-19 (Manifest `:650`): „Die Quittierung vergleicht auf Gleichheit,
nicht auf Ordnung: nach dem Überlauf wirkt das nächste Ereignis, und kein Ereignis
wird doppelt verbraucht", Rotbeweis „`==` bei `:1460` durch `<` ersetzen → das erste
Ereignis nach dem Überlauf verschwindet → rot". Der Bauplan legt auf denselben Zähler
den Vergleich, den M-19 als Bruch definiert. Verbraucht der Blockrand das Flag, bricht
SONDE-015 M-81 (`docs/beweise/SONDE-015.md:1159`): „Die Epoche wechselt einmal zu
Beginn und einmal am Ende einer Hostgeste beziehungsweise nach einer definierten
Ruhegrenze."

**d) Urteil.** BESTÄTIGT (Ergänzung: zweiter Verbraucher des Flags). **DEFEKT**. Kein
Ausschluss.

**e) Schließende Regel.** Die Schiedsregel zwischen Blockrand und Publikation kommt
ohne Ordnungsvergleich auf `hostEreignis` aus (Teil 2, Punkt 2), das Flag behält
genau einen Verbraucher, ihr Speicher ist genannt, und eine Zeile misst den
Zählerrand auf der Schiedsseite.

---

### D3 — `beendeCandidate` aus dem Callback verletzt die Besitzregel

**a) Beanstandeter Satz.** Manifest `:1189-1195` (§7.5 Schritt 3): „Die
klangwirksame Hälfte läuft sofort im Callback: `beendeCandidate` (`DspKern.cpp:434-443`)
arbeitet nur mit Atomics und einer Publikation und lässt die Candidate-Bank los; …"

**b) Nachmessung.** `dsp/DspKern.cpp:439` `candidateAktiv.store (false, std::memory_order_release);`
(atomar), `:441` `merkzettel[(size_t) Pfad::candidate].gueltig = false;` (nicht
atomar), `:442` `baenke.publiziereEnde (Pfad::candidate);`. `dsp/DspKern.h:599-603`:
„NAK-311 W03: Merkzettel und Zaehler gehoeren ALLEIN dem Worker - kein Atomic, keine
Sperre, nichts davon liest der Audiothread. …" / `std::array<Merkzettel, (size_t) kPfade> merkzettel {};`.
Weitere Zugriffe: `DspKern.cpp:288` (`merke`), `:324` (`vergebeKennungen` liest),
`:395` (ENDE-Marke). Reproduktion stimmt. Präzisierungen:
- Eine zweite Besitzregel bricht: `DspKern.h:157` stellt `beendeCandidate` (`:205`)
  unter „//== Control-Worker-Seite - laeuft NIE im Audiothread"; `publiziereEnde`
  steht in `dsp/DspBankPool.h:154` unter derselben Überschrift und ruft
  `verdraengeBereiten` (`dsp/DspBankPool.cpp:157-162`), das einen publizierten, noch
  nicht genommenen Slot per CAS `bereit → frei` setzt (`:136`). Aus dem Callback hieße
  das: der Audiothread gibt eine Bank frei.
- „Heute ruft nur der Worker `beendeCandidate` (`NakamaTransaktion.cpp:763`)" ist
  ungenau. `:763` liegt in `DspKernAusfuehrung::beendePreview`, und die erreichen drei
  Aufrufer, alle unter `zustandSchloss`: `SondeProcessor::beendePreview`
  (`SondeProcessor.cpp:1273-1277`), `setNonRealtime` (`:1325-1326`), `ladestart`
  (`state/NakamaTransaktion.cpp:268`, aus `setStateInformation` unter dem Schloss
  `SondeProcessor.cpp:1067-1089`). `setNonRealtime` ruft der VST3-Wrapper im
  Prozessaufruf (`juce_audio_plugin_client_VST3.cpp:3890-3892`, Bauartefakt): heute
  erreicht schon der Audiothread `beendeCandidate` — serialisiert mit dem Worker durch
  genau das Schloss, das T3-01-03 beanstandet. Erst der Plan nimmt das Schloss weg und
  erzeugt das Rennen.

**c) Was bricht.** Kommentar mit Besitzregel `DspKern.h:599-602` (oben) und
`DspKern.h:157`; `DspBankPool.h:37-39`: „WARUM DER AUDIOTHREAD NIE FREIGIBT: … `frei`
macht ausschliesslich der Worker (`ernteAcks`)."; Poolvertrag `DspBankPool.h:10-13`:
„Der Control-Worker schreibt ausschliesslich free-Slots … Der Audiothread uebernimmt
nur am Blockrand, liest bis Fade-Ende unveraendert". Der Satz „arbeitet nur mit
Atomics" widerspricht `DspKern.cpp:441`.

**d) Urteil.** PRÄZISIERT (zweite Besitzregel im Pool; „nur der Worker" ungenau).
**DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** Der Callback berührt beim Offline-Wechsel nur Atomics, die
ihm gehören oder ausdrücklich geteilt sind; Merkzettel, Poolpublikation und Freigabe
bleiben beim Worker unter dem Zustandsschloss (Teil 2, Punkt 3).

---

### D4 — M-41 heißt Regressionswache, obwohl der Bau sie verschiebt

**a) Beanstandete Zeile.** M-41 (Manifest `:697`): Zusage „Er beendet die Vorschau:
`previewAktiv()` ist danach false und `gewuenschteHoermatrix()` ist Processed,
spätestens nach dem ersten Block"; Frist „Hörmatrix ist ein Atomic (…) und wirkt
sofort; die Buchhaltung des Transaktionskerns zieht der nächste Kontrolltakt nach";
Rotbeweis „Regressionswache (hält heute)". Dazu §7.5 Schritt 4 (`:1196-1199`).

**b) Nachmessung.** `SondeProcessor.cpp:1309-1313` liest `transaktion->preview().aktiv`
unter `zustandSchloss`. Takt `:575-578`: `if (juce::Time::getMillisecondCounterHiRes() >= naechsterKontrollTakt) { dspKontrollTakt(); naechsterKontrollTakt = … + 5.0; }`.
B7: `tests/TransactionTestMain.cpp:2227` `a->setNonRealtime (true); b->setNonRealtime (true);`,
`:2228` je 8 Blöcke, `:2229-2230` je 100 Blöcke, `:2231` `! a->previewAktiv()`;
`fahreAudio` (`:1751-1768`) ruft keinen `kontrollTaktFuerTest`. Der Worker läuft im
Bein mit (`SondeProcessor.cpp:188-189`, außerhalb des Testdefines). Reproduktion
stimmt. Präzisierungen:
- M-41 widerspricht sich selbst: die eigene Fristspalte sagt, die Buchhaltung ziehe
  erst der nächste Kontrolltakt nach; die Zusage verlangt `previewAktiv() == false`
  „spätestens nach dem ersten Block".
- „Die 108 Blöcke … rechnen deutlich unter 5 ms" ist ungemessen und für den Befund
  nicht nötig. Tragend ist: zwischen `:2227` und `:2231` liegt kein
  deterministischer Takt.
- Der Takt ist kein fester 5-ms-Takt: bei leerer Analysequeue schläft der Worker
  zusätzlich bis zu 20 ms, `:739-740` `workerWarte.wait_for (l, std::chrono::milliseconds (20), [this] { return ! workerLaeuft.load(); });`,
  und `processBlock` weckt ihn nicht (geweckt wird nur `:225` im Destruktor und `:294`
  in `prepareToPlay`).

**c) Was bricht.** Etikettdefinition §6 (Manifest `:582-584`): „**Regressionswache**:
heute grün und hält, was der Bau nicht ändern darf". Matrixregel (`:589-593`): „Keine
Zeile hängt an Wandzeit oder Zufall: Ladestart, Blockrand, Kontrolltakt und
Destruktorfolge sind deterministisch ansteuerbar (`kontrollTaktFuerTest` …)". Nach
Etappe 5 hängt `:2231` am Wettlauf mit dem Workertakt.

**d) Urteil.** PRÄZISIERT (Zeile widerspricht sich selbst; Zeitzahl ungemessen, Takt
5 bis 20 ms). **DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** M-41 trennt die sofortige Hörmatrixhälfte
(Regressionswache) von der Buchhaltungshälfte (geänderte Zusage: false nach dem
nächsten Kontrolltakt), und jede Prüfung von `previewAktiv()` folgt einem
ausdrücklichen `kontrollTaktFuerTest()`.

---

### D5 — B6 bekäme eine Behauptung über einen Prozessor, den B6 nicht baut

**a) Beanstandeter Satz.** Manifest `:935-938` (§7.2): „B6 (`tools/beweise.ps1:723`)
und B7 (`:728`) bekommen je einen Satz: der Zähler misst den Bereich ab dem Eintritt
in `SondeProcessor::processBlock`, nicht erst ab `DspKern::verarbeite` …"

**b) Nachmessung.** `CMakeLists.txt:1148-1154`:
`juce_add_console_app(EqCopDspGoldenTest …)`,
`target_sources(EqCopDspGoldenTest PRIVATE tests/DspGoldenTestMain.cpp)`,
`nakama_kern_anbinden(EqCopDspGoldenTest)`. `nakama_kern_anbinden` (`:277-290`) linkt
`NakamaKern` und vier juce-Grundmodule; die Kernquellen (`:198-220`) enthalten kein
`sonde/SondeProcessor.cpp` (die Sonde wird je Ziel übersetzt,
`sonde/SondeProcessor.h:32-35`). Einziger `RtWache::Bereich` im Produkt:
`dsp/DspKern.cpp:1059` (Grep über `eq-copilot/`: sonst nur
`tests/DspGoldenTestMain.cpp:6621`). B6-Behauptung `tools/beweise.ps1:723` beginnt
„Aktiver DSP-Kern (plugin/dsp/) als Bibliothek". Reproduktion stimmt.

**c) Was bricht.** R-312-1 (Manifest `:126-129`): „Die Behauptungszeilen der Beine,
die sich auf die Wache stützen, sagen genau, was der Zähler sieht; …"

**d) Urteil.** BESTÄTIGT. **DEFEKT**. Kein Ausschluss (Behauptungszeilen sind
Gegenstand von R-312-1 und §7.2).

**e) Schließende Regel.** Jede Behauptungszeile nennt den Bereich, den ihr Bein
wirklich öffnet: B6 ab `DspKern::verarbeite` (Kernbibliothek ohne Prozessor), B7 und
die Sondenbeine ab `SondeProcessor::processBlock`.

---

### D6 — M-55: `end` ohne `begin`, und ein Fenster bis zum Timertick

**a) Beanstandete Zeile.** M-55 (Manifest `:726`): Zustand „Gen in Legacy-Rolle
(`sensor`), Marker SOLO oder PULS eingereicht, Editor offen"; Ereignis
„`setzeBindung ("hub", …)`, danach Blöcke fahren"; Zusage „Der Marker ist beendet,
bevor die Fläche wechselt: der Ausgang bleibt über 512 Samples nach dem Rollenwechsel
bitgleich zum Eingang, und im Interventionsring steht genau EIN `end` zu dem offenen
`begin`". Dazu §7.6 (`:1254-1265`).

**b) Nachmessung.**
- Hälfte 1: `src/prozessor/State.cpp:272`
  `istMainKlassifiziert.store (lebenslauf.audioAusnahmeErlaubt());`;
  `state/NakamaLebenslauf.h:137`
  `bool audioAusnahmeErlaubt() const noexcept { return zustand == Klassifikation::main; }`;
  `src/PluginProcessor.cpp:874` `const bool erlaubt = istMainKlassifiziert.load (std::memory_order_relaxed) && …`;
  `src/HoerMarkierung.h:493-499` `zielAn` verlangt `erlaubt`; `begin` nur bei
  `istHoerbar && ! warHoerbar` (`:618-623`); Sender `PluginProcessor.cpp:980-983`.
  In Legacy entsteht kein `begin`; die Zusage verlangt ein `end` dazu.
- Hälfte 2: `src/PluginEditor.cpp:230` `startTimerHz (30);`, `:244`
  `wechsleFlaecheWennNoetig();` im `timerCallback`; `setzeBindung` schärft synchron
  (`State.cpp:343-344`). Bis zum nächsten Tick (Periode 33,3 ms) blendet der Marker
  ab dem nächsten Block ein (`HoerMarkierung.h:588-596`) und meldet `begin`; der
  Tick beendet ihn danach über den Ausfade (`:569-570`: „bei 48 kHz, 80 ms Fade
  (3840 Samples)") mit `end`. Reproduktion stimmt in beiden Hälften.
- Zusatz: `wechsleFlaecheWennNoetig` tauscht die Fläche im selben Aufruf
  (`PluginEditor.cpp:845-849`), das `end` entsteht erst nach dem Ausfade in späteren
  Blöcken.

**c) Was bricht.** Die Zusage von M-55 ist in sich unerfüllbar. §7.6 (`:1259-1265`):
„Das Intervallende entsteht dabei im Audiothread über den regulären Ausfade (…) Erst
danach wechselt die Fläche." widerspricht `PluginEditor.cpp:845-849`. R-312-8
(Manifest `:169-170`): „kein Handgriff wirkt, dessen Bedienelement nicht sichtbar
ist" — der in Legacy eingereichte Auftrag wirkt im Fenster und danach ohne
sichtbaren Aus-Knopf.

**d) Urteil.** BESTÄTIGT. **DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** Die Rücknahme sitzt synchron im Klassifikationswechsel auf
dem Nachrichtenthread, bevor `istMainKlassifiziert` kippt, und M-55 sagt für die
Richtung zu Main zu: kein `begin`, kein `end`, Ausgang bitgleich ab dem ersten Block
(Teil 2, Punkt 4).

---

### D7 — M-74 verlangt, was §7.6 nicht baut

**a) Beanstandete Zeile.** M-74 (Manifest `:745`): Frist „`aktualisiereSourcesSteuerung`
(…) sucht das Ziel in derselben Menge, die `:1135` zeichnet"; Zusage „Kein sichtbarer,
scharfer Knopf steht auf einer ungezeichneten Quelle …". §7.6 (`:1309-1313`) baut nur
`stelleZielSicher`.

**b) Nachmessung.** `src/PluginEditor.cpp:905-907` sucht `q.hauptziel` über die ganze
`sourcesAnzeige.quellen`; `:919` `sourcesAktionKnopf.setVisible (mainFlaecheAktiv && hatZiel);`,
`:933` `sourcesLabelFeld.setVisible (mainFlaecheAktiv && hatZiel);`. Gezeichnet
werden die ersten `sourcesZeilen().size()` Einträge derselben Liste (`:1135-1138`),
geklickt ebenso (`:980-986`); bei 760x430 ergibt `:956-964` 430 − 16 − 48 = 366 px,
Zeilenhöhe `jlimit (18, 34, 366 / n)` = 18 ab 21 Quellen, also 20 Zeilen
(nachgerechnet). `src/SourcesModel.cpp:1774-1775`
`if (! hauptziel.empty() && eintraege.count (hauptziel) != 0) return;` — Ersatz nur bei
fehlendem Ziel; Sortierung `:1651-1664`. Reproduktion stimmt. Präzisierung zum
Umordnungsweg: der sichtbare Name folgt dem Labelfeld nur, wenn die Quelle keinen
gültigen Hostbusnamen meldet (`SourcesModel.cpp:1781-1789`: Hostname vor
`userLabel`). Häufiger rückt das Hauptziel durch eine beitretende Quelle mit kleinerem
Sortierschlüssel oder durch eine Hostumbenennung hinter Zeile 20 — das Ergebnis ist
dasselbe.

**c) Was bricht.** R-312-6 (Manifest `:156-158`): „Ersatz-Hauptziel und jede
Aktionssteuerung wählen nur aus der gezeichneten Menge, in Anzeigeordnung; kein
scharfer Knopf steht auf einer ungezeichneten Quelle." Die zweite Hälfte baut §7.6
nicht; M-74 misst sie nicht, weil ihr Szenario („Hauptziel per `stelleZielSicher`
vergeben") schon durch §7.6 heilt.

**d) Urteil.** PRÄZISIERT (Umordnungsweg). **DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** `aktualisiereSourcesSteuerung` bindet Knopf und Labelfeld
nur an ein Hauptziel innerhalb der gezeichneten Zeilen, und eine Zeile misst den Fall
„Hauptziel existiert, rückt durch Umordnung hinter die letzte gezeichnete Zeile"
(Teil 2, Punkt 5).

---

### D8 — M-04 und M-40 behaupten 200 und 0 für dieselbe Messung

**a) Beanstandete Zeilen.** M-04 (Manifest `:624`): „**`RtWache::sperren()` = 200** …",
Aufbau „Bein öffnet den Bereich wie der Wrapper um das Paar `setNonRealtime(true)` und
`processBlock`", Ereignis „200 Blöcke, vor jedem `setNonRealtime (true)`". M-40
(`:696`): derselbe Aufbau „(M-04)", dasselbe Ereignis, „**`RtWache::sperren()` = 0**".

**b) Nachmessung.** §7.5 Schritt 1 (`:1179-1182`): „Ist er gleich, kehrt die Funktion
sofort zurück"; Schritt 4 (`:1196-1199`): die Buchhaltung läuft im Takt unter dessen
Schloss. Nach Etappe 5 nimmt `setNonRealtime` weder im Normalfall noch beim Wechsel ein
Schloss; heute bei jedem Aufruf mit `offline == true`
(`SondeProcessor.cpp:1324-1325`). Beide Fälle bleiben in B7. Reproduktion stimmt.

**c) Was bricht.** Prüfauftrag `:16`: „… zwei Etappen oder zwei Änderungssätze einer
Etappe berühren dieselbe Funktion mit widersprüchlichen Änderungen (insbesondere
setNonRealtime zwischen Etappe 2 und 5 …)". Weder §6 noch §7.5 (auch nicht „Nicht
angefasst", `:1247-1250`) stellt M-04 in Etappe 5 um.

**d) Urteil.** BESTÄTIGT. **DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** M-04 ist an Etappen gebunden: der Änderungssatz der Etappe 5
stellt sie ausdrücklich um, und die Instrumentaussage trägt danach der Rotbeweis von
M-40 (Teil 2, Punkt 6).

---

### D9 — M-39 hängt an einer Wanduhrkoinzidenz

**a) Beanstandete Zeile.** M-39 (Manifest `:690`): Ereignis „die Schranke wird exakt
bei `kStopFristMs` freigegeben"; Zusage „… entweder hat der Destruktor gewartet
(`gewartetMs` ≥ 1) oder der Rückruf wurde abgewiesen (`abgewiesen` ≥ 1) — nie beides
0, und in keinem Fall ein Zugriff nach dem Ende".

**b) Nachmessung.** `core/ipc/controlclient/Schleuse.h:121-139`: erstes Lesen `:123`,
Erhöhen `:128`, zweites Lesen `:131`; `abgewiesen` steigt nur bei `:125` und `:134`.
`schliessen` `:145-165`: `if (aktivZaehler.load (std::memory_order_seq_cst) == 0) return;`
(`:148-149`) — `gewartetMs` wird nur gesetzt, wenn gewartet wurde (`:164`).
`SondeProcessor.cpp:228-229` `telemetryV3.stop(); controlV3.stop();`; Frist
`core/ipc/ControlClient.cpp:122-133` (Wanduhr, danach `thread.detach()`). Ergebnis
stimmt. Präzisierung: M-30 (`:681`) legt den Haken „VOR dem ersten Besitzerzugriff"
fest, nicht vor oder hinter `betreten()`; §7.4 (`:1121-1126`) lässt das ebenfalls
offen. Vor `betreten()` kann `abgewiesen` steigen, dahinter nur `gewartetMs`. In
beiden Lagen gilt: endet der bei `kStopFristMs` freigegebene Rückruf, bevor
`schliessen()` beginnt, sind beide Zähler 0 — bei korrektem Verhalten. „Damit kann
`abgewiesen` gar nicht mehr steigen" gilt nur für die zweite Lage.

**c) Was bricht.** Matrixregel (Manifest `:590-593`, wörtlich unter D4); die Zusage
„nie beides 0" verbietet einen zulässigen Ausgang. R-D2
(`docs/beweise/NAK-246.md:1008-1016`) verlangt nur: kein Zugriff nach dem Schließen,
ein laufender Rückruf wird zu Ende gewartet.

**d) Urteil.** PRÄZISIERT (Hakenlage offen). **DEFEKT**. Kein Ausschluss: der
Zahlenrand „Frist genau 2 s" ist verlangt (Etappe-1-Auftrag `:32`), also kein
theoretischer Randfall ohne Zusage.

**e) Schließende Regel.** M-39 ordnet Freigabe, `stop()` und `schliessen()` über
beobachtbare Zustände statt über die Frist und lässt alle zulässigen Ausgänge zu;
verboten sind nur ein Zugriff nach dem Ende und ein Destruktor, der bei laufendem
Rückruf zurückkehrt (Teil 2, Punkt 6).

---

### L1 — Hostwerte, die während des Ladens eintreffen

**a) Beanstandeter Satz.** Manifest `:1004-1012`: „… ihr Zähler steigt über den
quittierten Stand hinaus, und sie wirken beim nächsten Takt. **Ehrlich benannt:** ein
Wert, der genau zwischen Quittierung und `hostParameterAbgleichen` eintrifft, ist vom
Abgleich nicht unterscheidbar (…) und wird mitquittiert."

**b) Nachmessung.** Quittierung geplant im Schloss `SondeProcessor.cpp:1067-1098`,
Abgleich danach `:1103`. Die Herkunftssperre ist thread-lokal: `:76`
`thread_local int abgleichTiefe = 0;` — sie unterdrückt nur Rückmeldungen auf dem
Abgleichthread (`:1424`, `:1333`). Ein Hostwert aus dem Prozessaufruf (Listener im
Audiothread, `SondeProcessor.h:539-540`; Wrapper `juce_audio_plugin_client_VST3.cpp:3756-3757`)
erhöht `hostEreignis` nach der Quittierung (`:1336`); der nächste Takt schreibt ihn
ins Overlay (`:1459-1462`). Er wird also **nicht** mitquittiert. Trifft er ein, bevor
der Abgleich denselben Parameter anfasst, setzt der Abgleich den Regler auf den
geladenen Wert zurück (`:1428-1430` `if (p->getValue() != soll) p->setValueNotifyingHost (soll);`),
die Mailbox behält den Hostwert: der Klang folgt dem Hostwert, der Regler zeigt den
geladenen — bis zum nächsten Hostereignis auf diesem Parameter. Der Prüfbericht hat
„mitquittiert" übernommen und nur die fehlende Zeile beanstandet.

**c) Was bricht.** Ein Satz des Bauplans widerspricht dem Code am ZIEL (DEFEKT-Klasse,
Prüfauftrag `:16`). Prüfauftrag `:29` verlangt zu prüfen, „ob die Ladegeneration von
W02 Werte, die vor, während und nach dem Laden eintreffen, je eindeutig behandelt".
Ein Regler, der einen anderen Wert zeigt als den hörbaren, fällt unter dieselbe
Zusage wie M-16 (Manifest `:647`: `CLAUDE.md:51-52` „sinngemäß für den Hostregler").
R-312-10 zweiter Teil (`:184-186`) regelt „vor" und „nach", nicht „während".

**d) Urteil.** PRÄZISIERT (der Bauplansatz ist falsch). Einordnung umgestuft auf
**DEFEKT**; welche Regel für „während" gilt, entscheidet der Dirigent. Kein
Ausschluss.

**e) Schließende Regel.** Für einen Hostwert nach der Quittierung gilt eine festgelegte
Regel mit messender Zeile — entweder er wirkt, und der Abgleich überschreibt keinen
Parameter, dessen Zähler seit der Quittierung gestiegen ist, oder er ist wirkungslos,
und die Quittierung wird nach dem Abgleich unter dem Schloss wiederholt —, und in
beiden Fällen zeigen Regler und Klang danach denselben Wert.

---

### L2 — der „heute rot"-Lauf von M-30 und M-31 ist undefiniertes Verhalten

**a) Beanstandeter Satz.** §7.1 (Manifest `:868-871`): „Für „heute rot" steht
zusätzlich der Lauf des neuen Tests gegen den unveränderten Basisstand in der
Rohdatei …"; M-30 (`:681`), M-31 (`:682`).

**b) Nachmessung.** `SondeProcessor.h:523` `controlV3` vor `:537` `transaktion` →
`transaktion` stirbt zuerst; `SondeProcessor.cpp:139-140` fängt
`[this] { return v3Status(); }`; `:1139` `const juce::ScopedLock l (zustandSchloss);`
(Mitglied `SondeProcessor.h:442`), `:1143` `transaktion->revision()`. Reproduktion
stimmt. Ergänzung: dasselbe gilt für den Rotbeweis **nach** dem Bau (M-30: „Mutation:
es entfernen → Marke zählt → rot"): ohne `schliessen()` läuft der freigegebene Rückruf
ebenfalls in `v3Status()` auf einem zerstörten Prozessor. UB-frei wird der Lauf, wenn
der Haken nach dem Zählen an der Marke den Rückruf dauerhaft anhält; der Thread ist
dann abgelöst und berührt nichts mehr. Das steht nirgends.

**c) Was bricht.** Kein Satz verbietet den Lauf. §6 verlangt deterministische Zeilen
(`:590`), §7.1 den Lauf gegen den Basisstand; wie ein Rotlauf mit UB geführt wird,
sagt keine Quelle.

**d) Urteil.** BESTÄTIGT (Ergänzung: auch der Mutationslauf). **LÜCKE**. Kein
Ausschluss.

**e) Schließende Regel.** Der Rotbeweis von M-30 und M-31 zählt den Zugriff an der
Marke, bevor der Rückruf Prozessorzustand berührt, und hält ihn danach an, sodass
weder der Lauf gegen den Basisstand noch die Mutation freigegebenen Speicher berührt.

---

### L3 — R-312-8 nennt „weg von Main", die Matrix fährt „zu Main"

**a) Beanstandeter Satz.** R-312-8 (Manifest `:166-170`): „Ein Rollenwechsel weg von
Main beendet eine aktive Hörmarkierung (Solo, Pulse) mit Intervallende, bevor die
Fläche wechselt; …". M-55 (`:726`), M-57 (`:728`).

**b) Nachmessung.** Zu Main (`sensor` → `hub`): M-55. Weg von Main mit hörbarem
Marker: keine Zeile — M-57 fährt den Rückwechsel nach M-55, also nach der dort
gebauten Rücknahme, und misst Knopf und Auftrag, nicht Ausgang und Intervall. Heute im
Code: weg von Main fällt `erlaubt` (`PluginProcessor.cpp:874`), `zielAn` wird false
(`HoerMarkierung.h:493-499`), der Ausfade läuft weich (`:420-429`, 80 ms `:569-570`),
`end` entsteht am Fadeende (`:626-641`) — nach dem Flächenwechsel, der mit dem nächsten
Tick kommt (`PluginEditor.cpp:230`). Der Auftrag bleibt (`ausGewuenscht` setzt nur ein
Aus-Auftrag, `HoerMarkierung.h:484-485`) und wird beim nächsten Wechsel zu Main ohne
Handgriff wieder scharf. Reproduktion stimmt. Präzisierung der Einordnung: die Quellen
schweigen nicht. Prüfauftrag `:16`: „ODER das je Gegenstand in §1 und der
Quellvalidierung genannte Gegenbeispiel (… Solo oder Pulse aktiv beim Rollenwechsel
weg von Main; …) ist von keiner Zeile mit deterministischem Rotbeweis getragen";
Etappe-1-Auftrag `:29`: „R-312-8: Solo oder Pulse aktiv, Rollenwechsel weg von Main:
Marker endet mit Intervallende vor dem Flächenwechsel; …".

**c) Was bricht.** Prüfauftrag `:16` (oben) und R-312-8 Satz 1.

**d) Urteil.** PRÄZISIERT (Pflichtgegenbeispiel). Einordnung umgestuft auf
**DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** R-312-8 gilt in beiden Richtungen (Fassung Teil 2, Punkt 4),
und je Richtung trägt eine Zeile mit deterministischem Rotbeweis Ausgang und
Interventionsring gemeinsam.

---

### H1 — kein `end` zweimal nach `reset()`

**a) Beanstandeter Satz.** §7.6 (Manifest `:1280-1284`): „Die Override ruft einen
neuen öffentlichen Einstieg der Markierung, der `hartAus()` (…) ausführt und den
fälligen Übergang wie `vorbereiten` (`:297-320`) zurückgibt — **ohne**
`wetKapazitaet`, `wet.calloc` (…) und `versuchTrocken.assign` (…)".

**b) Nachmessung.** `HoerMarkierung.h:300-304` bildet den Übergang aus `warHoerbar`,
`:313-314` `warHoerbar = false; hoerbareSamples = 0;`; `hartAus` (`:760-765`) setzt
nur `fade`, `pulsPos` und Filterzustände. Der Plan sagt nicht, ob der neue Einstieg
`warHoerbar` zurücksetzt. Reproduktion stimmt; der Schaden kommt früher als
beschrieben: bleibt `warHoerbar` stehen, bildet (i) schon der nächste Block ohne
Erlaubnis ein zweites `end` (`:501`, `:511-517`), und (ii) ein Block mit Erlaubnis
blendet ein, ohne `begin` zu melden (`:619` verlangt `! warHoerbar`) — gefärbtes Audio
ohne offenes Intervall.

**c) Was bricht.** Ausdrücklich keiner — „wie `vorbereiten`" schließt die Rücksetzung
naheliegend ein, sagt sie aber nicht. Getroffen würden R-312-5 (Manifest `:149-150`:
„… mit genau einem `end` je `begin`") und `PluginProcessor.cpp:978-979`: „Ein
verlorenes Begin darf niemals eine scheinbar saubere Baseline erzeugen (§34.2)."

**d) Urteil.** PRÄZISIERT (Schaden schon im nächsten Block, fehlendes `begin`).
**HÄRTUNG** (bleibt). Kein Ausschluss.

Randbeobachtung, nicht im Prüfbericht: M-59 (Manifest `:730`) setzt „laufendem
Transport" voraus und sagt zu, „der erste Block nach `reset()` ist bitgleich zum
Eingang"; M-63 (`:734`, „wie M-59") sagt, der Auftrag „beginnt beim nächsten erlaubten
Block neu". Mit laufendem Transport ist der erste Block nach `reset()` erlaubt,
`zielAn` wahr, und das Einblenden schreibt ab Sample 0 (`HoerMarkierung.h:588-596`):
die zwei Zusagen schließen einander in derselben Vorbedingung aus.

**e) Schließende Regel.** Der Abbruchweg setzt `warHoerbar` und `hoerbareSamples`
zurück wie `vorbereiten`; eine Zeile misst über `reset()`, Folgeblöcke mit und ohne
Erlaubnis und ein späteres `prepareToPlay`, dass jedes `begin` genau ein `end` hat und
kein gefärbter Block ohne `begin` liegt; M-59 nennt den Transportzustand nach
`reset()`.

---

### H2 — Hostsicht der Parameter nur am Konsolenprozessor gemessen

**a) Beanstandeter Satz.** §7.5 (Manifest `:1215-1225`), dreiteiliger Identitätsbeweis
„(a) … (b) … (c) … B1 misst zusätzlich das neu gebaute Bundle (Identität NAK-30)."

**b) Nachmessung.** `tools/beweise.ps1:691` B1: „Bundle-Identitaet (CIDs,
JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren." `SondeProcessor.cpp:86-117`: flache Liste,
`:94` `const juce::ParameterID id { b.id, 1 };`, `AudioParameterBool/Float/Choice`.
Die VST3-Parameter-ID entsteht aus der JUCE-String-ID:
`juce_audio_plugin_client_VST3.cpp:753-762` (`generateVSTParamIDForParam` →
`convertJuceParameterId`), weil `JUCE_FORCE_USE_LEGACY_PARAM_IDS` im Projekt nicht
gesetzt ist (`CMakeLists.txt` nennt nur `JUCE_VST3_CAN_REPLACE_VST2=0`). Gleiche
String-IDs ergeben gleiche VST3-IDs: plausibel neutral, am Bundle ungemessen.
Reproduktion stimmt.

**c) Was bricht.** Keiner. R-312-4 (Manifest `:141-145`) verlangt Bytegleichheit von
„Parameter-IDs, Reihenfolge, Bereiche, Hostsicht, Automation und State" — M-48 misst
das am Prozessor; `eq-copilot/identity/` führt keine Parameter-IDs (Grep).

**d) Urteil.** BESTÄTIGT. **HÄRTUNG**. Kein Ausschluss (nicht der Wrapperpatch).

**e) Schließende Regel.** Wird die VST3-Parametersicht zugesagt, misst ein Bein die
aus dem Bundle gelesenen VST3-Parameter-IDs und ihre Reihenfolge gegen ein Golden des
Basisstands.

---

### H3 — §7.4 nennt B7 nicht

**a) Beanstandeter Satz.** §7.4 (Manifest `:1167-1168`): „das neue
Sonde-Lebenslaufbein, danach B15 `EqCopShot` und B14
`EqCopSonde012ProjectReloadTest`." M-38 (`:689`): „B7 bestehend, Zyklus um laufende
v3-Clients ergänzt".

**b) Nachmessung.** Stimmt. Ergänzung: auch die präzisierten Ticketpfade der Etappe 4
(`:791-795`) nennen `tests/TransactionTestMain.cpp` nicht; der Kopf deckt sie mit
`eq-copilot/plugin/tests/**` (`:17`). Randbeobachtung, nicht im Prüfbericht: der
Rotbeweis von M-38 — „in `betreten()` eine Sperre nehmen → Zähler steigt im
Audiopfad, sobald ein Rückruf dort liefe → rot" — kann nicht fallen. `betreten()`
läuft in den Provider-Lambdas (`SondeProcessor.cpp:139-142`, Clientthreads), nie in
`processBlock`; die Wache zählt nur bei Tiefe größer 0 des eigenen Threads
(`dsp/DspRtWache.h:36-38`, `:50-53`). Eine Mutation, die fällt: einen Provider (etwa
`v3Status()`) aus `processBlock` rufen — seine Zeichenketten (`SondeProcessor.cpp:1145`,
`:1172`) allokieren im Bereich.

**c) Was bricht.** Keiner (voller Kanon am Etappenende; §7.0 `:829-831` ordnet
`SondeProcessor.cpp` B7 zu).

**d) Urteil.** BESTÄTIGT. **HÄRTUNG**. Kein Ausschluss.

**e) Schließende Regel.** Beinliste und Ticketpfade jeder Etappe nennen jedes Bein
und jede Testdatei, die eine Zeile dieser Etappe trägt, und jeder Rotbeweis ist eine
Mutation, die der Zähler der Zeile sieht.

---

## Teil 2 — Entscheidungsgrundlagen

### 1. Zu D1: Blockbindung der Bandwerte (R-312-10)

**Fakten.**
- Hostparameter 112 (`state/NakamaParameter.cpp:49-81`). Nichttopologisch 80: 69
  kontinuierliche (`gleitkomma`, `:25-33`) und 11 boolesche (`:49`, `:62` achtmal,
  `:79`, `:81`). Topologisch 32 (`:63`, `:67`, `:68`, `:74`).
- Über Rampenziele am Blockrand (`dsp/DspKern.cpp:620-626`): `input_trim_db`,
  `output_trim_db`, `width`, `v2.global.mix`. Ziel `autoGain` =
  `pn.autoGainAn ? pn.autoGainLin : 1.0` (`:626`), abgeleitet
  (`dsp/DspProgramm.cpp:366-369`).
- Nur über ein Programm: Bandwerte (`DspProgramm.cpp:295-355`), `mono_bass_hz`
  (`:277-279`; der Wechsel zwischen 0 und größer 0 ist Kerntopologie:
  `rampenKompatibel` `:225`, Pfadkennung `DspKern.cpp:328-330`), die elf Booleschen.
- Übernahme: ein Wechsel während eines laufenden Übergangs wartet
  (`DspKern.cpp:524-529`, E-17); `kRampeSamples` = 256 (`dsp/DspProgramm.h:91`);
  bankpflichtige Programme brauchen eine von vier Bänken (`dsp/DspBankPool.h:128`,
  `DspKern.cpp:381`).
- Workertakt: mindestens 5 ms (`SondeProcessor.cpp:575-578`); bei leerer Analysequeue
  schläft der Worker bis zu 20 ms (`:738-740`); `processBlock` weckt ihn nicht (Wecken
  nur `:225`, `:294`). Echter Takt also 5 bis 20 ms.
- Offlinegeschwindigkeit: „Das Offline-Szenario verarbeitet 3,00394 s Audio in 0,21441 s
  Wandzeit" (`docs/audits/2026-09-15-tiefenaudit/roh/phase-16-e-last-lebenslauf.md:41`)
  = 14,01-fach, gemessen im Konsolenprozess, nicht in FL.

**Weg (A) — der Callback entwirft im Rampenfall selbst.**
- Machbar ohne Sperre, Allokation und Warten: der Audiothread entwirft schon heute
  Koeffizienten, `DspKern.cpp:791` `z.svfNach = svfEntwurf (b.typ, grundG, q, gainDb + gDyn);`
  alle `kDynamikSchritt` = 8 Samples (`DspProgramm.h:236`); die Entwürfe sind
  `inline … noexcept` ohne Heap (`dsp/DspFilter.h:116-255`, `dsp/DspSvf.h:87-164`).
- Was `baueProgramm` liest, das nicht atomar vorliegt: den ganzen `DspSatz`
  (`DspProgramm.cpp:244-256`), den der Worker aus `transaktion->wirksam()` bildet
  (`state/NakamaTransaktion.cpp:340-348`: bestätigt plus Overlay, beide unter
  `zustandSchloss`) und in `publiziereWirksam` um `setzeFreieSlotsNeutral` ergänzt
  (`:766-776`, `:207-218`; liest `occupied`, kein Hostparameter); dazu `abtastrate`
  und `kanalzahl` (`DspKern.h:570-577`, nur in `bereiteVor`/`freigeben` geschrieben,
  während `processBlock` stabil). Atomar in der Mailbox liegen nur `hostWert` und
  `hostEreignis` (`SondeProcessor.h:541-542`). Die Umrechnung normiert → Zelle
  (`zelleAusHost`, `SondeProcessor.cpp:1394-1420`) liest den bestätigten Wert unter dem
  Schloss (`:1398`) für die Ausnahme „gleich in Hostgenauigkeit" (`:1403-1404`); ohne
  ihn weicht der Blockrandwert in den letzten Bits vom Workerwert ab. Typ, Modus,
  `occupied` und nicht automatisierte Werte kann der Callback aus dem Programm seiner
  aktiven Bank nehmen.
- Kosten, an der Quelle gezählt: statischer Entwurf je Band 1 `pow` (bell, shelves),
  1 `sin`, 1 `cos`, bei Shelves 1 `sqrt`, 5 Divisionen (`DspFilter.h:116-217`);
  SVF-Band zusätzlich 1 `tan` (`DspSvf.h:89`), 1 `pow`, höchstens 1 `sqrt`
  (`:102-143`); Detektor 1 `sin`, 1 `cos` (`DspFilter.h:224-238`), Hüllkurve 3 `exp`
  (`:306-326`). Schlechtester Fall rund 11 transzendente Aufrufe je Band, 8 Bänder rund
  90, dazu Mono-Bass 2 und Trims 2. Auto-Gain-Ableitung (`DspProgramm.cpp:85-215`):
  121 Stellen (`DspProgramm.h:241`) × (1 `pow` + je aktivem Band 1 `cos` und 1 `sin`
  oder 1 `tan` + eine komplexe Division) + 1 `log10` — bei 8 Bändern 121 × 17 = 2 057
  transzendente Aufrufe je Ableitung.
- Harte Grenzen:
  1. SONDE-015 M-39 (`docs/beweise/SONDE-015.md:1087`): „Der Ausgleich wird
     **außerhalb des Audiothreads** beim Bauen des Programms gerechnet …", gemessen über
     `RtWache::meldeAbleitung` (`DspRtWache.h:55-63`, `DspProgramm.cpp:89`). Mit
     `auto_gain` an bleibt das Auto-Gain-Ziel wanduhrgebunden, oder M-39 fällt.
  2. Poolvertrag (`DspBankPool.h:10-13`, wörtlich unter D3): der Callback darf in
     keine Poolbank schreiben und keine reservieren; er braucht je Pfad ein eigenes,
     vorallokiertes Programm außerhalb des Pools — eine neue Besitzregel im
     geschlossenen Automaten (`DspKern.h:48-83`).
  3. Sprünge über den Wertekriterien (`DspProgramm.h:150`, `:158`, `:168`) verlangen
     Überblendung und Kaltstart mit neuer Kennung; Kennungen vergibt nur der Worker
     (`DspKern.h:508-522`, `:599-604`). Sie bleiben beim Worker.
  4. Die Booleschen und `mono_bass_hz` über 0 sind Kerntopologie (Bank, Überblendung):
     sie bleiben beim Worker.
  5. Spätere Workerpublikationen dürfen Blockrandwerte nicht zurückrollen: die
     Schiedsregel aus Punkt 2 gilt dann auch für Bandwerte.
- Nur beim Worker, nicht anzufassen: Merkzettel und `kennungsZaehler`
  (`DspKern.h:599-604`), `vorbau` und `vorbauBankpflichtig` (`:593-597`), die
  Workerseite des Pools (`DspBankPool.h:154-199`), die Berichtsatomics
  `autoGainBericht`, `autoGainRohBericht`, `autoGainGedeckeltBericht`
  (`DspKern.h:624-632`: „vom selben Schreiber (`meldeProgramm`, Worker)"), Overlay und
  Epoche (`NakamaTransaktion.cpp:316-332`), `hostEreignisGesehen`,
  `samplesBeiLetzterAutomation`, `publikationOffen` (`SondeProcessor.h:543`,
  `:546-547`: „unter Zustandsschloss"), `DspKernAusfuehrung::zuletzt`
  (`NakamaTransaktion.cpp:773-774`).
- Gleichheit Echtzeit/offline: für den Rampenfall ja (der Callback entscheidet je
  Block); nicht für Auto-Gain an, Kriteriensprünge, Boolesche, `mono_bass_hz` über 0.
- Umfang: `dsp/DspKern.h/.cpp` (`blockrand`, Lesepfad in `verarbeiteBank` und
  `verarbeiteBand`, eigenes Programm je Pfad, Schiedsregel), `dsp/DspProgramm.h/.cpp`
  (Bandentwurf aus `baueProgramm :295-355` als eigene `noexcept`-Funktion),
  `sonde/SondeProcessor.h/.cpp` (Mailbox und Umrechnung vor `verarbeite`),
  `state/NakamaTransaktion.cpp` (Stand der Publikation); Beine B6, B7, A16.
- Risiko: Nulltest gering (Pfad nur bei neuem Ereignis und engagiertem Kern); Goldens
  mittel (jede Automationsprüfung ändert ihren Übernahmemoment; B6 führt
  „bitgleich zum Basisstand"-Aussagen, `tools/beweise.ps1:723`); M-47 machbar,
  mit dem Zähler zu belegen; M-81 unberührt (Overlay und Epoche bleiben beim Worker),
  `dspBericht` hinkt dem Klang bis zum nächsten Takt nach.
- Audit-Nacharbeitsticket: **nein** — neue Fähigkeit des Kerns mit eigener
  Besitzregel, eigener Matrix und eigener T2-Prüfung.

**Weg (B) — der Worker bleibt Erbauer, die Übernahme wird an Blöcke gebunden.**
- Übernommen wird höchstens ein Programm je 256 Samples (E-17), bei 48 kHz eines je
  5,33 ms Audio — offline bei 14,01-fach eines je 0,38 ms Wandzeit; der Worker kommt
  alle 5 bis 20 ms. Ein fester Versatz, der das deckt, bräuchte offline
  20 ms × 14,01 ≈ 280 ms Audio; derselbe Versatz verspätet in Echtzeit jede Automation
  um rund 280 ms und bricht Entwurf `:3069-3071`: „Eine zweite freie Glättung darf die
  Hostautomation nicht zeitlich verschieben. Realtime- und Offline-Render müssen
  denselben Parameterverlauf erzeugen." Eine Obergrenze für die Verspätung des Workers
  gibt es nicht; wecken darf der Callback ihn nicht (Etappe-1-Auftrag `:36`: „nur
  Atomics, keine Signalisierung").
- Kommt der Worker zu spät, fährt der Callback ohne Warten (E-312-2, Manifest
  `:1450-1452`) ein älteres Programm; das Ergebnis hängt an der Wanduhr. Gleichheit ist
  **nicht zusagbar**.
- Umfang: Workerschleife, Ring, markierte Publikation in `DspKern` und `DspBankPool` —
  mittel bis groß. Risiko: Echtzeitlatenz, Goldens durch Zeitversatz. Audit-Nacharbeit:
  nein; fachlich nicht empfohlen.

**Weg (C) — die Zusage wird auf das Erfüllbare begrenzt.**
- Machbar: Weg 2 für `input_trim_db`, `output_trim_db`, `width`, `v2.global.mix` mit
  der Schiedsregel aus Punkt 2; Auto-Gain bleibt im Programm (M-39 hält).
- Was im Export hörbar falsch bleibt: 64 Bandwerte, `mono_bass_hz`, die elf Booleschen
  und das Auto-Gain-Ziel werden nur beim Takt übernommen, und nur mit dem letzten
  Mailboxwert (`SondeProcessor.cpp:1459-1462`). Offline bei 14,01-fach und Takt 5 bis
  20 ms liegen zwischen zwei Übernahmen 70 bis 280 ms Audio (rund 3 360 bis 13 450
  Samples bei 48 kHz): 53 bis 210 Blöcke bei Blockgröße 64, 10 bis 40 bei der
  Messblockgröße 333 (die Angabe „rund jeder zehnte Wert" ist die 5-ms-Untergrenze).
  Jede Übernahme gleitet über 256 Samples (5,3 ms). Eine 1-s-Bandfahrt wird im Export
  zu rund 4 bis 14 Stufen, in der Wiedergabe zu 50 bis 200; bei schnellen Fahrten hörbar
  gestuft, der Endwert stimmt (M-23: „der Schaden ist der Weg, nie das Ziel"). Ein
  automatisiertes `eq_enabled` oder `bypass` schaltet im Export bis zu rund 280 ms Audio
  versetzt. Die Offlinezahl ist eine Konsolenmessung; FLs Faktor hängt am Projekt.
- „Rest als datierte Registerzeile zum Wrapperpatch" trifft die Ursache nicht: der
  Patch (Entwurf `:3073-3083`) liefert Sampleoffsets vor `processBlock`, er entwirft
  keine Bandkoeffizienten im Audiothread; auch der Rückfallsatz ohne Patch
  (`:3084-3086`) setzt Blockrandentwurf voraus. Die Registerzeile muss auf Weg (A)
  zeigen.
- M-120 (`SONDE-015.md:1224`) und Entwurf `:3070-3071` bleiben für Bandwerte gebrochen.
  Eine dauerhafte Verengung ist Produktwirkung — der Export klingt bei automatisierten
  Bandwerten anders als die Wiedergabe — und gehört als Karte vor den User. Eine
  befristete Restlücke mit Folgeticket ist eine datierte Registerzeile (`CLAUDE.md:69-70`:
  „Widersprüche werden als Zeile in `docs/offene-punkte.md` geführt, nie kaschiert.").
- Umfang: `SondeProcessor.cpp` (`processBlock`: vier Mailboxfelder, Umrechnung),
  `DspKern.h/.cpp` (Setzer der vier Ziele, Schiedsregel bei der Übernahme `:620-623`),
  Stand der Publikation. Risiko: Nulltest keines (ohne Ereignis läuft nichts, M-28),
  Goldens keines ohne Hostereignisse, M-47 nur Atomics, M-81 unberührt.
  Audit-Nacharbeit: **ja**.

**Empfehlung.** Weg (C) in Etappe 3, mit der Schiedsregel aus Punkt 2; R-312-10,
E-312-1 und M-20 bis M-29 werden auf die vier namentlich genannten Parameter
zugeschnitten, und eine Grenzzeile hält fest, dass Bandwerte, `mono_bass_hz`, die
Booleschen und das Auto-Gain-Ziel taktgebunden bleiben. Der Rest geht als datierte
Registerzeile an ein Folgeticket für Weg (A), nicht an den Wrapperpatch. Begründung:
(B) kann Gleichheit ohne Warten nicht zusagen; (A) ist der einzige Weg, der den
Rückfallsatz für Bandwerte erfüllt, verlangt aber eine neue Besitzregel im Kern und
stößt an SONDE-015 M-39 — das ist kein Nacharbeitsumfang. Damit schließt NAK-312
T3-01-05 nur zum Teil; ob der Gegenstand geteilt wird und ob der User zur befristeten
Exportabweichung gefragt wird, entscheidet der Dirigent.

Randbeobachtung, nicht im Prüfbericht: M-22 (Manifest `:662`) führt Blockgrößen ab 512
als „Regressionswache, heute schon bitgleich", M-27 (`:667`) 4096 ebenso. Mit bis zu
20 ms Takt (`SondeProcessor.cpp:738-740`) gehen bei 512 Samples (10,7 ms) auch heute
Werte verloren, und ein Bein ohne Echtzeittaktung läuft ohnehin schneller als der
Takt. Beide Etiketten beschreiben den heutigen Zustand nicht; nach dem Bau sind die
vier Parameter taktfrei, dann tragen die Zeilen.

### 2. Zu D2: überlaufsichere Schiedsregel

- Ein Ordnungsvergleich ist nicht nötig. Regel:
  1. Der Blockrand liest in **jedem** Block die Zähler der abgedeckten Parameter (vier
     `relaxed` Loads) und übernimmt einen Wert genau dann, wenn
     `hostEreignis[i] != blockrandStand[i]`; danach `blockrandStand[i] = n`. Gleichheit
     wie `SondeProcessor.cpp:1460` — überlaufsicher, M-19 bleibt wörtlich.
  2. Die Mailboxlesung steht im selben Callback **vor** der Programmübernahme
     (`DspKern.cpp:531`).
  3. Jede Publikation trägt je abgedecktem Parameter den Zählerstand, aus dem ihr Wert
     stammt: der Worker setzt ihn aus `hostEreignisGesehen` (`:1461`); Commit und
     Ladestart quittieren (Stand = aktueller Zähler).
  4. Bei der Übernahme setzt das Programm ein Rampenziel nur, wenn
     `publiziert[i] == blockrandStand[i]`; sonst bleibt das Blockrandziel. Weil der
     Blockrand vorher gelesen hat, ist eine ungleiche Publikation immer die ältere —
     eine Richtung braucht die Regel nicht. Ein gleicher Wert erzeugt keine neue Rampe
     (`DspKern.h:414` `if (z == ziel) return;`).
- Flag: `hostEreignisOffen` bleibt beim Worker (einziger Verbraucher, `:1455`); der
  Blockrand braucht es nicht.
- Zählerbreite: 32 Bit genügen, weil nur auf Gleichheit verglichen wird. Wer 64 Bit
  will, sichert es wie `DspKern.cpp:16-17` mit `static_assert (…::is_always_lock_free)`.
- Speicher je abgedecktem Parameter: im Kern ein `std::uint32_t` (Audiothread-eigen,
  nicht atomar); im Programm ein `std::uint32_t`, übergeben mit der Bank über dieselbe
  Release/Acquire-Übergabe wie die Kennungen (`DspKern.cpp:413-416`). Für vier
  Parameter 16 Byte im Kern und 16 Byte je Programm (vier Bänke `DspBankPool.h:301`,
  zwei Vorbauplätze `DspKern.h:596`). „Kein Speicherzuwachs" ist damit nicht wörtlich,
  aber klein.
- Zahlenrand: eine Zeile mit Testsetter dicht unter `0xFFFFFFFF`; Mutation „`!=` am
  Blockrand durch `>` ersetzen" → das erste Ereignis nach dem Überlauf fehlt → rot.
- Heute vorhanden: `hostEreignis` (`SondeProcessor.h:542`), `hostEreignisGesehen`
  (`:543`), `hostEreignisOffen` (`:544`), Gleichheitsvergleich
  (`SondeProcessor.cpp:1460`). Nicht vorhanden: Stand im Kern, Stand im Programm, Weg
  der Mailbox in den Kern (`NakamaKern` sieht `SondeProcessor` nicht; der Prozessor
  übergibt Werte und Stände vor `verarbeite`).
- Voraussetzung für die Bitgleichheit in M-20: der Blockrand rechnet dieselbe Zelle wie
  `zelleAusHost` (`:1394-1420`) einschließlich der Ausnahme `:1403-1404`; dafür braucht
  er den bestätigten Wert der vier Parameter, den es heute nur unter dem Schloss gibt
  (`:1398`).

### 3. Zu D3 und D4: Offline-Wechsel ohne Workerbesitz

- Was der Callback darf: `juce::AudioProcessor::setNonRealtime` (JUCE hält
  `nonRealtime` als `std::atomic<bool>`, `juce_AudioProcessor.h:1603`,
  `juce_AudioProcessor.cpp:410-412`, Bauartefakt), `setzeHoermatrix` (`DspKern.h:244`),
  ein neues Atomic „hart schalten" und ein Atomic „Vorschauende angefordert". Das harte
  Schalten verbraucht der Audiothread in `verarbeiteStueck` vor `DspKern.cpp:1233` und
  setzt `hoerVorher = hoerLaufend = wirksam; hoerFadeRest = 0` — Zustand, der ihm
  allein gehört (`DspKern.h:609-613`: „Nur der Audiothread liest und schreibt ihn").
  Nicht: `beendeCandidate` (`DspKern.h:157`, `:599-602`; `DspBankPool.h:37-39`, `:154`).
- Reicht das: für den Klang ja. Ab Sample 0 des ersten Offlineblocks ist
  `wirksam = processed` (`DspKern.cpp:1114-1116`) und der Ausgang `cAus` des
  Committed-Pfads (`:1263`, `:1297`). Der Candidate-Pfad rechnet weiter, ist aber
  nicht hörbar und geht nicht in die Analyse (`SondeProcessor.cpp:438`, Tap
  `postCommitted`). R-312-3 hält damit unabhängig davon, wann `beendePreview` läuft.
- Wer lässt die Bank wann los: der Worker im nächsten Takt — Atomic verbrauchen,
  `transaktion->beendePreview()` (`NakamaTransaktion.cpp:309-314`) → `beendeCandidate`
  unter dem Schloss wie heute → ENDE-Marke. Am nächsten Blockrand nimmt der Kern sie
  (`DspKern.cpp:537-557`); weil die Hörmatrix keinen Candidate mehr mischt
  (`:494-498`), blendet der Pfad über `kFadeSamples` in die Ruhe statt in den Hörhalt
  und dient per ACK aus (`:1040-1046`); der folgende Takt gibt die Bank frei (`pflege`,
  `SondeProcessor.cpp:1452`). Stand beim Schalten schon ein Hörhalt, beendet
  `beendeHoerHalt` ihn am Ende desselben Stücks (`DspKern.cpp:1389`, `:500-517`). Keine
  Bank bleibt dauerhaft hängen. Folge für M-46 (Manifest `:702`, nicht im Prüfbericht):
  ihr Rotbeweis „`beendeCandidate` aus dem Callbackpfad entfernen und nur die Hörmatrix
  hart schalten → die Bank bleibt gehalten" fällt nicht; die Freigabe hängt am Takt und
  ist mit `kontrollTaktFuerTest` deterministisch messbar.
- Offenes Fenster bis zum Takt: `candidateAktiv` steht noch; ein
  `setzeHoermatrix (candidate)` von außen (öffentlich, `SondeProcessor.h:224`) machte
  den Candidate offline hörbar (`DspKern.cpp:1113-1115`). Schließbar mit dem
  Offline-Atomic als Riegel: solange er steht, wirkt jeder Hörwunsch als Processed —
  gedeckt von Entwurf `:2748` („Preview, Focus und Delta sind im Offline-Render
  neutral"), gelöst von `setNonRealtime (false)`.
- B7 deterministisch (`tests/TransactionTestMain.cpp:2216-2234`): nach `:2227` einen
  Block fahren und sofort `gewuenschteHoermatrix() == processed` und Bitgleichheit ab
  Sample 0 prüfen (M-42, ohne die acht Blöcke `:2228`); danach `a->kontrollTaktFuerTest()`
  und erst dann `! a->previewAktiv()`. Vor dem ausdrücklichen Takt keine Aussage über
  `previewAktiv()`: der Worker läuft mit (`SondeProcessor.cpp:188-189`) und darf früher
  ziehen; `beendePreview` ist idempotent (`NakamaTransaktion.cpp:311`
  `if (! vorschau.aktiv) return;`), das Ergebnis nach dem ausdrücklichen Takt also fest.
  M-46 ebenso: nach dem Takt `ceil (kFadeSamples / Blockgröße) + 1` Blöcke, erneut
  Takt, dann den Bankzustand lesen.

### 4. Zu D6 und L3: Ort der Rücknahme, gefährliche Richtung, R-312-8

- Code: `src/PluginProcessor.cpp:874-881` (`erlaubt` = `istMainKlassifiziert` ∧
  Echtzeit ∧ `spielt` ∧ `aufnahmeAus` ∧ nicht offline ∧ Editor offen); `:980-1020`
  (`begin` und `end` aus `schritt`, Endzeit am letzten gefärbten Sample);
  `src/HoerMarkierung.h:420-429` („bei `!erlaubt` wird NICHT mehr geschnitten. …
  Statt dessen laeuft die vorhandene Rampe zu Ende"), `:431-450` (Oversize),
  `:493-499` (`zielAn`); `src/PluginEditor.cpp:225-285` (Timerstart `:230`,
  `timerCallback` `:242-258` mit Rücksprung `:257` vor den Netzen `:270-278`, Aus-Knopf
  `:279`), `:833-849` (Flächenwechsel, `markierungAusKnopf` in der Legacy-Liste `:843`,
  `setVisible` `:845-846`).
- Gefährlich ist **sensor → hub**. Legacy ist audioneutral (`State.cpp:272`,
  `NakamaLebenslauf.h:137`); ein dort eingereichter, nie hörbarer Auftrag wird hörbar,
  sobald `istMainKlassifiziert` synchron in `setzeBindung` kippt (`State.cpp:343-344`);
  nach dem nächsten Tick ist der Aus-Knopf verborgen (`PluginEditor.cpp:846`), und die
  drei Netze ticken nicht (`:257`). **hub → sensor** ist heute nicht gefährlich, aber
  unvollständig: `erlaubt` fällt, der Marker fadet weich aus und meldet sein `end` am
  Fadeende (`HoerMarkierung.h:626-641`), die Legacy-Fläche zeigt den Aus-Knopf wieder
  (`PluginEditor.cpp:279`) — der Auftrag bleibt aber scharf (`HoerMarkierung.h:484-491`)
  und wird beim nächsten Wechsel zu Main ohne Handgriff wieder hörbar.
- Ort ohne Fenster: im Klassifikationswechsel auf dem Nachrichtenthread, **vor** dem
  Store in `spiegleKlassifikation` (`State.cpp:262-273`): ändert sich
  `audioAusnahmeErlaubt()` gegenüber dem gespiegelten Wert, zuerst
  `markierung.reicheAus()` (`HoerMarkierung.h:411-414`; `PluginProcessor.h:1078`), dann
  der Store. `spiegleKlassifikation` deckt alle Wechselwege: `setzeBindung`
  (`State.cpp:344`) und beide Zweige von `setStateInformation` (`:192`, `:215`).
  Randbedingungen:
  1. `reicheEin` hat genau einen Publisher, den Nachrichtenthread
     (`HoerMarkierung.h:344`; `schreibIndex` nicht atomar). `State.cpp:346-348`: „…
     Rollenwahl und `setStateInformation` laufen heute beide auf dem Message-Thread".
  2. Der Audiothread liest `istMainKlassifiziert` heute `relaxed`
     (`PluginProcessor.cpp:874`). Damit ein Block, der das neue Flag sieht, sicher auch
     den Aus-Auftrag sieht (Briefkasten mit `acq_rel`, `HoerMarkierung.h:360-361`), muss
     dieses Lesen `acquire` werden.
  3. `State.cpp` fehlt in den präzisierten Ticketpfaden der Etappe 6 (Manifest
     `:798-806`), steht aber im Kopf (`:17`); §7.7 (`:1361-1364`) sagt, der
     Rollenwechsel werde dort „gelesen, nicht verändert" — das ist anzupassen.
  Im Editor räumt `wechsleFlaecheWennNoetig` dann nur seinen Anzeigezustand
  (`markModus`, Statustext); `markierungBeenden` (`:817-826`) ist idempotent.
- Reihenfolge und NAK-47: „Intervallende vor dem Flächenwechsel" ist mit weichem
  Ausfade (80 ms, `HoerMarkierung.h:569-570`) und Tick (33,3 ms) nicht herstellbar, ohne
  die Fläche zu verzögern oder hart zu schneiden; den Schnitt verbietet NAK-47
  (`:420-429`). Herstellbar ist: Rücknahme vor dem Flächenwechsel, Ausfade läuft weich
  zu Ende, `end` danach.
- **Vorschlag R-312-8, eine Regel für beide Richtungen:** „Jeder Wechsel der
  Klassifikation von Gen — zu Main und weg von Main, über `setzeBindung` oder
  `setStateInformation` — nimmt einen eingereichten Markierungsauftrag zurück, bevor
  die neue Klassifikation im Audiothread wirkt. Ein hörbarer Marker endet über den
  weichen Ausfade mit genau einem `end` zu seinem `begin`; ein nie hörbarer erzeugt kein
  Ereignis und färbt kein Sample; danach wird kein Marker ohne neuen Handgriff wieder
  wirksam. Die drei Netze (Kill, Sampleratenwache, Totmann) ticken in jeder Fläche; kein
  Handgriff wirkt, dessen Bedienelement nicht sichtbar ist. Keine neue Oberfläche."
- **Deterministische Szenarien** (ein Thread, kein Timer; Aufbau wie
  `tests/MarkierungTestMain.cpp:451-458`: Prozessor auf dem Heap, `prepareToPlay`,
  `testForciereEchtzeit (true)`, laufender Transport über die Brücke,
  `setzeEditorOffen (true)`):
  - S1, zu Main: Rolle `sensor`, `markierungEinreichen` über die Prozessor-API,
    40 Blöcke: bitgleich, Ring leer; `setzeBindung ("hub", …)`, sofort 40 Blöcke:
    bitgleich, kein `begin`, kein `end`. Rotbeweis: Rücknahme in
    `spiegleKlassifikation` entfernen → Einblenden ab dem ersten Block, ein `begin` →
    rot.
  - S2, weg von Main — hier entsteht ein `begin` wirklich: `setzeBindung ("hub", …)`,
    `markierungEinreichen`, Blöcke bis `markierungHoerbar()` (Muster
    `MarkierungTestMain.cpp:470-477`): genau ein `begin` im Ring; `setzeBindung ("sensor", …)`;
    Blöcke über die Fadelänge: genau ein `end` zu diesem `begin` mit der gezählten
    Hördauer; `setzeBindung ("hub", …)`, 40 Blöcke: bitgleich, kein neues `begin`.
    Rotbeweis: Rücknahme entfernen → nach dem zweiten Wechsel zu Main ein neues `begin`
    → rot. Die `end`-Hälfte ist heute schon grün (Regressionswache).
- Editorhälfte (Aus-Knopf, Netze in der Main-Fläche, M-56): ein deterministischer Tick
  braucht einen Testzugang, denn der Editor erbt `juce::Timer` privat
  (`src/PluginEditor.h:50-51`, `:71`); das Muster steht bei `PluginEditor.h:60-68`.
  `PluginEditor.h` steht weder im Kopf (Manifest `:17`) noch in §7.0 — ohne diesen
  Zugang hängen M-55 bis M-58 am Nachrichtenloop und damit an der Wanduhr.
- Randbeobachtungen, nicht im Prüfbericht: M-57 (Manifest `:728`) sagt zu, nach dem
  Rückwechsel sei „der Aus-Knopf … wieder sichtbar, trägt aber keinen scharfen
  Auftrag" — nach der Rücknahme ist `markModus` aus und `PluginEditor.cpp:279` blendet
  den Knopf aus; ein sichtbarer Aus-Knopf ohne Auftrag wäre selbst ein totes Element
  (`CLAUDE.md:51-52`). Produktwirkung: nach dieser Regel hat die Hörmarkierung im
  Produkt keinen hörbaren Weg mehr — in Legacy lehnt `schalteMarkierung` ab (M-58), und
  in der Main-Fläche fehlt der Auslöser (die Befundliste hängt am `hinweisKnopf`,
  `PluginEditor.cpp:162`, `:513`, und der steht in der Legacy-Liste `:842`). Das folgt
  aus dem geparkten Design, ist aber für den User sichtbar.

### 5. Zu D7: kleinste Regel für `aktualisiereSourcesSteuerung`

- Regel: „Die Aktionssteuerung bindet nur an ein Hauptziel innerhalb der gezeichneten
  Zeilen." `aktualisiereSourcesSteuerung` sucht `hauptziel` nur in
  `sourcesAnzeige.quellen[0 … sourcesZeilen().size())` — dieselbe Rechnung wie Zeichnen
  (`PluginEditor.cpp:1135`) und Klick (`:980`). Liegt es dahinter, gilt
  `hatZiel = false`: Knopf und Labelfeld unsichtbar (`:919`, `:933`),
  `sourcesAktionsZiel` leer, das Modell unverändert. Keine neue Oberfläche; ein Klick
  auf eine gezeichnete Zeile wählt ein neues Hauptziel (`:984`).
- Zusammenspiel mit R-312-9: verschwindet das Labelfeld mit offenem Entwurf, löst das
  einen Fokusverlust aus (`onFocusLost`, `:210`); die Entwurfsregel aus §7.6 (Manifest
  `:1293-1307`: bestätigen, angewandt auf die Startquelle) muss greifen, bevor
  `sourcesAktionsZiel` geleert ist — heute kehrt `uebernehmeSourcesLabel` bei leerem
  Ziel still zurück (`:876-877`).
- Nicht empfohlen: das Hauptziel automatisch auf die erste gezeichnete Zeile setzen —
  ein Modellwechsel ohne Handgriff (SONDE-012 U02, `docs/beweise/SONDE-012.md:231`:
  „Auswahl oder Wegfall wird als ein Modellwechsel sichtbar").
- Zeile: 25 Quellen, Hauptziel in Zeile 5; eine Quelle mit kleinerem Sortierschlüssel
  tritt bei oder Quellen davor werden umbenannt, bis das Ziel hinter Zeile 20 liegt;
  Tick; Zusage: Knopf und Labelfeld unsichtbar, Hauptziel unverändert, kein
  Schreibversuch. Rotbeweis: Suche wieder über die ganze Liste → Knopf sichtbar → rot.

### 6. Zu D8 und D9: M-04 und M-39

- **M-04.** Die Zeile misst ein Instrument, dem Etappe 5 absichtlich den Messgegenstand
  nimmt. Fassung mit zwei Hälften und Etappenwechsel: „Bis Etappe 4:
  `RtWache::sperren()` = 200 — jede Nahme von `zustandSchloss` in `setNonRealtime` ist
  über den Adapter gemeldet; ohne Offlinewechsel 0. Ab Etappe 5 (im selben
  Änderungssatz wie M-40 umgestellt): dieselbe Messung ergibt 0 und fällt mit M-40
  zusammen; die Instrumentaussage (der Bereich sieht eine Sperre in `setNonRealtime`)
  trägt ab dann der Rotbeweis von M-40 (Sperre im Wechselpfad wieder über den
  gemeldeten Adapter → 200 → rot)." §7.5 nennt die Umstellung unter den Änderungen der
  Etappe 5. Rot und grün in Etappe 2 wie geschrieben (Adapter durch `juce::ScopedLock`
  ersetzen → 0 → rot).
- **M-39.** Die Frist ist keine Verhaltensgrenze der Schleuse: `betreten` und
  `schliessen` hängen nicht an `kStopFristMs`, nur die Ablösung in `stop()`
  (`core/ipc/ControlClient.cpp:122-133`). Fassung: Hakenlage ausdrücklich, Freigabe an
  beobachtbaren Zuständen statt an der Zeit, und die Marke fällt am Ende des
  Destruktorrumpfs nach `schliessen()` — fiele sie an seinem Anfang, zählte ein
  zulässiger Zugriff während des Wartens als „nach dem Ende".
  - Fall 1: Haken hinter `betreten()`; Freigabe, sobald `stand().geschlossen` wahr ist
    (Schleusenzustand abfragen, keine Zeitmarke) → `gewartetMs` ≥ 1, `abgewiesen` 0,
    Marke 0.
  - Fall 2: Haken vor `betreten()`; Freigabe nach Rückkehr des Destruktors (Join des
    zerstörenden Threads) → `abgewiesen` ≥ 1, `gewartetMs` 0, Marke 0.
  - Fall 3: Freigabe vor Beginn der Zerstörung → Rückruf endet vorher → `abgewiesen` 0,
    `gewartetMs` 0, Marke 0 — zulässig.
  - Zulässig sind damit je nach Ordnung (`abgewiesen`, `gewartetMs`) = (0, 0), (≥ 1, 0),
    (0, ≥ 1); verboten sind Marke größer 0 und ein Destruktor, der zurückkehrt, während
    `stand().aktiv` größer 0 ist.
  - Rotbeweise: Fall 1 — `schliessen()` entfernen → der Destruktor kehrt zurück, der
    Rückruf läuft danach → Marke zählt; Fall 2 — dasselbe → der Rückruf wird nicht
    abgewiesen → Marke zählt (UB-frei nur mit dem anhaltenden Haken aus L2). Die
    bisherige Mutation „zweites Lesen in `betreten()` entfernen"
    (`Schleuse.h:131-136`) fällt nur in einem Fenster von wenigen Befehlen zwischen
    erstem Lesen und Erhöhen; deterministisch wäre sie erst mit einem Haken in
    `Schleuse.h`, das §7.4 „nicht angefasst" lässt (Manifest `:1170`).
  - Der Zahlenrand „Frist genau 2 s" ist durch Fall 1 und Fall 2 auf beiden Seiten der
    Frist gedeckt; den Augenblick selbst trifft kein Test ohne Uhrnaht in den Clients
    (`kStopFristMs` ist Konstante: `core/ipc/controlclient/Intern.h:59`,
    `core/ipc/TelemetryClient.cpp:30`).

---

## Übersicht

| Befund | Urteil | Einordnung | schließende Regel |
|---|---|---|---|
| D1 | BESTÄTIGT (80 statt 69 durchgelassene Parameter) | DEFEKT | Die blockgebundene Parametermenge steht namentlich und codegleich in Matrix und Bauplan, der Rest ist datiert ausgenommen, und ein Bandwert und ein boolescher Parameter haben je eine messende Zeile. |
| D2 | BESTÄTIGT (zweiter Verbraucher des Flags) | DEFEKT | Die Schiedsregel vergleicht auf Gleichheit, liest die Mailbox vor der Übernahme, lässt `hostEreignisOffen` beim Worker, nennt ihren Speicher und hat eine Zählerrand-Zeile. |
| D3 | PRÄZISIERT (zweite Besitzregel im Pool; „nur der Worker" ungenau) | DEFEKT | Der Callback berührt beim Offline-Wechsel nur eigene oder geteilte Atomics; Merkzettel, Publikation und Freigabe bleiben beim Worker. |
| D4 | PRÄZISIERT (Zeile widerspricht sich selbst; Takt 5 bis 20 ms) | DEFEKT | M-41 trennt die sofortige Hörmatrix von der Buchhaltung, und `previewAktiv()` wird nur nach `kontrollTaktFuerTest()` geprüft. |
| D5 | BESTÄTIGT | DEFEKT | Jede Behauptungszeile nennt den Bereich, den ihr Bein wirklich öffnet. |
| D6 | BESTÄTIGT | DEFEKT | Die Rücknahme sitzt synchron im Klassifikationswechsel, und M-55 sagt für „zu Main" kein Ereignis und Bitgleichheit ab dem ersten Block zu. |
| D7 | PRÄZISIERT (Umordnung meist durch Beitritt oder Hostname) | DEFEKT | Die Aktionssteuerung bindet nur an ein gezeichnetes Hauptziel, gemessen im Umordnungsfall. |
| D8 | BESTÄTIGT | DEFEKT | M-04 wird in Etappe 5 ausdrücklich umgestellt, die Instrumentaussage trägt der Rotbeweis von M-40. |
| D9 | PRÄZISIERT (Hakenlage offen) | DEFEKT | M-39 ordnet über beobachtbare Zustände und lässt alle zulässigen Ausgänge zu. |
| L1 | PRÄZISIERT (Bauplansatz „mitquittiert" falsch) | DEFEKT (umgestuft) | Für Hostwerte nach der Quittierung gilt eine festgelegte Regel, nach der Regler und Klang denselben Wert zeigen, mit messender Zeile. |
| L2 | BESTÄTIGT (auch der Mutationslauf) | LÜCKE | Der Rotbeweis zählt an der Marke und hält den Rückruf danach an, ohne freigegebenen Speicher zu berühren. |
| L3 | PRÄZISIERT (Pflichtgegenbeispiel des Prüfauftrags) | DEFEKT (umgestuft) | R-312-8 gilt in beiden Richtungen, je Richtung mit deterministischer Zeile über Ausgang und Ring. |
| H1 | PRÄZISIERT (zweites `end` schon im nächsten Block, fehlendes `begin`) | HÄRTUNG | Der Abbruchweg setzt `warHoerbar` zurück, und eine Zeile misst `begin` und `end` über `reset()`, Folgeblöcke und `prepareToPlay`. |
| H2 | BESTÄTIGT | HÄRTUNG | Wird die VST3-Parametersicht zugesagt, misst ein Bein die IDs am Bundle gegen ein Golden. |
| H3 | BESTÄTIGT (auch Ticketpfade der Etappe 4) | HÄRTUNG | Beinliste und Ticketpfade je Etappe nennen jedes Bein und jede Testdatei ihrer Zeilen, und jeder Rotbeweis ist für den Zähler sichtbar. |

Randbeobachtungen außerhalb des Prüfberichts, je oben an der Quelle belegt: zweiter
Verbraucher von `hostEreignisOffen` (D2); Satz „mitquittiert" falsch (L1); M-59 gegen
M-63 (H1); Rotbeweis von M-38 kann nicht fallen (H3); Etiketten von M-22 und M-27
(Teil 2.1); Rotbeweis von M-46 fällt nicht (Teil 2.3); Tick-Testzugang in
`PluginEditor.h` außerhalb der Ticketpfade, `State.cpp` fehlt in §7.0, Zusage von M-57,
Produktwirkung auf die Hörmarkierung (Teil 2.4); Fenstermutation von M-39 (Teil 2.6).

## Nicht geprüft

- Kein Bau, kein Testlauf, kein Kanon, kein Skript, kein FL Studio (Auftrag). Jedes
  Urteil ist Quellenlektüre am HEAD; keines ist durch einen Lauf belegt.
- Nur gelesen, nicht nachgemessen: die Messwerte der Phase 16 (−8,99999944679 dB,
  +6,00000017258 dB, 3,00394 s Audio in 0,21441 s). Die Zahlen in Teil 2.1, Weg (C),
  sind daraus und aus Konstanten der Quelle gerechnet. FLs Offlinegeschwindigkeit und
  die Taktlage im Host sind nicht gemessen.
- Transzendente Aufrufe sind an der Quelle gezählt, nicht in Zeit gemessen.
- JUCE-Quellen unter `eq-copilot/build/_deps/juce-src/`
  (`juce_audio_plugin_client_VST3.cpp`, `juce_AudioProcessor.h/.cpp`): Bauartefakt,
  nicht gegen den gepinnten Stand oder den Wrapperpatch verifiziert.
- Nicht gelesen: `BEFUNDE.md`, die Rohberichte der Phase 16 außer
  `phase-16-e-last-lebenslauf.md:33-41`, die Quellvalidierung Teil A außer `:893-906`
  und `:1308-1318`, Teil B außer §4 (`:774-1050`), `docs/offene-punkte.md`,
  `docs/plugin-wissen.md`, die Karten U48, U49, U51, U56, die Sicherheitsbefunde
  T3-12-*, `tools/dirigent/pruefliste.md`.
- Ob die Befunde im Host auftreten, ist nicht gemessen; alle Aussagen betreffen
  Quelltext und Sätze des Manifests.

FERTIG Validierung Matrixprüfung 1, 8 bestätigt, 7 präzisiert, 0 widerlegt.
