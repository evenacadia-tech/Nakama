URTEIL: NEEDS_WORK — neun DEFEKTE, drei LÜCKEN, drei HÄRTUNGEN.

# NAK-312 — Matrixprüfung 1 (lesender Opus-Thread, Effort max, 21.09.2026)

**Auftrag:** `docs/beweise/roh/NAK-312-matrixpruefung-1-auftrag.txt` (Dirigent,
21.09.2026), Prüfstufe T2.
**Prüfbereich:** `git diff 945ebfb1...2deb5e3d -- docs/beweise/NAK-312.md`
(§5 bis §8, 77 Matrixzeilen M-01 bis M-77), Gate-Bezug §0 bis §4 und §9.2.
**HEAD zu Beginn:** `b7d82137633a28e9ac42099b6568a0b44bd12b0f`.
**Arbeitsbaum zu Beginn:** `git status --short` zeigt nur die zwei fremden
untracked Ordner `briefing-hub/` und `nimbalyst-local/`.

**Eigene Messungen, lesend.** `git diff --stat 945ebfb1 2deb5e3d` = eine Datei,
`docs/beweise/NAK-312.md`, +1210/−2 (bestätigt). `git diff 2deb5e3d HEAD` berührt
nur `docs/beweise/NAK-312.md` (+45, ein Hunk ab Zeile 1426, also §9) und den
Prüfauftrag — §0 bis §8 sind am ZIEL und am HEAD identisch, der Code am ZIEL ist
der Code im Arbeitsbaum. `git diff --stat 090d0fa1 HEAD -- eq-copilot broker
tools` ist **leer** (gemessen): die Zeilenangaben beider Quellvalidierungen und
die von §5 bis §8 gelten unverändert. `git diff --stat 2d6519e8 HEAD --
eq-copilot broker tools` zeigt genau eine Zeile in
`eq-copilot/install/nakama-installer-v1.json` (kein Produktpfad) — die Angabe in
§7.0 stimmt. `git ls-files --eol docs/beweise/NAK-312.md` = LF im Index und in
der Arbeitskopie. Die Matrix hat **77** Zeilen M-01 bis M-77 ohne Lücke, jede mit
genau elf Trennern und ohne ungeschützten senkrechten Strich (gezählt); §6.8 und
die Selbstaussage in §8.1 stimmen.

---

## Defekte

### D1 — §7.3, Weg 2 (Empfehlung, mit E-312-1 gebaut): die Abdeckungsaussage bricht am Code

**Der Satz.** §7.3, Weg 2: „Der Kern liest am Blockrand … und setzt daraus die
Rampenziele derselben kontinuierlichen globalen Gains, die er heute aus dem
Programm setzt (`eq-copilot/plugin/dsp/DspKern.cpp:618-626` …). **Abgedeckt sind
genau die Parameter, für die das Overlay heute schon durchlässt**
(`eq-copilot/plugin/state/NakamaTransaktion.cpp:345`: `! topologisch`)."

**Reproduktion am ZIEL.** `NakamaTransaktion.cpp:345` lässt jeden Parameter
durch, dessen `topologisch` false ist. In `eq-copilot/plugin/state/NakamaParameter.cpp:26-32`
setzt **jedes** `gleitkomma` unbedingt `b.topologisch = false`. Die Tabelle
(`:49-80`) führt damit 69 kontinuierliche nichttopologische Hostparameter:
`input_trim_db`, `output_trim_db`, `width`, `mono_bass_hz`, `v2.global.mix` und
je Slot acht Bandwerte (`freq_hz`, `q`, `gain_db`, `dynamic_range_db`,
`threshold_db`, `attack_ms`, `hold_ms`, `release_ms`), also 64 Bandwerte. Der
Blockrand des Kerns setzt dagegen genau fünf Rampenziele
(`DspKern.cpp:620-626`: `input`, `output`, `mix`, `width`, `autoGain`). Alles
andere wirkt nur über ein **Programm**: `gain_db`, `freq_hz` und `q` gehen in die
Koeffizienten (`eq-copilot/plugin/dsp/DspProgramm.cpp:298-313`,
`entwurf`/`svfEntwurf`), `mono_bass_hz` in `msStufeAktiv` und den Hochpass
(`:262`, `:277-279`), und das Programm baut der Worker unter dem Zustandsschloss
(`SondeProcessor.cpp:1476-1477`). Der Blockrand kann sie nicht setzen.

**Was bricht.** R-312-10 (§2): „Die Übernahme von Hostparameterwerten ist an den
Audioblock gebunden, nicht an die Wanduhr: derselbe Event- und Blockverlauf
liefert offline und in Echtzeit dieselbe Ausgabe innerhalb der Toleranz von
M-120." Und E-312-1 (§9.2): „nur für die nichttopologischen kontinuierlichen
Hostparameter". Nach Etappe 3 bleibt die Übernahme für 65 der 69 kontinuierlichen
Hostparameter wanduhrgebunden; M-120 (`docs/beweise/SONDE-015.md:1224`) bleibt für
Bandautomation gebrochen, und der Entwurfssatz „vom vorigen zum letzten
Blockwert" (`docs/FL-Nakama-Sonden-Design-Entwurf.md:3084-3086`) ist nicht
erfüllt. Keine Matrixzeile fällt darüber: M-20 bis M-29 fahren ausschließlich
`v1.global.output_trim_db` (M-20, Ereignisspalte; M-22, M-23, M-27, M-28 sind
Teilfälle von M-20). Der bestehende B7-Fall zeigt, dass der Harnisch
Bandparameter kennt (`eq-copilot/plugin/tests/TransactionTestMain.cpp:2050-2053`,
`hostSchreibt` auf `kGainDb`, `kEnabled`, `kType`, `kFreqHz`) — die Matrix nutzt
das nicht.

### D2 — §7.3, Weg 2 gegen M-19: Ordnungsvergleich auf einem Zähler, dessen Ordnungsvergleich M-19 rot macht

**Der Satz.** §7.3, Weg 2: „eine Publikation setzt ein Rampenziel nur, wenn ihr
Ereigniszählerstand **nicht älter ist** als der zuletzt am Blockrand gesehene;
verglichen wird derselbe monotone Zähler wie in `:1460`."

**Reproduktion am ZIEL.** Der Zähler bei `SondeProcessor.cpp:1460` ist
`hostEreignis[i]`, deklariert als `std::array<std::atomic<std::uint32_t>, …>`
(`eq-copilot/plugin/sonde/SondeProcessor.h:542`); er läuft über. „Nicht älter
als" ist ein Ordnungsvergleich.

**Was bricht.** M-19 sagt wörtlich zu: „Die Quittierung vergleicht auf
**Gleichheit, nicht auf Ordnung**: nach dem Überlauf wirkt das nächste Ereignis,
und kein Ereignis wird doppelt verbraucht", und ihr Rotbeweis macht genau den
Ordnungsvergleich zur brechenden Mutation („`==` bei `:1460` durch `<` ersetzen →
das erste Ereignis nach dem Überlauf verschwindet → rot"). Der Bauplan legt damit
auf denselben Zähler einen Vergleich, den die Matrix an derselben Zeile für rot
erklärt. Welche der beiden Aussagen gilt — und ob der Schiedsrichter eine eigene,
überlaufsichere Differenzbildung bekommt —, ist nicht entschieden; keine Zeile
misst den Zahlenrand für die Schiedsrichterseite (M-19 deckt nur die
Quittierung). Gleichzeitig trägt Weg 2 die Kostenaussage „Kein Speicherzuwachs",
obwohl der Schiedsrichter je Parameter einen „zuletzt am Blockrand gesehen"-Stand
im Kern und den Zählerstand in der Publikation braucht.

### D3 — §7.5, Schritt 3: `beendeCandidate` aus dem Callback verletzt eine im Quelltext festgeschriebene Besitzregel

**Der Satz.** §7.5, Schritt 3: „Die klangwirksame Hälfte läuft sofort im
Callback: `beendeCandidate` (`DspKern.cpp:434-443`) **arbeitet nur mit Atomics und
einer Publikation** und lässt die Candidate-Bank los."

**Reproduktion am ZIEL.** `DspKern.cpp:434-443` enthält bei `:441`
`merkzettel[(size_t) Pfad::candidate].gueltig = false;`. `merkzettel` ist
`std::array<Merkzettel, kPfade>` (`eq-copilot/plugin/dsp/DspKern.h:603`), und der
Kommentar unmittelbar darüber (`:599-602`) sagt: „NAK-311 W03: Merkzettel und
Zaehler gehoeren **ALLEIN dem Worker** - kein Atomic, keine Sperre, nichts davon
liest der Audiothread." Geschrieben wird der Merkzettel außerdem in `merke`
(`:288-…`) und in der ENDE-Marke (`:395`), beides Workerwege; gelesen wird er bei
`:324` für die Kennungsvergabe. Heute ruft nur der Worker `beendeCandidate`
(`eq-copilot/plugin/state/NakamaTransaktion.cpp:763`).

**Was bricht.** Der Callbackaufruf erzeugt ein Datenrennen auf nichtatomarem
Zustand und nimmt der Kennungsvergabe (NAK-311 W03, SONDE-015 M-07/E-8) die
Entscheidung, ob der nächste Candidate kalt startet. Das bricht die Besitzregel
in `DspKern.h:599-602` und die Zusage von M-38 und M-26 nur scheinbar nicht: ein
Rennen ist weder Sperre noch Allokation und fällt keinem der beiden Zähler auf.
Der Satz widerspricht dem gemessenen Code; Teil A hat ihn mit derselben
Formulierung vorgelegt (`docs/beweise/roh/NAK-312-quellvalidierung-teil-A.md:900-901`),
der Bauplan hat ihn übernommen, ohne die Merkzettelzeile zu lesen.

### D4 — M-41 ist als Regressionswache geführt, obwohl §7.5 Schritt 4 genau diese Eigenschaft verschiebt

**Die Zeile.** M-41, Zusage: „Er beendet die Vorschau: `previewAktiv()` ist
danach false und `gewuenschteHoermatrix()` ist Processed, **spätestens nach dem
ersten Block**", Etikett „**Regressionswache** (hält heute)".

**Reproduktion am ZIEL.** §7.5, Schritt 4: „`transaktion->beendePreview()`
braucht den Transaktionskern und gehört nicht in den Callback: ein zweites Atomic
‚Vorschauende angefordert' wird gesetzt, und `dspKontrollTakt` (`:1447-1478`)
verbraucht es unter dem Schloss, das es ohnehin hält." `previewAktiv()` liest
`transaktion->preview().aktiv` unter dem Zustandsschloss
(`SondeProcessor.cpp:1309-1313`). Nach dem Bau ist der Wert also erst nach dem
nächsten Kontrolltakt false, und der kommt vom Worker an der Wanduhr
(`:575-578`, 5 ms) — ein Block löst ihn nicht aus. Der bestehende B7-Fall, der
diese Zusage heute trägt, prüft sie bei
`eq-copilot/plugin/tests/TransactionTestMain.cpp:2231` (`! a->previewAktiv()`),
und `fahreAudio` (`:1751-1768`) ruft `kontrollTaktFuerTest` nicht. Die 108 Blöcke
zwischen `setNonRealtime (true)` (`:2227`) und der Prüfung (`:2231`) rechnen
deutlich unter 5 ms.

**Was bricht.** Die Etikettendefinition in §6: „**Regressionswache**: heute grün
und hält, was der Bau **nicht ändern darf**." Der Bau ändert es. Und der
bestehende, heute grüne Fall `offline_render_nutzt_den_bestaetigten_zustand` wird
am Ende der Etappe 5 zu einem Wanduhrrennen gegen den Workertakt — genau das, was
die Matrixregel „Keine Zeile hängt an Wandzeit oder Zufall" (§6) ausschließt. Die
Matrix sagt nicht, dass die Prüfstelle `:2231` in Etappe 5 auf den Kontrolltakt
umgestellt wird.

### D5 — §7.2: B6 bekäme eine Behauptung über einen Prozessor, den B6 nicht baut

**Der Satz.** §7.2: „**Wie die Behauptungszeilen lauten werden.** B6
(`tools/beweise.ps1:723`) und B7 (`:728`) bekommen je einen Satz: der Zähler misst
den Bereich **ab dem Eintritt in `SondeProcessor::processBlock`**, nicht erst ab
`DspKern::verarbeite` …"

**Reproduktion am ZIEL.** B6 ist `EqCopDspGoldenTest`; sein Ziel übersetzt genau
eine Quelle: `eq-copilot/plugin/CMakeLists.txt:1148-1154` —
`target_sources(EqCopDspGoldenTest PRIVATE tests/DspGoldenTestMain.cpp)` plus
`nakama_kern_anbinden`. Weder `sonde/SondeProcessor.cpp` noch
`NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` kommen darin vor. B6 kann keinen
`SondeProcessor` anlegen; sein Zähler misst den Bereich in
`DspKern::verarbeite` (`eq-copilot/plugin/dsp/DspKern.cpp:1059` — die einzige
`RtWache::Bereich`-Stelle im Produkt, repoweit gemessen).

**Was bricht.** R-312-1 (§2): „Die Behauptungszeilen der Beine, die sich auf die
Wache stützen, **sagen genau, was der Zähler sieht**." Der geplante Satz sagt für
B6 mehr, als B6 misst — dieselbe V0-Klasse, die R-312-1 beseitigen soll, nur an
einer neuen Stelle. Die Matrix ist hier richtiger als der Bauplan: M-01, M-02 und
M-04 liegen sämtlich in B7.

### D6 — M-55: die zugesagte `end`-Hälfte kann im eigenen Szenario nicht entstehen, und die Platzierung des Fixes lässt ein Fenster offen

**Die Zeile.** M-55, Zustand: „Gen in Legacy-Rolle (`sensor`), Marker SOLO oder
PULS eingereicht"; Ereignis: „`setzeBindung ("hub", …)`, danach Blöcke fahren";
Zusage: „Der Marker ist beendet, bevor die Fläche wechselt: der Ausgang bleibt
über 512 Samples **nach dem Rollenwechsel** bitgleich zum Eingang, und im
Interventionsring steht **genau EIN `end` zu dem offenen `begin`**."

**Reproduktion am ZIEL, Hälfte 1.** In der Legacy-Rolle ist
`istMainKlassifiziert` false (`eq-copilot/plugin/src/prozessor/State.cpp:272`),
damit `erlaubt` false (`eq-copilot/plugin/src/PluginProcessor.cpp:874-881`), damit
`zielAn` false (`eq-copilot/plugin/src/HoerMarkierung.h:493-499`). Der Marker wird
nie hörbar, `warHoerbar` bleibt false, und der Sender bildet kein `begin`
(`PluginProcessor.cpp:980-1020`). Ein „offenes `begin`", zu dem genau ein `end`
stehen könnte, gibt es in diesem Szenario nicht. Die beiden Hälften der Zusage
schließen einander aus: entweder färbt nichts (dann fehlt das Paar) oder es
entsteht ein Paar (dann ist der Ausgang nicht bitgleich).

**Reproduktion am ZIEL, Hälfte 2.** §7.6 setzt `markierungBeenden` in
`wechsleFlaecheWennNoetig` (`eq-copilot/plugin/src/PluginEditor.cpp:833-849`), und
diese Funktion läuft aus `timerCallback` (`:244`) mit `startTimerHz (30)`
(`:230`). `setzeBindung` schärft die Audioseite dagegen **synchron** auf dem
Nachrichtenthread (`State.cpp:272`, aus `setzeBindung` über
`spiegleKlassifikation`, `:344`). Zwischen dem Rollenwechsel und dem nächsten
Timertick liegen bis zu rund 33 ms, in denen der Marker scharf ist und färbt. Ein
Bein, das nach `setzeBindung` sofort Blöcke fährt, ist damit auch nach dem Bau
rot; ein Bein, das erst einen Timertick fährt, misst das Produktfenster nicht.

**Was bricht.** Die Zusage der Zeile gegen den gemessenen Code, und der Satz in
§7.6 „Das Intervallende entsteht dabei im Audiothread über den regulären Ausfade
… Erst danach wechselt die Fläche" — der Flächenwechsel (`:845-846`) läuft im
selben Funktionsaufruf, das Intervallende erst über die folgenden Audioblöcke.

### D7 — M-74 verlangt eine Änderung, die §7.6 nicht baut

**Die Zeile.** M-74, Reihenfolge und Frist: „`aktualisiereSourcesSteuerung`
(`eq-copilot/plugin/src/PluginEditor.cpp:902-944`) sucht das Ziel **in derselben
Menge, die `:1135` zeichnet**"; Zusage: „Kein sichtbarer, scharfer Knopf steht auf
einer ungezeichneten Quelle."

**Reproduktion am ZIEL.** §7.6 baut ausschließlich die Ordnung im Modell:
„`stelleZielSicher` (`eq-copilot/plugin/src/SourcesModel.cpp:1772-1777`) wählt
seinen Ersatz nach derselben Ordnung, in der die Fläche zeichnet
(`:1651-1663`)". `aktualisiereSourcesSteuerung` sucht das Hauptziel weiterhin über
die ganze Liste (`PluginEditor.cpp:905-907`) und schaltet danach
`sourcesAktionKnopf` (`:919`) und `sourcesLabelFeld` (`:933`) sichtbar.
`stelleZielSicher` greift nur, wenn das Hauptziel **fehlt** (`:1774-1775`). Ändert
sich die Anzeigeordnung, während das Hauptziel existiert, fällt es wieder hinter
die gezeichneten Zeilen: der Vergleich in `SourcesModel.cpp:1651-1663` ordnet nach
Mixerindex, dann sichtbarem Namen, dann `instanceId` — und der sichtbare Name ist
über genau das Labelfeld änderbar, das T3-14-01 behandelt
(`PluginEditor.cpp:874-900`). Bei mehr als 20 Quellen zeichnet `sourcesZeilen()`
genau 20 Zeilen (`:951-967`, unterer Anschlag 18 px bei `:958-959`).

**Was bricht.** R-312-6 (§2): „Ersatz-Hauptziel **und jede Aktionssteuerung**
wählen nur aus der gezeichneten Menge, in Anzeigeordnung; kein scharfer Knopf
steht auf einer ungezeichneten Quelle." Die zweite Hälfte ist weder gebaut noch
von einer Zeile gemessen; M-73 bis M-76 decken nur den Ersatzfall.

### D8 — M-04 und M-40 behaupten für dieselbe Messung gegensätzliche Zählerstände

**Die Zeilen.** M-04 (Etappe 2), Zusage: „**`RtWache::sperren()` = 200**: jede
Nahme von `zustandSchloss` in `setNonRealtime` ist eine gemeldete Sperre"; Aufbau:
„Bein öffnet den Bereich wie der Wrapper um das Paar `setNonRealtime(true)` und
`processBlock`", Ereignis „200 Blöcke, vor jedem `setNonRealtime (true)`".
M-40 (Etappe 5), derselbe Aufbau („das Bein öffnet den Bereich wie der Wrapper
(M-04)"), dasselbe Ereignis, Zusage: „**`RtWache::sperren()` = 0** über die 200
Blöcke".

**Reproduktion am ZIEL.** Beide Fälle liegen in B7 und bleiben dauerhaft im
Kanon. §7.5, Schritt 1 entfernt die Sperre für den Normalfall („Ist er gleich,
kehrt die Funktion sofort zurück"), Schritt 4 nimmt auch dem Wechselfall den
Griff zum Zustandsschloss. Nach Etappe 5 zählt derselbe Lauf 0 statt 200; M-04
wird damit rot, ohne dass ein Produktfehler vorliegt.

**Was bricht.** Der Auftragspunkt „zwei Etappen berühren dieselbe Funktion mit
widersprüchlichen Änderungen (insbesondere `setNonRealtime` zwischen Etappe 2 und
5)". Weder §6 noch §7.5 sagt, dass `312/M-04` in Etappe 5 zurückgenommen oder auf
„höchstens eine gemeldete Sperre je Wechsel" umgestellt wird; §7.5 nennt unter
„Nicht angefasst" auch keine Ausnahme dafür.

### D9 — M-39 hängt an einer Wanduhrkoinzidenz und verbietet einen zulässigen Ausgang

**Die Zeile.** M-39, Ereignis: „die Schranke wird **exakt bei `kStopFristMs`**
freigegeben"; Zusage: „entweder hat der Destruktor gewartet (`gewartetMs` ≥ 1)
oder der Rückruf wurde abgewiesen (`abgewiesen` ≥ 1) — **nie beides 0**, und in
keinem Fall ein Zugriff nach dem Ende".

**Reproduktion am ZIEL.** Der Rückruf wird nach M-30 **innerhalb** der Schleuse
gehalten (Haken „VOR dem ersten Besitzerzugriff"), also nach `betreten()`
(`eq-copilot/plugin/core/ipc/controlclient/Schleuse.h:121-139`). Damit kann
`abgewiesen` gar nicht mehr steigen. Bleibt `gewartetMs`: `schliessen()` zählt es
nur, „wenn ueberhaupt gewartet wurde" (`Schleuse.h:141-145`). Läuft der bei
`kStopFristMs` freigegebene Rückruf zu Ende, bevor der Destruktor nach
`telemetryV3.stop()`/`controlV3.stop()` (`SondeProcessor.cpp:228-229`)
`schliessen()` erreicht, sind beide Zähler 0 — bei vollständig korrektem
Verhalten und ohne jeden Zugriff nach dem Ende.

**Was bricht.** Die Matrixregel in §6: „Keine Zeile hängt an Wandzeit oder
Zufall: Ladestart, Blockrand, Kontrolltakt und Destruktorfolge sind
deterministisch ansteuerbar." Das Ereignis dieser Zeile ist eine
Wanduhrkoinzidenz, und die Zusage erklärt einen zulässigen Ausgang für rot.

---

## Lücken

**L1 — Hostwerte, die *während* des Ladens eintreffen, sind in Prosa entschieden,
aber von keiner Zeile getragen.** §7.3 benennt den Fall ehrlich: „ein Wert, der
genau zwischen Quittierung und `hostParameterAbgleichen` eintrifft, ist vom
Abgleich nicht unterscheidbar … und wird mitquittiert" (Fenster:
`SondeProcessor.cpp:1067-1098` bis `:1103`). M-10 misst „vor dem Ladestart",
M-11 „nach dem Recall"; für „während" gibt es keine Zeile, keinen Rotbeweis und
keine Aussage, was der User sieht (der Regler springt auf den geladenen Wert
zurück). Der Auftrag verlangt die Prüfung aller drei Fälle. Benennen, keine
Nacharbeit.

**L2 — der „heute rot"-Lauf von M-30 und M-31 ist ein Lauf in undefiniertem
Verhalten, und die Matrix entscheidet ihn nicht.** §7.1 verlangt: „Für ‚heute
rot' steht zusätzlich der Lauf des neuen Tests gegen den unveränderten
Basisstand in der Rohdatei". Für M-30 und M-31 ist dieser Lauf ein echter
Use-after-free: `transaktion` (`SondeProcessor.h:537`) stirbt vor `controlV3`
(`:523`), und der gehaltene Provider führt danach `transaktion->revision()`
(`SondeProcessor.cpp:1143`) aus. M-31 verlangt zwanzig solcher Zyklen. Ob der
Beleg stattdessen die Quellkette bleibt (wie in der Rotbeweisspalte von M-30
bereits geschrieben) oder ob der Lauf bewusst gefahren wird, ist nicht
entschieden.

**L3 — R-312-8 nennt die Gegenrichtung des Befundes, und die fehlende Richtung
hat keine Zeile.** R-312-8 (§2) lautet: „Ein Rollenwechsel **weg von Main**
beendet eine aktive Hörmarkierung (Solo, Pulse) mit Intervallende, bevor die
Fläche wechselt." Der Befund T3-07-02 und die Matrix behandeln die Gegenrichtung:
M-55 fährt `sensor → hub`, M-57 den Rückwechsel als Knopf- und Auftragszustand.
Eine Zeile, die einen **hörbaren** Marker beim Wechsel `hub → sensor` misst
(heute beendet ihn der weiche Ausfade über `erlaubt`, `HoerMarkierung.h:420-429`),
gibt es nicht. Ob R-312-8 umformuliert oder die Matrix ergänzt wird, entscheidet
der Dirigent.

---

## Härtungen

**H1 — kein `end` zweimal.** §7.6 lässt `reset()` den fälligen Übergang „wie
`vorbereiten` (`:297-320`)" zurückgeben; dort wird `warHoerbar` zurückgesetzt
(`HoerMarkierung.h:313-315`). Bleibt das aus, bildet ein späteres
`prepareToPlay` ein zweites `end` zum selben `begin`. M-61 fährt bereits
`reset()` → Blöcke → `prepareToPlay` und könnte die Aussage „kein zweites
Ereignis" mittragen; heute prüft sie dort nur den Oversize-Riegel.

**H2 — die Hostsicht der Parameter wird nur am Konsolenprozessor gemessen.** Der
dreiteilige Identitätsbeweis aus §7.5 (a, b, c) läuft am Prozessor ohne
VST3-Wrapper; B1 `EqCopIdentityTest` (`tools/beweise.ps1:691`) behauptet nur
„Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren". Die
numerischen VST3-Parameter-IDs und der Parameterbaum, den FL für seine
Automationsspuren speichert, werden von keinem Bein am gebauten Bundle gemessen.
Der Umbau ist plausibel neutral — `baueParameterLayout()`
(`eq-copilot/plugin/sonde/SondeProcessor.cpp:86-117`) legt eine **flache** Liste
mit `juce::ParameterID { b.id, 1 }` und `AudioParameterBool/Float/Choice` an, also
genau das, was auch ohne APVTS entsteht —, aber gemessen ist er dort nicht.

**H3 — §7.4 nennt B7 nicht.** M-38 (Etappe 4) liegt in „B7 bestehend, Zyklus um
laufende v3-Clients ergänzt"; die Liste „Beine, die einzeln zu fahren sind" in
§7.4 nennt nur das neue Lebenslaufbein, B15 und B14. Über §7.0 („`plugin/sonde/`
geht in A16, B7, die Sonde012-Ziele und das Bundle") und den vollen Kanon am
Etappenende ist es gedeckt.

---

## Geprüft

Ich habe den Diff im Prüfbereich vollständig gelesen (§5 bis §8) sowie §0 bis §4
und §9 als Gate-Bezug; dazu vollständig
`docs/beweise/roh/NAK-312-quellvalidierung-teil-A.md` und `-teil-B.md`,
`docs/beweise/roh/NAK-312-etappe-1-auftrag.txt` (§6 und §7 als Anforderungsliste)
und die Rohberichte `roh/phase-16-c-persistenz.md` und
`roh/phase-16-e-last-lebenslauf.md`. **Am ZIEL selbst nachgemessen** (Lesen und
Zählen an der Quelle, kein Bau, kein Lauf): den Gate-Text in
`docs/plan/plan.json:361` gegen den Kopf des Manifests; jede in §5 zitierte
Zusage im Wortlaut an ihrer Stelle (`CLAUDE.md:29-33`, `:51-52`, `:84-86`,
`:198-200`, `:201-203`, `:204-205`; SONDE-015 `:1100`, `:1114`, `:1159`, `:1162`,
`:1175`, `:1224`, `:3400`, `:3425`; SONDE-013 `:521`; SONDE-012 `:231`, `:232`;
NAK-246 `:1008-1016`; Entwurf `:1590`, `:2137`, `:2748`, `:3066-3067`,
`:3070-3071`, `:3082-3083`, `:3084-3086`, `:4277`; Statevertrag `:104`,
`:107-108`; `DspRtWache.h:9-15`; `Schleuse.h:17-23`, `:30-33`;
`PluginProcessor.h:95-99`; die Behauptungszeilen A3 `:509`, A16 `:688`, B6 `:723`,
B7 `:728`, B13 `:744`, B14 `:745`, B15 `:746`); die Codestellen aus §5 bis §7 in
`SondeProcessor.cpp` (`:139-142`, `:151-154`, `:157-164`, `:191-200`, `:212-230`,
`:284`, `:299-343`, `:369-495`, `:432`, `:466`, `:475`, `:487`, `:493-494`,
`:569-579`, `:739`, `:1024-1028`, `:1051-1106`, `:1143`, `:1181-1183`,
`:1321-1338`, `:1381-1391`, `:1394-1420`, `:1422-1432`, `:1447-1478`) und
`SondeProcessor.h` (`:158-159`, `:176-179`, `:254-256`, `:263`, `:361`, `:442`,
`:523-524`, `:533-537`, `:541-547`); `NakamaTransaktion.cpp:255-256`, `:265-268`,
`:316-324`, `:340-348` und `NakamaTransaktion.h:62`;
`NakamaParameter.cpp:15-32`, `:49-80`; `DspProgramm.cpp:250-313` und
`DspProgramm.h:87`, `:91`; `DspKern.cpp:288`, `:324`, `:395`, `:434-443`,
`:494-498`, `:500-517`, `:520-627`, `:1059`, `:1220-1239`, `:1339-1352`, `:1389`
und `DspKern.h:244`, `:599-603`; `Schleuse.h:121-145`;
`PluginProcessor.h:95-101`, `:111-114`, `:763-766`, `:1077`, `:1081`;
`PluginProcessor.cpp:874-881`; `State.cpp:265-365`; `Hostbruecke.cpp:34-145`;
`HoerMarkierung.h:295-322`, `:415-450`, `:465-500`, `:750-768`;
`PluginEditor.cpp:195-212`, `:225-285`, `:625-630`, `:686-712`, `:729-742`,
`:795-850`, `:870-945`, `:946-990`, `:1135`; `SourcesModel.cpp:1645-1668`,
`:1768-1780`; `NakamaState.h:77`; `Ipc.cpp:1203-1246`; die Tests
`TransactionTestMain.cpp:1751-1768`, `:1795`, `:2017-2036`, `:2040-2062`,
`:2178-2202`, `:2214-2236`, `MarkierungTestMain.cpp:177-187`, `:447-460`,
`:527-548`, `ShotTestMain.cpp:112-120`, `Sonde012ProjectReloadTest.cpp:440-476`,
`DspGoldenTestMain.cpp:6625-6680`, `:8180-8195`, `:8325-8350`; die Zieldefinitionen
`CMakeLists.txt:405-412`, `:425-447`, `:560-580`, `:1148-1154`, `:1320-1332`; die
JUCE-Schnittstelle `juce_CriticalSection.h:78`, `:88`, `:100` (nur `enter`,
`tryEnter`, `exit` — der Adapter aus §7.2 ist nötig und richtig); repoweit
`RtWache::Bereich` (genau ein Treffer im Produkt) und `beendeCandidate` /
`beendeHoerHalt` (Aufrufer). Geprüft habe ich außerdem je Regel R-312-1 bis
R-312-10 die Zuordnung der Zeilen aus §6.8, je Zeile das Etikett gegen den Code am
ZIEL, die Gegenfälle und Zahlenränder aus dem Etappe-1-Auftrag §6, die Reihenfolge
und die Ticketpfade der Etappen 2 bis 6, die Beine je Etappe gegen die Spalte
„Test", die Rotbeweis-, Golden- und Fixtureregel (§7.1) und die vier Aussagen zur
Etappe 7 (§7.7). Die Ticketgrenze ist gewahrt: keine Zeile verlangt eine neue
Stateversion, eine v3-Vertragsänderung, eine neue Oberfläche oder etwas, das an
U48, U49, U51 oder U56 hängt; die Etappen 2 bis 6 fassen `getBypassParameter`,
`processBlockBypassed` und `State.cpp:322` nicht an. Zwei Angaben aus §8.1 habe
ich eigens nachgemessen und **bestätigt**: `Ipc.cpp:1208`, `:1217`, `:1222-1223`
für die Wachen von `benenneSourcesHauptziel` und
`CMakeLists.txt:1320-1332` als Herkunft des Testdefines für
`EqCopSonde012ProjectReloadTest` (sein Zielblock `:562-578` setzt es nicht).

## Nicht geprüft

Kein Bau, kein Testlauf, kein Kanon, kein Skript, kein FL Studio, keine
Fehlerinjektion — der Auftrag verbietet sie. Alle Aussagen oben sind
Quellenlektüre am ZIEL. **Nur gelesen, nicht nachgemessen:** die Messwerte aus
Phase 16 (−8,99999944679 dB, +6,00000017258 dB, 3,00394 s Audio in 0,21441 s) —
ich habe ihre Fundstellen (`roh/phase-16-c-persistenz.md:15`, `:25`,
`roh/phase-16-e-last-lebenslauf.md:41`) und ihre Verträglichkeit mit der
Quellkette geprüft, nicht die Zahlen reproduziert; die Herleitung „rund jeder
zehnte Wert überlebt" (sie trifft das gemessene Offlineszenario mit Blockgröße
333, nicht die in M-20 gefahrene Blockgröße 64); die JUCE-Quellen unter
`eq-copilot/build/_deps/juce-src/` außer `juce_CriticalSection.h` — sie sind ein
Bauartefakt, und ich habe den gepinnten Stand nicht gegen den Bau verifiziert;
`BEFUNDE.md` und die Rohberichte `phase-16-f-gegenpruefung-c.md` und
`phase-16-abdeckung.md` (als Kontext gelesen, nicht Zeile für Zeile gegen die
Matrix geführt); `tools/dirigent/pruefliste.md` (Arbeitsliste, keine
Anforderungsquelle); die Sicherheitsbefunde T3-12-01 bis T3-12-05 (S25i); die
Produktkarten U48, U49, U51, U56 und alles, was an ihnen hängt; der gepinnte
JUCE-VST3-Wrapperpatch; `docs/offene-punkte.md` und `docs/plugin-wissen.md` über
die zitierten Stellen hinaus. Ob eine der neun Zeilen in D1 bis D9 im Host
tatsächlich auftritt, ist nicht gemessen — alle Aussagen sind Aussagen über
Quelltext und über die Sätze des Manifests.

---

**HEAD zu Beginn:** `b7d82137633a28e9ac42099b6568a0b44bd12b0f`
**HEAD am Ende:** `b7d82137633a28e9ac42099b6568a0b44bd12b0f`
Geschrieben wurde genau diese Datei; nichts gestaged, committet oder gepusht.

FERTIG Matrixprüfung 1, NEEDS_WORK, 9 Defekte, 3 Lücken, 3 Härtungen.
