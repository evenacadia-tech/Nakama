URTEIL: NEEDS_WORK

# Matrixprüfung 2 — NAK-312, Planschritt S25k (Wiederprüfung nach der Matrixnacharbeit 1)

Frischer, lesender Prüfer, 21.09.2026. Auftrag:
`docs/beweise/roh/NAK-312-matrixpruefung-2-auftrag.txt`. Geschrieben wurde genau
diese Datei; nichts gestaget, nichts committet, nichts gepusht. Kein Prozess des
Produkts, kein Compiler, kein Python, kein FL Studio. `briefing-hub/` und
`nimbalyst-local/` nicht angefasst.

**HEAD zu Beginn:** `416515424f91e965922d19a48a612c31b1a252c5`
**HEAD am Ende:** `416515424f91e965922d19a48a612c31b1a252c5` (gleich)

Prüfbereich gemessen:
`git diff --stat abb64275...d5e3e33a -- docs/beweise/NAK-312.md` = eine Datei,
**+728/−147**, genau ein Commit im Bereich (`d5e3e33a`). Produktcode gemessen:
`git diff --stat 090d0fa1 HEAD -- eq-copilot broker tools` ist **leer**, ebenso
`git diff --stat d5e3e33a HEAD -- eq-copilot broker tools` — die Quellen am ZIEL
sind die im Arbeitsbaum. `git status --short` zeigt vor und nach dem Lauf nur die
zwei fremden untracked Ordner.

Elf der dreizehn Befunde der Liste sind geschlossen. Drei Defekte bleiben: eine
Namenskollision, die die neuen Zeilen M-81 und M-84 erzeugt haben, und zwei
Mutationsbegründungen, die dem eigenen Bauplan widersprechen.

## 1. Defekte

### D-A — DEFEKT: die neuen Zeilen M-81 und M-84 kollidieren mit den fremden SONDE-015-Zeilen gleicher Nummer

**Welcher Satz bricht.** Der Kopf von §6 ist in dieser Runde geändert worden
(ZIEL Zeile 616-621) und lautet jetzt: „**Fremde Matrixzeilen (SONDE-015 M-nn,
SONDE-012 U-nn, SONDE-013 M-nn, NAK-246 M-nn) stehen nur in der Quellspalte, nie
als eigene Zeile**; eine Zeile hier heißt immer M-01 bis M-86 dieses Manifests."
Dieselbe Runde legt eigene Zeilen **M-81** (§6.3, ZIEL Zeile 773, „Grenzzeile
boolescher Parameter") und **M-84** (§6.7, ZIEL Zeile 851, „Szenario S2 weg von
Main") an. Die fremden Zeilen gleicher Nummer bleiben im Manifest ohne Präfix
stehen:

- SONDE-015 **M-81** = `docs/beweise/SONDE-015.md:1159` — „Host-Automation läuft
  auf einem Parameter … flüchtiges `AutomationOverlay` mit `automation_epoch`",
  nachgelesen.
- SONDE-015 **M-84** = `docs/beweise/SONDE-015.md:1162` — „Ein Projekt wird neu
  geladen … rekonstruiert denselben `state_hash`", nachgelesen.

**Zwei Stellen hat die Runde selbst falsch geschrieben** (beide im Diff, beide
innerhalb der inhaltlichen Zusagen von §6 und §7):

1. ZIEL Zeile 706, **neue Zeile M-78**, Quellspalte:
   `R-312-11; CLAUDE.md:51-52 (sinngemäß für den Hostregler, wie M-16); M-84`.
   Gemeint ist SONDE-015 M-84 (Reload/`state_hash`); gelesen wird nach der Regel
   von §6 die eigene Zeile M-84, die von Rollenwechsel und Hörmarkierung handelt.
2. ZIEL Zeile 1195-1196, **§7.3**, im neu gefassten R-312-11-Absatz:
   „M-84 verlangt weiter den geladenen Stand, M-11 misst den Gegenfall …, und
   M-78 misst den Fall ‚während‘." Die eigene Zeile M-84 sagt über den geladenen
   Stand nichts; der Satz ist als geschriebener falsch.

**Sechs bestehende Matrixzellen und ein §5.4-Satz werden mitgerissen**, weil sie
denselben bloßen Bezeichner tragen (alle am ZIEL gelesen):

- `M-84` bar: M-10 (Zeile 697, Quelle), M-12 (699, Quelle), M-14 (701, Befund
  „Hashhälfte M-84" **und** Quelle „M-84 (Hashhälfte)"), M-29 (770, Quelle),
  M-51 (834, Quelle).
- `M-81` bar: M-24 (765, Befund „T3-01-05 · M-81 · aktivieren↔abklingen" **und**
  Quelle „M-81"), M-25 (766, Quelle), dazu §5.4 Zeile 380 („Ruhegrenze M-81",
  Zitat aus §2 Zeile 183).

Korrekt präfixiert steht es nur an drei Stellen: Zeile 72, Zeile 327
(„SONDE-015 M-84") und Zeile 364 sowie 1318 („SONDE-015 M-81"). Der letzte
Beleg ist zugleich der Gegenbeweis: §7.3 Punkt 5 schreibt in derselben Runde
korrekt „SONDE-015 M-81 wäre gebrochen" — die Regel war also bekannt und ist an
den übrigen Stellen nicht angewandt worden.

**Reproduktion:** `git show d5e3e33a:docs/beweise/NAK-312.md`, Zeilen 616-621,
706, 773, 851, 1195-1196 gegen 697, 699, 701, 765, 766, 770, 834, 380;
`docs/beweise/SONDE-015.md:1159` und `:1162`.

### D-B — DEFEKT: M-38, die berichtigte Mutation behauptet einen Sperrenzähler, den der Code nicht führt

**Welcher Satz bricht.** M-38 (§6.4, ZIEL Zeile 858, in dieser Runde geändert)
setzt als Mutation: „den Provider `v3Status()` aus `processBlock` rufen → er
nimmt dort `zustandSchloss` (`SondeProcessor.cpp:1139`) und seine Zeichenketten
(`:1145`, `:1172`) allokieren im Bereich → **beide Zähler steigen** → rot".

Am ZIEL nachgemessen:

- `eq-copilot/plugin/sonde/SondeProcessor.cpp:1139` ist
  `const juce::ScopedLock l (zustandSchloss);` — eine gewöhnliche JUCE-Sperre.
- `RtWache::sperren()` steigt ausschließlich über `RtWache::meldeSperre()`
  (`eq-copilot/plugin/dsp/DspRtWache.h:50-53`), und das ruft allein
  `RtWache::GemeldeteSperre` (`:78-93`). Ein `juce::ScopedLock` ist für den
  Zähler unsichtbar.
- §7.2 (ZIEL Zeile 1085-1091) gibt den gemeldeten Adapter **genau einer** Stelle:
  „`setNonRealtime` nimmt `zustandSchloss` über `RtWache::GemeldeteSperre<Adapter>`
  statt über `juce::ScopedLock` (`SondeProcessor.cpp:1325`)". Keine andere Nahme
  des Zustandsschlosses wird gemeldet, auch `:1139` nicht.

**Bricht:** M-04 (§6.1), das denselben Sachverhalt für dieselbe Sperre wörtlich
festhält („heute `juce::ScopedLock` bei `SondeProcessor.cpp:1325`, für den
Zähler unsichtbar"), und §7.2 als Bauplansatz.

**Wirkung.** Der Rotbeweis fällt — aber nur über die Allokationshälfte:
`:1145` und `:1172` rufen `toStdString()`, und B7 zählt Allokationen
(`eq-copilot/plugin/tests/TransactionTestMain.cpp:76-91`). Die Sperrenhälfte der
Begründung wiederholt genau den Fehler, den H3 beanstandet hat: eine Sperre wird
dort behauptet, wo der Zähler bauartbedingt nichts sieht.

### D-C — DEFEKT: M-81, die Mutationsfolge widerspricht der Schiedsregel derselben Runde

**Welcher Satz bricht.** M-81 (§6.3, ZIEL Zeile 773, neu), Rotbeweisspalte:
„einmal gebrochen: `eq_enabled` in die Abdeckungstabelle aufnehmen → es gibt
kein Rampenziel dafür, **der Blockrand quittiert den Zähler trotzdem und der
Wert verschwindet für den Worker** → der Schalter wirkt gar nicht mehr → rot".

Dagegen §7.3, ebenfalls in dieser Runde geschrieben:

- Punkt 1 (ZIEL Zeile 1272-1277): der Blockrand liest die Zähler und schreibt
  danach allein `blockrandStand[i]`. M-82 (Zeile 774) sagt dazu: „der
  Blockrandstand ist ein `std::uint32_t`, der dem Audiothread allein gehört".
- Punkt 5 (ZIEL Zeile 1290-1296): „`hostEreignisOffen` (`SondeProcessor.h:544`)
  behält **genau einen** Verbraucher, den Worker (`SondeProcessor.cpp:1455`). Der
  Blockrand liest und tauscht es nicht."

Am ZIEL nachgemessen: `SondeProcessor.cpp:1455` ist
`if (hostEreignisOffen.exchange (false, std::memory_order_relaxed))`, `:1460` ist
`if (n == hostEreignisGesehen[(size_t) i]) continue;`, `:1461` ist
`hostEreignisGesehen[(size_t) i] = n;`, `:1462` schreibt ins Overlay. Der
Blockrand fasst keines dieser Felder an. Ein Eintrag in der Abdeckungstabelle
kann dem Worker den Wert also nicht nehmen; `eq_enabled` wirkte weiter über
`aus.eqEngagiert` (`eq-copilot/plugin/dsp/DspProgramm.cpp:263`, nachgemessen).

**Wirkung.** Die Zeile wird durch die Mutation trotzdem rot, aber an der
Mengenprüfung von M-79 („er steht nicht in der Abdeckungstabelle"), nicht an der
genannten Kette. Die Begründung ist als geschriebene falsch.

## 2. Befundliste

| Befund | geschlossen ja/nein | Beleg |
|---|---|---|
| D1 — Weg 2 deckte nur globale Rampenziele | **ja** | Menge namentlich und codegleich in §5.4 (Zeile 396-405), §6.3 (Zeile 741-753) und §7.3 (Zeile 1232-1240): `v1.global.input_trim_db`, `v1.global.output_trim_db`, `v1.global.width`, `v2.global.mix`. Selbst nachgemessen: `NakamaParameter.cpp:50-52` und `:80` tragen genau diese IDs; `DspKern.cpp:620-623` sind genau vier Rampenziele, `:626` ist `autoGain` aus `DspProgramm.cpp:367` (abgeleitet, kein Hostparameter). R-312-10 in §5.4 wörtlich aus §11.3 (Zeichen für Zeichen verglichen). Grenzzeilen M-80 (Bandwert) und M-81 (boolesch) sagen ausdrücklich keine Gleichheit zu und verweisen auf NAK-340. Blockrandzelle samt Ausnahme: `zelleAusHost` `:1394-1420`, Ausnahme `:1403-1404`, bestätigter Wert `:1398` — alle nachgemessen. §7.3 „Wie die bestätigten Werte dem Callback ohne Sperre vorliegen" nennt den Weg. Etiketten M-22 und M-27 berichtigt |
| D2 — Ordnungsvergleich auf überlaufendem Zähler | **ja** | §7.3 Punkte 1-5 (Zeile 1269-1296): Gleichheitsvergleich `!=`, Lesen vor der Programmübernahme (`DspKern.cpp:531` = `uebernehmeBereiten`, nachgemessen), Zählerstand je Publikation aus `hostEreignisGesehen` (`:1461`), `hostEreignisOffen` (`SondeProcessor.h:544`) allein beim Worker (`:1455`). M-19 unverändert (Kontextzeile im Diff). Speicher benannt: 16 + 96 + 48 Byte — nachgerechnet gegen `DspBankPool.h:128` (`kBaenke = 4`) und `DspKern.h:596` (zwei Vorbauplätze). Zählerrand M-82 neu |
| D3, D4 — `beendeCandidate` aus dem Callback; M-41 als Regressionswache | **ja** | §7.5 Grundsatz und Schritte 1-4 (Zeile 1471-1540). An der Quelle nachgemessen: `DspKern.cpp:441` schreibt `merkzettel[...].gueltig = false`, `:442` ruft `publiziereEnde` → `DspBankPool.cpp:157-162` → `verdraengeBereiten`; Besitzregeln wörtlich bei `DspKern.h:599-602`, `:157`, `DspBankPool.h:37-39`, `:154`. Offline-Riegel schließt das Fenster bei `DspKern.cpp:1113-1115`. Hartes Schalten im audiothread-eigenen Zustand `DspKern.h:609-613`, Wirkung `:1237-1239`. M-41 trägt nur noch die Hörmatrixhälfte, M-83 die Buchhaltung; `previewAktiv()` wird nur nach `kontrollTaktFuerTest()` geprüft (M-83, M-42, §7.5). B7-Fall `:2216-2234` ausdrücklich umgestellt, Verwurfszeile `:2228` und Prüfzeile `:2231` nachgemessen. M-46 misst am Takt |
| D5 — B6-Behauptung über einen Prozessor, den B6 nicht baut | **ja** | §7.2 (Zeile 1096-1113) und §8 Zeile E. Nachgemessen: `eq-copilot/plugin/CMakeLists.txt:1148-1154` übersetzt für `EqCopDspGoldenTest` genau `tests/DspGoldenTestMain.cpp` plus `nakama_kern_anbinden`; weder `sonde/SondeProcessor.cpp` noch `NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3` kommen darin vor. B7 `EqCopTransactionTest` (`CMakeLists.txt:1162-1173`) baut die Sonde sehr wohl. Den Satz bekommen nur B7, A16 und das neue Bein |
| D6, L3 — M-55 und die fehlende Richtung | **ja** | R-312-8 in §5.9 wörtlich aus §11.3 (verglichen), beide Richtungen. Ort nachgemessen: `State.cpp:262-273` ist `spiegleKlassifikation`, `:272` der Store `istMainKlassifiziert.store (lebenslauf.audioAusnahmeErlaubt())`; gerufen aus `setzeBindung` (`:344`) und beiden Zweigen von `setStateInformation` (`:192`, `:215`). Geordnetes Lesen: `PluginProcessor.cpp:874` ist heute der relaxed Load, Briefkasten `acq_rel` bei `HoerMarkierung.h:360-361`. Weicher Ausfade: Rücknahme über `reicheAus()` (`:411-414`), NAK-47-Kommentar `:420-429` wörtlich zitiert, `end` am Fadeende `:626-641`. S1 = M-55, S2 = M-84, beide deterministisch über 40 Blöcke. `State.cpp` und ein Tick-Testzugang in `PluginEditor.h` sind Ticketpfade (§7.0); `PluginEditor.h:50-51` und `:71` bestätigen die private `juce::Timer`-Ableitung, `:60-68` das Muster. M-57 berichtigt: Knopf nicht sichtbar |
| D7 — M-74 verlangte, was der Bauplan nicht baute | **ja** | §7.6 „Die Aktionssteuerung bindet nur an eine gezeichnete Zeile" (Zeile 1730-1756). Nachgemessen: `SourcesModel.cpp:1774-1775` ist der Frühausstieg von `stelleZielSicher` (greift nur bei fehlendem Ziel), `:1651-1664` die Sortierung, `:1781-1789` die Namensherkunft, `:1670` `waehleHauptziel`, `:1748` `setzeFixtureFuerTest`. Editor: `PluginEditor.cpp:905-907` sucht heute über die ganze Liste, `:919` und `:933` schalten sichtbar, `:1135` und `:980` rechnen `sourcesZeilen()`. Die Zeilenrechnung ist nachgerechnet: `:958-959` ergibt `jlimit (18, 34, 366 / 25)` = 18 und 366 / 18 = 20. Kein automatischer Zielwechsel, U02 wörtlich bei `SONDE-012.md:231` geprüft. Zusammenspiel mit R-312-9 über M-85; `:209-210`, `:210`, `:876-877`, `:908-909` nachgemessen |
| D8, D9 — M-04 gegen M-40; M-39 an der Wanduhr | **ja** | M-04 trägt zwei Hälften mit ausdrücklichem Etappenwechsel; §7.5 „Die Matrixzeile M-04" nennt die Umstellung im selben Änderungssatz. M-39 ordnet über drei Teilfälle mit benannter Hakenlage; alle drei am Code nachgemessen: Fall 1 ergibt `gewartetMs` ≥ 1 (`Schleuse.h:145-165`, Mindestwert 1 bei `:165`), Fall 2 `abgewiesen` ≥ 1 (`:123-126`), Fall 3 (0, 0) durch den Frühausstieg `:148`. `kStopFristMs` hängt weder in `betreten` (`:121-139`) noch in `schliessen` (`:145-165`) — die Aussage stimmt. Kein Haken in `Schleuse.h` (§7.4); die Mutation „zweites Lesen entfernen" (`:131-136`) entfällt |
| L1 — „mitquittiert" falsch, „während des Ladens" ohne Zeile | **ja** | §7.3 (Zeile 1177-1198) widerlegt den alten Satz an der Quelle. Nachgemessen: `SondeProcessor.cpp:76` ist `thread_local int abgleichTiefe = 0;`, `:1335` prüft es, `:1336` erhöht `hostEreignis` — ein Hostwert nach der Quittierung wird nicht mitquittiert. R-312-11 gebaut mit zweiter Prüfung nach der Schleife (`:1425-1431`), Abgleich `:1428-1430`. Messende Zeile M-78 neu; Schranke `:1067-1098`, Abgleich `:1103` nachgemessen |
| L2 — Rotläufe in undefiniertem Verhalten | **ja** | R-312-12 nachgezogen in §7.1 (Zeile 1019-1033) und §6.4 (Zeile 784-799): Schranke und Marke getrennt, Marke unmittelbar vor `transaktion->revision()`. Nachgemessen: `SondeProcessor.h:537` (`transaktion`) wird nach `:523` (`controlV3`) deklariert, stirbt also zuerst; `SondeProcessor.cpp:1143` liest `transaktion->revision()` im Providerweg. M-30, M-31 und M-39 tragen den Beleg „gezählte Marke", nicht „Absturz" |
| H1 — zweites `end` nach `reset()` | **ja** | §7.6 „Reset und Release" (Zeile 1673-1700) verlangt ausdrücklich das Zurücksetzen von `warHoerbar` und `hoerbareSamples`. Nachgemessen: `HoerMarkierung.h:313-314` (`vorbereiten`) und `:678-679` (`setzeSamplerate`) tun genau das; bliebe `warHoerbar` stehen, bildete `:501` → `:511-517` beziehungsweise `:626-641` ein zweites `end`, und `:619` verlangt `! warHoerbar` für ein neues `begin`. M-59 (Transport angehalten) und M-63 (Transport läuft weiter) schließen einander nicht mehr aus; M-86 misst die ganze Folge mit zwei `begin` und zwei `end`. `PluginProcessor.cpp:978-979` wörtlich geprüft, `PluginProcessor.h:111-114` hat keine `reset`-Override |
| H3 — Rotbeweis von M-38; §7.4 nannte B7 nicht | **ja, aber mit D-B** | Beinliste und Ticketpfade nennen B7 jetzt: §7.0 Etappe 4 trägt `TransactionTestMain.cpp`, §7.4 „Beine, die einzeln zu fahren sind" nennt B7 `EqCopTransactionTest`, M-38 ebenso. Der Rotbeweis fällt jetzt auch — über `allokationen()` an `:1145`/`:1172`. Die Sperrenhälfte der Begründung ist falsch: siehe **D-B** |
| Randbeobachtungen M-22, M-27, M-46, M-57, M-59/M-63, Ticketpfade | **ja** | Workertakt am Code nachgemessen: `SondeProcessor.cpp:575-578` pollt alle 5 ms, `:738-740` schläft bis 20 ms — die Neueinstufung „heute rot bei jeder gefahrenen Blockgröße" (M-22) und „heute rot" (M-27) trifft zu. Die Zahlen stimmen: 240 Samples = 5 ms, 512 = 10,7 ms, 4096 = 85 ms bei 48 kHz. M-46 misst am Takt, M-57 verlangt kein totes Element mehr, M-59/M-63 fahren verschiedene Transportlagen, §7.0 trägt `State.cpp`, `PluginProcessor.cpp`, `PluginEditor.h` und `TransactionTestMain.cpp` |
| M-32 Zeilenberichtigung | **ja** | Nachgemessen: `ControlClient.cpp:129` ist `++k->zustand.stopFristUeberschritten;`, `:132` ist `thread.detach();`. Die alten Angaben `:130` und `:133` waren um eins verschoben; die Berichtigung stimmt. Ebenso `Hostbruecke.cpp:33` (`prepareToPlay`), `MarkierungTestMain.cpp:184` (Ende von `alsMainKlassifizieren`), `NakamaTransaktion.cpp:762`, `DspBankPool.h:128` |

## 3. Lücke

**L-A — der Kommentar, den E-312-8 falsch macht, ist nirgends zugeordnet.**
§7.6 Punkt 2 hebt das Lesen von `istMainKlassifiziert` im Audiothread von
`relaxed` auf `acquire` (`eq-copilot/plugin/src/PluginProcessor.cpp:874`). Am
ZIEL steht bei `eq-copilot/plugin/src/prozessor/State.cpp:264-271` der
Kommentar: „Der Store bleibt bewusst seq_cst … Gelesen wird im processBlock
relaxed - dort haengt kein anderer Wert daran." Nach dem Bau ist der zweite
Halbsatz doppelt falsch: nicht mehr relaxed, und es hängt sehr wohl ein anderer
Wert daran — die Rücknahme. §1 führt genau drei irreführende Kommentare, §7.5
ordnet sie ihren Änderungssätzen zu; dieser vierte, den die Runde selbst neu
erzeugt, ist in §7.6 nicht benannt. Die Zeilen liegen zwar innerhalb der dort
genannten Spanne `:262-273`, ein Satz, der die Berichtigung verlangt, fehlt.
Benannt, keine Nacharbeit.

## 4. Härtung

**H-A — die Schleusenhälfte von M-38 hat keine eigene Mutation mehr.** Die
Zusagespalte von M-38 ist unverändert und sagt weiter zu: „der Callbackpfad der
Schleuse arbeitet nur mit Atomics"; die Reihenfolgespalte verweist auf
`Schleuse.h:30-37` („`warteMutex` und `warte` gehoeren ausschliesslich dem
Schliesspfad", am ZIEL gelesen). Die berichtigte Mutation bricht diese Hälfte
nicht; eine, die es täte, bräuchte einen Haken in `Schleuse.h` (`:121-139`), und
genau den schließen E-312-10 und §7.4 ausdrücklich aus. Von keiner Zusage
verlangt, deshalb nur benannt.

## 5. Geprüft

Den Diff der Runde habe ich vollständig gelesen (1129 Zeilen Rohdiff, 22 Hunks)
und §11.2 sowie §11.3 als Maßstab danebengelegt. Die beiden wörtlichen Zitate
in §5.4 (R-312-10 in der Fassung von E-312-5) und §5.9 (R-312-8 in der Fassung
von E-312-8) habe ich Satz für Satz gegen §11.3 verglichen — sie stimmen.

**Selbst am ZIEL nachgemessen** (jede Stelle gelesen, nicht übernommen):
`NakamaParameter.cpp:15-33`, `:49-89` (daraus nachgezählt: `kHostParameter` =
109 + 3 = **112**, davon nichttopologisch durchgelassen 69 kontinuierliche plus
11 boolesche = **80**, abgedeckt **vier**, übrig **108** — die Zahlen des
Manifests stimmen); `DspKern.cpp:531`, `:434-443`, `:494-517`, `:537-557`,
`:615-627`, `:1040-1046`, `:1059`, `:1113-1116`, `:1233-1242`, `:1263`, `:1297`,
`:1349`, `:1389`; `DspKern.h:157`, `:205`, `:244`, `:399`, `:404-418`,
`:593-616`; `DspProgramm.cpp:86-92`, `:255-282`, `:364-370`; `DspProgramm.h:87`;
`DspBankPool.h:35-42`, `:126-130`, `:150-158`; `DspBankPool.cpp:153-165`;
`DspRtWache.h:30-64`, `:76-96`; `SondeProcessor.cpp:74-84`, `:136-144`,
`:186-232`, `:369-372`, `:430-439`, `:573-582`, `:735-742`, `:1060-1110`,
`:1136-1150`, `:1168-1175`, `:1318-1342`, `:1392-1435`, `:1445-1482`;
`SondeProcessor.h:222-226`, `:314-328`, `:519-548`;
`NakamaTransaktion.cpp:306-320`, `:343-347`, `:758-766`;
`NakamaLebenslauf.h:135-139`; `Schleuse.h:15-45`, `:119-140`, `:143-166`,
`:170-183`; `ControlClient.cpp:118-136`; `Intern.h:57-60`;
`TelemetryClient.cpp:28-32`; `State.cpp:188-196`, `:212-218`, `:258-278`,
`:318-352`; `PluginProcessor.cpp:870-884`, `:974-984`;
`PluginProcessor.h:109-116`; `PluginEditor.cpp:206-214`, `:228-232`, `:872-880`,
`:900-940`, `:952-962`, `:978-986`, `:1133-1137`; `PluginEditor.h:45-75`;
`HoerMarkierung.h:303-316`, `:340-364`, `:405-432`, `:470-474`, `:482-502`,
`:509-520`, `:566-572`, `:584-598`, `:616-644`, `:676-681`, `:758-766`;
`SourcesModel.cpp:1645-1672`, `:1744-1752`, `:1770-1792`;
`Hostbruecke.cpp:30-36`, `:76`, `:81`, `:97-100`, `:140-142`;
`CMakeLists.txt:1144-1174`; `TransactionTestMain.cpp:76-91`, `:1749-1770`,
`:2046-2056`, `:2214-2236`, `:3340-3380`; `MarkierungTestMain.cpp:177-187`,
`:449-460`, `:468-478`; `DspGoldenTestMain.cpp:98-110`; `tools/beweise.ps1:688`,
`:691`, `:693`, `:723`, `:728`, `:744-746`; `SONDE-015.md:1087`, `:1114`,
`:1159`, `:1162`, `:3425`; `SONDE-012.md:231`; `NAK-246.md:1008`.

**Die verlangten Nahtstellen habe ich gezielt gerechnet:**

- *Schiedsregel (E-312-6) gegen Ladestart und R-312-11.* Trägt. Ein Hostwert
  nach der Quittierung erhöht `hostEreignis` (`:1336`); der Blockrand nimmt ihn,
  weil sein Stand ungleich ist, und setzt `blockrandStand` darauf. Die
  Workerpublikation des Ladestarts trägt den älteren Stand, ist also nach §7.3
  Punkt 4 ungleich und überschreibt das Blockrandziel nicht. Der Abgleich
  überspringt denselben Parameter nach R-312-11. Regler und Klang zeigen
  dasselbe — genau das, was M-78 misst; M-29 bleibt widerspruchsfrei.
- *Offline-Riegel (E-312-7) gegen die Hörmatrixzeilen der Etappe 6 und gegen
  R-312-3.* Kein Widerspruch. Der Riegel wirkt auf `hoerwunsch` im DSP-Kern der
  Sonde; die Hörmarkierung von Gen hängt an `erlaubt`
  (`PluginProcessor.cpp:874-881`), das `! isNonRealtime()` bereits enthält.
  R-312-3 hält allein über Schritt 2, unabhängig davon, wann `beendePreview`
  läuft (§7.5), und M-42 misst Sample 0.
- *Rücknahme im Klassifikationswechsel (E-312-8) gegen `reset`,
  `releaseResources` (R-312-5) und „genau ein `end` je `begin`".* Trägt. Die
  Rücknahme erzeugt keinen Schnitt (`reicheAus` setzt nur `ausGewuenscht`,
  `:484-491`), die Rampe läuft aus, das `end` fällt einmal bei `:626-641`. Fällt
  ein `reset()` in denselben Ablauf, setzt dessen Abbruchpfad `warHoerbar`
  zurück, und der Abschluss bei `Hostbruecke.cpp:97-142` bildet genau ein `end`.
  M-84 und M-86 widersprechen sich nicht.
- *E-312-9 gegen R-312-9.* Trägt. `uebernehmeSourcesLabel` kehrt bei leerem Ziel
  still zurück (`:876-877`), deshalb muss der Abschluss vor `:908-909` liegen —
  genau so steht es in §7.6, und M-85 misst es.
- *Die zwei Hälften von M-04 gegen den Bauplan der Etappen 2 und 5.* Trägt.
  Etappe 2 macht die Sperre in `setNonRealtime` (`:1325`) über den gemeldeten
  Adapter sichtbar (200); Etappe 5 nimmt dort gar keine Sperre mehr (Schritt 1
  und Schritt 4), derselbe Lauf zählt 0, und M-40 übernimmt die
  Instrumentaussage mit derselben Zusagezeile `:1325`.

Die Zählung in §6.8 habe ich nachgerechnet (9 + 25 + 10 + 16 + 26 = 86, IDs M-01
bis M-86 lückenlos) und die Speicherrechnung in §7.3 (16 + 96 + 48 Byte); beide
stimmen. Sie sind nach dem Auftrag kein Prüfgegenstand und tragen deshalb keinen
Befund.

## 6. Nicht geprüft

Alles außerhalb des Diffs der Runde: unveränderte Matrixzeilen, unveränderte
Bauplanabschnitte, §0 bis §4, §9 bis §11 als solche, §12 als Liste (nach Auftrag
Hilfe, nicht Gegenstand — maßgeblich war der Diff selbst). Die Entscheide des
Dirigenten in §11.3 habe ich als Maßstab gelesen, nicht beurteilt; dass E-312-5
die Blockbindung auf vier Parameter begrenzt und den Rest in NAK-340 führt, ist
entschieden. Nicht Gegenstand waren H2 (Register NAK-342), die Etappe 7,
NAK-340, die Produktkarten U48, U49, U51, U56, der JUCE-VST3-Wrapperpatch und
die Sicherheitsbefunde T3-12-*.

Nicht gefahren: kein Bau, kein Test, kein Kanon, kein Broker, kein FL Studio,
kein Python. Ob die Zeilen nach dem Bau wirklich rot werden, ist damit nicht
gemessen, sondern am Code am ZIEL gerechnet — mehr verlangt die Etappe 1 nicht.
Zu `tools/dirigent/pruefliste.md` habe ich keinen Befund gebildet; sie ist
Arbeitsliste.

FERTIG Matrixprüfung 2, NEEDS_WORK, 3 Defekte, 1 Lücken, 1 Härtungen
